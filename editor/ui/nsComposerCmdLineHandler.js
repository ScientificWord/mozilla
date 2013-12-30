/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Mozilla Seamonkey Composer.
 *
 * The Initial Developer of the Original Code is
 * Benjamin Smedberg <bsmedberg@covad.net>.
 * Portions created by the Initial Developer are Copyright (C) 2004
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
 "use strict";

const nsICmdLineHandler     = Components.interfaces.nsICmdLineHandler;
const nsICommandLineHandler = Components.interfaces.nsICommandLineHandler;
const nsIFactory            = Components.interfaces.nsIFactory;
const nsISupports           = Components.interfaces.nsISupports;
const nsIModule             = Components.interfaces.nsIModule;
const nsIComponentRegistrar = Components.interfaces.nsIComponentRegistrar;
const nsICategoryManager    = Components.interfaces.nsICategoryManager;
const nsISupportsString     = Components.interfaces.nsISupportsString;
const nsIWindowWatcher      = Components.interfaces.nsIWindowWatcher;

const NS_ERROR_FAILURE        = Components.results.NS_ERROR_FAILURE;
const NS_ERROR_NO_AGGREGATION = Components.results.NS_ERROR_NO_AGGREGATION;
const NS_ERROR_NO_INTERFACE   = Components.results.NS_ERROR_NO_INTERFACE;

function nsComposerCmdLineHandler() {}
nsComposerCmdLineHandler.prototype = {
  get wrappedJSObject() {
    return this;
  },

  /* nsISupports */

  QueryInterface: function(iid) {
    if (iid.equals(nsISupports))
      return this;

    if (nsICmdLineHandler && iid.equals(nsICmdLineHandler))
      return this;

    if (nsICommandLineHandler && iid.equals(nsICommandLineHandler))
      return this;

    throw NS_ERROR_NO_INTERFACE;
  },

  /* nsICmdLineHandler */
  commandLineArgument : "-edit",
  prefNameForStartup : "general.startup.editor",
  chromeUrlForTask : "chrome://prince/content/prince.xul",
  helpText : "Edit document with SW/SWP/SNB.",
  handlesArgs : true,
  defaultArgs : "",
  openWindowWithArgs : true,

  /* nsICommandLineHandler */
  handle : function (cmdLine) {
    //dump("clh: Prince command line handler\n");
    var args = Components.classes["@mozilla.org/supports-string;1"]
                         .createInstance(nsISupportsString);
    var features = "chrome,all,dialog=no";
    try {
      var width = cmdLine.handleFlagWithParam("width", false);
      if (width != null)
        features += ",width=" + width;
    } catch (e) {
    }
    try {
      var height = cmdLine.handleFlagWithParam("height", false);
      if (height != null)
        features += ",height=" + height;
    } catch (e) {
    }
//        dump("clh: Features is '"+features+"'\n");
    try {
      var uristr = cmdLine.handleFlagWithParam("edit", false);
      if (uristr == null) {
        // Try the editor flag (used for general.startup.* prefs)
        uristr = cmdLine.handleFlagWithParam("editor", false);
      }

      if ((uristr==null) && !cmdLine.preventDefault &&cmdLine.length > 0) {
        uristr = cmdLine.getArgument(0);
        if (uristr && uristr.length > 0){
          if (!(/^-/).test(uristr)) {
            try {
              args.data = cmdLine.resolveURI(uristr).spec;
            }
            catch (e) {
            }
          }
        }
        else args.data = "";
      }
    }
    catch(e) {
    }
    var wwatch = Components.classes["@mozilla.org/embedcomp/window-watcher;1"]
                               .getService(nsIWindowWatcher);
    dump("+++ Opening prince window with args = '" + args.data + "' and features = '" + features + "'\n");
    wwatch.openWindow(null, "chrome://prince/content/prince.xul", "_blank",
                      features, args);
    cmdLine.preventDefault = true;
  },

  helpInfo : "  -edit <path>          Open document for editing.\n" +
             "  <path>                Open document for editing.\n" +
             "  -width <integer>      Set width of window in pixels.\n"+
             "  -height <integer>     Set height of window in pixels.\n"

};

function nsComposerCmdLineHandlerFactory() {
}

nsComposerCmdLineHandlerFactory.prototype = {
  /* nsISupports */

  QueryInterface: function(iid) {
    if (!iid.equals(nsIFactory) &&
        !iid.equals(nsISupports)) {
          throw Components.results.NS_ERROR_NO_INTERFACE;
    }
    return this;
  },

  /* nsIFactory */
  createInstance: function(outer, iid) {
    if (outer != null) {
      throw NS_ERROR_NO_AGGREGATION;
    }

    return new nsComposerCmdLineHandler().QueryInterface(iid);
  },

  lockFactory: function(lock) {
  }
};

const nsComposerCmdLineHandler_CID =
  Components.ID("{f7d8db95-ab5d-4393-a796-9112fe758cfa}");

const ContractIDPrefix =
  "@mozilla.org/commandlinehandler/general-startup;1?type=";

var thisModule = {
  /* nsISupports */

  QueryInterface: function(iid) {
    if (!iid.equals(nsIModule) &&
        !iid.equals(nsISupports)) {
          throw Components.results.NS_ERROR_NO_INTERFACE;
    }
    return this;
  },

  /* nsIModule */

  getClassObject: function (compMgr, cid, iid) {
    if (!cid.equals(nsComposerCmdLineHandler_CID)) {
      throw NS_ERROR_FAILURE;
    }

    if (!iid.equals(nsIFactory)) {
      throw NS_ERROR_NO_INTERFACE;
    }

    return new nsComposerCmdLineHandlerFactory();
  },

  registerSelf: function (compMgr, fileSpec, location, type) {
    var compReg = compMgr.QueryInterface(nsIComponentRegistrar);
    compReg.registerFactoryLocation(nsComposerCmdLineHandler_CID,
                                    "nsComposerCmdLineHandler",
                                    ContractIDPrefix + "edit",
                                    fileSpec, location, type);
    compReg.registerFactoryLocation(nsComposerCmdLineHandler_CID,
                                    "nsComposerCmdLineHandler",
                                    ContractIDPrefix + "editor",
                                    fileSpec, location, type);

    var catMan = Components.classes["@mozilla.org/categorymanager;1"].getService(nsICategoryManager);
    catMan.addCategoryEntry("command-line-argument-handlers",
                            "nsComposerCmdLineHandler",
                            ContractIDPrefix + "edit",
                            true, true);
    catMan.addCategoryEntry("command-line-handler",
                            "m-edit",
                            ContractIDPrefix + "edit",
                            true, true);
  },

  unregisterSelf: function (compMgr, location, type) {
    var compReg = compMgr.QueryInterface(nsIComponentRegistrar);
    compReg.unregisterFactoryLocation(nsComposerCmdLineHandler_CID,
                                      location);

    var catMan = Components.classes["@mozilla.org/categorymanager;1"].getService(nsICategoryManager);
    catMan.deleteCategoryEntry("command-line-argument-handlers",
                               "nsComposerCmdLineHandler", true);
    catMan.deleteCategoryEntry("command-line-handler",
                               "m-edit", true);
  },

  canUnload: function (compMgr) {
    return true;
  }
};

function NSGetModule(compMgr, fileSpec) {
  return thisModule;
}
