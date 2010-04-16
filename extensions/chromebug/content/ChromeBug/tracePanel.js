/* See license.txt for terms of usage */

/**
 * UI control of debug Logging for Firebug internals
 */
FBL.ns(function() { with (FBL) {

// ***********************************************************************************
// Shorcuts and Services

const Cc = Components.classes;
const Ci = Components.interfaces;

const traceService = Cc["@joehewitt.com/firebug-trace-service;1"].getService(Ci.nsIObserverService);

// ***********************************************************************************
// TraceConsole Module
var tracePanel = null;

Firebug.Chromebug.TraceConsoleModule = extend(Firebug.Module,
{
    dispatchName: "traceConsole",

    initialize: function(prefDomain, prefNames)  // the prefDomain is from the app, eg chromebug
    {
        if (FBTrace.DBG_CB_CONSOLE)
            FBTrace.sysout("cb.TraceConsoleModule.initialize, prefDomain (ignored): " + prefDomain);

        traceService.addObserver(this, "firebug-trace-on-message", false);
    },

    shutdown: function()
    {
        if (FBTrace.DBG_CB_CONSOLE)
            FBTrace.sysout("cb.TraceConsoleModule.shutdown, prefDomain: " + prefDomain);

        traceService.removeObserver(this, "firebug-trace-on-message");
    },

    initContext: function(context)
    {
        if (FBTrace.DBG_CB_CONSOLE)
            FBTrace.sysout("cb.TraceConsoleModule.initContext; " +
                context.getName() + " - " + context.getTitle());

    },

    destroyContext: function(context)
    {
        if (FBTrace.DBG_CB_CONSOLE)
            FBTrace.sysout("cb.TraceConsoleModule.destroyContext; " +
                context.getName() + " - " + context.getTitle());
        return true; // do not remove
    },

    // nsIObserver
    observe: function(subject, topic, data)
    {
        if (topic == "firebug-trace-on-message")
        {
            // Display messages only with "extensions.firebug" type.
            var messageInfo = subject.wrappedJSObject;

            // type is controlled by FBTrace.prefDomain in the XULWindow that sent the trace message
            if (messageInfo.type != "extensions.firebug")  // TODO selectable
                return;

            if (tracePanel)
            {
                tracePanel.dump(new Firebug.TraceModule.TraceMessage(
                        messageInfo.type, data, messageInfo.obj, messageInfo.scope));
                return false;
            }
            return false;
        }
    },

    clearPanel: function(context)
    {
        if (tracePanel)
            tracePanel.clear();
        else
            FBTrace("tracePanel.clearPanel no tracePanel");
    },
});


// ************************************************************************************************
// Trace Panel

Firebug.Chromebug.TraceConsolePanel = function() {}

Firebug.Chromebug.TraceConsolePanel.prototype = extend(Firebug.Panel,
{
    name: "trace",
    title: "Trace",
    searchable: true,
    editable: false,

    initializeNode: function(myPanelNode)
    {
        this.controller = new Firebug.TraceOptionsController("extensions.firebug", this.onPrefChange);
        this.logs = Firebug.TraceModule.MessageTemplate.createTable(myPanelNode);
        Firebug.TraceModule.onLoadConsole(window, myPanelNode);
        this.unwrapper = bind(this.unWrapMessage, this);

        if (tracePanel)  // share the panelNode across all contexts
        {
            this.panelNode.parentNode.removeChild(this.panelNode);
            this.panelNode = tracePanel.panelNode;
        }
        else
        {
            var doc = this.panelNode.ownerDocument;
            var stylesheet = createStyleSheet(doc, "chrome://firebug/skin/traceConsole.css");
            addStyleSheet(doc, stylesheet);
            tracePanel = this;
        }
        if (FBTrace.DBG_CB_CONSOLE)
            FBTrace.sysout("TraceConsolePanel initializeNode in panel "+this.name);
    },

    destroyNode: function()
    {
        if (FBTrace.DBG_CB_CONSOLE)
            FBTrace.sysout("TraceConsolePanel destroyNode");
    },

    destroy: function()
    {
        FBTrace.sysout("TraceConsolePanel.destory");
    },

    show: function(state)
    {
        if (FBTrace.DBG_CB_CONSOLE)
            FBTrace.sysout("cb.TraceConsolePanel.show ", state);

        this.showToolbarButtons("cbTraceButtons", true);
        var buttons = Firebug.chrome.$("cbTraceButtons");
        if (!buttons)
        {
            buttons = document.getElementById("cbTraceButtons");
            if (buttons)
                FBTrace.sysout("tracePanel fails with Firebug.chrome.$ but succeeds with document.getElementById");
            else
                FBTrace.sysout("tracePanel fails with document.getElementById "+window.location);
        }

        if (this.lastScrollTop && this.logs)
            this.logs.scrollTop = this.lastScrollTop;
    },

    hide: function()
    {
        this.showToolbarButtons("cbTraceButtons", false);

        if (FBTrace.DBG_CB_CONSOLE)
            FBTrace.sysout("TraceFirebug.panel hide", this);

        this.lastScrollTop = this.logs.scrollTop;
    },

    clear: function()
    {
        if (FBTrace.DBG_CB_CONSOLE)
            FBTrace.sysout("cb.TraceConsolePanel.clear");

        if (this.panelNode)
            clearNode(this.panelNode);

        this.logs = Firebug.TraceModule.MessageTemplate.createTable(this.panelNode);
    },

    getOptionsMenuItems: function()
    {
         var items = this.controller.getOptionsMenuItems();
         items.push(this.getAllOffOptionMenuItem());
         return items;
    },

    getAllOffOptionMenuItem: function()
    {
        var self = this;
        return {label: "AllOptionsOff",  nol10n: true, type: "checkbox", checked: false,
            command: function allOff()
        {
            if(FBTrace.DBG_OPTOINS)
                FBTrace.sysout("getAllOffOptionMenuItem ", self.controller);
            self.controller.clearOptions();
        }};
    },

    onPrefChange: function(optionName, optionValue)
    {
        FBTrace.sysout("tracePanel.onPrefChange: "+optionName+"="+optionValue);
    },

    search: function(text)
    {
        if (!text)
            return;

        // Make previously visible nodes invisible again
        if (this.matchSet)
        {
            for (var i in this.matchSet)
                removeClass(this.matchSet[i], "matched");
        }

        this.matchSet = [];

        var a_row = getElementByClass(this.logs, "messageRow");
        if (!a_row)
            return false; // too early

        var rows = a_row.parentNode;

        function findRow(node) {var mr = getAncestorByClass(node, "messageRow");
        FBTrace.sysout("findRow ",mr);return mr;}
        var search = new TextSearch(rows, findRow);
        FBTrace.sysout("tracePanel search #nodes"+rows.childNodes.length, search );
        var logRow = search.find(text);
        if (!logRow)
            return false;

        for (; logRow; logRow = search.findNext())
        {
            setClass(logRow, "matched");
            FBTrace.sysout("tracePanel search FOUND:\'"+text+"\'", logRow);
            FBTrace.sysout("tracePanel search #nodes"+rows.childNodes.length, search );
            this.matchSet.push(logRow);
        }

        return true;
    },
    // ********************************************************************************************
    // Message dump
    dump: function(message)
    {
        // Notify listeners
        Firebug.TraceModule.onDump(message);

        if (!this.logs)
            return;

        var scrolledToBottom = isScrolledToBottom(this.getScrollingNode());

        var index = message.text.indexOf("ERROR");
        if (index != -1)
            message.type = "DBG_ERRORS";

        index = message.text.indexOf("EXCEPTION");
        if (index != -1)
            message.type = "DBG_ERRORS";

        // The wrapper could a domplate but then the domplate expansion would be in the domplate and confuse me.
        var wrapper = this.logs.ownerDocument.createElement("tr");
        wrapper.className = "messageRow " + message.type;

        var wrapperNumberCell =  this.logs.ownerDocument.createElement("td");
        wrapperNumberCell.className = "messageNameCol messageCol";

        var wrapperNumber =  this.logs.ownerDocument.createElement("div");
        message.wrapperIndex = this.logs.firstChild.childNodes.length + 1;
        wrapperNumber.innerHTML = message.wrapperIndex +"";
        wrapperNumber.className = "messageNameLabel messageLabel";
        wrapperNumberCell.appendChild(wrapperNumber);

        var wrapperTime = this.logs.ownerDocument.createElement("td");
        wrapperTime.className = "messageTimeCol messageCol";
        var timeLabel =  this.logs.ownerDocument.createElement("div");
        timeLabel.className = "messageTimeLabel messageLabel";
        timeLabel.innerHTML = message.time;
        wrapperTime.appendChild(timeLabel);

        var wrapperData = this.logs.ownerDocument.createElement("td");
        wrapperData.innerHTML = message.getLabel(-1);
        wrapperData.className = "messageBodyCol messageCol messageLabel";

        wrapper.message = message;
        wrapper.addEventListener('click', this.unwrapper, true);

        wrapper.appendChild(wrapperNumberCell);
        wrapper.appendChild(wrapperTime);
        wrapper.appendChild(wrapperData);
        this.logs.firstChild.appendChild(wrapper);
        if (scrolledToBottom)
            scrollToBottom(this.getScrollingNode());
    },

    unWrapMessage: function(event)
    {
        var wrapper  = event.currentTarget;
        wrapper.removeEventListener('click', this.unwrapper, true);
        var message = wrapper.message;
        var index = message.wrapperIndex - 1; // somewhere in the domplate expansion a one is added.

        // expand the domplate into the end of parent
        var self = this;
        var outputNodes =
        {
            getScrollingNode: function()
            {
                window.dump("unWrapMessage getScrollingNode self.panelNode "+self.panelNode+"\n");
                return self.panelNode;
            },
            getTargetNode: function()
            {
                return wrapper.parentNode;
            },
        }


        var result = Firebug.TraceModule.MessageTemplate.dump(message, outputNodes, index);

        // move the new row over the wrapper

        try
        {
            var theUnwrapped = message.row;
            var old = wrapper.parentNode.replaceChild(theUnwrapped, wrapper);
        }
        catch (exc)
        {
            FBTrace.sysout("tracePanel.unWrapMessage replaceChild", exc);
        }
        Firebug.TraceModule.MessageTemplate.toggleRow(theUnwrapped);
    },


    // ********************************************************************************************
    // Interface to the output nodes, going by the name outputNodes

    getScrollingNode: function()
    {
        return this.panelNode;
    },

    getTargetNode: function()
    {
        return this.logs.firstChild;
    },
    // ********************************************************************************************

});

// ************************************************************************************************
// Registration

Firebug.registerModule(Firebug.Chromebug.TraceConsoleModule);
Firebug.registerPanel(Firebug.Chromebug.TraceConsolePanel);

// ************************************************************************************************

}});
