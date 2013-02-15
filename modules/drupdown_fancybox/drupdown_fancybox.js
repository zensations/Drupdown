(function($) {
  Drupal.behaviors.drupal_fancybox = {
    attach: function(context, settings) {
      if (! settings.fancybox || !settings.fancybox.options) {
        settings.fancybox = {};
        settings.fancybox.options = {};
      }

      var options = $.extend({}, settings.fancybox.options, {
        helpers: settings.fancybox.helpers
      });

      var single = $.extend(true, {}, options, {
        helpers: {
          thumbs: null
        }
      });

      var gallery = 0;
      $('a.drupdown-zoom', context).fancybox(single);

      $('.drupdown-group', context).each(function() {
        $('a.drupdown-zoom', this)
            .attr('rel', 'ddg' + (++gallery))
            .fancybox(options);
      });

      $('a.drupdown-video', context).fancybox($.extend(true, {}, single, {
        helpers: {
          media: {}
        }
      }));
    }
  };
}(jQuery));