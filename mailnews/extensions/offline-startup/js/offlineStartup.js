/* -*- Mode: Java; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * ***** BEGIN LICENSE BLOCK *****
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
 * The Original Code is the Offline Startup Handler
 *
 * The Initial Developer of the Original Code is
 * David Bienvenu.
 * Portions created by the Initial Developer are Copyright (C) 2004
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *  David Bienvenu <bienvenu@nventure.com> (Original Author)
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
const kDebug               = false;
const kBundleURI         = "chrome://messenger/locale/offlineStartup.properties";
const kOfflineStartupPref = "offline.startup_state";
var gStartingUp = true;
var gOfflineStartupMode; //0 = remember last state, 1 = ask me, 2 == online, 3 == offline, 4 = automatic
const kRememberLastState = 0;
const kAskForOnlineState = 1;
const kAlwaysOnline = 2;
const kAlwaysOffline = 3;
////////////////////////////////////////////////////////////////////////
//
//   nsOfflineStartup : nsIObserver
//
//   Check if the user has set the pref to be prompted for
//   online/offline startup mode. If so, prompt the user. Also,
//   check if the user wants to remember their offline state
//   the next time they start up.
//
////////////////////////////////////////////////////////////////////////

var nsOfflineStartup =
{
  onProfileStartup: function()
  {
    debug("onProfileStartup");

    var ioService = Components.classes["@mozilla.org/network/io-service;1"]
                              .getService(Components.interfaces.nsIIOService2);
    if (gStartingUp)
    {
      gStartingUp = false;
      // if checked, the "work offline" checkbox overrides
      if (ioService.offline && !ioService.manageOfflineStatus) {
        debug("already offline!");
        return;
      }
    }

    var prefs = Components.classes["@mozilla.org/preferences-service;1"]
                          .getService(Components.interfaces.nsIPrefBranch);
    gOfflineStartupMode = prefs.getIntPref(kOfflineStartupPref);

    if (gOfflineStartupMode == kAlwaysOffline)
    {
      ioService.manageOfflineStatus = false;
      ioService.offline = true;
    }
    else if (gOfflineStartupMode == kAlwaysOnline)
    {
      ioService.manageOfflineStatus = false;
      ioService.offline = false;
    }
    else if (gOfflineStartupMode == kRememberLastState)
    {
      ioService.manageOfflineStatus = false;
      ioService.offline = !prefs.getBoolPref("network.online");
    }
    else if (gOfflineStartupMode == kAskForOnlineState)
    {
      var promptService = Components.
        classes["@mozilla.org/embedcomp/prompt-service;1"].
        getService(Components.interfaces.nsIPromptService);

      var bundle = getBundle(kBundleURI);
      if (!bundle)
        return;

      var title = bundle.GetStringFromName("title");
      var desc = bundle.GetStringFromName("desc");
      var button0Text = bundle.GetStringFromName("workOnline");
      var button1Text = bundle.GetStringFromName("workOffline");
      var checkVal = {value:0};

      var result = promptService.confirmEx(null, title, desc,
        (promptService.BUTTON_POS_0 * promptService.BUTTON_TITLE_IS_STRING) +
        (promptService.BUTTON_POS_1 * promptService.BUTTON_TITLE_IS_STRING),
        button0Text, button1Text, null, null, checkVal);
      debug ("result = " + result + "\n");
      ioService.manageOfflineStatus = false;
      ioService.offline = result == 1;
    }
  },

  observe: function(aSubject, aTopic, aData)
  {
    debug("observe: " + aTopic);

    if (aTopic == "profile-change-net-teardown")
    {
      debug("remembering offline state");
      var prefs = Components.classes["@mozilla.org/preferences-service;1"]
                            .getService(Components.interfaces.nsIPrefBranch);
      var ioService = Components.classes["@mozilla.org/network/io-service;1"]
                                .getService(Components.interfaces.nsIIOService);
      prefs.setBoolPref("network.online", !ioService.offline);
    }
    else if (aTopic == "app-startup")
    {
      var observerService = Components.classes["@mozilla.org/observer-service;1"]
                                      .getService(Components.interfaces.nsIObserverService);
      observerService.addObserver(this, "profile-after-change", false);
      observerService.addObserver(this, "profile-change-net-teardown", false);
    }
    else if (aTopic == "profile-after-change")
    {
      this.onProfileStartup();
    }
  },


  QueryInterface: function(aIID)
  {
    if (aIID.equals(Components.interfaces.nsIObserver) ||
        aIID.equals(Components.interfaces.nsISupports))
      return this;

    Components.returnCode = Components.results.NS_ERROR_NO_INTERFACE;
    return null;
  }
}


var nsOfflineStartupModule =
{
  mClassName:     "Offline Startup",
  mContractID:    "@mozilla.org/offline-startup;1",
  mClassID:       Components.ID("3028a3c8-2165-42a4-b878-398da5d32736"),

  getClassObject: function(aCompMgr, aCID, aIID)
  {
    if (!aCID.equals(this.mClassID))
      throw Components.results.NS_ERROR_NO_INTERFACE;
    if (!aIID.equals(Components.interfaces.nsIFactory))
      throw Components.results.NS_ERROR_NOT_IMPLEMENTED;

    return this.mFactory;
  },

  registerSelf: function(aCompMgr, aFileSpec, aLocation, aType)
  {
    debug("*** Registering nsOfflineStartupModule (a JavaScript Module)\n");

    aCompMgr = aCompMgr.QueryInterface(
                 Components.interfaces.nsIComponentRegistrar);
    aCompMgr.registerFactoryLocation(this.mClassID, this.mClassName,
      this.mContractID, aFileSpec, aLocation, aType);

    // Receive startup notification.
    // We are |getService()|d at app-startup (before profile selection)
    this.getCategoryManager().addCategoryEntry("app-startup",
      "Offline-startup", "service," + this.mContractID, true, true);
  },

  unregisterSelf: function(aCompMgr, aFileSpec, aLocation)
  {
    aCompMgr = aCompMgr.QueryInterface(
                 Components.interfaces.nsIComponentRegistrar);
    aCompMgr.unregisterFactoryLocation(this.mClassID, aFileSpec);

    this.getCategoryManager().deleteCategoryEntry("app-startup",
      "Offline-startup", true);
  },

  canUnload: function(aCompMgr)
  {
    return true;
  },

  getCategoryManager: function()
  {
    return Components.classes["@mozilla.org/categorymanager;1"].
      getService(Components.interfaces.nsICategoryManager);
  },

  //////////////////////////////////////////////////////////////////////
  //
  //   mFactory : nsIFactory
  //
  //////////////////////////////////////////////////////////////////////
  mFactory:
  {
    createInstance: function(aOuter, aIID)
    {
      if (aOuter != null)
        throw Components.results.NS_ERROR_NO_AGGREGATION;

      // return the singleton 
      return nsOfflineStartup.QueryInterface(aIID);
    },

    lockFactory: function(aLock)
    {
      // quiten warnings
    }
  }
};

function NSGetModule(aCompMgr, aFileSpec)
{
  return nsOfflineStartupModule;
}

function getBundle(aURI)
{
  if (!aURI)
    return null;

  debug(aURI);
  var bundle = null;
  try
  {
    var strBundleService = Components.
      classes["@mozilla.org/intl/stringbundle;1"].
      getService(Components.interfaces.nsIStringBundleService);
    bundle = strBundleService.createBundle(aURI);
  }
  catch (ex)
  {
    bundle = null;
    debug("Exception getting bundle " + aURI + ": " + ex);
  }

  return bundle;
}


////////////////////////////////////////////////////////////////////////
//
//   Debug helper
//
////////////////////////////////////////////////////////////////////////
if (!kDebug)
  debug = function(m) {};
else
  debug = function(m) {dump("\t *** nsOfflineStartup: " + m + "\n");};
