// Copyright (c) 2006 MacKichan Software, Inc.  All Rights Reserved.

//const mmlns    = "http://www.w3.org/1998/Math/MathML";
const xhtmlns  = "http://www.w3.org/1999/xhtml";

var data;

// dialog initialization code
function Startup()
{
  var editor = GetCurrentEditor();
  if (!editor) {
    window.close();
    return;
  }

  doSetOKCancel(onAccept, onCancel);
  data = window.arguments[0];
  data.Cancel = false;
  gDialog.preambleText = data.preambleText;

  InitDialog();

  document.getElementById("preambleTextbox").focus();

  SetWindowLocation();
}

function InitDialog()
{
  document.getElementById("preambleTextbox").value = gDialog.preambleText;

//  checkInaccessibleAcceleratorKeys(document.documentElement);

//  gDialog.tabOrderArray = new Array( gDialog.positionSpecGroup,
//                                       document.documentElement.getButton("accept"),
//                                       document.documentElement.getButton("cancel") );
  
  document.documentElement.getButton("accept").setAttribute("default", true);
}

function onAccept()
{
  data.preambleText = document.getElementById("preambleTextbox").value;

  SaveWindowLocation();
  return true;
}

function onCancel()
{
  data.Cancel = true;
}

function doAccept()
{
  document.documentElement.getButton('accept').oncommand();
}
