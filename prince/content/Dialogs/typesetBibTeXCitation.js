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
  gDialog.bibTeXDir = null; //nsiLocalFile
  gDialog.key = data.key;  //a string
  gDialog.remark = data.remark;  //this should become arbitrary markup - a Document Fragment perhaps?
  gDialog.bBibEntryOnly = data.bBibEntryOnly;

  InitDialog();

  document.getElementById("keyTextbox").focus();

  SetWindowLocation();
}

function InitDialog()
{
  document.getElementById("keyTextbox").value = gDialog.key;
  //This next will have to be replaced by code to set up the MathML editor for the Remark field
  document.getElementById("remarkTextbox").value = gDialog.remark;
  fillDatabaseFileListbox();
  checkDisableControls();

//  checkInaccessibleAcceleratorKeys(document.documentElement);

//  gDialog.tabOrderArray = new Array( gDialog.positionSpecGroup,
//                                       document.documentElement.getButton("accept"),
//                                       document.documentElement.getButton("cancel") );
  
  document.documentElement.getButton("accept").setAttribute("default", true);
}

function onAccept()
{
  data.key = document.getElementById("keyTextbox").value;
  //This next will have to be replaced by code to extract data from the MathML editor for the Remark field
  data.remark = document.getElementById("remarkTextbox").value;
  data.databaseFile = gDialog.databaseFile;  //Note: since we respond to the selection action on the listbox anyway, we'll store this each time it changes
  data.bBibEntryOnly = document.getElementById("bibEntryOnlyCheckbox").checked;

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


function selectDatabaseFile(selectedListboxItem)
{
  if (selectedListboxItem)
    gDialog.databaseFile = selectedListboxItem.value;
  else
    gDialog.databaseFile = "";
  checkDisableControls();
}

function checkDisableControls()
{
//  var selDBase = document.getElementById("databaseFileListbox").selectedItem;
  if (gDialog.databaseFile.length > 0)
    document.getElementById("viewKeysButton").removeAttribute("disabled");
  else
    document.getElementById("viewKeysButton").setAttribute("disabled", "true");
  if (document.getElementById("keyTextbox").value.length > 0)
    document.documentElement.getButton('accept').removeAttribute("disabled");
  else
    document.documentElement.getButton('accept').setAttribute("disabled", "true");
}


function getBibTeXDirectory()
{
  if (gDialog.bibTeXDir == null)
    gDialog.bibTeXDir = lookUpBibTeXDirectory();

  return gDialog.bibTeXDir;
}

//function getBibTeXDirectory()
//{
//  if (gDialog.bibTeXDir != null)
//    return gDialog.bibTeXDir;
//
//  var prefs = Components.classes["@mozilla.org/preferences-service;1"].getService(Components.interfaces.nsIPrefBranch);
//  try
//  {
//    var startDir = prefs.getComplexValue("prince.bibtex.dir", Components.interfaces.nsILocalFile);
//    if (!startDir || !startDir.exists())
//    {
//      var dsprops = Components.classes["@mozilla.org/file/directory_service;1"].createInstance(Components.interfaces.nsIProperties);
//      var startDir = dsprops.get("CurProcD", Components.interfaces.nsIFile);
//
//    }
//    if (startDir.exists())
//      gDialog.bibTeXDir = startDir;
//  } 
//  catch(exception) { }
//
//  return gDialog.bibTeXDir;
//}

function fillDatabaseFileListbox()
{
  var theListbox = document.getElementById("databaseFileListbox");
  //Clear the listbox
  var rowCount = theListbox.getRowCount();
  for (var i = rowCount - 1; i >= 0; --i)
    theListbox.removeItemAt(i);

  var bibDir = getBibTeXDirectory();  //returns an nsILocalFile
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
  }
}


function doSelectBibTeXDirectory()
{
  var dirPicker = Components.classes["@mozilla.org/filepicker;1"].createInstance(Components.interfaces.nsIFilePicker);
  dirPicker.init(window, "Select directory", Components.interfaces.nsIFilePicker.modeGetFolder);
  if (gDialog.bibTeXDir)
    dirPicker.displayDirectory = gDialog.bibTeXDir;
  var res = dirPicker.show();
  if (res == nsIFilePicker.returnOK)
  {
    gDialog.bibTeXDir = dirPicker.file;
    fillDatabaseFileListbox();
  }
}

function doViewKeys()
{
  var keysData = new Object();
  keysData.databaseFile = gDialog.databaseFile;  //a string representation of file path
//  gDialog.bibTeXDir = null; //nsiLocalFile
  keysData.key = gDialog.key;  //a string
  keysData.baseDirectory = gDialog.bibTeXDir;

  window.openDialog("chrome://prince/content/typesetBibTeXKeys.xul", "_blank", "chrome,close,titlebar,modal", keysData);
  if (!keysData.Cancel)
  {
    gDialog.key = keysData.key;
    document.getElementById("keyTextbox").value = gDialog.key;
    checkDisableControls();
//    gDialog.packages[whichPackage].setOptions( packageData.options );
//    setPackageDescriptionLine(gDialog.packages[whichPackage]);
  }

}