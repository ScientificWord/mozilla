// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

var gBodyElement;

//var ourRDFRoot = "urn:texoptions:__Type__:__Name__:options";

const emptyElementStr=" ";

//const mmlns    = "http://www.w3.org/1998/Math/MathML";
//const xhtmlns  = "http://www.w3.org/1999/xhtml";

var data;

// dialog initialization code
function Startup()
{
//  var editor = GetCurrentEditor();
  var editorElement = msiGetParentEditorElementForDialog(window);
  var editor = msiGetEditor(editorElement);
  if (!editor) {
    window.close();
    return;
  }

  doSetOKCancel(onAccept, onCancel);
  data = window.arguments[0];
  data.Cancel = false;
  gDialog.bInternalMessage = false;
  gDialog.isPackage = data.isPackage;
  gDialog.theName = data.theName;
  gDialog.theOptions = data.optionSet;
//  gDialog.selectedOptionsStr = data.optionsStr;

  gDialog.selectedOptionsArray = data.options;
  removeEmptyElements(gDialog.selectedOptionsArray);
  gDialog.currSelectedChoices = new Array();

  gDialog.optionsListbox = document.getElementById("categoryListbox");
  gDialog.choicesListbox = document.getElementById("optionsListbox");
  gDialog.currentOptionsDescription = document.getElementById("selectedOptionsDescription");

  var ourRDFRoot = document.title;
  if (gDialog.isPackage)
    ourRDFRoot = ourRDFRoot.replace("__type__", "package");
  else
    ourRDFRoot = ourRDFRoot.replace("__type__", "class");
  document.title = ourRDFRoot.replace("__Name__", gDialog.theName);

  InitDialog();

//  gDialog.optionsListbox.focus();
  msiSetInitialDialogFocus(gDialog.optionsListbox);

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
  window.openDialog("chrome://prince/content/typesetNativeStyleOptions.xul", "nativestyleoptions", "chrome,close,resizable,titlebar,modal", optionsData);
  if (!optionsData.Cancel)
  {
    gDialog.selectedOptionsArray = optionsData.options;
    gDialog.currentOptionsDescription.value = gDialog.selectedOptionsArray.join(",");
  }
  else {
		msiGetEditor(editorElement).incrementModificationCount(1);
	}

}

function itemIsDefaultChoice(anItem)
{
  return false;
//  var bDefault = false;
//  if (anItem.label.search(/\s*\-\s*default/i) >= 0)
//    bDefault = true;
//  else if (anItem.value.length <= 0)
//    bDefault = true;
//  return bDefault;
}

function findDefaultOption()
{
  var nDefault = -1;
//  for (var i = 0; (nDefault < 0) && (i < gDialog.choicesListbox.getRowCount()); ++i)
//  {
//    if ( itemIsDefaultChoice(gDialog.choicesListbox.getItemAtIndex(i)) )
//      nDefault = i;
//  }
  return nDefault;
}

function setSelectedOptionChoice()
{
  var nFound = -1;
  var nMatched = -1;
  var bFound = false;
  var nDefault = findDefaultOption();
  for (var i = 0; i < gDialog.choicesListbox.getRowCount(); ++i)
  {
    var thisItem = gDialog.choicesListbox.getItemAtIndex(i);
    if (thisItem.value.length > 0)
    {
      nFound = findInArray(gDialog.selectedOptionsArray, thisItem.value);
      if (nFound >= 0)
	    {
		    bFound = true;
        if ((gDialog.currSelectedChoices.indexOf(thisItem.value) < 0) )
          gDialog.currSelectedChoices.push( thisItem.value );
		    gDialog.choicesListbox.addItemToSelection(thisItem);
	    }
    }
  }
//  if (!bFound)
//  {
//    if (nDefault >= 0)
//    {
//	    var theItem = gDialog.choicesListbox.getItemAtIndex(nDefault);
//      //gDialog.currSelectedChoices.push( theItem.value );	if this is the default option, we don't want to add anything to the string
//      gDialog.choicesListbox.addItemToSelection( theItem );
//    }
//  }
}


function changeOptionSelection()
{
  fillChoicesListbox();
  setSelectedOptionChoice();
}

//This should be called only once per invocation of the dialog, during initialization. (What if this dialog is modeless?)
function fillOptionsListbox()
{
  gDialog.bInternalMessage = true;
  var items = gDialog.optionsListbox.getElementsByTagName("listitem");
  for (var i = items.length - 1; i >= 0; --i)
    gDialog.optionsListbox.removeChild(items[i]);
  for (var anOption in gDialog.theOptions)
  {
    gDialog.optionsListbox.appendItem(anOption, anOption);
  }
//  gDialog.optionsListbox.setAttribute("ref", ourRDFRoot);	 //global variable defined at top
//  gDialog.optionsListbox.builder.rebuild();  //Need this??
  gDialog.bInternalMessage = false;
}

function fillChoicesListbox()
{
  gDialog.bInternalMessage = true;
  var selItem = gDialog.optionsListbox.getSelectedItem(0);
  var nFound = -1;
  var anOption;

  gDialog.currSelectedChoices.splice(0, gDialog.currSelectedChoices.length);  //empty the "currSelectedChoices" array
  var items = gDialog.choicesListbox.getElementsByTagName("listitem");
  for (var i = items.length - 1; i >= 0; --i)
    gDialog.choicesListbox.removeChild(items[i]);

  for (var aChoice in gDialog.theOptions[selItem.label].mChoices)
  {
    anOption = gDialog.theOptions[selItem.label].mChoices[aChoice];
    gDialog.choicesListbox.appendItem(aChoice, anOption.mChoiceData);
    nFound = findInArray(gDialog.selectedOptionsArray, anOption.mChoiceData);
    if (nFound >= 0)
      gDialog.currSelectedChoices.push( anOption.mChoiceData );
  }
  gDialog.bInternalMessage = false;
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

function diffArrays(oldArray, newArray)
{
  var retObj = {removeArray : [], addArray : []};
  for (var jx = 0; jx < newArray.length; ++jx)
    retObj.addArray.push( newArray[jx] );
  var foundInNew = -1;
  for (var ix = 0; ix < oldArray.length; ++ix)
  {
    if ((foundInNew = retObj.addArray.indexOf(oldArray[ix])) < 0)
      retObj.removeArray.push( oldArray[ix] );  //Does need to be removed
    else
      retObj.addArray.splice( foundInNew, 1 );  //Doesn't need to be added
  }
  return retObj;
}

function removeEmptyElements(stringArray)
{
  if (stringArray) {
    for (var ix = stringArray.length - 1; ix >= 0; --ix)
    {
      if (!stringArray[ix] || stringArray.length == 0)
        stringArray.splice(ix, 1);
    }
  }
}

function replaceStringsInArray(theArray, oldStrings, newStrings)
{
  var bFound = false;
  var removePos = -1;
  var newArray = new Array();
  var diffObj = diffArrays(oldStrings, newStrings);
  for (var i = theArray.length - 1; (i >= 0) && (diffObj.removeArray.length > 0); --i)
  {
	  if ( (removePos = diffObj.removeArray.indexOf(theArray[i])) >= 0 )  //we've arrived at one to remove   
	  {
      if (diffObj.addArray.length > 0)
      {
        theArray.splice( i, 1, diffObj.addArray[0] );
        diffObj.addArray.splice( 0, 1 );
      }
      else
        theArray.splice( i, 1 );
      diffObj.removeArray.splice( removePos, 1 );
	  }
  }
  theArray = theArray.concat( diffObj.addArray );
  return theArray;
}

function changeOptionChoice()
{
  if (gDialog.bInternalMessage)
    return;

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
