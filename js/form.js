/* 
 *  @copyright The Royal National Theatre
 *  @author John-Paul Drawneek <jdrawneek@nationaltheatre.org.uk>
 */
(function () {
    "use strict";
    var nt_tweet_form = {
        init: function () {
            var $this = this;
            jQuery('#edit-nt-twitter-type .form-radio').live('click', function () {
                $this.hide_tweets(jQuery(this), jQuery(this).val());
            });
            jQuery('#edit-nt-twitter-type .form-radio:checked').each(function () {
                $this.hide_tweets(jQuery(this), jQuery(this).val());
            });
        },
        hide_tweets: function (image, mode) {
            if (mode === "latest") {
                jQuery('#edit-tweet-list').hide();
            } else {
                jQuery('#edit-tweet-list').show();
            }
        }
    };
    jQuery(document).ready(function () {
        jQuery("#nt-tweet-table").tablesorter();
        nt_tweet_form.init();
    });
}) ();