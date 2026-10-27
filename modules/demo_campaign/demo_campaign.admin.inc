<?php
/**
 * @file
 * Staff pages: cards, School of Workers sign-ups, outbox, CRM log, settings.
 */

/**
 * Page callback: authorization cards with their CRM status.
 */
function demo_campaign_admin_cards() {
  $build['retry'] = backdrop_get_form('demo_campaign_retry_form');

  $header = array(t('Signed'), t('Name'), t('Employer'), t('Phone'), t('Source'), t('CRM'), t('Attempts'), t('Last error'));
  $rows = array();
  $result = db_query('SELECT * FROM {demo_campaign_card} ORDER BY cid DESC LIMIT 100');
  foreach ($result as $card) {
    $rows[] = array(
      format_date($card->created, 'short'),
      check_plain($card->first_name . ' ' . $card->last_name),
      check_plain($card->employer),
      check_plain($card->phone),
      check_plain(implode(' / ', array_filter(array($card->utm_source, $card->utm_medium, $card->utm_campaign)))),
      check_plain($card->crm_status . ($card->crm_id ? ' (' . $card->crm_id . ')' : '')),
      $card->crm_attempts,
      check_plain($card->crm_error),
    );
  }
  $build['cards'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('No cards yet.'),
  );
  return $build;
}

/**
 * Form: runs the CRM retry queue now instead of waiting for cron.
 */
function demo_campaign_retry_form($form, &$form_state) {
  $count = BackdropQueue::get('demo_campaign_crm')->numberOfItems();
  $form['status'] = array(
    '#markup' => '<p>' . format_plural($count, '1 card is waiting to retry the CRM.', '@count cards are waiting to retry the CRM.') . '</p>',
  );
  $form['run'] = array(
    '#type' => 'submit',
    '#value' => t('Retry now'),
    '#access' => $count > 0,
  );
  return $form;
}

/**
 * Form submission handler for demo_campaign_retry_form().
 */
function demo_campaign_retry_form_submit($form, &$form_state) {
  $queue = BackdropQueue::get('demo_campaign_crm');
  $processed = 0;
  $limit = $queue->numberOfItems();
  while ($processed < $limit && ($item = $queue->claimItem(30))) {
    try {
      demo_campaign_crm_queue_worker($item->data);
      $queue->deleteItem($item);
    }
    catch (Exception $e) {
      $queue->releaseItem($item);
    }
    $processed++;
  }
  backdrop_set_message(t('Retried @count queued cards.', array('@count' => $processed)));
}

/**
 * Page callback: School of Workers sign-ups.
 */
function demo_campaign_admin_school() {
  $lodges = config_get('demo_campaign.settings', 'lodges');
  $courses = config_get('demo_campaign.settings', 'courses');
  $header = array(t('Requested'), t('Name'), t('Lodge'), t('Course'), t('Status'), t('Decided'));
  $rows = array();
  $result = db_query('SELECT * FROM {demo_campaign_school} ORDER BY sid DESC LIMIT 100');
  foreach ($result as $request) {
    $rows[] = array(
      format_date($request->created, 'short'),
      check_plain($request->name),
      check_plain(isset($lodges[$request->lodge]) ? $lodges[$request->lodge]['name'] : $request->lodge),
      check_plain(isset($courses[$request->course]) ? $courses[$request->course] : $request->course),
      check_plain($request->status),
      $request->decided ? format_date($request->decided, 'short') : '-',
    );
  }
  return array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('No sign-ups yet.'),
  );
}

/**
 * Page callback: every email the module has "sent".
 */
function demo_campaign_admin_outbox() {
  $build['note'] = array(
    '#markup' => '<p>' . t('This demo captures email here instead of sending it. Links in each message work.') . '</p>',
  );
  $result = db_query('SELECT * FROM {demo_campaign_outbox} ORDER BY oid DESC LIMIT 50');
  foreach ($result as $mail) {
    $body = check_plain($mail->body);
    $body = preg_replace('@(https?://[^\s<]+)@', '<a href="$1">$1</a>', $body);
    $build['mail_' . $mail->oid] = array(
      '#type' => 'fieldset',
      '#title' => check_plain($mail->subject),
      'meta' => array('#markup' => '<p><strong>' . t('To') . ':</strong> ' . check_plain($mail->recipient) . ' &middot; ' . format_date($mail->created, 'short') . '</p>'),
      'body' => array('#markup' => '<pre style="white-space: pre-wrap">' . $body . '</pre>'),
    );
  }
  if (count($build) == 1) {
    $build['empty'] = array('#markup' => '<p>' . t('No email yet.') . '</p>');
  }
  return $build;
}

/**
 * Page callback: requests the CRM stub received.
 */
function demo_campaign_admin_crm_log() {
  $header = array(t('Received'), t('Response'), t('Payload'));
  $rows = array();
  $result = db_query('SELECT * FROM {demo_campaign_stub_log} ORDER BY lid DESC LIMIT 50');
  foreach ($result as $log) {
    $pretty = json_encode(json_decode($log->payload), JSON_PRETTY_PRINT | JSON_UNESCAPED_SLASHES);
    $rows[] = array(
      format_date($log->created, 'short'),
      $log->response_code,
      '<pre>' . check_plain($pretty ? $pretty : $log->payload) . '</pre>',
    );
  }
  return array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('The CRM has not received anything yet.'),
  );
}

/**
 * Form constructor for the settings page.
 */
function demo_campaign_settings_form($form, &$form_state) {
  $config = config('demo_campaign.settings');
  $form['#config'] = 'demo_campaign.settings';
  $form['crm_simulate_outage'] = array(
    '#type' => 'checkbox',
    '#title' => t('Simulate a CRM outage'),
    '#description' => t('The stub answers 503, so new cards are saved and queued for retry instead of synced.'),
    '#default_value' => $config->get('crm_simulate_outage'),
  );
  $form['crm_endpoint'] = array(
    '#type' => 'textfield',
    '#title' => t('CRM endpoint'),
    '#description' => t('Salesforce REST path for creating a Lead. Point it at a real org to go live.'),
    '#default_value' => $config->get('crm_endpoint'),
    '#maxlength' => 255,
  );
  $form['crm_max_attempts'] = array(
    '#type' => 'number',
    '#title' => t('Attempts before a card is marked failed'),
    '#min' => 1,
    '#max' => 20,
    '#default_value' => $config->get('crm_max_attempts'),
  );
  return system_settings_form($form);
}
