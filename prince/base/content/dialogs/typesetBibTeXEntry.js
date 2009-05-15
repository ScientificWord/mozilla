// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

var gBodyElement;

const emptyElementStr=" ";

//const mmlns    = "http://www.w3.org/1998/Math/MathML";
//const xhtmlns  = "http://www.w3.org/1999/xhtml";

var data;

// dialog initialization code
function Startup()
{
  doSetOKCancel(onAccept, onAccept);
  data = window.arguments[0];
  data.Cancel = false;
  gDialog.theData = data.theData;

  InitDialog();

//  document.getElementById("dataField").focus();
  msiSetInitialDialogFocus(document.getElementById("dataField"));

  SetWindowLocation();
}

function InitDialog()
{
  document.getElementById("dataField").value = gDialog.theData;

//  checkInaccessibleAcceleratorKeys(document.documentElement);

//  gDialog.tabOrderArray = new Array( gDialog.positionSpecGroup,
//                                       document.documentElement.getButton("accept"),
//                                       document.documentElement.getButton("cancel") );
  
//  document.documentElement.getButton("accept").setAttribute("hidden", true);
  document.documentElement.getButton("cancel").setAttribute("default", true);
}

function onAccept()
{
  data.Cancel = true;  //not that it matters
  SaveWindowLocation();
  return true;
}

function onCancel()
{
  data.Cancel = true;
}

function doAccept()
{
  document.documentElement.getButton('cancel').oncommand();
}
