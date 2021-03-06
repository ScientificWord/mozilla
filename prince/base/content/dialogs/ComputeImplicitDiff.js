// Copyright (c) 2004 MacKichan Software, Inc.  All Rights Reserved.

Components.utils.import("resource://app/modules/msiEditorDefinitions.jsm");

var data;

function Startup(){
  data = window.arguments[0];

  if (data.title && data.title.length > 0) {
    //hack.  should use window.title in Moz 1.8
    document.documentElement.setAttribute("title", data.title);
  }
  if (data.remark && data.remark.length > 0) {
    var remark = document.getElementById("instruction");
    remark.firstChild.nodeValue = data.remark;
  }
  if (data.label && data.label.length > 0) {
    var center = document.getElementById("center");
    center.setAttribute("label", data.label);
  }

  var theStringSource = GetComputeString("Math.emptyForInput");
  var indEditor = document.getElementById("indVarFrame");
  var depEditor = document.getElementById("depVarFrame");
  indEditor.mbSinglePara = true;
  depEditor.mbSinglePara = true;

  var editorInitializer = new msiEditorArrayInitializer();
  editorInitializer.addEditorInfo(indEditor, theStringSource, true, true);
  editorInitializer.addEditorInfo(depEditor, theStringSource, true, true);
  editorInitializer.doInitialize();
  dump("After setting up, depEditor has mbSinglePara [");
  if ("mbSinglePara" in depEditor)
    dump( depEditor.mbSinglePara ? "true]\n" : "false]\n" );
  else
    dump( "undefined]\n" );

////SLS the following copied from editor.js
//  gSourceContentWindow = document.getElementById("content-frame");
//  gSourceContentWindow.makeEditable("html", false);
//
//  EditorStartup();

//  // Initialize our source text <editor>
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
}

function OK(){
  data.Cancel = false;

  //validate?
  var doc = document.getElementById("indVarFrame").contentDocument;
  var mathnodes = doc.getElementsByTagName("math");
  if (mathnodes.length == 0) {
    dump("No math in independent variables control!\n");
    return false;  // should leave dialog up but doesn't seem to work
  }
  data.thevar = GetMathAsString(mathnodes[0]);

  doc = document.getElementById("depVarFrame").contentDocument;
  mathnodes = doc.getElementsByTagName("math");
  if (mathnodes.length > 0) {
    data.about = GetMathAsString(mathnodes[0]);
  } else {
    dump("No math in dependent variables control, returning 0 for it.\n");
    data.about = "<math><mn>0</mn></math>";
  }

  var editorElement = msiGetParentEditorElementForDialog(window);
  var editor = msiGetEditor(editorElement);
  var theWindow = window.opener;
  if (!theWindow || !("finishComputeImplicitDiff" in theWindow))
    theWindow = msiGetTopLevelWindow(window);
  try
  {
    theWindow.finishComputeImplicitDiff(data.theMath, editorElement, data);
  } catch(exc) {dump("Exception in ComputeImplicitDiff.js, in trying to call finishComputeImplicitDiff: [" + exc + "].\n");}


//  try
//  {
//    var mathstr = GetFixedMath(data.theMath);
//    msiComputeLogger.Sent4("implicit diff",mathstr,data.thevar,data.about);
//
//    ComputeCursor(editorElement);
//    try {
//      var out = GetCurrentEngine().implicitDiff(mathstr,data.thevar,data.about);
//      msiComputeLogger.Received(out);
//      appendLabeledResult(out, GetComputeString("Solution.fmt"),data.theMath, editorElement);
//    } catch(ex) {
//      msiComputeLogger.Exception(ex);
//    }
//    RestoreCursor(editorElement);
//  } catch(exc) {AlertWithTitle("Error in ComputeImplicitDiff.js", "Exception in OK(); exception is [" + exc + "].");}
//
  return true;
}

function Cancel(){
  data.Cancel = true;
  return true;
}



