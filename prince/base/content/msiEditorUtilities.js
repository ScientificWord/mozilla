// Copyright (c) 2006 MacKichan Software, Inc.  All Rights Reserved.


///**** NAMESPACES ****/
//const XUL_NS = "http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul";
//
//// Each editor window must include this file
//// Variables  shared by all dialogs:
//
//// Object to attach commonly-used widgets (all dialogs should use this)
//var gDialog = {};
//
//// Bummer! Can't get at enums from nsIDocumentEncoder.h
//// http://lxr.mozilla.org/seamonkey/source/content/base/public/nsIDocumentEncoder.h#111
var gStringBundle;
var gIOService;
var gPrefsService;
var gPrefsBranch;
//var gFilePickerDirectory;
//
var gOS = "";
//const msigWin = "Win";
//const msigUNIX = "UNIX";
//const msigMac = "Mac";
//
//const kWebComposerWindowID = "editorWindow";
//const kMailComposerWindowID = "msgcomposeWindow";
//
//var gIsHTMLEditor;
///************* Message dialogs ***************/
//
function AlertWithTitle(title, message, parentWindow)
{
  if (!parentWindow)
    parentWindow = window;

  var promptService = Components.classes["@mozilla.org/embedcomp/prompt-service;1"].getService();
  promptService = promptService.QueryInterface(Components.interfaces.nsIPromptService);

  if (promptService)
  {
    if (!title)
      title = GetString("Alert");

    // "window" is the calling dialog window
    promptService.alert(parentWindow, title, message);
  }
}

//// Optional: Caller may supply text to substitue for "Ok" and/or "Cancel"
//function ConfirmWithTitle(title, message, okButtonText, cancelButtonText)
//{
//  var promptService = Components.classes["@mozilla.org/embedcomp/prompt-service;1"].getService();
//  promptService = promptService.QueryInterface(Components.interfaces.nsIPromptService);
//
//  if (promptService)
//  {
//    var okFlag = okButtonText ? promptService.BUTTON_TITLE_IS_STRING : promptService.BUTTON_TITLE_OK;
//    var cancelFlag = cancelButtonText ? promptService.BUTTON_TITLE_IS_STRING : promptService.BUTTON_TITLE_CANCEL;
//
//    return promptService.confirmEx(window, title, message,
//                            (okFlag * promptService.BUTTON_POS_0) +
//                            (cancelFlag * promptService.BUTTON_POS_1),
//                            okButtonText, cancelButtonText, null, null, {value:0}) == 0;
//  }
//  return false;
//}
//
///************* String Utilities ***************/
//
function GetString(name)
{
  if (!gStringBundle)
  {
    try {
      var strBundleService =
          Components.classes["@mozilla.org/intl/stringbundle;1"].getService(); 
      strBundleService = 
          strBundleService.QueryInterface(Components.interfaces.nsIStringBundleService);

      gStringBundle = strBundleService.createBundle("chrome://prince/locale/editor.properties"); 

    } catch (ex) {}
  }
  if (gStringBundle)
  {
    try {
      return gStringBundle.GetStringFromName(name);
    } catch (e) {}
  }
  return null;
}

function TrimStringLeft(string)
{
  if(!string) return "";
  return string.replace(/^\s+/, "");
}

function TrimStringRight(string)
{
  if (!string) return "";
  return string.replace(/\s+$/, '');
}

// Remove whitespace from both ends of a string
function TrimString(string)
{
  if (!string) return "";
  return string.replace(/(^\s+)|(\s+$)/g, '')
}

function IsWhitespace(string)
{
  return /^\s/.test(string);
}

function TruncateStringAtWordEnd(string, maxLength, addEllipses)
{
  // Return empty if string is null, undefined, or the empty string
  if (!string)
    return "";

  // We assume they probably don't want whitespace at the beginning
  string = string.replace(/^\s+/, '');
  if (string.length <= maxLength)
    return string;

  // We need to truncate the string to maxLength or fewer chars
  if (addEllipses)
    maxLength -= 3;
  string = string.replace(RegExp("(.{0," + maxLength + "})\\s.*"), "$1")

  if (string.length > maxLength)
    string = string.slice(0, maxLength);

  if (addEllipses)
    string += "...";
  return string;
}

// Replace all whitespace characters with supplied character
// E.g.: Use charReplace = " ", to "unwrap" the string by removing line-end chars
//       Use charReplace = "_" when you don't want spaces (like in a URL)
function ReplaceWhitespace(string, charReplace)
{
  return string.replace(/(^\s+)|(\s+$)/g,'').replace(/\s+/g,charReplace)
}

// Replace whitespace with "_" and allow only HTML CDATA
//   characters: "a"-"z","A"-"Z","0"-"9", "_", ":", "-", ".",
//   and characters above ASCII 127
function ConvertToCDATAString(string)
{
  return string.replace(/\s+/g,"_").replace(/[^a-zA-Z0-9_\.\-\:\u0080-\uFFFF]+/g,'');
}

function msiGetSelectionAsText(editorElement)
{
  try {
    return msiGetEditor(mEditorElement).outputToString("text/plain", 1); // OutputSelectionOnly
  } catch (e) {}

  return "";
}


/************* Get Current Editor and associated interfaces or info ***************/
//const nsIPlaintextEditor = Components.interfaces.nsIPlaintextEditor;
//const nsIHTMLEditor = Components.interfaces.nsIHTMLEditor;
//const nsITableEditor = Components.interfaces.nsITableEditor;
//const nsIEditorStyleSheets = Components.interfaces.nsIEditorStyleSheets;
//const nsIEditingSession = Components.interfaces.nsIEditingSession;

//function msiGetCurrentEditor()
//{
//  // Get the active editor from the <editor> tag
//  // XXX This will probably change if we support > 1 editor in main Composer window
//  //      (e.g. a plaintext editor for HTMLSource)
//
//  // For dialogs: Search up parent chain to find top window with editor
//  var editor;
//  try {
//    var editorElement = msiGetCurrentEditorElement();
//    editor = editorElement.getEditor(editorElement.contentWindow);
//
//    // Do QIs now so editor users won't have to figure out which interface to use
//    // Using "instanceof" does the QI for us.
//    editor instanceof Components.interfaces.nsIPlaintextEditor;
//    editor instanceof Components.interfaces.nsIHTMLEditor;
//  } catch (e) { dump (e)+"\n"; }
//
//  return editor;
//}

function msiGetTableEditor(editorElement)
{
  var editor = msiGetEditor(editorElement);
  return (editor && (editor instanceof nsITableEditor)) ? editor : null;
}

//function msiGetCurrentEditorElement()
//{
//  var tmpWindow = window;
//  
//  do {
//    // Get the <editor> element(s)
//    var editorList = tmpWindow.document.getElementsByTagName("editor");
//
//    // This will change if we support > 1 editor element
//    if (editorList.item(0))
//      return editorList.item(0);
//
//    tmpWindow = tmpWindow.opener;
//  } 
//  while (tmpWindow);
//
//  return null;
//}
//
//function msiGetCurrentEditingSession()
//{
//  try {
//    return msiGetCurrentEditorElement().editingSession;
//  } catch (e) { dump (e)+"\n"; }
//
//  return null;
//}
//
//function msiGetCurrentCommandManager()
//{
//  try {
//    return msiGetCurrentEditorElement().commandManager;
//  } catch (e) { dump (e)+"\n"; }
//
//  return null;
//}
//
//function msiGetCurrentEditorType()
//{
//  try {
//    return msiGetCurrentEditorElement().editortype;
//  } catch (e) { dump (e)+"\n"; }
//
//  return "";
//}
//
//function msiIsHTMLEditor()
//{
//  // We don't have an editorElement, just return false
//  if (!msiGetCurrentEditorElement())
//    return false;
//
//  var editortype = msiGetCurrentEditorType();
//  switch (editortype)
//  {
//      case "html":
//      case "htmlmail":
//        return true;
//
//      case "text":
//      case "textmail":
//        return false
//
//      default:
//        dump("INVALID EDITOR TYPE: " + editortype + "\n");
//        break;
//  }
//  return false;
//}
//
function msiPageIsEmptyAndUntouched(editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  return msiIsDocumentEmpty(editorElement) && !msiIsDocumentModified(editorElement) && !msiIsHTMLSourceChanged(editorElement);
}

function msiIsWebComposer(theWindow)
{
//  return document.documentElement.id == "editorWindow";
  if (!theWindow)
    theWindow = window;
  if (theWindow.document && theWindow.document.documentElement)
    return theWindow.document.documentElement.id == "prince";
  return false;
}

function clearPrevActiveEditor(timerData)
{
  var theWindow = msiGetTopLevelWindow();

//Logging stuff only
  var logStr = msiEditorStateLogString(theWindow);
  var currFocusedElement = null;
  if (!theWindow.msiActiveEditorElement)
  {
    try
    {
      var winWatcher = Components.classes["@mozilla.org/embedcomp/window-watcher;1"]
                           .getService(Components.interfaces.nsIWindowWatcher);
      var activeWin = winWatcher.activeWindow;
      var commandDispatcher = activeWin.document.commandDispatcher;
      currFocusedElement = commandDispatcher.focusedElement;
    } catch(exc) {dump("In clearPrevActiveEditor, unable to retrieve curr focused element, error is [" + exc + "].\n");}
  }
//End logging stuff
  if (theWindow.msiClearEditorTimerList != null)
  {
    logStr += ";\n  clearPrevActiveEditor called, ";
    if (!theWindow.msiActiveEditorElement)
    {
      logStr += "with curr focused element [";
      if (currFocusedElement != null)
      {
        logStr += currFocusedElement.nodeName;
        if (currFocusedElement.id)
          logStr += ", id " + currFocusedElement.id;
      }
      logStr += "],";
    }
    try
    {
      theWindow.clearTimeout(timerData.nTimerID);
    } catch(exc) {dump("Exception in clearPrevActiveEditor; theWindow couldn't clear timeout [" + timerData.nTimerID + "], exception [" + exc + "].\n");}
    var nFound = msiFindTimerInArray(theWindow.msiClearEditorTimerList, timerData);
    if (nFound >= 0)
    {
      theWindow.msiClearEditorTimerList.splice(nFound, 1);
      if (theWindow.msiClearEditorTimerList.length == 0)
      {
        logStr += "deleting reference to prev editor.\n";
        theWindow.msiPrevEditorElement = null;
        if (theWindow.msiSingleDialogList)
          theWindow.msiSingleDialogList.reparentAppropriateDialogs(theWindow.msiActiveEditorElement);
        msiDoUpdateCommands("style", theWindow.msiActiveEditorElement);
      }
      else
        logStr += "but timer list still contains [" + theWindow.msiClearEditorTimerList.toString() + "], so not deleting prev editor.\n";
    }
    else
    {
      logStr += "but timer [";
      if (timerData != null)
        logStr += timerData.nTimerID;
      logStr += "] not found; doing nothing (?).\n";
    }
  }
  else
    logStr += ";\n  clearPrevActiveEditor called, but no timer to clear.\n";
  msiKludgeLogString(logStr);
}


function msiResetActiveEditorElement()
{
  var topWindow = msiGetTopLevelWindow();
//Logging stuff only
//  var logStr = "In msiResetActiveEditorElement; ";
//  logStr += msiEditorStateLogString(topWindow);
//End logging stuff

//  if (topWindow.msiPrevEditorElement && topWindow.msiActiveEditorElement && topWindow.msiPrevEditorElement != topWindow.msiActiveEditorElement)
  if (topWindow.msiPrevEditorElement && topWindow.msiPrevEditorElement != topWindow.msiActiveEditorElement)
  {
    topWindow.msiActiveEditorElement = topWindow.msiPrevEditorElement;
    topWindow.msiPrevEditorElement = null;
//Logging stuff only
//    logStr += ";\n  reverting to prev editor element.\n";
  }
//Logging stuff only
//  else
//    logStr += ";\n  making no change.\n";
//End logging stuff
  if (topWindow.msiClearEditorTimerList != null)
    msiClearAllFocusTimers(topWindow, topWindow.msiClearEditorTimerList);
//  msiKludgeLogString(logStr);
}

function msiEditorStateLogString(theWindow)
{
  return "";

  if (!theWindow)
    theWindow = msiGetTopLevelWindow();
  var currEdId = "";
  if ( ("msiActiveEditorElement" in theWindow) && (theWindow.msiActiveEditorElement != null) )
    currEdId = theWindow.msiActiveEditorElement.id;
  var prevEdId = "";
  if ( ("msiPrevEditorElement" in theWindow) && (theWindow.msiPrevEditorElement != null) )
    prevEdId = theWindow.msiPrevEditorElement.id;
  var logStr = "Current msiActiveEditor is [" + currEdId + "], prevEdId is [" + prevEdId + "]";
  logStr += ", clearEditorTimer IDs are [";
  if ("msiClearEditorTimerList" in theWindow && theWindow.msiClearEditorTimerList != null)
    logStr += theWindow.msiClearEditorTimerList.toString();
  logStr += "], for window titled [";
  logStr += theWindow.document.title;
  logStr += "]";
  return logStr;
}

function msiFindTimerInArray(timerList, timerData)
{
  var nIndex = -1;
  for (var ix = 0; nIndex < 0 && ix < timerList.length; ++ix)
  {
    if (timerList[ix] == timerData.nTimerID)
      nIndex = ix;
  }
  return nIndex;
}

function msiClearAllFocusTimers(theWindow, timerList)
{
  for (var ix = timerList.length - 1; ix >= 0; --ix)
  {
    try
    {
      theWindow.clearTimeout(timerList[ix]);
      timerList.splice(ix, 1);
    } catch(exc) {dump("In msiClearAllFocusTimers, theWindow couldn't clear timer [" + timerList[ix] + "], exception [" + exc + "].\n");}
  }
//  timerList.splice(0, timerList.length);
}

function msiSetActiveEditor(editorElement, bIsFocusEvent)
{
  var theWindow = msiGetTopLevelWindow();

//Logging stuff only:
//  var currEdId = "";
//  if ( ("msiActiveEditorElement" in theWindow) && (theWindow.msiActiveEditorElement != null) )
//    currEdId = theWindow.msiActiveEditorElement.id;
  var newEdId = "";
  if (editorElement)
    newEdId = editorElement.id;
//  var prevEdId = "";
//  if ( ("msiPrevEditorElement" in theWindow) && (theWindow.msiPrevEditorElement != null) )
//    prevEdId = theWindow.msiPrevEditorElement.id;
//End logging stuff

  var bIsDifferent = (!theWindow.msiActiveEditorElement || (theWindow.msiActiveEditorElement != editorElement));
  if (bIsDifferent)
  {
//Logging stuff only:
//    var logStr = "In msiSetActiveEditor, current msiActiveEditor is [" + currEdId + "], prevEdId is [" + prevEdId + "]";
//    var logStr = msiEditorStateLogString(theWindow);
//    logStr += ",\n  trying to change to [" + newEdId + "], setting; bIsFocusEvent is  "
//    if (bIsFocusEvent)
//      logStr += "true";
//    else
//      logStr += "false";
//End logging stuff
    if (bIsFocusEvent)
    {
      if (!theWindow.msiPrevEditorElement)
        theWindow.msiPrevEditorElement = theWindow.msiActiveEditorElement;
//    {
      var newTimerData = new Object();
      var nNewTimer = theWindow.setTimeout(clearPrevActiveEditor, 0, newTimerData);
      if (theWindow.msiClearEditorTimerList == null)
        theWindow.msiClearEditorTimerList = new Array();
      theWindow.msiClearEditorTimerList.push(nNewTimer);
      newTimerData.nTimerID = nNewTimer;
//Logging stuff only:
//      logStr += ", new clearEditorTimer ID is " + nNewTimer;
//    }
    }
//    logStr += ".\n";
//    msiKludgeLogString(logStr);
//End logging stuff
    //To Do: re-parent appropriate dialogs at this point? or if there's a timer set, wait for it?
    theWindow.msiActiveEditorElement = editorElement;
    if ((theWindow.msiClearEditorTimerList==null || theWindow.msiClearEditorTimerList.length == 0) && theWindow.msiSingleDialogList)
    {
      theWindow.msiSingleDialogList.reparentAppropriateDialogs(theWindow.msiActiveEditorElement);
      msiDoUpdateCommands("style", theWindow.msiActiveEditorElement);
    }
//    To Do: update command states, Math/Text state...
  }
    
//  if (!theWindow.bIgnoreNextFocus || !bIsFocusEvent)
//  {
////Logging stuff only:
//    var logStr = "In msiSetActiveEditor, current msiActiveEditor is [" + currEdId + "], trying to change to [" + newEdId + "], setting; bIgnoreNextFocus is "
//    if (theWindow.bIgnoreNextFocus)
//      logStr += "true, bIsFocusEvent is ";
//    else
//      logStr += "false, bIsFocusEvent is  ";
//    if (bIsFocusEvent)
//      logStr += "true";
//    else
//      logStr += "false";
////End logging stuff
//    if (bIsDifferent)
//    {
//      theWindow.msiActiveEditorElement = editorElement;
//      if (bIsFocusEvent)
//      {
//        theWindow.msiClearEditorTimer = setTimeout(clearPrevActiveEditor, 0);
//        logStr += ", clearEditorTimer ID is " + theWindow.msiClearEditorTimer + ".\n";
//      }
//      else
//        logStr += ".\n";
//    }
//    else
//      logStr += ".\n";
//    msiKludgeLogString(logStr);
//  }
//  else
//    msiKludgeLogString("In msiSetActiveEditor, current msiActiveEditor is [" + currEdId + "], trying to change to [" + newEdId + "], not setting; bIgnoreNextFocus is true, bIsFocusEvent is true.\n");

  //The rationale for this line is that, as long as we're processing focus messages from clicking on a control in the main window
  //while the active editor is in another one, we keep the active editor where it is until focus gets reset to it, or until
  //another mechanism forces us to change it.

  return null;
}

function msiGetCurrentEditor(theWindow)
{
  if (!theWindow)
    theWindow = window;
  var editorElement = msiGetActiveEditorElement(theWindow);
  if (editorElement)
    return msiGetEditor(editorElement);
  return GetCurrentEditor();  //punt
}

function msiGetActiveEditorElement(currWindow)
{
  if (!currWindow)
    currWindow = window.document.defaultView;
  if ("msiActiveEditorElement" in currWindow)
    return currWindow.msiActiveEditorElement;
  currWindow = msiGetTopLevelWindow(currWindow);
  if ("msiActiveEditorElement" in currWindow)
    return currWindow.msiActiveEditorElement;
//  if (!editorElement && currWindow.opener && currWindow.opener != currWindow)
//    editorElement = msiGetActiveEditorElement(currWindow.opener);
  var editorElement = msiGetCurrentEditorElementForWindow(currWindow);
  if (!editorElement)
    editorElement = currWindow.GetCurrentEditorElement();  //Give up?
  if (!editorElement)
    dump("\nCan't find active editor element!\n");
  return editorElement;
}

function msiGetTopLevelEditorElement(currWindow)  //Is this right?
{
  var editorElement = null;
  if (!currWindow)
    currWindow = window.document.defaultView;
  var topWindow = msiGetTopLevelWindow(currWindow);
  if (topWindow)
    editorElement = msiGetPrimaryEditorElementForWindow(topWindow);
//  return msiGetActiveEditorElement(window);
  if (!editorElement)
    editorElement = msiGetPrimaryEditorElementForWindow(currWindow);
  if (!editorElement)
    dump("\nmsiGetTopLevelEditorElement returning void or null\m");
  return editorElement;
}


function msiGetCurrentEditorElementForWindow(theWindow)
{
  if (!theWindow)
    theWindow = window.document.defaultView;
  var editorElement = null;
  var currFocus = theWindow.document.commandDispatcher.focusedElement;
  if (currFocus && currFocus.nodeName == "editor")
    return currFocus;
  var editorList = theWindow.document.getElementsByTagName("editor");
  if (!currFocus)
  {
    var focusWindow = theWindow.document.commandDispatcher.focusedWindow;
    if (focusWindow && focusWindow != theWindow)
    {
      for (var i = 0; i < editorList.length; ++i)
      {
        if (editorList[i].contentWindow == focusWindow)
        {
          editorElement = editorList[i];
          break;
        }
      }
//      if (!editorElement)
//        editorElement = msiGetCurrentEditorElementForWindow(focusedWindow);
    }
  }
  if (!editorElement)
    editorElement = editorList[0];  //just return the first one in the list
  if (!editorElement)
    dump("\nmsiGetCurrentEditorElementForWindow returning void or null\m");
  return editorElement;
}

function msiGetPrimaryEditorElementForWindow(theWindow)
{
  if (!theWindow)
    theWindow = window.document.defaultView;
  var theEditor = null;
  var editorElements = theWindow.document.getElementsByTagName("editor");
  for (var i = 0; (!theEditor) && (i < editorElements.length); ++i)
  {
    if (editorElements[i].getAttribute("type") == "content-primary")
      theEditor = editorElements[i];
  }
  if (!theEditor)
    theEditor = editorElements[0];
  if (!theEditor)
    dump("\nmsiGetPrimaryEditorElementForWindow returning void or null\m");
  return theEditor;
}

function msiIsTopLevelEditor(editorElement)
{
  var topEditor = msiGetTopLevelEditorElement();
  return (topEditor == editorElement);
}

function msiGetTopLevelWindow(currWindow)  //do we need a different function for this? probably not - there is one, sort of?
{
  if (!currWindow)
    currWindow = window.document.defaultView;
  if ("ownerDocument" in currWindow)
    currWindow = currWindow.ownerDocument.defaultView;
  if (currWindow.parent && currWindow.parent != currWindow)
    return msiGetTopLevelWindow(currWindow.parent);
  if (msiIsWebComposer(currWindow))
    return currWindow;
  if (currWindow.opener && currWindow.opener != currWindow)
    return msiGetTopLevelWindow(currWindow.opener);
  return currWindow;
}

function msiGetParentEditor(editorElement)
{
//  if ("msiParentEditor" in editorElement)
//    return editorElement.msiParentEditor;
  var parentWindow = msiGetWindowContainingEditor(editorElement);
  if ("msiParentEditor" in parentWindow)
  {
    if (!parentWindow.msiParentEditor)
      dump("\nmsiGetParentEditor returning void or null\m");
    return parentWindow.msiParentEditor;
//  var docElement = parentWindow.document.documentElement;
//  if ("msiParentEditor" in docElement)
//    return docElement.msiParentEditor;
  }
  dump("Can't find parent editor for editorElement " + editorElement.id);
  return null;
}

function msiSetParentEditor(parentEditor, theWindow)
{
  var retVal = false;
  if (!theWindow)
    theWindow = window;
  if (!parentEditor)
    dump("\nmsiSetParentEditor was given null or void SetParentEditor\n");    
  if ("msiParentEditor" in theWindow)
  {
    if (theWindow.msiParentEditor != parentEditor)
    {
      theWindow.msiParentEditor = parentEditor;
      retVal = true;
    }
  }
  var parentWindow = theWindow.parent;
  if (parentWindow != null && parentWindow != theWindow && "msiParentEditor" in parentWindow)
  {
    if (parentWindow.msiParentEditor != parentEditor)
    {
      parentWindow.msiParentEditor = parentEditor;
      retVal = true;
    }
  }
  return retVal;
}

function msiRemoveActiveEditor(editorElement)
{
  var currActive = msiGetActiveEditorElement(window);
  if (currActive == editorElement)
  {
    var ourParent = msiGetParentEditor(editorElement);
    msiSetActiveEditor(ourParent);  //want to do this even if null!?
  }
}

function msiCurrEditorSetFocus(theWindow)
{
  var retVal = true;
  if (!theWindow)
    theWindow = window;
  var currEdElement = msiGetActiveEditorElement(theWindow);
  if (currEdElement)
    currEdElement.contentWindow.focus();
  else
    retVal = false;
  return retVal;
}

function msiGetEditor(editorElement)
{
  var editor;
  if (!editorElement) return null;
  try
  {
    editor = editorElement.getEditor(editorElement.contentWindow);
    // Do QIs now so editor users won't have to figure out which interface to use
    // Using "instanceof" does the QI for us.
    editor instanceof Components.interfaces.nsIPlaintextEditor;
    editor instanceof Components.interfaces.nsIHTMLEditor;
  }
  catch(e) 
  { 
    dump("msiGetEditor: " + e + "\n"); 
  }
  return editor;
}

function msiGetCommandManager(editorElement)
{
  var commandManager;
  try {commandManager = editorElement.commandManager;}
  catch(e) {}
  return commandManager;
}


function msiGetControllerForCommand(command, editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var controller = null;
  if (editorElement)
  {
    try
    {
      controller = editorElement.contentWindow.controllers.getControllerForCommand(command);
//      controller = editorElement.contentWindow.document.commandDispatcher.getControllerForCommand(command);
      if (!controller)
      {
        var topWin = msiGetTopLevelWindow(editorElement);
        controller = topWin.controllers.getControllerForCommand(command);
//        controller = editorElement.ownerDocument.commandDispatcher.getControllerForCommand(command);
      }
    }
    catch(exc) {AlertWithTitle("Error in msiGetControllerForCommand!", exc);}
  }
  return controller;
}

function msiGetEditorType(editorElement)
{
  try {
    return editorElement.editortype;
  } catch (e) { dump (e)+"\n"; }

  return "";
}

function msiIsHTMLEditor(editorElement)
{
  // We don't have an editorElement, just return false
  if (!editorElement)
    return false;

  var editortype = msiGetEditorType(editorElement);
  switch (editortype)
  {
      case "html":
      case "htmlmail":
        return true;

      case "text":
      case "textmail":
        return false

      default:
        dump("INVALID EDITOR TYPE: " + editortype + "\n");
        break;
  }
  return false;
}

function msiWindowHasHTMLEditor(theWindow)
{

  if (!theWindow)
    theWindow = window;
  
  if (theWindow.document != null)
  {
    var editorList = theWindow.document.getElementsByTagName("editor");
    for (var i = 0; i < editorList.length; ++i)
    {
      if ( editorList.item(i)!=null && msiIsHTMLEditor(editorList.item(i)) )
        return true;
    }
  }
  return false;
}

//Stolen from GetCurrentEditorElement() and hacked.
function GetEditorElementForDocument(innerDocument, theWindow)
{
  if (!theWindow)
    theWindow = window;
//  var tmpWindow = theWindow;
  
//  do
//  {
    // Get the <editor> element(s)
    var editorList = theWindow.document.getElementsByTagName("editor");
    for (var i = 0; i < editorList.length; ++i)
    {
      if (editorList.item(i) && editorList.item(i).contentDocument == innerDocument)
        return editorList.item(i);
    }
//    if (tmpWindow.frameElement && tmpWindow != tmpWindow.frameElement.ownerDocument.defaultView)
//      tmpWindow = tmpWindow.frameElement.ownerDocument.defaultView;
//    else if (tmpWindow.top && tmpWindow != tmpWindow.top)
//      tmpWindow = tmpWindow.top;
//    else if (tmpWindow.opener && tmpWindow != tmpWindow.opener)
//      tmpWindow = tmpWindow.opener;
//    else
//      tmpWindow = null;
//  } 
//  while (tmpWindow);

  return null;
}

function findEditorElementForDocument(innerDocument)
{
  if (!innerDocument)
  {
    AlertWithTitle("Error in msiEditorUtilities.js", "Null innerDocument passed into findEditorElementForDocument!");
    return null;
  }

  var editorElement = null;
  var tmpWindow = null;
  var windowWatcher = Components.classes["@mozilla.org/embedcomp/window-watcher;1"].getService(Components.interfaces.nsIWindowWatcher);
  var winEnum = windowWatcher.getWindowEnumerator();
  while (!editorElement && winEnum.hasMoreElements())
  {
    tmpWindow = winEnum.getNext();
    //Then do the check on tmpWindow?
    editorElement = GetEditorElementForDocument(innerDocument, tmpWindow);
  }
  return editorElement;
}

function msiGetEditorElementFromEvent(theEvent)
{
  if (!theEvent)
  {
    AlertWithTitle("Error in msiEditorUtilities.js", "Null event passed into msiGetEditorElementFromEvent!");
    return null;
  }

  var editorElement = theEvent.currentTarget;
  if (!("editorType" in editorElement))
  {
    var origTarget = theEvent.explicitOriginalTarget;
    if (origTarget == null)
      origTarget = theEvent.originalTarget;
    var theDocument = null;
    if ("defaultView" in origTarget)
      theDocument = origTarget;
    if ((theDocument==null) && ("ownerDocument" in origTarget))
      theDocument = origTarget.ownerDocument;
    if ((theDocument==null) && ("top" in origTarget))
    {
      theDocument = origTarget.document;
//      var logStr = "In msiGetEditorElementFromEvent, original target document is [";
//      if (origTarget.document)
//        logStr += "not null";
//      else
//        logStr += "null";
//      logStr += "], but theDocument is [";
//      if (theDocument == null)
//        logStr += "null";
//      else
//        logStr += "not null";
//      logStr += "]\n";
//      msiKludgeLogString(logStr);
    }
    if (theDocument==null)
      editorElement = null;
    else
      editorElement = findEditorElementForDocument(theDocument);
  }
  return editorElement;
}

function msiGetUpdatableItemContainers(commandID, editorElement)
{
  var returnList = new Array();
  var topWindow = msiGetTopLevelWindow();
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var currWindow = msiGetWindowContainingEditor(editorElement);
  try
  {
    var theItem = topWindow.document.getElementById(commandID);
    if (theItem != null)
      returnList.push(topWindow.document);
  }
  catch(exc) {AlertWithTitle("Error in msiGetUpdatableItemContainers!", exc);}

  if (!msiIsTopLevelEditor(editorElement) && currWindow != null && currWindow != topWindow)
  {
    var theItem = currWindow.document.getElementById(commandID);
    if (theItem != null)
      returnList.push(currWindow.document);
  }
  if (window != topWindow && window != currWindow)
  {
    var theItem = window.document.getElementById(commandID);
    if (theItem != null)
      returnList.push(window.document);
  }
  return returnList;
}

function msiIsDocumentEditable(editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  try {
    return msiGetEditor(editorElement).isDocumentEditable;
  } catch (e) {}
  return false;
}

//If there should be an HTML Source window associated with an editor control, we should fetch its underlying nsEditor here.
//For now, assume NOT.
function msiGetHTMLSourceEditor(editorElement)
{
  if ("mSourceTextEditor" in editorElement)
    return editorElement.mSourceTextEditor;
  return null;
}

function msiGetHTMLSourceTextWindow(editorElement)
{
  if ("mSourceContentWindow" in editorElement)
    return editorElement.mSourceContentWindow;
  return null;
}

function msiIsHTMLSourceChanged(editorElement)
{
  var sourceEditor = msiGetHTMLSourceEditor(editorElement);
  if (sourceEditor)
    return sourceEditor.documentModified;
  return false;    
}

function msiIsInHTMLSourceMode(editorElement)
{
  return false;
}

// are we editing HTML (i.e. neither in HTML source mode, nor editing a text file)
function msiIsEditingRenderedHTML(editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  return msiIsHTMLEditor(editorElement) && !msiIsInHTMLSourceMode(editorElement);
}


function msiIsDocumentEmpty(editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  try {
    return msiGetEditor(editorElement).documentIsEmpty;
  } catch (e) {}
  return false;
}

function msiIsDocumentModified(editorElement)
{
  try {
    return msiGetEditor(editorElement).documentModified;
  } catch (e) {}
  return false;
}

function msiGetEditorURL(editorElement)
{
  var editor;
  try
  {
    editor = msiGetEditor(editorElement);
    var aDOMHTMLDoc = editor.document.QueryInterface(Components.interfaces.nsIDOMHTMLDocument);
    return aDOMHTMLDoc.URL;
  } catch (e) {
    try {
      return editor.document.documentURI;
    } catch (e){}
  }
  return "";
}

//function IsHTMLSourceChanged()
//{
//  return gSourceTextEditor.documentModified;
//}

function newCommandParams()
{
  try {
    return Components.classes["@mozilla.org/embedcomp/command-params;1"].createInstance(Components.interfaces.nsICommandParams);
  }
  catch(e) { dump("error thrown in newCommandParams: "+e+"\n"); }
  return null;
}

//Moved from computeOverlay.js
function insertXML(editor, text, node, offset, bDump)
{
  var parser = new DOMParser();
  var doc = parser.parseFromString(text,"application/xhtml+xml");
  var nodeList = doc.documentElement.childNodes;
  if (bDump)
    dump("\nIn insertXML, inserting [" + text + "], with documentElement [" + doc.documentElement.nodeName + "] in node [" + node.nodeName + "] at offset [" + offset + "].\n");
  var i;
  // we can only insert nodes under an element, not under anything else such as a text node or a
  // processing instruction node. The cursor is likely in a text node
  //
  if (node.nodeType != node.ELEMENT_NODE)
  {
    if (node.nodeType != node.TEXT_NODE)
    {
      dump("Unexpected node type = "+node.nodeType);
      return;
    }
    else
    {
    // split the text node
      var newNode = editor.document.createTextNode("");
      editor.splitNode(node, offset, newNode);
      var list = node.parentNode.childNodes;
      for (i = 0; i< list.length; i++) 
        if (list[i]==node) break;
      offset = i;
      node = node.parentNode;
    }
  }
  var nodeListLength = nodeList.length;
  for (i = nodeListLength-1; i >= 0; --i)
  {
    editor.insertNode( nodeList[i], node, offset );
  }
}

function insertXMLAtCursor(editor, text, bWithinPara, bSetCaret)
{
  if (!editor)
  {
    dump("Error in msiEditorUtilities.js, insertXMLAtCursor - null editor!\n");
    return false;
  }
  if (bWithinPara)
    text = "<para>" + text + "</para>";
  else
    text = "<body>" + text + "</body>";
  var theElement = null;
  var theOffset = 0;
  var theLength = 0;
  var bOK = true;
  try
  {
//    dump("Going to insertXML, text before call is [" + text + "].\n");
    theElement = editor.selection.focusNode;
    theOffset = editor.selection.focusOffset;
    var fixedPos = fixInsertPosition(editor, theElement, theOffset);
    if (fixedPos != null)
    {
      theElement = fixedPos.theElement;
      theOffset = fixedPos.theOffset;
    }
    theLength = theElement.childNodes.length;
    insertXML(editor, text, theElement, theOffset, true);
    dump("insertXML now done.\n");
  }
  catch(exc)
  {
    dump("If you see this dump, Barry loses his bet\n");
    dump("In insertXMLAtCursor, couldn't use editor.selection! (Exception: [" + exc + "]\n");
    theElement = editor.document.rootElement;
    if (theElement != null && theElement.childNodes.length > 0)
    {
      var targetTag = "body";
      if (bWithinPara)
        targetTag = "para";
      var theCollection = theElement.getElementsByTagName(targetTag);
      if (theCollection.length == 0 && bWithinPara)
        theCollection = theElement.getElementsByTagName("p");
      if (theCollection.length == 0)
        theCollection = theElement.childNodes;
      if (theCollection.length > 0)
        theElement = theCollection[theCollection.length - 1];
    }
    if (theElement != null)
    {
      theLength = theOffset = theElement.childNodes.length;
      try
      {
        insertXML(editor, text, theElement, theOffset, false);
      } 
      catch(exc)
      {
        bOK = false;
        dump("In insertXMLAtCursor, unable to insert at all! Exception: [" + exc + "].\n");
      }
    }
    else
    {
      dump("No content nodes to insert into in editor (insertXMLAtCursor)!\n");
      bOK = false;
    }
  }
  if (bOK)
  {
    var newElement = theElement;
    var newOffset = theOffset;
    try
    {
      var newDumpStr = "After insertion, focusNode is [";
      if (editor.selection.focusNode)
      {
        newElement = editor.selection.focusNode;
        newDumpStr += newElement.nodeName;
      }
      newOffset = editor.selection.focusOffset;
      newDumpStr += "], and focusOffset is [" + newOffset + "].\n";
      dump(newDumpStr);
    } catch(ex) {dump("Exception in insertXMLAtCursor trying to get post-insert position; exception is [" + ex + "].\n");}
    //Find inserted nodes now in order to rationally place cursor.
    var newLength = theElement.childNodes.length;

    if (bSetCaret)
    {
      var caretNodeData = findCaretPositionAfterInsert(editor.document, theElement, theOffset, newElement, newOffset);

//      dump("New contents of editor are: [\n");
//      if (editor != null)
//        editor.dumpContentTree();
//      dump("\n].\n");
//      if (caretNodeData == null && newElement != null)
//      {
//        caretNodeData = new Object();
//        caretNodeData.theNode = newElement;
//        caretNodeData.theOffset = newOffset;
//      }
      if (caretNodeData != null)
      {
        if (caretNodeData.theNode == null)
        {
          dump("In insertXMLAtCursor, got non-null caretNodeData with null caretNodeData.theNode!\n");
        }
        else
        {
          var adjustedCaret = moveCaretToTextChild(caretNodeData.theNode, caretNodeData.theOffset);
          if (adjustedCaret != null)
            caretNodeData = adjustedCaret;
          try
          {
            editor.selection.collapse(caretNodeData.theNode, caretNodeData.theOffset);
  //          dump("In insertXMLAtCursor, set caret inside node [" + caretNodeData.theNode.nodeName + "], at offset [" + caretNodeData.theOffset + "], with old length = [" + theLength + "] and new length = [" + newLength + "].\n");
          }
          catch(exc) {dump("In insertXMLAtCursor, unable to set caret inside node [" + caretNodeData.theNode.nodeName + "]; exception is [" + exc + "].\n");}
        }
      }
      else
        dump("In insertXMLAtCursor, no caretNode found.\n");
    }
  }
  return bOK;
}

function msiGetDocumentHead(theEditor)
{
  var docHead = null;
  var childElements = theEditor.document.documentElement.getElementsByTagName("*");
  var testFor = ["preamble", "meta", "title", "link"];
  for (var ix = 0; (docHead == null) && (ix < childElements.length); ++ix)
  {
    for (var jx = 0; jx < testFor.length; ++jx)
    {
      var testList = childElements[ix].getElementsByTagName(testFor[jx]);
      if (testList.length > 0)
      {
        docHead = childElements[ix];
        break;
      }
    }
  }
  if (docHead == null)
  {
    var headList = theEditor.document.getElementsByTagName("head");
    if (headList.length > 0)
      docHead = headList[0];
  }
  return docHead;
}

function msiGetRealBodyElement(document)
{
  var theElement = null;
  var targetTag = "body";
  var theNodes = null;
  if (!document.rootElement)
  {
    dump("No rootElement for document in msiGetRealBodyElement!\n");
    theNodes = document.getElementsByTagName(targetTag);
  }
  else
  {
    if (document.rootElement.nodeName == "body")
      return document.rootElement;
    theNodes = document.rootElement.getElementsByTagName(targetTag);
  }
  if (theNodes.length > 0)
  {
    for (var ix = 0; ix < theNodes.length; ++ix)
    {
      if (theNodes[ix].childNodes.length > 0)
      {
        theElement = theNodes[ix];
        break;
      }
    }
    if (theElement == null)
      theElement = theNodes[0];
  }
  else
    theElement = document.rootElement;
  return theElement;
}

function fixInsertPosition(editor, theNode, offset)
{
  var retVal = new Object();
  retVal.theElement = theNode;
  retVal.theOffset = offset;

  var theElement = theNode;
  var topNode = msiGetRealBodyElement(editor.document);
  if (topNode == null)
    topNode = editor.document.documentElement;

  if (theElement == topNode)
  {
    if (offset > 0)
    {
      theElement = topNode.childNodes[offset - 1];
      offset = theElement.childNodes.length;
    }
    else if (topNode.childNodes.length > 0)
      theElement = topNode.childNodes[0];      //Note that in this case offset" is already 0, which is what we'd set it to.
    else
    {
      dump("In fixInsertPosition(), can't find any reasonable position! Body appears empty.\n");
      return retVal;
    }
  }
  var theParent = msiGetBlockNodeParent(editor, theElement);
  if (theParent != null && theParent != topNode)
  {
    retVal.theElement = theElement;
    retVal.theOffset = offset;
    return retVal;
  }

  function findBlockParents(aNode)
  {
    var blockParent = msiGetBlockNodeParent(editor, aNode);
    if (blockParent == topNode)
      return NodeFilter.FILTER_SKIP;
    else if (blockParent != null) 
      return NodeFilter.FILTER_ACCEPT;
    else
      return NodeFilter.FILTER_REJECT;  //rejects whole subtree
  }

  var treeWalker = editor.document.createTreeWalker(topNode,
                                                NodeFilter.SHOW_ELEMENT,
                                                findBlockParents,
                                                true);
  if (treeWalker)
  {
    treeWalker.currentNode = theElement;
    var foundNode = treeWalker.previousNode();
    if (foundNode != null)
    {
      retVal.theElement = foundNode;
      retVal.theOffset = foundNode.childNodes.length;
    }
    else
    {
      foundNode = treeWalker.nextNode();
      if (foundNode != null)
      {
        retVal.theElement = foundNode;
        retVal.theOffset = 0;
      }
    }
  }

//If we got here, we were unable to find any paragraph-level elements to insert into. We just use what we arrived with?
  return retVal;
}

function msiGetBlockNodeParent(editor, aNode)
{
  for (var theParent = aNode; theParent != null; theParent = theParent.parentNode)
  {
    if (editor.nodeIsBlock(theParent))
      return theParent;
  }
  return null;
}

function msiFindParentOfType(startNode, nodeType, stopAt)
{
  var retNode = null;
  var theNode = startNode;
  if (stopAt == null || stopAt.length == 0)
    stopAt = "#document";
  while (retNode==null && theNode != null)
  {
    if (theNode.nodeName == nodeType)
      retNode = theNode;
    if (theNode.nodeName == stopAt)
      break;
    theNode = theNode.parentNode;
  }
  return retNode;
}

//The idea is to use a NodeIterator to walk the new or affected nodes. We want to find the marked caret position (using the
//"caretpos" attribute), or the first input box (always in an <mi>?), or, failing that, to leave it at the postNode and postOffset.
function findCaretPositionAfterInsert(document, preNode, preOffset, postNode, postOffset)
{
  //What we return if nothing found.
  var retVal = new Object();
  retVal.theNode = postNode;
  retVal.theOffset = postOffset;

  var startNode = preNode;
  var startOffset = preOffset;
  var endNode = postNode;
  var endOffset = postOffset;
  var endNode = postNode;
  var comp = startNode.compareDocumentPosition(postNode);
  if (comp == Node.DOCUMENT_POSITION_FOLLOWING)
  {
    startNode = postNode;
    startOffset = postOffset;
    endNode = preNode;
    endOffset = preOffset;
  }
  if (startOffset > 0 && startNode.childNodes.length > startOffset)
  {
    startNode = startNode.childNodes[startOffset];
    startOffset = 0;
  }
  if (endOffset > 0 && endNode.childNodes.length > endOffset)
  {
    endNode = endNode.childNodes[endOffset];
    endOffset = endNode.childNodes.length;
  }
  var theRange = document.createRange();
  theRange.setStart(preNode, preOffset);
  theRange.setEnd(postNode, postOffset);
  var rootNode = theRange.commonAncestorContainer;
  theRange.detach();
  
  function isCaretPosition(aNode)
  {

    if (aNode.hasAttribute("caretpos") || (aNode.hasAttribute("tempinput") && aNode.getAttribute("tempinput")=="true"))
      return NodeFilter.FILTER_ACCEPT;
    return NodeFilter.FILTER_SKIP;
  }

  var treeWalker = document.createTreeWalker(rootNode, NodeFilter.SHOW_ELEMENT,
                                                isCaretPosition, true);
  if (treeWalker)
  {
    var nextPos = treeWalker.currentNode = startNode;
    var bFoundInput = false;
    var bFoundCaretPos = false;
    while (!bFoundCaretPos && nextPos != null && nextPos.compareDocumentPosition(endNode) != Node.DOCUMENT_POSITION_PRECEDING)
    {
      if (nextPos.hasAttribute("caretpos"))
      {
        bFoundCaretPos = true;
        retVal.theNode = nextPos;
        retVal.theOffset = nextPos.getAttribute("caretpos");
      }
      else if (!bFoundInput && nextPos.hasAttribute("tempinput") && nextPos.getAttribute("tempinput")=="true")
      {
        bFoundInput = true;
        retVal.theNode = nextPos;
        retVal.theOffset = msiInputBoxCaretOffset;  //set to 1
      }
      nextPos = treeWalker.nextNode();
    }
  }

////  if (retVal != null) //In these cases we attempt to put the caret inside a child text node.
////  {
//  var nNonEmptyTextChild = -1;
//  for (var kx = 0; kx < retVal.theNode.childNodes.length; ++kx)
//  {
//    if (retVal.theNode.childNodes[kx].nodeType != Components.interfaces.nsIDOMNode.TEXT_NODE)
//    {
//      nNonEmptyTextChild = -1;  //There are non-text children, so we don't want to assume the offset's intended to go inside a text child.
//      break;
//    }
//    if (nNonEmptyTextChild < 0 && retVal.theNode.childNodes[kx].nodeValue.length > 0)
//      nNonEmptyTextChild = kx;
//  }
//  if (nNonEmptyTextChild >= 0)
//    retVal.theNode = retVal.theNode.childNodes[nNonEmptyTextChild];
////  }

  return retVal;
}

////Returns the index in theNode.childNodes of a unique nonempty text node, or a node containing only one child
////  which contains only one nonempty text node, if that's all it contains.
////Returns -1 if containing any other non-trivial structure.
//function getSingleNonemptyTextNode(theNode)
//{
//  var nNonEmptyTextChild = -1;
//  var nNonEmptyOtherChild = -1;
//  for (var kx = 0; kx < theNode.childNodes.length; ++kx)
//  {
//    if (theNode.childNodes[kx].nodeType != Components.interfaces.nsIDOMNode.TEXT_NODE)
//    {
//      if (nNonEmptyOtherChild >= 0 || nNonEmptyTextChild >= 0)
//        return null;
//      nNonEmptyOtherChild = kx;
//    }
//    else if (theNode.childNodes[kx].nodeValue.length > 0)  //found a text node
//    {
//      if (nNonEmptyTextChild >= 0 || nNonEmptyOtherChild >= 0)  //found something else previously
//        return null;
//      nNonEmptyTextChild = kx;
//    }
//  }
//  if (nNonEmptyOtherChild >= 0)
//    return getSingleNonemptyTextNode(theNode.childNodes[nNonEmptyOtherChild]);
//  if (nNonEmptyTextChild >= 0)
//    return theNode.childNodes[nNonEmptyTextChild];
//  return null;
//}

//Cases still needing special treatment in placement of the caret:
//  i) If a Node's children are all text nodes, they may be regarded as one text-node child, and the offset should
//     be used to index into the string as a whole.
// ii) If a Node contains, other than empty text nodes, only one child Node, the offset may be passed into it to check
//     for situation (i).
//iii) Otherwise, if a Node contains a child Node at the passed-in non-zero offset (that is, at childNodes[theOffset-1], except
//     that -1 is regarded as "last"), the decision should be passed on to the child Node, with an offset of -1; while if
//     the offset is 0, the decision should be passed on the childNodes[0] with an offset of 0.
//Most of the above is rubbish. We have to assume the node and offset are meaningful positions in the DOM, thus the only
//business we have is to go to the position and see whether there's a good text-node position to place the caret at.
function moveCaretToTextChild(theNode, theOffset)
{
  if (theNode.nodeType == Components.interfaces.nsIDOMNode.TEXT_NODE)
  {
    var posObject = new Object();
    posObject.theNode = theNode;
    if (theNode.parentNode.hasAttribute("tempinput") && theNode.parentNode.getAttribute("tempinput")=="true")
      posObject.theOffset = msiInputBoxCaretOffset;  //set to 1
    else if (theOffset == -1 || theOffset > theNode.nodeValue.length)
      posObject.theOffset = theNode.nodeValue.length;
    else
      posObject.theOffset = theOffset;
    return posObject;
  }
  //Okay, we aren't a text node. Where does theOffset point?
  var theChild = null;
  var nLength = theNode.childNodes.length;
  if (!nLength)
    return null;

  if (theOffset == -1 || theOffset > nLength)
  {
    for (var jx = nLength; jx > 0; --jx)
    {
      if (theNode.childNodes[jx-1].nodeType != Components.interfaces.nsIDOMNode.TEXT_NODE || theNode.childNodes[jx-1].nodeValue.length > 0)
      {
        theChild = theNode.childNodes[jx-1];
        break;
      }
    }
  }
  else if (theOffset == 0)
  {
    for (var jx = 0; jx < nLength; ++jx)
    {
      if (theNode.childNodes[jx].nodeType != Components.interfaces.nsIDOMNode.TEXT_NODE || theNode.childNodes[jx].nodeValue.length > 0)
      {
        theChild = theNode.childNodes[jx];
        break;
      }
    }
  }
  else
    theChild = theNode.childNodes[theOffset - 1];

  if (theOffset != 0)
    theOffset = -1;
  return moveCaretToTextChild(theChild, theOffset);
}

//function moveCaretToTextChild(theNode, theOffset)
//{
//  if (theNode.nodeType == Components.interfaces.nsIDOMNode.TEXT_NODE)
//  {
//    var posObject = new Object();
//    posObject.theNode = theNode;
//    if (theOffset == -1 || theOffset > theNode.nodeValue.length)
//      posObject.theOffset = theNode.nodeValue.length;
//    else
//      posObject.theOffset = theOffset;
//    return posObject;
//  }
//
//  var childNode = null;
//  if (theNode.childNodes.length > 0)
//  {
//    if (bFromLeft)
//      return moveCaretToTextChild(theNode.childNodes[0], true);
//    else
//      return moveCaretToTextChild(theNode.childNodes[theNode.childNodes.length - 1], false);
//  }
//
//  return null;
//}

function findCaretPositionInNode(parentNode)
{
  var foundCaret = null;
  var bDoneMi = false;
  var bFoundAny = false;
  var bFoundInputBox = false;
  var bFoundCaretPos = false;
  var caretNode = null;
  var caretOffset = -1;
  var bFoundNew = false;
  var searchNodes = null;
  if (parentNode.childNodes.length == 0)
  {
    searchNodes = new Array(parentNode);
    bDoneMi = true;
  }
  else
    searchNodes = parentNode.getElementsByTagName("mi");
  if (searchNodes == null)
    return foundCaret;
  for (var jx = 0; ((jx < searchNodes.length) || (!bDoneMi)) && !bFoundCaretPos; ++jx)
  {
    bFoundNew = false;
    if (jx >= searchNodes.length && !bDoneMi)
    {
      searchNodes = parentNode.getElementsByTagName("*");
      bDoneMi = true;
      if (searchNodes.length == 0)
        break;
      jx = 0;
    }
    //The use of the attribute "caretpos" here is illustrative - need to find out what we may actually use.
    if ( !bFoundAny && (searchNodes[jx].nodeType == Components.interfaces.nsIDOMNode.TEXT_NODE) && (searchNodes[jx].nodeValue.length > 0) )
    {
      bFoundAny = bFoundNew = true;
      caretNode = searchNodes[jx];
      caretOffset = 0;
    }
    else if (searchNodes[jx].hasAttribute("caretpos"))
    {
      bFoundCaretPos = bFoundNew = true;
      caretNode = searchNodes[jx];
      caretOffset = caretNode.getAttribute("caretpos");
    }
    else if (!bFoundInputBox && searchNodes[jx].hasAttribute("tempinput") && searchNodes[jx].getAttribute("tempinput")=="true")
    {
      bFoundInputBox = bFoundNew = true;
      caretNode = searchNodes[jx];
      caretOffset = msiInputBoxCaretOffset;  //set to 1
    }
    else if ( !bFoundInputBox && !bFoundAny && ((searchNodes[jx].nodeType != Components.interfaces.nsIDOMNode.TEXT_NODE) || (searchNodes[jx].nodeValue.length > 0)) )
    {
      //Found a viable cursor position.
      caretNode = searchNodes[jx];
      bFoundAny = bFoundNew = true;
    }
    if (bFoundNew && caretNode != null) //In these cases we attempt to put the caret inside a child text node.
    {
      var nNonEmptyTextChild = -1;
      for (var kx = 0; kx < caretNode.childNodes.length; ++kx)
      {
        if (caretNode.childNodes[kx].nodeType != Components.interfaces.nsIDOMNode.TEXT_NODE)
        {
          nNonEmptyTextChild = -1;  //There are non-text children, so we don't want to assume the offset's intended to go inside a text child.
          break;
        }
        if (nNonEmptyTextChild < 0 && caretNode.childNodes[kx].nodeValue.length > 0)
          nNonEmptyTextChild = kx;
      }
      if (nNonEmptyTextChild >= 0)
        caretNode = caretNode.childNodes[nNonEmptyTextChild];
      if (caretOffset < 0)
        caretOffset = 0;
    }
  }
  if (caretNode != null)
  {
    foundCaret = new Object();
    foundCaret.theNode = caretNode;
    foundCaret.theOffset = caretOffset;
    if (bFoundCaretPos)
      foundCaret.theType = "caretpos";
    else if (bFoundInputBox)
      foundCaret.theType = "inputbox";
    else
      foundCaret.theType = "any";
  }
  return foundCaret;
}

//Not all of the following is probably necessary - the internalEditor flags are, however. Should be able to edit this
//function down a bit.
function msiEnableEditorControl(editorElement, bEnable)
{
  if (!editorElement)
  {
    dump("msiEnableEditorControl called on null editorElement!\n");
    return;
  }
  var internalEditor = editorElement.getEditor(editorElement.contentWindow);
  var elementStyle = editorElement.style;
  if (bEnable)
  {
    editorElement.removeAttribute("disabled");
    editorElement.allowevents = true;
    if (internalEditor != null)
    {
      msiDumpWithID("Got HTML editor for element [@] in msiEnableEditorControl; editor flags are [" + internalEditor.flags + "].\n", editorElement);
      internalEditor.flags &= ~(Components.interfaces.nsIPlaintextEditor.eEditorReadonlyMask | Components.interfaces.nsIPlaintextEditor.eEditorDisabledMask);
    }
    else
      msiDumpWithID("Unable to get HTML editor for element [@] in msiEnableEditorControl.\n", editorElement);
    if (elementStyle != null)
    {
//      dump("Original value of moz-user-focus on editor is " + elementStyle.getPropertyValue("-moz-user-focus") + ".\n");
      elementStyle.setProperty("-moz-user-focus", "normal", "");
      elementStyle.setProperty("-moz-user-input", "enabled", "");
    }
//    theEditor.setAttribute("-moz-user-focus", "normal");
  }
  else
  {
    editorElement.setAttribute("disabled", "true");
    editorElement.allowevents = false;
    if (internalEditor != null)
    {
      msiDumpWithID("Got HTML editor for element [@] in msiEnableEditorControl; editor flags are [" + internalEditor.flags + "].\n", editorElement);
      internalEditor.flags |= (Components.interfaces.nsIPlaintextEditor.eEditorReadonlyMask | Components.interfaces.nsIPlaintextEditor.eEditorDisabledMask);
    }
    else
      msiDumpWithID("Unable to get HTML editor for element [@] in msiEnableEditorControl.\n", editorElement);
    if (elementStyle != null)
    {
//      dump("Original value of moz-user-focus on editor is " + elementStyle.getPropertyValue("-moz-user-focus") + ".\n");
      elementStyle.setProperty("-moz-user-focus", "ignore", "");
      elementStyle.setProperty("-moz-user-input", "disabled", "");
    }
  }
//  else if (theEditor.hasAttribute("disabled"))
}
/************* Dialog management for editors ***************/

//A per-main-editor-window list of open dialogs which can have only a single invocation. This includes insertion uses of
//properties dialogs, document property dialogs, etc.
//The map will be indexed by dialog name strings. (What are these?)
function singleDialogList(theWindow)
{
  this.mWindow = theWindow;
  if (!this.ourList)
  {
    singleDialogList.prototype.ourList = new Object();
//    AlertWithTitle("Information", "singleDialogList.ourList created!");
  }
    
//  this.ourList = new Object();
  this.findDialog = function(dialogName)
  {
    var theDialog = null;
    if (this.ourList[dialogName])
      theDialog = this.ourList[dialogName].theDialog;
    return theDialog;
  };
  this.registerDialog = function(dialogName, theDialog, theCommand, editorElement)
  {
    this.ourList[dialogName] = new Object();
    this.ourList[dialogName].theDialog = theDialog;
    this.ourList[dialogName].theCommand = theCommand;
    this.ourList[dialogName].theEditor = editorElement;
//    if (!this.windowWatcher)
//      this.prototype.windowWatcher = Components.classes["@mozilla.org/embedcomp/window-watcher;1"].getService(Components.interfaces.nsIWindowWatcher);
    theDialog.addEventListener("unload", this.closingObserver, false);
  };
  this.closingObserver = function(event)
  {
//    msiKludgeLogString("singleDialogList closingObserver called.\n");
    var topWin = msiGetTopLevelWindow();
    if (topWin.msiSingleDialogList)
    {
      var theTarget = event.origTarget;
      if (!theTarget)
        theTarget = event.target;
      var theDialog = msiGetDialogContaining(theTarget);
      for (var entry in topWin.msiSingleDialogList.ourList)
      {
        if (topWin.msiSingleDialogList.ourList[entry].theDialog == theDialog)
        {
//          msiKludgeLogString("Removing dialog entry in closing observer.\n");
          delete topWin.msiSingleDialogList.ourList[entry];
          break;
        }
//        else
//          msiKludgeLogString("Dialog [" + theDialog + "] failed to match entry [" + topWin.msiSingleDialogList.ourList[entry] + "].\n");
      }
    }
  };
  this.reparentAppropriateDialogs = function(newActiveEditorElement)
  {
    if (!newActiveEditorElement)
      return;  //Only want to change parenting if focus is being set to a new editor.
    for (var entry in this.ourList)
    {
      if (msiEditorSupportsCommand(newActiveEditorElement, this.ourList[entry].theCommand) && 
            !msiEditorIsDependentOnWindow(newActiveEditorElement, this.ourList[entry].theDialog))
      {
        this.ourList[entry].theDialog.msiParentEditor = this.ourList[entry].theEditor = newActiveEditorElement;
        this.ourList[entry].theDialog.opener = msiGetWindowContainingEditor(newActiveEditorElement);
      }
    }
  };
  this.removeListeners = function()
  {
    for (var entry in this.ourList)
    {
      this.ourList[entry].theDialog.removeEventListener("unload", this.closingObserver, false);
    }
  };
//  this.getParentEditorElementByDialogName = function(dialogName)
//  {
//    var theEditorElement = null;
//    if (this.ourList[dialogName])
//      theEditorElement = this.ourList[dialogName].theEditor;
//    return theEditorElement;
//  };
  this.getParentEditorElementByDialog = function(theDialog)
  {
    for (var entry in this.ourList)
    {
      if (this.ourList[entry].theDialog == theDialog)
      {
        return this.ourList[entry].theEditor;
      }
    }
    return null;
  };
}

function msiGetDialogContaining(aNode)
{
  var parentWindow = null;
  var theDocument = null;
  if ("documentElement" in aNode)
  {
    if (aNode.documentElement.nodeName == "dialog")
      return aNode.defaultView;
    aNode = aNode.defaultView;
  }
  if ("opener" in aNode)
  {
    parentWindow = aNode;
    if (aNode.parent != aNode)
      parentWindow = aNode.parent;
    if (parentWindow.document.documentElement.nodeName == "dialog")
      return parentWindow;
  }
  if ("ownerDocument" in aNode && aNode.ownerDocument != null && aNode.ownerDocument.documentElement.nodeName == "dialog")
    return aNode.ownerDocument.defaultView;
  return null;
};

function msiEditorIsDependentOnWindow(editorElement, theWindow)
{
  var currParentWin = editorElement.ownerDocument.defaultView;
  var currWindow = editorElement.contentWindow;
  var bIsDependent = (currWindow == theWindow) || (currParentWin == theWindow);
  while (!bIsDependent && currParentWin!=null)
  {
    if (currParentWin.parent && currParentWin.parent != currParentWin)
      currParentWin = currParentWin.parent;
    else
      currParentWin = currParentWin.opener;
    bIsDependent = (currParentWin == theWindow);
    if (!bIsDependent && currParentWin)
    {
      var parentEditor = findEditorElementForDocument(currParentWin.document);
      if (parentEditor != null)
      {
        currWindow = currParentWin;
        currParentWin = parentEditor.ownerDocument.defaultView;
        if (currParentWin.top)
          currParentWin = currParentWin.top;
        bIsDependent = (currParentWin == theWindow);
      }
    }
  }
  return bIsDependent;
}

function msiEditorSupportsCommand(editorElement, commandStr)
{
  try
  {
    if ( editorElement.contentWindow && editorElement.contentWindow.controllers
           && editorElement.contentWindow.controllers.getControllerForCommand(commandStr) )
      return true;
  }
  catch(exc) {}
  return false;
}

function msiLaunchSingleInstanceDialog(chromeUrl, dlgName, options, targetEditor, commandID, extraArgsArray)
{
  var parentWindow = msiGetWindowContainingEditor(targetEditor);
  var theDialog = null;
  var numArgs = extraArgsArray.length;
  if (numArgs > 5)
  {
    AlertWithTitle("Code needs repair", "Too many arguments in the array passed to msiLaunchSingleInstanceDialog!");
    numArgs = 5;
  }
  switch(numArgs)
  {
    case 0:  //the minimum - no extra args
      theDialog = parentWindow.openDialog(chromeUrl, dlgName, options);
    break;
    case 1:  //One extra arg
      theDialog = parentWindow.openDialog(chromeUrl, dlgName, options, extraArgsArray[0]);
    break;
    case 2:
      theDialog = parentWindow.openDialog(chromeUrl, dlgName, options, extraArgsArray[0], extraArgsArray[1]);
    break;
    case 3:
      theDialog = parentWindow.openDialog(chromeUrl, dlgName, options, extraArgsArray[0], extraArgsArray[1], extraArgsArray[2]);
    break;
    case 4:
      theDialog = parentWindow.openDialog(chromeUrl, dlgName, options, extraArgsArray[0], extraArgsArray[1], extraArgsArray[2], extraArgsArray[3]);
    break;
    case 5:
      theDialog = parentWindow.openDialog(chromeUrl, dlgName, options, extraArgsArray[0], extraArgsArray[1], extraArgsArray[2], extraArgsArray[3], extraArgsArray[4]);
    break;
  }
  if (theDialog)
  {  
    theDialog.msiParentEditor = targetEditor;
    var topWindow = msiGetTopLevelWindow(window);
    msiRegisterSingleInstanceDialog(topWindow, chromeUrl, theDialog, commandID, targetEditor);
  }
  return theDialog;
}

function msiLaunchPropertiesDialog(chromeUrl, dlgName, options, targetEditorElement, commandID, reviseObject, extraArgsArray)
{
  var parentWindow = msiGetWindowContainingEditor(targetEditorElement);
  var theDialog = null;
  var numArgs = extraArgsArray.length;
  if (numArgs > 5)
  {
    AlertWithTitle("Code needs repair", "Too many arguments in the array passed to msiLaunchPropertiesDialog!");
    numArgs = 5;
  }
  switch(numArgs)
  {
    case 0:  //the minimum - no extra args
      theDialog = parentWindow.openDialog(chromeUrl, dlgName, options);
    break;
    case 1:  //One extra arg
      theDialog = parentWindow.openDialog(chromeUrl, dlgName, options, extraArgsArray[0]);
    break;
    case 2:
      theDialog = parentWindow.openDialog(chromeUrl, dlgName, options, extraArgsArray[0], extraArgsArray[1]);
    break;
    case 3:
      theDialog = parentWindow.openDialog(chromeUrl, dlgName, options, extraArgsArray[0], extraArgsArray[1], extraArgsArray[2]);
    break;
    case 4:
      theDialog = parentWindow.openDialog(chromeUrl, dlgName, options, extraArgsArray[0], extraArgsArray[1], extraArgsArray[2], extraArgsArray[3]);
    break;
    case 5:
      theDialog = parentWindow.openDialog(chromeUrl, dlgName, options, extraArgsArray[0], extraArgsArray[1], extraArgsArray[2], extraArgsArray[3], extraArgsArray[4]);
    break;
  }
  if (theDialog)
  {  
    theDialog.msiParentEditor = targetEditorElement;
    var topWindow = msiGetTopLevelWindow(window);
    msiRegisterPropertiesDialog(topWindow, chromeUrl, theDialog, commandID, targetEditorElement, reviseObject);
  }
  return theDialog;
}

function msiOpenModelessDialog(chromeUrl, dlgName, options, targetEditorElement, commandID, commandHandler)
{
  var editor = msiGetEditor(targetEditorElement);
  var reviseObject = null;
  if (commandHandler && ("msiGetReviseObject" in commandHandler))
    reviseObject = commandHandler.msiGetReviseObject(editor);
  var extraArgsArray = new Array();
  for (var i = 6; i < arguments.length; ++i)
  {
    extraArgsArray.push(arguments[i]);
  }
  if (reviseObject != null)
    return msiOpenModelessPropertiesDialog(chromeUrl, dlgName, options, targetEditorElement, commandID, reviseObject, extraArgsArray);
  else
    return msiOpenSingleInstanceModelessDialog(chromeUrl, dlgName, options, targetEditorElement, commandID, extraArgsArray);
}

function msiOpenSingleInstanceModelessDialog(chromeUrl, dlgName, options, targetEditor, commandID, extraArgsArray)
{
//  var extraArgsArray = new Array();
//  for (var i = 5; i < arguments.length; ++i)
//  {
//    extraArgsArray.push(arguments[i]);
//  }
  var dialog = msiFindSingleInstanceDialog(chromeUrl);
  if (dialog)
  {
    //Reparent the dialog to targetEditor if possible
      //If not, can - or should - we reparent the dialog containing the targetEditor to somewhere else then?
      //Leave this code commented out for now! May decide to implement it after trying out first attempts...
//    if (msiEditorIsDependentOnWindow(targetEditor, dialog))
//    {
//      var ancestorEditor = msiGetParentEditor(dialog);
//      msiSetParentEditor(ancestorEditor, dialog);
//      if (!msiEditorIsDependentOnWindow(targetEditor, dialog))
//      {
//        var ancestorWindow = msiGetWindowContainingEditor(ancestorEditor);
//        dialog.opener = ancestorWindow;
//      }
//    }
    //Now we repeat the test, since we may have fixed the problem
    if (!msiEditorIsDependentOnWindow(targetEditor, dialog))
    {
      dialog.msiParentEditor = targetEditor;
      dialog.opener = msiGetWindowContainingEditor(targetEditor);
    }
  }
  else
  {
    dialog = msiLaunchSingleInstanceDialog(chromeUrl, dlgName, options, targetEditor, commandID, extraArgsArray);
  }
  if (dialog)
    dialog.focus();
}

function msiFindSingleInstanceDialog(chromeUrl)
{
  var theDialog = null;
  var theWindow = msiGetTopLevelWindow(window);
  if (!theWindow.msiSingleDialogList)
    theWindow.msiSingleDialogList = new singleDialogList(window);
  return theWindow.msiSingleDialogList.findDialog(chromeUrl);
}

function msiOpenModelessPropertiesDialog(chromeUrl, dlgName, options, targetEditorElement, commandID, reviseObject, extraArgsArray)
{
  if (!reviseObject)
  {
    AlertWithTitle("Error in msiOpenModelessPropertiesDialog!", "No object to revise for dialog [" + chromeUrl + "]");
    msiOpenSingleInstanceModelessDialog(chromeUrl, dlgName, options, targetEditorElement, commandID, extraArgsArray);
    return;
  }
  var theDialog = null;
//  reviseObject = commandHandler.msiGetReviseObject(editor);
  theDialog = msiFindPropertiesDialogForObject(reviseObject);
  if (!theDialog)
  {
    theDialog = msiLaunchPropertiesDialog(chromeUrl, dlgName, options, targetEditorElement, commandID, reviseObject, extraArgsArray);
  }
  if (theDialog)
    theDialog.focus();
}


function msiFindPropertiesDialogForObject(reviseObject)
{
  var theWindow = msiGetTopLevelWindow(window);
  if (!theWindow.msiPropertiesDialogList)
    theWindow.msiPropertiesDialogList = new propertyDialogList(window);
  return theWindow.msiPropertiesDialogList.findDialogForObject(reviseObject);
}


function msiRegisterSingleInstanceDialog(topWindow, dlgNameToUse, theDialog, commandID, editorElement)
{
  if (!topWindow.msiSingleDialogList)
    topWindow.msiSingleDialogList = new singleDialogList(topWindow);
  topWindow.msiSingleDialogList.registerDialog(dlgNameToUse, theDialog, commandID, editorElement);
}

function msiRegisterPropertiesDialog(topWindow, dlgNameToUse, theDialog, commandID, editorElement, reviseObject)
{
  if (!topWindow.msiPropertiesDialogList)
    topWindow.msiPropertiesDialogList = new propertyDialogList(topWindow);
  topWindow.msiPropertiesDialogList.registerDialog(dlgNameToUse, theDialog, commandID, editorElement, reviseObject);
}

function msiGetWindowContainingEditor(editorElement)
{
  //How do we do this??
//  var theWindow = editorElement.contentWindow;
////Logging stuff only
//  var logStr = "In msiGetWindowContainingEditor, editorElement [" + editorElement.id + "] has contentWindow [";
//  if (theWindow.id)
//    logStr += theWindow.id;
//  else
//    logStr += theWindow;
//  logStr += "], while contentWindow.parent is [";
//  if (theWindow.parent.id)
//    logStr += theWindow.parent.id;
//  else
//    logStr += theWindow.parent;
//  logStr += "],\n  contentWindow.opener is [";
//  if (theWindow.opener)
//  {
//    if (theWindow.opener.id)
//      logStr += theWindow.opener.id;
//    else
//      logStr += theWindow.opener;
//  }
//  logStr += "], contentWindow.windowRoot is [";
//  if (theWindow.windowRoot)
//  {
//    if (theWindow.windowRoot.id)
//      logStr += theWindow.windowRoot.id;
//    else
//      logStr += theWindow.windowRoot;
//  }
//  logStr += "], and contentWindow.frameElement is [";
//  if (theWindow.frameElement)
//  {
//    if (theWindow.frameElement.id)
//      logStr += theWindow.frameElement.id;
//    else
//      logStr += theWindow.frameElement.nodeName;
//  }
//  logStr += "].\n";
//  msiKludgeLogString(logStr);
////End logging stuff
  if (!editorElement)
    return null;
  var parWindow = editorElement.ownerDocument.defaultView;
  return parWindow;
//  if (parWindow != theWindow)
//    return parWindow;
//  //else???? for now, assume this always works?
//  var theWindow = editorElement.contentWindow;
//  if (theWindow.parent && theWindow.parent != theWindow)
//    return theWindow.parent;
//  return theWindow;
}

function msiGetParentWindowForNewDialog(ownerEditorElement)
{
  return msiGetWindowContainingEditor(ownerEditorElement);
}

function ShutdownEditorsForWindow(theWindow)
{
  if (!theWindow)
    theWindow = window;
  var editorList = theWindow.document.getElementsByTagName("editor");
  for (var i = 0; i < editorList.length; ++i)
    ShutdownAnEditor(editorList[i]);
}

//A per-editor list of open properties-type dialogs referencing existing nodes in the DOM.
function propertyDialogList(theWindow)
{
  this.mWindow = theWindow;
  if (!this.ourList)
  {
    propertyDialogList.prototype.ourList = new Object();
//    AlertWithTitle("Information", "propertyDialogList.ourList created!");
  }
    
  this.findDialogForObject = function(dialogName, reviseObject)
  {
    var theDialog = null;
    if (this.ourList[dialogName])
    {
      for (var i = 0; i < this.ourList[dialogName].length; ++i)
      {
        if (this.ourList[dialogName][i].theObject == reviseObject)
        {
          theDialog = this.ourList[dialogName][i].theDialog;
          break;
        }
      }
    }
    return theDialog;
  };
  this.registerDialog = function(dialogName, theDialog, theCommand, editorElement, reviseObject)
  {
    if (!this.ourList[dialogName])
      this.ourList[dialogName] = new Array();
    var theEntry = new Object();
    theEntry.theDialog = theDialog;
    theEntry.theCommand = theCommand;
    theEntry.theEditor = editorElement;
    theEntry.theObject = reviseObject;
    this.ourList[dialogName].push(theEntry);
//    if (!this.windowWatcher)
//      this.prototype.windowWatcher = Components.classes["@mozilla.org/embedcomp/window-watcher;1"].getService(Components.interfaces.nsIWindowWatcher);
    theDialog.addEventListener("unload", this.closingObserver, false);
  };
  this.closingObserver = function(event)
  {
//    msiKludgeLogString("singleDialogList closingObserver called.\n");
    var topWin = msiGetTopLevelWindow();
    if (topWin.msiPropertiesDialogList)
    {
      var theTarget = event.origTarget;
      if (!theTarget)
        theTarget = event.target;
      var theDialog = msiGetDialogContaining(theTarget);
      var bFound = false;
      for (var entry in topWin.msiPropertiesDialogList.ourList)
      {
        for (var i = 0; i < topWin.msiPropertiesDialogList.ourList[entry].length; ++i)
        {
          if (topWin.msiPropertiesDialogList.ourList[entry][i].theDialog == theDialog)
          {
  //          msiKludgeLogString("Removing dialog entry in closing observer.\n");
            delete topWin.msiPropertiesDialogList.ourList[entry][i];
            bFound = true;
            if (topWin.msiPropertiesDialogList.ourList[entry].length == 0)
              delete topWin.msiPropertiesDialogList.ourList[entry];
            break;
          }
        }
        if (bFound)
          break;
      }
    }
  };
//  this.reparentAppropriateDialogs = function(newActiveEditorElement)
//  {
//    if (!newActiveEditorElement)
//      return;  //Only want to change parenting if focus is being set to a new editor.
//    for (var entry in this.ourList)
//    {
//      if (msiEditorSupportsCommand(newActiveEditorElement, this.ourList[entry].theCommand) && 
//            !msiEditorIsDependentOnWindow(newActiveEditorElement, this.ourList[entry].theDialog))
//      {
//        this.ourList[entry].theDialog.msiParentEditor = this.ourList[entry].theEditor = newActiveEditorElement;
//        this.ourList[entry].theDialog.opener = msiGetWindowContainingEditor(newActiveEditorElement);
//      }
//    }
//  }
  this.removeListeners = function()
  {
    for (var entry in this.ourList)
    {
      for (var i = 0; i < this.ourList[entry].length; ++i)
        this.ourList[entry][i].theDialog.removeEventListener("unload", this.closingObserver, false);
    }
  }
  this.getParentEditorElementByDialog = function(theDialog)
  {
    for (var entry in this.ourList)
    {
      for (var i = 0; i < this.ourList[entry].length; ++i)
      {
        if (this.ourList[entry][i].theDialog == theDialog)
        {
//          msiKludgeLogString("Removing dialog entry in closing observer.\n");
          return this.ourList[entry][i].theEditor;
          break;
        }
      }
    }
    return null;
  }
}


//Utility for use by editors in dialogs.
function msiGetParentEditorElementForDialog(dialogWindow)
{
  if (!dialogWindow)
    dialogWindow = window;
  if (dialogWindow.top && (dialogWindow != dialogWindow.top))
    dialogWindow = dialogWindow.top;
  var editorElement = null;
  if ("msiParentEditor" in dialogWindow)
    editorElement = dialogWindow.msiParentEditor;
  if (!editorElement)
  {
    var topWindow = msiGetTopLevelWindow(dialogWindow);
    if (topWindow.msiSingleDialogList)
      editorElement = topWindow.msiSingleDialogList.getParentEditorElementByDialog(dialogWindow);
    if (!editorElement && topWindow.msiPropertiesDialogList)
      editorElement = topWindow.msiPropertiesDialogList.getParentEditorElementByDialog(dialogWindow);
  }
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  return editorElement;
}


/************* General editing command utilities ***************/

function msiGetDocumentTitle(editorElement)
{
  var title = document.getElementById("filename").value;
  if (title.length > 0) return title;
  try {
    return new XPCNativeWrapper(msiGetEditor(editorElement).document, "title").title;
  } catch (e) {}

  return "";
}

function msiSetDocumentTitle(editorElement, title)
{
  // if we changed the name of a shell document, we saved the filename in 
  // a broadcaster with id="filename"
  var newtitle = document.getElementById("filename").value;
  if (newtitle.length > 0) title = newtitle;
  try {
    msiGetEditor(editorElement).setDocumentTitle(title);

    // Update window title (doesn't work if called from a dialog)
    if ("UpdateWindowTitle" in window)
    {
      window.UpdateWindowTitle();
    }
  } catch (e) {}
}

//var gAtomService;
//function GetAtomService()
//{
//  gAtomService = Components.classes["@mozilla.org/atom-service;1"].getService(Components.interfaces.nsIAtomService);
//}

function msiEditorGetTextProperty(editorElement, property, attribute, value, firstHas, anyHas, allHas)
{
  try {
    if (!gAtomService) GetAtomService();
    var propAtom = gAtomService.getAtom(property);

    msiGetEditor(editorElement).getInlineProperty(propAtom, attribute, value,
                                         firstHas, anyHas, allHas);
  }
  catch(e) {}
}

function msiEditorSetTextProperty(editorElement, property, attribute, value)
{
  try {
    if (!gAtomService) GetAtomService();
    var propAtom = gAtomService.getAtom(property);

    msiGetEditor(editorElement).setInlineProperty(propAtom, attribute, value);
    if (!msiCurrEditorSetFocus(window) && "gContentWindow" in window)
      window.gContentWindow.focus();
  }
  catch(e) {}
}

function msiEditorRemoveTextProperty(editorElement, property, attribute)
{
  try {
    if (!gAtomService) GetAtomService();
    var propAtom = gAtomService.getAtom(property);

    msiGetEditor(editorElement).removeInlineProperty(propAtom, attribute);
    if (!msiCurrEditorSetFocus(window) && "gContentWindow" in window)
      window.gContentWindow.focus();
  }
  catch(e) {}
}

/************* Element enbabling/disabling ***************/

// this function takes an elementID and a flag
// if the element can be found by ID, then it is either enabled (by removing "disabled" attr)
// or disabled (setAttribute) as specified in the "doEnable" parameter
function msiSetRelevantElementsEnabledById(elementID, doEnable)
{
  var editorElement = msiGetActiveEditorElement();
  var docList = msiGetUpdatableItemContainers(elementID, editorElement);
  for (var i = 0; i < docList.length; ++i)
  {
    docList[i].defaultView.SetElementEnabled(docList[i].getElementById(elementID), doEnable);
  }
}

function SetElementEnabledById(elementID, doEnable)
{
  SetElementEnabled(document.getElementById(elementID), doEnable);
}

function SetElementEnabled(element, doEnable)
{
  if ( element )
  {
    if ( doEnable )
      element.removeAttribute("disabled");
    else
      element.setAttribute("disabled", "true");
  }
  else
  {
    dump("Element null in SetElementEnabled\n");
  }
}

/************* Services / Prefs ***************/

function msiGetIOService()
{
  if (gIOService)
    return gIOService;

  gIOService = Components.classes["@mozilla.org/network/io-service;1"]
               .getService(Components.interfaces.nsIIOService);

  return gIOService;
}

function msiGetFileProtocolHandler()
{
  var ios = msiGetIOService();
  var handler = ios.getProtocolHandler("file");
  return handler.QueryInterface(Components.interfaces.nsIFileProtocolHandler);
}

function GetPrefsService()
{
  if (gPrefsService)
    return gPrefsService;

  try {
    gPrefsService = Components.classes["@mozilla.org/preferences-service;1"].getService(Components.interfaces.nsIPrefService);
  }
  catch(ex) {
    dump("failed to get prefs service!\n");
  }

  return gPrefsService;
}

function GetPrefs()
{
  if (gPrefsBranch)
    return gPrefsBranch;

  try {
    var prefService = GetPrefsService();
    if (prefService)
      gPrefsBranch = prefService.getBranch(null);

    if (gPrefsBranch)
      return gPrefsBranch;
    else
      dump("failed to get root prefs!\n");
  }
  catch(ex) {
    dump("failed to get root prefs!\n");
  }
  return null;
}

function GetStringPref(name)
{
  try {
    return GetPrefs().getComplexValue(name, Components.interfaces.nsISupportsString).data;
  } catch (e) {}
  return "";
}

function GetBoolPref(name)
{
  try {
    return GetPrefs().getBoolPref(name);
  } catch (e) {}
  return false;
}

function PrefHasValue(name)
{
  try
  {
    return GetPrefs().prefHasUserValue(name);
  } catch(exc) {dump("Exception trying to query whether pref " + name + " is set: [" + exc + "].\n");}
  return false;
}

function GetIntPref(name)
{
  try
  {
    return GetPrefs().getIntPref(name);
  } catch(exc) {dump("Exception trying to get int pref " + name + ": [" + exc + "].\n");}
  return 0;
}

function SetIntPref(name, value)
{
  var prefs = GetPrefs();
  if (prefs)
    prefs.setIntPref(name, value);
}

function SetUnicharPref(aPrefName, aPrefValue)
{
  var prefs = GetPrefs();
  if (prefs)
  {
    try {
      var str = Components.classes["@mozilla.org/supports-string;1"]
                          .createInstance(Components.interfaces.nsISupportsString);
      str.data = aPrefValue;
      prefs.setComplexValue(aPrefName, Components.interfaces.nsISupportsString, str);
    }
    catch(e) {}
  }
}

function GetUnicharPref(aPrefName, aDefVal)
{
  var prefs = GetPrefs();
  if (prefs)
  {
    try {
      return prefs.getComplexValue(aPrefName, Components.interfaces.nsISupportsString).data;
    }
    catch(e) {}
  }
  return "";
}

// Set initial directory for a filepicker from URLs saved in prefs
function msiSetFilePickerDirectory(filePicker, fileType)
{
  if (filePicker)
  {
    try {
      var prefBranch = GetPrefs();
      if (prefBranch)
      {
        // Save current directory so we can reset it in SaveFilePickerDirectory
        window.gFilePickerDirectory = filePicker.displayDirectory;

        var location = prefBranch.getComplexValue("editor.lastFileLocation."+fileType, Components.interfaces.nsILocalFile);
        if (location)
          filePicker.displayDirectory = location;
      }
    }
    catch(e) {}
  }
}

// Save the directory of the selected file to prefs
function msiSaveFilePickerDirectory(filePicker, fileType)
{
  if (filePicker && filePicker.file)
  {
    try {
      var prefBranch = GetPrefs();

      var fileDir;
      if (filePicker.file.parent)
        fileDir = filePicker.file.parent.QueryInterface(Components.interfaces.nsILocalFile);

      if (prefBranch)
       prefBranch.setComplexValue("editor.lastFileLocation."+fileType, Components.interfaces.nsILocalFile, fileDir);
    
      var prefsService = GetPrefsService();
        prefsService.savePrefFile(null);
    } catch (e) {}
  }

  // Restore the directory used before SetFilePickerDirectory was called;
  // This reduces interference with Browser and other module directory defaults
  if ("gFilePickerDirectory" in window)
    filePicker.displayDirectory = window.gFilePickerDirectory;

  window.gFilePickerDirectory = null;
}


// Save the directory of the selected file to prefs (Extended so we can choose a parent of the selected file)
function msiSaveFilePickerDirectoryEx(filePicker, path, fileType)
{
  if (filePicker && path)
  {
    try {
      var prefBranch = GetPrefs();

      var fileDir = Components.classes["@mozilla.org/file/local;1"].
          createInstance(Components.interfaces.nsILocalFile);
      fileDir.initWithPath(path);
      fileDir = fileDir.QueryInterface(Components.interfaces.nsILocalFile);

      if (prefBranch)
       prefBranch.setComplexValue("editor.lastFileLocation."+fileType, Components.interfaces.nsILocalFile, fileDir);
    
      var prefsService = GetPrefsService();
        prefsService.savePrefFile(null);
    } catch (e) {}
  }

  // Restore the directory used before SetFilePickerDirectory was called;
  // This reduces interference with Browser and other module directory defaults
  if ("gFilePickerDirectory" in window)
    filePicker.displayDirectory = window.gFilePickerDirectory;

  window.gFilePickerDirectory = null;
}

function msiCopyFileAndDirectoryToBak( documentfile ) // an nsILocalFile
{
  try {
  // new scheme: save the bak files with no name change in the ..._files/bak directory
    var leafname = documentfile.leafName;
    var i = leafname.lastIndexOf(".");
    if (i > 0) leafname = leafname.substr(0,i);
    var parentdir = documentfile.parent.clone();
    var _filesdir = parentdir.clone();
    _filesdir.append(leafname+"_files");
    var tempbakdir = parentdir.clone();
    tempbakdir.append("bak");
    var finalbakdir = _filesdir.clone();
    finalbakdir.append("bak");
  // nuke current bak dir contents if there are any
    if (finalbakdir.exists()) finalbakdir.remove(true);
  // copy stuff to temp bak dir
    documentfile.copyTo(tempbakdir, documentfile.leafName);
  // copy _files to temp bak dir
    _filesdir.copyTo(tempbakdir, leafname+"_files");
  // We can copy the 'revert' directory to 'bak' or we can copy 'bak' to 'revert',
  // but we can't do both without getting recursion. So delete the 'revert' directory
  // that we just copied.
    var revertdir = tempbakdir.clone();
    revertdir.append(leafname+"_files");
    revertdir.append("revert");
    if (revertdir.exists()) revertdir.remove(true);
    
  // now move the temp dir to ..._files/bak
    tempbakdir.moveTo(_filesdir, "bak"); 
  }
  catch (e) {
    dump("msiCopyFileAndDirectoryToBak failed: "+e+"\n");
  }
}

function msiCopyAuxDirectory( originalfile, newfile ) // both nsILocalFiles
{
  try
  {
    var filename = originalfile.leafName;
    var newfilename = newfile.leafName;
    var i = filename.lastIndexOf(".");
    if (i >= 0) filename = filename.substr(0,i);
    i = newfilename.lastIndexOf(".");
    if (i >= 0) newfilename = newfilename.substr(0,i);
    filename += "_files";
    newfilename += "_files";
    var parentdir = originalfile.parent.clone();
    var newdir = newfile.parent.clone();
    var testfile = newdir.clone();
    testfile.append(newfilename);
    if (testfile.exists()) testfile.remove();
    parentdir.append(filename);
    parentdir.copyTo(newdir, newfilename);
  }
  catch(e) {
    dump("unable to copy aux directory: "+e+"\n");
  }
}

// This replaces a file with its backup file, and the same for its associated directory
// The File/Revert calls this function, but also reloads the reverted file. The pre-reverted
// file is saved in the revert directory
// If there is no backup file (i.e., the file has never been changed) and delete==true, then
// we delete the file and its associated directory.

// BBM this can fail if the file has been renamed, so the .sci file in the bak directory has
// a different name. We should fall back to searching for the only .sci file or the only directory
// in the bak directory.

function msiRevertFile (documentfile, del) // an nsILocalFile
{
  try {
  // new scheme: save the bak files with no name change in the ..._files/bak directory
    var leafname = documentfile.leafName;
    var i = leafname.lastIndexOf(".");
    if (i > 0) leafname = leafname.substr(0,i);
    var templeafname = leafname+"_temp";
    var parentdir = documentfile.parent.clone();
    var tempdir = parentdir.clone();
    tempdir.append(templeafname+"_files");
    if (tempdir.exists()) tempdir.remove(true);
    var _filesdir = parentdir.clone();
    _filesdir.append(leafname+"_files");
    var bakdir = _filesdir.clone();
    bakdir.append("bak");
    if (_filesdir.exists() && !bakdir.exists() && del)
    {
      // reverting a file that has not ever been saved
      _filesdir.remove(true);
      documentfile.remove(false);
    }
    else
    {
      var working = bakdir.clone();
      working.append(leafname+"."+MSI_EXTENSION);
      // copy doc_files/bak/doc.sci to doc_temp.sci
      working.moveTo(parentdir,templeafname+"."+MSI_EXTENSION);
      working = bakdir.clone();
      working.append(leafname + "_files");                     
      // copy doc_files/bak/doc_files to doc_temp_files
      working.moveTo(parentdir, templeafname + "_files");
      // remove bak and revert for doc_temp_files
      vorking = tempdir.clone();
      working.append("bak");
      if (working.exists()) working.remove(true);
      working = tempdir.clone();
      working.append("revert");
      if (working.exists()) working.remove(true);
      working = tempdir.clone();
      working.append("revert");
      working.create(1, 0755);
      // save current document and doc_files in the new revert directory
      documentfile.moveTo(working, null);
      _filesdir.moveTo(working, null);
      // rename the temp files
      tempdir.moveTo(parentdir, leafname+"_files");  
      working = parentdir.clone();
      working.append(templeafname + "." + MSI_EXTENSION);
      working.moveTo(parentdir, leafname + "." + MSI_EXTENSION);
    }
  }
  catch(e) {
    dump("msiRevertFile failed: "+e+"\n");
  }
}

function GetDefaultBrowserColors()
{
  var prefs = GetPrefs();
  var colors = { TextColor:0, BackgroundColor:0, LinkColor:0, ActiveLinkColor:0 , VisitedLinkColor:0 };
  var useSysColors = false;
  try { useSysColors = prefs.getBoolPref("browser.display.use_system_colors"); } catch (e) {}
                        
  if (!useSysColors)
  {
    try { colors.TextColor = prefs.getCharPref("browser.display.foreground_color"); } catch (e) {}

    try { colors.BackgroundColor = prefs.getCharPref("browser.display.background_color"); } catch (e) {}
  }
  // Use OS colors for text and background if explicitly asked or pref is not set
  if (!colors.TextColor)
    colors.TextColor = "windowtext";

  if (!colors.BackgroundColor)
    colors.BackgroundColor = "window";

  colors.LinkColor = prefs.getCharPref("browser.anchor_color");
  colors.ActiveLinkColor = prefs.getCharPref("browser.active_color");
  colors.VisitedLinkColor = prefs.getCharPref("browser.visited_color");

  return colors;
}

/************* URL handling ***************/

function TextIsURI(selectedText)
{
  return selectedText && /^http:\/\/|^https:\/\/|^file:\/\/|\
    ^ftp:\/\/|^about:|^mailto:|^news:|^snews:|^telnet:|^ldap:|\
    ^ldaps:|^gopher:|^finger:|^javascript:/i.test(selectedText);
}

function IsUrlUntitled(urlString)
{
  var re = new RegExp("/"+GetString("untitled")+"[0-9]*\.", "i");
  return re.test(urlString);
}


function IsUrlAboutBlank(urlString)
{
  return (urlString == "about:blank");
}

function msiMakeRelativeUrl(url, editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();

  var inputUrl = TrimString(url);
  if (!inputUrl)
    return inputUrl;

  // Get the filespec relative to current document's location
  // NOTE: Can't do this if file isn't saved yet!
  var docUrl = msiGetDocumentBaseUrl(editorElement);
  return msiMakeUrlRelativeTo(inputUrl, docUrl);
}

function msiMakeUrlRelativeTo(inputUrl, baseUrl, editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();

  var baseScheme = GetScheme(baseUrl);

  // Can't relativize if no base scheme (page hasn't been saved)
  if (!baseScheme)
    return inputUrl;

  var urlScheme = GetScheme(inputUrl);

  // Do nothing if not the same scheme or url is already relativized
  if (baseScheme != urlScheme)
    return inputUrl;

  var IOService = msiGetIOService();
  if (!IOService)
    return inputUrl;

  // Host must be the same
  var baseHost = GetHost(baseUrl);
  var urlHost = GetHost(inputUrl);
  if (baseHost != urlHost)
    return inputUrl;


  // Get just the file path part of the urls
  // XXX Should we use GetCurrentEditor().documentCharacterSet for 2nd param ?
  var basePath = IOService.newURI(baseUrl, msiGetEditor(editorElement).documentCharacterSet, null).path;
  var urlPath = IOService.newURI(inputUrl, msiGetEditor(editorElement).documentCharacterSet, null).path;

  // We only return "urlPath", so we can convert
  //  the entire basePath for case-insensitive comparisons
  var os = GetOS();
  var doCaseInsensitive = (baseScheme == "file" && os == msigWin);
  if (doCaseInsensitive)
    basePath = basePath.toLowerCase();

  // Get base filename before we start chopping up the basePath
  var baseFilename = GetFilename(basePath);

  // Both url and base paths now begin with "/"
  // Look for shared dirs starting after that
  urlPath = urlPath.slice(1);
  basePath = basePath.slice(1);

  var firstDirTest = true;
  var nextBaseSlash = 0;
  var done = false;

  // Remove all matching subdirs common to both base and input urls
  do {
    nextBaseSlash = basePath.indexOf("\/");
    var nextUrlSlash = urlPath.indexOf("\/");

    if (nextUrlSlash == -1)
    {
      // We're done matching and all dirs in url
      // what's left is the filename
      done = true;

      // Remove filename for named anchors in the same file
      if (nextBaseSlash == -1 && baseFilename)
      { 
        var anchorIndex = urlPath.indexOf("#");
        if (anchorIndex > 0)
        {
          var urlFilename = doCaseInsensitive ? urlPath.toLowerCase() : urlPath;
        
          if (urlFilename.indexOf(baseFilename) == 0)
            urlPath = urlPath.slice(anchorIndex);
        }
      }
    }
    else if (nextBaseSlash >= 0)
    {
      // Test for matching subdir
      var baseDir = basePath.slice(0, nextBaseSlash);
      var urlDir = urlPath.slice(0, nextUrlSlash);
      if (doCaseInsensitive)
        urlDir = urlDir.toLowerCase();

      if (urlDir == baseDir)
      {

        // Remove matching dir+"/" from each path
        //  and continue to next dir
        basePath = basePath.slice(nextBaseSlash+1);
        urlPath = urlPath.slice(nextUrlSlash+1);
      }
      else
      {
        // No match, we're done
        done = true;

        // Be sure we are on the same local drive or volume 
        //   (the first "dir" in the path) because we can't 
        //   relativize to different drives/volumes.
        // UNIX doesn't have volumes, so we must not do this else
        //  the first directory will be misinterpreted as a volume name
        if (firstDirTest && baseScheme == "file" && os != msigUNIX)
          return inputUrl;
      }
    }
    else  // No more base dirs left, we're done
      done = true;

    firstDirTest = false;
  }
  while (!done);

  // Add "../" for each dir left in basePath
  while (nextBaseSlash > 0)
  {
    urlPath = "../" + urlPath;
    nextBaseSlash = basePath.indexOf("\/", nextBaseSlash+1);
  }
  return urlPath;
}

function msiMakeAbsoluteUrl(url, editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();

  var resultUrl = TrimString(url);
  if (!resultUrl)
    return resultUrl;

  // Check if URL is already absolute, i.e., it has a scheme
  var urlScheme = GetScheme(resultUrl);

  if (urlScheme)
    return resultUrl;

  var docUrl = msiGetDocumentBaseUrl(editorElement);
  var docScheme = GetScheme(docUrl);

  // Can't relativize if no doc scheme (page hasn't been saved)
  if (!docScheme)
    return resultUrl;

  var  IOService = msiGetIOService();
  if (!IOService)
    return resultUrl;
  
  // Make a URI object to use its "resolve" method
  var absoluteUrl = resultUrl;
  var docUri = IOService.newURI(docUrl, msiGetCurrentEditor().documentCharacterSet, null);

  try {
    absoluteUrl = docUri.resolve(resultUrl);
    // This is deprecated and buggy! 
    // If used, we must make it a path for the parent directory (remove filename)
    //absoluteUrl = IOService.resolveRelativePath(resultUrl, docUrl);
  } catch (e) {}

  return absoluteUrl;
}

// Get the HREF of the page's <base> tag or the document location
// returns empty string if no base href and document hasn't been saved yet
function msiGetDocumentBaseUrl(editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  try {
    var docUrl;

    // if document supplies a <base> tag, use that URL instead 
    var baseList = msiGetEditor(editorElement).document.getElementsByTagName("base");
    if (baseList)
    {
      var base = baseList.item(0);
      if (base)
        docUrl = base.getAttribute("href");
    }
    if (!docUrl)
      docUrl = msiGetEditorURL(editorElement);

//    if (!IsUrlAboutBlank(docUrl))
      return docUrl;
  } catch (e) {}
  return "";
}

//function GetDocumentUrl()
//{
//  try {
//    var aDOMHTMLDoc = GetCurrentEditor().document.QueryInterface(Components.interfaces.nsIDOMHTMLDocument);
//    return aDOMHTMLDoc.URL;
//  }
//  catch (e) {}
//  return "";
//}

//// Extract the scheme (e.g., 'file', 'http') from a URL string
function GetScheme(urlspec)
{
  var resultUrl = TrimString(urlspec);
  // Unsaved document URL has no acceptable scheme yet
  if (!resultUrl || IsUrlAboutBlank(resultUrl))
    return "";

  var IOService = msiGetIOService();
  if (!IOService)
    return "";

  var scheme = "";
  try {
    // This fails if there's no scheme
    scheme = IOService.extractScheme(resultUrl);
  } catch (e) {}

  return scheme ? scheme.toLowerCase() : "";
}

function GetHost(urlspec)
{
  if (!urlspec)
    return "";

  var IOService = msiGetIOService();
  if (!IOService)
    return "";

  var host = "";
  try {
    host = IOService.newURI(urlspec, null, null).host;
   } catch (e) {}

  return host;
}

function GetUsername(urlspec)
{
  if (!urlspec)
    return "";

  var IOService = msiGetIOService();
  if (!IOService)
    return "";

  var username = "";
  try {
    username = IOService.newURI(urlspec, null, null).username;
  } catch (e) {}

  return username;
}

function GetFilename(urlspec)
{
  if (!urlspec || IsUrlAboutBlank(urlspec))
    return "";

  var IOService = msiGetIOService();
  if (!IOService)
    return "";

  var filename;

  try {
    var uri = IOService.newURI(urlspec, null, null);
    if (uri)
    {
      var url = uri.QueryInterface(Components.interfaces.nsIURL);
      if (url)
        filename = url.fileName;
    }
  } catch (e) {}

  return filename ? filename : "";
}

function GetFilepath(urlspec)
{
  if (!urlspec || IsUrlAboutBlank(urlspec))
    return "";

  var IOService = msiGetIOService();
  if (!IOService)
    return "";

  var filepath;

  try {
    var uri = IOService.newURI(urlspec, null, null);
    if (uri)
    {
      var url = uri.QueryInterface(Components.interfaces.nsIURL);
      if (url)
        filepath = decodeURIComponent(url.path.substr(1));
    }
  } catch (e) {}

  return filepath ? filepath : "";
}


// apparently no longer needed 
//function GetLastDirectory(urlspec)
//{
//  if (!urlspec || IsUrlAboutBlank(urlspec))
//    return "";
//
//  var IOService = msiGetIOService();
//  if (!IOService)
//    return "";
//
//  var filename;
//
//  try {
//    var uri = IOService.newURI(urlspec, null, null);
//    if (uri)
//    {
//      var url = uri.QueryInterface(Components.interfaces.nsIURL);
//      if (url)
//        filename = url.directory;
//    }
//  } catch (e) {}
//
//  var re = /[a-zA-Z0-9\%\.]+\/$/;
//  try {
//    filename = re.exec(filename)[0];
//    filename = filename.substr(0,filename.length - 1);
//  }
//  catch(e) { filename = ""; }
//  return filename ? filename : "";
//}

// Return the url without username and password
// Optional output objects return extracted username and password strings
// This uses just string routines via nsIIOServices
function StripUsernamePassword(urlspec, usernameObj, passwordObj)
{
  urlspec = TrimString(urlspec);
  if (!urlspec || IsUrlAboutBlank(urlspec))
    return urlspec;

  if (usernameObj)
    usernameObj.value = "";
  if (passwordObj)
    passwordObj.value = "";

  // "@" must exist else we will never detect username or password
  var atIndex = urlspec.indexOf("@");
  if (atIndex > 0)
  {
    try {
      var IOService = msiGetIOService();
      if (!IOService)
        return urlspec;

      var uri = IOService.newURI(urlspec, null, null);
      var username = uri.username;
      var password = uri.password;

      if (usernameObj && username)
        usernameObj.value = username;
      if (passwordObj && password)
        passwordObj.value = password;
      if (username)
      {
        var usernameStart = urlspec.indexOf(username);
        if (usernameStart != -1)
          return urlspec.slice(0, usernameStart) + urlspec.slice(atIndex+1);
      }
    } catch (e) {}
  }
  return urlspec;
}

function StripPassword(urlspec, passwordObj)
{
  urlspec = TrimString(urlspec);
  if (!urlspec || IsUrlAboutBlank(urlspec))
    return urlspec;

  if (passwordObj)
    passwordObj.value = "";

  // "@" must exist else we will never detect password
  var atIndex = urlspec.indexOf("@");
  if (atIndex > 0)
  {
    try {
      var IOService = msiGetIOService();
      if (!IOService)
        return urlspec;

      var password = IOService.newURI(urlspec, null, null).password;

      if (passwordObj && password)
        passwordObj.value = password;
      if (password)
      {
        // Find last ":" before "@"
        var colon = urlspec.lastIndexOf(":", atIndex);
        if (colon != -1)
        {
          // Include the "@"
          return urlspec.slice(0, colon) + urlspec.slice(atIndex);
        }
      }
    } catch (e) {}
  }
  return urlspec;
}

// Version to use when you have an nsIURI object
function StripUsernamePasswordFromURI(uri)
{
  var urlspec = "";
  if (uri)
  {
    try {
      urlspec = uri.spec;
      var userPass = uri.userPass;
      if (userPass)
      {
        start = urlspec.indexOf(userPass);
        urlspec = urlspec.slice(0, start) + urlspec.slice(start+userPass.length+1);
      }
    } catch (e) {}    
  }
  return urlspec;
}

function InsertUsernameIntoUrl(urlspec, username)
{
  if (!urlspec || !username)
    return urlspec;

  try {
    var ioService = msiGetIOService();
    var URI = ioService.newURI(urlspec, msiGetCurrentEditor().documentCharacterSet, null);
    URI.username = username;
    return URI.spec;
  } catch (e) {}

  return urlspec;
}

function GetOS()
{
  if (gOS)
    return gOS;

  var platform = navigator.platform.toLowerCase();

  if (platform.indexOf("win") != -1)
    gOS = msigWin;
  else if (platform.indexOf("mac") != -1)
    gOS = msigMac;
  else if (platform.indexOf("unix") != -1 || platform.indexOf("linux") != -1 || platform.indexOf("sun") != -1)
    gOS = msigUNIX;
  else
    gOS = "";
  // Add other tests?

  return gOS;
}

function ConvertRGBColorIntoHEXColor(color)
{
  if ( /rgb\(\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*\)/.test(color) ) {
    var r = Number(RegExp.$1).toString(16);
    if (r.length == 1) r = "0"+r;
    var g = Number(RegExp.$2).toString(16);
    if (g.length == 1) g = "0"+g;
    var b = Number(RegExp.$3).toString(16);
    if (b.length == 1) b = "0"+b;
    return "#"+r+g+b;
  }
  else
  {
    return color;
  }
}

/************* CSS ***************/

function msiGetHTMLOrCSSStyleValue(editorElement, element, attrName, cssPropertyName)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var prefs = GetPrefs();
  var IsCSSPrefChecked = prefs.getBoolPref("editor.use_css");
  var value;
  if (IsCSSPrefChecked && msiIsHTMLEditor(editorElement))
    value = element.style.getPropertyValue(cssPropertyName);

  if (!value)
    value = element.getAttribute(attrName);

  if (!value)
    return "";

  return value;
}

/************* Miscellaneous ***************/
//Fix This!
//Following needs to be filled in meaningfully - but for now, just return a default.
//Note that in the document info for old SWP .tex files, the flags for ViewSettings are:
//     don't ShowInvisibles =  1
//     don't ShowHelperLines = 2
//     don't ShowInputBoxes =  4
//     don't ShowMarkers =     8
//     don't ShowIndexEntries = 16
// Thus showing everything results in ViewSettings = "0".
function msiViewSettings(viewFlags)
{
  this.showInvisibles = ((viewFlags & this.hideInvisiblesFlag) == 0);
  this.showHelperLines = ((viewFlags & this.hideHelperLinesFlag) == 0);
  this.showInputBoxes = ((viewFlags & this.hideInputBoxesFlag) == 0);
  this.showMarkers = ((viewFlags & this.hideMarkersFlag) == 0);
  this.showIndexEntries = ((viewFlags & this.hideIndexEntriesFlag) == 0);

  this.match = function(otherSettings)
  {
    if (this.showInvisibles != otherSettings.showInvisibles)
      return false;
    if (this.showHelperLines != otherSettings.showHelperLines)
      return false;
    if (this.showInputBoxes != otherSettings.showInputBoxes)
      return false;
    if (this.showMarkers != otherSettings.showMarkers)
      return false;
    if (this.showIndexEntries != otherSettings.showIndexEntries)
      return false;
    return true;
  };

  this.getFlags = function()
  {
    var theFlags = 0;
    if (!this.showInvisibles)
      theFlags |= this.hideInvisiblesFlag;
    if (!this.showHelperLines)
      theFlags |= this.hideHelperLinesFlag;
    if (!this.showInputBoxes)
      theFlags |= this.hideInputBoxesFlag;
    if (!this.showMarkers)
      theFlags |= this.hideMarkersFlag;
    if (!this.showIndexEntries)
      theFlags |= this.hideIndexEntriesFlag;
    return theFlags;
  };

}

var msiViewSettingsBase =
{ 
  hideInvisiblesFlag   :  1,
  hideHelperLinesFlag  :  2,
  hideInputBoxesFlag   :  4,
  hideMarkersFlag      :  8,
  hideIndexEntriesFlag : 16
};

msiViewSettings.prototype = msiViewSettingsBase;

function msiGetCurrViewSettings(editorElement)
{
  var viewSettings = null;
  if (("viewSettings" in editorElement) && (editorElement.viewSettings != null))
    viewSettings = editorElement.viewSettings;
  else
    viewSettings = getViewSettingsFromViewMenu();

  return viewSettings;
}

function msiGetCurrNoteViewSettings(editorElement)
{
  var viewSettings = null;
  if (("noteViewSettings" in editorElement) && (editorElement.noteViewSettings != null))
    viewSettings = editorElement.noteViewSettings;
  else if (PrefHasValue("noteViewSettings"))
    viewSettings = new msiViewSettings( GetIntPref("noteViewSettings") );
  else
    viewSettings = msiGetCurrViewSettings(editorElement);

  return viewSettings;
}

//Fix This!
function msiGetCurrViewPerCent(editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  return 100;
}

//The default print options should be stored as a numerical bit-encoded value, as of old? (Since otherwise they'd
//  be a bit lengthy.) But we should return an object with the bits interpreted(?).
//NOTE that this doesn't do any querying of current view settings (if that flag is set). It simply translates
//  the bits.
function msiPrintOptions(printFlags)
{
  this.useCurrViewSettings = (printFlags & msiDocumentInfoBase.printUseViewSettingsFlag) != 0;
  this.printInvisibles = (printFlags & msiDocumentInfoBase.printShowInvisiblesFlag) != 0;    
  this.printHelperLines = (printFlags & msiDocumentInfoBase.printShowMatrixLinesFlag) != 0;
  this.printInputBoxes = (printFlags & msiDocumentInfoBase.printShowInputBoxesFlag) != 0;
  this.printMarkers = (printFlags & msiDocumentInfoBase.printShowIndexFieldsFlag) != 0;
  this.printIndexEntries = (printFlags & msiDocumentInfoBase.printShowMarkerFieldsFlag) != 0;
  this.allTextInBlack = (printFlags & msiDocumentInfoBase.printBlackTextFlag) != 0;
  this.allLinesInBlack = (printFlags & msiDocumentInfoBase.printBlackLinesFlag) != 0;
  this.backgroundsTransparent = (printFlags & msiDocumentInfoBase.printTransparentBackgroundFlag) != 0;
  this.grayButtonsTransparent = (printFlags & msiDocumentInfoBase.printTransparentGrayButtonsFlag) != 0;
  this.suppressGrayBoxes = (printFlags & msiDocumentInfoBase.printSuppressGrayButtonsFlag) != 0;
  this.useCurrViewZoom = (printFlags & msiDocumentInfoBase.printUseViewSettingZoomFlag) != 0;

  this.reflectViewSettings = function(editorElement)
  {
    if (this.useCurrViewSettings)
    {
      var viewSettings = msiGetCurrViewSettings(editorElement);
      this.printInvisibles = viewSettings.showInvisibles;
      this.printHelperLines = viewSettings.showHelperLines;
      this.printInputBoxes = viewSettings.showInputBoxes;
      this.printMarkers = viewSettings.showMarkers;
      this.printIndexEntries = viewSettings.showIndexEntries;
    }
  };

  this.getFlags = function()
  {
    var printFlags = 0;
    if (this.useCurrViewSettings)
      printFlags |= msiDocumentInfoBase.printUseViewSettingsFlag;
    if (this.printInvisibles)
      printFlags |= msiDocumentInfoBase.printShowInvisiblesFlag;
    if (this.printHelperLines)
      printFlags |= msiDocumentInfoBase.printShowMatrixLinesFlag;
    if (this.printInputBoxes)
      printFlags |= msiDocumentInfoBase.printShowInputBoxesFlag;
    if (this.printMarkers)
      printFlags |= msiDocumentInfoBase.printShowIndexFieldsFlag;
    if (this.printIndexEntries)
      printFlags |= msiDocumentInfoBase.printShowMarkerFieldsFlag;
    if (this.allTextInBlack)
      printFlags |= msiDocumentInfoBase.printBlackTextFlag;
    if (this.allLinesInBlack)
      printFlags |= msiDocumentInfoBase.printBlackLinesFlag;
    if (this.backgroundsTransparent)
      printFlags |= msiDocumentInfoBase.printTransparentBackgroundFlag;
    if (this.grayButtonsTransparent)
      printFlags |= msiDocumentInfoBase.printTransparentGrayButtonsFlag;
    if (this.suppressGrayBoxes)
      printFlags |= msiDocumentInfoBase.printSuppressGrayButtonsFlag;
    if (this.useCurrViewZoom)
      printFlags |= msiDocumentInfoBase.printUseViewSettingZoomFlag;
    return printFlags;
  };
  this.match = function(otherOptions)
  {
    if (this.useCurrViewSettings != otherOptions.useCurrViewSettings)
      return false;
    if (this.printInvisibles != otherOptions.printInvisibles)
      return false;
    if (this.printHelperLines != otherOptions.printHelperLines)
      return false;
    if (this.printInputBoxes != otherOptions.printInputBoxes)
      return false;
    if (this.printMarkers != otherOptions.printMarkers)
      return false;
    if (this.printIndexEntries != otherOptions.printIndexEntries)
      return false;
    if (this.allTextInBlack != otherOptions.allTextInBlack)
      return false;
    if (this.allLinesInBlack != otherOptions.allLinesInBlack)
      return false;
    if (this.backgroundsTransparent != otherOptions.backgroundsTransparent)
      return false;
    if (this.grayButtonsTransparent != otherOptions.grayButtonsTransparent)
      return false;
    if (this.suppressGrayBoxes != otherOptions.suppressGrayBoxes)
      return false;
    if (this.useCurrViewZoom != otherOptions.useCurrViewZoom)
      return false;

    return true;
  };
}

//Returns an msiPrintOptions object reflecting the bits in default "printOptions". If view settings need to be taken
//  into account, the caller will have to do that.
function msiGetDefaultPrintOptions()
{
  var printFlags = (msiDocumentInfoBase.printUseViewSettingsFlag | msiDocumentInfoBase.printBlackTextFlag | msiDocumentInfoBase.printBlackLinesFlag | msiDocumentInfoBase.printTransparentBackgroundFlag | msiDocumentInfoBase.printUseViewSettingZoomFlag);
  if (PrefHasValue("printOptions"))
    printFlags = GetIntPref("printOptions");
  var theOptions = new msiPrintOptions(printFlags);
  return theOptions;
}

//"theOptions" is understood to be a msiPrintOptions object.
function msiSetDefaultPrintOptions(theOptions)
{
  var printFlags = theOptions.getFlags();
  SetIntPref("printOptions", printFlags);
}

/**********************Units management functions************************/
//The following are derived from Barry's typesetDocFormat.js. They are put here for general use involving units.

function msiUnitsList(unitConversions) 
{
  this.mUnitFactors = unitConversions;

  this.defaultUnit = function()
  {
    if ((this.mUnitfactors == null) || ("mm" in this.mUnitFactors))
      return "mm";
    return this.mUnitFactors[0];
  };

  this.convertUnits = function(invalue, inunit, outunit)
  {
    if (inunit == outunit) return invalue;
    if (!(inunit in this.mUnitFactors) || !(outunit in this.mUnitFactors))
    {
      var dumpStr = "Bad units in msiUnitsList.convertUnits;";
      if (!(inunit in this))
        dumpStr += " in unit is [" + inunit + "];";
      if (!(outunit in this))
        dumpStr += " out unit is [" + outunit + "];";
      dump( dumpStr + " returning.\n");
      return invalue;
    }
    var outvalue = invalue*this.mUnitFactors[inunit];
    outvalue /= this.mUnitFactors[outunit];
    dump(invalue+inunit+" = "+outvalue+outunit+"\n");
    return outvalue;
  };

  this.getDisplayString = function(theUnit)
  {
    if (!this.mStringBundle)
    {
      try {
        var strBundleService = Components.classes["@mozilla.org/intl/stringbundle;1"].getService(); 
        strBundleService = strBundleService.QueryInterface(Components.interfaces.nsIStringBundleService);
        this.mStringBundle = strBundleService.createBundle("chrome://prince/locale/msiDialogs.properties"); 
      } catch (ex) {dump("Problem in initializing string bundle in msiUnitsList.getDisplayString: exception is [" + ex + "].\n");}
    }
    if (this.mStringBundle)
    {
      var unitsPrefix = "units.";
      try
      {
        return this.mStringBundle.GetStringFromName(unitsPrefix + theUnit);
      } catch (e) {dump("Problem in msiUnitsList.getDisplayString for unit [" + theUnit + "]: exception is [" + e + "].\n");}
    }
    return null;
  };

  this.getNumberAndUnitFromString = function(valueStr)
  {
    var unitsStr = "";
    for (var aUnit in this.mUnitFactors)
    {
      if (unitsStr.length > 0)
        unitsStr += "|";
      unitsStr += aUnit;
    }
    var ourRegExp = new RegExp("(\\-?\\d*\\.?\\d*).*(" + unitsStr + ")");
    var matchArray = ourRegExp.exec(valueStr);
    if (matchArray != null)
    {
      var retVal = new Object();
      retVal.number = Number(matchArray[1]);
      retVal.unit = matchArray[2];
      return retVal;
    }
    return null;
  };

}

//Following need to be added to. They're called "CSSUnitConversions", but are intended to handle any units showing up in
//markup - particularly in XBL (see latex.xml!).
var msiCSSUnitConversions =
{
  pt: .3514598,  //mm per pt
  in: 25.4,  //mm per in
  mm: 1, // mm per mm
  cm: 10 // mm per cm
};

var msiCSSUnitsList = new msiUnitsList(msiCSSUnitConversions);
msiCSSUnitsList.defaultUnit = function() {return "pt";}

var msiBaseMathNameList = 
{
  bInitialized: false,
  namesDoc : null,
  sourceFile : "mathnames.xml",
  bModified: false,
  bModifiedAutoSubs: false,

  initialize : function()
  {
    var dsprops = Components.classes["@mozilla.org/file/directory_service;1"].getService(Components.interfaces.nsIProperties);
    var basedir =dsprops.get("resource:app", Components.interfaces.nsIFile);
    basedir.append("res");

//    var theFile = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
//    theFile.initWithPath(this.sourceFile);
//    var inputStream = Components.classes["@mozilla.org/network/file-input-stream;1"].createInstance(Components.interfaces.nsIFileInputStream);
//    inputStream.init(theFile, -1, -1, false);  //these values for parameters taken from various examples...?
//    this.namesDoc = domParser.parseFromStream(inputStream, "utf-8", inputStream.available(), "text/xml");
//    inputstream.close();

    var mathNameFile = basedir;
    mathNameFile.append("tagdefs");
    mathNameFile.append(this.sourceFile);
    this.sourceFile = mathNameFile.path;
    this.namesDoc = document.implementation.createDocument("", "mathnames", null);
    var nodeList;
    var node;
    var s;
    var arrayElement;
    this.namesDoc.async = false;
    if (this.namesDoc.load("file:///" + mathNameFile.path))
    {
      this.bInitialized = true;
      this.initAutoCompleteList();
    }
  },

  initAutoCompleteList : function()
  {
    if (!this.bInitialized)
    {
      dump("In msiBaseMathNameList, initAutoCompleteList being called without bInitialized being set!?\n");
      this.initialize();
    }
    var ACSA = Components.classes["@mozilla.org/autocomplete/search;1?name=stringarray"].getService();
    ACSA.QueryInterface(Components.interfaces.nsIAutoCompleteSearchStringArray);
    var nameNodesList = this.namesDoc.getElementsByTagName("mathname");
    for (var ix = 0; ix < nameNodesList.length; ++ix)
    {
      var theId = nameNodesList[ix].getAttribute("id");
      ACSA.addString("mathnames", theId);
    }
    ACSA.sortArrays();
  },

  createDialogNameList : function()
  {
    if (!this.bInitialized)
      this.initialize();
    var newNameList = new Object();
    var nameNodesList = this.namesDoc.getElementsByTagName("mathname");
    for (var ix = 0; ix < nameNodesList.length; ++ix)
    {
      var theId = nameNodesList[ix].getAttribute("id");
      newNameList[theId] = this.copyDataToObject(nameNodesList[ix]);
    }
    return newNameList;
  },

  getMathNameData : function(aName)
  {
    if (!this.bInitialized)
      this.initialize();
    var theNode = this.namesDoc.getElementById(aName);
    if (theNode != null)
      return this.copyDataToObject(theNode)
    return null;
  },

  copyDataToObject : function(nameNode)
  {
    if (!nameNode)
    {
      dump("Error in msiEditorUtilities.js, in msiBaseMathNameList.copyDataToObject; null nameNode passed in!\n");
      return null;
    }
    var nameData = new Object();
    nameData.val = nameNode.getAttribute("id");
    nameData.type = nameNode.getAttribute("type");
    var theAppearanceList = nameNode.getElementsByTagName("appearance");
    if (theAppearanceList != null && theAppearanceList.length > 0)
      nameData.appearance = theAppearanceList[0].cloneNode(true);
    nameData.builtIn = (nameNode.hasAttribute("builtIn") && nameNode.getAttribute("builtIn") == "true");
    if (nameNode.hasAttribute("limitPlacement"))
      nameData.limitPlacement = nameNode.getAttribute("limitPlacement");
    if ( (nameNode.hasAttribute("engineFunction")) && (nameNode.getAttribute("engineFunction") == "true") )
      nameData.enginefunction = true;
    if (this.nameHasAutoSubstitution(nameData.val))
      nameData.autoSubstitute = true;
    return nameData;
  },

  deleteName : function(aName)
  {
    var nameNode = this.namesDoc.getElementById(aName);
    if (nameNode != null)
    {
      var isBuiltIn = nameNode.getAttribute("builtIn");
      if ((!isBuiltIn) || (isBuiltIn != "true"))
      {
        if (this.nameHasAutoSubstitution(aName))
          this.removeAutoSubstitution(aName);
        nameNode.parentNode.removeChild(nameNode);
        var ACSA = Components.classes["@mozilla.org/autocomplete/search;1?name=stringarray"].getService();
        ACSA.QueryInterface(Components.interfaces.nsIAutoCompleteSearchStringArray);
        ACSA.deleteString("mathnames", aName);
        this.bModified = true;
        return true;
      }
    }
    return false;
  },

  addName : function(aName, aNameData)
  {
    var nameNode = this.namesDoc.getElementById(aName);
    if (nameNode != null)
    {
      var isBuiltIn = nameNode.getAttribute("builtIn");
      if (isBuiltIn && (isBuiltIn == "true"))
        return false;
      nameNode.parentNode.removeChild(nameNode);
    }
    var parentNode = this.namesDoc.getElementById("nameBase");
    var newNode = this.namesDoc.createElement("mathname");
    newNode.setAttribute("id", aName);
    newNode.setAttribute("type", aNameData.type);
    if ("appearance" in aNameData)
    {
      var newAppearance = aNameData.appearance.cloneNode(true);
      newNode.appendChild(newAppearance);
    }
    var newData = null;
    if (("nameData" in aNameData) && (aNameData.nameData != null) && ("nodeType" in aNameData.nameData))
    {
      newData = aNameData.nameData.cloneNode(true);
      newNode.appendChild(newData);
    }
    if ("limitPlacement" in aNameData)
      newNode.setAttribute("limitPlacement", aNameData.limitPlacement);
    parentNode.appendChild(newNode);
    var ACSA = Components.classes["@mozilla.org/autocomplete/search;1?name=stringarray"].getService();
    ACSA.QueryInterface(Components.interfaces.nsIAutoCompleteSearchStringArray);
    ACSA.addString("mathnames", aName);
    ACSA.sortArrays();
    if (aNameData.autoSubstitute == true)
      this.addAutoSubstitution(aName);
    this.bModified = true;
    return true;
  },

  updateName : function(aName, aNameData)
  {
    return this.addName(aName, aNameData);  //is this what we should do?
  },

  nameHasAutoSubstitution : function(aName)
  {
    var autosub = Components.classes["@mozilla.org/autosubstitute;1"].getService(Components.interfaces.msiIAutosub);
    autosub.Reset();
    var result = Components.interfaces.msiIAutosub.STATE_INIT;
    for (var ix = aName.length - 1; ix >= 0; --ix)
    {
      result = autosub.nextChar(aName.charAt(ix));
      if (result == Components.interfaces.msiIAutosub.STATE_FAIL)
        return false;
    }
    return (result == Components.interfaces.msiIAutosub.STATE_SUCCESS);
  },

  addAutoSubstitution : function(aName)
  {
    dump("In msiBaseMathNameList.addAutoSubstitution for name " + aName + "\n.");
    if (this.nameHasAutoSubstitution(aName))
      return;
      //The following should be reinstated as soon as it's implemented in the object.
    var autosub = Components.classes["@mozilla.org/autosubstitute;1"].getService(Components.interfaces.msiIAutosub);
    var bAdded = autosub.addEntry( aName, Components.interfaces.msiIAutosub.CONTEXT_MATHONLY, Components.interfaces.msiIAutosub.ACTION_EXECUTE,
                      "insertMathname('" + aName + "')", "", "" );
    dump("autosub.addEntry returned " + (bAdded ? "true" : "false") + "\n.");
    this.bModifiedAutoSubs = true;
  },

  removeAutoSubstitution : function(aName)
  {
    var autosub = Components.classes["@mozilla.org/autosubstitute;1"].getService(Components.interfaces.msiIAutosub);
    autosub.removeEntry( aName);
    this.bModifiedAutoSubs = true;
  },

  updateFile : function()
  {
    dump("msiBaseMathNameList.updateFile being called; this.bModified is " + (this.bModified ? "true" : "false") + ".\n");
    if (this.bModified && (this.namesDoc != null))
    {
      var theFile = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
      theFile.initWithPath(this.sourceFile);
      var outputStream = Components.classes["@mozilla.org/network/file-output-stream;1"].createInstance(Components.interfaces.nsIFileOutputStream);
      var fileMode = 0x22;  //MODE_WRONLY|MODE_TRUNCATE - rewrite the file
      var permissions = 0x777; //all permissions for everybody?
      outputStream.init(theFile, fileMode, permissions, 0);
      var serializer = new XMLSerializer();
      serializer.serializeToStream(this.namesDoc, outputStream, "utf-8");
      outputStream.close();
      this.bModified = false;
      //The following should be reinstated as soon as it's implemented in the object.
      if (this.bModifiedAutoSubs)
      {
        var autosub = Components.classes["@mozilla.org/autosubstitute;1"].getService(Components.interfaces.msiIAutosub);
        var bSaved = autosub.Save();  //not currently implemented
        dump("autosub.Save() returned " + (bSaved ? "true" : "false") + "\n");
        this.bModifiedAutoSubs = false;
      }
    }
  }
};

function msiMathNameList()
{
  this.names = msiBaseMathNameList.createDialogNameList();
  this.updateBaseList = function()
  {
//    for (var aName in this.names)
//    {
//      try
//      {
//        if (("bDeleted" in this.names[aName]) && (this.names[aName].bDeleted == true))
//          msiBaseMathNameList.deleteName(aName);
//        else if (("added" in this.names[aName]) && (this.names[aName].added == true))
//          msiBaseMathNameList.addName(aName, this.names[aName]);
//        else if (("changed" in this.names[aName]) && (this.names[aName].changed == true))
//          msiBaseMathNameList.updateName(aName, this.names[aName]);
//      } catch(exc) {dump("Problem checking properties for member [" + aName + "] of msiMathNameList.names: [" + exc + "].\n");}
//    }
    msiBaseMathNameList.updateFile();
  };
  this.canDelete = function(aName)
  {
    if ((aName in this.names) && (this.names[aName] != null) && (!this.names[aName].builtIn))
    {
//      return !("bDeleted" in this.names[aName]) || (this.names[aName].bDeleted != true);
      return true;
    }
    return false;
  };
  this.canAdd = function(aName)
  {
    if ((aName in this.names) && (this.names[aName] != null))
    {
      return (!this.names[aName].builtIn) && ("bDeleted" in this.names[aName]) && (this.names[aName].bDeleted == true);
//      return (!this.names[aName].builtIn);
    }
    return true;
  };
  this.hasAutoSubstitution = function(aName)
  {
    if ((aName in this.names) && ("autoSubstitute" in this.names[aName]) && (this.names[aName].autoSubstitute == true))
      return true;
    return false;
  };
  this.isBuiltIn = function(aName)
  {
    if ((aName in this.names) && (this.names[aName] != null))
    {
      if (this.names[aName].builtIn == true)
        return true;
    }
    return false;
  };
  this.addName = function(aName, aType, bEngineFunction, bAutoSubstitute, aLimitPlacement, appearanceList)
  {
    var nameData = new Object();
    nameData.val = aName;
    nameData.type = aType;
//    nameData.added = true;
    if (appearanceList != null && !this.appearanceIsUsual(appearanceList, aName, aType, aLimitPlacement))
      nameData.appearance = appearanceList;
    nameData.builtIn = false;
    if ((aType == "operator") && (aLimitPlacement != null) && (aLimitPlacement.length > 0) && (aLimitPlacement != "auto"))
      nameData.limitPlacement = aLimitPlacement;
    nameData.enginefunction = bEngineFunction;
    nameData.autoSubstitute = bAutoSubstitute;
    this.names[aName] = nameData;
    msiBaseMathNameList.addName(aName, nameData);
  };
  this.deleteName = function(aName)
  {
    if (this.canDelete(aName))
    {
      msiBaseMathNameList.deleteName(aName);
      delete this.names[aName];
    }
//      this.names[aName].bDeleted = true;
  };
  this.appearanceIsUsual = function(appearanceList, aName, aType, aLimitPlacement)
  {
    return true;  //need to settle how this should be passed in first - is it guaranteed to be a NodeList?
  };
}

/**************************More general utilities**********************/

// Clone simple JS objects
//function Clone(obj) 
//{ 
//  var clone = {};
//  for (var i in obj)
//  {
//    if( typeof obj[i] == 'object')
//      clone[i] = Clone(obj[i]);
//    else
//      clone[i] = obj[i];
//  }
//  return clone;
//}

//Following stolen from calendar/resources/content/importExport.js and modified:
function addDataToFile(aFilePath, aDataStream)
{
  const LOCALFILE_CTRID = "@mozilla.org/file/local;1";
  const FILEOUT_CTRID = "@mozilla.org/network/file-output-stream;1";
  const nsILocalFile = Components.interfaces.nsILocalFile;
  const nsIFileOutputStream = Components.interfaces.nsIFileOutputStream;

  const MODE_RDONLY   = 0x01;
  const MODE_WRONLY   = 0x02;
  const MODE_RDWR     = 0x04;
  const MODE_CREATE   = 0x08;
  const MODE_APPEND   = 0x10;
  const MODE_TRUNCATE = 0x20;
  const MODE_SYNC     = 0x40;
  const MODE_EXCL     = 0x80;

  var localFileInstance;
  var outputStream;
  
  var LocalFileInstance = Components.classes[LOCALFILE_CTRID].createInstance(nsILocalFile);
  LocalFileInstance.initWithPath(aFilePath);

  outputStream = Components.classes[FILEOUT_CTRID].createInstance(nsIFileOutputStream);
  try
  {
//    if(charset)
//       aDataStream = convertFromUnicode( charset, aDataStream );

    outputStream.init(LocalFileInstance, MODE_WRONLY | MODE_CREATE | MODE_APPEND, 0664, 0);
    outputStream.write(aDataStream, aDataStream.length);
    // outputStream.flush();
    outputStream.close();
  }
  catch(ex)
  {
    alert("Unable to write to file " + aFilePath);
  }
}

function msiKludgeLogString(logStr)
{
  return;
  dump(logStr);

  if (!("msiLogPath" in window) || !window.msiLogPath)
  {
    var logLocalFile = null;
    try
    {
      var dsprops = Components.classes["@mozilla.org/file/directory_service;1"].createInstance(Components.interfaces.nsIProperties);
      logLocalFile = dsprops.get("CurProcD", Components.interfaces.nsIFile);
    } 
    catch(exception) {alert("Couldn't set log path in msiKludgeLotString!"); window.msiLogPath = "failed"; return;}
    if (!logLocalFile)
    {
      var localFileInstance = Components.classes["@mozilla.org/file/local;1"].createInstance(nsILocalFile);
      localFileInstance.initWithPath("C:\\");
      logLocalFile = localFileInstance;
    }
//    alert("Initial log path is " + logLocalFile.path);
    if (!logLocalFile.isDirectory())
      logLocalFile = logLocalFile.parent;
    logLocalFile.append("msiInfo.log");
    window.msiLogPath = logLocalFile.path;
//    alert("Log path is " + window.msiLogPath);
  }
  addDataToFile(window.msiLogPath, logStr);
}

function msiDumpWithID(str, element)
{
  var replStr = "";
  if (element!=null && element.id)
    replStr = element.id;
  dump( str.replace("@", replStr) );
}

