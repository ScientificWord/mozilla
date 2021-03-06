/*****************************************************************************

VCam object section.

The following functions will be bound to VCam plot objects, so 'this' in the
code refers to the plot object. The attachment of the functions to the object
happen when the object is instantiated, i.e. clicked on, in doVCamInitialize.

******************************************************************************/

/* This code is called both when editing a document with plots, and when displaying
help files that contain plots. The difference is that in the latter case there is no
editor element, and msiGetActiveEditorElement is not defined. Hence there are
a number of try blocks surrounding calls to msiGetActiveEditorElement. */


// VCamObject definition

// Constructor and prototype
Components.utils.import("resource://app/modules/msiEditorDefinitions.jsm");

var vcamObjArray = [];
var vcamIdArray = [];
var vcamWrapperArray = [];

// Dizzy array of objects:
//
// We hold onto html vcam plugin objects to get around security problems
// A VCamObject contains cached state values of a plugin object, and contains the
// plugin object itself.

// If an HTMLObjectElement gets proxied to an XPCSafeJSObjectWrapper, we can rescue it from
// vcamObjArray by one of the next three functions

function objFromId(id) {
  try {
    return vcamObjArray[vcamIdArray.indexOf(id)];
  }
  catch(e) {
    return null;
  }
}

function objFromObj(obj) {
  if (obj.readyState == 2)
    return obj;
  return objFromId(obj.id);
}

var currentVCamObjectNum = -1;

function setCurrentVCamObject(obj) {
  currentVCamObjectNum = vcamIdArray.indexOf(obj.id);
  if (currentVCamObjectNum < 0) {
    return saveObj(obj);
  }
  return currentVCamObjectNum;
}

// stores a plugin object; returns its index
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
  wrappedJSObject: this,

  validateObj: function validateObj() {
    if (this.obj) {
      this.obj = objFromObj(this.obj);
      return true;
    } else
      return false;
  },

  setupUI: function(obj) {
    if (!this.initialized) this.init(obj);
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
    var obj;
    try {
      if (msiGetActiveEditorElement != null) {
        editorElement = msiGetActiveEditorElement();
        if (editorElement) {
          editor = msiGetEditor(editorElement);
        }
      }
    } catch (e) {} // it's ok to fail
    if (!this.validateObj()) return;
    obj = this.obj;
    obj.addEvent('leftMouseDown', this.onVCamLeftMouseDown);
    obj.addEvent('leftMouseUp', this.onVCamLeftMouseUp);
    obj.addEvent('leftMouseDoubleClick', this.onVCamLeftDblClick);
    obj.addEvent('rightMouseDown', this.onVCamRightMouseDown);
    obj.addEvent('rightMouseUp', this.onVCamRightMouseUp);
    obj.addEvent('dragMove', this.onVCamDragMove);
    obj.addEvent('drop', this.onVCamDrop);
    obj.addEvent('dragLeave', this.onVCamDragLeave);
    if (this.showAnimationTime) obj.addEvent("currentTimeChange", this.showAnimationTime);

    if (editorElement) editorElement.focus();
  },

  setAnimationTime: function() {
    if (!this.validateObj()) return;
    var time = this.obj.beginTime + (document.getElementById("vc-AnimScale").value / 100) *
      (this.obj.endTime - this.obj.beginTime);
    this.obj.currentTime = time;
    this.obj.startAnimation();
  },

  showAnimationTime: function(time) {
    if (!this.validateObj()) return;
    var newval = Math.round(100 * (time / (this.obj.endTime - this.obj.beginTime)));
    document.getElementById("vc-AnimScale").value = newval;
  },

  setAnimSpeed: function(factor) {
    if (!this.validateObj()) return;
    this.obj.animationSpeed = this.animationSpeed = factor;
  },

  setLoopMode: function(mode) {
    if (!this.validateObj()) return;
    this.obj.animationLoopingMode = mode;
  },

  setActionSpeed: function(factor) {
    if (!this.validateObj()) return;
    try {
      this.obj.actionSpeed = this.actionSpeed = factor;
    }
    catch(e) {
      msidump("exception: "+e.message);
    }
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
        this.wrapper.setupUI(this.obj);
      }
    } catch (e) {}
  },

  onVCamLeftDblClick: function onVCamLeftDblClick(screenX, screenY) {
    // alert('double click');
    try {
      if (msiGetActiveEditorElement != null) {
        var editorElement = msiGetActiveEditorElement();
        if (this.cursorTool !== "zoomIn" && this.cursorTool !== "zoomOut")
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

  onVCamDrop: function(event) {
    var data;
    var graphNode;
    var string;
    var editorElement = msiGetActiveEditorElement();
    const kHTMLMime = "text/html";

    try {
      if (DNDUtils.checkCanDrop([kHTMLMime])) {
        data = DNDUtils.getData(kHTMLMime, 0);
        string = data.QueryInterface(Components.interfaces["nsISupportsString"]);
        graphNode = this.parentNode.parentNode;
        newPlotFromText(graphNode, string.data, editorElement);
        // alert("Dropped " + string.data);
      }
    } catch (e) {
      throw new MsiException('in VCam drop', e);
    }
  },

  doCommand: function(cmd, editorElement) {
    var doc = null;
    var editor = null;
    if (!this.validateObj()) return;
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
          if (this.horizontalAction == 1) {
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
          this.makeSnapshot(true);
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
    case "cmd_vcReset":
      this.obj.resetViewpoint();
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
    catch(e) {
      finalThrow(cmdFailString(cmd), e.message);
    }
  },

  makeSnapshot: function( exporting ) {
    // This function serves two purposes. A snapshot is required whenever we need to display a plot in a location
    // not supported by VCam; these include in PDF and direct prints, in HTML documents, and for plots to be displayed
    // in Scientific Word.
    // The other purpose is to take a snapshot to be put into a location specified by the user, to be used by any program
    // that can display bitmap graphics. It is possible, as well, to have a single VCam plot saved in several variations
    // even in SWP and SNB.
    // The boolean parameter 'exporting' is true in the second case. In this case we must as the user for the seve location
    // Otherwise the snapshots go into the gcache subdirectory of the document directory.

    if (!this.validateObj()) return;

    var editorElement;
    try {
      editorElement = msiGetActiveEditorElement();
    } catch (e) {
      return;
    }
    if (!exporting) msiRequirePackage(editorElement, "wrapfig", "");
    var doc = editorElement.contentDocument;
    var ready = this.obj.readyState;
    if (!ready) {
      this.init();
      ready = this.obj.readyState;
    }
    if (ready > 1) {
      try {
        var path = this.makeSnapshotPath( exporting );
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
          // we really got a bmp; change the extension
          oldsnapshot.moveTo(null, oldsnapshot.leafName.replace(/png$/,'bmp'));
          abspath = convertBMPtoPNG(oldsnapshot, this.obj.dimension == 3, exporting);
        }
        if (!exporting) this.insertSnapshot(abspath);
      } catch (e) {
        throw new MsiException("Error inserting plot snapshot", e);
      }
    }
  },


// Make a path for the snapshot. If "exporting" is true, put up a dialog asking for
// a save location. Otherwise the file goes into the graphics subdirectory of the working
// directory.

  makeSnapshotPath: function( exporting ) {
    var extension = 'png';  // In the Windows case, we ask VCam for a bmp file which we convert to png.
                            // The user will not see the bmp extension at all.
    var path;
    var fp;
    if (exporting) fp = Components.classes["@mozilla.org/filepicker;1"].createInstance(msIFilePicker);
    try {
      var strSrc = this.obj.getAttribute("data") || this.obj.getAttribute("src");
      var match = /plots\/(.*$)/.exec(strSrc);
      var fileName = match[1];
      fileName = fileName.replace(/xv[cz]$/, extension);
      if (exporting) {
        fp.init(window, "Save snapshot", msIFilePicker.modeSave);
        fp.defaultExtension = extension;
        fp.defaultString=fileName;
        fp.appendFilter("Bitmap graphics",".png");
        var dialogResult = fp.show();
        if (dialogResult != msIFilePicker.returnCancel)
        path = fp.file.path;
      }
      else {
        path = "graphics/" + fileName;
      }
    }
    catch (e) {
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
        ssobj.setAttribute("width", w);
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
      if (wrapperlist[i].firstChild && wrapperlist[i].firstChild.nodeName === "object") {
        vcamlastId++;
        saveObj(wrapperlist[i].firstChild);
        var x=3; // for debugger
      }
    }
  }
}

function rebuildSnapshots(doc) {
  netscape.security.PrivilegeManager.enablePrivilege('UniversalXPConnect');
  // BBM: test to see if this is necessary

  var wrapperlist, objlist, length, i, obj;
  var obj = null;
  var doRefresh = false;
  var aVCamObjectNum;
  wrapperlist = doc.documentElement.getElementsByTagName("msiframe");
  length = wrapperlist.length;
  for (i = 0; i < length; i++) {
    if (wrapperlist[i].parentNode.tagName === "graph") {
      objlist = wrapperlist[i].getElementsByTagName('object');
      obj = objlist[0];
      if (obj) {
        aVCamObjectNum = setCurrentVCamObject(obj);
        vcamWrapperArray[aVCamObjectNum].makeSnapshot( false );
      }
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
function convertBMPtoPNG( aFile, is3d, exporting ) {
  // used only for converting BMP  snapshots to PNG on Windows.
  var leaf = aFile.leafName;
  var x;
  var utilityDir;
  var wscript;
  var invisscript;
  var process;
  var dsprops;
  var codeFile;
  var outfile;
  var sourcefile;
  var destfile;

  if (!is3d) {
    x=3;  // something to break on
  }
  var basename = leaf.replace(/\.bmp$/,'');
  var theDirectory;
  var workDirectory;
  if (exporting) {
    theDirectory = aFile.parent;
  }
  else {
    workDirectory = aFile.parent.parent.clone();
    workDirectory.append('gcache');
    if (! workDirectory.exists()) workDirectory.create(1, 0775);
    workDirectory = workDirectory.parent;
    theDirectory = workDirectory;
  }
  dsprops = Components.classes["@mozilla.org/file/directory_service;1"].getService(Components.interfaces.nsIProperties);
  codeFile = dsprops.get("CurProcD", Components.interfaces.nsIFile);
  wscript = dsprops.get("WinD", Components.interfaces.nsIFile);
  wscript.append('system32');
  wscript.append('WScript.exe');
  codeFile.append('invis.vbs');

  invisscript = codeFile.path;
  codeFile = codeFile.parent;
  codeFile.append("utilities");
  utilityDir = codeFile.path;
  sourcefile = theDirectory.clone();

  process = Components.classes["@mozilla.org/process/util;1"].createInstance(Components.interfaces.nsIProcess);
  process.init(wscript);
  // process.run(true, [invisscript, codeFile.path, workDirectory.path, utilityDir, basename, (is3d ? 1 : 0 )], 6);
  if (exporting) {
    process.run(true, [invisscript, utilityDir, theDirectory.path+"\\", theDirectory.path+"\\", basename, 1], 6);
    outfile = aFile.parent;
  }
  else {
    sourcefile.append('graphics');
    destfile = theDirectory.clone();
    destfile.append('gcache');
    process.run(true, [invisscript, utilityDir, sourcefile.path+"\\", destfile.path+"\\", basename, 1], 6);
    outfile = destfile;
  }
  sourcefile.append(basename+'.bmp');
  sourcefile.remove(true);
  outfile.append(basename+'.png');
  return outfile.path;
}
