/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998-1999
 * the Initial Developer. All Rights Reserved.
 *
 * Copyright 2006 MacKichan Software, Inc.
 * ***** END LICENSE BLOCK ***** */

/* Implementations of nsIControllerCommand for composer commands */
#include productname.inc

function msiInitEditorContextMenuItems(aEvent)
{
  var shouldShowEditPage = !gContextMenu.onImage && !gContextMenu.onLink && !gContextMenu.onTextInput && !gContextMenu.inDirList;
  gContextMenu.showItem( "context-editpage", shouldShowEditPage );

  var shouldShowEditLink = gContextMenu.onSaveableLink; 
  gContextMenu.showItem( "context-editlink", shouldShowEditLink );

  // Hide the applications separator if there's no add-on apps present. 
  gContextMenu.showItem("context-sep-apps", gContextMenu.shouldShowSeparator("context-sep-apps"));
}
  
function msiInitEditorContextMenuListener(aEvent)
{
  var popup = document.getElementById("contentAreaContextMenu");
  if (popup)
    popup.addEventListener("popupshowing", msiInitEditorContextMenuItems, false);
}

addEventListener("load", msiInitEditorContextMenuListener, false);

function msiEditDocument(aDocument)      
{
  if (!aDocument)
    aDocument = window.content.document;

  msiEditPage(aDocument.URL, window, false, false); 
}

function msiEditPageOrFrame()
{
  var focusedWindow = document.commandDispatcher.focusedWindow;

  // if the uri is a specific frame, grab it, else use the frameset uri 
  // and let Composer handle error if necessary
  var url = getContentFrameURI(focusedWindow);
  msiEditPage(url, window, false, false);
}


function msiPrefs()
{
#ifndef PROD_SW
  var compsample = GetCurrentEngine();
  msiComputeLogger.Sent("user settings","");
#endif
  var o = {chromeDoc: document};
  openDialog("chrome://prince/content/preferences.xul", "preferences", "chrome,titlebar,resizable,toolbar,centerscreen,dialog='yes'", o);
}

function msiNewEditorFromTemplate()
{
  // XXX not implemented
}

function msiNewEditorFromDraft()
{
  // XXX not implemented
}
