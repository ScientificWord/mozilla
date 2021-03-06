// Copyright (c) 2006 MacKichan Software, Inc.  All Rights Reserved.

//const mmlns    = "http://www.w3.org/1998/Math/MathML";
//const xhtmlns  = "http://www.w3.org/1999/xhtml";
Components.utils.import("resource://app/modules/msiEditorDefinitions.jsm");

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
  gDialog.outputChoice = data.outputChoice;
  if (gDialog.outputChoice.length <= 0)
    gDialog.outputChoice = "dvi";

  InitDialog();

//  document.getElementById("prepareForRadioGroup").focus();
  msiSetInitialDialogFocus(document.getElementById("prepareForRadioGroup"));

  SetWindowLocation();
}

function InitDialog()
{
//  checkInaccessibleAcceleratorKeys(document.documentElement);

//  gDialog.tabOrderArray = new Array( gDialog.positionSpecGroup,
//                                       document.documentElement.getButton("accept"),
//                                       document.documentElement.getButton("cancel") );
  var theRadioGroup = document.getElementById("prepareForRadioGroup");
  var selectedItems = theRadioGroup.getElementsByAttribute("value", gDialog.outputChoice);
  if (!selectedItems.length)
    selectedItems = theRadioGroup.getElementsByTagName("radio");
  if (selectedItems.length > 0)
    theRadioGroup.selectedItem = selectedItems[0];
  
  document.documentElement.getButton("accept").setAttribute("default", true);
}

function onAccept()
{
  var selItem = document.getElementById("prepareForRadioGroup").selectedItem;
  data.outputChoice = selItem.value;

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
