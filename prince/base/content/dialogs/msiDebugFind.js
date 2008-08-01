/* -*- Mode: Java; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * ***** BEGIN LICENSE BLOCK *****
Copyright 2007 MacKichan Software, Inc.
 * ***** END LICENSE BLOCK ***** */

var gTestFindDialog;      // Quick access to document/form elements.
var gFindInst;           // nsIWebBrowserFind that we're going to use
var gFindService;        // Global service which remembers find params
var gEditor;             // the editor we're using

function initDialogObject()
{
  // Create gTestFindDialog object and initialize.
  gTestFindDialog = {};
  gTestFindDialog.findInput       = document.getElementById("findInput");
  gTestFindDialog.XPathInput      = document.getElementById("XPathInput");
  gTestFindDialog.caseSensitive   = document.getElementById("dialog.caseSensitive");
  gTestFindDialog.wrap            = document.getElementById("dialog.wrap");
  gTestFindDialog.searchBackwards = document.getElementById("dialog.searchBackwards");
  gTestFindDialog.checkXPath      = document.getElementById("checkXPath");
  gTestFindDialog.findNext        = document.getElementById("findNext");
  gTestFindDialog.findContentFilter = null;
  gTestFindDialog.findContentNodes = null;
  gTestFindDialog.xPathExpression = null;
  gTestFindDialog.textExpression = null;
  gTestFindDialog.xPathEval = null;
  gTestFindDialog.nsResolver = null;
  gTestFindDialog.bSearchExpressionChanged = true;  //start out with true
  gTestFindDialog.xPathFoundNodes = null;
  gTestFindDialog.mSearchState = null;
}

function msiEditorChangeObserver(editorElement)
{
  this.mEditorElement = editorElement;
  this.observe = function(aSubject, aTopic, aData)
  {
    // Should we allow this even if NOT the focused editor?
//    msiDumpWithID("In autoSubstituteDialog documentCreated observer for editor [@], observing [" + aTopic + "].\n", this.mEditorElement);
    if (!this.mEditorElement.docShell)
    {
      msiDumpWithID("In autoSubstituteDialog documentCreated observer for editor [@], returning as docShell is null.\n", this.mEditorElement);
      return;
    }
    var commandManager = msiGetCommandManager(this.mEditorElement);
    if (commandManager != aSubject)
    {
      msiDumpWithID("In msiDebugFind documentCreated observer for editor [@], observing [" + aTopic + "]; returning, as commandManager doesn't equal aSubject; aSubject is [" + aSubject + "], while commandManager is [" + commandManager + "].\n", this.mEditorElement);
//      if (commandManager != null)
        return;
    }

    switch(aTopic)
    {
      case "cmd_bold":
      case "cmd_setDocumentModified":
      {
        msiDumpWithID("In msiDebugFind command observer [" + aTopic + "] for editor [@]; calling doEnabling().\n", this.mEditorElement);
        doEnabling();
      }
      break;

      case "obs_documentCreated":
      {
        var bIsRealDocument = false;
        var currentURL = msiGetEditorURL(this.mEditorElement);
        msiDumpWithID("In msiDebugFind documentCreated observer for editor element [@], currentURL is " + currentURL + "].\n", this.mEditorElement);
        if (currentURL != null)
        {
          var fileName = GetFilename(currentURL);
          bIsRealDocument = (fileName != null && fileName.length > 0);
        }
        if (bIsRealDocument)
        {
          if (!gTestFindDialog.bEditorReady)
          {
            gTestFindDialog.bEditorReady = true;
          }
        }
//        else
          msiDumpWithID("In msiDebugFind documentCreated observer for editor [@], bIsRealDocument is false.\n", this.mEditorElement);
//        setControlsForSubstitution();
      }
      break;
    }
  };
}

function loadDialog()
{
  // Set initial dialog field contents.
  // Set initial dialog field contents. Use the gFindInst attributes first,
  // this is necessary for window.find()
//  gTestFindDialog.findInput.value         = (gFindInst.searchString
//                                            ? gFindInst.searchString
//                                            : gFindService.searchString);
//  gTestFindDialog.replaceInput.value = gFindService.replaceString;

  var theStringSource = (gFindInst.searchString ? gFindInst.searchString
                                                : gFindService.searchString);
  if (theStringSource != null && theStringSource.length == 0)
    theStringSource = null;

  gTestFindDialog.bEditorReady = false;
  var substitutionControlObserver = new msiEditorChangeObserver(gTestFindDialog.findInput);
  var commandBoldObserverData = new Object();
  commandBoldObserverData.mCommand = "cmd_bold";
  commandBoldObserverData.mObserver = substitutionControlObserver;
  var commandSetModifiedObserverData = new Object();
  commandSetModifiedObserverData.mCommand = "cmd_setDocumentModified";
  commandSetModifiedObserverData.mObserver = substitutionControlObserver;
  var editorDocLoadedObserverData = new Object();
  editorDocLoadedObserverData.mCommand = "obs_documentCreated";
  editorDocLoadedObserverData.mObserver = substitutionControlObserver;

  gTestFindDialog.findInput.mInitialDocObserver = [commandSetModifiedObserverData, editorDocLoadedObserverData, commandBoldObserverData];

  msiInitializeEditorForElement(gTestFindDialog.findInput, theStringSource, true);

//  var theStringSource2 = gFindService.replaceString;
//  if (theStringSource2 != null && theStringSource2.length == 0)
//    theStringSource2 = null;
////  msiInitializeEditorForElement(gTestFindDialog.replaceInput, theStringSource2, true);
//  var editorInitializer = new msiEditorArrayInitializer();
//  editorInitializer.addEditorInfo(gTestFindDialog.findInput, theStringSource, true);
//  editorInitializer.addEditorInfo(gTestFindDialog.replaceInput, theStringSource2, true);
//  editorInitializer.doInitialize();

//  gTestFindDialog.replaceInput.makeEditable("html", false);


  gTestFindDialog.caseSensitive.checked   = (gFindInst.matchCase
                                            ? gFindInst.matchCase
                                            : gFindService.matchCase);
  gTestFindDialog.wrap.checked            = (gFindInst.wrapFind
                                            ? gFindInst.wrapFind
                                            : gFindService.wrapFind);
  gTestFindDialog.searchBackwards.checked = (gFindInst.findBackwards
                                            ? gFindInst.findBackwards
                                            : gFindService.findBackwards);

  gTestFindDialog.tabOrderArray = new Array( gTestFindDialog.findInput, gTestFindDialog.XPathInput, gTestFindDialog.caseSensitive,
                                       gTestFindDialog.wrap, gTestFindDialog.searchBackwards,
                                       gTestFindDialog.findNext, 
                                       document.documentElement.getButton("cancel") );
  doEnabling();
}

function onLoad()
{
  // Get the xul <editor> element:
  var editorElement = window.arguments[0];

  // If we don't get the editor, then we won't allow replacing.
//  dump("w");
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

  // Init gTestFindDialog.
  initDialogObject();
  gTestFindDialog.mTargetEditorElement = editorElement;

  // Change "OK" to "Find".
  //dialog.find.label = document.getElementById("fBLT").getAttribute("label");

  // Fill dialog.
  loadDialog();

//  if (gTestFindDialog.findInput.value)
//    gTestFindDialog.findInput.select();
//  else
    gTestFindDialog.findInput.focus();
}

function onUnload() {
  // Disconnect context from this dialog.
//  gFindReplaceData.replaceDialog = null;
}

function saveFindData()
{
  // Set data attributes per user input.
  if (gFindService)
  {
////    gFindService.searchString  = gTestFindDialog.findInput.value;
//    var serializer = new XMLSerializer();
//    gFindService.searchString = serializer.serializeToString(gTestFindDialog.findInput.contentDocument.documentElement);

    if (gTestFindDialog.findContentFilter == null)
      gTestFindDialog.findContentFilter = new msiDialogEditorContentFilter(gTestFindDialog.findInput);
    gFindService.searchString = gTestFindDialog.findContentFilter.getDocumentFragmentString();

    gFindService.matchCase     = gTestFindDialog.caseSensitive.checked;
    gFindService.wrapFind      = gTestFindDialog.wrap.checked;
    gFindService.findBackwards = gTestFindDialog.searchBackwards.checked;
  }
}

function retrieveSearchExpression()
{
  if (gTestFindDialog.findContentFilter == null)
    gTestFindDialog.findContentFilter = new msiDialogEditorContentFilter(gTestFindDialog.findInput);
//  gFindInst.searchString = gTestFindDialog.findContentFilter.getTextString();
  return gTestFindDialog.findContentFilter.getXMLNodesAsDocFragment();
}

function setUpFindInst()
{
//  gFindInst.searchString  = gTestFindDialog.findInput.value;
//  var serializer = new XMLSerializer();
//  gFindInst.searchString = serializer.serializeToString(gTestFindDialog.findInput.contentDocument.documentElement);
  //This is a temporary hack - we need to be using the tags in the find information.

//  gFindInst.searchString = getStringFromDialogEditor(gTestFindDialog.findInput);

  var theString = "";
  if (gTestFindDialog.findContentNodes != null)
  {
    var xmlSerializer = new XMLSerializer();
    for (var ix = 0; ix < gTestFindDialog.findContentNodes.childNodes.length; ++ix)
      theString += xmlSerializer.serializeToString(gTestFindDialog.findContentNodes.childNodes[ix]);
  }
  dump("In msiDebugFind.js, setUpFindInst(), setting gFindInst.searchString to [\n  " + theString + "\n].\n");
  gFindInst.searchString = theString;
//  gFindInst.searchString = gTestFindDialog.findContentFilter.getMarkupString();  //this is what it should be - above is just here to remove one level of redundant work

  gFindInst.matchCase     = gTestFindDialog.caseSensitive.checked;
  gFindInst.wrapFind      = gTestFindDialog.wrap.checked;
  gFindInst.findBackwards = gTestFindDialog.searchBackwards.checked;
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

    //The data depending on "current state" of the search rather than the search string will be kept in gTestFindDialog.mSearchState.
    //Its data will consist of:
    //  mSearchState.direction = 1 for forwards, 0 for backwards?
    //  mSearchState.xPathAxis - the "axis" string to be used in an XPath search; generally "following::" or "preceding::" if
    //    we were able to get a context node just before the selection start (for forward searches) or after 
    //    the selection end (for backwards searches)
    //  mSearchState.xPathContextNode - the node to set the context for the XPath search.
    //  mSearchState.globalRange  - contains start and end of whole search unless pattern is changed. If the initial selection is
    //    collapsed, these will be the same, of course.
    //  mSearch.currentRange - 

function msiXPathSearchInstanceState(theEditor, theFlags)
{
  this.mEditor = theEditor;
  var selection = theEditor.selection;
  var selecRange;
  if (selection.rangeCount > 0)
    selecRange = selection.getRangeAt(0);
  this.globalRange = selecRange.cloneRange();
  this.flags = theFlags;
  this.xPathFoundNodes = null;
  this.xPathCurrFoundNodeIndex = -1;
  this.bFirstPassDone = false;
  this.bSecondPassDone = false;
  this.nextAxisStr = null;
  this.nextConditionStr = null;
  this.nextContextNode = null;

  this.backwards = function()
  {
    return (this.flags != null) && (this.flags.indexOf("b") >= 0);
  };

  this.setBackwards = function(bDoSet)
  {
    if (this.backwards() != bDoSet)
    {
      if (bDoSet)
        this.flags += "b";
      else
        this.flags.replace("b", "", "gi");
    }
  };

  this.wrapping = function()
  {
    return (this.flags != null) && (this.flags.indexOf("w") >= 0);
  };

  this.setWrapping = function(bDoSet)
  {
    if (this.wrapping() != bDoSet)
    {
      if (bDoSet)
        this.flags += "w";
      else
        this.flags.replace("w", "", "gi");
    }
  };

  this.collapsed = function()
  {
    return (this.globalRange == null) || (this.globalRange.collapsed);
  };

  this.retrieveNextXPathResultNode = function()
  {
    var result = null;
    if (this.xPathFoundNodes != null)
    {
      if (this.backwards())
      {
        while ( (result == null) && ((--this.xPathCurrFoundNodeIndex) >= 0) )
        {
          result = this.xPathFoundNodes.snapshotItem(this.xPathCurrFoundNodeIndex);
          //Here we need to test result and see whether it's been deleted by a previous replace operation. How to do that?
  //        if ((result != null) && (result.
        } 
      }
      else
      {
        while ( (result == null) && ((++this.xPathCurrFoundNodeIndex) < this.xPathFoundNodes.snapshotLength) )
        {
          result = this.xPathFoundNodes.snapshotItem(this.xPathCurrFoundNodeIndex);
          //Here we need to test result and see whether it's been deleted by a previous replace operation. How to do that?
        }
      }
    }
    return result;
  };

  this.prepareFirstXPathIteration = function()
  {
    var retVal = false;
    var startNode = null;
    var axisStr = ".//";
    var additionalCondition = "";
    var parentNode = null;
    if (this.backwards())
    {
      startNode = msiNavigationUtils.getNodeAfterPosition(this.globalRange.endContainer, this.globalRange.endOffset);
      if (startNode != null)
        axisStr = "preceding::";
    }
    else
    {
      startNode = msiNavigationUtils.getNodeBeforePosition(this.globalRange.startContainer, this.globalRange.startOffset);
      if (startNode != null)
        axisStr = "following::";
    }
    if (startNode == null)
    {
      startNode = msiNavigationUtils.getRootDocumentNode();
      axisStr = "descendant-or-self::";
    }
    if (!this.collapsed())
    {
      parentNode = this.globalRange.commonAncestorContainer;
      additionalCondition = "[ancestor::*[local-name=\"" + msiGetBaseNodeName(parentNode) + "\"]]";
    }
    this.nextAxisStr = axisStr;
    this.nextConditionStr = additionalCondition;
    this.nextContextNode = startNode;
//    this.bFirstPassDone = true;
    return true;
  };

  this.prepareNextXPathIteration = function()
  {
    var retVal = false;
    var axisStr = "";
    var conditionStr = "";
    var endNode = null;
    if (!this.bFirstPassDone)
      retVal = this.prepareFirstXPathIteration();

    if (!retVal && this.wrapping() && this.collapsed() && !this.bSecondPassDone)
    {
      if (this.backwards())
      {
        endNode = msiNavigationUtils.getNodeBeforePosition(this.globalRange.endContainer, this.globalRange.endOffset);
        if (endNode != null)
          axisStr = "following::";
      }
      else
      {
        endNode = msiNavigationUtils.getNodeAfterPosition(this.globalRange.startContainer, this.globalRange.startOffset);
        if (endNode != null)
          axisStr = "preceding::";
      }
      if (endNode == null)
        return false;
      this.nextAxisStr = axisStr;
      this.nextConditionStr = conditionStr;
      this.nextContextNode = endNode;
      retVal = true;
//      this.bSecondPassDone = true;
      dump("In msiDebugFind.js, in msiXPathSearchInstanceState.prepareNextXPathIteration, doing second pass; axisStr is {" + axisStr + "}.\n");
    }
    return retVal;
  };

  this.doEvaluateXPath = function(xPathExpression)
  {
    this.xPathFoundNodes = evaluateXPath(this.nextContextNode, xPathExpression);
    if (this.backwards())
      this.xPathCurrFoundNodeIndex = this.xPathFoundNodes.snapshotLength;
    else
      this.xPathCurrFoundNodeIndex = -1;
    if (this.bFirstPassDone)
    {
      if (!this.wrapping())
        dump("Problem in msiDebugFind.js, msiXPathSearchInstanceState.doEvaluateXPath; bFirstPassDone is true and wrapping is false, so why are we here?\n");
      this.bSecondPassDone = true;
    }
    else
      this.bFirstPassDone = true;
  };

  this.getAxisString = function()
  {
    return this.nextAxisStr;
  };

  this.getConditionString = function()
  {
    return this.nextConditionStr;
  };

  this.getContextNode = function()
  {
    return this.nextContextNode;
  };
}

function setUpSearchState()
{
  var theFlags = "";
  if (gTestFindDialog.searchBackwards.checked)
    theFlags += "b";
  if (gTestFindDialog.wrap.checked)
    theFlags += "w";
  gTestFindDialog.mSearchState = new msiXPathSearchInstanceState(gEditor, theFlags);
  gTestFindDialog.mSearchState.prepareNextXPathIteration();
  //Another thing to watch for - if we wrap around, the search may have to actually proceed past the original cursor position to
  //  check for an entire match. How to set up the start and end positions for the second time?

//        START HERE - TRY TO FIND PREVIOUS NODE TO USE following:: AXIS, OTHERWISE USE DOCUMENT NODE?
//        Trouble is that this has to be determined before the expression can be generated properly??
//        //Also note that the starting cursor position may be in the middle of a Text node, or in a run
//        //  of text within an object also containing other types of nodes (like a paragraph containing
//        //  math). If there's always a Text node to work from in this case, the algorithm of backing up
//        //  to get a node from which we can use "following::" will probably work okay. Otherwise, we'll
//        //  have to use the document root search ".//".
//        //The other issue is how to prevent searching, for instance, Front Matter.??
//      }
//      else if ( (aNode.childNodes != null) && (aNode.childNodes.length > selecRange.startoffset) )
//      {
//      }
//    }
}

function getNextXPathResultNode()
{
  if (gTestFindDialog.mSearchState == null)
    onCheckXPath();

  dump("In msiDebugFind.js, getNextXPathResultNode, value of XPath textbox is {" + gTestFindDialog.XPathInput.value + "}.\n");

  var resultNode = gTestFindDialog.mSearchState.retrieveNextXPathResultNode();
  if (resultNode == null)
  {
    if (!gTestFindDialog.mSearchState.prepareNextXPathIteration())
      dump("Error in msiDebugFind.js, getNextXPathResultNode; prepareNextXPathIteration() is failing.\n");
    else
    {
      onCheckXPath();
      gTestFindDialog.mSearchState.doEvaluateXPath(gTestFindDialog.xPathExpression);
      resultNode = gTestFindDialog.mSearchState.retrieveNextXPathResultNode();
    }
  }
  return resultNode;
}

function onFindNext()
{
  // Transfer dialog contents to the find service.
  saveFindData();

  var newRange;
  var result = null;
  if (gTestFindDialog.bSearchExpressionChanged)
    onCheckXPath();

  if (gTestFindDialog.xPathExpression != null)
  {
//    var selection = gEditor.selection;
//    var selecRange;
//    if (selection.rangeCount > 0)
//      selecRange = selection.getRangeAt(0);
//    var aNode = selecRange.startContainer;
//    if (!msiNavigationUtils.positionIsAtEnd(aNode, selecRange.startOffset))
//    {
//      if (msiNavigationUtils.positionIsAtStart(aNode, selecRange.startOffset))
//      {
//        START HERE - TRY TO FIND PREVIOUS NODE TO USE following:: AXIS, OTHERWISE USE DOCUMENT NODE?
//        Trouble is that this has to be determined before the expression can be generated properly??
//        //Also note that the starting cursor position may be in the middle of a Text node, or in a run
//        //  of text within an object also containing other types of nodes (like a paragraph containing
//        //  math). If there's always a Text node to work from in this case, the algorithm of backing up
//        //  to get a node from which we can use "following::" will probably work okay. Otherwise, we'll
//        //  have to use the document root search ".//".
//        //The other issue is how to prevent searching, for instance, Front Matter.??
//      }
//      else if ( (aNode.childNodes != null) && (aNode.childNodes.length > selecRange.startoffset) )
//      {
//      }
//    }

    //onFindNext may be called in the middle of processing a list of returned XPath nodes - check for that first
    result = getNextXPathResultNode();
    if (result != null)
    {
      newRange = gEditor.document.createRange();
      newRange.selectNode(result);
      gEditor.selection.removeAllRanges();
      gEditor.selection.addRange(newRange);
    }
  }
  else  //must be a text search
  {
    // set up the find instance
    setUpFindInst();

    // Search.
    result = gFindInst.findNext();
  }

  if (!result)
  {
    gTestFindDialog.mSearchState = null;
    gTestFindDialog.mSearchManager = null;
    var bundle = document.getElementById("findBundle");
    AlertWithTitle(null, bundle.getString("notFoundWarning"));
    SetTextboxFocus(gTestFindDialog.findInput);
//    gTestFindDialog.findInput.select();
    gTestFindDialog.findInput.focus();
    return false;
  } 
  return true;
}

function flagsHaveChanged()
{
  var retVal = false;
  if (gTestFindDialog.mSearchManager != null)
  {
    if (gTestFindDialog.mSearchManager.isCaseInsensitive() == gTestFindDialog.caseSensitive.checked)
    {
      retVal = true;
      gTestFindDialog.mSearchManager.setCaseInsensitive(!gTestFindDialog.caseSensitive.checked);
    }
  }
  else
    retVal = true;  //no mSearchManager
  if (gTestFindDialog.mSearchState != null)
  {
    if (gTestFindDialog.mSearchState.backwards() != gTestFindDialog.searchBackwards.checked)
    {
      retVal = true;
      gTestFindDialog.mSearchState.setBackwards(gTestFindDialog.searchBackwards.checked);
    }
    if (gTestFindDialog.mSearchState.wrapping() != gTestFindDialog.wrap.checked)
    {
      retVal = true;
      gTestFindDialog.mSearchState.setWrapping(gTestFindDialog.wrap.checked);
    }
  }
  else
    retVal = true;
  
  return retVal;
}

function onCheckXPath()
{
  if (!gEditor)
    return false;
  
  if (gTestFindDialog.mSearchManager == null)
  {
    if (!gTestFindDialog.bSearchExpressionChanged)
      dump("In msiDebugFind.js, onCheckXPath(), mSearchManager is uninitialized but bSearchExpressionChanged shows false!\n");
    gTestFindDialog.bSearchExpressionChanged = true;
  }

  gTestFindDialog.bSearchExpressionChanged = gTestFindDialog.bSearchExpressionChanged || flagsHaveChanged();
    
  if (gTestFindDialog.bSearchExpressionChanged)
  {

//    var specStr = msiGetEditor(gTestFindDialog.findInput).outputToString("text/plain", 1024); // OutputLFLineBreak
    gTestFindDialog.findContentNodes = retrieveSearchExpression();
    var searchFlags = "";
    var searchRange = null;

    setUpSearchState();
    //The data depending on "current state" of the search rather than the search string will be kept in gTestFindDialog.mSearchState.
    //Its data will consist of:
    //  mSearchState.direction = 1 for forwards, 0 for backwards?
    //  mSearchState.xPathAxis - the "axis" string to be used in an XPath search; generally "following::" or "preceding::" if
    //    we were able to get a context node just before the selection start (for forward searches) or after 
    //    the selection end (for backwards searches)
    //  mSearchState.xPathContextNode - the node to set the context for the XPath search.
    //  mSearchState.globalRange  - contains start and end of whole search unless pattern is changed. If the initial selection is
    //    collapsed, these will be the same, of course.
//    var axisStr = determineXPathAxisString();

    if (!gTestFindDialog.caseSensitive.checked)
      searchFlags += "i";
    gTestFindDialog.mSearchManager = new msiSearchManager(gTestFindDialog.mTargetEditorElement, gTestFindDialog.findContentNodes, searchFlags, searchRange);
  }
  else if (gTestFindDialog.mSearchState == null)
  {
    dump("In msiDebugFind.js, in onCheckXPath(); gTestFindDialog.mSearchState is null but bSearchExpressionChanged is false!");
    setUpSearchState();
  }

  dump("In msiDebugFind.js, in onCheckXPath(), calling mSearchManager.setUpSearch with axis string {" + gTestFindDialog.mSearchState.getAxisString() + "}.\n");
  gTestFindDialog.mSearchManager.setUpSearch(gTestFindDialog.mSearchState.getAxisString(), gTestFindDialog.mSearchState.getConditionString());

  var theString = gTestFindDialog.mSearchManager.getXPathSearchString();
  if ( (theString == null) || (theString.length == 0) )
  {
    gTestFindDialog.xPathExpression = null;
    gTestFindDialog.textExpression = theString = gTestFindDialog.mSearchManager.getTextSearchString();
  }
  else
  {
    gTestFindDialog.xPathExpression = theString;
    gTestFindDialog.textExpression = null;
  }
  var serializedFrag = gTestFindDialog.findContentFilter.getDocumentFragmentString();
  dump("In msiDebugFind.js, onCheckXPath(); search string is:\n  " + theString + "\n\n; contents of search window were\n  " + serializedFrag + "\n\n");
  gTestFindDialog.bSearchExpressionChanged = false;
  gTestFindDialog.XPathInput.value = theString;
}
//  var specStr = gTestFindDialog.findContentFilter.getTextString();

  // Unfortunately, because of whitespace we can't just check
  // whether (selStr == specStr), but have to loop ourselves.
  // N chars of whitespace in specStr can match any M >= N in selStr.


function checkFoundMatch()
{
//THE STUFF HERE NEEDS TO BE REPLACED BY SOMETHING STRONGER!!!
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
  if (gTestFindDialog.searchBackwards.checked && selection.rangeCount > 0)
  {
    newRange = selection.getRangeAt(0).cloneRange();
    newRange.collapse(true);
  }

  // nsPlaintextEditor::InsertText fails if the string is empty,
  // so make that a special case:
//  var serializer = new XMLSerializer();
//  var replStr = serializer.serializeToString(gTestFindDialog.replaceInput.contentDocument.documentElement);
//  var replStr = getStringFromEditor(gTestFindDialog.replaceInput);
//  var replNodes = gTestFindDialog.replaceContentFilter.getXMLNodesAsDocFragment();
//rwa  var replFrag = gTestFindDialog.replaceContentFilter.getContentsAsDocumentFragment();
//  var replString = gTestFindDialog.replaceContentFilter.getDocumentFragmentString();
  var replString = gTestFindDialog.replaceContentFilter.getMarkupString();

//rwa  if (replStr == "")
    gEditor.deleteSelection(0);
  if (replString != null && replString.length > 0)
  {
    dump("In msiDebugFind.onReplace(), inserting HTML [" + replString + "].\n");
    gEditor.insertHTMLWithContext(replString, null, null, "text/html", null,null,0,true);
  }
//rwa    insertXMLNodesAtCursor(gEditor, replFrag.childNodes, true);
//rwa    insertXMLNodesAtCursor(gEditor, replNodes, true);
//rwa  else
//rwa    insertXMLAtCursor(gEditor, replStr, bIsSinglePara, true);


////  else if (gEditor.editortype == "html" || gEditor.editortype == "htmlmail")
//  else if ("insertHTML" in gEditor)
//    gEditor.insertHTML(replStr);
//  else
//    gEditor.insertText(replStr);

  // For reverse finds, need to move caret just before the replaced text
  if (gTestFindDialog.searchBackwards.checked && newRange)
  {
    gEditor.selection.removeAllRanges();
    gEditor.selection.addRange(newRange);
  }

  return true;
}

//function onReplaceAll()
//{
//  if (!gEditor)
//    return;
//
//  setUpFindInst();  //added this so we can use it
//  // Transfer dialog contents to the find service.
//  saveFindData();
//
//  var findStr = gFindInst.searchString;
////  var serializer = new XMLSerializer();
////  var repStr = serializer.serializeToString(gTestFindDialog.replaceInput.contentDocument.documentElement);
//  if (gTestFindDialog.replaceContentFilter == null)
//    gTestFindDialog.replaceContentFilter = new msiDialogEditorContentFilter(gTestFindDialog.replaceInput);
//
////  var repStr = getStringFromEditor(gTestFindDialog.replaceInput);
////  var repNodes = gTestFindDialog.replaceContentFilter.getXMLNodesAsDocFragment();
////rwa  var repFrag = gTestFindDialog.replaceContentFilter.getContentsAsDocumentFragment();
////  var repString = gTestFindDialog.replaceContentFilter.getDocumentFragmentString();
//  var replString = gTestFindDialog.replaceContentFilter.getMarkupString();
////  var repStr = gTestFindDialog.replaceInput.value;
//
//  var finder = Components.classes["@mozilla.org/embedcomp/rangefind;1"].createInstance().QueryInterface(Components.interfaces.nsIFind);
//
//  finder.caseSensitive = gTestFindDialog.caseSensitive.checked;
//  finder.findBackwards = gTestFindDialog.searchBackwards.checked;
//
//  // We want the whole operation to be undoable in one swell foop,
//  // so start a transaction:
//  gEditor.beginTransaction();
//
//  // and to make sure we close the transaction, guard against exceptions:
//  try {
//    // Make a range containing the current selection, 
//    // so we don't go past it when we wrap.
//    var selection = gEditor.selection;
//    var selecRange;
//    if (selection.rangeCount > 0)
//      selecRange = selection.getRangeAt(0);
//    var origRange = selecRange.cloneRange();
//
//    // We'll need a range for the whole document:
//    var wholeDocRange = gEditor.document.createRange();
//    var rootNode = gEditor.rootElement.QueryInterface(Components.interfaces.nsIDOMNode);
//    wholeDocRange.selectNodeContents(rootNode);
//
//    // And start and end points:
//    var endPt = gEditor.document.createRange();
//
//    if (gTestFindDialog.searchBackwards.checked)
//    {
//      endPt.setStart(wholeDocRange.startContainer, wholeDocRange.startOffset);
//      endPt.setEnd(wholeDocRange.startContainer, wholeDocRange.startOffset);
//    }
//    else
//    {
//      endPt.setStart(wholeDocRange.endContainer, wholeDocRange.endOffset);
//      endPt.setEnd(wholeDocRange.endContainer, wholeDocRange.endOffset);
//    }
//
//    // Find and replace from here to end (start) of document:
//    var foundRange;
//    var searchRange = wholeDocRange.cloneRange();
//    while ((foundRange = finder.Find(findStr, searchRange,
//                                     selecRange, endPt)) != null)
//    {
//      gEditor.selection.removeAllRanges();
//      gEditor.selection.addRange(foundRange);
//
//      // The editor will leave the caret at the end of the replaced text.
//      // For reverse finds, we need it at the beginning,
//      // so save the next position now.
//      if (gTestFindDialog.searchBackwards.checked)
//      {
//        selecRange = foundRange.cloneRange();
//        selecRange.setEnd(selecRange.startContainer, selecRange.startOffset);
//      }
//
//      // nsPlaintextEditor::InsertText fails if the string is empty,
//      // so make that a special case:
////rwa      if (repStr == "")
//        gEditor.deleteSelection(0);
////rwa//      else if (gEditor.editortype == "html" || gEditor.editortype == "htmlmail")
////rwa      else if ("insertHTML" in gEditor)
////rwa        gEditor.insertHTML(repStr);
////rwa      else
////rwa        gEditor.insertText(repStr);
//      if (replString != null && replString.length > 0)
//        gEditor.insertHTMLWithContext(replString, null, null, "text/html", null,null,0,true);
////rwa        if (repFrag != null)
////rwa          insertXMLNodesAtCursor(gEditor, repFrag.childNodes, true);
////rwa        insertXMLNodesAtCursor(gEditor, replNodes, true);
//
//      // If we're going forward, we didn't save selecRange before, so do it now:
//      if (!gTestFindDialog.searchBackwards.checked)
//      {
//        selection = gEditor.selection;
//        if (selection.rangeCount <= 0) {
//          gEditor.endTransaction();
//          return;
//        }
//        selecRange = selection.getRangeAt(0).cloneRange();
//      }
//    }
//
//    // If no wrapping, then we're done
//    if (!gTestFindDialog.wrap.checked) {
//      gEditor.endTransaction();
//      return;
//    }
//
//    // If wrapping, find from start/end of document back to start point.
//    if (gTestFindDialog.searchBackwards.checked)
//    {
//      // Collapse origRange to end
//      origRange.setStart(origRange.endContainer, origRange.endOffset);
//      // Set current position to document end
//      selecRange.setEnd(wholeDocRange.endContainer, wholeDocRange.endOffset);
//      selecRange.setStart(wholeDocRange.endContainer, wholeDocRange.endOffset);
//    }
//    else
//    {
//      // Collapse origRange to start
//      origRange.setEnd(origRange.startContainer, origRange.startOffset);
//      // Set current position to document start
//      selecRange.setStart(wholeDocRange.startContainer,
//                          wholeDocRange.startOffset);
//      selecRange.setEnd(wholeDocRange.startContainer, wholeDocRange.startOffset);
//    }
//
//    while ((foundRange = finder.Find(findStr, wholeDocRange,
//                                     selecRange, origRange)) != null)
//    {
//      gEditor.selection.removeAllRanges();
//      gEditor.selection.addRange(foundRange);
//
//      // Save insert point for backward case
//      if (gTestFindDialog.searchBackwards.checked)
//      {
//        selecRange = foundRange.cloneRange();
//        selecRange.setEnd(selecRange.startContainer, selecRange.startOffset);
//      }
//
//      // nsPlaintextEditor::InsertText fails if the string is empty,
//      // so make that a special case:
////rwa      if (repStr == "")
//        gEditor.deleteSelection(0);
////rwa//      else if (gEditor.editortype == "html" || gEditor.editortype == "htmlmail")
////rwa      else if ("insertHTML" in gEditor)
////rwa        gEditor.insertHTML(repStr);
////rwa      else
////rwa        gEditor.insertText(repStr);
//      insertXMLNodesAtCursor(gEditor, replNodes, true);
//
//      // Get insert point for forward case
//      if (!gTestFindDialog.searchBackwards.checked)
//      {
//        selection = gEditor.selection;
//        if (selection.rangeCount <= 0) {
//          gEditor.endTransaction();
//          return;
//        }
//        selecRange = selection.getRangeAt(0);
//      }
//    }
//  } // end try
//  catch (e) { }
//
//  gEditor.endTransaction();
//}

function doEnabling()
{
//  var findEditor = msiGetEditor(gTestFindDialog.findInput);
//  var findStr = null;
//  if (findEditor != null)
////    findStr = findEditor.outputToString("text/plain", 1024); // OutputLFLineBreak
//    findStr = getStringFromEditor(gTestFindDialog.findInput);
  if (gTestFindDialog.findContentFilter == null)
    gTestFindDialog.findContentFilter = new msiDialogEditorContentFilter(gTestFindDialog.findInput);
//  var findStr = gTestFindDialog.findContentFilter.getMarkupString();  - this is what it should be
  var findStr = gTestFindDialog.findContentFilter.getTextString();
  dump("In msiDebugFind.doEnabling, findStr was [" + findStr + "].\n");
  if (findStr.length <= 0)
    findStr = null;
//  var repStr = gTestFindDialog.replaceInput.value;  //Not used anyway - we'd probably want to check the serialized data more closely otherwise?
  gTestFindDialog.enabled = findStr;
  gTestFindDialog.findNext.disabled = !findStr;
  gTestFindDialog.bSearchExpressionChanged = true;  //start out with true
//  gTestFindDialog.replace.disabled = !findStr;
//  gTestFindDialog.replaceAndFind.disabled = !findStr;
//  gTestFindDialog.replaceAll.disabled = !findStr;
}


//We want to arrange the search so the following will match:

//I. The expression "Just some words and stuff" should match:
//   1. <em>Just some</em> words and <sf>stuff and then going on</sf>
//   2. Just some</para></part><section><para>words and <em>stuff and then more</em>
//   3. Just some <em>words and <bold>stu</bold>f</em>f and then more

//II. The expression "<math><mfrac><mi>a</mi><mi>b</mi></mfrac></math>" should match:
//   1. <msqrt><mrow><mn>3</mn><mo>+</mo><mfrac><mi>a</mi><mi>b</mi></mfrac></mrow></msqrt>
//   2. <mfrac><mrow><mi>a</mi><mo>-<mo><mn>2</mn></mrow><mi>b</mi></mfrac>
//Here are some of the trials I've done in Mozilla's XPath Evaluator:
//    //mfrac[contains(string(.[1]),"a")] - correctly found the mfrac
//    //mfrac//*[string-length()>0][1][contains(string(),"a")] - correctly found the mi
//    //mfrac[.//*[string-length()>0][1][contains(string(),"a")]] - correctly found the mfrac, as also did
//    //mfrac[./*[string-length()>0][1][contains(string(),"a")]] - (using direct child single-/ instead of descendant "//"
//   //mfrac[./*[string-length()>0][1][contains(string(),"a")]][./*[string-length()>0][2][contains(string(),"b")]] - correctly found the mfrac
//   The last one worked also on #2. above.
//   //mfrac[./*[string-length()>0][1]/mi[contains(string(),"a")]][./*[string-length()>0][2][contains(string(),"b")]] - worked on #2

//More results:
//  (//msqrt|//mroot|//mo)//mi - this sort of formulation will extract child mi's.

//Let's get together a list. To recognize either a square root or a mroot that is a square root, could use:
//  (//msqrt|//mroot[./*[string-length(normalize-space())>0][2][(@tempinput="true") or (string()="2")]])
//  or perhaps better:
//  (//msqrt|//mroot[(./mi|./mn|./mrow)[2][(@tempinput="true") or (string()="2")]])
// or perhaps best:
//  //msqrt|(//mroot[(./mi|./mn|./mrow|./msqrt|./mroot|./mstyle|./mfrac|./mo)[2][(@tempinput="true") or (string()="2")]])

//We may want to use XSLT since it's more powerful. Unfortunately, it wouldn't be as amenable to repeated
//  uses; also, wouldn't this make it necessary to introduce a lot of extraneous id's into the document?
//Following copied from Mozilla MDC Using XPath documentation. Now,
//  how to use it here remains a bit of a mystery.
// Evaluate an XPath expression aExpression against a given DOM node
// or Document object (aNode), returning the results as an array
// thanks wanderingstan at morethanwarm dot mail dot com for the
// initial work.
//var XPathResult = Components.interfaces.XPathResult;

function evaluateXPath(aNode, aExpr)
{
  if (!gTestFindDialog.xPathEval)
  {
    gTestFindDialog.xPathEval = new XPathEvaluator();
    gTestFindDialog.nsResolver = gTestFindDialog.xPathEval.createNSResolver(aNode.ownerDocument == null ?
                                             aNode.documentElement : aNode.ownerDocument.documentElement);
  }

  var result = gTestFindDialog.xPathEval.evaluate(aExpr, aNode, gTestFindDialog.nsResolver, XPathResult.ORDERED_NODE_SNAPSHOT_TYPE, null);
  return result;
//  var found = [];
//  var res;
//  while (res = result.iterateNext())
//    found.push(res);
//  return found;
}

function prepareXPathExpression(aNodeSet)
{

}