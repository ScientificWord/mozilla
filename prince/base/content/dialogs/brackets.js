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
  var editor = GetCurrentEditor();
  if (!editor) {
    window.close();
    return;
  }

  doSetOKCancel(onAccept, onCancel);
  data = window.arguments[0];
  data.Cancel = false;

  gDialog.LeftBracketGroup = document.getElementById("leftBracketGroup");
  gDialog.RightBracketGroup = document.getElementById("rightBracketGroup");
  gDialog.BracketPreview = document.getElementById("BracketPreview");

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

  gDialog.LeftBracketGroup.focus();

  SetWindowLocation();
}

function InitDialog()
{
  makeMSIButtonGroup(gDialog.LeftBracketGroup, true, gDialog.RightBracketGroup.id);
  makeMSIButtonGroup(gDialog.RightBracketGroup, true);
  gDialog.LeftBracketGroup.valueStr = data.leftBracket;
  gDialog.RightBracketGroup.valueStr = data.rightBracket;
  var kids =  gDialog.LeftBracketGroup.getElementsByTagName("button");
  var n =  Number(gDialog.LeftBracketGroup.getAttribute("lastval"));
  
  // if left and right bracket are not passed in, use the ones last used by the user. These
  // are persisted as "lastval" in the bracket groups.  (BBM)
  var nWhichSelL = -1;
  var nWhichSelR = -1;
  
  if ((!gDialog.LeftBracketGroup.valueStr.length)&&(n>=0)&&(n<kids.length))
  {  
    setSelectionByIndex(n,"leftBracketGroup");
    gDialog.LeftBracketGroup.valueStr = getButtonValue(kids[n]);
    nWhichSelL = n;
  }
 
  kids =  gDialog.RightBracketGroup.getElementsByTagName("button");
  n =  Number(gDialog.RightBracketGroup.getAttribute("lastval"));
  
  if ((!gDialog.RightBracketGroup.valueStr.length)&&(n>=0)&&(n<kids.length))
  {  
    setSelectionByIndex(n,"rightBracketGroup");
    gDialog.RightBracketGroup.valueStr = getButtonValue(kids[n]);
    nWhichSelR = n;
  }
  if (nWhichSelL == -1)   
    nWhichSel = setSelectionByValue(gDialog.LeftBracketGroup, gDialog.LeftBracketGroup.valueStr);
//  We do NOT want to set defaults. They are gotten from the persisted user data, if there is any, or from the XUL file --BBM
//  if (nWhichSel < 0)
//    setSelectionByIndex(0, gDialog.LeftBracketGroup.id);
  if (nWhichSelR < 0);
    nWhichSel = setSelectionByValue(gDialog.RightBracketGroup, gDialog.RightBracketGroup.valueStr);
//  We do NOT want to set defaults. They are gotten from the persisted user data, if there is any, or from the XUL file -- BBM
//  if (nWhichSel < 0)
//    setSelectionByIndex(0, gDialog.RightBracketGroup.id);


// If you work out the logic of the above, there is no case for when valueStr has no value and n has no value. Actually,
// n is guaranteed to exist since it is in the original XUL file (as lastval) and if modified only if the new value is
// legal.  (BBM)
  gDialog.tabOrderArray = new Array( gDialog.LeftBracketGroup, gDialog.RightBracketGroup,
                                       document.documentElement.getButton("accept"),
                                       document.documentElement.getButton("cancel") );

  checkInaccessibleAcceleratorKeys(document.documentElement);
  

  document.documentElement.getButton("accept").setAttribute("default", true);
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
//  gDialog.BracketPreview.setAttribute(styleStr, styleValue);

  var sampleDependencies = new Array(gDialog.LeftBracketGroup, gDialog.RightBracketGroup);
  makeSampleWindowDependOn(gDialog.BracketPreview, sampleDependencies);

  drawSample(gDialog.BracketPreview);
}

function drawSample(sampleControl)
{
  document.getElementById("leftBracketSample").textContent = gDialog.LeftBracketGroup.valueStr;
  document.getElementById("rightBracketSample").textContent = gDialog.RightBracketGroup.valueStr;
//  var opening = document.createElementNS(mmlns, "mo");
//  opening.setAttribute("form", "prefix");
//  opening.setAttribute("fence", "true");
//  opening.setAttribute("stretchy", "true");
//  opening.appendChild(document.createTextNode(gDialog.LeftBracketGroup.valueStr));
//  var closing = document.createElementNS(mmlns, "mo");
//  closing.setAttribute("form", "postfix");
//  closing.setAttribute("fence", "true");
//  closing.setAttribute("stretchy", "true");
//  closing.appendChild(document.createTextNode(gDialog.RightBracketGroup.valueStr));
//  var content = document.createElementNS(mmlns,"mi");
//  content.appendChild(document.createTextNode(String.fromCharCode(0x2039,0x203a)));
////  content.appendChild(document.createTextNode(emptyElementStr));
//  var mrow = document.createElementNS(mmlns, "mrow");
//  mrow.appendChild(opening);
//  mrow.appendChild(content);
//  mrow.appendChild(closing);
//  var div = document.createElementNS(xhtmlns,"div");
//  div.appendChild(mrow);
//  var math = document.createElementNS(mmlns,"math");
//  math.setAttribute("display","inline");
//  math.appendChild(div);
//  var oldMathKids = sampleControl.getElementsByTagNameNS(mmlns, "math");
//  sampleControl.replaceChild(math, oldMathKids.item(0));
}

function onAccept()
{
  data.leftBracket = gDialog.LeftBracketGroup.valueStr;
  data.rightBracket = gDialog.RightBracketGroup.valueStr;
  var kids =  gDialog.LeftBracketGroup.getElementsByTagName("button");
  // set 'lastval' attributes
  for (var i = 0; i < kids.length; ++i)
  {
    if (kids[i].checked)
    {
      gDialog.LeftBracketGroup.setAttribute("lastval",String(i));
      break;
    }
  }
  kids =  gDialog.RightBracketGroup.getElementsByTagName("button");
  for (var i = 0; i < kids.length; ++i)
  {
    if (kids[i].checked)
    {
      gDialog.RightBracketGroup.setAttribute("lastval",String(i));
      break;
    }
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