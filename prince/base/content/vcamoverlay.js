/*****************************************************************************

VCam object section.

The following functions will be bound to VCam plot objects, so 'this' in the
code refers to the plot object. The attachment of the functions to the objects
happen when the object is instantiated, i.e. clicked on, in doVCamInitialize.

******************************************************************************/

/* This code is called both when editing a document with plots, and when displaying
help files that contain plots. The difference is that in the latter case there is no
editor element, and msiGetActiveEditorElement is not defined. Hence there are
a number of try blocks surrounding calls to msiGetActiveEditorElement. */


// VCamObject definition

// Constructor and prototype

var currentVCamObject = null;

function VCamObject( vcampluginObject) {
  netscape.security.PrivilegeManager.enablePrivilege('UniversalXPConnect');  // BBM: test to see if this is necessary
  if (vcampluginObject.wrappedJSObject) {
    this.obj = vcampluginObject.wrappedJSObject;
  }
  else {
    this.obj = vcampluginObject;
  }

}

VCamObject.prototype = {

  // plot info
  threedplot: false,
  twodplot: false,
  animplot: false,

  // plot control
  setAnimationTime: null,
  showAnimationTime: null,
  setAnimSpeed: null,
  setLoopMode: null,
  setActionSpeed: null,

  // plot event handlers
  onVCamLeftMouseDown: null,
  onVCamRightMouseDown: null,
  onVCamLeftMouseUp: null,
  onVCamRightMouseUp: null,
  onVCamLeftDblClick: null,
  onVCamDragMove: null,
  onVCamDrop: null,

  init: function() {
    netscape.security.PrivilegeManager.enablePrivilege('UniversalXPConnect');  // BBM: test to see if this is necessary
    this.initToolbar();
    this.initEventHandlers();
    },

  initToolbar: function() {
    // sets the state of the toolbar buttons from the plugin state
    // Call the function for initialization and after each button action.

    // Cursor tool section
    var ct = this.obj.cursorTool,
      dim = this.obj.dimension,
      anim = this.obj.isAnimated,
      za = this.obj.zoomAction;

    document.getElementById("vc-Reset").removeAttribute("disabled");
    document.getElementById("vc-FitContents").removeAttribute("disabled");
    document.getElementById("vc-SnapShot").removeAttribute("disabled");
    try {
      msiGetActiveEditorElement();
    }
    catch (e) {
      document.getElementById("vc-SnapShot").setAttribute("hidden");
      // this disables snapshots if we are not in an editor; e.g., in a help file.
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
      var va = this.obj.rotateVerticalAction;
      document.getElementById("vc-RotateRight").checked = (va === 1);
      document.getElementById("vc-RotateRight").removeAttribute("disabled");
      document.getElementById("vc-RotateLeft").checked = (va === 2);
      document.getElementById("vc-RotateLeft").removeAttribute("disabled");
      var ha = this.obj.rotateHorizontalAction;
      document.getElementById("vc-RotateUp").checked = (ha === 1);
      document.getElementById("vc-RotateUp").removeAttribute("disabled");
      document.getElementById("vc-RotateDown").checked = (ha === 2);
      document.getElementById("vc-RotateDown").removeAttribute("disabled");

      // Speed control
      var spd = this.obj.actionSpeed;
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
      var aspd = this.obj.animationSpeed;
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
    document.getElementById("vcamactive").setAttribute("hidden", "false");
    // The above line reveals the VCam toolbar,
  },

  initEventHandlers: function() {
    // there might not be an editor element; e.g., which displaying help files
    var editorElement = null;
    var editor = null;
    try {
      if (msiGetActiveEditorElement != null) {
        editorElement = msiGetActiveEditorElement();
        if (editorElement) {
          editor = msiGetEditor(editorElement);
        }
      }
    }
    catch(e) {} // it's ok to fail

    // Currently we keep a global pointer to the currently active object.
    currentVCamObject = this;

    this.threedplot = (this.obj.dimension === 3);
    this.twodplot = (this.obj.dimension === 2);
    if (this.threedplot) {
      document.getElementById("3dplot").removeAttribute("hidden");
    }
    else {
      document.getElementById("3dplot").setAttribute("hidden", "true");
    }
    if (this.twodplot) {
      document.getElementById("2dplot").removeAttribute("hidden");
    }
    else {
      document.getElementById("2dplot").setAttribute("hidden", "true");
    }
    this.animplot = this.obj.isAnimated;

    if (this.animplot) // set up the progress bar
    {
      document.getElementById("animplot").removeAttribute("hidden");
      this.setAnimationTime = function() {
        var time = this.obj.beginTime + (document.getElementById("vc-AnimScale").value / 100) *
          (this.obj.endTime - this.obj.beginTime);
        this.obj.currentTime = time;
        this.obj.startAnimation();
      };

      this.showAnimationTime = function(time) {
        var newval = Math.round(100 * (time / (this.obj.endTime - this.obj.beginTime)));
        document.getElementById("vc-AnimScale").value = newval;
      };

      this.setAnimSpeed = function(factor) {
        this.obj.animationSpeed = factor;
      };

      this.setLoopMode = function(mode) {
        this.obj.animationLoopingMode = mode;
      };
    }
    else {
      document.getElementById("animplot").setAttribute("hidden", "true");
    }
    this.setActionSpeed = function(factor) {
      this.obj.actionSpeed = factor;
    };

    // define mouse handlers

    this.onVCamLeftMouseUp = function onVCamLeftMouseUp() {
      // alert("left mouse up in plugin!");
    }

    this.onVCamLeftMouseDown = function onVCamLeftMouseDown(screenX, screenY) {
      // alert('left mouse');
      // try {
      //   if (msiGetActiveEditorElement != null) {
      //     var editorElement = msiGetActiveEditorElement();
      //     var editor = msiGetEditor(editorElement);
      //     editor.selection.collapse(this.obj.parentNode, 0);
      //     editor.checkSelectionStateForAnonymousButtons(editor.selection);
      //   }
      // }
      // catch(e) {}
//      this.doVCamInitialize();
    }



    this.onVCamRightMouseUp = function onVCamRightMouseUp() {
       // alert("right mouse up in plugin!");
    }


    this.onVCamLeftDblClick = function onVCamLeftDblClick(screenX, screenY) {
      // alert('double click');
      try {
        if (msiGetActiveEditorElement != null) {
          var editorElement = msiGetActiveEditorElement();
          goDoPrinceCommand("cmd_objectProperties", this.obj.parentNode.parentNode, editorElement); // go up to the graph object
        }
      }
      catch(e) {}
    }

    this.onVCamRightMouseDown = function onVCamRightMouseDown(screenX, screenY)
    {
      // alert('right mouse');
      try {
        var editorElement = msiGetActiveEditorElement();
      }
      catch(e) { return; }
      var editor = msiGetEditor(editorElement);
      var graphNode = editor.getElementOrParentByTagName("graph", this.obj);
      var contextMenu = document.getElementById("msiEditorContentContext");
      if (contextMenu)
      {
        contextMenu.showPopup(graphNode, screenX, screenY, "none", "none");
        contextMenu.focus();
      }
    }

    this.onVCamRightMouseUp = function onVCamRightMouseUp(screenX, screenY)  // BBM: ???
    {
      try {
        // alert('right mouse up');
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

    this.onVCamKeyDown = function onVCamKeyDown(aKeyCodeMaybe)
    {
      dump("In onVCamKeyDown, keyCode was " + (aKeyCodeMaybe ? aKeyCodeMaybe : "null") + "\n");
    }

    this.onVCamKeyUp = function onVCamKeyUp(aKeyCodeMaybe)
    {
      dump("In onVCamKeyUp, keyCode was " + (aKeyCodeMaybe ? aKeyCodeMaybe : "null") + "\n");
    }

    this.onVCamTreeChange = function onVCamTreeChange(treeEvent)
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

    this.onVCamDragLeave = function onVCamDragLeave(x, y) {}

    this.onVCamDragMove = function(x, y) {
      // parameters are unused
      var data;
      const kHTMLMime = "text/html";

      try {
        if (DNDUtils.checkCanDrop([kHTMLMime])) {
          data = DNDUtils.getData(kHTMLMime, 0);
          if (/<math/.test(data)) {
            return 1;
          }
        }
      }
      catch(e) {
        throw new MsiException('in VCam drag/move', e);
      }
      return 0;
    };

    this.onVCamDrop = function(x, y) {
      var data;
      const kHTMLMime = "text/html";

      try {
        if (this.editorElement) {
          if (DNDUtils.checkCanDrop([kHTMLMime])) {
            data = DNDUtils.getData(kHTMLMime, 0);
            if (/<math/.test(data)) {
              alert("Dropped "+data);
            }
          }
        }
      }
      catch(e) {
        throw new MsiException('in VCam drop', e);
      }
    };

    this.obj.addEvent('leftMouseDown', this.onVCamLeftMouseDown.bind(this, screenX, screenY));
    this.obj.addEvent('leftMouseUp', this.onVCamLeftMouseUp.bind(this));
    this.obj.addEvent('leftMouseDoubleClick', this.onVCamLeftDblClick.bind(this, screenX, screenY));
    this.obj.addEvent('rightMouseDown', this.onVCamRightMouseDown.bind(this, screenX, screenY));
    this.obj.addEvent('rightMouseUp', this.onVCamRightMouseUp.bind(this));
    this.obj.addEvent('dragMove', this.onVCamDragMove.bind(this));
    this.obj.addEvent('drop', this.onVCamDrop.bind(this));
    this.obj.addEvent('dragLeave', (function() {}));
    this.obj.addEvent("currentTimeChange", this.showAnimationTime.bind(this));

    if (editorElement) editorElement.focus();
  },

  doCommand: function(cmd, editorElement) {
    var doc = null;
    var editor = null;
    try {
      if (editorElement === null && msiGetActiveEditorElement != null) {
       editorElement = msiGetActiveEditorElement();
      }
    }
    catch(e) {} // it's ok to fail   if (editorElement) doc = editorElement.contentDocument;
    // netscape.security.PrivilegeManager.enablePrivilege('UniversalXPConnect');  // BBM: test to see if this is necessary
    if (editorElement) {
      editor = msiGetEditor(editorElement);
    }

    try {
      switch (cmd) {
        case "cmd_vcSelObj":
          this.obj.cursorTool = "select";
          break;
        case "cmd_vcRotateLeft":
          if (this.obj.rotateVerticalAction == 2) {
            this.obj.rotateVerticalAction = 0;
          } else {
            this.obj.rotateVerticalAction = 2;
          }
          break;
        case "cmd_vcRotateRight":
          if (this.obj.rotateVerticalAction == 1) {
            this.obj.rotateVerticalAction = 0;
          } else {
            this.obj.rotateVerticalAction = 1;
          }
          break;
        case "cmd_vcRotateUp":
          if (this.obj.rotateHorizontalAction == 1) {
            this.obj.rotateHorizontalAction = 0;
          } else {
            this.obj.rotateHorizontalAction = 1;
          }
         break;
        case "cmd_vcRotateDown":
          if (this.obj.rotateHorizontalAction == 2) {
            this.obj.rotateHorizontalAction = 0;
          } else {
            this.obj.rotateHorizontalAction = 2;
          }
         break;
        case "cmd_vcRotateScene":
          this.obj.cursorTool = "rotate";
          break;
        case "cmd_vcZoom":
          this.obj.cursorTool = "zoom";
          break;
        case "cmd_vcMove":
          this.obj.cursorTool = "move";
          break;
        case "cmd_vcQuery":
          this.obj.cursorTool = "query";
          break;
        case "cmd_vcZoomBoxIn":
          this.obj.cursorTool = "select"; // This.obj is zoomBoxIn in the 2d case
          break;
          case "cmd_vcZoomBoxOut":
          this.obj.cursorTool = "select";
          break;
        case "cmd_vcZoomIn":
          this.obj.cursorTool = "zoomIn";
          break;
        case "cmd_vcZoomOut":
          this.obj.cursorTool = "zoomOut";
          break;
        case "cmd_vcSnapshot":
          this.makeSnapshot();
          break;
        case "cmd_vcAutoSpeed":
          dump("cmd_vcAutoSpeed not implemented");
          break;
        case "cmd_vcAnimSpeed":
          dump("cmd_vcAnimSpeed not implemented");
          break;
        case "cmd_vcAutoZoomIn":
          if (this.obj.zoomAction == 1) this.obj.zoomAction = 0;
          else this.obj.zoomAction = 1;
          break;
        case "cmd_vcAutoZoomOut":
          if (this.obj.zoomAction == 2) this.obj.zoomAction = 0;
          else this.obj.zoomAction = 2;
          break;
        case "cmd_vcGoToEnd":
          this.obj.currentTime = this.obj.endTime;
          break;
        case "cmd_vcGoToStart":
          this.obj.currentTime = this.obj.beginTime;
          break;
        case "cmd_vcLoopType":
          dump("cmd_vcLoopType not implemented");
          break;
        case "cmd_vcPlay":
          if (isRunning) this.obj.stopAnimation();
          else this.obj.startAnimation();
          isRunning = !isRunning;
          break;
        case "cmd_vcFitContents":
          this.obj.fitContents();
          break;
        //    case "cmd_refresh":  // shouldn't be needed, but this causes the display to refresh
        //      this.camera.focalPointX = this.camera.focalPointX;
        //      break;

        default:
      }
    }
    catch (e) {
      throw new MsiException("Unable to execute VCam command "+cmd, e);
    }
  },

  makeSnapshot: function() {
    var editorElement;
    try {
      editorElement = msiGetActiveEditorElement();
    }
    catch(e) {
      return;
    }
    msiRequirePackage(editorElement, "wrapfig", "");
    var doc = editorElement.contentDocument;
    var ready = this.obj.readyState;
    if (ready > 1) {
      try {
        var path = this.makeSnapshotPath();
        var abspath;
        var abspath2;
        var prefs;
        var res;
        var DOMGraph;
        var graph = null;
        var plotWrapper = this.obj.parentNode;
        try {
          prefs = GetPrefs();
          res = prefs.getIntPref("swp.graph.snapshotres");
        } catch (e) {
          res = 300;
        }
        if (graph == null) {
          graph = new Graph();
          DOMGraph = msiFindParentOfType(this, "graph");
          graph.extractGraphAttributes(DOMGraph);
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
        this.obj.makeSnapshot(abspath, res);
        if ( getOS(window) == "win")  {
          graphicsConverter.init(window, snapshotDir.parent);
          // oldsnapshot is an nsIFile pointing to the .bmp file
          abspath = graphicsConverter.copyAndConvert(oldsnapshot, false);
        }

        this.insertSnapshot(abspath);
      } catch (e) {
        throw new MsiException("Error inserting plot snapshot", e);
      }
    }
  },

  makeSnapshotPath: function() {
    var extension;
    if ( getOS(window) == "win")  {
      extension = "bmp";
    } else {
      extension = "png";
    }
    var path;
    try {
      var strSrc = this.obj.getAttribute("data") || this.obj.getAttribute("src");
      var match = /plots\/(.*$)/.exec(strSrc);
      var fileName = match[1];
      fileName = fileName.replace(/xv[cz]$/, extension);
      path = "graphics/" + fileName;
    }
    catch (e) {
      throw new MsiException("Error finding path for plot snapshot", e);
    }
    return path;
  },

  insertSnapshot: function(abssnapshotpath) {
    var parent, i, objectlist, element, ssobj, graph, gslist, w, h, units, oldpath, file, url;
    parent = this.obj.parentNode;
    objectlist = parent.getElementsByTagName("object");
    var snapshotUrl = msiFileURLFromAbsolutePath(abssnapshotpath);
    var snapshotRelUrl;
    if (snapshotUrl) {
      snapshotRelUrl= msiMakeRelativeUrl(snapshotUrl.spec);
    } else {
      snapshotRelUrl = abssnapshotpath; // we were actually passed a relative url to begin with
    }

    // remove current snapshots
    for (i = 0; i < objectlist.length;) {
      element = objectlist[i];
      if (element.hasAttribute("msisnap")) {
        parent.removeChild(element);
      } else {
        i++;
      }
    }

    graph = msiFindParentOfType(this.obj, "graph");
    gslist = graph.getElementsByTagName("graphSpec");
    if (gslist.length > 0) {
      gslist = gslist[0];
    }
    ssobj = this.obj.cloneNode(true); // copies useful attributes
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
  },



};

function rebuildSnapshots(doc)
{
  netscape.security.PrivilegeManager.enablePrivilege('UniversalXPConnect');
  // BBM: test to see if this is necessary

  var wrapperlist, objlist, length, objlength, i, obj, snapshot;
  var obj = null;
  var doRefresh = false;
  var aVCamObject;
  wrapperlist = doc.documentElement.getElementsByTagName("msiframe");
  length = wrapperlist.length;
  for (i = 0; i < length; i++) {
    if (wrapperlist[i].parentNode.tagName === "graph")
    {
      obj = wrapperlist[i].firstChild;
      if (obj) {
        img = obj.nextSibling;
      }
      else continue;
      aVCamObject = new VCamObject(obj);
      aVCamObject.makeSnapshot();
    }
  }
}

function VCamCommand(cmd) {
  currentVCamObject.doCommand(cmd);
}


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

