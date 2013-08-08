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
  data = window.arguments[0];
  if (data == null)
    data = new Object();
  if ("reviseData" in data)
    setDataFromReviseData(data.reviseData);
  data.Cancel = false;

  InitDialog();

  window.mMSIDlgManager = new msiDialogConfigManager(window);
  window.mMSIDlgManager.configureDialog();

//  document.getElementById("breaksRadioGroup").focus();
  msiSetInitialDialogFocus(document.getElementById("breaksRadioGroup"));

  SetWindowLocation();
}

function InitDialog()
{
  var theBreakType = data.breakType;
  if (!data.breakType || !data.breakType.length)
    theBreakType = "allowBreak";
  document.getElementById("breaksRadioGroup").value = theBreakType;

  setCustomControls();
  checkEnableControls();
}

function isReviseDialog()
{
  if (data && ("reviseData" in data) && (data.reviseData!=null))
    return true;
  return false;
}

function setDataFromReviseData(reviseData)
{
  var revBreakInfo = reviseData.getSpaceInfo();
  if (revBreakInfo)
  {
    data.breakType = revBreakInfo.theSpace;
    if (revBreakInfo.theSpace == "customNewLine")
      data.customBreakData = setCustomBreakDataFromReviseBreakData(revBreakInfo);
  }
}

function setCustomBreakDataFromReviseBreakData(breakInfo)
{
  var customBreakData = {typesetChoice : "always"};
  if (("atEnd" in breakInfo) && breakInfo.atEnd == "false")
    customBreakData.typesetChoice = "discardAtLineEnd";
  customBreakData.sizeData = {units : "in", size : 0.00};
  if (("theDim" in breakInfo) && breakInfo.theDim.length)
  {
    var theSize = msiCSSUnitsList.getNumberAndUnitFromString(breakInfo.theDim);
    customBreakData.sizeData.units = theSize.unit;
    customBreakData.sizeData.size = theSize.number;
  }
  return customBreakData;
}

function setUpCustomBreakData(inData)
{
  var customBreakData = inData;
  if (customBreakData == null)
  {
    customBreakData = new Object();
    customBreakData.typesetChoice = "always";
  }

  if (!customBreakData.sizeData || (customBreakData.sizeData == null))
  {
    customBreakData.sizeData = new Object();
    customBreakData.sizeData.units = "in";
    customBreakData.sizeData.size = 0.00;
  }
  return customBreakData;
}

function changeUnits(listbox)
{
  if (!("mUnitsController" in listbox))
    return;
  if (listbox.selectedItem == undefined || listbox.selectedItem.value == undefined)
    dump("In msiBreaksDialog.js, changeUnits; selectedItem.value is undefined!\n");
  else
    listbox.mUnitsController.changeUnits(listbox.selectedItem.value);
}

function setCustomControls()
{
  if (!("customBreakData" in data))
    data.customBreakData = null;
  data.customBreakData = setUpCustomBreakData(data.customBreakData);
  document.getElementById("typesetRadioGroup").value= data.customBreakData.typesetChoice;
  var unitsListBox = document.getElementById("sizeUnitsbox");
  var unitsControlGroup = [document.getElementById("depthSizeTextbox")];
  var unitsController = new msiUnitsListbox(unitsListBox, unitsControlGroup);
//  unitsListBox.mUnitsController = new msiUnitsListbox(unitsListBox, unitsControlGroup);
  var unitsValueGroup = [String(data.customBreakData.sizeData.size) + data.customBreakData.sizeData.units];
  unitsController.setUp(data.customBreakData.sizeData.units, unitsValueGroup);
//  checkInaccessibleAcceleratorKeys(document.documentElement);

//  gDialog.tabOrderArray = new Array( gDialog.limitsSpecGroup, gDialog.sizeSpecGroup, gDialog.OperatorsGroup,
//                                       document.documentElement.getButton("accept"),
//                                       document.documentElement.getButton("cancel") );
  
//  document.documentElement.getButton("accept").setAttribute("default", true);
}

//var typesetControlGroup = ["typesetRadioGroup", "typesetDiscardAtLineEnd", "typesetAlways"];
//var fillWithControlGroup = [

function checkEnableControls()
{
  data.breakType = document.getElementById("breaksRadioGroup").value;
  var customControlGroup = document.getElementById("customControls");
  if (data.breakType == "customNewLine" && data.customBreakData && (data.customBreakData != null))
  {
    if (customControlGroup.hasAttribute("collapsed"))
    {
      customControlGroup.removeAttribute("collapsed");
      //window.sizeToContent();
    }
//    document.getElementById("fixedSizeControls").removeAttribute("collapsed");
//    enableControlsByID(typesetControlGroup, true);
  }
  else
  {
    if ( !(customControlGroup.hasAttribute("collapsed")) || (customControlGroup.getAttribute("collapsed") == "false") )
    {
      customControlGroup.setAttribute("collapsed", "true");
      //window.sizeToContent();
    }
  }
}

////This is only called for a fixed-width custom space; otherwise the units box should be invisible.
//function changeUnits()
//{
//  var oldVal = document.getElementById("fixedSizeTextbox").value;
//  var oldUnits = data.customSpaceData.sizeData.units;
//  data.customSpaceData.sizeData.units = document.getElementById("fixedSizeUnitsbox").value;
//  data.customSpaceData.sizeData.size = msiConvertUnits(oldVal.getValue(), oldUnits, data.customSpaceData.sizeData.units);
//  document.getElementById("fixedSizeTextbox").value = String(data.customSpaceData.sizeData.size);
//}

function onAccept()
{
  data.breakType = document.getElementById("breaksRadioGroup").value;
  if (data.breakType == "customNewLine")
  {
    data.customBreakData.sizeData.units = document.getElementById("sizeUnitsbox").value;
    data.customBreakData.sizeData.size = document.getElementById("depthSizeTextbox").value;
    data.customBreakData.typesetChoice = document.getElementById("typesetRadioGroup").value;
  }

  var editorElement = msiGetParentEditorElementForDialog(window);
  var theWindow = window.opener;
  if (isReviseDialog())
  {
    if (!theWindow || !("msiReviseBreaks" in theWindow))
      theWindow = msiGetTopLevelWindow();
    theWindow.msiReviseBreaks(data.reviseData, data, editorElement);
  }
  else
  {
    if (!theWindow || !("msiInsertBreaks" in theWindow))
      theWindow = msiGetTopLevelWindow();
    theWindow.msiInsertBreaks(data, editorElement);
  }

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