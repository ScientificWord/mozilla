// Copyright (c) 2006 MacKichan Software, Inc.  All Rights Reserved.

//var gComposerWindowControllerID = 0;
//var prefAuthorString = "";
//                                        
//const kDisplayModeNormal = 0;
//const kDisplayModeAllTags = 1;
//const kDisplayModeSource = 2;
//const kDisplayModePreview = 3;
//const kDisplayModeMenuIDs = ["viewNormalMode", "viewAllTagsMode", "viewSourceMode", "viewPreviewMode"];
//const kDisplayModeTabIDS = ["NormalModeButton", "TagModeButton", "SourceModeButton", "PreviewModeButton"];
//const kNormalStyleSheet = "chrome://editor/content/EditorContent.css";
//const kAllTagsStyleSheet = "chrome://editor/content/EditorAllTags.css";
//const kParagraphMarksStyleSheet = "chrome://editor/content/EditorParagraphMarks.css";
//
//const kTextMimeType = "text/plain";
//const kHTMLMimeType = "text/html";
//
//const nsIWebNavigation = Components.interfaces.nsIWebNavigation;
////
////var gPreviousNonSourceDisplayMode = 1;
//var gEditorDisplayMode = -1;
////var gDocWasModified = false;  // Check if clean document, if clean then unload when user "Opens"
//
////var gContentWindow = 0;
////var gSourceContentWindow = 0;
////var gSourceTextEditor = null;
//var gContentWindowDeck;
//var gTagSelectBar;
//var gComputeToolbar;
//var gViewFormatToolbar;

function aColorObj(editorElement) 
{
  this.mEditorElement = editorElement;
  this.LastTextColor = "";
  this.LastBackgroundColor = "";
  this.LastHighlightColor = "";
  this.Type = "";
  this.SelectedType = "";
  this.NoDefault = false;
  this.Cancel = false;
  this.HighlightColor = "";
  this.BackgroundColor = "";
  this.PageColor = "";
  this.TextColor = "";
  this.TableColor = "";
  this.CellColor = "";
};

//var gDefaultTextColor = "";
//var gDefaultBackgroundColor = "";
//
////var gCSSPrefListener;
//var gPrefs;
////var gLocalFonts = null;
//
////var gLastFocusNode = null;
////var gLastFocusNodeWasSelected = false;
//
////// These must be kept in synch with the XUL <options> lists
////var gFontSizeNames = ["xx-small","x-small","small","medium","large","x-large","xx-large"];
////
//////MSI stuff, see msiColorObj::Format() for more comprehensive value.
//var gMathStyleSheet = "data:text/css,math { color: #FF0000; }";
////
////const nsIFilePicker = Components.interfaces.nsIFilePicker;
////
//const kEditorToolbarPrefs = "editor.toolbars.showbutton.";

//function msiShowHideToolbarSeparators(toolbar) {
//  dump("===> msiShowHideToolbarSeparators\n");
//
//  var childNodes = toolbar.childNodes;
//  var separator = null;
//  var hideSeparator = true;
//  for (var i = 0; childNodes[i].localName != "spacer"; i++) {
//    if (childNodes[i].localName == "toolbarseparator") {
//      if (separator)
//        separator.hidden = true;
//      separator = childNodes[i];
//    } else if (!childNodes[i].hidden) {
//      if (separator)
//        separator.hidden = hideSeparator;
//      separator = null;
//      hideSeparator = false;
//    }
//  }
//}
//
//function msiShowHideToolbarButtons()
//{
//  dump("===> msiShowHideToolbarButtons\n");
//
//  var array = GetPrefs().getChildList(kEditorToolbarPrefs, {});
//  for (var i in array) {
//    var prefName = array[i];
//    var id = prefName.substr(kEditorToolbarPrefs.length) + "Button";
//    var button = document.getElementById(id);
//    if (button)
//      button.hidden = !gPrefs.getBoolPref(prefName);
//  }
//  msiShowHideToolbarSeparators(document.getElementById("EditToolbar"));
//  msiShowHideToolbarSeparators(document.getElementById("FormatToolbar"));
//}
  
function msiAddToolbarPrefListener(editorElement)
{
  try {
    var pbi = GetPrefs().QueryInterface(Components.interfaces.nsIPrefBranch2);
    pbi.addObserver(kEditorToolbarPrefs, editorElement.mEditorToolbarPrefListener, false);
  } catch(ex) {
    dump("Failed to observe prefs: " + ex + "\n");
  }
}

function msiRemoveToolbarPrefListener(editorElement)
{
  try {
    var pbi = GetPrefs().QueryInterface(Components.interfaces.nsIPrefBranch2);
    pbi.removeObserver(kEditorToolbarPrefs, editorElement.mEditorToolbarPrefListener);
  } catch(ex) {
    dump("Failed to remove pref observer: " + ex + "\n");
  }
}

//// Pref listener constants
//const gEditorToolbarPrefListener =
function msiEditorToolbarPrefListener(editorElement) 
{
  this.mEditorElement = editorElement;
  this.observe = function(subject, topic, prefName)
  {
    // verify that we're changing a button pref
    if (topic != "nsPref:changed")
      return;

    var id = prefName.substr(kEditorToolbarPrefs.length) + "Button";
    var button = document.getElementById(id);
    if (button) {
      button.hidden = !gPrefs.getBoolPref(prefName);
      msiShowHideToolbarSeparators(button.parentNode);
    }
  };
};

function msiButtonPrefListener(editorElement)
{
  this.mEditorElement = editorElement;
//}

//// implements nsIObserver
//nsButtonPrefListener.prototype =
//{
  this.domain = "editor.use_css";
  this.startup = function()
  {
    try {
      var pbi = GetPrefs().QueryInterface(Components.interfaces.nsIPrefBranch2);
      pbi.addObserver(this.domain, this, false);
    } catch(ex) {
      dump("Failed to observe prefs (msiButtonPrefListener): " + ex + "\n");
    }
  };
  this.shutdown = function()
  {
    try {
      var pbi = GetPrefs().QueryInterface(Components.interfaces.nsIPrefBranch2);
      pbi.removeObserver(this.domain, this);
    } catch(ex) {
      dump("Failed to remove pref observers: " + ex + "\n");
    }
  };
  this.observe = function(subject, topic, prefName)
  {
    if (!msiIsHTMLEditor(mEditorElement))
      return;
    // verify that we're changing a button pref
    if (topic != "nsPref:changed") return;
    if (prefName.substr(0, this.domain.length) != this.domain) return;

    var cmd = document.getElementById("cmd_highlight");
    if (cmd) {
      var prefs = GetPrefs();
      var useCSS = prefs.getBoolPref(prefName);
      var editor = msiGetEditor(mEditorElement);
      if (useCSS && editor) {
        var mixedObj = {};
        var state = editor.getHighlightColorState(mixedObj);
        cmd.setAttribute("state", state);
        cmd.collapsed = false;
      }      
      else {
        cmd.setAttribute("state", "transparent");
        cmd.collapsed = true;
      }

      if (editor)
        editor.isCSSEnabled = useCSS;
    }
  };
  this.startup();
}

//function msiMainWindowMouseDownListener(event)
//{
//  var target = event.explicitOriginalTarget;
//  if (!target)
//    target = event.originalTarget;
////  if (!target)
////    return;
//
//  var changeActiveEditor = true;
//  var nodeStr = "";
//  if (target)
//  {
//    switch(target.nodeName)
//    {
//      case "menu":
//      case "menulist":
//      case "menuitem":
//      case "menupopup":
//      case "toolbar":
//      case "toolbarbutton":
//      case "toolbarpalette":
//      case "toolbaritem":
//        changeActiveEditor = false;
//      break;
//    }
//    nodeStr = target.nodeName;
//  }
//  var logStr = "In msiMainWindowMouseDownListener, target.nodeType is [" + nodeStr + "]\n";
//  msiKludgeLogString(logStr);
////  window.bIgnoreNextFocus = !changeActiveEditor;
//  if (!changeActiveEditor)
//  {
//    msiResetActiveEditorElement(this);
//  }
//}

//TO DO - find out whether this ever happens (appears not to be called from anywhere)
//function AfterHighlightColorChange()
//{
//  if (!IsHTMLEditor())
//    return;
//
//  var button = document.getElementById("cmd_highlight");
//  if (button) {
//    var mixedObj = {};
//    try {
//      var state = GetCurrentEditor().getHighlightColorState(mixedObj);
//      button.setAttribute("state", state);
//      onHighlightColorChange();
//    } catch (e) {}
//  }      
//}

function msiEditorArrayInitializer()
{
  this.mInfoList = new Array();
  this.addEditorInfo = function(anEditorElement, anInitialText, bIsMultiPara)
  {
    if (this.findEditorInfo(anEditorElement) < 0)
    {
      var newInfo = new Object();
      newInfo.mEditorElement = anEditorElement;
      newInfo.mInitialText = anInitialText;
      if (bIsMultiPara == null)
        newInfo.mbWithContainingHTML = false;
      else
        newInfo.mbWithContainingHTML = bIsMultiPara;
      newInfo.bDone = false;
      this.mInfoList.push(newInfo);
      anEditorElement.mEditorSeqInitializer = this;
    }
  };
  this.doInitialize = function()
  {
    this.initializeNextEditor(0);
  };
  this.finishedEditor = function(anEditorElement)
  {
    var editorIndex = this.findEditorInfo(anEditorElement);
    if (editorIndex >= 0)
    {
      this.mInfoList[editorIndex].bDone = true;
    }
    anEditorElement.mEditorSeqInitializer = null;
    ++editorIndex;
//    dump( "In msiEditorArrayInitializer.finishedEditor for editor [" + anEditorElement.id + "]; moving on to editor number" + editorIndex + "].\n" );
    this.initializeNextEditor(editorIndex);
  };
  this.findEditorInfo = function(anEditorElement)
  {
    if (anEditorElement == null)
      return -1;
    for (var ix = 0; ix < this.mInfoList.length; ++ix)
    {
      if (this.mInfoList[ix].mEditorElement == anEditorElement)
      {
        return ix;
      }
    }
    dump("In msiEditorArrayInitializer, unable to find editorElement " + anEditorElement.id + "!\n");
    return -1;
  };
  this.initializeNextEditor = function(editorIndex)
  {
    var nFound = -1;
    if (editorIndex < 0)
      editorIndex = 0;
    for (var ix = editorIndex; (nFound < 0) && (ix < this.mInfoList.length); ++ix)
    {
      if (!this.mInfoList[ix].bDone)
      {
        nFound = ix;
        break;
      }
    }
    if (nFound >= 0)
    {
      var theEditorElement = this.mInfoList[nFound].mEditorElement;
//      msiDumpWithID("In msiEditorArrayInitializer.initializeNextEditor, about to initialize editor [@].", theEditorElement);
//      theEditorElement.mEditorSeqInitializer = this;  should we set this only for one at a time, or when we add the item? Try the latter for now.
      msiInitializeEditorForElement(theEditorElement, this.mInfoList[nFound].mInitialText, this.mInfoList[nFound].mbWithContainingHTML);
    }
//    else
//      dump("In msiEditorArrayInitializer.initializeNextEditor, found no next editor to initialize.\n");
  };
}


function msiInitializeEditorForElement(editorElement, initialText, bWithContainingHTML)
{
//  // See if argument was passed.
//  if ( window.arguments && window.arguments[0] )
//  {
//      // Opened via window.openDialog with URL as argument.
//      // Put argument where EditorStartup expects it.
//    document.getElementById( "args" ).setAttribute( "value", window.arguments[0] );
//  }

//  // get default character set if provided
//  if ("arguments" in window && window.arguments.length > 1 && window.arguments[1])
//  {
//    if (window.arguments[1].indexOf("charset=") != -1)
//    {
//      var arrayArgComponents = window.arguments[1].split("=");
//      if (arrayArgComponents)
//      {
//        // Put argument where EditorStartup expects it.
//        document.getElementById( "args" ).setAttribute("charset", arrayArgComponents[1]);
//      }
//    }
//  }

//  window.tryToClose = msiEditorCanClose;
//  window.addEventListener("mousedown", msiMainWindowMouseDownListener, true);

  // Continue with normal startup.
//  var editorElement = document.getElementById("content-frame");
//  msiDumpWithID("Entering msiInitializeEditorForElement for element [@].\n", editorElement);
  var startText;
//  if ( ((initialText != null) && (initialText.length > 0)) || bWithContainingHTML )
  if ( (initialText != null) && (initialText.length > 0) )
  {
//    if (bWithContainingHTML && ((initialText==null) || !initialText.length))
//    {
//      if (editorElement.mbInitialContentsMultiPara)
//        startText = "<body></body>";
////        startText = "<body>" + initialText + "</body>";
//      else
//        startText = "<para></para>";
////        startText = "<para>" + initialText + "</para>";
////      startText = "<html><head></head><BODY><para>" + initialText + "</para></BODY></html>";
//    }
//    else
      startText = initialText;
    editorElement.initialEditorContents = startText;
  }
  EditorStartupForEditorElement(editorElement);

  // Initialize our source text <editor>
  try {
//    editorElement.mSourceContentWindow = document.getElementById("content-source");
//    editorElement.mSourceContentWindow.makeEditable("text", false);
//    editorElement.mSourceTextEditor = editorElement.mSourceContentWindow.getEditor(editorElement.mSourceContentWindow.contentWindow);
//    editorElement.mSourceTextEditor.QueryInterface(Components.interfaces.nsIPlaintextEditor);
//    editorElement.mSourceTextEditor.enableUndo(false);
//    editorElement.mSourceTextEditor.rootElement.style.fontFamily = "-moz-fixed";
//    editorElement.mSourceTextEditor.rootElement.style.whiteSpace = "pre";
//    editorElement.mSourceTextEditor.rootElement.style.margin = 0;
//    //ljh I don't understand why a new instance of the controller is being created?????
//    var controller = Components.classes["@mozilla.org/embedcomp/base-command-controller;1"]
//                               .createInstance(Components.interfaces.nsIControllerContext);
//    controller.init(null);
//    controller.setCommandContext(editorElement.mSourceContentWindow);
//    editorElement.mSourceContentWindow.contentWindow.controllers.insertControllerAt(0, controller);
//    var commandTable = controller.QueryInterface(Components.interfaces.nsIInterfaceRequestor)
//                                 .getInterface(Components.interfaces.nsIControllerCommandTable);
    var commandTable = msiGetComposerCommandTable(editorElement);
    commandTable.registerCommand("cmd_find",        msiFindCommand);
    commandTable.registerCommand("cmd_findNext",    msiFindAgainCommand);
    commandTable.registerCommand("cmd_findPrev",    msiFindAgainCommand);
    
    msiSetupMSIMathMenuCommands(editorElement);
    msiSetupMSIComputeMenuCommands(editorElement);
    if ("msiSetupMSITypesetMenuCommands" in window)
    {
      msiSetupMSITypesetMenuCommands(editorElement);
      msiSetupMSITypesetInsertMenuCommands(editorElement);
    }
  } catch (e) { msiDumpWithID("msiInitializeEditorForElement [@] failed: "+e+"\n", editorElement); }
}


//const gSourceTextListener =
function msiSourceTextListener(editorElement)
{
  this.mEditorElement = editorElement;
  this.NotifyDocumentCreated = function NotifyDocumentCreated() {msiDumpWithID("In msiSourceTextListener for editorElement [@], NotifyDocumentCreated.\n", editorElement);};
  this.NotifyDocumentWillBeDestroyed = function NotifyDocumentWillBeDestroyed() {};
  this.NotifyDocumentStateChanged = function NotifyDocumentStateChanged(isChanged)
  {
    window.updateCommands("save");
  };
};

//const gSourceTextObserver =
function msiSourceTextObserver(editorElement)
{
  this.mEditorElement = editorElement;
  this.observe = function observe(aSubject, aTopic, aData)
  {
    // we currently only use this to update undo
    window.updateCommands("undo");
  };
};
//
//function TextEditorOnLoad()
//{
//    // See if argument was passed.
//    if ( window.arguments && window.arguments[0] ) {
//        // Opened via window.openDialog with URL as argument.
//        // Put argument where EditorStartup expects it.
//        document.getElementById( "args" ).setAttribute( "value", window.arguments[0] );
//    }
//    // Continue with normal startup.
//    EditorStartup();
//}

// This should be called by all editor users when they close their window
//  or other similar "done with editor" actions, like recycling a Mail Composer window.
//function EditorCleanup()
//{
////  SwitchInsertCharToAnotherEditorOrClose();
//}

//var DocumentReloadListener =
function aDocumentReloadListener(editorElement)
{
  this.mEditorElement  = editorElement;
  this.NotifyDocumentCreated = function() {};
  this.NotifyDocumentWillBeDestroyed = function() {};

  this.NotifyDocumentStateChanged = function( isNowDirty )
  {
    var editor = msiGetEditor(editorElement);
    try {
      // unregister the listener to prevent multiple callbacks
      editor.removeDocumentStateListener( this );

      var charset = editor.documentCharacterSet;

      // update the META charset with the current presentation charset
      editor.documentCharacterSet = charset;

    } catch (e) {}
  };
};

//function addEditorClickEventListener()
//{
//  try {
//    var bodyelement = GetBodyElement();
//    if (bodyelement)
//      bodyelement.addEventListener("click", EditorClick, false);
//  } catch (e) {}
//}


function msiGetBodyElement(editorElement)
{
  try
  {
    var editor = msiGetEditor(editorElement);
    return editor.rootElement;
  }
  catch (ex)
  {
    dump("no body tag found?!\n");
    //  better have one, how can we blow things up here?
  }
  return null;
}

function addClickEventListenerForEditor(editorElement)
{
  try {
    var bodyelement = msiGetBodyElement(editorElement);
    if (bodyelement)
      bodyelement.addEventListener("click", EditorClick, false);
  } catch (e) {dump("Unable to register click event listener; error [" + e + "].\n");}
}


function addFocusEventListenerForEditor(editorElement)
{
//  if (msiIsTopLevelEditor(editorElement))
//  {
//  }
//  else
//  {
    try
    {
      editorElement.contentWindow.addEventListener("focus", msiEditorOnFocus, true);
      editorElement.contentWindow.addEventListener("blur", msiEditorOnBlur, true);
    }
    catch(exc)
    {
      var errMsg = "Problem registering focus event listeners for editor element [";
      if (editorElement)
        errMsg += editorElement.id;
      errMsg += "]; error is";
      errMsg += exc;
      AlertWithTitle("Error", errMsg);
    }
//  }
}

function msiEditorOnFocus(event)
{
//  var editorElement = GetEditorElementForDocument(event.currentTarget.ownerDocument);
  var editorElement = msiGetEditorElementFromEvent(event);
//Logging stuff only
  var logStr = "msiEditorOnFocus called, editorElement [";
  if (editorElement)
    logStr += editorElement.id;
  else
  {
    logStr += "], current event target is [";
    if (event.currentTarget)
      logStr += event.currentTarget.nodeName;
    logStr += "], explicit original target is [";
    if (event.explicitOriginalTarget)
      logStr += event.explicitOriginalTarget.nodeName;
    if ("document" in event.explicitOriginalTarget)
    {
      logStr += "], explicit original target frame element is [";
      if (event.explicitOriginalTarget.frameElement)
        logStr += event.explicitOriginalTarget.frameElement.nodeName; 
    }
    else if ("defaultView" in event.explicitOriginalTarget)
    {
      logStr += "], explicit original target default view frame element is [";
      if (event.explicitOriginalTarget.defaultView.frameElement)
        logStr += event.explicitOriginalTarget.defaultView.frameElement.nodeName; 
    }
  }
  logStr += "].\n";
  msiKludgeLogString(logStr);
//End logging
  msiSetActiveEditor(editorElement, true);
}

function msiEditorOnBlur(event)
{
  var editorElement = msiGetEditorElementFromEvent(event);
//Logging stuff only
  var logStr = "msiEditorOnBlur called, editorElement [";
  if (editorElement)
    logStr += editorElement.id;
  logStr += "]";
//End logging
  if (editorElement == msiGetActiveEditorElement())
  {
    logStr += ", calling msiSetActiveEditor to null.\n";
    msiKludgeLogString(logStr);
    msiSetActiveEditor(null, true);
  }
//Logging stuff only
  else
  {
    logStr += ", doing nothing (not the current active editor).\n";
    msiKludgeLogString(logStr);
  }
//End logging
}

//Stolen from GetCurrentEditorElement() and hacked.
function ShutdownAllEditors()
{
  var tmpWindow = window;
  var keepgoing = true;
  
//  do {
    // Get the <editor> element(s)
    var editorList = document.getElementsByTagName("editor");
    for (var i = 0; i < editorList.length; ++i)
    {
      if (editorList.item(i))
      {
        // the next two lines assign the OR of the two return values, but we can't use || since we
        // want the side effects of both function calls
        keepgoing = (!msiIsTopLevelEditor(editorList.item(i)));
        if (msiCheckAndSaveDocument(editorList.item(i), "cmd_close", true)) keepgoing = true;
        if (keepgoing)
          ShutdownAnEditor(editorList.item(i));
        else break;
      }
    }

//    tmpWindow = tmpWindow.opener;
//  } 
//  while (tmpWindow);

  return !keepgoing;
}

// implements nsIObserver
//var gEditorDocumentObserver =
function msiEditorDocumentObserver(editorElement)
{
  this.mEditorElement = editorElement;
//  this.mDumpMessage = 3;
  this.mbInsertInitialContents = true;
  this.mbAddOverrideStyleSheets = true;
  this.doInitFastCursor = function() 
  {
    if (this.mEditorElement && ("fastCursorInit" in this.mEditorElement) && this.mEditorElement.fastCursorInit!==null)
    {
      this.mEditorElement.fastCursorInit();
      this.mEditorElement.fastCursorInit = null;
    }
  }
  this.observe = function(aSubject, aTopic, aData)
  {              
    // Should we allow this even if NOT the focused editor?
    if (!this.mEditorElement.docShell)
    {
      msiDumpWithID("In documentCreated observer for editor [@], observing [" + aTopic + "]; returning, as editor has no doc shell!\m", this.mEditorElement);
      return;
    }
//    msiDumpWithID("In documentCreated observer for editor [@], observing [" + aTopic + "]; editor's boxObject is [" + this.mEditorElement.boxObject + "]\n", this.mEditorElement);
    var commandManager = msiGetCommandManager(this.mEditorElement);
    if (commandManager != aSubject)
    {
      msiDumpWithID("In documentCreated observer for editor [@], observing [" + aTopic + "]; returning, as commandManager doesn't equal aSubject; aSubject is [" + aSubject + "], while commandManager is [" + commandManager + "].\n", this.mEditorElement);
//      if (commandManager != null)
        return;
    }
    var editor = null;
    try
    { editor = msiGetEditor(this.mEditorElement); }
    catch(exc) 
    { msiDumpWithID("In documentCreated observer for editorElement [@]; error trying to get editor: " + exc + "\n", this.mEditorElement);
      editor = null;
    }

    var edStr = "";
    if (editor != null)
      edStr = "non-null";
//    if (this.mDumpMessage > 0)
//      msiDumpWithID("In documentCreated observer for editor [@]; aTopic is [" + aTopic + "], aData is [" + aData + "]; msiGetEditor returned [" + edStr + "] before switch.\n", editorElement);

    switch(aTopic)
    {
      case "obs_documentCreated":
        // Just for convenience
//        gContentWindow = window.content;

        // Get state to see if document creation succeeded
        var params = newCommandParams();
        if (!params)
          return;
        try {
          commandManager.getCommandState(aTopic, this.mEditorElement.contentWindow, params);
          var errorStringId = 0;
          var editorStatus = params.getLongValue("state_data");
          if (!editor && editorStatus == nsIEditingSession.eEditorOK)
          {
            dump("\n ****** NO EDITOR BUT NO EDITOR ERROR REPORTED ******* \n\n");
            editorStatus = nsIEditingSession.eEditorErrorUnknown;
          }

          switch (editorStatus)
          {
            case nsIEditingSession.eEditorErrorCantEditFramesets:
              errorStringId = "CantEditFramesetMsg";
              break;
            case nsIEditingSession.eEditorErrorCantEditMimeType:
              errorStringId = "CantEditMimeTypeMsg";
              break;
            case nsIEditingSession.eEditorErrorUnknown:
              errorStringId = "CantEditDocumentMsg";
              break;
            // Note that for "eEditorErrorFileNotFound, 
            // network code popped up an alert dialog, so we don't need to
          }
          if (errorStringId)
            AlertWithTitle("", GetString(errorStringId));
        } catch(e) { dump("EXCEPTION GETTING obs_documentCreated state "+e+"\n"); }

        // We have a bad editor -- nsIEditingSession will rebuild an editor
        //   with a blank page, so simply abort here
        if (editorStatus)
          return; 

        editorElement.softsavetimer = new SS_Timer(2*60*1000, editor, editorElement);
        if (!("InsertCharWindow" in window))
          window.InsertCharWindow = null;

        if (msiIsHTMLEditor(this.mEditorElement))
        {
          editor.addTagInfo("resource:///res/tagdefs/latexdefs.xml");
          try {
            editorElement.mgMathStyleSheet = msiColorObj.FormatStyleSheet(editorElement);
//            dump("Internal style sheet contents: \n\n" + editorElement.mgMathStyleSheet + "\n\n");
          } catch(e) { dump("Error formatting style sheet using msiColorObj: [" + e + "]\n"); }
          // Now is a good time to initialize the key mapping. This is a service, and so is initialized only one. Later
          // initializations will not do anything
          var keymapper;
          try {
            keymapper =  Components.classes["@mackichan.com/keymap/keymap_service;1"]
                                 .createInstance(Components.interfaces.msiIKeyMap);
            keymapper.loadKeyMapFile();
            keymapper.saveKeyMaps();
          }
          catch(e) { dump("Failed to load keytables.xml -- "+e); }
        }

        var is_topLevel = msiIsTopLevelEditor(this.mEditorElement);
        if (!is_topLevel)
        {
          var parentEditorElement = msiGetParentOrTopLevelEditor(this.mEditorElement);
          if (parentEditorElement != null)
            msiFinishInitDialogEditor(this.mEditorElement, parentEditorElement);
          else
            msiDumpWithID("No parent editor element for editorElement [@] found in documentCreated observer!", this.mEditorElement);
        }
        var isInlineSpellCheckerEnabled = gPrefs.getBoolPref("swp.spellchecker.enablerealtimespell");
        editor.setSpellcheckUserOverride(isInlineSpellCheckerEnabled);
        // set up a listener in case the preference changes at runtime.

        var bIsRealDocument = false;
        var currentURL = msiGetEditorURL(this.mEditorElement);
        msiDumpWithID("For editor element [@], currentURL is " + currentURL + "].\n", this.mEditorElement);
        if (currentURL != null)
        {
          var fileName = GetFilename(currentURL);
          bIsRealDocument = (fileName != null && fileName.length > 0);
        }

        try {
          editor.QueryInterface(nsIEditorStyleSheets);

          //  and extra styles for showing anchors, table borders, smileys, etc
          editor.addOverrideStyleSheet(kNormalStyleSheet);
          if (bIsRealDocument && this.mbAddOverrideStyleSheets && this.mEditorElement.overrideStyleSheets && this.mEditorElement.overrideStyleSheets != null)
          {
            for (var ix = 0; ix < this.mEditorElement.overrideStyleSheets.length; ++ix)
            {
              msiDumpWithID("Adding override style sheet [" + this.mEditorElement.overrideStyleSheets[ix] + "] for editor [@].\n", this.mEditorElement);
//              dump("Adding override style sheet [" + editorElement.overrideStyleSheets[ix] + "] for editor [" + editorElement.id + "].\n");
              editor.addOverrideStyleSheet(this.mEditorElement.overrideStyleSheets[ix]);
            }
          }
          this.mbAddOverrideStyleSheets = false;
          if (this.mEditorElement.mgMathStyleSheet != null)
            editor.addOverrideStyleSheet(this.mEditorElement.mgMathStyleSheet);
          else
            editor.addOverrideStyleSheet(gMathStyleSheet);
        } catch (e) {dump("Exception in msiEditorDocumentObserver obs_documentCreated, adding overrideStyleSheets: " + e);}

        try {
          this.doInitFastCursor();
        }
        catch(e) {
          dump("Failed to init fast cursor: "+e+"\n");
        }
        if ("UpdateWindowTitle" in window)
          UpdateWindowTitle();
        // Add mouse click watcher if right type of editor
//        msiDumpWithID("About to enter 'if msiIsHTMLEditor' clause in msiEditorDocumentObserver obs_documentCreated for editor [@].\n", this.mEditorElement);
        if (msiIsHTMLEditor(this.mEditorElement))
        {
          addClickEventListenerForEditor(this.mEditorElement);
          addFocusEventListenerForEditor(this.mEditorElement);

          // Force color widgets to update
          msiOnFontColorChange();
          msiOnBackgroundColorChange();
//          editor.addTagInfo("resource:///res/tagdefs/latexdefs.xml");
          editor.setTopXULWindow(window);
          // also initialize the sidebar in this case
          try {initSidebar();} catch(e){}
          // now is the time to initialize the autosubstitute engine
          try {
            var autosub = Components.classes["@mozilla.org/autosubstitute;1"].getService(Components.interfaces.msiIAutosub);
            autosub.initialize("resource:///res/tagdefs/autosubs.xml");
          }
          catch(e) {
            dump(e+"\n");
          }
//temp?          editor.SetTagListPath("chrome://editor/content/default.xml");
          // the order here is important, since the autocomplete component has to read the tag names 
//          initializeAutoCompleteStringArrayForEditor(editor);
        }

//        if (this.mDumpMessage)
//        {
//          msiDumpWithID("Current contents of editor [@] with URL [" + currentURL + "] are: [\n", this.mEditorElement);
//          if (editor != null)
//            editor.dumpContentTree();
//          dump("\n].\n");
//        }
        if (bIsRealDocument)
        {
          try
          {
            var documentInfo = new msiDocumentInfo(editorElement);
            documentInfo.initializeDocInfo();
            var dlgInfo = documentInfo.getDialogInfo();  //We aren't going to launch the dialog, just want the data in this form.
            if (dlgInfo.saveOptions.storeViewSettings)
              editorElement.viewSettings = msiGetViewSettingsFromDocument(editorElement);
            else
              msiEditorDoShowInvisibles(editorElement, getViewSettingsFromViewMenu());
          }
          catch (exc) {dump("Exception in msiEditorDocumentObserver obs_documentCreated, showing invisibles: " + exc + "\n");}
        }

//        msiDumpWithID("About to check for mbInsertInitialContents in msiEditorDocumentObserver obs_documentCreated for editor [@].\n", this.mEditorElement);
        if (bIsRealDocument && this.mbInsertInitialContents && ("initialEditorContents" in this.mEditorElement) && (this.mEditorElement.initialEditorContents != null)
                       && (this.mEditorElement.initialEditorContents.length > 0))
        {
          try
          {
            msiDumpWithID("Adding initial contents for editorElement [@]; contents are [" + this.mEditorElement.initialEditorContents + "].\n", this.mEditorElement);
            var htmlEditor = this.mEditorElement.getHTMLEditor(this.mEditorElement.contentWindow);
            var bIsSinglePara = true;
            if (this.mEditorElement.mbInitialContentsMultiPara)
              bIsSinglePara = false;
  //          htmlEditor.insertHTML(editorElement.initialEditorContents);
//              htmlEditor.insertHTMLWithContext(this.mEditorElement.mInitialEditorContents, null, null, "text/html", null,null,0,true);
            if (insertXMLAtCursor(htmlEditor, this.mEditorElement.initialEditorContents, bIsSinglePara, true))
              this.mbInsertInitialContents = false;
          }
          catch (exc) {dump("Exception in msiEditorDocumentObserver obs_documentCreated, adding initialContents: " + exc + "\n");}
        }
//        --this.mDumpMessage;
//        msiDumpWithID("About to check for mInitialEditorObserver in msiEditorDocumentObserver obs_documentCreated for editor [@].\n", this.mEditorElement);
        if (bIsRealDocument && ("mInitialEditorObserver" in this.mEditorElement) && (this.mEditorElement.mInitialEditorObserver != null))
        {
          var editorStr = "non-null";
          if (editor == null)
            editorStr = "null";
          msiDumpWithID("Adding mInitialEditorObserver for editor [@]; editor is " + editorStr + ".\n", this.mEditorElement);
          try
          {
            for (var ix = 0; ix < this.mEditorElement.mInitialEditorObserver.length; ++ix)
            {
//              var editor = msiGetEditor(editorElement);
              editor.addEditorObserver( this.mEditorElement.mInitialEditorObserver[ix] );
            }
          }    
          catch (exc) {dump("Exception in msiEditorDocumentObserver obs_documentCreated, adding initialEditorObserver: " + exc);}
        }
//        var extraDumpStr = "  bIsRealDocument is ";
//        if (bIsRealDocument)
//          extraDumpStr += "true, and mEditorSeqInitializer is [";
//        else
//          extraDumpStr += "false, and mEditorSeqInitializer is [";
//        extraDumpStr += this.mEditorElement.mEditorSeqInitializer;
//        dump(extraDumpStr + "].\n");
//        msiDumpWithID("About to check for mEditorSeqInitializer in msiEditorDocumentObserver obs_documentCreated for editor [@].\n", this.mEditorElement);
        if (bIsRealDocument && ("mEditorSeqInitializer" in this.mEditorElement) && (this.mEditorElement.mEditorSeqInitializer != null))
        {
          this.mEditorElement.mEditorSeqInitializer.finishedEditor(this.mEditorElement);
        }
        break;

      case "cmd_setDocumentModified":
//        msiDumpWithID("Hit setDocumentModified observer in base msiEditorDocumentObserver, for editor [@].\n", this.mEditorElement);
        window.updateCommands("save");
//        window.updateCommands("undo");
//        msiDoUpdateCommands("undo", this.mEditorElement);
        break;

      case "obs_documentWillBeDestroyed":
        dump("obs_documentWillBeDestroyed notification\n");
        break;

      case "obs_documentLocationChanged":
        // Ignore this when editor doesn't exist,
        //   which happens once when page load starts
        if (editor)
        {
          var params = newCommandParams();
          if (!params)
            return;
//          try {
//            commandManager.getCommandState(aTopic, this.mEditorElement.contentWindow, params);
//            var newURI = params.getISupportsValue("state_data");
//            if (newURI != null)
//            {
//              newURI.QueryInterface(Components.interfaces.nsIURI);
//              dump("Document location changed; new path is [" + newURI.path + "].\n");
//            }
//          } catch(exc) { dump("Exception in getting URI in obs_documentLocationChanged observer: [" + exc + "].\n"); }

          try {
            editor.updateBaseURL();
          } catch(e) { dump (e); }
        }
        break;

      case "cmd_bold":
        // Update all style items
        // cmd_bold is a proxy; see EditorSharedStartup (above) for details
        window.updateCommands("style");
        window.updateCommands("undo");
        break;
    }
  }
}

function msiSetFocusOnStartup(editorElement)
{
  try
  {
    setZoom();
    editorElement.contentWindow.focus();
  } catch(e) {}
}


function isShell (filename)
{
  var foundit;
  var re = /\/shells\//i;
  foundit = re.test(filename);
  if (!foundit)
  {
    re = /\\shells\\/i;
    foundit = re.test(filename);
  }
  dump("isShell is " + foundit + "\n");
  return foundit;
}

function EditorStartupForEditorElement(editorElement)
{

//  msiDumpWithID("Entering EditorStartupForEditorElement for element [@].\n", editorElement);
  var is_HTMLEditor = msiIsHTMLEditor(editorElement);
  var is_topLevel = msiIsTopLevelEditor(editorElement);
  var prefs = GetPrefs();
  var filename = "untitled";

  editorElement.mPreviousNonSourceDisplayMode = 0;
  editorElement.mEditorDisplayMode = -1;
  editorElement.mDocWasModified = false;  // Check if clean document, if clean then unload when user "Opens"
  editorElement.mLastFocusNode = null;
  editorElement.mLastFocusNodeWasSelected = false;

//  if (is_HTMLEditor)
  if (is_topLevel)
  {
    // XUL elements we use when switching from normal editor to edit source
    window.gContentWindowDeck = document.getElementById("ContentWindowDeck");
    window.gComputeToolbar = document.getElementById("ComputeToolbar");
    window.gViewFormatToolbar = document.getElementById("viewFormatToolbar");
  }

  // set up our global prefs object
  GetPrefsService();

  // Startup also used by other editor users, such as Message Composer
  SharedStartupForEditor(editorElement);

  // Commands specific to the Composer Application window,
  //  (i.e., not embedded editors)
  //  such as file-related commands, HTML Source editing, Edit Modes...
  msiSetupComposerWindowCommands(editorElement);
//  msiDumpWithID("Completed msiSetupComposerWindowCommands for editor [@].\n", editorElement);

  // msiShowHideToolbarButtons();
  editorElement.mEditorToolbarPrefListener = new msiEditorToolbarPrefListener(editorElement);
  msiAddToolbarPrefListener(editorElement);

  var toolbox = document.getElementById("EditorToolbox");
  if (toolbox)
    toolbox.customizeDone = MainToolboxCustomizeDone;

  editorElement.mCSSPrefListener = new msiButtonPrefListener(editorElement);

  // hide Highlight button if we are in an HTML editor with CSS mode off
  var cmd = document.getElementById("cmd_highlight");
  if (cmd)
  {
    var useCSS = prefs.getBoolPref("editor.use_css");
    if (!useCSS && is_HTMLEditor)
    {
      cmd.collapsed = true;
    }
  }

  msiDumpWithID("Just before loading Shell URL in EditorStartupForEditorElement, for editorElement [@]; docShell is currently [" + editorElement.docShell + "].\n", editorElement);
  msiLoadInitialDocument(editorElement, is_topLevel);
//  else
//  {
//    var parentEditor = msiGetParentOrTopLevelEditor(editorElement);
//    if (parentEditor != null)
//      msiFinishInitDialogEditor(editorElement, parentEditor);
//    else
//      msiLoadInitialDocument(editorElement);  //What else to do?
//  }
}

  // Get url for editor content and load it.
  // the editor gets instantiated by the editingSession when the URL has finished loading.
function msiLoadInitialDocument(editorElement, bTopLevel)
{
  try {
    var prefs = GetPrefs();
    var filename = "untitled";
    var theArgs = document.getElementById("args");
    var n = 1;
    var url = "";
    var docname = "";
    var docdir;
    var shelldir;
    var shellfile;
    var charset = "";
//rwa    var leafname = "";
//rwa    var changename = false;
    if (theArgs)
    {
      docname = document.getElementById("args").getAttribute("value");
      if ( (docname.indexOf("file://") == 0) || (docname.indexOf("about:") == 0) )
        docname = GetFilepath(docname);
  // for Windows
#ifdef XP_WIN32
      docname = docname.replace("/","\\","g");
#endif
      charset = document.getElementById("args").getAttribute("charset")
    };
    var dsprops = Components.classes["@mozilla.org/file/directory_service;1"].getService(Components.interfaces.nsIProperties);
    dump("docname starts out as " + docname + "\n");
    if (docname.length == 0)
    {
      if (!bTopLevel)
      {
        try { docname = prefs.getCharPref("swp.defaultDialogShell"); }
        catch(exc) {docname = "";}
      }
      if (docname.length == 0)
        docname = prefs.getCharPref("swp.defaultShell");
      shellfile = dsprops.get("resource:app", Components.interfaces.nsILocalFile);
      shellfile.append("shells");
      dump("shelldir is "+shellfile.path + "\n");
      docname = shellfile.path + "/" + docname;
  // for Windows
#ifdef XP_WIN32
      docname = docname.replace("/","\\","g");
#endif
      dump("Docname is " + docname + "\n");
    }
//rwa    else
//rwa    {
//rwa      shellfile = Components.classes["@mozilla.org/file/local;1"].
//rwa            createInstance(Components.interfaces.nsILocalFile);
//rwa    } 
    if (isShell(docname))
    {
      editorElement.isShellFile = true;
      if (bTopLevel)
        docname = msiPrepareDocumentTargetForShell(docname);

//*******************BEGIN SECTION REFACTORED OUT
//      shellfile.initWithPath(docname);
//      // copy the shell to the default document area
//      try
//      {
//        var docdirname = prefs.getCharPref("swp.prefDocumentDir");
//        dump("swp.prefDcoumentDir is ", docdirname + "\n");
//        docdir = Components.classes["@mozilla.org/file/local;1"].
//            createInstance(Components.interfaces.nsILocalFile);
//        docdir.initWithPath(docdirname);
//        if (!valueOf(docdir.exists()))
//          docdir.create(1, 0755);
//      }
//      catch (e)
//      {
//        var dirkey;
//#ifdef XP_WIN
//          dirkey = "Pers";
//#else
//#ifdef XP_MACOSX
//          dirkey = "UsrDocs";
//#else
//          dirkey = "Home";
//#endif
//#endif
//        // if we can't find the one in the prefs, get the default
//        docdir = dsprops.get(dirkey, Components.interfaces.nsILocalFile);
//        if (!docdir.exists()) docdir.create(1,0755);
//        // Choose one of the three following lines depending on the app
//        docdir.append("SWP Docs");
//        if (!docdir.exists()) docdir.create(1,0755);
//        // docdir.append("SW Docs");
//        // docdir.append("SNB Docs");
//        dump("default document directory is "+docdir.path+"\n");
//      }
//      // find n where untitledn.MSI_EXTENSION is the first unused file name in that folder
//      try
//      {
//        dump("shellfile is " + shellfile.path + "\n");
//        leafname = shellfile.leafName;
//        var i = leafname.lastIndexOf(".");
//        if (i > 0) leafname = leafname.substr(0,i);
//        dump("leafname is " + leafname + "\n");
//        while (n < 100)
//        {
//          try {
//            dump("Copying "+shellfile.path + " to directory " + docdir.path + ", file "+filename+n+"."+MSI_EXTENSION+"\n");
//            shellfile.copyTo(docdir,filename+n+"."+MSI_EXTENSION);
//            // if the copy succeeded, continue to copy the "..._files" directory
//            try {
//              dump("Succeeded\n");
//              var auxdir = shellfile.parent.clone();
//              dump("auxdir is " + auxdir.path+"\n");
//              auxdir.append(leafname+"_files");
//              dump("Succeeded. auxdir is " + auxdir.path + "\n");
//              dump("Trying to copy auxdir to docdir\n");
//              auxdir.copyTo(docdir,filename+n+"_files");
//            }
//            catch(e) {
//              dump("Copying auxdir caused error "+e+"\n");
//            }
//            break;
//          }
//          catch(e) {
//            dump("File already exists? "+e+"\n");
//            n++;
//          }
//        }
//      }// at this point, shellfile is .../untitledxxx.MSI_EXTENSION
//      catch(e)
//      {
//        dump("Unable to open document for editing\n");
//      }
//      // set the url for the xhtml portion of the document
//      docdir.append("untitled"+n+"." + MSI_EXTENSION);
//      dump("Final docdir path is " + docdir.path + "\n");
//      url = docdir.path;
//      dump("url is " + url + "\n");
//*******************END SECTION REFACTORED OUT

      
//*************FOLLOWING NOW HANDLED UNIFORMLY BELOW
//rwa      // set document title when?? It is too early now, so we will save the name in the XUL
//rwa      var theFilename = document.getElementById("filename");
//rwa      if (theFilename != null)
//rwa        theFilename.value = filename+n;
    }

//rwa    else
//rwa    {
    url = docname;
  // for Windows
#ifdef XP_WIN32
    docname = docname.replace("\\","/","g");
#endif
    var dotIndex = docname.lastIndexOf(".");
    var lastSlash = docname.lastIndexOf("/")+1;
    var theFilename = document.getElementById("filename");
    if (theFilename != null)
      theFilename.value = dotIndex == -1?docname.slice(lastSlash):docname.slice(lastSlash,dotIndex);
//rwa    }

    var contentViewer = null;
    if (editorElement.docShell)
      contentViewer = editorElement.docShell.contentViewer;
    if (contentViewer)
    {
      contentViewer.QueryInterface(Components.interfaces.nsIMarkupDocumentViewer);
      contentViewer.defaultCharacterSet = charset;
      contentViewer.forceCharacterSet = charset;
    }
    msiEditorLoadUrl(editorElement, url);
//    msiDumpWithID("Back from call to msiEditorLoadUrl for editor [@].\n", editorElement);
  } catch (e) {
    dump("Error in loading URL in msiLoadInitialDocument: [" + e + "]\n");
  }
}

  // Get url for editor content and load it.
  // the editor gets instantiated by the editingSession when the URL has finished loading.
function msiPrepareDocumentTargetForShell(docname)
{
  try {
    var prefs = GetPrefs();
    var filename = "untitled";
    var theArgs = document.getElementById("args");
    var n = 1;
    var url = "";
    var docdir;
//    var shelldir;
//    var shellfile;
//    var charset = "";
    var leafname = "";
//    if (theArgs)
//    {
//      docname = document.getElementById("args").getAttribute("value");
//      if (docname.indexOf("file://") == 0) docname = GetFilepath(docname);
//  // for Windows
//#ifdef XP_WIN32
//      docname = docname.replace("/","\\","g");
//#endif
//      charset = document.getElementById("args").getAttribute("charset")
//    };
    var dsprops = Components.classes["@mozilla.org/file/directory_service;1"].getService(Components.interfaces.nsIProperties);
//    dump("docname starts out as " + docname + "\n");
//    if (docname.length == 0)
//    {
//       docname=prefs.getCharPref("swp.defaultShell");
//       shellfile = dsprops.get("resource:app", Components.interfaces.nsILocalFile);
//       shellfile.append("shells");
//       dump("shelldir is "+shellfile.path + "\n");
//       docname = shellfile.path + "/" + docname;
//  // for Windows
//#ifdef XP_WIN32
//      docname = docname.replace("/","\\","g");
//#endif
//      dump("Docname is " + docname + "\n");
//    }
//    else
//    {
//      shellfile = Components.classes["@mozilla.org/file/local;1"].
//            createInstance(Components.interfaces.nsILocalFile);
//    } 
//    if (isShell(docname))
//    {
//      editorElement.isShellFile = true;


    var shellfile = Components.classes["@mozilla.org/file/local;1"].
          createInstance(Components.interfaces.nsILocalFile);
    shellfile.initWithPath(docname);
    // copy the shell to the default document area
    try
    {
      var docdirname = prefs.getCharPref("swp.prefDocumentDir");
      dump("swp.prefDcoumentDir is ", docdirname + "\n");
      docdir = Components.classes["@mozilla.org/file/local;1"].
          createInstance(Components.interfaces.nsILocalFile);
      docdir.initWithPath(docdirname);
      if (!valueOf(docdir.exists()))
        docdir.create(1, 0755);
    }
    catch (e)
    {
      var dirkey;
      var dirname;
#ifdef XP_WIN
        dirkey = "Pers";
        dirname = "SWP Docs";
        // dirname = "SW Docs";
        // dirname = "SNB Docs";
#else
#ifdef XP_MACOSX
        dirkey = "UsrDocs";
#else
        dirkey = "Home";
#endif
        dirname = "SWP_Docs";
        // dirname = "SW_Docs";
        // dirname = "SNB_Docs";
#endif
      // if we can't find the one in the prefs, get the default
      docdir = dsprops.get(dirkey, Components.interfaces.nsILocalFile);
      if (!docdir.exists()) docdir.create(1,0755);
      docdir.append( dirname );
      if (!docdir.exists()) docdir.create(1,0755);
      dump("default document directory is "+docdir.path+"\n");
    }
    // find n where untitledn.MSI_EXTENSION is the first unused file name in that folder.
    // Windows version expected a copy failure to indicate the file already exists.
    // That doesn't work in Unix---the copy always succeeds.
    // Instead we explicitly look for a file that doesn't yet exist, and use that file.
    try
    {
      leafname = shellfile.leafName;
      var i = leafname.lastIndexOf(".");
      if (i > 0) leafname = leafname.substr(0,i);
      // dump("leafname is " + leafname + "\n");
      while (n < 100)
      {
        var targetfile =  Components.classes["@mozilla.org/file/local;1"].
          createInstance(Components.interfaces.nsILocalFile);
        targetfile.initWithPath(docdir.path);
        targetfile.append(filename + n+"." + MSI_EXTENSION);
        // dump( "File name is "+targetfile.path+"\n");
        if( targetfile.exists() ) {
          n++;
          // dump( targetfile.path+" exists. New n is "+n+"\n");
        } else break;
      }
      if (n>=100) dump("Too many untitled files. using untitled100\n");
          // dump("Copying "+shellfile.path + " to directory " + docdir.path + ", file "+filename+n+"."+MSI_EXTENSION+"\n");
          shellfile.copyTo(docdir,filename+n+"."+MSI_EXTENSION);
          // dump("Copy done.\n");
          try {
            var auxdir = shellfile.parent.clone();
            // dump("auxdir is " + auxdir.path+"\n");
            auxdir.append(leafname+"_files");
            auxdir.copyTo(docdir,filename+n+"_files");
          }
          catch(e) {
            dump("Copying auxdir caused error "+e+"\n");
          }
    }// at this point, shellfile is .../untitledxxx.MSI_EXTENSION
    catch(e)
    {
      dump("Unable to open document for editing\n");
    }
    // set the url for the xhtml portion of the document
    docdir.append(filename + n+"." + MSI_EXTENSION);
    // dump("Final docdir path is " + docdir.path + "\n");
    url = docdir.path;
    // dump("url is " + url + "\n");
  }
  catch(e)
  {
    dump("Uncaught error in msiPrepareDocumentTargetForShell: " + e + "\n.");
  }
  return url;
      
//      // set document title when?? It is too early now, so we will save the name in the XUL
//      var theFilename = document.getElementById("filename");
//      if (theFilename != null)
//        theFilename.value = filename+n;
//    }
//    else
//    {
//      url = docname;
//  // for Windows
//#ifdef XP_WIN32
//      docname = docname.replace("\\","/","g");
//#endif
//      var dotIndex = docname.lastIndexOf(".");
//      var lastSlash = docname.lastIndexOf("/")+1;
//      var theFilename = document.getElementById("filename");
//      if (theFilename != null)
//        theFilename.value = dotIndex == -1?docname.slice(lastSlash):docname.slice(lastSlash,dotIndex);
//    }  
////    var contentViewer = GetCurrentEditorElement().docShell.contentViewer;
//    var contentViewer = null;
//    if (editorElement.docShell)
//      contentViewer = editorElement.docShell.contentViewer;
//    if (contentViewer)
//    {
//      contentViewer.QueryInterface(Components.interfaces.nsIMarkupDocumentViewer);
//      contentViewer.defaultCharacterSet = charset;
//      contentViewer.forceCharacterSet = charset;
//    }
//    msiEditorLoadUrl(editorElement, url);
////    msiDumpWithID("Back from call to msiEditorLoadUrl for editor [@].\n", editorElement);
//  } catch (e) {
//    dump("Error in loading URL in EditorStartupForEditorElement: [" + e + "]\n");
//  }
}

function msiFinishInitDialogEditor(editorElement, parentEditorElement)
{

  var parentEditor = msiGetEditor(parentEditorElement);
//  msiDumpWithID("In msiEditor.msiFinishInitDialogEditor for editorElement [@], parentEditor is [" + parentEditor + "].\n", editorElement);
  if (parentEditor)
  {
    var editor = msiGetEditor(editorElement);
    if (editor != null)
      editor.tagListManager = parentEditor.tagListManager;
    //editor.QueryInterface(nsIEditorStyleSheets);
    try
    {
      var parentDOMStyle = parentEditor.document.QueryInterface(Components.interfaces.nsIDOMDocumentStyle);
      var parentSheets = parentDOMStyle.styleSheets;
      if (parentSheets.length > 0)
      {
        if (!("overrideStyleSheets" in editorElement) || (editorElement.overrideStyleSheets == null))
          editorElement.overrideStyleSheets = new Array();
      }
      for (var ix = 0; ix < parentSheets.length; ++ix)
      {
        editorElement.overrideStyleSheets.push( parentSheets.item(ix).href );  //parentSheets.item(ix) is an nsIDOMStyleSheet, supposedly.
        msiDumpWithID("In msiEditor.msiFinishInitDialogEditor for editor [@], adding parent style sheet href = [" + parentSheets.item(ix).href + "].\n", editorElement);
      }
    }
    catch(exc) { msiDumpWithID("In msiEditor.msiFinishInitDialogEditor for editor [@], unable to access parent style sheets: [" + exc + "].\n", editorElement); }
  }
}

function msiEditorLoadUrl(editorElement, url)
{
  try {
    if (url)
//      GetCurrentEditorElement().webNavigation.loadURI(url, // uri string
      editorElement.webNavigation.loadURI(url, // uri string
             msIWebNavigation.LOAD_FLAGS_BYPASS_CACHE,     // load flags
             null,                                         // referrer
             null,                                         // post-data stream
             null);
  } catch (e) { dump(" EditorLoadUrl failed: "+e+"\n"); }
}

// This should be called by all Composer types
function SharedStartupForEditor(editorElement)
{
  // Just for convenience
//  gContentWindow = window.content;

  // Set up the mime type and register the commands.
  if (msiIsHTMLEditor(editorElement))
  {
//    alert("Calling msiSetupHTMLEditorCommands from SharedStartupForEditor");
    msiSetupHTMLEditorCommands(editorElement);
  }
  else
  {
//    alert("Calling msiSetupTextEditorCommands from SharedStartupForEditor");
    msiSetupTextEditorCommands(editorElement);
  }

  var theDocument = null;

  // add observer to be called when document is really done loading 
  // and is modified
  // Note: We're really screwed if we fail to install this observer!
  try {
    theDocument = msiGetTopLevelWindow(window).document;
    var commandManager = msiGetCommandManager(editorElement);
    if (!editorElement.mEditorDocumentObserver || (editorElement.mEditorDocumentObserver == null))
      editorElement.mEditorDocumentObserver = new msiEditorDocumentObserver(editorElement);

    commandManager.addCommandObserver(editorElement.mEditorDocumentObserver, "obs_documentCreated");
    commandManager.addCommandObserver(editorElement.mEditorDocumentObserver, "cmd_setDocumentModified");
    commandManager.addCommandObserver(editorElement.mEditorDocumentObserver, "obs_documentWillBeDestroyed");
    commandManager.addCommandObserver(editorElement.mEditorDocumentObserver, "obs_documentLocationChanged");

    // Until nsIControllerCommandGroup-based code is implemented,
    //  we will observe just the bold command to trigger update of
    //  all toolbar style items
    commandManager.addCommandObserver(editorElement.mEditorDocumentObserver, "cmd_bold");
//    msiDumpWithID("In SharedStartupForEditor for editor [@], got through adding CommandObservers.\n", editorElement);
    dump("  Currently the docshell is [" + editorElement.docShell + "] and commandManager is [" + editorElement.commandManager + "]; commandManager to which observers were added is [" + commandManager + "].\n");

    if ("mInitialDocObserver" in editorElement && editorElement.mInitialDocObserver != null)
    {
      for (var ix = 0; ix < editorElement.mInitialDocObserver.length; ++ix)
      {
        if (("bAdded" in editorElement.mInitialDocObserver[ix]) && (editorElement.mInitialDocObserver[ix].bAdded == true))
          continue;
//        msiDumpWithID("Adding mInitialDocObserver for command " + editorElement.mInitialDocObserver[ix].mCommand + " for editor [@].\n", editorElement);
        commandManager.addCommandObserver(editorElement.mInitialDocObserver[ix].mObserver, editorElement.mInitialDocObserver[ix].mCommand);
        editorElement.mInitialDocObserver[ix].bAdded = true;
//        msiDumpWithID("Adding doc observer for editor [@]; for command [" + editorElement.mInitialDocObserver[ix].mCommand + "].\n", editorElement);
      }
//      commandManager.addCommandObserver(editorElement.mInitialDocCreatedObserver, "obs_documentCreated");
//      editorElement.mInitialDocCreatedObserver = null;
    }
  } catch (e) { dump("In SharedStartupForEditor, exception: [" + e + "].\n"); }

  var isMac = (GetOS() == msigMac);

  // Set platform-specific hints for how to select cells
  // Mac uses "Cmd", all others use "Ctrl"
  var tableKey = GetString(isMac ? "XulKeyMac" : "TableSelectKey");
  var dragStr = tableKey+GetString("Drag");
  var clickStr = tableKey+GetString("Click");

  var delStr = GetString(isMac ? "Clear" : "Del");

  SafeSetAttribute(theDocument, "menu_SelectCell", "acceltext", clickStr);
  SafeSetAttribute(theDocument, "menu_SelectRow", "acceltext", dragStr);
  SafeSetAttribute(theDocument, "menu_SelectColumn", "acceltext", dragStr);
  SafeSetAttribute(theDocument, "menu_SelectAllCells", "acceltext", dragStr);
  // And add "Del" or "Clear"
  SafeSetAttribute(theDocument, "menu_DeleteCellContents", "acceltext", delStr);

  // Set text for indent, outdent keybinding

  // hide UI that we don't have components for
//  msiRemoveInapplicableUIElements(editorElement);

  gPrefs = GetPrefs();

  // Use browser colors as initial values for editor's default colors
  var BrowserColors = GetDefaultBrowserColors();
  if (BrowserColors)
  {
    gDefaultTextColor = BrowserColors.TextColor;
    gDefaultBackgroundColor = BrowserColors.BackgroundColor;
  }

  // For new window, no default last-picked colors
  editorElement.mColorObj = new aColorObj(editorElement);
  editorElement.mColorObj.LastTextColor = "";
  editorElement.mColorObj.LastBackgroundColor = "";
  editorElement.mColorObj.LastHighlightColor = "";
}

// This method is only called by Message composer when recycling a compose window
function msiEditorResetFontAndColorAttributes(editorElement)
{
  try {  
    document.getElementById("cmd_fontFace").setAttribute("state", "");
    msiEditorRemoveTextProperty(editorElement, "font", "color");
    msiEditorRemoveTextProperty(editorElement, "font", "bgcolor");
    msiEditorRemoveTextProperty(editorElement, "font", "size");
    msiEditorRemoveTextProperty(editorElement, "small", "");
    msiEditorRemoveTextProperty(editorElement, "big", "");
    var bodyelement = msiGetBodyElement(editorElement);
    if (bodyelement)
    {
      var editor = msiGetEditor(editorElement);
      editor.removeAttributeOrEquivalent(bodyelement, "text", true);
      editor.removeAttributeOrEquivalent(bodyelement, "bgcolor", true);
      bodyelement.removeAttribute("link");
      bodyelement.removeAttribute("alink");
      bodyelement.removeAttribute("vlink");
      editor.removeAttributeOrEquivalent(bodyelement, "background", true);
    }
    else
    {
      msiDumpWithID("In msiEditorResetFontAndColorAttributes, no bodyelement for editorElement [@].\n", editorElement);
    }
    editorElement.mColorObj.LastTextColor = "";
    editorElement.mColorObj.LastBackgroundColor = "";
    editorElement.mColorObj.LastHighlightColor = "";
    document.getElementById("cmd_fontColor").setAttribute("state", "");
    document.getElementById("cmd_backgroundColor").setAttribute("state", "");
    msiUpdateDefaultColors(editorElement);
  } catch (e) {}
}


function ShutdownAnEditor(editorElement)
{
  try
  {
    SetUnicharPref("swp.zoom_factor", msiGetMarkupDocumentViewer(editorElement).textZoom);
  } catch(e) { dump( "In ShutdownAnEditor, setting unicharpref, error: " + e + "\n" ); }
  if (editorElement.softsavetimer) editorElement.softsavetimer.cancel();
  try
  {
    msiRemoveActiveEditor(editorElement);
    msiRemoveToolbarPrefListener(editorElement);
    editorElement.mCSSPrefListener.shutdown();
  } catch(e) { dump( "In ShutdownAnEditor, removing active editor and toolbar pref listener, error: " + e + "\n" ); }

  try {
    var commandManager = msiGetCommandManager(editorElement);
    commandManager.removeCommandObserver(editorElement.mEditorDocumentObserver, "obs_documentCreated");
    commandManager.removeCommandObserver(editorElement.mEditorDocumentObserver, "obs_documentWillBeDestroyed");
    commandManager.removeCommandObserver(editorElement.mEditorDocumentObserver, "obs_documentLocationChanged");
    if ("mInitialDocObserver" in editorElement && editorElement.mInitialDocObserver != null)
    {
      for (var ix = 0; ix < editorElement.mInitialDocObserver.length; ++ix)
      {
        if (("bAdded" in editorElement.mInitialDocObserver[ix]) && (editorElement.mInitialDocObserver[ix].bAdded == true))
        {
          commandManager.removeCommandObserver(editorElement.mInitialDocObserver[ix].mObserver, editorElement.mInitialDocObserver[ix].mCommand);
          editorElement.mInitialDocObserver[ix].bAdded = false;
        }
      }
    }
  } catch (e) {dump( "In ShutdownAnEditor, removing command observers, error: " + e + "\n" );}   
}

function SafeSetAttribute(theDocument, nodeID, attributeName, attributeValue)
{
  var theNode = theDocument.getElementById(nodeID);
  if (theNode)
      theNode.setAttribute(attributeName, attributeValue);
}

function msiDocumentHasBeenSaved(editorElement)
{
  var fileurl = "";
  try {
    fileurl = msiGetEditorURL(editorElement);
  } catch (e) {
    return false;
  }

  if (!fileurl || IsUrlUntitled(fileurl) || IsUrlAboutBlank(fileurl))
    return false;

  // We have a file URL already
  return true;
}

function msiCheckAndSaveDocument(editorElement, command, allowDontSave)
{
  var document;
  try {
    // if we don't have an editor or an document, bail
    var editor = msiGetEditor(editorElement);
    if (!editor) return true;
    document = editor.document;
    if (!document)
      return true;
    if (!editor.documentModified && !msiIsHTMLSourceChanged(editorElement))
    {
      if (command == "cmd_close" && ("isShellFile" in editorElement) && editorElement.isShellFile)
      // if the document is a shell and has never been saved, it will be deleted by Revert
        doRevert(editorElement, true);
      return true;
    }
  } catch (e) { return true; }

  // call window.focus, since we need to pop up a dialog
  // and therefore need to be visible (to prevent user confusion)
  top.document.commandDispatcher.focusedWindow.focus();  

  var scheme = GetScheme(msiGetEditorURL(editorElement));
  var doPublish = (scheme && scheme != "file");

  var strID;
  switch (command)
  {
    case "cmd_close":
      strID = "BeforeClosing";
      break;
    case "cmd_preview":
      strID = "BeforePreview";
      break;
    case "cmd_editSendPage":
      strID = "SendPageReason";
      break;
    case "cmd_validate":
      strID = "BeforeValidate";
      break;
  }
    
  var reasonToSave = strID ? GetString(strID) : "";

  var title = GetFilename(msiGetDocumentBaseUrl(editorElement));
  if (!title) title="untitled document";

  var dialogTitle = GetString(doPublish ? "PublishPage" : "SaveDocument");
  var dialogMsg = GetString(doPublish ? "PublishPrompt" : "SaveFilePrompt");
  dialogMsg = (dialogMsg.replace(/%title%/,title)).replace(/%reason%/,reasonToSave);

  var promptService = msiGetPromptService();
  if (!promptService)
    return false;

  var result = {value:0};
  var promptFlags = promptService.BUTTON_TITLE_CANCEL * promptService.BUTTON_POS_1;
  var button1Title = null;
  var button3Title = null;

  if (doPublish)
  {
    promptFlags += promptService.BUTTON_TITLE_IS_STRING * promptService.BUTTON_POS_0;
    button1Title = GetString("Publish");
    button3Title = GetString("DontPublish");    
  }
  else
  {
    promptFlags += promptService.BUTTON_TITLE_SAVE * promptService.BUTTON_POS_0;
  }

  // If allowing "Don't..." button, add that
  if (allowDontSave)
    promptFlags += doPublish ?
        (promptService.BUTTON_TITLE_IS_STRING * promptService.BUTTON_POS_2)
        : (promptService.BUTTON_TITLE_DONT_SAVE * promptService.BUTTON_POS_2);
  
  result = promptService.confirmEx(window, dialogTitle, dialogMsg, promptFlags,
                          button1Title, null, button3Title, null, {value:0});

  if (result == 0)
  {
    // Save, but first finish HTML source mode
    if (msiIsHTMLSourceChanged(editorElement)) {
      try {
        msiFinishHTMLSource(editorElement);
      } catch (e) { return false;}
    }

    if (doPublish)
    {
      // We save the command the user wanted to do in a global
      // and return as if user canceled because publishing is asynchronous
      // This command will be fired when publishing finishes
      editorElement.mgCommandAfterPublishing = command;
      msiGoDoCommand("cmd_publish", editorElement);
      return false;
    }

    // Save to local disk
    var contentsMIMEType;
    if (msiIsHTMLEditor(editorElement))
      contentsMIMEType = editor.contentsMIMEType;
    else
      contentsMIMEType = kTextMimeType;
    var success = msiSaveDocument(false, false, false, contentsMIMEType, editorElement);
    return success;
  }

  if (result == 2) 
  {
    // "Don't Save"
    if (command == "cmd_close" && ("isShellFile" in editorElement) && editorElement.isShellFile)
    {
      var del = true;
      doRevert(editorElement, del);
    }
    return true;
  }
  // Default or result == 1 (Cancel)
  return false;
}


function doRevert(editorElement, del)
{
  var urlstring = msiGetEditorURL(editorElement);
  var documentfile = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
  var currFilePath = GetFilepath(urlstring);
// for Windows
#ifdef XP_WIN32
  currFilePath = currFilePath.replace("/","\\","g");
#endif
  documentfile.initWithPath( currFilePath );
  msiRevertFile( documentfile, del );
}
// --------------------------- File menu ---------------------------
//The File menu items should only be accessible by the main editor window, really. Commented out here, available in msiMainEditor.js.

//// used by openLocation. see openLocation.js for additional notes.
//function delayedOpenWindow(chrome, flags, url)
//{
//  dump("setting timeout\n");
//  setTimeout("window.openDialog('"+chrome+"','_blank','"+flags+"','"+url+"')", 10);
//}
//
function msiEditorNewPlaintext()
{
  try
  {
    var editorElement = msiGetActiveEditorElement();
    var parentWindow = msiGetWindowContainingEditor(editorElement);
    parentWindow.openDialog( "chrome://editor/content/TextEditorAppShell.xul",
                             "_blank",
                             "chrome,dialog=no,all",
                             "about:blank");
  }
  catch(exc) {AlertWithTitle("Error in msiEditor.js", "In msiEditorNewPlaintext(), failed to open; exception: " + exc);}
}

//// Check for changes to document and allow saving before closing
//// This is hooked up to the OS's window close widget (e.g., "X" for Windows)
//function msiEditorCanClose(editorElement)
//{
//  // Returns FALSE only if user cancels save action
//  if (!editorElement)
//    editorElement = GetCurrentEditorElement();
//
//  // "true" means allow "Don't Save" button
//  var canClose = msiCheckAndSaveDocument(editorElement, "cmd_close", true);
//
//  // This is our only hook into closing via the "X" in the caption
//  //   or "Quit" (or other paths?)
//  //   so we must shift association to another
//  //   editor or close any non-modal windows now
//  if (canClose && "InsertCharWindow" in window && window.InsertCharWindow)
//    SwitchInsertCharToAnotherEditorOrClose();
//
//  return canClose;
//}

// --------------------------- View menu ---------------------------

// used by viewZoomOverlay.js
function msiGetMarkupDocumentViewer(editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var contentViewer = null;
  if (editorElement)
    contentViewer = editorElement.docShell.contentViewer;
  if (!contentViewer)
    contentViewer = msiGetActiveEditorElement().docShell.contentViewer;
  contentViewer.QueryInterface(Components.interfaces.nsIMarkupDocumentViewer);
  return contentViewer;
}

function setZoom()
{
  var zoomfactor = 1.0;
  try {
    var zoomstr;
    zoomstr = gPrefs.getCharPref("swp.zoom_factor");
    zoomfactor = parseFloat(zoomstr);
  }
  catch(ex) {
    dump("\nfailed to get zoom_factor pref!\n");
  }
  getMarkupDocumentViewer().textZoom = zoomfactor;
}

var gRealtimeSpellPrefs = {
  RTSPref: "swp.spellchecker.enablerealtimespell",
 
  observe: function(aSubject, aTopic, aPrefName)
  {
    if (aTopic != "nsPref:changed" || aPrefName != "enablerealtimespell")
      return;

    try {
      var prefService = Components.classes["@mozilla.org/preferences-service;1"]
        .getService(Components.interfaces.nsIPrefBranch);
      editorElement = msiGetActiveEditorElement();
      var editor = msiGetEditor(editorElement);
    
      editor.setSpellcheckUserOverride(prefService.getBoolPref(this.RTSPref));
    }
    catch(e)
    {
      dump("Failed to reset spell checking preference");
    }
  },

  register: function()
  {
    var prefService = Components.classes["@mozilla.org/preferences-service;1"]
                                .getService(Components.interfaces.nsIPrefService);
    this._branch = prefService.getBranch("swp.spellchecker.");
    this._branch.QueryInterface(Components.interfaces.nsIPrefBranch2);
    this._branch.addObserver("", this, false);
  },

  unregister: function()
  {
    if(!this._branch) return;
    this._branch.removeObserver("", this);
  }
}
gRealtimeSpellPrefs.register();

function SetDocumentCharacterSetForEditor(aCharset, editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  try {
    var editor = msiGetEditor(editorElement);
    editor.documentCharacterSet = aCharset;
    var docUrl = msiGetEditorURL(editorElement);
    if( !IsUrlAboutBlank(docUrl))
    {
      // reloading the document will reverse any changes to the META charset, 
      // we need to put them back in, which is achieved by a dedicated listener
      editor.addDocumentStateListener( aDocumentReloadListener(editorElement) );
      EditorLoadUrl(docUrl);
    }
  } catch (e) {}
}

// ------------------------------------------------------------------
function msiUpdateCharsetPopupMenu(menuPopup)
{
  var editorElement = msiGetActiveEditorElement();
  if (msiIsDocumentModified(editorElement) && !msiIsDocumentEmpty(editorElement))
//  if (IsDocumentModified() && !IsDocumentEmpty())
  {
    for (var i = 0; i < menuPopup.childNodes.length; i++)
    {
      var menuItem = menuPopup.childNodes[i];
      menuItem.setAttribute('disabled', 'true');
    }
  }
}

// --------------------------- Text style ---------------------------

function msiOnParagraphFormatChange(paraMenuList, commandID)
{
  if (!paraMenuList)
    return;

//  var documentList = msiGetUpdatableItemContainers(commandID);  //Returns the documents containing the command nodes to update.
//
//  for (var i = 0; i < documentList.length; ++i)
//  {
//    var theDoc = documentList[i];
//    if (paraMenuList.ownerDocument == theDoc)
//    {
      try
      {
        var commandNode = theDoc.getElementById(commandID);
        var state = commandNode.getAttribute("state");

        // force match with "normal"
        if (state == "body")
          state = "";

        if (state == "mixed")
        {
          //Selection is the "mixed" ( > 1 style) state
          paraMenuList.selectedItem = null;
          paraMenuList.setAttribute("label",GetString('Mixed'));
        }
        else
        {
          var menuPopup = document.getElementById("ParagraphPopup");
          var menuItems = menuPopup.childNodes;
          for (var i=0; i < menuItems.length; i++)
          {
            var menuItem = menuItems.item(i);
            if ("value" in menuItem && menuItem.value == state)
            {
              paraMenuList.selectedItem = menuItem;
              break;
            }
          }
        }
      }
      catch(exc)
      {
        AlertWithTitle("Error in onFontFaceChange", exc);
      }
//      break;  //we found it
//    }
//  }
}

function onFontFaceChange(fontFaceMenuList, commandID)
{
//  var documentList = msiGetUpdatableItemContainers(commandID);  //Returns the documents containing the command nodes to update.
//
//  for (var i = 0; i < documentList.length; ++i)
//  {
//    var theDoc = documentList[i];
//    if (theDoc == fontFaceMenuList.ownerDocument)
//    {
      try
      {
        var commandNode = theDoc.getElementById(commandID);
        var state = commandNode.getAttribute("state");

        if (state == "mixed")
        {
          //Selection is the "mixed" ( > 1 style) state
          fontFaceMenuList.selectedItem = null;
          fontFaceMenuList.setAttribute("label",GetString('Mixed'));
        }
        else
        {
          var menuPopup = theDoc.getElementById("FontFacePopup");
          var menuItems = menuPopup.childNodes;
          for (var i=0; i < menuItems.length; i++)
          {
            var menuItem = menuItems.item(i);
            if (menuItem.getAttribute("label") && ("value" in menuItem && menuItem.value.toLowerCase() == state.toLowerCase()))
            {
              fontFaceMenuList.selectedItem = menuItem;
              break;
            }
          }
        }
      }
      catch(exc)
      {
        AlertWithTitle("Error in onFontFaceChange", exc);
      }
//    break;
//  }
}

function msiEditorSelectFontSize(editorElement)
{
  var theDoc = null;
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  try
  {
    theDoc = msiGetTopLevelWindow().document;
  }
  catch(exc) {AlertWithTitle("Error in msiEditorSelectFontSize!", exc);}

  if (theDoc == null)
    theDoc = document;
  var select = theDoc.getElementById("FontSizeSelect");
  if (select)
  {
    if (select.selectedIndex == -1)
      return;

    msiEditorSetFontSize(gFontSizeNames[select.selectedIndex], editorElement);
  }
}

function onFontSizeChange(fontSizeMenulist, commandID)
{
  // If we don't match anything, set to "0 (normal)"
  var newIndex = 2;
  var size = fontSizeMenulist.getAttribute("size");
  if ( size == "mixed")
  {
    // No single type selected
    newIndex = -1;
  }
  else
  {
    for (var i = 0; i < gFontSizeNames.length; i++)
    {
      if( gFontSizeNames[i] == size )
      {
        newIndex = i;
        break;
      }
    }
  }
  if (fontSizeMenulist.selectedIndex != newIndex)
    fontSizeMenulist.selectedIndex = newIndex;
}

function msiEditorSetFontSize(size, editorElement)
{
//  var editorElement = msiGetParentEditorElementForDialog(window);
//  var editor = msiGetEditor(editorElement);
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  if (!editorElement)
    editorElement = msiGetTopLevelEditorElement();
  if (!editorElement)
  {
    AlertWithTitle("Error", "No editor in msiEditorSetFontSize!");
  }
  msiEditorSetTextProperty(editorElement, "fontsize", "size", size);
//  }
  try {if (editorElement.contentWindow) editorElement.contentWindow.focus();}
  catch (e)
  {}
}

function msiInitFontFaceMenu(menuPopup, editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  if (!editorElement)
    editorElement = msiGetTopLevelEditorElement();
  msiInitLocalFontFaceMenu(menuPopup, editorElement);

  if (menuPopup)
  {
    var children = menuPopup.childNodes;
    if (!children) return;

    var firstHas = { value: false };
    var anyHas = { value: false };
    var allHas = { value: false };

    // we need to set or clear the checkmark for each menu item since the selection
    // may be in a new location from where it was when the menu was previously opened

    // Fixed width (second menu item) is special case: old TT ("teletype") attribute
    msiEditorGetTextProperty(editorElement, "tt", "", "", firstHas, anyHas, allHas);
    children[1].setAttribute("checked", allHas.value);

    if (!anyHas.value)
      msiEditorGetTextProperty(editorElement, "font", "face", "", firstHas, anyHas, allHas);

    children[0].setAttribute("checked", !anyHas.value);

    // Skip over default, TT, and separator
    for (var i = 3; i < children.length; i++)
    {
      var menuItem = children[i];
      var faceType = menuItem.getAttribute("value");

      if (faceType)
      {
        msiEditorGetTextProperty(editorElement, "font", "face", faceType, firstHas, anyHas, allHas);

        // Check the menuitem only if all of selection has the face
        if (allHas.value)
        {
          menuItem.setAttribute("checked", "true");
          break;
        }

        // in case none match, make sure we've cleared the checkmark
        menuItem.removeAttribute("checked");
      }
    }
  }
}

//const kFixedFontFaceMenuItems = 7; // number of fixed font face menuitems  (defined elsewhere)

function msiInitLocalFontFaceMenu(menuPopup, editorElement)
{
  if (!editorElement)
  {
    editorElement = msiGetActiveEditorElement();
    if (!editorElement)
      return;
  }
  if (!editorElement.mLocalFonts)
  {
    // Build list of all local fonts once per editor
    try 
    {
      var enumerator = Components.classes["@mozilla.org/gfx/fontenumerator;1"]
                                 .getService(Components.interfaces.nsIFontEnumerator);
      var localFontCount = { value: 0 }
      editorElement.mLocalFonts = enumerator.EnumerateAllFonts(localFontCount);
    }
    catch(e) { }
  }
  
  var useRadioMenuitems = (menuPopup.parentNode.localName == "menu"); // don't do this for menulists  
  if (menuPopup.childNodes.length == kFixedFontFaceMenuItems) 
  {
    if (editorElement.mLocalFonts.length == 0) {
      menuPopup.childNodes[kFixedFontFaceMenuItems - 1].hidden = true;
    }
    for (var i = 0; i < editorElement.mLocalFonts.length; ++i)
    {
      if (editorElement.mLocalFonts[i] != "")
      {
        var itemNode = document.createElementNS(XUL_NS, "menuitem");
        itemNode.setAttribute("label", editorElement.mLocalFonts[i]);
        itemNode.setAttribute("value", editorElement.mLocalFonts[i]);
        if (useRadioMenuitems) {
          itemNode.setAttribute("type", "radio");
          itemNode.setAttribute("name", "2");
          itemNode.setAttribute("observes", "cmd_renderedHTMLEnabler");
        }
        menuPopup.appendChild(itemNode);
      }
    }
  }
}
 

//TO BE DONE: Again, propose calling via "goDoCommand(cmd_initFontSizeMenu)" with appropriate command handler.
function msiInitFontSizeMenu(menuPopup, editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();

  if (menuPopup)
  {
    var children = menuPopup.childNodes;
    if (!children) return;

    var firstHas = { value: false };
    var anyHas = { value: false };
    var allHas = { value: false };

    var sizeWasFound = false;

    // we need to set or clear the checkmark for each menu item since the selection
    // may be in a new location from where it was when the menu was previously opened

    // First 2 items add <small> and <big> tags
    // While it would be better to show the number of levels,
    //  at least this tells user if either of them are set
    var menuItem = children[0];
    if (menuItem)
    {
      msiEditorGetTextProperty(editorElement, "small", "", "", firstHas, anyHas, allHas);
      menuItem.setAttribute("checked", allHas.value);
      sizeWasFound = anyHas.value;
    }

    menuItem = children[1];
    if (menuItem)
    {
      msiEditorGetTextProperty(editorElement, "big", "", "", firstHas, anyHas, allHas);
      menuItem.setAttribute("checked", allHas.value);
      sizeWasFound |= anyHas.value;
    }

    // Fixed size items start after menu separator
    var menuIndex = 3;
    // Index of the medium (default) item
    var mediumIndex = 5;

    // Scan through all supported "font size" attribute values
    for (var i = -2; i <= 3; i++)
    {
      menuItem = children[menuIndex];

      // Skip over medium since it'll be set below.
      // If font size=0 is actually set, we'll toggle it off below if
      // we enter this loop in this case.
      if (menuItem && (i != 0))
      {
        var sizeString = (i <= 0) ? String(i) : ("+" + String(i));
        msiEditorGetTextProperty(editorElement, "font", "size", sizeString, firstHas, anyHas, allHas);
        // Check the item only if all of selection has the size...
        menuItem.setAttribute("checked", allHas.value);
        // ...but remember if ANY of of selection had size set
        sizeWasFound |= anyHas.value;
      }
      menuIndex++;
    }

    // if no size was found, then check default (medium)
    // note that no item is checked in the case of "mixed" selection
    children[mediumIndex].setAttribute("checked", !sizeWasFound);
  }
}

function onHighlightColorChange()
{
  var topWindow = msiGetTopLevelWindow();
  var commandNode = topWindow.document.getElementById("cmd_highlight");
  if (commandNode)
  {
    var color = commandNode.getAttribute("state");
    var button = topWindow.document.getElementById("HighlightColorButton");
    if (button)
    {
      // No color set - get color set on page or other defaults
      if (!color)
        color = "transparent" ;

      button.setAttribute("style", "background-color:"+color+" !important");
    }
  }
}

function msiOnHighlightColorChange()
{
  var topWindow = msiGetTopLevelWindow();
  var commandNode = topWindow.document.getElementById("cmd_highlight");
  if (commandNode)
  {
    var color = commandNode.getAttribute("state");
    var button = topWindow.document.getElementById("HighlightColorButton");
    if (button)
    {
      // No color set - get color set on page or other defaults
      if (!color)
        color = "transparent" ;

      button.setAttribute("style", "background-color:"+color+" !important");
    }
  }
}

function msiOnFontColorChange()
{
  var topWindow = msiGetTopLevelWindow();
  var commandNode = topWindow.document.getElementById("cmd_fontColor");
  if (commandNode)
  {
    var color = commandNode.getAttribute("state");
    var button = topWindow.document.getElementById("TextColorButton");
    if (button)
    {
      // No color set - get color set on page or other defaults
      if (!color)
        color = gDefaultTextColor;
      button.setAttribute("style", "background-color:"+color);
    }
  }
}

function msiOnBackgroundColorChange()
{
  var topWindow = msiGetTopLevelWindow();
  var commandNode = topWindow.document.getElementById("cmd_backgroundColor");
  if (commandNode)
  {
    var color = commandNode.getAttribute("state");
    var button = topWindow.document.getElementById("BackgroundColorButton");
    if (button)
    {
      if (!color)
        color = gDefaultBackgroundColor;

      button.setAttribute("style", "background-color:"+color);
    }
  }
}

// Call this when user changes text and/or background colors of the page
function msiUpdateDefaultColors(editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();

  var BrowserColors = GetDefaultBrowserColors();
  var bodyelement = msiGetBodyElement(editorElement);
  var defTextColor = gDefaultTextColor;
  var defBackColor = gDefaultBackgroundColor;

  if (bodyelement)
  {
    var color = bodyelement.getAttribute("text");
    if (color)
      editorElement.gDefaultTextColor = color;
    else if (BrowserColors)
      editorElement.gDefaultTextColor = BrowserColors.TextColor;

    color = bodyelement.getAttribute("bgcolor");
    if (color)
      editorElement.gDefaultBackgroundColor = color;
    else if (BrowserColors)
      editorElement.gDefaultBackgroundColor = BrowserColors.BackgroundColor;
  }
  else
  {
    msiDumpWithID("In msiUpdateDefaultColors, unable to find bodyelement for editorElement [@].\n", editorElement);
  }

  // Trigger update on toolbar
  if (defTextColor != editorElement.gDefaultTextColor)
  {
    msiGoUpdateCommandState("cmd_fontColor", editorElement);
    msiOnFontColorChange();
  }
  if (defBackColor != editorElement.gDefaultBackgroundColor)
  {
    msiGoUpdateCommandState("cmd_backgroundColor", editorElement);
    msiOnBackgroundColorChange();
  }
}

function msiGetBackgroundElementWithColor(editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var editor = msiGetTableEditor(editorElement);
  if (!editor)
    return null;

  editorElement.mColorObj.Type = "";
  editorElement.mColorObj.PageColor = "";
  editorElement.mColorObj.TableColor = "";
  editorElement.mColorObj.CellColor = "";
  editorElement.mColorObj.BackgroundColor = "";
  editorElement.mColorObj.SelectedType = "";

  var tagNameObj = { value: "" };
  var element;
  try {
    element = editor.getSelectedOrParentTableElement(tagNameObj, {value:0});
  }
  catch(e) {}

  if (element && tagNameObj && tagNameObj.value)
  {
    editorElement.mColorObj.BackgroundColor = msiGetHTMLOrCSSStyleValue(editorElement, element, "bgcolor", "background-color");
    editorElement.mColorObj.BackgroundColor = ConvertRGBColorIntoHEXColor(editorElement.mColorObj.BackgroundColor);
    if (tagNameObj.value.toLowerCase() == "td")
    {
      editorElement.mColorObj.Type = "Cell";
      editorElement.mColorObj.CellColor = editorElement.mColorObj.BackgroundColor;

      // Get any color that might be on parent table
      var table = GetParentTable(element);
      editorElement.mColorObj.TableColor = msiGetHTMLOrCSSStyleValue(editorElement, table, "bgcolor", "background-color");
      editorElement.mColorObj.TableColor = ConvertRGBColorIntoHEXColor(editorElement.mColorObj.TableColor);
    }
    else
    {
      editorElement.mColorObj.Type = "Table";
      editorElement.mColorObj.TableColor = editorElement.mColorObj.BackgroundColor;
    }
    editorElement.mColorObj.SelectedType = editorElement.mColorObj.Type;
  }
  else
  {
    var prefs = GetPrefs();
    var IsCSSPrefChecked = prefs.getBoolPref("editor.use_css");
    if (IsCSSPrefChecked && msiIsHTMLEditor(editorElement))
    {
      var selection = editor.selection;
      if (selection)
      {
        element = selection.focusNode;
        while (!editor.nodeIsBlock(element))
          element = element.parentNode;
      }
      else
      {
        element = msiGetBodyElement(editorElement);
      }
    }
    else
    {
      element = msiGetBodyElement(editorElement);
    }
    if (element)
    {
      editorElement.mColorObj.Type = "Page";
      editorElement.mColorObj.BackgroundColor = msiGetHTMLOrCSSStyleValue(editorElement, element, "bgcolor", "background-color");
      if (editorElement.mColorObj.BackgroundColor == "")
      {
        editorElement.mColorObj.BackgroundColor = "transparent";
      }
      else
      {
        editorElement.mColorObj.BackgroundColor = ConvertRGBColorIntoHEXColor(editorElement.mColorObj.BackgroundColor);
      }
      editorElement.mColorObj.PageColor = editorElement.mColorObj.BackgroundColor;
    }
  }
  return element;
}

function msiSetSmiley(editorElement, smileyText)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  try {
    msiGetEditor(editorElement).insertText(smileyText);
    editorElement.focus();
  }
  catch(e) {}
}

function msiEditorSelectColor(colorType, mouseEvent, editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);
  if (!editor || !editorElement.mColorObj)
    return;

  // Shift + mouse click automatically applies last color, if available
  var useLastColor = mouseEvent ? ( mouseEvent.button == 0 && mouseEvent.shiftKey ) : false;
  var element;
  var table;
  var currentColor = "";
  var commandNode;

  if (!colorType)
    colorType = "";

  if (colorType == "Text")
  {
    editorElement.mColorObj.Type = colorType;

    // Get color from command node state
    commandNode = document.getElementById("cmd_fontColor");
    currentColor = commandNode.getAttribute("state");
    currentColor = ConvertRGBColorIntoHEXColor(currentColor);
    editorElement.mColorObj.TextColor = currentColor;

    if (useLastColor && editorElement.mColorObj.LastTextColor )
      editorElement.mColorObj.TextColor = editorElement.mColorObj.LastTextColor;
    else
      useLastColor = false;
  }
  else if (colorType == "Highlight")
  {
    editorElement.mColorObj.Type = colorType;

    // Get color from command node state
    commandNode = document.getElementById("cmd_highlight");
    currentColor = commandNode.getAttribute("state");
    currentColor = ConvertRGBColorIntoHEXColor(currentColor);
    editorElement.mColorObj.HighlightColor = currentColor;

    if (useLastColor && editorElement.mColorObj.LastHighlightColor )
      editorElement.mColorObj.HighlightColor = editorElement.mColorObj.LastHighlightColor;
    else
      useLastColor = false;
  }
  else
  {
    element = msiGetBackgroundElementWithColor(editorElement);
    if (!element)
      return;

    // Get the table if we found a cell
    if (editorElement.mColorObj.Type == "Table")
      table = element;
    else if (editorElement.mColorObj.Type == "Cell")
      table = GetParentTable(element);

    // Save to avoid resetting if not necessary
    currentColor = editorElement.mColorObj.BackgroundColor;

    if (colorType == "TableOrCell" || colorType == "Cell")
    {
      if (editorElement.mColorObj.Type == "Cell")
        editorElement.mColorObj.Type = colorType;
      else if (editorElement.mColorObj.Type != "Table")
        return;
    }
    else if (colorType == "Table" && editorElement.mColorObj.Type == "Page")
      return;

    if (colorType == "" && editorElement.mColorObj.Type == "Cell")
    {
      // Using empty string for requested type means
      //  we can let user select cell or table
      editorElement.mColorObj.Type = "TableOrCell";
    }

    if (useLastColor && editorElement.mColorObj.LastBackgroundColor )
      editorElement.mColorObj.BackgroundColor = editorElement.mColorObj.LastBackgroundColor;
    else
      useLastColor = false;
  }
  // Save the type we are really requesting
  colorType = editorElement.mColorObj.Type;

  if (!useLastColor)
  {
    // Avoid the JS warning
    editorElement.mColorObj.NoDefault = false;

    // Launch the ColorPicker dialog
    // TODO: Figure out how to position this under the color buttons on the toolbar
    window.openDialog("chrome://editor/content/EdColorPicker.xul", "_blank", "chrome,close,titlebar,modal", "", editorElement.mColorObj);

    // User canceled the dialog
    if (editorElement.mColorObj.Cancel)
      return;
  }

  if (editorElement.mColorObj.Type == "Text")
  {
    if (currentColor != editorElement.mColorObj.TextColor)
    {
      if (editorElement.mColorObj.TextColor)
        msiEditorSetTextProperty(editorElement, "font", "color", editorElement.mColorObj.TextColor);
      else
        msiEditorRemoveTextProperty(editorElement, "font", "color");
    }
    // Update the command state (this will trigger color button update)
    msiGoUpdateCommandState("cmd_fontColor", editorElement);
  }
  else if (editorElement.mColorObj.Type == "Highlight")
  {
    if (currentColor != editorElement.mColorObj.HighlightColor)
    {
      if (editorElement.mColorObj.HighlightColor)
        msiEditorSetTextProperty(editorElement, "font", "bgcolor", editorElement.mColorObj.HighlightColor);
      else
        msiEditorRemoveTextProperty(editorElement, "font", "bgcolor");
    }
    // Update the command state (this will trigger color button update)
    msiGoUpdateCommandState("cmd_highlight", editorElement);
  }
  else if (element)
  {
    if (editorElement.mColorObj.Type == "Table")
    {
      // Set background on a table
      // Note that we shouldn't trust "currentColor" because of "TableOrCell" behavior
      if (table)
      {
        var bgcolor = table.getAttribute("bgcolor");
        if (bgcolor != editorElement.mColorObj.BackgroundColor)
        try {
          if (editorElement.mColorObj.BackgroundColor)
            editor.setAttributeOrEquivalent(table, "bgcolor", editorElement.mColorObj.BackgroundColor, false);
          else
            editor.removeAttributeOrEquivalent(table, "bgcolor", false);
        } catch (e) {}
      }
    }
    else if (currentColor != editorElement.mColorObj.BackgroundColor && msiIsHTMLEditor(editorElement))
    {
      editor.beginTransaction();
      try
      {
        editor.setBackgroundColor(editorElement.mColorObj.BackgroundColor);

        if (editorElement.mColorObj.Type == "Page" && editorElement.mColorObj.BackgroundColor)
        {
          // Set all page colors not explicitly set,
          //  else you can end up with unreadable pages
          //  because viewer's default colors may not be same as page author's
          var bodyelement = msiGetBodyElement(editorElement);
          if (bodyelement)
          {
            var defColors = GetDefaultBrowserColors();
            if (defColors)
            {
              if (!bodyelement.getAttribute("text"))
                editor.setAttributeOrEquivalent(bodyelement, "text", defColors.TextColor, false);

              // The following attributes have no individual CSS declaration counterparts
              // Getting rid of them in favor of CSS implies CSS rules management
              if (!bodyelement.getAttribute("link"))
                editor.setAttribute(bodyelement, "link", defColors.LinkColor);

              if (!bodyelement.getAttribute("alink"))
                editor.setAttribute(bodyelement, "alink", defColors.LinkColor);

              if (!bodyelement.getAttribute("vlink"))
                editor.setAttribute(bodyelement, "vlink", defColors.VisitedLinkColor);
            }
          }
        }
      }
      catch(e) {}

      editor.endTransaction();
    }

    msiGoUpdateCommandState("cmd_backgroundColor", editorElement);
  }
  editorElement.focus();
}

function GetParentTable(element)
{
  var node = element;
  while (node)
  {
    if (node.nodeName.toLowerCase() == "table")
      return node;

    node = node.parentNode;
  }
  return node;
}

function GetParentTableCell(element)
{
  var node = element;
  while (node)
  {
    if (node.nodeName.toLowerCase() == "td" || node.nodeName.toLowerCase() == "th")
      return node;

    node = node.parentNode;
  }
  return node;
}

function EditorDblClick(event)
{
  // We check event.explicitOriginalTarget here because .target will never
  // be a textnode (bug 193689)
//  var editorElement = GetEditorElementForDocument(event.currentTarget.ownerDocument);

  var editorElement = msiGetEditorElementFromEvent(event);
  msiSetActiveEditor(editorElement, false);

  if (event.explicitOriginalTarget)
  {
    // Only bring up properties if clicked on an element or selected link
    var element;
    try {
      element = event.explicitOriginalTarget.QueryInterface(Components.interfaces.nsIDOMElement);
    } catch (e) {}

    if (!element)
      try {
//        var editorElement = GetEditorElementForDocument(event.currentTarget.ownerDocument);
        element = msiGetEditor(editorElement).getSelectedElement("href");
      } catch (e) {}

    if (element)
    {
      dump("In EditorDblClick, element is [" + element.nodeName + "].\n");
      goDoPrinceCommand("cmd_objectProperties", element, editorElement);
      event.preventDefault();
    }
  }
}

function EditorClick(event)
{
  if (!event)
    return;

  if (event.detail == 2)
  {
    EditorDblClick(event);
    return;
  }

//  event.currentTarget should be "body" or something...

  // For Web Composer: In Show All Tags Mode,
  // single click selects entire element,
  //  except for body and table elements
  var editorElement = msiGetEditorElementFromEvent(event);
//  if (IsWebComposer() && event.explicitOriginalTarget && msiIsHTMLEditor(editorElement) &&
//      msiGetEditorDisplayMode(editorElement) == kDisplayModeAllTags)
  msiSetActiveEditor(editorElement, false);
  if (event.explicitOriginalTarget && msiIsHTMLEditor(editorElement) &&
          msiGetEditorDisplayMode(editorElement) == kDisplayModeAllTags)
  {
    try
    {
      // We check event.explicitOriginalTarget here because .target will never
      // be a textnode (bug 193689)
      var element = event.explicitOriginalTarget.QueryInterface(
                        Components.interfaces.nsIDOMElement);
      var name = element.localName.toLowerCase();
      if (name != "body" && name != "table" &&
          name != "td" && name != "th" && name != "caption" && name != "tr")
      {          
        
        msiGetEditor(editorElement).selectElement(event.explicitOriginalTarget);
        event.preventDefault();
      }
    } catch (e) {}
  }
}

function msiGetCharForProperties(editorElement)
{
  var nodeData = msiGetObjectDataForProperties(editorElement);
  if (nodeData != null && nodeData.theNode != null && nodeData.theOffset != null)
  {
    return nodeData;
  }
  return null;
}

//Resolves whether the object at anIndex inside aNode is a character or a node.
function msiPropertiesObjectData(aNode, anIndex)
{
  if (aNode == null)
  {
    dump("In msiPropertiesObjectData constructor, null node was passed in!\n");
    return null;
  }

  if (anIndex != null)
  {
    if (aNode.nodeType == nsIDOMNode.TEXT_NODE)
    {
      //What do we return to say we want the character to the left?
      this.theNode = aNode;
      this.theOffset = anIndex;
    }
    else if (anIndex < aNode.childNodes.length)
    {
      this.theNode = aNode.childNodes[anIndex];
      this.theOffset = null;
    }
    else  //trouble??
    {
      dump("Problem case in msiPropertiesObjectData - aNode is [" + aNode.nodeName + "], with length [" + aNode.childNodes.length + "], and anIndex is [" + anIndex + "].\n");
      this.theNode = aNode;
      this.theOffset = null;
    }
  }
  else
  {
    this.theNode = aNode;
    this.theOffset = null;
  }
}

function msiGetObjectDataForProperties(editorElement)
{
  var editor = msiGetEditor(editorElement);
  var nodeData = msiSelectPropertiesObjectFromSelection(editor);
  return nodeData;
}

/*TODO: We need an oncreate hook to do enabling/disabling for the
        Format menu. There should be code like this for the
        object-specific "Properties" item

*/
// For property dialogs, we want the selected element,
//  but will accept a parent link, list, or table cell if inside one
function msiGetObjectForProperties(editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);
  if (!editor || !msiIsHTMLEditor(editorElement))
    return null;
  return msiEditorGetObjectForProperties(editor);
}

function msiEditorGetObjectForProperties(editor)
{
  var element;
  try {
    element = editor.getSelectedElement("");
  } catch (e) {}
  if (element)
    return element;

  // Find nearest parent of selection anchor node
  //   that is a link, list, table cell, or table

  var nodeData = msiSelectPropertiesObjectFromSelection(editor);
  if (nodeData != null && nodeData.theNode != null)
  {
    if (nodeData.theOffset == null)
      return nodeData.theNode;
    else
      return null; //Calls to this function are assumed to want some object or other to revise - those which can
                   //revise a character in text should go in through the msiGetCharForProperties(editorElement) method,
                   //or directly through the msiSelectPropertiesObjectFromSelection function?
  }
  var node = null;
  var anchorNode;
  try {
    anchorNode = editor.selection.anchorNode;
    if (anchorNode.firstChild)
    {
      // Start at actual selected node
      var offset = editor.selection.anchorOffset;
      // Note: If collapsed, offset points to element AFTER caret,
      //  thus node may be null
      node = anchorNode.childNodes.item(offset);
    }
    if (!node)
      node = anchorNode;
  } catch (e) {}

  var origNode = node;

  //We didn't pick up anything. Try again the old-fashioned (Composer code) way.
  node = origNode;
  while (node)
  {
    if (node.nodeName)
    {
      var nodeName = node.nodeName.toLowerCase();

      // Done when we hit the body
      if (nodeName == "body") break;

      if ((nodeName == "a" && node.href) ||
          nodeName == "ol" || nodeName == "ul" || nodeName == "dl" ||
          nodeName == "td" || nodeName == "th" ||
          nodeName == "table")
      {
        return node;
      }
    }
    node = node.parentNode;
  }
  return null;
}

function msiSelectPropertiesObjectFromSelection(editor)
{
  var retObj = null;
  if (editor.selection.isCollapsed)
    retObj = msiSelectPropertiesObjectFromCursor(editor);
  else
  {
    var theRange = editor.selection.getRangeAt(0);
    if (theRange == null)
    {
      theRange = editor.document.createRange();
      theRange.setStart(editor.selection.anchorNode, editor.selection.anchorOffset);
      theRange.setEnd(editor.selection.focusNode, editor.selection.focusOffset);
      if (theRange.collapsed)  //According to the W3C DOM Range interface specification, this will happen if anchor was after focus.
      {
        theRange.setStart(editor.selection.focusNode, editor.selection.focusOffset);
        theRange.setEnd(editor.selection.anchorNode, editor.selection.anchorOffset);
      }
    }
    if (theRange.collapsed)
      retObj = msiSelectPropertiesObjectFromCursor(editor);
    else
      retObj = msiSelectPropertiesObjectFromRange(editor, theRange);
  }

  if ( (retObj != null) && (retObj.theOffset == null) )  //in the case we've identified a revisable object
  {
    var parentNode = msiNavigationUtils.findWrappingStyleNode(retObj.theNode);
    if (parentNode != null)
      retObj.theNode = parentNode;
  }

  return retObj;
  //We're trying to identify the situation where there's essentially (to be laboriously defined below) one object selected.
  //The plan is:
  //  (1) In the best situation, we'll find that anchorNode and focusNode are children of the same element, with a difference
  //        of one in their offsets. That node is then the revisable one.
  //  (2) If one of anchorNode/focusNode is a child of the common ancestor but the other isn't, we want to look for the situation
  //        where the non-direct child is essentially at the start/end of the node we're hoping for in (1). This should be handled
  //        carefully, but the idea is much like the msiFindRevisableObjectToLeft() function - we'd like to be able to move up
  //        to the parent but have to ensure the situation warrants it.
  //  (3) If neither is a direct child of commonAncestor, we try to move both positions up to the parents. The hope is that the
  //        positions passed in are just beyond the ends of an object, with nothing significant between; failing that, the hope is
  //        that they're just inside the ends of an object.
  //  Now write the code?
//  var rangeContentList = msiNavigationUtils.getSignificantRangeContent(theRange);

}

function msiSelectPropertiesObjectFromRange(editor, aRange)
{
  var aNode = null;
  var anOffset = null;

  if (aRange.startContainer == aRange.endContainer)  //the simple case
  {
    if (aRange.endOffset - aRange.startOffset == 1)  //the simplest (hoped-for) case
    {
      //Want something else here - another function that returns one of our "properties objects" from a node and offset?
      //Even in the simplest case, have to distinguish between selecting a character (parent is Text) and selecting a node.
      return new msiPropertiesObjectData(aRange.startContainer, aRange.startOffset);
    }
    if (msiNavigationUtils.positionIsAtStart(aRange.startContainer, aRange.startOffset) && msiNavigationUtils.positionIsAtEnd(aRange.endContainer, aRange.endOffset))
    {
      return new msiPropertiesObjectData(aRange.startContainer, null);
    }
    return null;
  }
  else  //Can we move up the tree without essentially changing the range?
  {
    var retVal = null;
    var ancestorNode = aRange.commonAncestorContainer;
    var trialSequenceStart = [0];
    if (aRange.startContainer != ancestorNode)
      trialSequenceStart = [1,2];  //means try left first, then right
    var trialSequenceEnd = [0];
    if (aRange.endContainer != ancestorNode)
      trialSequenceEnd = [2,1];  //means try right first, then left

    for (var jx = 0; jx < trialSequenceStart.length; ++jx)
    {
      for (var kx = 0; kx < trialSequenceEnd.length; ++kx)
      {
        var bChanged = false;
        var newRange = aRange.cloneRange();
        if ((trialSequenceStart[jx] == 1) && msiNavigationUtils.positionIsAtStart(aRange.startContainer, aRange.startOffset))
        {
          bChanged = true;
          newRange.setStartBefore(aRange.startContainer);
//          msiNavigationUtils.moveRangeStartOutOfObject(newRange, false);  //the "false" means "to left" as opposed to "to right"
        }
        else if ((trialSequenceStart[jx] == 2) && msiNavigationUtils.positionIsAtEnd(aRange.startContainer, aRange.startOffset) && msiNavigationUtils.boundaryIsTransparent(aRange.startContainer, editor, msiNavigationUtils.rightEndToRight))
//RWA-insert        else if ((trialSequenceStart[jx] == 2) && msiNavigationUtils.positionIsAtEnd(aRange.startContainer, aRange.startOffset) && msiNavigationUtils.boundaryIsTransparent(aRange.startContainer, editor, msiNavigationUtils.rightEndToRight))
        {
          bChanged = true;
          newRange.setStartAfter(aRange.startContainer);
//          msiNavigationUtils.moveRangeStartOutOfObject(newRange, true);  //the "true" means "to right"
        }
        if ((trialSequenceEnd[kx] == 1) && msiNavigationUtils.positionIsAtStart(aRange.endContainer, aRange.endOffset) && msiNavigationUtils.boundaryIsTransparent(aRange.endContainer, editor, msiNavigationUtils.leftEndToLeft))
//RWA-insert        if ((trialSequenceEnd[kx] == 1) && msiNavigationUtils.positionIsAtStart(aRange.endContainer, aRange.endOffset) && msiNavigationUtils.boundaryIsTransparent(aRange.endContainer, editor, msiNavigationUtils.leftEndToLeft))
        {
          bChanged = true;
          newRange.setEndBefore(aRange.endContainer);
//          msiNavigationUtils.moveRangeEndOutOfObject(newRange, false);  //the "false" means "to left" as opposed to "to right"
        }
        else if ((trialSequenceEnd[kx] == 2) && msiNavigationUtils.positionIsAtEnd(aRange.endContainer, aRange.endOffset))
        {
          bChanged = true;
          newRange.setEndAfter(aRange.endContainer);
//          msiNavigationUtils.moveRangeEndOutOfObject(newRange, true);  //the "true" means "to right"
        }

        if (bChanged)
          retVal = msiSelectPropertiesObjectFromRange(editor, newRange);
      }

      if ((retVal != null))
      {
        return retVal;
        break;
      }
    }
  }

  return null;
}

function msiSelectPropertiesObjectFromCursor(editor)
{
  var currNode = editor.selection.anchorNode;
  //Look to the left of the cursor:
  return msiFindRevisableObjectToLeft(currNode, editor.selection.anchorOffset, editor);
}

function msiFindRevisableObjectToLeft(aNode, anOffset, editor)
{
  var returnVal = new Object();
  returnVal.theNode = null;
  returnVal.theOffset = null;
  if (aNode.nodeType == nsIDOMNode.TEXT_NODE)
  {
    if (msiNavigationUtils.isMathname(aNode.parentNode) || msiNavigationUtils.isUnit(aNode.parentNode) )
    {
      returnVal.theNode = aNode.parentNode;
      return returnVal;
    }
    if (anOffset > 0)
    {
      //What do we return to say we want the character to the left?
      returnVal.theNode = aNode;
      returnVal.theOffset = anOffset - 1;
      return returnVal;
    }
  }
  var nextNode = null;
//  if (anOffset >= aNode.childNodes.length)
  if (msiNavigationUtils.positionIsAtEnd(aNode, anOffset))
    nextNode = aNode;
  for (var ix = anOffset; (nextNode == null) && (ix > 0); --ix)
  {
    //NOTE that we should never be in this clause if this is a text node, since unless anOffset == 0 we would have bailed out in the previous clause.
    if ( !msiNavigationUtils.isIgnorableWhitespace(aNode.childNodes[ix-1]) )
    {
      nextNode = aNode.childNodes[ix-1];
      break;
    }
  }
  if (ix != 0 && nextNode == null)
  {
    dump("Unexpected result in msiFindObjectToLeft! Return null.\n");
    return returnVal;
  }

  //Now the only way that nextNode should be null is really if we're at the beginning of aNode:
//  while ((nextNode == null) && msiNavigationUtils.boundaryIsTransparent(aNode, editor, posAndDir))
  while ( (nextNode == null) && msiNavigationUtils.positionIsAtStart(aNode, ix) )
  {
    if (msiNavigationUtils.boundaryIsTransparent(aNode, editor, msiNavigationUtils.leftEndToLeft))
    //Move to left and try again...
      nextNode = aNode.previousSibling;
    else
      return returnVal;  //No reasonable object to revise here.
    //Move to our parent since we're at his beginning,
    if (nextNode == null)
    {
      var nextParent = aNode.parentNode;
//      ix = msiNavigationUtils.offsetInParent(aNode);  //Should we just say 0 since we found no previousSibling?
      ix = 0;
      aNode = nextParent;
    }
  }

  if (nextNode != null)  //In this case we've identified an object to our left. We check whether we should descend into it:
  {
    aNode = nextNode;
    nextNode = null;
    while (aNode != null && msiNavigationUtils.boundaryIsTransparent(aNode, editor, msiNavigationUtils.rightEndToLeft))
      aNode = msiNavigationUtils.getLastSignificantChild(aNode);
//      aNode = aNode.lastChild;
  }

  //Finally, one last check on the validity of aNode. If it fails, there's nothing good to revise.
  if (msiNavigationUtils.cannotSelectNodeForProperties(aNode))
    return returnVal;

  returnVal.theNode = aNode;
  if (aNode.nodeType == nsIDOMNode.TEXT_NODE)
    returnVal.theOffset = aNode.length - 1;
  else
    returnVal.theOffset = null;
  return returnVal;
}

/******Display Mode stuff - for the time being, only applicable to main editor window, but leave the functions here anyway******/

function msiSetEditMode(mode, editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  if (!msiIsHTMLEditor(editorElement))
    return;

  var bodyElement = msiGetBodyElement(editorElement);
  if (!bodyElement)
  {
    dump("SetEditMode: We don't have a body node!\n");
    return;
  }

  // must have editor if here!
  var editor = msiGetEditor(editorElement);

  // Switch the UI mode before inserting contents
  //   so user can't type in source window while new window is being filled
  var previousMode = msiGetEditorDisplayMode(editorElement);
  if (!msiSetDisplayMode(editorElement, mode))
    return;

  if (mode == kDisplayModeSource)
  {
    // Display the DOCTYPE as a non-editable string above edit area
    var domdoc;
    try { domdoc = editor.document; } catch (e) { dump( e + "\n");}
    if (domdoc)
    {
      var doctypeNode = document.getElementById("doctype-text");
      var dt = domdoc.doctype;
      if (doctypeNode)
      {
        if (dt)
        {
          doctypeNode.collapsed = false;
          var doctypeText = "<!DOCTYPE " + domdoc.doctype.name;
          if (dt.publicId)
            doctypeText += " PUBLIC \"" + domdoc.doctype.publicId;
          if (dt.systemId)
            doctypeText += " "+"\"" + dt.systemId;
          doctypeText += "\">"
          doctypeNode.setAttribute("value", doctypeText);
        }
        else
          doctypeNode.collapsed = true;
      }
    }
    // Get the entire document's source string

    var flags = (editor.documentCharacterSet == "ISO-8859-1")
      ? 32768  // OutputEncodeLatin1Entities
      : 16384; // OutputEncodeBasicEntities
    try { 
      var encodeEntity = gPrefs.getCharPref("editor.encode_entity");
      switch (encodeEntity) {
        case "basic"  : flags = 16384; break; // OutputEncodeBasicEntities
        case "latin1" : flags = 32768; break; // OutputEncodeLatin1Entities
        case "html"   : flags = 65536; break; // OutputEncodeHTMLEntities
        case "none"   : flags = 0;     break;
      }
    } catch (e) { }

    try { 
      var prettyPrint = gPrefs.getBoolPref("editor.prettyprint");
      if (prettyPrint)
        flags |= 2; // OutputFormatted

    } catch (e) {}

    flags |= 1024; // OutputLFLineBreak
    var source = editor.outputToString("text/xml", flags);
    var start = source.search(/<html/i);
    if (start == -1) start = 0;
    var sourceTextEditor = msiGetHTMLSourceEditor(editorElement);
    try
    {
      var theSelection = sourceTextEditor.selection;
    }
    catch(ex) {dump("Unable to dump selection from source editor, error is [" + ex + "].\n");}
    sourceTextEditor.QueryInterface(Components.interfaces.nsIPlaintextEditor);
    var sourceContentWindow = msiGetHTMLSourceTextWindow(editorElement);
    if (!editorElement.mSourceTextObserver)
      editorElement.mSourceTextObserver = new msiSourceTextObserver(editorElement);
    if (!editorElement.mSourceTextListener)
      editorElement.mSourceTextListener = new msiSourceTextListener(editorElement);
    if (sourceContentWindow!=null && sourceTextEditor!=null)
    {
      sourceTextEditor.insertText(source.slice(start));
      sourceTextEditor.resetModificationCount();
      sourceTextEditor.addDocumentStateListener(editorElement.mSourceTextListener);
      sourceTextEditor.enableUndo(true);
      sourceContentWindow.commandManager.addCommandObserver(editorElement.mSourceTextObserver, "cmd_undo");
      sourceContentWindow.contentWindow.focus();
    }
    goDoCommand("cmd_moveTop");
  }
  else if (previousMode == kDisplayModeSource)
  {
    // Only rebuild document if a change was made in source window
    if (msiIsHTMLSourceChanged(editorElement))
    {
      // Reduce the undo count so we don't use too much memory
      //   during multiple uses of source window 
      //   (reinserting entire doc caches all nodes)
      try {
        editor.transactionManager.maxTransactionCount = 1;
      } catch (e) {}

      editor.beginTransaction();
      try {
        // We are coming from edit source mode,
        //   so transfer that back into the document
        var sourceTextEditor = msiGetHTMLSourceEditor(editorElement);
        source = sourceTextEditor.outputToString(kTextMimeType, 1024); // OutputLFLineBreak
        editor.rebuildDocumentFromSource(source);

        // Get the text for the <title> from the newly-parsed document
        // (must do this for proper conversion of "escaped" characters)
        var title = "";
        var titlenodelist = editor.document.getElementsByTagName("title");
        if (titlenodelist)
        {
          var titleNode = titlenodelist.item(0);
          if (titleNode && titleNode.firstChild && titleNode.firstChild.data)
            title = titleNode.firstChild.data;
        }
        if (editor.document.title != title)
          msiSetDocumentTitle(editorElement, title);

      } catch (ex) {
        dump(ex);
      }
      editor.endTransaction();

      // Restore unlimited undo count
      try {
        editor.transactionManager.maxTransactionCount = -1;
      } catch (e) {}
    }

    // Clear out the string buffers
    var sourceContentWindow = msiGetHTMLSourceTextWindow(editorElement);
    var sourceTextEditor = msiGetHTMLSourceEditor(editorElement);
    if (sourceContentWindow && sourceTextEditor)
    {
      sourceContentWindow.commandManager.removeCommandObserver(editorElement.mSourceTextObserver, "cmd_undo");
      sourceTextEditor.removeDocumentStateListener(editorElement.mSourceTextListener);
      sourceTextEditor.enableUndo(false);
      sourceTextEditor.selectAll();
      sourceTextEditor.deleteSelection(sourceTextEditor.eNone);
      sourceTextEditor.resetModificationCount();
    }
    editorElement.contentWindow.focus();
  }
}

function msiCancelHTMLSource(editorElement)
{
  // Don't convert source text back into the DOM document
  try
  {
    var sourceTextEditor = msiGetSourceTextEditor(editorElement);
    if (sourceTextEditor)
    {
      sourceTextEditor.resetModificationCount();
      msiSetDisplayMode(editorElement, msiGetPreviousNonSourceDisplayMode(editorElement));
    }
  } catch(e) {}
}

function msiFinishHTMLSource(editorElement)
{
  //Here we need to check whether the HTML source contains <head> and <body> tags
  //Or RebuildDocumentFromSource() will fail.
  if (msiIsInHTMLSourceMode(editorElement))
  {
    var sourceTextEditor = msiGetHTMLSourceEditor(editorElement);
    var htmlSource = sourceTextEditor.outputToString(kTextMimeType, 1024); // OutputLFLineBreak
    if (htmlSource.length > 0)
    {
      var beginHead = htmlSource.indexOf("<head");
      if (beginHead == -1)
      {
        AlertWithTitle(GetString("Alert"), GetString("NoHeadTag"));
        //cheat to force back to Source Mode
        msiSetDisplayMode(editorElement, kDisplayModePreview);
//        gEditorDisplayMode = kDisplayModePreview;
//        SetDisplayMode(kDisplayModeSource);
        throw Components.results.NS_ERROR_FAILURE;
      }

      var beginBody = htmlSource.indexOf("<body");
      if (beginBody == -1)
      {
        AlertWithTitle(GetString("Alert"), GetString("NoBodyTag"));
        //cheat to force back to Source Mode
        editorElement.mEditorDisplayMode = kDisplayModePreview;
        if (editorElement.contentWindow = window.content && ("gEditorDisplayMode" in window))
          window.gEditorDisplayMode = kDisplayModePreview;
        msiSetDisplayMode(editorElement, kDisplayModeSource);
        throw Components.results.NS_ERROR_FAILURE;
      }
    }
  }

  // Switch edit modes -- converts source back into DOM document
  msiSetEditMode(msiGetPreviousNonSourceDisplayMode(editorElement), editorElement);
}


function msiGetPreviousNonSourceDisplayMode(editorElement)
{
  if ("mPreviousNonSourceDisplayMode" in editorElement)
    return editorElement.mPreviousNonSourceDisplayMode;
  return kDisplayModeNormal;
}

function msiGetEditorDisplayMode(editorElement)
{
  if ("mEditorDisplayMode" in editorElement)
    return editorElement.mEditorDisplayMode;
  if ("gEditorDisplayMode" in window)
    return window.gEditorDisplayMode;
  return kDisplayModeNormal;
}

function msiSetDisplayMode(editorElement, mode)
{
  if (!msiIsHTMLEditor(editorElement))
    return false;

  // Already in requested mode:
  //  return false to indicate we didn't switch
  var previousMode = msiGetEditorDisplayMode(editorElement);
  if (mode == previousMode)
    return false;

  editorElement.mEditorDisplayMode = mode;
  if (("gEditorDisplayMode" in window) && editorElement.contentWindow == window.content)
    window.gEditorDisplayMode = mode;
  msiResetStructToolbar(editorElement);

  if ((mode == kDisplayModeSource)||(mode == kDisplayModePreview))
  {

    //Hide the formatting toolbar if not already hidden
    if ("gViewFormatToolbar" in window && window.gViewFormatToolbar != null)
      window.gViewFormatToolbar.hidden = true;
    if ("gComputeToolbar" in window && window.gComputeToolbar != null)
      window.gComputeToolbar.hidden = true;

    msiHideItem("MSIMathMenu");
    msiHideItem("cmd_viewComputeToolbar");
    msiHideItem("MSIComputeMenu");
    msiHideItem("MSITypesetMenu");
    msiHideItem("MSIInsertTypesetObjectMenu");
    msiHideItem("StandardToolbox");
    msiHideItem("SymbolToolbox");
    msiHideItem("MathToolbox");
    msiHideItem("EditingToolbox");
    msiHideItem("structToolbar");
    if ("gSourceContentWindow" in window)
      window.gSourceContentWindow.contentWindow.focus();
    // Switch to the sourceWindow or bWindow(second or third in the deck)
    if ("gContentWindowDeck" in window)
      window.gContentWindowDeck.selectedIndex = (mode==kDisplayModeSource?1:2);
  }
  else
  {
    // Save the last non-source mode so we can cancel source editing easily
    editorElement.mPreviousNonSourceDisplayMode = mode;

    // Load/unload appropriate override style sheet
    try {
      var editor = msiGetEditor(editorElement);
      editor.QueryInterface(nsIEditorStyleSheets);
      editor instanceof Components.interfaces.nsIHTMLObjectResizer;

      switch (mode)
      {
        case kDisplayModePreview:
          // Disable all extra "edit mode" style sheets 
          editor.enableStyleSheet(kNormalStyleSheet, false);
          if (editorElement.mgMathStyleSheet != null)
            editor.enableStyleSheet(editorElement.mgMathStyleSheet, false);
          editor.enableStyleSheet(gMathStyleSheet, false);
          editor.enableStyleSheet(kAllTagsStyleSheet, false);
          editor.isImageResizingEnabled = true;
          break;

        case kDisplayModeNormal:
          editor.addOverrideStyleSheet(kNormalStyleSheet);
          if (editorElement.mgMathStyleSheet != null)
            editor.addOverrideStyleSheet(editorElement.mgMathStyleSheet);
          else
            editor.addOverrideStyleSheet(gMathStyleSheet);
          // Disable ShowAllTags mode
          editor.enableStyleSheet(kAllTagsStyleSheet, false);
          editor.isImageResizingEnabled = true;
          editor.enableTagMananger();
        break;

        case kDisplayModeAllTags:
          editor.addOverrideStyleSheet(kNormalStyleSheet);
          if (editorElement.mgMathStyleSheet != null)
            editor.addOverrideStyleSheet(editorElement.mgMathStyleSheet);
          else
            editor.addOverrideStyleSheet(gMathStyleSheet);
          editor.addOverrideStyleSheet(kAllTagsStyleSheet);
          // don't allow resizing in AllTags mode because the visible tags
          // change the computed size of images and tables...
          editor.enableTagMananger();
          if (editor.resizedObject) {
            editor.hideResizers();
          }
          editor.isImageResizingEnabled = false;
          break;
      }
    } catch(e) {}

    // Switch to the normal editor (first in the deck)
    if ("gContentWindowDeck" in window)
      window.gContentWindowDeck.selectedIndex = 0;

    // Restore menus and toolbars
    if ("gViewFormatToolbar" in window && window.gViewFormatToolbar != null)
      window.gViewFormatToolbar.hidden = false;
    if ("gComputeToolbar" in window && window.gComputeToolbar != null)
      window.gComputeToolbar.hidden = false;
    msiShowItem("MSIMathMenu");
    msiShowItem("cmd_viewComputeToolbar");
    msiShowItem("MSIComputeMenu");
    msiShowItem("SymbolToolbar");
    msiShowItem("MSITypesetMenu");
    msiShowItem("MSIInsertTypesetObjectMenu");
    msiShowItem("SymbolToolbox");
    msiShowItem("StandardToolbox");
    msiShowItem("MathToolbox");
    msiShowItem("EditingToolbox");
    msiShowItem("structToolbar");
    if ("gContentWindow" in window)
      window.gContentWindow.focus();
    else
      editorElement.focus();
  }

  // update commands to disable or re-enable stuff
  window.updateCommands("mode_switch");

  // Set the selected tab at bottom of window:
  // (Note: Setting "selectedIndex = mode" won't redraw tabs when menu is used.)
  document.getElementById("EditModeTabs").selectedItem = document.getElementById(kDisplayModeTabIDS[mode]);

  // Uncheck previous menuitem and set new check since toolbar may have been used
  if (previousMode >= 0)
    document.getElementById(kDisplayModeMenuIDs[previousMode]).setAttribute("checked","false");
  document.getElementById(kDisplayModeMenuIDs[mode]).setAttribute("checked","true");
  

  return true;
}

/******End Display mode functions******/

function msiEditorToggleParagraphMarks(editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var menuItem = document.getElementById("viewParagraphMarks");
  if (menuItem)
  {
    // Note that the 'type="checbox"' mechanism automatically
    //  toggles the "checked" state before the oncommand is called,
    //  so if "checked" is true now, it was just switched to that mode
    var checked = menuItem.getAttribute("checked");
    try {
      var editor = msiGetEditor(editorElement);
      editor.QueryInterface(nsIEditorStyleSheets);

      if (checked == "true")
        editor.addOverrideStyleSheet(kParagraphMarksStyleSheet);
      else
        editor.enableStyleSheet(kParagraphMarksStyleSheet, false);
    }
    catch(e) { return; }
  }
}

function msiEditorDoShowInvisibles(editorElement, viewSettings)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  if (!editorElement)
  {
    alert("No active editor to apply the invisibles command to!");
    return;
  }
  if (("viewSettings" in editorElement) && (editorElement.viewSettings != null)
             && editorElement.viewSettings.match(viewSettings))
  {
    //already done
    return;
  }

  var editor = msiGetEditor(editorElement);
//  if (!editor.QueryInterface(Components.interfaces.nsIEditorStyleSheets))
//  {
//    alert("Can't get nsIEditorStyleSheets interface in msiEditorDoShowInvisibles!");
//    return;
//  }

  var theBody = msiGetRealBodyElement(editor.document);
  if (viewSettings.showInvisibles)
    theBody.setAttribute("showinvis", "true");
  else
    theBody.removeAttribute("showinvis");
  if (!viewSettings.showHelperLines)
    theBody.setAttribute("hideHelperLines", "true");
  else
    theBody.removeAttribute("hideHelperLines");
  if (!viewSettings.showInputBoxes)
    theBody.setAttribute("hideInputBoxes", "true");
  else
    theBody.removeAttribute("hideInputBoxes");
  if (!viewSettings.showIndexEntries)
    theBody.setAttribute("hideIndexEntries", "true");
  else
    theBody.removeAttribute("hideIndexEntries");
  if (!viewSettings.showMarkers)
    theBody.setAttribute("hideMarkers", "true");
  else
    theBody.removeAttribute("hideMarkers");
  if (!viewSettings.showFootnotes)
    theBody.setAttribute("hideFootnotes", "true");
  else
    theBody.removeAttribute("hideFootnotes");
  if (!viewSettings.showOtherNotes)
    theBody.setAttribute("hideOtherNotes", "true");
  else
    theBody.removeAttribute("hideOtherNotes");

  //  var dumpStr = "Element [" + theBody.nodeName + "] now has settings: [";
  //  var attribNames = ["showinvis", "hideHelperLines", "hideInputBoxes", "hideIndexEntries", "hideMarkers"];
  //  for (var ix = 0; ix < attribNames.length; ++ix)
  //  {
  //    if (ix > 0)
  //      dumpStr += ", ";
  //    dumpStr += attribNames[ix] + ": ";
  //    if (theBody.hasAttribute(attribNames[ix]))
  //      dumpStr += theBody.getAttribute(attribNames[ix]);
  //    else
  //      dumpStr += "(none)";
  //  }
  //  dump(dumpStr + "]\n");
  //viewSettings is an object containing members showInvisibles, showHelperLines, showInputBoxes,
  // showIndexEntries, showMarkers. Here we want to take the actual actions to cause these effects.
//  if (editorElement.mInvisiblesStyleSheet)
//    editor.removeOverrideStyleSheet(editorElement.mInvisiblesStyleSheet);
//  if (currSettings.showInvisibles != viewSettings.showInvisibles)  //this requires changing all the whitespace in the document! Here goes:
//  {
//    if (viewSettings.showInvisibles)
//      msiRewriteShowInvisibles(editor);
//    else
//      msiRewriteHideInvisibles(editor);
//  }
//  editorElement.mInvisiblesStyleSheet = viewSettings.formatStyleSheet();
//  editor.addOverrideStyleSheet(editorElement.mInvisiblesStyleSheet);
  editorElement.viewSettings = viewSettings;

} 

function msiGetViewSettingsFromDocument(editorElement)
{
  var retVal = msiGetCurrViewSettings(editorElement);
  var editor = msiGetEditor(editorElement);
  var theBody = null;
  if (editor != null)
    theBody = msiGetRealBodyElement(editor.document);
  if (!theBody)
  {
    dump("Can't get body element of document in getViewSettingsFromDocument! Aborting...\n");
    return retVal;
  }

  if (theBody.hasAttribute("showinvis"))
    retVal.showInvisibles = (theBody.getAttribute("showinvis")=="true");
  if (theBody.hasAttribute("hideHelperLines"))
    retVal.showHelperLines = (theBody.getAttribute("hideHelperLines")!="true");
  if (theBody.hasAttribute("hideInputBoxes"))
    retVal.showInputBoxes = (theBody.getAttribute("hideInputBoxes")!="true");
  if (theBody.hasAttribute("hideIndexEntries"))
    retVal.showIndexEntries = (theBody.getAttribute("hideIndexEntries")!="true");
  if (theBody.hasAttribute("hideMarkers"))
    retVal.showMarkers = (theBody.getAttribute("hideMarkers")!="true");

  return retVal;
}

function msiGetSpaceTypeFromString(theText)
{
  switch(theText)
  {
    case " ":        return "normal";
    default:         return "custom";
  }
}

function msiRewriteShowInvisibles(editor)
{
//if you find invisibles in the text node, replace them by CSS-addressable nodes

  //NOTE: the "\s" specifier is equivalent to: [\t\n\v\f\r \u00a0\u2000\u2001\u2002\u2003\u2004\u2005\u2006\u2007\u2008\u2009\u200a\u200b\u2028\u2029\u3000]
  // (per the JavaScript1.5 online reference).
  function replaceTextNodeByInvisibles(aNode)
  {
    var invisRegExp = /[\s]/g;
    var theParent = aNode.parentNode;
    var nextNode = aNode.nextSibling;
    var theString = aNode.nodeValue;
    var invisMatches;
    var startPos = 0;
    while ((invisMatches = invisRegExp.exec(theString)) != undefined)
    {
      if (startPos < invisMatches.index)
      {
        var newTextNode = aNode.ownerDocument.createTextNode(theString.substring(startPos, invisMatches.index));
        if (nextNode)
          theParent.insertBefore(newTextNode, nextNode);
        else
          theParent.appendChild(newTextNode);
      }
      var newInvisNode = aNode.ownerDocument.createElement("space");
      newInvisNode.setAttribute("msitemp-invis", "true");
      newInvisNode.setAttribute( "spacingType", msiGetSpaceTypeFromString(invisMatches[0]) );
      newInvisNode.appendChild( aNode.ownerDocument.createTextNode(invisMatches[0]) );
      if (nextNode)
        theParent.insertBefore(newInvisNode, nextNode);
      else
        theParent.appendChild(newInvisNode);
      startPos = invisRegExp.lastIndex;
    }
  }

  var rootElement = editor.document.documentElement;
  var treeWalker = editor.document.createTreeWalker(rootElement, NodeFilter.SHOW_TEXT, null, true);
  if (treeWalker)
  {
    for (var currNode = treeWalker.nextNode(); currNode != null; )
    {
      var nextNode = treeWalker.nextNode();
      replaceTextNodeByInvisibles(currNode);
      currNode = nextNode;
    }
  }
}

function msiRewriteHideInvisibles(editor)
{
  function findInvisNodes(aNode)
  {
    if (aNode.hasAttribute("msitemp-invis"))
      return NodeFilter.FILTER_ACCEPT;
    return NodeFilter.FILTER_SKIP;
  }

  //The use of this little function assumes that no Node with non-trivial (non-#text) child nodes will ever be given
  //the attribute "msitemp-invis"; also that no such Node will ever manage to absorb children. Perhaps this is too strong
  //an assumption??
  //In practice, if during editing one of these msitemp-invis nodes were to accept content, it would mess up the invisible's
  //appearance. Somehow this needs to be precluded - can we get TagManager to do it? Probably! So let's assume this is done.
  function replaceByTextNode(aNode)
  {
    var theText = "";
    for (var ix = 0; ix < aNode.childNodes.length; ++ix)
    {
      if (aNode.childNodes[ix].nodeName == "#text")
        theText += aNode.childNodes[ix].nodeValue;
    }
    var newNode = aNode.ownerDocument.createTextNode(theText);
    var nextNode = aNode.nextSibling;
    var parentNode = aNode.parentNode;
    parentNode.removeChild(aNode);
    if (nextNode != null)
      parentNode.insertBefore(newNode, nextNode);
    else
      parentNode.appendChild(newNode);
  }

  var rootElement = editor.document.documentElement;
  var treeWalker = editor.document.createTreeWalker(rootElement, NodeFilter.SHOW_ELEMENT, findInvisNodes, true);
  if (treeWalker)
  {
    for (var currNode = treeWalker.nextNode(); currNode != null; )
    {
      var nextNode = treeWalker.nextNode();
      replaceByTextNode(currNode);
      currNode = nextNode;
    }
    rootElement.normalize();
  }
}

function msiInitPasteAsMenu(editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var menuItem = document.getElementById("menu_pasteTable")
  if(menuItem)
  {
    menuItem.IsInTable  
    menuItem.setAttribute("label", GetString(msiIsInTable(editorElement) ? "NestedTable" : "Table"));
   // menuItem.setAttribute("accesskey",GetString("ObjectPropertiesAccessKey"));
  }
  // TODO: Do enabling based on what is in the clipboard
}


function msiEditorInitFormatMenu(editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  try {
    msiInitObjectPropertiesMenuitem(editorElement, "objectProperties");
    msiInitRemoveStylesMenuitems(editorElement, "removeStylesMenuitem", "removeLinksMenuitem", "removeNamedAnchorsMenuitem");
  } catch(ex) {dump("Exception in msiEditor.js, msiEditorInitFormatMenu: [" + ex + "].\n");}
}

function msiInitObjectPropertiesMenuitem(editorElement, id)
{
  // Set strings and enable for the [Object] Properties item
  // Note that we directly do the enabling instead of
  //  using goSetCommandEnabled since we already have the menuitem
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var menuItem = document.getElementById(id);
  if (!menuItem) return null;

  var nodeData;
  var element;
  var menuStr = GetString("AdvancedProperties");
  var name;

  if (msiIsEditingRenderedHTML(editorElement))
  {
    nodeData = msiGetObjectDataForProperties(editorElement);
    if (nodeData && nodeData.theNode != null)
      element = nodeData.theNode;
  }

  if (element && element.nodeName)
  {
    var objStr = "";
    menuItem.setAttribute("disabled", "");
//    name = element.nodeName.toLowerCase();
    name = msiGetBaseNodeName(element);
    if (name != null)
      name = name.toLowerCase();

    var wrappedChildElement = element;
    while ( (name == 'mstyle') || (name == 'mrow') )
    {
      var newChildElement = msiNavigationUtils.getSingleWrappedChild(wrappedChildElement);
      if (newChildElement == null)
        break;
      wrappedChildElement = newChildElement;
      name = msiGetBaseNodeName(wrappedChildElement).toLowerCase();
    }

    switch (name)
    {
      case "#text":
      break;

      case "img":
        // Check if img is enclosed in link
        //  (use "href" to not be fooled by named anchor)
        try
        {
          if (msiGetEditor(editorElement).getElementOrParentByTagName("href", element))
            objStr = GetString("ImageAndLink");
        } catch(e) {}
        
        if (objStr == "")
          objStr = GetString("Image");
        break;
      case "hr":
        objStr = GetString("HLine");
        break;
      case "table":
        objStr = GetString("Table");
        break;
      case "th":
        name = "td";
      case "td":
        objStr = GetString("TableCell");
        break;
      case "ol":
      case "ul":
      case "dl":
        objStr = GetString("List");
        break;
      case "li":
        objStr = GetString("ListItem");
        break;
      case "form":
        objStr = GetString("Form");
        break;
      case "input":
        var type = element.getAttribute("type");
        if (type && type.toLowerCase() == "image")
          objStr = GetString("InputImage");
        else
          objStr = GetString("InputTag");
        break;
      case "textarea":
        objStr = GetString("TextArea");
        break;
      case "select":
        objStr = GetString("Select");
        break;
      case "button":
        objStr = GetString("Button");
        break;
      case "label":
        objStr = GetString("Label");
        break;
      case "fieldset":
        objStr = GetString("FieldSet");
        break;
      case "a":
        if (element.name)
        {
          objStr = GetString("NamedAnchor");
          name = "anchor";
        }
          else if(wrappedChildElement.href)
        {
          objStr = GetString("Link");
          name = "href";
        }
        break;

      case 'hspace':
        objStr = GetString("HorizontalSpace");
      break;

      case 'vspace':
        objStr = GetString("VerticalSpace");
      break;

      case 'msirule':
        objStr = GetString("Rule");
      break;

      case 'msibreak':
        objStr = GetString("GenBreak");
      break;

      case 'mfrac':
        objStr = GetString("Fraction");
      break;

      case 'mroot':
      case 'msqrt':
        objStr = GetString("Radical");
      break;

      case 'msub':
      case 'msup':
      case 'msubsup':
        if (msiNavigationUtils.getEmbellishedOperator(wrappedChildElement) != null)
          objStr = GetString("Operator");
//        msiGoDoCommandParams("cmd_MSIreviseScriptsCmd", cmdParams, editorElement);
// Should be no Properties dialog available for these cases? SWP has none...
      break;

      case 'mover':
      case 'munder':
      case 'munderover':
        if (msiNavigationUtils.getEmbellishedOperator(wrappedChildElement) != null)
          objStr = GetString("Operator");
        else
          objStr = GetString("Decoration");
      break;

      case 'mmultiscripts':
        if (msiNavigationUtils.getEmbellishedOperator(wrappedChildElement) != null)
          objStr = GetString("Operator");
        else
          objStr = GetString("Tensor");
      break;

      case 'mmatrix':
        objStr = GetString("Matrix");
      break;

      case 'mi':
        if (msiNavigationUtils.isUnit(wrappedChildElement))
          objStr = GetString("Unit");
        else if (msiNavigationUtils.isMathname(wrappedChildElement))
          objStr = GetString("MathName");
      break;

//  commandTable.registerCommand("cmd_MSIreviseSymbolCmd",    msiReviseSymbolCmd);

      case 'mrow':
      case 'mstyle':
        if (msiNavigationUtils.isFence(wrappedChildElement))
        {
          if (msiNavigationUtils.isBinomial(wrappedChildElement))
            objStr = GetString("Binomial");
          else
            objStr = GetString("GenBracket");
        }
      break;

      case 'mo':
        if (msiNavigationUtils.isMathname(wrappedChildElement))
          objStr = GetString("MathName");
        else
          objStr = GetString("Operator");
      break;

    }
    if (objStr)
      menuStr = GetString("ObjectProperties").replace(/%obj%/,objStr);
  }
  else
  {
    // We show generic "Properties" string, but disable menu item
    menuItem.setAttribute("disabled","true");
  }
  menuItem.setAttribute("label", menuStr);
  menuItem.setAttribute("accesskey",GetString("ObjectPropertiesAccessKey"));
  return name;
}

function msiInitParagraphMenu(editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();

  var mixedObj = { value: null };
  var state;
  try {
    state = msiGetEditor(editorElement).getParagraphState(mixedObj);
  }
  catch(e) {}
  var IDSuffix;

  // PROBLEM: When we get blockquote, it masks other styles contained by it
  // We need a separate method to get blockquote state

  // We use "x" as uninitialized paragraph state
  if (!state || state == "x")
    IDSuffix = "bodyText" // No paragraph container
  else
    IDSuffix = state;
  var menuID = "menu_"+IDSuffix;

  // Set "radio" check on one item, but...
  var docList = msiGetUpdatableItemContainers(menuID, editorElement);
  for (var i = 0; i < docList.length; ++i)
  {
//    var menuItem = document.getElementById("menu_"+IDSuffix);
    var menuItem = docList.getElementById(menuID);
    menuItem.setAttribute("checked", "true");

    // ..."bodyText" is returned if mixed selection, so remove checkmark
    if (mixedObj.value)
      menuItem.setAttribute("checked", "false");
  }
}

function msiGetListStateString(editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();

  try {
    var editor = msiGetEditor(editorElement);

    var mixedObj = { value: null };
    var hasOL = { value: false };
    var hasUL = { value: false };
    var hasDL = { value: false };
    editor.getListState(mixedObj, hasOL, hasUL, hasDL);

    if (mixedObj.value)
      return "mixed";
    if (hasOL.value)
      return "ol";
    if (hasUL.value)
      return "ul";

    if (hasDL.value)
    {
      var hasLI = { value: false };
      var hasDT = { value: false };
      var hasDD = { value: false };
      editor.getListItemState(mixedObj, hasLI, hasDT, hasDD);
      if (mixedObj.value)
        return "mixed";
      if (hasLI.value)
        return "li";
      if (hasDT.value)
        return "dt";
      if (hasDD.value)
        return "dd";
    }
  } catch (e) {}

  // return "noList" if we aren't in a list at all
  return "noList";
}

//TO DO: Make this respond to a "goDoCommand(cmd_initListMenu)" with appropriate command handler.
function msiInitListMenu(editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  if (!msiIsHTMLEditor(editorElement))
    return;

  var IDSuffix = msiGetListStateString(editorElement);

  // Set enable state for the "None" menuitem
  msiGoSetCommandEnabled("cmd_removeList", IDSuffix != "noList", editorElement);

  // Set "radio" check on one item, but...
  // we won't find a match if it's "mixed"
  var menuID = "menu_"+IDSuffix;
  var docList = msiGetUpdatableItemContainers(menuID, editorElement);
  for (var i = 0; i < docList.length; ++i)
  {
//    var menuItem = document.getElementById("menu_"+IDSuffix);
    var menuItem = docList[i].getElementById(menuID);
    if (menuItem)
      menuItem.setAttribute("checked", "true");
  }
}

function msiGetAlignmentString(editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();

  var mixedObj = { value: null };
  var alignObj = { value: null };
  try {
    msiGetEditor(editorElement).getAlignment(mixedObj, alignObj);
  } catch (e) {}

  if (mixedObj.value)
    return "mixed";
  if (alignObj.value == nsIHTMLEditor.eLeft)
    return "left";
  if (alignObj.value == nsIHTMLEditor.eCenter)
    return "center";
  if (alignObj.value == nsIHTMLEditor.eRight)
    return "right";
  if (alignObj.value == nsIHTMLEditor.eJustify)
    return "justify";

  // return "left" if we got here
  return "left";
}

//TO DO: Make this respond to a "goDoCommand(cmd_initAlignMenu)" with appropriate command handler.
function msiInitAlignMenu(editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  if (!msiIsHTMLEditor(editorElement))
    return;

  var IDSuffix = msiGetAlignmentString(editorElement);

  // we won't find a match if it's "mixed"
  var menuID = "menu_"+IDSuffix;
  var docList = msiGetUpdatableItemContainers(menuID, editorElement);
  for (var i = 0; i < docList.length; ++i)
  {
//    var menuItem = document.getElementById("menu_"+IDSuffix);
    var menuItem = docList[i].getElementById(menuID);
    if (menuItem)
      menuItem.setAttribute("checked", "true");
  }
}

//function SetDefaultPrefsAndDoctypeForEditor(editor)
function msiEditorSetDefaultPrefsAndDoctype(editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);

  var domdoc;
  try { 
    domdoc = editor.document;
  } catch (e) { dump( e + "\n"); }
  if ( !domdoc )
  {
    dump("SetDefaultPrefsAndDoctypeForEditor: EDITOR DOCUMENT NOT FOUND\n");
    return;
  }

  // Insert a doctype element 
  // if it is missing from existing doc
  if (!domdoc.doctype)
  {
    var newdoctype = domdoc.implementation.createDocumentType("HTML", "-//W3C//DTD HTML 4.01 Transitional//EN","");
    if (newdoctype)
      domdoc.insertBefore(newdoctype, domdoc.firstChild);
  }
  
  // search for head; we'll need this for meta tag additions
  var headelement = 0;
  var headnodelist = domdoc.getElementsByTagName("head");
  if (headnodelist)
  {
    var sz = headnodelist.length;
    if ( sz >= 1 )
      headelement = headnodelist.item(0);
  }
  else
  {
    headelement = domdoc.createElement("head");
    if (headelement)
      domdoc.insertAfter(headelement, domdoc.firstChild);
  }

  /* only set default prefs for new documents */
  var url = msiGetEditorUrl(editorElement);
  if (!IsUrlAboutBlank(url)&&!IsUrlUntitled(url) )
    return;

  // search for author meta tag.
  // if one is found, don't do anything.
  // if not, create one and make it a child of the head tag
  //   and set its content attribute to the value of the editor.author preference.

  var nodelist = domdoc.getElementsByTagName("meta");
  if ( nodelist )
  {
    // we should do charset first since we need to have charset before
    // hitting other 8-bit char in other meta tags
    // grab charset pref and make it the default charset
    var element;
    var prefCharsetString = 0;
    try
    {
      prefCharsetString = gPrefs.getComplexValue("intl.charset.default",
                                                 Components.interfaces.nsIPrefLocalizedString).data;
    }
    catch (ex) {}
    if ( prefCharsetString && prefCharsetString != 0)
    {
        element = domdoc.createElement("meta");
        if ( element )
        {
          element.setAttribute("http-equiv", "content-type");
          element.setAttribute("content", editor.contentsMIMEType + "; charset=" + prefCharsetString);
          headelement.insertBefore( element, headelement.firstChild );
        }
    }

    var node = 0;
    var listlength = nodelist.length;

    // let's start by assuming we have an author in case we don't have the pref
    var authorFound = false;
    for (var i = 0; i < listlength && !authorFound; i++)
    {
      node = nodelist.item(i);
      if ( node )
      {
        var value = node.getAttribute("name");
        if (value && value.toLowerCase() == "author")
        {
          authorFound = true;
        }
      }
    }

    var prefAuthorString = 0;
    try
    {
      prefAuthorString = gPrefs.getComplexValue("editor.author",
                                                Components.interfaces.nsISupportsString).data;
    }
    catch (ex) {}
    if ( prefAuthorString && prefAuthorString != 0)
    {
      if ( !authorFound && headelement)
      {
        /* create meta tag with 2 attributes */
        element = domdoc.createElement("meta");
        if ( element )
        {
          element.setAttribute("name", "author");
          element.setAttribute("content", prefAuthorString);
          headelement.appendChild( element );
        }
      }
    }
  }

  // add title tag if not present
  var titlenodelist = editor.document.getElementsByTagName("title");
  if (headelement && titlenodelist && titlenodelist.length == 0)
  {
     titleElement = domdoc.createElement("title");
     if (titleElement)
       headelement.appendChild(titleElement);
  }

  // Get editor color prefs
  var use_custom_colors = false;
  try {
    use_custom_colors = gPrefs.getBoolPref("editor.use_custom_colors");
  }
  catch (ex) {}

  // find body node
  var bodyelement = msiGetBodyElement(editorElement);
  if (bodyelement)
  {
    if ( use_custom_colors )
    {
      // try to get the default color values.  ignore them if we don't have them.
      var text_color;
      var link_color;
      var active_link_color;
      var followed_link_color;
      var background_color;

      try { text_color = gPrefs.getCharPref("editor.text_color"); } catch (e) {}
      try { link_color = gPrefs.getCharPref("editor.link_color"); } catch (e) {}
      try { active_link_color = gPrefs.getCharPref("editor.active_link_color"); } catch (e) {}
      try { followed_link_color = gPrefs.getCharPref("editor.followed_link_color"); } catch (e) {}
      try { background_color = gPrefs.getCharPref("editor.background_color"); } catch(e) {}

      // add the color attributes to the body tag.
      // and use them for the default text and background colors if not empty
      try {
        if (text_color)
        {
          editor.setAttributeOrEquivalent(bodyelement, "text", text_color, true);
          editorElement.gDefaultTextColor = text_color;
        }
        if (background_color)
        {
          editor.setAttributeOrEquivalent(bodyelement, "bgcolor", background_color, true);
          editorElement.gDefaultBackgroundColor = background_color;
        }

        if (link_color)
          bodyelement.setAttribute("link", link_color);
        if (active_link_color)
          bodyelement.setAttribute("alink", active_link_color);
        if (followed_link_color)
          bodyelement.setAttribute("vlink", followed_link_color);
      } catch (e) {}
    }
    // Default image is independent of Custom colors???
    try {
      var background_image = gPrefs.getCharPref("editor.default_background_image");
      if (background_image)
        editor.setAttributeOrEquivalent(bodyelement, "background", background_image, true);
    } catch (e) {dump("BACKGROUND EXCEPTION: "+e+"\n"); }

  }
  // auto-save???
}

// --------------------------- Logging stuff ---------------------------

function msiEditorGetNodeFromOffsets(offsets, editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();

  var node = null;
  try
  {
    node = msiGetEditor(editorElement).document;

    for (var i = 0; i < offsets.length; i++)
      node = node.childNodes[offsets[i]];
  } catch (e) {}
  return node;
}

function msiEditorSetSelectionFromOffsets(selRanges, editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  try
  {
    var editor = msiGetEditor(editorElement);
    var selection = editor.selection;
    selection.removeAllRanges();

    var rangeArr, start, end, node, offset;
    for (var i = 0; i < selRanges.length; i++)
    {
      rangeArr = selRanges[i];
      start    = rangeArr[0];
      end      = rangeArr[1];

      var range = editor.document.createRange();

      node   = msiEditorGetNodeFromOffsets(start[0], editorElement);
      offset = start[1];

      range.setStart(node, offset);

      node   = msiEditorGetNodeFromOffsets(end[0], editorElement);
      offset = end[1];

      range.setEnd(node, offset);

      selection.addRange(range);
    }
  } catch (e) {}
}

////--------------------------------------------------------------------
//NOTE: The following functions do not need to be overwritten.

function msiInitFontStyleMenu(menuPopup)
{
  for (var i = 0; i < menuPopup.childNodes.length; i++)
  {
    var menuItem = menuPopup.childNodes[i];
    var theStyle = menuItem.getAttribute("state");
    if (theStyle)
    {
      menuItem.setAttribute("checked", theStyle);
    }
  }
}

////--------------------------------------------------------------------
function msiOnButtonUpdate(button, commmandID)
{
  try
  {
    var commandNode = document.getElementById(commmandID);
    if (!commandNode)
    {
      var topWindow = msiGetTopLevelWindow(window);
      commandNode = topWindow.document.getElementById(commandID);
    }  
    var state = commandNode.getAttribute("state");
    button.checked = state == "true";
  }
  catch(exc) {AlertWithTitle("Error in msiEditor.js", "Error in msiOnButtonUpdate: " + exc);}
}

////--------------------------------------------------------------------
function msiOnStateButtonUpdate(button, commmandID, onState)
{
  try
  {
    var commandNode = document.getElementById(commmandID);
    var state = commandNode.getAttribute("state");
    if (!commandNode)
    {
      var topWindow = msiGetTopLevelWindow(window);
      commandNode = topWindow.document.getElementById(commandID);
    }  

    button.checked = state == onState;
  }
  catch(exc) {AlertWithTitle("Error in msiEditor.js", "Error in msiOnStateButtonUpdate: " + exc);}
}

function msiGetColorAndSetColorWell(ColorPickerID, ColorWellID)
{
  try
  {
    var topWindow = msiGetTopLevelWindow(window);
    var theDoc = topWindow.document;
    var colorWell;
    if (ColorWellID)
      colorWell = theDoc.getElementById(ColorWellID);

    var colorPicker = theDoc.getElementById(ColorPickerID);
    if (colorPicker)
    {
      // Extract color from colorPicker and assign to colorWell.
      color = colorPicker.getAttribute("color");

      if (colorWell && color)
      {
        // Use setAttribute so colorwell can be a XUL element, such as button
        colorWell.setAttribute("style", "background-color: " + color);
      }
    }
    return color;
  }
  catch(exc) {AlertWithTitle("Error in msiEditor.js", "Error in msiGetColorAndSetColorWell: " + exc);}
  return "";
}

////-----------------------------------------------------------------------------------
function msiIsSpellCheckerInstalled()
{
  return "@mozilla.org/spellchecker;1" in Components.classes;
}

////-----------------------------------------------------------------------------------
function msiIsFindInstalled()
{
  return "@mozilla.org/embedcomp/rangefind;1" in Components.classes
          && "@mozilla.org/find/find_service;1" in Components.classes;
}

//

//TO DO: Make this respond to a "goDoCommand(cmd_initTableMenu)" with appropriate command handler.
// Command Updating Strategy:
//   Don't update on on selection change, only when menu is displayed,
//   with this "oncreate" hander:
function msiEditorInitTableMenu(editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  try {
    msiInitJoinCellMenuitem("menu_JoinTableCells", editorElement);
  } catch (ex) {}

  // Set enable states for all table commands
  msiGoUpdateTableMenuItems(document.getElementById("composerTableMenuItems"), editorElement);
}

function msiInitJoinCellMenuitem(id, editorElement)
{
  // Change text on the "Join..." item depending if we
  //   are joining selected cells or just cell to right
  // TODO: What to do about normal selection that crosses
  //       table border? Try to figure out all cells
  //       included in the selection?
  var menuText;
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var docList = msiGetUpdatableItemContainers(id, editorElement);

//  var menuItem = document.getElementById(id);
  if (!docList.length)
    return;
//  if (!menuItem) return;

  // Use "Join selected cells if there's more than 1 cell selected
  var numSelected;
  var foundElement;
  
  try {
    var tagNameObj = {};
    var countObj = {value:0}
    foundElement = msiGetTableEditor(editorElement).getSelectedOrParentTableElement(tagNameObj, countObj);
    numSelected = countObj.value;
  }
  catch(e) {}
  if (foundElement && numSelected > 1)
    menuText = GetString("JoinSelectedCells");
  else
    menuText = GetString("JoinCellToRight");

  for (var i = 0; i < docList.length; ++i)
  {
    var menuItem = docList[i].getElementById(id);
    if (menuItem)
    {
      menuItem.setAttribute("label",menuText);
      menuItem.setAttribute("accesskey",GetString("JoinCellAccesskey"));
    }
  }
}

function msiInitRemoveStylesMenuitems(editorElement, removeStylesId, removeLinksId, removeNamedAnchorsId)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);
  if (!editor)
    return;
  var docList = msiGetUpdatableItemContainers(removeStylesId, editorElement);

  // Change wording of menuitems depending on selection
//  var stylesItem = document.getElementById(removeStylesId);
//  var linkItem = document.getElementById(removeLinksId);

  var isCollapsed = editor.selection.isCollapsed;
  for (var i = 0; i < docList.length; ++i)
  {
    var stylesItem = docList[i].getElementById(removeStylesId);
    var linkItem = docList[i].getElementById(removeLinksId);
    if (stylesItem)
    {
      stylesItem.setAttribute("label", isCollapsed ? GetString("StopTextStyles") : GetString("RemoveTextStyles"));
      stylesItem.setAttribute("accesskey", GetString("RemoveTextStylesAccesskey"));
    }
    if (linkItem)
    {
      linkItem.setAttribute("label", isCollapsed ? GetString("StopLinks") : GetString("RemoveLinks"));
      linkItem.setAttribute("accesskey", GetString("RemoveLinksAccesskey"));
      // Note: disabling text style is a pain since there are so many - forget it!

      // Disable if not in a link, but always allow "Remove"
      //  if selection isn't collapsed since we only look at anchor node
      try {
        docList[i].defaultView.SetElementEnabled(linkItem, !isCollapsed ||
                        editor.getElementOrParentByTagName("href", null));
      } catch(e) {}      
    }
    // Disable if selection is collapsed
  }   //end of the docList loop
  msiSetRelevantElementsEnabledById(removeNamedAnchorsId, !isCollapsed);
}

function msiGoUpdateTableMenuItems(commandset, editorElement)
{
  if (!commandset)
    return;

  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var editor = msiGetTableEditor(editorElement);
  if (!editor)
  {
    dump("goUpdateTableMenuItems: too early, not initialized\n");
    return;
  }

  var enabled = false;
  var enabledIfTable = false;

  var flags = editor.flags;
  if (!(flags & nsIPlaintextEditor.eEditorReadonlyMask) &&
      msiIsEditingRenderedHTML(editorElement))
  {
    var tagNameObj = { value: "" };
    var element;
    try {
      element = editor.getSelectedOrParentTableElement(tagNameObj, {value:0});
    }
    catch(e) {}

    if (element)
    {
      // Value when we need to have a selected table or inside a table
      enabledIfTable = true;

      // All others require being inside a cell or selected cell
      enabled = (tagNameObj.value == "td");
    }
  }

  // Loop through command nodes
  for (var i = 0; i < commandset.childNodes.length; i++)
  {
    var commandID = commandset.childNodes[i].getAttribute("id");
//    var docList = msiGetUpdatableItemContainers(commandID, editorElement);
    if (commandID)
    {
      if (commandID == "cmd_InsertTable" ||
          commandID == "cmd_JoinTableCells" ||
          commandID == "cmd_SplitTableCell" ||
          commandID == "cmd_ConvertToTable")
      {
        // Call the update method in the command class
        msiGoUpdateCommand(commandID, editorElement);
      }
      // Directly set with the values calculated here
      else if (commandID == "cmd_DeleteTable" ||
               commandID == "cmd_NormalizeTable" ||
               commandID == "cmd_editTable" ||
               commandID == "cmd_TableOrCellColor" ||
               commandID == "cmd_SelectTable")
      {
        msiGoSetCommandEnabled(commandID, enabledIfTable, editorElement);
      } else {
        msiGoSetCommandEnabled(commandID, enabled, editorElement);
      }
    }
  }
}

//-----------------------------------------------------------------------------------
// Helpers for inserting and editing tables:

function msiIsInTable(editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);
  try {
    var flags = editor.flags;
    return (msiIsHTMLEditor(editorElement) &&
            !(flags & nsIPlaintextEditor.eEditorReadonlyMask) &&
            msiIsEditingRenderedHTML(editorElement) &&
            null != editor.getElementOrParentByTagName("table", null));
  } catch (e) {}
  return false;
}

function msiIsInTableCell(editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  try {
    var editor = msiGetEditor(editorElement);
    var flags = editor.flags;
    return (msiIsHTMLEditor(editorElement) &&
            !(flags & nsIPlaintextEditor.eEditorReadonlyMask) && 
            msiIsEditingRenderedHTML(editorElement) &&
            null != editor.getElementOrParentByTagName("td", null));
  } catch (e) {}
  return false;

}

function msiIsSelectionInOneCell(editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  try {
    var editor = msiGetEditor(editorElement);
    var selection = editor.selection;

    if (selection.rangeCount == 1)
    {
      // We have a "normal" single-range selection
      if (!selection.isCollapsed &&
         selection.anchorNode != selection.focusNode)
      {
        // Check if both nodes are within the same cell
        var anchorCell = editor.getElementOrParentByTagName("td", selection.anchorNode);
        var focusCell = editor.getElementOrParentByTagName("td", selection.focusNode);
        return (focusCell != null && anchorCell != null && (focusCell == anchorCell));
      }
      // Collapsed selection or anchor == focus (thus must be in 1 cell)
      return true;
    }
  } catch (e) {}
  return false;
}

//Later - NEED TO FIX EdTableProps dialog? For now we'll just leave it as is. But we will want our own table dialogs - or perhaps
//  just to revise the existing ones to apply to different editors.
// Call this with insertAllowed = true to allow inserting if not in existing table,
//   else use false to do nothing if not in a table
function msiEditorInsertOrEditTable(insertAllowed, editorElement, command, commandHandler)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  if (msiIsInTable(editorElement))
  {
    // Edit properties of existing table
//    msiOpenModelessDialog("chrome://editor/content/EdTableProps.xul", "_blank", "chrome,close,titlebar,dependent", editorElement,
//                                         command, commandHandler, "", "TablePanel");
    window.openDialog("chrome://editor/content/EdTableProps.xul", "_blank", "chrome,close,titlebar,modal", "","TablePanel");
    editorElement.contentWindow.focus();
  } 
  else if (insertAllowed)
  {
    try {
      if (msiGetEditor(editorElement).selection.isCollapsed)
        // If we have a caret, insert a blank table...
        msiEditorInsertTable(editorElement, command, commandHandler);
      else
        // else convert the selection into a table
        msiGoDoCommand("cmd_ConvertToTable");
    } catch (e) {}
  }
}


function msiEditorInsertTable(editorElement, command, commandHandler)
{
  // Insert a new table
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  if (!msiIsHTMLEditor(editorElement))
    return;

      //HERE USE MODELESS DIALOG FUNCTIONALITY??
//  msiOpenModelessDialog("chrome://editor/content/EdInsertTable.xul", "_blank", "chrome,close,titlebar,dependent", editorElement, 
//                                         command, commandHandler, "")

  window.openDialog("chrome://editor/content/EdInsertTable.xul", "_blank", "chrome,close,titlebar,modal", "");
  editorElement.focus();
}

//Later - NEED TO FIX EdTableProps dialog? For now we'll just leave it as is. But we will want our own table dialogs - or perhaps
//  just to revise the existing ones to apply to different editors.

function msiEditorTableCellProperties(editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  if (!msiIsHTMLEditor(editorElement))
    return;

  try {
    var cell = msiGetEditor(editorElement).getElementOrParentByTagName("td", null);
    if (cell) {
      // Start Table Properties dialog on the "Cell" panel
      //HERE USE MODELESS DIALOG FUNCTIONALITY!
      window.openDialog("chrome://editor/content/EdTableProps.xul", "_blank", "chrome,close,titlebar,modal", "", "CellPanel");
      editorElement.focus();
    }
  } catch (e) {}
}

function msiGetNumberOfContiguousSelectedRows(editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  if (!msiIsHTMLEditor(editorElement))
    return 0;

  var rows = 0;
  try {
    var editor = msiGetTableEditor(editorElement);
    var rowObj = { value: 0 };
    var colObj = { value: 0 };
    var cell = editor.getFirstSelectedCellInTable(rowObj, colObj);
    if (!cell)
      return 0;

    // We have at least one row
    rows++;

    var lastIndex = rowObj.value;
    do {
      cell = editor.getNextSelectedCell({value:0});
      if (cell)
      {
        editor.getCellIndexes(cell, rowObj, colObj);
        var index = rowObj.value;
        if (index == lastIndex + 1)
        {
          lastIndex = index;
          rows++;
        }
      }
    }
    while (cell);
  } catch (e) {}

  return rows;
}

function msiGetNumberOfContiguousSelectedColumns(editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  if (!msiIsHTMLEditor(editorElement))
    return 0;

  var columns = 0;
  try {
    var editor = msiGetTableEditor(editorElement);
    var colObj = { value: 0 };
    var rowObj = { value: 0 };
    var cell = editor.getFirstSelectedCellInTable(rowObj, colObj);
    if (!cell)
      return 0;

    // We have at least one column
    columns++;

    var lastIndex = colObj.value;
    do {
      cell = editor.getNextSelectedCell({value:0});
      if (cell)
      {
        editor.getCellIndexes(cell, rowObj, colObj);
        var index = colObj.value;
        if (index == lastIndex +1)
        {
          lastIndex = index;
          columns++;
        }
      }
    }
    while (cell);
  } catch (e) {}

  return columns;
}

//FIX INSERTCHAR STUFF BELOW. This will just consist of revising the dialog to use a parent editor as per usual...

//No apparent need to reimplement the InsertCharWindow functions at this point. A different mechanism should be used
//for this anyway.
//function SwitchInsertCharToThisWindow(windowWithDialog)
//{
//  if (windowWithDialog && "InsertCharWindow" in windowWithDialog &&
//      windowWithDialog.InsertCharWindow)
//  {
//    // Move dialog association to the current window
//    window.InsertCharWindow = windowWithDialog.InsertCharWindow;
//    windowWithDialog.InsertCharWindow = null;
//
//    // Switch the dialog's opener to current window's
//    window.InsertCharWindow.opener = window;
//
//    // Bring dialog to the forground
//    window.InsertCharWindow.focus();
//    return true;
//  }
//  return false;
//}
//
//function FindEditorWithInsertCharDialog()
//{
//  try {
//    // Find window with an InsertCharsWindow and switch association to this one
//    var windowManager = Components.classes['@mozilla.org/appshell/window-mediator;1'].getService();
//    var windowManagerInterface = windowManager.QueryInterface( Components.interfaces.nsIWindowMediator);
//    var enumerator = windowManagerInterface.getEnumerator( null );
//
//    while ( enumerator.hasMoreElements()  )
//    {
//      var tempWindow = enumerator.getNext();
//
//      if (tempWindow != window && "InsertCharWindow" in tempWindow &&
//          tempWindow.InsertCharWindow)
//      {
//        return tempWindow;
//      }
//    }
//  }
//  catch(e) {}
//  return null;
//}
//
//function msiEditorFindOrCreateInsertCharWindow(editorElement)
//{
//  //THIS SHOULD ALL CHANGE SHORTLY!! rwa
//  if ("InsertCharWindow" in window && window.InsertCharWindow)
//    window.InsertCharWindow.focus();
//  else
//  {
//    // Since we switch the dialog during EditorOnFocus(),
//    //   this should really never be found, but it's good to be sure
//    var windowWithDialog = FindEditorWithInsertCharDialog();
//    if (windowWithDialog)
//    {
//      SwitchInsertCharToThisWindow(windowWithDialog);
//    }
//    else
//    {
//      // The dialog will set window.InsertCharWindow to itself
//      window.openDialog("chrome://editor/content/EdInsertChars.xul", "_blank", "chrome,close,titlebar", "");
//    }
//  }
//}

//// Find another HTML editor window to associate with the InsertChar dialog
////   or close it if none found  (May be a mail composer)
//function SwitchInsertCharToAnotherEditorOrClose()
//{
//  if ("InsertCharWindow" in window && window.InsertCharWindow)
//  {
//    var windowManager = Components.classes['@mozilla.org/appshell/window-mediator;1'].getService();
//    var enumerator;
//    try {
//      var windowManagerInterface = windowManager.QueryInterface( Components.interfaces.nsIWindowMediator);
//      enumerator = windowManagerInterface.getEnumerator( null );
//    }
//    catch(e) {}
//    if (!enumerator) return;
//
//    // TODO: Fix this to search for command controllers and look for "cmd_InsertChars"
//    // For now, detect just Web Composer and HTML Mail Composer
//    while ( enumerator.hasMoreElements()  )
//    {
//      var  tempWindow = enumerator.getNext();
//      if (tempWindow != window && tempWindow != window.InsertCharWindow &&
//          "GetCurrentEditor" in tempWindow && tmpWindow.GetCurrentEditor())
//      {
//        tempWindow.InsertCharWindow = window.InsertCharWindow;
//        window.InsertCharWindow = null;
//        tempWindow.InsertCharWindow.opener = tempWindow;
//        return;
//      }
//    }
//    // Didn't find another editor - close the dialog
//    window.InsertCharWindow.close();
//  }
//}


function msiResetStructToolbar(editorElement)
{
  editorElement.mLastFocusNode = null;
  msiUpdateStructToolbar(editorElement);
}

function newCommandListener(element)
{
  return function() { return msiSelectFocusNodeAncestor(null, element); };
}

function newContextmenuListener(button, element)
{
  return function() { return InitStructBarContextMenu(button, element); };
}


function msiUpdateStructToolbar(editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);
  if (!editor) return;
  if (!editor.tagListManager) return;
  editor.tagListManager.buildParentTagList();

  var mixed = msiGetSelectionContainer(editorElement);
  if (!mixed) return;
  var element = mixed.node;
  var oneElementSelected = mixed.oneElementSelected;

  if (!element) return;

  if (element == editorElement.mLastFocusNode &&
      oneElementSelected == editorElement.mLastFocusNodeWasSelected)
    return;

  editorElement.mLastFocusNode = element;
  editorElement.mLastFocusNodeWasSelected = mixed.oneElementSelected;

  var theDocument = document;
  var toolbar = document.getElementById("structToolbar");
  if (!toolbar)
  {
    try
    {
      theDocument = msiGetTopLevelWindow().document;
      toolbar = theDocument.getElementById("structToolbar");
    }
    catch(exc) { AlertWithTitle("Error in msiUpdateStructToolbar!", exc); }
  }
  if (!toolbar) return;
  var childNodes = toolbar.childNodes;
  var childNodesLength = childNodes.length;
  // We need to leave the <label> to flex the buttons to the left
  // so, don't remove the last child at position length - 1
  for (var i = childNodesLength - 2; i >= 0; i--) {
    toolbar.removeChild(childNodes.item(i));
  }

  toolbar.removeAttribute("label");

  if ( msiIsInHTMLSourceMode(editorElement) ) {
    // we have destroyed the contents of the status bar and are
    // about to recreate it ; but we don't want to do that in
    // Source mode
    return;
  }

  var tag, button;
  var bodyElement = msiGetBodyElement(editorElement);
  var isFocusNode = true;
  var tmp;

  // the theory here is that by following up the chain of parentNodes, 
  // we will eventually get to the root <body> tag. But due to some bug, 
  // there may be multiple <body> elements in the document. 
  do {
    tag = element.nodeName;

    button = theDocument.createElementNS(XUL_NS, "toolbarbutton");
    button.setAttribute("label",   "<" + tag + ">");
    button.setAttribute("value",   tag);
    button.setAttribute("context", "structToolbarContext");
    button.className = "struct-button";

    toolbar.insertBefore(button, toolbar.firstChild);

    button.addEventListener("command", newCommandListener(element), false);

    button.addEventListener("contextmenu", newContextmenuListener(button, element), false);

    if (isFocusNode && oneElementSelected) {
      button.setAttribute("checked", "true");
      isFocusNode = false;
    }

    tmp = element;
    element = element.parentNode;

  } while (element && (tmp != bodyElement) && (tag != "body"));
}

function msiSelectFocusNodeAncestor(editorElement, element)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);
  if (editor) {
    if (element == msiGetBodyElement(editorElement))
      editor.selectAll();
    else
      editor.selectElement(element);
  }
  msiResetStructToolbar(editorElement);
}

function msiGetSelectionContainer(editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);
  if (!editor) return null;

  try {
    var selection = editor.selection;
    if (!selection) return null;
  }
  catch (e) { return null; }

  var result = { oneElementSelected:false };

  if (selection.isCollapsed) {
    result.node = selection.focusNode;
  }
  else {
    var rangeCount = selection.rangeCount;
    if (rangeCount == 1) {
      result.node = editor.getSelectedElement("");
      var range = selection.getRangeAt(0);

      // check for a weird case : when we select a piece of text inside
      // a text node and apply an inline style to it, the selection starts
      // at the end of the text node preceding the style and ends after the
      // last char of the style. Assume the style element is selected for
      // user's pleasure
      if (!result.node &&
          range.startContainer.nodeType == Node.TEXT_NODE &&
          range.startOffset == range.startContainer.length &&
          range.endContainer.nodeType == Node.TEXT_NODE &&
          range.endOffset == range.endContainer.length &&
          range.endContainer.nextSibling == null &&
          range.startContainer.nextSibling == range.endContainer.parentNode)
        result.node = range.endContainer.parentNode;

      if (!result.node) {
        // let's rely on the common ancestor of the selection
        result.node = range.commonAncestorContainer;
      }
      else {
        result.oneElementSelected = true;
      }
    }
    else {
      // assume table cells !
      var i, container = null;
      for (i = 0; i < rangeCount; i++) {
        range = selection.getRangeAt(i);
        if (!container) {
          container = range.startContainer;
        }
        else if (container != range.startContainer) {
          // all table cells don't belong to same row so let's
          // select the parent of all rows
          result.node = container.parentNode;
          break;
        }
        result.node = container;
      }
    }
  }

  // make sure we have an element here
  while (result.node.nodeType != Node.ELEMENT_NODE)
    result.node = result.node.parentNode;

  // and make sure the element is not a special editor node like
  // the <br> we insert in blank lines
  // and don't select anonymous content !!! (fix for bug 190279)
  while (result.node.hasAttribute("_moz_editor_bogus_node") ||
         editor.isAnonymousElement(result.node))
    result.node = result.node.parentNode;

  return result;
}

function FillInHTMLTooltip(tooltip)
{
  const XLinkNS = "http://www.w3.org/1999/xlink";
  var tooltipText = null;
  var editorElement = msiGetActiveEditorElement();
  if (msiGetEditorDisplayMode(editorElement) == kDisplayModePreview)
  {
    for (var node = document.tooltipNode; node; node = node.parentNode)
    {
      if (node.nodeType == Node.ELEMENT_NODE)
      {
        tooltipText = node.getAttributeNS(XLinkNS, "title");
        if (tooltipText && /\S/.test(tooltipText))
        {
          tooltip.setAttribute("label", tooltipText);
          return true;
        }
        tooltipText = node.getAttribute("title");
        if (tooltipText && /\S/.test(tooltipText))
        {
          tooltip.setAttribute("label", tooltipText);
          return true;
        }
      }
    }
  }
  else
  {
    for (var node = document.tooltipNode; node; node = node.parentNode)
    {
      if (node instanceof Components.interfaces.nsIDOMHTMLImageElement ||
                 node instanceof Components.interfaces.nsIDOMHTMLInputElement)
        tooltipText = node.getAttribute("src");
      else if (node instanceof Components.interfaces.nsIDOMHTMLAnchorElement)
        tooltipText = node.getAttribute("href") || node.name;
      if (tooltipText)
      {
        tooltip.setAttribute("label", tooltipText);
        return true;
      }
    }
  }
  return false;
}



//function initializeAutoCompleteStringArrayForEditor(theEditor)
//{ 
//  dump("===> initializeAutoCompleteStringArray\n");
//                              
////  var stringArraySearch = Components.classes["@mozilla.org/autocomplete/search;1?name=stringarray"].getService(Components.interfaces.nsIAutoCompleteSearchStringArray);
////  stringArraySearch.editor = theEditor;
//}
 
 
 
// handle events on prince-specific elements here, or call the default goDoCommand() 
function goDoPrinceCommand (cmdstr, element, editorElement) 
{ dump ("SMR msiEditor.js got doPrinceCommand " +  element.localName + " " + element.getAttribute("msigraph") + "\n");
  try
  {
    if (!editorElement)
      editorElement = findEditorElementForDocument(element.ownerDocument);

    var elementName = element.localName;
    if (elementName == "object")
      elementName = element.parentNode.localName;
    if (elementName == "notewrapper")
    {
      element = element.getElementsByTagName("note")[0];
      if (element) elementName = element.localName;
    }
    if (elementName == "note")
    {
      msiNote(element,editorElement);
    }
    if (elementName == "msiframe")
    {
      msiFrame(element,editorElement);
    }
    else if ((elementName == "img") || (elementName=="graph"))
    {
      var bIsGraph = (element.getAttribute("msigraph") == "true");
      if (!bIsGraph)
      {
        for (var ix = 0; !bIsGraph && (ix < element.childNodes.length); ++ix)
        {
          if (element.childNodes[ix].getAttribute("msigraph") == "true")
            bIsGraph = true;
        }
      }
      if (bIsGraph)
      {
//        dump("In goDoPrinceCommand, bIsGraph is true.\n");
        var theWindow = window;
        if (!("graphClickEvent" in theWindow))
          theWindow = msiGetTopLevelWindow(window);
        theWindow.graphClickEvent(cmdstr, editorElement);
      }
//      else
//        dump("In goDoPrinceCommand, bIsGraph is false.\n");
    }
    else if ((element.localName == "object") && (element.getAttribute("msigraph") == "true"))
    {      dump ("SMR msiEditor.js got double click on msigraph object\n");
      var theWindow = window;
      if (!("graphClickEvent" in theWindow))
        theWindow = msiGetTopLevelWindow(window);
      theWindow.graphObjectClickEvent(cmdstr, editorElement);
    }
    else 
    {
//      dump("In goDoPrinceCommand, elementName is [" + elementName + "].\n");
      msiGoDoCommand(cmdstr, editorElement);
    }
  }
  catch(exc) {AlertWithTitle("Error in msiEditor.js", "Error in goDoPrinceCommand: " + exc);}
}


//
// Command Updater functions
//
//function msiGoUpdateCommand(command, editorElement)
//{
//  if (!editorElement)
//    editorElement = msiGetActiveEditorElement();
//
//  try {
//    var bLogIt = (command == "cmd_MSImathtext");
//    if (bLogIt)
//    {
//      var bStopHere = true;
//      goUpdateCommand(command);
////      return;
//    }
//
//    var controller = null;
//    var bControllerFromTop = false;
//    if (editorElement)
//      controller = msiGetControllerForCommand(command, editorElement);
//    if (!controller)
//    {
//      controller = top.document.commandDispatcher.getControllerForCommand(command);
//      bControllerFromTop = true;
//    }
//
//    var enabled = false;
//
//    if ( controller )
//      enabled = controller.isCommandEnabled(command);
//
//    if (bLogIt)
//    {
//      var logStr = "In msiGoUpdateCommand for cmd_MSImathtext; editorElement is [";
//      if (editorElement)
//        logStr += editorElement.id;
//      logStr += "], and controller is ";
//      if (!controller)
//        logStr += "not found.\n";
//      else if (bControllerFromTop)
//        logStr += "found from top.document.commandDispatcher.\n";
//      else
//        logStr += "found from msiGetControllerForCommand for the editorElement.\n";
//      msiKludgeLogString(logStr);
//    }
//
//    msiGoSetCommandEnabled(command, enabled, editorElement);
//  }
//  catch (e) {
//    dump("An error occurred updating the "+command+" command\n");
//  }
//}

//function msiGoDoCommand(command, editorElement)
//{
//  if (!editorElement)
//    editorElement = msiGetActiveEditorElement();
//  try {
//    var controller = null;
//    if (editorElement)
//      controller = msiGetControllerForCommand(command, editorElement);
//    if (!controller)
//      controller = top.document.commandDispatcher.getControllerForCommand(command);
//    if ( controller && controller.isCommandEnabled(command))
//      controller.doCommand(command);
//  }
//  catch (e) {
//    dump("An error occurred executing the "+command+" command\n");
//  }
//}


//function msiGoSetCommandEnabled(id, enabled, editorElement)
//{
//  if (!editorElement)
//    editorElement = msiGetActiveEditorElement();
//  var docList = msiGetUpdatableItemContainers(id, editorElement);
//
//  for (var i = 0; i < docList.length; ++i)
//  {
//    var node = docList[i].getElementById(id);
//    if ( node )
//    {
//      if ( enabled )
//        node.removeAttribute("disabled");
//      else
//        node.setAttribute('disabled', 'true');
//    }
//  }
//}

function msiDoUpdateCommands(eventStr, editorElement)
{
  var theWindow = msiGetTopLevelWindow();
  theWindow.updateCommands(eventStr);
  if (theWindow != window)
    window.updateCommands(eventStr);
}

//function msiGoSetMenuValue(command, labelAttribute, editorElement)
//{
//  if (!editorElement)
//    editorElement = msiGetActiveEditorElement();
//  var docList = msiGetUpdatableItemContainers(command, editorElement);
//
//  for (var i = 0; i < docList.length; ++i)
//  {
////    var commandNode = top.document.getElementById(command);
//    var commandNode = docList[i].getElementById(command);
//    if ( commandNode )
//    {
//      var label = commandNode.getAttribute(labelAttribute);
//      if ( label )
//        commandNode.setAttribute('label', label);
//    }
//  }
//}

//function msiGoSetAccessKey(command, valueAttribute, editorElement)
//{
//  if (!editorElement)
//    editorElement = msiGetActiveEditorElement();
//  var docList = msiGetUpdatableItemContainers(command, editorElement);
//
//  for (var i = 0; i < docList.length; ++i)
//  {
////    var commandNode = top.document.getElementById(command);
//    var commandNode = docList[i].getElementById(command);
//    if ( commandNode )
//    {
//      var value = commandNode.getAttribute(valueAttribute);
//      if ( value )
//        commandNode.setAttribute('accesskey', value);
//    }
//  }
//}

/**
 * Command Updater
 */
var msiCommandUpdater = {
  /**
   * Gets a controller that can handle a particular command. 
   * @param   command
   *          A command to locate a controller for, preferring controllers that
   *          show the command as enabled.
   * @returns In this order of precedence:
   *            - the first controller supporting the specified command 
   *              associated with the focused element that advertises the
   *              command as ENABLED
   *            - the first controller supporting the specified command 
   *              associated with the global window that advertises the
   *              command as ENABLED
   *            - the first controller supporting the specified command
   *              associated with the focused element
   *            - the first controller supporting the specified command
   *              associated with the global window
   */
  _getControllerForCommand: function(command, editorElement) {
    try {

//      var bLogIt = (command == "cmd_MSImathtext");
//      if (bLogIt)
//      {
//        var bStopHere = true;
//        goUpdateCommand(command);
//  //      return;
//      }

      var controller = null;
      var bControllerFromTop = false;
      if (editorElement)
        controller = msiGetControllerForCommand(command, editorElement);
      if (!controller)
      {
        controller = top.document.commandDispatcher.getControllerForCommand(command);
        bControllerFromTop = true;
      }

      var bIsEnabled = false;
      if (controller)
      {
        try
        {
          bIsEnabled = controller.isCommandEnabled(command);
        } catch(exc) {dump("Error in msiEditor.js, in msiCommandUpdater._getControllerForCommand, command is [" + command + "], error is [" + exc + "].\n");}
      }
      if (bIsEnabled)
        return controller;
    }
    catch(e) {
    }
    var controllerCount = window.controllers.getControllerCount();
    for (var i = 0; i < controllerCount; ++i) {
      var current = window.controllers.getControllerAt(i);
      try {
        if (current.supportsCommand(command) && current.isCommandEnabled(command))
          return current;
      }
      catch (e) {
        var dumpingStr = "Error in msiEditor.js, in msiCommandUpdater._getControllerForCommand, command is [" + command + "], controller is [";
        if (current != null)
          dumpingStr += "non-null";
        dumpingStr += "], error is [" + e + "].\n";
        dump(dumpingStr);
      }
    }
    return controller || window.controllers.getControllerForCommand(command);
  },
  
  /**
   * Updates the state of a XUL <command> element for the specified command
   * depending on its state.
   * @param   command
   *          The name of the command to update the XUL <command> element for
   */
  updateCommand: function(command, editorElement) {
    if (!editorElement)
      editorElement = msiGetActiveEditorElement();
    var controller = this._getControllerForCommand(command, editorElement);
    if (!controller)
      return;
    try {
      this.enableCommand(command, controller.isCommandEnabled(command, editorElement), editorElement);
    }
    catch (e) {
      dump("Error in msiEditor.js, in msiCommandUpdater.updateCommand, command is [" + command + "]");
    }
  },
  
  /**
   * Enables or disables a XUL <command> element.
   * @param   command
   *          The name of the command to enable or disable
   * @param   enabled
   *          true if the command should be enabled, false otherwise.
   */
  enableCommand: function(command, enabled, editorElement) {
    if (!editorElement)
      editorElement = msiGetActiveEditorElement();
    var docList = msiGetUpdatableItemContainers(command, editorElement);

    for (var i = 0; i < docList.length; ++i)
    {
      var node = docList[i].getElementById(command);
      if ( node )
      {
        if ( enabled )
          node.removeAttribute("disabled");
        else
          node.setAttribute('disabled', 'true');
      }
    }
//    var element = document.getElementById(command);
//    if (!element)
//      return;
//    if (enabled)
//      element.removeAttribute("disabled");
//    else
//      element.setAttribute("disabled", "true");
  },

  /**
   * Performs the action associated with a specified command using the most
   * relevant controller.
   * @param   command
   *          The command to perform.
   */
  doCommand: function(command, editorElement) {
    if (!editorElement)
      editorElement = msiGetActiveEditorElement();
    try {
//      if ( controller && controller.isCommandEnabled(command))
//        controller.doCommand(command);
      var controller = this._getControllerForCommand(command, editorElement);
      if (!controller)
        return;
      controller.doCommand(command);
    }
    catch (e) {
      dump("An error occurred executing the "+command+" command\n");
    }
  }, 
  
  /**
   * Changes the label attribute for the specified command.
   * @param   command
   *          The command to update.
   * @param   labelAttribute
   *          The label value to use.
   */
  setMenuValue: function(command, labelAttribute, editorElement) {
    if (!editorElement)
      editorElement = msiGetActiveEditorElement();
    var docList = msiGetUpdatableItemContainers(command, editorElement);

    for (var i = 0; i < docList.length; ++i)
    {
      var commandNode = docList[i].getElementById(command);
      if ( commandNode )
      {
        var label = commandNode.getAttribute(labelAttribute);
        if ( label )
          commandNode.setAttribute('label', label);
      }
    }
  },
  
  /**
   * Changes the accesskey attribute for the specified command.
   * @param   command
   *          The command to update.
   * @param   valueAttribute
   *          The value attribute to use.
   */
  setAccessKey: function(command, valueAttribute) {
    if (!editorElement)
      editorElement = msiGetActiveEditorElement();
    var docList = msiGetUpdatableItemContainers(command, editorElement);

    for (var i = 0; i < docList.length; ++i)
    {
      var commandNode = docList[i].getElementById(command);
      if ( commandNode )
      {
        var value = commandNode.getAttribute(valueAttribute);
        if ( value )
          commandNode.setAttribute('accesskey', value);
      }
    }
  },

  /**
   * Inform all the controllers attached to a node that an event has occurred
   * (e.g. the tree controllers need to be informed of blur events so that they can change some of the
   * menu items back to their default values)
   * @param   node
   *          The node receiving the event
   * @param   event
   *          The event.
   */
  onEvent: function(node, event) {
    var numControllers = node.controllers.getControllerCount();
    var controller;

    for ( var controllerIndex = 0; controllerIndex < numControllers; controllerIndex++ )
    {
      controller = node.controllers.getControllerAt(controllerIndex);
      if ( controller )
        controller.onEvent(event);
    }
  }
  
//  doReviseCommand: function(command, editorElement) {
//    if (!editorElement)
//      editorElement = msiGetActiveEditorElement();
//    try {
////      if ( controller && controller.isCommandEnabled(command))
////        controller.doCommand(command);
//      var controller = this._getControllerForCommand(command, editorElement);
//      if (!controller)
//        return;
//      controller.bRevising = true;
//      controller.doCommand(command);
//      controller.bRevising = false;
//    }
//    catch (e) {
//      dump("An error occurred executing the "+command+" command\n");
//    }
//  }  
};
// Shim for compatibility with existing code. 
function msiGoDoCommand(command, editorElement) { msiCommandUpdater.doCommand(command, editorElement); }
function msiGoUpdateCommand(command, editorElement) { msiCommandUpdater.updateCommand(command, editorElement); }
function msiGoSetCommandEnabled(command, enabled, editorElement) { msiCommandUpdater.enableCommand(command, enabled, editorElement); }
function msiGoSetMenuValue(command, labelAttribute, editorElement) { msiCommandUpdater.setMenuValue(command, labelAttribute, editorElement); }
function msiGoSetAccessKey(command, valueAttribute, editorElement) { msiCommandUpdater.setAccessKey(command, valueAttribute, editorElement); }
function msiGoOnEvent(node, event) { msiCommandUpdater.onEvent(node, event); }
//function msiGoDoReviseCommand(command, editorElement) { msiCommandUpdate.doReviseCommand(command, editorElement); }



//// this function is used to inform all the controllers attached to a node that an event has occurred
//// (e.g. the tree controllers need to be informed of blur events so that they can change some of the
//// menu items back to their default values)
//function goOnEvent(node, event)
//{
//  var numControllers = node.controllers.getControllerCount();
//  var controller;
//
//  for ( var controllerIndex = 0; controllerIndex < numControllers; controllerIndex++ )
//  {
//    controller = node.controllers.getControllerAt(controllerIndex);
//    if ( controller )
//      controller.onEvent(event);
//  }
//}


function msiDialogEditorContentFilter(anEditorElement)
{
  this.reject = 0;
  this.accept = 1;
  this.skip = 2;
  this.acceptAll = 3;
  this.mEditorElement = anEditorElement;
  this.mAtomService = Components.classes["@mozilla.org/atom-service;1"].getService(Components.interfaces.nsIAtomService);
  this.mXmlSerializer = new XMLSerializer();
  this.mDOMUtils = Components.classes["@mozilla.org/inspector/dom-utils;1"].createInstance(Components.interfaces.inIDOMUtils);

  this.dlgNodeFilter = function(aNode)
  {
    var nodename = aNode.nodeName;
    var namespaceAtom = null;
    if (aNode.namespaceURI != null)
      namespaceAtom = this.mAtomService.getAtom(aNode.namespaceURI);
    var editor = msiGetEditor(this.mEditorElement);
    if (editor==null)
    {
      dump("Null editor in msiDialogEditorContentFilter.dlgNodeFilter for editorElement " + this.mEditorElement.id + ".\n");
      return this.acceptAll;
    }
    if (editor.tagListManager)
    {
      var isHidden = editor.tagListManager.getStringPropertyForTag(nodename, namespaceAtom, "hidden");
      if (isHidden != null && isHidden=="1")
        return this.skip;
    }
    switch(aNode.nodeType)
    {
      case nsIDOMNode.TEXT_NODE:
        if (this.mDOMUtils.isIgnorableWhitespace(aNode))
          return this.reject;
        else
          return this.acceptAll;
      break;
    }
    switch(aNode.nodeName)
    {
      case "dialogbase":
      case "sw:dialogbase":
        return this.skip;
      break;
      case "mi":
        if (aNode.hasAttribute("tempinput") && (aNode.getAttribute("tempinput")=="true") )
          return this.reject;
      break;
    }
    return this.acceptAll;  
    //We still need to fill in the tags for which we want to accept the tag but leave open the possibility of not accepting a child.
    //Examples may include field tags, list tags, etc.; the point being that one may occur as the parent of something like a
    //  sw:dialogbase paragraph. Not implemented that way at this point.  rwa, 8-
  };
  this.getXMLNodesForParent = function(newParent, parentNode)
  {
    for (var ix = 0; ix < parentNode.childNodes.length; ++ix)
    {
      switch( this.dlgNodeFilter(parentNode.childNodes[ix]) )
      {
        case this.acceptAll:
          newParent.appendChild( parentNode.childNodes[ix].cloneNode(true) );
        break;
        case this.skip:
          this.getXMLNodesForParent( newParent, parentNode.childNodes[ix] );
        break;
        case this.accept:
        {
          var aNewNode = parentNode.childNodes[ix].cloneNode(false);
          this.getXMLNodesForParent( aNewNode, parentNode.childNodes[ix] );
          newParent.appendChild( aNewNode );
        }
        break;
        case this.reject:
        break;
      }
    }
  };
  this.getXMLNodesAsDocFragment = function()
  {
    var docFragment = null;
    var doc = this.mEditorElement.contentDocument;
    var editor = msiGetEditor(this.mEditorElement);
    if (doc != null)
    {
      docFragment = doc.createDocumentFragment();
      var rootNode = msiGetRealBodyElement(doc);
      this.getXMLNodesForParent( docFragment, rootNode );
    }
    this.checkForTrailingBreak(docFragment);
//    var dumpStr = "In msiDialogEditorContentFilter.getXMLNodes, returning a docFragment containing: [";
//    for (var ix = 0; ix < docFragment.childNodes.length; ++ix)
//      dumpStr += this.mXmlSerializer.serializeToString(docFragment.childNodes[ix]);
//    dump(dumpStr + "] for editorElement [" + this.mEditorElement.id + "].\n");
    return docFragment;
  };
  this.nodeHasRealContent = function(parentElement, bIsLast)
  {
    var bFoundContent = false;
    if (parentElement != null)
    {
      for (var ix = 0; (!bFoundContent) && (ix < parentElement.childNodes.length); ++ix)
      {
        switch( this.dlgNodeFilter(parentElement.childNodes[ix]) )
        {
          case this.skip:
            bFoundContent = this.nodeHasRealContent( parentElement.childNodes[ix], (bIsLast && (ix==parentElement.childNodes.length - 1)) );
          break;
          case this.acceptAll:
          case this.accept:
            if (bIsLast && (ix == parentElement.childNodes.length - 1))
              bFoundContent = (parentElement.childNodes[ix].nodeName != "br");
            else
              bFoundContent = true;
            break;
          case this.reject:
            break;
        }
      }
//      return bFoundContent;
    }
    return bFoundContent;
  };
  this.isNonEmpty = function()
  {
    var parentElement = null;
    var doc = this.mEditorElement.contentDocument;
    if (doc != null)
      parentElement = msiGetRealBodyElement(doc);
    return this.nodeHasRealContent( parentElement, true );
  };
  this.checkForTrailingBreak = function(parentNode)
  {
    var lastNode = parentNode.lastChild;
    if (lastNode != null && lastNode != parentNode)
    {
      if (lastNode.nodeName == "br")
        parentNode.removeChild(lastNode);
      else if (lastNode.nodeType == nsIDOMNode.TEXT_NODE)
      {
        if (this.mDOMUtils.isIgnorableWhitespace(lastNode))
          parentNode.removeChild(lastNode);
      }
      else
        this.checkForTrailingBreak(lastNode);
    }
  };
  this.hasNonEmptyMathContent = function()
  {
    var retval = false;
    var editor = msiGetEditor(this.mEditorElement);
    if (editor != null)
    {
      var mathNodes = editor.document.getElementsByTagName("math");
      for (var ix = 0; (!retval) && (ix < mathNodes.length); ++ix)
      {
        var bIsLast = false;
        var parent = mathNodes[ix].parentNode;
        if ((parent != null) && (parent.childNodes != null) && (parent.childNodes.length > 0))
          bIsLast = (parent.childNodes[parent.childNodes.length - 1] == mathNodes[ix]);
        retval = this.nodeHasRealContent(mathNodes[ix], bIsLast);
      }
    }
    return retval;
  };
  this.hasNonEmptyContent = function(bMathOnly)
  {
    if (bMathOnly)
      return this.hasNonEmptyMathContent();
    return this.isNonEmpty();
  };
  this.getContentsAsRange = function()
  {
    var theRange = null;
    var doc = null;
    var editor = msiGetEditor(this.mEditorElement);
    if (editor != null)
      doc = editor.document;
    if (doc != null)
    {
      var rootNode = msiGetRealBodyElement(doc);
      var initialParaNode = null;
      var initialParaList = rootNode.getElementsByTagNameNS("sw", "dialogbase");
      if (initialParaList.length == 0)
        initialParaList = rootNode.getElementsByTagName("sw:dialogbase");
      if (initialParaList.length == 0)
        initialParaList = rootNode.getElementsByTagName("dialogbase");
      if (initialParaList.length > 0)
        initialParaNode = initialParaList[0];
      else
        initialParaNode = rootNode.childNodes[0];
      var startNode = null;
      if (initialParaNode.childNodes.length)
        startNode = initialParaNode.childNodes[0];
      var docRangeObj = doc.QueryInterface(Components.interfaces.nsIDOMDocumentRange);
      theRange = docRangeObj.createRange();
      theRange.setStart(startNode, 0);
      var lastNode = rootNode.childNodes[rootNode.childNodes.length - 1];
      var trailingBreak = this.findTrailingWhiteSpace(rootNode);
      if (trailingBreak != null)
        theRange.setEndBefore(trailingBreak);
      else
        theRange.setEndAfter(lastNode);
    }
    if (theRange != null)
    {
//      dump("In msiEdReplace.js, msiDialogEditorContentFilter.getContentsAsRange for editor element [" + this.mEditorElement.id + "], start of range is at [" + theRange.startContainer.nodeName + ", " + theRange.startOffset + "], while end is at [" + theRange.endContainer.nodeName + ", " + theRange.endOffset + "].\n");
      var topChildrenStr = "";
      if (rootNode.childNodes.length > 0)
        topChildrenStr = rootNode.childNodes[0].nodeName;
      for (var ix = 1; ix < rootNode.childNodes.length; ++ix)
        topChildrenStr += ", " + rootNode.childNodes[ix].nodeName;
//      dump("  [Note: the rootNode is [" + rootNode.nodeName + "], with child nodes [" + topChildrenStr + "].]\n");
    }
    return theRange;
  };
  this.findTrailingWhiteSpace = function(parentNode)
  {
    var spaceNode = null;
    if (parentNode.nodeName == "br")
      spaceNode = parentNode;
    else if (parentNode.nodeType == nsIDOMNode.TEXT_NODE)
    {
      if (this.mDOMUtils.isIgnorableWhitespace(parentNode))
        spaceNode = parentNode;
    }
    else if (parentNode.childNodes && parentNode.childNodes.length > 0)
    {
      var lastNode = parentNode.childNodes[parentNode.childNodes.length - 1];
      if (lastNode != parentNode)
        spaceNode = this.findTrailingWhiteSpace(lastNode);
    }
    return spaceNode;
  };
  this.getContentsAsDocumentFragment = function()
  {
    var theFragment = null;
    var theRange = this.getContentsAsRange();
    if (theRange != null)
    {
      theFragment = theRange.cloneContents();
      theFragment.normalize();
    }
    return theFragment;
  };
  this.getDocumentFragmentString = function()
  {
    var theString = "";
    var theFragment = this.getContentsAsDocumentFragment();
    if (theFragment != null)
    {
      for (var ix = 0; ix < theFragment.childNodes.length; ++ix)
        theString += this.mXmlSerializer.serializeToString(theFragment.childNodes[ix]);
    }
//    dump("In msiDialogEditorContentFilter.getDocumentFragmentString, returning [" + theString + "] for editorElement [" + this.mEditorElement.id + "].\n");
    return theString;
  };
  this.getMarkupString = function()
  {
    var theString = "";
    var docFragment = this.getXMLNodesAsDocFragment();
    if (docFragment != null)
    {
      for (var ix = 0; ix < docFragment.childNodes.length; ++ix)
        theString += this.mXmlSerializer.serializeToString(docFragment.childNodes[ix]);
    }
//    dump("In msiDialogEditorContentFilter.getMarkupString, returning [" + theString + "] for editorElement [" + this.mEditorElement.id + "].\n");
    return theString;
  };
  this.getTextString = function()
  {
    var theFragment = this.getContentsAsDocumentFragment();
    if (theFragment != null)
    {
//      dump("In msiDialogEditorContentFilter.getTextString, returning [" + theFragment.textContent + "] for editorElement [" + this.mEditorElement.id + "].\n");
      return theFragment.textContent;
    }
//    dump("In msiDialogEditorContentFilter.getTextString, returning [] for editorElement [" + this.mEditorElement.id + "].\n");
    return "";
  };
  this.hasTextContent = function()
  {
    var str = this.getTextString();
    return ( (str != null) && (str.length > 0) );
  };
}


function OpenExtensions(aOpenMode)
{
  const EMTYPE = "Extension:Manager";
  
  var wm = Components.classes["@mozilla.org/appshell/window-mediator;1"]
                     .getService(Components.interfaces.nsIWindowMediator);
  var needToOpen = true;
  var windowType = EMTYPE + "-" + aOpenMode;
  var windows = wm.getEnumerator(windowType);
  while (windows.hasMoreElements()) {
    var theEM = windows.getNext().QueryInterface(Components.interfaces.nsIDOMWindowInternal);
    if (theEM.document.documentElement.getAttribute("windowtype") == windowType) {
      theEM.focus();
      needToOpen = false;
      break;
    }
  }

  if (needToOpen) {
    const EMURL = "chrome://mozapps/content/extensions/extensions.xul?type=" + aOpenMode;
    const EMFEATURES = "chrome,dialog=no,resizable";
    window.openDialog(EMURL, "", EMFEATURES);
  }
}
