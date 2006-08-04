// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

var gBodyElement;

var ourRDFRoot = "urn:texoptions:__Type__:__Name__:options";

const emptyElementStr=" ";

//const mmlns    = "http://www.w3.org/1998/Math/MathML";
const xhtmlns  = "http://www.w3.org/1999/xhtml";

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
  gDialog.isPackage = data.isPackage;
  gDialog.theName = data.theName;
//  gDialog.selectedOptionsStr = data.optionsStr;

  gDialog.selectedOptionsArray = data.options;
  gDialog.currSelectedChoices = new Array();

  gDialog.optionsListbox = document.getElementById("categoryListbox");
  gDialog.choicesListbox = document.getElementById("optionsListbox");
  gDialog.currentOptionsDescription = document.getElementById("selectedOptionsDescription");

  if (gDialog.isPackage)
    ourRDFRoot = ourRDFRoot.replace("__Type__", "package");
  else
    ourRDFRoot = ourRDFRoot.replace("__Type__", "class");
  ourRDFRoot = ourRDFRoot.replace("__Name__", gDialog.theName);

  InitDialog();

  gDialog.optionsListbox.focus();

  SetWindowLocation();
}

function InitDialog()
{
  setWindowTitle();
  fillOptionsListbox();
  if (gDialog.optionsListbox.getRowCount() > 0)
  {
	gDialog.optionsListbox.selectedIndex = 0;
    changeOptionSelection();
  }
//  checkDisabledControls();

//  checkInaccessibleAcceleratorKeys(document.documentElement);

//  gDialog.tabOrderArray = new Array( gDialog.positionSpecGroup,
//                                       document.documentElement.getButton("accept"),
//                                       document.documentElement.getButton("cancel") );
  
  document.documentElement.getButton("accept").setAttribute("default", true);
}

function onAccept()
{
  data.options = gDialog.selectedOptionsArray;

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


function setWindowTitle(packageName)
{
  var theTitle = document.documentElement.getAttribute("title");
  var strBundle = document.getElementById("titleStrings");
  var replaceNameStr = strBundle.getString("ClassOrPackageNameReplaceTemplate");
  var replaceTypeStr = strBundle.getString("ClassOrPackageReplaceTemplate");
  theTitle = theTitle.replace(replaceNameStr, gDialog.theName);
  var typeStr = strBundle.getString("ClassTypeString");
  if (gDialog.isPackage)
	typeStr = strBundle.getString("PackageTypeString");
  theTitle = theTitle.replace(replaceTypeStr, typeStr);
  document.documentElement.setAttribute("title", theTitle);
}

function doGoNative()
{
  var optionsData = new Object();
  optionsData.options = gDialog.selectedOptionsArray;
  window.openDialog("chrome://prince/content/typesetNativeStyleOptions.xul", "_blank", "chrome,close,titlebar,modal", optionsData);
  if (!optionsData.Cancel)
  {
    gDialog.selectedOptionsArray = optionsData.options;
  }
}

function findDefaultOption()
{
  var nDefault = -1;
  for (var i = 0; (nDefault < 0) && (i < gDialog.choicesListbox.getRowCount()); ++i)
  {
    if (gDialog.choicesListbox.getItemAtIndex(i).value.length <= 0)
      nDefault = i;
    else if (gDialog.choicesListbox.getItemAtIndex(i).label.search(/\s*\-\s*default/i) >= 0)
      nDefault = i;
  }
  return nDefault;
}

function setSelectedOptionChoice()
{
  var nFound = -1;
  var nMatched = -1;
  var bFound = false;
  for (var i = 0; i < gDialog.choicesListbox.getRowCount(); ++i)
  {
    var thisItem = gDialog.choicesListbox.getItemAtIndex(i);
    if (thisItem.value.length > 0)
    {
      nFound = findInArray(gDialog.selectedOptionsArray, thisItem.value);
      if (nFound >= 0)
	  {
		bFound = true;
        gDialog.currSelectedChoices.push( thisItem.value );
		gDialog.choicesListbox.addItemToSelection(thisItem);
	  }
    }
  }
  if (!bFound)
  {
    nFound = findDefaultOption();
    if (nFound >= 0)
    {
	  var theItem = gDialog.choicesListbox.getItemAtIndex(nFound);
      //gDialog.currSelectedChoices.push( theItem.value );	if this is the default option, we don't want to add anything to the string
      gDialog.choicesListbox.addItemToSelection( theItem );
    }
  }
}


function changeOptionSelection()
{
  fillChoicesListbox();
  setSelectedOptionChoice();
}

//This should be called only once per invocation of the dialog, during initialization. (What if this dialog is modeless?)
function fillOptionsListbox()
{
  gDialog.optionsListbox.setAttribute("ref", ourRDFRoot);	 //global variable defined at top
  gDialog.optionsListbox.builder.rebuild();  //Need this??
}

function fillChoicesListbox()
{
  var selItem = gDialog.optionsListbox.getSelectedItem(0);
  var newRDFRoot = ourRDFRoot;
  newRDFRoot += ":" + selItem.value + ":values";
  if (newRDFRoot == gDialog.choicesListbox.getAttribute("ref"))
    return;

  gDialog.currSelectedChoices.splice(0, gDialog.currSelectedChoices.length);  //empty the "currSelectedChoices" array
  if (newRDFRoot.length <= 0)
    alert("Empty value string for option " + selItem.label);
  gDialog.choicesListbox.setAttribute("ref", newRDFRoot);
  gDialog.choicesListbox.builder.rebuild();
}

//function removeStringFromArray(theArray, theString)
//{
//  if (theArray.length > 0)
//  {
//    var nFound = theArray.indexOf(theString);
//    if (nFound >= 0)
//      theArray.splice(nFound, 1);
//  }
//}

function replaceStringsInArray(theArray, oldStrings, newStrings)
{
  var bFound = false;
  var newArray = new Array();
  for (var i = 0; i < theArray.length; ++i)
  {
	if (findInArray(oldStrings, theArray[i]) >= 0)  //we've arrived at where the old ones were
	{
	  if (!bFound)	//insert the replacement strings here
	  {
		bFound = true;
		newArray = newArray.concat(newStrings);
	  }
	}
	else
	  newArray.push(theArray[i]);
  }
  if (!bFound)
    theArray = newArray.concat(newStrings);
  else
    theArray = newArray;
  return theArray;
}

function changeOptionChoice()
{
  var stringArray = new Array();
  for (var i = 0; i < gDialog.choicesListbox.selectedItems.length; ++i)
  {
	if (gDialog.choicesListbox.selectedItems[i].value.length > 0)
      stringArray.push(gDialog.choicesListbox.selectedItems[i].value);
  }
  gDialog.selectedOptionsArray = replaceStringsInArray(gDialog.selectedOptionsArray, gDialog.currSelectedChoices, stringArray);
  gDialog.currSelectedChoices = stringArray;
  gDialog.currentOptionsDescription.value = gDialog.selectedOptionsArray.join(",");
}

//function checkDisabledControls()
//{
//  var numPackages = gDialog.packagesInUseListbox.getRowCount();
//  var selectedIndex = gDialog.packagesInUseListbox.selectedIndex;
//  if (selectedIndex < 0)
//  {
//    document.GetElementById('moveUpButton').setAttribute('disabled','true');
//    document.GetElementById('moveDownButton').setAttribute('disabled','true');
//    document.GetElementById('removePackageButton').setAttribute('disabled','true');
//    document.GetElementById('modifyPackageButton').setAttribute('disabled','true');
//  }
//  else
//  {
//    document.GetElementById('removePackageButton').setAttribute('disabled','false');
//    document.GetElementById('modifyPackageButton').setAttribute('disabled','false');
//    if (selectedIndex >0)
//      document.GetElementById('moveUpButton').setAttribute('disabled','false');
//    else
//      document.GetElementById('moveUpButton').setAttribute('disabled','true');
//    if (selectedIndex < numPackages)
//      document.GetElementById('moveDownButton').setAttribute('disabled','false');
//    else
//      document.GetElementById('moveDownButton').setAttribute('disabled','true');
//  }
//}
