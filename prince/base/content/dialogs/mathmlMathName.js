// Copyright (c) 2004 MacKichan Software, Inc.  All Rights Reserved.

var target;

function parseBool(s){
  if (s == "true")
    return true;
  else
    return false;
}

function Startup() {
  target = window.arguments[0];
  gDialog.nameList = new msiMathNameList();  //see msiEditorUtilities.js
  gDialog.bStopNextEnter = false;

  function sortNames(name1, name2) {  //the idea is to sort irrespective of case, unless two names differ only by case.
    var lc1 = name1.toLowerCase();
    var lc2 = name2.toLowerCase();
    if (lc1 < lc2)
      return -1;
    else if (lc2 < lc1)
      return 1;
    else if (name1 < name2)
      return -1;
    else if (name2 < name1)
      return 1;
    else
      return 0;
  }

//  var nameArray = [];
//  for (var aName in gDialog.nameList.names)
//    nameArray.push(aName);
//  nameArray.sort(sortNames);
//  var namesBox = document.getElementById("mathNamesBox");
//  for (var ix = 0; ix < nameArray.length; ++ix)
//    namesBox.appendItem(nameArray[ix], nameArray[ix]);  //set the value and the label to be the same
  if (target != null && ("val" in target) && (target.val.length > 0))
    namesBox.value = target.val;
//  else
//    namesBox.selectedIndex = 0;
//  dump("Value of namesBox.disableautoselect is [" + namesBox.disableautoselect + "].\n");
//  namesBox.disableautoselect = true;
//  dump("Value of namesBox.disableautoselect is [" + namesBox.disableautoselect + "].\n");
  changeName();
  // Now load the mathnames file
  // We need to prebuild these so that the keyboard shortcut works
  // ACSA = autocomplete string array
//  var ACSA = Components.classes["@mozilla.org/autocomplete/search;1?name=stringarray"].getService();
//  ACSA.QueryInterface(Components.interfaces.nsIAutoCompleteSearchStringArray);
}

//updateControls() here needs to disable the limitPlacementGroup if the type isn't operator. If the selected name
//  is already in the list, the Add button should be disabled. If not, the Delete button should be. Also, the "Add automatic
//  substitution" checkbox should be disabled if it's already in that list.
//(Q. Do we want to also disable deleting certain core MathNames? This seems to have been done in SWP, so I guess so.
//  How are they to be identified? Or should there be two files containing MathNames, one for "user" names and one for
//  "built-in" ones? Solution proposed for now will be to use one file, and mark built-in ones as such.?)
function updateControls()
{
  var theType = document.getElementById("nameTypeRadioGroup").value;
  var currName = document.getElementById("mathNamesBox").value;
  var bIsNew = gDialog.nameList.canAdd(currName);
  var nameTypeControls = ["nameTypeGroup", "nameTypeLabel", "nameTypeRadioGroup"];
  enableControlsByID(nameTypeControls, bIsNew);
  var dumpStr = "For mathname [" + currName + "]; "
  var limitPlacementControls = ["limitPlacementGroup", "limitPlacementGroupCaption", "operatorLimitPlacementRadioGroup"];
  if (theType == "operator")
  {
    //document.getElementById("limitPlacementGroup").removeAttribute("disabled");
    enableControlsByID(limitPlacementControls, true);
    var thePlacement = "auto";
    if ((currName in gDialog.nameList.names) && ("limitPlacement" in gDialog.nameList.names[currName]))
      thePlacement = gDialog.nameList.names[currName].limitPlacement;
    document.getElementById("operatorLimitPlacementRadioGroup").value = thePlacement;
    dumpStr += "enabling the limitplacementgroup";
  }
  else
  {
    //document.getElementById("limitPlacementGroup").setAttribute("disabled", "true");
    enableControlsByID(limitPlacementControls, false);
    dumpStr += "disabling the limitplacementgroup";
  }

  enableControlsByID(["addButton"], bIsNew);
  enableControlsByID(["deleteButton"], gDialog.nameList.canDelete(currName));
  dumpStr += " canAdd returned ";
  if (bIsNew)
    dumpStr += "true";
  else
    dumpStr += "false";
  dumpStr += " canDelete returned ";
  if (gDialog.nameList.canDelete(currName))
    dumpStr += "true";
  else
    dumpStr += "false";
  var bAutoSub = gDialog.nameList.hasAutoSubstitution(currName);
  if (bAutoSub)
    document.getElementById("addAutoSubstitution").setAttribute("checked", "true");
  else
    document.getElementById("addAutoSubstitution").removeAttribute("checked");
//  var bBuiltIn = gDialog.nameList.isBuiltIn(currName);
//  enableControlsByID(["addAutoSubstitution", "addAutoSubstitutionDescription", "enginefunction", "enginefunctionDescription"], !bBuiltIn);
//  enableControlsByID(["addAutoSubstitution", "addAutoSubstitutionDescription", "enginefunction", "enginefunctionDescription"], !bBuiltIn);
  enableControlsByID(["addAutoSubstitution", "addAutoSubstitutionDescription", "enginefunction", "enginefunctionDescription"], bIsNew);
  dumpStr += "autosubstitution and enginefunction should be ";
  if (!bIsNew)
    dumpStr += "disabled";
  else
    dumpStr += "enabled";
  if (bAutoSub)
    dumpStr += " and autosubstitution is checked";
  else
    dumpStr += " and autosubstitution is unchecked";
  dump(dumpStr + ".\n");
}

//function textEntered(event)
//{
////  var eventAttrStr = "";
////  for (var eventAttr in event)
////  {
////    if (eventAttrStr.length > 0)
////      eventAttrStr += "; [" + eventAttr + "]: [" + event[eventAttr] + "]";
////    else
////      eventAttrStr += "[" + eventAttr + "]: [" + event[eventAttr] + "]";
////  }
////  alert("Got the oninput message!\nEvent looks like: \n" + eventAttrStr);
//  var theKey = event.charCode;
//  if (theKey == 0 || event.altKey || event.ctrlKey || event.metaKey)
//    return;
//  var menuBox = document.getElementById("mathNamesBox");
//  var inputBox = menuBox.inputField;
//  var currText = inputBox.value;
//  var newText = "";
//  if (inputBox.selectionStart > 0)
//    newText = currText.substr(0, inputBox.selectionStart);
//  newText += String.fromCharCode(theKey);
//  if (inputBox.selectionEnd < currText.length)
//    newText += currText.substr(inputBox.selectionEnd);
////  alert("Original text was [" + currText + "]; new text is [" + newText + "].\n");
//  inputBox.value = newText;
//  var nCurrMatchPos = -1;
//  var nChars = newText.length;
//  for (var ix = 0; ix < menuBox.menupopup.childNodes.length; ++ix)
//  {
//    var entry = menuBox.menupopup.childNodes[ix].value;
//    if (entry.substr(0, nChars) == newText)
//    {
//      nCurrMatchPos = ix;
//      break;
//    }
//  }
//  if (nCurrMatchPos >= 0)
//  {
//    dump("Text [" + newText + "] matches beginning of [" + menuBox.menupop.childNodes[nCurrMatchPos].value + "] at position [" + nCurrMatchPos + "].\n");
//    //how do we select in the listbox without replacing the text?
//  }
//  document.getElementById("mathNamesBox").value = newText;
//
//  event.stopPropagation();
//  event.preventDefault();
//
//  changeName();
//}

function checkKeyPressEvent(control, theEvent)
{
  var dumpStr = "In checkKeyPressEvent handler, gDialog.bStopNextEnter is [";
  if (gDialog.bStopNextEnter)
    dumpStr += "true]; ";
  else
    dumpStr += "false]; ";
  if (!theEvent.altKey)
  {
    if (theEvent.keyCode==KeyEvent.DOM_VK_RETURN)
    {
      if (gDialog.bStopNextEnter)
      {
        gDialog.bStopNextEnter = false;
        if (!control)
        {
          dump("Null control in checkKeyPressEvent!\n");
          control = document.getElementById("mathNamesBox");
        }
        control.controller.handleEnter();
        theEvent.stopPropagation();
        theEvent.preventDefault();
        dumpStr += "called theEvent.stopPropagation().\n";
      }
    }
    else //now we're typing into the name field, so we assume we should stop the next enter from accepting the dialog
    {
      gDialog.bStopNextEnter = true;
      dumpStr += "setting gDialog.bStopNextEnter.\n";
    }
  }
  dump(dumpStr);
  //Now hopefully continue processing as usual.
}

function changeName(currName)
{
//  var currName = document.getElementById("mathNamesBox").value;
  var theType = document.getElementById("nameTypeRadioGroup").value;
  var isEngineFunction = false;
  if (currName in gDialog.nameList.names)
  {
    theType = gDialog.nameList.names[currName].type;
    if (("engineFunction" in gDialog.nameList.names[currName]) && (gDialog.nameList.names[currName].engineFunction == true))
      isEngineFunction = true;
  }
  else if (theType == null || theType.length == 0)
    theType = "function";
  document.getElementById("nameTypeRadioGroup").value = theType;
  if (isEngineFunction)
    document.getElementById("enginefunction").setAttribute("checked", "true");
  else
    document.getElementById("enginefunction").setAttribute("checked", "false");
//  if (event != null)
//    event.stopPropagation();
  updateControls();
}

//This function will add the current name to the listbox, and to the local gDialog.nameList.
//Writing to the XML file, and updating the prototype mathNameList, occurs onOK?? Or is this wrong?
function addCurrentName()
{
  var currName = document.getElementById("mathNamesBox").value;
  var theType = document.getElementById("nameTypeRadioGroup").value;
  var bEngineFunction = (document.getElementById("enginefunction").getAttribute("checked") == "true");
  var bAutoSubstitute = (document.getElementById("addAutoSubstitution").getAttribute("checked") == "true");
  var aLimitPlacement = "";
  if (theType == "operator")
    aLimitPlacement = document.getElementById("operatorLimitPlacementRadioGroup").value;
  var appearanceList = null;
//  var bEngFuncStr = bEngineFunction ? "true" : "false";
//  var bAutoSubStr = bAutoSubstitute ? "true" : "false";
//  alert("Calling to add name: [" + currName + "], of type [" + theType + "], with bEngineFunction [" + bEngFuncStr + "] and bAutoSubstitute [" + bAutoSubStr + "].\n");
  gDialog.nameList.addName(currName, theType, bEngineFunction, bAutoSubstitute, aLimitPlacement, appearanceList);
  gDialog.nameList.updateBaseList();  //see msiEditorUtilities.js

  updateControls();
}

function deleteCurrentName()
{
  var currName = document.getElementById("mathNamesBox").value;
  gDialog.nameList.deleteName(currName);
  gDialog.nameList.updateBaseList();  //see msiEditorUtilities.js

  updateControls();
}

function onOK() {
  var currName = document.getElementById("mathNamesBox").value;
  if (currName in gDialog.nameList.names)
    target = gDialog.nameList.names[currName];
  target.val = currName;
  target.type = document.getElementById("nameTypeRadioGroup").value;
  if (target.type == "operator")
    target.limitPlacement = document.getElementById("operatorLimitPlacementRadioGroup").value;

  var enginefunction = document.getElementById("enginefunction");
  if (enginefunction.getAttribute("checked") == "true")
    target.enginefunction = true;

//  gDialog.nameList.updateBaseList();  //see msiEditorUtilities.js

  var parentEditorElement = msiGetParentEditorElementForDialog(window);
  var dumpStr = "In mathmlMathName.js onOK(); inserting mathname object [" + target.val + "] in editor element [";
  if (parentEditorElement != null)
    dumpStr += parentEditorElement.id;
  dumpStr += "].\n";
  dump(dumpStr);
  var theWindow = window.opener;
  if (!theWindow || !("insertMathnameObject" in theWindow))
    theWindow = msiGetTopLevelWindow();

  theWindow.insertMathnameObject(target, parentEditorElement);

  SaveWindowLocation();
  return false;
}

function onCancel() {
  target.val = "";
  target.enginefunction = false;
  return(true);
}

