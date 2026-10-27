<?php
/**
 * @file
 * Backdrop settings, read from environment variables.
 */

$database = array(
  'database' => getenv('DB_NAME'),
  'username' => getenv('DB_USER'),
  'password' => getenv('DB_PASS'),
  'host' => getenv('DB_HOST'),
  'port' => '3306',
  'driver' => 'mysql',
  'prefix' => '',
);

$config_directories['active'] = 'files/config/active';
$config_directories['staging'] = 'files/config/staging';

$settings['hash_salt'] = getenv('HASH_SALT');
$settings['update_free_access'] = FALSE;

// The site's own hostname (SITE_HOST) plus loopback, which the CRM stub uses.
$settings['trusted_host_patterns'] = array('^127\.0\.0\.1(:\d+)?$', '^localhost(:\d+)?$');
if (getenv('SITE_HOST')) {
  $settings['trusted_host_patterns'][] = '^' . preg_quote(getenv('SITE_HOST')) . '(:\d+)?$';
}

// Behind a TLS-terminating proxy (Traefik on Coolify), trust its scheme header
// so Backdrop builds https:// links.
if (isset($_SERVER['HTTP_X_FORWARDED_PROTO']) && $_SERVER['HTTP_X_FORWARDED_PROTO'] === 'https') {
  $_SERVER['HTTPS'] = 'on';
}

// Trust X-Forwarded-For only from a proxy on the private Docker network, so
// ip_address() returns the visitor's IP. Without this every visitor shares the
// proxy's address, and per-IP flood limits (card form, login) apply to everyone at once.
if (!empty($_SERVER['REMOTE_ADDR']) && preg_match('/^(10\.|172\.(1[6-9]|2\d|3[01])\.|192\.168\.)/', $_SERVER['REMOTE_ADDR'])) {
  $settings['reverse_proxy'] = TRUE;
  $settings['reverse_proxy_addresses'] = array($_SERVER['REMOTE_ADDR']);
}

$settings['backdrop_drupal_compatibility'] = TRUE;
ini_set('session.gc_probability', 1);
ini_set('session.gc_divisor', 100);
ini_set('session.gc_maxlifetime', 200000);
ini_set('session.cookie_lifetime', 2000000);
