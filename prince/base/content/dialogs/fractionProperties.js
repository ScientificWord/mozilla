// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

Components.utils.import("resource://app/modules/msiEditorDefinitions.jsm");

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
  if (!editor)
  {
    window.close();
    return;
  }

  doSetOKCancel(onAccept, onCancel);
  data = window.arguments[0];
  data.Cancel = false;

  if ( (data == null) || !("reviseObject" in data) || (data.reviseObject == null) )
  {
    window.close();
    return false;
  }
  var reviseData = setDataValuesFromPropertiesObject(data.reviseObject);
  if (reviseData != null)
  {
    data.lineSpec = reviseData.lineSpec;
    data.sizeSpec = reviseData.sizeSpec;
  }

  gDialog.lineSpec = data.lineSpec;
  gDialog.sizeSpec = data.sizeSpec;

  gDialog.lineSpecGroup = document.getElementById("lineRadioGroup");
  gDialog.sizeSpecGroup = document.getElementById("sizeRadioGroup");
  gDialog.fractionPreview = document.getElementById("FractionPreview");

//Probably need to restore this for drawing the sample.
//  try {
//    gBodyElement = editor.rootElement;
//  } catch (e) {}
//
//  if (!gBodyElement) {
//    dump("Failed to get BODY element!\n");
//    window.close();
//  }   
//  // Set element we will edit
//  globalElement = gBodyElement.cloneNode(false);

  InitDialog();

//  window.mMSIDlgManager = new msiDialogConfigManager(window);
//  window.mMSIDlgManager.configureDialog();

//  gDialog.lineSpecGroup.focus();
  msiSetInitialDialogFocus(gDialog.lineSpecGroup);

  SetWindowLocation();
}


function setDataValuesFromPropertiesObject(reviseObject)
{
  var retVal = null;
  var theName = msiGetBaseNodeName(reviseObject);
  switch(theName)
  {
    case "mfrac":
      retVal = new Object();
      if (reviseObject.hasAttribute("linethickness"))
        retVal.lineSpec = reviseObject.getAttribute("linethickness");
      else
        retVal.lineSpec = "";
      retVal.sizeSpec = "";
    break;

    case "mstyle":
    case "mrow":
    {
      var nodeContents = msiNavigationUtils.getSignificantContents(reviseObject);
      if (nodeContents.length == 1)
        retVal = setDataValuesFromPropertiesObject(nodeContents[0]);
      if (retVal != null && theName == "mstyle" && reviseObject.hasAttribute("displaystyle"))
      {
        if (reviseObject.getAttribute("displaystyle") == "false")
          retVal.sizeSpec = "small";
        else
          retVal.sizeSpec = "big";
      }
    }
    break;

    default:
    break;
  }

  return retVal;
}

function InitDialog()
{
  var lineSpecItem = gDialog.lineSpecGroup.getElementsByAttribute("value", gDialog.lineSpec);
  if (lineSpecItem.length > 0)
    gDialog.lineSpecGroup.selectedItem = lineSpecItem[0];
  var sizeSpecItem = gDialog.sizeSpecGroup.getElementsByAttribute("value", gDialog.sizeSpec);
  if (sizeSpecItem.length > 0)
    gDialog.sizeSpecGroup.selectedItem = sizeSpecItem[0];

  gDialog.tabOrderArray = new Array( gDialog.lineSpecGroup, gDialog.sizeSpecGroup,
                                       document.documentElement.getButton("accept"),
                                       document.documentElement.getButton("cancel") );

  checkInaccessibleAcceleratorKeys(document.documentElement);
  
  document.documentElement.getButton("accept").setAttribute("default", true);

  var sampleDependencies = new Array(gDialog.lineSpecGroup, gDialog.sizeSpecGroup);
  makeSampleWindowDependOn(gDialog.fractionPreview, sampleDependencies);

//  //query (this) document for colors
//  var tmp = GetHTMLOrCSSStyleValue(globalElement, mathStr, cssColorStr);
//  if (tmp) {
//    defaultMathColor = ConvertRGBColorIntoHEXColor(tmp);
//  }
//  customMathColor = data.mathColor;
////  customMathnameColor = data.mathnameColor;
////  customUnitColor = data.unitColor;
////  customMtextColor = data.mtextColor;
////  customMatrixColor = data.matrixColor;
//
//
//  if (!customMathColor) {
//    customMathColor = defaultMathColor;
//    UseDefaultColors("mathCW");
//    gDialog.MathColorGroup.selectedIndex = 0;
//  } else {
//    UseCustomColors("mathCW");
//    gDialog.MathColorGroup.selectedIndex = 1;
//  }
//  
//  tmp  = GetHTMLOrCSSStyleValue(globalElement, bgcolorStr, cssBackgroundColorStr);
//  customBackgroundColor  = ConvertRGBColorIntoHEXColor(tmp);
//  if (!customBackgroundColor)
//    customBackgroundColor = defaultBackgroundColor;
//  var styleValue = backColorStyle+customBackgroundColor+";";
//  gDialog.BinomialPreview.setAttribute(styleStr, styleValue);

  drawSample(gDialog.fractionPreview);
}

function drawSample(sampleControl)
{
  var style = document.getElementById("fractionSampleMStyle");
  if (gDialog.sizeSpec == "big")
  {
    style.setAttribute("displaystyle", "true");
    style.setAttribute("scriptlevel", "0");
  }
  else if (gDialog.sizeSpec == "small")
  {
    style.setAttribute("displaystyle", "false");
    style.setAttribute("scriptlevel", "0");
  }
  else
  {
    style.removeAttribute("displaystyle");
    style.removeAttribute("scriptlevel"); 
  }
  if (gDialog.lineSpec == "thick")
    document.getElementById("fractionSampleMFrac").setAttribute("linethickness", "thick");
  else if (gDialog.lineSpec == "0")
    document.getElementById("fractionSampleMFrac").setAttribute("linethickness", "0");
  else
    document.getElementById("fractionSampleMFrac").removeAttribute("linethickness");
}

function setLineThickness(lineSpec)
{
  gDialog.lineSpec = lineSpec.value;
}

function setFractionSize(sizeSpec)
{
  gDialog.sizeSpec = sizeSpec.value;
}

function onAccept()
{
  data.lineSpec = gDialog.lineSpec;
  data.sizeSpec = gDialog.sizeSpec;

  var editorElement = msiGetParentEditorElementForDialog(window);
  var editor = msiGetEditor(editorElement);
  var theWindow = window.opener;
  if (!theWindow || !("reviseFraction" in theWindow))
    theWindow = msiGetTopLevelWindow();

  theWindow.reviseFraction(data.reviseObject, data.lineSpec, data.sizeSpec, editorElement);

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