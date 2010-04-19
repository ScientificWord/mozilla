/* See license.txt for terms of usage */

FBL.ns(function() { with (FBL) {

// ************************************************************************************************
// Constants

const Cc = Components.classes;
const Ci = Components.interfaces;

// List of contexts with XHR spy attached.
var contexts = [];

// ************************************************************************************************
// Spy Module

/**
 * @module Represents a XHR Spy module. The main purpose of the XHR Spy feature is to monitor
 * XHR activity of the current page and create appropriate log into the Console panel.
 * This feature can be controlled by an option <i>Show XMLHttpRequests</i> (from within the
 * console panel).
 *
 * The module is responsible for attaching/detaching a HTTP Observers when Firebug is
 * activated/deactivated for a site.
 */
Firebug.Spy = extend(Firebug.Module,
/** @lends Firebug.Spy */
{
    dispatchName: "spy",

    initialize: function()
    {
        if (Firebug.TraceModule)
            Firebug.TraceModule.addListener(this.TraceListener);

        Firebug.Module.initialize.apply(this, arguments);
    },

    shutdown: function()
    {
        Firebug.Module.shutdown.apply(this, arguments);

        if (Firebug.TraceModule)
            Firebug.TraceModule.removeListener(this.TraceListener);
    },

    initContext: function(context)
    {
        context.spies = [];

        if (Firebug.showXMLHttpRequests && Firebug.Console.isAlwaysEnabled())
            this.attachObserver(context, context.window);

    },

    destroyContext: function(context)
    {
        // For any spies that are in progress, remove our listeners so that they don't leak
        this.detachObserver(context, null);

        delete context.spies;

    },

    watchWindow: function(context, win)
    {
        if (Firebug.showXMLHttpRequests && Firebug.Console.isAlwaysEnabled())
            this.attachObserver(context, win);
    },

    unwatchWindow: function(context, win)
    {
        try
        {
            // This make sure that the existing context is properly removed from "contexts" array.
            this.detachObserver(context, win);
        }
        catch (ex)
        {
            // Get exceptions here sometimes, so let's just ignore them
            // since the window is going away anyhow
            ERROR(ex);
        }
    },

    updateOption: function(name, value)
    {
        // XXXjjb Honza, if Console.isEnabled(context) false, then this can't be called,
        // but somehow seems not correct
        if (name == "showXMLHttpRequests")
        {
            var tach = value ? this.attachObserver : this.detachObserver;
            for (var i = 0; i < TabWatcher.contexts.length; ++i)
            {
                var context = TabWatcher.contexts[i];
                iterateWindows(context.window, function(win)
                {
                    tach.apply(this, [context, win]);
                });
            }
        }
    },

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
    // Attaching Spy to XHR requests.

    /**
     * Returns false if Spy should not be attached to XHRs executed by the specified window.
     */
    skipSpy: function(win)
    {
        if (!win)
            return true;

        // Don't attach spy to chrome.
        var uri = safeGetWindowLocation(win);
        if (uri && (uri.indexOf("about:") == 0 || uri.indexOf("chrome:") == 0))
            return true;
    },

    attachObserver: function(context, win)
    {
        if (Firebug.Spy.skipSpy(win))
            return;

        for (var i=0; i<contexts.length; ++i)
        {
            if ((contexts[i].context == context) && (contexts[i].win == win))
                return;
        }

        // Register HTTP observers only once.
        if (contexts.length == 0)
        {
            httpObserver.addObserver(SpyHttpObserver, "firebug-http-event", false);
            SpyHttpActivityObserver.registerObserver();
        }

        contexts.push({context: context, win: win});

    },

    detachObserver: function(context, win)
    {
        for (var i=0; i<contexts.length; ++i)
        {
            if (contexts[i].context == context)
            {
                if (win && (contexts[i].win != win))
                    continue;

                contexts.splice(i, 1);

                // If no context is using spy, remvove the (only one) HTTP observer.
                if (contexts.length == 0)
                {
                    httpObserver.removeObserver(SpyHttpObserver, "firebug-http-event");
                    SpyHttpActivityObserver.unregisterObserver();
                }

                return;
            }
        }
    },

    /**
     * Return XHR object that is associated with specified request <i>nsIHttpChannel</i>.
     * Returns null if the request doesn't represent XHR.
     */
    getXHR: function(request)
    {
        // Does also query-interface for nsIHttpChannel.
        if (!(request instanceof Ci.nsIHttpChannel))
            return null;

        try
        {
            var callbacks = request.notificationCallbacks;
            return (callbacks ? callbacks.getInterface(Ci.nsIXMLHttpRequest) : null);
        }
        catch (exc)
        {
            if (exc.name == "NS_NOINTERFACE")
            {
            }
        }

       return null;
    },
});

// ************************************************************************************************

/**
 * @class This observer uses {@link HttpRequestObserver} to monitor start and end of all XHRs.
 * using <code>http-on-modify-request</code>, <code>http-on-examine-response</code> and
 * <code>http-on-examine-cached-response</code> events. For every monitored XHR a new
 * instance of {@link Firebug.Spy.XMLHttpRequestSpy} object is created. This instance is removed
 * when the XHR is finished.
 */
var SpyHttpObserver =
/** @lends SpyHttpObserver */
{
    observe: function(request, topic, data)
    {
        try
        {
            if (topic != "http-on-modify-request" &&
                topic != "http-on-examine-response" &&
                topic != "http-on-examine-cached-response")
            {
                return;
            }

            this.observeRequest(request, topic);
        }
        catch (exc)
        {
        }
    },

    observeRequest: function(request, topic)
    {
        var win = getWindowForRequest(request);
        var xhr = Firebug.Spy.getXHR(request);

        // The request must be associated with window (i.e. tab) and it also must be
        // real XHR request.
        if (!win || !xhr)
            return;

        for (var i=0; i<contexts.length; ++i)
        {
            var context = contexts[i];
            if (context.win == win)
            {
                var spyContext = context.context;
                var requestName = request.URI.asciiSpec;
                var requestMethod = request.requestMethod;

                if (topic == "http-on-modify-request")
                    this.requestStarted(request, xhr, spyContext, requestMethod, requestName);
                else if (topic == "http-on-examine-response")
                    this.requestStopped(request, xhr, spyContext, requestMethod, requestName);
                else if (topic == "http-on-examine-cached-response")
                    this.requestStopped(request, xhr, spyContext, requestMethod, requestName);

                return;
            }
        }
    },

    requestStarted: function(request, xhr, context, method, url)
    {
        var spy = getSpyForXHR(request, xhr, context);
        spy.method = method;
        spy.href = url;

        if (method == "POST" || method == "PUT")
            spy.postText = readPostTextFromRequest(request, context);

        spy.urlParams = parseURLParams(spy.href);

        // In case of redirects there is no stack and the source link is null.
        spy.sourceLink = getStackSourceLink();

        if (!spy.requestHeaders)
            spy.requestHeaders = getRequestHeaders(spy);

        // If it's enabled log the request into the console tab.
        if (Firebug.showXMLHttpRequests && Firebug.Console.isAlwaysEnabled())
        {
            spy.logRow = Firebug.Console.log(spy, spy.context, "spy", null, true);
            setClass(spy.logRow, "loading");
        }

        // Notify registered listeners. The onStart event is fired once for entire XHR
        // (even if there is more redirects within the process).
        var name = request.URI.asciiSpec;
        var origName = request.originalURI.asciiSpec;
        if (name == origName)
            dispatch(Firebug.Spy.fbListeners, "onStart", [context, spy]);

        // Remember the start time et the end, so it's most accurate.
        spy.sendTime = new Date().getTime();
    },

    requestStopped: function(request, xhr, context, method, url)
    {
        var spy = getSpyForXHR(request, xhr, context);
        if (!spy)
            return;

        spy.endTime = new Date().getTime();
        spy.responseTime = spy.endTime - spy.sendTime;
        spy.mimeType = Firebug.NetMonitor.Utils.getMimeType(request.contentType, request.name);

        if (!spy.responseHeaders)
            spy.responseHeaders = getResponseHeaders(spy);

        if (!spy.statusText)
        {
            try
            {
                spy.statusCode = request.responseStatus;
                spy.statusText = request.responseStatusText;
            }
            catch (exc)
            {
            }
        }

        if (spy.logRow)
        {
            updateLogRow(spy);
            updateHttpSpyInfo(spy);
        }

        // Remove only the Spy object that has been created for an intermediate rediret
        // request. These exist only to be also displayed in the console and they
        // don't attach any listeners to the original XHR object (which is always created
        // only once even in case of redirects).
        // xxxHonza: These requests are not observer by the activityObserver now
        // (if they should be observed we have to remove them in the activityObserver)
        if (!spy.onLoad && spy.context.spies)
            remove(spy.context.spies, spy);

    }
};

// ************************************************************************************************
// Activity Observer

/**
 * @class This observer is used to properly monitor even mulipart XHRs. It's based on
 * an activity-observer component that has been introduced in Firefox 3.6.
 */
var SpyHttpActivityObserver = extend(Firebug.NetMonitor.NetHttpActivityObserver,
/** @lends SpyHttpActivityObserver */
{
    activeRequests: [],

    observeRequest: function(request, activityType, activitySubtype, timestamp,
        extraSizeData, extraStringData)
    {
        if (activityType != Ci.nsIHttpActivityObserver.ACTIVITY_TYPE_HTTP_TRANSACTION &&
           (activityType == Ci.nsIHttpActivityObserver.ACTIVITY_TYPE_SOCKET_TRANSPORT &&
            activitySubtype != Ci.nsISocketTransport.STATUS_RECEIVING_FROM))
            return;

        var win = getWindowForRequest(request);
        if (!win)
        {
            var index = this.activeRequests.indexOf(request);
            if (!(win = this.activeRequests[index+1]))
                return;
        }

        for (var i=0; i<contexts.length; ++i)
        {
            var context = contexts[i];
            if (context.win == win)
            {
                var spyContext = context.context;
                var spy = getSpyForXHR(request, null, spyContext, true);
                if (spy)
                    this.observeXHRActivity(win, spy, request, activitySubtype, timestamp);
                return;
            }
        }
    },

    observeXHRActivity: function(win, spy, request, activitySubtype, timestamp)
    {
        // Activity observer has precise time info so, use it.
        var time = new Date();
        time.setTime(timestamp/1000);

        if (activitySubtype == Ci.nsIHttpActivityObserver.ACTIVITY_SUBTYPE_REQUEST_HEADER)
        {
            this.activeRequests.push(request);
            this.activeRequests.push(win);

            spy.sendTime = time;
            spy.transactionStarted = true;
        }
        else if (activitySubtype == Ci.nsIHttpActivityObserver.ACTIVITY_SUBTYPE_TRANSACTION_CLOSE)
        {
            var index = this.activeRequests.indexOf(request);
            this.activeRequests.splice(index, 2);

            spy.endTime = time;
            spy.transactionClosed = true;

            // This should be the proper time to detach the Spy object, but only
            // in the case when the XHR is already loaded. If the XHR is made as part of the
            // page load, it may happen that the event (readyState == 4) comes later
            // than actual TRANSACTION_CLOSE.
            if (spy.loaded)
                spy.detach();
        }
        else if (activitySubtype == Ci.nsISocketTransport.STATUS_RECEIVING_FROM)
        {
            spy.endTime = time;
        }
    }
});

// ************************************************************************************************

function getSpyForXHR(request, xhrRequest, context, noCreate)
{
    var spy = null;

    // Iterate all existing spy objects in this context and look for one that is
    // already created for this request.
    var length = context.spies.length;
    for (var i=0; i<length; i++)
    {
        spy = context.spies[i];
        if (spy.request == request)
            return spy;
    }

    if (noCreate)
        return null;

    spy = new Firebug.Spy.XMLHttpRequestSpy(request, xhrRequest, context);
    context.spies.push(spy);

    var name = request.URI.asciiSpec;
    var origName = request.originalURI.asciiSpec;

    // Attach spy only to the original request. Notice that there can be more network requests
    // made by the same XHR if redirects are involved.
    if (name == origName)
        spy.attach();

    return spy;
}

// ************************************************************************************************

/**
 * @class This class represents a Spy object that is attached to XHR. This object
 * registers various listeners into the XHR in order to monitor various events fired
 * during the request process (onLoad, onAbort, etc.)
 */
Firebug.Spy.XMLHttpRequestSpy = function(request, xhrRequest, context)
{
    this.request = request;
    this.xhrRequest = xhrRequest;
    this.context = context;
    this.responseText = "";

    // For compatibility with the Net templates.
    this.isXHR = true;

    // Support for activity-observer
    this.transactionStarted = false;
    this.transactionClosed = false;
};

Firebug.Spy.XMLHttpRequestSpy.prototype =
/** @lends Firebug.Spy.XMLHttpRequestSpy */
{
    attach: function()
    {
        var spy = this;
        this.onReadyStateChange = function(event) { onHTTPSpyReadyStateChange(spy, event); };
        this.onLoad = function() { onHTTPSpyLoad(spy); };
        this.onError = function() { onHTTPSpyError(spy); };
        this.onAbort = function() { onHTTPSpyAbort(spy); };

        // xxxHonza: #502959 is still failing on Fx 3.5
        // Use activity distributor to identify 3.6
        if (SpyHttpActivityObserver.getActivityDistributor())
        {
            this.onreadystatechange = this.xhrRequest.onreadystatechange;
            this.xhrRequest.onreadystatechange = this.onReadyStateChange;
        }

        this.xhrRequest.addEventListener("load", this.onLoad, false);
        this.xhrRequest.addEventListener("error", this.onError, false);
        this.xhrRequest.addEventListener("abort", this.onAbort, false);

        // xxxHonza: should be removed from FB 3.6
        if (!SpyHttpActivityObserver.getActivityDistributor())
            this.context.sourceCache.addListener(this);
    },

    detach: function()
    {
        // Bubble out if already detached.
        if (!this.onLoad)
            return;

        // If the activity distributor is available, let's detach it when the XHR
        // transaction is closed. Since, in case of multipart XHRs the onLoad method
        // (readyState == 4) can be called mutliple times.
        // Keep in mind:
        // 1) It can happen that that the TRANSACTION_CLOSE event comes before
        // the onLoad (if the XHR is made as part of the page load) so, detach if
        // it's already closed.
        // 2) In case of immediate cache responses, the transaction doesn't have to
        // be started at all (or the activity observer is no available in Firefox 3.5).
        // So, also detach in this case.
        if (this.transactionStarted && !this.transactionClosed)
            return;

        remove(this.context.spies, this);

        if (this.onreadystatechange)
            this.xhrRequest.onreadystatechange = this.onreadystatechange;

        try { this.xhrRequest.removeEventListener("load", this.onLoad, false); } catch (e) {}
        try { this.xhrRequest.removeEventListener("error", this.onError, false); } catch (e) {}
        try { this.xhrRequest.removeEventListener("abort", this.onAbort, false); } catch (e) {}

        this.onreadystatechange = null;
        this.onLoad = null;
        this.onError = null;
        this.onAbort = null;

        // xxxHonza: shouuld be removed from FB 1.6
        if (!SpyHttpActivityObserver.getActivityDistributor())
            this.context.sourceCache.removeListener(this);
    },

    getURL: function()
    {
        return this.xhrRequest.channel ? this.xhrRequest.channel.name : this.href;
    },

    // Cache listener
    onStopRequest: function(context, request, responseText)
    {
        if (!responseText)
            return;

        if (request == this.request)
            this.responseText = responseText;
    },
};

// ************************************************************************************************

function onHTTPSpyReadyStateChange(spy, event)
{
    var originalHandler = spy.onreadystatechange;

    // Force response text to be updated in the UI (in case the console entry
    // has been already expanded and the response tab selected).
    if (spy.logRow && spy.xhrRequest.readyState >= 3)
    {
        var netInfoBox = getChildByClass(spy.logRow, "spyHead", "netInfoBody");
        if (netInfoBox)
        {
            netInfoBox.htmlPresented = false;
            netInfoBox.responsePresented = false;
        }
    }

    // If the request is loading update the end time.
    if (spy.xhrRequest.readyState == 3)
    {
        spy.responseTime = spy.endTime - spy.sendTime;
        updateTime(spy);
    }

    // Request loaded. Get all the info from the request now, just in case the
    // XHR would be aborted in the original onReadyStateChange handler.
    if (spy.xhrRequest.readyState == 4)
    {
        // Cumulate response so, multipart response content is properly displayed.
        if (SpyHttpActivityObserver.getActivityDistributor())
            spy.responseText += spy.xhrRequest.responseText;
        else
        {
            // xxxHonza: remove from FB 1.6
            if (!spy.responseText)
                spy.responseText = spy.xhrRequest.responseText;
        }

        // The XHR is loaded now (used also by the activity observer).
        spy.loaded = true;

        // Update UI.
        updateHttpSpyInfo(spy);

        // Notify Net pane about a request beeing loaded.
        // xxxHonza: I don't think this is necessary.
        var netProgress = spy.context.netProgress;
        if (netProgress)
            netProgress.post(netProgress.stopFile, [spy.request, spy.endTime, spy.postText, spy.responseText]);

        // Notify registered listeners about finish of the XHR.
        dispatch(Firebug.Spy.fbListeners, "onLoad", [spy.context, spy]);
    }

    // Pass the event to the original page handler.
    callPageHandler(spy, event, originalHandler);
}

function onHTTPSpyLoad(spy)
{
    spy.detach();

    // xxxHonza: Still needed for Fx 3.5 (#502959)
    if (!SpyHttpActivityObserver.getActivityDistributor())
        onHTTPSpyReadyStateChange(spy, null);

    // If the spy is not loaded yet (and so, the response was not cached), do it now.
    // This can happen since synchronous XHRs don't fire onReadyStateChange event (issue 2868).
    if (!spy.loaded)
    {
        spy.loaded = true;
        spy.responseText = spy.xhrRequest.responseText;
    }
}

function onHTTPSpyError(spy)
{
    spy.detach();
    spy.loaded = true;

    if (spy.logRow)
    {
        removeClass(spy.logRow, "loading");
        setClass(spy.logRow, "error");
    }
}

function onHTTPSpyAbort(spy)
{
    spy.detach();
    spy.loaded = true;

    // Ignore aborts if the request already has a response status.
    if (spy.xhrRequest.status)
        return;

    if (spy.logRow)
    {
        removeClass(spy.logRow, "loading");
        setClass(spy.logRow, "error");
    }

    spy.statusText = "Aborted";
    updateLogRow(spy);

    // Notify Net pane about a request beeing aborted.
    // xxxHonza: the net panel shoud find out this itself.
    var netProgress = spy.context.netProgress;
    if (netProgress)
        netProgress.post(netProgress.abortFile, [spy.request, spy.endTime, spy.postText, spy.responseText]);
}

// ************************************************************************************************

function callPageHandler(spy, event, originalHandler)
{
    try
    {
        // Calling the page handler throwed an exception (see #502959)
        // This should be fixed in Firefox 3.5
        if (originalHandler && event)
            originalHandler.handleEvent(event);
    }
    catch (exc)
    {
        var error = Firebug.Errors.reparseXPC(exc, spy.context);
        if (error)
        {
            // TODO attach trace
            throw new Error(error.message, error.href, error.lineNo);
        }
    }
}

// ************************************************************************************************

/**
 * @domplate Represents a template for XHRs logged in the Console panel. The body of the
 * log (displayed when expanded) is rendered using {@link Firebug.NetMonitor.NetInfoBody}.
 */
Firebug.Spy.XHR = domplate(Firebug.Rep,
/** @lends Firebug.Spy.XHR */
{
    tag:
        DIV({"class": "spyHead", _repObject: "$object"},
            TABLE({"class": "spyHeadTable focusRow outerFocusRow", cellpadding: 0, cellspacing: 0,
                "role": "listitem", "aria-expanded": "false"},
                TBODY({"role": "presentation"},
                    TR({"class": "spyRow"},
                        TD({"class": "spyTitleCol spyCol", onclick: "$onToggleBody"},
                            DIV({"class": "spyTitle"},
                                "$object|getCaption"
                            ),
                            DIV({"class": "spyFullTitle spyTitle"},
                                "$object|getFullUri"
                            )
                        ),
                        TD({"class": "spyCol"},
                            DIV({"class": "spyStatus"}, "$object|getStatus")
                        ),
                        TD({"class": "spyCol"},
                            IMG({"class": "spyIcon", src: "blank.gif"})
                        ),
                        TD({"class": "spyCol"},
                            SPAN({"class": "spyTime"})
                        ),
                        TD({"class": "spyCol"},
                            TAG(FirebugReps.SourceLink.tag, {object: "$object.sourceLink"})
                        )
                    )
                )
            )
        ),

    getCaption: function(spy)
    {
        return spy.method.toUpperCase() + " " + cropString(spy.getURL(), 100);
    },

    getFullUri: function(spy)
    {
        return spy.method.toUpperCase() + " " + spy.getURL();
    },

    getStatus: function(spy)
    {
        var text = "";
        if (spy.statusCode)
            text += spy.statusCode + " ";

        if (spy.statusText)
            return text += spy.statusText;

        return text;
    },

    onToggleBody: function(event)
    {
        var target = event.currentTarget;
        var logRow = getAncestorByClass(target, "logRow-spy");

        if (isLeftClick(event))
        {
            toggleClass(logRow, "opened");

            var spy = getChildByClass(logRow, "spyHead").repObject;
            var spyHeadTable = getAncestorByClass(target, "spyHeadTable");

            if (hasClass(logRow, "opened"))
            {
                updateHttpSpyInfo(spy);
                if (spyHeadTable)
                    spyHeadTable.setAttribute('aria-expanded', 'true');
            }
            else
            {
                var netInfoBox = getChildByClass(spy.logRow, "spyHead", "netInfoBody");
                dispatch(Firebug.NetMonitor.NetInfoBody.fbListeners, "destroyTabBody", [netInfoBox, spy]);
                if (spyHeadTable)
                    spyHeadTable.setAttribute('aria-expanded', 'false');
            }
        }
    },

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    copyURL: function(spy)
    {
        copyToClipboard(spy.getURL());
    },

    copyParams: function(spy)
    {
        var text = spy.postText;
        if (!text)
            return;

        var url = reEncodeURL(spy, text, true);
        copyToClipboard(url);
    },

    copyResponse: function(spy)
    {
        copyToClipboard(spy.responseText);
    },

    openInTab: function(spy)
    {
        openNewTab(spy.getURL(), spy.postText);
    },

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    supportsObject: function(object)
    {
        return object instanceof Firebug.Spy.XMLHttpRequestSpy;
    },

    browseObject: function(spy, context)
    {
        var url = spy.getURL();
        openNewTab(url);
        return true;
    },

    getRealObject: function(spy, context)
    {
        return spy.xhrRequest;
    },

    getContextMenuItems: function(spy)
    {
        var items = [
            {label: "CopyLocation", command: bindFixed(this.copyURL, this, spy) }
        ];

        if (spy.postText)
        {
            items.push(
                {label: "CopyLocationParameters", command: bindFixed(this.copyParams, this, spy) }
            );
        }

        items.push(
            {label: "CopyResponse", command: bindFixed(this.copyResponse, this, spy) },
            "-",
            {label: "OpenInTab", command: bindFixed(this.openInTab, this, spy) }
        );

        return items;
    }
});

// ************************************************************************************************

Firebug.XHRSpyListener =
{
    onStart: function(context, spy)
    {
    },

    onLoad: function(context, spy)
    {
    }
};

// ************************************************************************************************

function updateTime(spy)
{
    var timeBox = spy.logRow.getElementsByClassName("spyTime").item(0);
    if (spy.responseTime)
        timeBox.textContent = " " + formatTime(spy.responseTime);
}

function updateLogRow(spy)
{
    updateTime(spy);

    var statusBox = spy.logRow.getElementsByClassName("spyStatus").item(0);
    statusBox.textContent = Firebug.Spy.XHR.getStatus(spy);

    removeClass(spy.logRow, "loading");
    setClass(spy.logRow, "loaded");

    try
    {
        var errorRange = Math.floor(spy.xhrRequest.status/100);
        if (errorRange == 4 || errorRange == 5)
            setClass(spy.logRow, "error");
    }
    catch (exc)
    {
    }
}

function updateHttpSpyInfo(spy)
{
    if (!spy.logRow || !hasClass(spy.logRow, "opened"))
        return;

    if (!spy.params)
        spy.params = parseURLParams(spy.href+"");

    if (!spy.requestHeaders)
        spy.requestHeaders = getRequestHeaders(spy);

    if (!spy.responseHeaders && spy.loaded)
        spy.responseHeaders = getResponseHeaders(spy);

    var template = Firebug.NetMonitor.NetInfoBody;
    var netInfoBox = getChildByClass(spy.logRow, "spyHead", "netInfoBody");
    if (!netInfoBox)
    {
        var head = getChildByClass(spy.logRow, "spyHead");
        netInfoBox = template.tag.append({"file": spy}, head);
        dispatch(template.fbListeners, "initTabBody", [netInfoBox, spy]);
        template.selectTabByName(netInfoBox, "Response");
    }
    else
    {
        template.updateInfo(netInfoBox, spy, spy.context);
    }
}

// ************************************************************************************************

function getRequestHeaders(spy)
{
    var headers = [];

    var channel = spy.xhrRequest.channel;
    if (channel instanceof Ci.nsIHttpChannel)
    {
        channel.visitRequestHeaders({
            visitHeader: function(name, value)
            {
                headers.push({name: name, value: value});
            }
        });
    }

    return headers;
}

function getResponseHeaders(spy)
{
    var headers = [];

    try
    {
        var channel = spy.xhrRequest.channel;
        if (channel instanceof Ci.nsIHttpChannel)
        {
            channel.visitResponseHeaders({
                visitHeader: function(name, value)
                {
                    headers.push({name: name, value: value});
                }
            });
        }
    }
    catch (exc)
    {
    }

    return headers;
}

// ************************************************************************************************
// Tracing Listener

Firebug.Spy.TraceListener =
{
    onDump: function(message)
    {
        var prefix = "spy.";
        var index = message.text.indexOf(prefix);
        if (index == 0)
        {
            message.text = message.text.substr(prefix.length);
            message.text = trim(message.text);
            message.type = "DBG_SPY";
        }
    }
};

// ************************************************************************************************
// Registration

Firebug.registerModule(Firebug.Spy);
Firebug.registerRep(Firebug.Spy.XHR);

// ************************************************************************************************
}});
