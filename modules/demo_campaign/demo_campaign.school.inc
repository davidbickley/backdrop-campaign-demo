<?php
/**
 * @file
 * School of Workers sign-up and business manager approval.
 *
 * The approver comes from the lodge record, never from the form, so an
 * applicant can't route their own request to an address they control. The
 * emailed link carries a random token; only its hash is stored, it expires,
 * and it is cleared once used. Opening the link changes nothing: approval
 * needs a button press, because mail scanners open links on their own.
 */

/**
 * Page callback: School of Workers landing with the sign-up form.
 */
function demo_campaign_school_page() {
  module_load_include('inc', 'demo_campaign', 'demo_campaign.pages');
  $courses = config_get('demo_campaign.settings', 'courses');
  $html = '<section class="rb-hero rb-hero--short"><div class="rb-hero-grid">'
    . '<div class="rb-hero-copy"><span class="rb-eyebrow">' . t('School of Workers · Winter session') . '</span>'
    . '<h1>' . t('Learn the job behind the job.') . '</h1>'
    . '<p class="rb-lede">' . t('Free courses for members in good standing: @list. Your lodge business manager approves each seat, so we only need a few details.', array('@list' => implode(', ', array_map('strtolower', $courses)))) . '</p></div>'
    . '<div class="rb-hero-art"><img src="' . demo_campaign_image('school.jpg') . '" alt="' . t('A machinist in a blue hard hat works at a lathe.') . '"></div>'
    . '</div></section>';

  $build['hero'] = array('#markup' => $html);
  $build['form'] = array(
    '#prefix' => '<section class="rb-sec rb-card-sec"><div class="rb-wrap rb-card-grid">',
    '#suffix' => '<div class="rb-next"><h3>' . t('How approval works') . '</h3><ol>'
      . '<li><b>' . t('You apply') . '</b><span>' . t('Pick your lodge and a course. It takes a minute.') . '</span></li>'
      . '<li><b>' . t('Your business manager reviews it') . '</b><span>' . t('They get a secure link that works once and expires in a week.') . '</span></li>'
      . '<li><b>' . t('You hear back by email') . '</b><span>' . t('Approved members get dates and details for the session.') . '</span></li>'
      . '</ol></div></div></section>',
    'form' => backdrop_get_form('demo_campaign_school_form'),
  );
  return $build;
}

/**
 * Form constructor for the School of Workers sign-up.
 */
function demo_campaign_school_form($form, &$form_state) {
  $settings = config('demo_campaign.settings');
  $lodges = array();
  foreach ($settings->get('lodges') as $id => $lodge) {
    $lodges[$id] = $lodge['name'];
  }

  $form['#attributes']['class'][] = 'rb-card';
  $form['#attributes']['class'][] = 'rb-card--single';
  $form['head'] = array(
    '#markup' => '<div class="rb-card-head"><h3>' . t('Request a seat') . '</h3><span>' . t('Members only') . '</span></div>',
  );
  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Your name'),
    '#required' => TRUE,
    '#maxlength' => 128,
  );
  $form['email'] = array(
    '#type' => 'email',
    '#title' => t('Your email'),
    '#required' => TRUE,
  );
  $form['lodge'] = array(
    '#type' => 'select',
    '#title' => t('Your lodge'),
    '#options' => $lodges,
    '#empty_option' => t('- Choose your lodge -'),
    '#required' => TRUE,
  );
  $form['course'] = array(
    '#type' => 'select',
    '#title' => t('Course'),
    '#options' => $settings->get('courses'),
    '#empty_option' => t('- Choose a course -'),
    '#required' => TRUE,
  );
  $form['reason'] = array(
    '#type' => 'textarea',
    '#title' => t('Why do you want to attend? (optional)'),
    '#rows' => 3,
  );
  $form['actions'] = array(
    '#type' => 'actions',
    '#attributes' => array('class' => array('rb-card-foot')),
  );
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Send for approval'),
    '#attributes' => array('class' => array('rb-btn', 'rb-btn-amber')),
  );
  return $form;
}

/**
 * Form submission handler for demo_campaign_school_form().
 */
function demo_campaign_school_form_submit($form, &$form_state) {
  $settings = config('demo_campaign.settings');
  $values = $form_state['values'];
  $lodge = $settings->get('lodges.' . $values['lodge']);
  $courses = $settings->get('courses');
  $days = (int) $settings->get('review_link_days');

  $token = backdrop_random_key(32);
  $sid = db_insert('demo_campaign_school')
    ->fields(array(
      'name' => trim($values['name']),
      'email' => trim($values['email']),
      'lodge' => $values['lodge'],
      'course' => $values['course'],
      'reason' => trim($values['reason']),
      'status' => 'pending',
      'token_hash' => hash('sha256', $token),
      'expires' => REQUEST_TIME + $days * 86400,
      'created' => REQUEST_TIME,
    ))
    ->execute();

  backdrop_mail('demo_campaign', 'school_review', $lodge['manager_email'], language_default(), array(
    'request' => $values,
    'course_label' => $courses[$values['course']],
    'days' => $days,
    'review_url' => url('school/review/' . $sid . '/' . $token, array('absolute' => TRUE)),
  ));

  backdrop_set_message(t('Thanks, @name. Your request went to the business manager at @lodge. You will get an email when they decide.', array(
    '@name' => $values['name'],
    '@lodge' => $lodge['name'],
  )));
  $form_state['redirect'] = 'demo';
}

/**
 * Loads a request if the token matches, is unused and has not expired.
 */
function demo_campaign_school_load_for_review($sid, $token) {
  $request = db_query('SELECT * FROM {demo_campaign_school} WHERE sid = :sid', array(':sid' => (int) $sid))->fetchObject();
  if (!$request || empty($request->token_hash) || $request->status != 'pending') {
    return FALSE;
  }
  if (!hash_equals($request->token_hash, hash('sha256', $token)) || $request->expires < REQUEST_TIME) {
    return FALSE;
  }
  return $request;
}

/**
 * Page callback: shows the request and the approve/decline form.
 */
function demo_campaign_school_review_page($sid, $token) {
  $request = demo_campaign_school_load_for_review($sid, $token);
  if (!$request) {
    return array(
      '#markup' => '<section class="rb-sec"><div class="rb-wrap rb-narrow"><p class="rb-big">' . t('This review link has expired or has already been used.') . '</p><p>' . t('If you still need to decide on this request, please contact the lodge office.') . '</p></div></section>',
    );
  }

  $settings = config('demo_campaign.settings');
  $courses = $settings->get('courses');
  $lodge = $settings->get('lodges.' . $request->lodge);

  $rows = array(
    array(t('Name'), check_plain($request->name)),
    array(t('Email'), check_plain($request->email)),
    array(t('Lodge'), check_plain($lodge['name'])),
    array(t('Course'), check_plain($courses[$request->course])),
    array(t('Reason'), $request->reason ? check_plain($request->reason) : '-'),
    array(t('Requested'), format_date($request->created, 'medium')),
  );
  $build['details'] = array(
    '#prefix' => '<section class="rb-sec"><div class="rb-wrap rb-narrow"><div class="rb-card rb-review"><div class="rb-card-head"><h3>' . t('School of Workers request') . '</h3><span>' . t('Waiting for your decision') . '</span></div><div class="rb-review-body">',
    '#theme' => 'table',
    '#rows' => $rows,
    '#attributes' => array('class' => array('rb-dl')),
  );
  $build['form'] = backdrop_get_form('demo_campaign_school_review_form', $request, $token);
  $build['form']['#suffix'] = '</div></div><p class="rb-muted">' . t('Opening this page changes nothing. The request is only decided when you press a button.') . '</p></div></section>';
  return $build;
}

/**
 * Form constructor for the approve/decline buttons.
 */
function demo_campaign_school_review_form($form, &$form_state, $request, $token) {
  $form_state['sid'] = $request->sid;
  $form_state['token'] = $token;
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['approve'] = array(
    '#type' => 'submit',
    '#value' => t('Approve'),
    '#decision' => 'approved',
    '#attributes' => array('class' => array('rb-btn', 'rb-btn-amber')),
  );
  $form['actions']['decline'] = array(
    '#type' => 'submit',
    '#value' => t('Decline'),
    '#decision' => 'declined',
    '#attributes' => array('class' => array('rb-btn', 'rb-btn-outline')),
  );
  return $form;
}

/**
 * Form submission handler for demo_campaign_school_review_form().
 */
function demo_campaign_school_review_form_submit($form, &$form_state) {
  // Check the token again: the page may have sat open past its expiry.
  $request = demo_campaign_school_load_for_review($form_state['sid'], $form_state['token']);
  if (!$request) {
    backdrop_set_message(t('This review link has expired or has already been used.'), 'error');
    return;
  }

  $decision = $form_state['triggering_element']['#decision'];
  // The status condition makes a double-click or second tab a no-op.
  $updated = db_update('demo_campaign_school')
    ->fields(array(
      'status' => $decision,
      'token_hash' => NULL,
      'decided' => REQUEST_TIME,
    ))
    ->condition('sid', $request->sid)
    ->condition('status', 'pending')
    ->execute();
  if (!$updated) {
    return;
  }

  $request->status = $decision;
  $courses = config_get('demo_campaign.settings', 'courses');
  backdrop_mail('demo_campaign', 'school_decision', $request->email, language_default(), array(
    'request' => $request,
    'course_label' => $courses[$request->course],
  ));

  backdrop_set_message($decision == 'approved'
    ? t('Approved. @name has been notified.', array('@name' => $request->name))
    : t('Declined. @name has been notified.', array('@name' => $request->name)));
  $form_state['redirect'] = 'demo';
}
