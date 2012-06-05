
Components.utils.import("resource://app/modules/os.jsm");

const xhtmlns  = "http://www.w3.org/1999/xhtml";

var gPrefsService;
var gPrefsBranch;
var isRunning = false;
var currdoc = null;
var gProgressbar;

function currObj() {
  return currdoc.getElementById("msi-current");
}


function addClickEventListenerForBrowserElement(browserElement)
{
  try {
    if (browserElement && browserElement.contentDocument)
    {
      if (currdoc != browserElement.contentDocument)
      {
        currdoc = browserElement.contentDocument;
        currdoc.addEventListener("click", msiHelpHandleClick, true);
      }
    }
  } catch (e) {dump("Unable to register browser element click event listener; error [" + e + "].\n");}
}

function msiGetBaseNodeName(node)
{
  return (node && node.localName) || null;
}

function getEventParentByTag( event, tagname)
{
	var node = event.target;
	while (node && node.localName !== tagname) node = node.parentNode;
	if (node && node.localName === tagname)
	  return node;
  return null;
}

function GetPrefsService()
{
  if (gPrefsService)
    return gPrefsService;
  try {
  }
  catch(ex) {
    dump("failed to get prefs service!\n");
  }
  return gPrefsService;
}

function GetPrefs()
{
  if (gPrefsBranch)
    return gPrefsBranch;

  try {
    var prefService = GetPrefsService();
    if (prefService)
      gPrefsBranch = prefService.getBranch(null);

    if (gPrefsBranch)
      return gPrefsBranch;
    else
      dump("failed to get root prefs!\n");
  }
  catch(ex) {
    dump("failed to get root prefs!\n");
  }
  return null;
}

//This is called from the nsIWebProgressListener onStateChange() message //handler
function msiCheckLoadStatus(aWebProgress, aRequest, aStateFlags, aStatus)
{
  var stateStopFlag = 0x0010;
  var isDocumentFlag = 0x00020000;
  var flagsWeWant = stateStopFlag|isDocumentFlag;
  if ((aStateFlags & stateStopFlag) && (aStateFlags & isDocumentFlag))
  {
    addClickEventListenerForBrowserElement(document.getElementById("help-content"));
  }
  else
    dump("In msiCheckLoadStatus, stateflags are " + aStateFlags.toString(16) + " and flagsWeWant are " + flagsWeWant.toString(16) + ".\n");
}


function msiHelpHandleClick(event)
{
  if (!event)
    return;

#ifdef PROD_SW
  return;
#endif

  if (event.detail == 1)
  {
    if ((currObj() === event.target) && (document.getElementById("vcamactive").getAttribute("hidden") == "false")) return;
    var isPlot = false;
    var oldobj;
    var node;
    var objName = event.target.localName;
    var graphnode;
    if (oldobj = currdoc.getElementById("msi-current")) oldobj.removeAttribute("msi-current");
    if (objName !== "object")
    {
      graphnode = getEventParentByTag(event, "plotwrapper");
      if (graphnode) 
      {
        node = graphnode.getElementsByTagName("object")[0];
        if (currObj() === node ) return;
      }
    }
    else node = event.target;
    if (node) 
    {
        if (node.getAttribute("msigraph") === "true")
        {
          isPlot = true;
        }
     }
    else {
      if (event.target.getAttribute("msigraph") == "true")
      {
        isPlot = true;
      }
    }
    if (isPlot)
    {
      node.setAttribute("id","msi-current");
      doVCamInitialize(node);
	    document.getElementById("vcamactive").setAttribute("hidden",false);
    }
    else
    {
	    document.getElementById("vcamactive").setAttribute("hidden",true);
      if (oldobj = currdoc.getElementById("msi-current")) oldobj.removeAttribute("msi-current");
    }
  }
}


function msiHelpInit()
{
  var browserElement = document.getElementById("help-content");
  currdoc = browserElement.contentWindow;
  addClickEventListenerForBrowserElement(browserElement);
  netscape.security.PrivilegeManager.enablePrivilege('UniversalXPConnect');

#ifndef PROD_SW
  // BBM: this has to be a run-time test.
   var wrapperList = currdoc.getElementsByTagName("object");
  for (var ii = 0; ii < wrapperList.length; ++ii)
  {
    if (wrapperList[ii].getAttribute("msigraph") == "true")
      doVCamPreInitialize(wrapperList[ii]);
  }
#endif
}

#ifndef PROD_SW

function makeSnapshotPathAbsolute(relpath, browserElement) {
  var longfilename;
  var leaf;
  try {
    var propService = Components.classes["@mozilla.org/file/directory_service;1"].getService(Components.interfaces.nsIProperties);
    var tempDir = propService.get("TmpD", Components.interfaces.nsIFile);
    
//    var documentfile;
//    var docauxdirectory;
//    var currdocdirectory;
//    var url = browserElement.currentURI;
//    documentfile = url.QueryInterface(Components.interfaces.nsIFileURL).file;

//    currdocdirectory = documentfile.parent.clone();
    var pathParts = relpath.split("/");
    tempDir.append(pathParts[pathParts.length - 1]);
//    var i;
//    for (i = 0; i < pathParts.length; i++) {
//      currdocdirectory.append(pathParts[i]);
//    }
    longfilename = tempDir.path;
  } catch (e) {
    dump("Error: " + e + "\n");
  }
  return longfilename;
}

function getElementOrParentByTagName(tagName, node)
{
  var retNode = null;
  var nextNode = node;
  while (!retNode && nextNode)
  {
    if (nextNode.localName == tagName)
      retNode = nextNode;
    nextNode = nextNode.parentNode;
  }
  return retNode;
}

function vcamToolbarFromPlugin()
{
  if (currObj() == null) return;
  var ct = currObj().cursorTool;
  var dim = currObj().dimension;
  var anim = currObj().isAnimated;
  var za = currObj().zoomAction;
  document.getElementById("vc-SelObj").checked = (ct === 'select');
  document.getElementById("vc-RotateScene").checked = (ct === 'rotate');
  document.getElementById("vc-Move").checked = (ct === 'move');
  document.getElementById("vc-Query").checked = (ct === 'query');
  document.getElementById("vc-AutoZoomIn").checked = (za === 1);
  document.getElementById("vc-AutoZoomOut").checked = (za === 2);
    // Rotation section
  if (dim === 3) {
    document.getElementById("vc-Zoom").checked = (ct === 'zoom');
    var va = currObj().rotateVerticalAction;
    document.getElementById("vc-RotateRight").checked = (va === 1);
    document.getElementById("vc-RotateLeft").checked = (va === 2);
    var ha = currObj().rotateHorizontalAction;
    document.getElementById("vc-RotateUp").checked = (ha === 1);
    document.getElementById("vc-RotateDown").checked = (ha === 2);
    // Zoom section
  //  document.getElementById("AutoZoomIn").checked = (za === 1);
  //  document.getElementById("AutoZoomOut").checked = (za === 2);
    // Speed controlf
    var spd = currObj().actionSpeed;
    if (spd <= 0.125) document.getElementById("actionspeed8s").checked = true;
    else if (spd > 0.125 && spd <= 0.25) 
      document.getElementById("actionspeed4s").checked = true;
    else if (spd > 0.25 && spd <= 0.5) 
      document.getElementById("actionspeed2s").checked = true;
    else if (spd > 0.5 && spd <= 1.0) 
      document.getElementById("actionspeedN").checked = true;
    else if (spd > 1.0 && spd <= 2.0) 
      document.getElementById("actionspeed2f").checked = true;
    else if (spd > 2.0 && spd <= 4.0) 
      document.getElementById("actionspeed4f").checked = true;
    else if (spd > 4.0) 
      document.getElementById("actionspeed8f").checked = true;
  }  
  if (anim) { 
  // animation section, including animation speed control
    var aspd = currObj().animationSpeed;
    if (aspd <= 0.125) document.getElementById("animspeed8s").checked = true;
    else if (aspd > 0.125 && aspd <= 0.25) 
      document.getElementById("animspeed4s").checked = true;
    else if (aspd > 0.25 && aspd <= 0.5) 
      document.getElementById("animspeed2s").checked = true;
    else if (aspd > 0.5 && aspd <= 1.0) 
      document.getElementById("animspeedN").checked = true;
    else if (aspd > 1.0 && aspd <= 2.0) 
      document.getElementById("animspeed2f").checked = true;
    else if (aspd > 2.0 && aspd <= 4.0) 
      document.getElementById("animspeed4f").checked = true;
    else if (aspd > 4.0) 
      document.getElementById("animspeed8f").checked = true;
  }
}

function makeSnapshotPath( object)
{
  var extension;
//  var graph, gslist, animated, dimension;
//  if (extension == null) {
//    graph = msiFindParentOfType( currObj()ect, "graph");
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
  if ( getOS(window) == "win") {
    extension = "bmp";
  }
  else {
    extension = "png";
  }
  try {
    var path = currObj().data;
    path = path.replace(/file:[-/[a-z_A-Z0-9.]+\/plots\//,'graphics/');
    path = path.replace(/xv[cz]$/,extension);  
  }
  catch(e) {
    dump(e.message);
  }
  return path;
}

function insertSnapshot( object, snapshotpath )
{
  var parent, i, element, ssobj, graph, gslist, objlist, w, h, units,
    oldpath, file, url;
  parent = currObj().parentNode;
  objlist = parent.getElementsByTagName("object");
  for (i = 0; i < objlist.length; ) {
    element = objlist[i];
    if (element.hasAttribute("msisnap")) {
      // remove the snapshot file as well as the currObj()ect node
      oldpath = element.data;
      if (oldpath && oldpath.length > 7) {
        oldpath = oldpath.slice(7);
      }
      file = Components.classes["@mozilla.org/file/local;1"].  
                           createInstance(Components.interfaces.nsILocalFile);  
      file.initWithPath(oldpath);
      if (file.exists())
      {
        try {
          file.remove(false);
        }
        catch (e) 
        {
          dump(e.message);
        }
      }
      parent.removeChild(element);
    } else {i++;}
  }
  graph = msiFindParentOfType( currObj(), "graph");
  gslist = graph.getElementsByTagName("graphSpec");
  if (gslist.length > 0) {
    gslist = gslist[0];
  }
  sscurrobj = currObj().cloneNode(true); // copies useful attributes
  sscurrobj.removeAttribute("msigraph");
  sscurrobj.setAttribute("data", snapshotpath);
  sscurrobj.setAttribute("msisnap", "true");
  sscurrobj.removeAttribute("type");
  parent.appendChild(sscurrObj());
  if (gslist) { // copy some attributes from the graphspec
    w = gslist.getAttribute("Width");
    if (w) {
      sscurrobj.setAttribute("naturalWidth",w);
      sscurrobj.setAttribute("imageWidth", w);
    }
    h = gslist.getAttribute("Height");
    if (h) {
      sscurrobj.setAttribute("naturalHeight",h);
      sscurrobj.setAttribute("imageHeight", h);
    }
    sscurrobj.setAttribute("units", gslist.getAttribute("Units"));
  }
}


function buildSnapshotFile(obj, abspath, res)
{
  var func = function(abspath, res) {currObj().makeSnapshot(abspath, res);}
  func(abspath, res);
}

function doMakeSnapshot(obj, browserElement) {
//  tryUntilSuccessful(200,10, function(){
//  var intervalId;
//  var count = 0;
//  intervalId = setInterval(function() {
//    if (count >= 10) {
//      clearInterval(intervalId);
//      return;
//    }
    if (currObj().makeSnapshot && currObj().readyState === 2) {
      try {
        var path = makeSnapshotPath(currObj());
        var abspath;
        var prefs;
        var res;
        var plotWrapper = currObj().parentNode;
        try {
          prefs = GetPrefs();
          res = prefs.getIntPref("swp.GraphicsSnapshotRes");
        }
        catch(e){
          res = 300;
        }
//rwa The following can have no effect in the Help documents - these will not have graphSpec elements.
//        if (graph == null) {
//          graph = new Graph();
//          DOMGraph = msiFindParentOfType( currObj(), "graph");
//          gslist = DOMGraph.getElementsByTagName("graphSpec");
//          if (gslist.length > 0) {
//            gslist = gslist[0];
//          }
//          graph.extractGraphAttributes(DOMGraph);
//        }
        abspath = makeSnapshotPathAbsolute(path, browserElement);
        plotWrapper.wrappedObj.makeSnapshot(abspath, res);
        insertSnapshot( currObj(), path );
      }
      catch(e) {
        dump(e.message);
      }
    }
//    count++;
//  }, 200);
}

function rebuildSnapshots (doc) {
  var wrapperlist, objlist, length, objlength, i, regexp, match, name1, name2;
//  var editorElement = msiGetActiveEditorElement();
  wrapperlist = wrapperlist = doc.documentElement.getElementsByTagName("plotwrapper");
  length = wrapperlist.length;
  var browserElement = document.GetElementById("help-content");
  for (i = 0; i < length; i++)
  {
    objlist = wrapperlist[i].getElementsByTagName("object");
    objlength = objlist.length;
    if (objlength === 1) {
      doMakeSnapshot(objlist[0], null, browserElement);
    }
    else if (objlength > 1){
      regexp = /plot\d+/;
      match = regexp.exec(objlist[0].data);
      name1 = match[0];
      match = regexp.exec(objlist[1].data);
      name2 = match[0];
      if (name1 !== name2) {
        doMakeSnapshot(objlist[0], null, browserElement);
      }
    }
  }
}



//function onVCamMouseDown(screenX, screenY)
//{
////  var editorElement = msiGetActiveEditorElement();
////  var editor = msiGetEditor(editorElement);
//////  var evt = editor.document.createEvent("MouseEvents");
//////  evt.initMouseEvent("mousedown", true, true, editor.document.defaultView,null,
//////    screenX, screenY, null, null, 0, 0, 0, 0, null, null);
////  editor.selection.collapse(this.parentNode,0);
////  editor.checkSelectionStateForAnonymousButtons(editor.selection);
//  doVCamInitialize(this);
//}

function onVCamDblClick(screenX, screenY)
{
}

function onVCamDragMove(x,y)
{
  return 1;
}

function onVCamDragLeave(x,y)
{
}

function doVCamPreInitialize(obj)
{
  var domGraph = getElementOrParentByTagName("graph", obj);
  var plotWrapper = getElementOrParentByTagName("plotwrapper", obj);
  var browserElement = document.getElementById("help-content");
  try {
    if (obj.addEvent && obj.readyState===2) {   
      obj.addEvent('leftMouseDown', onVCamMouseDown);
      obj.addEvent('dragMove', onVCamDragMove);
      obj.addEvent('dragLeave', (function() {}));
      // add a method for writing a snapshot
      var fn = function() {
        return doMakeSnapshot(obj, browserElement);
      }
      plotWrapper.wrappedObj = obj;
      doMakeSnapshot(obj, browserElement);
      return true;
    }
  }
  catch(e){
    dump(e.message);
  }
  return false;
}

function doVCamInitialize(obj)
{
//  document.getElementById("vcamactive").setAttribute("hidden","false");
  if (currObj() == obj) return;
  
  var threedplot = currObj().dimension === 3;
  var twodplot = currObj().dimension === 2;
  document.getElementById("3dplot").setAttribute("hidden", threedplot?"false":"true");
  document.getElementById("2dplot").setAttribute("hidden", twodplot?"false":"true");
  var animplot = currObj().isAnimated;
  document.getElementById("animplot").setAttribute("hidden", animplot?"false":"true");
//  if (animplot) // set up the progress bar
//  {
//    setAnimationTime = (function() {
//      return function() {
//        var time = currObj().beginTime + (gProgressbar.value/100)*(currObj().endTime-currObj().beginTime);
//        currObj().currentTime = time;
//      };
//    }()); 
//    try {
//      gProgressbar = document.getElementById("vc-AnimScale");
//      currObj().addEvent("currentTimeChange", showAnimationTime(currObj()));
//    }
//    catch (e)
//    {
//      dump("failure: " + e.toString() + "\n");
//    }
//    setAnimSpeed = (function() {
//      return function(factor) {
//        currObj().animationSpeed = factor;
//      }
//    }());
//    setLoopMode = (function() {
//      return function( mode ) {
//        currObj().animationLoopingMode = mode;
//      }
//   }());
//  }
  vcamToolbarFromPlugin();
}

function showAnimationTime(time)
{
  var obj = currObj();
  var newval = Math.round(100*(time/(obj.endTime - obj.beginTime)));
  gProgressbar.value = newval;
} 

function dontSetAnimationTime()
{
  return;
}


function vcSelObj () {
  currObj().cursorTool = "select";
  vcamToolbarFromPlugin();
};

function vcRotateLeft () {
  var obj = currObj();
  if (obj.rotateVerticalAction == 2) 
  {
    obj.rotateVerticalAction = 0;
  } 
  else 
  {
    obj.rotateVerticalAction = 2;
  }
  vcamToolbarFromPlugin();
};

function vcRotateRight () {
  var obj = currObj();
  if (obj.rotateVerticalAction == 1) 
  {
    obj.rotateVerticalAction = 0;
  } 
  else 
  {
    obj.rotateVerticalAction = 1;
  }
  vcamToolbarFromPlugin();
};   

function vcRotateUp () {
  var obj = currObj();
  if (obj.rotateHorizontalAction == 1) 
  {
    obj.rotateHorizontalAction = 0;
  } 
  else 
  {
    obj.rotateHorizontalAction = 1;
  }
  vcamToolbarFromPlugin();
};
  
function vcRotateDown () {
  var obj = currObj();
  if (obj.rotateHorizontalAction == 2) 
  {
    obj.rotateHorizontalAction = 0;
  } 
  else 
  {
    obj.rotateHorizontalAction = 2;
  }
  vcamToolbarFromPlugin();
};
  
function vcRotateScene () {
  currObj().cursorTool = "rotate";
  vcamToolbarFromPlugin();
};
  
function vcMove () {
  currObj().cursorTool = "move";    
  vcamToolbarFromPlugin();
};

function vcQuery () {
  currObj().cursorTool = "query";
  vcamToolbarFromPlugin();
};

function vcZoomBoxIn () {
  currObj().cursorTool = "select"; // This is zoomBoxIn in the 2d case
  vcamToolbarFromPlugin();
};

function vcZoomBoxOut () {
  currObj().cursorTool = "select"; 
  vcamToolbarFromPlugin();
};
  
function vcZoomIn () {
  currObj().cursorTool = "zoomIn";
  vcamToolbarFromPlugin();
};
  
function vcZoomOut () {
  currObj().cursorTool = "zoomOut";
  vcamToolbarFromPlugin();
};

function vcSnapshot () {
  doMakeSnapshot(currObj(), browserElement);
  vcamToolbarFromPlugin();
};

function vcAutoSpeed () {
  dump("cmd_vcAutoSpeed not implemented");
  vcamToolbarFromPlugin();
};
  
function vcAnimSpeed () {
  dump("cmd_vcAnimSpeed not implemented");
  vcamToolbarFromPlugin();
};
  
function vcAutoZoomIn () {
  if (currObj().zoomAction == 1) currObj().zoomAction = 0; else currObj().zoomAction = 1;
  vcamToolbarFromPlugin();
};

function vcAutoZoomOut () {
  if (currObj().zoomAction == 2) currObj().zoomAction = 0; else currObj().zoomAction = 2;
  vcamToolbarFromPlugin();
};

function vcGoToEnd () {
  currObj().currentTime = currObj().endTime;
  vcamToolbarFromPlugin();
};

function vcGoToStart () {
  currObj().currentTime = currObj().beginTime;
  vcamToolbarFromPlugin();
};

function vcLoopType () {
  dump("cmd_vcLoopType not implemented");
  vcamToolbarFromPlugin();
};

function vcPlay () {
  if (isRunning) currObj().stopAnimation();
  else currObj().startAnimation();
  isRunning = !isRunning;
  vcamToolbarFromPlugin();
};
  
function vcFitContents () {
  currObj().fitContents();
  vcamToolbarFromPlugin();
};

function setActionSpeed (factor) {
  currObj().actionSpeed = factor;
};

function vcReset () {
  currObj().reset();
};

#endif