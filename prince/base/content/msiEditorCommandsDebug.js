/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
  Copyright 2006 MacKichan Software, Inc.
 * ***** END LICENSE BLOCK ***** */

/* Main Composer window debug menu functions */

// --------------------------- Output ---------------------------


function msiEditorGetText()
{
  try {
    dump("Getting text\n");
    var  outputText = msiGetCurrentEditor().outputToString("text/plain", kOutputFormatted);
    dump("<<" + outputText + ">>\n");
  } catch (e) {}
}

function msiEditorGetHTML()
{
  try {
    dump("Getting HTML\n");
    var  outputHTML = msiGetCurrentEditor().outputToString("text/html", kOutputEncodeW3CEntities);
    dump(outputHTML + "\n");
  } catch (e) {}
}


function msiDoRewrap()
{
  try
  { 
    var edElm = msiGetActiveEditorElement();
//    AlertWithTitle("Diagnostic", "msiActiveEditorElement is [" + edElm.id + "].");
    msiGetEditor(edElm).rewrap(true);
    edElm.contentWindow.focus();
  }
  catch (e) {}
}

function msiEditorDumpContent()
{
  dump("==============  Content Tree: ================\n");
  msiGetCurrentEditor().dumpContentTree();
}

function msiEditorInsertText(textToInsert)
{
  msiGetCurrentEditor().insertText(textToInsert);
}


function xmlFragToTeX(intermediateString)
{
  var xsltProcessor = setupXMLToTeXProcessor();
  return processXMLFragWithLoadedStylesheet(xsltProcessor, intermediateString);
}

function msiEditorTestSelection()
{
  dump("Testing selection\n");
  var selection = msiGetCurrentEditor().selection;
  if (!selection)
  {
    dump("No selection!\n");
    return;
  }

  dump("Selection contains:\n");
  // 3rd param = column to wrap
  dump(selection.QueryInterface(Components.interfaces.nsISelectionPrivate)
       .toStringWithFormat("text/plain",
                           kOutputFormatted | kOutputSelectionOnly,
                           0) + "\n");

  var output, i;

  dump("====== Selection as node and offsets==========\n");
  dump("rangeCount = " + selection.rangeCount + "\n");
  for (i = 0; i < selection.rangeCount; i++)
  {
    var range = selection.getRangeAt(i);
    if (range)
    {
      dump("Range "+i+": StartParent="+range.startContainer.nodeName+", offset="+range.startOffset+"\n");
      dump("Range "+i+":   EndParent="+range.endContainer.nodeName+", offset="+range.endOffset+"\n\n");
    }
  }

  var editor = msiGetCurrentEditor();

  dump("====== Selection as unformatted text ==========\n");
  output = editor.outputToString("text/plain", kOutputSelectionOnly);
  dump(output + "\n\n");

  dump("====== Selection as formatted text ============\n");
  output = editor.outputToString("text/plain", kOutputFormatted | kOutputSelectionOnly);
  dump(output + "\n\n");

  dump("====== Selection as tex ============\n");
  var intermediateText;
  intermediateText = editor.outputToString("text/xml", kOutputFormatted | kOutputSelectionOnly);
  output = xmlFragToTeX(intermediateText);
  dump(output + "\n\n");
  alert(output);

  dump("====== Selection as HTML ======================\n");
  output = editor.outputToString("text/html", kOutputSelectionOnly);
  dump(output + "\n\n");

  dump("====== Selection as XML ======================\n");
  output = editor.outputToString("text/xml", kOutputSelectionOnly);
  dump(output + "\n\n");

  dump("====== Selection as prettyprinted HTML ========\n");
  output = editor.outputToString("text/html", kOutputFormatted | kOutputSelectionOnly);
  dump(output + "\n\n");

  dump("====== Length and status =====================\n");
  output = "Document is ";
  if (editor.documentIsEmpty)
    output += "empty\n";
  else
    output += "not empty\n";
  output += "Text length is " + editor.textLength + " characters";
  dump(output + "\n\n");
}

function msiEditorTestTableLayout()
{
  dump("\n\n\n************ Dump Selection Ranges ************\n");
  var selection = msiGetCurrentEditor().selection;
  var i;
  for (i = 0; i < selection.rangeCount; i++)
  {
    var range = selection.getRangeAt(i);
    if (range)
    {
      dump("Range "+i+": StartParent="+range.startParent+", offset="+range.startOffset+"\n");
    }
  }
  dump("\n\n");

  var editor = msiGetCurrentEditor();
  var table = editor.getElementOrParentByTagName("table", null);
  if (!table) {
    dump("Enclosing Table not found: Place caret in a table cell to do this test\n\n");
    return;
  }

  var cell;
  var startRowIndexObj = { value: null };
  var startColIndexObj = { value: null };
  var rowSpanObj = { value: null };
  var colSpanObj = { value: null };
  var actualRowSpanObj = { value: null };
  var actualColSpanObj = { value: null };
  var isSelectedObj = { value: false };
  var startRowIndex = 0;
  var startColIndex = 0;
  var rowSpan;
  var colSpan;
  var actualRowSpan;
  var actualColSpan;
  var isSelected;
  var col = 0;
  var row = 0;
  var rowCount = 0;
  var maxColCount = 0;
  var doneWithRow = false;
  var doneWithCol = false;

  dump("\n\n\n************ Starting Table Layout test ************\n");

  // Note: We could also get the number of rows, cols and use for loops,
  //   but this tests using out-of-bounds offsets to detect end of row or column

  while (!doneWithRow)  // Iterate through rows
  {
    dump("* Data for ROW="+row+":\n");
    while(!doneWithCol)  // Iterate through cells in the row
    {
      try {
        cell = editor.getCellDataAt(table, row, col,
                                    startRowIndexObj, startColIndexObj,
                                    rowSpanObj, colSpanObj,
                                    actualRowSpanObj, actualColSpanObj,
                                    isSelectedObj);

        if (cell)
        {
          rowSpan = rowSpanObj.value;
          colSpan = colSpanObj.value;
          actualRowSpan = actualRowSpanObj.value;
          actualColSpan = actualColSpanObj.value;
          isSelected = isSelectedObj.value;

          dump(" Row="+row+", Col="+col+"  StartRow="+startRowIndexObj.value+", StartCol="+startColIndexObj.value+"\n");
          dump("  RowSpan="+rowSpan+", ColSpan="+colSpan+"  ActualRowSpan="+actualRowSpan+", ActualColSpan="+actualColSpan);
          if (isSelected)
            dump("  Cell is selected\n");
          else
            dump("  Cell is NOT selected\n");

          // Save the indexes of a cell that will span across the cellmap grid
          if (rowSpan > 1)
            startRowIndex = startRowIndexObj.value;
          if (colSpan > 1)
            startColIndex = startColIndexObj.value;

          // Initialize these for efficient spanned-cell search
          startRowIndexObj.value = startRowIndex;
          startColIndexObj.value = startColIndex;

          col++;
        } else {
          doneWithCol = true;
          // Get maximum number of cells in any row
          if (col > maxColCount)
            maxColCount = col;
          dump("  End of row found\n\n");
        }
      }
      catch (e) {
        dump("  *** GetCellDataAt failed at Row="+row+", Col="+col+" ***\n\n");
        return;
      }
    }
    if (col == 0) {
      // Didn't find a cell in the first col of a row,
      // thus no more rows in table
      doneWithRow = true;
      rowCount = row;
      dump("No more rows in table\n\n");
    } else {
      // Setup for next row
      col = 0;
      row++;
      doneWithCol = false;
    }
  }
  dump("Counted during scan: Number of rows="+rowCount+" Number of Columns="+maxColCount+"\n");
  rowCount = editor.getTableRowCount(table);
  maxColCount = editor.getTableColumnCount(table);
  dump("From nsITableLayout: Number of rows="+rowCount+" Number of Columns="+maxColCount+"\n****** End of Table Layout Test *****\n\n");
}

function msiEditorShowEmbeddedObjects()
{
  dump("\nEmbedded Objects:\n");
  try {
    var objectArray = msiGetCurrentEditor().getEmbeddedObjects();
    dump(objectArray.Count() + " embedded objects\n");
    for (var i=0; i < objectArray.Count(); ++i)
      dump(objectArray.GetElementAt(i) + "\n");
  } catch(e) {}
}

function msiEditorUnitTests()
{
  dump("Running Unit Tests\n");
  var numTests       = { value:0 };
  var numTestsFailed = { value:0 };
  msiGetCurrentEditor().debugUnitTests(numTests, numTestsFailed);
}

function msiEditorTestDocument()
{
  dump("Getting document\n");
  var theDoc = msiGetCurrentEditor().document;
  if (theDoc)
  {
    dump("Got the doc\n");
    dump("Document name:" + theDoc.nodeName + "\n");
    dump("Document type:" + theDoc.doctype + "\n");
  }
  else
  {
    dump("Failed to get the doc\n");
  }
}

// --------------------------- Logging stuff ---------------------------

function EditorExecuteScript(theFile)
{
  var inputStream = Components.classes["@mozilla.org/network/file-input-stream;1"].createInstance();
  inputStream = inputStream.QueryInterface(Components.interfaces.nsIFileInputStream);

  inputStream.init(theFile, 1, 0, false);    // open read only

  var scriptableInputStream = Components.classes["@mozilla.org/scriptableinputstream;1"].createInstance();
  scriptableInputStream = scriptableInputStream.QueryInterface(Components.interfaces.nsIScriptableInputStream);

  scriptableInputStream.init(inputStream);    // open read only

  var buf         = { value:null };
  var tmpBuf      = { value:null };
  var didTruncate = { value:false };
  var lineNum     = 0;
  var ex;

/*
  // Log files can be quite huge, so read in a line
  // at a time and execute it:

  while (!inputStream.eof())
  {
    buf.value         = "";
    didTruncate.value = true;

    // Keep looping until we get a complete line of
    // text, or we hit the end of file:

    while (didTruncate.value && !inputStream.eof())
    {
      didTruncate.value = false;
      fileSpec.readLine(tmpBuf, 1024, didTruncate);
      buf.value += tmpBuf.value;

      // XXX Need to null out tmpBuf.value to avoid crashing
      // XXX in some JavaScript string allocation method.
      // XXX This is probably leaking the buffer allocated
      // XXX by the readLine() implementation.

      tmpBuf.value = null;
    }

    ++lineNum;
*/
  {
    // suck in the entire file
    var fileSize = scriptableInputStream.available();
    var fileContents = scriptableInputStream.read(fileSize);

    dump(fileContents);

    try       { eval(fileContents); }
    catch(ex) { dump("Playback ERROR: Line " + lineNum + "  " + ex + "\n"); return; }
  }

  buf.value = null;
}

function EditorGetScriptFileSpec()
{
  var dirServ = Components.classes['@mozilla.org/file/directory_service;1'].createInstance();
  dirServ = dirServ.QueryInterface(Components.interfaces.nsIProperties);
  var processDir = dirServ.get("ProfD", Components.interfaces.nsIFile);
  processDir.append("journal.js");
  return processDir;
}

function msiEditorStartLog()
{
  try {
    var editorElement = msiGetActiveEditorElement();
    var edlog = msiGetEditor(editorElement).QueryInterface(Components.interfaces.nsIEditorLogging);
    var fs = EditorGetScriptFileSpec();
    edlog.startLogging(fs);
    editorElement.contentWindow.focus();

    fs = null;
  }
  catch(ex) { dump("Can't start logging!:\n" + ex + "\n"); }
}

function msiEditorStopLog()
{
  try {
    var editorElement = msiGetActiveEditorElement();
    var edlog = msiGetEditor(editorElement).QueryInterface(Components.interfaces.nsIEditorLogging);
    edlog.stopLogging();
    editorElement.contentWindow.focus();
  }
  catch(ex) { dump("Can't stop logging!:\n" + ex + "\n"); }
}

function msiEditorRunLog()
{
  var fs;
  var editorElement = msiGetActiveEditorElement();
  fs = EditorGetScriptFileSpec();
  EditorExecuteScript(fs);
  try
  {
    editorElement.contentWindow.focus();
  }
  catch(exc) {AlertWithTitle("Error in msiEditorCommandsDebug.js", "Error in msiEditorRunLog(); exception: " + exc);}
}

// --------------------------- TransactionManager ---------------------------


function msiDumpUndoStack()
{
  try {
    var txmgr = msiGetCurrentEditor().transactionManager;

    if (!txmgr)
    {
      msidump("**** Editor has no TransactionManager!\n");
      return;
    }

    msidump("---------------------- BEGIN UNDO STACK DUMP\n");
    msidump("<!-- Bottom of Stack -->\n");
    PrintTxnList(txmgr.getUndoList(), "");
    msidump("<!--  Top of Stack  --->\n");
    msidump("Num Undo Items: " + txmgr.numberOfUndoItems + "\n");
    msidump("---------------------- END  UNDO  STACK DUMP\n");
  } catch (e) {
    msidump("ERROR: DumpUndoStack() failed: " + e);
  }
}

function msiDumpRedoStack()
{
  try {
    var txmgr = msiGetCurrentEditor().transactionManager;

    if (!txmgr)
    {
      dump("**** Editor has no TransactionManager!\n");
      return;
    }

    dump("---------------------- BEGIN REDO STACK DUMP\n");
    dump("<!-- Bottom of Stack -->\n");
    PrintTxnList(txmgr.getRedoList(), "");
    dump("<!--  Top of Stack  --->\n");
    dump("Num Redo Items: " + txmgr.numberOfRedoItems + "\n");
    dump("---------------------- END  REDO  STACK DUMP\n");
  } catch (e) {
    dump("ERROR: DumpUndoStack() failed: " + e);
  }
}

function PrintTxnList(txnList, prefixStr)
{
  var i;

  for (i=0 ; i < txnList.numItems; i++)
  {
    var txn = txnList.getItem(i);
    var desc = "TXMgr Batch";

    if (txn)
    {
      try {
        txn = txn.QueryInterface(Components.interfaces.nsPIEditorTransaction);
        desc = txn.txnDescription;
      } catch(e) {
        desc = "UnknownTxnType";
      }
    }
    msidump(prefixStr + "+ " + desc + "\n");
    PrintTxnList(txnList.getChildListForItem(i), prefixStr + "|    ");
  }
}

// ------------------------ 3rd Party Transaction Test ------------------------


//function sampleJSTransaction()
//{
//  this.wrappedJSObject = this;
//}
//
//sampleJSTransaction.prototype = {
//
//  isTransient: false,
//  mStrData:    "[Sample-JS-Transaction-Content]",
//  mObject:     null,
//  mContainer:  null,
//  mOffset:     null,
//
//  doTransaction: function()
//  {
//    if (this.mContainer.nodeType != Node.TEXT_NODE)
//    {
//      // We're not in a text node, so create one and
//      // we'll just insert it at (mContainer, mOffset).
//
//      this.mObject = this.mContainer.ownerDocument.createTextNode(this.mStrData);
//    }
//
//    this.redoTransaction();
//  },
//
//  undoTransaction: function()
//  {
//    if (!this.mObject)
//      this.mContainer.deleteData(this.mOffset, this.mStrData.length);
//    else
//      this.mContainer.removeChild(this.mObject);
//  },
//
//  redoTransaction: function()
//  {
//    if (!this.mObject)
//      this.mContainer.insertData(this.mOffset, this.mStrData);
//    else
//      this.insert_node_at_point(this.mObject, this.mContainer, this.mOffset);
//  },
//
//  merge: function(aTxn)
//  {
//    // We don't do any merging!
//
//    return false;
//  },
//
//  QueryInterface: function(aIID, theResult)
//  {
//    if (aIID.equals(Components.interfaces.nsITransaction) ||
//        aIID.equals(Components.interfaces.nsISupports))
//      return this;
//
//    Components.returnCode = Components.results.NS_ERROR_NO_INTERFACE;
//    return null;
//  },
//
//  insert_node_at_point: function(node, container, offset)
//  {
//    var childList = container.childNodes;
//
//    if (childList.length == 0 || offset >= childList.length)
//      container.appendChild(node);
//    else
//      container.insertBefore(node, childList.item(offset));
//  }
//}

function msiExecuteJSTransactionViaTxmgr()
{
  try {
    var editor = msiGetCurrentEditor();
    var txmgr = editor.transactionManager;
    txmgr = txmgr.QueryInterface(Components.interfaces.nsITransactionManager);

    var selection = editor.selection;
    var range =  selection.getRangeAt(0);

    var txn = new sampleJSTransaction();

    txn.mContainer = range.startContainer;
    txn.mOffset = range.startOffset;

    txmgr.doTransaction(txn);
  } catch (e) {
    dump("ExecuteJSTransactionViaTxmgr() failed!");
  }
}

function msiExecuteJSTransactionViaEditor()
{
  try {
    var editor = msiGetCurrentEditor();

    var selection = editor.selection;
    var range =  selection.getRangeAt(0);

    var txn = new sampleJSTransaction();

    txn.mContainer = range.startContainer;
    txn.mOffset = range.startOffset;

    editor.doTransaction(txn);
  } catch (e) {
    dump("ExecuteJSTransactionViaEditor() failed!");
  }
}


//--------------------------------------------------
function msiTestGraphScript() {
  var editorElement = msiGetActiveEditorElement();
  var theDoc = msiGetEditor(editorElement).document;
  var graphs = theDoc.getElementsByTagName("graph");
  dump("SMR testGraphScript, length is " + graphs.length +  "\n");
  for (var i=0; i<graphs.length; i++) {
    recreateGraph (graphs[i], editorElement);
  }
}  

function msiTestPreparePlotAll() {
  var theDoc = msiGetCurrentEditor().document;
  var mathelems = theDoc.getElementsByTagName("graph");
  dump("SMR TestPreparePlotAll with " + mathelems.length + " items\n");
  for (var i=0; i<mathelems.length; i++) {
    testQuery (mathelems[i]);
  }
}

function msiTestPreparePlotGraph() {
  var editorElement = msiGetActiveEditorElement();
  var theDoc = msiGetEditor(editorElement).document;
  var mathelems = theDoc.getElementsByTagName("graph");
  dump("SMR TestPreparePlotAll with " + mathelems.length + " items\n");
  for (var i=0; i<mathelems.length; i++) {
    testQueryGraph (mathelems[i], editorElement);
  }
}

function msiDumpClipboard() {
  var textmimetypes = new Object();
  textmimetypes.kUnicodeMime                 = "text/unicode";
  textmimetypes.kHTMLMime                    = "text/html";
  textmimetypes.kHTMLContext                 = "text/_moz_htmlcontext";
  textmimetypes.kHTMLInfo                    = "text/_moz_htmlinfo";
  textmimetypes.kNativeHTMLMime              = "application/x-moz-nativehtml";

  var pastetext;
  var clip = Components.classes["@mozilla.org/widget/clipboard;1"].
    getService(Components.interfaces.nsIClipboard); 
  if (!clip) return false; 
  var trans = Components.classes["@mozilla.org/widget/transferable;1"].
    createInstance(Components.interfaces.nsITransferable); 
  if (!trans) return false; 
  var dumpStr;
  dumpStr = "\nClipboard contents: \n\n"; 
  for (var i in textmimetypes)
  {
    trans.addDataFlavor(textmimetypes[i]);
    clip.getData(trans,clip.kGlobalClipboard); 
    var str = new Object(); 
    var strLength = new Object();
    try
    {
      trans.getTransferData(textmimetypes[i],str,strLength);
      if (str) str = str.value.QueryInterface(Components.interfaces.nsISupportsString); 
      if (str) pastetext = str.data.substring(0,strLength.value / 2);
      dumpStr += ("  "+textmimetypes[i]+": \"" + pastetext+"\"\n\n");
    }
    catch (e)
    {
      dump("  "+textmimetypes[i]+" not supported\n\n");
    }
    trans.removeDataFlavor(textmimetypes[i]);
  }
  alert(dumpStr);
  return false;
}

function msiTestSearch()
{
  var editorElement = msiGetActiveEditorElement();
  var theDialog = msiOpenModelessDialog("chrome://prince/content/msiDebugFind.xul", "_blank", 
      "chrome,close,titlebar,resizable,dependent",
      editorElement, "cmd_msiTestSearch", null, editorElement);

}

  
function extendSelection()
{
  var selection = msiGetCurrentEditor().selection;
  var rangeCount = selection.rangeCount;
  var range;
  var i;
  for (i=0; i<rangeCount; i++)
  {
    dump("extending range #"+(i+1)+"\n");
    range = selection.getRangeAt(i);
    selection.removeRange(range);
    adjustRange(range,selection);
    selection.addRange(range);
  }
}  

function adjustRange(range, selection)
{
  var commonAncestor = range.commonAncestorContainer;
  var node = range.startContainer;
  var newContainer = null;
  var newEndOffset;
  var tagname;
  while ( node && node!=commonAncestor )
  {
    tagname = node.tagName;
    if (tagname == 'mfrac' || tagname == "mroot"|| tagname == "mtable" || tagname == "mtd" | tagname == "mtd"
      || tagname == "msub" || tagname == "msup" || tagname == "msubsup" )
    {
      newContainer = node;
    }
    node = node.parentNode;
  }
  if (newContainer)
    range.setStart(newContainer,0);
  node = range.endContainer;
  newContainer = null;
  while ( node && node!=commonAncestor )
  {
    tagname = node.tagName;
    if (tagname == 'mfrac' || tagname == "mroot"|| tagname == "mtable" || tagname == "mtd" | tagname == "mtd"
      || tagname == "msub" || tagname == "msup" || tagname == "msubsup" )
    {
      newContainer = node;
      newEndOffset = node.childElementCount + 1;
    }
    node = node.parentNode;
  }
}     