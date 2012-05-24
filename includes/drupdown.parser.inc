<?php
function drupdown_block_elements() {
  if ($cache = cache_get('drupdown_block_elements')) {
    return $cache->data;
  }
  $elements = module_invoke_all('drupdown_block_elements');
  $elements['paragraph'] = array(
    'label' => t('Paragraph'),
    'pattern' => '/^(.+)$/',
    'process' => '_drupdown_paragraph',
    'weight' => 100,
  );
  $elements['heading'] = array(
    'label' => t('Heading'),
    'pattern' => '/^(#{1,6})\ (.*)$/',
    'process' => '_drupdown_heading',
    'weight' => 50,
  );
  uasort($elements, 'drupal_sort_weight');
  cache_set('drupdown_block_elements', $elements);
  return $elements;
}

function drupdown_span_elements() {
  if ($cache = cache_get('drupdown_span_elements')) {
    return $cache->data;
  }
  $elements = module_invoke_all('drupdown_span_elements');
  $elements['emphasized'] = array(
    'label' => t('Emphasized'),
    'pattern' => '/\_(.*?)\_/',
    'process' => '_drupdown_emphasized',
    'weight' => 50,
  );
  $elements['strong'] = array(
    'label' => t('Emphasized'),
    'pattern' => '/\*(.*?)\*/',
    'process' => '_drupdown_strong',
    'weight' => 50,
  );
  uasort($elements, 'drupal_sort_weight');
  cache_set('drupdown_span_elements', $elements);
  return $elements;
}

function drupdown_process_blocks($input, $settings) {
  $result = '';
  $lines = explode("\n", $input);
  $current = '_drupdown_paragraph';
  $block_elements = drupdown_block_elements();
  $block = array();
  foreach ($lines as $line) {
    if ($line == '') {
      if (count($block) > 0) {
        $result .= call_user_func($current, $block, $settings);
        $block = array();
      }
      continue;
    }
    foreach ($block_elements as $element) {
      if (preg_match($element['pattern'], $line, $matches)) {
        if ($current != $element['process']) {
          if (count($block) > 0) {
            $result .= call_user_func($current, $block, $settings);
            $block = array();
          }
        }
        $current = $element['process'];
        $block[] = $matches;
        break;
      }
    }
  }
  if (count($block) > 0) {
    $result .= call_user_func($current, $block, $settings);
  }
  return $result;
}

function drupdown_process_spans($input, $settings) {
  $span_elements = drupdown_span_elements();
  foreach ($span_elements as $element) {
    if (preg_match_all($element['pattern'], $input, $matches, PREG_SET_ORDER)) {
      foreach ($matches as $match) {
        $replace = call_user_func($element['process'], $match, $settings);
        $input = str_replace($match[0], $replace, $input);
      }
    }
  }
  return $input;
}

// ======================================================================
// BLOCK PROCESSORS
// ======================================================================
function _drupdown_paragraph($lines, $settings) {
  $paragraph = array();
  foreach ($lines as $line) {
    $paragraph[] = drupdown_process_spans($line[1], $settings);
  }
  return '<p>' . implode('<br/>', $paragraph) . '</p>';
}

function _drupdown_heading($lines, $settings) {
  $content = '';
  foreach ($lines as $line) {
    $level = strlen($line[1]);
    $text = drupdown_process_spans($line[2], $settings);
    $content .= "<h$level>$text</h$level>";
  }
  return $content;
}

// ======================================================================
// SPAN PROCESSORS
// ======================================================================
function _drupdown_emphasized($match, $settings) {
  return '<em>' . $match[1] . '</em>';
}
function _drupdown_strong($match, $settings) {
  return '<strong>' . $match[1] . '</strong>';
}

// ======================================================================
// EMBEDDED REFERENCES
// ======================================================================
function drupdown_process_reference($text, $url, $title, $modifier, $settings) {
  // remove leading slashes
  $url = preg_replace('/^\//', '', $url);
  $floatmap = array(
    '!' => 'center',
    '>' => 'right',
    '<' => 'left',
  );
  $patterns = drupdown_get_url_patterns();
  foreach ($patterns as $pattern => $callback) {
    if (preg_match($pattern, $url, $matches)) {
      if (array_key_exists('file', $callback)) {
        include_once $callback['file'];
      }
      $result = call_user_func_array($callback['function'], array($text, $url, $settings, $matches));
      if (strlen($result) > 0) {
        return theme('figure', array(
          'content' => $result,
          'caption' => $title,
          'float' => $floatmap[$modifier],
          'classes' => array_key_exists('classes', $callback)?$callback['classes']:array(),
        ));
        return $result;
      }
    }
  }
}

function drupdown_get_url_patterns() {
  if ($cache = cache_get('drupdown_url_patterns')) {
    return $cache->data;
  }
  $patterns = module_invoke_all('drupdown_url_patterns');
  $patterns['/.*/'] = array(
    'function' => 'drupdown_link_callback',
    'weight' => 100,
    'classes' => array('drupdown-link'),
  );
  drupal_alter('drupdown_url_patterns', $patterns);
  uasort($patterns, 'drupal_sort_weight');
  cache_set('drupdown_url_patterns', $patterns);
  return $patterns;
}