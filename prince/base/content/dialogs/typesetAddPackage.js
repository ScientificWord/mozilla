// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

var gBodyElement;

const emptyElementStr=" ";

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
  data.Cancel = false;
  gDialog.docClassName = data.docClassName;
  gDialog.packagesInUse = data.packages;  //this should be an array of package objects
  gDialog.packagesAvailable = data.packagesAvailable;

  gDialog.packagesListbox = document.getElementById("packagesListbox");

  InitDialog();

//  gDialog.packagesListbox.focus();
  msiSetInitialDialogFocus(gDialog.packagesListbox);

  SetWindowLocation();
}

function InitDialog()
{
  document.getElementById("classNameDescription").text = gDialog.docClassName;
  fillPackagesAvailableListbox();
  removePackagesInUseFromListbox();
  checkDisabledControls();

//  checkInaccessibleAcceleratorKeys(document.documentElement);

//  gDialog.tabOrderArray = new Array( gDialog.positionSpecGroup,
//                                       document.documentElement.getButton("accept"),
//                                       document.documentElement.getButton("cancel") );
  
  document.documentElement.getButton("accept").setAttribute("default", true);
}

function onAccept()
{
  data.newPackage = gDialog.packagesListbox.getSelectedItem(0).label;

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


function findPackageByName(packageList, packageName)
{
  var theIndex = -1;
  for (var i = 0; i < packageList.length; ++i)
  {
    if (packageList[i].packageName == packageName)
    {
      theIndex = i;
      break;
    }
  }
  return theIndex;
}

function fillPackagesAvailableListbox()
{
  var lcComp = function(a,b)
  { return a.toLowerCase().localeCompare(b.toLowerCase());  };

  gDialog.packagesAvailable.sort(lcComp);

  var items = gDialog.packagesListbox.getElementsByTagName("listitem");
  for (var i = items.length - 1; i >= 0; --i)
    gDialog.packagesListbox.removeChild(items[i]);

  for (var ix = 0; ix < gDialog.packagesAvailable.length; ++ix)
    gDialog.packagesListbox.appendItem(gDialog.packagesAvailable[ix]);
}

function removePackagesInUseFromListbox()
{
  var nPackages = gDialog.packagesListbox.getRowCount();
  var pkgName;
  for (var i = nPackages-1; i >= 0; --i)  //traverse in reverse so as not to access already deleted items
  {
    if (gDialog.packagesListbox.getItemAtIndex(i))
    {
      pkgName = gDialog.packagesListbox.getItemAtIndex(i).label;
      if (findPackageByName(gDialog.packagesInUse, pkgName) >= 0)
        gDialog.packagesListbox.removeItemAt(i);
    }
  }
}


function checkDisabledControls()
{
  var selectedIndex = gDialog.packagesListbox.selectedIndex;
  if (selectedIndex < 0)
    document.documentElement.getButton('accept').setAttribute('disabled', 'true');
  else
    document.documentElement.getButton('accept').removeAttribute('disabled');
}
