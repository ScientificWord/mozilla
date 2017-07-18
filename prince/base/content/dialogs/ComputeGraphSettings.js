// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.
// Three data items are passed into this dialog:
// window.arguments[0]: editorElement
// window.arguments[1]: commandStr
// window.arguments[2]: DOMGraph, the DOM element that should be replaced

#include ../productname.inc //

Components.utils.import("resource://app/modules/unitHandler.jsm");
var gFrameModeImage = true;
var gFrameModeTextFrame = false;
var graph;
var frame;
var graphnode;
var plotwrapper;
var plotArray = [];
var gDefaultWidth = 200;
var gDefaultHeight = 100;
var gDefaultUnit = "pt";
var gInitialSrc = "";
var gHaveDocumentUrl = false;
var gOriginalSrcUrl = "";
var gSrcUrl;
var gPreviewImageWidth = 80;
var gPreviewImageHeight = 50;
var gDialog;
var gInsertNewObject = true;

function getFirstElementByTagName(node, name) {
  var  element = node.getElementsByTagName(name);
  if (element.length > 0) {
    element = element[0];
    return element;
  }
  return null;
}

// Populate the dialog with the current values stored in the Graph object.
// The element ids in ComputeGraphSettings.xul match the Graph attributes
// if the document has an element matching an attribute name,
//   extract the value of the attribute and put it in the document
function Startup(){
  var units, alist, id, i, plotNumControl, numPlots, firstActivePlot,
    plot, theStringSource, oldval, captionnode, placeLocation, obj, frame, topWindow;
  var graphEditorControl, radiusEditorControl;
  try {
    gDialog = {};
    graphnode = window.arguments[2];
    if (graphnode.nodeName !== 'graph')
      graphnode = findtagparent(graphnode, 'graph');
    var editorElement = window.arguments[0];
    graph = new Graph();

    // get graph attributes
    graph.extractGraphAttributes(graphnode);
    frame = getFirstElementByTagName(graphnode,"msiframe");
    obj = getFirstElementByTagName(graphnode, "object");
    if (obj && obj.hasAttribute("data")) gInsertNewObject = false;

    // get frame attributes
    // turn off inline, left, right, inside, outside options for plots
    document.getElementById('forplots').hidden = true;
    initFrameTab(gDialog, frame, gInsertNewObject, obj);

    topWindow = msiGetTopLevelWindow();
    if (topWindow && topWindow.document && topWindow.document.getElementById("vcamactive")
      ) //   && (topWindow.document.getElementById("vcamactive").getAttribute("hidden")=="false"))
      queryVCamValues(obj, graph, graphnode, true);

    units = frame.getAttribute("units");
    if (!units || units.length === 0)
    {
      units = "pt";
      frame.setAttribute("units") = units;
    }
    // setHasNaturalSize(false);
#ifndef PROD_SNB
    setCanRotate(false);
#endif
    document.getElementById("role-image").setAttribute("hidden",(gFrameModeImage?"false":"true"));

    alist = graph.graphAttributeList();
    for ( i=0; i<alist.length; i++) {
      id = mapid(alist[i]);
      if (document.getElementById(id)) {
        putValueToControlByID(id, graph.getValue(alist[i]));
//        document.getElementById(id).value = graph.getValue(alist[i]);
      }
    }

    plotNumControl    = document.getElementById('plotnumber');
    numPlots = graph.getNumActivePlots();  //don't count any that may be already deleted - though probably not relevant during startup
    if (numPlots ===  0){
      addPlot();
      numPlots = 1;
    }
    plotNumControl.max = numPlots;
    plotNumControl.valueNumber = 1;
    firstActivePlot = getPlotInternalNum(1);  //get first active plot
    graph.plotnumber = firstActivePlot.toString();
    plot = graph.plots[firstActivePlot];
    graph.currentDisplayedPlot = -1;  //initialize to -1 so that populateDialog will execute (so that currentDisplayedPlot != plotnum)
    // some attributes can't be found as values of dialog elements
//    setColorWell("baseColorWell", makeColorVal(plot.getPlotValue("BaseColor")));
//    setColorWell("secondColorWell", makeColorVal(plot.getPlotValue("SecondaryColor")));
//    setColorWell("lineColorWell", makeColorVal(plot.getPlotValue("LineColor")));
    try {
      alist = graph.frame.FRAMEATTRIBUTES;
      for ( i=0; i<alist.length; i++) {
        id = mapid(alist[i]);
        if (document.getElementById(id)) {
          if (graph.frame.getFrameAttribute(alist[i]))
          putValueToControlByID(id, graph.frame.getFrameAttribute(alist[i]));
  //        document.getElementById(id).value = graph.frame.getFrameAttribute(alist[i]);
        }
      }
    }
    catch(e) {
      msidump(e.message);
    }
    document.getElementById("defaultCameraCheckbox").checked = !graph.cameraValuesUserSet();
    document.getElementById("defaultviewintervals").checked = !graph.viewRangesUserSet();

    // initKeyList();
    initializePlotEditors(firstActivePlot);
    captionnode = getFirstElementByTagName(graphnode,"imagecaption");

    testUseSignificantDigits();
    // Caption placement
    document.getElementById("captionLocation").value = graph.CaptionPlace;
  //  checkEnableFloating();
  }
  catch(e) {
    msidump(e.message);
  }
}

function initializePlotEditors(plotnum, contentsOnly) {
  var theStringSource, graphEditorControl, radiusEditorControl, editorInitializer, ptype, editorInitializer;
  graphEditorControl = document.getElementById("plotDlg-content-frame");
  graphEditorControl.mInitialDocObserver = [{mCommand : "obs_documentCreated", mObserver : msiEditorDocumentObserverG}];
  graphEditorControl.mbSinglePara = true;
  graphEditorControl.mInitialContentListener = invisibleMathOpFilter;  //in plotDlgUtils.js
  theStringSource = graph.plots[plotnum].element.Expression;
  editorInitializer = new msiEditorArrayInitializer();
  editorInitializer.addEditorInfo(graphEditorControl, theStringSource, true);


  radiusEditorControl = document.getElementById("plotDlg-tube-radius");
  radiusEditorControl.mbSinglePara = true;
  radiusEditorControl.mInitialContentListener = invisibleMathOpFilter;  //in plotDlgUtils.js
  ptype = graph.getPlotValue ("PlotType", plotnum);
  if (ptype === "tube") {
    theStringSource = graph.plots[plotnum].getPlotValue("TubeRadius");
    if (!theStringSource || theStringSource.length === 0) {
      theStringSource = GetComputeString("Math.emptyForInput");
    }
  } else {
    theStringSource = GetComputeString("Math.emptyForInput");
  }
  editorInitializer.addEditorInfo(radiusEditorControl, theStringSource, true);
  editorInitializer.doInitialize();
}


// This part pastes data into the editor after the editor has started.
// implements nsIObserver
var msiEditorDocumentObserverG = {
  observe: function (aSubject, aTopic, aData) {
    var plotno;
    if (aTopic === "obs_documentCreated") {
       plotno = Number(graph.plotnumber);
      //      var editor = GetCurrentEditor();
      //      editor.addOverrideStyleSheet("chrome://editor/content/MathVarsDialog.css");
      populateDialog(plotno);
    }
  }
};

function mapid( graphattribute)
// because we are using general-purpose overlays, our scheme of matching graph attributes
// to dialog ids has some exceptions, which are corrected here
{
  switch (graphattribute)
  {
    case "Width"              :   return "frameWidthInput";
    case "Height"             :   return "frameHeightInput";
    case "Placement"          :   return "locationList";
    case "CaptionPlace"       :   return "captionLocation";
    case "Key"                :   return "keyInput";
    case "Dimension"          :   return "dimension";
    case "Float"              :   return "float";
    case "Units"              :   return "frameUnitMenulist";
    case "AxesType"           :   return "axistype";
    case "EqualScaling"       :   return "equalscale";
    case "BGColor"            :   return "plotCW";
    case "ViewingBoxXMin"     :   return "xrangelow";
    case "ViewingBoxXMax"     :   return "xrangehigh";
    case "ViewingBoxYMin"     :   return "yrangelow";
    case "ViewingBoxYMax"     :   return "yrangehigh";
    case "ViewingBoxZMin"     :   return "zrangelow";
    case "ViewingBoxZMax"     :   return "zrangehigh";

    //then to avoid the automatic placing of values where not appropriat  :
    case "plotnumber"         :   return "";

    default                   :   return graphattribute;
  }
}


function mapPlotID(plotAttribute, plotnum)
{
  var plotType = graph.getPlotValue("PlotType", plotnum);
  var plotDim = graph.getDimension();
  switch(plotAttribute)
  {
    case "BaseColor":                 return "baseColorWell";                   break;
    case "SecondaryColor":            return "secondColorWell";                 break;
    case "LineColor":                 return "lineColorWell";                   break;
    case "PlotType":                  return (plotDim == 3 ? "pt3d" : "pt2d");    break;
  }
  return plotAttribute;
}

// Extract the values from the dialog and store them in the data structure
// Only save values that are not the defaults
function OK() {
  var editorElement, changed, theWindow;
  editorElement = msiGetParentEditorElementForDialog(window);
  var editor = msiGetEditor(editorElement);
  GetValuesFromDialog();
  graph.reviseGraphDOMElement(graphnode, false, editorElement);
  graph.setGraphAttribute("returnvalue", "true");
  graph.recomputeVCamImage(editorElement, graphnode);
  initVCamObjects(editor.document);
  //  var editor = msiGetEditor(editorElement);
//   changed = true;
//   if (changed) {
//     graph.recomputeVCamImage(editorElement);
//   }
//   theWindow = window.opener;
//   if (!theWindow || !(theWindow.hasOwnProperty("nonmodalRecreateGraph"))) {
//     theWindow = msiGetTopLevelWindow();
//   }
//   try {
//     theWindow.nonmodalRecreateGraph(graph, window.arguments[2], editorElement);
//   }
//   catch (e) {}
//   var parentWindow = window.opener;
//   var data;
//   var obj = graphnode.getElementsByTagName("object");
//   if (obj && obj.length)
//   {
//     obj = obj[0];
//   }
// //     if (obj) {
// //       if (obj.wrappedJSObject) obj = obj.wrappedJSObject;
// //       try {
// //         data = graphnode.getElementsByTagName('graphSpec')[0].getAttribute('ImageFile');
// //         obj.setAttribute('data', data);
// // //        parentWindow.doVCamInitialize(obj);
// //       }
// //       catch(e)
// //       {}
// //     }
// //  }
// //  graph.setGraphAttribute("returnvalue", "true");
  return true;
}

// Extract the values from the dialog and store them in the data structure
// Only save values that are not the defaults
function GetValuesFromDialog(){
  // grab anything that's in the graph attribute list
  var editorElement = msiGetParentEditorElementForDialog(window);
  var editor = msiGetEditor(editorElement);
  var alist = graph.graphAttributeList();
  var anID, attr;
  var newval, oldval;
  var frame = graph.frame;
  for (var i=0; i<alist.length; i++) {
    anID = mapid(alist[i]);
    if (document.getElementById(anID)) {
      newval = getValueFromControlByID(anID);
      if (newval !== "undefined" && newval !== "unspecified") {   // NOTE, "" is OK, e.g. delete a label
        oldval = graph.getValue (alist[i]);
        if (newval != oldval) {
           graph.setGraphAttribute(alist[i], newval);
        }
      }
    }
  }
  graph.markCameraValuesUserSet( !document.getElementById("defaultCameraCheckbox").checked );
  graph.markViewRangesUserSet( !document.getElementById("defaultviewintervals").checked );

  // grab anything that's in the plot attribute list
  // we save data for only the currently displayed plot, since the others are
  // already saved.
  var plotno = Number(graph.getGraphAttribute("plotnumber"));
  var plot = graph.plots[plotno];
  if (!plot) return;
  var alist  = plot.plotAttributeList();
  var dim    = graph.getDimension();
  for (var i=0; i<alist.length; i++) {
    anID = mapPlotID(alist[i], plotno);
    if (document.getElementById(anID)) {
      var newval = getValueFromControlByID(anID);
//      var newval = document.getElementById(anID).value;
      if (newval && (newval != "") && (newval != "undefined")) {
        var oldval = graph.getPlotValue (alist[i], plotno);
        if (newval != oldval) {
           graph.setPlotValue(alist[i], plotno, newval);
        }
      }
    }
  }
  var tempFrame = graphnode.getElementsByTagName("msiframe");
  if (tempFrame.length > 0) {
    tempFrame = tempFrame[0];
  }
  var f = tempFrame;  //.cloneNode(true);
  setFrameAttributes(tempFrame, msiNavigationUtils.getFirstSignificantChild(tempFrame), editor);  // reuse code in msiFrameOverlay.
  frame.extractFrameAttributes(tempFrame);
  var oldpt = plot.attributes["PlotType"];
  var newpt;
  if (dim == 3) {
    newpt = document.getElementById("pt3d").value;
  } else {
    newpt = document.getElementById("pt2d").value;
  }
  if (newpt != oldpt) {
    if (newpt) plot.attributes["PlotType"] = newpt;
    plot.attributes["PlotStatus"] = "New";
  }
  var oldanimate = (plot.attributes["Animate"] === "true");
  var newanimate = document.getElementById("animate").checked;
  if (oldanimate != newanimate)
    plot.attributes["Animate"] = (newanimate ? "true" : "false");
  try
  {
    var doc = document.getElementById("plotDlg-content-frame").contentDocument;
    var mathnode = doc.getElementsByTagName("math")[0];
    if (mathnode) {
      newval = graph.ser.serializeToString(mathnode);
      oldval = plot.element["Expression"];
      if (oldval != newval)
      {
        plot.element["Expression"] = newval;
        plot.attributes["PlotStatus"] = "New";
      }
    }
    if (newpt == "tube")
    {
      doc = document.getElementById("plotDlg-tube-radius").contentDocument;
      mathnode = doc.getElementsByTagName("math")[0];
      if (mathnode) {
        newval = graph.ser.serializeToString(mathnode);
        oldval = plot.element["TubeRadius"];
        if (newval != oldval)
        {
          plot.element["TubeRadius"] = newval;  //should we mark plot as new for this? naaah
        };
      }
    }
  }
  catch (e)
  {
    msidump("Error: " + e.message + "\n");
  }

  // if new, query
  var status = plot.attributes["PlotStatus"];
  if (status == "New")
     plot.computeQuery();


  // Layout Tab
  // printAttribute
  if (document.getElementById("printattr")) {
    newval = document.getElementById("printattr").selectedItem.value;
    oldval = graph.getValue ("PrintAttribute");
    if (newval != oldval) {
      graph.setGraphAttribute ("PrintAttribute", newval);
    }
  }
  // ScreenDisplay
  if (document.getElementById("screendisplayattr")) {
    newval = document.getElementById("screendisplayattr").selectedItem.value;
    oldval = graph.getValue ("PrintFrame");
    if (newval != oldval) {
      graph.setGraphAttribute ("PrintFrame", newval);
    }
  }
  // GraphAxesType
  if (document.getElementById("placementRadioGroup")) {
    var placement = document.getElementById("placementRadioGroup").selectedItem;
    var newval = placement ? placement.id : "";
    var oldval = graph.getValue ("Placement");
    if (newval != oldval) {
      graph.setGraphAttribute ("Placement", newval);
    }
  }
  // Labelling Tab
  // Captionplace
  if (document.getElementById("CaptionPlace")) {
    newval = document.getElementById("CaptionPlace").value;
    graph.setGraphAttribute ("CaptionPlace", newval);
  }
}

// for conformals, save the horizontal and vertical samples
// row is a table row with four elements
function SaveConformalSamples (name, row, plotno) {
  try {
    var cols = row.getElementsByTagName ("td");
    var newval = ExtractTextFromNode(cols[3]);
    var oldval = graph.getPlotValue (name, plotno);
    if (oldval != newval) {
      graph.setPlotValue(name, plotno, newval);
    }
  }
  catch (e) {
    alert ("ERROR: Unable to save values in plot table\n");
  }
}

function ExtractTextFromNode (node) {
  if (node.nodeType == 3)
    return node.nodeValue;
  return (ExtractTextFromNode (node.childNodes[0]));
}

function Cancel(){
  graph.setGraphAttribute("returnvalue", "false");
}

// This is the callback for the command button to add a new plot
// Create a new plot, fire up the ComputePlotSettings.xul edit dialog.
// Set the PlotStatus to New so OK can call the preparePlot function
function addPlot () {
  // save any changes to current plot, then change plots. Cancel ignores all changes
  try
  {
    GetValuesFromDialog();
    graph.setGraphAttribute("returnvalue", "false");
    addPlotDialogContents();
  }
  catch(e){
    msidump(e.message+"\n");
  }
}

function plotValuesToCopy(oldplot)
{
  var copyAttrs = oldplot.plotAttributeList().concat(copyAttrs);
  copyAttrs = attributeArrayRemove(copyAttrs, "PlotStatus");
  copyAttrs = copyAttrs.concat(oldplot.plotElementList());
  copyAttrs = attributeArrayRemove(copyAttrs, "Expression");
  copyAttrs = attributeArrayRemove(copyAttrs, "TubeRadius");
  return copyAttrs;
}

function addPlotDialogContents () {
  var firstPlotNum = getPlotInternalNum(1);
  var ptype = graph.getPlotValue("PlotType", firstPlotNum);
  if (!ptype)
    ptype = "Rectangular";
  var plot = new Plot(graph.getDimension(), ptype);
  graph.addPlot(plot);
  var plotnum = graph.getNumActivePlots();
  graph.setGraphAttribute("plotnumber", String(getPlotInternalNum(plotnum)));
  var plotNumControl  = document.getElementById('plotnumber');
  plotNumControl.max = plotnum;
  plotNumControl.valueNumber = plotnum;
//  var newElement = document.createElement('menuitem');
//  newElement.setAttribute("label", plotnumber.toString());
//  newElement.setAttribute("value", plotnumber.toString());
//  popup.appendChild(newElement);
//  document.getElementById("plot").selectedItem = newElement;
  // grab the plottype from plot 1 and set it as default
  plotnum = getPlotInternalNum(plotnum);
  var copyAttrs;
  if (plot.attributes["PlotType"] == "")
    plot.attributes["PlotType"] = "rectangular";
  plot.attributes["PlotStatus"] = "New";
  firstPlotNum = getPlotInternalNum(1);
  if (firstPlotNum != plotnum)  //should also copy some other attributes
  {
    copyAttrs = plotValuesToCopy(graph.plots[firstPlotNum]);
    plot.copyAttributes(graph.plots[firstPlotNum], copyAttrs);
  }
  populateDialog (plotnum);
}

function fromDialogToPlot( plotnum ) {
  var editorControl = document.getElementById("plotDlg-content-frame"),
    doc = editorControl.contentDocument,
    math = doc.getElementsByTagName("math")[0],
    radiusControl = document.getElementById("plotDlg-tube-radius"),
    radiusStringSource, radiusMath, radiusEditor,
    editor = msiGetEditor(editorControl),
    doit = true,
    ptype="Unknown",
    alist = graph.plotAttributeList(plotnum),
    anID, theVal, str;

  removeInvisibles(math);

  var theStringSource = graph.ser.serializeToString(math),
  ptype = graph.getPlotValue ("PlotType", plotnum);
  for (var i = 0; i < alist.length; ++i)
  {
    anID = mapPlotID(alist[i], plotnum);
    if (document.getElementById(anID)) {
      graph.setPlotValue(alist[i], plotnum, getValueFromControlByID(anID));
    }
  }



  graph.plots[plotnum].element["Expression"]= runFixup(theStringSource);
  radiusEditor = msiGetEditor(radiusControl);
  radiusMath = radiusControl.contentDocument.getElementsByTagName("math")[0];
  if (ptype === "tube")
  {
    radiusStringSource = graph.ser.serializeToString(radiusMath);
    graph.plots[plotnum].element["TubeRadius"] = radiusStringSource;
  }

}


function replaceMathNodeWithText(editor, math, source) {
  if (!math) return;
  var mathParent = math.parentNode, mathOffset
  if (mathParent) {
    mathOffset = offsetOfChild(mathParent, math);
    mathParent.removeChild(math);
    insertXML( editor, '<dummy>'+source+'</dummy>', mathParent, mathOffset, false);
  }
}

// plotnum is the number of an *existing* plot
function fromPlotToDialog( plotnum ) {
  var ptype = graph.getPlotValue ("PlotType", plotnum),
    editorControl = document.getElementById("plotDlg-content-frame"),
    doc = editorControl.contentDocument,
    editor = msiGetEditor(editorControl),
    math = doc.getElementsByTagName("math")[0], // math is presumably just an input box initially.
    dim = graph.getDimension(),
    animatevalue = graph.getPlotValue ("Animate",plotnum),
    ptype = graph.getPlotValue ("PlotType", plotnum),
    aiMethod = (ptype === "approximateIntegral") ? graph.getPlotValue("AIMethod", plotnum) : "",
    plotml = document.getElementById("plotnumber"),
    alist = graph.plotAttributeList(plotnum),
    anID, theVal, i, math, mathparent, mathoffset, theStringSource,
    radiusControl = document.getElementById("plotDlg-tube-radius"),
    radiusStringSource, radiusMath, radiusEditor;

  graph.currentDisplayedPlot = plotnum;
  graph["plotnumber"] = String(plotnum);
  theStringSource = graph.plots[plotnum].element["Expression"];
  replaceMathNodeWithText(editor, math, theStringSource);
   // Still need to do this for tube plot radius

  if (ptype == "tube")
  {
    radiusStringSource = graph.plots[plotnum].getPlotValue("TubeRadius");
    if (!radiusStringSource || radiusStringSource.length === 0) {
      radiusStringSource = GetComputeString("Math.emptyForInput");
    }
    radiusEditor = msiGetEditor(radiusControl);
  }
  try
  {
    // handle the top descriptor
    populateDescription ("dimension", String(dim));
    hideShowControls(dim, ptype, graph.isAnimated(), aiMethod)

    if (dim === 3) {
      populatePopupMenu ("pt3d", ptype);
    } else {
      populatePopupMenu ("pt2d", ptype);
    }
    document.getElementById("animate").checked = (animatevalue === "true");
    plotml.valueNumber = getActivePlotNumber(plotnum);
    for (i = 0; i < alist.length; ++i)
    {
      anID = mapPlotID(alist[i], plotnum);
      if (document.getElementById(anID)) {
        theVal = graph.getPlotValue (alist[i], plotnum);
        putValueToControlByID(anID, theVal);
      }
    }
  }
  catch (e)
  {
    msidump("fromPlotToDialog: " + e.message + "\n");
  }
}

// Delete a plot and set the dialog to the next available plot
function deletePlot () {
  // extract the plot number from the dialog
  var newplotno, numplots, plotno, plotNumControl;
  plotNumControl    = document.getElementById('plotnumber');
//  plotno = document.getElementById("plot").selectedItem.value;
  plotno = Number(plotNumControl.value);
  var intPlotNum = getPlotInternalNum(plotno);
  graph.deletePlot(intPlotNum);
  // find the next plot if there is one, or the previous plot if there is one
  numplots = graph.getNumActivePlots();  //don't count the deleted ones
  plotNumControl.max = (numplots ? numplots : 1);
  newplotno = intPlotNum;
  if (newplotno >= numplots) {
    newplotno = numplots - 1;
  }
  if (newplotno < 0) {   // no undeleted plots, add one.
    addPlotDialogContents();
  } else {               // use the next plot
    newplotno = getPlotInternalNum(newplotno);
    graph.setGraphAttribute("plotnumber", String(newplotno));
    graph.currentDisplayedPlot = -1; //to avoid copying plot attributes from the plot being deleted
    populateDialog (newplotno);
  }
}

// the user has just selected a new plot type for this plot. Rebuild the
// dialog
function changePlotType () {
  // get the plot number and the plot type. Then populate
  var dim    = graph.getDimension();
  var plotno = getPlotInternalNum(document.getElementById("plotnumber").valueNumber);
  if (!plotno)
    plotno = 0;
  var oldpt = graph.getPlotValue ("PlotType", plotno);
//  if ((plotno == null) || (plotno == "")) {
//  }
  graph.setGraphAttribute("plotnumber", String(plotno));
  var newpt;
  if (dim == 3) {
    newpt = document.getElementById("pt3d").value;
  } else {
    newpt = document.getElementById("pt2d").value;
  }
  if (newpt != oldpt) {
    graph.setPlotValue ("PlotType", plotno, newpt);
    graph.setPlotValue ("PlotStatus", plotno, "New");
  }
  populateDialog (plotno);
}

// the user has just selected a new plot number in the dialog: populate the
// current screen with the data for this plot
function changePlot () {
  // save any changes to this plot, then change plots. Cancel ignores all changes
  GetValuesFromDialog();
  graph.setGraphAttribute("returnvalue", "false");

  // extract the plot number from the dialog
  var plotno = document.getElementById("plotnumber").value;
  if ((plotno == null) || (plotno == "")) {
    plotno = 1;
  }
  plotno = getPlotInternalNum(plotno);
  graph.setGraphAttribute("plotnumber", String(plotno));
  populateDialog (plotno);
}

function getPlotInternalNum(activePlotNum)
{
  var numplots = graph.getNumPlots();
  var nFound = 0;
  for (var n = 0; n < numplots; ++n)
  {
    if (graph.plots[n].attributes["PlotStatus"] != "Deleted")
      ++nFound;
    if (nFound == activePlotNum)
      return n;
  }
  return 0;
}

function getActivePlotNumber(internalPlotNum)
// skips over deleted plots and converts from 0-based to 1-based
{
  var numplots = graph.getNumPlots();
  var nFound = 0;
  for (var n = 0; n < numplots; ++n)
  {
    if (graph.plots[n].attributes["PlotStatus"] != "Deleted")
      ++nFound;
    if (n == internalPlotNum)
      return (nFound > 0) ? nFound : 1;
  }
  //if we didn't get that far, return the number of non-deleted plots (ordinal of the last one)
  return (nFound > 0) ? nFound : 1;
}

function populateDialog (plotno) {
  // remove contents here
  try
  {
    var oldplotno = graph.currentDisplayedPlot;
    if ((plotno == oldplotno) || (plotno < 0) || plotno >= graph.getNumPlots()) return;
    if (oldplotno >= 0) {
      fromDialogToPlot(oldplotno);
    }
    fromPlotToDialog(plotno);
  }
  catch(e) {
    msidump(e.message);
  }
}

function populatePopupMenu (popupname, datavalue) {
  var popup  = document.getElementById (popupname);
  var items  = popup.getElementsByTagName ("menuitem");
  popup.selectedItem = items[0];
  for(var i=0; i<items.length; i++) {
    if (items[i].value == datavalue) {
      popup.selectedItem = items[i];
      break;
    }
  }
}

function populateDescription (name, datavalue) {
  var elem   = document.getElementById (name);
  if (!elem) dump("populateDescription failed, name = "+name+"\n");
  elem.value = datavalue;
}

// set the radio group identified by elemID to the element with curval
function radioGroupSetCurrent (elemID, oldval) {
  var elem = document.getElementById(elemID);
  if (elem) {
    elem.selectedIndex=0;
    var children = elem.childNodes;
    for (var i=0; i<children.length; i++) {
      if (oldval == children[i].value) {
	      elem.selectedIndex = i;
	    }
    }
  }
}


function initKeyList()
{
  gDialog.markerList = new msiKeyMarkerList(window);
  gDialog.markerList.setUpTextBoxControl(document.getElementById("keyInput"));
}

function tagConflicts()
{
  var keyList = document.getElementById("keylist");
  if (keyList.controller && (keyList.controller.searchStatus === nsIAutoCompleteController.STATUS_COMPLETE_MATCH))
    //"new" entry matches an existing key
  {
    document.getElementById("uniquekeywarning").hidden = false;
    //      msiPostDialogMessage("dlgErrors.markerInUse", {markerString : gDialog.keyInput.value});
    keyList.focus();
    return true;
  }
  return false;
}

function getNodeChildrenAsString(aNode)
{
  var retStr = "";
  var serializer = new XMLSerializer();
  var jx
  var nodeKids = msiNavigationUtils.getSignificantContents(aNode);
  for ( jx = 0; jx < nodeKids.length; ++jx) {
    retStr += serializer.serializeToString(nodeKids[jx]);
  }
  return retStr;
}

//Translate a CSS color to a 6 (or 8?) hex digit value
function hideShowControls(dim, ptype, graphAnimated, aiMethod)
{
  var showFillColors = 0;
  var showLineColors = 1;
  var bUseMesh = false;
  var bUseDirShading = false;
  var bAnimated = graph.isAnimated();
  if (dim == 3) {
    document.getElementById("plotcs3d").collapsed = false;
    document.getElementById("plotcs2d").collapsed = true;
    document.getElementById("threeDim").collapsed = false;
    document.getElementById("colorAlphaEnabled").setAttribute("hasAlpha", "true");
    bUseDirShading = true;
    bUseMesh = true;
    showFillColors = 2;
    showLineColors = 0;
    switch(ptype)
    {
      case "curve":
        showLineColors = 1;
        showFillColors = 0;
        bUseMesh = false;
      break;
      case "vectorField":
      case "gradient":
        bUseMesh = false;
      break;
    }
  } else {
    document.getElementById("plotcs2d").collapsed = false;
    document.getElementById("plotcs3d").collapsed = true;
    document.getElementById("threeDim").collapsed = true;
    document.getElementById("colorAlphaEnabled").setAttribute("hasAlpha", "false");
    switch(ptype)
    {
      case "inequality":            showFillColors = 2;   break;
      case "conformal":             showLineColors = 2;   break;
      case "approximateIntegral":
        switch(aiMethod)
        {
          case "LeftRight":
          case "LowerUpper":
          case "LowerUpperAbs":
            showFillColors = 2;
          break;
          default:
            showFillColors = 1;
          break;
        }
      break;
    }
  }
  document.getElementById("animated").hidden = !graphAnimated;
  document.getElementById("tubeOrLinePts").setAttribute( "selectedIndex", ((ptype ==="tube") ? 1 : 0) );
//  document.getElementById("tubeOrLinePtsDeck").selectedIndex = (ptype ==="tube") ? 1 : 0;
  document.getElementById("enableLinesAndPoints").collapsed = (showLineColors < 1);
  document.getElementById("use1LineColor").collapsed = (showLineColors < 1);
  document.getElementById("use2LineColors").collapsed = (showLineColors < 2);
  document.getElementById("use1FillColor").collapsed = (showFillColors < 1);
  document.getElementById("use2FillColors").collapsed = (showFillColors < 2);
  document.getElementById("useDirectionalShading").collapsed = !bUseDirShading;
  document.getElementById("useBaseColor").collapsed = (showFillColors < 1) && (showLineColors < 2);
  document.getElementById("useMesh").collapsed = !bUseMesh;
  document.getElementById("approxIntPlot").collapsed = (ptype !== "approximateIntegral");
  document.getElementById("useAreaFill").collapsed = ((ptype != "approximateIntegral") && (ptype !== "inequality"));
  document.getElementById("useDiscAdjust").collapsed = ((ptype != "rectangular") && (ptype != "parametric") && (ptype != "implicit"));
  changeDefaultCamera();  //this just enables or disables according to the checkbox state
  changeDefaultViewIntervals();  //this just enables or disables according to the checkbox state
}

function openVariablesAndIntervalsDlg()
{
  var graphData = new graphVarData(graph);
  openDialog('chrome://prince/content/intervalsAndAnimation.xul', 'plotintervalsandanimation', 'chrome,close,titlebar,modal,resizable', graphData);
}

function openAnimationSettingsDlg()
{
  openDialog('chrome://prince/content/plotAnimationSettings.xul', 'animationsettings', 'chrome,close,titlebar,modal,resizable', graph);
}

function openAxisFontSettingsDlg()
{
  var mapAttrs = {face : "AxisFontFamily", size : "AxisFontSize", bold : "AxisFontBold",
                  italic : "AxisFontItalic", color : "AxisFontColor"};
  var value, attr, attrName;
  var fontObj = {whichFont : "axes", face : "", size : "", bold : "", italic : "",
                 color : "", Canceled : false};
  for (attr in mapAttrs)
  {
    attrName = mapAttrs[attr];
    if (!graph.omitAttributeIfDefault(attrName) || graph.isUserSet(attrName))
      fontObj[attr] = graph.getGraphAttribute(attrName);
  }
  openDialog('chrome://prince/content/plotFontSettings.xul', 'axistickfontsettings', 'chrome,close,titlebar,modal,resizable', fontObj);

  if (!fontObj.Canceled)
  {
    for (attr in mapAttrs)
    {
      attrName = mapAttrs[attr];
      value = fontObj[attr];
      if (value && value != "")
      {
        graph.setGraphAttribute(attrName, value);
        graph.markUserSet(attrName, true);
      }
      else
      {
        graph.setGraphAttribute(attrName, null);
        graph.markUserSet(attrName, false);
      }
    }
  }
}

function openAxisTickFontSettingsDlg()
{
  var mapAttrs = {face : "TicksFontFamily", size : "TicksFontSize", bold : "TicksAxisFontBold",
                  italic : "TicksFontItalic", color : "TicksFontColor"};
  var value, attr, attrName;
  var fontObj = {whichFont : "axesTicks", face : "", size : "", bold : "", italic : "",
                 color : "", Canceled : false };
  for (attr in mapAttrs)
  {
    attrName = mapAttrs[attr];
    if (!graph.omitAttributeIfDefault(attrName) || graph.isUserSet(attrName))
      fontObj[attr] = graph.getGraphAttribute(attrName);
  }
  var defsize = Number(graph.getValue("AxisFontSize"));
  if (defsize != Number.NaN)
    defsize = 4 * defsize/5;
  else
    defsize = 8;
  var defaultFont = {face : graph.getValue("AxisFontFamily"),
                     size : String(defsize),
                     bold : graph.getValue("AxisFontBold"),
                     italic : graph.getValue("AxisFontItalic"),
                     color : graph.getValue("AxisFontColor")};
  openDialog('chrome://prince/content/plotFontSettings.xul', 'axistickfontsettings', 'chrome,close,titlebar,modal,resizable', fontObj, defaultFont);
  if (!fontObj.Canceled)
  {
    for (attr in mapAttrs)
    {
      attrName = mapAttrs[attr];
      value = fontObj[attr];
      if (value && value != "")
      {
        graph.setGraphAttribute(attrName, value);
        graph.markUserSet(attrName, true);
      }
      else
      {
        graph.setGraphAttribute(attrName, null);
        graph.markUserSet(attrName, false);
      }
    }
  }
}

function openPlotLabelsDlg()
{
  openDialog('chrome://prince/content/plotLabelsDlg.xul', 'plotlabels', 'chrome,close,titlebar,modal,resizable', graph);
}

function makeAxisLabelCustom(control)
{
  var whichAxis = "";
  switch(control.id)
  {
    case "customXAxisLabel":    whichAxis = "X";      break;
    case "customYAxisLabel":    whichAxis = "Y";      break;
    case "customZAxisLabel":    whichAxis = "Z";      break;
  }
  if (!whichAxis.length)
  {
    msidump("Unrecognized control in ComputeGraphSettings.js, makeAxisLabelCustom!\n");
    return;
  }
  document.getElementById(whichAxis + "AxisLabel").disabled = !control.checked;
  graph.markUserSet(whichAxis + "AxisLabel", control.checked);
}

function changeDefaultViewIntervals()
{
  var control = document.getElementById("defaultviewintervals");
  if (control.checked)
    document.getElementById("viewRangesActive").setAttribute("disabled", "true");
  else
    document.getElementById("viewRangesActive").removeAttribute("disabled");
}

function changeDefaultCamera()
{
  if (document.getElementById("defaultCameraCheckbox").checked)
    document.getElementById("customCameraProperties").setAttribute("disabled", "true");
  else
    document.getElementById("customCameraProperties").removeAttribute("disabled");
}

function changeAIMethod()
{
  var dim = graph.getDimension();
  var ptype;
  if (dim == 3) {
    ptype = document.getElementById("pt3d").value;
  } else {
    ptype = document.getElementById("pt2d").value;
  }
  hideShowControls(dim, ptype,
                     (graph.isAnimated() || document.getElementById("animate").checked),
                     document.getElementById("AIMethod").value);
}
