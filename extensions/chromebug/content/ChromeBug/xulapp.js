/* See license.txt for terms of usage */

FBL.ns(function() { with (FBL) {

// ***********************************************************************************
// Shorcuts and Services

const Cc = Components.classes;
const Ci = Components.interfaces;

const docShellTypeNames = ["Chrome", "Content", "ContentWrapper", "ChromeWrapper"]; // see nsIDocShellTreeItem

var windowMediator = Cc["@mozilla.org/appshell/window-mediator;1"]
        .getService(Ci.nsIWindowMediator);

var windowWatcher = Cc["@mozilla.org/embedcomp/window-watcher;1"]
                       .getService(Ci.nsIWindowWatcher);

const observerService = Cc["@mozilla.org/observer-service;1"].getService(Ci.nsIObserverService);

var panelName = "window";

const reChromebug = /^chrome:\/\/chromebug\//;

// ***********************************************************************************
// Chromebug XULApp Module

/**
 * Implementation of a Module & Panel for XULApp viewer.
 */
Chromebug.XULAppModule = extend(Firebug.Module,
{

    // ****************************************************
    // A XUL App's windows

    xulWindows: [],  // all xul_windows
    xulWindowTags: [], // co-indexed strings for xulWindows

    getDocShellByDOMWindow: function(domWindow)
    {
       if (domWindow instanceof Ci.nsIInterfaceRequestor)
       {
           try
           {
               var navi = domWindow.getInterface(Ci.nsIWebNavigation);
           }
           catch (exc)
           {
               FBTrace.sysout("Chromebug getDocShellByDOMWindow, domWindow.getInterface FAILS", domWindow);
           }
           if (navi instanceof Ci.nsIDocShellTreeItem)
           {
               return navi;
           }
           else
               FBTrace.sysout("Chromebug getDocShellByDOMWindow, nsIWebNavigation notA nsIDowShellTreeItem");
        }
        else
        {
            FBTrace.sysout("Chromebug getDocShellByDOMWindow, window notA nsIInterfaceRequestor:", domWindow);
            FBTrace.sysout("getDocShellByDOMWindow domWindow.location:"+domWindow.location, " isA nsIDOMWindow: "+(domWindow instanceof Ci.nsIDOMWindow));
        }
    },

    getDocumentTypeByDOMWindow: function(domWindow)
    {
        var docShell = this.getDocShellByDOMWindow(domWindow);
        if (docShell instanceof Ci.nsIDocShellTreeItem)
        {
            var typeIndex = docShell.itemType;
            return docShellTypeNames[typeIndex];
        }
        else
            FBTrace.sysout("Chromebug.getDocumentType, docShell is not a nsIDocShellTreeItem:", docShell);
    },

    getXULWindowByRootDOMWindow: function(domWindow)
    {
        if(FBTrace.DBG_CHROMEBUG)
            FBTrace.sysout("XULAppModule.getXULWindowByRootDOMWindow "+domWindow.location.href+" for xulWIndows.length="+this.xulWindows.length+"\n");
        for (var i = 0; i < this.xulWindows.length; i++)
        {
            var xul_window = this.xulWindows[i];
            var xul_windows_domWindow = this.getDOMWindowByXULWindow(xul_window);
            if (FBTrace.DBG_CHROMEBUG)
            {
                if (xul_windows_domWindow)
                    FBTrace.sysout("getXULWindowByRootDOMWindow comparing "+xul_windows_domWindow.location.href+" with "+domWindow.location.href+"\n");
                else
                    FBTrace.sysout("getXULWindowByRootDOMWindow no domWindow for xul_window #"+i, xul_window);
            }

            if (xul_windows_domWindow == domWindow)
                return xul_window;
        }
    },

    getDOMWindowByXULWindow: function(xul_window)
    {
        return this.getDOMWindowByDocShell(xul_window.docShell);
    },

    getDOMWindowByDocShell: function(docShell)
    {
        try
        {
            if (docShell)
            {
                if (docShell instanceof Ci.nsIInterfaceRequestor)
                {
                    var win = docShell.getInterface(Ci.nsIDOMWindow);
                    if (win)
                        return win;
                    else
                    {
                        if (FBTrace.DBG_ERRORS)
                            FBTrace.sysout("getDOMWindowByXULWindow xul_win.docShell has nsIInterfaceRequestor but not nsIDOMWindow\n");
                    }
                }
                else
                {
                    if (FBTrace.DBG_ERRORS)
                        FBTrace.sysout("getDOMWindowByXULWindow xul_win.docShell has no nsIInterfaceRequestor\n");
                }
            }
            else
            {
                if (FBTrace.DBG_ERRORS)
                    FBTrace.sysout("getDOMWindowByXULWindow xul_win has no docShell");
            }
        }
        catch (exc)
        {
            if (FBTrace.DBG_ERRORS)
                FBTrace.sysout("getDOMWindowByXULWindow FAILS "+exc, exc);
        }
    },

    getDocShellTreeItem: function(xul_window)
    {
        var docShell = xul_window.docShell;
        if (docShell instanceof Ci.nsIInterfaceRequestor)
        {
            var item = docShell.getInterface(Ci.nsIDocShellTreeItem);
            if (item instanceof Ci.nsIDocShellTreeItem)
                return item;
        }
        FBTrace.sysout("xulapp.getDocShellTreeItem fails for docShell ", docShell);
    },

    getXULWindows: function()
    {
        return this.xulWindows;
    },

    getXULWindowIndex: function(xul_win)
    {
        return this.xulWindows.indexOf(xul_win);
    },

    getXULWindowTags: function()
    {
        return this.xulWindowTags;
    },

    getXULWindowTag: function(xul_window)
    {
        var i = this.getXULWindowIndex(xul_window);
        return this.xulWindowTags[i];
    },

    getXULWindowByTag: function(xul_window_tag)
    {
        var i = this.xulWindowTags.indexOf(xul_window_tag);
        return this.xulWindows[i];
    },

    eachXULWindow: function(iterator)
    {
        for(var i = 0; i < this.xulWindows.length; i++ )
        {
            var rc = iterator(this.xulWindows[i]);
            if (rc)
                return rc;
        }
        return null;
    },

    eachDocShell: function(docShell, content_chrome_OR_all, iterator)
    {
        var treeItemType = Ci.nsIDocShellTreeItem.typeAll;
        if (content_chrome_OR_all == "chrome")
            treeItemType = Ci.nsIDocShellTreeItem.typeChrome;
        else if (content_chrome_OR_all == "content")
            treeItemType = Ci.nsIDocShellTreeItem.typeContent;
        // From inspector@mozilla.org inspector.js appendContainedDocuments
        // Load all the window's content docShells
        var containedDocShells = docShell.getDocShellEnumerator(treeItemType,
                                          Ci.nsIDocShell.ENUMERATE_FORWARDS);
        while (containedDocShells.hasMoreElements())
        {
            try
            {
                var childDocShell = containedDocShells.getNext().QueryInterface(Ci.nsIDocShell);
                iterator(childDocShell);
            }
            catch (exc)
            {
                FBTrace.sysout("XULAppModule.eachDocShell FAILED", exc);
            }
        }
        return true;
    },

    iterateOuterDOMWindows: function(handler)
    {
        for(var i = 0; i < this.xulWindows.length; i++)
        {
            var xul_window = this.xulWindows[i];
            Chromebug.XULAppModule.eachDocShell
            (
                xul_window.docShell, true, function(childDocShell)
                {
                    if (childDocShell.contentViewer) // nsiDocShell.nsIContentViewer
                    {
                        var childDoc = childDocShell.contentViewer.DOMDocument;

                        if (childDoc instanceof Ci.nsIDOMDocument && childDoc.defaultView instanceof Ci.nsIDOMWindow)
                            //FBL.iterateWindows(childDoc.defaultView, handler);
                            handler(childDoc.defaultView);
                    }
                }
            );
        }
    },

    getDOMWindowTreeByXULWindow: function(xulWindow)
    {
        var docShell = xulWindow.docShell;
        return this.getDOMWindowTreeByDocShell(docShell);
    },

    getDOMWindowTreeByDocShell: function(docShell)
    {
        var domWindow = Chromebug.XULAppModule.getDOMWindowByDocShell(docShell);
        var url = safeGetWindowLocation(domWindow);

        var domWindowsByURL = {};

        this.eachDocShell(docShell, "all", function visitSubShells(childShell)
        {
            if (childShell == docShell)
            {
                domWindowsByURL[url] = domWindow;
                return;
            }

            if (!domWindowsByURL)
                domWindowsByURL = {};

            var childDOMWindow = Chromebug.XULAppModule.getDOMWindowByDocShell(childShell);

            var key = Chromebug.XULAppModule.getKeyByDOMWindow(childDOMWindow, domWindowsByURL);

            var familyTree = Chromebug.XULAppModule.getDOMWindowTreeByDocShell(childShell);  // recurse

            domWindowsByURL[key] = familyTree; // either a window or a table
        });

        return domWindowsByURL;
    },

    getKeyByDOMWindow: function(domWindow, domWindowsByURL)
    {
        var url = safeGetWindowLocation(domWindow);
        var title = domWindow.document.title;
        var key = url +" - "+title;
        if (!key)
                key = "(no location)";

        while (key in domWindowsByURL)
                key = key +".";

        return key;
    },

    getDOMWindowTree: function()
    {
        var domWindowsByURL = {};

        var enumerator = windowMediator.getXULWindowEnumerator(null);  // null means all
        while(enumerator.hasMoreElements())
        {
             var xul_window = enumerator.getNext();
             if (xul_window instanceof Ci.nsIXULWindow)
             {
                 if (xul_window.docShell)
                 {
                     var domWindow = this.getDOMWindowByXULWindow(xul_window);
                     var familyTree = this.getDOMWindowTreeByXULWindow(xul_window);  // recurse

                     var key = this.getKeyByDOMWindow(domWindow, domWindowsByURL);
                     domWindowsByURL[key] = familyTree; // either a window or a table
                 }
                 else
                     FBTrace.sysout("A XUL Window without a docShell??", xul_window);
             }
             else
             {
                 FBTrace.sysout("getXULWindowEnumerator gave element that was not nsIXULWindow!", xul_window);
             }
         }
        return domWindowsByURL;
    },

    getWindowMediatorDOMWindows: function()
    {
        // http://mxr.mozilla.org/mozilla-central/source/xpfe/appshell/public/nsIWindowMediator.idl

        var domWindowsByURL = {};
        var enumerator = windowMediator.getEnumerator(null);  // null means all
        while(enumerator.hasMoreElements()) {
             var domWindow = enumerator.getNext();
             var url = safeGetWindowLocation(domWindow);
             if (url)
                 domWindowsByURL[url] = domWindow;
             else
                 domWindowsByURL["(no location)"] = domWindow; // this will overwrite, but just to notify anyway
        }
        return domWindowsByURL;
    },

    //***********************************************************************************
    // observers

    unwatchXULWindow :
    {
        observe: function(subject, topic, data)
        {
            FBTrace.sysout("xulapp unwatchXULWindow.observe xul-window-destroyed == "+topic, {subject: subject, data: data});
            // The subject is null, the window is gone, see http://mxr.mozilla.org/mozilla-central/source/xpfe/appshell/src/nsXULWindow.cpp#556

            var closers = Chromebug.XULAppModule.closers;

            while(closers.length)
                (closers.pop())();
        },
    },

    watchXULWindow :
    {
        observe: function(subject, topic, data)
        {
            FBTrace.sysout("xulapp unwatchXULWindow.observe xul-window-registered == "+topic, {subject: subject, data: data});

        },
    },

    //***********************************************************************************
    // nsIWindowMediatorListener

    onOpenWindow: function(xul_window)
    {
        try
        {
            if (xul_window instanceof Ci.nsIXULWindow)
                this.addXULWindow(xul_window);
        }
        catch (e)
        {
            FBTrace.sysout("chromebug-onOpenWindow-FAILS", e);
            FBTrace.sysout("chromebug-onOpenWindow-xul_window", xul_window);
        }
    },

    addXULWindow: function(xul_window)
    {
        if (!xul_window.docShell)
            FBTrace.sysout("Firebug.Chromebug.addXULWindow no docShell", xul_window);

        var outerDOMWindow = this.getDOMWindowByXULWindow(xul_window);

        if (outerDOMWindow == document.defaultView)
            return;  // This is my life we're talking about.

        if (outerDOMWindow.location.href == "chrome://fb4cb/content/traceConsole.xul")
            return; // don't track our own tracing console.

        this.xulWindows.push(xul_window);

        var newTag = "tag-"+this.xulWindowTagSeed++;
        this.xulWindowTags.push(newTag);  // co-indexed arrays

        dispatch(this.fbListeners, "onXULWindowAdded", [xul_window, outerDOMWindow]);

        if (FBTrace.DBG_CHROMEBUG)
            FBTrace.sysout("Chromebug.XULAppModule.addXULWindow "+outerDOMWindow.location.href+ " complete length="+this.xulWindows.length, " index="+this.getXULWindowIndex(xul_window));

        return newTag;
    },

    keypressToBreakIntoWindow: function(event, context)
    {
        if (event.charCode == 126) // twiddle '~'
        {  FBTrace.sysout("keypressToBreakIntoWindow  "+context.getName(), event);
            if (isControlShift(event))
            {
                if (FBTrace.DBG_CHROMEBUG)
                    FBTrace.sysout("keypressToBreakIntoWindow isControlShift "+context.getName(), event);
                cancelEvent(event);
                Firebug.Debugger.breakOnNext(context);
                /*
                var halter = context.window.document.getElementById("chromebugHalter");
                if (halter)
                    halter.parentNode.removeChild(halter);
                var haltingScript = "window.dump(\"halting in \"+window.location);\ndebugger;\n";
                halter = addScript(context.window.document, "chromebugHalter", haltingScript);
                if (FBTrace.DBG_CHROMEBUG)
                    FBTrace.sysout("keypressToBreakIntoWindow haltingScript "+haltingScript, halter);

                */
            }
        }
    },

    closers: [],

    onCloseWindow: function(xul_win)
    {
        this.closers.push(bind(this.cleanUpXULWindow, this, xul_win));
    },

    addCloser: function(closer)
    {
        this.closers.push(closer);
    },

    cleanUpXULWindow: function(xul_win)
    {
        try
        {
            if (xul_win instanceof Ci.nsIXULWindow)
            {
                var mark = this.getXULWindowIndex(xul_win);
                if (mark == -1)   // A window closed but we don't know which one.
                {
                    // https://bugzilla.mozilla.org/show_bug.cgi?id=325636
                    var SIP=Components.Constructor("@mozilla.org/supports-interface-pointer;1",
                        Ci.nsISupportsInterfacePointer);
                    for (var i = 0; i < this.xulWindows.length; i++)
                    {
                        var ptr = new SIP;
                        ptr.data = xul_win;
                        if (ptr.data == this.xulWindows[i])
                        {
                            mark = i;
                            if (FBTrace.DBG_CHROMEBUG)
                                FBTrace.sysout("XULAppModule.onclose: timeless nsISupportsInterfacePointer found mark="+mark+"\n");
                            break;
                        }
                    }
                }
                if (mark != -1)
                {
                    if (FBTrace.DBG_CHROMEBUG)
                        FBTrace.sysout("XULAppModule.onclose: removing getXULWindowIndex="+mark);

                    var tag = this.xulWindowTags[mark];
                    this.xulWindows.splice(mark,1);
                    this.xulWindowTags.splice(mark,1);
                }
                else
                {
                    var outerDOMWindow = this.getDOMWindowByXULWindow(xul_win);
                    var url = new String(outerDOMWindow.location);
                    if (reChromebug.test(url))
                        return; // ignore self
                    FBTrace.sysout("XULAppModule.onclose: xul_window is unknown to us at location "+outerDOMWindow.location);
                }
             }
             else
                 FBTrace.sysout("XULAppModule.onclose: not a nsIXULWindow");
        }
        catch(e)
        {
            FBTrace.sysout("XULAppModule.onClose fails ", e);
            throw "NO, do not exit";
        }
    },

    onWindowTitleChange: function(xul_win , newTitle)
    {
        if (FBTrace.DBG_CHROMEBUG)
        {
            try
            {
                var tag = this.getXULWindowTag(xul_win);
                FBTrace.sysout("XULAppModule.onWindowTitleChange tag:"+tag+" to \'"+newTitle+"\'\n");

                var outerDOMWindow = this.getDOMWindowByXULWindow(xul_win);

                if (outerDOMWindow.location.href == "chrome://fb4cb/content/traceConsole.xul")
                {
                    FBTrace.sysout("onWindowTitleChange ignoring outerDOMWindow.location.href "+outerDOMWindow.location.href+"\n");
                    this.onCloseWindow(xul_win);  // don't track our own tracing console.
                }

            }
            catch (exc) {window.dump("XULAppModule.onWindowTitleChange:"+exc+"\n");}   // sometimes FBTrace is not defined?
        }
        return;
    },

    reloadWindow: function(xul_window)
    {
        var outerDOMWindow = Chromebug.XULAppModule.getDOMWindowByXULWindow(xul_window);
        if (outerDOMWindow && outerDOMWindow instanceof Ci.nsIDOMWindow)
        {
            try
            {
                if (!this.seesionStore)
                {
                    this.sessionStore = Components.classes["@mozilla.org/browser/sessionstore;1"].
                        getService(Ci.nsISessionStore);
                }
                var storedState = sessionStore.getWindowState(outerDOMWindow);
                var ss = sessionStore;
                // save until the window is ready for state
                this.stateReloader = function(event)
                {
                    var windowToBeRestored = event.currentTarget;
                    windowToBeRestored.dump("setWindowState for "+windowToBeRestored.location+" to "+storedState+"\n");
                    windowToBeRestored.removeEventListener("DOMContentLoaded", Chromebug.XULAppModule.stateReloader, "true");
                    sessionStore.setWindowState(windowToBeRestored, storedState, true);
                    delete Chromebug.XULAppModule.stateReloader;
                }
            }
            catch (exc)
            {
                var ssEnabled = prefs.getBoolPref("browser.sessionstore.enabled");
                FBTrace.sysout("Firebug.Chromebug.reloadWindow FAILS with browser.sessionstore.enabled= "+ssEnabled, exc);
            }

            FBTrace.sysout("ChromeBug reloadWindow closing outerDOMWindow\n");
            outerDOMWindow.close();
            FBTrace.sysout("ChromeBug reloadWindow opening new window\n");
            var ff = window.open();
            return ff;
        }
        else
            FBTrace.sysout("XULAppModule.reload, no domWindow for xul_window_tag:"+xul_window_tag+"\n");
        return false;
    },

    // *****************************************************
    dispatchName: panelName,

    initialize: function(prefDomain, prefNames)
    {
        if (FBTrace.DBG_CBWINDOW)
            FBTrace.sysout("Chromebug.XULAppModule.initialize" + prefDomain);

        this.xulWindowTagSeed = FBL.getUniqueId();
        windowWatcher.registerNotification(this);
    },

    initializeUI: function()
    {
        if (FBTrace.DBG_INITIALIZE)
            FBTrace.sysout("xulapp.initializeUI -------------------------- start creating contexts --------");
        this.watchXULWindows();
    },

    watchXULWindows: function()
    {
        // get the existing windows first
        var enumerator = windowMediator.getXULWindowEnumerator(null);
        while(enumerator.hasMoreElements())
        {
            var xul_window = enumerator.getNext();
            if (xul_window instanceof Ci.nsIXULWindow)
                this.addXULWindow(xul_window);
        }
        try
        {
            // then watch for new ones
            windowMediator.addListener(this);  // removed in this.shutdown
        }
        catch(exc)
        {
            FBTrace.sysout("Chromebug.XULAppModule initialize fails", exc);
        }
    },

    shutdown: function()
    {
        try
        {
            windowWatcher.unregisterNotification(this);
            windowMediator.removeListener(this);  // added in this.initialize()
        }
        catch (exc)
        {
            FBTrace.sysout("Chromebug.XULAppModule shutdown fails", exc);
        }
    },

    initContext: function(context)
    {
        if (FBTrace.DBG_CBWINDOW)
            FBTrace.sysout("Chromebug.XULAppModule.initContext; " + context.name);
    },

    // Helpers
    addStyleSheet: function()
    {
        var panelType = Firebug.getPanelType(panelName);
        var doc = FirebugChrome.getPanelDocument(panelType);

        // Make sure the stylesheet isn't appended twice.
        if ($("cbXULApps", doc))
            return;

        var styleSheet = createStyleSheet(doc, "chrome://chromebug/skin/windowPanel.css");
        styleSheet.setAttribute("id", "cbXULApps");
        addStyleSheet(doc, styleSheet);
    },

    // UI Commands
    refresh: function(context)
    {
        var panel = context.getPanel(panelName, false);
        panel.refresh();
    }
});

// Helper shortcut
var Module = Chromebug.XULAppModule;

// ************************************************************************************************
// Firebug Cache Panel

Chromebug.XULAppPanel = function() {}

Chromebug.XULAppPanel.prototype = extend(Firebug.DOMPanel.prototype,
{
    name: panelName,
    title: "XUL Windows",
    searchable: false,
    editable: false,

    supportsObject: function(object)
    {
        if (object instanceof Ci.nsIXULWindow)
            return 10;
        else
            return 0;
    },

    getDefaultSelection: function()
    {
        return Chromebug.XULAppModule.getDOMWindowTree();
    },

    show: function(state)
    {
        this.selection = null;
        if (FBTrace.DBG_CBWINDOW)
            FBTrace.sysout("Chromebug.XULAppModule.XULAppPanel.show;", state);

        FBTrace.sysout('xulapp show enumerating windows');
        var enumerator = windowWatcher.getWindowEnumerator();
        while (enumerator.hasMoreElements())
        {
            var win = enumerator.getNext();
            if (win instanceof Ci.nsIDOMWindow)
                FBTrace.sysout("xulapp show window "+safeGetWindowLocation(win));
            else
                FBTrace.sysout("xulapp show not an nsIDOMWindow");
        }

        this.showToolbarButtons("cbWindowButtons", true);
        Firebug.DOMPanel.prototype.show.apply(this, arguments);
    },

    hide: function()
    {
        if (FBTrace.DBG_CBWINDOW)
            FBTrace.sysout("Chromebug.XULAppModule.XULAppPanel.hide;");

        this.showToolbarButtons("cbWindowButtons", false);
        Firebug.DOMPanel.prototype.hide.apply(this, arguments);
    },

    getOptionsMenuItems: function()
    {
         var items = [];
         return items;
    },

    onPrefChange: function(optionName, optionValue)
    {
        if (FBTrace.DBG_CBWINDOW)
            FBTrace.sysout("Chromebug.XULAppModule.XULAppPanel.onPrefChange; " + optionName + "=" + optionValue);
    },
});

 // ************************************************************************************************

/**
 * This template represents list of windows
 * Each tab in that window maintains its own window.
 */
Chromebug.XULAppModule.WindowList = domplate(Firebug.Rep,
{
    tableTag:
        TABLE({"class": "windowListTable", cellpadding: 0, cellspacing: 0, onclick: "$onClick"},
            TBODY(
                FOR("window", "$windowList",
                    TR({"class": "windowListRow", _repObject: "$window"},
                        TD({"class": "windowNameCol windowListCol"},
                            SPAN({"class": "windowName"},
                                "$window|getWindowName"
                            ),
                            SPAN({"class": "windowChildren"},
                                "$window|getChildWindows"
                            )
                        )
                    )
                )
            )
        ),

    windowBodyTag:
        TR({"class": "windowBodyRow", _repObject: "$window"},
            TD({"class": "windowBodyCol", colspan: 1})
        ),

    summaryTag:
        TR({"class": "summaryRow"},
            TD({"class": "totalSize summaryCol"},
                SPAN("Total Size: $windowList|getTotalSize")
            )
        ),

    getWindowName: function(xul_window)
    {
        FBTrace.sysout('getWindowName '+xul_window.title, xul_window);
        return xul_window.title;
    },

    getChildWindows: function(xul_window)
    {
        return "FIXME";
    },

    getTotalSize: function(windowList)
    {
        var totalSize = 0;
        for (var i=0; i<windowList.length; i++)
            totalSize += windowList[i].size;
        return formatSize(totalSize);
    },

    onClick: function(event)
    {
        if (isLeftClick(event))
        {
            var row = getAncestorByClass(event.target, "windowListRow");
            if (row)
            {
                this.toggleRow(row);
                cancelEvent(event);
            }
        }
    },

    expandGroup: function(row)
    {
        if (hasClass(row, "windowListRow"))
            this.toggleRow(row, true);
    },

    collapseGroup: function(row)
    {
        if (hasClass(row, "windowListRow") && hasClass(row, "opened"))
            this.toggleRow(row);
    },

    toggleRow: function(row, forceOpen)
    {
        var opened = hasClass(row, "opened");
        if (opened && forceOpen)
            return;

        toggleClass(row, "opened");
        if (hasClass(row, "opened"))
        {
            var cache = row.repObject;
            var infoBodyRow = this.windowBodyTag.insertRows({cache: cache}, row)[0];
            infoBodyRow.repObject = cache;
            this.initBody(infoBodyRow);
        }
        else
        {
            var infoBodyRow = row.nextSibling;
            row.parentNode.removeChild(infoBodyRow);
        }
    },

    initBody: function(infoBodyRow)
    {
        var tabCache = infoBodyRow.repObject;

        var entries = [];
        for (var url in tabCache.sourceCache.cache)
            entries.push(new Chromebug.XULAppModule.CacheEntry(url, tabCache.sourceCache.cache[url]));

        if (!entries.length)
        {
            infoBodyRow.firstChild.innerHTML = "There are no window entries";
            return;
        }

        // Create basic structure for list of window entries
        // and insert all window entries into it.
        var table = Chromebug.XULAppModule.WindowTable.tableTag.replace({}, infoBodyRow.firstChild);
        Chromebug.XULAppModule.WindowTable.resultTag.insertRows({entries: entries}, table.firstChild);
    }
});

// ************************************************************************************************

/**
 * This template represents an URL (displayed as a table-row within the panel)
 * that is windowd within the Firebug window.
 */
Chromebug.XULAppModule.WindowTable = domplate(Firebug.Rep,
{
    tableTag:
        TABLE({"class": "fbWindowTable", cellpadding: 0, cellspacing: 0, onclick: "$onClick"},
            TBODY()
        ),

    resultTag:
        FOR("entry", "$entries",
            TR({"class": "windowEntryRow", _repObject: "$entry"},
                TD({"class": "windowEntryCol", width: "100%"},
                    SPAN({"class": "windowEntryURL windowEntryLabel"},
                        "$entry|getURL"
                    ),
                    SPAN({"class": "windowEntrySize windowEntryLabel"},
                        "$entry|getSize"
                    )
                )
            )
        ),

    resultInfoTag:
        TR({"class": "windowEntryInfoRow", _repObject: "$entry"},
            TD({"class": "windowEntryInfoCol", colspan: 2})
        ),

    getURL: function(entry)
    {
        return entry.url;
    },

    getSize: function(entry)
    {
        try
        {
            var data = entry.lines ? entry.lines.join("") : "";
            return formatSize(data.length);
        }
        catch (err)
        {
            if (FBTrace.DBG_CBWINDOW)
                FBTrace.sysout("Chromebug.XULAppModule.WindowList.getSize; ", err);
        }
    },

    onClick: function(event)
    {
        if (isLeftClick(event))
        {
            var row = getAncestorByClass(event.target, "windowEntryRow");
            if (row)
            {
                this.toggleResultRow(row);
                cancelEvent(event);
            }
        }
    },

    toggleResultRow: function(row)
    {
        var entry = row.repObject;

        toggleClass(row, "opened");
        if (hasClass(row, "opened"))
        {
            var infoBodyRow = this.resultInfoTag.insertRows({entry: entry}, row)[0];
            this.initInfoBody(infoBodyRow);
        }
        else
        {
            var infoBodyRow = row.nextSibling;
            row.parentNode.removeChild(infoBodyRow);
        }
    },

    initInfoBody: function(infoBodyRow)
    {
        var entry = infoBodyRow.repObject;
        var TabView = Chromebug.XULAppModule.CacheEntryTabView;
        var tabViewNode = TabView.viewTag.replace({entry: entry}, infoBodyRow.firstChild, TabView);

        // Select default tab.
        TabView.selectTabByName(tabViewNode, "Data");
    },
});

// ************************************************************************************************

/**
 * This template represents an "info-body" for expanded window entries.
 */
Chromebug.XULAppModule.CacheEntryTabView = domplate(Firebug.Rep,
{
    listeners: [],

    viewTag:
        TABLE({"class": "tabView", cellpadding: 0, cellspacing: 0},
            TBODY(
                TR({"class": "tabViewRow"},
                    TD({"class": "tabViewCol", valign: "top"},
                        TAG("$tabList", {entry: "$entry"})
                    )
                )
            )
        ),

    tabList:
        DIV({"class": "tabViewBody"},
            TAG("$tabBar", {entry: "$entry"}),
            TAG("$tabBodies")
        ),

    // List of tabs
    tabBar:
        DIV({"class": "tabBar"},
            A({"class": "DataTab tab", onclick: "$onClickTab",
                view: "Data", $collapsed: "$entry|hideDataTab"},
                    $STR("Chromebug.XULAppModule.tab.Data")
            )
        ),

    // List of tab bodies
    tabBodies:
        DIV({"class": "tabBodies"},
            DIV({"class": "tabDataBody tabBody"})
        ),

    hideDataTab: function(result)
    {
        return false;
    },

    onClickTab: function(event)
    {
        this.selectTab(event.target);
    },

    selectTabByName: function(tabView, tabName)
    {
        var tab = getElementByClass(tabView, tabName + "Tab");
        if (tab)
            this.selectTab(tab);
    },

    selectTab: function(tab)
    {
        var view = tab.getAttribute("view");
        var viewBody = getAncestorByClass(tab, "tabViewBody");

        // Deactivate current tab.
        if (viewBody.selectedTab)
        {
            viewBody.selectedTab.removeAttribute("selected");
            viewBody.selectedBody.removeAttribute("selected");
        }

        // Store info about new active tab. Each tab has to have a body,
        // which is identified by class.
        var tabBody = getElementByClass(viewBody, "tab" + view + "Body");
        viewBody.selectedTab = tab;
        viewBody.selectedBody = tabBody;

        // Activate new tab.
        viewBody.selectedTab.setAttribute("selected", "true");
        viewBody.selectedBody.setAttribute("selected", "true");

        this.updateTabBody(viewBody, view);
    },

    updateTabBody: function(viewBody, tabName)
    {
        var tab = viewBody.selectedTab;
        var infoRow = getAncestorByClass(viewBody, "windowEntryInfoRow");
        var entry = infoRow.repObject;

        // Update Stack tab content
        var tabDataBody = getElementByClass(viewBody, "tabDataBody");
        if (tabName == "Data" && !tabDataBody.updated)
        {
            tabDataBody.updated = true;
            var text = entry.lines.join("");
            if (text && text.length > 10*1024)
                text = text.substr(0, 10*1024) + "...";
            tabDataBody.innerHTML = wrapText(text);
        }
    },
});

// ************************************************************************************************

/**
 * Helper object representing the Window hierarchy
 */
Chromebug.XULAppModule.DOMWindowRepNode = function(domWindow)
{
    this.name = name;
    this.sourceCache = sourceCache;
    this.size = 0; // Computed in getCacheSize
}

/**
 * Helper object for window entries.
 */
Chromebug.XULAppModule.CacheEntry = function(url, lines)
{
    this.url = url;
    this.lines = lines;
    this.size = (lines && lines.length) ? lines.join("").length : 0;
}


// ************************************************************************************************
// Registration

observerService.addObserver(Chromebug.XULAppModule.unwatchXULWindow, "xul-window-destroyed", false);
observerService.addObserver(Chromebug.XULAppModule.watchXULWindow, "xul-window-registered", false);

Firebug.registerModule(Chromebug.XULAppModule);
Firebug.registerPanel(Chromebug.XULAppPanel);

// ************************************************************************************************

}});
