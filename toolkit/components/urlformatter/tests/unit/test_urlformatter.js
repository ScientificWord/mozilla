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
 * The Initial Developer of the Original Code is Mozilla Foundation.
 * Portions created by the Initial Developer are Copyright (C) 2007
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Ryan Flint <rflint@dslr.net> (Original Author)
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
function run_test() {
  var formatter = Cc["@mozilla.org/toolkit/URLFormatterService;1"].
                  getService(Ci.nsIURLFormatter);
  var locale = Cc["@mozilla.org/chrome/chrome-registry;1"].
               getService(Ci.nsIXULChromeRegistry).
               getSelectedLocale('global');
  var prefs = Cc['@mozilla.org/preferences-service;1'].
              getService(Ci.nsIPrefBranch);

  var upperUrlRaw = "http://%LOCALE%.%VENDOR%.foo/?name=%NAME%&id=%ID%&version=%VERSION%&platversion=%PLATFORMVERSION%&abid=%APPBUILDID%&pbid=%PLATFORMBUILDID%&app=%APP%";
  var lowerUrlRaw = "http://%locale%.%vendor%.foo/?name=%name%&id=%id%&version=%version%&platversion=%platformversion%&abid=%appbuildid%&pbid=%platformbuildid%&app=%app%";
  //XXX %APP%'s RegExp is not global, so it only replaces the first space
  var ulUrlRef = "http://" + locale + ".Mozilla.foo/?name=Url Formatter Test&id=urlformattertest@test.mozilla.org&version=1&platversion=2.0&abid=2007122405&pbid=2007122406&app=urlformatter test";
  var multiUrl = "http://%VENDOR%.%VENDOR%.%NAME%.%VENDOR%.%NAME%";
  var multiUrlRef = "http://Mozilla.Mozilla.Url Formatter Test.Mozilla.Url Formatter Test";

  var pref = "xpcshell.urlformatter.test";
  var str = Cc["@mozilla.org/supports-string;1"].
            createInstance(Ci.nsISupportsString);
  str.data = upperUrlRaw;
  prefs.setComplexValue(pref, Ci.nsISupportsString, str);

  do_check_eq(formatter.formatURL(upperUrlRaw), ulUrlRef);
  do_check_eq(formatter.formatURLPref(pref), ulUrlRef);
  // Keys must be uppercase
  do_check_neq(formatter.formatURL(lowerUrlRaw), ulUrlRef);
  do_check_eq(formatter.formatURL(multiUrl), multiUrlRef);
}
