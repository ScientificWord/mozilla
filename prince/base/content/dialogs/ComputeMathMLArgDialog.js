// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.
// This is intended to provide an edit window for an arbitrary number of math
// inputs. The edit window contains a table with two columns: the left column
// contains prompt strings. The right column contains mathml elements.

// To use: pass in an object with the following attributes:
//    .fieldcount     : number of fields to display in the table
//    .prompt[]       : an array of strings with prompts
//    .initialvalue[] :	an array of strings with initial values. These should
//                      be mathml strings (which may be stored in the 
//                      compute.properties file)
// This returns:
//    .mathresult[]   : an array of mathml strings acquired by the dialog

var data;
  
// This part pastes data into the editor after the editor has started. 
const math = '<math>';
const fullmath = '<math xmlns="http://www.w3.org/1998/Math/MathML">';

// implements nsIObserver
var msiEditorDocumentObserver =
{ 
  observe: function(aSubject, aTopic, aData)
  { // create an editor window with a 2-column table

    var editor = GetCurrentEditor();
    if (aTopic == "obs_documentCreated") {
      var doc = document.getElementById("content-frame").contentDocument;
      var body = doc.getElementsByTagName("body");

      var str = "";
      str += "<table class=\"MathVarsDialog\" xmlns=\"http://www.w3.org/1999/xhtml\">";
      str += "<tbody>";
	
      var i = 0;
      while (i < data.fieldcount) {
        str += "<tr>";               
        str += "<td class=\"label\">";               
        str += data.prompt[i];       
        str += "</td>";              
        str += "<td class=\"value\">";               
        str += data.initialvalue[i]; 
        str += "</td>";      
        str += "</tr>";        
        ++i;
      }
         
      str += "</tbody>";
      str += "</table>";
      editor.addOverrideStyleSheet("chrome://editor/content/MathVarsDialog.css");
      editor.insertHTML(str);
    }  
  }
}


function Startup(){
  data = window.arguments[0];
  if (data.title && data.title.length > 0) {
      document.documentElement.setAttribute("title", data.title);
  }

  //SLS the following copied from editor.js
  gSourceContentWindow = document.getElementById("content-frame");
  gSourceContentWindow.makeEditable("html", false);

  EditorStartup();

  // Initialize our source text <editor>
  try {
    gSourceTextEditor = gSourceContentWindow.getEditor(gSourceContentWindow.contentWindow);
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

  var doc = document.getElementById("content-frame").contentDocument;
  var mathnodes = doc.getElementsByTagName("math");

  // first mathnode is the variable
  if (mathnodes.length < data.fieldcount) {
    dump("Not enough math fields in input!\n");
    return false;  // should leave dialog up but doesn't seem to work
  }

  data.mathresult = new Array (data.fieldcount);
  for (var i=0; i<data.fieldcount; ++i) {
    if (HasEmptyMath(mathnodes[i])) {
      dump("math has temp input or is empty!\n");
      data.Cancel = true;
      return false;
    }
    data.mathresult[i] = CleanMathString(GetMathAsString(mathnodes[i]));
  }
  return true;
}

function Cancel(){
  data.Cancel = true;
  return true;
}

