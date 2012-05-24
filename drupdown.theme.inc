<?php
/**
 * @file
 *   Drupdown theme implementations.
 */


function theme_drupdown_figure($variables) {
  $variables['classes'][] = 'drupdown-figure';
  $caption = FALSE;
  if (strlen($variables['caption']) > 0) {
    $caption = $variables['caption'];
    $variables['classes'][] = 'drupdown-captioned';
  }
  $content = '<div class="drupdown-figure-wrapper drupdown-' . $variables['float'] . '">'
    .'<div class="' . implode(' ', $variables['classes']) . '">'
    .'<div class="drupdown-figure-content">' . $variables['content'] . '</div>';
  if ($caption) {
    $content .= '<div class="drupdown-caption">' . $caption . '</div>';
  }
  $content .= '</div></div>';
  return $content;
}

function theme_drupdown_link($variables) {
  return '';
}