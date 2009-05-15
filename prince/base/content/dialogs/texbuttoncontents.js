// Copyright (c) 2006 MacKichan Software, Inc.  All Rights Reserved.

//const xhtmlns  = "http://www.w3.org/1999/xhtml";

var data;

// dialog initialization code
function Startup()
{
  var editorElement = msiGetParentEditorElementForDialog(window);
  var editor = msiGetEditor(editorElement);
  if (!editor) {
    window.close();
    return;
  }
  doSetOKCancel(onAccept, onCancel);
  data = window.arguments[0];
  data.Cancel = false;
  gDialog.texbuttonText = data.tex;

  InitDialog();

//  document.getElementById("texbuttonTextbox").focus();
  msiSetInitialDialogFocus(document.getElementById("texbuttonTextbox"));

  SetWindowLocation();
}

function InitDialog()
{
  document.getElementById("texbuttonTextbox").value = gDialog.texbuttonText;

  document.documentElement.getButton("accept").setAttribute("default", true);
}

function onAccept()
{
  data.tex = document.getElementById("texbuttonTextbox").value;
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
