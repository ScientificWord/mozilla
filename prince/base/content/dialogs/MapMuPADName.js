
// Copyright (c) 2004 MacKichan Software, Inc.  All Rights Reserved.

var data;

function Startup(){
  data = window.arguments[0];
  try
  {
    var theStringSource = GetComputeString("Math.emptyForInput");
    var editorControl = document.getElementById("swpname.input");
    msiInitializeEditorForElement(editorControl, theStringSource, true);
  }
  catch(exc) 
  {
    dump("In Startup for MapMuPADName dialog, error initializing editor swpname.input: [" + exc + "].\n");
  }

}

