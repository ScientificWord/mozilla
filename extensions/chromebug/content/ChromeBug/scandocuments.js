/* See license.txt for terms of usage */

FBL.ns(function() { with (FBL) {

Chromebug.DocumentScanner = extend(Firebug.Module,
{
    dispatchName: "documentScanner",
    scanningDocuments: false,

    initialize: function()
    {
        this.onScanningDocumentsMouseOver = bind(this.onScanningDocumentsMouseOver, this);
        this.onScanningDocumentsMouseDown = bind(this.onScanningDocumentsMouseDown, this);
        this.onScanningDocumentsClick = bind(this.onScanningDocumentsClick, this);
    },

   // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    onScanningDocumentsMouseOver: function(event)
    {
        if (FBTrace.DBG_INSPECT)
           FBTrace.sysout("onScanningDocuments mouse over", event);
        if (!this.scanDocuments(event.target))
        {
            cancelEvent(event);
            if (FBTrace.DBG_INSPECT)
                FBTrace.sysout("onScanningDocuments canceled mouseover Event", event);
        }
        // else let it go to inspect
    },

    onScanningDocumentsMouseDown: function(event)
    {
        if (FBTrace.DBG_INSPECT)
           FBTrace.sysout("onScanningDocuments mouse down", event);
        this.stopScanningDocuments(false, true);
        cancelEvent(event);  // inspector sees nothing or mousedown and stopInspecting but not inspecting
        Firebug.Inspector.stopInspecting(false, false);  // not canceled and don't wait, I'll wait for you.
    },

    onScanningDocumentsClick: function(event)
    {
        if (FBTrace.DBG_INSPECT)
           FBTrace.sysout("onScanningDocuments click", event);
        var win = event.currentTarget.defaultView;
        if (win)
        {
            win = getRootWindow(win);
            this.detachClickInspectListeners(win);
        }
        else
            FBTrace.sysout("onScanningDocuments no defaultView", event.currentTarget);
        cancelEvent(event);  // inspector sees nothing or if on same target, 'click' and redundant detach occurs
    },

    hasContentDocument: ["tabbrowser", "browser", "iframe"],

    scanDocuments: function(node)
    {
        if (node && node.nodeType != 1)
            node = node.parentNode;

        if (node && node.firebugIgnore)
            return;

        if (!node)
            return;

        var nodeAsScanned = node;
        var tagName = node.tagName;
        FBTrace.sysout("scanDocuments tag:"+tagName+" scanning "+(this.scanningDOMWindow?this.scanningDOMWindow.location:"none"));

        if (this.hasContentDocument.indexOf(tagName) != -1) // then the tag is a container of content
        {
            var doc = node.contentDocument;
            node = doc.documentElement;  // switch to the inside of the container
        }

        var domWindow = node.ownerDocument.defaultView;
        if (domWindow == this.scanningDOMWindow)  // we did not cross windows,
        {
            if (FBTrace.DBG_INSPECT)
                FBTrace.sysout("scanDocuments tag:"+tagName+" still scanning window "+(this.scanningDOMWindow?this.scanningDOMWindow.location:"none"));
            return true;                          // carry on with inspect
        }

        var context = Firebug.Chromebug.getContextByGlobal(domWindow);
        if (!context)
        {
             if (FBTrace.DBG_INSPECT)
                 FBTrace.sysout("scanDocuments tag:"+tagName+" no context! "+domWindow.location);

            //this.scanningContext.hoverNode = nodeAsScanned;
            return false;  // we crossed windows, don't inspect here
        }
        if (this.scanningContext != context)
            Firebug.Chromebug.selectContext(context);

        this.scanningContext = context;

        if (FBTrace.DBG_INSPECT)
            FBTrace.sysout("scanDocuments tag:"+tagName+" new context, scanning "+this.scanningContext.getName());

                 Firebug.Inspector.stopInspecting(false, false);  // on old window

        this.scanningDOMWindow = domWindow;
        context.hoverNode = node;  // tell inspect where to start

        Firebug.Inspector.startInspecting(context);  // on new window

        if (FBTrace.DBG_INSPECT)
                FBTrace.sysout("ScanDocuments startInspecting new context "+context.window.location);

        return true; // do inspect
    },

    //*****************************************************************************
    toggleScanningDocuments: function(context)
    {
        try
        {
            if (FBTrace.DBG_INSPECT)
                FBTrace.sysout("ChromeBug startScanning with "+context?context.getName():"NULL CONTEXT!"+" in "+window.location);
            if (this.scanningDocuments)
                this.stopScanningDocuments();
            else
                this.startScanningDocuments(context);
        }
        catch(exc)
        {
            FBTrace.sysout("toggleScanningDocuments fails "+exc, exc);
        }
    },

    startScanningDocuments: function(context)
    {
        this.scanningDocuments = true;
        this.scanningContext = context;

        if (FBTrace.DBG_INSPECT)
            FBTrace.sysout("ChromeBug startScanning with "+context?context.getName():"NULL CONTEXT!"+" in "+window.location);
        Firebug.chrome.setGlobalAttribute("cmd_toggleScanningDocuments", "checked", "true");
        Firebug.chrome.setGlobalAttribute("cmd_toggleInspecting", "checked", "true");

        this.attachScanListeners();

        // Remember the previous panel and bar state so we can revert if the user cancels
        this.previousContext = context;
        this.previousPanelName = context.panelName;
        this.previousSidePanelName = context.sidePanelName;
        this.previouslyCollapsed = $("fbContentBox").collapsed;
        this.previouslyFocused = context.detached && Firebug.chrome.isFocused();

        var htmlPanel = Firebug.chrome.selectPanel("html");
        this.previousObject = htmlPanel.selection;

        htmlPanel.panelNode.focus();
        htmlPanel.startInspecting();

        if (context.hoverNode)
            this.scanDocuments(context.hoverNode);
    },

    stopScanningDocuments: function(cancelled, waitForClick)
    {
        if (FBTrace.DBG_INSPECT)
            FBTrace.sysout("ChromeBug stopScanning  scanning:"+this.scanningDocuments+"\n");
        if (!this.scanningDocuments)
            return;

        var context = this.scanningContext;

        this.detachInspectListeners();
        if (!waitForClick)
            this.detachClickInspectListeners();

        Firebug.chrome.setGlobalAttribute("cmd_toggleScanningDocuments", "checked", "false");
        Firebug.chrome.setGlobalAttribute("cmd_toggleInspecting", "checked", "false");

        var htmlPanel = Firebug.chrome.unswitchToPanel(context, "html", cancelled);

        if (this.previouslyFocused)
            Firebug.chrome.focus();

        if (cancelled)
        {
            if (this.previouslyCollapsed)
                Firebug.showBar(false);

            if (this.previousPanelName == "html")
                Firebug.chrome.select(this.previousObject);
            else
                Firebug.chrome.selectPanel(this.previousPanelName, this.previousSidePanelName);
        }

        //htmlPanel.stopInspecting(htmlPanel.selection, cancelled);

        delete this.previousObject;
        delete this.previousPanelName;
        delete this.previousSidePanelName;
        delete this.scanningContext;
        this.scanningDocuments = false;
        delete this.scanningDOMWindow;
        delete context.hoverNode;

    },

    attachScanListeners: function()
    {
        this.keyListeners =
        [
            FirebugChrome.keyCodeListen("RETURN", null, bindFixed(this.stopScanningDocuments, this)),
            FirebugChrome.keyCodeListen("ESCAPE", null, bindFixed(this.stopScanningDocuments, this, true)),
            FirebugChrome.keyCodeListen("UP", isControl, bindFixed(this.inspectNodeBy, this, "up"), true),
            FirebugChrome.keyCodeListen("DOWN", isControl, bindFixed(this.inspectNodeBy, this, "down"), true),
        ];

        Chromebug.XULAppModule.iterateOuterDOMWindows( bind(function(subWin)
        {
            var context = Firebug.Chromebug.getContextByGlobal(subWin);
            if (!context)  // don't attach to windows we are not watching
                 return;
            // If you change the bubbling/capture option on these, do so on the removeEventListener as well.
            subWin.document.addEventListener("mouseover", this.onScanningDocumentsMouseOver, true); // trigger on capture
            subWin.document.addEventListener("mousedown", this.onScanningDocumentsMouseDown, true);
            subWin.document.addEventListener("click", this.onScanningDocumentsClick, true);
            if (FBTrace.DBG_INSPECT)
                FBTrace.sysout("scanDocuments attachListeners to "+subWin.location+"\n");
        }, this));
    },

    detachInspectListeners: function()
    {
        if (this.keyListeners)  // XXXjjb for some reason this is null some times...
        {
            for (var i = 0; i < this.keyListeners.length; ++i)
                FirebugChrome.keyIgnore(this.keyListeners[i]);
            delete this.keyListeners;
        }

        Chromebug.XULAppModule.iterateOuterDOMWindows( bind(function(subWin)
        {
            try
            {
                subWin.document.removeEventListener("mouseover", this.onScanningDocumentsMouseOver, true);
                subWin.document.removeEventListener("mousedown", this.onScanningDocumentsMouseDown, true);
            }
            catch (exc)
            {
                if (FBTrace.DBG_ERRORS)
                    FBTrace.sysout("scandocuments detachInspectListeners fails for subWin.document "+subWin.document.location+" because: "+exc, exc);
            }
        }, this));
    },

    detachClickInspectListeners: function()
    {
        // We have to remove the click listener in a second phase because if we remove it
        // after the mousedown, we won't be able to cancel clicked links
        Chromebug.XULAppModule.iterateOuterDOMWindows( bind(function(subWin)
        {
            subWin.document.removeEventListener("click", this.onScanningDocumentsClick, true);
        }, this));
    },
});

Firebug.registerModule(Chromebug.DocumentScanner);

// ************************************************************************************************

}});
