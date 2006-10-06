// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

var gBodyElement;

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
  gDialog.docClassName = data.docClassName;
  gDialog.docClassOptions = data.docClassOptions.split(',');
  gDialog.packages = new Array();
  for (var i = 0; i < data.packages.length; ++i)
  {
    gDialog.packages[i] = new Package();
    gDialog.packages[i].packageName = data.packages[i].packageName;
    gDialog.packages[i].setOptions(data.packages[i].packageOptions);
  }
//  gDialog.packages = data.packages;  //this should be an array of objects; each object will have a package name and an array of options (strings)

  gDialog.packagesInUseListbox = document.getElementById("packagesInUseListbox");
  gDialog.currentPackageDescription = document.getElementById("currentPackageDescription");
  gDialog.descCaptionTemplate = document.getElementById("currentPackageDescriptionCaption").label;

  InitDialog();

  document.getElementById("modifyButton").focus();

  SetWindowLocation();
}

function InitDialog()
{
  setDocumentClassDescription(gDialog.docClassName);
  fillPackagesListbox(gDialog.packages);
  if (gDialog.packagesInUseListbox.getRowCount() > 0)
  {
    var selItem = gDialog.packagesInUseListbox.getItemAtIndex(0);
    gDialog.packagesInUseListbox.selectItem(selItem);
    selectPackage(selItem.label);
  }
  else
    setPackageDescriptionLine(null);
  document.getElementById("optionsDescriptionBox").value=gDialog.docClassOptions.join(",");
  checkDisabledControls();

//  checkInaccessibleAcceleratorKeys(document.documentElement);

//  gDialog.tabOrderArray = new Array( gDialog.positionSpecGroup,
//                                       document.documentElement.getButton("accept"),
//                                       document.documentElement.getButton("cancel") );
  
  document.documentElement.getButton("accept").setAttribute("default", true);
}

function onAccept()
{
  data.packages = new Array();
  for (var i = 0; i < gDialog.packages.length; ++i)
  {
    data.packages[i] = new Object();
    data.packages[i].packageName = gDialog.packages[i].packageName;
    data.packages[i].packageOptions = gDialog.packages[i].packageOptions.join(",");
  }
  data.docClassOptions = gDialog.docClassOptions.join(",");

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


function setDocumentClassDescription(className)
{
  var docClassControl = document.getElementById('documentClassDescription');
  var initialClassString = docClassControl.value;
  var docClassString = initialClassString.replace(docClassControl.getAttribute('replaceTemplate'), className);
  docClassControl.value = docClassString;
}

function goNative()
{
  var packagesData = new Object();
  packagesData.packages = gDialog.packages;
  window.openDialog("chrome://prince/content/typesetNativePackages.xul", "_blank", "chrome,close,titlebar,modal", packagesData);
  if (!packagesData.Cancel)
  {
    gDialog.packages = packagesData.packages;
  }
}

function doModifyDialog()
{
  var classOptionData = new Object();
  classOptionData.isPackage = false;
  classOptionData.theName = gDialog.docClassName;
//  gDialog.selectedOptionsStr = data.optionsStr;

  classOptionData.options = gDialog.docClassOptions;
  window.openDialog("chrome://prince/content/typesetStyleOptions.xul", "_blank", "chrome,close,titlebar,modal", classOptionData);
  if (!classOptionData.Cancel)
  {
    gDialog.docClassOptions = classOptionData.options;
    document.getElementById("optionsDescriptionBox").value=gDialog.docClassOptions.join(",");
  }
}

function movePackage(bUp)
{
  var currIndex = gDialog.packagesInUseListbox.selectedIndex;
  if (currIndex < 0)
    return;
  var packageName = gDialog.packagesInUseListbox.getSelectedItem(0).label;
  var desiredIndex = currIndex + 1;
  if (bUp)
    desiredIndex = currIndex - 1;
  if (desiredIndex >= 0 && desiredIndex < gDialog.packagesInUseListbox.getRowCount())
  {
    var theItem = gDialog.packagesInUseListbox.removeItemAt(currIndex);
    var newItem = gDialog.packagesInUseListbox.insertItemAt(desiredIndex, theItem.label);
    gDialog.packagesInUseListbox.selectItem(newItem);
  }
  if (length(packageName) > 0)
  {
    currIndex = findPackageByName(gDialog.packages, packageName);
    if (currIndex >= 0)
    {
      desiredIndex = currIndex + 1;
      if (bUp)
        desiredIndex = currIndex - 1;
      if (desiredIndex >= 0 && desiredIndex < gDialog.packages.length)
      {
        var packagesItem = gDialog.packages.splice(currIndex, 1);
        gDialog.packages.splice(desiredIndex, 0, packagesItem);
      }
    }
  }
  checkDisabledControls();
}

function addPackage()
{
  var addPackageData = new Object();
  addPackageData.packages = gDialog.packages;
  addPackageData.docClassName = gDialog.docClassName;
  window.openDialog("chrome://prince/content/typesetAddPackage.xul", "_blank", "chrome,close,titlebar,modal", addPackageData);
  if (!addPackageData.Cancel)
  {
    var newPackage = addPackageData.newPackage;
    if ( (newPackage.length > 0) && (findPackageByName(gDialog.packages, newPackage) < 0) )  //if we're actually adding anything
    {
      var newPackage = new Package();
      newPackage.packageName = addPackageData.newPackage;
      gDialog.packages.push(newPackage);
      gDialog.packagesInUseListbox.selectItem( gDialog.packagesInUseListbox.appendItem(addPackageData.newPackage, '') );
      selectPackage(newPackage.packageName);
      checkDisabledControls();
    }
  }
}

function removeCurrentlySelectedPackage()
{
  var selItem = gDialog.packagesInUseListbox.getSelectedItem(0);
  removePackage(selItem);
  checkDisabledControls();
}

function removePackage(packageListItem)
{
  var packageName = packageListItem.label;
  var whichPackage = findPackageByName(gDialog.packages, packageName);
  gDialog.packages.splice(whichPackage, 1);
  whichPackage = gDialog.packagesInUseListbox.getIndexOfItem(packageListItem);
  gDialog.packagesInUseListbox.removeItemAt(whichPackage);
}

function modifyCurrentlySelectedPackage()
{
  var selItem = gDialog.packagesInUseListbox.getSelectedItem(0);
  modifyPackage(selItem.label);
}

function modifyPackage(packageName)
{
  var whichPackage = findPackageByName(gDialog.packages, packageName);
  var packageData = new Object();
  packageData.isPackage = true;
  packageData.options = null;
  if (whichPackage >= 0)
  {
    packageData.options = gDialog.packages[whichPackage].packageOptions;
    packageData.theName = gDialog.packages[whichPackage].packageName;
  }

  window.openDialog("chrome://prince/content/typesetStyleOptions.xul", "_blank", "chrome,close,titlebar,modal", packageData);
  if (!packageData.Cancel)
  {
    gDialog.packages[whichPackage].setOptions( packageData.options );
    setPackageDescriptionLine(gDialog.packages[whichPackage]);
  }
}

//Function moved to typesetDialogUtils.js, as it can be generally used in the child dialogs.
//function findPackageByName(packageList, packageName)
//{
//  var theIndex = -1;
//  for (var i = 0; i < packageList.length; ++i)
//  {
//    if (packageList[i].packageName == packageName)
//    {
//      theIndex = i;
//      break;
//    }
//  }
//  return theIndex;
//}

function fillPackagesListbox(packageList)
{
  for (var i = 0; i < packageList.length; ++i)
  {
    gDialog.packagesInUseListbox.appendItem(packageList[i].packageName, packageList[i].packageOptions);
  }
}


function checkDisabledControls()
{
  var numPackages = gDialog.packagesInUseListbox.getRowCount();
  var selectedIndex = gDialog.packagesInUseListbox.selectedIndex;
  if (selectedIndex < 0)
  {
    document.getElementById('moveUpButton').setAttribute('disabled','true');
    document.getElementById('moveDownButton').setAttribute('disabled','true');
    document.getElementById('removePackageButton').setAttribute('disabled','true');
    document.getElementById('modifyPackageButton').setAttribute('disabled','true');
  }
  else
  {
    document.getElementById('removePackageButton').setAttribute('disabled','false');
    document.getElementById('modifyPackageButton').setAttribute('disabled','false');
    if (selectedIndex >0)
      document.getElementById('moveUpButton').setAttribute('disabled','false');
    else
      document.getElementById('moveUpButton').setAttribute('disabled','true');
    if (selectedIndex < numPackages)
      document.getElementById('moveDownButton').setAttribute('disabled','false');
    else
      document.getElementById('moveDownButton').setAttribute('disabled','true');
  }
}

function selectPackage(packageName)
{
  var whichPackage = findPackageByName(gDialog.packages, packageName);
  var thePackage = null;
  if (whichPackage >= 0)
    thePackage = gDialog.packages[whichPackage];
  if (thePackage == null)
    alert("Package " + packageName + " not found in package list!");
  setPackageDescriptionLine(thePackage);
  checkDisabledControls();
}

function setPackageDescriptionLine(thePackage)
{
  if (thePackage == null)
  {
    document.getElementById("currentPackageDescriptionCaption").label = "";
    gDialog.currentPackageDescription.value = "";
  }
  else
  {
    var caption = document.getElementById("currentPackageDescriptionCaption");
    caption.label = gDialog.descCaptionTemplate.replace(caption.getAttribute("replaceTemplate"), thePackage.packageName);
    gDialog.currentPackageDescription.value = thePackage.getOptionsStr();
  }
}