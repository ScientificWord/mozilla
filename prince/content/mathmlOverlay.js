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

function goUpdateMSIMathMenuItems(commandset)
{
  return;  //ljh todo -- do something reasonable
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
    insertfraction("", "");
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
    insertroot();
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
    insertradical();
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
    insertsup();
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
    insertsub();
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
    insertinlinemath();
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
    insertdisplay();
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
    insertmathname("sin");
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
    insertmathname("cos");
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
    insertmathname("tan");
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
    insertmathname("ln");
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
    insertmathname("log");
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
    insertmathname("exp");
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
    doMathnameDlg();
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
    insertfence("(",")");
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
    insertfence("[","]");
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
    insertfence("{","}");
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
    insertfence("|","|");
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
    insertfence("\u2016","\u2016");
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
    doMatrixDlg();
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
    doColorsDlg();
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
    doBracketsDlg("(",")","");
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
    doBinomialsDlg("(",")","0","auto");
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
    doOperatorsDlg(String.fromCharCode(0x222B), "auto", "auto");
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
    doDecorationsDlg(String.fromCharCode(0x00AF), "", "");
  }
};

const mmlns    = "http://www.w3.org/1998/Math/MathML";
const xhtmlns  = "http://www.w3.org/1999/xhtml";

function insertinlinemath() 
{
  var editor = GetCurrentEditor();
  try 
  {
    var mathmlEditor = editor.QueryInterface(Components.interfaces.msiIMathMLEditor);
    mathmlEditor.InsertInlineMath();
  } 
  catch (e) 
  {
  }
}

function insertdisplay() 
{
  var editor = GetCurrentEditor();
  try 
  {
    var mathmlEditor = editor.QueryInterface(Components.interfaces.msiIMathMLEditor);
    mathmlEditor.InsertDisplay();
  } 
  catch (e) 
  {
  }
}

function insertsup() 
{
  var editor = GetCurrentEditor();
  try 
  {
    var mathmlEditor = editor.QueryInterface(Components.interfaces.msiIMathMLEditor);
    mathmlEditor.InsertSuperscript();
  } 
  catch (e) 
  {
  }
}

function insertsub() 
{
  var editor = GetCurrentEditor();
  try 
  {
    var mathmlEditor = editor.QueryInterface(Components.interfaces.msiIMathMLEditor);
    mathmlEditor.InsertSubscript();
  } 
  catch (e) 
  {
  }
}

function insertfraction(lineSpec, sizeSpec) 
{
  var editor = GetCurrentEditor();
  try 
  {
    var mathmlEditor = editor.QueryInterface(Components.interfaces.msiIMathMLEditor);
    var sizeFlags = Components.interfaces.msiIMMLEditDefines.MO_ATTR_autoSize;
    if (sizeSpec == "small")
      sizeFlags = Components.interfaces.msiIMMLEditDefines.MO_ATTR_smallSize;
    else if (sizeSpec == "big")
      sizeFlags = Components.interfaces.msiIMMLEditDefines.MO_ATTR_displaySize;
    mathmlEditor.InsertFraction(lineSpec, sizeFlags);
  } 
  catch (e) 
  {
  }
}

function insertBinomial(openingBracket, closingBracket, lineSpec, sizeSpec)
{
  var editor = GetCurrentEditor();
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
  } 
  catch (e) 
  {
  }
}

function insertOperator(operator, limitPlacement, sizeSpec)
{
  var editor = GetCurrentEditor();
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
  } 
  catch (e) 
  {
  }
}

function insertDecoration(decorationAboveStr, decorationBelowStr, decorationAroundStr)
{
//  alert("Inserting Decoration above: [" + decorationAboveStr + "], below: [" + decorationBelowStr + "], around: [" + decorationAroundStr + "].");
  var editor = GetCurrentEditor();
  try 
  {
    var mathmlEditor = editor.QueryInterface(Components.interfaces.msiIMathMLEditor);
    mathmlEditor.InsertDecoration(decorationAboveStr, decorationBelowStr);
  } 
  catch (e) 
  {
  }
}

function insertroot() 
{
  var editor = GetCurrentEditor();
  try 
  {
    var mathmlEditor = editor.QueryInterface(Components.interfaces.msiIMathMLEditor);
    mathmlEditor.InsertRoot();
  } 
  catch (e) 
  {
  }
}

function insertradical() 
{
  var editor = GetCurrentEditor();
  try 
  {
    var mathmlEditor = editor.QueryInterface(Components.interfaces.msiIMathMLEditor);
    mathmlEditor.InsertSqRoot();
  } 
  catch (e) 
  {
  }
}


function insertmathname(name) 
{
  var editor = GetCurrentEditor();
  try 
  {
    var mathmlEditor = editor.QueryInterface(Components.interfaces.msiIMathMLEditor);
    mathmlEditor.InsertMathname(name);
  } 
  catch (e) {}
}

function insertenginefunction(name) 
{
  var editor = GetCurrentEditor();
  try 
  {
    var mathmlEditor = editor.QueryInterface(Components.interfaces.msiIMathMLEditor);
    mathmlEditor.InsertEngineFunction(name);
  } 
  catch (e) {}
}

function doMathnameDlg()
{
  var o = new Object();
  o.val = "";
  window.openDialog("chrome://editor/content/MathmlMathname.xul", "_blank", "chrome,close,titlebar,modal", o);
  if (o.val.length > 0) {
    if (o.enginefunction)
      insertenginefunction(o.val);
    else
      insertmathname(o.val);
  }
}

function doMatrixDlg()
{
  var o = new Object();
  o.rows = 0;
  o.cols = 0;
  o.rowsignature = "";
  window.openDialog("chrome://editor/content/MathmlMatrix.xul", "_blank", "chrome,close,titlebar,modal", o);
  if (o.rows > 0 && o.cols > 0)
    insertmatrix(o.rows, o.cols, o.rowsignature);
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
function doColorsDlg()
{
  window.openDialog("chrome://editor/content/MathColors.xul", "_blank", "chrome,close,titlebar,modal", msiColorObj);
  if (msiColorObj.Cancel)
    return;

  var editor = GetCurrentEditor();
  try {
    editor.QueryInterface(nsIEditorStyleSheets);

    editor.removeOverrideStyleSheet(gMathStyleSheet);
    gMathStyleSheet = msiColorObj.Format();
    editor.addOverrideStyleSheet(gMathStyleSheet);
  } catch (e) {
    dump("Something wrong with stylesheet mechanism\n");
  }
}


function doBracketsDlg(leftBrack, rightBrack, sep)
{
  var bracketData = new Object();
  // Eventually we want to initialize this with whatever is in the document, if possible. In other cases, it is
  // an error to initialize it since that defeats any persistence of the dialog state.
  bracketData.leftBracket = ""; //leftBrack;
  bracketData.rightBracket = ""; //rightBrack;
  bracketData.separator = sep;
  window.openDialog("chrome://editor/content/Brackets.xul", "_blank", "chrome,close,titlebar,modal", bracketData);
  if (bracketData.Cancel)
    return;
  insertfence(bracketData.leftBracket, bracketData.rightBracket, bracketData.separator);
}


function doBinomialsDlg(leftBrack, rightBrack, line, size)
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
  window.openDialog("chrome://editor/content/Binomial.xul", "_blank", "chrome,close,titlebar,modal", binomialData);
  if (binomialData.Cancel)
    return;
  if (!binomialData.withDelimiters)
  {
    binomialData.leftBracket = "";
    binomialData.rightBracket = "";
  }
  insertBinomial(binomialData.leftBracket, binomialData.rightBracket,
                        binomialData.lineSpec, binomialData.sizeSpec);
//  if (binomialData.withDelimiters)
//    insertfence(binomialData.leftBracket, binomialData.rightBracket, "");
//  insertfraction(binomialData.lineSpec, binomialData.sizeSpec);
}

function doOperatorsDlg(operatorStr, limitPlacement, size)
{
  var operatorData = new Object();
  operatorData.operator = operatorStr;
  operatorData.limitsSpec = limitPlacement;
  operatorData.sizeSpec = size;
  window.openDialog("chrome://editor/content/Operators.xul", "_blank", "chrome,close,titlebar,modal", operatorData);
  if (operatorData.Cancel)
    return;
  insertOperator(operatorData.operator, operatorData.limitsSpec, operatorData.sizeSpec);
//  alert("Insert operator [" + operatorData.operator + "] with limit placement [" + operatorData.limitsSpec + "] and size [" + operatorData.sizeSpec + "].");
}

function doDecorationsDlg(decorationAboveStr, decorationBelowStr, decorationAroundStr)
{
  var decorationData = new Object();
  decorationData.decorationAboveStr = decorationAboveStr;
  decorationData.decorationBelowStr = decorationBelowStr;
  decorationData.decorationAroundStr = decorationAroundStr;
  window.openDialog("chrome://editor/content/Decorations.xul", "_blank", "chrome,close,titlebar,modal", decorationData);
  if (decorationData.Cancel)
    return;
  insertDecoration(decorationData.decorationAboveStr, decorationData.decorationBelowStr, decorationData.decorationAroundStr);
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

function insertsymbol(s) 
{
  var editor = GetCurrentEditor();
  try 
  {
    var mathmlEditor = editor.QueryInterface(Components.interfaces.msiIMathMLEditor);
    mathmlEditor.InsertSymbol(s.charCodeAt(0));
  } 
  catch (e) 
  {
    dump("insertsymbol failed: "+e+"\n");
  }
}
function insertfence(left, right) 
{
  var editor = GetCurrentEditor();
  try 
  {
    var mathmlEditor = editor.QueryInterface(Components.interfaces.msiIMathMLEditor);
    mathmlEditor.InsertFence(left, right);
  } 
  catch (e) 
  {
  }
}

function insertmatrix(rows, cols, rowsignature) 
{
  var editor = GetCurrentEditor();
  try 
  {
    var mathmlEditor = editor.QueryInterface(Components.interfaces.msiIMathMLEditor);
    mathmlEditor.InsertMatrix(rows, cols, rowsignature);
  } 
  catch (e) 
  {
  }
}

function insertmath() 
{
  var editor = GetCurrentEditor();
  var d = editor.document.createDocumentFragment();
  var b = newbox(editor);
  d.appendChild(b);
  insertfragment(editor, d);
  caret_on_box(editor, b);
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
