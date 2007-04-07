// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.
// Three data items are passed into this dialog: 
// window.arguments[0]: the graph object 
// window.arguments[1]: DOMGraph, the DOM element that should be replaced
// window.arguments[2]: the list of DOMGraphs currently being edited:
//    it only makes sense to have one dialog per <graph> element. Don't 
//    allow any others. 

var graph;

function VCAMEditorOnLoad() {
  dump("SMR VCAMEditorOnLoad\n");
  graph = window.arguments[0];
  var plotFileName = graph.getValue ("ImageFile");
  dump("SMR VCAMEditorOnLoad plot file name is " + plotFileName + "\n");
  
  var plot = document.getElementById('plot');
  var cam = plot.camera;
      
  var i;
  i = document.getElementById('animateReset');
  i.onclick = animateReset;
  i = document.getElementById('animateStart');
  i.onclick = animateStart;
  i = document.getElementById('animateStop');
  i.onclick = animateStop;
  i = document.getElementById('animateEnd');
  i.onclick = animateEnd;

  i = document.getElementById('cameraGet');
  i.onclick = cameraGet;
  i = document.getElementById('cameraSet');
  i.onclick = cameraSet;
  i = document.getElementById('cameraReset');
  i.onclick = cameraReset;
  i = document.getElementById('cameraFit');
  i.onclick = cameraFit;

  i = document.getElementById('cameraOrtho');
  i.onclick = cameraOrtho;
  i = document.getElementById('cameraKeepUp');
  i.onclick = cameraUpVector;

  document.getElementById('cameraPosX').onchange  = cameraSet;  
  document.getElementById('cameraPosY').onchange  = cameraSet;  
  document.getElementById('cameraPosZ').onchange  = cameraSet;  
  document.getElementById('focX').onchange        = cameraSet;  
  document.getElementById('focY').onchange        = cameraSet;  
  document.getElementById('focZ').onchange        = cameraSet;  
  document.getElementById('upX').onchange         = cameraSet;  
  document.getElementById('upY').onchange         = cameraSet;  
  document.getElementById('upZ').onchange         = cameraSet;  
  document.getElementById('cameraAngle').onchange = cameraSet;  
  document.getElementById('cameraRotateLeft').onclick  = cameraRotateLeft;  
  document.getElementById('cameraRotateStop').onclick  = cameraRotateStop;  
  document.getElementById('cameraRotateRight').onclick = cameraRotateRight;  
  document.getElementById('cameraRotateUp').onclick    = cameraRotateUp;  
  document.getElementById('cameraRotate2Stop').onclick = cameraRotate2Stop;  
  document.getElementById('cameraRotateDown').onclick  = cameraRotateDown;  
  document.getElementById('cameraZoomIn').onclick      = cameraZoomIn;  
  document.getElementById('cameraZoomStop').onclick    = cameraZoomStop;  
  document.getElementById('cameraZoomOut').onclick     = cameraZoomOut;  

  i = document.getElementById('url');
  i.onchange = loadUrl;
  i = document.getElementById('URLLoad');
  i.onchange = loadUrl;
  dump("SMR ComputeVcamSettings 68, plot file is " + plotFileName + "\n");
  plot.load ("file:///" + plotFileName);
  dump("SMR ComputeVcamSettings after plot.load\n");
  plot.addEvent('readyStateChange', onStateChange);
  dump("SMR ComputeVcamSettings addEvent\n");
  plot.addEvent('queryNoHit', onNoHit);
  dump("SMR ComputeVcamSettings NoHit\n");
  msiReadGraph();
  dump("SMR ComputeVcamSettings after msiReadGraph()\n");
}


//-----------------------------------------------------

function animateReset() { 
  plot.currentTime = plot.beginTime;
}

function animateStart() { 
  plot.startAnimation();
}

function animateStop() { 
  plot.stopAnimation();
}

function animateEnd() {
  plot.currentTime = plot.endTime;
}

function animateSpeed() { 
  var i = document.getElementById('animateSpeed');
  var val = i.selectedItem.value - 0;
  plot.animationSpeed = val;
}

function animateLoop() { 
  var i = document.getElementById('animateLoop');
  var val = i.selectedItem.value - 0;
  plot.animationLoopingMode = val;
}

function CBcursorTool() { 
  var val = document.getElementById("cursorTool").selectedItem.value;
  onNoHit(); 
  plot.cursorTool = val;
}

function cursorSpeed() { 
  var val = document.getElementById("PUCursorSpeed").selectedItem.value;
  onNoHit(); 
  alert ("setting speed to " + val + ":  ERROR IN PLUGIN INTERFACE HERE");
  // plot.actionSpeed = val;
}

function cameraGet() { 
  var cam = plot.camera;
  document.getElementById('cameraPosX').value = '' + cam.positionX;
  document.getElementById('cameraPosY').value = '' + cam.positionY;
  document.getElementById('cameraPosZ').value = '' + cam.positionZ;

  document.getElementById('focX').value = '' + cam.focalPointX;
  document.getElementById('focY').value = '' + cam.focalPointY;
  document.getElementById('focZ').value = '' + cam.focalPointZ;

  document.getElementById('upX').value = '' + cam.upVectorX;
  document.getElementById('upY').value = '' + cam.upVectorY;
  document.getElementById('upZ').value = '' + cam.upVectorZ;
  
  document.getElementById('cameraAngle').value = '' + cam.viewingAngle;
  
  document.getElementById('cameraOrtho').checked = cam.orthogonalProjection;
  document.getElementById('cameraKeepUp').checked = cam.keepUpVector;
}

function cameraSet() { 
  var cam = plot.camera;
  cam.positionX = document.getElementById('cameraPosX').value - 0;
  cam.positionY = document.getElementById('cameraPosY').value - 0;
  cam.positionZ = document.getElementById('cameraPosZ').value - 0;

  cam.focalPointX = document.getElementById('focX').value - 0;
  cam.focalPointY = document.getElementById('focY').value - 0;
  cam.focalPointZ = document.getElementById('focZ').value - 0;

  cam.upVectorX = document.getElementById('upX').value - 0;
  cam.upVectorY = document.getElementById('upY').value - 0;
  cam.upVectorZ = document.getElementById('upZ').value - 0;
  
  cam.viewingAngle = document.getElementById('cameraAngle').value - 0;
  
  cam.orthogonalProjection = document.getElementById('cameraOrtho').checked;
  cam.keepUpVector = document.getElementById('cameraKeepUp').checked;
  msiWriteGraph ();
}

function cameraReset() { 
  plot.resetViewpoint(); 
  cameraGet(); 
}

function cameraFit() { 
  plot.fitContents(); 
  cameraGet();
}

function cameraOrtho() {
  cam.orthogonalProjection = document.getElementById('cameraOrtho').checked;
}

function cameraUpVector() { 
  var val = document.getElementById('cameraKeepUp').checked;
  alert("setting up " + val);
  cam.keepUpVector = document.getElementById('cameraKeepUp').checked;
}

function cameraRotateLeft () {
  plot.rotateVerticalAction = -1;
}  

function cameraRotateStop () { 
  plot.rotateVerticalAction = 0;
}

function cameraRotateRight () {
  plot.rotateVerticalAction = 1;
}

function cameraRotateUp () {  
  plot.rotateHorizontalAction = 1;
}

function cameraRotate2Stop () {
  plot.rotateHorizontalAction = 0;
}

function cameraRotateDown () { 
  plot.rotateHorizontalAction = -1;
}

function cameraZoomIn () {  
  plot.zoomAction = 1;
}

function cameraZoomStop () {  
  plot.zoomAction = 0;
}

function cameraZoomOut () {  
  plot.zoomAction = -1;
}


function loadUrl() { 
  plot.load(document.getElementById('url').value); 
}

//-----------------------------------------------------
//-----------------------------------------------------

// should happen just once, after an image is loaded
function onStateChange(s) {
    if (s == 2) { // loaded
        if (plot.dimension != 3) {
            alert("Not a 3D Graphics.");
        } else {
            if (plot.isAnimated) {
               document.getElementById('animationControl').collapsed = false;
            }   
            cameraGet();
            msiReadGraph();
        }
    }
}


function onPositionChange() {
  alert("Position change!");
}


function onNoHit() {
//  document.getElementById('queryX').value = '';
//  document.getElementById('queryY').value = '';
//  document.getElementById('queryZ').value = '';
}


function onTreeChange() {
  alert("Tree change!");
}


function updateVals(event) {
  cameraGet();
}

// read the DOM, create a graph object, and use it to set initial values in vcam
function msiReadGraph () {
  var x = graph.getValue ("CameraLocationX") - 0;
  var y = graph.getValue ("CameraLocationY") - 0;
  var z = graph.getValue ("CameraLocationZ") - 0;
  if ((x==0) && (y==0) &&(z==0)) {
    // use defaults
  } else {
    cam.positionX = x;
    cam.positionY = y;
    cam.positionZ = z;
  }
    
  var angle = graph.getValue ("ViewingAngle") - 0;
  if (angle != 0)
    cam.viewingAngle = angle;
  
  x = graph.getValue ("FocalPointX") - 0;
  y = graph.getValue ("FocalPointY") - 0;
  z = graph.getValue ("FocalPointZ") - 0;
  if ((x==0) && (y==0) &&(z==0)) {
    // use defaults
  } else {
    cam.focalPointX = x;
    cam.focalPointY = y;
    cam.focalPointZ = z;
  }

  x = graph.getValue ("UpVectorX") - 0;
  y = graph.getValue ("UpVectorY") - 0;
  z = graph.getValue ("UpVectorZ") - 0;
  if ((x==0) && (y==0) &&(z==0)) {
    // use defaults
  } else {
    cam.upVectorX = x;
    cam.upVectorY = y;
    cam.upVectorZ = z;
  }
  
  var flag = false;
  var flag = graph.getValue ("OrthogonalProjection");
  cam.orthogonalProjection = (flag==true);
  flag = graph.getValue ("ViewingAngle");
  cam.keepUpVector = (flag==true);
}        

// replace current <graph> with new one
function msiWriteGraph () {
  var cam = plot.camera;
  graph.setGraphAttribute ("CameraLocationX", cam.positionX);
  graph.setGraphAttribute ("CameraLocationY", cam.positionY);
  graph.setGraphAttribute ("CameraLocationZ", cam.positionZ);

//                                     "FocalPointX", "FocalPointY", "FocalPointZ",
//                                     "UpVectorX", "UpVectorY", "UpVectorZ",
//                                     "ViewingAngle", "OrthogonalProjection, KeepUp",

  var newgraph = graph.createGraphDOMElement (false);
  newgraph.setAttribute ("id","graphelement");
  newgraph.setAttribute ("sr","new child");
  var ge = document.getElementById("graphelement");
  var parent = ge.parentNode;
  parent.removeChild (ge);
  parent.appendChild (newgraph);
}        

//==============================================================================

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

}                                                                                            





// Extract the values from the dialog and store them in the data structure
// Only save values that are not the defaults
function OK(){
  GetValuesFromDialog();
  window.arguments[0].setGraphAttribute("returnvalue", true);    
             
  // nonmodal, call the code that redraws. This dialog closes when
  // the return is executed, so ensure that happens, even if there
  // are problems.
  var editorElement = msiGetParentEditorElementForDialog(window);
  try {                                                                                         
    nonmodalRecreateGraph (window.arguments[0], window.arguments[1], editorElement);
  }                                                                                             
  catch (e) {                                                                                    
  }                                                  
  DOMGListRemove (window.arguments[1], window.arguments[2]);
  return true;
}                          

  
// Extract the values from the dialog and store them in the data structure
// Only save values that are not the defaults
function GetValuesFromDialog(){
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
//  addPlotDialogContents();
}

