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
//const gWin = "Win";
//const gUNIX = "UNIX";
//const gMac = "Mac";
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

      gStringBundle = strBundleService.createBundle("chrome://editor/locale/editor.properties"); 

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
    return parentWindow.msiParentEditor;
//  var docElement = parentWindow.document.documentElement;
//  if ("msiParentEditor" in docElement)
//    return docElement.msiParentEditor;
  AlertWithTitle("Error", "Can't find parent editor for editorElement " + editorElement.id);
  return null;
}

function msiSetParentEditor(parentEditor, theWindow)
{
  var retVal = false;
  if (!theWindow)
    theWindow = window;
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
    dump(e) + "\n"; 
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
  try
  {
    var aDOMHTMLDoc = msiGetEditor(editorElement).document.QueryInterface(Components.interfaces.nsIDOMHTMLDocument);
    return aDOMHTMLDoc.URL;
  } catch (e) {}
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
  try {
    return new XPCNativeWrapper(msiGetEditor(editorElement).document, "title").title;
  } catch (e) {}

  return "";
}

function msiSetDocumentTitle(editorElement, title)
{

  try {
    msiGetEditor(editorElement).setDocumentTitle(title);

    // Update window title (doesn't work if called from a dialog)
    if ("UpdateWindowTitle" in window)
    {
      alert("3");
      window.UpdateWindowTitle();
      alert("4");
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

function IsUrlAboutBlank(urlString)
{
  return (urlString == "about:blank");
}

function MakeRelativeUrl(url)
{
  var inputUrl = TrimString(url);
  if (!inputUrl)
    return inputUrl;

  // Get the filespec relative to current document's location
  // NOTE: Can't do this if file isn't saved yet!
  var docUrl = GetDocumentBaseUrl();
  var docScheme = GetScheme(docUrl);

  // Can't relativize if no doc scheme (page hasn't been saved)
  if (!docScheme)
    return inputUrl;

  var urlScheme = GetScheme(inputUrl);

  // Do nothing if not the same scheme or url is already relativized
  if (docScheme != urlScheme)
    return inputUrl;

  var IOService = msiGetIOService();
  if (!IOService)
    return inputUrl;

  // Host must be the same
  var docHost = GetHost(docUrl);
  var urlHost = GetHost(inputUrl);
  if (docHost != urlHost)
    return inputUrl;


  // Get just the file path part of the urls
  // XXX Should we use GetCurrentEditor().documentCharacterSet for 2nd param ?
  var docPath = IOService.newURI(docUrl, GetCurrentEditor().documentCharacterSet, null).path;
  var urlPath = IOService.newURI(inputUrl, GetCurrentEditor().documentCharacterSet, null).path;

  // We only return "urlPath", so we can convert
  //  the entire docPath for case-insensitive comparisons
  var os = GetOS();
  var doCaseInsensitive = (docScheme == "file" && os == msigWin);
  if (doCaseInsensitive)
    docPath = docPath.toLowerCase();

  // Get document filename before we start chopping up the docPath
  var docFilename = GetFilename(docPath);

  // Both url and doc paths now begin with "/"
  // Look for shared dirs starting after that
  urlPath = urlPath.slice(1);
  docPath = docPath.slice(1);

  var firstDirTest = true;
  var nextDocSlash = 0;
  var done = false;

  // Remove all matching subdirs common to both doc and input urls
  do {
    nextDocSlash = docPath.indexOf("\/");
    var nextUrlSlash = urlPath.indexOf("\/");

    if (nextUrlSlash == -1)
    {
      // We're done matching and all dirs in url
      // what's left is the filename
      done = true;

      // Remove filename for named anchors in the same file
      if (nextDocSlash == -1 && docFilename)
      { 
        var anchorIndex = urlPath.indexOf("#");
        if (anchorIndex > 0)
        {
          var urlFilename = doCaseInsensitive ? urlPath.toLowerCase() : urlPath;
        
          if (urlFilename.indexOf(docFilename) == 0)
            urlPath = urlPath.slice(anchorIndex);
        }
      }
    }
    else if (nextDocSlash >= 0)
    {
      // Test for matching subdir
      var docDir = docPath.slice(0, nextDocSlash);
      var urlDir = urlPath.slice(0, nextUrlSlash);
      if (doCaseInsensitive)
        urlDir = urlDir.toLowerCase();

      if (urlDir == docDir)
      {

        // Remove matching dir+"/" from each path
        //  and continue to next dir
        docPath = docPath.slice(nextDocSlash+1);
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
        if (firstDirTest && docScheme == "file" && os != msigUNIX)
          return inputUrl;
      }
    }
    else  // No more doc dirs left, we're done
      done = true;

    firstDirTest = false;
  }
  while (!done);

  // Add "../" for each dir left in docPath
  while (nextDocSlash > 0)
  {
    urlPath = "../" + urlPath;
    nextDocSlash = docPath.indexOf("\/", nextDocSlash+1);
  }
  return urlPath;
}

function MakeAbsoluteUrl(url)
{
  var resultUrl = TrimString(url);
  if (!resultUrl)
    return resultUrl;

  // Check if URL is already absolute, i.e., it has a scheme
  var urlScheme = GetScheme(resultUrl);

  if (urlScheme)
    return resultUrl;

  var docUrl = GetDocumentBaseUrl();
  var docScheme = GetScheme(docUrl);

  // Can't relativize if no doc scheme (page hasn't been saved)
  if (!docScheme)
    return resultUrl;

  var  IOService = msiGetIOService();
  if (!IOService)
    return resultUrl;
  
  // Make a URI object to use its "resolve" method
  var absoluteUrl = resultUrl;
  var docUri = IOService.newURI(docUrl, GetCurrentEditor().documentCharacterSet, null);

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

    if (!IsUrlAboutBlank(docUrl))
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
    var URI = ioService.newURI(urlspec, GetCurrentEditor().documentCharacterSet, null);
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

//function ConvertRGBColorIntoHEXColor(color)
//{
//  if ( /rgb\(\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*\)/.test(color) ) {
//    var r = Number(RegExp.$1).toString(16);
//    if (r.length == 1) r = "0"+r;
//    var g = Number(RegExp.$2).toString(16);
//    if (g.length == 1) g = "0"+g;
//    var b = Number(RegExp.$3).toString(16);
//    if (b.length == 1) b = "0"+b;
//    return "#"+r+g+b;
//  }
//  else
//  {
//    return color;
//  }
//}

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

