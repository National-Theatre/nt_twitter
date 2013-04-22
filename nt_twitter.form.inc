<?php
/**
 * @file
 * @copyright The Royal National Theatre
 * @author John-Paul Drawneek <jdrawneek@nationaltheatre.org.uk>
 */

/**
 * Config for for accessing the Twitter OAuth service.
 *
 * @param array $form
 *   Current from structure.
 * @param array $form_state
 *   Current form structure state.
 *
 * @return array
 *   Form structure to be rendered.
 */
function nt_twitter_admin_config($form, $form_state) {
  $form['security'] = array(
    '#type'        => 'fieldset',
    '#title'       => t('CMS service security'),
    '#description' => 'ToDo',
    'token'        => array(
      '#type'   => 'item',
      '#title'  => t('Token'),
      '#markup' => unserialize(variable_get('nt_twitter_oauth_token', 'none')),
    ),
    'nt_twitter_oauth_key' => array(
      '#type' => 'textfield',
      '#title' => t('CMS Key'),
      '#default_value' => variable_get('nt_twitter_oauth_key', ''),
    ),
    'nt_twitter_oauth_secret' => array(
      '#type' => 'textfield',
      '#title' => t('CMS Secret'),
      '#default_value' => variable_get('nt_twitter_oauth_secret', ''),
    ),
    'get_token' => array(
      '#type'   => 'submit',
      '#value'  => t('Get token'),
      '#submit' => array('nt_twitter_config_get_token'),
    ),
  );
  return system_settings_form($form);
}
/**
 * Callback to get capture the access token when returned from Twitter.
 * @return string
 *   Token passed back.
 */
function nt_twitter_access_callback() {
  $key = variable_get('nt_twitter_oauth_key', '');
  $secret = variable_get('nt_twitter_oauth_secret', '');
  $consumer = new DrupalOAuthConsumer($key, $secret, array());
  $token = unserialize(variable_get('nt_twitter_rtoken'));
  $client = new \DrupalOAuthClient($consumer, $token);
  $endpoint = 'https://api.twitter.com/oauth/access_token';
  $access_token = $client->getAccessToken($endpoint);
  variable_set('nt_twitter_oauth_token', serialize($access_token));
  drupal_set_message(t('Access token saved.'));
  return '<pre>' . print_r($access_token, TRUE) . '</pre>';
}
/**
 * Callback to get twitter data thats behind REST 1.1 API OAuth.
 */
function nt_twitter_get_oauth_data() {
  $output = array();
  if (!empty($_GET) && isset($_GET['url'])) {
    $url = 'https://api.twitter.com/1.1/statuses/' . $_GET['url']
      . '?' . urldecode($_GET['query']);
    $signed_url  = _nt_twitter_wrap_oauth($url);
    $twitter_raw = drupal_http_request($signed_url, array('method' => 'GET'));
    $last_tweet = variable_get('nt_twitter_last_tweet', array());
    if ($twitter_raw->code == 200) {
      $data_list = json_decode($twitter_raw->data);
      $tweet_data = FALSE;
      parse_str(urldecode($_GET['query']), $args);
      if (!isset($args['direct']) || $args['direct'] == 0) {
        foreach ($data_list as $data) {
          if (strpos($data->text, '@') === FALSE || strpos($data->text, '@') > 0) {
            $tweet_data = $data;
            break;
          }
        }
      }
      else {
        $tweet_data = array_shift($data_list);
      }
      if ($tweet_data) {
        $output = json_encode(array($tweet_data));
        print $output;
        variable_set('ntshed_fp_last_tweet', $output);
      }
      else {
        print $last_tweet;
      }
    }
    else {
      if (isset($last_tweet[$_GET['query']])) {
        print $last_tweet[$_GET['query']];
      }
      else {
        $output['error']   = TRUE;
        $output['message'] = t('Twitter API Error, check status.');
        drupal_json_output($output);
      }
    }
  }
  else {
    $output['error']   = TRUE;
    $output['message'] = t('Not a post request.');
    drupal_json_output($output);
  }
}
