<?php
/**
 * @file
 * Mail system that captures messages in a table instead of sending them.
 */

/**
 * Writes outgoing mail to {demo_campaign_outbox} so the demo can show it.
 */
class DemoCampaignOutboxMail implements MailSystemInterface {

  /**
   * {@inheritdoc}
   */
  public function format(array $message) {
    $message['body'] = implode("\n\n", $message['body']);
    $message['body'] = backdrop_wrap_mail($message['body']);
    return $message;
  }

  /**
   * {@inheritdoc}
   */
  public function mail(array $message) {
    db_insert('demo_campaign_outbox')
      ->fields(array(
        'mail_key' => $message['key'],
        'recipient' => $message['to'],
        'subject' => $message['subject'],
        'body' => $message['body'],
        'created' => REQUEST_TIME,
      ))
      ->execute();
    return TRUE;
  }

}
