// Copyright (c) 2006 MacKichan Software, Inc.  All Rights Reserved.

var gBibChoice = "manual";  //a kludge - must get hooked up to editor to really work

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

function goUpdateMSITypesetMenuItems(commandset)
{
  return;  //rwa to do
}

function goUpdateMSITypesetInsertMenuItems(commandset)
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
  gContentWindow.focus();   // needed for command dispatch to work

  try
  {
    var cmdParams = newCommandParams();
    if (!cmdParams) return;

    cmdParams.setStringValue("value", newValue);
    goDoCommandParams(commandID, cmdParams);
  } catch(e) { dump("error thrown in doParamCommand: "+e+"\n"); }
}

//ljh

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
    doFrontMatterDlg();
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
    doInsertCitation();
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

function doFrontMatterDlg()
{
  alert("Front Matter Dialog not implemented!");
}

function doPreambleDlg()
{
  var preambleData = new Object();
  preambleData.preambleText = "\\input{tcilatex}";
  window.openDialog("chrome://editor/content/typesetPreamble.xul", "_blank", "chrome,close,titlebar,modal", preambleData);
  if (!preambleData.Cancel)
  {
    alert("Preamble Dialog returned:\n[" + preambleData.preambleText + "]\n Needs to be hooked up to do something!");
  }
}

function doBibChoiceDlg()
{
  var bibChoiceData = new Object();
  bibChoiceData.bBibTeX = false;
  if (gBibChoice = "BibTeX")  //a kludge - must get hooked up to editor to really work
    bibChoiceData.bBibTeX = true;
  window.openDialog("chrome://editor/content/typesetBibChoice.xul", "_blank", "chrome,close,titlebar,modal", bibChoiceData);
  if (!bibChoiceData.Cancel)
  {
    var choiceStr = "manual bibliography";
    if (bibChoiceData.bBibTeX)
      choiceStr = "BibTeX bibliography";
    alert("Bibliography Choice Dialog returned " + choiceStr + "; needs to be hooked up to do something!");
  }
}

function doOptionsAndPackagesDlg()
{
  var options = new Object();
  options.docClassName = "sebase";  //hard-wired, for now
  options.docClassOptions = "";
  options.packages = new Array();
  window.openDialog("chrome://editor/content/typesetOptionsAndPackages.xul", "_blank", "chrome,close,titlebar,modal", options);
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
  window.openDialog("chrome://editor/content/typesetOutputChoice.xul", "_blank", "chrome,close,titlebar,modal", outputChoiceData);
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

function doInsertCitation()
{
  if (gBibChoice = "BibTeX")  //a kludge - must get hooked up to editor to really work
  {
    var bibCiteData = new Object();
    bibCiteData.databaseFile = "";
    bibCiteData.key = "";  //a string
    bibCiteData.remark = "";  //this should become arbitrary markup - a Document Fragment perhaps?
    bibCiteData.bBibEntryOnly = false;
    window.openDialog("chrome://editor/content/typesetBibTeXCitation.xul", "_blank", "chrome,close,titlebar,modal", bibCiteData);
    if (!bibCiteData.Cancel)
    {
      alert("BibTeX Citation Dialog returned key: [" + bibCiteData.key + "] from file [" + bibCiteData.databaseFile + "], remark: [" + bibCiteData.remark + "]; needs to be hooked up to do something!");
    }
  }
  else
    alert("Insert citation not implemented!");
}

function doInsertBibliography()
{
  var bibliographyData = new Object();
  bibliographyData.databaseFile = "";
  bibliographyData.styleFile = "";
  window.openDialog("chrome://editor/content/typesetBibTeXBibliography.xul", "_blank", "chrome,close,titlebar,modal", bibliographyData);
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
