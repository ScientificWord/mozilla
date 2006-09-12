// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.
// Three data items are passed into this dialog: 
// window.arguments[0]: the graph object 
// window.arguments[1]: DOMGraph, the DOM element that should be replaced
// window.arguments[2]: the list of DOMGraphs currently being edited:
//    it only makes sense to have one dialog per <graph> element. Don't 
//    allow any others. 


// Populate the dialog with the current values stored in the Graph object.
// The element ids in ComputeGraphSettings.xul match the Graph attributes
// if the document has an element matching an attribute name, 
//   extract the value of the attribute and put it in the document
function Startup(){ 
  var alist = window.arguments[0].graphAttributeList();                    
  for (var i=0; i<alist.length; i++) {                       
    if (document.getElementById(alist[i])) {                    
      var value = window.arguments[0].getValue (alist[i]);                       
      document.getElementById(alist[i]).value = value;         
    }                                                       
  }                                                          
  // add the plots to the plot selection dialog
  var popup    = document.getElementById('plotnumber');
  var numPlots = window.arguments[0].getNumPlots();
  for(var i=1; i<=numPlots; i++) {                                          
    var newElement = document.createElement('menuitem');                    
    newElement.setAttribute("label", i.toString());                         
    newElement.setAttribute("value", i.toString());                         
    popup.appendChild(newElement);                                          
    if (i == 1) document.getElementById("plot").selectedItem = newElement;  
  }                                                                         
  window.arguments[0].setGraphAttribute("plotnumber", "1");

   //SLS the following copied from editor.js
  gSourceContentWindow = document.getElementById("content-frame");
  gSourceContentWindow.makeEditable("html", false);
  EditorStartup();

  // Initialize our source text <editor>
  try {
    gSourceTextEditor = gSourceContentWindow.getEditor(gSourceContentWindow.contentWindow);
    var controller = Components.classes["@mozilla.org/embedcomp/base-command-controller;1"]
                               .createInstance(Components.interfaces.nsIControllerContext);
    controller.init(null);
    controller.setCommandContext(gSourceContentWindow);
    gSourceContentWindow.contentWindow.controllers.insertControllerAt(0, controller);
    var commandTable = controller.QueryInterface(Components.interfaces.nsIInterfaceRequestor)
                                 .getInterface(Components.interfaces.nsIControllerCommandTable);
    commandTable.registerCommand("cmd_find",        nsFindCommand);
    commandTable.registerCommand("cmd_findNext",    nsFindAgainCommand);
    commandTable.registerCommand("cmd_findPrev",    nsFindAgainCommand);
    SetupMSIMathMenuCommands();
  } catch (e) { dump("makeEditable failed in Startup(): "+e+"\n"); }

  // see EditorSharedStartup() in editor.js
  var commandManager = GetCurrentCommandManager();
  commandManager.addCommandObserver(msiEditorDocumentObserverG, "obs_documentCreated");
}                                                                                            


// This part pastes data into the editor after the editor has started. 
// implements nsIObserver
var msiEditorDocumentObserverG =
{ 
  observe: function(aSubject, aTopic, aData)
  { 
    if (aTopic == "obs_documentCreated") {
      var plotno = window.arguments[0].getGraphAttribute("plotnumber");
      var editor = GetCurrentEditor();
      editor.addOverrideStyleSheet("chrome://editor/content/MathVarsDialog.css");
	  populateDialog (plotno);
    }  
  }
}


function tableRow4 (v, min, max, npts,chrome) {
  var str = "<tr>";
  if (chrome) classs = "label";    // class is a reserved word
  else classs = "value";                                                   
  str += "<td class=\"" + classs + "\">" + v    + "</td>";               
  str += "<td class=\"" + classs + "\">" + min  + "</td>";               
  str += "<td class=\"" + classs + "\">" + max  + "</td>";               
  str += "<td class=\"" + classs + "\">" + npts + "</td>";               
  str += "</tr>";        									   
  return str;
}


// return a string with the xml containing the expression and plot limits
function buildEditorTable (plotno) {
  var str = "";
  // put the expression in it's own table
  var curval = window.arguments[0].getPlotValue ("Expression", plotno);
  str += "<table class=\"MathVarsDialog\" chrome=\"1\" xmlns=\"http://www.w3.org/1999/xhtml\">";
  str += "<tbody>";
  str += "<tr>";                               
  str += "<td class=\"label\">";               
  str += "Plot expression";                               
  str += "</td></tr>";                              
  str += "<tr>";                               
  str += "<td class=\"value\">";               
  str += curval;                               
  str += "</td>";                              
  str += "</tr></tbody></table>";
  
  // put the ranges in a table
  var tmpstr = "<math xmlns=\"http://www.w3.org/1998/Math/MathML\"><mrow><mi tempinput=\"true\">()</mi></mrow></math>";
  str += "<BR/><BR/>";
  str += "<table chrome=\"1\" class=\"MathVarsDialog\" xmlns=\"http://www.w3.org/1999/xhtml\">";
  str += "<tbody>";
  str += tableRow4 ("Variable", "Min", "Max", "Sample Points", 1);
  
   // grab XMin, Xmax, Ymin, Ymax, Zmin, Zmax, Xvar, Yvar, Zvar, XNPTS, YNPTS
  var v, min, max, npts;                                           
  var ptype = window.arguments[0].getPlotValue ("PlotType", plotno);                      
  v    = window.arguments[0].getPlotValue ("XVar", plotno);                      
  if (ptype == "conformal")  v = "Re(" + v + ")";               
  min  = window.arguments[0].getPlotValue ("XMin", plotno);                      
  max  = window.arguments[0].getPlotValue ("XMax", plotno);                      
  npts = window.arguments[0].getPlotValue ("XPts", plotno);                      
  str += tableRow4 (v, min, max, npts, 0);

  if (needTwoVars (plotno)) {
    if (ptype == "conformal") { 
      v = "Im(" + v + ")";               
    } else {  
      v    = window.arguments[0].getPlotValue ("YVar", plotno);                      
    }  
    min  = window.arguments[0].getPlotValue ("YMin", plotno);                      
    max  = window.arguments[0].getPlotValue ("YMax", plotno);                      
    npts = window.arguments[0].getPlotValue ("YPts", plotno);                      
    str += tableRow4 (v, min, max, npts, 0);
                                                                 
    if (needThreeVars (plotno)) {
      v    = window.arguments[0].getPlotValue ("ZVar", plotno);                      
      min  = window.arguments[0].getPlotValue ("ZMin", plotno);                      
      max  = window.arguments[0].getPlotValue ("ZMax", plotno);                      
      npts = window.arguments[0].getPlotValue ("ZPts", plotno);                      
      str += tableRow4 (v, min, max, npts, 0);
    }  
  }
  if (ptype == "conformal") {
      var hpts = window.arguments[0].getPlotValue ("ConfHorizontalPts", plotno);                      
      var vpts = window.arguments[0].getPlotValue ("ConfVerticalPts", plotno);                      
      str += tableRow4 ("Horizontal Samples", "", "", hpts, 1);
      str += tableRow4 ("Vertical Samples", "", "", vpts, 1);
    }  

  str += "</tbody>";
  str += "</table>";

  if ((window.arguments[0].getPlotValue ("PlotType", plotno)) == "tube") {
    var curval = window.arguments[0].getPlotValue ("TubeRadius", plotno);
    str += "<table class=\"MathVarsDialog\" xmlns=\"http://www.w3.org/1999/xhtml\">";
    str += "<tbody>";
    str += "<tr>";                               
    str += "<td class=\"label\">";               
    str += "Tube Radius";                               
    str += "</td></tr>";                              
    str += "<tr>";                               
    str += "<td class=\"value\">";               
    str += curval;                               
    str += "</td>";                              
    str += "</tr></tbody></table>";
  }

  return str;
}



// Extract the values from the dialog and store them in the data structure
// Only save values that are not the defaults
function OK(){
  GetValuesFromDialog();
  window.arguments[0].setGraphAttribute("returnvalue", true);    
             
  // nonmodal, call the code that redraws. This dialog closes when
  // the return is executed, so ensure that happens, even if there
  // are problems.
  try {                                                                                         
    nonmodalRecreateGraph (window.arguments[0], window.arguments[1]);
  }                                                                                             
  catch (e) {                                                                                    
  }                                                  
  DOMGListRemove (window.arguments[1], window.arguments[2]);
  return true;
}                          

  
// Extract the values from the dialog and store them in the data structure
// Only save values that are not the defaults
function GetValuesFromDialog(){
  // grab anything that's in the graph attribute list
  var alist = window.arguments[0].graphAttributeList();                                
  for (var i=0; i<alist.length; i++) {                                   
    if (document.getElementById(alist[i])) {                            
      var newval = document.getElementById(alist[i]).value;            
      if (newval != "undefined") {   // NOTE, "" is OK, e.g. delete a label              
        var oldval = window.arguments[0].getValue (alist[i]);                        
        if (newval != oldval) {                                        
           window.arguments[0].setGraphAttribute(alist[i], newval);                  
        }                                                              
      }                                                                
    }                                                                   
  }                                                                               

  // grab anything that's in the plot attribute list
  var alist  = window.arguments[0].plotAttributeList();                                
  var plotno = window.arguments[0].getGraphAttribute("plotnumber");
  var dim    = window.arguments[0].getGraphAttribute("Dimension");
  for (var i=0; i<alist.length; i++) {                                   
    if (document.getElementById(alist[i])) {                            
      var newval = document.getElementById(alist[i]).value;            
      if ((newval != "") && (newval != "undefined")) {                 
        var oldval = window.arguments[0].getPlotValue (alist[i], plotno);                        
        if (newval != oldval) {                                        
           window.arguments[0].setPlotAttribute (PlotAttrName(alist[i], plotno), newval);
        }                                                              
      }                                                                
    }                                                                   
  } 
  var oldpt = window.arguments[0].getPlotValue ("PlotType", plotno);                        
  var newpt;
  if (dim == "2") {
    newpt = document.getElementById("pt2d").value;            
  } else { 
    newpt = document.getElementById("pt3d").value;            
  }
  if (newpt != oldpt) {
    window.arguments[0].setPlotAttribute (PlotAttrName("PlotType", plotno), newpt);
    window.arguments[0].setPlotAttribute (PlotAttrName("PlotStatus", plotno), "New");
  }    
  var oldanimate = window.arguments[0].getPlotValue ("Animate",plotno);
  var newanimate = document.getElementById("animate").checked;
  if (oldanimate != newanimate)
    window.arguments[0].setPlotAttribute (PlotAttrName("Animate", plotno), (newanimate)?"true":"false");
                                                                                

  // ITEMS PLOTTED TAB
  // now grab things out of the table in the center and add them to the graph
  // There are two tables: one has just the expression. The other has the ranges
  var doc = document.getElementById("content-frame").contentDocument;
  var tables = doc.getElementsByTagName("table");
  if (tables && tables.length > 1) {
    var exptable = tables[0];
	var rangetable = tables[1];
    // expression
    var mathnode = exptable.getElementsByTagName("math");
    puttableval ("Expression", mathnode[0], plotno);
	// ranges
    var rows = rangetable.getElementsByTagName ("tr");
    SaveTableVal ("X", rows[1], plotno);
    if (needTwoVars(plotno)) 
      SaveTableVal ("Y", rows[2], plotno);
    if (needThreeVars (plotno))
      SaveTableVal ("Z", rows[3], plotno);
    var ptype   = window.arguments[0].getPlotValue ("PlotType", plotno);
    if ((ptype == "tube") && (tables.length>2)) {
      var tr = tables[2].getElementsByTagName("math");
      puttableval ("TubeRadius", tr[0], plotno);
    }  
    if (ptype == "conformal") {
      var animate   = window.arguments[0].getPlotValue ("Animate",plotno);
      var i = (animate == "true") ? 4 : 3;
      SaveConformalSamples ("ConfHorizontalPts", rows[i], plotno);
      SaveConformalSamples ("ConfVerticalPts", rows[i+1], plotno);
    }
  }  
  else {
    dump ("WARNING: plotOK can't find the table\n");
  }
  // if new, query
  var status = window.arguments[0].getPlotValue ("PlotStatus", plotno);
  if (status == "New")
     window.arguments[0].computeQuery(plotno);

  // AXES TAB
  // GraphAxesScale
  if (document.getElementById("axisscale")) {                            
    var newval = document.getElementById("axisscale").selectedItem.value;
    var oldval = window.arguments[0].getValue ("AxisScale");  
    if (newval != oldval) {                                        
      window.arguments[0].setGraphAttribute ("AxisScale", newval);
    }                                                               
  }
  // GraphEqualScaling
  if (document.getElementById("equalscale")) {                            
    var newval = document.getElementById("equalscale").checked ? "true" : "false";
    var oldval = window.arguments[0].getValue ("EqualScaling");  
    if (newval != oldval) {                                        
      window.arguments[0].setGraphAttribute ("EqualScaling", newval);
    }                                                               
  }
  // GraphAxesTips
  if (document.getElementById("axestips")) {                            
    var newval = document.getElementById("axestips").checked ? "true" : "false";
    var oldval = window.arguments[0].getValue ("AxesTips");  
    if (newval != oldval) {                                        
      window.arguments[0].setGraphAttribute ("AxesTips", newval);
    }                                                               
  }
  // GraphGridLines
  if (document.getElementById("gridlines")) {                            
    var newval = document.getElementById("gridlines").checked ? "true" : "false";
    var oldval = window.arguments[0].getValue ("GridLines");  
    if (newval != oldval) {                                        
      window.arguments[0].setGraphAttribute ("GridLines", newval);
    }                                                               
  }
  // GraphAxesType
  if (document.getElementById("axistype")) {                            
    var index  = document.getElementById("axistype").selectedIndex;
    var newval = document.getElementById("axistype").selectedItem.value;
    var oldval = window.arguments[0].getValue ("AxesType"); 
    if (newval != oldval) {                                        
      window.arguments[0].setGraphAttribute ("AxesType", newval);
    }                                                               
  }

  // Layout Tab
  // printAttribute
  if (document.getElementById("printattr")) {                            
    var newval = document.getElementById("printattr").selectedItem.value;
    var oldval = window.arguments[0].getValue ("PrintAttribute"); 
    if (newval != oldval) {                                        
      window.arguments[0].setGraphAttribute ("PrintAttribute", newval);
    }                                                               
  }
  // ScreenDisplay
  if (document.getElementById("screendisplayattr")) {                            
    var newval = document.getElementById("screendisplayattr").selectedItem.value;
    var oldval = window.arguments[0].getValue ("PrintFrame"); 
    if (newval != oldval) {                                        
      window.arguments[0].setGraphAttribute ("PrintFrame", newval);
    }                                                               
  }
  // GraphAxesType
  if (document.getElementById("placement")) {                            
    var newval = document.getElementById("placement").selectedItem.value;
    var oldval = window.arguments[0].getValue ("Placement"); 
    if (newval != oldval) {                                        
      window.arguments[0].setGraphAttribute ("Placement", newval);
    }                                                               
  }
  // Labelling Tab
  // Captionplacement
  if (document.getElementById("captionplacement")) {                            
    var newval = document.getElementById("captionplacement").selectedItem.value;
    var oldval = window.arguments[0].getValue ("CaptionPlace"); 
    if (newval != oldval) {                                        
      window.arguments[0].setGraphAttribute ("CaptionPlace", newval);
    }                                                               
  }
  // View Tab
  // Orientation
  var camLocX = document.getElementById("CameraLocationX").value;
  var camLocY = document.getElementById("CameraLocationY").value;
  var camLocZ = document.getElementById("CameraLocationZ").value;
  window.arguments[0].setGraphAttribute ("CameraLocationX", camLocX);
  window.arguments[0].setGraphAttribute ("CameraLocationY", camLocY);
  window.arguments[0].setGraphAttribute ("CameraLocationZ", camLocZ);

  var focalPtX = document.getElementById("FocalPointX").value;
  var focalPtY = document.getElementById("FocalPointY").value;
  var focalPtZ = document.getElementById("FocalPointZ").value;
  window.arguments[0].setGraphAttribute ("FocalPointX", focalPtX);
  window.arguments[0].setGraphAttribute ("FocalPointY", focalPtY);
  window.arguments[0].setGraphAttribute ("FocalPointZ", focalPtZ);

  var upVecX = document.getElementById("UpVectorX").value;
  var upVecY = document.getElementById("UpVectorY").value;
  var upVecZ = document.getElementById("UpVectorZ").value;
  window.arguments[0].setGraphAttribute ("UpVectorX", upVecX);
  window.arguments[0].setGraphAttribute ("UpVectorY", upVecY);
  window.arguments[0].setGraphAttribute ("UpVectorZ", upVecZ);

  var va = document.getElementById("ViewingAngle").value;
  var op = document.getElementById("OrthogonalProjection").checked ? "true" : "false";
  var ku = document.getElementById("KeepUp").checked ? "true" : "false";
  window.arguments[0].setGraphAttribute ("ViewingAngle", va);
  window.arguments[0].setGraphAttribute ("OrthogonalProjection", op);
  window.arguments[0].setGraphAttribute ("KeepUp", ku);
}                          


// extract values from the edit table for a variable: varname, max, min, npts
// axis is "X", "Y", or "Z". 
// row is a table row with four elements
function SaveTableVal (axis, row, plotno) {
  var v    = axis + "Var";  
  var min  = axis + "Min";  
  var max  = axis + "Max";  
  var npts = axis + "Pts"; 
  var mathnode;
  try {
    var cols = row.getElementsByTagName ("td");
    mathnode = cols[0].getElementsByTagName("math");
    puttableval (v, mathnode[0], plotno);
    mathnode = cols[1].getElementsByTagName("math");
    puttableval (min, mathnode[0], plotno);
    mathnode = cols[2].getElementsByTagName("math");
    puttableval (max, mathnode[0], plotno);
    mathnode = cols[3].getElementsByTagName("math");
    puttableval (npts, mathnode[0], plotno);
  }
  catch (e) {
    alert ("ERROR: Unable to save values in plot table\n");
  }  
}

function puttableval (name, mathnode, plotno) {   
  if ((mathnode == null) || (HasEmptyMath(mathnode))) {
        value = "";
      }
  else {
    var oldval = window.arguments[0].getPlotValue (name, plotno);   
	// workaround: clean to eliminate <mn/>
    var newval = CleanMathString (GetMathAsString (mathnode));
	// workaround: run fixup twice
    //newval = runFixup(runFixup(newval));
    newval = runFixup(newval);
    if (oldval != newval) {
      window.arguments[0].setPlotAttribute (PlotAttrName(name, plotno), newval);
    }
  }
}

// for conformals, save the horizontal and vertical samples
// row is a table row with four elements
function SaveConformalSamples (name, row, plotno) {
  try {
    var cols = row.getElementsByTagName ("td");
    var newval = ExtractTextFromNode(cols[3]);
    var oldval = window.arguments[0].getPlotValue (name, plotno);   
    if (oldval != newval) {
      window.arguments[0].setPlotAttribute (PlotAttrName(name, plotno), newval);
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
  window.arguments[0].setGraphAttribute("returnvalue", false);
  DOMGListRemove (window.arguments[1], window.arguments[2]);
}

// This is the callback for the command button to add a new plot
// Create a new plot, fire up the ComputePlotSettings.xul edit dialog. 
// Set the PlotStatus to New so OK can call the preparePlot function
function addPlot () {
  // save any changes to current plot, then change plots. Cancel ignores all changes
  GetValuesFromDialog();
  window.arguments[0].setGraphAttribute("returnvalue", false);                 
  addPlotDialogContents();
}


function addPlotDialogContents () {
  var plotnumber = window.arguments[0].addPlot();
  window.arguments[0].setGraphAttribute("plotnumber", plotnumber);
  var popup  = document.getElementById('plotnumber');
  var newElement = document.createElement('menuitem');
  newElement.setAttribute("label", plotnumber.toString());
  newElement.setAttribute("value", plotnumber.toString());
  popup.appendChild(newElement);
  document.getElementById("plot").selectedItem = newElement; 
  // grab the plottype from plot 1 and set it as default
  var oldtype = window.arguments[0].getPlotValue ("PlotType", 1);
  if (oldtype == "") oldtype = "rectangular";
  window.arguments[0].setPlotAttribute (PlotAttrName("PlotType", plotnumber), oldtype);
  window.arguments[0].setPlotAttribute (PlotAttrName("PlotStatus", plotnumber), "New");
  populateDialog (plotnumber);   
}         



// This is the callback for the command button to edit a plot
// on entry, the "plotnumber" widget has the plot number of the items to be edited. 
// on exit, the ComputePlotSettings.xul dialog has saved the new data 
function formatPlot () {
  // only open one dialog per window
  if (DOMGListMemberP (window, window.arguments[2])) {
    return;                                                                           
  }
  DOMGListAdd (window, window.arguments[2]);
  var count = document.getElementById("plot").selectedItem.value; 
  window.arguments[0].setGraphAttribute("plotnumber", count);
  window.openDialog("chrome://editor/content/ComputePlotSettings.xul", 
                    "Plot_Settings", "chrome,close,titlebar,dependent", 
                    window.arguments[0], window, window.arguments[2]);
}

// Mark a plot as deleted and set the dialog to the next available plot
function deletePlot () {
  // extract the plot number from the dialog
  var plotno = document.getElementById("plot").selectedItem.value; 
  // Set status to Deleted and delete the plot
  window.arguments[0].setPlotAttribute (PlotAttrName("PlotStatus", plotno), "Deleted");
  // remove this from the plotnumber dialog
  var popup    = document.getElementById('plotnumber');                    
  for (var idx=0; idx<popup.childNodes.length; idx++) {                    
    if (popup.childNodes[idx].value == plotno) popup.removeChild(popup.childNodes[idx]);                                   
  }                                                                        
  //  window.arguments[0].deletePlot();

  // find the next plot if there is one, or the previous plot if there is one
  var numplots = window.arguments[0].getNumPlots();
  var newplot;
  for (newplot = plotno; 
       ((newplot <=numplots) && 
        (window.arguments[0].getPlotAttribute (PlotAttrName ("PlotStatus", newplot)) == "Deleted"));
       newplot++);
  if (newplot > numplots) {
    for (newplot = plotno; 
         ((newplot > 0) && 
          (window.arguments[0].getPlotAttribute (PlotAttrName ("PlotStatus", newplot)) == "Deleted"));
         newplot--);
  }      
  if (newplot == 0) {   // no undeleted plots, add one.
    addPlotDialogContents ()
  } else {              // use the plot we just found
    window.arguments[0].setGraphAttribute("plotnumber", newplot);
    populateDialog (newplot);
  }
}

// the user has just selected a new plot type for this plot. Rebuild the 
// dialog
function changePlotType () {
  // get the plot number and the plot type. Then populate
  var dim    = window.arguments[0].getGraphAttribute("Dimension");
  var oldpt = window.arguments[0].getPlotValue ("PlotType", plotno);                        
  var plotno = document.getElementById("plot").selectedItem.value; 
  if ((plotno == null) || (plotno == "")) {              
    plotno = 1;
  }
  window.arguments[0].setGraphAttribute("plotnumber", plotno);
  var newpt;
  if (dim == "2") {
    newpt = document.getElementById("pt2d").value;            
  } else { 
    newpt = document.getElementById("pt3d").value;            
  }
  if (newpt != oldpt) {
    window.arguments[0].setPlotAttribute (PlotAttrName("PlotType", plotno), newpt);
    window.arguments[0].setPlotAttribute (PlotAttrName("PlotStatus", plotno), "New");
  }    
  populateDialog (plotno);
}


// the user has just selected a new plot number in the dialog: populate the 
// current screen with the data for this plot
function changePlot () {
  // save any changes to this plot, then change plots. Cancel ignores all changes
  GetValuesFromDialog();
  window.arguments[0].setGraphAttribute("returnvalue", false);                 

  // extract the plot number from the dialog
  var plotno = document.getElementById("plot").selectedItem.value; 
  if ((plotno == null) || (plotno == "")) {              
    plotno = 1;
  }
  window.arguments[0].setGraphAttribute("plotnumber", plotno);
  populateDialog (plotno);
}

function populateDialog (plotno) {
  // remove contents here
  var doc = document.getElementById("content-frame").contentDocument;
  var body = doc.getElementsByTagName("table");
  while (body.length>0) {
    body[0].parentNode.removeChild (body[0]);
  }
  var str = buildEditorTable (plotno);
  var editor = GetCurrentEditor();
  editor.insertHTML(str);
  // now mark the body as in the chrome, to prevent resize widgets around the table.
  
  body = doc.getElementsByTagName("body");
  if (body.length > 0)
    body[0].setAttribute("chrome","1");
    
  // handle the top descriptor
  var dim = window.arguments[0].getValue ("Dimension");
  populateDescription ("dimension", dim);
  var ptype = window.arguments[0].getPlotValue ("PlotType", plotno);
  if (dim == "2") {
    document.getElementById("plotcs2d").collapsed = false;            
    document.getElementById("plotcs3d").collapsed = true;            
    populatePopupMenu ("pt2d", ptype);  
  } else {
    document.getElementById("plotcs3d").collapsed = false;            
    document.getElementById("plotcs2d").collapsed = true;            
    populatePopupMenu ("pt3d", ptype);
  }
  var animatevalue = window.arguments[0].getPlotValue ("Animate",plotno);
  document.getElementById("animate").checked = (animatevalue == "true")?true:false;
  //populateDescription ("animate", window.arguments[0].getPlotValue ("Animate",plotno));

  // put the current plot number into the dialog box
  var plotml = document.getElementById("plotnumber");
  for (var idx=0; idx<plotml.childNodes.length; idx++) { 
    if (plotml.childNodes[idx].getAttribute("value") == plotno) {
      document.getElementById("plot").selectedItem = plotml.childNodes[idx]; 
    }    
  }       

  // AXES TAB
  // GraphAxesScale
  var oldval = window.arguments[0].getValue ("AxisScale"); 
  radioGroupSetCurrent ("axisscale", oldval);
  // GraphEqualScaling
  if (document.getElementById("equalscale")) {  
    var oldval = window.arguments[0].getValue ("EqualScaling");  
    if (oldval == "true") {
       document.getElementById("equalscale").checked = true;            
    }                                                               
  }
  // GraphAxesTips
  if (document.getElementById("axestips")) {                            
    var oldval = window.arguments[0].getValue ("AxesTips");  
    if (oldval == "true") {
       document.getElementById("axestips").checked = true;            
    }                                                               
  }
  // GraphGridLines
  if (document.getElementById("gridlines")) {                            
    var oldval = window.arguments[0].getValue ("GridLines");  
    if (oldval == "true") {
       document.getElementById("gridlines").checked = true;            
    }                                                               
  }
  // GraphAxesType
  var oldval = window.arguments[0].getValue ("AxesType");  
  radioGroupSetCurrent ("axistype", oldval);
  // Layout Tab
  // printAttribute
  var oldval = window.arguments[0].getValue ("PrintAttribute");  
  radioGroupSetCurrent ("printattr", oldval);
  // ScreenDisplay
  var oldval = window.arguments[0].getValue ("PrintFrame");  
  radioGroupSetCurrent ("screendisplayattr", oldval);
  // GraphAxesType
  var oldval = window.arguments[0].getValue ("Placement");  
  radioGroupSetCurrent ("placement", oldval);

  // Labelling Tab
  // Captionplacement
  var oldval = window.arguments[0].getValue ("CaptionPlace");  
  radioGroupSetCurrent ("captionplacement", oldval);

  // Axes tab
  var camLocX = window.arguments[0].getValue ("CameraLocationX");
  var camLocY = window.arguments[0].getValue ("CameraLocationY");
  var camLocZ = window.arguments[0].getValue ("CameraLocationZ");
  populateDescription ("CameraLocationX", camLocX);
  populateDescription ("CameraLocationY", camLocY);
  populateDescription ("CameraLocationZ", camLocZ);

  var focalPtX = window.arguments[0].getValue ("FocalPointX");
  var focalPtY = window.arguments[0].getValue ("FocalPointY");
  var focalPtZ = window.arguments[0].getValue ("FocalPointZ");
  populateDescription ("FocalPointX", focalPtX);
  populateDescription ("FocalPointY", focalPtY);
  populateDescription ("FocalPointZ", focalPtZ);

  var upVecX = window.arguments[0].getValue ("UpVectorX");
  var upVecY = window.arguments[0].getValue ("UpVectorY");
  var upVecZ = window.arguments[0].getValue ("UpVectorZ");
  populateDescription ("UpVectorX", upVecX);
  populateDescription ("UpVectorY", upVecY);
  populateDescription ("UpVectorZ", upVecZ);

  var va = window.arguments[0].getValue ("ViewingAngle");
  var op = window.arguments[0].getValue ("OrthogonalProjection");
  var ku = window.arguments[0].getValue ("KeepUp");
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
  var dim  = window.arguments[0].getValue ("Dimension");
  var pt   = window.arguments[0].getPlotValue ("PlotType", plotno);
  var anim = window.arguments[0].getPlotValue ("Animate",plotno);
  var nvars = CountPlotVars (dim, pt, anim);
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
  var dim  = window.arguments[0].getValue ("Dimension");
  var pt   = window.arguments[0].getPlotValue ("PlotType", plotno);
  var anim = window.arguments[0].getPlotValue ("Animate", plotno);
  var nvars = CountPlotVars (dim, pt, anim);
  return (nvars > 2);
/*                                                                         */
/*   if ((dim[0] == "2") && (anim != "true")) return false;                */
/*   if ((pt == "rectangular") || (pt == "parametric") || (pt == "tube")   */
/*      || (pt == "spherical") || (pt == "cylindrical")|| (pt == "curve")) */
/*     return false;                                                       */
/*   return true;                                                          */
  
}



function GetGraphColor (attributeName)
{
  // Don't allow a blank color, i.e., using the "default"
  var colorObj = { NoDefault:true, Type:"", TextColor:0, PageColor:0, Cancel:false };
  var oldcolor = window.arguments[0].getValue (attributeName);                        
  if (oldcolor != "") {
     colorObj.TextColor = oldcolor;
     colorObj.PageColor = oldcolor;
  }
  window.openDialog("chrome://editor/content/EdColorPicker.xul", "_blank", "chrome,close,titlebar,modal", "", colorObj);

  // User canceled the dialog
  if (colorObj.Cancel)
    return;

  var color = colorObj.TextColor;
  window.arguments[0].setGraphAttribute (attributeName, color);
}
