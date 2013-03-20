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

function nt_twitter_get_oauth_data() {
  $output = array();
  if (!empty($_GET) && isset($_GET['url'])) {
    $key = variable_get('nt_twitter_oauth_key', '');
    $secret = variable_get('nt_twitter_oauth_secret', '');
    $consumer = new DrupalOAuthConsumer($key, $secret, array());
    $token = unserialize(variable_get('nt_twitter_oauth_token'));
    $url = 'https://api.twitter.com/1.1/statuses/' . $_GET['url']
      . '?' . urldecode($_GET['query']);
    $result = DrupalOAuthRequest::from_consumer_and_token($consumer, $token, 'GET', $url);
    $result->sign_request(new \OAuthSignatureMethod_HMAC_SHA1(), $consumer, $token);
    $twitter_raw = drupal_http_request($result->to_url(), array('method' => 'GET'));
    $output             = $twitter_raw->data;
    print $twitter_raw->data;
    $output['error']    = FALSE;
    $output['sign_url'] = $result->to_url();
  }
  else {
    $output['error']   = TRUE;
    $output['message'] = t('Not a post request.');
    drupal_json_output($output);
  }
}
