(function($) {
  define('drupdown/toolbar', ['exports'], function(require, exports) {
    var Toolbar = function() {};
    require('ace/lib/oop').inherits(Toolbar, require('drupal/ace/toolbar/default').Toolbar);
    (function() {
      var Range = require("ace/range").Range;
      this.header = function(level) {
        var from, lines, pos, row, session, signs, to, to_column, underline;
        session = this.editor.getSession();
        pos = this.editor.getCursorPosition().row;
        from = pos > 0 ? pos - 1 : pos;
        to = session.doc.getAllLines().length > (pos + 1) ? pos + 1 : pos;
        row = from < pos ? 1 : 0;
        lines = from === to ? [session.getLine(from)] : session.getLines(from, to);
        to_column = lines[lines.length - 1].length;
        if (lines[row].match(/^[=-]{3}/)) row--;
        if (!$.trim(lines[row]).length) {
          lines[row] = Drupal.t('Insert headline ...');
        }
        if (row > 0 && $.trim(lines[row - 1]).length) lines.splice(row++, 0, '');
        if (row < lines.length - 1 && $.trim(lines[row + 1]).length) {
          if (lines[row + 1].match(/^[=-]{3}/)) {
            lines.splice(row + 1, 1);
          } else {
            lines.splice(row + 1, 0, '');
          }
        }
        lines[row] = lines[row].replace(/^#+[ ]/, '');
        signs = '';
        while (--level >= 0) {
          signs += '#';
        }
        lines[row] = signs + ' ' + lines[row];
        return session.replace(new Range(from, 0, to, to_column), lines.join('\n'));
      };

      this.emphasize = function(level) {
        var i, range, session, signs;
        session = this.editor.getSession();
        signs = '';
        for (i = 1; 1 <= level ? i <= level : i >= level; 1 <= level ? i++ : i--) {
          signs += '*';
        }
        range = this.editor.getSelectionRange();
        if (range.start.column === range.end.column && range.start.row === range.end.row) {
          return;
        }
        session.insert(range.start, signs);
        range.end.column += level;
        session.insert(range.end, signs);
        return this.editor.clearSelection();
      };

      this.prefixLines = function(sign) {
        var session = this.editor.getSession();
        var range = this.editor.getSelectionRange();
        var lines = session.getLines(range.start.row, range.end.row);
        var to = lines[lines.length - 1].length;
        var pref = sign;
        for (var i = 0; i < lines.length; i++) {
          if (sign === '.') {
            pref = (i + 1) + sign;
          }
          lines[i] = "" + pref + " " + lines[i];
        }
        session.replace(new Range(range.start.row, 0, range.end.row, to), lines.join('\n'));
        return this.editor.clearSelection();
      };

      this.embedOptions = function(text) {
        return $([
          '<div class="drupdown-ref-options">',
            '<label for="text">' + (Drupal.t('Link text')) + '</label>',
            '<input type="text" name="text" class="form-text" value="' + text + '"/>',
            '<label for="title">' + (Drupal.t('Link title')) + '</label>',
            '<input type="text" name="title" class="form-text" value="' + text + '"/>',
            '<label for="uri">' + (Drupal.t('Web address')) + '</label>',
            '<input type="text" name="uri" class="form-text" value=""/>',
          '</div>'
        ].join(''));
      };

      this.imageOptions = function() {
        var $form = $([
          '<div class="drupdown-image-options">',
            '<label for="format">' + Drupal.t('Format') + '</label>',
            '<select name="format" id="format"></select>',
            '<label for="file">' + Drupal.t('Image') + '</label>',
            '<select name="file" id="file"></select>',
          '</div>'
        ].join(''));

        var files = [];
        $('.drupdown-embed-uri').each(function(){
          files.push($(this).val());
        });

        var formats = Drupal.settings.drupdown_images.styles[this.format];
        var $formats = $('#format', $form);
        $.each(formats, function(name, style) {
          $formats.append(['<option value="' + name + '">', name, '</option>'].join(''));
        });

        var $files = $('#file', $form);
        for (var i in files) {
          $files.append([
            '<option value="' + files[i] + '">',
            files[i].replace(/(.*\/)/, ''),
            '</option>'
          ].join(''));
        }
        return $form;
      };

      this.buttons = function() {
        var buttons = [];
        // Add buttons for headings 1-5
        var that = this;
        var headings = [];
        for (i = 1; i <= 5; i++) {
          headings.push({
            title: 'H' + i,
            class: 'heading heading-' + i,
            description: Drupal.t('Insert heading level !level.',{
              '!level': i
            }),
            callback: function(level) {
              that.header(level);
            },
            arguments: [i+1]
          });
        }
        buttons.push({
          title: Drupal.t('Headings'),
          class: 'headings',
          children: headings
        });

        // Add buttons for emphasized and strong emphasized.
        buttons.push({
          title: Drupal.t('Accentuations'),
          class: 'accentuations',
          children: [{
            title: Drupal.t('I'),
            class: 'emphasized',
            description: Drupal.t('Emphasize text.'),
            callback: function() {
              that.emphasize(1);
            }
          },{
            title: Drupal.t('B'),
            class: 'strong',
            description: Drupal.t('Strongly emphasize text.'),
            callback: function() {
              that.emphasize(2);
            }
          }]
        });

        // Add buttons for lists.
        buttons.push({
          title: Drupal.t('Lists'),
          class: 'lists',
          children: [{
            title: Drupal.t('UL'),
            class: 'unordered-list',
            description: Drupal.t('Create an unordered list.'),
            callback: function() {
              that.prefixLines('-');
            }
          },{
            title: Drupal.t('OL'),
            class: 'ordered-list',
            description: Drupal.t('Create an ordered list.'),
            callback: function() {
              that.prefixLines('.');
            }
          }]
        });

        // Quote button.
        buttons.push({
          title: Drupal.t('Quote'),
          class: 'quote',
          description: Drupal.t('Mark text as quoted and choose a floating direction.'),
          callback: function() {
            that.prefixLines('> ');
          }
        });

        // Link button.
        buttons.push({
          title: Drupal.t('Hyperlink'),
          class: 'hyperlink',
          description: Drupal.t('Insert a hyperlink.'),
          callback: function() {
            var range = that.editor.getSelectionRange();
            var text = that.editor.getSession().doc.getTextRange(range);
            var $dialog = $('<div title="' + (Drupal.t('Insert Link')) + ':">');
            $dialog.append(that.embedOptions(text));
            that.dialog($dialog, function(values){
              var link = "[" + values['text'] + "](" + values['uri'] + " \"" + values['title'] + "\")";
              that.editor.getSession().replace(range, link);
            });
          }
        });
        return buttons;
      };
    }).call(Toolbar.prototype);
    exports.Toolbar = Toolbar;
  });
}(jQuery));
