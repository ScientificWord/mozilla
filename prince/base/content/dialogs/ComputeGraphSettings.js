// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.
// Three data items are passed into this dialog: 
// window.arguments[0]: editorElement 
// window.arguments[1]: commandStr
// window.arguments[2]: DOMGraph, the DOM element that should be replaced

Components.utils.import("resource://app/modules/unitHandler.jsm"); 
var gFrameModeImage = true;
var gFrameModeTextFrame = false;
var plotUnitsHandler = new UnitHandler();
var graph;
var graphnode;
var plotwrapper;
var plotArray = [];

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
  var plotwrapper, units, alist, id, i, plotNumControl, numPlots, firstActivePlot, 
    plot, theStringSource, oldval, captionnode, placeLocation, obj, frame, topWindow;
  var graphEditorControl, radiusEditorControl;
  try {
    var gd = {};
    graphnode = window.arguments[2];
    var editorElement = window.arguments[0];
    graph = new Graph();
    graph.extractGraphAttributes(graphnode);
    frame = getFirstElementByTagName(graphnode,"msiframe");
    plotwrapper = getFirstElementByTagName(graphnode,"plotwrapper");
    obj = getFirstElementByTagName(graphnode, "object");
    topWindow = msiGetTopLevelWindow();
    if (topWindow && topWindow.document && topWindow.document.getElementById("vcamactive") 
         && (topWindow.document.getElementById("vcamactive").getAttribute("hidden")=="false"))
      queryVCamValues(obj, graph, graphnode, true);

    units = graph["Units"];
    if (!units || units.length === 0) 
    {
      units = "cm";
      graph["Units"] = units;
    }
    setHasNaturalSize(false);
    setCanRotate(false);
    document.getElementById("role-image").setAttribute("hidden",(gFrameModeImage?"false":"true"));
  //  graph.frame.extractFrameAttributes(frame, plotwrapper);
    gd = initFrameTab(gd, frame, false, plotwrapper);
  
    alist = graph.graphAttributeList(); 
    for ( i=0; i<alist.length; i++) { 
      id = mapid(alist[i]);                      
      if (document.getElementById(id)) {                    
        putValueToControlByID(id, graph.getValue(alist[i]));
//        document.getElementById(id).value = graph.getValue(alist[i]);
      }                                                       
    } 
  ///  return; 
    plotNumControl    = document.getElementById('plotnumber');
    numPlots = graph.getNumActivePlots();  //don't count any that may be already deleted - though probably not relevant during startup
    if (numPlots ===  0){ 
      addPlot();
      numPlots = 1;
    }
    plotNumControl.max = numPlots;
    plotNumControl.valueNumber = 1;
    firstActivePlot = getPlotInternalNum(1);  //get first active plot
    graph["plotnumber"] = firstActivePlot.toString();
    plot = graph.plots[firstActivePlot];
    graph.currentDisplayedPlot = -1;  //initialize to -1 so that populateDialog will execute (so that currentDisplayedPlot != plotnum)
    // some attributes can't be found as values of dialog elements  
//    setColorWell("baseColorWell", makeColorVal(plot.getPlotValue("BaseColor")));  
//    setColorWell("secondColorWell", makeColorVal(plot.getPlotValue("SecondaryColor")));
//    setColorWell("lineColorWell", makeColorVal(plot.getPlotValue("LineColor")));
    alist = graph.frame.FRAMEATTRIBUTES; 
    for ( i=0; i<alist.length; i++) { 
      id = mapid(alist[i]);                      
      if (document.getElementById(id)) {
        putValueToControlByID(id, graph.frame.getFrameAttribute(alist[i]));
//        document.getElementById(id).value = graph.frame.getFrameAttribute(alist[i]);         
      }                                                       
    }
    document.getElementById("defaultCameraCheckbox").checked = !graph.cameraValuesUserSet();
    document.getElementById("defaultviewintervals").checked = !graph.viewRangesUserSet();
    placeLocation = graph.frame.getFrameAttribute("placeLocation");
    if (document.getElementById("placeForceHereCheck")) document.getElementById("placeForceHereCheck").checked = (placeLocation.search("H") != -1);
    if (document.getElementById("placeHereCheck")) document.getElementById("placeHereCheck").checked = (placeLocation.search("h") != -1);
    if (document.getElementById("placeFloatsCheck")) document.getElementById("placeFloatsCheck").checked = (placeLocation.search("p") != -1);
    if (document.getElementById("placeTopCheck")) document.getElementById("placeTopCheck").checked = (placeLocation.search("t") != -1);
    if (document.getElementById("placeBottomCheck")) document.getElementById("placeBottomCheck").checked = (placeLocation.search("b") != -1);
    
    initKeyList();
  
    graphEditorControl = document.getElementById("plotDlg-content-frame");
    graphEditorControl.mInitialDocObserver = [{mCommand : "obs_documentCreated", mObserver : msiEditorDocumentObserverG}];
    graphEditorControl.mbSinglePara = true;
    graphEditorControl.mInitialContentListener = invisibleMathOpFilter;  //in plotDlgUtils.js
//    graphEditorControl.overrideStyleSheets = ["chrome://prince/skin/MathVarsDialog.css"];
    theStringSource = graph.plots[firstActivePlot].element["Expression"];
//    msiInitializeEditorForElement(editorControl, theStringSource, true);
    var editorInitializer = new msiEditorArrayInitializer();
    editorInitializer.addEditorInfo(graphEditorControl, theStringSource, true);
    
    captionnode = getFirstElementByTagName(graphnode,"caption");
 
    radiusEditorControl = document.getElementById("plotDlg-tube-radius");
    radiusEditorControl.mbSinglePara = true;
    radiusEditorControl.mInitialContentListener = invisibleMathOpFilter;  //in plotDlgUtils.js
    var ptype = graph.getPlotValue ("PlotType", firstActivePlot);
    if (ptype == "tube") {
      theStringSource = graph.plots[firstActivePlot].getPlotValue("TubeRadius");
      if (!theStringSource || theStringSource.length === 0) {
        theStringSource = GetComputeString("Math.emptyForInput");
//        theStringSource = "<math xmlns='http://www.w3.org/1998/Math/MathML'><mn>1</mn></math>";
      }
    } else {
      theStringSource = GetComputeString("Math.emptyForInput");
//      theStringSource = "<math xmlns='http://www.w3.org/1998/Math/MathML'><mn>1</mn></math>";
    }
//    msiInitializeEditorForElement(editorControl, theStringSource, true);
    editorInitializer.addEditorInfo(radiusEditorControl, theStringSource, true);

    editorInitializer.doInitialize();

    testUseSignificantDigits();
    // Caption placement
    document.getElementById("CaptionPlace").value = graph["CaptionPlace"];
  //  checkEnableFloating();
  }
  catch(e) {
    msidump(e.message);
  }
}                                                                                            
// This part pastes data into the editor after the editor has started. 
// implements nsIObserver
var msiEditorDocumentObserverG = {
  observe: function (aSubject, aTopic, aData) {
    if (aTopic === "obs_documentCreated") {
      var plotno = Number(graph["plotnumber"]);
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
    case "Width"             :   return "frameWidthInput";
    case "Height"            :   return "frameHeightInput";
    case "Placement"         :   return "placementRadioGroup";
    case "Float"             :   return "float";
    case "Units"             :   return "frameUnitMenulist";
    case "AxesType"          :   return "axistype";
    case "EqualScaling"      :   return "equalscale";
    case "BGColor"           :   return "plotCW";
    case "ViewingBoxXMin"    :   return "xrangelow";
    case "ViewingBoxXMax"    :   return "xrangehigh";
    case "ViewingBoxYMin"    :   return "yrangelow";
    case "ViewingBoxYMax"    :   return "yrangehigh";
    case "ViewingBoxZMin"    :   return "zrangelow";
    case "ViewingBoxZMax"    :   return "zrangehigh";

    //then to avoid the automatic placing of values where not appropriate:
    case "plotnumber"        :   return "";

    default                  :   return graphattribute;
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
  GetValuesFromDialog();
  graph.reviseGraphDOMElement(graphnode, false, editorElement);
  graph.setGraphAttribute("returnvalue", "true");
  //  var editor = msiGetEditor(editorElement);
  changed = true;
  if (changed) {
    graph.recomputeVCamImage(editorElement); 
  }
  theWindow = window.opener;
  if (!theWindow || !(theWindow.hasOwnProperty("nonmodalRecreateGraph"))) {
    theWindow = msiGetTopLevelWindow();
  }
  try {
    theWindow.nonmodalRecreateGraph(graph, window.arguments[2], editorElement);
  }
  catch (e) {}
  var parentWindow = window.opener;
//  parentWindow.ensureVCamPreinitForPlot(graphnode, editorElement);
  var obj = graphnode.getElementsByTagName("object");
  if (obj && obj.length)
  {
    obj = obj[0];
    if (obj) {
      obj.setAttribute('data', graphnode.getElementsByTagName('graphSpec')[0].getAttribute('ImageFile'));
      parentWindow.doVCamInitialize(obj);
    }
  }

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
      if (newval != "undefined") {   // NOTE, "" is OK, e.g. delete a label              
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
//  var alist  = Plot.prototype.plotAttributeList();                                
  var plotno = Number(graph.getGraphAttribute("plotnumber"));
  var plot = graph.plots[plotno];
  var alist  = plot.plotAttributeList();                                
  if (!plot) return;
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
  var tempPw = graphnode.getElementsByTagName("msiframe");
  if (tempPw.length > 0) {
    tempPw = tempPw[0];
  }
  var f = tempFrame.cloneNode(true);
  var p = tempPw.cloneNode(true);
  setFrameAttributes(f, p, editor);  // treuse code in msiFrameOverlay.
  frame.extractFrameAttributes(f, p);
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
    newval = graph.ser.serializeToString(mathnode);
    oldval = plot.element["Expression"];
    if (oldval != newval)
    {
      plot.element["Expression"] = newval;
      plot.attributes["PlotStatus"] = "New";
    }
    if (newpt == "tube")
    {
      doc = document.getElementById("plotDlg-tube-radius").contentDocument;
      mathnode = doc.getElementsByTagName("math")[0];
      newval = graph.ser.serializeToString(mathnode);
      oldval = plot.element["TubeRadius"];
      if (newval != oldval)
      {
        plot.element["TubeRadius"] = newval;  //should we mark plot as new for this? naaah
      };
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

  // AXES TAB
//  // GraphAxesScale
////  if (document.getElementById("AxisScale")) {
////    var newval = document.getElementById("AxisScale").selectedItem.value;
//  newval = getValueFromControl("AxisScale");
//  oldval = graph.getValue ("AxisScale");  
//  if (newval != oldval) {                                        
//    graph.setGraphAttribute ("AxisScale", newval);
//  }                                                               
////  }
//  // GraphEqualScaling
////  if (document.getElementById("equalscale")) {                            
////    var newval = document.getElementById("equalscale").checked ? "true" : "false";
//  newval = getValueFromControl("equalscale");
//  oldval = graph.getValue ("EqualScaling");  
//  if (newval != oldval) {                                        
//    graph.setGraphAttribute ("EqualScaling", newval);
//  }                                                               
////  }
//  // GraphAxesTips
//  if (document.getElementById("AxesTips")) {                            
//    var newval = document.getElementById("AxesTips").checked ? "true" : "false";
//    var oldval = graph.getValue ("AxesTips");  
//    if (newval != oldval) {                                        
//      graph.setGraphAttribute ("AxesTips", newval);
//    }                                                               
//  }
//  // GraphGridLines
//  if (document.getElementById("gridlines")) {                            
//    var newval = document.getElementById("GridLines").checked ? "true" : "false";
//    var oldval = graph.getValue ("GridLines");  
//    if (newval != oldval) {                                        
//      graph.setGraphAttribute ("GridLines", newval);
//    }                                                               
//  }
//  // GraphAxesType
//  if (document.getElementById("axistype")) {                            
//    var index  = document.getElementById("axistype").selectedIndex;
//    var newval = document.getElementById("axistype").selectedItem.value;
//    var oldval = graph.getValue ("AxesType"); 
//    if (newval != oldval) {                                        
//      graph.setGraphAttribute ("AxesType", newval);
//    }                                                               
//  }

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
  // View Tab
  // Orientation
//  if (document.getElementById("CameraLocationX")) {
//    var camLocX = document.getElementById("CameraLocationX").value;
//    graph.setGraphAttribute ("CameraLocationX", camLocX);
//  }
//  if (document.getElementById("CameraLocationY")){
//    var camLocY = document.getElementById("CameraLocationY").value;
//    graph.setGraphAttribute ("CameraLocationY", camLocY);
//  }
//
//  if ( document.getElementById("CameraLocationZ") ){
//    var camLocZ = document.getElementById("CameraLocationZ").value;
//    graph.setGraphAttribute ("CameraLocationZ", camLocZ);
//  }
//
//  if ( document.getElementById("FocalPointX") ){
//    var focalPtX = document.getElementById("FocalPointX").value;
//    graph.setGraphAttribute ("FocalPointX", focalPtX);
//  }
//  if ( document.getElementById("FocalPointY") ){
//    var focalPtY = document.getElementById("FocalPointY").value;
//    graph.setGraphAttribute ("FocalPointY", focalPtY);
//  }
//
//  if ( document.getElementById("FocalPointZ") ) {
//    var focalPtZ = document.getElementById("FocalPointZ").value;
//    graph.setGraphAttribute ("FocalPointZ", focalPtZ);
//  }
//
//  if (document.getElementById("UpVectorX")){
//    var upVecX = document.getElementById("UpVectorX").value;
//    graph.setGraphAttribute ("UpVectorX", upVecX);
//  }
//  if (document.getElementById("UpVectorY")){
//    var upVecY = document.getElementById("UpVectorY").value;
//    graph.setGraphAttribute ("UpVectorY", upVecY);
//  }
//  if (document.getElementById("UpVectorZ")){
//    var upVecZ = document.getElementById("UpVectorZ").value;
//    graph.setGraphAttribute ("UpVectorZ", upVecZ);
//  }
//  if (document.getElementById("ViewingAngle")){
//     var va = document.getElementById("ViewingAngle").value;
//     graph.setGraphAttribute ("ViewingAngle", va);
//  }
//  if (document.getElementById("OrthogonalProjection")){
//     var op = document.getElementById("OrthogonalProjection").checked ? "true" : "false";
//     graph.setGraphAttribute ("OrthogonalProjection", op);
//  }
//  if (document.getElementById("KeepUp")){
//    var ku = document.getElementById("KeepUp").checked ? "true" : "false";
//    graph.setGraphAttribute ("KeepUp", ku);
//  }
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

//// This is the callback for the command button to edit a plot
//// on entry, the "plotnumber" widget has the plot number of the items to be edited. 
//// on exit, the ComputePlotSettings.xul dialog has saved the new data 
//function formatPlot () {
//  // only open one dialog per window
//  var count = document.getElementById("plotnumber").valueNumber; 
//  graph.setGraphAttribute("plotnumber", getPlotInternalNum(count));
//  window.openDialog("chrome://prince/content/ComputePlotSettings.xul", 
//                    "Plot_Settings", "chrome,close,titlebar,dependent,resizable", 
//                    graph, window, window.arguments[2]);
//}

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
    var editorControl = document.getElementById("plotDlg-content-frame");
    var doc = editorControl.contentDocument;
    var math = doc.getElementsByTagName("math")[0];
    var str;
    var radiusControl = document.getElementById("plotDlg-tube-radius");
    var radiusStringSource, radiusMath, radiusEditor;
    var editor = msiGetEditor(editorControl);
    var theStringSource = graph.ser.serializeToString(math);
//    editorControl.contentDocument.documentElement = null;
    var doit = true;
    var oldptype="Unknown";
    if (oldplotno >= 0)
    {
      oldptype = graph.getPlotValue ("PlotType", oldplotno);
      graph.plots[oldplotno].element["Expression"]=theStringSource;
    }
    var ptype = graph.getPlotValue ("PlotType", plotno);
    radiusEditor = msiGetEditor(radiusControl);
    radiusMath = radiusControl.contentDocument.getElementsByTagName("math")[0];
    if (oldptype == "tube")
    {
      radiusStringSource = graph.ser.serializeToString(radiusMath);
      if (oldplotno >= 0)
        graph.plots[oldplotno].element["TubeRadius"] = radiusStringSource;
    }
    graph["plotnumber"] = String(plotno);
    theStringSource = graph.plots[plotno].element["Expression"];
    if (!theStringSource || (theStringSource.length === 0))
    {
      theStringSource = GetComputeString("Math.emptyForInput");
    }
    while (math.firstChild)
    {
      math.removeChild(math.firstChild);
    }
//    insertXML(editor, theStringSource, math, 0, false);
    editor.addInsertionListener(invisibleMathOpFilter);   //in plotDlgUtils.js
    editor.insertHTMLWithContext(theStringSource, "", "", "", null, math, 0, false);
    editor.removeInsertionListener(invisibleMathOpFilter);
    graph.currentDisplayedPlot = plotno;
    if (ptype == "tube")
    {
      radiusStringSource = graph.plots[plotno].getPlotValue("TubeRadius");
      if (!radiusStringSource || radiusStringSource.length === 0)
        radiusStringSource = GetComputeString("Math.emptyForInput");
      while (radiusMath.firstChild)
        radiusMath.removeChild(radiusMath.firstChild);
      radiusEditor.addInsertionListener(invisibleMathOpFilter);   //in plotDlgUtils.js
      radiusEditor.insertHTMLWithContext(radiusStringSource, "", "", "", null, radiusMath, 0, false);
      radiusEditor.removeInsertionListener(invisibleMathOpFilter);   //in plotDlgUtils.js
    }
  }
  catch (e)
  {
    msidump("populateDialog: " + e.message + "\n");
  }
  
  try
  {
    // handle the top descriptor
    var dim = graph.getDimension();
    populateDescription ("dimension", String(dim));
    var ptype = graph.getPlotValue ("PlotType", plotno);
    var aiMethod = (ptype === "approximateIntegral") ? graph.getPlotValue("AIMethod", plotno) : "";
    hideShowControls(dim, ptype, graph.isAnimated(), aiMethod)

    if (dim == 3) {
//      document.getElementById("plotcs3d").collapsed = false;            
//      document.getElementById("plotcs2d").collapsed = true;            
      populatePopupMenu ("pt3d", ptype);
    } else {
//      document.getElementById("plotcs2d").collapsed = false;            
//      document.getElementById("plotcs3d").collapsed = true;            
      populatePopupMenu ("pt2d", ptype);  
    }
//    if (ptype == "tube")
//    {
//      document.getElementById("tubeControls").collapsed = false;
//      document.getElementById("lineAndPointLabels").collapsed = true;
//      document.getElementById("lineAndPointControls").collapsed = true;
//    }
//    else
//    {
//      document.getElementById("tubeControls").collapsed = true;
//      document.getElementById("lineAndPointLabels").collapsed = false;
//      document.getElementById("lineAndPointControls").collapsed = false;
//    }
    var animatevalue = graph.getPlotValue ("Animate",plotno);
    document.getElementById("animate").checked = (animatevalue == "true")?true:false;
    //populateDescription ("animate", graph.getPlotValue ("Animate",plotno));

    // put the current plot number into the dialog box
    var plotml = document.getElementById("plotnumber");
    plotml.valueNumber = getActivePlotNumber(plotno);
//This happens automatically with number-type textboxes.
//    for (var idx=0; idx<plotml.childNodes.length; idx++) { 
//      if (plotml.childNodes[idx].getAttribute("value") == plotno) {
//        document.getElementById("plotnumber").selectedItem = plotml.childNodes[idx]; 
//      }    
//    }       

    var alist = graph.plotAttributeList(plotno);
    var anID, theVal;
    for (var i = 0; i < alist.length; ++i)
    {
      anID = mapPlotID(alist[i], plotno);
      if (document.getElementById(anID)) {
        theVal = graph.getPlotValue (alist[i], plotno);
        putValueToControlByID(anID, theVal);
//        document.getElementById(anID).value = theVal;
      }                                                                   
    }
//    setColorWell("baseColorWell", makeColorVal(graph.getPlotValue("BaseColor", plotno)));  
//    setColorWell("secondColorWell", makeColorVal(graph.getPlotValue("SecondaryColor", plotno)));
//    setColorWell("lineColorWell", makeColorVal(graph.getPlotValue("LineColor", plotno)));
  
//**rwa - these should already have been taken carae of, during the loop through the graph's attributes***//
//    // AXES TAB
//    // GraphAxesScale
//    var oldval = graph.getValue ("AxisScale"); 
//    radioGroupSetCurrent ("AxisScale", oldval);
//    // GraphEqualScaling
//    if (document.getElementById("equalscale")) {  
//      var oldval = graph.getValue ("EqualScaling");  
//      if (oldval == "true") {
//         document.getElementById("equalscale").checked = true;            
//      }                                                               
//    }
//    // GraphAxesTips
//    if (document.getElementById("AxesTips")) {                            
//      var oldval = graph.getValue ("AxesTips");  
//      if (oldval == "true") {
//         document.getElementById("AxesTips").checked = true;            
//      }                                                               
//    }
//    // GraphGridLines
//    if (document.getElementById("GridLines")) {                            
//      var oldval = graph.getValue ("GridLines");  
//      if (oldval == "true") {
//         document.getElementById("GridLines").checked = true;            
//      }                                                               
//    }
//    // GraphAxesType
//    var oldval = graph.getValue ("AxesType");  
//    radioGroupSetCurrent ("axistype", oldval);

    // Layout Tab
    // printAttribute
    var oldval = graph.getValue ("PrintAttribute");  
    radioGroupSetCurrent ("printattr", oldval);
    // ScreenDisplay
    var oldval = graph.getValue ("PrintFrame");  
    radioGroupSetCurrent ("screendisplayattr", oldval);
    // GraphAxesType
    var oldval = graph.getValue ("Placement");  
    radioGroupSetCurrent ("placement", oldval);

    // Labelling Tab
    // Captionplace
    document.getElementById("CaptionPlace").value = graph.getValue ("CaptionPlace");

//**rwa - these should already have been taken carae of, during the loop through the graph's attributes***//
//    // View tab
//    var camLocX = graph.getValue ("CameraLocationX");
//    var camLocY = graph.getValue ("CameraLocationY");
//    var camLocZ = graph.getValue ("CameraLocationZ");
//    populateDescription ("CameraLocationX", camLocX);
//    populateDescription ("CameraLocationY", camLocY);
//    populateDescription ("CameraLocationZ", camLocZ);
//
//    var focalPtX = graph.getValue ("FocalPointX");
//    var focalPtY = graph.getValue ("FocalPointY");
//    var focalPtZ = graph.getValue ("FocalPointZ");
//    populateDescription ("FocalPointX", focalPtX);
//    populateDescription ("FocalPointY", focalPtY);
//    populateDescription ("FocalPointZ", focalPtZ);
//
//    var upVecX = graph.getValue ("UpVectorX");
//    var upVecY = graph.getValue ("UpVectorY");
//    var upVecZ = graph.getValue ("UpVectorZ");
//    populateDescription ("UpVectorX", upVecX);
//    populateDescription ("UpVectorY", upVecY);
//    populateDescription ("UpVectorZ", upVecZ);
//
//    var va = graph.getValue ("ViewingAngle");
//    var op = graph.getValue ("OrthogonalProjection");
//    var ku = graph.getValue ("KeepUp");
//    populateDescription ("ViewingAngle", va);
//    document.getElementById("OrthogonalProjection").checked = (op == "true")?true:false;
//    document.getElementById("KeepUp").checked = (ku == "true")?true:false;
  }
  catch (e)
  {
    msidump("populateDialog: " + e.message + "\n");
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

//function needTwoVars (plotno) {
//  var dim  = graph.getValue ("Dimension");
//  var pt   = graph.getPlotValue ("PlotType", plotno);
//  var anim = graph.getPlotValue ("Animate",plotno);
//  var nvars = PlotVarsNeeded (dim, pt, anim);
//  return (nvars > 1);
///*                                                                                               */
///*                                                                                               */
///*   if (anim == "true") return true;                                                            */
///*   if (pt == "curve") return false;                                                            */
///*   if (dim[0] == "3") return true;                                                             */
///*   if ((pt == "rectangular") || (pt == "parametric") || (pt == "conformal") || (pt == "polar") */
///*       || (pt == "approximateIntegral"))                                                       */
///*     return false;                                                                             */
///*   return true;                                                                                */
//}
//
//function needThreeVars (plotno) {
//  var dim  = graph.getValue ("Dimension");
//  var pt   = graph.getPlotValue ("PlotType", plotno);
//  var anim = graph.getPlotValue ("Animate", plotno);
//  var nvars = PlotVarsNeeded (dim, pt, anim);
//  return (nvars > 2);
///*                                                                         */
///*   if ((dim[0] == "2") && (anim != "true")) return false;                */
///*   if ((pt == "rectangular") || (pt == "parametric") || (pt == "tube")   */
///*      || (pt == "spherical") || (pt == "cylindrical")|| (pt == "curve")) */
///*     return false;                                                       */
///*   return true;                                                          */
//  
//}


function initKeyList()
{
  gDialog.markerList = new msiKeyMarkerList(window);
  gDialog.markerList.setUpTextBoxControl(document.getElementById("Key"));
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
  openDialog('chrome://prince/content/intervalsAndAnimation.xul', 'Plot Intervals and Animation', 'chrome,close,titlebar,modal,resizable', graphData);
}

function openAnimationSettingsDlg()
{
  openDialog('chrome://prince/content/plotAnimationSettings.xul', 'Animation Settings', 'chrome,close,titlebar,modal,resizable', graph);
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
  openDialog('chrome://prince/content/plotFontSettings.xul', 'Axis Tick Font Settings', 'chrome,close,titlebar,modal,resizable', fontObj);

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
  openDialog('chrome://prince/content/plotFontSettings.xul', 'Axis Tick Font Settings', 'chrome,close,titlebar,modal,resizable', fontObj, defaultFont);
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
  openDialog('chrome://prince/content/plotLabelsDlg.xul', 'Plot Labels', 'chrome,close,titlebar,modal,resizable', graph);
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