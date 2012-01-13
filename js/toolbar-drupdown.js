(function() {
  var $;

  $ = jQuery;

  define('ace/toolbar/drupdown', ['require', 'exports', 'module'], function(require, exports, module) {
    var DrupdownToolbar, Range;
    Range = require("ace/range").Range;
    DrupdownToolbar = (function() {

      function DrupdownToolbar(element, editor) {
        this.element = element;
        this.editor = editor;
      }

      DrupdownToolbar.prototype.header = function(level) {
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
        switch (level) {
          case 1:
          case 2:
            underline = lines[row].replace(/./g, level - 1 ? '-' : '=');
            lines.splice(row + 1, 0, underline);
            break;
          default:
            signs = '';
            while (--level) {
              signs += '#';
            }
            lines[row] = signs + ' ' + lines[row];
        }
        return session.replace(new Range(from, 0, to, to_column), lines.join('\n'));
      };

      DrupdownToolbar.prototype.emphasize = function(level) {
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

      DrupdownToolbar.prototype.prefixLines = function(sign) {
        var i, lines, range, session, to, _ref;
        session = this.editor.getSession();
        range = this.editor.getSelectionRange();
        lines = session.getLines(range.start.row, range.end.row);
        to = lines[lines.length - 1].length;
        for (i = 0, _ref = lines.length; 0 <= _ref ? i < _ref : i > _ref; 0 <= _ref ? i++ : i--) {
          lines[i] = "" + sign + " " + lines[i];
        }
        session.replace(new Range(range.start.row, 0, range.end.row, to), lines.join('\n'));
        return this.editor.clearSelection();
      };

      DrupdownToolbar.prototype.floatOptions = function() {
        return '<div class="drupdown-float-options clearfix">\n  <div class="column">\n    <div class="icon-float-left">Left</div>\n    <input type="radio" name="position" value="<"/>\n  </div>\n  <div class="column">\n    <div class="icon-float-center">Center</div>\n    <input type="radio" name="position" value="|" checked/>\n  </div>\n  <div class="column">\n    <div class="icon-float-right">Right</div>\n    <input type="radio" name="position" value=">"/>\n  </div>\n</div>';
      };

      DrupdownToolbar.prototype.processQuote = function() {};

      DrupdownToolbar.prototype.render = function() {
        var blocks, headingbutton, headings, i, lists, styles,
          _this = this;
        headings = $('<span></span>');
        headingbutton = function(i) {
          var button;
          return button = $("<button>H" + i + "</button>").button().click(function() {
            _this.header(i);
            return false;
          });
        };
        for (i = 1; i <= 5; i++) {
          headings.append(headingbutton(i));
        }
        headings.buttonset().appendTo(this.element);
        styles = $('<span></span>');
        $('<button><span style="font-weight:bold">B</span></button>').button().click(function() {
          _this.emphasize(2);
          return false;
        }).appendTo(styles);
        $('<button><span style="font-style:italic">I</span></button>').button().click(function() {
          _this.emphasize(1);
          return false;
        }).appendTo(styles);
        styles.buttonset().appendTo(this.element);
        lists = $('<span></span>');
        $('<button>ul</button>').button({
          text: false,
          icons: {
            primary: 'ui-icon-bullet'
          }
        }).click(function() {
          _this.prefixLines('-');
          return false;
        }).appendTo(lists);
        $('<button>ol</button>').button({
          text: false,
          icons: {
            primary: 'ui-icon-check'
          }
        }).click(function() {
          _this.prefixLines('+');
          return false;
        }).appendTo(lists);
        lists.buttonset().appendTo(this.element);
        blocks = $('<span></span>');
        $('<button>quote</button>').button({
          text: false,
          icons: {
            primary: 'ui-icon-comment'
          }
        }).click(function() {
          var dialog;
          dialog = $("<div title=\"" + (Drupal.t('Choose position')) + "\">" + (_this.floatOptions()) + "</div>");
          $('.drupdown-float-options .column', dialog).click(function() {
            return $('input', this).attr('checked', 'checked');
          });
          dialog.dialog({
            modal: true,
            show: 'fade',
            hide: 'fade',
            buttons: {
              'OK': function() {
                var sign;
                sign = $('input[name=position]:checked', dialog).val();
                _this.prefixLines(sign);
                return dialog.dialog('close');
              }
            }
          });
          return false;
        }).appendTo(blocks);
        $('<button>link</button>').button({
          text: false,
          icons: {
            primary: 'ui-icon-link'
          }
        }).click(function() {
          var dialog, range, text;
          range = _this.editor.getSelectionRange();
          text = _this.editor.getSession().doc.getTextRange(range);
          dialog = $("<div title=\"" + (Drupal.t('Insert Link')) + "\" class=\"drupdown-dialog\">\n ')label for=\"text\">" + (Drupal.t('Link text')) + "</label>\n  <input type=\"text\" name=\"text\" class=\"ui-widget-content ui-corner-all\" value=\"" + text + "\"/>\n  <label for=\"title\">" + (Drupal.t('Link title')) + "</label>\n  <input type=\"text\" name=\"title\" class=\"ui-widget-content ui-corner-all\" value=\"" + text + "\"/>\n  <label for=\"uri\">" + (Drupal.t('Web address')) + "</label>\n  <input type=\"text\" name=\"uri\" class=\"ui-widget-content ui-corner-all\" value=\"\"/>\n</div>").dialog({
            modal: true,
            show: 'fade',
            hide: 'fade',
            buttons: {
              'OK': function() {
                var link, title, uri;
                text = $('input[name=text]', dialog).val();
                title = $('input[name=title]', dialog).val();
                uri = $('input[name=uri]', dialog).val();
                link = "[" + text + "](" + uri + " \"" + title + "\")";
                _this.editor.getSession().replace(range, link);
                return dialog.dialog('close');
              }
            }
          });
          return false;
        }).appendTo(blocks);
        $('<button>embed</button>').button({
          text: false,
          icons: {
            primary: 'ui-icon-image'
          }
        }).click(function() {
          var dialog, file, files, input, range, text, _i, _len;
          range = _this.editor.getSelectionRange();
          text = _this.editor.getSession().doc.getTextRange(range);
          dialog = $("<div title=\"" + (Drupal.t('Insert Resource')) + "\" class=\"drupdown-dialog\">\n  " + (_this.floatOptions()) + "\n  <label for=\"text\">" + (Drupal.t('Alternative text')) + "</label>\n  <input type=\"text\" name=\"text\" class=\"ui-widget-content ui-corner-all\" value=\"" + text + "\"/>\n  <label for=\"title\">" + (Drupal.t('Caption')) + "</label>\n  <input type=\"text\" name=\"title\" class=\"ui-widget-content ui-corner-all\" value=\"" + text + "\"/>\n  <label for=\"uri\">" + (Drupal.t('Web address')) + "</label>\n  <div class=\"drupdown-resource\">\n    <input type=\"text\" name=\"uri\" class=\"uri ui-widget-content ui-corner-all\" value=\"\"/>\n    <button class=\"drupdown-resource-choose\">Choose</button>\n  </div>\n</div>");
          $('.drupdown-float-options .column', dialog).click(function() {
            return $('input', this).attr('checked', 'checked');
          });
          files = (function() {
            var _i, _len, _ref, _results;
            _ref = $('.drupdown-resource');
            _results = [];
            for (_i = 0, _len = _ref.length; _i < _len; _i++) {
              file = _ref[_i];
              _results.push($(file).val());
            }
            return _results;
          })();
          for (_i = 0, _len = files.length; _i < _len; _i++) {
            file = files[_i];
            $.each(Drupal.settings.drupdown.styles, function(style, val) {
              var path;
              if (val && val !== 'original') {
                path = file.replace(/^original:\/\//, style + '://');
                return files.push(path);
              }
            });
          }
          input = $('input[name=uri]', dialog);
          $(input).autocomplete({
            source: files,
            minlength: 0,
            delay: 0
          });
          $('.drupdown-resource-choose', dialog).button({
            text: false,
            icons: {
              primary: 'ui-icon-search'
            }
          }).click(function() {
            $(input).autocomplete('search', ':');
            $(this).blur();
            $(input).focus();
            return false;
          });
          dialog.dialog({
            modal: true,
            show: 'fade',
            hide: 'fade',
            buttons: {
              'OK': function() {
                var link, sign, title, uri;
                sign = $('input[name=position]:checked', dialog).val();
                if (sign === '|') sign = '!';
                text = $('input[name=text]', dialog).val();
                title = $('input[name=title]', dialog).val();
                uri = $('input[name=uri]', dialog).val();
                link = "" + sign + "[" + text + "](" + uri + " \"" + title + "\")";
                _this.editor.getSession().replace(range, link);
                return dialog.dialog('close');
              }
            }
          });
          return false;
        }).appendTo(blocks);
        return blocks.buttonset().appendTo(this.element);
      };

      return DrupdownToolbar;

    })();
    exports.Toolbar = DrupdownToolbar;
  });

}).call(this);
