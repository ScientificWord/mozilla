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
  gDialog.bBibTeX = false;
  if (data.bBibTeX == true)
	gDialog.bBibTeX = true;

  InitDialog();

  if (data.bBibTeX)
  {
    document.getElementById("BibTeXButton").focus();
    document.getElementById("BibTeXButton").setAttribute("selected", "true");
  }
  else
  {
    document.getElementById("manualBibButton").focus();
    document.getElementById("manualBibButton").setAttribute("selected", "true");
  }

  SetWindowLocation();
}

function InitDialog()
{
//  checkInaccessibleAcceleratorKeys(document.documentElement);

//  gDialog.tabOrderArray = new Array( gDialog.positionSpecGroup,
//                                       document.documentElement.getButton("accept"),
//                                       document.documentElement.getButton("cancel") );
  
  document.documentElement.getButton("accept").setAttribute("default", true);
}

function onAccept()
{
  data.bBibTeX = document.getElementById("BibTeXButton").selected;

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
