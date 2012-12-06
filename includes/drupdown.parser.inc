<?php
/**
 * @file
 * Extension of markdown extra parser.
 */

/**
 * Extend MarkdownExtra_parser to add additional features and fixes.
 */
class DrupdownParser extends MarkdownExtra_Parser {
  /**
   * Remove "doImages" to unify handling of external resources.
   */
  public function __construct() {
    $this->block_gamut['doEmbeds'] = 55;
    $this->block_gamut['doCodeBlocks'] = 5;
    $this->span_gamut['doInlineEmbeds'] = 55;
    parent::__construct();
    unset($this->span_gamut['doAnchors']);
    unset($this->block_gamut['doAnchors']);
    unset($this->span_gamut['doImages']);
    unset($this->block_gamut['doImages']);
  }

  /**
   * Process block level embeddables.
   */
  public function doEmbeds($text) {
    $pattern = implode('', array(
      '!?\[(?<alt>.*?)\]',
      '(',
        '(\(',
          '(?<url>.*?)',
          '(',
            '\s?',
            '\"(?<title>.*?)\"',
          ')?',
        '\))',
        '|',
        '(\[(?<ref>.*?)\])',
      ')',
    ));
    preg_match_all('{(' . $pattern . '\\n)+}x', $text, $matches);
    foreach ($matches[0] as $block) {
      preg_match_all('/' . $pattern . '/', $block, $m);
      $items = array();
      for ($i = 0; $i < count($m['alt']); $i++) {
        if (strlen($m['ref'][$i]) > 0) {
          $m['url'][$i] = $this->urls[$m['ref'][$i]];
          $m['title'][$i] = $this->titles[$m['ref'][$i]];
        }
        $items[] = drupdown_process_reference($m['alt'][$i], $m['url'][$i], $m['title'][$i]);
      }
      if (count($items) == 1) {
        $content = $items[0];
      }
      else {
        $content = '<ul class="drupdown-group">';
        foreach ($items as $item) {
          $content .= '<li>' . $item . '</li>';
        }
        $content .= '</ul>';
      }
      $text = str_replace($block, "\n" . $this->hashBlock($content) . "\n", $text);
    }
    return $text;
  }

  /**
   * Process span level embeddables (links).
   */
  public function doInlineEmbeds($text) {
    $pattern = implode('', array(
      '!?\[(?<alt>.*?)\]',
      '(',
        '(\(',
          '(?<url>.*?)',
          '(',
            '\s?',
            '\"(?<title>.*?)\"',
          ')?',
        '\))',
        '|',
        '(\[(?<ref>.*?)\])',
      ')',
    ));
    return preg_replace_callback('/' . $pattern . '/', array(&$this, 'inlineEmbedsCallback'), $text);
  }

  public function inlineEmbedsCallback($matches) {
    $info = drupdown_match_pattern($matches['url']);
    if (!is_array($info['classes'])) {
      $info['classes'] = array();
    }
    $link = l($matches['alt'], $matches['url'], array(
      'html' => TRUE,
      'attributes' => array(
        'class' => $info['classes'] + array('drupdown-' . $info['id']),
      ),
    ));
    return $this->hashPart($link);
  }

  /**
   * Remove h1, start at h2 instead.
   */
  public function _doHeaders_callback_setext($matches) {
    if ($matches[3] == '-' && preg_match('{^- }', $matches[1])) {
      return $matches[0];
    }
    $level = $matches[3][0] == '=' ? 2 : 3;
    $id =& $matches[2];
    $attr  = $this->_doHeaders_attr($id);
    $block = "<h$level$attr>" . $this->runSpanGamut($matches[1]) . "</h$level>";
    return "\n" . $this->hashBlock($block) . "\n\n";
  }

  /**
   * Callback for custom headers.
   */
  public function _doHeaders_callback_atx($matches) {
    $level = strlen($matches[1]) + 1;
    $id =& $matches[3];
    $attr  = $this->_doHeaders_attr($id);
    $block = "<h$level$attr>" . $this->runSpanGamut($matches[2]) . "</h$level>";
    return "\n" . $this->hashBlock($block) . "\n\n";
  }

  /**
   * Every break should be a break.
   */
  function doHardBreaks($text) {
    // Do hard breaks.
    return preg_replace_callback('/(?<=\s{2}|[.:;!?])\n/',
        array(&$this, '_doHardBreaks_callback'), $text);
  }

  /**
   * Fix alignment for tables.
   */
  function _doTable_callback($matches) {
    $head = $matches[1];
    $underline = $matches[2];
    $content = $matches[3];

    // Remove any tailing pipes for each line.
    $head = preg_replace('/[|] *$/m', '', $head);
    $underline = preg_replace('/[|] *$/m', '', $underline);
    $content = preg_replace('/[|] *$/m', '', $content);

    // Reading alignement from header underline.
    $separators = preg_split('/ *[|] */', $underline);
    $attr = array();
    foreach ($separators as $n => $s) {
      if (preg_match('/^ *-+: *$/', $s)) {
        $attr[$n] = ' class="drupdown-table-right"';
      }
      elseif (preg_match('/^ *:-+: *$/', $s)) {
        $attr[$n] = ' class="drupdown-table-center"';
      }
      elseif (preg_match('/^ *:-+ *$/', $s)) {
        $attr[$n] = ' class="drupdown-table-left"';
      }
      else {
        $attr[$n] = '';
      }
    }
    $head = $this->parseSpan($head);
    $headers = preg_split('/ *[|] */', $head);
    $col_count = count($headers);
    $text = "<table>\n";
    $text .= "<thead>\n";
    $text .= "<tr>\n";
    foreach ($headers as $n => $header) {
      $text .= "  <th{$attr[$n]}>" . $this->runSpanGamut(trim($header)) . "</th>\n";
    }
    $text .= "</tr>\n";
    $text .= "</thead>\n";

    $rows = explode("\n", trim($content, "\n"));

    $text .= "<tbody>\n";
    foreach ($rows as $row) {
      $row = $this->parseSpan($row);
      $row_cells = preg_split('/ *[|] */', $row, $col_count);
      $row_cells = array_pad($row_cells, $col_count, '');
      $text .= "<tr>\n";
      foreach ($row_cells as $n => $cell) {
        $text .= "  <td{$attr[$n]}>" . $this->runSpanGamut(trim($cell)) . "</td>\n";
      }
      $text .= "</tr>\n";
    }
    $text .= "</tbody>\n";
    $text .= "</table>";

    return $this->hashBlock($text) . "\n";
  }

  /**
   * Git style codeblocks.
   */
  function doCodeBlocks($text) {
    $pattern = implode('', array(
      '(```(.*?)\n((?>.*\n+)+?)```\n)',
      '|',
      '(\n\n',
        '(?>',
          '\s{2}.*\n+',
        ')+',
      ')',
    ));
    $text = preg_replace_callback('{' . $pattern . '}xm', array(&$this, '_doCodeBlocks_callback'), $text);
    return $text;
  }

  function _doCodeBlocks_callback($matches) {
    $codeblock = strlen($matches[3]) > 0 ? $matches[3] : $matches[4];
    if (strlen($matches[3]) == 0) {
      $codeblock = trim(preg_replace('/\n\s{2}/', "\n", $codeblock));
    }
    $codeblock = htmlspecialchars($codeblock, ENT_NOQUOTES);
    if (strlen($matches[2]) > 0) {
      $codeblock = '<code class="drupdown-code" data-language="' . $matches[2] . '">'
        . $codeblock
        . '</code>';
    }
    $codeblock = "<pre>$codeblock</pre>";
    return "\n\n" . $this->hashBlock($codeblock) . "\n\n";
  }
}
