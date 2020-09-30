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


Components.utils.import("resource://app/modules/msiEditorDefinitions.jsm");

var data;
  
// This part pastes data into the editor after the editor has started. 
//const math = '<math>';
//const fullmath = '<math xmlns="http://www.w3.org/1998/Math/MathML">';

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
  var theStringSource1 = data.initialvalue[0];
  var varEditorControl = document.getElementById("mmlArg-var-frame");
  var theStringSource2 = data.initialvalue[1];
  var initEditorControl = document.getElementById("mmlArg-initVal-frame");

  var theStringSource3 = data.initialvalue[2];
  var termsEditorControl = document.getElementById("mmlArg-numTerms-frame");

  var editorInitializer = new msiEditorArrayInitializer();
  editorInitializer.addEditorInfo(varEditorControl, theStringSource1, true, true);
  editorInitializer.addEditorInfo(initEditorControl, theStringSource2, true, true);
  editorInitializer.addEditorInfo(termsEditorControl, theStringSource3, true, true);
  editorInitializer.doInitialize();
  msiSetInitialDialogFocus(varEditorControl);
}

function OK(){
  data.Cancel = false;

  var mathFieldNames = new Array ("mmlArg-var-frame", "mmlArg-initVal-frame", "mmlArg-numTerms-frame");

  // first mathnode is the variable

  //data.mathresult = new Array (3);
  for (var i=0; i<3; ++i)
  {
    var editElement = document.getElementById(mathFieldNames[i]);
    var doc = editElement.contentDocument;
    var mathnodes = doc.getElementsByTagName("math");
    if (mathnodes.length == 0) {
      dump("Not enough math fields in input!\n");
      return false;  // should leave dialog up but doesn't seem to work
    }
    if (HasEmptyMath(mathnodes[0])) {
      var editContentFilter = new msiDialogEditorContentFilter(editElement);
      dump("math has temp input or is empty in edit field [" + mathFieldNames[i] + "]!\n");
      var contentStr = editContentFilter.getMarkupString();
      dump("  Contents are: [" + contentStr + "].\n");
      data.Cancel = true;
      return false;
    }
    data.mathresult[i] = CleanMathString(GetMathAsString(mathnodes[0]));
  }
  
//  var editorElement = msiGetParentEditorElementForDialog(window);
//  var theWindow = window.opener;
//  if (!theWindow || !("finishComputePowerSeries" in theWindow))
//    theWindow = msiGetTopLevelWindow(window);
//   try
//   {
//     theWindow.finishComputePowerSeries(editorElement, data);
//   } catch(exc) {dump("Exception in ComputePowerSeriesArgDialog.js, in trying to call finishComputePowerSeries: [" + exc + "].\n");}

  return true;
}

function Cancel(){
  data.Cancel = true;
  return true;
}

