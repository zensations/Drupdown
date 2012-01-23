define('drupdown/highlight', ['require', 'exports', 'module' , 'ace/lib/oop', 'ace/mode/text_highlight_rules', 'ace/mode/javascript_highlight_rules', 'ace/mode/xml_highlight_rules', 'ace/mode/html_highlight_rules', 'ace/mode/css_highlight_rules'], function(require, exports, module) {
    "use strict";

    var oop = require("ace/lib/oop");
    var TextHighlightRules = require("ace/mode/text_highlight_rules").TextHighlightRules;
    var JavaScriptHighlightRules = require("ace/mode/javascript_highlight_rules").JavaScriptHighlightRules;
    var XmlHighlightRules = require("ace/mode/xml_highlight_rules").XmlHighlightRules;
    var HtmlHighlightRules = require("ace/mode/html_highlight_rules").HtmlHighlightRules;
    var CssHighlightRules = require("ace/mode/css_highlight_rules").CssHighlightRules;

    function github_embed(tag, prefix) {
        return { // Github style block
            token : "support.function",
            regex : "^```" + tag + "\\s*$",
            next  : prefix + "start"
        };
    }

    var DrupdownHighlightRules = function() {

        // regexp must not have capturing parentheses
        // regexps are ordered -> the first match is used

        this.$rules = {
            "start" : [ {
                token : "empty_line",
                regex : '^$'
            }, { // code span `
                token : "support.function",
                regex : "(`+)([^\\r]*?[^`])(\\1)"
            }, { // code block
                token : "support.function",
                regex : "^[ ]{4}.+"
            }, { // h1
                token: "markup.heading.1",
                regex: "^=+(?=\\s*$)"
            }, { // h2
                token: "markup.heading.1",
                regex: "^\\-+(?=\\s*$)"
            }, { // header
                token : function(value) {
                    return "markup.heading." + value.length;
                },
                regex : "^#{1,6}.*$"
            },
            github_embed("javascript", "js-"),
            github_embed("xml", "xml-"),
            github_embed("html", "html-"),
            github_embed("css", "css-"),
            { // Github style block
                token : "support.function",
                regex : "^```[a-zA-Z]+\\s*$",
                next  : "githubblock"
            }, { // block quote
                token : "string",
                regex : "^[<>!][ ].+$",
                next  : "blockquote"
            }, { // reference
                token : ["text", "string", "text", "support.function", "string", "text"],
                regex : "^([ ]{0,3}\\[)([^\\]]+)(\\]:\\s*)([^ ]+)(\\s*(?:[\"][^\"]+[\"])?\\s*)$"
            }, { // link by reference
                token : ["text", "string", "text", "support.function", "text"],
                regex : "(\\[)((?:[[^\\]]*\\]|[^\\[\\]])*)(\\][ ]?(?:\\n[ ]*)?\\[)(.*?)(\\])"
            }, { // simple reference
                token : ["text", "string", "text"],
                regex : "(\\[)((?:[[^\\]]*\\]|[^\\[\\]])*)(\\])"
            }, { // link by url
                token : ["text", "string", "text"],
                regex : "(\\[)"+
                    "(\\[[^\\]]*\\]|[^\\[\\]]*)"+
                    "(\\]\\([ \\t]*)"+
                    "(<?(?:(?:[^\\(]*?\\([^\\)]*?\\)\\S*?)|(?:.*?))>?)"+
                    "((?:[ \t]*\"(?:.*?)\"[ \\t]*)?)"+
                    "(\\))"
            }, { // embed
                token : ["markup.list", "markup.list", "string", "markup.list", "support.function", "string", "markup.list"],
                regex : "([<>!])" +
                    "(\\[)"+
                    "(\\[[^\\]]*\\]|[^\\[\\]]*)"+
                    "(\\]\\([ \\t]*)"+
                    "(<?(?:(?:[^\\(]*?\\([^\\)]*?\\)\\S*?)|(?:.*?))>?)"+
                    "((?:[ \t]*\"(?:.*?)\"[ \\t]*)?)"+
                    "(\\))"
            }, { // HR *
                token : "constant",
                regex : "^[ ]{0,2}(?:[ ]?\\*[ ]?){3,}\\s*$"
            }, { // HR -
                token : "constant",
                regex : "^[ ]{0,2}(?:[ ]?\\-[ ]?){3,}\\s*$"
            }, { // HR _
                token : "constant",
                regex : "^[ ]{0,2}(?:[ ]?\\_[ ]?){3,}\\s*$"
            }, { // list
                token : "markup.list",
                regex : "^\\s{0,3}(?:[*+-]|\\d+\\.)\\s+",
                next  : "listblock"
            }, { // strong ** __
                token : "string",
                regex : "([*]{2}|[_]{2}(?=\\S))([^\\r]*?\\S[*_]*)(\\1)"
            }, { // emphasis * _
                token : "string",
                regex : "([*]|[_](?=\\S))([^\\r]*?\\S[*_]*)(\\1)"
            }, { // 
                token : ["text", "url", "text"],
                regex : "(<)("+
                    "(?:https?|ftp|dict):[^'\">\\s]+"+
                    "|"+
                    "(?:mailto:)?[-.\\w]+\\@[-a-z0-9]+(?:\\.[-a-z0-9]+)*\\.[a-z]+"+
                    ")(>)"
            }, {
                token : "text",
                regex : "[^\\*_%$`\\[#<>]+"
            } ],

            "listblock" : [ { // Lists only escape on completely blank lines.
                token : "empty_line",
                regex : "^$",
                next  : "start"
            }, {
                token : "markup.list",
                regex : ".+"
            } ],

            "blockquote" : [ { // BLockquotes only escape on blank lines.
                token : "empty_line",
                regex : "^\\s*$",
                next  : "start"
            }, {
                token : "string",
                regex : ".+"
            } ],

            "githubblock" : [ {
                token : "support.function",
                regex : "^```",
                next  : "start"
            }, {
                token : "support.function",
                regex : ".+"
            } ]
        };

        this.embedRules(JavaScriptHighlightRules, "js-", [{
            token : "support.function",
            regex : "^```",
            next  : "start"
        }]);

        this.embedRules(HtmlHighlightRules, "html-", [{
            token : "support.function",
            regex : "^```",
            next  : "start"
        }]);

        this.embedRules(CssHighlightRules, "css-", [{
            token : "support.function",
            regex : "^```",
            next  : "start"
        }]);

        this.embedRules(XmlHighlightRules, "xml-", [{
            token : "support.function",
            regex : "^```",
            next  : "start"
        }]);
    };
    oop.inherits(DrupdownHighlightRules, TextHighlightRules);

    exports.DrupdownHighlightRules = DrupdownHighlightRules;
});
