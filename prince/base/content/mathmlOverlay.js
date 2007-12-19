// Copyright (c) 2004 MacKichan Software, Inc.  All Rights Reserved.

function SetupMSIMathMenuCommands()
{
  var commandTable = GetComposerCommandTable();
  
  //dump("Registering msi math menu commands\n");
  commandTable.registerCommand("cmd_MSIinlineMathCmd",  msiInlineMath);
  commandTable.registerCommand("cmd_MSIdisplayMathCmd", msiDisplayMath);
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
  commandTable.registerCommand("cmd_MSIreviseMatrixCmd",       msiReviseMatrixCmd);
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
  commandTable.registerCommand("cmd_MSIreviseMatrixCmd",       msiReviseMatrixCmd);
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
    var theFrac = aParams.getISupportsValue("reviseObject");
    if ( (editorElement != null) && (theFrac != null) )
    {
      var fractionData = new Object();
      fractionData.reviseObject = theFrac;
      var argArray = [fractionData];
      msiOpenModelessPropertiesDialog("chrome://prince/content/fractionProperties.xul", "_blank", "chrome,close,titlebar,dependent",
                                        editorElement, "cmd_MSIreviseFractionCmd", theFrac, argArray);
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
    var theRadical = aParams.getISupportsValue("reviseObject");
    if ( (editorElement != null) && (theRadical != null) )
    {
      var radicalData = new Object();
      radicalData.reviseObject = theRadical;
      var argArray = [radicalData];
      msiOpenModelessPropertiesDialog("chrome://prince/content/radicalProperties.xul", "_blank", "chrome,close,titlebar,dependent",
                                        editorElement, "cmd_MSIreviseRadicalCmd", theRadical, argArray);
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
    var theMathname = aParams.getISupportsValue("reviseObject");
    AlertWithTitle("mathmlOverlay.js", "In msiReviseMathnameCmd, trying to revise mathname, dialog unimplemented.");
//    reviseFraction(editorElement, theFrac);
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

var msiReviseMatrixCmd =
{
  isCommandEnabled: function(aCommand, dummy)
  { return true; },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon)
  {
    var editorElement = msiGetActiveEditorElement(window);
    var theMatrix = aParams.getISupportsValue("reviseObject");
    AlertWithTitle("mathmlOverlay.js", "In msiReviseMatrixCmd, trying to revise matrix, dialog unimplemented.");
//    reviseFraction(editorElement, theFrac);
  },

  doCommand: function(aCommand)
  {
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
    var theBrackets = aParams.getISupportsValue("reviseObject");
    AlertWithTitle("mathmlOverlay.js", "In msiReviseGenBracketsCmd, trying to revise brackets, dialog unimplemented.");
//    reviseFraction(editorElement, theFrac);
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
    var theBinomial = aParams.getISupportsValue("reviseObject");
    AlertWithTitle("mathmlOverlay.js", "In msiReviseBinomialsCmd, trying to revise binomial, dialog unimplemented.");
//    reviseFraction(editorElement, theFrac);
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
    var theOperator = aParams.getISupportsValue("reviseObject");
    AlertWithTitle("mathmlOverlay.js", "In msiReviseOperatorsCmd, trying to revise operator, dialog unimplemented.");
//    reviseFraction(editorElement, theFrac);
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
    doDecorationsDlg(String.fromCharCode(0x00AF), "", "", editorElement);
  }
};

var msiReviseDecorationsCmd =
{
  isCommandEnabled: function(aCommand, dummy)
  { return true; },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon)
  {
    var editorElement = msiGetActiveEditorElement(window);
    var theDecoration = aParams.getISupportsValue("reviseObject");
    AlertWithTitle("mathmlOverlay.js", "In msiReviseDecorationsCmd, trying to revise decoration, dialog unimplemented.");
//    reviseFraction(editorElement, theFrac);
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
    dump("Reached the msiUnitsDialog command handler.\n");
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
    var theUnit = aParams.getISupportsValue("reviseObject");
    AlertWithTitle("mathmlOverlay.js", "In msiReviseUnitsCmd, trying to revise unit, dialog unimplemented.");
//    reviseFraction(editorElement, theFrac);
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
  try
  {
//    var mathmlEditor = editor.QueryInterface(Components.interfaces.msiIMathMLEditor);
    var realFrac = msiNavigationUtils.getWrappedObject(theFraction, "mfrac");
    if (realFrac != null)
      realFrac.setAttribute("linethickness", lineSpec);
    else
    {
      AlertWithTitle("mathmlOverlay.js", "Problem in reviseFraction! No fraction found in node passed in...\n");
      return theFraction;
    }
    var styleObj = new Object();
    if (sizeSpec == "small")
      styleObj["displaystyle"] = "false";
    else if (sizeSpec == "big")
      styleObj["displaystyle"] = "true";
    else
      styleObj["displaystyle"] = "";
    retVal = applyMathStyleToObject(styleObj, "mfrac", theFraction);

//    AlertWithTitle("mathmlOverlay.js", "In reviseFraction, functionality needs to be implemented.");
//    mathmlEditor.InsertFraction(lineSpec, sizeFlags);
    editorElement.contentWindow.focus();
  }
  catch(e)
  {
    dump("Exception in mathmlOverlay.js, in reviseFraction; exception is [" + e + "].\n");
  }
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

function insertDecoration(decorationAboveStr, decorationBelowStr, decorationAroundStr, editorElement)
{
//  alert("Inserting Decoration above: [" + decorationAboveStr + "], below: [" + decorationBelowStr + "], around: [" + decorationAroundStr + "].");
  if (!editorElement)
    editorElement = msiGetActiveEditorElement(window);
  var editor = msiGetEditor(editorElement);
  try 
  {
    var mathmlEditor = editor.QueryInterface(Components.interfaces.msiIMathMLEditor);
    mathmlEditor.InsertDecoration(decorationAboveStr, decorationBelowStr);
    editorElement.contentWindow.focus();
  } 
  catch (e) 
  {
  }
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
  try
  {
//    var mathmlEditor = editor.QueryInterface(Components.interfaces.msiIMathMLEditor);

    var currRoot = msiNavigationUtils.getWrappedObject(theRadical, objectName);
    if (currRoot != null)
      return theRadical;  //it was already of the desired kind - we're done.

    var oldName = "mroot";
    if (objectName == "mroot")
      oldName = "msqrt";
    currRoot = msiNavigationUtils.getWrappedObject(theRadical, oldName);
    if (currRoot != null)
    {
      var newRoot = theRadical.ownerDocument.createElementNS(mmlns, objectName);
      var nextSibling = currRoot.nextSibling;
      if (nextSibling == null)
        newRoot = currRoot.parentNode.appendChild(newRoot);
      else
        newRoot = currRoot.parentNode.insertBefore(newRoot, nextSibling);
      var children = msiNavigationUtils.getSignificantContents(currRoot);
      if (objectName == "mroot")  //Changing a square root to a general root.
      {
        if (children.length == 1)
          newRoot.appendChild(children[0]);
        else
        {
          var newRow = theRadical.ownerDocument.createElementNS(mmlns, "mrow");
          for (var ix = 0; ix < children.length; ++ix)
            newRow.appendChild( theRadical.removeChild(children[ix]) );
          newRoot.appendChild(newRow);
        }
        newRoot.appendChild(newbox(editor));  //create an empty "root" box
      }
      else  //creating a msqrt - any "inferred row" of children is okay
      {
        if ( msiNavigationUtils.isOrdinaryMRow(children[0]) )
        {
          var newChildren = msiNavigationUtils.getSignificantContent(children[0]);
          for (var jx = 0; jx < newChildren.length; ++jx)
            newRoot.appendChild( children[0].removeChild(newChildren[jx]) );
        }
        else
          newRoot.appendChild( currRoot.removeChild(children[0]) );
      }
      if (currRoot == theRadical)  //radical was top level object being modified; new one should be the return value
        retVal = newRoot;          //(otherwise original object should still be returned)
      currRoot.parentNode.removeChild(currRoot);
    }
    else
    {
      AlertWithTitle("mathmlOverlay.js", "Problem in reviseFraction! No fraction found in node passed in...\n");
      return theRadical;
    }

//    AlertWithTitle("mathmlOverlay.js", "In reviseFraction, functionality needs to be implemented.");
//    mathmlEditor.InsertFraction(lineSpec, sizeFlags);
    editorElement.contentWindow.focus();
  }
  catch(e)
  {
    dump("Exception in mathmlOverlay.js, in reviseRadical; exception is [" + e + "].\n");
  }
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
function insertmathunit(name, editorElement) 
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement(window);
  var editor = msiGetEditor(editorElement);
  var nameData = msiBaseMathUnitsList.getUnitNameData(name);

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
//  window.openDialog("chrome://prince/content/MathmlMathname.xul", "_blank", "chrome,close,titlebar,modal", o);
  msiOpenModelessDialog("chrome://prince/content/MathmlMathname.xul", "_blank", "chrome,close,titlebar,dependent",
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
      for (var ix = 0; ix < insertNodes.length; ++ix)
      {
        var newNode = insertNodes[ix].cloneNode(true);
        editor.insertElementAtSelection(newNode, (ix==0));  //"true" means delete selection, so want to do it on first insertion
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
      insertmathname(mathNameObj.val, editorElement);
  }
}

function doInsertMathName(aName, editorElement)
{
  var mathNameObj = msiBaseMathNameList.getMathNameData(aName);
  if (mathNameObj == null)
  {
    mathNameObj = new Object();
    mathNameObj.type = "function";
    mathNameObj.val = aName;
    mathNameObj.enginefunction = false;
  }
  insertMathnameObject(mathNameObj, editorElement);
}

function doMatrixDlg(editorElement)
{
  var o = new Object();
  o.rows = 0;
  o.cols = 0;
  o.rowsignature = "";
  window.openDialog("chrome://prince/content/mathmlMatrix.xul", "_blank", "chrome,close,titlebar,modal", o);
  if (o.rows > 0 && o.cols > 0)
    insertmatrix(o.rows, o.cols, o.rowsignature, editorElement);
}

// maintain color attributes and generate corresponding CSS
var msiColorObj = 
{
  Format: function()
  {
    var res = "data:text/css,";
    if (this.mathColor.length > 0)
      res += "math { color: "+this.mathColor+"; } ";
    if (this.mathnameColor.length > 0)
      res += "mi[msiMathname=\"true\"] { color: "+this.mathnameColor+"; } ";
    if (this.unitColor.length > 0)
      res += "mi[class=\"msi_unit\"], mstyle[class=\"msi_unit\"] { color: "+this.unitColor+"; font-style: normal; } ";
    if (this.mtextColor.length > 0)
      res += "mtext { color: "+this.mtextColor+"; } ";
    if (this.matrixColor.length > 0) {
      res += "mi[tempinput=\"true\"] { color: "+this.matrixColor+"; } ";
      res += "mtable[border=\"0\"], mtable[border=\"0\"] > mtr > mtd, mtable:not([border]), mtable:not([border]) > mtr > mtd { border: 1px solid "+this.matrixColor+"; } ";
    }
    res += "mi[msiclass=\"enginefunction\"] { text-decoration: underline; } ";
    res += "p[class=\"msi_passthrough\"] { padding-left: 2em; } ";

    return res;
  },


//Following doesn't belong here at all. Move this functionality.
//Additionally, implementation of hiding "helper lines" should be via putting a "not([hideHelperLines])" at the outside?
//Probably not - just making an overriding rule.
  FormatStyleSheet : function(editorElement)
  {
    if (!editorElement)
      editorElement = msiGetActiveEditorElement();
    var editor = msiGetEditor(editorElement);
    var theTagManager = editor ? editor.tagListManager : null;

    function getParaTagList(tagManager)
    {
      return ["para","p","mathp", "*|dialogbase"];
    }
    function getMarkerTagList(tagManager)
    {
      return ["marker"];  //what should this be?
    }
    function getIndexTagList(tagManager)
    {
      return ["indexEntry"];  //what should this be?
    }
    function getHelperLineSelectors(tagManager)
    {
      return ["mtable[border=\"0\"]", "mtable[border=\"0\"] > mtr > mtd", "mtable:not([border])", "mtable:not([border]) > mtr > mtd"];
    }
    function getInputBoxSelectors(tagManager)
    {
      return ["mi[tempinput=\"true\"]"];
    }

    var res = "data:text/css,";
    if (this.mathColor.length > 0)
      res += "math { color: "+this.mathColor+"; } ";
    if (this.mathnameColor.length > 0)
      res += "mi[msiMathname=\"true\"] { color: "+this.mathnameColor+"; } ";
    if (this.unitColor.length > 0)
      res += "mi[class=\"msi_unit\"], mstyle[class=\"msi_unit\"] { color: "+this.unitColor+"; font-style: normal; } ";
    if (this.mtextColor.length > 0)
      res += "mtext { color: "+this.mtextColor+"; } ";
    if (this.matrixColor.length > 0) {
      res += "mi[tempinput=\"true\"] { color: "+this.matrixColor+"; } ";
      res += "mtable[border=\"0\"], mtable[border=\"0\"] > mtr > mtd, mtable:not([border]), mtable:not([border]) > mtr > mtd { border: 1px solid "+this.matrixColor+"; } ";
    }
    res += "mi[msiclass=\"enginefunction\"] { text-decoration: underline; } ";
    res += "p[class=\"msi_passthrough\"] { padding-left: 2em; } ";

    var invisColorStr = "green";
    var paraSelectors = getParaTagList(theTagManager);
    for (var ix = 0; ix < paraSelectors.length; ++ix)
    {
      if (ix > 0)
        res += ", ";
      res += "[showinvis=\"true\"] " + paraSelectors[ix] + ":after"
    }
    res +=  " {content: \"\\B6\"; display: inline; font-family: Courier New; color: " + invisColorStr + "; font-weight: bold;} ";

    var markerTags = getMarkerTagList(theTagManager);
    for (var ix = 0; ix < markerTags.length; ++ix)
    {
      if (ix > 0)
        res += ", ";
      res += "[hideMarkers=\"true\"] " + markerTags[ix];
    }
    res += " {display: none;} ";

    var indexEntryTags = getIndexTagList(theTagManager);
    for (var ix = 0; ix < indexEntryTags.length; ++ix)
    {
      if (ix > 0)
        res += ", "
      res += "[hideIndexEntries=\"true\"] " + indexEntryTags[ix];
    }
    res += " {display: none;} ";

    var helperLineSelectors = getHelperLineSelectors(theTagManager);
    for (var ix = 0; ix < helperLineSelectors.length; ++ix)
    {
      if (ix > 0)
        res += ", ";
      res += "[hideHelperLines=\"true\"] " + helperLineSelectors[ix];
    }
    res += " {border: 0px;} ";

    var inputBoxTags = getInputBoxSelectors(theTagManager);
    for (var ix = 0; ix < inputBoxTags.length; ++ix)
    {
      if (ix > 0)
        res += ", ";
      res += "[hideInputBoxes=\"true\"] " + inputBoxTags[ix];
    }
    res += " {visibility: hidden}";

    return res;

  },


  Init: function()
  {
    dump("\Init() not implemented!\n");
  },
  Persist: function()
  {
    dump("\nPersist() not implemented!\n");
  },

  mathColor: "#FF0000",
  mathnameColor: "#888888",
  unitColor:   "#009900",
  mtextColor:  "#000000",
  matrixColor: "#00CC00"
};

// Run color dialog.
//XXX This shouldn't happen in Source View or Preview mode.
function doColorsDlg(editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);

  window.openDialog("chrome://prince/content/MathColors.xul", "_blank", "chrome,close,titlebar,modal", msiColorObj);
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
//  window.openDialog("chrome://prince/content/Brackets.xul", "_blank", "chrome,close,titlebar,modal", bracketData);
  msiOpenModelessDialog("chrome://prince/content/Brackets.xul", "_blank", "chrome,close,titlebar,dependent",
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
//  window.openDialog("chrome://prince/content/Binomial.xul", "_blank", "chrome,close,titlebar,modal", binomialData);
  msiOpenModelessDialog("chrome://prince/content/Binomial.xul", "_blank", "chrome,close,titlebar,dependent",
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
//  window.openDialog("chrome://prince/content/Operators.xul", "_blank", "chrome,close,titlebar,modal", operatorData);
//  if (operatorData.Cancel)
//    return;
//  insertOperator(operatorData.operator, operatorData.limitsSpec, operatorData.sizeSpec, editorElement);
//  alert("Insert operator [" + operatorData.operator + "] with limit placement [" + operatorData.limitsSpec + "] and size [" + operatorData.sizeSpec + "].");
}

function doDecorationsDlg(decorationAboveStr, decorationBelowStr, decorationAroundStr, editorElement)
{
  var decorationData = new Object();
  decorationData.decorationAboveStr = decorationAboveStr;
  decorationData.decorationBelowStr = decorationBelowStr;
  decorationData.decorationAroundStr = decorationAroundStr;
  window.openDialog("chrome://prince/content/Decorations.xul", "_blank", "chrome,close,titlebar,modal", decorationData);
  if (decorationData.Cancel)
    return;
  insertDecoration(decorationData.decorationAboveStr, decorationData.decorationBelowStr, decorationData.decorationAroundStr, editorElement);
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
function applyMathStyleToObject(styleVals, objType, targ)
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

  function findStyleUntilObj(targNode, objTypeStr, expectedStyle, foundStyle)
  {
    var retStyleNode = null;
    var nodeName = msiGetBaseNodeName(targNode);
    if (nodeName == "mstyle")
    {
      retStyleNode = targNode;
      for each (var styleItem in expectedStyle)
      {
        if (targNode.hasAttribute(styleItem))
          foundStyle[styleItem] = targNode.getAttribute(styleItem);
      }
    }
    var wrappedChild = msiNavigationUtils.getSingleWrappedChild(targNode);
    if (wrappedChild != null)
    {
      var otherStyle = findStyleUntilObj(wrappedChild, objTypeStr, expectedStyle, foundStyle);
      if (otherStyle != null)
        retStyleNode = otherStyle;
    }
    //Following seems to be unnecessary!
//    switch(objTypeStr)
//    {
//      case "fence":
//        if (nodeName == "mrow" && msiNavigationUtils.isFence(targNode))
//          return retStyleNode;
//      break;
//      case "binomial":
//        if (nodeName == "mrow" && msiNavigationUtils.isBinomial(targNode))
//          return retStyleNode;
//      break;
//      default:
//        if (nodeName == objTypeStr)
//          return retStyleNode;
//      break; 
//    }
    return retStyleNode;
  }

  styleNode = findStyleUntilObj(targ, objType, styleVals, foundAttrs);
  
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
      if (styleVals[attrib].length > 0)
        styleNode.setAttribute(attrib, styleVals[attrib]);
      else if (styleNode.hasAttribute(attrib))
        styleNode.removeAttribute(attrib);
    }
    delete styleVals[attrib];
  }

  if (styleNode.attributes.length == 0)  //no more attributes - get rid of the mstyle!
  {
    if (!bAddStyleNode)
    {
      var kid = msiNavigationUtils.getSingleWrappedChild(styleNode);
      if (kid != null)
      {
        var nextSib = styleNode.nextSibling;
        if (nextSib != null)
        {
          //NOTE! Here we set "targ" so it can be used as the return value - it's the new revisable object
          targ = styleNode.parentNode.insertBefore(styleNode.removeChild(kid), nextSib);
          styleNode.parentNode.removeChild(styleNode);
        }
        else
        {
          //NOTE! Here we set "targ" so it can be used as the return value - it's the new revisable object
          targ = styleNode.parentNode.appendChild(styleNode.removeChild(kid));
          styleNode.parentNode.removeChild(styleNode);
        }
      }
    }
  }
  else if (bAddStyleNode)
  {
    var nextSib = targ.nextSibling;
    var theParent = targ.parentNode;
    var newTarg = theParent.removeChild(targ);
    styleNode.appendChild(newTarg);
    if (nextSib != null)
    {
      theParent.insertBefore(styleNode, nextSib);
    }
    else
    {
      theParent.appendChild(styleNode);
    }
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
//  box.appendChild(editor.document.createTextNode("\u25A1"));
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
