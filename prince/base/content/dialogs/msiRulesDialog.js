// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

var gBodyElement;

//const defaultMathColor="#000000";
//const cssBackgroundColorStr = "background-color";
//const emptyElementStr=" ";

//const mmlns    = "http://www.w3.org/1998/Math/MathML";
//const xhtmlns  = "http://www.w3.org/1999/xhtml";

//var customMathColor;

var data;
var mUnitsList = null;

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
  var tb = document.getElementById("liftSizeTextbox");
  if (tb.getAttribute("defaultvalue"))
    tb.value = tb.getAttribute("defaultvalue");
  tb = document.getElementById("widthSizeTextbox");
  if (tb.getAttribute("defaultvalue"))
    tb.value = tb.getAttribute("defaultvalue");
  tb = document.getElementById("heightSizeTextbox");
  if (tb.getAttribute("defaultvalue"))
    tb.value = tb.getAttribute("defaultvalue");

  data = window.arguments[0];
  if (!data)
    data = new Object();
  if ("reviseData" in data)
    setDataFromReviseData(data.reviseData);
  data.Cancel = false;

  InitDialog();

  window.mMSIDlgManager = new msiDialogConfigManager(window);
  window.mMSIDlgManager.configureDialog();

//  document.getElementById("liftSizeTextbox").focus();
  msiSetInitialDialogFocus(document.getElementById("liftSizeTextbox"));

  SetWindowLocation();
}

var dataComponents = ["lift", "width", "height"];

function InitDialog()
{
  setUpData();
  setUnitsAndPositionControls();
  setColorWell("colorWell", data.ruleColor);
}

function isReviseDialog()
{
  if (data && ("reviseData" in data) && (data.reviseData!=null))
    return true;
  return false;
}

function setDataFromReviseData(reviseData)
{
  var ruleNode = reviseData.getReferenceNode();
  if (ruleNode)
  {
    mUnitsList = msiCreateCSSUnitsListForElement(ruleNode);

    var valStr = null;
    var theVal = null;
    for (var ix = 0; ix < dataComponents.length; ++ix)
    {
      var valStr = ruleNode.getAttribute(dataComponents[ix]);
      if (valStr && valStr.length)
      {
        theVal = mUnitsList.getNumberAndUnitFromString(valStr);
        data[dataComponents[ix]] = {units: theVal.unit, size: theVal.number};
      }
    }
    valStr = ruleNode.getAttribute("color");
    if (valStr)
      data.ruleColor = msiHTMLNamedColors.colorStringToHexRGBString(valStr);
  }
}

function setUpData()
{
  for (var ix = 0; ix < dataComponents.length; ++ix)
  {
    if (!(dataComponents[ix] in data) || (data[dataComponents[ix]] == null))
    {
      data[dataComponents[ix]] = new Object();
      data[dataComponents[ix]].units = document.getElementById("sizeUnitsbox").value;
      data[dataComponents[ix]].size = (ix==0? document.getElementById("liftSizeTextbox").value : 
        (ix==1 ? document.getElementById("widthSizeTextbox").value : document.getElementById("heightSizeTextbox").value));
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
  var unitsController = new msiUnitsListbox(unitsListBox, unitsControlGroup, mUnitsList);
//  unitsListBox.mUnitsController = new msiUnitsListbox(unitsListBox, unitsControlGroup);
  var unitsValueGroup = new Array(3);
  for (var ix = 0; ix < dataComponents.length; ++ix)
    unitsValueGroup[ix] = String(data[dataComponents[ix]].size) + data[dataComponents[ix]].units;
  unitsController.setUp(data[dataComponents[0]].units, unitsValueGroup);
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

  window.openDialog("chrome://editor/content/EdColorPicker.xul", "colorpicker", "chrome,close,titlebar,modal,resizable", "", colorObj);

  // User canceled the dialog
  if (colorObj.Cancel)
    return;

  data.ruleColor = colorObj.TextColor;
  
  setColorWell("colorWell", data.ruleColor); 
}

function onAccept()
{
  var theUnits = document.getElementById("sizeUnitsbox").value;
  var tb = document.getElementById("liftSizeTextbox");
  tb.setAttribute("defaultvalue",tb.value);
  tb = document.getElementById("widthSizeTextbox");
  tb.setAttribute("defaultvalue",tb.value);
  tb = document.getElementById("heightSizeTextbox");
  tb.setAttribute("defaultvalue",tb.value);
  var sizeBoxes = [document.getElementById("liftSizeTextbox"), document.getElementById("widthSizeTextbox"),
                             document.getElementById("heightSizeTextbox")];
  for (var ix = 0; ix < dataComponents.length; ++ix)
  {
    data[dataComponents[ix]].units = theUnits;
    data[dataComponents[ix]].size = sizeBoxes[ix].value;
  }

  var editorElement = msiGetParentEditorElementForDialog(window);
  var theWindow = window.opener;
  if (isReviseDialog())
  {
    if (!theWindow || !("msiReviseRules" in theWindow))
      theWindow = msiGetTopLevelWindow();
    theWindow.msiReviseRules(data.reviseData, data, editorElement);
  }
  else
  {
    if (!theWindow || !("msiInsertRules" in theWindow))
      theWindow = msiGetTopLevelWindow();
    theWindow.msiInsertRules(data, editorElement);
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