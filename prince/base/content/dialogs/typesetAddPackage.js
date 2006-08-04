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
  gDialog.packages = data.packages;  //this should be an array of objects; each object will have a package name and an array of options (strings)

  gDialog.packagesListbox = document.getElementById("packagesListbox");

  InitDialog();

  gDialog.packagesListbox.focus();

  SetWindowLocation();
}

function InitDialog()
{
  document.getElementById("classNameDescription").text = gDialog.docClassName;
  var rdfRef = gDialog.packagesListbox.getAttribute('ref');
  gDialog.packagesListbox.setAttribute('ref', rdfRef.replace('__className__', gDialog.docClassName));
  removePackagesInUseFromListbox()
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

function removePackagesInUseFromListbox()
{
  var nPackages = gDialog.packagesListbox.getRowCount();
  for (var i = nPackages-1; i >= 0; --i)  //traverse in reverse so as not to access already deleted items
  {
    if (findPackageByName(gDialog.packagesInUse, gDialog.packagesListbox.getItem(i).label) >= 0)
      gDialog.packagesListbox.deleteItem(i);
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
