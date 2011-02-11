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


function msiPrefs()
{
  dump("\n ** msiPrefs\n");

  var compsample = GetCurrentEngine();

  msiComputeLogger.Sent("user settings","");

  var o = new Object();

  o.mfenced      = compsample.getUserPref(compsample.use_mfenced);
  o.sig_digits   = compsample.getUserPref(compsample.Sig_digits_rendered);
  o.lower        = compsample.getUserPref(compsample.SciNote_lower_thresh);
  o.upper        = compsample.getUserPref(compsample.SciNote_upper_thresh);
  o.trigargs     = compsample.getUserPref(compsample.Parens_on_trigargs);
  o.imaginaryi   = compsample.getUserPref(compsample.Output_imaginaryi);
  o.diffD        = compsample.getUserPref(compsample.Output_diffD_uppercase);
  o.diffd        = compsample.getUserPref(compsample.Output_diffd_lowercase);
  o.expe         = compsample.getUserPref(compsample.Output_Euler_e);
  o.matrix_delim = compsample.getUserPref(compsample.Default_matrix_delims);
  o.usearc       = compsample.getUserPref(compsample.Output_InvTrigFuncs_1);
  o.mixednum     = compsample.getUserPref(compsample.Output_Mixed_Numbers);
  o.derivformat  = compsample.getUserPref(compsample.Default_derivative_format);
  o.primesasn    = compsample.getUserPref(compsample.Primes_as_n_thresh);
  o.primederiv   = compsample.getUserPref(compsample.Prime_means_derivative);

  o.loge         = compsample.getUserPref(compsample.log_is_base_e);
  o.dotderiv     = compsample.getUserPref(compsample.Dot_derivative);
  o.barconj      = compsample.getUserPref(compsample.Overbar_conjugate);
  o.i_imaginary  = compsample.getUserPref(compsample.Input_i_Imaginary);
  o.j_imaginary  = compsample.getUserPref(compsample.Input_j_Imaginary);
  o.e_exp        = compsample.getUserPref(compsample.Input_e_Euler);


  o.digits      = compsample.getEngineAttr(compsample.Digits);
  o.degree      = compsample.getEngineAttr(compsample.MaxDegree);
  o.principal   = compsample.getEngineAttr(compsample.PvalOnly) == 0 ? false : true;
  o.special     = compsample.getEngineAttr(compsample.IgnoreSCases) == 0 ? false : true;
  o.logSent     = msiComputeLogger.logMMLSent;
  o.logReceived = msiComputeLogger.logMMLReceived;
  o.engSent     = msiComputeLogger.logEngSent;
  o.engReceived = msiComputeLogger.logEngReceived;
  
  openDialog("chrome://prince/content/preferences.xul", "preferences", "chrome,titlebar,toolbar,centerscreen,modal", o);

  if (o.Cancel)
    return;

  compsample.setUserPref(compsample.use_mfenced,               o.mfenced);
  compsample.setUserPref(compsample.Sig_digits_rendered,       o.sig_digits);
  compsample.setUserPref(compsample.SciNote_lower_thresh,      o.lower);
  compsample.setUserPref(compsample.SciNote_upper_thresh,      o.upper);
  compsample.setUserPref(compsample.Parens_on_trigargs,        o.trigargs);
  compsample.setUserPref(compsample.Output_imaginaryi,         o.imaginaryi);
  compsample.setUserPref(compsample.Output_diffD_uppercase,    o.diffD);
  compsample.setUserPref(compsample.Output_diffd_lowercase,    o.diffd);
  compsample.setUserPref(compsample.Output_Euler_e,            o.expe);
  compsample.setUserPref(compsample.Default_matrix_delims,     o.matrix_delim);
  compsample.setUserPref(compsample.Output_InvTrigFuncs_1,     o.usearc);
  compsample.setUserPref(compsample.Output_Mixed_Numbers,      o.mixednum);
  compsample.setUserPref(compsample.Default_derivative_format, o.derivformat);
  compsample.setUserPref(compsample.Primes_as_n_thresh,        o.primesasn);
  compsample.setUserPref(compsample.Prime_means_derivative,    o.primederiv);

  compsample.setUserPref(compsample.log_is_base_e,             o.loge);
  compsample.setUserPref(compsample.Dot_derivative,            o.dotderiv);
  compsample.setUserPref(compsample.Overbar_conjugate,         o.barconj);
  compsample.setUserPref(compsample.Input_i_Imaginary,         o.i_imaginary);
  compsample.setUserPref(compsample.Input_j_Imaginary,         o.j_imaginary);
  compsample.setUserPref(compsample.Input_e_Euler,             o.e_exp);
  
  compsample.setEngineAttr(compsample.Digits, o.digits);
  compsample.setEngineAttr(compsample.MaxDegree, o.degree);
  compsample.setEngineAttr(compsample.PvalOnly, o.principal ? 1 : 0);
  compsample.setEngineAttr(compsample.IgnoreSCases, o.special ? 1 : 0);
  msiComputeLogger.LogMMLSent(o.logSent);
  msiComputeLogger.LogMMLReceived(o.logReceived);
  msiComputeLogger.LogEngSent(o.engSent);
  msiComputeLogger.LogEngReceived(o.engReceived);

}

// Any non-editor window wanting to create an editor with a URL
//   should use this instead of "window.openDialog..."
//  We must always find an existing window with requested URL
// (When calling from a dialog, "launchWindow" is dialog's "opener"
//   and we need a delay to let dialog close)
function msiEditPage(url, launchWindow, delay)
{
  // Always strip off "view-source:" and #anchors; kludge: accept string url or nsIURI url.
  var urlstring;
  try {
    urlstring = url.spec.replace(/^view-source:/, "").replace(/#.*/, "");
  }
  catch(e) {
    urlstring = url.replace(/^view-source:/, "").replace(/#.*/, "");
  }
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
    var uri = msiCreateURI(urlstring, null, null);

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
      launchWindow.openDialog("chrome://prince/content", "_blank", "chrome,all,dialog=no", uri.spec, charsetArg);

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
