<?php
/**
 * @file
 * Url pattern callbacks for drupdown.
 */

function drupdown_link_callback($text, $url, $matches) {
  if (file_uri_scheme($url) == 'public') {
    $url = file_create_url($url);
  }
  return l($text, $url);
}

function drupdown_image_callback($text, $url, $matches, &$classes) {
  $protocol = $matches[1];
  if ($protocol == 'original') {
    $protocol = 'public';
    $url = str_replace('original://', 'public://', $url);
  }
  $zoom = FALSE;
  $original_url = FALSE;
  if (!in_array($protocol, array('http', 'https', 'public', 'private'))) {
    if (in_array($protocol, array_keys(image_styles()))) {
      $pub = str_replace($protocol . '://', 'public://', $url);
      $style_url = image_style_url($protocol, $pub);
      $style_path = drupal_realpath(image_style_path($protocol, $pub));
      $original_path = drupal_realpath($pub);
      $image_styles = image_styles();
      if (!file_exists($style_path)) {
        image_style_create_derivative($image_styles[$protocol], $original_path, $style_path);
      }
      $classes[] = 'drupdown-imagestyle-' . $protocol;
      $style_info = image_get_info($style_path);
      $original_info = image_get_info($original_path);
      $zoom = $style_info['width']*$style_info['height'] < ($original_info['width']*$original_info['height'])/1.3;
      $original_url = file_create_url($pub);
      $url = $style_url;
    }
    else {
      return FALSE;
    }
  }

  $img = theme('image', array(
    'path' => $url,
    'title' => $text,
    'alt' => $text,
  ));

  if ($zoom) {
    $img = l($img, $original_url, array(
      'html' => TRUE,
      'attributes' => array(
        'class' => 'drupdown-zoom',
        'title' => $text,
      ),
    ));
  }

  return $img;
}

function drupdown_flickr_gallery_callback($text, $url, $matches) {
  $api_key = variable_get('drupdown_flickr_api_key', FALSE);
  if (!$api_key) {
    return "<p>" . t('No flickr api key set.') . "</p>";
  }
  $user = $matches[1];
  $set = $matches[2];
  $url = "http://api.flickr.com/services/rest/?method=flickr.photosets.getPhotos&api_key=$api_key&photoset_id=$set&format=php_serial";
  $result = drupal_http_request($url);
  $images = array();
  if ($result->data) {
    $set = unserialize($result->data);
    foreach ($set['photoset']['photo'] as $photo) {
      $thumb = "http://farm{$photo['farm']}.staticflickr.com/{$photo['server']}/{$photo['id']}_{$photo['secret']}_s.jpg";
      $large = "http://farm{$photo['farm']}.staticflickr.com/{$photo['server']}/{$photo['id']}_{$photo['secret']}_c.jpg";
      $images[] = l(theme('image', array(
        'path' => $thumb,
        'title' => $photo['title'],
      )), $large, array(
        'attributes' => array(
          'class' => array('drupdown-zoom'),
          'rel' => $set['photoset']['id'],
          'title' => $photo['title'],
        ),
        'html' => TRUE,
      ));
    }
    return theme('item_list', array(
      'attributes' => array(
        'class' => 'drupdown-flickr-album',
      ),
      'items' => $images,
    ));
  }
  else {
    return "<p>Empty photoset</p>";
  }
}


function drupdown_picasa_gallery_callback($text, $url, $matches) {
  $user_id = $matches[1];
  $album = $matches[2];
  $url = 'https://picasaweb.google.com/data/feed/api/user/' . $user_id . '/album/' . $album . '?alt=json';
  $result = drupal_http_request($url);
  $data = drupal_json_decode($result->data);
  if (!$data) {
    return '';
  }
  $list = array();
  foreach ($data['feed']['entry'] as $img) {
    $list[] = l(theme('image', array(
      'path' => $img['media$group']['media$thumbnail'][0]['url'],
      'title' => $img['title']['$t'],
    )), $img['content']['src'], array(
      'attributes' => array(
        'class' => array('drupdown-zoom'),
        'rel' => $img['gphoto$albumid']['$t'],
        'title' => $img['title']['$t'],
      ),
      'html' => TRUE,
    ));
  }
  return theme('item_list', array(
    'items' => $list,
    'attributes' => array(
      'class' => 'drupdown-picasa-album',
    ),
  ));
}

function drupdown_youtube_callback($text, $url, $matches) {
  return '<iframe
    class="drupdown-youtube"
    src="https://www.youtube-nocookie.com/embed/' . $matches[1] . '"></iframe>';
}

function drupdown_vimeo_callback($text, $url, $matches) {
  return '<iframe
    class="drupdown-vimeo"
    src="http://player.vimeo.com/video/' . $matches[1] . '?title=0&amp;byline=0&amp;portrait=0"></iframe>';
}

function drupdown_soundcloud_callback($text, $url, $matches) {
  $client_id = variable_get('drupdown_soundcloud_client_id');
  if (!$client_id) {
    return '<p>No Soundcloud client id configured. </p>';
  }
  $result = drupal_json_decode(drupal_http_request('http://api.soundcloud.com/resolve.json?client_id=' . $client_id . '&url=' . $url)->data);
  return '<iframe width="100%" height="166" scrolling="no" frameborder="no" src="http://w.soundcloud.com/player/?url=http%3A%2F%2Fapi.soundcloud.com%2Ftracks%2F' . $result['id'] . '&amp;auto_play=false&amp;show_artwork=false&amp;color=ff7700"></iframe>';
}
