<?php

/**
 * Page callback for a simple playground form for trying out drupdown.
 */
function drupdown_playground($form, &$form_state) {
  $content = drupdown_to_html(@$form_state['values']['input'], array('styles' => array('original' => 'original', 'thumbnail' => 'thumbnail')));
  $form = array(
    'input' => array(
      '#type' => 'textarea',
      '#ace' => TRUE,
      '#ace_format' => 'drupdown',
    ),
    'transform' => array(
      '#type' => 'button',
      '#value' => t('Transform'),
      '#ajax' => array(
        'wrapper' => 'drupdown-playground-output',
        'callback' => 'drupdown_playground_callback',
        'effect' => 'fade',
      ),
    ),
    'output' => array(
      '#prefix' => '<div id="drupdown-playground-output">',
      '#suffix' => '</div>',
      'formatted' => array(
        '#prefix' => '<div class="formatted-output clearfix">',
        '#suffix' => '</div>',
        '#markup' => $content,
      ),
      'raw' => array(
        '#prefix' => '<pre class="raw-output">',
        '#suffix' => '</pre>',
        '#markup' => htmlentities($content),
      ),
    ),
  );
  return $form;
}

/**
 * Ajax callback for the playground form.
 */
function drupdown_playground_callback($form, $form_state) {
  return $form['output'];
}

