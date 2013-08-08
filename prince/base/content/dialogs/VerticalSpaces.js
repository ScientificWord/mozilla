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
  if (!data)
    data = new Object();
  if ("reviseData" in data)
    setDataFromReviseData(data.reviseData);
  data.Cancel = false;

  InitDialog();

  window.mMSIDlgManager = new msiDialogConfigManager(window);
  window.mMSIDlgManager.configureDialog();

//  document.getElementById("spacesRadioGroup").focus();
  msiSetInitialDialogFocus(document.getElementById("spacesRadioGroup"));

  SetWindowLocation();
}

function InitDialog()
{
  var theSpaceType = data.spaceType;
  if (!data.spaceType || !data.spaceType.length)
    theSpaceType = "smallSkip";
  document.getElementById("spacesRadioGroup").value = theSpaceType;

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
  var revSpaceInfo = reviseData.getSpaceInfo();
  if (revSpaceInfo)
  {
    data.spaceType = revSpaceInfo.theSpace;
    if (revSpaceInfo.theSpace == "customSpace")
      data.customSpaceData = setCustomSpaceDataFromReviseSpaceData(revSpaceInfo);
  }
}

function setCustomSpaceDataFromReviseSpaceData(spaceInfo)
{
  var customSpaceData = {customType : spaceInfo.customType, typesetChoice : "always"};
  if (("atEnd" in spaceInfo) && spaceInfo.atEnd == "false")
    customSpaceData.typesetChoice = "discardAtLineEnd";
//  customSpaceData.stretchData = {factor : 1.00, fillWith : "fillNothing"};
  customSpaceData.sizeData = {units : "in", size : 0.00};
//  if (spaceInfo.customType == "stretchy")
//  {
//    if (spaceInfo.stretchFactor)
//      customSpaceData.stretchData.factor = Number(spaceInfo.stretchFactor);
//  }
//  else  //"fixed"
//  {
    if (("theDim" in spaceInfo) && spaceInfo.theDim.length)
    {
      var theSize = msiCSSUnitsList.getNumberAndUnitFromString(spaceInfo.theDim);
      customSpaceData.sizeData.units = theSize.unit;
      customSpaceData.sizeData.size = theSize.number;
    }
//  }
  return customSpaceData;
}

function setUpCustomSpaceData(inData)
{
  var customSpaceData = inData;
  if (customSpaceData == null)
  {
    customSpaceData = new Object();
    customSpaceData.typesetChoice = "always";
  }

  if (!customSpaceData.sizeData || (customSpaceData.sizeData == null))
  {
    customSpaceData.sizeData = new Object();
    customSpaceData.sizeData.units = "in";
    customSpaceData.sizeData.size = 0.00;
  }
  return customSpaceData;
}

function changeUnits(listbox)
{
  if (!("mUnitsController" in listbox))
    return;
  if (listbox.selectedItem == undefined || listbox.selectedItem.value == undefined)
    dump("In VerticalSpaces.js, changeUnits; selectedItem.value is undefined!\n");
  else
    listbox.mUnitsController.changeUnits(listbox.selectedItem.value);
}

function setCustomControls()
{
  if (!("customSpaceData" in data))
    data.customSpaceData = null;
  data.customSpaceData = setUpCustomSpaceData(data.customSpaceData);
  document.getElementById("typesetRadioGroup").value= data.customSpaceData.typesetChoice;
  var unitsListBox = document.getElementById("fixedSizeUnitsbox");
  var unitsControlGroup = [document.getElementById("fixedSizeTextbox")];
//  unitsListBox.mUnitsController = new msiUnitsListbox(unitsListBox, unitsControlGroup);
  var unitsController = new msiUnitsListbox(unitsListBox, unitsControlGroup);
  var unitsValueGroup = [String(data.customSpaceData.sizeData.size) + data.customSpaceData.sizeData.units];
  unitsController.setUp(data.customSpaceData.sizeData.units, unitsValueGroup);
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
  data.spaceType = document.getElementById("spacesRadioGroup").value;
  var customControlGroup = document.getElementById("customControls");
  if (data.spaceType == "customSpace" && data.customSpaceData && (data.customSpaceData != null))
  {
    if (customControlGroup.hasAttribute("collapsed"))
    {
      customControlGroup.removeAttribute("collapsed");
      //window.sizeToContent();
    }
    document.getElementById("fixedSizeControls").removeAttribute("collapsed");
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
  data.spaceType = document.getElementById("spacesRadioGroup").value;
  dump("In VerticalSpaces onAccept.\n");
  if (data.spaceType == "customSpace")
  {
    data.customSpaceData.sizeData.units = document.getElementById("fixedSizeUnitsbox").value;
    data.customSpaceData.sizeData.size = document.getElementById("fixedSizeTextbox").value;
    data.customSpaceData.typesetChoice = document.getElementById("typesetRadioGroup").value;
  }

  dump("In VerticalSpaces onAccept 2.\n");
  var editorElement = msiGetParentEditorElementForDialog(window);
  var theWindow = window.opener;

  if (isReviseDialog())
  {
    if (!theWindow || !("msiReviseVerticalSpace" in theWindow))
      theWindow = msiGetTopLevelWindow();
    theWindow.msiReviseVerticalSpace(data.reviseData, data, editorElement);
  }
  else
  {
    dump("In VerticalSpaces onAccept 3.\n");
    if (!theWindow || !("msiInsertVerticalSpace" in theWindow))
      theWindow = msiGetTopLevelWindow();
    theWindow.msiInsertVerticalSpace(data, editorElement);
    dump("In VerticalSpaces onAccept 4.\n");
  }

  SaveWindowLocation();
//  return false;
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