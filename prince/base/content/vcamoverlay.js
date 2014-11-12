/*****************************************************************************

VCam object section.

The following functions will be bound to VCam plot objects, so 'this' in the
code refers to the plot object. The attachment of the functions to the objects
happen when the object is instantiated, i.e. clicked on, in doVCamInitialize.

******************************************************************************/

/* This code is called both when editing a document with plots, and when displaying
help files that contain plots. The difference is that in the latter case therer is no
editor element, and msiGetActiveEditorElement is not defined. Hence there are
a number of try blocks surrounding calls to msiGetActiveEditorElement. */

function vcamToolbarFromPlugin() {
  // sets the state of the toolbar buttoms from the plugin state
  // Call the function for initialization and after each button action.
  // Called so that *this* is the VCam object


  // Cursor tool section
  var ct = this.cursorTool;
  var dim = this.dimension;
  var anim = this.isAnimated;
  var za = this.zoomAction;
  document.getElementById("vc-Reset").removeAttribute("disabled");
  document.getElementById("vc-FitContents").removeAttribute("disabled");
  document.getElementById("vc-SnapShot").removeAttribute("disabled");
  try {
    msiGetActiveEditorElement();
  }
  catch (e) {
    document.getElementById("vc-SnapShot").setAttribute("hidden");
  }
  document.getElementById("vc-SelObj").checked = (ct === 'select');
  document.getElementById("vc-SelObj").removeAttribute("disabled");
  document.getElementById("vc-RotateScene").checked = (ct === 'rotate');
  document.getElementById("vc-RotateScene").removeAttribute("disabled");
  document.getElementById("vc-Move").checked = (ct === 'move');
  document.getElementById("vc-Move").removeAttribute("disabled");
  document.getElementById("vc-Query").checked = (ct === 'query');
  document.getElementById("vc-Query").removeAttribute("disabled");
  document.getElementById("vc-AutoZoomIn").checked = (za === 1);
  document.getElementById("vc-AutoZoomIn").removeAttribute("disabled");
  document.getElementById("vc-AutoZoomOut").checked = (za === 2);
  document.getElementById("vc-AutoZoomOut").removeAttribute("disabled");

  // Rotation section
  if (dim === 3) {
    document.getElementById("vc-Zoom").checked = (ct === 'zoom');
    document.getElementById("vc-Zoom").removeAttribute("disabled");
    var va = this.rotateVerticalAction;
    document.getElementById("vc-RotateRight").checked = (va === 1);
    document.getElementById("vc-RotateRight").removeAttribute("disabled");
    document.getElementById("vc-RotateLeft").checked = (va === 2);
    document.getElementById("vc-RotateLeft").removeAttribute("disabled");
    var ha = this.rotateHorizontalAction;
    document.getElementById("vc-RotateUp").checked = (ha === 1);
    document.getElementById("vc-RotateUp").removeAttribute("disabled");
    document.getElementById("vc-RotateDown").checked = (ha === 2);
    document.getElementById("vc-RotateDown").removeAttribute("disabled");

    // Speed control
    var spd = this.actionSpeed;
    if (spd <= 0.125) {
      document.getElementById("actionspeed8s").checked = true;
    }
    else if (spd > 0.125 && spd <= 0.25) {
      document.getElementById("actionspeed4s").checked = true;
    }
    else if (spd > 0.25 && spd <= 0.5) {
      document.getElementById("actionspeed2s").checked = true;
    }
    else if (spd > 0.5 && spd <= 1.0) {
      document.getElementById("actionspeedN").checked = true;
    }
    else if (spd > 1.0 && spd <= 2.0) {
      document.getElementById("actionspeed2f").checked = true;
    }
    else if (spd > 2.0 && spd <= 4.0) {
      document.getElementById("actionspeed4f").checked = true;
    }
    else if (spd > 4.0) {
      document.getElementById("actionspeed8f").checked = true;
    }
    document.getElementById("actionspeed8s").removeAttribute("disabled");
    document.getElementById("actionspeed4s").removeAttribute("disabled");
    document.getElementById("actionspeed2s").removeAttribute("disabled");
    document.getElementById("actionspeedN").removeAttribute("disabled");
    document.getElementById("actionspeed2f").removeAttribute("disabled");
    document.getElementById("actionspeed4f").removeAttribute("disabled");
    document.getElementById("actionspeed8f").removeAttribute("disabled");

  }
  if (anim) {
    // animation section, including animation speed control
    var aspd = this.animationSpeed;
    if (aspd <= 0.125) document.getElementById("animspeed8s").checked = true;
    else if (aspd > 0.125 && aspd <= 0.25) document.getElementById("animspeed4s").checked = true;
    else if (aspd > 0.25 && aspd <= 0.5) document.getElementById("animspeed2s").checked = true;
    else if (aspd > 0.5 && aspd <= 1.0) document.getElementById("animspeedN").checked = true;
    else if (aspd > 1.0 && aspd <= 2.0) document.getElementById("animspeed2f").checked = true;
    else if (aspd > 2.0 && aspd <= 4.0) document.getElementById("animspeed4f").checked = true;
    else if (aspd > 4.0) document.getElementById("animspeed8f").checked = true;
    document.getElementById("animspeed8s").removeAttribute("disabled");
    document.getElementById("animspeed4s").removeAttribute("disabled");
    document.getElementById("animspeed2s").removeAttribute("disabled");
    document.getElementById("animspeedN").removeAttribute("disabled");
    document.getElementById("animspeed2f").removeAttribute("disabled");
    document.getElementById("animspeed4f").removeAttribute("disabled");
    document.getElementById("animspeed8f").removeAttribute("disabled");
  }
}

function makeSnapshotPath() {
  var extension;
  if ( getOS(window) == "win")  {
    extension = "bmp";
  } else {
    extension = "png";
  }
  var path;
  try {
    var strSrc = this.getAttribute("data") || this.getAttribute("src");
    var match = /plots\/(.*$)/.exec(strSrc);
    var fileName = match[1];
    fileName = fileName.replace(/xv[cz]$/, extension);
    path = "graphics/" + fileName;
  } catch (e) {
    throw new MsiException("Error finding path for plot snapshot", e);
  }
  return path;
}

function insertSnapshot(abssnapshotpath) {
  var parent, i, objectlist, element, ssobj, graph, gslist, w, h, units, oldpath, file, url;
  parent = this.parentNode;
  objectlist = parent.getElementsByTagName("object");
  var snapshotUrl = msiFileURLFromAbsolutePath(abssnapshotpath);
  var snapshotRelUrl;
  if (snapshotUrl) {
    snapshotRelUrl= msiMakeRelativeUrl(snapshotUrl.spec);
  } else {
    snapshotRelUrl = abssnapshotpath; // we were actually passed a relative url to begin with
  }
  for (i = 0; i < objectlist.length;) {
    element = objectlist[i];
    if (element.hasAttribute("msisnap")) {
      parent.removeChild(element);
    } else {
      i++;
    }
  }
  graph = msiFindParentOfType(this, "graph");
  gslist = graph.getElementsByTagName("graphSpec");
  if (gslist.length > 0) {
    gslist = gslist[0];
  }
  ssobj = this.cloneNode(true); // copies useful attributes
  ssobj.id = "ss" + ssobj.id;
  ssobj.removeAttribute("msigraph");
  ssobj.setAttribute("data", snapshotRelUrl);
  ssobj.setAttribute("msisnap", "true");
  ssobj.removeAttribute("type");
  parent.appendChild(ssobj);
  if (gslist) { // copy some attributes from the graphspec
    w = gslist.getAttribute("Width");
    if (w) {
      ssobj.setAttribute("naturalWidth", w);
      ssobj.setAttribute("ltx_width", w);
    }
    h = gslist.getAttribute("Height");
    if (h) {
      ssobj.setAttribute("naturalHeight", h);
      ssobj.setAttribute("ltx_height", h);
    }
    ssobj.setAttribute("units", gslist.getAttribute("Units"));
  }
}

function doMakeSnapshot()
{ 
  var editorElement;
  try {
    editorElement = msiGetActiveEditorElement();
  }
  catch(e) {
    return;
  }
  var doc = editorElement.contentDocument;
  var ready = this.readyState;
  if (ready > 1) {  
    try {
      var path = makeSnapshotPath.call(this);
      var abspath;
      var abspath2;
      var prefs;
      var res;
      var DOMGraph;
      var gslist;
      var graph = null;
      var plotWrapper = this.parentNode;
      try {
        prefs = GetPrefs();
        res = prefs.getIntPref("swp.GraphicsSnapshotRes");
      } catch (e) {
        res = 300;
      }
      if (graph == null) {
        graph = new Graph();
        DOMGraph = msiFindParentOfType(this, "graph");
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
      var oldsnapshot;
      snapshotDir.initWithPath(abspath);
      oldsnapshot = snapshotDir.clone();
      if (oldsnapshot.exists()) oldsnapshot.remove(true);
      snapshotDir = snapshotDir.parent;
      if (!snapshotDir.exists()) snapshotDir.create(1, 0755);
      this.makeSnapshot(abspath, res);
      if ( getOS(window) == "win")  {
        graphicsConverter.init(window, snapshotDir.parent);
        // oldsnapshot is an nsIFile pointing to the .bmp file
        abspath = graphicsConverter.copyAndConvert(oldsnapshot, false);
      }

      insertSnapshot.call(this, abspath);
    } catch (e) {
      throw new MsiException("Error inserting plot snapshot", e);
    }
  }
}

function rebuildSnapshots(doc) {
  netscape.security.PrivilegeManager.enablePrivilege('UniversalXPConnect');  // BBM: test to see if this is necessary

  var wrapperlist, objlist, length, objlength, i, obj, snapshot;
  var img = null;
  var obj = null;
  var doRefresh = false;
  wrapperlist = doc.documentElement.getElementsByTagName("plotwrapper");
  length = wrapperlist.length;
  for (i = 0; i < length; i++) {
    objlist = wrapperlist[i].getElementsByTagName("object");
    if (objlist.length > 0) {
      obj = objlist[0];
      if (obj.wrappedJSObject) obj = obj.wrappedJSObject;
      if (objlist.length > 1) img = objlist[1];
      if (!img) {
        return doMakeSnapshot.call(obj);
      }
      if (needRefresh(obj.getAttribute('data'), img.getAttribute('data'))) {
        return doMakeSnapshot.call(obj);
      }
    }
  }
}

function doVCamCommandOnObject(cmd, editorElement) {
//  if (!editorElement && msiGetActiveEditorElement != null) editorElement = msiGetActiveEditorElement();
  var doc = null;
  if (editorElement) doc = editorElement.contentDocument;
  // netscape.security.PrivilegeManager.enablePrivilege('UniversalXPConnect');  // BBM: test to see if this is necessary
  try {
    switch (cmd) {
    case "cmd_vcSelObj":
      this.cursorTool = "select";
      break;
    case "cmd_vcRotateLeft":
      if (this.rotateVerticalAction == 2) {
        this.rotateVerticalAction = 0;
      } else {
        this.rotateVerticalAction = 2;
      }
      break;
    case "cmd_vcRotateRight":
      if (this.rotateVerticalAction == 1) {
        this.rotateVerticalAction = 0;
      } else {
        this.rotateVerticalAction = 1;
      }
      break;
    case "cmd_vcRotateUp":
      if (this.rotateHorizontalAction == 1) {
        this.rotateHorizontalAction = 0;
      } else {
        this.rotateHorizontalAction = 1;
      }
      break;
    case "cmd_vcRotateDown":
      if (this.rotateHorizontalAction == 2) {
        this.rotateHorizontalAction = 0;
      } else {
        this.rotateHorizontalAction = 2;
      }
      break;
    case "cmd_vcRotateScene":
      this.cursorTool = "rotate";
      break;
    case "cmd_vcZoom":
      this.cursorTool = "zoom";
      break;
    case "cmd_vcMove":
      this.cursorTool = "move";
      break;
    case "cmd_vcQuery":
      this.cursorTool = "query";
      break;
    case "cmd_vcZoomBoxIn":
      this.cursorTool = "select"; // This is zoomBoxIn in the 2d case
      break;
    case "cmd_vcZoomBoxOut":
      this.cursorTool = "select";
      break;
    case "cmd_vcZoomIn":
      this.cursorTool = "zoomIn";
      break;
    case "cmd_vcZoomOut":
      this.cursorTool = "zoomOut";
      break;
    case "cmd_vcSnapshot":
      this.vcMakeSnapshot();
      break;
    case "cmd_vcAutoSpeed":
      dump("cmd_vcAutoSpeed not implemented");
      break;
    case "cmd_vcAnimSpeed":
      dump("cmd_vcAnimSpeed not implemented");
      break;
    case "cmd_vcAutoZoomIn":
      if (this.zoomAction == 1) this.zoomAction = 0;
      else this.zoomAction = 1;
      break;
    case "cmd_vcAutoZoomOut":
      if (this.zoomAction == 2) this.zoomAction = 0;
      else this.zoomAction = 2;
      break;
    case "cmd_vcGoToEnd":
      this.currentTime = this.endTime;
      break;
    case "cmd_vcGoToStart":
      this.currentTime = this.beginTime;
      break;
    case "cmd_vcLoopType":
      dump("cmd_vcLoopType not implemented");
      break;
    case "cmd_vcPlay":
      if (isRunning) this.stopAnimation();
      else this.startAnimation();
      isRunning = !isRunning;
      break;
    case "cmd_vcFitContents":
      this.fitContents();
      break;
//    case "cmd_refresh":  // shouldn't be needed, but this causes the display to refresh
//      this.camera.focalPointX = this.camera.focalPointX;
//      break;

    default:
    }
  } catch (e) {
    throw new MsiException("Unable to execute VCam command "+cmd, e);2  
  }
  vcamToolbarFromPlugin.call(this);
  return;
}

var gProgressbar;

/*
VCam event handler section

*/

function onVCamMouseUp() {
  //  alert("Mouse up in plugin!");
}

function onVCamMouseDown(screenX, screenY) {
  try {
    if (msiGetActiveEditorElement != null) {
      var editorElement = msiGetActiveEditorElement();
      var editor = msiGetEditor(editorElement);
      editor.selection.collapse(this.parentNode, 0);
      editor.checkSelectionStateForAnonymousButtons(editor.selection);
    }
  }
  catch(e) {}
  this.doVCamInitialize();
}

function onVCamDblClick(screenX, screenY) {
  try {
    if (msiGetActiveEditorElement != null) {
      var editorElement = msiGetActiveEditorElement();
      goDoPrinceCommand("cmd_objectProperties", this, editorElement);
    }
  }
  catch(e) {}
}

function onVCamRightMouseDown(screenX, screenY)
{
  try {
    var editorElement = msiGetActiveEditorElement();
  }
  catch(e) { return; }
  var editor = msiGetEditor(editorElement);
  var graphNode = editor.getElementOrParentByTagName("graph", this);
  var contextMenu = document.getElementById("msiEditorContentContext");
  if (contextMenu)
  {
    contextMenu.showPopup(graphNode, screenX, screenY, "none", "none");
	  contextMenu.focus();
  }
}

function onVCamRightMouseUp(screenX, screenY)  // BBM: ???
{
  try {
    if (msiGetActiveEditorElement != null)  {
      var editorElement = msiGetActiveEditorElement();
      var editor = msiGetEditor(editorElement);
      var evt = editor.document.createEvent("MouseEvents");
      evt.initMouseEvent("mouseup", true, true, editor.document.defaultView, 1,
        screenX, screenY, null, null, 0, 0, 0, 0, 2, null);
      this.parentNode.dispatchEvent(evt);
    }
  }
  catch(e) {}
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
  } 
  catch(exc)
  {
    msidump("Got exception in onVCamTreeChange: " + exc + "\n");
  }
}

function onVCamDragLeave(x, y) {}

function queryVCamValues(obj, graph, domGraph, bUserSetIfChanged)
{
  var cameraVals = null;
  var coordSysVals = {XAxisTitle : "x", YAxisTitle : "y",
                      ViewingBoxXMin : "-5", ViewingBoxXMax : "5",
                      ViewingBoxYMin : "-5", ViewingBoxYMax : "5"};
  var camera, vcamDoc, kidNode, coordSysNode, sceneNode
  var dim = graph.Dimension;
  var sceneNodeName = "Scene" + dim + "d";
  var docElement;
  var coordSysNodeName = "CoordinateSystem" + dim + "d";
  var aProp;

  if (obj.wrappedJSObject) {
    obj = obj.wrappedJSObject;
  }
  
  if (dim == 3)
  {
    cameraVals = {positionX : "0", positionY : "0", positionZ : "0",
                    focalPointX : "0", focalPointY : "0", focalPointZ : "0",
                    upVectorX : "0", upVectorY : "0", upVectorZ : "1",
                    keepUpVector : "false", viewingAngle : "2.0944", orthogonalProjection : "false"};
    coordSysVals.ViewingBoxZMin = "-5";
    coordSysVals.ViewingBoxZMax = "5";
    coordSysVals.ZAxisTitle = "z";
    if (graph.camera)
    {
      for (aProp in cameraVals)
      {
        cameraVals[aProp] = String(graph.camera[aProp]);
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
  if (graph.isAnimated() )
  {
    animVals = {beginTime : 0, endTime : 10, currentTime : 0};
    for (aProp in animVals)
      animVals[aProp] = String(graph[aProp]);
    animVals.framesPerSecond = 5;
    if (graph)
      graph.setAnimationValsFromVCam(animVals, domGraph, bUserSetIfChanged);
  }
  vcamDoc = obj.document;
  if (vcamDoc)
  {
    try {
      if (vcamDoc.wrappedJSObject) {
        vcamDoc = vcamDoc.wrappedJSObject;
      }
      docElement = vcamDoc.documentElement;
      if (docElement.wrappedJSObject) {
        docElement = docElement.wrappedJSObject;
      }
      sceneNode = getChildByTagName(docElement,sceneNodeName)
      coordSysNode = getChildByTagName(sceneNode,coordSysNodeName);
      if (coordSysNode)
      {
        kidNode = coordSysNode.firstChild;
        while (kidNode)
        {
          if (kidNode.nodeName in coordSysVals)
            coordSysVals[kidNode.nodeName] = String(kidNode.firstChild.nodeValue);
          kidNode = kidNode.nextSibling;
        }
      }
    }
    catch(e) {
      msidump(e.message);
    }
  }
  if (!coordSysNode)
    coordSysVals = null;
  if (graph)
    graph.setCoordSysValsFromVCam(coordSysVals, domGraph, bUserSetIfChanged);
    
}


function doVCamClose()
{
  if (document.getElementById("vcamactive") && (document.getElementById("vcamactive").getAttribute("hidden")=="false"))
    return;  //In other words, if we're being called because of the broadcaster turning VCam on, do nothing
  // var editorElement = msiGetActiveEditorElement();
//  return vcamCloseFunction(editorElement);   //BBM: fix this up
}

var VCamObject = null;


function doVCamInitialize(obj) {
  document.getElementById("vcamactive").setAttribute("hidden", "false");
  // The above line reveals the VCam toolbar,
  var editorElement = null;
  var editor = null;
  try {
    if (msiGetActiveEditorElement != null) {
      editorElement = msiGetActiveEditorElement();
      editor = msiGetEditor(editorElement); 
    }
  }
  catch(e) {}
  var os = getOS(window);

  // Now tailor the toolbar to the plot type.
  if (obj.wrappedJSObject) obj = obj.wrappedJSObject;
  VCamObject = obj;
  obj.threedplot = (obj.dimension === 3);
  obj.twodplot = (obj.dimension === 2);
  document.getElementById("3dplot").setAttribute("hidden", obj.threedplot ? "false" : "true");
  document.getElementById("2dplot").setAttribute("hidden", obj.twodplot ? "false" : "true");
  obj.animplot = obj.isAnimated;
  document.getElementById("animplot").setAttribute("hidden", obj.animplot ? "false" : "true");
  if (obj.animplot) // set up the progress bar
  {
    obj.setAnimationTime = function setAnimationTime() {
      var time = this.beginTime + (document.getElementById("vc-AnimScale").value / 100) * (this.endTime - this.beginTime);
      this.currentTime = time;
    };
    obj.showAnimationTime = function showAnimationTime(time) {
      var newval = Math.round(100 * (time / (this.endTime - this.beginTime)));
      document.getElementById("vc-AnimScale").value = newval;
    };
    obj.addEvent("currentTimeChange", obj.showAnimationTime.call(VCamObject));
    obj.setAnimSpeed = function setAnimSpeed(factor) {
      this.animationSpeed = factor;
    };
    obj.setLoopMode = function(mode) {
      this.animationLoopingMode = mode;
    };
  }
  vcamToolbarFromPlugin.call(obj);
  obj.command = function command(cmd ) { 
      return doVCamCommandOnObject.call( this, cmd, editorElement);
  };
  obj.setActionSpeed = function setActionSpeed(factor) {
    this.actionSpeed = factor;
  };

  obj.onVCamMouseUp = function onVCamMouseUp() {
    //  alert("Mouse up in plugin!");
  };

  obj.onVCamMouseDown = function onVCamMouseDown(screenX, screenY) {
    try 
    {
      var editorElement = msiGetActiveEditorElement();
      var editor = msiGetEditor(editorElement);
      editor.selection.collapse(this.parentNode, 0);
      editor.checkSelectionStateForAnonymousButtons(editor.selection);
    }
    catch(e) {return;}
  };

  obj.onVCamDblClick = function onVCamDblClick(screenX, screenY) {
    goDoPrinceCommand("cmd_objectProperties", this, editorElement);
  };

  obj.onVCamDragMove = function onVCamDragMove(x, y) {
    return 1;
  };

  obj.vcMakeSnapshot = function vcMakeSnapshot () { 
    doMakeSnapshot.call(obj);
  };

  VCamObject = obj;

  obj.addEvent('leftMouseDown', obj.onVCamMouseDown.bind(obj, screenX, screenY));
  obj.addEvent('leftMouseUp', obj.onVCamMouseUp.bind(obj));
  obj.addEvent('leftMouseDoubleClick', obj.onVCamDblClick.bind(obj, screenX, screenY));
  obj.addEvent('dragMove', obj.onVCamDragMove.bind(obj));
  obj.addEvent('dragLeave', (function() {}));

  if (editorElement) editorElement.focus();
}

function VCamCommand(cmd) {
  doVCamCommandOnObject.call(VCamObject, cmd, null);
}

