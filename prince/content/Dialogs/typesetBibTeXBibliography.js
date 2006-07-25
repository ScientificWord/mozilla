// Copyright (c) 2006 MacKichan Software, Inc.  All Rights Reserved.


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
  gDialog.databaseFile = data.databaseFile;  //a string representation of file path
  gDialog.styleFile = data.styleFile;  //a string representation of file path
  gDialog.bibTeXDir = null; //nsiLocalFile

  InitDialog();

  document.getElementById("databaseFileListbox").focus();

  SetWindowLocation();
}

function InitDialog()
{
  fillDatabaseFileListbox();
  fillStyleFileListbox();
//  checkDisableControls();

//  checkInaccessibleAcceleratorKeys(document.documentElement);

//  gDialog.tabOrderArray = new Array( gDialog.positionSpecGroup,
//                                       document.documentElement.getButton("accept"),
//                                       document.documentElement.getButton("cancel") );
  
  document.documentElement.getButton("accept").setAttribute("default", true);
}

function onAccept()
{
  var databaseItem = document.getElementById("databaseFileListbox").getSelectedItem(0);
  if (databaseItem)
    data.databaseFile = databaseItem.value;

  var styleItem = document.getElementById("styleFileListbox").getSelectedItem(0);
  if (styleItem)
    data.styleFile = styleItem.value;

  if (gDialog.bibTeXDir && gDialog.bibTeXDir.exists())
  {
    var prefs = Components.classes["@mozilla.org/preferences-service;1"].getService(Components.interfaces.nsIPrefBranch);
    try
    {
      prefs.setComplexValue("prince.bibtex.dir", Components.interfaces.nsILocalFile, gDialog.bibTeXDir);
    }
    catch(exception) {}
  }   

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


//function selectDatabaseFile(selectedListboxItem)
//{
//  if (selectedListboxItem)
//    gDialog.databaseFile = selectedListboxItem.label;
//  else
//    gDialog.databaseFile = "";
//  checkDisableControls();
//}

//function checkDisableControls()
//{
////  var selDBase = document.getElementById("databaseFileListbox").selectedItem;
//  if (gDialog.databaseFile.length > 0)
//    document.getElementById("viewKeysButton").removeAttribute("disabled");
//  else
//    document.getElementById("viewKeysButton").setAttribute("disabled", "true");
//  if (document.getElementById("keyTextbox").value.length > 0)
//    document.documentElement.getButton('accept').removeAttribute("disabled");
//  else
//    document.documentElement.getButton('accept').setAttribute("disabled", "true");
//}


function getBibTeXDirectory()
{
  if (gDialog.bibTeXDir == null)
    gDialog.bibTeXDir = lookUpBibTeXDirectory();

  return gDialog.bibTeXDir;
}

function fillDatabaseFileListbox()
{
  var theListbox = document.getElementById("databaseFileListbox");
  //Clear the listbox
  var rowCount = theListbox.getRowCount();
  for (var i = rowCount - 1; i >= 0; --i)
    theListbox.removeItemAt(i);

  var bibDir = getBibTeXDirectory();  //returns an nsiLocalFile
  if (bibDir && bibDir.exists())
  {
    sortedDirList = new Array();
    addDirToSortedFilesList(sortedDirList, bibDir, ["bib"], true);
    sortedDirList.sort( sortFileEntries );
    var selItem = null;
    for (var i = 0; i < sortedDirList.length; ++i)
    {
      if (gDialog.databaseFile == sortedDirList[i].fileName)
        selItem = theListbox.appendItem(sortedDirList[i].fileName, sortedDirList[i].filePath);
      else
        theListbox.appendItem(sortedDirList[i].fileName, sortedDirList[i].filePath);
    }
    if (selItem != null)
      theListbox.selectItem(selItem);
    else if (sortedDirList.length > 0)
      theListbox.selectedIndex = 0;
  }
}

function fillStyleFileListbox()
{
  var theListbox = document.getElementById("styleFileListbox");
  //Clear the listbox
  var rowCount = theListbox.getRowCount();
  for (var i = rowCount - 1; i >= 0; --i)
    theListbox.removeItemAt(i);

  var bibDir = getBibTeXDirectory();  //returns an nsiLocalFile
  if (bibDir && bibDir.exists())
  {
    sortedDirList = new Array();
    addDirToSortedFilesList(sortedDirList, bibDir, ["bst"], true);
    sortedDirList.sort( sortFileEntries );
    var selItem = null;
    for (var i = 0; i < sortedDirList.length; ++i)
    {
      if (gDialog.styleFile == sortedDirList[i].fileName)
        selItem = theListbox.appendItem(sortedDirList[i].fileName, sortedDirList[i].filePath);
      else
        theListbox.appendItem(sortedDirList[i].fileName, sortedDirList[i].filePath);
    }
    if (selItem != null)
      theListbox.selectItem(selItem);
    else if (sortedDirList.length > 0)
      theListbox.selectedIndex = 0;
  }
}

//function doSelectBibTeXDirectory()
//{
//  var dirPicker = Components.classes["@mozilla.org/filepicker;1"].createInstance(Components.interfaces.nsIFilePicker);
//  dirPicker.init(window, "Select directory", Components.interfaces.nsIFilePicker.modeGetFolder);
//  if (gDialog.bibTeXDir)
//    dirPicker.displayDirectory = gDialog.bibTeXDir;
//  var res = dirPicker.show();
//  if (res == nsIFilePicker.returnOK)
//  {
//    gDialog.bibTeXDir = dirPicker.file;
//    fillDatabaseFileListbox();
//  }
//}

