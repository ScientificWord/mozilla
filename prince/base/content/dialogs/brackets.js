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

  var bRevise = isReviseDialog();
  if (bRevise)
    setDataFromReviseObject(data.reviseObject);

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

  window.mMSIDlgManager = new msiDialogConfigManager(window);
  if (bRevise)
  {
    window.mMSIDlgManager.mbIsRevise = true;
    window.mMSIDlgManager.mbCloseOnAccept = true;
  }
  window.mMSIDlgManager.configureDialog();

//  gDialog.LeftBracketGroup.focus();
  msiSetInitialDialogFocus(gDialog.LeftBracketGroup);


  SetWindowLocation();
}

function isReviseDialog()
{
  if ( ("reviseObject" in data) && (data.reviseObject != null) )
    return true;
  return false;
}

function setDataFromReviseObject(objectNode)
{
  var fenceNode = msiNavigationUtils.getWrappedObject(objectNode, "fence");
  if (fenceNode == null)
  {
    dump("Problem in Brackets.js, setDataFromReviseObject - fence node not found!\n");
    return;
  }
  var children = msiNavigationUtils.getSignificantContents(fenceNode);
  if (children.length < 1)
  {
    dump("Problem in Brackets.js, setDataFromReviseObject - fence node has no children!\n");
    return;
  }
  data.leftBracket = children[0].textContent;
  data.rightBracket = children[children.length - 1].textContent;
}

function getPropertiesDialogTitle()
{
  return document.getElementById("propertiesTitle").value;
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
    nWhichSelL = setSelectionByValue(gDialog.LeftBracketGroup, gDialog.LeftBracketGroup.valueStr);
//  We do NOT want to set defaults. They are gotten from the persisted user data, if there is any, or from the XUL file --BBM
//  if (nWhichSel < 0)
//    setSelectionByIndex(0, gDialog.LeftBracketGroup.id);
  if (nWhichSelR < 0);
    nWhichSelR = setSelectionByValue(gDialog.RightBracketGroup, gDialog.RightBracketGroup.valueStr);
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

  var sampleStyle = document.defaultView.getComputedStyle(gDialog.BracketPreview, null);
  var initHeight = sampleStyle.getPropertyValue("height");
  gDialog.BracketPreview.style.maxHeight = initHeight;
}

//NOTE!! All the convoluted replacement code used below is necessary due to a bug in Mozilla MathML rendering.
//Changing the textContent of a token node <mo> or <mi> does not reliably force a rerendering.
function drawSample(sampleControl)
{
//  document.getElementById("leftBracketSample").textContent = gDialog.LeftBracketGroup.valueStr;
//  document.getElementById("rightBracketSample").textContent = gDialog.RightBracketGroup.valueStr;
  var leftBrackSamp = document.getElementById("leftBracketSample");
//  var opening = document.createElementNS(mmlns, "mo");
//  opening.setAttribute("form", "prefix");
//  opening.setAttribute("fence", "true");
//  opening.setAttribute("stretchy", "true");
//  opening.appendChild(document.createTextNode(gDialog.LeftBracketGroup.valueStr));
//  leftBrackSamp.parentNode.replaceChild( opening, leftBrackSamp );
//  opening.setAttribute("id", "leftBracketSample");

  var opening = msiSetMathTokenText(leftBrackSamp, gDialog.LeftBracketGroup.valueStr, null);
  if (opening != leftBrackSamp)
  {
    if (opening.getAttribute("id") != "leftBracketSample")
      dump("Problem in Brackets.js, drawSample! Replacement left bracket has lost the id attribute.\n");
    leftBrackSamp = opening;
  }

//  var leftText = document.createTextNode(gDialog.LeftBracketGroup.valueStr);
//  var n = leftBrackSamp.childNodes.length;
//  for (var i = n-1; i >= 0; --i)
//    leftBrackSamp.removeChild(leftBrackSamp.childNodes[i]);
//  leftBrackSamp.appendChild(leftText);
//  document.getElementById("rightBracketSample").textContent = gDialog.RightBracketGroup.valueStr;
  var rightBrackSamp = document.getElementById("rightBracketSample");
//  var closing = document.createElementNS(mmlns, "mo");
//  closing.setAttribute("form", "postfix");
//  closing.setAttribute("fence", "true");
//  closing.setAttribute("stretchy", "true");
//  closing.appendChild(document.createTextNode(gDialog.RightBracketGroup.valueStr));
//  rightBrackSamp.parentNode.replaceChild( closing, rightBrackSamp );
//  closing.setAttribute("id", "rightBracketSample");

  var closing = msiSetMathTokenText(rightBrackSamp, gDialog.RightBracketGroup.valueStr, null);
  if (closing != rightBrackSamp)
  {
    if (closing.getAttribute("id") != "rightBracketSample")
      dump("Problem in Brackets.js, drawSample! Replacement right bracket has lost the id attribute.\n");
    if (closing.getAttribute("form") != "postfix")
      dump("Problem in Brackets.js, drawSample! Replacement right bracket has lost the form attribute.\n");
    if (closing.getAttribute("fence") != "true")
      dump("Problem in Brackets.js, drawSample! Replacement right bracket has lost the fence attribute.\n");
    if (closing.getAttribute("stretchy") != "true")
      dump("Problem in Brackets.js, drawSample! Replacement right bracket has lost the stretchy attribute.\n");
    rightBrackSamp = closing;
  }

//usethis??  window.sizeToContent();

//  var rightText = document.createTextNode(gDialog.RightBracketGroup.valueStr);
//  n = rightBrackSamp.childNodes.length;
//  for (var i = n-1; i >= 0; --i)
//    rightBrackSamp.removeChild(rightBrackSamp.childNodes[i]);
//  rightBrackSamp.appendChild(rightText);
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

  var editorElement = msiGetParentEditorElementForDialog(window);
  var theWindow = window.opener;
  if (isReviseDialog())
  {
    if (!theWindow || !("reviseFence" in theWindow))
      theWindow = msiGetTopLevelWindow();
    theWindow.reviseFence(data.reviseObject, data.leftBracket, data.rightBracket, editorElement);
  }
  else
  {
    if (!theWindow || !("insertfence" in theWindow))
    {
////Logging stuff only
//      var logStr = "window.opener is [";
//      if (window.opener)
//      {
//        if (window.opener.name)
//          logStr += window.opener.name;
//        else if (window.opener.document)
//        {
//          if (window.opener.document.localName)
//            logStr += "document: " + window.opener.document.localName;
//          else if (window.opener.document.documentElement)
//            logStr += "document element: " + window.opener.document.documentElement.nodeName;
//          else
//            logStr += "Anonymous document";
//        }
//        else
//          logstr += "Anonymous window (without document)";
//      }
//      logStr += "]";
//      msiKludgeLogString(logStr);
////End logging stuff
      theWindow = msiGetTopLevelWindow();
    }
//    theWindow.insertfence(data.leftBracket, data.rightBracket, data.separator, editorElement);
    if (isInMath(editorElement))
      theWindow.insertfence(data.leftBracket, data.rightBracket, editorElement);
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