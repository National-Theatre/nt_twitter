/* 
 *  @copyright The Royal National Theatre
 *  @author John-Paul Drawneek <jdrawneek@nationaltheatre.org.uk>
 *  Hacked up version of jquery.tweet.js
 *  Copyright (c) 2008-2012 Todd Matthews & Steve Purcell

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
(function ($) {
    
    function replacer (regex, replacement) {
      return function() {
        var returning = [];
        this.each(function() {
          returning.push(this.replace(regex, replacement));
        });
        return $(returning);
      };
    }
    $.fn.extend({
      linkUser: replacer(/(^|[\W])@(\w+)/gi, "$1<span class=\"at\">@</span><a href=\"http://twitter.com/$2\">$2</a>"),
      // Support various latin1 (\u00**) and arabic (\u06**) alphanumeric chars
      linkHash: replacer(/(?:^| )[\#]+([\w\u00c0-\u00d6\u00d8-\u00f6\u00f8-\u00ff\u0600-\u06ff]+)/gi,
                         ' <a href="http://search.twitter.com/search?q=&tag=$1&lang=all'+
                         
                         '" class="tweet_hashtag">#$1</a>'),
      makeHeart: replacer(/(&lt;)+[3]/gi, "<tt class='heart'>&#x2665;</tt>")
    });
    
    var tweetHelper = {
        settings: {},
        // See http://daringfireball.net/2010/07/improved_regex_for_matching_urls
        url_regexp: /\b((?:https?:\/\/|www\d{0,3}[.]|[a-z0-9.\-]+[.][a-z]{2,4}\/)(?:[^\s()<>]+|\(([^\s()<>]+|(\([^\s()<>]+\)))*\))+(?:\(([^\s()<>]+|(\([^\s()<>]+\)))*\)|[^\s`!()\[\]{};:'".,<>?«»“”‘’]))/gi,
        init: function (config) {
            this.settings = $.extend({
                twitter_url: "twitter.com",
                twitter_api_url: "api.twitter.com",
                twitter_search_url: "search.twitter.com",
                twitter_id: false
            }, config);
            this.each(function () {
//                console.log(this);
                tweetHelper.process($(this));
            });
        },
        process: function (element) {
            var tweet = {
                id: element.attr('tweet_id'),
                channel: element.attr('channel'),
                hash: element.attr('hash')
            };
            
            $.getJSON(this.build_api_url(tweet), {}, function (data) {
//                console.log(data);
                var info;
                if (typeof data.text !== 'undefined') {
                    info = tweetHelper.linkURLs(data.text, data.entities);
                    info = $([info]).linkUser().linkHash();
                    element.find('.message').append(info[0]);
                    element.find('.time').append(tweetHelper.format_relative_time(tweetHelper.extract_relative_time(tweetHelper.parse_date(data.created_at))));
                } else if ((typeof data.results !== 'undefined') && (data.results.length > 0)) {
                    info = tweetHelper.linkURLs(data.results[0].text, data.results[0].entities);
                    info = $([info]).linkUser().linkHash();
                    element.find('.message').append(info[0]);
                    element.find('.time').append(tweetHelper.format_relative_time(tweetHelper.extract_relative_time(tweetHelper.parse_date(data.results[0].created_at))));
                } else {
                    element.find('.message').html('<p>No tweets found.</p>');
                }
            });
        },
        escapeHTML: function (s) {
          return s.replace(/</g,"&lt;").replace(/>/g,"^&gt;");
        },
        linkURLs: function (text, entities) {
          return text.replace(tweetHelper.url_regexp, function(match) {
            var url = (/^[a-z]+:/i).test(match) ? match : "http://"+match;
            var text = match;
            for(var i = 0; i < entities.length; ++i) {
              var entity = entities[i];
              if (entity.url == url && entity.expanded_url) {
                url = entity.expanded_url;
                text = entity.display_url;
                break;
              }
            }
            return "<a href=\""+tweetHelper.escapeHTML(url)+"\">"+tweetHelper.escapeHTML(text)+"</a>";
          });
        },
        build_api_url: function (config) {
          var proto = ('https:' == document.location.protocol ? 'https:' : 'http:');
          var common_params = '&include_entities=1&callback=?';
          if (typeof config.id !== 'undefined' && config.id != "") {
              return proto+"//api.twitter.com/1/statuses/show/" + config.id + '.json?' + common_params;
          } else if(typeof config.hash !== 'undefined' && config.hash != "") {
              return proto+"//search.twitter.com/search.json?rpp=1&result_type=recent&q=from:" + encodeURIComponent(config.channel + '+#' + config.hash) + common_params;
          } else if (typeof config.channel !== 'undefined' && config.channel != "") {
              return proto+"//search.twitter.com/search.json?rpp=1&result_type=recent&q=from:" + encodeURIComponent(config.channel) + common_params;
          } else {
              return '';
          }
        },
        parse_date: function (date_str) {
          // The non-search twitter APIs return inconsistently-formatted dates, which Date.parse
          // cannot handle in IE. We therefore perform the following transformation:
          // "Wed Apr 29 08:53:31 +0000 2009" => "Wed, Apr 29 2009 08:53:31 +0000"
          return Date.parse(date_str.replace(/^([a-z]{3})( [a-z]{3} \d\d?)(.*)( \d{4})$/i, '$1,$2$4$3'));
        },
        extract_relative_time: function (date) {
          var toInt = function(val) { return parseInt(val, 10); };
          var relative_to = new Date();
          var delta = toInt((relative_to.getTime() - date) / 1000);
          if (delta < 1) delta = 0;
          return {
            days:    toInt(delta / 86400),
            hours:   toInt(delta / 3600),
            minutes: toInt(delta / 60),
            seconds: toInt(delta)
          };
        },
        format_relative_time: function (time_ago) {
          if ( time_ago.days > 2 )     return 'about ' + time_ago.days + ' days ago';
          if ( time_ago.hours > 24 )   return 'about a day ago';
          if ( time_ago.hours > 2 )    return 'about ' + time_ago.hours + ' hours ago';
          if ( time_ago.minutes > 45 ) return 'about an hour ago';
          if ( time_ago.minutes > 2 )  return 'about ' + time_ago.minutes + ' minutes ago';
          if ( time_ago.seconds > 1 )  return 'about ' + time_ago.seconds + ' seconds ago';
          return 'just now';
        }
    };
    
    $.fn.tweetListHelper = function(config) {
        var settings  = {},
            tweet     = {},
            tweetList = [],
            max_id    = false,
            pointer   = 0,
            template  = $('<blockquote class="tweet"><div class="message"></div><div class="time"></div></blockquote>'),
            nav_tmpl  = $('<li><a class="button back" href="#">Back</a></li><li><a class="button next" href="#">Next</a></li>'),
            wrapper;
        function init (config, element) {
            settings = $.extend({
                twitter_url: "twitter.com",
                twitter_api_url: "api.twitter.com",
                twitter_search_url: "search.twitter.com",
                twitter_id: false,
                count: 4,
                nav: 'ul.nav'
            }, config);
            wrapper = $(element);
            process(settings);
        }
        function draw () {
            wrapper.children().remove();
            $(settings.nav).children().remove();
            if (tweetList.length <= settings.count) {
                get_tweets(tweet);
            }
            var tweet_tpl;
            for (var i= pointer; (i<pointer+settings.count) && (i<tweetList.length); i++) {
                var data = tweetList[i];
                tweet_tpl = template.clone();
                if (typeof data.text !== 'undefined') {
                    info = tweetHelper.linkURLs(data.text, data.entities);
                    info = $([info]).linkUser().linkHash();
                    tweet_tpl.find('.message').append(info[0]);
                    tweet_tpl.find('.time').append(tweetHelper.format_relative_time(tweetHelper.extract_relative_time(tweetHelper.parse_date(data.created_at))));
                } else if ((typeof data.results !== 'undefined') && (data.results.length > 0)) {
                    info = tweetHelper.linkURLs(data.results[0].text, data.results[0].entities);
                    info = $([info]).linkUser().linkHash();
                    tweet_tpl.find('.message').append(info[0]);
                    tweet_tpl.find('.time').append(tweetHelper.format_relative_time(tweetHelper.extract_relative_time(tweetHelper.parse_date(data.results[0].created_at))));
                } else {
                    tweet_tpl.find('.message').html('<p>No tweets found.</p>');
                }
                wrapper.append(tweet_tpl);
            }
            var nav = nav_tmpl.clone();
            if (pointer - settings.count < 0) {
                nav.find('a.button.back').addClass('is-disabled');
            }
            if (pointer + settings.count > tweetList.length) {
                nav.find('a.button.next').addClass('is-disabled');
            }
            nav.find('a.button.back').click(function () {
                if (!$(this).hasClass('is-disabled')) {
                    pointer = pointer - settings.count;
                    draw();
                }
                return false;
            });
            nav.find('a.button.next').click(function () {
                if (!$(this).hasClass('is-disabled')) {
                    pointer = pointer + settings.count;
                    draw();
                }
                return false;
            });
            $(settings.nav).append(nav);
        }
        function build_api_url (config) {
          var proto = ('https:' == document.location.protocol ? 'https:' : 'http:');
          var common_params = '&count=200&include_rts=false&include_entities=1&callback=?';
          if (max_id) {
              var max_id_url = '&max_id=' + max_id;
          }
          else {
              var max_id_url = '';
          }
          return proto + "//api.twitter.com/1/statuses/user_timeline.json?screen_name=" + config.channel + max_id_url + common_params;
        }
        function get_tweets (tweet) {
            $.getJSON(build_api_url(tweet), {}, function (data) {
                var counter = 0;
                $(data).each(function (i) {
                    var flag = false;
                    $(this.entities.hashtags).each(function (i) {
                        if (this.text === tweet.hash) {
                            flag = true;
                        }
                    });
                    if (flag) {
                        tweetList.push(this);
                        counter++;
                    }
                    max_id = this.id;
                });
                if (counter > 0) {
                    draw();
                }
            });
        }
        function process (settings) {
            tweet = {
                channel: settings.channel,
                hash: settings.hashtag
            };
            get_tweets(tweet);
        }
        
        return this.each(function(i, element){
            init(config, element);
        });
    };
    
    $.fn.tweetHelper = function() {
        return tweetHelper.init.apply( this, arguments );
    }
    $(document).ready(function(){
        $('.twitter.usejs').tweetHelper();
    });
})(jQuery);
