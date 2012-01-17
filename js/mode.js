define('drupdown/mode', ['require', 'exports', 'module' , 'ace/lib/oop', 'ace/mode/text', 'ace/mode/javascript', 'ace/mode/xml', 'ace/mode/html', 'ace/tokenizer', 'drupdown/highlight'], function(require, exports, module) {
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

    (function() {
        this.getNextLineIndent = function(state, line, tab) {
            if (state == "listblock") {
                var match = /^((?:.+)?)([-+*][ ]+)/.exec(line);
                if (match) {
                    return new Array(match[1].length + 1).join(" ") + match[2];
                } else {
                    return "";
                }
            } else if (state == "blockquote") {
                var match = /^((?:.+)?)([<>!][ ]+)/.exec(line);
                if (match) {
                    return new Array(match[1].length + 1).join(" ") + match[2];
                } else {
                    return "";
                }
            } else {
                return this.$getIndent(line);
            }
        };
    }).call(Mode.prototype);

    exports.Mode = Mode;
});
