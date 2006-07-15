/* -*- Mode: Java; tab-width: 4; c-basic-offset: 4; -*-
 * 
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
 * The Original Code is mozilla.org Code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Neil Rashbrook <neil@parkwaycc.co.uk>
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

function openCookieViewer(viewerType)
{
  const wm = Components.classes["@mozilla.org/appshell/window-mediator;1"]
                       .getService(Components.interfaces.nsIWindowMediator);
  var enumerator = wm.getEnumerator("mozilla:cookieviewer");
  while (enumerator.hasMoreElements()) {
    var viewer = enumerator.getNext();
    if (viewer.arguments[0] == viewerType) {
      viewer.focus();
      return;
    }
  }
  window.openDialog("chrome://communicator/content/permissions/cookieViewer.xul",
                    "_blank", "chrome,resizable", viewerType);
}

function showPermissionsManager(viewerType, host) {
  var wm = Components.classes["@mozilla.org/appshell/window-mediator;1"]
                     .getService(Components.interfaces.nsIWindowMediator);
  var existingWindow = wm.getMostRecentWindow("permissions-" + viewerType);
  if (existingWindow) {
    existingWindow.setHost(host);
    existingWindow.focus();
  }
  else {
    var params = { blockVisible: (viewerType == "image"),
                   sessionVisible: false,
                   allowVisible: true,
                   prefilledHost: host,
                   permissionType: viewerType };
    window.openDialog("chrome://communicator/content/permissions/permissionsManager.xul", "_blank",
                      "chrome,resizable=yes", params);
  }
}

function viewPopups(host) {
  showPermissionsManager("popup", host);
}

function viewImages() {
  showPermissionsManager("image", "");
}

function viewInstalls() {
  showPermissionsManager("install", "");
}

function viewCookies() {
  openCookieViewer("cookieManager");
}  

function viewCookiesFromIcon() {
  openCookieViewer("cookieManagerFromIcon");
}  

function viewP3P() {
  window.openDialog
    ("chrome://communicator/content/permissions/cookieP3P.xul","_blank","chrome,resizable=no");
}  
