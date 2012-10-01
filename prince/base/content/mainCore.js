// Copyright (c) 2004 MacKichan Software, Inc.  All Rights Reserved.
"use strict";

function CustomizeMainToolbar(id1, id2, id3, id4, id5) {
  // make sure our toolbar buttons have the correct enabled state restored to them...
  if (this.UpdateMainToolbar == null) dump("UPDATEMAINTOOLBAR undefined\n");

  // Disable the toolbar context menu items
  // var menubar = document.getElementById("main-menubar");
  // for (var i = 0; i < menubar.childNodes.length; ++i)
  //   menubar.childNodes[i].setAttribute("disabled", true);
  var customizePopup = document.getElementById("CustomizeMainToolbar");
  if (customizePopup) customizePopup.setAttribute("disabled", "true");
  dump("Should call CustomizeToolbar");
  window.openDialog("chrome://global/content/customizeToolbar.xul", "CustomizeToolbar", "chrome,all,dependent,alwaysRaised", document.getElementById(id1), document.getElementById(id2), document.getElementById(id3), document.getElementById(id4), document.getElementById(id5));
}


function createContextMenuItem(id, label, target, collapsed) {
  var item = document.createElementNS("http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul", "menuitem");

  item.id = "contextmenuitem_" + id;
  item.setAttribute("label", label);
  item.setAttribute("value", target);
  item.setAttribute("type", "checkbox");
  item.setAttribute("checked", !collapsed);
  item.setAttribute("oncommand", "changeCollapsedState('" + target + "');");
  return item;
}

function toggleToolbarItem(id, label, target, collapsed) {
  var item = document.createElementNS("http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul", "menuitem");

  item.id = "contextmenuitem_" + id;
  item.setAttribute("label", label);
  item.setAttribute("value", target);
  item.setAttribute("type", "checkbox");
  item.setAttribute("checked", !collapsed);
  item.setAttribute("oncommand", "changeCollapsedState('" + item.id + "');");
  return item;
}

function changeCollapsedState(target) {
  var tb = document.getElementById(target);
  if (tb.getAttribute("collapsed") === "true") tb.setAttribute("collapsed", "false");
  else tb.setAttribute("collapsed", "true");
}

function initCustomizeMenu(popup, id) {
  var toolbox = document.getElementById(id);
  var toolbars = toolbox.getElementsByTagName("toolbar");
  var customizeitem = null;
  var menuitems;
  var i;
  var item;
  var tb;
  var name;
  var mi;
  // first remove existing toolbar items.
  menuitems = popup.getElementsByTagName("menuitem");
  while (menuitems.length > 0) {
    mi = menuitems[0];
    if (mi.id == "customizeItem") {
      customizeitem = mi;
      break;
    }
    if (mi.id == "customizeItem") alert("Deleting customizeitem!");
    mi.parentNode.removeChild(mi);
    menuitems = popup.getElementsByTagName("menuitem");
  }
  for (i = 0; i < toolbars.length; i++) {
    tb = toolbars[i];
    if (tb.hasAttribute("toolbarname")) {
      name = tb.getAttribute("toolbarname");
      popup.insertBefore(createContextMenuItem(i, name, tb.id, tb.collapsed), customizeitem);
    }
  }
}



function MainToolboxCustomizeDone(aToolboxChanged) {
  // Update global UI elements that may have been added or removed
  // Re-enable parts of the UI we disabled during the dialog
  var menubar = document.getElementById("main-menubar");
  for (var i = 0; i < menubar.childNodes.length; ++i)
  menubar.childNodes[i].setAttribute("disabled", false);

  var customizePopup = document.getElementById("CustomizeMainToolbar");
  if (customizePopup) customizePopup.removeAttribute("disabled");

  // make sure our toolbar buttons have the correct enabled state restored to them...
  if (this.UpdateMainToolbar != null) UpdateMainToolbar(focus);
}

function onViewToolbarCommand(aToolbarId, aMenuItemId) {
  var toolbar = document.getElementById(aToolbarId);
  var menuItem = document.getElementById(aMenuItemId);

  if (!toolbar || !menuItem) return;

  var toolbarCollapsed = toolbar.collapsed;

  // toggle the checkbox
  menuItem.setAttribute('checked', toolbarCollapsed);

  // toggle visibility of the toolbar
  toolbar.collapsed = !toolbarCollapsed;

  document.persist(aToolbarId, 'collapsed');
  document.persist(aMenuItemId, 'checked');
}

function UpdateMainToolbar(caller) {
  dump("===> UPDATING TOOLBAR\n");
}
