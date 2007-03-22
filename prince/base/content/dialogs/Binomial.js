// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

var gBodyElement;

const defaultMathColor="#000000";
const cssBackgroundColorStr = "background-color";
const emptyElementStr=" ";
//const colorStyle = cssColorStr + ": ";

const mmlns    = "http://www.w3.org/1998/Math/MathML";
const xhtmlns  = "http://www.w3.org/1999/xhtml";

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

  gDialog.withDelimiters = data.withDelimiters;
  gDialog.lineSpec = data.lineSpec;
  gDialog.sizeSpec = data.sizeSpec;

  gDialog.LeftBracketGroup = document.getElementById("leftBracketGroup");
  gDialog.RightBracketGroup = document.getElementById("rightBracketGroup");
  gDialog.LeftBracketGroup.valueStr = data.leftBracket;
  gDialog.RightBracketGroup.valueStr = data.rightBracket;
  if (!gDialog.LeftBracketGroup.valueStr.length)
    gDialog.LeftBracketGroup.valueStr = "(";
  if (!gDialog.RightBracketGroup.valueStr.length)
    gDialog.RightBracketGroup.valueStr = ")";

  gDialog.lineSpecGroup = document.getElementById("lineRadioGroup");
  gDialog.sizeSpecGroup = document.getElementById("sizeRadioGroup");
  gDialog.withDelimsCheckbox = document.getElementById("DelimitersCheckBox");
  gDialog.BinomialPreview = document.getElementById("BinomialPreview");

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

  gDialog.lineSpecGroup.focus();

  SetWindowLocation();
}

function InitDialog()
{
  makeMSIButtonGroup(gDialog.LeftBracketGroup, true, gDialog.RightBracketGroup.id);
  makeMSIButtonGroup(gDialog.RightBracketGroup, true);
  var nWhichSel = -1;
  nWhichSel = setSelectionByValue(gDialog.LeftBracketGroup, gDialog.LeftBracketGroup.valueStr);
  if (nWhichSel < 0)
    setSelectionByIndex(0, gDialog.LeftBracketGroup.id);
  nWhichSel = -1;
  nWhichSel = setSelectionByValue(gDialog.RightBracketGroup, gDialog.RightBracketGroup.valueStr);
  if (nWhichSel < 0)
    setSelectionByIndex(0, gDialog.RightBracketGroup.id);

  var lineSpecItem = gDialog.lineSpecGroup.getElementsByAttribute("value", gDialog.lineSpec);
  if (lineSpecItem.length > 0)
    gDialog.lineSpecGroup.selectedItem = lineSpecItem[0];
  var sizeSpecItem = gDialog.sizeSpecGroup.getElementsByAttribute("value", gDialog.sizeSpec);
  if (sizeSpecItem.length > 0)
    gDialog.sizeSpecGroup.selectedItem = sizeSpecItem[0];
  gDialog.withDelimsCheckbox.checked = gDialog.withDelimiters;

  gDialog.tabOrderArray = new Array( gDialog.lineSpecGroup, gDialog.sizeSpecGroup, gDialog.withDelimsCheckbox,
                                       gDialog.LeftBracketGroup, gDialog.RightBracketGroup,
                                       document.documentElement.getButton("accept"),
                                       document.documentElement.getButton("cancel") );

  checkInaccessibleAcceleratorKeys(document.documentElement);
  
  document.documentElement.getButton("accept").setAttribute("default", true);

  var sampleDependencies = new Array(gDialog.LeftBracketGroup, gDialog.RightBracketGroup, gDialog.lineSpecGroup,
                                       gDialog.sizeSpecGroup, gDialog.withDelimsCheckbox);
  makeSampleWindowDependOn(gDialog.BinomialPreview, sampleDependencies);

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

  drawSample(gDialog.BinomialPreview);
}

function drawSample(sampleControl)
{
  if (gDialog.withDelimiters)
  {
    document.getElementById("binomialSampleLeftFence").firstChild.nodeValue = gDialog.LeftBracketGroup.valueStr;
    document.getElementById("binomialSampleRightFence").firstChild.nodeValue = gDialog.RightBracketGroup.valueStr;
  }
  else
  {
    document.getElementById("binomialSampleLeftFence").textContent = "";
    document.getElementById("binomialSampleRightFence").textContent = "";
  }
  var style = document.getElementById("binomialSampleMStyle");
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
    document.getElementById("binomialSampleMFrac").setAttribute("linethickness", "thick");
  else if (gDialog.lineSpec == "0")
    document.getElementById("binomialSampleMFrac").setAttribute("linethickness", "0");
  else
    document.getElementById("binomialSampleMFrac").removeAttribute("linethickness");
}

function setLineThickness(lineSpec)
{
  gDialog.lineSpec = lineSpec.value;
}

function setBinomialSize(sizeSpec)
{
  gDialog.sizeSpec = sizeSpec.value;
}

function turnDelimitersOnOff(event)
{
  if (event.originalTarget.checked == true)
    document.getElementById("enableBrackets").setAttribute("disabled", "false");
  else
    document.getElementById("enableBrackets").setAttribute("disabled", "true");
  gDialog.withDelimiters = event.originalTarget.checked;
}

function onAccept()
{
  data.leftBracket = gDialog.LeftBracketGroup.valueStr;
  data.rightBracket = gDialog.RightBracketGroup.valueStr;
  data.withDelimiters = gDialog.withDelimiters;
  data.lineSpec = gDialog.lineSpec;
  data.sizeSpec = gDialog.sizeSpec;

  if (!data.withDelimiters)
  {
    data.leftBracket = "";
    data.rightBracket = "";
  }

  var editorElement = msiGetParentEditorElementForDialog(window);
  var editor = msiGetEditor(editorElement);
  var theWindow = window.opener;
  if (!theWindow || !("insertBinomial" in theWindow))
    theWindow = msiGetTopLevelWindow();

  theWindow.insertBinomial(data.leftBracket, data.rightBracket, data.lineSpec, data.sizeSpec, editorElement);

  SaveWindowLocation();
  return false;
}

function onCancel()
{
  data.Cancel = true;
}

function doAccept()
{
  document.documentElement.getButton('accept').oncommand();
}