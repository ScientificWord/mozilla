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
        doEnabling();
      }
      break;

      case "obs_documentCreated":
      {
        var bIsRealDocument = false;
        var currentURL = msiGetEditorURL(this.mEditorElement);
        msiDumpWithID("In msiEdReplace documentCreated observer for editor element [@], currentURL is " + currentURL + "].\n", this.mEditorElement);
        if (currentURL != null)
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
  if (theStringSource != null && theStringSource.length == 0)
    theStringSource = null;

  gReplaceDialog.bEditorReady = false;
  var substitutionControlObserver = new msiEditorChangeObserver(gReplaceDialog.findInput);
  var commandBoldObserverData = new Object();
  commandBoldObserverData.mCommand = "cmd_bold";
  commandBoldObserverData.mObserver = substitutionControlObserver;
  var commandSetModifiedObserverData = new Object();
  commandSetModifiedObserverData.mCommand = "cmd_setDocumentModified";
  commandSetModifiedObserverData.mObserver = substitutionControlObserver;
  var editorDocLoadedObserverData = new Object();
  editorDocLoadedObserverData.mCommand = "obs_documentCreated";
  editorDocLoadedObserverData.mObserver = substitutionControlObserver;

  gReplaceDialog.findInput.mInitialDocObserver = [commandSetModifiedObserverData, editorDocLoadedObserverData, commandBoldObserverData];

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
//  var replNodes = gReplaceDialog.replaceContentFilter.getXMLNodesAsDocFragment();
//rwa  var replFrag = gReplaceDialog.replaceContentFilter.getContentsAsDocumentFragment();
//  var replString = gReplaceDialog.replaceContentFilter.getDocumentFragmentString();
  var replString = gReplaceDialog.replaceContentFilter.getMarkupString();

//rwa  if (replStr == "")
    gEditor.deleteSelection(0);
  if (replString != null && replString.length > 0)
  {
    dump("In msiEdReplace.onReplace(), inserting HTML [" + replString + "].\n");
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
//  var repNodes = gReplaceDialog.replaceContentFilter.getXMLNodesAsDocFragment();
//rwa  var repFrag = gReplaceDialog.replaceContentFilter.getContentsAsDocumentFragment();
//  var repString = gReplaceDialog.replaceContentFilter.getDocumentFragmentString();
  var replString = gReplaceDialog.replaceContentFilter.getMarkupString();
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
      if (replString != null && replString.length > 0)
        gEditor.insertHTMLWithContext(replString, null, null, "text/html", null,null,0,true);
//rwa        if (repFrag != null)
//rwa          insertXMLNodesAtCursor(gEditor, repFrag.childNodes, true);
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
  dump("In msiEdReplace.doEnabling, findStr was [" + findStr + "].\n");
  if (findStr.length <= 0)
    findStr = null;
//  var repStr = gReplaceDialog.replaceInput.value;  //Not used anyway - we'd probably want to check the serialized data more closely otherwise?
  gReplaceDialog.enabled = findStr;
  gReplaceDialog.findNext.disabled = !findStr;
  gReplaceDialog.replace.disabled = !findStr;
  gReplaceDialog.replaceAndFind.disabled = !findStr;
  gReplaceDialog.replaceAll.disabled = !findStr;
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
function evaluateXPath(aNode, aExpr)
{
  if (!gReplaceDialog.xpathEval)
  {
    gReplaceDialog.xpathEval = new XPathEvaluator();
    gReplaceDialog.nsResolver = gReplaceDialog.xpathEval.createNSResolver(aNode.ownerDocument == null ?
                                             aNode.documentElement : aNode.ownerDocument.documentElement);
  }

  var result = gReplaceDialog.xpathEval.evaluate(aExpr, aNode, gReplaceDialog.nsResolver, 0, null);
  var found = [];
  var res;
  while (res = result.iterateNext())
    found.push(res);
  return found;
}

function prepareXPathExpression(aNodeSet)
{

}