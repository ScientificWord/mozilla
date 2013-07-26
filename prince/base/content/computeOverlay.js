// Copyright (c) 2004 MacKichan Software, Inc.  All Rights Reserved.
// Copyright (c) 2006 MacKichan Software, Inc.  All Rights Reserved.
# include productname.inc
# ifndef PROD_SW
Components.utils.import("resource://app/modules/computelogger.jsm");
Components.utils.import("resource://app/modules/os.jsm");
Components.utils.import("resource://app/modules/graphicsConverter.jsm");

//-----------------------------------------------------------------------------------
var msiEvaluateCommand = {
  isCommandEnabled: function(aCommand, editorElement) {
    var theEditorElement = msiGetActiveEditorElement();
    return (theEditorElement && msiGetEditor(theEditorElement) && msiIsDocumentEditable(theEditorElement) && msiIsEditingRenderedHTML(theEditorElement) && (isInMath(theEditorElement) || aCommand == "cmd_MSIComputeFillMatrix" || aCommand == "cmd_MSIComputeRandomMatrix" || aCommand == "cmd_MSIComputeRandomNumbers" || (aCommand == "cmd_MSIComputePassthru" && msiGetEditor(theEditorElement).selection && (!msiGetEditor(theEditorElement).selection.isCollapsed))));
  },

  getCommandStateParams: function(aCommand, aParams, editorElement) {},
  doCommandParams: function(aCommand, aParams, editorElement) {},

  doCommand: function(aCommand, editorElement) {
    var theEditorElement = msiGetActiveEditorElement();
    doComputeCommand(aCommand, theEditorElement, this);
  }
};

var msiComputeStopCommand = {
  isCommandEnabled: function(aCommand, editorElement) {
    var theEditorElement = msiGetActiveEditorElement();
    return (theEditorElement && msiGetEditor(theEditorElement) && msiIsDocumentEditable(theEditorElement) && msiIsEditingRenderedHTML(theEditorElement) && (isInMath(theEditorElement) || aCommand == "cmd_MSIComputeFillMatrix" || aCommand == "cmd_MSIComputeRandomMatrix" || aCommand == "cmd_MSIComputeRandomNumbers" || (aCommand == "cmd_MSIComputePassthru" && msiGetEditor(theEditorElement).selection && (!msiGetEditor(theEditorElement).selection.isCollapsed))));
  },
  getCommandStateParams: function(aCommand, aParams, editorElement) {},
  doCommandParams: function(aCommand, aParams, editorElement) {},

  doCommand: function(aCommand, editorElement) {
    var theEditorElement = msiGetActiveEditorElement();
    doComputeStopCommand();
  }

  
};

var msiEvaluateCommandKeyboard = {
  isCommandEnabled: function(aCommand, editorElement) {
    var theEditorElement = msiGetActiveEditorElement();
    return (theEditorElement && msiGetEditor(theEditorElement) && msiIsDocumentEditable(theEditorElement) && msiIsEditingRenderedHTML(theEditorElement) && (isInMath(theEditorElement) || aCommand == "cmd_MSIComputeFillMatrix" || aCommand == "cmd_MSIComputeRandomMatrix" || aCommand == "cmd_MSIComputeRandomNumbers" || (aCommand == "cmd_MSIComputePassthru" && msiGetEditor(theEditorElement).selection && (!msiGetEditor(theEditorElement).selection.isCollapsed))));
  },

  getCommandStateParams: function(aCommand, aParams, editorElement) {},
  doCommandParams: function(aCommand, aParams, editorElement) {},

  doCommand: function(aCommand, editorElement) {
    var theEditorElement = msiGetActiveEditorElement();
    doComputeCommand(aCommand, theEditorElement, this, false);
  }
};

var msiDefineCommand = {
  isCommandEnabled: function(aCommand, dummy) {
    var editorElement = msiGetActiveEditorElement();
    return (editorElement && msiGetEditor(editorElement) && msiIsDocumentEditable(editorElement) && msiIsEditingRenderedHTML(editorElement) && (isInMath(editorElement) || aCommand == "cmd_MSIComputeShowDefs" || aCommand == "cmd_MSIComputeMapMuPADName" ||
    //aCommand == "cmd_MSI/ComputeUserSettings" ||
    aCommand == "cmd_MSIComputeClearDefs" ||
    //aCommand == "cmd_MSIComputeSettings" ||
    aCommand == "cmd_MSIComputeSwitchEngines"));

  },

  getCommandStateParams: function(aCommand, aParams, editorElement) {},
  doCommandParams: function(aCommand, aParams, editorElement) {},

  doCommand: function(aCommand, editorElement) {
    var theEditorElement = msiGetActiveEditorElement();
    doComputeCommand(aCommand, theEditorElement);
  }
};

function SetupMSIComputeMenuCommands() {
  var commandTable = GetComposerCommandTable();
  //dump("Registering msi compute menu commands\n");
  // note hokey use of shared command
  doSetupMSIComputeMenuCommands(commandTable);
}

//var msiPlotInsertionTimerBase = 
//{
//  mGraphNode : null,
////  mIncrement : 300,
////  totalTime : 600,
//  mTimer : null,
////  mMSecsUsed : 0,
//  ourWindow : null,
//  notify : function(aTimer)
//  { 
//    if (aTimer)
//      this.mTimer = aTimer;
////    mMSecsUsed += this.increment;
////    if (mmSecsUsed >= this.totalTime)
////    {
//      if (this.mTimer)
//        this.mTimer.cancel();
//      if (this.mGraphNode && this.ourWindow)
//        this.ourWindow.ensureVCamPreinitForPlot(this.mGraphNode, this.mEditorElement);
////    }
//  }
//};
//
//function msiPlotInsertionTimer(graphNode, editorElement, hostWindow, nmSecs, incr)
//{
//  this.mEditorElement = editorElement;
//  this.totalTime = nmSecs;
//  this.increment = incr;
//  this.mGraphNode = graphNode;
//  this.ourWindow = hostWindow;
//}
//
//msiPlotInsertionTimer.prototype = msiPlotInsertionTimerBase;
//
////This is an nsIContentFilter - we're only interested for now in identifying <graph> objects.
//var msiEditorInsertionListenerBase = 
//{
//  timersToAdd : [],
//  dataSize : {textChars : 0, nodes : 0},
//  hostWindow : null,
//  notifyOfInsertion : function(mimeType, contentSourceURL, sourceDocument, willDeleteSelection,
//                                    docFragment, contentStartNode, contentStartOffset,
//                                    contentEndNode, contentEndOffset, insertionPointNode,
//                                    insertionPointOffset, continueWithInsertion)
//  {
//    var currNode;
//    currNode = contentStartNode.value;
//    var currOffset = contentStartOffset.value;
//    var startOffsets = [currOffset];
//    var endOffsets = [contentEndOffset.value];
////    var startNodes = [{node : contentStartNode, offset : contentStartOffset}];
////    var endNodes = [{node : contentEndNode, offset : contentEndOffset}];
//    while (currNode && currNode != docFragment.value)
//    {
//      currOffset = msiNavigationUtils.offsetInParent(currNode);
//      currNode = currNode.parentNode;
//      startOffsets.push( currOffset );
////      startNodes.push( {node : parent, offset : currOffset} );
//    }
//    currNode = contentEndNode.value;
//    while (currNode && currNode != docFragment.value)
//    {
//      currOffset = msiNavigationUtils.offsetInParent(currNode);
//      currNode = currNode.parentNode;
//      endOffsets.push( currOffset );
////      startNodes.push( {node : parent, offset : currOffset} );
//    }
//    this.checkForPlots(docFragment.value, startOffsets, endOffsets);
//    continueWithInsertion.value = true;
//  },
//
//  checkForPlots : function(aNode, startOffsets, endOffsets)
//  {
//    var aStart = startOffsets.length ? startOffsets.pop() : 0;
//    var anEnd = endOffsets.length ? endOffsets.pop() : -1;
//    if (msiNavigationUtils.isTextNode(aNode))
//    {
//      if (anEnd < 0)
//        anEnd = aNode.data.length;
//      this.dataSize.textChars += anEnd - aStart;
//      return;
//    }
//    ++this.dataSize.nodes;
//    if (anEnd < 0)
//      anEnd = aNode.childNodes.length - 1;
//    else if (!endOffsets.length)  //this is the last one - the endOffset handed in - it will be one too many in this case
//      --anEnd;
//    if (msiGetBaseNodeName(aNode) == "graph")
//    {
//      this.timersToAdd.push(aNode);
//    }
//    var ourStarts = [];
//    var ourEnds = [];
//    for (var jj = aStart; jj <= anEnd; ++jj)
//    {
//      if (jj == aStart)
//        ourStarts = startOffsets;
//      else
//        ourStarts = [];
//      if (jj == anEnd)
//        ourEnds = endOffsets;
//      else
//        ourEnds = [];
//      this.checkForPlots(aNode.childNodes[jj], ourStarts, ourEnds);
//    }
//  },
//  addPlotInsertionTimers : function()
//  {
//    var nmSecs = 200;
//    nmSecs += this.dataSize.nodes * 10;
//    nmSecs += this.dataSize.textChars/2;
//    var plotInsertTimer, theTimer;
//    for (var ix = 0; ix < this.timersToAdd.length; ++ix)
//    {
//      plotInsertionTimer = new msiPlotInsertionTimer(this.timersToAdd[ix], this.editorElement, this.hostWindow);
//      theTimer = Components.classes["@mozilla.org/timer;1"].createInstance(Components.interfaces.nsITimer);
//      theTimer.initWithCallback( plotInsertionTimer, nmSecs, Components.interfaces.nsITimer.TYPE_ONE_SHOT);
//    }
//  }
//};
//
//function msiEditorInsertionListener(editorElement, aWindow)
//{
//  this.editorElement = editorElement;
//  this.editor = msiGetEditor(editorElement);
//  this.hostWindow = aWindow;
//}
//
//msiEditorInsertionListener.prototype = msiEditorInsertionListenerBase;
//
//var msiEditorInsertionCommandControllerBase = 
//{
//  defaultController : {},
//  srcWindow : null,
//  cmdNames : ["cmd_paste", "cmd_undo", "cmd_redo"],
//  supportsCommand : function(cmd)
//  {
//    return ( this.cmdNames.indexOf(cmd) >= 0 );
//  },
//  isCommandEnabled : function(cmd)
//  {
//    if (this.defaultController[cmd])
//      return this.defaultController[cmd].isCommandEnabled(cmd);
//    var bCanDo = {value : false};
//    var bDummy = {value : false};
//    if (this.mEditor)
//    {
//      switch(cmd)
//      {
//        case "cmd_paste":
//          this.mEditor.canPaste(bCanDo);
//          return bCanDo.value;
//        break;
//        case "cmd_undo":   
//          this.mEditor.canUndo(bCanDo, bDummy);
//          return bCanDo.value;
//        break;
//        case "cmd_redo":
//          this.mEditor.canRedo(bCanDo, bDummy);
//          return bCanDo.value;
//        break;
//      }
//    }
//  },
//  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
//  doCommandParams: function(aCommand, aParams, aRefCon) {},
//  doCommand : function(cmd)
//  {
//    if (!this.mEditor)
//      this.mEditor = msiGetEditor(this.mEditorElement);
//    this.insertionListener = new msiEditorInsertionListener(this.mEditorElement, this.srcWindow);
//    if (this.mEditor && this.insertionListener)
//      this.mEditor.addInsertionListener(this.insertionListener);
//    if (this.defaultController[cmd])
//      this.defaultController[cmd].doCommand(cmd);
//    if (this.mEditor && this.insertionListener)
//    {
//      this.mEditor.removeInsertionListener(this.insertionListener);
//      this.insertionListener.addPlotInsertionTimers();
//    }
//  }
//};
//
//function msiEditorInsertionCommandController(editorElement, srcWindow)
//{
//  this.mEditorElement = editorElement;
//  this.mEditor = msiGetEditor(editorElement);
//  this.srcWindow = srcWindow;
//  var controllers = editorElement.contentWindow.controllers;
//  for (var ix = 0; ix < this.cmdNames.length; ++ix)
//  {
//    this.defaultController[this.cmdNames[ix]] = controllers.getControllerForCommand(this.cmdNames[ix]);
//  }
//}
//
//msiEditorInsertionCommandController.prototype = msiEditorInsertionCommandControllerBase;

function msiSetupMSIComputeMenuCommands(editorElement) {
  var commandTable = msiGetComposerCommandTable(editorElement);
  //dump("Registering msi compute menu commands\n");
  doSetupMSIComputeMenuCommands(commandTable);
  
//  var ourEditorInsertionController = new msiEditorInsertionCommandController(editorElement, window);
//  commandTable.registerCommand("cmd_paste", ourEditorInsertionController);
//  commandTable.registerCommand("cmd_undo", ourEditorInsertionController);
//  commandTable.registerCommand("cmd_redo", ourEditorInsertionController);
}

function doSetupMSIComputeMenuCommands(commandTable) {
  // note hokey use of shared command
  commandTable.registerCommand("cmd_MSIComputeEval", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeEvalKeyboard", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeEvalNum", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeSimplify", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeCombineExponentials", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeCombineLogs", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeCombinePowers", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeCombineTrig", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeCombineArctan", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeCombineHyperbolics", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeExpand", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeFactor", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeRewriteRational", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeRewriteFloat", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeRewriteMixed", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeRewriteExponential", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeRewriteFactorial", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeRewriteGamma", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeRewriteLogarithm", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeRewriteSinAndCos", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeRewriteSinhAndCosh", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeRewriteSin", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeRewriteCos", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeRewriteTan", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeRewriteArcsin", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeRewriteArccos", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeRewriteArctan", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeRewriteArccot", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeRewritePolar", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeRewriteRectangular", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeRewriteNormal", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeRewriteEquationsAsMatrix", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeRewriteMatrixAsEquations", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeCheckEquality", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeSolveExact", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeSolveNum", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeSolveRecursion", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeSolveInteger", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeCollect", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeDivide", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputePartialFractions", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeRoots", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeSort", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeCompanionMatrix", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeByParts", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeChangeVariable", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeApproxIntegral", msiEvaluateCommand);
  //disable  commandTable.registerCommand("cmd_MSIComputePlotApproxIntegral", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeFindExtrema", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeIterate", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeImplicitDiff", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeSolveODEExact", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeSolveODELaplace", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeSolveODENumeric", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeSolveODESeries", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputePowerSeries", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeFourier", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeInverseFourier", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeLaplace", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeInverseLaplace", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeGradient", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeDivergence", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeCurl", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeLaplacian", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeJacobian", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeHessian", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeWronskian", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeScalarPot", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeVectorPot", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeAdjugate", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeCharPoly", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeCholesky", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeColBasis", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeConcat", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeConditionNum", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeDefinitenessTests", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeDeterminant", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeEigenvalues", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeEigenvectors", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeFillMatrix", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeFFGE", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeGaussElim", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeHermite", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeHermitianTranspose", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeInverse", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeJordan", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeMap", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeMinPoly", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeNorm", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeNullspaceBasis", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeOrthogonalityTest", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputePermanent", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputePLU", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeRandomMatrix", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeRank", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeRationalCanonical", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeRREF", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeReshape", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeRowBasis", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeQR", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeSingularValues", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeSVD", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeSmith", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeSpectralRadius", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeStack", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeTrace", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeSimplexDual", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeSimplexFeasible", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeSimplexMaximize", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeSimplexMinimize", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeSimplexStandardize", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeTranspose", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeFitCurve", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeRandomNumbers", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeMean", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeMedian", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeMode", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeCorrelation", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeCovariance", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeGeometricMean", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeHarmonicMean", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeMeanDeviation", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeMoment", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeQuantile", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeStandardDeviation", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeVariance", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputePlot2DRectangular", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputePlot2DPolar", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputePlot2DImplicit", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputePlot2DInequality", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputePlot2DParametric", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputePlot2DConformal", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputePlot2DGradient", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputePlot2DVector", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputePlot2DODE", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputePlot2DAI", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputePlot3DRectangular", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputePlot3DCurve", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputePlot3DCylindrical", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputePlot3DSpherical", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputePlot3DParametric", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputePlot3DImplicit", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputePlot3DTube", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputePlot3DGradient", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputePlot3DVector", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputePlot2DARectangular", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputePlot2DAPolar", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputePlot2DAImplicit", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputePlot2DAInequality", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputePlot2DAParametric", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputePlot2DAConformal", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputePlot2DAGradient", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputePlot2DAVector", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputePlot3DARectangular", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputePlot3DACurve", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputePlot3DACylindrical", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputePlot3DASpherical", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputePlot3DAParametric", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputePlot3DAImplicit", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputePlot3DATube", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputePlot3DAGradient", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputePlot3DAVector", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeEditPlot", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeSetBasisVariables", msiDefineCommand);
  commandTable.registerCommand("cmd_MSIComputeDefine", msiDefineCommand);
  commandTable.registerCommand("cmd_MSIComputeUndefine", msiDefineCommand);
  commandTable.registerCommand("cmd_MSIComputeShowDefs", msiDefineCommand);
  commandTable.registerCommand("cmd_MSIComputeClearDefs", msiDefineCommand);
  commandTable.registerCommand("cmd_MSIComputeMapMuPADName", msiDefineCommand);
  //commandTable.registerCommand("cmd_MSIComputeUserSettings",  msiDefineCommand);
  //commandTable.registerCommand("cmd_MSIComputeSettings",      msiDefineCommand);
  //commandTable.registerCommand("cmd_MSIComputeSwitchEngines", msiDefineCommand);
  commandTable.registerCommand("cmd_MSIComputeInterpret", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeFixup", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputePassthru", msiEvaluateCommand);
  commandTable.registerCommand("cmd_MSIComputeStop", msiComputeStopCommand);
}

function goUpdateMSIcomputeMenuItems(commandset) {
  var commandNode;
  var commandID;
  var controller;
  var computeEnabled;
  var inMathBroadcaster;
  var editorElement;
  for (var i = 0; i < commandset.childNodes.length; i++) {
    commandNode = commandset.childNodes[i];
    commandID = commandNode.id;
    if (commandID) {
      goUpdateCommand(commandID);
      if (commandNode.hasAttribute("state")) goUpdateCommandState(commandID);
    }
  }
  // We find out if we are in math -- by finding if cmd_MSIComputeEval is enabled
  // If so, we set imMathBroadcaster to disabled. Operations disabled in math, such
  // as insert footnote, will observe this and hence be disabled.
  editorElement = msiGetActiveEditorElement();
  controller = msiGetControllerForCommand('cmd_MSIComputeEval', editorElement);
  computeEnabled = controller && controller.isCommandEnabled('cmd_MSIComputeEval');
  inMathBroadcaster = document.getElementById("inMathBroadcaster");
  if (computeEnabled) {
    inMathBroadcaster.setAttribute("disabled", "true");
  } else {
    inMathBroadcaster.removeAttribute("disabled");
  }
}

function msiGoUpdateMSIcomputeMenuItems(commandset, editorElement) {
  var commandNode;
  var commandID;
  var controller;
  var computeEnabled;
  var inMathBroadcaster;
  for (var i = 0; i < commandset.childNodes.length; i++) {
    commandNode = commandset.childNodes[i];
    commandID = commandNode.id;
    if (commandID) {
      msiGoUpdateCommand(commandID, editorElement);
      if (commandNode.hasAttribute("state")) msiGoUpdateCommandState(commandID, editorElement);
    }
  }
  // We find out if we are in math -- by finding if cmd_MSIComputeEval is enabled
  // If so, we set imMathBroadcaster to disabled. Operations disabled in math, such
  // as insert footnote, will observe this and hence be disabled.
//  controller = msiGetControllerForCommand('cmd_MSIComputeEval', editorElement);
//  computeEnabled = controller && controller.isCommandEnabled('cmd_MSIComputeEval');
  //rwa 050213 - The above WON'T DO - Scientific Word doesn't have computation at all! Use isInMath() instead:
  inMathBroadcaster = document.getElementById("inMathBroadcaster");
//  if (computeEnabled) {
  if (isInMath(editorElement)) {
    inMathBroadcaster.setAttribute("disabled", "true");
  } else {
    inMathBroadcaster.removeAttribute("disabled");
  }
}

// const fullmath = '<math xmlns="http://www.w3.org/1998/Math/MathML">';
var gComputeStringBundle;

function GetComputeString(name) {
  if (!gComputeStringBundle) {
    try {
      var strBundleService = Components.classes["@mozilla.org/intl/stringbundle;1"].getService();
      strBundleService = strBundleService.QueryInterface(Components.interfaces.nsIStringBundleService);

      gComputeStringBundle = strBundleService.createBundle("chrome://prince/locale/compute.properties");

    } catch (ex) {}
  }
  if (gComputeStringBundle) {
    try {
      return gComputeStringBundle.GetStringFromName(name);
    } catch (e) {}
  }
  return null;
}

// used by computeDebug.js.  Need to consider scripting interface in more detail.


function doComputeEvaluate(math, editorElement, inPlace) {
  if (!editorElement) editorElement = msiGetActiveEditorElement();
  doEvalComputation(math, GetCurrentEngine().Evaluate, "<mo>=</mo>", "evaluate", editorElement, inPlace);
}

//var ctrlClick = false;
var buttonPressed = -1;

function doComputeCommand2(event, cmd, editorElement, cmdHandler) {
  var inPlace = (getOS(window) == "osx") ? (event.metaKey) : (event.ctrlKey);
  doComputeCommand(cmd, editorElement, cmdHandler, inPlace);
}

function isSelectionMath(selection) {
  // findmathparent doesn't work when the selection is exactly a math node
  if (selection.focusNode === selection.anchorNode) {
    if (selection.focusNode.nodeName === "math") {
      return selection.focusNode;
    } else if (selection.focusOffset === selection.anchorOffset + 1) {
      if (selection.anchorNode.childNodes[selection.anchorOffset].nodeName === "math") {
        return selection.anchorNode.childNodes[selection.anchorOffset];
      }
    } else if (selection.anchorOffset === selection.focusOffset + 1) {
      if (selection.focusNode.childNodes[selection.focusOffset].nodeName === "math") {
        return selection.focusNode.childNodes[selection.focusOffset];
      }
    }
  }
  return findmathparent(selection.focusNode);
}

function doComputeStopCommand() {
   var eng = GetCurrentEngine();
   eng.stopProcessor();
}

function doComputeCommand(cmd, editorElement, cmdHandler, inPlace) {
  inPlace = !! inPlace;
  if (!editorElement) editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);
  var selection = editor.selection;
  if (selection) {
    var element = isSelectionMath(selection);
    if (!element) {
      dump("not in math!\n");
      return;
    }
    if (HasEmptyMath(element)) {
      dump("math has temp input\n");
      return;
    }
    editor.beginTransaction();
    var eng = GetCurrentEngine();
    switch (cmd) {
    case "cmd_compute_Evaluate":
      doEvalComputation(element, eng.Evaluate, "<mo>=</mo>", "evaluate", editorElement, inPlace);
      break;
    case "cmd_compute_EvaluateNumeric":
      doEvalComputation(element, eng.Evaluate_Numerically, "<mo>" + String.fromCharCode(0x2248) + "</mo>", "evaluate numeric", editorElement, inPlace);
      break;
    case "cmd_compute_Simplify":
      doEvalComputation(element, eng.Simplify, "<mo>=</mo>", "simplify", editorElement, inPlace);
      break;
    case "cmd_compute_CombineExponentials":
      doEvalComputation(element, eng.Combine_Exponentials, "<mo>=</mo>", "combine exp", editorElement, inPlace);
      break;
    case "cmd_compute_CombineLogs":
      doEvalComputation(element, eng.Combine_Logs, "<mo>=</mo>", "combine log", editorElement, inPlace);
      break;
    case "cmd_compute_CombinePowers":
      doEvalComputation(element, eng.Combine_Powers, "<mo>=</mo>", "combine pow", editorElement, inPlace);
      break;
    case "cmd_compute_CombineTrig":
      doEvalComputation(element, eng.Combine_Trig_Functions, "<mo>=</mo>", "combine trig", editorElement, inPlace);
      break;
    case "cmd_compute_CombineArctan":
      doEvalComputation(element, eng.Combine_Arctan, "<mo>=</mo>", "combine arctan", editorElement, inPlace);
      break;
    case "cmd_compute_CombineHyperbolics":
      doEvalComputation(element, eng.Combine_Hyperbolic_Functions, "<mo>=</mo>", "combine hyperbolic", editorElement, inPlace);
      break;
    case "cmd_compute_Expand":
      doEvalComputation(element, eng.Expand, "<mo>=</mo>", "expand", editorElement, inPlace);
      break;
    case "cmd_compute_Factor":
      doEvalComputation(element, eng.Factor, "<mo>=</mo>", "factor", editorElement, inPlace);
      break;
    case "cmd_compute_RewriteRational":
      doEvalComputation(element, eng.Rewrite_Rational, "<mo>=</mo>", "rewrite rational", editorElement, inPlace);
      break;
    case "cmd_compute_RewriteFloat":
      doEvalComputation(element, eng.Rewrite_Float, "<mo>=</mo>", "rewrite float", editorElement, inPlace);
      break;
    case "cmd_compute_RewriteMixed":
      doEvalComputation(element, eng.Rewrite_Mixed, "<mo>=</mo>", "rewrite mixed", editorElement, inPlace);
      break;
    case "cmd_compute_RewriteExponential":
      doEvalComputation(element, eng.Rewrite_Exponential, "<mo>=</mo>", "rewrite exponential", editorElement, inPlace);
      break;
    case "cmd_compute_RewriteFactorial":
      doEvalComputation(element, eng.Rewrite_Factorial, "<mo>=</mo>", "rewrite factorial", editorElement, inPlace);
      break;
    case "cmd_compute_RewriteGamma":
      doEvalComputation(element, eng.Rewrite_Gamma, "<mo>=</mo>", "rewrite gamma", editorElement, inPlace);
      break;
    case "cmd_compute_RewriteLogarithm":
      doEvalComputation(element, eng.Rewrite_Logarithm, "<mo>=</mo>", "rewrite logarithm", editorElement, inPlace);
      break;
    case "cmd_compute_RewriteSinAndCos":
      doEvalComputation(element, eng.Rewrite_sin_and_cos, "<mo>=</mo>", "rewrite sincos", editorElement, inPlace);
      break;
    case "cmd_compute_RewriteSinhAndCosh":
      doEvalComputation(element, eng.Rewrite_sinh_and_cosh, "<mo>=</mo>", "rewrite sinhcosh", editorElement, inPlace);
      break;
    case "cmd_compute_RewriteSin":
      doEvalComputation(element, eng.Rewrite_sin, "<mo>=</mo>", "rewrite sin", editorElement, inPlace);
      break;
    case "cmd_compute_RewriteCos":
      doEvalComputation(element, eng.Rewrite_cos, "<mo>=</mo>", "rewrite cos", editorElement, inPlace);
      break;
    case "cmd_compute_RewriteTan":
      doEvalComputation(element, eng.Rewrite_tan, "<mo>=</mo>", "rewrite tan", editorElement, inPlace);
      break;
    case "cmd_compute_RewriteArcsin":
      doEvalComputation(element, eng.Rewrite_arcsin, "<mo>=</mo>", "rewrite arcsin", editorElement, inPlace);
      break;
    case "cmd_compute_RewriteArccos":
      doEvalComputation(element, eng.Rewrite_arccos, "<mo>=</mo>", "rewrite arccos", editorElement, inPlace);
      break;
    case "cmd_compute_RewriteArctan":
      doEvalComputation(element, eng.Rewrite_arctan, "<mo>=</mo>", "rewrite arctan", editorElement, inPlace);
      break;
    case "cmd_compute_RewriteArccot":
      doEvalComputation(element, eng.Rewrite_arccot, "<mo>=</mo>", "rewrite arccot", editorElement, inPlace);
      break;
    case "cmd_compute_RewritePolar":
      doEvalComputation(element, eng.Rewrite_Polar, "<mo>=</mo>", "rewrite polar", editorElement, inPlace);
      break;
    case "cmd_compute_RewriteRectangular":
      doEvalComputation(element, eng.Rewrite_Rectangular, "<mo>=</mo>", "rewrite rectangular", editorElement, inPlace);
      break;
    case "cmd_compute_RewriteNormal":
      doEvalComputation(element, eng.Rewrite_Normal_Form, "<mo>=</mo>", "rewrite normal", editorElement, inPlace);
      break;
    case "cmd_compute_RewriteEquationsAsMatrix":
      doVarsComputation(element, ", Corresponding matrix: ", eng.Rewrite_Equations_as_Matrix, GetComputeString("EqnsAsMatrix.title"), editorElement, cmd, cmdHandler);
      break;
    case "cmd_compute_RewriteMatrixAsEquations":
      doVarsComputation(element, ", Corresponding equations: ", eng.Rewrite_Matrix_as_Equations, GetComputeString("MatrixAsEqns.title"), editorElement, cmd, cmdHandler);
      break;
    case "cmd_compute_CheckEquality":
      doComputeCheckEquality(element, editorElement);
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
    case "cmd_compute_SolveInteger":
      doComputeSolveInteger(element, "", editorElement, cmd, cmdHandler);
      break;
    case "cmd_compute_Collect":
      doVarsEvalComputation(element, eng.Polynomial_Collect, "<mo>=</mo>", GetComputeString("Collect.title"), "", editorElement, cmd, cmdHandler);
      break;
    case "cmd_compute_Divide":
      doComputeDivide(element, "", editorElement, cmd, cmdHandler);
      break;
    case "cmd_compute_PartialFractions":
      doComputePartialFractions(element, "", editorElement, cmd, cmdHandler);
      break;
    case "cmd_compute_Roots":
      //doLabeledComputation(element,"", eng.Polynomial_Roots,"Roots.fmt", editorElement);
      doComputeRoots(element, "", editorElement, cmd, cmdHandler);
      break;
    case "cmd_compute_Sort":
      doComputeSort(element, "", editorElement, cmd, cmdHandler);
      break;
    case "cmd_compute_CompanionMatrix":
      doComputeCompanionMatrix(element, "", editorElement, cmd, cmdHandler);
      break;
    case "cmd_compute_ByParts":
      doVarsEvalComputation(element, eng.Calculus_Integrate_by_Parts, "<mo>=</mo>", GetComputeString("ByParts.title"), GetComputeString("ByParts.remark"), editorElement, cmd, cmdHandler);
      break;
    case "cmd_compute_FindExtrema":
      doEvalComputation(element, eng.Calculus_Find_Extrema, "<mo>=</mo>", "find extrema", editorElement, inPlace, inPlace);
      //doVarsEvalComputation(element,eng.Calculus_Find_Extrema,"<mo>=</mo>",GetComputeString("ChangeVar.title"),GetComputeString("ChangeVar.remark"), editorElement, cmd, cmdHandler);
      break;
    case "cmd_compute_ChangeVariable":
      doVarsEvalComputation(element, eng.Calculus_Change_Variable, "<mo>=</mo>", GetComputeString("ChangeVar.title"), GetComputeString("ChangeVar.remark"), editorElement, cmd, cmdHandler);
      break;
    case "cmd_compute_ApproxIntegral":
      doComputeApproxIntegral(element, editorElement);
      break;
    case "cmd_compute_ImplicitDiff":
      doComputeImplicitDiff(element, editorElement, cmdHandler);
      break;
    case "cmd_compute_SolveODEExact":
      doComputeSolveODEExact(element, "ODE.fmt", "ODE.title", "", editorElement, cmd, cmdHandler);
      break;
    case "cmd_compute_SolveODELaplace":
      doComputeSolveODELaplace(element, "ODELaplace.fmt", "ODE.title", "", editorElement, cmd, cmdHandler);
      break;
    case "cmd_compute_SolveODENumeric":
      doComputeSolveODENumeric(element, "ODENumeric.fmt", "ODENumeric.title", "", editorElement, cmd, cmdHandler);
      break;
    case "cmd_compute_SolveODESeries":
      doComputeSolveODESeries(element, editorElement);
      break;
    case "cmd_compute_PowerSeries":
      doComputePowerSeries(element, editorElement, null);
      break;
    case "cmd_compute_Fourier":
      doLabeledComputation(element, "", eng.Fourier_Transform, "Fourier.fmt", editorElement);
      break;
    case "cmd_compute_InverseFourier":
      doLabeledComputation(element, "", eng.Inverse_Fourier_Transform, "InvFourier.fmt", editorElement);
      break;
    case "cmd_compute_Laplace":
      doLabeledComputation(element, "", eng.Laplace_Transform, "Laplace.fmt", editorElement);
      break;
    case "cmd_compute_InverseLaplace":
      doLabeledComputation(element, "", eng.Inverse_Laplace_Transform, "InvLaplace.fmt", editorElement);
      break;
    case "cmd_compute_Gradient":
      doLabeledComputation(element, "", eng.Gradient, "Gradient.fmt", editorElement);
      break;
    case "cmd_compute_Divergence":
      doLabeledComputation(element, "", eng.Divergence, "Divergence.fmt", editorElement);
      break;
    case "cmd_compute_Curl":
      doLabeledComputation(element, "", eng.Curl, "Curl.fmt", editorElement);
      break;
    case "cmd_compute_Laplacian":
      doLabeledComputation(element, "", eng.Laplacian, "Laplacian.fmt", editorElement);
      break;
    case "cmd_compute_Jacobian":
      doLabeledComputation(element, "", eng.Jacobian, "Jacobian.fmt", editorElement);
      break;
    case "cmd_compute_Hessian":
      doLabeledComputation(element, "", eng.Hessian, "Hessian.fmt", editorElement);
      break;
    case "cmd_compute_Wronskian":
      doComputeWronskian(element, "", editorElement, cmd, cmdHandler);
      break;
    case "cmd_compute_ScalarPot":
      doScalarPotential(element, eng.Scalar_Potential, "ScalarPot.fmt", editorElement);
      break;
    case "cmd_compute_VectorPot":
      doLabeledComputation(element, "", eng.Vector_Potential, "VectorPot.fmt", editorElement);
      break;
    case "cmd_MSIComputeAdjugate":
      doLabeledComputation(element, "", eng.Adjugate, "Adjugate.fmt", editorElement);
      break;
    case "cmd_MSIComputeCharPoly":
      doComputeCharPoly(element, "", editorElement, cmd, cmdHandler);
      break;
    case "cmd_MSIComputeCholesky":
      doLabeledComputation(element, "", eng.Cholesky_Decomposition, "Cholesky.fmt", editorElement);
      break;
    case "cmd_MSIComputeColBasis":
      doLabeledComputation(element, "", eng.Column_Basis, "ColBasis.fmt", editorElement);
      break;
    case "cmd_MSIComputeConcat":
      doLabeledComputation(element, "", eng.Concatenate, "Concat.fmt", editorElement);
      break;
    case "cmd_MSIComputeConditionNum":
      doLabeledComputation(element, "", eng.Condition_Number, "ConditionNum.fmt", editorElement);
      break;
    case "cmd_MSIComputeDefinitenessTests":
      doLabeledComputation(element, "", eng.Definiteness_Tests, "DefTest.fmt", editorElement);
      break;
    case "cmd_MSIComputeDeterminant":
      doLabeledComputation(element, "", eng.Determinant, "Determinant.fmt", editorElement);
      break;
    case "cmd_MSIComputeEigenvalues":
      doLabeledComputation(element, "", eng.Eigenvalues, "Eigenvalues.fmt", editorElement);
      break;
    case "cmd_MSIComputeEigenvectors":
      // formatting needs to be fixed
      doLabeledComputation(element, "", eng.Eigenvectors, "Eigenvectors.fmt", editorElement);
      break;
    case "cmd_MSIComputeFFGE":
      doLabeledComputation(element, "", eng.Fraction_Free_Gaussian_Elimination, "FFGaussElim.fmt", editorElement);
      break;
    case "cmd_MSIComputeGaussElim":
      doLabeledComputation(element, "", eng.Gaussian_Elimination, "GaussElim.fmt", editorElement);
      break;
    case "cmd_MSIComputeHermite":
      doLabeledComputation(element, "", eng.Hermite_Normal_Form, "Hermite.fmt", editorElement);
      break;
    case "cmd_MSIComputeHermitianTranspose":
      doLabeledComputation(element, "", eng.Hermitian_Transpose, "HermitianTr.fmt", editorElement);
      break;
    case "cmd_MSIComputeInverse":
      doLabeledComputation(element, "", eng.Inverse, "Inverse.fmt", editorElement);
      break;
    case "cmd_MSIComputeJordan":
      //doLabeledComputation(element,eng.Jordan_Form,"Jordan.fmt", editorElement);
      doEvalComputation(element, "", eng.Jordan_Form, "<mo>=</mo>", "evaluate jordan", editorElement, inPlace);
      break;
    case "cmd_MSIComputeMap":
      doComputeMap(element, editorElement, cmd, cmdHandler);
      break;
    case "cmd_MSIComputeMinPoly":
      doComputeMinPoly(element, "", editorElement, cmd, cmdHandler);
      break;
    case "cmd_MSIComputeNorm":
      doLabeledComputation(element, "", eng.Norm, "Norm.fmt", editorElement);
      break;
    case "cmd_MSIComputeNullspaceBasis":
      doLabeledComputation(element, "", eng.Nullspace_Basis, "Nullspace.fmt", editorElement);
      break;
    case "cmd_MSIComputeOrthogonalityTest":
      doLabeledComputation(element, "", eng.Orthogonality_Test, "Orthogonality.fmt", editorElement);
      break;
    case "cmd_MSIComputePermanent":
      doLabeledComputation(element, "", eng.Permanent, "Permanent.fmt", editorElement);
      break;
    case "cmd_MSIComputePLU":
      doLabeledComputation(element, "", eng.PLU_Decomposition, "PLU.fmt", editorElement);
      break;
    case "cmd_MSIComputeRank":
      doLabeledComputation(element, "", eng.Rank, "Rank.fmt", editorElement);
      break;
    case "cmd_MSIComputeRationalCanonical":
      doLabeledComputation(element, "", eng.Rational_Canonical_Form, "Rational.fmt", editorElement);
      break;
    case "cmd_MSIComputeRREF":
      doLabeledComputation(element, "", eng.Reduced_Row_Echelon_Form, "RREchelonForm.fmt", editorElement);
      break;
    case "cmd_MSIComputeReshape":
      doComputeReshape(element, editorElement);
      break;
    case "cmd_MSIComputeRowBasis":
      doLabeledComputation(element, "", eng.Row_Basis, "RowBasis.fmt", editorElement);
      break;
    case "cmd_MSIComputeQR":
      doLabeledComputation(element, "", eng.QR_Decomposition, "QR.fmt", editorElement);
      break;
    case "cmd_MSIComputeSingularValues":
      doLabeledComputation(element, "", eng.Singular_Values, "Singular.fmt", editorElement);
      break;
    case "cmd_MSIComputeSVD":
      doLabeledComputation(element, "", eng.SVD, "SVD.fmt", editorElement);
      //doEvalComputation(element,"",eng.SVD,"<mo>=</mo>", "evaluate SVD", editorElement, inPlace);
      break;
    case "cmd_MSIComputeSmith":
      doLabeledComputation(element, "", eng.Smith_Normal_Form, "Smith.fmt", editorElement);
      break;
    case "cmd_MSIComputeSpectralRadius":
      doLabeledComputation(element, "", eng.Spectral_Radius, "SpectralRadius.fmt", editorElement);
      break;
    case "cmd_MSIComputeStack":
      doLabeledComputation(element, "", eng.Stack, "Stack.fmt", editorElement);
      break;
    case "cmd_MSIComputeTrace":
      doLabeledComputation(element, "", eng.Trace, "Trace.fmt", editorElement);
      break;
    case "cmd_MSIComputeTranspose":
      doLabeledComputation(element, "", eng.Transpose, "Transpose.fmt", editorElement);
      break;

    case "cmd_compute_SimplexDual":
      doLabeledComputation(element, "", eng.Simplex_Dual, "Dual.fmt", editorElement);
      break;
    case "cmd_compute_SimplexFeasible":
      doLabeledComputation(element, "", eng.Simplex_Feasible, "Feasible.fmt", editorElement);
      break;
    case "cmd_compute_SimplexMaximize":
      doLabeledComputation(element, "", eng.Simplex_Maximize, "Maximize.fmt", editorElement);
      break;
    case "cmd_compute_SimplexMinimize":
      doLabeledComputation(element, "", eng.Simplex_Minimize, "Minimize.fmt", editorElement);
      break;
    case "cmd_compute_SimplexStandardize":
      doLabeledComputation(element, "", eng.Simplex_Standardize, "Standardize.fmt", editorElement);
      break;

    case "cmd_compute_FitCurve":
      doComputeFitCurve(element, editorElement);
      break;
    case "cmd_compute_Mean":
      doLabeledComputation(element, "", eng.Mean, "Mean.fmt", editorElement);
      break;
    case "cmd_compute_Median":
      doLabeledComputation(element, "", eng.Median, "Median.fmt", editorElement);
      break;
    case "cmd_compute_Mode":
      doLabeledComputation(element, "", eng.Mode, "Mode.fmt", editorElement);
      break;
    case "cmd_compute_Correlation":
      doLabeledComputation(element, "", eng.Correlation, "Correlation.fmt", editorElement);
      break;
    case "cmd_compute_Covariance":
      doLabeledComputation(element, "", eng.Covariance, "Covariance.fmt", editorElement);
      break;
    case "cmd_compute_GeometricMean":
      doLabeledComputation(element, "", eng.Geometric_Mean, "GeometricMean.fmt", editorElement);
      break;
    case "cmd_compute_HarmonicMean":
      doLabeledComputation(element, "", eng.Harmonic_Mean, "HarmonicMean.fmt", editorElement);
      break;
    case "cmd_compute_MeanDeviation":
      doLabeledComputation(element, "", eng.Mean_Deviation, "MeanDeviation.fmt", editorElement);
      break;
    case "cmd_compute_Moment":
      doComputeMoment(element, editorElement, null);
      break;
    case "cmd_compute_Quantile":
      doComputeQuantile(element, editorElement, cmd, cmdHandler);
      break;
    case "cmd_compute_StandardDeviation":
      doLabeledComputation(element, "", eng.Standard_Deviation, "StdDeviation.fmt", editorElement);
      break;
    case "cmd_compute_Variance":
      doLabeledComputation(element, "", eng.Variance, "Variance.fmt", editorElement);
      break;

    case "cmd_compute_Plot2DRectangular":
      doComputePlot(element, 2, "rectangular", false, editorElement);
      break;
    case "cmd_compute_Plot2DPolar":
      doComputePlot(element, 2, "polar", false, editorElement);
      break;
    case "cmd_compute_Plot2DImplicit":
      doComputePlot(element, 2, "implicit", false, editorElement);
      break;
    case "cmd_compute_Plot2DInequality":
      doComputePlot(element, 2, "inequality", false, editorElement);
      break;
    case "cmd_compute_Plot2DParametric":
      doComputePlot(element, 2, "parametric", false, editorElement);
      break;
    case "cmd_compute_Plot2DConformal":
      doComputePlot(element, 2, "conformal", false, editorElement);
      break;
    case "cmd_compute_Plot2DGradient":
      doComputePlot(element, 2, "gradient", false, editorElement);
      break;
    case "cmd_compute_Plot2DVector":
      doComputePlot(element, 2, "vectorField", false, editorElement);
      break;
    case "cmd_compute_Plot2DODE":
      doComputePlot(element, 2, "ode", false, editorElement);
      break;
    case "cmd_compute_Plot2DAI":
      doComputePlot(element, 2, "approximateIntegral", false, editorElement);
      break;
    case "cmd_compute_Plot3DRectangular":
      doComputePlot(element, 3, "rectangular", false, editorElement);
      break;
    case "cmd_compute_Plot3DCurve":
      doComputePlot(element, 3, "curve", false, editorElement);
      break;
    case "cmd_compute_Plot3DCylindrical":
      doComputePlot(element, 3, "cylindrical", false, editorElement);
      break;
    case "cmd_compute_Plot3DSpherical":
      doComputePlot(element, 3, "spherical", false, editorElement);
      break;
    case "cmd_compute_Plot3DParametric":
      doComputePlot(element, 3, "parametric", false, editorElement);
      break;
    case "cmd_compute_Plot3DImplicit":
      doComputePlot(element, 3, "implicit", false, editorElement);
      break;
    case "cmd_compute_Plot3DTube":
      doComputePlot(element, 3, "tube", false, editorElement);
      break;
    case "cmd_compute_Plot3DGradient":
      doComputePlot(element, 3, "gradient", false, editorElement);
      break;
    case "cmd_compute_Plot3DVector":
      doComputePlot(element, 3, "vectorField", false, editorElement);
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
      doEvalComputation(element, eng.Interpret, "<mo>=</mo>", "interpret", editorElement);
      break;
    case "cmd_compute_Fixup":
      doFixupComputation(element, eng.Fixup, "<mo>=</mo>", "fixup", editorElement);
      break;
    default:
      dump("Unknown compute command. (" + cmd + ")\n");
      return;
    }
    editor.endTransaction();
  }
}

function doGlobalComputeCommand(cmd, editorElement) {
  if (!editorElement) editorElement = msiGetActiveEditorElement();
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
  case "cmd_compute_MapMuPADName":
    doComputeMapMuPADName(editorElement);
    break;

    //   case "cmd_compute_UserSettings":
    //     doComputeUserSettings();
    //     break;
    //   case "cmd_compute_Settings":
    //     doComputeSettings();
    //     break;
    //   case "cmd_compute_SwitchEngines":
    //     doComputeSwitchEngines();
    //     break;
  case "cmd_compute_Passthru":
    doComputePassthru(editorElement);
    break;
  case "cmd_compute_stop":
    doComputeStopCommand();
    break;
  default:
    dump("Unknown global compute command. (" + cmd + ")\n");
    return;
  }
}

var isRunning = false;

function vcamToolbarFromPlugin(obj) {
  // sets the state of the toolbar buttoms from the plugin state
  // Call for initialization and after each button action.
  // Cursor tool section
  var ct = obj.cursorTool;
  var dim = obj.dimension;
  var anim = obj.isAnimated;
  var za = obj.zoomAction;
  document.getElementById("vc-SelObj").checked = (ct === 'select');
  document.getElementById("vc-RotateScene").checked = (ct === 'rotate');
  document.getElementById("vc-Move").checked = (ct === 'move');
  document.getElementById("vc-Query").checked = (ct === 'query');
  document.getElementById("vc-AutoZoomIn").checked = (za === 1);
  document.getElementById("vc-AutoZoomOut").checked = (za === 2);
  // Rotation section
  if (dim === 3) {
    document.getElementById("vc-Zoom").checked = (ct === 'zoom');
    var va = obj.rotateVerticalAction;
    document.getElementById("vc-RotateRight").checked = (va === 1);
    document.getElementById("vc-RotateLeft").checked = (va === 2);
    var ha = obj.rotateHorizontalAction;
    document.getElementById("vc-RotateUp").checked = (ha === 1);
    document.getElementById("vc-RotateDown").checked = (ha === 2);
    // Zoom section
    //  document.getElementById("AutoZoomIn").checked = (za === 1);
    //  document.getElementById("AutoZoomOut").checked = (za === 2);
    // Speed controlf
    var spd = obj.actionSpeed;
    if (spd <= 0.125) document.getElementById("actionspeed8s").checked = true;
    else if (spd > 0.125 && spd <= 0.25) document.getElementById("actionspeed4s").checked = true;
    else if (spd > 0.25 && spd <= 0.5) document.getElementById("actionspeed2s").checked = true;
    else if (spd > 0.5 && spd <= 1.0) document.getElementById("actionspeedN").checked = true;
    else if (spd > 1.0 && spd <= 2.0) document.getElementById("actionspeed2f").checked = true;
    else if (spd > 2.0 && spd <= 4.0) document.getElementById("actionspeed4f").checked = true;
    else if (spd > 4.0) document.getElementById("actionspeed8f").checked = true;
  }
  if (anim) {
    // animation section, including animation speed control
    var aspd = obj.animationSpeed;
    if (aspd <= 0.125) document.getElementById("animspeed8s").checked = true;
    else if (aspd > 0.125 && aspd <= 0.25) document.getElementById("animspeed4s").checked = true;
    else if (aspd > 0.25 && aspd <= 0.5) document.getElementById("animspeed2s").checked = true;
    else if (aspd > 0.5 && aspd <= 1.0) document.getElementById("animspeedN").checked = true;
    else if (aspd > 1.0 && aspd <= 2.0) document.getElementById("animspeed2f").checked = true;
    else if (aspd > 2.0 && aspd <= 4.0) document.getElementById("animspeed4f").checked = true;
    else if (aspd > 4.0) document.getElementById("animspeed8f").checked = true;
  }
}

function makeSnapshotPath(object) {
  var extension;
  //  var graph, gslist, animated, dimension;
  //  if (extension == null) {
  //    graph = msiFindParentOfType( object, "graph");
  //    gslist = graph.getElementsByTagName("graphSpec");
  //    if (gslist.length > 0) {
  //      gslist = gslist[0];
  //    }
  //    extension = "png";
  //    if (gslist) {
  //      dimension = gslist.getAttribute("Dimension");
  //      animated = gslist.getAttribute("Animate");
  //      if (animated == "true") {
  //        extension = "avi";
  //      } else if (dimension == 3) {
  //        extension = "png";
  //      } else if (dimension == 2) {
  //        extension = "svg";
  //      }
  //    }
  //  }
  if (getOS(window) == "win") {
    extension = "bmp";
  } else {
    extension = "png";
  }
  var path;
  try {
    var srcPath = msiGetIOService().newURI(object.data, null, null);
    srcPath = srcPath.QueryInterface(Components.interfaces.nsIURL);
    var fileName = srcPath.fileName;
    fileName = fileName.replace(/xv[cz]$/, extension);
    //    path = object.data;
    path = "graphics/" + fileName;
    //    path = path.replace(/(file:\/\/.*)(\/plots\/)([^\/]+)/,'graphics/$3');
    //    path = path.replace(/xv[cz]$/,extension);
  } catch (e) {
    msidump(e.message);
  }
  return path;
}

function insertSnapshot(object, snapshotpath) {
  var parent, i, objectlist, element, ssobj, graph, gslist, w, h, units, oldpath, file, url;
  parent = object.parentNode;
  objectlist = parent.getElementsByTagName("object");
  var snapshotUrl = msiFileURLFromAbsolutePath(snapshotpath);
  var snapshotRelUrl = msiMakeRelativeUrl(snapshotUrl.spec);
  for (i = 0; i < objectlist.length;) {
    element = objectlist[i];
    if (element.hasAttribute("msisnap")) {
      // remove the snapshot file as well as the object node
      oldpath = element.data;
      if (oldpath && oldpath.length > 7) {
        oldpath = oldpath.slice(7);
      }
      file = Components.classes["@mozilla.org/file/local;1"].
      createInstance(Components.interfaces.nsILocalFile);
      file.initWithPath(oldpath);
      if (file.exists()) {
        try {
          file.remove(false);
        } catch (e) {
          msidump(e.message);
        }
      }
      parent.removeChild(element);
    } else {
      i++;
    }
  }
  graph = msiFindParentOfType(object, "graph");
  gslist = graph.getElementsByTagName("graphSpec");
  if (gslist.length > 0) {
    gslist = gslist[0];
  }
  ssobj = object.cloneNode(true); // copies useful attributes
  ssobj.removeAttribute("msigraph");
  ssobj.setAttribute("data", snapshotRelUrl);
  ssobj.setAttribute("msisnap", "true");
  ssobj.removeAttribute("type");
  parent.appendChild(ssobj);
  if (gslist) { // copy some attributes from the graphspec
    w = gslist.getAttribute("Width");
    if (w) {
      ssobj.setAttribute("naturalWidth", w);
      ssobj.setAttribute("imageWidth", w);
    }
    h = gslist.getAttribute("Height");
    if (h) {
      ssobj.setAttribute("naturalHeight", h);
      ssobj.setAttribute("imageHeight", h);
    }
    ssobj.setAttribute("units", gslist.getAttribute("Units"));
  }
}

function buildSnapshotFile(obj, abspath, res) {
  var func = function(abspath, res) {
      obj.makeSnapshot(abspath, res);
    };
  func(abspath, res);
}

function doMakeSnapshot(obj, graph, editorElement, graphicsMultiCallback) {
  //  tryUntilSuccessful(200,10, function(){
  //  var intervalId;
  //  var count = 0;
  //  intervalId = setInterval(function() {
  //    if (count >= 10) {
  //      clearInterval(intervalId);
  //      return;
  //    }
  if (obj.makeSnapshot && obj.readyState === 2) {
    try {
      var path = makeSnapshotPath(obj);
      var abspath;
      var prefs;
      var res;
      var plotWrapper = obj.parentNode;
      try {
        prefs = GetPrefs();
        res = prefs.getIntPref("swp.GraphicsSnapshotRes");
      } catch (e) {
        res = 300;
      }
      if (graph == null) {
        graph = new Graph();
        DOMGraph = msiFindParentOfType(obj, "graph");
        gslist = DOMGraph.getElementsByTagName("graphSpec");
        if (gslist.length > 0) {
          gslist = gslist[0];
        }
        graph.extractGraphAttributes(DOMGraph);
      }
      if (editorElement == null) {
        editorElement = msiGetActiveEditorElement();
      }
      abspath = makeRelPathAbsolute(path, editorElement);
      var snapshotDir = Components.classes["@mozilla.org/file/local;1"].
      createInstance(Components.interfaces.nsILocalFile);
      snapshotDir.initWithPath(abspath);
      snapshotDir = snapshotDir.parent;
      if (!snapshotDir.exists()) snapshotDir.create(1, 0755);
      //        if ( getOS(window) == "win") {
      //        // in this case path is a complete file url string
      //          abspath = path.slice(8);
      //          abspath = abspath.replace("/","\\","g");
      //          abspath = abspath.replace("%20", " ", "g");
      //        }
      plotWrapper.wrappedObj.makeSnapshot(abspath, res);
      if (getOS(window) == "win") {
        var file = Components.classes["@mozilla.org/file/local;1"].
        createInstance(Components.interfaces.nsILocalFile);
        file.initWithPath(abspath);
        var graphicsPath = file.clone();
        graphicsPath = graphicsPath.parent;
        //          graphicsPath.append("graphics");
        var timerHandler = null;
        if (graphicsMultiCallback) timerHandler = graphicsMultiCallback.addNewTimerHandler(10000); //set time limit of 10 seconds for conversion to finish?
        var targFile = graphicsConverter.doGraphicsImport(file, graphicsPath, "import", window, timerHandler);
        //          doGraphicsImport(file, {inFileType:"bmp",exepath:"%ImageMagick%",output:"png",commandLine:"convert %inputFile% %outputFile%" }, "import");
        path = targFile.path;
        //          path.replace("plots","graphics");
        //          path.replace(".bmp", ".png");
      }
      insertSnapshot(obj, path);
    } catch (e) {
      msidump(e.message);
    }
  }
  //    count++;
  //  }, 200);
}

function rebuildSnapshots(doc) {
  var wrapperlist, objlist, length, objlength, i, regexp, match, name1, name2;
  var editorElement = msiGetActiveEditorElement();
  var graphicsCallbackObj = new graphicsMultipleTimerHandler();
  wrapperlist = doc.documentElement.getElementsByTagName("plotwrapper");
  length = wrapperlist.length;
  for (i = 0; i < length; i++) {
    objlist = wrapperlist[i].getElementsByTagName("object");
    objlength = objlist.length;
    if (objlength === 1) {
      doMakeSnapshot(objlist[0], null, editorElement, graphicsCallbackObj);
    } else if (objlength > 1) {
      regexp = /plot\d+/;
      match = regexp.exec(objlist[0].data);
      name1 = match[0];
      match = regexp.exec(objlist[1].data);
      name2 = match[0];
      if (name1 !== name2) {
        doMakeSnapshot(objlist[0], null, editorElement, graphicsCallbackObj);
      }
    }
  }
  var checkGraphicsCallback = (function(callbackObj) {
    return function() {
      return callbackObj.isFinished();
    };
  })(graphicsCallbackObj);

  tryUntilSuccessful(200, 200, checkGraphicsCallback);
}

function doVCamCommandOnObject(obj, cmd, editorElement) {
  if (!editorElement) editorElement = msiGetActiveEditorElement();
  netscape.security.PrivilegeManager.enablePrivilege('UniversalXPConnect');
  try {
    switch (cmd) {
    case "cmd_vcSelObj":
      obj.cursorTool = "select";
      break;
    case "cmd_vcRotateLeft":
      if (obj.rotateVerticalAction == 2) {
        obj.rotateVerticalAction = 0;
      } else {
        obj.rotateVerticalAction = 2;
      }
      break;
    case "cmd_vcRotateRight":
      if (obj.rotateVerticalAction == 1) {
        obj.rotateVerticalAction = 0;
      } else {
        obj.rotateVerticalAction = 1;
      }
      break;
    case "cmd_vcRotateUp":
      if (obj.rotateHorizontalAction == 1) {
        obj.rotateHorizontalAction = 0;
      } else {
        obj.rotateHorizontalAction = 1;
      }
      break;
    case "cmd_vcRotateDown":
      if (obj.rotateHorizontalAction == 2) {
        obj.rotateHorizontalAction = 0;
      } else {
        obj.rotateHorizontalAction = 2;
      }
      break;
    case "cmd_vcRotateScene":
      obj.cursorTool = "rotate";
      break;
    case "cmd_vcZoom":
      obj.cursorTool = "zoom";
      break;
    case "cmd_vcMove":
      obj.cursorTool = "move";
      break;
    case "cmd_vcQuery":
      obj.cursorTool = "query";
      break;
    case "cmd_vcZoomBoxIn":
      obj.cursorTool = "select"; // This is zoomBoxIn in the 2d case
      break;
    case "cmd_vcZoomBoxOut":
      obj.cursorTool = "select";
      break;
    case "cmd_vcZoomIn":
      obj.cursorTool = "zoomIn";
      break;
    case "cmd_vcZoomOut":
      obj.cursorTool = "zoomOut";
      break;
    case "cmd_vcSnapshot":
      doMakeSnapshot(obj, null, editorElement);
      break;
    case "cmd_vcAutoSpeed":
      dump("cmd_vcAutoSpeed not implemented");
      break;
    case "cmd_vcAnimSpeed":
      dump("cmd_vcAnimSpeed not implemented");
      break;
    case "cmd_vcAutoZoomIn":
      if (obj.zoomAction == 1) obj.zoomAction = 0;
      else obj.zoomAction = 1;
      break;
    case "cmd_vcAutoZoomOut":
      if (obj.zoomAction == 2) obj.zoomAction = 0;
      else obj.zoomAction = 2;
      break;
    case "cmd_vcGoToEnd":
      obj.currentTime = obj.endTime;
      break;
    case "cmd_vcGoToStart":
      obj.currentTime = obj.beginTime;
      break;
    case "cmd_vcLoopType":
      dump("cmd_vcLoopType not implemented");
      break;
    case "cmd_vcPlay":
      if (isRunning) obj.stopAnimation();
      else obj.startAnimation();
      isRunning = !isRunning;
      break;
    case "cmd_vcFitContents":
      obj.fitContents();
      break;
    default:
    }
  } catch (e) {
    msidump(e.message);
  }
  vcamToolbarFromPlugin(obj);
  return;
}

function onCloseVCam(obj, editorElement)
{
  if (document.getElementById("vcamactive") && (document.getElementById("vcamactive").getAttribute("hidden")=="false"))
    return;
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);
  var graph = new Graph();
  var graphNode = editor.getElementOrParentByTagName("graph", obj);
  graph.extractGraphAttributes(graphNode);
  queryVCamValues(obj, graph, graphNode, true);
}

var gProgressbar;

function onVCamMouseDown(screenX, screenY) {
  var editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);
  //  var evt = editor.document.createEvent("MouseEvents");
  //  evt.initMouseEvent("mousedown", true, true, editor.document.defaultView,null,
  //    screenX, screenY, null, null, 0, 0, 0, 0, null, null);
  editor.selection.collapse(this.parentNode, 0);
  editor.checkSelectionStateForAnonymousButtons(editor.selection);
  doVCamInitialize(this);
}

function onVCamDblClick(screenX, screenY) {
  var editorElement = msiGetActiveEditorElement();
  //  var editor = msiGetEditor(editorElement);
  //  var evt = editor.document.createEvent("MouseEvents");
  //  evt.initMouseEvent("dblclick", true, true, editor.document.defaultView,null,
  //    screenX, screenY, null, null, 0, 0, 0, 0, null, null);
  goDoPrinceCommand("cmd_objectProperties", this, editorElement);
}

function onVCamRightMouseDown(screenX, screenY)
{
  var editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);
  var graphNode = editor.getElementOrParentByTagName("graph", this);
  var contextMenu = document.getElementById("msiEditorContentContext");
  if (contextMenu)
  {
    contextMenu.showPopup(graphNode, screenX, screenY, "none", "none");
	contextMenu.focus();
  }
//  var evt = editor.document.createEvent("MouseEvents");
//  evt.initMouseEvent("mousedown", true, true, editor.document.defaultView, 1,
//    screenX, screenY, null, null, 0, 0, 0, 0, 2, null);
//  this.parentNode.dispatchEvent(evt);
}

function onVCamRightMouseUp(screenX, screenY)
{
  var editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);
  var evt = editor.document.createEvent("MouseEvents");
  evt.initMouseEvent("mouseup", true, true, editor.document.defaultView, 1,
    screenX, screenY, null, null, 0, 0, 0, 0, 2, null);
  this.parentNode.dispatchEvent(evt);
}

function onVCamKeyDown(aKeyCodeMaybe)
{
  dump("In onVCamKeyDown, keyCode was " + (aKeyCodeMaybe ? aKeyCodeMaybe : "null") + "\n");
}

function onVCamKeyUp(aKeyCodeMaybe)
{
  dump("In onVCamKeyUp, keyCode was " + (aKeyCodeMaybe ? aKeyCodeMaybe : "null") + "\n");
}

function onVCamTreeChange(treeEvent)
{
  try
  {
    msidump("Reported VCam tree change event: type is [" + treeEvent.type + "], target node is [" + treeEvent.target.nodeName + "], property changed is [" + treeEvent.property + "]\n");
  } catch(exc)
  {msidump("Got exception in onVCamTreeChange: " + exc + "\n");}
}

function onVCamDragMove(x, y) {
  return 1;
}

function onVCamDragLeave(x, y) {}

function queryVCamValues(obj, graph, domGraph, bUserSetIfChanged)
{
  var cameraVals = null;
  var coordSysVals = {XAxisTitle : "x", YAxisTitle : "y",
                      ViewingBoxXMin : "-5", ViewingBoxXMax : "5",
                      ViewingBoxYMin : "-5", ViewingBoxYMax : "5"};
  var camera, vcamDoc, kidNode, coordSysNode, sceneNode
  var dim = obj.dimension;
  var sceneNodeName = "Scene" + dim + "d";
  var coordSysNodeName = "CoordinateSystem" + dim + "d";
  var aProp;
  
  if (dim == 3)
  {
    cameraVals = {positionX : "0", positionY : "0", positionZ : "0",
                    focalPointX : "0", focalPointY : "0", focalPointZ : "0",
                    upVectorX : "0", upVectorY : "0", upVectorZ : "1",
                    keepUpVector : "false", viewingAngle : "2.0944", orthogonalProjection : "false"};
    coordSysVals.ViewingBoxZMin = "-5";
    coordSysVals.ViewingBoxZMax = "5";
    coordSysVals.ZAxisTitle = "z";
    if (obj.camera)
    {
      for (aProp in cameraVals)
      {
        cameraVals[aProp] = obj.camera[aProp];
      }
//      aVal = camera.positionX;
//      aVal = camera.positionY;
//      aVal = camera.positionZ;
//      aVal = camera.focalPointX;
//      aVal = camera.focalPointY;
//      aVal = camera.focalPointZ;
//      aVal = camera.upVectorX;
//      aVal = camera.upVectorY;
//      aVal = camera.upVectorZ;
    }
    else
      cameraVals = null;
    if (graph)
    {
      graph.setCameraValsFromVCam(cameraVals, domGraph, bUserSetIfChanged);
    }
  }
  if (obj.isAnimated)
  {
    animVals = {beginTime : 0, endTime : 10, currentTime : 0};
    for (aProp in animVals)
      animVals[aProp] = obj[aProp];
    animVals.framesPerSecond = 5;
    if (graph)
      graph.setAnimationValsFromVCam(animVals, domGraph, bUserSetIfChanged);
  }
  vcamDoc = obj.document;
  if (vcamDoc)
  {
    sceneNode = vcamDoc.documentElement.getChildByTagName(sceneNodeName)
    coordSysNode = sceneNode.getChildByTagName(coordSysNodeName);
    if (coordSysNode)
    {
      kidNode = coordSysNode.firstChild;
      while (kidNode)
      {
        if (kidNode.nodeName in coordSysVals)
          coordSysVals[kidNode.nodeName] = kidNode.firstChild.nodeValue;
        kidNode = kidNode.nextSibling;
      }
    }
  }
  if (!coordSysNode)
    coordSysVals = null;
  if (graph)
    graph.setCoordSysValsFromVCam(coordSysVals, domGraph, bUserSetIfChanged);
    
}

function doVCamPreInitialize(obj, graph) {
  tryUntilSuccessful(200, 10, function() {
    var editorElement = msiGetActiveEditorElement();
    var editor = msiGetEditor(editorElement);
    var domGraph = editor.getElementOrParentByTagName("graph", obj);
    var plotWrapper = obj.parentNode;
    try {
      if (obj.addEvent && (obj.readyState === 2)) {
        obj.addEvent('leftMouseDown', onVCamMouseDown);
        obj.addEvent('leftMouseUp', onVCamMouseUp);
        obj.addEvent('leftMouseDoubleClick', onVCamDblClick);
        obj.addEvent('dragMove', onVCamDragMove);
        obj.addEvent('dragLeave', (function() {}));
        if (graph) {
          obj.addEvent('dragEnter', graph.provideDragEnterHandler(editorElement, domGraph));
          obj.addEvent('drop', graph.provideDropHandler(editorElement, domGraph));
        }
        queryVCamValues(obj, graph, domGraph);
        // add a method for writing a snapshot
        var fn = function() {
            return doMakeSnapshot(obj, graph, editorElement);
          };
        plotWrapper.wrappedObj = obj;
        doMakeSnapshot(obj, graph, editorElement);
        obj.vcamStatus = "initialized";  //add a property so we know we've successfully initialized the VCam object
        return true;
      }
    } catch (e) {
      if (e.message == "NPMethod called on non-NPObject wrapped JSObject!")
      {
        obj.vcamStatus = "needRecreate";
        return true;  //just to stop the repetition - this isn't going to succeed
      }
      msidump(e.message);
    }
    return false;
  });
}

//function preInitializeVCam(obj, graph, editorElement, successHandler, failHandler, exceptionHandler) {
//  tryUntilSuccessfulWithHandlers(200, 10, function() {
//    msidump("Entering an attempt in preInitializeVCam\n");
//    var editor = msiGetEditor(editorElement);
//    var domGraph = editor.getElementOrParentByTagName("graph", obj);
//    var plotWrapper = obj.parentNode;
//    if (obj.addEvent && (obj.readyState === 2)) {
//      obj.addEvent('leftMouseDown', onVCamMouseDown);
//      obj.addEvent('leftMouseUp', onVCamMouseUp);
//      obj.addEvent('leftMouseDoubleClick', onVCamDblClick);
//      obj.addEvent('dragMove', onVCamDragMove);
//      obj.addEvent('dragLeave', (function() {}));
//      if (graph) {
//        obj.addEvent('dragEnter', graph.provideDragEnterHandler(editorElement, domGraph));
//        obj.addEvent('drop', graph.provideDropHandler(editorElement, domGraph));
//      }
//      queryVCamValues(obj, graph, domGraph);
//      // add a method for writing a snapshot
//      var snapFn = function() {
//        return doMakeSnapshot(obj, graph, editorElement);
//      };
//      plotWrapper.wrappedObj = obj;
//      doMakeSnapshot(obj, graph, editorElement);
//      obj.vcamStatus = "initialized";  //add a property so we know we've successfully initialized the VCam object
//      msidump("Leaving a successful attempt in preInitializeVCam\n");
//      return true;
//    }
//    msidump("Leaving an unsuccessful attempt in preInitializeVCam\n");
//    return false;
//  }, successHandler, failHandler, exceptionHandler);
//}

function onVCamMouseUp() {
  //  alert("Mouse up in plugin!");
}

var setAnimationTime;
var VCamCommand;
var setActionSpeed;
var setAnimSpeed;
var setLoopMode;
var vcamCloseFunction;

function doVCamCommand(cmd) {
  VCamCommand(cmd);
}

function doVCamClose()
{
  if (document.getElementById("vcamactive") && (document.getElementById("vcamactive").getAttribute("hidden")=="false"))
    return;  //In other words, if we're being called because of the broadcaster turning VCam on, do nothing
  var editorElement = msiGetActiveEditorElement();
  return vcamCloseFunction(editorElement);
}

function doVCamInitialize(obj) {
  dump("doVCamInitialize");
  document.getElementById("vcamactive").setAttribute("hidden", "false");
  var editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);
  var os = getOS(window);

  VCamCommand = (function() {
    var thisobj = obj;
    return function(_cmd, _editorElement) {
      return doVCamCommandOnObject(thisobj, _cmd, _editorElement);
    };
  }());
  setActionSpeed = (function() {
    var thisobj = obj;
    return function(factor) {
      thisobj.actionSpeed = factor;
    };
  }());
  vcamCloseFunction = (function() {
    var thisobj = obj;
    return function(_editorElement) {
      return onCloseVCam(thisobj, _editorElement);
    };
  }());
  var threedplot = obj.dimension === 3;
  var twodplot = obj.dimension === 2;
  document.getElementById("3dplot").setAttribute("hidden", threedplot ? "false" : "true");
  document.getElementById("2dplot").setAttribute("hidden", twodplot ? "false" : "true");
  var animplot = obj.isAnimated;
  document.getElementById("animplot").setAttribute("hidden", animplot ? "false" : "true");
  if (animplot) // set up the progress bar
  {
    setAnimationTime = (function() {
      var thisobj = obj;
      return function() {
        var time = thisobj.beginTime + (gProgressbar.value / 100) * (thisobj.endTime - thisobj.beginTime);
        obj.currentTime = time;
      };
    }());
    try {
      gProgressbar = document.getElementById("vc-AnimScale");
      obj.addEvent("currentTimeChange", showAnimationTime(obj));
    } catch (e) {
      dump("failure: " + e.toString() + "\n");
    }
    setAnimSpeed = (function() {
      var thisobj = obj;
      return function(factor) {
        thisobj.animationSpeed = factor;
      };
    }());
    setLoopMode = (function() {
      var thisobj = obj;
      return function(mode) {
        thisobj.animationLoopingMode = mode;
      };
    }());
  }
  vcamToolbarFromPlugin(obj);
  editorElement.focus();
}

function showAnimationTime(obj) {
  var thisobj = obj;
  return function(time) {
    var newval = Math.round(100 * (time / (thisobj.endTime - thisobj.beginTime)));
    gProgressbar.value = newval;
  };
}

function dontSetAnimationTime() {
  return;
}

function initComputeLogger(engine) {
  dump("initComputeLogger with " + engine + "\n");
  var prefs = GetPrefs();
  var logMMLSent, logMMLReceived, logEngSent, logEngReceived;
  try {
    logMMLSent = prefs.getBoolPref("swp.user.logSent");
  } catch (ex) {
    dump("\nfailed to get swp.user.logSent pref!\n");
  }
  try {
    logMMLReceived = prefs.getBoolPref("swp.user.logReceived");
  } catch (ex) {
    dump("\nfailed to get swp.user.logReceived pref!\n");
  }
  try {
    logEngSent = prefs.getBoolPref("swp.user.engSent");
  } catch (ex) {
    dump("\nfailed to get swp.user.engSent pref!\n");
  }
  try {
    logEngReceived = prefs.getBoolPref("swp.user.engReceived");
  } catch (ex) {
    dump("\nfailed to get swp.user.engReceived pref!\n");
  }
  dump("call msiComputeLogger.Init\n");
  msiComputeLogger.Init(engine, logMMLSent, logMMLReceived, logEngSent, logEngReceived);
}

// our connection to the computation code
var compsample;
var compengine;

function GetCurrentEngine() {
  if (!compsample) {
    compsample = Components.classes["@mackichan.com/simplecomputeengine;2"].getService(Components.interfaces.msiISimpleComputeEngine);
    try {
      var dsprops = Components.classes["@mozilla.org/file/directory_service;1"].createInstance(Components.interfaces.nsIProperties);
      var inifile = dsprops.get("resource:app", Components.interfaces.nsIFile);
      //      var inifile = dsprops.get("GreD", Components.interfaces.nsIFile);
      //inifile.append("xulrunner");
      inifile.append("mupInstall.gmr");
      compsample.startup(inifile);
      compengine = 2;
      initComputeLogger(compsample);
      initEnginePrefs(compsample);
    } catch (e) {
      var msg_key;
      if (e.result == Components.results.NS_ERROR_NOT_AVAILABLE) msg_key = "Error.notavailable";
      else if (e.result == Components.results.NS_ERROR_FILE_NOT_FOUND) msg_key = "Error.notfound";
      else if (e.result == Components.results.NS_ERROR_NOT_INITIALIZED) msg_key = "Error.notinitialized";
      else if (e.result == Components.results.NS_ERROR_FAILURE) msg_key = "Error.failure";
      else throw e;
      AlertWithTitle(GetComputeString("Error.title"), GetComputeString(msg_key));
    }
  }
  return compsample;
}



function initEnginePrefs(currEngine)
{
  var prefs = GetPrefs();
  var delimstyle = prefs.getCharPref("swp.user.matrix_delim");
  var val;
  if (delimstyle === "matrix_brackets") val = 1;
    else if (delimstyle === "matrix_parens") val = 2;
    else if (delimstyle === "matrix_braces") val = 3;
    else val = 0;
  var eng = GetCurrentEngine();
  dump("HERE" + eng);
  eng.setUserPref("Default_matrix_delims", val); 
  // Do we need to set all the other engine settings. This is the only one that I
  // know of that needs translation from strings to integers. BBM
}


// cursor helpers


function ComputeCursor(editorElement) {
  if (!editorElement) editorElement = msiGetActiveEditorElement();
  var theWin = msiGetWindowContainingEditor(editorElement);
  theWin.setCursor("spinning");
}

function RestoreCursor(editorElement) {
  if (!editorElement) editorElement = msiGetActiveEditorElement();
  var theWin = msiGetWindowContainingEditor(editorElement);
  if (theWin) theWin.setCursor("default");
}

// return node on RHS of =, if structure is that simple


function GetRHS(math) {
  var ch = last_child(math);
  var mathout = math.cloneNode(false);

  while (ch) {
    if (ch.nodeType == Node.ELEMENT_NODE && ch.localName == "mo") {
      var op = ch.firstChild;
      if (op.nodeType == Node.TEXT_NODE && op.data == "=") {
        var m = node_after(ch);
        while (m) {
          var cpy = m.cloneNode(true);
          mathout.appendChild(cpy);
          m = node_after(m);
        }
        return mathout;
      }
    }
    ch = node_before(ch);
  }
  return math;
}

function isAChildOf(node, parent) {
  while (node) {
    if (node == parent) {
      return true;
    } else {
      node = node.parentNode;
    }
  }
  return false;
}

function isEqualSign(node) {
  if (node != null && node.nodeType == Node.ELEMENT_NODE && node.localName == "mo") {
    var op = node.firstChild;
    if (op != null && op.nodeType == Node.TEXT_NODE && op.data == "=") return true;
  }
  return false;
}

function FindLeftEndOfSide(mathElement, node) {
  // find left end of mathElement containing the node
  var leftEnd = first_child(mathElement);
  var m = leftEnd;

  while (m) {
    if (isAChildOf(node, m)) break;

    if (isEqualSign(m)) {
      leftEnd = node_after(m);
    }

    m = node_after(m);
  }

  return leftEnd;
}

function FindRightEndOfSide(mathElement, leftEnd) {
  var rightEnd = leftEnd;
  var next = rightEnd;

  while (next) {
    if (isEqualSign(next)) break;
    else rightEnd = next;

    next = node_after(next);
  }

  return rightEnd;
}

function CloneTheSide(mathElement, leftEnd, rightEnd) {
  var mathout = mathElement.cloneNode(false);
  var m = leftEnd;
  while (m) {
    var cpy = m.cloneNode(true);
    mathout.appendChild(cpy);
    if (m == rightEnd) break;
    m = node_after(m);
  }
  return mathout;
}

function GetASide(mathElement, editorElement) {
  var anchor = msiGetEditor(editorElement).selection.anchorNode;
  var leftEnd = FindLeftEndOfSide(mathElement, anchor);
  var rightEnd = FindRightEndOfSide(mathElement, leftEnd);

  var mathOut = CloneTheSide(mathElement, leftEnd, rightEnd);
  return mathOut;
}

//SLS for unknown reasons, get parsing error if text is at outer level


function insertLabeledXML(editor, text, node, offset) {
  editor.setCaretAfterElement(node);
  editor.insertHTML("<span>" + text + "</span>");

  //   var parser = new DOMParser();
  //   var wrapped = "<span>" + text + "</span>";
  //   var doc = parser.parseFromString(wrapped,"application/xhtml+xml");
  //   var nodeList = doc.documentElement.childNodes;
  //   var nodeListLength = nodeList.length;
  //   var i;
  //   for (i = nodeListLength-1; i >= 0; --i)
  //   {
  //     editor.insertNode( nodeList[i], node, offset );
  //   }
}

function appendResult(result, sep, math, editorElement) {
  if (!editorElement) editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);

  var appendedResult = result.replace(fullmath, fullmath + sep);

  insertXML(editor, appendedResult, math, math.childNodes.length);
  coalescemath(editorElement);
}

function GetOffset(math, node) {
  var i = 1;
  var n = first_child(math);
  while (n != node) {
    n = node_after(n);
    ++i;
  }
  return i;
}

function insertResult(result, sep, mathElement, editorElement, rightEnd) {
  if (!editorElement) editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);

  var appendedResult = result.replace(fullmath, fullmath + sep);

  insertXML(editor, appendedResult, mathElement, GetOffset(mathElement, rightEnd));

  coalescemath(editorElement);
}

function appendLabel(label, math, editorElement) {
  if (!editorElement) editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);
  insertXML(editor, label, math, math.childNodes.length);
  //  msiGetEditor(editorElement).insertHTMLWithContext(
  //      label,
  //      "", "", "", null,
  //      math, math.childNodes.length, false );
}

// "label" is a format string like ", Solution: %result%"


function appendLabeledResult(result, label, math, editorElement) {
  if (!editorElement) editorElement = msiGetActiveEditorElement();

  var editor = msiGetEditor(editorElement);
  var resultArray;
  var preStr;
  var midStr;
  var postStr;
  editor.beginTransaction();
  var resultLoc = label.search(/%result%/);
  var res1Loc = -1;
  var res2Loc = -1;
  var resPre;
  var resPost;
  if (resultLoc == -1) {
    res1Loc = label.search(/%res1%/);
    res2Loc = label.search(/%res2%/);
  }
  var temp;
  var space = "<hspace type='thinSpace' dim='0.17em'/>";

  if (-1 == resultLoc) {
    if (res1Loc == res2Loc == -1) {
      preStr = label;
      postStr = "";
    }  else {  //res1 or res2 or both found
      preStr  = label.substr(0, res1Loc);
      midStr  =  label.substr(res1Loc + 6, res2Loc - (res1Loc + 6));
      postStr = label.substr(res2Loc + 6);
    }
  } else {
    preStr = label.substr(0, resultLoc);
    var match = "%result%";
    postStr = label.substr(resultLoc + match.length);
  }

  editor.setCaretAfterElement(math);
  if (res2Loc == -1 && res1Loc == -1) {
    editor.insertHTML(preStr);
    editor.insertHTML(result);
    editor.insertHTML(postStr);
  }
  else {
    resPre = "<math xmlns=\"http://www.w3.org/1998/Math/MathML\"><mrow>";
    resPost = "</mrow></math>";
    temp = result.replace(resPre, "");
    temp = temp.replace(resPost,"");
    resultArray = temp.split("<mo>,</mo>");
    if (resultArray && resultArray.length > 1) {
      editor.insertHTML(preStr + space);
      editor.insertHTML(resPre + resultArray[0] + resPost + space);
      editor.insertHTML(midStr + space);
      editor.insertHTML(resPre + resultArray[1] + resPost + space);
      editor.insertHTML(postStr + space);
    }
  }
  editor.endTransaction();
}

function appendTaggedResult(result, label, body, index, editorElement) {
  if (!editorElement) editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);
  var str = label.replace(/%result%/, result);
  insertXML(editor, str, body, index);
  //  msiGetEditor(editorElement).insertHTMLWithContext(str,"","","",null,body,index,false);
}

function addResult(result, editorElement) {
  if (!editorElement) editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);
  var selection = editor.selection;
  //insertXML(editor, result, selection.focusNode, selection.focusOffset );
  editor.insertHTMLWithContext(result, "", "", "", null, selection.focusNode, selection.focusOffset, false);
}


function runFixup(math) {
  try {
    var out = GetCurrentEngine().perform(math, GetCurrentEngine().Fixup);
    return out;
  } catch (e) {
    msiComputeLogger.Exception(e);
    return math; // what else to do?  Fixup shouldn't fail.
  }
}

function GetFixedMath(math) {
  return runFixup(GetMathAsString(math));
}

function doLabeledComputation(math, vars, op, labelID, editorElement) {
  var mathstr = GetFixedMath(GetRHS(math));
  if (!vars) vars = "";

  msiComputeLogger.Sent("doing " + labelID + " after fixup", mathstr);
  ComputeCursor(editorElement);
  try {
    var out = GetCurrentEngine().perform(mathstr, op);
    msiComputeLogger.Received(out);
    appendLabeledResult(out, GetComputeString(labelID), math, editorElement);
  } catch (ex) {
    if (ex.result == compsample.needvars) {
      var o = {};
      o.mParentWin = this;
      o.theMath = math;
      o.vars = vars;
      o.theCommand = cmd;
      o.theCommandHandler = cmdHandler;
      o.afterDialog = function(editorElement) {
        if (this.Cancel) return;
        this.mParentWin.doComputeSolveExact(this.theMath, this.vars, editorElement, this.theCommand, this.theCommandHandler);
      };
      try {
        theDialog = msiOpenModelessDialog("chrome://prince/content/ComputeVariables.xul", "_blank", "chrome,close,titlebar,resizable,dependent", editorElement, cmd, cmdHandler, o);
      } catch (e) {
        AlertWithTitle("Error in computeOverlay.js", "Exception in doLabeledComputation: [" + e + "]");
        return;
      }
      //        parentWin.openDialog("chrome://prince/content/ComputeVariables.xul", "computevariables", "chrome,close,titlebar,modal", o);
      //        if (o.Cancel)
      //          return;
      //        vars = runFixup(o.vars);
      if (!o.Cancel) msiGetEditor(editorElement).incrementModificationCount(1);
    } else {
      msiComputeLogger.Exception(ex);
      //        done = true;
    }
    msiComputeLogger.Exception(e);
  }
  RestoreCursor(editorElement);
}

function doScalarPotential(math, op, labelID, editorElement) {
  var mathstr = GetFixedMath(GetRHS(math));
  msiComputeLogger.Sent("doing " + labelID + " after fixup", mathstr);
  ComputeCursor(editorElement);
  try {
    var out = GetCurrentEngine().perform(mathstr, op);
    msiComputeLogger.Received(out);

    if (out == "<math xmlns=\"http://www.w3.org/1998/Math/MathML\"><mrow><mn>0</mn></mrow></math>") {
      var editor = msiGetEditor(editorElement);
      editor.setCaretAfterElement(math);
      editor.insertHTML(GetComputeString("DoesNotExist"));
    } else {
      appendLabeledResult(out, GetComputeString(labelID), math, editorElement);
    }
  } catch (e) {
    msiComputeLogger.Exception(e);
  }
  RestoreCursor(editorElement);
}

function CloneTheRange(mathElement, r, editorElement) {
  var mathout = mathElement.cloneNode(false);
  var editor = msiGetEditor(editorElement);
  //var c = r.cloneRange();
  //mathout.insert(c);
  var nodeArray = editor.nodesInRange(r);
  var enumerator = nodeArray.enumerate();
  while (enumerator.hasMoreElements()) {
    var node = enumerator.getNext();
    node = node.cloneNode(true);
    mathout.appendChild(node);
  }
  return mathout;
}

// like above, but use operator instead of text between input and result


function doEvalComputation(mathElement, op, joiner, remark, editorElement, inPlace) {
  // Get a side of the (possible) equations
  var sel = msiGetEditor(editorElement).selection;
  var anchor = sel.anchorNode;
  var leftEnd;
  var rightEnd;
  var mathOut;
  if (sel.isCollapsed) {
    leftEnd = FindLeftEndOfSide(mathElement, anchor);
    rightEnd = FindRightEndOfSide(mathElement, leftEnd);
    mathOut = CloneTheSide(mathElement, leftEnd, rightEnd);
  } else {
    var r = sel.getRangeAt(0);
    leftEnd = FindLeftEndOfSide(mathElement, anchor);
    rightEnd = FindRightEndOfSide(mathElement, leftEnd);
    mathOut = CloneTheRange(mathElement, r, editorElement);
  }

  var mathstr = GetFixedMath(mathOut);

  msiComputeLogger.Sent(remark + " after fixup", mathstr);
  ComputeCursor(editorElement);
  try {
    var out = GetCurrentEngine().perform(mathstr, op);
    msiComputeLogger.Received(out);
    //appendResult(out,joiner,math, editorElement);
    if (!inPlace) {
      insertResult(out, joiner, mathElement, editorElement, rightEnd);
    } else {
      if (!editorElement) editorElement = msiGetActiveEditorElement();

      var editor = msiGetEditor(editorElement);
      msiGoDoCommand('cmd_delete');
      //editor.deleteSelection(editor.eNone);
      sel = msiGetEditor(editorElement).selection;
      sel.collapseToStart();
      var destNode = sel.anchorNode;
      var destOffset = sel.anchorOffset;
      if (!msiNavigationUtils.isMathNode(sel.anchorNode)) {
        destNode = editor.document.createElementNS("http://www.w3.org/1998/Math/MathML", "math");
        editor.insertNode(destNode, sel.anchorNode, sel.anchorOffset);
        destOffset = 0;
      }
      insertXML(editor, out, destNode, destOffset);
    }
    coalescemath(editorElement);
  } catch (e) {
    msiComputeLogger.Exception(e);
  }
  RestoreCursor(editorElement);
}

// like above, but make sure to run Fixup exactly once


function doFixupComputation(math, op, joiner, remark, editorElement) {
  var mathstr = GetMathAsString(GetRHS(math));
  msiComputeLogger.Sent(remark, mathstr);
  ComputeCursor(editorElement);
  try {
    var out = GetCurrentEngine().perform(mathstr, op);
    msiComputeLogger.Received(out);
    appendResult(out, joiner, math, editorElement);
  } catch (e) {
    msiComputeLogger.Exception(e);
  }
  RestoreCursor(editorElement);
}

function postDialogTimerCallback(editorElement, obj) {
  dump("Hit postDialogTimerCallback!\n");
  if (obj == null) {
    dump("No object passed in to postDialogTimerCallback!\n");
    return;
  }
  clearTimeout(obj.mDialogTimer);
  if (("afterDialog" in obj) && obj.afterDialog != null) obj.afterDialog(editorElement);
}

// like above, but asks user for variable(s) first


function doVarsComputation(math, label, func, title, editorElement, cmd, cmdHandler) {
  var o = {};
  o.title = title;
  o.mParentWin = this;
  o.theMath = math;
  o.theLabel = label;
  o.theFunc = func;
  o.afterDialog = function(editorElement) {
    this.mParentWin.finishVarsComputation(editorElement, this);
  };
  var theDialog = null;
  try {
    theDialog = msiOpenModelessDialog("chrome://prince/content/ComputeVariables.xul", "_blank", "chrome,close,titlebar,resizable,dependent", editorElement, cmd, cmdHandler, o);
  } catch (e) {
    AlertWithTitle("Error in computeOverlay.js", "Exception in doVarsComputation: [" + e + "]");
    return;
  }
}

function finishVarsComputation(editorElement, o) {
  dump("\n*** finishVarsComputation ***");
  if (o.Cancel) return;
  dump("\n*** Cancel was false");
  var vars = o.vars;
  var mathstr = GetFixedMath(o.theMath);
  msiComputeLogger.Sent4(o.theLabel + " after fixup", mathstr, "specifying", vars);

  ComputeCursor(editorElement);
  try {
    var eng = GetCurrentEngine();
    if (o.theFunc == eng.Rewrite_Equations_as_Matrix) {
      out = eng.equationsAsMatrix(mathstr, vars);
    } else if (o.theFunc == eng.Rewrite_Matrix_as_Equations) {
      out = eng.matrixAsEquations(mathstr, vars);
    } else {
      out = eng.perform(mathstr, o.theFunc);
    }
    msiComputeLogger.Received(out);
    appendLabeledResult(out, o.theLabel, o.theMath, editorElement);
  } catch (e) {
    msiComputeLogger.Exception(e);
  }
  RestoreCursor(editorElement);
}

// like above, but use operator instead of text between input and result


function doVarsEvalComputation(math, func, joiner, title, label, editorElement, cmd, cmdHandler) {
  var o = {};
  o.title = title;
  o.label = label;
  o.theMath = math;
  o.theFunc = func;
  o.theJoiner = joiner;
  o.mParentWin = this;
  o.afterDialog = function(editorElement) {
    this.mParentWin.finishVarsEvalComputation(editorElement, this);
  };
  var parentWin = msiGetParentWindowForNewDialog(editorElement);
  try {
    theDialog = msiOpenModelessDialog("chrome://prince/content/ComputeVariables.xul", "_blank", "chrome,close,titlebar,resizable,dependent", editorElement, cmd, cmdHandler, o);

  } catch (e) {
    AlertWithTitle("Error in computeOverlay.js", "Exception in doVarsEvalComputation: [" + e + "]");
    return;
  }
  //  parentWin.openDialog("chrome://prince/content/ComputeVariables.xul", "computevariables", "chrome,close,titlebar,modal", o);
}

function finishVarsEvalComputation(editorElement, o) {
  if (o.Cancel) return;

  var vars = o.vars;
  var mathstr = GetFixedMath(o.theMath);
  //  vars = runFixup(vars);
  msiComputeLogger.Sent4(title + " after fixup", mathstr, "specifying", vars);

  ComputeCursor(editorElement);
  try {
    //var out = o.theFunc(mathstr,vars);
    var out = "";
    var eng = GetCurrentEngine();
    if (o.theFunc == eng.Polynomial_Collect) {
      out = eng.collect(mathstr, vars);
    } else if (o.theFunc == eng.Calculus_Integrate_by_Parts) {
      out = eng.byParts(mathstr, vars);
    } else if (o.theFunc == eng.Calculus_Change_Variable) {
      out = eng.changeVar(mathstr, vars);
    }

    msiComputeLogger.Received(out);
    appendResult(out, o.theJoiner, o.theMath, editorElement);
  } catch (e) {
    msiComputeLogger.Exception(e);
  }
  RestoreCursor(editorElement);
}

// actual command handlers

function doComputeCheckEquality(math, editorElement) {
  var mathstr = GetFixedMath(math);

  msiComputeLogger.Sent("doing " + "CheckEquality" + " after fixup", mathstr);

  ComputeCursor(editorElement);
  try {
    var out = GetCurrentEngine().perform(mathstr, GetCurrentEngine().Check_Equality);
    msiComputeLogger.Received(out);
    appendLabeledResult(out, GetComputeString("CheckEquality.fmt"), math, editorElement);
  } catch (e) {
    msiComputeLogger.Exception(e);
  }
  RestoreCursor(editorElement);

  //var out = GetCurrentEngine().checkEquality(mathstr,vars);
  //doLabeledComputation(math, eng.Check_Equality, "CheckEquality.fmt", editorElement);
}

function doComputeSolveExact(math, vars, editorElement, cmd, cmdHandler) {
  var mathstr = GetFixedMath(math);
  if (!vars) vars = "";
  if (!editorElement) editorElement = msiGetActiveEditorElement();
  msiComputeLogger.Sent4("solve exact", mathstr, "specifying", vars);
  try {
    ComputeCursor(editorElement);
    var out = GetCurrentEngine().solveExact(mathstr, vars);
    msiComputeLogger.Received(out);
    appendLabeledResult(out, GetComputeString("Solution.fmt"), math, editorElement);
    RestoreCursor(editorElement);
  } catch (ex) {
    RestoreCursor(editorElement);
    if (ex.result == compsample.nosol) {
      appendLabel(GetComputeString("NoSolution"), math, editorElement);
      done = true;
    } else if (ex.result == compsample.needvars) {
      var o = {};
      o.mParentWin = this;
      o.theMath = math;
      o.vars = vars;
      o.theCommand = cmd;
      o.theCommandHandler = cmdHandler;
      o.afterDialog = function(editorElement) {
        if (this.Cancel) return;
        this.mParentWin.doComputeSolveExact(this.theMath, this.vars, editorElement, this.theCommand, this.theCommandHandler);
      };
      try {
        theDialog = msiOpenModelessDialog("chrome://prince/content/ComputeVariables.xul", "_blank", "chrome,close,titlebar,resizable,dependent", editorElement, cmd, cmdHandler, o);
      } catch (e) {
        AlertWithTitle("Error in computeOverlay.js", "Exception in doComputeSolveExact: [" + e + "]");
        return;
      }
      //        parentWin.openDialog("chrome://prince/content/ComputeVariables.xul", "computevariables", "chrome,close,titlebar,modal", o);
      //        if (o.Cancel)
      //          return;
      //        vars = runFixup(o.vars);
      if (!o.Cancel) msiGetEditor(editorElement).incrementModificationCount(1);
    } else {
      msiComputeLogger.Exception(ex);
      //        done = true;
    }
    //    }
  }
}

function doComputeSolveInteger(math, vars, editorElement, cmd, cmdHandler) {
  var mathstr = GetFixedMath(math);
  if (!vars) vars = "";

  if (!editorElement) editorElement = msiGetActiveEditorElement();

  msiComputeLogger.Sent4("solve integer", mathstr, "specifying", vars);

  try {
    ComputeCursor(editorElement);
    var out = GetCurrentEngine().solveInteger(mathstr, vars);
    msiComputeLogger.Received(out);
    appendLabeledResult(out, GetComputeString("Solution.fmt"), math, editorElement);
    RestoreCursor(editorElement);
  } catch (ex) {
    RestoreCursor(editorElement);
    if (ex.result == compsample.nosol) {
      appendLabel(GetComputeString("NoSolution"), math, editorElement);
      done = true;
    } else if (ex.result == compsample.needvars) {
      var o = {};
      o.mParentWin = this;
      o.theMath = math;
      o.vars = vars;
      o.theCommand = cmd;
      o.theCommandHandler = cmdHandler;
      o.afterDialog = function(editorElement) {
        if (this.Cancel) return;
        this.mParentWin.doComputeSolveInteger(this.theMath, this.vars, editorElement, this.theCommand, this.theCommandHandler);
      };
      try {
        theDialog = msiOpenModelessDialog("chrome://prince/content/ComputeVariables.xul", "_blank", "chrome,close,titlebar,resizable,dependent", editorElement, cmd, cmdHandler, o);
      } catch (e) {
        AlertWithTitle("Error in computeOverlay.js", "Exception in doComputeSolveInteger: [" + e + "]");
        return;
      }
    } else {
      msiComputeLogger.Exception(ex);
    }
  }
}

function doComputeSolveNumeric(math, editorElement) {
  var mathstr = GetFixedMath(math);
  msiComputeLogger.Sent("solve numeric after fixup", mathstr);
  if (!editorElement) editorElement = msiGetActiveEditorElement();

  ComputeCursor(editorElement);
  try {
    var out = GetCurrentEngine().perform(mathstr, GetCurrentEngine().Solve_Numeric);
    msiComputeLogger.Received(out);
    appendLabeledResult(out, GetComputeString("Solution.fmt"), math, editorElement);
  } catch (ex) {
    if (ex.result == compsample.nosol) {
      appendLabel(GetComputeString("NoSolution"), math, editorElement);
    } else {
      msiComputeLogger.Exception(ex);
    }
  }
  RestoreCursor(editorElement);
}

function doComputeSolveRecursion(math, editorElement) {
  var mathstr = GetFixedMath(math);
  msiComputeLogger.Sent("solve recursion after fixup", mathstr);
  if (!editorElement) editorElement = msiGetActiveEditorElement();

  ComputeCursor(editorElement);
  try {
    var out = GetCurrentEngine().perform(mathstr, GetCurrentEngine().Solve_Recursion);
    msiComputeLogger.Received(out);
    appendLabeledResult(out, GetComputeString("Solution.fmt"), math, editorElement);
  } catch (ex) {
    if (ex.result == compsample.nosol) {
      appendLabel(GetComputeString("NoSolution"), math, editorElement);
    } else {
      msiComputeLogger.Exception(ex);
    }
  }
  RestoreCursor(editorElement);
}

function doComputePartialFractions(math, vars, editorElement, cmd, cmdHandler) {
  if (!editorElement) editorElement = msiGetActiveEditorElement();
  var mathstr = GetFixedMath(math);
  if (!vars) vars = "";
  //  var done = false;
  //  while (!done) {
  msiComputeLogger.Sent4("partial fractions", mathstr, "specifying", vars);
  try {
    ComputeCursor(editorElement);
    var out = GetCurrentEngine().partialFractions(mathstr, vars);
    msiComputeLogger.Received(out);
    appendResult(out, "<mo>=</mo>", math, editorElement);
    RestoreCursor(editorElement);
    //      done = true;
  } catch (ex) {
    RestoreCursor(editorElement);
    if (ex.result == compsample.needvars) {
      var o = {};
      o.title = GetComputeString("ParFrac.title");
      o.mParentWin = this;
      o.theMath = math;
      o.vars = vars;
      o.theCommand = cmd;
      o.theCommandHandler = cmdHandler;
      o.afterDialog = function(editorElement) {
        if (this.Cancel) return;
        this.mParentWin.doComputePartialFractions(this.theMath, this.vars, editorElement, this.theCommand, this.theCommandHandler);
      };
      try {
        var theDialog = msiOpenModelessDialog("chrome://prince/content/ComputeVariables.xul", "_blank", "chrome,close,titlebar,resizable,dependent", editorElement, cmd, cmdHandler, o);
      } catch (e) {
        AlertWithTitle("Error in computeOverlay.js", "Exception in doComputePartialFractions: [" + e + "]");
        return;
      }

      //        var parentWin = msiGetParentWindowForNewDialog(editorElement);
      //        parentWin.openDialog("chrome://prince/content/ComputeVariables.xul", "computevariables", "chrome,close,titlebar,modal", o);
      //        if (o.Cancel)
      //          return;
      //        vars = runFixup(o.vars);
    } else {
      msiComputeLogger.Exception(ex);
    }
  }
  //  }
}

function doComputeDivide(math, vars, editorElement, cmd, cmdHandler) {
  if (!editorElement) editorElement = msiGetActiveEditorElement();
  var mathstr = GetFixedMath(math);

  if (!vars) vars = "";
  //  var done = false;
  //  while (!done) {
  msiComputeLogger.Sent4("divide", mathstr, "specifying", vars);
  try {
    ComputeCursor(editorElement);
    var out = GetCurrentEngine().divide(mathstr, vars);
    msiComputeLogger.Received(out);
    appendResult(out, "<mo>=</mo>", math, editorElement);
    RestoreCursor(editorElement);
    //      done = true;
  } catch (ex) {
    RestoreCursor(editorElement);
    if (ex.result == compsample.needvars) {
      var o = {};
      o.title = GetComputeString("Divide.title");
      o.mParentWin = this;
      o.theMath = math;
      o.vars = vars;
      o.theCommand = cmd;
      o.theCommandHandler = cmdHandler;
      o.afterDialog = function(editorElement) {
        if (this.Cancel) return;
        this.mParentWin.doComputeDivide(this.theMath, this.vars, editorElement, this.theCommand, this.theCommandHandler);
      };
      try {
        var theDialog = msiOpenModelessDialog("chrome://prince/content/ComputeVariables.xul", "_blank", "chrome,close,titlebar,resizable,modal", editorElement, cmd, cmdHandler, o);
      } catch (e) {
        AlertWithTitle("Error in computeOverlay.js", "Exception in doComputeDivide: [" + e + "]");
        return;
      }

      if (o.Cancel) return;
      vars = runFixup(o.vars);
    } else {
      msiComputeLogger.Exception(ex);
    }
  }
  //  }
}

function doComputeRoots(math, vars, editorElement, cmd, cmdHandler) {
  if (!editorElement) editorElement = msiGetActiveEditorElement();

  var mathstr = GetFixedMath(math);
  if (!vars) vars = "";

  msiComputeLogger.Sent4("roots", mathstr, "specifying", vars);
  try {
    ComputeCursor(editorElement);
    var out = GetCurrentEngine().roots(mathstr, vars);
    msiComputeLogger.Received(out);
    appendLabeledResult(out, GetComputeString("Roots.fmt"), math, editorElement);
    RestoreCursor(editorElement);
    done = true;
  } catch (ex) {
    RestoreCursor(editorElement);
    if (ex.result == compsample.needvars) {
      var o = {};
      o.title = GetComputeString("Companion.title");
      o.mParentWin = this;
      o.theMath = math;
      o.vars = vars;
      o.theCommand = cmd;
      o.theCommandHandler = cmdHandler;
      o.afterDialog = function(editorElement) {
        if (this.Cancel) return;
        this.mParentWin.doComputeRoots(this.theMath, this.vars, editorElement, this.theCommand, this.theCommandHandler);
      };
      try {
        var theDialog = msiOpenModelessDialog("chrome://prince/content/ComputeVariables.xul", "_blank", "chrome,close,titlebar,resizable,dependent", editorElement, cmd, cmdHandler, o);
      } catch (e) {
        AlertWithTitle("Error in computeOverlay.js", "Exception in doComputeRoots: [" + e + "]");
        return;
      }

    } else {
      msiComputeLogger.Exception(ex);
    }
  }
}

function doComputeSort(math, vars, editorElement, cmd, cmdHandler) {
  if (!editorElement) editorElement = msiGetActiveEditorElement();
  if (!vars) vars = "";
  var mathstr = GetFixedMath(math);
  msiComputeLogger.Sent4("sort", mathstr, "specifying", vars);
  try {
    ComputeCursor(editorElement);
    var out = GetCurrentEngine().sort(mathstr, vars);
    msiComputeLogger.Received(out);
    appendResult(out, "<mo>=</mo>", math, editorElement);
    RestoreCursor(editorElement);
  } catch (ex) {
    RestoreCursor(editorElement);
    if (ex.result == compsample.needvars) {
      var o = {};
      o.title = GetComputeString("Sort.title");
      o.mParentWin = this;
      o.theMath = math;
      o.vars = vars;
      o.theCommand = cmd;
      o.theCommandHandler = cmdHandler;
      o.afterDialog = function(editorElement) {
        if (this.Cancel) return;
        this.mParentWin.doComputeSort(this.theMath, this.vars, editorElement, this.theCommand, this.theCommandHandler);
      };
      try {
        var theDialog = msiOpenModelessDialog("chrome://prince/content/ComputeVariables.xul", "_blank", "chrome,close,titlebar,resizable,dependent", editorElement, cmd, cmdHandler, o);
      } catch (e) {
        AlertWithTitle("Error in computeOverlay.js", "Exception in doComputeSort: [" + e + "]");
        return;
      }

      //      var parentWin = msiGetParentWindowForNewDialog(editorElement);
      //      parentWin.openDialog("chrome://prince/content/ComputeVariables.xul", "computevariables", "chrome,close,titlebar,modal", o);
      //        if (o.Cancel)
      //          return;
      //        vars = runFixup(o.vars);
      //      } else {
      //        msiComputeLogger.Exception(ex);
    }
  }
}

function doComputeCompanionMatrix(math, vars, editorElement, cmd, cmdHandler) {
  if (!editorElement) editorElement = msiGetActiveEditorElement();
  var mathstr = GetFixedMath(math);
  if (!vars) vars = "";

  msiComputeLogger.Sent4("companion matrix", mathstr, "specifying", vars);
  try {
    ComputeCursor(editorElement);
    var out = GetCurrentEngine().companionMatrix(mathstr, vars);
    msiComputeLogger.Received(out);
    appendLabeledResult(out, GetComputeString("Companion.fmt"), math, editorElement);
    //appendResult(out,"<mo>=</mo>",math, editorElement);
    RestoreCursor(editorElement);
    done = true;
  } catch (ex) {
    RestoreCursor(editorElement);
    if (ex.result == compsample.needvars) {
      var o = {};
      o.title = GetComputeString("Companion.title");
      o.mParentWin = this;
      o.theMath = math;
      o.vars = vars;
      o.theCommand = cmd;
      o.theCommandHandler = cmdHandler;
      o.afterDialog = function(editorElement) {
        if (this.Cancel) return;
        this.mParentWin.doComputeCompanionMatrix(this.theMath, this.vars, editorElement, this.theCommand, this.theCommandHandler);
      };
      try {
        var theDialog = msiOpenModelessDialog("chrome://prince/content/ComputeVariables.xul", "_blank", "chrome,close,titlebar,resizable,dependent", editorElement, cmd, cmdHandler, o);
      } catch (e) {
        AlertWithTitle("Error in computeOverlay.js", "Exception in doComputeCompanionMatrix: [" + e + "]");
        return;
      }

      //        var parentWin = msiGetParentWindowForNewDialog(editorElement);
      //        parentWin.openDialog("chrome://prince/content/ComputeVariables.xul", "computevariables", "chrome,close,titlebar,modal", o);
      //        if (o.Cancel)
      //          return;
      //        vars = runFixup(o.vars);
    } else {
      msiComputeLogger.Exception(ex);
    }
  }
}

function doComputeApproxIntegral(math, editorElement) {
  if (!editorElement) editorElement = msiGetActiveEditorElement();
  var o = {};
  o.intervals = 10;
  o.form = 1;
  o.lowerBound = 0;
  o.upperBound = 5;

  var parentWin = msiGetParentWindowForNewDialog(editorElement);
  parentWin.openDialog("chrome://prince/content/ComputeApproxIntegral.xul", "approxint", "chrome,close,titlebar,modal,resizable", o);
  if (o.Cancel) {
    return;
  } else {
    msiGetEditor(editorElement).incrementModificationCount(1);
  }
  var intervals = GetNumAsMathML(o.intervals);
  var mathstr = GetFixedMath(math);
  msiComputeLogger.Sent4("approximate integral", mathstr, o.form, intervals);

  ComputeCursor(editorElement);
  try {
    var out = GetCurrentEngine().approxIntegral(mathstr, o.form, intervals, o.lowerBound, o.upperBound);
    msiComputeLogger.Received(out);
    appendLabeledResult(out, GetComputeString("ApproximateInt.fmt"), math, editorElement); // SWP labels with type
  } catch (ex) {
    msiComputeLogger.Exception(ex);
  }
  RestoreCursor(editorElement);
}

function doComputeIterate(editorElement, cmdHandler) {
  var o = {};
  o.title = GetComputeString("Iterate.title");
  o.fieldcount = 3;
  o.prompt = new Array(o.fieldcount);
  o.initialvalue = new Array(o.fieldcount);
  o.prompt[0] = GetComputeString("Iterate.fnprompt");
  o.initialvalue[0] = GetComputeString("Iterate.fndefault");
  o.prompt[1] = GetComputeString("Iterate.centerprompt");
  o.initialvalue[1] = GetComputeString("Iterate.centerdefault");
  o.prompt[2] = GetComputeString("Iterate.terms");
  o.initialvalue[2] = GetComputeString("Iterate.termsdefault");

  var parentWin = msiGetParentWindowForNewDialog(editorElement);
  try {
    msiOpenModelessDialog("chrome://prince/content/ComputeMathMLArgDialog.xul", "_blank", "chrome,close,titlebar,resizable,dependent", editorElement, "cmd_compute_Iterate", cmdHandler, o);
  } catch (e) {
    AlertWithTitle("Error in computeOverlay.js", "Exception in doComputeIterate: [" + e + "]");
    return;
  }
  //  parentWin.openDialog("chrome://prince/content/ComputeMathMLArgDialog.xul", "mathmlarg",
  //                    "chrome,close,titlebar,modal", o);
}

function finishComputeIterate(editorElement, o) {
  if (o.Cancel) return;

  ComputeCursor(editorElement);
  var fn = runFixup(o.mathresult[0]);
  var center = runFixup(o.mathresult[1]);
  var order = runFixup(o.mathresult[2]);
  msiComputeLogger.Sent4("iterate", fn, center, order);

  ComputeCursor(editorElement);
  try {
    var out = GetCurrentEngine().iterate(fn, center, order);
    msiComputeLogger.Received(out);
    addResult(out, editorElement);
  } catch (ex) {
    msiComputeLogger.Exception(ex);
  }
  RestoreCursor(editorElement);
}

function doComputeImplicitDiff(math, editorElement, cmdHandler) {
  if (!editorElement) editorElement = msiGetActiveEditorElement();
  //  var mathstr = GetFixedMath(math);
  var vars = "";
  var o = {};
  o.theMath = math;
  //  var parentWin = msiGetParentWindowForNewDialog(editorElement);
  //  parentWin.openDialog("chrome://prince/content/ComputeImplicitDiff.xul", "implicitdiff", "chrome,close,titlebar,modal", o);
  try {
    msiOpenModelessDialog("chrome://prince/content/ComputeImplicitDiff.xul", "_blank", "chrome,close,titlebar,dependent,resizable", editorElement, "cmd_MSIComputeImplicitDiff", cmdHandler, o);
  } catch (e) {
    AlertWithTitle("Error in computeOverlay.js", "Exception in doComputeImplicitDiff: [" + e + "]");
    return;
  }
}

function finishComputeImplicitDiff(math, editorElement, o) {
  if (o.Cancel) return;
  var mathstr = GetFixedMath(math);
  msiComputeLogger.Sent4("implicit diff", mathstr, o.thevar, o.about);

  ComputeCursor(editorElement);
  try {
    var out = GetCurrentEngine().implicitDiff(mathstr, o.thevar, o.about);
    msiComputeLogger.Received(out);
    appendLabeledResult(out, GetComputeString("Solution.fmt"), math, editorElement);
  } catch (ex) {
    msiComputeLogger.Exception(ex);
  }
  RestoreCursor(editorElement);
}

function doComputeSolveODEExact(math, labelID, titleID, vars, editorElement, cmd, cmdHandler) {
  var mathstr = GetFixedMath(math);
  if (!editorElement) editorElement = msiGetActiveEditorElement();
  if (!vars) vars = "";
  msiComputeLogger.Sent4(labelID, mathstr, "specifying", vars);
  try {
    ComputeCursor(editorElement);
    var eng = GetCurrentEngine();
    var out = eng.solveODEExact(mathstr, vars);
    msiComputeLogger.Received(out);
    appendLabeledResult(out, GetComputeString(labelID), math, editorElement);
    RestoreCursor(editorElement);
  } catch (ex) {
    RestoreCursor(editorElement);
    if (ex.result == compsample.nosol) {
      appendLabel(GetComputeString("NoSolution"), math, editorElement);
      done = true;
    } else if (ex.result == compsample.needivars) {
      var o = {};
      o.title = GetComputeString(titleID);
      o.label = GetComputeString("ODE.label");
      o.mParentWin = this;
      o.theMath = math;
      o.theLabelID = labelID;
      o.theTitleID = titleID;
      o.vars = vars;
      o.theCommand = cmd;
      o.theCommandHandler = cmdHandler;
      o.afterDialog = function(editorElement) {
        if (this.Cancel) return;
        this.mParentWin.doComputeSolveODEExact(this.theMath, this.theLabelID, this.theTitleID, this.vars, editorElement, this.theCommand, this.theCommandHandler);
      };
      try {
        var theDialog = msiOpenModelessDialog("chrome://prince/content/ComputeVariables.xul", "_blank", "chrome,close,titlebar,resizable,dependent", editorElement, cmd, cmdHandler, o);
      } catch (e) {
        AlertWithTitle("Error in computeOverlay.js", "Exception in doComputeSolveODEExact: [" + e + "]");
        return;
      }

    } else {
      msiComputeLogger.Exception(ex);
    }
  }
}

function doComputeSolveODELaplace(math, labelID, titleID, vars, editorElement, cmd, cmdHandler) {
  var mathstr = GetFixedMath(math);
  if (!editorElement) editorElement = msiGetActiveEditorElement();
  if (!vars) vars = "";
  msiComputeLogger.Sent4(labelID, mathstr, "specifying", vars);
  try {
    ComputeCursor(editorElement);
    //var out = func(mathstr,vars);
    var eng = GetCurrentEngine();
    var out = eng.solveODELaplace(mathstr, vars);
    //var out = func(mathstr, vars);
    msiComputeLogger.Received(out);
    appendLabeledResult(out, GetComputeString(labelID), math, editorElement);
    RestoreCursor(editorElement);
  } catch (ex) {
    RestoreCursor(editorElement);
    if (ex.result == compsample.nosol) {
      appendLabel(GetComputeString("NoSolution"), math, editorElement);
      done = true;
    } else if (ex.result == compsample.needivars) {
      var o = {};
      o.title = GetComputeString(titleID);
      o.label = GetComputeString("ODE.label");
      o.mParentWin = this;
      o.theMath = math;
      o.theLabelID = labelID;
      o.theTitleID = titleID;
      o.vars = vars;
      o.theCommand = cmd;
      o.theCommandHandler = cmdHandler;
      o.afterDialog = function(editorElement) {
        if (this.Cancel) return;
        this.mParentWin.doComputeSolveODELaplace(this.theMath, this.theLabelID, this.theTitleID, this.vars, editorElement, this.theCommand, this.theCommandHandler);
      };
      try {
        var theDialog = msiOpenModelessDialog("chrome://prince/content/ComputeVariables.xul", "_blank", "chrome,close,titlebar,resizable,dependent", editorElement, cmd, cmdHandler, o);
      } catch (e) {
        AlertWithTitle("Error in computeOverlay.js", "Exception in doComputeSolveODE: [" + e + "]");
        return;
      }

    } else {
      msiComputeLogger.Exception(ex);
    }
  }
}

function doComputeSolveODENumeric(math, labelID, titleID, vars, editorElement, cmd, cmdHandler) {
  var mathstr = GetFixedMath(math);

  if (!editorElement) editorElement = msiGetActiveEditorElement();

  if (!vars) vars = "";

  msiComputeLogger.Sent4(labelID, mathstr, "specifying", vars);
  try {
    ComputeCursor(editorElement);
    eng = GetCurrentEngine();
    var out = eng.solveODENumeric(mathstr, vars);
    msiComputeLogger.Received(out);
    appendLabeledResult(out, GetComputeString(labelID), math, editorElement);
    RestoreCursor(editorElement);
  } catch (ex) {
    RestoreCursor(editorElement);
    if (ex.result == compsample.nosol) {
      appendLabel(GetComputeString("NoSolution"), math, editorElement);
      done = true;
    } else if (ex.result == compsample.needivars) {
      var o = {};
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
      o.afterDialog = function(editorElement) {
        if (this.Cancel) return;
        this.mParentWin.doComputeSolveODE(this.theMath, this.theLabelID, this.theFunc, this.theTitleID, this.vars, editorElement, this.theCommand, this.theCommandHandler);
      };
      try {
        var theDialog = msiOpenModelessDialog("chrome://prince/content/ComputeVariables.xul", "_blank", "chrome,close,titlebar,resizable,dependent", editorElement, cmd, cmdHandler, o);
      } catch (e) {
        AlertWithTitle("Error in computeOverlay.js", "Exception in doComputeSolveODE: [" + e + "]");
        return;
      }

    } else {
      msiComputeLogger.Exception(ex);
    }
  }
}

function doComputeSolveODESeries(math, editorElement) {
  if (!editorElement) editorElement = msiGetActiveEditorElement();
  var o = {};
  o.order = "5"; // sticky? But really, should be general content
  o.title = GetComputeString("ODESeries.title");
  o.fieldcount = 3;
  o.prompt = new Array(o.fieldcount);
  o.initialvalue = new Array(o.fieldcount);
  o.prompt[0] = GetComputeString("PowerSeries.varprompt");
  o.initialvalue[0] = GetComputeString("PowerSeries.vardefault");
  o.prompt[1] = GetComputeString("PowerSeries.centerprompt");
  o.initialvalue[1] = GetComputeString("PowerSeries.centerdefault");
  o.prompt[2] = GetComputeString("PowerSeries.termsprompt");
  o.initialvalue[2] = GetComputeString("PowerSeries.termsdefault");
  o.theMath = math;
  o.mathresult = new Array(3);

  var parentWin = msiGetParentWindowForNewDialog(editorElement);
  parentWin.openDialog("chrome://prince/content/ComputePowerSeriesArgDialog.xul", "powerseries", "chrome,close,titlebar,modal,resizable", o);
  if (o.Cancel) return;
  else {
    msiGetEditor(editorElement).incrementModificationCount(1);
  }
  var mathstr = GetFixedMath(math);

  var variable = runFixup(o.mathresult[0]);
  var center = runFixup(o.mathresult[1]);
  var order = runFixup(o.mathresult[2]);

  ComputeCursor(editorElement);
  msiComputeLogger.Sent4("Solve ODE Power Series ", mathstr, variable + " @ " + center, order);

  try {
    var out = GetCurrentEngine().solveODESeries(mathstr, variable, center, order);
    msiComputeLogger.Received(out);
    appendLabeledResult(out, GetComputeString("ODESeries.fmt"), math, editorElement);
  } catch (e) {
    msiComputeLogger.Exception(e);
  }
  RestoreCursor(editorElement);
}

function doComputePowerSeries(math, editorElement, cmdHandler) {
  if (!editorElement) editorElement = msiGetActiveEditorElement();
  var o = {};
  o.title = GetComputeString("PowerSeries.title");
  o.fieldcount = 3;
  o.prompt = new Array(o.fieldcount);
  o.initialvalue = new Array(o.fieldcount);
  o.prompt[0] = GetComputeString("PowerSeries.varprompt");
  o.initialvalue[0] = GetComputeString("PowerSeries.vardefault");
  o.prompt[1] = GetComputeString("PowerSeries.centerprompt");
  o.initialvalue[1] = GetComputeString("PowerSeries.centerdefault");
  o.prompt[2] = GetComputeString("PowerSeries.termsprompt");
  o.initialvalue[2] = GetComputeString("PowerSeries.termsdefault");
  o.theMath = math;
  o.mathresult = new Array(3);

  var parentWin = msiGetParentWindowForNewDialog(editorElement);
  try {
    parentWin = msiGetParentWindowForNewDialog(editorElement);

    parentWin.openDialog("chrome://prince/content/ComputePowerSeriesArgDialog.xul", "powerseries", "chrome,close,titlebar,modal,resizable", o);

    if (o.Cancel) return;
    else {
      msiGetEditor(editorElement).incrementModificationCount(1);
    }

    finishComputePowerSeries(editorElement, o);

  } catch (e) {
    AlertWithTitle("Error in computeOverlay.js", "Exception in doComputePowerSeries: [" + e + "]");
    return;
  }

}

function finishComputePowerSeries(editorElement, o) {

  var mathstr = runFixup(GetMathAsString(GetRHS(o.theMath)));

  ComputeCursor(editorElement);
  var variable = runFixup(o.mathresult[0]);
  var center = runFixup(o.mathresult[1]);
  var order = runFixup(o.mathresult[2]);
  msiComputeLogger.Sent4("Power Series ", mathstr, variable + " @ " + center, order);
  try {
    var out = GetCurrentEngine().powerSeries(mathstr, variable, center, order);
    msiComputeLogger.Received(out);
    appendLabeledResult(out, GetComputeString("Series.fmt"), o.theMath, editorElement);
  } catch (e) {
    msiComputeLogger.Exception(e);
  }
  RestoreCursor(editorElement);
}

// vector calculus


function doComputeWronskian(math, vars, editorElement, cmd, cmdHandler) {
  if (!editorElement) editorElement = msiGetActiveEditorElement();
  var mathstr = GetFixedMath(math);
  if (!vars) vars = "";
  msiComputeLogger.Sent4("Wronskian", mathstr, "specifying", vars);
  try {
    ComputeCursor(editorElement);
    var out = GetCurrentEngine().wronskian(mathstr, vars);
    msiComputeLogger.Received(out);
    appendLabeledResult(out, GetComputeString("Wronskian.fmt"), math, editorElement);
    RestoreCursor(editorElement);
  } catch (ex) {
    RestoreCursor(editorElement);
    if (ex.result == compsample.needvars) {
      var o = {};
      o.title = GetComputeString("Wronskian.title");
      o.mParentWin = this;
      o.theMath = math;
      o.vars = vars;
      o.theCommand = cmd;
      o.theCommandHandler = cmdHandler;
      o.afterDialog = function(editorElement) {
        if (this.Cancel) return;
        this.mParentWin.doComputeWronskian(this.theMath, this.vars, editorElement, this.theCommand, this.theCommandHandler);
      };
      try {
        var theDialog = msiOpenModelessDialog("chrome://prince/content/ComputeVariables.xul", "_blank", "chrome,close,titlebar,resizable,dependent", editorElement, cmd, cmdHandler, o);
      } catch (e) {
        AlertWithTitle("Error in computeOverlay.js", "Exception in doComputeWronskian: [" + e + "]");
        return;
      }

      //      var parentWin = msiGetParentWindowForNewDialog(editorElement);
      //      parentWin.openDialog("chrome://prince/content/ComputeVariables.xul", "computevars", "chrome,close,titlebar,modal", o);
      //      if (o.Cancel)
      //        return;
      //      vars = runFixup(o.vars);
    } else {
      msiComputeLogger.Exception(ex);
    }
  }
}

// matrix commands


function doComputeCharPoly(math, vars, editorElement, cmd, cmdHandler) {
  if (!editorElement) editorElement = msiGetActiveEditorElement();
  var mathstr = GetFixedMath(math);
  if (!vars) vars = "";
  msiComputeLogger.Sent4("characteristic polynomial", mathstr, "specifying", vars);
  try {
    ComputeCursor(editorElement);
    var out = GetCurrentEngine().characteristicPolynomial(mathstr, vars);
    msiComputeLogger.Received(out);
    appendLabeledResult(out, GetComputeString("CharPolynomial.fmt"), math, editorElement);
    RestoreCursor(editorElement);
  } catch (ex) {
    RestoreCursor(editorElement);
    if (ex.result == compsample.needvars) {
      var o = {};
      o.title = GetComputeString("Charpoly.title");
      o.mParentWin = this;
      o.theMath = math;
      o.vars = vars;
      o.theCommand = cmd;
      o.theCommandHandler = cmdHandler;
      o.afterDialog = function(editorElement) {
        if (this.Cancel) return;
        this.mParentWin.doComputeCharPoly(this.theMath, this.vars, editorElement, this.theCommand, this.theCommandHandler);
      };
      try {
        var theDialog = msiOpenModelessDialog("chrome://prince/content/ComputeVariables.xul", "_blank", "chrome,close,titlebar,resizable,dependent", editorElement, cmd, cmdHandler, o);
      } catch (e) {
        AlertWithTitle("Error in computeOverlay.js", "Exception in doComputeCharPoly: [" + e + "]");
        return;
      }

      //      var parentWin = msiGetParentWindowForNewDialog(editorElement);
      //      parentWin.openDialog("chrome://prince/content/ComputeVariables.xul", "computevars", "chrome,close,titlebar,modal", o);
      //      if (o.Cancel)
      //        return;
      //      vars = runFixup(o.vars);
    } else {
      msiComputeLogger.Exception(ex);
    }
  }
}

function doComputeFillMatrix(editorElement, cmdHandler) {
  if (!editorElement) editorElement = msiGetActiveEditorElement();
  var o = {};
  var parentWin = msiGetParentWindowForNewDialog(editorElement);
  try {
    msiOpenModelessDialog("chrome://prince/content/ComputeFillMatrix.xul", "_blank", "chrome,close,titlebar,resizable,dependent", editorElement, "cmd_MSIComputeFillMatrix", cmdHandler, o);
  } catch (e) {
    AlertWithTitle("Error in computeOverlay.js", "Exception in doComputeFillMatrix: [" + e + "]");
    return;
  }
  //  parentWin.openDialog("chrome://prince/content/ComputeFillMatrix.xul", "fillmatrix", "chrome,close,titlebar,modal", o);
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

function doComputeMap(math, editorElement, cmd, cmdHandler) {
  if (!editorElement) editorElement = msiGetActiveEditorElement();
  var o = {};
  o.title = GetComputeString("Map.title");
  o.label = GetComputeString("Map.label");
  o.mParentWin = this;
  o.theMath = math;
  o.afterDialog = function(editorElement) {
    if (this.Cancel) return;
    this.mParentWin.finishComputeMap(editorElement, this);
  };
  try {
    var theDialog = msiOpenModelessDialog("chrome://prince/content/ComputeVariables.xul", "_blank", "chrome,close,titlebar,resizable,dependent", editorElement, cmd, cmdHandler, o);
  } catch (e) {
    AlertWithTitle("Error in computeOverlay.js", "Exception in doComputeMap: [" + e + "]");
    return;
  }

  //  var parentWin = msiGetParentWindowForNewDialog(editorElement);
  //  parentWin.openDialog("chrome://prince/content/ComputeVariables.xul", "computervars", "chrome,close,titlebar,modal", o);
}

function finishComputeMap(editorElement, o) {
  if (o.Cancel) return;
  var mathstr = GetFixedMath(GetRHS(o.theMath));
  msiComputeLogger.Sent4("Map ", mathstr, " @ ", o.vars);

  ComputeCursor(editorElement);
  try {
    var out = GetCurrentEngine().map(mathstr, o.vars);
    msiComputeLogger.Received(out);
    appendLabeledResult(out, GetComputeString("Map.fmt"), o.theMath, editorElement);
  } catch (e) {
    msiComputeLogger.Exception(e);
  }
  RestoreCursor(editorElement);
}

function doComputeMinPoly(math, vars, editorElement, cmd, cmdHandler) {
  if (!editorElement) editorElement = msiGetActiveEditorElement();
  var mathstr = GetFixedMath(math);
  if (!vars) vars = "";
  msiComputeLogger.Sent4("minimal polynomial", mathstr, "specifying", vars);
  try {
    ComputeCursor(editorElement);
    var out = GetCurrentEngine().minimalPolynomial(mathstr, vars);
    msiComputeLogger.Received(out);
    label = " Minimal Polynomial: ";
    appendLabeledResult(out, GetComputeString("MinPolynomial.fmt"), math, editorElement);
    RestoreCursor(editorElement);
  } catch (ex) {
    RestoreCursor(editorElement);
    if (ex.result == compsample.needvars) {
      var o = {};
      o.title = GetComputeString("Minpoly.title");
      o.mParentWin = this;
      o.theMath = math;
      o.vars = vars;
      o.theCommand = cmd;
      o.theCommandHandler = cmdHandler;
      o.afterDialog = function(editorElement) {
        if (this.Cancel) return;
        this.mParentWin.doComputeMinPoly(this.theMath, this.vars, editorElement, this.theCommand, this.theCommandHandler);
      };
      try {
        var theDialog = msiOpenModelessDialog("chrome://prince/content/ComputeVariables.xul", "_blank", "chrome,close,titlebar,resizable,dependent", editorElement, cmd, cmdHandler, o);
      } catch (e) {
        AlertWithTitle("Error in computeOverlay.js", "Exception in doComputeMinPoly: [" + e + "]");
        return;
      }

      //      var parentWin = msiGetParentWindowForNewDialog(editorElement);
      //      parentWin.openDialog("chrome://prince/content/ComputeVariables.xul", "computevars", "chrome,close,titlebar,modal", o);
      //      if (o.Cancel)
      //        return;
      //      vars = runFixup(o.vars);
    } else {
      msiComputeLogger.Exception(ex);
    }
  }
}

function doComputeRandomMatrix(editorElement) {
  if (!editorElement) editorElement = msiGetActiveEditorElement();
  var o = {};
  o.rows = "3"; // sticky?
  o.cols = "3";
  o.min = "-9";
  o.max = "9";
  var parentWin = msiGetParentWindowForNewDialog(editorElement);
  parentWin.openDialog("chrome://prince/content/ComputeRandomMatrix.xul", "randommatrix", "chrome,close,titlebar,resizable,modal", o);
  if (o.Cancel) return;
  else {
    msiGetEditor(editorElement).incrementModificationCount(1);
  }
  var mRows = GetNumAsMathML(o.rows);
  var mCols = GetNumAsMathML(o.cols);
  var mMin = GetNumAsMathML(o.min);
  var mMax = GetNumAsMathML(o.max);
  msiComputeLogger.Sent4("random matrix", mRows + " by " + mCols, mMin + " to " + mMax, o.type);

  ComputeCursor(editorElement);
  try {
    var out = GetCurrentEngine().matrixRandom(o.type, mRows, mCols, mMin, mMax);
    msiComputeLogger.Received(out);
    addResult(out, editorElement);
  } catch (e) {
    msiComputeLogger.Exception(e);
  }
  RestoreCursor(editorElement);
}

function doComputeReshape(math, editorElement) {
  if (!editorElement) editorElement = msiGetActiveEditorElement();
  var o = {};
  o.ncols = "2"; // sticky?
  var parentWin = msiGetParentWindowForNewDialog(editorElement);
  parentWin.openDialog("chrome://prince/content/ComputeReshape.xul", "reshape", "chrome,close,titlebar,modal,resizable", o);
  if (o.Cancel) return;
  else {
    msiGetEditor(editorElement).incrementModificationCount(1);
  }
  var mCols = GetNumAsMathML(o.ncols);
  var mathstr = GetFixedMath(GetRHS(math));
  msiComputeLogger.Sent4("reshape", mathstr, "with cols", mCols);

  ComputeCursor(editorElement);
  try {
    var out = GetCurrentEngine().matrixReshape(mathstr, mCols);
    msiComputeLogger.Received(out);
    appendLabeledResult(out, GetComputeString("Reshape.fmt"), math, editorElement);
  } catch (e) {
    msiComputeLogger.Exception(e);
  }
  RestoreCursor(editorElement);
}

function doComputeFitCurve(math, editorElement) {
  if (!editorElement) editorElement = msiGetActiveEditorElement();
  var o = {};
  o.code = 1;
  o.column = 1;
  o.degree = 2;
  var parentWin = msiGetParentWindowForNewDialog(editorElement);
  parentWin.openDialog("chrome://prince/content/ComputeFitCurve.xul", "fitcurve", "chrome,close,titlebar,modal,resizable", o);
  if (o.Cancel) return;
  else {
    msiGetEditor(editorElement).incrementModificationCount(1);
  }
  var mathstr = GetFixedMath(math);
  msiComputeLogger.Sent4("Fit Curve @ ", mathstr, o.code, o.column + "/" + o.degree);
  var label;
  if (o.code == 2) label = GetComputeString("FitCurvePolynomial.fmt");
  else label = GetComputeString("FitCurveRegression.fmt");

  ComputeCursor(editorElement);
  try {
    var out = GetCurrentEngine().fitCurve(mathstr, o.code, o.column, o.degree);
    msiComputeLogger.Received(out);
    appendLabeledResult(out, label, math, editorElement);
  } catch (ex) {
    msiComputeLogger.Exception(ex);
  }
  RestoreCursor(editorElement);
}

function doComputeRandomNumbers(editorElement) {
  if (!editorElement) editorElement = msiGetActiveEditorElement();
  var o = {};
  // JLF - tally should have intially setting, then retain user input
  // o.tally      = "10";  // sticky?
  o.dist = 0;
  o.param1 = "";
  o.param2 = "";
  var parentWin = msiGetParentWindowForNewDialog(editorElement);
  parentWin.openDialog("chrome://prince/content/ComputeRandomNumbers.xul", "randomnumbers", "chrome,close,titlebar,modal,resizable", o);
  if (o.Cancel) return;
  else {
    msiGetEditor(editorElement).incrementModificationCount(1);
  }
  var mTally = GetNumAsMathML(o.tally);
  var mParam1 = GetNumAsMathML(o.param1);
  var mParam2 = GetNumAsMathML(o.param2);
  msiComputeLogger.Sent4("random numbers", o.dist, mTally, mParam1 + " to " + mParam2);

  ComputeCursor(editorElement);
  try {
    var out = GetCurrentEngine().randomNumbers(o.dist, mTally, mParam1, mParam2);
    msiComputeLogger.Received(out);
    addResult(out, editorElement);
  } catch (e) {
    msiComputeLogger.Exception(e);
  }
  RestoreCursor(editorElement);
}

function doComputeMoment(math, editorElement, cmdHandler) {
  if (!editorElement) editorElement = msiGetActiveEditorElement();
  var o = {};
  //  o.title = GetComputeString("Moment.title");
  //  o.label = GetComputeString("Moment.label");
  //  o.remark = GetComputeString("Moment.remark");
  o.theMath = math;
  var parentWin = msiGetParentWindowForNewDialog(editorElement);
  try {
    msiOpenModelessDialog("chrome://prince/content/ComputeMoment.xul", "_blank", "chrome,close,titlebar,resizable,dependent", editorElement, "cmd_MSIComputeMoment", cmdHandler, o);
  } catch (e) {
    AlertWithTitle("Error in computeOverlay.js", "Exception in doComputeMoment: [" + e + "]");
    return;
  }

  //  parentWin.openDialog("chrome://prince/content/ComputeMoment.xul", "moment", "chrome,close,titlebar,modal", o);
}

function finishComputeMoment(editorElement, o) {
  if (o.Cancel) return;
  var num = o.thevar;
  var origin = o.about;
  var meanisorigin = o.meanisorigin;
  var mathstr = GetFixedMath(o.theMath);
  msiComputeLogger.Sent4("Moment ", mathstr, origin + " @ ", num + " / " + meanisorigin);

  ComputeCursor(editorElement);
  try {
    var out = GetCurrentEngine().moment(mathstr, num, origin, meanisorigin);
    msiComputeLogger.Received(out);
    appendLabeledResult(out, GetComputeString("Moment.fmt"), o.theMath, editorElement);
  } catch (ex) {
    msiComputeLogger.Exception(ex);
  }
  RestoreCursor(editorElement);
}

function doComputeQuantile(math, editorElement, cmd, cmdHandler) {
  if (!editorElement) editorElement = msiGetActiveEditorElement();
  var o = {};
  o.title = GetComputeString("Quantile.title");
  o.label = GetComputeString("Quantile.label");
  o.mParentWin = this;
  o.theMath = math;
  o.afterDialog = function(editorElement) {
    if (this.Cancel) return;
    this.mParentWin.finishComputeQuantile(editorElement, this);
  };
  try {
    var theDialog = msiOpenModelessDialog("chrome://prince/content/ComputeVariables.xul", "_blank", "chrome,close,titlebar,resizable,dependent", editorElement, cmd, cmdHandler, o);
  } catch (e) {
    AlertWithTitle("Error in computeOverlay.js", "Exception in doComputeQuantile: [" + e + "]");
    return;
  }

  //  var parentWin = msiGetParentWindowForNewDialog(editorElement);
  //  parentWin.openDialog("chrome://prince/content/ComputeVariables.xul", "computevars", "chrome,close,titlebar,modal", o);
}

function finishComputeQuantile(editorElement, o) {
  if (o.Cancel) return;
  var quantile = o.vars;
  var mathstr = GetFixedMath(o.theMath);
  msiComputeLogger.Sent4("Quantile", mathstr, "specifying", quantile);

  ComputeCursor(editorElement);
  try {
    var out = GetCurrentEngine().quantile(mathstr, quantile);
    msiComputeLogger.Received(out);
    appendLabeledResult(out, GetComputeString("Quantile.fmt"), o.theMath, editorElement);
  } catch (ex) {
    msiComputeLogger.Exception(ex);
  }
  RestoreCursor(editorElement);
}

function doComputeDefine(math, editorElement) {
  if (!editorElement) editorElement = msiGetActiveEditorElement();
  var mathstr = GetFixedMath(math);

  var subscript = "";
  var done = false;
  while (!done) {
    msiComputeLogger.Sent4("define", mathstr, "specifying", subscript);
    try {
      ComputeCursor(editorElement);
      var out = GetCurrentEngine().define(mathstr, subscript);
      msiComputeLogger.Received(out);
      // there is no result
      done = true;
    } catch (ex) {
      RestoreCursor(editorElement);
      if (ex.result == compsample.needsubinterp) {
        var o = {};
        var parentWin = msiGetParentWindowForNewDialog(editorElement);
        parentWin.openDialog("chrome://prince/content/ComputeInterpretSubscript.xul", "interpretsub", "chrome,close,titlebar,modal,resizable", o);
        if (o.Cancel) {
          return;
        } else {
          msiGetEditor(editorElement).incrementModificationCount(1);
        }
        subscript = o.subscript;
      } else {
        msiComputeLogger.Exception(ex);
        return;
      }
    }
  }

  RestoreCursor(editorElement);
}

function doComputeUndefine(math) {
  ComputeCursor();
  var mathstr = GetFixedMath(math);
  msiComputeLogger.Sent("undefine", mathstr);
  try {
    var out = GetCurrentEngine().undefine(mathstr);
    msiComputeLogger.Received(out);
    // there is no result
  } catch (e) {
    msiComputeLogger.Exception(e);
  }
  RestoreCursor();
}

function doComputeShowDefs(editorElement) {
  if (!editorElement) editorElement = msiGetActiveEditorElement();
  msiComputeLogger.Sent("show definitions", "");
  var out = GetCurrentEngine().getDefinitions();
  msiComputeLogger.Received(out);

  var o = {};
  o.val = out;
  var parentWin = msiGetParentWindowForNewDialog(editorElement);
  parentWin.openDialog("chrome://prince/content/ComputeShowDefs.xul", "showdefs", "chrome,close,titlebar,resizable,dependent", o);
}

function doComputeClearDefs() {
  msiComputeLogger.Sent("clear definitions", "");
  GetCurrentEngine().clearDefinitions();
}

function doComputeMapMuPADName(editorElement) {
  if (!editorElement) editorElement = msiGetActiveEditorElement();

  msiComputeLogger.Sent("Define MuPAD Name", "");

  var o = {};

  var parentWin = msiGetParentWindowForNewDialog(editorElement);
  parentWin.openDialog("chrome://prince/content/MapMuPADName.xul", "showdefs", "modal,chrome,close,titlebar,resizable,dependent", o);

  if (o.Cancel) return;
  else {
    msiGetEditor(editorElement).incrementModificationCount(1);
  }

  var swpname = o.swpname;
  var mupname = o.mupadname;
  var infile = o.infile;

  var eng = GetCurrentEngine();
  var res = eng.defineMupadName(swpname, mupname, infile);

}

function doComputeSetBasisVars(editorElement, cmd) {
  if (!editorElement) editorElement = msiGetActiveEditorElement();
  var compsample = GetCurrentEngine();

  msiComputeLogger.Sent("vector basis", "queried");

  var o = {};
  o.vars = runFixup(compsample.getVectorBasis());
  o.mParentWin = this;
  o.theEngine = compsample;
  o.afterDialog = function(editorElement) {
    if (this.Cancel) return;
    this.mParentWin.finishComputeSetBasisVars(this.vars, this.theEngine);
  };

  try {
    var theDialog = msiOpenModelessDialog("chrome://prince/content/ComputeVarList.xul", "_blank", "chrome,close,titlebar,resizable,dependent", editorElement, cmd, null, o);
  } catch (e) {
    AlertWithTitle("Error in computeOverlay.js", "Exception in doComputeMinPoly: [" + e + "]");
    return;
  }

  //  var parentWin = msiGetParentWindowForNewDialog(editorElement);
  //  parentWin.openDialog("chrome://prince/content/ComputeVarList.xul", "varlist", "chrome,close,titlebar,modal", o);
  //  if (o.Cancel) {
  //    return;
  //  }
}

function finishComputeSetBasisVars(vars, compsample) {
  compsample.setVectorBasis(vars);
  msiComputeLogger.Sent("new vector basis", vars);
}
//
// function doComputeSwitchEngines()
// {
//   var compsample = GetCurrentEngine();
//
//   var o = {};
//   o.engine      = compengine;
//   window.openDialog("chrome://prince/content/ComputeSwitchEngines.xul", "switchengines", "chrome,close,titlebar,modal", o);
//   if (o.Cancel)
//     return;
//
//   compengine = o.engine;
//   if (compengine == 1)
//     compsample.startup("mplInstall.gmr");
//   else
//     compsample.startup("mupInstall.gmr");
//   msiComputeLogger.Sent("Switching engine to",compengine);
// }

function doComputePassthru(editorElement) {
  if (!editorElement) editorElement = msiGetActiveEditorElement();

  var str = "";
  var element = null;
  var first = 0;
  var last = 0;
  var anchor;
  var focus;

  try {
    var selection = msiGetEditor(editorElement).selection;
    if (!selection) {
      dump("no selection!\n");
      return;
    }

    //element = findtagparent(selection.focusNode,"code");
    //if (!element) {
    //  dump("not in code tag!\n");
    //  return;
    //}
    //var text = element.firstChild;
    //if (text.nodeType != Node.TEXT_NODE) {
    //  dump("bad node structure.\n");
    //  return;
    //}
    //str = WrapInMtext(text.nodeValue);
    // Anchor and Focus the same?
    anchor = selection.anchorNode;
    focus = selection.focusNode;
    if (anchor != focus) {
      dump("\nanchor != focus in passtrhu\n");
      return;
    }

    var r = selection.getRangeAt(0);
    var editor = msiGetEditor(editorElement);
    var nodeArray = editor.nodesInRange(r);
    var enumerator = nodeArray.enumerate();
    var node = enumerator.getNext();
    var content = node.nodeValue;

    first = (selection.anchorOffset < selection.focusOffset) ? selection.anchorOffset : selection.focusOffset;
    last = (selection.anchorOffset < selection.focusOffset) ? selection.focusOffset : selection.anchorOffset;
    str = "<mtext>" + content.substr(first, last - first) + "</mtext>";

    msiComputeLogger.Sent("passthru", str);
  } catch (exc) {
    AlertWithTitle("Error in computeOverlay.js", "Exception in doComputePassthru; exception is [" + exc + "].");
    return;
  }

  ComputeCursor(editorElement);
  var compsample = GetCurrentEngine();
  var out;
  try {
    out = GetCurrentEngine().perform(str, compsample.PassThru);
    msiComputeLogger.Received(out);
  } catch (e) {
    msiComputeLogger.Exception(e);
  }

  //var child = element;
  //var node = child.parentNode;
  //while (node) {
  //  if (node.localName == "body")
  //    break;
  //  else {
  //    child = node;
  //    node = child.parentNode;
  //  }
  //}
  if (out) {
    //var idx;
    //for (idx = 0; idx < node.childNodes.length; idx++) {
    //  if (child == node.childNodes[idx])
    //    break;
    //}
    appendTaggedResult(out, GetComputeString("Passthru.fmt"), anchor, last, editorElement);
  }
  RestoreCursor(editorElement);
}

// "math" is the expression to plot
// "type" is the type from the menu.


function doComputePlot(math, dimension, type, animate, editorElement) {
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);
  var selection;
  if (editor)
    selection = editor.selection;
  insertNewGraph(math, dimension, type, animate, editorElement, selection);
}

function doEditPlot() {
  graphObjectClickEvent();
}

function checkVCamStatusForPlot(objElement, graph, editorElement)
{
  var graphPath, objPath;
  if (!objElement.vcamStatus)
    objElement.vcamStatus = "uninitialized";
  else if (objElement.vcamStatus == "initialized")
  {
    if (!objElement.addEvent)
      objElement.vcamStatus = "needRecreate";  //it was initialized but has become disconnected
  }
  if (!(objElement.vcamStatus == "needRecreate"))
  {
    graphPath = graph.getGraphAttribute("ImageFile");
    objPath = objElement.getAttribute("data");
    if (graphPath && objPath && isDifferentPlotFile(objPath, graphPath, editorElement))
    {
      if (objElement.load)
        objElement.vcamStatus = "needReload";
      else
        objElement.vcamStatus = "needRecreate";
        //the VCam scripting interface doesn't survive calling setAttribute("data") to change source file;
        //if we can't call the VCam load function, we should regenerate the object node
    }
  }
  return objElement.vcamStatus;
}

function regeneratePlotObject(objElement, graph, editorElement)
{
  var parent = objElement.parentNode;
//  var newObj = objElement.ownerDocument.createElementNS(xhtmlns, "object");
  var newObj = document.createElementNS(xhtmlns, "object");
//Intentionally not passing in an editorElement - don't want to use editor methods here
//In particular, this function shouldn't be used to do anything that should be undo-able;
//  we're just replacing a vcam <object> that has become disfunctional
  msiCopyElementAttributesExcluding(newObj, objElement, null, ["data","src","type"]);
  newObj.vcamStatus = "uninitialized";
  parent.replaceChild(newObj, objElement);
//NOTE!!! You must set the vcam source file in the object before setting its type, or we don't seem to be able to get
//  the scriptable vcam interface to work!
  newObj.setAttribute("data", msiMakeAbsoluteUrl(graph.getGraphAttribute("ImageFile"),editorElement));
  newObj.setAttribute("type", objElement.getAttribute("type"));
  return newObj;
}

var preInitializeVCamCallbackObjectBase = 
{
  mCount : 0,
  mInterval : 200,
  mAttempts : 10,
//  mStart : 0,

  start : function(interval, nAttempts/*, startAt*/ )
  {
    if (interval)
      this.mInterval = interval;
    if (nAttempts)
      this.mAttempts = nAttempts;
//    if (startAt)
//    {
//      this.mStart = startAt;
//      this.mAttempts += startAt;
//    }
    if (!this.mObj || !this.mGraph)
      throw("Calling preInitializeVCam with null object node or graph object");
    if (!this.mEditorElement)
      this.mEditorElement = msiGetActiveEditorElement();
//    if ((this.mStart > 0) || !this.tryOne())
    if (!this.tryOne())
    {
      //start the timer
      this.mTimer = Components.classes["@mozilla.org/timer;1"].createInstance(Components.interfaces.nsITimer);
      this.mTimer.initWithCallback( this, this.mInterval, Components.interfaces.nsITimer.TYPE_REPEATING_SLACK);
    }
  },
  tryOne : function()
  {
    if ("checkVCamStatusForPlot" in window)
      window.checkVCamStatusForPlot(this.mObj, this.mGraph, this.mEditorElement);
    if (this.mObj.vcamStatus == "initialized")
      return true;
    var bDone = false;
    try
    {
      bDone = this.doPreInitVCam();
    }
    catch(ex)
    {
      if (this.mbHandleException)
        this.handleException(ex);
      else
        msidump("Exception in preInitializeVCam: " + ex.message + " - continuing to try.\n");
    }
    return bDone;
  },
  doPreInitVCam : function()
  {
    msidump("Entering an attempt in preInitializeVCam\n");
    var editor = msiGetEditor(this.mEditorElement);
    var domGraph = editor.getElementOrParentByTagName("graph", this.mObj);
    var plotWrapper = this.mObj.parentNode;
    if (this.mObj.addEvent && (this.mObj.readyState === 2)) {
      this.mObj.addEvent('leftMouseDown', onVCamMouseDown);
      this.mObj.addEvent('leftMouseUp', onVCamMouseUp);
      this.mObj.addEvent('leftMouseDoubleClick', onVCamDblClick);
      this.mObj.addEvent('dragMove', onVCamDragMove);
      this.mObj.addEvent('dragLeave', (function() {}));
      if (domGraph) {
        this.mObj.addEvent('dragEnter', this.mGraph.provideDragEnterHandler(this.mEditorElement, domGraph));
        this.mObj.addEvent('drop', this.mGraph.provideDropHandler(this.mEditorElement, domGraph));
      }
      this.mObj.addEvent("rightMouseDown", onVCamRightMouseDown);
      this.mObj.addEvent("rightMouseUp", onVCamRightMouseUp);
      this.mObj.addEvent("keyDown", onVCamKeyDown);
      this.mObj.addEvent("keyUp", onVCamKeyUp);
//      this.mObj.addEvent('treeChange', onVCamTreeChange);
      queryVCamValues(this.mObj, this.mGraph, domGraph);
      // add a method for writing a snapshot
      var snapFn = function() {
        return doMakeSnapshot(this.mObj, this.mGraph, this.mEditorElement);
      };
      plotWrapper.wrappedObj = this.mObj;
      doMakeSnapshot(this.mObj, this.mGraph, this.mEditorElement);
      this.mObj.vcamStatus = "initialized";  //add a property so we know we've successfully initialized the VCam object
      msidump("Leaving a successful attempt in preInitializeVCam\n");
      return true;
    }
    msidump("Leaving an unsuccessful attempt in preInitializeVCam; readyState reports [" + this.mObj.readyState + "]\n");
    return false;
  },
  notify : function(aTimer)
  { 
    if (aTimer)
      this.mTimer = aTimer;
    ++this.mCount;
//    if ((this.mCount > this.mStart) && this.tryOne())
    if (this.tryOne())
    {
      this.mTimer.cancel();
      if (this.onSuccess)
        this.onSuccess(this);
    }
    else if (this.mCount >= this.mAttempts)
    {
      this.mTimer.cancel();
      if (this.mbHandleFail)
        this.handleFail();
    }
  },
  handleFail : function()
  {
    var objPath = this.mObj.getAttribute("data");
    var objFile = Components.classes["@mozilla.org/file/local;1"].
                     createInstance(Components.interfaces.nsILocalFile);
    objFile.initWithPath( makeFilePathAbsolute(objPath, this.mEditorElement));
    if (objFile.exists())
    {
      msidump("preInitializeVCam failed but file " + objPath + " exists. Regenerating object.\n");
      var newObj = regeneratePlotObject(this.mObj, this.mGraph, this.mEditorElement);
      preInitializeVCam( newObj, this.mGraph, this.mEditorElement, false, false, 200, 20/*, 5*/ );  //don't want to spawn any more attempts, so pass null handlers
    }
    else
      msidump("preInitializeVCam failed; file " + objPath + " doesn't exist.\n");
  },
  handleException : function(exc)
  {
    if (exc.message == "NPMethod called on non-NPObject wrapped JSObject!")
    {
      var parent = this.mObj.parentNode;
      var newObj = regeneratePlotObject(this.mObj, this.mGraph, this.mEditorElement);
      preInitializeVCam( newObj, this.mGraph, false, false );  //don't want to spawn any more attempts, so pass null handlers
      return true;  //to stop the current timer
    }
    msidump("Exception in preInitializeVCam: " + exc.message + " - continuing to try.\n");
    return false;
  }
};

function preInitializeVCamCallbackObject(graph, objElement, editorElement, bHandleFail, bHandleException)
{
  this.mGraph = graph;
  this.mObj = objElement;
  this.mEditorElement = editorElement;
  this.mbHandleFail = bHandleFail;
  this.mbHandleException = bHandleException;
}

preInitializeVCamCallbackObject.prototype = preInitializeVCamCallbackObjectBase;

function preInitializeVCam( objElement, theGraph, editorElement, bHandleFail, bHandleExceptions, useInterval, useIterations/*, delayStart*/ )
{
  if (!useInterval)
    useInterval = 200;
  if (!useIterations)
    useIterations = 10;
//  if (!delayStart)
//    delayStart = 0;
  if (theGraph && theGraph.plotFailed())
  {
    dump("Plot failed; not initializing VCam.\n");
    return;
  }
  var initializer = new preInitializeVCamCallbackObject(theGraph, objElement, editorElement, bHandleFail, bHandleExceptions);
  initializer.start(useInterval, useIterations/*, delayStart*/);
}

//Warning! This function may replace the <object> node for the plot, so you should use getElementsByTagName()
//again afterwards before referencing it. Furthermore, it calls tryUntilSuccessful() and so is asynchronous.
//It's meant to set a plot which has been created (perhaps via a paste or document load) or modified into a
//reliable state.
function ensureVCamPreinitForPlot(graphNode, editorElement)
{
  var editor = msiGetEditor(editorElement);
  var objElement = graphNode.getElementsByTagName("object");
  var theGraph;
  if (objElement && objElement.length)
    objElement = objElement[0];
  var parent, newObj;
  if (objElement)
  {
    theGraph = new Graph();
    theGraph.extractGraphAttributes(graphNode);
    checkVCamStatusForPlot(objElement, theGraph, editorElement);
    if (objElement.vcamStatus === "initialized")
      return;
    if ( (objElement.vcamStatus === "needRecreate") || (objElement.vcamStatus === "needReload") )
    {
      objElement = regeneratePlotObject(objElement, theGraph, editorElement);
//    }
//    else if (objElement.vcamStatus === "needReload")
//    {
//      var vcamUri = msiMakeAbsoluteUrl(theGraph.getGraphAttribute("ImageFile"),editorElement);
////      var vcamPath = msiPathFromFileURL(msiURIFromString(vcamUri));
//      msidump("In ensureVCamPreinitForPlot, calling vcam load [" + vcamUri + "]\n");
//      objElement.load(vcamUri);
//      objElement.setAttribute("data", vcamUri);
//      objElement.vcamStatus = "uninitialized";
    }
    preInitializeVCam( objElement, theGraph, editorElement, true, true);
  }
}

// form a single run of math but don't put caret on end


function coalescemath(editorElement, leaveCursorAsIs) {
  if (!editorElement) editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);
  var selection = editor.selection;
  if (selection) {
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
    var left = node_before(element);
    while (left) 
    {
      if (left.localName != "math") {
        left = null;
      }
      if (left) {
        editor.joinNodes(left, element, element.parentNode);
        // joinNodes merges the nodes and deletes element
        element = left;  // we do so so that element is the non-empty element.
        left = node_before(element);
      }
    }
    var right = node_after(element);
    while (right) 
    {
      if (right.localName != "math") {
        right = null;
      }
      if (right) {
        editor.joinNodes(element, right, right.parentNode);
        // joinNodes merges the nodes and deletes right
        right = node_after(element);
      }
    }
  }
  if (!leaveCursorAsIs) {
    editor.setCaretAfterElement(last_child(element));
  }
  return element;
}

function findtagparent(node, tag) {
  if (!node || node.localName == tag) return node;
  else return findtagparent(node.parentNode, tag);
}

function findmathparent(node) {
  return findtagparent(node, "math");
  // really, should check namespace, too
}
