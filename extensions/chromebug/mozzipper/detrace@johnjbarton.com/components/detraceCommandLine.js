const Cc = Components.classes;
const Ci = Components.interfaces;
const Cr = Components.results;

Components.utils.import("resource://gre/modules/XPCOMUtils.jsm");

function detraceCommandLineHandler() {
}

detraceCommandLineHandler.prototype = {
  handle: function(cmdLine)
  {
     try
        {
           var appShellService = Components.classes["@mozilla.org/appshell/appShellService;1"].
                getService(Components.interfaces.nsIAppShellService);
            window = appShellService.hiddenDOMWindow;
            window.dump("detraceCommandLine"+this.classDescription+"\n");
            var detrace = cmdLine.handleFlag("detrace", false);
            if (detrace)
            {
                window.detrace = true;
                window.dump("Detrace: "+window.detrace+"\n");
            }
        }
        catch (e)
        {
            Components.utils.reportError("Detrace Command Line Handler FAILS: "+e);
            return;
        }
  },

  helpInfo: "",

  classDescription: "McCoy Detrace Line Handler",
  contractID: "@mozilla.org/mccoy/mccoy-detrace-clh;1",
  classID: Components.ID("{7af7d560-25df-11dd-bd0b-0800200c9a66}"),
  QueryInterface: XPCOMUtils.generateQI([Ci.nsICommandLineHandler]),
  _xpcom_categories: [{ category: "command-line-handler", entry: "m-mccoy-detrace" }]
};

function NSGetModule(compMgr, fileSpec)
{
  return XPCOMUtils.generateModule([detraceCommandLineHandler]);
}

/* See license.txt for terms of usage */
/* John J. Barton johnjbarton@johnjbarton.com March 2007 */