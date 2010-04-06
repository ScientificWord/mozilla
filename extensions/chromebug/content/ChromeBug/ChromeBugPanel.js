/* See license.txt for terms of usage */

/*
 * This file has a lot of experimental code, partly because I am unsure about the model and partly because so
 * much of this is unknown to anyone.
 *
 * For the most part Chromebug is a wrapper around Firebug. You can see from the UI it is Firebug with an extra
 * top toolbar.
 *
 * The biggest difference is that Firebug deals with neatly contained windows whose life time in contained with in
 * browser.xul. Chromebug deals multiple XUL windows and components, some with lifetimes similar to Chromebug itself.
 *
 * Firebug creates a meta-data object called "context" for each web page. (So it incorrectly mixes iframe info)
 * Chromebug creates a context for each nsIDOMWondow plus one for components.
 *
 * Most of Chromebug works on any XUL application. A few uses of DTD or CSS files from 'browser' causes problems
 * however.
 *
 */

FBL.ns(function chromebug() { with (FBL) {

const Cc = Components.classes;
const Ci = Components.interfaces;
const windowWatcher = CCSV("@mozilla.org/embedcomp/window-watcher;1", "nsIWindowWatcher");
const windowMediator = CCSV("@mozilla.org/appshell/window-mediator;1", "nsIWindowMediator");
const nsIDOMWindow = Ci.nsIDOMWindow;
const nsIDOMDocument = Ci.nsIDOMDocument;
const nsIXULWindow = Ci.nsIXULWindow;
const nsIDocShellTreeItem = Ci.nsIDocShellTreeItem;
const nsIDocShell = Ci.nsIDocShell;
const nsIInterfaceRequestor = Ci.nsIInterfaceRequestor;
const nsIWebProgress = Ci.nsIWebProgress;
const nsISupportsWeakReference = Ci.nsISupportsWeakReference;
const nsISupports = Ci.nsISupports;
const nsISupportsCString = Ci.nsISupportsCString;
const jsdIExecutionHook = Components.interfaces.jsdIExecutionHook;

const NOTIFY_ALL = nsIWebProgress.NOTIFY_ALL;
const nsIObserverService = Ci.nsIObserverService
const observerService = CCSV("@mozilla.org/observer-service;1", "nsIObserverService");



const iosvc = CCSV("@mozilla.org/network/io-service;1", "nsIIOService");
const chromeReg = CCSV("@mozilla.org/chrome/chrome-registry;1", "nsIToolkitChromeRegistry");
const directoryService = CCSV("@mozilla.org/file/directory_service;1", "nsIProperties");

const PrefService = Cc["@mozilla.org/preferences-service;1"];
const nsIPrefBranch2 = Components.interfaces.nsIPrefBranch2;
const prefs = PrefService.getService(nsIPrefBranch2);
const nsIPrefService = Components.interfaces.nsIPrefService;
const prefService = PrefService.getService(nsIPrefService);

const fbBox = $("fbContentBox");
const interfaceList = $("cbInterfaceList");
const inspectClearProfileBar = $("fbToolbar");
const appcontent = $("appcontent");
const cbContextList = $('cbContextList');
const cbPackageList = $('cbPackageList');
const versionURL = "chrome://chromebug/content/branch.properties";
const fbVersionURL = "chrome://firebug/content/branch.properties";

const statusText = $("cbStatusText");

// We register a Module so we can get initialization and shutdown signals and so we can monitor context activities
//
Firebug.Chromebug = extend(Firebug.Module,
{
    // This is our interface to TabWatcher
    fakeTabBrowser: {browsers: []},

    getBrowsers: function()
    {
         return Firebug.Chromebug.fakeTabBrowser.browsers;
    },

    getCurrentBrowser: function()
    {
        return this.fakeTabBrowser.selectedBrowser;
    },

    onXULWindowAdded: function(xul_window, outerDOMWindow)
    {
        try
        {
            var context = Firebug.Chromebug.getOrCreateContext(outerDOMWindow, "XUL Window");
            context.xul_window = xul_window;

            if (Chromebug.XULAppModule.stateReloader)  // TODO this should be per xul_window
                outerDOMWindow.addEventListener("DOMContentLoaded", Firebug.Chromebug.stateReloader, true);

            // 'true' for capturing, so all of the sub-window loads also trigger
            context.onLoad = bind(context.loadHandler, context);
            outerDOMWindow.addEventListener("DOMContentLoaded", context.onLoad, true);

            context.onUnload = bind(context.unloadHandler, context)
            outerDOMWindow.addEventListener("unload", context.onUnload, true);

            outerDOMWindow.addEventListener("keypress", bind(Chromebug.XULAppModule.keypressToBreakIntoWindow, this, context), true);
        }
        catch(exc)
        {
            FBTrace.sysout("onXULWindowAdded FAILS "+exc, exc);
        }
    },

    // Browsers in ChromeBug hold our context info

    createBrowser: function(global, fileName)
    {
        var browser = document.createElement("browser");  // in chromebug.xul
        // Ok, this looks dubious. Firebug has a context for every browser (tab), we have a tabbrowser but don;t use the browser really.
        browser.persistedState = null;
        browser.showFirebug = true;
        browser.detached = true;
        browser.webProgress =
        {
            isLoadingDocument: false // we are already in Firefox so we must not be loading...
        };
        browser.addProgressListener = function() {}
        browser.contentWindow = global;
        browser.tag = this.fakeTabBrowser.browsers.length;

        var browserName = null;
        var browserNameFrom = "global.location";
        if (global && global instanceof Window && !global.closed && global.location)
        {
            var browserName = safeToString(global.location);
        }
        else if (isDataURL(browserName))
        {
            var props = splitDataURL(browserName);
            FBTrace.sysout('isDataURL props ', props);
            browserNameFrom = "dataURL";
            browserName = props['decodedfileName'];
        }
        else if (fileName)
        {
            browserNameFrom = "fileName";
            if (fileName.indexOf('jetpack') > 0) // hack around jetpack hack
            {
                browserName = "chrome://jetpack/content/js/jquery-sandbox.js"
            }
            else
                browserName = fileName;
        }
        else
        {
            browserNameFrom = "fake tag";
            var browserName = "chrome://chromebug/fakeTabBrowser/"+browser.tag;
        }

        browser.currentURI = makeURI(browserName);

        if (!browser.currentURI)
        {
            FBTrace.sysout("createBrowser create name from "+browserNameFrom+" gave browserName "+browserName+' FAILED makeURI ' + (global?safeToString(global):"no global") );
        }

        this.fakeTabBrowser.browsers[browser.tag] = browser;
        this.fakeTabBrowser.selectedBrowser = this.fakeTabBrowser.browsers[browser.tag];
        this.fakeTabBrowser.currentURI = browser.currentURI; // allows tabWatcher to showContext
        FBTrace.sysout("createBrowser "+browser.tag+" global:"+(global?safeToString(global):"no global")+" for browserName "+browserName+' with URI '+browser.currentURI.spec);
        return browser;
    },

    selectBrowser: function(browser)
    {
        this.fakeTabBrowser.selectedBrowser = browser;
        this.fakeTabBrowser.currentURI = browser.currentURI; // allows tabWatcher to showContext
    },

    selectContext: function(context)  // this operation is similar to a user picking a tab in Firefox, but for chromebug
    {
        if (!context)
            return;

        Firebug.Chromebug.selectBrowser(context.browser);
        if (FBTrace.DBG_CHROMEBUG)
            FBTrace.sysout("selectContext "+context.getName() + " with context.window: "+safeGetWindowLocation(context.window) );

        if (FirebugContext)
            context.panelName = FirebugContext.panelName; // don't change the panel with the context in Chromebug

        Firebug.Chromebug.contextList.setCurrentLocation(context);

        Firebug.chrome.setFirebugContext(context);  // we've overridden this one to prevent Firebug code from doing...
        FirebugContext = context;                   // ...this

        dispatch(Firebug.modules, "showContext", [context.browser, context]);  // tell modules we may show UI

        Firebug.syncResumeBox(context);
    },

    dispatchName: "chromebug",

    onOptionsShowing: function(menu)
    {
        FBTrace.sysout("Firebug.Chromebug.onOptionsShowing\n");
    },

    getProfileURL: function()
    {
        var profileURL = Firebug.Chromebug.getPlatformStringURL("ProfD");
        return profileURL;
    },

    getPlatformStringURL: function(string)
    {
        var dir = directoryService.get(string, Components.interfaces.nsIFile);
        var URL = FBL.getURLFromLocalFile(dir);
        return URL;
    },

    debug: FBTrace.DBG_CHROMEBUG,

    initialize: function()
    {
        FBTrace.DBG_CHROMEBUG = Firebug.getPref("extensions.chromebug", "DBG_CHROMEBUG");
        FBTrace.DBG_CB_CONSOLE = Firebug.getPref("extensions.chromebug", "DBG_CB_CONSOLE");

        this.fbVersion = Firebug.loadVersion(fbVersionURL);

        this.fullVersion = Firebug.loadVersion(versionURL);
        if (this.fullVersion)
            document.title = "Chromebug "+this.fullVersion +" on Firebug "+this.fbVersion;

        var versionCompat = (parseFloat(this.fbVersion) == parseFloat(this.fullVersion));
        if (!versionCompat)
            document.title =  ">>>>> Chromebug "+this.fullVersion+" requires Firebug "+parseFloat(this.fullVersion)+" but found "+this.fbVersion+" <<<<<";

        if (FBTrace.DBG_CHROMEBUG)
            FBTrace.sysout("Chromebug.initialize this.fullVersion: "+this.fullVersion+" in "+document.location+" title:"+document.title);

        this.uid = FBL.getUniqueId();

        if (!this.contexts)
            this.contexts = TabWatcher.contexts;

        window.arguments[0] = {browser: this.fakeTabBrowser};

        Chromebug.XULAppModule.addListener(this);  // XUL window creation monitoring
        Firebug.Debugger.addListener(this);
        Firebug.Debugger.setDefaultState(true);

        Firebug.registerUIListener(Firebug.Chromebug.allFilesList);

        Firebug.Chromebug.PackageList.addListener(Firebug.Chromebug.allFilesList);  // how changes to the package filter are sensed by AllFilesList

        Firebug.TraceModule.addListener(this);

        if (FBTrace.DBG_CHROMEBUG) FBTrace.sysout("Chromebug.initialize module "+this.uid+" Firebug.Debugger:"+Firebug.Debugger.fbListeners.length+" window.location="+window.location+"\n");

        var wantIntro = prefs.getBoolPref("extensions.chromebug.showIntroduction");

        if (FBTrace.DBG_INITIALIZE)
            FBTrace.sysout("Chromebug.initializeUI from prefs wantIntro: "+wantIntro+"\n");

        if (wantIntro || !versionCompat)
            fbBox.setAttribute("collapsed", true);
        else
            Firebug.Chromebug.toggleIntroduction();

        this.prepareForCloseEvents();
        this.restructureUI();

        // cause onJSDActivate to be triggered.
        this.initializeDebugger();
        // from this point forward scripts should come via debugger interface

        // Wait to let the initial windows open, then return to users past settings
        this.retryRestoreID = setInterval( bind(Firebug.Chromebug.restoreState, this), 500);
        setTimeout( function stopTrying()
        {
            if(Firebug.Chromebug.stopRestoration())  // if the window is not up by now give up.
            {
                var context = Firebug.Chromebug.contextList.getCurrentLocation();
                if (context)
                    Firebug.Chromebug.contextList.setDefaultLocation(context);
                else
                    context = Firebug.Chromebug.contextList.getDefaultLocation();

                Firebug.Chromebug.selectContext(context);
                if (FBTrace.DBG_INITIALIZE)
                    FBTrace.sysout("Had to stop restoration, set context to "+context.getName());
            }

        }, 5000);

        window.addEventListener("unload", function whyIsThis(event)
        {
            FBTrace.sysout("unloading " + window.location, getStackDump());
        }, true);
    },

    prepareForCloseEvents: function()
    {
        // Prepare for close events
        var chromeBugDOMWindow = document.defaultView; // or window.parentNode.defaultView
        this.onUnloadTopWindow = bind(this.onUnloadTopWindow, this); // set onUnload hook on the domWindow of our chromebug
        chromeBugDOMWindow.addEventListener("close", this.onUnloadTopWindow, true);
    },

    restructureUI: function()
    {
        Firebug.setChrome(FirebugChrome, "detached"); // 1.4
    },

    restoreState: function()  // TODO context+file
    {
        var previousStateJSON = prefs.getCharPref("extensions.chromebug.previousContext");
        if (previousStateJSON && previousStateJSON.length > 2)
        {
            var previousState = parseJSONString(previousStateJSON, window.location.toString());
            if (!previousState)
            {
                FBTrace.sysout("restoreState could not parse previousStateJSON "+previousStateJSON);
                this.stopRestoration();
                return;
            }
            else
            {
                FBTrace.sysout("restoreState parse previousStateJSON "+previousStateJSON, previousState);
            }

            var context = this.restoreContext(previousState);
            if (context)
            {
                var pkg = this.restoreFilter(previousState, context);

                this.stopRestoration();

                var panelName = previousState.panelName;
                var sourceLink = previousState.sourceLink;
                // show the restored context, after we let the init finish
                setTimeout( function delayShowContext()
                {
                    if (sourceLink)
                        FirebugChrome.select(sourceLink, panelName);
                    else if (panelName)
                        FirebugChrome.selectPanel(panelName);
                    else
                        FirebugChrome.selectPanel('trace');
                });
            }
            // else keep trying

        }
        else
        {
            if (FBTrace.DBG_INITIALIZE)
                FBTrace.sysout("restoreState NO previousStateJSON ");
            this.stopRestoration(); // no reason to beat our head against the wall...
        }
    },

    restoreContext: function(previousState)
    {
        var name = previousState.contextName;
        var context = Firebug.Chromebug.eachContext(function matchName(context){
            if (context.getName() == name)
                return context;
        });
        if (context)
        {
            Firebug.Chromebug.selectContext( context );
            FBTrace.sysout("restoreState found previousState and set context "+context.getName());
            return context;
        }
        else
        {
            FBTrace.sysout("restoreState did not find context "+name);
            return false;
        }
    },

    restoreFilter: function(previousState, context)
    {
        if (previousState.pkgName)
        {
            var pkg = Firebug.Chromebug.PackageList.getPackageByName(previousState.pkgName);
            if (pkg)
            {
                Firebug.Chromebug.PackageList.setCurrentLocation(pkg.getContextDescription(context));
                FBTrace.sysout("restoreFilter found "+previousState.pkgName+" and set PackageList to ", Firebug.Chromebug.PackageList.getCurrentLocation());
                return pkg;
            }
            else  // we had a package named, but its not available (yet?)
                return false;
        }
        else
        {
            FBTrace.sysout("restoreFilter, no pkgName");
            this.stopRestoration(); // we had a context but no package name, oh well we did our best.
            return false;
        }
    },

    saveState: function(context)  // only call on user operations
    {
        this.stopRestoration();

        var panel = Firebug.chrome.getSelectedPanel();
        if (panel && panel.getSourceLink)
        {
            var sourceLink = panel.getSourceLink();
            if (sourceLink)
                var sourceLinkJSON = sourceLink.toJSON();
        }

        var pkgDescription = Firebug.Chromebug.PackageList.getCurrentLocation();

        var previousContextJSON = "{"+
            " \"contextName\": \"" + context.getName() +"\"," +
            (pkgDescription? (" \"pkgName\": \"" + pkgDescription.pkg.name +"\",") : "") +
            (panel? (" \"panelName\": \"" + panel.name +"\",") : "") +
            (sourceLinkJSON? (" \"sourceLink\": " + sourceLinkJSON+", ") : "") +
            "}";
        prefs.setCharPref("extensions.chromebug.previousContext", previousContextJSON);
        prefService.savePrefFile(null);
        FBTrace.sysout("saveState "+previousContextJSON);
    },

    stopRestoration: function()
    {
        FBTrace.sysout("stopRestoration retryRestoreID: "+this.retryRestoreID);
        if (this.retryRestoreID)
        {
            clearTimeout(this.retryRestoreID);
            delete this.retryRestoreID;
            return true;
        }
        else
            return false;
    },

    setDefaultContext: function()
    {
        Firebug.Chromebug.selectContext(Firebug.Chromebug.contexts[0]);
    },

    initializeDebugger: function()
    {
        fbs.DBG_FBS_FF_START = true;

        if (FBTrace.DBG_INITIALIZE)
            FBTrace.sysout("initializeDebugger start, enabled: "+fbs.enabled+" ******************************");

        Firebug.Debugger.isChromeDebugger = true;
        Firebug.Debugger.wrappedJSObject = Firebug.Debugger;
        Firebug.Debugger.addListener(this);
        if (FBTrace.DBG_INITIALIZE)
            FBTrace.sysout("initializeDebugger complete ******************************");

    },

    onUnloadTopWindow: function(event)
    {
        try
        {
            event.currentTarget.removeEventListener("close", this.onUnloadTopWindow, true);
            FBTrace.sysout("onUnloadTopWindow ", event);
            FirebugChrome.shutdown();
        }
        catch(exc)
        {
            FBTrace.sysout("onUnloadTopWindow FAILS", exc);
        }
    },

    shutdown: function()
    {
        if(Firebug.getPref("extensions.chromebug", 'defaultPanelName')=='ChromeBug')
            Firebug.setPref("extensions.chromebug", 'defaultPanelName','console');
        prefs.setIntPref("extensions.chromebug.outerWidth", window.outerWidth);
        prefs.setIntPref("extensions.chromebug.outerHeight", window.outerHeight);

        if (FBTrace.DBG_CHROMEBUG)
             FBTrace.sysout("Firebug.Chromebug.shutdown set prefs w,h="+window.outerWidth+","+window.outerHeight+")\n");

        window.dump(window.location+ " shutdown:\n "+getStackDump());

        Firebug.shutdown();
        if(FBTrace.DBG_INITIALIZE)
            FBTrace.sysout("ChromeBugPanel.shutdown EXIT\n");
    },

    showPanel: function(browser, panel)
    {
        try {
            if (panel && panel.name == "ChromeBug")
            {
                panel.showPanel();

                if (FBTrace.DBG_CHROMEBUG) FBTrace.sysout("showPanel module:"+this.uid+" panel:"+panel.uid+" location:"+panel.location+"\n");
            }
        } catch(e) {
            FBTrace.sysout("chromebug.showPanel error", e);
        }
    },

    // ******************************************************************************
    createContext: function(global, name)
    {
        var persistedState = null; // TODO
        // domWindow in fbug is browser.contentWindow type nsIDOMWindow.
        // docShell has a nsIDOMWindow interface
        var browser = Firebug.Chromebug.createBrowser(global, name);
        if (global instanceof Ci.nsIDOMWindow)
        {
            var context = TabWatcher.watchTopWindow(global, safeGetWindowLocation(global), true);

            var url = safeToString(global ? global.location : null);
            if (isDataURL(url))
            {
                var lines = context.sourceCache.load(url);
                var props = splitDataURL(url);
                if (props.fileName)
                    context.sourceCache.storeSplitLines(props.fileName, lines);
                FBTrace.sysout("createContext data url stored in to context under "+(props.fileName?props.fileName+ " & ":"just dataURL ")+url);
            }
        }
        else
        {
            var context = TabWatcher.createContext(global, browser, Chromebug.DomWindowContext);
        }

        if (FBTrace.DBG_ACTIVATION)
            FBTrace.sysout('+++++++++++++++++++++++++++++++++ Chromebug.createContext nsIDOMWindow: '+(global instanceof Ci.nsIDOMWindow)+" name: "+context.getName(), context);
        context.onLoadWindowContent = true; // all Chromebug contexts are active
        return context;
    },

    eachContext: function(fnTakesContext)
    {
        for (var i = 0; i < this.contexts.length; i++)
        {
            var rc = fnTakesContext(this.contexts[i]);
            if (rc)
                return rc;
        }
        return false;
    },

    eachSourceFile: function(fnTakesSourceFile)
    {
        return Firebug.Chromebug.eachContext(function visitSourceFiles(context)
        {
            for (var url in context.sourceFileMap)
            {
                if (context.sourceFileMap.hasOwnProperty(url))
                {
                    var sourceFile = context.sourceFileMap[url];
                    var rc = fnTakesSourceFile(sourceFile);
                    if (rc)
                        return rc;
                }
            }
        });
    },

    getContextByJSContextTag: function(jsContextTag)
    {
         return this.eachContext(function findMatch(context)
         {
             if (context.jsContextTag == jsContextTag)
                 return context;
         });
    },

    getOrCreateContext: function(global, name)
    {
        var context = Firebug.Chromebug.getContextByGlobal(global);

        if (FBTrace.DBG_CHROMEBUG)
            FBTrace.sysout("--------------------------- getOrCreateContext got context: "+(context?context.getName():"to be created as "+name));
        if (!context)
            context = Firebug.Chromebug.createContext(global, name);

        return context;
    },

    getContextByGlobal: function(global)
    {
        if (!this.contexts)
            this.contexts = TabWatcher.contexts;


        if (global instanceof Window)
        {
            var docShellType = Chromebug.XULAppModule.getDocumentTypeByDOMWindow(global);
            if (docShellType === "Content")
                return TabWatcher.getContextByWindow(global);
        }

        for (var i = 0; i < this.contexts.length; ++i)
        {
            var context = this.contexts[i];
            if (context.global && (context.global == global)) // will that test work?
                return context;
        }

        if (FBTrace.DBG_CHROMEBUG)
            FBTrace.sysout("getContextByGlobal; no find and not instanceof Content Window "+safeToString(global));

        return null;
    },

    initContext: function(context)
    {
        if (FBTrace.DBG_CHROMEBUG)
        {
                try {
                    FBTrace.sysout("Firebug.Chromebug.Module.initContext "+this.dispatchName+" context: "+context.uid+", "+context.getName()+" FirebugContext="+(FirebugContext?FirebugContext.getName():"undefined")+"\n");
                    //window.dump(getStackDump());
                } catch(exc) {
                    FBTrace.sysout("Firebug.Chromebug.Module.initContext "+exc+"\n");
                }
        }
    },

    loadedContext: function(context)
    {
         if (FBTrace.DBG_CHROMEBUG)
            FBTrace.sysout("Firebug.Chromebug.Module.loadedContext context: "+context.getName()+"\n");
    },
    reattachContext: function(browser, context) // FirebugContext from chrome.js
    {
        // this is called after the chromebug window has opened.
        if (FBTrace.DBG_CHROMEBUG)
            FBTrace.sysout("ChromeBugPanel. reattachContext for context:"+context.uid+" isChromeBug:"+context.isChromeBug+"\n");
    },

    unwatchWindow: function(context, win)
    {
        if (FBTrace.DBG_CHROMEBUG)
            FBTrace.sysout("ChromeBugPanel.unwatchWindow for context:"+context.getName()+"(uid="+context.uid+") isChromeBug:"+context.isChromeBug+"\n");

    },

    destroyContext: function(context)
    {
        if (context.browser)
            delete context.browser.detached;

        Firebug.Chromebug.PackageList.deleteContext(context);

        var currentSelection = Firebug.Chromebug.contextList.getCurrentLocation();

        if ((currentSelection && currentSelection == context) || (!currentSelection) )
        {
         // Pick a new context to be selected.
            var contexts = TabWatcher.contexts;
            for (var i = contexts.length; i; i--)
            {
                nextContext = contexts[i - 1];
                if (!nextContext)
                    FBTrace.sysout("Chromebug destroycontext TabWatcher.contexts has an undefined value at i-1 "+(i-1)+"/"+TabWatcher.contexts.length);
                if (nextContext.window && nextContext.window.closed)
                    continue;
                if (nextContext != context)
                    break;
            }

           if (FBTrace.DBG_CHROMEBUG)
           {
               if (nextContext)
                   FBTrace.sysout("ChromeBugPanel.destroyContext " +context.getName()+" nextContext: "+nextContext.getName());
               else
                   FBTrace.sysout("ChromeBugPanel.destroyContext " +context.getName()+" null nextContext with contexts.length: "+contexts.length);
           }

            Firebug.Chromebug.selectContext(nextContext);
        }
    },


    // ********************************************************
    // implements Firebug.DebuggerListener

    onPauseJSDRequested: function(rejection)
    {
        rejection.push(true);
        FBTrace.sysout("chromebug onPauseJSDRequested: rejection ", rejection);
    },

    onJSDDeactivate: function(active, why)
    {
        FBTrace.sysout("chromebug onJSDDeactivate active: "+active+" why "+why);
    },

    onJSDActivate: function(active, why)  // just before hooks are set in fbs
    {
        if (Firebug.Chromebug.activated)
            return;

        if (FBTrace.DBG_CHROMEBUG)
            FBTrace.sysout("ChromeBug onJSDActivate "+(this.jsContexts?"already have jsContexts":"take the stored jsContexts"));
        try
        {
            var startupObserver = Cc["@getfirebug.com/chromebug-startup-observer;1"].getService(Ci.nsISupports).wrappedJSObject;

            var jsdState = startupObserver.getJSDState();
            if (!jsdState || !jsdState._chromebug)
            {
                setTimeout(function waitForFBTrace()
                {
                    FBTrace.sysout("ChromeBug onJSDActivate NO jsdState! startupObserver:", startupObserver);
                }, 1500);
                return;
            }
            //https://developer.mozilla.org/En/Working_with_windows_in_chrome_code TODO after FF2 is history, us Application.storage
            if (jsdState._chromebug)
            {
                // For now just clear the breakpoints, could try to put these into fbs .onX
                var bps = jsdState._chromebug.breakpointedScripts;
                for (tag in bps)
                {
                   var script = bps[tag];
                   if (script.isValid)
                       script.clearBreakpoint(0);
                }
                delete 	jsdState._chromebug.breakpointedScripts;

                var globals = jsdState._chromebug.globals; // []
                var globalTagByScriptTag = jsdState._chromebug.globalTagByScriptTag; // globals index by script tag
                var globalTagByScriptFileName = jsdState._chromebug.globalTagByScriptFileName; // globals index by script fileName
                var xulScriptsByURL = jsdState._chromebug.xulScriptsByURL;
                Firebug.Chromebug.buildInitialContextList(globals, globalTagByScriptTag, xulScriptsByURL, globalTagByScriptFileName);

                delete jsdState._chromebug.globalTagByScriptTag;
                delete jsdState._chromebug.jsContexts;

                // We turned on jsd to get initial values. Maybe we don't want it on
                if (!Firebug.Debugger.isAlwaysEnabled())
                    fbs.countContext(false); // connect to firebug-service

            }
            else
                FBTrace.sysout("ChromebugPanel.onJSDActivate: no _chromebug in startupObserver, maybe the command line handler is broken\n");

        }
        catch(exc)
        {
            FBTrace.sysout("onJSDActivate fails "+exc, exc);
        }
        finally
        {
            Firebug.Chromebug.activated = true;
            FBTrace.sysout("onJSDActivate exit");
        }
    },

    isChromebugURL: function(URL)
    {
        if (URL)
            return (URL.indexOf("/chromebug/") != -1 || URL.indexOf("/fb4cb/") != -1 || URL.indexOf("/firebug-service.js") != -1);
        else
            return false;
    },

    buildInitialContextList: function(globals, globalTagByScriptTag, xulScriptsByURL, globalTagByScriptFileName)
    {
        var previousContext = {global: null};
        FBL.jsd.enumerateScripts({enumerateScript: function(script)
            {
                var url = normalizeURL(script.fileName);
                if (!url)
                {
                    if (FBTrace.DBG_SOURCEFILES)
                        FBTrace.sysout("buildEnumeratedSourceFiles got bad URL from script.fileName:"+script.fileName, script);
                    return;
                }
                if (script.isValid)
                    script.clearBreakpoint(0);  // just in case

                if (Firebug.Chromebug.isChromebugURL(url))
                {
                    delete globalTagByScriptTag[script.tag];
                    return;
                }

                var globalsTag = globalTagByScriptTag[script.tag];
                if (typeof (globalsTag) == 'undefined' )
                {
                    globalsTag = globalTagByScriptFileName[script.fileName];
                    if ( typeof (globalsTag) == 'undefined' )
                    {
                        //if (FBTrace.DBG_ERRORS)
                            FBTrace.sysout("buildEnumeratedSourceFiles NO globalTag for script tag "+script.tag+" in "+script.fileName);
                        return;
                    }
                    else
                    {
                        FBTrace.sysout("buildEnumeratedSourceFiles globalTag: "+globalsTag+" for "+script.fileName);
                    }
                }

                var global = globals[globalsTag];
                if (global)
                {
                    if (Firebug.Chromebug.isChromebugURL(safeToString(global.location)))
                    {
                        delete globalTagByScriptTag[script.tag];
                        return;
                    }

                    var context = null;
                    if (previousContext.global == global)
                        context = previousContext;
                    else
                        context = Firebug.Chromebug.getOrCreateContext(global, url);
                }

                if (!context)
                {
                    if (! Firebug.Chromebug.unreachablesContext)
                        Firebug.Chromebug.unreachablesContext = Firebug.Chromebug.createContext();
                        Firebug.Chromebug.unreachablesContext.setName("chrome://unreachable/");
                    context =  Firebug.Chromebug.unreachablesContext;
                    FBTrace.sysout("buildEnumeratedSourceFiles NO context for script tag "+script.tag+" in "+script.fileName+" with globalsTag "+globalsTag);
                }
                previousContext = context;

                var sourceFile = context.sourceFileMap[url];

                if (!sourceFile)
                {
                    sourceFile = new Firebug.EnumeratedSourceFile(url);
                    context.addSourceFile(sourceFile);
                }
                if (FBTrace.DBG_SOURCEFILES)
                    FBTrace.sysout("Using globalsTag "+globalsTag+ " assigned "+script.tag+"|"+url+" to "+ context.getName());
                sourceFile.innerScripts[script.tag] = script;

                delete globalTagByScriptTag[script.tag];
            }});
        if (FBTrace.DBG_SOURCEFILES)
        {
            var lostScriptTags = {};
            for (var scriptTag in globalTagByScriptTag)
            {
                var globalTag = globalTagByScriptTag[scriptTag];
                if (!(globalTag in lostScriptTags))
                    lostScriptTags[globalTag] = [];
                lostScriptTags[globalTag].push(scriptTag);
            }
            for (var globalTag in lostScriptTags)
            {
                FBTrace.sysout("Lost scripts from globalTag "+globalTag+" :"+lostScriptTags[globalTag].join(', '));
            }
        }
    },

    getAppShellService: function()
    {
        if (!this.appShellService)
            this.appShellService = Components.classes["@mozilla.org/appshell/appShellService;1"].
                getService(Components.interfaces.nsIAppShellService);
        return this.appShellService;
    },

    onStop: function(context, frame, type, rv)
    {
        // FirebugContext is not context. Maybe this does not happen in firebug because the user always starts
        // with an active tab with FirebugContext and cause the breakpoints to land in the default context.
        if (FirebugContext != context)
            Firebug.showContext(context.browser, context);

        var stopName = getExecutionStopNameFromType(type);
        if (FBTrace.DBG_UI_LOOP)
            FBTrace.sysout("ChromeBugPanel.onStop type: "+stopName+ " context.getName():"+context.getName() + " context.stopped:"+context.stopped );

        try
        {
            var src = frame.script.isValid ? frame.script.functionSource : "<invalid script>";
        } catch (e) {
            var src = "<invalid script>";
        }

        if (FBTrace.DBG_UI_LOOP)
            FBTrace.sysout("ChromeBugPanel.onStop script.tag: "+frame.script.tag+" @"+frame.line+":"+frame.pc+" in "+frame.script.fileName, "source:"+src);

        var cbContextList = document.getElementById('cbContextList');
        cbContextList.setAttribute("highlight", "true");

        // The argument 'context' is stopped, but other contexts on the stack are not stopped.
        var calledFrame = frame;
        while (frame = frame.callingFrame)
        {
            var callingContext = Firebug.Debugger.getContextByFrame(frame);
            if (FBTrace.DBG_UI_LOOP)
                FBTrace.sysout("ChromeBugPanel.onStop stopping context: "+(callingContext?callingContext.getName():null));
            if (callingContext)
            {
                callingContext.stopped = true;
                callingContext.debugFrame = calledFrame;
            }
        }

        return -1;
    },

    onResume: function(context)
    {
        var cbContextList = document.getElementById('cbContextList');
        cbContextList.removeAttribute("highlight");

        var panel = context.getPanel("script", true);
        if (panel && panel.location)
        {
            var location = "0@"+panel.location.href;

            if (panel.selectedSourceBox) // see firebug.js buildViewAround
                var lineNo = panel.selectedSourceBox.firstViewableLine + panel.selectedSourceBox.halfViewableLines;

            if (lineNo)
                location = lineNo+"@"+panel.location.href;

            //prefs.setCharPref("extensions.chromebug.previousContext", location);
            //prefService.savePrefFile(null);

            if (FBTrace.DBG_INITIALIZE)
                FBTrace.sysout("ChromeBugPanel.onResume previousContext:"+ location);
        }

        Firebug.Chromebug.eachContext(function clearStopped(context)
        {
            delete context.stopped;
            delete context.debugFrame;
        });

        FBTrace.sysout("ChromeBugPanel.onResume context.getName():"+context.getName() + " context.stopped:"+context.stopped );

    },

    onThrow: function(context, frame, rv)
    {
        return false; /* continue throw */
    },

    onError: function(context, frame, error)
    {
    },


    onFunctionConstructor: function(context, frame, ctor_script, url)
    {
        FBTrace.sysout("ChromeBug onFunctionConstructor");
    },

    onSourceFileCreated: function(context, sourceFile)
    {
        var description = Chromebug.parseURI(sourceFile.href);
        var pkg = Firebug.Chromebug.PackageList.getOrCreatePackage(description);
        pkg.appendContext(context);
        if (FBTrace.DBG_SOURCEFILES)
            FBTrace.sysout("onSourceFileCreated sourceFile "+sourceFile.href+" in  "+pkg.name+" context "+context.getName());
    },

    getGlobalByFrame: function(frame)
    {
        return fbs.getOutermostScope(frame);
    },
    //******************************************************************************
    // traceModule listener

    onLoadConsole: function(win, rootNode)
    {
        if (rootNode)
        {
            rootNode.addEventListener("click", function anyClick(event)
            {
                //FBTrace.sysout("Chromebug click on traceConsole ", event);
                var isAStackFrame = hasClass(event.target, "stackFrameLink");
                if (isAStackFrame)
                {
                    var context = FirebugContext;
                    var info = getAncestorByClass(event.target, "messageInfoBody");
                    var message = info.repObject;
                    if (!message && info.wrappedJSObject)
                        message = info.wrappedJSObject.repObject;
                    if (message)
                    {
                        var filename = encodeURI(event.target.text);
                        var line = event.target.getAttribute("lineNumber");

                        var found = Firebug.Chromebug.allFilesList.eachSourceFileDescription(function findMatching(d)
                        {
                            var testName = d.href;
                            //FBTrace.sysout("click traceConsole filename "+ filename +"=?="+testName);
                            if (testName == filename)
                            {
                                var context = d.context;
                                if (context)
                                {
                                    Firebug.Chromebug.selectContext(context);
                                    FBTrace.sysout("onLoadConsole.eventListener found matching description: "+d+" context set to "+context.getName(), message);
                                    var link = new SourceLink(filename, line, "js" );
                                    FBTrace.sysout("Chromebug click on traceConsole isAStackFrame SourceLink:"+(link instanceof SourceLink), {target: event.target, href: filename, lineNo:line, link:link});
                                    Firebug.chrome.select(link, "script");
                                    return true;
                                }
                                else
                                    FBTrace.sysout("onLoadConsole.eventListener no context in matching description", d);
                            }
                            return false;
                        });
                        if (!found)
                        {
                            if (filename) // Fallback is to just open the view-source window on the file
                                viewSource(filename, line);
                            else
                                FBTrace.sysout("onLoadConsole.eventListener no filename in event target", event);
                        }
                    }
                    else
                        FBTrace.sysout("onLoadConsole.eventListener no message found on info", info);

                    event.stopPropagation();
                }

            },true);
            FBTrace.sysout("Chromebug onLoadConsole set hook");
        }
    },
    //******************************************************************************

    formatLists: function(header, lists)
    {
        var str = header;
        for (listName in lists)
        {
            str += listName + "\n";
            var list = lists[listName];
            for (var i = 0; i < list.length; i++ )
            {
                str += "   "+list[i].toString() + "\n";
            }
        }
        return str;
    },

    createSourceFile: function(sourceFileMap, script)
    {
            var url = normalizeURL(script.fileName);
            if (!url)
            {
                FBTrace.sysout("createSourceFile got bad URL from script.fileName:"+script.fileName, script);
                return;
            }
            var sourceFile = sourceFileMap[url];
            if (!sourceFile)
            {
                sourceFile = new Firebug.EnumeratedSourceFile(url);
                sourceFileMap[url] = sourceFile;
                if (FBTrace.DBG_SOURCEFILES)
                    FBTrace.sysout("Firebug.Chromebug.createSourceFile script.fileName="+url+"\n");
            }
            sourceFile.innerScripts.push(script);
    },

    addComponentScripts: function(sourceFileMap)
    {
        FBL.jsd.enumerateScripts({enumerateScript: function(script)
        {
            var url = normalizeURL(script.fileName);
            var c = Chromebug.parseComponentURI(url);
            if (c)
            {
                var sourceFile = sourceFileMap[url];
                if (!sourceFile)
                {
                    sourceFile = new Firebug.EnumeratedSourceFile(url);
                    sourceFileMap[url] = sourceFile;
                    var name = c.pkgName;
                    sourceFile.component = name;
                    if (FBTrace.DBG_SOURCEFILES)
                        FBTrace.sysout("Firebug.Chromebug.getComponents found script.fileName="+url+"\n");
                }
                sourceFile.innerScripts.push(script);
            }
            else
            {
                if (url.indexOf('/components/') != -1)
                    FBTrace.sysout("Firebug.Chromebug.getComponents missed="+url+"\n");
            }
        }});
    },

    //*****************************************************************************

    watchDocument: function(window)
    {
        var doc = window.document;
        doc.addEventListener("DOMAttrModified", this.onMutateAttr, false);
        doc.addEventListener("DOMCharacterDataModified", this.onMutateText, false);
        doc.addEventListener("DOMNodeInserted", this.onMutateNode, false);
        doc.addEventListener("DOMNodeRemoved", this.onMutateNode, false);
    },

    unWatchDocument: function(window)
    {
        var doc = window.document;
        doc.removeEventListener("DOMAttrModified", this.onMutateAttr, false);
        doc.removeEventListener("DOMCharacterDataModified", this.onMutateText, false);
        doc.removeEventListener("DOMNodeInserted", this.onMutateNode, false);
        doc.removeEventListener("DOMNodeRemoved", this.onMutateNode, false);
    },

    onMutateAttr: function(event) {
        FBTrace.sysout("Firebug.Chromebug.onMutateAttr\n");
    },

    onMutateText: function(event) {
        FBTrace.sysout("Firebug.Chromebug.onMutateText\n");
    },

    onMutateNode: function(event) {
        FBTrace.sysout("Firebug.Chromebug.onMutateNode\n");
    },
    //**************************************************************************************
    // Commands

    toggleIntroductionTrue: function()
    {
        prefs.setBoolPref("extensions.chromebug.showIntroduction", false);
        prefService.savePrefFile(null);
        Firebug.Chromebug.toggleIntroduction();
    },

    showIntroduction: true,

    toggleIntroduction: function()
    {
        if (FBTrace.DBG_INITIALIZE)
            FBTrace.sysout("toggleIntroduction ", "Firebug.Chromebug.showIntroduction "+ Firebug.Chromebug.showIntroduction);
        Firebug.Chromebug.showIntroduction = !Firebug.Chromebug.showIntroduction;

        if (Firebug.Chromebug.showIntroduction)
        {
            $('content').removeAttribute("collapsed");
            fbBox.setAttribute("collapsed", true);
        }
        else
        {
            $('content').setAttribute("collapsed", true);
            fbBox.removeAttribute("collapsed");
        }

    },

    setStatusText: function(text)
    {
         statusText.setAttribute("value", text);
         statusText.setAttribute("collapsed", false);
    },

    reload: function(skipCache)
    {
        // get the window we plan to kill
        var current_location = Firebug.Chromebug.contextList.getCurrentLocation();
        FBTrace.sysout("Firebug.Chromebug.reload current_location", Firebug.Chromebug.contextList.getObjectLocation(current_location));
        if (current_location && current_location.getContainingXULWindow)
        {
            var xul_window = current_location.getContainingXULWindow();
            if (xul_window && xul_window instanceof nsIXULWindow)
            {
                var reloadedWindow = Firebug.Chromebug.reloadWindow(xul_window);
                if (reloadedWindow)
                    return;
            }
        }
        // else ask for a window
        cbContextList.showPopup();
    },

    chromeList: function()
    {
        var w = window.screen.availWidth;
        var h = window.screen.availHeight;
        features = "outerWidth="+w+","+"outerHeight="+h;
        var params = "";
        this.chromeList = openWindow('chromelist', "chrome://chromebug/content/chromelist.xul", features, params);
    },

    openXPCOMExplorer: function()
    {
        var xpcomExplorerURL = "chrome://xpcomExplorer/content/xpcomExplorer.xul";
        this.xpcomExplorer = openWindow('xpcomExplorer',xpcomExplorerURL);
    },

    openProfileDir: function(context)
    {
        var profileFolder = directoryService.get("ProfD", Ci.nsIFile);
        var path = profileFolder.QueryInterface(Ci.nsILocalFile).path;
        var fileLocal = CCIN("@mozilla.org/file/local;1", "nsILocalFile");
        fileLocal.initWithPath(path);
        fileLocal.launch();
    },

    exitFirefox: function()
    {
        goQuitApplication();
    },

    avoidStrict: function(subject, topic, data)
    {
        if (data)
        {
            var c = data.indexOf("options.strict");
            if (c >= 0)
                prefs.clearUserPref("javascript.options.strict"); // avoid crashing FF
            FBTrace.sysout("Firebug.Chromebug.avoidStrict prefs\n");
        }
        else
            FBTrace.sysout("CHromeBug.avoidStrict no data for subject", subject);
    },

    createDirectory: function(parent, name)
    {
        var subdir = parent.clone();
        subdir.append(name);
        if( !subdir.exists() || !subdir.isDirectory() ) {   // if it doesn't exist, create
            subdir.create(Components.interfaces.nsIFile.DIRECTORY_TYPE, 0777);
        }
        return subdir;
    },

    dumpDirectory: function()
    {
        FBTrace.sysout("dumpDirectory begins\n", directoryService);
        if (directoryService instanceof Components.interfaces.nsIProperties)
        {
            FBTrace.sysout("dumpDirectory finds an nsIProperties\n");
            var keys = directoryService.getKeys({});
            FBTrace.sysout("dumpDirectory has "+keys.length+"\n");
            for (var i = 0; i < keys.length; i++)
                FBTrace.sysout(i+": "+keys[i]+"\n");
        }

    },

    writeString: function(file, data)
    {
        // http://developer.mozilla.org/en/docs/Code_snippets:File_I/O
        //file is nsIFile, data is a string
        var foStream = Components.classes["@mozilla.org/network/file-output-stream;1"]
                         .createInstance(Components.interfaces.nsIFileOutputStream);

        // use 0x02 | 0x10 to open file for appending.
        foStream.init(file, 0x02 | 0x08 | 0x20, 0666, 0); // write, create, truncate
        foStream.write(data, data.length);
        foStream.close();
    },

    dumpStackToConsole: function(context, title)
    {
        if (FBTrace.DBG_STACK) FBTrace.sysout("ChromeBugPanel.dumpStackToConsole for: ", title);
        var trace = FBL.getCurrentStackTrace(context);  // halt(), getCurrentStackTrace(), dumpStackToConsole(), =>3
        if (trace)
        {
            trace.frames = trace.frames.slice(3);

            Firebug.Console.openGroup(title, context)
            Firebug.Console.log(trace, context, "stackTrace");
            Firebug.Console.closeGroup(context, true);
        }
        else
            if (FBTrace.DBG_STACK) FBTrace.sysout("ChromeBugPanel.dumpStackToConsole FAILS for "+title, " context:"+context.getName());
    },

    openAboutDialog: function()
    {
        var extensionManager = CCSV("@mozilla.org/extensions/manager;1", "nsIExtensionManager");
        openDialog("chrome://mozapps/content/extensions/about.xul", "",
            "chrome,centerscreen,modal", "urn:mozilla:item:chromebug@johnjbarton.com", extensionManager.datasource);
    },

    onClickStatusIcon: function()
    {
        Firebug.Chromebug.contextAnalysis(FirebugContext);
    },

    contextAnalysis: function(context)
    {
        if (!FirebugContext)
            return;
        Firebug.Console.openGroup("Context Analysis", FirebugContext)
        Firebug.Console.log(Firebug.Chromebug.contexts, FirebugContext);
        Firebug.Console.log(Firebug.Chromebug.jsContexts, FirebugContext);
        var ejs = fbs.eachJSContext();
        if (ejs)
            Firebug.Console.log(ejs, FirebugContext);
        if (context)
            Firebug.Console.log(context, FirebugContext);
        else
            Firebug.Console.log(FirebugContext, FirebugContext);
        Firebug.Console.closeGroup(FirebugContext, true);
    },


});


window.timeOut = function(title)
{
    var t = new Date();
    if (window.startTime)
        window.dump(title+": "+(t - window.startTime)+"\n");
    window.startTime = t;
}

Firebug.Chromebug.Package = function(name, kind)
{
    this.name = name;
    this.kind = kind;

    this.contexts = [];

    FBTrace.sysout("Create Package "+name+"("+kind+")");
}

Firebug.Chromebug.Package.prototype =
{
    appendContext: function(context)
    {
        if (this.hasContext(context))
            return;

        this.contexts.push(context);
        context.pkg = this;
        if (FBTrace.DBG_LOCATIONS)
            FBTrace.sysout("appendContext "+context.getName()+" to package "+this.name+" total contexts:"+this.contexts.length);
    },

    getContexts: function()
    {
        return this.contexts;
    },

    getContextDescription: function(context)
    {
        return {context: context, pkg: this, label: this.name};
    },

    getContextDescriptions: function()
    {
        var descriptions = [];
        var thePackage = this;
        this.eachContext(function buildList(context)
        {
            descriptions.push( thePackage.getContextDescription(context) );
        });
        return descriptions;
    },

    hasContext: function(context)
    {
        var found = this.eachContext( function seek(areYouMine)
        {
            if (context.uid == areYouMine.uid)
                return areYouMine;
        });
        return found;
    },

    eachContext: function(fnTakesContext)
    {
        for (var i = 0; i < this.contexts.length; i++)
        {
            var rc = fnTakesContext(this.contexts[i]);
            if (rc)
                return rc;
        }
        return false;
    },

    deleteContext: function(context)
    {
        var i = this.contexts.indexOf(context);
        this.contexts.slice(i, 1);
    },

}
//**************************************************************************
// chrome://<packagename>/<part>/<file>
// A list of packages each with a context list
//
Firebug.Chromebug.PackageList = extend(new Firebug.Listener(),
{
    //  key name of package, value Package object containing contexts
    pkgs: {},

    getPackageByName: function(name)
    {
        return this.pkgs[name];
    },

    eachPackage: function(fnTakesPackage)
    {
        for (var p in this.pkgs)
        {
            if (this.pkgs.hasOwnProperty(p))
            {
                var rc = fnTakesPackage(this.pkgs[p]);
                if (rc)
                    return rc;
            }
        }
    },

    eachContext: function(fnTakesContext)  // this will visit each context more than one time!
    {
        return this.eachPackage( function overContexts(pkg)
        {
            return pkg.eachContext(fnTakesContext);
        });
    },

    getSummary: function(where)
    {
        var str = where + ": All Packages =(";
        this.eachPackage(function sayAll(pkg){ str+=pkg.name+","; });
        str[str.length - 1] = ")";
        return str;
    },

    getOrCreatePackage: function(description)
    {
        var pkgName = description.pkgName;
        var kind = description.kind;

        if (!this.pkgs.hasOwnProperty(pkgName))
            this.pkgs[pkgName] = new Firebug.Chromebug.Package(pkgName, kind);

        return this.pkgs[pkgName];
    },

    assignContextToPackage: function(context)  // a window context owned by a package
    {
        var url = context.getName();
        var description = Chromebug.parseURI(url);
        if (description && description.path)
        {
            var pkg = this.getOrCreatePackage(description);
            pkg.appendContext(context);
        }
        else
        {
            if (FBTrace.DBG_CHROMEBUG || FBTrace.DBG_LOCATIONS)
                FBTrace.sysout("PackageList skipping context "+url);
        }
    },

    deleteContext: function(context)
    {
        this.eachPackage(function deleteContextFromPackage(pkg)
        {
            if (pkg.hasContext(context))
                pkg.deleteContext(context);
        });
    },

    getCurrentLocation: function() // a context filtered by package
    {
        return cbPackageList.repObject;
    },

    setCurrentLocation: function(filteredContext)
    {
          cbPackageList.location = filteredContext;
          if (FBTrace.DBG_LOCATIONS)
              FBTrace.sysout("PackageList.setCurrentLocation sent onSetLocation to "+this.fbListeners.length);
          dispatch(this.fbListeners, "onSetLocation", [this, filteredContext]);
    },

    getLocationList: function()  // list of contextDescriptions
    {
        var list = [];
        for (var p in this.pkgs)
        {
            if (this.pkgs.hasOwnProperty(p))
                list = list.concat(this.pkgs[p].getContextDescriptions());
        }

        list.push(this.getDefaultLocation());

        if (FBTrace.DBG_LOCATIONS)
        {
            FBTrace.sysout(this.getSummary("getLocationList"));
            FBTrace.sysout("PackageList getLocationList list "+list.length, list);
        }

        return list;
    },

    getFilter: function()
    {
        var current = this.getCurrentLocation();
        if (current && current.pkg.name != this.getDefaultPackageName())
            return current.pkg.name;
        else
            return "";
    },

    getDefaultPackageName: function()
    {
        return "        No Filtering, Current Context:   "; // in lexographical order the spaces will be early
    },

    getDefaultLocation: function()
    {
        return {context: FirebugContext, pkg: {name: this.getDefaultPackageName() }, label: "(no filter)"};
    },

    getObjectLocation: function(filteredContext)
    {
        return filteredContext.pkg.name;
    },

    getObjectDescription: function(filteredContext)
    {
        var context = filteredContext.context;
        var title = (context.window ? context.getTitle() : null);
        var d =  {path: filteredContext.pkg.name, name: context.getName() +(title?"   "+title:""), label:  filteredContext.label};
        if (FBTrace.DBG_LOCATIONS)
            FBTrace.sysout("getObjectDescription for context "+context.uid+" path:"+d.path+" name:"+d.name, d);
        return d;
    },


    onSelectLocation: function(event)
    {
        var filteredContext = event.currentTarget.repObject;
        if (filteredContext)
        {
            var context = filteredContext.context;
            Firebug.Chromebug.selectContext(context);
            Firebug.Chromebug.PackageList.setCurrentLocation(filteredContext);
       }
       else
       {
           FBTrace.sysout("onSelectLocation FAILED, no repObject in currentTarget", event.currentTarget);
       }
    },

});

Firebug.Chromebug.packageListLocator = function(xul_element)
{
    var list = Firebug.Chromebug.PackageList;
    return Chromebug.connectedList(xul_element, list);
}

//**************************************************************************
// The implements the list on the right side of the top bar with the prefix "context:"

Firebug.Chromebug.contextList =
{

    getCurrentLocation: function() // a context in a package
    {
        return cbContextList.repObject;
    },

    setCurrentLocation: function(context) // call from Firebug.Chromebug.selectContext(context);
    {
        cbContextList.location = context;  // in binding.xml, both the location and repObject are changed in the setter.
    },

    getLocationList: function()  // list of contextDescriptions
    {
        var list = Firebug.Chromebug.contexts;

        if (FBTrace.DBG_LOCATIONS)
            FBTrace.sysout("ContextList getLocationList list "+list.length, list);

        return list;
    },

    getDefaultLocation: function()
    {
        if (this.defaultContext && Firebug.Chromebug.contexts.indexOf(this.defaultContext) != -1)
            return this.defaultContext;

        var locations = this.getLocationList();
        if (locations && locations.length > 0) return locations[0];
    },

    setDefaultLocation: function(context)
    {
        if (context)
            this.defaultContext = context;
    },

    getObjectLocation: function(context)
    {
        return  context.getWindowLocation();
    },

    getObjectDescription: function(context)
    {
        var title = (context.window ? context.getTitle() : null);
        var d = Chromebug.parseURI(context.getName());

        d = {
           path: d ? d.path : "parseURI fails",
           name: (d ? d.name : context.getName()) +(title?"   "+title:""),
           label: context.getName(),
           href: context.getName(),
           highlight: context.stoppped,
           };

        if (FBTrace.DBG_LOCATIONS)
            FBTrace.sysout("getObjectDescription for context "+context.uid+" path:"+d.path+" name:"+d.name, d);
        return d;
    },

    onSelectLocation: function(event)
    {
        var context = event.currentTarget.repObject;
        if (context)
        {
            if (!FirebugContext)
                FirebugContext = context;

            Firebug.Chromebug.selectContext(context);

            if (FBTrace.DBG_LOCATIONS)
                FBTrace.sysout("Chromebug.contextList.onSelectLocation context:"+ context.getName()+" FirebugContext:"+FirebugContext.getName());

            event.currentTarget.location = context;

            setTimeout( function delaySave()  // we only want to do this when the user selects
            {
                Firebug.Chromebug.saveState(context);
            }, 500);

        }
        else
        {
            FBTrace.sysout("onSelectLocation FAILED, no repObject in currentTarget", event.currentTarget);
        }
    },

    /*
     * Called when the filelist is already showing, highlight any stopped contexts.
     */
    onPopUpShown: function(event)
    {
        var list = document.getAnonymousElementByAttribute(event.currentTarget, "anonid", "popup");

        if (FBTrace.DBG_SOURCEFILES)
            FBTrace.sysout("Context on popup shown "+list, list);

        var child = list.firstChild;
        while(child)
        {
            if (FBTrace.DBG_SOURCEFILES)
                FBTrace.sysout("Context onPopUpShown "+child+ " has repObject "+(child.repObject?child.repObject.getName():"none"));

            if (child.repObject && child.repObject.stopped)
                child.setAttribute('highlight', "true");

            child = child.nextSibling;
        }
    },

    getContextByURL: function(url)
    {
        return Firebug.Chromebug.eachContext(function matchURL(context)
        {
            if (safeToString(context.global.location) === url)
                return context;
        });
    }
}

Firebug.Chromebug.contextListLocator = function(xul_element)
{
    var list = Firebug.Chromebug.contextList;
    return Chromebug.connectedList(xul_element, list);
}



Firebug.Chromebug.allFilesList = extend(new Chromebug.SourceFileListBase(), {

    kind: "all",

    parseURI: top.Chromebug.parseURI,


    // **************************************************************************
    // PackageList listener

    onSetLocation: function(packageList, current)
    {
        FBTrace.sysout("onSetLocation current: "+current.pkg.name);
        var noFilter = packageList.getDefaultPackageName();
        if (current.pkg.name == noFilter)
            Firebug.Chromebug.allFilesList.setFilter(null)
        else
        {
            var targetName = current.pkg.name;
            Firebug.Chromebug.allFilesList.setFilter( function byPackageName(description)
            {
                return (description && (targetName == description.pkgName) );
            });
        }
    },

    //**************************************************************************
    // Sync the Chromebug file list to the context's Script file list.
    onPanelNavigate: function(object, panel)
    {
        if (panel.name !== "script")
            return;

        var sourceFile = object;

        if (FBTrace.DBG_LOCATIONS)
            FBTrace.sysout("onPanelNavigate "+sourceFile, panel);

        if (sourceFile)
               $('cbAllFilesList').location = this.getDescription(sourceFile);
    },

});

Firebug.Chromebug.allFilesListLocator = function(xul_element)
{
    if (FBTrace.DBG_LOCATIONS)
        FBTrace.sysout("AllFilesListLocator called");
    return Chromebug.connectedList(xul_element, Firebug.Chromebug.allFilesList);
}


Firebug.Chromebug.dumpFileTrack = function()
{
    var jsdState = Application.storage.get('jsdState', null);
    if (jsdState)
        fbs.dumpFileTrack(jsdState.getAllTrackedFiles());
    else
        FBTrace.sysout("dumpFileTrack, no jsdState!");
}

function getFrameWindow(frame)
{
   // if (debuggers.length < 1)  // too early, frame.eval will crash FF2
    //        return;
    try
    {
        var result = {};
        frame.eval("window", "", 1, result);
        var win = new XPCNativeWrapper(result.value.getWrappedValue());
        FBTrace.sysout("getFrameWindow eval window is ", win.location);
        return getRootWindow(win);
    }
    catch (exc)
    {
        if (FBTrace.DBG_ERRORS && FBTrace.DBG_WINDOWS)
            FBTrace.sysout("ChromeBugPanel getFrameWindow fails: ", exc);  // FBS.DBG_WINDOWS
        return null;
    }
}

function getRootWindow(win)
{
    for (; win; win = win.parent)
    {
        if (FBTrace.DBG_WINDOWS)
            FBTrace.sysout("getRootWindow win.parent is ", (win.parent?win.parent.location:"null"));
        if (!win.parent || win == win.parent)
            return win;
    }
    return null;
}

function getExecutionStopNameFromType(type)                                                                            /*@explore*/
{                                                                                                                      /*@explore*/
    switch (type)                                                                                                      /*@explore*/
    {                                                                                                                  /*@explore*/
        case jsdIExecutionHook.TYPE_INTERRUPTED: return "interrupted";                                                 /*@explore*/
        case jsdIExecutionHook.TYPE_BREAKPOINT: return "breakpoint";                                                   /*@explore*/
        case jsdIExecutionHook.TYPE_DEBUG_REQUESTED: return "debug requested";                                         /*@explore*/
        case jsdIExecutionHook.TYPE_DEBUGGER_KEYWORD: return "debugger_keyword";                                       /*@explore*/
        case jsdIExecutionHook.TYPE_THROW: return "interrupted";                                                       /*@explore*/
        default: return "unknown("+type+")";                                                                           /*@explore*/
    }                                                                                                                  /*@explore*/
}

function remove(list, item)
{
    var index = list.indexOf(item);
    if (index != -1)
        list.splice(index, 1);
}

function ChromeBugOnLoad(event)
{
    FBTrace.sysout("ChromeBugOnLoad "+event.originalTarget.documentURI+"\n");
}
function ChromeBugOnDOMContentLoaded(event)
{
    FBTrace.sysout("ChromeBugOnDOMContentLoaded "+event.originalTarget.documentURI+"\n");
}



Firebug.registerModule(Firebug.Chromebug);
}});

