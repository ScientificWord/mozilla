/* See license.txt for terms of usage */

FBL.ns(function() { with (FBL) {

// ************************************************************************************************
// Constants

const Cc = Components.classes;
const Ci = Components.interfaces;
const Cr = Components.results;

const CacheService = Cc["@mozilla.org/network/cache-service;1"];
const ImgCache = Cc["@mozilla.org/image/cache;1"];
const IOService = Cc["@mozilla.org/network/io-service;1"];
const prefs = Cc["@mozilla.org/preferences-service;1"].getService(Ci.nsIPrefBranch2);

const NOTIFY_ALL = Ci.nsIWebProgress.NOTIFY_ALL;

const nsIWebProgressListener = Ci.nsIWebProgressListener;
const STATE_IS_WINDOW = nsIWebProgressListener.STATE_IS_WINDOW;
const STATE_IS_DOCUMENT = nsIWebProgressListener.STATE_IS_DOCUMENT;
const STATE_IS_NETWORK = nsIWebProgressListener.STATE_IS_NETWORK;
const STATE_IS_REQUEST = nsIWebProgressListener.STATE_IS_REQUEST;
const STATE_START = nsIWebProgressListener.STATE_START;
const STATE_STOP = nsIWebProgressListener.STATE_STOP;
const STATE_TRANSFERRING = nsIWebProgressListener.STATE_TRANSFERRING;

const LOAD_BACKGROUND = Ci.nsIRequest.LOAD_BACKGROUND;
const LOAD_FROM_CACHE = Ci.nsIRequest.LOAD_FROM_CACHE;
const LOAD_DOCUMENT_URI = Ci.nsIChannel.LOAD_DOCUMENT_URI;

const NS_ERROR_CACHE_KEY_NOT_FOUND = 0x804B003D;
const NS_ERROR_CACHE_WAIT_FOR_VALIDATION = 0x804B0040;

var nsIHttpActivityObserver = Ci.nsIHttpActivityObserver;
var nsIHttpActivityObserver = Ci.nsIHttpActivityObserver;
var nsISocketTransport = Ci.nsISocketTransport;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

const reIgnore = /about:|javascript:|resource:|chrome:|jar:/;
const reResponseStatus = /HTTP\/1\.\d\s(\d+)\s(.*)/;
const layoutInterval = 300;
const indentWidth = 18;

var cacheSession = null;
var contexts = new Array();
var panelName = "net";
var maxQueueRequests = 500;
var panelBar1 = $("fbPanelBar1");
var activeRequests = [];

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

const mimeExtensionMap =
{
    "txt": "text/plain",
    "html": "text/html",
    "htm": "text/html",
    "xhtml": "text/html",
    "xml": "text/xml",
    "css": "text/css",
    "js": "application/x-javascript",
    "jss": "application/x-javascript",
    "jpg": "image/jpg",
    "jpeg": "image/jpeg",
    "gif": "image/gif",
    "png": "image/png",
    "bmp": "image/bmp",
    "swf": "application/x-shockwave-flash",
    "flv": "video/x-flv"
};

const fileCategories =
{
    "undefined": 1,
    "html": 1,
    "css": 1,
    "js": 1,
    "xhr": 1,
    "image": 1,
    "flash": 1,
    "txt": 1,
    "bin": 1
};

const textFileCategories =
{
    "txt": 1,
    "html": 1,
    "xhr": 1,
    "css": 1,
    "js": 1
};

const binaryFileCategories =
{
    "bin": 1,
    "flash": 1
};

const mimeCategoryMap =
{
    "text/plain": "txt",
    "application/octet-stream": "bin",
    "text/html": "html",
    "text/xml": "html",
    "application/xhtml+xml": "html",
    "text/css": "css",
    "application/x-javascript": "js",
    "text/javascript": "js",
    "application/javascript" : "js",
    "text/ecmascript": "js",
    "application/ecmascript" : "js", // RFC4329
    "image/jpeg": "image",
    "image/jpg": "image",
    "image/gif": "image",
    "image/png": "image",
    "image/bmp": "image",
    "application/x-shockwave-flash": "flash",
    "video/x-flv": "flash"
};

const binaryCategoryMap =
{
    "image": 1,
    "flash" : 1
};

// ************************************************************************************************

/**
 * @module Represents a module object for the Net panel. This object is derived
 * from <code>Firebug.ActivableModule</code> in order to support activation (enable/disable).
 * This allows to avoid (performance) expensive features if the functionality is not necessary
 * for the user.
 */
Firebug.NetMonitor = extend(Firebug.ActivableModule,
{
    dispatchName: "netMonitor",
    clear: function(context)
    {
        // The user pressed a Clear button so, remove content of the panel...
        var panel = context.getPanel(panelName, true);
        if (panel)
            panel.clear();
    },

    onToggleFilter: function(context, filterCategory)
    {
        if (!context.netProgress)
            return;

        Firebug.setPref(Firebug.prefDomain, "netFilterCategory", filterCategory);

        // The content filter has been changed. Make sure that the content
        // of the panel is updated (CSS is used to hide or show individual files).
        var panel = context.getPanel(panelName, true);
        if (panel)
        {
            panel.setFilter(filterCategory);
            panel.updateSummaries(now(), true);
        }
    },

    syncFilterButtons: function(chrome)
    {
        var button = chrome.$("fbNetFilter-" + Firebug.netFilterCategory);
        button.checked = true;
    },

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
    // extends Module

    initializeUI: function()
    {
        Firebug.ActivableModule.initializeUI.apply(this, arguments);

        // Initialize max limit for logged requests.
        NetLimit.updateMaxLimit();

        // Synchronize UI buttons with the current filter.
        this.syncFilterButtons(FirebugChrome);

        prefs.addObserver(Firebug.prefDomain, NetLimit, false);
    },

    initialize: function()
    {
        this.panelName = panelName;

        Firebug.ActivableModule.initialize.apply(this, arguments);

        if (Firebug.TraceModule)
            Firebug.TraceModule.addListener(this.TraceListener);

        // HTTP observer must be registered now (and not in monitorContext, since if a
        // page is opened in a new tab the top document request would be missed otherwise.
        Firebug.NetMonitor.NetHttpObserver.registerObserver();
        NetHttpActivityObserver.registerObserver();

        Firebug.Debugger.addListener(this.DebuggerListener);
    },

    internationalizeUI: function(doc)
    {
        var element = doc.getElementById("fbNetPersist");
        FBL.internationalize(element, "label");
        FBL.internationalize(element, "tooltiptext");
    },

    shutdown: function()
    {
        prefs.removeObserver(Firebug.prefDomain, this, false);
        if (Firebug.TraceModule)
            Firebug.TraceModule.removeListener(this.TraceListener);

        Firebug.NetMonitor.NetHttpObserver.unregisterObserver();
        NetHttpActivityObserver.unregisterObserver();

        Firebug.Debugger.removeListener(this.DebuggerListener);
    },

    initContext: function(context, persistedState)
    {
        Firebug.ActivableModule.initContext.apply(this, arguments);

        if (context.window && 'addEventListener' in context.window)
        {
            var window = context.window;

            // Register "load" listener in order to track window load time.
            var onWindowLoadHandler = function() {
                if (context.netProgress)
                    context.netProgress.post(windowLoad, [window, now()]);
                window.removeEventListener("load", onWindowLoadHandler, true);
            }
            window.addEventListener("load", onWindowLoadHandler, true);

            // Register "DOMContentLoaded" listener to track timing.
            var onContentLoadHandler = function() {
                if (context.netProgress)
                    context.netProgress.post(contentLoad, [window, now()]);
                window.removeEventListener("DOMContentLoaded", onContentLoadHandler, true);
            }

            window.addEventListener("DOMContentLoaded", onContentLoadHandler, true);
        }

        if (Firebug.NetMonitor.isAlwaysEnabled())
            monitorContext(context);

        if (context.netProgress)
        {
            // Load existing breakpoints
            var persistedPanelState = getPersistedState(context, panelName);
            if (persistedPanelState.breakpoints)
                context.netProgress.breakpoints = persistedPanelState.breakpoints;
        }
    },

    reattachContext: function(browser, context)
    {
        Firebug.ActivableModule.reattachContext.apply(this, arguments);
        this.syncFilterButtons(Firebug.chrome);
    },

    destroyContext: function(context, persistedState)
    {
        Firebug.ActivableModule.destroyContext.apply(this, arguments);

        if (context.netProgress)
        {
            // Remember existing breakpoints.
            var persistedPanelState = getPersistedState(context, panelName);
            persistedPanelState.breakpoints = context.netProgress.breakpoints;
        }

        if (Firebug.NetMonitor.isAlwaysEnabled())
            unmonitorContext(context);
    },

    showContext: function(browser, context)
    {
        Firebug.ActivableModule.showContext.apply(this, arguments);

    },

    loadedContext: function(context)
    {
        var tabId = Firebug.getTabIdForWindow(context.browser.contentWindow);
        delete contexts[tabId];

        var netProgress = context.netProgress;
        if (netProgress)
        {
            netProgress.loaded = true;

            // Set Page title and id into all document objects.
            for (var i=0; i<netProgress.documents.length; i++)
            {
                var doc = netProgress.documents[i];
                doc.id = context.uid;
                doc.title = getPageTitle(context);
            }
        }
    },

    onEnabled: function(context)
    {
        NetHttpActivityObserver.registerObserver();

        monitorContext(context);
    },

    onDisabled: function(context)
    {
        NetHttpActivityObserver.unregisterObserver();

        unmonitorContext(context);
    },

    onResumeFirebug: function()
    {
        if (Firebug.NetMonitor.isAlwaysEnabled())
            TabWatcher.iterateContexts(monitorContext);
    },

    onSuspendFirebug: function()
    {
        if (Firebug.NetMonitor.isAlwaysEnabled())
            TabWatcher.iterateContexts(unmonitorContext);
    },

    togglePersist: function(context)
    {
        var panel = context.getPanel(panelName);
        panel.persistContent = panel.persistContent ? false : true;
        Firebug.chrome.setGlobalAttribute("cmd_togglePersistNet", "checked", panel.persistContent);
    }
});

// ************************************************************************************************

/**
 * @panel Represents a Firebug panel that displayes info about HTTP activity associated with
 * the current page. This class is derived from <code>Firebug.ActivablePanel</code> in order
 * to support activation (enable/disable). This allows to avoid (performance) expensive
 * features if the functionality is not necessary for the user.
 */
function NetPanel() {}
NetPanel.prototype = extend(Firebug.ActivablePanel,
{
    name: panelName,
    searchable: true,
    editable: true,
    breakable: true,

    initialize: function(context, doc)
    {
        this.queue = [];

        Firebug.registerUIListener(this);

        this.onContextMenu = bind(this.onContextMenu, this);

        Firebug.ActivablePanel.initialize.apply(this, arguments);
    },

    destroy: function(state)
    {
        Firebug.unregisterUIListener(this);

        Firebug.ActivablePanel.destroy.apply(this, arguments);
    },

    initializeNode : function()
    {
        this.panelNode.addEventListener("contextmenu", this.onContextMenu, false);

        dispatch([Firebug.A11yModel], "onInitializeNode", [this]);
    },

    destroyNode : function()
    {
        this.panelNode.removeEventListener("contextmenu", this.onContextMenu, false);

        dispatch([Firebug.A11yModel], "onDestroyNode", [this]);
    },

    loadPersistedContent: function(state)
    {
        this.initLayout();

        var tbody = this.table.firstChild;
        var lastRow = tbody.lastChild.previousSibling;

        // Move all net-rows from the persistedState to this panel.
        var prevTableBody = state.panelNode.getElementsByClassName("netTableBody").item(0);
        if (!prevTableBody)
            return;

        var files = [];

        while (prevTableBody.firstChild)
        {
            var row = prevTableBody.firstChild;
            if (hasClass(row, "netRow") && hasClass(row, "hasHeaders") && !hasClass(row, "history"))
            {
                row.repObject.history = true;
                files.push({
                    file: row.repObject,
                    offset: 0 + "%",
                    width: 0 + "%",
                    elapsed:  -1
                });
            }

            if (hasClass(row, "netPageRow"))
            {
                removeClass(row, "opened");
                tbody.insertBefore(row, lastRow);
            }
            else
                prevTableBody.removeChild(row);
        }

        if (files.length)
        {
            var pageRow = NetPage.pageTag.insertRows({page: state}, lastRow)[0];
            pageRow.files = files;

            lastRow = tbody.lastChild.previousSibling;
        }

        if (this.table.getElementsByClassName("netPageRow").item(0))
            NetPage.separatorTag.insertRows({}, lastRow);

        scrollToBottom(this.panelNode);
    },

    savePersistedContent: function(state)
    {
        Firebug.ActivablePanel.savePersistedContent.apply(this, arguments);

        state.pageTitle = getPageTitle(this.context);
    },

    // UI Listener
    showUI: function(browser, context)
    {
    },

    hideUI: function(browser, context)
    {
    },

    show: function(state)
    {
        var enabled = Firebug.NetMonitor.isAlwaysEnabled();
        this.showToolbarButtons("fbNetButtons", enabled);

        if (enabled)
        {
            Firebug.NetMonitor.disabledPanelPage.hide(this);
            Firebug.chrome.setGlobalAttribute("cmd_togglePersistNet", "checked", this.persistContent);
        }
        else
        {
            Firebug.NetMonitor.disabledPanelPage.show(this);
            this.table = null;
        }

        if (!enabled)
            return;

        if (!this.filterCategory)
            this.setFilter(Firebug.netFilterCategory);

        this.layout();

        if (!this.layoutInterval)
            this.layoutInterval = setInterval(bindFixed(this.updateLayout, this), layoutInterval);

        if (this.wasScrolledToBottom)
            scrollToBottom(this.panelNode);
    },

    hide: function()
    {
        this.showToolbarButtons("fbNetButtons", false);

        Firebug.Debugger.syncCommands(this.context);

        delete this.infoTipURL;  // clear the state that is tracking the infotip so it is reset after next show()
        this.wasScrolledToBottom = isScrolledToBottom(this.panelNode);

        clearInterval(this.layoutInterval);
        delete this.layoutInterval;
    },

    updateOption: function(name, value)
    {
        if (name == "netFilterCategory")
        {
            Firebug.NetMonitor.syncFilterButtons(Firebug.chrome);
            for (var i = 0; i < TabWatcher.contexts.length; ++i)
            {
                var context = TabWatcher.contexts[i];
                Firebug.NetMonitor.onToggleFilter(context, value);
            }
        }
    },

    updateSelection: function(object)
    {
        if (!object)
            return;

        var netProgress = this.context.netProgress;
        var file = netProgress.getRequestFile(object.request);
        if (!file)
        {
            for (var i=0; i<netProgress.requests.length; i++) {
                if (safeGetName(netProgress.requests[i]) == object.href) {
                   file = netProgress.files[i];
                   break;
                }
            }
        }

        if (file)
        {
            scrollIntoCenterView(file.row);
            if (!hasClass(file.row, "opened"))
                NetRequestEntry.toggleHeadersRow(file.row);
        }
    },

    getPopupObject: function(target)
    {
        var header = getAncestorByClass(target, "netHeaderRow");
        if (header)
            return NetRequestTable;

        return Firebug.ActivablePanel.getPopupObject.apply(this, arguments);
    },

    supportsObject: function(object)
    {
        return ((object instanceof SourceLink && object.type == "net") ? 2 : 0);
    },

    getOptionsMenuItems: function()
    {
        return [
            this.disableCacheOption()
        ];
    },

    disableCacheOption: function()
    {
        var BrowserCache = Firebug.NetMonitor.BrowserCache;
        var disabled = !BrowserCache.isEnabled();
        return {label: "net.option.Disable Browser Cache", type: "checkbox", checked: disabled,
            command: bindFixed(BrowserCache.enable, BrowserCache, disabled) };
    },

    getContextMenuItems: function(nada, target)
    {
        var items = [];

        var file = Firebug.getRepObject(target);
        if (!file || !(file instanceof NetFile))
            return items;

        var object = Firebug.getObjectByURL(this.context, file.href);
        var isPost = Utils.isURLEncodedRequest(file, this.context);

        items.push(
            {label: "CopyLocation", command: bindFixed(copyToClipboard, FBL, file.href) }
        );

        if (isPost)
        {
            items.push(
                {label: "CopyLocationParameters", command: bindFixed(this.copyParams, this, file) }
            );
        }

        items.push(
            {label: "CopyRequestHeaders",
                command: bindFixed(this.copyHeaders, this, file.requestHeaders) },
            {label: "CopyResponseHeaders",
                command: bindFixed(this.copyHeaders, this, file.responseHeaders) }
        );

        if (textFileCategories.hasOwnProperty(file.category))
        {
            items.push(
                {label: "CopyResponse", command: bindFixed(this.copyResponse, this, file) }
            );
        }

        items.push(
            "-",
            {label: "OpenInTab", command: bindFixed(this.openRequestInTab, this, file) }
        );

        if (textFileCategories.hasOwnProperty(file.category))
        {
            items.push(
                {label: "Open Response In New Tab", command: bindFixed(this.openResponseInTab, this, file) }
            );
        }

        if (!file.loaded)
        {
            items.push(
                "-",
                {label: "StopLoading", command: bindFixed(this.stopLoading, this, file) }
            );
        }

        if (object)
        {
            var subItems = Firebug.chrome.getInspectMenuItems(object);
            if (subItems.length)
            {
                items.push("-");
                items.push.apply(items, subItems);
            }
        }

        if (file.isXHR)
        {
            var bp = this.context.netProgress.breakpoints.findBreakpoint(file.getFileURL());

            items.push(
                "-",
                {label: "net.label.Break On XHR", type: "checkbox", checked: !!bp,
                    command: bindFixed(this.breakOnRequest, this, file) }
            );

            if (bp)
            {
                items.push(
                    {label: "EditBreakpointCondition",
                        command: bindFixed(this.editBreakpointCondition, this, file) }
                );
            }
        }

        return items;
    },

    // Context menu commands
    copyParams: function(file)
    {
        var text = Utils.getPostText(file, this.context, true);
        var url = reEncodeURL(file, text, true);
        copyToClipboard(url);
    },

    copyHeaders: function(headers)
    {
        var lines = [];
        if (headers)
        {
            for (var i = 0; i < headers.length; ++i)
            {
                var header = headers[i];
                lines.push(header.name + ": " + header.value);
            }
        }

        var text = lines.join("\r\n");
        copyToClipboard(text);
    },

    copyResponse: function(file)
    {
        var allowDoublePost = Firebug.getPref(Firebug.prefDomain, "allowDoublePost");
        if (!allowDoublePost && !file.cacheEntry)
        {
            if (!confirm("The response can be re-requested from the server, OK?"))
                return;
        }

        // Copy response to the clipboard
        copyToClipboard(Utils.getResponseText(file, this.context));
    },

    openRequestInTab: function(file)
    {
        openNewTab(file.href, file.postText);
    },

    openResponseInTab: function(file)
    {
        try
        {
            var response = Utils.getResponseText(file, this.context);
            var inputStream = getInputStreamFromString(response);
            var stream = CCIN("@mozilla.org/binaryinputstream;1", "nsIBinaryInputStream");
            stream.setInputStream(inputStream);
            var encodedResponse = btoa(stream.readBytes(stream.available()));
            var dataURI = "data:" + file.request.contentType + ";base64," + encodedResponse;
            gBrowser.selectedTab = gBrowser.addTab(dataURI);
        }
        catch (err)
        {
        }
    },

    breakOnRequest: function(file)
    {
        if (!file.isXHR)
            return;

        // Create new or remove an existing breakpoint.
        var breakpoints = this.context.netProgress.breakpoints;
        var url = file.getFileURL();
        var bp = breakpoints.findBreakpoint(url);
        if (bp)
            breakpoints.removeBreakpoint(url);
        else
            breakpoints.addBreakpoint(url);

        this.enumerateRequests(function(currFile)
        {
            if (url != currFile.getFileURL())
                return;

            if (bp)
                currFile.row.removeAttribute("breakpoint");
            else
                currFile.row.setAttribute("breakpoint", "true");
        })
    },

    stopLoading: function(file)
    {
        const NS_BINDING_ABORTED = 0x804b0002;

        file.request.cancel(NS_BINDING_ABORTED);
    },

    // Support for xhr breakpoint conditions.
    onContextMenu: function(event)
    {
        if (!hasClass(event.target, "sourceLine"))
            return;

        var row = getAncestorByClass(event.target, "netRow");
        if (!row)
            return;

        var file = row.repObject;
        var bp = this.context.netProgress.breakpoints.findBreakpoint(file.getFileURL());
        if (!bp)
            return;

        this.editBreakpointCondition(file);
        cancelEvent(event);
    },

    editBreakpointCondition: function(file)
    {
        var bp = this.context.netProgress.breakpoints.findBreakpoint(file.getFileURL());
        if (!bp)
            return;

        var condition = bp ? bp.condition : "";

        this.selectedSourceBox = this.panelNode;
        Firebug.Editor.startEditing(file.row, condition);
    },

    getEditor: function(target, value)
    {
        if (!this.conditionEditor)
            this.conditionEditor = new Firebug.NetMonitor.ConditionEditor(this.document);

        return this.conditionEditor;
    },

    // Support for activation.
    disablePanel: function(module)
    {
        Firebug.ActivablePanel.disablePanel.apply(this, arguments);
        this.table = null;
    },

    breakOnNext: function(breaking)
    {
        this.context.breakOnXHR = breaking;
    },

    shouldBreakOnNext: function()
    {
        return this.context.breakOnXHR;
    },

    getBreakOnNextTooltip: function(enabled)
    {
        return (enabled ? $STR("net.Disable Break On XHR") : $STR("net.Break On XHR"));
    },

    // Support for info tips.
    showInfoTip: function(infoTip, target, x, y)
    {
        var row = getAncestorByClass(target, "netRow");
        if (row && row.repObject)
        {
            if (getAncestorByClass(target, "netTotalSizeCol"))
            {
                var infoTipURL = "netTotalSize";
                if (infoTipURL == this.infoTipURL)
                    return true;

                this.infoTipURL = infoTipURL;
                return this.populateTotalSizeInfoTip(infoTip, row);
            }
            else if (getAncestorByClass(target, "netSizeCol"))
            {
                var infoTipURL = row.repObject.href + "-netsize";
                if (infoTipURL == this.infoTipURL && row.repObject == this.infoTipFile)
                    return true;

                this.infoTipURL = infoTipURL;
                this.infoTipFile = row.repObject;
                return this.populateSizeInfoTip(infoTip, row.repObject);
            }
            else if (getAncestorByClass(target, "netTimeCol"))
            {
                var infoTipURL = row.repObject.href + "-nettime";
                if (infoTipURL == this.infoTipURL && row.repObject == this.infoTipFile)
                    return true;

                this.infoTipURL = infoTipURL;
                this.infoTipFile = row.repObject;
                return this.populateTimeInfoTip(infoTip, row.repObject);
            }
            else if (hasClass(row, "category-image") &&
                !getAncestorByClass(target, "netRowHeader"))
            {
                var infoTipURL = row.repObject.href + "-image";
                if (infoTipURL == this.infoTipURL)
                    return true;

                this.infoTipURL = infoTipURL;
                return Firebug.InfoTip.populateImageInfoTip(infoTip, row.repObject.href);
            }
        }
    },

    populateTimeInfoTip: function(infoTip, file)
    {
        Firebug.NetMonitor.TimeInfoTip.render(file, infoTip);
        return true;
    },

    populateSizeInfoTip: function(infoTip, file)
    {
        Firebug.NetMonitor.SizeInfoTip.render(file, infoTip);
        return true;
    },

    populateTotalSizeInfoTip: function(infoTip, row)
    {
        var totalSizeLabel = row.getElementsByClassName("netTotalSizeLabel").item(0);
        var file = {size: totalSizeLabel.getAttribute("totalSize")};
        Firebug.NetMonitor.SizeInfoTip.tag.replace({file: file}, infoTip);
        return true;
    },

    // Support for search within the panel.
    getSearchOptionsMenuItems: function()
    {
        return [
            Firebug.Search.searchOptionMenu("search.Case Sensitive", "searchCaseSensitive"),
            //Firebug.Search.searchOptionMenu("search.net.Headers", "netSearchHeaders"),
            //Firebug.Search.searchOptionMenu("search.net.Parameters", "netSearchParameters"),
            Firebug.Search.searchOptionMenu("search.net.Response Bodies", "netSearchResponseBody")
        ];
    },

    search: function(text, reverse)
    {
        if (!text)
        {
            delete this.currentSearch;
            return false;
        }

        var row;
        if (this.currentSearch && text == this.currentSearch.text)
        {
            row = this.currentSearch.findNext(true, false, reverse, Firebug.Search.isCaseSensitive(text));
        }
        else
        {
            this.currentSearch = new NetPanelSearch(this);
            row = this.currentSearch.find(text, reverse, Firebug.Search.isCaseSensitive(text));
        }

        if (row)
        {
            var sel = this.document.defaultView.getSelection();
            sel.removeAllRanges();
            sel.addRange(this.currentSearch.range);

            scrollIntoCenterView(row, this.panelNode);
            dispatch([Firebug.A11yModel], 'onNetMatchFound', [this, text, row]);
            return true;
        }
        else
        {
            dispatch([Firebug.A11yModel], 'onNetMatchFound', [this, text, null]);
            return false;
        }
    },

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    updateFile: function(file)
    {
        if (!file.invalid)
        {
            file.invalid = true;
            this.queue.push(file);
        }
    },

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    updateLayout: function()
    {
        if (!this.queue.length)
            return;

        var rightNow = now();
        var length = this.queue.length;

        if (this.panelNode.offsetHeight)
            this.wasScrolledToBottom = isScrolledToBottom(this.panelNode);

        this.layout();

        if (this.wasScrolledToBottom)
            scrollToBottom(this.panelNode);

    },

    layout: function()
    {
        if (!this.queue.length || !this.context.netProgress ||
            !Firebug.NetMonitor.isAlwaysEnabled())
            return;

        this.initLayout();

        var rightNow = now();
        this.updateRowData(rightNow);
        this.updateLogLimit(maxQueueRequests);
        this.updateTimeline(rightNow);
        this.updateSummaries(rightNow);
    },

    initLayout: function()
    {
        if (!this.table)
        {
            var limitInfo = {
                totalCount: 0,
                limitPrefsTitle: $STRF("LimitPrefsTitle", [Firebug.prefDomain+".net.logLimit"])
            };

            this.table = NetRequestTable.tableTag.append({}, this.panelNode);
            this.limitRow = NetLimit.createRow(this.table.firstChild, limitInfo);
            this.summaryRow =  NetRequestEntry.summaryTag.insertRows({}, this.table.lastChild.lastChild)[0];

            // Update visibility of columns according to the preferences
            var hiddenCols = Firebug.getPref(Firebug.prefDomain, "net.hiddenColumns");
            if (hiddenCols)
                this.table.setAttribute("hiddenCols", hiddenCols);
        }
    },

    updateRowData: function(rightNow)
    {
        var queue = this.queue;
        this.queue = [];

        var phase;
        var newFileData = [];

        for (var i = 0; i < queue.length; ++i)
        {
            var file = queue[i];
            if (!file.phase)
              continue;

            file.invalid = false;

            phase = this.calculateFileTimes(file, phase, rightNow);

            this.updateFileRow(file, newFileData);
            this.invalidatePhase(phase);
        }

        if (newFileData.length)
        {
            var tbody = this.table.firstChild;
            var lastRow = tbody.lastChild.previousSibling;
            this.insertRows(newFileData, lastRow);
        }
    },

    insertRows: function(files, lastRow)
    {
        var row = NetRequestEntry.fileTag.insertRows({files: files}, lastRow)[0];

        for (var i = 0; i < files.length; ++i)
        {
            var file = files[i].file;
            row.repObject = file;
            file.row = row;

            // Make sure a breakpoint is displayed.
            var breakpoints = this.context.netProgress.breakpoints;
            if (breakpoints && breakpoints.findBreakpoint(file.getFileURL()))
                row.setAttribute("breakpoint", "true");

            // Allow customization of request entries in the list. A row is represented
            // by <TR> HTML element.
            dispatch(NetRequestTable.fbListeners, "onCreateRequestEntry", [this, row]);

            row = row.nextSibling;
        }
    },

    invalidatePhase: function(phase)
    {
        if (phase && !phase.invalidPhase)
        {
            phase.invalidPhase = true;
            this.invalidPhases = true;
        }
    },

    updateFileRow: function(file, newFileData)
    {
        var row = file.row;
        if (!row)
        {
            newFileData.push({
                file: file,
                offset: this.barOffset + "%",
                width: this.barReceivingWidth + "%",
                elapsed: file.loaded ? this.elapsed : -1
            });
        }
        else
        {
            var sizeLabel = row.childNodes[4].firstChild;

            var sizeText = NetRequestEntry.getSize(file);

            // Show also total downloaded size for requests in progress.
            if (file.totalReceived)
                sizeText += " (" + formatSize(file.totalReceived) + ")";

            sizeLabel.firstChild.nodeValue = sizeText;

            var methodLabel = row.childNodes[2].firstChild;
            methodLabel.firstChild.nodeValue = NetRequestEntry.getStatus(file);

            var hrefLabel = row.childNodes[1].firstChild;
            hrefLabel.firstChild.nodeValue = NetRequestEntry.getHref(file);

            if (file.mimeType)
            {
                // Force update category.
                file.category = null;
                for (var category in fileCategories)
                    removeClass(row, "category-" + category);
                setClass(row, "category-" + Utils.getFileCategory(file));
            }

            if (file.requestHeaders)
                setClass(row, "hasHeaders");

            if (file.fromCache)
                setClass(row, "fromCache");
            else
                removeClass(row, "fromCache");

            if (NetRequestEntry.isError(file))
                setClass(row, "responseError");
            else
                removeClass(row, "responseError");

            var timeLabel = row.childNodes[5].childNodes[1].lastChild.firstChild;
            timeLabel.innerHTML = NetRequestEntry.getElapsedTime({elapsed: this.elapsed});

            if (file.loaded)
                setClass(row, "loaded");
            else
                removeClass(row, "loaded");

            if (hasClass(row, "opened"))
            {
                var netInfoBox = row.nextSibling.getElementsByClassName("netInfoBody").item(0);
                NetInfoBody.updateInfo(netInfoBox, file, this.context);
            }
        }
    },

    updateTimeline: function(rightNow)
    {
        var tbody = this.table.firstChild;

        // XXXjoe Don't update rows whose phase is done and layed out already
        var phase;
        for (var row = tbody.firstChild; row; row = row.nextSibling)
        {
            var file = row.repObject;

            // Some rows aren't associated with a file (e.g. header, sumarry).
            if (!file)
                continue;

            if (!file.loaded)
                continue;

            phase = this.calculateFileTimes(file, phase, rightNow);

            // Get bar nodes
            var blockingBar = row.childNodes[5].childNodes[1].childNodes[1];
            var resolvingBar = blockingBar.nextSibling;
            var connectingBar = resolvingBar.nextSibling;
            var sendingBar = connectingBar.nextSibling;
            var waitingBar = sendingBar.nextSibling;
            var contentLoadBar = waitingBar.nextSibling;
            var windowLoadBar = contentLoadBar.nextSibling;
            var receivingBar = windowLoadBar.nextSibling;

            // All bars starts at the beginning
            resolvingBar.style.left = connectingBar.style.left = sendingBar.style.left =
                blockingBar.style.left =
                waitingBar.style.left = receivingBar.style.left = this.barOffset + "%";

            // Sets width of all bars (using style). The width is computed according to measured timing.
            blockingBar.style.width = this.barBlockingWidth + "%";
            resolvingBar.style.width = this.barResolvingWidth + "%";
            connectingBar.style.width = this.barConnectingWidth + "%";
            sendingBar.style.width = this.barSendingWidth + "%";
            waitingBar.style.width = this.barWaitingWidth + "%";
            receivingBar.style.width = this.barReceivingWidth + "%";

            if (this.contentLoadBarOffset) {
                contentLoadBar.style.left = this.contentLoadBarOffset + "%";
                contentLoadBar.style.display = "block";
                this.contentLoadBarOffset = null;
            }

            if (this.windowLoadBarOffset) {
                windowLoadBar.style.left = this.windowLoadBarOffset + "%";
                windowLoadBar.style.display = "block";
                this.windowLoadBarOffset = null;
            }
        }
    },

    calculateFileTimes: function(file, phase, rightNow)
    {
        var phases = this.context.netProgress.phases;

        if (phase != file.phase)
        {
            phase = file.phase;
            this.phaseStartTime = phase.startTime;
            this.phaseEndTime = phase.endTime ? phase.endTime : rightNow;

            // End of the first phase has to respect even the window "onload" event time, which
            // can occur after the last received file. This sets the extent of the timeline so,
            // the windowLoadBar is visible.
            if (phase.windowLoadTime && this.phaseEndTime < phase.windowLoadTime)
                this.phaseEndTime = phase.windowLoadTime;

            this.phaseElapsed = this.phaseEndTime - phase.startTime;
        }

        var elapsed = file.loaded ? file.endTime - file.startTime : 0; /*this.phaseEndTime - file.startTime*/
        this.barOffset = Math.floor(((file.startTime-this.phaseStartTime)/this.phaseElapsed) * 100);

        var blockingEnd = (file.sendingTime != file.startTime) ? file.sendingTime : file.waitingForTime;

        this.barResolvingWidth = Math.round(((file.connectingTime - file.startTime) / this.phaseElapsed) * 100);
        this.barConnectingWidth = Math.round(((file.connectedTime - file.startTime) / this.phaseElapsed) * 100);
        this.barBlockingWidth = Math.round(((blockingEnd - file.startTime) / this.phaseElapsed) * 100);
        this.barSendingWidth = Math.round(((file.waitingForTime - file.startTime) / this.phaseElapsed) * 100);
        this.barWaitingWidth = Math.round(((file.respondedTime - file.startTime) / this.phaseElapsed) * 100);
        this.barReceivingWidth = Math.round((elapsed / this.phaseElapsed) * 100);

        // Total request time doesn't include the time spent in queue.
        // xxxHonza: since all phases are now graphically distinguished it's easy to
        // see blocking requests. It's make sense to display the real total time now.
        this.elapsed = elapsed/* - (file.sendingTime - file.connectedTime)*/;

        // The nspr timer doesn't have 1ms precision, so it can happen that entire
        // request is executed in l ms (so the total is zero). Let's display at least
        // one bar in such a case so the timeline is visible.
        if (this.elapsed <= 0)
            this.barReceivingWidth = "1";

        // Compute also offset for the contentLoadBar and windowLoadBar, which are
        // displayed for the first phase.
        if (phase.contentLoadTime)
            this.contentLoadBarOffset = Math.floor(((phase.contentLoadTime-this.phaseStartTime)/this.phaseElapsed) * 100);

        if (phase.windowLoadTime)
            this.windowLoadBarOffset = Math.floor(((phase.windowLoadTime-this.phaseStartTime)/this.phaseElapsed) * 100);

        return phase;
    },

    updateSummaries: function(rightNow, updateAll)
    {
        if (!this.invalidPhases && !updateAll)
            return;

        this.invalidPhases = false;

        var phases = this.context.netProgress.phases;
        if (!phases.length)
            return;

        var fileCount = 0, totalSize = 0, cachedSize = 0, totalTime = 0;
        for (var i = 0; i < phases.length; ++i)
        {
            var phase = phases[i];
            phase.invalidPhase = false;

            var summary = this.summarizePhase(phase, rightNow);
            fileCount += summary.fileCount;
            totalSize += summary.totalSize;
            cachedSize += summary.cachedSize;
            totalTime += summary.totalTime
        }

        var row = this.summaryRow;
        if (!row)
            return;

        var countLabel = row.childNodes[1].firstChild;
        countLabel.firstChild.nodeValue = $STRP("plural.Request_Count", [fileCount]);

        var sizeLabel = row.childNodes[4].firstChild;
        sizeLabel.setAttribute("totalSize", totalSize);
        sizeLabel.firstChild.nodeValue = NetRequestEntry.formatSize(totalSize);

        var cacheSizeLabel = row.lastChild.firstChild.firstChild;
        cacheSizeLabel.setAttribute("collapsed", cachedSize == 0);
        cacheSizeLabel.childNodes[1].firstChild.nodeValue =
            NetRequestEntry.formatSize(cachedSize);

        var timeLabel = row.lastChild.firstChild.lastChild.firstChild;
        var timeText = NetRequestEntry.formatTime(totalTime);
        var firstPhase = phases[0];
        if (firstPhase.windowLoadTime)
        {
            var loadTime = firstPhase.windowLoadTime - firstPhase.startTime;
            // xxxHonza: localization?
            timeText += " (onload: " + NetRequestEntry.formatTime(loadTime) + ")";
        }

        timeLabel.innerHTML = timeText;
    },

    summarizePhase: function(phase, rightNow)
    {
        var cachedSize = 0, totalSize = 0;

        var category = Firebug.netFilterCategory;
        if (category == "all")
            category = null;

        var fileCount = 0;
        var minTime = 0, maxTime = 0;

        for (var i=0; i<phase.files.length; i++)
        {
            var file = phase.files[i];

            if (!category || file.category == category)
            {
                if (file.loaded)
                {
                    ++fileCount;

                    if (file.size > 0)
                    {
                        totalSize += file.size;
                        if (file.fromCache)
                            cachedSize += file.size;
                    }

                    if (!minTime || file.startTime < minTime)
                        minTime = file.startTime;
                    if (file.endTime > maxTime)
                        maxTime = file.endTime;
                }
            }
        }

        var totalTime = maxTime - minTime;
        return {cachedSize: cachedSize, totalSize: totalSize, totalTime: totalTime,
                fileCount: fileCount}
    },

    updateLogLimit: function(limit)
    {
        var netProgress = this.context.netProgress;

        if (!netProgress)  // XXXjjb Honza, please check, I guess we are getting here with the context not setup
        {
            return;
        }

        // Must be positive number;
        limit = Math.max(0, limit);

        var filesLength = netProgress.files.length;
        if (!filesLength || filesLength <= limit)
            return;

        // Remove old requests.
        var removeCount = Math.max(0, filesLength - limit);
        for (var i=0; i<removeCount; i++)
        {
            var file = netProgress.files[0];
            this.removeLogEntry(file);

            // Remove the file occurrence from the queue.
            for (var j=0; j<this.queue.length; j++)
            {
                if (this.queue[j] == file) {
                    this.queue.splice(j, 1);
                    j--;
                }
            }
        }
    },

    removeLogEntry: function(file, noInfo)
    {
        if (!this.removeFile(file))
            return;

        if (!this.table || !this.table.firstChild)
            return;

        if (file.row)
        {
            // The file is loaded and there is a row that has to be removed from the UI.
            var tbody = this.table.firstChild;
            clearDomplate(file.row);
            tbody.removeChild(file.row);
        }

        if (noInfo || !this.limitRow)
            return;

        this.limitRow.limitInfo.totalCount++;

        NetLimit.updateCounter(this.limitRow);

        //if (netProgress.currentPhase == file.phase)
        //  netProgress.currentPhase = null;
    },

    removeFile: function(file)
    {
        var netProgress = this.context.netProgress;
        var index = netProgress.files.indexOf(file);
        if (index == -1)
            return false;

        netProgress.files.splice(index, 1);
        netProgress.requests.splice(index, 1);

        // Don't forget to remove the phase whose last file has been removed.
        var phase = file.phase;

        // xxxHonza: This needs to be examined yet. Looks like the queue contains
        // requests from the previous page. When flushed the requestedFile isn't called
        // and the phase is not set.
        if (!phase)
            return true;

        phase.removeFile(file);
        if (!phase.files.length)
        {
            remove(netProgress.phases, phase);

            if (netProgress.currentPhase == phase)
                netProgress.currentPhase = null;
        }

        file.clear();

        return true;
    },

    insertActivationMessage: function()
    {
        if (!Firebug.NetMonitor.isAlwaysEnabled())
            return;

        // Make sure the basic structure of the table panel is there.
        this.initLayout();

        // Get the last request row before summary row.
        var tbody = this.table.firstChild;
        var lastRow = tbody.lastChild.previousSibling;

        // Insert an activation message (if the last row isn't the message already);
        if (hasClass(lastRow, "netActivationRow"))
            return;

        var message = NetRequestEntry.activationTag.insertRows({}, lastRow)[0];

    },

    enumerateRequests: function(fn)
    {
        if (!this.table)
            return;

        var rows = this.table.getElementsByClassName("netRow");
        for (var i=0; i<rows.length; i++)
        {
            var row = rows[i];
            var pageRow = hasClass(row, "netPageRow");

            if (hasClass(row, "collapsed") && !pageRow)
                continue;

            if (hasClass(row, "history"))
                continue;

            // Export also history. These requests can be collpased and so not visible.
            if (row.files)
            {
                for (var j=0; j<row.files.length; j++)
                    fn(row.files[j].file);
            }

            var file = Firebug.getRepObject(row);
            if (file)
                fn(file);
        }
    },

    setFilter: function(filterCategory)
    {
        this.filterCategory = filterCategory;

        var panelNode = this.panelNode;
        for (var category in fileCategories)
        {
            if (filterCategory != "all" && category != filterCategory)
                setClass(panelNode, "hideCategory-"+category);
            else
                removeClass(panelNode, "hideCategory-"+category);
        }
    },

    clear: function()
    {
        clearNode(this.panelNode);

        this.table = null;
        this.summaryRow = null;
        this.limitRow = null;

        this.queue = [];
        this.invalidPhases = false;

        if (this.context.netProgress)
            this.context.netProgress.clear();

    },
});

// ************************************************************************************************

/**
 * @domplate Represents a template that is used to render basic content of the net panel.
 */
Firebug.NetMonitor.NetRequestTable = domplate(Firebug.Rep, new Firebug.Listener(),
{
    inspectable: false,

    tableTag:

        TABLE({"class": "netTable", cellpadding: 0, cellspacing: 0, hiddenCols: "", "role": "treegrid"},
            TBODY({"class": "netTableBody", "role" : "presentation"},
                TR({"class": "netHeaderRow netRow focusRow outerFocusRow", onclick: "$onClickHeader", "role": "row"},
                    TD({id: "netBreakpointBar", width: "1%", "class": "netHeaderCell",
                        "role": "columnheader"},
                        "&nbsp;"
                    ),
                    TD({id: "netHrefCol", width: "18%", "class": "netHeaderCell alphaValue a11yFocus",
                        "role": "columnheader"},
                        DIV({"class": "netHeaderCellBox",
                        title: $STR("net.header.URL Tooltip")},
                        $STR("net.header.URL"))
                    ),
                    TD({id: "netStatusCol", width: "12%", "class": "netHeaderCell alphaValue a11yFocus",
                        "role": "columnheader"},
                        DIV({"class": "netHeaderCellBox",
                        title: $STR("net.header.Status Tooltip")},
                        $STR("net.header.Status"))
                    ),
                    TD({id: "netDomainCol", width: "12%", "class": "netHeaderCell alphaValue a11yFocus",
                        "role": "columnheader"},
                        DIV({"class": "netHeaderCellBox",
                        title: $STR("net.header.Domain Tooltip")},
                        $STR("net.header.Domain"))
                    ),
                    TD({id: "netSizeCol", width: "4%", "class": "netHeaderCell a11yFocus",
                        "role": "columnheader"},
                        DIV({"class": "netHeaderCellBox",
                        title: $STR("net.header.Size Tooltip")},
                        $STR("net.header.Size"))
                    ),
                    TD({id: "netTimeCol", width: "53%", "class": "netHeaderCell a11yFocus",
                        "role": "columnheader"},
                        DIV({"class": "netHeaderCellBox",
                        title: $STR("net.header.Timeline Tooltip")},
                        $STR("net.header.Timeline"))
                    )
                )
            )
        ),

    onClickHeader: function(event)
    {
        if (!isLeftClick(event) && !(event.type == "keypress" && event.keyCode == 13))
            return;

        var table = getAncestorByClass(event.target, "netTable");
        var column = getAncestorByClass(event.target, "netHeaderCell");
        this.sortColumn(table, column);
    },

    sortColumn: function(table, col, direction)
    {
        if (!col)
            return;

        var numerical = !hasClass(col, "alphaValue");

        var colIndex = 0;
        for (col = col.previousSibling; col; col = col.previousSibling)
            ++colIndex;

        // the first breakpoint bar column is not sortable.
        if (colIndex == 0)
            return;

        this.sort(table, colIndex, numerical, direction);
    },

    sort: function(table, colIndex, numerical, direction)
    {
        var tbody = table.lastChild;
        var headerRow = tbody.firstChild;

        // Remove class from the currently sorted column
        var headerSorted = getChildByClass(headerRow, "netHeaderSorted");
        removeClass(headerSorted, "netHeaderSorted");
        if (headerSorted)
            headerSorted.removeAttribute("aria-sort");

        // Mark new column as sorted.
        var header = headerRow.childNodes[colIndex];
        setClass(header, "netHeaderSorted");
        // If the column is already using required sort direction, bubble out.
        if ((direction == "desc" && header.sorted == 1) ||
            (direction == "asc" && header.sorted == -1))
            return;
        if (header)
            header.setAttribute("aria-sort", header.sorted === -1 ? "descending" : "ascending");
        var colID = header.getAttribute("id");

        var values = [];
        for (var row = tbody.childNodes[1]; row; row = row.nextSibling)
        {
            if (!row.repObject)
                continue;

            if (hasClass(row, "history"))
                continue;

            var cell = row.childNodes[colIndex];
            var value = numerical ? parseFloat(cell.textContent) : cell.textContent;

            if (colID == "netTimeCol")
                value = row.repObject.startTime;
            else if (colID == "netSizeCol")
                value = row.repObject.size;

            if (hasClass(row, "opened"))
            {
                var netInfoRow = row.nextSibling;
                values.push({row: row, value: value, info: netInfoRow});
                row = netInfoRow;
            }
            else
            {
                values.push({row: row, value: value});
            }
        }

        values.sort(function(a, b) { return a.value < b.value ? -1 : 1; });

        if ((header.sorted && header.sorted == 1) || (!header.sorted && direction == "asc"))
        {
            removeClass(header, "sortedDescending");
            setClass(header, "sortedAscending");
            header.sorted = -1;

            for (var i = 0; i < values.length; ++i)
            {
                tbody.appendChild(values[i].row);
                if (values[i].info)
                    tbody.appendChild(values[i].info);
            }
        }
        else
        {
            removeClass(header, "sortedAscending");
            setClass(header, "sortedDescending");

            header.sorted = 1;

            for (var i = values.length-1; i >= 0; --i)
            {
                tbody.appendChild(values[i].row);
                if (values[i].info)
                    tbody.appendChild(values[i].info);
            }
        }

        // Make sure the summary row is again at the end.
        var summaryRow = tbody.getElementsByClassName("netSummaryRow").item(0);
        tbody.appendChild(summaryRow);
    },

    supportsObject: function(object)
    {
        return (object == this);
    },

    /**
     * Provides menu items for header context menu.
     */
    getContextMenuItems: function(object, target, context)
    {
        var popup = $("fbContextMenu");
        if (popup.firstChild && popup.firstChild.getAttribute("command") == "cmd_copy")
            popup.removeChild(popup.firstChild);

        var items = [];

        // Iterate over all columns and create a menu item for each.
        var table = context.getPanel(panelName, true).table;
        var hiddenCols = table.getAttribute("hiddenCols");

        var lastVisibleIndex;
        var visibleColCount = 0;

        // Iterate all columns except of the first one for breakpoints.
        var header = getAncestorByClass(target, "netHeaderRow");
        var columns = cloneArray(header.childNodes);
        columns.shift();
        for (var i=0; i<columns.length; i++)
        {
            var column = columns[i];
            var visible = (hiddenCols.indexOf(column.id) == -1);

            items.push({
                label: column.textContent,
                type: "checkbox",
                checked: visible,
                nol10n: true,
                command: bindFixed(this.onShowColumn, this, context, column.id)
            });

            if (visible)
            {
                lastVisibleIndex = i;
                visibleColCount++;
            }
        }

        // If the last column is visible, disable its menu item.
        if (visibleColCount == 1)
            items[lastVisibleIndex].disabled = true;

        items.push("-");
        items.push({
            label: $STR("net.header.Reset_Header"),
            nol10n: true,
            command: bindFixed(this.onResetColumns, this, context)
        });

        return items;
    },

    onShowColumn: function(context, colId)
    {
        var table = context.getPanel(panelName, true).table;
        var hiddenCols = table.getAttribute("hiddenCols");

        // If the column is already presented in the list of hidden columns,
        // remove it, otherwise append.
        var index = hiddenCols.indexOf(colId);
        if (index >= 0)
        {
            table.setAttribute("hiddenCols", hiddenCols.substr(0,index-1) +
                hiddenCols.substr(index+colId.length));
        }
        else
        {
            table.setAttribute("hiddenCols", hiddenCols + " " + colId);
        }

        // Store current state into the preferences.
        Firebug.setPref(Firebug.prefDomain, "net.hiddenColumns", table.getAttribute("hiddenCols"));
    },

    onResetColumns: function(context)
    {
        var panel = context.getPanel(panelName, true);
        var header = panel.panelNode.getElementsByClassName("netHeaderRow").item(0);

        // Reset widths
        var columns = header.childNodes;
        for (var i=0; i<columns.length; i++)
        {
            var col = columns[i];
            if (col.style)
                col.style.width = "";
        }

        // Reset visibility. Only the Status column is hidden by default.
        panel.table.setAttribute("hiddenCols", "colStatus");
        Firebug.setPref(Firebug.prefDomain, "net.hiddenColumns", "colStatus");
    },
});

var NetRequestTable = Firebug.NetMonitor.NetRequestTable;

// ************************************************************************************************

/**
 * @domplate Represents a template that is used to render net panel entries.
 */
Firebug.NetMonitor.NetRequestEntry = domplate(Firebug.Rep, new Firebug.Listener(),
{
    fileTag:
        FOR("file", "$files",
            TR({"class": "netRow $file.file|getCategory focusRow outerFocusRow",
                onclick: "$onClick", "role": "row", "aria-expanded": "false",
                $hasHeaders: "$file.file|hasRequestHeaders",
                $history: "$file.file.history",
                $loaded: "$file.file.loaded",
                $responseError: "$file.file|isError",
                $fromCache: "$file.file.fromCache",
                $inFrame: "$file.file|getInFrame"},
                TD({"class": "netCol"},
                   DIV({"class": "sourceLine netRowHeader",
                   onclick: "$onClickRowHeader"},
                        "&nbsp;"
                   )
                ),
                TD({"class": "netHrefCol netCol a11yFocus", "role": "rowheader"},
                    DIV({"class": "netHrefLabel netLabel",
                         style: "margin-left: $file.file|getIndent\\px"},
                        "$file.file|getHref"
                    ),
                    DIV({"class": "netFullHrefLabel netHrefLabel",
                         style: "margin-left: $file.file|getIndent\\px"},
                        "$file.file.href"
                    )
                ),
                TD({"class": "netStatusCol netCol a11yFocus", "role": "gridcell"},
                    DIV({"class": "netStatusLabel netLabel"}, "$file.file|getStatus")
                ),
                TD({"class": "netDomainCol netCol a11yFocus", "role": "gridcell" },
                    DIV({"class": "netDomainLabel netLabel"}, "$file.file|getDomain")
                ),
                TD({"class": "netSizeCol netCol a11yFocus", "role": "gridcell", "aria-describedby": "fbNetSizeInfoTip"},
                    DIV({"class": "netSizeLabel netLabel"}, "$file.file|getSize")
                ),
                TD({"class": "netTimeCol netCol a11yFocus", "role": "gridcell", "aria-describedby": "fbNetTimeInfoTip"  },
                    DIV({"class": "netLoadingIcon"}),
                    DIV({"class": "netBar"},
                        "&nbsp;",
                        DIV({"class": "netBlockingBar", style: "left: $file.offset"}),
                        DIV({"class": "netResolvingBar", style: "left: $file.offset"}),
                        DIV({"class": "netConnectingBar", style: "left: $file.offset"}),
                        DIV({"class": "netSendingBar", style: "left: $file.offset"}),
                        DIV({"class": "netWaitingBar", style: "left: $file.offset"}),
                        DIV({"class": "netContentLoadBar", style: "left: $file.offset"}),
                        DIV({"class": "netWindowLoadBar", style: "left: $file.offset"}),
                        DIV({"class": "netReceivingBar", style: "left: $file.offset; width: $file.width"},
                            SPAN({"class": "netTimeLabel"}, "$file|getElapsedTime")
                        )
                    )
                )
            )
        ),

    headTag:
        TR({"class": "netHeadRow"},
            TD({"class": "netHeadCol", colspan: 6},
                DIV({"class": "netHeadLabel"}, "$doc.rootFile.href")
            )
        ),

    netInfoTag:
        TR({"class": "netInfoRow outerFocusRow", "role" : "row"},
            TD({"class": "sourceLine netRowHeader"}),
            TD({"class": "netInfoCol", colspan: 5, "role" : "gridcell"})
        ),

    activationTag:
        TR({"class": "netRow netActivationRow"},
            TD({"class": "netCol netActivationLabel", colspan: 6, "role": "status"},
                $STR("net.ActivationMessage")
            )
        ),

    summaryTag:
        TR({"class": "netRow netSummaryRow focusRow outerFocusRow", "role": "row", "aria-live": "polite"},
            TD({"class": "netCol"}, "&nbsp;"),
            TD({"class": "netCol netHrefCol a11yFocus", "role" : "rowheader"},
                DIV({"class": "netCountLabel netSummaryLabel"}, "-")
            ),
            TD({"class": "netCol netStatusCol a11yFocus", "role" : "gridcell"}),
            TD({"class": "netCol netDomainCol a11yFocus", "role" : "gridcell"}),
            TD({"class": "netTotalSizeCol netCol netSizeCol a11yFocus", "role" : "gridcell"},
                DIV({"class": "netTotalSizeLabel netSummaryLabel"}, "0KB")
            ),
            TD({"class": "netTotalTimeCol netCol netTimeCol a11yFocus", "role" : "gridcell"},
                DIV({"class": "netSummaryBar", style: "width: 100%"},
                    DIV({"class": "netCacheSizeLabel netSummaryLabel", collapsed: "true"},
                        "(",
                        SPAN("0KB"),
                        SPAN(" " + $STR("FromCache")),
                        ")"
                    ),
                    DIV({"class": "netTimeBar"},
                        SPAN({"class": "netTotalTimeLabel netSummaryLabel"}, "0ms")
                    )
                )
            )
        ),

    onClickRowHeader: function(event)
    {
        cancelEvent(event);

        var rowHeader = event.target;
        if (!hasClass(rowHeader, "netRowHeader"))
            return;

        var row = getAncestorByClass(event.target, "netRow");
        if (!row)
            return;

        var context = Firebug.getElementPanel(row).context;
        var panel = context.getPanel(panelName, true);
        if (panel)
            panel.breakOnRequest(row.repObject);
    },

    onClick: function(event)
    {
        if (isLeftClick(event))
        {
            var row = getAncestorByClass(event.target, "netRow");
            if (row)
            {
                // Click on the rowHeader element inserts a breakpoint.
                if (getAncestorByClass(event.target, "netRowHeader"))
                    return;

                this.toggleHeadersRow(row);
                cancelEvent(event);
            }
        }
    },

    toggleHeadersRow: function(row)
    {
        if (!hasClass(row, "hasHeaders"))
            return;

        var file = row.repObject;

        toggleClass(row, "opened");
        if (hasClass(row, "opened"))
        {
            var netInfoRow = this.netInfoTag.insertRows({}, row)[0];
            var netInfoCol = netInfoRow.getElementsByClassName("netInfoCol").item(0);
            var netInfoBox = NetInfoBody.tag.replace({file: file}, netInfoCol);

            // Notify listeners so additional tabs can be created.
            dispatch(NetInfoBody.fbListeners, "initTabBody", [netInfoBox, file]);

            NetInfoBody.selectTabByName(netInfoBox, "Headers");
            var category = Utils.getFileCategory(row.repObject);
            if (category)
                setClass(netInfoBox, "category-" + category);
            row.setAttribute('aria-expanded', 'true');
        }
        else
        {
            var netInfoRow = row.nextSibling;
            var netInfoBox = netInfoRow.getElementsByClassName("netInfoBody").item(0);

            dispatch(NetInfoBody.fbListeners, "destroyTabBody", [netInfoBox, file]);

            row.parentNode.removeChild(netInfoRow);
            row.setAttribute('aria-expanded', 'false');
        }
    },

    getCategory: function(file)
    {
        var category = Utils.getFileCategory(file);
        if (category)
            return "category-" + category;

        return "category-undefined";
    },

    getInFrame: function(file)
    {
        return !!file.document.parent;
    },

    getIndent: function(file)
    {
        // XXXjoe Turn off indenting for now, it's confusing since we don't
        // actually place nested files directly below their parent
        //return file.document.level * indentWidth;
        return 10;
    },

    isError: function(file)
    {
        if (file.aborted)
            return true;

        var errorRange = Math.floor(file.responseStatus/100);
        return errorRange == 4 || errorRange == 5;
    },

    getHref: function(file)
    {
        return (file.method ? file.method.toUpperCase() : "?") + " " + getFileName(file.href);
    },

    getStatus: function(file)
    {
        var text = "";

        if (file.responseStatus)
            text += file.responseStatus + " ";

        if (file.responseStatusText)
            text += file.responseStatusText;

        return text ? cropString(text) : " ";
    },

    getDomain: function(file)
    {
        return getPrettyDomain(file.href);
    },

    getSize: function(file)
    {
        return this.formatSize(file.size);
    },

    getElapsedTime: function(file)
    {
        if (!file.elapsed || file.elapsed < 0)
            return "";

        return this.formatTime(file.elapsed);
    },

    hasRequestHeaders: function(file)
    {
        return !!file.requestHeaders;
    },

    formatSize: function(bytes)
    {
        return formatSize(bytes);
    },

    formatTime: function(elapsed)
    {
        // Use formatTime util from the lib.
        return formatTime(elapsed);
    }
});

var NetRequestEntry = Firebug.NetMonitor.NetRequestEntry;

// ************************************************************************************************

Firebug.NetMonitor.NetPage = domplate(Firebug.Rep,
{
    separatorTag:
        TR({"class": "netRow netPageSeparatorRow"},
            TD({"class": "netCol netPageSeparatorLabel", colspan: 6, "role": "separator"})
        ),

    pageTag:
        TR({"class": "netRow netPageRow", onclick: "$onPageClick"},
            TD({"class": "netCol netPageCol", colspan: 6, "role": "separator"},
                DIV({"class": "netLabel netPageLabel netPageTitle"}, "$page|getTitle")
            )
        ),

    getTitle: function(page)
    {
        return page.pageTitle;
    },

    onPageClick: function(event)
    {
        if (!isLeftClick(event))
            return;

        var target = event.target;
        var pageRow = getAncestorByClass(event.target, "netPageRow");
        var panel = Firebug.getElementPanel(pageRow);

        if (!hasClass(pageRow, "opened"))
        {
            setClass(pageRow, "opened");

            var files = pageRow.files;

            // Move all net-rows from the persistedState to this panel.
            panel.insertRows(files, pageRow);

            for (var i=0; i<files.length; i++)
                panel.queue.push(files[i].file);

            panel.layout();
        }
        else
        {
            removeClass(pageRow, "opened");

            var nextRow = pageRow.nextSibling;
            while (!hasClass(nextRow, "netPageRow") && !hasClass(nextRow, "netPageSeparatorRow"))
            {
                var nextSibling = nextRow.nextSibling;
                nextRow.parentNode.removeChild(nextRow);
                nextRow = nextSibling;
            }
        }
    },
});

var NetPage = Firebug.NetMonitor.NetPage;

// ************************************************************************************************

/**
 * @domplate Represents a template that is used to reneder detailed info about a request.
 * This template is rendered when a request is expanded.
 */
Firebug.NetMonitor.NetInfoBody = domplate(Firebug.Rep, new Firebug.Listener(),
{
    tag:
        DIV({"class": "netInfoBody", _repObject: "$file"},
            TAG("$infoTabs", {file: "$file"}),
            TAG("$infoBodies", {file: "$file"})
        ),

    infoTabs:
        DIV({"class": "netInfoTabs focusRow subFocusRow", "role": "tablist"},
            A({"class": "netInfoParamsTab netInfoTab a11yFocus", onclick: "$onClickTab", "role": "tab",
                view: "Params",
                $collapsed: "$file|hideParams"},
                $STR("URLParameters")
            ),
            A({"class": "netInfoHeadersTab netInfoTab a11yFocus", onclick: "$onClickTab", "role": "tab",
                view: "Headers"},
                $STR("Headers")
            ),
            A({"class": "netInfoPostTab netInfoTab a11yFocus", onclick: "$onClickTab", "role": "tab",
                view: "Post",
                $collapsed: "$file|hidePost"},
                $STR("Post")
            ),
            A({"class": "netInfoPutTab netInfoTab a11yFocus", onclick: "$onClickTab", "role": "tab",
                view: "Put",
                $collapsed: "$file|hidePut"},
                $STR("Put")
            ),
            A({"class": "netInfoResponseTab netInfoTab a11yFocus", onclick: "$onClickTab", "role": "tab",
                view: "Response",
                $collapsed: "$file|hideResponse"},
                $STR("Response")
            ),
            A({"class": "netInfoCacheTab netInfoTab a11yFocus", onclick: "$onClickTab", "role": "tab",
               view: "Cache",
               $collapsed: "$file|hideCache"},
               $STR("Cache")
            ),
            A({"class": "netInfoHtmlTab netInfoTab a11yFocus", onclick: "$onClickTab", "role": "tab",
               view: "Html",
               $collapsed: "$file|hideHtml"},
               $STR("HTML")
            )
        ),

    infoBodies:
        DIV({"class": "netInfoBodies outerFocusRow"},
            TABLE({"class": "netInfoParamsText netInfoText netInfoParamsTable", "role": "tabpanel",
                    cellpadding: 0, cellspacing: 0}, TBODY()),
            DIV({"class": "netInfoHeadersText netInfoText", "role": "tabpanel"}),
            DIV({"class": "netInfoPostText netInfoText", "role": "tabpanel"}),
            DIV({"class": "netInfoPutText netInfoText", "role": "tabpanel"}),
            DIV({"class": "netInfoResponseText netInfoText", "role": "tabpanel"}),
            DIV({"class": "netInfoCacheText netInfoText", "role": "tabpanel"},
                TABLE({"class": "netInfoCacheTable", cellpadding: 0, cellspacing: 0, "role": "presentation"},
                    TBODY({"role": "list", "aria-label": $STR("Cache")})
                )
            ),
            DIV({"class": "netInfoHtmlText netInfoText", "role": "tabpanel"},
                IFRAME({"class": "netInfoHtmlPreview", "role": "document"})
            )
        ),

    headerDataTag:
        FOR("param", "$headers",
            TR({"role": "listitem"},
                TD({"class": "netInfoParamName", "role": "presentation"},
                    TAG("$param|getNameTag", {param: "$param"})
                ),
                TD({"class": "netInfoParamValue", "role": "list", "aria-label": "$param.name"},
                    FOR("line", "$param|getParamValueIterator",
                        CODE({"class": "focusRow subFocusRow", "role": "listitem"}, "$line")
                    )
                )
            )
        ),

    customTab:
        A({"class": "netInfo$tabId\\Tab netInfoTab", onclick: "$onClickTab", view: "$tabId", "role": "tab"},
            "$tabTitle"
        ),

    customBody:
        DIV({"class": "netInfo$tabId\\Text netInfoText", "role": "tabpanel"}),

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    nameTag:
        SPAN("$param|getParamName"),

    nameWithTooltipTag:
        SPAN({title: "$param.name"}, "$param|getParamName"),

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    getNameTag: function(param)
    {
        return (this.getParamName(param) == param.name) ? this.nameTag : this.nameWithTooltipTag;
    },

    getParamName: function(param)
    {
        var name = param.name;
        var limit = Firebug.netParamNameLimit;
        if (limit <= 0)
            return name;

        if (name.length > limit)
            name = name.substr(0, limit) + "...";
        return name;
    },

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    hideParams: function(file)
    {
        return !file.urlParams || !file.urlParams.length;
    },

    hidePost: function(file)
    {
        return file.method.toUpperCase() != "POST";
    },

    hidePut: function(file)
    {
        return file.method.toUpperCase() != "PUT";
    },

    hideResponse: function(file)
    {
        return file.category in binaryFileCategories;
    },

    hideCache: function(file)
    {
        //xxxHonza: I don't see any reason why not to display the cache also info for images.
        return !file.cacheEntry/* || file.category=="image"*/;
    },

    hideHtml: function(file)
    {
        return (file.mimeType != "text/html") && (file.mimeType != "application/xhtml+xml");
    },

    onClickTab: function(event)
    {
        this.selectTab(event.currentTarget);
    },

    getParamValueIterator: function(param)
    {
        // This value is inserted into CODE element and so, make sure the HTML isn't escaped (1210).
        // This is why the second parameter is true.
        // The CODE (with style white-space:pre) element preserves whitespaces so they are
        // displayed the same, as they come from the server (1194).
        // In case of a long header values of post parameters the value must be wrapped (2105).
        return wrapText(param.value, true);
    },

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    appendTab: function(netInfoBox, tabId, tabTitle)
    {
        // Create new tab and body.
        var args = {tabId: tabId, tabTitle: tabTitle};
        this.customTab.append(args, netInfoBox.getElementsByClassName("netInfoTabs").item(0));
        this.customBody.append(args, netInfoBox.getElementsByClassName("netInfoBodies").item(0));
    },

    selectTabByName: function(netInfoBox, tabName)
    {
        var tab = getChildByClass(netInfoBox, "netInfoTabs", "netInfo"+tabName+"Tab");
        if (tab)
            this.selectTab(tab);
    },

    selectTab: function(tab)
    {
        var netInfoBox = getAncestorByClass(tab, "netInfoBody");

        var view = tab.getAttribute("view");
        if (netInfoBox.selectedTab)
        {
            netInfoBox.selectedTab.removeAttribute("selected");
            netInfoBox.selectedText.removeAttribute("selected");
            netInfoBox.selectedTab.setAttribute("aria-selected", "false");
        }

        var textBodyName = "netInfo" + view + "Text";

        netInfoBox.selectedTab = tab;
        netInfoBox.selectedText = netInfoBox.getElementsByClassName(textBodyName).item(0);

        netInfoBox.selectedTab.setAttribute("selected", "true");
        netInfoBox.selectedText.setAttribute("selected", "true");
        netInfoBox.selectedTab.setAttribute("aria-selected", "true");

        var file = Firebug.getRepObject(netInfoBox);
        var context = Firebug.getElementPanel(netInfoBox).context;
        this.updateInfo(netInfoBox, file, context);
    },

    updateInfo: function(netInfoBox, file, context)
    {
        if (!netInfoBox)
        {
            return;
        }

        var tab = netInfoBox.selectedTab;
        if (hasClass(tab, "netInfoParamsTab"))
        {
            if (file.urlParams && !netInfoBox.urlParamsPresented)
            {
                netInfoBox.urlParamsPresented = true;
                this.insertHeaderRows(netInfoBox, file.urlParams, "Params");
            }
        }

        if (hasClass(tab, "netInfoHeadersTab"))
        {
            var headersText = netInfoBox.getElementsByClassName("netInfoHeadersText").item(0);

            if (file.responseHeaders && !netInfoBox.responseHeadersPresented)
            {
                netInfoBox.responseHeadersPresented = true;
                NetInfoHeaders.renderHeaders(headersText, file.responseHeaders, "ResponseHeaders");
            }

            if (file.requestHeaders && !netInfoBox.requestHeadersPresented)
            {
                netInfoBox.requestHeadersPresented = true;
                NetInfoHeaders.renderHeaders(headersText, file.requestHeaders, "RequestHeaders");
            }
        }

        if (hasClass(tab, "netInfoPostTab"))
        {
            if (!netInfoBox.postPresented)
            {
                netInfoBox.postPresented  = true;
                var postText = netInfoBox.getElementsByClassName("netInfoPostText").item(0);
                NetInfoPostData.render(context, postText, file);
            }
        }

        if (hasClass(tab, "netInfoPutTab"))
        {
            if (!netInfoBox.putPresented)
            {
                netInfoBox.putPresented  = true;
                var putText = netInfoBox.getElementsByClassName("netInfoPutText").item(0);
                NetInfoPostData.render(context, putText, file);
            }
        }

        if (hasClass(tab, "netInfoResponseTab") && file.loaded && !netInfoBox.responsePresented)
        {
            var responseTextBox = netInfoBox.getElementsByClassName("netInfoResponseText").item(0);
            if (file.category == "image")
            {
                netInfoBox.responsePresented = true;

                var responseImage = netInfoBox.ownerDocument.createElement("img");
                responseImage.src = file.href;

                clearNode(responseTextBox);
                responseTextBox.appendChild(responseImage, responseTextBox);
            }
            else if (!(binaryCategoryMap.hasOwnProperty(file.category)))
            {
                this.setResponseText(file, netInfoBox, responseTextBox, context);
            }
        }

        if (hasClass(tab, "netInfoCacheTab") && file.loaded && !netInfoBox.cachePresented)
        {
            var responseTextBox = netInfoBox.getElementsByClassName("netInfoCacheText").item(0);
            if (file.cacheEntry) {
                netInfoBox.cachePresented = true;
                this.insertHeaderRows(netInfoBox, file.cacheEntry, "Cache");
            }
        }

        if (hasClass(tab, "netInfoHtmlTab") && file.loaded && !netInfoBox.htmlPresented)
        {
            netInfoBox.htmlPresented = true;

            var text = Utils.getResponseText(file, context);
            var iframe = netInfoBox.getElementsByClassName("netInfoHtmlPreview").item(0);
            iframe.contentWindow.document.body.innerHTML = text;
        }

        // Notify listeners about update so, content of custom tabs can be updated.
        dispatch(NetInfoBody.fbListeners, "updateTabBody", [netInfoBox, file, context]);
    },

    setResponseText: function(file, netInfoBox, responseTextBox, context)
    {
        // Get response text and make sure it doesn't exceed the max limit.
        var text = Utils.getResponseText(file, context);
        var limit = Firebug.netDisplayedResponseLimit + 15;
        var limitReached = text ? (text.length > limit) : false;
        if (limitReached)
            text = text.substr(0, limit) + "...";

        // Insert the response into the UI.
        if (text)
            insertWrappedText(text, responseTextBox);
        else
            insertWrappedText("", responseTextBox);

        // Append a message informing the user that the response isn't fully displayed.
        if (limitReached)
        {
            var object = {
                text: $STR("net.responseSizeLimitMessage"),
                onClickLink: function() {
                    var panel = context.getPanel("net", true);
                    panel.openResponseInTab(file);
                }
            };
            Firebug.NetMonitor.ResponseSizeLimit.append(object, responseTextBox);
        }

        netInfoBox.responsePresented = true;

    },

    insertHeaderRows: function(netInfoBox, headers, tableName, rowName)
    {
        if (!headers.length)
            return;

        var headersTable = netInfoBox.getElementsByClassName("netInfo"+tableName+"Table").item(0);
        var tbody = getChildByClass(headersTable, "netInfo" + rowName + "Body");
        if (!tbody)
            tbody = headersTable.firstChild;
        var titleRow = getChildByClass(tbody, "netInfo" + rowName + "Title");

        this.headerDataTag.insertRows({headers: headers}, titleRow ? titleRow : tbody);
        removeClass(titleRow, "collapsed");
    },
});

var NetInfoBody = Firebug.NetMonitor.NetInfoBody;

// ************************************************************************************************

/**
 * @domplate Represents posted data within request info (the info, which is visible when
 * a request entry is expanded. This template renders content of the Post tab.
 */
Firebug.NetMonitor.NetInfoPostData = domplate(Firebug.Rep, new Firebug.Listener(),
{
    // application/x-www-form-urlencoded
    paramsTable:
        TABLE({"class": "netInfoPostParamsTable", cellpadding: 0, cellspacing: 0, "role": "presentation"},
            TBODY({"role": "list", "aria-label": $STR("net.label.Parameters")},
                TR({"class": "netInfoPostParamsTitle", "role": "presentation"},
                    TD({colspan: 2, "role": "presentation"},
                        DIV({"class": "netInfoPostParams"},
                            $STR("net.label.Parameters"),
                            SPAN({"class": "netInfoPostContentType"},
                                "application/x-www-form-urlencoded"
                            )
                        )
                    )
                )
            )
        ),

    // multipart/form-data
    partsTable:
        TABLE({"class": "netInfoPostPartsTable", cellpadding: 0, cellspacing: 0, "role": "presentation"},
            TBODY({"role": "list", "aria-label": $STR("net.label.Parts")},
                TR({"class": "netInfoPostPartsTitle", "role": "presentation"},
                    TD({colspan: 2, "role":"presentation" },
                        DIV({"class": "netInfoPostParams"},
                            $STR("net.label.Parts"),
                            SPAN({"class": "netInfoPostContentType"},
                                "multipart/form-data"
                            )
                        )
                    )
                )
            )
        ),

    // application/json
    jsonTable:
        TABLE({"class": "netInfoPostJSONTable", cellpadding: 0, cellspacing: 0, "role": "presentation"},
            TBODY({"role": "list", "aria-label": $STR("jsonviewer.tab.JSON")},
                TR({"class": "netInfoPostJSONTitle", "role": "presentation"},
                    TD({"role": "presentation" },
                        DIV({"class": "netInfoPostParams"},
                            $STR("jsonviewer.tab.JSON")
                        )
                    )
                ),
                TR(
                    TD({"class": "netInfoPostJSONBody"})
                )
            )
        ),

    // application/xml
    xmlTable:
        TABLE({"class": "netInfoPostXMLTable", cellpadding: 0, cellspacing: 0, "role": "presentation"},
            TBODY({"role": "list", "aria-label": $STR("xmlviewer.tab.XML")},
                TR({"class": "netInfoPostXMLTitle", "role": "presentation"},
                    TD({"role": "presentation" },
                        DIV({"class": "netInfoPostParams"},
                            $STR("xmlviewer.tab.XML")
                        )
                    )
                ),
                TR(
                    TD({"class": "netInfoPostXMLBody"})
                )
            )
        ),

    sourceTable:
        TABLE({"class": "netInfoPostSourceTable", cellpadding: 0, cellspacing: 0, "role": "presentation"},
            TBODY({"role": "list", "aria-label": $STR("net.label.Source")},
                TR({"class": "netInfoPostSourceTitle", "role": "presentation"},
                    TD({colspan: 2, "role": "presentation"},
                        DIV({"class": "netInfoPostSource"},
                            $STR("net.label.Source")
                        )
                    )
                )
            )
        ),

    sourceBodyTag:
        TR({"role": "presentation"},
            TD({colspan: 2, "role": "presentation"},
                FOR("line", "$param|getParamValueIterator",
                    CODE({"class":"focusRow subFocusRow" , "role": "listitem"},"$line")
                )
            )
        ),

    getParamValueIterator: function(param)
    {
        return NetInfoBody.getParamValueIterator(param);
    },

    render: function(context, parentNode, file)
    {
        var text = Utils.getPostText(file, context, true);
        if (text == undefined)
            return;

        if (Utils.isURLEncodedRequest(file, context))
        {
            var lines = text.split("\n");
            var params = parseURLEncodedText(lines[lines.length-1]);
            if (params)
                this.insertParameters(parentNode, params);
        }

        if (Utils.isMultiPartRequest(file, context))
        {
            var data = this.parseMultiPartText(file, context);
            if (data)
                this.insertParts(parentNode, data);
        }

        var contentType = Utils.findHeader(file.requestHeaders, "content-type");

        if (Firebug.JSONViewerModel.isJSON(contentType))
            this.insertJSON(parentNode, file, context);

        if (Firebug.XMLViewerModel.isXML(contentType))
            this.insertXML(parentNode, file, context);

        var postText = Utils.getPostText(file, context);
        postText = Utils.formatPostText(postText);
        if (postText)
            this.insertSource(parentNode, postText);
    },

    insertParameters: function(parentNode, params)
    {
        if (!params || !params.length)
            return;

        var paramTable = this.paramsTable.append(null, parentNode);
        var row = paramTable.getElementsByClassName("netInfoPostParamsTitle").item(0);

        NetInfoBody.headerDataTag.insertRows({headers: params}, row);
    },

    insertParts: function(parentNode, data)
    {
        if (!data.params || !data.params.length)
            return;

        var partsTable = this.partsTable.append(null, parentNode);
        var row = partsTable.getElementsByClassName("netInfoPostPartsTitle").item(0);

        NetInfoBody.headerDataTag.insertRows({headers: data.params}, row);
    },

    insertJSON: function(parentNode, file, context)
    {
        var text = Utils.getPostText(file, context);
        var data = parseJSONString(text, "http://" + file.request.originalURI.host);
        if (!data)
            return;

        var jsonTable = this.jsonTable.append(null, parentNode);
        var jsonBody = jsonTable.getElementsByClassName("netInfoPostJSONBody").item(0);

        if (!this.toggles)
            this.toggles = {};

        Firebug.DOMPanel.DirTable.tag.replace(
            {object: data, toggles: this.toggles}, jsonBody);
    },

    insertXML: function(parentNode, file, context)
    {
        var text = Utils.getPostText(file, context);

        var jsonTable = this.xmlTable.append(null, parentNode);
        var jsonBody = jsonTable.getElementsByClassName("netInfoPostXMLBody").item(0);

        Firebug.XMLViewerModel.insertXML(jsonBody, text);
    },

    insertSource: function(parentNode, text)
    {
        var sourceTable = this.sourceTable.append(null, parentNode);
        var row = sourceTable.getElementsByClassName("netInfoPostSourceTitle").item(0);

        var param = {value: text};
        this.sourceBodyTag.insertRows({param: param}, row);
    },

    parseMultiPartText: function(file, context)
    {
        var text = Utils.getPostText(file, context);
        if (text == undefined)
            return null;

        FBTrace.sysout("net.parseMultiPartText; boundary: ", text);

        var boundary = text.match(/\s*boundary=\s*(.*)/)[1];

        var divider = "\r\n\r\n";
        var bodyStart = text.indexOf(divider);
        var body = text.substr(bodyStart + divider.length);

        var postData = {};
        postData.mimeType = "multipart/form-data";
        postData.params = [];

        var parts = body.split("--" + boundary);
        for (var i=0; i<parts.length; i++)
        {
            var part = parts[i].split(divider);
            if (part.length != 2)
                continue;

            var m = part[0].match(/\s*name=\"(.*)\"(;|$)/);
            postData.params.push({
                name: (m && m.length > 1) ? m[1] : "",
                value: trim(part[1])
            })
        }

        return postData;
    }
});

var NetInfoPostData = Firebug.NetMonitor.NetInfoPostData;

// ************************************************************************************************

/**
 * @domplate Used within the Net panel to display raw source of request and response headers
 * as well as pretty-formatted summary of these headers.
 */
Firebug.NetMonitor.NetInfoHeaders = domplate(Firebug.Rep, new Firebug.Listener(),
{
    tag:
        DIV({"class": "netInfoHeadersTable", "role": "tabpanel"},
            DIV({"class": "netInfoHeadersGroup netInfoResponseHeadersTitle"},
                SPAN($STR("ResponseHeaders")),
                SPAN({"class": "netHeadersViewSource response collapsed", onclick: "$onViewSource",
                    _sourceDisplayed: false, _rowName: "ResponseHeaders"},
                    $STR("net.headers.view source")
                )
            ),
            TABLE({cellpadding: 0, cellspacing: 0},
                TBODY({"class": "netInfoResponseHeadersBody", "role": "list",
                    "aria-label": $STR("ResponseHeaders")})
            ),
            DIV({"class": "netInfoHeadersGroup netInfoRequestHeadersTitle"},
                SPAN($STR("RequestHeaders")),
                SPAN({"class": "netHeadersViewSource request collapsed", onclick: "$onViewSource",
                    _sourceDisplayed: false, _rowName: "RequestHeaders"},
                    $STR("net.headers.view source")
                )
            ),
            TABLE({cellpadding: 0, cellspacing: 0},
                TBODY({"class": "netInfoRequestHeadersBody", "role": "list",
                    "aria-label": $STR("RequestHeaders")})
            )
        ),

    sourceTag:
        TR({"role": "presentation"},
            TD({colspan: 2, "role": "presentation"},
                PRE({"class": "source"})
            )
        ),

    onViewSource: function(event)
    {
        var target = event.target;
        var requestHeaders = (target.rowName == "RequestHeaders");

        var netInfoBox = getAncestorByClass(target, "netInfoBody");
        var file = netInfoBox.repObject;

        if (target.sourceDisplayed)
        {
            var headers = requestHeaders ? file.requestHeaders : file.responseHeaders;
            this.insertHeaderRows(netInfoBox, headers, target.rowName);
            target.innerHTML = $STR("net.headers.view source");
        }
        else
        {
            var source = requestHeaders ? file.requestHeadersText : file.responseHeadersText;
            this.insertSource(netInfoBox, source, target.rowName);
            target.innerHTML = $STR("net.headers.pretty print");
        }

        target.sourceDisplayed = !target.sourceDisplayed;

        cancelEvent(event);
    },

    insertSource: function(netInfoBox, source, rowName)
    {
        // This breaks copy to clipboard.
        //if (source)
        //    source = source.replace(/\r\n/gm, "<span style='color:lightgray'>\\r\\n</span>\r\n");

        var tbody = netInfoBox.getElementsByClassName("netInfo" + rowName + "Body").item(0);
        var node = this.sourceTag.replace({}, tbody);
        var sourceNode = node.getElementsByClassName("source").item(0);
        sourceNode.innerHTML = source;
    },

    insertHeaderRows: function(netInfoBox, headers, rowName)
    {
        var headersTable = netInfoBox.getElementsByClassName("netInfoHeadersTable").item(0);
        var tbody = headersTable.getElementsByClassName("netInfo" + rowName + "Body").item(0);

        clearNode(tbody);

        if (!headers.length)
            return;

        NetInfoBody.headerDataTag.insertRows({headers: headers}, tbody);

        var titleRow = getChildByClass(headersTable, "netInfo" + rowName + "Title");
        removeClass(titleRow, "collapsed");
    },

    init: function(parent)
    {
        var rootNode = this.tag.append({}, parent);

        var netInfoBox = getAncestorByClass(parent, "netInfoBody");
        var file = netInfoBox.repObject;

        var viewSource;

        viewSource = rootNode.getElementsByClassName("netHeadersViewSource request").item(0);
        if (file.requestHeadersText)
            removeClass(viewSource, "collapsed");

        viewSource = rootNode.getElementsByClassName("netHeadersViewSource response").item(0);
        if (file.responseHeadersText)
            removeClass(viewSource, "collapsed");
    },

    renderHeaders: function(parent, headers, rowName)
    {
        if (!parent.firstChild)
            this.init(parent);

        this.insertHeaderRows(parent, headers, rowName);
    }
});

var NetInfoHeaders = Firebug.NetMonitor.NetInfoHeaders;

// ************************************************************************************************

/**
 * @domplate Represents a template for popup tip that displays detailed timing info about
 * a network request.
 */
Firebug.NetMonitor.TimeInfoTip = domplate(Firebug.Rep,
{
    tableTag:
        TABLE({"class": "timeInfoTip", "id": "fbNetTimeInfoTip"},
            TBODY()
        ),

    timingsTag:
        FOR("time", "$timings",
            TR({"class": "timeInfoTipRow", $collapsed: "$time|hideBar"},
                TD({"class": "$time|getBarClass timeInfoTipBar",
                    $loaded: "$time.loaded",
                    $fromCache: "$time.fromCache",
                }),
                TD({"class": "timeInfoTipCell startTime"},
                    "$time.start|formatStartTime"
                ),
                TD({"class": "timeInfoTipCell elapsedTime"},
                    "$time.elapsed|formatTime"
                ),
                TD("$time|getLabel")
            )
        ),

    startTimeTag:
        TR(
            TD(),
            TD("$startTime.time|formatStartTime"),
            TD({"colspan": 2},
                "$startTime|getLabel"
            )
        ),

    separatorTag:
        TR(
            TD({"colspan": 4, "height": "10px"})
        ),

    eventsTag:
        FOR("event", "$events",
            TR({"class": "timeInfoTipEventRow"},
                TD({"class": "timeInfoTipBar", align: "center"},
                    DIV({"class": "$event|getBarClass timeInfoTipEventBar"})
                ),
                TD("$event.start|formatStartTime"),
                TD({"colspan": 2},
                    "$event|getLabel"
                )
            )
        ),

    hideBar: function(obj)
    {
        return !obj.elapsed && obj.bar == "Blocking";
    },

    getBarClass: function(obj)
    {
        return "net" + obj.bar + "Bar";
    },

    formatTime: function(time)
    {
        return formatTime(time)
    },

    formatStartTime: function(time)
    {
        var label = formatTime(time);
        if (!time)
            return label;

        return (time > 0 ? "+" : "") + label;
    },

    getLabel: function(obj)
    {
        return $STR("requestinfo." + obj.bar);
    },

    render: function(file, parentNode)
    {
        var infoTip = Firebug.NetMonitor.TimeInfoTip.tableTag.replace({}, parentNode);

        var elapsed = file.loaded ? file.endTime - file.startTime : file.phase.phaseEndTime - file.startTime;
        var blockingEnd = (file.sendingTime > file.startTime) ? file.sendingTime : file.waitingForTime;

        var timings = [];
        timings.push({bar: "Resolving",
            elapsed: file.connectingTime - file.startTime,
            start: 0});
        timings.push({bar: "Connecting",
            elapsed: file.connectedTime - file.connectingTime,
            start: file.connectingTime - file.startTime});
        timings.push({bar: "Blocking",
            elapsed: blockingEnd - file.connectedTime,
            start: file.connectedTime - file.startTime});

        // In Fx3.6 the STATUS_SENDING_TO is always fired (nsIActivityDistributor)
        // In Fx3.5 the STATUS_SENDING_TO (nsIWebProgressListener) doesn't have to come
        // This workaround is for 3.5
        var sendElapsed = file.sendStarted ? file.waitingForTime - file.sendingTime : 0;
        var sendStarted = timings[0].elapsed + timings[1].elapsed + timings[2].elapsed;

        timings.push({bar: "Sending",
            elapsed: sendElapsed,
            start: file.sendStarted ? file.sendingTime - file.startTime : sendStarted});
        timings.push({bar: "Waiting",
            elapsed: file.respondedTime - file.waitingForTime,
            start: file.waitingForTime - file.startTime});
        timings.push({bar: "Receiving",
            elapsed: file.endTime - file.respondedTime,
            start: file.respondedTime - file.startTime,
            loaded: file.loaded, fromCache: file.fromCache});

        var events = [];
        if (file.phase.contentLoadTime)
            events.push({bar: "ContentLoad", start: file.phase.contentLoadTime - file.startTime});
        if (file.phase.windowLoadTime)
            events.push({bar: "WindowLoad", start: file.phase.windowLoadTime - file.startTime});

        // Insert start request time.
        var startTime = {};
        startTime.time = file.startTime - file.phase.startTime;
        startTime.bar = "Started";
        this.startTimeTag.insertRows({startTime: startTime}, infoTip.firstChild);

        // Insert separator.
        this.separatorTag.insertRows({}, infoTip.firstChild);

        // Insert request timing info.
        this.timingsTag.insertRows({timings: timings}, infoTip.firstChild);

        // Insert events timing info.
        if (events.length)
        {
            this.separatorTag.insertRows({}, infoTip.firstChild);
            this.eventsTag.insertRows({events: events}, infoTip.firstChild);
        }

        return true;
    }
});

// ************************************************************************************************

/**
 * @domplate Represents a template for a pupup tip with detailed size info.
 */
Firebug.NetMonitor.SizeInfoTip = domplate(Firebug.Rep,
{
    tag:
        TABLE({"class": "sizeInfoTip", "id": "fbNetSizeInfoTip", role:"presentation"},
            TBODY(
                FOR("size", "$sizeInfo",
                    TAG("$size|getRowTag", {size: "$size"})
                )
            )
        ),

    sizeTag:
        TR({"class": "sizeInfoRow", $collapsed: "$size|hideRow"},
            TD({"class": "sizeInfoLabelCol"}, "$size.label"),
            TD({"class": "sizeInfoSizeCol"}, "$size|formatSize"),
            TD({"class": "sizeInfoDetailCol"}, "$size|formatNumber")
        ),

    separatorTag:
        TR(
            TD({"colspan": 3, "height": "7px"})
        ),

    descTag:
        TR(
            TD({"colspan": 3, "class": "sizeInfoDescCol"}, "$size.label")
        ),

    getRowTag: function(size)
    {
        if (size.size == -2)
            return this.descTag;

        return (size.label == "-") ? this.separatorTag : this.sizeTag;
    },

    hideRow: function(size)
    {
        return size.size < 0;
    },

    formatSize: function(size)
    {
        return formatSize(size.size);
    },

    formatNumber: function(size)
    {
        return size.size ? ("(" + formatNumber(size.size) + ")") : "";
    },

    render: function(file, parentNode)
    {
        var postText = Utils.getPostText(file, FirebugContext, true);
        postText = postText ? postText : "";

        var sizeInfo = [];
        sizeInfo.push({label: $STR("net.sizeinfo.Response Body"), size: file.size});
        sizeInfo.push({label: $STR("net.sizeinfo.Post Body"), size: postText.length});

        if (file.requestHeadersText)
        {
            var responseHeaders = file.responseHeadersText ? file.responseHeadersText : 0;

            sizeInfo.push({label: "-", size: 0});
            sizeInfo.push({label: $STR("net.sizeinfo.Total Received") + "*",
                size: responseHeaders.length + file.size});
            sizeInfo.push({label: $STR("net.sizeinfo.Total Sent") + "*",
                size: file.requestHeadersText.length + postText.length});
            sizeInfo.push({label: "*Including Headers", size: -2});
        }

        this.tag.replace({sizeInfo: sizeInfo}, parentNode);
    },
});

// ************************************************************************************************

Firebug.NetMonitor.NetLimit = domplate(Firebug.Rep,
{
    collapsed: true,

    tableTag:
        DIV(
            TABLE({width: "100%", cellpadding: 0, cellspacing: 0},
                TBODY()
            )
        ),

    limitTag:
        TR({"class": "netRow netLimitRow", $collapsed: "$isCollapsed"},
            TD({"class": "netCol netLimitCol", colspan: 6},
                TABLE({cellpadding: 0, cellspacing: 0},
                    TBODY(
                        TR(
                            TD(
                                SPAN({"class": "netLimitLabel"},
                                    $STRP("plural.Limit_Exceeded", [0])
                                )
                            ),
                            TD({style: "width:100%"}),
                            TD(
                                BUTTON({"class": "netLimitButton", title: "$limitPrefsTitle",
                                    onclick: "$onPreferences"},
                                  $STR("LimitPrefs")
                                )
                            ),
                            TD("&nbsp;")
                        )
                    )
                )
            )
        ),

    isCollapsed: function()
    {
        return this.collapsed;
    },

    onPreferences: function(event)
    {
        openNewTab("about:config");
    },

    updateCounter: function(row)
    {
        removeClass(row, "collapsed");

        // Update info within the limit row.
        var limitLabel = row.getElementsByClassName("netLimitLabel").item(0);
        limitLabel.firstChild.nodeValue = $STRP("plural.Limit_Exceeded", [row.limitInfo.totalCount]);
    },

    createTable: function(parent, limitInfo)
    {
        var table = this.tableTag.replace({}, parent);
        var row = this.createRow(table.firstChild.firstChild, limitInfo);
        return [table, row];
    },

    createRow: function(parent, limitInfo)
    {
        var row = this.limitTag.insertRows(limitInfo, parent, this)[0];
        row.limitInfo = limitInfo;
        return row;
    },

    // nsIPrefObserver
    observe: function(subject, topic, data)
    {
        // We're observing preferences only.
        if (topic != "nsPref:changed")
          return;

        if (data.indexOf("net.logLimit") != -1)
            this.updateMaxLimit();
    },

    updateMaxLimit: function()
    {
        var value = Firebug.getPref(Firebug.prefDomain, "net.logLimit");
        maxQueueRequests = value ? value : maxQueueRequests;
    }
});

var NetLimit = Firebug.NetMonitor.NetLimit;

// ************************************************************************************************

Firebug.NetMonitor.ResponseSizeLimit = domplate(Firebug.Rep,
{
    tag:
        DIV({"class": "netInfoResponseSizeLimit"},
            SPAN("$object.beforeLink"),
            A({"class": "objectLink", onclick: "$onClickLink"},
                "$object.linkText"
            ),
            SPAN("$object.afterLink")
        ),

    reLink: /^(.*)<a>(.*)<\/a>(.*$)/,
    append: function(obj, parent)
    {
        var m = obj.text.match(this.reLink);
        return this.tag.append({onClickLink: obj.onClickLink,
            object: {
            beforeLink: m[1],
            linkText: m[2],
            afterLink: m[3],
        }}, parent, this);
    }
});

// ************************************************************************************************

function NetProgress(context)
{
    this.context = context;
    this.breakpoints = new NetBreakpointGroup();

    var panel = null;
    var queue = [];

    this.post = function(handler, args)
    {
        if (panel)
        {
            var file = handler.apply(this, args);
            if (file)
            {
                panel.updateFile(file);

                // If the panel isn't currently visible, make sure the limit is up to date.
                if (!panel.layoutInterval)
                    panel.updateLogLimit(maxQueueRequests);

                return file;
            }
        }
        else
        {
            // The first page request is made before the initContext (known problem).
            queue.push(handler, args);
        }
    };

    this.flush = function()
    {
        for (var i=0; i<queue.length; i+=2)
            this.post(queue[i], queue[i+1]);

        queue = [];
    };

    this.activate = function(activePanel)
    {
        this.panel = panel = activePanel;
        if (panel)
            this.flush();
    };

    this.update = function(file)
    {
        if (panel)
            panel.updateFile(file);
    };

    this.clear = function()
    {
        for (var i=0; this.files && i<this.files.length; i++)
            this.files[i].clear();

        this.requests = [];
        this.files = [];
        this.phases = [];
        this.documents = [];
        this.windows = [];
        this.currentPhase = null;

        queue = [];
    };

    this.cacheListener = new NetCacheListener(this);

    this.clear();
}

NetProgress.prototype =
{
    panel: null,

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    startFile: function startFile(request, win)
    {
        var file = this.getRequestFile(request, win);
        if (file)
        {
            // XXXjjb Honza I have to set these to get the conditional to work
            file.urlParams = parseURLParams(file.href);
            this.breakOnXHR(file);
        }
    },

    requestedFile: function requestedFile(request, time, win, xhr)
    {
        var file = this.getRequestFile(request, win);
        if (file)
        {
            file.startTime = file.endTime = time;
            file.resolvingTime = time;
            file.connectingTime = time;
            file.connectedTime = time;
            file.sendingTime = time;
            file.waitingForTime = time;
            file.respondedTime = time;
            file.isXHR = xhr;
            file.isBackground = request.loadFlags & LOAD_BACKGROUND;
            file.method = request.requestMethod;
            //file.urlParams = parseURLParams(file.href);

            if (!Ci.nsIHttpActivityDistributor)
                Utils.getPostText(file, this.context);

            this.extendPhase(file);

            dispatch(Firebug.NetMonitor.fbListeners, "onRequest", [this.context, file]);

            return file;
        }
        else
        {
        }
    },

    breakOnXHR: function(file)
    {
        var halt = false;
        var conditionIsFalse = false;

        // If there is an enabled breakpoint with condition:
        // 1) break if the condition is evaluated to true.
        var breakpoints = this.context.netProgress.breakpoints;
        var bp = breakpoints ? breakpoints.findBreakpoint(file.getFileURL()) : null;
        if (bp && bp.checked)
        {
            halt = true;
            if (bp.condition)
            {
                halt = bp.evaluateCondition(this.context, file);
                conditionIsFalse = !halt;
            }
        }

        // 2) If break on XHR flag is set and there is no condition evaluated to false,
        // break with "break on next" breaking cause (this new breaking cause can override
        // an existing one that is set when evaluating a breakpoint condition).
        if (this.context.breakOnXHR && !conditionIsFalse)
        {
            this.context.breakingCause = {
                title: $STR("net.Break On XHR"),
                message: cropString(file.href, 200),
                copyAction: bindFixed(copyToClipboard, FBL, file.href)
            };

            halt = true;
        }

        // Ignore if there is no reason to break.
        if (!halt)
            return;

        // Even if the execution was stopped at breakpoint reset the global
        // breakOnXHR flag.
        this.context.breakOnXHR = false;

        Firebug.Breakpoint.breakNow(this.context.getPanel(panelName, true));
    },

    requestHeadersFile: function(request, time, requestHeadersText)
    {
        var file = this.getRequestFile(request);
        if (file)
            file.requestHeadersText = requestHeadersText;
    },

    responseHeadersFile: function(request, time, responseHeadersText)
    {
        var file = this.getRequestFile(request);
        if (file)
            file.responseHeadersText = responseHeadersText;
    },

    bodySentFile: function bodySentFile(request, time)
    {
        var file = this.getRequestFile(request);
        if (file)
        {
            Utils.getPostText(file, this.context);
        }
    },

    completedFile: function completedFile(request, time)
    {
        var file = this.getRequestFile(request);
        if (file)
        {
            file.respondedTime = time;
            file.endTime = time;
            return file;
        }
    },

    respondedFile: function respondedFile(request, time, info)
    {
        dispatch(Firebug.NetMonitor.fbListeners, "onExamineResponse", [this.context, request]);

        var file = this.getRequestFile(request);
        if (file)
        {
            if (!Ci.nsIHttpActivityDistributor)
            {
                file.respondedTime = time;
                file.endTime = time;

                if (request.contentLength >= 0)
                    file.size = request.contentLength;
            }

            if (info)
            {
                if (info.responseStatus == 304)
                    file.fromCache = true;
                else if (!file.fromCache)
                    file.fromCache = false;
            }

            Utils.getHttpHeaders(request, file);

            if (info)
            {
                file.responseStatus = info.responseStatus;
                file.responseStatusText = info.responseStatusText;
                file.postText = info.postText;
            }

            file.aborted = false;

            // Use ACTIVITY_SUBTYPE_RESPONSE_COMPLETE to get the info if possible.
            if (!Ci.nsIHttpActivityDistributor)
            {
                if (file.fromCache)
                    getCacheEntry(file, this);
            }

            if (file.loaded)
                return;

            this.endLoad(file);

            // If there is a network error, log it into the Console panel.
            if (Firebug.showNetworkErrors && NetRequestEntry.isError(file))
            {
                Firebug.Errors.increaseCount(this.context);
                var message = "NetworkError: " + NetRequestEntry.getStatus(file) + " - "+file.href;
                Firebug.Console.log(message, this.context, "error", null, true, file.getFileLink(message));
            }

            dispatch(Firebug.NetMonitor.fbListeners, "onResponse", [this.context, file]);
            return file;
        }
    },

    respondedCacheFile: function respondedCacheFile(request, time)
    {
        var file = this.getRequestFile(request, null, true);
        if (file)
        {
            this.panel.removeLogEntry(file, true);
        }
        else
        {
        }
    },

    waitingForFile: function waitingForFile(request, time)
    {
        var file = this.getRequestFile(request, null, true);
        if (file)
        {
            if (!file.receivingStarted)
            {
                file.waitingForTime = time;
                file.receivingStarted = true;
            }
        }

        // Don't update the UI now (optimalization).
        return null;
    },

    sendingFile: function sendingFile(request, time, size)
    {
        var file = this.getRequestFile(request, null, true);
        if (file)
        {
            // Remember when the send started.
            if (!file.sendStarted)
            {
                file.sendingTime = time;
                file.sendStarted = true;
            }

            file.totalSent = size;

        }

        // Don't update the UI now (optimalization).
        return null;
    },

    connectingFile: function connectingFile(request, time)
    {
        var file = this.getRequestFile(request, null, true);
        if (file)
        {
            file.connectingTime = time;
            file.connectedTime = time; // just in case connected_to would never came.
        }

        // Don't update the UI now (optimalization).
        return null;
    },

    connectedFile: function connectedFile(request, time)
    {
        var file = this.getRequestFile(request, null, true);
        if (file)
        {
            file.connectedTime = time;
        }

        // Don't update the UI now (optimalization).
        return null;
    },

    receivingFile: function receivingFile(request, time, size)
    {
        var file = this.getRequestFile(request, null, true);
        if (file)
        {
            file.endTime = time;
            file.totalReceived = size;

            // Update phase's lastFinishedFile in case of long time downloads.
            // This forces the timeline to have proper extent.
            if (file.phase && file.phase.endTime < time)
                file.phase.lastFinishedFile = file;

            // Force update UI.
            if (file.row && hasClass(file.row, "opened"))
            {
                var netInfoBox = file.row.nextSibling.getElementsByClassName("netInfoBody").item(0);
                if (netInfoBox)
                {
                    netInfoBox.responsePresented = false;
                    netInfoBox.htmlPresented = false;
                }
            }
        }

        return file;
    },

    completeFile: function completeFile(request, time, responseSize)
    {
        var file = this.getRequestFile(request, null, true);
        if (file)
        {
            if (responseSize > 0)
                file.size = responseSize;

            // This was only a helper to show download progress.
            file.totalReceived = 0;

            // The request is completed, get cache entry.
            getCacheEntry(file, this);

            // Sometimes the HTTP-ON-EXAMINE-RESPONSE doesn't come.
            if (!file.loaded  && file.responseHeadersText)
            {
                var info = null;
                var m = file.responseHeadersText.match(reResponseStatus);
                if (m.length == 3)
                    info = {responseStatus: m[1], responseStatusText: m[2]};
                this.respondedFile(request, now(), info);
            }
        }

        return file;
    },

    closedFile: function closedFile(request, time)
    {
        var file = this.getRequestFile(request, null, true);
        if (file)
        {
            if (!file.loaded && !file.responseHeadersText)
            {
                this.endLoad(file);

                file.aborted = true;
                if (!file.responseStatusText)
                    file.responseStatusText = "Aborted";
                file.respondedTime = time;
                file.endTime = time;
            }
        }

        return file;
    },

    resolvingFile: function resolvingFile(request, time)
    {
        var file = this.getRequestFile(request, null, true);
        if (file)
        {
            file.resolvingTime = time;
        }

        return file;
    },

    progressFile: function progressFile(request, progress, expectedSize, time)
    {
        var file = this.getRequestFile(request, null, true);
        if (file)
        {
            file.size = progress;
            file.expectedSize = expectedSize;
            file.endTime = time;
        }

        return file;
    },

    stopFile: function stopFile(request, time, postText, responseText)
    {
        var file = this.getRequestFile(request, null, true);
        if (file)
        {
            if (file.endTime == file.startTime)
                file.endTime = time;

            file.postText = postText;
            file.responseText = responseText;

            Utils.getHttpHeaders(request, file);

            this.endLoad(file);

            getCacheEntry(file, this);
        }

        return file;
    },

    abortFile: function abortFile(request, time, postText, responseText)
    {
        var file = this.getRequestFile(request, null, true);
        if (file)
        {
            file.aborted = true;
            file.responseStatusText = "Aborted";
        }

        return this.stopFile(request, time, postText, responseText);
    },

    windowLoad: function windowLoad(window, time)
    {
        if (!this.phases.length)
            return;

        // Update all requests that belong to the first phase.
        var firstPhase = this.phases[0];
        firstPhase.windowLoadTime = time;

        // Return the first file, so the layout is updated. I can happen that the
        // onLoad event is the last one and the graph end-time must be recalculated.
        return firstPhase.files[0];
    },

    contentLoad: function contentLoad(window, time)
    {
        if (!this.phases.length)
            return;

        // Update all requests that belong to the first phase.
        var firstPhase = this.phases[0];
        firstPhase.contentLoadTime = time;

        return null;
    },

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    getRequestFile: function getRequestFile(request, win, noCreate)
    {
        var name = safeGetName(request);
        if (!name || reIgnore.exec(name))
            return null;

        var index = this.requests.indexOf(request);
        if (index == -1 && noCreate)
            return null;

        if (index == -1)
        {
            if (!win || getRootWindow(win) != this.context.window)
                return;

            var fileDoc = this.getRequestDocument(win);
            var isDocument = request.loadFlags & LOAD_DOCUMENT_URI && fileDoc.parent;
            var doc = isDocument ? fileDoc.parent : fileDoc;

            var file = doc.createFile(request);
            if (isDocument)
            {
                fileDoc.documentFile = file;
                file.ownDocument = fileDoc;
            }

            file.request = request;
            this.requests.push(request);
            this.files.push(file);
            return file;
        }

        // There is already a file for the reqeust so use it.
        return this.files[index];
    },

    getRequestDocument: function(win)
    {
        if (win)
        {
            var index = this.windows.indexOf(win);
            if (index == -1)
            {
                var doc = new NetDocument();
                if (win.parent != win)
                    doc.parent = this.getRequestDocument(win.parent);

                //doc.level = getFrameLevel(win);

                this.documents.push(doc);
                this.windows.push(win);

                return doc;
            }
            else
                return this.documents[index];
        }
        else
            return this.documents[0];
    },

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    endLoad: function(file)
    {
        file.loaded = true;

        // Update last finished file of the associated phase.
        //xxxHonza: verify this.
        if (file.phase)
            file.phase.lastFinishedFile = file;
    },

    extendPhase: function(file)
    {
        if (this.currentPhase)
        {
            // If the new request has been started within a "phaseInterval" after the
            // previous reqeust has been started, associate it with the current phase;
            // otherwise create a new phase.
            var phaseInterval = Firebug.netPhaseInterval;
            var lastStartTime = this.currentPhase.lastStartTime;
            if (phaseInterval > 0 && this.loaded && file.startTime - lastStartTime >= phaseInterval)
                this.startPhase(file);
            else
                this.currentPhase.addFile(file);
        }
        else
        {
            // If there is no phase yet, just create it.
            this.startPhase(file);
        }
    },

    startPhase: function(file)
    {
        var phase = new NetPhase(file);
        phase.initial = !this.currentPhase;

        this.currentPhase = phase;
        this.phases.push(phase);
    },

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    QueryInterface: function(iid)
    {
        if (iid.equals(Ci.nsIWebProgressListener) ||
            iid.equals(Ci.nsISupportsWeakReference) ||
            iid.equals(Ci.nsISupports))
        {
            return this;
        }

        throw Components.results.NS_NOINTERFACE;
    },

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
    // nsIWebProgressListener

    onStateChange: function(progress, request, flag, status)
    {
        // We can't get the nsIHttpChannel for image requests (images use imgIRequest)
        // So, this method is not much useful.
    },

    onProgressChange : function(progress, request, current, max, total, maxTotal)
    {
        // The timing is measured by activity-distributor observer (if it's available).
        if (Ci.nsIHttpActivityDistributor)
            return;

        var file = this.getRequestFile(request, null, true);
        if (file)
        {
            this.post(progressFile, [request, current, max, now()]);
        }
    },

    onStatusChange: function(progress, request, status, message)
    {
        // The timing is measured by activity-distributor observer (if it's available).
        if (Ci.nsIHttpActivityDistributor)
            return;

        var file = this.getRequestFile(request, null, true);
        if (file)
        {
            if (status == Ci.nsISocketTransport.STATUS_RESOLVING)
                this.post(resolvingFile, [request, now()]);
            else if (status == Ci.nsISocketTransport.STATUS_CONNECTING_TO)
                this.post(connectingFile, [request, now()]);
            else if (status == Ci.nsISocketTransport.STATUS_CONNECTED_TO)
                this.post(connectedFile, [request, now()]);
            else if (status == Ci.nsISocketTransport.STATUS_SENDING_TO)
                this.post(sendingFile, [request, now(), -1]);
            else if (status == Ci.nsISocketTransport.STATUS_WAITING_FOR)
                this.post(waitingForFile, [request, now()]);
            else if (status == Ci.nsISocketTransport.STATUS_RECEIVING_FROM)
                this.post(receivingFile, [request, now(), -1]);
        }
    },

    stateIsRequest: false,
    onLocationChange: function() {},
    onSecurityChange : function() {},
    onLinkIconAvailable : function() {},
};

var startFile = NetProgress.prototype.startFile;
var requestHeadersFile = NetProgress.prototype.requestHeadersFile;
var responseHeadersFile = NetProgress.prototype.responseHeadersFile;
var requestedFile = NetProgress.prototype.requestedFile;
var respondedFile = NetProgress.prototype.respondedFile;
var bodySentFile = NetProgress.prototype.bodySentFile;
var completedFile = NetProgress.prototype.completedFile;
var respondedCacheFile = NetProgress.prototype.respondedCacheFile;
var connectingFile = NetProgress.prototype.connectingFile;
var connectedFile = NetProgress.prototype.connectedFile;
var waitingForFile = NetProgress.prototype.waitingForFile;
var sendingFile = NetProgress.prototype.sendingFile;
var receivingFile = NetProgress.prototype.receivingFile;
var completeFile = NetProgress.prototype.completeFile;
var closedFile = NetProgress.prototype.closedFile;
var resolvingFile = NetProgress.prototype.resolvingFile;
var progressFile = NetProgress.prototype.progressFile;
var windowLoad = NetProgress.prototype.windowLoad;
var contentLoad = NetProgress.prototype.contentLoad;

// XHR Spy
var stopFile = NetProgress.prototype.stopFile;
var abortFile = NetProgress.prototype.abortFile;

// ************************************************************************************************

/**
 * TabCache listner implementation. Net panel uses this listner to remember all
 * responses stored into the cache. There can be more requests to the same URL that
 * returns different responses. The Net panels must remember all of them (tab cache
 * remembers only the last one)
 */
function NetCacheListener(netProgress)
{
    this.netProgress = netProgress;
}

NetCacheListener.prototype =
{
    onStartRequest: function(context, request)
    {
        // Keep in mind that the file object (representing the request) doesn't have to be
        // created at this moment (top document request).
    },

    onStopRequest: function(context, request, responseText)
    {
        // Remember the response for this request.
        var file = this.netProgress.getRequestFile(request, null, true);
        if (file)
            file.responseText = responseText;

        dispatch(Firebug.NetMonitor.fbListeners, "onResponseBody", [context, file]);
    }
}

// ************************************************************************************************

/**
 * A Document is a helper object that represents a document (window) on the page.
 * This object is created for main page document and for every embedded document (iframe)
 * for which a request is made.
 */
function NetDocument()
{
    this.id = 0;
    this.title = "";
}

NetDocument.prototype =
{
    createFile: function(request)
    {
        return new NetFile(request.name, this);
    }
};

// ************************************************************************************************

/**
 * A File is a helper object that represents a file for which a request is made.
 * The document refers to it's parent document (NetDocument) through a member
 * variable.
 */
function NetFile(href, document)
{
    this.href = href;
    this.document = document;
}

NetFile.prototype =
{
    status: 0,
    files: 0,
    loaded: false,
    fromCache: false,
    size: -1,
    expectedSize: -1,
    endTime: null,
    waitingForTime: null,
    connectingTime: null,

    getFileLink: function(message)
    {
        var link = new FBL.SourceLink(this.href, null, "net", this.request);  // this.SourceLink = function(url, line, type, object, instance)
        return link;
    },

    getFileURL: function()
    {
        var index = this.href.indexOf("?");
        if (index < 0)
            return this.href;

        return this.href.substring(0, index);
    },

    clear: function()
    {
        // Remove all members to avoid circular references and memleaks.
        for (var name in this)
            delete this[name];
    }
};

Firebug.NetFile = NetFile;

// ************************************************************************************************

/**
 * A Phase is a helper object that groups requests made in the same time frame.
 * In other words, if a new requests is started within a given time (specified
 * by phaseInterval [ms]) - after previous request has been started -
 * it automatically belongs to the same phase.
 * If a request is started after this period, a new phase is created
 * and this file becomes to be the first in that phase.
 * The first phase is ended when the page finishes it's loading. Other phases
 * might be started by additional XHR made by the page.
 *
 * All phases are stored within NetProgress.phases array.
 *
 * Phases are used to compute size of the graphical timeline. The timeline
 * for each phase starts from the begining of the graph.
 */
function NetPhase(file)
{
  // Start time of the phase. Remains the same, even if the file
  // is removed from the log (due to a max limit of entries).
  // This ensures stability of the time line.
  this.startTime = file.startTime;

  // The last finished request (file) in the phase.
  this.lastFinishedFile = null;

  // Set to true if the phase needs to be updated in the UI.
  this.invalidPhase = null;

  // List of files associated with this phase.
  this.files = [];

  this.addFile(file);
}

NetPhase.prototype =
{
    addFile: function(file)
    {
        this.files.push(file);
        file.phase = this;
    },

    removeFile: function removeFile(file)
    {
        remove(this.files, file);
        file.phase = null;

        // If the last file has been removed, update the last file member.
        if (file == this.lastFinishedFile)
        {
          if (this.files.length == 0)
          {
            this.lastFinishedFile = null;
          }
          else
          {
            for (var i=0; i<this.files.length; i++) {
              if (this.lastFinishedFile.endTime < this.files[i].endTime)
                this.lastFinishedFile = this.files[i];
            }
          }
        }
    },

    get lastStartTime()
    {
      return this.files[this.files.length - 1].startTime;
    },

    get endTime()
    {
      return this.lastFinishedFile ? this.lastFinishedFile.endTime : null;
    }
};

// ************************************************************************************************

/*
 * Use this object to automatically select Net panel and inspect a network request.
 * Firebug.chrome.select(new FBL.NetFileLink(url [, request]));
 */
FBL.NetFileLink = function(href, request)
{
    this.href = href;
    this.request = request;
}

FBL.NetFileLink.prototype =
{
    toString: function()
    {
        return this.message + this.href;
    }
};

// ************************************************************************************************
// Local Helpers

function monitorContext(context)
{
    if (context.netProgress)
        return;

    var networkContext = null;

    // Use an existing context associated with the browser tab if any
    // or create a pure new network context.
    var tabId = Firebug.getTabIdForWindow(context.window);
    networkContext = contexts[tabId];

    if (networkContext)
    {
        networkContext.context = context;
        delete contexts[tabId];
    }
    else
    {
        networkContext = new NetProgress(context);
    }

    // Register activity-distributor observer if available (#488270)
    //NetHttpActivityObserver.registerObserver();

    var listener = context.netProgress = networkContext;

    // Add cache listener so, net panel has alwas fresh responses.
    context.sourceCache.addListener(networkContext.cacheListener);

    // This listener is used to observe downlaod progress.
    context.browser.addProgressListener(listener, NOTIFY_ALL);

    // Activate net panel sub-context.
    var panel = context.getPanel(panelName);
    context.netProgress.activate(panel);

    // Display info message, but only if the panel isn't just reloaded or Persist == true.
    if (!context.persistedState)
        panel.insertActivationMessage();

    // Update status bar icon.
    $('fbStatusIcon').setAttribute("net", "on");
}

function unmonitorContext(context)
{
    var netProgress = context ? context.netProgress : null;
    if (!netProgress)
        return;

    // Since the print into the UI is done by timeout asynchronously,
    // make sure there are no requests left.
    var panel = context.getPanel(panelName, true);
    if (panel)
        panel.updateLayout();

    //NetHttpActivityObserver.unregisterObserver();

    // Remove cache listener
    context.sourceCache.removeListener(netProgress.cacheListener);

    // Remove progress listener.
    if (context.browser.docShell)
        context.browser.removeProgressListener(netProgress, NOTIFY_ALL);

    // Deactivate net sub-context.
    context.netProgress.activate(null);

    // Update status bar icon.
    $('fbStatusIcon').removeAttribute("net");

    // And finaly destroy the net panel sub context.
    delete context.netProgress;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

function getCacheEntry(file, netProgress)
{
    // Bail out if the cache is disabled.
    if (!Firebug.NetMonitor.BrowserCache.isEnabled())
        return;

    // Don't request the cache entry twice.
    if (file.cacheEntryRequested)
        return;

    file.cacheEntryRequested = true;

    setTimeout(function()
    {
        try
        {
            delayGetCacheEntry(file, netProgress);
        }
        catch (exc)
        {
            if (exc.name != "NS_ERROR_CACHE_KEY_NOT_FOUND")
            {
            }
        }
    });
}

function delayGetCacheEntry(file, netProgress)
{
    if (!cacheSession)
    {
        var cacheService = CacheService.getService(Ci.nsICacheService);
        cacheSession = cacheService.createSession("HTTP", Ci.nsICache.STORE_ANYWHERE, true);
        cacheSession.doomEntriesIfExpired = false;
    }

    cacheSession.asyncOpenCacheEntry(file.href, Ci.nsICache.ACCESS_READ,
    {
        onCacheEntryAvailable: function(descriptor, accessGranted, status)
        {
            if (descriptor)
            {
                if (file.size == -1)
                    file.size = descriptor.dataSize;

                if (descriptor.lastModified && descriptor.lastFetched &&
                    descriptor.lastModified < Math.floor(file.startTime/1000)) {
                    file.fromCache = true;
                }

                file.cacheEntry = [
                  { name: "Last Modified",
                    value: Utils.getDateFromSeconds(descriptor.lastModified)
                  },
                  { name: "Last Fetched",
                    value: Utils.getDateFromSeconds(descriptor.lastFetched)
                  },
                  { name: "Expires",
                    value: Utils.getDateFromSeconds(descriptor.expirationTime)
                  },
                  { name: "Data Size",
                    value: descriptor.dataSize
                  },
                  { name: "Fetch Count",
                    value: descriptor.fetchCount
                  },
                  { name: "Device",
                    value: descriptor.deviceID
                  }
                ];

                // Get contentType from the cache.
                descriptor.visitMetaData(
                {
                    visitMetaDataElement: function(key, value)
                    {
                        if (key == "response-head")
                        {
                            var contentType = getContentTypeFromResponseHead(value);
                            file.mimeType = Utils.getMimeType(contentType, file.href);
                            return false;
                        }
                        return true;
                    }
                });

                descriptor.close();
                netProgress.update(file);
            }
        }
    });
}

function getContentTypeFromResponseHead(value)
{
    var values = value.split("\r\n");
    for (var i=0; i<values.length; i++)
    {
        var option = values[i].split(": ");
        var headerName = option[0];
        if (headerName && headerName.toLowerCase() == "content-type")
            return option[1];
    }
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

function now()
{
    return (new Date()).getTime();
}

function getFrameLevel(win)
{
    var level = 0;

    for (; win && (win != win.parent) && (win.parent instanceof Window); win = win.parent)
        ++level;

    return level;
}

// ************************************************************************************************

Firebug.NetMonitor.Utils =
{
    findHeader: function(headers, name)
    {
        if (!headers)
            return null;

        name = name.toLowerCase();
        for (var i = 0; i < headers.length; ++i)
        {
            var headerName = headers[i].name.toLowerCase();
            if (headerName == name)
                return headers[i].value;
        }
    },

    formatPostText: function(text)
    {
        if (text instanceof XMLDocument)
            return getElementXML(text.documentElement);
        else
            return text;
    },

    getPostText: function(file, context, noLimit)
    {
        if (!file.postText)
        {
            file.postText = readPostTextFromRequest(file.request, context);

            if (!file.postText && context)
                file.postText = readPostTextFromPage(file.href, context);
        }

        if (!file.postText)
            return file.postText;

        var limit = Firebug.netDisplayedPostBodyLimit;
        if (file.postText.length > limit && !noLimit)
        {
            return cropString(file.postText, limit,
                "\n\n... " + $STR("net.postDataSizeLimitMessage") + " ...\n\n");
        }

        return file.postText;
    },

    getResponseText: function(file, context)
    {
        // The response can be also empty string so, check agains "undefined".
        return (typeof(file.responseText) != "undefined") ?
            file.responseText :
            context.sourceCache.loadText(file.href, file.method, file);
    },

    isURLEncodedRequest: function(file, context)
    {
        var text = Utils.getPostText(file, context);
        if (text && text.toLowerCase().indexOf("content-type: application/x-www-form-urlencoded") == 0)
            return true;

        // The header value doesn't have to be always exactly "application/x-www-form-urlencoded",
        // there can be even charset specified. So, use indexOf rather than just "==".
        var headerValue = Utils.findHeader(file.requestHeaders, "content-type");
        if (headerValue && headerValue.indexOf("application/x-www-form-urlencoded") == 0)
            return true;

        return false;
    },

    isMultiPartRequest: function(file, context)
    {
        var text = Utils.getPostText(file, context);
        if (text && text.toLowerCase().indexOf("content-type: multipart/form-data") == 0)
            return true;
        return false;
    },

    getMimeType: function(mimeType, uri)
    {
        if (!mimeType || !(mimeCategoryMap.hasOwnProperty(mimeType)))
        {
            var ext = getFileExtension(uri);
            if (!ext)
                return mimeType;
            else
            {
                var extMimeType = mimeExtensionMap[ext.toLowerCase()];
                return extMimeType ? extMimeType : mimeType;
            }
        }
        else
            return mimeType;
    },

    getDateFromSeconds: function(s)
    {
        var d = new Date();
        d.setTime(s*1000);
        return d;
    },

    getHttpHeaders: function(request, file)
    {
        if (!(request instanceof Ci.nsIHttpChannel))
            return;

        // xxxHonza: is there any problem to do this in requestedFile method?
        file.method = request.requestMethod;
        file.urlParams = parseURLParams(file.href);

        try
        {
            file.status = request.responseStatus;
        }
        catch (e) { }

        try
        {
            file.mimeType = Utils.getMimeType(request.contentType, request.name);
        }
        catch (e) { }

        if (!Firebug.collectHttpHeaders)
            return;

        try
        {
            if (!file.requestHeaders)
            {
                var requestHeaders = [];
                request.visitRequestHeaders({
                    visitHeader: function(name, value)
                    {
                        requestHeaders.push({name: name, value: value});
                    }
                });
                file.requestHeaders = requestHeaders;
            }
        }
        catch (e) { }

        try
        {
            if (!file.responseHeaders)
            {
                var responseHeaders = [];
                request.visitResponseHeaders({
                    visitHeader: function(name, value)
                    {
                        responseHeaders.push({name: name, value: value});
                    }
                });
                file.responseHeaders = responseHeaders;
            }
        }
        catch (e) { }
    },

    isXHR: function(request)
    {
        try
        {
            var callbacks = request.notificationCallbacks;
            var xhrRequest = callbacks ? callbacks.getInterface(Ci.nsIXMLHttpRequest) : null;
            return (xhrRequest != null);
        }
        catch (exc)
        {
        }

       return false;
    },

    getFileCategory: function(file)
    {
        if (file.category)
        {
            return file.category;
        }

        if (file.isXHR)
        {
            return file.category = "xhr";
        }

        if (!file.mimeType)
        {
            var ext = getFileExtension(file.href);
            if (ext)
                file.mimeType = mimeExtensionMap[ext.toLowerCase()];
        }

        var mimeType = file.mimeType;
        if (mimeType)
            mimeType = mimeType.split(";")[0];

        return (file.category = mimeCategoryMap[mimeType]);
    }
};

var Utils = Firebug.NetMonitor.Utils;

// xxxHonza: should ba shared via lib.js
function safeGetName(request)
{
    try
    {
        return request.name;
    }
    catch (exc)
    {
    }

    return null;
}

// ************************************************************************************************

// HTTP listener - based on firebug-http-observer component
// This observer is used for observing the first document http-on-modify-request
// and http-on-examine-response events, which are fired before the context
// is initialized (initContext method call). Without this observer this events
// would be lost and the time measuring would be wrong.
//
// This observer stores these early requests in helper array (contexts) and maps
// them to appropriate tab - initContext then uses the array in order to access it.
//-----------------------------------------------------------------------------

Firebug.NetMonitor.NetHttpObserver =
{
    registered: false,

    registerObserver: function()
    {
        if (this.registered)
            return;

        httpObserver.addObserver(this, "firebug-http-event", false);
        this.registered = true;
    },

    unregisterObserver: function()
    {
        if (!this.registered)
            return;

        httpObserver.removeObserver(this, "firebug-http-event");
        this.registered = false;
    },

    /* nsIObserve */
    observe: function(subject, topic, data)
    {
        try
        {
            if (!(subject instanceof Ci.nsIHttpChannel))
                return;

            var win = getWindowForRequest(subject);
            var context = TabWatcher.getContextByWindow(win);

            // The context doesn't have to exist yet. In such cases a temp Net context is
            // created within onModifyRequest.

            // Some requests are not associated with any page (e.g. favicon).
            // These are ignored as Net panel shows only page requests.
            var tabId = win ? Firebug.getTabIdForWindow(win) : null;
            if (!tabId)
            {
                return;
            }

            if (topic == "http-on-modify-request")
                this.onModifyRequest(subject, win, tabId, context);
            else if (topic == "http-on-examine-response")
                this.onExamineResponse(subject, win, tabId, context);
            else if (topic == "http-on-examine-cached-response")
                this.onExamineCachedResponse(subject, win, tabId, context);
        }
        catch (err)
        {
        }
    },

    onModifyRequest: function(request, win, tabId, context)
    {
        var name = request.URI.asciiSpec;
        var origName = request.originalURI.asciiSpec;
        var isRedirect = (name != origName);

        // We only need to create a new context if this is a top document uri (not frames).
        if ((request.loadFlags & LOAD_DOCUMENT_URI) &&
            request.loadGroup && request.loadGroup.groupObserver &&
            win == win.parent && !isRedirect)
        {
            var browser = getBrowserForWindow(win);
            if (!TabWatcher.shouldCreateContext(browser, name, null))
            {
                return;
            }

            // Create a new network context prematurely.
            if (!contexts[tabId])
            {
                contexts[tabId] = new NetProgress(null);
            }
        }

        var networkContext = contexts[tabId];
        if (!networkContext)
            networkContext = context ? context.netProgress : null;

        if (networkContext)
        {
            networkContext.post(startFile, [request, win]);

            // If activity-distributor is available (Fx 3.6) it's used instead.
            if (!Ci.nsIHttpActivityDistributor)
            {
                var xhr = Utils.isXHR(request);
                networkContext.post(requestedFile, [request, now(), win, xhr]);
            }
        }
    },

    onExamineResponse: function(request, win, tabId, context)
    {
        var networkContext = contexts[tabId];
        if (!networkContext)
            networkContext = context ? context.netProgress : null;

        var info = new Object();
        info.responseStatus = request.responseStatus;
        info.responseStatusText = request.responseStatusText;

        // Initialize info.postText property.
        info.request = request;
        Utils.getPostText(info, context);

        if (networkContext)
            networkContext.post(respondedFile, [request, now(), info]);
    },

    onExamineCachedResponse: function(request, win, tabId, context)
    {
        var networkContext = contexts[tabId];
        if (!networkContext)
            networkContext = context ? context.netProgress : null;

        if (networkContext)
            networkContext.post(respondedCacheFile, [request, now()]);
    },

    /* nsISupports */
    QueryInterface: function(iid)
    {
        if (iid.equals(Ci.nsISupports) ||
            iid.equals(Ci.nsIObserver)) {
             return this;
         }

        throw Cr.NS_ERROR_NO_INTERFACE;
    }
}

// ************************************************************************************************
// Activity Observer

Firebug.NetMonitor.NetHttpActivityObserver =
{
    registered: false,

    registerObserver: function()
    {
        if (!Ci.nsIHttpActivityDistributor)
            return;

        if (this.registered)
            return;

        var distributor = this.getActivityDistributor();
        if (!distributor)
            return;

        distributor.addObserver(this);
        this.registered = true;
    },

    unregisterObserver: function()
    {
        if (!Ci.nsIHttpActivityDistributor)
            return;

        if (!this.registered)
            return;

        var distributor = this.getActivityDistributor();
        if (!distributor)
            return;

        distributor.removeObserver(this);
        this.registered = false;
    },

    getActivityDistributor: function()
    {
        if (!this.activityDistributor)
        {
            try
            {
                var hadClass = Cc["@mozilla.org/network/http-activity-distributor;1"];
                if (!hadClass)
                    return null;

                this.activityDistributor = hadClass.getService(Ci.nsIHttpActivityDistributor);

            }
            catch (err)
            {
            }
        }
        return this.activityDistributor;
    },

    /* nsIActivityObserver */
    observeActivity: function(httpChannel, activityType, activitySubtype, timestamp,
        extraSizeData, extraStringData)
    {
        try
        {
            if (httpChannel instanceof Ci.nsIHttpChannel)
                this.observeRequest(httpChannel, activityType, activitySubtype, timestamp,
                    extraSizeData, extraStringData);
        }
        catch (exc)
        {
            FBTrace.sysout("net.observeActivity: EXCEPTION "+exc, exc);
        }
    },

    observeRequest: function(httpChannel, activityType, activitySubtype, timestamp,
        extraSizeData, extraStringData)
    {
        var win = getWindowForRequest(httpChannel);
        if (!win)
        {
            var index = activeRequests.indexOf(httpChannel);
            if (!(win = activeRequests[index+1]))
                return;
        }

        var context = TabWatcher.getContextByWindow(win);
        var tabId = Firebug.getTabIdForWindow(win);
        if (!(tabId && win))
            return;

        var networkContext = contexts[tabId];
        if (!networkContext)
            networkContext = context ? context.netProgress : null;

        if (!networkContext)
            return;

        var time = new Date();
        time.setTime(timestamp/1000);

        time = time.getTime();

        if (activityType == nsIHttpActivityObserver.ACTIVITY_TYPE_HTTP_TRANSACTION)
        {
            if (activitySubtype == nsIHttpActivityObserver.ACTIVITY_SUBTYPE_REQUEST_HEADER)
            {
                activeRequests.push(httpChannel);
                activeRequests.push(win);

                networkContext.post(requestHeadersFile, [httpChannel, time, extraStringData]);
            }
            else if (activitySubtype == nsIHttpActivityObserver.ACTIVITY_SUBTYPE_TRANSACTION_CLOSE)
            {
                var index = activeRequests.indexOf(httpChannel);
                activeRequests.splice(index, 2);

                networkContext.post(closedFile, [httpChannel, time]);
            }
            else if (activitySubtype == nsIHttpActivityObserver.ACTIVITY_SUBTYPE_RESPONSE_HEADER)
            {
                networkContext.post(responseHeadersFile, [httpChannel, time, extraStringData]);
            }
        }

        if (activityType == nsIHttpActivityObserver.ACTIVITY_TYPE_HTTP_TRANSACTION)
        {
            if (activitySubtype == nsIHttpActivityObserver.ACTIVITY_SUBTYPE_REQUEST_HEADER)
                networkContext.post(requestedFile, [httpChannel, time, win, Utils.isXHR(httpChannel)]);
            if (activitySubtype == nsIHttpActivityObserver.ACTIVITY_SUBTYPE_REQUEST_BODY_SENT)
                networkContext.post(bodySentFile, [httpChannel, time]);
            else if (activitySubtype == nsIHttpActivityObserver.ACTIVITY_SUBTYPE_RESPONSE_START)
                networkContext.post(completedFile, [httpChannel, time]);
            else if (activitySubtype == nsIHttpActivityObserver.ACTIVITY_SUBTYPE_RESPONSE_COMPLETE)
                networkContext.post(completeFile, [httpChannel, time, extraSizeData]);
        }
        else if (activityType == nsIHttpActivityObserver.ACTIVITY_TYPE_SOCKET_TRANSPORT)
        {
            if (activitySubtype == nsISocketTransport.STATUS_RESOLVING)
                networkContext.post(resolvingFile, [httpChannel, time]);
            else if (activitySubtype == nsISocketTransport.STATUS_CONNECTING_TO)
                networkContext.post(connectingFile, [httpChannel, time]);
            else if (activitySubtype == nsISocketTransport.STATUS_CONNECTED_TO)
                networkContext.post(connectedFile, [httpChannel, time]);
            else if (activitySubtype == nsISocketTransport.STATUS_SENDING_TO)
                networkContext.post(sendingFile, [httpChannel, time, extraSizeData]);
            else if (activitySubtype == nsISocketTransport.STATUS_WAITING_FOR)
                networkContext.post(waitingForFile, [httpChannel, time]);
            else if (activitySubtype == nsISocketTransport.STATUS_RECEIVING_FROM)
                networkContext.post(receivingFile, [httpChannel, time, extraSizeData]);
        }
    },

    /* nsISupports */
    QueryInterface: function(iid)
    {
        if (iid.equals(Ci.nsISupports) ||
            iid.equals(Ci.nsIActivityObserver)) {
            return this;
         }

        throw Cr.NS_ERROR_NO_INTERFACE;
    }
}

var NetHttpActivityObserver = Firebug.NetMonitor.NetHttpActivityObserver;

// ************************************************************************************************
// Activity Observer Tracing Support

function getTimeLabel(date)
{
    var m = date.getMinutes() + "";
    var s = date.getSeconds() + "";
    var ms = date.getMilliseconds() + "";
    return "[" + ((m.length > 1) ? m : "0" + m) + ":" +
        ((s.length > 1) ? s : "0" + s) + "." +
        ((ms.length > 2) ? ms : ((ms.length > 1) ? "0" + ms : "00" + ms)) + "]";
}

function getActivityTypeDescription(a)
{
    switch (a)
    {
    case nsIHttpActivityObserver.ACTIVITY_TYPE_SOCKET_TRANSPORT:
        return "ACTIVITY_TYPE_SOCKET_TRANSPORT";
    case nsIHttpActivityObserver.ACTIVITY_TYPE_HTTP_TRANSACTION:
        return "ACTIVITY_TYPE_HTTP_TRANSACTION";
    default:
        return a;
    }
}

function getActivitySubtypeDescription(a)
{
    switch (a)
    {
    case nsIHttpActivityObserver.ACTIVITY_SUBTYPE_REQUEST_HEADER:
        return "ACTIVITY_SUBTYPE_REQUEST_HEADER";
    case nsIHttpActivityObserver.ACTIVITY_SUBTYPE_REQUEST_BODY_SENT:
          return "ACTIVITY_SUBTYPE_REQUEST_BODY_SENT";
    case nsIHttpActivityObserver.ACTIVITY_SUBTYPE_RESPONSE_START:
        return "ACTIVITY_SUBTYPE_RESPONSE_START";
    case nsIHttpActivityObserver.ACTIVITY_SUBTYPE_RESPONSE_HEADER:
        return "ACTIVITY_SUBTYPE_RESPONSE_HEADER";
    case nsIHttpActivityObserver.ACTIVITY_SUBTYPE_RESPONSE_COMPLETE:
        return "ACTIVITY_SUBTYPE_RESPONSE_COMPLETE";
    case nsIHttpActivityObserver.ACTIVITY_SUBTYPE_TRANSACTION_CLOSE:
        return "ACTIVITY_SUBTYPE_TRANSACTION_CLOSE";

    case nsISocketTransport.STATUS_RESOLVING:
        return "STATUS_RESOLVING";
    case nsISocketTransport.STATUS_CONNECTING_TO:
        return "STATUS_CONNECTING_TO";
    case nsISocketTransport.STATUS_CONNECTED_TO:
        return "STATUS_CONNECTED_TO";
    case nsISocketTransport.STATUS_SENDING_TO:
        return "STATUS_SENDING_TO";
    case nsISocketTransport.STATUS_WAITING_FOR:
        return "STATUS_WAITING_FOR";
    case nsISocketTransport.STATUS_RECEIVING_FROM:
        return "STATUS_RECEIVING_FROM";

    default:
        return a;
    }
}

function getPageTitle(context)
{
    var title = context.getTitle();
    return (title) ? title : context.getName();
}

// ************************************************************************************************
// Helper for tracing

function getPrintableTime()
{
    var date = new Date();
    return "(" + date.getSeconds() + ":" + date.getMilliseconds() + ")";
}

// ************************************************************************************************

Firebug.NetMonitor.TraceListener =
{
    // Called when console window is loaded.
    onLoadConsole: function(win, rootNode)
    {
    },

    // Called when a new message is logged in to the trace-console window.
    onDump: function(message)
    {
        var index = message.text.indexOf("net.");
        if (index == 0)
        {
            message.text = message.text.substr("net.".length);
            message.text = trim(message.text);
            message.type = "DBG_NET";
        }

        var prefix = "activityObserver.";
        var index = message.text.indexOf(prefix);
        if (index == 0)
        {
            message.text = message.text.substr(prefix.length);
            message.text = trim(message.text);
            message.type = "DBG_ACTIVITYOBSERVER";
        }
    }
};

// ************************************************************************************************

var NetPanelSearch = function(panel, rowFinder)
{
    var panelNode = panel.panelNode;
    var doc = panelNode.ownerDocument;
    var searchRange, startPt;

    // Common search object methods.
    this.find = function(text, reverse, caseSensitive)
    {
        this.text = text;

        finder.findBackwards = !!reverse;
        finder.caseSensitive = !!caseSensitive;

        this.currentRow = this.getFirstRow();
        this.resetRange();

        return this.findNext(false, false, reverse, caseSensitive);
    };

    this.findNext = function(wrapAround, sameNode, reverse, caseSensitive)
    {
        while (this.currentRow)
        {
            var match = this.findNextInRange(reverse, caseSensitive);
            if (match)
                return match;

            if (this.shouldSearchResponses())
                this.findNextInResponse(reverse, caseSensitive);

            this.currentRow = this.getNextRow(wrapAround, reverse);

            if (this.currentRow)
                this.resetRange();
        }
    };

    // Internal search helpers.
    this.findNextInRange = function(reverse, caseSensitive)
    {
        if (this.range)
        {
            startPt = doc.createRange();
            if (reverse)
                startPt.setStartBefore(this.currentNode);
            else
                startPt.setStart(this.currentNode, this.range.endOffset);

            this.range = finder.Find(this.text, searchRange, startPt, searchRange);
            if (this.range)
            {
                this.currentNode = this.range ? this.range.startContainer : null;
                return this.currentNode ? this.currentNode.parentNode : null;
            }
        }

        if (this.currentNode)
        {
            startPt = doc.createRange();
            if (reverse)
                startPt.setStartBefore(this.currentNode);
            else
                startPt.setStartAfter(this.currentNode);
        }

        this.range = finder.Find(this.text, searchRange, startPt, searchRange);
        this.currentNode = this.range ? this.range.startContainer : null;
        return this.currentNode ? this.currentNode.parentNode : null;
    },

    this.findNextInResponse = function(reverse, caseSensitive)
    {
        var file = Firebug.getRepObject(this.currentRow);
        if (!file)
            return;

        var scanRE = Firebug.Search.getTestingRegex(this.text);
        if (scanRE.test(file.responseText))
        {
            if (!hasClass(this.currentRow, "opened"))
                NetRequestEntry.toggleHeadersRow(this.currentRow);

            var netInfoRow = this.currentRow.nextSibling;
            var netInfoBox = netInfoRow.getElementsByClassName("netInfoBody").item(0);
            NetInfoBody.selectTabByName(netInfoBox, "Response");

            // Before the search is started, the new content must be properly
            // layouted within the page. The layout is executed by reading
            // the following property.
            // xxxHonza: This workaround can be removed as soon as #488427 is fixed.
            doc.body.offsetWidth;
        }
    },

    // Helpers
    this.resetRange = function()
    {
        searchRange = doc.createRange();
        searchRange.setStart(this.currentRow, 0);
        searchRange.setEnd(this.currentRow, this.currentRow.childNodes.length);

        startPt = searchRange;
    }

    this.getFirstRow = function()
    {
        var table = panelNode.getElementsByClassName("netTable").item(0);
        return table.firstChild.firstChild;
    }

    this.getNextRow = function(wrapAround, reverse)
    {
        // xxxHonza: reverse searching missing.
        for (var sib = this.currentRow.nextSibling; sib; sib = sib.nextSibling)
        {
            if (this.shouldSearchResponses())
                return sib;
            else if (hasClass(sib, "netRow"))
                return sib;
        }

        return wrapAround ? this.getFirstRow() : null;;
    }

    this.shouldSearchResponses = function()
    {
        return Firebug["netSearchResponseBody"];
    }
};

// ************************************************************************************************
// Breakpoints

Firebug.NetMonitor.DebuggerListener =
{
    getBreakpoints: function(context, groups)
    {
        if (context.netProgress && !context.netProgress.breakpoints.isEmpty())
            groups.push(context.netProgress.breakpoints);
    },
};

Firebug.NetMonitor.BreakpointRep = domplate(Firebug.Rep,
{
    inspectable: false,

    tag:
        DIV({"class": "breakpointRow focusRow", _repObject: "$bp",
            role: "option", "aria-checked": "$bp.checked"},
            DIV({"class": "breakpointBlockHead", onclick: "$onEnable"},
                INPUT({"class": "breakpointCheckbox", type: "checkbox",
                    _checked: "$bp.checked", tabindex : "-1"}),
                SPAN({"class": "breakpointName", title: "$bp|getTitle"}, "$bp|getName"),
                IMG({"class": "closeButton", src: "blank.gif", onclick: "$onRemove"})
            ),
            DIV({"class": "breakpointCondition"},
                SPAN("$bp.condition")
            )
        ),

    getTitle: function(bp)
    {
        return bp.href;
    },

    getName: function(bp)
    {
        return getFileName(bp.href);
    },

    onRemove: function(event)
    {
        cancelEvent(event);

        if (!hasClass(event.target, "closeButton"))
            return;

        var bpPanel = Firebug.getElementPanel(event.target);
        var context = bpPanel.context;

        // Remove from list of breakpoints.
        var row = getAncestorByClass(event.target, "breakpointRow");
        var bp = row.repObject;
        context.netProgress.breakpoints.removeBreakpoint(bp.href);

        // Remove from the UI.
        bpPanel.noRefresh = true;
        bpPanel.removeRow(row);
        bpPanel.noRefresh = false;

        var panel = context.getPanel(panelName, true);
        if (!panel)
            return;

        panel.enumerateRequests(function(file)
        {
            if (file.getFileURL() == bp.href)
            {
                file.row.removeAttribute("breakpoint");
                file.row.removeAttribute("disabledBreakpoint");
            }
        })
    },

    onEnable: function(event)
    {
        var checkBox = event.target;
        if (!hasClass(checkBox, "breakpointCheckbox"))
            return;

        var bpPanel = Firebug.getElementPanel(event.target);
        var context = bpPanel.context;

        var bp = getAncestorByClass(checkBox, "breakpointRow").repObject;
        bp.checked = checkBox.checked;

        var panel = context.getPanel(panelName, true);
        if (!panel)
            return;

        panel.enumerateRequests(function(file)
        {
            if (file.getFileURL() == bp.href)
                file.row.setAttribute("disabledBreakpoint", bp.checked ? "false" : "true");
        });
    },

    supportsObject: function(object)
    {
        return object instanceof Breakpoint;
    }
});

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

function Breakpoint(href)
{
    this.href = href;
    this.checked = true;
    this.condition = "";
    this.onEvaluateFails = bind(this.onEvaluateFails, this);
    this.onEvaluateSucceeds =  bind(this.onEvaluateSucceeds, this);
}

Breakpoint.prototype =
{
    evaluateCondition: function(context, file)
    {
        try
        {
            var scope = {};

            var params = file.urlParams;
            for (var i=0; params && i<params.length; i++)
            {
                var param = params[i];
                scope[param.name] = param.value;
            }

            scope["$postBody"] = Utils.getPostText(file, context);

            // The properties of scope are all strings; we pass them in then
            // unpack them using 'with'. The function is called immediately.
            var expr = "(function (){var scope = " + JSON.stringify(scope) +
                "; with (scope) { return  " + this.condition + ";}})();"

            // The callbacks will set this if the condition is true or if the eval faults.
            delete context.breakingCause;

            var rc = Firebug.CommandLine.evaluate(expr, context, null, context.window,
                this.onEvaluateSucceeds, this.onEvaluateFails );

            return !!context.breakingCause;
        }
        catch (err)
        {
        }

        return false;
    },

    onEvaluateSucceeds: function(result, context)
    {
        // Don't break if the result is false.
        if (!result)
            return;

        context.breakingCause = {
            title: $STR("net.Break On XHR"),
            message: this.condition
        };
    },

    onEvaluateFails: function(result, context)
    {
        // Break if there is an error when evaluating the condition (to display the error).
        context.breakingCause = {
            title: $STR("net.Break On XHR"),
            message: "Breakpoint condition evaluation fails ",
            prevValue: this.condition,
            newValue:result
        };
    },
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

function NetBreakpointGroup()
{
    this.breakpoints = [];
}

NetBreakpointGroup.prototype = extend(new Firebug.Breakpoint.BreakpointGroup(),
{
    name: "netBreakpoints",
    title: $STR("net.label.XHR Breakpoints"),

    addBreakpoint: function(href)
    {
        this.breakpoints.push(new Breakpoint(href));
    },

    removeBreakpoint: function(href)
    {
        var bp = this.findBreakpoint(href);
        remove(this.breakpoints, bp);
    },

    matchBreakpoint: function(bp, args)
    {
        var href = args[0];
        return bp.href == href;
    }
});

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

Firebug.NetMonitor.ConditionEditor = function(doc)
{
    Firebug.Breakpoint.ConditionEditor.apply(this, arguments);
}

Firebug.NetMonitor.ConditionEditor.prototype = domplate(Firebug.Breakpoint.ConditionEditor.prototype,
{
    endEditing: function(target, value, cancel)
    {
        if (cancel)
            return;

        var file = target.repObject;
        var panel = Firebug.getElementPanel(target);
        var bp = panel.context.netProgress.breakpoints.findBreakpoint(file.getFileURL());
        if (bp)
            bp.condition = value;
    }
});

// ************************************************************************************************
// Browser Cache

Firebug.NetMonitor.BrowserCache =
{
    cacheDomain: "browser.cache",

    isEnabled: function()
    {
        var diskCache = Firebug.getPref(this.cacheDomain, "disk.enable");
        var memoryCache = Firebug.getPref(this.cacheDomain, "memory.enable");
        return diskCache && memoryCache;
    },

    enable: function(state)
    {
        Firebug.setPref(this.cacheDomain, "disk.enable", state);
        Firebug.setPref(this.cacheDomain, "memory.enable", state);
    }
}

// ************************************************************************************************

Firebug.registerRep(Firebug.NetMonitor.NetRequestTable);
Firebug.registerActivableModule(Firebug.NetMonitor);
Firebug.registerPanel(NetPanel);
Firebug.registerRep(Firebug.NetMonitor.BreakpointRep);

// ************************************************************************************************
}});
