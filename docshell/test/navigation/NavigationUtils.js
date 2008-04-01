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
 * The Original Code is NavigationUtils.js
 *
 * The Initial Developer of the Original Code is
 * Stanford University
 * Portions created by the Initial Developer are Copyright (C) 2008
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Adam Barth <hk9565@gmail.com>
 *   Collin Jackson <mozilla@collinjackson.com>
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

///////////////////////////////////////////////////////////////////////////
//
// Utilities for navigation tests
// 
///////////////////////////////////////////////////////////////////////////

var body = "This frame was navigated.";
var target_url = "data:text/html,<html><body>" + body + "</body></html>";

///////////////////////////////////////////////////////////////////////////
// Functions that navigate frames
///////////////////////////////////////////////////////////////////////////

function navigateByLocation(wnd) {
  try {
    wnd.location = target_url;
  } catch(ex) {
    // We need to keep our finished frames count consistent.
    // Oddly, this ends up simulating the behavior of IE7.
    window.open(target_url, "_blank", "width=10,height=10");
  }
}

function navigateByOpen(name) {
  window.open(target_url, name, "width=10,height=10");
}

function navigateByForm(name) {
  var form = document.createElement("form");
  form.action = target_url;
  form.method = "POST";
  form.target = name; document.body.appendChild(form);
  form.submit();
}

var hyperlink_count = 0;

function navigateByHyperlink(name) {
  var link = document.createElement("a");
  link.href = target_url;
  link.target = name;
  link.id = "navigation_hyperlink_" + hyperlink_count++;
  document.body.appendChild(link);
  sendMouseEvent({type:"click"}, link.id);
}

///////////////////////////////////////////////////////////////////////////
// Functions that call into Mochitest framework
///////////////////////////////////////////////////////////////////////////

function isNavigated(wnd, message) {
  var result = null;
  try {
    result = wnd.document.body.innerHTML;
  } catch(ex) {
    result = ex;
  }
  is(result, body, message);
}

function isBlank(wnd, message) {
  var result = null;
  try {
    result = wnd.document.body.innerHTML;
  } catch(ex) {
    result = ex;
  }
  is(result, "This is a blank document.", message);
}

function isAccessible(wnd, message) {
  try {
    wnd.document.body.innerHTML;
    ok(true, message);
  } catch(ex) {
    ok(false, message);
  }
}

function isInaccessible(wnd, message) {
  try {
    wnd.document.body.innerHTML;
    ok(false, message);
  } catch(ex) {
    ok(true, message);
  }
}

///////////////////////////////////////////////////////////////////////////
// Functions that require UniversalXPConnect privilege
///////////////////////////////////////////////////////////////////////////

function xpcEnumerateContentWindows(callback) {
  netscape.security.PrivilegeManager.enablePrivilege("UniversalXPConnect");
  var ww = Components.classes["@mozilla.org/embedcomp/window-watcher;1"]
                     .getService(Components.interfaces.nsIWindowWatcher);
  var enumerator = ww.getWindowEnumerator();

  var contentWindows = [];

  while (enumerator.hasMoreElements()) {
    var win = enumerator.getNext();
    if (typeof ChromeWindow != "undefined" && win instanceof ChromeWindow) {
      if (win.gBrowser) {
        var tabs = win.gBrowser.browsers;
        for (var i = 0; i < tabs.length; i++)
          contentWindows.push(tabs[i].docShell.document.defaultView);
      }
    } else {
      contentWindows.push(win);
    }
  }

  while (contentWindows.length > 0)
    callback(contentWindows.pop());
}

// Note: This only searches for top-level frames with this name.
function xpcGetFramesByName(name) {
  var results = [];

  xpcEnumerateContentWindows(function(win) {
    if (win.name == name)
      results.push(win);
  });

  return results;
}

function xpcCleanupWindows() {
  xpcEnumerateContentWindows(function(win) {
    if (win.location.protocol == "data:")
      win.close();
  });
}

function xpcWaitForFinishedFrames(callback, numFrames) {
  var finishedFrameCount = 0;
  function frameFinished() {
    finishedFrameCount++;

    if (finishedFrameCount == numFrames) {
      clearInterval(frameWaitInterval);
      setTimeout(callback, 1);
      return;
    }

    if (finishedFrameCount > numFrames)
      throw "Too many frames loaded.";
  }

  var finishedWindows = [];

  function contains(obj, arr) {
    for (var i = 0; i < arr.length; i++) {
      if (obj === arr[i])
        return true;
    }
    return false;
  }

  function searchForFinishedFrames(win) {
    if (escape(unescape(win.location)) == escape(target_url)) {
      if (!contains(win, finishedWindows)) {
        finishedWindows.push(win);
        frameFinished();
      }
    }
    for (var i = 0; i < win.frames.length; i++)
      searchForFinishedFrames(win.frames[i]);
  }

  function poll() {
    try {
      // This only gives us UniversalXPConnect for the current stack frame
      // We're using setInterval, so the main page's privileges are still normal
      xpcEnumerateContentWindows(searchForFinishedFrames);
    } catch(ex) {
      // We might be accessing windows before they are fully constructed,
      // which can throw.  We'll find those frames on our next poll().
    }
  }

  var frameWaitInterval = setInterval(poll, 500);
}

