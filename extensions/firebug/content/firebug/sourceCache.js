/* See license.txt for terms of usage */

FBL.ns(function() { with (FBL) {

// ************************************************************************************************
// Constants

const Cc = Components.classes;
const Ci = Components.interfaces;
const nsIIOService = Ci.nsIIOService;
const nsIRequest = Ci.nsIRequest;
const nsICachingChannel = Ci.nsICachingChannel;
const nsIScriptableInputStream = Ci.nsIScriptableInputStream;
const nsIUploadChannel = Ci.nsIUploadChannel;
const nsIHttpChannel = Ci.nsIHttpChannel;

const IOService = Cc["@mozilla.org/network/io-service;1"];
const ioService = IOService.getService(nsIIOService);
const ScriptableInputStream = Cc["@mozilla.org/scriptableinputstream;1"];
const chromeReg = CCSV("@mozilla.org/chrome/chrome-registry;1", "nsIToolkitChromeRegistry");

const LOAD_FROM_CACHE = nsIRequest.LOAD_FROM_CACHE;
const LOAD_BYPASS_LOCAL_CACHE_IF_BUSY = nsICachingChannel.LOAD_BYPASS_LOCAL_CACHE_IF_BUSY;

const NS_BINDING_ABORTED = 0x804b0002;

// ************************************************************************************************

Firebug.SourceCache = function(context)
{
    this.context = context;
    this.cache = {};
};

Firebug.SourceCache.prototype = extend(new Firebug.Listener(),
{
    isCached: function(url)
    {
        return (this.cache[url] ? true : false);
    },

    loadText: function(url, method, file)
    {
        var lines = this.load(url, method, file);
        return lines ? lines.join("") : null;
    },

    load: function(url, method, file)
    {
        var response = this.cache[this.removeAnchor(url)];
        if (response)
            return response;

        var d = FBL.splitDataURL(url);  //TODO the RE should not have baseLine
        if (d)
        {
            var src = d.encodedContent;
            var data = decodeURIComponent(src);
            var lines = splitLines(data)
            this.cache[url] = lines;

            return lines;
        }

        var j = FBL.reJavascript.exec(url);
        if (j)
        {
            var src = url.substring(FBL.reJavascript.lastIndex);
            var lines = splitLines(src);
            this.cache[url] = lines;

            return lines;
        }

        var c = FBL.reChrome.test(url);
        if (c)
        {
            if (Firebug.filterSystemURLs)
                return ["Filtered chrome url "+url];  // ignore chrome

            var chromeURI = makeURI(url);
            var localURI = chromeReg.convertChromeURL(chromeURI);
            return this.loadFromLocal(localURI.spec);
        }

        c = FBL.reFile.test(url);
        if (c)
        {
            return this.loadFromLocal(url);
        }

        // Unfortunately, the URL isn't available so, let's try to use FF cache.
        // Notice that additional network request to the server can be made in
        // this method (double-load).
        return this.loadFromCache(url, method, file);
    },

    store: function(url, text)
    {
        var tempURL = this.removeAnchor(url);

        var lines = splitLines(text);
        return this.storeSplitLines(tempURL, lines);
    },

    removeAnchor: function(url)
    {
        var index = url.indexOf("#");
        if (index < 0)
            return url;

        return url.substr(0, index);
    },

    loadFromLocal: function(url)
    {
        // if we get this far then we have either a file: or chrome: url converted to file:
        var src = getResource(url);
        if (src)
        {
            var lines = splitLines(src);
            this.cache[url] = lines;

            return lines;
        }
    },

    loadFromCache: function(url, method, file)
    {
        var doc = this.context.window.document;
        if (doc)
            var charset = doc.characterSet;
        else
            var charset = "UTF-8";

        var channel;
        try
        {
            channel = ioService.newChannel(url, null, null);
            channel.loadFlags |= LOAD_FROM_CACHE | LOAD_BYPASS_LOCAL_CACHE_IF_BUSY;

            if (method && (channel instanceof nsIHttpChannel))
            {
                var httpChannel = QI(channel, nsIHttpChannel);
                httpChannel.requestMethod = method;
            }
        }
        catch (exc)
        {
            return;
        }

        if (url == this.context.browser.contentWindow.location.href)
        {
            if (channel instanceof nsIUploadChannel)
            {
                var postData = getPostStream(this.context);
                if (postData)
                {
                    var uploadChannel = QI(channel, nsIUploadChannel);
                    uploadChannel.setUploadStream(postData, "", -1);
                }
            }

            if (channel instanceof nsICachingChannel)
            {
                var cacheChannel = QI(channel, nsICachingChannel);
                cacheChannel.cacheKey = getCacheKey(this.context);
            }
        }
        else if ((method == "PUT" || method == "POST") && file)
        {
            if (channel instanceof nsIUploadChannel)
            {
                // In case of PUT and POST, don't forget to use the original body.
                var postData = getPostText(file, this.context);
                if (postData)
                {
                    var postDataStream = getInputStreamFromString(postData);
                    var uploadChannel = QI(channel, nsIUploadChannel);
                    uploadChannel.setUploadStream(postDataStream, "application/x-www-form-urlencoded", -1);
                }
            }
        }

        var stream;
        try
        {
            stream = channel.open();
        }
        catch (exc)
        {
            return ["sourceCache.load FAILS for url="+url, exc.toString()];
        }

        try
        {
            var data = readFromStream(stream, charset);
            var lines = splitLines(data);
            this.cache[url] = lines;
            return lines;
        }
        catch (exc)
        {
            return ["sourceCache.load FAILS for url="+url, exc.toString()];
        }
        finally
        {
            stream.close();
        }
    },

    storeSplitLines: function(url, lines)
    {
        return this.cache[url] = lines;
    },

    invalidate: function(url)
    {
        url = this.removeAnchor(url);

        delete this.cache[url];
    },

    getLine: function(url, lineNo)
    {
        var lines = this.load(url);
        if (lines)
        {
            if (lineNo <= lines.length)
                return lines[lineNo-1];
            else
                return (lines.length == 1) ? lines[0] : "("+lineNo+" out of range "+lines.length+")";
        }
        else
            return "(no source for "+url+")";
    }
});

// xxxHonza getPostText and readPostTextFromRequest are copied from
// net.js. These functions should be removed when this cache is
// refactored due to the double-load problem.
function getPostText(file, context)
{
    if (!file.postText)
        file.postText = readPostTextFromPage(file.href, context);

    if (!file.postText)
        file.postText = readPostTextFromRequest(file.request, context);

    return file.postText;
}

// ************************************************************************************************

function getPostStream(context)
{
    try
    {
        var webNav = context.browser.webNavigation;
        var descriptor = QI(webNav, Ci.nsIWebPageDescriptor).currentDescriptor;
        var entry = QI(descriptor, Ci.nsISHEntry);

        if (entry.postData)
        {
            // Seek to the beginning, or it will probably start reading at the end
            var postStream = QI(entry.postData, Ci.nsISeekableStream);
            postStream.seek(0, 0);
            return postStream;
        }
     }
     catch (exc)
     {
     }
}

function getCacheKey(context)
{
    try
    {
        var webNav = context.browser.webNavigation;
        var descriptor = QI(webNav, Ci.nsIWebPageDescriptor).currentDescriptor;
        var entry = QI(descriptor, Ci.nsISHEntry);
        return entry.cacheKey;
     }
     catch (exc)
     {
     }
}

// ************************************************************************************************
}});
