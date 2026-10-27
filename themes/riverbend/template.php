<?php
/**
 * @file
 * Riverbend theme functions.
 */

/**
 * Implements hook_preprocess_page().
 */
function riverbend_preprocess_page(&$variables) {
  backdrop_add_html_head_link(array('rel' => 'preconnect', 'href' => 'https://fonts.gstatic.com', 'crossorigin' => 'anonymous'));
  backdrop_add_css('https://fonts.googleapis.com/css2?family=Barlow+Condensed:wght@600;700;800&family=Barlow:wght@400;500;600&display=swap', array('type' => 'external'));
}

/**
 * Implements hook_preprocess_layout().
 *
 * Pages that draw their own hero heading skip the page-title band.
 */
function riverbend_preprocess_layout(&$variables) {
  $variables['riverbend_hide_title'] = backdrop_is_front_page()
    || (arg(0) == 'organize' && arg(2) === NULL)
    || (arg(0) == 'school' && arg(1) == 'apply');
}
