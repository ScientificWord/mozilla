/* See license.txt for terms of usage */
FBL.ns(function xpcomExplorer() { with (FBL) {

const Cc = Components.classes;
const Ci = Components.interfaces;

const directoryService = CCSV("@mozilla.org/file/directory_service;1", "nsIProperties");


top.XPCOMExplorer = {};


XPCOMExplorer.categoryListLocator = function(xul_element)
{
    if (!XPCOMExplorer.categoryList)
    {
        XPCOMExplorer.categoryList = {
            elementBoundTo: xul_element,

            getLocationList: function()
            {
                if (!this.catman)
                    this.catman = Components.classes["@mozilla.org/categorymanager;1"].getService(Components.interfaces.nsICategoryManager);

                var list = [];
                var categories = this.catman.enumerateCategories();
                while( categories.hasMoreElements() )
                {
                    var category =  categories.getNext().QueryInterface(nsISupportsCString);
                    var entries = this.catman.enumerateCategory(category);
                    while( entries.hasMoreElements() )
                    {
                        var entry = entries.getNext().QueryInterface(nsISupportsCString);
                        list.push(category + "/" + entry);
                    }
                }
                return list;
            },

            getDefaultLocation: function()
            {
                var locations = this.getLocationList();
                if (locations && locations.length > 0) return locations[0];
            },

            getObjectLocation: function(categorySlashEntry)
            {
                return categorySlashEntry;
            },

            getObjectDescription: function(categorySlashEntry)
            {
                var c = categorySlashEntry.indexOf('/');
                if (c != -1)
                    return { path: categorySlashEntry.substr(0, c), name: categorySlashEntry.substr(c+1)};
                else
                    return { path: "error", name: categorySlashEntry};
            },

            onSelectLocation: function(event)
            {
                var categorySlashEntry = event.currentTarget.repObject;
                if (categorySlashEntry)
                {
                    var d = XPCOMExplorer.categoryList.getObjectDescription(categorySlashEntry);
                    Firebug.Console.log(d);
                    var value = XPCOMExplorer.categoryList.catman.getCategoryEntry(d.path, d.name);
                    Firebug.Console.log(categorySlashEntry+": "+value); // category/entry
                }
                else
                    FBTrace.sysout("onSelectLocation FAILED, no repObject in currentTarget", event.currentTarget);
            }
        }
        xul_element.addEventListener("selectObject", XPCOMExplorer.categoryList.onSelectLocation, false);
    }
    return XPCOMExplorer.categoryList;
}

XPCOMExplorer.overlayListLocator = function(xul_element)
{
    if (!XPCOMExplorer.overlayList)
    {
        XPCOMExplorer.overlayList = {
            elementBoundTo: xul_element,

            getOverlayList: function(href)
            {

                var uri = makeURI(href);

                var prov = Components.classes["@mozilla.org/chrome/chrome-registry;1"].
                    getService(Components.interfaces.nsIXULOverlayProvider);

                var overlays =[];
                if (!uri)
                    return;

                var overlaysEnum = prov.getXULOverlays(uri)
                while(overlaysEnum.hasMoreElements()) {
                    var url = overlaysEnum.getNext().QueryInterface(Components.interfaces.nsIURI).spec;
                        overlays.push( { href: url, overlayType: "XUL"} );
                }

                var overlaysEnum = prov.getStyleOverlays(uri)
                while(overlaysEnum.hasMoreElements()) {
                    var url = overlaysEnum.getNext().QueryInterface(Components.interfaces.nsIURI).spec;
                    overlays.push( { href: url, overlayType: "Style"} );
                }
                if (FBTrace.DBG_CHROMEBUG)
                    FBTrace.sysout("OverlayListLocator "+overlays.length+" for "+href, overlays);

                return overlays;
            },

            getLocationList: function()
            {
                if (FirebugContext)
                {
                    // TODO use URI of current context
                    var win = FirebugContext.window;
                    var overlays = this.getOverlayList(win.location.toString());
                    return overlays;
                }
            },

            getDefaultLocation: function()
            {
                var locations = this.getLocationList();
                if (locations && locations.length > 0) return locations[0];
            },

            getObjectLocation: function(overlay)
            {
                return overlay.href;
            },

            getObjectDescription: function(overlay)
            {
                return Chromebug.parseURI(overlay.href);
            },

            getBrowserForOverlay: function(url)
            {
                var overlayViewer = $('cbOverlayViewer');
                var browsers = overlayViewer.getElementsByTagName("browser");
                for (var i = 0; i < browsers.length; i++)
                {
                    if (browsers[i].src == url)
                        return browsers[i];
                }
                // no find.
                return createBrowserForOverlay(overlayViewer, url);
            },

            createBrowserForOverlay: function(overlayViewer, url)
            {
                var browser = document.createElement('browser');
                browser.setAttribute("type", "content");
                browser.setAttribute("src", url)
                return browser;
            },

            onSelectLocation: function(event)
            {
                var object = event.currentTarget.repObject;

                if (!object.context)
                {
                    var description = Chromebug.parseURI(object.href);
                    if (description)
                    {
                        var pkg = Firebug.Chromebug.contextList.getPackageByName(description.path);
                        var browser = XPCOMExplorer.overlayList.getBrowserForOverlay(object.href);
                        var context = pkg.createContextInPackage(win, browser);
                        object.context = context;
                    }
                }
                FirebugChrome.showContext(object.context);
                FirebugChrome.selectPanel("HTML");

                FBTrace.sysout("XPCOMExplorer.overlayList onSelectLocation object", object);
            }
        }
        xul_element.addEventListener("selectObject", XPCOMExplorer.overlayList.onSelectLocation, false);
    }
    return XPCOMExplorer.overlayList;
}


XPCOMExplorer.componentList = extend(new Chromebug.SourceFileListBase(),
{
    kind: "component",

    onSelectLocation: function(event)
    {
        var object = event.currentTarget.repObject;
        if (object)
            FirebugChrome.select(object, "script", null, true);  // SourceFile
        else
            FBTrace.sysout("onSelectLocation FAILED, no repObject in currentTarget", event.currentTarget);
    }
});

XPCOMExplorer.componentListLocator = function(xul_element)
{
    return Chromebug.connectedList(xul_element, XPCOMExplorer.componentList);
}

XPCOMExplorer.moduleList = extend( new Chromebug.SourceFileListBase(),
{
      kind: "module",
});

XPCOMExplorer.moduleListLocator = function(xul_element)
{
    return Chromebug.connectedList(xul_element, XPCOMExplorer.moduleList);
}

// A list of the jsContexts built by enumerating them dynamically.
XPCOMExplorer.jsContextList = {

    getLocationList: function()
    {
        this.list = fbs.eachJSContext();
        return this.list;
    },

    getDefaultLocation: function()
    {
        var locations = this.getLocationList();
        if (locations && locations.length > 0) return locations[0];
    },

    getObjectLocation: function(jscontext)
    {
        if (!jscontext)
            FBTrace.sysout("getObjectLocation for nothing ");
        var global = jscontext.globalObject ? new XPCNativeWrapper(jscontext.globalObject.getWrappedValue()) : null;
        if (global)
        {
            var document = global.document;
            if (document)
            {
                var location = document.location.toString();
                var rootWindow = getRootWindow(global);
                if (rootWindow && rootWindow.location)
                    return location +" < "+rootWindow.location.toString();
                else
                    return location;
            }
            else
                return "noDocument://"+jscontext.tag;
            }
        else
            return "noGlobal://"+jscontext.tag;
    },

    getObjectDescription: function(jscontext)
    {
        var URI = this.getObjectLocation(jscontext);
        if (!URI)
            return {path: "no URI", name: "no URI"};
        if (Firebug.Chromebug.isChromebugURL(URI))
            var d = {path:"avoided chromebug", name: URI};
        if (!d)
            var d = Chromebug.parseURI( URI );
        if (!d)
            d = {path:"unparsable", name: this.getObjectLocation(jscontext)};
        d.name = d.name +" ("+jscontext.tag+")";
        return d;
    },

    getContextByLocation: function(location)  // TODO MOVE to context list
    {
        for (var i = 0; i < Chromebug.XULAppModule.contexts.length; ++i)
        {
            var context = Chromebug.XULAppModule.contexts[i];
            try
            {
                if (context.window.location == location)
                    return context;
            }
            catch(e)
            {
                //? Exception... "Unexpected error arg 0 [nsIDOMWindowInternal.location]"
                FBTrace.sysout("ChromeBugPanel.getContextByLocation: ignoring ", e);
            }
        }
    },

    onSelectLocation: function(event)
    {
        var object = event.currentTarget.repObject;
        if (object)
        {
            var jscontext = object;
            var global = new XPCNativeWrapper(jscontext.globalObject.getWrappedValue());
            if (global)
            {
                var context = Firebug.Chromebug.getContextByGlobal(global)
                if (context)
                {
                    Firebug.Chromebug.selectContext(context);
                }
                else
                {
                    FBTrace.sysout("onSelectLocation no context, showing global");
                    FirebugChrome.select(global, "DOM", null, true);
                }
            }
            else
            {
                FBTrace.sysout("XPCOMExplorer.jsContextList onSelectLocation: FAILED to no globalObject in jscontext\n");
                FirebugChrome.select(object, "script", null, true);  // jscontext
            }
        }
        else
        {
            FBTrace.sysout("onSelectLocation FAILED, no repObject in currentTarget", event.currentTarget);
        }
    }
}
XPCOMExplorer.jsContextListLocator = function(xul_element)
{
     return Chromebug.connectedList(xul_element, XPCOMExplorer.jsContextList);
}

XPCOMExplorer.pathListLocator = function(xul_element)
{
    if (!XPCOMExplorer.pathList)
    {
        XPCOMExplorer.pathList = {
            elementBoundTo: xul_element,
            directoryService: directoryService,
            strings: ['ProfD', 'DefProfRt', 'UChrm', 'DefRt', 'PrfDef', 'APlugns', 'AChrom','ComsD', 'Home', 'TmpD', 'ProfLD', 'resource:app', 'Desk', 'Progs', 'BMarks', 'DLoads', 'UStor'],

            getLocationList: function()
            {
                if (FBTrace.DBG_CHROMEBUG)
                    FBTrace.sysout("PathListLocator getLocationLst FirebugContext",FirebugContext.getName());

                var list = [];
                for (var i = 0; i < this.strings.length; i++)
                {
                    try
                    {
                        var file = this.directoryService.get(this.strings[i], Components.interfaces.nsIFile);
                        list.push({key: this.strings[i], nsIFile: file});
                    }
                    catch(exc)
                    {
                        list.push("FAILED: "+exc);
                    }
                }
                return list;
            },

            getDefaultLocation: function()
            {
                var locations = this.getLocationList();
                if (locations && locations.length > 0) return locations[0];
            },

            getObjectLocation: function(path)
            {
                return path;
            },

            getObjectDescription: function(obj)
            {
                if (obj)
                    return {path: obj.key, name: obj.nsIFile.path};
                else
                    return {path: "", name: "(null object)"};
            },

            onSelectLocation: function(event)
            {
                var object = event.currentTarget.repObject;
                if (object)
                {
                    var URL = iosvc.newFileURI(object.nsIFile);
                    FBL.openWindow(null, URL.spec);
                }
                else
                    FBTrace.sysout("onSelectLocation FAILED, no repObject in currentTarget", event.currentTarget);
            }
        }
        xul_element.addEventListener("selectObject", XPCOMExplorer.pathList.onSelectLocation, false);
    }
    return XPCOMExplorer.List;
}


XPCOMExplorer.extensionList = extend( new Chromebug.SourceFileListBase(),
{
    kind: "extension",
});

XPCOMExplorer.extensionListLocator = function(xul_element)
{
    return Chromebug.connectedList(xul_element, XPCOMExplorer.extensionList);
}

XPCOMExplorer.interfaceList = {

        getLocationList: function()
        {
            var ifaces = [];
            for(iface in Components.interfaces)
            {
                ifaces.push(iface);
            }
            return ifaces;
        },

        getDefaultLocation: function()
        {
            var locations = this.getLocationList();
            if (locations && locations.length > 0) return locations[0];
        },

        reLowerCase: /([a-z]*)(.*)/,

        getObjectLocation: function(iface)
        {
            var dot = iface.lastIndexOf('.');
            if (dot > 0)
                return iface.substr(dot+1);
            else
                return iface;
        },

        getObjectDescription: function(iface)
        {
            var ifaceName = this.getObjectLocation(iface);
            var prefix = "";
            var prefixFinder = this.reLowerCase.exec(ifaceName);
            if (prefixFinder)
            {
                return {path: "Components.interfaces."+prefixFinder[1], name: prefixFinder[2]};
            }
            else
                return iface;
        },

        onSelectLocation: function(event)
        {
            var ifaceName = event.currentTarget.repObject;
            if (ifaceName)
            {
                var docBase = "http://www.oxymoronical.com/experiments/apidocs/interface/";
                var url = docBase + ifaceName;
                openWindow( "navigator:browser", url );
                window.open(url, "navigator:browser");
                FirebugChrome.select(Components.interfaces[ifaceName]);
            }
            else
                FBTrace.sysout("onSelectLocation FAILED, no repObject in currentTarget", event.currentTarget);
            if (Components.interfaces[ifaceName] instanceof Components.interfaces.nsIJSIID)
                FBTrace.sysout("onSelectLocation "+ifaceName, Components.interfaces[ifaceName]);
        }
    }

XPCOMExplorer.interfaceListLocator = function(xul_element)
{
    return Chromebug.connectedList(xul_element, XPCOMExplorer.interfaceList);
}

XPCOMExplorer.Overlay = function(href, kind)
{
    this.href = href;
    this.kind = kind;
};

XPCOMExplorer.OverlayPanel = function XPCOMOverlayPanel() {};
XPCOMExplorer.OverlayPanel.prototype = extend(Firebug.HTMLPanel.prototype,
{
    name: "overlay",
    title: "Overlays",
    searchable: false,
    editable: false,

    initialize: function(context, doc)
    {
        Firebug.HTMLPanel.prototype.initialize.apply(this, arguments);
        this.overlayViewer = $('cbOverlayViewer');
    },

    show: function(state)
    {
        this.showToolbarButtons("fbHTMLButtons", false);
        delete this.uri;
    },

    updateSelection: function(info)
    {
        return Firebug.HTMLPanel.prototype.updateSelection.apply(this, arguments);
    },

    getDefaultSelection: function()
    {
        try
        {
            var location = this.getDefaultLocation();
            this.updateLocation(location);
            return this.selection;
        }
        catch (exc)
        {
            return null;
        }
    },

    hide: function()
    {
    },

    supportsObject: function(object)
    {
        if (object instanceof  XULElement  && object.tagName === "browser")
        {
            var url = object.getAttribute('src');
            if (url)
            {
                FBTrace.sysout('overlay.supportsObject '+object.tagName+" url "+url, object)
                var list = this.overlays.getList();
                for( var i = 0; i < list.length; i++)
                    if (list[i].href === url) return 100; // mine all mine
            }

            return 0;
        }

        return 0;
    },

    // ***********************************************************************************

    getLocationList: function()
    {
        return this.getOverlays();
    },

    getDefaultLocation: function()
    {
        var locations = this.getLocationList();
        if (locations && locations.length > 0) return locations[0];
    },

    getObjectLocation: function(overlay)
    {
        return overlay.href;
    },

    getObjectDescription: function(overlay)
    {
        return Chromebug.parseURI(overlay.href);
    },

    updateLocation: function(overlay)
    {
        FBTrace.sysout("overlay.updateLocation "+this.overlayViewer, overlay);
        if (overlay && overlay.href)
        {
            var overlayText = getResource(overlay.href);
            FBTrace.sysout("overlay.updateLocation 2 "+overlayText.length, overlay);

            var parser = Cc["@mozilla.org/xmlextras/domparser;1"].createInstance(Ci.nsIDOMParser);
            var doc = parser.parseFromString(overlayText, "text/xml");
            this.selection = doc.body ? doc.body : getPreviousElement(doc.documentElement.lastChild);
            FBTrace.sysout("overlay.updateLocation this.selection "+this.selection, this.selection);
            this.ioBox.select(this.selection, true, true);
            return;
        }

    },

    // ***********************************************************************************

    getOverlays: function()
    {
        var url = safeGetWindowLocation(this.context.window);
        if (url)
            var uri = makeURI(normalizeURL(url));
        FBTrace.sysout("overlay.getOverlays for "+url, uri);
        return this.overlays.getList(uri);
    },

    overlays:
    {
        overlay_provider: Cc["@mozilla.org/chrome/chrome-registry;1"].getService(Ci.nsIXULOverlayProvider),

        getList: function(uri)
        {
            if (this.list && this.uri === url)
                return this.list;

            var overlays = [];
            if (uri)
            {
                var overlaysEnum = this.overlay_provider.getXULOverlays(uri)
                while(overlaysEnum.hasMoreElements()) {
                    var url = overlaysEnum.getNext().QueryInterface(Components.interfaces.nsIURI).spec;
                        overlays.push( new XPCOMExplorer.Overlay(url, "XUL") );
                }

                var overlaysEnum = this.overlay_provider.getStyleOverlays(uri)
                while(overlaysEnum.hasMoreElements()) {
                    var url = overlaysEnum.getNext().QueryInterface(Components.interfaces.nsIURI).spec;
                    overlays.push(  new XPCOMExplorer.Overlay(url, "Style")  );
                }
            }

            return this.list = overlays;
        },

    }

});

XPCOMExplorer.Panel = function XPCOMPanel() {};
XPCOMExplorer.Panel.prototype = extend(Firebug.DOMBasePanel.prototype,
{
    name: "xpcom",
    title: "XPCOM",
    searchable: false,
    editable: false,

    initialize: function(context, doc)
    {
        Firebug.DOMBasePanel.prototype.initialize.apply(this, arguments);
        //appendStylesheet(doc, "eventBugStyles");

        this.Application       = Cc["@mozilla.org/fuel/application;1"].getService(Ci.extIApplication);
    },

    getDefaultSelection: function()
    {
        FBTrace.sysout("getDefaultSelection");
        this.xpcom =
        {
            categories: this.categories.getList(),
            file_paths: this.paths.getList(),
            Application: this.Application,
        }
        FBTrace.sysout("getDefaultSelection ", this.xpcom);
        return this.xpcom;
    },

    show: function(state)
    {
        if (!this.selection)
            this.selection = this.getDefaultSelection();

        Firebug.DOMBasePanel.prototype.show.apply(this, arguments);

        //this.showToolbarButtons("fbEventButtons", true);

        this.refresh();
    },

    hide: function()
    {
        Firebug.DOMBasePanel.prototype.hide.apply(this, arguments);

        //this.showToolbarButtons("fbEventButtons", false);
    },

    refresh: function()
    {
        if (!this.selection)
            this.selection = this.getDefaultSelection();
        this.rebuild(true);
    },


    getObjectPath: function(object)
    {
        if (FBTrace.DBG_EVENTS)
            FBTrace.sysout("events.getObjectPath NOOP", object);
    },

    supportsObject: function(object)
    {
        if (object instanceof Ci.nsICategoryManager)
            return 1;
        return 0;
    },
    // ***********************************************************************************
    categories:
    {
        category_manager: Cc["@mozilla.org/categorymanager;1"].getService(Ci.nsICategoryManager),
        getList: function()
        {
            var categoryEnumerator = this.category_manager.enumerateCategories();
            var categories = {};
            while( categoryEnumerator.hasMoreElements() )
            {
                var category  = {};
                var categoryName = categoryEnumerator.getNext().QueryInterface(Ci.nsISupportsCString);

                var entryEnumerator = this.category_manager.enumerateCategory(categoryName);
                while( entryEnumerator.hasMoreElements() )
                {
                    var entryName = entryEnumerator.getNext().QueryInterface(Ci.nsISupportsCString);
                    var value = this.category_manager.getCategoryEntry(categoryName, entryName);
                    category[entryName] = value;
                }
                categories[categoryName] = category;
            }
            return categories;
        }
    },
    // ***********************************************************************************
    paths:
    {
        directoryService: directoryService,

        strings: ['ProfD', 'DefProfRt', 'UChrm', 'DefRt', 'PrfDef', 'APlugns', 'AChrom','ComsD', 'Home', 'TmpD', 'ProfLD', 'resource:app', 'Desk', 'Progs', 'BMarks', 'DLoads', 'UStor'],

        getList: function()
        {
            var list = {};
            for (var i = 0; i < this.strings.length; i++)
            {
                var key = this.strings[i];
                try
                {
                    var file = this.directoryService.get(key, Ci.nsIFile);
                    list[key] = file;
                }
                catch(exc)
                {
                    list[key] = "FAILS "+exc;
                }
            }
            return list;
        },
    }

});


/*
 * Attach the XPCOM panel to only the BackstagePass
 */

XPCOMExplorer.Module = extend(Firebug.Module,
{
    dispatchName: "xpcomExplorer",

    initializeUI: function()
    {

    },
    initContext: function(context, persistedState)
    {
        if (context.getName().indexOf('BackstagePass') !== -1)
        {
            context.addPanelTypeConstructor(XPCOMExplorer.Panel);
        }
        else if (context.window && context.window instanceof Window)
        {
            context.addPanelTypeConstructor(XPCOMExplorer.OverlayPanel);
        }
    },
});

// Don't registerPanel we use the addPanelTypeConstructor
Firebug.registerModule(XPCOMExplorer.Module);

}});
