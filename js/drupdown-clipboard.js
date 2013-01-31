(function($){
  var clip = false;
  function glueClip(element) {
    if (!clip) {
      ZeroClipboard.setDefaults({
        moviePath: Drupal.settings.drupdown.ZeroClipboardPath
      });
      clip = new ZeroClipboard();
      clip.on('complete', function(client, args){
        var $el = $(this);
        var text = $el.text();
        Drupal.drupdown_clipboard_value = args.text;
        $el.html(Drupal.t('<em>Copied path to clipboard.</em>'));
        window.setTimeout(function(){
          $el.html(text);
        }, 3000);
      });
    }
    clip.glue(element);
  }
  Drupal.behaviors.drupdown_clipboard = {
    attach: function(context, settings) {
      $('.drupdown-copy-links a', context).each(function(){
        glueClip(this);
      });
      $('.drupdown-copy-links a', context).hover(function(){
        if (clip) {
          $(clip.htmlBridge).css({
            'top': $(this).offset().top,
            'left': $(this).offset().left,
            'width': $(this).width(),
            'height': $(this).height()
          });
        }
      });
    }
  };
}(jQuery));