/* See license.txt for terms of usage */

FBL.ns(function() { with (FBL) {

// ************************************************************************************************
// Constants

const Cc = Components.classes;
const Ci = Components.interfaces;

const ioService = Cc["@mozilla.org/network/io-service;1"].getService(Ci.nsIIOService);
const prefs = Cc["@mozilla.org/preferences-service;1"].getService(Ci.nsIPrefBranch2);
const versionChecker = Cc["@mozilla.org/xpcom/version-comparator;1"].getService(Ci.nsIVersionComparator);
const appInfo = Cc["@mozilla.org/xre/app-info;1"].getService(Ci.nsIXULAppInfo);

// Maximum cached size of a signle response (bytes)
var responseSizeLimit = 1024 * 1024 * 5;

// List of text content types. These content-types are cached.
var contentTypes =
{
    "text/plain": 1,
    "text/html": 1,
    "text/xml": 1,
    "text/xsl": 1,
    "text/xul": 1,
    "text/css": 1,
    "text/sgml": 1,
    "text/rtf": 1,
    "text/x-setext": 1,
    "text/richtext": 1,
    "text/javascript": 1,
    "text/jscript": 1,
    "text/tab-separated-values": 1,
    "text/rdf": 1,
    "text/xif": 1,
    "text/ecmascript": 1,
    "text/vnd.curl": 1,
    "text/x-json": 1,
    "text/x-js": 1,
    "text/js": 1,
    "text/vbscript": 1,
    "view-source": 1,
    "view-fragment": 1,
    "application/xml": 1,
    "application/xhtml+xml": 1,
    "application/vnd.mozilla.xul+xml": 1,
    "application/javascript": 1,
    "application/x-javascript": 1,
    "application/x-httpd-php": 1,
    "application/rdf+xml": 1,
    "application/ecmascript": 1,
    "application/http-index-format": 1,
    "application/json": 1,
    "application/x-js": 1,
    "multipart/mixed" : 1,
    "multipart/x-mixed-replace" : 1,
    "image/svg+xml" : 1
};

// ************************************************************************************************
// Model implementation

/**
 * Implementation of cache model. The only purpose of this object is to register an HTTP
 * observer so, HTTP communication can be interecepted and all incoming data stored within
 * a cache.
 */
Firebug.TabCacheModel = extend(Firebug.Module,
{
    dispatchName: "tabCache",
    contentTypes: contentTypes,
    fbListeners: [],

    initializeUI: function(owner)
    {
        responseSizeLimit = Firebug.getPref(Firebug.prefDomain, "cache.responseLimit");

        // Read additional text mime-types from preferences.
        var mimeTypes = Firebug.getPref(Firebug.prefDomain, "cache.mimeTypes");
        if (mimeTypes)
        {
            var list = mimeTypes.split(" ");
            for (var i=0; i<list.length; i++)
                contentTypes[list[i]] = 1;
        }

        // Register for HTTP events.
        if (Ci.nsITraceableChannel)
            httpObserver.addObserver(this, "firebug-http-event", false);
    },

    shutdown: function()
    {
        if (Ci.nsITraceableChannel)
            httpObserver.removeObserver(this, "firebug-http-event");
    },

    initContext: function(context)
    {
    },

    /* nsIObserver */
    observe: function(subject, topic, data)
    {
        try
        {
            if (!(subject instanceof Ci.nsIHttpChannel))
                return;
// XXXjjb this same code is in net.js, better to have it only once
            var win = getWindowForRequest(subject);
            if (win)
                var tabId = Firebug.getTabIdForWindow(win);
            if (!tabId)
                return;

            if (topic == "http-on-modify-request")
                this.onModifyRequest(subject, win, tabId);
            else if (topic == "http-on-examine-response")
                this.onExamineResponse(subject, win, tabId);
            else if (topic == "http-on-examine-cached-response")
                this.onCachedResponse(subject, win, tabId);
        }
        catch (err)
        {
        }
    },

    onModifyRequest: function(request, win, tabId)
    {
    },

    onExamineResponse: function(request, win, tabId)
    {
        this.registerStreamListener(request, win);
    },

    onCachedResponse: function(request, win, tabId)
    {
        this.registerStreamListener(request, win);
    },

    registerStreamListener: function(request, win)
    {
        try
        {
            // Due to #489317, the content type must be checked in onStartRequest
            //if (!this.shouldCacheRequest(request))
            //    return;

            request.QueryInterface(Ci.nsITraceableChannel);

            // Create Firebug's stream listener that is tracing HTTP responses.
            var newListener = CCIN("@joehewitt.com/firebug-channel-listener;1", "nsIStreamListener");
            newListener.wrappedJSObject.window = win;

            // Set proxy listener for back notifiction from XPCOM to chrome (using real interface
            // so nsIRequest object is properly passed from XPCOM scope).
            newListener.QueryInterface(Ci.nsITraceableChannel);
            newListener.setNewListener(new ChannelListenerProxy(win));

            // Add tee listener into the chain of request stream listeners so, the chain
            // doesn't include a JS code. This way all exceptions are propertly distributed
            // (#515051).
            // The nsIStreamListenerTee differes in following branches:
            // 1.9.1: not possible to registere nsIRequestObserver with tee
            // 1.9.2: implements nsIStreamListenerTee_1_9_2 with initWithObserver methods
            // 1.9.3: adds third parameter to the existing init method.
            if (versionChecker.compare(appInfo.version, "3.6*") >= 0)
            {
                var tee = CCIN("@mozilla.org/network/stream-listener-tee;1", "nsIStreamListenerTee");
                tee = tee.QueryInterface(Ci.nsIStreamListenerTee);

                if (Ci.nsIStreamListenerTee_1_9_2)
                    tee = tee.QueryInterface(Ci.nsIStreamListenerTee_1_9_2);

                // The response will be written into the outputStream of this pipe.
                // Both ends of the pipe must be blocking.
                var sink = CCIN("@mozilla.org/pipe;1", "nsIPipe");
                sink.init(true, true, 0, 0, null);

                var originalListener = request.setNewListener(tee);
                newListener.wrappedJSObject.sink = sink;

                if (tee.initWithObserver)
                    tee.initWithObserver(originalListener, sink.outputStream, newListener);
                else
                    tee.init(originalListener, sink.outputStream, newListener);
            }
            else
            {
                newListener.wrappedJSObject.listener = request.setNewListener(newListener);
            }

            // xxxHonza: this is a workaround for #489317. Just passing
            // shouldCacheRequest method to the component so, onStartRequest
            // can decide whether to cache or not.
            newListener.wrappedJSObject.shouldCacheRequest = function(request)
            {
                try {
                    return Firebug.TabCacheModel.shouldCacheRequest(request)
                } catch (err) {}
                return false;
            }

            // xxxHonza: this is a workaround for the tracing-listener to get the
            // right context. Notice that if the window (parent browser) is closed
            // during the response download the TabWatcher (used within this method)
            // is undefined. But in such a case no cache is needed anyway.
            // Another thing is that the context isn't available now, but will be
            // as soon as this method is used from the stream listener.
            newListener.wrappedJSObject.getContext = function(win)
            {
                try {
                    return TabWatcher.getContextByWindow(win);
                } catch (err){}
                return null;
            }
        }
        catch (err)
        {
        }
    },

    shouldCacheRequest: function(request)
    {
        request.QueryInterface(Ci.nsIHttpChannel);

        // Allow to customize caching rules.
        if (dispatch2(this.fbListeners, "shouldCacheRequest", [request]))
            return true;

        // Cache only text responses for now.
        var contentType = request.contentType;
        if (contentType)
            contentType = contentType.split(";")[0];

        contentType = trim(contentType);
        if (contentTypes[contentType])
            return true;

        // Hack to work around application/octet-stream for js files (2063).
        // Let's cache all files with js extensions.
        var extension = getFileExtension(safeGetName(request));
        if (extension == "js")
            return true;

        return false;
    },
});

// ************************************************************************************************

/**
 * This cache object is intended to cache all responses made by a specific tab.
 * The implementation is based on nsITraceableChannel interface introduced in
 * Firefox 3.0.4. This interface allows to intercept all incoming HTTP data.
 *
 * This object replaces the SourceCache, which still exist only for backward
 * compatibility.
 *
 * The object is derived from SourceCache so, the same interface and most of the
 * implementation is used.
 */
Firebug.TabCache = function(context)
{
    Firebug.SourceCache.call(this, context);
};

Firebug.TabCache.prototype = extend(Firebug.SourceCache.prototype,
{
    responses: [],       // responses in progress.

    storePartialResponse: function(request, responseText, win)
    {
        try
        {
            responseText = FBL.convertToUnicode(responseText, win.document.characterSet);
        }
        catch (err)
        {
        }

        var url = safeGetName(request);
        var response = this.getResponse(request);

        // Size of each response is limited.
        var limitNotReached = true;
        if (response.size + responseText.length >= responseSizeLimit)
        {
            limitNotReached = false;
            responseText = responseText.substr(0, responseSizeLimit - response.size);
            FBTrace.sysout("tabCache.storePartialResponse Max size limit reached for: " + url);
        }

        response.size += responseText.length;

        // Store partial content into the cache.
        this.store(url, responseText);

        // Return false if furhter parts of this response should be ignored.
        return limitNotReached;
    },

    getResponse: function(request)
    {
        var url = safeGetName(request);
        var response = this.responses[url];
        if (!response)
        {
            this.invalidate(url);
            this.responses[url] = response = {
                request: request,
                size: 0
            };
        }

        return response;
    },

    storeSplitLines: function(url, lines)
    {
        var currLines = this.cache[url];
        if (!currLines)
            currLines = this.cache[url] = [];

        // Join the last line with the new first one so, the source code
        // lines are properly formatted...
        if (currLines.length)
        {
            // ... but only if the last line isn't already completed.
            var lastLine = currLines[currLines.length-1];
            if (lastLine && lastLine.search(/\r|\n/) == -1)
                currLines[currLines.length-1] += lines.shift();
        }

        // Append new lines (if any) into the array for specified url.
        if (lines.length)
            this.cache[url] = currLines.concat(lines);

        return this.cache[url];
    },

    loadFromCache: function(url, method, file)
    {
        // The ancestor implementation (SourceCache) uses ioService.newChannel, which
        // can result in additional request to the server (in case the response can't
        // be loaded from the Firefox cache) - known as double-load problem.
        // This new implementation (TabCache) uses nsITraceableChannel so, all responses
        // should be already cached.

        // xxxHonza: TODO entire implementation of this method should be removed in Firebug 1.5
        // xxxHonza: let's try to get the response from the cache till #449198 is fixed.
        var stream;
        var responseText;
        try
        {
            if (!url)
                return responseText;

            var channel = ioService.newChannel(url, null, null);

            // These flag combination doesn't repost the request.
            channel.loadFlags = Ci.nsIRequest.LOAD_FROM_CACHE |
                Ci.nsICachingChannel.LOAD_ONLY_FROM_CACHE |
                Ci.nsICachingChannel.LOAD_BYPASS_LOCAL_CACHE_IF_BUSY;

            var charset = "UTF-8";
            var doc = this.context.window.document;
            if (doc)
                charset = doc.characterSet;

            stream = channel.open();

            // The response doesn't have to be in the browser cache.
            if (!stream.available())
            {
                stream.close();
                return [$STR("message.Failed to load source for") + ": " + url];
            }

            // Don't load responses that shouldn't be cached.
            if (!Firebug.TabCacheModel.shouldCacheRequest(channel))
            {
                stream.close();
                return [$STR("message.The resource from this URL is not text") + ": " + url];
            }

            responseText = readFromStream(stream, charset);

            responseText = this.store(url, responseText);
        }
        catch (err)
        {
        }
        finally
        {
            if (stream)
                stream.close();
        }

        return responseText;
    },

    // nsIStreamListener - callbacks from channel stream listener component.
    onStartRequest: function(request, requestContext)
    {
        var response = this.getResponse(request);

        dispatch(Firebug.TabCacheModel.fbListeners, "onStartRequest", [this.context, request]);
        dispatch(this.fbListeners, "onStartRequest", [this.context, request]);
    },

    onDataAvailable: function(request, requestContext, inputStream, offset, count)
    {
        var stream = {
            value: inputStream
        };

        dispatch(Firebug.TabCacheModel.fbListeners, "onDataAvailable",
            [this.context, request, requestContext, stream, offset, count]);
        dispatch(this.fbListeners, "onDataAvailable", [this.context,
            request, requestContext, stream, offset, count]);

        return stream.value;
    },

    onStopRequest: function(request, requestContext, statusCode)
    {
        // The response is finally received so, remove the request from the list of
        // current responses.
        var url = safeGetName(request);
        delete this.responses[url];

        var lines = this.cache[this.removeAnchor(url)];
        var responseText = lines ? lines.join("") : "";

        dispatch(Firebug.TabCacheModel.fbListeners, "onStopRequest", [this.context, request, responseText]);
        dispatch(this.fbListeners, "onStopRequest", [this.context, request, responseText]);
    }
});

// ************************************************************************************************
// Proxy Listener

function ChannelListenerProxy(win)
{
    this.wrappedJSObject = this;
    this.window = win;
}

ChannelListenerProxy.prototype =
{
    /* nsIStreamListener */
    onStartRequest: function(request, requestContext)
    {
        var context = this.getContext();
        if (context)
            context.sourceCache.onStartRequest(request, requestContext);
    },

    onDataAvailable: function(request, requestContext, inputStream, offset, count)
    {
        var context = this.getContext();
        if (!context)
            return null;

        return context.sourceCache.onDataAvailable(request, requestContext, inputStream, offset, count);
    },

    onStopRequest: function(request, requestContext, statusCode)
    {
        var context = this.getContext();
        if (context)
            context.sourceCache.onStopRequest(request, requestContext, statusCode);
    },

    /* nsISupports */
    QueryInterface: function(iid)
    {
        if (iid.equals(Ci.nsIStreamListener) ||
            iid.equals(Ci.nsISupportsWeakReference) ||
            iid.equals(Ci.nsISupports))
        {
            return this;
        }

        throw Components.results.NS_NOINTERFACE;
    },

    getContext: function()
    {
        try {
            return TabWatcher.getContextByWindow(this.window);
        } catch (e) {}
        return null;
    }
}

// ************************************************************************************************
// Helpers

function safeGetName(request)
{
    try {
        return request.name;
    }
    catch (exc) {
    }

    return null;
}

// ************************************************************************************************
// Registration

Firebug.registerModule(Firebug.TabCacheModel);

// ************************************************************************************************

}});
