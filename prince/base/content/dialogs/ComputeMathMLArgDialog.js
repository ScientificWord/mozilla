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
//const math = '<math>';
//const fullmath = '<math xmlns="http://www.w3.org/1998/Math/MathML">';

function createContentString()
{
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
  return str;
}

function Startup(){
  data = window.arguments[0];
  if (data.title && data.title.length > 0) {
      document.documentElement.setAttribute("title", data.title);
  }
  var captionControl = document.getElementById("fnprompt");
  captionControl.label = data.prompt[0];
//  captionControl.firstChild.nodeValue = data.prompt[0];
  captionControl = document.getElementById("initprompt");
  captionControl.label = data.prompt[1];
  captionControl = document.getElementById("termsprompt");
  captionControl.label = data.prompt[2];

  // Initialize our source text <editor>s
  var theStringSource1 = "";
  var theStringSource2 = "";
  var theStringSource3 = "";
  if ( data && ("initialvalue" in data) && (data.initialvalue.length > 0) )
  {
    theStringSource1 = data.initialvalue[0];
    if (data.initialvalue.length > 1)
    {
      theStringSource2 = data.initialvalue[1];
      if (data.initialvalue.length > 2)
        theStringSource3 = data.initialvalue[2];
    }
  }
  var exprEditorControl = document.getElementById("mmlArg-expr-frame");
  var initEditorControl = document.getElementById("mmlArg-initVal-frame");
  var termsEditorControl = document.getElementById("mmlArg-numTerms-frame");

//  try
//  {
//    var exprEditorControl = document.getElementById("mmlArg-expr-frame");
////  editorControl.overrideStyleSheets = new Array("chrome://prince/skin/MathVarsDialog.css");
//    msiInitializeEditorForElement(exprEditorControl, theStringSource, true);
//  }
//  catch(exc) {dump("In Startup for ComputeMathMLArgDialog, error initializing editor mmlArg-expr-frame: [" + exc + "].\n");}
//  try
//  {
//  var theStringSource2 = data.initialvalue[1];
//    var initEditorControl = document.getElementById("mmlArg-initVal-frame");
//    msiInitializeEditorForElement(initEditorControl, theStringSource, true);
//  }
//  catch(exc) {dump("In Startup for ComputeMathMLArgDialog, error initializing editor mmlArg-initVal-frame: [" + exc + "].\n");}
//  try
//  {
//  var theStringSource3 = data.initialvalue[2];
//    var termsEditorControl = document.getElementById("mmlArg-numTerms-frame");
//    msiInitializeEditorForElement(termsEditorControl, theStringSource, true);
//  }
//  catch(exc) {dump("In Startup for ComputeMathMLArgDialog, error initializing editor mmlArg-numTerms-frame: [" + exc + "].\n");}

  var editorInitializer = new msiEditorArrayInitializer();
  editorInitializer.addEditorInfo(exprEditorControl, theStringSource1, true);
  editorInitializer.addEditorInfo(initEditorControl, theStringSource2, true);
  editorInitializer.addEditorInfo(termsEditorControl, theStringSource3, true);
  editorInitializer.doInitialize();
}

function OK(){
  data.Cancel = false;

  var mathFieldNames = new Array ("mmlArg-expr-frame", "mmlArg-initVal-frame", "mmlArg-numTerms-frame");

  // first mathnode is the variable

  data.mathresult = new Array (3);
  for (var i=0; i<3; ++i)
  {
    var doc = document.getElementById(mathFieldNames[i]).contentDocument;
    var mathnodes = doc.getElementsByTagName("math");
    if (mathnodes.length == 0) {
      dump("Not enough math fields in input!\n");
      return false;  // should leave dialog up but doesn't seem to work
    }
    if (HasEmptyMath(mathnodes[0])) {
      dump("math has temp input or is empty!\n");
      data.Cancel = true;
      return false;
    }
    data.mathresult[i] = CleanMathString(GetMathAsString(mathnodes[0]));
  }
  
  var editorElement = msiGetParentEditorElementForDialog(window);
  var editor = msiGetEditor(editorElement);
  var theWindow = window.opener;
  if (!theWindow || !("finishComputeIterate" in theWindow))
    theWindow = msiGetTopLevelWindow(window);
  try
  {
    theWindow.finishComputeIterate(editorElement, data);
  } catch(exc) {dump("Exception in ComputeMathMLArgDialog.js, in trying to call finishComputeIterate: [" + exc + "].\n");}

  return true;
}

function Cancel(){
  data.Cancel = true;
  return true;
}

