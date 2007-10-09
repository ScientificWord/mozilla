// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

var gBodyElement;

//const defaultMathColor="#000000";
//const cssBackgroundColorStr = "background-color";
//const emptyElementStr=" ";

//const mmlns    = "http://www.w3.org/1998/Math/MathML";
//const xhtmlns  = "http://www.w3.org/1999/xhtml";

//var customMathColor;

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
  if (("arguments" in window) && window.arguments !=  null)
    data = window.arguments[0];
  else
    data = new Object();
  data.Cancel = false;

  InitDialog();

  window.mMSIDlgManager = new msiDialogConfigManager(window);
  window.mMSIDlgManager.configureDialog();

  document.getElementById("liftSizeTextbox").focus();

  SetWindowLocation();
}

var dataComponents = ["lift", "width", "height"];

function InitDialog()
{
  setUpData();
  setUnitsAndPositionControls();
  setColorWell("colorWell", data.ruleColor);
}

function setUpData()
{
  for (var ix = 0; ix < dataComponents.length; ++ix)
  {
    if (!(dataComponents[ix] in data) || (data[dataComponents[ix]] == null))
    {
      data[dataComponents[ix]] = new Object();
      data[dataComponents[ix]].units = "pt";
      data[dataComponents[ix]].size = 0.00;
    }
  }
  if (!("ruleColor" in data) || data.ruleColor == null)
    data.ruleColor = "black";
}

function changeUnits(listbox)
{
  if (!("mUnitsController" in listbox))
    return;
  if (listbox.selectedItem == undefined || listbox.selectedItem.value == undefined)
    dump("In msiRulesDilaog.js, changeUnits; selectedItem.value is undefined!\n");
  else
    listbox.mUnitsController.changeUnits(listbox.selectedItem.value);
}

function setUnitsAndPositionControls()
{
  var unitsListBox = document.getElementById("sizeUnitsbox");
  var unitsControlGroup = [document.getElementById("liftSizeTextbox"), document.getElementById("widthSizeTextbox"),
                             document.getElementById("heightSizeTextbox")];
  unitsListBox.mUnitsController = new msiUnitsListbox(unitsListBox, unitsControlGroup);
  var unitsValueGroup = new Array(3);
  for (var ix = 0; ix < dataComponents.length; ++ix)
    unitsValueGroup[ix] = String(data[dataComponents[ix]].size) + data[dataComponents[ix]].units;
  unitsListBox.mUnitsController.setUp(data[dataComponents[0]].units, unitsValueGroup);
//  checkInaccessibleAcceleratorKeys(document.documentElement);

//  gDialog.tabOrderArray = new Array( gDialog.limitsSpecGroup, gDialog.sizeSpecGroup, gDialog.OperatorsGroup,
//                                       document.documentElement.getButton("accept"),
//                                       document.documentElement.getButton("cancel") );
  
//  document.documentElement.getButton("accept").setAttribute("default", true);
}

function getColorAndUpdate()
{
  var colorWell = document.getElementById("colorWell");
  if (!colorWell) return;

  // Don't allow a blank color, i.e., using the "default"
  var colorObj = { NoDefault:true, Type:"Rule", TextColor:data.ruleColor, PageColor:0, Cancel:false };

  window.openDialog("chrome://editor/content/EdColorPicker.xul", "_blank", "chrome,close,titlebar,modal", "", colorObj);

  // User canceled the dialog
  if (colorObj.Cancel)
    return;

  data.ruleColor = colorObj.TextColor;
  setColorWell("colorWell", data.ruleColor); 
}

function onAccept()
{
  var theUnits = document.getElementById("sizeUnitsbox").value;
  var sizeBoxes = [document.getElementById("liftSizeTextbox"), document.getElementById("widthSizeTextbox"),
                             document.getElementById("heightSizeTextbox")];
  for (var ix = 0; ix < dataComponents.length; ++ix)
  {
    data[dataComponents[ix]].units = theUnits;
    data[dataComponents[ix]].size = sizeBoxes[ix].value;
  }

  var editorElement = msiGetParentEditorElementForDialog(window);
  var theWindow = window.opener;
  if (!theWindow || !("msiInsertRules" in theWindow))
    theWindow = msiGetTopLevelWindow();
  theWindow.msiInsertRules(data, editorElement);

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