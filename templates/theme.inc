<?php
/**
 * @file
 * @copyright The Royal National Theatre
 * @author John-Paul Drawneek <jdrawneek@nationaltheatre.org.uk>
 */

/**
 * Theme for tweets table for picking.
 *
 * @param array $variables
 *   List of variables to use.
 *
 * @return string
 *   Html to be rendered.
 */
function theme_nt_twitter_table(&$variables) {
  $rows = array();
  if (isset($variables['element']['#data']) && !empty($variables['element']['#data'])) {
    $data = $variables['element']['#data']['data'];
    foreach ($variables['element']['#data']['ordering'] as $key => $item) {
      if ($key % 2 == 1) {
        $class = 'odd';
      }
      else {
        $class = 'even';
      }
      unset($variables['element']['select_tweet'][$item]['#title']);
      $rows[] = array(
        'data' => array(
          $data[$item]['user'],
          $data[$item]['tweet'],
          $data[$item]['date'],
          drupal_render($variables['element']['select_tweet'][$item]),
        ),
        'class' => array($class),
      );
    }
  }
  $header = array(t('User'), t('Tweet'), t('date'), t('select'));
  $output = theme(
    'table',
    array(
      'header' => $header,
      'rows' => $rows,
      'attributes' => array('id' => 'nt-tweet-table'),
    )
  );
  return drupal_render($variables['element']['get_list_tweets']) . $output;
}
