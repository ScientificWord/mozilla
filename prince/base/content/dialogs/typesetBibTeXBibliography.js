// Copyright (c) 2006 MacKichan Software, Inc.  All Rights Reserved.
Components.utils.import("resource://app/modules/pathutils.jsm");
Components.utils.import("resource://app/modules/msiEditorDefinitions.jsm");

//const mmlns    = "http://www.w3.org/1998/Math/MathML";
//const xhtmlns  = "http://www.w3.org/1999/xhtml";

var data;

// dialog initialization code
function Startup()
{
  var editorElement = msiGetParentEditorElementForDialog(window);
  var editor = msiGetEditor(editorElement);
  if (!editor) {
    window.close();
    return;
  }

  gDialog.dbFileListbox = document.getElementById("databaseFileListbox");
  doSetOKCancel(onAccept, onCancel);
  data = window.arguments[0];
  data.Cancel = false;
  if (("reviseData" in data) && data.reviseData)
    setDataFromReviseData(data.reviseData);
//  gDialog.databaseFile = data.databaseFile;  //a string representation of file path
  gDialog.databaseFileList = [];
  gDialog.databaseFileList = gDialog.databaseFileList.concat(data.dbFileList);
  gDialog.styleFile = data.styleFile;  //a string representation of file path
  gDialog.bibTeXDirs = []; //nsiLocalFile array
  gDialog.bibTeXStyleDirs = []; //nsiLocalFile array

  InitDialog();

//  document.getElementById("databaseFileListbox").focus();
  msiSetInitialDialogFocus(gDialog.dbFileListbox);

  SetWindowLocation();
}

function InitDialog()
{
  var prefBranch = GetPrefs();
  var fileDir;
  fileDir = getPreferredBibTeXDir("bib");
  // try {
  //   fileDir = prefBranch.getComplexValue("swp.bibtex.dir", Components.interfaces.nsILocalFile);
  //   if (fileDir) {  //old style pref used files. We've switched to paths
  //     fileDir = fileDir.path;
  //     prefBranch.setCharPref("swp.bibtex.dir", fileDir);
  //   }
  // }
  // catch(e) {
  //   fileDir = prefBranch.getCharPref("swp.bibtex.dir");
  // }
  if (fileDir != null) {
    gDialog.userBibTeXDir = fileDir;
    fillDatabaseFileListbox(fileDir.path);
  }
  else {
    gDialog.userBibTeXDir = null;
  }

  var styleDir;

  styleDir = getPreferredBibTeXDir("bst");
  //   styleDir = prefBranch.getComplexValue("swp.bibtexstyle.dir", Components.interfaces.nsILocalFile);
  //   if (styleDir) {  //old style pref used files. We've switched to paths
  //     styleDir = styleDir.path;
  //     prefBranch.setCharPref("swp.bibtexstyle.dir", styleDir);
  //   }
  // }
  // catch(e) {
  //   styleDir = prefBranch.getCharPref("swp.bibtexstyle.dir");
  // }
  if (styleDir != null) {
    gDialog.userBibTeXStyleDir = styleDir;
    fillStyleFileListbox(styleDir.path);
  }
  else {
    gDialog.userBibTeXStyleDir = null;
  }
  document.documentElement.getButton("accept").setAttribute("default", true);
}

function setDataFromReviseData(reviseData)
{
  var biblioNode = reviseData.getReferenceNode();
  var theBibliographiesStr = biblioNode.getAttribute("databaseFile");
  if (theBibliographiesStr && theBibliographiesStr.length)
    data.dbFileList = theBibliographiesStr.split(",");
  else
    data.dbFileList = [];
  for (var ix = 0; ix < data.dbFileList.length; ++ix)
    data.dbFileList[ix] = TrimString(data.dbFileList[ix]);
  data.databaseFile = data.dbFileList[0];
  data.styleFile =  biblioNode.getAttribute("styleFile");
}

function changeDatabaseSelection()
{
  return;
  var selItems = gDialog.dbFileListbox.selectedItems;
  var addedItems = [];
  var deletedItems = [];
  for (var ix = 0; ix < gDialog.databaseFileList.length; ++ix)
    deletedItems.push(ix);
  var foundIndex;
  for (var ix = 0; ix < selItems.length; ++ix)
  {
    foundIndex = gDialog.databaseFileList.indexOf(selItems[ix].label);
    if (foundIndex >= 0)
      deletedItems[foundIndex] = -1;  //Marking it not to be deleted
    else
      addedItems.push(selItems[ix].label);
  }
  for (ix = deletedItems.length - 1; ix >= 0; --ix)
  {
    if (deletedItems[ix] >= 0)
      gDialog.databaseFileList.splice(deletedItems[ix], 1);
  }
  gDialog.databaseFileList = gDialog.databaseFileList.concat(addedItems);
}

function onAccept()
{
//  var databaseItem = document.getElementById("databaseFileListbox").getSelectedItem(0);
//  if (databaseItem)
//    data.databaseFile = databaseItem.label;
//    data.databaseFile = databaseItem.value;


  var selItems = gDialog.dbFileListbox.selectedItems;
  var styleItem = document.getElementById("styleFileListbox").getSelectedItem(0);
  if (styleItem)
    data.styleFile = styleItem.label;
  // data.dbFileList.splice(0, data.dbFileList.length);
  // data.dbFileList = data.dbFileList.concat(gDialog.databaseFileList);
  data.dbFileList = [];
  selItems.forEach( function ( value ) {data.dbFileList.push(value.label);});
  dump("In typesetBibTeXBibliography.js, onAccept(), data.dbFileList has [" + data.dbFileList.length + "] items.\n");
  data.databaseFile = data.dbFileList.join(",");
  var theWindow = window.opener;
  var parentEditorElement = msiGetParentEditorElementForDialog(window);
  if (("reviseData" in data) && data.reviseData)
  {
    if (!theWindow || !("doReviseBibTeXBibliography" in theWindow))
      theWindow = msiGetTopLevelWindow();
    if (parentEditorElement && theWindow)
      theWindow.doReviseBibTeXBibliography(parentEditorElement, data.reviseData, data);
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


function getBibTeXDirectories()
{
    gDialog.bibTeXDirs = lookUpBibTeXDirectories();

  return gDialog.bibTeXDirs;
}

function getBibTeXStyleDirectories()
{
    gDialog.bibTeXStyleDirs = lookUpBibTeXStyleDirectories();

  return gDialog.bibTeXStyleDirs;
}


function fillDatabaseFileListbox(fileDirPath)
{
//  var theListbox = document.getElementById("databaseFileListbox");
  //Clear the listbox
  var rowCount = gDialog.dbFileListbox.getRowCount();
  var fileDir = msiFileFromAbsolutePath( fileDirPath );
  var bibDirs;
  for (var i = rowCount - 1; i >= 0; --i)
    gDialog.dbFileListbox.removeItemAt(i);

  if (fileDirPath) {
    bibDirs = [fileDir];
  }
  else bibDirs= getBibTeXDirectories();  //returns an array of 2 nsILocalFiles
  var sortedDirList;
  var bibDir;
  var j;
  for (j=bibDirs.length - 1; j >= 0; j--)
  {
    bibDir = bibDirs[j];
    if (bibDir && bibDir.exists())
    {
      sortedDirList = new Array();
      addDirToSortedFilesList(sortedDirList, bibDir, ["bib"], true);
      sortedDirList.sort( sortFileEntries );
      var newItem = null;
      for (var i = 0; i < sortedDirList.length; ++i)
      {
        newItem = gDialog.dbFileListbox.appendItem(sortedDirList[i].fileName, sortedDirList[i].filePath);
        if (data.dbFileList.indexOf(sortedDirList[i].fileName) >= 0) {
          gDialog.dbFileListbox.addItemToSelection(newItem);
          newItem.setAttribute("selected", "true");
        }
      }
    }
  }
}

function fillStyleFileListbox(filePath)
{
  var theListbox = document.getElementById("styleFileListbox");
  //Clear the listbox
  var rowCount = theListbox.getRowCount();
  var fileDir;
  var bibDir;
  var sortedDirList;

  var j;
  for (var i = rowCount - 1; i >= 0; --i)
    theListbox.removeItemAt(i);
  var bibDirs = [];
  if (filePath) {
    fileDir = msiFileFromAbsolutePath(filePath);
    // fileDir && fileDir.append('bst');
    bibDirs = [fileDir];
  }
  else {
    bibDirs = getBibTeXStyleDirectories();  //returns an nsiLocalFile array
  }
  for (j=bibDirs.length - 1; j >= 0; j--)
  {
    bibDir = bibDirs[j];
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

