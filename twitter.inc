<?php
/**
 * @file
 * @copyright The Royal National Theatre
 * @author John-Paul Drawneek <jdrawneek@nationaltheatre.org.uk>
 */

class NtFeedTwitter extends NtFeedData {
  
  protected $search   = "http://search.twitter.com/search.json?result_type=recent&rpp=100&q=";
  protected $timeline = "https://api.twitter.com/1.1/statuses/user_timeline.json?count=200&screen_name=";
  protected $tweet    = 'https://api.twitter.com/1.1/statuses/show/|id|.json';

  public function __construct($transport, $options) {
    $this->transport = $transport;
    $this->options = $options;
    if (isset($options['screen_name']) && strlen(trim($options['screen_name'])) > 0) {
      $this->label = trim($options['screen_name']);
      $this->title = 'Twitter - ' . trim($options['screen_name']);
      $this->id = md5(
        __CLASS__ . trim($options['screen_name'])
      );
      if (isset($options['filter']) && strlen(trim($options['filter'])) > 0) {
        $query = '';
        if (isset($options['screen_name']) && strlen(trim($options['screen_name'])) > 0) {
          $query .= 'from:' . $options['screen_name'] . ' ';
        }
        $query .= '#' . $options['filter'];
        $this->url = $this->search . urlencode($query) . '&src=hash';
      }
      else {
        $this->url = $this->timeline . urlencode(trim($options['screen_name']));
      }
    }
    elseif (strlen(trim($options['filter'])) > 0) {
      $this->label = trim($options['filter']);
      $this->title = 'Twitter - #' . trim($options['filter']);
      $this->id = md5(
        __CLASS__ . trim($options['filter']) . 'f'
      );
      $query = '#' . $options['filter'];
      $this->url = $this->search . urlencode($query) . '&src=hash';
    }
    
    $this->getItems($this->url);
  }

  public function filter($filter) {
    
  }

  public function paginate($number = 10) {
    
  }

  public function valid() {
    if (!isset($this->data[$this->pointer])) {
      $url = $this->url;
      if (count($this->data) > 0) {
        $url .= '&max_id=' . $this->lastPointer;
      }
      $this->getItems($url);
      if (!isset($this->data[$this->pointer])) {
        return FALSE;
      }
    }
    return TRUE;
  }

  protected function getItems($url) {
    $result = $this->transport->get(
      array(
        'url' => $url,
        'options' => array(
          'method' => 'GET',
        ),
      )
    );
    if ($result->code == 200) {
      $data = json_decode($result->data);
      if (is_object($data) && isset($data->results)) {
        $data = $data->results;
      }
      foreach ($data as $item) {
        if ($item->id == $this->lastPointer) {
          continue;
        }
//        die('<pre>' . print_r($item, TRUE) . '</pre>');
        $this->packItem($item, TRUE);
      }
      if (is_object($this->data[count($this->data)-1])) {
        $this->lastPointer = $this->data[count($this->data)-1]->id;
      }
      else {
        $this->lastPointer = $this->data[count($this->data)-1]['id'];
      }
      //die('<pre>' . print_r($result, TRUE) . '</pre>');
      return count($data) > 0;
    }
    else {
      // Log error message.
    }
    //die('<pre>' . print_r($result, TRUE) . '</pre>');
    return FALSE;
  }
  /**
   * Render the asset in format for the plane layout.
   *
   * @param string $id
   *   Asset id to be rendered.
   *
   * @return array
   *   Structure to be rendered.
   */
  public function render($id) {
    $output = parent::render($id);
    $output['data']['type']['#value']       = $this->getId();
    $output['data']['feed_type']['#value']  = 'Twitter';
    $output['data']['label']['#markup']     = 'Twitter - '
      . t($this->dataKey[$id]['description']);
    $output['data']['del']['#name']         = __CLASS__ . '_' . $id . '_del';
    $output['data']['del']['#data']['type'] = $this->getId();
    return $output;
  }
  /**
   * Get a Tweet by its ID.
   *
   * @param int $id
   *   Tweet id to get data for.
   *
   * @return mixed
   *   Array of data or false if it can't be found.
   */
  public function getItem($id) {
    if (isset($this->dataKey[$id])) {
      return $this->dataKey[$id];
    }
    $url = str_replace('|id|', $id, $this->tweet);
    $result = $this->transport->get(
      array(
        'url'     => _nt_twitter_wrap_oauth($url),
        'options' => array(
          'method' => 'GET',
        ),
      )
    );
    if ($result->code == 200) {
      $data = json_decode($result->data);
      $this->packItem($data);
      return $this->dataKey[$id];
    }
    else {
      return FALSE;
    }
  }
  
  protected function packItem($item, $inOrder = FALSE) {
    if (isset($item->user)) {
      $title = $item->user->name;
      $name = $item->user->name;
    }
    else {
      $title = $item->from_user_name;
      $name = $item->from_user_name;
    }
    $store = array(
      'raw_data' => $item,
      'title'    => $title,
      'date'     => strtotime($item->created_at),
      'publish'  => strtotime($item->created_at),
      'id'       => $item->id,
      'html_id'  => 'tweet_' . $item->id,
      'description' => drupal_substr($item->text, 0, 50),
      'url' => 'http://twitter.com/' . $name . '/status/' . $item->{'id_str'},
    );
    $this->dataKey[$store['id']] = $store;
    if ($inOrder) {
      $this->data[] = $this->dataKey[$store['id']];
    }
  }
  /**
   * This function gets the id value for a asset from a provided url.
   * 
   * @param string $url
   *   Absolute url to search for data.
   *
   * @return mixed
   *   The Id for the asset the url points to, or false if it can't find one.
   */
  public function getIdFromUrl($url) {
    if (strpos($url, 'twitter.com') !== FALSE) {
      $url_parts = parse_url($url);
      $path = explode('/', $url_parts['path']);
      if (count($path) == 4) {
        return array_pop($path);
      }
      else {
        drupal_set_message('Can\'t find this Tweet id.', 'error');
      }
    }
    else {
      drupal_set_message('Don\'t know this Tweet format.', 'error');
    }
    return FALSE;
  }
}
