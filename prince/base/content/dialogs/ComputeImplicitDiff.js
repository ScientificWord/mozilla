// Copyright (c) 2004 MacKichan Software, Inc.  All Rights Reserved.

var data;

function Startup(){
  data = window.arguments[0];

  if (data.title && data.title.length > 0) {
    //hack.  should use window.title in Moz 1.8
    document.documentElement.setAttribute("title", data.title);
  }
  if (data.remark && data.remark.length > 0) {
    var remark = document.getElementById("math");
    remark.firstChild.nodeValue = data.remark;
  }
  if (data.label && data.label.length > 0) {
    var center = document.getElementById("center");
    center.setAttribute("label", data.label);
  }

//SLS the following copied from editor.js
  gSourceContentWindow = document.getElementById("content-frame");
  gSourceContentWindow.makeEditable("html", false);

  EditorStartup();

  // Initialize our source text <editor>
  try {
    gSourceTextEditor = gSourceContentWindow.getEditor(gSourceContentWindow.contentWindow);
//SLS don't know why this doesn't work here...doesn't really matter since we have no source view.
//    gSourceTextEditor.QueryInterface(Components.interfaces.nsIPlaintextEditor);
//    gSourceTextEditor.enableUndo(false);
//    gSourceTextEditor.rootElement.style.fontFamily = "-moz-fixed";
//    gSourceTextEditor.rootElement.style.whiteSpace = "pre";
//    gSourceTextEditor.rootElement.style.margin = 0;
    var controller = Components.classes["@mozilla.org/embedcomp/base-command-controller;1"]
                               .createInstance(Components.interfaces.nsIControllerContext);
    controller.init(null);
    controller.setCommandContext(gSourceContentWindow);
    gSourceContentWindow.contentWindow.controllers.insertControllerAt(0, controller);
    var commandTable = controller.QueryInterface(Components.interfaces.nsIInterfaceRequestor)
                                 .getInterface(Components.interfaces.nsIControllerCommandTable);
    commandTable.registerCommand("cmd_find",        nsFindCommand);
    commandTable.registerCommand("cmd_findNext",    nsFindAgainCommand);
    commandTable.registerCommand("cmd_findPrev",    nsFindAgainCommand);
    
    SetupMSIMathMenuCommands();

  } catch (e) { dump("makeEditable failed in Startup(): "+e+"\n"); }
}

function OK(){
  data.Cancel = false;

  //validate?
  var doc = document.getElementById("content-frame").contentDocument;
  var mathnodes = doc.getElementsByTagName("math");
  if (mathnodes.length == 0) {
    dump("No math in center field!\n");
    return false;  // should leave dialog up but doesn't seem to work
  }
  data.thevar = GetMathAsString(mathnodes[0]);
  if (mathnodes.length > 1) {
    data.about = GetMathAsString(mathnodes[1]);
  } else {
    dump("Only one math field found, returning 0 for second.\n");
    data.about = "<math><mn>0</mn></math>";
  }
  return true;
}

function Cancel(){
  data.Cancel = true;
  return true;
}



