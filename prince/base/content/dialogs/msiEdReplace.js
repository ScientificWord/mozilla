/* -*- Mode: Java; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * ***** BEGIN LICENSE BLOCK *****
Copyright 2007 MacKichan Software, Inc.
 * ***** END LICENSE BLOCK ***** */

var gReplaceDialog;      // Quick access to document/form elements.
var gFindInst;           // nsIWebBrowserFind that we're going to use
var gFindService;        // Global service which remembers find params
var gEditor;             // the editor we're using

function initDialogObject()
{
  // Create gReplaceDialog object and initialize.
  gReplaceDialog = {};
  gReplaceDialog.findInput       = document.getElementById("findInput");
  gReplaceDialog.replaceInput    = document.getElementById("replaceInput");
  gReplaceDialog.caseSensitive   = document.getElementById("dialog.caseSensitive");
  gReplaceDialog.wrap            = document.getElementById("dialog.wrap");
  gReplaceDialog.searchBackwards = document.getElementById("dialog.searchBackwards");
  gReplaceDialog.findNext        = document.getElementById("findNext");
  gReplaceDialog.replace         = document.getElementById("replace");
  gReplaceDialog.replaceAndFind  = document.getElementById("replaceAndFind");
  gReplaceDialog.replaceAll      = document.getElementById("replaceAll");
  gReplaceDialog.findContentFilter = null;
  gReplaceDialog.replaceContentFilter = null;
}

function loadDialog()
{
  // Set initial dialog field contents.
  // Set initial dialog field contents. Use the gFindInst attributes first,
  // this is necessary for window.find()
//  gReplaceDialog.findInput.value         = (gFindInst.searchString
//                                            ? gFindInst.searchString
//                                            : gFindService.searchString);
//  gReplaceDialog.replaceInput.value = gFindService.replaceString;

  var theStringSource = (gFindInst.searchString ? gFindInst.searchString
                                                : gFindService.searchString);
  if (theStringSource != null && theStringSource.length == 0)
    theStringSource = null;
//  msiInitializeEditorForElement(gReplaceDialog.findInput, theStringSource, true);

  var theStringSource2 = gFindService.replaceString;
  if (theStringSource2 != null && theStringSource2.length == 0)
    theStringSource2 = null;
//  msiInitializeEditorForElement(gReplaceDialog.replaceInput, theStringSource2, true);
  var editorInitializer = new msiEditorArrayInitializer();
  editorInitializer.addEditorInfo(gReplaceDialog.findInput, theStringSource, true);
  editorInitializer.addEditorInfo(gReplaceDialog.replaceInput, theStringSource2, true);
  editorInitializer.doInitialize();

//  gReplaceDialog.replaceInput.makeEditable("html", false);


  gReplaceDialog.caseSensitive.checked   = (gFindInst.matchCase
                                            ? gFindInst.matchCase
                                            : gFindService.matchCase);
  gReplaceDialog.wrap.checked            = (gFindInst.wrapFind
                                            ? gFindInst.wrapFind
                                            : gFindService.wrapFind);
  gReplaceDialog.searchBackwards.checked = (gFindInst.findBackwards
                                            ? gFindInst.findBackwards
                                            : gFindService.findBackwards);

  gReplaceDialog.tabOrderArray = new Array( gReplaceDialog.findInput, gReplaceDialog.replaceInput, gReplaceDialog.caseSensitive,
                                       gReplaceDialog.wrap, gReplaceDialog.searchBackwards,
                                       gReplaceDialog.findNext, gReplaceDialog.replace,
                                       gReplaceDialog.replaceAndFind, gReplaceDialog.replaceAll,
                                       document.documentElement.getButton("cancel") );
  doEnabling();
}

function onLoad()
{
  // Get the xul <editor> element:
  var editorElement = window.arguments[0];

  // If we don't get the editor, then we won't allow replacing.
  dump("w");
  gEditor = editorElement.getEditor(editorElement.contentWindow);
  if (!gEditor)
  {
    window.close();
    return;
  }

  // Get the nsIWebBrowserFind service:
  gFindInst = editorElement.webBrowserFind;

  try {
  // get the find service, which stores global find state
    gFindService = Components.classes["@mozilla.org/find/find_service;1"]
                         .getService(Components.interfaces.nsIFindService);
  } catch(e) { dump("No find service!\n"); gFindService = 0; }

  // Init gReplaceDialog.
  initDialogObject();

  // Change "OK" to "Find".
  //dialog.find.label = document.getElementById("fBLT").getAttribute("label");

  // Fill dialog.
  loadDialog();

//  if (gReplaceDialog.findInput.value)
//    gReplaceDialog.findInput.select();
//  else
    gReplaceDialog.findInput.focus();
}

function onUnload() {
  // Disconnect context from this dialog.
  gFindReplaceData.replaceDialog = null;
}

function saveFindData()
{
  // Set data attributes per user input.
  if (gFindService)
  {
//    gFindService.searchString  = gReplaceDialog.findInput.value;
    var serializer = new XMLSerializer();
    gFindService.searchString = serializer.serializeToString(gReplaceDialog.findInput.contentDocument.documentElement);

    gFindService.matchCase     = gReplaceDialog.caseSensitive.checked;
    gFindService.wrapFind      = gReplaceDialog.wrap.checked;
    gFindService.findBackwards = gReplaceDialog.searchBackwards.checked;
  }
}

function setUpFindInst()
{
//  gFindInst.searchString  = gReplaceDialog.findInput.value;
//  var serializer = new XMLSerializer();
//  gFindInst.searchString = serializer.serializeToString(gReplaceDialog.findInput.contentDocument.documentElement);
  //This is a temporary hack - we need to be using the tags in the find information.

//  gFindInst.searchString = getStringFromDialogEditor(gReplaceDialog.findInput);

  if (gReplaceDialog.findContentFilter == null)
    gReplaceDialog.findContentFilter = new msiDialogEditorContentFilter(gReplaceDialog.findInput);
//  gFindInst.searchString = gReplaceDialog.findContentFilter.getMarkupString();  - this is what it should be
  gFindInst.searchString = gReplaceDialog.findContentFilter.getTextString();

//  gFindInst.searchString = msiGetEditor(gReplaceDialog.findInput).outputToString("text/plain", 1024); // OutputLFLineBreak

  gFindInst.matchCase     = gReplaceDialog.caseSensitive.checked;
  gFindInst.wrapFind      = gReplaceDialog.wrap.checked;
  gFindInst.findBackwards = gReplaceDialog.searchBackwards.checked;
}


//function getStringFromDialogEditor(anEditorElement)
//{
//  var theString = "";
//  var doc = anEditorElement.contentDocument;
//  if (doc != null)
//  {
//    var rootNode = msiGetRealBodyElement(doc);
//    var xmlSerializer = new XMLSerializer();
//    for (var ix = 0; ix < rootNode.childNodes.length; ++ix)
//      theString += xmlSerializer.serializeToString(rootNode.childNodes[ix]);
//  }
//  return theString;
//}
//
//function getXMLNodesFromDialogEditor(anEditorElement)
//{
//  var nodeList = new Array();
//  var doc = anEditorElement.contentDocument;
//  if (doc != null)
//  {
//    var rootNode = msiGetRealBodyElement(doc);
//    for (var ix = 0; ix < rootNode.childNodes.length; ++ix)
//    {
//      if (msiShouldExtractNode(rootNode.childNodes[ix].nodeName, rootNode.childNodes[ix].namespaceURI))
//        nodeList.push(rootNode.childNodes[ix].cloneNode(true));
//      else
//      {
//        for (var jx = 0; jx < rootNode.childNodes[ix].childNodes.length; ++jx)
//          nodeList.push(rootNode.childNodes[ix].childNodes[jx].cloneNode(true));
//      }
//    }
//  }
//  return nodeList;
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
    var nodename = aNode.nodeMame;
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
    var fulltagStr = "";
    if (aNode.namespaceURI != null)
      fulltagStr += aNode.namespaceURI;
    fulltagStr += aNode.nodeName;
    switch( fulltagStr )
    {
      case "sw:dialogbase":
        return this.skip;
      break;
    }
    switch(aNode.nodeName)
    {
      case "dialogbase":
        return this.skip;
      break;
    }
    return this.acceptAll;  
    //We still need to fill in the tags for which we want to accept the tag but leave open the possibility of not accepting a child.
    //Examples may include field tags, list tags, etc.; the point being that one may occur as the parent of something like a
    //  sw:dialogbase paragraph. Not implemented that way at this point.  rwa, 8-
  };
  this.getXMLNodesForParent = function(parentNode)
  {
    var theNodes = new Array();
    for (var ix = 0; ix < parentNode.childNodes.length; ++ix)
    {
      switch( this.dlgNodeFilter(parentNode.childNodes[ix]) )
      {
        case this.acceptAll:
          theNodes.push( parentNode.childNodes[ix].cloneNode(true) );
        break;
        case this.skip:
          theNodes = theNodes.concat( this.getXMLNodesForParent(parentNode.childNodes[ix]) );
        break;
        case this.accept:
        {
          var aNewNode = parentNode.childNodes[ix].cloneNode(false);
          var childList = this.getXMLNodesForParent( parentNode.childNodes[ix] );
          for (var jx = 0; jx < childList.length; ++jx)
            aNewNode.appendChild( childList[jx] );
          theNodes.push( aNewNode );
        }
        break;
        case this.reject:
        break;
      }
    }
    return theNodes;
  };
  this.getXMLNodes = function()
  {
    var nodeList = new Array();
    var doc = this.mEditorElement.contentDocument;
    var editor = msiGetEditor(this.mEditorElement);
    if (doc != null)
    {
      var rootNode = msiGetRealBodyElement(doc);
      nodeList = this.getXMLNodesForParent( rootNode );
//      for (var ix = 0; ix < rootNode.childNodes.length; ++ix)
//      {
//      var treeWalker = doc.createTreeWalker(rootNode, NodeFilter.SHOW_ELEMENT, this.dlgNodeFilter, true);
//      if (treeWalker)
//      {
//        var nextNode = treeWalker.nextNode();
//        while (nextNode)
//        {
//          nodeList.push(nextNode.cloneNode(false));
//          nextNode = treeWalker.nextNode();
//        }
//      }
    }
    var dumpStr = "In msiDialogEditorContentFilter.getXMLNodes, returning a nodeList containing: [";
    for (var ix = 0; ix < nodeList.length; ++ix)
    {
      if (ix > 0)
        dumpStr += ", " + nodeList[ix].nodeName;
      else
        dumpStr += nodeList[ix].nodeName;
    }
    dump(dumpStr + "] for editorElement [" + this.mEditorElement.id + "].\n");
    return nodeList;
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
      var docRangeObj = doc.QueryInterface(Components.interfaces.nsIDOMDocumentRange);
      theRange = docRangeObj.createRange();
      theRange.setStart(initialParaNode, 0);
      var lastNode = rootNode.childNodes[rootNode.childNodes.length - 1];
      theRange.setEndAfter(lastNode);
    }
    return theRange;
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
    var theRange = this.getContentsAsRange();
    if (theRange != null)
      theString = theRange.toString();
    return theString;
  };
  this.getMarkupString = function()
  {
    var theString = "";
    var nodeList = this.getXMLNodes();
    if (nodeList != null)
    {
      for (var ix = 0; ix < nodeList.length; ++ix)
        theString += this.mXmlSerializer.serializeToString(nodeList[ix]);
    }
    dump("In msiDialogEditorContentFilter.getMarkupString, returning [" + theString + "] for editorElement [" + this.mEditorElement.id + "].\n");
    return theString;
  };
  this.getTextString = function()
  {
    var theFragment = this.getContentsAsDocumentFragment();
    if (theFragment != null)
      return theFragment.textContent;
    return "";
  };
}


function onFindNext()
{
  // Transfer dialog contents to the find service.
  saveFindData();
  // set up the find instance
  setUpFindInst();

  // Search.
  var result = gFindInst.findNext();

  if (!result)
  {
    var bundle = document.getElementById("findBundle");
    AlertWithTitle(null, bundle.getString("notFoundWarning"));
    SetTextboxFocus(gReplaceDialog.findInput);
//    gReplaceDialog.findInput.select();
    gReplaceDialog.findInput.focus();
    return false;
  } 
  return true;
}

function onReplace()
{
  if (!gEditor)
    return false;

  // Does the current selection match the find string?
  var selection = gEditor.selection;

  var selStr = selection.toString();
//  var specStr = msiGetEditor(gReplaceDialog.findInput).outputToString("text/plain", 1024); // OutputLFLineBreak
  if (gReplaceDialog.findContentFilter == null)
    gReplaceDialog.findContentFilter = new msiDialogEditorContentFilter(gReplaceDialog.findInput);
  if (gReplaceDialog.replaceContentFilter == null)
    gReplaceDialog.replaceContentFilter = new msiDialogEditorContentFilter(gReplaceDialog.replaceInput);

//  var specStr = gReplaceDialog.findContentFilter.getMarkupString();  - this is what it should be
  var specStr = gReplaceDialog.findContentFilter.getTextString();
  if (!gReplaceDialog.caseSensitive.checked)
  {
    selStr = selStr.toLowerCase();
    specStr = specStr.toLowerCase();
  }
  // Unfortunately, because of whitespace we can't just check
  // whether (selStr == specStr), but have to loop ourselves.
  // N chars of whitespace in specStr can match any M >= N in selStr.
  var matches = true;
  var specLen = specStr.length;
  var selLen = selStr.length;
  if (selLen < specLen)
    matches = false;
  else
  {
    var specArray = specStr.match(/\S+|\s+/g);
    var selArray = selStr.match(/\S+|\s+/g);
    if ( specArray.length != selArray.length)
      matches = false;
    else
    {
      for (var i=0; i<selArray.length; i++)
      {
        if (selArray[i] != specArray[i])
        {
          if ( /\S/.test(selArray[i][0]) || /\S/.test(specArray[i][0]) )
          {
            // not a space chunk -- match fails
            matches = false;
            break;
          }
          else if ( selArray[i].length < specArray[i].length )
          {
            // if it's a space chunk then we only care that sel be
            // at least as long as spec
            matches = false;
            break;
          }
        }
      }
    }
  }

  // If the current selection doesn't match the pattern,
  // then we want to find the next match, but not do the replace.
  // That's what most other apps seem to do.
  // So here, just return.
  if (!matches)
    return false;

  // Transfer dialog contents to the find service.
  saveFindData();

  // For reverse finds, need to remember the caret position
  // before current selection
  var newRange;
  if (gReplaceDialog.searchBackwards.checked && selection.rangeCount > 0)
  {
    newRange = selection.getRangeAt(0).cloneRange();
    newRange.collapse(true);
  }

  // nsPlaintextEditor::InsertText fails if the string is empty,
  // so make that a special case:
//  var serializer = new XMLSerializer();
//  var replStr = serializer.serializeToString(gReplaceDialog.replaceInput.contentDocument.documentElement);
//  var replStr = getStringFromEditor(gReplaceDialog.replaceInput);
//  var replNodes = gReplaceDialog.replaceContentFilter.getXMLNodes();
  var replFrag = gReplaceDialog.replaceContentFilter.getContentsAsDocumentFragment();

//rwa  if (replStr == "")
    gEditor.deleteSelection(0);
  if (replFrag != null)
    insertXMLNodesAtCursor(gEditor, replFrag.childNodes, true);
//rwa    insertXMLNodesAtCursor(gEditor, replNodes, true);
//rwa  else
//rwa    insertXMLAtCursor(gEditor, replStr, bIsSinglePara, true);


////  else if (gEditor.editortype == "html" || gEditor.editortype == "htmlmail")
//  else if ("insertHTML" in gEditor)
//    gEditor.insertHTML(replStr);
//  else
//    gEditor.insertText(replStr);

  // For reverse finds, need to move caret just before the replaced text
  if (gReplaceDialog.searchBackwards.checked && newRange)
  {
    gEditor.selection.removeAllRanges();
    gEditor.selection.addRange(newRange);
  }

  return true;
}

function onReplaceAll()
{
  if (!gEditor)
    return;

  setUpFindInst();  //added this so we can use it
  // Transfer dialog contents to the find service.
  saveFindData();

  var findStr = gFindInst.searchString;
//  var serializer = new XMLSerializer();
//  var repStr = serializer.serializeToString(gReplaceDialog.replaceInput.contentDocument.documentElement);
  if (gReplaceDialog.replaceContentFilter == null)
    gReplaceDialog.replaceContentFilter = new msiDialogEditorContentFilter(gReplaceDialog.replaceInput);

//  var repStr = getStringFromEditor(gReplaceDialog.replaceInput);
//  var repNodes = gReplaceDialog.replaceContentFilter.getXMLNodes();
  var repFrag = gReplaceDialog.replaceContentFilter.getContentsAsDocumentFragment();
//  var repStr = gReplaceDialog.replaceInput.value;

  var finder = Components.classes["@mozilla.org/embedcomp/rangefind;1"].createInstance().QueryInterface(Components.interfaces.nsIFind);

  finder.caseSensitive = gReplaceDialog.caseSensitive.checked;
  finder.findBackwards = gReplaceDialog.searchBackwards.checked;

  // We want the whole operation to be undoable in one swell foop,
  // so start a transaction:
  gEditor.beginTransaction();

  // and to make sure we close the transaction, guard against exceptions:
  try {
    // Make a range containing the current selection, 
    // so we don't go past it when we wrap.
    var selection = gEditor.selection;
    var selecRange;
    if (selection.rangeCount > 0)
      selecRange = selection.getRangeAt(0);
    var origRange = selecRange.cloneRange();

    // We'll need a range for the whole document:
    var wholeDocRange = gEditor.document.createRange();
    var rootNode = gEditor.rootElement.QueryInterface(Components.interfaces.nsIDOMNode);
    wholeDocRange.selectNodeContents(rootNode);

    // And start and end points:
    var endPt = gEditor.document.createRange();

    if (gReplaceDialog.searchBackwards.checked)
    {
      endPt.setStart(wholeDocRange.startContainer, wholeDocRange.startOffset);
      endPt.setEnd(wholeDocRange.startContainer, wholeDocRange.startOffset);
    }
    else
    {
      endPt.setStart(wholeDocRange.endContainer, wholeDocRange.endOffset);
      endPt.setEnd(wholeDocRange.endContainer, wholeDocRange.endOffset);
    }

    // Find and replace from here to end (start) of document:
    var foundRange;
    var searchRange = wholeDocRange.cloneRange();
    while ((foundRange = finder.Find(findStr, searchRange,
                                     selecRange, endPt)) != null)
    {
      gEditor.selection.removeAllRanges();
      gEditor.selection.addRange(foundRange);

      // The editor will leave the caret at the end of the replaced text.
      // For reverse finds, we need it at the beginning,
      // so save the next position now.
      if (gReplaceDialog.searchBackwards.checked)
      {
        selecRange = foundRange.cloneRange();
        selecRange.setEnd(selecRange.startContainer, selecRange.startOffset);
      }

      // nsPlaintextEditor::InsertText fails if the string is empty,
      // so make that a special case:
//rwa      if (repStr == "")
        gEditor.deleteSelection(0);
//rwa//      else if (gEditor.editortype == "html" || gEditor.editortype == "htmlmail")
//rwa      else if ("insertHTML" in gEditor)
//rwa        gEditor.insertHTML(repStr);
//rwa      else
//rwa        gEditor.insertText(repStr);
        if (repFrag != null)
          insertXMLNodesAtCursor(gEditor, repFrag.childNodes, true);
//rwa        insertXMLNodesAtCursor(gEditor, replNodes, true);

      // If we're going forward, we didn't save selecRange before, so do it now:
      if (!gReplaceDialog.searchBackwards.checked)
      {
        selection = gEditor.selection;
        if (selection.rangeCount <= 0) {
          gEditor.endTransaction();
          return;
        }
        selecRange = selection.getRangeAt(0).cloneRange();
      }
    }

    // If no wrapping, then we're done
    if (!gReplaceDialog.wrap.checked) {
      gEditor.endTransaction();
      return;
    }

    // If wrapping, find from start/end of document back to start point.
    if (gReplaceDialog.searchBackwards.checked)
    {
      // Collapse origRange to end
      origRange.setStart(origRange.endContainer, origRange.endOffset);
      // Set current position to document end
      selecRange.setEnd(wholeDocRange.endContainer, wholeDocRange.endOffset);
      selecRange.setStart(wholeDocRange.endContainer, wholeDocRange.endOffset);
    }
    else
    {
      // Collapse origRange to start
      origRange.setEnd(origRange.startContainer, origRange.startOffset);
      // Set current position to document start
      selecRange.setStart(wholeDocRange.startContainer,
                          wholeDocRange.startOffset);
      selecRange.setEnd(wholeDocRange.startContainer, wholeDocRange.startOffset);
    }

    while ((foundRange = finder.Find(findStr, wholeDocRange,
                                     selecRange, origRange)) != null)
    {
      gEditor.selection.removeAllRanges();
      gEditor.selection.addRange(foundRange);

      // Save insert point for backward case
      if (gReplaceDialog.searchBackwards.checked)
      {
        selecRange = foundRange.cloneRange();
        selecRange.setEnd(selecRange.startContainer, selecRange.startOffset);
      }

      // nsPlaintextEditor::InsertText fails if the string is empty,
      // so make that a special case:
//rwa      if (repStr == "")
        gEditor.deleteSelection(0);
//rwa//      else if (gEditor.editortype == "html" || gEditor.editortype == "htmlmail")
//rwa      else if ("insertHTML" in gEditor)
//rwa        gEditor.insertHTML(repStr);
//rwa      else
//rwa        gEditor.insertText(repStr);
      insertXMLNodesAtCursor(gEditor, replNodes, true);

      // Get insert point for forward case
      if (!gReplaceDialog.searchBackwards.checked)
      {
        selection = gEditor.selection;
        if (selection.rangeCount <= 0) {
          gEditor.endTransaction();
          return;
        }
        selecRange = selection.getRangeAt(0);
      }
    }
  } // end try
  catch (e) { }

  gEditor.endTransaction();
}

function doEnabling()
{
//  var findEditor = msiGetEditor(gReplaceDialog.findInput);
//  var findStr = null;
//  if (findEditor != null)
////    findStr = findEditor.outputToString("text/plain", 1024); // OutputLFLineBreak
//    findStr = getStringFromEditor(gReplaceDialog.findInput);
  if (gReplaceDialog.findContentFilter == null)
    gReplaceDialog.findContentFilter = new msiDialogEditorContentFilter(gReplaceDialog.findInput);
//  var findStr = gReplaceDialog.findContentFilter.getMarkupString();  - this is what it should be
  var findStr = gReplaceDialog.findContentFilter.getTextString();
  if (findStr.length <= 0)
    findStr = null;
//  var repStr = gReplaceDialog.replaceInput.value;  //Not used anyway - we'd probably want to check the serialized data more closely otherwise?
  gReplaceDialog.enabled = findStr;
  gReplaceDialog.findNext.disabled = !findStr;
  gReplaceDialog.replace.disabled = !findStr;
  gReplaceDialog.replaceAndFind.disabled = !findStr;
  gReplaceDialog.replaceAll.disabled = !findStr;
}
