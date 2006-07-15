/* -*- Mode: Java; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Ben Goodger <ben@netscape.com> (Original Author)
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either of the GNU General Public License Version 2 or later (the "GPL"),
 * or the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
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


////////////////////////////////////////////////////////////////////////////////
// Get the two bookmarks utility libraries running, attach controllers, focus
// tree widget, etc. 
function Startup() 
{
  // correct keybinding command attributes which don't do our business yet
  var key = document.getElementById("key_delete");
  if (key.getAttribute("command"))
    key.setAttribute("command", "cmd_bm_delete");
  key = document.getElementById("key_delete2");
  if (key.getAttribute("command"))
    key.setAttribute("command", "cmd_bm_delete");

  var bookmarksView = document.getElementById("bookmarks-view");  
  bookmarksView.tree.view.selection.select(0);
}

function manageBookmarks() {
  openDialog("chrome://communicator/content/bookmarks/bookmarksManager.xul", "", "chrome,dialog=no,resizable=yes");
}

function addBookmark() {
  var contentArea = top.document.getElementById('content');                   
  if (contentArea) {
    const browsers = contentArea.browsers;
    if (browsers.length > 1)
      BookmarksUtils.addBookmarkForTabBrowser(contentArea);
    else
      BookmarksUtils.addBookmarkForBrowser(contentArea.webNavigation, true);    
  }
  else
    BookmarksUtils.addBookmark(null, null, undefined, true);
}
