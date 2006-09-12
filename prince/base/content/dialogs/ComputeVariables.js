// Copyright (c) 2004 MacKichan Software, Inc.  All Rights Reserved.

var data;

function Startup(){
  data = window.arguments[0];

  if (data.title && data.title.length > 0) {
    //hack.  should use window.title in Moz 1.8
    document.documentElement.setAttribute("title", data.title);
  }
  if (data.label && data.label.length > 0) {
    var label = document.getElementById("label");
    label.setAttribute("label", data.label);
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

  var doc = document.getElementById("content-frame").contentDocument;
  var mathnodes = doc.getElementsByTagName("math");
  if (mathnodes.length == 0) {
    dump("No math in center field!\n");
    return false;  // should leave dialog up but doesn't seem to work
  }
  data.vars = GetMathAsString(mathnodes[0]);
  return true;
}

function Cancel(){
  data.Cancel = true;
  return true;
}



