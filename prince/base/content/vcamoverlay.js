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

var vcamObjArray = [];
var vcamIdArray = [];
var vcamWrapperArray = [];

// Dizzy array of objects:
//
// We hold onto html vcam plugin objects to get around security problems
// A VCamObject contains cached state values of a plugin object, and contains the 
// plugin object itself.

var currentVCamObjectNum = -1;

function setCurrentVCamObject(obj) {
  currentVCamObjectNum = vcamIdArray.indexOf(obj.id);
  if (currentVCamObjectNum < 0) {
    return saveObj(obj);
  }
  return currentVCamObjectNum;
}

// stores a pblugin object; returns its index
function saveObj(pluginobj) {
  var index;
  try {
    index = vcamIdArray.indexOf(pluginobj.id); // if the id exists, overwrite it.
    if (index === -1) {
      index = vcamIdArray.length;         // we will do the equivalent of a push
    }
    vcamWrapperArray[index] = new VCamObject(pluginobj);
    vcamObjArray[index] = pluginobj;
    vcamIdArray[index] = pluginobj.id;
  } catch (e) {
    msidump(e.message);
  }
  return index;
}

function VCamObject(vcamObject) {
  var index = -1;
  var id = vcamObject.id;
  netscape.security.PrivilegeManager.enablePrivilege('UniversalXPConnect'); // BBM: test to see if this is necessary
  vcamObject.wrapper = this;
  index = vcamIdArray.indexOf(id);
  if (index >= 0) {
    this.obj = vcamObjArray[index];
  } else {
    if (vcamObject.wrappedJSObject) {
      this.obj = vcamObject.wrappedJSObject;
    } else {
      this.obj = vcamObject;
    }
    // saveObj(this.obj);
  }
  if (this.obj) this.init();
}

VCamObject.prototype = {
  initialized: false,
  // plot info
  threedplot: false,
  twodplot: false,
  animplot: false,
  cursorTool: null,
  zoomAction: null,
  verticalAction: null,
  horizontalAction: null,
  actionSpeed: null,
  animationSpeed: null,

  setupUI: function() {
    if (!this.initialized) this.init();
    if (this.initialized) {
      this.initToolbar();
      document.getElementById("vcamactive").setAttribute("hidden", "false");
      // The above line reveals the VCam toolbar,
    }
  },

  init: function() {
    var editorElement, o, editor
    // Tuning constants
    var interval = 100;
    var count = 20;
    var _this = this;
    netscape.security.PrivilegeManager.enablePrivilege('UniversalXPConnect'); // BBM: test to see if this is necessary
    if (!this.obj.readyState || !this.obj.cursorTool) {
      editorElement = msiGetActiveEditorElement();
      if (editorElement) {
        editor = msiGetEditor(editorElement);
      }
      o = editor.document.getElementById(this.obj.id);
      if (o.readyState) {
        this.obj = o;
      }
    }
    var timeoutId;
    if (!this.initialized) {
      timeoutId = window.setInterval(function (_this) {
        if (count-- === 0) {
          window.clearInterval(timeoutId);
        }
        else {
          if (_this.obj.readyState && _this.obj.readyState === 2) {
            _this.cursorTool = _this.obj.cursorTool;
            _this.twodplot = (2 === _this.obj.dimension);
            _this.threedplot = !_this.twodplot;
            _this.animplot = _this.obj.isAnimated;
            _this.zoomAction = _this.obj.zoomAction;
            _this.verticalAction = _this.obj.rotateVerticalAction;
            _this.horizontalAction = _this.obj.rotateHorizontalAction;
            _this.actionSpeed = _this.obj.actionSpeed;
            _this.animationSpeed = _this.obj.animationSpeed;
            _this.initEventHandlers();
            _this.initialized = true;
            window.clearInterval(timeoutId);
          }
        }
      }, interval, _this);
    }
  },

  initToolbar: function() {
    // sets the state of the toolbar buttons from the plugin state
    // Call the function for initialization and after each button action.
    if (!this.initialized) return;

    // Cursor tool section

    document.getElementById("vc-Reset").removeAttribute("disabled");
    document.getElementById("vc-FitContents").removeAttribute("disabled");
    document.getElementById("vc-SnapShot").removeAttribute("disabled");
    try {
      msiGetActiveEditorElement();
    } catch (e) {
      document.getElementById("vc-SnapShot").setAttribute("hidden");
      // this disables snapshots if we are not in an editor; e.g., in a help file.
    }
    document.getElementById("vc-SelObj").checked = (this.cursorTool == 'select');
    document.getElementById("vc-SelObj").removeAttribute("disabled");
    document.getElementById("vc-RotateScene").checked = (this.cursorTool == 'rotate');
    document.getElementById("vc-RotateScene").removeAttribute("disabled");
    document.getElementById("vc-Move").checked = (this.cursorTool == 'move');
    document.getElementById("vc-Move").removeAttribute("disabled");
    document.getElementById("vc-Query").checked = (this.cursorTool == 'query');
    document.getElementById("vc-Query").removeAttribute("disabled");
    document.getElementById("vc-AutoZoomIn").checked = (this.zoomAction == 1);
    document.getElementById("vc-AutoZoomIn").removeAttribute("disabled");
    document.getElementById("vc-AutoZoomOut").checked = (this.zoomAction == 2);
    document.getElementById("vc-AutoZoomOut").removeAttribute("disabled");

    // Rotation section
    if (this.threedplot) {
      document.getElementById("vc-Zoom").checked = (this.cursorTool === 'zoom');
      document.getElementById("vc-Zoom").removeAttribute("disabled");
      document.getElementById("vc-RotateRight").checked = (this.verticalAction == 1);
      document.getElementById("vc-RotateRight").removeAttribute("disabled");
      document.getElementById("vc-RotateLeft").checked = (this.verticalAction == 2);
      document.getElementById("vc-RotateLeft").removeAttribute("disabled");
      document.getElementById("vc-RotateUp").checked = (this.horizontalAction == 1);
      document.getElementById("vc-RotateUp").removeAttribute("disabled");
      document.getElementById("vc-RotateDown").checked = (this.horizontalAction == 2);
      document.getElementById("vc-RotateDown").removeAttribute("disabled");

      // Speed control
      if (this.actionSpeed <= 0.125) {
        document.getElementById("actionspeed8s").checked = true;
      } else if (this.actionSpeed > 0.125 && this.actionSpeed <= 0.25) {
        document.getElementById("actionspeed4s").checked = true;
      } else if (this.actionSpeed > 0.25 && this.actionSpeed <= 0.5) {
        document.getElementById("actionspeed2s").checked = true;
      } else if (this.actionSpeed > 0.5 && this.actionSpeed <= 1.0) {
        document.getElementById("actionspeedN").checked = true;
      } else if (this.actionSpeed > 1.0 && this.actionSpeed <= 2.0) {
        document.getElementById("actionspeed2f").checked = true;
      } else if (this.actionSpeed > 2.0 && this.actionSpeed <= 4.0) {
        document.getElementById("actionspeed4f").checked = true;
      } else if (this.actionSpeed > 4.0) {
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
    if (this.animplot) {
      // animation section, including animation speed control
      document.getElementById("animplot").removeAttribute("hidden");
      if (this.animationSpeed <= 0.125) document.getElementById("animspeed8s").checked = true;
      else if (this.animationSpeed > 0.125 && this.animationSpeed <= 0.25) document.getElementById(
        "animspeed4s").checked = true;
      else if (this.animationSpeed > 0.25 && this.animationSpeed <= 0.5) document.getElementById(
        "animspeed2s").checked = true;
      else if (this.animationSpeed > 0.5 && this.animationSpeed <= 1.0) document.getElementById(
        "animspeedN").checked = true;
      else if (this.animationSpeed > 1.0 && this.animationSpeed <= 2.0) document.getElementById(
        "animspeed2f").checked = true;
      else if (this.animationSpeed > 2.0 && this.animationSpeed <= 4.0) document.getElementById(
        "animspeed4f").checked = true;
      else if (this.animationSpeed > 4.0) document.getElementById("animspeed8f").checked = true;
      document.getElementById("animspeed8s").removeAttribute("disabled");
      document.getElementById("animspeed4s").removeAttribute("disabled");
      document.getElementById("animspeed2s").removeAttribute("disabled");
      document.getElementById("animspeedN" ).removeAttribute("disabled");
      document.getElementById("animspeed2f").removeAttribute("disabled");
      document.getElementById("animspeed4f").removeAttribute("disabled");
      document.getElementById("animspeed8f").removeAttribute("disabled");
    }
    document.getElementById("3dplot").hidden = !this.threedplot;
    document.getElementById("2dplot").hidden = !this.twodplot;
    document.getElementsByTagName("animplot").hidden = !this.animplot;
  },

  initEventHandlers: function() {
    // there might not be an editor element; e.g., when displaying help files
    var editorElement = null;
    var editor = null;
    try {
      if (msiGetActiveEditorElement != null) {
        editorElement = msiGetActiveEditorElement();
        if (editorElement) {
          editor = msiGetEditor(editorElement);
        }
      }
    } catch (e) {} // it's ok to fail
    this.obj.addEvent('leftMouseDown', this.onVCamLeftMouseDown);
    this.obj.addEvent('leftMouseUp', this.onVCamLeftMouseUp);
    this.obj.addEvent('leftMouseDoubleClick', this.onVCamLeftDblClick);
    this.obj.addEvent('rightMouseDown', this.onVCamRightMouseDown);
    this.obj.addEvent('rightMouseUp', this.onVCamRightMouseUp);
    this.obj.addEvent('dragMove', this.onVCamDragMove);
    this.obj.addEvent('drop', this.onVCamDrop);
    this.obj.addEvent('dragLeave', this.onVCamDragLeave);
    if (this.showAnimationTime) this.obj.addEvent("currentTimeChange", this.showAnimationTime);

    if (editorElement) editorElement.focus();
  },

  setAnimationTime: function() {
    var time = this.obj.beginTime + (document.getElementById("vc-AnimScale").value / 100) *
      (this.obj.endTime - this.obj.beginTime);
    this.obj.currentTime = time;
    this.obj.startAnimation();
  },

  showAnimationTime: function(time) {
    var newval = Math.round(100 * (time / (this.obj.endTime - this.obj.beginTime)));
    document.getElementById("vc-AnimScale").value = newval;
  },

  setAnimSpeed: function(factor) {
    this.obj.animationSpeed = this.animationSpeed = factor;
  },

  setLoopMode: function(mode) {
    this.obj.animationLoopingMode = mode;
  },

  setActionSpeed: function(factor) {
    this.obj.actionSpeed = this.actionSpeed = factor;
  },

  // define mouse handlers

  onVCamLeftMouseUp: function onVCamLeftMouseUp() {
    // alert("left mouse up in plugin!");
  },

  onVCamLeftMouseDown: function onVCamLeftMouseDown(screenX, screenY) {
#ifdef XP_MACOSX
    return;
#endif
    try {
      if (msiGetActiveEditorElement != null) {
        var editorElement = msiGetActiveEditorElement();
        var editor = msiGetEditor(editorElement);
        editor.selection.collapse(this.parentNode, 0);
        editor.selection.extend(this.parentNode, 1);
        editor.checkSelectionStateForAnonymousButtons(editor.selection);
        this.wrapper.setupUI();
      }
    } catch (e) {}
  },

  onVCamRightMouseUp: function onVCamRightMouseUp() {
    // alert("right mouse up in plugin!");
  },

  onVCamLeftDblClick: function onVCamLeftDblClick(screenX, screenY) {
    // alert('double click');
    try {
      if (msiGetActiveEditorElement != null) {
        var editorElement = msiGetActiveEditorElement();
        goDoPrinceCommand("cmd_objectProperties", This.parentNode.parentNode, editorElement); // go up to the graph object
      }
    } catch (e) {}
  },

  onVCamRightMouseDown: function onVCamRightMouseDown(screenX, screenY) {
    try {
      var editorElement = msiGetActiveEditorElement();
    } catch (e) {
      return;
    }
    var editor = msiGetEditor(editorElement);
    var graphNode = editor.getElementOrParentByTagName("graph", this.obj);
    var contextMenu = document.getElementById("msiEditorContentContext");
    if (contextMenu) {
      contextMenu.showPopup(graphNode, screenX, screenY, "none", "none");
      contextMenu.focus();
    }
  },

  onVCamRightMouseUp: function onVCamRightMouseUp(screenX, screenY) // BBM: ???
    {
#ifdef XP_MACOSX
    return;
#endif
      try {
        if (msiGetActiveEditorElement != null) {
          var editorElement = msiGetActiveEditorElement();
          var editor = msiGetEditor(editorElement);
          var evt = editor.document.createEvent("MouseEvents");
          evt.initMouseEvent("mouseup", true, true, editor.document.defaultView, 1,
            screenX, screenY, null, null, 0, 0, 0, 0, 2, null);
          this.parentNode.dispatchEvent(evt);
        }
      } catch (e) {}
    },

  onVCamKeyDown: function onVCamKeyDown(aKeyCodeMaybe) {
    dump("In onVCamKeyDown, keyCode was " + (aKeyCodeMaybe ? aKeyCodeMaybe : "null") + "\n");
  },

  onVCamKeyUp: function onVCamKeyUp(aKeyCodeMaybe) {
    dump("In onVCamKeyUp, keyCode was " + (aKeyCodeMaybe ? aKeyCodeMaybe : "null") + "\n");
  },

  onVCamTreeChange: function onVCamTreeChange(treeEvent) {
    try {
      msidump("Reported VCam tree change event: type is [" + treeEvent.type +
        "], target node is [" + treeEvent.target.nodeName + "], property changed is [" +
        treeEvent.property + "]\n");
    } catch (exc) {
      msidump("Got exception in onVCamTreeChange: " + exc + "\n");
    }
  },

  onVCamDragLeave: function onVCamDragLeave(x, y) {},

  onVCamDragMove: function(x, y) {
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
    } catch (e) {
      throw new MsiException('in VCam drag/move', e);
    }
    return 0;
  },

  onVCamDrop: function(x, y) {
    var data;
    const kHTMLMime = "text/html";

    try {
      if (this.editorElement) {
        if (DNDUtils.checkCanDrop([kHTMLMime])) {
          data = DNDUtils.getData(kHTMLMime, 0);
          if (/<math/.test(data)) {
            alert("Dropped " + data);
          }
        }
      }
    } catch (e) {
      throw new MsiException('in VCam drop', e);
    }
  },

  doCommand: function(cmd, editorElement) {
    var doc = null;
    var editor = null;
    try {
      if (editorElement === null && msiGetActiveEditorElement != null) {
        editorElement = msiGetActiveEditorElement();
      }
    } catch (e) {} // it's ok to fail   if (editorElement) doc = editorElement.contentDocument;
    // netscape.security.PrivilegeManager.enablePrivilege('UniversalXPConnect');  // BBM: test to see if this is necessary
    if (editorElement) {
      editor = msiGetEditor(editorElement);
    }

    try {
      switch (cmd) {
        case "cmd_vcSelObj":
          this.obj.cursorTool = this.cursorTool = "select";
          break;
        case "cmd_vcRotateLeft":
          if (this.verticalAction == 2) {
            this.obj.rotateVerticalAction = this.verticalAction = 0;
          } else {
            this.obj.rotateVerticalAction = this.verticalAction = 2;
          }
          break;
        case "cmd_vcRotateRight":
          if (this.verticalAction == 1) {
            this.obj.rotateVerticalAction = this.verticalAction = 0;
          } else {
            this.obj.rotateVerticalAction = this.verticalAction = 1;
          }
          break;
        case "cmd_vcRotateUp":
          if (this.obj.horizontalAction == 1) {
            this.obj.rotateHorizontalAction = 0;
            this.horizontalAction = 0;
          } else {
            this.obj.rotateHorizontalAction =1;
            this.horizontalAction = 1;
          }
          break;
        case "cmd_vcRotateDown":
          if (this.horizontalAction == 2) {
            this.obj.rotateHorizontalAction = this.horizontalAction = 0;
          } else {
            this.obj.rotateHorizontalAction = this.horizontalAction = 2;
          }
          break;
        case "cmd_vcRotateScene":
          this.obj.cursorTool = this.cursorTool = "rotate";
          break;
        case "cmd_vcZoom":
          this.obj.cursorTool = this.cursorTool = "zoom";
          break;
        case "cmd_vcMove":
          this.obj.cursorTool = this.cursorTool = "move";
          break;
        case "cmd_vcQuery":
          this.obj.cursorTool = this.cursorTool = "query";
          break;
        case "cmd_vcZoomBoxIn":
          this.obj.cursorTool = this.cursorTool = "select"; // This.obj is zoomBoxIn in the 2d case
          break;
        case "cmd_vcZoomBoxOut":
          this.obj.cursorTool = this.cursorTool = "select";
          break;
        case "cmd_vcZoomIn":
          this.obj.cursorTool = this.cursorTool = "zoomIn";
          break;
        case "cmd_vcZoomOut":
          this.obj.cursorTool = this.cursorTool = "zoomOut";
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
          if (this.obj.zoomAction == 1) this.obj.zoomAction = this.zoomAction = 0;
          else this.obj.zoomAction = this.zoomAction = 1;
          break;
        case "cmd_vcAutoZoomOut":
          if (this.obj.zoomAction == 2) this.obj.zoomAction = this.zoomAction = 0;
          else this.obj.zoomAction = this.zoomAction = 2;
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
    } catch (e) {
      throw new MsiException("Unable to execute VCam command " + cmd, e);
    }
  },

  makeSnapshot: function() {
    var editorElement;
    try {
      editorElement = msiGetActiveEditorElement();
    } catch (e) {
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
        netscape.security.PrivilegeManager.enablePrivilege('UniversalXPConnect'); // BBM: test to see if this is necessary

        // if (typeof this.obj.makeSnapshot !== 'function') {
        //   this.obj = new VCamObject(this.obj);
        // }
        this.obj.makeSnapshot(abspath, res);
        if (getOS(window) == "win") {
          abspath = convertBMPtoPNG(oldsnapshot, this.obj.dimension == 3);
        }
        this.insertSnapshot(abspath);
      } catch (e) {
        throw new MsiException("Error inserting plot snapshot", e);
      }
    }
  },

  makeSnapshotPath: function() {
    var extension;
    if (getOS(window) == "win") {
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
    } catch (e) {
      throw new MsiException("Error finding path for plot snapshot", e);
    }
    return path;
  },

  insertSnapshot: function(abssnapshotpath) {
    var parent, i, objectlist, element, ssobj, graph, gslist, w, h, units, oldpath, file, url, editorElement, editor;
    parent = this.obj.parentNode;
    objectlist = parent.getElementsByTagName("object");
    var snapshotUrl = msiFileURLFromAbsolutePath(abssnapshotpath);
    var snapshotRelUrl;
    if (snapshotUrl) {
      snapshotRelUrl = msiMakeRelativeUrl(snapshotUrl.spec);
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
      units = gslist.getAttribute("Units");
      editorElement = msiGetActiveEditorElement();
      if (editorElement) {
        editor = msiGetEditor(editorElement);
      }
      w = gslist.getAttribute("Width") + units;
      if (w) {
        ssobj.setAttribute("naturalWidth", w);
        ssobj.setAttribute("ltx_width", w);
        setStyleAttributeOnNode(ssobj, "width", w, editor)
      }
      h = gslist.getAttribute("Height")+units;
      if (h) {
      }
      ssobj.setAttribute("units", units);
    }
  },
};

function initVCamObjects(doc) {
  netscape.security.PrivilegeManager.enablePrivilege('UniversalXPConnect');
  // BBM: test to see if this is necessary

  var wrapperlist, length, i, obj;
  var obj = null;
  vcamlastId = 0;
  wrapperlist = doc.documentElement.getElementsByTagName("msiframe");
  length = wrapperlist.length;
  for (i = 0; i < length; i++) {
    if (wrapperlist[i].parentNode.tagName === "graph") {
      obj = wrapperlist[i].firstChild;
      if (obj && obj.nodeName === "object") {
        vcamlastId++;
        saveObj(obj);
        var x=3; // for debugger
      }
    }
  }
}

function rebuildSnapshots(doc) {
  netscape.security.PrivilegeManager.enablePrivilege('UniversalXPConnect');
  // BBM: test to see if this is necessary

  var wrapperlist, objlist, length, objlength, i, obj, snapshot;
  var obj = null;
  var doRefresh = false;
  var aVCamObjectNum;
  wrapperlist = doc.documentElement.getElementsByTagName("msiframe");
  length = wrapperlist.length;
  for (i = 0; i < length; i++) {
    if (wrapperlist[i].parentNode.tagName === "graph") {
      obj = wrapperlist[i].firstChild;
      if (obj) {
        img = obj.nextSibling;
      } else continue;
      aVCamObjectNum = setCurrentVCamObject(obj);
      vcamWrapperArray[aVCamObjectNum].makeSnapshot();
    }
  }
}

function VCamCommand(cmd) {
  vcamWrapperArray[currentVCamObjectNum].doCommand(cmd);
}


function queryVCamValues(obj, graph, domGraph, bUserSetIfChanged) {
  var cameraVals = null;
  var coordSysVals = {
    XAxisTitle: "x",
    YAxisTitle: "y",
    ViewingBoxXMin: "-5",
    ViewingBoxXMax: "5",
    ViewingBoxYMin: "-5",
    ViewingBoxYMax: "5"
  };
  var camera, vcamDoc, kidNode, coordSysNode, sceneNode
  var dim = graph.Dimension;
  var sceneNodeName = "Scene" + dim + "d";
  var docElement;
  var coordSysNodeName = "CoordinateSystem" + dim + "d";
  var aProp;

  if (obj.wrappedJSObject) {
    obj = obj.wrappedJSObject;
  }

  if (dim == 3) {
    cameraVals = {
      positionX: "0",
      positionY: "0",
      positionZ: "0",
      focalPointX: "0",
      focalPointY: "0",
      focalPointZ: "0",
      upVectorX: "0",
      upVectorY: "0",
      upVectorZ: "1",
      keepUpVector: "false",
      viewingAngle: "2.0944",
      orthogonalProjection: "false"
    };
    coordSysVals.ViewingBoxZMin = "-5";
    coordSysVals.ViewingBoxZMax = "5";
    coordSysVals.ZAxisTitle = "z";
    if (graph.camera) {
      for (aProp in cameraVals) {
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
    } else
      cameraVals = null;
    if (graph) {
      graph.setCameraValsFromVCam(cameraVals, domGraph, bUserSetIfChanged);
    }
  }
  if (graph.isAnimated()) {
    animVals = {
      beginTime: 0,
      endTime: 10,
      currentTime: 0
    };
    for (aProp in animVals)
      animVals[aProp] = String(graph[aProp]);
    animVals.framesPerSecond = 5;
    if (graph)
      graph.setAnimationValsFromVCam(animVals, domGraph, bUserSetIfChanged);
  }
  vcamDoc = obj.document;
  if (vcamDoc) {
    try {
      if (vcamDoc.wrappedJSObject) {
        vcamDoc = vcamDoc.wrappedJSObject;
      }
      docElement = vcamDoc.documentElement;
      if (docElement.wrappedJSObject) {
        docElement = docElement.wrappedJSObject;
      }
      sceneNode = getChildByTagName(docElement, sceneNodeName)
      coordSysNode = getChildByTagName(sceneNode, coordSysNodeName);
      if (coordSysNode) {
        kidNode = coordSysNode.firstChild;
        while (kidNode) {
          if (kidNode.nodeName in coordSysVals)
            coordSysVals[kidNode.nodeName] = String(kidNode.firstChild.nodeValue);
          kidNode = kidNode.nextSibling;
        }
      }
    } catch (e) {
      msidump(e.message);
    }
  }
  if (!coordSysNode)
    coordSysVals = null;
  if (graph)
    graph.setCoordSysValsFromVCam(coordSysVals, domGraph, bUserSetIfChanged);

}


function doVCamClose() {
  if (document.getElementById("vcamactive") && (document.getElementById("vcamactive").getAttribute(
      "hidden") == "false"))
    return; //In other words, if we're being called because of the broadcaster turning VCam on, do nothing
  // var editorElement = msiGetActiveEditorElement();
  //  return vcamCloseFunction(editorElement);   //BBM: fix this up
}

// oldsnapshot is an nsIFile pointing to the .bmp file
function convertBMPtoPNG( aFile, is3d ) {
  // used only for converting BMP 3-D snapshots to PNG on Windows.
  var leaf = aFile.leafName;  
  var x;
  if (!is3d) {
    x=3;  // something to break on
  }
  var basename = leaf.replace(/\.bmp$/,'');
  var workDirectory = aFile.parent.parent.clone();
  var utilityDir;
  var wscript;
  var invisscript;
  var process;
  var dsprops = Components.classes["@mozilla.org/file/directory_service;1"].getService(Components.interfaces.nsIProperties);
  var codeFile = dsprops.get("CurProcD", Components.interfaces.nsIFile);
  wscript = dsprops.get("WinD", Components.interfaces.nsIFile);
  wscript.append('system32');
  wscript.append('WScript.exe');
  workDirectory.append('gcache');
  if (! workDirectory.exists()) workDirectory.create(1, 0775);
  workDirectory = workDirectory.parent;
  codeFile.append('invis.vbs');

  invisscript = codeFile.path;
  codeFile = codeFile.parent;
  codeFile.append("utilities");
  utilityDir = codeFile.path;

  process = Components.classes["@mozilla.org/process/util;1"].createInstance(Components.interfaces.nsIProcess);
  process.init(wscript);
  // process.run(true, [invisscript, codeFile.path, workDirectory.path, utilityDir, basename, (is3d ? 1 : 0 )], 6);
  process.run(true, [invisscript, workDirectory.path, utilityDir, basename, 1], 5);
  var outfile = workDirectory.clone();
  outfile.append('gcache');
  outfile.append(basename+'.png');
  return outfile.path;
}
