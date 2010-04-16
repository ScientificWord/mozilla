const Cc = Components.classes;
const Ci = Components.interfaces;
const Cr = Components.results;

Components.utils.import("resource://gre/modules/XPCOMUtils.jsm");

function CommandLineHandler() {
}

CommandLineHandler.prototype = {
  handle: function(cmdLine)
  {
     try
        {
           var appShellService = Components.classes["@mozilla.org/appshell/appShellService;1"].
                getService(Components.interfaces.nsIAppShellService);
            window = appShellService.hiddenDOMWindow;
            window.dump("mozzipperCommandLine\n");
            var source = cmdLine.handleFlagWithParam("source", false);
            if (source)
            {
                window.zipperSourceDirectory = source;
                window.dump("Extension source: "+window.zipperSourceDirectory+"\n");
            }
            var addOnFileName = cmdLine.handleFlagWithParam("target", false);
            if (addOnFileName)
            {
                window.zipperXPIFileName = addOnFileName;
                window.dump("target XPI filename "+window.zipperXPIFileName+"\n");
            }

        }
        catch (e)
        {
            Components.utils.reportError("Zipper Command Line Handler FAILS: "+e);
            return;
        }
  },

  helpInfo: "",

  classDescription: "McCoy Zipper",
  contractID: "@mozilla.org/mccoy/mccoy-zipper-clh;1",
  classID: Components.ID("{92f21782-b57f-4fb0-9f87-dfcc1d220911}"),
  QueryInterface: XPCOMUtils.generateQI([Ci.nsICommandLineHandler]),
  _xpcom_categories: [{ category: "command-line-handler", entry: "m-mccoy-zipper" }]
};

function NSGetModule(compMgr, fileSpec)
{
  return XPCOMUtils.generateModule([CommandLineHandler]);
}
/* See license.txt for terms of usage */
/* John J. Barton johnjbarton@johnjbarton.com March 2007 */