// Copyright (c) 2004 MacKichan Software, Inc.  All Rights Reserved.

var data;

//// implements nsIObserver
//function msiFillMatrixEditorDocumentObserver(editorElement)
var msiFillMatrixEditorDocumentObserver =
{ 
//  this.msiEditorElement = editorElement;
//  this.observe = function(aSubject, aTopic, aData)
  observe: function(aSubject, aTopic, aData)
  {
    dump("ComputeFillMatrix observer hit!\n");
//    var editor = msiGetEditor(editorElement);
    switch(aTopic)
    {
      case "obs_documentCreated":
//        try
//        {
//          insertmath(this.msiEditorElement);
//        } catch(exc) {dump("Exception inserting math in FillMatrix edit window: [" + exc + "].\n");}
        // type defaults to first choice according to XUL
        Switch("zero");
      break;
    }
  }
}


function Startup(){
  data = window.arguments[0];

  var editorElement = msiGetParentEditorElementForDialog(window);
  var editor = msiGetEditor(editorElement);
  if (!editor) {
    AlertWithTitle("Error", "No editor found in ComputeFillMatrix Startup! Closing dialog window...");
    window.close();
    return;
  }

  var rows = document.getElementById("rows");
  if (data && ("rows" in data))
    rows.value = data.rows.toString();

  var cols = document.getElementById("cols");
  if (data && ("cols" in data))
    cols.value = data.cols.toString();

//SLS the following copied from editor.js
//  gSourceContentWindow = document.getElementById("content-frame");
//  gSourceContentWindow.makeEditable("html", false);
//
//  EditorStartup();

  // Initialize our source text <editor>
  var theStringSource = GetComputeString("Math.emptyForInput");
  var editorControl = document.getElementById("fillmat-content-frame");
  var docObserver = new Object();
  docObserver.mObserver = msiFillMatrixEditorDocumentObserver;
  docObserver.mCommand = "obs_documentCreated";
  editorControl.mInitialDocObserver = new Array(docObserver);
  msiInitializeEditorForElement(editorControl, theStringSource, true);

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
//  var commandManager = msiGetCommandManager(editorControl);
//  commandManager.addCommandObserver(msiFillMatrixEditorDocumentObserver, "obs_documentCreated");
  
//  editorControl.makeEditable("html", false);
}

function OK(){
  data.Cancel = false;

  var rows = document.getElementById("rows");
  data.rows  = parseInt(rows.value,10);
  // rows.value is not the same as the value attribute, so we make it so in order to get persistence to work.
  rows.setAttribute("value", rows.value);

  var cols = document.getElementById("cols");
  data.cols = parseInt(cols.value,10);
  cols.setAttribute("value", cols.value);

  var type = document.getElementById("type");
  data.type = type.selectedIndex + 1;

  if (data.type >= 4) {
    var doc = document.getElementById("fillmat-content-frame").contentDocument;
    var mathnodes = doc.getElementsByTagName("math");
    if (mathnodes.length === 0) {
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
  default: return;
  }
  var theEditor = document.getElementById("fillmat-content-frame");
  msiEnableEditorControl(theEditor, !val);

//  var elementStyle = theEditor.style;
//  var internalEditor = theEditor.getEditor(theEditor.contentWindow);
//  if (val)
//  {
//    theEditor.setAttribute("disabled", "true");
//    theEditor.allowevents = false;
//    if (internalEditor != null)
//    {
//      internalEditor.flags |= (Components.interfaces.nsIPlaintextEditor.eEditorReadonlyMask | Components.interfaces.nsIPlaintextEditor.eEditorDisabledMask);
//    }
//    if (elementStyle != null)
//    {
//      dump("Original value of moz-user-focus on editor is " + elementStyle.getPropertyValue("-moz-user-focus") + ".\n");
//      elementStyle.setProperty("-moz-user-focus", "ignore", "");
//    }
//  }
////  else if (theEditor.hasAttribute("disabled"))
//  else
//  {
//    theEditor.removeAttribute("disabled");
//    if (internalEditor != null)
//    {
//      internalEditor.flags &= ~(Components.interfaces.nsIPlaintextEditor.eEditorReadonlyMask | Components.interfaces.nsIPlaintextEditor.eEditorDisabledMask);
//    }
//    if (elementStyle != null)
//    {
//      dump("Original value of moz-user-focus on editor is " + elementStyle.getPropertyValue("-moz-user-focus") + ".\n");
//      elementStyle.setProperty("-moz-user-focus", "normal", "");
//    }
////    theEditor.setAttribute("-moz-user-focus", "normal");
//    theEditor.allowevents = true;
//  }
}

function Cancel(){
  data.Cancel = true;
}



