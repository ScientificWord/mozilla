 // Copyright (c) 2006 MacKichan Software, Inc.  All Rights Reserved.
"use strict";
Components.utils.import("resource://app/modules/pathutils.jsm");
Components.utils.import("resource://app/modules/os.jsm");
Components.utils.import("resource://app/modules/unitHandler.jsm");
Components.utils.import("resource://app/modules/graphicsConverter.jsm");

#include productname.inc
const msiEditorJS_duplicateTest = "Bad";
var dynAllTagsStyleSheet;

function aColorObj(editorElement)
{
  this.mEditorElement = editorElement;
  this.LastTextColor = "";
  this.LastBackgroundColor = "";
  this.LastHighlightColor = "";
  this.Type = "";
  this.SelectedType = "";
  this.NoDefault = false;
  this.Cancel  = false;
  this.HighlightColor = "";
  this.BackgroundColor = "";
  this.PageColor = "";
  this.TextColor = "";
  this.TableColor = "";
  this.CellColor = "";
};


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
  if (!("mEditorToolbarPrefListener" in editorElement))
    return;
  try {
    var pbi = GetPrefs().QueryInterface(Components.interfaces.nsIPrefBranch2);
    pbi.removeObserver(kEditorToolbarPrefs, editorElement.mEditorToolbarPrefListener);
  } catch(ex) {
    dump("Failed to remove pref observer: " + ex + "\n");
  }
}

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

function initMetaData( doc ) {
  var node = doc.getElementById("sw-meta");
  if (node) return; // if the node exists, it has already been initialized.
  try {
    var headnode = doc.getElementsByTagName("head")[0];
  }
  catch(e) {}
  if (!node) {
    node = doc.createElement("sw-meta");
    node.setAttribute("id", "sw-meta");
    // put in the head
    headnode.insertBefore(node, headnode.firstChild);
  }
  // Initial data are version, created, lastrevised
  var product = "Scientific WorkPlace";
#ifdef PROD_SWP
  product = "Scientific WorkPlace";
#endif
#ifdef PROD_SW
  product = "Scientific Word";
#endif
#ifdef PROD_SNB
  product = "Scientific Notebook";
#endif
  node.setAttribute("product", product);
  node.setAttribute("version", navigator.productSub);
  var dt = new Date();
  var datetime = dt.toString();
  node.setAttribute("created", datetime);
  node.setAttribute("lastrevised", datetime);
}

function onsaveMetaData( doc ) {
  var node = doc.getElementById("sw-meta");
  if (!node) {  // meta tag doesn't exist; create one, but creation time will be wrong. C'est la vie.
    return initMetaData(doc);
  }
  var dt = new Date();
  var datetime = dt.toString();
  node.setAttribute("lastrevised", datetime);
}

function updateMetaData( doc, updateObject ) {
  var prop;
  var node;
  var node = doc.getElementById("sw-meta");
  if (!node) {  // meta tag doesn't exist; create one, but creation time will be wrong. C'est la vie.
    initMetaData(doc);
  }
  node = doc.getElementById("sw-meta");
  if (!node) { return; } //wtf?
  for (prop in updateObject) {
    node.setAttribute(prop, udateObject[prop]); // rely on caller to make updateObject contain only strings
  }
}


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
    var editorIndex = this.findEditorInfo(anEditorElement, true);
    if (editorIndex >= 0)
    {
      this.mInfoList[editorIndex].bDone = true;
    }
    anEditorElement.mEditorSeqInitializer = null;
    ++editorIndex;
    dump( "In msiEditorArrayInitializer.finishedEditor for editor [" + anEditorElement.id + "]; moving on to editor number" + editorIndex + "].\n" );
    this.initializeNextEditor(editorIndex);
  };
  this.findEditorInfo = function(anEditorElement, bReport)
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
    if (bReport)
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


function msiInitializeEditorForElement(editorElement, initialText, bWithContainingHTML, topwindow)
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
  EditorStartupForEditorElement(editorElement, topwindow);
  msiDumpWithID("In msiInitializeEditorForElement for element [@], back from EditorStartupForEditorElement call.\n", editorElement);

  try {
    var commandTable = msiGetComposerCommandTable(editorElement);
    commandTable.registerCommand("cmd_find",        msiFindCommand);
    commandTable.registerCommand("cmd_findNext",    msiFindAgainCommand);
    commandTable.registerCommand("cmd_findPrev",    msiFindAgainCommand);

    msiSetupMSIMathMenuCommands(editorElement);
#ifndef PROD_SW
    if (msiSetupMSIComputeMenuCommands) msiSetupMSIComputeMenuCommands(editorElement);
#endif
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


function addKeyDownEventListenerForEditor(editorElement)
{
  try
  {
    editorElement.contentWindow.addEventListener("keydown", msiEditorKeyListener, true);
  }
  catch(ex) {dump("Unable to register keydown event listener; error [" + ex + "].\n");}
}

function addObjectResizeListenerForEditor(editorElement)
{
  try
  {
    var editor = msiGetEditor(editorElement);
    editor instanceof Components.interfaces.nsIHTMLObjectResizer;
    editor.addObjectResizeEventListener(new msiResizeListenerForEditor(editor));
  }
  catch(ex) {dump("Unable to register object resize event listener; error [" + ex + "].\n");}
}

function addFocusEventListenerForEditor(editorElement)
{
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

function msiEditorKeyListener(event)
{
  if (event.ctrlKey || event.altKey || event.metaKey)
    return;

  switch(event.keyCode)
  {
    case event.DOM_VK_ENTER:
    case event.DOM_VK_RETURN:
      if (msiEditorCheckEnter(event))
      {
        event.preventDefault();
        event.stopPropagation();
      }
    break;
    case event.DOM_VK_TAB:
      if (msiEditorDoTab(event))
      {
        event.preventDefault();
        event.stopPropagation();
      }
    break;
//    case event.DOM_VK_ESCAPE:
//      if (msiEditorCheckEscape(event))
//      {
//        event.preventDefault();
//        event.stopPropagation();
//      }
//    break;
    default:
    break;
  }
}


var msiResizeListener =
{
  onStartResizing : function(aElement) {},

  onEndResizing : function(anElement, oldWidth, oldHeight, newWidth, newHeight)
  {
    switch(msiGetBaseNodeName(anElement))
    {
      case "object":
      case "embed":
        this.resizeGraphic(anElement, oldWidth, oldHeight, newWidth, newHeight);
      break;
      // case "plotwrapper":
      //   this.resizePlot(anElement, oldWidth, oldHeight, newWidth, newHeight);
      //   break;
      case "msiframe":
        this.resizeFrame(anElement, oldWidth, oldHeight, newWidth, newHeight);
        break;
      case "table":
        this.resizeFrame(anElement, oldWidth, oldHeight, newWidth, newHeight);
        break;
      default:
        msidump("Got resizing request for " + anElement.tagName);
        break;
    }
  },

  resizeGraphic : function(anElement, oldWidth, oldHeight, newWidth, newHeight)
  {
    var theUnits = anElement.getAttribute("units");
    // var style = "";
    var elemWidth = anElement.getAttribute("imageWidth");
    var elemHeight = anElement.getAttribute("imageHeight");
    var bSetWidth = elemWidth && (Number(elemWidth) != 0);
    var bSetHeight = elemHeight && (Number(elemHeight) != 0);
    if (bSetWidth && !bSetHeight)  //only set height if no longer preserves aspect ratio
    {
      var ratio = Number(anElement.getAttribute("naturalHeight"))/Number(anElement.getAttribute("naturalWidth"));
      if (!ratio || (ratio == Number.POSITIVE_INFINITY) || (ratio == Number.NEGATIVE_INFINITY) || (ratio == Number.NaN))
        bSetHeight = true;
      else
        bSetHeight = (Math.abs(ratio * newWidth - newHeight) > 2.0);
    }
    else if (!bSetWidth)  //Note that this includes case where neither attribute was set - if the ratio is okay we'll set only the width
    {
      var ratio = Number(anElement.getAttribute("naturalWidth"))/Number(anElement.getAttribute("naturalHeight"));
      if (!ratio || (ratio == Number.POSITIVE_INFINITY) || (ratio == Number.NEGATIVE_INFINITY) || (ratio == Number.NaN))
        bSetWidth = true;
      else
        bSetWidth = (Math.abs(ratio * newHeight - newWidth) > 2.0);
    }
    if (bSetWidth) {
      msiEditorEnsureElementAttribute(anElement, "imageWidth", msiCSSUnitsList.convertUnits(newWidth, "pt", theUnits), this.mEditor);
      // style += "width: "+msiCSSUnitsList.convertUnits(newWidth, "px", theUnits) + "; ";
    }
    if (bSetHeight) {
      msiEditorEnsureElementAttribute(anElement, "imageHeight", msiCSSUnitsList.convertUnits(newHeight, "pt", theUnits), this.mEditor);
      // style += "height: "+msiCSSUnitsList.convertUnits(newHeight, "px", theUnits) + "; ";
    }
    // anElement.setAttribute('style', style);
    msiSetGraphicFrameAttrsFromGraphic(anElement, null);
    // recompute the cached bitmap if doing so will improve things; i.e., if the src is a vector graphic.
    var copiedSrcUrl = anElement.getAttribute('copiedSrcUrl');
    if (copiedSrcUrl) {
      var ext = /\....$/.exec(copiedSrcUrl) [0];
      if (ext == '.eps' || ext == '.pdf' || ext == '.ps') { // recompute bit map image
        var editorElement = msiGetActiveEditorElement();
        var docUrlString = msiGetEditorURL(editorElement);
        var url = msiURIFromString(docUrlString);
        var baseDir = msiFileFromFileURL(url);
        baseDir = baseDir.parent; // and now it points to the working directory
        graphicsConverter.init(window, baseDir);
        var decomposedRelativePath = copiedSrcUrl.split('/');
        var graphicsDir = baseDir.clone(false);
        while (decomposedRelativePath[0] && decomposedRelativePath[0].length > 0) {
          graphicsDir.append(decomposedRelativePath.shift());
        }
        
        graphicsConverter.copyAndConvert(graphicsDir, false, msiCSSUnitsList.convertUnits(newWidth, "px", theUnits),
          msiCSSUnitsList.convertUnits(newHeight, "px", theUnits) );

      }
    }

  },

  resizePlot : function(anElement, oldWidth, oldHeight, newWidth, newHeight)
  {
    // anElement is a plotwrapper. Parent is an msiframe; grandparent is graph;
    // dimensions are given in pixels.
    if (oldWidth === newWidth && oldHeight === newHeight) {
      return;
    }
    var DOMGraph = anElement.parentNode.parentNode;
    if (DOMGraph.nodeName !== "graph") {
      return;
    }
    try {
      var unithandler = new UnitHandler();
      var units;
// skip preserving aspect ratio for now.
      var graph = new Graph();
      var editorElement = msiGetActiveEditorElement();
      graph.extractGraphAttributes(DOMGraph);
      units = graph.getGraphAttribute("Units");
      unithandler.initCurrentUnit(units);
      var newWidthInUnits = unithandler.getValueOf(newWidth, "px");
      var newHeightInUnits = unithandler.getValueOf(newHeight, "px");
      graph.setGraphAttribute("Width", String(newWidthInUnits));
      graph.setGraphAttribute("Height", String(newHeightInUnits));
      graph.reviseGraphDOMElement(DOMGraph, false, editorElement);
    }
    catch(e) {
      msidump( e.message );
    }
  },

  resizeFrame : function(anElement, oldWidth, oldHeight, newWidth, newHeight)
  {
    // dimensions are given in pixels.
    if (oldWidth === newWidth && oldHeight === newHeight) {
      return;
    }
    try {
      var unithandler = new UnitHandler();
      var units;
// skip preserving aspect ratio for now.
// adjust width or height using aspect ratio when the time comes
//      var editorElement = msiGetActiveEditorElement();
      units = anElement.getAttribute("units");
      unithandler.initCurrentUnit(units);
      var newWidthInUnits = unithandler.getValueOf(newWidth, "px");
      var newHeightInUnits = unithandler.getValueOf(newHeight, "px");
      anElement.setAttribute("width", String(newWidthInUnits));
      anElement.setAttribute("height", String(newHeightInUnits));
      setStyleAttributeOnNode(anElement, "width", String(newWidthInUnits) + units, this.mEditor);
      setStyleAttributeOnNode(anElement, "height", String(newHeightInUnits) + units, this.mEditor);
      var parent = anElement.parentNode;
      if (parent.nodeName === 'graph') {
        // the frame is part of a graph
        var graph = new Graph();
        var obj = parent.getElementsByTagName("object")[0];
        var editorElement = msiGetActiveEditorElement();
        graph.extractGraphAttributes(parent);
        graph.setGraphAttribute("Width", String(newWidthInUnits));
        graph.setGraphAttribute("Height", String(newHeightInUnits));
        graph.reviseGraphDOMElement(parent, false, editorElement);
        if (obj) {
          doVCamInitialize(obj);
        }
      }
    }
    catch(e) {
      msidump( e.message );
    }
  },  
}

function msiResizeListenerForEditor(editor)
{
  this.mEditor = editor;
}

msiResizeListenerForEditor.prototype = msiResizeListener;

function addDOMEventListenerForEditor(editorElement)
{
  try
  {
    editorElement.contentWindow.addEventListener("DOMNodeInserted", msiEditorDOMChangeListener, false);
    editorElement.contentWindow.addEventListener("DOMNodeRemoved", msiEditorDOMChangeListener, false);
    editorElement.contentWindow.addEventListener("DOMSubtreeModified", msiEditorDOMChangeListener, true);
  }
  catch(ex) {dump("Unable to register DOM change event listener; error [" + ex + "].\n");}
}

function msiEditorDOMChangeListener(event)
{
  var targ = event.originalTarget;
  var displayNode, tableNode;
  var editor, editorElement;
  if (!targ)
    targ = event.target;
  switch(msiGetBaseNodeName(targ))
  {
    case "mtd":
    case "mtr":
    case "mtable":
    case "button":
    case "msidisplay":
      displayNode = msiNavigationUtils.getEnclosingDisplay(targ);
      if (displayNode != null)
      {
        checkNumberingOfParentEqn(displayNode);
        checkLayoutOfParentEqn(displayNode);
      }
    break;
    default:
    break;
  }
  return;
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
  msiKludgeLogString(logStr, "editorFocus");
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
    msiKludgeLogString(logStr, "editorFocus");
  }
//End logging
}

//Stolen from GetCurrentEditorElement() and hacked.
function ShutdownAllEditors()
{
  var tmpWindow = window;
  var keepgoing = true;

  try {
    // Get the <editor> element(s)
    var editorList = document.getElementsByTagName("editor");
    for (var i = 0; i < editorList.length; ++i)
    {
      if (editorList.item(i))
      {
        // the next two lines assign the OR of the two return values, but we can't use || since we
        // want the side effects of both function calls
        keepgoing = (!msiIsTopLevelEditor(editorList.item(i)));
        if (!keepgoing && msiCheckAndSaveDocument(editorList.item(i), "cmd_close", true))
          keepgoing = true;
        if (keepgoing)
          ShutdownAnEditor(editorList.item(i));
        else break;
      }
    }
  }
  catch (e) {
    msidump(e.message);
  }
  return !keepgoing;
}

function coalesceDocumentOptions(editor)
{
  var doc = editor.document;
  var optionliststring;
  var optionlist;
  var colistElements;
  var colist;
  var option = [];
  var optionval;

  var docClassElements = doc.getElementsByTagName("documentclass");
  if (docClassElements.length !== 1) {
    return;
  }
  var docclass = docClassElements[0];
  if (docclass.hasAttribute("options"))
  {
    optionliststring = docclass.getAttribute("options");
    docclass.removeAttribute("options");
    if (optionliststring && optionliststring.length > 0) {
      optionlist = optionliststring.split(/\s*\,\s*/);
    }
    colistElements = doc.getElementsByTagName("colist");
    if (colistElements.length > 0) colist = colistElements[0];
    else {
      colist = doc.createElement("colist");
      var preambles = doc.getElementsByTagName('preamble');
      if (preambles.length > 0) {
        editor.insertNode(colist, preambles[0], 0);
      }
    }
    for (i=0; i < optionlist.length; i++) {
      option = optionlist[i].split(/\s*=\s*/);
      if (option[1] && option[1].length > 0) 
        optionval = option[1];
      else optionval = option[0];
      colist.setAttribute("co_" + option[0], optionval);
    }
  }
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

    switch(aTopic)
    {
      case "obs_documentCreated":
        // Get state to see if document creation succeeded
        msiDumpWithID("Got obs_documentCreated message in documentCreated observer for editor [@]; aData is [" + aData + "]; msiGetEditor returned [" + edStr + "].\n", this.mEditorElement);
        setZoom();
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

        var is_topLevel = msiIsTopLevelEditor(this.mEditorElement);
        if (is_topLevel) {
          initMetaData(msiGetEditor(this.mEditorElement).document);
        }
        var seconds = GetIntPref("swp.saveintervalseconds");
        if (!seconds) seconds = 120;
        if (is_topLevel && (seconds > 0) && isLicensed())
          this.mEditorElement.softsavetimer = new SS_Timer(seconds*1000, editor, this.mEditorElement);
        if (!("InsertCharWindow" in window))
          window.InsertCharWindow = null;
        if (msiIsHTMLEditor(this.mEditorElement))
        {
          dump("=======" + this.mEditorElement + "========");
          try {coalesceDocumentOptions(editor);} catch(e){}
          var match;
          var dirs;
          var file;
          var path;
          var fileurl;
          var i;
          // check the document for tagdef processing instructions
          var tagdeflist = processingInstructionsList(editor.document, "sw-tagdefs", false);
          //  if nothing returned, use the default tagdefs
          if (tagdeflist.length < 1) tagdeflist = ["resource://app/res/tagdefs/latexdefs.xml"];
          for (i = 0; i < tagdeflist.length; i++)
          {
            match = (/resource:\/\/app\/res\/(.*)/i).exec(tagdeflist[i]);
            if (match != null)
            {
              dirs = match[1].split("/");
              file = getUserResourceFile( dirs[1], dirs[0]);
              fileurl = msiFileURLFromAbsolutePath( file.path );
              editor.addTagInfo(fileurl.spec);
            }
            else
              editor.addTagInfo(tagdeflist[i]);

          }

					UpdateWindowTitle();
// Add language tags if there is a <babel> tag
					addLanguageTagsFromBabelTag(editor.document)
				  var htmlurlstring = editor.document.documentURI;;
				  var htmlurl = msiURIFromString(htmlurlstring);
				  var htmlFile = msiFileFromFileURL(htmlurl);
          editor.QueryInterface(nsIEditorStyleSheets);
				  if (htmlFile)
				  {
						var cssFile = htmlFile.parent;
					  cssFile.append("css");
					  cssFile.append("msi_Tags.css");
            if (!(cssFile.exists()))
            {
              buildAllTagsViewStylesheet(editor);
            }
					  dynAllTagsStyleSheet= msiFileURLStringFromFile(cssFile);
            var stylesheet;
            try
            {
              stylesheet = editor.getStyleSheetForURL(dynAllTagsStyleSheet);
              if (stylesheet) editor.removeStyleSheet(dynAllTagsStyleSheet);
            }
            catch(e) {}  // if there was an exception, there was no stylesheet to remove
            try {
              editor.addOverrideStyleSheet(dynAllTagsStyleSheet);
              editor.enableStyleSheet(dynAllTagsStyleSheet, false);
            }catch(e) {}
					}

          try{
             var elemList = editor.document.getElementsByTagName("definitionlist");
             var defnList = null;
             var n = 0;
             if (elemList && elemList.length)
               defnList = elemList[0].getElementsByTagName("math");
             if (defnList)
               n =  defnList.length;

             var defn;
             for (var ix = 0; ix < n; ++ix)
             {
               defn = defnList[ix];
               doComputeDefine(defn)
             }
          }
          catch (e) {
            dump("Problem restoring compute definitions. Exception: " + e + "\n");
          }

          try {
            this.mEditorElement.mgMathStyleSheet = msiColorObj.Format();
          } catch(e) { dump("Error formatting style sheet using msiColorObj: [" + e + "]\n"); }
          // Now is a good time to initialize the key mapping. This is a service, and so is initialized only once. Later
          // initializations will not do anything
          var keymapper;
          try {
            keymapper =  Components.classes["@mackichan.com/keymap;1"]
                                 .createInstance(Components.interfaces.msiIKeyMap);
            keymapper.loadKeyMapFile();
            keymapper.saveKeyMaps();
          }
          catch(e) { dump("Failed to load keytables.xml -- "+e); }
        }

        if (!is_topLevel)
        {
          var parentEditorElement = msiGetParentOrTopLevelEditor(this.mEditorElement);
          if (parentEditorElement != null)
            msiFinishInitDialogEditor(this.mEditorElement, parentEditorElement);
          else
            msiDumpWithID("No parent editor element for editorElement [@] found in documentCreated observer!", this.mEditorElement);
        }
        var isInlineSpellCheckerEnabled = gPrefs.getBoolPref("swp.spellchecker.enablerealtimespell");
        RealTimeSpell.Init(editor, isInlineSpellCheckerEnabled);
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
        // Add mouse click watcher if right type of editor
//        msiDumpWithID("About to enter 'if msiIsHTMLEditor' clause in msiEditorDocumentObserver obs_documentCreated for editor [@].\n", this.mEditorElement);
        if (msiIsHTMLEditor(this.mEditorElement))
        {
          addClickEventListenerForEditor(this.mEditorElement);
          addFocusEventListenerForEditor(this.mEditorElement);
          addKeyDownEventListenerForEditor(this.mEditorElement);
          addObjectResizeListenerForEditor(this.mEditorElement);

//          addDOMEventListenerForEditor(editorElement);

          this.mEditorElement.pdfModCount = -1;

          // Force color widgets to update
          msiOnFontColorChange();
          msiOnBackgroundColorChange();
          editor.setTopXULWindow(window);
          // also initialize list of macros and fragments
          try {initializeMacrosAndFragments();} catch(e){}
          // now is the time to initialize the autosubstitute engine
          try {
            var autosub = Components.classes["@mozilla.org/autosubstitute;1"].getService(Components.interfaces.msiIAutosub);
            var autosubsfile = getUserResourceFile("autosubs.xml", "xml");
            var fileurl = msiFileURLFromFile(autosubsfile);
            dump("Opening autosubs file, path is: "+autosubsfile.path+"\n");
            autosub.initialize(fileurl.spec);
          }
          catch(e) {
            dump(e+"\n");
          }

          // Try to start compute engine
          try {
             GetCurrentEngine();
          }
          catch(e) {
            dump(e+"\n");
          }
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
            var documentInfo = new msiDocumentInfo(this.mEditorElement);
            documentInfo.initializeDocInfo();
            var dlgInfo = documentInfo.getDialogInfo();  //We aren't going to launch the dialog, just want the data in this form.
            if (dlgInfo.saveOptions.storeViewSettings)
              this.mEditorElement.viewSettings = msiGetViewSettingsFromDocument(this.mEditorElement);
            else
              msiEditorDoShowInvisibles(this.mEditorElement, getViewSettingsFromViewMenu());
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
            if (("mInitialContentListener" in this.mEditorElement) && this.mEditorElement.mInitialContentListener)
              htmlEditor.addInsertionListener(this.mEditorElement.mInitialContentListener);
            if (insertXMLAtCursor(htmlEditor, this.mEditorElement.initialEditorContents, bIsSinglePara, true))
            {
              this.mbInsertInitialContents = false;
              if (("mInitialContentListener" in this.mEditorElement) && this.mEditorElement.mInitialContentListener)
              {
                htmlEditor.removeInsertionListener(this.mEditorElement.mInitialContentListener);
                this.mEditorElement.mInitialContentListener = null;
              }
            }
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
        if (msiIsHTMLEditor(this.mEditorElement) && ("mbSinglePara" in this.mEditorElement))
        {
          msiSetEditorSinglePara(this.mEditorElement, this.mEditorElement.mbSinglePara);
        }

        if (bIsRealDocument && ("mEditorSeqInitializer" in this.mEditorElement) && (this.mEditorElement.mEditorSeqInitializer != null))
        {
          this.mEditorElement.mEditorSeqInitializer.finishedEditor(this.mEditorElement);
        }

        if (bIsRealDocument && ("initialMarker" in this.mEditorElement) && (this.mEditorElement.initialMarker.length))
        {
          var markerStr = decodeURIComponent(this.mEditorElement.initialMarker);
          delete this.mEditorElement.initialMarker;  //don't want to leave this around for later reloads or anything
          msiGoToMarker(this.mEditorElement, markerStr);
        }
        if (bIsRealDocument){
          // Convert graphics
          var doc = msiGetEditor(this.mEditorElement).document;
          var win = this.mEditorElement.contentWindow;

          var editorElement = msiGetActiveEditorElement();
          var docUrlString = msiGetEditorURL(editorElement);
          var url = msiURIFromString(docUrlString);
          var baseDir = msiFileFromFileURL(url);
          baseDir = baseDir.parent; // and now it points to the working directory
          graphicsConverter.init(win, baseDir);

          graphicsConverter.ensureTypesetGraphicsForDocument(doc, win);
        }
        if (bIsRealDocument)
          this.mEditorElement.mbInitializationCompleted = true;

      break;

      case "cmd_setDocumentModified":
//        msiDumpWithID("Hit setDocumentModified observer in base msiEditorDocumentObserver, for editor [@].\n", this.mEditorElement);
        window.updateCommands("save");
        if (msiPreviewCommand.isCommandEnabled()) {
          document.getElementById('PreviewModeButton').removeAttribute('disabled');
        } else {
          document.getElementById('PreviewModeButton').setAttribute('disabled', 'true');
        }
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
        if (editor)
        {
          var enabled={value: null};
          var can = {value: null};
          editor.canUndo(enabled, can);
          if (enabled.value && can.value) {
            document.getElementById("cmd_PreviewMode").setAttribute("disabled",  true);
          } else
            document.getElementById("cmd_PreviewMode").removeAttribute("disabled");
        }
        break;
    }
  }
}

function msiSetFocusOnStartup(editorElement)
{
  try
  {
    editorElement.contentWindow.focus();
  } catch(e) {}
}
// BBM - this function is not robust wrt localization.
// TODO - find a better way or use a localizable string.

function isShell (filename)
{
  var foundit = false;
  var re = /\/shells\//i;
  foundit = re.test(filename);
  if (!foundit)
  {
    re = /\\shells\\/i;
    foundit = re.test(filename);
  }
  return foundit;
}

function EditorStartupForEditorElement(editorElement, topwindow, isShell)
{

//  msiDumpWithID("Entering EditorStartupForEditorElement for element [@].\n", editorElement);
  var is_HTMLEditor = msiIsHTMLEditor(editorElement);
  var is_topLevel = topwindow;
	if (topwindow == null) is_topLevel = msiIsTopLevelEditor(editorElement);
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
  msiLoadInitialDocument(editorElement, is_topLevel, isShell);
}

  // Get url for editor content and load it.
  // The editor gets instantiated by the editingSession when the URL has finished loading.

  // orphaned lines of code:
  //  var dsprops = Components.classes["@mozilla.org/file/directory_service;1"].getService(Components.interfaces.nsIProperties);
  //  dir = doc.parent;

function msiLoadInitialDocument(editorElement, bTopLevel)
{
  try {
    var prefs = GetPrefs();
    var theArgs = document.getElementById("args");
    var needToCreateDirectory = false;
    var n = 1;
    var url = "";
    var docurlstring;
    var docurl;
    var docpath="";
    var docdir;
    var shelldir;
    var doc;
    var dir;
    var charset = "";
    var initMarker = "";
    var isShell = false;
    if (theArgs)
    {
      charset = theArgs.getAttribute("charset");
      isShell = theArgs.getAttribute("isShell") == "true";
      editorElement.isShellFile == isShell;
      initMarker = theArgs.getAttribute("initialMarker");
      if (initMarker && initMarker.length)
        editorElement.initialMarker = initMarker;
      docurlstring = theArgs.getAttribute("value");
      if (docurlstring.length > 0)
        docurl = msiURIFromString(docurlstring);
      if (docurl != null) dump("Url in args is "+docurl.spec+"\n");
    };
    if (!docurl && bTopLevel) {
      var prefs = GetPrefs();
      var fUseLastSavedFile;
      try
      {
        fUseLastSavedFile = prefs.getBoolPref("swp.openlastfile");
      } catch(ex) {msidump("Exception in msiLoadInitialDocument; couldn't get preference swp.lastfilesaved\n"); fUseLastSavedFile = false;}
      if (fUseLastSavedFile) {
        docurlstring = prefs.getCharPref("swp.lastfilesaved");
        if (docurlstring.length > 0) {
          var file = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
          file.initWithPath(docurlstring);
          if (file.exists())
            docurl = msiURIFromString("file://"+docurlstring);
        }
      }
    }
// Two cases: if (docurl), then the url of a doc to load was passed. It might be for a shell.
//            if (!docurl), nothing was passed. Load the default shell.
    if (!docurl)
    {
      editorElement.isShellFile = true;
      editorElement.fileLeafName = "unsaved file";
      if (!bTopLevel)
      {
        try {
          docurlstring = prefs.getCharPref("swp.defaultDialogShell");   //BBM: check this later
        }
        catch(exc) {
          dump("In msiLoadInitialDocument(), trying to get default dialog shell, error: " + exc + ".\n");
          docurlstring = "chrome://prince/content/StdDialogShell.xhtml";
//          docurl = null;
//          return;
        }
        if (docurlstring.length > 0)
          docurl = msiURIFromString(docurlstring);
        else
          return;
      }
      else
      {
        docurlstring = prefs.getCharPref("swp.defaultShell");
        // docurlstring is *relative* to the shells directory.
        if (docurlstring.length == 0)
          docurlstring = "-Standard_LaTeX/Standard_LaTeX_Article.sci";
        var dsprops = Components.classes["@mozilla.org/file/directory_service;1"].getService(Components.interfaces.nsIProperties);
        var doc = dsprops.get("resource:app", Components.interfaces.nsILocalFile);
        doc.append("shells");
        var dirs = docurlstring.split(/\//);
        var i;
        for (i = 0; i<dirs.length; i++) if (dirs[i].length >0) doc.append(dirs[i]);
        docpath = doc.path;
        docurl = msiFileURLFromAbsolutePath(docpath); // this converts docurl to a file URL
        needToCreateDirectory = true;
      }
    }
    else editorElement.isShellFile = isShell;
    if (!docurl)
    {
      dump("Unable to find Doc Url\n");
      return;
    }
// now we have a url for the doc.

    if (docurl && docurl.schemeIs("resource"))
    {
      // convert to a file URL
      docpath = msiPathFromFileURL( docurl );
      docurl = msiFileURLFromAbsolutePath(docpath); // this converts docurl to a file URL
    }

    if (!docurl.schemeIs("chrome"))
    {
      doc = msiFileFromFileURL(docurl);
      if (!doc)
      {
        docpath = docurl.path;
        doc = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
        try
        { doc.initWithPath(docpath); }
        catch(ex) {doc = null;}
      }
      if (doc)
        dir = doc.parent;
    }
    if (!editorElement.isShellFile && doc) editorElement.fileLeafName = doc.leafName;
    // in cases where the user has gone through a dialog, such a File/New or File/Open, the working directory
    // has already been created and the document name changed. When starting up, or starting with a file on the
    // command line we still need to call "createWorkingDirectory."
    var isSciRegEx = /\.sci$/i;
    var isSci = isSciRegEx.test(docurl.spec);
    if (isSci)
    {
      var newdoc;
      newdoc = createWorkingDirectory(doc);
      docurl = msiFileURLFromFile(newdoc);
    }

    var contentViewer = null;
    if (editorElement.docShell)
      contentViewer = editorElement.docShell.contentViewer;
    if (contentViewer)
    {
      contentViewer.QueryInterface(Components.interfaces.nsIMarkupDocumentViewer);
      contentViewer.defaultCharacterSet = charset;
      contentViewer.forceCharacterSet = charset;
    }
    dump("Trying to load editor with url = "+docurl.spec+"\n");
    msiEditorLoadUrl(editorElement, docurl);
    msiUpdateWindowTitle();
  }
  catch (e) {
    dump("Error in loading URL in msiLoadInitialDocument: [" + e + "]\n");
  }
//  msiDumpWithID("Back from call to msiEditorLoadUrl for editor [@].\n", editorElement);
}

//   // Get url for editor content and load it.
//   // the editor gets instantiated by the editingSession when the URL has finished loading.
// function msiPrepareDocumentTargetForShell(docname)
// {
//   try {
//     var prefs = GetPrefs();
//     var filename = "untitled";
//     var theArgs = document.getElementById("args");
//     var n = 1;
//     var url = "";
//     var docdir;
// //    var shelldir;
// //    var shellfile;
// //    var charset = "";
//     var leafname = "";
// //    if (theArgs)
// //    {
// //      docname = document.getElementById("args").getAttribute("value");
// //      if (docname.indexOf("file://") == 0) docname = GetFilepath(docname);
// //  // for Windows
// //#ifdef XP_WIN32
// //      docname = docname.replace("/","\\","g");
// //#endif
// //      charset = document.getElementById("args").getAttribute("charset")
// //    };
//     var dsprops = Components.classes["@mozilla.org/file/directory_service;1"].getService(Components.interfaces.nsIProperties);
// //    dump("docname starts out as " + docname + "\n");
// //    if (docname.length == 0)
// //    {
// //       docname=prefs.getCharPref("swp.defaultShell");
// //       shellfile = dsprops.get("resource:app", Components.interfaces.nsILocalFile);
// //       shellfile.append("shells");
// //       dump("shelldir is "+shellfile.path + "\n");
// //       docname = shellfile.path + "/" + docname;
// //  // for Windows
// //#ifdef XP_WIN32
// //      docname = docname.replace("/","\\","g");
// //#endif
// //      dump("Docname is " + docname + "\n");
// //    }
// //    else
// //    {
// //      shellfile = Components.classes["@mozilla.org/file/local;1"].
// //            createInstance(Components.interfaces.nsILocalFile);
// //    }
// //    if (isShell(docname))
// //    {
// //      editorElement.isShellFile = true;
//
//
//     var shellfile = Components.classes["@mozilla.org/file/local;1"].
//           createInstance(Components.interfaces.nsILocalFile);
//     shellfile.initWithPath(docname);
//     // copy the shell to the default document area
//     try
//     {
//       var docdirname = prefs.getCharPref("swp.prefDocumentDir");
//       dump("swp.prefDcoumentDir is ", docdirname + "\n");
//       docdir = Components.classes["@mozilla.org/file/local;1"].
//           createInstance(Components.interfaces.nsILocalFile);
//       docdir.initWithPath(docdirname);
//       if (!valueOf(docdir.exists()))
//         docdir.create(1, 0755);
//     }
//     catch (e)
//     {
//       var dirkey;
//       var dirname;
// #ifdef XP_WIN
//         dirkey = "Pers";
//         dirname = "SWP Docs";
//         // dirname = "SW Docs";
//         // dirname = "SNB Docs";
// #else
// #ifdef XP_MACOSX
//         dirkey = "UsrDocs";
// #else
//         dirkey = "Home";
// #endif
//         dirname = "SWP_Docs";
//         // dirname = "SW_Docs";
//         // dirname = "SNB_Docs";
// #endif
//       // if we can't find the one in the prefs, get the default
//       docdir = dsprops.get(dirkey, Components.interfaces.nsILocalFile);
//       if (!docdir.exists()) docdir.create(1,0755);
//       docdir.append( dirname );
//       if (!docdir.exists()) docdir.create(1,0755);
//       dump("default document directory is "+docdir.path+"\n");
//     }
//     // find n where untitledn.MSI_EXTENSION is the first unused file name in that folder.
//     // Windows version expected a copy failure to indicate the file already exists.
//     // That doesn't work in Unix---the copy always succeeds.
//     // Instead we explicitly look for a file that doesn't yet exist, and use that file.
//     try
//     {
//       leafname = shellfile.leafName;
//       var i = leafname.lastIndexOf(".");
//       if (i > 0) leafname = leafname.substr(0,i);
//       // dump("leafname is " + leafname + "\n");
//       while (n < 100)
//       {
//         var targetfile =  Components.classes["@mozilla.org/file/local;1"].
//           createInstance(Components.interfaces.nsILocalFile);
//         targetfile.initWithPath(docdir.path);
//         targetfile.append(filename + n+"." + MSI_EXTENSION);
//         // dump( "File name is "+targetfile.path+"\n");
//         if( targetfile.exists() ) {
//           n++;
//           // dump( targetfile.path+" exists. New n is "+n+"\n");
//         } else break;
//       }
//       if (n>=100) dump("Too many untitled files. using untitled100\n");
//           // dump("Copying "+shellfile.path + " to directory " + docdir.path + ", file "+filename+n+"."+MSI_EXTENSION+"\n");
//           shellfile.copyTo(docdir,filename+n+"."+MSI_EXTENSION);
//           // dump("Copy done.\n");
//           try {
//             var auxdir = shellfile.parent.clone();
//             // dump("auxdir is " + auxdir.path+"\n");
//             auxdir.append(leafname+"_files");
//             auxdir.copyTo(docdir,filename+n+"_files");
//           }
//           catch(e) {
//             dump("Copying auxdir caused error "+e+"\n");
//           }
//     }// at this point, shellfile is .../untitledxxx.MSI_EXTENSION
//     catch(e)
//     {
//       dump("Unable to open document for editing\n");
//     }
//     // set the url for the xhtml portion of the document
//     docdir.append(filename + n+"." + MSI_EXTENSION);
//     // dump("Final docdir path is " + docdir.path + "\n");
//     url = docdir.path;
//     // dump("url is " + url + "\n");
//   }
//   catch(e)
//   {
//     dump("Uncaught error in msiPrepareDocumentTargetForShell: " + e + "\n.");
//   }
//   return url;
//
// //      // set document title when?? It is too early now, so we will save the name in the XUL
// //      var theFilename = document.getElementById("filename");
// //      if (theFilename != null)
// //        theFilename.value = filename+n;
// //    }
// //    else
// //    {
// //      url = docname;
// //  // for Windowss
// //#ifdef XP_WIN32
// //      docname = docname.replace("\\","/","g");
// //#endif
// //      var dotIndex = docname.lastIndexOf(".");
// //      var lastSlash = docname.lastIndexOf("/")+1;
// //      var theFilename = document.getElementById("filename");
// //      if (theFilename != null)
// //        theFilename.value = dotIndex == -1?docname.slice(lastSlash):docname.slice(lastSlash,dotIndex);
// //    }
// ////    var contentViewer = GetCurrentEditorElement().docShell.contentViewer;
// //    var contentViewer = null;
// //    if (editorElement.docShell)
// //      contentViewer = editorElement.docShell.contentViewer;
// //    if (contentViewer)
// //    {
// //      contentViewer.QueryInterface(Components.interfaces.nsIMarkupDocumentViewer);
// //      contentViewer.defaultCharacterSet = charset;
// //      contentViewer.forceCharacterSet = charset;
// //    }
// //    msiEditorLoadUrl(editorElement, url);
// ////    msiDumpWithID("Back from call to msiEditorLoadUrl for editor [@].\n", editorElement);
// //  } catch (e) {
// //    dump("Error in loading URL in EditorStartupForEditorElement: [" + e + "]\n");
// //  }
// }

function msiFinishInitDialogEditor(editorElement, parentEditorElement)
{

  var parentEditor = msiGetEditor(parentEditorElement);
//  msiDumpWithID("In msiEditor.msiFinishInitDialogEditor for editorElement [@], parentEditor is [" + parentEditor + "].\n", editorElement);
  if (parentEditor)
  {
    //NOTE THE FOLLOWING! This means that if you want a dialog editor to allow multiple paragraphs, you must set mbSinglePara=false before
    //  calling the editor initialization. This reflects that single-para is the default.
    if (!("mbSinglePara" in editorElement))
      editorElement.mbSinglePara = true;
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
  if ( ("mbSetFocusOnStartup" in editorElement) && editorElement.mbSetFocusOnStartup)
    editorElement.contentWindow.focus();
  else if ( ("mResetInitialFocusTo" in editorElement) && (editorElement.mResetInitialFocusTo != null))
  {
    editorElement.mResetInitialFocusTo.focus();
    editorElement.mResetInitialFocusTo = null;
  }
}

function guaranteeSciFile( url )
{
  if (url.scheme !== "file") return;
  var file = msiFileFromFileURL(url); // file is .../foo_work/main.xhtml, usually
  var dir = file.parent;            // dir is .../foo_work
  var leaf = dir.leafName;                // leaf is "foo_work"
  leaf = leaf.replace("_work","");    // leaf is now "foo"
  var zipfile = dir.parent;
  zipfile.append(leaf+".sci"); // zipfile is foo.sci, sibling of foo_work
  if (!zipfile.exists())              // there was no sci file corresponding to foo_work, as in the shell file case
  {   // correct that now
    var compression;
    var prefs = Components.classes["@mozilla.org/preferences-service;1"].getService(Components.interfaces.nsIPrefBranch);
    try
    {
      compression = prefs.getIntPref("swp.sci.compression");
    }
    catch(ex2) {compression = 0;}
    var zw = Components.classes["@mozilla.org/zipwriter;1"]
                          .createInstance(Components.interfaces.nsIZipWriter);
    zipfile.create(0, 0755);
    zw.open( zipfile, PR_RDWR | PR_CREATE_FILE | PR_TRUNCATE);
    zipDirectory(zw, "", dir, compression);
    zw.close();
  }
}

function msiEditorLoadUrl(editorElement, url, markerStr)
{
  dump("msiEditorLoadUrl: url.spec= "+url.spec+"\n");
  guaranteeSciFile(url);
  try {
    if (markerStr)
      editorElement.initialMarker = markerStr;
    if (url)
      editorElement.webNavigation.loadURI(url.spec, // uri string
             msIWebNavigation.LOAD_FLAGS_BYPASS_CACHE,     // load flags
             null,                                         // referrer
             null,                                         // post-data stream
             null);
  } 
  catch (e) { dump(" EditorLoadUrl failed: "+e+"\n"); }
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

  var isMac = (getOS(window) == "osx");

  // Set platform-specific hints for how to select cells
  // Mac uses "Cmd", all others use "Ctrl"
  var tableKey = GetString(isMac ? "XulKeyMac" : "TableSelectKey");
  var dragStr = tableKey+GetString("Drag");
  var clickStr = tableKey+GetString("Click");

  var delStr = GetString(isMac ? "Clear" : "Del");

  SafeSetAttribute(theDocument, "menu_SelectCell", "acceltext", '');
  SafeSetAttribute(theDocument, "menu_SelectRow", "acceltext", '');
  SafeSetAttribute(theDocument, "menu_SelectColumn", "acceltext", '');
  SafeSetAttribute(theDocument, "menu_SelectAllCells", "acceltext", '');
  // And add "Del" or "Clear"
  SafeSetAttribute(theDocument, "menu_DeleteCellContents", "acceltext", '');

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
  var enginelog = document.getElementById("EngineConsoleBox");
  if (enginelog) enginelog.destroy();
  try
  {
    msiRemoveActiveEditor(editorElement);
    msiRemoveToolbarPrefListener(editorElement);
    if ("mCSSPrefListener" in editorElement)
      editorElement.mCSSPrefListener.shutdown();
  } catch(e) { msiDumpWithID( "In ShutdownAnEditor for editor [@], removing active editor and toolbar pref listener, error: " + e + "\n", editorElement ); }

  try {
    if (msiIsTopLevelEditor(editorElement)) deleteWorkingDirectory(editorElement);
    var commandManager = msiGetCommandManager(editorElement);
    if ("mEditorDocumentObserver" in editorElement)
    {
      commandManager.removeCommandObserver(editorElement.mEditorDocumentObserver, "obs_documentCreated");
      commandManager.removeCommandObserver(editorElement.mEditorDocumentObserver, "obs_documentWillBeDestroyed");
      commandManager.removeCommandObserver(editorElement.mEditorDocumentObserver, "obs_documentLocationChanged");
    }
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
  } catch (e) {msiDumpWithID( "In ShutdownAnEditor for editor [@], removing command observers, error: " + e + "\n", editorElement );}
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


function doRevert(aContinueEditing, editorElement, del)
{
  var urlstring = msiGetEditorURL(editorElement);
  var url = msiURIFromString(urlstring);
  var documentfile = msiFileFromFileURL(url);
  msiRevertFile( aContinueEditing, documentfile, del );
}

function msiCheckAndSaveDocument(editorElement, command, allowDontSave)
{
  var document;
  editorElement.focus();
  try {
    // if we don't have an editor or an document, bail
    var editor = msiGetEditor(editorElement);
    if (!editor) return true;
    document = editor.document;
    if (!document)
      return true;
		var htmlurlstring = msiGetEditorURL(editorElement);
	  var sciurlstring = msiFindOriginalDocname(htmlurlstring);
	  var fileURL = msiURIFromString(sciurlstring);
	  var file = msiFileFromFileURL(fileURL);
		var scifileExists = file.exists();
    var prefs = GetPrefs();
    var newfileisdirty = prefs.getBoolPref("swp.newFileConsideredDirty");
    if ((!editor.documentModified) && (!msiIsHTMLSourceChanged(editorElement)) && (scifileExists || !newfileisdirty))
    {
      if (command == "cmd_close" && ("isShellFile" in editorElement) && editorElement.isShellFile)
      // if the document is a shell and has never been saved, it will be deleted by Revert
        if (file.exists()) file.remove(false);
      return true;
    }
  }
	catch (e) {
		return false;
	}

  // call window.focus, since we need to pop up a dialog
  // and therefore need to be visible (to prevent user confusion)
  top.document.commandDispatcher.focusedWindow.focus();

  var scheme = GetScheme(htmlurlstring);
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
    default:
      break;
  }

  var reasonToSave = strID ? GetString(strID) : "";

  if (/_work/.test(sciurlstring))
  {
    sciurlstring = sciurlstring.replace((/_work\/[^\/]*\.[a-z0-9]+$/i),"")+".sci";
  }
  var leafregex = /.*\/([^\/]+$)/;
  var arr = leafregex.exec(sciurlstring);
  if (arr && arr.length >1) document.title = arr[1];
  if (!document.title) document.title=GetString("untitled");

  var dialogTitle = GetString("SaveDocument");
  var dialogMsg = GetString("SaveFilePrompt");
  dialogMsg = (dialogMsg.replace(/%title%/,document.title)).replace(/%reason%/,reasonToSave);

  var promptService = msiGetPromptService();
  if (!promptService)
    return false;

  var result = {value:0};
  var promptFlags = promptService.BUTTON_TITLE_CANCEL * promptService.BUTTON_POS_1;
  var button1Title = null;
  var button3Title = GetString("DontSave");

  promptFlags += promptService.BUTTON_TITLE_SAVE * promptService.BUTTON_POS_0;

  // If allowing "Don't..." button, add that
  if (allowDontSave)
    promptFlags += (promptService.BUTTON_TITLE_DONT_SAVE * promptService.BUTTON_POS_2);

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
    var success = msiSaveDocument(false, false, false, contentsMIMEType, editor, editorElement);
    return success;
  }

  if (result == 2)
  {
    // "Don't Save"
    if (command == "cmd_close")
    {
      var del = (("isShellFile" in editorElement) && (editorElement.isShellFile));
      doRevert(false, editorElement, del);
    }
    return true;
  }
  // Default or result == 1 (Cancel)
  return false;
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
                             "plaintext",
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
    var editorElement = msiGetActiveEditorElement();
    zoomfactor = msiGetSavedViewPercent(editorElement);
    if (zoomfactor == null) {
      zoomstr = gPrefs.getCharPref("swp.zoom_factor");
      zoomfactor = parseFloat(zoomstr);
    }
  }
  catch(ex) {
    dump("\nfailed to get zoom_factor pref!\n");
  }
  //ZoomManager.zoom = zoomfactor;
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
  const XUL_NS = "http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul";
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
    window.openDialog("chrome://editor/content/EdColorPicker.xul", "colorpicker", "chrome,close,titlebar,modal,resizable", "", editorElement.mColorObj);

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
    if ((node.nodeName.toLowerCase() == "table") || (node.nodeName.toLowerCase() == "mtable"))
      return node;

    node = node.parentNode;
  }
  return node;
}

function GetParentTableCell(element)
{
  var node = element;
  var name;
  while (node)
  {
    name = node.nodeName.toLowerCase();
    if (name == "td" || name == "th" || name == "mtd")
      return node;

    node = node.parentNode;
  }
  return node;
}


function EditorDblClick(event)
{
  // We check event.explicitOriginalTarget here because .target will never
  // be a textnode (bug 193689)
  var editorElement = msiGetEditorElementFromEvent(event);
  msiSetActiveEditor(editorElement, false);
  var element = event.explicitOriginalTarget;

  if (!element) {
    try {
      element = msiGetEditor(editorElement).getSelectedElement("href");
    } catch (e) {}
  }

  if (element)
  {
    goDoPrinceCommand("cmd_objectProperties", element, editorElement);
    event.preventDefault();
  }
}


function EditorClick(event)
{
  if (!event)
    return;
//  if (event.target.onclick) return;
  var editorElement = msiGetEditorElementFromEvent(event);
  try
  {
    if (event.detail == 2)
    {
      EditorDblClick(event);
      return;
    }
    else if (event.detail == 1)
    {
      var obj, theURI, targWin;
      var objName = msiGetBaseNodeName(event.target);
	    var editor = msiGetEditor(editorElement);
	    var graphnode = getEventParentByTag(event, "graph");
      var linkNode;
      if (!graphnode)
      {
        if (document.getElementById("vcamactive") && document.getElementById("vcamactive").getAttribute("hidden")=="false")
        {
          document.getElementById("vcamactive").setAttribute("hidden", true);
        }
	      linkNode = getEventParentByTag(event, "xref");
        if (!linkNode)
	        linkNode = getEventParentByTag(event, "a");
      }
      if (graphnode)
      {
        var obj = graphnode.getElementsByTagName("object")[0];
        if (obj) {
          doVCamInitialize(obj);
        }
//        if (obj.wrappedJSObject) obj = obj.wrappedJSObject;  // not necessary here
      }
      else if (linkNode && (objName=="xref"))
      {
        theURI = event.target.getAttribute("href");
        if (!theURI)
          theURI = event.target.getAttribute("key");
        if (theURI && theURI.length)
          theURI = "#" + theURI;
        msiClickLink(event, theURI, targWin, editorElement);
      }
      else if (linkNode)
      {
        theURI = event.target.getAttribute("href");
        if (event.target.hasAttribute("target"))
          targWin = event.target.getAttribute("target");
        msiClickLink(event, theURI, targWin, editorElement);
      }
    }
  } catch(ex) {
    msidump("Exception in msiEditor.js, EditorClick() : " + ex + "\n");
  }

//  event.currentTarget should be "body" or something...

  // For Web Composer: In Show All Tags Mode,
  // single click selects entire element,
  //  except for body and table elements
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

//The "bToRight" parameter indicates the direction in which to resolve positions between two cells.
//  Thus, for instance, it would be false if the position is at the right end of a selection but true at the left end...
function msiFindCellFromPositionInTableOrMatrix(aTableNode, aChildNode, anOffset, bToRight, anEditor)
{
  var retNode = null;
  var topChild = aTableNode;
  var nextChild = null;
  var newOffset = 0;
  var bTakeLast = false;

  function isAboveCellLevel(aNode)
  {
    switch(msiGetBaseNodeName(aNode))
    {
      case "table":
      case "thead":
      case "tbody":
      case "tfoot":
      case "mtable":
      case "tr":
      case "mtr":
      case "mlabeledtr":
        return true;
      break;
      default:
      break;
    }
    return false;
  }

  function containsNoCells(aNode)
  {
    var childList = null;
    switch(msiGetBaseNodeName(aNode))
    {
      case "colgroup":
      case "col":
        return true;
      break;
      case "table":
      case "thead":
      case "tbody":
      case "tr":
        childList = aNode.getElementsByTagName("td");
        if (childList && childList.length > 0)
          return false;
      break;
      case "mtable":
      case "mtr":
      case "mlabeledtr":
        childList = aNode.getElementsByTagName("mtd");
        if (childList && childList.length > 0)
          return false;
      break;
    }
    if (!isAboveCellLevel(aNode))
      return false;
    childList = msiNavigationUtils.getSignificantContents(aNode);
    for (var ix = 0; ix < childList.length; ++ix)
    {
      if (!containsNoCells(childList[ix]))
        return false;
    }
    return true;
  }

  for (nextChild = msiNavigationUtils.findTopChildContaining(aChildNode, topChild); !retNode && (nextChild != null); nextChild = msiNavigationUtils.findTopChildContaining(aChildNode, topChild))
  {
    if (!isAboveCellLevel(nextChild))
    {
      retNode = nextChild;
      break;
    }
    if (containsNoCells(nextChild))
    {
      nextChild = nextChild.nextSibling;
      break;
    }
    if (aChildNode == nextChild)
      break;
    topChild = nextChild;
  }

  var ourOffset = anOffset;
  while (!retNode && nextChild)
  {
    //If we arrive here, we should have a node at a level above cells. If it's the same as the node we started out with, we use the
    //offset handed in and go looking for a cell-level construct.
    if (containsNoCells(nextChild))
      nextChild = bToRight ? nextChild.nextSibling : nextChild.previousSibling;
    else
    {
      if (nextChild != aChildNode)
        ourOffset = bToRight ? 0 : msiNavigationUtils.lastIndex(nextChild);
      if (bToRight || anOffset == 0)
      {
        nextChild = msiNavigationUtils.getSignificantChildFollowingPosition(nextChild, ourOffset, false);
        if (!nextChild)
          nextChild = msiNavigationUtils.getSignificantChildPrecedingPosition(nextChild, ourOffset, false);
      }
      else
      {
        nextChild = msiNavigationUtils.getSignificantChildPrecedingPosition(nextChild, ourOffset, false);
        if (!nextChild)
          nextChild = msiNavigationUtils.getSignificantChildFollowingPosition(nextChild, ourOffset, false);
      }
    }
    if (!isAboveCellLevel(nextChild))
      retNode = nextChild;
  }

  if (retNode && msiKludgeTestKeys(["tableEdit"]))
  {
    var retName = msiGetBaseNodeName(retNode);
    switch(retName)
    {
      case "td":
      case "th":
      case "mtd":
      break;  //we're okay
      default:
        msiKludgeLogNodeContents(retNode, ["tableEdit"], "Didn't find good cell in table or mtable in msiFindCellFromPositionInATableOrMatrix; node returned ", true);
      break;
    }
  }

  return retNode;
}

//function msiFindCellFromPositionInTableOrMatrix(aTableNode, aChildNode, anOffset, bToRight, anEditor)
//{
//  var retNode = null;
//  var topChild = null;
//  var newOffset = 0;
//  var bCollapsed = anEditor.selection.isCollapsed;
//  if (aChildNode == aTableNode)
//  {
//    if (anOffset < aTableNode.childNodes.length)
//      topChild = aTableNode.childNodes[anOffset];
//    else
//      topChild = aTableNode.childNodes[aTableNode.childNodes.length - 1];
//    if (!bShift || !bCollapsed)
//      newOffset = topChild.childNodes.length;
//    return findCellFromPositionInTableOrMatrix(aTableNode, topChild, newOffset, bShift, anEditor);
//  }
//  var nextChild = null;
//  switch(msiGetBaseNodeName(aChildNode))
//  {
//    case "rowgroup":
//    case "tr":
//    case "mtr":
//      if (anOffset < aChildNode.childNodes.length)
//        nextChild = aChildNode.childNodes[anOffset];
//      else
//        nextChild = aChildNode.childNodes[aChildNode.childNodes.length - 1];
//      if (!bShift || !bCollapsed)
//        newOffset = nextChild.childNodes.length;
//      return msiFindCellFromPositionInTableOrMatrix(aTableNode, nextChild, newOffset, bShift, anEditor);
//    break;
//    case "td":
//    case "th":
//    case "mtd":
//      retNode = aChildNode;
//    break;
//    default:
//      topChild = msiNavigationUtils.findTopChildContaining(aChildNode, aTableNode);
//      if (topChild == aChildNode)  //What now? Is this the cell?
//      {
//        if (anOffset < aChildNode.childNodes.length)
//          nextChild = aChildNode.childNodes[anOffset];
//        else
//          nextChild = aChildNode.childNodes[aChildNode.childNodes.length - 1];
//        if (!bShift || !bCollapsed)
//          newOffset = nextChild.childNodes.length;
//        return msiFindCellFromPositionInTableOrMatrix(aTableNode, nextChild, newOffset, bShift, anEditor);
//      }
//      nextChild = topChild;
//      while (!retNode)
//      {
//        switch(msiGetBaseNodeName(nextChild))
//        {
//          case "td":
//          case "th":
//          case "mtd":
//            retNode = topChild;
//          break;
//        }
//        topChild = nextChild;
//        nextChild = msiNavigationUtils.findTopChildContaining(aChildNode, topChild);
//        if (!retNode && ((nextChild == aChildNode) || !nextChild) )
//        {
//          //we can't find anything else! just use aChildNode
//          retNode = aChildNode;
//        }
//      }
//    break;
//  }
//  return retNode;
//}

function msiEditorNextField(bShift, editorElement)
{
//  function canDoTab(aNode)
//  {
//    if (msiNavigationUtils.isMathTemplate(aNode))
//      return true;
//    switch(msiGetBaseNodeName(aNode))
//    {
//      case "mtable":
//      case "msqrt":  //separated as it will require special handling
//        return true;
//      break;
//      default:
//      break;
//    }
//    return false;
//  }

  function squareRootDoNextField(aNode, totalRange, bShift, anEditor)
  {
    var newRoot = anEditor.document.createElementNS(mmlns, "mroot");
    var children = msiNavigationUtils.getSignificantContents(aNode);
    anEditor.beginTransaction();
    if (children.length == 1)
      msiEditorMoveChild(newRoot, children[0], anEditor);
    else
    {
      var newRow = aNode.ownerDocument.createElementNS(mmlns, "mrow");
      msiEditorMoveChildren(newRow, aNode, anEditor);
      anEditor.insertNode(newRow, newRoot, 0);
    }
    var insertBox = newbox(anEditor);
    anEditor.insertNode(insertBox, newRoot, 1);
    anEditor.replaceNode(newRoot, aNode, aNode.parentNode);
    return { mNode : insertBox, mOffset : 1, bInTransaction : true };
  }

  //It's assumed that "aChildNode" is either a cell or within one.
  function tableDoNextField(aNode, totalRange, bShift, anEditor)
  {
//    var bCollapsed = anEditor.selection.isCollapsed;
//    var anOffset = 0;
//    if (!bShift || !bCollapsed)
//      anOffset = msiNavigationUtils.lastOffset(aChildNode);
    var bIsTableEditor = false;
    if ((anEditor instanceof nsITableEditor))
      bIsTableEditor = true;
    dump("In msiEditor.js, tableDoNextField(), anEditor " + (bIsTableEditor ? "is" : "is NOT") + " an instance of nsITableEditor.\n");
    var aTableCell = msiFindCellFromPositionInTableOrMatrix(aNode, totalRange.endContainer, totalRange.endOffset, bShift, anEditor);
//    if (!bCollapsed)
//      return aTableCell;
    var nRowObj = {value : 0};
    var nColObj = {value : 0};
    anEditor.getTableSize(aNode, nRowObj, nColObj);
    var numRows = nRowObj.value;
    var numCols = nColObj.value;
    anEditor.getCellIndexes(aTableCell, nRowObj, nColObj);
    var nRow = nRowObj.value;
    var nCol = nColObj.value;
    var startCol = nCol;
    var startRow = nRow;
    var selNode = null;
    var selInfo = null;
    var bDone = false;
    while (!bDone)
    {
      if (bShift)
      {
        if (!nCol)
        {
          nCol = numCols - 1;
          if (!nRow)
            nRow = numRows;
          --nRow;
        }
        else
          --nCol;
      }
      else
      {
        ++nCol;
        if (nCol == numCols)
        {
          nCol = 0;
          ++nRow;
          if (nRow == numRows)
            nRow = 0;
        }
      }
      if (nRow == startRow && nCol == startCol)
        break;
      selNode = anEditor.getCellAt(aNode, nRow, nCol);
      if (selNode != aTableCell)
        bDone = true;
      //Now check to be sure this isn't going back to the top of a multi-row cell
      if (bDone)
      {
        anEditor.getCellIndexes(selNode, nRowObj, nColObj);
        if (nRowObj.value != nRow)
          bDone = false;
      }
    }
    if (selNode)
    {
      selInfo = new Object();
      selInfo.mNode = selNode;
      selInfo.mOffset = bShift ? msiNavigationUtils.lastOffset(selInfo.mNode) : 0;
      selInfo.bInTransaction = false;
    }
    return selInfo;
  }

  //It's assumed that "aChildNode" is either a cell or within one.
  function mTableDoNextField(aNode, totalRange, bShift, anEditor)
  {
//    var bCollapsed = anEditor.selection.isCollapsed;
//    var anOffset = totalRange.endOffset;
//    var childNode = totalRange.endContainer;
//    if (!bShift || !bCollapsed)
//      anOffset = msiNavigationUtils.lastOffset(aChildNode);
    var aTableCell = msiFindCellFromPositionInTableOrMatrix(aNode, totalRange.endContainer, totalRange.endOffset, bShift, anEditor);
    var mathmlEditor = anEditor.QueryInterface(Components.interfaces.msiIMathMLEditor);
//    if (!bCollapsed)
//      return aTableCell;
    var nRowObj = {value : 0};
    var nColObj = {value : 0};
    mathmlEditor.findMatrixCell(aNode, aTableCell, nRowObj, nColObj);
    var nRow = nRowObj.value;
    var nCol = nColObj.value;
    mathmlEditor.getMatrixSize(aNode, nRowObj, nColObj);
    var numRows = nRowObj.value;
    var numCols = nColObj.value;
    var startCol = nCol;
    var startRow = nRow;
    var selInfo = null;
    var selNode = null;
    var bDone = false;
    while (!bDone)
    {
      if (bShift)
      {
        if (nCol == 1)
        {
          nCol = numCols;
          --nRow;
          if (!nRow)
            nRow = numRows;
        }
        else
          --nCol;
      }
      else
      {
        if (nCol == numCols)
        {
          nCol = 1;
          ++nRow;
          if (nRow > numRows)
            nRow = 1;
        }
        else
          ++nCol;
      }
      if (nRow == startRow && nCol == startCol)
        break;
      selNode = mathmlEditor.getMatrixCellAt(aNode, nRow, nCol);
      if (selNode && (selNode != aTableCell))
        bDone = true;
      //Now check to be sure this isn't going back to the top of a multi-row cell
      if (bDone)
      {
        mathmlEditor.findMatrixCell(aNode, selNode, nRowObj, nColObj);
        if (nRowObj.value != nRow)
          bDone = false;
      }
    }
    if (selNode)
    {
      selInfo = new Object();
      selInfo.mNode = selNode;
      selInfo.mOffset = bShift ? msiNavigationUtils.lastOffset(selInfo.mNode) : 0;
      selInfo.bInTransaction = false;
    }
    return selInfo;
  }

  function scriptDoNextField(aNode, totalRange, bShift, anEditor)
  {
    if (!totalRange.collapsed)
      return null;

    var nPos = 0;
    var topChild = null;
    if (totalRange.startContainer == aNode)
    {
      nPos = totalRange.startOffset;
      if (nPos == 0)
        topChild = msiNavigationUtils.getFirstSignificantChild(aNode);
      else
        topChild = msiNavigationUtils.getNodeBeforePosition(aNode, nPos);
    }
    else
    {
      topChild = msiNavigationUtils.findTopChildContaining(totalRange.startContainer, aNode);
    }
    nPos = msiNavigationUtils.significantOffsetInParent(topChild);
    if (nPos <= 0)  //Either topChild isn't a "significant" child (should NOT happen here!) or it's at the base position as opposed to being in the script.
      return null;

    var selInfo = null;
    if (nPos > 2)  //Should never happen anyway!
      nPos = 2;
    var newNodeName = "";
    var nMoveToPos = -1;
    var newNode = null;
    switch(msiGetBaseNodeName(aNode))
    {
      case "msub":
        nMoveToPos = 1;  //then fallthrough
      case "msup":
        if (nMoveToPos < 0)
          nMoveToPos = 2;
        newNodeName = "msubsup";
      break;
      case "munder":
        nMoveToPos = 1;  //then fallthrough
      case "mover":
        if (nMoveToPos < 0)
          nMoveToPos = 2;
        newNodeName = "munderover";
      break;
      case "msubsup":
      case "munderover":
        //The easy cases - just return the other script child
        selInfo = new Object();
        selInfo.mNode = msiNavigationUtils.getIndexedSignificantChild(aNode, 3 - nPos);
        selInfo.mOffset = bShift ? msiNavigationUtils.lastOffset(selInfo.mNode) : 0;
        selInfo.bInTransaction = false;
      break;
    }
    if (newNodeName.length && (nMoveToPos > 0))
    {
      anEditor.beginTransaction();
      newNode = anEditor.document.createElementNS(mmlns, newNodeName);
      msiEditorMoveChildToPosition(newNode, nMoveToPos, topChild, anEditor);
      msiEditorMoveChildToPosition(newNode, 0, msiNavigationUtils.getFirstSignificantChild(aNode), anEditor);
      var insertBox = newbox(anEditor);
      anEditor.insertNode(insertBox, newNode, 3-nMoveToPos);
      anEditor.replaceNode(newNode, aNode, aNode.parentNode);
      selInfo = new Object();
      selInfo.mNode = insertBox;
      selInfo.mOffset = 1;
      selInfo.bInTransaction = true;
    }
    return selInfo;
  }

  function nodeDoNextField(aNode, totalRange, bShift, anEditor)
  {
    var selInfo = null;
    var childNode = null;
    switch(msiGetBaseNodeName(aNode))
    {
      case "msub":
      case "msup":
      case "msubsup":
      case "munder":
      case "mover":
      case "munderover":
        selInfo = scriptDoNextField(aNode, totalRange, bShift, anEditor);
      break;

      case "table":
        selInfo = tableDoNextField(aNode, totalRange, bShift, anEditor);
      break;

      case "mtable":
        selInfo = mTableDoNextField(aNode, totalRange, bShift, anEditor);
      break;

      case "msqrt":
        selInfo = squareRootDoNextField(aNode, totalRange, bShift, anEditor);
      break;

      case "msidisplay":  //May add some stuff here sometime
      break;

      default:
        if (msiNavigationUtils.isMathTemplate(aNode))
        {
          if (bShift)
          {
            selInfo = new Object();
            if (totalRange.startContainer == aNode)
              selInfo.mNode = msiNavigationUtils.getSignificantChildPrecedingPosition(aNode, totalRange.startOffset, true);
            else
              selInfo.mNode = msiNavigationUtils.getSignificantChildPrecedingNode(aNode, totalRange.startContainer, true);
            selInfo.mOffset = selInfo.mNode ? msiNavigationUtils.lastOffset(selInfo.mNode) : 0;
          }
          else
          {
            selInfo = new Object();
            if (totalRange.endContainer == aNode)
              selInfo.mNode = msiNavigationUtils.getSignificantChildFollowingPosition(aNode, totalRange.endOffset, true);
            else
              selInfo.mNode = msiNavigationUtils.getSignificantChildFollowingNode(aNode, totalRange.endContainer, true);
            selInfo.mOffset = 0;  //In either case
          }
          selInfo.bInTransaction = false;
        }
      break;
    }

    if (selInfo)
    {
      try {
        if (!selInfo.bInTransaction)
          anEditor.beginTransaction();
        // avoid putting the cursor in a place that can't accept text
        var tlmanager = anEditor.tagListManager;
        var namespace = null;
        if (tlmanager.nodeCanContainTag(selInfo.mNode, '#text', namespace))
          anEditor.selection.collapse(selInfo.mNode, selInfo.mOffset);
        else {
          var treeWalker = anEditor.document.createTreeWalker(
            selInfo.mNode,
            NodeFilter.SHOW_ELEMENT,
            { acceptNode: function(node) { return NodeFilter.FILTER_ACCEPT; } },
            false
          );
          while(treeWalker.nextNode() && !tlmanager.nodeCanContainTag(treeWalker.currentNode, '#text', namespace));
          if (treeWalker.currentNode) anEditor.selection.collapse(treeWalker.currentNode, 0);
          // else ????
        }
        anEditor.endTransaction();
      }
      catch(e) {
        msidump(e.message);
      }
      return true;
    }
    return false;
  }

//  function objectTabFromSelection(aNode, bShift, anEditor)
//  {
//    if (!msiNavigationUtils.nodeHasContentBeforeSelection(anEditor.selection, aNode)
//        && !msiNavigationUtils.nodeHasContentAfterSelection(anEditor.selection, aNode) )
//      return false;
//
//    var wholeRange = msiNavigationUtils.getRangeContainingSelection(anEditor.selection);
//    var selNode = null;
//    var childNode = bShift ? wholeRange.startContainer : wholeRange.endContainer;
//    switch(msiGetBaseNodeName(aNode))
//    {
//      case "msub":
//      case "msup":
//      case "msubsup":
//      case "munder":
//      case "mover":
//      case "munderover":
//        selNode = doTabInScript(aNode, childNode, bShift, anEditor);
//      break;
//      case "msqrt":
//        selNode = doTabInSquareRoot(aNode, anEditor);
//      break;
//      case "mtable":
////        var childNode = null;
//        selNode = doTabInMTable(aNode, childNode, bShift, anEditor);
//      break;
//      case "table":
////        var childNode = null;
//        selNode = doTabInTable(aNode, childNode, bShift, anEditor);
//      break;
//      default:
//        if (msiNavigationUtils.isTemplate(aNode))
//        {
//          if (!bCollapsed)
//          {
//            if (wholeRange.endNode != aNode)
//              selNode = msiNavigationUtils.findTopChildContaining(wholeRange.endNode, aNode);
//            else
//              selNode = msiNavigationUtils.getSignificantChildPrecedingPosition(aNode, wholeRange.endOffset, true);
//          }
//          else if (bShift)
//          {
//            if (wholeRange.startNode != aNode)
//              selNode = msiNavigationUtils.getSignificantChildPrecedingNode(aNode, wholeRange.startNode, true);
//            else
//              selNode = msiNavigationUtils.getSignificantChildPrecedingPosition(aNode, wholeRange.startOffset, true);
//          }
//          else
//          {
//            if (wholeRange.endNode != aNode)
//              selNode = msiNavigationUtils.getSignificantChildFollowingNode(aNode, wholeRange.endNode, true);
//            else
//              selNode = msiNavigationUtils.getSignificantChildFollowingPosition(aNode, wholeRange.endOffset, true);
//          }
//        }
//      break;
//    }
//    if (selNode)
//    {
//      anEditor.selection.collapse(selNode, 0);
//      return true;
//    }
//    return false;
//  }
  var retVal = false;
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);

  var container = msiNavigationUtils.getCommonAncestorForSelection(editor.selection);
  var wholeRange = msiNavigationUtils.getRangeContainingSelection(editor.selection);
  for ( var containerParent = container; !retVal && (containerParent != null); containerParent = containerParent.parentNode )
  {
    retVal = nodeDoNextField(containerParent, wholeRange, bShift, editor);
  }
  return retVal;
}

//The sole purpose of this function is to block certain <enter>s from being processed, and hand them off instead to our parent (dialog?) window.
function msiEditorCheckEnter(event)
{
  var bEditorHandle = false;
  var editorElement = msiGetEditorElementFromEvent(event);
  var editor = null;
  if (editorElement)
    editor = msiGetEditor(editorElement);
  if (!editor || !msiEditorIsSinglePara(editorElement))
  {
//    if (editor)
//      dump("In msiEditorCheckEnter, we don't seem to have a single-para editor; editor's flags are [" + editor.flags + "].\n");
//    else
//      dump("In msiEditorCheckEnter, we don't find an editor!.\n");
    return false;  //In this case, we let nature take its course
  }

//  dump("In msiEditorCheckEnter, we have a single-para editor.\n");
  var container = msiNavigationUtils.getCommonAncestorForSelection(editor.selection);
  if (msiGetContainingTableOrMatrix(container))
    bEditorHandle = true;

  if (!bEditorHandle && msiNavigationUtils.nodeIsInMath(container))
  {
    //Here we have to be careful. If we're out in the open in math, it would be a normal paragraph entry, which we don't want.
    //If we're inside a math "template", we'll pass it on down.
    var foundSplittable = false;
    for (var parent = container; parent && !foundSplittable; parent = parent.parentNode)
    {
//      dump("In msiEditorCheckEnter, checking parent node [" + msiGetBaseNodeName(parent) + "].\n");
      if (msiNavigationUtils.isMathTemplate(parent))
      {
        foundSplittable = true;
      }
      else if (msiNavigationUtils.isFence(parent))
      {
        foundSplittable = true;
      }
      else if (msiGetBaseNodeName(parent) == "math")
      {
        foundSplittable = (msiGetBaseNodeName(parent.parentNode) == "msidisplay");
        break;
      }
      else
      {
        switch(msiGetBaseNodeName(parent))
        {
          case "mtd":
          //Presumably we don't want to return true for mtable rows or mtables?
          case "msqrt":
            foundSplittable = true;
          break;
        }
      }
    }
    dump("In msiEditorCheckEnter, we're in math, and we " + (foundSplittable ? "found" : "didn't find") + " a splittable parent.\n");
    if (foundSplittable)
      bEditorHandle = true;
  }
  if (bEditorHandle)
  {
    editor.insertReturn();
    return true;  //it's been handled
  }

  //So we're not in a math template or a table. We want to block this one from the editor.
  var dlg = window.document.documentElement;
//  dump("In msiEditorCheckEnter, bEditorHandle was false, and documentElement is [" + dlg.nodeName + "].\n");
  if (dlg.nodeName == "dialog")
  {
    dlg._hitEnter(event);
    return true;
  }
  return false;
}

function msiEditorDoTab(event)
{
  function doTabWithSelectionInNode(aNode, totalRange, anEditor)
  {
    var rv = false;
    switch(msiGetBaseNodeName(aNode))
    {
      case "table":
      case "tr":
      case "mtable":
      case "mtr":
        var aTableCell = msiFindCellFromPositionInTableOrMatrix(aNode, totalRange.endContainer, totalRange.endOffset, false, anEditor);
        editor.selection.collapse(aTableCell, 0);
        rv = true;
      break;

      default:
      break;
    }
    return rv;
  }

  var bHandled = false;
  var editorElement = msiGetEditorElementFromEvent(event);
  if (!editorElement)
    return false;

//    editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);
  var bShift = event.shiftKey;
  if (bShift && !editor.selection.isCollapsed)  //We don't do anything if there's a selection and the shift key is down
    return false;
  if (editor.selection.isCollapsed)
    bHandled = msiEditorNextField(bShift, editorElement);

  //Otherwise, we do have a selection.
  else
  {
    var container = msiNavigationUtils.getCommonAncestorForSelection(editor.selection);
    var wholeRange = msiNavigationUtils.getRangeContainingSelection(editor.selection);
  //  for ( var containerParent = container; !bHandled && (containerParent != null); containerParent = containerParent.parentNode )
  //  {
      bHandled = doTabWithSelectionInNode(container, wholeRange, bShift, editor);
  //  }
  }
  if (!bHandled)
  {
    var dlg = window.document.documentElement;
    if (dlg.nodeName == "dialog")
    {
      var commandDispatcher = window.document.commandDispatcher;
      if (bShift)
      {
        if ("msiTabBack" in window)
          msiTabBack(event);
        else
          commandDispatcher.rewindFocus();
      }
      else
      {
        if ("msiTabForward" in window)
          msiTabForward(event);
        else
          commandDispatcher.advanceFocus();
      }
      bHandled = true;
    }
  }
  if (!bHandled && !bShift) // none of the above code did anything. Put in a 2em space
  {
    editor.beginTransaction();
    if (!editor.selection.isCollapsed) editor.deleteSelection(editor.eNone);
    var wrapper = new Object();
    wrapper.spaceType="twoEmSpace";
    msiInsertHorizontalSpace(wrapper,editorElement);
    editor.endTransaction();
    bHandled = true;
  }

  return bHandled; //Actually want to return false if the event has been handled
}

//Only check in case we've brought up the context menu "artificially"; then we need to close it
function msiEditorCheckEscape(event)
{
  var contextMenu = document.getElementById("msiEditorContentContext");
  if (contextMenu.open)
  {
    contextMenu.hidePopup();
    return true;
  }
  return false;
}

//function msiGetCharForProperties(editorElement)
//{
//  var nodeData = msiGetObjectDataForProperties(editorElement);
//  if (nodeData != null && nodeData.theNode != null && nodeData.theOffset != null)
//  {
//    return nodeData;
//  }
//  return null;
//}

////Resolves whether the object at anIndex inside aNode is a character or a node.
//function msiPropertiesObjectData(aNode, anIndex)
//{
//  if (aNode == null)
//  {
//    dump("In msiPropertiesObjectData constructor, null node was passed in!\n");
//    return null;
//  }
//
//  if (anIndex != null)
//  {
//    if (aNode.nodeType == nsIDOMNode.TEXT_NODE)
//    {
//      //What do we return to say we want the character to the left?
//      this.theNode = aNode;
//      this.theOffset = anIndex;
//    }
//    else if (anIndex < aNode.childNodes.length)
//    {
//      this.theNode = aNode.childNodes[anIndex];
//      this.theOffset = null;
//    }
//    else  //trouble??
//    {
//      dump("Problem case in msiPropertiesObjectData - aNode is [" + aNode.nodeName + "], with length [" + aNode.childNodes.length + "], and anIndex is [" + anIndex + "].\n");
//      this.theNode = aNode;
//      this.theOffset = null;
//    }
//  }
//  else
//  {
//    this.theNode = aNode;
//    this.theOffset = null;
//  }
//}

function msiGetObjectDataForProperties(editorElement)
{
//  var editor = msiGetEditor(editorElement);
  var nodeData = msiGetPropertiesObjectFromSelection(editorElement);
  return nodeData;
}

/*TODO: We need an oncreate hook to do enabling/disabling for the
        Format menu. There should be code like this for the
        object-specific "Properties" item

*/
// For property dialogs, we want the selected element,
//  but will accept a parent link, list, or table cell if inside one
//function msiGetObjectForProperties(editorElement)
//{
//  if (!editorElement)
//    editorElement = msiGetActiveEditorElement();
//  var editor = msiGetEditor(editorElement);
//  if (!editor || !msiIsHTMLEditor(editorElement))
//    return null;
//  return msiEditorGetObjectForProperties(editor);
//}

function msiEditorGetObjectForProperties(editorElement)
{
  var element;
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);
  if (!editor || !msiIsHTMLEditor(editorElement))
    return null;
  try {
    element = editor.getSelectedElement("");
  } catch (e) {}
  if (element)
    return element;

  // Find nearest parent of selection anchor node
  //   that is a link, list, table cell, or table

  var nodeData = msiGetPropertiesObjectFromSelection(editorElement);
  if (nodeData != null && nodeData.theNode != null)
  {
    if (nodeData.theOffset == null)
      return nodeData.theNode;
    else
      return null; //Calls to this function are assumed to want some object or other to revise - those which can
                   //revise a character in text should go in through the msiGetCharForProperties(editorElement) method,
                   //or directly through the msiGetPropertiesObjectFromSelection function?
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

function msiCreatePropertiesObjectDataFromSelection(aSelection, editorElement)
{
  var retObj = null;
  var editor = msiGetEditor(editorElement);
  var container = msiNavigationUtils.getCommonAncestorForSelection(editor.selection);
//  var containerData = msiGetSelectionContainer(editorElement);
//  var container = containerData.node;
  var theRange = null;
  var theText = null;

  switch(msiGetBaseNodeName(container))
  {
    //Note that a container which is a cell in a table shouldn't be constructed here, as it's essentially no differrent from any other container.
    case "thead":
    case "tbody":
    case "tfoot":
    case "tr":
    case "td":
    case "mtable":
    case "mtr":
    case "mlabeledtr":
    case "mtd":
      var tableParent = msiGetContainingTableOrMatrix(container);
      if (msiNavigationUtils.isEquationArray(editorElement, tableParent))
      {
        retObj = new msiEquationPropertiesObjectData();
        retObj.initFromSelection(editor.selection, editorElement);
      }
      else
      {
        retObj = new msiTablePropertiesObjectData();
        retObj.initFromSelection(editor.selection, editorElement);
      }
    break;

    case "msidisplay":
      retObj = new msiEquationPropertiesObjectData();
      retObj.initFromSelection(editor.selection, editorElement);
    break;

    case '#text':
      retObj = new msiCharPropertiesObjectData();
      retObj.initFromSelection(editor.selection, editorElement);
    break;

    default:
    break;
  }
  if (!retObj && editor.selection.rangeCount == 1)
    retObj = msiCreatePropertiesObjectDataFromRange(editor.selection.getRangeAt(0), editorElement);

  return retObj;
}

function msiGetPropertiesObjectFromSelection(editorElement)
{
  var retObj = null;
  var editor = msiGetEditor(editorElement);
  if (editor.selection.isCollapsed)
    retObj = msiSelectPropertiesObjectFromCursor(editorElement);
  else
//rwa    {
    retObj = msiCreatePropertiesObjectDataFromSelection(editor.selection, editorElement);
//rwa      if (retObj)
//rwa        return retObj;
//rwa
//rwa      var theRange = editor.selection.getRangeAt(0);
//rwa      if (theRange == null)
//rwa      {
//rwa        theRange = editor.document.createRange();
//rwa        theRange.setStart(editor.selection.anchorNode, editor.selection.anchorOffset);
//rwa        theRange.setEnd(editor.selection.focusNode, editor.selection.focusOffset);
//rwa        if (theRange.collapsed)  //According to the W3C DOM Range interface specification, this will happen if anchor was after focus.
//rwa        {
//rwa          theRange.setStart(editor.selection.focusNode, editor.selection.focusOffset);
//rwa          theRange.setEnd(editor.selection.anchorNode, editor.selection.anchorOffset);
//rwa        }
//rwa      }
//rwa      if (theRange.collapsed)
//rwa        retObj = msiSelectPropertiesObjectFromCursor(editorElement);
//rwa      else
//rwa        retObj = msiSelectPropertiesObjectFromRange(editorElement, theRange);
//rwa    }
//rwa
//rwa    if ( (retObj != null) && (retObj.theOffset == null) )  //in the case we've identified a revisable object
//rwa    {
//rwa      var  parentNode = msiNavigationUtils.findWrappingStyleNode(retObj.theNode);
//rwa      if (parentNode != null)
//rwa        retObj.theNode = parentNode;
//rwa    }
//rwa
//rwa    return retObj;
//rwa    //We're trying to identify the situation where there's essentially (to be laboriously defined below) one object selected.
//rwa    //The plan is:
//rwa    //  (1) In the best situation, we'll find that anchorNode and focusNode are children of the same element, with a difference
//rwa    //        of one in their offsets. That node is then the revisable one.
//rwa    //  (2) If one of anchorNode/focusNode is a child of the common ancestor but the other isn't, we want to look for the situation
//rwa    //        where the non-direct child is essentially at the start/end of the node we're hoping for in (1). This should be handled
//rwa    //        carefully, but the idea is much like the msiFindRevisableObjectToLeft() function - we'd like to be able to move up
//rwa    //        to the parent but have to ensure the situation warrants it.
//rwa    //  (3) If neither is a direct child of commonAncestor, we try to move both positions up to the parents. The hope is that the
//rwa    //        positions passed in are just beyond the ends of an object, with nothing significant between; failing that, the hope is
//rwa    //        that they're just inside the ends of an object.
//rwa    //  Now write the code?
//rwa  //  var rangeContentList = msiNavigationUtils.getSignificantRangeContent(theRange);

  return retObj;
}

function msiCreatePropertiesObjectDataFromRange(aRange, editorElement)
{
  var retObj = null;
  var anOffset = null;
  var ancestorNode = aRange.commonAncestorContainer;
  var editor = msiGetEditor(editorElement);

  if (aRange.startContainer == aRange.endContainer)
  {
    if (aRange.endOffset - aRange.startOffset == 1)
    {
      if (aRange.startContainer.childNodes.length > aRange.startOffset)
        return msiCreatePropertiesObjectDataFromNode(aRange.startContainer.childNodes[aRange.startOffset], editorElement);
    }
    if (msiNavigationUtils.positionIsAtStart(aRange.startContainer, aRange.startOffset) && msiNavigationUtils.positionIsAtEnd(aRange.endContainer, aRange.endOffset))
    {
      return msiCreatePropertiesObjectDataFromNode(aRange.startContainer, editorElement);
    }
  }
  else
  {
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
        }
        else if ((trialSequenceStart[jx] == 2) && msiNavigationUtils.positionIsAtEnd(aRange.startContainer, aRange.startOffset) && msiNavigationUtils.boundaryIsTransparent(aRange.startContainer, editor, msiNavigationUtils.rightEndToRight))
        {
          bChanged = true;
          newRange.setStartAfter(aRange.startContainer);
        }
        if ((trialSequenceEnd[kx] == 1) && msiNavigationUtils.positionIsAtStart(aRange.endContainer, aRange.endOffset) && msiNavigationUtils.boundaryIsTransparent(aRange.endContainer, editor, msiNavigationUtils.leftEndToLeft))
        {
          bChanged = true;
          newRange.setEndBefore(aRange.endContainer);
        }
        else if ((trialSequenceEnd[kx] == 2) && msiNavigationUtils.positionIsAtEnd(aRange.endContainer, aRange.endOffset))
        {
          bChanged = true;
          newRange.setEndAfter(aRange.endContainer);
        }

        if (bChanged)
          retObj = msiCreatePropertiesObjectDataFromRange(newRange, editorElement);
//          retObj = msiSelectPropertiesObjectFromRange(editorElement, newRange);
      }

      if ((retObj != null))
        return retObj;
    }
  }

  //If we reach here, we didn't find anything appropriate. so just return null - or "retObj", which is the same thing.
  return retObj;
}


//function msiSelectPropertiesObjectFromRange(editorElement, aRange)
//{
//  var aNode = null;
//  var anOffset = null;
//  var editor = msiGetEditor(editorElement);
//
//  if (aRange.startContainer == aRange.endContainer)  //the simple case
//  {
//    //Note that we're not looking at a #text node - those should have been handled in the calling msiCreatePropertiesObjectDataFromSelection() function
//    if (aRange.endOffset - aRange.startOffset == 1)  //the simplest (hoped-for) case
//    {
//      //Want something else here - another function that returns one of our "properties objects" from a node and offset?
//      //Even in the simplest case, have to distinguish between selecting a character (parent is Text) and selecting a node.
//      return new msiPropertiesObjectData(aRange.startContainer, aRange.startOffset);
//    }
//    if (msiNavigationUtils.positionIsAtStart(aRange.startContainer, aRange.startOffset) && msiNavigationUtils.positionIsAtEnd(aRange.endContainer, aRange.endOffset))
//    {
//      return new msiPropertiesObjectData(aRange.startContainer, null);
//    }
//    return null;
//  }
//  else  //Can we move up the tree without essentially changing the range?
//  {
//    var retVal = null;
//    var ancestorNode = aRange.commonAncestorContainer;
//    var trialSequenceStart = [0];
//    if (aRange.startContainer != ancestorNode)
//      trialSequenceStart = [1,2];  //means try left first, then right
//    var trialSequenceEnd = [0];
//    if (aRange.endContainer != ancestorNode)
//      trialSequenceEnd = [2,1];  //means try right first, then left
//
//    for (var jx = 0; jx < trialSequenceStart.length; ++jx)
//    {
//      for (var kx = 0; kx < trialSequenceEnd.length; ++kx)
//      {
//        var bChanged = false;
//        var newRange = aRange.cloneRange();
//        if ((trialSequenceStart[jx] == 1) && msiNavigationUtils.positionIsAtStart(aRange.startContainer, aRange.startOffset))
//        {
//          bChanged = true;
//          newRange.setStartBefore(aRange.startContainer);
////          msiNavigationUtils.moveRangeStartOutOfObject(newRange, false);  //the "false" means "to left" as opposed to "to right"
//        }
//        else if ((trialSequenceStart[jx] == 2) && msiNavigationUtils.positionIsAtEnd(aRange.startContainer, aRange.startOffset) && msiNavigationUtils.boundaryIsTransparent(aRange.startContainer, editor, msiNavigationUtils.rightEndToRight))
////RWA-insert        else if ((trialSequenceStart[jx] == 2) && msiNavigationUtils.positionIsAtEnd(aRange.startContainer, aRange.startOffset) && msiNavigationUtils.boundaryIsTransparent(aRange.startContainer, editor, msiNavigationUtils.rightEndToRight))
//        {
//          bChanged = true;
//          newRange.setStartAfter(aRange.startContainer);
////          msiNavigationUtils.moveRangeStartOutOfObject(newRange, true);  //the "true" means "to right"
//        }
//        if ((trialSequenceEnd[kx] == 1) && msiNavigationUtils.positionIsAtStart(aRange.endContainer, aRange.endOffset) && msiNavigationUtils.boundaryIsTransparent(aRange.endContainer, editor, msiNavigationUtils.leftEndToLeft))
////RWA-insert        if ((trialSequenceEnd[kx] == 1) && msiNavigationUtils.positionIsAtStart(aRange.endContainer, aRange.endOffset) && msiNavigationUtils.boundaryIsTransparent(aRange.endContainer, editor, msiNavigationUtils.leftEndToLeft))
//        {
//          bChanged = true;
//          newRange.setEndBefore(aRange.endContainer);
////          msiNavigationUtils.moveRangeEndOutOfObject(newRange, false);  //the "false" means "to left" as opposed to "to right"
//        }
//        else if ((trialSequenceEnd[kx] == 2) && msiNavigationUtils.positionIsAtEnd(aRange.endContainer, aRange.endOffset))
//        {
//          bChanged = true;
//          newRange.setEndAfter(aRange.endContainer);
////          msiNavigationUtils.moveRangeEndOutOfObject(newRange, true);  //the "true" means "to right"
//        }
//
//        if (bChanged)
//          retVal = msiSelectPropertiesObjectFromRange(editorElement, newRange);
//      }
//
//      if ((retVal != null))
//      {
//        return retVal;
//        break;
//      }
//    }
//  }
//
//  return null;
//}

function msiSelectPropertiesObjectFromCursor(editorElement)
{
  var editor = msiGetEditor(editorElement);
  var currNode = editor.selection.anchorNode;
  var bindingParent = currNode ? currNode.ownerDocument.getBindingParent(currNode) : null;
  if (bindingParent)
  {
    var offset = bindingParent.childNodes ? bindingParent.childNodes.length : 0;
    return msiFindRevisableObjectToLeft(bindingParent, offset, editorElement);
  }

  //Look to the left of the cursor:
  return msiFindRevisableObjectToLeft(currNode, editor.selection.anchorOffset, editorElement);
}

function msiFindRevisableObjectToLeft(aNode, anOffset, editorElement)
{
//  var returnVal = new Object();
//  returnVal.theNode = null;
//  returnVal.theOffset = null;
  var editor = msiGetEditor(editorElement);
  var retObj = null;

  if (aNode.nodeType == nsIDOMNode.TEXT_NODE)
  {
    if (anOffset > 0)
    {
      if ( msiNavigationUtils.isMathname(aNode.parentNode) || msiNavigationUtils.isUnit(aNode.parentNode)
           || msiNavigationUtils.isBigOperator(aNode.parentNode) || msiNavigationUtils.isSpacingObject(aNode.parentNode)
           || msiNavigationUtils.isMathMLLeafNode(aNode.parentNode) )
      {
        retObj = msiCreatePropertiesObjectDataFromNode(aNode.parentNode, editorElement);
  //      returnVal.theNode = aNode.parentNode;
        return retObj;
      }

      //What do we return to say we want the character to the left?
      retObj = new msiCharPropertiesObjectData();
      retObj.initFromNodeAndOffset(aNode, anOffset, editorElement);
//      returnVal.theNode = aNode;
//      returnVal.theOffset = anOffset - 1;
      return retObj;
    }
  }

  aNode = msiFindRevisableNodeToLeft(aNode, anOffset, editor);
//  var nextNode = null;
////  if (anOffset >= aNode.childNodes.length)
//  if (msiNavigationUtils.positionIsAtEnd(aNode, anOffset))
//    nextNode = aNode;
//  for (var ix = anOffset; (nextNode == null) && (ix > 0); --ix)
//  {
//    //NOTE that we should never be in this clause if this is a text node, since unless anOffset == 0 we would have bailed out in the previous clause.
//    if ( !msiNavigationUtils.isIgnorableWhitespace(aNode.childNodes[ix-1]) )
//    {
//      nextNode = aNode.childNodes[ix-1];
//      break;
//    }
//  }
//  if (ix != 0 && nextNode == null)
//  {
//    dump("Unexpected result in msiFindObjectToLeft! Return null.\n");
//    return retObj;
//  }
//
//  //Now the only way that nextNode should be null is really if we're at the beginning of aNode:
////  while ((nextNode == null) && msiNavigationUtils.boundaryIsTransparent(aNode, editor, posAndDir))
//  while ( (nextNode == null) && msiNavigationUtils.positionIsAtStart(aNode, ix) )
//  {
//    if (msiNavigationUtils.boundaryIsTransparent(aNode, editor, msiNavigationUtils.leftEndToLeft))
//    //Move to left and try again...
//      nextNode = aNode.previousSibling;
//    else
//      return retObj;  //No reasonable object to revise here.
//    //Move to our parent since we're at his beginning,
//    if (nextNode == null)
//    {
//      var nextParent = aNode.parentNode;
////      ix = msiNavigationUtils.offsetInParent(aNode);  //Should we just say 0 since we found no previousSibling?
//      ix = 0;
//      aNode = nextParent;
//    }
//  }
//
//  if (nextNode != null)  //In this case we've identified an object to our left. We check whether we should descend into it:
//  {
//    aNode = nextNode;
////    nextNode = null;
//    while (nextNode != null && msiNavigationUtils.boundaryIsTransparent(nextNode, editor, msiNavigationUtils.rightEndToLeft))
//      nextNode = msiNavigationUtils.getLastSignificantChild(nextNode);
//    if (nextNode != null)
//      aNode = nextNode;
////      aNode = aNode.lastChild;
//  }
//
  //Finally, one last check on the validity of aNode. If it fails, there's nothing good to revise.
  if (msiNavigationUtils.cannotSelectNodeForProperties(aNode))
    return retObj;

//  returnVal.theNode = aNode;
  if (aNode.nodeType == nsIDOMNode.TEXT_NODE)
  {
//    returnVal.theOffset = aNode.length - 1;
    retObj = new msiCharPropertiesObjectData();
    retObj.initFromNodeAndOffset(aNode, aNode.length, editorElement);
  }
  else
    retObj = msiCreatePropertiesObjectDataFromNode(aNode, editorElement);
//    returnVal.theOffset = null;

  return retObj;
}

function msiFindRevisableNodeToLeft(aNode, anOffset, editor)
{
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
    return retObj;
  }

  //Now the only way that nextNode should be null is really if we're at the beginning of aNode:
//  while ((nextNode == null) && msiNavigationUtils.boundaryIsTransparent(aNode, editor, posAndDir))
  while ( (nextNode == null) && msiNavigationUtils.positionIsAtStart(aNode, ix) )
  {
    if (msiNavigationUtils.boundaryIsTransparent(aNode, editor, msiNavigationUtils.leftEndToLeft))
    //Move to left and try again...
      nextNode = aNode.previousSibling;
    else
      return nextNode;  //No reasonable object to revise here.
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
//    nextNode = null;
    while (nextNode != null && msiNavigationUtils.boundaryIsTransparent(nextNode, editor, msiNavigationUtils.rightEndToLeft))
      nextNode = msiNavigationUtils.getLastSignificantChild(nextNode);
    if (nextNode != null)
      aNode = nextNode;
//      aNode = aNode.lastChild;
  }
  return aNode;
}

/******Display Mode stuff - for the time being, only applicable to main editor window, but leave the functions here anyway******/

function handleSourceParseError(errorMsg) // returns true if the user wants to go back to editing
{
  try
  {
    var editorElement = msiGetActiveEditorElement();
    var parentWindow = msiGetWindowContainingEditor(editorElement);
    var data = {msg: errorMsg,
                result: false };
    parentWindow.openDialog( "chrome://prince/content/sourceparseerror.xul",
                             "parseerror",
                             "chrome,resizable,titlebar,modal",
                             data);
    //BBM: somehow returning data.result returned a string.
    if (data.result) return true;
    return false;
  }
  catch(exc) {AlertWithTitle("Error in msiEditor.js", "In msiEditorNewPlaintext(), failed to open; exception: " + exc);}
  return false;
}

function msiClearSource(editorElement)
{
  var editor = msiGetEditor(editorElement);
  var sourceIframe = document.getElementById("content-source");
  var sourceEditor = sourceIframe.contentWindow.gEditor;
  if (sourceEditor)
  {
    sourceEditor.clearHistory();
    sourceEditor.setValue("");
  }
}

function displayDocTypeIfExists(editor)
{
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
}

var gSelectionStartNode;
var gSelectionEndNode;
var gSelectionStartData;
var gSelectionEndData;
const kSW = "--SW--";

function MarkSelection(editor)
{
  gSelectionStartNode = null;
  gSelectionEndNode = null;
  gSelectionStartData = "";
  gelectionEndData = "";

  var selection = editor.selection;
  for (var count = 0; count < 1; count++) {
    var range = selection.getRangeAt(count);
    var startContainer = range.startContainer;
    var endContainer   = range.endContainer;
    var startOffset    = range.startOffset;
    var endOffset      = range.endOffset;

    if (startContainer.nodeType == Node.TEXT_NODE) {
      var data = startContainer.data;
      gSelectionStartNode = startContainer;
      gSelectionStartData = data;
      data = data.substr(0, startOffset) + kSW + data.substr(startOffset);
      startContainer.data = data;
    }
    else if (startContainer.nodeType == Node.ELEMENT_NODE) {
      if (startOffset < startContainer.childNodes.length) {
        var node = startContainer.childNodes.item(startOffset);
        if (node.nodeType == Node.TEXT_NODE) {
          var data = node.data;
          gSelectionStartNode = node;
          gSelectionStartData = data;
          data = kSW + data;
          node.data = data;
        }
        else {
          var t = editor.document.createTextNode(kSW);
          gSelectionStartNode = t;
          startContainer.insertBefore(t, node);
        }
      }
      else {
        var t = editor.document.createTextNode(kSW);
        gSelectionStartNode = t;
        startContainer.appendChild(t);
      }
    }

    if (endContainer.nodeType == Node.TEXT_NODE) {
      // same node as start node???
      if (endContainer == startContainer) {
        var data = endContainer.data;
        gSelectionEndNode = endContainer;
        gSelectionEndData = data;
        data = data.substr(0, endOffset + kSW.length) + kSW + data.substr(endOffset + kSW.length);
        endContainer.data = data;
      }
      else {
        var data = endContainer.data;
        gSelectionEndNode = endContainer;
        gSelectionEndData = data;
        data = data.substr(0, endOffset) + kSW + data.substr(endOffset);
        endContainer.data = data;
      }
    }
    else if (endContainer.nodeType == Node.ELEMENT_NODE) {
      var node = endContainer.childNodes.item(Math.max(0, endOffset - 1));
      if (node.nodeType == Node.TEXT_NODE) {
        var data = node.data;
        gSelectionEndNode = node;
        gSelectionEndData = data;
        data += kSW;
        node.data = data;
      }
      else {
        var t = editor.document.createTextNode(kSW);
        gSelectionEndNode = t;
        endContainer.insertBefore(t, node.nextSibling);
      }
    }
  }
}


function UnmarkSelection(editor)
{
  if (gSelectionEndNode) {
    if (gSelectionEndData)
      gSelectionEndNode.data = gSelectionEndData;
    else
      gSelectionEndNode.parentNode.removeChild(gSelectionEndNode);
  }

  if (gSelectionStartNode) {
    if (gSelectionStartData)
      gSelectionStartNode.data = gSelectionStartData;
    else if (gSelectionStartNode.parentNode) // if not already removed....
      gSelectionStartNode.parentNode.removeChild(gSelectionStartNode);
  }
}

function MarkSelectionInCM(aSourceEditor)
{
  aSourceEditor.setSelection( { line: 0, ch: 0 }, { line: 0, ch: 0 } );

  var searchCursor = aSourceEditor.getSearchCursor(kSW, { line: 0, ch: 0 }, true);
  searchCursor.findNext();
  var startRow    = searchCursor.from().line;
  var startColumn = searchCursor.from().ch;
  searchCursor.replace("");

  searchCursor = aSourceEditor.getSearchCursor(kSW, { line: 0, ch: 0 }, true);
  searchCursor.findNext();
  var endRow      = searchCursor.from().line;
  var endColumn   = searchCursor.from().ch;
  searchCursor.replace("");

  aSourceEditor.clearHistory();
  aSourceEditor.setSelection( { line: startRow, ch: startColumn }, { line: endRow, ch: endColumn } );
}

function CloneElementContents(editor, sourceElt, destElt)
{
  editor.cloneAttributes(destElt, sourceElt);
  var lastChild = destElt.lastChild;
  if (!lastChild || lastChild.nodeName.toLowerCase() != "br") {
    lastChild = editor.document.createElement("br");
    lastChild.setAttribute("type", "_moz");
    editor.insertNode(lastChild, destElt, destElt.childNodes.length);
  }

  var sourceChild = sourceElt.firstChild;
  while (sourceChild) {
    if (sourceChild.nodeType == Node.ELEMENT_NODE) {
      var destChild = editor.document.importNode(sourceChild, true);
      editor.insertNode(destChild, destElt, destElt.childNodes.length);
    }
    else if (sourceChild.nodeType == Node.TEXT_NODE) {
      t = editor.document.createTextNode(sourceChild.data);
      editor.insertNode(t, destElt, destElt.childNodes.length);
    }
    else if (sourceChild.nodeType == Node.COMMENT_NODE) {
      t = editor.document.createComment(sourceChild.data);
      editor.insertNode(t, destElt, destElt.childNodes.length);
    }

    sourceChild = sourceChild.nextSibling;
  }

  var child = destElt.firstChild;
  do {
    var stopIt = (child == lastChild);
    editor.deleteNode(child);
    child = destElt.firstChild;
  } while (!stopIt);
}


function RebuildFromSource(aDoc, editorElement, aContext)
{
  if (aContext)
    delete aContext;
  var editor = msiGetEditor(editorElement);
  try {

    // make sure everything is aggregated under one single txn
    editor.beginTransaction();
    // clone html attributes
    editor.cloneAttributes(editor.document.documentElement, aDoc.documentElement);
    // clone head
    CloneElementContents(editor, aDoc.getElementsByTagName("head").item(0), editor.document.getElementsByTagName("head").item(0));
    // clone body
    CloneElementContents(editor, aDoc.getElementsByTagName("body").item(0), editor.document.getElementsByTagName("body").item(0));
    editor.endTransaction();

    // the window title is updated by DOMTitleChanged event
  } catch(ex) {
  }
//  NotifierUtils.notify("afterLeavingSourceMode");
  window.content.focus();
  editorElement.focus();
}

//function escaped (source) {
//  return source.replace(/>/g,'&gt;').replace(/</g, '&lt;').replace(/"/g,'&quot;').replace(/'/g,'&apos;');
//}

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
  var sourceIframe = document.getElementById("content-source");
  var sourceEditor = sourceIframe.contentWindow.gEditor;

  // Switch the UI mode before inserting contents
  //   so user can't type in source window while new window is being filled
  var previousMode = msiGetEditorDisplayMode(editorElement);

  if (!msiSetDisplayMode(editorElement, mode))
    return;
  if (mode == kDisplayModeSource)
  {
    // Display the DOCTYPE as a non-editable string above edit area, if it exists
    displayDocTypeIfExists(editor);
    // Get the entire document's source string
    MarkSelection(editor);
    var source = prettyprint(editor);
    // const nsIDE = Components.interfaces.nsIDocumentEncoder;
    // var encoder = Components.classes["@mozilla.org/layout/documentEncoder;1?type=" + mimeType]
    //                .createInstance(nsIDE);
    // var source = encoder.encodeToString();
    // source = escaped(source);
    UnmarkSelection(editor);
    // replace the following with proper selection tracking
    var start = 0;
    sourceIframe.contentWindow.gChangeCallback = null; //onSourceChangeCallback;

    var theme = "neat";
    // try {
    //   theme = GetPrefs().getCharPref("SWP.source.theme");
    // }
    // catch(e) {}
    var tagManager = editor.tagListManager;
    var tagsArray = [].concat(
      tagManager.getTagsInClass('hidden',',',false).split(','),
      tagManager.getTagsInClass('texttag',',',false).split(','),
      tagManager.getTagsInClass('paratag',',',false).split(','),
      tagManager.getTagsInClass('listparenttag',',',false).split(','),
      tagManager.getTagsInClass('listtag',',',false).split(','),
      tagManager.getTagsInClass('structtag',',',false).split(','),
      tagManager.getTagsInClass('envtag',',',false).split(','),
      tagManager.getTagsInClass('frontmtag',',',false).split(','));

    sourceIframe.contentWindow./*wrappedJSObject.*/installCodeMirror(onBrowserKeyDown,
                                                 theme,
                                                 tagsArray,
                                                 null);
    //Ensure a line end at the end
    var lastEditableChild = editor.document.body.lastChild;
    if (lastEditableChild.nodeType == Node.TEXT_NODE)
      lastEditableChild.data = lastEditableChild.data.replace( /\s*$/, "\n");
    sourceEditor.setValue(source.replace( /\r\n/g, "\n").replace( /\r/g, "\n"));
    sourceIframe.focus();
    sourceEditor.refresh();
    sourceEditor.focus();
    MarkSelectionInCM(sourceEditor);
    sourceIframe.setUserData("oldSource", sourceEditor.getValue(), null);
  }
  else if (previousMode == kDisplayModeSource)
  {
    // Only rebuild document if a change was made in source window and licensed
    if (isLicensed()) {
      var historyCount = sourceEditor.historySize();
      if (historyCount.undo > 0 )
      {
  //   Reduce the undo count so we don't use too much memory
  //   during multiple uses of source window
  //   (reinserting entire doc caches all nodes)
  //      try {
  //      editor.transactionManager.maxTransactionCount = 1;
  //      } catch (e) {}
  //
        var errMsg="";
        var willReturn = false;
        source = sourceEditor.getValue();
        //source = decodeEntities(source);
        var xmlParser = new DOMParser();
        try {
          var doc = xmlParser.parseFromString(source, "text/xml");
          if (doc.documentElement.nodeName == "parsererror") {
            var errMsg = doc.documentElement.firstChild.textContent;
            var ptrLine = doc.documentElement.lastChild.textContent;
            errMsg += "\n"+ptrLine;;
            willReturn = handleSourceParseError(errMsg);
            if (willReturn)
            {
               msiSetDisplayMode(editorElement, kDisplayModeSource);
              return;
            }
          }
          else {
            RebuildFromSource(doc, editorElement);
          }
        }
        catch (e) {
        }
          // Get the text for the <title> from the newly-parsed document
          // (must do this for proper conversion of "escaped" characters)
        var title = "";
        var preambles = editor.document.getElementsByTagName("preamble");
        if (preambles.length > 0)
        {
          var titlenodelist =  preambles[0].getElementsByTagName("title");
          if (titlenodelist.length > 0)
          {
            var titleNode = titlenodelist.item(0);
            if (titleNode)
              title = titleNode.textContent;
          }
        }
        if (editor.document.title != title && ("msiUpdateWindowTitle" in window))
        {
          editor.document.title = title;
          msiUpdateWindowTitle();
        }
      }
      editorElement.makeEditable("html");
    }
 
    // Clear out the string buffers
    msiClearSource(editorElement);
    editorElement.contentWindow.focus();
  }
  else editorElement.contentWindow.focus();
  msiSetAllTagsMode(editor, mode == kDisplayModeAllTags)
}


function msiCancelHTMLSource(editorElement)
{
  // Don't convert source text back into the DOM document
  msiClearSource(editorElement);
  editorElement.makeEditable("html");
  editorElement.contentWindow.focus();
//  try
//  {
//    var sourceTextEditor = msiGetSourceTextEditor(editorElement);
//    if (sourceTextEditor)
//    {
//      sourceTextEditor.resetModificationCount();
//      msiSetDisplayMode(editorElement, msiGetPreviousNonSourceDisplayMode(editorElement));
//    }
//  } catch(e) {}
}

function msiFinishHTMLSource(editorElement)
{
  if (msiIsInHTMLSourceMode(editorElement))
  {
    // Switch edit modes -- converts source back into DOM document
    msiSetEditMode(msiGetPreviousNonSourceDisplayMode(editorElement), editorElement);
  }
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

function msiSetAllTagsMode(editor, on)
{
  editor.QueryInterface(nsIEditorStyleSheets);
  editor.enableStyleSheet(dynAllTagsStyleSheet, on);
  editor instanceof Components.interfaces.nsIHTMLObjectResizer;
  if (editor.resizedObject)
  {
    editor.hideResizers();
  }
}

function msiSetDisplayMode(editorElement, mode)
{
  if (!msiIsHTMLEditor(editorElement))
    return false;
  var editor = msiGetEditor(editorElement);

  //  Already in requested mode:
  //  return false to indicate we didn't switch
  var previousMode = msiGetEditorDisplayMode(editorElement);
  if (mode == previousMode)
    return false;

  var prefs = GetPrefs();
  var pdfAction = prefs.getCharPref("swp.prefPDFPath");
  if (pdfAction == "default" || mode != kDisplayModePreview)
  {
    editorElement.mEditorDisplayMode = mode;
  }
  if (("gEditorDisplayMode" in window) && editorElement.contentWindow == window.content)
    window.gEditorDisplayMode = mode;
  msiResetStructToolbar(editorElement);

  if ((mode == kDisplayModeSource)||(mode == kDisplayModePreview))
  {
    if (mode ==  kDisplayModePreview)
    {
      if (editorElement.pdfModCount != editor.getModificationCount() || pdfAction != "default")
      {
        dump("Document changed, recompiling\n");
        printTeX(true, true);
      }
      else
      {
        var docUrlString = msiGetEditorURL(editorElement);
        var url = msiURIFromString(docUrlString);
        var pdffile = msiFileFromFileURL(url);
        pdffile = pdffile.parent; // and now it points to the working directory
        pdffile.append("tex");
        pdffile.append(currPDFfileLeaf);
        dump("Trying to display current PDF file "+pdffile.path+"\n");
        if (pdffile.exists())
        {
          dump("Displaying PDF file\n");
          document.getElementById("preview-frame").loadURI(msiFileURLStringFromFile(pdffile));
        }
        else
        {
          dump("PDF file not found\n");
          printTeX(true, true);
        }
      }
      if (pdfAction != "default")
      {
        if ("gContentWindowDeck" in window)
        {
          if (previousMode < 0) previousMode = 0;
          window.gContentWindowDeck.selectedIndex = previousMode;
          document.getElementById("EditModeTabs").selectedIndex = previousMode;
        }

        return false;
      }
    }
    //Hide the formatting toolbar if not already hidden
    if ("gViewFormatToolbar" in window && window.gViewFormatToolbar != null)
      window.gViewFormatToolbar.hidden = true;
    if ("gComputeToolbar" in window && window.gComputeToolbar != null)
      window.gComputeToolbar.hidden = true;

//    msiHideItem("MSIMathMenu");
//    msiHideItem("cmd_viewComputeToolbar");
//    msiHideItem("MSIComputeMenu");
//    msiHideItem("MSITypesetMenu");
//    msiHideItem("MSIInsertTypesetObjectMenu");
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
  {
    document.getElementById(kDisplayModeMenuIDs[previousMode]).setAttribute("checked","false");
  }
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

function removeFootnoteOverrides(root)
{
  var notelist = root.getElementsByTagName("note");
  var len = notelist.length;
  var elem;
  for (var i = 0; i < len; i++)
  {
    elem = notelist[i];
    if (elem.getAttribute("type") == "footnote") elem.removeAttribute("hide");
  }
}

function msiEditorGetShowInvisibles(editorElement)  // returns viewSettings
// Gets view settings from body element attributes
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);
  var viewSettings = {};
  var theBody = msiGetRealBodyElement(editor.document);
  theBody.getAttribute("showinvis") === "true"
  
  viewSettings.showInvisibles = theBody.getAttribute("showinvis")==="true";
  viewSettings.showSectionExpanders = theBody.getAttribute("showexpanders") ==="true";
  viewSettings.showShortTitles = theBody.getAttribute("showshort") === "true";
  // viewSettings.showFMButtons = theBody.getAttribute("showfmbuttons") === "true";
  viewSettings.showHelperLines = theBody.getAttribute("hideHelperLines") !== "true";
  viewSettings.showInputBoxes = theBody.getAttribute("hideInputBoxes") !== "true";
  viewSettings.showIndexEntries = theBody.getAttribute("hideindexentries") !== "true";
  viewSettings.showMarkers = theBody.getAttribute("hidemarkers") !== "true";
  viewSettings.showFootnotes = theBody.getAttribute("hideFootnotes") !== "true";
  viewSettings.showOtherNotes = theBody.getAttribute("hideOtherNotes") !== "true";
  theBody.setAttribute("-moz_dirty","true");
  return viewSettings;
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
  if (viewSettings.showSectionExpanders)
    theBody.setAttribute("showexpanders", "true");
  else
    theBody.removeAttribute("showexpanders");
  if (viewSettings.showShortTitles)
    theBody.setAttribute("showshort", "true");
  else
    theBody.removeAttribute("showshort");
  // if (viewSettings.showFMButtons)
  //   theBody.setAttribute("showfmbuttons", "true");
  // else
  //   theBody.removeAttribute("showfmbuttons");
  if (!viewSettings.showHelperLines)
    theBody.setAttribute("hideHelperLines", "true");
  else
    theBody.removeAttribute("hideHelperLines");
  if (!viewSettings.showInputBoxes)
    theBody.setAttribute("hideInputBoxes", "true");
  else
    theBody.removeAttribute("hideInputBoxes");
  if (!viewSettings.showIndexEntries)
    theBody.setAttribute("hideindexentries", "true");
  else
    theBody.removeAttribute("hideindexentries");
  if (!viewSettings.showMarkers)
    theBody.setAttribute("hidemarkers", "true");
  else
    theBody.removeAttribute("hidemarkers");
  if (!viewSettings.showFootnotes)
  {
    removeFootnoteOverrides(theBody);
    theBody.setAttribute("hideFootnotes", "true");
  }
  else
  {
    removeFootnoteOverrides(theBody);
    theBody.removeAttribute("hideFootnotes");
  }
  if (!viewSettings.showOtherNotes)
    theBody.setAttribute("hideOtherNotes", "true");
  else
    theBody.removeAttribute("hideOtherNotes");

  //  var dumpStr = "Element [" + theBody.nodeName + "] now has settings: [";
  //  var attribNames = ["showinvis", "hideHelperLines", "hideInputBoxes", "hideindexentries", "hidemarkers"];
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
  if (theBody.hasAttribute("hideindexentries"))
    retVal.showIndexEntries = (theBody.getAttribute("hideindexentries")!="true");
  if (theBody.hasAttribute("hidemarkers"))
    retVal.showMarkers = (theBody.getAttribute("hidemarkers")!="true");

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

function msiEditorInitViewMenu()
{
  var i;
  var logcommands = ['cmd_showTeXLog', msiShowTeXLogCommand,'cmd_showBibTeXLog', msiShowBibTeXLogCommand,'cmd_showTeXFile',msiShowTeXFileCommand];
  for (i = 0; i < logcommands.length; i+=2) {
    if (logcommands[i+1].isCommandEnabled(null, null)) {
      document.getElementById(logcommands[i]).removeAttribute('disabled');
    }
    else
      document.getElementById(logcommands[i]).setAttribute('disabled', 'true');
  }
}

function msiEditorInitFormatMenu(event, theMenu)
{
  var bRightOne = (event.target == theMenu) || ( (event.target.nodeName == "menupopup") && (event.target.parentNode == theMenu) );
  if (!bRightOne)
    return;

  var editorElement = msiGetActiveEditorElement();
  try {
    msiInitObjectPropertiesMenuitem(editorElement, "propertiesMenu");
    msiInitRemoveStylesMenuitems(editorElement, "removeStylesMenuitem", "removeLinksMenuitem", "removeNamedAnchorsMenuitem");
  } catch(ex) {dump("Exception in msiEditor.js, msiEditorInitFormatMenu: [" + ex + "].\n");}
}

//function getObjectPropertiesDataFromNodeData(editorElement, element, bIncludeParaAndStructure)
function msiCreatePropertiesObjectDataFromNode(element, editorElement, bIncludeParaAndStructure)
{
  var objStr = null;
  var commandStr = null;
  var scriptStr = null;
  var coreElement = null;
  var fixedName = null;
  var theMenuStr = null;
  var propsData = null;
  var containingNodeData = null;
  var bindingParent = null;
  var tagclass;

  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);
//  var element = null;
//  if (nodeData)
//    element = nodeData.theNode;

  if (!element.ownerDocument)
    return null;
  if (bindingParent = element.ownerDocument.getBindingParent(element))
    return msiCreatePropertiesObjectDataFromNode(bindingParent, editorElement, bIncludeParaAndStructure);
  if (element && element.nodeName)
  {
    var name = msiGetBaseNodeName(element);
//    if (name != null)
//      name = name.toLowerCase();

    var wrappedChildElement = element;
    while ( (name == 'mstyle') || (name == 'mrow') || (name == "notewrapper") || (name == "msiframe") )
    {
      var newChildElement = msiNavigationUtils.getSingleWrappedChild(wrappedChildElement);
      if (newChildElement == null)
        break;
      wrappedChildElement = newChildElement;
      name = msiGetBaseNodeName(wrappedChildElement).toLowerCase();
    }

    coreElement = wrappedChildElement;

    switch (name)
    {
      case "#text":
      break;

      case "msiframe":
        if (!element.hasAttribute("frametype") || (element.getAttribute("frametype") != "image"))
        {
          objStr = name;
          theMenuStr = GetString("TagPropertiesMenuLabel");
          theMenuStr = theMenuStr.replace(/%tagname%/, GetString("msiFrame"));
          scriptStr = "msiFrame(event.target.refEditor, null, event.target.refElement);";
          break;
        }
        //otherwise fallthrough
      case "object":
        if (element.getAttribute("msigraph") == "true")
        {
          objStr = name;
          theMenuStr = GetString("TagPropertiesMenuLabel");
          theMenuStr = theMenuStr.replace(/%tagname%/, GetString("functiongraph"));
          scriptStr = "openGraphDialog('graph', event.target.refElement, event.target.refEditor);";
          break;
        }
        //otherwise fallthrough
      case "img":
      case "embed":
        // Check if img is enclosed in link
        //  (use "href" to not be fooled by named anchor)
//        try
//        {
//          if (editor.getElementOrParentByTagName("href", element))
//            objStr = GetString("ImageAndLink");
//        } catch(e) {}

        if (!objStr || !objStr.length)
        {
          if (coreElement.getAttribute("isVideo") == "true")
          {
            objStr = GetString("Video");
            commandStr = "cmd_reviseVideo";
          }
          else
          {
            objStr = GetString("Image");
            commandStr = "cmd_reviseImage";
          }
        }
        if ((msiGetBaseNodeName(coreElement.parentNode) == "msiframe") && (coreElement.parentNode.getAttribute("frametype") == "image"))
          coreElement = coreElement.parentNode;
        break;
      case "hr":
        objStr = GetString("HLine");
        commandStr = "cmd_reviseLine";
        break;

      case "table":
       objStr = GetString("Table");
//        scriptStr = "msiEditorInsertOrEditTable(false, editorElement, 'cmd_objectProperties', this)";
       commandStr = "cmd_editTable";
       break;
      case "th":
//        name = "td";
      case "td":
//        objStr = GetString("TableCell");
////        scriptStr = "msiEditorTableCellProperties(editorElement)";
//        commandStr = "cmd_editTable";
//        break;
      case "thead":
      case "tbody":
      case "tfoot":
      case "tr":
      case "td":
      case "mtable":
      case "mtr":
      case "mlabeledtr":
      case "mtd":
        var tableParent = msiGetContainingTableOrMatrix(coreElement);
        if (msiNavigationUtils.isEquationArray(editorElement, tableParent))
        {
          propsData = new msiEquationPropertiesObjectData();
          propsData.initFromNode(coreElement, editorElement);
        }
        else
        {
          propsData = new msiTablePropertiesObjectData();
          propsData.initFromNode(coreElement, editorElement);
        }
      break;

      case "ol":
      case "ul":
      case "dl":
        objStr = GetString("List");
        commandStr = "cmd_listProperties";
//        scriptStr =   "msiGoDoCommand('cmd_listProperties', editorElement)";
        break;
      case "li":
        objStr = GetString("ListItem");
        commandStr = "cmd_listProperties";
//        scriptStr =   "msiGoDoCommand('cmd_listProperties', editorElement)";
        break;
      case "form":
        objStr = GetString("Form");
        commandStr = "cmd_reviseForm";
        break;
      case "input":
        var type = element.getAttribute("type");
        if (type && type.toLowerCase() == "image")
        {
          objStr = GetString("InputImage");
          scriptStr = "msiGoDoCommand('cmd_inputimage', editorElement)";
        }
        else
        {
          objStr = GetString("InputTag");
          scriptStr = "msiGoDoCommand('cmd_inputtag', editorElement)";
        }
        break;
      case "textarea":
        objStr = GetString("TextArea");
        commandStr = "cmd_reviseTextarea";
        break;
      case "select":
        objStr = GetString("Select");
        break;
      case "button":
        objStr = GetString("Button");
        commandStr = "cmd_reviseButton";
        break;
//      case "label":
//        objStr = GetString("Label");
//        commandStr = "cmd_reviseLabel";
//        break;
      case "fieldset":
        objStr = GetString("FieldSet");
        commandStr = "cmd_reviseFieldset";
        break;
      case "a":
        if (element.name)
        {
          objStr = GetString("NamedAnchor");
          name = "anchor";
          commandStr = "cmd_reviseAnchor";
        }
          else if(wrappedChildElement.href)
        {
          objStr = GetString("Link");
          name = "href";
          commandStr = "cmd_msiReviseHyperlink";
        }
        break;

      case 'hspace':
        objStr = GetString("HorizontalSpace");
        commandStr = "cmd_reviseHorizontalSpaces";
      break;

      case 'vspace':
        objStr = GetString("VerticalSpace");
        commandStr = "cmd_reviseVerticalSpaces";
      break;

      case 'msirule':
        objStr = GetString("Rule");
        commandStr = "cmd_msiReviseRules";
      break;

      case 'msibr':
        objStr = GetString("GenBreak");
        commandStr = "cmd_msiReviseBreaks";
      break;

      case 'mfrac':
        objStr = GetString("Fraction");
        commandStr = "cmd_MSIreviseFractionCmd";
      break;

      case 'mroot':
      case 'msqrt':
        objStr = GetString("Radical");
        commandStr = "cmd_MSIreviseRadicalCmd";
      break;

      case 'msub':
      case 'msup':
      case 'msubsup':
        var childOp = msiNavigationUtils.getEmbellishedOperator(wrappedChildElement);
        if (childOp != null && msiNavigationUtils.isBigOperator(childOp))
        {
          objStr = GetString("Operator");
          commandStr = "cmd_MSIreviseOperatorsCmd";
        }
//        msiGoDoCommandParams("cmd_MSIreviseScriptsCmd", cmdParams, editorElement);
// Should be no Properties dialog available for these cases? SWP has none...
      break;

      case 'mover':
      case 'munder':
      case 'munderover':
        var childOp = msiNavigationUtils.getEmbellishedOperator(wrappedChildElement);
        if (childOp != null && msiNavigationUtils.isBigOperator(childOp))
        {
          objStr = GetString("Operator");
          commandStr = "cmd_MSIreviseOperatorsCmd";
        }
        else if (containingNodeData = msiNavigationUtils.getTopMathNodeAsAccentedCharacter(wrappedChildElement))
        {
          objStr = GetString("Character");
          commandStr = "cmd_reviseChars";
          coreElement = containingNodeData.mNode;
        }
        else
        {
          objStr = GetString("Decoration");
          commandStr = "cmd_MSIreviseDecorationsCmd";
        }
      break;

      case 'menclose':
        objStr = GetString("Decoration");
        commandStr = "cmd_MSIreviseDecorationsCmd";
      break;

      case 'mmultiscripts':
        var childOp = msiNavigationUtils.getEmbellishedOperator(wrappedChildElement);
        if (childOp != null && msiNavigationUtils.isBigOperator(childOp))
        {
          objStr = GetString("Operator");
          commandStr = "cmd_MSIreviseOperatorsCmd";
        }
        else
        {
          objStr = GetString("Tensor");
          commandStr = "cmd_MSIreviseTensorCmd";
        }
      break;

//      case 'mtable':
//        objStr = GetString("Matrix");
//        commandStr = "cmd_MSIreviseMatrixCmd";
//      break;

      case 'mi':
        if (msiNavigationUtils.isUnit(wrappedChildElement))
        {
          objStr = GetString("Unit");
          commandStr = "cmd_MSIreviseUnitsCommand";
        }
        else if (msiNavigationUtils.isMathname(wrappedChildElement))
        {
          objStr = GetString("MathName");
          commandStr = "cmd_MSIreviseMathnameCmd";
        }
        else if (containingNodeData = msiNavigationUtils.getTopMathNodeAsAccentedCharacter(wrappedChildElement))
        {
          objStr = GetString("Character");
          commandStr = "cmd_reviseChars";
          coreElement = containingNodeData.mNode;
        }
      break;

//  commandTable.registerCommand("cmd_MSIreviseSymbolCmd",    msiReviseSymbolCmd);

      case 'mrow':
      case 'mstyle':
        if (msiNavigationUtils.isFence(wrappedChildElement))
        {
          if (msiNavigationUtils.isBinomial(wrappedChildElement))
          {
            objStr = GetString("Binomial");
            commandStr = "cmd_MSIreviseBinomialsCmd";
          }
          else
          {
            objStr = GetString("GenBracket");
            commandStr = "cmd_MSIreviseGenBracketsCmd";
          }
        }
      break;

      case 'mo':
        if (msiNavigationUtils.isMathname(wrappedChildElement))
        {
          objStr = GetString("MathName");
          commandStr = "cmd_MSIreviseMathnameCmd";
        }
        else if (msiNavigationUtils.isBigOperator(wrappedChildElement))
        {
          objStr = GetString("Operator");
          commandStr = "cmd_MSIreviseOperatorsCmd";
        }
      break;

      case 'msidisplay':
        propsData = new msiEquationPropertiesObjectData();
        propsData.initFromNode(coreElement, editorElement);
      break;

      case 'bibitem':
        objStr = GetString("BibEntry");
        commandStr = "cmd_reviseManualBibItemCmd";
      break;

      case "bibtexbibliography":
        objStr = GetString("BibTeXBibliography");
        commandStr = "cmd_reviseBibTeXBibliographyCmd";
      break;

      case "citation":
        objStr = GetString("Citation");
        commandStr = "cmd_reviseCitation";
      break;

      case "xref":
        objStr = GetString("CrossRef");
        commandStr = "cmd_reviseCrossRef";
      break;

      case "graph":
        objStr = name;
        theMenuStr = GetString("TagPropertiesMenuLabel");
        theMenuStr = theMenuStr.replace(/%tagname%/, GetString("functiongraph"));
        scriptStr = "openGraphDialog('graph', event.target.refElement, event.target.refEditor);";
      break;

      case "otfont":
        objStr = name;
        theMenuStr = GetString("TagPropertiesMenuLabel");
        theMenuStr = theMenuStr.replace(/%tagname%/, GetString("opentypefont"));
        scriptStr = "openOTFontDialog('otfont', event.target.refElement);";
      break;

      case "rawTeX":
        objStr = name;
        theMenuStr = GetString("TagPropertiesMenuLabel");
        theMenuStr = theMenuStr.replace(/%tagname%/, GetString("rawtex"));
        scriptStr = "openOTFontDialog('rawtex', event.target.refElement);";
      break;

      case "htmlfield":
        objStr = name;
        theMenuStr = GetString("TagPropertiesMenuLabel");
        theMenuStr = theMenuStr.replace(/%tagname%/, GetString("htmlfield"));
        scriptStr = "openHTMLField(event.target.refElement);";
      break;

      case "fontcolor":
        objStr = name;
        theMenuStr = GetString("TagPropertiesMenuLabel");
        theMenuStr = theMenuStr.replace(/%tagname%/, GetString("fontcolor"));
        scriptStr = "openOTFontDialog('fontcolor', event.target.refElement);";
      break;

      case "fontsize":
        objStr = name;
        theMenuStr = GetString("TagPropertiesMenuLabel");
        theMenuStr = theMenuStr.replace(/%tagname%/, GetString("fontsize"));
        scriptStr = "openOTFontDialog('fontsize', event.target.refElement);";
      break;

      case "note":
        objStr = name;
        theMenuStr = GetString("TagPropertiesMenuLabel");
        theMenuStr = theMenuStr.replace(/%tagname%/, GetString("note"));
        scriptStr = "msiNote(event.target.refElement, null);";
      break;

      case "indexitem":
        objStr = name;
        theMenuStr = GetString("TagPropertiesMenuLabel");
        theMenuStr = theMenuStr.replace(/%tagname%/, GetString("IndexEntry"));
        scriptStr = "doInsertIndexEntry(event.target.refEditor, event.target.refElement);";
      break;

      default:
        tagclass = editor.tagListManager.getRealClassOfTag(name, null);
        switch (tagclass)
        {
          case "texttag":
          break;
          case "paratag":
            objStr = name;
            theMenuStr = GetString("TagPropertiesMenuLabel");
            theMenuStr = theMenuStr.replace(/%tagname%/, name);
            scriptStr = "openParaTagDialog('"+ name + "',event.target.refElement, event.target.refEditor);";
          break;
          case "structtag":
            objStr = name;
            theMenuStr = GetString("TagPropertiesMenuLabel");
            theMenuStr = theMenuStr.replace(/%tagname%/, name);
            scriptStr = "openStructureTagDialog('"+ name + "',event.target.refElement, event.target.refEditor);";
          break;
    // currently no dialogs for list tags, environments, and front matter.
    //      case "listtag":
    //        break;
          case "envtag":
            if (editor.tagListManager.tagCanContainTag( name, null, "envLeadIn", null))
            {
              objStr = name;
              theMenuStr = GetString("TagPropertiesMenuLabel");
              theMenuStr = theMenuStr.replace(/%tagname%/, name);
              scriptStr = "openEnvTagDialog('"+ name + "',event.target.refElement, event.target.refEditor);";
            }
          break;
    //      case "frontmtag":
    //        break;
          default: break;
        }
      break;
    }

    if (!objStr && !propsData)
    {
      var textChild = null;
      if (containingNodeData = msiNavigationUtils.getTopMathNodeAsAccentedCharacter(wrappedChildElement))
      {
        objStr = GetString("Character");
        commandStr = "cmd_reviseChars";
        coreElement = containingNodeData.mNode;
      }
      else if (textChild = msiNavigationUtils.getSingleTextNodeContent(wrappedChildElement))
      {
        propsData = new msiCharPropertiesObjectData();
        propsData.initFromNode(textChild, editorElement);
        if (propsData.hasReviseData(0))
          propsData.setTopNode(wrappedChildElement);
        else
          propsData = null;
      }
    }
  }

  if (!propsData && objStr && objStr.length)  //That is, we're constructing a garden variety simple propertiesData object:
  {
    propsData = new msiPropertiesObjectData();
    propsData.initFromNode(coreElement, editorElement);
//    propsData = new Object();
//    propsData.theNode = element;
    if (theMenuStr)
      propsData.menuStr = theMenuStr;
    else
      propsData.menuStr = GetString("ObjectProperties").replace(/%obj%/,objStr);
    propsData.commandStr = commandStr;
    propsData.scriptStr = scriptStr;
    propsData.coreElement = coreElement;
  }
  return propsData;
}


function msiSelectPropertiesMenu(event, theMenu)
{
  var bRightOne = (event.target == theMenu) || ( (event.target.nodeName == "menupopup") && (event.target.parentNode == theMenu) );
  var theEvent;
  if (bRightOne)
  {
    if ( ("defaultItem" in theMenu) && (theMenu.defaultItem != null) )
    {
      if (theMenu.commandStr)
        msiDoAPropertiesDialogFromMenu(commandStr, theMenu.defaultItem);
//      theEvent = document.createEvent("XULCommandEvent");
//      theEvent.initCommandEvent("command", true, true);  this way seems too hard
    }
  }
}

function msiGetPropertiesMenuIDs(startID)
{
  var theIDs = new Object();
  var startItem = document.getElementById(startID);
  if (!startItem) return null;

  theIDs.menuID = "propertiesMenu";
  theIDs.itemID = "objectProperties";
  theIDs.popupID = "propertiesMenuPopup";
  theIDs.bIsContextMenu = false;
  for (var menuAncestor = startItem; menuAncestor && !theIDs.bIsContextMenu; menuAncestor = menuAncestor.parentNode)
  {
    if (menuAncestor.id)
    {
      if ( (menuAncestor.id == "propertiesMenu_cm") || (menuAncestor.id == "propertiesMenuPopup_cm") )
        theIDs.bIsContextMenu = true;
//      if ( (menuAncestor.id == "objectProperties_cm") || (menuAncestor.id == "msiEditorContentContext") )
//        bIsContextMenu = true;
    }
  }
  if (theIDs.bIsContextMenu)
  {
    theIDs.menuID = "propertiesMenu_cm";
    theIDs.popupID = "propertiesMenuPopup_cm";
    theIDs.itemID = "objectProperties_cm";
  }
  return theIDs;
}

function msiGetEnclosingTableOrMatrixDimensions(editorElement, nodeInTable)
{
  var retDims = {nRows: 0, nCols : 0};
  var theTable = null;
  var theMatrix = null;
  var nRow = 0;
  var nCol = 0;
  var currRow = null;

  var aParent = msiGetContainingTableOrMatrix(nodeInTable);
  switch(msiGetBaseNodeName(aParent))
  {
    case "table":
      theTable = aParent;
    break;
    case "mtable":
      theMatrix = aParent;
    break;
    default:
    break;
  }

  if (theTable)
  {
    var nRowObj = {value : 0};
    var nColObj = {value : 0};
    var tableEditor = msiGetTableEditor(editorElement);
    if (tableEditor)
      tableEditor.getTableSize(theTable, nRowObj, nColObj);
    retDims.nRows = nRowObj.value;
    retDims.nCols = nColObj.value;
  }
  else if (theMatrix)
  {
    for (var ix = 0; ix < theMatrix.childNodes.length; ++ix)
    {     
      switch(msiGetBaseNodeName(theMatrix.childNodes[ix]))
      {
        case "mtr":
        case "mlabeledtr":
          ++nRow;
          currRow = theMatrix.childNodes[ix];
          nCol = 0;
          for (var jx = 0; jx < currRow.childNodes.length; ++jx)
          {
            switch(msiGetBaseNodeName(currRow.childNodes[jx]))
            {
              case "mtd":
                if (currRow.childNodes[jx].hasAttribute("colspan"))
                  nCol += Number(currRow.childNodes[jx].getAttribute("colspan"));
                else
                  ++nCol;
              break;

              case "#text":
              break;

              default:
                ++nCol;
              break;
            }
          }
        break;
        default:
          nCol = 1;
        break;
      }
      if (nCol > retDims.nCols)
        retDims.nCols = nCol;
    }
    retDims.nRows = nRow;
  }

  return retDims;
}

function msiGetContainingTableOrMatrix(aNode)
{
  var tableOrMatrix = null;
  for ( var aParent = aNode; (!tableOrMatrix) && (aParent); aParent = aParent.parentNode)
  {
    switch(msiGetBaseNodeName(aParent))
    {
      case "table":
      case "mtable":
        tableOrMatrix = aParent;
      break;
      default:
      break;
    }
  }
  return tableOrMatrix;
}

//function msiGetCellsInSelection(rangeArray, tableDims, editorElement)
//{
//  var retArray = new Array();
//  var tableParent = msiGetContainingTableOrMatrix(rangeArray[0].startContainer);
//  for (var ix = 0; ix < rangeArray.length; ++ix)
//  {
//
//  }
//}
//
//function msiGetSelectionForEnclosingRowsAndColumns(rangeArray, tableDims, editorElement)
//{
//  var containingTable = msiGetContainingTableOrMatrix(rangeArray[0].startContainer);
//  if (!editorElement)
//    editorElement = msiGetActiveEditorElement();
//  var tableEditor = null;
//  var bSelContainsWholeRows = false;
//}


function msiGetRowAndColumnData(tableElement, tableDims, editorElement)
{
  if (!tableDims)
    tableDims = msiGetEnclosingTableOrMatrixDimensions(editorElement, tableElement);
  var retTableData = new Object();
  retTableData.cellInfoArray = new Array(tableDims.nRows);
  for (var ii = 0; ii < tableDims.nRows; ++ii)
    retTableData.cellInfoArray[ii] = new Array(tableDims.nCols);
  retTableData.rowsData = new Array(tableDims.nRows);
  retTableData.colsData = new Array(tableDims.nCols);
  retTableData.m_nRows = tableDims.nRows;
  retTableData.m_nCols = tableDims.nCols;
  var rowCol = {m_nRow : 1, m_nCol : 1};

  function addRowsToList(aTableData, aParent, currPos)
  {
    var childNode = null;
//    var currRow = 1;
    for (var ix = 0; ix < aParent.childNodes.length; ++ix)
    {
      currPos.m_nCol = 1;
      childNode = aParent.childNodes[ix];
      switch(msiGetBaseNodeName(childNode))
      {
        case "table":
        case "mtable":
        case "thead":
        case "tfoot":
        case "tbody":
          addRowsToList(aTableData, childNode, currPos);
        break;

        case "tr":
        case "mtr":
        case "mlabeledtr":
          for (var jx = 0; jx < childNode.childNodes.length; ++jx)
          {
            if (childNode.childNodes[jx].nodeType == 1)
              addCellToList(aTableData, childNode.childNodes[jx], currPos);
          }
        break;

        case "caption":
        break;

        case "th":
        case "td":
        default:
          addCellToList(aTableData, childNode, currPos);
        break;
      }
      ++currPos.m_nRow;
    }
  }

//  function setCellData(aTableData, nRowIndex, nColIndex, datumName, datumValue)
//  {
//    var cellData = aTableData.cellInfoArray[nRowIndex][nColIndex];
//    if (!cellData)
//    {
//      aTableData.cellInfoArray[nRowIndex][nColIndex] = new Object();
//      cellData = aTableData.cellInfoArray[nRowIndex][nColIndex];
//    }
//    var compare = 1;
//    switch(datumName)
//    {
//      case "mRowContinuation":
//      case "mColContinuation":
//        compare = 0;
//      break;
//    }
//    if ( !(datumName in rowData) || ( (compare * rowData[datumName]) < (compare * datumValue) ) )
//      rowData[datumName] = datumValue;
//  }
//
  function setRowData(aTableData, nRowIndex, datumName, datumValue)
  {
    var rowData = aTableData.rowsData[nRowIndex];
    if (!rowData)
    {
      aTableData.rowsData[nRowIndex] = {firstRow : nRowIndex, lastRow : nRowIndex};
      rowData = aTableData.rowsData[nRowIndex];
    }
    var compare = 1;
    switch(datumName)
    {
      case "firstRow":
        compare = -1;
      break;
    }
    if ( !(datumName in rowData) || ( (compare * rowData[datumName]) < (compare * datumValue) ) )
      rowData[datumName] = datumValue;
  }

  function setColData(aTableData, nColIndex, datumName, datumValue)
  {
    var colData = aTableData.colsData[nColIndex];
    if (!colData)
    {
      aTableData.colsData[nColIndex] = {firstCol : nColIndex, lastCol : nColIndex};
      colData = aTableData.colsData[nColIndex];
    }
    var compare = 1;
    switch(datumName)
    {
      case "firstCol":
        compare = -1;
      break;
    }
    if ( !(datumName in colData) || ( compare * (colData[datumName]- datumValue) < 0) )
      colData[datumName] = datumValue;
  }

  function addCellToList(aTableData, cellNode, currPos)
  {
    // next line added by BBM
    return;
    var numCols = aTableData.cellInfoArray[0].length;
    var numRows = aTableData.cellInfoArray.length;
    var colspan = 1;
    var rowspan = 1;

    try 
    {
      switch(msiGetBaseNodeName(cellNode))
      {
        case "th":
        case "td":
        case "mtd":
          while ( (currPos.m_nRow <= numRows) && (currPos.m_nCol <= numCols) && (aTableData.cellInfoArray[currPos.m_nRow-1][currPos.m_nCol-1] != null) )
          {
            ++currPos.m_nCol;
          }
          if (currPos.m_nCol > numCols)  //There's no empty spot in this row for the cell data.
          {
            dump("In msiEditor.js, msiGetRowAndColumnData(), problem with too many cells in row [" + currPos.m_nRow + "].\n");
            return;
          }
          aTableData.cellInfoArray[currPos.m_nRow-1][currPos.m_nCol-1] = {mNode : cellNode, mRowContinuation : 0, mColContinuation : 0};
          if (cellNode.hasAttribute("colspan"))
            colspan = Number(cellNode.getAttribute("colspan"));
          if (colspan == 0)
            colspan = numCols - currPos.m_nCol + 1;
          else if (colspan > numCols - currPos.m_nCol + 1)
            colspan = numCols - currPos.m_nCol + 1;
          if (cellNode.hasAttribute("rowspan"))
            rowspan = Number(cellNode.getAttribute("rowspan"));
          if (rowspan == 0)
            rowspan = numRows - currPos.m_nRow + 1;
          else if (rowspan > numRows - currPos.m_nRow + 1)
            rowspan = numRows - currPos.m_nRow + 1;
          setRowData(aTableData, currPos.m_nRow-1, "lastNonemptyCell", currPos.m_nCol - 2 + colspan);
          setColData(aTableData, currPos.m_nCol-1, "lastNonemptyCell", currPos.m_nRow - 2 + rowspan);
          if (colspan != 1)
          {
            aTableData.cellInfoArray[currPos.m_nRow-1][currPos.m_nCol-1].mColContinuation = colspan - 1;
            setColData(aTableData, currPos.m_nCol-1, "firstCol", currPos.m_nCol - 1);
            setColData(aTableData, currPos.m_nCol-1, "lastCol", currPos.m_nCol - 2 + colspan); //lastCol calculation is (nCol-1) + (colspan-1)
          }
          if (rowspan != 1)
          {
            aTableData.cellInfoArray[currPos.m_nRow-1][currPos.m_nCol-1].mRowContinuation = rowspan - 1;
            setRowData(aTableData, currPos.m_nRow-1, "firstRow", currPos.m_nRow - 1);
            setRowData(aTableData, currPos.m_nRow-1, "lastRow", currPos.m_nRow - 2 + rowspan);   //lastRow calculation is (nRow-1) + (rowspan-1)
          }
          for (var ii = 0; ii < rowspan; ++ii)
          {
            for (var jj = 0; jj < colspan; ++jj)
            {
              if (!ii && !jj)
                continue;
              if (!aTableData.cellInfoArray[currPos.m_nRow-1+ii][currPos.m_nCol-1+jj])
                aTableData.cellInfoArray[currPos.m_nRow-1+ii][currPos.m_nCol-1+jj] = {mNode : cellNode, mRowContinuation : -ii, mColContinuation : -jj};
  //            aTableData.cellInfoArray[nRow-1+ii][nCol-1+jj].mNode = cellNode;
  //            aTableData.cellInfoArray[nRow-1+ii][nCol-1+jj].mRowContinuation = -ii-1;
  //            aTableData.cellInfoArray[nRow-1+ii][nCol-1+jj].mColContinuation = -jj-1;
              if (ii > 0)
              {
                setRowData(aTableData, currPos.m_nRow-1+ii, "firstRow", currPos.m_nRow - 1);
                setRowData(aTableData, currPos.m_nRow-1+ii, "lastRow", currPos.m_nRow - 2 + rowspan);
              }
              if (jj > 0)
              {
                setColData(aTableData, currPos.m_nCol-1+jj, "firstCol", currPos.m_nCol - 1);
                setColData(aTableData, currPos.m_nCol-1+jj, "lastCol", currPos.m_nCol - 2 + colspan);
              }
            }
          }
          currPos.m_nCol += colspan;  //we don't change nRow here, incidentally - it can only change at the outer level of the loop (in the calling function).
        break;

        default:
          // We end up here when the cell contains a text node. Not an error.
          // dump("In msiEditor.js, msiGetRowAndColumnData(), bad cell node passed in to addCellToList - node is [" + msiGetBaseNodeName(cellNode) + "].\n");
        break;
      }
    }
    catch(e) {
      dump(e.message);
    }
  }

//  for (var ix = 0; ix < tableElement.childNodes.length; ++ix)
//  {

  addRowsToList(retTableData, tableElement, rowCol);
//  }

  return retTableData;
}

function msiMergeArrayIntoArray( targArray, srcArray, orderFunction )
{
  function doInsertOne(anItem)
  {
    var bDone = false;
    if (orderFunction)
    {
      for (var jx = 0; jx < targArray.length; ++jx)
      {
        var orderRes = orderFunction(targArray[jx], anItem);
        if ( orderRes > 0 )
        {
          targArray.splice(jx, 1, anItem);
          bDone = true;
        }
        else if (orderRes == 0)
          bDone = true;
      }
    }
    if (!bDone)  //this happens if either there is ordering but anItem belongs after all the others, or if there's no ordering
      targArray.push(anItem);
  }

  for (var ix = 0; ix < srcArray.length; ++ix)
  {
    if (targArray.indexOf(srcArray[ix]) < 0)
      doInsertOne(srcArray[ix]);
  }
}

//WHAT'S LEFT TO DO:
//  i) Need to ensure that the code descending into an mrow or mstyle to be a "wrapped" element gets executed everywhere.

function msiPropertiesObjectData() {}

var msiPropertiesObjectDataBase =
{

// NOTE: We're probably abandoning all this "ContentType" stuff...
//  //Constants: represents a jumbled attempt to identify some orthogonal properties (which can hopefully be improved
//  //  into a coherent attempt) Below, the phrase "selected text" indicates "content under consideration", which may not
//  //  be the same as the on-screen selection, and of course may not be "text".
//  ContentType_Unknown : 0,
//  ContentType_Node : 1,  //the selected text is the content of a single node (and not one of the special cases below).
//  ContentType_Character : 2,  //The selected text is a character within a (Text) node.
//  ContentType_Characters : 3,  //The selected text is a string of characters within a Text node.
//  ContentType_Space : 4,       //The selected text is a space character within a text node. (Other special spacing objects will occur as ContentType_Node.)
//  ContentType_Table : 5,   //The selected text is a table.
//  ContentType_TableCell : 0x11,  //The selected text is a table cell.
//  ContentType_TableCellGroup : 0x12,  //The selected text is a group (block?) of table cells.
//  ContentType_TableRows : 0x14,  //The selected text consists of a group of cells spanning entire table rows. This may or may not be the same as a <tr>.
//  ContentType_TableColumns : 0x18, //The selected text consists of a group of cells spanning entire table columns.

//Constants and data:
  Selected_SomeSelected : 1,
  Selected_SomeUnselected : 2,
  Selected_MixedSelection : 3,

  menuStr : null,
  commandStr : null,
  scriptStr : null,

////Interface:
//  QueryInterface : function(aIID)
//  {
//    if (aIID.equals(Components.interfaces.nsISupports)
//    || aIID.equals(Components.interfaces.nsISupportsWeakReference))
//      return this;
//    throw Components.results.NS_NOINTERFACE;
//  },

  initFromNode : function(aNode, editorElement)
  {
    this.setEditorElement(editorElement);
    this.mNode = aNode;
//    this.mContentType = this.ContentType_Node;
    this.finishInit();
  },

  initFromSelection : function(editorElement)
  {
    this.setEditorElement(editorElement);
    var theEditor = msiGetEditor(this.mEditorElement);
    var container = msiNavigationUtils.getCommonAncestorForSelection(theEditor.selection);
    if (container)
      this.mNode = container;
//    var containerData = msiGetSelectionContainer(editorElement);
//    if (containerData)
//      this.mNode = containerData.node;
    this.mSelection = [];
    if (theEditor)
    {
      for (var ix = 0; ix < theEditor.selection.rangeCount; ++ix)
        this.mSelection.push( theEditor.selection.getRangeAt(ix).cloneRange() );
    }
    this.finishInit();
//    this.initFromData(rangeArray, editorElement);
  },

//  initFromData : function(rangeArray, editorElement)
//  {
//    this.mEditorElement = editorElement;
//    this.mSelection = rangeArray;
//    this.mNode =
////    this.mContentType = this.getContentType();
//    this.finishInit();
//  },

  doSelectItems : function(menuLabel)
  {
    var editorElement = this.mEditorElement;
    if (!editorElement)
      this.mEditorElement = editorElement = msiGetActiveEditorElement();
    var theEditor = msiGetEditor(editorElement);
    var aNode, aRange, aRangeArray;
    aRange = this.getRange();
    if (aRange)
      aRangeArray = [aRange];
    else
      aRangeArray = this.getRangeArray();
    if (aRangeArray)
    {
      theEditor.selection.removeAllRanges();
      for (var ii = 0; ii < aRangeArray.length; ++ii)
        theEditor.selection.addRange(aRangeArray[ii]);
      return;
    }
    aNode = this.getNode();
    if (aNode)
    {
      theEditor.selectElement(aNode);
      return;
    }
    else
      dump("Trouble in msiEditor.js, in msiPropertiesObject.doSelectItems - we don't have a node, range, or range array to select!\n");
  },

  getReferenceNode : function()  //The "reference Node" is intended to be the Node used to index or look up a properties dialog in our list of properties dialogs.
  {
    var retNode = this.getNode();
    if (retNode)
      return retNode;
    //otherwise we represent a selection - or should a character in a text run be a special case?

  },

  getPropertiesDialogNode : function()
  {
    return this.getTopNode();
  },

  getNode : function()
  {
    return this.mNode;
  },

  getRange : function()
  {
    if (this.mSelection && this.mSelection.length == 1)
      return this.mSelection[0];
    return null;
  },

  getRangeArray : function()
  {
    if (this.mSelection && this.mSelection.length > 1)
      return this.mSelection;
    return null;
  },

  getTopNode : function()
  {
    if (this.mTopNode)
      return this.mTopNode;
    return this.getReferenceNode();
  },

  getMenuString : function(nWhich)
  {
    if ( (nWhich==0) && this.menuStr )
      return this.menuStr;
    return null;
  },

  getCommandString : function(nWhich)
  {
    if ( (nWhich==0) && this.commandStr )
      return this.commandStr;
    return null;
  },

  getScriptString : function(nWhich)
  {
    if ( (nWhich==0) && this.scriptStr )
      return this.scriptStr;
    return null;
  },

  getSpaceInfo : function()
  {
    var retInfo = null;
    if (this.mNode)
      retInfo = msiSpaceUtils.getSpaceInfoFromNode(this.mNode);
    return retInfo;
  },

  hasReviseData : function(iter)
  {
    return ((iter == 0) && this.menuStr);
  },

  isMatrix : function()
  {
    return false;
  },

  isTextReviseData : function()
  {
    return false;
  },

  getTextLength : function()
  {
    return 0;  //is this the right thing to do? Hopefully won't come up...
  },

//Implementation (non-interface) methods:
  setEditorElement : function(editorElement)
  {
    if (!editorElement)
      editorElement = msiGetActiveEditorElement();
    this.mEditorElement = editorElement;
  },

  finishInit : function()
  {
    this.setTopNode();
//    this.setStrings();
  },

  setStrings : function()
  {
    //Do nothing here. For a regular simple node-based properties data, the strings are set in the function creating us.
  },

  setTopNode : function(ourNode)
  {
    if (!ourNode)
      ourNode = this.getReferenceNode();
    var parentNode = msiNavigationUtils.findWrappingNode(ourNode);
    if (parentNode != null)
      this.mTopNode = parentNode;
    else
      this.mTopNode = ourNode;
  },

  //Need a function to "normalize" a range or selection. What we need to know is whether the range actually contains just one node,
  //  or whether it's within a run of text and then how many characters it contains.
  normalizeRange : function(aRange)
  {
    var retRange = aRange.cloneRange();
    var topNode = retRange.commonAncestorContainer;
    while (retRange.startContainer != topNode)  //in this case the startContainer is a descendant of topNode, and endContainer is a different descendant (or is topNode itself).
    {
      //We can only do anything to help if there's no content inside startContainer before us, in which case we can move the start outside.
      //On the other hand, if the startContainer has no content after the range start we can move out to the right.
      if (!msiNavigationUtils.nodeHasContentBeforeRangeStart(retRange, retRange.startContainer))
        retRange.setStartBefore(retRange.startContainer);
      else if (!msiNavigationUtils.nodeHasContentAfterRangeStart(retRange, retRange.startContainer))
        retRange.setStartAfter(retRange.startContainer);
      else
        break;
    }
    while (retRange.endContainer != topNode)  //in this case the endContainer is a descendant of topNode, and startContainer is a different descendant (or is topNode itself).
    {
      //We can only do anything to help if there's no content inside endContainer after us, in which case we can move the end outside.
      if (!msiNavigationUtils.nodeHasContentAfterRangeEnd(retRange, retRange.endContainer))
        retRange.setEndAfter(retRange.endContainer);
      else if (!msiNavigationUtils.nodeHasContentBeforeRangeEnd(retRange, retRange.endContainer))
        retRange.setEndBefore(retRange.endContainer);
      else
        break;
    }
    return retRange;
  }

//  getContentType : function()
//  {
//    if (this.mContentType && (this.mContentType != this.ContentType_Unknown))
//      return this.mContentType;
//
//    if (this.mNode)
//    {
//      this.mContentType = this.contentTypeFromNode(this.mNode);
//      return this.mContentType;
//    }
//
//    //Otherwise we're looking at a selection. Is it just a range?
//    var theRange = this.getRange();
//    if (theRange)
//    {
//      this.mContentType = this.contentTypeFromRange(theRange);
//      return this.mContentType;
//    }
//
//    //Must be a complex selection:
//    var rangeArray = this.getRangeArray();
//    if (rangeArray)
//    {
//      this.mContentType = this.contentTypeFromRangeArray(rangeArray);
//      return this.mContentType;
//    }
//
//    this.mContentType = this.ContentType_Unknown;
//    return this.mContentType;
//  },
//
//  contentTypeFromNode : function(aNode)
//  {
//    var retType = this.mContentType_Node;
//    switch(msiGetBaseNodeName(aNode))
//    {
//      case '':  //node without a name? probably a null node passed in.
//        retType = this.ContentType_Unknown;
//      break;
//      case 'mtd':
//      case 'td':
//      case 'th':
//        retType = this.ContentType_TableCell;
//      break;
//      case 'mtable':
//      case 'table':
//        retType = this.ContentType_Table;
//      break;
//      case 'mtr':
//      case 'mlabeledtr':
//      case 'tr':
//        retType = this.ContentType_TableRows;
//      break;
//      case '#text':
//        if (aNode.textContent.length > 1)
//          retType = this.ContentType_Characters;
//        else if (aNode.textContent.length < 1)
//          retType = this.ContentType_Unknown;
//        else  //Have to look at the content
//        {
//          if (msiNavigationUtils.isWhiteSpace(aNode.textContent))
//            retType = this.ContentType_Space;
//          else
//            retType = this.ContentType_Character;
//        }
//      break;
//    }
//    return retType;
//  },

//  contentTypeFromRange : function(theRange)
//  {
//    var retType = this.ContentType_Unknown;
//    var testRange = this.normalizeRange(theRange);
//    if (testRange.startContainer == testRange.endContainer)
//    {
//      var nLength = testRange.endOffset - testRange.startOffset;
//      if ( ("childNodes" in testRange.startContainer) && (testRange.startContainer.childNodes.length >= testRange.endOffset) )
//      {
//        if (nLength == 1)
//          retType = this.contentTypeFromNode(testRange.startContainer.childNodes[testRange.endOffset-1]);
//      }
//      if (retType == this.ContentType_Unknown)
//      {
//        switch(msiGetBaseNodeName(testRange.startContainer))
//        {
//          case '#text':
//            var theText = testRange.startContainer.textContent;
//            if (theText.length >= testRange.endOffset)
//            {
//              if (nLength > 1)
//                retType = this.ContentType_Characters;
//              else if (nLength < 1)
//                retType = this.ContentType_Unknown;
//              else  //Have to look at the content
//              {
//                if (msiNavigationUtils.isWhiteSpace(theText.substr(testRange.startOffset, 1))
//                  retType = this.ContentType_Space;
//                else
//                  retType = this.ContentType_Character;
//              }
//            }
//          break;
//          case 'mtd':
//          case 'td':
//          case 'th':  //in this case we're some subset of a table cell - can't do anything for this
//          break;
//          case 'mtr':
//          case 'mlabeledtr':
//          case 'tr':
//          case 'table':
//            retType = this.contentTypeForTable([testRange]); //pass it as a single-entry array
//          break;
//          default:
//          break;
//        }
//      }
//    }
//    return retType;
//  },

};
msiPropertiesObjectData.prototype = msiPropertiesObjectDataBase;

function msiCharPropertiesObjectData() {}
msiCharPropertiesObjectData.prototype =
{
//Data:
  mOffset : null,
  mText : null,
  mLength : 1,

//Interface:
  //Note that in this function the text to be possibly modified is to the LEFT of "anOffset".
  initFromNode : function(aNode, editorElement)
  {
    var nLen = aNode.textContent.length;
    this.initFromNodeAndOffset(aNode, nLen, editorElement);
  },

  initFromNodeAndOffset : function(aNode, anOffset, editorElement)
  {
    this.setEditorElement(editorElement);
    this.mNode = aNode;
    var aRange = msiNavigationUtils.getCharacterRange(aNode.textContent, anOffset);
//    if (anOffset > 0)
//      this.mOffset = msiNavigationUtils.findSingleCharStart(aNode.textContent, anOffset);
//    else
//      this.mOffset = 0;
    this.mText = aNode.textContent.substring(aRange.mStart, aRange.mEnd);
    this.mOffset = aRange.mStart;
    this.mLength = this.mText.length;
    this.examineText();
    this.setTopNode();
  },

  initFromSelection : function(aSelection, editorElement)
  {
    this.setEditorElement(editorElement);
    var editor = msiGetEditor(editorElement);
    var container = msiNavigationUtils.getCommonAncestorForSelection(editor.selection);
//    var containerData = msiGetSelectionContainer(editorElement);
//    var container = containerData ? containerData.node : null;
    this.mNode = container;
    if (aSelection.rangeCount == 1)
    {
      var theRange = aSelection.getRangeAt(0);
      if ( (theRange.startContainer == container) && (theRange.endContainer == container) )  //are there any other cases to consider?
      {
        this.mLength = theRange.endOffset - theRange.startOffset;
        this.mText = container.textContent.substr(theRange.startOffset, this.mLength);
        this.mOffset = theRange.startOffset;
      }
    }
    this.examineText();
    this.setTopNode();
  },

  doSelectItems : function(menuLabel)
  {
    var editorElement = this.mEditorElement;
    if (!editorElement)
      this.mEditorElement = editorElement = msiGetActiveEditorElement();
    var theEditor = msiGetEditor(editorElement);
    var aNode, aRange, aRangeArray;
    aRange = this.getRange();
    if (aRange)
      aRangeArray = [aRange];
    else
      aRangeArray = this.getRangeArray();
    if (aRangeArray)
    {
      theEditor.selection.removeAllRanges();
      for (var ii = 0; ii < aRangeArray.length; ++ii)
        theEditor.selection.addRange(aRangeArray[ii]);
      return;
    }
    aNode = this.getNode();
    if (aNode)
    {
      theEditor.selectElement(aNode);
      return;
    }
    else
      dump("Trouble in msiEditor.js, in msiPropertiesObject.doSelectItems - we don't have a node, range, or range array to select!\n");
  },

//Implementation:
  setStrings : function()
  {
    //do nothing - the setting of appropriate strings should have taken place during the examineText() function
    dump("In msiEditor.js, msiCharPropertiesObjectData.setStrings() was called - this shouldn't happen!\n");
  },

  getSpaceInfo : function()
  {
    var retVal = null;
    if (this.mText)
      retVal = msiSpaceUtils.spaceInfoFromChars(this.mText);
    return retVal;
  },

  isTextReviseData : function()
  {
    return true;
  },

  getTextLength : function()
  {
    return this.mLength;
  },

  getTextOffset : function()
  {
    return this.mOffset;
  },

  getText : function()
  {
    return this.mText;
  },

  examineText : function()
  {
    var objStr;
    if (this.mText)
    {
      var spaceInfo = msiSpaceUtils.spaceInfoFromChars(this.mText);
      if (spaceInfo != null)
      {
        switch(spaceInfo.theType)
        {
          case 'hspace':
//            objStr = GetString("HorizontalSpace");
            objStr = "HorizontalSpace";
            this.commandStr = "cmd_reviseHorizontalSpaces";
          break;

          case 'vspace':
//            objStr = GetString("VerticalSpace");
            objStr = "VerticalSpace";
            this.commandStr = "cmd_reviseVerticalSpaces";
          break;

          case 'msibr':
//            objStr = GetString("GenBreak");
            objStr = "GenBreak";
            this.commandStr = "cmd_msiReviseBreaks";
          break;

          default:
          break;
        }
      }
//      else if (this.mText.length == 1)
      else if (msiNavigationUtils.isSingleCharacter(this.mText))
      {
//        objStr = GetString("Character");
        objStr = GetString("Character");
        this.commandStr = "cmd_reviseChars";
      }

      if (objStr)
      {
        this.menuStr = msiFormatPropertiesMenuString(objStr);
//        this.menuStr = GetString("ObjectProperties").replace(/%obj%/,objStr);
        var newRange = this.mNode.ownerDocument.createRange();
        newRange.setStart(this.mNode, this.mOffset);
        newRange.setEnd(this.mNode, this.mOffset + this.mText.length);
        this.mSelection = [newRange];
      }
    }
  }
};

msiCharPropertiesObjectData.prototype.__proto__ = msiPropertiesObjectDataBase;

function msiTablePropertiesObjectData()
{
  this.menuStrings = [];
  this.commandStrings = [];
  this.scriptStrings = [];
}

msiTablePropertiesObjectData.prototype =
{
//Data:
  menuStrings: [],
  commandStrings : [],
  scriptStrings : [],
  mRowSelectionArray : [],
  mColSelectionArray : [],


  matrixStrArray : [//"MatrixCell", 
                    "MatrixCellGroup", 
                    // "MatrixRow", "MatrixColumn", 
                    "Matrix"],
  tableStrArray : [//"TableCell", 
                    "TableCellGroup", 
                    // "TableRow", "TableColumn", 
                    "Table"],

//Interface:
  initFromNode : function(aNode, editorElement, bSelected)
  {
    this.setEditorElement(editorElement);
    this.mStartNode = aNode;
    switch(msiGetBaseNodeName(aNode))
    {
      case "mtd":
      case "td":
      case "th":
        this.mCell = aNode;
        this.mTableElement = msiGetContainingTableOrMatrix(aNode);
      break;

      case "mtr":
      case "mlabeledtr":
      case "tr":
      case "thead":
      case "tfoot":
      case "tbody":
        this.mTableElement = msiGetContainingTableOrMatrix(aNode);
      break;

      case "table":
      case "mtable":
        this.mTableElement = aNode;
      break;
    }
    var aRange = aNode.ownerDocument.createRange();
    //Is this right????? Yes - if we're being initialized from a node, it should be treated as selected for the purposes of finding containing rows and columns...
    aRange.setEndAfter(aNode);
    aRange.setStartBefore(aNode);
    this.examineTable( [aRange] );
    this.finishInit();
    this.setStrings();
  },

  initFromSelection : function(aSelection, editorElement)
  {
    this.setEditorElement(editorElement);
    this.mSelection = [];
    for (var ix = 0; ix < aSelection.rangeCount; ++ix)
      this.mSelection.push( aSelection.getRangeAt(ix).cloneRange() );
    this.mTableElement = msiGetContainingTableOrMatrix(this.mSelection[0].startContainer);
    this.examineTable(this.mSelection);
//    var tableDims = msiGetEnclosingTableOrMatrixDimensions(this.mEditorElement, this.mTableElement);
//    this.mTableInfo = msiGetRowAndColumnData(this.mTableElement, tableDims, editorElement);
//    this.markCellsInRangeArray(rangeArray);
//    this.mRowSelection = this.getSelectionForWholeRows(rangeArray);
//    this.mColSelection = this.getSelectionForWholeColumns(rangeArray);
//    if (!this.mNode && !this.mRowSelection && !this.mColSelection)
      //does this mean the entire table is selected?

    this.finishInit();
    this.setStrings();
  },

  //This function should only be called after we've done all our work - and thus things SHOULD be readily available...
  doSelectItems : function(menuLabel)
  {
    var editorElement = this.mEditorElement;
    if (!editorElement)
      this.mEditorElement = editorElement = msiGetActiveEditorElement();
    var theEditor = msiGetEditor(editorElement);
    var strArray = this.getMenuStringArray();

    var aNode, aRangeArray;
    if (menuLabel == msiFormatPropertiesMenuString(strArray[4]))  //"Table" or "Matrix"
      aNode = this.mTableElement;
//    else if (menuLabel == GetString("TableCell"))  //This should only be available if our original selection is within a single table cell.
    else if (menuLabel == msiFormatPropertiesMenuString(strArray[0]))  //Table Cell or Matrix Cell
    {
      aNode = this.mCell;  //will this work?
      if (!aNode)
      {
        dump("In msiTablePropertiesObjectData.doSelectItems(), mNode hasn't been set though we've reported TableCell as an option!\n");
        aNode = this.getSingleSelectedCell();
        //If this still fails, we'll return null and further operations will simply not happen.
      }
    }
//    else if (menuLabel == GetString("TableColumn"))
    else if (menuLabel == msiFormatPropertiesMenuString(strArray[3]))  //TableColumn or MatrixColumn
      aRangeArray = this.mColSelection;
//    else if (menuLabel == GetString("TableRow"))
    else if (menuLabel == msiFormatPropertiesMenuString(strArray[2]))  //TableRow or MatrixRow
      aRangeArray = this.mRowSelection;
    else if (menuLabel == msiFormatPropertiesMenuString(strArray[1]))  //TableCellGroup or MatrixCellGroup
      aRangeArray = this.mSelection;

    if (aNode)
    {
      theEditor.selectElement(aNode);
      return;
    }
    if (aRangeArray && (aRangeArray.length > 0))
    {
      theEditor.selection.removeAllRanges();
      for (var ii = 0; ii < aRangeArray.length; ++ii)
        theEditor.selection.addRange(aRangeArray[ii]);
    }
    else
      dump("Trouble in msiEditor.js, in msiPropertiesObject.doSelectItems - we don't have a node, range, or range array to select!\n");
  },

  getReferenceNode : function()
  {
    return this.mTableElement;
  },

  getNode : function()
  {
    if (this.mStartNode)
      return this.mStartNode;
    if (this.mCell)
      return this.mCell;
    return null;
  },

  getRange : function()
  {
    if (this.mSelection && this.mSelection.length == 1)
      return this.mSelection[0];
    return null;
  },

  getRangeArray : function()
  {
    if (this.mSelection && this.mSelection.length > 1)
      return this.mSelection;
    return null;
  },

  getMenuString : function(nWhich)
  {
    if (this.menuStrings[nWhich])
      return this.menuStrings[nWhich];
    return null;
  },

  getCommandString : function(nWhich)
  {
    if (this.commandStrings[nWhich])
      return this.commandStrings[nWhich];
    return null;
  },

  getScriptString : function(nWhich)
  {
    if (this.scriptStrings[nWhich])
      return this.scriptStrings[nWhich];
    return null;
  },

  hasReviseData : function(iter)
  {
    return (iter < this.menuStrings.length);
  },

  isMatrix : function()
  {
    switch( msiGetBaseNodeName(this.mTableElement) )
    {
      case "mtable":
        return true;
      break;
      default:
        return false;
      break;
    }
  },

  getSelectionType : function(commandStr)
  {
    var retStr = "";
    switch(commandStr)
    {
      case "cmd_MSIreviseMatrixCellCmd":
      case "cmd_editTableCell":
        return "Cell";
      break;
      case "cmd_MSIreviseMatrixCellGroupCmd":
      case "cmd_editTableCellGroup":
        return "CellGroup";
      break;
      case "cmd_MSIreviseMatrixRowsCmd":
      case "cmd_editTableRows":
        return "Row";
      break;
      case "cmd_MSIreviseMatrixColsCmd":
      case "cmd_editTableCols":
        return "Col";
      break;
      case "cmd_MSIreviseMatrixCmd":
      case "cmd_editTable":
        return "Table";
      break;
      default:
        //The default case is what happens below - not sure whether there's a good case for expecting this to happen or not.
      break;
    }
    if (this.mCell != null)
      retStr = "Cell";
    else if (this.mSelection)
    {
      if (this.mSelection == this.mRowSelection)
        retStr = "Row";
      else if (this.mSelection == this.mColSelection)
        retStr = "Col";
      if (this.allCellsSelected())
        retStr = "Table";
      if (!retStr.length)
        retStr = "CellGroup";
    }
    return retStr;
  },

  getTableDims : function()
  {
    var tableDims = {nRows : 0, nCols : 0};
    if (!this.mTableInfo)
    {
      dump("In in msiEditor.js, msiTablePropertiesObject.getTableDims() called without having mTableInfo set!\n");
      tableDims = msiGetEnclosingTableOrMatrixDimensions(this.mEditorElement, this.mTableElement);
    }
    else
    {
      tableDims.nRows = this.mTableInfo.m_nRows;
      tableDims.nCols = this.mTableInfo.m_nCols;
    }
    return tableDims;
  },

  beginSelectedCellIteration : function(selectionMode)
  {
    var retIter = { mSelMode: selectionMode, nRow : 0, nCol : 0, mRowContinuation : 0, mColContinuation : 0};
    return retIter;
  },

  getNextSelectedCell : function(cellIter)
  {
    return null;
    var bFound = false;
    var ix = cellIter.nRow;
    var startJ = cellIter.nCol;
    if (cellIter.mCell)
      ++startJ;
    var theCell = null;
    for (; !bFound && (ix < this.mTableInfo.m_nRows); ++ix)
    {
      for (var jx = startJ; !bFound && (jx < this.mTableInfo.cellInfoArray[ix].length); ++jx)
      {
        if (this.cellIsInSelection(ix, jx, cellIter.mSelMode))
//        if ( this.mTableInfo.cellInfoArray[ix][jx] && (this.mTableInfo.cellInfoArray[ix][jx].mSelected == msiPropertiesObjectDataBase.Selected_SomeSelected))
        {
          cellIter.nRow = ix;
          cellIter.nCol = jx;
          cellIter.mRowContinuation = this.mTableInfo.cellInfoArray[ix][jx].mRowContinuation;
          cellIter.mColContinuation = this.mTableInfo.cellInfoArray[ix][jx].mColContinuation;
          theCell = this.mTableInfo.cellInfoArray[ix][jx].mNode;
          bFound = true;
        }
      }
      startJ = 0;
    }
    cellIter.mCell = theCell;
    return theCell;
  },

  cellIsInSelection : function(nRow, nCol, modeStr)
  {
    switch(modeStr)
    {
      case "Row":
        return (this.mRowSelectionArray.indexOf(nRow) >= 0);
      break;
      case "Col":
        return (this.mColSelectionArray.indexOf(nCol) >= 0);
      break;
      case "Cell":
      case "CellGroup":
        return ( this.mTableInfo.cellInfoArray[nRow][nCol] && ("mSelected" in this.mTableInfo.cellInfoArray[nRow][nCol])
          && (this.mTableInfo.cellInfoArray[nRow][nCol].mSelected == msiPropertiesObjectDataBase.Selected_SomeSelected) );
      break;
      case "Table":
        return true;
      break;
    }
  },

  getCellExtent : function(nRow, nCol)
  {
    var retDims = {mRowContinuation : this.mTableInfo.cellInfoArray[nRow][nCol].mRowContinuation,
                   mColContinuation : this.mTableInfo.cellInfoArray[nRow][nCol].mColContinuation};
    return retDims;
  },

  getColsInSelection : function(modeStr)
  {
    var retArray;
    var nRows = this.mTableInfo.m_nRows;
    if (modeStr == "Table")
    {
      retArray = [];
      for (var ix = 0; ix < this.mTableInfo.m_nCols; ++ix)
        retArray.push(ix);
      return retArray;
    }
    return this.mColSelectionArray;
  },

  getRowsInSelection : function(modeStr)
  {
    var retArray;
    if (modeStr == "Table")
    {
      retArray = [];
      for (var ix = 0; ix < this.mTableInfo.m_nRows; ++ix)
        retArray.push(ix);
      return retArray;
    }
    return this.mRowSelectionArray;
  },

//Implementation (non-interface) methods:
  setStrings : function()
  {
    //Point of this logic is:
    //  (i) if we have "mCell" it means the selection is a single cell, so Table Cell, Table Row, Table Column, and Table properties should be on menu.
    // (ii) We don't want to show Table Row properties only if the rows selection includes the whole table, and similarly for columns.
    //(iii) if we do have a cell then the others should show up if present? (If we have a cell in a one-row table, would we want to activate
    //      column properties?)
    var ourArray = [//"mCell", 
      "mSelection", 
      //"mRowSelection", "mColSelection", 
      "mTableElement"];
    var strArray = null;
    var commandArray = null;
    var strArray = this.getMenuStringArray();

    if (this.isMatrix())
    {
//    //  strArray = ["MatrixCell", "MatrixCellGroup", "MatrixRow", "MatrixColumn", "Matrix"];
      //commandArray = [//"cmd_MSIreviseMatrixCellCmd", 
      //"cmd_MSIreviseMatrixCellGroupCmd", //"cmd_MSIreviseMatrixRowsCmd", "cmd_MSIreviseMatrixColsCmd", 
      //"cmd_MSIreviseMatrixCmd"];
    }
    else
    {
//      strArray = ["TableCell", "TableCellGroup", "TableRow", "TableColumn", "Table"];
      commandArray = [//"cmd_editTableCell", 
      "cmd_editTableCellGroup", 
      //"cmd_editTableRows", "cmd_editTableCols", 
      "cmd_editTable"];
    }

    //New decision - abandon this. Whatever part of a table or matrix we're in or have selected, we can bring up a properties dialog.
    var bDoIt = true;
    for (var ix = 0; ix < ourArray.length; ++ix)
    {
      // bDoIt = (this[ourArray[ix]] != null);
      // if (bDoIt && (ix > 0) && (ix < ourArray.length - 1))  //if selection is within a cell, don't show anything else except table
      // {
      //   if (this.mCell != null)
      //     bDoIt = false;
      // }
      // if (bDoIt)
      // {
      //   switch(ourArray[ix])
      //   {
      //     // case "mCell":
      //     //   if ((this.mRowSelection == null) && (this.mColSelection == null))  //means selecting all rows or cols gives whole table - so selection is essentially whole table
      //     //     bDoIt = false;
      //     // break;
      //     case "mSelection":   //The "Cell Group" properties should only appear if the selection doesn't match any of the other types
      //       bDoIt = true;
      //       // if ((this.mSelection == this.mRowSelection) || (this.mSelection == this.mColSelection))
      //       //   bDoIt = false;
      //       // if (this.allCellsSelected())
      //       //   bDoIt = false;
      //     break;
      //     // case "mRowSelection":
      //     //   if (this.mSelection != this.mRowSelection)
      //     //     bDoIt = false;
      //     // break;
      //     // case "mColSelection":
      //     //   if (this.mSelection != this.mColSelection)
      //     //     bDoIt = false;
      //     // break;
      //   }
      // }
      // if (bDoIt)
      // {
        this.menuStrings.push( msiFormatPropertiesMenuString( strArray[ix] ) );
        this.commandStrings.push( commandArray[ix] );
      // }
    }
  },

  getMenuStringArray : function()
  {
    if (this.isMatrix())
      return this.matrixStrArray;
    return this.tableStrArray;
  },

  allCellsSelected : function()
  {
    var allSelected = false;
    if (this.mTableInfo)
    {
      if (this.mTableInfo.rowsData)
      {
        allSelected = true;
        for (var ix = 0; allSelected && (ix < this.mTableInfo.rowsData.length); ++ix)
        {
          if (this.mTableInfo.rowsData[ix].mSelected != msiPropertiesObjectDataBase.Selected_SomeSelected)
            allSelected = false;
        }
      }
      if (allSelected && this.mTableInfo.colsData)
      {
        for (var jx = 0; allSelected && (jx < this.mTableInfo.colsData.length); ++jx)
        {
          if (this.mTableInfo.colsData[jx].mSelected != msiPropertiesObjectDataBase.Selected_SomeSelected)
            allSelected = false;
        }
      }
      else
        allSelected = false;
    }
    return allSelected;
  },

  examineTable : function(rangeArray)
  {
    var tableDims = msiGetEnclosingTableOrMatrixDimensions(this.mEditorElement, this.mTableElement);
    this.mTableInfo = msiGetRowAndColumnData(this.mTableElement, tableDims, this.mEditorElement);
    this.markCellsInRangeArray(rangeArray);
    this.mRowSelection = this.getSelectionForWholeRows(rangeArray);
    this.mColSelection = this.getSelectionForWholeColumns(rangeArray);
    this.mCell = this.getSingleSelectedCell();  //May, of course, be null
  },

//  contentTypeForTable : function(rangeArray)
//  {
//    if (!this.mTableElement || !this.mTableInfo)
//      this.initFromData(rangeArray);
////Now need to decide among:
////  ContentType_Table : 5,   //The selected text is a table.
////  ContentType_TableCell : 0x11,  //The selected text is a table cell.
////  ContentType_TableCellGroup : 0x12,  //The selected text is a group (block?) of table cells.
////  ContentType_TableRows : 0x14,  //The selected text consists of a group of cells spanning entire table rows. This may or may not be the same as a <tr>.
////  ContentType_TableColumns : 0x18, //The selected text consists of a group of cells spanning entire table columns.
//
//  },

  markRowSelected : function(tableData, nWhichRow, nSelectionType)
  {
    // if (!tableData.rowsData[nWhichRow])
    //   tableData.rowsData[nWhichRow] = new Object();
    // if (!tableData.rowsData[nWhichRow].mSelected)
    //   tableData.rowsData[nWhichRow].mSelected = nSelectionType;
    // else
    //   tableData.rowsData[nWhichRow].mSelected |= nSelectionType;
  },

  markColumnSelected : function(tableData, nWhichCol, nSelectionType)
  {
    // if (!tableData.colsData[nWhichCol])
    //   tableData.colsData[nWhichCol] = new Object();
    // if (!tableData.colsData[nWhichCol].mSelected)
    //   tableData.colsData[nWhichCol].mSelected = nSelectionType;
    // else
    //   tableData.colsData[nWhichCol].mSelected |= nSelectionType;
  },

  markCellSelected : function(aCell, tableData, nSelectionType)
  {
    var bDone = false;
    for (var ii = 0; !bDone && (ii < tableData.m_nRows); ++ii)
    {
      for (var jj = 0; !bDone && (jj < tableData.cellInfoArray[ii].length); ++jj)
      {
        if ( tableData.cellInfoArray[ii][jj] && tableData.cellInfoArray[ii][jj].mNode && (tableData.cellInfoArray[ii][jj].mNode == aCell) )
        {
          //Since we'll come first to the top left corner of an "extended" cell, we worry only about setting things to our right and below us.
          //  (Recall from above that we recorded negative "mContinuation" values for the cells not at the upper left.)
          for (var kk = 0; kk <= tableData.cellInfoArray[ii][jj].mRowContinuation; ++kk)
          {
            for (var ll = 0; ll <= tableData.cellInfoArray[ii][jj].mColContinuation; ++ll)
            {
              if (tableData.cellInfoArray[ii + kk][jj + ll].mSelected)
                tableData.cellInfoArray[ii + kk][jj + ll].mSelected |= nSelectionType;
              else
                tableData.cellInfoArray[ii + kk][jj + ll].mSelected = nSelectionType;
              this.markColumnSelected(tableData, jj + ll, nSelectionType);
            }
            this.markRowSelected(tableData, ii + kk, nSelectionType);
          }
          bDone = true;
        }
      }
    }
    return bDone;
  },

  markCellsInNode : function(aNode, aRange, tableInfo)
  {
    var startPos = 0;
    var endPos = msiNavigationUtils.lastOffset(aNode);
    if (aRange)
    {
      if (aRange.startContainer == aNode)
        startPos = aRange.startOffset;
      if (aRange.endContainer == aNode)
        endPos = aRange.endOffset;
    }

    switch( msiGetBaseNodeName(aNode) )
    {
      case "mtd":
      case "td":
      case "th":
        if (!aRange)
          this.markCellSelected(aNode, tableInfo, msiPropertiesObjectDataBase.Selected_SomeSelected);
        else if (aRange.collapsed)
          this.markCellSelected(aNode, tableInfo, msiPropertiesObjectDataBase.Selected_MixedSelection);  //"3" means the cell is partially selected
        else if (msiNavigationUtils.nodeHasContentBeforeRangeStart(aRange, aNode) || msiNavigationUtils.nodeHasContentAfterRangeEnd(aRange, aNode))
          this.markCellSelected(aNode, tableInfo, msiPropertiesObjectDataBase.Selected_MixedSelection);  //"3" means the cell is partially selected
        else
          this.markCellSelected(aNode, tableInfo, msiPropertiesObjectDataBase.Selected_SomeSelection);
      break;

      case "mtr":
      case "mlabeledtr":
      case "tr":
      case "thead":
      case "tfoot":
      case "tbody":
      case "table":
      case "mtable":
        for (var ix = startPos; ix < endPos; ++ix)
        {
          if (aNode.childNodes[ix])
            this.markCellsInNode(aNode.childNodes[ix], null, tableInfo);
        }
      break;

      default:
      break;
    }
  },

  checkRowSelection : function(tableData)
  {
    var bAllCellsSelected = true;
    for (var ii = 0; ii < tableData.rowsData.length; ++ii)
    {
      if (tableData.rowsData[ii] && ("mSelected" in tableData.rowsData[ii]) && (tableData.rowsData[ii].mSelected == msiPropertiesObjectDataBase.Selected_SomeSelected) )
      {
        bAllCellsSelected = true;
        for (var jj = 0; bAllCellsSelected && jj < tableData.m_nRows; ++jj)
        {
          bAllCellsSelected = ( tableData.cellInfoArray[ii][jj] && ("mSelected" in tableData.cellInfoArray[ii][jj]) && (tableData.cellInfoArray[ii][jj].mSelected == msiPropertiesObjectDataBase.Selected_SomeSelected) );
        }
        if (!bAllCellsSelected)
          tableData.rowsData[ii].mSelected |= msiPropertiesObjectDataBase.Selected_SomeUnselected;
      }
      if (!tableData.rowsData[ii] || !("mSelected" in tableData.rowsData[ii]) || (tableData.rowsData[ii].mSelected != msiPropertiesObjectDataBase.Selected_SomeSelected) )
        this.markRowSelected(tableData, ii, msiPropertiesObjectDataBase.Selected_SomeUnselected);
    }
  },

  checkColumnSelection : function(tableData)
  {
    var bAllCellsSelected = true;
    for (var jj = 0; jj < tableData.colsData.length; ++jj)
    {
      if (tableData.colsData[jj] && ("mSelected" in tableData.colsData[jj]) && (tableData.colsData[jj].mSelected == msiPropertiesObjectDataBase.Selected_SomeSelected) )
      {
        bAllCellsSelected = true;
        for (var ii = 0; bAllCellsSelected && ii < tableData.m_nRows; ++ii)
        {
          bAllCellsSelected = ( tableData.cellInfoArray[ii][jj] && ("mSelected" in tableData.cellInfoArray[ii][jj]) && (tableData.cellInfoArray[ii][jj].mSelected == msiPropertiesObjectDataBase.Selected_SomeSelected) );
        }
        if (!bAllCellsSelected)
          tableData.colsData[jj].mSelected |= msiPropertiesObjectDataBase.Selected_SomeUnselected;
      }
      if (!tableData.colsData[jj] || !("mSelected" in tableData.colsData[jj]) || (tableData.colsData[jj].mSelected != msiPropertiesObjectDataBase.Selected_SomeSelected) )
        this.markColumnSelected(tableData, jj, msiPropertiesObjectDataBase.Selected_SomeUnselected);
    }
  },

  markCellsInRange : function(aRange, tableInfo)
  {
    var testRange = this.normalizeRange(aRange);
    var parent = testRange.commonAncestorContainer;


    while (testRange.startContainer != parent)
    {
      this.markCellsInNode(testRange.startContainer, testRange, this.mTableInfo);
      testRange.setStartAfter(testRange.startContainer);
    }
    while (testRange.endContainer != parent)
    {
      this.markCellsInNode(testRange.endContainer, testRange, this.mTableInfo);
      testRange.setEndBefore(testRange.endContainer);
    }

    if (testRange.startContainer == testRange.endContainer)
      this.markCellsInNode(testRange.startContainer, testRange, this.mTableInfo);
  },

  markCellsInRangeArray : function(rangeArray)
  {
    //For starters, we'll assume (as does code elsewhere) that the array of ranges in a Selection is always document-ordered.
    //  This should be tested, however. (And we're also assuming that any array of ranges passed to this function came from an
    //  actual selection in the document.)
    for (var ii = 0; ii < rangeArray.length; ++ii)
    {
      this.markCellsInRange(rangeArray[ii], ii);
    }

    this.checkRowSelection(this.mTableInfo);
    this.checkColumnSelection(this.mTableInfo);
  },

  //A return of null from this function means the existing selection contains whole rows. Otherwise, it's an array of Ranges.
  //A return of null from this function SHOULD mean that the entire table gets selected if we take whole rows.
  getSelectionForWholeRows : function(rangeArray)
  {
    var retRangeArray = [];
    var nRows = this.mTableInfo.m_nRows;
    var nCols = this.mTableInfo.m_nCols;
    var addRowsArray = [];
    var extraRowsArray = [];
    var bNeedExpandSel = false;
    var tableElement = msiGetContainingTableOrMatrix(rangeArray[0].startContainer);

    function checkARow(nWhichRow, extraRowArray, tableInfo)
    {
      var bAddRow = false;
      var bWasSelected = true;
      var newExtraRows = [];
      var nExtraRow = -1;

      if (tableInfo.rowsData[nWhichRow] && tableInfo.rowsData[nWhichRow].mSelected)
      {
        bAddRow = ( (tableInfo.rowsData[nWhichRow].mSelected & msiPropertiesObjectDataBase.Selected_SomeSelected) != 0 );
        if (bAddRow)
          bWasSelected = (tableInfo.rowsData[nWhichRow].mSelected == msiPropertiesObjectDataBase.Selected_SomeSelected);
      }

      if (bAddRow)
      {
        for (var ix = tableInfo.rowsData[nWhichRow].firstRow; ix <= tableInfo.rowsData[nWhichRow].lastRow; ++ix)
        {
//        extraRowArray = extraRowArray.concat(newExtraRows);
          if (extraRowArray.indexOf(ix) < 0)
            extraRowArray.push(ix);
        }
      }

      //Finally, we only want to return TRUE if we're actually changing the selection.
      bAddRow = bAddRow && !bWasSelected;
      return bAddRow;
    }

    function getExtraRows( tableData, foundRows, rowsToCheck )
    {
      var newExtraRows = [];
      for (var ii = 0; ii < rowsToCheck.length; ++ii)
      {
        if (tableData.rowsData[rowsToCheck[ii]] && tableData.rowsData[rowsToCheck[ii]].firstRow)
        {
          for (var jj = tableData.rowsData[rowsToCheck[ii]].firstRow; jj <= tableData.rowsData[rowsToCheck[ii]].lastRow; ++jj)
          {
            if (foundRows.indexOf(jj) < 0)
              newExtraRows.push(jj);
          }
        }
      }
      return newExtraRows;
    }

    function findRowParent(aNode)
    {
      var retParent = null;
      for (var aParent = aNode; !retParent && aParent; aParent = aParent.parentNode)
      {
        switch(msiGetBaseNodeName(aParent))
        {
          case "mtr":
          case "mlabeledtr":
          case "tr":
            return aParent;
          break;

          case "thead":
          case "tfoot":
          case "tbody":
          case "table":
          case "mtable":
            return null;
          break;
        }
      }
      return null;
    }

    function addRowsToRangeList(rangeList, tableInfo, startingRow, endingRow)
    {
      var newRange = tableElement.ownerDocument.createRange();
      var firstCellData =tableInfo.cellInfoArray[startingRow][0];
      var nLastCell = tableInfo.m_nCols;
      if ("lastNonemptyCell" in tableInfo.rowsData[endingRow])
        nLastCell = tableInfo.rowsData[endingRow].lastNonemptyCell;
      else
      {
        while (!tableInfo.cellInfoArray[endingRow][nLastCell] || !tableInfo.cellInfoArray[endingRow][nLastCell].mNode)
          --nLastCell;
      }
      var lastCellData = tableInfo.cellInfoArray[endingRow][nLastCell];
      var startRowNode = null;
      var endRowNode = null;

      if (firstCellData && lastCellData)
      {
        startRowNode = findRowParent(firstCellData.mNode);
        endRowNode = findRowParent(lastCellData.mNode);
        if (!startRowNode)
          startRowNode = firstCellData.mNode;
        if (!endRowNode)
          endRowNode = lastCellData.mNode;
      }

      if (startRowNode == endRowNode)
      {
        newRange.selectNode(startRowNode);
      }
      else
      {
        newRange.setStartBefore(startRowNode);
        newRange.setEndAfter(endRowNode);
      }
      rangeList.push(newRange);
    }

    for (var ix = 0; ix < nRows; ++ix)
      bNeedExpandSel = checkARow(ix, addRowsArray, this.mTableInfo) || bNeedExpandSel;
    addRowsArray.sort( function(a,b) {return (a-b);} );

    extraRowsArray = extraRowsArray.concat(addRowsArray);
    do
    {
      extraRowsArray = getExtraRows(this.mTableInfo, extraRowsArray, addRowsArray);
      msiMergeArrayIntoArray( addRowsArray, extraRowsArray, function(a,b) {return (a-b);} );
    } while (extraRowsArray.length > 0);  //since this can only keep iterating if it's finding new rows, it has to terminate after nRows iterations at most
    this.mRowSelectionArray = addRowsArray;

    if (addRowsArray.length != nRows)  //Not all rows are to be selected!
    {
//      if (addRowsArray.length == nRows)  //all rows are to be selected! Has to be the whole table
//      {
//        var newRange = tableElement.ownerDocument.createRange();
//        newRange.selectNode(tableElement);
//        retRangeArray.push(newRange);
//      }
//      else
      if (bNeedExpandSel)
      {
        var startRow = addRowsArray[0];
        var endRow = addRowsArray[0];
        for (var ii = 1; ii < addRowsArray.length; ++ii)
        {
          if (addRowsArray[ii] == endRow + 1)
            ++endRow;
          else
          {
            addRowsToRangeList(retRangeArray, this.mTableInfo, startRow, endRow);
            startRow = endRow = addRowsArray[ii];
          }
        }
        //Then there's one set left over to add after the loop:
        addRowsToRangeList(retRangeArray, this.mTableInfo, startRow, endRow);
      }
      else
        retRangeArray = rangeArray;
//      {   //just copy the existing rangeArray
//        for var ii = 0; ii < rangeArray.length; ++ii)
//          retRangeArray.push( rangeArray[ii].cloneRange() );
//      }
    }
    else  //all rows are to be selected! Has to be the whole table - return nothing
      retRangeArray = null;

    return retRangeArray;
  },

  //A return of null from this function means the existing selection contains whole columns. Otherwise, it's an array of Ranges.
  getSelectionForWholeColumns : function(rangeArray)
  {
    var retRangeArray = [];
    var nRows = this.mTableInfo.m_nRows;
    var nCols = this.mTableInfo.m_nCols;
    var addColsArray = [];
    var extraColsArray = [];
    var bNeedExpandSel = false;
    var tableElement = msiGetContainingTableOrMatrix(rangeArray[0].startContainer);

    function checkAColumn(nWhichCol, extraColArray, tableInfo)
    {
      var bAddCol = false;
      var bWasSelected = true;
      var newExtraCols = [];
      var nExtraCol = -1;

      if (tableInfo.colsData[nWhichCol] && tableInfo.colsData[nWhichCol].mSelected)
      {
        bAddCol = ( (tableInfo.colsData[nWhichCol].mSelected & msiPropertiesObjectDataBase.Selected_SomeSelected) != 0 );
        if (bAddCol)
          bWasSelected = (tableInfo.colsData[nWhichCol].mSelected == msiPropertiesObjectDataBase.Selected_SomeSelected);
      }

      if (bAddCol)
      {
        for (var ix = tableInfo.colsData[nWhichCol].firstCol; ix <= tableInfo.colsData[nWhichCol].lastCol; ++ix)
        {
//        extraColArray = extraColArray.concat(newExtraCols);
          if (extraColArray.indexOf(ix) < 0)
            extraColArray.push(ix);
        }
      }

      //Finally, we only want to return TRUE if we're actually changing the selection.
      bAddCol = bAddCol && !bWasSelected;
      return bAddCol;
    }

    function addColsToRangeList(rangeList, tableInfo, startingCol, endingCol)
    {
      //added by BBM
      return;
      var newRange = null;
      for (var ix = 0; ix < tableInfo.m_nRows; ++ix)
      {
        newRange = tableElement.ownerDocument.createRange();
        if (startingCol == endingCol)
          newRange.selectNode(tableInfo.cellInfoArray[ix][startingCol].mNode);
        else
        {
          newRange.setStartBefore(tableInfo.cellInfoArray[ix][startingCol].mNode);
          newRange.setEndAfter(tableInfo.cellInfoArray[ix][endingCol].mNode);
        }
        rangeList.push(newRange);
      }
    }

    function getExtraCols( tableData, foundCols, colsToCheck )
    {
      var newExtraCols = [];
      for (var ii = 0; ii < colsToCheck.length; ++ii)
      {
        if (tableData.colsData[colsToCheck[ii]] && tableData.colsData[colsToCheck[ii]].firstCol)
        {
          for (var jj = tableData.colsData[colsToCheck[ii]].firstCol; jj <= tableData.colsData[colsToCheck[ii]].lastCol; ++jj)
          {
            if (foundCols.indexOf(jj) < 0)
              newExtraCols.push(jj);
          }
        }
      }
      return newExtraCols;
    }

    for (var jx = 0; jx < nCols; ++jx)
      bNeedExpandSel = checkAColumn(jx, addColsArray, this.mTableInfo) || bNeedExpandSel;
    addColsArray.sort( function(a,b) {return (a-b);} );

    extraColsArray = extraColsArray.concat(addColsArray);
    do
    {
      extraColsArray = getExtraCols(this.mTableInfo, extraColsArray, addColsArray);
      msiMergeArrayIntoArray( addColsArray, extraColsArray, function(a,b) {return (a-b);} );
    } while (extraColsArray.length > 0);  //since this can only keep iterating if it's finding new rows, it has to terminate after nRows iterations at most


//    if (bNeedExpandSel)
    this.mColSelectionArray = addColsArray;
    if (addColsArray.length != nCols)  //Not all columns are to be selected!
    {
//      if (addColsArray.length == nCols)  //all columns are to be selected! Has to be the whole table
//      {
//        var newRange = tableElement.ownerDocument.createRange();
//        newRange.selectNode(tableElement);
//        retRangeArray.push(newRange);
//      }
//      else
      if (bNeedExpandSel)
      {
        var startCol = addColsArray[0];
        var endCol = addColsArray[0];
        for (var ii = 1; ii < addColsArray.length; ++ii)
        {
          if (addColsArray[ii] == endCol + 1)
            ++endCol;
          else
          {
            addColsToRangeList(retRangeArray, this.mTableInfo, startCol, endCol);
            startCol = endCol = addColsArray[ii];
          }
        }
        //Then there's one set left over to add after the loop:
        addColsToRangeList(retRangeArray, this.mTableInfo, startCol, endCol);
      }
      else
        retRangeArray = rangeArray;
//      {
//        for (var jj = 0; jj < rangeArray.length; ++jj)
//          retRangeArray.push( rangeArray[jj].cloneRange() );
//      }
    }
    else  //all columns are to be selected! Has to be the whole table
      retRangeArray = null;

    return retRangeArray;
  },

  getSingleSelectedCell : function()
  {
    var retCell = null;
    var bDone = false;
    for (var ix = 0; !bDone && (ix < this.mTableInfo.m_nRows); ++ix)
    {
      for (var jx = 0; !bDone && (jx < this.mTableInfo.m_nCols); ++jx)
      {
        if (this.mTableInfo.cellInfoArray[ix][jx] && this.mTableInfo.cellInfoArray[ix][jx].mSelected)
        {
          if (retCell && (retCell != this.mTableInfo.cellInfoArray[ix][jx].mNode))
          {
            bDone = true;
            retCell = null;  //There isn't just a single cell to report!
          }
          else
            retCell = this.mTableInfo.cellInfoArray[ix][jx].mNode;
        }
      }
    }
    return retCell;
  }

};

msiTablePropertiesObjectData.prototype.__proto__ = msiPropertiesObjectDataBase;

function msiEquationPropertiesObjectData()
{
  this.mnRows = 1;
  this.mCurRow = 1;
  this.mDisplay = null;
  this.mTableElement = null;
  this.mWholeArrayMarker = null;
  this.mbAlignmentEnabled = false;
  this.mAlignment = null;
  this.mbSubEqnNumbersEnabled = false;
  this.mbSubEqnContinuation = false;
  this.mRowData = [];
}

msiEquationPropertiesObjectData.prototype =
{

//Interface:
  initFromNode : function(aNode, editorElement)
  {
    this.setEditorElement(editorElement);
    this.mStartNode = aNode;
    switch(msiGetBaseNodeName(aNode))
    {
      case "mtd":
      case "mtr":
      case "mlabeledtr":
        this.mTableElement = msiGetContainingTableOrMatrix(aNode);
        this.mDisplay = msiNavigationUtils.getEnclosingDisplay(aNode);
      break;

      case "mtable":
        this.mTableElement = aNode;
        this.mDisplay = msiNavigationUtils.getEnclosingDisplay(aNode);
      break;

      case "msidisplay":
        this.mDisplay = aNode;
      break;

      case "math":
        this.mDisplay = msiNavigationUtils.getEnclosingDisplay(aNode);
      break;
    }
    var aRange = aNode.ownerDocument.createRange();
    //Is this right????? Yes - if we're being initialized from a node, it should be treated as selected for the purposes of finding containing rows and columns...
    aRange.setEndAfter(aNode);
    aRange.setStartBefore(aNode);
    this.examineEqnArray( [aRange] );
    this.finishInit();
    this.setStrings();
  },

  initFromSelection : function(aSelection, editorElement)
  {
    this.setEditorElement(editorElement);
    this.mSelection = [];
    for (var ix = 0; ix < aSelection.rangeCount; ++ix)
      this.mSelection.push( aSelection.getRangeAt(ix).cloneRange() );
    this.mTableElement = msiGetContainingTableOrMatrix(this.mSelection[0].startContainer);
    this.mDisplay = msiNavigationUtils.getEnclosingDisplay(this.mTableElement);
    this.examineEqnArray(this.mSelection);
    this.finishInit();
    this.setStrings();
  },

  //This function should only be called after we've done all our work - and thus things SHOULD be readily available...
  doSelectItems : function(menuLabel)
  {
    var editorElement = this.mEditorElement;
    if (!editorElement)
      this.mEditorElement = editorElement = msiGetActiveEditorElement();
    var theEditor = msiGetEditor(editorElement);

    if (this.mDisplay)
    {
      theEditor.selectElement(this.mDisplay);
      return;
    }
    if (aRangeArray && (aRangeArray.length > 0))
    {
      theEditor.selection.removeAllRanges();
      for (var ii = 0; ii < aRangeArray.length; ++ii)
        theEditor.selection.addRange(aRangeArray[ii]);
    }
    else
      dump("Trouble in msiEditor.js, in msiPropertiesObject.doSelectItems - we don't have a node, range, or range array to select!\n");
  },

  getReferenceNode : function()
  {
    return this.mDisplay;
  },

  getNode : function()
  {
    return this.mDisplay;
  },

  getTableNode : function()
  {
    return this.mTableElement;
  },

  getRowNode : function(nWhichRow)
  {
    if (nWhichRow < this.mnRows)
      return this.mRowData[nWhichRow].rowNode;
  },

  getRange : function()
  {
    if (this.mSelection && this.mSelection.length == 1)
      return this.mSelection[0];
    return null;
  },

  getRangeArray : function()
  {
    if (this.mSelection && this.mSelection.length > 1)
      return this.mSelection;
    return null;
  },

  isLineNumbered : function(nLine)
  {
    if (nLine < this.mnRows)
      return (this.mRowData[nLine].labelType != "numberingNone");
    return false;
  },

  getCustomLabelForLine : function(nLine) //Does this return a DOMFragment or just text????
  {
    if (nLine < this.mnRows)
      return this.mRowData[nLine].customLabel;
    return null;
  },

  setCustomLabelForLine : function(nLine, theLabel) //Does this hold a DOMFragment or just text????
  {
    if (nLine > this.mnRows)
    {
      dump("Problem in msiEditor.js, msiEquationPropertiesObjectData.setCustomLabelForLine called with invalid line number [" + nLine + "]\n");
      return;
    }
    if (theLabel != null && theLabel.length)
    {
      this.mRowData[nLine].labelType = "numberingCustom";
      this.mRowData[nLine].customLabel = theLabel;
    }
    else
    {
      dump("In msiEditor.js, msiEquationPropertiesObjectData.setCustomLabelForLine, called for line [" + nLine + "] with null or empty label; setting to numberingAuto\n");
      this.mRowData[nLine].labelType = "numberingAuto";
      this.mRowData[nLine].customLabel = null;
    }
  },

  getMarkerForLine : function(nLine)
  {
    if (nLine < this.mnRows)
      return this.mRowData[nLine].marker;
    return null;
  },

  thisLine : function()
  {
    return this.mCurRow;
  },

  numberLines : function()
  {
    return this.mnRows;
  },

  getMarkerForWholeArray : function()
  {
    return this.mWholeArrayMarker;
  },

  subEquationNumbersEnabled : function()
  {
    return this.mbSubEqnNumbersEnabled;
  },

  subEquationContinuation : function()
  {
    return this.mbSubEqnContinuation;
  },

  getAlignment : function()
  {
    return this.mAlignment;  //One of "alignStandard", "alignSingleEqn", "alignCentered"
  },

  getAlignmentEnabled : function()
  {
    return this.mbAlignmentEnabled;
  },

  getSuppressAnnotation : function(nLine)
  {
    if (nLine < this.mnRows)
      return this.mRowData[nLine].mbSuppressAnnotation;
    return false;
  },

  getSpaceAfterLine : function(nLine)
  {
    if (nLine < this.mnRows)
      return this.mRowData[nLine].spaceAfterLine;
    return null;
  },


//Implementation (non-interface) methods:
  setStrings : function()
  {
    this.menuStr = msiFormatPropertiesMenuString( "EquationArray" );
    this.commandStr = "cmd_MSIreviseEqnArrayCommand";
  },

  setRows : function(nRows)
  {
    this.mnRows = nRows;
    this.mRowData.splice(0, this.mRowData.length);  //empty it out
    for (var jj = 0; jj < nRows; ++jj)    //We'll just stick in the right number of objects to be filled in later
    {
      this.mRowData.push( {rowNode : null, labelType : "numberingAuto", marker : null, customLabel : null, mbSuppressAnnotation : false, spaceAfterLine : "0.0pt"} );
    }
  },

  examineEqnArray : function(rangeArray)
  {
    var foundNode = null, nextNode = null;
    var foundCell = null;
    var foundRow = null;
    var theAttr, matchArray, spacingArray;
    var paddingBottomRE = /padding-bottom\:\s*([^;^}]+)/;
    var theEditor, mathmlEditor;

    if (!this.mDisplay)
    {
      dump("Problem in msiEditor.js, msiEquationPropertiesObjectData.examineEqnArray - this.mDisplay isn't set!\n");
      if (this.mNode)
        this.mDisplay = msiNavigationUtils.getEnclosingDisplay(this.mNode);
      else if (this.mSelection && this.mSelection[0])
        this.mDisplay = msiNavigationUtils.getEnclosingDisplay(this.mSelection[0].startContainer);
      else
      {
        dump("  Unable to find a display node! Aborting...\n");
        return;
      }
    }
    if (this.mTableElement)
    {
      var tableDims = msiGetEnclosingTableOrMatrixDimensions(this.mEditorElement, this.mTableElement);
      if (tableDims.nCols == 1)
        this.setRows(tableDims.nRows);
      else
        this.mTableElement = null;
    }
    else
    {
      foundNode = this.mDisplay;
      while (foundNode && !this.mTableElement)
      {
        nextNode = msiNavigationUtils.getSingleSignificantChild(foundNode, true);
        if (nextNode)
        {
          if (msiGetBaseNodeName(nextNode) == "mtable")
          {
            var tableDims = msiGetEnclosingTableOrMatrixDimensions(this.mEditorElement, nextNode);
            if ((nextNode.getAttribute("type") == "eqnarray") || (tableDims.nCols == 1))
              this.mTableElement = nextNode;
            this.setRows(tableDims.nRows);
            break;  //Once we've examined a matrix element, it was our only shot whether it worked or not.
          }
          foundNode = nextNode;  //go further?
        }
        else  //getSingleSignificantChild returned null = multiple children? So there's no mtable that will work.
          break;
      }
    }
    if (this.mTableElement){
      theAttr = this.mTableElement.getAttribute("subtype");
      if (theAttr && theAttr.length){
         if (theAttr === "align" || theAttr === "align*") {
            this.mbAlignmentEnabled = true;
            this.mAlignment = "alignStandard";
         } else if (theAttr === "gather" || theAttr === "gather*"){
            this.mbAlignmentEnabled = true;
            this.mAlignment = "alignCentered";
         } else if (theAttr === "multline" || theAttr === "multline*"){
            this.mbAlignmentEnabled = true;
            this.mAlignment = "alignSingleEqn";
         }
      }
    }
      
    if (this.mDisplay)
    {
      if (!this.mTableElement)
        this.setRows(1);
      theEditor = msiGetEditor(this.mEditorElement);

      theAttr = this.mDisplay.getAttribute("marker");
      if (theAttr && theAttr.length)
        this.mWholeArrayMarker = theAttr;

      // theAttr = this.mDisplay.getAttribute("alignment");
      // if (theAttr && theAttr.length)
      // {
      //   this.mbAlignmentEnabled = true;
      //   this.mAlignment = theAttr;
      // }
  
      theAttr = this.mDisplay.getAttribute("subEquationNumbers");
      if (theAttr && theAttr == "true")
      {
        this.mbSubEqnNumbersEnabled = true;
        theAttr = this.mDisplay.getAttribute("subEquationContinuation");
        if (theAttr && theAttr == "true")
          this.mbSubEqnContinuation = true;
      }

      try  //Now need to get hold of the rows
      {
        mathmlEditor = theEditor.QueryInterface(Components.interfaces.msiIMathMLEditor);
        if (this.mTableElement)
        {
          theAttr = this.mTableElement.getAttribute("rowspacing");
          if (theAttr)
          {
            spacingArray = theAttr.split(/\s+/);
            var nFound = spacingArray.length;
            if (nFound > 0)
            {
              for (ix = nFound; ix < this.mnRows - 1; ++ix)  //Clone the last value for unspecified ones
                spacingArray.push(spacingArray[nFound - 1]);
            }
          }
        }

        for (var ix = 0; ix < this.mnRows; ++ix)
        {
          foundRow = foundCell = null;
          if (this.mTableElement)
            foundCell = mathmlEditor.getMatrixCellAt(this.mTableElement, ix+1, 1);
          else if (ix == 0)
            foundRow = foundCell = this.mDisplay;  //the attributes will be placed on the msidisplay element for a single displayed equation
          if (foundCell)
          {
            if (!foundRow)
              foundRow = msiNavigationUtils.getParentOfType(foundCell, "mtr");
            if (!foundRow)
              foundRow = foundCell;
            this.mRowData[ix].rowNode = foundRow;
//            if (this.mbSubEqnNumbersEnabled)
//            {
            theAttr = foundRow.getAttribute("numbering");
            if (theAttr && theAttr == "none")
              this.mRowData[ix].labelType = "numberingNone";
            else
              this.mRowData[ix].labelType = "numberingAuto";
            theAttr = foundRow.getAttribute("customLabel");
            if (theAttr && theAttr.length)
            {
              this.mRowData[ix].labelType = "numberingCustom";
              this.mRowData[ix].customLabel = theAttr;
              theAttr = foundRow.getAttribute("suppressAnnotation");
              if (theAttr && theAttr == "true")
                this.mRowData[ix].mbSuppressAnnotation = true;
            }
//            }
            theAttr = foundRow.getAttribute("marker");
            if (theAttr && theAttr.length)
              this.mRowData[ix].marker = theAttr;
            if (spacingArray && spacingArray.length > ix)
            {
              this.mRowData[ix].spaceAfterLine = spacingArray[ix];
            }
            else
            {
              theAttr = foundRow.getAttribute("style");
              if (theAttr)
              {
                matchArray = paddingBottomRE.exec(theAttr);
                if (matchArray && matchArray[1])
                  this.mRowData[ix].spaceAfterLine = matchArray[1];
                //NOTE that this is probably NOT RIGHT! There's presumably default padding in any case,
                //  and the padding reflected for the cells should probably subtract the default in order
                //  to get the spaceAfterLine. TO DO?  rwa 3-08-11
              }
            }
          }
        }
      }
      catch(exc) {dump("Exception in msiEquationPropertiesObjectData.examineEqnArray: [" + exc + "].\n");}
    }
    else
    {
      this.setRows(1);
//      this.mRows = [this.mDisplay];   //The display itself is the only equation
    }
  }
};

msiEquationPropertiesObjectData.prototype.__proto__ = msiPropertiesObjectDataBase;

//rwa//function msiInitObjectPropertiesMenuitem(editorElement, id)
//rwa//{
//rwa//  try {
//rwa//  var editor = msiGetEditor(editorElement);
//rwa//  var propertiesMenu = document.getElementById(id);
//rwa//  if (!editor || !propertiesMenu) return;
//rwa//  var popupMenu = propertiesMenu.getElementsByTagName("menupopup")[0];
//rwa//  if (!popupMenu) return;
//rwa//  // clean out popupMenu
//rwa//  var child;
//rwa//  while (child = popupMenu.firstChild) popupMenu.removeChild(child);
//rwa//
//rwa//  var parentTagString = editor.tagListManager.getParentTagList(",",false);
//rwa//  var parentTagArray = parentTagString.split(",");
//rwa//  var i;
//rwa//  var paL = parentTagArray.length;
//rwa//  var node = editor.getSelectionContainer();
//rwa//  // We have a problem when a node is selected.
//rwa//  if (editor.selection.focusNode == editor.selection.anchorNode &&
//rwa//    Math.abs(editor.selection.focusOffset-editor.selection.anchorOffset)==1)
//rwa//  {
//rwa//    if (editor.selection.focusNode.nodeType != 3) node =
//rwa//      editor.selection.focusNode.childNodes.item(Math.min(editor.selection.focusOffset,editor.selection.anchorOffset));
//rwa//  }
//rwa//  if (node.nodeType == 3) node = node.parentNode; //if text node, go to the parent
//rwa//  var tagclass;
//rwa//  var tag;
//rwa//  var label = GetString("TagPropertiesMenuLabel");
//rwa//  var newitem;
//rwa//  var count = 0;
//rwa//  for (i = 0; i < paL; i++)
//rwa//  {
//rwa//    tag = parentTagArray[i].replace(/ .*$/,"");
//rwa//    if (node.localName != tag)
//rwa//    {
//rwa//      dump("In msiInitObjectPropertiesMenuitem, assertion failure: tag array name = "
//rwa//        + tag + ", but node is " + node.localName + " and i is " +i+"\n");
//rwa//      continue;
//rwa//    }
//rwa//    // Now we go through the list of things that have properties dialogs.
//rwa//    tagclass = editor.tagListManager.getRealClassOfTag(tag, null);
//rwa//    var propsdata;
//rwa//    switch (tagclass)
//rwa//    {
//rwa//      case "texttag":
//rwa//        switch( tag )
//rwa//        {
//rwa//          case "otfont":
//rwa//            newitem = propertiesMenu.appendItem(label.replace("%tagname%","opentype font"));
//rwa//            newitem.setAttribute("oncommand","openOTFontDialog('otfont', event.target.node);");
//rwa//            newitem.node = node;
//rwa//            count++;
//rwa//            break;
//rwa//          case "fontcolor":
//rwa//            newitem = propertiesMenu.appendItem(label.replace("%tagname%","font color"));
//rwa//            newitem.setAttribute("oncommand","openFontColorDialog('fontcolor', event.target.node);");
//rwa//            newitem.node = node;
//rwa//            count++;
//rwa//            break;
//rwa//          case "fontsize":
//rwa//            newitem = propertiesMenu.appendItem(label.replace("%tagname%","font size"));
//rwa//            newitem.setAttribute("oncommand","openFontSizeDialog('fontsize', event.target.node);");
//rwa//            newitem.node = node;
//rwa//            count++;
//rwa//            break;
//rwa//          default: break;
//rwa//        }
//rwa//        break;
//rwa//      case "paratag":
//rwa//        newitem = propertiesMenu.appendItem(label.replace("%tagname%",tag));
//rwa//        newitem.setAttribute("oncommand", "openParaTagDialog('"+ tag + "',event.target.node);");
//rwa//        newitem.node = node;
//rwa//        count++;
//rwa//        break;
//rwa//      case "structtag":
//rwa//        newitem = propertiesMenu.appendItem(label.replace("%tagname%",tag));
//rwa//        newitem.setAttribute("oncommand", "openStructureTagDialog('"+ tag + "',event.target.node);");
//rwa//        newitem.node = node;
//rwa//        count++;
//rwa//        break;
//rwa//// currently no dialogs for list tags, environments, and front matter.
//rwa////      case "listtag":
//rwa////        break;
//rwa//      case "envtag":
//rwa//      {
//rwa//        switch( tag )
//rwa//        {
//rwa//          case "note":
//rwa//            newitem = propertiesMenu.appendItem(label.replace("%tagname%","note"));
//rwa//            newitem.setAttribute("oncommand", "msiNote(event.target.node, null);");
//rwa//            newitem.node = node;
//rwa//            count++;
//rwa//            break;
//rwa//          default: break;
//rwa//        }
//rwa//        break;
//rwa////      case "frontmtag":
//rwa////        break;
//rwa//      }
//rwa//      default: break;
//rwa//    }
//rwa//    if (tagclass == "" || tagclass=="othertag")
//rwa//    {
//rwa//      switch( tag )
//rwa//      {
//rwa//        case "msiframe":
//rwa//          break;
//rwa//        case "object":
//rwa//          newitem = propertiesMenu.appendItem(label.replace("%tagname%","embedded object"));
//rwa//          newitem.setAttribute("oncommand","openObjectTagDialog('object', event.target.node);");
//rwa//          newitem.node = node;
//rwa//          count++;
//rwa//          break;
//rwa//        case "graph":
//rwa//          newitem = propertiesMenu.appendItem(label.replace("%tagname%","function graph"));
//rwa//          newitem.setAttribute("oncommand","openGraphDialog('graph', event.target.node, event.target.editorElement);");
//rwa//          newitem.node = node;
//rwa//          newitem.editorElement = editorElement;
//rwa//          count++;
//rwa//          break;
//rwa//        case "texb":
//rwa//          newitem = propertiesMenu.appendItem(label.replace("%tagname%","TeX field"));
//rwa//          newitem.setAttribute("oncommand","openTeXButtonDialog('texb', event.target.node);");
//rwa//          newitem.node = node;
//rwa//          count++;
//rwa//        case "line":
//rwa//          break;
//rwa//        case "mtable":
//rwa//          break;
//rwa//        default:
//rwa//          propsdata = msiCreatePropertiesObjectDataFromNode(node, editorElement, false);
//rwa//          if (propsdata)
//rwa//          {
//rwa//            try
//rwa//            {
//rwa//              if (propsdata.menuStrings)
//rwa//              {
//rwa//                for (var k = 0; k < propsdata.menuStrings.length; k++)
//rwa//                {
//rwa//                  newitem = propertiesMenu.appendItem(propsdata.menuStrings[k]);
//rwa//                  if (propsdata.menuStrings && propsdata.menuStrings[k].length)
//rwa//                    newitem.setAttribute("command", propsdata.menuStrings[k]);
//rwa//                  else
//rwa//                    newitem.setAttribute("oncommand", propsdata.scriptStrings[k]);
//rwa//                  newitem.node = node;
//rwa//                  count++;
//rwa//                }
//rwa//              }
//rwa//              else
//rwa//              {
//rwa//                newitem = propertiesMenu.appendItem(propsdata.menuStr);
//rwa//                if (propsdata.commandStr && propsdata.commandStr.length)
//rwa////                  newitem.setAttribute("command", propsdata.commandStr);
//rwa//                  newitem.setAttribute("oncommand", "msiDoAPropertiesDialogFromMenu('" + propsdata.commandStr + "', this);");
//rwa//                else
//rwa//                  newitem.setAttribute("oncommand", propsdata.scriptStr);
//rwa//                newitem.refElement = node;
//rwa//                newitem.propertiesData = propsdata;
//rwa//                count++;
//rwa//              }
//rwa//            }
//rwa//            catch(e){}
//rwa//          }
//rwa//          break;
//rwa//      }
//rwa//    }
//rwa//    node = node.parentNode;
//rwa//  }
//rwa//  }
//rwa//  catch (e) {
//rwa//    dump(e.message+"\n");
//rwa//  }
//rwa//}



// Set strings and enable for the [Object] Properties item
// Note that we directly do the enabling instead of
//  using goSetCommandEnabled since we already have the menuitem
function msiInitObjectPropertiesMenuitem(editorElement, id)
{
  var name;

  try {
    if (!editorElement)
      editorElement = msiGetActiveEditorElement();
    var editor = msiGetEditor(editorElement);
    if ( (id != "propertiesMenu") && (id != "propertiesMenu_cm") )
      dump("Problem in msiEditor.js, msiInitObjectPropertiesMenuitem() - id passed in isn't correct (it's " + id + ").\n");

    var menuItem;
    var subMenu = document.getElementById(id);
    if (!subMenu) return null;

    var menuInfo = msiGetPropertiesMenuIDs(id);
    if (!menuInfo)
      return null;
    subMenu = document.getElementById(menuInfo.menuID);
    var parentPropertiesMenu = subMenu;
// --BBM ////  var itemID = "objectProperties";
// --BBM ////  var popupID = "propertiesMenuPopup";
    var subPopup = document.getElementById(menuInfo.popupID);
    var childItem;
    while (childItem = subPopup.firstChild)
      subPopup.removeChild(childItem);

    function copyCurrSelection(theEd)
    {
      var retSel = new Array();
      var len = theEd.selection.rangeCount;
      for (var ii = 0; ii < len; ++ii)
        retSel.push(theEd.selection.getRangeAt(ii).cloneRange());
      return retSel;
    }

    function addPropsMenuItem(propData, menuString, commandString, scriptString)
    {
      var item = document.createElementNS("http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul",
                                             "menuitem");

      if (commandString)
        item.setAttribute("oncommand", "msiPropMenuClearOrigSel('" + menuInfo.popupID + "'); msiDoAPropertiesDialogFromMenu('" + commandString + "', this);");
      else if (scriptString)
			{
			  item.setAttribute("oncommand", "msiPropMenuClearOrigSel('"+ menuInfo.popupID + "');" + scriptString);
			}
			if (propData.mNode && propData.mNode.setAttribute)
			{
				item.addEventListener("DOMMenuItemActive", function (event) {
					event.target.propertiesData.mNode.setAttribute("hilite","1");
				}, false);
				item.addEventListener("DOMMenuItemInactive", function (event) {
					event.target.propertiesData.mNode.removeAttribute("hilite");
				}, false);
			}
      item.setAttribute("label", menuString);
      item.refElement = propData.getReferenceNode();
      item.refEditor = editorElement;
      item.propertiesData = propData;
      if (!subPopup)
        createMorePropsSubmenu();
      if (!("origSelection" in subPopup))
        subPopup.origSelection = copyCurrSelection(editor);
      if (propData.isMatrix())
        AddInsertMatrixRowsColumnsMenuItems(parentPropertiesMenu, propData);

      return subPopup.appendChild(item);
    }

//    var menuItem = document.getElementById(menuInfo.itemID);
    var element;
//    var menuStr = GetString("AdvancedProperties");
    var menuStr;
    var propsData = null;
    var nextNode = null;
    var commandStr, scriptStr;

    if (msiIsEditingRenderedHTML(editorElement))
    {
      propsData = msiGetObjectDataForProperties(editorElement);
      if (propsData)
      {
        element = propsData.getReferenceNode();
        nextNode = propsData.getTopNode().parentNode;
      }
    }

    if (!propsData)
    {
      if (!element)
        element = editor.selection.getRangeAt(0).commonAncestorContainer;
      if (element)
        propsData = msiCreatePropertiesObjectDataFromNode(element, editorElement, true);
    }
    if (propsData)
    {
      subPopup.origSelection = copyCurrSelection(editor);
      menuStr = propsData.getMenuString(0);
      commandStr = propsData.getCommandString(0);
      scriptStr = propsData.getScriptString(0);
      if ( menuStr && menuStr.length && ((commandStr && commandStr.length) || (scriptStr || scriptStr.length)) )
        addPropsMenuItem(propsData, menuStr, commandStr, scriptStr);

//      menuStr = propsData.getMenuString(0);
//      if (!menuStr || !menuStr.length)
//      {
//        menuStr = GetString("AdvancedProperties");
//        menuItem.setAttribute("disabled","true");
//      }
//      else
//        menuItem.removeAttribute("disabled");
//      commandStr = propsData.getCommandString(0);
//      if (commandStr)
//      {
//        menuItem.setAttribute("oncommand", "msiPropMenuResetOrigSel('" + menuInfo.popupID + "'); msiDoAPropertiesDialogFromMenu('" + commandStr + "', this);");
//        subMenu.defaultItem = menuItem;
//        subMenu.commandStr = commandStr;
//      }
//      else
//      {
//        scriptStr = propsData.getScriptString(0);
//        if (propsData.scriptStr)
//          menuItem.setAttribute("oncommand", "msiPropMenuClearOrigSel('" + menuInfo.popupID + "');" + scriptStr);
//      }
//      menuItem.addEventListener("DOMMenuItemActive", msiPropertiesMenuItemHover, false);
////How to generate a reasonable ID? But does it need one?
//      menuItem.propertiesData = propsData;
//      menuItem.refElement = propsData.getReferenceNode();
//      menuItem.refEditor = propsData.mEditorElement;
//      if (propsData.isMatrix())
//        AddInsertMatrixRowsColumnsMenuItems(parentPropertiesMenu, propsData);
    }
//    else
//    {
//      // We show generic "Properties" string, but disable menu item
//      menuItem.setAttribute("disabled","true");
//    }
//    menuItem.setAttribute("label", menuStr);
//    menuItem.setAttribute("accesskey",GetString("ObjectPropertiesAccessKey"));

    if (!nextNode)
      nextNode = element.parentNode;
    var lastCoreNode = element;
    var lastPropsData = propsData;

//First check whether the current object has more than one properties string to put up:
    if (propsData)
    {
      menuStr = null;
      for (var iter = 1; propsData.hasReviseData(iter); ++iter)
      {
        menuStr = propsData.getMenuString(iter);
        commandStr = propsData.getCommandString(iter);
        scriptStr = propsData.getScriptString(iter);
        addPropsMenuItem(propsData, menuStr, commandStr, scriptStr);
        //addPropsMenuItem(propsData);
      }
    }

    while (nextNode != null)
    {
      propsData = msiCreatePropertiesObjectDataFromNode(nextNode, editorElement, true);
      if (propsData)
      {
        if (!lastPropsData || (propsData.getReferenceNode() != lastPropsData.getReferenceNode()))  //we've hit a really new object
        {
          if (!subMenu)
          {
            //Note! In this case, we've hit the first properties data with a coreElement differing from that of the
            //  original item, which is still being held in prevPropData. Thus we don't want to add an item now, but we
            //  are assured that an item will be needed. Just create the submenu and submenupop and get out.
            dump("In msiEditor.js, in msiInitObjectPropertiesMenuitem(), subMenu is currently null - should no longer happen!\n");
            createMorePropsSubmenu();
          }
          menuStr = null;
          for (var iter = 0; propsData.hasReviseData(iter); ++iter)
          {
            menuStr = propsData.getMenuString(iter);
            commandStr = propsData.getCommandString(iter);
            scriptStr = propsData.getScriptString(iter);
            addPropsMenuItem(propsData, menuStr, commandStr, scriptStr);
            //addPropsMenuItem(propsData);
          }
        }
        lastPropsData = propsData;
        nextNode = propsData.getTopNode().parentNode;
      }
      else
        nextNode = nextNode.parentNode;
    }
  }
  catch(exc) {dump("In msiInitObjectPropertiesMenuitem(), exception: " + exc);}
  return name;
}

function msiFormatPropertiesMenuString(objectStringID)
{
  var objStr = "";
  if (objectStringID)
    objStr = GetString(objectStringID);
  if (!objStr.length)
    return GetString("AdvancedProperties");
  return GetString("ObjectProperties").replace(/%obj%/,objStr);
}

function msiCleanUpPropertiesMenu(event, theMenu, menuID)
{
  var bRightOne = (event.target == theMenu) || ( (event.target.nodeName == "menupopup") && (event.target.parentNode == theMenu) );
  if (!bRightOne)
    return;

  RemoveInsertMatrixRowsColumnsMenuItems(theMenu);

  var menuInfo = msiGetPropertiesMenuIDs(menuID);
//  var subPropsMenu = document.getElementById("morePropertiesMenu");
  var subPropsMenu = document.getElementById(menuInfo.popupID);
  var itemID;
  if ( subPropsMenu && (subPropsMenu.childNodes.length > 1) )
  {
    for (var ix = subPropsMenu.childNodes.length - 1; ix>=0; --ix)
    {
      itemID = subPropsMenu.childNodes[ix].id;
      if (!itemID || !itemID.length)
        itemID = subPropsMenu.childNodes[ix].getAttribute("id");
      if (!itemID || itemID != menuInfo.itemID)
        subPropsMenu.removeChild(subPropsMenu.childNodes[ix]);
    }
//    subPropsMenu.parentNode.removeChild(subPropsMenu);
  }
}

function msiPropMenuCloseup(event, thePopupMenu)
{
  if (event.target != thePopupMenu)
    return;
  msiPropMenuResetOrigSel(thePopupMenu.id);
}

function AddInsertMatrixRowsColumnsMenuItems(parentPropertiesMenu, propsData)
{
//  var editPopup = parentPropertiesMenu.parentNode;
//  var bIsContextMenu = false;
//  if (editPopup.id.substr(-3) == "_cm")
//    bIsContextMenu = true;
//  var sepID = "InsertMatrixRowColSeparator";
//	var insertMatrixID = "matrixInsert";
//	var selectMatrixID = "matrixSelect";
//	var deleteMatrixID = "matrixDelete";
//
////  var rowID = "InsertMatrixRows";
////  var colID = "InsertMatrixColumns";
//  if (bIsContextMenu)
//  {
//		insertMatrixID += "_cm";
//		selectMatrixID += "_cm";
//		deleteMatrixID += "_cm";
//  }
//  var menuItems = editPopup.getElementsByAttribute("id", rowID);
//  if (menuItems && (menuItems.length > 0))
//    return;  //Already done, don't do again
////  else
////  {
////    dump("In AddInsertMatrixRowsColumnsMenuItems, item with rowID [" + rowID + "] not; adding!\n");
////    dump( msiKludgeLogNodeContentsAndAttributes(editPopup, ["tableEdit"], "  Looked inside node", true, ["id", "label"], false) );
////  }
//  var xulNS = "http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul";
//  var sepItem = document.createElementNS(xulNS, "menuseparator");
//	var insertMatrixItem = document.createElementNS(xulNS, "menuitem");
//	var selectMatrixItem = document.createElementNS(xulNS, "menuitem");
//	var deleteMatrixItem = document.createElementNS(xulNS, "menuitem");
////  var rowItem = document.createElementNS(xulNS, "menuitem");
////  var colItem = document.createElementNS(xulNS, "menuitem");
//
//  sepItem.setAttribute("id", sepID);
//  insertMatrixItem.setAttribute("oncommand", "msiDoAPropertiesDialogFromMenu('cmd_MSIaddMatrixRowsCmd', this);");
////  rowItem.addEventListener("DOMMenuItemActive", msiPropertiesMenuItemHover, false);
//  insertMatrixItem.setAttribute("id", rowID);
//  insertMatrixItem.setAttribute("label", GetString("InsertMatrixRows"));
//  insertMatrixItem.setAttribute("accesskey", GetString("InsertMatrixRowsAccessKey"));
//  selectMatrixItem.setAttribute("oncommand", "msiDoAPropertiesDialogFromMenu('cmd_MSIaddMatrixColumnsCmd', this);");
////  colItem.addEventListener("DOMMenuItemActive", msiPropertiesMenuItemHover, false);
//  selectMatrixItem.setAttribute("id", colID);
//  selectMatrixItem.setAttribute("label", GetString("InsertMatrixColumns"));
//  selectMatrixItem.setAttribute("accesskey", GetString("InsertMatrixColumnsAccessKey"));
//
//  selectMatrixItem.refElement = rowItem.refElement = propsData.getReferenceNode();
//  selectMatrixItem.refEditor = rowItem.refEditor = propsData.mEditorElement;
//  colItem.propertiesData = rowItem.propertiesData = propsData;
//
//  editPopup.appendChild(sepItem);
//  editPopup.appendChild(rowItem);
//  editPopup.appendChild(colItem);
}

function RemoveInsertMatrixRowsColumnsMenuItems(parentMenu)
{
  var bIsContextMenu = false;
  if (parentMenu.id.substr(-3) == "_cm")
    bIsContextMenu = true;
  var sepID = "InsertMatrixRowColSeparator";
  var rowID = "InsertMatrixRows";
  var colID = "InsertMatrixColumns";
  if (bIsContextMenu)
  {
    sepID += "_cm";
    rowID += "_cm";
    colID += "_cm";
  }
//  var theSep = document.getElementById(sepID);
  var sepItems = parentMenu.getElementsByAttribute("id", sepID);
  for (var ix = sepItems.length - 1; sepItems && (ix >= 0); --ix)
  {
    if (sepItems[ix].parentNode)
      sepItems[ix].parentNode.removeChild(sepItems[ix]);
  }
//  var theRowItem = document.getElementById(rowID);
  var rowItems = parentMenu.getElementsByAttribute("id", rowID);
  for (var ix = rowItems.length - 1; rowItems && (ix >= 0); --ix)
  {
    if (rowItems[ix].parentNode)
      rowItems[ix].parentNode.removeChild(rowItems[ix]);
  }
//  var theColItem = document.getElementById(colID);
  var colItems = parentMenu.getElementsByAttribute("id", colID);
  for (var ix = colItems.length - 1; colItems && (ix >= 0); --ix)
  {
    if (colItems[ix].parentNode)
      colItems[ix].parentNode.removeChild(colItems[ix]);
  }
}

//function msiPropMenuDefItemSelect(event, theItem)
//{
//  var menuInfo = msiGetPropertiesMenuIDs(theItem.id);
//  var popupMenu;
//  if (menuInfo)
//    msiPropMenuResetOrigSel(menuInfo.popupID);
//}

function msiPropMenuResetOrigSel(popupID)
{
  var thePopupMenu = document.getElementById(popupID);
  var theEditorElement = null;
  if (thePopupMenu.childNodes.length)
  {
    for (var ix = 0; (ix < thePopupMenu.childNodes.length) && !theEditorElement; ++ix)
    {
      if ("propertiesData" in thePopupMenu.childNodes[ix])
        theEditorElement = thePopupMenu.childNodes[ix].propertiesData.mEditorElement;
      if (!theEditorElement && ("refEditor" in thePopupMenu.childNodes[ix]))
        theEditorElement = thePopupMenu.childNodes[ix].refEditor;
    }
  }
  var theEditor = msiGetEditor(theEditorElement);
  if (thePopupMenu.origSelection && thePopupMenu.origSelection.length)
  {
    theEditor.selection.removeAllRanges();
    for (var ii = 0; ii < thePopupMenu.origSelection.length; ++ii)
      theEditor.selection.addRange(thePopupMenu.origSelection[ii]);
  }
}

function msiPropMenuClearOrigSel(popupID)
{
  var thePopup = null;
  if (popupID && popupID.length)
    thePopup = document.getElementById(popupID);
  if (thePopup && ("origSelection" in thePopup))
    delete thePopup.origSelection;
}

function msiPropertiesMenuItemHover(event)
{
  var menuItem = event.currentTarget;
  if (!menuItem)
    menuItem = event.target;
  if ( menuItem.propertiesData && ("doSelectItems" in menuItem.propertiesData) )
    menuItem.propertiesData.doSelectItems(menuItem.label);
  else if (menuItem.refElement && menuItem.refEditor)
    msiGetEditor(menuItem.refEditor).selectElement(menuItem.refElement);
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
function msiOnButtonUpdate(button, commmandID, invert)
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
    button.checked = invert?(!(state == "true")):state=="true";
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
      enabled = (tagNameObj.value == "td" || tagNameObj.value == "mtd");
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
        try {
          msiGoUpdateCommand(commandID, editorElement);
        }
        catch(e) {          
        }
      }
      // Directly set with the values calculated here
      else if (commandID == "cmd_DeleteTable" ||
               commandID == "cmd_NormalizeTable" ||
               commandID == "cmd_editTable" ||
      //       commandID == "cmd_TableOrCellColor" ||
               commandID == "cmd_SelectTable")
      {
        try {
            msiGoSetCommandEnabled(commandID, enabledIfTable, editorElement);
        }
        catch(e) {          
        }
      } 
      else 
      {
        try {
          msiGoSetCommandEnabled(commandID, enabled, editorElement);
        }
        catch(e) {          
        }
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

function msiIsInMatrixCell(editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  try {
    var editor = msiGetEditor(editorElement);
    var flags = editor.flags;
    return (msiIsHTMLEditor(editorElement) &&
            !(flags & nsIPlaintextEditor.eEditorReadonlyMask) &&
            msiIsEditingRenderedHTML(editorElement) &&
            null != editor.getElementOrParentByTagName("mtd", null));
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
function msiEditorInsertOrEditTable(insertAllowed, editorElement, command, commandHandler, reviseObjectData)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);
  if (!reviseObjectData && msiIsInTable(editorElement))
  {
    reviseObjectData = new msiTablePropertiesObjectData();
    reviseObjectData.initFromSelection(editor.selection, editorElement);
  }
  if (reviseObjectData != null)
  {
      // Edit properties of existing table
    var theData = {reviseCommand : command, reviseData : reviseObjectData};
    window.openDialog("chrome://prince/content/msiEdTableProps.xul", "_blank", "modal, chrome,resizable,close,titlebar,dependent", editorElement,
                                         command, commandHandler, theData);
//      window.openDialog("chrome://editor/content/EdTableProps.xul", "tableprops", "chrome,close,titlebar,modal", "","TablePanel");
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


  window.openDialog("chrome://prince/content/msiEdTableProps.xul", "inserttable", "modal, chrome,close,titlebar,resizable", "");
	msiGetEditor(editorElement).incrementModificationCount(1);
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
      window.openDialog("chrome://editor/content/EdTableProps.xul", "tableprops", "chrome,close,titlebar,modal,resizable", "", "CellPanel");
			msiGetEditor(editorElement).incrementModificationCount(1);
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
//      window.openDialog("chrome://editor/content/EdInsertChars.xul", "insertchars", "chrome,close,titlebar", "");
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
//  editorElement.mLastFocusNode = null;
  msiUpdateStructToolbar(editorElement, true);
}

function newCommandListener(element)
{
  return function() { return msiSelectFocusNodeAncestor(null, element); };
}

function newContextmenuListener(button, element)
{
  return function() { return InitStructBarContextMenu(button, element); };
}

function insertStructToolbarButton(tag, toolbar, realElement, theDocument)
{
  var button;
  var uiButton;
  var uiCommand;
  var editorElement = msiGetActiveEditorElement();
  if (tag==="math") {
    try {
      setMathTextToggle(editorElement, true);
    }
    catch(e)
    {
      msidump(e.message);
    }     
  }
  button = theDocument.createElementNS(XUL_NS, "toolbarbutton");
  button.setAttribute("label",   "<" + tag + ">");
  button.setAttribute("value",   tag);
  button.setAttribute("context", realElement?"structToolbarContext":"");
  button.className = "struct-button";

  toolbar.insertBefore(button, toolbar.firstChild);
  // update the tagbutton, if there is one.
//  uiButton = document.getElementById(tag+"Button");
//  if (uiButton) {
//    uiCommand = uiButton.getAttribute("observes");
//    uiCommand.setAttribute("active");
//  }
  return button;
}

function msiUpdateStructToolbar(editorElement, noopt)
// noopt means no optimization. It is true only when the cursor postion (node and offset) 
// has not changed, but the ancestors of node might have changed.
{
  const XUL_NS = "http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul";
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

  if (noopt == null && element == editorElement.mLastFocusNode &&
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
  var i;
  var propertyStack = [];

  // the theory here is that by following up the chain of parentNodes,
  // we will eventually get to the root <body> tag. But due to some bug,
  // there may be multiple <body> elements in the document.
  setMathTextToggle(editorElement, false);
  var cursorSetProps = editor.readCursorSetProps().split(";"); // these are tags set on the cursor
  var cursorClearedProps = editor.readCursorClearedProps().split(";"); // these are tags cleared at the cursor
  for (i = 0; i < cursorClearedProps.length; i++)
  {
    tmp = cursorClearedProps[i].split(",");
    cursorClearedProps[i] = tmp[0];
  }
  var property;
  var propertyStack;
  for (i = cursorSetProps.length - 1; i >= 0; i--)
  {
    property = cursorSetProps[i].split(",");
    if (property[0].length > 0) {
      insertStructToolbarButton(property[0], toolbar, false, theDocument);
      propertyStack.push(property[0]);
    }
  }
  do {
    tag = element.nodeName;
    if (cursorClearedProps.indexOf(tag) === -1) {
      button = insertStructToolbarButton(tag, toolbar, true, theDocument);
      propertyStack.push(tag);
      button.addEventListener("command", newCommandListener(element), false);

      button.addEventListener("contextmenu", newContextmenuListener(button, element), false);

      if (isFocusNode && oneElementSelected) {
        button.setAttribute("checked", "true");
        isFocusNode = false;
      }
    }
    tmp = element;
    element = element.parentNode;

  } while (element && (tmp != bodyElement) && (tag != "body"));
  setTagFieldContents(editor, propertyStack);
}

function setTagFieldContents(editor, propertyStack)  // probably should be renamed to setTagButtons
{
  var tagManager = editor.tagListManager;
  var str = propertyStack.pop();
  var klass;
  var textbox;
  var tt, pt, st, ft;
  tt = document.getElementById("TextTagSelections");
  pt = document.getElementById("ParaTagSelections");
  st = document.getElementById("StructTagSelections");
  ft = document.getElementById("FrontMTagSelections");
  if (tt) tt.value = "";
  if (pt) pt.value = "";
  if (st) st.value = "";
  if (ft) ft.value = "";
  try
  {
    document.getElementById("cmd_textBold").removeAttribute("checked");
    document.getElementById("cmd_textItalic").removeAttribute("checked");
    while (str && (str.length > 0)) {
      if (str === "bold") {
        document.getElementById("cmd_textBold").setAttribute("checked", "true");
      }
      else if (str === "italics") {
        document.getElementById("cmd_textItalic").setAttribute("checked", "true");
      }
      str = propertyStack.pop();
    }
  }
  catch(e) {
    dump(e);
  }
}

function msiSelectFocusNodeAncestor(editorElement, element, inner)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);
  if (editor) {
    if (inner)
    {
      editor.selection.collapse(element,0);
      editor.selection.extend(element, element.childNodes.length);
    }
    else
    {
      // if (element == msiGetBodyElement(editorElement))
      //   editor.selectAll();
      // else
        editor.selectElement(element);
    }
  }
  top.document.commandDispatcher.focusedWindow.focus();
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
  while ((result.node != null) && (result.node.nodeType != Node.ELEMENT_NODE))
    result.node = result.node.parentNode;

  // and make sure the element is not a special editor node like
  // the <br> we insert in blank lines
  // and don't select anonymous content !!! (fix for bug 190279)
  while ((result.node != null) && ( result.node.hasAttribute("_moz_editor_bogus_node") ||  editor.isAnonymousElement(result.node)))
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
{
  var elementName;
  try
  {
    if (!editorElement)
      editorElement = findEditorElementForDocument(element.ownerDocument);

    elementName = element.localName;
    if ((elementName == "object" || elementName == "embed") && !element.hasAttribute("msigraph"))
    { // if this is one of our graphics objects ...
      openObjectTagDialog(elementName, element, editorElement);
      elementName = element.parentNode.localName;
    }
    else if (elementName == "notewrapper")
    {
      element = element.getElementsByTagName("note")[0];
      var isDisplayed;
      if (element.hasAttribute("hide"))
        isDisplayed = (element.getAttribute("hide")=="false");
      else
      {
        var body = msiGetBodyElement(editorElement);
        isDisplayed = !(body.getAttribute("hideFootnotes") && body.getAttribute("hideFootnotes") == "true");
      }
      if (isDisplayed)
      {
        element.setAttribute("hide","true");
      }
      else
      {
        element.setAttribute("hide","false");
      }
    }
    else if (elementName == "table"||elementName=="thead"||elementName=="tr"||elementName=="td")
    {
      msiTable(element,editorElement);
    }
    else if (elementName == "note")
    {
      msiNote(element,editorElement);
    }
    else if (elementName == "a" && element.hasAttribute("key"))
    {
      msiGoDoCommand("cmd_marker", editorElement);
    }
    else if (elementName == "texb")
    {
      openTeXButtonDialog('texb', element);
    }
    else if (elementName == "msiframe" && element.parentNode.nodeName !== "graph")
    {
      msiFrame(editorElement, null, element);
    }
    else if (elementName == "rawTeX")
    {
      openOTFontDialog(elementName,element);
    }
    else if (elementName == "htmlfield")
    {
      openHTMLField(element);
    }
    else if (elementName == "fontcolor")
    {
      openFontColorDialog(elementName,element);
    }
    else if (elementName == "bibitem") 
    {
      msiGoDoCommand("cmd_reviseManualBibItemCmd");
    }
    else if (elementName == "bibtexbibliography")
    {
      msiGoDoCommand('cmd_reviseBibTeXBibliographyCmd');
    }
    else if (elementName == "fontsize")
    {
      openFontSizeDialog(elementName,element);
    }
    else if ((elementName == "img") || (elementName=="graph") || (elementName == "msiframe"))
    {
      var bIsGraph = (elementName !== "img");
      // if (!bIsGraph)
      // {
      //   for (var ix = 0; !bIsGraph && (ix < element.childNodes.length); ++ix)
      //   {
      //     if (element.childNodes[ix].tagName == "plotwrapper")
      //       bIsGraph = true;
      //   }
      // }
      if (bIsGraph)
      {
//        dump("In goDoPrinceCommand, bIsGraph is true.\n");
        var theWindow = window;
        if (!("graphClickEvent" in theWindow))
          theWindow = msiGetTopLevelWindow(window);
        theWindow.graphClickEvent(cmdstr, editorElement, element);
      }
//      else
//        dump("In goDoPrinceCommand, bIsGraph is false.\n");
    }
    else if ((element.localName == "object") && (element.getAttribute("msigraph") == "true"))
    {      dump ("SMR msiEditor.js got double click on msigraph object\n");
      var theWindow = window;
      if (!("graphClickEvent" in theWindow))
        theWindow = msiGetTopLevelWindow(window);
      // theWindow.graphObjectClickEvent(cmdstr,element, editorElement);
      theWindow.graphClickEvent(cmdstr, editorElement, element);
    }
    else
    {
//      dump("In goDoPrinceCommand, elementName is [" + elementName + "].\n");
      msiGoDoCommand(cmdstr, editorElement);
    }
  }
  catch(exc) {AlertWithTitle("Error in msiEditor.js", "Error in goDoPrinceCommand: " + exc);}
}

function msiSetGraphicFrameAttrsFromGraphic(imageObj, editor)
{
  var frameObj = msiNavigationUtils.getParentOfType(imageObj, "msiframe");
  if (!frameObj)
    return;

  var theUnits = imageObj.getAttribute("units");
  var unitHandler = new UnitHandler();
  unitHandler.initCurrentUnit(theUnits);
  var width = Number(imageObj.getAttribute("imageWidth"));
  var borderWidth = Number(imageObj.getAttribute("borderw"));
  if (!borderWidth || isNaN(borderWidth))
    borderWidth = 0;
  var paddingWidth = Number(imageObj.getAttribute("padding"));
  if (!paddingWidth || isNaN(paddingWidth))
    paddingWidth = 0;

  if (width && !isNaN(width))
  {
    width += 2 * borderWidth + 2 * paddingWidth;
    msiEditorEnsureElementAttribute(frameObj, "width", String(width), editor);
    msiEnsureElementCSSProperty(frameObj, "width", String(unitHandler.getValueAs(width, "px")), editor);
    msiEditorEnsureElementAttribute(frameObj, "units", theUnits, editor);
  }
  var height = Number(imageObj.getAttribute("imageHeight"));
  if (height && !isNaN(height))
  {
    height += 2 * borderWidth + 2 * paddingWidth;
    msiEditorEnsureElementAttribute(frameObj, "height", String(height), editor);
    msiEnsureElementCSSProperty(frameObj, "height", String(unitHandler.getValueAs(height, "px")), editor);
    msiEditorEnsureElementAttribute(frameObj, "units", theUnits, editor);
  }
  var rotation = imageObj.getAttribute("rotation");
  if (rotation && rotation.length)
    msiEditorEnsureElementAttribute(frameObj, "rotation", rotation, editor);
}



function msiDoUpdateCommands(eventStr, editorElement)
{
  var theWindow = msiGetTopLevelWindow();
  theWindow.updateCommands(eventStr);
  if (theWindow != window)
    window.updateCommands(eventStr);
}


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
      dump("An error occurred executing the "+command+" command: "+e.message+"\n");
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
};

// Shim for compatibility with existing code.
function msiGoDoCommand(command, editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  // editorElement && editorElement.focus();
	msiCommandUpdater.doCommand(command, editorElement);
}
function msiGoUpdateCommand(command, editorElement) { msiCommandUpdater.updateCommand(command, editorElement); }
function msiGoSetCommandEnabled(command, enabled, editorElement) { msiCommandUpdater.enableCommand(command, enabled, editorElement); }
function msiGoSetMenuValue(command, labelAttribute, editorElement) { msiCommandUpdater.setMenuValue(command, labelAttribute, editorElement); }
function msiGoSetAccessKey(command, valueAttribute, editorElement) { msiCommandUpdater.setAccessKey(command, valueAttribute, editorElement); }
function msiGoOnEvent(node, event) { msiCommandUpdater.onEvent(node, event); }

var msiDialogEditorContentFilterBase = 
{
  reject : 0,
  accept : 1,
  skip : 2,
  acceptAll : 3,
  mAtomService : Components.classes["@mozilla.org/atom-service;1"].getService(Components.interfaces.nsIAtomService),
  mXmlSerializer : new XMLSerializer(),
  mDOMUtils : Components.classes["@mozilla.org/inspector/dom-utils;1"].createInstance(Components.interfaces.inIDOMUtils),
  defaultParaTag : "para",

  dlgNodeFilter : function(aNode)
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
    if (aNode.nodeName == this.defaultParaTag)
    {
      if (this.mbAtFirst)
        return this.skip;
      else
        return this.accept;
    }
    switch(aNode.nodeName)
    {
      case "dialogbase":
      case "sw:dialogbase":
        return this.skip;
      break;
      case "br":
        if (aNode.hasAttribute("temp") && (aNode.getAttribute("temp")=="true") )
          return this.reject;
      break;
//      case "mi":
//        if (aNode.hasAttribute("tempinput") && (aNode.getAttribute("tempinput")=="true") )
//          return this.reject;
//      break;
    }
    return this.acceptAll;
    //We still need to fill in the tags for which we want to accept the tag but leave open the possibility of not accepting a child.
    //Examples may include field tags, list tags, etc.; the point being that one may occur as the parent of something like a
    //  sw:dialogbase paragraph. Not implemented that way at this point.  rwa, 8-
  },
  getXMLNodesForParent : function(newParent, parentNode)
  {
    if (!parentNode || !parentNode.childNodes)
      return;
    for (var ix = 0; ix < parentNode.childNodes.length; ++ix)
    {
      switch( this.dlgNodeFilter(parentNode.childNodes[ix]) )
      {
        case this.acceptAll:
          newParent.appendChild( parentNode.childNodes[ix].cloneNode(true) );
          this.mbAtFirst = false;
        break;
        case this.skip:
          this.getXMLNodesForParent( newParent, parentNode.childNodes[ix] );
          this.mbAtFirst = false;
        break;
        case this.accept:
        {
          var aNewNode = parentNode.childNodes[ix].cloneNode(false);
          this.getXMLNodesForParent( aNewNode, parentNode.childNodes[ix] );
          newParent.appendChild( aNewNode );
          this.mbAtFirst = false;
        }
        break;
        case this.reject:
        break;
      }
    }
  },
  getXMLNodesAsDocFragment : function()
  {
    var docFragment = null;
    var doc = this.mEditorElement.contentDocument;
    this.mbAtFirst = true;
    if (doc != null)
    {
      docFragment = doc.createDocumentFragment();
      var rootNode = this.getRootNode();
      this.getXMLNodesForParent( docFragment, rootNode );
    }
    this.checkForTrailingBreak(docFragment);
//    var dumpStr = "In msiDialogEditorContentFilter.getXMLNodes, returning a docFragment containing: [";
//    for (var ix = 0; ix < docFragment.childNodes.length; ++ix)
//      dumpStr += this.mXmlSerializer.serializeToString(docFragment.childNodes[ix]);
//    dump(dumpStr + "] for editorElement [" + this.mEditorElement.id + "].\n");
    return docFragment;
  },
  nodeHasRealContent : function(parentElement, bIsLast)
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
            this.mbAtFirst = false;
          break;
          case this.acceptAll:
          case this.accept:
            if (bIsLast && (ix == parentElement.childNodes.length - 1))
              bFoundContent = (parentElement.childNodes[ix].nodeName != "br");
            else
              bFoundContent = true;
            this.mbAtFirst = false;
            break;
          case this.reject:
            break;
        }
      }
//      return bFoundContent;
    }
    return bFoundContent;
  },
  isNonEmpty : function()
  {
    var parentElement = null;
    var doc = this.mEditorElement.contentDocument;
    this.mbAtFirst = true;
    if (doc != null)
      parentElement = msiGetRealBodyElement(doc);
    return this.nodeHasRealContent( parentElement, true );
  },
  checkForTrailingBreak : function(parentNode)
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
  },
  hasNonEmptyMathContent : function()
  {
    var retval = false;
    var editor = msiGetEditor(this.mEditorElement);
    if (editor != null)
    {
      var mathNodes = editor.document.getElementsByTagName("math");
      this.mbAtFirst = true;
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
  },
  hasNonEmptyContent : function(bMathOnly)
  {
    if (bMathOnly)
      return this.hasNonEmptyMathContent();
    return this.isNonEmpty();
  },
  getContentsAsRange : function()
  {
    var theRange = null;
    var doc = null;
    var editor = msiGetEditor(this.mEditorElement);
    if (editor != null)
      doc = editor.document;
    if (doc != null)
    {
      var rootNode = this.getRootNode();
//      var rootNode = msiGetRealBodyElement(doc);
      var initialParaNode = null;
      var initialParaList = rootNode.getElementsByTagName("dialogbase");
      if (!initialParaList.length)
        initialParaList = rootNode.getElementsByTagName(this.defaultParaTag);
      if (initialParaList.length > 0)
        initialParaNode = initialParaList[0];
      else
        initialParaNode = rootNode.childNodes[0];
      var startNode = null;
      if (initialParaNode.childNodes.length)
        startNode = initialParaNode.childNodes[0];
      else
        startNode = initialParaNode;
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
  },
  findTrailingWhiteSpace : function(parentNode)
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
  },
  getContentsAsDocumentFragment : function()
  {
    var theFragment = null;
    var theRange = this.getContentsAsRange();
    if (theRange != null)
    {
      theFragment = theRange.cloneContents();
      theFragment.normalize();

      this.checkForTrailingBreak(theFragment);
//      var dumpStr = "In msiDialogEditorContentFilter.getContentsAsDocumentFragment, returning a docFragment containing: [";
//      for (var ix = 0; ix < theFragment.childNodes.length; ++ix)
//        dumpStr += this.mXmlSerializer.serializeToString(theFragment.childNodes[ix]);
//      dump(dumpStr + "] for editorElement [" + this.mEditorElement.id + "].\n");

    }
    return theFragment;
  },
  getDocumentFragmentString : function()
  {
    var theString = "";
//    var theFragment = this.getContentsAsDocumentFragment();
    var theFragment = this.getXMLNodesAsDocFragment();
    if (theFragment != null)
    {
      for (var ix = 0; ix < theFragment.childNodes.length; ++ix)
        theString += this.mXmlSerializer.serializeToString(theFragment.childNodes[ix]);
    }
//    dump("In msiDialogEditorContentFilter.getDocumentFragmentString, returning [" + theString + "] for editorElement [" + this.mEditorElement.id + "].\n");
    return theString;
  },
  getMarkupString : function()
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
  },
  getTextString : function()
  {
//    var theFragment = this.getContentsAsDocumentFragment();
    var theFragment = this.getXMLNodesAsDocFragment();
    if (theFragment != null)
    {
//      dump("In msiDialogEditorContentFilter.getTextString, returning [" + theFragment.textContent + "] for editorElement [" + this.mEditorElement.id + "].\n");
      return theFragment.textContent;
    }
//    dump("In msiDialogEditorContentFilter.getTextString, returning [] for editorElement [" + this.mEditorElement.id + "].\n");
    return "";
  },
  hasTextContent : function()
  {
    var str = this.getTextString();
    return ( (str != null) && (str.length > 0) );
  },
  getFullContentString : function()
  {
    var theStr = "";
    var doc = null;
    var editor = msiGetEditor(this.mEditorElement);
    if (editor != null)
      doc = editor.document;
    if (doc != null)
    {
//      var rootNode = msiGetRealBodyElement(doc);
      var rootNode = this.getRootNode();
      theStr = this.mXmlSerializer.serializeToString(rootNode);
    }
    return theStr;
  },
  getRootNode : function()
  {
    var rootNode = null;
    var editor = msiGetEditor(this.mEditorElement);
    var doc = null;
    if (editor != null)
      doc = editor.document;
    if (doc != null)
      rootNode = msiGetRealBodyElement(doc);
//    if (rootNode && this.mbSinglePara)  //This isn't yet used, though it probably should be. Postpone implementation until needed...
//    {
//    }
    if (rootNode && this.mbMathOnly)  //Take only the first <math> element.
    {
      var mathList = rootNode.getElementsByTagName("math");
      rootNode = mathList[0];
    }
    return rootNode;
  },
  setMathOnly : function(bTrue)
  {
    this.mbMathOnly = bTrue;
  }
};

function msiDialogEditorContentFilter(anEditorElement)
{
  this.mEditorElement = anEditorElement;
  this.mbMathOnly = false;
  this.mbSinglePara = false;
  if (anEditorElement.mbSinglePara)
    this.mbSinglePara = true;
  this.mbAtFirst = true;
  var editor = msiGetEditor(this.mEditorElement);
  if (editor && editor.tagListManager)
  {
    var namespace = new Object();
    this.defaultParaTag = editor.tagListManager.getDefaultParagraphTag(namespace);
  }
}

msiDialogEditorContentFilter.prototype = msiDialogEditorContentFilterBase;

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

function openStructureTagDialog(tagname, node, editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  msiDoModelessPropertiesDialog("chrome://prince/content/structureproperties.xul", "structureproperties", "chrome,close,titlebar,resizable, dependent", editorElement, "cmd_reviseStructureNode", node, node);
// openDialog( "chrome://prince/content/structureproperties.xul",
//                             "structureproperties",
//                             "chrome, close, titlebar, resizable, dependent",
//                             node);
}

function openParaTagDialog(tagname, node, editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  if (tagname === "title"){
    msiDoModelessPropertiesDialog("chrome://prince/content/titleProperties.xul", "titleproperties", "chrome,close,titlebar,resizable, dependent", editorElement, "cmd_reviseParagraphNode", node, node);
  }
  else{
    msiDoModelessPropertiesDialog("chrome://prince/content/paragraphproperties.xul", "paraproperties", "chrome,close,titlebar,resizable, dependent", editorElement, "cmd_reviseParagraphNode", node, node);
  } 
}

function openEnvTagDialog(tagname, aNode, editorElement)
{
  var theDoc;
  theDoc = aNode.ownerDocument;
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  theDoc = aNode.ownerDocument;
  var thmList = new msiTheoremEnvListForDocument(aNode.ownerDocument);
  var thmInfo = thmList.getTheoremEnvInfoForTag(tagname);
  if (thmInfo != null)
  {
    var numbering = thmInfo.numbering;
    if (!numbering || !numbering.length)
      numbering = tagname;
    var thmstyle = thmInfo.thmStyle;
    if (!thmstyle || !thmstyle.length)
      thmstyle = "plain";
    var defaultThmEnvNumbering = thmList.getDefaultTheoremEnvNumbering();
    var thmdata = {envNode : aNode, defaultNumbering : numbering, defaultTheoremEnvNumbering : defaultThmEnvNumbering, theoremstyle : thmstyle};
    msiDoModelessPropertiesDialog("chrome://prince/content/thmproperties.xul", "thmproperties", "chrome,close,titlebar,resizable, dependent",
                                                     editorElement, "cmd_reviseTheoremNode", aNode, thmdata);
  }
  else
  {
    msiDoModelessPropertiesDialog("chrome://prince/content/envproperties.xul", "envproperties", "chrome,close,titlebar,resizable, dependent",
                                                     editorElement, "cmd_reviseEnvNode", aNode, aNode);
  }
  thmList.detach();
}

function openObjectTagDialog(tagname, node, editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var imgData = {isVideo : false, mNode : node};
  var cmdStr = "cmd_reviseImage";
  if (node.getAttribute("isVideo") == "true")
  {
    imgData.isVideo = true;
    cmdStr = "cmd_reviseVideo";
  }
  msiDoModelessPropertiesDialog("chrome://prince/content/msiEdImageProps.xul", "_blank", "chrome,close,titlebar,resizable, dependent",
                                                     editorElement, cmdStr, node, imgData);
//  openDialog('chrome://prince/content/msiEdImageProps.xul', '_blank', 'chrome,close,titlebar,resizable, dependent',
//    null, 'cmd_reviseImage', node);

}

function openTeXButtonDialog(tagname, node)
{
  openDialog('chrome://prince/content/texbuttoncontents.xul', '_blank', 'chrome,close,titlebar,resizable, dependent',
    node);
  var editorElement = msiGetActiveEditorElement();
	msiGetEditor(editorElement).incrementModificationCount(1);
}

function openOTFontDialog(tagname, node)
{
  openDialog('chrome://prince/content/otfont.xul', '_blank', 'chrome,close,titlebar,resizable, dependent',
    node);
  var editorElement = msiGetActiveEditorElement();
  msiGetEditor(editorElement).incrementModificationCount(1);
}

function openHTMLField(node)
{
  openDialog('chrome://prince/content/htmlfield.xul', '_blank', 'chrome,close,titlebar,resizable, dependent',
    node);
  var editorElement = msiGetActiveEditorElement();
	msiGetEditor(editorElement).incrementModificationCount(1);
}

//function openFontColorDialog(tagname, node)
//{
//  var colorObj = { NoDefault:true, Type:"Font", TextColor:"black", PageColor:0, Cancel:false };
//  openDialog('chrome://prince/content/color.xul', '_blank', 'chrome,close,titlebar,resizable, dependent',
//    "",colorObj,node);
//}
//
//function openFontSizeDialog(tagname, node)
//{
//  openDialog('chrome://prince/content/fontsize.xul', '_blank', 'chrome,close,titlebar,resizable, dependent',
//    node, editor);
//}
//

function openGraphDialog(tagname, node, editorElement)
{
  // non-modal dialog, the return is immediate
  var dlgWindow = openDialog("chrome://prince/content/ComputeGraphSettings.xul", "Plot Dialog", "chrome,close,resizable,titlebar,dependent",
     editorElement, "cmd_objectProperties", node);
// why find it again???  var editorElement = msiGetActiveEditorElement();
	msiGetEditor(editorElement).incrementModificationCount(1);

}

function getMSIDocumentInfo(editorElement)
{
  var docInfo = new msiDocumentInfo(editorElement);
  docInfo.initializeDocInfo();
  return docInfo;
}

function getBibliographyScheme(editorElement)
{
  var docInfo = getMSIDocumentInfo(editorElement);
  if (docInfo && docInfo.generalSettings)
  {
    if ( ("bibliographyscheme" in docInfo.generalSettings) && ("contents" in docInfo.generalSettings.bibliographyscheme) )
      return docInfo.generalSettings.bibliographyscheme.contents;
  }
  var prefs = GetPrefs();
  var bibchoice = prefs.getCharPref("swp.bibchoice");

  return bibchoice;
}

function setBibliographyScheme(editorElement, whichScheme)
{
  var docInfo = getMSIDocumentInfo(editorElement);
  docInfo.setObjectFromData(docInfo.generalSettings, "bibliographyscheme", (whichScheme.length > 0), "BibliographyScheme", whichScheme, "comment-key-value");
  docInfo.putDocInfoToDocument();
}

function msiClickLink(event, theURI, targWinStr, editorElement)
{
  var doFollowLink = false;
  var os = getOS(window);
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var theWindow = window;
  if (editorElement)
    theWindow = editorElement.contentWindow;
  var editor = msiGetEditor(editorElement);
  var flags = editor.flags;
  if (flags & nsIPlaintextEditor.eEditorReadonlyMask)
    doFollowLink = true;
  else {
    if (os === 'osx') {
      if (event.altKey) {
        doFollowLink = true;
      }
    }
    else {
      if (event.ctrlKey) {
        doFollowLink = true;
      }
    }
  }
  if (!doFollowLink)
    return;
  if (theURI.indexOf('.sci') === -1 ) {
    msiFollowLink(event.target);
    return;
  }

  var objName = msiGetBaseNodeName(event.target);
  var preferMarker = (objName == "xref");
  var targURIStr, targMarker;
  var newWindow;
  var sharpPos = theURI.indexOf("#");
  if (sharpPos < 0)
    targURIStr = theURI;
  else
  {
    targMarker = theURI.substr(sharpPos+1);
    targURIStr = theURI.substr(0, sharpPos);
  }
  // msiMakeAbsoluteUrl uses foo_work as the base. We want to use the parent directory (SWPDOCS) instead, so
  // BBM should be checking that these strings are relative
  targURIStr = '../'+targURIStr;
  theURI = '../'+theURI;
  var targURI = msiCreateURI(msiMakeAbsoluteUrl(targURIStr, editorElement));
  var fullTargURI = msiCreateURI(msiMakeAbsoluteUrl(theURI, editorElement));

//  if (!targWinStr || (targWinStr == "_blank"))  //RWA Commenting this out for now! Don't try to re-use editors until we know how...
    targWinStr = "";

  var targEditor, targWin, winWatcher;
  if (!targURIStr.length)
    targEditor = editorElement;

  var winNameToUse = "";
//  if (targWinStr.length)
//  {
//    switch(targWinStr)
//    {
//      case "_top":
//        targEditor = msiGetTopLevelEditorElement(window);
//      break;
//      case "_parent":
//        targEditor = msiGetParentOrTopLevelEditor(editorElement);
//      break;
//      case "_self":
//        targEditor = editorElement;
//      break;
//      default:  //using a window identifier string - find the window?
//        winWatcher = Components.classes["@mozilla.org/embedcomp/window-watcher;1"].getService(Components.interfaces.nsIWindowWatcher);
//        targWin = winWatcher.getWindowByName(targWinStr, null);
//        if (targWin)
//          targEditor =  msiGetPrimaryEditorElementForWindow(targWin);
//        else
//          winNameToUse = targWinStr;
//      break;
//    }
//  }

  if (!targEditor)
    targEditor = msiEditPage(fullTargURI, theWindow, false, false, winNameToUse);
  else if (targURI)
  {
    msiCheckAndSaveDocument(targEditor, "cmd_close", true);
    var isSciRegEx = /\.sci$/i;
    var isSci = isSciRegEx.test(targURI.spec);
    if (isSci)
    {
      var doc = msiFileFromFileURL(targURI);
      var newdoc = createWorkingDirectory(doc);
      targURI = msiFileURLFromFile(newdoc);
    }
    msiEditorLoadUrl(targEditor, targURI, targMarker);
  }
  else if (targMarker && targMarker.length)
    msiGoToMarker(targEditor, targMarker, preferMarker);
}


function msiGoToMarker(editorElement, markerStr, bPreferKey)
{
  var editor = msiGetEditor(editorElement);
//  var targNode = editor.document.getElementById(markerStr);
  var targNode;
  var ourExpr = ".//*[(@key='" + markerStr + "') or (@id='" + markerStr + "') or (@marker='" + markerStr + "') or (@customLabel='" + markerStr + "')]";
  var xPathEval = new XPathEvaluator();
  var nsResolver = xPathEval.createNSResolver(editor.document.documentElement);

  var resultNodes = xPathEval.evaluate(ourExpr, editor.document.documentElement, nsResolver, XPathResult.ORDERED_NODE_SNAPSHOT_TYPE, null);
  if (resultNodes.snapshotLength==1)
    targNode = resultNodes.snapshotItem(0);
  else if (resultNodes.snapshotLength > 1)
  {
    for (var ix = 0; !targNode && (ix < resultNodes.snapshotLength); ++ix)
    {
      currNode = resultNodes.snapshotItem(ix);
      if (bPreferKey)
      {
        if (currNode.hasAttribute("key") && (msiGetBaseNodeName(currNode) != "xref") && (currNode.getAttribute("key") == markerStr))
          targNode = currNode;
        else if (currNode.hasAttribute("marker") && (currNode.getAttribute("marker") == markerStr))
          targNode = currNode;
      }
      else if (currNode.hasAttribute("id") && (currNode.getAttribute("id") == markerStr))
        targNode = currNode;

    }
    if (!targNode)
      targNode = resultNodes.snapshotItem(0);
  }
  else  //No results found! Alert:
  {
    var missingMarkerTitleStr = GetString("MissingMarkerErrorTitle");
    var missingMarkerStr = GetString("MissingMarkerError");
    missingMarkerStr = missingMarkerStr.replace("%name%", markerStr);
    AlertWithTitle(missingMarkerTitleStr, missingMarkerStr);
  }

//    if (treeWalker)
//    {
//      for (var currNode = treeWalker.nextNode(); currNode != null; currNode = treeWalker.nextNode())
//      {
//        if ( (currNode.getAttribute("marker") == markerStr) || (currNode.getAttribute("key") == markerStr) )
//        {
//          targNode = currNode;
//          break;
//        }
//      }
//    }
//  }
  if (targNode)
  {
    var currNode = editor.selection.focusNode;
    var currOffset = editor.selection.focusOffset;
    var bAlignWithTop = (msiNavigationUtils.comparePositions(targNode, 0, currNode, currOffset) < 0);
    editor.selection.collapse(targNode,0);
    targNode.scrollIntoView(bAlignWithTop);
  }
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

// Any non-editor window wanting to create an editor with a URL
//   should use this instead of "window.openDialog..."
//  We must always find an existing window with requested URL
// (When calling from a dialog, "launchWindow" is dialog's "opener"
//   and we need a delay to let dialog close)
function msiEditPage(url, launchWindow, delay, isShell, windowName)
{
  // Always strip off "view-source:" and #anchors; kludge: accept string url or nsIURI url.
  if (!url.spec) url = msiURIFromString(url);  //some url's are passed as strings
  var urlstring, fullUrlstring;
  try {
    fullUrlstring = url.spec.replace(/^view-source:/, "");
  }
  catch(e) {
    fullUrlstring = url.replace(/^view-source:/, "");
  }
  urlstring = fullUrlstring.replace(/#.*/, "");

  var markerArg, marker;
  var sharpPos = fullUrlstring.indexOf("#");
  if (sharpPos >= 0)
  {
    marker = fullUrlstring.substr(sharpPos + 1);
    markerArg = "initialMarker=" + marker;
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
      return null;
    }
  }

  // if the current window is a browser window, then extract the current charset menu setting from the current
  // document and use it to initialize the new composer window...

  var wintype = document.documentElement.getAttribute('windowtype');
  var charsetArg, win;

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
      win = enumerator.getNext().QueryInterface(Components.interfaces.nsIDOMWindowInternal);
      if ( win && msiIsWebComposer(win))
      {
        useEditorElement = msiCheckOpenWindowForURIMatch(uri, win);
        if (useEditorElement != null)
        {
          // We found an editor with our url
          useEditorElement.focus();
          useEditorElement.isShellFile = isShell;
          if (marker && marker.length)
            msiGoToMarker(useEditorElement, marker);
          return useEditorElement;
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
      msiEditorLoadUrl(useEditorElement, uri, marker);
      useEditorElement.focus();
      useEditorElement.isShellFile = isShell;
      msiSetSaveAndPublishUI(uri.spec, useEditorElement);


//      if (emptyWindow.IsInHTMLSourceMode())
//        emptyWindow.SetEditMode(emptyWindow.PreviousNonSourceDisplayMode);
//      emptyWindow.EditorLoadUrl(url);
//      emptyWindow.focus();
//      emptyWindow.SetSaveAndPublishUI(url);
      return useEditorElement;
    }

    // Create new Composer window
    if (!windowName || !windowName.length)
      windowName = "_blank";
    if (delay)
    {
      win = launchWindow.delayedOpenWindow("chrome://prince/content", null, "chrome,all,dialog=no", url, null, null, isShell);
    }
    else
      win = launchWindow.openDialog("chrome://prince/content", windowName, "chrome,all,dialog=no", uri.spec, charsetArg, markerArg, isShell);

    return useEditorElement;
  } catch(e) {}
  return null;
}

// returns file picker result
function msiGetSaveLocationForImage(editorElement)
{
  var dialogResult = {};
  dialogResult.filepickerClick = msIFilePicker.returnCancel;
//  dialogResult.resultingURI = "";
  dialogResult.resultingLocalFile = null;

  if (!editorElement)
    editorElement = msiGetActiveEditorElement();

  var fp = null;
  try {
    fp = Components.classes["@mozilla.org/filepicker;1"].createInstance(msIFilePicker);
  } catch (e) {}
  if (!fp) return dialogResult;

  // determine prompt string based on type of saving we'll do
  var promptString = GetString("SavePictureAs");
  fp.init(window, promptString, msIFilePicker.modeSave);

  // Set filters according to the type of output
  var exportTypes = ["png","pdf","jpg","svg"];
  var os = getOS(window);
  if (os==="win")
  {
    exportTypes.push("emf");
//rwa12-19-12 We won't have the WMF option visible; though the code will produce one if asked to, the user will generally
//rwa12-19-12   be unhappy with the results thus far, for reasons documented elsewhere.
//rwa12-19-12  exportTypes.push("wmf");
  }
  for (var ii = 0; ii < exportTypes.length; ++ii)
  {
    fp.appendFilter(GetString("SaveImageAsDesc_" + exportTypes[ii]),"*."+exportTypes[ii]);
  }
  fp.defaultExtension = "png";
  fp.appendFilters(msIFilePicker.filterAll);

//  // now let's actually set the filepicker's suggested filename
//  var suggestedFileName = msiGetSuggestedFileName(aDocumentURLString, aMIMEType, editorElement);
//  if (suggestedFileName)
//  {
//    var lastDot = suggestedFileName.lastIndexOf(".");
//    if (lastDot != -1)
//      suggestedFileName = suggestedFileName.slice(0, lastDot);
//  
//    fp.defaultString = suggestedFileName;
//  }

  // set the file picker's current directory
  // assuming we have information needed (like prior saved location)
      // Initialize to the last-used directory for the particular type (saved in prefs)
  msiSetFilePickerDirectory(fp, "SaveAsPicture");

  dialogResult.filepickerClick = fp.show();
  if (dialogResult.filepickerClick != msIFilePicker.returnCancel)
  {
    // reset urlstring to new save location
//    dialogResult.resultingURIString = fileHandler.getURLSpecFromFile(fp.file);
    dialogResult.resultingLocalFile = fp.file;
    msiSaveFilePickerDirectory(fp, "SaveAsPicture");
  }

  return dialogResult;
}

function msiCopyAsPicture(editorElement)
{
//  msiSaveAsPicture(editorElement);  //temp only!

  var editor = msiGetEditor(editorElement);
//  var mathmlEditor = editor.QueryInterface(Components.interfaces.msiIMathMLEditor);
//  var selection = editor.selection;
//
////  AlertWithTitle("Copy as Picture", "Feature not yet implemented.");
  editor.copySelectionAsImage();
}

function msiSaveAsPicture(editorElement)
{
  var editor = msiGetEditor(editorElement);
//  var mathmlEditor = editor.QueryInterface(Components.interfaces.msiIMathMLEditor);
  var selection = editor.selection;

//  AlertWithTitle("Copy as Picture", "Feature not yet implemented.");
  var dialogResult = msiGetSaveLocationForImage(editorElement);
  if (dialogResult.filepickerClick != msIFilePicker.returnCancel)
    editor.saveSelectionAsImage(dialogResult.resultingLocalFile.path);
}



