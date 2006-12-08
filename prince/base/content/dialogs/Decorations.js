// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

var gBodyElement;

const defaultMathColor="#000000";
const cssBackgroundColorStr = "background-color";
const emptyElementStr=" ";
//const colorStyle = cssColorStr + ": ";

//const mmlns    = "http://www.w3.org/1998/Math/MathML";
//const xhtmlns  = "http://www.w3.org/1999/xhtml";

var customMathColor;

var data;

// dialog initialization code
function Startup()
{
  var editorElement = msiGetParentEditorElementForDialog(window);
  var editor = msiGetEditor(editorElement);
//  var editor = GetCurrentEditor();
  if (!editor) {
    window.close();
    return;
  }

  doSetOKCancel(onAccept, onCancel);
  data = window.arguments[0];
  data.Cancel = false;

  gDialog.decorationAboveStr = data.decorationAboveStr;
  gDialog.decorationBelowStr = data.decorationBelowStr;
  gDialog.decorationAroundStr = data.decorationAroundStr;
  if (!gDialog.decorationAboveStr.length && !gDialog.decorationBelowStr.length && !gDialog.decorationAroundStr.length)
    gDialog.decorationAboveStr = String.fromCharCode(0x00AF);
//  gDialog.decorationPos = data.decorationPos;
//  if (!gDialog.decorationPos.length)
//    gDialog.decorationPos = "over";

  gDialog.DecorationsAboveGroup = document.getElementById("decorationsAboveButtonGroup");
  gDialog.DecorationsBelowGroup = document.getElementById("decorationsBelowButtonGroup");
  gDialog.DecorationsAroundGroup = document.getElementById("decorationsAroundButtonGroup");

  InitDialog();

  if (gDialog.DecorationsAboveGroup.valueStr.length > 0)
    gDialog.DecorationsAboveGroup.focus();
  else if (gDialog.DecorationsBelowGroup.valueStr.length > 0)
    gDialog.DecorationsBelowGroup.focus();
  else if (gDialog.DecorationsAroundGroup.valueStr.length > 0)
    gDialog.DecorationsAroundGroup.focus();
  else
    gDialog.DecorationsAboveGroup.focus();

  SetWindowLocation();
}

function InitDialog()
{
  makeMSIButtonGroup(gDialog.DecorationsAboveGroup, false);
  var nWhichSel = -1;
  if (gDialog.decorationAboveStr)
  {
    nWhichSel = setSelectionByValue(gDialog.DecorationsAboveGroup, gDialog.decorationAboveStr);
    if (nWhichSel < 0)
      setSelectionByIndex(0, gDialog.DecorationsAboveGroup.id);
  }

  makeMSIButtonGroup(gDialog.DecorationsBelowGroup, false);
  nWhichSel = -1;
  if (gDialog.decorationBelowStr)
  {
    nWhichSel = setSelectionByValue(gDialog.DecorationsBelowGroup, gDialog.decorationBelowStr);
    if (nWhichSel < 0)
      setSelectionByIndex(0, gDialog.DecorationsBelowGroup.id);
  }

  makeMSIButtonGroup(gDialog.DecorationsAroundGroup, false);
  nWhichSel = -1;
  if (gDialog.decorationAroundStr)
  {
    nWhichSel = setSelectionByValue(gDialog.DecorationsAroundGroup, gDialog.decorationAroundStr);
    if (nWhichSel < 0)
      setSelectionByIndex(0, gDialog.DecorationsAroundGroup.id);
  }

  checkInaccessibleAcceleratorKeys(document.documentElement);

  gDialog.tabOrderArray = new Array( gDialog.DecorationsAboveGroup, gDialog.DecorationsBelowGroup,
                                       gDialog.DecorationsAroundGroup,
                                       document.documentElement.getButton("accept"),
                                       document.documentElement.getButton("cancel") );

  document.documentElement.addEventListener('ButtonGroupSelectionChange', checkEnableControls, false);
  checkEnableControls();
  
  document.documentElement.getButton("accept").setAttribute("default", true);
}

function checkEnableControls(event)
{
  var disableAboveBelowStr = "false";
  var disableAroundStr = "false";
  var disableOKStr = "false";
  if (gDialog.DecorationsAboveGroup.valueStr.length > 0 || gDialog.DecorationsBelowGroup.valueStr.length > 0)
  {
    disableAroundStr = "true";
  }
  else if (gDialog.DecorationsAroundGroup.valueStr.length > 0)
  {
    disableAboveBelowStr = "true";
  }
  else
    disableOKStr = "true";

  document.getElementById("enableDecorationsAboveBelow").setAttribute("disabled", disableAboveBelowStr);
  document.getElementById("enableDecorationsAboveBelow").setAttribute("disabled", disableAboveBelowStr);
  document.getElementById("enableDecorationsAround").setAttribute("disabled", disableAroundStr);
  document.documentElement.getButton("accept").setAttribute("disabled", disableOKStr);
}

function onAccept()
{
  gDialog.decorationAboveStr = gDialog.DecorationsAboveGroup.valueStr;
  gDialog.decorationBelowStr = gDialog.DecorationsBelowGroup.valueStr;
  gDialog.decorationAroundStr = gDialog.DecorationsAroundGroup.valueStr;
  data.decorationAboveStr = gDialog.decorationAboveStr;
  data.decorationBelowStr = gDialog.decorationBelowStr;
  data.decorationAroundStr = gDialog.decorationAroundStr;

//  var editorElement = msiGetParentEditorElementForDialog(window);

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