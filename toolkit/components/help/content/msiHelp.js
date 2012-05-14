//Need:
//     getOS()
//     msidump()
//     graph and DOMGraph?
//     makeRelPathAbsolute and a path to start from (won't have editor!)
//  editor.checkSelectionStateForAnonymousButtons(editor.selection);?? don't really need?

Components.utils.import("resource://app/modules/os.jsm");

const xhtmlns  = "http://www.w3.org/1999/xhtml";
//const nsIWebProgressListener = Components.interfaces.nsIWebProgressListener;

var gPrefsService;
var gPrefsBranch;

var isRunning = false;
var currDocument = null;

function addClickEventListenerForBrowserElement(browserElement)
{
  try {
//    var bodyelement = msiGetBodyElement(editorElement);
    if (browserElement && browserElement.contentDocument)
    {
      if (currDocument != browserElement.contentDocument)
      {
        dump("Adding click event listener for document.\n");
        currDocument = browserElement.contentDocument;
        currDocument.addEventListener("click", msiHelpHandleClick, true);
      }
    }
  } catch (e) {dump("Unable to register browser element click event listener; error [" + e + "].\n");}
}

function msiGetBaseNodeName(node)
{
  return (node && ((node.localName)||(node.nodeName))) || null;
}

function getEventParentByTag( event, tagname)
{
	var node = event.target;
	while (node && msiGetBaseNodeName(node) !== tagname) node = node.parentNode;
	if (node && msiGetBaseNodeName(node) === tagname)
	  return node;
  return null;
}

function getChildrenByTagName(node, tagname)
{
  var nodeList = node.getElementsByTagName(tagname);
  if (!nodeList || !nodeList.length)
    nodeList = node.getElementsByTagNameNS(xhtmlns, tagname);
  return nodeList;
}

function GetPrefsService()
{
  if (gPrefsService)
    return gPrefsService;

  try {
    gPrefsService = Components.classes["@mozilla.org/preferences-service;1"].getService(Components.interfaces.nsIPrefService);
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

//This is called from the nsIWebProgressListener onStateChange() message handler
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
//  if (event.target.onclick) return;
  if (event.detail == 1)
  {
    var plotObj = null;
    var theURI, targWin;
    var objName = msiGetBaseNodeName(event.target);
	  var graphnode = getEventParentByTag(event, "plotwrapper");
    var linkNode;
    if (graphnode) 
    {
      plotObj = getChildrenByTagName(graphnode, "object")[0];
//      plotObj = graphnode.getElementsByTagName("object")[0];
    }
    else if (objName == "object")
    {
      if (event.target.getAttribute("msigraph") == "true")
        plotObj = event.target;
    }
//    if (!plotObj)
//     linkNode = getEventParentByTag(event, "xref");
//      if (!linkNode)
//	      linkNode = getEventParentByTag(event, "a");
//    }
    if (plotObj) 
    {
      doVCamInitialize(plotObj);
    }
//    else if (linkNode && (objName=="xref"))
//    {
//      theURI = event.target.getAttribute("href");
//      if (!theURI)
//        theURI = event.target.getAttribute("key");
//      if (theURI && theURI.length)
//        theURI = "#" + theURI;
//      msiClickLink(event, theURI, targWin, editorElement);
//    }
//    else if (linkNode)
//    {
//      theURI = event.target.getAttribute("href");
//      if (event.target.hasAttribute("target"))
//        targWin = event.target.getAttribute("target");
//      msiClickLink(event, theURI, targWin, editorElement);
//    }
    else
    {
		  if (document.getElementById("vcamactive") && document.getElementById("vcamactive").getAttribute("hidden")=="false") 
	    {
	      document.getElementById("vcamactive").setAttribute("hidden",true);
	    }
    }
  }

//  event.currentTarget should be "body" or something...

  // For Web Composer: In Show All Tags Mode,
  // single click selects entire element,
  //  except for body and table elements
//  if (IsWebComposer() && event.explicitOriginalTarget && msiIsHTMLEditor(editorElement) &&
//      msiGetEditorDisplayMode(editorElement) == kDisplayModeAllTags)
}

function msiHelpInit()
{
  var browserElement = document.getElementById("help-content");
  addClickEventListenerForBrowserElement(browserElement);

#ifndef PROD_SW
  var innerDoc = browserElement.contentDocument;
  var wrapperList = innerDoc.getElementsByTagName("object");
  for (var ii = 0; ii < wrapperList.length; ++ii)
  {
    if (wrapperList[ii].getAttribute("msigraph") == "true")
      doVCamPreInitialize(wrapperList[ii]);
  }
  var nsWrapperList = innerDoc.getElementsByTagNameNS(xhtmlns, "object");
  for (var jj = 0; jj < nsWrapperList.length; ++jj)
  {
    if (nsWrapperList[jj].getAttribute("msigraph") == "true")
      doVCamPreInitialize(nsWrapperList[jj]);
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
    if (msiGetBaseNodeName(nextNode) == tagName)
      retNode = nextNode;
    nextNode = nextNode.parentNode;
  }
  return retNode;
}

function vcamToolbarFromPlugin(obj)
{
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
    var aspd = obj.animationSpeed;
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
  if ( getOS(window) == "win") {
    extension = "bmp";
  }
  else {
    extension = "png";
  }
  try {
    var path = object.data;
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
  var parent, i, objectlist, element, ssobj, graph, gslist, w, h, units,
    oldpath, file, url;
  parent = object.parentNode;
  objectlist = getChildrenByTagName(parent, "object");
//  objectlist = parent.getElementsByTagName("object");
  for (i = 0; i < objectlist.length; ) {
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
  graph = msiFindParentOfType( object, "graph");
  gslist = getChildrenByTagName(graph, "graphSpec");
//  gslist = graph.getElementsByTagName("graphSpec");
  if (gslist.length > 0) {
    gslist = gslist[0];
  }
  ssobj = object.cloneNode(true); // copies useful attributes
  ssobj.removeAttribute("msigraph");
  ssobj.setAttribute("data", snapshotpath);
  ssobj.setAttribute("msisnap", "true");
  ssobj.removeAttribute("type");
  parent.appendChild(ssobj);
  if (gslist) { // copy some attributes from the graphspec
    w = gslist.getAttribute("Width");
    if (w) {
      ssobj.setAttribute("naturalWidth",w);
      ssobj.setAttribute("imageWidth", w);
    }
    h = gslist.getAttribute("Height");
    if (h) {
      ssobj.setAttribute("naturalHeight",h);
      ssobj.setAttribute("imageHeight", h);
    }
    ssobj.setAttribute("units", gslist.getAttribute("Units"));
  }
}


function buildSnapshotFile(obj, abspath, res)
{
  var func = function(abspath, res) {obj.makeSnapshot(abspath, res);}
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
        }
        catch(e){
          res = 300;
        }
//rwa The following can have no effect in the Help documents - these will not have graphSpec elements.
//        if (graph == null) {
//          graph = new Graph();
//          DOMGraph = msiFindParentOfType( obj, "graph");
//          gslist = DOMGraph.getElementsByTagName("graphSpec");
//          if (gslist.length > 0) {
//            gslist = gslist[0];
//          }
//          graph.extractGraphAttributes(DOMGraph);
//        }
        abspath = makeSnapshotPathAbsolute(path, browserElement);
        plotWrapper.wrappedObj.makeSnapshot(abspath, res);
        insertSnapshot( obj, path );
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
  wrapperlist = getChildrenByTagName(doc.documentElement, "plotwrapper");
//  wrapperlist = doc.documentElement.getElementsByTagName("plotwrapper");
  length = wrapperlist.length;
  var browserElement = document.GetElementById("help-content");
  for (i = 0; i < length; i++)
  {
    objlist = getChildrenByTagName(wrapperlist[i], "object");
//    objlist = wrapperlist[i].getElementsByTagName("object");
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

function doVCamCommandOnObject(obj, cmd)
{
  var browserElement = document.getElementById("help-content");
  netscape.security.PrivilegeManager.enablePrivilege('UniversalXPConnect');
  try {
    switch (cmd) {
      case "cmd_vcSelObj" :
        obj.cursorTool = "select";
        break;
      case "cmd_vcRotateLeft":
        if (obj.rotateVerticalAction == 2) {obj.rotateVerticalAction = 0;} else {obj.rotateVerticalAction = 2;}
        break;
      case "cmd_vcRotateRight":
        if (obj.rotateVerticalAction == 1) {obj.rotateVerticalAction = 0;} else {obj.rotateVerticalAction = 1;}
        break;
      case "cmd_vcRotateUp":
        if (obj.rotateHorizontalAction == 1) {obj.rotateHorizontalAction = 0;} else {obj.rotateHorizontalAction = 1;}
        break;
      case "cmd_vcRotateDown":
        if (obj.rotateHorizontalAction == 2) {obj.rotateHorizontalAction = 0;} else {obj.rotateHorizontalAction = 2;}
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
        doMakeSnapshot(obj, browserElement);
        break;
      case "cmd_vcAutoSpeed":
        dump("cmd_vcAutoSpeed not implemented");
        break;
      case "cmd_vcAnimSpeed":
        dump("cmd_vcAnimSpeed not implemented");
        break;
      case "cmd_vcAutoZoomIn":
        if (obj.zoomAction == 1) obj.zoomAction = 0; else obj.zoomAction = 1;
        break;
      case "cmd_vcAutoZoomOut":
        if (obj.zoomAction == 2) obj.zoomAction = 0; else obj.zoomAction = 2;
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
  }
  catch (e) {
    dump(e.message);
  }
  vcamToolbarFromPlugin(obj);
  return;
}

var gProgressbar;

function onVCamMouseDown(screenX, screenY)
{
//  var editorElement = msiGetActiveEditorElement();
//  var editor = msiGetEditor(editorElement);
////  var evt = editor.document.createEvent("MouseEvents");
////  evt.initMouseEvent("mousedown", true, true, editor.document.defaultView,null,
////    screenX, screenY, null, null, 0, 0, 0, 0, null, null);
//  editor.selection.collapse(this.parentNode,0);
//  editor.checkSelectionStateForAnonymousButtons(editor.selection);
  doVCamInitialize(this);
}

function onVCamDblClick(screenX, screenY)
{
//  var editorElement = msiGetActiveEditorElement();
////  var editor = msiGetEditor(editorElement);
////  var evt = editor.document.createEvent("MouseEvents");
////  evt.initMouseEvent("dblclick", true, true, editor.document.defaultView,null,
////    screenX, screenY, null, null, 0, 0, 0, 0, null, null);
//  goDoPrinceCommand("cmd_objectProperties", this, editorElement);
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
  tryUntilSuccessful(200,10, function(){
    var domGraph = getElementOrParentByTagName("graph", obj);
    var plotWrapper = getElementOrParentByTagName("plotwrapper", obj);
    var browserElement = document.getElementById("help-content");
    try {
      if (obj.addEvent && obj.readyState===2) {   
        obj.addEvent('leftMouseDown', onVCamMouseDown);
        obj.addEvent('leftMouseUp', onVCamMouseUp);
        obj.addEvent('leftMouseDoubleClick', onVCamDblClick);
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
  });
}


function onVCamMouseUp()
{
//  alert("Mouse up in plugin!");
}

var setAnimationTime;
var VCamCommand;
var setActionSpeed;
var setAnimSpeed;
var setLoopMode;


function doVCamCommand(cmd)
{
	VCamCommand(cmd);
}

function doVCamInitialize(obj)
{
  dump("doVCamInitialize");
  document.getElementById("vcamactive").setAttribute("hidden","false");
//  var editorElement = msiGetActiveEditorElement();
//  var editor = msiGetEditor(editorElement);
  var os = getOS(window);

  VCamCommand = (function () {
    var thisobj = obj;
    return function(_cmd) {
      return doVCamCommandOnObject(thisobj, _cmd);
    };
  }()); 
  setActionSpeed = (function() {
    var thisobj = obj;
    return function(factor) {
      thisobj.actionSpeed = factor;
    };
  }());
  var threedplot = obj.dimension === 3;
  var twodplot = obj.dimension === 2;
  document.getElementById("3dplot").setAttribute("hidden", threedplot?"false":"true");
  document.getElementById("2dplot").setAttribute("hidden", twodplot?"false":"true");
  var animplot = obj.isAnimated;
  document.getElementById("animplot").setAttribute("hidden", animplot?"false":"true");
  if (animplot) // set up the progress bar
  {
    setAnimationTime = (function() {
      var thisobj = obj;
      return function() {
        var time = thisobj.beginTime + (gProgressbar.value/100)*(thisobj.endTime-thisobj.beginTime);
        obj.currentTime = time;
      };
    }()); 
    try {
      gProgressbar = document.getElementById("vc-AnimScale");
      obj.addEvent("currentTimeChange", showAnimationTime(obj));
    }
    catch (e)
    {
      dump("failure: " + e.toString() + "\n");
    }
    setAnimSpeed = (function() {
      var thisobj = obj;
      return function(factor) {
        thisobj.animationSpeed = factor;
      }
    }());
    setLoopMode = (function() {
      var thisobj = obj;
      return function( mode ) {
        thisobj.animationLoopingMode = mode;
      }
   }());
  }
  vcamToolbarFromPlugin(obj);
}

function showAnimationTime(obj)
{
  var thisobj = obj;
  return function(time) {
    var newval = Math.round(100*(time/(thisobj.endTime - thisobj.beginTime)));
    gProgressbar.value = newval;
  };
} 

function dontSetAnimationTime()
{
  return;
}

#endif