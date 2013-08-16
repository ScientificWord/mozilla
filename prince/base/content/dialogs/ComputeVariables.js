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

  if (data.description && data.description.length > 0) {
    var instructions = document.getElementById("instructions");
    instructions.setAttribute("value", data.description);
  }

  var theStringSource = GetComputeString("Math.emptyForInput");
  if (("vars" in data) && data.vars.length > 0)
    theStringSource = data.vars;
  try
  {
    var editorControl = document.getElementById("vars-frame");
    msiInitializeEditorForElement(editorControl, theStringSource, true, false);
  }
  catch(exc) {dump("In Startup for ComputeVariables dialog, error initializing editor vars-frame: [" + exc + "].\n");}

}

function OK(){
  data.Cancel = false;

  var doc = document.getElementById("vars-frame").contentDocument;
  var mathnodes = doc.getElementsByTagName("math");
  if (mathnodes.length == 0) {
    dump("No math in variables control in ComputeVariables dialog!\n");
    return false;  // should leave dialog up but doesn't seem to work
  }
  data.vars = runFixup(GetMathAsString(mathnodes[0]));

//  dump("In OK for ComputeVariables dialog, data.dialogTimer is [" + data.dialogTimer + "].\n");
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



