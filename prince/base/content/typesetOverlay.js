// Copyright (c) 2006 MacKichan Software, Inc.  All Rights Reserved.

const typesetOverlayJS_duplicateTest = "Bad";

var gBibChoice = "manual";  //a kludge - must get hooked up to editor to really work
var gBibItemList = ["bibItem1", "bibItem2", "journalBibEntry", "bookBibEntry"];


function msiSetupMSITypesetMenuCommands(editorElement)
{
  var commandTable = msiGetComposerCommandTable(editorElement);
  
  //dump("Registering msi math menu commands\n");
  commandTable.registerCommand("cmd_MSIDocFormatCmd",                   msiDocFormat);
  commandTable.registerCommand("cmd_MSIfrontMatterCmd",                 msiFrontMatter);
  commandTable.registerCommand("cmd_MSIpreambleCmd",                    msiPreamble);
  commandTable.registerCommand("cmd_MSIbibChoiceCmd",                   msiBibChoice);
  commandTable.registerCommand("cmd_MSItypesetOptionsAndPackagesCmd",   msiTypesetOptionsAndPackages);
  commandTable.registerCommand("cmd_MSItypesetOutputChoiceCmd",         msiTypesetOutputChoice);
  commandTable.registerCommand("cmd_MSItypesetPreviewCmd",              msiTypesetPreview);
  commandTable.registerCommand("cmd_MSItypesetPrintCmd",                msiTypesetPrint);
  commandTable.registerCommand("cmd_MSItypesetCompileCmd",              msiTypesetCompile);
  commandTable.registerCommand("cmd_MSItypesetPDFPreviewCmd",           msiTypesetPDFPreview);
  commandTable.registerCommand("cmd_MSItypesetPDFPrintCmd",             msiTypesetPDFPrint);
  commandTable.registerCommand("cmd_MSItypesetPDFCompileCmd",           msiTypesetPDFCompile);
  commandTable.registerCommand("cmd_MSItypesetGenSettingsCmd",          msiTypesetGenSettings);
  commandTable.registerCommand("cmd_MSItypesetExpertSettingsCmd",       msiTypesetExpertSettings);
//  commandTable.registerCommand("cmd_MSIrunBibTeXCmd",                   msiRunBibTeX);
  commandTable.registerCommand("cmd_MSIrunMakeIndexCmd",                msiRunMakeIndex);
  commandTable.registerCommand("cmd_reviseBibTeXBibliographyCmd",       msiReviseBibTeXBibliography);
  commandTable.registerCommand("cmd_reviseManualBibItemCmd",            msiReviseManualBibItemCmd);
}


function msiSetupMSITypesetInsertMenuCommands(editorElement)
{
  var commandTable = msiGetComposerCommandTable(editorElement);

  commandTable.registerCommand("cmd_MSIinsertIndexEntryCmd",            msiInsertIndexEntry);
  commandTable.registerCommand("cmd_MSIinsertCrossReferenceCmd",	      msiInsertCrossReference);
  commandTable.registerCommand("cmd_MSIinsertCitationCmd",					    msiCitationCommand);
  commandTable.registerCommand("cmd_MSIinsertBibliographyCmd",			    msiInsertBibTeXBibliography);
  commandTable.registerCommand("cmd_MSIinsertTeXFieldCmd",					    msiInsertTeXField);
  commandTable.registerCommand("cmd_MSIinsertSubdocumentCmd",				    msiInsertSubdocument);
}

function goUpdateMSITypesetMenuItems(commandset)
{
  return;  //rwa to do
}

function msiGoUpdateMSITypesetMenuItems(commandset, editorElement)
{
  return;  //rwa to do
}

function goUpdateMSITypesetInsertMenuItems(commandset)
{
  return;  //rwa to do
}

function msiGoUpdateMSITypesetInsertMenuItems(commandset, editorElement)
{
  return;  //rwa to do
}

function initTypesetMenu(menu)
{
  return;  //rwa to do
}

function InitTypesetObjectMenu(menu)
{
  return;  //rwa to do
}

// like doStatefulCommand()
function doParamCommand(commandID, newValue)
{
  var commandNode = document.getElementById(commandID);
  if (commandNode)
      commandNode.setAttribute("value", newValue);
  msiGetActiveEditorElement(window).contentWindow.focus();
//  gContentWindow.focus();   // needed for command dispatch to work

  try
  {
    var cmdParams = newCommandParams();
    if (!cmdParams) return;

    cmdParams.setStringValue("value", newValue);
    msiGoDoCommandParams(commandID, cmdParams);
  } catch(e) { dump("error thrown in doParamCommand: "+e+"\n"); }
}

var msiDocFormat =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement();
    doDocFormatDlg(editorElement, this);
  }
};

var msiFrontMatter =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement();
    doFrontMatterDlg(editorElement, this);
  }
};

var msiPreamble =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    doPreambleDlg();
  }
};

var msiBibChoice =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    doBibChoiceDlg();
  }
};



var msiTypesetOptionsAndPackages =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    doOptionsAndPackagesDlg();
  }
};

var msiTypesetOutputChoice =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement();
    doOutputChoiceDlg(editorElement);
  }
};


var msiTypesetPreview =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    doTeXPreviewDlg();
  }
};

var msiTypesetPrint =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    doTeXPrintDlg();
  }
};

var msiTypesetCompile =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    doTeXCompileDlg();
  }
};

var msiTypesetPDFPreview =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    doPDFPreviewDlg();
  }
};

var msiTypesetPDFPrint =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    doPDFPrintDlg();
  }
};

var msiTypesetPDFCompile =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    doPDFCompileDlg();
  }
};

var msiTypesetGenSettings =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    doGenSettingsDlg();
  }
};

var msiTypesetExpertSettings =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    doExpertSettingsDlg();
  }
};

//var msiRunBibTeX =
//{
//  isCommandEnabled: function(aCommand, dummy)
//  {
//    return true;
//  },
//
//  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
//  doCommandParams: function(aCommand, aParams, aRefCon) {},
//
//  doCommand: function(aCommand)
//  {
//    doRunBibTeX();
//  }
//};
//
var msiRunMakeIndex =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    doRunMakeIndex();
  }
};


var msiInsertIndexEntry = 
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    doInsertIndexEntry();
  }
};

var msiInsertCrossReference =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
//    var editorElement = msiGetParentEditorElementForDialog(window);
    var editorElement = msiGetActiveEditorElement();
//    gActiveEditor = msiGetEditor(editorElement);
//    if (!gActiveEditor)
//    {
//      dump("Failed to get active editor!\n");
//      window.close();
//      return;
//    }
//  var xref = gActiveEditor.getSelectedElement("xref");
    var xrefData = {key: "", refType : "obj"};
    var dlgWindow = msiOpenModelessDialog("chrome://prince/content/xref.xul", "Cross Reference", "chrome, resizable=yes, close, titlebar, dependent",
                                                                                editorElement, "cmd_MSIinsertCrossReferenceCmd", this, xrefData);
//  window.openDialog("chrome://prince/content/xref.xul", "Cross reference", "chrome, resizable=yes, close, titlebar", xref);
  }
};

var msiInsertBibTeXBibliography =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement();
    var bibliographyData = {dbFileList : [], styleFile : ""};
    window.openDialog("chrome://prince/content/typesetBibTeXBibliography.xul", "bibtexbiblio", "chrome,close,titlebar,modal,resizable", bibliographyData);
    if (!bibliographyData.Cancel)
    {
      doInsertBibTeXBibliography(editorElement, bibliographyData);		  
			msiGetEditor(editorElement).incrementModificationCount(1);
    }
  }
};

var msiReviseBibTeXBibliography = 
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) 
  {
    var editorElement = msiGetActiveEditorElement();
    var bibliographyReviseData = msiGetPropertiesDataFromCommandParams(aParams);
    var bibliographyData = {dbFileList : [], styleFile : "", reviseData : bibliographyReviseData};
    var dlgWindow = msiOpenModelessDialog("chrome://prince/content/typesetBibTeXBibliography.xul", "_blank", "chrome,close,titlebar,dependent,resizable",
                                                           editorElement, "cmd_reviseBibTeXBibliographyCmd", this, bibliographyData);
    editorElement.focus();
  },

  doCommand: function(aCommand) {}
};

var msiReviseManualBibItemCmd =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) 
  {
    var editorElement = msiGetActiveEditorElement();
    var bibItemReviseData = msiGetPropertiesDataFromCommandParams(aParams);
    var bibItemData = {key : "", bibLabel : "", reviseData : bibItemReviseData};
    var dlgWindow = msiOpenModelessDialog("chrome://prince/content/typesetBibitemDlg.xul", "_blank", "chrome,close,titlebar,dependent,resizable",
                                                           editorElement, "cmd_reviseManualBibItemCmd", this, bibItemData);
    editorElement.focus();
  },

  doCommand: function(aCommand) {
    var editorElement = msiGetActiveEditorElement();
    var editor = msiGetEditor(editorElement);
    var bibItemData = {key : "", bibLabel : "", paragraphNode : editor.selection.focusNode, offset : editor.selection.focusOffset};
    var dlgWindow = msiOpenModelessDialog("chrome://prince/content/typesetBibitemDlg.xul", "_blank", "chrome,close,titlebar,dependent,resizable",
                                                           editorElement, "cmd_reviseManualBibItemCmd", this, bibItemData);
  }
};

var msiInsertTeXField =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    doInsertTeXField();
  }
};

var msiInsertSubdocument =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    doInsertSubdocument();
  }
};

//const mmlns    = "http://www.w3.org/1998/Math/MathML";
//const xhtmlns  = "http://www.w3.org/1999/xhtml";


function doDocFormatDlg()
{
  var editorElement = document.getElementById("content-frame");
  window.openDialog("chrome://prince/content/typesetDocFormat.xul", "docformat",
    "chrome,close,resizable,titlebar,dependent", msiGetActiveEditorElement);
  //if (!doDocFormatData.Cancel)
  {
		msiGetEditor(editorElement).incrementModificationCount(1);
  }
}



function doFrontMatterDlg(editorElement, commandHandler)
{
  var frontMatterData = {};
  var frontMatterFrag = getFrontMatterDocumentFragment(editorElement.contentDocument);
  var serializer = Components.classes["@mozilla.org/xmlextras/xmlserializer;1"]
                               .createInstance(Components.interfaces.nsIDOMSerializer);
  frontMatterData.frontMatterText = serializer.serializeToString(frontMatterFrag);
  if (!frontMatterData.frontMatterText.length)
    frontMatterData.frontMatterText = "<p>Just a paragraph.</p>";
  var dlgWindow = msiOpenModelessDialog("chrome://prince/content/typesetFrontMatter.xul", "_blank", "chrome,close,titlebar,dependent,resizable",
                                                                              editorElement, "cmd_MSIfrontMatterCmd", commandHandler, frontMatterData);
  msiGetEditor(editorElement).incrementModificationCount(1);
}

function doPreambleDlg()
{

  var editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);
  var document = editor.document;

  var preambleNode = document.getElementsByTagName("preamble")[0];

  var preambleTeXNodeSet = preambleNode.getElementsByTagName("preambleTeX");
  if (preambleTeXNodeSet.length == 0){
     preambleTeXNode = null;
  } else {
     preambleTeXNode = preambleTeXNodeSet[0];
  }
  

  window.openDialog("chrome://prince/content/typesetPreamble.xul", "preamble", "resizable,chrome,close,titlebar,modal", preambleTeXNode);
	msiGetEditor(editorElement).incrementModificationCount(1);
}

function doBibChoiceDlg(editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var bibChoiceData = new Object();
  bibChoiceData.bBibTeX = false;
  var theBibChoice = getBibliographyScheme(editorElement);
  if (theBibChoice == "bibtex")  //a kludge - must get hooked up to editor to really work
    bibChoiceData.bBibTeX = true;
  window.openDialog("chrome://prince/content/typesetBibChoice.xul", "bibchoice", "chrome,close,titlebar,modal,resizable", bibChoiceData);
  if (!bibChoiceData.Cancel)
  {
    var choiceStr = "manual";
    if (bibChoiceData.bBibTeX)
      choiceStr = "bibtex";
    else
      choiceStr = "manual";
    setBibliographyScheme(editorElement, choiceStr);
		msiGetEditor(editorElement).incrementModificationCount(1);
  }
}

function doOptionsAndPackagesDlg(editorElement)
{
//  var options = new Object();
//  options.docClassName = "sebase";  //hard-wired, for now
//  options.docClassOptions = "";
//  options.packages = new Array();
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);
  var document = editor.document;
  var options = msiGetPackagesAndOptionsDataForDocument(document);
  window.openDialog("chrome://prince/content/typesetOptionsAndPackages.xul", "optionsandpackages", "chrome,close,titlebar,modal,resizable", options);
  if (!options.Cancel)
	{
		msiGetEditor(editorElement).incrementModificationCount(1); 
	}
}

function reviseLaTeXPackagesAndOptions(editorElement, dlgData)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);
  var aDocument = editor.document;
  var pkgArray = [];
  for (var ii = 0; ii < dlgData.packages.length; ++ii)
  {
    pkgArray.push( copyPackageData(dlgData.packages[ii]) );
  }
  var startNode = aDocument.documentElement;
  var heads = aDocument.getElementsByTagName("head");
  if (heads.length)
    startNode = heads[0];
  var currPreambleWalker = aDocument.createTreeWalker(startNode, NodeFilter.SHOW_ELEMENT, isDocStyleOrPackage, true);
  var pkgObject, pkgName, newNode;
  var pkgIndex = -1;
  var insertPos = 0;
  var insertParent, insertNewAfter;

  if (currPreambleWalker)
  {
    var nextNode;
    var delNodes = [];
    while (nextNode = currPreambleWalker.nextNode())
    {
      switch(msiGetBaseNodeName(nextNode))
      {
        case "requirespackage":
        case "usepackage":
          if (!insertParent)
            insertParent = nextNode.parentNode;
          pkgName = nextNode.getAttribute("req");
          if (!pkgName) pkgName = nextNode.getAttribute("package");
          pkgIndex = findPackageInData(pkgArray, pkgName);
          pkgObject = pkgArray[pkgIndex];
          if (pkgObject)
          {
//            if (pkgObject.opt && pkgObject.opt.length)
              msiEditorEnsureElementAttribute(nextNode, "opt", pkgObject.opt, editor);
//            else
//              msiEditorEnsureElementAttribute(nextNode, "opt", null, editor);
//            if ("pri" in pkgObject)
//              msiEditorEnsureElementAttribute(nextNode, "pri", String(pkgObject.pri), editor);
//            else
//              msiEditorEnsureElementAttribute(nextNode, "pri", 100, editor);
            insertNewAfter = nextNode;
            pkgArray.splice( pkgIndex, 1 );  //Now that it's taken care of, remove it
          }
          else if (nextNode.nodeName === "usepackage")
          {
            if (!insertNewAfter)
              insertNewAfter = nextNode.previousSibling;
            delNodes.push(nextNode);
          }
        break;
        case "documentclass":  
          var colist = nextNode.parentNode.getElementsByTagName("colist");
          if (colist && colist.length > 0) {
            colist = colist[0];
            if (colist) {
              editor.deleteNode(colist);
            }
          }
          if ("docClassOptions" in dlgData) {
            colist = editor.createNode("colist", nextNode.parentNode, 0);
            var k;
            var arr;
            var s;
            for (k = 0; k < dlgData.docClassOptions.length; k++)
            {
              s = dlgData.docClassOptions[k];
              arr = s.split("=");
              if (arr.length === 2) {
                msiEditorEnsureElementAttribute(colist, arr[0], arr[1], editor);
              }
            }
          }
        break;
        default:
        break;
      }
    }

    if (!insertParent)
       insertParent = startNode;       // ?

    for (var ix = 0; ix < delNodes.length; ++ix)
      editor.deleteNode(delNodes[ix]);

//    dump("In reviseLaTeXPackagesAndOptions(), before inserting new nodes.\n");
    insertPos = 0;
    if (insertNewAfter)
      insertPos = msiNavigationUtils.offsetInParent(insertNewAfter) + 1;
    for (var jx = 0; jx < pkgArray.length; ++jx)
    {
      newNode = aDocument.createElement("usepackage");
      pkgObject = pkgArray[jx];
      dump("In reviseLaTeXPackagesAndOptions(), inserting usepackage node for [" + pkgObject.packageName + "].\n");
      if (!insertParent)
          insertParent =  nextNode;
      editor.insertNode( newNode, insertParent, insertPos++);
      msiEditorEnsureElementAttribute(newNode, "req", pkgObject.pkg, editor);
        msiEditorEnsureElementAttribute(newNode, "opt", pkgObject.opt, editor);
      if ("pri" in pkgObject)
        msiEditorEnsureElementAttribute(newNode, "pri", String(pkgObject.pri), editor);
      if (!insertParent)
        insertParent = startNode;  //should be the preamble!
    }
  }
  
  var isDocStyleOrPackage = {
    acceptNode: function(aNode)
    {
      switch(msiGetBaseNodeName(aNode))
      {
        case "requirespackage":
        case "documentclass":
          return NodeFilter.FILTER_ACCEPT;
        break;
        default:
        break;
      }
      return NodeFilter.FILTER_SKIP;
    }
  };

  function findPackageInData(packageList, aName)
  {
    for (var ix = 0; ix < packageList.length; ++ix)
    {
      if (packageList[ix].pkg == aName)
        return ix;
    }
    return null;
  }

  function copyPackageData(srcPkg)
  {
    var retVal = {pkg : srcPkg.packageName};
    if ("packageOptions" in srcPkg) {
      retVal.opt = srcPkg.packageOptions;
    }
    if ("packagePriority" in srcPkg)
    {
      if (!isNaN(srcPkg.packagePriority))
        retVal.pri = srcPkg.packagePriority;
    }
    return retVal;
  }
}

function doGenSettingsDlg()
{
  var genSettingsData = getTypesetGenSettingsFromPrefs();

  window.openDialog("chrome://prince/content/typesetGenSettingsDialog.xul", "General Typeset Settings", "chrome,close,titlebar,modal,resizable", 
                       genSettingsData);
  var editorElement = msiGetActiveEditorElement();
	msiGetEditor(editorElement).incrementModificationCount(1);
}

function getTypesetGenSettingsFromPrefs()
{
  var theData = new Object();
  theData.bWarnNonPortFilename = GetBoolPref("swp.typeset.warnNonPortableFilename");
  theData.bUseExistingAuxFiles = GetBoolPref("swp.typeset.useExistingAuxFiles");
  theData.bConvertLinksToPDF = GetBoolPref("swp.pdftypeset.convertLinksToPDF");
  theData.bPassThroughUniMacro = GetBoolPref("swp.typeset.passThroughUniMacro");
//  theData.bibTeXExePath = GetLocalFilePref("swp.bibtex.appPath");
  theData.bibTeXDBaseDir = GetLocalFilePref("swp.bibtex.dir");
//  theData.bibTeXStyleDir = GetLocalFilePref("swp.bibtex.styledir");
  return theData;
}

function setTypesetGenSettings(genSettingsData)
{
  SetBoolPref("swp.typeset.warnNonPortableFilename", genSettingsData.bWarnNonPortFilename);
  SetBoolPref("swp.typeset.useExistingAuxFiles", genSettingsData.bUseExistingAuxFiles);
  SetBoolPref("swp.pdftypeset.convertLinksToPDF", genSettingsData.bConvertLinksToPDF);
  SetBoolPref("swp.typeset.passThroughUniMacro", genSettingsData.bPassThroughUniMacro);
//  SetLocalFilePref("swp.bibtex.appPath", genSettingsData.bibTeXExePath);
  SetLocalFilePref("swp.bibtex.dir", genSettingsData.bibTeXDBaseDir);
//  SetLocalFilePref("swp.bibtex.styledir", genSettingsData.bibTeXStyleDir);
}


//function doRunBibTeX()
//{
//  alert("Run BibTeX not implemented!");
//}
//
//function doRunMakeIndex()
//{
//  alert("Run MakeIndex not implemented!");
//}

function doInsertIndexEntry(editorElement, indexNode)
{
  if (!editorElement)
    editorElement = msiGetParentEditorElementForDialog(window);
  gActiveEditor = msiGetEditor(editorElement);
  if (!gActiveEditor)
  {
    dump("Failed to get active editor!\n");
    window.close();
    return;
  }
  if (!indexNode)
    indexNode = gActiveEditor.getSelectedElement("indexitem");
  window.openDialog("chrome://prince/content/indexentry.xul", "Index Entry", "chrome,resizable=yes, close,titlebar,dependent", indexNode);
}

function doInsertManualCitation(editorElement, dlgData)
{
  var editor = msiGetEditor(editorElement);
  var theText = "<citation xmlns=\"" + xhtmlns + "\" citekey=\"" + dlgData.key + "\"";
  if (dlgData.remark && dlgData.remark.length)
    theText += " hasRemark=\"true\"><biblabel class=\"remark\" xmlns=\"" + xhtmlns + "\">" + dlgData.remark + "</biblabel></citation>";
  else
    theText += "/>"
  editor.insertHTMLWithContext(theText, "", "", "text/html", null, null, 0, true);
}

function doReviseManualCitation(editorElement, reviseData, dlgData)
{
  var editor = msiGetEditor(editorElement);
  editor.endTransaction();
  var citeNode = reviseData.getReferenceNode();
  msiEditorEnsureElementAttribute(citeNode, "citekey", dlgData.key, editor);
  if (dlgData.bRemarkChanged)
  {
    var currRemNode = null;
    var children = msiNavigationUtils.getSignificantContents(citeNode);
    for (var ix = 0; ix < children.length; ++ix)
    {
      if (msiGetBaseNodeName(children[ix]) == "biblabel")
      {
        currRemNode = children[ix];
        break;
      }
    }
    if (dlgData.remark.length && !currRemNode)
    {
      currRemNode = editor.document.createElement("biblabel");
      currRemNode.setAttribute("class", "remark");
      msiEditorEnsureElementAttribute(currRemNode, "xmlns", xhtmlns, editor);
      editor.insertNode(currRemNode, citeNode, 0);  //Remark always goes at the start - though the cite shouldn't have any other content anyway
    }
    else if (!dlgData.remark.length && currRemNode)
    {
      editor.deleteNode(currRemNode);
    }
    if (dlgData.remark.length)
    {
      for (ix = 0; ix < currRemNode.childNodes.length; ++ix)
        editor.deleteNode(currRemNode.childNodes[ix]);
      editor.insertHTMLWithContext(dlgData.remark, "", "", "", null, currRemNode, 0, false);
      msiEditorEnsureElementAttribute(citeNode, "hasRemark", "true", editor);
    }
    else
      msiEditorEnsureElementAttribute(citeNode, "hasRemark", "false", editor);
  }
  editor.endTransaction();
}

function doInsertBibTeXCitation(editorElement, dlgData)
{
  var editor = msiGetEditor(editorElement);
  var theText = "<citation xmlns=\"" + xhtmlns + "\" type=\"bibtex\" citekey=\"" + dlgData.key + "\"";
  if (dlgData.bBibEntryOnly)
    theText += " nocite=\"true\"";
  if (dlgData.remark && dlgData.remark.length)
    theText += " hasRemark=\"true\"><biblabel xmlns=\"" + xhtmlns + "\" class=\"remark\">" + dlgData.remark + "</biblabel></citation>";
  else
    theText += "/>"
  editor.insertHTMLWithContext(theText, "", "", "text/html", null, null, 0, true);
}

function doReviseBibTeXCitation(editorElement, reviseData, dlgData)
{
  var editor = msiGetEditor(editorElement);
  editor.endTransaction();
  var citeNode = reviseData.getReferenceNode();
  msiEditorEnsureElementAttribute(citeNode, "citekey", dlgData.key, editor);
  msiEditorEnsureElementAttribute(citeNode, "nocite", dlgData.bBibEntryOnly, editor);
  if (dlgData.bRemarkChanged)
  {
    var currRemNode = null;
    var children = msiNavigationUtils.getSignificantContents(citeNode);
    msiKludgeLogNodeContentsAndAllAttributes(citeNode, ["bibliography"], "In doReviseBibTeXCitation before adding remark, citeNode", true);
    for (var ix = 0; ix < children.length; ++ix)
    {
      if (msiGetBaseNodeName(children[ix]) == "biblabel")
      {
        currRemNode = children[ix];
        break;
      }
    }
    msiKludgeLogNodeContents(currRemNode, ["bibliography"], "In doReviseBibTeXCitation before adding remark, currRemNode", true);
//    if (dlgData.remark.length && !currRemNode)
//    {
//      currRemNode = editor.document.createElement("biblabel");
//      currRemNode.setAttribute("class", "remark");
//      editor.insertNode(currRemNode, citeNode, 0);  //Remark always goes at the start - though the cite shouldn't have any other content anyway
//    }
//    else if (!dlgData.remark.length && currRemNode)
    if (currRemNode)
    {
      editor.deleteNode(currRemNode);
      currRemNode = null;
    }
    if (dlgData.remark.length)
    {
//      for (ix = 0; ix < currRemNode.childNodes.length; ++ix)
//        editor.deleteNode(currRemNode.childNodes[ix]);
//      editor.insertHTMLWithContext(dlgData.remark, "", "", "", null, currRemNode, 0, false);
      editor.insertHTMLWithContext("<biblabel class=\"remark\" xmlns=\"" + xhtmlns + "\">" + dlgData.remark + "</biblabel>", "", "", "", null, citeNode, 0, false);
      msiEditorEnsureElementAttribute(citeNode, "hasRemark", "true", editor);
      msiKludgeLogNodeContentsAndAllAttributes(citeNode, ["bibliography"], "In doReviseBibTeXCitation after insertHTMLWithContext, citeNode", true);
    }
    else
      msiEditorEnsureElementAttribute(citeNode, "hasRemark", "false", editor);
  }
  editor.endTransaction();
}

function getEditorBibItemList(editor)
{
  return gBibItemList;
}

//These function, and the variable "gBibItemList" above,  are solely artificial, to allow simulating the behavior of an 
//editor that would traverse its paragraphs looking for bibliography items. We'll clean this up shortly.
function updateEditorBibItemList(editor, newList)
{
  gBibItemList = unionArrayWith(gBibItemList, newList);
}

function doInsertBibTeXBibliography(editorElement, dlgData)
{
  var editor = msiGetEditor(editorElement);
  var theText = "<bibtexbibliography xmlns=\"" + xhtmlns + "\" databaseFile=\"" + dlgData.databaseFile + "\" styleFile=\"" + dlgData.styleFile + "\"/>";
  editor.insertHTMLWithContext(theText, "", "", "", null, null, 0, true);
}

function doReviseBibTeXBibliography(editorElement, reviseData, dlgData)
{
  var editor = msiGetEditor(editorElement);
  var bibliographyNode = reviseData.getReferenceNode();
  editor.beginTransaction();
  msiEditorEnsureElementAttribute(bibliographyNode, "databaseFile", dlgData.databaseFile, editor);
  msiEditorEnsureElementAttribute(bibliographyNode, "styleFile", dlgData.styleFile, editor);
  editor.endTransaction();
}


function doReviseManualBibItem(editorElement, bibitemNode, dlgData)
{
  var ix;
  var editor = msiGetEditor(editorElement);
  editor.beginTransaction();
  var bodytext = getChildByTagName(bibitemNode, "bodyText");
  if (!bodytext) bodytext = editor.createNode("bodytext", bibitemNode, 0);

  var bibkey = getChildByTagName(bibitemNode, "bibkey");
  if (!bibkey) bibkey = editor.createNode("bibkey", bibitemNode, 0);
  bibkey.textContent = dlgData.key;

  if (dlgData.bBibLabelChanged)
  {
    var currLabelNode = null;
    var children = msiNavigationUtils.getSignificantContents(bibitemNode);
    for (var ix = 0; ix < children.length; ++ix)
    {
      if (msiGetBaseNodeName(children[ix]) == "biblabel")
      {
        currLabelNode = children[ix];
        break;
      }
    }
    if (dlgData.bibLabel.length && !currLabelNode)
    {
      currLabelNode = editor.document.createElement("biblabel");
      currLabelNode.setAttribute("class", "bibitemlabel");
      msiEditorEnsureElementAttribute(bibitemNode, "xmlns", xhtmlns, editor);
      editor.insertNode(currLabelNode, bibitemNode, 0);  //Label always goes at the start
    }
    else if (!dlgData.bibLabel.length && currLabelNode)
    {
      editor.deleteNode(currLabelNode);
    }
    if (dlgData.bibLabel.length)
    {
      for (ix = currLabelNode.childNodes.length; ix > 0; --ix)
        editor.deleteNode(currLabelNode.childNodes[ix-1]);
      editor.insertHTMLWithContext(dlgData.bibLabel, "", "", "", null, currLabelNode, 0, false);
      msiEditorEnsureElementAttribute(bibitemNode, "hasLabel", "true", editor);
    }
    else
      msiEditorEnsureElementAttribute(bibitemNode, "hasLabel", "false", editor);
  }

  editor.endTransaction();
}

function doInsertTeXField()
{
  var editorElement = msiGetParentEditorElementForDialog(window);
  gActiveEditor = msiGetEditor(editorElement);
  if (!gActiveEditor)
  {
    dump("Failed to get active editor!\n");
    window.close();
    return;
  }
  var tbutton = gActiveEditor.getSelectedElement("texb");
  if (!tbutton) tbutton = gActiveEditor.getSelectedElement("texbutton");
  window.openDialog("chrome://prince/content/texbuttoncontents.xul", "TeX field", "resizable=yes,chrome,close,titlebar,dependent", tbutton);
	msiGetEditor(editorElement).incrementModificationCount(1);
}


//function doInsertSubdocument()
//{
//  alert("Insert subdocument not implemented!");
//}
//

function doInsertCrossReference(editorElement, dlgData)
{
  var editor = msiGetEditor(editorElement);
  var xrefNode = editor.document.createElementNS(xhtmlns, "xref");
  if (dlgData.key && dlgData.key.length > 0)
  {
    xrefNode.setAttribute("key", dlgData.key);
    xrefNode.setAttribute("href", dlgData.key);
  }
  xrefNode.setAttribute("reftype", dlgData.refType);
  if (dlgData.vario)
    xrefNode.setAttribute("req", "varioref");
  editor.insertElementAtSelection(xrefNode, true);
}

function doReviseCrossReference(editorElement, xrefNode, dlgData)
{
  var editor = msiGetEditor(editorElement);
  editor.beginTransaction();
  msiEditorEnsureElementAttribute(xrefNode, "key", dlgData.key, editor);
  msiEditorEnsureElementAttribute(xrefNode, "href", dlgData.key, editor);
  msiEditorEnsureElementAttribute(xrefNode, "reftype", dlgData.refType, editor);
  if (dlgData.vario)
    msiEditorEnsureElementAttribute(xrefNode, "req", "varioref", editor);
  else xrefNode.removeAttribute("req");
  editor.endTransaction();
}

function msiGetPackagesAndOptionsDataForDocument(aDocument)
{
  var isDocStyleOrPackage = {
    acceptNode: function(aNode)
    {
      switch(msiGetBaseNodeName(aNode))
      {
        case "requirespackage":
        case "usepackage":
        case "documentclass":
        case "colist":
          return NodeFilter.FILTER_ACCEPT;
        break;
        default:
        break;
      }
      return NodeFilter.FILTER_SKIP;
    }
  };

  var retObj = { docClassName : "article", docClassOptions : "", packages : [] };
  var pkgName = null;
  var pkgPriority = 0;
  var options = null;
  var startNode = aDocument.documentElement;
  var heads = aDocument.getElementsByTagName("head");
  if (heads.length)
    startNode = heads[0];
  var currPreambleWalker = aDocument.createTreeWalker(startNode, NodeFilter.SHOW_ELEMENT, isDocStyleOrPackage, true);
  if (currPreambleWalker)
  {
    var nextNode;
    var ignoreAttrs = /^[_\-]moz/;
    while (nextNode = currPreambleWalker.nextNode())
    {
      switch(msiGetBaseNodeName(nextNode))
      {
        case "requirespackage":
        case "usepackage":
          pkgName = nextNode.getAttribute("req");
          if (!pkgName || !pkgName.length)
            pkgName = nextNode.getAttribute("package");
          pkgPriority = Number( nextNode.getAttribute("pri") );
          options = nextNode.getAttribute("opt");
          if (pkgName && pkgName.length)
            retObj.packages.push( {packageName : pkgName, packageOptions : options, packagePriority : pkgPriority} );
        break;
        case "documentclass":
          retObj.docClassName = nextNode.getAttribute("class");
          retObj.docClassOptions = "";
          var optionstr = nextNode.getAttribute("options");
          if (optionstr && optionstr.length)
            retObj.docClassOptions = optionstr;
        break;
        case "colist":
          for (var ix = 0; ix < nextNode.attributes.length; ++ix)
          {
            if ((nextNode.attributes[ix].nodeName != "enabled") && (nextNode.attributes[ix].nodeName.search(ignoreAttrs) < 0))
            {
              if (retObj.docClassOptions.length > 0)
                retObj.docClassOptions += ", ";
              retObj.docClassOptions += nextNode.attributes[ix].textContent;
            }
          }
        break;
        default:
        break;
      }
    }
  }

  return retObj;
}

//This list needs to be variable depending on the document - have to have "isFrontMatterTag()" available as query on
//tags.
function nodeIsFrontMatterNode(theNode)
{
  var editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);
  if (editor.tagListManager.getTagInClass("frontmtag", theNode.tagName, null))
  {
      return NodeFilter.FILTER_ACCEPT;
  }
  return NodeFilter.FILTER_SKIP;
}

function getFrontMatterDocumentFragment(theDocument)
{
  var docFrag = theDocument.createDocumentFragment();
  var namespaceStr = "sw:";

  var namespaceStr = "sw:";
  var currFrontMatterWalker = theDocument.createTreeWalker(theDocument.documentElement, NodeFilter.SHOW_ELEMENT, nodeIsFrontMatterNode, true);
  if (currFrontMatterWalker)
  {
    var nextNode = currFrontMatterWalker.nextNode();
    while (nextNode)
    {
      docFrag.appendChild(nextNode.cloneNode(true));
      nextNode = currFrontMatterWalker.nextNode();
    }
  }
  return docFrag;
}

function insertFrontMatter(editorElement, frontMatterData)
{
  var parser = new DOMParser;
  var frontMatterDoc = parser.parseFromString(frontMatterData.frontMatterText, "text/xml");
  var namespaceStr = "sw:";
  var editor = msiGetEditor(editorElement);
  var currFrontMatterWalker = editor.document.createTreeWalker(editor.document.documentElement, NodeFilter.SHOW_ELEMENT, nodeIsFrontMatterNode, true);
                                                
  var firstNode = null;
  if (currFrontMatterWalker)
  {
    var nextNode = currFrontMatterWalker.nextNode();
    firstNode = nextNode;
    nextNode = currFrontMatterWalker.nextNode();
    while (nextNode)
    {
      var tmp = currFrontMatterWalker.nextNode();
      nextNode.parentNode.removeChild(nextNode);
      nextNode = tmp;
    }
  }
  if (!firstNode)
  {
    var docBody = editor.document.getElementsByTagNameNS(namespaceStr, "docbody");
    if (!docBody)
      docBody = editor.document.getElementsByTagName("body");
    if (!docBody)
      docBody = editor.document.getElementsByTagName("BODY");
    if (docBody.length > 0)
      firstNode = docBody[0].firstChild;
  }
  if (!firstNode)
    firstNode = editor.document.documentElement.firstChild;
  var newNodes = frontMatterDoc.documentElement.childNodes;
  for (var i = 0; i < newNodes.length; ++i)
  {
    firstNode.parentNode.insertBefore(newNodes[i].cloneNode(true), firstNode);
  }
}

 
