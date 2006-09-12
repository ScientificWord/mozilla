// Copyright (c) 2004 MacKichan Software, Inc.  All Rights Reserved.

var gBodyElement;

const defaultMathColor="#000000";
const defaultMathnameColor="#000000";
const defaultUnitColor="#000000";
const defaultMtextColor="#000000";
const defaultMatrixColor="";
const defaultBackgroundColor="#FFFFFF";
const styleStr = "style";
const mathStr = "math";
const bgcolorStr = "bgcolor";
const backgroundStr = "background";
const cssColorStr = "color";
const cssBackgroundColorStr = "background-color";
const colorStyle = cssColorStr + ": ";
const backColorStyle = cssBackgroundColorStr + ": ";

var customMathColor;
var customMathnameColor;
var customUnitColor;
var customMtextColor;
var customMatrixColor;

var data;

// dialog initialization code
function Startup()
{
  var editor = GetCurrentEditor();
  if (!editor) {
    window.close();
    return;
  }

  data = window.arguments[0];
  data.Cancel = false;

  gDialog.ColorPreview = document.getElementById("ColorPreview");
  gDialog.MathText = document.getElementById("MathText");
  gDialog.MathnameText = document.getElementById("MathnameText");
  gDialog.UnitText = document.getElementById("UnitText");
  gDialog.MtextText = document.getElementById("MtextText");
  gDialog.MatrixLines = document.getElementById("MatrixLines");
  gDialog.MathColorGroup = document.getElementById("MathColorGroup");
  gDialog.MathnameColorGroup = document.getElementById("MathnameColorGroup");
  gDialog.UnitColorGroup = document.getElementById("UnitColorGroup");
  gDialog.MtextColorGroup = document.getElementById("MtextColorGroup");
  gDialog.MatrixColorGroup = document.getElementById("MatrixColorGroup");

  try {
    gBodyElement = editor.rootElement;
  } catch (e) {}

  if (!gBodyElement) {
    dump("Failed to get BODY element!\n");
    window.close();
  }

  // Set element we will edit
  globalElement = gBodyElement.cloneNode(false);

  InitDialog();

  gDialog.MathColorGroup.focus();

  SetWindowLocation();
}

function InitDialog()
{
  //query (this) document for colors
  var tmp = GetHTMLOrCSSStyleValue(globalElement, mathStr, cssColorStr);
  if (tmp) {
    defaultMathColor = ConvertRGBColorIntoHEXColor(tmp);
    // attribute based
    // customMathnameColor        = globalElement.getAttribute(linkStr);
    // customUnitColor      = globalElement.getAttribute(alinkStr);
    // customMatrixColor     = globalElement.getAttribute(vlinkStr);
    defaultMathnameColor = defaultMathColor;
    defaultUnitColor = defaultMathColor;
    defaultMtextColor = defaultMathColor;
  }
  customMathColor = data.mathColor;
  customMathnameColor = data.mathnameColor;
  customUnitColor = data.unitColor;
  customMtextColor = data.mtextColor;
  customMatrixColor = data.matrixColor;


  if (!customMathColor) {
    customMathColor = defaultMathColor;
    UseDefaultColors("mathCW");
    gDialog.MathColorGroup.selectedIndex = 0;
  } else {
    UseCustomColors("mathCW");
    gDialog.MathColorGroup.selectedIndex = 1;
  }
  if (!customMathnameColor) {
    customMathnameColor = defaultMathnameColor;
    UseDefaultColors("mathnameCW");
    gDialog.MathnameColorGroup.selectedIndex = 0;
  } else {
    UseCustomColors("mathnameCW");
    gDialog.MathnameColorGroup.selectedIndex = 1;
  }
  if (!customUnitColor) {
    customUnitColor = defaultUnitColor;
    UseDefaultColors("unitCW");
    gDialog.UnitColorGroup.selectedIndex = 0;
  } else {
    UseCustomColors("unitCW");
    gDialog.UnitColorGroup.selectedIndex = 1;
  }
  if (!customMtextColor) {
    customMtextColor = defaultMtextColor;
    UseDefaultColors("mtextCW");
    gDialog.MtextColorGroup.selectedIndex = 0;
  } else {
    UseCustomColors("mtextCW");
    gDialog.MtextColorGroup.selectedIndex = 1;
  }
  if (!customMatrixColor) {
    customMatrixColor = defaultMatrixColor;
    UseDefaultColors("matrixCW");
    gDialog.MatrixColorGroup.selectedIndex = 0;
  } else {
    UseCustomColors("matrixCW");
    gDialog.MatrixColorGroup.selectedIndex = 1;
  }

  tmp  = GetHTMLOrCSSStyleValue(globalElement, bgcolorStr, cssBackgroundColorStr);
  customBackgroundColor  = ConvertRGBColorIntoHEXColor(tmp);
  if (!customBackgroundColor)
    customBackgroundColor = defaultBackgroundColor;
  var styleValue = backColorStyle+customBackgroundColor+";";
  gDialog.ColorPreview.setAttribute(styleStr, styleValue);
}

function GetColorAndUpdate(ColorWellID)
{
  var colorWell = document.getElementById(ColorWellID);
  if (!colorWell) return;

  // Don't allow a blank color, i.e., using the "default"
  var colorObj = { NoDefault:true, Type:"", TextColor:0, PageColor:0, Cancel:false };

  switch( ColorWellID )
  {
    case "mathCW":
      colorObj.Type = "Math";
      colorObj.TextColor = customMathColor;
      break;
    case "mathnameCW":
      colorObj.Type = "Mathname";
      colorObj.TextColor = customMathnameColor;
      break;
    case "unitCW":
      colorObj.Type = "Unit";
      colorObj.TextColor = customUnitColor;
      break;
    case "mtextCW":
      colorObj.Type = "Mtext";
      colorObj.TextColor = customMtextColor;
      break;
    case "matrixCW":
      colorObj.Type = "Matrix";
      colorObj.TextColor = customMatrixColor;
      break;
  }

  window.openDialog("chrome://editor/content/EdColorPicker.xul", "_blank", "chrome,close,titlebar,modal", "", colorObj);

  // User canceled the dialog
  if (colorObj.Cancel)
    return;

  var color = "";
  switch( ColorWellID )
  {
    case "mathCW":
      color = customMathColor = colorObj.TextColor;
      break;
    case "mathnameCW":
      color = customMathnameColor = colorObj.TextColor;
      break;
    case "unitCW":
      color = customUnitColor = colorObj.TextColor;
      break;
    case "mtextCW":
      color = customMtextColor = colorObj.TextColor;
      break;
    case "matrixCW":
      color = customMatrixColor = colorObj.TextColor;
      break;
  }

  setColorWell(ColorWellID, color); 
  SetColorPreview(ColorWellID, color);
}

function SetColorPreview(ColorWellID, color)
{
  var attrStr;
  if (color.length > 0) 
    attrStr = colorStyle+color
  else
    attrStr = "";
  switch( ColorWellID )
  {
    case "mathCW":
      gDialog.MathText.setAttribute(styleStr,attrStr);
      break;
    case "mathnameCW":
      gDialog.MathnameText.setAttribute(styleStr,attrStr);
      break;
    case "unitCW":
      gDialog.UnitText.setAttribute(styleStr,attrStr);
      break;
    case "mtextCW":
      gDialog.MtextText.setAttribute(styleStr,attrStr);
      break;
    case "matrixCW":
      gDialog.MatrixLines.setAttribute(styleStr,attrStr);
      break;
  }
}

function UseCustomColors(ColorWellID)
{
  switch (ColorWellID) {
    case "mathCW":
      SetElementEnabledById("MathButton", true);
      SetElementEnabledById("Math", true);
      SetColorPreview("mathCW", customMathColor);
      setColorWell("mathCW", customMathColor);
      break;
    case "mathnameCW":
      SetElementEnabledById("MathnameButton", true);
      SetElementEnabledById("Mathname", true);
      SetColorPreview("mathnameCW", customMathnameColor);
      setColorWell("mathnameCW",    customMathnameColor);
      break;
    case "unitCW":
      SetElementEnabledById("UnitButton", true);
      SetElementEnabledById("Unit", true);
      SetColorPreview("unitCW",     customUnitColor);
      setColorWell("unitCW",        customUnitColor);
      break;
    case "mtextCW":
      SetElementEnabledById("MtextButton", true);
      SetElementEnabledById("Mtext", true);
      SetColorPreview("mtextCW",     customMtextColor);
      setColorWell("mtextCW",        customMtextColor);
      break;
    case "matrixCW":
      SetElementEnabledById("MatrixButton", true);
      SetElementEnabledById("Matrix", true);
      SetColorPreview("matrixCW",    customMatrixColor);
      setColorWell("matrixCW",       customMatrixColor);
      break;
  }
}

function UseDefaultColors(ColorWellID)
{
  switch (ColorWellID) {
    case "mathCW":
      SetElementEnabledById("MathButton", false);
      SetElementEnabledById("Math", false);
      SetColorPreview("mathCW", defaultMathColor);
      setColorWell("mathCW", "");
      break;
    case "mathnameCW":
      SetElementEnabledById("MathnameButton", false);
      SetElementEnabledById("Mathname", false);
      SetColorPreview("mathnameCW", defaultMathnameColor);
      setColorWell("mathnameCW", "");
      break;
    case "unitCW":
      SetElementEnabledById("UnitButton", false);
      SetElementEnabledById("Unit", false);
      SetColorPreview("unitCW", defaultUnitColor);
      setColorWell("unitCW", "");
      break;
    case "mtextCW":
      SetElementEnabledById("MtextButton", false);
      SetElementEnabledById("Mtext", false);
      SetColorPreview("mtextCW", defaultMtextColor);
      setColorWell("mtextCW", "");
      break;
    case "matrixCW":
      SetElementEnabledById("MatrixButton", false);
      SetElementEnabledById("Matrix", false);
      SetColorPreview("matrixCW", defaultMatrixColor);
      setColorWell("matrixCW", "");
      break;
  }
}

function onAccept()
{
  if (gDialog.MathColorGroup.selectedIndex == 0) {
    data.mathColor = "";
  } else {
    data.mathColor = customMathColor;
  }
  if (gDialog.MathnameColorGroup.selectedIndex == 0) {
    data.mathnameColor = ""
  } else {
    data.mathnameColor = customMathnameColor;
  }
  if (gDialog.UnitColorGroup.selectedIndex == 0) {
    data.unitColor = ""
  } else {
    data.unitColor = customUnitColor;
  }
  if (gDialog.MtextColorGroup.selectedIndex == 0) {
    data.mtextColor = ""
  } else {
    data.mtextColor = customMtextColor;
  }
  if (gDialog.MatrixColorGroup.selectedIndex == 0) {
    data.matrixColor = ""
  } else {
    data.matrixColor = customMatrixColor;
  }

  SaveWindowLocation();
  return true;
}

function onCancel()
{
  data.Cancel = true;
}
