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
  if ( !("reviseObject" in data) || (data.reviseObject == null) )
  {
    AlertWithTitle("radicalProperties.js", "Error in radicalProperties dialog - no radical to revise passed in!");
    window.close();
    return;
  }
  setDataFromReviseObject(data.reviseObject);

//  gDialog.rootStr = data.rootStr;

  gDialog.RadicalGroup = document.getElementById("radicalButtonGroup");

  InitDialog();

  if (gDialog.RadicalGroup.valueStr.length > 0)
    gDialog.RadicalGroup.focus();

  SetWindowLocation();
}

function InitDialog()
{
  makeMSIButtonGroup(gDialog.RadicalGroup, false);
  var nWhichSel = -1;
  if (gDialog.radicalStr)
  {
    nWhichSel = setSelectionByValue(gDialog.RadicalGroup, gDialog.radicalStr);
    if (nWhichSel < 0)
      setSelectionByIndex(0, gDialog.RadicalGroup.id);
  }

  checkInaccessibleAcceleratorKeys(document.documentElement);
  gDialog.tabOrderArray = new Array( gDialog.RadicalGroup,
                                       document.documentElement.getButton("accept"),
                                       document.documentElement.getButton("cancel") );

//  document.documentElement.addEventListener('ButtonGroupSelectionChange', checkEnableControls, false);
//  checkEnableControls();
  
  document.documentElement.getButton("accept").setAttribute("default", true);
}

function setDataFromReviseObject(reviseObject)
{
  var retVal = null;
  var theName = msiGetBaseNodeName(reviseObject);
  switch(theName)
  {
    case "mroot":
    case "msqrt":
      gDialog.radicalStr = theName;
      return;
    break;

    case "mstyle":
    case "mrow":
    {
      var nodeContents = msiNavigationUtils.getSignificantContents(reviseObject);
      if (nodeContents.length == 1)
        setDataValuesFromPropertiesObject(nodeContents[0]);
    }
    break;

    default:
    break;
  }
}

//function checkEnableControls(event)
//{
//  var disableAboveBelowStr = "false";
//  var disableAroundStr = "false";
//  var disableOKStr = "false";
//  if (gDialog.DecorationsAboveGroup.valueStr.length > 0 || gDialog.DecorationsBelowGroup.valueStr.length > 0)
//  {
//    disableAroundStr = "true";
//  }
//  else if (gDialog.DecorationsAroundGroup.valueStr.length > 0)
//  {
//    disableAboveBelowStr = "true";
//  }
//  else
//    disableOKStr = "true";
//
//  document.getElementById("enableDecorationsAboveBelow").setAttribute("disabled", disableAboveBelowStr);
//  document.getElementById("enableDecorationsAboveBelow").setAttribute("disabled", disableAboveBelowStr);
//  document.getElementById("enableDecorationsAround").setAttribute("disabled", disableAroundStr);
//  document.documentElement.getButton("accept").setAttribute("disabled", disableOKStr);
//}

function onAccept()
{
  gDialog.radicalStr = gDialog.RadicalGroup.valueStr;
  data.radicalStr = gDialog.radicalStr;

//  var editorElement = msiGetParentEditorElementForDialog(window);

  var editorElement = msiGetParentEditorElementForDialog(window);
  var editor = msiGetEditor(editorElement);
  var theWindow = window.opener;
  if (!theWindow || !("reviseRadical" in theWindow))
    theWindow = msiGetTopLevelWindow();

  theWindow.reviseRadical(data.reviseObject, data.radicalStr, editorElement);

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