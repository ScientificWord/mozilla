const Cc = Components.classes;
const Ci = Components.interfaces;
const Cr = Components.results;

Components.utils.import("resource://gre/modules/XPCOMUtils.jsm");

function signCommandLineHandler() {
}

signCommandLineHandler.prototype = {
  handle: function(cmdLine)
  {
     try
        {
           var appShellService = Components.classes["@mozilla.org/appshell/appShellService;1"].
                getService(Components.interfaces.nsIAppShellService);
            window = appShellService.hiddenDOMWindow;
            window.dump("signOnCommandLine"+this.classDescription+"\n");
            var filename = cmdLine.handleFlagWithParam("sign", false);
            if (filename)
            {
                window.signOnTheLineFileName = filename;
                window.dump("Signing filename: "+filename+"\n");
            }
            var key = cmdLine.handleFlagWithParam("key", false);
            if (key)
            {
                window.signOnTheLineKey = key;
                window.dump("Signing with key named:"+key+"\n");
            }
            var addOnFileName = cmdLine.handleFlagWithParam("addOnFileName", false);
            if (addOnFileName)
            {
                window.signOnTheLineAddOnFileName = addOnFileName;
                window.dump("Hashing xpi file "+addOnFileName+"\n");
            }

        }
        catch (e)
        {
            Components.utils.reportError("SignOnTheLine Command Line Handler FAILS: "+e);
            return;
        }
  },

  helpInfo: "",

  classDescription: "McCoy Sign On The Command Line Handler",
  contractID: "@mozilla.org/mccoy/mccoy-sign-clh;1",
  classID: Components.ID("{6e2cec70-f075-11dc-95ff-0800200c9a66}"),
  QueryInterface: XPCOMUtils.generateQI([Ci.nsICommandLineHandler]),
  _xpcom_categories: [{ category: "command-line-handler", entry: "m-mccoy-sign" }]
};

function NSGetModule(compMgr, fileSpec)
{
  return XPCOMUtils.generateModule([signCommandLineHandler]);
}

/* See license.txt for terms of usage */
/* John J. Barton johnjbarton@johnjbarton.com March 2007 */