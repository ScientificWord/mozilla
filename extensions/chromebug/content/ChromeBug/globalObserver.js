/* See license.txt for terms of usage */

FBL.ns(function() { with (FBL) {

// ************************************************************************************************
// Shorcuts and Services

const Cc = Components.classes;
const Ci = Components.interfaces;

const nsIObserverService = Ci.nsIObserverService
const observerService = CCSV("@mozilla.org/observer-service;1", "nsIObserverService");

// ************************************************************************************************

Chromebug.globalObserver =
{
    observe: function(subject, topic, data)
    {
        var FirebugTrace = Cc["@joehewitt.com/firebug-trace-service;1"]
            .getService(Ci.nsISupports).wrappedJSObject.getTracer("extensions.firebug");

        // Log info into the Firebug tracing console.
        var shout = (Chromebug.globalObserver.shoutOptionValue?"GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG globalObserver.":"");
        FirebugTrace.sysout(shout+"observe: "+topic, {subject:subject, data: data});

        if (topic == 'domwindowopened')
        {
            try
            {
                if (subject instanceof Ci.nsIDOMWindow)
                {
                    if (FBTrace.DBG_CHROMEBUG || FBTrace.DBG_WINDOWS)
                        FBTrace.sysout("Chromebug.globalObserver found domwindowopened "+subject.location+"\n");
                }
            }
            catch(exc)
            {
                FBTrace.sysout("Chromebug.globalObserver notify console opener FAILED ", exc);
            }
        }
        else if (topic == 'domwindowclosed') // Apparently this event comes before the unload event on the DOMWindow
        {
            if (subject instanceof Ci.nsIDOMWindow)
            {
                if (FBTrace.DBG_WINDOWS)
                    FBTrace.sysout("Chromebug.globalObserver found domwindowclosed "+subject.location+getStackDump());
                if (subject.location.toString() == "chrome://chromebug/content/chromebug.xul")
                    throw new Error("Chromebug.globalObserver should not find chromebug.xul");
            }
        }
        else if (topic == 'dom-window-destroyed')  // subject appears to be the nsIDOMWindow with a location that is invalid and closed == true; data null
        {
            if (FBTrace.DBG_WINDOWS)
                FBTrace.sysout("Chromebug.globalObserver found dom-window-destroyed subject:", subject);
        }
    },

    shoutOptionName: "shoutAboutObserverEvents",
    shoutOptionLabel: "Shout About Observer Events",
    shoutOptionValue: true,
};

Chromebug.globalObserver.wrappedJSObject = Chromebug.globalObserver;  // and eye of newt

// ************************************************************************************************

Chromebug.ObserverServiceModule = extend(Firebug.Module,
{
    dispatchName: "observerService",

    initialize: function(prefDomain, prefNames)  // the prefDomain is from the app, eg chromebug
    {
        if (FBTrace.DBG_CB_CONSOLE)
            FBTrace.sysout("cb.ObserverServiceModule.initialize, prefDomain (ignored): " + prefDomain);

        //?observerService.addObserver(Chromebug.globalObserver, "domwindowopened", false);
        //observerService.addObserver(Chromebug.globalObserver, "domwindowclosed", false);
        //observerService.addObserver(Chromebug.globalObserver, "dom-window-destroyed", false);

        Firebug.TraceModule.addListener(this);
    },

    shutdown: function()
    {
        if (FBTrace.DBG_CB_CONSOLE)
            FBTrace.sysout("cb.ObserverServiceModule.shutdown, prefDomain: " + prefDomain);

        //observerService.removeObserver(Chromebug.globalObserver, "domwindowopened", false);
        //observerService.removeObserver(Chromebug.globalObserver, "domwindowclosed", false);
        //observerService.removeObserver(Chromebug.globalObserver, "dom-window-destroyed", false);

        Firebug.TraceModule.removeListener(this);
    },

    initContext: function(context)  // use these to check the tracking of windows to contexts
    {
    },

    destroyContext: function(context)
    {
    },

    clearPanel: function(context)
    {
        if (this.tracePanel)
            this.tracePanel.clear();
        else
            FBTrace("tracePanel.clearPanel no this.tracePanel");
    },

    // Firebug.TraceModule listener
    onDump: function(message)
    {
        var prefix = "globalObserver.";
        var index = message.text.indexOf(prefix);
        if (index == 0)
            message.type = "DBG_GLOBALOBSERVER";
    },

    addStyleSheet: function()
    {
        var panelType = Firebug.getPanelType("trace");
        var doc = FirebugChrome.getPanelDocument(panelType);

        // Make sure the stylesheet isn't appended twice.
        var styleId = "globalObserver";
        if ($(styleId, doc))
            return;

        var styleSheet = createStyleSheet(doc, "chrome://chromebug/skin/tracePanel.css");
        styleSheet.setAttribute("id", styleId);
        addStyleSheet(doc, styleSheet);
    },
});

//************************************************************************************************

Chromebug.ObserverServicePanel = function() {}

Chromebug.ObserverServicePanel.prototype = extend(Firebug.Panel,
{
    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
    // extends Panel

    name: "observer",
    title: "Observer",
    parentPanel: "trace",
    order: 1,

    initialize: function(context, doc)
    {
         Firebug.Panel.initialize.apply(this, arguments);
    },

    destroy: function(state)
    {
        Firebug.Panel.destroy.apply(this, arguments);
    },

    initializeNode : function(oldPanelNode)
    {
        dispatch([Firebug.A11yModel], 'onInitializeNode', [this, this.name]);
    },

    destroyNode : function()
    {
        dispatch([Firebug.A11yModel], 'onDestroyNode', [this, this.name]);
    },

    show: function(state)
    {
          this.refresh();
    },

    supportsObject: function(object)
    {
        return 0;
    },

    tag:
        DIV({onclick: "$onClick", role : 'list'},
            FOR("topic", "$topics",
                    DIV({class: "traceOption"}, "$topic")
            )
        ),

    refresh: function()
    {
        try
        {
            // Make sure appropriate stylesheet for Global Observer logs is included.
            Chromebug.ObserverServiceModule.addStyleSheet();

            if (FBTrace.DBG_CHROMEBUG)
                FBTrace.sysout("globalObserver refresh topics "+this.topics.length, this.topics);
            clearNode(this.panelNode);
            this.tag.replace({topics: this.topics, onClick: bind(this.toggleTopic, this)}, this.panelNode);
            this.updateTopics();
            if (FBTrace.DBG_CHROMEBUG)
                FBTrace.sysout("globalObserver refresh panelNode "+this.panelNode.childNodes.length, this.panelNode);
        }
        catch(exc)
        {
            if (FBTrace.DBG_ERRORS) FBTrace.sysout("globalObserver refresh FAILS "+exc, exc);
        }
        try
        {
/*
            //var observerService = Components.classes["@mozilla.org/observer-service;1"].getService(Components.interfaces.nsIObserverService);
            //var obs = {observe: function() {}};
            //obs.wrappedJSObject = obs;
            observerService.addObserver(Chromebug.globalObserver, "domwindowclosed", false);
            var enu = observerService.enumerateObservers("domwindowclosed");
            enu.hasMoreElements();
            var x1 = enu.getNext()
            if (x1.wrappedJSObject == Chromebug.globalObserver)
                FBTrace.sysout('MATCH');
            else
                FBTrace.sysout("FAILS");
                */
        }
        catch(exc)
        {
            FBTrace.sysout("EXCEPTION "+exc);
        }
    },

    forEachTopicElement: function(fn)
    {
        var topicDivs = getElementsByClass(this.panelNode, "traceOption")
        for (var i = 0; i < topicDivs.length; i++)
        {
            var topicElt = topicDivs[i];
            fn(topicElt);
        }
    },

    updateTopics: function()
    {
        return;
        var self = this;
        this.forEachTopicElement(function setChecked(topicElt)
        {
            var topic = topicElt.innerHTML;
            if (self.isObserved(topicElt))
                topicElt.setAttribute("checked", "true");
            else
                topicElt.removeAttribute("checked");
        });
    },

    allTopics: function(add)
    {
        var self = this;
        this.forEachTopicElement(function addIfTrue(topicElt)
        {
            try
            {
                if (add)
                    self.addTopic(topicElt);
                else
                    self.removeTopic(topicElt);

                if (FBTrace.DBG_CHROMEBUG)
                    FBTrace.sysout("AllTopics add: "+add+" isObserved "+self.isObserved(topicElt)+" "+topicElt.innerHTML);
            }
            catch(exc)
            {
                FBTrace.sysout("globalObserver allTopics fails for "+topicElt.innerHTML+" "+exc, exc);
            }
        });
    },

    isObserved: function(topicElt)
    {
        var topic = topicElt.innerHTML;
        var observers = observerService.enumerateObservers(topic);
        if (!observers)
        {
            if (FBTrace.DBG_CHROMEBUG)
                FBTrace.sysout("isObserved no observers of topic "+topic);

            return false;
        }
        while (observers.hasMoreElements())
        {
            var x = observers.getNext();
            if (x.wrappedJSObject == Chromebug.globalObserver)
            {
                if (FBTrace.DBG_CHROMEBUG)
                    FBTrace.sysout("isObserved found Chromebug.globaleObserver of topic "+topic, x);
                return true;
            }
        }
        if (FBTrace.DBG_CHROMEBUG)
            FBTrace.sysout("isObserved no observers of topic "+topic+" match Chromebug.globalObserver");
        return false
    },

    toggleTopic: function(event)
    {
        var topicElt = event.target;
        if (FBTrace.DBG_CHROMEBUG)
                FBTrace.sysout("globalObserver panel onclick "+topicElt.innerHTML);
        if (!this.isObserved(topicElt))
            this.addTopic(topicElt)
        else
            this.removeTopic(topicElt);
        if (FBTrace.DBG_CHROMEBUG)
            FBTrace.sysout("globalObserver panel onclick isObserved:"+this.isObserved(topicElt));
    },

    addTopic: function(node)
    {
        var topic = node.innerHTML;
        observerService.addObserver(Chromebug.globalObserver, topic, false);
        node.setAttribute("checked", "true");
        if (FBTrace.DBG_CHROMEBUG)
            FBTrace.sysout("addObserver "+topic);
    },

    removeTopic: function(node)
    {
        var topic = node.innerHTML;
        observerService.removeObserver(Chromebug.globalObserver, topic);
        node.removeAttribute("checked");
        if (FBTrace.DBG_CHROMEBUG)
            FBTrace.sysout("removeObserver "+topic);
    },

    getOptionsMenuItems: function()
    {
        var items = [
            {label: "All On", command: bindFixed(this.allTopics, this, true) },
            {label: "All Off", command: bindFixed(this.allTopics, this, false) },
            optionMenu(Chromebug.globalObserver.shoutOptionLabel, Chromebug.globalObserver.shoutOptionName),
            ];
        return items;
    },

    updateOption: function(name, value)
    {
        if (name == Chromebug.globalObserver.shoutOptionName)
            Chromebug.globalObserver.shoutOptionValue = (value?true:false); // force to boolean
    },

    topics: [
             "Migration:Ended",
             "Migration:ItemAfterMigrate",
             "Migration:ItemBeforeMigrate",
             "Migration:Started",
             "a11y-init-or-shutdown",
             "accessible-event",
             "addons-message-notification",
             "agent-sheet-added",
             "agent-sheet-removed",
             "app-handler-pane-loaded",
             "browser-search-engine-modified",
             "browser-ui-startup-complete",
             "browser:purge-session-history",
             "cacheservice:empty-cache",
             "chrome-flush-caches",
             "chrome-flush-skin-caches",
             "cookie-changed",
             "cookie-rejected",
             "cycle-collector-begin",
             "dl-cancel",
             "dl-start",
             "dom-storage-changed",
             "dom-storage-warn-quota-exceeded",
             "domwindowclosed",
             "domwindowopened",
             "dom-window-destroyed",
             "download-manager-remove-download",
             "dummy-observer-created",
             "dummy-observer-item-added",
             "dummy-observer-visited",
             "earlyformsubmit",
             "em-action-requested",
             "final-ui-startup",
             "formhistory-expire-now",
             "http-on-examine-cached-response",
             "http-on-examine-merged-response",
             "http-on-examine-response",
             "http-on-modify-request",
             "idle-daily",
             "memory-pressure",
             "net:clear-active-logins",
             "network:offline-status-changed",
             "offline-app-removed",
             "offline-cache-update-added",
             "offline-cache-update-completed",
             "offline-requested",
             "page-info-dialog-loaded",
             "passwordmgr-found-form",
             "passwordmgr-found-logins",
             "passwordmgr-storage-changed",
             "perm-changed",
             "places-database-locked",
             "places-init-complete",
             "plugins-list-updated",
             "prefservice:after-app-defaults",
             "private-browsing",
             "profile-after-change",
             "profile-approve-change",
             "profile-before-change",
             "profile-change-net-restore",
             "profile-change-net-teardown",
             "profile-change-teardown",
             "profile-do-change",
             "quit-application",
             "quit-application-forced",
             "quit-application-granted",
             "quit-application-requested",
             "session-save",
             "sessionstore-state-write",
             "sessionstore-windows-restored",
             "shell:desktop-background-changed",
             "shutdown-cleanse",
             "signonChanged",
             "signonSelectUser",
             "sleep_notification",
             "softkb-change",
             "system-display-dimmed-or-off",
             "system-display-on",
             "user-interaction-active",
             "user-interaction-inactive",
             "user-sheet-added",
             "user-sheet-removed",
             "wake_notification",
             "xmlparser",
             "xpcom-category-entry-added",
             "xpcom-shutdown",
             "xpcom-shutdown-loaders",
             "xpcom-shutdown-threads",
             "xpinstall-download-started",
             "xpinstall-install-blocked",
             "xul-window-destroyed",
             "xul-window-registered"
             ],

});

//************************************************************************************************

Firebug.registerModule(Chromebug.ObserverServiceModule);
Firebug.registerPanel(Chromebug.ObserverServicePanel);

}});