<?php
/**
 * @file
 * Public pages: the lodge home, campaign pages and the authorization card.
 */

/**
 * Returns the URL of an image shipped with the Riverbend theme.
 */
function demo_campaign_image($file) {
  return base_path() . backdrop_get_path('theme', 'riverbend') . '/images/' . $file;
}

/**
 * Counts a campaign's cards: those entered from paper plus those signed online.
 */
function demo_campaign_card_count($campaign) {
  // A page showing the live count must not be served from the page cache.
  backdrop_page_is_cacheable(FALSE);
  $online = db_query('SELECT COUNT(*) FROM {demo_campaign_card} WHERE campaign = :c', array(':c' => $campaign['slug']))->fetchField();
  return (int) $campaign['signed_offline'] + (int) $online;
}

/**
 * Renders the signature tally with its progress bar.
 */
function demo_campaign_tally($campaign) {
  $count = demo_campaign_card_count($campaign);
  $size = (int) $campaign['unit_size'];
  $pct = min(100, round($count / $size * 100));
  $goal = (int) $campaign['goal_percent'];
  return '<div class="rb-tally">'
    . '<div class="rb-tally-top"><div class="rb-tally-num">' . $count . ' <span>' . t('of @size', array('@size' => $size)) . '</span></div>'
    . '<div class="rb-tally-label">' . t('@employer coworkers <br>have signed a card', array('@employer' => $campaign['title'])) . '</div></div>'
    . '<div class="rb-bar" role="img" aria-label="' . t('@pct percent signed', array('@pct' => $pct)) . '"><i style="width:' . $pct . '%"></i><em style="left:30%"></em><em style="left:' . $goal . '%"></em></div>'
    . '<div class="rb-bar-legend"><span>0</span><span>' . t('30% · enough to file') . '</span><span>' . t('@goal% · our goal', array('@goal' => $goal)) . '</span><span>' . $size . '</span></div>'
    . '</div>';
}

/**
 * Page callback: the lodge home page, which also explains the demo.
 */
function demo_campaign_demo_page() {
  $campaign = demo_campaign_campaign_load('harbor-fab');
  $link = url('organize/harbor-fab', array('query' => array('utm_source' => 'facebook', 'utm_medium' => 'paid', 'utm_campaign' => 'fall-drive')));

  $html = '<section class="rb-hero rb-hero--home"><div class="rb-hero-grid">'
    . '<div class="rb-hero-copy"><span class="rb-eyebrow">' . t('Riverbend Lodge 412') . '</span>'
    . '<h1>' . t('Skilled trades. One voice.') . '</h1>'
    . '<p class="rb-lede">' . t('Lodge 412 represents fabrication, machining and heavy-metal trades workers across the Riverbend valley. Right now we are organizing at Harbor Fabrication, and our School of Workers is enrolling for the winter session.') . '</p>'
    . '<div class="rb-actions"><a class="rb-btn rb-btn-amber" href="' . $link . '">' . t('See the Harbor campaign') . '</a><a class="rb-btn rb-btn-ghost" href="' . url('school/apply') . '">' . t('School of Workers') . '</a></div></div>'
    . '<div class="rb-hero-art"><img src="' . demo_campaign_image('how.jpg') . '" alt="' . t('Sparks fly from a welder working on steel tubing.') . '">' . demo_campaign_tally($campaign) . '</div>'
    . '</div></section>';

  $html .= '<section class="rb-sec"><div class="rb-wrap">'
    . '<div class="rb-section-head"><span class="rb-eyebrow rb-eyebrow--dark">' . t('About this demo') . '</span><h2>' . t('Two workflows, built on Backdrop') . '</h2>'
    . '<p>' . t('This site is a working prototype with a fictional lodge. Everything below runs on a custom Backdrop module: real forms, a real database and a real queue. Email is captured in a staff outbox instead of being sent.') . '</p></div>'
    . '<div class="rb-tiles">'
    . '<a class="rb-tile" href="' . $link . '"><img src="' . demo_campaign_image('floor.jpg') . '" alt=""><div><h3>' . t('Organizing campaign') . '</h3><p>' . t('An issue page with an authorization card. Cards alert an organizer, sync to a Salesforce-style endpoint and retry if it is down. The link carries ad tracking tags, as a paid post would.') . '</p><span class="rb-btn rb-btn-dark">' . t('Open the campaign') . '</span></div></a>'
    . '<a class="rb-tile" href="' . url('school/apply') . '"><img src="' . demo_campaign_image('school.jpg') . '" alt=""><div><h3>' . t('School of Workers') . '</h3><p>' . t('A course sign-up that the lodge business manager approves from an emailed link that expires and works once. The applicant is told the decision.') . '</p><span class="rb-btn rb-btn-dark">' . t('Open the sign-up') . '</span></div></a>'
    . '<a class="rb-tile" href="' . url('admin/demo-campaign') . '"><img src="' . demo_campaign_image('card.jpg') . '" alt=""><div><h3>' . t('Staff dashboard') . '</h3><p>' . t('Cards and their CRM status, sign-ups, the email outbox, the exact JSON the CRM received, and a switch that simulates a CRM outage. Requires a staff login.') . '</p><span class="rb-btn rb-btn-dark">' . t('Open the dashboard') . '</span></div></a>'
    . '</div></div></section>';

  return array('#markup' => $html);
}

/**
 * Page callback: a campaign's issue page with the card form.
 */
function demo_campaign_campaign_page($campaign) {
  $card_url = '#card';

  $html = '<section class="rb-hero"><div class="rb-hero-grid">'
    . '<div class="rb-hero-copy"><span class="rb-eyebrow">' . check_plain($campaign['eyebrow']) . '</span>'
    . '<h1>' . check_plain($campaign['headline']) . '</h1>'
    . '<p class="rb-lede">' . check_plain($campaign['lede']) . '</p>'
    . '<div class="rb-actions"><a class="rb-btn rb-btn-amber" href="' . $card_url . '">' . t('Sign your card') . '</a><a class="rb-btn rb-btn-ghost" href="#rights">' . t('Know your rights') . '</a></div></div>'
    . '<div class="rb-hero-art"><img src="' . demo_campaign_image($campaign['hero_image']) . '" alt="' . t('A welder in a blue jacket and leather sleeves works on steel as sparks fly.') . '">' . demo_campaign_tally($campaign) . '</div>'
    . '</div></section>';

  $issues = '';
  foreach ($campaign['issues'] as $i => $issue) {
    $issues .= '<div class="rb-issue"><div class="rb-num">' . sprintf('%02d', $i + 1) . '</div><h3>' . check_plain($issue['title']) . '</h3><p>' . check_plain($issue['body']) . '</p></div>';
  }
  $html .= '<section class="rb-sec rb-sec--issues" id="issues"><div class="rb-wrap">'
    . '<div class="rb-section-head"><span class="rb-eyebrow rb-eyebrow--dark">' . t('What we keep hearing') . '</span><h2>' . t('Three problems, one shop floor') . '</h2><p>' . check_plain($campaign['issues_intro']) . '</p></div>'
    . '<div class="rb-issues">' . $issues . '</div>'
    . '<figure class="rb-quote"><img src="' . demo_campaign_image('floor.jpg') . '" alt="' . t('Late sun through the windows of an empty fabrication shop.') . '"><blockquote><p>' . check_plain($campaign['quote']) . '</p><cite>' . check_plain($campaign['quote_cite']) . '</cite></blockquote></figure>'
    . '</div></section>';

  $html .= '<section class="rb-sec rb-dark" id="how" style="--rb-bg:url(' . demo_campaign_image('how.jpg') . ')"><div class="rb-wrap">'
    . '<div class="rb-section-head"><span class="rb-eyebrow">' . t('How it works') . '</span><h2>' . t('From one card to a contract') . '</h2><p>' . t('Every step is set out in federal law, and you get a say at every one of them.') . '</p></div>'
    . '<div class="rb-steps">'
    . '<div class="rb-step"><span class="rb-n">' . t('Step 1') . '</span><h3>' . t('Sign a card') . '</h3><p>' . t('Your card tells us you want a union. It goes to the lodge, not to @employer management.', array('@employer' => $campaign['title'])) . '</p></div>'
    . '<div class="rb-step"><span class="rb-n">' . t('Step 2') . '</span><h3>' . t('Reach 30%, then more') . '</h3><p>' . t('At 30% we can ask the National Labor Relations Board for an election. We wait for a strong majority first.') . '</p></div>'
    . '<div class="rb-step"><span class="rb-n">' . t('Step 3') . '</span><h3>' . t('Vote by secret ballot') . '</h3><p>' . t('The NLRB runs the election. If most votes say yes, Lodge 412 represents your shop.') . '</p></div>'
    . '<div class="rb-step"><span class="rb-n">' . t('Step 4') . '</span><h3>' . t('Bargain, then ratify') . '</h3><p>' . t('Coworkers you elect sit at the table. The contract only takes effect if members vote to approve it.') . '</p></div>'
    . '</div></div></section>';

  $html .= '<section class="rb-sec" id="rights"><div class="rb-wrap rb-rights">'
    . '<div><div class="rb-section-head"><span class="rb-eyebrow rb-eyebrow--dark">' . t('Your rights') . '</span><h2>' . t('The law is on your side') . '</h2><p>' . t('The National Labor Relations Act protects private-sector workers who organize.') . '</p></div>'
    . '<ul class="rb-checks">'
    . '<li>' . t('You have the right to talk with coworkers about forming a union, on breaks and off the clock.') . '</li>'
    . '<li>' . t("Your employer can't fire, discipline, threaten or question you for supporting the union.") . '</li>'
    . '<li>' . t("Your employer can't promise rewards for voting no.") . '</li>'
    . '<li>' . t('If any of that happens, tell an organizer. We document it and file charges with the NLRB.') . '</li>'
    . '</ul></div>'
    . '<div class="rb-faq">'
    . '<details open><summary>' . t('Will my supervisor see my card?') . '</summary><p>' . t('No. Cards come to the lodge. If we file for an election, the NLRB checks them confidentially and @employer never sees who signed.', array('@employer' => $campaign['title'])) . '</p></details>'
    . '<details><summary>' . t('Does signing cost anything?') . '</summary><p>' . t('No. Nobody pays dues until members vote to approve a first contract.') . '</p></details>'
    . '<details><summary>' . t('Is a card the same as a vote?') . '</summary><p>' . t('No. A card says you want an election. The vote itself is a secret ballot run by the NLRB.') . '</p></details>'
    . '<details><summary>' . t('Can I talk to someone first?') . '</summary><p>' . t('Yes. Choose "call" or "text" on the card and an organizer will reach out within one business day.') . '</p></details>'
    . '</div></div></section>';

  $build['top'] = array('#markup' => $html);

  $build['card'] = array(
    '#prefix' => '<section class="rb-sec rb-card-sec" id="card"><div class="rb-wrap rb-card-grid">',
    '#suffix' => '<div class="rb-next"><img class="rb-next-img" src="' . demo_campaign_image('card.jpg') . '" alt="' . t('A worker measures a steel frame on the shop floor.') . '"><h3>' . t('What happens next') . '</h3><ol>'
      . '<li><b>' . t('Within one business day') . '</b><span>' . t('An organizer calls or texts you, outside work hours unless you say otherwise.') . '</span></li>'
      . '<li><b>' . t('You get the campaign updates') . '</b><span>' . t('Meeting times, the latest count, and what coworkers are saying.') . '</span></li>'
      . '<li><b>' . t('You decide how involved to be') . '</b><span>' . t('Some people sign and wait for the vote. Others join the organizing committee.') . '</span></li>'
      . '</ol><p class="rb-lock"><span class="rb-mark rb-mark--sm">§7</span>' . t('Protected by Section 7 of the National Labor Relations Act.') . '</p></div></div></section>',
    'form' => backdrop_get_form('demo_campaign_card_form', $campaign),
  );

  $build['closing'] = array(
    '#markup' => '<section class="rb-closing" style="--rb-bg:url(' . demo_campaign_image('crew.jpg') . ')"><div class="rb-wrap">'
      . '<div><h2>' . t('@count coworkers have already signed.', array('@count' => demo_campaign_card_count($campaign))) . '</h2><p>' . t('It takes about two minutes, and your supervisor will never see it.') . '</p></div>'
      . '<a class="rb-btn rb-btn-dark rb-btn-lg" href="#card">' . t('Sign your card') . '</a></div></section>',
  );
  return $build;
}

/**
 * Form constructor for the authorization card.
 */
function demo_campaign_card_form($form, &$form_state, $campaign) {
  $form_state['campaign'] = $campaign;
  $form['#attributes']['class'][] = 'rb-card';

  $form['head'] = array(
    '#markup' => '<div class="rb-card-head"><h3>' . t('Sign your card') . '</h3><span>' . t('About two minutes') . '</span></div>',
  );
  $form['fields'] = array(
    '#type' => 'container',
    '#attributes' => array('class' => array('rb-card-body')),
  );
  $f = &$form['fields'];
  $f['first_name'] = array(
    '#type' => 'textfield',
    '#title' => t('First name'),
    '#required' => TRUE,
    '#maxlength' => 64,
  );
  $f['last_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Last name'),
    '#required' => TRUE,
    '#maxlength' => 64,
  );
  $f['employer'] = array(
    '#type' => 'textfield',
    '#title' => t('Employer'),
    '#default_value' => $campaign['title'],
    '#required' => TRUE,
    '#maxlength' => 128,
  );
  $f['job_title'] = array(
    '#type' => 'textfield',
    '#title' => t('Job title'),
    '#attributes' => array('placeholder' => t('Welder, fitter, operator…')),
    '#maxlength' => 128,
  );
  $f['phone'] = array(
    '#type' => 'tel',
    '#title' => t('Mobile phone'),
    '#required' => TRUE,
    '#description' => t('Never shared with your employer.'),
  );
  $f['email'] = array(
    '#type' => 'email',
    '#title' => t('Personal email (optional)'),
    '#description' => t('Not a work address.'),
  );
  $f['contact_pref'] = array(
    '#type' => 'radios',
    '#title' => t('Best way to reach you'),
    '#options' => array('text' => t('Text'), 'call' => t('Call'), 'email' => t('Email')),
    '#default_value' => 'text',
    '#required' => TRUE,
    '#wrapper_attributes' => array('class' => array('rb-full', 'rb-pills')),
  );
  $f['authorize'] = array(
    '#type' => 'checkbox',
    '#title' => t('I authorize the Allied Metal Trades Union to represent me in collective bargaining with my employer.'),
    '#required' => TRUE,
    '#wrapper_attributes' => array('class' => array('rb-full', 'rb-auth')),
  );

  // Honeypot: hidden from people, filled in by most bots.
  $f['website'] = array(
    '#type' => 'textfield',
    '#title' => t('Leave this field blank'),
    '#attributes' => array('autocomplete' => 'off', 'tabindex' => '-1'),
    '#wrapper_attributes' => array('class' => array('rb-hp')),
  );

  // Where the visitor came from, read from the URL the ad or post linked to.
  foreach (array('utm_source', 'utm_medium', 'utm_campaign') as $key) {
    $form[$key] = array(
      '#type' => 'hidden',
      '#default_value' => isset($_GET[$key]) ? substr($_GET[$key], 0, 64) : '',
    );
  }

  $form['actions'] = array(
    '#type' => 'actions',
    '#attributes' => array('class' => array('rb-card-foot')),
  );
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Sign my card'),
    '#attributes' => array('class' => array('rb-btn', 'rb-btn-amber')),
    '#suffix' => '<small>' . t('Your card is confidential. @employer will not see it.', array('@employer' => $campaign['title'])) . '</small>',
  );
  return $form;
}

/**
 * Form validation handler for demo_campaign_card_form().
 */
function demo_campaign_card_form_validate($form, &$form_state) {
  $values = $form_state['values'];

  if ($values['website'] !== '') {
    form_set_error('', t('Your card could not be submitted.'));
    return;
  }

  // Limit each visitor to 5 cards an hour.
  if (!flood_is_allowed('demo_campaign_card', 5, 3600)) {
    form_set_error('', t('Too many cards have been submitted from your connection. Please try again later.'));
    return;
  }

  $digits = preg_replace('/\D+/', '', $values['phone']);
  if (strlen($digits) == 11 && $digits[0] == '1') {
    $digits = substr($digits, 1);
  }
  if (strlen($digits) != 10) {
    form_set_error('phone', t('Please enter a 10-digit phone number.'));
  }
  else {
    form_set_value($form['fields']['phone'], $digits, $form_state);
  }

  if ($values['email'] !== '' && !valid_email_address($values['email'])) {
    form_set_error('email', t('Please enter a valid email address.'));
  }
  if ($values['contact_pref'] == 'email' && $values['email'] === '') {
    form_set_error('email', t('Add an email address, or choose text or call.'));
  }
}

/**
 * Form submission handler for demo_campaign_card_form().
 *
 * Saves the card first, so a CRM outage can never lose a signature, then
 * alerts the organizer and tries the CRM. A failed push goes to the queue.
 */
function demo_campaign_card_form_submit($form, &$form_state) {
  $campaign = $form_state['campaign'];
  $values = $form_state['values'];
  flood_register_event('demo_campaign_card', 3600);

  $cid = db_insert('demo_campaign_card')
    ->fields(array(
      'campaign' => $campaign['slug'],
      'first_name' => trim($values['first_name']),
      'last_name' => trim($values['last_name']),
      'employer' => trim($values['employer']),
      'job_title' => trim($values['job_title']),
      'phone' => $values['phone'],
      'email' => trim($values['email']),
      'contact_pref' => $values['contact_pref'],
      'utm_source' => substr($values['utm_source'], 0, 64),
      'utm_medium' => substr($values['utm_medium'], 0, 64),
      'utm_campaign' => substr($values['utm_campaign'], 0, 64),
      'created' => REQUEST_TIME,
    ))
    ->execute();

  $card = demo_campaign_card_load($cid);
  backdrop_mail('demo_campaign', 'card_alert', $campaign['organizer_email'], language_default(), array(
    'card' => $card,
    'campaign_title' => $campaign['title'],
  ));

  module_load_include('inc', 'demo_campaign', 'demo_campaign.crm');
  if (!demo_campaign_crm_push($cid)) {
    BackdropQueue::get('demo_campaign_crm')->createItem($cid);
  }

  $_SESSION['demo_campaign_first_name'] = trim($values['first_name']);
  $form_state['redirect'] = 'organize/' . $campaign['slug'] . '/thanks';
}

/**
 * Page callback: thank-you page, which also records an analytics event.
 */
function demo_campaign_thanks_page($campaign) {
  $name = isset($_SESSION['demo_campaign_first_name']) ? $_SESSION['demo_campaign_first_name'] : '';
  unset($_SESSION['demo_campaign_first_name']);

  // Records a Pirsch custom event when the Pirsch script is on the page.
  $js = 'if (window.pirsch) { pirsch("Authorization card", {meta: {campaign: ' . backdrop_json_encode($campaign['slug']) . '}}); }';
  $build['#attached']['js'][] = array('data' => $js, 'type' => 'inline', 'scope' => 'footer');
  backdrop_set_title(t('Your card is in'));

  $build['body'] = array(
    '#markup' => '<section class="rb-sec"><div class="rb-wrap rb-thanks">'
      . '<div><p class="rb-big">' . ($name ? t('Thank you, @name.', array('@name' => $name)) : t('Thank you.')) . '</p>'
      . '<p>' . t('An organizer from the @title campaign will reach out within one business day. Your card is confidential, and @title will not see it.', array('@title' => $campaign['title'])) . '</p>'
      . demo_campaign_tally($campaign)
      . '<p><a class="rb-btn rb-btn-dark" href="' . url('organize/' . $campaign['slug']) . '">' . t('Back to the campaign') . '</a></p></div>'
      . '<img src="' . demo_campaign_image('crew.jpg') . '" alt="' . t('A crew in hard hats climbs a stairway together.') . '">'
      . '</div></section>',
  );
  return $build;
}
