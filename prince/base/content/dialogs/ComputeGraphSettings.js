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
    plot, editorControl, theStringSource, oldval, captionnode, placeLocation;
  try {
    var gd = {};
    graphnode = window.arguments[2];
    var editorElement = window.arguments[0];
    graph = new Graph();
    graph.extractGraphAttributes(graphnode);
    var frame = getFirstElementByTagName(graphnode,"msiframe");
    plotwrapper = getFirstElementByTagName(graphnode,"plotwrapper");
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
        document.getElementById(id).value = graph[alist[i]];         
      }                                                       
    } 
  ///  return; 
    plotNumControl    = document.getElementById('plotnumber');
    numPlots = graph.getNumPlots();
    if (numPlots ===  0){ 
      addPlot();
      numPlots = 1;
    }
    plotNumControl.max = numPlots;
    plotNumControl.valueNumber = 1;
    firstActivePlot = 0;
    graph["plotnumber"] = firstActivePlot.toString();
    plot = graph["plots"][0];
    graph.currentDisplayedPlot = 0;
    // some attributes can't be found as values of dialog elements  
    setColorWell("baseColorWell", plot.attributes["BaseColor"]);  
    setColorWell("secondColorWell", plot.attributes["SecondaryColor"]);  
    setColorWell("lineColorWell", plot.attributes["LineColor"]);  
    alist = graph.frame.FRAMEATTRIBUTES; 
    for ( i=0; i<alist.length; i++) { 
      id = mapid(alist[i]);                      
      if (document.getElementById(id)) {                    
        document.getElementById(id).value = graph.frame.getFrameAttribute(alist[i]);         
      }                                                       
    } 
    placeLocation = graph.frame.getFrameAttribute("placeLocation");
    document.getElementById("placeForceHereCheck").checked = (placeLocation.search("H") != -1);
    document.getElementById("placeHereCheck").checked = (placeLocation.search("h") != -1);
    document.getElementById("placeFloatsCheck").checked = (placeLocation.search("p") != -1);
    document.getElementById("placeTopCheck").checked = (placeLocation.search("t") != -1);
    document.getElementById("placeBottomCheck").checked = (placeLocation.search("b") != -1);
    
  
    initKeyList();
  
    editorControl = document.getElementById("plotDlg-content-frame");
    editorControl.overrideStyleSheets = ["chrome://prince/skin/MathVarsDialog.css"];
    theStringSource = graph.plots[firstActivePlot].element["Expression"];
    msiInitializeEditorForElement(editorControl, theStringSource, true);
  //  editorControl.makeEditable("html");
    editorControl = document.getElementById("captionText");
    editorControl.overrideStyleSheets = ["chrome://prince/skin/MathVarsDialog.css"];
    captionnode = getFirstElementByTagName(graphnode,"imageCaption");
    if (captionnode) {
      theStringSource = graph.ser.serializeToString(captionnode);
      if (!theStringSource || theStringSource.length === 0) {
        theStringSource = "<br temp='1'/>";
      }
    } else (theStringSource = "<br temp='1'/>")
    msiInitializeEditorForElement(editorControl, theStringSource, true);
  //  editorControl.makeEditable("html");

    // Caption placement
    oldval = graph["CaptionPlace"];  
    radioGroupSetCurrent ("captionplacement", oldval);
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
      var plotno = graph["plotnumber"];
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
    case "Width" : return "frameWidthInput";
    case "Height": return "frameHeightInput";
    case "Placement" : return "placementRadioGroup";
    case "Float"     : return "float";
    case "Units"     : return "frameUnitMenulist";
    default          : return graphattribute;
  }
}


// Extract the values from the dialog and store them in the data structure
// Only save values that are not the defaults
function OK() {
  var editorElement, changed, theWindow;
  editorElement = msiGetParentEditorElementForDialog(window);
  GetValuesFromDialog();
  graph.reviseGraphDOMElement(graphnode, editorElement);
  graph.setGraphAttribute("returnvalue", true);
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
  return true;
}

// Extract the values from the dialog and store them in the data structure
// Only save values that are not the defaults
function GetValuesFromDialog(){
  // grab anything that's in the graph attribute list
  var editorElement = msiGetParentEditorElementForDialog(window);
  var editor = msiGetEditor(editorElement);
  var alist = graph.graphAttributeList(); 
  var frame = graph.frame;                               
  for (var i=0; i<alist.length; i++) {                                   
    if (document.getElementById(alist[i])) {                            
      var newval = document.getElementById(alist[i]).value;            
      if (newval != "undefined") {   // NOTE, "" is OK, e.g. delete a label              
        var oldval = graph.getValue (alist[i]);                        
        if (newval != oldval) {                                        
           graph.setGraphAttribute(alist[i], newval);                  
        }                                                              
      }                                                                
    }                                                                   
  }                                                                               

  // grab anything that's in the plot attribute list
  // we save data for only the currently displayed plot, since the others are
  // already saved.
  var alist  = Plot.prototype.plotAttributeList();                                
  var plotno = graph.getGraphAttribute("plotnumber");
  var plot = graph.plots[plotno];
  if (!plot) return;
  var dim    = graph.getGraphAttribute("Dimension");
  for (var i=0; i<alist.length; i++) {                                   
    if (document.getElementById(alist[i])) {                            
      var newval = document.getElementById(alist[i]).value;            
      if ((newval != "") && (newval != "undefined")) {                 
        var oldval = graph.getPlotValue (alist[i], plotno);                        
        if (newval != oldval) {                                        
           graph.setPlotAttribute (PlotAttrName(alist[i], plotno), newval);
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
  setFrameAttributes(f, p, editor);  // trick to reuse code in msiFrameOverlay.
  frame.extractFrameAttributes(f, p);
  var oldpt = plot.attributes["PlotType"];                        
  var newpt;
  if (dim == "2") {
    newpt = document.getElementById("pt2d").value;            
  } else { 
    newpt = document.getElementById("pt3d").value;            
  }
  if (newpt != oldpt) {
    if (newpt) plot.attributes["PlotType"] = newpt;
    plot.attributes["PlotStatus"] = "New";
  }    
  var oldanimate = plot.attributes["Animate"];
  var newanimate = document.getElementById("animate").checked;
  if (oldanimate != newanimate)
    plot.attributes["Animate"] = (newanimate?"true":"false");
  try  
  {
    var doc = document.getElementById("plotDlg-content-frame").contentDocument;
    var mathnode = doc.getElementsByTagName("math")[0];
    plot.element["Expression"] = graph.ser.serializeToString(mathnode);
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
  // GraphAxesScale
  if (document.getElementById("axisscale")) {                            
    var newval = document.getElementById("axisscale").selectedItem.value;
    var oldval = graph.getValue ("AxisScale");  
    if (newval != oldval) {                                        
      graph.setGraphAttribute ("AxisScale", newval);
    }                                                               
  }
  // GraphEqualScaling
  if (document.getElementById("equalscale")) {                            
    var newval = document.getElementById("equalscale").checked ? "true" : "false";
    var oldval = graph.getValue ("EqualScaling");  
    if (newval != oldval) {                                        
      graph.setGraphAttribute ("EqualScaling", newval);
    }                                                               
  }
  // GraphAxesTips
  if (document.getElementById("axestips")) {                            
    var newval = document.getElementById("axestips").checked ? "true" : "false";
    var oldval = graph.getValue ("AxesTips");  
    if (newval != oldval) {                                        
      graph.setGraphAttribute ("AxesTips", newval);
    }                                                               
  }
  // GraphGridLines
  if (document.getElementById("gridlines")) {                            
    var newval = document.getElementById("gridlines").checked ? "true" : "false";
    var oldval = graph.getValue ("GridLines");  
    if (newval != oldval) {                                        
      graph.setGraphAttribute ("GridLines", newval);
    }                                                               
  }
  // GraphAxesType
  if (document.getElementById("axistype")) {                            
    var index  = document.getElementById("axistype").selectedIndex;
    var newval = document.getElementById("axistype").selectedItem.value;
    var oldval = graph.getValue ("AxesType"); 
    if (newval != oldval) {                                        
      graph.setGraphAttribute ("AxesType", newval);
    }                                                               
  }

  // Layout Tab
  // printAttribute
  if (document.getElementById("printattr")) {                            
    var newval = document.getElementById("printattr").selectedItem.value;
    var oldval = graph.getValue ("PrintAttribute"); 
    if (newval != oldval) {                                        
      graph.setGraphAttribute ("PrintAttribute", newval);
    }                                                               
  }
  // ScreenDisplay
  if (document.getElementById("screendisplayattr")) {                            
    var newval = document.getElementById("screendisplayattr").selectedItem.value;
    var oldval = graph.getValue ("PrintFrame"); 
    if (newval != oldval) {                                        
      graph.setGraphAttribute ("PrintFrame", newval);
    }                                                               
  }
  // GraphAxesType
  if (document.getElementById("placementRadioGroup")) {                            
    var newval = document.getElementById("placementRadioGroup").selectedItem.id;
    var oldval = graph.getValue ("Placement"); 
    if (newval != oldval) {                                        
      graph.setGraphAttribute ("Placement", newval);
    }                                                               
  }
  // Labelling Tab
  // Captionplacement
  if (document.getElementById("captionplacement")) {                            
    var newval = document.getElementById("captionplacement").selectedItem.value;
    var oldval = graph.getValue ("CaptionPlace"); 
    if (newval != oldval) {                                        
      graph.setGraphAttribute ("CaptionPlace", newval);
    }                                                               
  }
  // Caption
  if (document.getElementById("captionText")) {     
    var editorControl = document.getElementById("captionText");
    var doc = editorControl.contentDocument;
    var aNode = doc.documentElement;
    if (aNode.nodeName !== 'dialogbase') {
      aNode = aNode.getElementsByTagName("dialogbase");
      if (aNode.length > 0) {
        aNode = aNode[0];
      } else return;
    }     
    var newval = getNodeChildrenAsString(aNode)                  
    var oldval = graph.getValue ("Caption"); 
    if (newval != oldval) {                                        
      graph.setGraphAttribute ("Caption", newval);
    }                                                               
  }
  // View Tab
  // Orientation
  if (document.getElementById("CameraLocationX")) {
    var camLocX = document.getElementById("CameraLocationX").value;
    graph.setGraphAttribute ("CameraLocationX", camLocX);
  }
  if (document.getElementById("CameraLocationY")){
    var camLocY = document.getElementById("CameraLocationY").value;
    graph.setGraphAttribute ("CameraLocationY", camLocY);
  }

  if ( document.getElementById("CameraLocationZ") ){
    var camLocZ = document.getElementById("CameraLocationZ").value;
    graph.setGraphAttribute ("CameraLocationZ", camLocZ);
  }

  if ( document.getElementById("FocalPointX") ){
    var focalPtX = document.getElementById("FocalPointX").value;
    graph.setGraphAttribute ("FocalPointX", focalPtX);
  }
  if ( document.getElementById("FocalPointY") ){
    var focalPtY = document.getElementById("FocalPointY").value;
    graph.setGraphAttribute ("FocalPointY", focalPtY);
  }

  if ( document.getElementById("FocalPointZ") ) {
    var focalPtZ = document.getElementById("FocalPointZ").value;
    graph.setGraphAttribute ("FocalPointZ", focalPtZ);
  }

  if (document.getElementById("UpVectorX")){
    var upVecX = document.getElementById("UpVectorX").value;
    graph.setGraphAttribute ("UpVectorX", upVecX);
  }
  if (document.getElementById("UpVectorY")){
    var upVecY = document.getElementById("UpVectorY").value;
    graph.setGraphAttribute ("UpVectorY", upVecY);
  }
  if (document.getElementById("UpVectorZ")){
    var upVecZ = document.getElementById("UpVectorZ").value;
    graph.setGraphAttribute ("UpVectorZ", upVecZ);
  }
  if (document.getElementById("ViewingAngle")){
     var va = document.getElementById("ViewingAngle").value;
     graph.setGraphAttribute ("ViewingAngle", va);
  }
  if (document.getElementById("OrthogonalProjection")){
     var op = document.getElementById("OrthogonalProjection").checked ? "true" : "false";
     graph.setGraphAttribute ("OrthogonalProjection", op);
  }
  if (document.getElementById("KeepUp")){
    var ku = document.getElementById("KeepUp").checked ? "true" : "false";
    graph.setGraphAttribute ("KeepUp", ku);
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
      graph.setPlotAttribute (PlotAttrName(name, plotno), newval);
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
  graph.setGraphAttribute("returnvalue", false);
}

// This is the callback for the command button to add a new plot
// Create a new plot, fire up the ComputePlotSettings.xul edit dialog. 
// Set the PlotStatus to New so OK can call the preparePlot function
function addPlot () {
  // save any changes to current plot, then change plots. Cancel ignores all changes
  try
  {
    GetValuesFromDialog();
    graph.setGraphAttribute("returnvalue", false);                 
    addPlotDialogContents();
  }
  catch(e){
    msidump(e.message+"\n");
  }
}

function addPlotDialogContents () {
  var plot = new Plot();
  var plotnum = graph.addPlot(plot);
  graph.setGraphAttribute("plotnumber", plotnum);
  var plotNumControl  = document.getElementById('plotnumber');
  plotNumControl.max = plotnum + 1;
  plotNumControl.valueNumber = plotnum+1;
//  var newElement = document.createElement('menuitem');
//  newElement.setAttribute("label", plotnumber.toString());
//  newElement.setAttribute("value", plotnumber.toString());
//  popup.appendChild(newElement);
//  document.getElementById("plot").selectedItem = newElement; 
  // grab the plottype from plot 1 and set it as default
  var oldtype = graph.plots[0].attributes["PlotType"];
  if (oldtype == "") oldtype = "rectangular";
  plot.attributes["PlotType"] = oldtype;
  plot.attributes["PlotStatus"] = "New";
  populateDialog (plotnum);   
}         

// This is the callback for the command button to edit a plot
// on entry, the "plotnumber" widget has the plot number of the items to be edited. 
// on exit, the ComputePlotSettings.xul dialog has saved the new data 
function formatPlot () {
  // only open one dialog per window
  var count = document.getElementById("plotnumber").valueNumber; 
  graph.setGraphAttribute("plotnumber", getPlotInternalNum(count));
  window.openDialog("chrome://prince/content/ComputePlotSettings.xul", 
                    "Plot_Settings", "chrome,close,titlebar,dependent,resizable", 
                    graph, window, window.arguments[2]);
}

// Delete a plot and set the dialog to the next available plot
function deletePlot () {
  // extract the plot number from the dialog
  var newplotno, numplots, plotno, plotNumControl;
  plotno = document.getElementById("plot").selectedItem.value; 
  plotno = plotno - 1; // plot numbers are 0-based, the UI uses 1-based.
  graph.deletePlot(plotno);
  plotNumControl    = document.getElementById('plotnumber');                    
  // find the next plot if there is one, or the previous plot if there is one
  numplots = graph.getNumPlots();
  plotNumControl.max = numplots;
  newplotno = plotno;
  if (newplotno >= numplots) {
    newplotno = numplots - 1;
  }
  if (newplotno < 0) {   // no undeleted plots, add one.
    addPlotDialogContents ();
  } else {               // use the next plot
    graph.setGraphAttribute("plotnumber", newplotno);
    populateDialog (newplotno);
  }
}

// the user has just selected a new plot type for this plot. Rebuild the 
// dialog
function changePlotType () {
  // get the plot number and the plot type. Then populate
  var dim    = graph.getGraphAttribute("Dimension");
  var oldpt = graph.getPlotValue ("PlotType", plotno);                        
  var plotno = getPlotInternalNum(document.getElementById("plotnumber").valueNumber); 
//  if ((plotno == null) || (plotno == "")) {
  if (!plotno)
    plotno = 1;
//  }
  graph.setGraphAttribute("plotnumber", plotno);
  var newpt;
  if (dim == "2") {
    newpt = document.getElementById("pt2d").value;            
  } else { 
    newpt = document.getElementById("pt3d").value;            
  }
  if (newpt != oldpt) {
    graph.setPlotAttribute (PlotAttrName("PlotType", plotno), newpt);
    graph.setPlotAttribute (PlotAttrName("PlotStatus", plotno), "New");
  }    
  populateDialog (plotno);
}

// the user has just selected a new plot number in the dialog: populate the 
// current screen with the data for this plot
function changePlot () {
  // save any changes to this plot, then change plots. Cancel ignores all changes
  GetValuesFromDialog();
  graph.setGraphAttribute("returnvalue", false);                 

  // extract the plot number from the dialog
  var plotno = document.getElementById("plot").selectedItem.value; 
  if ((plotno == null) || (plotno == "")) {              
    plotno = 1;
  }
  graph.setGraphAttribute("plotnumber", plotno);
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
  return internalPlotNum+1;
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
    var editor = msiGetEditor(editorControl);
    var theStringSource = graph.ser.serializeToString(math);
//    editorControl.contentDocument.documentElement = null;
    var doit = true;
    graph.plots[oldplotno].element["Expression"]=theStringSource;
    graph["plotnumber"] = plotno;
    theStringSource = graph.plots[plotno].element["Expression"];
    if (!theStringSource || (theStringSource.length === 0))
    {
      theStringSource = GetComputeString("Math.emptyForInput");
    }
    while (math.firstChild)
    {
      math.removeChild(math.firstChild);
    }
    insertXML(editor, theStringSource, math, 0, false);
    graph.currentDisplayedPlot = plotno;
  }
  catch (e)
  {
    msidump("populateDialog: " + e.message + "\n");
  }
  // handle the top descriptor
  var dim = graph.getValue ("Dimension");
  populateDescription ("dimension", dim);
  var ptype = graph.getPlotValue ("PlotType", plotno);
  if (dim == "2") {
    document.getElementById("plotcs2d").collapsed = false;            
    document.getElementById("plotcs3d").collapsed = true;            
    populatePopupMenu ("pt2d", ptype);  
  } else {
    document.getElementById("plotcs3d").collapsed = false;            
    document.getElementById("plotcs2d").collapsed = true;            
    populatePopupMenu ("pt3d", ptype);
  }
  var animatevalue = graph.getPlotValue ("Animate",plotno);
  document.getElementById("animate").checked = (animatevalue == "true")?true:false;
  //populateDescription ("animate", graph.getPlotValue ("Animate",plotno));

  // put the current plot number into the dialog box
  var plotml = document.getElementById("plotnumber");
  plotml.valueNumber = getActivePlotNumber(plotno+1);
  for (var idx=0; idx<plotml.childNodes.length; idx++) { 
    if (plotml.childNodes[idx].getAttribute("value") == plotno) {
      document.getElementById("plotnumber").selectedItem = plotml.childNodes[idx]; 
    }    
  }       

  // AXES TAB
  // GraphAxesScale
  var oldval = graph.getValue ("AxisScale"); 
  radioGroupSetCurrent ("axisscale", oldval);
  // GraphEqualScaling
  if (document.getElementById("equalscale")) {  
    var oldval = graph.getValue ("EqualScaling");  
    if (oldval == "true") {
       document.getElementById("equalscale").checked = true;            
    }                                                               
  }
  // GraphAxesTips
  if (document.getElementById("axestips")) {                            
    var oldval = graph.getValue ("AxesTips");  
    if (oldval == "true") {
       document.getElementById("axestips").checked = true;            
    }                                                               
  }
  // GraphGridLines
  if (document.getElementById("gridlines")) {                            
    var oldval = graph.getValue ("GridLines");  
    if (oldval == "true") {
       document.getElementById("gridlines").checked = true;            
    }                                                               
  }
  // GraphAxesType
  var oldval = graph.getValue ("AxesType");  
  radioGroupSetCurrent ("axistype", oldval);
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
  // Captionplacement
  var oldval = graph.getValue ("CaptionPlace");  
  radioGroupSetCurrent ("captionplacement", oldval);

  // Axes tab
  var camLocX = graph.getValue ("CameraLocationX");
  var camLocY = graph.getValue ("CameraLocationY");
  var camLocZ = graph.getValue ("CameraLocationZ");
  populateDescription ("CameraLocationX", camLocX);
  populateDescription ("CameraLocationY", camLocY);
  populateDescription ("CameraLocationZ", camLocZ);

  var focalPtX = graph.getValue ("FocalPointX");
  var focalPtY = graph.getValue ("FocalPointY");
  var focalPtZ = graph.getValue ("FocalPointZ");
  populateDescription ("FocalPointX", focalPtX);
  populateDescription ("FocalPointY", focalPtY);
  populateDescription ("FocalPointZ", focalPtZ);

  var upVecX = graph.getValue ("UpVectorX");
  var upVecY = graph.getValue ("UpVectorY");
  var upVecZ = graph.getValue ("UpVectorZ");
  populateDescription ("UpVectorX", upVecX);
  populateDescription ("UpVectorY", upVecY);
  populateDescription ("UpVectorZ", upVecZ);

  var va = graph.getValue ("ViewingAngle");
  var op = graph.getValue ("OrthogonalProjection");
  var ku = graph.getValue ("KeepUp");
  populateDescription ("ViewingAngle", va);
  document.getElementById("OrthogonalProjection").checked = (op == "true")?true:false;
  document.getElementById("KeepUp").checked = (ku == "true")?true:false;
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

function needTwoVars (plotno) {
  var dim  = graph.getValue ("Dimension");
  var pt   = graph.getPlotValue ("PlotType", plotno);
  var anim = graph.getPlotValue ("Animate",plotno);
  var nvars = PlotVarsNeeded (dim, pt, anim);
  return (nvars > 1);
/*                                                                                               */
/*                                                                                               */
/*   if (anim == "true") return true;                                                            */
/*   if (pt == "curve") return false;                                                            */
/*   if (dim[0] == "3") return true;                                                             */
/*   if ((pt == "rectangular") || (pt == "parametric") || (pt == "conformal") || (pt == "polar") */
/*       || (pt == "approximateIntegral"))                                                       */
/*     return false;                                                                             */
/*   return true;                                                                                */
}

function needThreeVars (plotno) {
  var dim  = graph.getValue ("Dimension");
  var pt   = graph.getPlotValue ("PlotType", plotno);
  var anim = graph.getPlotValue ("Animate", plotno);
  var nvars = PlotVarsNeeded (dim, pt, anim);
  return (nvars > 2);
/*                                                                         */
/*   if ((dim[0] == "2") && (anim != "true")) return false;                */
/*   if ((pt == "rectangular") || (pt == "parametric") || (pt == "tube")   */
/*      || (pt == "spherical") || (pt == "cylindrical")|| (pt == "curve")) */
/*     return false;                                                       */
/*   return true;                                                          */
  
}


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

