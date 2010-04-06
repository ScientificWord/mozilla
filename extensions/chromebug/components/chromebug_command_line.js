/* See license.txt for terms of usage */

const  nsIAppShellService    = Components.interfaces.nsIAppShellService;
const  nsISupports           = Components.interfaces.nsISupports;
const  nsICategoryManager    = Components.interfaces.nsICategoryManager;
const  nsIComponentRegistrar = Components.interfaces.nsIComponentRegistrar;
const  nsICommandLine        = Components.interfaces.nsICommandLine;
const  nsICommandLineHandler = Components.interfaces.nsICommandLineHandler;
const  nsIFactory            = Components.interfaces.nsIFactory;
const  nsIModule             = Components.interfaces.nsIModule;


const PrefService = Components.classes["@mozilla.org/preferences-service;1"];
const nsIPrefBranch2 = Components.interfaces.nsIPrefBranch2;
const prefs = PrefService.getService(nsIPrefBranch2);

const iosvc = Components.classes["@mozilla.org/network/io-service;1"].getService(Components.interfaces.nsIIOService);
const chromeReg = Components.classes["@mozilla.org/chrome/chrome-registry;1"].getService(Components.interfaces.nsIToolkitChromeRegistry);
const appShellService = Components.classes["@mozilla.org/appshell/appShellService;1"].getService(Components.interfaces.nsIAppShellService);

const appInfo =  Components.classes["@mozilla.org/xre/app-info;1"].getService(Components.interfaces.nsIXULAppInfo);

const  clh_CID = Components.ID("{B5D5631C-4FE1-11DB-8373-B622A1EF5492}");
const  clh_contractID = "@mozilla.org/commandlinehandler/general-startup;1?type=chromebug";

const Cc = Components.classes;
const Ci = Components.interfaces;
const Cr = Components.results;

// category names are sorted alphabetically. Typical command-line handlers use a
// category that begins with the letter "m".
const  clh_category = "aaa-chromebug";

const nsIWindowMediator = Components.interfaces.nsIWindowMediator;
const reXUL = /\.xul$|\.xml$|^XStringBundle$|\/modules\//;

const trace = false;
/**
 * The XPCOM component that implements nsICommandLineHandler.
 * It also implements nsIFactory to serve as its own singleton factory.
 */
const  chromebugCommandLineHandler = {

    debug: false,

    openWindow: function(opener, windowType, url, w, h, params)
    {
        var wm = Components.classes["@mozilla.org/appshell/window-mediator;1"].
                    getService(Components.interfaces.nsIWindowMediator);

        var win = windowType ? wm.getMostRecentWindow(windowType) : null;

        if (win)
        {
            if ("initWithParams" in win)
                  win.initWithParams(params);
            win.focus();
            Components.utils.reportError("chromebugCommandLineHandler reused a window");
        }
        else
        {
            if (!w)
                w = opener.screen.availWidth;
            if (!h)
                h = opener.screen.availHeight;

            if (!w || w < 200) // see issue 2576
                w = 400;
            if (!h || h < 200)
                h = 300;

            var features = "outerWidth="+w+","+"outerHeight="+h;

            var winFeatures = "resizable,dialog=no,centerscreen" + (features != "" ? ("," + features) : "");
            if (chromebugCommandLineHandler.debug)
            {
                Components.utils.reportError("chromebug_command_line opening window with features: "+features);
            }
            win = opener.openDialog(url, "_blank", winFeatures, params);
        }
        return win;
    },

    openChromebug: function(window)
    {
        var inType = "chromebug:ui"; // MUST BE windowType on chromebug.xul
        var url = "chrome://chromebug/content/chromebug.xul";
        var title = "ChromeBug";

        var prefedWidth = prefs.getIntPref("extensions.chromebug.outerWidth");
        var prefedHeight = prefs.getIntPref("extensions.chromebug.outerHeight");

        var chromeBugWindow = this.openWindow(window, inType, url, prefedWidth, prefedHeight);
        chromeBugWindow.document.title = title;

        if (chromebugCommandLineHandler.debug)
        {
            var chromeURI = iosvc.newURI(url, null, null);
            var localURI = chromeReg.convertChromeURL(chromeURI);
            Components.utils.reportError(title+" maps "+url+' to '+localURI.spec);
            Components.utils.reportError("ChromeBug x,y,w,h = ["+chromeBugWindow.screenX+","+chromeBugWindow.screenY+","+
               chromeBugWindow.width+","+chromeBugWindow.height+"]");
        }

        return chromeBugWindow;
    },


  /* nsISupports */
  QueryInterface : function clh_QI(iid)
  {
    if (iid.equals(nsICommandLineHandler) ||
        iid.equals(nsIFactory) ||
        iid.equals(nsISupports))
    {
        this.wrappedJSObject = this;
        return this;
    }

    throw Components.results.NS_ERROR_NO_INTERFACE;
  },


  /* nsICommandLineHandler */

    handle : function clh_handle(cmdLine)
    {
        window = appShellService.hiddenDOMWindow;
        try
        {
            window.dump("Chromebug Command Line Handler taking arguments from state:"+cmdLine.state+"\n");
            for (var i = 0; i < cmdLine.length; i++)
                window.dump("Chromebug Command Line Handler arguments on cmdLine: "+cmdLine.length+"."+i+")"+cmdLine.getArgument(i)+"\n");

            if (cmdLine.state == cmdLine.STATE_REMOTE_AUTO) // FF is already running
            {
                var skipChrome = cmdLine.handleFlagWithParam("chromebug", false); // take ourselves out
                window.dump("Chromebug Command Line Handler removing chrome arguments from command line:"+skipChrome+"\n");
                var noProfile = cmdLine.handleFlagWithParam("p", false); // remove annoying messages about -p
            }
            else  // New chromebug that may launch FF
            {
                try
                {
                    var launchChromebug = cmdLine.handleFlag("chromebug", false);
                    if (launchChromebug)
                    {
                        getStartupObserver().startOnStartupNextTime();
                        prefs.setBoolPref("extensions.firebug.service.filterSystemURLs", false);  // See firebug-service.js
                        chromebugCommandLineHandler.openChromebug(window);
                    }
                    else
                    {
                        getStartupObserver().dontStartOnStartupNextTime();
                    }
                }
                catch (e)
                {
                    Components.utils.reportError("Failed to process command line because of: "+e);
                }
            }
        }
        catch (e)
        {
            Components.utils.reportError("Chromebug Command Line Handler FAILS: "+e);
            return;
        }
    },


  // CHANGEME: change the help info as appropriate, but
  // follow the guidelines in nsICommandLineHandler.idl
  // specifically, flag descriptions should start at
  // character 24, and lines should be wrapped at
  // 72 characters with embedded newlines,
  // and finally, the string should end with a newline
  //          01234567890123456789001234
  helpInfo :  "  -chromebug              start chromebug then continue as normal\n",

  /* nsIFactory */

  createInstance : function clh_CI(outer, iid)
  {
    if (outer != null)
      throw Components.results.NS_ERROR_NO_AGGREGATION;

    return this.QueryInterface(iid);
  },

  lockFactory : function clh_lock(lock)
  {
    /* no-op */
  }
};

function getStartupObserver()
{
    var chromebugAppStartClass =  Components.classes["@getfirebug.com/chromebug-startup-observer;1"];
    var chromebugAppStartService = chromebugAppStartClass.getService(nsISupports);
    if (chromebugAppStartService)
        return chromebugAppStartService.wrappedJSObject;
    else
        return null;
}

/**
 * The XPCOM glue that implements nsIModule
 */
const  chromebugCommandLineHandlerModule =
{
      /* nsISupports */
      QueryInterface : function mod_QI(iid)
    {
        if (iid.equals(nsIModule) ||
            iid.equals(nsISupports))
          return this;

        throw Components.results.NS_ERROR_NO_INTERFACE;
    },

  /* nsIModule */
  getClassObject : function mod_gch(compMgr, cid, iid)
  {
    if (cid.equals(clh_CID))
      return chromebugCommandLineHandler.QueryInterface(iid);

    throw Components.results.NS_ERROR_NOT_REGISTERED;
  },

  registerSelf : function mod_regself(compMgr, fileSpec, location, type)
  {
    compMgr.QueryInterface(nsIComponentRegistrar);

    compMgr.registerFactoryLocation(clh_CID,
                                    "chromebugCommandLineHandler",
                                    clh_contractID,
                                    fileSpec,
                                    location,
                                    type);

    var catMan = Components.classes["@mozilla.org/categorymanager;1"].
      getService(Ci.nsICategoryManager);
    catMan.addCategoryEntry("command-line-handler",
                            clh_category,
                            clh_contractID, true, true);
  },

  unregisterSelf : function mod_unreg(compMgr, location, type)
  {
    compMgr.QueryInterface(nsIComponentRegistrar);
    compMgr.unregisterFactoryLocation(clh_CID, location);

    var catMan = Components.classes["@mozilla.org/categorymanager;1"].
      getService(nsICategoryManager);
    catMan.deleteCategoryEntry("command-line-handler", clh_category);
  },

  canUnload : function (compMgr)
  {
    return true;
  }
};

/* The NSGetModule function is the magic entry point that XPCOM uses to find what XPCOM objects
 * this component provides
 */
function NSGetModule(comMgr, fileSpec)
{
  return chromebugCommandLineHandlerModule;
}

function getTmpFile()
{
    var file = Components.classes["@mozilla.org/file/directory_service;1"].
        getService(Components.interfaces.nsIProperties).
        get("TmpD", Components.interfaces.nsIFile);
    file.append("fbs.tmp");
    file.createUnique(Components.interfaces.nsIFile.NORMAL_FILE_TYPE, 0666);
    appShellService.hiddenDOMWindow.dump("cbcl opened tmp file "+file.path+"\n");
    return file;
}

function getTmpStream(file)
{
    // file is nsIFile, data is a string
    var foStream = Components.classes["@mozilla.org/network/file-output-stream;1"].
                             createInstance(Components.interfaces.nsIFileOutputStream);

    // use 0x02 | 0x10 to open file for appending.
    foStream.init(file, 0x02 | 0x08 | 0x20, 0666, 0);
    // write, create, truncate
    // In a c file operation, we have no need to set file mode with or operation,
    // directly using "r" or "w" usually.

    return foStream;
}

var fbs = chromebugCommandLineHandler;

function tmpout(text)
{
    if (!fbs.foStream)
        fbs.foStream = getTmpStream(getTmpFile());

    fbs.foStream.write(text, text.length);
}

