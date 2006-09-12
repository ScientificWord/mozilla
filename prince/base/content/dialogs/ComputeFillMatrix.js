// Copyright (c) 2004 MacKichan Software, Inc.  All Rights Reserved.

var data;

// implements nsIObserver
var msiEditorDocumentObserver =
{ 
  observe: function(aSubject, aTopic, aData)
  {
    dump("ComputeFillMatrix observer hit!\n");
    var editor = GetCurrentEditor();
    switch(aTopic)
    {
      case "obs_documentCreated":
    
        // type defaults to first choice according to XUL
        Switch("zero");
    }
  }
}


function Startup(){
  data = window.arguments[0];

  var rows = document.getElementById("rows");
  rows.value = data.rows.toString();

  var cols = document.getElementById("cols");
  cols.value = data.cols.toString();

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
  
  // see EditorSharedStartup() in editor.js
  var commandManager = GetCurrentCommandManager();
  commandManager.addCommandObserver(msiEditorDocumentObserver, "obs_documentCreated");
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
  document.getElementById("content-frame").disabled = val;
}

function Cancel(){
  data.Cancel = true;
}



