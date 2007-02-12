// Copyright (c) 2006 MacKichan Software, Inc.  All Rights Reserved.

var gBibChoice = "manual";  //a kludge - must get hooked up to editor to really work
var gBibItemList = ["bibItem1", "bibItem2", "journalBibEntry", "bookBibEntry"];

function SetupMSITypesetMenuCommands()
{
  var commandTable = GetComposerCommandTable();
  
  //dump("Registering msi math menu commands\n");
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
  commandTable.registerCommand("cmd_MSIrunBibTeXCmd",                   msiRunBibTeX);
  commandTable.registerCommand("cmd_MSIrunMakeIndexCmd",                msiRunMakeIndex);
}

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
  commandTable.registerCommand("cmd_MSIrunBibTeXCmd",                   msiRunBibTeX);
  commandTable.registerCommand("cmd_MSIrunMakeIndexCmd",                msiRunMakeIndex);
}

function SetupMSITypesetInsertMenuCommands()
{
  var commandTable = GetComposerCommandTable();

  commandTable.registerCommand("cmd_MSIinsertIndexEntryCmd",            msiInsertIndexEntry);
  commandTable.registerCommand("cmd_MSIinsertCrossReferenceCmd",	      msiInsertCrossReference);
  commandTable.registerCommand("cmd_MSIinsertCitationCmd",					    msiInsertCitation);
  commandTable.registerCommand("cmd_MSIinsertBibliographyCmd",			    msiInsertBibliography);
  commandTable.registerCommand("cmd_MSIinsertTeXFieldCmd",					    msiInsertTeXField);
  commandTable.registerCommand("cmd_MSIinsertSubdocumentCmd",				    msiInsertSubdocument);
}

function msiSetupMSITypesetInsertMenuCommands(editorElement)
{
  var commandTable = msiGetComposerCommandTable(editorElement);

  commandTable.registerCommand("cmd_MSIinsertIndexEntryCmd",            msiInsertIndexEntry);
  commandTable.registerCommand("cmd_MSIinsertCrossReferenceCmd",	      msiInsertCrossReference);
  commandTable.registerCommand("cmd_MSIinsertCitationCmd",					    msiInsertCitation);
  commandTable.registerCommand("cmd_MSIinsertBibliographyCmd",			    msiInsertBibliography);
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

//ljh
msiDocFormat
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
    doOutputChoiceDlg();
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

var msiRunBibTeX =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    doRunBibTeX();
  }
};

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
    doInsertCrossReference();
  }
};

var msiInsertCitation =
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
    doInsertCitation(editorElement, this);
  }
};

var msiInsertBibliography =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    doInsertBibliography();
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
  window.openDialog("chrome://prince/content/typesetDocFormat.xul", "_blank", 
    "chrome,close,resizable, titlebar,modal", editorElement);
//  if (!doDocFormatData.Cancel)
//  {
//    alert("Document Format Dialog returned.\nNeeds to be hooked up to do something!");
//  }
}



function doFrontMatterDlg(editorElement, commandHandler)
{
  var frontMatterData = new Object();
  var frontMatterFrag = getFrontMatterDocumentFragment(editorElement.contentDocument);
  var serializer = Components.classes["@mozilla.org/xmlextras/xmlserializer;1"]
                               .createInstance(Components.interfaces.nsIDOMSerializer);
  frontMatterData.frontMatterText = serializer.serializeToString(frontMatterFrag);
  if (!frontMatterData.frontMatterText.length)
    frontMatterData.frontMatterText = "<p>Just a paragraph.</p>";
  var dlgWindow = msiOpenModelessDialog("chrome://prince/content/typesetFrontMatter.xul", "_blank", "chrome,close,titlebar,dependent",
                                                                              editorElement, "cmd_MSIfrontMatterCmd", commandHandler, frontMatterData);
//  alert("Front Matter Dialog not implemented!");
}

function doPreambleDlg()
{
  var preambleData = new Object();
  preambleData.preambleText = "\\input{tcilatex}";
  window.openDialog("chrome://prince/content/typesetPreamble.xul", "_blank", "chrome,close,titlebar,modal", preambleData);
  if (!preambleData.Cancel)
  {
    alert("Preamble Dialog returned:\n[" + preambleData.preambleText + "]\n Needs to be hooked up to do something!");
  }
}

function doBibChoiceDlg()
{
  var bibChoiceData = new Object();
  bibChoiceData.bBibTeX = false;
  if (gBibChoice == "BibTeX")  //a kludge - must get hooked up to editor to really work
    bibChoiceData.bBibTeX = true;
  window.openDialog("chrome://prince/content/typesetBibChoice.xul", "_blank", "chrome,close,titlebar,modal", bibChoiceData);
  if (!bibChoiceData.Cancel)
  {
    var choiceStr = "manual bibliography";
    if (bibChoiceData.bBibTeX)
    {
      choiceStr = "BibTeX bibliography";
      gBibChoice = "BibTeX";
    }
    else
      gBibChoice = "manual";
    alert("Bibliography Choice Dialog returned " + choiceStr + "; needs to be hooked up to do something!");
  }
}

function doOptionsAndPackagesDlg()
{
  var options = new Object();
  options.docClassName = "sebase";  //hard-wired, for now
  options.docClassOptions = "";
  options.packages = new Array();
  window.openDialog("chrome://prince/content/typesetOptionsAndPackages.xul", "_blank", "chrome,close,titlebar,modal", options);
  if (!options.Cancel)
  {
    var packagesOptionsStr = options.docClassName;
    if (options.docClassOptions.length > 0)
      packagesOptionsStr += "[" + options.docClassOptions + "]";
    for (var i = 0; i < options.packages.length; ++i)
    {
      var packageStr = "\n";
      if (options.packages[i].packageOptions.length)
        packageStr += "[" + options.packages[i].packageOptions + "]";
      packageStr += "{" + options.packages[i].packageName + "}";
      packagesOptionsStr += packageStr;
    }
    alert("Options and packages dialog returned; needs to be hooked up to do something! Options and packages are:\n" + packagesOptionsStr);
  }
}

function doOutputChoiceDlg()
{
  var outputChoiceData = new Object();
  outputChoiceData.outputChoice = "dvi";
  window.openDialog("chrome://prince/content/typesetOutputChoice.xul", "_blank", "chrome,close,titlebar,modal", outputChoiceData);
  if (!outputChoiceData.Cancel)
  {
    alert("Output Choice Dialog returned " + outputChoiceData.outputChoice + "; needs to be hooked up to do something!");
  }
}

function doTeXPreviewDlg()
{
  alert("TeX Preview Dialog not implemented!");
}

function doTeXPrintDlg()
{
  alert("TeX Print Dialog not implemented!");
}

function doTeXCompileDlg()
{
  alert("TeX Compile Dialog not implemented!");
}

function doPDFPreviewDlg()
{
  alert("PDF Preview Dialog not implemented!");
}

function doPDFPrintDlg()
{
  alert("PDF Print Dialog not implemented!");
}

function doPDFCompileDlg()
{
  alert("PDF Compile Dialog not implemented!");
}

function doGenSettingsDlg()
{
  alert("General Settings Dialog not implemented!");
}

function doExpertSettingsDlg()
{
  alert("Expert Settings Dialog not implemented!");
}

function doRunBibTeX()
{
  alert("Run BibTeX not implemented!");
}

function doRunMakeIndex()
{
  alert("Run MakeIndex not implemented!");
}

function doInsertIndexEntry()
{
  alert("Insert index entry not implemented!");
}

function doInsertCrossReference()
{
  alert("Insert cross reference not implemented!");
}

function doInsertCitation(editorElement, command, commandHandler)
{
  if (gBibChoice == "BibTeX")  //a kludge - must get hooked up to editor to really work
  {
    var bibCiteData = new Object();
    bibCiteData.databaseFile = "";
    bibCiteData.key = "";  //a string
    bibCiteData.remark = "";  //this should become arbitrary markup - a Document Fragment perhaps?
    bibCiteData.bBibEntryOnly = false;
    var dlgWindow = msiOpenModelessDialog("chrome://prince/content/typesetBibTeXCitation.xul", "_blank", "chrome,close,titlebar,dependent",
                                                     editorElement, "cmd_MSIinsertCitationCmd", commandHandler, bibCiteData);
//    window.openDialog("chrome://prince/content/typesetBibTeXCitation.xul", "_blank", "chrome,close,titlebar,modal", bibCiteData);
//    if (!bibCiteData.Cancel)
//    {
//      alert("BibTeX Citation Dialog returned key: [" + bibCiteData.key + "] from file [" + bibCiteData.databaseFile + "], remark: [" + bibCiteData.remark + "]; needs to be hooked up to do something!");
//    }
  }
  else
  {
    var manualCiteData = new Object();
    manualCiteData.key = "";  //a string
    manualCiteData.remark = "";  //this should become arbitrary markup - a Document Fragment perhaps?
    manualCiteData.keyList = new Array();
    var editor = msiGetEditor(editorElement);
    if (editor)
      manualCiteData.keyList = manualCiteData.keyList.concat(getEditorBibItemList(editor));

    var dlgWindow = msiOpenModelessDialog("chrome://prince/content/typesetManualCitation.xul", "_blank", "chrome,close,titlebar,dependent",
                                                           editorElement, "cmd_MSIinsertCitationCmd", commandHandler, manualCiteData);
//    window.openDialog("chrome://editor/content/typesetManualCitation.xul", "_blank", "chrome,close,titlebar,modal", manualCiteData);
//    if (!manualCiteData.Cancel)
//    {
//      alert("Manual Citation Dialog returned key: [" + manualCiteData.key + "], remark: [" + manualCiteData.remark + "]; needs to be hooked up to do something!");
//      if (editor)
//        updateEditorBibItemList(editor, manualCiteData.keyList);
//    }
  }
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

function doInsertBibliography()
{
  var bibliographyData = new Object();
  bibliographyData.databaseFile = "";
  bibliographyData.styleFile = "";
  window.openDialog("chrome://prince/content/typesetBibTeXBibliography.xul", "_blank", "chrome,close,titlebar,modal", bibliographyData);
  if (!bibliographyData.Cancel)
  {
    alert("BibTeX Bibliography Dialog returned bibliography file: [" + bibliographyData.databaseFile + "], style file: [" + bibliographyData.styleFile + "]; needs to be hooked up to do something!");
  }
}

function doInsertTeXField()
{
  alert("Insert TeX field not implemented!");
}

function doInsertSubdocument()
{
  alert("Insert subdocument not implemented!");
}


//This list needs to be variable depending on the document - have to have "isFrontMatterTag()" available as query on
//tags.
var gFrontMatterTags = ["author", "title", "makeTitle", "makeTOC"];

function nodeIsFrontMatterNode(theNode)
{
  for (var i = 0; i < gFrontMatterTags.length; ++i)
  {
    if (theNode.nodeName.toLowerCase() == "gFrontMatterTags[i]")
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
