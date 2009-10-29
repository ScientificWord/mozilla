# -*- Mode: HTML -*-
# ***** BEGIN LICENSE BLOCK *****
# Version: MPL 1.1/GPL 2.0/LGPL 2.1
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Original Code is mozilla.org viewsource frontend.
# 
# The Initial Developer of the Original Code is
# Netscape Communications Corporation.
# Portions created by the Initial Developer are Copyright (C) 2003
# the Initial Developer.  All Rights Reserved.
# 
# Contributor(s):
#     Blake Ross     <blake@cs.stanford.edu> (Original Author)
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 2 or later (the "GPL"), or
# the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
# in which case the provisions of the GPL or the LGPL are applicable instead
# of those above. If you wish to allow use of your version of this file only
# under the terms of either the GPL or the LGPL, and not to allow others to
# use your version of this file under the terms of the MPL, indicate your
# decision by deleting the provisions above and replace them with the notice
# and other provisions required by the LGPL or the GPL. If you do not delete
# the provisions above, a recipient may use your version of this file under
# the terms of any one of the MPL, the GPL or the LGPL.
#
#***** END LICENSE BLOCK ***** -->


const FIND_NORMAL = 0;
const FIND_TYPEAHEAD = 1;
const FIND_LINKS = 2;
const FIND_ARROW = 3;

const SEL_COLLAPSED = 0;
const SEL_EXTEND = 1;

// Global find variables
var gFindMode = FIND_NORMAL;
var gQuickFindTimeout = null;
var gQuickFindTimeoutLength = 0;
var gHighlightTimeout = null;
var gUseTypeAheadFind = false;
var gWrappedToTopStr = "";
var gWrappedToBottomStr = "";
var gNotFoundStr = "";
var gTypeAheadFindBuffer = "";
var gIsBack = true;
var gBackProtectBuffer = 3;
var gFlashFastCursorBar = 0;
var gFlashFastCursorBarCount = 6;
var gFlashFastCursorBarTimeout = null;
var gLastHighlightString = "";
var gTypeAheadLinksOnly = false;
var gFindInst = null;
var gFindService = null;
var gArrowStateService = null;
var gSelMode = SEL_COLLAPSED;

// DOMRange used during highlighting
var searchRange;
var startPt;
var endPt;

var gFastCursorPrefs = {
  useFCPref: "fastcursor.enabled",
  flashFCBar: "fastcursor.flashbar",
 
  observe: function(aSubject, aTopic, aPrefName)
  {
    if (aTopic != "nsPref:changed" || (aPrefName != this.useFCPref)&&(aPrefName != this.flashFCBar))
      return;

    var prefService = Components.classes["@mozilla.org/preferences-service;1"]
      .getService(Components.interfaces.nsIPrefBranch);
      
    gUseFastCursor = prefService.getBoolPref("fastcursor.enabled");
  }
};



// Fastcursorbar state
//
// During initialization, event listeners are added to the main editor.
// Various buffers are initialized.
//
// At the beginning of a search, buffers are re-initialized, which wipes
// out the results of the previous search.
//
// When the user presses Escape or clicks the fastcursor close box or starts 
// another search, the fastcursorbar is hidden. (It will show again if the 
// user proceeds with a new search).

function addListeners()
{
  try {
	  var normaleditor = document.getElementById("content-frame");
    if (normaleditor)
    {
      normaleditor.addEventListener("keypress", onBrowserKeyPress, false);
	    normaleditor.addEventListener("mousedown", onBrowserMouseDown, false);
      normaleditor.addEventListener("keydown", onBrowserKeyDown, false);
      normaleditor.addEventListener("keyup", onBrowserKeyUp, false);
    }
	  var sourceeditor = document.getElementById("content-source");
    if (sourceeditor)
    {
      sourceeditor.addEventListener("keypress", onBrowserKeyPress, false);
	    sourceeditor.addEventListener("mousedown", onBrowserMouseDown, false);
      sourceeditor.addEventListener("keydown", onBrowserKeyDown, false);
      sourceeditor.addEventListener("keyup", onBrowserKeyUp, false);
    }
  } catch(e) { dump("Error: "+e.message + "\n"); }
}


function removeListeners()
{
  dump("Removing listeners\n");
  var normaleditor = document.getElementById("content-frame");
  if (normaleditor)
  {
    normaleditor.removeEventListener("keypress", onBrowserKeyPress, false);
    normaleditor.removeEventListener("mousedown", onBrowserMouseDown, false);
    normaleditor.removeEventListener("keydown", onBrowserKeyDown, false);
    normaleditor.removeEventListener("keyup", onBrowserKeyUp, false);
  }
  var sourceeditor = document.getElementById("content-source");
  if (sourceeditor)
  {
    sourceeditor.removeEventListener("keypress", onBrowserKeyPress, false);
    sourceeditor.removeEventListener("mousedown", onBrowserMouseDown, false);
    sourceeditor.removeEventListener("keydown", onBrowserKeyDown, false);
    sourceeditor.removeEventListener("keyup", onBrowserKeyUp, false);
  }
}


function initFastCursorBar()
{
  // Get the xul <editor> element:
  var editorElement = document.getElementById("content-source");  
  try {
    // get the find service, which stores global find state
      gFindService = Components.classes["@mozilla.org/find/find_service;1"]
                           .getService(Components.interfaces.nsIFindService);
  } catch(e) { gFindService = 0; }

  try {
    // get the arrow state service, which stores global arrow state
      gArrowStateService = Components.classes["@mackichan.com/arrowstate/arrowstate_service;1"].createInstance();
      gArrowStateService.QueryInterface(Components.interfaces.msiIArrowStateService);
  } catch(e) { gArrowStateService = 0; }

  addListeners(); 
  // get preferences 
  var prefService = Components.classes["@mozilla.org/preferences-service;1"]
                              .getService(Components.interfaces.nsIPrefBranch);

  var pbi = prefService.QueryInterface(Components.interfaces.nsIPrefBranch2);

  gFlashFastCursorBar = prefService.getIntPref("fastcursor.flashbar");

}

function Find(pattern)
{
  if (!pattern) pattern = document.getElementById("fastcursor-field").value;
  if (!gFindInst){
    var editorElement = msiGetActiveEditorElement();
    gFindInst = editorElement.webBrowserFind;
  }
  gFindService.searchString = pattern;
  
  
  gFindInst.searchString  = pattern;
  gFindInst.matchCase     = gFindService.matchCase;
  gFindInst.wrapFind      = false;
  gFindInst.findBackwards = gFindService.findBackwards;
  var result = gFindInst.findNext();
  return result;
}

// Highlighting section

function toggleHighlight(aHighlight)
{
  dump("toggleHighlight "+aHighlight+"\n");
  var word = document.getElementById("fastcursor-field").value;
  if (aHighlight) {
    highlightDoc('yellow', word);
  } else {
    highlightDoc(null, null);
    gLastHighlightString = null;
  }
}

function highlightDoc(color, word, win)
{
  if (!win)
    win = window._content; 

  for (var i = 0; win.frames && i < win.frames.length; i++) {
    highlightDoc(color, word, win.frames[i]);
  }

  var doc = win.document;
  if (!document)
    return;

  if (!("body" in doc))
    return;

  var body = doc.body;
  
  var count = body.childNodes.length;
  searchRange = doc.createRange();
  startPt = doc.createRange();
  endPt = doc.createRange();

  searchRange.setStart(body, 0);
  searchRange.setEnd(body, count);

  startPt.setStart(body, 0);
  startPt.setEnd(body, 0);
  endPt.setStart(body, count);
  endPt.setEnd(body, count);

  if (!color) {
    // Remove highlighting.  We use the find API again rather than
    // searching for our span elements by id so that we gain access to the
    // anonymous content that nsIFind searches.

    if (!gLastHighlightString)
      return;

    var retRange = null;
    var finder = Components.classes["@mozilla.org/embedcomp/rangefind;1"].createInstance()
                         .QueryInterface(Components.interfaces.nsIFind);

    while ((retRange = finder.Find(gLastHighlightString, searchRange,startPt, endPt))) {
      var startContainer = retRange.startContainer;
      var elem = null;
      try {
        elem = startContainer.parentNode;
      }
      catch (e) { }

      if (elem && elem.getAttribute("id") == "__firefox-findbar-search-id") {
        var child = null;
        var docfrag = doc.createDocumentFragment();
        var next = elem.nextSibling;
        var parent = elem.parentNode;

        while ((child = elem.firstChild)) {
          docfrag.appendChild(child);
        }

        startPt = doc.createRange();
        startPt.setStartAfter(elem);

        parent.removeChild(elem);
        parent.insertBefore(docfrag, next);
      }
      else {
        // Somehow we didn't highlight this instance; just skip it.
        startPt = doc.createRange();
        startPt.setStart(retRange.endContainer, retRange.endOffset);
      }

      startPt.collapse(true);
    }
    return;
  }

  var baseNode = doc.createElement("span");
  baseNode.setAttribute("style", "background-color: " + color + ";");
  baseNode.setAttribute("id", "__firefox-findbar-search-id");

  highlightText(word, baseNode);
}

function highlightText(word, baseNode)
{
  var retRange = null;
  var finder = Components.classes["@mozilla.org/embedcomp/rangefind;1"].createInstance()
                         .QueryInterface(Components.interfaces.nsIFind);

  finder.caseSensitive = false;

  while((retRange = finder.Find(word, searchRange, startPt, endPt))) {
    // Highlight
    var nodeSurround = baseNode.cloneNode(true);
    var node = highlight(retRange, nodeSurround);
    startPt = node.ownerDocument.createRange();
    startPt.setStart(node, node.childNodes.length);
    startPt.setEnd(node, node.childNodes.length);
  }
  
  gLastHighlightString = word;
}

function highlight(range, node)
{
  var startContainer = range.startContainer;
  var startOffset = range.startOffset;
  var endOffset = range.endOffset;
  var docfrag = range.extractContents();
  var before = startContainer.splitText(startOffset);
  var parent = before.parentNode;
  node.appendChild(docfrag);
  parent.insertBefore(node, before);
  return node;
}

// End of highlighting section

function getSelectionControllerForFastCursorToolbar(ds)
{
  var display = ds.QueryInterface(Components.interfaces.nsIInterfaceRequestor).getInterface(Components.interfaces.nsISelectionDisplay);
  if (!display)
    return null;
  return display.QueryInterface(Components.interfaces.nsISelectionController);
}

function changeSelectionColor(aAttention)  // see if we actually need this.
{
  var ds = getBrowser().docShell;
  var dsEnum = ds.getDocShellEnumerator(Components.interfaces.nsIDocShellTreeItem.typeContent,
                                        Components.interfaces.nsIDocShell.ENUMERATE_FORWARDS);
  while (dsEnum.hasMoreElements()) {
    ds = dsEnum.getNext().QueryInterface(Components.interfaces.nsIDocShell);
    var controller = getSelectionControllerForFastCursorToolbar(ds);
    if (!controller)
      continue;
    const selCon = Components.interfaces.nsISelectionController;
    controller.setDisplaySelection(aAttention? selCon.SELECTION_ATTENTION : selCon.SELECTION_ON);
  }
}

function openFastCursorBar(isUp)  
{
  if (!gNotFoundStr) {
    var bundle = document.getElementById("bundle_FastCursorBar");
    gNotFoundStr = bundle.getString("NotFound");
  }

  var FastCursorToolbar = document.getElementById("fastCursorPanel");
  FastCursorToolbar.hidden = false;
  var statusIcon = document.getElementById("fastcursor-status-icon");
  var statusText = document.getElementById("fastcursor-status");
  var FastCursorField = document.getElementById("fastcursor-field");
  FastCursorField.value="";
  gArrowStateService.findBuffer ="";
  FastCursorField.removeAttribute("status");
  statusIcon.removeAttribute("status");
  statusText.value = "";
  var diricon = document.getElementById("fastcursor-dir");
  if (isUp) diricon.setAttribute("up","true");
  else diricon.removeAttribute("up");
  diricon.setAttribute("hidden", "false");

  return true;
}

//function focusFastCursorBar() -- not called
//{
////  var FastCursorField = document.getElementById("fastcursor-field");
////  FastCursorField.focus();    
//}
//
//function selectFastCursorBar() -- not called
//{
//  var FastCursorField = document.getElementById("fastcursor-field");
//  FastCursorField.select();    
//}
//
function hideFastCursorBar()
{
//  var FastCursorField = document.getElementById("fastcursor-field");
//  var ww = Components.classes["@mozilla.org/embedcomp/window-watcher;1"]
//                     .getService(Components.interfaces.nsIWindowWatcher);
//  if (window == ww.activeWindow && document.commandDispatcher.focusedElement &&
//      document.commandDispatcher.focusedElement.parentNode.parentNode == FastCursorField) {
//    _content.focus();
//  }
//
  dump("hideFastCursorBar\n");
  var FastCursorToolbar = document.getElementById("fastCursorPanel");
  FastCursorToolbar.hidden = true;
  document.getElementById("fastcursor-field").value="";
  gArrowStateService.findBuffer ="";
////  removeListeners();
//  gTypeAheadFindBuffer = "";
//  changeSelectionColor(false);
//  if (gQuickFindTimeout) {
//    clearTimeout(gQuickFindTimeout);
//    gQuickFindTimeout = null;    
//  } 
}

//function shouldFastFastCursor(evt)
//{
//  if (evt.ctrlKey || evt.altKey || evt.metaKey || evt.getPreventDefault())
//    return false;
//    
//  var elt = document.commandDispatcher.focusedElement;
//  if (elt) {
//    var ln = elt.localName.toLowerCase();
//    if (ln == "input" || ln == "textarea" || ln == "select" || ln == "button" || ln == "isindex")
//      return false;
//  }
//  
//  var win = document.commandDispatcher.focusedWindow;
//  if (win && win.document.designMode == "on") 
//    return false;
//  else
//    return true;
//}

//function onFastCursorBarFocus() -- not called
//{
////  toggleLinkFocus(false);
//}

//function onFastCursorBarBlur() -- not called
//{
////  toggleLinkFocus(true);
// // changeSelectionColor(false);
//}


function dumpglobals()
{
  var selection;
  selection = window._content.getSelection();
  dump("startFocusNode="+startFocusNode+"\n");
  dump("startFocusOffset="+startFocusOffset+"\n");
  dump("startAnchorNode="+startAnchorNode+"\n");
  dump("startAnchorOffset="+startAnchorOffset+"\n");
  dump("searchStartNode="+searchStartNode+"\n");
  dump("searchStartOffset="+searchStartOffset+"\n");
  dump("selection.anchorOffset="+selection.anchorOffset+", selection.focusOffset="+selection.focusOffset+"\n");
}



function onBrowserMouseDown(evt)
{
  var fastcursorToolbar = document.getElementById("fastCursorPanel");
  if (!fastcursorToolbar.hidden && gFindMode != FIND_NORMAL)
    hideFastCursorBar();
}

var startFocusNode;
var startFocusOffset;
var startAnchorNode;
var startAnchorOffset;
var searchStartNode;
var searchStartOffset;
//var searchEndNode;
//var searchEndOffset;

function recordFindResults(isVertical, isForward)
{
//  dump("Entering FindResults\n");
//  dumpglobals();
  var selection;
  var selEndNode;
  var selEndOffset;
  var selStartNode;
  var selStartOffset;
  selection = window._content.getSelection();
  if (isVertical.value) // incremental find, not character find
  {
    selEndNode = selection.focusNode;
    selEndOffset = selection.focusOffset;
    if (isForward.value) {
      searchStartNode = selection.anchorNode;
      searchStartOffset = selection.anchorOffset;
    } else {
      selEndNode = selection.anchorNode;
      selEndOffset = selection.anchorOffset;
      selection.collapseToEnd();
      goDoCommand("cmd_charNext");
      searchStartNode = selection.anchorNode;
      searchStartOffset = selection.anchorOffset;
    }
    if (gSelMode == SEL_COLLAPSED)
    {
      if (!isForward.value)
      {
        selection.collapse(selStartNode, selStartOffset);
        selection.extend(selEndNode, selEndOffset);
      } // otherwise the selection is fine as it is.
    }
    else
    {
      selection.collapse(startAnchorNode, startAnchorOffset)
      selection.extend(selEndNode, selEndOffset);
    }
  }
  else
  {
    if (gSelMode == SEL_COLLAPSED) {
  //    if (isForward.value){
  //      selection.collapseToStart();
  //    }
  //    else {
  //      selection.collapseToEnd();
  //      goDoCommand("cmd_charNext");
  //    }
      if (isForward.value) {
        searchStartNode = selection.focusNode;
        searchStartOffset = selection.focusOffset;
      } else {
        searchStartNode = selection.anchorNode;
        searchStartOffset = selection.anchorOffset;
      }
    } else //SEL_EXTEND
    {
      if (isForward.value){
        searchStartNode = selection.focusNode;
        searchStartOffset = selection.focusOffset;
        selEndNode = selection.focusNode;
        selEndOffset = selection.focusOffset;
        selection.collapse(startAnchorNode, startAnchorOffset);
        selection.extend(selEndNode, selEndOffset);
      }
      else {
        selStartNode = selection.anchorNode;
        selStartOffset = selection.anchorOffset;
        dump("selection.anchorOffset="+selStartOffset+", selection.focusOffset="+selection.focusOffset+"\n");
        searchStartNode = selection.anchorNode;
        searchStartOffset = selection.anchorOffset;
        selection.collapse(startAnchorNode, startAnchorOffset);
        selection.extend(selStartNode, selStartOffset);
      }
    }
  }  
//  dump("Exiting FindResults\n");
//  dumpglobals();
}      


function onBrowserKeyDown(evt)
{
  var keyCode = new Number();
  var isArrow = new Boolean();
  var wasArrow = new Boolean();
  var arrowKeyCode = new Number();
  var selection;
  var isRepeating = new Boolean();
  var isVertical = new Boolean();
  var isForward = new Boolean();
  var isFirstArrowPress = new Boolean();
  var selection;
  gArrowStateService.findKeyCode(evt, keyCode, isArrow);
  if (!keyCode.value) return 0;
  if (isArrow.value)
  {
    hideFastCursorBar();
    gArrowStateService.findArrowKeyState(wasArrow, arrowKeyCode, isRepeating,
      isVertical, isForward, keyCode.value, isFirstArrowPress);
    if (!wasArrow.value) // if an arrow is being pressed so that it changes the state, check the shift key
      // to determine if we are setting or extending the selection.
    {
      fFindInitialized = false;
      if (evt.shiftKey) gSelMode = SEL_EXTEND
      else gSelMode = SEL_COLLAPSED;
      selection = window._content.getSelection();
      searchStartNode = selection.focusNode;
      searchStartOffset = selection.focusOffset;
      startAnchorNode = selection.anchorNode;
      startAnchorOffset = selection.anchorOffset;
      var direction = document.getElementById("fastcursor-dir");
      if (isVertical && !isForward) 
        direction.setAttribute("up", "true");
      else direction.removeAttribute("up");
      
      
    }
    gArrowStateService.arrowKeyDown( keyCode.value );
  }
  return 0;
}

function onBrowserKeyUp(evt)
{
  var keyCode = new Number();
  var arrowKeyCode = new Number();
  var isArrow = new Boolean();
  var isRepeating = new Boolean();
  var isVertical = new Boolean();
  var isForward = new Boolean();
  var isFirstArrowPress = new Boolean();
  gArrowStateService.findKeyCode(evt, keyCode, isArrow);
  if (!keyCode.value) return 0;
  if (isArrow.value) {
    gArrowStateService.arrowKeyUp( keyCode.value );
    gArrowStateService.findArrowKeyState(isArrow, arrowKeyCode, isRepeating,
      isVertical, isForward, keyCode.value, isFirstArrowPress);
    if (!isArrow.value) {
      var fastcursorDir = document.getElementById("fastcursor-dir");
      fastcursorDir.setAttribute("hidden", "true");
    }
  }
  return 0;
}

function prepareForFind(selection, isForward)
{
//  dump("Entering prepareForFind\n");
//  dumpglobals();
  selection.collapse(searchStartNode, searchStartOffset);
//  dump("Exiting prepareForFind\n");
//  dumpglobals();
}


var fFindInitialized = false;

function onBrowserKeyPress(evt)
{  
//  // Check focused elt
//  if (!shouldFastFastCursor(evt))
//    return;
  var keyCode = new Number(0);
  var arrowKeyCode = new Number();
  var isArrow = new Boolean();
  var isRepeating = new Boolean;
  var isVertical = new Boolean();
  var isForward = new Boolean();
  var isFirstArrowPress = new Boolean();
  var res;
  var selection;
  gArrowStateService.findKeyCode(evt, keyCode, isArrow);
  if (!isArrow.value)
  {
    if (keyCode.value == KeyEvent.DOM_VK_ESCAPE) {
      hideFastCursorBar();
    }
    gArrowStateService.nonArrowKeyPress(keyCode.value, evt.charCode);
    // if we are deleting, we want to search again from the beginning
    gArrowStateService.findArrowKeyState(isArrow, arrowKeyCode, isRepeating,
      isVertical, isForward, keyCode.value, isFirstArrowPress);
    if (isArrow.value) {
      if (keyCode.value == KeyEvent.DOM_VK_BACK_SPACE)
      {
        searchStartNode = startFocusNode;
        searchStartOffset = startFocusOffset;
      }
      var findField = document.getElementById("fastcursor-field");
      findField.value = gArrowStateService.findBuffer;
      gFindService.findBackwards = !(isForward.value); 
      selection = window._content.getSelection();     
      if (isVertical.value)  {
        if (findField.value.length > 0){
          var diricon = document.getElementById("fastcursor-dir");
          if (!isForward.value) diricon.setAttribute("up","true");
          else diricon.removeAttribute("up");
          diricon.setAttribute("hidden", "false");
          document.getElementById("fastCursorPanel").hidden = false;
        }
        if (!fFindInitialized && (findField.value.length == 1)) { // this is the first find in this incremental search; save cursor position
          selection = window._content.getSelection();
          searchStartNode = selection.focusNode;
          searchStartOffset = selection.focusOffset;
          startFocusNode = selection.focusNode;
          startFocusOffset = selection.focusOffset;
          startAnchorNode = selection.anchorNode;
          startAnchorOffset = selection.anchorOffset;
          fFindInitialized = true;
        }
        prepareForFind(selection, isForward);
        res = Find(findField.value);
        recordFindResults(isVertical, isForward);
        updateStatus(res);
      } else 
      {
        prepareForFind(selection, isForward);
        res = Find(findField.value);
        recordFindResults(isVertical, isForward);
      }
    }
  }
  return 0;
}
  
        
      
  
//  var findField = document.getElementById("fastcursor-field");
//  if (gFindMode != FIND_NORMAL && gQuickFindTimeout) {    
//    if (evt.keyCode == 8) { // Backspace
//      if (findField.value) {
//        findField.value = findField.value.substr(0, findField.value.length - 1);
//        gIsBack = true;   
//        gBackProtectBuffer = 3;
//      }
//      else if (gBackProtectBuffer > 0) {
//        gBackProtectBuffer--;
//      }
//      
//      if (gIsBack || gBackProtectBuffer > 0)
//        evt.preventDefault();
//        
//      find(findField.value);
//    }
//    else if (evt.keyCode == 27) { // Escape
//      closeFastCursorBar();
//      evt.preventDefault();
//    }
//    else if (evt.charCode) {
//      if (evt.charCode == 32) // Space
//        evt.preventDefault();
//        
//      findField.value += String.fromCharCode(evt.charCode);
//      find(findField.value);
//    }
//    return;
//  }
//  
//  if (evt.charCode == 39 /* ' */ || evt.charCode == 47 /* / */ || (gUseTypeAheadFind && evt.charCode && evt.charCode != 32)) {   
//    gFindMode = (evt.charCode == 39 || (gTypeAheadLinksOnly && evt.charCode != 47)) ? FIND_LINKS : FIND_TYPEAHEAD;
//    toggleLinkFocus(true);
//    if (openFastCursorBar()) {      
//      setFastCursorCloseTimeout();      
//      if (gUseTypeAheadFind && evt.charCode != 39 && evt.charCode != 47) {
//        gTypeAheadFindBuffer += String.fromCharCode(evt.charCode);        
//        findField.value = gTypeAheadFindBuffer;
//        find(findField.value);
//      }
//      else {
//        findField.value = "";
//      }
//    }
//    else {
//      if (gFindMode == FIND_NORMAL) {
//        selectFastCursorBar();      
//        focusFastCursorBar();
//      }
//      else {
//        findField.value = String.fromCharCode(evt.charCode);
//        find(findField.value);
//      }
//    }        
//  }
//}
//
//function toggleLinkFocus(aFocusLinks)
//{
//}

//function onFastCursorBarKeyPress(evt)
//{
//  if (evt.keyCode == KeyEvent.DOM_VK_RETURN) {
//    var findString = document.getElementById("fastcursor-field");
//    if (!findString.value)
//      return;
//      
//    if (evt.ctrlKey) {
//      document.getElementById("highlight").click();
//      return;
//    }
//    
//    if (evt.shiftKey)
//      findPrevious();
//    else
//      findNext();
//  }
//  else if (evt.keyCode == KeyEvent.DOM_VK_ESCAPE) {
//    closeFastCursorBar();
//    evt.preventDefault();
//  } 
//  else if (evt.keyCode == KeyEvent.DOM_VK_PAGE_UP) {
//    window.top._content.scrollByPages(-1);
//    evt.preventDefault();
//  }
//  else if (evt.keyCode == KeyEvent.DOM_VK_PAGE_DOWN) {
//    window.top._content.scrollByPages(1);
//    evt.preventDefault();
//  }
//  else if (evt.keyCode == KeyEvent.DOM_VK_UP) {
//    window.top._content.scrollByLines(-1);
//    evt.preventDefault();
//  }
//  else if (evt.keyCode == KeyEvent.DOM_VK_DOWN) {
//    window.top._content.scrollByLines(1);
//    evt.preventDefault();
//  }
//
//} 

function enableFastCursorButtons(aEnable)	
// this is not used (yet) but I'll keep it for while.							
{
  var findNext = document.getElementById("find-next");
  var findPrev = document.getElementById("find-previous");  
  var highlight = document.getElementById("highlight");
  findNext.disabled = findPrev.disabled = highlight.disabled = !aEnable;  
}

function setUpFindInst()
{
  gFindInst.searchString  = document.getElementById("fastcursor-field").value;
  gFindInst.matchCase     = false;
  gFindInst.wrapFind      = PR_FALSE;
  gFindInst.findBackwards = PR_FALSE;	   // change this later
}

function onFindNext()
{
  // Transfer dialog contents to the find service.
//  saveFindData();
  // set up the find instance
  setUpFindInst();

  // Search.
  var result = gFindInst.findNext();

  if (!result)
  {
//    var bundle = document.getElementById("findBundle");
//    AlertWithTitle(null, bundle.getString("notFoundWarning"));
//    SetTextboxFocus(gReplaceDialog.findInput);
//    gReplaceDialog.findInput.select();
//    gReplaceDialog.findInput.focus();
    return false;
  } else
  return true;
}

function find(val)
{
  if (!val)
    val = document.getElementById("fastcursor-field").value;
    
  enableFindButtons(val);
 
  var highlightBtn = document.getElementById("highlight");
//  if (highlightBtn.checked)
//    setHighlightTimeout();
        
  changeSelectionColor(true);
  var res = Find(val);
  updateStatus(res);
 }

function flashFastCursorBar()
{
  var fastCursorToolbar = document.getElementById("fastCursorPanel");
  if (gFlashFastCursorBarCount-- == 0) {
    clearInterval(gFlashFastCursorBarTimeout);
    fastCursorToolbar.removeAttribute("flash");
    gFlashFastCursorBarCount = 6;
    return true;
  }
  fastCursorToolbar.setAttribute("flash", (gFlashFastCursorBarCount % 2 == 0) ? "false" : "true");
  return true;
}

//function onFindCmd()
//{
//  gFindMode = FIND_NORMAL;
//  openFastCursorBar();
//  gFlashFastCursorBar = 10;
//  if (gFlashFastCursorBar) {
//    gFlashFastCursorBarTimeout = setInterval(flashFastCursorBar, 500);
//    var prefService = Components.classes["@mozilla.org/preferences-service;1"]
//                                .getService(Components.interfaces.nsIPrefBranch);
//
//    prefService.setIntPref("fastcursor.flashbar", --gFlashFastCursorBar);
//  }
//  selectFastCursorBar();
//  focusFastCursorBar();
//}
//
//function onFindAgainCmd()
//{
//	var res = Find(null);
//  if (!res) {
//    if (openFastCursorBar()) {
//      focusFastCursorBar();
//      selectFastCursorBar();
//      updateStatus(res);
//    }
//  }
//}
//
//function onFindPreviousCmd()
//{
//  gFindService.findBackwards = true;
//  var res = Find();
//  if (!res) {
//    if (openFastCursorBar()) {
//      focusFastCursorBar();
//      selectFastCursorBar();
//      updateStatus(res);
//    }
//  }
//}

function setHighlightTimeout()
{
  if (gHighlightTimeout)
    clearTimeout(gHighlightTimeout);
  gHighlightTimeout = setTimeout(function() { toggleHighlight(false); toggleHighlight(true); }, 500);  
}

function isFastCursorBarVisible()
{
  var fastCursorBar = document.getElementById("fastCursorPanel");
  return !fastCursorBar.hidden;
}

function findNext()
{
  changeSelectionColor(true);
  gFindService.findBackwards = false;
  var res = Find();  
  updateStatus(res);
  
  return res;
}

function findPrevious()
{
  changeSelectionColor(true);
  gFindService.findBackwards = true;
  var res = Find();  
  updateStatus(res);
  return res;
}

function updateStatus(res)
{
  var fastCursorBar = document.getElementById("fastCursorPanel");
  var field = document.getElementById("fastcursor-field");
  var statusIcon = document.getElementById("fastcursor-status-icon");
  var statusText = document.getElementById("fastcursor-status");
  switch(res) {
    case false:
      statusIcon.setAttribute("status", "notfound");
      statusIcon.setAttribute("hidden", "false");
      statusText.value = "Not found!";
      field.setAttribute("status", "notfound");      
      break;
    case true:
    default:
      statusIcon.removeAttribute("status");      
      statusIcon.setAttribute("hidden", "true");
      statusText.value = "";
      field.removeAttribute("status");
      break;
  }
}

//function setFastCursorCloseTimeout()
//{
//  if (gQuickFindTimeout)
//    clearTimeout(gQuickFindTimeout);
//  gQuickFindTimeout = setTimeout(function() { if (gFindMode != FIND_NORMAL) closeFastCursorBar(); }, gQuickFindTimeoutLength);
//}
