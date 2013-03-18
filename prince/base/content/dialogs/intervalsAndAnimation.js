var graph;
var plot;
var plotData;
var dim = 3;
var numvars = 2;
var plotno = 0;
var isAnimated = false;
var plottype = "rectangular";
var animOffset = 0;
var inRoleChange = false;
var whichVars = [];
var rowData = {};
var xsltToTeXProcessor;
var editsReady = [];

var animAttributes = ["AnimCommonOrCustomSettings", "AnimateStart", "AnimateEnd",
                           "AnimateFPS", "AnimateVisBefore", "AnimateVisAfter"];

// This part pastes data into the editor after the editor has started. 
// implements nsIObserver
var minMaxDocumentObserverBase = {
  observe: function (aSubject, aTopic, aData)
  {
    if (aTopic === "obs_documentCreated")
    {
      editsReady.push(this.ctrlID);
      checkEditsReady();
    }
  }
};

function minMaxDocumentObserver(ctrl)
{
  this.ctrlID = ctrl.id;
}

minMaxDocumentObserver.prototype = minMaxDocumentObserverBase;

function checkEditsReady()
{
  var isReady = true;
  for (var aRowId in rowData)
  {
    if (editsReady.indexOf(rowData[aRowId].startEdit.id) < 0)
    {
      isReady = false;
      break;
    }
  }
  if (!isReady)
    msidump("Edits not yet ready in IntervalsAndAnimations dialog!\n");
  return isReady;
}

function startup(){
//  var graph = window.arguments[0];
  var graphData = window.arguments[0];
  var jj, menuId;
  graph = graphData.mGraph;
  if (graph)
  {
    plotno = graph["plotnumber"];
    plot = graph.plots[plotno];
    plotData = graphData.findPlotData(plot);
    dim = graph.getDimension();
    isAnimated = plot.attributes["Animate"];
    if (isAnimated)
      animOffset = 1;
    plottype = plot.attributes["PlotType"];
    numvars = plotVarsNeeded(dim, plottype, isAnimated);
  }

  xsltToTeXProcessor = setupXMLToTeXProcessor();

//  document.getElementById("twod").collapsed = (numVars < 1) || (isAnimated && (nVars < 2));
  document.getElementById("1Var").collapsed = (numvars < 1);
  document.getElementById("2Vars").collapsed = (numvars < 2);
  document.getElementById("3Vars").collapsed = (numvars < 3);
  document.getElementById("4Vars").collapsed = (numvars < 4);
  document.getElementById("isanimated").collapsed = !isAnimated;
  document.getElementById("tube").collapsed = (plottype != "tube");

  whichVars = [];
  if (numvars - animOffset > 0)
  {
    whichVars.push("X");
    if (numvars - animOffset > 1)
    {
      whichVars.push("Y");
      if (numvars - animOffset > 2)
      {
        whichVars.push("Z");
      }
    }
  }
  if (isAnimated)
    whichVars.push("Anim");

  var varIdentifier;
  var rowObj;
  for (jj = 0; jj < whichVars.length; ++jj)
  {
    varIdentifier = "var" + String(jj+1);
    menuId = varIdentifier + "Role";
    rowData[menuId] = { whichVar : whichVars[jj],
                          varName : document.getElementById(varIdentifier),
                          startEdit : document.getElementById(varIdentifier + "StartEdit"),
                          endEdit : document.getElementById(varIdentifier + "EndEdit"),
                          numPtsTextbox : document.getElementById("ptssamp" + String(jj+1)) };
  }

  inRoleChange = true;
  setVariableRoleStrings();
  var minVal, maxVal, ptsVal;
  var editorInitializer = new msiEditorArrayInitializer();
  for (var aRowId in rowData)
  {
    rowObj = rowData[aRowId];
    minVal = plot.getPlotValue(rowObj.whichVar + "Min");
    maxVal = plot.getPlotValue(rowObj.whichVar + "Max");
    rowObj.startEdit.mInitialDocObserver = [{mCommand : "obs_documentCreated", mObserver : minMaxDocumentObserver(rowObj.startEdit)}];
    rowObj.endEdit.mInitialDocObserver = [{mCommand : "obs_documentCreated", mObserver : minMaxDocumentObserver(rowObj.endEdit)}];
    editorInitializer.addEditorInfo(rowObj.startEdit, minVal, true);
    editorInitializer.addEditorInfo(rowObj.endEdit, maxVal, true);

    document.getElementById(aRowId).value = rowObj.whichVar;
    putMathMLExpressionToControl(rowObj.varName, plot.getPlotValue(rowObj.whichVar + "Var"));
//    putMathMLExpressionToControl(rowObj.startEdit, minVal);
//    putMathMLExpressionToControl(rowObj.endEdit, maxVal);
    rowObj.bDefaultStart = isDefaulted(minVal, rowObj.whichVar, "Min");
    rowObj.bDefaultEnd = isDefaulted(maxVal, rowObj.whichVar, "Max");
    if (rowObj.whichVar == "Anim")
    {
      rowObj.bDefaultNumPoints = true;
      rowObj.numPtsTextbox.hidden = true;
    }
    else
    {
      ptsVal = plot.getPlotValue(rowObj.whichVar + "Pts");
      putMathMLExpressionToControl(rowObj.numPtsTextbox, ptsVal);
      rowObj.bDefaultNumPoints = isDefaulted(ptsVal, rowObj.whichVar, "Pts");
    }
  }
  editorInitializer.doInitialize();

  if (plottype == "tube")
    putMathMLExpressionToControlByID("ptssampTubeRadius", plot.getPlotValue("TubeRadialPts"));
  if (isAnimated)
    putAnimationDataToDialog();
  inRoleChange = false;
}

function putAnimationDataToDialog()
{
  for (var ii = 0; ii < animAttributes.length; ++ii)
    putValueToControl(animAttributes[ii], plot.getPlotValue(animAttributes[ii]));
  changeCommonOrCustomAnimationSettings();
}

function getValuesFromDialog()
{
  var serialize = new XMLSerializer();

  if (plot)
  {
    var rowObj;
    for (var aRowId in rowData)
    {
      rowObj = rowData[aRowId];
      plot.setPlotValue( rowObj.whichVar + "Var", getMathMLExpressionFromControl(rowObj.varName, serialize) );
      plot.setPlotValue( rowObj.whichVar + "Min", getMathMLExpressionFromControl(rowObj.startEdit, serialize) );
      plot.setPlotValue( rowObj.whichVar + "Max", getMathMLExpressionFromControl(rowObj.endEdit, serialize) );
      plot.markUserSet(rowObj.whichVar + "Var", true);
      plot.markUserSet(rowObj.whichVar + "Min", true);
      plot.markUserSet(rowObj.whichVar + "Max", true);
      if (rowObj.whichVar !== "Anim")
      {
        plot.setPlotValue( rowObj.whichVar + "Pts", getMathMLExpressionFromControl(rowObj.numPtsTextbox, serialize) );
        plot.markUserSet(rowObj.whichVar + "Pts", true);
      }
    }
    if (plottype == "tube")
      plot.setPlotValue( "TubeRadialPts", getMathMLExpressionFromControlByID("ptssampTubeRadius", serialize) );
    if (isAnimated)
    {
      getAnimationDataFromDialog();
    }
  }
}

function getAnimationDataFromDialog()
{
  for (var ii = 0; ii < animAttributes.length; ++ii)
    plot.setPlotValue(animAttributes[ii], getValueFromControl(animAttributes[ii]));
  changeCommonOrCustomAnimationSettings();
}

function changeCommonOrCustomAnimationSettings()
{
  var currVal = document.getElementById("AnimCommonOrCustomSettings").value;
  document.getElementById("customAnimSettings").disabled = (currVal != "custom");
}

function onAccept() {
  getValuesFromDialog();
  return true;
}

function onCancel() {
  return true;
}

function onChangeRole(whichControl)
{
  if (inRoleChange)
    return;
  inRoleChange = true;

  var jj, currVar, menulist;
  var whichVariable = whichControl.id;
  var newVal = whichControl.value;
  var oldVal = rowData[whichVariable].whichVar;
//  var bIncludeAnim = ((newVal == "AnimVar") || (oldVal == "AnimVar"));
  var cycleVals = [];
  var newVar, foundIndex;
  var oldIndex = whichVars.indexOf(oldVal);
  var newIndex = whichVars.indexOf(newVal);
  try
  {
    if (newIndex < oldIndex)  //e.g. changing current YVar to XVar, so we need to shift to get YVar replaced
    {
      for (var jj = newIndex; jj <= oldIndex; ++jj)
        cycleVals.push(whichVars[jj]);  //so it should look like [XVar, YVar] or [XVar, YVar, ZVar]
      cycleVals.push(newVal);
    }
    else   //changing current XVar to AnimVar, so we need new XVar
    {
      for (jj = newIndex; jj >= oldIndex; --jj)
        cycleVals.push(whichVars[jj]);
      cycleVals.push(newVal);     //so it should look like [AnimVar, YVar, XVar, AnimVar]
    }
    for (var menuId in rowData)
    {
      currVar = rowData[menuId].whichVar;
      foundIndex = cycleVals.indexOf(currVar);
      if (foundIndex < 0)
        continue;
      menulist = document.getElementById(menuId);
      newVar = cycleVals[foundIndex + 1];  //this is okay, since the last element is same as the first, it'll show up with index 0
      menulist.value = newVar;
      checkDefaultsOnVarChange(currVar, newVar, rowData[menuId]);
      rowData[menuId].whichVar = newVar;
    }
  } catch(ex)
  { msidump("Error in intervalsAndAnimation.js, onChangeRole(): " + ex + "\n"); }

  inRoleChange = false; 
}

//If this variable was using default values for start, end, or num points, change them to defaults for new variable
function checkDefaultsOnVarChange(oldVar, newVar, rowObj)
{
  rowObj.bDefaultStart = rowObj.bDefaultStart && matchesDefault(rowObj.startEdit, oldVar, "Min");
  rowObj.bDefaultEnd = rowObj.bDefaultEnd && matchesDefault(rowObj.endEdit, oldVar, "Max");
  if (rowObj.bDefaultStart)
    putMathMLExpressionToControl(rowObj.startEdit, plot.getDefaultPlotValue(newVar + "Min"));
  if (rowObj.bDefaultEnd)
    putMathMLExpressionToControl(rowObj.endEdit, plot.getDefaultPlotValue(newVar + "Max"));

  rowObj.bDefaultNumPoints = rowObj.bDefaultNumPoints && matchesDefault(rowObj.numPtsTextbox, oldVar, "Pts");
  if (isAnimated)
    rowObj.numPtsTextbox.hidden = (newVar == "Anim");
  if (rowObj.bDefaultNumPoints && !rowObj.numPtsTextbox.hidden)
    putMathMLExpressionToControl(rowObj.numPtsTextbox, plot.getDefaultNumPlotPoints(newVar + "Pts"));
}

function matchesDefault(control, whichVar, key)
{
  if (plot.isUserSet(whichVar + key))
    return false;
  var ctrlVal = getMathMLExpressionFromControl(control, graph.ser);
  return isDefaulted(ctrlVal, whichVar, key);
}

function isDefaulted(val, whichVar, key)
{
  if (plot.isUserSet(whichVar + key))
    return false;
  var defVal;
  if (key === "Pts")
  {
    if (whichVar === "Anim")
      return true;
    defVal = plot.getDefaultNumPlotPoints(whichVar + key);
  }
  else
    defVal = plot.getDefaultPlotValue(whichVar + key);
  if (val === defVal)
    return true;
  var teXVal = plotData.parent.convertXMLFragToSimpleTeX(val);
  var defTeXVal = plotData.parent.convertXMLFragToSimpleTeX(defVal);
  return (teXVal === defTeXVal);
}

function setVariableRoleStrings()
{
  var computebundle = document.getElementById("computeBundle");
  var menulistIDs = ["var1Role", "var2Role", "var3Role", "var4Role"];
  var menulist, menuitem;
  var prefixStr = "";
  switch(plotData.plottype)
  {
    case "polar":
    case "cylindrical":
    case "spherical":
      prefixStr = plotData.plottype;
    break;
    default:
      if (plotData.isParametric())
        prefixStr = "parametric";
    break;
  }
  for (var ix = 0; ix < menulistIDs.length; ++ix)
  {
    menulist = document.getElementById(menulistIDs[ix]);
    if (menulist.hidden)
      continue;
    for (var jx = menulist.itemCount - 1; jx >= 0; --jx)
    {
      menuitem = menulist.getItemAtIndex(jx);
      if (whichVars.indexOf(menuitem.value) < 0)
        menulist.removeItemAt(jx);
      else
        menuitem.label = getMenuItemLabel(computebundle, prefixStr, menuitem.value + "Var");
    }
  }
}

function getMenuItemLabel(theBundle, prefixStr, varNameStr)
{
  var res="";
  var initialStr = "Intervals.";
  try
  {
    res = theBundle.getString(initialStr + prefixStr + varNameStr);
  }
  catch(exc)
  {
    try
    {
      res = theBundle.getString(initialStr + varNameStr);
    }
    catch(ex)
    {
      msidump("Problem in getting intervalsAndAnimation.js, getMenuItemLabel; unable to get string for " + initialStr + prefixStr + varNameStr + ".\n");
      res = "";
    }
  }
  return res;
}