define(function(require, exports, module) {
  "use strict";

  var oop = require("ace/lib/oop");
  var TextMode = require("ace/mode/text").Mode;
  var JavaScriptMode = require("ace/mode/javascript").Mode;
  var XmlMode = require("ace/mode/xml").Mode;
  var HtmlMode = require("ace/mode/html").Mode;
  var Tokenizer = require("ace/tokenizer").Tokenizer;
  var DrupdownHighlightRules = require("drupdown/highlight").DrupdownHighlightRules;

  var Mode = function() {
    var highlighter = new DrupdownHighlightRules();
    this.$tokenizer = new Tokenizer(highlighter.getRules());
    this.$embeds = highlighter.getEmbeds();
    this.createModeDelegates({
      "js-": JavaScriptMode,
      "xml-": XmlMode,
      "html-": HtmlMode
    });
  };
  oop.inherits(Mode, TextMode);
  exports.Mode = Mode;
});
