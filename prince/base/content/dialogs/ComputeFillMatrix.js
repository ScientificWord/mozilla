// Copyright (c) 2004 MacKichan Software, Inc.  All Rights Reserved.

var data;

//// implements nsIObserver
function msiFillMatrixEditorDocumentObserver(editorElement)
{ 
  this.msiEditorElement = editorElement;
  this.observe = function(aSubject, aTopic, aData)
  {
    dump("ComputeFillMatrix observer hit!\n");
//    var editor = msiGetEditor(editorElement);
    switch(aTopic)
    {
      case "obs_documentCreated":
        try
        {
          insertmath(this.msiEditorElement);
        } catch(exc) {AlertWithTitle("Error in ComputeFillMatrix.js", "Exception in inserting math: [" + exc + "]");}
        // type defaults to first choice according to XUL
        Switch("zero");
    }
  };
}


function Startup(){
  data = window.arguments[0];

  var rows = document.getElementById("rows");
  rows.value = data.rows.toString();

  var cols = document.getElementById("cols");
  cols.value = data.cols.toString();

  var editorElement = msiGetParentEditorElementForDialog(window);
  var editor = msiGetEditor(editorElement);
  if (!editor) {
    AlertWithTitle("Error", "No editor found in ComputeFillMatrix Startup! Closing dialog window...");
    window.close();
    return;
  }

//SLS the following copied from editor.js
//  gSourceContentWindow = document.getElementById("content-frame");
//  gSourceContentWindow.makeEditable("html", false);
//
//  EditorStartup();

  // Initialize our source text <editor>
  var theStringSource = "";
  var editorControl = document.getElementById("content-frame");
  msiInitializeEditorForElement(editorControl, theStringSource);
//  editorControl.makeEditable("html", false);

//  try {
//    gSourceTextEditor = gSourceContentWindow.getEditor(gSourceContentWindow.contentWindow);
////SLS don't know why this doesn't work here...doesn't really matter since we have no source view.
////    gSourceTextEditor.QueryInterface(Components.interfaces.nsIPlaintextEditor);
////    gSourceTextEditor.enableUndo(false);
////    gSourceTextEditor.rootElement.style.fontFamily = "-moz-fixed";
////    gSourceTextEditor.rootElement.style.whiteSpace = "pre";
////    gSourceTextEditor.rootElement.style.margin = 0;
//    var controller = Components.classes["@mozilla.org/embedcomp/base-command-controller;1"]
//                               .createInstance(Components.interfaces.nsIControllerContext);
//    controller.init(null);
//    controller.setCommandContext(gSourceContentWindow);
//    gSourceContentWindow.contentWindow.controllers.insertControllerAt(0, controller);
//    var commandTable = controller.QueryInterface(Components.interfaces.nsIInterfaceRequestor)
//                                 .getInterface(Components.interfaces.nsIControllerCommandTable);
//    commandTable.registerCommand("cmd_find",        nsFindCommand);
//    commandTable.registerCommand("cmd_findNext",    nsFindAgainCommand);
//    commandTable.registerCommand("cmd_findPrev",    nsFindAgainCommand);
//    
//    SetupMSIMathMenuCommands();
//
//  } catch (e) { dump("makeEditable failed in Startup(): "+e+"\n"); }
  
  // see EditorSharedStartup() in editor.js
  var commandManager = msiGetCommandManager(editorControl);
//  var commandManager = GetCurrentCommandManager();
  var docObserver = new msiFillMatrixEditorDocumentObserver(editorControl);
  commandManager.addCommandObserver(docObserver, "obs_documentCreated");
  editorControl.makeEditable("html", false);
}

function OK(){
  data.Cancel = false;

  var rows = document.getElementById("rows");
  data.rows  = parseInt(rows.value,10);

  var cols = document.getElementById("cols");
  data.cols = parseInt(cols.value,10);

  var type = document.getElementById("type");
  data.type = type.selectedIndex + 1;

  if (data.type >= 4) {
    var doc = document.getElementById("content-frame").contentDocument;
    var mathnodes = doc.getElementsByTagName("math");
    if (mathnodes.length == 0) {
      dump("No math in center field!\n");
      return false;  // should leave dialog up but doesn't seem to work
    }
    data.expr = GetMathAsString(mathnodes[0]);
  } else {
    data.expr = "";
  }

  var editorElement = msiGetParentEditorElementForDialog(window);
//  var theWindow = msiGetWindowContainingEditor(editorElement);
  ComputeCursor(editorElement);
  var mRows = GetNumAsMathML(data.rows);
  var mCols = GetNumAsMathML(data.cols);
  var mExpr  = data.expr;
  msiComputeLogger.Sent4("Fill matrix",mRows+" by "+mCols, mExpr,data.type);
  try {
    var out = GetCurrentEngine().matrixFill(data.type,mRows,mCols,mExpr);
    msiComputeLogger.Received(out);
    addResult(out, editorElement);
  } catch (e) {
    msiComputeLogger.Exception(e);
  }
  RestoreCursor(editorElement);

  return true;
}

function Switch(id){
  var val;
  switch (id) {
  case "zero":
  case "Identity":
  case "random":
    val = true;
    break;
  case "Jordan":
  case "function":
  case "band":
    val = false;
    break;
  }
  var theEditor = document.getElementById("content-frame");
  if (val)
    theEditor.disabled = true;
  else if (theEditor.hasAttribute("disabled"))
    theEditor.removeAttribute("disabled");
}

function Cancel(){
  data.Cancel = true;
}



