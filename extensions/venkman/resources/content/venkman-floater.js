/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
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
 * The Original Code is The JavaScript Debugger.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Robert Ginda, <rginda@netscape.com>, original author
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

var dd       = opener.dd;
var console  = opener.console;
var getMsg   = opener.getMsg;
var dispatch = console.dispatch;
var windowId;
var containedViews = new Object();

function onLoad()
{
    var ary = document.location.search.match(/(?:\?|&)id=([^&]+)/);
    if (!ary)
    {
        dd ("No window id in url " + document.location);
        return;
    }
    
    windowId = ary[1];
    
    if (console.prefs["menubarInFloaters"])
        console.createMainMenu (window.document);

    if ("arguments" in window && 0 in window.arguments &&
        typeof window.arguments[0] == "function")
    {
        setTimeout(window.arguments[0], 500, window);
    }
}

function onClose()
{
    window.isClosing = true;
    return true;
}

function onUnload()
{
    console.viewManager.destroyWindow (windowId);
}

function onViewRemoved(view)
{
    delete containedViews[view.viewId];
    updateTitle();
}

function onViewAdded(view)
{
    containedViews[view.viewId] = view;
    updateTitle();
}

function updateTitle()
{
    var ary = new Array();
    
    for (v in containedViews)
        ary.push(containedViews[v].caption);

    var views = ary.join(opener.MSG_COMMASP);
    if (views == "")
        views = opener.MSG_VAL_NONE;
    
    document.title = getMsg(opener.MSN_FLOATER_TITLE, views);
}


