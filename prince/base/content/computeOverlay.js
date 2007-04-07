// Copyright (c) 2004 MacKichan Software, Inc.  All Rights Reserved.

//-----------------------------------------------------------------------------------
var msiEvaluateCommand =
{
  isCommandEnabled: function(aCommand, editorElement)
  {
    var theEditorElement = msiGetActiveEditorElement();
    return (msiIsDocumentEditable(theEditorElement) && msiIsEditingRenderedHTML(theEditorElement));
  },

  getCommandStateParams: function(aCommand, aParams, editorElement) {},
  doCommandParams: function(aCommand, aParams, editorElement) {},

  doCommand: function(aCommand, editorElement)
  {
    var theEditorElement = msiGetActiveEditorElement();
    doComputeCommand(aCommand, theEditorElement, this);
  }
};

var msiDefineCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    return (msiIsDocumentEditable(editorElement) && msiIsEditingRenderedHTML(editorElement));
  },

  getCommandStateParams: function(aCommand, aParams, editorElement) {},
  doCommandParams: function(aCommand, aParams, editorElement) {},

  doCommand: function(aCommand, editorElement)
  {
    var theEditorElement = msiGetActiveEditorElement();
    doComputeCommand(aCommand, theEditorElement);
  }
};

/////////////////////////
var msiComputeLogger = 
{
  Sent: function(name,expr)
  {
    if (this.logMMLSent)
      dump("compsample " + name + ": ======================================\n" + expr + "\n");
  },
  Sent4: function(name,expr,arg1,arg2)
  {
    if (this.logMMLSent)
      dump("compsample " + name + ": ======================================\n" + expr + "\n" + arg1 + "\n" + arg2 + "\n");
  },
  LogEngineStrs: function()
  {
    if (this.logEngSent)
      dump("compsample sent:  ===> " + GetCurrentEngine().getEngineSent() + "\n");
    if (this.logEngReceived)
      dump("compsample rcvd: <===  " + GetCurrentEngine().getEngineReceived() + "\n");
  },
  Received: function(expr)
  {
    this.LogEngineStrs();
    if (this.logMMLReceived)
      dump("compsample result: ======================================\n" + expr + "\n");
  },
  Exception: function(e)
  {
    this.LogEngineStrs();
    if (this.logMMLReceived) {
      dump("compsample exception: !!!!!!!!!!!!\n");
    }
    dump(e);
    // separate pref for errors?
    dump("\ncompsample engine:  " + GetCurrentEngine());
    dump("\n           errors:  " + GetCurrentEngine().getEngineErrors() + "\n");
  },
  Init: function()
  {
    try {
      this.logMMLSent = gPrefs.getBoolPref("prince.MuPAD.log_mathml_sent");
    }
    catch(ex) {
      dump("\nfailed to get MuPAD.log_mathml_sent pref!\n");
    }
    try {
      this.logMMLReceived = gPrefs.getBoolPref("prince.MuPAD.log_mathml_received");
    }
    catch(ex) {
      dump("\nfailed to get MuPAD.log_mathml_received pref!\n");
    }
    try {
      this.engMMLSent = gPrefs.getBoolPref("prince.MuPAD.log_engine_sent");
    }
    catch(ex) {
      dump("\nfailed to get MuPAD.log_engine_sent pref!\n");
    }
    try {
      this.engMMLReceived = gPrefs.getBoolPref("prince.MuPAD.log_engine_received");
    }
    catch(ex) {
      dump("\nfailed to get MuPAD.log_engine_received pref!\n");
    }
  },
  LogMMLSent: function(log)
  {
    this.logMMLSent = log;
    gPrefs.setBoolPref("prince.MuPAD.log_mathml_sent",log);
  },
  LogMMLReceived: function(log)
  {
    this.logMMLReceived = log;
    gPrefs.setBoolPref("prince.MuPAD.log_mathml_received",log);
  },
  LogEngSent: function(log)
  {
    this.logEngSent = log;
    gPrefs.setBoolPref("prince.MuPAD.log_engine_sent",log);
  },
  LogEngReceived: function(log)
  {
    this.logEngReceived = log;
    gPrefs.setBoolPref("prince.MuPAD.log_engine_received",log);
  },

  logMMLSent:      true,
  logMMLReceived:  true,
  logEngSent:      true,
  logEngReceived:  true
};

function SetupMSIComputeMenuCommands()
{
  var commandTable = GetComposerCommandTable();
  //dump("Registering msi compute menu commands\n");
  // note hokey use of shared command
  doSetupMSIComputeMenuCommands(commandTable);
}

function msiSetupMSIComputeMenuCommands(editorElement)
{
  var commandTable = msiGetComposerCommandTable(editorElement);
  //dump("Registering msi compute menu commands\n");
  doSetupMSIComputeMenuCommands(commandTable);
}

function doSetupMSIComputeMenuCommands(commandTable)
{
  // note hokey use of shared command
  commandTable.registerCommand("cmd_MSIComputeEval",             msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeEvalNum",          msiEvaluateCommand);      
  commandTable.registerCommand("cmd_MSIComputeSimplify",         msiEvaluateCommand);     
  commandTable.registerCommand("cmd_MSIComputeCombineExponentials",      msiEvaluateCommand);       
  commandTable.registerCommand("cmd_MSIComputeCombineLogs",              msiEvaluateCommand);       
  commandTable.registerCommand("cmd_MSIComputeCombinePowers",            msiEvaluateCommand);       
  commandTable.registerCommand("cmd_MSIComputeCombineTrig",              msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeCombineArctan",            msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeCombineHyperbolics",       msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeExpand",                   msiEvaluateCommand);       
  commandTable.registerCommand("cmd_MSIComputeFactor",                   msiEvaluateCommand);       
  commandTable.registerCommand("cmd_MSIComputeRewriteRational",          msiEvaluateCommand);       
  commandTable.registerCommand("cmd_MSIComputeRewriteFloat",             msiEvaluateCommand);       
  commandTable.registerCommand("cmd_MSIComputeRewriteMixed",             msiEvaluateCommand);       
  commandTable.registerCommand("cmd_MSIComputeRewriteExponential",       msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeRewriteFactorial",         msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeRewriteGamma",             msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeRewriteLogarithm",         msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeRewriteSinAndCos",         msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeRewriteSinhAndCosh",       msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeRewriteSin",               msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeRewriteCos",               msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeRewriteTan",               msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeRewriteArcsin",            msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeRewriteArccos",            msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeRewriteArctan",            msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeRewriteArccot",            msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeRewritePolar",             msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeRewriteRectangular",       msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeRewriteNormal",            msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeRewriteEquationsAsMatrix", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeRewriteMatrixAsEquations", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeCheckEquality",    msiEvaluateCommand);       
  commandTable.registerCommand("cmd_MSIComputeSolveExact",       msiEvaluateCommand);   
  commandTable.registerCommand("cmd_MSIComputeSolveNum",         msiEvaluateCommand);     
  commandTable.registerCommand("cmd_MSIComputeSolveRecursion",   msiEvaluateCommand);     
  commandTable.registerCommand("cmd_MSIComputeCollect",          msiEvaluateCommand);   
  commandTable.registerCommand("cmd_MSIComputeDivide",           msiEvaluateCommand);     
  commandTable.registerCommand("cmd_MSIComputePartialFractions", msiEvaluateCommand);     
  commandTable.registerCommand("cmd_MSIComputeRoots",            msiEvaluateCommand);   
  commandTable.registerCommand("cmd_MSIComputeSort",             msiEvaluateCommand);     
  commandTable.registerCommand("cmd_MSIComputeCompanionMatrix",  msiEvaluateCommand);     
  commandTable.registerCommand("cmd_MSIComputeByParts",          msiEvaluateCommand);     
  commandTable.registerCommand("cmd_MSIComputeChangeVariable",   msiEvaluateCommand);     
  commandTable.registerCommand("cmd_MSIComputeApproxIntegral",   msiEvaluateCommand);     
//disable  commandTable.registerCommand("cmd_MSIComputePlotApproxIntegral", msiEvaluateCommand);     
  commandTable.registerCommand("cmd_MSIComputeIterate",          msiEvaluateCommand);     
  commandTable.registerCommand("cmd_MSIComputeImplicitDiff",     msiEvaluateCommand);     
  commandTable.registerCommand("cmd_MSIComputeSolveODEExact",    msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeSolveODELaplace",  msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeSolveODENumeric",  msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeSolveODESeries",   msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputePowerSeries",      msiEvaluateCommand);   
  commandTable.registerCommand("cmd_MSIComputeFourier",          msiEvaluateCommand);   
  commandTable.registerCommand("cmd_MSIComputeInverseFourier",   msiEvaluateCommand);   
  commandTable.registerCommand("cmd_MSIComputeLaplace",          msiEvaluateCommand);   
  commandTable.registerCommand("cmd_MSIComputeInverseLaplace",   msiEvaluateCommand);   
  commandTable.registerCommand("cmd_MSIComputeGradient",         msiEvaluateCommand);   
  commandTable.registerCommand("cmd_MSIComputeDivergence",       msiEvaluateCommand);     
  commandTable.registerCommand("cmd_MSIComputeCurl",             msiEvaluateCommand);   
  commandTable.registerCommand("cmd_MSIComputeLaplacian",        msiEvaluateCommand);     
  commandTable.registerCommand("cmd_MSIComputeJacobian",         msiEvaluateCommand);   
  commandTable.registerCommand("cmd_MSIComputeHessian",          msiEvaluateCommand);     
  commandTable.registerCommand("cmd_MSIComputeWronskian",        msiEvaluateCommand);     
  commandTable.registerCommand("cmd_MSIComputeScalarPot",        msiEvaluateCommand);   
  commandTable.registerCommand("cmd_MSIComputeVectorPot",        msiEvaluateCommand);     
  commandTable.registerCommand("cmd_MSIComputeAdjugate",     msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeCharPoly",     msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeCholesky",     msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeColBasis",     msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeConcat",     msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeConditionNum",     msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeDefinitenessTests",msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeDeterminant",      msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeEigenvalues",     msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeEigenvectors",     msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeFillMatrix",       msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeFFGE",     msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeGaussElim",     msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeHermite",     msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeHermitianTranspose", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeInverse",     msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeJordan",     msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeMap",     msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeMinPoly",     msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeNorm",     msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeNullspaceBasis",     msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeOrthogonalityTest",msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputePermanent",        msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputePLU",     msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeRandomMatrix",     msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeRank",     msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeRationalCanonical",msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeRREF",     msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeReshape",     msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeRowBasis",     msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeQR",     msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeSingularValues",   msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeSVD",     msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeSmith",     msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeSpectralRadius",   msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeStack",     msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeTrace",     msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeSimplexDual",        msiEvaluateCommand);   
  commandTable.registerCommand("cmd_MSIComputeSimplexFeasible",    msiEvaluateCommand);   
  commandTable.registerCommand("cmd_MSIComputeSimplexMaximize",    msiEvaluateCommand);   
  commandTable.registerCommand("cmd_MSIComputeSimplexMinimize",    msiEvaluateCommand);   
  commandTable.registerCommand("cmd_MSIComputeSimplexStandardize", msiEvaluateCommand);   
  commandTable.registerCommand("cmd_MSIComputeTranspose",     msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeFitCurve",         msiEvaluateCommand);   
  commandTable.registerCommand("cmd_MSIComputeRandomNumbers",    msiEvaluateCommand);   
  commandTable.registerCommand("cmd_MSIComputeMean",             msiEvaluateCommand);   
  commandTable.registerCommand("cmd_MSIComputeMedian",           msiEvaluateCommand);   
  commandTable.registerCommand("cmd_MSIComputeMode",             msiEvaluateCommand);   
  commandTable.registerCommand("cmd_MSIComputeCorrelation",      msiEvaluateCommand);   
  commandTable.registerCommand("cmd_MSIComputeCovariance",       msiEvaluateCommand);   
  commandTable.registerCommand("cmd_MSIComputeGeometricMean",    msiEvaluateCommand);   
  commandTable.registerCommand("cmd_MSIComputeHarmonicMean",     msiEvaluateCommand);   
  commandTable.registerCommand("cmd_MSIComputeMeanDeviation",    msiEvaluateCommand);   
  commandTable.registerCommand("cmd_MSIComputeMoment",           msiEvaluateCommand);   
  commandTable.registerCommand("cmd_MSIComputeQuantile",         msiEvaluateCommand);   
  commandTable.registerCommand("cmd_MSIComputeStandardDeviation",msiEvaluateCommand);   
  commandTable.registerCommand("cmd_MSIComputeVariance",         msiEvaluateCommand);   
  commandTable.registerCommand("cmd_MSIComputePlot2DRectangular",msiEvaluateCommand);         
  commandTable.registerCommand("cmd_MSIComputePlot2DPolar",      msiEvaluateCommand);         
  commandTable.registerCommand("cmd_MSIComputePlot2DImplicit",   msiEvaluateCommand);         
  commandTable.registerCommand("cmd_MSIComputePlot2DInequality", msiEvaluateCommand);         
  commandTable.registerCommand("cmd_MSIComputePlot2DParametric", msiEvaluateCommand);         
  commandTable.registerCommand("cmd_MSIComputePlot2DConformal",  msiEvaluateCommand);         
  commandTable.registerCommand("cmd_MSIComputePlot2DGradient",   msiEvaluateCommand);         
  commandTable.registerCommand("cmd_MSIComputePlot2DVector",     msiEvaluateCommand);         
  commandTable.registerCommand("cmd_MSIComputePlot2DODE",        msiEvaluateCommand);         
  commandTable.registerCommand("cmd_MSIComputePlot2DAI",         msiEvaluateCommand);         
  commandTable.registerCommand("cmd_MSIComputePlot3DRectangular",msiEvaluateCommand);         
  commandTable.registerCommand("cmd_MSIComputePlot3DCurve",      msiEvaluateCommand);         
  commandTable.registerCommand("cmd_MSIComputePlot3DCylindrical",msiEvaluateCommand);         
  commandTable.registerCommand("cmd_MSIComputePlot3DSpherical",  msiEvaluateCommand);         
  commandTable.registerCommand("cmd_MSIComputePlot3DParametric", msiEvaluateCommand);         
  commandTable.registerCommand("cmd_MSIComputePlot3DImplicit",   msiEvaluateCommand);         
  commandTable.registerCommand("cmd_MSIComputePlot3DTube",       msiEvaluateCommand);         
  commandTable.registerCommand("cmd_MSIComputePlot3DGradient",   msiEvaluateCommand);         
  commandTable.registerCommand("cmd_MSIComputePlot3DVector",     msiEvaluateCommand);         
  commandTable.registerCommand("cmd_MSIComputePlot2DARectangular",msiEvaluateCommand);         
  commandTable.registerCommand("cmd_MSIComputePlot2DAPolar",      msiEvaluateCommand);         
  commandTable.registerCommand("cmd_MSIComputePlot2DAImplicit",   msiEvaluateCommand);         
  commandTable.registerCommand("cmd_MSIComputePlot2DAInequality", msiEvaluateCommand);         
  commandTable.registerCommand("cmd_MSIComputePlot2DAParametric", msiEvaluateCommand);         
  commandTable.registerCommand("cmd_MSIComputePlot2DAConformal",  msiEvaluateCommand);         
  commandTable.registerCommand("cmd_MSIComputePlot2DAGradient",   msiEvaluateCommand);         
  commandTable.registerCommand("cmd_MSIComputePlot2DAVector",     msiEvaluateCommand);         
  commandTable.registerCommand("cmd_MSIComputePlot3DARectangular",msiEvaluateCommand);         
  commandTable.registerCommand("cmd_MSIComputePlot3DACurve",      msiEvaluateCommand);         
  commandTable.registerCommand("cmd_MSIComputePlot3DACylindrical",msiEvaluateCommand);         
  commandTable.registerCommand("cmd_MSIComputePlot3DASpherical",  msiEvaluateCommand);         
  commandTable.registerCommand("cmd_MSIComputePlot3DAParametric", msiEvaluateCommand);         
  commandTable.registerCommand("cmd_MSIComputePlot3DAImplicit",   msiEvaluateCommand);         
  commandTable.registerCommand("cmd_MSIComputePlot3DATube",       msiEvaluateCommand);         
  commandTable.registerCommand("cmd_MSIComputePlot3DAGradient",   msiEvaluateCommand);         
  commandTable.registerCommand("cmd_MSIComputePlot3DAVector",     msiEvaluateCommand);         
  commandTable.registerCommand("cmd_MSIComputeEditPlot",          msiEvaluateCommand);         
  commandTable.registerCommand("cmd_MSIComputeSetBasisVariables", msiDefineCommand);     
  commandTable.registerCommand("cmd_MSIComputeDefine",        msiDefineCommand);
  commandTable.registerCommand("cmd_MSIComputeUndefine",      msiDefineCommand);     
  commandTable.registerCommand("cmd_MSIComputeShowDefs",      msiDefineCommand);     
  commandTable.registerCommand("cmd_MSIComputeClearDefs",     msiDefineCommand);    
  commandTable.registerCommand("cmd_MSIComputeUserSettings",  msiDefineCommand);     
  commandTable.registerCommand("cmd_MSIComputeSettings",      msiDefineCommand);     
  commandTable.registerCommand("cmd_MSIComputeSwitchEngines", msiDefineCommand);     
  commandTable.registerCommand("cmd_MSIComputeInterpret",     msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeFixup",         msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputePassthru",      msiDefineCommand);
}

function goUpdateMSIcomputeMenuItems(commandset)
{
  for (var i = 0; i < commandset.childNodes.length; i++)
  {
    var commandNode = commandset.childNodes[i];
    var commandID = commandNode.id;
    if (commandID)
    {
      goUpdateCommand(commandID);
      if (commandNode.hasAttribute("state"))
        goUpdateCommandState(commandID);
} } }


function msiGoUpdateMSIcomputeMenuItems(commandset, editorElement)
{
  for (var i = 0; i < commandset.childNodes.length; i++)
  {
    var commandNode = commandset.childNodes[i];
    var commandID = commandNode.id;
    if (commandID)
    {
      msiGoUpdateCommand(commandID, editorElement);
      if (commandNode.hasAttribute("state"))
        msiGoUpdateCommandState(commandID, editorElement);
    } 
  } 
}


const htmlns   = "http://www.w3.org/1999/xhtml";
const fullmath = '<math xmlns="http://www.w3.org/1998/Math/MathML">';

var gComputeStringBundle;

function GetComputeString(name)
{
  if (!gComputeStringBundle)
  {
    try {
      var strBundleService =
          Components.classes["@mozilla.org/intl/stringbundle;1"].getService(); 
      strBundleService = 
          strBundleService.QueryInterface(Components.interfaces.nsIStringBundleService);

      gComputeStringBundle = strBundleService.createBundle("chrome://prince/locale/compute.properties"); 

    } catch (ex) {}
  }
  if (gComputeStringBundle)
  {
    try {
      return gComputeStringBundle.GetStringFromName(name);
    } catch (e) {}
  }
  return null;
}


// used by computeDebug.js.  Need to consider scripting interface in more detail.
function doComputeEvaluate(math, editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  doEvalComputation(math,GetCurrentEngine().Evaluate,"<mo>=</mo>","evaluate", editorElement);
}

function doComputeCommand(cmd, editorElement, cmdHandler)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var selection = msiGetEditor(editorElement).selection;
  if (selection)
  {
    var element = findmathparent(selection.focusNode);
    if (!element) {
	    dump("not in math!\n");
      return;
    }
    if (HasEmptyMath(element)) {
      dump("math has temp input\n");
      return;
    }
    var eng = GetCurrentEngine();
    switch (cmd) {
    case "cmd_compute_Evaluate":
      doEvalComputation(element,eng.Evaluate,"<mo>=</mo>","evaluate", editorElement);
      break;
    case "cmd_compute_EvaluateNumeric":
      doEvalComputation(element,eng.Evaluate_Numerically,"<mo>"+String.fromCharCode(0x2248)+"</mo>","evaluate numeric", editorElement);
      break;
    case "cmd_compute_Simplify":
      doEvalComputation(element,eng.Simplify,"<mo>=</mo>","simplify", editorElement);
      break;
    case "cmd_compute_CombineExponentials":
      doEvalComputation(element,eng.Combine_Exponentials,"<mo>=</mo>","combine exp", editorElement);
      break;
    case "cmd_compute_CombineLogs":
      doEvalComputation(element,eng.Combine_Logs,"<mo>=</mo>","combine log", editorElement);
      break;
    case "cmd_compute_CombinePowers":
      doEvalComputation(element,eng.Combine_Powers,"<mo>=</mo>","combine pow", editorElement);
      break;
    case "cmd_compute_CombineTrig":
      doEvalComputation(element,eng.Combine_Trig_Functions,"<mo>=</mo>","combine trig", editorElement);
      break;
    case "cmd_compute_CombineArctan":
      doEvalComputation(element,eng.Combine_Arctan,"<mo>=</mo>","combine arctan", editorElement);
      break;
    case "cmd_compute_CombineHyperbolics":
      doEvalComputation(element,eng.Combine_Hyperbolic_Functions,"<mo>=</mo>","combine hyperbolic", editorElement);
      break;
    case "cmd_compute_Expand":
      doEvalComputation(element,eng.Expand,"<mo>=</mo>","expand", editorElement);
      break;
    case "cmd_compute_Factor":
      doEvalComputation(element,eng.Factor,"<mo>=</mo>","factor", editorElement);
      break;
    case "cmd_compute_RewriteRational":
      doEvalComputation(element,eng.Rewrite_Rational,"<mo>=</mo>","rewrite rational", editorElement);
      break;
    case "cmd_compute_RewriteFloat":
      doEvalComputation(element,eng.Rewrite_Float,"<mo>=</mo>","rewrite float", editorElement);
      break;
    case "cmd_compute_RewriteMixed":
      doEvalComputation(element,eng.Rewrite_Mixed,"<mo>=</mo>","rewrite mixed", editorElement);
      break;
    case "cmd_compute_RewriteExponential":
      doEvalComputation(element,eng.Rewrite_Exponential,"<mo>=</mo>","rewrite exponential", editorElement);
      break;
    case "cmd_compute_RewriteFactorial":
      doEvalComputation(element,eng.Rewrite_Factorial,"<mo>=</mo>","rewrite factorial", editorElement);
      break;
    case "cmd_compute_RewriteGamma":
      doEvalComputation(element,eng.Rewrite_Gamma,"<mo>=</mo>","rewrite gamma", editorElement);
      break;
    case "cmd_compute_RewriteLogarithm":
      doEvalComputation(element,eng.Rewrite_Logarithm,"<mo>=</mo>","rewrite logarithm", editorElement);
      break;
    case "cmd_compute_RewriteSinAndCos":
      doEvalComputation(element,eng.Rewrite_sin_and_cos,"<mo>=</mo>","rewrite sincos", editorElement);
      break;
    case "cmd_compute_RewriteSinhAndCosh":
      doEvalComputation(element,eng.Rewrite_sinh_and_cosh,"<mo>=</mo>","rewrite sinhcosh", editorElement);
      break;
    case "cmd_compute_RewriteSin":
      doEvalComputation(element,eng.Rewrite_sin,"<mo>=</mo>","rewrite sin", editorElement);
      break;
    case "cmd_compute_RewriteCos":
      doEvalComputation(element,eng.Rewrite_cos,"<mo>=</mo>","rewrite cos", editorElement);
      break;
    case "cmd_compute_RewriteTan":
      doEvalComputation(element,eng.Rewrite_tan,"<mo>=</mo>","rewrite tan", editorElement);
      break;
    case "cmd_compute_RewriteArcsin":
      doEvalComputation(element,eng.Rewrite_arcsin,"<mo>=</mo>","rewrite arcsin", editorElement);
      break;
    case "cmd_compute_RewriteArccos":
      doEvalComputation(element,eng.Rewrite_arccos,"<mo>=</mo>","rewrite arccos", editorElement);
      break;
    case "cmd_compute_RewriteArctan":
      doEvalComputation(element,eng.Rewrite_arctan,"<mo>=</mo>","rewrite arctan", editorElement);
      break;
    case "cmd_compute_RewriteArccot":
      doEvalComputation(element,eng.Rewrite_arccot,"<mo>=</mo>","rewrite arccot", editorElement);
      break;
    case "cmd_compute_RewritePolar":
      doEvalComputation(element,eng.Rewrite_Polar,"<mo>=</mo>","rewrite polar", editorElement);
      break;
    case "cmd_compute_RewriteRectangular":
      doEvalComputation(element,eng.Rewrite_Rectangular,"<mo>=</mo>","rewrite rectangular", editorElement);
      break;
    case "cmd_compute_RewriteNormal":
      doEvalComputation(element,eng.Rewrite_Normal_Form,"<mo>=</mo>","rewrite normal", editorElement);
      break;
    case "cmd_compute_RewriteEquationsAsMatrix":
      doVarsComputation(element,", Corresponding matrix: ",eng.equationsAsMatrix,GetComputeString("EqnsAsMatrix.title"), editorElement, cmd, cmdHandler);
      break;
    case "cmd_compute_RewriteMatrixAsEquations":
      doVarsComputation(element,", Corresponding equations: ",eng.matrixAsEquations,GetComputeString("MatrixAsEqns.title"), editorElement, cmd, cmdHandler);
      break;
    case "cmd_compute_CheckEquality":
      doLabeledComputation(element,eng.Check_Equality,"CheckEquality.fmt", editorElement);
      break;
    case "cmd_compute_SolveExact":
      doComputeSolveExact(element, "", editorElement, cmd, cmdHandler);
      break;
    case "cmd_compute_SolveNumeric":
      doComputeSolveNumeric(element, editorElement);
      break;
    case "cmd_compute_SolveRecursion":
      doComputeSolveRecursion(element, editorElement);
      break;
    case "cmd_compute_Collect":
      doVarsEvalComputation(element,eng.collect,"<mo>=</mo>",GetComputeString("Collect.title"),"", editorElement, cmd, cmdHandler);
      break;
    case "cmd_compute_Divide":
      doComputeDivide(element, "", editorElement, cmd, cmdHandler);
      break;
    case "cmd_compute_PartialFractions":
      doComputePartialFractions(element, "", editorElement, cmd, cmdHandler);
      break;
    case "cmd_compute_Roots":
      doLabeledComputation(element,eng.Polynomial_Roots,"Roots.fmt", editorElement);
      break;
    case "cmd_compute_Sort":
      doComputeSort(element, "", editorElement, cmd, cmdHandler);
      break;
    case "cmd_compute_CompanionMatrix":
      doComputeCompanionMatrix(element, "", editorElement, cmd, cmdHandler);
      break;
    case "cmd_compute_ByParts":
      doVarsEvalComputation(element,eng.byParts,"<mo>=</mo>",GetComputeString("ByParts.title"),GetComputeString("ByParts.remark"), editorElement, cmd, cmdHandler);
      break;
    case "cmd_compute_ChangeVariable":
      doVarsEvalComputation(element,eng.changeVar,"<mo>=</mo>",GetComputeString("ChangeVar.title"),GetComputeString("ChangeVar.remark"), editorElement, cmd, cmdHandler);
      break;
    case "cmd_compute_ApproxIntegral":
      doComputeApproxIntegral(element, editorElement);
      break;
    case "cmd_compute_ImplicitDiff":
      doComputeImplicitDiff(element, editorElement, cmdHandler);
      break;
    case "cmd_compute_SolveODEExact":
      doComputeSolveODE(element,"ODE.fmt",eng.solveODEExact,"ODE.title", "", editorElement, cmd, cmdHandler);
      break;
    case "cmd_compute_SolveODELaplace":
      doComputeSolveODE(element,"ODELaplace.fmt",eng.solveODELaplace,"ODELaplace.title", "", editorElement, cmd, cmdHandler);
      break;
    case "cmd_compute_SolveODENumeric":
      doComputeSolveODE(element,"ODENumeric.fmt",eng.solveODENumeric,"ODENumeric.title", "", editorElement, cmd, cmdHandler);
      break;
    case "cmd_compute_SolveODESeries":
      doComputeSolveODESeries(element, editorElement);
      break;
    case "cmd_compute_PowerSeries":
      doComputePowerSeries(element, editorElement, null);
      break;
    case "cmd_compute_Fourier":
      doLabeledComputation(element,eng.Fourier_Transform,"Fourier.fmt", editorElement);
      break;
    case "cmd_compute_InverseFourier":
      doLabeledComputation(element,eng.Inverse_Fourier_Transform,"InvFourier.fmt", editorElement);
      break;
    case "cmd_compute_Laplace":
      doLabeledComputation(element,eng.Laplace_Transform,"Laplace.fmt", editorElement);
      break;
    case "cmd_compute_InverseLaplace":
      doLabeledComputation(element,eng.Inverse_Laplace_Transform,"InvLaplace.fmt", editorElement);
      break;
    case "cmd_compute_Gradient":
      doLabeledComputation(element,eng.Gradient,"Gradient.fmt", editorElement);
      break;
    case "cmd_compute_Divergence":
      doLabeledComputation(element,eng.Divergence,"Divergence.fmt", editorElement);
      break;
    case "cmd_compute_Curl":
      doLabeledComputation(element,eng.Curl,"Curl.fmt", editorElement);
      break;
    case "cmd_compute_Laplacian":
      doLabeledComputation(element,eng.Laplacian,"Laplacian.fmt", editorElement);
      break;
    case "cmd_compute_Jacobian":
      doLabeledComputation(element,eng.Jacobian,"Jacobian.fmt", editorElement);
      break;
    case "cmd_compute_Hessian":
      doLabeledComputation(element,eng.Hessian,"Hessian.fmt", editorElement);
      break;
    case "cmd_compute_Wronskian":
      doComputeWronskian(element, "", editorElement, cmd, cmdHandler);
      break;
    case "cmd_compute_ScalarPot":
      doLabeledComputation(element,eng.Scalar_Potential,"ScalarPot.fmt", editorElement);
      break;
    case "cmd_compute_VectorPot":
      doLabeledComputation(element,eng.Vector_Potential,"VectorPot.fmt", editorElement);
      break;

    case "cmd_MSIComputeAdjugate":  
      doLabeledComputation(element,eng.Adjugate,"Adjugate.fmt", editorElement);
      break;
    case "cmd_MSIComputeCharPoly":
      doComputeCharPoly(element, "", editorElement, cmd, cmdHandler);
      break;
    case "cmd_MSIComputeCholesky":      
      doLabeledComputation(element,eng.Cholesky_Decomposition,"Cholesky.fmt", editorElement);
      break;
    case "cmd_MSIComputeColBasis":   
      doLabeledComputation(element,eng.Column_Basis,"ColBasis.fmt", editorElement);
      break;
    case "cmd_MSIComputeConcat":   
      doLabeledComputation(element,eng.Concatenate,"Concat.fmt", editorElement);
      break;
    case "cmd_MSIComputeConditionNum":  
      doLabeledComputation(element,eng.Condition_Number,"ConditionNum.fmt", editorElement);
      break;
    case "cmd_MSIComputeDefinitenessTests":  
      doLabeledComputation(element,eng.Definiteness_Tests,"DefTest.fmt", editorElement);
      break;
    case "cmd_MSIComputeDeterminant":  
      doLabeledComputation(element,eng.Determinant,"Determinant.fmt", editorElement);
      break;
    case "cmd_MSIComputeEigenvalues":
      doLabeledComputation(element,eng.Eigenvalues,"Eigenvalues.fmt", editorElement);
      break;
    case "cmd_MSIComputeEigenvectors":
      // formatting needs to be fixed
      doLabeledComputation(element,eng.Eigenvectors,"Eigenvectors.fmt", editorElement);
      break;
    case "cmd_MSIComputeFFGE":
      doLabeledComputation(element,eng.Fraction_Free_Gaussian_Elimination,"FFGaussElim.fmt", editorElement);
      break;
    case "cmd_MSIComputeGaussElim":
      doLabeledComputation(element,eng.Gaussian_Elimination,"GaussElim.fmt", editorElement);
      break;
    case "cmd_MSIComputeHermite":  
      doLabeledComputation(element,eng.Hermite_Normal_Form,"Hermite.fmt", editorElement);
      break;
    case "cmd_MSIComputeHermitianTranspose":  
      doLabeledComputation(element,eng.Hermitian_Transpose,"HermitianTr.fmt", editorElement);
      break;
    case "cmd_MSIComputeInverse":  
      doLabeledComputation(element,eng.Inverse,"Inverse.fmt", editorElement);
      break;
    case "cmd_MSIComputeJordan":  
      doLabeledComputation(element,eng.Jordan_Form,"Jordan.fmt", editorElement);
      break;
    case "cmd_MSIComputeMap":  
      doComputeMap(element, editorElement, cmd, cmdHandler);
      break;
    case "cmd_MSIComputeMinPoly":
      doComputeMinPoly(element, "", editorElement, cmd, cmdHandler);
      break;
    case "cmd_MSIComputeNorm":     
      doLabeledComputation(element,eng.Norm,"Norm.fmt", editorElement);
      break;
    case "cmd_MSIComputeNullspaceBasis":   
      doLabeledComputation(element,eng.Nullspace_Basis,"Nullspace.fmt", editorElement);
      break;
    case "cmd_MSIComputeOrthogonalityTest":  
      doLabeledComputation(element,eng.Orthogonality_Test,"Orthogonality.fmt", editorElement);
      break;
    case "cmd_MSIComputePermanent":  
      doLabeledComputation(element,eng.Permanent,"Permanent.fmt", editorElement);
      break;
    case "cmd_MSIComputePLU":      
      doLabeledComputation(element,eng.PLU_Decomposition,"PLU.fmt", editorElement);
      break;
    case "cmd_MSIComputeRank":  
      doLabeledComputation(element,eng.Rank,"Rank.fmt", editorElement);
      break;
    case "cmd_MSIComputeRationalCanonical":  
      doLabeledComputation(element,eng.Rational_Canonical_Form,"Rational.fmt", editorElement);
      break;
    case "cmd_MSIComputeRREF":
      doLabeledComputation(element,eng.Reduced_Row_Echelon_Form,"RREchelonForm.fmt", editorElement);
      break;
    case "cmd_MSIComputeReshape":       
      doComputeReshape(element, editorElement);
      break;
    case "cmd_MSIComputeRowBasis":   
      doLabeledComputation(element,eng.Row_Basis,"RowBasis.fmt", editorElement);
      break;
    case "cmd_MSIComputeQR":       
      doLabeledComputation(element,eng.QR_Decomposition,"QR.fmt", editorElement);
      break;
    case "cmd_MSIComputeSingularValues":      
      doLabeledComputation(element,eng.Singular_Values,"Singular.fmt", editorElement);
      break;
    case "cmd_MSIComputeSVD":      
      doLabeledComputation(element,eng.SVD,"SVD.fmt", editorElement);
      break;
    case "cmd_MSIComputeSmith":    
      doLabeledComputation(element,eng.Smith_Normal_Form,"Smith.fmt", editorElement);
      break;
    case "cmd_MSIComputeSpectralRadius":    
      doLabeledComputation(element,eng.Spectral_Radius,"SpectralRadius.fmt", editorElement);
      break;
    case "cmd_MSIComputeStack":    
      doLabeledComputation(element,eng.Stack,"Stack.fmt", editorElement);
      break;
    case "cmd_MSIComputeTrace":
      doLabeledComputation(element,eng.Trace,"Trace.fmt", editorElement);
      break;
    case "cmd_MSIComputeTranspose":
      doLabeledComputation(element,eng.Transpose,"Transpose.fmt", editorElement);
      break;

    case "cmd_compute_SimplexDual":
      doLabeledComputation(element,eng.Simplex_Dual,"Dual.fmt", editorElement);
      break;
    case "cmd_compute_SimplexFeasible":
      doLabeledComputation(element,eng.Simplex_Feasible,"Feasible.fmt", editorElement);
      break;
    case "cmd_compute_SimplexMaximize":
      doLabeledComputation(element,eng.Simplex_Maximize,"Maximize.fmt", editorElement);
      break;
    case "cmd_compute_SimplexMinimize":
      doLabeledComputation(element,eng.Simplex_Minimize,"Minimize.fmt", editorElement);
      break;
    case "cmd_compute_SimplexStandardize":
      doLabeledComputation(element,eng.Simplex_Standardize,"Standardize.fmt", editorElement);
      break;

    case "cmd_compute_FitCurve":
      doComputeFitCurve(element, editorElement);
      break;
    case "cmd_compute_Mean":
      doLabeledComputation(element,eng.Mean,"Mean.fmt", editorElement);
      break;
    case "cmd_compute_Median":
      doLabeledComputation(element,eng.Median,"Median.fmt", editorElement);
      break;
    case "cmd_compute_Mode":
      doLabeledComputation(element,eng.Mode,"Mode.fmt", editorElement);
      break;
    case "cmd_compute_Correlation":
      doLabeledComputation(element,eng.Correlation,"Correlation.fmt", editorElement);
      break;
    case "cmd_compute_Covariance":
      doLabeledComputation(element,eng.Covariance,"Covariance.fmt", editorElement);
      break;
    case "cmd_compute_GeometricMean":
      doLabeledComputation(element,eng.Geometric_Mean,"GeometricMean.fmt", editorElement);
      break;
    case "cmd_compute_HarmonicMean":
      doLabeledComputation(element,eng.Harmonic_Mean,"HarmonicMean.fmt", editorElement);
      break;
    case "cmd_compute_MeanDeviation":
      doLabeledComputation(element,eng.Mean_Deviation,"MeanDeviation.fmt", editorElement);
      break;
    case "cmd_compute_Moment":
      doComputeMoment(element, editorElement, null);
      break;
    case "cmd_compute_Quantile":
      doComputeQuantile(element, editorElement, cmd, cmdHandler);
      break;
    case "cmd_compute_StandardDeviation":
      doLabeledComputation(element,eng.Standard_Deviation,"StdDeviation.fmt", editorElement);
      break;
    case "cmd_compute_Variance":
      doLabeledComputation(element,eng.Variance,"Variance.fmt", editorElement);
      break;

    case "cmd_compute_Plot2DRectangular":
      doComputePlot(element, 2, "rectangular", editorElement);
      break;
    case "cmd_compute_Plot2DPolar":
      doComputePlot(element, 2, "polar", editorElement);
      break;
    case "cmd_compute_Plot2DImplicit":
      doComputePlot(element, 2, "implicit", editorElement);
      break;
    case "cmd_compute_Plot2DInequality":
      doComputePlot(element, 2, "inequality", editorElement);
      break;
    case "cmd_compute_Plot2DParametric":
      doComputePlot(element, 2, "parametric", editorElement);
      break;
    case "cmd_compute_Plot2DConformal":
      doComputePlot(element, 2, "conformal", editorElement);
      break;
    case "cmd_compute_Plot2DGradient":
      doComputePlot(element, 2, "gradient", editorElement);
      break;
    case "cmd_compute_Plot2DVector":
      doComputePlot(element, 2, "vectorField", editorElement);
      break;
    case "cmd_compute_Plot2DODE":
      doComputePlot(element, 2, "ode", editorElement);
      break;
    case "cmd_compute_Plot2DAI":
      doComputePlot(element, 2, "approximateIntegral", editorElement);
      break;
    case "cmd_compute_Plot3DRectangular":
      doComputePlot(element, 3, "rectangular", editorElement);
      break;
    case "cmd_compute_Plot3DCurve":
      doComputePlot(element, 3, "curve", editorElement);
      break;
    case "cmd_compute_Plot3DCylindrical":
      doComputePlot(element, 3, "cylindrical", editorElement);
      break;
    case "cmd_compute_Plot3DSpherical":
      doComputePlot(element, 3, "spherical", editorElement);
      break;
    case "cmd_compute_Plot3DParametric":
      doComputePlot(element, 3, "parametric", editorElement);
      break;
    case "cmd_compute_Plot3DImplicit":
      doComputePlot(element, 3, "implicit", editorElement);
      break;
    case "cmd_compute_Plot3DTube":
      doComputePlot(element, 3, "tube", editorElement);
      break;
    case "cmd_compute_Plot3DGradient":
      doComputePlot(element, 3, "gradient", editorElement);
      break;
    case "cmd_compute_Plot3DVector":
      doComputePlot(element, 3, "vectorField", editorElement);
      break;
    case "cmd_compute_Plot2DARectangular":
      doComputePlot(element, 2, "rectangular", true, editorElement);
      break;
    case "cmd_compute_Plot2DAPolar":
      doComputePlot(element, 2, "polar", true, editorElement);
      break;
    case "cmd_compute_Plot2DAImplicit":
      doComputePlot(element, 2, "implicit", true, editorElement);
      break;
    case "cmd_compute_Plot2DAInequality":
      doComputePlot(element, 2, "inequality", true, editorElement);
      break;
    case "cmd_compute_Plot2DAParametric":
      doComputePlot(element, 2, "parametric", true, editorElement);
      break;
    case "cmd_compute_Plot2DAConformal":
      doComputePlot(element, 2, "conformal", true, editorElement);
      break;
    case "cmd_compute_Plot2DAGradient":
      doComputePlot(element, 2, "gradient", true, editorElement);
      break;
    case "cmd_compute_Plot2DAVector":
      doComputePlot(element, 2, "vectorField", true, editorElement);
      break;
    case "cmd_compute_Plot3DARectangular":
      doComputePlot(element, 3, "rectangular", true, editorElement);
      break;
    case "cmd_compute_Plot3DACurve":
      doComputePlot(element, 3, "curve", true, editorElement);
      break;
    case "cmd_compute_Plot3DACylindrical":
      doComputePlot(element, 3, "cylindrical", true, editorElement);
      break;
    case "cmd_compute_Plot3DASpherical":
      doComputePlot(element, 3, "spherical", true, editorElement);
      break;
    case "cmd_compute_Plot3DAParametric":
      doComputePlot(element, 3, "parametric", true, editorElement);
      break;
    case "cmd_compute_Plot3DAImplicit":
      doComputePlot(element, 3, "implicit", true, editorElement);
      break;
    case "cmd_compute_Plot3DATube":
      doComputePlot(element, 3, "tube", true, editorElement);
      break;
    case "cmd_compute_Plot3DAGradient":
      doComputePlot(element, 3, "gradient", true, editorElement);
      break;
    case "cmd_compute_Plot3DAVector":
      doComputePlot(element, 3, "vectorField", true, editorElement);
      break;
    case "cmd_compute_EditPlot":
      doEditPlot();
      break;
    case "cmd_compute_Define":
      doComputeDefine(element, editorElement);
      break;
    case "cmd_compute_Undefine":
      doComputeUndefine(element);
      break;
    case "cmd_compute_Interpret":
      doEvalComputation(element,eng.Interpret,"<mo>=</mo>","interpret", editorElement);
      break;
    case "cmd_compute_Fixup":
      doFixupComputation(element,eng.Fixup,"<mo>=</mo>","fixup", editorElement);
      break;
    default:
      dump("Unknown compute command. (" + cmd + ")\n");
      return;
} } }

function doGlobalComputeCommand(cmd, editorElement)
{
  if(!editorElement)
    editorElement = msiGetActiveEditorElement();
  switch (cmd) {
  case "cmd_compute_Iterate":
    doComputeIterate(editorElement, null);
    break;
  case "cmd_MSIComputeFillMatrix":       
    doComputeFillMatrix(editorElement, null);
    break;
  case "cmd_MSIComputeRandomMatrix":       
    doComputeRandomMatrix(editorElement);
    break;
  case "cmd_MSIComputeRandomNumbers":       
    doComputeRandomNumbers(editorElement);
    break;
  case "cmd_compute_SetBasisVariables":
    doComputeSetBasisVars(editorElement, cmd);
    break;
  case "cmd_compute_ShowDefs":
    doComputeShowDefs(editorElement);
    break;
  case "cmd_compute_ClearDefs":
    doComputeClearDefs();
    break;
  case "cmd_compute_UserSettings":
    doComputeUserSettings();
    break;
  case "cmd_compute_Settings":
    doComputeSettings();
    break;
  case "cmd_compute_SwitchEngines":
    doComputeSwitchEngines();
    break;
  case "cmd_compute_Passthru":
    doComputePassthru(editorElement);
    break;
  default:
    dump("Unknown global compute command. (" + cmd + ")\n");
    return;
} }

// our connection to the computation code
var compsample;
var compengine;

function GetCurrentEngine()
{
  if (!compsample) {
    compsample = Components.classes["@mackichan.com/simplecomputeengine;2"].getService(Components.interfaces.msiISimpleComputeEngine);
    try {
      var dsprops = Components.classes["@mozilla.org/file/directory_service;1"].createInstance(Components.interfaces.nsIProperties);
//      var inifile = dsprops.get("resource:app", Components.interfaces.nsIFile);
      var inifile = dsprops.get("GreD", Components.interfaces.nsIFile);
      inifile.append("mupInstall.gmr");
      var inipath = inifile.path;
      compsample.startup(inifile.path);
      compengine = 2;
      msiComputeLogger.Init();
    } catch(e) {
      var msg_key;
      if (e.result == Components.results.NS_ERROR_NOT_AVAILABLE)
        msg_key = "Error.notavailable";
      else if (e.result == Components.results.NS_ERROR_FILE_NOT_FOUND)
        msg_key = "Error.notfound";
      else if (e.result == Components.results.NS_ERROR_NOT_INITIALIZED)
        msg_key = "Error.notinitialized";
      else if (e.result == Components.results.NS_ERROR_FAILURE)
        msg_key = "Error.failure";
      else
        throw e;
      AlertWithTitle(GetComputeString("Error.title"), GetComputeString(msg_key));
    }
  }
  return compsample;
}

// cursor helpers
function ComputeCursor(editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var theWin = msiGetWindowContainingEditor(editorElement);
  theWin.setCursor( "spinning" );
}

function RestoreCursor(editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var theWin = msiGetWindowContainingEditor(editorElement);
  theWin.setCursor( "default" );
}

// return node on RHS of =, if structure is that simple
function GetRHS(math)
{
  var ch = last_child(math);
  while (ch) {
    if (ch.nodeType == Node.ELEMENT_NODE && ch.localName == "mo") {
      var op = ch.firstChild;
      if (op.nodeType == Node.TEXT_NODE && op.data == "=") {
        var m = node_after(ch);
        if (m)
          return m;
    } }
    ch = node_before(ch);
  }
  return math;
}

//Moved to msiEditorUtilities.js
//function insertXML(editor, text, node, offset)
//{
//  var parser = new DOMParser();
//  var doc = parser.parseFromString(text,"application/xhtml+xml");
//  var nodeList = doc.documentElement.childNodes;
//  var nodeListLength = nodeList.length;
//  var i;
//  for (i = nodeListLength-1; i >= 0; --i)
//  {
//    editor.insertNode( nodeList[i], node, offset );
//  }
//}

//SLS for unknown reasons, get parsing error if text is at outer level
function insertLabeledXML(editor, text, node, offset)
{
  var parser = new DOMParser();
  var wrapped = "<span>" + text + "</span>";
  var doc = parser.parseFromString(wrapped,"application/xhtml+xml");
  var nodeList = doc.documentElement.childNodes;
  var nodeListLength = nodeList.length;
  var i;
  for (i = nodeListLength-1; i >= 0; --i)
  {
    editor.insertNode( nodeList[i], node, offset );
  }
}

function appendResult(result,sep,math,editorElement)
{
  if(!editorElement)
    editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);
  var appendedResult = result.replace(fullmath, fullmath+sep);
  insertXML( editor, appendedResult, math, math.childNodes.length );
  /*
  msiGetEditor(editorElement).insertHTMLWithContext(
      result.replace(fullmath,fullmath+sep),
      "", "", "", null,
      math, math.childNodes.length, false );   */
  coalescemath(editorElement);
}

function appendLabel(label,math,editorElement)
{
  if(!editorElement)
    editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);
  insertXML( editor, label, math, math.childNodes.length );
//  msiGetEditor(editorElement).insertHTMLWithContext(
//      label,
//      "", "", "", null,
//      math, math.childNodes.length, false );
}

function appendLabeledResult(result,label,math,editorElement)
{
  if(!editorElement)
    editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);
  var str;
  if (-1 == label.search(/%result%/))
    str = result.replace(fullmath,label+fullmath);
  else
    str = label.replace(/%result%/,result);
  insertLabeledXML(editor, str, math, math.childNodes.length );
//  msiGetEditor(editorElement).insertHTMLWithContext(str,"","","",null,math,math.childNodes.length,false);
}

function appendTaggedResult(result,label,body,index,editorElement)
{
  if(!editorElement)
    editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);
  var str = label.replace(/%result%/,result);

  insertXML(editor, str, body, index );
//  msiGetEditor(editorElement).insertHTMLWithContext(str,"","","",null,body,index,false);
}

function addResult(result, editorElement)
{
  if(!editorElement)
    editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);
  var selection = editor.selection;
  insertXML(editor, result, selection.focusNode, selection.focusOffset );
//  editor.insertHTMLWithContext(result,"","","",null,selection.focusNode,selection.focusOffset,false);
}

function runFixup(math)
{
  try {
    var out = GetCurrentEngine().perform(math,GetCurrentEngine().Fixup);
    return out;
  } catch(e) {
    msiComputeLogger.Exception(e);
    return math;  // what else to do?  Fixup shouldn't fail.
  }
}

function GetFixedMath(math)
{
  return runFixup(GetMathAsString(math));
}


function doLabeledComputation(math,op,labelID, editorElement)
{
  var mathstr = GetFixedMath(GetRHS(math));
  msiComputeLogger.Sent("doing "+labelID+" after fixup",mathstr);
  ComputeCursor(editorElement);
  try {
    var out = GetCurrentEngine().perform(mathstr,op);
    msiComputeLogger.Received(out);
    appendLabeledResult(out,GetComputeString(labelID),math, editorElement);
  } catch (e) {
    msiComputeLogger.Exception(e);
  }
  RestoreCursor(editorElement);
}

// like above, but use operator instead of text between input and result
function doEvalComputation(math,op,joiner,remark, editorElement)
{
  var mathstr = GetFixedMath(GetRHS(math));
  msiComputeLogger.Sent(remark+" after fixup",mathstr);
  ComputeCursor(editorElement);
  try {
    var out = GetCurrentEngine().perform(mathstr,op);
    msiComputeLogger.Received(out);
    appendResult(out,joiner,math, editorElement);
  } catch (e) {
    msiComputeLogger.Exception(e);
  }
  RestoreCursor(editorElement);
}

// like above, but make sure to run Fixup exactly once
function doFixupComputation(math,op,joiner,remark, editorElement)
{
  var mathstr = GetMathAsString(GetRHS(math));
  msiComputeLogger.Sent(remark,mathstr);
  ComputeCursor(editorElement);
  try {
    var out = GetCurrentEngine().perform(mathstr,op);
    msiComputeLogger.Received(out);
    appendResult(out,joiner,math, editorElement);
  } catch (e) {
    msiComputeLogger.Exception(e);
  }
  RestoreCursor(editorElement);
}

function postDialogTimerCallback(editorElement, obj)
{
  dump("Hit postDialogTimerCallback!\n");
  if (obj == null)
  {
    dump("No object passed in to postDialogTimerCallback!\n");
    return;
  }
  clearTimeout(obj.mDialogTimer);
  if (("afterDialog" in obj) && obj.afterDialog != null)
    obj.afterDialog(editorElement);
}

// like above, but asks user for variable(s) first
function doVarsComputation(math, label, func, title, editorElement, cmd, cmdHandler)
{
  var o = new Object();
  o.title = title;
  o.mParentWin = this;
  o.theMath = math;
  o.theLabel = label;
  o.theFunc = func;
  o.afterDialog = function(editorElement)
    { 
      this.mParentWin.finishVarsComputation(editorElement, this);
    };
//  var parentWin = msiGetParentWindowForNewDialog(editorElement);
  var theDialog = null;
  try {
    theDialog = msiOpenModelessDialog("chrome://prince/content/ComputeVariables.xul", "_blank", "chrome,close,titlebar,resizable,dependent",
                                      editorElement, cmd, cmdHandler, o);
  } catch(e) {AlertWithTitle("Error in computeOverlay.js", "Exception in doVarsComputation: [" + e + "]"); return;}
//  parentWin.openDialog("chrome://prince/content/ComputeVariables.xul", "_blank", "chrome,close,titlebar,modal", o);
}

function finishVarsComputation(editorElement, o)
{
  if (o.Cancel)
    return;
  var vars = o.vars;
  var mathstr = GetFixedMath(o.theMath);
//  vars = runFixup(vars);
  msiComputeLogger.Sent4(o.theLabel + " after fixup", mathstr, "specifying", vars);

  ComputeCursor(editorElement);
  try {
    var out = o.theFunc(mathstr,vars);
    msiComputeLogger.Received(out);
    appendLabeledResult(out, o.theLabel, o.theMath, editorElement);
  } catch(e) {
    msiComputeLogger.Exception(e);
  }
  RestoreCursor(editorElement);
}

// like above, but use operator instead of text between input and result
function doVarsEvalComputation(math, func, joiner, title, label, editorElement, cmd, cmdHandler)
{
  var o = new Object();
  o.title = title;
  o.label = label;
  o.theMath = math;
  o.theFunc = func;
  o.theJoiner = joiner;
  o.mParentWin = this;
  o.afterDialog = function(editorElement)
    { 
      this.mParentWin.finishVarsEvalComputation(editorElement, this);
    };
  var parentWin = msiGetParentWindowForNewDialog(editorElement);
  try {
    theDialog = msiOpenModelessDialog("chrome://prince/content/ComputeVariables.xul", "_blank", "chrome,close,titlebar,resizable,dependent",
                                      editorElement, cmd, cmdHandler, o);
  } catch(e) {AlertWithTitle("Error in computeOverlay.js", "Exception in doVarsEvalComputation: [" + e + "]"); return;}
//  parentWin.openDialog("chrome://prince/content/ComputeVariables.xul", "_blank", "chrome,close,titlebar,modal", o);
}

function finishVarsEvalComputation(editorElement, o)
{
  if (o.Cancel)
    return;
  var vars = o.vars;
  var mathstr = GetFixedMath(o.theMath);
//  vars = runFixup(vars);
  msiComputeLogger.Sent4(title+" after fixup",mathstr,"specifying",vars);
  
  ComputeCursor(editorElement);
  try {
    var out = o.theFunc(mathstr,vars);
    msiComputeLogger.Received(out);
    appendResult(out, o.theJoiner, o.theMath, editorElement);
  } catch(e) {
    msiComputeLogger.Exception(e);
  }
  RestoreCursor(editorElement);
}

// actual command handlers

function doComputeSolveExact(math, vars, editorElement, cmd, cmdHandler)
{
  var mathstr = GetFixedMath(math);
  if (!vars)
    vars = "";
//  else if (vars.length > 0)
//    vars = runFixup(vars);
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();

//  var vars = "";
//  var done = false;
//  while (!done) {
    msiComputeLogger.Sent4("solve exact",mathstr,"specifying",vars);
    try {
      ComputeCursor(editorElement);
      var out = GetCurrentEngine().solveExact(mathstr,vars);
      msiComputeLogger.Received(out);
      appendLabeledResult(out,GetComputeString("Solution.fmt"),math, editorElement);
      RestoreCursor(editorElement);
//      done = true;
    } catch(ex) {
      RestoreCursor(editorElement);
      if (ex.result == compsample.nosol) {
        appendLabel(GetComputeString("NoSolution"),math, editorElement);
        done = true;
      } else if (ex.result == compsample.needvars) {
        var o = new Object();
        o.mParentWin = this;
        o.theMath = math;
        o.vars = vars;
        o.theCommand = cmd;
        o.theCommandHandler = cmdHandler;
        o.afterDialog = function(editorElement)
        { 
          if (this.Cancel)
            return;
          this.mParentWin.doComputeSolveExact(this.theMath, this.vars, editorElement, this.theCommand, this.theCommandHandler);
        };
        try {
          theDialog = msiOpenModelessDialog("chrome://prince/content/ComputeVariables.xul", "_blank", "chrome,close,titlebar,resizable,dependent",
                                            editorElement, cmd, cmdHandler, o);
        } catch(e) {AlertWithTitle("Error in computeOverlay.js", "Exception in doComputeSolveExact: [" + e + "]"); return;}
//        parentWin.openDialog("chrome://prince/content/ComputeVariables.xul", "_blank", "chrome,close,titlebar,modal", o);
//        if (o.Cancel)
//          return;
//        vars = runFixup(o.vars);
      } else {
        msiComputeLogger.Exception(ex);
//        done = true;
      } 
//    } 
  } 
}

function doComputeSolveNumeric(math, editorElement)
{
  var mathstr = GetFixedMath(math);
  msiComputeLogger.Sent("solve numeric after fixup",mathstr);
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();

  ComputeCursor(editorElement);
  try {
    var out = GetCurrentEngine().perform(mathstr,GetCurrentEngine().Solve_Numeric);
    msiComputeLogger.Received(out);
    appendLabeledResult(out,GetComputeString("Solution.fmt"),math, editorElement);
  } catch(ex) {
    if (ex.result == compsample.nosol) {
      appendLabel(GetComputeString("NoSolution"),math, editorElement);
    } else {
      msiComputeLogger.Exception(ex);
  } }
  RestoreCursor(editorElement);
}

function doComputeSolveRecursion(math, editorElement)
{
  var mathstr = GetFixedMath(math);
  msiComputeLogger.Sent("solve recursion after fixup",mathstr);
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();

  ComputeCursor(editorElement);
  try {
    var out = GetCurrentEngine().perform(mathstr,GetCurrentEngine().Solve_Recursion);
    msiComputeLogger.Received(out);
    appendLabeledResult(out,GetComputeString("Solution.fmt"),math, editorElement);
  } catch(ex) {
    if (ex.result == compsample.nosol) {
      appendLabel(GetComputeString("NoSolution"),math, editorElement);
    } else {
      msiComputeLogger.Exception(ex);
  } }
  RestoreCursor(editorElement);
}

function doComputePartialFractions(math, vars, editorElement, cmd, cmdHandler)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var mathstr = GetFixedMath(math);
  if (!vars)
    vars = "";
//  var done = false;
//  while (!done) {
    msiComputeLogger.Sent4("partial fractions",mathstr,"specifying",vars);
    try {
      ComputeCursor(editorElement);
      var out = GetCurrentEngine().partialFractions(mathstr,vars);
      msiComputeLogger.Received(out);
      appendResult(out,"<mo>=</mo>",math, editorElement);
      RestoreCursor(editorElement);
//      done = true;
    } catch(ex) {
      RestoreCursor(editorElement);
      if (ex.result == compsample.needvars) {
        var o = new Object();
        o.title = GetComputeString("ParFrac.title");
        o.mParentWin = this;
        o.theMath = math;
        o.vars = vars;
        o.theCommand = cmd;
        o.theCommandHandler = cmdHandler;
        o.afterDialog = function(editorElement)
        { 
          if (this.Cancel)
            return;
          this.mParentWin.doComputePartialFractions(this.theMath, this.vars, editorElement, this.theCommand, this.theCommandHandler);
        };
        try {
          var theDialog = msiOpenModelessDialog("chrome://prince/content/ComputeVariables.xul", "_blank", "chrome,close,titlebar,resizable,dependent",
                                            editorElement, cmd, cmdHandler, o);
        } catch(e) {AlertWithTitle("Error in computeOverlay.js", "Exception in doComputeSolveExact: [" + e + "]"); return;}

//        var parentWin = msiGetParentWindowForNewDialog(editorElement);
//        parentWin.openDialog("chrome://prince/content/ComputeVariables.xul", "_blank", "chrome,close,titlebar,modal", o);
//        if (o.Cancel)
//          return;
//        vars = runFixup(o.vars);
      } else {
        msiComputeLogger.Exception(ex);
      } 
    } 
//  } 
}

function doComputeDivide(math, vars, editorElement, cmd, cmdHandler)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var mathstr = GetFixedMath(math);
  
  if (!vars)
    vars = "";
//  var done = false;
//  while (!done) {
    msiComputeLogger.Sent4("divide",mathstr,"specifying",vars);
    try {
      ComputeCursor(editorElement);
      var out = GetCurrentEngine().divide(mathstr,vars);
      msiComputeLogger.Received(out);
      appendResult(out,"<mo>=</mo>",math, editorElement);
      RestoreCursor(editorElement);
//      done = true;
    } catch(ex) {
      RestoreCursor(editorElement);
      if (ex.result == compsample.needvars) {
        var o = new Object();
        o.title = GetComputeString("Divide.title");
        o.mParentWin = this;
        o.theMath = math;
        o.vars = vars;
        o.theCommand = cmd;
        o.theCommandHandler = cmdHandler;
        o.afterDialog = function(editorElement)
        { 
          if (this.Cancel)
            return;
          this.mParentWin.doComputeDivide(this.theMath, this.vars, editorElement, this.theCommand, this.theCommandHandler);
        };
        try {
          var theDialog = msiOpenModelessDialog("chrome://prince/content/ComputeVariables.xul", "_blank", "chrome,close,titlebar,resizable,dependent",
                                            editorElement, cmd, cmdHandler, o);
        } catch(e) {AlertWithTitle("Error in computeOverlay.js", "Exception in doComputeDivide: [" + e + "]"); return;}

//        var parentWin = msiGetParentWindowForNewDialog(editorElement);
//        parentWin.openDialog("chrome://prince/content/ComputeVariables.xul", "_blank", "chrome,close,titlebar,modal", o);
        if (o.Cancel)
          return;
        vars = runFixup(o.vars);
      } else {
        msiComputeLogger.Exception(ex);
      }
    }
//  }
}

function doComputeSort(math, vars, editorElement, cmd, cmdHandler)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  if (!vars)
    vars = "";
  var mathstr = GetFixedMath(math);
  msiComputeLogger.Sent4("sort",mathstr,"specifying",vars);
  try {
    ComputeCursor(editorElement);
    var out = GetCurrentEngine().sort(mathstr,vars);
    msiComputeLogger.Received(out);
    appendResult(out,"<mo>=</mo>",math, editorElement);
    RestoreCursor(editorElement);
  } catch(ex) {
    RestoreCursor(editorElement);
    if (ex.result == compsample.needvars) {
      var o = new Object();
      o.title = GetComputeString("Sort.title");
      o.mParentWin = this;
      o.theMath = math;
      o.vars = vars;
      o.theCommand = cmd;
      o.theCommandHandler = cmdHandler;
      o.afterDialog = function(editorElement)
      { 
        if (this.Cancel)
          return;
        this.mParentWin.doComputeSort(this.theMath, this.vars, editorElement, this.theCommand, this.theCommandHandler);
      };
      try {
        var theDialog = msiOpenModelessDialog("chrome://prince/content/ComputeVariables.xul", "_blank", "chrome,close,titlebar,resizable,dependent",
                                          editorElement, cmd, cmdHandler, o);
      } catch(e) {AlertWithTitle("Error in computeOverlay.js", "Exception in doComputeSort: [" + e + "]"); return;}

//      var parentWin = msiGetParentWindowForNewDialog(editorElement);
//      parentWin.openDialog("chrome://prince/content/ComputeVariables.xul", "_blank", "chrome,close,titlebar,modal", o);
//        if (o.Cancel)
//          return;
//        vars = runFixup(o.vars);
//      } else {
//        msiComputeLogger.Exception(ex);
} } }

function doComputeCompanionMatrix(math, vars, editorElement, cmd, cmdHandler)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var mathstr = GetFixedMath(math);
  if (!vars)
    vars = "";

  msiComputeLogger.Sent4("companion matrix",mathstr,"specifying",vars);
  try {
    ComputeCursor(editorElement);
    var out = GetCurrentEngine().companionMatrix(mathstr,vars);
    msiComputeLogger.Received(out);
    appendResult(out,"<mo>=</mo>",math, editorElement);
    RestoreCursor(editorElement);
    done = true;
  } catch(ex) {
    RestoreCursor(editorElement);
    if (ex.result == compsample.needvars) {
      var o = new Object();
      o.title = GetComputeString("Companion.title");
      o.mParentWin = this;
      o.theMath = math;
      o.vars = vars;
      o.theCommand = cmd;
      o.theCommandHandler = cmdHandler;
      o.afterDialog = function(editorElement)
      { 
        if (this.Cancel)
          return;
        this.mParentWin.doComputeCompanionMatrix(this.theMath, this.vars, editorElement, this.theCommand, this.theCommandHandler);
      };
      try {
        var theDialog = msiOpenModelessDialog("chrome://prince/content/ComputeVariables.xul", "_blank", "chrome,close,titlebar,resizable,dependent",
                                          editorElement, cmd, cmdHandler, o);
      } catch(e) {AlertWithTitle("Error in computeOverlay.js", "Exception in doComputeCompanionMatrix: [" + e + "]"); return;}

//        var parentWin = msiGetParentWindowForNewDialog(editorElement);
//        parentWin.openDialog("chrome://prince/content/ComputeVariables.xul", "_blank", "chrome,close,titlebar,modal", o);
//        if (o.Cancel)
//          return;
//        vars = runFixup(o.vars);
    } else {
      msiComputeLogger.Exception(ex);
} } }

function doComputeApproxIntegral(math, editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var o = new Object();
  o.intervals = 10;
  o.form = 1;
  var parentWin = msiGetParentWindowForNewDialog(editorElement);
  parentWin.openDialog("chrome://prince/content/ComputeApproxIntegral.xul", "_blank", "chrome,close,titlebar,modal", o);
  if (o.Cancel) {
    return;
  }
  var intervals = GetNumAsMathML(o.intervals);
  var mathstr = GetFixedMath(math);
  msiComputeLogger.Sent4("approximate integral",mathstr,o.form,intervals);

  ComputeCursor(editorElement);
  try {
    var out = GetCurrentEngine().approxIntegral(mathstr,o.form,intervals);
    msiComputeLogger.Received(out);
    appendLabeledResult(out,GetComputeString("ApproximateInt.fmt"),math, editorElement);  // SWP labels with type
  } catch(ex) {
    msiComputeLogger.Exception(ex);
  }
  RestoreCursor(editorElement);
}

function doComputeIterate(editorElement, cmdHandler)
{                                                                                    
  var o = new Object();                                                              
  o.title 			= GetComputeString("Iterate.title");                             
  o.fieldcount      = 3;                                                             
  o.prompt          = new Array(o.fieldcount);                                       
  o.initialvalue    = new Array(o.fieldcount);                                       
  o.prompt[0] 		= GetComputeString("Iterate.fnprompt");                          
  o.initialvalue[0] = GetComputeString("Iterate.fndefault");                         
  o.prompt[1] 		= GetComputeString("Iterate.centerprompt");                      
  o.initialvalue[1] = GetComputeString("Iterate.centerdefault");                     
  o.prompt[2] 		= GetComputeString("Iterate.terms");                             
  o.initialvalue[2] = GetComputeString("Iterate.termsdefault");                      
                                       
  var parentWin = msiGetParentWindowForNewDialog(editorElement);
  try {
    msiOpenModelessDialog("chrome://prince/content/ComputeMathMLArgDialog.xul", "_blank", "chrome,close,titlebar,resizable,dependent",
                                      editorElement, "cmd_compute_Iterate", cmdHandler, o);
  } catch(e) {AlertWithTitle("Error in computeOverlay.js", "Exception in doComputeIterate: [" + e + "]"); return;}
//  parentWin.openDialog("chrome://prince/content/ComputeMathMLArgDialog.xul", "_blank",  
//                    "chrome,close,titlebar,modal", o);
}

function finishComputeIterate(editorElement, o)
{
  if (o.Cancel)                                                                      
    return;                                                                          
                                                                                   
  ComputeCursor(editorElement);
  var fn       = runFixup(o.mathresult[0]);                                                    
  var center   = runFixup(o.mathresult[1]);                                                    
  var order    = runFixup(o.mathresult[2]);                                                    
  msiComputeLogger.Sent4("iterate",fn,center,order);                                 
                                                                                     
  ComputeCursor(editorElement);
  try {                                                                              
    var out = GetCurrentEngine().iterate(fn,center,order);                           
    msiComputeLogger.Received(out);                                                  
    addResult(out, editorElement);
  } catch(ex) {                                                                      
    msiComputeLogger.Exception(ex);                                                  
  }                                                                                  
  RestoreCursor(editorElement);
}                                                                                    

							                                                                                   
function doComputeImplicitDiff(math, editorElement, cmdHandler)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
//  var mathstr = GetFixedMath(math);

  var vars = "";
  var o = new Object();
  o.theMath = math;
//  var parentWin = msiGetParentWindowForNewDialog(editorElement);
//  parentWin.openDialog("chrome://prince/content/ComputeImplicitDiff.xul", "_blank", "chrome,close,titlebar,modal", o);
  try {
    msiOpenModelessDialog("chrome://prince/content/ComputeImplicitDiff.xul", "_blank", "chrome,close,titlebar,dependent",
                                      editorElement, "cmd_MSIComputeImplicitDiff", cmdHandler, o);
  } catch(e) {AlertWithTitle("Error in computeOverlay.js", "Exception in doComputeFillMatrix: [" + e + "]"); return;}
}

function finishComputeImplicitDiff(math, editorElement, o)
{
  if (o.Cancel)
    return;
  var mathstr = GetFixedMath(math);
  msiComputeLogger.Sent4("implicit diff",mathstr,o.thevar,o.about);

  ComputeCursor(editorElement);
  try {
    var out = GetCurrentEngine().implicitDiff(mathstr,o.thevar,o.about);
    msiComputeLogger.Received(out);
    appendLabeledResult(out,GetComputeString("Solution.fmt"),math, editorElement);
  } catch(ex) {
    msiComputeLogger.Exception(ex);
  }
  RestoreCursor(editorElement);
}

function doComputeSolveODE(math,labelID,func,titleID, vars, editorElement, cmd, cmdHandler)
{
  var mathstr = GetFixedMath(math);
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  if (!vars)
    vars = "";
  msiComputeLogger.Sent4(labelID,mathstr,"specifying",vars);
  try {
    ComputeCursor(editorElement);
    var out = func(mathstr,vars);
    msiComputeLogger.Received(out);
    appendLabeledResult(out,GetComputeString(labelID),math, editorElement);
    RestoreCursor(editorElement);
  } catch(ex) {
    RestoreCursor(editorElement);
    if (ex.result == compsample.nosol) {
      appendLabel(GetComputeString("NoSolution"),math, editorElement);
      done = true;
    } else if (ex.result == compsample.needivars) {
      var o = new Object();
      o.title = GetComputeString(titleID);
      o.label = GetComputeString("ODE.label");
      o.mParentWin = this;
      o.theMath = math;
      o.theLabelID = labelID;
      o.theFunc = func;
      o.theTitleID = titleID;
      o.vars = vars;
      o.theCommand = cmd;
      o.theCommandHandler = cmdHandler;
      o.afterDialog = function(editorElement)
      { 
        if (this.Cancel)
          return;
        this.mParentWin.doComputeSolveODE(this.theMath, this.theLabelID, this.theFunc, this.theTitleID, this.vars, editorElement, this.theCommand, this.theCommandHandler);
      };
      try {
        var theDialog = msiOpenModelessDialog("chrome://prince/content/ComputeVariables.xul", "_blank", "chrome,close,titlebar,resizable,dependent",
                                          editorElement, cmd, cmdHandler, o);
      } catch(e) {AlertWithTitle("Error in computeOverlay.js", "Exception in doComputeSolveODE: [" + e + "]"); return;}

//      var parentWin = msiGetParentWindowForNewDialog(editorElement);
//      parentWin.openDialog("chrome://prince/content/ComputeVariables.xul", "_blank", "chrome,close,titlebar,modal", o);
//      if (o.Cancel)
//        return;
//      vars = runFixup(o.vars);
    } else {
      msiComputeLogger.Exception(ex);
//      return;
} } }

function doComputeSolveODESeries(math, editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var o = new Object();
  o.order = "5";  // sticky? But really, should be general content
  o.title = GetComputeString("ODESeries.title");

  var parentWin = msiGetParentWindowForNewDialog(editorElement);
  parentWin.openDialog("chrome://prince/content/ComputePowerSeries.xul", "_blank", "chrome,close,titlebar,modal", o);
  if (o.Cancel)
    return;
  var mathstr = GetFixedMath(math);
  var ord = GetNumAsMathML(o.order);

  ComputeCursor(editorElement);
  msiComputeLogger.Sent4("Solve ODE Power Series ",mathstr,o.thevar + " @ " + o.about,ord);
  try {
    var out = GetCurrentEngine().solveODESeries(mathstr,o.thevar,o.about,ord);
    msiComputeLogger.Received(out);
    appendLabeledResult(out,GetComputeString("ODESeries.fmt"),math, editorElement);
  } catch (e) {
    msiComputeLogger.Exception(e);
  }
  RestoreCursor(editorElement);
}

function doComputePowerSeries(math, editorElement, cmdHandler)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var o = new Object();
  o.title 			= GetComputeString("PowerSeries.title");
  o.fieldcount      = 3;
  o.prompt = new Array(o.fieldcount);
  o.initialvalue = new Array(o.fieldcount);
  o.prompt[0] 		= GetComputeString("PowerSeries.varprompt");
  o.initialvalue[0] 	= GetComputeString("PowerSeries.vardefault");
  o.prompt[1] 		= GetComputeString("PowerSeries.centerprompt");
  o.initialvalue[1] = GetComputeString("PowerSeries.centerdefault");
  o.prompt[2] 		= GetComputeString("PowerSeries.termsprompt");
  o.initialvalue[2] = GetComputeString("PowerSeries.termsdefault");
  o.theMath = math;

  var parentWin = msiGetParentWindowForNewDialog(editorElement);
  try {
    msiOpenModelessDialog("chrome://prince/content/ComputePowerSeriesArgDialog.xul", "_blank", "chrome,close,titlebar,resizable,dependent",
                                      editorElement, "cmd_MSIComputePowerSeries", cmdHandler, o);
  } catch(e) {AlertWithTitle("Error in computeOverlay.js", "Exception in doComputePowerSeries: [" + e + "]"); return;}

//  parentWin.openDialog("chrome://prince/content/ComputeMathMLArgDialog.xul", "_blank", 
//                    "chrome,close,titlebar,modal", o);
}

function finishComputePowerSeries(editorElement, o)
{
  if (o.Cancel)
    return;
  var mathstr = GetMathAsString(GetRHS(o.theMath));

  ComputeCursor(editorElement);
  var variable = runFixup(o.mathresult[0]);
  var center   = runFixup(o.mathresult[1]);
  var order    = runFixup(o.mathresult[2]);
  msiComputeLogger.Sent4("Power Series ",mathstr,variable + " @ " + center, order);
  try {
    var out = GetCurrentEngine().powerSeries(mathstr, variable, center, order);
    msiComputeLogger.Received(out);
    appendLabeledResult(out,GetComputeString("Series.fmt"),o.theMath, editorElement);
  } catch (e) {
    msiComputeLogger.Exception(e);
  }
  RestoreCursor(editorElement);
}

// vector calculus
function doComputeWronskian(math, vars, editorElement, cmd, cmdHandler)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var mathstr = GetFixedMath(math);
  if (!vars)
    vars = "";
  msiComputeLogger.Sent4("Wronskian",mathstr,"specifying",vars);
  try {
    ComputeCursor(editorElement);
    var out = GetCurrentEngine().wronskian(mathstr,vars);
    msiComputeLogger.Received(out);
    appendLabeledResult(out,GetComputeString("Wronskian.fmt"),math, editorElement);
    RestoreCursor(editorElement);
  } catch(ex) {
    RestoreCursor(editorElement);
    if (ex.result == compsample.needvars) {
      var o = new Object();
      o.title = GetComputeString("Wronskian.title");
      o.mParentWin = this;
      o.theMath = math;
      o.vars = vars;
      o.theCommand = cmd;
      o.theCommandHandler = cmdHandler;
      o.afterDialog = function(editorElement)
      { 
        if (this.Cancel)
          return;
        this.mParentWin.doComputeWronskian(this.theMath, this.vars, editorElement, this.theCommand, this.theCommandHandler);
      };
      try {
        var theDialog = msiOpenModelessDialog("chrome://prince/content/ComputeVariables.xul", "_blank", "chrome,close,titlebar,resizable,dependent",
                                          editorElement, cmd, cmdHandler, o);
      } catch(e) {AlertWithTitle("Error in computeOverlay.js", "Exception in doComputeWronskian: [" + e + "]"); return;}

//      var parentWin = msiGetParentWindowForNewDialog(editorElement);
//      parentWin.openDialog("chrome://prince/content/ComputeVariables.xul", "_blank", "chrome,close,titlebar,modal", o);
//      if (o.Cancel)
//        return;
//      vars = runFixup(o.vars);
    } else {
      msiComputeLogger.Exception(ex);
} } }

// matrix commands
function doComputeCharPoly(math, vars, editorElement, cmd, cmdHandler)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var mathstr = GetFixedMath(math);
  if (!vars)
    vars = "";
  msiComputeLogger.Sent4("characteristic polynomial",mathstr,"specifying",vars);
  try {
    ComputeCursor(editorElement);
    var out = GetCurrentEngine().characteristicPolynomial(mathstr,vars);
    msiComputeLogger.Received(out);
    appendLabeledResult(out,GetComputeString("CharPolynomial.fmt"),math, editorElement);
    RestoreCursor(editorElement);
  } catch(ex) {
    RestoreCursor(editorElement);
    if (ex.result == compsample.needvars) {
      var o = new Object();
      o.title = GetComputeString("Charpoly.title");
      o.mParentWin = this;
      o.theMath = math;
      o.vars = vars;
      o.theCommand = cmd;
      o.theCommandHandler = cmdHandler;
      o.afterDialog = function(editorElement)
      { 
        if (this.Cancel)
          return;
        this.mParentWin.doComputeCharPoly(this.theMath, this.vars, editorElement, this.theCommand, this.theCommandHandler);
      };
      try {
        var theDialog = msiOpenModelessDialog("chrome://prince/content/ComputeVariables.xul", "_blank", "chrome,close,titlebar,resizable,dependent",
                                          editorElement, cmd, cmdHandler, o);
      } catch(e) {AlertWithTitle("Error in computeOverlay.js", "Exception in doComputeCharPoly: [" + e + "]"); return;}

//      var parentWin = msiGetParentWindowForNewDialog(editorElement);
//      parentWin.openDialog("chrome://prince/content/ComputeVariables.xul", "_blank", "chrome,close,titlebar,modal", o);
//      if (o.Cancel)
//        return;
//      vars = runFixup(o.vars);
    } else {
      msiComputeLogger.Exception(ex);
} } }

function doComputeFillMatrix(editorElement, cmdHandler)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var o = new Object();
  o.rows = "3";  // sticky?
  o.cols = "3";
  var parentWin = msiGetParentWindowForNewDialog(editorElement);
  try {
    msiOpenModelessDialog("chrome://prince/content/ComputeFillMatrix.xul", "_blank", "chrome,close,titlebar,dependent",
                                      editorElement, "cmd_MSIComputeFillMatrix", cmdHandler, o);
  } catch(e) {AlertWithTitle("Error in computeOverlay.js", "Exception in doComputeFillMatrix: [" + e + "]"); return;}
//  parentWin.openDialog("chrome://prince/content/ComputeFillMatrix.xul", "_blank", "chrome,close,titlebar,modal", o);
//  if (o.Cancel)
//    return;

//  ComputeCursor(editorElement);
//  var mRows = GetNumAsMathML(o.rows);
//  var mCols = GetNumAsMathML(o.cols);
//  var mExpr  = o.expr;
//  msiComputeLogger.Sent4("Fill matrix",mRows+" by "+mCols, mExpr,o.type);
//  try {
//    var out = GetCurrentEngine().matrixFill(o.type,mRows,mCols,mExpr);
//    msiComputeLogger.Received(out);
//    addResult(out, editorElement);
//  } catch (e) {
//    msiComputeLogger.Exception(e);
//  }
//  RestoreCursor(editorElement);
}

function doComputeMap(math, editorElement, cmd, cmdHandler)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var o = new Object();
  o.title = GetComputeString("Map.title");
  o.label = GetComputeString("Map.label");
  o.mParentWin = this;
  o.theMath = math;
  o.afterDialog = function(editorElement)
  { 
    if (this.Cancel)
      return;
    this.mParentWin.finishComputeMap(editorElement, this);
  };
  try {
    var theDialog = msiOpenModelessDialog("chrome://prince/content/ComputeVariables.xul", "_blank", "chrome,close,titlebar,resizable,dependent",
                                      editorElement, cmd, cmdHandler, o);
  } catch(e) {AlertWithTitle("Error in computeOverlay.js", "Exception in doComputeMap: [" + e + "]"); return;}

//  var parentWin = msiGetParentWindowForNewDialog(editorElement);
//  parentWin.openDialog("chrome://prince/content/ComputeVariables.xul", "_blank", "chrome,close,titlebar,modal", o);
}

function finishComputeMap(editorElement, o)
{
  if (o.Cancel)
    return;
  var mathstr = GetFixedMath(GetRHS(o.theMath));
  msiComputeLogger.Sent4("Map ",mathstr," @ ",o.vars);

  ComputeCursor(editorElement);
  try {
    var out = GetCurrentEngine().map(mathstr,o.vars);
    msiComputeLogger.Received(out);
    appendLabeledResult(out, GetComputeString("Map.fmt"), o.theMath, editorElement);
  } catch (e) {
    msiComputeLogger.Exception(e);
  }
  RestoreCursor(editorElement);
}

function doComputeMinPoly(math, vars, editorElement, cmd, cmdHandler)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var mathstr = GetFixedMath(math);
  if (!vars)
    vars = "";
  msiComputeLogger.Sent4("minimal polynomial",mathstr,"specifying",vars);
  try {
    ComputeCursor(editorElement);
    var out = GetCurrentEngine().minimalPolynomial(mathstr,vars);
    msiComputeLogger.Received(out);
    label = " Minimal Polynomial: ";
    appendLabeledResult(out,GetComputeString("MinPolynomial.fmt"),math, editorElement);
    RestoreCursor(editorElement);
  } catch(ex) {
    RestoreCursor(editorElement);
    if (ex.result == compsample.needvars) {
      var o = new Object();
      o.title = GetComputeString("Minpoly.title");
      o.mParentWin = this;
      o.theMath = math;
      o.vars = vars;
      o.theCommand = cmd;
      o.theCommandHandler = cmdHandler;
      o.afterDialog = function(editorElement)
      { 
        if (this.Cancel)
          return;
        this.mParentWin.doComputeMinPoly(this.theMath, this.vars, editorElement, this.theCommand, this.theCommandHandler);
      };
      try {
        var theDialog = msiOpenModelessDialog("chrome://prince/content/ComputeVariables.xul", "_blank", "chrome,close,titlebar,resizable,dependent",
                                          editorElement, cmd, cmdHandler, o);
      } catch(e) {AlertWithTitle("Error in computeOverlay.js", "Exception in doComputeMinPoly: [" + e + "]"); return;}

//      var parentWin = msiGetParentWindowForNewDialog(editorElement);
//      parentWin.openDialog("chrome://prince/content/ComputeVariables.xul", "_blank", "chrome,close,titlebar,modal", o);
//      if (o.Cancel)
//        return;
//      vars = runFixup(o.vars);
    } else {
      msiComputeLogger.Exception(ex);
} } }

function doComputeRandomMatrix(editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var o = new Object();
  o.rows = "3";  // sticky?
  o.cols = "3";
  o.min  = "-9";
  o.max  = "9";
  var parentWin = msiGetParentWindowForNewDialog(editorElement);
  parentWin.openDialog("chrome://prince/content/ComputeRandomMatrix.xul", "_blank", "chrome,close,titlebar,modal", o);
  if (o.Cancel)
    return;
  var mRows = GetNumAsMathML(o.rows);
  var mCols = GetNumAsMathML(o.cols);
  var mMin  = GetNumAsMathML(o.min);
  var mMax  = GetNumAsMathML(o.max);
  msiComputeLogger.Sent4("random matrix",mRows+" by "+mCols, mMin+ " to "+mMax,o.type);

  ComputeCursor(editorElement);
  try {
    var out = GetCurrentEngine().matrixRandom(o.type,mRows,mCols,mMin,mMax);
    msiComputeLogger.Received(out);
    addResult(out, editorElement);
  } catch (e) {
    msiComputeLogger.Exception(e);
  }
  RestoreCursor(editorElement);
}

function doComputeReshape(math, editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var o = new Object();
  o.ncols = "2";  // sticky?
  var parentWin = msiGetParentWindowForNewDialog(editorElement);
  parentWin.openDialog("chrome://prince/content/ComputeReshape.xul", "_blank", "chrome,close,titlebar,modal", o);
  if (o.Cancel)
    return;
  var mCols = GetNumAsMathML(o.ncols);
  var mathstr = GetFixedMath(GetRHS(math));
  msiComputeLogger.Sent4("reshape",mathstr,"with cols",mCols);

  ComputeCursor(editorElement);
  try {
    var out = GetCurrentEngine().matrixReshape(mathstr,mCols);
    msiComputeLogger.Received(out);
    appendLabeledResult(out,GetComputeString("Reshape.fmt"),math, editorElement);
  } catch (e) {
    msiComputeLogger.Exception(e);
  }
  RestoreCursor(editorElement);
}

function doComputeFitCurve(math, editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var o = new Object();
  o.code   = 1;
  o.column = 1;
  o.degree = 2;
  var parentWin = msiGetParentWindowForNewDialog(editorElement);
  parentWin.openDialog("chrome://prince/content/ComputeFitCurve.xul", "_blank", "chrome,close,titlebar,modal", o);
  if (o.Cancel)
    return;
  var mathstr = GetFixedMath(math);
  msiComputeLogger.Sent4("Fit Curve @ ",mathstr,o.code,o.column+"/"+o.degree);
  var label;
  if (o.code == 2)
    label = GetComputeString("FitCurvePolynomial.fmt");
  else
    label = GetComputeString("FitCurveRegression.fmt");

  ComputeCursor(editorElement);
  try {
    var out = GetCurrentEngine().fitCurve(mathstr,o.code,o.column,o.degree);
    msiComputeLogger.Received(out);
    appendLabeledResult(out,label,math, editorElement);
  } catch(ex) {
    msiComputeLogger.Exception(ex);
  }
  RestoreCursor(editorElement);
}

function doComputeRandomNumbers(editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var o = new Object();
  o.tally      = "10";  // sticky?
  o.dist       = 0;
  o.param1     = "";
  o.param2     = "";
  var parentWin = msiGetParentWindowForNewDialog(editorElement);
  parentWin.openDialog("chrome://prince/content/ComputeRandomNumbers.xul", "_blank", "chrome,close,titlebar,modal", o);
  if (o.Cancel)
    return;
  var mTally    = GetNumAsMathML(o.tally);
  var mParam1   = GetNumAsMathML(o.param1);
  var mParam2   = GetNumAsMathML(o.param2);
  msiComputeLogger.Sent4("random numbers",o.dist,mTally,mParam1+ " to "+mParam2);

  ComputeCursor(editorElement);
  try {
    var out = GetCurrentEngine().randomNumbers(o.dist,mTally,mParam1,mParam2);
    msiComputeLogger.Received(out);
    addResult(out, editorElement);
  } catch (e) {
    msiComputeLogger.Exception(e);
  }
  RestoreCursor(editorElement);
}

function doComputeMoment(math, editorElement, cmdHandler)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var o = new Object();
//  o.title = GetComputeString("Moment.title");
//  o.label = GetComputeString("Moment.label");
//  o.remark = GetComputeString("Moment.remark");
  o.theMath = math;
  var parentWin = msiGetParentWindowForNewDialog(editorElement);
  try {
    msiOpenModelessDialog("chrome://prince/content/ComputeMoment.xul", "_blank", "chrome,close,titlebar,resizable,dependent",
                                      editorElement, "cmd_MSIComputeMoment", cmdHandler, o);
  } catch(e) {AlertWithTitle("Error in computeOverlay.js", "Exception in doComputeMoment: [" + e + "]"); return;}

//  parentWin.openDialog("chrome://prince/content/ComputeMoment.xul", "_blank", "chrome,close,titlebar,modal", o);
}

function finishComputeMoment(editorElement, o)
{
  if (o.Cancel)
    return;
  var num = o.thevar;
  var origin = o.about;
  var meanisorigin = o.meanisorigin;
  var mathstr = GetFixedMath(o.theMath);
  msiComputeLogger.Sent4("Moment ",mathstr,origin + " @ ",num+" / "+meanisorigin);

  ComputeCursor(editorElement);
  try {
    var out = GetCurrentEngine().moment(mathstr,num,origin,meanisorigin);
    msiComputeLogger.Received(out);
    appendLabeledResult(out,GetComputeString("Moment.fmt"),o.theMath, editorElement);
  } catch(ex) {
    msiComputeLogger.Exception(ex);
  }
  RestoreCursor(editorElement);
}

function doComputeQuantile(math, editorElement, cmd, cmdHandler)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var o = new Object();
  o.title = GetComputeString("Quantile.title");
  o.label = GetComputeString("Quantile.label");
  o.mParentWin = this;
  o.theMath = math;
  o.afterDialog = function(editorElement)
  { 
    if (this.Cancel)
      return;
    this.mParentWin.finishComputeQuantile(editorElement, this);
  };
  try {
    var theDialog = msiOpenModelessDialog("chrome://prince/content/ComputeVariables.xul", "_blank", "chrome,close,titlebar,resizable,dependent",
                                      editorElement, cmd, cmdHandler, o);
  } catch(e) {AlertWithTitle("Error in computeOverlay.js", "Exception in doComputeQuantile: [" + e + "]"); return;}

//  var parentWin = msiGetParentWindowForNewDialog(editorElement);
//  parentWin.openDialog("chrome://prince/content/ComputeVariables.xul", "_blank", "chrome,close,titlebar,modal", o);
}

function finishComputeQuantile(editorElement, o)
{
  if (o.Cancel)
    return;
  var quantile = o.vars;
  var mathstr = GetFixedMath(o.theMath);
  msiComputeLogger.Sent4("Quantile", mathstr, "specifying", quantile);

  ComputeCursor(editorElement);
  try {
    var out = GetCurrentEngine().quantile(mathstr,quantile);
    msiComputeLogger.Received(out);
    appendLabeledResult(out,GetComputeString("Quantile.fmt"), o.theMath, editorElement);
  } catch(ex) {
    msiComputeLogger.Exception(ex);
  }
  RestoreCursor(editorElement);
}

function doComputeDefine(math, editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var mathstr = GetFixedMath(math);

  var subscript = "";
  var done = false;
  while (!done) {
    msiComputeLogger.Sent4("define",mathstr,"specifying",subscript);
    try {
      ComputeCursor(editorElement);
      var out = GetCurrentEngine().define(mathstr,subscript);
      msiComputeLogger.Received(out);
      // there is no result
      done = true;
    } catch(ex) {
      RestoreCursor(editorElement);
      if (ex.result == compsample.needsubinterp) {
        var o = new Object();
        var parentWin = msiGetParentWindowForNewDialog(editorElement);
        parentWin.openDialog("chrome://prince/content/ComputeInterpretSubscript.xul", "_blank", "chrome,close,titlebar,modal", o);
        if (o.Cancel) {
          return;
        }
        subscript = o.subscript;
      } else {
        msiComputeLogger.Exception(ex);
        return;
  } } }

  RestoreCursor(editorElement);
}

function doComputeUndefine(math)
{
  ComputeCursor();
  var mathstr = GetFixedMath(math);
  msiComputeLogger.Sent("undefine",mathstr);
  try {
    var out = GetCurrentEngine().undefine(mathstr);
    msiComputeLogger.Received(out);
    // there is no result
  } catch (e) {
    msiComputeLogger.Exception(e);
  }
  RestoreCursor();
}

function doComputeShowDefs(editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  msiComputeLogger.Sent("show definitions","");
  var out = GetCurrentEngine().getDefinitions();
  msiComputeLogger.Received(out);

  var o = new Object();
  o.val = out;
  var parentWin = msiGetParentWindowForNewDialog(editorElement);
  parentWin.openDialog("chrome://prince/content/ComputeShowDefs.xul", "_blank", "chrome,close,titlebar,modal", o);
}

function doComputeClearDefs()
{
  msiComputeLogger.Sent("clear definitions","");
  GetCurrentEngine().clearDefinitions();
}

function doComputeSetBasisVars(editorElement, cmd)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var compsample = GetCurrentEngine();

  msiComputeLogger.Sent("vector basis","queried");

  var o = new Object();
  o.vars = runFixup(compsample.getVectorBasis());
  o.mParentWin = this;
  o.theEngine = compsample;
  o.afterDialog = function(editorElement)
  { 
    if (this.Cancel)
      return;
    this.mParentWin.finishComputeSetBasisVars(this.vars, this.theEngine);
  };

  try {
    var theDialog = msiOpenModelessDialog("chrome://prince/content/ComputeVarList.xul", "_blank", "chrome,close,titlebar,resizable,dependent",
                                      editorElement, cmd, null, o);
  } catch(e) {AlertWithTitle("Error in computeOverlay.js", "Exception in doComputeMinPoly: [" + e + "]"); return;}

//  var parentWin = msiGetParentWindowForNewDialog(editorElement);
//  parentWin.openDialog("chrome://prince/content/ComputeVarList.xul", "_blank", "chrome,close,titlebar,modal", o);
//  if (o.Cancel) {
//    return;
//  }
}

function finishComputeSetBasisVars(vars, compsample)
{
  compsample.setVectorBasis(vars);
  msiComputeLogger.Sent("new vector basis",vars);
}

function doComputeUserSettings()
{
  var compsample = GetCurrentEngine();

  msiComputeLogger.Sent("user settings","");

  var o = new Object();

  o.mfenced      = compsample.getUserPref(compsample.use_mfenced);
  o.digits       = compsample.getUserPref(compsample.Sig_digits_rendered);
  o.lower        = compsample.getUserPref(compsample.SciNote_lower_thresh);
  o.upper        = compsample.getUserPref(compsample.SciNote_upper_thresh);
  o.trigargs     = compsample.getUserPref(compsample.Parens_on_trigargs);
  o.imaginaryi   = compsample.getUserPref(compsample.Output_imaginaryi);
  o.diffD        = compsample.getUserPref(compsample.Output_diffD_uppercase);
  o.diffd        = compsample.getUserPref(compsample.Output_diffd_lowercase);
  o.expe         = compsample.getUserPref(compsample.Output_Euler_e);
  o.matrix_delim = compsample.getUserPref(compsample.Default_matrix_delims);
  o.usearc       = compsample.getUserPref(compsample.Output_InvTrigFuncs_1);
  o.mixednum     = compsample.getUserPref(compsample.Output_Mixed_Numbers);
  o.derivformat  = compsample.getUserPref(compsample.Default_derivative_format);
  o.primesasn    = compsample.getUserPref(compsample.Primes_as_n_thresh);
  o.primederiv   = compsample.getUserPref(compsample.Prime_means_derivative);

  o.loge         = compsample.getUserPref(compsample.log_is_base_e);
  o.dotderiv     = compsample.getUserPref(compsample.Dot_derivative);
  o.barconj      = compsample.getUserPref(compsample.Overbar_conjugate);
  o.i_imaginary  = compsample.getUserPref(compsample.Input_i_Imaginary);
  o.j_imaginary  = compsample.getUserPref(compsample.Input_j_Imaginary);
  o.e_exp        = compsample.getUserPref(compsample.Input_e_Euler);
  
  window.openDialog("chrome://prince/content/ComputeUserSettings.xul", "_blank", "chrome,close,titlebar,modal", o);
  if (o.Cancel)
    return;

  compsample.setUserPref(compsample.use_mfenced,               o.mfenced);
  compsample.setUserPref(compsample.Sig_digits_rendered,       o.digits);
  compsample.setUserPref(compsample.SciNote_lower_thresh,      o.lower);
  compsample.setUserPref(compsample.SciNote_upper_thresh,      o.upper);
  compsample.setUserPref(compsample.Parens_on_trigargs,        o.trigargs);
  compsample.setUserPref(compsample.Output_imaginaryi,         o.imaginaryi);
  compsample.setUserPref(compsample.Output_diffD_uppercase,    o.diffD);
  compsample.setUserPref(compsample.Output_diffd_lowercase,    o.diffd);
  compsample.setUserPref(compsample.Output_Euler_e,            o.expe);
  compsample.setUserPref(compsample.Default_matrix_delims,     o.matrix_delim);
  compsample.setUserPref(compsample.Output_InvTrigFuncs_1,     o.usearc);
  compsample.setUserPref(compsample.Output_Mixed_Numbers,      o.mixednum);
  compsample.setUserPref(compsample.Default_derivative_format, o.derivformat);
  compsample.setUserPref(compsample.Primes_as_n_thresh,        o.primesasn);
  compsample.setUserPref(compsample.Prime_means_derivative,    o.primederiv);

  compsample.setUserPref(compsample.log_is_base_e,             o.loge);
  compsample.setUserPref(compsample.Dot_derivative,            o.dotderiv);
  compsample.setUserPref(compsample.Overbar_conjugate,         o.barconj);
  compsample.setUserPref(compsample.Input_i_Imaginary,         o.i_imaginary);
  compsample.setUserPref(compsample.Input_j_Imaginary,         o.j_imaginary);
  compsample.setUserPref(compsample.Input_e_Euler,             o.e_exp);
}

function doComputeSettings()
{
  var compsample = GetCurrentEngine();

  msiComputeLogger.Sent("settings","");

  var o = new Object();
  o.digits      = compsample.getEngineAttr(compsample.Digits);
  o.degree      = compsample.getEngineAttr(compsample.MaxDegree);
  o.principal   = compsample.getEngineAttr(compsample.PvalOnly) == 0 ? false : true;
  o.special     = compsample.getEngineAttr(compsample.IgnoreSCases) == 0 ? false : true;
  o.logSent     = msiComputeLogger.logMMLSent;
  o.logReceived = msiComputeLogger.logMMLReceived;
  o.engSent     = msiComputeLogger.logEngSent;
  o.engReceived = msiComputeLogger.logEngReceived;
  window.openDialog("chrome://prince/content/ComputeSettings.xul", "_blank", "chrome,close,titlebar,modal", o);
  if (o.Cancel)
    return;

  compsample.setEngineAttr(compsample.Digits,o.digits);
  compsample.setEngineAttr(compsample.MaxDegree,o.degree);
  compsample.setEngineAttr(compsample.PvalOnly,o.principal ? 1 : 0);
  compsample.setEngineAttr(compsample.IgnoreSCases,o.special ? 1 : 0);
  msiComputeLogger.LogMMLSent(o.logSent);
  msiComputeLogger.LogMMLReceived(o.logReceived);
  msiComputeLogger.LogEngSent(o.engSent);
  msiComputeLogger.LogEngReceived(o.engReceived);
}

function doComputeSwitchEngines()
{
  var compsample = GetCurrentEngine();

  var o = new Object();
  o.engine      = compengine;
  window.openDialog("chrome://prince/content/ComputeSwitchEngines.xul", "_blank", "chrome,close,titlebar,modal", o);
  if (o.Cancel)
    return;

  compengine = o.engine;
  if (compengine == 1)
    compsample.startup("mplInstall.gmr");
  else
    compsample.startup("mupInstall.gmr");
  msiComputeLogger.Sent("Switching engine to",compengine);
}

function doComputePassthru(editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var str = "";
  var element = null;
  try
  {
    var selection = msiGetEditor(editorElement).selection;
    if (!selection) {
	    dump("no selection!\n");
      return;
    }
    element = findtagparent(selection.focusNode,"code");
    if (!element) {
	    dump("not in code tag!\n");
      return;
    }

    var text = element.firstChild;
    if (text.nodeType != Node.TEXT_NODE) {
      dump("bad node structure.\n");
      return;
    }

    str = WrapInMtext(text.nodeValue);
    msiComputeLogger.Sent("passthru",str);
  }
  catch(exc) {AlertWithTitle("Error in computeOverlay.js", "Exception in doComputePassthru; exception is [" + exc + "]."); return;}

  ComputeCursor(editorElement);
  var compsample = GetCurrentEngine();
  var out;                                                 
  try {
    out = GetCurrentEngine().perform(str,compsample.PassThru);
    msiComputeLogger.Received(out);
  } catch (e) {
    msiComputeLogger.Exception(e);
  }

  var child = element;
  var node = child.parentNode;
  while (node) {
    if (node.localName == "body")
      break;
    else {
      child = node;
      node = child.parentNode;
    }
  }
 
  if (out) {
    var idx;
    for (idx = 0; idx < node.childNodes.length; idx++) {
      if (child == node.childNodes[idx])
        break;
    }
    appendTaggedResult(out,GetComputeString("Passthru.fmt"),node,idx+1, editorElement);
  }
  RestoreCursor(editorElement);
}


// "math" is the expression to plot
// "type" is the type from the menu. 
function doComputePlot(math, dimension, type, animate, editorElement)
{
  insertNewGraph(math, dimension, type, animate, editorElement);
}

function doEditPlot()
{
  graphObjectClickEvent();
}

// form a single run of math and put caret on end
function coalescemath(editorElement) {
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);
  var selection = editor.selection;
  if (selection)
  {
    var f = selection.focusNode;
    var element = findmathparent(f);
    if (!element) {
      f = f.previousSibling;
      element = findmathparent(f);
      if (!element) {
	      dump("focus not in math!\n");
        return;
      }
    }
    var last = node_before(element);
    if (!last || last.localName != "math") {
      dump("previous is not math!\n");
      return;
    }
    var ch = element.firstChild;  // move children to previous math element
    var nextch;
    while (ch) {
      nextch = ch.nextSibling;
      last.appendChild(element.removeChild(ch));
      ch = nextch;
    }
    element.parentNode.removeChild(element);  // now empty

    editor.setCaretAfterElement(last_child(last));
} }

function findtagparent(node,tag) {
  if (!node || node.localName == tag)
    return node;
  else
    return findtagparent(node.parentNode,tag);
}

function findmathparent(node) {
  return findtagparent(node,"math");
  // really, should check namespace, too
}
