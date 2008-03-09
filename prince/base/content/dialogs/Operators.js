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
  if (!editor) {
    window.close();
    return;
  }

  doSetOKCancel(onAccept, onCancel);
  data = window.arguments[0];
  data.Cancel = false;
  var bRevise = false;
  if (isReviseDialog())
  {
    SetDataFromReviseObject(data.reviseObject);
    bRevise = true;
  }

  gDialog.operatorStr = data.operator;
  if (!gDialog.operatorStr.length)
    gDialog.operatorStr = String.fromCharCode(0x222B);
  gDialog.limitsSpec = data.limitsSpec;
  gDialog.sizeSpec = data.sizeSpec;

  gDialog.OperatorsGroup = document.getElementById("operatorsButtonGroup");
  gDialog.limitsSpecGroup = document.getElementById("limitPositionRadioGroup");
  gDialog.sizeSpecGroup = document.getElementById("sizeRadioGroup");

  InitDialog();

  window.mMSIDlgManager = new msiDialogConfigManager(window);
  if (bRevise)
  {
    window.mMSIDlgManager.mbIsRevise = true;
    window.mMSIDlgManager.mbCloseOnAccept = true;
  }
  window.mMSIDlgManager.configureDialog();

  gDialog.limitsSpecGroup.focus();

  SetWindowLocation();
}

function isReviseDialog()
{
  if ( ("reviseObject" in data) && (data.reviseObject != null) )
    return true;
  return false;
}

function getPropertiesDialogTitle()
{
  return document.getElementById("propertiesTitle").value;
}

function SetDataFromReviseObject(objectNode)
{
  try 
  {
    var outerOperator = msiNavigationUtils.getWrappedObject(objectNode, "operator");
    var operatorNode = null;
    if (msiGetBaseNodeName(outerOperator) == "mo")
      operatorNode = outerOperator;
    else
      operatorNode = msiNavigationUtils.getEmbellishedOperator(outerOperator);

    data.operator = operatorNode.textContent;
    data.limitsSpec = "auto";
    if (operatorNode.hasAttribute("msiLimitPlacement"))
    {
      switch(operatorNode.getAttribute("msiLimitPlacement"))
      {
        case "msiLimitsAboveBelow":
          data.limitsSpec = "aboveBelow";
        break;
        case "msiLimitsAtRight":
          data.limitsSpec = "atRight";
        break;
        default:
        break;
      }
    }

    var styleVals = new Object();
    styleVals["displaystyle"] = "";
    var foundAttrs = new Object();
    var styleNode = msiNavigationUtils.findStyleEnclosingObj(objectNode, "operator", styleVals, foundAttrs);
    data.sizeSpec = "auto";
    if ("displaystyle" in foundAttrs)
    {
      if (foundAttrs["displaystyle"] == "true")
        data.sizeSpec = "big";
      else if (foundAttrs["displaystyle"] == "false")
        data.sizeSpec = "small";
    }
  }
  catch(err) { dump("In Operators.js, in SetDataFromReviseObject, exception: [" + err + "].\n"); }
}

function InitDialog()
{
  makeMSIButtonGroup(gDialog.OperatorsGroup, true);
  var nWhichSel = -1;
  nWhichSel = setSelectionByValue(gDialog.OperatorsGroup, gDialog.operatorStr);
  if (nWhichSel < 0)
  {
    if (isReviseDialog())
      gDialog.OperatorsGroup.valueStr = "";
    else
      setSelectionByIndex(0, gDialog.OperatorsGroup.id);
  }

  var limitSpecItems = gDialog.limitsSpecGroup.getElementsByAttribute("value", gDialog.limitsSpec);
  if (!limitSpecItems.length)
    limitSpecItems = gDialog.limitsSpecGroup.getElementsByAttribute("group", gDialog.limitsSpecGroup.id);
  if (!limitSpecItems.length)
    limitSpecItems = gDialog.limitsSpecGroup.getElementsByTagName("radio");
  if (limitSpecItems.length > 0)
    gDialog.limitsSpecGroup.selectedItem = limitSpecItems[0];

  var sizeSpecItems = gDialog.sizeSpecGroup.getElementsByAttribute("value", gDialog.sizeSpec);
  if (!sizeSpecItems.length)
    sizeSpecItems = gDialog.sizeSpecGroup.getElementsByAttribute("group", gDialog.sizeSpecGroup.id);
  if (!sizeSpecItems.length)
    sizeSpecItems = gDialog.sizeSpecGroup.getElementsByTagName("radio");
  if (sizeSpecItems.length > 0)
    gDialog.sizeSpecGroup.selectedItem = sizeSpecItems[0];

  checkInaccessibleAcceleratorKeys(document.documentElement);

  gDialog.tabOrderArray = new Array( gDialog.limitsSpecGroup, gDialog.sizeSpecGroup, gDialog.OperatorsGroup,
                                       document.documentElement.getButton("accept"),
                                       document.documentElement.getButton("cancel") );
  
  document.documentElement.getButton("accept").setAttribute("default", true);

  var sampleDependencies = new Array(gDialog.limitsSpecGroup, gDialog.sizeSpecGroup, gDialog.OperatorsGroup);
  var operatorSample = document.getElementById("OperatorPreview");
  makeSampleWindowDependOn(operatorSample, sampleDependencies);

  drawSample(operatorSample);
//  sizeToContent();
}

function getCurrOperatorString()
{
  if (gDialog.OperatorsGroup.valueStr.length > 0)
    return gDialog.OperatorsGroup.valueStr;
  return gDialog.operatorStr;
}

function drawSample(sampleControl)
{
  var sampleMStyle = document.getElementById("operatorSampleMStyle");

  switch (gDialog.sizeSpecGroup.value)
  {
    case "big":
      sampleMStyle.setAttribute("displaystyle", "true");
    break;
    case "small":
      sampleMStyle.setAttribute("displaystyle", "false");
    break;
    case "auto":
      sampleMStyle.removeAttribute("displaystyle");
    default:
    break;
  }

  var theOperator = document.getElementById("operatorSampleBase");
  var limitsValue = gDialog.limitsSpecGroup.value;
  if (limitsValue == "auto")
    theOperator.setAttribute("movablelimits", "true");
  else
    theOperator.setAttribute("movablelimits", "false");
  var currOperatorStr = getCurrOperatorString();
  var newOperator = msiSetMathTokenText(theOperator, currOperatorStr, null);
  if (newOperator != theOperator)
    theOperator = newOperator;  //this actually doesn't currently get used below, but just in case...
//  if (theOperator.textContent != currOperatorStr)
//  {
////    theOperator.textContent = currOperatorStr;
//    var newOp = document.createElementNS(mmlns, "mo");
//    msiCopyElementAttributes(newOp, theOperator);
//    newOp.appendChild(document.createTextNode(currOperatorStr));
//    theOperator.parentNode.replaceChild(newOp, theOperator);
//    theOperator = newOp;
////      firstNode.textContent = left;
//  }

  var mStyleContents = msiNavigationUtils.getSignificantContents(sampleMStyle);
  var currOuterOp = mStyleContents[0];
  var currOuterOpName = msiGetBaseNodeName(currOuterOp);
  var newNode = null;
  if (limitsValue == "aboveBelow")
  {
    if (currOuterOpName == "msubsup")
      newNode = document.createElementNS(mmlns, "munderover");
  }
  else if (currOuterOpName == "munderover")
  {
    newNode = document.createElementNS(mmlns, "msubsup");
  }
  if (newNode != null)
  {
    var outerOpContents = msiNavigationUtils.getSignificantContents(currOuterOp);
    for (var ix = 0; ix < outerOpContents.length; ++ix)
      newNode.appendChild( currOuterOp.removeChild(outerOpContents[ix]) );
    sampleMStyle.replaceChild( newNode, currOuterOp );
  }

//  try
//  {
//    var requestor = window.QueryInterface(Components.interfaces.nsIInterfaceRequestor);
//    if (!requestor && window.parent)
//      requestor = window.parent.QueryInterface(Components.interfaces.nsIInterfaceRequestor);
//    var webnavigation=requestor.getInterface(Components.interfaces.nsIWebNavigation);
//    var basewindow=webnavigation.QueryInterface(Components.interfaces.nsIBaseWindow);
//    basewindow.repaint(true);
//  }
//  catch(exc) {AlertWithTitle("Error", "Unable to get nsIBaseWindow interface in Operators.js, drawSample() - error is [" + exc + "].");}
}

function onAccept()
{
//  gDialog.operatorStr = gDialog.OperatorsGroup.valueStr;
  data.operator = getCurrOperatorString();
  data.limitsSpec = gDialog.limitsSpecGroup.value;
  data.sizeSpec = gDialog.sizeSpecGroup.value;

  var editorElement = msiGetParentEditorElementForDialog(window);
  var theWindow = window.opener;
  if (isReviseDialog())
  {
    if (!theWindow || !("reviseOperator" in theWindow))
      theWindow = msiGetTopLevelWindow();
    theWindow.reviseOperator(data.reviseObject, data.operator, data.limitsSpec, data.sizeSpec, editorElement);
  }
  else
  {
    if (!theWindow || !("insertOperator" in theWindow))
      theWindow = msiGetTopLevelWindow();
    theWindow.insertOperator(data.operator, data.limitsSpec, data.sizeSpec, editorElement);
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