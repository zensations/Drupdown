<?php
/**
 * @file
 *   Drupdown parser implementation.
 */

/**
 * Retrieve all defined block patterns.
 */
function drupdown_block_patterns() {
  if (FALSE && $cache = cache_get('drupdown_block_patterns')) {
    return $cache->data;
  }
  $patterns = module_invoke_all('drupdown_block_patterns');
  uasort($patterns, 'drupal_sort_weight');
  cache_set('drupdown_block_patterns', $patterns);
  return $patterns;
}

/**
 * Retrieve all defined span patterns.
 */
function drupdown_span_patterns() {
  if (FALSE && $cache = cache_get('drupdown_span_patterns')) {
    return $cache->data;
  }
  $patterns = module_invoke_all('drupdown_span_patterns');
  uasort($patterns, 'drupal_sort_weight');
  cache_set('drupdown_span_patterns', $patterns);
  return $patterns;
}

/**
 * Main Parser class
 */
class DrupdownParser {

  private $settings;

  private $context;
  private $hashes;

  private $block_patterns;
  private $span_patterns;

  public function __construct($settings = FALSE) {
    $this->block_patterns = drupdown_block_patterns();
    $this->span_patterns = drupdown_span_patterns();
    $this->context = new stdClass();
    $this->settings = $settings;
  }

  function parse($text) {
    foreach ($this->block_patterns as $pattern) {
      $block = array();
      $block_start = 0;
      $offset = 0;
      $diff = 0;
      $preg_flag = PREG_SET_ORDER | PREG_OFFSET_CAPTURE;
      if (preg_match_all($pattern['pattern'], $text, $matches, $preg_flag)) {
        foreach ($matches as $match) {
          // enter a new block
          if ($offset < $match[0][1]) {
            dpm($block);
            $block = array();
            $offset = $match[0][1];
            $block_start = $offset;
          }
          $offset = $offset + strlen($match[0][0]) + 1;
          $block[] = $this->_simplifyMatch($match);
        }
        dpm($block);
      }
    }
    return $text;
  }

  function _simplifyMatch($match) {
    return $match;
  }

  function spans($text) {

  }
}
