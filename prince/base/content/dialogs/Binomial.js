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
//Data storage: the "data" member enters the dialog carrying information about an object to be revised; otherwise it's empty.
//  When the dialog is accepted, the information to be transferred to the document is contained in the "data" member.
//The data used and set by the controls in the dialog is to be located in "gDialog". The two are kept distinct as the controls
//  can't necessarily reflect the real value of attributes (e.g., "linethickness" could take a dimension, or the opening or closing
//  delimiter may not be among the predefined button set).
//The determination of which value is "current" (i.e., would be passed to the object being inserted or revised) is made by checking
//  whether the control contains a different value from its starting one (kept in "gDialog"). If so, that value is to be used; otherwise,
//  the value from "data" is current.
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

  var bRevise =isReviseDialog();
  if (bRevise)
    setDataFromReviseObject( data.reviseObject );
  else
  {
    ensureDataSet(data);
    gDialog.withDelimiters = data.withDelimiters;
    gDialog.lineSpec = data.lineSpec;
    gDialog.sizeSpec = data.sizeSpec;

    gDialog.leftBracket = data.leftBracket;
    gDialog.rightBracket = data.rightBracket;
  }

  gDialog.LeftBracketGroup = document.getElementById("leftBracketGroup");
  gDialog.RightBracketGroup = document.getElementById("rightBracketGroup");
//  gDialog.LeftBracketGroup.valueStr = gDialog.leftBracket;
//  gDialog.RightBracketGroup.valueStr = gDialog.rightBracket;
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

  window.mMSIDlgManager = new msiDialogConfigManager(window);
  if (bRevise)
  {
    window.mMSIDlgManager.mbIsRevise = true;
    window.mMSIDlgManager.mbCloseOnAccept = true;
  }
  window.mMSIDlgManager.configureDialog();

  gDialog.lineSpecGroup.focus();

  SetWindowLocation();
}

function isReviseDialog()
{
  return ( ("reviseObject" in data) && (data.reviseObject != null) );
}

function setDataFromReviseObject(objectNode)
{
  var wrappedBinomialNode = msiNavigationUtils.getWrappedObject(objectNode, "binomial");
  if (wrappedBinomialNode != null)
  {
    data.withDelimiters = true;
    data.sizeSpec = "";

    var theChildren = msiNavigationUtils.getSignificantContents(wrappedBinomialNode);
    if (theChildren.length > 2)
    {
      var firstNode = theChildren[0];
      data.leftBracket = firstNode.textContent;

      var lastNode = theChildren[theChildren.length - 1];
      data.rightBracket = lastNode.textContent;

      gDialog.lineSpec = "";
      var fracNode = theChildren[1];
      data.lineSpec = "";
      if (fracNode.hasAttribute("linethickness"))
        data.lineSpec = fracNode.getAttribute("linethickness");
      gDialog.lineSpec = msiMathStyleUtils.convertLineThicknessToDialogForm(fracNode, data.lineSpec);
    }

    var styleObj = new Object();
    var foundAttrs = new Object();
    styleObj["displaystyle"] = "";
    msiNavigationUtils.findStyleEnclosingObj(objectNode, "binomial", styleObj, foundAttrs);

    if ("displaystyle" in foundAttrs)
    {
      if  (foundAttrs["displaystyle"] == "false")
        data.sizeSpec = "small";
      else if (foundAttrs["displaystyle"] == "true")
        data.sizeSpec = "big";
    }
    
    gDialog.withDelimiters = data.withDelimiters;
    gDialog.sizeSpec = data.sizeSpec;

    gDialog.leftBracket = data.leftBracket;
    gDialog.rightBracket = data.rightBracket;
    if (!gDialog.leftBracket.length)
      gDialog.leftBracket = "(";
    if (!gDialog.rightBracket.length)
      gDialog.rightBracket = ")";
    
  }
}


function ensureDataSet()
{
  if (!("withDelimiters" in data))
    data.withDelimiters = true;
  if (!("lineSpec" in data))
    data.lineSpec = "";
  if (!("sizeSpec" in data))
    data.sizeSpec = "";

  if (!("leftBracket" in data))
    data.leftBracket = "(";
  if (!("rightBracket" in data))
    data.rightBracket = ")";
}

function getPropertiesDialogTitle()
{
  return document.getElementById("propertiesTitle").value;
}

function InitDialog()
{
  var bSelectOnFocus = true;
  makeMSIButtonGroup(gDialog.LeftBracketGroup, true, gDialog.RightBracketGroup.id);
  makeMSIButtonGroup(gDialog.RightBracketGroup, true);

  var nWhichSel = -1;
  nWhichSel = setSelectionByValue(gDialog.LeftBracketGroup, gDialog.leftBracket);
  if (nWhichSel < 0)
  {
    setSelectionByIndex(0, gDialog.LeftBracketGroup.id);
    gDialog.leftBracket = gDialog.LeftBracketGroup.valueStr;
    if (isReviseDialog())
      bSelectOnFocus = false;
  }
  nWhichSel = -1;
  nWhichSel = setSelectionByValue(gDialog.RightBracketGroup, gDialog.rightBracket);
  if (nWhichSel < 0)
  {
    setSelectionByIndex(0, gDialog.RightBracketGroup.id);
    gDialog.rightBracket = gDialog.RightBracketGroup.valueStr;
    if (isReviseDialog())
      bSelectOnFocus = false;
  }
  if (!bSelectOnFocus)
  {
    gDialog.LeftBracketGroup.setAttribute("selectsOnFocus", "false");
    gDialog.RightBracketGroup.setAttribute("selectsOnFocus", "false");
  }

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


//The determination of which value is "current" (i.e., would be passed to the object being inserted or revised) is made by checking
//  whether the control contains a different value from its starting one (kept in "gDialog"). If so, that value is to be used; otherwise,
//  the value from "data" is current.
function getCurrLeftDelimiter()
{
  if (gDialog.LeftBracketGroup.valueStr !=  gDialog.leftBracket)
    return gDialog.LeftBracketGroup.valueStr;
  return data.leftBracket;
}

function getCurrRightDelimiter()
{
  if (gDialog.RightBracketGroup.valueStr !=  gDialog.rightBracket)
    return gDialog.RightBracketGroup.valueStr;
  return data.rightBracket;
}

function getCurrLineThickness()
{
  if (gDialog.lineSpecGroup.value != gDialog.lineSpec)
    return gDialog.lineSpecGroup.value;
  return data.lineSpec;
}

function drawSample(sampleControl)
{
  if (gDialog.withDelimiters)
  {
    document.getElementById("binomialSampleLeftFence").firstChild.nodeValue = getCurrLeftDelimiter();
    document.getElementById("binomialSampleRightFence").firstChild.nodeValue = getCurrRightDelimiter();
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
  var currThickness = getCurrLineThickness();
  if (currThickness == "thick")
    document.getElementById("binomialSampleMFrac").setAttribute("linethickness", "thick");
  else if (currThickness == "0")
    document.getElementById("binomialSampleMFrac").setAttribute("linethickness", "0");
  else
    document.getElementById("binomialSampleMFrac").removeAttribute("linethickness");
}

function setLineThickness(lineSpec)
{
//  gDialog.lineSpec = lineSpec.value;
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
//  data.leftBracket = gDialog.LeftBracketGroup.valueStr;
//  data.rightBracket = gDialog.RightBracketGroup.valueStr;
  data.leftBracket = getCurrLeftDelimiter();
  data.rightBracket = getCurrRightDelimiter();
  data.withDelimiters = gDialog.withDelimiters;
//  data.lineSpec = gDialog.lineSpec;
  data.lineSpec = getCurrLineThickness();
  data.sizeSpec = gDialog.sizeSpec;

  if (!data.withDelimiters)
  {
    data.leftBracket = "";
    data.rightBracket = "";
  }

  var editorElement = msiGetParentEditorElementForDialog(window);
  var editor = msiGetEditor(editorElement);
  var theWindow = window.opener;

  if (isReviseDialog())
  {
    if (!theWindow || !("reviseBinomial" in theWindow))
      theWindow = msiGetTopLevelWindow();

    theWindow.reviseBinomial(data.reviseObject, data.leftBracket, data.rightBracket, data.lineSpec, data.sizeSpec, editorElement);
  }
  else
  {
    if (!theWindow || !("insertBinomial" in theWindow))
      theWindow = msiGetTopLevelWindow();

    theWindow.insertBinomial(data.leftBracket, data.rightBracket, data.lineSpec, data.sizeSpec, editorElement);
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