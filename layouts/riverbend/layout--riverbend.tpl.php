<?php
/**
 * @file
 * Riverbend layout: lodge header, full-width content, lodge footer.
 *
 * Pages that draw their own hero (campaign pages, the lodge home) set
 * $riverbend_hide_title in riverbend_preprocess_layout().
 */
$campaign = url('organize/harbor-fab');
?>
<div class="rb <?php print implode(' ', $classes); ?>"<?php print backdrop_attributes($attributes); ?>>
  <a href="#main-content" class="element-invisible element-focusable"><?php print t('Skip to main content'); ?></a>

  <div class="rb-util"><div class="rb-wrap">
    <span><?php print t('Allied Metal Trades Union · Riverbend Lodge 412'); ?></span>
    <span><?php print t('Talking to an organizer is confidential'); ?></span>
  </div></div>

  <header class="rb-head" role="banner">
    <div class="rb-wrap">
      <a class="rb-brand" href="<?php print url('<front>'); ?>">
        <span class="rb-mark">412</span>
        <span><b><?php print t('Riverbend Lodge 412'); ?></b><small><?php print t('Allied Metal Trades Union'); ?></small></span>
      </a>
      <nav class="rb-nav" aria-label="<?php print t('Main'); ?>">
        <a href="<?php print $campaign; ?>"><?php print t('Harbor campaign'); ?></a>
        <a href="<?php print url('school/apply'); ?>"><?php print t('School of Workers'); ?></a>
        <a href="<?php print $campaign; ?>#rights"><?php print t('Your rights'); ?></a>
        <a class="rb-btn rb-btn-amber" href="<?php print $campaign; ?>#card"><?php print t('Sign your card'); ?></a>
      </nav>
    </div>
  </header>

  <a id="main-content"></a>
  <?php if ($messages): ?>
    <div class="rb-messages rb-wrap" role="status"><?php print $messages; ?></div>
  <?php endif; ?>

  <?php if (empty($riverbend_hide_title) && $title): ?>
    <div class="rb-pagehead"><div class="rb-wrap">
      <?php print render($title_prefix); ?>
      <h1><?php print $title; ?></h1>
      <?php print render($title_suffix); ?>
    </div></div>
  <?php endif; ?>

  <?php if ($tabs): ?>
    <nav class="tabs rb-wrap" role="tablist"><?php print $tabs; ?></nav>
  <?php endif; ?>
  <?php print $action_links; ?>

  <main class="rb-main" role="main">
    <?php print $content['content']; ?>
  </main>

  <footer class="rb-foot">
    <div class="rb-wrap">
      <div class="rb-foot-grid">
        <div><h4><?php print t('Riverbend Lodge 412'); ?></h4><p><?php print t('Allied Metal Trades Union. Representing fabrication, machining and heavy-metal trades workers in the Riverbend valley.'); ?></p></div>
        <div><h4><?php print t('Organizing'); ?></h4><ul>
          <li><a href="<?php print $campaign; ?>"><?php print t('Harbor Fabrication'); ?></a></li>
          <li><a href="<?php print $campaign; ?>#how"><?php print t('How it works'); ?></a></li>
          <li><a href="<?php print $campaign; ?>#rights"><?php print t('Your rights'); ?></a></li>
        </ul></div>
        <div><h4><?php print t('Members'); ?></h4><ul>
          <li><a href="<?php print url('school/apply'); ?>"><?php print t('School of Workers'); ?></a></li>
          <li><a href="<?php print user_is_logged_in() ? url('admin/demo-campaign') : url('user/login', array('query' => array('destination' => 'admin/demo-campaign'))); ?>"><?php print t('Staff dashboard'); ?></a></li>
        </ul></div>
      </div>
      <div class="rb-fine">
        <p><?php print t('The Allied Metal Trades Union, Riverbend Lodge 412 and Harbor Fabrication are fictional. Design and development demo by David Bickley, built on Backdrop CMS.'); ?></p>
        <p><?php print t('Photos via Unsplash: Heber Davis, Cemrecan Yurtman, Spencer Davis, Rafael Juárez and sol.'); ?></p>
      </div>
    </div>
  </footer>
</div>
