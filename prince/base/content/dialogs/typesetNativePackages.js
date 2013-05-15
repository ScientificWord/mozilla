// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

var gBodyElement;
var gDialog;

const emptyElementStr=" ";

//const mmlns    = "http://www.w3.org/1998/Math/MathML";
//const xhtmlns  = "http://www.w3.org/1999/xhtml";

var data;

// dialog initialization code
function Startup()
{
//  var editor = GetCurrentEditor();
  var editorElement = msiGetParentEditorElementForDialog(window);
  var editor = msiGetEditor(editorElement);
  if (!editor) {
    window.close();
    return;
  }

  doSetOKCancel(onAccept, onCancel);
  data = window.arguments[0];
  data.Cancel = false;
  gDialog.packages = data.packages;
  gDialog.suppressManagement = false;
  if (data.suppressManagement != null)
    gDialog.suppressManagement = data.suppressManagement;

  InitDialog();

//  document.getElementById("packagesTextbox").focus();
  msiSetInitialDialogFocus(document.getElementById("packagesTextbox"));

  SetWindowLocation();
}

function InitDialog()
{
  document.getElementById("packagesTextbox").value = packageListToString(gDialog.packages);
  document.getElementById("suppressManagementCheckbox").checked = gDialog.suppressManagement;

//  checkInaccessibleAcceleratorKeys(document.documentElement);

//  gDialog.tabOrderArray = new Array( gDialog.positionSpecGroup,
//                                       document.documentElement.getButton("accept"),
//                                       document.documentElement.getButton("cancel") );
  
  document.documentElement.getButton("accept").setAttribute("default", true);
}

function onAccept()
{
  data.packages = packageAndOptionsStringToPackageList(document.getElementById("packagesTextbox").value);
  data.suppressManagement = document.getElementById("suppressManagementCheckbox").checked;

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
