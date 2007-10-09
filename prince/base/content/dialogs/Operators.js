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
  if (!editor) {
    window.close();
    return;
  }

  doSetOKCancel(onAccept, onCancel);
  data = window.arguments[0];
  data.Cancel = false;

  gDialog.operatorStr = data.operator;
  if (!gDialog.operatorStr.length)
    gDialog.operatorStr = String.fromCharCode(0x222B);
  gDialog.limitsSpec = data.limitsSpec;
  gDialog.sizeSpec = data.sizeSpec;

  gDialog.OperatorsGroup = document.getElementById("operatorsButtonGroup");
  gDialog.limitsSpecGroup = document.getElementById("limitPositionRadioGroup");
  gDialog.sizeSpecGroup = document.getElementById("sizeRadioGroup");

  InitDialog();

  window.mMSIDlgManager = new msiDialogConfigManager(window);
  window.mMSIDlgManager.configureDialog();

  gDialog.limitsSpecGroup.focus();

  SetWindowLocation();
}

function InitDialog()
{
  makeMSIButtonGroup(gDialog.OperatorsGroup, true);
  var nWhichSel = -1;
  nWhichSel = setSelectionByValue(gDialog.OperatorsGroup, gDialog.operatorStr);
  if (nWhichSel < 0)
    setSelectionByIndex(0, gDialog.OperatorsGroup.id);

  var limitSpecItems = gDialog.limitsSpecGroup.getElementsByAttribute("value", gDialog.limitsSpec);
  if (!limitSpecItems.length)
    limitSpecItems = gDialog.limitsSpecGroup.getElementsByAttribute("group", gDialog.limitsSpecGroup.id);
  if (!limitSpecItems.length)
    limitSpecItems = gDialog.limitsSpecGroup.getElementsByTagName("radio");
  if (limitSpecItems.length > 0)
    gDialog.limitsSpecGroup.selectedItem = limitSpecItems[0];

  var sizeSpecItems = gDialog.sizeSpecGroup.getElementsByAttribute("value", gDialog.sizeSpec);
  if (!sizeSpecItems.length)
    sizeSpecItems = gDialog.sizeSpecGroup.getElementsByAttribute("group", gDialog.sizeSpecGroup.id);
  if (!sizeSpecItems.length)
    sizeSpecItems = gDialog.sizeSpecGroup.getElementsByTagName("radio");
  if (sizeSpecItems.length > 0)
    gDialog.sizeSpecGroup.selectedItem = sizeSpecItems[0];

  checkInaccessibleAcceleratorKeys(document.documentElement);

  gDialog.tabOrderArray = new Array( gDialog.limitsSpecGroup, gDialog.sizeSpecGroup, gDialog.OperatorsGroup,
                                       document.documentElement.getButton("accept"),
                                       document.documentElement.getButton("cancel") );
  
  document.documentElement.getButton("accept").setAttribute("default", true);
}

function onAccept()
{
  gDialog.operatorStr = gDialog.OperatorsGroup.valueStr;
  data.operator = gDialog.operatorStr;
  data.limitsSpec = gDialog.limitsSpecGroup.value;
  data.sizeSpec = gDialog.sizeSpecGroup.value;

  var editorElement = msiGetParentEditorElementForDialog(window);
  var theWindow = window.opener;
  if (!theWindow || !("insertOperator" in theWindow))
    theWindow = msiGetTopLevelWindow();
  theWindow.insertOperator(data.operator, data.limitsSpec, data.sizeSpec, editorElement);

  SaveWindowLocation();
  return true;
//  return false;
}

function onCancel()
{
  data.Cancel = true;
}

function doAccept()
{
  document.documentElement.getButton('accept').oncommand();
}