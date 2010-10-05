// Copyright (c) 2010 MacKichan Software, Inc.  All Rights Reserved.

var data;

// dialog initialization code
function Startup()
{
//  var editorElement = msiGetParentEditorElementForDialog(window);
//  var editor = msiGetEditor(editorElement);
//  if (!editor) {
//    window.close();
//    return;
//  }

  doSetOKCancel(onAccept, onCancel);
  data = window.arguments[0];
  data.Cancel = false;

  //These data items are nsILocalFiles
  gDialog.bibTeXExe = null;
  if (data.bibTeXExePath)
    gDialog.bibTeXExe = data.bibTeXExePath;
  gDialog.bibTeXDBDir = null;
  if (data.bibTeXDBaseDir)
    gDialog.bibTeXDBDir = data.bibTeXDBaseDir;
  gDialog.bibTeXStyDir = null;
  if (data.bibTeXStyleDir)
    gDialog.bibTeXStyDir = data.bibTeXStyleDir;

  gDialog.bWarnNonPortableFilename = data.bWarnNonPortFilename;
  gDialog.bUseOldAuxFiles = data.bUseExistingAuxFiles;
  gDialog.bConvertTeXLinksToPDF = data.bConvertLinksToPDF;
  gDialog.bPassThroughUnicodeMacro = data.bPassThroughUniMacro;

  if ("PDFGraphicsSettings" in data)
  {
    gDialog.PDFGraphicsSettings = new Object();
    data.PDFGraphicsSettings.copyValuesTo(gDialog.PDFGraphicsSettings);
  }

  InitDialog();

  msiSetInitialDialogFocus(document.getElementById("warnNonPortableFilenamesCheckbox"));

  SetWindowLocation();
}

function InitDialog()
{
  document.getElementById("warnNonPortableFilenamesCheckbox").checked = gDialog.bWarnNonPortableFilename;
  document.getElementById("useOldAuxFilesCheckbox").checked = gDialog.bUseOldAuxFiles;
  document.getElementById("convertTeXLinksToPDF").checked = gDialog.bConvertTeXLinksToPDF;
  document.getElementById("passThroughUnicodeMacro").checked = gDialog.bPassThroughUnicodeMacro;
  
  if (gDialog.bibTeXExe)
    document.getElementById("bibTeXExecutableTextbox").value = gDialog.bibTeXExe.path;
  if (gDialog.bibTeXDBDir)
    document.getElementById("bibTeXDatabaseDirTextbox").value = gDialog.bibTeXDBDir.path;
  if (gDialog.bibTeXStyDir)
    document.getElementById("bibTeXStyleDirTextbox").value = gDialog.bibTeXStyDir.path;

//  checkInaccessibleAcceleratorKeys(document.documentElement);
//  gDialog.tabOrderArray = new Array( gDialog.limitsSpecGroup, gDialog.sizeSpecGroup, gDialog.OperatorsGroup,
//                                       document.documentElement.getButton("accept"),
//                                       document.documentElement.getButton("cancel") );
  
//  document.documentElement.getButton("accept").setAttribute("default", true);

//  sizeToContent();
}

function onBrowseBibTeXExecutable()
{
  var filePicker = Components.classes["@mozilla.org/filepicker;1"].createInstance(Components.interfaces.nsIFilePicker);
  filePicker.init(window, msiGetDialogString("filePicker.selectBibStyleDir"), Components.interfaces.nsIFilePicker.modeOpen);
  filePicker.appendFilters(Components.interfaces.nsIFilePicker.filterApps);
  if (gDialog.bibTeXExe)
  {
    filePicker.defaultString = gDialog.bibTeXExe.leafName;
    filePicker.displayDirectory = gDialog.bibTeXExe.parent;
  }
  var res = filePicker.show();
  if (res == Components.interfaces.nsIFilePicker.returnOK)
  {
    gDialog.bibTeXExe = filePicker.file;
    document.getElementById("bibTeXExecutableTextbox").value = gDialog.bibTeXExe.path;
  }
}

function onBrowseBibTeXDatabaseDir()
{
  var dirPicker = Components.classes["@mozilla.org/filepicker;1"].createInstance(Components.interfaces.nsIFilePicker);
  dirPicker.init(window, msiGetDialogString("filePicker.selectBibDBDir"), Components.interfaces.nsIFilePicker.modeGetFolder);
  if (gDialog.bibTeXDBDir)
    dirPicker.displayDirectory = gDialog.bibTeXDBDir;
  var res = dirPicker.show();
  if (res == Components.interfaces.nsIFilePicker.returnOK)
  {
    gDialog.bibTeXDBDir = dirPicker.file;
    document.getElementById("bibTeXDatabaseDirTextbox").value = gDialog.bibTeXDBDir.path;
  }
}

function onBrowseBibTeXStyleDir()
{
  var dirPicker = Components.classes["@mozilla.org/filepicker;1"].createInstance(Components.interfaces.nsIFilePicker);
  dirPicker.init(window, msiGetDialogString("filePicker.selectBibStyleDir"), Components.interfaces.nsIFilePicker.modeGetFolder);
  if (gDialog.bibTeXStyDir)
    dirPicker.displayDirectory = gDialog.bibTeXStyDir;
  var res = dirPicker.show();
  if (res == Components.interfaces.nsIFilePicker.returnOK)
  {
    gDialog.bibTeXStyDir = dirPicker.file;
    document.getElementById("bibTeXStyleDirTextbox").value = gDialog.bibTeXStyDir.path;
  }
}

function doPDFGraphicsSettings()
{
  AlertWithTitle("Unimplemented", "The PDF Graphics Settings dialog is not yet implemented.");
}

function getDataFromControls()
{
  gDialog.bWarnNonPortableFilename = document.getElementById("warnNonPortableFilenamesCheckbox").checked;
  gDialog.bUseOldAuxFiles = document.getElementById("useOldAuxFilesCheckbox").checked;
  gDialog.bConvertTeXLinksToPDF = document.getElementById("convertTeXLinksToPDF").checked;
  gDialog.bPassThroughUnicodeMacro = document.getElementById("passThroughUnicodeMacro").checked;
  
  setLocalFileFromPath(gDialog.bibTeXExe, document.getElementById("bibTeXExecutableTextbox").value);
  setLocalFileFromPath(gDialog.bibTeXDBDir, document.getElementById("bibTeXDatabaseDirTextbox").value);
  setLocalFileFromPath(gDialog.bibTeXStyDir, document.getElementById("bibTeXStyleDirTextbox").value);
}

function setLocalFileFromPath(aFileObj, aPath)
{
  if (!aFileObj)
    aFileObj = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
  aFileObj.initWithPath(aPath);
}

function onAccept()
{
  getDataFromControls();

  data.bWarnNonPortFilename = gDialog.bWarnNonPortableFilename;
  data.bUseExistingAuxFiles = gDialog.bUseOldAuxFiles;
  data.bConvertLinksToPDF = gDialog.bConvertTeXLinksToPDF;
  data.bPassThroughUniMacro = gDialog.bPassThroughUnicodeMacro;
  data.bibTeXExePath = gDialog.bibTeXExe;
  data.bibTeXDBaseDir = gDialog.bibTeXDBDir;
  data.bibTeXStyleDir = gDialog.bibTeXStyDir;
  if ("PDFGraphicsSettings" in gDialog)
  {
    if (!("PDFGraphicsSettings" in data))
      data.PDFGraphicsSettings = new Object();
    gDialog.PDFGraphicsSettings.copyValuesTo(data.PDFGraphicsSettings);
  }
  
  var theWindow = window.opener;
  if (!theWindow || !("setTypesetGenSettings" in theWindow))
    theWindow = msiGetTopLevelWindow();
  theWindow.setTypesetGenSettings(data);

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