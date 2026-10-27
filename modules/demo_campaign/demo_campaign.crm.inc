<?php
/**
 * @file
 * Pushes authorization cards to the CRM, plus a stub CRM endpoint.
 *
 * The payload uses Salesforce Lead field names and the request goes to the
 * same REST path Salesforce uses, so pointing crm_endpoint at a real org
 * (with an OAuth access token) is a configuration change, not a rewrite.
 */

/**
 * Maps a card to Salesforce Lead fields.
 */
function demo_campaign_crm_payload($card) {
  return array(
    'FirstName' => $card->first_name,
    'LastName' => $card->last_name,
    'Company' => $card->employer,
    'Title' => $card->job_title,
    'Phone' => $card->phone,
    'Email' => $card->email,
    'LeadSource' => $card->utm_source ? $card->utm_source : 'Web',
    'Description' => 'Authorization card: ' . $card->campaign,
    // Custom fields in a real org would carry these.
    'Campaign_Slug__c' => $card->campaign,
    'UTM_Medium__c' => $card->utm_medium,
    'UTM_Campaign__c' => $card->utm_campaign,
    'Preferred_Contact__c' => $card->contact_pref,
    'Card_Id__c' => $card->cid,
  );
}

/**
 * Pushes one card to the CRM and records the outcome.
 *
 * @return bool
 *   TRUE if the CRM accepted the card.
 */
function demo_campaign_crm_push($cid) {
  $card = demo_campaign_card_load($cid);
  if (!$card || $card->crm_status == 'synced') {
    return TRUE;
  }

  $config = config('demo_campaign.settings');
  $response = backdrop_http_request($config->get('crm_endpoint'), array(
    'method' => 'POST',
    'data' => backdrop_json_encode(demo_campaign_crm_payload($card)),
    'headers' => array(
      'Content-Type' => 'application/json',
      'Authorization' => 'Bearer ' . $config->get('crm_token'),
    ),
    'timeout' => 5,
  ));

  $attempts = $card->crm_attempts + 1;
  $body = isset($response->data) ? json_decode($response->data, TRUE) : NULL;

  if ($response->code == 201 && !empty($body['success'])) {
    db_update('demo_campaign_card')
      ->fields(array(
        'crm_status' => 'synced',
        'crm_id' => $body['id'],
        'crm_attempts' => $attempts,
        'crm_error' => '',
      ))
      ->condition('cid', $cid)
      ->execute();
    return TRUE;
  }

  $error = 'HTTP ' . $response->code;
  if (isset($body[0]['errorCode'])) {
    $error .= ' ' . $body[0]['errorCode'] . ': ' . $body[0]['message'];
  }
  elseif (isset($response->error)) {
    $error .= ' ' . $response->error;
  }
  $status = $attempts >= $config->get('crm_max_attempts') ? 'failed' : 'retrying';
  db_update('demo_campaign_card')
    ->fields(array(
      'crm_status' => $status,
      'crm_attempts' => $attempts,
      'crm_error' => substr($error, 0, 255),
    ))
    ->condition('cid', $cid)
    ->execute();
  watchdog('demo_campaign', 'CRM push for card @cid failed (attempt @n): @error', array(
    '@cid' => $cid,
    '@n' => $attempts,
    '@error' => $error,
  ), $status == 'failed' ? WATCHDOG_ERROR : WATCHDOG_WARNING);
  return FALSE;
}

/**
 * Page callback: a stand-in for Salesforce's "create Lead" endpoint.
 *
 * Checks the bearer token, requires LastName and Company as Salesforce does,
 * and can be switched to return 503 to show the retry queue at work.
 */
function demo_campaign_crm_stub() {
  $config = config('demo_campaign.settings');
  $raw = file_get_contents('php://input');
  $auth = isset($_SERVER['HTTP_AUTHORIZATION']) ? $_SERVER['HTTP_AUTHORIZATION'] : '';
  if (!$auth && function_exists('getallheaders')) {
    $headers = array_change_key_case(getallheaders(), CASE_LOWER);
    $auth = isset($headers['authorization']) ? $headers['authorization'] : '';
  }

  if ($_SERVER['REQUEST_METHOD'] != 'POST') {
    $code = 405;
    $result = array(array('errorCode' => 'METHOD_NOT_ALLOWED', 'message' => 'Use POST.'));
  }
  elseif (!hash_equals('Bearer ' . $config->get('crm_token'), $auth)) {
    $code = 401;
    $result = array(array('errorCode' => 'INVALID_SESSION_ID', 'message' => 'Session expired or invalid'));
  }
  elseif ($config->get('crm_simulate_outage')) {
    $code = 503;
    $result = array(array('errorCode' => 'SERVER_UNAVAILABLE', 'message' => 'Simulated outage'));
  }
  else {
    $lead = json_decode($raw, TRUE);
    if (empty($lead['LastName']) || empty($lead['Company'])) {
      $code = 400;
      $result = array(array('errorCode' => 'REQUIRED_FIELD_MISSING', 'message' => 'Required fields are missing: [LastName, Company]'));
    }
    else {
      $code = 201;
      $result = array('id' => '00Q' . strtoupper(substr(hash('sha256', $raw . microtime()), 0, 15)), 'success' => TRUE, 'errors' => array());
    }
  }

  db_insert('demo_campaign_stub_log')
    ->fields(array(
      'response_code' => $code,
      'payload' => $raw,
      'created' => REQUEST_TIME,
    ))
    ->execute();

  $reasons = array(201 => 'Created', 400 => 'Bad Request', 401 => 'Unauthorized', 405 => 'Method Not Allowed', 503 => 'Service Unavailable');
  backdrop_add_http_header('Status', $code . ' ' . $reasons[$code]);
  backdrop_json_output($result);
  backdrop_exit();
}
