/* See license.txt for terms of usage */

FBL.ns(function() { with (FBL) {

// ***********************************************************************************
// Shorcuts and Services

const Cc = Components.classes;
const Ci = Components.interfaces;

var panelName = "firebugCache";

// ***********************************************************************************
// Firebug Cache Module

/**
 * Implementation of a Module & Panel for Fierebug cache viewer.
 */
Firebug.Chromebug.FBCacheModule = extend(Firebug.Module,
{
    dispatchName: panelName,

    initialize: function(prefDomain, prefNames)
    {
        if (FBTrace.DBG_FBCACHE)
            FBTrace.sysout("fbcache.initialize" + prefDomain);
    },

    shutdown: function()
    {
        if (FBTrace.DBG_FBCACHE)
            FBTrace.sysout("fbcache.shutdown, prefDomain: " + prefDomain);
    },

    initContext: function(context)
    {
        if (FBTrace.DBG_FBCACHE)
            FBTrace.sysout("fbcache.initContext; " + context.name);
    },

    // Helpers
    addStyleSheet: function()
    {
        var panelType = Firebug.getPanelType(panelName);
        var doc = FirebugChrome.getPanelDocument(panelType);

        // Make sure the stylesheet isn't appended twice.
        if ($("cbFBCache", doc))
            return;

        var styleSheet = createStyleSheet(doc, "chrome://chromebug/skin/firebugCache.css");
        styleSheet.setAttribute("id", "cbFBCache");
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
var Module = Firebug.Chromebug.FBCacheModule;

// ************************************************************************************************
// Firebug Cache Panel

Firebug.Chromebug.FBCachePanel = function() {}

Firebug.Chromebug.FBCachePanel.prototype = extend(Firebug.Panel,
{
    name: panelName,
    title: "Firebug Cache",
    searchable: false,
    editable: false,

    initializeNode: function(panelNode)
    {
        if (FBTrace.DBG_FBCACHE)
            FBTrace.sysout("fbcache.FBCachePanel.initializeNode; " + this.context.name);

        // Refresh panel content.
        this.refresh();
    },

    refresh: function()
    {
        if (FBTrace.DBG_FBCACHE)
            FBTrace.sysout("fbcache.FBCachePanel.refresh; " + this.context.name, this.context);

        clearNode(this.panelNode);

        // Append custom stylesheet for this panel.
        Module.addStyleSheet();

        // If the current browser window (associated with the context) has TabWatcher
        // display its cache entries. Otherwise bail out.
        var tabWatcher = this.context.browser.contentWindow.TabWatcher;
        if (!tabWatcher)
        {
            if (FBTrace.DBG_FBCACHE)
                FBTrace.sysout("fbcache.FBCachePanel.refresh; NO TAB-WATCHER " +
                    this.context.name, this.context);

            // Display info about no Firebug in the current window instance.
            Module.Message.render(this.panelNode, "Firebug is not embedded in: " +
                this.context.name + " - " + this.context.getTitle()); //xxxHonza: localization
            return;
        }

        // Get all caches from the browser window (there is one cache per tab).
        var cacheList = [];
        tabWatcher.iterateContexts(function(context)
        {
            if (FBTrace.DBG_FBCACHE)
                FBTrace.sysout("fbcache.refresh; context found: " + context.name, context);

            cacheList.push(new Module.TabCache(context.name, context.sourceCache));
        });

        // The cache is empty.
        if (!cacheList.length)
        {
            if (FBTrace.DBG_FBCACHE)
                FBTrace.sysout("fbcache.refresh; empty cache: " + this.context.name, this.context);

            // Display info about empty cache in the current window instance.
            Module.Message.render(this.panelNode, "Firebug cache is empty for: " +
                this.context.name + " - " + this.context.getTitle()); //xxxHonza: localization
            return;
        }

        // Insert all existing caches into the UI.
        Module.CacheList.tableTag.replace({cacheList: cacheList}, this.panelNode);

        // Append also a summary info.
        var table = this.panelNode.firstChild;
        var tbody = table.firstChild;
        Module.CacheList.summaryTag.insertRows({cacheList: cacheList}, tbody.lastChild);
    },

    destroyNode: function()
    {
        if (FBTrace.DBG_FBCACHE)
            FBTrace.sysout("fbcache.FBCachePanel.destroyNode;");
    },

    show: function(state)
    {
        if (FBTrace.DBG_FBCACHE)
            FBTrace.sysout("fbcache.FBCachePanel.show;", state);

        this.showToolbarButtons("cbFBCacheButtons", true);
    },

    hide: function()
    {
        if (FBTrace.DBG_FBCACHE)
            FBTrace.sysout("fbcache.FBCachePanel.hide;");

        this.showToolbarButtons("cbFBCacheButtons", false);
    },

    getOptionsMenuItems: function()
    {
         if (this.controller)
            return this.controller.getOptionsMenuItems();

         return [];
    },

    onPrefChange: function(optionName, optionValue)
    {
        if (FBTrace.DBG_FBCACHE)
            FBTrace.sysout("fbcache.FBCachePanel.onPrefChange; " + optionName + "=" + optionValue);
    },
});

// ************************************************************************************************
// Domplate Messages

/**
 * This template is used for messages displayed within the panel
 */
Firebug.Chromebug.FBCacheModule.Message = domplate(Firebug.Rep,
{
    tag:
        TABLE({"class": "fbCacheMessageTable", cellpadding: 0, cellspacing: 0},
            TBODY(
                TR({"class": "fbCacheMessageRow"},
                    TD({"class": "fbCacheMessageCol"},
                        DIV("$message"),
                        BUTTON({"class": "fbCacheRefreshBtn", onclick: "$onRefresh"},
                            "Refresh" //xxxHonza localization
                        )
                    )
                )
            )
        ),

    onRefresh: function()
    {
        Module.refresh(FirebugContext);
    },

    render: function(parentNode, message)
    {
        this.tag.replace({message: message}, parentNode);

        if (FBTrace.DBG_FBCACHE)
            FBTrace.sysout("fbcache.Message.render; " + message);
    }
});

// ************************************************************************************************

/**
 * This template represents list of caches in associated browser window.
 * Each tab in that window maintains its own cache.
 */
Firebug.Chromebug.FBCacheModule.CacheList = domplate(Firebug.Rep,
{
    tableTag:
        TABLE({"class": "cacheListTable", cellpadding: 0, cellspacing: 0, onclick: "$onClick"},
            TBODY(
                FOR("cache", "$cacheList",
                    TR({"class": "cacheListRow", _repObject: "$cache"},
                        TD({"class": "cacheNameCol cacheListCol"},
                            SPAN({"class": "cacheName"},
                                "$cache|getCacheName"
                            ),
                            SPAN({"class": "cacheSize"},
                                "$cache|getCacheSize"
                            )
                        )
                    )
                )
            )
        ),

    cacheBodyTag:
        TR({"class": "cacheBodyRow", _repObject: "$cache"},
            TD({"class": "cacheBodyCol", colspan: 1})
        ),

    summaryTag:
        TR({"class": "summaryRow"},
            TD({"class": "totalSize summaryCol"},
                SPAN("Total Size: $cacheList|getTotalSize")
            )
        ),

    getCacheName: function(cache)
    {
        return cache.name;
    },

    getCacheSize: function(cache) 
    {
        if (!cache.entries)
        {
            cache.size = 0;
            cache.entries = [];
            for (var url in cache.sourceCache.cache)
            {
                var entry = new Module.CacheEntry(url, cache.sourceCache.cache[url]);
                cache.size += entry.size;
                cache.entries.push(entry);
            }
        }

        return formatSize(cache.size);
    },

    getTotalSize: function(cacheList)
    {
        var totalSize = 0;
        for (var i=0; i<cacheList.length; i++)
            totalSize += cacheList[i].size;
        return formatSize(totalSize);
    },

    onClick: function(event)
    {
        if (isLeftClick(event)) 
        {
            var row = getAncestorByClass(event.target, "cacheListRow");
            if (row) 
            {
                this.toggleRow(row);
                cancelEvent(event);
            }
        }
    },

    expandGroup: function(row)
    {
        if (hasClass(row, "cacheListRow"))
            this.toggleRow(row, true);
    },

    collapseGroup: function(row)
    {
        if (hasClass(row, "cacheListRow") && hasClass(row,"opened"))
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
            var infoBodyRow = this.cacheBodyTag.insertRows({cache: cache}, row)[0];
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
            entries.push(new Module.CacheEntry(url, tabCache.sourceCache.cache[url]));

        if (!entries.length)
        {
            infoBodyRow.firstChild.innerHTML = "There are no cache entries";
            return;
        }

        // Create basic structure for list of cache entries
        // and insert all cache entries into it.
        var table = Module.CacheTable.tableTag.replace({}, infoBodyRow.firstChild);
        Module.CacheTable.resultTag.insertRows({entries: entries}, table.firstChild);
    }
});

// ************************************************************************************************

/**
 * This template represents an URL (displayed as a table-row within the panel)
 * that is cached within the Firebug cache.
 */
Firebug.Chromebug.FBCacheModule.CacheTable = domplate(Firebug.Rep,
{
    tableTag:
        TABLE({"class": "fbCacheTable", cellpadding: 0, cellspacing: 0, onclick: "$onClick"},
            TBODY()
        ),

    resultTag:
        FOR("entry", "$entries",
            TR({"class": "cacheEntryRow", _repObject: "$entry"},
                TD({"class": "cacheEntryCol", width: "100%"},
                    SPAN({"class": "cacheEntryURL cacheEntryLabel"},
                        "$entry|getURL"
                    ),
                    SPAN({"class": "cacheEntrySize cacheEntryLabel"},
                        "$entry|getSize"
                    )
                )
            )
        ),

    resultInfoTag:
        TR({"class": "cacheEntryInfoRow", _repObject: "$entry"},
            TD({"class": "cacheEntryInfoCol", colspan: 2})
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
            if (FBTrace.DBG_FBCACHE)
                FBTrace.sysout("fbcache.CacheList.getSize; ", err);
        }
    },

    onClick: function(event)
    {
        if (isLeftClick(event))
        {
            var row = getAncestorByClass(event.target, "cacheEntryRow");
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
        var TabView = Firebug.Chromebug.FBCacheModule.CacheEntryTabView;
        var tabViewNode = TabView.viewTag.replace({entry: entry}, infoBodyRow.firstChild, TabView);

        // Select default tab.
        TabView.selectTabByName(tabViewNode, "Data");
    },
});

// ************************************************************************************************

/**
 * This template represents an "info-body" for expanded cache entries.
 */
Firebug.Chromebug.FBCacheModule.CacheEntryTabView = domplate(Firebug.Rep,
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
                    $STR("fbcache.tab.Data")
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
        var infoRow = getAncestorByClass(viewBody, "cacheEntryInfoRow");
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
 * Helper object for source cache (for one tab in Firefox).
 */
Firebug.Chromebug.FBCacheModule.TabCache = function(name, sourceCache)
{
    this.name = name;
    this.sourceCache = sourceCache;
    this.size = 0; // Computed in getCacheSize
}

/**
 * Helper object for cache entries.
 */
Firebug.Chromebug.FBCacheModule.CacheEntry = function(url, lines)
{
    this.url = url;
    this.lines = lines;
    this.size = (lines && lines.length) ? lines.join("").length : 0;
}

// ************************************************************************************************
// Registration

Firebug.registerModule(Firebug.Chromebug.FBCacheModule);
Firebug.registerPanel(Firebug.Chromebug.FBCachePanel);

// ************************************************************************************************

}});
