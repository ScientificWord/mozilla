// Copyright (c) 2006 MacKichan Software, Inc.  All Rights Reserved.
//Components.utils.import("resource://app/modules/pathutils.jsm");
Components.utils.import("resource://app/modules/os.jsm");

const msiEditorUtilitiesJS_duplicateTest = "Bad";

var gStringBundle;
var gIOService;
var gPrefsService;
var gPrefsBranch;
var gOS = "";

///************* Message dialogs ***************/
//

// a tracing utility
function msidump(str, indent, clear)
{
	var pref = GetStringPref("swp.messagelogger");
	var indentstring="";
	var spaces="                 ";
	if (pref === null || pref.length === 0) pref = "dump";
	if (indent!== null && indent > 0)
	  indentstring = spaces.substring(1,indent+indent);
	switch (pref)
	{
		case "dump":
			dump(indentstring+str+"\n");
			break;
		case "jsconsole":
			var cons;
		  cons = Components.classes['@mozilla.org/consoleservice;1']
	            .getService(Components.interfaces.nsIConsoleService);
			if (clear)
				cons.reset();
			cons.logStringMessage(indentstring+str);
			break;
		case "alert":
			AlertWithTitle("Dump", str, null);
			break;
	}
}

// The following was added by BBM for diagnostic purposes. It should not be in the release version
// If the selection is collapsed and in a math object, this will dump the math object and show
// the location of the selection point.

function dumpMath()
{
	msidump('',0,	true);
  var editorElement = getCurrentEditorElement();
  var editor = msiGetEditor(editorElement);
  var HTMLEditor = editor.QueryInterface(Components.interfaces.nsIHTMLEditor);
  var rootnode = HTMLEditor.getSelectionContainer();
  var i = 2;
  while (rootnode && rootnode.parentNode && i-- >0) rootnode= rootnode.parentNode;
//  while (rootnode && rootnode.localName !== "math" && editor.tagListManager.getTagInClass("paratags",rootnode.localName,null)) rootnode = rootnode.parentNode;
//  if (!rootnode)
//  { 
//    msidump("Failed to find math or paragraph node\n");
//    return;
//  }
  try
  {
		var sel = HTMLEditor.selection;
	  var selNode = sel.anchorNode;
	  var selOffset = sel.anchorOffset;
	  var focNode = sel.focusNode;
	  var focOffset = sel.focusOffset;
	  var indent = 0;
	  msidump(selNode.nodeName + " to " + focNode.nodeName+"\n",0);
	  msidump("Selection:  selNode="+(selNode.nodeType === Node.TEXT_NODE?"text":selNode.nodeName)+", offset="+selOffset+"\n",0);
	  msidump("focusNode="+(focNode.nodeType === Node.TEXT_NODE?"text":focNode.nodeName)+", offset="+focOffset+"\n",6);
	  msidump("rootnode="+rootnode.nodeName+"\n",6);
	  dumpNodeMarkingSel(rootnode, selNode, selOffset, focNode, focOffset, indent);
	}
	catch(e)
	{
		finalThrow(commandFailureStrin("dumpMath"), e.message);
	}
}


function doIndent( k )
{
  for (j = 0; j < k; j++) msidump("  ");
}


function dumpNodeMarkingSel(node, selnode, seloffset, focnode, focoffset, indent)
{
try{
	
//  msidump("dumpNodeMarkingSel, indent = "+indent+", node = "+node.nodeName+"\n");
  var len = node.childNodes.length;
  if (node.nodeType === Node.ELEMENT_NODE)
  {
    msidump("<"+node.nodeName+"> \n",indent);
    for (var i = 0; i < len; i++)
    {    
	    if (node===selnode && seloffset===i) 
	    {
	      msidump("<selection anchor>\n", indent);
	    }
	    if (node===focnode && focoffset===i) 
	    {
	      msidump("<selection focus>\n", indent);
	    }
      dumpNodeMarkingSel(node.childNodes[i],selnode,seloffset, focnode, focoffset, indent+1);
    }
    msidump("</"+node.nodeName+">\n", indent);
  }
  else if (node.nodeType === Node.TEXT_NODE)
  {
    var offset1;
		var offset2;
		if (node === selnode && selnode === focnode)
		{
			offset1 = Math.min(seloffset, focoffset);
			offset2 = Math.max(seloffset, focoffset);
			msidump(node.nodeValue.slice(0, offset1)+"<selection " + (offset1===seloffset?"anchor":"focus")+">"+
			  node.nodeValue.slice(offset1,offset2) + "<selection "+(offset1===seloffset?"focus":"anchor")+">"+node.nodeValue.slice(offset2))
		}
		else
		{
			if (node===selnode)
	    {
	      msidump(node.nodeValue.slice(0,seloffset)+"<selection anchor>"+node.nodeValue.slice(seloffset)+"\n",indent);
	    }
	    else {
	      if (node===focnode)
	      {
	        msidump(node.nodeValue.slice(0,focoffset)+"<selection focus>"+node.nodeValue.slice(focoffset)+"\n",indent);
	      }
	      else {
	        var s = node.nodeValue;
	        var t = s.replace(/^\s*/,'');
	        var r = t.replace(/\s*$/,'');
	        if (r.length>0)
	        {
		          msidump(r+'\n', indent);
	        }
	        else msidump("whitespace node\n");
	      }
			}
    }  
  }
}
catch(e)
{
	finalThrow("dumpNodeMarkingSel failed", e.message);
}
}   
  

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

function finalThrow(message, moremessage)
{
	var prompts = Components.classes["@mozilla.org/embedcomp/prompt-service;1"]  
	                        .getService(Components.interfaces.nsIPromptService);  

	var flags = prompts.BUTTON_POS_1 * prompts.BUTTON_TITLE_IS_STRING  +  
	            prompts.BUTTON_POS_2 * prompts.BUTTON_TITLE_CANCEL +
							33554432;
	var checkBool = {};
  var button = prompts.confirmEx(null, GetString("commandfailed"), message,  
	                               flags, "", GetString("moreinfo"), "", null, checkBool );  
	if (button===1)
	  prompts.alert(null, GetString("moredetails"), moremessage);
}

function cmdFailString(command) {
  var message = GetString("CmdFailed");
  return message.replace("##",command);
}

function MsiException( message, previous )
{
   this.message = message + "\n";
   if (previous) {
     this.message += previous.message;
   }
   this.toString = function() {
      return this.message;
   };
}

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

    } catch (e) {
      throw new MsiException("Unable to load editor.properties string bundle", e);
    }
  }
  if (gStringBundle)
  {
    try {
      return gStringBundle.GetStringFromName(name);
    } catch (e) {
      throw  new MsiException("Can\'t find name \'" + name + " in string bundle", e);
    }
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
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  try {
    return msiGetEditor(editorElement).outputToString("text/plain", 1); // OutputSelectionOnly
  } catch (e) {
    throw new MsiException("Failure converting selection to text",e);
  }

  return "";
}

function hexcolor(rgbcolor)
{
	var regexp = /\s*rgb\s*\(\s*(\d+)[^\d]*(\d+)[^\d]*(\d+)/ ;
	var arr = regexp.exec(rgbcolor);
  if (!arr || (arr.length < 4)) // color was given a 'black' or 'blue'
	  return textColorToHex(rgbcolor);

	var r = parseFloat(arr[1]).toString(16);
	if (r.length<2) r = "0"+r;
	var g = parseFloat(arr[2]).toString(16);
	if (g.length<2) g = "0"+g;
	var b = parseFloat(arr[3]).toString(16);
	if (b.length<2) b = "0"+b;
	return "#"+ r + g + b;
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
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
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
//  return document.documentElement.id === "editorWindow";
  if (!theWindow)
    theWindow = window;
  if (theWindow.document && theWindow.document.documentElement)
    return theWindow.document.documentElement.id === "prince";
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
    } catch(e) {
      throw new MsiException("In clearPrevActiveEditor, unable to retrieve curr focused element",e);
    }
  }
//End logging stuff
  if (theWindow.msiClearEditorTimerList !== null)
  {
    logStr += ";\n  clearPrevActiveEditor called, ";
    if (!theWindow.msiActiveEditorElement)
    {
      logStr += "with curr focused element [";
      if (currFocusedElement !== null)
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
    } catch(e) {
      throw new MsiException("Exception in clearPrevActiveEditor",e);
    }
    var nFound = msiFindTimerInArray(theWindow.msiClearEditorTimerList, timerData);
    if (nFound >= 0)
    {
      theWindow.msiClearEditorTimerList.splice(nFound, 1);
      if (theWindow.msiClearEditorTimerList.length === 0)
      {
        logStr += "deleting reference to prev editor.\n";
        theWindow.msiPrevEditorElement = null;
        if (theWindow.msiSingleDialogList)
          theWindow.msiSingleDialogList.reparentAppropriateDialogs(theWindow.msiActiveEditorElement);
        if (theWindow.msiActiveEditorElement !== null)
        {
          msiDoUpdateCommands("style", theWindow.msiActiveEditorElement);
          var editor = msiGetEditor(theWindow.msiActiveEditorElement);
          if (editor && editor.tagListManager)
            editor.tagListManager.enable();  //This will set the autocomplete string imp in use to the editor's.
        }
      }
      else
        logStr += "but timer list still contains [" + theWindow.msiClearEditorTimerList.toString() + "], so not deleting prev editor.\n";
    }
    else
    {
      logStr += "but timer [";
      if (timerData !== null)
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

//  if (topWindow.msiPrevEditorElement && topWindow.msiActiveEditorElement && topWindow.msiPrevEditorElement !== topWindow.msiActiveEditorElement)
  if (topWindow.msiPrevEditorElement && topWindow.msiPrevEditorElement !== topWindow.msiActiveEditorElement)
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
  if ("msiClearEditorTimerList" in topWindow && topWindow.msiClearEditorTimerList !== null)
    msiClearAllFocusTimers(topWindow, topWindow.msiClearEditorTimerList);
//  msiKludgeLogString(logStr);
}

function msiEditorStateLogString(theWindow)
{
  return "";

  if (!theWindow)
    theWindow = msiGetTopLevelWindow();
  var currEdId = "";
  if ( ("msiActiveEditorElement" in theWindow) && (theWindow.msiActiveEditorElement !== null) )
    currEdId = theWindow.msiActiveEditorElement.id;
  var prevEdId = "";
  if ( ("msiPrevEditorElement" in theWindow) && (theWindow.msiPrevEditorElement !== null) )
    prevEdId = theWindow.msiPrevEditorElement.id;
  var logStr = "Current msiActiveEditor is [" + currEdId + "], prevEdId is [" + prevEdId + "]";
  logStr += ", clearEditorTimer IDs are [";
  if ("msiClearEditorTimerList" in theWindow && theWindow.msiClearEditorTimerList !== null)
    logStr += theWindow.msiClearEditorTimerList.toString();
  logStr += "], for window titled [";
  logStr += theWindow.document.title;
  logStr += "]";
  return logStr;
}

function msiFindTimerInArray(timerList, timerData)
{
  if (timerList && timerData)
  {
    var nIndex = -1;
    for (var ix = 0; nIndex < 0 && ix < timerList.length; ++ix)
    {
      if (timerList[ix] == timerData.nTimerID)
        nIndex = ix;
    }
  }
  return nIndex;
}

function msiClearAllFocusTimers(theWindow, timerList)
{
  if (theWindow && timerList)
  {
    for (var ix = timerList.length - 1; ix >= 0; --ix)
    {
      try
      {
        theWindow.clearTimeout(timerList[ix]);
        timerList.splice(ix, 1);
      } catch(e) {
        throw new MsiException("In msiClearAllFocusTimers, theWindow couldn't clear timer",e);
      }
    }
  }
//  timerList.splice(0, timerList.length);
}

function msiSetActiveEditor(editorElement, bIsFocusEvent)
{
  var theWindow = msiGetTopLevelWindow();

//Logging stuff only:
//  var currEdId = "";
//  if ( ("msiActiveEditorElement" in theWindow) && (theWindow.msiActiveEditorElement !== null) )
//    currEdId = theWindow.msiActiveEditorElement.id;
  var newEdId = "";
  if (editorElement)
    newEdId = editorElement.id;
//  var prevEdId = "";
//  if ( ("msiPrevEditorElement" in theWindow) && (theWindow.msiPrevEditorElement !== null) )
//    prevEdId = theWindow.msiPrevEditorElement.id;
//End logging stuff

  var editor = msiGetEditor(theWindow.msiActiveEditorElement);
  if (editor && bIsFocusEvent && editor.tagListManager)
    editor.tagListManager.enable();  //This will set the autocomplete string imp in use to the editor's.
  var bIsDifferent = (!theWindow.msiActiveEditorElement || (theWindow.msiActiveEditorElement !== editorElement));
  if (bIsDifferent)
  {
//Logging stuff only:
//    var logStr = "In msiSetActiveEditor, current msiActiveEditor is [" + currEdId + "], prevEdId is [" + prevEdId + "]";
    var logStr = "In msiSetActiveEditor; ";
    logStr += msiEditorStateLogString(theWindow);
    logStr += ",\n  trying to change to [" + newEdId + "], setting; bIsFocusEvent is  "
    if (bIsFocusEvent)
      logStr += "true";
    else
      logStr += "false";
//End logging stuff
    if (bIsFocusEvent)
    {
      if (!theWindow.msiPrevEditorElement)
        theWindow.msiPrevEditorElement = theWindow.msiActiveEditorElement;
//    {
      var newTimerData = new Object();
      var nNewTimer = theWindow.setTimeout(clearPrevActiveEditor, 0, newTimerData);
      if (!("msiClearEditorTimerList" in theWindow) || (theWindow.msiClearEditorTimerList === null) )
        theWindow.msiClearEditorTimerList = new Array();
      theWindow.msiClearEditorTimerList.push(nNewTimer);
      newTimerData.nTimerID = nNewTimer;
//Logging stuff only:
//      logStr += ", new clearEditorTimer ID is " + nNewTimer;
//    }
    }
    logStr += ".\n";
    msiKludgeLogString(logStr);
//End logging stuff
    //To Do: re-parent appropriate dialogs at this point? or if there's a timer set, wait for it?
    theWindow.msiActiveEditorElement = editorElement;
    if ((theWindow.msiClearEditorTimerList==null || theWindow.msiClearEditorTimerList.length === 0) && theWindow.msiSingleDialogList)
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

//When this function is called, we need to not be returning null (as opposed to msiGetActiveEditorElement,
//which will return null if an editor isn't currently set as active - though that may not be correct either,
//it is intentional, so that for instance menu items can be disabled if no editor currently has focus.)
function msiGetCurrentEditor(theWindow)
{
  if (!theWindow)
    theWindow = window;
  var editorElement = msiGetActiveEditorElement(theWindow);
  if (editorElement)
    return msiGetEditor(editorElement);
  var currWindow = msiGetTopLevelWindow(currWindow);
  editorElement = msiGetCurrentEditorElementForWindow(currWindow);
  return msiGetEditor(editorElement);
//  return GetCurrentEditor();  //punt - this functionality should be handled by the above calls.
}

function msiGetActiveEditorElement(currWindow)
{
  var editorElement;
	if (!currWindow)
    currWindow = window.document.defaultView;
  if ("msiActiveEditorElement" in currWindow && (currWindow.msiActiveEditorElement !== null) )
    return currWindow.msiActiveEditorElement;
  currWindow = msiGetTopLevelWindow(currWindow);
  if ("msiActiveEditorElement" in currWindow  && (currWindow.msiActiveEditorElement !== null) )
    return currWindow.msiActiveEditorElement;
//  if (!editorElement && currWindow.opener && currWindow.opener !== currWindow)
//    editorElement = msiGetActiveEditorElement(currWindow.opener);
  editorElement = msiGetCurrentEditorElementForWindow(currWindow);
// BBM: stop here! The next line leads to an infinite loop
//  if (!editorElement)
//    editorElement = currWindow.GetCurrentEditorElement();  //Give up?
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
  if (currFocus && currFocus.nodeName === "editor")
    return currFocus;
  var editorList = theWindow.document.getElementsByTagName("editor");
  if (!currFocus)
  {
    var focusWindow = theWindow.document.commandDispatcher.focusedWindow;
    if (focusWindow && focusWindow !== theWindow)
    {
      for (var i = 0; i < editorList.length; ++i)
      {
        if (editorList[i].contentWindow === focusWindow)
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
  if (theWindow.document)
  {
    var editorElements = theWindow.document.getElementsByTagName("editor");
    for (var i = 0; (!theEditor) && (i < editorElements.length); ++i)
    {
      if (editorElements[i].getAttribute("type") === "content-primary")
        theEditor = editorElements[i];
    }
    if (!theEditor)
      theEditor = editorElements[0];
    if (!theEditor)
      dump("\nmsiGetPrimaryEditorElementForWindow returning void or null\m");
    return theEditor;
  }
  else {
    return msiGetEditorElement();
  }
}

function msiIsTopLevelEditor(editorElement)
{
  var topEditor = msiGetTopLevelEditorElement();
  return (topEditor === editorElement);
}

function msiGetTopLevelWindow(currWindow)  //do we need a different function for this? probably not - there is one, sort of?
{
  if (!currWindow)
    currWindow = window.document.defaultView;
  if ("ownerDocument" in currWindow)
    currWindow = currWindow.ownerDocument.defaultView;
  if (currWindow.parent && currWindow.parent !== currWindow)
    return msiGetTopLevelWindow(currWindow.parent);
  if (msiIsWebComposer(currWindow))
    return currWindow;
  if (currWindow.opener && currWindow.opener !== currWindow)
    return msiGetTopLevelWindow(currWindow.opener);
  return currWindow;
}

function msiGetParentOrTopLevelEditor(editorElement)
{
  var parent = msiGetParentEditor(editorElement);
  if (parent === null)
    parent = msiGetTopLevelEditorElement();
  return parent;
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
  dump("Can't find parent editor for editorElement " + editorElement.id + ".\n");
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
    if (theWindow.msiParentEditor !== parentEditor)
    {
      theWindow.msiParentEditor = parentEditor;
      retVal = true;
    }
  }
  var parentWindow = theWindow.parent;
  if (parentWindow !== null && parentWindow !== theWindow && "msiParentEditor" in parentWindow)
  {
    if (parentWindow.msiParentEditor !== parentEditor)
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
  if (currActive === editorElement)
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
    throw new MsiException("msiGetEditor exception",e);
    editor = null;
  }
  return editor;
}

function msiGetCommandManager(editorElement)
{
  var commandManager;
  try {commandManager = editorElement.commandManager;}
  catch(e) {
    throw new MsiException("In msiGetCommandManager",e);
    commandManager = null;
  }
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
    catch(e) {
      throw new MsiException("msiGetControllerForCommand",e);
    }
  }
  return controller;
}

function msiGetEditorType(editorElement)
{
  try {
    return editorElement.editortype;
  } catch (e) { 
    throw new MsiException("msiGetEditorType",e);
    return "";
  }
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
  
  if (theWindow.document !== null)
  {
    var editorList = theWindow.document.getElementsByTagName("editor");
    for (var i = 0; i < editorList.length; ++i)
    {
      if ( editorList.item(i)!==null && msiIsHTMLEditor(editorList.item(i)) )
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
      if (editorList.item(i) && editorList.item(i).contentDocument === innerDocument)
        return editorList.item(i);
    }
//    if (tmpWindow.frameElement && tmpWindow !== tmpWindow.frameElement.ownerDocument.defaultView)
//      tmpWindow = tmpWindow.frameElement.ownerDocument.defaultView;
//    else if (tmpWindow.top && tmpWindow !== tmpWindow.top)
//      tmpWindow = tmpWindow.top;
//    else if (tmpWindow.opener && tmpWindow !== tmpWindow.opener)
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
//    AlertWithTitle("Error in msiEditorUtilities.js", "Null innerDocument passed into findEditorElementForDocument!");
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
  if (msiGetBaseNodeName(editorElement) === "key")
    editorElement = msiGetCurrentEditorElementForWindow(window);

  if (!editorElement || !("editortype" in editorElement))
  {
    var origTarget = theEvent.explicitOriginalTarget;
    if (origTarget === null)
      origTarget = theEvent.originalTarget;
    var theDocument = null;
    if ("defaultView" in origTarget)
      theDocument = origTarget;
    if ((theDocument===null) && ("ownerDocument" in origTarget))
      theDocument = origTarget.ownerDocument;
    if ((theDocument===null) && ("top" in origTarget))
    {
      theDocument = origTarget.document;
//      var logStr = "In msiGetEditorElementFromEvent, original target document is [";
//      if (origTarget.document)
//        logStr += "not null";
//      else
//        logStr += "null";
//      logStr += "], but theDocument is [";
//      if (theDocument === null)
//        logStr += "null";
//      else
//        logStr += "not null";
//      logStr += "]\n";
//      msiKludgeLogString(logStr);
    }
    if (theDocument===null)
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
    if (theItem !== null)
      returnList.push(topWindow.document);
  }
  catch(e) {
    throw new MsiException("Error in msiGetUpdatableItemContainers", e);
  }

  if (!msiIsTopLevelEditor(editorElement) && currWindow !== null && currWindow !== topWindow)
  {
    var theItem = currWindow.document.getElementById(commandID);
    if (theItem !== null)
      returnList.push(currWindow.document);
  }
  if (window !== topWindow && window !== currWindow)
  {
    var theItem = window.document.getElementById(commandID);
    if (theItem !== null)
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
  } catch (e) {
    throw new MsiException("Error in msiIsDocumentEditable",e);
  }
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
  } catch (e) {
    throw new MsiException("Error in msiIsDocumentEmpty",e);
  }
  return false;
}

function msiIsDocumentModified(editorElement)
{
  try {
    return msiGetEditor(editorElement).documentModified;
  } catch (e) {
    throw new MsiException("Error in msiIsDocumentModified",e);
  }
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
      editor = msiGetEditor(editorElement);
      return editor.document.documentURI;
    } catch (e){
      throw new MsiException("Error in msiGetEditorURL",e);
    }
  }
  return "";
}


function msiRequirePackage(editorElement, packagename, options)
{
  try {
    var editor = msiGetEditor(editorElement); // BBM: NO, get main editor.
    var doc = editor.document;
    var preamble = doc.getElementsByTagName("preamble")[0];
    var currentReq = preamble.getElementsByTagName("requirespackage");
    var i;
    var req;
    var opt;
    for (i = 0; i < currentReq.length; i++)
    {
      req = currentReq[i].getAttribute("requirespackage");
      if (req === packagename) {
        opt = currentReq[i].getAttribute("opt");
        if (opt && options && opt === options) return; // already there
        if ((!opt) && (!options)) return; // both are void or null
      }
    }
    var reqpkg = doc.createElement("requirespackage");
    reqpkg.setAttribute("req", packagename);
    if (options && options.length > 0)
      reqpkg.setAttribute("opt", options);
    preamble.appendChild(reqpkg);
  }
  catch(e)
  {
    throw new MsiException("Exception in msiRequirePackage",e);
  }
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
  catch(e) {
    throw new MsiException("Error in newCommandParams",e); 
  }
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
  insertXMLNodes(editor, nodeList, node, offset);
}

function insertXMLNodes(editor, nodeList, node, offset)
{
  var i;
  // we can only insert nodes under an element, not under anything else such as a text node or a
  // processing instruction node. The cursor is likely in a text node
  //
  if (node.nodeType !== node.ELEMENT_NODE)
  {
    if (node.nodeType !== node.TEXT_NODE)
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
        if (list[i]===node) break;
      offset = i;
      node = node.parentNode;
    }
  }
  
  var nodeListLength = nodeList.length;
  for (i = nodeListLength-1; i >= 0; --i)
  {
    editor.insertNode(nodeList[i], node, offset);
  }
  editor.selection.collapse(node, offset+nodeListLength);   
}

function insertXMLAtCursor(editor, text, bWithinPara, bSetCaret)
{
  if (bWithinPara)
    text = "<para>" + text + "</para>";
  else
    text = "<body>" + text + "</body>";
  var parser = new DOMParser();
  var doc = parser.parseFromString(text,"application/xhtml+xml");
  var nodeList = doc.documentElement.childNodes;
  return insertXMLNodesAtCursor(editor, nodeList, bSetCaret);
}

function insertXMLNodesAtCursor(editor, nodeList, bSetCaret)
{
  if (!editor)
  {
    dump("Error in msiEditorUtilities.js, insertXMLAtCursor - null editor!\n");
    return false;
  }
  var theElement = null;
  var theOffset = 0;
  var theLength = 0;
  var bOK = true;
  try
  {
    theElement = editor.selection.focusNode;
    theOffset = editor.selection.focusOffset;
    var fixedPos = fixInsertPosition(editor, theElement, theOffset);
    if (fixedPos !== null)
    {
      theElement = fixedPos.theElement;
      theOffset = fixedPos.theOffset;
    }
    theLength = theElement.childNodes.length;
    insertXMLNodes(editor, nodeList, theElement, theOffset, true);
//    dump("insertXML now done.\n");
  }
  catch(e)
  {
    dump("If you see this dump, Barry loses his bet\n");
    dump("In insertXMLAtCursor, couldn't use editor.selection! (Exception: [" + exc + "]\n");
    theElement = editor.document.rootElement;
    if (theElement !== null && theElement.childNodes.length > 0)
    {
      var targetTag = "body";
      if (bWithinPara)
        targetTag = "bodyText";
      var theCollection = theElement.getElementsByTagName(targetTag);
      if (theCollection.length === 0 && bWithinPara)
        theCollection = theElement.getElementsByTagName("p");
      if (theCollection.length === 0)
        theCollection = theElement.childNodes;
      if (theCollection.length > 0)
        theElement = theCollection[theCollection.length - 1];
    }
    if (theElement !== null)
    {
      theLength = theOffset = theElement.childNodes.length;
      try
      {
        insertXMLNodes(editor, nodeList, theElement, theOffset, false);
      } 
      catch(exc)
      {
        bOK = false;
        throw new MsiException("Error in insertXMLNodesAtCursor",e);
      }
    }
    else
    {
      //No content nodes to insert into in editor
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
    } catch(e) {
      throw new MsiException("Error in insertXMLAtCursor",e);
    }
    //Find inserted nodes now in order to rationally place cursor.
    var newLength = theElement.childNodes.length;

    if (bSetCaret)
    {
      var caretNodeData = findCaretPositionAfterInsert(editor.document, theElement, theOffset, newElement, newOffset);

//      dump("New contents of editor are: [\n");
//      if (editor !== null)
//        editor.dumpContentTree();
//      dump("\n].\n");
//      if (caretNodeData === null && newElement !== null)
//      {
//        caretNodeData = new Object();
//        caretNodeData.theNode = newElement;
//        caretNodeData.theOffset = newOffset;
//      }
      if (caretNodeData !== null)
      {
        if (caretNodeData.theNode === null)
        {
          dump("In insertXMLAtCursor, got non-null caretNodeData with null caretNodeData.theNode!\n");
        }
        else
        {
          var adjustedCaret = moveCaretToTextChild(caretNodeData.theNode, caretNodeData.theOffset);
          if (adjustedCaret !== null)
            caretNodeData = adjustedCaret;
          try
          {
            editor.selection.collapse(caretNodeData.theNode, caretNodeData.theOffset);
  //          dump("In insertXMLAtCursor, set caret inside node [" + caretNodeData.theNode.nodeName + "], at offset [" + caretNodeData.theOffset + "], with old length = [" + theLength + "] and new length = [" + newLength + "].\n");
          }
          catch(exc) {
            throw new MsiException("Error in insertXMLAtCursor",exc);
          }
        }
      }
      else
        dump("In insertXMLAtCursor, no caretNode found.\n");
    }
  }
  return bOK;
}

function msiDeleteBodyContents(editor)
{
  if (!editor || (editor===null))
    editor = msiGetCurrentEditor();
  if (!editor)
  {
    dump("Error in msiEditorUtilities.js, msiDeleteBodyContents! No editor available!\n");
    return false;
  }
  var bodyElement = msiGetRealBodyElement(editor.document);
  var anchorNode = bodyElement;
  var focusNode = bodyElement;
  var focusOffset = 0;

  var DOMUtils = Components.classes["@mozilla.org/inspector/dom-utils;1"].createInstance(Components.interfaces.inIDOMUtils);
  function useNodeForSelection(aNode)
  {
    if (aNode.nodeName==="dialogbase")return true;
    if (aNode.nodeType === Components.interfaces.nsIDOMNode.TEXT_NODE)
      return !DOMUtils.isIgnorableWhitespace(aNode);
    return true;
  }

  if (bodyElement.childNodes.length)
  {
    for (var ix = 0; ix < bodyElement.childNodes.length; ++ix)
    {
      if (useNodeForSelection(bodyElement.childNodes[ix]))
      {
        anchorNode = bodyElement.childNodes[ix];
        break;
      }
    }
    for (var ix = bodyElement.childNodes.length - 1; ix >= 0; --ix)
    {
      if (useNodeForSelection(bodyElement.childNodes[ix]))
      {
        focusNode = bodyElement.childNodes[ix];
        break;
      }
    }
  }
  if (focusNode.childNodes && focusNode.childNodes.length)
    focusOffset = focusNode.childNodes.length;
  else if ("textContent" in focusNode)
    focusOffset = focusNode.textContent.length;

//  editor.selection.anchorNode = bodyElement;
//  editor.selection.anchorOffset = 0;
  editor.selection.collapse(anchorNode, 0);
//  editor.selection.focusNode = bodyElement;
//  editor.selection.focusOffset = bodyElement.childNodes.length;
  editor.selection.extend(focusNode, focusOffset);
//  dump( "In msiDeleteBodyContents, anchorNode and offset are [" + msiDumpDocLocation(editor.selection.anchorNode) + "; " + editor.selection.anchorOffset + "], while focusNode and offset are [" + msiDumpDocLocation(editor.selection.focusNode) + "; " + editor.selection.focusOffset + "].\n" );
//  dump("In msiDeleteBodyContents, anchorNode and offset are [" + editor.selection.anchorNode.nodeName + "," + editor.selection.anchorOffset + "], while focusNode and offset are [" + editor.selection.focusNode.nodeName + "," + editor.selection.focusOffset + "].\n");
  editor.deleteSelection(Components.interfaces.nsIEditor.eNone);
  return true;
}

function msiDumpDocLocation(node)
{
  function positionInParent(aNode)
  {
    if (aNode.parentNode !== null)
    {
      for (var ix = 0; ix < aNode.parentNode.childNodes.length; ++ix)
      {
        if (aNode.parentNode.childNodes[ix] === aNode)
          return ix;
      }
    }
    return -1;
  }
  var outString = "";
  var ancestors = new Array();
  for (var currNode = node; currNode.parentNode !== null && currNode.nodeType !== node.DOCUMENT_NODE; currNode = currNode.parentNode)
  {
    var ancestorRef = new Object();
    ancestorRef.nodename = currNode.nodeName;
    ancestorRef.nodeposition = positionInParent(currNode);
    ancestors.push(ancestorRef);
  }
  for (var jx = ancestors.length - 1; jx >= 0; --jx)
  {
    if (jx < ancestors.length - 1)
      outString += ", ";
    outString += "(" + ancestors[jx].nodeposition + ": " + ancestors[jx].nodename + ")";
  }
  return outString;
}

function msiGetPreamble(theEditor)
// BBM
// We don't want the head; we want the preamble. Otherwise we get TeX before the \documentclass
// in the resulting TeX file. I changed the name from msiGetDocumentHead to msiGetPreamble(theEditor)
{
  var docHead = null;
  var preambles = theEditor.document.documentElement.getElementsByTagName("preamble");
  if (preambles.length > 0) docHead = preambles[0];
//  var childElements = theEditor.document.documentElement.getElementsByTagName("*");
//  var testFor = ["preamble", "meta", "title", "link"];
//  for (var ix = 0; (docHead === null) && (ix < childElements.length); ++ix)
//  {
//    for (var jx = 0; jx < testFor.length; ++jx)
//    {
//      var testList = childElements[ix].getElementsByTagName(testFor[jx]);
//      if (testList.length > 0)
//      {
//        docHead = childElements[ix];
//        break;
//      }
//    }
//  }
//  if (docHead === null)
//  {
//    var headList = theEditor.document.getElementsByTagName("head");
//    if (headList.length > 0)
//      docHead = headList[0];
//  }
  return docHead;
}

function msiGetRealBodyElement(theDocument)
{
  var theElement = null;
  var targetTag = "body";
  var theNodes = null;
  if (!theDocument.rootElement)
  {
    theNodes = theDocument.getElementsByTagName(targetTag);
//    dump("In msiGetRealBodyElement; no document.rootElement, and search for 'body' nodes returned [" + theNodes.length + "] nodes.\n");
//    for (var iii = 0; iii < theNodes.length; ++iii)
//    {
//      var dumpStr = "  Body node [" + iii + "] contained [" + theNodes[iii].childNodes.length + "] children";
//      if (theNodes[iii].childNodes.length > 0)
//        dumpStr += ", of which the first is a [" + theNodes[iii].childNodes[0].nodeName + "]";
//      dump( dumpStr + "; the text content is [\n  " + theNodes[iii].textContent + "\n].\n");
//    }
  }
  else
  {
    if (theDocument.rootElement.nodeName === "body")
      return theDocument.rootElement;
    theNodes = theDocument.rootElement.getElementsByTagName(targetTag);
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
    if (theElement === null)
      theElement = theNodes[0];
  }
  else
    theElement = theDocument.rootElement;
  if (theElement === null)
    dump("No rootElement for document in msiGetRealBodyElement!\n");
  return theElement;
}

function fixInsertPosition(editor, theNode, offset)
{
  var retVal = new Object();
  retVal.theElement = theNode;
  retVal.theOffset = offset;

  var theElement = theNode;
  var topNode = msiGetRealBodyElement(editor.document);
  if (topNode === null)
    topNode = editor.document.documentElement;

  if (theElement === topNode)
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
  if (theParent !== null && theParent !== topNode)
  {
    retVal.theElement = theElement;
    retVal.theOffset = offset;
    return retVal;
  }

  function findBlockParents(aNode)
  {
    var blockParent = msiGetBlockNodeParent(editor, aNode);
    if (blockParent === topNode)
      return NodeFilter.FILTER_SKIP;
    else if (blockParent !== null) 
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
    if (foundNode !== null)
    {
      retVal.theElement = foundNode;
      retVal.theOffset = foundNode.childNodes.length;
    }
    else
    {
      foundNode = treeWalker.nextNode();
      if (foundNode !== null)
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
  for (var theParent = aNode; theParent !== null; theParent = theParent.parentNode)
  {
    if (editor.nodeIsBlock(theParent))
      return theParent;
    if (theParent.nodeName==="dialogbase")
      return theParent;
  }
  return null;
}

function msiFindParentOfType(startNode, nodeType, stopAt)
{
  var retNode = null;
  var theNode = startNode;
  if (stopAt == null || stopAt.length === 0)
    stopAt = "#document";
  while (retNode==null && theNode != null)
  {
    if (theNode.nodeName === nodeType)
      retNode = theNode;
    if (theNode.nodeName === stopAt)
      break;
    theNode = theNode.parentNode;
  }
  return retNode;
}

function msiGetBaseNodeName(node)
{
  return (node && ((node.localName)||(node.nodeName))) || null;
}

function msiElementCanHaveAttribute(elementNode, attribName)
{
  var retVal = true;  //by default, prevent nothing
  var elementName = msiGetBaseNodeName(elementNode);
  if (elementNode.nodeType === nsIDOMNode.TEXT_NODE)
    return false;

  switch( attribName.toLowerCase() )  //copy attributes we always need
  {
    case "limitPlacement":
      if (elementName !== "mo")
        retVal = false;
    break;
    case "msiclass":
      if (elementName !== "mi")
        retVal = false;  //is this right?
    break;
    default:  //don't bother generally? Maybe not a good idea
    break;
  }
  return retVal;
}

//Note that "editor" can be null - then this just goes through msiEnsureElementAttribute()
function msiEnsureElementCSSProperty(elementNode, propName, propValue, editor)
{
//  if (propValue && propValue.length)
//    elementNode.style.setProperty(propName, propValue, "");
//  else
//    elementNode.style.removeProperty(propName);
  var currStyleStr = elementNode.getAttribute("style");
  var replaceStr = "";
  if (propValue && propValue.length)
    replaceStr = propName + ": " + propValue + ";";
  var searchRE = new RegExp(propName + "\\:\\s*[^;]+;?");
  if (currStyleStr && currStyleStr.length)
  {
    if (searchRE.test(currStyleStr))
      currStyleStr.replace(searchRE, replaceStr);
    else
      currStyleStr += replaceStr;
  }
  else
    currStyleStr = replaceStr;
  msiEditorEnsureElementAttribute(elementNode, "style", currStyleStr, editor);
//  elementNode.setAttribute("style", currStyleStr);
}

function msiEnsureElementPackage(element, packageName, editor, options, priority)
{
  var attribStr = element.getAttribute("req");
  var currPackages = [];
  var currIndex = -1;
  var currOpts = [];
  var currPris = [];
  var retVal = false;

  if (attribStr && attribStr.length)
  {
    currPackages = attribStr.split(";");
    currIndex = currPackages.indexOf(packageName);
  }
  if (currIndex < 0)
  {
    currIndex = currPackages.length;  //Do this before pushing, since we'd have to subtract 1 to get correct index anyway
    currPackages.push(packageName);
  }
  attribStr = currPackages.join(";");
  if (attribStr.search(/[^\;\s]/) < 0)
    attribStr = "";
  retVal = msiEditorEnsureElementAttribute(element, "req", attribStr, editor) || retVal;

  attribStr = element.getAttribute("opt");
  if (attribStr && (attribStr > 0))
    currOpts = attribStr.split(";");
  if (options && (options.length > 0))
    currOpts[currIndex] = options;
  else
    currOpts[currIndex] = "";
  attribStr = currOpts.join(";");
  if (attribStr.search(/[^\;\s]/) < 0)
    attribStr = "";
  retVal = msiEditorEnsureElementAttribute(element, "opt", attribStr, editor) || retVal;

  attribStr = element.getAttribute("pri");
  if (attribStr && (attribStr > 0))
    currPris = attribStr.split(";");
  if (priority >= 0)
    currPris[currIndex] = priority;
  else
    currPris[currIndex] = "";
  attribStr = currPris.join(";");
  if (attribStr.search(/[^\;\s]/) < 0)
    attribStr = "";
  retVal = msiEditorEnsureElementAttribute(element, "pri", attribStr, editor) || retVal;

  return retVal;
}

function msiRemoveElementPackage(element, packageName, editor)
{
  var attribStr = element.getAttribute("req");
  var currPackages = [];
  var currIndex = -1;
  var currOpts = [];
  var currPris = [];
  var retVal = false;

  if (attribStr && attribStr.length)
  {
    currPackages = attribStr.split(";");
    currIndex = currPackages.indexOf(packageName);
  }
  if (currIndex < 0)
    return false;

  currPackages.splice(currIndex,1);
  attribStr = currPackages.join(";");
  if (attribStr.search(/[^\;\s]/) < 0)
    attribStr = "";
  retVal = msiEditorEnsureElementAttribute(element, "req", attribStr, editor) || retVal;

  attribStr = element.getAttribute("opt");
  if (attribStr && (attribStr > 0))
  {
    currOpts = attribStr.split(";");
    if (currOpts.length > currIndex)
      currOpts.splice(currIndex,1);
    attribStr = currOpts.join(";");
    if (attribStr.search(/[^\;\s]/) < 0)
      attribStr = "";
    retVal = msiEditorEnsureElementAttribute(element, "opt", attribStr, editor) || retVal;
  }

  attribStr = element.getAttribute("pri");
  if (attribStr && (attribStr > 0))
  {
    currPris = attribStr.split(";");
    if (currPris.length > currIndex)
      currPris.splice(currIndex,1);
    attribStr = currPris.join(";");
    if (attribStr.search(/[^\;\s]/) < 0)
      attribStr = "";
    retVal = msiEditorEnsureElementAttribute(element, "pri", attribStr, editor) || retVal;
  }

  return retVal;
}

function msiClearElementPackages(element, packageName, editor)
{
  var retVal = msiEditorEnsureElementAttribute(element, "req", null, editor);
  retVal = msiEditorEnsureElementAttribute(element, "req", null, editor) || retVal;
  retVal = msiEditorEnsureElementAttribute(element, "req", null, editor) || retVal;
  return retVal;
}

//Note that "editor" can be null - then this just goes through msiEnsureElementAttribute()
function msiEditorEnsureElementAttribute(elementNode, attribName, attribValue, editor)
{
  if (!editor)
    return msiEnsureElementAttribute(elementNode, attribName, attribValue);

  var retVal = false;
  if ( (!attribValue) || (attribValue.length === 0) )
  {
    if (elementNode.hasAttribute(attribName))
    {
      editor.removeAttribute(elementNode, attribName);
      retVal = true;
    }
  }
  else
  {
    if ( !elementNode.hasAttribute(attribName) || (elementNode.getAttribute(attribName) !== attribValue) )
    {
      editor.setAttribute(elementNode, attribName, attribValue);
      retVal = true;
    }
  }
  return retVal;
}


//This function sets an attribute value if needed and returns true if the value was changed.
function msiEnsureElementAttribute(elementNode, attribName, attribValue)
{
  var retVal = false;
  if (!attribValue || !attribValue.length)
  {
    if (elementNode.hasAttribute(attribName))
    {
      elementNode.removeAttribute(attribName);
      retVal = true;
    }
  }
  else
  {
    if ( !elementNode.hasAttribute(attribName) || (elementNode.getAttribute(attribName) !== attribValue) )
    {
      elementNode.setAttribute(attribName, attribValue);
      retVal = true;
    }
  }
  return retVal;
}

function msiGetObjElementParam(anElement, attrName)
{
  var theParam = null;
  var paramKids = msiNavigationUtils.getChildrenByTagName(anElement, "param");
  for (var ii = 0; !theParam && (ii < paramKids.length); ++ii)
  {
    if (paramKids[ii].getAttribute("name") == attrName)
      theParam = paramKids[ii];
  }
  return theParam;
}

function msiEditorEnsureObjElementParam(anElement, attrName, attrVal, editor)
{
  if (!editor)
    return msiEnsureObjElementParam(anElement, attrName, attrVal);

  var theParam = msiGetObjElementParam(anElement, attrName);
  if (theParam)
  {
    if (attrVal && attrVal.length)
      return msiEditorEnsureElementAttribute(theParam, "value", attrVal, editor);
    editor.deleteNode(theParam);
    return true;
  }

  if (!attrVal || !attrVal.length)  //We're deleting a <param> that isn't there; no change, so return false
    return false;

  theParam = editor.document.createElementNS(xhtmlns, "param");
  theParam.setAttribute("name", attrName);
  theParam.setAttribute("value", attrVal);
  editor.insertNode(theParam, anElement, anElement.childNodes.length);
  return true;
}

function msiEnsureObjElementParam(anElement, attrName, attrVal)
{
  var refDoc;  //We need a document for document.createElement()
  if (document)
    refDoc = document;
  else
  {
    dump("In msiEnsureObjElementParam, the 'document' variable is null!?\n");
    var editor = msiGetActiveEditor();
    if (editor)
      refDoc = editor.document;
  }

  var theParam = msiGetObjElementParam(anElement, attrName);
  if (theParam)
  {
    if (attrVal && attrVal.length)
      return msiEnsureElementAttribute(theParam, "value", attrVal);
    anElement.removeChild(theParam);
    return true;
  }

  if (!attrVal || !attrVal.length)  //We're deleting a <param> that isn't there; no change, so return false
    return false;

  theParam = refDoc.createElementNS(xhtmlns, "param");
  theParam.setAttribute("name", attrName);
  theParam.setAttribute("value", attrVal);
  anElement.appendChild(theParam);
  return true;
}

function msiEditorEnsureAttributeOrParam(anElement, attrName, attrVal, editor)
{
  if (msiGetBaseNodeName(anElement) == "object")
    return msiEditorEnsureObjElementParam(anElement, attrName, attrVal, editor);

  return msiEditorEnsureElementAttribute(anElement, attrName, attrVal, editor);
}

function msiCopyElementAttributes(newElement, oldElement, editor, bSuppressID)
{
  var theAttrs = oldElement.attributes;
  for (var jx = 0; jx < theAttrs.length; ++jx)
  {
//    var attrName = msiGetBaseNodeName(theAttrs.item(jx));
    var attrName = theAttrs.item(jx).nodeName;
    if (bSuppressID && theAttrs.item(jx).isId)
      continue;
    switch(attrName)
    {
      case '_moz-dirty':
      case '-moz-math-font-style':
      break;
      default:
        if (msiElementCanHaveAttribute(newElement, attrName))
        {
          if (editor)
            msiEditorEnsureElementAttribute(newElement, attrName, theAttrs.item(jx).textContent, editor);
          else
            msiEnsureElementAttribute(newElement, attrName, theAttrs.item(jx).textContent);
        }
      break;
    }
  }
}

function msiCopySpecifiedElementAttributes(newElement, oldElement, editor, attrList)
{
  var attrName, attrVal;
  for (var jx = 0; jx < attrList.length; ++jx)
  {
    attrName = attrList[jx];
    if (oldElement.hasAttribute(attrName))
      attrVal = oldElement.getAttribute(attrName);
    else
      attrVal = null;
    if (editor)
      msiEditorEnsureElementAttribute(newElement, attrName, attrVal, editor);
    else
      msiEnsureElementAttribute(newElement, attrName, attrVal);
  }
}

function msiCopySpecifiedObjElementParams(newObj, oldObj, paramList, editor)
{
  var oldKid, oldVal;
  for (var ii = 0; ii < paramList.length; ++ii)
  {
    oldKid = msiGetObjElementParam(oldObj, paramList[ii]);
    if (oldKid)
      oldVal = oldKid.getAttribute("value");
    else
      oldVal = null;
    msiEditorEnsureObjElementParam(newObj, paramList[ii], oldVal, editor);
  }
}

//This function appends a child node to a new parent
function msiEditorMoveChild(newParent, childNode, editor)
{
  msiEditorMoveChildToPosition(newParent, newParent.childNodes.length, childNode, editor);
}

function msiEditorMoveChildToPosition(newParent, nOffset, childNode, editor)
{
  try
  {
    editor.deleteNode(childNode);
    editor.insertNode(childNode, newParent, nOffset);
  }
  catch(exc) {
    throw new MsiException("Error in msiEditorMoveChildToPosition",exc);
  }
}


//This function appends the significant children of "fromElement" to the children of "toElement"
function msiEditorMoveChildren(toElement, fromElement, editor)
{
  if (msiNavigationUtils.isMathTemplate(toElement) || msiNavigationUtils.isMathTemplate(fromElement))
    return msiEditorMoveCorrespondingContents(toElement, fromElement, editor);
  var theChildren = msiNavigationUtils.getSignificantContents(fromElement);
  try
  {
    for (var ix = 0; ix < theChildren.length; ++ix)
      msiEditorMoveChild(toElement, theChildren[ix], editor);
  }
  catch(exc) {dump("Exception in msiEditorUtilities.js, msiEditorMoveChildren; exception is [" + exc + "].\n");}
}

function msiEditorMoveCorrespondingContents(targNode, srcNode, editor)
{
  var childContentTable = 
  {
    mover : { base : 1, sup : 2 },
    munder : { base : 1, sub : 2 },
    munderover : { base : 1, sub : 2, sup : 3 },
    msup : { base : 1, sup : 2 },
    msub : { base : 1, sub : 2 },
    msubsup : { base : 1, sub : 2, sup : 3 },
    menclose : { base : -1 },
    msqrt : { base : -1 },
    mroot : { base : 1, index : 2},
    mfrac : { num : 1, denom : 2}
  };
  function positionToContentName(aNodeName, nPos)
  {
    if (! (aNodeName in childContentTable) )
      return null;
    for (var aChild in childContentTable[aNodeName])
    {
      if (childContentTable[aNodeName][aChild] === nPos)
        return aChild;
    }
    return null;
  }
  function lastChildPosition(aNodeName)
  {
    var nPos = 0;
    if (aNodeName in childContentTable)
    {
      for (var aChild in childContentTable[aNodeName])
      {
        if (childContentTable[aNodeName][aChild] > nPos)
          nPos = childContentTable[aNodeName][aChild];
      }
    }
    return nPos;
  }

  var newName = msiGetBaseNodeName(targNode);
  var oldName = msiGetBaseNodeName(srcNode);
  var childNode = null;
  var newPos, oldPos;
  var aPosition = null;
  aPosition = positionToContentName(newName, -1);
  if (aPosition)  //so this one has all children together in one place
  {
    if ((oldName in childContentTable) && (aPosition in childContentTable[oldName]))
    {
      oldPos = childContentTable[oldName][aPosition];
      if (oldPos > 0)  //moving it from a specified position - may be wrapped in an mrow
      {
        childNode = msiNavigationUtils.getIndexedSignificantChild(srcNode, oldPos - 1);
        if (childNode)
        {
          if (msiNavigationUtils.isOrdinaryMRow(childNode))
            msiEditorMoveChildren(targNode, childNode, editor);
          else  //just a regular single node
          {
            editor.deleteNode(childNode);
            editor.insertNode(childNode, targNode, 0);
          }
        }
      }
      else
      {
        childNode = msiNavigationUtils.getIndexedSignificantChild(srcNode, 0);  //We're using "childNode" as a marker for success, so set it to the first child
        if (childNode)
          msiEditorMoveChildren(targNode, srcNode);
      }
    }
    if (!childNode)
    {
      childNode = newbox(editor);  //create an input box at this position
      editor.insertNode(childNode, targNode, 0);
    }
    return targNode;  //Since all of the child nodes are at unspecified positions, there can be nothing else to do.
  }
  for (var nn = 1; nn <= lastChildPosition(newName); ++nn)
  {
    aPosition = positionToContentName(newName, nn);
    if (aPosition)
    {
      if ( (oldName in childContentTable) && (aPosition in childContentTable[oldName]) )
      {
        oldPos = childContentTable[oldName][aPosition];
        if (oldPos < 0)  //this means we're moving all the children of srcNode to the desired position in targNode
        {
          childNode = srcNode.ownerDocument.createElementNS(mmlns, "mrow");
          editor.insertNode(childNode, targNode, nn - 1);
          msiEditorMoveChildren(childNode, srcNode, editor);
        }
        else  //so the old node had a specified position
        {
          childNode = msiNavigationUtils.getIndexedSignificantChild(srcNode, oldPos - 1);
          editor.deleteNode(childNode);
          editor.insertNode(childNode, targNode, nn - 1);
          editor.insertNode( newbox(editor), srcNode, oldPos - 1 );  //Do this to keep the child count of srcNode intact
        }
      }
      if (!childNode)
      {
        childNode = newbox(editor);
        editor.insertNode(childNode, targNode, nn - 1);
      }
    }
  }
}

function msiEditorReplaceTextWithText(editor, textNode, startOffset, endOffset, replaceText)
{
  var currentContent = textNode.textContent;
  var newContentEnd = currentContent.slice(endOffset);
  var newContentStart = currentContent.slice(0,startOffset);
  var newContent = newContentStart+replaceText+newContentEnd;
  var newTextNode = editor.document.createTextNode(newContent);
  var parent = textNode.parentNode;
  var offset = -1;
  var i;
  var kids = parent.childNodes;
  var l = kids.length;
  for (i=0; i < l; i++) {
    if (kids[i] === textNode) {
      offset = i;
      break;
    }
  }
  if (offset >= 0) {
    editor.insertNode(newTextNode, parent, offset);
    editor.deleteNode(textNode);
  }  
}
  
//  var newTextNode = editor.document.createTextNode(replaceText);
//  var theParentNode = textNode.parentNode;
//  var nOrigLen = textNode.textContent.length;
//  msiEditorReplaceTextWithNode2(editor, textNode, startOffset, endOffset, newTextNode);
//  var insertPos = msiNavigationUtils.offsetInParent(newTextNode);
//  var joinedNode = newTextNode;
//
//  if (startOffset > 0)
//  {
//    if ( (insertPos < 1) || !msiNavigationUtils.isTextNode(theParentNode.childNodes[insertPos-1]) )
//      dump("Error in msiEditorUtilities.js, in msiEditorReplaceTextWithText; newTextNode's previous sibling isn't a text node although startOffset>0!\n");
//    else
//    {
//      joinedNode = theParentNode.childNodes[insertPos - 1];
//      editor.joinNodes(joinedNode, newTextNode, theParentNode);
//      --insertPos;
//    }
//  }
//  msiKludgeLogString("In msiEditorUtilities.js, in msiEditorReplaceTextWithText, after the first joinNode.\n", ["reviseChars"]);
//  msiKludgeLogNodeContents(joinedNode, ["reviseChars"], "  joinedNode", true);
//  if (!joinedNode.parentNode || (joinedNode.parentNode !== theParentNode))
//    msiKludgeLogNodeContents(theParentNode.childNodes[insertPos], ["reviseChars"], "  The node at position [" + insertPos + "] in theParentNode");
//  if (endOffset < nOrigLen)
//  {
//    var logStr = "  Now parent node has [" + theParentNode.childNodes.length + "] children; ";
//    if ( (insertPos+1 >= theParentNode.childNodes.length) || !msiNavigationUtils.isTextNode(theParentNode.childNodes[insertPos+1]) )
//      dump("Error in msiEditorUtilities.js, in msiEditorReplaceTextWithText; newTextNode's next sibling isn't a text node although endOffset<origLen!\n");
//    else
//    {
//      var rightNode = theParentNode.childNodes[insertPos+1];
//      msiKludgeLogString(logStr, ["reviseChars"]);
//      msiKludgeLogNodeContents(joinedNode, ["reviseChars"], "  joinedNode", true);
//      msiKludgeLogNodeContents(rightNode, ["reviseChars"], "  rightNode", true);
//      editor.joinNodes(joinedNode, rightNode, theParentNode);
//    }
//  }
//  msiKludgeLogString("In msiEditorUtilities.js, in msiEditorReplaceTextWithText, after the second joinNode.\n", ["reviseChars"]);
//}

function msiEditorPrepareForInsertion(editor, nodeToInsert, insertPosition)
{
  var theParentNode = insertPosition.mNode;
  var theInsertPos = insertPosition.mOffset;
  var oldParent = null;
  msiKludgeLogString("Inside msiEditorPrepareForInsertion, with passed-in insertPosition.mNode [" + insertPosition.mNode.nodeName + "] and insertPosition.mOffset [" + insertPosition.mOffset + "]\n", ["reviseChars"]);

  function encloseNode(childToEnclose, enclosingNodeName, enclosingNodeNameSpace)
  {
    var newNode = null;
    if (enclosingNodeNameSpace)
      newNode = editor.document.createElementNS(enclosingNodeNameSpace, enclosingNodeName);
    else
      newNode = editor.document.createElement(enclosingNodeName);
    var posInParent = msiNavigationUtils.offsetInParent(childToEnclose);
    var theParent = childToEnclose.parentNode;
    editor.deleteNode(childToEnclose);
    editor.insertNode(newNode, theParent, posInParent);
    editor.insertNode(childToEnclose, newNode, 0);
    return newNode;
  }

  while (theParentNode && !msiNavigationUtils.nodeCanBeChild(nodeToInsert, theParentNode, editor))
  {
    msiKludgeLogNodeContents(nodeToInsert, ["reviseChars"], "Inside msiEditorPrepareForInsertion(), nodeToInsert can't be child of parent node.\n  nodeToInsert", false);
    msiKludgeLogNodeContents(theParentNode, ["reviseChars"], "  theParentNode", true);

    oldParent = theParentNode;
    if (msiNavigationUtils.isMathTemplate(theParentNode.parentNode))
      theParentNode = encloseNode(theParentNode, "mrow", mmlns);
    if (msiNavigationUtils.positionIsAtStart(oldParentNode, theInsertPos))  //in this case insert in parent's parent before parent
      theInsertPos = msiNavigationUtils.offsetInParent(oldParentNode);
    else if (msiNavigationUtils.positionIsAtEnd(oldParentNode, theInsertPos))  //in this case insert in parent's parent after parent
      theInsertPos = msiNavigationUtils.offsetInParent(oldParentNode) + 1;
    else  //do the split at the parent level
    {
      var aLeftNodeObj = new Object();
      editor.splitNode(oldParentNode, theInsertPos, aLeftNodeObj);
      theInsertPos = msiNavigationUtils.offsetInParent(oldParentNode);  //set up to insert at the split
    }
    if (oldParent === theParentNode)
      theParentNode = theParentNode.parentNode;
  }
//  if (msiNavigationUtils.isMathTemplate(theParentNode.parentNode))  //This surely was wrong
  if (msiNavigationUtils.isMathTemplate(theParentNode))
  {
    var replaceChild = msiNavigationUtils.getIndexedSignificantChild(theParentNode, theInsertPos);
    if (replaceChild !== null)
      theParentNode = encloseNode(replaceChild, "mrow", mmlns);
    msiKludgeLogString("Inside msiEditorPrepareForInsertion, isMathTemplate() returned true for theParentNode [" + theParentNode.nodeName + "]\n", ["reviseChars"]);
//    theParentNode = encloseNode(theParentNode, "mrow", mmlns);  //This surely was wrong
  }
  insertPosition.mNode = theParentNode;
  insertPosition.mOffset = theInsertPos;
  msiKludgeLogString("Ending msiEditorPrepareForInsertion, returning insertPosition.mNode [" + insertPosition.mNode.nodeName + "] and insertPosition.mOffset [" + insertPosition.mOffset + "]\n", ["reviseChars"]);
  return (theParentNode !== null);
}

function msiEditorReplaceTextWithNode2(editor, textNode, startOffset, endOffset, replaceNode)
{
  var rightNode = textNode;
  var leftNodeObj = new Object();
  var midNodeObj = new Object();
  var deleteNode = rightNode;
  var nOrigLen = textNode.textContent.length;
  var theParentNode = textNode.parentNode;
  if (startOffset > 0)
    editor.splitNode(rightNode, startOffset, leftNodeObj);
  if ((endOffset > startOffset) && (endOffset < nOrigLen))
  {
    editor.splitNode(rightNode, endOffset - startOffset, midNodeObj);
    deleteNode = midNodeObj.value;
  }

  var logStr = "In msiEditorUtilities.js, in msiEditorReplaceTextWithNode, after splitNode calls; leftNode is [";
  if (leftNodeObj && leftNodeObj.value)
    logStr += leftNodeObj.value.textContent;
  logStr += "], midNode is [";
  if (midNodeObj && midNodeObj.value)
    logStr += midNodeObj.value.textContent;
  logStr += "], and rightNode is [";
  if (rightNode && rightNode.textContent)
    logStr += rightNode.textContent;
  logStr += "].\n";
  msiKludgeLogString(logStr, ["reviseChars"]);

  var insertPos = msiNavigationUtils.offsetInParent(deleteNode);
  editor.deleteNode(deleteNode);
  logStr = "In msiEditorUtilities.js, in msiEditorReplaceTextWithNode, after deleteNode, parent node has [" + theParentNode.childNodes.length + "] children.\n";
  msiKludgeLogString(logStr, ["reviseChars"]);

  var insertNode = replaceNode;
  if (msiNavigationUtils.isMathNode(replaceNode) && !findmathparent(theParentNode) && (msiGetBaseNodeName(replaceNode) !== "math"))
  {
    var mathNode = editor.document.createElementNS(mmlns, "math");
    if (msiGetBaseNodeName(replaceNode) !== "mrow")
    {
      insertNode = editor.document.createElementNS(mmlns, "mrow");
      insertNode.appendChild(replaceNode);
    }
    mathNode.appendChild(insertNode);
    insertNode = mathNode;
  }

//HOW TO DEAL WITH MATH TEMPLATES? Example: We want to replace part of the text in a math leaf. The leaf node does not allow
//  inserting anything other than text, and so we try to split it into two adjacent leaf nodes and insert the appropriate node.
//  But this (or even the first step?) may force the introduction of an mrow - how do we check???
//THE OTHER ISSUE may involve math leaf nodes which will lose their text content and thus need to be replaced. In this case,
//  we should see the insertNode unable to be a child of the leaf, so the algorithm would split the leaf, leaving a null (left?)
//  piece and an empty right one. In this case we should delete the remaining empty node!
//Code moved to msiEditorPrepareForInsertion() function above.
  var insertPosition = {mNode : theParentNode, mOffset : insertPos};
  if (msiEditorPrepareForInsertion(editor, insertNode, insertPosition))
    editor.insertNode(insertNode, insertPosition.mNode, insertPosition.mOffset);

  logStr = "In msiEditorUtilities.js, in msiEditorReplaceTextWithNode, after insertNode, parent node has [" + theParentNode.childNodes.length + "] children; \n";
  msiKludgeLogString(logStr, ["reviseChars"]);
  msiKludgeLogNodeContents(replaceNode, ["reviseChars"], "  replaceNode", true);
  if (replaceNode !== insertNode)
    msiKludgeLogNodeContents(insertNode, ["reviseChars"], "  insertNode", true);
  msiKludgeLogNodeContents(rightNode, ["reviseChars"], "  rightNode", true);
  if (!replaceNode.parentNode || (replaceNode.parentNode !== theParentNode))
    msiKludgeLogNodeContents(theParentNode.childNodes[insertPos], ["reviseChars"], "  The node at position [" + insertPos + "] in theParentNode");
}

function msiEditorReplaceTextWithNode(theEditor, textNode, startOffset, endOffset, replaceNode)
{
//  What can we use to split a node when that function (editor.splitNode) doesn't work properly? I'm guessing just replace the nodes, if we can
//    isolate the case...
  if (!textNode || (textNode.nodeType !== nsIDOMNode.TEXT_NODE))
  {
    dump("Problem in msiEditorUtilities.js, msiEditorReplaceTextWithNode() - null textNode passed in!\n");
    return;
  }
  if (textNode.nodeType !== nsIDOMNode.TEXT_NODE)
  {
    dump("Problem in msiEditorUtilities.js, msiEditorReplaceTextWithNode() - non-text textNode [" + textNode.nodeName + "] passed in!\n");
    return;
  }
  theEditor.beginTransaction();
  var logStr = "";

  var leftTextNode = null;
//  var deleteNode = null;
  var rightTextNode = null;
  var theText = textNode.textContent;
  var leftText = theText.substring(0, startOffset);
  var midText = theText.substring(startOffset, endOffset);
  var rightText = theText.substring(endOffset);
  var nTextLen = theText.length;
  var theParentNode = textNode.parentNode;
  var nStartPos = msiNavigationUtils.offsetInParent(textNode);
  var nParentLen = theParentNode.childNodes.length;
//  var dummyLeftNode = theEditor.document.createTextNode("");
//  if ( (startOffset > 0) && (endOffset < nTextLen) )
//  {
//    msiKludgeLogString("In msiEditorUtilities.js, in msiEditorReplaceTextWithNode(); before the first splitNode.\n", ["spaces"]);
//    theEditor.splitNode(textNode, startOffset, dummyLeftNode);
//    if (theParentNode.childNodes.length !== nParentLen + 1)  //this means the split simply failed! fix it by hand
//    {
//      logStr = "In msiEditorUtilities.js, in msiEditorReplaceTextWithNode(); after the first splitNode, inside the trouble clause; textNode ";
//      if (textNode)
//        logStr += "has content [" + textNode.textContent + "].\n";
//      else
//        logStr += "is null.\n", 
//      msiKludgeLogString(logStr, ["spaces"]);
//      theEditor.deleteNode(textNode);
//      msiKludgeLogString("In msiEditorUtilities.js, in msiEditorReplaceTextWithNode(); after the first splitNode, inside the trouble clause, after the deleteNode.\n", ["spaces"]);
//      leftTextNode = theEditor.document.createTextNode(leftText);
//      theEditor.insertNode(leftTextNode, theParentNode, nStartPos);
//      msiKludgeLogString("In msiEditorUtilities.js, in msiEditorReplaceTextWithNode(); after the first splitNode, inside the trouble clause, after the first insertNode.\n", ["spaces"]);
//      textNode = theEditor.document.createTextNode(midText + rightText)
//      theEditor.insertNode(textNode, theParentNode, ++nStartPos);  //nStartPos tracks the offset at which we want to replace
//      msiKludgeLogString("In msiEditorUtilities.js, in msiEditorReplaceTextWithNode(); after the first splitNode, inside the trouble clause, after the second insertNode.\n", ["spaces"]);
//    }
//    else
//    {
//      ++nStartPos;
//      leftTextNode = textNode.prevSibling;
//    }
//    deleteNode = textNode;
//    nParentLen = theParentNode.childNodes.length;  //reset this now
//
//    var secondDummy = theEditor.document.createTextNode("");
////    logStr = "In msiEditorUtilities.js, in msiEditorReplaceTextWithNode(); inside before the second splitNode.\n  textNode contains [";
////    logStr += textNode.textContent + "], leftText is [" + leftText + "] and rightText is [" + rightText + "].\n";
////    msiKludgeLogString(logStr, ["spaces"]);
//    msiKludgeLogNodeContents(theParentNode, ["spaces"], "In msiEditorUtilities.js, in msiEditorReplaceTextWithNode(); inside before the second splitNode.\n  theParentNode ");
//    theEditor.splitNode(textNode, endOffset - startOffset, secondDummy);
//    if (theParentNode.childNodes.length !== nParentLen + 1)  //this means the split simply failed! fix it by hand
//    {
//      msiKludgeLogString("In msiEditorUtilities.js, in msiEditorReplaceTextWithNode(); after the second splitNode, inside the trouble clause; theParentNode.childNodes.length is [" + theParentNode.childNodes.length + "] and nParentLen is [" + nParentLen + "].\n", ["spaces"]);
//      theEditor.deleteNode(textNode);
//      deleteNode = theEditor.document.createTextNode(midText)
//      theEditor.insertNode(deleteNode, theParentNode, nStartPos);
//      rightTextNode = theEditor.document.createTextNode(rightText);
//      theEditor.insertNode(rightTextNode, theParentNode, ++nStartPos);  //nStartPos tracks the offset at which we want to replace
//    }
//    else
//    {
//      rightTextNode = textNode;
//      deleteNode = textNode.prevSibling;
//      msiKludgeLogNodeContents(theParentNode, ["spaces"], "In msiEditorUtilities.js, in msiEditorReplaceTextWithNode(); inside before the second splitNode.\n  theParentNode ");
//
//      logStr = "In msiEditorUtilities.js, in msiEditorReplaceTextWithNode(); after the second splitNode, in the okay clause.\n  deleteNode contains [";
//      if (deleteNode)
//        logStr += deleteNode.textContent;
//      logStr += "], rightTextNode contains [" + rightTextNode.textContent + "].\n  theParentNode";
//      msiKludgeLogNodeContents(theParentNode, ["spaces"], logStr);
////      msiKludgeLogString(logStr, ["spaces"]);
//    }
//  }
//  else 
//Forget all the splitNode calls - just try to hammer it in the crude way:
//  {
    logStr = "In msiEditorUtilities.js, in msiEditorReplaceTextWithNode(); inside the trouble clause, before deleting and inserting.\n  textNode contains [";
    logStr += textNode.textContent + "], leftText is [" + leftText + "and rightText is [" + rightText + "].\n";
    msiKludgeLogString(logStr, ["spaces"]);
    //Add stuff to copy any attributes on the text nodes!
    if (leftText && leftText.length)
    {
      leftTextNode = theEditor.document.createTextNode(leftText);
      theEditor.insertNode(leftTextNode, theParentNode, nStartPos++);
    }
    if (rightText && rightText.length)
    {
      rightTextNode = theEditor.document.createTextNode(rightText);
      theEditor.insertNode(rightTextNode, theParentNode, nStartPos);
    }
    theEditor.deleteNode(textNode);
//  }
//  if (deleteNode)
//    theEditor.deleteNode(deleteNode);
  theEditor.insertNode(replaceNode, theParentNode, nStartPos);
  theEditor.endTransaction();
}


//NOTE!! This is to get around a bug in Mozilla. When the bug is fixed, this function should just read:
//  theElement.textContent = newText; 
//  return theElement;
function msiSetMathTokenText(theElement, newText, editor)
{
  if (theElement.textContent === newText)
    return theElement;

  var newElement = theElement.ownerDocument.createElementNS(mmlns, theElement.nodeName);
  msiCopyElementAttributes(newElement, theElement);
  newElement.appendChild(theElement.ownerDocument.createTextNode(newText));
  if (theElement.parentNode !== null)
  {
    if (editor !== null)
      editor.replaceNode(newElement, theElement, theElement.parentNode);
    else
      theElement.parentNode.replaceChild(newElement, theElement);
  }
  return newElement;
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
  if (comp === Node.DOCUMENT_POSITION_FOLLOWING)
  {
    startNode = postNode;
    startOffset = postOffset;
    endNode = preNode;
    endOffset = preOffset;
  }
  if (startOffset > startNode.childNodes.length)
    startOffset = startNode.childNodes.length;
  if (startOffset > 0 && startNode.childNodes.length > startOffset)
  {
    startNode = startNode.childNodes[startOffset];
    startOffset = 0;
  }
  if (endOffset > endNode.childNodes.length)
    endOffset = endNode.childNodes.length;
  if (endOffset > 0 && endNode.childNodes.length > endOffset)
  {
    endNode = endNode.childNodes[endOffset];
    endOffset = endNode.childNodes.length;
  }
  var theRange = document.createRange();
//  theRange.setStart(preNode, preOffset);
//  theRange.setEnd(postNode, postOffset);
  theRange.setStart(startNode, startOffset);
  theRange.setEnd(endNode, endOffset);
  var rootNode = theRange.commonAncestorContainer;
  theRange.detach();
  
  function isCaretPosition(aNode)
  {

    if (aNode.hasAttribute("caretpos") || (aNode.hasAttribute("tempinput") && aNode.getAttribute("tempinput")==="true"))
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
    while (!bFoundCaretPos && nextPos !== null && nextPos.compareDocumentPosition(endNode) !== Node.DOCUMENT_POSITION_PRECEDING)
    {
      if (nextPos.hasAttribute("caretpos"))
      {
        bFoundCaretPos = true;
        retVal.theNode = nextPos;
        retVal.theOffset = nextPos.getAttribute("caretpos");
      }
      else if (!bFoundInput && nextPos.hasAttribute("tempinput") && nextPos.getAttribute("tempinput")==="true")
      {
        bFoundInput = true;
        retVal.theNode = nextPos;
        retVal.theOffset = msiInputBoxCaretOffset;  //set to 1
      }
      nextPos = treeWalker.nextNode();
    }
  }

////  if (retVal !== null) //In these cases we attempt to put the caret inside a child text node.
////  {
//  var nNonEmptyTextChild = -1;
//  for (var kx = 0; kx < retVal.theNode.childNodes.length; ++kx)
//  {
//    if (retVal.theNode.childNodes[kx].nodeType !== Components.interfaces.nsIDOMNode.TEXT_NODE)
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
//    if (theNode.childNodes[kx].nodeType !== Components.interfaces.nsIDOMNode.TEXT_NODE)
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
  if (theNode.nodeType === Components.interfaces.nsIDOMNode.TEXT_NODE)
  {
    var posObject = new Object();
    posObject.theNode = theNode;
    if (theNode.parentNode.hasAttribute("tempinput") && theNode.parentNode.getAttribute("tempinput")==="true")
      posObject.theOffset = msiInputBoxCaretOffset;  //set to 1
    else if (theOffset === -1 || theOffset > theNode.nodeValue.length)
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

  if (theOffset === -1 || theOffset > nLength)
  {
    for (var jx = nLength; jx > 0; --jx)
    {
      if (theNode.childNodes[jx-1].nodeType !== Components.interfaces.nsIDOMNode.TEXT_NODE || theNode.childNodes[jx-1].nodeValue.length > 0)
      {
        theChild = theNode.childNodes[jx-1];
        break;
      }
    }
  }
  else if (theOffset === 0)
  {
    for (var jx = 0; jx < nLength; ++jx)
    {
      if (theNode.childNodes[jx].nodeType !== Components.interfaces.nsIDOMNode.TEXT_NODE || theNode.childNodes[jx].nodeValue.length > 0)
      {
        theChild = theNode.childNodes[jx];
        break;
      }
    }
  }
  else
    theChild = theNode.childNodes[theOffset - 1];

  if (theOffset !== 0)
    theOffset = -1;
  return moveCaretToTextChild(theChild, theOffset);
}

//function moveCaretToTextChild(theNode, theOffset)
//{
//  if (theNode.nodeType === Components.interfaces.nsIDOMNode.TEXT_NODE)
//  {
//    var posObject = new Object();
//    posObject.theNode = theNode;
//    if (theOffset === -1 || theOffset > theNode.nodeValue.length)
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
  if (parentNode.childNodes.length === 0)
  {
    searchNodes = new Array(parentNode);
    bDoneMi = true;
  }
  else
    searchNodes = parentNode.getElementsByTagName("mi");
  if (searchNodes === null)
    return foundCaret;
  for (var jx = 0; ((jx < searchNodes.length) || (!bDoneMi)) && !bFoundCaretPos; ++jx)
  {
    bFoundNew = false;
    if (jx >= searchNodes.length && !bDoneMi)
    {
      searchNodes = parentNode.getElementsByTagName("*");
      bDoneMi = true;
      if (searchNodes.length === 0)
        break;
      jx = 0;
    }
    //The use of the attribute "caretpos" here is illustrative - need to find out what we may actually use.
    if ( !bFoundAny && (searchNodes[jx].nodeType === Components.interfaces.nsIDOMNode.TEXT_NODE) && (searchNodes[jx].nodeValue.length > 0) )
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
    else if (!bFoundInputBox && searchNodes[jx].hasAttribute("tempinput") && searchNodes[jx].getAttribute("tempinput")==="true")
    {
      bFoundInputBox = bFoundNew = true;
      caretNode = searchNodes[jx];
      caretOffset = msiInputBoxCaretOffset;  //set to 1
    }
    else if ( !bFoundInputBox && !bFoundAny && ((searchNodes[jx].nodeType !== Components.interfaces.nsIDOMNode.TEXT_NODE) || (searchNodes[jx].nodeValue.length > 0)) )
    {
      //Found a viable cursor position.
      caretNode = searchNodes[jx];
      bFoundAny = bFoundNew = true;
    }
    if (bFoundNew && caretNode !== null) //In these cases we attempt to put the caret inside a child text node.
    {
      var nNonEmptyTextChild = -1;
      for (var kx = 0; kx < caretNode.childNodes.length; ++kx)
      {
        if (caretNode.childNodes[kx].nodeType !== Components.interfaces.nsIDOMNode.TEXT_NODE)
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
  if (caretNode !== null)
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
    if (internalEditor !== null)
    {
      msiDumpWithID("Got HTML editor for element [@] in msiEnableEditorControl; editor flags are [" + internalEditor.flags + "].\n", editorElement);
      internalEditor.flags &= ~(Components.interfaces.nsIPlaintextEditor.eEditorReadonlyMask | Components.interfaces.nsIPlaintextEditor.eEditorDisabledMask);
    }
    else
      msiDumpWithID("Unable to get HTML editor for element [@] in msiEnableEditorControl.\n", editorElement);
    if (elementStyle !== null)
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
    if (internalEditor !== null)
    {
      msiDumpWithID("Got HTML editor for element [@] in msiEnableEditorControl; editor flags are [" + internalEditor.flags + "].\n", editorElement);
      internalEditor.flags |= (Components.interfaces.nsIPlaintextEditor.eEditorReadonlyMask | Components.interfaces.nsIPlaintextEditor.eEditorDisabledMask);
    }
    else
      msiDumpWithID("Unable to get HTML editor for element [@] in msiEnableEditorControl.\n", editorElement);
    if (elementStyle !== null)
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
        if (topWin.msiSingleDialogList.ourList[entry].theDialog === theDialog)
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
      if (this.ourList[entry].theDialog === theDialog)
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
    if (aNode.documentElement.nodeName === "dialog")
      return aNode.defaultView;
    aNode = aNode.defaultView;
  }
  if ("opener" in aNode)
  {
    parentWindow = aNode;
    if (aNode.parent !== aNode)
      parentWindow = aNode.parent;
    if (parentWindow.document.documentElement.nodeName === "dialog")
      return parentWindow;
  }
  if ("ownerDocument" in aNode && aNode.ownerDocument !== null && aNode.ownerDocument.documentElement.nodeName === "dialog")
    return aNode.ownerDocument.defaultView;
  return null;
};

function msiEditorIsDependentOnWindow(editorElement, theWindow)
{
  var currParentWin = editorElement.ownerDocument.defaultView;
  var currWindow = editorElement.contentWindow;
  var bIsDependent = (currWindow === theWindow) || (currParentWin === theWindow);
  while (!bIsDependent && currParentWin!==null)
  {
    if (currParentWin.parent && currParentWin.parent !== currParentWin)
      currParentWin = currParentWin.parent;
    else
      currParentWin = currParentWin.opener;
    bIsDependent = (currParentWin === theWindow);
    if (!bIsDependent && currParentWin)
    {
      var parentEditor = findEditorElementForDocument(currParentWin.document);
      if (parentEditor !== null)
      {
        currWindow = currParentWin;
        currParentWin = parentEditor.ownerDocument.defaultView;
        if (currParentWin.top)
          currParentWin = currParentWin.top;
        bIsDependent = (currParentWin === theWindow);
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

function msiSetEditorSinglePara(editorElement, bSet)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);
  if (editor && editor.document)
  {
    try {
      var flags = editor.flags;
      editor.flags = bSet ?  
            flags | nsIPlaintextEditor.eEditorSingleLineMask :
            flags & ~nsIPlaintextEditor.eEditorSingleLineMask;
    } catch(e) {}
//    editorElement.mbSinglePara = bSet;
    // update all commands
    window.updateCommands("create");
  }  
}

function msiEditorIsSinglePara(editorElement)
{
  if (editorElement.mbSinglePara)
    return true;
  var editor = msiGetEditor(editorElement);
  if (editor && editor.document)
    return ((editor.flags & nsIPlaintextEditor.eEditorSingleLineMask) !== 0);
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
//  if (commandHandler && ("bRevising" in commandHandler) && commandHandler.bRevising && ("msiGetReviseObject" in commandHandler))
  if (commandHandler && ("msiGetReviseObject" in commandHandler))
    reviseObject = commandHandler.msiGetReviseObject(targetEditorElement);
  var extraArgsArray = new Array();
  for (var i = 6; i < arguments.length; ++i)
  {
    extraArgsArray.push(arguments[i]);
  }
  if (reviseObject !== null)
    return msiOpenModelessPropertiesDialog(chromeUrl, dlgName, options, targetEditorElement, commandID, reviseObject, extraArgsArray);
  else
    return msiOpenSingleInstanceModelessDialog(chromeUrl, dlgName, options, targetEditorElement, commandID, extraArgsArray);
}

function msiDoModelessPropertiesDialog(chromeUrl, dlgName, options, targetEditorElement, commandID, reviseObject)
{
  var editor = msiGetEditor(targetEditorElement);
  var extraArgsArray = new Array();
  for (var i = 6; i < arguments.length; ++i)
  {
    extraArgsArray.push(arguments[i]);
  }
  return msiOpenModelessPropertiesDialog(chromeUrl, dlgName, options, targetEditorElement, commandID, reviseObject, extraArgsArray);
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
//  reviseObject = commandHandler.msiGetReviseObject(targetEditorElement);
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
//  if (parWindow !== theWindow)
//    return parWindow;
//  //else???? for now, assume this always works?
//  var theWindow = editorElement.contentWindow;
//  if (theWindow.parent && theWindow.parent !== theWindow)
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
        if (this.ourList[dialogName][i].theObject === reviseObject)
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
        for (var i = topWin.msiPropertiesDialogList.ourList[entry].length; i >= 1; --i)
        {
          if (topWin.msiPropertiesDialogList.ourList[entry][i-1].theDialog === theDialog)
          {
  //          msiKludgeLogString("Removing dialog entry in closing observer.\n");
            topWin.msiPropertiesDialogList.ourList[entry].splice(i-1, 1);
//            delete topWin.msiPropertiesDialogList.ourList[entry][i-1];
            bFound = true;
            if (topWin.msiPropertiesDialogList.ourList[entry].length === 0)
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
    var theEntry = this.findEntryForDialog(theDialog);
    if (theEntry !== null)
      return theEntry.theEditor;
    return null;
  }
  this.findEntryForDialog = function(theDialog)
  {
    for (var entry in this.ourList)
    {
      for (var i = 0; i < this.ourList[entry].length; ++i)
      {
        if (this.ourList[entry][i].theDialog === theDialog)
        {
//          msiKludgeLogString("Removing dialog entry in closing observer.\n");
          return this.ourList[entry][i];
          break;
        }
      }
    }
    return null;
  }
}

//function msiSupportsVoid(dataObj)
//{
//  ourData : dataObj,
//////Interface:
//  QueryInterface : function(aIID)
//  {
//    if (aIID.equals(Components.interfaces.nsISupports)
//    || aIID.equals(Components.interfaces.nsISupportsWeakReference))
//      return this;
//    throw Components.results.NS_NOINTERFACE;
//  },
//
//
//}

function msiSetCommandParamWeakRefValue(commandParams, dataName, dataObj)
{
//  var iSupportsVoid = Components.classes["@mozilla.org/supports-void;1"]
//                          .createInstance(Components.interfaces.nsISupportsVoid);
//  iSupportsVoid.data = dataObj;
  var weak = Components.utils.getWeakReference(dataObj);
//  dump("In msiSetCommandParamVoidValue, the weak reference returns [" + weak.get() + "]\n");
  commandParams.setISupportsValue(dataName, weak);
}

function msiGetReviseObjectFromCommandParams(aParams)
{
  var theObject = null;
  var propsData = msiGetPropertiesDataFromCommandParams(aParams);
  if (propsData)
    theObject = propsData.getPropertiesDialogNode();
  if (!theObject)
  {
    theObject = aParams.getISupportsValue("reviseObject");
    dump("In msiEditorUtilities.js, msiGetReviseObjectFromCommandParams for object [" + theObject + "], no propertiesData object available, or the propertiesData object reports no reference node!\n");
  }
  return theObject;
}

function msiGetPropertiesDataFromCommandParams(aParams)
{
  var propsData = null;
  var propsDataPtr = aParams.getISupportsValue("propertiesData");
  var qiPropsDataPtr = propsDataPtr.QueryInterface(Components.interfaces.xpcIJSWeakReference);
  if (qiPropsDataPtr)
    propsData = qiPropsDataPtr.get();
  return propsData;
}


function msiDebugWindowInfo(aWindow)
{
  var idStr = "";
  if (!aWindow)
    return "null window";
  if ("id" in aWindow)
    idStr += "id: " + aWindow.id + ",";
  idStr += "docTitle: {" + aWindow.document.title + "}";
  return idStr;
}

//Utility for use by editors in dialogs.
function msiGetParentEditorElementForDialog(dialogWindow, bLogEverything)
{
  if (!dialogWindow)
  {
    dialogWindow = window;
    if (bLogEverything)
      dump("In msiGetParentEditorElementForDialog, dialogWindow not passed in, 'window' is [" + msiDebugWindowInfo(dialogWindow) + "].\n");
  }
  if (dialogWindow.top && (dialogWindow !== dialogWindow.top))
  {
    dialogWindow = dialogWindow.top;
    if (bLogEverything)
      dump("In msiGetParentEditorElementForDialog, dialogWindow.top differs from dialogWindow, top is [" + msiDebugWindowInfo(dialogWindow) + "].\n");
  }
  var editorElement = null;
  if ("msiParentEditor" in dialogWindow)
  {
    editorElement = dialogWindow.msiParentEditor;
    if (bLogEverything)
      msiDumpWithID("In msiGetParentEditorElementForDialog, msiParentEditor data member for dialogWindow has id [@]\n", editorElement);
  }
  if (!editorElement)
  {
    var topWindow = msiGetTopLevelWindow(dialogWindow);
    if (topWindow.msiSingleDialogList)
    {
      editorElement = topWindow.msiSingleDialogList.getParentEditorElementByDialog(dialogWindow);
      if (bLogEverything)
        msiDumpWithID("In msiGetParentEditorElementForDialog, msiParentEditor data member for dialogWindow was null, msiSingleDialogList yields editor with id [@]\n", editorElement);
    }
    if (!editorElement && topWindow.msiPropertiesDialogList)
    {
      editorElement = topWindow.msiPropertiesDialogList.getParentEditorElementByDialog(dialogWindow);
      if (bLogEverything)
        msiDumpWithID("In msiGetParentEditorElementForDialog, msiParentEditor data member for dialogWindow was null, msiPropertiesDialogList yields editor with id [@]\n", editorElement);
    }
  }
  if (!editorElement)
  {
    var parentWindow = dialogWindow.opener;
    if (parentWindow)
    {
      editorElement = msiGetPrimaryEditorElementForWindow(parentWindow);
      if (bLogEverything)
        msiDumpWithID("In msiGetParentEditorElementForDialog, msiParentEditor data member for dialogWindow was null, lists failed, using window.opener's primary editor, with id [@]\n", editorElement);
    }
  }
  if (!editorElement)
  {
    editorElement = msiGetActiveEditorElement();
    if (bLogEverything)
      msiDumpWithID("In msiGetParentEditorElementForDialog, msiParentEditor data member for dialogWindow was null, lists failed, using active editor, with id [@]\n", editorElement);
  }
  return editorElement;
}


/************* General editing command utilities ***************/

function msiGetDocumentTitle(editorElement)
{
    var title = "untitled"; 
    var doc = msiGetEditor(editorElement).document;
    var nodes = doc.getElementsByTagName('preamble');
    var titleNodes;
    if (nodes.length > 0) titleNodes= nodes[0].getElementsByTagName("title");
    if (titleNodes.length > 0 && titleNodes[0].textContent.length > 0)
      title = nodes[0].textContent;
    return title.replace(/^[ \n\t]*/,"").replace(/[ \n\t]*$/,"");  // BBM: localize
}

function msiSetDocumentTitle(editorElement, title)
{
  // if we changed the name of a shell document, we saved the filename in 
  // a broadcaster with id="filename"
  var theFilename = document.getElementById("filename");
  var newtitle = "";
  if (theFilename)
    newtitle = theFilename.value;
  if (newtitle.length > 0) title = newtitle;
  try {
    msiGetEditor(editorElement).msietDocumentTitle(title);

    // Update window title (doesn't work if called from a dialog)
    if ("msiUpdateWindowTitle" in window)
    {
      window.msiUpdateWindowTitle(msiGetDocumentTitle(editorElement),newtitle);
    }
  } catch (e) {}
}

var gAtomService;
function GetAtomService()
{
  gAtomService = Components.classes["@mozilla.org/atom-service;1"].getService(Components.interfaces.nsIAtomService);
}

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
    dump("msiEditorSetTextProperty for "+editorElement.id+", property = "+property+", attribute = " + attribute + ", value = "+value+"\n");
    msiGetEditor(editorElement).setInlineProperty(propAtom, attribute, value);
    if (!msiCurrEditorSetFocus(window) && "gContentWindow" in window)
      window.gContentWindow.focus();
  }
  catch(e) {
    dump("Error is msiEditorSetTextProperty, e= "+e);
  }
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
  } catch (exc) {dump("Exception trying to query whether pref " + name + " is set: [" + exc + "].\n");}
  return false;
}

function SetBoolPref(name, value)
{
  try {
    return GetPrefs().setBoolPref(name, value);
  } catch (exc) {dump("Exception trying to set pref " + name + ": [" + exc + "].\n");}
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

//Returns an nsILocalFile
function GetLocalFilePref(name)
{
  try
  {
    return GetPrefs().getComplexValue(name, Components.interfaces.nsILocalFile);
  }
  catch(exc) {dump("Exception trying to get file pref " + name + ": [" + exc + "].\n");}
  return null;
}

//"theFile" is an nsILocalFile
function SetLocalFilePref(name, theFile)
{
  var prefs = GetPrefs();
  if (prefs)
  {
    try
    {
      prefs.setComplexValue(name, Components.interfaces.nsILocalFile, theFile);
    }
    catch (exc) {dump("Exception trying to set pref " + name + ": [" + exc + "].\n");}
  }
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

     
function getUntitledName(destinationDirectory)
{
  var untitled = "untitled";
  var f = destinationDirectory.clone();
  var ffile;
  var fdir;
  var returnval;
  var count = 1; 
  var maxcount = 100; // a maximum allowed for files named "untitledxx.sci"
  while (count < maxcount)
  {
    ffile = f.clone();
    fdir = f.clone();
    returnval = untitled+(count).toString();
    ffile.append(returnval+".sci");
    fdir.append(returnval+"_work");
    count++;
    if (!ffile.exists() && !fdir.exists()) 
    {
//      fdir.append("main.xhtml");
      return returnval;
    }
  }
  alert("too many files called 'untitledxx.sci' in directory "+destinationDirectory.path); // BBM: fix this up
  return "";
};
    

function installZipEntry(aZipReader, aZipEntry, aDestination) 
{
  var file = aDestination.clone();

  var dirs = aZipEntry.split(/\//);
  var isDirectory = /\/$/.test(aZipEntry);

  var end = dirs.length;
  if (!isDirectory)
    --end;

  for (var i = 0; i < end; ++i) {
    file.append(dirs[i]);
    if (!file.exists()) {
      file.create(1, 0755);
    }
  }

  if (!isDirectory) {
    file.append(dirs[end]);
    aZipReader.extract(aZipEntry, file);
  }
};


// Write a file to the zip file represented by aZipWriter. RelPath is the path relative to the
// directory we are zipping and sourceFile is the particular file being written.
// BBM - doesn't seem to be used??
 
function writeZipEntry(aZipWriter, relPath, sourceFile, compression) 
{
  var path = "";
  var dirs = relPath.split(/\//);
  var isDirectory = /\/$/.test(aZipEntry);
  if (compression === null) compression = 0;

  var end = dirs.length;
  if (!isDirectory)
    --end;

  for (var i = 0; i < end; ++i) {
    path = path+"/"+dirs[i];
    if (!aZipWriter.hasEntry(path)) {
      addEntryDirectory(path,0,false);
    }
  }

  if (!isDirectory) {
    path = path+"/"+dirs[end];
    aZipWriter.addEntryFile(path, compression, sourceFile, false); // should get compression preference here
  }
}

// zipDirectory is called recursively. The first call has currentpath="". sourceDirectory is the directory
// we are zipping, and currentpath is the path of sourceDirectory relative to the root directory.

function zipDirectory(aZipWriter, currentpath, sourceDirectory, compression)
{
  var e;
  var f;
  e = sourceDirectory.directoryEntries;
  if (compression === null) compression = 0;
  var more = e.hasMoreElements();
  while (more)
  {
    f = e.getNext().QueryInterface(Components.interfaces.nsIFile);
    var leaf = f.leafName;
    var path;
    if (currentpath.length > 0) path = currentpath + "/" + leaf;
    else path = leaf;
    if (f.isDirectory())
    {
      aZipWriter.addEntryDirectory(path, f.lastModifiedTime, false);
      zipDirectory(aZipWriter, path, f, compression);
    }
    else
    {
      if (aZipWriter.hasEntry(path))
        aZipWriter.removeEntry(path,true);
      aZipWriter.addEntryFile(path, compression, f, false);
    }
    more = e.hasMoreElements();
  }
}

// copyDirectory is called recursively. sourceDirectory is the directory
// we are copying, and destDirectory is the directory we are copyint to.
                         
function copyDirectory(destDirectory, sourceDirectory)
{
  var e;
  var f;
  var dest = destDirectory.clone();
  var dest2;
  if (!dest.exists())
  {
    dest.create(1,0755);
  }
  e = sourceDirectory.directoryEntries;
  while (e.hasMoreElements())
  {                       
    f = e.getNext().QueryInterface(Components.interfaces.nsIFile);
    var leaf;
    if (f) {
      leaf = f.leafName;
      dest2 = dest.clone();
      dest2.append(leaf);
  //    var path;
      if (f.isDirectory())
      {
  // skip temp directory
        if (leaf !== 'temp')
        {       
          if (!dest2.exists()) {
            dest2.create(1, 0755);
          }
          copyDirectory(dest2.clone(), f.clone());
        }
      }
      else
      {
        f.copyTo(destDirectory, null);
      }
    }
  }
}


function extractZipTree(aZipReader, destdirectory)
{
  var strIterator;
  strIterator = aZipReader.findEntries(null);
  while (strIterator.hasMore())
  {
    installZipEntry(aZipReader, strIterator.getNext(), destdirectory);
  }
}

// If the url of the document is something like ..../foo_work/main.xhtml, then
// we look at the parent directory's name to synthesize the original name .../foo.sci

function msiFindOriginalDocname(docUrlString)
{
  var path = unescape(docUrlString);
  var regEx = /\/main.xhtml$/i;  // BBM: localize this
  if (regEx.test(path))
  {
    var parentDirRegEx = /(.*\/)([A-Za-z0-9_ \b\-]*)_work\/main.xhtml$/i; //BBM: localize this
    var arr = parentDirRegEx.exec(path);
    if (arr && (arr.length > 2) && arr[2].length > 0)
      path = arr[1]+arr[2]+".sci";
  }
  return path;
}


// createWorkingDirectory does the following: 
// It creates a directory, foo_work, and unpacks the contents of foo.sci into the new directory.
// The return value is an nsILocalFile which is the main xhtml file in the .sci file, usually named
// main.xhtml.
// if the file parameter is something like foo.sci, it knows that the file is a jar file.
//
// If the file parameter is not a zip file, then it creates the directory and 
// copies, e.g., foo.xhtml to foo_work/main.xhtml. If the directory containing the file looks like a .sci
// directory (has the .sci extension, the file is called main.xhtml, or there are only one xhtml and zero 
// or more xml files in the directory, plus other files with junk extensions (such as.bak)) then it
// clones the drectory and its contents. It returns the nsILocalFile for foo_work/main.xhtml.
//

function createWorkingDirectory(documentfile)
{
  var i;
  var dir;
  var destfile;
  var str;
  var baseLeafName;
  var extension;
  var skipBackup;
  var theCase;
  var name;
  if (!(documentfile.isFile() || documentfile.isDirectory())) return null;
  var re = /(.*)(\.[a-zA-Z0-9]+$)/;
  var arr = re.exec(documentfile.leafName);
  if (arr.length > 2)
  {
    baseLeafName = arr[1];
    extension = arr[2];
  }
  // this is the name of the new directory, unless documentfile is a shell
  if (documentfile.isFile())
  {
    if (extension.toLowerCase() === ".sci")
    {
      // .sci document
      theCase = 1;
      extension = ".xhtml";
    }
    else
    {
      // other file, not .sci
      theCase = 2;
    }
  }
  else // must be a directory
  {
    if (regEx.test(documentfile.path))
    {
      theCase = 3; //.sci directory
    }
    else return null; // we don't handle directories unless they are named .sci
  }
  name = "main"+extension;
  dir = documentfile.parent.clone();
  dir.append(baseLeafName+"_work"); // build the working directory data; we will ignore this if a shell
  if (theCase === 2)
  {
    // we create a new directory
    dir.create(1 /*directory*/, 0755);
  }
  else if (theCase === 3)
  {
    documentfile.copyTo(documentfile.parent, dir.leafName);
  }
  if (theCase === 2)
  {
    var done = false;
    var index = 0;
    name = "main"+extension;
    while (!done)
    {
      dump("Trying name "+name+"\n");
      try
      {
        documentfile.copyTo(dir, name);  // That finishes this case
        done = true;
      }
      catch (e)
      {
        dump (e.message+"\n");
        name="main"+(index++).toString()+extension;
        done = false;        
      }
    }
  }
  if (theCase === 1)
  // remaining case is the main one, a .sci file
  {  
    var doc = documentfile.clone();
    extension = ".xhtml"
    var zr;
    zr = Components.classes["@mozilla.org/libjar/zip-reader;1"]
      .createInstance(Components.interfaces.nsIZipReader);
    var basename;
    try
    {
      if (isShell(doc.path))
      {
        // if we are opening a shell document, we create the working directory 
        // and change the document leaf name to "untitledxxx"
        dir =  msiDefaultNewDocDirectory();
        basename = getUntitledName(dir);
        dir.append(basename+"_work");
      }
      if (dir.exists())
      {
        var mainfile = dir.clone();
        mainfile.append("main"+extension);
        if (mainfile.exists())
        {
          var data = {value: false};
          var result = window.openDialog("chrome://prince/content/useWorkInProgress.xul", "workinprogress", "chrome,titlebar,resizable,modal", data);
          if (data.value)
          {
            return mainfile; 
          }
        }
        try
        {
          dir.remove(true);
        }
        catch(e)
        {
          AlertWithTitle("Error in removing directory", "This directory "+dir.leafName+" cannot be removed. Does some other application have one of its files open?");
          return null;
        }
      }
      dir.create(1, 0755);

      try {
        zr.open(documentfile);
      }
      catch(e)
      {
        AlertWithTitle("Unable to open zip file","The file "+documentfile.leafName+" cannot be opened");
        return null;
      }
      extractZipTree(zr, dir);
    }
    catch( e) {
      dump("Error in createWorkingDirectory: "+e.toString()+"\n");
      return null;
    } 
  }
  var newdocfile;
  newdocfile = dir.clone();
  newdocfile.append(name); 
  // We expect that the file is main.xhtml, but if the user opens an html file, it might be
  // .html or .htm or .shtml.
  if (!newdocfile.exists())
  {
    newdocfile = null;
    var fileenum = dir.directoryEntries;
    var file;
    while (fileenum.hasMoreElements())
    {
      file = fileenum.getNext();
      file.QueryInterface(Components.interfaces.nsIFile);      
      if ((/\.[xs]?html?/i).test(file.leafName)) 
      {
        newdocfile = file;
        break;
      }
    }
  }
  if (newdocfile)  newdocfile.permissions = 0644;
  return newdocfile;
}


// Returns an nsIFile directory designated as the default for new documents.
// The path is given by the swp.prefDocumentDir
function msiDefaultNewDocDirectory()
{
  var docdir;
  var prefs = GetPrefs();
  var docdirname;
  docdir = Components.classes["@mozilla.org/file/local;1"].
      createInstance(Components.interfaces.nsILocalFile);
  try {
    docdirname = prefs.getCharPref("swp.prefDocumentDir");
  } catch (e) {
    docdirname = null
  }
  if (docdirname)
  {
    try {
      docdir.initWithPath(docdirname);
      if (!docdir.exists())
        docdir.create(1, 0755);
      return docdir;
    }
    catch (e) {}
  }  
  var dirkey;
  var dsprops = Components.classes["@mozilla.org/file/directory_service;1"].getService(Components.interfaces.nsIProperties);
  var os = getOS(window);
  if (os==="win")
    dirkey = "Pers";
  else if (os==="osx")
    dirkey = "UsrDocs";
  else
    dirkey = "Home";
  // if we can't find the one in the prefs, get the default
  docdir = dsprops.get(dirkey, Components.interfaces.nsILocalFile);
  if (!docdir.exists()) docdir.create(1,0755);
  docdir.append(GetString("DefaultDocDir"));
  if (!docdir.exists()) docdir.create(1,0755);
  return docdir;
}


// function msiCopyFileAndDirectoryToBak( documentfile ) // an nsILocalFile
// {
//   try {
//   // new scheme: save the bak files with no name change in the ..._files/bak directory
//     var leafname = documentfile.leafName;
//     var i = leafname.lastIndexOf(".");
//     if (i > 0) leafname = leafname.substr(0,i);
//     var parentdir = documentfile.parent.clone();
//     var _filesdir = parentdir.clone();
//     _filesdir.append(leafname+"_files");
//     var tempbakdir = parentdir.clone();
//     tempbakdir.append("bak");
//     var finalbakdir = _filesdir.clone();
//     finalbakdir.append("bak");
//   // nuke current bak dir contents if there are any
//     if (finalbakdir.exists()) finalbakdir.remove(true);
//   // copy stuff to temp bak dir
//     documentfile.copyTo(tempbakdir, documentfile.leafName);
//   // copy _files to temp bak dir
//     _filesdir.copyTo(tempbakdir, leafname+"_files");
//   // We can copy the 'revert' directory to 'bak' or we can copy 'bak' to 'revert',
//   // but we can't do both without getting recursion. So delete the 'revert' directory
//   // that we just copied.
//     var revertdir = tempbakdir.clone();
//     revertdir.append(leafname+"_files");
//     revertdir.append("revert");
//     if (revertdir.exists()) revertdir.remove(true);
//     
//   // now move the temp dir to ..._files/bak
//     tempbakdir.moveTo(_filesdir, "bak"); 
//   }
//   catch (e) {
//     dump("msiCopyFileAndDirectoryToBak failed: "+e+"\n");
//   }
// }
// 

// This should be no longer needed
// function msiCopyAuxDirectory( originalfile, newfile ) // both nsILocalFiles
// {
//   try
//   {
//     var filename = originalfile.leafName;
//     var newfilename = newfile.leafName;
//     var i = filename.lastIndexOf(".");
//     if (i >= 0) filename = filename.substr(0,i);
//     i = newfilename.lastIndexOf(".");
//     if (i >= 0) newfilename = newfilename.substr(0,i);
//     filename += "_files";
//     newfilename += "_files";
//     var parentdir = originalfile.parent.clone();
//     var newdir = newfile.parent.clone();
//     var testfile = newdir.clone();
//     testfile.append(newfilename);
//     if (testfile.exists()) testfile.remove();
//     parentdir.append(filename);
//     parentdir.copyTo(newdir, newfilename);
//   }
//   catch(e) {
//     dump("unable to copy aux directory: "+e+"\n");
//   }
// }

// Recall that the current editing session has changed only files in the working directory, and
// that the .sci file has not been changed.
// 
// The File/Revert calls this function, but also reloads the .sci file. The pre-reverted
// file is saved.
// If the file was created from a shell file then del===true, and we delete the file. 
//
// The algorithm:
// Our file is currently main.xhtml in a directory we call D. Let A be the leafname of the .sci file
// If del===false, do a soft save, and save the directory D into a zipfile which we call A.undorevert.
// Delete D.
// If del===true, delete A.sci.
//
// The final state depends on del:
// If true, A.sci is gone as is the directory D. We leave A.bak if it exists, but it probably never does when del===true;
// If false, A.sci and A.bak are as they were before editing, A.undorevert is the state of the document just before the revert,
// Reloading the file A.sci will rebuild D

function msiRevertFile (aContinueEditing, documentfile, del) // an nsILocalFile
{
  try {
    var editorElement = msiGetActiveEditorElement();
    if (!msiIsTopLevelEditor(editorElement))
      return false;
    var editor = msiGetEditor(editorElement);

    var tempfile;
    var leafname;    
    var url = msiFileURLFromFile(documentfile);
    var docUrlString = msiFindOriginalDocname(url.spec);
    var leafregex = /.*\/([^\/\.]+)\.sci$/i;
    var arr = leafregex.exec(docUrlString);
    if (arr && arr.length >1) leafname = arr[1];

    var dir = documentfile.parent.clone();
    
    if (del)
    {
      tempfile = dir.clone();
      tempfile.append(leafname + ".sci");
      if (tempfile.exists()) tempfile.remove(0);
      dir.remove(1);
      return true;
    }
    else
    {
      var success =  msiSoftSave( editor, editorElement);
      if (!success) 
        throw Components.results.NS_ERROR_UNEXPECTED;

      var zipfile = dir.parent.clone();
      zipfile.append(leafname + ".undorevert"); // this is the file P.

    // zip D into the zipfile
      try {
        var compression = 0;
        var prefs = Components.classes["@mozilla.org/preferences-service;1"].getService(Components.interfaces.nsIPrefBranch);
        try
        {
          compression = prefs.getIntPref("swp.webzip.compression");
        }
        catch(e) {}
        var zw = Components.classes["@mozilla.org/zipwriter;1"]
                              .createInstance(Components.interfaces.nsIZipWriter);
        if (zipfile.exists()) zipfile.remove(0);
        zipfile.create(0,0755);
        zw.open( zipfile, PR_RDWR | PR_CREATE_FILE | PR_TRUNCATE);
        zipDirectory(zw, "", dir, compression); 
        zw.close();
      }
      catch(e) {
        throw Components.results.NS_ERROR_UNEXPECTED;
      } 
      dir.remove(1);
      return true;
    }
  }
  catch(e) {
    dump("msiRevertFile failed: "+e+"\n");
  }
  return false;
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
  return (urlString === "about:blank");
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
  if (baseScheme !== urlScheme)
    return inputUrl;

  var IOService = msiGetIOService();
  if (!IOService)
    return inputUrl;

  // Host must be the same
  var baseHost = GetHost(baseUrl);
  var urlHost = GetHost(inputUrl);
  if (baseHost !== urlHost)
    return inputUrl;


  // Get just the file path part of the urls
  var basePath = IOService.newURI(baseUrl, null, null).path;
  var urlPath = IOService.newURI(inputUrl, null, null).path;

  // We only return "urlPath", so we can convert
  //  the entire basePath for case-insensitive comparisons
  var os = getOS(window);
  var doCaseInsensitive = (os !== "win");
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

    if (nextUrlSlash === -1)
    {
      // We're done matching and all dirs in url
      // what's left is the filename
      done = true;

      // Remove filename for named anchors in the same file
      if (nextBaseSlash === -1 && baseFilename)
      { 
        var anchorIndex = urlPath.indexOf("#");
        if (anchorIndex > 0)
        {
          var urlFilename = doCaseInsensitive ? urlPath.toLowerCase() : urlPath;
        
          if (urlFilename.indexOf(baseFilename) === 0)
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

      if (urlDir === baseDir)
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
        if (firstDirTest && baseScheme === "file" && os !== "osx")
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
  if (urlspec.indexOf(":") < 1)
	  urlspec = "file://"+urlspec;

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

function GetFilepath(urlspec) // BBM: I believe this can be simplified
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
      {
        if (getOS(window)==="win")
          filepath = decodeURIComponent(url.path.substr(1));
        else
           filepath = decodeURIComponent(url.path);
      }
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
        if (usernameStart !== -1)
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
        if (colon !== -1)
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


function ConvertRGBColorIntoHEXColor(color)
{
  if ( /rgb\(\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*\)/.test(color) ) {
    var r = Number(RegExp.$1).toString(16);
    if (r.length === 1) r = "0"+r;
    var g = Number(RegExp.$2).toString(16);
    if (g.length === 1) g = "0"+g;
    var b = Number(RegExp.$3).toString(16);
    if (b.length === 1) b = "0"+b;
    return "#"+r+g+b;
  }
  else
  {
    return color;
  }
}

/************* CSS ***************/

function msiGetHTMLOrCSSStyleValue(editorElement, theElement, attrName, cssPropertyName, bPreferAttr)
{
  if (!theElement)
  {
    dump("In msiEditorUtilities.js, msiGetHTMLOrCSSStyleValue() called with null element!\n");
    return "";
  }  

  var element = theElement;
  var value;
  if (bPreferAttr)
  {
    value = element.getAttribute(attrName);
    if (value && value.length)
      return value;
  }

  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var prefs = GetPrefs();
  var IsCSSPrefChecked = prefs.getBoolPref("editor.use_css");
  var styleVal, match;
  var elemStyle = element.style;
  var regExp = new RegExp("^(?:.*;\\s*)*" + cssPropertyName + ":\\s*([^;]+)(;|$)");
  if (IsCSSPrefChecked && msiIsHTMLEditor(editorElement))
  {
    if (elemStyle)
      value = elemStyle.getPropertyValue(cssPropertyName);
    if (!value)
    {
      styleVal = element.getAttribute("style");
      if (styleVal)
        match = regExp.exec(styleVal);
      if (match && match.length > 1)
        value = match[1];
    }
  }
  if (!value && !bPreferAttr)
    value = element.getAttribute(attrName);

  if (!value)
  {
    element = msiNavigationUtils.findWrappingNode(element);
    if (element && (element !== theElement))
      value = msiGetHTMLOrCSSStyleValue(editorElement, element, attrName, cssPropertyName, bPreferAttr);
  }
  
  if (!value)
    return "";
  return value;
}

//This list (copied here from the Cascades code) represents the base HTML named colors.
var msiHTMLNamedColorsList = 
{  aqua : "#00ffff", black : "#000000", blue : "#0000ff", fuchsia : "#ff00ff",
   gray : "#808080", green : "#008000", lime : "#c0ff00", maroon : "#800000",
   navy : "#000080", olive : "#808000", purple : "#c00040", red : "#ff0000",
   silver : "#c0c0c0", teal : "#008080", white : "#ffffff", yellow : "#ffff00",
   cyan : "#00ffff", magenta : "#ff00ff", darkgray : "#404040", lightgray : "#c0c0c0",
   brown : "#c08040", orange : "#ff8000", pink : "#ffc0c0", violet : "#800080"
};


function textColorToHex(colorname)
{
	var name = TrimString(colorname).toLowerCase();
  var retStr = msiHTMLNamedColors.colorStringToHexRGBString(name);
  if (!retStr || !retStr.length)
    retStr = "#ffffff";
  return retStr;
}


function msiNamedColors(aNameSet)
{
  this.mColorNames = aNameSet;
  this.hexRegExp = /(#?)([0-9a-f]{6})/i;

  this.namedColorToHexRGBString = function(colorName)
  {
    if (colorName in this.mColorNames)
      return this.mColorNames[colorName];
    return "";
  };

  this.colorStringToHexRGBString = function(aColorDesc)
  {
    var hexStr = "";
    var targStr = ConvertRGBColorIntoHEXColor(aColorDesc);
    var theMatch = this.hexRegExp.exec(targStr);
    if (theMatch && theMatch[2])
      hexStr = "#" + theMatch[2].toLowerCase();
    else
      hexStr = this.namedColorToHexRGBString(targStr);
    return hexStr;
  };

  this.hexRGBStringToNamedColor = function(hexString)
  {
    var colorName = "";
    var theMatch = this.hexRegExp.exec(hexString);
    if (theMatch && theMatch[2])
    {
      var searchStr = "#" + theMatch[2].toLowerCase();
      for (var aName in this.mColorNames)
      {
        if (this.mColorNames[aName] === searchStr)
        {
          colorName = aName;
          break;
        }
      }
    }
    return colorName;
  };
}

var msiHTMLNamedColors = new msiNamedColors(msiHTMLNamedColorsList);

var msiCSSUtils = 
{
  getLengthWithUnitsStringFromCSSPrimitiveValue : function(cssValue)
  {
    var lengthAndUnits = this.getLengthWithUnitsFromCSSPrimitiveValue(cssValue);
    return lengthAndUnits.mString;
  },

  getLengthWithUnitsFromCSSPrimitiveValue : function(cssValue)
  {
    var retValue = {number : 0.0, unit : "", mString : ""};
    switch(cssValue.primitiveType)
    {
      case nsICSSPrimitive.CSS_CM:
        if (!theUnitStr.length)
          retValue.unit = "cm";
      case nsICSSPrimitive.CSS_EMS:
        if (!retValue.unit.length)
          retValue.unit = "em";
      case nsICSSPrimitive.CSS_EXS:
        if (!retValue.unit.length)
          retValue.unit = "ex";
      case nsICSSPrimitive.CSS_IN:
        if (!retValue.unit.length)
          retValue.unit = "in";
      case nsICSSPrimitive.CSS_MM:
        if (!retValue.unit.length)
          retValue.unit = "mm";
      case nsICSSPrimitive.CSS_PC:
        if (!retValue.unit.length)
          retValue.unit = "pc";
      case nsICSSPrimitive.CSS_PT:
        if (!retValue.unit.length)
          retValue.unit = "pt";
      case nsICSSPrimitive.CSS_PX:
        if (!retValue.unit.length)
          retValue.unit = "px";
      case nsICSSPrimitive.CSS_PERCENTAGE:
        if (!retValue.unit.length)
          retValue.unit = "%";
        retValue.number = cssValue.getFloatValue(cssValue.primitiveType);
        retValue.mString = String(retValue.number) + retValue.unit;
      break;
      case nsICSSPrimitive.CSS_DIMENSION:
      case nsICSSPrimitive.CSS_NUMBER:
        retValue.number = cssValue.getFloatValue(cssValue.primitiveType);
        retValue.mString = String(retValue.number);
      break;
      case nsICSSPrimitive.CSS_STRING:
      case nsICSSPrimitive.CSS_IDENT:
      case nsICSSPrimitive.CSS_ATTR:
        retValue.mString = cssValue.getStringValue();
      break;
      default:
      break;
    }
    return retValue;
  },

  getRGBColorValFromCSSPrimitive : function(cssValue)
  {
    var retStr = "";
    switch(cssValue.primitiveType)
    {
      case nsICSSPrimitive.CSS_RGBCOLOR:
//        retStr = String(cssValue.getRGBColorValue());
        var theColor = cssValue.getRGBColorValue();
        var r = theColor.red.getFloatValue(nsICSSPrimitive.CSS_NUMBER).toString(16);
        if (r.length === 1) r = "0"+r;
        var g = theColor.green.getFloatValue(nsICSSPrimitive.CSS_NUMBER).toString(16);
        if (g.length === 1) g = "0"+g;
        var b = theColor.blue.getFloatValue(nsICSSPrimitive.CSS_NUMBER).toString(16);
        if (b.length === 1) b = "0"+b;
        retStr = "#"+r+g+b;
      break;
      case nsICSSPrimitive.CSS_STRING:
      case nsICSSPrimitive.CSS_IDENT:
      case nsICSSPrimitive.CSS_ATTR:
        retStr = cssValue.getStringValue();
        if ( (retStr !== "transparent") && (retStr !== "inherit") )
          retStr = msiHTMLNamedColors.colorStringToHexRGBString(retStr);
      break;
      default:
      break;
    }
    return retStr;
  },

  subtractCSSLengthValues : function(strVal1, strVal2, contextNode, outUnit, outPrecision)
  {
    var ourMsiList;
    if (contextNode)
      ourMsiList = msiCreateCSSUnitsListForElement(contextNode);
    else
      ourMsiList = new msiCSSWithFontUnitsList(12, "pt");
    var unitsToUse = val1.units;
    return ourMsiList.subtractUnitStrings(strVal1, strVal2, outUnit, outPrecision);
  },

  addCSSLengthValues : function(strVal1, strVal2, contextNode, outUnit, outPrecision)
  {
    var ourMsiList;
    if (contextNode)
      ourMsiList = msiCreateCSSUnitsListForElement(contextNode);
    else
      ourMsiList = new msiCSSWithFontUnitsList(12, "pt");
    var unitsToUse = val1.units;
    return ourMsiList.addUnitStrings(strVal1, strVal2, outUnit, outPrecision);
  },

  getCSSComputedLengthValue : function(anElement, aProperty, outUnit, outPrecision)
  {
    var defView = anElement.ownerDocument.defaultView;
    var docCSS = defView.QueryInterface(Components.interfaces.nsIDOMViewCSS);
    var theStyle = docCSS.getComputedStyle(anElement, "");
    var theCSSVal = theStyle.getPropertyCSSValue("font-size");
    var numWithUnits = this.getLengthWithUnitsFromCSSPrimitiveValue(theCSSVal);
    if (numWithUnits && numWithUnits.unit && numWithUnits.number)
    {
      if (!outUnit)
        outUnit = numWithUnits.unit;
      return (Number(numWithUnits.number).toPrecision(outPrecision) + outUnit);
    }
    return "";
  }
};

//CSS stylesheet editing functions

//Call the following function by code like:
//    valueArray = [];
//    for (i = 0; i < ourProperties.length; ++i)
//      valueArray.push( new cssPropertyChangeRecord(ourProperties[i], ourValues[i], bImportant[i]) );
//    changeCSSPropertiesForSelector(document, "theorem,corollary", valueArray);
function changeCSSPropertiesForSelector(aDoc, selectorText, propValArray)
{
  var cssMan = getCSSChangeManager(aDoc);
  return cssMan.changePropertiesForSelector(selectorText, propValArray);
}

//Call the following function by code like:
//    thmAndCorArray = [];
//    thmAndCorLabelArray = [];
//    for (i = 0; i < thmAndCorProperties.length; ++i)
//      thmAndCorArray.push( new cssPropertyChangeRecord(thmAndCorProperties[i], thmAndCorValues[i], thmAndCorImportant[i]) );
//    for (i = 0; i < thmAndCorLabelProperties.length; ++i)
//      thmAndCorLabelArray.push( new cssPropertyChangeRecord(thmAndCorLabelProperties[i], thmAndCorLabelValues[i], thmAndCorLabelImportant[i]) );
//    changeCSSPropertiesForSelector(document, [["theorem,corollary", thmAndCorArray], 
//                                             ["theorem > *:first-child:before, corollary > *:first-child:before", thmAndCorLabelArray]] );
//This has the advantage of making all the changes at once (hopefully).
function changeCSSPropertiesForSelectorPropertiesArray(aDoc, theArray)
{
  var cssMan = getCSSChangeManager(aDoc);
  return cssMan.changeArrayOfProperties(theArray);
}

var cssPropertyChangeRecordBase = 
{
  notePropertyIsSet : function(aSelectorObj, aRuleSetObj, matchedSelector, valueStr, priorityStr, anIndex)
  {
    if (!this.valuesTable)
      this.valuesTable = new Object();
    if (!(aSelectorObj.m_selString in this.valuesTable))
      this.valuesTable[aSelectorObj.m_selString] = [];
    var bImportant = (priorityStr && priorityStr === "important");
    var jj;
    var bInserted = false;
    if (bImportant)
    {
      for (jj = 0; jj < this.valuesTable[aSelectorObj.m_selString].length; ++jj)
      {
        if (!this.valuesTable[aSelectorObj.m_selString][jj].m_bImportant)
        {
          this.valuesTable[aSelectorObj.m_selString].splice( jj, 0, {m_ruleSetObj : aRuleSetObj, m_val : valueStr, m_bImportant : true, m_index : anIndex, m_matchedSelector : matchedSelector} );
          bInserted = true;
        }
      }
    }
    if (!bInserted)
      this.valuesTable[aSelectorObj.m_selString].push( {m_ruleSetObj : aRuleSetObj, m_val : valueStr, m_bImportant : bImportant, m_index : anIndex, m_matchedSelector : matchedSelector} );
  },

  actionRequired : function(changesRequired, aSelectorObj)
  {
    var addRule = {theSelector : aSelectorObj, theProperty : this.m_property, theValue : this.m_value, m_bImportant : this.m_bImportant};
    if (!this.valuesTable || !this.valuesTable[aSelectorObj.m_selString] || !this.valuesTable[aSelectorObj.m_selString].length)
    {
      changesRequired.addProperties.push(addRule);
      return;
    }

    var currRecord = null;
    var bNeedImportant = this.m_bImportant;
    var deleteRecords = [];
    var firstMatching, changeRecord;
    var priorityStr = "";

    function selectorMatchIsExact(aValueRecord)
    {
//      return (aValueRecord.m_matchedSelector && (aSelectorObj.m_selString === aValueRecord.m_matchedSelector.m_selString));
      return (aValueRecord.m_matchedSelector && aSelectorObj.matchesExact(aValueRecord.m_matchedSelector));
    }
    //Algorithm: we proceed "down the cascade" hitting the "important" rules first and then those in the modifiable sheet.
    //  We stop when we encounter the first rule which isn't one of the above unless it matches the required value, in which
    //  case, if there were no important rules from non-modifiable stylesheets encountered, we only need to delete the rules
    //  we passed first in the modifiable sheet.
    for (var ii = 0; ii < this.valuesTable[aSelectorObj.m_selString].length; ++ii)
    {
      currRecord = this.valuesTable[aSelectorObj.m_selString][ii];
      if ((currRecord.m_val === this.m_value) && (!bNeedImportant || currRecord.m_bImportant))
        firstMatching = currRecord;
      else if (this.recordIsModifiable(currRecord) && selectorMatchIsExact(currRecord))
      {
        if (!changeRecord && !firstMatching)
          changeRecord = currRecord;
        else
          deleteRecords.push(currRecord);
      }
      else if (currRecord.m_bImportant)
        bNeedImportant = true;
      else
        break;
    }
    if (bNeedImportant)
      priorityStr = "important";
    if (changeRecord && firstMatching)
    {
      deleteRecords.push(changeRecord);
      changeRecord = null;
    }
    if (changeRecord)  //we found a modifiable rule but it has the wrong attributes.
    {
      this.addChangeToChangesList( {theSelector : aSelectorObj, theProperty : this.m_property, theValue : this.m_value, thePriority : priorityStr, theRule : changeRecord.m_ruleSetObj, theMatchedSelector : changeRecord.m_matchedSelector}, changesRequired.changeProperties );
    }
    else if (!firstMatching && addRule)
    {
      addRule.m_bImportant = bNeedImportant;
      this.addRecordToAdditionsList( addRule, changesRequired.addProperties );
    }
    for (ii = 0; ii < deleteRecords.length; ++ii)
    {
      this.addChangeToChangesList( {theSelector : aSelectorObj, theProperty : this.m_property, theValue : "", thePriority : "", theRule : deleteRecords[ii].m_ruleSetObj, theMatchedSelector : deleteRecords[ii].m_matchedSelector}, changesRequired.changeProperties );
    }
  },

  recordIsModifiable : function(aRecord)
  {
    return (aRecord.m_index < 0);
  },

  addChangeToChangesList : function( aRecord, aList )
  {
    var theRuleSet = aRecord.theRule;
    var bFound = false;
    for (var ix = 0; ix < aList.length; ++ix)
    {
      if (aList[ix].theRule === aRecord.theRule)
        bFound = true;
      else if (bFound)
        aList.splice(ix, 0, aRecord);
    }
    if (!bFound)
      aList.push(aRecord);
  },

  addRecordToAdditionsList : function( aRecord, aList )
  {
    var foundSelector = null;
    var nInsertPos = -1;
    for (var ix = 0; (nInsertPos < 0) && (ix < aList.length); ++ix)
    {
      if (foundSelector && (aList[ix].theSelector !== aRecord.theSelector))
        nInsertPos = ix;
      else if (aList[ix].theSelector === aRecord.theSelector)
        foundSelector = aList[ix];
    }
    if (nInsertPos < 0)
      aList.push( aRecord );
    else
      aList.splice(nInsertPos, 0, aRecord);
  }
};

function cssPropertyChangeRecord(propName, newValue, bImportant)
{
  this.m_property = propName;
  this.m_value = newValue;
  this.m_bImportant = bImportant ? true : false;
}

cssPropertyChangeRecord.prototype = cssPropertyChangeRecordBase;

var cssChangesManagerBase = 
{
  m_replaceCharsRE : /[\*\.\$\^\|\[\]\(\)\{\}\+\?]/g,
  m_deletionMarker : "del----",
  m_document : null,
  m_modifiableStyleSheet : null,  
  //this is set up to have one target stylesheet associated with the document which we'll modify
  m_styleSheetArray : null,
  m_bStyleSheetModified : false,

  //These functions expect the propChangesArrays to be arrays of cssPropertyChangeRecords...
  changePropertiesForSelector : function(aSelectorStr, propChangesArray)
  {
    return this.changeArrayOfProperties( [[aSelectorStr, propChangesArray]] );
  },

  //And here the array should contains [<selector strings>, <propChangesArray>] pairs.
  changeArrayOfProperties : function( selectorAndPropsArray )
  {
    var ix, jx, kx;
    var styleSheets = this.getStyleSheetArray();
    var aSelectorStr, propChangesArray;
    var selectorObjArray;
    var currRuleSetObj;
    var nInsertPos;

    var changesRequired = {addProperties : [], changeProperties : []};
    for (ix = 0; ix < selectorAndPropsArray.length; ++ix)
    {
      if ( !("length" in selectorAndPropsArray[ix]) || (selectorAndPropsArray[ix].length < 2) )
      {
        dump("Bad selector and properties pair passed to cssChangesManager.changeArrayOfProperties, at index [" + ix + "].\n");
        continue;
      }
      aSelectorStr = selectorAndPropsArray[ix][0];
      propChangesArray = selectorAndPropsArray[ix][1];
      selectorObjArray = parseCSSSelectorString(aSelectorStr);
      this.checkPropertyChangeArray(this.m_modifiableStyleSheet, selectorObjArray, propChangesArray, -1);
      for (jx = 0; jx < styleSheets.length; ++jx)
      {
        this.checkPropertyChangeArray(styleSheets[jx], selectorObjArray, propChangesArray, jx);
      }
      for (jx = 0; jx < selectorObjArray.length; ++jx)
      {
        for (kx = 0; kx < propChangesArray.length; ++kx)
        {
          propChangesArray[kx].actionRequired(changesRequired, selectorObjArray[jx]);
        }
      }
    }

    //At this point, all the changes needed should be recorded in changesRequired.
    //Now make the changes! and mark the style sheet modified
    //Note that the changeProperties list is ordered by rule set, so we collect them and edit each rule set as we go.
    var ruleSetChangeArray = [];
    var ruleChanges = [];
    var ruleAdditions = [];
    var currSelector = null;
    var currRuleChange;
    var ruleText = "";
    ix = 0; 
    while (ix < changesRequired.addProperties.length)
    {
      currSelector = changesRequired.addProperties[ix].theSelector;
      ruleText = currSelector.m_selString + " {\n";
      while ( (ix < changesRequired.addProperties.length) && (changesRequired.addProperties[ix].theSelector === currSelector) )
      {
        ruleText += "  " + changesRequired.addProperties[ix].theProperty + ": " + changesRequired.addProperties[ix].theValue;
      if (changesRequired.addProperties[ix].m_bImportant)
        ruleText += " !important;\n"
      else
        ruleText += ";\n";
      ++ix;
      }
      ruleText += "}";
      ruleAdditions.push( {m_rule : ruleText, m_insertAfterRule : null} );
    }
    for (ix = 0; ix < changesRequired.changeProperties.length; ++ix)
    {
      if (changesRequired.changeProperties[ix].theRule === currRuleSetObj)
        ruleSetChangeArray.push(changesRequired.changeProperties[ix]);
      else
      {
        if (currRuleSetObj)
          this.prepareEditRuleSet(ruleSetChangeArray, ruleChanges, ruleAdditions);
        currRuleSetObj = changesRequired.changeProperties[ix].theRule;
        ruleSetChangeArray = [changesRequired.changeProperties[ix]];
      }
    }
    if (currRuleSetObj)   //get the last one too
      this.prepareEditRuleSet(ruleSetChangeArray, ruleChanges, ruleAdditions);

    //Finally actually make the changes to the style sheet object.
    //Do the additions first in case we delete somebody's m_insertAfterRule
    for (ix = 0; ix < ruleAdditions.length; ++ix)
    {
      if (ruleAdditions[ix].m_insertAfterRule && ruleAdditions[ix].m_insertAfterRule)
        nInsertPos = this.getIndexOfRuleSet(ruleAdditions[ix].m_insertAfterRule.m_cssRule) + 1;
      else
        nInsertPos = this.m_modifiableStyleSheet.cssRules.length;
      this.m_modifiableStyleSheet.insertRule(ruleAdditions[ix].m_rule, nInsertPos);
    }
    for (ix = 0; ix < ruleChanges.length; ++ix)
    {
      currRuleSetObj = ruleChanges[ix].m_ruleSet;
      if (ruleChanges[ix].m_changedSelectorStr)
        currRuleSetObj.m_cssRule.selectorText = ruleChanges[ix].m_changedSelectorStr;
      for (jx = 0; jx < ruleChanges[ix].m_changes.length; ++jx)
      {
        currRuleChange = ruleChanges[ix].m_changes[jx];
        if (currRuleChange.m_value === this.m_deletionMarker)
          currRuleSetObj.m_cssRule.style.removeProperty(currRuleChange.m_property);
        else
        {
          currRuleSetObj.m_cssRule.style.setProperty(currRuleChange.m_property, currRuleChange.m_value, currRuleChange.m_priority);
//          if (currRuleChange.m_priority)
//            currRuleSetObj.m_cssRule.style.setPropertyPriority(currRuleChange.m_property, currRuleChange.m_priority);
        }
      }
    }
    //Check for rule sets which have been emptied
    for (ix = 0; ix < ruleChanges.length; ++ix)
    {
      if (ruleChanges[ix].m_ruleSet.m_cssRule.style.length === 0)
      {
        nInsertPos = this.getIndexOfRuleSet(ruleChanges[ix].m_ruleSet.m_cssRule);
        if (nInsertPos >= 0)
          this.m_modifiableStyleSheet.deleteRule(nInsertPos);
      }
    }

    if (ruleChanges.length || ruleAdditions.length)
      this.m_bStyleSheetModified = true;
    
  },

  writeCSSFile : function()  //These should come in as a nsIFile and a string respectively
  {
    if (!this.m_modifiableStyleSheet)
    {
      dump("cssChangesManager.writeCSSFile called for document [" + this.m_document.path + "] with no modifiable style sheet!\n");
      return false;
    }
    if (!this.m_bStyleSheetModified)
      return true;  //don't write it if not modified>
    //Now write it out
    var rv = true;
    try
    {
      var stylePath = this.m_modifiableStyleSheet.href;
      var styleUrl = msiURIFromString(stylePath);
      var styleNSFile = msiFileFromFileURL(styleUrl);

      var fos = Components.classes["@mozilla.org/network/file-output-stream;1"].createInstance(Components.interfaces.nsIFileOutputStream);
      fos.init(styleNSFile, -1, -1, false);
      var os = Components.classes["@mozilla.org/intl/converter-output-stream;1"].createInstance(Components.interfaces.nsIConverterOutputStream);
      os.init(fos, "UTF-8", 4096, "?".charCodeAt(0));
      for (var ii = 0; ii < this.m_modifiableStyleSheet.cssRules.length; ++ii)
        os.writeString(this.m_modifiableStyleSheet.cssRules[ii].cssText + "\n\n");
      os.close();
      fos.close();
    }
    catch(ex) 
    {
      dump("Exception in writing out CSS file [" + this.m_modifiableStyleSheet.href + "]; exception is [" + ex + "].\n");
      rv = false;
    }
    return rv;
  },

  checkPropertyChangeArray : function(aStyleSheet, selectorObjArray, propChangeArray, anIndex)
  {
    var testStr = this.getSearchableStringFromSelectorArray(selectorObjArray);
    var testStrRE = new RegExp(testStr);
    var testRuleSet = null;
    for (var jx = aStyleSheet.cssRules.length - 1; jx >= 0; --jx)
    {
      if (aStyleSheet.cssRules[jx].type !== CSSRule.STYLE_RULE)
        continue;
      if (testStrRE.test(aStyleSheet.cssRules[jx].selectorText))
      {
        testRuleSet = cssRuleSetToRuleSetObj(aStyleSheet.cssRules[jx]);
        for (var ix = 0; ix < selectorObjArray.length; ++ix)
          this.checkRuleSetForProperties( selectorObjArray[ix], propChangeArray, testRuleSet, anIndex );
      }  
    }
  },

  checkRuleSetForProperties : function( selectorObj, propChangeArray, aRuleSetObj, anIndex )
  {
    var jj, valStr, aPriority, nMatched;
    var nMatched = selectorObj.matchesOneOf(aRuleSetObj.m_selectors);
    if (nMatched >= 0)
    {
      for (var jj = 0; jj < propChangeArray.length; ++jj)
      {
        valStr = aRuleSetObj.m_cssRule.style.getPropertyValue(propChangeArray[jj].m_property);
        if (valStr && valStr.length)
        {
          aPriority = aRuleSetObj.m_cssRule.style.getPropertyPriority(propChangeArray[jj].m_property);
          propChangeArray[jj].notePropertyIsSet(selectorObj, aRuleSetObj, aRuleSetObj.m_selectors[nMatched], valStr, aPriority, anIndex);
        }
      }
    }
  },

  prepareEditRuleSet : function(aRuleSetChangeArray, ruleChanges, ruleAdditions)
  {
    if (!aRuleSetChangeArray.length)
      return;
//    var recordsDone = [];
//    var changedSelectors = [];
    var theRuleSetObj = aRuleSetChangeArray[0].theRule;
    var addRules = [];
    var changeProps = [];

    var allSelectors = theRuleSetObj.m_selectors;
    var aProperty, aValue, ix, jx, matchedSelector;
    var propValueTable = new Object();
//    var changesBySelector = new Object();
    var exceptions = [];
    var nMatchedSelector = -1;
    var globalChangedSelectors = [];
    var unchangedProps = [];
    //Organize into a two-dimensional array by property and value, noting which selectors change the value
    //The plan is to change the value in this ruleset whenever possible (whenever all selectors agree on it).
    //  When some selectors don't agree, they'll have to be separated out and combined in one or more new rulesets.
    //We look at each property affected to see whether all the selectors agree, and separating those which don't. However,
    //  we should recall that only one new value is generally specified for each property, so this should come down to a
    //  question of just which selectors are being changed.
    for (ix = 0; ix < aRuleSetChangeArray.length; ++ix)
    {
      aProperty = aRuleSetChangeArray[ix].theProperty;
      aValue = aRuleSetChangeArray[ix].theValue;
      if (aRuleSetChangeArray[ix].thePriority.length)
        aValue += " !" + aRuleSetChangeArray[ix].thePriority;  //should just be "!important"
      if (!aValue || !aValue.length)
        aValue = this.m_deletionMarker;
      if (!(aProperty in propValueTable))
        propValueTable[aProperty] = {changedSelectors : [], values : {}};
      if (!(aValue in propValueTable[aProperty].values))
        propValueTable[aProperty].values[aValue] = [];
      nMatchedSelector = allSelectors.indexOf(aRuleSetChangeArray[ix].theMatchedSelector);
      if (nMatchedSelector >= 0)
      {
        propValueTable[aProperty].changedSelectors.push(nMatchedSelector);
        globalChangedSelectors.push(nMatchedSelector);
        if (propValueTable[aProperty].values[aValue].indexOf(nMatchedSelector) < 0)
          propValueTable[aProperty].values[aValue].push(nMatchedSelector);
      }
//      if (!(matchedSelector.m_selString in changesBySelector))
//        changesBySelectors[matchedSelector.m_selString] = [];
//      changesBySelectors[matchedSelector.m_selString].push(ix);
    }
//    for (ix = 0; ix < theRuleSetObj.m_cssRule.style.length; ++ix)
//    {
//      aProperty = theRuleSetObj.m_cssRule.style.item(ix);
//      if ( !(aProperty in propValueTable) )
//        unchangedProps.push(aProperty);
//    }

    var nSelectors = allSelectors.length;
    var removeSelectors = [];
    var keepInRule = [];
    var valueToUse, priorityStr;
    var nMaxCount, nSplitPos;

    var minSelectors = Math.max(2, nSelectors - globalChangedSelectors);
    if (minSelectors > nSelectors)
      minSelectors = nSelectors;
    if ((2 * globalChangedSelectors.length) < nSelectors)  //so most of the selectors aren't affected - we leave this rule set alone and pull out what need changing
    {
      removeSelectors = removeSelectors.concat(globalChangedSelectors);
    }
    else  //most selectors are affected - try to just reset the properties when possible, and remove the unaffected selectors
    {
      for (jx = 0; jx < nSelectors; ++jx)
      {
        if (globalChangedSelectors.indexOf(jx) < 0)
          removeSelectors.push(jx);
      }
      keepInRule = keepInRule.concat(globalChangedSelectors);  //start with just copying the globalChangeSelectors
      for (aProperty in propValueTable)
      {
        nMaxCount = 0;
        valueToUse = "";
        for (aValue in propValueTable[aProperty].values)
        {
          if (propValueTable[aProperty].values[aValue].length > nMaxCount)
          {
            nMaxCount = propValueTable[aProperty].values[aValue].length;
            valueToUse = aValue;
          }
        }
        if (nMaxCount)
          keepInRule = intersectArrayWith(keepInRule, propValueTable[aProperty].values[valueToUse]);
        if (keepInRule.length < minSelectors)  //Give up on changing the rules and just keep the selectors with no changes
        {
          removeSelectors = removeSelectors.concat(globalChangedSelectors);
          changeProps = [];
          break;
        }
        else
        {
          for (jx = 0; jx < nSelectors; ++jx)
          {
            if ( (keepInRule.indexOf(jx) < 0) && (removeSelectors.indexOf(jx) < 0) )
              removeSelectors.push(jx);
          }
          //prepare the change record
          nSplitPos = valueToUse.lastIndexOf("!");
          if (nSplitPos >= 0)
          {
            priorityStr = TrimString(valueToUse.substr(nSplitPos));
            valueToUse = TrimString(valueToUse.substr(0, nSplitPos));
          }
          else
            priorityStr = "";
          changeProps.push( {m_property : aProperty, m_value : valueToUse, m_priority : priorityStr} );
        }
      }
    }

    //The end should be:
    var changedSelectorStr = null;
    if (removeSelectors.length)
    {
      changedSelectorStr = "";    //the value of null would prevent the use of changedSelectorStr below; if it ends up empty, the rule set will eventually be deleted
      for (ix = 0; ix < removeSelectors.length; ++ix)
      {
        ruleText = "{\n";
        for (aProperty in propValueTable)
        {
          if (propValueTable[aProperty].changedSelectors.indexOf(removeSelectors[ix]) >= 0)
          {
            for (aValue in propValueTable[aProperty].values)
            {
              if (propValueTable[aProperty].values[aValue].indexOf(removeSelectors[ix]) >= 0)
              {
                if (aValue !== this.m_deletionMarker)
                  ruleText += "  " + aProperty + ":  " + aValue + ";\n";
                break;
              }
            }
          }
          else
          {
            ruleText += "  " + theRuleSetObj.m_cssRule.style.getPropertyValue(aProp);
            aPriority = theRuleSetObj.m_cssRule.style.getPropertyPriority(aProp);
            if (aPriority.length)
              ruleText += " !" + aPriority + ";\n";
            else
              ruleText += ";\n";
          }
        }
        ruleText += "\n}";
        ruleAdditions.push( {m_selector : allSelectors[removeSelectors[ix]].m_selString, m_rule : ruleText, m_insertAfterRule : theRuleSetObj.m_cssRule} );
      }
    }
    var delimStr = "";
    if (removeSelectors.length)
    {
      for (ix = 0; ix < allSelectors.length; ++ix)
      {
        if ( removeSelectors.indexOf(ix) < 0 )
        {
          changedSelectorStr += delimStr + allSelectors[ix].m_selString;
          delimStr = ", ";
        }
      }
    }
    
    ruleChanges.push( {m_ruleSet : theRuleSetObj, m_changedSelectorStr : changedSelectorStr, m_changes : changeProps} );
  },

  getStyleSheetArray : function() //We reverse the usual array so as to read the cascade in reverse later
  {
    if (!this.m_styleSheetArray)
    {
      this.m_styleSheetArray = new Array();
      for (var ix = this.m_document.styleSheets.length - 1; ix >= 0; --ix)
      {
        if (this.isModifiableStyleSheet(this.m_document.styleSheets[ix]))
          this.m_modifiableStyleSheet = this.m_document.styleSheets[ix];
        else
          this.m_styleSheetArray.push(this.m_document.styleSheets[ix]);
        this.m_styleSheetArray = this.m_styleSheetArray.concat(this.getIncludedStyleSheets(this.m_document.styleSheets[ix]));
      }
    }
    return this.m_styleSheetArray;
  },

  getSearchableStringFromSelectorArray : function(selectorObjArray)
  {
//    var regexpStr = "/(" + this.getSearchableStringFromSelector(selectorObjArray[0]) + ")";
    var regexpStr = this.getSearchableStringFromSelector(selectorObjArray[0]);
    for (var ii = 1; ii < selectorObjArray.length; ++ii)
    {
//      regexpStr += "|(" + this.getSearchableStringFromSelector(selectorObjArray[ii]) + ")";
      regexpStr += "|" + this.getSearchableStringFromSelector(selectorObjArray[ii]);
    }
//    regexpStr += "/";
    return regexpStr;
  },

  getSearchableStringFromSelector : function(selectorObj)
  {
    var regexpStr = "(" + selectorObj.m_simpleSelectors[0].m_elemSelector.replace(this.m_replaceCharsRE, "\\$1") + ")";
    for (var ii = 1; ii < selectorObj.m_simpleSelectors.length; ++ii)
    {
      if (selectorObj.m_simpleSelectors[ii].m_elemSelector && (selectorObj.m_simpleSelectors[ii].m_elemSelector.length > 1) )
        regexpStr += "|(" + selectorObj.m_simpleSelectors[ii].m_elemSelector.replace(this.m_replaceCharsRE, "\\$1") + ")";
    }
    return regexpStr;
  },

  getIndexOfRuleSet : function(aRuleSet)
  {
    var theStyleSheet = aRuleSet.parentStyleSheet;
    if (!theStyleSheet)
      return -1;
    for (var ix = 0; ix < theStyleSheet.cssRules.length; ++ix)
    {
      if (theStyleSheet.cssRules.item(ix) === aRuleSet)
        return ix;
    }
    return -1;
  },

  isModifiableStyleSheet : function(aStyleSheet)
  {
    var cssUrl, styleNSFile;
    var uriStr = aStyleSheet.href;
    try
    {
      cssUrl = msiURIFromString(uriStr);
      styleNSFile = msiFileFromFileURL(cssUrl);
    }
    catch(exc) {dump("In cssChangesManager.isModifiableStyleSheet, exception: " + exc + ".\n");}
    //This is stupid! Improve it...
    if (styleNSFile && (styleNSFile.leafName === "my.css"))
      return true;
    if (aStyleSheet.ownerRule === null)
      return true;  //is this better?
    return false;
  },

  getIncludedStyleSheets : function(aStyleSheet)
  {
    var styleSheetArray = [];
    var impRule;
    for (var ii = aStyleSheet.cssRules.length - 1; ii >= 0 ; --ii)
    {
      if (aStyleSheet.cssRules[ii].type === CSSRule.IMPORT_RULE)
      {
        impRule = aStyleSheet.cssRules[ii];
        if ( ("styleSheet" in impRule) && (impRule.styleSheet !== null) )
        {
          styleSheetArray.push(impRule.styleSheet);
          styleSheetArray = styleSheetArray.concat(this.getIncludedStyleSheets(impRule.styleSheet));
        }
      }
    }
    return styleSheetArray;
  }

};

function cssChangesManager(aDocument)
{
  this.m_document = aDocument;
  this.m_modifiableStyleSheet = null;
  this.m_styleSheetArray = null;
  this.m_bStyleSheetModified = false;
}

cssChangesManager.prototype = cssChangesManagerBase;

var cssEditingManagers = [];

function findCSSChangeManager(aDocument)
{
  for (var ix = 0; ix < cssEditingManagers.length; ++ix)
  {
    if (cssEditingManagers[ix].m_document === aDocument)
    return cssEditingManagers[ix];
  }
  return null;
}

function getCSSChangeManager(aDocument)
{
  var cssMan = findCSSChangeManager(aDocument);
  if (!cssMan)
  {
    cssMan = new cssChangesManager(aDocument);
    cssEditingManagers.push( cssMan );
  }
  return cssMan;
}

function saveCSSFileIfChanged(aDocument)
{
  var cssMan = findCSSChangeManager(aDocument);
  if (cssMan)
    cssMan.writeCSSFile();
}

var cssRuleSetBase = 
{
  m_cssRule : null,  //a cssRule object - provides access to the individual property settings
  m_selectors : [],  //an array of cssSelectors (see below)
//  m_properties : [],      //an array containing the string representations of individual properties

  initObj : function()
  {
    if (!this.m_cssRule)
    {
      return;
    }
    if (this.m_cssRule.type !== CSSRule.STYLE_RULE)
    {
      dump("cssRuleSetBase.initObj() called with a CSSRule of a type other than STYLE_RULE. Aborting...\n");
      return;
    }
    this.m_selectors = parseCSSSelectorString(this.m_cssRule.selectorText);
  }
};

function cssRuleSetObj(aRuleSet)
{
  this.m_cssRule = aRuleSet;
  this.m_selectors = [];
  this.initObj();
}

function cssRuleSetToRuleSetObj(aRuleSet)
{
  if (aRuleSet.type !== CSSRule.STYLE_RULE)
    return null;
  return new cssRuleSetObj(aRuleSet);
}

cssRuleSetObj.prototype = cssRuleSetBase;

function parseCSSSelectorString(selString)
{
  separatorsRE = /\s*([\"\,\'\\])\s*/;

  var bInSingleQuotes = false, bInDoubleQuotes = false, bIsEscapedChar = false;
  var nCurrIndex = 0;
  var currSelector = "";
  var selectorStrings = [];
  var selectorPieces = selString.split(separatorsRE);
  var selectorObjs = [];
  for (var jx = 0; jx < selectorPieces.length; ++jx)
  {
    if (bIsEscapedChar)
    {
      bIsEscapedChar = false;
      currSelector += selectorPieces[jx];
    }
    else if (bInSingleQuotes)
    {
      bInSingleQuotes = (selectorPieces[jx] !== "'");
      currSelector += selectorPieces[jx];
    }
    else if (bInDoubleQuotes)
    {
      bInDoubleQuotes = selectorPieces[jx] !== "\"";
      currSelector += selectorPieces[jx];
    }
    else
    {
      switch(selectorPieces[jx])
      {
        case "\\":
          bIsEscapedChar = true;
          currSelector += selectorPieces[jx];
        break;
        case "\"":
          bInDoubleQuotes = true;
          currSelector += selectorPieces[jx];
        break;
        case "'":
          bInSingleQuotes = true;
          currSelector += selectorPieces[jx];
        break;
        case ",":
          selectorStrings[nCurrIndex] = currSelector;
          currSelector = "";
          ++nCurrIndex;
        break;
        default:
          currSelector += selectorPieces[jx];
        break;
      }
    }
  }
  if (currSelector.length)  //Collect what's left at the end
    selectorStrings[nCurrIndex] = currSelector;

  for (var ii = 0; ii < selectorStrings.length; ++ii)
    selectorObjs.push( new cssSelectorObj(selectorStrings[ii]) );  //this completes the parsing for each one
  return selectorObjs;
}

var cssSelectorObjBase = 
{
  m_combinatorsRE : /(?:\s*([\\\"\'\+>])\s*)|(\s+)/,
  m_whiteSpaceRE : /^\s+$/,
  m_selString : "",
  m_simpleSelectors : [],
  m_separators : [],

  parseSelectorString : function()
  {
    if (!this.m_selString.length)
    {
      dump("cssSelectorObjBase.parseSelectorString() called with empty m_selString! Aborting...\n");
      return;
    }

    var bInSingleQuotes = false, bInDoubleQuotes = false, bIsEscapedChar = false;
    var nCurrSelIndex = 0, nCurrSepIndex = 0;
    var currSimpleSelector = "";
    var simpleSelectorStrings = [];
    var selectorPieces = this.m_selString.split(this.m_combinatorsRE);
    for (var jx = 0; jx < selectorPieces.length; ++jx)
    {
      if (bIsEscapedChar)
      {
        bIsEscapedChar = false;
        currSimpleSelector += selectorPieces[jx];
      }
      else if (bInSingleQuotes)
      {
        bInSingleQuotes = (selectorPieces[jx] !== "'");
        currSimpleSelector += selectorPieces[jx];
      }
      else if (bInDoubleQuotes)
      {
        bInDoubleQuotes = selectorPieces[jx] !== "\"";
        currSimpleSelector += selectorPieces[jx];
      }
      else
      {
        switch(selectorPieces[jx])
        {
          case "\\":
            bIsEscapedChar = true;
            currSimpleSelector += selectorPieces[jx];
          break;
          case "\"":
            bInDoubleQuotes = true;
            currSimpleSelector += selectorPieces[jx];
          break;
          case "'":
            bInSingleQuotes = true;
            currSimpleSelector += selectorPieces[jx];
          break;
          default:
            if (this.m_whiteSpaceRE.test(selectorPieces[jx])) //replace the whitespace with a single space character
            {
              simpleSelectorStrings.push(currSimpleSelector);
              this.m_separators.push(" ");
              currSimpleSelector = "";
            } 
            else
            {
              currSimpleSelector += selectorPieces[jx];
            }
          break;
          case ">":
          case "+":
            simpleSelectorStrings.push(currSimpleSelector);
            this.m_separators.push(selectorPieces[jx]);
            currSimpleSelector = "";
          break;
        }
      }
    }
    if (currSimpleSelector.length)
      simpleSelectorStrings.push(currSimpleSelector);

    for (var ii = 0; ii < simpleSelectorStrings.length; ++ii)
      this.m_simpleSelectors.push( new cssSimpleSelectorObj(simpleSelectorStrings[ii]) );  //this completes the parsing for each one
  },

  matchesOneOf : function(selectorArray)
  {
    var retVal = -1;
    for (var ii = 0; (retVal < 0) && (ii < selectorArray.length); ++ii)
    {
      if (this.matches(selectorArray[ii]))
        retVal = ii;
    }
    return retVal;
  },

  //This function determines whether this selector selects a subset of what otherSelector does.
  matches : function(otherSelectorObj)
  {
    var matchIndex = 0;
    for (var jj = 0; jj < otherSelectorObj.m_simpleSelectors.length; ++jj)
    {
      if (matchIndex >= this.m_simpleSelectors.length)
        return false; //otherSelector has more conditions to put, and we're all done, so it selects a subset of ours?
      if (!this.m_simpleSelectors[matchIndex].matches(otherSelectorObj.m_simpleSelectors[jj]))
      {
        return false;  //except for when?
      }
    if ( otherSelectorObj.m_separators[jj] && (!this.m_separators[matchIndex] || (this.m_separators[matchIndex] !== otherSelectorObj.m_separators[jj])) )
      return false;
    if (this.m_separators[matchIndex] && !otherSelectorObj.m_separators[jj])
    return false;
      ++matchIndex;
    }
    return true;
  },

  matchesExact : function(otherSelectorObj)
  {
    if (this.m_simpleSelectors.length !== otherSelectorObj.m_simpleSelectors.length)
      return false;
    if (this.m_separators.length !== otherSelectorObj.m_separators.length)
      return false;
    for (var jj = 0; jj < otherSelectorObj.m_simpleSelectors.length; ++jj)
    {
      if (!this.m_simpleSelectors[jj].matchesExact(otherSelectorObj.m_simpleSelectors[jj]))
      {
        return false;  //except for when?
      }
      if ( otherSelectorObj.m_separators[jj] && (!this.m_separators[jj] || (this.m_separators[jj] !== otherSelectorObj.m_separators[jj])) )
        return false;
      if (this.m_separators[jj] && !otherSelectorObj.m_separators[jj])
        return false;
    }
    return true;
  }

};

function cssSelectorObj(selectorStr)
{
  this.m_selString = selectorStr;
  this.m_simpleSelectors = [];
  this.m_separators = [];
  this.parseSelectorString();
}

cssSelectorObj.prototype = cssSelectorObjBase;

var cssSimpleSelectorBase =
{
  //constants
  m_tokenCharsRE : /([\|\[\]=\:\.#\*\|~\\\"\'])/,
  m_attributeRE : /\s*([^\s=]+)\s*(?:([\|\~]?=)\s*([^\s].*))?$/,
  // color ~= "0x808080"
  noSelector : 0,
  nsSelector : 1,
  elemSelector : 2,
  idSelector : 3,
  classSelector : 4,
  attrSelector : 5,
  pseudoClassSelector : 6,
  m_selString : "",
  m_nsSelector : "",
  m_elemSelector : "",
  m_attrSelectors : [],
  m_classSelectors : [],
  m_pseudoClassSelectors : [],
  m_idSelector : "",

  createAttrObj : function(attrString)
  {
    var attrPieces = this.m_attributeRE.exec(attrString);
    var attrObj;
    if (attrPieces && attrPieces.length > 1)
    {
      attrObj = {attrName : attrPieces[1]};  //is this right?
      if (attrPieces.length > 2)
      {
        if (attrPieces.length > 3)
        {
          attrObj.binaryOp = attrPieces[2];
          attrObj.attrValue = TrimString(attrPieces[3]);
        }
        else
          attrObj.attrValue = TrimString(attrPieces[2]);
      }
    }
    return attrObj;
  },

  storeAPiece : function(whichType, thePiece)
  {
    if (thePiece.length === 0)
      return;
    switch(whichType)
    {
      case this.nsSelector :          this.m_nsSelector = thePiece;                 break;
      case this.elemSelector :        this.m_elemSelector = thePiece;               break;
      case this.idSelector :          this.m_idSelector = thePiece;                 break;
      case this.classSelector :       this.m_classSelectors.push(thePiece);         break;
      case this.pseudoClassSelector : this.m_pseudoClassSelectors.push(thePiece);   break;
      case this.attrSelector:
        this.m_attrSelectors.push( this.createAttrObj(thePiece) );
      break;
      default:
        dump("cssSimpleSelectorBase.storeAPiece() called with no type set (piece is [" + thePiece + "]! Aborting...\n");
      break;
    }
  },

  parseSelectorString : function()
  {
    if (!this.m_selString.length)
    {
      dump("cssSelectorObjBase.parseSelectorString() called with empty m_selString! Aborting...\n");
      return;
    }

    var bInSingleQuotes = false, bInDoubleQuotes = false, bIsEscapedChar = false;
    var nCurrIndex = 0;
    var currSelectorPiece = "";
    var nextPart = this.elemSelector;
    var inPart = this.elemSelector;
    var simpleSelectorStrings = [];
    var selectorPieces = this.m_selString.split(this.m_tokenCharsRE);
    var bFinished = false;
    for (var jx = 0; jx < selectorPieces.length; ++jx)
    {
      if (bIsEscapedChar)
      {
        bIsEscapedChar = false;
        currSelectorPiece += selectorPieces[jx];
      }
      else if (bInSingleQuotes)
      {
        bInSingleQuotes = (selectorPieces[jx] !== "'");
        currSelectorPiece += selectorPieces[jx];
      }
      else if (bInDoubleQuotes)
      {
        bInDoubleQuotes = selectorPieces[jx] !== "\"";
        currSelectorPiece += selectorPieces[jx];
      }
      else
      {
        switch(selectorPieces[jx])
        {
          case "\\":
            bIsEscapedChar = true;
            currSelectorPiece += selectorPieces[jx];
          break;
          case "\"":
            bInDoubleQuotes = true;
            currSelectorPiece += selectorPieces[jx];
          break;
          case "'":
            bInSingleQuotes = true;
            currSelectorPiece += selectorPieces[jx];
          break;
          case ":":
            bFinished = true;
            nextPart = this.pseudoClassSelector;
          break;
          case "[":
            if ((inPart === this.elemSelector) && !currSelectorPiece.length)
              currSelectorPiece = "*";  //this can be inferred for an id selector
            nextPart = this.attrSelector;
            bFinished = true;
          break;
          //From here down they all fall through if the condition isn't met and just append to the currSelectorPiece
          case "]":
            if (inPart === this.attrSelector)
            {
              nextPart = this.noSelector;
              bFinished = true;
              break;
            }
          break;
          case ".":
            if (inPart === this.elemSelector)
            {
              if (!currSelectorPiece.length)
                currSelectorPiece = "*";  //this can be inferred for an id selector
              bFinished = true;
              nextPart = this.classSelector;
              break;
            }
          case "|":
            if (inPart === this.elemSelector)
            {
              inPart = this.nsSelector;
              nextPart = this.elemSelector;
              bFinished = true;
              break;
            }
//          break;
          case "#":
            if (inPart === this.elemSelector)
            {
              if (!currSelectorPiece.length)
                currSelectorPiece = "*";  //this can be inferred for an id selector
              bFinished = true;
              nextPart = this.idSelector;
              break;
            }
          case "=":  //No need to treat this differently from other characters?
          case "*":  //No need to treat this differently from other characters?
          default:
            currSelectorPiece += selectorPieces[jx];
          break;
        }
      }
      if (bFinished)
      {
        if ((inPart === this.elemSelector) && !currSelectorPiece.length)
          currSelectorPiece = "*";
        this.storeAPiece(inPart, currSelectorPiece);
        currSelectorPiece = "";
        inPart = nextPart;
        bFinished = false;
      }
    }
    if (currSelectorPiece.length)
      this.storeAPiece(inPart, currSelectorPiece);

//    for (var ii = 0; ii < simpleSelectorStrings.length; ++ii)
//      this.m_simpleSelectors.push( new cssSimpleSelectorObj(simpleSelectorStrings[ii]) );  //this completes the parsing for each one
  },

  matches : function(otherSimpleSelectorObj)
  {
    if ( otherSimpleSelectorObj.m_nsSelector.length && (otherSimpleSelectorObj.m_nsSelector !== "*") && this.m_nsSelector.length && (this.m_nsSelector !== otherSimpleSelectorObj.m_nsSelector) )
      return false;
    if ( (otherSimpleSelectorObj.m_elemSelector !== "*") && (otherSimpleSelectorObj.m_elemSelector !== this.m_elemSelector) )
      return false;
    if (otherSimpleSelectorObj.m_idSelector && (!this.m_idSelector || (this.m_idSelector !== otherSimpleSelectorObj.m_idSelector)) )
      return false;
    var ii, bFound;
    for (ii = 0; ii < otherSimpleSelectorObj.m_classSelectors.length; ++ii)
    {
      if (this.m_classSelectors.indexOf( otherSimpleSelectorObj.m_classSelectors[ii] ) < 0)
        return false;
    }
    for (ii = 0; ii < otherSimpleSelectorObj.m_attrSelectors.length; ++ii)
    {
      var attribName = otherSimpleSelectorObj.m_attrSelectors[ii].attrName;
      bFound = false;
      for (var jj = 0; !bFound && (jj < this.m_attrSelectors.length); ++jj)
      {
        if (this.m_attrSelectors[jj].attrName === attribName)
        {
          if (!otherSimpleSelectorObj.m_attrSelectors[ii].attrValue || (otherSimpleSelectorObj.m_attrSelectors[ii].attrValue === this.m_attrSelectors[jj].attrValue))
            bFound = true;
        }
      }
      if (!bFound)
        return false;
    }
    for (ii = 0; ii < otherSimpleSelectorObj.m_pseudoClassSelectors.length; ++ii)
    {
      if (this.m_pseudoClassSelectors.indexOf(otherSimpleSelectorObj.m_pseudoClassSelectors[ii]) < 0)
        return false;
    }
    return true;
  },

  matchesExact : function(otherSimpleSelectorObj)
  {
    if (this.m_nsSelector !== otherSimpleSelectorObj.m_nsSelector)
    {
      if (this.m_nsSelector.length && (this.m_nsSelector !== "*"))
        return false;
      if (otherSimpleSelectorObj.m_nsSelector.length && (otherSimpleSelectorObj.m_nsSelector !== "*"))
        return false;
    }
    if (this.m_elemSelector !== otherSimpleSelectorObj.m_elemSelector)
    {
      if (this.m_elemSelector.length && (this.m_elemSelector !== "*"))
        return false;
      if (otherSimpleSelectorObj.m_elemSelector.length && (otherSimpleSelectorObj.m_elemSelector !== "*"))
        return false;
    }
    if (this.m_idSelector !== otherSimpleSelectorObj.m_idSelector)
    {
      if (this.m_idSelector.length || otherSimpleSelectorObj.m_idSelector.length)
        return false;
    }
    var ii, bFound;
    if (this.m_classSelectors.length !== otherSimpleSelectorObj.m_classSelectors.length)
      return false;
    for (ii = 0; ii < otherSimpleSelectorObj.m_classSelectors.length; ++ii)
    {
      if (this.m_classSelectors.indexOf( otherSimpleSelectorObj.m_classSelectors[ii] ) < 0)
        return false;
    }
    if (this.m_attrSelectors.length !== otherSimpleSelectorObj.m_attrSelectors.length)
      return false;
    for (ii = 0; ii < otherSimpleSelectorObj.m_attrSelectors.length; ++ii)
    {
      var attribName = otherSimpleSelectorObj.m_attrSelectors[ii].attrName;
      bFound = false;
      for (var jj = 0; !bFound && (jj < this.m_attrSelectors.length); ++jj)
      {
        if (this.m_attrSelectors[jj].attrName === attribName)
        {
          if (!otherSimpleSelectorObj.m_attrSelectors[ii].attrValue && !otherSimpleSelectorObj.m_attrSelectors[ii].attrValue)
            bFound = true;
          else if (otherSimpleSelectorObj.m_attrSelectors[ii].attrValue === this.m_attrSelectors[jj].attrValue)
            bFound = true;
        }
      }
      if (!bFound)
        return false;
    }
    if (this.m_pseudoClassSelectors.length !== otherSimpleSelectorObj.m_pseudoClassSelectors.length)
      return false;
    for (ii = 0; ii < otherSimpleSelectorObj.m_pseudoClassSelectors.length; ++ii)
    {
      if (this.m_pseudoClassSelectors.indexOf(otherSimpleSelectorObj.m_pseudoClassSelectors[ii]) < 0)
        return false;
    }
    return true;
  }

};

function cssSimpleSelectorObj(selectorStr)
{
  this.m_selString = selectorStr;
  this.m_nsSelector = "";
  this.m_elemSelector = "";
  this.m_attrSelectors = [];
  this.m_classSelectors = [];
  this.m_pseudoClassSelectors = [];
  this.m_idSelector = "";
  this.parseSelectorString();
}

cssSimpleSelectorObj.prototype = cssSimpleSelectorBase;
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
  this.showInvisibles = ((viewFlags & this.hideInvisiblesFlag) === 0);
  this.showHelperLines = ((viewFlags & this.hideHelperLinesFlag) === 0);
  this.showInputBoxes = ((viewFlags & this.hideInputBoxesFlag) === 0);
  this.showMarkers = ((viewFlags & this.hidemarkersFlag) === 0);
  this.showFootnotes = ((viewFlags & this.hideFootnotesFlag) === 0);
  this.showOtherNotes = ((viewFlags & this.hideOtherNotesFlag) === 0);
  this.showIndexEntries = ((viewFlags & this.hideindexentriesFlag) === 0);

  this.match = function(otherSettings)
  {
    if (this.showInvisibles !== otherSettings.showInvisibles)
      return false;
    if (this.showHelperLines !== otherSettings.showHelperLines)
      return false;
    if (this.showInputBoxes !== otherSettings.showInputBoxes)
      return false;
    if (this.showMarkers !== otherSettings.showMarkers)
      return false;
    if (this.showMFootnotes !== otherSettings.showFootnotes)
      return false;
    if (this.showOtherNotess !== otherSettings.showOtherNotes)
      return false;
    if (this.showIndexEntries !== otherSettings.showIndexEntries)
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
      theFlags |= this.hidemarkersFlag;
    if (!this.showIndexEntries)
      theFlags |= this.hideindexentriesFlag;
    if (!this.showFootnotes)
      theFlags |= this.hideFootnotesFlag;
    if (!this.showOtherNotes)
      theFlags |= this.hideOtherNotesFlag;
    return theFlags;
  };

}

var msiViewSettingsBase =
{ 
  hideInvisiblesFlag   :  1,
  hideHelperLinesFlag  :  2,
  hideInputBoxesFlag   :  4,
  hidemarkersFlag      :  8,
  hideindexentriesFlag : 16,
  hideFootnotesFlag    : 32,
  hideOtherNotesFlag   : 64
};

msiViewSettings.prototype = msiViewSettingsBase;

function msiGetCurrViewSettings(editorElement)
{
  var viewSettings = null;
  if (("viewSettings" in editorElement) && (editorElement.viewSettings !== null))
    viewSettings = editorElement.viewSettings;
  else
    viewSettings = getViewSettingsFromViewMenu();

  return viewSettings;
}

function msiGetCurrNoteViewSettings(editorElement)
{
  var viewSettings = null;
  if (("noteViewSettings" in editorElement) && (editorElement.noteViewSettings !== null))
    viewSettings = editorElement.noteViewSettings;
  else if (PrefHasValue("noteViewSettings"))
    viewSettings = new msiViewSettings( GetIntPref("noteViewSettings") );
  else
    viewSettings = msiGetCurrViewSettings(editorElement);

  return viewSettings;
}


function msiGetCurrViewPercent()
{
  var zoom = ZoomManager.zoom * 100;
  return zoom;
}

function msiSetCurrViewPercent(zoomvalue)
{
  ZoomManager.zoom = zoomvalue/100;
  return ZoomManager.zoom;
}

function msiGetSavedViewPercent(editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);
  var doc = editor.document;
  var regex = /TCIDATA\{meta name=\"ViewPercent\" content=\"([.\d]+)\"\}/;
  var treeWalker = doc.createTreeWalker(
      doc.documentElement,
      NodeFilter.SHOW_COMMENT,
      { acceptNode: function(node) { return regex.test(node.nodeValue)?NodeFilter.FILTER_ACCEPT:NodeFilter.FILTER_REJECT; } },
      false
  );
  var node = treeWalker.nextNode();
  if (node) {
    match = regex.exec(node.nodeValue);
    if (match) {
      if (match[1])
      {
        return match[1]/100;
      }
    }
  }
  return null;
}

function msiSetSavedViewPercent(editorElement, zoompercent)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);
  var doc = editor.document;
  var regex = /TCIDATA\{meta name=\"ViewPercent\" content=\"([.\d]+)\"\}/;
  var treeWalker = doc.createTreeWalker(
      doc.documentElement,
      NodeFilter.SHOW_COMMENT,
      { acceptNode: function(node) { return regex.test(node.nodeValue)?NodeFilter.FILTER_ACCEPT:NodeFilter.FILTER_REJECT; } },
      false
  );
  node = treeWalker.nextNode();
  if (node) {
    match = regex.exec(node.nodeValue);
    if (match) {
      if (match[1])
      {
        node.nodeValue = 'TCIDATA{meta name="ViewPercent" content="'+ zoompercent + '"}';
      }
    }
  }
}


//The default print options should be stored as a numerical bit-encoded value, as of old? (Since otherwise they'd
//  be a bit lengthy.) But we should return an object with the bits interpreted(?).
//NOTE that this doesn't do any querying of current view settings (if that flag is set). It simply translates
//  the bits.
function msiPrintOptions(printFlags)
{
  this.useCurrViewSettings = (printFlags & msiDocumentInfoBase.printUseViewSettingsFlag) !== 0;
  this.printInvisibles = (printFlags & msiDocumentInfoBase.printShowInvisiblesFlag) !== 0;    
  this.printHelperLines = (printFlags & msiDocumentInfoBase.printShowMatrixLinesFlag) !== 0;
  this.printInputBoxes = (printFlags & msiDocumentInfoBase.printShowInputBoxesFlag) !== 0;
  this.printMarkers = (printFlags & msiDocumentInfoBase.printShowIndexFieldsFlag) !== 0;
  this.printIndexEntries = (printFlags & msiDocumentInfoBase.printShowMarkerFieldsFlag) !== 0;
  this.allTextInBlack = (printFlags & msiDocumentInfoBase.printBlackTextFlag) !== 0;
  this.allLinesInBlack = (printFlags & msiDocumentInfoBase.printBlackLinesFlag) !== 0;
  this.backgroundsTransparent = (printFlags & msiDocumentInfoBase.printTransparentBackgroundFlag) !== 0;
  this.grayButtonsTransparent = (printFlags & msiDocumentInfoBase.printTransparentGrayButtonsFlag) !== 0;
  this.suppressGrayBoxes = (printFlags & msiDocumentInfoBase.printSuppressGrayButtonsFlag) !== 0;
  this.useCurrViewZoom = (printFlags & msiDocumentInfoBase.printUseViewSettingZoomFlag) !== 0;

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
    if (this.useCurrViewSettings !== otherOptions.useCurrViewSettings)
      return false;
    if (this.printInvisibles !== otherOptions.printInvisibles)
      return false;
    if (this.printHelperLines !== otherOptions.printHelperLines)
      return false;
    if (this.printInputBoxes !== otherOptions.printInputBoxes)
      return false;
    if (this.printMarkers !== otherOptions.printMarkers)
      return false;
    if (this.printIndexEntries !== otherOptions.printIndexEntries)
      return false;
    if (this.allTextInBlack !== otherOptions.allTextInBlack)
      return false;
    if (this.allLinesInBlack !== otherOptions.allLinesInBlack)
      return false;
    if (this.backgroundsTransparent !== otherOptions.backgroundsTransparent)
      return false;
    if (this.grayButtonsTransparent !== otherOptions.grayButtonsTransparent)
      return false;
    if (this.suppressGrayBoxes !== otherOptions.suppressGrayBoxes)
      return false;
    if (this.useCurrViewZoom !== otherOptions.useCurrViewZoom)
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

var msiUnitsListBase =
{
  mUnitFactors : [],
  mParseRE : null,

  defaultUnit : function()
  {
    if ((this.mUnitfactors === null) || ("mm" in this.mUnitFactors))
      return "mm";
    return this.mUnitFactors[0];
  },

  convertUnits : function(invalue, inunit, outunit)
  {
    if (inunit === outunit) return invalue;
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
  },

  getDisplayString : function(theUnit)
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
  },

  getParsingRegExp : function()
  {
    if (this.mParseRE)
      return this.mParseRE;

    var unitsStr = "";
    for (var aUnit in this.mUnitFactors)
    {
      if (unitsStr.length > 0)
        unitsStr += "|";
      unitsStr += aUnit;
    }
    this.mParseRE = new RegExp("(\\-?\\d*\\.?\\d*).*(" + unitsStr + ")");
    return this.mParseRE;
  },

  getNumberAndUnitFromString : function(valueStr)
  {
    var matchArray = this.getParsingRegExp().exec(valueStr);
    if (matchArray !== null)
    {
      var retVal = new Object();
      retVal.number = Number(matchArray[1]);
      retVal.unit = matchArray[2];
      return retVal;
    }
    return null;
  },

  //if outUnit is null, no conversion is done. If outPrecision is null, it's just a Number.toString concatenated with outUnit.
  stringValue : function(aValue, aUnit, outUnit, outPrecision)
  {
    var outValue = aValue;
    if (outUnit && (outUnit !== aUnit))
      outValue = this.convertUnits(aValue, aUnit, outUnit);
    return (Number(outValue).toPrecision(outPrecision) + outUnit);
  },

  compareUnitStrings : function(value1, value2)
  {
    var firstValue = this.getNumberAndUnitFromString(value1);
    var secondValue = this.getNumberAndUnitFromString(value2);
    if ((first === null) || (second === null))
    {
      dump("Problem in msiUnitsList.compareUnitStrings - trying to compare unrecognized units!\n");
      return Number.NaN;
    }
    var convertedFirst = this.convertUnits(value1.number, value1.unit, value2.unit);
    if(convertedFirst < value2.number)
      return -1;
    else if (convertedFirst > value2.number)
      return 1;
    else
      return 0;
  },

  //outUnit and outPrecision are optional - see stringValue function above for default behavior.
  subtractUnitStrings : function(value1, value2, outUnit, outPrecision)
  {
    var firstValue = this.getNumberAndUnitFromString(value1);
    var secondValue = this.getNumberAndUnitFromString(value2);
    if ((firstValue === null) || (secondValue === null))
    {
      dump("Problem in msiUnitsList.compareUnitStrings - trying to compare unrecognized units!\n");
      return "";
    }
    var convertedFirst = this.convertUnits(firstValue.number, firstValue.unit, "pt");
    var convertedSecond = this.convertUnits(secondValue.number, secondValue.unit, "pt");
    if (!outUnit)
      outUnit = firstValue.unit;
    return this.stringValue( convertedFirst - convertedSecond, "pt", outUnit, outPrecision );
  },

  //outUnit and outPrecision are optional - see stringValue function above for default behavior.
  addUnitStrings : function(value1, value2, outUnit, outPrecision)
  {
    var firstValue = this.getNumberAndUnitFromString(value1);
    var secondValue = this.getNumberAndUnitFromString(value2);
    if ((firstValue === null) || (secondValue === null))
    {
      dump("Problem in msiUnitsList.compareUnitStrings - trying to compare unrecognized units!\n");
      return "";
    }
    var convertedFirst = this.convertUnits(firstValue.number, firstValue.unit, "pt");
    var convertedSecond = this.convertUnits(secondValue.number, secondValue.unit, "pt");
    if (!outUnit)
      outUnit = firstValue.unit;
    return this.stringValue( convertedFirst + convertedSecond, "pt", outUnit, outPrecision );
  },

  isNullOrZero : function(stringVal)
  {
    var retVal = true;
    if (stringVal && stringVal.length)
    {
      var theValue = this.getNumberAndUnitFromString(stringVal);
      retVal = (theValue.number === 0);
    }
    return retVal;
  }

}

function msiUnitsList(unitConversions) 
{
  this.mUnitFactors = unitConversions;
}

msiUnitsList.prototype = msiUnitsListBase;

//Following need to be added to. They're called "CSSUnitConversions", but are intended to handle any units showing up in
//markup - particularly in XBL (see latex.xml!).
var msiCSSUnitConversions =
{
  pt: .3514598,  //mm per pt
  "in": 25.4,  //mm per in
  mm: 1, // mm per mm
  cm: 10 // mm per cm
};

var msiCSSUnitsList = new msiUnitsList(msiCSSUnitConversions);
msiCSSUnitsList.defaultUnit = function() {return "pt";};

//Question remains of what to do with percent values??
function msiCSSWithFontUnitsList(fontSize, fontUnits)
{
  this.mUnitFactors = new Object();
  for (var aUnit in msiCSSUnitConversions)
    this.mUnitFactors[aUnit] = msiCSSUnitConversions[aUnit];
  if (fontSize && fontUnits)
    this.mUnitFactors.em = msiCSSUnitsList.convertUnits(fontSize, fontUnits, "mm");
  else
    this.mUnitFactors.em = 12 * msiCSSUnitConversions.pt;  //wild-ass guess
  this.mUnitFactors.ex = .5 * this.mUnitFactors.em;
  this.mUnitFactors.px = .75 * msiCSSUnitConversions.pt;  //generic guess is that a pixel is 3/4 of a point
  this.mUnitFactors.pc = 12 * msiCSSUnitConversions.pt;
  this.defaultUnit = function() {return "pt";};
  this.fontSizeInPoints = function() 
    {return Math.round( this.convertUnits(1, "em", "pt") );}
  ;
}

msiCSSWithFontUnitsList.prototype = new msiUnitsList;

function msiCreateCSSUnitsListForElement(anElement)
{
  var defView = anElement.ownerDocument.defaultView;
  var docCSS = defView.QueryInterface(Components.interfaces.nsIDOMViewCSS);
  var theStyle = docCSS.getComputedStyle(anElement, "");
  var theFontHtVal = theStyle.getPropertyCSSValue("font-size");
  var fontHtNumberAndUnits = null;
  if (theFontHtVal)
    fontHtNumberAndUnits = msiCSSUtils.getLengthWithUnitsFromCSSPrimitiveValue(theFontHtVal);
  if (fontHtNumberAndUnits && fontHtNumberAndUnits.unit && fontHtNumberAndUnits.number)
    return new msiCSSWithFontUnitsList(fontHtNumberAndUnits.number, fontHtNumberAndUnits.units);
  else if (fontHtNumberAndUnits.number)
    return new msiCSSWithFontUnitsList(fontHtNumberAndUnits.number, "px");
  else
    return new msiCSSWithFontUnitsList(12 * msiCSSUnitConversions.pt, "mm");
}

function msiGetNumberAndLengthUnitFromString (valueStr)
{
  var unitsStr = "pt|in|mm|cm|pc|em|ex|px";
  var ourRegExp = new RegExp("(\\-?\\d*\\.?\\d*).*(" + unitsStr + ")");
  var matchArray = ourRegExp.exec(valueStr);
  if (matchArray !== null)
  {
    var retVal = new Object();
    retVal.number = Number(matchArray[1]);
    retVal.unit = matchArray[2];
    return retVal;
  }
  return null;
};

var msiBaseMathNameList = 
{
  bInitialized: false,
  namesDoc : null,
  sourceFile : "mathnames.xml",
  bModified: false,
  bModifiedAutoSubs: false,

  initialize : function()
  {
    var mathNameFile = getUserResourceFile("mathnames.xml","xml");
    this.sourceFile = mathNameFile.target;
//    this.namesDoc = document.implementation.createDocument("", "mathnames", null);
//    var nodeList;
//    var node;
//    var s;
//    var arrayElement;

    var request = Components.classes["@mozilla.org/xmlextras/xmlhttprequest;1"].createInstance();
    request.QueryInterface(Components.interfaces.nsIXMLHttpRequest);
    var thePath = msiFileURLFromAbsolutePath( mathNameFile.target ).spec;
    try {
      request.open("GET", thePath, false);
      request.send(null);
                                
      this.namesDoc = request.responseXML; 
      if (!this.namesDoc && request.responseText)
        throw("file exists but cannot be parsed as XML");

      this.bInitialized = true;
      this.initAutoCompleteList();
    }
    catch(exc)
    {
      dump("Unable to load math names file \"" + this.sourceFile + "\"; exception is [" + exc + "].\n");
      return null;
    }


//    this.namesDoc.async = false;
//    if (this.namesDoc.load("file:///" + mathNameFile.path))
//    {
//      this.bInitialized = true;
//      this.initAutoCompleteList();
//    }
  },

  initAutoCompleteList : function()
  {
    if (!this.bInitialized)
    {
      dump("In msiBaseMathNameList, initAutoCompleteList being called without bInitialized being set!?\n");
      this.initialize();
    }
//    var ACSA = Components.classes["@mozilla.org/autocomplete/search;1?name=stringarray"].getService();
//    ACSA.QueryInterface(Components.interfaces.nsIAutoCompleteSearchStringArray);
    var ACSA = msiSearchStringManager.setACSAImpGetService();
    var nameNodesList = this.namesDoc.getElementsByTagName("mathname");
    // BBM: should we initialize this list??
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
    if (theNode !== null)
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
    if (theAppearanceList !== null && theAppearanceList.length > 0)
      nameData.appearance = theAppearanceList[0].cloneNode(true);
    nameData.builtIn = (nameNode.hasAttribute("builtIn") && nameNode.getAttribute("builtIn") === "true");
    if (nameNode.hasAttribute("limitPlacement"))
      nameData.limitPlacement = nameNode.getAttribute("limitPlacement");
    if ( (nameNode.hasAttribute("engineFunction")) && (nameNode.getAttribute("engineFunction") === "true") )
      nameData.enginefunction = true;
    if (this.nameHasAutoSubstitution(nameData.val))
      nameData.autoSubstitute = true;
    return nameData;
  },

  deleteName : function(aName)
  {
    var nameNode = this.namesDoc.getElementById(aName);
    if (nameNode !== null)
    {
      var isBuiltIn = nameNode.getAttribute("builtIn");
      if ((!isBuiltIn) || (isBuiltIn !== "true"))
      {
        if (this.nameHasAutoSubstitution(aName))
          this.removeAutoSubstitution(aName);
        nameNode.parentNode.removeChild(nameNode);
//        var ACSA = Components.classes["@mozilla.org/autocomplete/search;1?name=stringarray"].getService();
//        ACSA.QueryInterface(Components.interfaces.nsIAutoCompleteSearchStringArray);
        var ACSA = msiSearchStringManager.setACSAImpGetService();
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
    if (nameNode !== null)
    {
      var isBuiltIn = nameNode.getAttribute("builtIn");
      if (isBuiltIn && (isBuiltIn === "true"))
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
    if (("nameData" in aNameData) && (aNameData.nameData !== null) && ("nodeType" in aNameData.nameData))
    {
      newData = aNameData.nameData.cloneNode(true);
      newNode.appendChild(newData);
    }
    if ("limitPlacement" in aNameData)
      newNode.setAttribute("limitPlacement", aNameData.limitPlacement);
    parentNode.appendChild(newNode);
//    var ACSA = Components.classes["@mozilla.org/autocomplete/search;1?name=stringarray"].getService();
//    ACSA.QueryInterface(Components.interfaces.nsIAutoCompleteSearchStringArray);
    var ACSA = msiSearchStringManager.setACSAImpGetService();
    ACSA.addString("mathnames", aName);
    ACSA.sortArrays();
    if (aNameData.autoSubstitute === true)
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
      result = autosub.nextChar(true,aName.charAt(ix));
      if (result === Components.interfaces.msiIAutosub.STATE_FAIL)
        return false;
    }
    return (result === Components.interfaces.msiIAutosub.STATE_SUCCESS);
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
    if (this.bModified && (this.namesDoc !== null))
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
//        if (("bDeleted" in this.names[aName]) && (this.names[aName].bDeleted === true))
//          msiBaseMathNameList.deleteName(aName);
//        else if (("added" in this.names[aName]) && (this.names[aName].added === true))
//          msiBaseMathNameList.addName(aName, this.names[aName]);
//        else if (("changed" in this.names[aName]) && (this.names[aName].changed === true))
//          msiBaseMathNameList.updateName(aName, this.names[aName]);
//      } catch(exc) {dump("Problem checking properties for member [" + aName + "] of msiMathNameList.names: [" + exc + "].\n");}
//    }
    msiBaseMathNameList.updateFile();
  };
  this.canDelete = function(aName)
  {
    if ((aName in this.names) && (this.names[aName] !== null) && (!this.names[aName].builtIn))
    {
//      return !("bDeleted" in this.names[aName]) || (this.names[aName].bDeleted !== true);
      return true;
    }
    return false;
  };
  this.canAdd = function(aName)
  {
    if ((aName in this.names) && (this.names[aName] !== null))
    {
      return (!this.names[aName].builtIn) && ("bDeleted" in this.names[aName]) && (this.names[aName].bDeleted === true);
//      return (!this.names[aName].builtIn);
    }
    return true;
  };
  this.hasAutoSubstitution = function(aName)
  {
    if ((aName in this.names) && ("autoSubstitute" in this.names[aName]) && (this.names[aName].autoSubstitute === true))
      return true;
    return false;
  };
  this.isBuiltIn = function(aName)
  {
    if ((aName in this.names) && (this.names[aName] !== null))
    {
      if (this.names[aName].builtIn === true)
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
    if (appearanceList !== null && !this.appearanceIsUsual(appearanceList, aName, aType, aLimitPlacement))
      nameData.appearance = appearanceList;
    nameData.builtIn = false;
    if ((aType === "operator") && (aLimitPlacement !== null) && (aLimitPlacement.length > 0) && (aLimitPlacement !== "auto"))
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
  this.setUpTextBoxControl = function(theControl)
  {
//    theControl.markerList = this;
    var currStr = "";
    if (theControl.hasAttribute("onfocus"))
      currStr = theControl.getAttribute("onfocus");
    theControl.setAttribute("onfocus", "msiSearchStringManager.setACSAImp();" + currStr);
//    theControl.setAttribute("autocompletesearchparam", this.getIndexString());
  };
}

var msiBaseMathUnitsList = 
{
  bInitialized: false,
  namesDoc : null,
  sourceFile : "unitnames.xml",
  bModified: false,
  bModifiedAutoSubs: false,

  initialize : function()
  {
    var unitNameFile;
    unitNameFile = getUserResourceFile("unitnames.xml", "xml");
    this.sourceFile = unitNameFile.path;

    var request = Components.classes["@mozilla.org/xmlextras/xmlhttprequest;1"].createInstance();
    request.QueryInterface(Components.interfaces.nsIXMLHttpRequest);
    var thePath = msiFileURLFromAbsolutePath( unitNameFile.target ).spec;
    try {
      request.open("GET", thePath, false);
      request.send(null);
                                
      this.namesDoc = request.responseXML; 
      if (!this.namesDoc && request.responseText)
        throw("file exists but cannot be parsed as XML");

      this.bInitialized = true;
    }
    catch(exc)
    {
      dump("Unable to load math units file \"" + this.sourceFile + "\"; exception is [" + exc + "].\n");
      return null;
    }

//    this.namesDoc = document.implementation.createDocument("", "unitnames", null);
//    var nodeList;
//    var node;
//    var s;
//    var arrayElement;
//    this.namesDoc.async = false;
//    if (this.namesDoc.load("file:///" + unitNameFile.path))
//    {
//      this.bInitialized = true;
////      this.initAutoCompleteList();
//    }
  },

//  initAutoCompleteList : function()
//  {
//    if (!this.bInitialized)
//    {
//      dump("In msiBaseMathNameList, initAutoCompleteList being called without bInitialized being set!?\n");
//      this.initialize();
//    }
//    var ACSA = Components.classes["@mozilla.org/autocomplete/search;1?name=stringarray"].getService();
//    ACSA.QueryInterface(Components.interfaces.nsIAutoCompleteSearchStringArray);
//    var nameNodesList = this.namesDoc.getElementsByTagName("mathname");
//    for (var ix = 0; ix < nameNodesList.length; ++ix)
//    {
//      var theId = nameNodesList[ix].getAttribute("id");
//      ACSA.addString("mathnames", theId);
//    }
//    ACSA.sortArrays();
//  },

  createDialogNameList : function()
  {
    if (!this.bInitialized)
      this.initialize();
    var newNameList = new Object();
    var nameNodesList = this.namesDoc.getElementsByTagName("unitname");
    for (var ix = 0; ix < nameNodesList.length; ++ix)
    {
      var theId = nameNodesList[ix].getAttribute("id");
      newNameList[theId] = this.copyDataToObject(nameNodesList[ix]);
    }
    return newNameList;
  },

  getUnitNameData : function(aName)
  {
    if (!this.bInitialized)
      this.initialize();
    var theNode = this.namesDoc.getElementById(aName);
    if (theNode !== null)
      return this.copyDataToObject(theNode);
    return null;
  },

  copyDataToObject : function(nameNode)
  {
    if (!nameNode)
    {
      dump("Error in msiEditorUtilities.js, in msiBaseMathUnitsList.copyDataToObject; null nameNode passed in!\n");
      return null;
    }
    var nameData = new Object();
    nameData.id = nameNode.getAttribute("id");
    nameData.name = nameNode.getAttribute("name");
    nameData.data = this.getUnitStrFromNode(nameNode);
    nameData.type = nameNode.getAttribute("type");
    nameData.builtIn = (nameNode.hasAttribute("builtIn") && nameNode.getAttribute("builtIn") === "true");
    var theConversionList = nameNode.getElementsByTagName("conversion");
    if (theConversionList !== null && theConversionList.length > 0)
      nameData.conversion = theConversionList[0].cloneNode(true);
    var appearanceList = nameNode.getElementsByTagName("appearance");
    if (appearanceList !== null && appearanceList.length > 0 && appearanceList[0].childNodes.length > 0)
    {
      nameData.appearance = appearanceList[0].cloneNode(true);
      if (appearanceList[0].namespaceURI !== null && appearanceList[0].namespaceURI.length > 0)
        dump("In msiBaseMathUnitsList.copyDataToObject, cloning node with namespace [" + appearanceList[0].namespaceURI + "], got one with namespace [" + nameData.appearance.namespaceURI + "].\n");
    }
    if (theConversionList !== null && theConversionList.length > 0)
      nameData.conversion = theConversionList[0].cloneNode(true);
    if (this.nameHasAutoSubstitution(nameData.id))
      nameData.autoSubstitute = true;
    return nameData;
  },

  getUnitStrFromNode : function(nameNode)
  {
    var theDataList = nameNode.getElementsByTagName("data");
    if (theDataList !== null && theDataList.length > 0)
      return theDataList.item(0).textContent;
    return '';
  },

  deleteName : function(aName)
  {
    var nameNode = this.namesDoc.getElementById(aName);
    if (nameNode !== null)
    {
      var isBuiltIn = nameNode.getAttribute("builtIn");
      if ((!isBuiltIn) || (isBuiltIn !== "true"))
      {
        var unitStr = this.getUnitStrFromNode(nameNode);
        if (this.nameHasAutoSubstitution(unitStr))
          this.removeAutoSubstitution(unitStr);
        nameNode.parentNode.removeChild(nameNode);
//        var ACSA = Components.classes["@mozilla.org/autocomplete/search;1?name=stringarray"].getService();
//        ACSA.QueryInterface(Components.interfaces.nsIAutoCompleteSearchStringArray);
//        ACSA.deleteString("unitnames", aName);
        this.bModified = true;
        return true;
      }
    }
    return false;
  },

  addName : function(aName, aNameData)
  {
    var nameNode = this.namesDoc.getElementById(aName);
    if (nameNode !== null)
    {
      var isBuiltIn = nameNode.getAttribute("builtIn");
      if (isBuiltIn && (isBuiltIn === "true"))
        return false;
      nameNode.parentNode.removeChild(nameNode);
    }
    var parentNode = this.namesDoc.getElementById("nameBase");
    var newNode = this.namesDoc.createElement("unitname");
    newNode.setAttribute("id", aName);
    newNode.setAttribute("type", aNameData.type);
    if ("conversion" in aNameData)
    {
      var newConversion = aNameData.conversion.cloneNode(true);
      newNode.appendChild(newConversion);
    }
    if (("data" in aNameData) && (aNameData.data !== null) && (aNameData.data.length > 0))
    {
      var newData = this.namesDoc.createElement("data");
      newData.appendChild(this.namesDoc.createTextNode(aNameData.data));
      newNode.appendChild(newData);
    }
    if ("appearance" in aNameData)
    {
      var newAppearance = aNameData.appearance.cloneNode(true);
      newNode.appendChild(newAppearance);
    }
    parentNode.appendChild(newNode);
//    var ACSA = Components.classes["@mozilla.org/autocomplete/search;1?name=stringarray"].getService();
//    ACSA.QueryInterface(Components.interfaces.nsIAutoCompleteSearchStringArray);
//    ACSA.addString("unitnames", aName);
//    ACSA.sortArrays();
    if (aNameData.autoSubstitute === true)
      this.addAutoSubstitution(aNameData.id);
    this.bModified = true;
    return true;
  },
//
//  updateName : function(aName, aNameData)
//  {
//    return this.addName(aName, aNameData);  //is this what we should do?
//  },

  nameHasAutoSubstitution : function(unitStr)
  {
//    if (!this.bInitialized)
//      this.initialize();
//    var theNode = this.namesDoc.getElementById(aName);
//    var unitStr = "u" + this.getUnitStrFromNode(aNode);
    unitStr = "u" + unitStr;

    var autosub = Components.classes["@mozilla.org/autosubstitute;1"].getService(Components.interfaces.msiIAutosub);
    autosub.Reset();
    var result = Components.interfaces.msiIAutosub.STATE_INIT;
    for (var ix = unitStr.length - 1; ix >= 0; --ix)
    { 
      result = autosub.nextChar(true, unitStr.charAt(ix));
      if (result === Components.interfaces.msiIAutosub.STATE_FAIL)
        return false;
    }
    return (result === Components.interfaces.msiIAutosub.STATE_SUCCESS);
  },

  addAutoSubstitution : function(unitStr)
  {
    dump("In msiBaseMathUnitsList.addAutoSubstitution for unit string " + unitStr + "\n.");
    var autosub = Components.classes["@mozilla.org/autosubstitute;1"].getService(Components.interfaces.msiIAutosub);
    var autoStr = "u" + unitStr;
    var bAdded = autosub.addEntry( autoStr, Components.interfaces.msiIAutosub.CONTEXT_MATHONLY, Components.interfaces.msiIAutosub.ACTION_EXECUTE,
                      "insertMathunit('" + unitStr + "')", "", "" );
    dump("autosub.addEntry returned " + (bAdded ? "true" : "false") + "\n.");
    this.bModifiedAutoSubs = true;
  },

  removeAutoSubstitution : function(unitStr)
  {
    var autosub = Components.classes["@mozilla.org/autosubstitute;1"].getService(Components.interfaces.msiIAutosub);
    unitStr = "u" + unitStr;
    autosub.removeEntry(unitStr);
    this.bModifiedAutoSubs = true;
  },

  updateFile : function()
  {
    dump("msiBaseMathUnitsList.updateFile being called; this.bModified is " + (this.bModified ? "true" : "false") + ".\n");
    if (this.bModified && (this.namesDoc !== null))
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
    }
    if (this.bModifiedAutoSubs)
    {
      var autosub = Components.classes["@mozilla.org/autosubstitute;1"].getService(Components.interfaces.msiIAutosub);
      var bSaved = autosub.Save();
      dump("autosub.Save() returned " + (bSaved ? "true" : "false") + "\n");
      this.bModifiedAutoSubs = false;
    }
  }
};

function msiMathUnitsList()
{
  this.names = msiBaseMathUnitsList.createDialogNameList();
  this.prepareTypeList = function()
  {
    var theTypeList = new Object();
    for (var aUnit in this.names)
    {
      if (!("type" in this.names[aUnit]))
      {
        dump("In msiMathUnitsList.prepareTypeList, no type for unit [" + aUnit + "].\n");
        continue;
      }
      var theType = this.names[aUnit].type;
      if (!(theType in theTypeList))
        theTypeList[theType] = new Array();
      theTypeList[theType].push(aUnit);
    }
    return theTypeList;
  };
  this.typeList = this.prepareTypeList();
//  this.updateBaseList = function()
//  {
//    msiBaseMathUnitsList.updateFile();
//  };
  this.canDelete = function(aName)
  {
    if ((aName in this.names) && (this.names[aName] !== null) && (!this.names[aName].builtIn))
    {
      return true;
    }
    return false;
  };
  this.canAdd = function(aName)
  {
    if ((aName in this.names) && (this.names[aName] !== null))
    {
      return (!this.names[aName].builtIn);
    }
    return true;
  };
  this.findUnitByString = function(unitStr)
  {
    for (var aName in this.names)
    {
      if (this.names[aName].data === unitStr)
        return this.names[aName];
    }
    return null;
  };
  this.hasAutoSubstitution = function(aName)
  {
    if ((aName in this.names) && ("autoSubstitute" in this.names[aName]) && (this.names[aName].autoSubstitute === true))
      return true;
    return false;
  };
  this.isBuiltIn = function(aName)
  {
    if ((aName in this.names) && (this.names[aName] !== null))
    {
      if (this.names[aName].builtIn === true)
        return true;
    }
    return false;
  };
  this.addToTypeList = function(aName, aType)
  {
    if (!("typeList" in this) || (this.typeList === null))
      this.typeList = this.prepareTypeList();
    if (!(aType in this.typeList))
      this.typeList[aType] = new Array();
    this.typeList[aType].push(aName);
  };
  this.addUnitName = function(aName, aLongName, aType, bAutoSubstitute, unitString, appearanceNode, conversionNode)
  {
    var nameData = new Object();
    nameData.id = aName;
    if (aLongName === null || aLongName.length === 0)
      aLongName = aName;
    nameData.name = aLongName;
    nameData.type = aType;
    nameData.data = unitString;
    if (appearanceNode !== null)
      nameData.appearance = appearanceNode.cloneNode(true);
    if (conversionNode !== null)
      nameData.conversion = conversionNode.cloneNode(true);
    nameData.builtIn = false;
    nameData.autoSubstitute = bAutoSubstitute;
    this.names[aName] = nameData;
    this.addToTypeList(aName, aType);
    msiBaseMathNameList.addName(aName, nameData);
  };
  this.removeFromTypeList = function(aName, aType)
  {
    if (!("typeList" in this) || (this.typeList === null))
      return;
    if (!(aType in this.typeList) || this.typeList[aType] === null)
      return;
    var nIndex = this.typeList[aType].indexOf(aName);
    if (nIndex >= 0)
      this.typeList[aType].splice(nIndex, 1);
  };
  this.deleteUnitName = function(aName)
  {
    if (this.canDelete(aName))
    {
      msiBaseMathUnitsList.deleteName(aName);
      removeFromTypeList(aName, this.names[aName].type);
      delete this.names[aName];
    }
  };
}


var msiAutosubstitutionList = 
{

  bInitialized: false,
  autosubsFilename : "autosubs.xml",
  bModified: false,
  mSubsList: null,

  loadDocument : function()
  {
    var autosubsFile = getUserResourceFile("autosubs.xml", "xml");

    var request = Components.classes["@mozilla.org/xmlextras/xmlhttprequest;1"].createInstance();
    request.QueryInterface(Components.interfaces.nsIXMLHttpRequest);
    var thePath = msiFileURLFromAbsolutePath( autosubsFile.target ).spec;
    try {
      request.open("GET", thePath, false);
      request.send(null);
                                
      var subsDoc = request.responseXML; 
      if (!subsDoc && request.responseText)
        throw("file exists but cannot be parsed as XML");

      this.bInitialized = true;
    }
    catch(exc)
    {
      dump("Unable to load autosubstitution file \"" + autosubsFile.path + "\"; exception is [" + exc + "]. Aborting Auto Substitution dialog.\n");
      return null;
    }
    return subsDoc;
  },

  initialize: function()
  {
    var subsDoc = this.loadDocument();
    if (subsDoc === null)
      return;

    this.mSubsList = new Object();
    var retVal = false;  //until we get something in the list
    // We need to prebuild these so that the keyboard shortcut works
    // ACSA = autocomplete string array
//    var ACSAService = Components.classes["@mozilla.org/autocomplete/search;1?name=stringarray"].getService();
//    ACSAService.QueryInterface(Components.interfaces.nsIAutoCompleteSearchStringArray);
//    var ACSA = ACSAService.getGlobalSearchStringArray();
    var ACSA = msiSearchStringManager.setACSAImpGetService();
  
    var rootElementList = subsDoc.getElementsByTagName("subs");
    dump("In msiAutoSubstitutionList.initialize(), subsDoc loaded, rootElementList has length [" + rootElementList.length + "].\n");
    var nameNodesList = null;
    if (rootElementList.length > 0)
      nameNodesList = rootElementList[0].getElementsByTagName("sub");
    else
      nameNodesList = subsDoc.getElementsByTagName("sub");
    dump("In autoSubstituteDialog.js, in createSubstitutionList(), subsDoc loaded, nameNodesList has length [" + nameNodesList.length + "].\n");
    if (nameNodesList.length === 0)
      AlertWithTitle("autoSubstituteDialog.js", "Empty nameNodesList returned.");
    var thePattern = "";
    var stringType = "";
    var newObject = null;
  //  var unitRegExp = /insertMathunit\([\'\"]([^\'\"]+)[\'\"]\);/;
  //  var mathnameRegExp = /insertMathname\([\'\"]([^\'\"]+)[\'\"]\);/;
    var nAddedToList = 0;
    for (var ix = 0; ix < nameNodesList.length; ++ix)
    {
      thePattern = nameNodesList[ix].getElementsByTagName("pattern").item(0).textContent;
      if (thePattern !== null && thePattern.length > 0)
      {
        newObject = new Object();
        stringType = nameNodesList[ix].getAttribute("tp");
        newObject.mathContext = nameNodesList[ix].getAttribute("ctx");
        newObject.theData = nameNodesList[ix].getElementsByTagName("data").item(0).textContent;
        if (stringType === "sc")
        {
          newObject.theContext = "";
          newObject.theInfo = "";
        }
        else
        {
          newObject.theContext = nameNodesList[ix].getElementsByTagName("context").item(0).textContent;
          newObject.theInfo = nameNodesList[ix].getElementsByTagName("info").item(0).textContent;
        }
        if (stringType === "sc")
        {
          newObject.type = "script";
        }
        else if (stringType === "subst")
          newObject.type = "substitution";
        else
          newObject.type = "";

        if (newObject.type.length > 0)
        {
          if (ACSA.addString("autosubstitution", thePattern))
            ++nAddedToList;
          this.mSubsList[thePattern] = newObject;
          retVal = true;
  //        dump("In autoSubstituteDialog.createSubstitutionList, adding pattern [" + thePattern + "] with data [" + newObject.theData + "].\n");
        }
      }
    }
    ACSA.sortArrays();
    dump("In msiAutosubstitutionList.createDialogSubstitutionList, added " + nAddedToList + " elements to autocomplete array.\n");
//    return theSubsList;
  },

  createDialogSubstitutionList: function()
  {
    var returnList = new Object();
    if (!this.bInitialized)
      this.initialize();
    for (var aPattern in this.mSubsList)
    {
      returnList[aPattern] = this.mSubsList[aPattern];
    }
    return returnList;
  },

  setUpTextBoxControl : function(theControl)
  {
    var currStr = "";
    if (theControl.hasAttribute("onfocus"))
      currStr = theControl.getAttribute("onfocus");
    theControl.setAttribute("onfocus", "msiSearchStringManager.setACSAImp();" + currStr);
  }
};

var msiSearchStringManager = 
{
  mDocumentArrays : [],
  baseString : "",

  getSearchStringArrayRecordByName : function(aDocument, aName)
  {
    return this.getSearchStringArrayRecordImp(aDocument, {mString : aName});
  },

  getSearchStringArrayRecordForControl : function(aControl)  //Here as below, "aControl" is most likely a dialog window, but...
  {
    var topEditorElement = msiGetTopLevelEditorElement(aControl);
    var topEditor = msiGetEditor(topEditorElement);
    var aSubIdent = {mControl : aControl};
    return this.getSearchStringArrayRecordImp(topEditor.document, aSubIdent);
  },

  getSearchStringIDByName : function(aDocument, aName)
  {
    var aRecord = this.getSearchStringArrayRecordByName(aDocument, aName);
    return aRecord.mKey;
  },

  getSearchStringIDForControl : function(aControl)
  {
    var aRecord = this.getSearchStringArrayRecordForControl(aControl);
    return aRecord.mKey;
  },

  setACSAImpGetService : function()
  {
    var ACSAService = Components.classes["@mozilla.org/autocomplete/search;1?name=stringarray"].getService();
    ACSAService.QueryInterface(Components.interfaces.nsIAutoCompleteSearchStringArray);
    var ACSAImp = this.getGlobalACSAImp();
    if (!ACSAImp)
    {
      ACSAImp = ACSAService.getNewImplementation();
      this.setGlobalACSAImp(ACSAImp);
    }
    ACSAService.setImplementation(ACSAImp);
    return ACSAService;
  },

  setACSAImp : function()
  {
    this.setACSAImpGetService();
  },

  getGlobalACSAImp : function()
  {
    var theWindow = msiGetTopLevelWindow();
    if (theWindow && ("mGeneralACSAImp" in theWindow))
      return theWindow.mGeneralACSAImp;
    return null;
  },

  setGlobalACSAImp : function(ACSAImp)
  {
    var theWindow = msiGetTopLevelWindow();
    if (!theWindow)
      theWindow = window;
    theWindow.mGeneralACSAImp = ACSAImp;
  },

  removeSearchStringArrayRecord : function(aRecord)
  {
    var theDocRecord = this.getRecordForDocument(aRecord.mDocument);
    this.setACSAImp();
//    var ACSA = Components.classes["@mozilla.org/autocomplete/search;1?name=stringarray"].getService();
//    ACSA.QueryInterface(Components.interfaces.nsIAutoCompleteSearchStringArray);
    var ACSA = this.setACSAImpGetService();
    ACSA.resetArray(aRecord.mKey);
    var jx = theDocRecord.mStringArrays.indexOf(aRecord);
    if (jx >= 0)
      theDocRecord.mStringArrays.splice(jx, 1);
    if (!theDocRecord.mStringArrays.length)
      this.removeDocumentRecord(theDocRecord.mKey);
  },

  removeDocumentRecord : function(docKey)
  {
    delete this.mDocumentArrays[docKey];
  },

//Internal implementation
  getSearchStringArrayRecordImp : function(aDocument, aSubIdent)
  {
    var docRecord = this.getRecordForDocument(aDocument);
    var controlRecord = null;
    if (docRecord)
    {
      for (var ix = 0; ix < docRecord.mStringArrays.length; ++ix)
      {
        if (this.equalSubIdents(docRecord.mStringArrays[ix].mIdent, aSubIdent))
        {
          controlRecord = docRecord.mStringArrays[ix];
          break;
        }
      }
      if (!controlRecord)
        controlRecord = this.addSearchStringArrayRecordImp(docRecord, aSubIdent);
    }
    return controlRecord;
  },

  addSearchStringArrayRecordImp : function(aDocRecord, aSubIdent)
  {
    var theBaseString = this.baseString.concat("-", aDocRecord.mKey);
    var subIdentStr = this.getSubIdentBaseString(aSubIdent);
    theBaseString = theBaseString.concat("-", subIdentStr);
//    var ACSA = Components.classes["@mozilla.org/autocomplete/search;1?name=stringarray"].getService();
//    ACSA.QueryInterface(Components.interfaces.nsIAutoCompleteSearchStringArray);
    var ACSA = this.setACSAImpGetService();
    var theString = theBaseString;
    var nSize = ACSA.sizeofArray(theString);
    for (var jj=0; (jj < 100) && (nSize >= 0); ++jj)
    {
      theString = theBaseString + String(jj);
      nSize = ACSA.sizeofArray(theString);
    }
    if (nSize >= 0)
    {
      dump("Problem adding record to msiSearchStringManager for document [" + theTitle + "]\n");
      var theDate = new Date();
      theString = theBaseString + theDate.toString();
    }
    var newControlRecord = {mIdent : aSubIdent, mKey : theString, mDocument : aDocRecord.mDocument};
    aDocRecord.mStringArrays.push(newControlRecord);
    return newControlRecord;
  },

  getSubIdentBaseString : function(aSubIdent)
  {
    if ("mString" in aSubIdent)
      return aSubIdent.mString;
    var retStr = "";
    if ("mControl" in aSubIdent)
    {
      var controlStr = null;
      if ("title" in aSubIdent.mControl)
        controlStr = aSubIdent.mControl.title;
      if ((!controlStr || !controlStr.length) && ("id" in aSubIdent.mControl))
        controlStr = aSubIdent.mControl.id;
      if ((!controlStr || !controlStr.length) && ("nodeName" in aSubIdent.mControl))
        controlStr = aSubIdent.mControl.nodeName;
      if (controlStr)
        retStr = ReplaceWhitespace(controlStr, "");
    }
    return retStr;
  },

  equalSubIdents : function(firstIdent, secondIdent)
  {
    if ( ("mString" in firstIdent) && ("mString" in secondIdent) )
      return (!firstIdent.mString.localeCompare(secondIdent.mString));
    else if ( ("mControl" in firstIdent) && ("mControl" in secondIdent) )
      return firstIdent.mControl === secondIdent.mControl;
    return false;
  },

  addDocumentRecord : function(aDocument)
  {
    var theTitle = aDocument.title;
    var theString = "Unnamed";
    if (theTitle && theTitle.length)
    {
      theTitle = ReplaceWhitespace(theTitle, "");
      if (theTitle.length > 12)
        theString = theTitle.substr(0, 12);
      else
        theString = theTitle;
    }
    var baseString = theString;
    
    for (var ix = 0; (ix < 1000) && (theString in this.mDocumentArrays); ++ix)
    {
      theString = baseString + String(++ix);
    }
    if (ix === 1000)
    {
      dump("Problem adding record to msiSearchStringManager for document [" + theTitle + "]\n");
      return null;
    }
    this.mDocumentArrays[theString] = {mKey : theString, mDocument : aDocument, mStringArrays : []};
    return theString;
  },

  getRecordForDocument : function(aDocument)
  {
    var docRecord = this.findRecordForDocument(aDocument);
    if (!docRecord)
    {
      var docRecordKey = this.addDocumentRecord(aDocument);
      if (docRecordKey)
        docRecord = this.mDocumentArrays[docRecordKey];
    }
    return docRecord;
  },

  findRecordForDocument : function(aDocument)
  {
    var retVal = null;
    for (var aRec in this.mDocumentArrays)
    {
      if (this.mDocumentArrays[aRec].mDocument === aDocument)
      {
        retVal = this.mDocumentArrays[aRec];
        break;
      }
    }
    return retVal;
  }

};

var msiKeyListManager =
{
  baseString : "keys",
//  mXPathStr : xsltSheetForKeyAttrib,
  mDocumentArrays : new Object(),

  checkChangesAgainstDocument : function(aControlRecord, addStringsArray, deleteStringsArray)
  {
//    var aControlRecord = this.getSearchStringArrayRecordForControl(aControl);
    var currDocKeys = this.getMarkerStringList(aControlRecord.mDocument, true);
    var duplicateArray = [];
    for (var ix = 0; ix < addStringsArray; ++ix)
    {
      if (currDocKeys.indexOf(addStringsArray[ix]) >= 0)
        duplicateArray.push(addStringsArray[ix]);
    }
    return duplicateArray;
  },

  initMarkerListForControl : function(aControl, bForce)
  {
    var aControlRecord = this.getSearchStringArrayRecordForControl(aControl);
    var docRecord = this.getRecordForDocument(aControlRecord.mDocument);
    var editorElement = msiGetTopLevelEditorElement(aControl);
    if (editorElement)
      docRecord.mEditor = msiGetEditor(editorElement);
    return this.initMarkerList(aControlRecord, bForce);
  },

  initMarkerList : function(aControlRecord, bForce)
  {
    return this.initMarkerListForDocument(aControlRecord, aControlRecord.mDocument, bForce);
  },

  initMarkerListForDocument : function(aControlRecord, aDocument, bForce)
  {
    var retVal = false;
    try
    {
//      var ACSA = Components.classes["@mozilla.org/autocomplete/search;1?name=stringarray"].getService();
//      ACSA.QueryInterface(Components.interfaces.nsIAutoCompleteSearchStringArray);
      var ACSA = this.setACSAImpGetService();
      var currDocKeys = this.getMarkerStringList(aDocument, bForce);
      for (var ix = 0; ix < currDocKeys.length; ++ix)
      {
        if (currDocKeys[ix].length > 0)
          ACSA.addString(aControlRecord.mKey, currDocKeys[ix]);
      }
      retVal = true;
    }
    catch(exc) {dump("Exception in msiKeyListManager.initMarkerList! Error is [" + exc + "]\n");}
    return retVal;
  },

  resetMarkerListForControl : function(aControl, bForce) 
  {
    var aControlRecord = this.getSearchStringArrayRecordForControl(aControl);
    return this.resetMarkerList(aControlRecord, bForce);
  },

  resetMarkerList : function(aControlRecord, bForce)
  {
    try
    {
      if (aControlRecord)
      {
//        var ACSA = Components.classes["@mozilla.org/autocomplete/search;1?name=stringarray"].getService();
//        ACSA.QueryInterface(Components.interfaces.nsIAutoCompleteSearchStringArray);
        var ACSA = this.setACSAImpGetService();
        ACSA.resetArray(aControlRecord.mKey);
        return this.initMarkerList(aControlRecord, bForce);
      }
    }
    catch(exc) {dump("Exception in msiKeyListManager.resetMarkerList! Error is [" + exc + "]\n");}
    return false;
  },

  clearMarkerList : function(aControlRecord)
  {
    try
    {
      if (aControlRecord)
      {
//        var ACSA = Components.classes["@mozilla.org/autocomplete/search;1?name=stringarray"].getService();
//        ACSA.QueryInterface(Components.interfaces.nsIAutoCompleteSearchStringArray);
        var ACSA = this.setACSAImpGetService();
        ACSA.resetArray(aControlRecord.mKey);
        return true;
      }
    }
    catch(exc) {dump("Exception in msiKeyListManager.clearMarkerList! Error is [" + exc + "]\n");}
    return false;
  },

  getMarkerStringList : function(aDocument, bForce)
  {
    
    var docRecord = this.getRecordForDocument(aDocument);
    return this.updateMarkerList(docRecord, bForce);
  },

  updateMarkerList : function(aDocRecord, bForce)
  {
    if (bForce || this.needsMarkerListRefresh(aDocRecord))
    {
      if (!("mEditor" in aDocRecord))
      {
        var editorElement = msiGetTopLevelEditorElement(window);
        if (editorElement)
          aDocRecord.mEditor = msiGetEditor(editorElement);
      }
      aDocRecord.markerList = msiGetKeyListForDocument(aDocRecord.mDocument, aDocRecord.mEditor);
      aDocRecord.bDocModified = false;
    }
    return aDocRecord.markerList;
  },

  needsMarkerListRefresh : function(aDocRecord)
  {
    if (("markerList" in aDocRecord)  && ("bDocModified" in aDocRecord) && (!aDocRecord.bDocModified))
      return false;
    return true;
  }
};

msiKeyListManager.__proto__ = msiSearchStringManager;

var msiMarkerListPrototype =
{
//  mbInitialized : false,
//  mDocument : null,
//  mKeyListManagerRecord : null,

//  setForDocument : function(aDocument)
//  {
//    mDocument = aDocument;
//    this.resetList();
//  },

  checkAllChanges : function()
  {
    return this.mKeyListManager.checkChangesAgainstDocument(this.mKeyListManagerRecord, this.mAddedElements, this.mDeletedElements);
  },

  checkChanges : function(addedStringArray, deletedStringArray)
  {
    return this.mKeyListManager.checkChangesAgainstDocument(this.mKeyListManagerRecord, addedStringArray, deletedStringArray);
  },

  resetList : function(bForce)
  {
    return this.mKeyListManager.resetMarkerList(this.mKeyListManagerRecord);
//    var aDocument = this.getDocument();
//    if (!aDocument)
//    {
//      dump("In msiEditorUtilities.js, msiMarkerList.resetList() called with no current document! Returning...\n");
//      return;
//    }
//    var keyStrings = msiGetKeyListForDocument(aDocument);
//    var ourKey = this.getIndexString();
//    var ACSA = Components.classes["@mozilla.org/autocomplete/search;1?name=stringarray"].getService();
//    ACSA.QueryInterface(Components.interfaces.nsIAutoCompleteSearchStringArray);
//    var ourKey = this.getIndexString();
//    ACSA.resetArray(ourKey);
//    for (i=0, len=keyStrings.length; i<len; i++)
//    {
//      if (keyStrings[i].length > 0) 
//        ACSA.addString(ourKey, keyStrings[i]);
//    }
  },

//  getMarkerList : function(aDocument)
//  {
//    if (this.mDocument !== aDocument)
//      this.setForDocument(aDocument);
//  },

  clearList : function()
  {
    return this.mKeyListManager.clearMarkerList(this.mKeyListManagerRecord);
  },

  changeSourceDocument : function(aDocument)
  {
    this.clearList();
    this.mDeletedItems = [];
    this.mAddedItems = [];
    this.mTargetDocument = aDocument;
    return this.mKeyListManager.initMarkerListForDocument(this.mKeyListManagerRecord, aDocument, true);
  },

  getIndexString : function()
  {
    if (this.mKeyListManagerRecord)
      return this.mKeyListManagerRecord.mKey;
    return "";
  },

  getDocument : function()
  {
    if ("mTargetDocument" in this)
      return this.mTargetDocument;
    if (this.mKeyListManagerRecord)
      return this.mKeyListManagerRecord.mDocument;
    return null;
  },

//  getMarkerListForEditorElement : function(editorElement)
//  {
//    if (!editorElement)
//      editorElement = msiGetActiveEditorElement();
//    var editor = msiGetEditor(editorElement);
//    if (!editor)
//    {
//      dump("In msiEditorUtilities.js, msiMarkerList.getMarkerListForEditorElement() called with no active editor! Returning...\n");
//      return;
//    }
//    this.getMarkerList(editor.document);
//  },

  addString : function(aString)
  {
//    var ACSA = Components.classes["@mozilla.org/autocomplete/search;1?name=stringarray"].getService();
//    ACSA.QueryInterface(Components.interfaces.nsIAutoCompleteSearchStringArray);
    var ACSA = msiSearchStringManager.setACSAImpGetService();
    var retVal = ACSA.addString( this.getIndexString(), aString);
    if (retVal)
    {
      var ix = this.mDeletedElements.indexOf(aString);
      if (ix >= 0)
        this.mDeletedElements.splice(ix, 1);
      else
        this.mAddedElements.push(aString);
    }
    return retVal;
  },

  deleteString : function(aString)
  {
//    var ACSA = Components.classes["@mozilla.org/autocomplete/search;1?name=stringarray"].getService();
//    ACSA.QueryInterface(Components.interfaces.nsIAutoCompleteSearchStringArray);
    var ACSA = msiSearchStringManager.setACSAImpGetService();
    var retVal = ACSA.deleteString( this.getIndexString(), aString);
    if (retVal)
    {
      var ix = this.mAddedElements.indexOf(aString);
      if (ix >= 0)
        this.mAddedElements.splice(ix, 1);
      else
        this.mDeletedElements.push(aString);
    }
    return retVal;
  },

  changeString : function(oldString, newString)
  {
    var retVal = true;
    if (oldString)
      retVal = this.deleteString(oldString);
    if (newString)
      retVal = this.addString(newString) && retVal;  //Anti-lazy evaluation!
    return retVal;
  },

  detach : function()
  {
    this.mKeyListManager.removeSearchStringArrayRecord(this.mKeyListManagerRecord);
    this.mbInitialized = false;
    this.mKeyListManagerRecord = null;
  },

  setACSAImp : function()
  {
    msiSearchStringManager.setACSAImpGetService();
  },

  setUpTextBoxControl : function(theControl)
  {
    theControl.markerList = this;
    var currStr = "";
    if (theControl.hasAttribute("onfocus"))
      currStr = theControl.getAttribute("onfocus");
    theControl.setAttribute("onfocus", "msiSearchStringManager.setACSAImp();" + currStr);
    theControl.setAttribute("autocompletesearchparam", this.getIndexString());
  }

};

function msiKeyMarkerList(aControl) 
{
  this.mControl = aControl;
  this.mKeyListManager = msiKeyListManager;
  this.mKeyListManagerRecord = msiKeyListManager.getSearchStringArrayRecordForControl(aControl);
  this.mbInitialized = msiKeyListManager.initMarkerList(this.mKeyListManagerRecord);
  this.mAddedElements = [];
  this.mDeletedElements = [];
}

msiKeyMarkerList.prototype = msiMarkerListPrototype;

var msiBibItemKeyListManager =
{
  baseString : "bibitemkeys",
//  mXPathStr : xsltSheetForKeyAttrib,
  mDocumentArrays : new Object(),

  updateMarkerList : function(aDocRecord, bForce)
  {
    if (bForce || this.needsMarkerListRefresh(aDocRecord))
    {
      aDocRecord.markerList = msiGetBibItemKeyListForDocument(aDocRecord.mDocument);
      aDocRecord.bDocModified = false;
    }
    return aDocRecord.markerList;
  }

};

msiBibItemKeyListManager.__proto__ = msiKeyListManager;

function msiBibItemKeyMarkerList(aControl) 
{
  this.mControl = aControl;
  this.mKeyListManager = msiBibItemKeyListManager;
  this.mKeyListManagerRecord = msiBibItemKeyListManager.getSearchStringArrayRecordForControl(aControl);
  this.mbInitialized = msiBibItemKeyListManager.initMarkerList(this.mKeyListManagerRecord);
  this.mAddedElements = [];
  this.mDeletedElements = [];
}

msiBibItemKeyMarkerList.prototype = msiMarkerListPrototype;

function msiGetKeyListForDocument(aDocument, editor)
{
//  var parser = new DOMParser();
//  var dom = parser.parseFromString(xsltSheetForKeyAttrib, "text/xml");
//  dump(dom.documentElement.nodeName === "parsererror" ? "error while parsing" + dom.documentElement.textContent : dom.documentElement.nodeName);
//  var processor = new XSLTProcessor();
//  processor.importStylesheet(dom.documentElement);
//  var newDoc;
//  if (aDocument)
//    newDoc = processor.transformToDocument(aDocument, document);
//  dump(newDoc.documentElement.localName+"\n");
//  var keyString = newDoc.documentElement.textContent;
//  var keys = keyString.split(/\n+/);
//  var i;
//  var len;
//  keys.sort();
//  var lastkey = "";
//  for (i=keys.length-1; i >= 0; i--)
//  {
//    if (keys[i] === "" || keys[i] === lastkey) keys.splice(i,1);
//    else lastkey = keys[i];
//  }  
//  dump("Keys are : "+keys.join()+"\n");    
//  return keys;
	var ignoreIdsList = "section--subsection--subsubsection--part--chapter";
  if (editor)
    ignoreIdsList = editor.tagListManager.getTagsInClass("structtag","--", false);
	ignoreIdsList = "--" + ignoreIdsList + "--";
  var xsltSheetForKeyAttrib = "<?xml version='1.0'?><xsl:stylesheet version='1.1' xmlns:xsl='http://www.w3.org/1999/XSL/Transform' xmlns:html='http://www.w3.org/1999/xhtml' xmlns:mathml='http://www.w3.org/1998/Math/MathML' ><xsl:output method='text' encoding='UTF-8'/><xsl:variable name='hyphen'>--</xsl:variable>";
  xsltSheetForKeyAttrib += "<xsl:variable name='ignoreIDs'>" + ignoreIdsList + "</xsl:variable><xsl:variable name='xrefName'>xref</xsl:variable>";
  xsltSheetForKeyAttrib += "<xsl:template match='/'>  <xsl:apply-templates select='//*[@key][not(local-name()=$xrefName)]|//*[@id]|//mathml:mtable//*[@marker]|//mathml:mtable//*[@customLabel]'/></xsl:template>\
                               <xsl:template match='//*[@key]|//*[@id]|//mathml:mtable//*[@marker]|//mathml:mtable//*[@customLabel]'>\
                                 <xsl:choose><xsl:when test='@key and not(local-name()=$xrefName)'><xsl:value-of select='@key'/><xsl:text>\n</xsl:text></xsl:when>\
                                             <xsl:when test='@marker and not(@key and @key=@marker)'><xsl:value-of select='@marker'/><xsl:text>\n</xsl:text></xsl:when>\
                                             <xsl:when test='@id and not(contains($ignoreIDs,concat($hyphen,local-name(),$hyphen))) and not(@key and @key=@id) and not(@marker and @marker=@id)'><xsl:value-of select='@id'/><xsl:text>\n</xsl:text></xsl:when>\
                                             <xsl:when test='@customLabel and not(@key and @key=@customLabel) and not(@marker and @marker=@customLabel) and not (@id and @id=@customLabel)'><xsl:value-of select='@customLabel'/><xsl:text>\n</xsl:text></xsl:when>\
                               </xsl:choose></xsl:template> </xsl:stylesheet>";
  var sepRE = /\n+/;
  return msiGetItemListForDocumentFromXSLTemplate(aDocument, xsltSheetForKeyAttrib, sepRE, true);
}

function msiGetBibItemKeyListForDocument(aDocument)
{
  var xsltSheetForBibItemKeyAttrib = "<?xml version='1.0'?><xsl:stylesheet version='1.1' xmlns:xsl='http://www.w3.org/1999/XSL/Transform' xmlns:html='http://www.w3.org/1999/xhtml' ><xsl:output method='text' encoding='UTF-8'/> <xsl:template match='/'>  <xsl:apply-templates select='//*[@bibitemkey]'/></xsl:template><xsl:template match='//*[@bibitemkey]'>   <xsl:value-of select='@bibitemkey'/><xsl:text>\n</xsl:text></xsl:template> </xsl:stylesheet>";
  var sepRE = /\n+/;
  return msiGetItemListForDocumentFromXSLTemplate(aDocument, xsltSheetForBibItemKeyAttrib, sepRE, true);
}

function msiGetItemListForDocumentFromXSLTemplate(aDocument, aTemplate, separatorRegExpr, bSort)
{
  var parser = new DOMParser();
  var dom = parser.parseFromString(aTemplate, "text/xml");
  dump(dom.documentElement.nodeName === "parsererror" ? "error while parsing" + dom.documentElement.textContent : dom.documentElement.nodeName);
  var processor = new XSLTProcessor();
  processor.importStylesheet(dom.documentElement);
  var newDoc;
  if (aDocument)
    newDoc = processor.transformToDocument(aDocument, document);
  dump(newDoc.documentElement.localName+"\n");
  var keyString = newDoc.documentElement.textContent;
  var items = keyString.split(separatorRegExpr);
  var i;
  var len;
  if (bSort)
    items.sort();
  var lastitem = "";
  for (i=items.length-1; i >= 0; i--)
  {
    if (items[i] === "" || items[i] === lastitem) items.splice(i,1);
    else lastitem = items[i];
  }  
  dump("Keys are : "+items.join()+"\n");    
  return items;
}

var msiTheoremEnvListManager = 
{
  baseString : "theoremenv",

//  checkChangesAgainstDocument : function(aControlRecord, addStringsArray, deleteStringsArray)
//  {
////    var aControlRecord = this.getSearchStringArrayRecordForControl(aControl);
//    var currDocKeys = this.getMarkerStringList(aControlRecord.mDocument, true);
//    var duplicateArray = [];
//    for (var ix = 0; ix < addStringsArray; ++ix)
//    {
//      if (currDocKeys.indexOf(addStringsArray[ix]) >= 0)
//        duplicateArray.push(addStringsArray[ix]);
//    }
//    return duplicateArray;
//  },

//The following redundancy is almost certainly not useful!
//  initTheoremListForControl : function(aControl, bForce)
//  {
//    var aControlRecord = this.getSearchStringArrayRecordForControl(aControl);
//    return this.initTheoremList(aControlRecord, bForce);
//  },

  getGenericTheoremListRecordForDocument : function(aDocument)
  {
    return this.getSearchStringArrayRecordByName(aDocument, "docTheoremManager");
  },

//The following redundancy is almost certainly not useful! The callers will be holding records and
//  will only call initTheoremList below.
//  initGenericTheoremListForDoc : function(aDocument, bForce)
//  {
//    var aControlRecord = this.getSearchStringArrayRecordByName(aDocument, "docTheoremManager");
//    return this.initTheoremList(aControlRecord, bForce);
//  },

  markDocumentModifiedForRecord : function(aControlRecord)
  {
    var docRecord = this.getRecordForDocument(aControlRecord.mDocument);
    docRecord.bDocModified = true;
  },

  initTheoremList : function(aControlRecord, bForce)
  {
    var retVal = false;
    try
    {
//      var ACSA = Components.classes["@mozilla.org/autocomplete/search;1?name=stringarray"].getService();
//      ACSA.QueryInterface(Components.interfaces.nsIAutoCompleteSearchStringArray);
      var currDocThmEnvs = this.getTheoremList(aControlRecord.mDocument, bForce);
//      for (var ix = 0; ix < currDocKeys.length; ++ix)
//      {
//        if (currDocKeys[ix].length > 0)
//          ACSA.addString(aControlRecord.mKey, currDocKeys[ix]);
//      }
      retVal = true;
    }
    catch(exc) {dump("Exception in msiTheoremEnvListManager.initTheoremList! Error is [" + exc + "]\n");}
    return retVal;
  },

  resetTheoremListForControl : function(aControl, bForce) 
  {
    var aControlRecord = this.getSearchStringArrayRecordForControl(aControl);
    return this.resetTheoremList(aControlRecord, bForce);
  },

  resetGenericTheoremListForDoc : function(aDocument, bForce) 
  {
    var aControlRecord = this.getSearchStringArrayRecordByName(aDocument, "docTheoremManager");
    return this.resetTheoremList(aControlRecord, bForce);
  },

  resetTheoremList : function(aControlRecord, bForce)
  {
    try
    {
      if (aControlRecord)
      {
//        var ACSA = Components.classes["@mozilla.org/autocomplete/search;1?name=stringarray"].getService();
//        ACSA.QueryInterface(Components.interfaces.nsIAutoCompleteSearchStringArray);
//        ACSA.resetArray(aControlRecord.mKey);
        return this.initTheoremList(aControlRecord, bForce);
      }
    }
    catch(exc) {dump("Exception in msiTheoremEnvListManager.resetTheoremList! Error is [" + exc + "]\n");}
    return false;
  },

  getTheoremList : function(aDocument, bForce)
  {
    var docRecord = this.getRecordForDocument(aDocument);
    return this.updateTheoremList(docRecord, bForce);
  },

  updateTheoremListForRecord : function(aControlRecord, bForce)
  {
    var aDocRecord = this.getRecordForDocument(aControlRecord.mDocument);
    return this.updateTheoremList(aDocRecord, bForce);
  },

  updateTheoremList : function(aDocRecord, bForce)
  {
    if (bForce || this.needsTheoremListRefresh(aDocRecord))
    {
      aDocRecord.thmList = msiGetNewTheoremListFromDocument(aDocRecord.mDocument);
      aDocRecord.bDocModified = false;
    }
    return aDocRecord.thmList;
  },

  needsTheoremListRefresh : function(aDocRecord)
  {
    if (("thmList" in aDocRecord)  && ("bDocModified" in aDocRecord) && (!aDocRecord.bDocModified))
      return false;
    return true;
  }
};

msiTheoremEnvListManager.__proto__ = msiSearchStringManager;

var msiTheoremListPrototype =
{
//  mbInitialized : false,
//  mDocument : null,
//  mKeyListManagerRecord : null,

//  setForDocument : function(aDocument)
//  {
//    mDocument = aDocument;
//    this.resetList();
//  },

//  checkAllChanges : function()
//  {
//    return this.mKeyListManager.checkChangesAgainstDocument(this.mKeyListManagerRecord, this.mAddedElements, this.mDeletedElements);
//  },

//  checkChanges : function(addedStringArray, deletedStringArray)
//  {
//    return this.mKeyListManager.checkChangesAgainstDocument(this.mKeyListManagerRecord, addedStringArray, deletedStringArray);
//  },

  resetList : function(bForce)
  {
    return this.mTheoremListManager.resetTheoremList(this.mTheoremListManagerRecord);
  },

  getIndexString : function()
  {
    if (this.mTheoremListManagerRecord)
      return this.mTheoremListManagerRecord.mKey;
    return "";
  },

  getDocument : function()
  {
    if (this.mTheoremListManagerRecord)
      return this.mTheoremListManagerRecord.mDocument;
    return null;
  },

  markDocModified : function()
  {
    this.mTheoremListManager.markDocumentModifiedForRecord(this.mTheoremListManagerRecord);
  },

//Structure of theorem list is:
//  {defaultNumbering : "self", defaultStyle : "theorem", theoremArray : []};
//where each member of theoremArray looks like:     
//  {tagName : theTagName, numbering : theNumbering, bDefault : isDefault, thmStyle : theThmStyle};
  getTheoremList : function(bForceUpdate)
  {
    return this.mTheoremListManager.updateTheoremListForRecord(this.mTheoremListManagerRecord, bForceUpdate);
  },

  tagIsTheoremEnv : function(aTag)
  {
    var thmList = this.getTheoremList();
    for (var ix = 0; ix < thmList.theoremArray.length; ++ix)
    {
      if (thmList.theoremArray[ix].tagName === aTag)
        return true;
    }
    return false;
  },

  getTheoremEnvInfoForTag : function(aTag)
  {
    var thmList = this.getTheoremList();
    for (var ix = 0; ix < thmList.theoremArray.length; ++ix)
    {
      if (thmList.theoremArray[ix].tagName === aTag)
        return thmList.theoremArray[ix];
    }
    return null;
  },

  getDefaultTheoremEnvNumbering : function()
  {
    var thmList = this.getTheoremList();
    if ("defaultNumbering" in thmList)
      return thmList.defaultNumbering;
    return "self";
  },

  getDefaultTheoremEnvNumberWithin : function()
  {
    var thmList = this.getTheoremList();
    if ("defaultNumberWithin" in thmList)
      return thmList.defaultNumberWithin;
    return "section";
  },

//  getMarkerListForEditorElement : function(editorElement)
//  {
//    if (!editorElement)
//      editorElement = msiGetActiveEditorElement();
//    var editor = msiGetEditor(editorElement);
//    if (!editor)
//    {
//      dump("In msiEditorUtilities.js, msiMarkerList.getMarkerListForEditorElement() called with no active editor! Returning...\n");
//      return;
//    }
//    this.getMarkerList(editor.document);
//  },

  getDefaultForTag : function(envtag, attr)
  {
    var thmInfo = this.getTheoremEnvInfoForTag(envtag);
    if (thmInfo && (attr in thmInfo))
      return thmInfo[attr];
    return null;
  },

  changeDefaultForTag : function(envtag, oldNumbering, newNumbering, oldStyle, newStyle)
  {
    this.changeNewTheoremDeclarations(envtag, oldNumbering, newNumbering, oldStyle, newStyle);
    this.changeCSSDeclarations(envtag, oldNumbering, newNumbering, oldStyle, newStyle);
  },

  changeNewTheoremDeclarations : function(envtag, oldNumbering, newNumbering, oldStyle, newStyle)
  {
    var theDoc = this.getDocument();
    var newThmList = theDoc.getElementsByTagName("newtheorem");
    var envNodes = [];
    var envNode;
    var numbering, style, ix;
    var theLabelStr;
    for (ix = 0; ix < newThmList.length; ++ix)
    {
      envNode = newThmList[ix];
      if (this.newTheoremNodeRepresentsTag(envNode, envtag))
      {
        envNodes.push(envNode);
      }
      if ((envNode.getAttribute("name") === envtag))
        theLabelStr = envNode.getAttribute("label");
    }
    if (!theLabelStr || !theLabelStr.length)
      theLabelStr = envtag.substr(0,1).toUpperCase() + envtag.substr(1);
    if (oldNumbering === newNumbering) //the easy case - just change the style attribute for each def node
    {
      for (ix = 0; ix < envNodes.length; ++ix)
      {
        envNodes[ix].setAttribute("theoremstyle", newStyle);
        if (newStyle !== "plain")
          envNodes[ix].setAttribute("req", "amsthm");
      }
    }
    else  //replace the nodes altogether
    {
      var theParent, insertBeforeNode, newThmNode;
      for (ix = envNodes.length - 1; ix >= 0; --ix) //Go in reverse so theParent will be the first parent when we're done (not that they should be different)
      {
        theParent = envNodes[ix].parentNode;
        if (ix === 0)
          insertBeforeNode = envNodes[ix].nextSibling;
        theParent.removeChild(envNodes[ix]);
      }
      if (theParent)
      {
        newThmNode = theDoc.createElementNS(xhtmlns, "newtheorem");
        newThmNode.setAttribute("name", envtag);
        newThmNode.setAttribute("theoremstyle", newStyle);
        newThmNode.setAttribute("counter", newNumbering);
        newThmNode.setAttribute("label", theLabelStr);
        if ((newNumbering === "none") || (newStyle !== "plain"))
          newThmNode.setAttribute("req", "amsthm");
        theParent.insertBefore(newThmNode, insertBeforeNode);
      }
    }
    this.markDocModified();
  },

  changeCSSDeclarations : function(envtag, oldNumbering, newNumbering, oldStyle, newStyle)
  {
    var theDoc = this.getDocument();
    var thmEnvArray = [];
    var labelArray = [];
    var thmEnvSelStr = envtag;
    var labelSelStr = envtag + " > *:first-child:before";
    var tagLabelStr = this.getDefaultForTag(envtag, "label");
    if (!tagLabelStr || !tagLabelStr.length)
      tagLabelStr = envtag.substr(0,1).toUpperCase() + envtag.substr(1);  //unless this is available otherwise?
    var labelContent = "'" + tagLabelStr + " '";
    var outerCounterStr = "section";

    if (oldNumbering !== newNumbering)
    {
      if (newNumbering !== "none")
      {
        if (outerCounterStr.length)
          labelContent += "counter(" + outerCounterStr + ") '.' counter(" + newNumbering + ") ' '";
        else
          labelContent += "counter(" + newNumbering + ") ' '";
      }
      thmEnvArray.push( new cssPropertyChangeRecord("counter-increment", newNumbering) );
      labelArray.push( new cssPropertyChangeRecord("content", labelContent) );
    }

//                     content: 'Algorithm ' counter(section) "." counter(theorem) ' ';
    var labelFontStyleStr = "normal";
    var labelFontWeightStr = "bold";
    var envFontStyleStr = "italic";
    var envFontWeightStr = "normal";
    if (newStyle !== oldStyle)
    {
      switch(newStyle)
      {
        case "definition":
          envFontStyleStr = "normal";
        break;
        case "remark":
        break;
        default:
        case "plain":
        break;
      }
      //commented out next two lines because in practice these all get the same attributes for lead-in font (normal bold)
//      labelArray.push( new cssPropertyChangeRecord("font-style", labelFontStyleStr) );
//      labelArray.push( new cssPropertyChangeRecord("font-weight", labelFontWeightStr) );
      thmEnvArray.push( new cssPropertyChangeRecord("font-style", envFontStyleStr) );
      thmEnvArray.push( new cssPropertyChangeRecord("font-weight", envFontWeightStr) );
    }

    if (labelArray.length)
      labelArray.push( new cssPropertyChangeRecord("-moz-user-select", "-moz-none") );
    if (thmEnvArray.length)
      thmEnvArray.push( new cssPropertyChangeRecord("display", "block") );

    changeCSSPropertiesForSelectorPropertiesArray(theDoc, [[thmEnvSelStr, thmEnvArray], [labelSelStr, labelArray]]);
  },

  newTheoremNodeRepresentsTag : function(thmenvNode, thmTag)
  {
    var theName = thmenvNode.getAttribute("name");
    if (theName === thmTag)
      return true;
    var texName = thmenvNode.getAttribute("texname");
    if (!texName || !texName.length)
      texName = theName;
    if (texName === thmTag)
      return true;
    if (texName === thmTag + "*")
      return true;
    if (texName === thmTag + "+")
      return true;
    return false;
  },

//The following functions should have no use for the TheoremEnvList.
//  addString : function(aString)
//  deleteString : function(aString)
//  changeString : function(oldString, newString)

  detach : function()
  {
    this.mTheoremListManager.removeSearchStringArrayRecord(this.mTheoremListManagerRecord);
    this.mbInitialized = false;
    this.mTheoremListManagerRecord = null;
  }
};

function msiTheoremEnvList(aControl) 
{
  this.mControl = aControl;
  this.mTheoremListManager = msiTheoremEnvListManager;
  this.mTheoremListManagerRecord = msiTheoremEnvListManager.getSearchStringArrayRecordForControl(aControl);
  this.mbInitialized = msiTheoremEnvListManager.initTheoremList(this.mTheoremListManagerRecord, true);
//  this.mAddedCSSRules = [];
//  this.mDeletedCSSRules = [];
}

function msiTheoremEnvListForDocument(aDoc)
{
  this.mTheoremListManager = msiTheoremEnvListManager;
  this.mTheoremListManagerRecord = msiTheoremEnvListManager.getGenericTheoremListRecordForDocument(aDoc);
  this.mbInitialized = msiTheoremEnvListManager.initTheoremList(this.mTheoremListManagerRecord, true);
}

msiTheoremEnvList.prototype = msiTheoremListPrototype;
msiTheoremEnvListForDocument.prototype = msiTheoremListPrototype;

function msiGetNewTheoremListFromDocument(aDocument)
{
//  var xsltSheetForNewTheoremList = "<?xml version='1.0'?><xsl:stylesheet version='1.1' xmlns:xsl='http://www.w3.org/1999/XSL/Transform' xmlns:html='http://www.w3.org/1999/xhtml' ><xsl:output method='xml' encoding='UTF-8'/> <xsl:template match='/'>  <xsl:apply-templates select='//newtheorem'/></xsl:template><xsl:template match='newtheorem'><xsl:element name='newtheoremenv'><xsl:attribute name='tagname'><xsl:value-of select='@name'/></xsl:attribute><xsl:attribute name='numbering'><xsl:choose><xsl:when test='@counter'><xsl:value-of select='@counter'/></xsl:when><xsl:otherwise><xsl:value-of select='@name'/></xsl:otherwise></xsl:choose></xsl:attribute><xsl:attribute name='TeXTagName'><xsl:choose><xsl:when test='@texname'><xsl:value-of select='@texname'/></xsl:when><xsl:otherwise><xsl:value-of select='@name'/></xsl:otherwise></xsl:choose></xsl:attribute><xsl:attribute name='theoremStyle'><xsl:choose><xsl:when test='@theoremstyle'><xsl:value-of select='@theoremstyle'/></xsl:when><xsl:otherwise><xsl:text>theorem</xsl:text></xsl:otherwise></xsl:choose></xsl:attribute><xsl:attribute name='label'><xsl:value-of select='@label'/></xsl:attribute></xsl:element></xsl:template> </xsl:stylesheet>";
  var xsltSheetForNewTheoremList = "<?xml version='1.0'?><xsl:stylesheet version='1.1' xmlns:xsl='http://www.w3.org/1999/XSL/Transform' xmlns:html='http://www.w3.org/1999/xhtml' > <xsl:output method='xml' encoding='UTF-8'/> <xsl:template match='/'> <xsl:element name='resultBase'><xsl:apply-templates select='//*[name()=\"newtheorem\"]'/></xsl:element></xsl:template><xsl:template match='*' /><xsl:template match='*[name()=\"newtheorem\"]'><xsl:element name='newtheoremenv'><xsl:attribute name='tagname'><xsl:value-of select='@name'/></xsl:attribute><xsl:attribute name='numbering'><xsl:choose><xsl:when test='@counter'><xsl:value-of select='@counter'/></xsl:when><xsl:otherwise><xsl:value-of select='@name'/></xsl:otherwise></xsl:choose></xsl:attribute><xsl:attribute name='TeXTagName'><xsl:choose><xsl:when test='@texname'><xsl:value-of select='@texname'/></xsl:when><xsl:otherwise><xsl:value-of select='@name'/></xsl:otherwise></xsl:choose></xsl:attribute><xsl:attribute name='theoremStyle'><xsl:choose><xsl:when test='@theoremstyle'><xsl:value-of select='@theoremstyle'/></xsl:when><xsl:otherwise><xsl:text>plain</xsl:text></xsl:otherwise></xsl:choose></xsl:attribute><xsl:attribute name='label'><xsl:value-of select='@label'/></xsl:attribute></xsl:element></xsl:template></xsl:stylesheet>";
  var parser = new DOMParser();
  var dom = parser.parseFromString(xsltSheetForNewTheoremList, "text/xml");
  dump(dom.documentElement.nodeName === "parsererror" ? "error while parsing" + dom.documentElement.textContent : dom.documentElement.nodeName);
  var processor = new XSLTProcessor();
  processor.importStylesheet(dom.documentElement);
  var newDoc;
  var thmEnvList = {defaultNumbering : "self", defaultStyle : "theorem", theoremArray : []};
  var numberingTypeCount = {self : 0};
  var styleCount = {theorem : 0};
  if (aDocument)
    newDoc = processor.transformToDocument(aDocument, document);
  dump(newDoc.documentElement.localName+"\n");
  var searchNode, newEntry;
  var theTagName, theNumbering, theTeXName, theLabel, theThmStyle;
  var isDefault = false;
  var ourNodes = newDoc.documentElement.getElementsByTagName("newtheoremenv");
  for (var ix = 0; ix < ourNodes.length; ++ix)  //is this right?
  {
    searchNode = ourNodes[ix];
    theTagName = searchNode.getAttribute("tagname");
    theNumbering = searchNode.getAttribute("numbering");
    if (!theNumbering || !theNumbering.length)
      theNumbering = theTagName;
    if (theNumbering === theTagName)
      ++numberingTypeCount.self;
    else if (! (theNumbering in numberingTypeCount) )
      numberingTypeCount[theNumbering] = 1;
    else
      ++numberingTypeCount[theNumbering];
    theTeXName = searchNode.getAttribute("TeXTagName");
    if (!theTeXName || !theTeXName.length || (theTeXName === theTagName))
      isDefault = true;
    theThmStyle = searchNode.getAttribute("theoremStyle");
    if (!theThmStyle || !theThmStyle.length)
      theThmStyle = "theorem";
    if (! (theThmStyle in styleCount) )
      styleCount[theThmStyle] = 1;
    else
      ++styleCount[theThmStyle];
    theLabel = searchNode.getAttribute("label");
    if (!theLabel || !theLabel.length)
      theLabel = theTagName.substr(0,1).toUpperCase() + theTagName.substr(1);
    newEntry = {tagName : theTagName, numbering : theNumbering, bDefault : isDefault, thmStyle : theThmStyle};
    thmEnvList.theoremArray.push( newEntry );
  }
  for (var aNumbering in numberingTypeCount)
  {
    if (numberingTypeCount[aNumbering] > numberingTypeCount[thmEnvList.defaultNumbering])
      thmEnvList.defaultNumbering = aNumbering;
  }
  for (var aStyle in styleCount)
  {
    if (styleCount[aStyle] > styleCount[thmEnvList.defaultStyle])
      thmEnvList.defaultStyle = aStyle;
  }
  return thmEnvList;
}

function getEquationArrayCells(displayNode)
{
  var theCells = [];
  var ix, tableNode, aNode, cellNodes, rowNode, otherNode, typeAttr;

  function findMatrixCallback(theNode)
  {
    switch(theNode.nodeName)
    {
      case "msidisplay":
      case "math":
      case "mstyle":
      case "mrow":
      case "#text":
        return NodeFilter.FILTER_SKIP;
      break;
      case "mtable":
        return NodeFilter.FILTER_ACCEPT;
      break;
      default:
        return NodeFilter.FILTER_REJECT;  //rejects whole subtree
      break;
    }
  }

  function findCellsCallback(theNode)
  {
    switch(theNode.nodeName)
    {
      case "mstyle":
      case "mtr":
      case "#text":
        return NodeFilter.FILTER_SKIP;
      break;
      case "mtd":
        return NodeFilter.FILTER_ACCEPT;
      break;
      case "msidisplay":
      default:
        return NodeFilter.FILTER_REJECT;  //rejects whole subtree
      break;
    }
  }
  
  var walker = document.createTreeWalker( displayNode, NodeFilter.SHOW_ELEMENT, findMatrixCallback, false ); 
  while (aNode = walker.nextNode())
  {
    typeAttr = aNode.getAttribute("type");
    if (typeAttr && (typeAttr === "eqnarray"))
    {
      tableNode = aNode;
      break;
    }
  }
  if (!tableNode)
    return null;

  walker = document.createTreeWalker( tableNode, NodeFilter.SHOW_ELEMENT, findCellsCallback, false );
  while (aNode = walker.nextNode())
  {
    theCells.push(aNode);
  }

  return theCells;
}

//Here anEqnNode is probably a cell in an equation array
function checkNumberingOfParentEqn(anEqnNode)
{
  var dispNode = msiNavigationUtils.getEnclosingDisplay(anEqnNode);
  var ix, cellAttr;
  var currNumAttr, newNumAttr, theCells;
  if (dispNode)
  {
    currNumAttr = dispNode.getAttribute("numbering");
    newNumAttr = currNumAttr;
    theCells = getEquationArrayCells(dispNode);
    if (theCells !== null)
    {
      newNumAttr = "none";
      for (ix = 0; ix < theCells.length; ++ix)
      {
        cellAttr = theCells[ix].getAttribute("numbering");
        if (!cellAttr || !cellAttr.length || cellAttr !== "none")
        {
          newNumAttr = "eqns";
          break;
        }
      }
    }
    if (newNumAttr && (!currNumAttr || newNumAttr !== currNumAttr))
      dispNode.setAttribute("numbering", newNumAttr);
  }
}

function checkLayoutOfParentEqn(anEqnNode)
{
  var dispNode = msiNavigationUtils.getEnclosingDisplay(anEqnNode);
  if (!dispNode)
    return;

  var subnode;
  var crect, crect2;
  var ourCrect;
  var button;
  var newtop, newright, oldright;
//        netscape.security.PrivilegeManager.enablePrivilege('UniversalXPConnect');
  var theCells = getEquationArrayCells(dispNode);
  crect = dispNode.getClientRects();
  if (crect.length)
    ourCrect = crect[0];
  for (ix = 0; ix < theCells.length; ++ix)
  {
    subnode = theCells[ix];
    button = document.getAnonymousElementByAttribute(subnode, "class", "eqnnum");
    if (!button)
      button = document.getAnonymousElementByAttribute(subnode, "class", "subeqnnum");
    if (!button)
      button = document.getAnonymousElementByAttribute(subnode, "class", "eqnmarker");
    if (!button)
    {
//      var buttons = document.getAnonymousNodes(subnode);
//      for (var jx = 0; jx < buttons.length; ++jx)
      for (var jx = 0; jx < subnode.childNodes.length; ++jx)
      {
        var classAttr = subnode.childNodes[jx].getAttribute("class");
        if ( (classAttr === "eqnnum") || (classAttr === "subeqnnum") || (classAttr === "eqnmarker") )
        {
          button = subnode.childNodes[jx];
          break;
        }
      }
    }
    if (button)
    {
      crect = subnode.getClientRects();
      if (crect.length > 0)
      {
        crect = crect[0];
        crect2 = button.getClientRects();
        oldright = 0;
        if (crect2 && crect2.length)
          oldright = crect2[0].right;
        if (crect)
        {
          newtop = (crect.bottom - crect.top)/2 -10;
          newright = (crect.right - oldright) - (ourCrect.right - crect.right);
          button.setAttribute("style", "top:" + newtop + "px; right:" + newright + "px;");
        }
      }
    }
  }
}

function checkSubEqnContinuation(aChangedEqn)
{
  var subContAttr = aChangedEqn.getAttribute("subEquationContinuation");
  var bSubCont = (subContAttr !== null) && (subContAttr === "true");
  var bPrevSubCont, bNextSubCont;
  var bResetSubs = false;
  var prevEqn, nextEqn;
  var container = getSiblingEquationsContainer(aChangedEqn);
  var eqnlist = container.getElementsByTagName("msidisplay");
  for (var ix = 0; ix < eqnlist.length; ++ix)
  {
    if (eqnlist[ix] === aChangedEqn)
    {
      if (ix > 0)
        prevEqn = eqnlist[ix-1];
      if (ix < eqnlist.length - 1)
        nextEqn = eqnlist[ix+1];
      break;
    }
  }

  if (bSubCont && prevEqn)
  {
    subContAttr = prevEqn.getAttribute("subEquationContinuation");
    bPrevSubCont = (subContAttr !== null) && (subContAttr === "true");
    if (!bPrevSubCont)
      bResetSubs = true;
  }
  if (bResetSubs)
    aChangedEqn.setAttribute("subEquationReset", "true");
  else  //we don't need this attribute if we're the same subEqCont status as the previous
    aChangedEqn.removeAttribute("subEquationReset");
  if (nextEqn)
  {
    subContAttr = nextEqn.getAttribute("subEquationContinuation");
    bNextSubCont = (subContAttr !== null) && (subContAttr === "true");
    if (bNextSubCont && !bSubCont)
      nextEqn.setAttribute("subEquationReset", "true");
    else
      nextEqn.removeAttribute("subEquationReset");
  }
}

function getSiblingEquationsContainer(theNode)
{
  var findTags = ["body", "section"];  //this needs to be configurable! Should include all tags which reset equation counter.
  var currName;
  for (var aNode = theNode; aNode; aNode = aNode.parentNode)
  {
    currName = aNode.nodeName;
    if (findTags.indexOf(currName) >= 0)
      return aNode;
  }
  return document.documentElement; //should NOT happen
}

function checkForMultiRowInTable(aTable, editor)
{
  function checkForMultiRowCell(aNode)
  {
    var nodeName = msiGetBaseNodeName(aNode);
    switch(nodeName)
    {
      case "table":
      case "mtable":
        if (aNode !== aTable)
          return NodeFilter.FILTER_REJECT;  //rejects whole subtree
      break;
      case "td":
      case "th":
      case "mtd":
        return NodeFilter.FILTER_ACCEPT;
      break;
      case "tbody":
      case "thead":
      case "tfoot":
      case "tr":
      case "mtr":
      case "mlabeledtr":
        return NodeFilter.FILTER_SKIP;
      break;
      default:
        return NodeFilter.FILTER_REJECT;  //rejects whole subtree
      break;
    }
  }

  var walker = editor.document.createTreeWalker(aTable, NodeFilter.SHOW_ELEMENT,
                                                checkForMultiRowCell, true);
  var nextNode;
  var multiRowCells = [];
  var singleRowCells = [];
  while (nextNode = walker.nextNode())
  {
    if (nextNode.hasAttribute("rowspan") && (Number(nextNode.getAttribute("rowspan")) > 1) )
      multiRowCells.push(nextNode);
    else if (nextNode.hasAttribute("req") && (nextNode.getAttribute("req") === "multirow") )
      singleRowCells.push(nextNode);
  }
  if (!multiRowCells.length && !singleRowCells.length)
    return;

  editor.beginTransaction();
  var ii;
  for (ii = 0; ii < multiRowCells.length; ++ii)
    msiEditorEnsureElementAttribute(multiRowCells[ii], "req", "multirow", editor);
  for (ii = 0; ii < singleRowCells.length; ++ii)
    msiEditorEnsureElementAttribute(singleRowCells[ii], "req", null, editor);
  editor.endTransaction();
}

/**************************msiNavigationUtils**********************/
var msiNavigationUtils = 
{
  m_DOMUtils : Components.classes["@mozilla.org/inspector/dom-utils;1"].createInstance(Components.interfaces.inIDOMUtils),
  mAtomService : Components.classes["@mozilla.org/atom-service;1"].getService(Components.interfaces.nsIAtomService),
//a few constants:
  leftEndToLeft : 0,
  leftEndToRight : 1,
  rightEndToLeft : 2,
  rightEndToRight : 3,
  toLeft : 0,
  leftEnd : 0,
  toRight : 1,
  rightEnd : 2,

  isIgnorableWhitespace : function(node)
  {
    return this.doWhitespaceTest(node, false);
  },

  isTrueWhitespace : function(node)  //This finds one of the true throw-away nodes generated by whitespace (linebreaks, for instance) in the source
  {
    return this.doWhitespaceTest(node, true);
  },

  doWhitespaceTest : function(node, testStrict)
  {
    if (!testStrict && (node.nodeType !== nsIDOMNode.TEXT_NODE))
    {
      switch(msiGetBaseNodeName(node))
      {
        case "mi":
        case "mo":
          var childNodes = this.getSignificantContents(node);
          if (childNodes.length === 0)
            return true;
          if (childNodes.length > 1)
            dump("In msiNavigationUtils.isIgnorableWhitespace, found too many children for node of type " + msiGetBaseNodeName(node) + ".\n");
          return this.isIgnorableWhitespace(childNodes[0]);
        break;
        default:
          return false;
        break;
      }
    }
    return this.m_DOMUtils.isIgnorableWhitespace(node);
  },

  mWhiteSpaceTestRE : /^\s*$/,
  isWhiteSpace : function(aTextPiece)
  {
    return ( this.mWhiteSpaceTestRE.test(aTextPiece) );
  },


  boundaryIsTransparent : function(node, editor, posAndDirection)  //This has to do with whether node's children at right or left are considered adjacent to following or preceding objects, not with cursor movement.
  {
    if (node.nodeType === nsIDOMNode.TEXT_NODE)
      return true;

    if (this.isMathMLLeafNode(node))
      return !(this.isMathname(node) || this.isUnit(node));

    switch( this.getTagClass( node, editor) )
    {
      case "texttag":
//  In the cases below, we'd want to identify a space or break at the end of a paragraph as an object to be revised from the right?
//      case "paratag":
//      case "listtag":
//      case "structtag":
//      case "envtag":
        return true;
      case "frontmtag":
      default:
      break;                          
    }
    switch(msiGetBaseNodeName(node))
    {
      case "mrow":
      case "mstyle":
      //Need to examine stuff here - like is the parent an mfrac? Is the mrow a fence?
      {
        if (this.isFence(node))
          return false;
        if (node.parentNode !== null)
        {
          var parentName = msiGetBaseNodeName(node.parentNode);
//          var posInParent = this.significantOffsetInParent(node);
          switch(parentName)
          {
            case "mfrac":
            case "mroot":
            case "msqrt":
            case "mradical":
            case "msub":
            case "msup":
            case "msubsup":
            case "mmultiscripts":
            case "munder":
            case "mover":
            case "munderover":
              return ( (posAndDirection === this.rightEndToLeft) || (posAndDirection === this.leftEndToRight) );
//              return false;
            break;
          }
        }
        return true;
      }
      break;
      case "math":
        if (!node.hasAttribute("display") || (node.getAttribute("display") !== "block") )
          return true;
      break;
      default:
      break;
    }
    return false;
  },

  positionIsAtStart : function(aNode, anOffset)
  {
    if (anOffset === 0)
      return true;
    if (aNode.nodeType === nsIDOMNode.TEXT_NODE)
      return this.isIgnorableWhitespace(aNode);
    else if (anOffset < aNode.childNodes.length)
    {
      for (var ix = anOffset; ix > 0; --ix)
      {
        if (!this.isIgnorableWhitespace(aNode.childNodes[ix - 1]))
          break;
      }
      if (ix === 0)
        return true;
    }
    return false;
  },

  positionIsAtEnd : function(aNode, anOffset)
  {
    var nLength = 0;
    if (aNode.nodeType === nsIDOMNode.TEXT_NODE)
      nLength = aNode.data.length;
    else
      nLength = aNode.childNodes.length;
    if (anOffset === nLength)
      return true;
    if (aNode.nodeType === nsIDOMNode.TEXT_NODE)
      return this.isIgnorableWhitespace(aNode);
    else if (anOffset < nLength)
    {
      for (var ix = anOffset; ix < nLength; ++ix)
      {
        if (!this.isIgnorableWhitespace(aNode.childNodes[ix]))
          break;
      }
      if (ix === nLength)
        return true;
    }
    return false;
  },

  offsetInParent : function(aNode)
  {
    var retVal = -1;
    for (var ix = 0; ix < aNode.parentNode.childNodes.length; ++ix)
    {
      if (aNode.parentNode.childNodes[ix] === aNode)
      {
        retVal = ix;
        break;
      }  
    }
    return retVal;
  },

  isDescendant : function(aNode, refNode)
  {
    return (this.findTopChildContaining(refNode, aNode) !== null);
  },

  isAncestor : function(aNode, refNode)
  {
    return (this.findTopChildContaining(aNode, refNode) !== null);
  },

  //Note that this function will return ancestorNode if aNode===ancestorNode, so it isn't necessarily a child
  findTopChildContaining : function(aNode, ancestorNode)
  {
    var retVal = null;
    for (var targNode = aNode; targNode; targNode = targNode.parentNode)
    {
      if ( (targNode === ancestorNode) || (targNode.parentNode === ancestorNode) )
      {
        retVal = targNode;
        break;
      }
    }
    return retVal;
  },

  findCommonAncestor : function(someNodes)
  {
    var anAncestor = someNodes[0];
    for (var ix = 1; ix < someNodes.length; ++ix)
    {
      while ( anAncestor && (!this.isAncestor(someNodes[ix], anAncestor)) )
      {
        anAncestor = anAncestor.parentNode;
      }
    }
    return anAncestor;
  },

  comparePositions : function(aNode, anOffset, refNode, refOffset, bLogIt)
  {
    var aNodeAncestors = new Array();
    var nNodeAncestors = 0;
    var refAncestors = new Array();
    var nRefAncestors = 0;
    var nRealOffset = anOffset;
    var nRealRefOffset = refOffset;
    var lastNode = aNode;
    var lastRefNode = refNode;
    for (var anAncestor = aNode; anAncestor; anAncestor = anAncestor.parentNode)
    {
      nNodeAncestors = aNodeAncestors.push(anAncestor);
    }
    for (var aRefAncestor = refNode; aRefAncestor; aRefAncestor = aRefAncestor.parentNode)
    {
      nRefAncestors = refAncestors.push(aRefAncestor);
    }
    if (aNodeAncestors.pop() !== refAncestors.pop())
    {
      dump("In msiNavigationUtils.comparePositions(), no common ancestor for nodes [" + aNode.nodeName + ",[" + aNode.textContent + "]] and [" + refNode.nodeName + ",[" + refNode.textContent + "]]!\n");
      return 0;
    }
    //Descend the ancestor chains until we reach the last point of common ancestry (or rather the first point of different ancestry)
    while ((aNodeAncestors.length > 0) || (refAncestors.length > 0))
    {
      lastNode = aNodeAncestors.pop();
      lastRefNode = refAncestors.pop();
      if (lastNode !== lastRefNode)
        break;
    }
    try
    {
    if (lastNode !== lastRefNode)  //if they're equal, it should mean that aNode was equal to refNode, and then we want to compare incoming offsets so leave them alone.
    {
      if (lastNode)
        nRealOffset = this.offsetInParent(lastNode);
      if (lastRefNode)
        nRealRefOffset = this.offsetInParent(lastRefNode);
    }
    } catch(exc) {bLogIt = true; dump("Exception in msiNavigationUtils.comparePositions: [" + exc + "].\n");}
    if (bLogIt)
    {
      function describeNode(someNode)
      {
        var retString = "[";
        if (someNode)
          retString += someNode.nodeName + ",[" + someNode.textContent + "]]";
        else
          retString += "null]";
        return retString;
      };
//      dump("In msiNavigationUtils.comparePositions; aNode is [" + aNode.nodeName + ",[" + aNode.textContent + "]], while refNode is [" + refNode.nodeName + ",[" + refNode.textContent + "]].\n");
      dump("In msiNavigationUtils.comparePositions; aNode is " + describeNode(aNode) + ", while refNode is " + describeNode(refNode) + ".\n");
      dump("  After checking ancestor arrays, ended with lastNode being " + describeNode(lastNode) + " and lastRefNode " + describeNode(lastRefNode) + "; nRealOffset is then [" + nRealOffset + "] and nRealRefOffset is [" + nRealRefOffset + "].\n");
    }
    if (nRealOffset > nRealRefOffset)
      return 1;
    else if (nRealOffset < nRealRefOffset)
      return -1;
//    else
    if (lastNode && !lastRefNode)  //in this case our position is inside lastNode and thus to the right of the ref position
      return 1;
    if (lastRefNode && !lastNode)  //in this case the ref position is inside lastRefNode and thus to the right of our position
      return -1;
        
    return 0;
  },

//  comparePositions : function(aNode, anOffset, refNode, refOffset)
//  {
//    var compVal = aNode.compareDocumentPosition(refNode);
//    var aParent, refParent;
//    if (!compVal)  //this means aNode and refNode are the same
//    {
//      if (anOffset < refOffset)
//        return -1;
//      if (anOffset > refOffset)
//        return 1;
//      return 0;
//    }
//
//    if (compVal & nsIDOMNode.DOCUMENT_POSITION_CONTAINS)  //refNode is a child of our node - before or after offset?
//    {
//      refParent = refNode;
//      while (refParent)
//      {
//        if (refParent.parentNode === aNode)
//        {
//          if (anOffset <= this.offsetInParent(refParent))
//            return -1;  //refNode is a child of aNode.childNodes[offsetInParent] and so follows the (aNode,anOffset) position
//          else
//            return 1;  //otherwise refNode comes before (aNode, anOffset)
//        }
//        refParent = refParent.parentNode;
//      }
//    }
//    else if (compVal & nsIDOMNode.DOCUMENT_POSITION_IS_CONTAINED)
//    {
//      aParent = aNode;
//      while (aParent)
//      {
//        if (aParent.parentNode === refNode)
//        {
//          if (refOffset <= this.offsetInParent(aParent))
//            return 1;  //aNode is a child of refNode.childNodes[offsetInParent] and so follows the (refNode,refOffset) position
//          else
//            return -1;  //otherwise aNode comes before (refNode, refOffset)
//        }
//        aParent = aParent.parentNode;
//      }
//    }
//    if (compVal & nsIDOMNode.DOCUMENT_POSITION_FOLLOWING)
//      return 1;  //we're after the ref node
//    if (compVal & nsIDOMNode.DOCUMENT_POSITION_PRECEDING)
//      return -1;  //we're before the ref node
//    return 0;  //Big leap of faith here - just assume if we couldn't find it anywhere else that it's the same position?????
//  },

  getNodeBeforePosition : function(aNode, offset)
  {
    var retNode = null;
    if (!this.positionIsAtStart(aNode, offset))
    {
      if ( (aNode.childNodes !== null) && (aNode.childNodes.length >= offset) )
        retNode = aNode.childNodes[offset - 1];
    }
    if (retNode === null)
    {
      retNode = aNode.previousSibling;
      while ( (retNode === null) && (aNode.parentNode !== null) )
      {
        aNode = aNode.parentNode;
        retNode = aNode.previousSibling;
      }
    }
    return retNode;
  },

  getNodeAfterPosition : function(aNode, offset)
  {
    var retNode = null;
    if (!this.positionIsAtEnd(aNode, offset))
    {
      if ( (aNode.childNodes !== null) && (aNode.childNodes.length > offset) )
        retNode = aNode.childNodes[offset];
    }
    if (retNode === null)
    {
      retNode = aNode.nextSibling;
      while ( (retNode === null) && (aNode.parentNode !== null) )
      {
        aNode = aNode.parentNode;
        retNode = aNode.nextSibling;
      }
    }
    return retNode;
  },

  significantOffsetInParent : function(aNode)
  {
    var siblings = this.getSignificantContents(aNode.parentNode);
    var retVal = -1;
    for (var ix = 0; ix < siblings.length; ++ix)
    {
      if (siblings[ix] === aNode)
      {
        retVal = ix;
        break;
      }
    }
    return retVal;
  },

  getTagClass : function(node, editor)
  {
    var nsAtom = null;
    if (node.namespaceURI !== null)
      nsAtom = this.mAtomService.getAtom(node.namespaceURI);

    var retVal = "othertag";
    if (editor !== null)
    {
      retVal = editor.tagListManager.getClassOfTag( node.nodeName, nsAtom);
      if (retVal === null || retVal.length === 0)
        retVal = editor.tagListManager.getClassOfTag( node.nodeName, null );
    }
    return retVal;
  },

  isDefaultParaTag : function(node, editor)
  {
    var nsAtom = null;
    if (node.namespaceURI !== null)
      nsAtom = this.mAtomService.getAtom(node.namespaceURI);

    var namespace = new Object();
    var paraTag = "";
    if (editor !== null)
      paraTag = editor.tagListManager.getDefaultParagraphTag(namespace);
    if ((paraTag === node.nodeName) && ((nsAtom === null) || (namespace.value && (nsAtom === namespace.value))) )
        return true;
    return false;
  },

  isFence : function(node)
  {
    var nodeName = msiGetBaseNodeName(node);
//    if (nodeName === 'mstyle' && node.childNodes.length === 1)
//      return this.isFence(node.childNodes[0]);
    if (nodeName === 'mfenced')
      return true;

    if (nodeName === 'mrow' || nodeName === 'mstyle')
    {
      var children = this.getSignificantContents(node);
      if (children.length > 1)
      {
        return ( (msiGetBaseNodeName(children[0]) === 'mo') && (children[0].getAttribute('fence') === 'true') && (children[0].getAttribute('form') === 'prefix')
            && (msiGetBaseNodeName(children[children.length - 1]) === 'mo')
            && (children[children.length - 1].getAttribute('fence') === 'true')
            && (children[children.length - 1].getAttribute('form') === 'postfix') )
      }
    }
    return false;
  },

  nodeIsSignificant : function(aNode)
  {
    return (!this.isIgnorableWhitespace(aNode));
  },

  getSignificantContents : function(node)
  {
    var retList = new Array();
    if (node === null)
      return retList;

    for (var ix = 0; ix < node.childNodes.length; ++ix)
    {
      if (this.nodeIsSignificant(node.childNodes[ix]))
        retList.push(node.childNodes[ix]);
    }
    return retList;
  },

  getFirstSignificantChild : function(node)
  {
    var children = this.getSignificantContents(node);
    if (children.length > 0)
      return children[0];
    return null;
  },

  getLastSignificantChild : function(node)
  {
    var children = this.getSignificantContents(node);
    if (children.length > 0)
      return children[children.length - 1];
    return null;
  },

  getSignificantChildFollowingNode : function(parentNode, childNode, bWrap)
  {
    var retVal = null;
    var sigChildren = this.getSignificantContents(parentNode);
    var topChild = this.findTopChildContaining(childNode, parentNode);
    if (topChild && (topChild !== parentNode))
    {
      retVal = topChild;
      do
      {
        retVal = retVal.nextSibling;
        if (retVal && (sigChildren.indexOf(retVal) >= 0))
          break;
      } while (retVal);
    }
    if (!retVal && bWrap)
      return this.getFirstSignificantChild(parentNode);
    return retVal;
  },

  getSignificantChildFollowingPosition : function(parentNode, nPos, bWrap)
  {
    var precedingNode = this.getNodeBeforePosition(parentNode, nPos);
    if (precedingNode)
      return this.getSignificantChildFollowingNode(parentNode, precedingNode, bWrap);
    if (bWrap)
      return this.getFirstSignificantChild(parentNode);
    return null;
  },

  getSignificantChildPrecedingNode : function(parentNode, childNode, bWrap)
  {
    var retVal = null;
    var sigChildren = this.getSignificantContents(parentNode);
    var topChild = this.findTopChildContaining(childNode, parentNode);
    if (topChild && (topChild !== parentNode))
    {
      retVal = topChild;
      while (retVal)
      {
        retVal = retVal.previousSibling;
        if (retVal && (sigChildren.indexOf(retVal) >= 0))
          break;
      }
    }
    if (!retVal && bWrap)
      return this.getLastSignificantChild(parentNode);
    return retVal;
  },

  getSignificantChildPrecedingPosition : function(parentNode, nPos, bWrap)
  {
    var nextNode = this.getNodeAfterPosition(parentNode, nPos);
    if (nextNode)
      return this.getSignificantChildPrecedingNode(parentNode, nextNode, bWrap);
    if (bWrap)
      return this.getLastSignificantChild(parentNode);
    return null;
  },

  //Note that this expects a 0-based child number.
  getIndexedSignificantChild : function(node, nChild)
  {
    var children = this.getSignificantContents(node);
    if (children.length > nChild)
      return children[nChild];
    return null;
  },

  getSignificantRangeContent : function(range)
  {
    var retList = new Array();
    var clonedDocFragment = range.cloneContents();
    for (var ix = 0; ix < clonedDocFragment.childNodes.length; ++ix)
    {
      if (!this.isIgnorableWhitespace(clonedDocFragment.childNodes[ix]))
        retList.push(clonedDocFragment.childNodes[ix]);
    }
    return retList;
  },

  getChildrenByTagName : function(aNode, nodeName)
  {
    var tagList = [];
    for (var ix = 0; ix < aNode.childNodes.length; ++ix)
    {
      if (msiGetBaseNodeName(aNode.childNodes[ix]) === nodeName)
        tagList.push(aNode.childNodes[ix]);
    }
    return tagList;
  },

  getParentOfType : function(aNode, nodeName)
  {
    if (!aNode || (msiGetBaseNodeName(aNode) === nodeName))
      return aNode;
    else
      return this.getParentOfType(aNode.parentNode, nodeName);
  },

  getTopParagraphParent : function(aNode, editor)
  {
    var thePara = null;
    var currNode = aNode;
    while (currNode)
    {
      switch (this.getTagClass(currNode, editor))
      {
        case "paratag":
        case "listtag":
          thePara = currNode;
        break;
        default:
        break;
      }
      currNode = currNode.parentNode;
    }
    return thePara;
  },

  nodeIsInMath : function(aNode)
  {
    return (this.getParentOfType(aNode, "math") !== null);
  },

  isMathTag : function(tagName)
  {
    switch(tagName)
    {
      case "mrow":
      case "math":
      case "mtable":
      case "mtd":
      case "mtr":
      case "mi":
      case "mo":
      case "mn":
      case 'mfrac':
      case 'msub':
      case 'msubsup':
      case 'msup':
      case 'munder':
      case 'mover':
      case 'munderover':
      case 'mroot':
      case 'msqrt':
      case 'mrow':
      case 'mstyle':
        return true;
      break;
      default:
        return false;
      break;
    }
    return false;
  },

  isMathNode : function(aNode)
  {
    var mathNS = this.mAtomService.getAtom(mmlns);
    var nodeNS = null;
    if (aNode.namespaceURI && aNode.namespaceURI.length)
      nodeNS = this.mAtomService.getAtom(aNode.namespaceURI);
    if (nodeNS === mathNS)
      return true;
    return false;
  },

  isBoundFence : function(node)
  {
//    if (msiGetBaseNodeName(node) === 'mstyle' && node.childNodes.length === 1)
//      return this.isBoundFence(node.childNodes[0]);
//    var singleChild = this.getSingleWrappedChild(node);
//    if (singleChild !== null)
//      return this.isBoundFence(singleChild);

    if( this.isFence(node) )
    {
      var children = this.getSignificantContents(node);
      if (children.length > 1)
      {
        return (children[0].hasAttribute('msiBoundFence') && (children[0].getAttribute('msiBoundFence') === 'true') 
               && children[children.length - 1].hasAttribute('msiBoundFence') && (children[children.length - 1].getAttribute('msiBoundFence') === 'true') );
      }
    }
    return false;
  },

  isBinomial : function(node)
  {
    if (msiGetBaseNodeName(node) === 'mstyle' && node.childNodes.length === 1)
      return this.isBinomial(node.childNodes[0]);
    if (this.isBoundFence(node))
    {
      var children = this.getSignificantContents(node);
      if (children.length === 3)
      {
        if (msiGetBaseNodeName(children[1]) === 'mfrac')
          return true;
      }
    }
    return false;
  },

  isMathTemplate : function(node)
  {
    if (node === null)
      return false;

    switch(msiGetBaseNodeName(node))
    {
      case 'mfrac':
      case 'msub':
      case 'msubsup':
      case 'msup':
      case 'munder':
      case 'mover':
      case 'munderover':
      case 'mroot':
//		  case 'msqrt':  
//		  case 'mtr':
//			case 'mtd':
//		  case 'mtable': 
        return true;
      break;
      case 'mrow':
      case 'mstyle':
        if (this.isFence(node))
          return true;
        var singleChild = this.getSingleWrappedChild(node);
        if (this.isMathTemplate(singleChild))
          return true;
 				break;
//			case "math":
//				if (node.hasAttribute("display") && node.getAttribute("display")==="block")
//				{
//					return true;
//				}
//      break;
    }
    return false;
  },

// the next two functions give more precise information than isMathTemplate
  isUnsplittableMath : function(node)
	{
	  if (node === null)
      return false;

    switch(node.localName)
		{
			case 'mfrac':
			case 'msub':
			case 'msubsup':
			case 'msup':
			case 'munder':
			case 'mover':
			case 'munderover':
			case 'mroot':
			case 'msqrt':  
			case 'mtr':
			case 'mtd':
			case 'mtable': 
			  return true;
			break;
			case 'mrow':
			case 'mstyle':
			  if (this.isFence(node))
			    return true;
			break;
			case "math":
			if (node.hasAttribute("display") && node.getAttribute("display")==="block")
			{
				return true;
			}
			break;	
		}
		return false;
	},
	
	hasFixedNumberOfChildren : function(node)
	{
	  if (node === null)
      return false;
    switch(node.localName)
		{
			case 'mfrac':
			case 'msub':
			case 'msubsup':
			case 'msup':
			case 'munder':
			case 'mover':
			case 'munderover':
			case 'mroot':
			case 'mtr':
			case 'mtable': 
			  return true;
		}
	  return false;	
	},
	
  isUnit : function(node)
  {
    if ( node !== null && node.hasAttribute("msiunit") && (node.getAttribute("msiunit") === "true") )
      return true;
    if (msiGetBaseNodeName(node) === "mstyle" && node.childNodes.length === 1)
      return this.isUnit(node.childNodes[0]);
    return false;
  },

  isMathname : function(node)
  {
    if ( node === null)
    {
      dump("Bad input to msiNavigationUtils.isMathname - null node!\n");
      return false;
    }
    if (node.hasAttribute("msimathname") && (node.getAttribute("msimathname") === "true") )
      return true;
    if (msiGetBaseNodeName(node) === "mstyle" && node.childNodes.length === 1)
      return this.isMathname(node.childNodes[0]);

    return false;
  },

  isMathMLLeafNode : function(aNode)
  {
    switch(msiGetBaseNodeName(aNode))
    {
      case "mi":
      case "mo":
      case "mn":
        return true;
      break;
    }
    return false;
  },

  isTextNode : function(aNode)
  {
    return (aNode.nodeType === nsIDOMNode.TEXT_NODE);
  },

  isBigOperator : function(aNode)
  {
    if ( (aNode !== null) && (msiGetBaseNodeName(aNode) === 'mo') && aNode.hasAttribute("largeop")
                                  && (aNode.getAttribute("largeop") === "true") )
      return true;
    return false;
  },

  getEmbellishedOperator : function(node)
  {
    if ( node !== null)
    {
      switch(msiGetBaseNodeName(node))
      {
        case 'msub':
        case 'msup':
        case 'msubsup':
        case 'mover':
        case 'munder':
        case 'munderover':
        case 'mmultiscripts':
        //According to the MathML 2.1 spec, we should also include 'mfrac" here. I'm not convinced that would give the right thing generally here.
        {
          var opNode = this.getFirstSignificantChild(node);
          if (opNode !== null)
          {
            var opNodeName = msiGetBaseNodeName(opNode);
            if ( (opNodeName === 'mstyle') || (opNodeName === 'mrow') )
              opNode = this.getSingleWrappedChild(opNode);
            if ( (opNode !== null) && (msiGetBaseNodeName(opNode) === 'mo') && opNode.hasAttribute("largeop")
                                          && (opNode.getAttribute("largeop") === "true") )
              return opNode;
          }
        }
        break;
        default:
        break;
      }
    }
    var singleChild = this.getSingleWrappedChild(node);
    if (singleChild !== null)
      return this.getEmbellishedOperator(singleChild);
//    if (msiGetBaseNodeName(node) === "mstyle" && node.childNodes.length === 1)
//      return this.isEmbellishedOperator(node.childNodes[0]);

    return null;
  },

  getSingleWrappedChild : function(aNode)
  {
    switch(msiGetBaseNodeName(aNode))
    {
      case 'mstyle':
      case 'mrow':
        var nodeContents = this.getSignificantContents(aNode);
        if (nodeContents.length === 1)
          return nodeContents[0];
      break;

      case 'notewrapper':
        var nodeContents = this.getSignificantContents(aNode);
        if ((nodeContents.length > 1) && (msiGetBaseNodeName(nodeContents[0]) === "br"))
          nodeContents.splice(0, 1);
        if ((nodeContents.length > 1) && (msiGetBaseNodeName(nodeContents[nodeContents.length - 1]) === "br"))
          nodeContents.splice(nodeContents.length - 1, 1);
        if (nodeContents.length === 1)
          return nodeContents[0];
        dump("Surprising contents in notewrapper! Has [" + nodeContents.length + "] children!\n");
        for (var ix = 0; ix < nodeContents.length; ++ix)
        {
          if (msiGetBaseNodeName(nodeContents[ix]) === "note")
            return nodeContents[ix];
        }  
      break;

      case 'msiframe':
        if (aNode.getAttribute("frametype") == "image")
        {
          var nodeContents = this.getSignificantContents(aNode);
          var foundOther = false;
          var imageNode;
          for (var ii = 0; !foundOther && (ii < nodeContents.length); ++ii)
          {
            switch(msiGetBaseNodeName(nodeContents[ii]))
            {
              case 'img':
              case 'object':
              case 'embed':
                imageNode = nodeContents[ii];
              break;
              case 'imagecaption':  //just keep looking
              break;
              default:
                foundOther = true;
              break;
            }
          }
          if (imageNode && !foundOther)
            return imageNode;
        }
      break;

      default:
      break;
    }
    return null;
  },

  getWrappedObject : function(aNode, objType)
  {
    var theName = msiGetBaseNodeName(aNode);
    var singleKid = null;
    switch(theName)
    {
      case 'mstyle':
        singleKid = this.getSingleWrappedChild(aNode);
        if (singleKid !== null)
          return this.getWrappedObject(singleKid, objType);
        if (objType === 'fence' && this.isFence(aNode))
          return aNode;
        else if (objType === 'binomial' && this.isBinomial(aNode))
          return aNode;
      break;

      case 'mrow':
        if (objType === 'fence' && this.isFence(aNode))
          return aNode;
        else if (objType === 'binomial' && this.isBinomial(aNode))
          return aNode;
        else
        {
          singleKid = this.getSingleWrappedChild(aNode);
          if (singleKid !== null)
            return this.getWrappedObject(singleKid, objType);
        }
      break;

      default:
        switch(objType)
        {
          case 'mathname':
            if (this.isMathname(aNode))
              return aNode;
          break;
          case 'unit':
            if (this.isUnit(aNode))
              return aNode;
          break;
          case 'operator':
            if (theName === 'mo')
              return aNode;
            if (this.getEmbellishedOperator(aNode) !== null)
              return aNode;
          break;
          default:
            if (theName === objType)
              return aNode;
          break;
        }
      break;
    }
    return null;
  },

  getSingleSignificantChild : function(aNode, bIncludeWrappers)
  {
    var retNode = null;
    var nodeContents = this.getSignificantContents(aNode);
    if (nodeContents.length === 1)
    {
      if (!bIncludeWrappers)
        retNode = this.getSingleWrappedChild(nodeContents[0]);
      if (!retNode)
        retNode = nodeContents[0];
    }
    return retNode;
  },

  findWrappingStyleNode : function(aNode)
  {
    if (msiGetBaseNodeName(aNode) === 'mstyle')
      return aNode;

    if ( (aNode !== null) && (aNode.parentNode !== null) )
    {
      if ( this.getSingleWrappedChild(aNode.parentNode) === aNode )
      {
        return this.findWrappingStyleNode(aNode.parentNode);
      }
    }
    return null;
  },

  findWrappingNode : function(aNode)
  {
    if ( (aNode !== null) && (aNode.parentNode !== null) )
    {
      if ( this.getSingleWrappedChild(aNode.parentNode) === aNode )
      {
        return this.findWrappingNode(aNode.parentNode);
      }
    }
    return aNode;
  },

  isEmptyInputBox : function(aNode)
  {
    if (msiGetBaseNodeName(aNode) === "mi")
    {
        if (aNode.hasAttribute("tempinput") && (aNode.getAttribute("tempinput") === "true") )
        return true;
    }
    return false;
  },

  isOrdinaryMRow : function(aNode)
  {
    if (msiGetBaseNodeName(aNode) === 'mrow')
    {
      if (this.isFence(aNode))
        return false;
      var retVal = true;
      var theAttrs = aNode.attributes;
      for (var jx = 0; (jx < theAttrs.length) && retVal; ++jx)
      {
        switch(theAttrs.item(jx).nodeName)
        {
          case '_moz_dirty':
          break;
          default:
            retVal = false;
          break;
        }
      }
      return retVal;
    }
    return false;
  },

  isSingleSignificantChild : function(aNode)
  {
    var aParent = aNode.parentNode;
    return (aNode === this.getSingleSignificantChild(aParent, true));
  },

  isEquationArray : function(editorElement, aTable)
  {
    if (aTable.getAttribute("type") === "eqnarray")
      return true;
    var tableDims = msiGetEnclosingTableOrMatrixDimensions(editorElement, aTable);
    if (tableDims.nCols !== 1)
      return false;
    var topNode = this.findWrappingNode(aTable);  //inside displays we often see nested <mstyle> and <mrow>s
    var isOK = true;
    while (isOK && topNode)
    {
      switch(msiGetBaseNodeName(topNode))
      {
        case "msidisplay":  //Success! Return true
          return true;
        break;
        case "math":
        break;
        default:
          isOK = false;
        break;
      }
      if (!this.isSingleSignificantChild(topNode))
        isOK = false;
      topNode = topNode.parentNode;
    }
    return false;
  },

  getEnclosingDisplay : function(aNode)
  {
    var topNode = this.findWrappingNode(aNode);  //inside displays we often see nested <mstyle> and <mrow>s
    var isOK = true;
    while (isOK && topNode)
    {
      switch(msiGetBaseNodeName(topNode))
      {
        case "msidisplay":  //Success! Return true
          return topNode;
        break;
        default:
        break;
      }
      topNode = topNode.parentNode;
    }
    return null;
  },

  isUnnecessaryMStyle : function(aNode)
  {
    var bUnnecessary = false;
    if (msiGetBaseNodeName(aNode) === 'mstyle')
    {
      if (this.isFence(aNode))
        return false;
      bUnnecessary = true;
      var theAttrs = aNode.attributes;
      for (var jx = 0; (jx < theAttrs.length) && bUnnecessary; ++jx)
      {
        switch(theAttrs.item(jx).nodeName)
        {
          case '_moz_dirty':
          break;
          default:
//            dump("In msiNavigationUtils.isUnnecessaryMStyle(), node has attribute [" + theAttrs.item(jx).nodeName + "].\n");
            bUnnecessary = false;
          break;
        }
      }
    }
//    dump("In msiNavigationUtils.isUnnecessaryMStyle(), returning [" + bUnnecessary + "].\n");
    return bUnnecessary;
  },

  getLeafNodeText : function(aNode)
  {
    if (aNode.nodeType === nsIDOMNode.TEXT_NODE)
      return aNode.data;
    switch(msiGetBaseNodeName(aNode))
    {
      case "mi":
      case "mo":
      case "mn":
      {
        var childNodes = this.getSignificantContents(aNode);
        if (childNodes.length !== 1)
          dump("In msiNavigationUtils.getLeafNodeText, found too many children for node of type " + msiGetBaseNodeName(aNode) + ".\n");
        return this.getLeafNodeText(childNodes[0]);
      }
      break;
    }
    var kid = this.getSingleWrappedChild(aNode);
    if (kid !== null)
      return this.getLeafNodeText(kid);
    return null;
  },

  //What seems to be needed is a generic way to go coherently "up the ladder". If we find that this node satisfies the conditions,
  //  we'd like to test our parent to see if it does as well. The problem is that part of the check involves testing child nodes,
  //  so we keep getting into a loop.
  //Cases to consider:
  //  Obvious ones: <mi>
  //  An <mstyle> wrapping only a qualifying node.
  //  An <mrow> wrapping a qualifying node, but only if it's an (unnecessary) <mrow> representing a child of a <mover>, <munder>, or <munderover>,
  //    or the only child of an <mstyle>.
  //Maybe the key is to do a simple test recursively - a test which would never pass to the parent node - and repeatedly apply the
  //  test to successive parents as long as it succeeds. This separation of the test from the overall function (getTopMathNodeAsAccentedCharacter, below), 
  //  which does have the responsibility to find the topmost qualifying node, may finally solve this stupidity.
  treatMathNodeAsAccentedCharacter : function(aNode, bDontCheckKids)
  {
    msiKludgeLogNodeContents(aNode, ["reviseChars"], "In msiEditorUtilities.js, in msiNavigationUtils.treatMathNodeAsAccentedCharacter, with node", false);

    var ourData = null;
    var aboveText, belowText, baseText;

    switch(msiGetBaseNodeName(aNode))
    {
      case "mover":
        baseText = this.getLeafNodeText( this.getIndexedSignificantChild(aNode, 0) );
        aboveText = this.getLeafNodeText( this.getIndexedSignificantChild(aNode, 1) );
      break;

      case "munder":
        baseText = this.getLeafNodeText( this.getIndexedSignificantChild(aNode, 0) );
        belowText = this.getLeafNodeText( this.getIndexedSignificantChild(aNode, 1) );
      break;

      case "munderover":
        baseText = this.getLeafNodeText( this.getIndexedSignificantChild(aNode, 0) );
        belowText = this.getLeafNodeText( this.getIndexedSignificantChild(aNode, 1) );
        aboveText = this.getLeafNodeText( this.getIndexedSignificantChild(aNode, 2) );
      break;

      case "mi":
      case "mo":
      case "mn":
        baseText = this.getLeafNodeText(aNode);
      break;

      case "mstyle":  //is this the only time we want to do this?
      case "mrow":
//      case "mphantom":  ???
        var kid = this.getSingleWrappedChild(aNode);
        if (kid !== null && !bDontCheckKids)
          return this.treatMathNodeAsAccentedCharacter(kid, false);
      break;

      default:
      break;
    }

    if (baseText && this.isSingleCharacter(baseText))
    {
      if (!aboveText || this.isCombiningAboveAccent(aboveText))
      {
        if (!belowText || this.isCombiningBelowAccent(belowText))

          return {mNode : aNode, mBaseText : baseText, mAboveText : aboveText, mBelowText : belowText};
      }
    }

    return null;
  },

  getTopMathNodeAsAccentedCharacter : function(aNode)
  {
    var nextResult, ourResult;
    var nextTestNode = aNode;
    do
    {
      ourResult = nextResult;
      nextResult = this.treatMathNodeAsAccentedCharacter(nextTestNode, true);
      nextTestNode = nextTestNode.parentNode;
    } while ((nextResult !== null) && (nextTestNode !== null));
    return ourResult;
  },

  getSingleTextNodeContent : function(aNode)
  {
    if (aNode.nodeType === nsIDOMNode.TEXT_NODE)
      return aNode;
    
    var childNodes = this.getSignificantContents(aNode);
    if ( (childNodes.length === 1) && (childNodes[0].nodeType === nsIDOMNode.TEXT_NODE) )
      return childNodes[0];
    
    var kid = this.getSingleWrappedChild(aNode);
    if (kid !== null)
      return this.getLeafNodeText(kid);
    return null;
  },

  findStyleEnclosingObj : function(targNode, objTypeStr, expectedStyle, foundStyle)
  {
    var retStyleNode = null;
    var nodeName = msiGetBaseNodeName(targNode);
    if (nodeName === "mstyle")
    {
      retStyleNode = targNode;
      for (var styleItem in expectedStyle)
      {
        if (targNode.hasAttribute(styleItem))
          foundStyle[styleItem] = targNode.getAttribute(styleItem);
      }
    }
    var wrappedChild = this.getSingleWrappedChild(targNode);
    if (wrappedChild !== null)
    {
      var otherStyle = this.findStyleEnclosingObj(wrappedChild, objTypeStr, expectedStyle, foundStyle);
      if (otherStyle !== null)
        retStyleNode = otherStyle;
    }
    //Following seems to be unnecessary!
//    switch(objTypeStr)
//    {
//      case "fence":
//        if (nodeName === "mrow" && msiNavigationUtils.isFence(targNode))
//          return retStyleNode;
//      break;
//      case "binomial":
//        if (nodeName === "mrow" && msiNavigationUtils.isBinomial(targNode))
//          return retStyleNode;
//      break;
//      default:
//        if (nodeName === objTypeStr)
//          return retStyleNode;
//      break; 
//    }
    return retStyleNode;
  },

  nodeIsPieceOfUnbreakable : function(aNode)
  {
    var retVal = false;
    switch(msiGetBaseNodeName(aNode))
    {
      case "mo":
        if (this.isFence(aNode.parentNode))
        {
          if (this.getFirstSignificantChild(aNode.parentNode) === aNode)
            retVal = true;
          else if (this.getLastSignificantChild(aNode.parentNode) === aNode)
            retVal = true;
        }
      break;
      case "mfrac":
        if (this.isBoundFence(aNode.parentNode))
        {
          var theKids = this.getSignificantContents(aNode.parentNode);
          if ( (theKids.length > 2) && (theKids[1] === aNode) )
            retVal = true;
        }
      break;
      default:
      break;
    }
    return retVal;
  },

  isSpacingObject : function(aNode)
  {
    var retVal = false;
    switch(msiGetBaseNodeName(aNode))
    {
      case "invis":
      case "hspace":
      case "vspace":
      case "msibreak":
      case "msirule":
        retVal = true;
      break;
      default:
      break;
    }
    return retVal;
  },

  isCombiningCharacter : function(aChar)
  {
    //The following is strictly ad hoc - should actually reference functionality in the intl/unicharutil directory somehow
    if (this.isCombiningAboveAccent(aChar) || this.isCombiningBelowAccent(aChar))
      return true;

    switch(aChar)
    {
      case "\u0338":   //The negating slash
        return true;
      break;
    }
    return false;
  },

  isCombiningAboveAccent : function(aChar)
  {
    switch(aChar)
    {
      case "^":
      case "\u0302":
      case "\u02c7":
      case "\u030c":
      case "~":
      case "\u0303":
      case "\u00b4":
      case "\u0301":
      case "\u0060":
      case "\u0300":
      case "\u02d8":
      case "\u0306":
      case "\u00af":
      case "\u0305":
      case "\u02dd":
      case "\u030b":
      case "\u02da":
      case "\u030a":
      case "\u02d9":
      case "\u0307":
      case "\u00a8":
      case "\u0308":
      case "\u20db":
      case "\u20dc":
      case "\u20d7":
        return true;
      break;
    }
    return false;
  },

  isCombiningBelowAccent: function(aChar)
  {
    switch(aChar)
    {
      case "\u00b8":
      case "\u0327":
      case "\u02db":
      case "\u0328":
      case "\u02d3":
      case "\u0323":
      case "\u005f":
      case "\u0332":
        return true;
      break;
    }
    return false;
  },

  upperAccentCombinesWithCharInMath : function(aChar)
  {
    return false;
  },

  lowerAccentCombinesWithCharInMath : function(aChar)
  {
    switch(aChar)
    {
      case "\u0327":
      case "\u0328":
      case "\u0323":
      case "\u0332":
        return true;
      break;
    }
    return false;
  },

  isSingleCharacter : function(someText)
  {
    if (!someText.length)
      return false;
    var rv = true;
    for (var ix = someText.length - 1; rv && (ix > 0); --ix)
    {
      if (!this.isCombiningCharacter(someText[ix]))
        rv = false;
    }
    return rv;
  },

  findSingleCharStart : function(someText, nOffset)
  {
    if (!nOffset || (nOffset >= someText.length))
      return nOffset;
    for (var ix = nOffset - 1; ix > 0; --ix)
    {
      if (!this.isCombiningCharacter(someText[ix]))
        return ix;
    }
    return 0;
  },

  getCharacterRange : function(someText, nOffset)
  {
    if (!nOffset || (nOffset > someText.length))
      return {mStart : nOffset, mEnd : nOffset};
    for (var ix = nOffset - 1; ix > 0; --ix)
    {
      if (!this.isCombiningCharacter(someText[ix]))
        break;
    }
    for (var jx = nOffset; jx < someText.length; ++jx)
    {
      if (!this.isCombiningCharacter(someText[jx]))
        break;
    }
    return {mStart : ix, mEnd : jx};
  },

  upperAccentForcesMath : function(aChar)
  {
    switch(aChar)
    {
      case "\u20db":
      case "\u20dc":
      case "\u20d7":
        return true;
      break;
    }
    return false;
  },

  lowerAccentForcesMath : function(aChar)
  {
    return false;
  },

  cannotSelectNodeForProperties : function(aNode)
  {
    if (this.nodeIsPieceOfUnbreakable(aNode))
      return true;
    return false;
  },

  isInputBox : function(aNode)
  {
    if (msiGetBaseNodeName(aNode) === "mi")
    {
      if (aNode.hasAttribute("tempinput") && aNode.getAttribute("tempinput")==="true")
        return true;
    }
    return false;
  },

  lastOffset : function(aNode)
  {
    if (aNode.nodeType === nsIDOMNode.TEXT_NODE)
      return aNode.textContent.length;
    if (aNode.childNodes)
      return aNode.childNodes.length;
    return 0;
  },

  nodeHasContentBeforeRangeStart : function(aRange, aNode)
  {
    var retVal = false;
    var compVal = this.comparePositions(aNode, 0, aRange.startContainer, aRange.startOffset);
    if (compVal < 0) //start of aNode is before start position of aRange
      retVal = true;
//    var compNode = msiNavigationUtils.getNodeBeforePosition(aRange.startContainer, aRange.startOffset);
//    if (compNode && msiNavigationUtils.isAncestor(aNode, compNode))
//      retVal = true;
//    else if (msiNavigationUtils.isAncestor(aRange.startContainer, aNode))
//      retVal = false;  //in this case, compNode should have been aNode or contained it if aNode had content before aRange
//    else  //No content of aNode is immediately to the left of the range start; now we only want to return true if rangeStart is altogether before aNode.
//    {
//      compNode = aRange.startContainer;
//      compVal = aRange.startContainer.compareDocumentPosition(aNode);
//      retVal = ( (compVal & Node.DOCUMENT_POSITION_FOLLOWING) !== null);
//    }
    return retVal;  
  },

  nodeHasContentAfterRangeStart : function(aRange, aNode)
  {
    var retVal = false;
    var anOffset = this.lastOffset(aNode);
    var compVal = this.comparePositions(aNode, anOffset, aRange.startContainer, aRange.startOffset);
    if (compVal > 0) //end of aNode is after start position of aRange
      retVal = true;
    return retVal;  
  },

  nodeHasContentBeforeSelection : function(aSelection, aNode)
  {
    var bContentFound = true;
    for (var ix = 0; bContentFound && (ix < aSelection.rangeCount); ++ix)
    {
      bContentFound = this.nodeHasContentBeforeRangeStart(aSelection.getRangeAt(ix));
    }
    return bContentFound;
  },

  nodeHasContentAfterSelection : function(aSelection, aNode)
  {
    var bContentFound = true;
    for (var ix = 0; bContentFound && (ix < aSelection.rangeCount); ++ix)
    {
      bContentFound = this.nodeHasContentAfterRangeEnd(aSelection.getRangeAt(ix));
    }
    return bContentFound;
  },

  nodeHasContentAfterRangeEnd : function(aRange, aNode)
  {
    var retVal = false;
    var anOffset = this.lastOffset(aNode);
    var compVal = this.comparePositions(aNode, anOffset, aRange.endContainer, aRange.endOffset);
    if (compVal > 0) //last offset in aNode is after end position of aRange
      retVal = true;
    
//    var compNode = msiNavigationUtils.getNodeAfterPosition(aRange.endContainer, aRange.endOffset);
//    if (compNode && msiNavigationUtils.isAncestor(aNode, compNode))
//      retVal = true;
//    else if (msiNavigationUtils.isAncestor(aRange.endContainer, aNode))
//      retVal = false;
//    else  //No content of aNode is immediately to the right of the range end; now we only want to return true if rangeEnd is altogether before aNode.
//    {
//      compVal = aRange.endContainer.compareDocumentPosition(aNode);
//      retVal = ( (compVal & Node.DOCUMENT_POSITION_PRECEDING) !== null);
//    }
    return retVal;  
  },

  nodeHasContentBeforeRangeEnd : function(aRange, aNode)
  {
    var retVal = false;
    var compVal = this.comparePositions(aNode, 0, aRange.endContainer, aRange.endOffset);
    if (compVal < 0) //start of aNode is before end position of aRange
      retVal = true;
    return retVal;  
  },

  getCommonAncestorForSelection : function(aSelection)
  {
    var topNode = null;
    var parentNodes = [];
    if (aSelection.rangeCount === 1)
      return aSelection.getRangeAt(0).commonAncestorContainer;

    for (var ix = 0; ix < aSelection.rangeCount; ++ix)
      parentNodes.push( aSelection.getRangeAt(ix).commonAncestorContainer );
    return this.findCommonAncestor(parentNodes);
  },

  getRangeContainingSelection : function(aSelection)
  {
    var theRange = aSelection.getRangeAt(0).cloneRange();
    var nextRange = null;
    for (var ix = 1; ix < aSelection.rangeCount; ++ix)
    {
      nextRange = aSelection.getRangeAt(ix);
      if (this.comparePositions(nextRange.startContainer, nextRange.startOffset, theRange.startContainer, theRange.startOffset) < 0)
      {
        theRange.startContainer = nextRange.startContainer;
        theRange.startOffset = nextRange.startOffset;
      }
      if (this.comparePositions(nextRange.endContainer, nextRange.endOffset, theRange.endContainer, theRange.endOffset) > 0)
      {
        theRange.endContainer = nextRange.endContainer;
        theRange.endOffset = nextRange.endOffset;
      }
    }
    return theRange;
  },

  canContainTextNode : function(aNode, editor)
  {
    switch(msiGetBaseNodeName(aNode))
    {
      case "br":
      case "hr":
      case "hspace":
      case "vspace":
        return false;
      break;
    }
    return true;
  },

  nodeCanBeChild : function(aNode, aParent, editor)
  {
    if (this.isMathMLLeafNode(aParent))
      return this.isTextNode(aNode);
    if (this.isTextNode(aNode))
      return this.canContainTextNode(aParent, editor);
    if (this.isMathTemplate(aParent))
      return true;
    if (this.isFence(aParent))
      return true;
    switch(msiGetBaseNodeName(aParent))
    {
      case "mtext":
        return this.isTextNode(aNode);
      break;
      case "mrow":
      case "mstyle":
      case "mphantom":
      case "msqrt":
      case "menclose":
      case "mtd":
        return true;
      break;
      case "mtable":
        return (msiGetBaseNodeName(aNode) == "mtr");
      break;
      case "mtr":
        return (msiGetBaseNodeName(aNode) == "mtd");
      break;
      case "hspace":
      case "vspace":
      case "br":
      case "hr":
        return false;
      break;
      default:
      break;
    }

    var tagManager = editor ? editor.tagListManager : null;
    if (tagManager && !tagManager.nodeCanContainNode( aParent, aNode))
      return false;
    return true;
  }

};


/**************************More general utilities**********************/

// Clone simple JS objects
//function Clone(obj) 
//{ 
//  var clone = {};
//  for (var i in obj)
//  {
//    if( typeof obj[i] === 'object')
//      clone[i] = Clone(obj[i]);
//    else
//      clone[i] = obj[i];
//  }
//  return clone;
//}

var msiSpaceUtils = 
{
//How to properly use the "charContent" fields in the following is unclear. Where there are "dimensions" they provide a much more
//  straightforward way to produce the effects desired without undesirable editing effects (like cursors in the middle of spaces).
//  The current plan is to use the charContent (via CSS rules) only where the dimensions are missing, and in those cases NOT to wrap
//  them in <sw:invis> nodes.

  hSpaceInfo : {
    //requiredSpace :         {charContent: "&#x205f;"},  //MEDIUM MATHEMATICAL SPACE in Unicode?
    //requiredSpace :         {charContent: " "},  //MEDIUM MATHEMATICAL SPACE in Unicode?
    requiredSpace :         {dimensions: "1em", charContent: "&#x205f;"},
    //nonBreakingSpace :      {charContent: "&#x00a0;"},
    nonBreakingSpace :      {charContent: " "},
    emSpace :               {dimensions: "1em", charContent: "&#x2003;"},
    twoEmSpace :            {dimensions: "2em", charContent: "&#x2001;"}, //EM QUAD
    thinSpace :             {dimensions: "0.17em", charContent: "&#x2009;"},
    thickSpace :            {dimensions: "0.5em", charContent: "&#x2002;"},   //"EN SPACE" in Unicode?
    italicCorrectionSpace : {dimensions: "0.083en", charContent: "&#x200a;"},   //the "HAIR SPACE" in Unicode?
    negativeThinSpace :     {dimensions: "0.0em"},
    zeroSpace :             {dimensions: "0.0em", charContent: "&#x200b;"},
    noIndent :              {dimensions: "0.0em", showInvisibleChars: "&#x2190;"} 
  },

  vSpaceInfo : {
    smallSkip :             {dimensions: "3pt"},
    mediumSkip :            {dimensions: "6pt"},
    bigSkip :               {dimensions: "12pt"},
    strut :                 {lineHeight: "100%"},
    mathStrut:              {lineHeight:  "100%"}   //not really right, but for the moment
  },

  breaksInfo : {
    allowBreak :            {charContent: "&#x200b;"},  //this is the zero-width space  -   showInvisibleChars:  "|"?
    discretionaryHyphen :   {charContent: "&#x00ad;", showInvisibleChars: "-"},
    noBreak:                {charContent: "&#x2060;", showInvisibleChars: "~"},
    pageBreak:              {charContent: "<newPageRule></newPageRule>"},  //formfeed?  - showInvisibleChars: "&#x21b5;"?
    newPage:                {charContent: "<newPageRule/>"},  //formfeed?  - showInvisibleChars: "&#x21b5;"?
    lineBreak:              {charContent: "<br xmlns=\"http://www.w3.org/1999/xhtml\"></br>", showInvisibleChars: "&#x21b5;"},
    newLine:                {charContent: "<br xmlns=\"http://www.w3.org/1999/xhtml\"></br>", showInvisibleChars: "&#x21b5;"}
//    lineBreak:              {charContent: "<br xmlns=\"" + xhtmlns + "\"></br>", showInvisibleChars: "&#x21b5;"},
//    newLine:                {charContent: "<br xmlns=\"" + xhtmlns + "\"></br>", showInvisibleChars: "&#x21b5;"}
  },
  
  spaceInfoFromChars : function(charStr)
  {
    var retData = null;
    var spaceTypes = ["hspace", "vspace", "msibreak"];
    var spaceInfoTypes = ["hSpaceInfo", "vSpaceInfo", "breaksInfo"];
    if (charStr === " ")
      retData = {theType: "hspace", theSpace: "normalSpace"};
    for (var ix = 0; !retData && (ix < spaceInfoTypes.length); ++ix)
    {
      for (var anInfo in this[spaceInfoTypes[ix]])
      {
        if ( ("charContent" in this[spaceInfoTypes[ix]][anInfo]) && (this[spaceInfoTypes[ix]][anInfo].charContent === charStr) )
        {
          retData = {theType: spaceTypes[ix], theSpace : anInfo};
          break;
        }
      }
    }
    if (!retData && msiNavigationUtils.isWhiteSpace(charStr))
      retData = {theType: "hspace", theSpace: "normalSpace"};  //punt, but not entirely
    return retData;
  },

  getSpaceInfoFromNode : function(aNode)
  {
    var retData = null;
    var spaceTypes = ["hspace", "vspace", "msibreak"];
    var spaceInfoTypes = ["hSpaceInfo", "vSpaceInfo", "breaksInfo"];
    var nodeName = msiGetBaseNodeName(aNode);
    var nodeType = aNode.getAttribute("type");
    var valStr = "";
    switch(nodeName)
    {
      case "hspace":
      case 'vspace':
//      case 'msibreak':
        retData = {theType : nodeName, theSpace : nodeType};
        if ((nodeType === "customSpace") || (nodeType === "stretchySpace"))  //second shouldn't be necessary, but heretofore was apparently what we produced
        {
          retData.theSpace = "customSpace";  //just in case it came in anomalously
          valStr = aNode.getAttribute("dim");
          if (valStr && valStr.length)
            retData.theDim = valStr;
          if (aNode.getAttribute("class") === "stretchySpace")
          {
            retData.customType = "stretchy";
            valStr = aNode.getAttribute("flex");
            if (valStr && valStr.length)
              retData.stretchFactor = valStr;
            valStr = aNode.getAttribute("fillWith");
            if (valStr && valStr.length)
              retData.fillWith = valStr;
          }
          else
            retData.customType = "fixed";
          valStr = aNode.getAttribute("atEnd");
          if (valStr && valStr.length)
            retData.atEnd = valStr;
        }
      break;
      case 'msibreak':
        retData = {theType : nodeName, theSpace : nodeType};
        if (nodeType === "customNewLine")
        {
          valStr = aNode.getAttribute("dim");
          if (valStr && valStr.length)
            retData.theDim = valStr;
        }
      break;
      default:
      break;
    }
    return retData;
  },

  getHSpaceDims : function(spaceName)
  {
    var retDims = null;
    if ( (spaceName in this.hSpaceInfo) && ("dimensions" in this.hSpaceInfo[spaceName]) )
      retDims = this.hSpaceInfo[spaceName].dimensions;
    return retDims;
  },

  getVSpaceDims : function(spaceName)
  {
    var retDims = null;
    if ( (spaceName in this.vSpaceInfo) && ("dimensions" in this.vSpaceInfo[spaceName]) )
      retDims = this.vSpaceInfo[spaceName].dimensions;
    return retDims;
  },

  getVSpaceLineHeight : function(spaceName)
  {
    var retHt = null;
    if ( (spaceName in this.vSpaceInfo) && ("lineHeight" in this.vSpaceInfo[spaceName]) )
      retHt = this.vSpaceInfo[spaceName].lineHeight;
    return retHt;
  },

  getHSpaceCharContent : function(spaceName)
  {
    var theContent = null;
    if ( (spaceName in this.hSpaceInfo) && ("charContent" in this.hSpaceInfo[spaceName]) )
      theContent = this.hSpaceInfo[spaceName].charContent;
    return theContent;
  },

  getHSpaceDisplayableContent : function(spaceName)
  {
    var retStr = null;
    if (this.getHSpaceDims(spaceName) === null)
    {
      retStr = this.getHSpaceCharContent(spaceName);
      var invisContent = this.getHSpaceShowInvis(spaceName);
      if (invisContent && invisContent.length)
      {
        if (!retStr)
          retStr = "";
        retStr += "<sw:invis>" + invisContent + "</sw:invis>";
      }
    }
    return retStr;
  },

  getVSpaceCharContent : function(spaceName)
  {
    var theContent = null;
    if ( (spaceName in this.vSpaceInfo) && ("charContent" in this.vSpaceInfo[spaceName]) )
      theContent = this.vSpaceInfo[spaceName].charContent;
    return theContent;
  },

  getVSpaceDisplayableContent : function(spaceName)
  {
    var retStr = null;
    if ( (this.getVSpaceDims(spaceName) === null) && (this.getVSpaceLineHeight(spaceName) === null) )
    {
      retStr = this.getVSpaceCharContent(spaceName);
      var invisContent = this.getVSpaceShowInvis(spaceName);
      if (invisContent && invisContent.length)
      {
        if (!retStr)
          retStr = "";
        retStr += "<sw:invis>" + invisContent + "</sw:invis>";
      }
    }
    return retStr;
  },

  getBreakCharContent : function(breakName)
  {
    var theContent = null;
    if ( (breakName in this.breaksInfo) && ("charContent" in this.breaksInfo[breakName]) )
      theContent = this.breaksInfo[breakName].charContent;
    return theContent;
  },

  getHSpaceShowInvis : function(spaceName)
  {
    var theInvisChars = null;
    if ( (spaceName in this.hSpaceInfo) && ("showInvisibleChars" in this.hSpaceInfo[spaceName]) )
      theInvisChars = this.hSpaceInfo[spaceName].showInvisibleChars;
    return theInvisChars;
  },

  getVSpaceShowInvis : function(spaceName)
  {
    var theInvisChars = null;
    if ( (spaceName in this.vSpaceInfo) && ("showInvisibleChars" in this.vSpaceInfo[spaceName]) )
      theInvisChars = this.vSpaceInfo[spaceName].showInvisibleChars;
    return theInvisChars;
  },

  getBreakShowInvis : function(breakName)
  {
    var theInvisChars = null;
    if ( (breakName in this.breaksInfo) && ("showInvisibleChars" in this.breaksInfo[breakName]) )
      theInvisChars = this.breaksInfo[breakName].showInvisibleChars;
    return theInvisChars;
  },

  getBreakDisplayableContent : function(breakName)
  {
    var retStr = this.getBreakCharContent(breakName);
    var invisContent = this.getBreakShowInvis(breakName);
    if (invisContent && invisContent.length)
    {
      if (!retStr)
        retStr = "";
      retStr += "<sw:invis>" + invisContent + "</sw:invis>";
    }
    return retStr;
  }

};

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

function msiDumpWithID(str, element)
{
  var replStr = "";
  if (element!==null && element.id)
    replStr = element.id;
  dump( str.replace("@", replStr) );
}

function msiKludgeTestKeys(keyArray)
{
  var keysInUse = [];
//#if DEBUG_Ron            BBM: I commented this out so that I could change this file without building every time.
//  keysInUse.push("search");
//  keysInUse.push("tableEdit");
//  keysInUse.push("spaces");
//  keysInUse.push("reviseChars");
//  keysInUse.push("editorFocus");
//  keysInUse.push("bibliography");
//#endif

  var bDoIt = false;
  if (keysInUse.length && keyArray && keyArray.length)
  {
    for (var ix = 0; ix < keyArray.length; ++ix)
    {
      if (keysInUse.indexOf( keyArray[ix] ) >= 0)
      {
        bDoIt = true;
        break;
      }
    }
  }
  return bDoIt;
}

function msiKludgeLogString(logStr, keyArray)
{
  var bDoIt = msiKludgeTestKeys(keyArray);
  if (bDoIt)
    dump(logStr);
}

function msiKludgeLogNodeContents(aNode, keyArray, prefaceStr, bIncludePosInParent)
{
  return msiKludgeLogNodeContentsAndAttributes(aNode, keyArray, prefaceStr, bIncludePosInParent, []);
//  var bDoIt = msiKludgeTestKeys(keyArray);
//  if (!bDoIt)
//    return;
//  var retStr = "Node";
//  if (prefaceStr && prefaceStr.length)
//    retStr = prefaceStr;
//  if (bIncludePosInParent)
//  {
//    if (aNode.parentNode)
//      retStr += " is at position [" + msiNavigationUtils.offsetInParent(aNode) + "] in its parent, and";
//    else
//      retStr += " has no parent node, and";
//  }
//  if (msiNavigationUtils.isTextNode(aNode))
//    retStr += " is a text node, with content [" + aNode.textContent + "].\n";
//  else
//  {
//    retStr += " is a [" + aNode.nodeName + "] node with [" + aNode.childNodes.length + "] children:";
//    for (var ix = 0; ix < aNode.childNodes.length; ++ix)
//    {
//      retStr += "\n  child [" + ix + "] is a [" + aNode.childNodes[ix].nodeName + "] + with text content [" + aNode.childNodes[ix].textContent + "]";
//    }
//    retStr += "\n";
//  }
//  dump(retStr);
}

function msiKludgeLogNodeContentsAndAllAttributes(aNode, keyArray, prefaceStr, bIncludePosInParent)
{
  return msiKludgeLogNodeContentsAndAttributes(aNode, keyArray, prefaceStr, bIncludePosInParent, [], true);
}

function msiKludgeLogNodeContentsAndAttributes(aNode, keyArray, prefaceStr, bIncludePosInParent, attribList, bAllAttrs)
{
  var bDoIt = msiKludgeTestKeys(keyArray);
  if (!bDoIt)
    return;
  var retStr = "Node";
  if (prefaceStr && prefaceStr.length)
    retStr = prefaceStr;
  if (!aNode)
  {
    retStr += " is null!\n";
    dump(retStr);
    return;
  }
  if (bIncludePosInParent)
  {
    if (aNode.parentNode)
      retStr += " is at position [" + msiNavigationUtils.offsetInParent(aNode) + "] in its parent, and";
    else
      retStr += " has no parent node, and";
  }
  if (msiNavigationUtils.isTextNode(aNode))
    retStr += " is a text node, with content [" + aNode.textContent + "].\n";
  else
  {
    retStr += " is a [" + aNode.nodeName + "] node with [" + aNode.childNodes.length + "] children.";
    retStr += msiKludgeGetAttributesString(aNode, "", attribList, bAllAttrs);
//    var thisAttr = null;
//    if (bAllAttrs)
//    {
//      var attList = aNode.attributes;
//      for (ix = 0; ix < attList.length; ++ix)
//      {
//        thisAttr = attList.item(ix);
//        retStr += "\n    attribute [" + thisAttr.name + "] has value [" + thisAttr.value + "];";
//      }
//    }
//    else
//    {
//      for (ix = 0; ix < attribList.length; ++ix)
//      {
//        retStr += "\n    attribute [" + attribList[ix];
//        if (aNode.hasAttribute(attribList[ix]))
//          retStr += "has value [" + aNode.getAttribute(attribList[ix]) + "];";
//        else
//          retStr += "] is not present;";
//      }
//    }
    for (var ix = 0; ix < aNode.childNodes.length; ++ix)
    {
      retStr += "\n  child [" + ix + "] is a [" + aNode.childNodes[ix].nodeName + "] + with text content [" + aNode.childNodes[ix].textContent + "]";
      retStr += msiKludgeGetAttributesString(aNode.childNodes[ix], "  ", attribList, bAllAttrs);
    }
    retStr += "\n";
  }
  dump(retStr);
}

function msiKludgeGetAttributesString(aNode, prefaceStr, attribList, bAllAttrs)
{
  var retStr = "";
  var thisAttr = null;
  if (bAllAttrs)
  {
    var attList = aNode.attributes;
    for (var ix = 0; attList && (ix < attList.length); ++ix)
    {
      thisAttr = attList.item(ix);
      retStr += "\n    " + prefaceStr + " attribute [" + thisAttr.name + "] has value [" + thisAttr.value + "];";
    }
  }
  else
  {
    for (var ix = 0; ix < attribList.length; ++ix)
    {
      retStr += "\n    " + prefaceStr + " attribute [" + attribList[ix];
      if (aNode.hasAttribute(attribList[ix]))
        retStr += "] has value [" + aNode.getAttribute(attribList[ix]) + "];";
      else
        retStr += "] is not present;";
    }
  }
  return retStr;
}

function msiAuxDirFromDocPath(documentURIString)
{
  var spec = unescape(documentURIString);
  var i = spec.lastIndexOf(".");
  if (i > 0) spec = spec.substr(0,i);
  dump("spec is " + spec + "\n");
  spec = spec+"_files";
  var url = msiURIFromString(spec);
  var dir = msiFileFromFileURL(url);
  if (!dir.exists()) dir.create(1, 0755);
  return dir.clone();
}


// receives a style string, sets a new value for attribute
// if the new value is null, we remove the style attribute
function setStyleAttribute(elem, attribute, value )
{
  var inStyleString = elem.getAttribute("style");
  var array = inStyleString.split(";");
  var i, foundindex;
  var found = false;
  for (i=0; i<array.length; i++)
  {
    array[i] = array[i].split(":");
    array[i][0] = (array[i][0]).replace(" ","","g");
  }
  for (i=0; i<array.length; i++)
  {
    if (array[i][0] === attribute)
      {
        array[i][1] = value;
        found = true;
      foundindex = i;
      }
      array[i] = array[i].join(":"); 
  }  
  if (value===null && found) array.splice(foundindex,1);  
  var outStyleString = array.join(";");
  if (!found)  outStyleString = attribute+": "+value+"; "+outStyleString;
  elem.setAttribute("style", outStyleString);
}
      

function msiToPixels(distance,unit)
{
  return msiCSSUnitsList.convertUnits(distance, unit, 'in')*100;  //100 pixels/inch
}

// a timer for automatic soft saves


function SS_Timer(delayMS, editor, editorElement) {
  this.debuginfo = "soft save timer";
  this.editor = editor;
  this.editorElement = editorElement;
  this.modCount = 0;
  var prefService = Components.classes["@mozilla.org/preferences-service;1"]
                              .getService(Components.interfaces.nsIPrefBranch);

  var pbi = prefService.QueryInterface(Components.interfaces.nsIPrefBranch2);

  var interval = prefService.getIntPref("swp.saveintervalseconds");
  if (!interval || interval === 0) return;
  this.timer_ = Components.classes["@mozilla.org/timer;1"].createInstance(Components.interfaces.nsITimer);
//  this.observerService_ = new G_ObserverServiceObserver(
//                                        'xpcom-shutdown',
//                                        BindToObject(this.cancel, this));

  // Ask the timer to use nsITimerCallback (.notify()) when ready
  // Interval is the time between saves in seconds
  this.timer_.initWithCallback(this, interval*1000, 1);
}

SS_Timer.prototype.callback_ = function()
{
  var modCt = this.editor.getModificationCount();
  if (modCt !== this.modCount) doSoftSave(this.editorElement, this.editor);
  this.modCount = modCt;
  return true;
}



/**
 * Cancel this timer 
 */
SS_Timer.prototype.cancel = function() {
  if (!this.timer_) {
    return;
  }

  this.timer_.cancel();
  // Break circular reference created between this.timer_ and the SS_Timer
  // instance (this)
  this.timer_ = null;
  this.callback_ = null;

//  // We don't need the shutdown observer anymore
//  this.observerService_.unregister();
}

/*
 * Invoked by the timer when it fires
 * 
 * @param timer Reference to the nsITimer which fired (not currently 
 *              passed along)
 */
SS_Timer.prototype.notify = function(timer) {
  // fire callback and save results
  var ret = this.callback_();
  
  return ret;
}

/**
 * XPCOM cruft
 */
SS_Timer.prototype.QueryInterface = function(iid) {
  if (iid.equals(Components.interfaces.nsISupports) ||
      iid.equals(Components.interfaces.nsITimerCallback))
    return this;

  throw Components.results.NS_ERROR_NO_INTERFACE;
}

function processingInstructionsList( doc, target, fNodes )
{
  var list =[];
  var regexp;
  var a;
  var treeWalker = doc.createTreeWalker(
    doc,
    NodeFilter.SHOW_PROCESSING_INSTRUCTION,
    { 
      acceptNode: function(node) { 
        if (node.target === target)
          return NodeFilter.FILTER_ACCEPT;
        else return NodeFilter.FILTER_REJECT; }
    },
    false
   );
   regexp = /href\=[\'\"]([^\'\"]*)/i;
   try {
    while(treeWalker.nextNode()){
      if (fNodes)
      {
        list.push(treeWalker.currentNode);
      }
      else {
        a = regexp(treeWalker.currentNode.textContent);
        list.push(a[1])
      }
    }
  }
  catch(e) {
    dump("treeWalker error: " + e.toString() + "\n");
  }
  return list;
}


function deleteProcessingInstructions( doc, target )
{
  var list =[];
  var regexp;
  var i;
  var arr=[];
  var editor;
  var arrToDelete =[];
  var editorElement = msiGetActiveEditorElement();
  if (editorElement)
    editor = msiGetEditor(editorElement);
  if (!editor) {
    dump("Cannot find editor in 'deleteProcessingInstructions'");
    return;
  }
  var treeWalker = doc.createTreeWalker(
    doc,
    NodeFilter.SHOW_PROCESSING_INSTRUCTION,
    { 
      acceptNode: function(node) { 
        if (node.target === target)
          return NodeFilter.FILTER_ACCEPT;
        else return NodeFilter.FILTER_REJECT; }
    },
    false
  );
  regexp = /href\=[\'\"]([^\'\"]*)/i;
  try {
    while(treeWalker.nextNode()){
      arr.push(treeWalker.currentNode);
    }
    for (i=0; i<arr.length; i++) arr[i].parentNode.removeChild(arr[i]);
  }
  catch(e) {
    dump("treeWalker error: "+e.toString());
  }
  return list;
}

function addProcessingInstruction(doc, target, data, type)
{
  var editor;
  var editorElement = msiGetActiveEditorElement();
  if (editorElement)
    editor = msiGetEditor(editorElement);
  if (!editor) {
    dump("Cannot find editor in 'deleteProcessingInstructions'");
    return;
  }
  dump("This is a place to put a break point\n");
  try {
    var contents = 'href="'+data+'" type="'+type+'"';
    var pi = doc.createProcessingInstruction(target, contents);
    var root = doc.getElementsByTagName('html')[0];
    root.parentNode.insertBefore(pi,root);
  }
  catch(e)
  {
    dump(e.toString()+"\n");
  }
}



function getUserResourceFile( name, resdirname )
{
  var dsprops, userAreaFile, resdir, file, basedir;
  dsprops = Components.classes["@mozilla.org/file/directory_service;1"].getService(Components.interfaces.nsIProperties);
  basedir = dsprops.get("ProfD", Components.interfaces.nsIFile);
  userAreaFile = basedir.clone();
  userAreaFile.append(name);
  if (!userAreaFile.exists())
  { // copy from resource area
    resdir = dsprops.get("resource:app", Components.interfaces.nsIFile);
    if (resdir) resdir.append("res");
    if (resdir) {
      file = resdir.clone();
      if (resdirname && resdirname.length > 0) file.append(resdirname);
      file.append(name);
      try {
        if (file.exists()) file.copyTo(basedir,"");
      }
      catch(e) {
        dump("failed to copy: "+e.toString());
      }
    }
    userAreaFile = file.clone();
  }
  return userAreaFile;
}
 



function getResourceFile( name, resdirname )
{
  var dsprops, resfile;
  dsprops = Components.classes["@mozilla.org/file/directory_service;1"].getService(Components.interfaces.nsIProperties);
  resfile = dsprops.get("resource:app", Components.interfaces.nsIFile);
  if (resfile) resfile.append("res");
  if (resfile) {
    if (resdirname && resdirname.length > 0) resfile.append(resdirname);
    resdir.append(name);
  }    
  return resfile;
}
 
var wordSep = /\s+/;  // this needs to be localizable BBM

function countNodeWords(node) //the intent is to count words in a text node
{
  var s = node.textContent;
  var arr = s.split(wordSep);
  var n = arr.length;
  var countOfEmpties = 0;
  // we can get empty 'words' at the ends
  if (n > 0 && arr[0].length === 0) countOfEmpties = 1;
  if (n > 1 && arr[n-1].length === 0) countOfEmpties++;
  return n - countOfEmpties;
}

var xpath;
function countWords(doc)
{
  var iterator;
  doc.documentElement.normalize();
// a hack for saying /html/body//text() without messing with namespaces and resolvers
  iterator = doc.evaluate("/*[1]/*[2]//text()", doc, null, XPathResult.UNORDERED_NODE_ITERATOR_TYPE, null );
  var wordcount = 0;
  var thisNode;
  try {
    var thisNode = iterator.iterateNext();
  
    while (thisNode) {
      wordcount += countNodeWords(thisNode);
      thisNode = iterator.iterateNext();
    }   
  }
  catch (e) {
    dump( 'Error: Document tree modified during iteration ' + e );
  }
  return wordcount;
}

function getTagsXPath( editor, tagcategory ) // tagcategory is one of texttag, paratag, structtag, etc.
{
  var tagarray, taglist, theTagManager, i, length, xpath;
  theTagManager = editor ? editor.tagListManager : null;
  if (!theTagManager) return;
  taglist = theTagManager.getTagsInClass(tagcategory,',',false); 
  tagarray = taglist.split(',');
  length = tagarray.length;
  xpath = "";
  for (i=0; i< length; i++)
  {
    xpath += (i>0?"|":"") + "html:" + tagarray[i];
  }
  return xpath;
}

function gotoFirstNonspaceInElement( editor, node )
{
  var i, len, re, arr, currNode, firstNode;
  re = /\S/;
  var treeWalker = document.createTreeWalker(
      node,
      NodeFilter.SHOW_TEXT,
      { acceptNode: function(node) { return NodeFilter.FILTER_ACCEPT; } },
      false
  );

  firstNode = treeWalker.nextNode();
  if (firstNode){
    editor.selection.collapse(firstNode,0);
	currNode = firstNode;
  }
  else{
    editor.selection.collapse(node,0);
	currNode = node;
  }
  // put the selection at the beginning of the first text node in case there is only white space
  for ( ; currNode !== null; currNode = treeWalker.nextNode())
  {
    if (arr = re.exec(currNode.textContent) !== null)
    {  // there is a match, and it begins at arr.index
      dump("Node #"+i+"is '" + currNode.textContent + "' and non-whitespace starts at "+arr.index+"\n");
      editor.selection.collapse(currNode,arr.index);
      break;
    }
  }
  var selectionController = editor.selectionController;
  selectionController.scrollSelectionIntoView(selectionController.SELECTION_NORMAL,selectionController.SELECTION_ANCHOR_REGION, 
    false);
}


// since the onkeypress event gets called *before* the value of a text box is updated,
// we handle the updating here. This function takes a textbox element and an event and sets
// the value of the text box
// This is used for number boxes

function updateTextNumber(textelement, id, event)
{
  if (event && (event.type === "keypress"))
  {
    var val = textelement.value;
    var textbox = document.getElementById(id);
    //textbox is the text box; textelement in the underlying html:input
    var selStart = textelement.selectionStart;
    var selEnd = textelement.selectionEnd;
    var keycode = event.keyCode;
    var charcode = event.charCode;
  
    if (keycode === event.DOM_VK_BACK_SPACE) {
      if (selStart > 0) {
        selStart--;
        val = val.slice(0,selStart)+val.slice(selEnd);
      }
    }
    else 
    if (keycode === event.DOM_VK_DELETE) {
      selEnd++;
      val = val.slice(0,selStart)+val.slice(selEnd);
    }
    else
    if (charcode === "-".charCodeAt(0) || charcode === "+".charCodeAt(0) || charcode === ".".charCodeAt(0) ||
      (charcode >= 48 && charcode <58))
    {
      if (selEnd >= selStart) val = val.slice(0,selStart)+ String.fromCharCode(charcode)+val.slice(selEnd);
      selStart++;
    }
    else return;
    // now check to see if we have a string
    try {
      if (!isNaN(Number(val))) 
      {
        textelement.value = val;
        textelement.setSelectionRange(selStart, selStart)
        event.preventDefault();
      }
    }
    catch(e)
    {
      dump(e.toString + "\n");
    }
  }
  var eventsource = document.getElementById(id);
  if (eventsource) eventsource._validateValue(eventsource.inputField.value,false,true);
}


function goUp(id)
{
  var element = document.getElementById(id);
  var value = Number(element.value);
  if (value === NaN) return;
  var max = Number(element.getAttribute("max"));
  if ((max === 0)||(max === NaN)) max = Number.MAX_VALUE;
  value += Number(element.getAttribute("increment"));
  value = Math.min(max,value);
  element.value = unitRound(value);
}

function goDown(id)
{
  var element = document.getElementById(id);
  var value = Number(element.value);
  if (value === NaN) return;
  var min = Number(element.getAttribute("min"));
  if (min === NaN) min = 0;
  value -= Number(element.getAttribute("increment"));
  value = Math.max(min,value);
  element.value = unitRound(value);
}

// msiFileURLFromAbsolutePath
// Takes an absolute path (the direction of the slashes is OS-dependent) and
// produces a file URL

function msiFileURLFromAbsolutePath( absPath )
{
  try {
    var file = Components.classes["@mozilla.org/file/local;1"].  
                         createInstance(Components.interfaces.nsILocalFile);  
    file.initWithPath( absPath );
    return msiFileURLFromFile( file );
  }
  catch (e)
  {
    dump("//// error in msiFileURLFromAbsolutePath: "+e.message+"\n");
    return null;
  }
}                       

function msiFileURLFromChromeURI( chromePath )  //chromePath is a nsURI
{
  var retPath;
  if (!chromePath || !(/^chrome:/.test(chromePath)))
  {
    dump("In msiFileURLFromChrome, path [" + chromePath + "] isn't a chrome URL! Returning null.\n");
    return retPath;
  }
   
  var ios = Components.classes['@mozilla.org/network/io-service;1'].getService(Components.interfaces.nsIIOService);
  var uri = ios.newURI(chromePath, "UTF-8", null);
  var cr = Components.classes['@mozilla.org/chrome/chrome-registry;1'].getService(Components.interfaces.nsIChromeRegistry);
  retPath = cr.convertChromeURL(uri);
  var pathStr = retPath.spec;

  if (/^jar:/.test(pathStr))
    pathStr = pathStr.substr(4);  //after the "jar:"
  if (!(/^file:/.test(pathStr)))
    pathStr = "file://" + pathStr;
  if (pathStr !== retPath.spec)
    retPath = msiURIFromString(pathStr);
  retPath = msiFileURLFromAbsolutePath(retPath.path);

  return retPath;
}

function msiFileURLFromFile( file )
{
  // file is nsIFile  
  var ios = Components.classes["@mozilla.org/network/io-service;1"].  
                      getService(Components.interfaces.nsIIOService);  
  return ios.newFileURI(file);  
}

function msiURIFromString(str)
{
  var ios = Components.classes["@mozilla.org/network/io-service;1"].  
                      getService(Components.interfaces.nsIIOService);  
  return ios.newURI(str, null, null);  
}
  
function msiFileURLStringFromFile( file )
{
  return  msiFileURLFromFile( file ).spec;
}

function msiFileFromFileURL(url)
{
  try {
    return url.QueryInterface(Components.interfaces.nsIFileURL).file;
  }
  catch (e)
  {
    dump("Error in msiFileFromFileURL: url = "+url.spec+" "+e.message+"\n");
  }
}
  
function msiPathFromFileURL( url )     // redundant BBM: remove instances of this or of GetFilePath
{
  //return GetFilepath( url );
  // or
  return msiFileFromFileURL(url).path; 
}


//The purpose of this function is to remove elements of "theArray" which aren't in "otherArray". In particular,
//it leaves the order of elements in "theArray" unchanged.
function intersectArrayWith(theArray, otherArray)
{
  for (var i = theArray.length - 1; i >= 0; --i)
  {
    if (findInArray(otherArray, theArray[i]) < 0)
      theArray.splice(i, 1);
  }
  return theArray;
}

//The purpose of this function is to add elements of "otherArray" which aren't in "theArray". In particular,
//it leaves the order of elements in "theArray" unchanged.
function unionArrayWith(theArray, otherArray)
{
  for (var i = 0; i < otherArray.length; ++i)
  {
    if (findInArray(theArray, otherArray[i]) < 0)
      theArray.push(otherArray[i]);
  }
  return theArray;
}

 
function openAllSubdocs()
{
  netscape.security.PrivilegeManager.enablePrivilege('UniversalXPConnect');
  var editorElement = msiGetActiveEditorElement();
  var editor;
  var tagarray;
  if (editorElement) editor = msiGetEditor(editorElement);
  if (!editor) return;
  try {
    var doc = editor.document;
    var theTagManager = editor.tagListManager;
    if (!theTagManager) return;
    var taglist;
    var list;
    taglist = theTagManager.getTagsInClass('structtag',',',false); 
    tagarray = taglist.split(',');
    var i,j;
    for (i=0; i<tagarray.length; i++)
    {
      list=doc.getElementsByTagName(tagarray[i]);
      for (j=0; j<list.length; j++)
      {                          
        if (list[j].hasAttribute("subdoc") && list[j].hasAttribute("open")
          && list[j].getAttribute("open") === "false")
        {
          var node = list[j];
          var fname = node.getAttribute("subdoc");
          var ioService = Components.classes['@mozilla.org/network/io-service;1']  
                .getService(Components.interfaces.nsIIOService);  
          var fileHandler = ioService.getProtocolHandler('file')  
                  .QueryInterface(Components.interfaces.nsIFileProtocolHandler);  
          var file = fileHandler.getFileFromURLSpec(node.baseURI);  
          file = file.parent; // and now it points to the working directory
          file.append(fname+".xml");
          // now convert to URL
          var url = ioService.newFileURI(file);  
          var fileURL = url.spec;  
          var req = new XMLHttpRequest();
          req.open("GET", fileURL, false); 
          req.send(null);
          // print the name of the root element or error message
          var dom = req.responseXML;
          dump(dom.documentElement.nodeName === "parsererror" ? "error while parsing" : dom.documentElement.nodeName);
          if (dom.documentElement.nodeName === "parseerror") break;
          var loadedNode = dom.documentElement;
          var children = loadedNode.childNodes;
          var i = 0;
          while (children[i].nodeType !== Node.ELEMENT_NODE) i++;
          i++;
          while (i < children.length){
            node.appendChild(doc.adoptNode(children[i]));
          } 
          list[j].setAttribute("open","true");
          file.remove(false);
        }
      }
    }
  }
  catch(e)
  {
    dump(e.message+"\n");
  }    
}          

/* The following code is for pretty printing. The idea is to cause little change in the text form for little
   changes in the document. This is done by having canonical indents for all elements (except text tags
   will be inline), and to change the line breaks minimally. That is long lines will be broken, very short lines
   will be consolidated, but the changes should not propagate to the end of a paragraph. */

var indentIncrement = "  ";  // should eventually come from a user 
var maxLengthDefault = 100; // should come from prefs; if a line is longer than this, we must break it
var minLengthDefault = 60; // if a line is shorter than this, we at least try to consolidate it
var maxLength; 
var minLength; 
var reallyMinLength = 50; 

function replacer(str, p1, p2, offset, s)
{
  switch (str)
  {
    case "\"": return "&quot;"; break;
    case "&" : return "&amp;"; break; 
    case "<" : return "&lt;"; break;
    case ">" : return "&gt;"; break;
    default: return str; break;
  }
}

function reversereplacer(str, p1, p2, offset, s)
{
  switch (str)
  { 
    case "&quot;": return "\""; break;
    case "&lt;"  : return "<"; break;
    case "&gt;"  : return ">"; break;
    default      : return str; break;
  }
}

function encodeEntities(instring)
{
  return instring.replace(/[&"<>]/g, replacer, "g");
}

function decodeEntities(instring)
{
  return instring.replace(/&amp;|&quot;|&lt;|&gt;/g, reversereplacer, "g");
}


function isEmptyText(textnode)
{                                                                   
  return !/\S/.test(textnode.textContent);
}

function writeLineInPieces( output, currentline )
{
  var lastLength = 10000000;
  var L;
  while (currentline.s.length > 0)
  {
    L = currentline.s.length;
		if (L < maxLength || L >= lastLength) // this assures us we get out of the while loop
    {
      output.s += currentline.s.replace("\n"," ", "g") + "\n";
      currentline.s = "";
    }
    else  // see if there is an existing linebreak between minLength and maxLength
    {
      lastLength = L;
      var index = currentline.s.indexOf("\n", minLength);
      var firstLine;
      if (index < 0 || index > maxLength)
      { // no linebreaks where we want. Look harder at shorter lines
        index = currentline.s.indexOf("\n", reallyMinLength);
      } 
      if (index >=0 && index <= maxLength) 
      {
        firstLine = currentline.s.substr(0,index);
        output.s += firstLine.replace("\n"," ","g")+"\n";
        firstLine = currentline.s.substr(index+1);
        currentline.s = firstLine;
      }    
      else // no convenient linebreaks, look for spaces
      {
        index = currentline.s.lastIndexOf(" ", maxLength);
        var forced = false;
        if (index <0 || index > maxLength) // no spaces? Japanese? force a linebreak at maxLength -5
        {
          forced = true;
          // we can't break text in a tag. Try to find previous '<'
          index = currentline.s.lastIndexOf("<", maxLength);
          if (index === -1) index = maxLength - 5;
        }
        firstLine = currentline.s.substr(0,index);
        output.s += firstLine+"\n";
        if (!forced) index++;
        firstLine = currentline.s.substr(index);
        currentline.s = firstLine;
      }                                                              
    }                                              
  }
}

function newline(output, currentline, indent)
{
  if (/\S/.test(currentline.s))
  {
    writeLineInPieces(output, currentline);
  }    
  currentline.s ="";
  if (indent) 
    for (var i = 0; i < indent; i++) currentline.s += indentIncrement;
}

var nonInlineTags=".math.html.head.requirespackage.newtheorem.definitionslist.documentclass.preamble.usepackage.preambleTeX."+
  "msidisplay.pagelayout.page.textregion.columns.header.footer.plot."+
  "titleprototype.docformat.numberstyles.sectitleformat.docformat.numberstyles.texprogram.";
function isInlineElement(editor, element)
{
  if (nonInlineTags.search("."+element.localName+".") >= 0) return false;
  if (msiNavigationUtils.isMathNode(element)) return false;
  var tagclass = editor.tagListManager.getClassOfTag(element.localName, null);
  if (tagclass === "texttag" || tagclass === "othertag" || tagclass.length === 0) return true;
  return false;
}
  
/* ELEMENT_NODE =1 */
function processElement( editor, node, treeWalker, output, currentline, indent )
{
//  dump("ProcessElement, indent = "+indent+"\n");
  var inline = isInlineElement(editor, node);
  if (!inline) 
    newline(output, currentline, indent);
  currentline.s += "<" + node.nodeName;
  if (node.hasAttributes())
  {
    var attrs = node.attributes;
    var len = attrs.length;   
    for (var i = 0; i < len; i++)
    {
      if ((attrs[i].name.indexOf("-moz-") === -1)
         &&(attrs[i].name!=="_moz_dirty")
         &&(attrs[i].name!=="msiSelectionManagerID"))
        currentline.s += ' '+attrs[i].name + '="' +attrs[i].value+'"';
    }
  }   
  var child = treeWalker.firstChild();
  if (child)
  {
    currentline.s += ">";
    while (child) 
    {
      processNode(editor, child, treeWalker, output, currentline, indent+1);
      treeWalker.currentNode = child;
      child = treeWalker.nextSibling();
    }
    if (!inline)
      newline(output, currentline, indent);
    currentline.s += "</"+node.nodeName+">";
  }
  else currentline.s +="/>";
  if (!inline) newline(output, currentline, indent);   
}       

/* ATTRIBUTE_NODE = 2, handled in element code
   TEXT_NODE = 3*/

function processText( node, output, currentline)
{
    if ( !isEmptyText(node)) currentline.s += encodeEntities((node.textContent.replace(/\s+/," ","g")));
}                                                        

/* CDATA_SECTION_NODE = 4 */

function processCData( node, output, currentline, indent )
{
  currentline.s += "<![CDATA[";
  currentline.s += node.data;
  currentline.s += "]]>";
}

/* ENTITY_REFERENCE_NODE = 5
   ENTITY_NODE = 6 */
   
/* PROCESSING_INSTRUCTION_NODE = 7 */

function processPINode (node, output, currentline, indent)
{
  newline(output, currentline, 0);
  currentline.s += "<?"+node.target+" "+node.data+"?>";
  newline(output, currentline, indent);
}   

/* COMMENT_NODE = 8*/

function processComment(node, output, currentline, indent )
{
  newline(output, currentline,0);
  currentline.s += "<!--";
  currentline.s += node.data;
  currentline.s += "-->";
  newline(output, currentline, indent);
}

/* DOCUMENT_NODE = 9*/

function processDocument(editor, node, treeWalker, output, currentline, indent )
{
  currentline.s += '<?xml version="1.0" encoding="UTF-8"?>';
  newline(output, currentline, 0);
  var child = treeWalker.firstChild();
  while (child) 
  {
    processNode(editor, child, treeWalker, output, currentline, indent);
    treeWalker.currentNode = child;
    child = treeWalker.nextSibling();
  }
}

/* DOCUMENT_TYPE_NODE = 10*/
function processDocumentType(node, output, currentline, indent)
{  //maybe this should be based on a mode. When building source view, doctype declaration is handled differently
//  newline(output, currentline,0);
//  currentline.s +=
//  '<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1 plus MathML 2.0//EN" "http://www.w3.org/TR/MathML2/dtd/xhtml-math11-f.dtd">'
//  newline(output, currentline, indent);
}


function processNode( editor, node, treeWalker, output, currentline, indent)
{
  switch (node.nodeType) {                                                    
    case 1: //Node.ELEMENT_NODE: 
      processElement(editor, node, treeWalker, output, currentline, indent); 
      break;
    case 3: //Node.TEXT_NODE: 
      processText(node, output, currentline); 
      break;
    case 4:  //Node.CDATA_SECTION_NODE: 
      processCData(node, output, currentline, indent); 
      break;
    case 7: //Node.PROCESSING_INSTRUCTION_NODE: 
      processPINode( node, output, currentline, indent); 
      break;
    case 8: //Node.COMMENT_NODE: 
      processComment( node, output, currentline, indent); 
      break;
    case 9: //Node.DOCUMENT_NODE: 
      processDocument(editor, node, treeWalker, output, currentline, indent); 
      break;
    case 10: //Node.DOCUMENT_NODE: 
      processDocumentType(node, output, currentline, indent); 
      break;
    default:  
      newline(output, currentline, 0); 
      currentline.s += "Stub for node type "+node.nodeType; 
      newline(output, currentline, indent); 
      break;
  }
}


function prettyprint(editor)
{
  var output = new Object();
  output.s = "";
  var currentline = new Object;
  currentline.s = "";          
  var indent = 0;
  if (!editor) {
    var editorElement = msiGetActiveEditorElement();
    var editor;
    if (editorElement)
    {
      editor = msiGetEditor(editorElement);
    }
    if (!editor) return;
  }
  maxLength = GetIntPref("swp.sourceview.maxlinelength");
  if (maxLength === 0) maxLength = maxLengthDefault;
  minLength = GetIntPref("swp.sourceview.minlinelength");
  if (minLength === 0) minLength = minLengthDefault;
  var intInc = GetIntPref("swp.sourceview.indentincrement");
  if (intInc !== 0) intervalIncrement = "          ".substr(0,intInc);
  editor.document.normalize();
  var treeWalker = editor.document.createTreeWalker(editor.document,
        1021,     // everything but fragments and attributes
        { acceptNode: function(node) { return NodeFilter.FILTER_ACCEPT; } },
        false);
  dump("First node is "+treeWalker.root.nodeName+"\n");
  processNode(editor, treeWalker.root, treeWalker, output, currentline, indent);
  return output.s;
} 
  
 
function getSelectionParentByTag( editor, tagname)
{
  var sel = editor.selection;
  var range = sel.getRangeAt(0);
  var ancestor = range.commonAncestorContainer;
  while (ancestor && ancestor.tagName !== tagname) ancestor = ancestor.parentNode;
  if (ancestor && ancestor.tagName === tagname) return ancestor;
  return null;
}

function getEventParentByTag( event, tagname)
{
	var node = event.target;
	while (node && node.tagName !== tagname) node = node.parentNode;
	if (node && node.tagName === tagname) return node;
}



function writeStringAsFile( str, file, mode )
{
  if (!mode)
    mode = -1;
  var fos = Components.classes["@mozilla.org/network/file-output-stream;1"].createInstance(Components.interfaces.nsIFileOutputStream);
  fos.init(file, -1, mode, false);
  var os = Components.classes["@mozilla.org/intl/converter-output-stream;1"]
    .createInstance(Components.interfaces.nsIConverterOutputStream);
  os.init(fos, "UTF-8", 4096, "?".charCodeAt(0));
  os.writeString(str);
  os.close();
  fos.close();
}

function getFileAsString( url )
{
  var req = new XMLHttpRequest();
  req.overrideMimeType("text/plain");
  req.open('GET', url, false);   
  req.send(null);  
  if (req.status === 0)
    return req.responseText;
  else
    return null;
}

function addLanguagesToTagDefs(lang1, lang2)	
{
  var editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);
	var babelTags = editor.tagListManager.getBabelTags();
	var tagArray = babelTags.split(",");
	var i;
	var lang;
	for (i = 0; i < tagArray.length; i++)
	{
		var index = i+1;
		tag = tagArray[i];
		hidden = editor.tagListManager.getStringPropertyForTag( tag, null, "hidden");
		if (index%2 === 1) lang = lang1;
		else lang = lang2;
		if (lang)	
		{
			needsResetting = true;
			editor.tagListManager.setTagVisibility(tag, null, false);
			if (index < 3)
			{
				editor.tagListManager.setTagName(tag, null, "text"+ lang);
			}
			else
			{
				editor.tagListManager.setTagName(tag, null, (lang==="arabic" ? "Arabic" : lang) )
			}
		}
		else
		{
			lang = "##lang" + (1 + index%2).toString();
			editor.tagListManager.setTagVisibility(tag, null, true);
			if (index < 3)
			{
				editor.tagListManager.setTagName(tag, null, "text"+ lang);
			}
			else
			{
				editor.tagListManager.setTagName(tag, null, lang);
			}			
		}
	}
	//if (needsResetting)
	editor.tagListManager.rebuildHash();
	buildAllTagsViewStylesheet(editor);	
}

function addLanguageTagsFromBabelTag(doc)
{
	var babeltags = doc.getElementsByTagName("babel");
	var babeltag;
	if (babeltags && babeltags.length > 0)
	{
		var lang1;
		var lang2;
		babeltag = babeltags[0];
		lang1 = babeltag.getAttribute("lang1");
		lang2 = babeltag.getAttribute("lang2");
		if (lang1 || lang2)
		{
			addLanguagesToTagDefs(lang1, lang2);
		}
	}
}

function buildAllTagsViewStylesheet(editor)
{
	var templatefile = msiFileFromFileURL(msiURIFromString("resource://app/res/css/tagtemplate.css"));
	var data = "";  
	var fstream = Components.classes["@mozilla.org/network/file-input-stream;1"].  
	                        createInstance(Components.interfaces.nsIFileInputStream);  
	var cstream = Components.classes["@mozilla.org/intl/converter-input-stream;1"].  
	                        createInstance(Components.interfaces.nsIConverterInputStream);  
	fstream.init(templatefile, -1, 0, 0);  
	cstream.init(fstream, "UTF-8", 0, 0);  

	var templatestr = {};
  cstream.readString(-1, templatestr); // read the whole file and put it in str.value  
  data = templatestr.value;  
	cstream.close(); // this closes fstream  
	var classtemplates = data.split(/\-{4,}/);
	var j;
	for (j = 0; j < classtemplates.length; j++) classtemplates[j]=classtemplates[j].replace(/^\s*/,"");


	var tagclasses = ["texttag","paratag","listparenttag","listtag","structtag","envtag","frontmtag"];
	var taglist;
	var i;
	var k;
	var str = "";
	var ok;
	var classname;
	var classtemplate;
	for (j = 0; j < tagclasses.length; j++)
	{
	  ok = false;
	  classname= tagclasses[j];
	  for (k = 0; k < classtemplates.length; k++)
	  {
	    if (classtemplates[k].indexOf(classname)===0) 
	    {
	      classtemplate = classtemplates[k];
	      ok = true;
	      break;
	    }
	  }

	  taglist = (editor.tagListManager.getTagsInClass(classname," ", false)).split(" ");
	  for (i = 0; i < taglist.length; i++)
	  {
	    if (taglist[i].length && taglist[i][0] !== "(")
	      str += classtemplate.replace(classname,taglist[i],"g")+"\n";
	  }
	}

	try {
	  var htmlurlstring = editor.document.documentURI;;
	  var htmlurl = msiURIFromString(htmlurlstring);
	     // ... seems ok
	  var htmlFile = msiFileFromFileURL(htmlurl);
	   // Throws exception. htmlurl doesn't have nsIFileURL interface.
	   // Can fix by setting the dialog shell in the prefs to something like
	   // ...   "resource://app/res/StdDialogShell.xhtml"
	   // and moving the file there in the build/install.

	  var cssFile = htmlFile.parent;
 
	  cssFile.append("css");
	  if (!cssFile.exists()) cssFile.create(1, 0755);
   
	  cssFile.append("msi_Tags.css");
		if (cssFile.exists()) cssFile.remove(0);
		cssFile.create(0, 0755);


	  var fos = Components.classes["@mozilla.org/network/file-output-stream;1"].createInstance(Components.interfaces.nsIFileOutputStream);
	  fos.init(cssFile, -1, -1, false);
	  var os = Components.classes["@mozilla.org/intl/converter-output-stream;1"]
	    .createInstance(Components.interfaces.nsIConverterOutputStream);
	  os.init(fos, "UTF-8", 4096, "?".charCodeAt(0));
	  os.writeString(str);
	  os.close();
	  fos.close();
	}
	catch (e) {
	  dump ("Problem creating msi_tags.css. Exception:" + e + "\n");
	}
}

function msiEditorFindJustInsertedElement(tagName, editor)
{
  var currNode = editor.selection.focusNode;
  var currOffset = editor.selection.focusOffset;
  var currName;
  var childList;
  if (msiNavigationUtils.isMathTag(tagName) && currNode && msiNavigationUtils.isMathNode(currNode))
  {
    while (currNode && (msiGetBaseNodeName(currNode) !== tagName))
    {
      if (msiNavigationUtils.isEmptyInputBox(currNode))
        currNode = currNode.parentNode;
      else
      {
        childList = msiNavigationUtils.getSignificantContents(currNode);
        if (childList.length <= 1)
          currNode = currNode.parentNode;
        else if (msiGetBaseNodeName(currNode) === "mtr")
          currNode = currNode.parentNode;
        else
          currNode = null;  //stop looking
      }
    }
    if (currNode)
      return currNode;
    currNode = editor.selection.focusNode;  //otherwise reset it and try the usual approach below
  }

  var lookWhere = ["current", "right", "left"];  //sequence after inserting a normal container
  var nOffsetIncrement = 1;
  var bLookInCurrNode = true;
  switch(tagName)
  {
    case "object":
    case "embed":
    case "msiframe":
      lookWhere = ["left", "right"];
    break;
    case "a":
      lookWhere = ["right","current","left"];
    break;
  }

  var bFound = false;
  for (var ii = 0; !bFound && (ii < lookWhere.length); ++ii)
  {
    switch(lookWhere[ii])
    {
      case "current":
        bFound = (msiGetBaseNodeName(currNode) === tagName);
      break;
      case "left":
        while (!bFound && currNode && (currOffset > 0))
        {
          currNode = currNode.childNodes[currOffset - 1];
          bFound = (msiGetBaseNodeName(currNode) === tagName);
          if (currNode.childNodes)
            currOffset = currNode.childNodes.length;
          else
            currOffset = 0;  //if we're a text or other atomic node, this'll break the search
        }
      break;
      case "right":
        while (!bFound && currNode && currNode.childNodes && (currOffset < currNode.childNodes.length))
        {
          currNode = currNode.childNodes[currOffset];
          bFound = (msiGetBaseNodeName(currNode) === tagName);
          currOffset = 0;
        }
      break;
      default:
      break;
    }
    if (!bFound)  //reset for the next try
    {
      currNode = editor.selection.focusNode;
      currOffset = editor.selection.focusOffset;
    }
  }

  return (bFound ? currNode : null);
}



function offsetOfChild(parent, child)
{
	var offset = 0;
	if (child.parentNode != parent)
	{
		throw ("offsetOfChild: 'parent' must by parent of 'child'");
	}
	var node = parent.firstChild;
	while (node && node != child)
	{
		node = node.nextSibling;
		offset++;
	}	
	return offset;
}

function tryUntilSuccessful(interval, timeout, funct)
// Try function funct every interval milliseconds until it has run timeout times or funct returns true.
{
  var intervalId, count;
  count = 0;
  intervalId = setInterval(function () {
    if (funct()) {
      clearInterval(intervalId);      
    } else if (count >= timeout) {
      clearInterval(intervalId);
    } else {
      count++;
      msidump("A try failed in 'tryUntilSuccessful");
    }
  },interval);
}

function checkPackageDependenciesForEditor(editor)
{
  var aDocument = editor.document;
//  if (!aDocument.documentModified)
//    return;

  var ourExpr = "//*[local-name()='table'][@width]";  //This can be adjusted as needed to allow checking other items
  var xPathEval = new XPathEvaluator();
  var nsResolver = xPathEval.createNSResolver(aDocument.documentElement);
  var resultNodes = xPathEval.evaluate(ourExpr, aDocument.documentElement, nsResolver, XPathResult.ORDERED_NODE_SNAPSHOT_TYPE, null);
  var currNode;
  var package;
  try
  {
    for (var i = 0; i < resultNodes.snapshotLength; ++i)
    {
      currNode = resultNodes.snapshotItem(i);
//      dump("In checkPackageDependenciesForEditor, examining node [" + msiGetBaseNodeName(currNode) + "].\n");
      switch(msiGetBaseNodeName(currNode))
      {
        case "table":
          package = "tabulary";
          var parentTable = msiNavigationUtils.getParentOfType(currNode.parentNode, "table");
          if (parentTable && parentTable.hasAttribute("width"))
            package = null;  //we could use tabularx?? Will use tabular* instead...
          msiEnsureElementAttribute(currNode, "req", package); //don't use the msiEnsureElementAttributeEditor method, so as not to be on the undo stack
        break;
      }
    }
  } catch(ex) {dump("Error in checkPackageDependenciesForEditor: " + ex + ".\n");}
}

