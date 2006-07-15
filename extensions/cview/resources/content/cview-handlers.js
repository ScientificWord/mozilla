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
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Robert Ginda, rginda@netscape.com, original author
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

/*
 * This file contains event handlers for cview.  The main code of cview is
 * divided between this file and cview-static.js
 */

function onLoad()
{
    initTrees();
    cview.totalComponents = cview.visibleComponents = 
        cview.componentView.rowCount;
    cview.totalInterfaces = cview.visibleInterfaces = 
        cview.interfaceView.rowCount;
    refreshLabels();
}

function onUnload()
{
    /* nothing important to do onUnload yet */
}

function onComponentClick(e)
{
    if (e.originalTarget.localName == "treecol")
    {
        onTreeResort(e, cview.componentView);
    }
    else
    {
        /*
        cview.lastContractID = e.target.parentNode.getAttribute("contractid");
    
        if (cview.interfaceMode == "implemented-by")
        {
            cview.interfaceFilter = cview.lastContractID;
            filterInterfaces();
        }
        */
    }
}

function onInterfaceClick(e)
{
    if (e.originalTarget.localName == "treecol")
    {
        onTreeResort(e, cview.interfaceView);
    }
    else
    {
        /*
        cview.lastIID = e.target.parentNode.getAttribute("iid");
        refreshLabels();
        */
    }
}

function onTreeResort (e, view)
{
    /* resort by column */
    var rowIndex = new Object();
    var col = new Object();
    var childElt = new Object();
    
    var obo = view.tree;
    obo.getCellAt(e.clientX, e.clientY, rowIndex, col, childElt);
    if (rowIndex.value == -1)
      return;

    var prop;
    switch (col.value.id.substr(4))
    {
        case "name":
            prop = "sortName";
            break;
        case "number":
            prop = "sortNumber";
            break;
    }
    
    var root = view.childData;
    var dir = (prop == root._share.sortColumn) ?
        root._share.sortDirection * -1 : 1;
    root.setSortColumn (prop, dir);
}

function onComponentSelect (e)
{
    var index = cview.componentView.selection.currentIndex;
    var row = cview.componentView.childData.locateChildByVisualRow (index);
    if (!row)
        return;

    var text = document.getElementById ("output-text");
    text.value = row.getText();
}

function onInterfaceSelect (e)
{
    var index = cview.interfaceView.selection.currentIndex;
    var row = cview.interfaceView.childData.locateChildByVisualRow (index);
    if (!row)
        return;

    var text = document.getElementById ("output-text");
    text.value = row.getText();
}
            
function onLXRIFCLookup (e, type)
{
    var index = cview.interfaceView.selection.currentIndex;
    var row = cview.interfaceView.childData.locateChildByVisualRow (index);
    if (!row)
        return;
    
    /* Specifing "_content" as the second parameter to window.open places
     * the url in the most recently open browser window
     */
    window.open ("http://lxr.mozilla.org/mozilla/" + type + row.name,
                 "_content", "");
}

function onChangeDisplayMode (e)
{
    var id = e.target.getAttribute("id");
    var ary = id.match (/menu-(cmp|ifc)-show-(.*)/);
    if (!ary)
    {
        dd ("** UNKNOWN ID '" + id + "' in onChangeDisplayMode **");
        return;
    }
    
    var typeDesc;
        var el;
    if (ary[1] == "cmp") {
        typeDesc = "Components";
        el = document.getElementById("menu-cmp-show-all");
        el.setAttribute("checked", false);
        el = document.getElementById("menu-cmp-show-contains");
        el.setAttribute("checked", false);
        el = document.getElementById("menu-cmp-show-starts-with");
        el.setAttribute("checked", false);
    } else {
        typeDesc = "Interfaces";
        el = document.getElementById("menu-ifc-show-all");
        el.setAttribute("checked", false);
        el = document.getElementById("menu-ifc-show-contains");
        el.setAttribute("checked", false);
        el = document.getElementById("menu-ifc-show-starts-with");
        el.setAttribute("checked", false);
        el = document.getElementById("menu-ifc-show-implemented-by");
        el.setAttribute("checked", false);
    }
    
    e.target.setAttribute ("checked", true);
    var filter = "";

    if (ary[2] == "contains")
    {
        filter = window.prompt ("Show only " + typeDesc + " containing...");
        if (!filter)
            return;
        filter = new RegExp(filter, "i");    
    }
    
    if (ary[2] == "starts-with")
    {
        filter = window.prompt ("Show only " + typeDesc + " starting with...");
        if (!filter)
            return;
        filter = new RegExp(filter, "i");    
    }

    if (ary[2] == "implemented-by")
        filter = cview.lastContractID;

    if (ary[1] == "cmp")
    {
        cview.componentMode = ary[2];
        cview.componentFilter = filter;
        filterComponents();
    }
    else
    {
        cview.interfaceMode = ary[2];
        cview.interfaceFilter = filter;
        filterInterfaces();
    }
    
}
