// Copyright (c) 2004 MacKichan Software, Inc.  All Rights Reserved.

var data;
const math = '<math>';
//const fullmath = '<math xmlns="http://www.w3.org/1998/Math/MathML">';

// implements nsIObserver
var msiEditorDocumentObserver =
{ 
  observe: function(aSubject, aTopic, aData)
  {
    dump("ComputeVarList observer hit!\n");
    var editor = GetCurrentEditor();
    switch(aTopic)
    {
      case "obs_documentCreated":
    
        // try to paste data into editor!
        var doc = document.getElementById("content-frame").contentDocument;
        var body = doc.getElementsByTagName("body");
        editor.insertHTMLWithContext(
            data.vars.replace(math,fullmath),
            "", "", "", null,
            body[0], 0, false );
    }
  }
}


function Startup(){
  data = window.arguments[0];

  var theStringSource = GetComputeString("Math.emptyForInput");
  if (("vars" in data) && data.vars.length > 0)
    theStringSource = data.vars.replace(math, fullmath);
  try
  {
    var editorControl = document.getElementById("varList-frame");
    msiInitializeEditorForElement(editorControl, theStringSource, true);
    editorControl.makeEditable("html", false);
  }
  catch(exc) {dump("In Startup for ComputeVarLists dialog, error initializing editor varList-frame: [" + exc + "].\n");}
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
//  } catch (e) { dump("makeEditable failed in Startup(): "+e+"\n"); }
//  
//  // see EditorSharedStartup() in editor.js
//  var commandManager = GetCurrentCommandManager();
//  commandManager.addCommandObserver(msiEditorDocumentObserver, "obs_documentCreated");
}

function OK(){
  data.Cancel = false;

  var doc = document.getElementById("varList-frame").contentDocument;
  var mathnodes = doc.getElementsByTagName("math");
  if (mathnodes.length == 0) {
    dump("No math in varList!\n");
    return false;  // should leave dialog up but doesn't seem to work
  }
  data.vars = GetMathAsString(mathnodes[0]);
  return true;
}

function doPostDialogFunction()
{
  var editorElement = msiGetParentEditorElementForDialog(window);
//  dump("Hit postdialog function in ComputeVariables dialog.\n");
  ShutdownEditorsForWindow();
  if (data.mParentWin && ("postDialogTimerCallback" in data.mParentWin))
    data.mDialogTimer = data.mParentWin.setTimeout(data.mParentWin.postDialogTimerCallback, 0, editorElement, data);
//  if (data!=null && ("afterDialog" in data) && data.afterDialog != null)
//    data.afterDialog(editorElement);
}

function Cancel(){
  data.Cancel = true;
  return true;
}



