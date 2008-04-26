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

  msiEditPage(aDocument.URL, window, false); 
}

function msiEditPageOrFrame()
{
  var focusedWindow = document.commandDispatcher.focusedWindow;

  // if the uri is a specific frame, grab it, else use the frameset uri 
  // and let Composer handle error if necessary
  var url = getContentFrameURI(focusedWindow);
  msiEditPage(url, window, false)
}

// Any non-editor window wanting to create an editor with a URL
//   should use this instead of "window.openDialog..."
//  We must always find an existing window with requested URL
// (When calling from a dialog, "launchWindow" is dialog's "opener"
//   and we need a delay to let dialog close)
function msiEditPage(url, launchWindow, delay)
{
  // Always strip off "view-source:" and #anchors
  url = url.replace(/^view-source:/, "").replace(/#.*/, "");

  // User may not have supplied a window
  if (!launchWindow)
  {
    if (window)
    {
      launchWindow = window;
    }
    else
    {
      dump("No window to launch an editor from!\n");
      return;
    }
  }

  // if the current window is a browser window, then extract the current charset menu setting from the current 
  // document and use it to initialize the new composer window...

  var wintype = document.documentElement.getAttribute('windowtype');
  var charsetArg;

  if (launchWindow && (wintype == "navigator:browser") && launchWindow.content.document)
    charsetArg = "charset=" + launchWindow.content.document.characterSet;

  try {
    var uri = msiCreateURI(url, null, null);

    var windowManager = Components.classes['@mozilla.org/appshell/window-mediator;1'].getService();
    var windowManagerInterface = windowManager.QueryInterface( Components.interfaces.nsIWindowMediator);
    var enumerator = windowManagerInterface.getEnumerator( "swp:xhtml_mathml" );
    var emptyWindow;
    var useEditorElement = null;
    while ( enumerator.hasMoreElements() )
    {
      var win = enumerator.getNext().QueryInterface(Components.interfaces.nsIDOMWindowInternal);
      if ( win && msiIsWebComposer(win))
      {
        useEditorElement = msiCheckOpenWindowForURIMatch(uri, win);
        if (useEditorElement != null)
        {
          // We found an editor with our url
          useEditorElement.focus();
          return;
        }
//        else if (!emptyWindow && msiPageIsEmptyAndUntouched(editorElement)
//        else if (!emptyWindow)
//        else if (!useEditorElement)
//        {
//          var editorElement = msiGetPrimaryEditorElementForWindow(win);
//          if (msiPageIsEmptyAndUntouched(editorElement))
//            useEditorElement = editorElement;
////            emptyWindow = win;
//        }
      }
    }

//    if (emptyWindow)
    if (useEditorElement != null)
    {
      // we have an empty editor we can use
      if (msiIsInHTMLSourceMode(useEditorElement))
        msiSetEditMode(msiGetPreviousNonSourceDisplayMode(useEditorElement), useEditorElement);
      msiEditorLoadUrl(useEditorElement, url);
      useEditorElement.focus();
      msiSetSaveAndPublishUI(url, useEditorElement);

//      if (emptyWindow.IsInHTMLSourceMode())
//        emptyWindow.SetEditMode(emptyWindow.PreviousNonSourceDisplayMode);
//      emptyWindow.EditorLoadUrl(url);
//      emptyWindow.focus();
//      emptyWindow.SetSaveAndPublishUI(url);
      return;
    }

    // Create new Composer window
    if (delay)
    {
      launchWindow.delayedOpenWindow("chrome://prince/content", "chrome,all,dialog=no", url);
    }
    else
      launchWindow.openDialog("chrome://prince/content", "_blank", "chrome,all,dialog=no", url, charsetArg);

  } catch(e) {}
}

function msiCreateURI(urlstring)
{
  try {
    var ioserv = Components.classes["@mozilla.org/network/io-service;1"]
               .getService(Components.interfaces.nsIIOService);
    return ioserv.newURI(urlstring, null, null);
  } catch (e) {}

  return null;
}

function msiCheckOpenWindowForURIMatch(uri, win)
{
  var editorList = win.document.getElementsByTagName("editor");
  for (var i = 0; i < editorList.length; ++i)
  {
    try {
      var contentDoc = editorList[i].contentDocument;
//      var contentWindow = win.content;  // need to QI win to nsIDOMWindowInternal?
//      var contentDoc = contentWindow.document;
      var htmlDoc = contentDoc.QueryInterface(Components.interfaces.nsIDOMHTMLDocument);
      var winuri = msiCreateURI(htmlDoc.URL);
      if (winuri.equals(uri))
        return editorList[i];
    } catch (e) {}
  }
  return null;
}

function msiNewEditorFromTemplate()
{
  // XXX not implemented
}

function msiNewEditorFromDraft()
{
  // XXX not implemented
}
