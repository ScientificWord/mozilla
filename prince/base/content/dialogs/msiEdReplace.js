/* -*- Mode: Java; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * ***** BEGIN LICENSE BLOCK *****
Copyright 2007 MacKichan Software, Inc.
 * ***** END LICENSE BLOCK ***** */

var gReplaceDialog;      // Quick access to document/form elements.
var gFindInst;           // nsIWebBrowserFind that we're going to use
var gFindService;        // Global service which remembers find params
var gEditor;             // the editor we're using
var gStringBundle;

function initDialogObject()
{
  // Create gReplaceDialog object and initialize.
  var strBundleService =
    Components.classes["@mozilla.org/intl/stringbundle;1"].getService(); 
  strBundleService = 
    strBundleService.QueryInterface(Components.interfaces.nsIStringBundleService);
  gStringBundle = strBundleService.createBundle('chrome://prince/locale/editor.properties');
  gReplaceDialog = {};
  gReplaceDialog.findInput       = document.getElementById("findInput");
  gReplaceDialog.replaceInput    = document.getElementById("replaceInput");
  gReplaceDialog.caseSensitive   = document.getElementById("dialog.caseSensitive");
  gReplaceDialog.wrap            = document.getElementById("dialog.wrap");
  gReplaceDialog.searchBackwards = document.getElementById("dialog.searchBackwards");
  gReplaceDialog.entireWord      = document.getElementById("dialog.entireWord");
  gReplaceDialog.findNext        = document.getElementById("findNext");
  gReplaceDialog.replace         = document.getElementById("replace");
  gReplaceDialog.replaceAndFind  = document.getElementById("replaceAndFind");
  gReplaceDialog.replaceAll      = document.getElementById("replaceAll");
//  gReplaceDialog.multiParaCheckbox = document.getElementById("multiParaCheckbox");

  // gReplaceDialog.findContentFilter = null;
  // gReplaceDialog.replaceContentFilter = null;
  // gReplaceDialog.findContentNodes = null;
  // gReplaceDialog.xPathExpression = null;
  // gReplaceDialog.textExpression = null;      
  // gReplaceDialog.xPathEval = null;
  // gReplaceDialog.nsResolver = null;
  // gReplaceDialog.bSearchExpressionChanged = true;  //start out with true
  // gReplaceDialog.xPathFoundNodes = null;
  // gReplaceDialog.mSearchState = null;
}

function msiEditorChangeObserver(editorElement)
{
  this.mEditorElement = editorElement;
  return;
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
      msiDumpWithID("In msiEdReplace documentCreated observer for editor [@], observing [" + aTopic + "]; returning, as commandManager doesn't equal aSubject; aSubject is [" + aSubject + "], while commandManager is [" + commandManager + "].\n", this.mEditorElement);
//      if (commandManager != null)
        return;
    }

    switch(aTopic)
    {
      case "cmd_bold":
      case "cmd_setDocumentModified":
      {
        msiDumpWithID("In msiEdReplace command observer [" + aTopic + "] for editor [@]; calling doEnabling().\n", this.mEditorElement);
        //doEnabling();
      }
      break;

      case "obs_documentCreated":
      {
        var bIsRealDocument = false;
        var currentURL = msiGetEditorURL(this.mEditorElement);
        msiDumpWithID("In msiEdReplace documentCreated observer for editor element [@], currentURL is " + currentURL + "].\n", this.mEditorElement);
        if (currentURL !== null)
        {
          var fileName = GetFilename(currentURL);
          bIsRealDocument = (fileName != null && fileName.length > 0);
        }
        if (bIsRealDocument)
        {
          if (!gReplaceDialog.bEditorReady)
          {
            gReplaceDialog.bEditorReady = true;
          }
        }
//        else
          msiDumpWithID("In msiEdReplace documentCreated observer for editor [@], bIsRealDocument is false.\n", this.mEditorElement);
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
//  gReplaceDialog.findInput.value         = (gFindInst.searchString
//                                            ? gFindInst.searchString
//                                            : gFindService.searchString);
//  gReplaceDialog.replaceInput.value = gFindService.replaceString;

  var theStringSource = (gFindInst.searchString ? gFindInst.searchString
                                                : gFindService.searchString);
  if (theStringSource != null && theStringSource.length === 0)
    theStringSource = null;

  gReplaceDialog.bEditorReady = false;
//  var substitutionControlObserver = new msiEditorChangeObserver(gReplaceDialog.findInput);
//  var commandBoldObserverData = new Object();
//  commandBoldObserverData.mCommand = "cmd_bold";
  //commandBoldObserverData.mObserver = substitutionControlObserver;
  //var commandSetModifiedObserverData = new Object();
  //commandSetModifiedObserverData.mCommand = "cmd_setDocumentModified";
  //commandSetModifiedObserverData.mObserver = substitutionControlObserver;
  //var editorDocLoadedObserverData = new Object();
  //editorDocLoadedObserverData.mCommand = "obs_documentCreated";
  //editorDocLoadedObserverData.mObserver = substitutionControlObserver;
//
  //gReplaceDialog.findInput.mInitialDocObserver = [commandSetModifiedObserverData, editorDocLoadedObserverData, commandBoldObserverData];
  // gReplaceDialog.findInput.mbSinglePara = true;
  // gReplaceDialog.replaceInput.mbSinglePara = true;

//  msiInitializeEditorForElement(gReplaceDialog.findInput, theStringSource, true);

  var theStringSource2 = gFindService.replaceString;
  if (theStringSource2 != null && theStringSource2.length == 0)
    theStringSource2 = null;
//  msiInitializeEditorForElement(gReplaceDialog.replaceInput, theStringSource2, true);
//RWA now replaced  gReplaceDialog.findInput.mbSetFocusOnStartup = true;
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
  gReplaceDialog.entireWord.checked      = (gFindInst.entireWord
                                            ? gFindInst.entireWord
                                            : gFindService.entireWord);

//  gReplaceDialog.multiParaCheckbox.checked = false;

  gReplaceDialog.tabOrderArray = new Array( gReplaceDialog.findInput, gReplaceDialog.replaceInput, gReplaceDialog.caseSensitive,
                                       gReplaceDialog.wrap, gReplaceDialog.entireWord, gReplaceDialog.caseSensitive, gReplaceDialog.searchBackwards,
                                       gReplaceDialog.findNext, gReplaceDialog.replace,
                                       gReplaceDialog.replaceAndFind, gReplaceDialog.replaceAll,
                                       document.documentElement.getButton("cancel") );
  // doEnabling();
}

function onLoad()
{
  // Get the xul <editor> element:
  var editorElement = window.arguments[0];

  // If we don't get the editor, then we won't allow replacing.
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
  gReplaceDialog.mTargetEditorElement = editorElement;

  // Change "OK" to "Find".
  //dialog.find.label = document.getElementById("fBLT").getAttribute("label");

  // Fill dialog.
  loadDialog();
//  toggleMultiPara();
//  if (gReplaceDialog.findInput.value)
//    gReplaceDialog.findInput.select();
//  else
//    gReplaceDialog.findInput.focus();
  msiSetInitialDialogFocus(gReplaceDialog.findInput);
  msiSetActiveEditor(gReplaceDialog.findInput, false);
  msiGetEditor(gReplaceDialog.findInput).selectAll();
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
    gFindService.searchString = gReplaceDialog.findInput.textContent;

    gFindService.matchCase     = gReplaceDialog.caseSensitive.checked;
    gFindService.wrapFind      = gReplaceDialog.wrap.checked;
    gFindService.entireWord    = gReplaceDialog.entireWord.checked;
    gFindService.findBackwards = gReplaceDialog.searchBackwards.checked;
  }
}

// function retrieveSearchExpression()
// {
//   if (gReplaceDialog.findContentFilter == null)
//     gReplaceDialog.findContentFilter = new msiDialogEditorContentFilter(gTestFindDialog.findInput);
//   return gReplaceDialog.findContentFilter.getXMLNodesAsDocFragment();
// }

function initFindData()
{
  var arr = arr=/\s*LaTeX Dialog\s*(\S.*)\s*/.exec(gReplaceDialog.findInput.contentDocument.documentElement.textContent);
  if (arr.length > 1) gFindInst.searchString = arr[1];
  else return false;
  gFindInst.matchCase     = gReplaceDialog.caseSensitive.checked;
  gFindInst.wrapFind      = gReplaceDialog.wrap.checked;
  gFindInst.entireWord    = gReplaceDialog.entireWord.checked;
  gFindInst.searchFrames  = false;
  gFindInst.findBackwards = gReplaceDialog.searchBackwards.checked;
  saveFindData();
  return true;
}

function onFindNext()
{
  var result = false;
  clearMessages();
  if (initFindData()) {
    result = gFindInst.findNext();
  }
  if (!result) showNotFound();
  return result;
}


function selectionMatchesFindString() {
  var selStr = selection.toString();
  var specStr = gFindInst.searchString;
  if (!gReplaceDialog.caseSensitive.checked)
  {
    selStr = selStr.toLowerCase();
    specStr = specStr.toLowerCase();
  }
  // Unfortunately, because of whitespace we can't just check
  // whether (selStr == specStr), but have to loop ourselves.
  // N chars of whitespace in specStr can match any M >= N in selStr.
  var specLen = specStr.length;
  var selLen = selStr.length;
  if (selLen < specLen)
    return false;
  var specArray = specStr.match(/\S+|\s+/g);
  var selArray = selStr.match(/\S+|\s+/g);
  if ( specArray.length != selArray.length)
    return false;
  for (var i=0; i<selArray.length; i++)
  {
    if (selArray[i] != specArray[i])
    {
      if ( /\S/.test(selArray[i][0]) || /\S/.test(specArray[i][0]) )
      {
        // not a space chunk -- match fails
        return false;
      }
      else if ( selArray[i].length < specArray[i].length )
      {
        // if it''s a space chunk then we only care that sel be
        // at least as long as spec
        return false;
      }
    }
  }
  return true;
}


function onReplace()
{
  var matches = true;
  clearMessages();
  // Does the current selection match the find string?
  matches = selectionMatchesFindString();
  // If the current selection doesn''t match the pattern,
  // then we want to find the next match, but not do the replace.
  // That''s what most other apps seem to do.
  // So here, just return.
  if (!matches)
    return false;

  // nsPlaintextEditor::InsertText fails if the string is empty,
  // so make that a special case:
  var ed = msiGetEditor(gReplaceDialog.replaceInput);
  var str = ed.outputToString("text/xml",2);
  var re = /.*dialogbase>(.*)<\/dialogbase.*/;
  var arr = re.exec(str);
  if (arr && arr.length > 1) replStr = arr[1];
  else return false;
  if (replStr === "")
    gEditor.deleteSelection(0);
  else
    gEditor.insertHTMLWithContext(replStr, "", "0,0", "", null, null, 0, true, true );

  return true;
}

// Handling notification messages

function clearMessages()
{
  document.getElementById("notfound").setAttribute('hidden', 'true');
  document.getElementById("replaceallResults").setAttribute('hidden','true');
}

function showNotFound()
{
  document.getElementById("notfound").removeAttribute("hidden");
}

function showReplacedCount(count)
{
  document.getElementById("replacemessage").textContent =
    gStringBundle.GetStringFromName("Replaced") + " " + 
      count + " " + gStringBundle.GetStringFromName("occurrences");
  document.getElementById("replaceallResults").removeAttribute("hidden");
}

function onReplaceAll()
{
  clearMessages();
  var occurrences = 0;
  // if (GetCurrentViewMode() == "source") {
  //   var sourceIframe = EditorUtils.getCurrentSourceEditorElement();
  //   var sourceEditor = sourceIframe.contentWindow.wrappedJSObject.gEditor;
  //   sourceEditor.setCursor(0,0);
  //   var query = gDialog.bespinFindTextbox.value;
  //   var isCaseSensitive = gDialog.bespinFindCaseSensitive.checked;

  //   sourceIframe.contentWindow.wrappedJSObject.findNeedle(true, true, query, isCaseSensitive);

  //   //BespinFind(true, false);

  //   while (sourceEditor.lastNeedle &&
  //          sourceEditor.lastNeedle.from() &&
  //          sourceEditor.lastNeedle.to()) {
  //     occurrences++;
  //     var end = sourceEditor.lastNeedle.to();
  //     sourceEditor.lastNeedle.replace(gDialog.bespinReplaceTextbox.value);
  //     sourceEditor.setCursor(end);

  //     BespinFind(true, false);
  //     var from = sourceEditor.getCursor(true);
  //     if (from.line == end.line && from.ch == end.ch)
  //       break;
  //   }
  // }
  // else 
  var found = true;
  initFindData();

  // var findInst = EditorUtils.getCurrentEditorElement().webBrowserFind;
  // findInst.searchString  = gDialog.bespinFindTextbox.value;
  // findInst.matchCase     = gDialog.bespinFindCaseSensitive.checked;
  // findInst.wrapFind      = false;
  // findInst.findBackwards = false;
  // findInst.searchFrames  = true;
  // findInst.entireWord    = false;

  // var findInFrames = findInst.QueryInterface(Components.interfaces.nsIWebBrowserFindInFrames);
  // findInFrames.rootSearchFrame = EditorUtils.getCurrentEditorElement().contentWindow;
  // findInFrames.currentSearchFrame = findInFrames.rootSearchFrame;

  gEditor.beginningOfDocument();

  var result = gFindInst.findNext();

  gEditor.beginTransaction();
  while (result) {
    occurrences++;
    onReplace();
    result = gFindInst.findNext();
  }
  gEditor.endTransaction();
  showReplacedCount(occurrences);
}

    //The data depending on "current state" of the search rather than the search string will be kept in gReplaceDialog.mSearchState.
    //Its data will consist of:
    //  mSearchState.direction = 1 for forwards, 0 for backwards?
    //  mSearchState.xPathAxis - the "axis" string to be used in an XPath search; generally "following::" or "preceding::" if
    //    we were able to get a context node just before the selection start (for forward searches) or after 
    //    the selection end (for backwards searches)
    //  mSearchState.xPathContextNode - the node to set the context for the XPath search.
    //  mSearchState.globalRange  - contains start and end of whole search unless pattern is changed. If the initial selection is
    //    collapsed, these will be the same, of course.
    //  mSearch.currentRange - 

// function msiXPathSearchInstanceState(theEditor, theFlags)
// {
//   this.mEditor = theEditor;
//   var selection = theEditor.selection;
//   var selecRange;
// //  if (selection.rangeCount > 0)
//     selecRange = selection.getRangeAt(0);
//   this.globalRange = selecRange.cloneRange();
//   this.flags = theFlags;
//   this.xPathFoundNodes = null;
//   this.xPathCurrFoundNodeIndex = -1;
//   this.bFirstPassDone = false;
//   this.bSecondPassDone = false;
//   this.nextAxisStr = null;
//   this.nextConditionStr = null;
//   this.nextContextNode = null;
//   this.bFirstPassPrepared = false;
//   this.bSecondPassPrepared = false;

//   this.backwards = function()
//   {
//     return (this.flags != null) && (this.flags.indexOf("b") >= 0);
//   };

//   this.setBackwards = function(bDoSet)
//   {
//     if (this.backwards() != bDoSet)
//     {
//       if (bDoSet)
//         this.flags += "b";
//       else
//         this.flags.replace("b", "", "gi");
//     }
//   };

//   this.wrapping = function()
//   {
//     return (this.flags != null) && (this.flags.indexOf("w") >= 0);
//   };

//   this.setWrapping = function(bDoSet)
//   {
//     if (this.wrapping() != bDoSet)
//     {
//       if (bDoSet)
//         this.flags += "w";
//       else
//         this.flags.replace("w", "", "gi");
//     }
//   };

//   this.collapsed = function()
//   {
//     return (this.globalRange == null) || (this.globalRange.collapsed);
//   };

//   this.retrieveNextXPathResultNode = function()
//   {
//     var result = null;
//     if (this.xPathFoundNodes != null)
//     {
//       if (this.backwards())
//       {
//         while ( (result == null) && ((--this.xPathCurrFoundNodeIndex) >= 0) )
//         {
//           result = this.xPathFoundNodes.snapshotItem(this.xPathCurrFoundNodeIndex);
//           //Here we need to test result and see whether it's been deleted by a previous replace operation. How to do that?
//   //        if ((result != null) && (result.
//         } 
//       }
//       else
//       {
//         while ( (result == null) && ((++this.xPathCurrFoundNodeIndex) < this.xPathFoundNodes.snapshotLength) )
//         {
//           result = this.xPathFoundNodes.snapshotItem(this.xPathCurrFoundNodeIndex);
//           //Here we need to test result and see whether it's been deleted by a previous replace operation. How to do that?
//         }
//       }
//     }
//     return result;
//   };

//   this.prepareFirstXPathIteration = function()
//   {
//     var retVal = false;
//     var startNode = null;
//     var axisStr = ".//";
//     var additionalCondition = "";
//     var parentNode = null;
//     if (this.backwards())
//     {
//       startNode = msiNavigationUtils.getNodeAfterPosition(this.globalRange.endContainer, this.globalRange.endOffset);
//       if (startNode != null)
//         axisStr = "preceding::";
//     }
//     else
//     {
//       startNode = msiNavigationUtils.getNodeBeforePosition(this.globalRange.startContainer, this.globalRange.startOffset);
//       if (startNode != null)
//         axisStr = "following::";
//     }
//     if (startNode == null)
//     {
//       startNode = msiGetRealBodyElement(this.mEditor.document);
//       axisStr = "descendant-or-self::";
//     }
//     if (!this.collapsed())
//     {
//       parentNode = this.globalRange.commonAncestorContainer;
//       additionalCondition = "[ancestor::*[local-name=\"" + msiGetBaseNodeName(parentNode) + "\" and text()=\"" + parentNode.textContent + "\"]]";
//     }
//     this.nextAxisStr = axisStr;
//     this.nextConditionStr = additionalCondition;
//     this.nextContextNode = startNode;
//     this.bFirstPassPrepared = true;
//     return true;
//   };

//   this.prepareNextXPathIteration = function()
//   {
//     var retVal = false;
//     var axisStr = "";
//     var conditionStr = "";
//     var endNode = null;
//     if (!this.bFirstPassDone)
//       retVal = this.prepareFirstXPathIteration();

//     if (!retVal && this.wrapping() && this.collapsed() && !this.bSecondPassDone)
//     {
//       if (this.backwards())
//       {
//         endNode = msiNavigationUtils.getNodeBeforePosition(this.globalRange.endContainer, this.globalRange.endOffset);
//         if (endNode != null)
//           axisStr = "following::";
//       }
//       else
//       {
//         endNode = msiNavigationUtils.getNodeAfterPosition(this.globalRange.startContainer, this.globalRange.startOffset);
//         if (endNode != null)
//           axisStr = "preceding::";
//       }
//       if (endNode == null)
//         return false;
//       this.nextAxisStr = axisStr;
//       this.nextConditionStr = conditionStr;
//       this.nextContextNode = endNode;
//       retVal = true;
//       this.bSecondPassPrepared = true;
//       dump("In msiEdReplace.js, in msiXPathSearchInstanceState.prepareNextXPathIteration, doing second pass; axisStr is {" + axisStr + "}.\n");
//     }
//     return retVal;
//   };

//   this.isReady = function()
//   {
//     if (!this.bFirstPassDone)
//       return this.bFirstPassPrepared;
//     else if (!this.bSecondPassDone)
//       return this.bSecondPassPrepared;
//     else
//       return false;
//   };
  
//   this.doEvaluateXPath = function(xPathExpression)
//   {
//     this.xPathFoundNodes = evaluateXPath(this.nextContextNode, xPathExpression);
//     if (this.backwards())
//       this.xPathCurrFoundNodeIndex = this.xPathFoundNodes.snapshotLength;
//     else
//       this.xPathCurrFoundNodeIndex = -1;
//     if (this.bFirstPassDone)
//     {
//       if (!this.wrapping())
//         dump("Problem in msiEdReplace.js, msiXPathSearchInstanceState.doEvaluateXPath; bFirstPassDone is true and wrapping is false, so why are we here?\n");
//       this.bSecondPassDone = true;
//     }
//     else
//       this.bFirstPassDone = true;
//   };

//   this.getAxisString = function()
//   {
//     return this.nextAxisStr;
//   };

//   this.getConditionString = function()
//   {
//     return this.nextConditionStr;
//   };

//   this.getContextNode = function()
//   {
//     return this.nextContextNode;
//   };
// }

// function setUpSearchState()
// {
//   var theFlags = "";
//   if (gReplaceDialog.searchBackwards.checked)
//     theFlags += "b";
//   if (gReplaceDialog.wrap.checked)
//     theFlags += "w";
//   gReplaceDialog.mSearchState = new msiXPathSearchInstanceState(gEditor, theFlags);
//   gReplaceDialog.mSearchState.prepareNextXPathIteration();
//   //Another thing to watch for - if we wrap around, the search may have to actually proceed past the original cursor position to
//   //  check for an entire match. How to set up the start and end positions for the second time?

// //        START HERE - TRY TO FIND PREVIOUS NODE TO USE following:: AXIS, OTHERWISE USE DOCUMENT NODE?
// //        Trouble is that this has to be determined before the expression can be generated properly??
// //        //Also note that the starting cursor position may be in the middle of a Text node, or in a run
// //        //  of text within an object also containing other types of nodes (like a paragraph containing
// //        //  math). If there's always a Text node to work from in this case, the algorithm of backing up
// //        //  to get a node from which we can use "following::" will probably work okay. Otherwise, we'll
// //        //  have to use the document root search ".//".
// //        //The other issue is how to prevent searching, for instance, Front Matter.??
// //      }
// //      else if ( (aNode.childNodes != null) && (aNode.childNodes.length > selecRange.startoffset) )
// //      {
// //      }
// //    }
// }

 

// function getXPathSearchExpression()
// {
//   if (gReplaceDialog.xPathExpression)
//     return gReplaceDialog.xPathExpression;
//   if (gReplaceDialog.textExpression)
//     return gReplaceDialog.textExpression;
//   return "";
// }

// function findAndCheckNext()
// {
//   var result = true;
//   var resultNode = null;
//   var newRange = null;

//   while (result)
//   {
//     if (gReplaceDialog.xPathExpression != null)
//     {
//       resultNode = getNextXPathResultNode();
//       if (resultNode != null)
//       {
//         newRange = gEditor.document.createRange();
//         newRange.selectNode(resultNode);
//       }
//       else
//         newRange = null;
//       result = (newRange != null);
//     }
//     else  //must be a text search
//     {
// //      // set up the find instance
// //      setUpFindInst();

//       // Search.
//       result = gFindInst.findNext();
//       if (result)
//         newRange = gEditor.selection.getRangeAt(0);
//       else
//         newRange = null;
//     }

//     if (newRange && gReplaceDialog.mSearchManager.verifySearch(newRange))
//     {
//       gEditor.selection.removeAllRanges();
//       gEditor.selection.addRange(newRange);
//       break;
//     }
// //    else
// //      result = getNextXPathResultNode();
//   //        if (gReplaceDialog.mSearchState.backwards())
//   //          gEditor.selection.collapseToStart();
//   //        else
//   //          gEditor.selection.collapseToEnd();
//   };
//   return result;
// }

// function onFindNext()
// {
//   // Transfer dialog contents to the find service.
//   initFindData();

//   var newRange;
//   var result = null;
//   if (gReplaceDialog.bSearchExpressionChanged)
//     ensureSearchExpressionReady();
// //    onCheckXPath();

// //  do
// //  {
// //    if (gReplaceDialog.xPathExpression != null)
// //    {
// //      result = getNextXPathResultNode();
// //      if (result != null)
// //      {
// //        newRange = gEditor.document.createRange();
// //        newRange.selectNode(result);
// //      }
// //      else
// //        newRange = null;
// //    }
// //    else  //must be a text search
// //    {
// //      // set up the find instance
// //      setUpFindInst();
// //
// //      // Search.
// //      result = gFindInst.findNext();
// //      if (result)
// //        newRange = gEditor.selection.getRangeAt(0);
// //      else
// //        newRange = null;
// //    }
// //
// //    if (newRange && gReplaceDialog.mSearchManager.verifySearch(newRange))
// //    {
// //      gEditor.selection.removeAllRanges();
// //      gEditor.selection.addRange(newRange);
// //      break;
// //    }
// //    else
// //      result = getNextXPathResultNode();
// //  //        if (gReplaceDialog.mSearchState.backwards())
// //  //          gEditor.selection.collapseToStart();
// //  //        else
// //  //          gEditor.selection.collapseToEnd();
// //  } while (result);

//   result = findAndCheckNext();

//   if (!result)
//   {
//     gReplaceDialog.mSearchState = null;
//     gReplaceDialog.mSearchManager = null;
//     var bundle = document.getElementById("findBundle");
//     AlertWithTitle(null, bundle.getString("notFoundWarning"));
// //    SetTextboxFocus(gReplaceDialog.findInput);
// //    gTestFindDialog.findInput.select();
//     gReplaceDialog.findInput.focus();
//     return false;
//   } 
//   return true;
// }


// function onReplace()
// {
//   if (!gEditor)
//     return false;

//   var selection = gEditor.selection;
//   var matches = true;

//Do we need the next stuff at all? All of the match checking should be taken care of in the onFind() function.
//  if (gReplaceDialog.xPathExpression == null)  //this is the case when we're doing a text search:
//  {
//    // Does the current selection match the find string?
//
//    var selStr = selection.toString();
//  //  var specStr = msiGetEditor(gReplaceDialog.findInput).outputToString("text/plain", 1024); // OutputLFLineBreak
//    if (gReplaceDialog.findContentFilter == null)
//      gReplaceDialog.findContentFilter = new msiDialogEditorContentFilter(gReplaceDialog.findInput);
//    if (gReplaceDialog.replaceContentFilter == null)
//      gReplaceDialog.replaceContentFilter = new msiDialogEditorContentFilter(gReplaceDialog.replaceInput);
//
//  //  var specStr = gReplaceDialog.findContentFilter.getMarkupString();  - this is what it should be
//    var specStr = gReplaceDialog.findContentFilter.getTextString();
//    if (!gReplaceDialog.caseSensitive.checked)
//    {
//      selStr = selStr.toLowerCase();
//      specStr = specStr.toLowerCase();
//    }
//    // Unfortunately, because of whitespace we can't just check
//    // whether (selStr == specStr), but have to loop ourselves.
//    // N chars of whitespace in specStr can match any M >= N in selStr.
//    var specLen = specStr.length;
//    var selLen = selStr.length;
//    if (selLen < specLen)
//      matches = false;
//    else
//    {
//      var specArray = specStr.match(/\S+|\s+/g);
//      var selArray = selStr.match(/\S+|\s+/g);
//      if ( specArray.length != selArray.length)
//        matches = false;
//      else
//      {
//        for (var i=0; i<selArray.length; i++)
//        {
//          if (selArray[i] != specArray[i])
//          {
//            if ( /\S/.test(selArray[i][0]) || /\S/.test(specArray[i][0]) )
//            {
//              // not a space chunk -- match fails
//              matches = false;
//              break;
//            }
//            else if ( selArray[i].length < specArray[i].length )
//            {
//              // if it's a space chunk then we only care that sel be
//              // at least as long as spec
//              matches = false;
//              break;
//            }
//          }
//        }
//      }
//    }
//  }
//  // If the current selection doesn't match the pattern,
//  // then we want to find the next match, but not do the replace.
//  // That's what most other apps seem to do.
//  // So here, just return.
//  if (!matches)
//    return false;

  // Transfer dialog contents to the find service.
//   initFindData();

//   // For reverse finds, need to remember the caret position
//   // before current selection
//   var newRange;
//   if (gReplaceDialog.searchBackwards.checked && selection.rangeCount > 0)
//   {
//     newRange = selection.getRangeAt(0).cloneRange();
//     newRange.collapse(true);
//   }

//   // nsPlaintextEditor::InsertText fails if the string is empty,
//   // so make that a special case:
// //  var serializer = new XMLSerializer();
// //  var replStr = serializer.serializeToString(gReplaceDialog.replaceInput.contentDocument.documentElement);
// //  var replStr = getStringFromEditor(gReplaceDialog.replaceInput);
// //  var replNodes = gReplaceDialog.replaceContentFilter.getXMLNodesAsDocFragment();
// //rwa  var replFrag = gReplaceDialog.replaceContentFilter.getContentsAsDocumentFragment();
// //  var replString = gReplaceDialog.replaceContentFilter.getDocumentFragmentString();
//   // if (gReplaceDialog.replaceContentFilter == null)
//   //   gReplaceDialog.replaceContentFilter = new msiDialogEditorContentFilter(gReplaceDialog.replaceInput);
//   // var replString = gReplaceDialog.replaceContentFilter.getMarkupString();
//   var replString = gReplaceDialog.replaceContentFilter.textContent;

// //rwa  if (replStr == "")
//     gEditor.deleteSelection(0);
//   if (replString != null && replString.length > 0)
//   {
//     dump("In msiEdReplace.onReplace(), inserting HTML [" + replString + "].\n");
//     gEditor.insertHTMLWithContext(replString, null, null, "text/html", null,null,0,true);
//   }
// //rwa    insertXMLNodesAtCursor(gEditor, replFrag.childNodes, true);
// //rwa    insertXMLNodesAtCursor(gEditor, replNodes, true);
// //rwa  else
// //rwa    insertXMLAtCursor(gEditor, replStr, bIsSinglePara, true);


// ////  else if (gEditor.editortype == "html" || gEditor.editortype == "htmlmail")
// //  else if ("insertHTML" in gEditor)
// //    gEditor.insertHTML(replStr);
// //  else
// //    gEditor.insertText(replStr);

//   // For reverse finds, need to move caret just before the replaced text
//   if (gReplaceDialog.searchBackwards.checked && newRange)
//   {
//     gEditor.selection.removeAllRanges();
//     gEditor.selection.addRange(newRange);
//   }

//   return true;
// }


// function replaceAllFindCheckNext(theFinder, findString, srchRange, selRange, endPointRange)
// {
//   var resultNode = null;
//   var result = true;
//   var theRange = null;
//   while (result)
//   {
//     if (gReplaceDialog.xPathExpression != null)
//     {
//       resultNode = getNextXPathResultNode();
//       if (resultNode != null)
//       {
//         theRange = gEditor.document.createRange();
//         theRange.selectNode(resultNode);
//       }
//       else
//         theRange = null;
//       result = (theRange != null);
//     }
//     else  //must be a text search
//     {
//       theRange = theFinder.Find(findString, srchRange, selRange, endPointRange);
//       result = theRange != null;
//     }

//     if (theRange && gReplaceDialog.mSearchManager.verifySearch(theRange))
//     {
//       var compVal = 0;
//       //check to ensure we haven't gone beyond the endPoint:
//       if (gReplaceDialog.searchBackwards.checked)
//       {
//         compVal = msiNavigationUtils.comparePositions(theRange.startContainer, theRange.startOffset, endPointRange.startContainer, endPointRange.startOffset);
//         if (compVal < 0)  //our found range starts before endPoint
//           theRange = null;
//       }
//       else
//       {
//         compVal = msiNavigationUtils.comparePositions(theRange.endContainer, theRange.endOffset, endPointRange.endContainer, endPointRange.endOffset);
//         if (compVal > 0)  //our found range ends after endPoint
//           theRange = null;
//       }
//       if (theRange)
//         break;
//     }
//     else
//       theRange = null;
//   }
//   return theRange;
// };


// function onReplaceAll()
// {
//   if (!gEditor)
//     return;

//   initFindData();

//   var findStr = gFindInst.searchString;
//   var replString = gReplaceDialog.replaceInput.textContent;
// //  var repStr = gReplaceDialog.replaceInput.value;

//   var finder = Components.classes["@mozilla.org/embedcomp/rangefind;1"].createInstance().QueryInterface(Components.interfaces.nsIFind);

//   finder.caseSensitive = gReplaceDialog.caseSensitive.checked;
//   finder.findBackwards = gReplaceDialog.searchBackwards.checked;

//   // We want the whole operation to be undoable in one swell foop,
//   // so start a transaction:
//   gEditor.beginTransaction();

//   // and to make sure we close the transaction, guard against exceptions:
//   try {
//     // Make a range containing the current selection, 
//     // so we don't go past it when we wrap.
//     var selection = gEditor.selection;
//     var selecRange;
//     if (selection.rangeCount > 0)
//       selecRange = selection.getRangeAt(0);
//     var origRange = selecRange.cloneRange();

//     // We'll need a range for the whole document:
//     var wholeDocRange = gEditor.document.createRange();
//     var rootNode = gEditor.rootElement.QueryInterface(Components.interfaces.nsIDOMNode);
//     wholeDocRange.selectNodeContents(rootNode);

//     // And start and end points:
//     var endPt = gEditor.document.createRange();

//     if (gReplaceDialog.searchBackwards.checked)
//     {
//       endPt.setStart(wholeDocRange.startContainer, wholeDocRange.startOffset);
//       endPt.setEnd(wholeDocRange.startContainer, wholeDocRange.startOffset);
//     }
//     else
//     {
//       endPt.setStart(wholeDocRange.endContainer, wholeDocRange.endOffset);
//       endPt.setEnd(wholeDocRange.endContainer, wholeDocRange.endOffset);
//     }

//     // Find and replace from here to end (start) of document:
//     var foundRange;
//     var searchRange = wholeDocRange.cloneRange();


// //    while ((foundRange = finder.Find(findStr, searchRange,
// //                                     selecRange, endPt)) != null)
//     while ( (foundRange = replaceAllFindCheckNext(finder, findStr, searchRange, selecRange, endPt)) != null)
//     {
//       gEditor.selection.removeAllRanges();
//       gEditor.selection.addRange(foundRange);

//       // The editor will leave the caret at the end of the replaced text.
//       // For reverse finds, we need it at the beginning,
//       // so save the next position now.
//       if (gReplaceDialog.searchBackwards.checked)
//       {
//         selecRange = foundRange.cloneRange();
//         selecRange.setEnd(selecRange.startContainer, selecRange.startOffset);
//       }

//       // nsPlaintextEditor::InsertText fails if the string is empty,
//       // so make that a special case:
// //rwa      if (repStr == "")
//         gEditor.deleteSelection(0);
// //rwa//      else if (gEditor.editortype == "html" || gEditor.editortype == "htmlmail")
// //rwa      else if ("insertHTML" in gEditor)
// //rwa        gEditor.insertHTML(repStr);
// //rwa      else
// //rwa        gEditor.insertText(repStr);
//       if (replString != null && replString.length > 0)
//         gEditor.insertHTMLWithContext(replString, null, null, "text/html", null,null,0,true);
// //rwa        if (repFrag != null)
// //rwa          insertXMLNodesAtCursor(gEditor, repFrag.childNodes, true);
// //rwa        insertXMLNodesAtCursor(gEditor, replNodes, true);

//       // If we're going forward, we didn't save selecRange before, so do it now:
//       if (!gReplaceDialog.searchBackwards.checked)
//       {
//         selection = gEditor.selection;
//         if (selection.rangeCount <= 0) {
//           gEditor.endTransaction();
//           return;
//         }
//         selecRange = selection.getRangeAt(0).cloneRange();
//       }
//     }

//     // If no wrapping, then we're done
//     if (!gReplaceDialog.wrap.checked) {
//       gEditor.endTransaction();
//       return;
//     }

//     // If wrapping, find from start/end of document back to start point.
//     if (gReplaceDialog.searchBackwards.checked)
//     {
//       // Collapse origRange to end
//       origRange.setStart(origRange.endContainer, origRange.endOffset);
//       // Set current position to document end
//       selecRange.setEnd(wholeDocRange.endContainer, wholeDocRange.endOffset);
//       selecRange.setStart(wholeDocRange.endContainer, wholeDocRange.endOffset);
//     }
//     else
//     {
//       // Collapse origRange to start
//       origRange.setEnd(origRange.startContainer, origRange.startOffset);
//       // Set current position to document start
//       selecRange.setStart(wholeDocRange.startContainer,
//                           wholeDocRange.startOffset);
//       selecRange.setEnd(wholeDocRange.startContainer, wholeDocRange.startOffset);
//     }

// //    while ((foundRange = finder.Find(findStr, wholeDocRange,
// //                                     selecRange, origRange)) != null)
//     while ( (foundRange = replaceAllFindCheckNext(finder, findStr, wholeDocRange, selecRange, origRange)) != null)
//     {
//       gEditor.selection.removeAllRanges();
//       gEditor.selection.addRange(foundRange);

//       // Save insert point for backward case
//       if (gReplaceDialog.searchBackwards.checked)
//       {
//         selecRange = foundRange.cloneRange();
//         selecRange.setEnd(selecRange.startContainer, selecRange.startOffset);
//       }

//       // nsPlaintextEditor::InsertText fails if the string is empty,
//       // so make that a special case:
// //rwa      if (repStr == "")
//         gEditor.deleteSelection(0);
// //rwa//      else if (gEditor.editortype == "html" || gEditor.editortype == "htmlmail")
// //rwa      else if ("insertHTML" in gEditor)
// //rwa        gEditor.insertHTML(repStr);
// //rwa      else
// //rwa        gEditor.insertText(repStr);
//         gEditor.insertHTMLWithContext(replString, null, null, "text/html", null,null,0,true);
// //      insertXMLNodesAtCursor(gEditor, replNodes, true);

//       // Get insert point for forward case
//       if (!gReplaceDialog.searchBackwards.checked)
//       {
//         selection = gEditor.selection;
//         if (selection.rangeCount <= 0) {
//           gEditor.endTransaction();
//           return;
//         }
//         selecRange = selection.getRangeAt(0);
//       }
//     }
//   } // end try
//   catch (e) { }

//   gEditor.endTransaction();
// }

// function doEnabling()
// {
//   if (gReplaceDialog.findContentFilter == null)
//     gReplaceDialog.findContentFilter = new msiDialogEditorContentFilter(gReplaceDialog.findInput);

//   dump("In msiEdReplace.doEnabling, whole content of find string window is: [\n  " + gReplaceDialog.findContentFilter.getFullContentString() + "\n].\n");
// //Following is just logging stuff - will we need it again?
// //  var findEditor = msiGetEditor(gReplaceDialog.findInput);
// //  var theSel = findEditor.selection;
// //  if (theSel.rangeCount > 0)
// //  {
// //    var theRange = theSel.getRangeAt(0);
// //    dump( "The current insert position there is at offset [" + theRange.startOffset + "] in node [" + theRange.startContainer.nodeName + "], text content [" + theRange.startContainer.textContent + "].\n" );
// //  }
//   var findStr = gReplaceDialog.findContentFilter.getDocumentFragmentString();
//   dump("In msiEdReplace.doEnabling, findStr was [" + findStr + "].\n");
// ////  var repStr = gReplaceDialog.replaceInput.value;  //Not used anyway - we'd probably want to check the serialized data more closely otherwise?

// //  if (findStr.length <= 0)
// //    findStr = null;
// //  gReplaceDialog.enabled = findStr;
// //  gReplaceDialog.findNext.disabled = !findStr;

//   var bNonEmpty = gReplaceDialog.findContentFilter.isNonEmpty();
//   gReplaceDialog.enabled = bNonEmpty;
//   gReplaceDialog.findNext.disabled = !bNonEmpty;
//   gReplaceDialog.bSearchExpressionChanged = true;  //start out with true, and reset to true on each change notification
//   gReplaceDialog.replace.disabled = !bNonEmpty;
//   gReplaceDialog.replaceAndFind.disabled = !bNonEmpty;
//   gReplaceDialog.replaceAll.disabled = !bNonEmpty;
// }


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
// function evaluateXPath(aNode, aExpr)
// {
// //  if (!gReplaceDialog.xpathEval)
// //  {
// //    gReplaceDialog.xpathEval = new XPathEvaluator();
// //    gReplaceDialog.nsResolver = gReplaceDialog.xpathEval.createNSResolver(aNode.ownerDocument == null ?
// //                                             aNode.documentElement : aNode.ownerDocument.documentElement);
// //  }
// //
// //  var result = gReplaceDialog.xpathEval.evaluate(aExpr, aNode, gReplaceDialog.nsResolver, 0, null);
// //  var found = [];
// //  var res;
// //  while (res = result.iterateNext())
// //    found.push(res);
// //  return found;


//   if (!gReplaceDialog.xPathEval)
//   {
//     gReplaceDialog.xPathEval = new XPathEvaluator();
//     gReplaceDialog.nsResolver = gReplaceDialog.xPathEval.createNSResolver(aNode.ownerDocument == null ?
//                                              aNode.documentElement : aNode.ownerDocument.documentElement);
//   }

//   var result = gReplaceDialog.xPathEval.evaluate(aExpr, aNode, gReplaceDialog.nsResolver, XPathResult.ORDERED_NODE_SNAPSHOT_TYPE, null);
//   return result;

// }

//function verifyFind()
//{
//  var theRange = gEditor.selection.getRangeAt(0);
//  if (!gReplaceDialog.mSearchManager.verifySearch(theRange))
//  {
////    theRange.collapse(false);
//    if (gReplaceDialog.mSearchState.backwards())
//      gEditor.selection.collapseToStart();
//    else
//      gEditor.selection.collapseToEnd();
//  }
//}

// function flagsHaveChanged()
// {
//   var retVal = false;
//   if (gReplaceDialog.mSearchManager != null)
//   {
//     if (gReplaceDialog.mSearchManager.isCaseInsensitive() == gReplaceDialog.caseSensitive.checked)
//     {
//       retVal = true;
//       gReplaceDialog.mSearchManager.setCaseInsensitive(!gReplaceDialog.caseSensitive.checked);
//     }
//   }
//   else
//     retVal = true;  //no mSearchManager
//   if (gReplaceDialog.mSearchState != null)
//   {
//     if (gReplaceDialog.mSearchState.backwards() != gReplaceDialog.searchBackwards.checked)
//     {
//       retVal = true;
//       gReplaceDialog.mSearchState.setBackwards(gReplaceDialog.searchBackwards.checked);
//     }
//     if (gReplaceDialog.mSearchState.wrapping() != gReplaceDialog.wrap.checked)
//     {
//       retVal = true;
//       gReplaceDialog.mSearchState.setWrapping(gReplaceDialog.wrap.checked);
//     }
//   }
//   else
//     retVal = true;
  
//   return retVal;
// }


// function ensureSearchExpressionReady()
// {
//   if (!gEditor)
//     return false;
  
//   var bNeedNewSearchString = false;
//   if (gReplaceDialog.mSearchManager == null)
//   {
//     if (!gReplaceDialog.bSearchExpressionChanged)
//       dump("In msiEdReplace.js, ensureSearchExpressionReady(), mSearchManager is uninitialized but bSearchExpressionChanged shows false!\n");
//     gReplaceDialog.bSearchExpressionChanged = true;
//   }

//   gReplaceDialog.bSearchExpressionChanged = gReplaceDialog.bSearchExpressionChanged || flagsHaveChanged();
    

//   if (gReplaceDialog.bSearchExpressionChanged)
//   {

// //    var specStr = msiGetEditor(gTestFindDialog.findInput).outputToString("text/plain", 1024); // OutputLFLineBreak
//     gReplaceDialog.findContentNodes = retrieveSearchExpression();
//     var searchFlags = "";
// //    var searchRange = null;
//     bNeedNewSearchString = true;

//     setUpSearchState();
//     //The data depending on "current state" of the search rather than the search string will be kept in gTestFindDialog.mSearchState.
//     //Its data will consist of:
//     //  mSearchState.direction = 1 for forwards, 0 for backwards?
//     //  mSearchState.xPathAxis - the "axis" string to be used in an XPath search; generally "following::" or "preceding::" if
//     //    we were able to get a context node just before the selection start (for forward searches) or after 
//     //    the selection end (for backwards searches)
//     //  mSearchState.xPathContextNode - the node to set the context for the XPath search.
//     //  mSearchState.globalRange  - contains start and end of whole search unless pattern is changed. If the initial selection is
//     //    collapsed, these will be the same, of course.
// //    var axisStr = determineXPathAxisString();

//     if (!gReplaceDialog.caseSensitive.checked)
//       searchFlags += "i";
//     gReplaceDialog.mSearchManager = new msiSearchManager(gReplaceDialog.mTargetEditorElement, gReplaceDialog.findContentNodes, searchFlags);
//   }
//   else if (gReplaceDialog.mSearchState == null)
//   {
//     dump("In msiEdReplace.js, in ensureSearchExpressionReady(); gReplaceDialog.mSearchState is null but bSearchExpressionChanged is false!");
//     setUpSearchState();
//     bNeedNewSearchString = true;
//   }
//   else if (gReplaceDialog.mSearchState.retrieveNextXPathResultNode() == null)  //the other case in which we get called
//   {
//     bNeedNewSearchString = true;
//   }

//   if (bNeedNewSearchString)
//   {
//     dump("In msiEdReplace.js, in ensureSearchExpressionReady(), calling mSearchManager.setUpSearch with axis string {" + gReplaceDialog.mSearchState.getAxisString() + "}.\n");
//     gReplaceDialog.mSearchManager.setUpSearch(gReplaceDialog.mSearchState.getAxisString(), gReplaceDialog.mSearchState.getConditionString());

//     var theString = gReplaceDialog.mSearchManager.getXPathSearchString();
//     if ( (theString == null) || (theString.length == 0) )
//     {
//       gReplaceDialog.xPathExpression = null;
//       gReplaceDialog.textExpression = theString = gReplaceDialog.mSearchManager.getTextSearchString();
//     }
//     else
//     {
//       gReplaceDialog.xPathExpression = theString;
//       gReplaceDialog.textExpression = null;
//     }
//     var serializedFrag = gReplaceDialog.findContentFilter.getDocumentFragmentString();
//     dump("In msiEdReplace.js, ensureSearchExpressionReady(); search string is:\n  " + theString + "\n\n; contents of search window were\n  " + serializedFrag + "\n\n");
//     gReplaceDialog.bSearchExpressionChanged = false;
//   }

//   setUpFindInst();
// }
