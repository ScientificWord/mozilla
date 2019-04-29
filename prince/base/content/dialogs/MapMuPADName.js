
// Copyright (c) 2004 MacKichan Software, Inc.  All Rights Reserved.

var data;

function Startup(){
  data = window.arguments[0];
  try
  {
    var theStringSource = GetComputeString("Math.emptyForInput");
    var editorControl = document.getElementById("swpnameInput");
    msiInitializeEditorForElement(editorControl, theStringSource, true, null, true);
  }
  catch(exc) 
  {
    dump("In Startup for MapMuPADName dialog, error initializing editor swpnameInput: [" + exc + "].\n");
  }

}

function OK(){
  data.Cancel = false;
  var theStringSource = GetComputeString("Math.emptyForInput");
  var doc = document.getElementById("swpnameInput").contentDocument;
  var mathnodes = doc.getElementsByTagName("math");

  if (mathnodes.length == 0) {
    return false;  // should leave dialog up but doesn't seem to work
  }

  data.swpname = runFixup(GetMathAsString(mathnodes[0]));

  var mupname = document.getElementById("mupadname.input");
  data.mupadname = mupname.value;

  var infile = document.getElementById("infile.input");
  data.infile = infile.value;
    
   
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




