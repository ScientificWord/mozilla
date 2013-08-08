// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

var gBodyElement;

//const defaultMathColor="#000000";
//const cssBackgroundColorStr = "background-color";
//const emptyElementStr=" ";

//const mmlns    = "http://www.w3.org/1998/Math/MathML";
//const xhtmlns  = "http://www.w3.org/1999/xhtml";

//var customMathColor;

var data;
var bStartUpWithCustomControls = false;

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
    theSpaceType = "normalSpace";
  document.getElementById("spacesRadioGroup").value = theSpaceType;
  if (document.getElementById("customControls").getAttribute("collapsed") != "true")
    bStartUpWithCustomControls = true;
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
  customSpaceData.stretchData = {factor : 1.00, fillWith : "fillNothing"};
  customSpaceData.fixedData = {units : "in", size : 0.00};
  if (spaceInfo.customType == "stretchy")
  {
    if (spaceInfo.stretchFactor)
      customSpaceData.stretchData.factor = Number(spaceInfo.stretchFactor);
    if (spaceInfo.fillWith)
    {
      switch(spaceInfo.fillWith)
      {
        case "line": customSpaceData.stretchData.fillWith = "fillLine";      break;
        case "dots": customSpaceData.stretchData.fillWith = "fillDots";      break;
        default:                                                             break;
      }
    }
  }
  else  //"fixed"
  {
    if (("theDim" in spaceInfo) && spaceInfo.theDim.length)
    {
      var theSize = msiCSSUnitsList.getNumberAndUnitFromString(spaceInfo.theDim);
      customSpaceData.fixedData.units = theSize.unit;
      customSpaceData.fixedData.size = theSize.number;
    }
  }
  return customSpaceData;
}

function setUpCustomSpaceData(inData)
{
  var customSpaceData = inData;
  if (customSpaceData == null)
  {
    customSpaceData = new Object();
    customSpaceData.customType = "fixed";
    customSpaceData.typesetChoice = "always";
  }
  else if (customSpaceData.customType == "stretchy")
    customSpaceData.typesetChoice = "always";

  if (!customSpaceData.fixedData || (customSpaceData.fixedData == null))
  {
    customSpaceData.fixedData = new Object();
    customSpaceData.fixedData.units = "in";
    customSpaceData.fixedData.size = 0.00;
  }
  if (!customSpaceData.stretchData || (customSpaceData.stretchData == null))
  {
    customSpaceData.stretchData = new Object();
    customSpaceData.stretchData.factor = 1.00;
    customSpaceData.stretchData.fillWith = "fillNothing";
  }
  return customSpaceData;
}

function changeUnits(listbox)
{
  if (!("mUnitsController" in listbox))
    return;
  if (listbox.selectedItem == undefined || listbox.selectedItem.value == undefined)
    dump("In HorizontalSpaces.js, changeUnits; selectedItem.value is undefined!\n");
  else
    listbox.mUnitsController.changeUnits(listbox.selectedItem.value);
}

function setCustomControls()
{
  if (!("customSpaceData" in data))
    data.customSpaceData = null;
  data.customSpaceData = setUpCustomSpaceData(data.customSpaceData);
  document.getElementById("fixedOrStretchyRadioGroup").value = data.customSpaceData.customType;
  document.getElementById("typesetRadioGroup").value= data.customSpaceData.typesetChoice;
  var unitsListBox = document.getElementById("fixedSizeUnitsbox");
  var unitsControlGroup = [document.getElementById("fixedSizeTextbox")];
//  unitsListBox.mUnitsController = new msiUnitsListbox(unitsListBox, unitsControlGroup);
  var unitsController = new msiUnitsListbox(unitsListBox, unitsControlGroup);
  var unitsValueGroup = [String(data.customSpaceData.fixedData.size) + data.customSpaceData.fixedData.units];
  unitsController.setUp(data.customSpaceData.fixedData.units, unitsValueGroup);
//  if (data.customSpaceData.customType == "fixed")
//  {
//    document.getElementById("fixedSizeUnitsbox").value= data.customSpaceData.fixedData.units;
//    document.getElementById("fixedSizeTextbox").value= String(data.customSpaceData.fixedData.size);
//  }
//  else
//  {
    document.getElementById("stretchySizeFactor").value= data.customSpaceData.stretchData.factor;
    document.getElementById("fillWithRadioGroup").value= data.customSpaceData.stretchData.fillWith;
//  }
//  checkInaccessibleAcceleratorKeys(document.documentElement);

//  gDialog.tabOrderArray = new Array( gDialog.limitsSpecGroup, gDialog.sizeSpecGroup, gDialog.OperatorsGroup,
//                                       document.documentElement.getButton("accept"),
//                                       document.documentElement.getButton("cancel") );
  
//  document.documentElement.getButton("accept").setAttribute("default", true);
}

var typesetControlGroup = ["typesetRadioGroup", "typesetDiscardAtLineEnd", "typesetAlways"];
//var fillWithControlGroup = [

function checkShowCustomControls()
{
  if (bStartUpWithCustomControls)
  {
    bStartUpWithCustomControls = false;
    return true;
  }
  return (data.spaceType == "customSpace" && data.customSpaceData && (data.customSpaceData != null));
}

function checkEnableControls()
{
  data.spaceType = document.getElementById("spacesRadioGroup").value;
  var customControlGroup = document.getElementById("customControls");
  if (checkShowCustomControls())
  {
    if (customControlGroup.hasAttribute("collapsed"))
    {
      customControlGroup.removeAttribute("collapsed");
      //window.sizeToContent();
    }
    data.customSpaceData.customType = document.getElementById("fixedOrStretchyRadioGroup").value;
    if (data.customSpaceData.customType == "stretchy")
    {
      document.getElementById("fixedSizeControls").setAttribute("collapsed", "true");
      document.getElementById("stretchySizeControls").removeAttribute("collapsed");
      enableControlsByID(typesetControlGroup, false);
      document.getElementById("fillWithGroupbox").removeAttribute("hidden");
    }
    else
    {
      document.getElementById("fixedSizeControls").removeAttribute("collapsed");
      document.getElementById("stretchySizeControls").setAttribute("collapsed", "true");
      enableControlsByID(typesetControlGroup, true);
      document.getElementById("fillWithGroupbox").setAttribute("hidden", "true");
    }
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
//  var oldUnits = data.customSpaceData.fixedData.units;
//  data.customSpaceData.fixedData.units = document.getElementById("fixedSizeUnitsbox").value;
//  data.customSpaceData.fixedData.size = msiConvertUnits(oldVal.getValue(), oldUnits, data.customSpaceData.fixedData.units);
//  document.getElementById("fixedSizeTextbox").value = String(data.customSpaceData.fixedData.size);
//}

function onAccept()
{
  data.spaceType = document.getElementById("spacesRadioGroup").value;
  if (data.spaceType == "customSpace")
  {
    data.customSpaceData.customType = document.getElementById("fixedOrStretchyRadioGroup").value;
    if (data.customSpaceData.customType == "fixed")
    {
      data.customSpaceData.fixedData.units = document.getElementById("fixedSizeUnitsbox").value;
      data.customSpaceData.fixedData.size = document.getElementById("fixedSizeTextbox").value;
      data.customSpaceData.typesetChoice = document.getElementById("typesetRadioGroup").value;
    }
    else
    {
      data.customSpaceData.stretchData.factor = document.getElementById("stretchySizeFactor").value;
      data.customSpaceData.stretchData.fillWith = document.getElementById("fillWithRadioGroup").value;
      data.customSpaceData.typesetChoice = "always";
    }
  }

  var editorElement = msiGetParentEditorElementForDialog(window);
  var theWindow = window.opener;
  if (isReviseDialog())
  {
    if (!theWindow || !("msiReviseHorizontalSpace" in theWindow))
      theWindow = msiGetTopLevelWindow();
    theWindow.msiReviseHorizontalSpace(data.reviseData, data, editorElement);
  }
  else
  {
    if (!theWindow || !("msiInsertHorizontalSpace" in theWindow))
      theWindow = msiGetTopLevelWindow();
    theWindow.msiInsertHorizontalSpace(data, editorElement);
  }

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