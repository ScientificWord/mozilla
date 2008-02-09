// Copyright (c) 2007 MacKichan Software, Inc.  All Rights Reserved.

//var gBodyElement;

//const mmlns    = "http://www.w3.org/1998/Math/MathML";
//const xhtmlns  = "http://www.w3.org/1999/xhtml";

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
  gDialog.unitsList = new msiMathUnitsList();

  InitDialog();

//  window.mMSIDlgManager = new msiDialogConfigManager(window);
//  window.mMSIDlgManager.configureDialog();

  SetWindowLocation();
}

function InitDialog()
{
//  fillTypesListbox();
  var startUnit = null;
  var startType = null;
  var bIsRevise = false;
  if (data != null && ("reviseObject" in data))
  {
    bIsRevise = true;
    setDataFromReviseObject(data.reviseObject);
  }
    
  window.mMSIDlgManager = new msiDialogConfigManager(window);
  if (bIsRevise)
  {
    window.mMSIDlgManager.mbIsRevise = true;
    window.mMSIDlgManager.mbCloseOnAccept = true;
  }
  window.mMSIDlgManager.configureDialog();
  
  if (data != null && ("unitString" in data))
  {
    var startUnitStr = data.unitString;
    var startUnitObj = gDialog.unitsList.findUnitByString(startUnitStr);
    if (startUnitObj != null)
    {
      startType = startUnitObj.type;
      startUnit = startUnitObj.id;
    }
  }
  fillTypesListbox(startType);
  startType = document.getElementById("unitTypesListbox").value;
  setUnitType(startType, startUnit);
//  checkInaccessibleAcceleratorKeys(document.documentElement);
//
//  gDialog.tabOrderArray = new Array( gDialog.DecorationsAboveGroup, gDialog.DecorationsBelowGroup,
//                                       gDialog.DecorationsAroundGroup,
//                                       document.documentElement.getButton("accept"),
//                                       document.documentElement.getButton("cancel") );

  document.documentElement.getButton("accept").setAttribute("default", true);
}

function setDataFromReviseObject(reviseNode)
{
  var wrappedUnitNode = msiNavigationUtils.getWrappedObject(reviseNode, "unit");
  data.unitString = wrappedUnitNode.textContent;
}

function isReviseDialog()
{
  return ((data != null) && ("reviseObject" in data) && (data.reviseObject != null));
}

function getPropertiesDialogTitle()
{
  return document.getElementById("propertiesTitle").value;
}

function fillTypesListbox(selectType)
{
  var theListbox = document.getElementById("unitTypesListbox");
  var unitStrings = document.getElementById("unitStrings");
  var typeString = "";
  var selectItem = null;
  var typeInfoList = new Array();
  for (var aType in gDialog.unitsList.typeList)
  {
    typeString = "";
    try
    {
      typeString = unitStrings.getString(aType);
    } catch(exc) {typeString = aType;}
    var typeInfo = new Object();
    typeInfo.displayString = typeString;
    typeInfo.rawString = aType;
    typeInfoList.push( typeInfo );
  }
  typeInfoList.sort(sortByDisplayString);
  for (var ix = 0; ix < typeInfoList.length; ++ix)
  {
    if (selectType != null && selectType == typeInfoList[ix].rawString)
      selectItem = theListbox.appendItem(typeInfoList[ix].displayString, typeInfoList[ix].rawString);
    else
      theListbox.appendItem(typeInfoList[ix].displayString, typeInfoList[ix].rawString);
  }
  if (selectItem != null)
    theListbox.selectItem(selectItem);
  else
    theListbox.selectedIndex = 0;
}

function sortByDisplayString(item1, item2)
{
  if (!item1.displayString || !item2.displayString)
  {
    var badItemStr = "unknown";
    if (!item1.displayString)
    {
      if (item1.rawString)
        badItemStr = item1.rawString;
    }
    else if (!item2.displayString)
    {
      if (item2.rawString)
        badItemStr = item2.rawString;
    }
    dump("Problem in mathUnitsDialog.sortByDisplayString - no display string passed in for item [" + badItemStr + "].\n");
    return 0;
  }

  var lc1 = item1.displayString.toLowerCase();
  var lc2 = item2.displayString.toLowerCase();
  if (lc1 < lc2)
    return -1;
  else if (lc2 < lc1)
    return 1;
  else if (item1.displayString < item2.displayString)
    return -1;
  else if (item2.displayString < item1.displayString)
    return 1;
  else
    return 0;
}

function setUnitType(whichType, unitToSelect)
{
  var unitsListbox = document.getElementById("unitNamesListbox");
  var unitStrings = document.getElementById("unitStrings");
  var displayStr = "";
  var theUnitStr = "";
  if (!(whichType in gDialog.unitsList.typeList) || (gDialog.unitsList.typeList[whichType] == null))
  {
    dump("In mathUnitsDialog.js, failed to find unit type [" + whichType + "] in unitsList.\n");
  }
  for (var jx = unitsListbox.getRowCount() - 1; jx >= 0; --jx)
    unitsListbox.removeItemAt(jx);
  var unitInfoList = new Array();
  for (var ix = 0; ix < gDialog.unitsList.typeList[whichType].length; ++ix)
  {
    theUnitStr = gDialog.unitsList.typeList[whichType][ix];
    displayStr = "";
    try
    {
      var theUnitName = gDialog.unitsList.names[theUnitStr].name;
      displayStr = unitStrings.getString(theUnitName);
    } catch(exc) {displayStr = theUnitStr;}
    var unitInfo = new Object();
    unitInfo.displayString = displayStr;
    unitInfo.rawString = theUnitStr;
    unitInfoList.push( unitInfo );
  }
  unitInfoList.sort(sortByDisplayString);
  for (var ix = 0; ix < unitInfoList.length; ++ix)
  {
//    if (unitToSelect != null && unitToSelect == unitInfoList[ix].displayString)
    if (unitToSelect != null && unitToSelect == unitInfoList[ix].rawString)
      unitsListbox.selectItem(unitsListbox.appendItem(unitInfoList[ix].displayString, unitInfoList[ix].rawString));
    else
      unitsListbox.appendItem(unitInfoList[ix].displayString, unitInfoList[ix].rawString);
  }
  if (unitToSelect == null)
    unitsListbox.selectedIndex = 0;
}

function selectUnit(unitToSelect)
{
  var unitsListbox = document.getElementById("unitNamesListbox");
  for (var ix = 0; ix < unitsListbox.getRowCount(); ++ix)
  {
    if (unitsListbox.getItemAtIndex(ix).value == unitToSelect)
    {
      unitsListbox.selectedIndex = ix;
      break;
    }
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
  var unitsListbox = document.getElementById("unitNamesListbox");
  var theUnitName = unitsListbox.value;
//  var unit = gDialog.unitsList.names[theUnitName].data;
  var editorElement = msiGetParentEditorElementForDialog(window);

  var theWindow = window.opener;
  if (isReviseDialog())
  {
    if (!theWindow || !("reviseMathUnit" in theWindow))
      theWindow = msiGetTopLevelWindow();
    var unitNode = data.reviseObject;
    theWindow.reviseMathUnit(unitNode, theUnitName, editorElement);
  }
  else
  {
    if (!theWindow || !("insertmathunit" in theWindow))
      theWindow = msiGetTopLevelWindow();
    theWindow.insertmathunit(theUnitName, editorElement);
  }

//  SaveWindowLocation();
  return true;
//  return false;  //don't close
}

function onCancel()
{
//  data.Cancel = true;
}

function doAccept()
{
  document.documentElement.getButton('accept').oncommand();
}