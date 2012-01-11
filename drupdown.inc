<?php
class DrupdownParser extends MarkdownExtra_Parser {
  /**
   * Remove "doImages" to unify handling of external resources. 
   */
  function __construct() {
    $this->block_gamut['doEmbeds'] = 55;
    $this->span_gamut['doEmbeds'] = 0;
    $this->block_gamut['doCodeBlocks'] = 5;
    parent::__construct();
    unset($this->block_gamut['doFencedCodeBlocks']);
    unset($this->span_gamut['doImages']);
  }

	function doEmbeds($text) {

	#
	# Turn Markdown link shortcuts into XHTML <a> tags.
	#
		if ($this->in_anchor) return $text;
		$this->in_anchor = true;
		
		#
		# First, handle reference-style links: [link text] [id]
		#
		$text = preg_replace_callback('{
			(					# wrap whole match in $1
        ([<>!])
			  \[
				('.$this->nested_brackets_re.')	# link text = $3
			  \]

			  [ ]?				# one optional space
			  (?:\n[ ]*)?		# one optional newline followed by spaces

			  \[
				(.*?)		# id = $4
			  \]
			)
			}xs',
			array(&$this, '_doEmbeds_reference_callback'), $text);

		#
		# Next, inline-style links: [link text](url "optional title")
		#
		$text = preg_replace_callback('{
			(				# wrap whole match in $1
        ([<>!])
        \[
				('.$this->nested_brackets_re.')	# link text = $3
			  \]
			  \(			# literal paren
				[ \n]*
				(?:
					<(.+?)>	# href = $4
				|
					('.$this->nested_url_parenthesis_re.')	# href = $5
				)
				[ \n]*
				(			# $6
				  ([\'"])	# quote char = $7
				  (.*?)		# Title = $8
				  \7		# matching quote
				  [ \n]*	# ignore any spaces/tabs between closing quote and )
				)?			# title is optional
			  \)
			)
			}xs',
			array(&$this, '_doEmbeds_inline_callback'), $text);

		#
		# Last, handle reference-style shortcuts: [link text]
		# These must come last in case you've also got [link text][1]
		# or [link text](/foo)
		#
		$text = preg_replace_callback('{
			(					# wrap whole match in $1
        ([<>!])
			  \[
				([^\[\]]+)		# link text = $3; can\'t contain [ or ]
			  \]
			)
			}xs',
			array(&$this, '_doEmbeds_reference_callback'), $text);

		$this->in_anchor = false;
		return $text;
	}


	function _doEmbeds_reference_callback($matches) {
		$whole_match =  $matches[1];
    $modifier    =  $matches[2];
		$link_text   =  $matches[3];
		$link_id     =& $matches[4];

		if ($link_id == "") {
			# for shortcut links like [this][] or [this].
			$link_id = $link_text;
		}
		
		# lower-case and turn embedded newlines into spaces
		$link_id = strtolower($link_id);
		$link_id = preg_replace('{[ ]?\n}', ' ', $link_id);

		if (isset($this->urls[$link_id])) {
			$url = $this->urls[$link_id];
			$url = $this->encodeAttribute($url);
			
			if ( isset( $this->titles[$link_id] ) ) {
				$title = $this->titles[$link_id];
				$title = $this->encodeAttribute($title);
			}
		
			$link_text = $this->runSpanGamut($link_text);
      $result = drupdown_process_reference($link_text, $url, $title, $modifier);
			$result = $this->hashBlock($result);
		}
		else {
			$result = $whole_match;
		}
		return $result;
	}

	function _doEmbeds_inline_callback($matches) {
		$whole_match	=  $matches[1];
    $modifier    =  $matches[2];
		$link_text		=  $this->runSpanGamut($matches[3]);
		$url			=  $matches[4] == '' ? $matches[5] : $matches[4];
		$title			=& $matches[8];

		$url = $this->encodeAttribute($url);

		if (isset($title)) {
			$title = $this->encodeAttribute($title);
		}
		
		$link_text = $this->runSpanGamut($link_text);

    $result = drupdown_process_reference($link_text, $url, $title, $modifier);
		return $this->hashBlock($result);
	}

  /**
   * Remove h1, start at h2 instead.
   */
	function _doHeaders_callback_setext($matches) {
		# Terrible hack to check we haven't found an empty list item.
		if ($matches[2] == '-' && preg_match('{^-(?: |$)}', $matches[1]))
			return $matches[0];
		
		$level = $matches[3]{0} == '=' ? 2 : 3;
		$block = "<h$level>".$this->runSpanGamut($matches[1])."</h$level>";
		return "\n" . $this->hashBlock($block) . "\n\n";
	}
	function _doHeaders_callback_atx($matches) {
		$level = strlen($matches[1]) + 1;
		$block = "<h$level>".$this->runSpanGamut($matches[2])."</h$level>";
		return "\n" . $this->hashBlock($block) . "\n\n";
	}
  /**
   * Override list behavior. "+" creates ordered lists now.
   */
	function _doLists_callback($matches) {
		# Re-usable patterns to match list item bullets and number markers:
		$marker_ul_re  = '[*-]';
		$marker_ol_re  = '\d+[.]|\+';
		$marker_any_re = "(?:$marker_ul_re|$marker_ol_re)";
		
		$list = $matches[1];
		$list_type = preg_match("/$marker_ul_re/", $matches[4]) ? "ul" : "ol";
		
		$marker_any_re = ( $list_type == "ul" ? $marker_ul_re : $marker_ol_re );
		
		$list .= "\n";
		$result = $this->processListItems($list, $marker_any_re);
		
		$result = $this->hashBlock("<$list_type>\n" . $result . "</$list_type>");
		return "\n". $result ."\n\n";
	}

  /**
   * Every break should be a break.
   */
	function doHardBreaks($text) {
		# Do hard breaks:
		return preg_replace_callback('/\n/', 
			array(&$this, '_doHardBreaks_callback'), $text);
	}

  /**
   * Blockquotes: adds left/right/center classes to blockquotes.
   */
	function doBlockQuotes($text) {
		$text = preg_replace_callback('/
			  (								# Wrap whole match in $1
				(?>
				  ^[ ]*(<|>|\|)[ ]		# ">" at the start of a line
					.+\n					# rest of the first line
				  (.+\n)*					# subsequent consecutive lines
				  \n*						# blanks
				)+
			  )
			/xm',
			array(&$this, '_doBlockQuotes_callback'), $text);

		return $text;
	}

  /**
   * Blockquotes: adds left/right/center classes to blockquotes.
   */
	function _doBlockQuotes_callback($matches) {
		$bq = $matches[1];
    $mod = $matches[2];
		# trim one level of quoting - trim whitespace-only lines
		$bq = preg_replace('/^[ ]*(<|>|\|)[ ]|^[ ]+$/m', '', $bq);
		$bq = $this->runBlockGamut($bq);		# recurse

		$bq = preg_replace('/^/m', "  ", $bq);
		# These leading spaces cause problem with <pre> content, 
		# so we need to fix that:
		$bq = preg_replace_callback('{(\s*<pre>.+?</pre>)}sx', 
			array(&$this, '_doBlockQuotes_callback2'), $bq);

    $classes = array(
      '<' => 'drupdown-left',
      '>' => 'drupdown-right',
      '|' => 'drupdown-center',
    );
		return "\n". $this->hashBlock("<blockquote class=\"{$classes[$mod]}\">\n$bq\n</blockquote>")."\n\n";
	}

  /**
   * Fix alignment for tables.
   */
	function _doTable_callback($matches) {
		$head		= $matches[1];
		$underline	= $matches[2];
		$content	= $matches[3];

		# Remove any tailing pipes for each line.
		$head		= preg_replace('/[|] *$/m', '', $head);
		$underline	= preg_replace('/[|] *$/m', '', $underline);
		$content	= preg_replace('/[|] *$/m', '', $content);
		
		# Reading alignement from header underline.
		$separators	= preg_split('/ *[|] */', $underline);
		foreach ($separators as $n => $s) {
			if (preg_match('/^ *-+: *$/', $s))		$attr[$n] = ' class="drupdown-table-right"';
			else if (preg_match('/^ *:-+: *$/', $s))$attr[$n] = ' class="drupdown-table-center"';
			else if (preg_match('/^ *:-+ *$/', $s))	$attr[$n] = ' class="drupdown-table-left"';
			else									$attr[$n] = '';
		}
		
		# Parsing span elements, including code spans, character escapes, 
		# and inline HTML tags, so that pipes inside those gets ignored.
		$head		= $this->parseSpan($head);
		$headers	= preg_split('/ *[|] */', $head);
		$col_count	= count($headers);
		
		# Write column headers.
		$text = "<table>\n";
		$text .= "<thead>\n";
		$text .= "<tr>\n";
		foreach ($headers as $n => $header)
			$text .= "  <th$attr[$n]>".$this->runSpanGamut(trim($header))."</th>\n";
		$text .= "</tr>\n";
		$text .= "</thead>\n";
		
		# Split content by row.
		$rows = explode("\n", trim($content, "\n"));
		
		$text .= "<tbody>\n";
		foreach ($rows as $row) {
			# Parsing span elements, including code spans, character escapes, 
			# and inline HTML tags, so that pipes inside those gets ignored.
			$row = $this->parseSpan($row);
			
			# Split row by cell.
			$row_cells = preg_split('/ *[|] */', $row, $col_count);
			$row_cells = array_pad($row_cells, $col_count, '');
			
			$text .= "<tr>\n";
			foreach ($row_cells as $n => $cell)
				$text .= "  <td$attr[$n]>".$this->runSpanGamut(trim($cell))."</td>\n";
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
	#
	#	Process Markdown `<pre><code>` blocks.
	#
		$text = preg_replace_callback('{
				(?:```(.*?)\n)
				(	            # $1 = the code block -- one or more lines, starting with a space/tab
				  (?>
					.*\n+
				  )+?
				)
        (?:```\n)
			}xm',
			array(&$this, '_doCodeBlocks_callback'), $text);

		return $text;
	}
	function _doCodeBlocks_callback($matches) {
		$codeblock = $matches[2];

		$codeblock = htmlspecialchars($codeblock, ENT_NOQUOTES);

		$codeblock = "<pre><code class=\"drupdown-code\" data-language=\"{$matches[1]}\">$codeblock\n</code></pre>";
		return "\n\n".$this->hashBlock($codeblock)."\n\n";
	}
}
