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

  // this doesn't seem to work when initFastCursorBar is called from its present
  // position.  I'll have to try to delay the call.
  try {
	  window._content.addEventListener("keypress", onBrowserKeyPress, false);
	  window._content.addEventListener("mousedown", onBrowserMouseDown, false);
    window._content.addEventListener("keydown", onBrowserKeyDown, false);
    window._content.addEventListener("keyup", onBrowserKeyUp, false);
  } catch(e) {  }
  
  var prefService = Components.classes["@mozilla.org/preferences-service;1"]
                              .getService(Components.interfaces.nsIPrefBranch);

  var pbi = prefService.QueryInterface(Components.interfaces.nsIPrefBranchInternal);

  gFlashFastCursorBar = prefService.getIntPref("fastcursor.flashbar");

}

function Find(pattern)
{
  var editorElement = document.getElementById("content-source");  
  var pat = pattern;
  if (!pat) pat = document.getElementById("fastcursor-field").value;
//  if (!gFindInst) 
    gFindInst = editorElement.webBrowserFind;
  gFindService.searchString = pat;
  
  
  gFindInst.searchString  = pat;
  gFindInst.matchCase     = gFindService.matchCase;
  gFindInst.wrapFind      = false;
  gFindInst.findBackwards = gFindService.findBackwards;
  var result = gFindInst.findNext();
  return result;
}

function uninitFastCursorBar()
{
   var prefService = Components.classes["@mozilla.org/preferences-service;1"]
                               .getService(Components.interfaces.nsIPrefBranch);

   var pbi = prefService.QueryInterface(Components.interfaces.nsIPrefBranchInternal);
   pbi.removeObserver(gTypeAheadFind.useFCPref, gFastCursor);

   getBrowser().removeEventListener("keypress", onBrowserKeyPress, false);
   getBrowser().removeEventListener("keyup", onBrowserKeyUp, false);
   getBrowser().removeEventListener("keydown", onBrowserKeyDown, false);
   getBrowser().removeEventListener("mousedown", onBrowserMouseDown, false);
}

function toggleHighlight(aHighlight)
{
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

  finder.caseSensitive = document.getElementById("fastcursor-case-sensitive").checked;

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

function getSelectionControllerForFastCursorToolbar(ds)
{
  var display = ds.QueryInterface(Components.interfaces.nsIInterfaceRequestor).getInterface(Components.interfaces.nsISelectionDisplay);
  if (!display)
    return null;
  return display.QueryInterface(Components.interfaces.nsISelectionController);
}

function toggleCaseSensitivity(aCaseSensitive)
{
  gFindService.matchCase = aCaseSensitive;
}
  
function changeSelectionColor(aAttention)
{
//  var ds = getBrowser().docShell;
//  var dsEnum = ds.getDocShellEnumerator(Components.interfaces.nsIDocShellTreeItem.typeContent,
//                                        Components.interfaces.nsIDocShell.ENUMERATE_FORWARDS);
//  while (dsEnum.hasMoreElements()) {
//    ds = dsEnum.getNext().QueryInterface(Components.interfaces.nsIDocShell);
//    var controller = getSelectionControllerForFastCursorToolbar(ds);
//    if (!controller)
//      continue;
//    const selCon = Components.interfaces.nsISelectionController;
//    controller.setDisplaySelection(aAttention? selCon.SELECTION_ATTENTION : selCon.SELECTION_ON);
//  }
}

function openFastCursorBar()
{
  if (!gNotFoundStr) {
    var bundle = document.getElementById("bundle_FastCursorBar");
    gNotFoundStr = bundle.getString("NotFound");
  }

  var FastCursorToolbar = document.getElementById("fastCursorPanel");
//  if (FastCursorToolbar.hidden) {
    FastCursorToolbar.hidden = false;
  
    var statusIcon = document.getElementById("fastcursor-status-icon");
    var statusText = document.getElementById("fastcursor-status");
    var FastCursorField = document.getElementById("fastcursor-field");
    FastCursorField.removeAttribute("status");
    statusIcon.removeAttribute("status");
    statusText.value = "";

    return true;
//  }
//  return false;
}

function focusFastCursorBar()
{
  var FastCursorField = document.getElementById("fastcursor-field");
  FastCursorField.focus();    
}

function selectFastCursorBar()
{
  var FastCursorField = document.getElementById("fastcursor-field");
  FastCursorField.select();    
}

function closeFastCursorBar()
{
  var FastCursorField = document.getElementById("fastcursor-field");
  var ww = Components.classes["@mozilla.org/embedcomp/window-watcher;1"]
                     .getService(Components.interfaces.nsIWindowWatcher);
  if (window == ww.activeWindow && document.commandDispatcher.focusedElement &&
      document.commandDispatcher.focusedElement.parentNode.parentNode == FastCursorField) {
    _content.focus();
  }

  var fastCursorToolbar = document.getElementById("fastCursorPanel");
  fastCursorToolbar.hidden = true;
  gTypeAheadFindBuffer = "";
  changeSelectionColor(false);
  if (gQuickFindTimeout) {
    clearTimeout(gQuickFindTimeout);
    gQuickFindTimeout = null;    
  } 
}

function shouldFastFastCursor(evt)
{
  if (evt.ctrlKey || evt.altKey || evt.metaKey || evt.getPreventDefault())
    return false;
    
  var elt = document.commandDispatcher.focusedElement;
  if (elt) {
    var ln = elt.localName.toLowerCase();
    if (ln == "input" || ln == "textarea" || ln == "select" || ln == "button" || ln == "isindex")
      return false;
  }
  
  var win = document.commandDispatcher.focusedWindow;
  if (win && win.document.designMode == "on") 
    return false;
  else
    return true;
}

function onFastCursorBarFocus()
{
  toggleLinkFocus(false);
}

function onFastCursorBarBlur()
{
  toggleLinkFocus(true);
  changeSelectionColor(false);
}

function onBrowserMouseDown(evt)
{
  var fastcursorToolbar = document.getElementById("fastCursorPanel");
  if (!fastcursorToolbar.hidden && gFindMode != FIND_NORMAL)
    closeFastCursorBar();
}

var startSelectionNode;
var startSelectionOffset;

function onBrowserKeyDown(evt)
{
  var keyCode = new Number();
  var isArrow = new Boolean();
  var selection;
  gArrowStateService.findKeyCode(evt, keyCode, isArrow);
  if (!keyCode.value) return 0;
  if (isArrow.value)
  {
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
    //  If there are no more arrows down, hide the FastCursorBar
    gArrowStateService.findArrowKeyState(isArrow, arrowKeyCode, isRepeating,
      isVertical, isForward, keyCode.value, isFirstArrowPress);
    if (!isArrow.value) {
      var fastcursorToolbar = document.getElementById("fastCursorPanel");
      fastcursorToolbar.hidden = true;
    }
  }
  return 0;
}



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
  if (!isArrow.value) {
    if (keyCode.value == KeyEvent.DOM_VK_ESCAPE) {
      document.getElementById("fastCursorPanel").hidden =true;
    }
    gArrowStateService.nonArrowKeyPress(keyCode.value, evt.charCode);
    // if we are deleting, we want to search again from the beginning
    gArrowStateService.findArrowKeyState(isArrow, arrowKeyCode, isRepeating,
      isVertical, isForward, keyCode.value, isFirstArrowPress);
    if (isArrow.value) {
      if (keyCode.value == KeyEvent.DOM_VK_BACK_SPACE)
        window._content.getSelection().collapse(startSelectionNode, startSelectionOffset);
      var findField = document.getElementById("fastcursor-field");
      findField.value = gArrowStateService.findBuffer;
      gFindService.findBackwards = !(isForward.value); 
      selection = window._content.getSelection();     
      if (isVertical.value)  {
        if (findField.value.length > 0) document.getElementById("fastCursorPanel").hidden = false;
        if (findField.value.length == 1) { // this is the first find in this incremental search; save cursor position
          selection = window._content.getSelection();
          startSelectionNode = selection.focusNode;
          startSelectionOffset = selection.focusOffset;
        }
        if (isForward.value){
          selection.collapseToStart();
        }
        else {
          selection.collapseToEnd();
          goDoCommand("cmd_charNext");
        }
        res = Find(findField.value); 
        updateStatus(res);
      } else {
          if (isForward.value) selection.collapseToEnd();
          else selection.collapseToStart();
          res = Find(findField.value);
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
//
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

//} 

function enableFastCursorButtons(aEnable)								
{
  var findNext = document.getElementById("find-next");
  var findPrev = document.getElementById("find-previous");  
  var highlight = document.getElementById("highlight");
  findNext.disabled = findPrev.disabled = highlight.disabled = !aEnable;  
}

function setUpFindInst()
{
  gFindInst.searchString  = document.getElementById("fastcursor-field").value;
  gFindInst.matchCase     = document.getElementById("fastcursor-case-sensitive").checked;
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
  if (highlightBtn.checked)
    setHighlightTimeout();
        
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

function onFindCmd()
{
  gFindMode = FIND_NORMAL;
  openFastCursorBar();
  if (gFlashFastCursorBar) {
    gFlashFastCursorBarTimeout = setInterval(flashFastCursorBar, 500);
    var prefService = Components.classes["@mozilla.org/preferences-service;1"]
                                .getService(Components.interfaces.nsIPrefBranch);

    prefService.setIntPref("fastcursor.flashbar", --gFlashFastCursorBar);
  }
  selectFastCursorBar();
  focusFastCursorBar();
}

function onFindAgainCmd()
{
	var res = Find(null);
  if (!res) {
    if (openFastCursorBar()) {
      focusFastCursorBar();
      selectFastCursorBar();
      updateStatus(res);
    }
  }
}

function onFindPreviousCmd()
{
  gFindService.findBackwards = true;
  var res = Find();
  if (!res) {
    if (openFastCursorBar()) {
      focusFastCursorBar();
      selectFastCursorBar();
      updateStatus(res);
    }
  }
}

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
      statusText.value = gNotFoundStr;
      field.setAttribute("status", "notfound");      
      break;
    case true:
    default:
      statusIcon.removeAttribute("status");      
      statusText.value = "";
      field.removeAttribute("status");
      break;
  }
}

function setFastCursorCloseTimeout()
{
  if (gQuickFindTimeout)
    clearTimeout(gQuickFindTimeout);
  gQuickFindTimeout = setTimeout(function() { if (gFindMode != FIND_NORMAL) closeFastCursorBar(); }, gQuickFindTimeoutLength);
}
