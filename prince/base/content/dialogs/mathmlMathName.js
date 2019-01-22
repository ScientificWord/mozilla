// Copyright (c) 2004 MacKichan Software, Inc.  All Rights Reserved.
Components.utils.import("resource://app/modules/mathnamedictionary.jsm");

var node;

function parseBool(s){
  return (s==="true"); 
}

/*
Not needed as long as we are happy with the case-sensitive sorting of autocompletestringarrat,
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
*/

function Startup() {
  try {
    namesdict.init();
    let node = window.arguments[0];
    let name = "";
    if (typeof node == "string") 
      name = node.val;
    if (!node || (node.nodeName !== 'mi' && node.nodeName !== 'mo'))
      node = null;
    if (node && node.textContent)
      name = node.textContent;
    let nameData = namesdict.getNameData(name);
    // nameData is null, or is the default data, if any, for the given name.
    if (!nameData) {
      nameData = {};
      if (name) nameData = { val: name};
    }
    // Now copy node attributes to nameData, when they exist
    if (node) {
      if (node.hasAttribute("type")) nodeData.type = node.getAttribute("type").charAt(0);
      if (node.hasAttribute("builtin")) nodeData.builtin = node.getAttribute("builtin");
      if (node.hasAttribute("lp")) nodeData.lp = node.getAttribute("lp");
      if (node.hasAttribute("enginefunction")) nodeData.engine = node.getAttribute("enginefunction")==="true"?true:false;
      if (node.hasAttribute("movableLimits")) nodeData.movableLimits = node.getAttribute("movableLimits");
      if (node.hasAttribute("size")) nodeData.size = node.getAttribute("size");
    }
    // Now set up the dialog contents
    if (nameData.val) document.getElementById(mathNamesBox).value = nameData.val;
    if (nameData.type) document.getElementById(nameTypeRadioGroup).value = nameData.type;
    if (nameData.type === 'o') { // enable operator-only stuff
      document.getElementById('operatorLimitPlacementRadioGroup').disabled = false;
    }
    document.getElementById('enginefunction').checked = nameData.engine;

  }
  catch(e){
    dump("Error: "+e.toString()+"\n");
  }
}

// function isPropertiesDialog()
// {
//   return ( ("reviseObject" in inputData) && (inputData.reviseObject != null) );
// }

// function getPropertiesDialogTitle()
// {
//   return document.getElementById("propertiesTitle").value;
// }

// function getDataFromPropertiesObject(reviseObject)
// {
//   var mathNameNode = msiNavigationUtils.getWrappedObject(reviseObject, "mathname");
//   if (!mathNameNode)
//     return null;

//   var retObject = new Object();
//   if ("msimathnameText" in reviseObject)
//     retObject.val = reviseObject.msimathnameText;
//   else if ( (mathNameNode != reviseObject) && ("msimathnameText" in mathNameNode) )
//     retObject.val = mathNameNode.msimathnameText;
//   else  //get it from mathNameObject?
//     retObject.val = msiNavigationUtils.getLeafNodeText(mathNameNode);

//   var styleVals = new Object();
//   styleVals["displaystyle"] = "";
//   var foundAttrs = new Object();
//   var styleNode = msiNavigationUtils.findStyleEnclosingObj(mathNameNode, "mathname", styleVals, foundAttrs);

//   switch(mathNameNode.nodeName)
//   {
//     case "mo":
//       retObject.type = "operator";
//       retObject.limitPlacement = "auto";
//       //Now work out the "size" and "limitplacement" stuff:
//       if (mathNameNode.hasAttribute("msiLimitPlacement"))
//       {
//         if (mathNameNode.getAttribute("msiLimitPlacement") == "msiLimitsAtRight")
//           retObject.limitPlacement = "atRight";
//         else if (mathNameNode.getAttribute("msiLimitPlacement") == "msiLimitsAboveBelow")
//           retObject.limitPlacement == "aboveBelow";
//       }
//       if ("displaystyle" in foundAttrs)
//       {
//         if (foundAttrs.displaystyle == "true")
//           retObject.size = "big";
//         else
//           retObject.size = "small";
//       }
//     break;

//     case "mi":
//       retObject.type = "function";
//       if ( ("msiclass" in mathNameNode) && (mathNameNode.msiclass == "enginefunction") )
//         retObject.enginefunction = "true";
//     break;
    
//     default:
//     //what goes here?? Assumption should be that this is "appearance" case (that is, a complex mathname object).
//     //However, this case should now be obsolete. Nothing goes here????
// //      retObject.appearance = reviseObject.ownerDocument.createElement("appearance");
// //      retObject.appearance.appendChild( mathNameNode.cloneNode(true) );
//     break;
//   }

//   return retObject;
// }

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
  var bIsNew = namesdict.getNameData(currName) != null;
  var nameTypeControls = ["nameTypeGroup", "nameTypeLabel", "nameTypeRadioGroup"];
  // enableControlsByID(nameTypeControls, bIsNew);
  var limitPlacementControls = ["limitPlacementGroup", "limitPlacementGroupCaption", "operatorLimitPlacementRadioGroup"];
  if (theType === "operator" || theType === "function")
  {
    //document.getElementById("limitPlacementGroup").removeAttribute("disabled");
    enableControlsByID(limitPlacementControls, true);
    var thePlacement = "auto";
    if ((currName in gDialog.nameList.names) && ("limitPlacement" in gDialog.nameList.names[currName]))
      thePlacement = gDialog.nameList.names[currName].limitPlacement;
    document.getElementById("operatorLimitPlacementRadioGroup").value = thePlacement;
  }
  else
  {
    //document.getElementById("limitPlacementGroup").setAttribute("disabled", "true");
    enableControlsByID(limitPlacementControls, false);
  }

  enableControlsByID(["addButton"], bIsNew);
  enableControlsByID(["deleteButton"], namesdict.getNameData(currName) != null);

  var bAutoSub = gDialog.nameList.hasAutoSubstitution(currName);
  if (bAutoSub)
    document.getElementById("addAutoSubstitution").setAttribute("checked", "true");
  else
    document.getElementById("addAutoSubstitution").removeAttribute("checked");
//  var bBuiltIn = gDialog.nameList.isBuiltIn(currName);
//  enableControlsByID(["addAutoSubstitution", "addAutoSubstitutionDescription", "enginefunction", "enginefunctionDescription"], !bBuiltIn);
//  enableControlsByID(["addAutoSubstitution", "addAutoSubstitutionDescription", "enginefunction", "enginefunctionDescription"], !bBuiltIn);
  // enableControlsByID(["addAutoSubstitution", "addAutoSubstitutionDescription", "enginefunction", "enginefunctionDescription"], bIsNew);
  var dumpStr = "autosubstitution and enginefunction should be ";
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
}
//if (!theEvent.altKey)
//{
//  if (theEvent.keyCode==KeyEvent.DOM_VK_RETURN)
//  {
//    if (gDialog.bStopNextEnter)
//    {
//      gDialog.bStopNextEnter = false;
//      if (!control)
//      {
//        dump("Null control in checkKeyPressEvent!\n");
//        control = document.getElementById("mathNamesBox");
//      }
//      control.controller.handleEnter(false);
//      theEvent.stopPropagation();
//      theEvent.preventDefault();
//      dumpStr += "called theEvent.stopPropagation().\n";
//    }
//  }
//  else //now we're typing into the name field, so we assume we should stop the next enter from accepting the dialog
//  {
//    gDialog.bStopNextEnter = true;
//    dumpStr += "setting gDialog.bStopNextEnter.\n";
//  }
//}
//dump(dumpStr);
////Now hopefully continue processing as usual.
//}

function changeName(currName)
{
  var nameObject = null;
  if (currName in gDialog.nameList.names)
    nameObject = gDialog.nameList.names[currName];
  setControlsFromObject(nameObject);
}

function setControlsFromObject(nameData)
{
//  var currName = document.getElementById("mathNamesBox").value;
  var theType = document.getElementById("nameTypeRadioGroup").value;
  var isEngineFunction = false;
  if (nameData != null)
  {
    theType = nameData.type;
    if (("engineFunction" in nameData) && (nameData.engineFunction == true))
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
function addCurrentName(bNoUpdate)
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

  if (!bNoUpdate)
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
  namesdict.setNameData(currName, 
    document.getElementById('nameTypeRadioGroup').value,
    null,
    document.getElementById("operatorLimitPlacementRadioGroup").value,
    document.getElementById("enginefunction").checked );
 
  var parentEditorElement = msiGetParentEditorElementForDialog(window);
  var theWindow = window.opener;
    theWindow.insertMathname(currName, parentEditorElement);

  SaveWindowLocation();
  return true;
//  return bRevise;
}


function onCancel() {
  return(true);
}

