// Copyright (c) 2006 MacKichan Software, Inc.  All Rights Reserved.


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
  if ("reviseData" in data)
    setDataFromReviseData(data.reviseData);

//  setDlgDatabaseFile(data.databaseFile);
  gDialog.bibTeXDir = null; //nsiLocalFile
  gDialog.key = data.key;  //a string
  gDialog.remark = data.remark;  //this should become arbitrary markup - a Document Fragment perhaps?
  gDialog.bBibEntryOnly = data.bBibEntryOnly;

  InitDialog();

//  document.getElementById("keyTextbox").focus();
  msiSetInitialDialogFocus(document.getElementById("keyTextbox"));

  SetWindowLocation();
}

function InitDialog()
{
  document.getElementById("keyTextbox").value = gDialog.key;

  //This next will have to be replaced by code to set up the MathML editor for the Remark field
//  document.getElementById("remarkTextbox").value = gDialog.remark;
//  if (!gDialog.remark || gDialog.remark.length == 0)
//    gDialog.remark = "Is this bold?";
//  var theStringSource = "<span style='font-weight: bold;'>" + gDialog.remark + "</span>";
  var editorControl = document.getElementById("remarkEditControl");
  msiInitializeEditorForElement(editorControl, gDialog.remark);
  var prefBranch = GetPrefs();
  var fileDir = 
    prefBranch.getComplexValue("swp.bibtex.dir", Components.interfaces.nsILocalFile);
  if (fileDir != null)
    gDialog.userBibTeXDir = fileDir;
  else gDialog.userBibTeXDir = nulll;
  fillDatabaseFileListbox(fileDir);
  checkDisableControls();

//  checkInaccessibleAcceleratorKeys(document.documentElement);

//  gDialog.tabOrderArray = new Array( gDialog.positionSpecGroup,
//                                       document.documentElement.getButton("accept"),
//                                       document.documentElement.getButton("cancel") );
  
  document.documentElement.getButton("accept").setAttribute("default", true);
}

function setDataFromReviseData(reviseData)
{
  var citeNode = reviseData.getReferenceNode();
  var theKeys = citeNode.getAttribute("citekey");
  if (theKeys && theKeys.length)
    data.keyList = theKeys.split(",");
  for (var ix = 0; ix < data.keyList.length; ++ix)
    data.keyList[ix] = TrimString(data.keyList[ix]);
  data.key = data.keyList[0];  //a string
  dump("In typesetBibTeXCitation.js, setDataFromReviseData, citekey attribute was [" + theKeys + "], data.keyList has length [" + data.keyList.length + "] and data.key is [" + data.key + "].\n");
  data.remark = "";
  var remarks = msiNavigationUtils.getSignificantContents(citeNode);
  for (ix = 0; ix < remarks.length; ++ix)
  {
    if (msiGetBaseNodeName(remarks[ix]) == "biblabel")
    {
      var serializer = new XMLSerializer();
      var remarkNodes = msiNavigationUtils.getSignificantContents(remarks[ix]);
      for (var jx = 0; jx < remarkNodes.length; ++jx)
        data.remark += serializer.serializeToString(remarkNodes[jx]);
      break;
    }
  }
  if (citeNode.hasAttribute("bibTeXDBFile"))
    data.databaseFile = citeNode.getAttribute("bibTeXDBFile");
  if (citeNode.hasAttribute("nocite"))
    data.bBibEntryOnly = (citeNode.getAttribute("nocite") == "true") ? true : false;
}

function onAccept()
{
  data.key = document.getElementById("keyTextbox").value;

  //This next will have to be replaced by code to extract data from the MathML editor for the Remark field
//  data.remark = document.getElementById("remarkTextbox").value;
  var remarkControl = document.getElementById("remarkEditControl");
  var remarkContentFilter = new msiDialogEditorContentFilter(remarkControl);
  gDialog.remark = remarkContentFilter.getDocumentFragmentString();
  if (data.remark != gDialog.remark)
  {
    data.bRemarkChanged = true;
    data.remark = gDialog.remark;
  }

  if (gDialog.databaseFile)
    data.databaseFile = gDialog.databaseFile.path;  //Note: since we respond to the selection action on the listbox anyway, we'll store this each time it changes
  else
    data.databaseFile = null;
  data.bBibEntryOnly = document.getElementById("bibEntryOnlyCheckbox").checked;

  if (gDialog.userBibTeXDir && gDialog.userBibTeXDir.exists())
  {
    var prefs = Components.classes["@mozilla.org/preferences-service;1"].getService(Components.interfaces.nsIPrefBranch);
    try
    {
      prefs.setComplexValue("swp.bibtex.dir", Components.interfaces.nsILocalFile, gDialog.userBibTeXDir);
    }
    catch(exception) {}
  }   
//  else prefs.setComplexValue("swp.bibtex.dir", Components.interfaces.nsILocalFile, null);

  var parentEditorElement = msiGetParentEditorElementForDialog(window);
//  var parentEditor = msiGetEditor(parentEditorElement);
//  AlertWithTitle("Information", "BibTeX Citation Dialog returned key: [" + data.key + "] from file [" + data.databaseFile + "], remark: [" + data.remark + "]; needs to be hooked up to do something!");
  var theWindow = window.opener;
  if ("reviseData" in data)
  {
    if (!theWindow || !("doReviseBibTeXCitation" in theWindow))
      theWindow = msiGetTopLevelWindow();
    if (parentEditorElement && theWindow)
      theWindow.doReviseBibTeXCitation(parentEditorElement, data.reviseData, data);
  }
  else
  {
    if (!theWindow || !("doInsertBibTeXCitation" in theWindow))
      theWindow = msiGetTopLevelWindow();
    if (parentEditorElement && theWindow)
      theWindow.doInsertBibTeXCitation(parentEditorElement, data);
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
  var filePath = "";
  if (selectedListboxItem)
    filePath = selectedListboxItem.value;
  setDlgDatabaseFile(filePath);
  checkDisableControls();
}

function setDlgDatabaseFile(filePath)
{
  if (!filePath || !filePath.length)
  {
    gDialog.databaseFile = null;
    return;
  }
  if (!gDialog.databaseFile)
    gDialog.databaseFile = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
  gDialog.databaseFile.initWithPath(filePath);
}

function checkDisableControls()
{
//  var selDBase = document.getElementById("databaseFileListbox").selectedItem;
  if (gDialog.databaseFile && (gDialog.databaseFile.path.length > 0))
    document.getElementById("viewKeysButton").removeAttribute("disabled");
  else
    document.getElementById("viewKeysButton").setAttribute("disabled", "true");
  if (document.getElementById("keyTextbox").value.length > 0)
    document.documentElement.getButton('accept').removeAttribute("disabled");
  else
    document.documentElement.getButton('accept').setAttribute("disabled", "true");
}


function getBibTeXDirectories()
{
//  if (gDialog.bibTeXDirs == [])
    gDialog.bibTeXDirs = lookUpBibTeXDirectories();

  return gDialog.bibTeXDirs;
}

function fillDatabaseFileListbox( theDirectory)
{
  try {
  var theListbox = document.getElementById("databaseFileListbox");
  //Clear the listbox
  var rowCount = theListbox.getRowCount();
  var i;
  var j;
  var bibDir;
  var bibDirs = [];
  for (i = rowCount - 1; i >= 0; --i)
    theListbox.removeItemAt(i);
  if (theDirectory) {
    bibDirs[0] = theDirectory;
  }
  else bibDirs = getBibTeXDirectories();  //returns an nsILocalFile
  for (i = bibDirs.length - 1; i >=0; i--)
  {
    bibDir = bibDirs[i];
    if (bibDir && bibDir.exists())
    {
      var sortedDirList = new Array();
      addDirToSortedFilesList(sortedDirList, bibDir, ["bib"], true);
      sortedDirList.sort( sortFileEntries );
      var selItem = null;
      var ourFileName = gDialog.databaseFile ? gDialog.databaseFile.leafName : "";
      if (ourFileName.length)
        ourFileName = ourFileName.substring(0, ourFileName.lastIndexOf("."));
      for (j = 0; j < sortedDirList.length; ++j)
      {
        if (ourFileName == sortedDirList[j].fileName)
          selItem = theListbox.appendItem(sortedDirList[j].fileName, sortedDirList[j].filePath);
        else
          theListbox.appendItem(sortedDirList[j].fileName, sortedDirList[j].filePath);
      }
      if (selItem != null)
        theListbox.selectItem(selItem);
    }
  }
}
catch(e)
{
  msidump(e.message);
}
}


function doSelectBibTeXDirectory()
{
  var dirPicker = Components.classes["@mozilla.org/filepicker;1"].createInstance(Components.interfaces.nsIFilePicker);
  dirPicker.init(window, "Select directory", Components.interfaces.nsIFilePicker.modeGetFolder);
  if (gDialog.bibTeXDirs)
    dirPicker.displayDirectory = gDialog.bibTeXDirs[1];
  var res = dirPicker.show();
  if (res == Components.interfaces.nsIFilePicker.returnOK)
  {
    gDialog.userBibTeXDir = dirPicker.file;
    fillDatabaseFileListbox(dirPicker.file);
  }
}

function doViewKeys()
{
  var keysData = new Object();
  keysData.databaseFile = gDialog.databaseFile;  //nsILocalFile object
//  gDialog.bibTeXDir = null; //nsiLocalFile
  keysData.key = gDialog.key;  //a string
  keysData.baseDirectory = gDialog.bibTeXDir;

  window.openDialog("chrome://prince/content/typesetBibTeXKeys.xul", "bibtexkeys", "chrome,close,titlebar,modal,resizable", keysData);
  if (!keysData.Cancel)
  {
    gDialog.key = keysData.key;
    document.getElementById("keyTextbox").value = gDialog.key;
    checkDisableControls();
//    gDialog.packages[whichPackage].setOptions( packageData.options );
//    setPackageDescriptionLine(gDialog.packages[whichPackage]);
  }

}