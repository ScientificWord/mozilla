// Copyright (c) 2004 MacKichan Software, Inc.  All Rights Reserved.
Components.utils.import("resource://app/modules/mathnamedictionary.jsm");

var node;
var saveNode;

function parseBool(s){
  return (s==="true"); 
}

/*
Not needed as long as we are happy with the case-sensitive sorting of autocompletestringarray,
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
    let inputData = window.arguments[0];
    if ("reviseObject" in inputData)
    {
      node = inputData.reviseObject;
      saveNode = node;
    }
    else node = window.arguments[0];
    let name = "";
    let lp;  // limit placement: atRight, aboveBelow, or auto
    let placementString;
    let movableLimits = null
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
      if (node.hasAttribute("type")) nameData.type = node.getAttribute("type").charAt(0);
      if (node.hasAttribute("builtin")) nameData.builtin = node.getAttribute("builtin");
      if (node.hasAttribute("msiLimitPlacement")) {
        lp = node.getAttribute("msiLimitPlacement");
        if (lp === 'msiLimitsAtRight') placementString = 'atRight';
        else if (lp === 'msiLimitsAboveBelow') placementString = 'aboveBelow';
        else {
          movableLimits = 'true';
          placementString = 'auto';
        }
      }
      if (node.hasAttribute("enginefunction")) nameData.engine = node.getAttribute("enginefunction")==="true"?true:false;
      if (node.hasAttribute("size")) nameData.size = node.getAttribute("size");
    }
    if (nameData.val) document.getElementById('mathNamesBox').value = nameData.val;
    if (nameData.type) document.getElementById('nameTypeRadioGroup').value = nameData.type;
    document.getElementById('operatorLimitPlacementRadioGroup').disabled = nameData.type !== 'o';
    document.getElementById('enginefunction').checked = nameData.engine;
    document.getElementById('operatorLimitPlacementRadioGroup').value = placementString;
  }
  catch(e){
    dump("Error: "+e.toString()+"\n");
  }
}

function currentName() {
  return document.getElementById("mathNamesBox").value;
}




//updateControls() here needs to disable the limitPlacementGroup if the type isn't operator. If the selected name
//  is already in the list, the Add button should be disabled. If not, the Delete button should be. Also, the "Add automatic
//  substitution" checkbox should be disabled if it's already in that list.
//(Q. Do we want to also disable deleting certain core MathNames? This seems to have been done in SWP, so I guess so.
//  How are they to be identified? Or should there be two files containing MathNames, one for "user" names and one for
//  "built-in" ones? Solution proposed for now will be to use one file, and mark built-in ones as such.?)
function updateControls()
{
  var currName = currentName();
  var bIsNew = namesdict.getNameData(currName) == null;
  var type = document.getElementById('nameTypeRadioGroup').value;
  var isOperator = type === 'o';
  if (isOperator) {
    document.getElementById('limitPlacementGroup').removeAttribute('disabled');
  } else {
    document.getElementById('limitPlacementGroup').setAttribute('disabled','true');
  }
  if (currName == null || currName.length === 0) {
    document.getElementById('deleteButton').setAttribute('disabled','true');
  } else {
    document.getElementById('deleteButton').removeAttribute('disabled');
  }
}


function checkKeyPressEvent(control, theEvent)
{
}

var defaultNameData = {
  val: '',
  type: 'v',
  builtin: null,
  engine: false,
  // movable limits?
  size: null
};

function setDialogDefaults(nameData) {
  // when a new name is selected (or when the dialog is initialized) all
  // fields other than the name should have initial values saved.
  if (!nameData) nameData = defaultNameData;
  if (nameData.val) document.getElementById('mathNamesBox').value = nameData.val;
  if (nameData.type) document.getElementById('nameTypeRadioGroup').value = nameData.type;
  if (nameData.lp) document.getElementById('operatorLimitPlacementRadioGroup').value = nameData.lp;
  document.getElementById('enginefunction').checked = !!nameData.engine;
}


function deleteCurrentName()
{

  // BBM: needs work
  var currName = currentName();
  namesdict.remove(currName);
  document.getElementById('mathNamesBox').value = '';
  setDialogDefaults(null);
  // updateControls();
}

function isBuiltIn(name) {
  try {
    return namesdict.getNameData(name).builtin;
  }
  catch(e) {
    return false;
  }
}

function setCurrentName() {
  // save the current mathname info in the math name list
  // so it will get included for save. This also returns itn
  // current info for onOk.
  let type = document.getElementById('nameTypeRadioGroup').value;
  let limplacement = document.getElementById("operatorLimitPlacementRadioGroup").value;
  let name = document.getElementById('mathNamesBox').value;
  let engine;
  if (type === 'o') {
    engine = document.getElementById("enginefunction").checked;
  }
  else engine = null;
  return namesdict.setNameData(name, type, isBuiltIn(name), limplacement, engine);
}

function save() {
  try {
    setCurrentName();
    namesdict.save(); 
    writeStatusMessage("Complete math name list saved");    
  }
  catch(e) {
  }
}

function onOK() {
  let nameData = setCurrentName();
  var parentEditorElement = msiGetParentEditorElementForDialog(window);
  if (saveNode) {
    reviseMathname(saveNode, nameData, parentEditorElement);
  } else {
    insertMathname(nameData.val);
  }
  return true;
}


function onCancel() {
  return(true);
}

