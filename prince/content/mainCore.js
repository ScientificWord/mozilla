// Copyright (c) 2004 MacKichan Software, Inc.  All Rights Reserved.

function CustomizeMainToolbar(id1, id2, id3, id4, id5 )
{
  // make sure our toolbar buttons have the correct enabled state restored to them...
  if (this.UpdateMainToolbar == undefined)
      dump("UPDATEMAINTOOLBAR undefined\n");

  // Disable the toolbar context menu items
  // var menubar = document.getElementById("main-menubar");
  // for (var i = 0; i < menubar.childNodes.length; ++i)
  //   menubar.childNodes[i].setAttribute("disabled", true);
    
  var customizePopup = document.getElementById("CustomizeMainToolbar");
  if (customizePopup)
    customizePopup.setAttribute("disabled", "true");
  dump("Should call CustomizeToolbar"); 
  window.openDialog("chrome://prince/content/customizeToolbar.xul", "CustomizeToolbar",
                    "chrome,all,dependent", document.getElementById(id1), document.getElementById(id2),
                    document.getElementById(id3), document.getElementById(id4), document.getElementById(id5));
}

function MainToolboxCustomizeDone(aToolboxChanged)
{
  // Update global UI elements that may have been added or removed

  // Re-enable parts of the UI we disabled during the dialog
  var menubar = document.getElementById("main-menubar");
  for (var i = 0; i < menubar.childNodes.length; ++i)
    menubar.childNodes[i].setAttribute("disabled", false);

  var customizePopup = document.getElementById("CustomizeMainToolbar");
  if (customizePopup)
    customizePopup.removeAttribute("disabled");

  // make sure our toolbar buttons have the correct enabled state restored to them...
  if (this.UpdateMainToolbar != undefined)
    UpdateMainToolbar(focus); 
}

function onViewToolbarCommand(aToolbarId, aMenuItemId)
{
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

function UpdateMainToolbar(caller)
{
    dump("===> UPDATING TOOLBAR\n");
}
