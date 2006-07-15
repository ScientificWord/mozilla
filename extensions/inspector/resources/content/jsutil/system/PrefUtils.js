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
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2001
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Joe Hewitt <hewitt@netscape.com> (original author)
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

/***************************************************************
* PrefUtils -------------------------------------------------
*  Utility for easily using the Mozilla preferences system.
* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
* REQUIRED IMPORTS:
****************************************************************/

//////////// global variables /////////////////////

//////////// global constants ////////////////////

const nsIPrefBranch = Components.interfaces.nsIPrefBranch;

////////////////////////////////////////////////////////////////////////////
//// class PrefUtils

var PrefUtils = 
{
  mPrefs: null,
  
  init: function()
  {
    var prefService = XPCU.getService("@mozilla.org/preferences-service;1", "nsIPrefService");
    this.mPrefs = prefService.getBranch(null);
  },

  addObserver: function(aDomain, aFunction)
  {
    if (!this.mPrefs) this.init();

    var pbi = XPCU.QI(this.mPrefs, "nsIPrefBranchInternal");
    if (pbi)
      pbi.addObserver(aDomain, aFunction, false);
  },

  removeObserver: function(aDomain, aFunction)
  {
    if (!this.mPrefs) this.init();

    var pbi = XPCU.QI(this.mPrefs, "nsIPrefBranchInternal");
    if (pbi)
      pbi.removeObserver(aDomain, aFunction);
  },

  setPref: function(aName, aValue)
  {
    if (!this.mPrefs) this.init();
    
    var type = this.mPrefs.getPrefType(aName);
    try {
      if (type == nsIPrefBranch.PREF_STRING) {
        var str = Components.classes["@mozilla.org/supports-string;1"]
                            .createInstance(Components.interfaces.nsISupportsString);
        str.data = aValue;
        this.mPrefs.setComplexValue(aName, Components.interfaces.nsISupportsString, str);
      } else if (type == nsIPrefBranch.PREF_BOOL) {
        this.mPrefs.setBoolPref(aName, aValue);
      } else if (type == nsIPrefBranch.PREF_INT) {
        this.mPrefs.setIntPref(aName, aValue);
      }
    } catch(ex) {
      debug("ERROR: Unable to write pref \"" + aName + "\".\n");
    }
  },

  getPref: function(aName)
  {
    if (!this.mPrefs) this.init();

    var type = this.mPrefs.getPrefType(aName);
    try {
      if (type == nsIPrefBranch.PREF_STRING) {
        return this.mPrefs.getComplexValue(aName, Components.interfaces.nsISupportsString).data;
      } else if (type == nsIPrefBranch.PREF_BOOL) {
        return this.mPrefs.getBoolPref(aName);
      } else if (type == nsIPrefBranch.PREF_INT) {
        return this.mPrefs.getIntPref(aName);
      }
    } catch(ex) {
      debug("ERROR: Unable to read pref \"" + aName + "\".\n");
    }
    return null;
  }
  
};

