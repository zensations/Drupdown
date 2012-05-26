define(function(require, exports, module) {
    "use strict";
    var oop = require("ace/lib/oop");
    var TextHighlightRules = require("ace/mode/text_highlight_rules").TextHighlightRules;
    var JavaScriptHighlightRules = require("ace/mode/javascript_highlight_rules").JavaScriptHighlightRules;
    var XmlHighlightRules = require("ace/mode/xml_highlight_rules").XmlHighlightRules;
    var HtmlHighlightRules = require("ace/mode/html_highlight_rules").HtmlHighlightRules;
    var CssHighlightRules = require("ace/mode/css_highlight_rules").CssHighlightRules;

    var DrupdownHighlightRules = function() {
        // regexp must not have capturing parentheses
        // regexps are ordered -> the first match is used
        this.$rules = {
          "start" : Drupal.settings.drupdown.rules
        };
    };
    oop.inherits(DrupdownHighlightRules, TextHighlightRules);

    exports.DrupdownHighlightRules = DrupdownHighlightRules;
});
