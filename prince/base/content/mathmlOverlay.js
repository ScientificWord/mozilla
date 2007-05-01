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
    doMathnameDlg(editorElement);
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
  catch (e) {}
}
function insertmathunit(name, editorElement) 
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement(window);
  var editor = msiGetEditor(editorElement);
  try 
  {
    var mathmlEditor = editor.QueryInterface(Components.interfaces.msiIMathMLEditor);
    mathmlEditor.InsertMathunit(name);
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

function doMathnameDlg(editorElement)
{
  var o = new Object();
  o.val = "";
  window.openDialog("chrome://prince/content/MathmlMathname.xul", "_blank", "chrome,close,titlebar,modal", o);
  if (o.val.length > 0) {
    if (o.enginefunction)
      insertenginefunction(o.val, editorElement);
    else
      insertmathname(o.val, editorElement);
  }
}

function doMatrixDlg(editorElement)
{
  var o = new Object();
  o.rows = 0;
  o.cols = 0;
  o.rowsignature = "";
  window.openDialog("chrome://prince/content/MathmlMatrix.xul", "_blank", "chrome,close,titlebar,modal", o);
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
      return ["para","p","mathp"];
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
  box.appendChild(editor.document.createTextNode("\u25A1"));
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
