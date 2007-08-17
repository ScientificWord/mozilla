// Copyright (c) 2004 MacKichan Software, Inc.  All Rights Reserved.

var data;

var computeMomentEditorDocumentObserver =
{ 
//  this.msiEditorElement = editorElement;
//  this.observe = function(aSubject, aTopic, aData)
  observe: function(aSubject, aTopic, aData)
  {
//    dump("ComputeMoment origin editor observer hit!\n");
//    var editor = msiGetEditor(editorElement);
    switch(aTopic)
    {
      case "obs_documentCreated":
        checkEnableControls();
      break;
      case "cmd_setDocumentModified":
        checkEnableControls();
      break;
    }
  }
}

function Startup(){
  data = window.arguments[0];

//  if (data.title && data.title.length > 0) {
//    //hack.  should use window.title in Moz 1.8
//    document.documentElement.setAttribute("title", data.title);
//  }
//  if (data.remark && data.remark.length > 0) {
//    var remark = document.getElementById("math");
//    remark.firstChild.nodeValue = data.remark;
//  }
//  if (data.label && data.label.length > 0) {
//    var center = document.getElementById("center");
//    center.setAttribute("label", data.label);
//  }

  var theStringSource = GetComputeString("Moment.defaultDegree");
  try
  {
    var degreeEditorControl = document.getElementById("degree-frame");
    msiInitializeEditorForElement(degreeEditorControl, theStringSource, true);
  }
  catch(exc) {dump("In Startup for ComputeMoment dialog, error initializing editor degree-frame: [" + exc + "].\n");}

  theStringSource = GetComputeString("Moment.defaultOrigin");
  try
  {
    var originEditorControl = document.getElementById("origin-frame");
    originEditorControl.mInitialDocObserver = new Array(1);
//    originEditorControl.mInitialDocObserver = new Array(2);
    originEditorControl.mInitialDocObserver[0] = new Object();
    originEditorControl.mInitialDocObserver[0].mObserver = computeMomentEditorDocumentObserver;
    originEditorControl.mInitialDocObserver[0].mCommand = "obs_documentCreated";
//    originEditorControl.mInitialDocObserver[1] = new Object();
//    originEditorControl.mInitialDocObserver[1].mObserver = computeMomentEditorDocumentObserver;
//    originEditorControl.mInitialDocObserver[1].mCommand = "cmd_setDocumentModified";
    msiInitializeEditorForElement(originEditorControl, theStringSource, true);
  }
  catch(exc) {dump("In Startup for ComputeMoment dialog, error initializing editor origin-frame: [" + exc + "].\n");}

//  checkEnableControls();

////SLS the following copied from editor.js
//  gSourceContentWindow = document.getElementById("content-frame");
//  gSourceContentWindow.makeEditable("html", false);
//
//  EditorStartup();
//
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
//  } catch (e) { dump("makeEditable failed in ComputeMoment.Startup(): "+e+"\n"); }
}

function checkEnableControls()
{
  var origin = document.getElementById("origin");
  var bExplicitOrigin = (origin.selectedIndex != 0);
  var originEditor = document.getElementById("origin-frame");
  msiEnableEditorControl(originEditor, bExplicitOrigin);
  var bIsOK = true;
  if (bExplicitOrigin)
  {
    var doc = originEditor.contentDocument;
    var mathnodes = doc.getElementsByTagName("math");
    if (mathnodes.length == 0 || HasEmptyMath(mathnodes[0]))
      bIsOK = false;
  }
  var okButton = document.documentElement.getButton("accept");
  if (!bIsOK)
    okButton.disabled = true;
  else if (okButton.hasAttribute("disabled"))
    okButton.removeAttribute("disabled");
}

function OK(){
  data.Cancel = false;

  //validate?
  var doc = document.getElementById("degree-frame").contentDocument;
  var mathnodes = doc.getElementsByTagName("math");
  if (mathnodes.length == 0) {
    dump("No math in degree field!\n");
    return false;  // should leave dialog up but doesn't seem to work
  }
  data.thevar = GetMathAsString(mathnodes[0]);
  doc = document.getElementById("origin-frame").contentDocument;
  var mathnodes = doc.getElementsByTagName("math");
  if (mathnodes.length == 0 || HasEmptyMath(mathnodes[0])) {
    dump("Empty math field found in origin control in ComputeMoment dialog, returning 0 for origin - this shouldn't happen!\n");
    data.about = "<math><mn>0</mn></math>";
  } else {
    data.about = GetMathAsString(mathnodes[0]);
  }

  var origin = document.getElementById("origin");
  data.meanisorigin  = (origin.selectedIndex==0);

  var parentEditorElement = msiGetParentEditorElementForDialog(window);
  var parentWin =window.opener;
  if (!parentWin || !("finishComputeMoment" in parentWin))
    parentWin = msiGetTopLevelWindow(window);
  try
  {
    parentWin.finishComputeMoment(parentEditorElement, data);
  } catch(exc) { dump("Exception in ComputeMoment dialog, trying to call finishComputeMoment: [" + exc + "].\n"); }

  return true;
}

function Cancel(){
  data.Cancel = true;
  return true;
}



