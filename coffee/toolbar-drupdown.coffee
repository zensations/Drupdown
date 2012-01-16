$ = jQuery
define 'ace/toolbar/drupdown', ['require', 'exports', 'module'], (require, exports, module) ->
  Range = require("ace/range").Range;
  class DrupdownToolbar
    constructor: (@element, @editor, @field_name, @format) ->

    header: (level) ->
      session = @editor.getSession()
      pos = @editor.getCursorPosition().row

      from = if pos > 0 then pos - 1 else pos
      to = if session.doc.getAllLines().length > (pos + 1) then pos + 1 else pos
      row = if from < pos then 1 else 0
      lines = if from is to then [session.getLine(from)] else session.getLines(from, to)
      to_column = lines[lines.length - 1].length

      if lines[row].match /^[=-]{3}/
        row--

      if not $.trim(lines[row]).length
        lines[row] = Drupal.t('Insert headline ...')

      if row > 0 and $.trim(lines[row-1]).length
        lines.splice(row++, 0, '')

      if row < lines.length - 1 and $.trim(lines[row + 1]).length
        if lines[row+1].match /^[=-]{3}/
          lines.splice(row + 1, 1)
        else
          lines.splice(row + 1, 0, '')

      lines[row] = lines[row].replace(/^#+[ ]/, '')

      switch level
        when 1, 2
          underline = lines[row].replace(/./g, if (level - 1) then '-' else '=')
          lines.splice(row + 1, 0, underline)
        else
          signs = ''
          signs += '#' while --level
          lines[row] = signs + ' ' + lines[row]
      session.replace(new Range(from, 0, to, to_column), lines.join('\n'))

    emphasize: (level) ->
      session = @editor.getSession()
      signs = ''
      signs += '*' for i in [1..level]
      range = @editor.getSelectionRange()

      if range.start.column == range.end.column and range.start.row == range.end.row
        return

      session.insert(range.start, signs)
      range.end.column += level
      session.insert(range.end, signs)
      @editor.clearSelection()

    prefixLines: (sign) ->
      session = @editor.getSession()
      range = @editor.getSelectionRange()
      lines = session.getLines(range.start.row, range.end.row)
      to = lines[lines.length - 1].length
      lines[i] = "#{sign} #{lines[i]}" for i in [0...lines.length]
      session.replace(new Range(range.start.row, 0, range.end.row, to), lines.join('\n'))
      @editor.clearSelection()

    floatOptions: ->
      return '''
        <div class="drupdown-float-options clearfix">
          <div class="column">
            <div class="icon-float-left">Left</div>
            <input type="radio" name="position" value="<"/>
          </div>
          <div class="column">
            <div class="icon-float-center">Center</div>
            <input type="radio" name="position" value="!" checked/>
          </div>
          <div class="column">
            <div class="icon-float-right">Right</div>
            <input type="radio" name="position" value=">"/>
          </div>
        </div>
        '''
    processQuote: ->

    render: ->
      # HEADING BUTTONS
      headings = $('<span></span>')
      headingbutton = (i) =>
        button = $("<button>H#{i}</button>").button().click =>
          @header(i)
          return false
      headings.append(headingbutton(i)) for i in [1..5]
      headings.buttonset().appendTo(@element)

      # BOLD AND ITALIC
      styles = $('<span></span>')
      $('<button><span style="font-weight:bold">B</span></button>').button().click =>
        @emphasize(2)
        return false
      .appendTo(styles)

      $('<button><span style="font-style:italic">I</span></button>').button().click =>
        @emphasize(1)
        return false
      .appendTo(styles)
      styles.buttonset().appendTo(@element)

      # LISTS
      lists = $('<span></span>')
      $('<button>ul</button>').button({text:false,icons:{primary:'ui-icon-bullet'}}).click =>
        @prefixLines('-')
        return false;
      .appendTo(lists)
      $('<button>ol</button>').button({text:false,icons:{primary:'ui-icon-check'}}).click =>
        @prefixLines('+')
        return false;
      .appendTo(lists)
      lists.buttonset().appendTo(@element)

      # QUOTE
      blocks = $('<span></span>')
      $('<button>quote</button>').button({text:false,icons:{primary:'ui-icon-comment'}}).click( =>
        dialog = $("<div title=\"#{Drupal.t('Choose position')}\">#{@floatOptions()}</div>")
        $('.drupdown-float-options .column', dialog).click ->
          $('input', this).attr('checked', 'checked')
        dialog.dialog
          modal: true
          show: 'fade'
          hide: 'fade'
          buttons:
            'OK': =>
              sign = $('input[name=position]:checked', dialog).val()
              @prefixLines(sign)
              dialog.dialog('close')
        return false
      ).appendTo(blocks)

      # EXTERNALS
      $('<button>link</button>').button({text:false,icons:{primary:'ui-icon-link'}}).click =>
        range = @editor.getSelectionRange()
        text = @editor.getSession().doc.getTextRange(range)
        dialog = $("""
          <div title="#{Drupal.t('Insert Link')}" class="drupdown-dialog">
           ')label for="text">#{Drupal.t('Link text')}</label>
            <input type="text" name="text" class="ui-widget-content ui-corner-all" value="#{text}"/>
            <label for="title">#{Drupal.t('Link title')}</label>
            <input type="text" name="title" class="ui-widget-content ui-corner-all" value="#{text}"/>
            <label for="uri">#{Drupal.t('Web address')}</label>
            <input type="text" name="uri" class="ui-widget-content ui-corner-all" value=""/>
          </div>
        """).dialog
          modal: true
          show: 'fade'
          hide: 'fade'
          buttons:
            'OK': =>
              text = $('input[name=text]', dialog).val()
              title = $('input[name=title]', dialog).val()
              uri = $('input[name=uri]', dialog).val()
              link = "[#{text}](#{uri} \"#{title}\")"
              @editor.getSession().replace(range, link)
              dialog.dialog('close')
        return false;
      .appendTo(blocks)

      $('<button>embed</button>').button({text:false,icons:{primary:'ui-icon-image'}}).click =>
        range = @editor.getSelectionRange()
        text = @editor.getSession().doc.getTextRange(range)
        dialog = $("""
          <div title="#{Drupal.t('Insert Resource')}" class="drupdown-dialog">
            #{@floatOptions()}
            <label for="text">#{Drupal.t('Alternative text')}</label>
            <input type="text" name="text" class="ui-widget-content ui-corner-all" value="#{text}"/>
            <label for="title">#{Drupal.t('Caption')}</label>
            <input type="text" name="title" class="ui-widget-content ui-corner-all" value="#{text}"/>
            <label for="uri">#{Drupal.t('Web address')}</label>
            <div class="drupdown-resource">
              <input type="text" name="uri" class="uri ui-widget-content ui-corner-all" value=""/>
              <button class="drupdown-resource-choose">Choose</button>
            </div>
          </div>
        """)
        $('.drupdown-float-options .column', dialog).click ->
          $('input', this).attr('checked', 'checked')
        files = []
        formats = Drupal.settings.drupdown.styles[@field_name][@format]
        for file in ($(file).val() for file in $('.drupdown-resource'))
          if file.match /^original:\/\//
            for style in formats
              files.push file.replace /^original:\/\//, style + '://'
          else
            files.push file

        input = $('input[name=uri]', dialog)
        $(input).autocomplete({source: files, minlength:0, delay:0})
        $('.drupdown-resource-choose', dialog).button({text:false,icons:{primary:'ui-icon-search'}}).click ->
          $(input).autocomplete('search', ':')
          $(this).blur()
          $(input).focus()
          return false

        dialog.dialog
          modal: true
          show: 'fade'
          hide: 'fade'
          buttons:
            'OK': =>
              sign = $('input[name=position]:checked', dialog).val()
              sign = '!' if sign is '|'
              text = $('input[name=text]', dialog).val()
              title = $('input[name=title]', dialog).val()
              uri = $('input[name=uri]', dialog).val()
              link = "#{sign}[#{text}](#{uri} \"#{title}\")"
              @editor.getSession().replace(range, link)
              dialog.dialog('close')
        return false;
      .appendTo(blocks)
      blocks.buttonset().appendTo(@element)

  exports.Toolbar = DrupdownToolbar
  return
