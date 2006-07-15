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
 * The Original Code is Mozilla Communicator client code, released
 * March 31, 1998.
 *
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation. Portions created by Netscape are
 * Copyright (C) 1998-1999 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s):   Ben Goodger     (ben@netscape.com)
 *                   Joe Hewitt      (hewitt@netscape.com)
 *                   Blake Ross      (blaker@netscape.com)
 *                   Stephen Walker  (walk84@usa.net)
 *                   J.Betak         (jbetak@netscape.com)
 *
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


function SelectListItem(listRef, itemValue)
{

  try {

    var selectedItem;

    if (itemValue) {
      var elements = listRef.getElementsByAttribute("value", itemValue);
      selectedItem = elements.item(0);
    }

    if (selectedItem)
    {
      listRef.selectedItem = selectedItem;
      return true;
    }
    else
      return false;
  } 

  catch(e)
  {
    return false;
  }

}


function Startup()
{
  var defaultLanguage;
  var languageList = document.getElementById("langList");
  var selectedLanguage = window.arguments.length ? window.arguments[0] : null;

  //get pref defaults
  try
  {
    const nsIPrefLocalizedString = Components.interfaces.nsIPrefLocalizedString;
    var prefBranch = Components.classes["@mozilla.org/preferences-service;1"]
                               .getService(Components.interfaces.nsIPrefService)
                               .getBranch("general.useragent.");
    defaultLanguage = prefBranch.getComplexValue("locale", nsIPrefLocalizedString).data;
  }

  catch(e) 
  {}

  //persist previous user selection, highlight a default otherwise
  if (!SelectListItem(languageList, selectedLanguage))
    if (!SelectListItem(languageList, defaultLanguage))
      languageList.selectedIndex = 0;

}


function onAccept()
{
  //cache language on the parent window
  var languageList = document.getElementById("langList");
  var selectedItem = languageList.selectedItems.length ? languageList.selectedItems[0] : null;

  if (selectedItem) {
    var langName = selectedItem.getAttribute("value");
    var langStore = opener.document.getElementById("profileLanguage");

    if (langStore)
      langStore.setAttribute("data", langName);
  }

  return true;
}
