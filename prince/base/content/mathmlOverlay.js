// Copyright (c) 2004 MacKichan Software, Inc.  All Rights Reserved.

const mathmlOverlayJS_duplicateTest = "Bad";

function SetupMSIMathMenuCommands()
{
  var commandTable = GetComposerCommandTable();
  alert("Should not be calling SetupMSIMathMenuCommands()!");
  //dump("Registering msi math menu commands\n");
  commandTable.registerCommand("cmd_MSIinlineMathCmd",  msiInlineMath);
  commandTable.registerCommand("cmd_MSIdisplayMathCmd", msiDisplayMath);
  commandTable.registerCommand("cmd_MSImathtext",       msiToggleMathText);
  commandTable.registerCommand("cmd_MSItextCmd",        msiDoSomething);
  commandTable.registerCommand("cmd_MSIfractionCmd",    msiFraction);
  commandTable.registerCommand("cmd_MSIradicalCmd",     msiRadical);
  commandTable.registerCommand("cmd_MSIrootCmd",        msiRoot);
  commandTable.registerCommand("cmd_MSIsupCmd",         msiSuperscript);
  commandTable.registerCommand("cmd_MSIsubCmd",         msiSubscript);
  commandTable.registerCommand("cmd_MSItensorCmd",      msiDoSomething);
  commandTable.registerCommand("cmd_MSIsinCmd",         msiSin);
  commandTable.registerCommand("cmd_MSIcosCmd",         msiCos);
  commandTable.registerCommand("cmd_MSItanCmd",         msiTan);
  commandTable.registerCommand("cmd_MSIlnCmd",          msiLn);
  commandTable.registerCommand("cmd_MSIlogCmd",         msiLog);
  commandTable.registerCommand("cmd_MSIexpCmd",         msiExp);
  commandTable.registerCommand("cmd_MSImathnameCmd",    msiMathname);
  commandTable.registerCommand("cmd_MSIMatrixCmd",      msiMatrix);
  commandTable.registerCommand("cmd_MSIparenCmd",       msiParen);
  commandTable.registerCommand("cmd_MSIbracketCmd",     msiBracket);
  commandTable.registerCommand("cmd_MSIbraceCmd",       msiBrace);
  commandTable.registerCommand("cmd_MSIabsvalueCmd",    msiAbsValue);
  commandTable.registerCommand("cmd_MSInormCmd",        msiNorm);
  commandTable.registerCommand("cmd_MSIsymbolCmd",       msiSymbol);
  commandTable.registerCommand("cmd_MSIColorsCmd",       msiColors);
  commandTable.registerCommand("cmd_MSIgenBracketsCmd",  msiGenBrackets);
  commandTable.registerCommand("cmd_MSIbinomialsCmd",    msiBinomials);
  commandTable.registerCommand("cmd_MSIoperatorsCmd",    msiOperators);
  commandTable.registerCommand("cmd_MSIdecorationsCmd",  msiDecorations);
  commandTable.registerCommand("cmd_MSIunitsCommand",    msiUnitsDialog);

  commandTable.registerCommand("cmd_MSIreviseFractionCmd",     msiReviseFractionCmd);
  commandTable.registerCommand("cmd_MSIreviseRadicalCmd",      msiReviseRadicalCmd);
//  commandTable.registerCommand("cmd_MSIreviseScriptsCmd",      msiReviseScriptsCmd);
  commandTable.registerCommand("cmd_MSIreviseMatrixCmd",       msiReviseMatrixCmd);  //Need to implement!
  commandTable.registerCommand("cmd_MSIreviseMatrixCellCmd",   msiReviseMatrixCmd);
  commandTable.registerCommand("cmd_MSIreviseMatrixCellGroupCmd", msiReviseMatrixCmd);
  commandTable.registerCommand("cmd_MSIreviseMatrixRowsCmd",   msiReviseMatrixCmd);
  commandTable.registerCommand("cmd_MSIreviseMatrixColsCmd",   msiReviseMatrixCmd);
  commandTable.registerCommand("cmd_MSIreviseTensorCmd",       msiDoSomething);
  commandTable.registerCommand("cmd_MSIreviseMathnameCmd",     msiReviseMathnameCmd);
//    commandTable.registerCommand("cmd_MSIreviseSymbolCmd",    msiReviseSymbolCmd);
  commandTable.registerCommand("cmd_MSIreviseGenBracketsCmd",  msiReviseGenBracketsCmd);
	commandTable.registerCommand("cmd_MSIreviseBinomialsCmd",    msiReviseBinomialsCmd);
	commandTable.registerCommand("cmd_MSIreviseOperatorsCmd",    msiReviseOperatorsCmd);
	commandTable.registerCommand("cmd_MSIreviseDecorationsCmd",  msiReviseDecorationsCmd);
  commandTable.registerCommand("cmd_MSIreviseUnitsCommand",    msiReviseUnitsCommand);


  try {
    gMathStyleSheet = msiColorObj.Format();
  } catch(e) { dump("Error setting up msiColorObj\n");  }

// too slow for debugging.  Turn back on if you want style-correct toolbars
//  // build symbol panels, since overlays don't seem to fire onload handlers
//  doPanelLoad(document.getElementById("symbol.lcGreek"),"mi");
//  doPanelLoad(document.getElementById("symbol.ucGreek"),"mi");
//  doPanelLoad(document.getElementById("symbol.binOp"),"mo");
//  doPanelLoad(document.getElementById("symbol.binRel"),"mo");
//  doPanelLoad(document.getElementById("symbol.negRel"),"mo");
//  doPanelLoad(document.getElementById("symbol.arrow"),"mo");
//  doPanelLoad(document.getElementById("symbol.misc"),"mi");
//  doPanelLoad(document.getElementById("symbol.delims"),"mo");
}  

function msiSetupMSIMathMenuCommands(editorElement)
{
  var commandTable = msiGetComposerCommandTable(editorElement);
  
  //dump("Registering msi math menu commands\n");
  commandTable.registerCommand("cmd_MSIinlineMathCmd",  msiInlineMath);
  commandTable.registerCommand("cmd_MSIdisplayMathCmd", msiDisplayMath);
  commandTable.registerCommand("cmd_MSImathtext",       msiToggleMathText);
  commandTable.registerCommand("cmd_MSIfractionCmd",    msiFraction);
  commandTable.registerCommand("cmd_MSIradicalCmd",     msiRadical);
  commandTable.registerCommand("cmd_MSIrootCmd",        msiRoot);
  commandTable.registerCommand("cmd_MSIsupCmd",         msiSuperscript);
  commandTable.registerCommand("cmd_MSIsubCmd",         msiSubscript);
  commandTable.registerCommand("cmd_MSItensorCmd",      msiDoSomething);
  commandTable.registerCommand("cmd_MSIsinCmd",         msiSin);
  commandTable.registerCommand("cmd_MSIcosCmd",         msiCos);
  commandTable.registerCommand("cmd_MSItanCmd",         msiTan);
  commandTable.registerCommand("cmd_MSIlnCmd",          msiLn);
  commandTable.registerCommand("cmd_MSIlogCmd",         msiLog);
  commandTable.registerCommand("cmd_MSIexpCmd",         msiExp);
  commandTable.registerCommand("cmd_MSImathnameCmd",    msiMathname);
  commandTable.registerCommand("cmd_MSIMatrixCmd",      msiMatrix);
  commandTable.registerCommand("cmd_MSIparenCmd",       msiParen);
  commandTable.registerCommand("cmd_MSIbracketCmd",     msiBracket);
  commandTable.registerCommand("cmd_MSIbraceCmd",       msiBrace);
  commandTable.registerCommand("cmd_MSIabsvalueCmd",    msiAbsValue);
  commandTable.registerCommand("cmd_MSInormCmd",        msiNorm);
  commandTable.registerCommand("cmd_MSIsymbolCmd",       msiSymbol);
  commandTable.registerCommand("cmd_MSIColorsCmd",       msiColors);
  commandTable.registerCommand("cmd_MSIgenBracketsCmd",  msiGenBrackets);
  commandTable.registerCommand("cmd_MSIbinomialsCmd",    msiBinomials);
  commandTable.registerCommand("cmd_MSIoperatorsCmd",    msiOperators);
  commandTable.registerCommand("cmd_MSIdecorationsCmd",  msiDecorations);
  commandTable.registerCommand("cmd_MSIunitsCommand",    msiUnitsDialog);

  commandTable.registerCommand("cmd_MSIreviseFractionCmd",     msiReviseFractionCmd);
  commandTable.registerCommand("cmd_MSIreviseRadicalCmd",      msiReviseRadicalCmd);
//  commandTable.registerCommand("cmd_MSIreviseScriptsCmd",      msiReviseScriptsCmd);
  commandTable.registerCommand("cmd_MSIreviseMatrixCmd",       msiReviseMatrixCmd);
  commandTable.registerCommand("cmd_MSIreviseMatrixCellCmd",   msiReviseMatrixCmd);
  commandTable.registerCommand("cmd_MSIreviseMatrixCellGroupCmd", msiReviseMatrixCmd);
  commandTable.registerCommand("cmd_MSIreviseMatrixRowsCmd",   msiReviseMatrixCmd);
  commandTable.registerCommand("cmd_MSIreviseMatrixColsCmd",   msiReviseMatrixCmd);
  commandTable.registerCommand("cmd_MSIreviseTensorCmd",       msiDoSomething);
  commandTable.registerCommand("cmd_MSIreviseMathnameCmd",     msiReviseMathnameCmd);
//    commandTable.registerCommand("cmd_MSIreviseSymbolCmd",    msiReviseSymbolCmd);
  commandTable.registerCommand("cmd_MSIreviseGenBracketsCmd",  msiReviseGenBracketsCmd);
	commandTable.registerCommand("cmd_MSIreviseBinomialsCmd",    msiReviseBinomialsCmd);
	commandTable.registerCommand("cmd_MSIreviseOperatorsCmd",    msiReviseOperatorsCmd);
	commandTable.registerCommand("cmd_MSIreviseDecorationsCmd",  msiReviseDecorationsCmd);
  commandTable.registerCommand("cmd_MSIreviseUnitsCommand",    msiReviseUnitsCommand);

//  try {
//    editorElement.mgMathStyleSheet = msiColorObj.FormatStyleSheet(editorElement);
//  } catch(e) { dump("Error setting up msiColorObj\n"); msiKludgeLogString("Error setting up msiColorObj\n"); }

// too slow for debugging.  Turn back on if you want style-correct toolbars
//  // build symbol panels, since overlays don't seem to fire onload handlers
//  doPanelLoad(document.getElementById("symbol.lcGreek"),"mi");
//  doPanelLoad(document.getElementById("symbol.ucGreek"),"mi");
//  doPanelLoad(document.getElementById("symbol.binOp"),"mo");
//  doPanelLoad(document.getElementById("symbol.binRel"),"mo");
//  doPanelLoad(document.getElementById("symbol.negRel"),"mo");
//  doPanelLoad(document.getElementById("symbol.arrow"),"mo");
//  doPanelLoad(document.getElementById("symbol.misc"),"mi");
//  doPanelLoad(document.getElementById("symbol.delims"),"mo");
}  

function goUpdateMSIMathMenuItems(commandset)
{
  return;  //ljh todo -- do something reasonable
}

function msiGoUpdateMSIMathMenuItems(commandset)
{
  return;  //ljh todo -- do something reasonable
}

// like doStatefulCommand()
function doParamCommand(commandID, newValue)
{
  var commandNode = document.getElementById(commandID);
  if (commandNode)
      commandNode.setAttribute("value", newValue);
  if (!msiCurrEditorSetFocus(window)  && ("gContentWindow" in window) && window.gContentWindow != null)
    gContentWindow.focus();   // needed for command dispatch to work

  try
  {
    var cmdParams = newCommandParams();
    if (!cmdParams) return;

    cmdParams.setStringValue("value", newValue);
//    goDoCommandParams(commandID, cmdParams);
    msiGoDoCommandParams(commandID, cmdParams);
  } catch(e) { dump("error thrown in doParamCommand: "+e+"\n"); }
}

//ljh
var msiDoSomething =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    dump("msiDoSomething():  ie not implemented.\n");
    return;
  }
};


var msiToggleMathText =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement(window);
    var editor = msiGetEditor(editorElement);
    insertinlinemath();
    toggleMathText(editor);
    dump("called msiToggleMathText\n");
    //dump("Clearing math mode is not implemented.\n");
    return;
  }
};


var msiFraction =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement(window);
    insertfraction("", "", editorElement);
  }
};

var msiReviseFractionCmd = 
{
  isCommandEnabled: function(aCommand, dummy)
  { return true; },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon)
  {
    var editorElement = msiGetActiveEditorElement(window);
    var theFrac = msiGetReviseObjectFromCommandParams(aParams);
    if ( (editorElement != null) && (theFrac != null) )
    {
      var fractionData = new Object();
      fractionData.reviseObject = theFrac;
//      var argArray = [fractionData];
//      msiOpenModelessPropertiesDialog("chrome://prince/content/fractionProperties.xul", "_blank", "chrome,close,titlebar,dependent",
//                                        editorElement, "cmd_MSIreviseFractionCmd", theFrac, argArray);
      var dlgWindow = msiDoModelessPropertiesDialog("chrome://prince/content/fractionProperties.xul", "_blank", "chrome,close,titlebar,dependent",
                                                     editorElement, "cmd_MSIreviseFractionCmd", theFrac, fractionData);
    }
  },

  doCommand: function(aCommand)
  {
  }
};

var msiRoot =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement(window);
    insertroot(editorElement);
  }
};

var msiRadical =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement(window);
    insertradical(editorElement);
  }
};


var msiReviseRadicalCmd =
{
  isCommandEnabled: function(aCommand, dummy)
  { return true; },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon)
  {
    var editorElement = msiGetActiveEditorElement(window);
    var theRadical = msiGetReviseObjectFromCommandParams(aParams);
    if ( (editorElement != null) && (theRadical != null) )
    {
      var radicalData = new Object();
      radicalData.reviseObject = theRadical;
//      var argArray = [radicalData];
//      msiOpenModelessPropertiesDialog("chrome://prince/content/radicalProperties.xul", "_blank", "chrome,close,titlebar,dependent",
//                                        editorElement, "cmd_MSIreviseRadicalCmd", theRadical, argArray);
      var dlgWindow = msiDoModelessPropertiesDialog("chrome://prince/content/radicalProperties.xul", "_blank", "chrome,close,titlebar,dependent",
                                                     editorElement, "cmd_MSIreviseRadicalCmd", theRadical, radicalData);
    }
//    AlertWithTitle("mathmlOverlay.js", "In msiReviseRadicalCmd, trying to revise radical, dialog unimplemented.");
  },

  doCommand: function(aCommand)
  {
  }
};


var msiSuperscript =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement(window);
    insertsup(editorElement);
  }
};

var msiSubscript =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement(window);
    insertsub(editorElement);
  }
};


var msiInlineMath =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement(window);
    insertinlinemath(editorElement);
  }
};

var msiDisplayMath =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement(window);
    insertdisplay(editorElement);
  }
};

var msiSin =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement(window);
    insertmathname("sin", editorElement);
  }
};

var msiCos =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement(window);
    insertmathname("cos", editorElement);
  }
};

var msiTan =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement(window);
    insertmathname("tan", editorElement);
  }
};

var msiLn =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement(window);
    insertmathname("ln", editorElement);
  }
};

var msiLog =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement(window);
    insertmathname("log", editorElement);
  }
};

var msiExp =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement(window);
    insertmathname("exp", editorElement);
  }
};

var msiMathname =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement(window);
    doMathnameDlg(editorElement, "cmd_MSImathnameCmd", this);
  }
};

var msiReviseMathnameCmd =
{
  isCommandEnabled: function(aCommand, dummy)
  { return true; },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon)
  {
    var editorElement = msiGetActiveEditorElement(window);
    var theMathname = msiGetReviseObjectFromCommandParams(aParams);

    var mathNameData = new Object();
    mathNameData.reviseObject = theMathname;
//    var argArray = [mathNameData];
//    msiOpenModelessPropertiesDialog("chrome://prince/content/MathmlMathname.xul", "_blank", "chrome,close,titlebar,dependent",
//                                      editorElement, "cmd_MSIreviseMathnameCmd", theMathname, argArray);
    var dlgWindow = msiDoModelessPropertiesDialog("chrome://prince/content/mathmlMathName.xul", "_blank", "chrome,close,titlebar,dependent",
                                                     editorElement, "cmd_MSIreviseMathnameCmd", theMathname, mathNameData);
  },

  doCommand: function(aCommand)
  {
  }
};

var msiParen =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement(window);
    insertfence("(", ")", editorElement);
  }
};

var msiBracket =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement(window);
    insertfence("[", "]", editorElement);
  }
};

var msiBrace =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement(window);
    insertfence("{", "}", editorElement);
  }
};

var msiAbsValue =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement(window);
    insertfence("|", "|", editorElement);
  }
};

var msiNorm =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement(window);
    insertfence("\u2016", "\u2016", editorElement);
  }
};

var msiSymbol =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},

  doCommandParams: function(aCommand, aParams, aRefCon)
  {
    insertsymbol( aParams.getStringValue("value") );
  },

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement(window);
    dump("msiSymbol::doCommand HuH?\n");
  }
};

var msiMatrix =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement(window);
    doMatrixDlg(editorElement);
  }
};

//"cmd_MSIreviseMatrixCellCmd", "cmd_MSIreviseMatrixRowsCmd", "cmd_MSIreviseMatrixColsCmd" also go through here.
//  (Code should pay attention to "aCommand"!!)
var msiReviseMatrixCmd =
{
  isCommandEnabled: function(aCommand, dummy)
  { return true; },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon)
  {
    var editorElement = msiGetActiveEditorElement(window);
    var theMatrixData = msiGetPropertiesDataFromCommandParams(aParams);
//    var theMatrixData = msiGetReviseObjectFromCommandParams(aParams);
//    AlertWithTitle("mathmlOverlay.js", "In msiReviseMatrixCmd, trying to revise matrix, dialog unimplemented.");
    var theData = { reviseCommand : aCommand, reviseData : theMatrixData };
    var dlgWindow = msiDoModelessPropertiesDialog("chrome://prince/content/msiEdTableProps.xul", "_blank", "chrome,resizable,close,titlebar,dependent",
                                                     editorElement, aCommand, this, theData);
  },

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement(window);
    var editor = msiGetEditor(editorElement);
    var theMatrixData = new msiTablePropertiesObjectData();
    theMatrixData.initFromSelection(editor.selection, editorElement);
//    var theMatrixData = msiGetPropertiesObjectFromSelection(editorElement);
    var theData = { reviseCommand : aCommand, reviseData : theMatrixData };
    var dlgWindow = msiDoModelessPropertiesDialog("chrome://prince/content/msiEdTableProps.xul", "_blank", "chrome,resizable,close,titlebar,dependent",
                                                     editorElement, aCommand, this, theData);
  }
};


var msiColors =
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
    doColorsDlg(editorElement);
  }
};

var msiGenBrackets = 
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
    doBracketsDlg("(", ")", "", "cmd_MSIgenBracketsCmd", editorElement, this);
  }
};

var msiReviseGenBracketsCmd =
{
  isCommandEnabled: function(aCommand, dummy)
  { return true; },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon)
  {
    var editorElement = msiGetActiveEditorElement(window);
    var theBrackets = msiGetReviseObjectFromCommandParams(aParams);
    var bracketData = new Object();
    bracketData.reviseObject = theBrackets;
//    var argArray = [bracketData];
//    msiOpenModelessPropertiesDialog("chrome://prince/content/Brackets.xul", "_blank", "chrome,close,titlebar,dependent",
//                                      editorElement, "cmd_MSIreviseGenBracketsCmd", theBrackets, argArray);
    var dlgWindow = msiDoModelessPropertiesDialog("chrome://prince/content/Brackets.xul", "_blank", "chrome,close,titlebar,dependent",
                                                     editorElement, "cmd_MSIreviseGenBracketsCmd", theBrackets, bracketData);
  },

  doCommand: function(aCommand)
  {
  }
};


var msiBinomials = 
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
    doBinomialsDlg("(", ")", "0", "auto", "cmd_MSIbinomialsCmd", editorElement, this);
  }
};


var msiReviseBinomialsCmd =
{
  isCommandEnabled: function(aCommand, dummy)
  { return true; },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon)
  {
    var editorElement = msiGetActiveEditorElement(window);
    var theBinomial = msiGetReviseObjectFromCommandParams(aParams);
    var binomialData = new Object();
    binomialData.reviseObject = theBinomial;
//    var argArray = [binomialData];
//    msiOpenModelessPropertiesDialog("chrome://prince/content/Binomial.xul", "_blank", "chrome,close,titlebar,dependent",
//                                      editorElement, "cmd_MSIreviseBinomialsCmd", theBinomial, argArray);
    var dlgWindow = msiDoModelessPropertiesDialog("chrome://prince/content/Binomial.xul", "_blank", "chrome,close,titlebar,dependent",
                                                     editorElement, "cmd_MSIreviseBinomialsCmd", theBinomial, binomialData);
    
//    AlertWithTitle("mathmlOverlay.js", "In msiReviseBinomialsCmd, trying to revise binomial, dialog unimplemented.");
  },

  doCommand: function(aCommand)
  {
  }
};

var msiOperators = 
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement(window);
    doOperatorsDlg(String.fromCharCode(0x222B), "auto", "auto", "cmd_MSIoperatorsCmd", editorElement, this);
  }
};

var msiReviseOperatorsCmd =
{
  isCommandEnabled: function(aCommand, dummy)
  { return true; },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon)
  {
    var editorElement = msiGetActiveEditorElement(window);
    var theOperator = msiGetReviseObjectFromCommandParams(aParams);
    var operatorData = new Object();
    operatorData.reviseObject = theOperator;
//    var argArray = [operatorData];
//    msiOpenModelessPropertiesDialog("chrome://prince/content/Operators.xul", "_blank", "chrome,close,titlebar,dependent",
//                                      editorElement, "cmd_MSIreviseOperatorsCmd", theOperator, argArray);
    var dlgWindow = msiDoModelessPropertiesDialog("chrome://prince/content/Operators.xul", "_blank", "chrome,close,titlebar,dependent",
                                                     editorElement, "cmd_MSIreviseOperatorsCmd", theOperator, operatorData);
  },

  doCommand: function(aCommand)
  {
  }
};

var msiDecorations = 
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement(window);
    doDecorationsDlg(String.fromCharCode(0x00AF), "", "", "cmd_MSIdecorationsCmd", editorElement, this);
  }
};

var standardDecorAboveStrings = 
  [ String.fromCharCode(0x00AF), String.fromCharCode(0x2190), String.fromCharCode(0x2192),
    String.fromCharCode(0x2194), String.fromCharCode(0xFE37), String.fromCharCode(0x0302),
    String.fromCharCode(0x02DC) ];

var standardDecorBelowStrings = 
  [ String.fromCharCode(0x0332), String.fromCharCode(0x2190), String.fromCharCode(0x2192),
    String.fromCharCode(0x2194), String.fromCharCode(0xFE38) ];

var standardAroundDecorNotationStrings = 
  { frame : {mNotation: "box", mType: "frame"},
    fbox : {mNotation: "box", mType: "fbox"},
    roundedbox : {mNotation: "roundedbox"} };

function msiGetMEncloseNotationAndTypeFromDecorString(decorationAroundStr)
{
  if (decorationAroundStr in standardAroundDecorNotationStrings)
    return standardAroundDecorNotationStrings[decorationAroundStr];
  return null;
}

function msiGetDecorStringFromMEncloseNotationAndType(notationSpec, typeSpec)
{
  for (var aDecor in standardAroundDecorNotationStrings)
  {
    if (standardAroundDecorNotationStrings[aDecor].mNotation == notationSpec)
    {
      if ( !("mType" in standardAroundDecorNotationStrings[aDecor]) || (standardAroundDecorNotationStrings[aDecor].mType == typeSpec) )
        return aDecor;
    }
  }
  return "";
}

//This function is a utility used both in the reviseDecoration function (called when the dialog is accepted) and within the dialog
//  to set the dialog data from the object.
function extractDataFromDecoration(decorationNode, decorData)
{
  var baseDecorNode = decorationNode;
  var underDecorChild, overDecorChild;

  function extractDecorationTextFromOverUnder(aChild, bOver)
  {
    var theText = "";
    switch( msiGetBaseNodeName(aChild))
    {
      case "mo":
        theText = msiNavigationUtils.getLeafNodeText(aChild);
        if (bOver)
        {
          if (standardDecorAboveStrings.indexOf(theText) < 0)
            theText = "label";
        }
        else
        {
          if (standardDecorBelowStrings.indexOf(theText) < 0)
            thetext = "label";
        }
        return theText;
      break;
      default:
        //what?? I think we must assume that these are what we call "labels" - and encode using that string.
        return "label";
      break;
    }
    return null;
  }

  switch( msiGetBaseNodeName(decorationNode) )
  {
    case "menclose":
      var notationSpec = decorationNode.getAttribute("notation");
      var typeSpec = decorationNode.getAttribute("type");
      var childNode = msiNavigationUtils.getSingleSignificantChild(decorationNode);
      if (childNode)
        baseDecorNode = extractDataFromDecoration(childNode, decorData);
      if (!baseDecorNode)
        baseDecorNode = decorationNode;
//      else
//      {
//        aboveStr = null;
//        belowStr = null;
//      }
      decorData.aroundStr = msiGetDecorStringFromMEncloseNotationAndType(notationSpec, typeSpec);
    break;

    case "mover":
      overDecorChild = msiNavigationUtils.getIndexedSignificantChild(decorationNode, 1);  //2nd child - expects 0-based index
      decorData.aboveStr = String(extractDecorationTextFromOverUnder(overDecorChild, true));
//      belowStr = null;
//      aroundStr = null;
    break;

    case "munder":
      underDecorChild = msiNavigationUtils.getIndexedSignificantChild(decorationNode, 1);  //2nd child - expects 0-based index
      decorData.belowStr = String(extractDecorationTextFromOverUnder(underDecorChild, false));
//      aboveStr = null;
//      aroundStr = null;
    break;

    case "munderover":
      overDecorChild = msiNavigationUtils.getIndexedSignificantChild(decorationNode, 2);  //3rd child - expects 0-based index
      decorData.aboveStr = String(extractDecorationTextFromOverUnder(overDecorChild, true));
      underDecorChild = msiNavigationUtils.getIndexedSignificantChild(decorationNode, 1);  //2nd child - expects 0-based index
      decorData.belowStr = String(extractDecorationTextFromOverUnder(underDecorChild, false));
//      aroundStr = null;
    break;

    default:
      baseDecorNode = null;  //Anything else isn't a decoration
    break;
  }

  return baseDecorNode;
}

var msiReviseDecorationsCmd =
{
  isCommandEnabled: function(aCommand, dummy)
  { return true; },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon)
  {
    var editorElement = msiGetActiveEditorElement(window);
    var theDecoration = msiGetReviseObjectFromCommandParams(aParams);
    var decorationData = new Object();
    decorationData.reviseObject = theDecoration;
//    var aboveStr, belowStr, aroundStr;
//    var coreDecoration = extractDataFromDecoration(theDecoration, aboveStr, belowStr, aroundStr);
//    doDecorationsDlg(aboveStr, belowStr, aroundStr, editorElement, theDecoration);
//    AlertWithTitle("mathmlOverlay.js", "In msiReviseDecorationsCmd, trying to revise decoration, dialog unimplemented.");
////    reviseFraction(editorElement, theFrac);

    var dlgWindow = msiDoModelessPropertiesDialog("chrome://prince/content/Decorations.xul", "_blank", "chrome,resizable, close,titlebar,dependent",
                                                     editorElement, "msiReviseDecorationsCmd", theDecoration, decorationData);
  },

  doCommand: function(aCommand)
  {
  }
};

var msiUnitsDialog = 
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement(window);
    doUnitsDlg("", "cmd_MSIunitsCommand", editorElement, this);
  }
};

var msiReviseUnitsCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  { return true; },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon)
  {
    var editorElement = msiGetActiveEditorElement(window);
    var theUnit = msiGetReviseObjectFromCommandParams(aParams);
//    AlertWithTitle("mathmlOverlay.js", "In msiReviseUnitsCmd, trying to revise unit, dialog unimplemented.");
//    doUnitsDlg("", "cmd_MSIunitsCommand", editorElement, this);
    try
    {
      var unitsData = new Object();
      unitsData.reviseObject = theUnit;
//      var argArray = [unitsData];
//      msiOpenModelessPropertiesDialog("chrome://prince/content/mathUnitsDialog.xul", "_blank", "chrome,close,titlebar,dependent",
//                                        editorElement, "cmd_MSIreviseUnitsCmd", theUnit, argArray);
      var dlgWindow = msiDoModelessPropertiesDialog("chrome://prince/content/mathUnitsDialog.xul", "_blank", "chrome,close,titlebar,dependent",
                                                     editorElement, "cmd_MSIreviseUnitsCmd", theUnit, unitsData);
    }
    catch(exc) { dump("In msiReviseUnitsCommand, exception: " + exc + ".\n"); }
  },

  doCommand: function(aCommand)
  {
  }
};

//const mmlns    = "http://www.w3.org/1998/Math/MathML";
//const xhtmlns  = "http://www.w3.org/1999/xhtml";

function insertinlinemath(editorElement) 
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement(window);
  var editor = msiGetEditor(editorElement);
  try 
  {
    var mathmlEditor = editor.QueryInterface(Components.interfaces.msiIMathMLEditor);
    mathmlEditor.InsertInlineMath();
    editorElement.contentWindow.focus();
  } 
  catch (e) 
  {
  }
}

function insertdisplay(editorElement) 
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement(window);
  var editor = msiGetEditor(editorElement);
  try 
  {
    var mathmlEditor = editor.QueryInterface(Components.interfaces.msiIMathMLEditor);
    mathmlEditor.InsertDisplay();
    editorElement.contentWindow.focus();   // needed for command dispatch to work
  } 
  catch (e) 
  {
  }
}

function insertsup(editorElement) 
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement(window);
  var editor = msiGetEditor(editorElement);
  try 
  {
    var mathmlEditor = editor.QueryInterface(Components.interfaces.msiIMathMLEditor);
    mathmlEditor.InsertSuperscript();
    editorElement.contentWindow.focus();   // needed for command dispatch to work
  } 
  catch (e) 
  {
  }
}

function insertsub(editorElement) 
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement(window);
  var editor = msiGetEditor(editorElement);
  try 
  {
    var mathmlEditor = editor.QueryInterface(Components.interfaces.msiIMathMLEditor);
    mathmlEditor.InsertSubscript();
    editorElement.contentWindow.focus();   // needed for command dispatch to work
  } 
  catch (e) 
  {
  }
}

function insertfraction(lineSpec, sizeSpec, editorElement) 
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement(window);
  var editor = msiGetEditor(editorElement);
  try 
  {
    var mathmlEditor = editor.QueryInterface(Components.interfaces.msiIMathMLEditor);
    var sizeFlags = Components.interfaces.msiIMMLEditDefines.MO_ATTR_autoSize;
    if (sizeSpec == "small")
      sizeFlags = Components.interfaces.msiIMMLEditDefines.MO_ATTR_smallSize;
    else if (sizeSpec == "big")
      sizeFlags = Components.interfaces.msiIMMLEditDefines.MO_ATTR_displaySize;
    mathmlEditor.InsertFraction(lineSpec, sizeFlags);
    editorElement.contentWindow.focus();
  } 
  catch (e) 
  {
  }
}

function reviseFraction(theFraction, lineSpec, sizeSpec, editorElement)
{
  if (!theFraction || !editorElement)
  {
    dump("Entering reviseFraction with a null editorElement or fraction! Aborting...\n");
    return null;
  }
  var retVal = theFraction;
  var editor = msiGetEditor(editorElement);
  var realFrac = msiNavigationUtils.getWrappedObject(theFraction, "mfrac");
  if (realFrac == null)
  {
    AlertWithTitle("mathmlOverlay.js", "Problem in reviseFraction! No fraction found in node passed in...\n");
    return theFraction;
  }

  editor.beginTransaction();

  try
  {
//    var mathmlEditor = editor.QueryInterface(Components.interfaces.msiIMathMLEditor);
//      realFrac.setAttribute("linethickness", lineSpec);
    msiEditorEnsureElementAttribute(realFrac, "linethickness", lineSpec, editor);

    var styleObj = new Object();
    if (sizeSpec == "small")
      styleObj["displaystyle"] = "false";
    else if (sizeSpec == "big")
      styleObj["displaystyle"] = "true";
    else
      styleObj["displaystyle"] = "";
    retVal = applyMathStyleToObject(styleObj, "mfrac", theFraction, editor);

//    AlertWithTitle("mathmlOverlay.js", "In reviseFraction, functionality needs to be implemented.");
//    mathmlEditor.InsertFraction(lineSpec, sizeFlags);
    editorElement.contentWindow.focus();
  }
  catch(e)
  {
    dump("Exception in mathmlOverlay.js, in reviseFraction; exception is [" + e + "].\n");
  }

  editor.endTransaction();

  return retVal;
}

function insertBinomial(openingBracket, closingBracket, lineSpec, sizeSpec, editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement(window);
  var editor = msiGetEditor(editorElement);
  try 
  {
    var mathmlEditor = editor.QueryInterface(Components.interfaces.msiIMathMLEditor);
    var sizeFlags = Components.interfaces.msiIMMLEditDefines.MO_ATTR_autoSize;
    if (sizeSpec == "small")
      sizeFlags = Components.interfaces.msiIMMLEditDefines.MO_ATTR_smallSize;
    else if (sizeSpec == "big")
      sizeFlags = Components.interfaces.msiIMMLEditDefines.MO_ATTR_displaySize;
    if (lineSpec == "undefined")
      lineSpec = "";  
    mathmlEditor.InsertBinomial(openingBracket, closingBracket, lineSpec, sizeFlags);
    editorElement.contentWindow.focus();
  } 
  catch (e) 
  {
  }
}

function reviseBinomial(objectNode, openingBracket, closingBracket, lineSpec, sizeSpec, editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement(window);
  var editor = msiGetEditor(editorElement);
  var retVal = objectNode;

//  editor.enableUndo(true);

  editor.beginTransaction();
  try 
  {
//    var mathmlEditor = editor.QueryInterface(Components.interfaces.msiIMathMLEditor);

    var wrappedBinomialNode = msiNavigationUtils.getWrappedObject(objectNode, "binomial");
    if (wrappedBinomialNode != null)
    {
      var theChildren = msiNavigationUtils.getSignificantContents(wrappedBinomialNode);
      if (theChildren.length > 2)
      {
        var firstNode = theChildren[0];
        var newFirst = null;
        if (openingBracket.length > 0)
          newFirst = msiSetMathTokenText(firstNode, openingBracket, editor);

        var lastNode = theChildren[theChildren.length - 1];
        var newLast = null;
        if (closingBracket.length > 0)
          newLast = msiSetMathTokenText(lastNode, closingBracket, editor);

        var fracNode = theChildren[1];
        if (msiGetBaseNodeName(fracNode) != "mfrac")
          dump("Problem in reviseBinomial - the second node isn't an mfrac, but is a <" + fracNode.nodeName + ">.\n");
        var theLineSpec = null;
        if (lineSpec.length > 0)
          theLineSpec = lineSpec;

//        editor.setAttribute(fracNode, "linethickness", lineSpec);
        msiEditorEnsureElementAttribute(fracNode, "linethickness", lineSpec, editor);

        if (newFirst == null)
//          wrappedBinomialNode.removeChild(firstNode);
          editor.deleteNode(firstNode);
        if (newLast == null)
//          wrappedBinomialNode.removeChild(lastNode);
          editor.deleteNode(lastNode);

        var styleObj = new Object();
        if (sizeSpec == "small")
          styleObj["displaystyle"] = "false";
        else if (sizeSpec == "big")
          styleObj["displaystyle"] = "true";
        else
          styleObj["displaystyle"] = "";
        retVal = applyMathStyleToObject(styleObj, "binomial", objectNode, editor);

      }
      else
        dump("Problem in mathmlOverlay.js, reviseBinomial - supposed binomial node hasn't enough children?\n");
    }
    else
      dump("Problem in mathmlOverlay.js, reviseBinomial - binomial node not found!\n");
    editorElement.contentWindow.focus();
  } 
  catch (e) { dump("Error in mathmlOverlay.js, reviseBinomial! [" + e + "].\n"); }

  editor.endTransaction();

  return retVal;
}

function insertOperator(operator, limitPlacement, sizeSpec, editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement(window);
  var editor = msiGetEditor(editorElement);
  try 
  {
    var mathmlEditor = editor.QueryInterface(Components.interfaces.msiIMathMLEditor);
    var sizeFlags = Components.interfaces.msiIMMLEditDefines.MO_ATTR_autoSize;
    var largeOpFlag = Components.interfaces.msiIMMLEditDefines.MO_ATTR_largeop_T;
    if (sizeSpec == "small")
      sizeFlags = Components.interfaces.msiIMMLEditDefines.MO_ATTR_smallSize;
    else if (sizeSpec == "big")
      sizeFlags = Components.interfaces.msiIMMLEditDefines.MO_ATTR_displaySize;
    var limitFlags = Components.interfaces.msiIMMLEditDefines.MO_ATTR_autoLimits;
    if (limitPlacement == "atRight")
      limitFlags = Components.interfaces.msiIMMLEditDefines.MO_ATTR_atRightLimits;
    else if (limitPlacement == "aboveBelow")
    {
      limitFlags = Components.interfaces.msiIMMLEditDefines.MO_ATTR_aboveBelowLimits;
      limitFlags |= Components.interfaces.msiIMMLEditDefines.MO_ATTR_movablelimits_F;
    }
    else
      limitFlags = Components.interfaces.msiIMMLEditDefines.MO_ATTR_movablelimits_T;
    mathmlEditor.InsertOperator(operator, (sizeFlags | limitFlags | largeOpFlag),
                                "", "", "", "");
    editorElement.contentWindow.focus();
  } 
  catch (e) 
  {
  }
}

function reviseOperator(objectNode, newOperatorStr, limitPlacement, sizeSpec, editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement(window);
  var editor = msiGetEditor(editorElement);

  var retVal = objectNode;

  editor.beginTransaction();

  try 
  {
    var outerOperator = msiNavigationUtils.getWrappedObject(objectNode, "operator");
    var bWrapped = (objectNode != outerOperator);
    var operatorNode = null;
    if (msiGetBaseNodeName(outerOperator) == "mo")
      operatorNode = outerOperator;
    else
      operatorNode = msiNavigationUtils.getEmbellishedOperator(outerOperator);
//    var mathmlEditor = editor.QueryInterface(Components.interfaces.msiIMathMLEditor);
    var limitPlacementStr = "";
    var bMovableLimits = "false";
    if (limitPlacement == "atRight")
      limitPlacementStr = "msiLimitsAtRight";
    else if (limitPlacement == "aboveBelow")
      limitPlacementStr = "msiLimitsAboveBelow";
    else
      bMovableLimits = "true";

    msiEditorEnsureElementAttribute(operatorNode, "movablelimits", bMovableLimits, editor);
    var bLimitPlacementChanged = msiEditorEnsureElementAttribute(operatorNode, "msiLimitPlacement", ((limitPlacementStr.length > 0) ? limitPlacementStr : null), editor);

    //Now try to match the new ones with the old; if they differ on limit placement, we may have to replace <msubsup>/<msub>/<msup>
    //  by <munderover>/<munder>/<mover> or vice versa, while if they differ on size, we have to invoke the mathStyle mechanism to fix it.
    if (bLimitPlacementChanged && (operatorNode != outerOperator))  //So this operator has limits already:
    {
      var newNodeName = oldNodeName;
      var oldNodeName = msiGetBaseNodeName(outerOperator);
      switch(oldNodeName)
      {
        case "mover":
        case "msup":
          if (limitPlacementStr == "msiLimitsAboveBelow")
            newNodeName = "mover";
          else
            newNodeName = "msup";
        break;
        case "munder":
        case "msub":
          if (limitPlacementStr == "msiLimitsAboveBelow")
            newNodeName = "munder";
          else
            newNodeName = "msub";
        break;
        case "munderover":
        case "msubsup":
          if (limitPlacementStr == "msiLimitsAboveBelow")
            newNodeName = "munderover";
          else
            newNodeName = "msubsup";
        break;
      }
      if (newNodeName != oldNodeName)
      {
        var newNode = null;
        if (newNodeName.length > 0)
          newNode = outerOperator.ownerDocument.createElementNS(mmlns, newNodeName);

        var theParent = outerOperator.parentNode;
        if (newNode != null)
        {
//          var theChildren = msiNavigationUtils.getSignificantContents(outerOperator);
//          for (var ix = 0; ix < theChildren.length; ++ix)
////            newNode.appendChild( theChildren[ix].cloneNode(true) );
//            newNode.appendChild( outerOperator.removeChild(theChildren[ix]) );
          msiEditorMoveChildren(newNode, outerOperator, editor)

//          for (var ii = 0; ii < outerOperator.attributes.length; ++ii)
//          {
//            var theAttr = outerOperator.attributes.item(ii);
//            var attrName = msiGetBaseNodeName( theAttr );
//            if (msiElementCanHaveAttribute(newNode, attrName))
//              newNode.setAttributeNode( outerOperator.removeAttributeNode(theAttr) );
//          }
          msiCopyElementAttributes(newNode, outerOperator, editor);

//          theParent.replaceChild(newNode, outerOperator);
          editor.replaceNode(newNode, outerOperator, theParent);
          outerOperator = newNode;
        }
        else  //This clause is apparently useless, and was wrong in any case - corrected, but can't be entered I believe.
        {
//          theParent.removeChild(newNode, outerOperator);
          editor.deleteNode(outerOperator);
          outerOperator = operatorNode;
        }
      }
    }

    var newOp = msiSetMathTokenText(operatorNode, newOperatorStr, editor);
    if (operatorNode == outerOperator)
      outerOperator = newOp;
    if (!bWrapped)
      objectNode = outerOperator;

    var styleObj = new Object();
    if (sizeSpec == "small")
      styleObj["displaystyle"] = "false";
    else if (sizeSpec == "big")
      styleObj["displaystyle"] = "true";
    else
      styleObj["displaystyle"] = "";
    var objTypeStr = "operator";
    if (outerOperator != operatorNode)
      objTypeStr = msiGetBaseNodeName(outerOperator);
    retVal = applyMathStyleToObject(styleObj, objTypeStr, objectNode, editor);
    
    editorElement.contentWindow.focus();
  } 
  catch (e) 
  {
    dump("Error in mathmlOverlay.js, reviseOperator; error is [" + e + "].\n");
  }

  editor.endTransaction();

  return retVal;
}

function msiSetEncloseDecoration(targNode, decorationAroundStr, editor)
{
  var notationSpec = null;
  if (msiGetBaseNodeName(targNode) == "menclose")
  {
    notationSpec = msiGetMEncloseNotationAndTypeFromDecorString(decorationAroundStr);
    if (notationSpec)
    {
      if (notationSpec.mNotation && notationSpec.mNotation.length)
        msiEditorEnsureElementAttribute(targNode, "notation", notationSpec.mNotation, editor);
      if (notationSpec.mType && notationSpec.mType.length)
        msiEditorEnsureElementAttribute(targNode, "type", notationSpec.mType, editor);
    }
  }
  else
    dump("Problem in mathmlOverlay.js, reviseDecoration, msiSetEncloseDecoration! targNode isn't an menclose!\n");
}

function insertDecoration(decorationAboveStr, decorationBelowStr, decorationAroundStr, editorElement)
{
//  alert("Inserting Decoration above: [" + decorationAboveStr + "], below: [" + decorationBelowStr + "], around: [" + decorationAroundStr + "].");
  if (!editorElement)
    editorElement = msiGetActiveEditorElement(window);
  var editor = msiGetEditor(editorElement);
  try 
  {
    var mathmlEditor = editor.QueryInterface(Components.interfaces.msiIMathMLEditor);
    var theParent = editor.selection.focusNode;
    var theOffset = editor.selection.focusOffset;
//    var theParent = msiNavigationUtils.getCommonAncestorForSelection(editor.selection);
    if (decorationAroundStr && decorationAroundStr.length)
    {
      var encloseNode = editor.document.createElementNS(mmlns, "menclose");
      editor.insertNode(encloseNode, theParent, theOffset);
      msiSetEncloseDecoration(encloseNode, decorationAroundStr, editor);
      var childNode = newbox(editor);
      editor.insertNode( childNode, encloseNode, 0 );
      editor.selection.collapse(childNode, 0);
    }
    if ( (decorationAboveStr && decorationAboveStr.length) || (decorationBelowStr && decorationBelowStr.length) )
      mathmlEditor.InsertDecoration(decorationAboveStr, decorationBelowStr);
    editorElement.contentWindow.focus();
  } 
  catch (e) 
  {
  }
}

function reviseDecoration(decorationNode, decorationAboveStr, decorationBelowStr, decorationAroundStr, editorElement)
{
//The plan we want to follow here - and possibly generalize to simplify these functions in the future - is:
//  (i) Determine the base node type, old and new. "Base node" here means a munder, mover, or munderover, which may then be wrapped
//      by a menclose. (A big problem here: menclose isn't implemented in the core! Though it could almost be done at the level of
//      XBL, a true implementation with its own Frame objects would be the way to go.) Note, however, that if the base node - either
//      old or new - is missing, we should use the outer menclose node as the base node. If there is no outer menclose, then - what
//      are we doing here? Removing decorations from something? How to deal with that? (SWP doesn't allow using the revise decorations
//      dialog to remove them, so let's not allow it here either.)
// (ii) If the base node types aren't the same, and a new one is needed, create it. Now you want to move corresponding pieces of the
//      old node to the new one. We do that by iterating through the significant children of the old node and doing a move.
//(iii) Finally we substitute pieces that have changed. Note that in doing this, a label above could be moved to a label below and we'd
//      preserve its contents.
  var editor = msiGetEditor(editorElement);

//*********************************************************//
//The set of internal functions following are intended to potentially be used more generally in math editing.
  function moveAChildNode(targParent, targIndex, srcParent, srcIndex)
  {
    var theChild = msiNavigationUtils.getIndexedSignificantChild(srcParent, srcIndex);
    if (theChild)
    {
      editor.deleteNode(theChild);
      editor.insertNode(theChild, targParent, targIndex);
      editor.insertNode( newbox(editor), srcParent, srcIndex ); //We don't want to mess up the child count of the source
    }
    return theChild;
  }

  function replaceDecorationChild(targParent, targIndex, decorStr)
  {
    var oldChild = msiNavigationUtils.getIndexedSignificantChild(targParent, targIndex);
    var newChild = null;
    if (decorStr == "label")
    {
      if (!msiNavigationUtils.isEmptyInputBox(oldChild))
      {
        editor.deleteNode(oldChild);
        newChild = newbox(editor);
        editor.insertNode(newChild, targParent, targIndex);
      }
    }
    else
    {
      if (msiGetBaseNodeName(oldChild) == "mo")
        oldChild.textContent = decorStr;
      else
      {
        newChild = editor.document.createElementNS(mmlns, "mo");
        newChild.textContent = decorStr;
        editor.insertNode(newChild, targParent, targIndex);
      }
    }
  }

  function moveCorrespondingContents(targNode, srcNode)
  {
    var childContentTable = 
    {
      mover : { base : 1, sup : 2 },
      munder : { base : 1, sub : 2 },
      munderover : { base : 1, sub : 2, sup : 3 },
      menclose : { base : -1 }
    };
    function positionToContentName(aNodeName, nPos)
    {
      if (! (aNodeName in childContentTable) )
        return null;
      for (var aChild in childContentTable[aNodeName])
      {
        if (childContentTable[aNodeName][aChild] == nPos)
          return aChild;
      }
      return null;
    }
    function lastChildPosition(aNodeName)
    {
      var nPos = 0;
      if (aNodeName in childContentTable)
      {
        for (var aChild in childContentTable[aNodeName])
        {
          if (childContentTable[aNodeName][aChild] > nPos)
            nPos = childContentTable[aNodeName][aChild];
        }
      }
      return nPos;
    }

    var newName = msiGetBaseNodeName(targNode);
    var oldName = msiGetBaseNodeName(srcNode);
    var childNode = null;
    var newPos, oldPos;
    var aPosition = null;
    aPosition = positionToContentName(newName, -1);
    if (aPosition)  //so this one has all children together in one place
    {
      if (aPosition in childContentTable[oldName])
      {
        oldPos = childContentTable[oldName][aPosition];
        if (oldPos > 0)  //moving it from a specified position - may be wrapped in an mrow
        {
          childNode = msiNavigationUtils.getIndexedSignificantChild(srcNode, oldPos - 1);
          if (childNode)
          {
            if (msiNavigationUtils.isOrdinaryMRow(childNode))
              msiEditorMoveChildren(targNode, childNode, editor);
            else  //just a regular single node
            {
              editor.deleteNode(childNode);
              editor.insertNode(childNode, targNode, 0);
            }
          }
        }
        else
        {
          childNode = msiNavigationUtils.getIndexedSignificantChild(srcNode, 0);  //We're using "childNode" as a marker for success, so set it to the first child
          if (childNode)
            msiEditorMoveChildren(targNode, srcNode);
        }
      }
      if (!childNode)
      {
        childNode = newbox(editor);  //create an input box at this position
        editor.insertNode(childNode, targNode, 0);
      }
      return targNode;  //Since all of the child nodes are at unspecified positions, there can be nothing else to do.
    }
    for (var nn = 1; nn <= lastChildPosition(newName); ++nn)
    {
      aPosition = positionToContentName(newName, nn);
      if (aPosition)
      {
        if ( (oldName in childContentTable) && (aPosition in childContentTable[oldName]) )
        {
          oldPos = childContentTable[oldName][aPosition];
          if (oldPos < 0)  //this means we're moving all the children of srcNode to the desired position in targNode
          {
            childNode = srcNode.ownerDocument.createElementNS(mmlns, "mrow");
            editor.insertNode(childNode, targNode, nn - 1);
            msiEditorMoveChildren(childNode, srcNode, editor);
          }
          else  //so the old node had a specified position
          {
            childNode = msiNavigationUtils.getIndexedSignificantChild(srcNode, oldPos - 1);
            editor.deleteNode(childNode);
            editor.insertNode(childNode, targNode, nn - 1);
            editor.insertNode( newbox(editor), srcNode, oldPos - 1 );  //Do this to keep the child count of srcNode intact
          }
        }
        if (!childNode)
        {
          childNode = newbox(editor);
          editor.insertNode(childNode, targNode, nn - 1);
        }
      }
    }
  }
//*********************************************************//

  var currNodeName = msiGetBaseNodeName(decorationNode);
  var bWasEnclose = (currNodeName == "menclose");
  var bIsEnclose = (decorationAroundStr && (decorationAroundStr.length > 0) );
  var oldAboveStr, oldBelowStr, oldAroundStr;
  var decorData = new Object();
  var coreDecorNode = extractDataFromDecoration(decorationNode, decorData);
  currNodeName = msiGetBaseNodeName(coreDecorNode);
  var newNodeName = null;
  var newTopName = null;
  var targCoreNode = null;
  var targTopNode = null;
  if (decorationAboveStr && (decorationAboveStr.length > 0) )
  {
    if (decorationBelowStr && (decorationBelowStr.length > 0) )
      newNodeName = "munderover";
    else
      newNodeName = "mover";
  }
  else if (decorationBelowStr && (decorationBelowStr.length > 0) )
    newNodeName = "munder";
  if (!newNodeName)
  {
    if (bIsEnclose)
      newNodeName = "menclose";
    else
    {
      dump("In mathmlOverlay.js, reviseDecoration(); now show no decoration present - this should NOT happen! Just spill the contents out into our parent?\n");
      return;
    }
  }
  if (bIsEnclose)  //So whatever else we're creating has to end up as the child of an menclose.
    newTopName = "menclose";

  if (newNodeName == currNodeName)
  {
    targCoreNode = coreDecorNode;
    newNodeName = null;  //Don't need a new core node
  }
  else if (newNodeName == "menclose")
  {
    if (bWasEnclose)
    {
      targTopNode = targCoreNode = decorationNode;
      newNodeName = null;  //Don't need a new core node
    }
  }
  if (newTopName == "menclose")
  {
    if (bWasEnclose)
    {
      targTopNode = decorationNode;
      newTopName = null;  //Don't need a new menclose node.
    }
  }

  editor.beginTransaction();
  if (newNodeName)
  {
    targCoreNode = decorationNode.ownerDocument.createElementNS(mmlns, newNodeName);
    if (coreDecorNode)
      moveCorrespondingContents(targCoreNode, coreDecorNode);
    if (newNodeName == "menclose")
      targTopNode = targCoreNode;
  }
  //The one weird case to deal with for decorations is where a label above or below is moved to the other position.
  //  Thus we check for newNodeName = "mover" and oldNodeName = "munder" or vice versa with a corresponding string of "label";
  //  if this is found, we move the contents of the label child.
  var bDone = false;
  if ( (newNodeName == "mover") && (currNodeName == "munder") )
  {
    if ( (decorData.belowStr == "label") && (decorationAboveStr == "label") )
    {
      //Copy the old munder contents to the new mover
      moveAChildNode(targCoreNode, 1, coreDecorNode, 1);
      bDone = true;
    }
  }
  else if ( (newNodeName == "munder") && (currNodeName == "mover") )
  {
    if ( (decorData.aboveStr == "label") && (decorationBelowStr == "label") )
    {
      //Copy the old munder contents to the new mover
      moveAChildNode(targCoreNode, 1, coreDecorNode, 1);
      bDone = true;
    }
  }
  if (!bDone)
  {
    switch(msiGetBaseNodeName(targCoreNode))
    {
      case "mover":
        if (!decorData.aboveStr || (decorData.aboveStr != decorationAboveStr))
          replaceDecorationChild(targCoreNode, 1, decorationAboveStr);
      break;
      case "munderover":
        if (!decorData.aboveStr || (decorData.aboveStr != decorationAboveStr))
          replaceDecorationChild(targCoreNode, 2, decorationAboveStr);
        if (!decorData.belowStr || (decorData.belowStr != decorationBelowStr))
          replaceDecorationChild(targCoreNode, 1, decorationBelowStr);
      break;
      case "munder":
        if (!decorData.belowStr || (decorData.belowStr != decorationBelowStr))
          replaceDecorationChild(targCoreNode, 1, decorationBelowStr);
      break;
    }
  }

  if (newTopName && !targTopNode)
    targTopNode = decorationNode.ownerDocument.createElementNS(mmlns, newTopName);
  
  var theParent = decorationNode.parentNode;
  if (targTopNode)
  {
    if (targCoreNode != targTopNode)
    {
      if (targCoreNode != msiNavigationUtils.getSingleSignificantChild(targTopNode, false))
      {
        //Do we need to worry about removing targCoreNode from where it may have been in the document?
        if (targCoreNode.parentNode)
          editor.removeNode(targCoreNode);
        var encloseChildren = msiNavigationUtils.getSignificantContents(targTopNode);
        for (var ix = 0; ix < encloseChildren.length; ++ix)
          editor.removeNode(encloseChildren[ix]);
        editor.insertNode(targCoreNode, targTopNode, 0);
      }
    }
    if (targTopNode != decorationNode)
      editor.replaceNode(targTopNode, decorationNode, theParent);
    if (!decorData.aroundStr || (decorData.aroundStr != decorationAroundStr))
      msiSetEncloseDecoration(targTopNode, decorationAroundStr, editor);
  }
  else
  {
    if (targCoreNode != coreDecorNode)
    {
      theParent = coreDecorNode.parentNode;
      editor.replaceNode(targCoreNode, coreDecorNode, theParent);
    }
  }

  editor.endTransaction();
}

function insertroot(editorElement) 
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement(window);
  var editor = msiGetEditor(editorElement);
  try 
  {
    var mathmlEditor = editor.QueryInterface(Components.interfaces.msiIMathMLEditor);
    mathmlEditor.InsertRoot();
    editorElement.contentWindow.focus();
  } 
  catch (e) 
  {
  }
}

function insertradical(editorElement) 
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement(window);
  var editor = msiGetEditor(editorElement);
  try 
  {
    var mathmlEditor = editor.QueryInterface(Components.interfaces.msiIMathMLEditor);
    mathmlEditor.InsertSqRoot();
    editorElement.contentWindow.focus();
  } 
  catch (e) 
  {
  }
}


function reviseRadical(theRadical, objectName, editorElement)
{
  if (!theRadical || !editorElement)
  {
    dump("Entering reviseRadical with a null editorElement or radical! Aborting...\n");
    return null;
  }
  var retVal = theRadical;
  var editor = msiGetEditor(editorElement);

  var currRoot = msiNavigationUtils.getWrappedObject(theRadical, objectName);
  if (currRoot != null)
    return theRadical;  //it was already of the desired kind - we're done.

  var oldName = "mroot";
  if (objectName == "mroot")
    oldName = "msqrt";
  currRoot = msiNavigationUtils.getWrappedObject(theRadical, oldName);
  if (currRoot == null)
  {
    AlertWithTitle("mathmlOverlay.js", "Problem in reviseFraction! No fraction found in node passed in...\n");
    return theRadical;
  }

  editor.beginTransaction();

  try
  {
//    var mathmlEditor = editor.QueryInterface(Components.interfaces.msiIMathMLEditor);
    var newRoot = theRadical.ownerDocument.createElementNS(mmlns, objectName);
//    var nextSibling = currRoot.nextSibling;
//    if (nextSibling == null)
//      newRoot = currRoot.parentNode.appendChild(newRoot);
//    else
//      newRoot = currRoot.parentNode.insertBefore(newRoot, nextSibling);
    var children = msiNavigationUtils.getSignificantContents(currRoot);
    if (objectName == "mroot")  //Changing a square root to a general root.
    {
      if (children.length == 1)
        msiEditorMoveChild(newRoot, children[0], editor);
      else
      {
        var newRow = currRoot.ownerDocument.createElementNS(mmlns, "mrow");
        msiEditorMoveChildren(newRow, currRoot, editor);
        editor.insertNode(newRow, newRoot, 0);
      }
      editor.insertNode(newbox(editor), newRoot, 1);
    }
    else  //creating a msqrt - any "inferred row" of children is okay, but only want to move children of first child of mroot
    {
      if ( msiNavigationUtils.isOrdinaryMRow(children[0]) )
      {
        msiEditorMoveChildren(newRoot, children[0], editor);
      }
      else
        msiEditorMoveChild(newRoot, children[0], editor); 
    }
    editor.replaceNode(newRoot, currRoot, currRoot.parentNode);
    if (currRoot == retVal)  //radical was top level object being modified; new one should be the return value
      retVal = newRoot;          //(otherwise original object should still be returned)
//    currRoot.parentNode.removeChild(currRoot);

//    AlertWithTitle("mathmlOverlay.js", "In reviseFraction, functionality needs to be implemented.");
//    mathmlEditor.InsertFraction(lineSpec, sizeFlags);
    editorElement.contentWindow.focus();
  }
  catch(e)
  {
    dump("Exception in mathmlOverlay.js, in reviseRadical; exception is [" + e + "].\n");
  }

  editor.endTransaction();

  return retVal;
}


function insertmathname(name, editorElement) 
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement(window);
  var editor = msiGetEditor(editorElement);
  try 
  {
    var mathmlEditor = editor.QueryInterface(Components.interfaces.msiIMathMLEditor);
    mathmlEditor.InsertMathname(name);
    editorElement.contentWindow.focus();
  } 
  catch (e) {dump("In mathmlOverlay.js, insertmathname(" + name + ") for editorElement [" + editorElement.id + "], error: [" + e + "].\n");}
}

function reviseMathname(theMathnameNode, newMathNameData, editorElement)
{
  if (!theMathnameNode || !editorElement)
  {
    dump("Entering reviseMathname with a null editorElement or mathname node! Aborting...\n");
    return null;
  }
  var retVal = theMathnameNode;
  var editor = msiGetEditor(editorElement);

  var wrappedMathName = msiNavigationUtils.getWrappedObject(theMathnameNode, "mathname");
  if ((wrappedMathName == null) || (newMathNameData.val.length == 0))
  {
    AlertWithTitle("mathmlOverlay.js", "Problem in reviseMathName! No fraction found in node passed in...\n");
    return theRadical;
  }

  editor.beginTransaction();

  try
  {
    var newNode = null;
    var oldNodeName = msiGetBaseNodeName(wrappedMathName);
    if (newMathNameData.type == "operator")  //should now be an "mo"
    {
      if (oldNodeName != "mo")
      {
        newNode = wrappedMathName.ownerDocument.createElementNS(mmlns, "mo");
        var newText = wrappedMathName.owerDocument.createTextNode(newMathNameData.val);
        newNode.appendChild(newText);
      }
    }
    else    //all others are "mi"
    {
      if (oldNodeName != "mi")
      {
        newNode = wrappedMathName.ownerDocument.createElementNS(mmlns, "mi");
        var newText = wrappedMathName.owerDocument.createTextNode(newMathNameData.val);
        newNode.appendChild(newText);
      }
    }
  
    if (newNode !=  null)  //Need to replace the old mathname node with the new one
    {
      newNode.setAttribute("msimathname", "true");
      var parent = wrappedMathName.parentNode;
      msiCopyElementAttributes(newNode, wrappedMathName, editor);
      editor.replaceNode(newNode, wrappedMathName, parent);

      if (theMathnameNode == wrappedMathNode)
        theMathnameNode = newNode;
      wrappedMathNode = newNode;  //from here on in we deal with the new one
    }
    else
      wrappedMathName = msiSetMathTokenText(wrappedMathName, newMathNameData.val, editor);

    //Now whether we've inserted a new node or not, we adjust attribute and style values.

    if (newMathNameData.type == "operator")  //should now be an "mo"
    {
      var limitPlacement = "";
      if ( ("limitPlacement" in newMathNameData) && (newMathNameData.limitPlacement != "auto") )
        limitPlacement = newMathNameData.limitPlacement;
      msiEditorEnsureElementAttribute(wrappedMathName, "limitPlacement", limitPlacement, editor);
    }
    else  //an "mi"
    {
      var msiClassAttr = "";
      if (newMathNameData.enginefunction)
        msiClassAttr = "enginefunction";
      msiEditorEnsureElementAttribute(wrappedMathName, "msiclass", msiClassAttr, editor);
    }

    var sizeSpec = "";
    if ( ("size" in newMathNameData) && (newMathNameData.size != "auto") )
      sizeSpec = newMathNameData.size;
    var styleObj = new Object();
    if (sizeSpec == "small")
      styleObj["displaystyle"] = "false";
    else if (sizeSpec == "big")
      styleObj["displaystyle"] = "true";
    else
      styleObj["displaystyle"] = "";

    retVal = applyMathStyleToObject(styleObj, msiGetBaseNodeName(wrappedMathName), theMathnameNode, editor);

    editorElement.contentWindow.focus();
  }  //end "try"
  catch(e)
  {
    dump("Exception in mathmlOverlay.js, in reviseMathname; exception is [" + e + "].\n");
  }

  editor.endTransaction();

  return retVal;
}


function insertmathunit(unitName, editorElement) 
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement(window);
  var editor = msiGetEditor(editorElement);
  var nameData = msiBaseMathUnitsList.getUnitNameData(unitName);

  try 
  {
    var mathmlEditor = editor.QueryInterface(Components.interfaces.msiIMathMLEditor);
    var bUsedAppearance = false;
    if (nameData.appearance != null)
    {
      if (("nodeName" in nameData.appearance) && nameData.appearance.childNodes.length > 0)
      {
//        ADD HERE if !math mathmlEditor.insertMath();
        bUsedAppearance = true;
        for (var ix = 0; ix < nameData.appearance.childNodes.length; ++ix)
        {
          try
          {
            insertfragment(editor, nameData.appearance.childNodes[ix]);
//            editor.insertElementAtSelection(nameData.appearance.childNodes[ix], (ix==0)); //"true" means delete selection - should only apply to the first insertion
          } catch(exc)
          {
            var nodeStr = nameData.appearance.childNodes[ix].nodeName;
            nodeStr += ", " + nameData.appearance.childNodes[ix].nodeValue;
            dump("Exception in insertmathunit, exception is [" + exc + "], trying to insert node [" + nodeStr + "].\n");
          }
        }
      }
    }
    if (!bUsedAppearance)
      mathmlEditor.InsertMathunit(nameData.data);
    editorElement.contentWindow.focus();
  } 
  catch (e) {}
}

function reviseMathUnit(unitNode, newName, editorElement) 
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement(window);
  var editor = msiGetEditor(editorElement);
  var nameData = msiBaseMathUnitsList.getUnitNameData(newName);

  editor.beginTransaction();
  try 
  {
    var wrappedUnitNode = msiNavigationUtils.getWrappedObject(unitNode, "unit");
    var retVal = unitNode;
//    var mathmlEditor = editor.QueryInterface(Components.interfaces.msiIMathMLEditor);
    var bUsedAppearance = false;
    if (nameData.appearance != null)
    {
      var theChildren = null;
      if ("nodeName" in nameData.appearance)
        theChildren = msiNavigationUtils.getSignificantContents(nameData.appearance);

      if (theChildren.length > 0)
      {
        bUsedAppearance = true;
        var insertNode = null;
        if (theChildren.length == 1)
        {
          insertNode = theChildren[0].cloneNode(true);
        }
        else
        {
          insertNode = editor.document.createElementNS(mmlns, "mrow");
          for (var ix = 0; ix < theChildren.length; ++ix)
          {
            insertNode.appendChild( theChildren[ix].cloneNode(true) );
          }
        }
        if (insertNode != null)
        {
          editor.replaceNode(insertNode, wrappedUnitNode, wrappedUnitNode.parentNode);
          if (wrappedUnitNode == unitNode)
            retVal = insertNode;
        }
      }
    }
    if (!bUsedAppearance)
    {
      msiSetMathTokenText(wrappedUnitNode, nameData.data, editor);
    }
    editorElement.contentWindow.focus();
  } 
  catch (e) {dump("Error in mathmlOverlay.js, reviseMathUnit; exception is [" + e + "].\n");}
  editor.endTransaction();

  return retVal;
}

function insertenginefunction(name, editorElement) 
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement(window);
  var editor = msiGetEditor(editorElement);
  try 
  {
    var mathmlEditor = editor.QueryInterface(Components.interfaces.msiIMathMLEditor);
    mathmlEditor.InsertEngineFunction(name);
    editorElement.contentWindow.focus();
  } 
  catch (e) {}
}

function doMathnameDlg(editorElement, commandID, commandHandler)
{
  var o = new Object();
  o.val = "";
//  window.openDialog("chrome://prince/content/MathmlMathname.xul", "mathname", "chrome,close,titlebar,modal", o);
  msiOpenModelessDialog("chrome://prince/content/mathmlMathName.xul", "mathname", "chrome,close,titlebar,dependent",
                                        editorElement, commandID, commandHandler, o);
}

function insertMathnameObject(mathNameObj, editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement(window);
  var editor = msiGetEditor(editorElement);
  if (mathNameObj.val.length > 0)
  {
    if (mathNameObj.enginefunction)
      insertenginefunction(mathNameObj.val, editorElement);
    else if (("appearance" in mathNameObj) && (mathNameObj.appearance != null))
    {
      var insertNodes = mathNameObj.appearance.childNodes;
      var topNode = null;
      if (insertNodes.length > 1)
      {
        topNode = editor.document.createElementNS(mmlns, "mrow");
        for (var ix = 0; ix < insertNodes.length; ++ix)
        {
          var newNode = insertNodes[ix].cloneNode(true);
          topNode.appendChild(newNode);
        }
        editor.insertElementAtSelection(topNode, true);
      }
      else
      {
        topNode = insertNodes[0].cloneNode(true);
      }
      if (topNode != null)
      {
        topNode.setAttribute("msimathname", "true");
        topNode.setAttribute("msimathnameText", mathNameObj.val);
        editor.insertElementAtSelection(topNode, true);  //"true" means delete selection
      }
    }
    else if (mathNameObj.type == "operator")
    {
      var limitPlacement = "auto";
      var sizeSpec = "auto";
      if ("limitPlacement" in mathNameObj)
        limitPlacement = mathNameObj.limitPlacement;
      if ("size" in mathNameObj)
        sizeSpec = mathNameObj.size;
      insertOperator(mathNameObj.val, limitPlacement, sizeSpec, editorElement);
      //need to give it the msimathname="true" attribute also!
      var sel = editor.selection;
      if (sel != null)
      {
        var opNode = editor.getElementOrParentByTagName("mo", sel.focusNode);
        if (opNode == null)  //thus the focus is not in the added operator element - look to the left?
        {
          var nOffset = sel.focusOffset;
          if (nOffset > 0)
          {
            var prevNode = sel.focusNode.childNodes[nOffset - 1];
            if (prevNode && prevNode.nodeName == "mo")
              opNode = prevNode;
          }
        }
        if (opNode != null)
          opNode.setAttribute("msimathname", "true");
//        var focusNode = sel.focusNode;
//        var focusOffset = sel.focusOffset;
//        dump("After inserting math operator, focus node and offset are [" + focusNode.nodeName + ", " + focusOffset + "].\n");
      }
    }
    else
      insertmathname(mathNameObj.val, editorElement);  //these go in as "mi"s
  }
}

function doInsertMathName(aName, editorElement)
{
  var editor = msiGetEditor(editorElement);
  editor.InsertMathname(aName);
}
//  var mathNameObj = msiBaseMathNameList.getMathNameData(aName);
//  if (mathNameObj == null)
//  {
//    mathNameObj = new Object();
//    mathNameObj.type = "function";
//    mathNameObj.val = aName;
//    mathNameObj.enginefunction = false;
//  }
//  insertMathnameObject(mathNameObj, editorElement);
//}

function doMatrixDlg(editorElement)
{
  var o = new Object();
  o.rows = 0;
  o.cols = 0;
  o.rowsignature = "";
  window.openDialog("chrome://prince/content/mathmlMatrix.xul", "matrix", "chrome,close,titlebar,modal", o);
  if (o.rows > 0 && o.cols > 0)
    insertmatrix(o.rows, o.cols, o.rowsignature, editorElement);
}


var msiMathStyleUtils =
{
  //Here we use the "old-fashioned" code of Roger Sidje that "ruleThickness = NSToCoordRound(40.000f/430.556f * xHeight);"
  //The "units" parameter is handled by a msiUnitsList() object - see msiEditorUtilities.js for this.
  getFracLineThickness : function(objectNode, units)
  {
    var xHeight = this.getFontXHeight(objectNode, units);
    if (!isNaN(xHeight))
    {
      return ((xHeight * 40.0) / 430.556);
    }
    return Number.NaN;
  },

  //need to use nsIDOMViewCSS::getComputedStyle() to get font information, then get the xHeight out of that
  getFontXHeight : function(elementNode, units)
  {
    var retVal = Number.NaN;
    if ( (elementNode == null) || (elementNode.ownerDocument == null) )
    {
      dump("In mathmlOverlay.js, msiMathStyleUtils.getFontXHeight(), null owner document or elementNode!\n");
      return retVal;
    }
    try
      {
  //    var docView = elementNode.ownerDocument.QueryInterface(Components.interfaces.nsIDOMDocumentView);
  //    if (!docView)
  //    {
  //      dump("In mathmlOverlay.js, msiMathStyleUtils.getFontXHeight(), can't get nsIDOMDocumentView interface!\n");
  //      return Number.NaN;
  //    }
      var defView = elementNode.ownerDocument.defaultView;
      var docCSS = defView.QueryInterface(Components.interfaces.nsIDOMViewCSS);
      var computedStyle = docCSS.getComputedStyle(elementNode, "");
      var nsIPrimitive = Components.interfaces.nsIDOMCSSPrimitiveValue;
      var unitType = null;
      var theHeight = 0.0;
      switch(units)
      {
        case "cm":         unitType = nsIPrimitive.CSS_CM;             break;
        case "mm":         unitType = nsIPrimitive.CSS_MM;             break;
        case "in":         unitType = nsIPrimitive.CSS_IN;             break;
        case "pt":         unitType = nsIPrimitive.CSS_PT;             break;
        case "px":         unitType = nsIPrimitive.CSS_PX;             break;
        case "em":         theHeight = 1.0;                            break;  //is this right??
        case "ex":         return 1.0;                                 break;
      }
      var xMult = 0.50;
      if (unitType != null)
      {
        var fontHeight = computedStyle.getPropertyCSSValue("font-size");
        theHeight = fontHeight.QueryInterface(nsIPrimitive).getFloatValue(unitType);
        var sizeAdjust = computedStyle.getPropertyCSSValue("font-size-adjust");
        if ( (sizeAdjust != null) && (sizeAdjust.cssValueType == Components.interfaces.nsIDOMCSSValue.CSS_PRIMITIVE_VALUE) )
        {
          try { xMult = sizeAdjust.QueryInterface(nsIPrimitive).getFloatValue(nsIPrimitive.CSSNumber); }
          catch(exc) {dump("Exception trying to get float value from font-size-adjust in msiMathStyleUtils.getFontXHeight, exception [" + exc + "].\n");}
        }
      }
      retVal = xMult * theHeight;
    }
    catch(exc) {dump("In mathmlOverlay.js, msiMathStyleUtils.getFontXHeight(), exception: [" + exc + "].\n");}
    return retVal;
  },

  convertLineThicknessToDialogForm : function(elementNode, theLineSpec)
  {
    switch(theLineSpec)
    {
      case "thick":
      case "0":
      case "":
        return theLineSpec;
      break;
    }

    var retVal = "";
    var valWithUnits = msiGetNumberAndLengthUnitFromString(theLineSpec);
    if (valWithUnits != null)
    {
      var defaultLineSpec = this.getFracLineThickness(elementNode, valWithUnits.unit);
      if (!isNaN(defaultLineSpec))
      {
        if (valWithUnits.number < 0.3 * defaultLineSpec)
          retVal = "0";
        else if (valWithUnits.number >= 2.0 * defaultLineSpec)
          retVal = "thick";
      }
    }
    else
    {
      var lineSpec = parseFloat(theLineSpec);
      if (!isNaN(lineSpec))
      {
        if (lineSpec < 0.3)
          retVal = "0";
        else if (lineSpec > 2.0)
          retVal = "thick";
      }
    }
    return retVal;
  }
};

// maintain color attributes and generate corresponding CSS
var msiColorObj = 
{
  Format: function()
  {
    var res = "data:text/css,";
    if (this.mathColor && this.mathColor.length > 0)
      res += "math { color: "+this.mathColor+"; } ";
    if (this.mathnameColor && this.mathnameColor.length > 0)
      res += "mi[msiMathname=\"true\"] { color: "+this.mathnameColor+"; } ";
    if (this.unitColor && this.unitColor.length > 0)
      res += "[class=\"msi_unit\"]{ color: "+this.unitColor+";} ";
    if (this.mtextColor && this.mtextColor.length > 0)
      res += "mtext { color: "+this.mtextColor+"; } ";
    if (this.matrixColor && this.matrixColor.length > 0) {
      res += "mtd { border-color: "+this.matrixColor+"; } ";
    }
    return res;
  },
};

// Run color dialog.
//XXX This shouldn't happen in Source View or Preview mode.
function doColorsDlg(editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);

  window.openDialog("chrome://prince/content/MathColors.xul", "mathcolors", "chrome,close,titlebar,modal", msiColorObj);
  if (msiColorObj.Cancel)
    return;

  try {
    editor.QueryInterface(nsIEditorStyleSheets);
    editor.removeOverrideStyleSheet(gMathStyleSheet);
    if (editorElement.mgMathStyleSheet != null)
    {
      editor.removeOverrideStyleSheet(editorElement.mgMathStyleSheet);
      editorElement.mgMathStyleSheet = msiColorObj.FormatStyleSheet(editorElement);
      editor.addOverrideStyleSheet(editorElement.mgMathStyleSheet);
    }
    else
    {
      gMathStyleSheet = msiColorObj.Format();
      editor.addOverrideStyleSheet(gMathStyleSheet);
    }
  } catch (e) {
    dump("Something wrong with stylesheet mechanism\n");
  }
}


function doBracketsDlg(leftBrack, rightBrack, sep, commandID, editorElement, commandHandler)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var bracketData = new Object();
  // Eventually we want to initialize this with whatever is in the document, if possible. In other cases, it is
  // an error to initialize it since that defeats any persistence of the dialog state.
  bracketData.leftBracket = ""; //leftBrack;
  bracketData.rightBracket = ""; //rightBrack;
  bracketData.separator = sep;
//  window.openDialog("chrome://prince/content/Brackets.xul", "brackets", "chrome,close,titlebar,modal", bracketData);
  msiOpenModelessDialog("chrome://prince/content/Brackets.xul", "brackets", "chrome,close,titlebar,dependent",
                                        editorElement, commandID, commandHandler, bracketData);
//  if (bracketData.Cancel)
//    return;
//  insertfence(bracketData.leftBracket, bracketData.rightBracket, bracketData.separator, editorElement);
}


function doBinomialsDlg(leftBrack, rightBrack, line, size, commandID, editorElement, commandHandler)
{
  var binomialData = new Object();
  binomialData.leftBracket = leftBrack;
  binomialData.rightBracket = rightBrack;
  if (!leftBrack.length || !rightBrack.length)
    binomialData.withDelimiters = false;
  else
    binomialData.withDelimiters = true;
  binomialData.lineSpec = line;
  binomialData.sizeSpec = size;
//  window.openDialog("chrome://prince/content/Binomial.xul", "binomial", "chrome,close,titlebar,modal", binomialData);
  msiOpenModelessDialog("chrome://prince/content/Binomial.xul", "binomial", "chrome,close,titlebar,dependent",
                                        editorElement, commandID, commandHandler, binomialData);
//  if (binomialData.Cancel)
//    return;
//  if (!binomialData.withDelimiters)
//  {
//    binomialData.leftBracket = "";
//    binomialData.rightBracket = "";
//  }
//  insertBinomial(binomialData.leftBracket, binomialData.rightBracket,
//                        binomialData.lineSpec, binomialData.sizeSpec, editorElement);
////  if (binomialData.withDelimiters)
////    insertfence(binomialData.leftBracket, binomialData.rightBracket, "");
////  insertfraction(binomialData.lineSpec, binomialData.sizeSpec);
}

function doOperatorsDlg(operatorStr, limitPlacement, size, commandID, editorElement, commandHandler)
{
  var operatorData = new Object();
  operatorData.operator = operatorStr;
  operatorData.limitsSpec = limitPlacement;
  operatorData.sizeSpec = size;
  msiOpenModelessDialog("chrome://prince/content/Operators.xul", "_blank", "chrome,close,titlebar,dependent",
                                        editorElement, commandID, commandHandler, operatorData);
//  window.openDialog("chrome://prince/content/Operators.xul", "operators", "chrome,close,titlebar,modal", operatorData);
//  if (operatorData.Cancel)
//    return;
//  insertOperator(operatorData.operator, operatorData.limitsSpec, operatorData.sizeSpec, editorElement);
//  alert("Insert operator [" + operatorData.operator + "] with limit placement [" + operatorData.limitsSpec + "] and size [" + operatorData.sizeSpec + "].");
}

function doDecorationsDlg(decorationAboveStr, decorationBelowStr, decorationAroundStr, commandID, editorElement, commandHandler)
{
  var decorationData = new Object();
  decorationData.decorationAboveStr = decorationAboveStr;
  decorationData.decorationBelowStr = decorationBelowStr;
  decorationData.decorationAroundStr = decorationAroundStr;
  msiOpenModelessDialog("chrome://prince/content/Decorations.xul", "_blank", "chrome,close,titlebar,resizable, dependent",
                                        editorElement, commandID, commandHandler, decorationData);
//  window.openDialog("chrome://prince/content/Decorations.xul", "decorations", "chrome,close,titlebar,modal", decorationData);
//  if (decorationData.Cancel)
//    return;
//  insertDecoration(decorationData.decorationAboveStr, decorationData.decorationBelowStr, decorationData.decorationAroundStr, editorElement);
}


function doUnitsDlg(unitStr, commandID, editorElement, commandHandler)
{
  var unitsData = new Object();
  unitsData.unitString = unitStr;
  msiOpenModelessDialog("chrome://prince/content/mathUnitsDialog.xul", "_blank", "chrome,close,titlebar,dependent",
                                        editorElement, commandID, commandHandler, unitsData);
}

// change symbol panels to MathML so they're styled correctly.
// for some reason, just adding the children to the button doesn't do the job, so we have to build 
// a new button
function doPanelLoad(panel,elementtype)
{
  var children = panel.childNodes;
  for (var i = 0; i < children.length; i++)
  {
    var button = children.item(i);
    var symbol = button.getAttribute("label");
    var tooltip = button.getAttribute("tooltiptext");
    var observes = button.getAttribute("observes");

    var mi = document.createElementNS(mmlns,elementtype);
    mi.appendChild(document.createTextNode(symbol));
    var div = document.createElementNS(xhtmlns,"div");
    div.appendChild(mi);
    var math = document.createElementNS(mmlns,"math");
    math.setAttribute("display","inline");
    math.appendChild(div);
    var newbutton = document.createElement("toolbarbutton");
    newbutton.setAttribute("tooltiptext",tooltip);
    newbutton.setAttribute("observes",observes);
    newbutton.appendChild(math);

    panel.replaceChild(newbutton,button);
  }
}

//In the following, "styleVals" is assumed to be passed in as a key-value type object (i.e., styleVals[key]=value).
function applyMathStyleToObject(styleVals, objType, targ, editor)
{
  //First we need to see whether the "targ" object or a near descendant (or ancestor? shouldn't happen, but may) is an "mstyle".
  //By "near descendant or ancestor" I mean a node which is essentially a wrapper around "targ", or which "targ" is a wrapper around.

  var foundAttrs = new Object();
  var styleNode = null;

//  aNode = targ;
//  while (aNode != null)
//  {
//    var nodeName = msiGetBaseNodeName(aNode);
//    if (nodeName == objType)
//      break;
//    else if (nodeName == "mrow")
//    {
//      if (objType == "fence" && msiNavigationUtils.isFence(aNode))
//        break;
//      if (objType == "binomial" && msiNavigationUtils.isBinomial(aNode))
//        break;
//    }
//    else if (nodeName == "mstyle")
//    {
//      styleNode = aNode;
//    }
//
//    if (!msiNavigationUtils.nodeWrapsContent)
//      aNode = null;
//    else
//    {
//      var nodeContents = msiNavigationUtils.getSignificantContents(aNode);
//      if (nodeContents.length > 1)
//        break;
//      aNode = nodeContents[0];
//    }
//  }

  styleNode = msiNavigationUtils.findStyleEnclosingObj(targ, objType, styleVals, foundAttrs);
  
  var bAddStyleNode = false;
  if (styleNode == null)
  {
    styleNode = targ.ownerDocument.createElement("mstyle");
    bAddStyleNode = true;
  }
  
  for (var attrib in styleVals)
  {
    if ( !(attrib in foundAttrs) || (foundAttrs[attrib] != styleVals[attrib]) )
    {
      msiEditorEnsureElementAttribute(styleNode, attrib, styleVals[attrib], editor);
//      if (styleVals[attrib].length > 0)
//        styleNode.setAttribute(attrib, styleVals[attrib]);
//      else if (styleNode.hasAttribute(attrib))
//        styleNode.removeAttribute(attrib);
    }
    delete styleVals[attrib];
  }

//  if (styleNode.attributes.length == 0)
  if (msiNavigationUtils.isUnnecessaryMStyle(styleNode))    //no more attributes - get rid of the mstyle!
  {
    if (!bAddStyleNode)
    {
      var kid = msiNavigationUtils.getSingleWrappedChild(styleNode);
      if (kid != null)
      {
        var theParentNode = styleNode.parentNode;
//        kid = styleNode.removeChild(kid);
        editor.deleteNode(kid);
        editor.replaceNode(kid, styleNode, theParentNode);
        //NOTE! Here we set "targ" so it can be used as the return value - it's the new revisable object
        targ = kid;
      }
    }
    else
      bAddStyleNode = false;
  }
  else if (bAddStyleNode)
  {
    var theParent = targ.parentNode;
    var newTarg = targ.cloneNode(true);
    styleNode.appendChild(newTarg);
    editor.replaceNode(styleNode, targ, theParent);
  }

  if (bAddStyleNode)
    return styleNode;
  return targ;
}

function insertsymbol(s, editorElement) 
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);
  try 
  {
    var mathmlEditor = editor.QueryInterface(Components.interfaces.msiIMathMLEditor);
    mathmlEditor.InsertSymbol(s.charCodeAt(0));
    editorElement.contentWindow.focus();
  } 
  catch (e) 
  {
    dump("insertsymbol failed: "+e+"\n");
  }
}


function inserttext(s, editorElement) 
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);
  try 
  {
    var plaintextEditor = editor.QueryInterface(Components.interfaces.nsIPlaintextEditor);
    plaintextEditor.insertText(s);
    editorElement.contentWindow.focus();
  } 
  catch (e) 
  {
    dump("inserttext failed: "+e+"\n");
  }
}



function insertfence(left, right, editorElement) 
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement(window);
  var editor = msiGetEditor(editorElement);
  var logStr = "";
  try 
  {
    if (!editorElement)
      logStr = "In insertfence, editor element is null!\n";
    else if (!editor)
      logStr = "In insertfence, editor element [" + editorElement.id + "] has null editor!\n";
    var mathmlEditor = editor.QueryInterface(Components.interfaces.msiIMathMLEditor);
    if (!mathmlEditor)
      logStr = "In insertfence, editor element [" + editorElement.id + "] has editor, but null mathmlEditor!\n";
    mathmlEditor.InsertFence(left, right);
    editorElement.contentWindow.focus();
  } 
  catch (e) 
  {
    AlertWithTitle("Error in insertfence!", e);
    if (logStr.length > 0)
      msiKludgeLogString(logStr);
  }
}

//NOTE!! All the convoluted replacement code used below is necessary due to a bug in Mozilla MathML rendering.
//Changing the textContent of a token node <mo> or <mi> does not reliably force a rerendering.
function reviseFence(fenceNode, left, right, editorElement) 
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement(window);
  var editor = msiGetEditor(editorElement);
  var retVal = fenceNode;
  var wrappedFenceNode = msiNavigationUtils.getWrappedObject(fenceNode, "fence");

  if (wrappedFenceNode == null)
  {
    dump("Problem in mathmlOverlay.js, reviseFence - fence node not found!\n");
    return retVal;
  }
  var theChildren = msiNavigationUtils.getSignificantContents(wrappedFenceNode);
  if (theChildren.length == 0)
  {
    dump("Problem in mathmlOverlay.js, reviseFence - supposed fence node has no children?\n");
    return retVal;
  }

  editor.beginTransaction();
  try 
  {
//    var mathmlEditor = editor.QueryInterface(Components.interfaces.msiIMathMLEditor);
//    if (!mathmlEditor)
//      logStr = "In insertfence, editor element [" + editorElement.id + "] has editor, but null mathmlEditor!\n";

    var firstNode = theChildren[0];
    var newFirst = msiSetMathTokenText(firstNode, left, editor);

    var lastNode = theChildren[theChildren.length - 1];
    var newLast = msiSetMathTokenText(lastNode, right, editor);
    editorElement.contentWindow.focus();
  } 
  catch (e) { dump("Error in mathmlOverlay.js, reviseFence! [" + e + "].\n"); }
  editor.endTransaction();

  return retVal;
}

function insertmatrix(rows, cols, rowsignature, editorElement) 
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement(window);
  var editor = msiGetEditor(editorElement);
  try 
  {
    var mathmlEditor = editor.QueryInterface(Components.interfaces.msiIMathMLEditor);
    mathmlEditor.InsertMatrix(rows, cols, rowsignature);
    editorElement.contentWindow.focus();
  } 
  catch (e) 
  {
  }
}

function insertmath(editorElement) 
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement(window);
  var editor = msiGetEditor(editorElement);
  var d = editor.document.createDocumentFragment();
  var b = newbox(editor);
  d.appendChild(b);
  insertfragment(editor, d);
  caret_on_box(editor, b);
  editorElement.contentWindow.focus();
}

function newbox(editor) 
{
  var box = editor.document.createElementNS(mmlns,"mi");
////  box.appendChild(editor.document.createTextNode("\u25A1"));
  box.appendChild(editor.document.createTextNode("\u200B"));
  box.setAttribute("tempinput","true");
  return box;
}

function caret_on_box(editor, rb) 
{
  var c = rb.firstChild;
  while (c && c.nodeType != Node.TEXT_NODE)
    c = c.firstChild;
  editor.selection.collapse(c,1);
}


function insertfragment(editor, node) 
{
  if (editor.selection) 
  {
    var end = editor.selection.focusNode;
    var off = editor.selection.focusOffset;
    var sel_length = Math.abs(off-editor.selection.anchorOffset);
    if (editor.selection.anchorNode == end && inputboxselected(end)) 
    {
      replacebox(end,node);
    } 
    else if (end.nodeType == Node.TEXT_NODE && off > 0) 
    {
      if (off == end.length) 
      {
        appendhere(editor, end, node);
      } 
      else 
      {
        insertbefore(editor, end.splitText(off), node);
      }
    } 
    else if (end.localName == "mrow") 
    {
      var cl = end.childNodes;
      if (off < cl.length) 
      {
        insertbefore(editor, end.childNodes[off],node);
      } 
      else 
      {
        end.appendChild(node)
      }
    } 
    else 
    {
      insertbefore(editor, end, node);
    } 
  }
}

function replacebox(loc,node) 
{
  var parent = loc.parentNode;
  var m = findmathparent(parent);
  if (parent.localName == "mi") 
  {
    parent.parentNode.replaceChild(node,parent);
  } 
  else 
  {
    dump("\nmathmled: impossible replacebox!");
  }
}

function appendhere(editor, loc, node) {
  var n = findmathparent(loc);
  if (!n) {
    var d = nestinmath(editor, node);
    insertafter(loc,d);
    return;
  } else {
    var p = loc.parentNode;
    if (cant_nest_here(p)) {
      loc = p;
      if (isaccent(loc)) {
        loc = loc.parentNode;
    } }
    insertafter(loc,node);
} }

function nestinmath(editor, node) 
{
  var d = editor.document.createDocumentFragment();
  var m = editor.document.createElementNS(mmlns,"math");
  m.setAttribute("xmlns",mmlns);
  var r = editor.document.createElementNS(mmlns,"mrow");
  r.appendChild(node); 
  m.appendChild(r); 
  d.appendChild(m);
  return d;
}

function insertbefore(editor, loc, node) {
  var n = findmathparent(loc);
  var p = loc.parentNode;
  if (!n) {
    p.insertBefore(nestinmath(editor, node),loc);
  } else {
    if (cant_nest_here(p)) {
      loc = p;
      p = loc.parentNode;
    }
    p.insertBefore(node,loc);
} }

function insertafter(loc,node) {
  var sib = loc.nextSibling;
  if (sib)
    loc.parentNode.insertBefore(node,sib);
  else
    loc.parentNode.appendChild(node);
}

function inputboxselected(node) 
{
  if (node.nodeType == Node.TEXT_NODE
   && node.data == "\u25A1"
   && node.parentNode.localName == "mi")
    return true;
  else if (node.localName == "mi")
    return inputboxselected(node.firstChild);
  else
    return false;
} 



function postProcessMathML(frag)
{
  frag.normalize();
  var j;
  var mathnodes=[];
  for (j=0; j<frag.childNodes.length; j++)
  {
    if (frag.childNodes[j].localName!="math")
      mathnodes = frag.childNodes[j].getElementsByTagName("mml:math");
    var i;
    var mnode;
    var textNode;
    var savedNode;
    var isFirst;
    var text;
    for (i=0; i<Math.max(1,mathnodes.length); i++)
    {
      if (mathnodes.length > 0) mnode = mathnodes[i];
      else mnode = frag.childNodes[j];
      // find all textnodes that are not whitespace only
      // if it is not in mi, mn, mo, or mtext, 
      // then if it is the first or last real text, pull it out of <math>
      // otherwise wrap it in an mtext.
      var tw = mnode.ownerDocument.createTreeWalker(mnode,4, //show text nodes
        null, true);
      var n;
      tw.nextNode();
      isFirst = true;
      n = tw.currentNode;
      while (n)
      {
        text = n.textContent;
        if (/\S/.test(text))
        {
          //non-whitespace
          parentName = n.parentNode.localName;
          if (/^mi$|^mo$|^mn$|^mtext$/.test(parentName)) break;
          textNode = frag.ownerDocument.createElement("mtext");
          textNode = n.parentNode.insertBefore(textNode, n);
          textNode.appendChild(n.cloneNode(false));
          savedNode = n;
          n = tw.nextNode();
          savedNode.parentNode.removeChild(savedNode);
          isFirst = false;
        }
      }
    }
  }
}


function mathNodeToText(editor, node)
{
  var frag = gProcessor.transformToFragment(node,editor.document);   
  postProcessMathML(frag);
  editor.replaceNode(frag,node,node.parentNode);
}


var gProcessor;
function mathToText(editor)
{
  var i;
  var j;
  var range;
  var nodeArray;
  var docfrag;
  var children;
  var enumerator;
  var node;
  if (editor.selection.collapsed)
  {
    dump("here we check to see if we can get out of math mode\n");
    return;
  }
  editor.beginTransaction();
  try
  {
    if (!gProcessor) gProcessor = new XSLTProcessor();
    else gProcessor.reset();
    var req = new XMLHttpRequest();

    req.open("GET", "chrome://prince/content/math2text.xsl", false); 
    req.send(null);
    // print the name of the root element or error message
    var xsldom = req.responseXML;
    gProcessor.importStylesheet(xsldom);
  
    for (i=0; i< editor.selection.rangeCount; i++)
    {
      //BBM: we have to work to make this undoable
      range = editor.selection.getRangeAt(i);
      nodeArray = editor.nodesInRange(range);
      dump(nodeArray.length+" nodes\n");
      enumerator = nodeArray.enumerate();
      while (enumerator.hasMoreElements())
      {
        node = enumerator.getNext();
        mathNodeToText(editor,node);
      }
    }
  }
  catch(e) {
    dump("error in MathNodeToText: "+e.message+"\n");
  }
  editor.endTransaction();
}


function toggleMathText(editor)
{
  try {
    mathToText(editor);
  }
  catch(e) {
    dump("Exception in toggleMathText: "+e.message+"\n");
  }
}