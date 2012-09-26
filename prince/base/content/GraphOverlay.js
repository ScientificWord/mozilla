#include productname.inc
#ifndef PROD_SW
Components.utils.import("resource://app/modules/os.jsm");
//----------------------------------------------------------------------------------
// ************ Graph section ******

function Graph() {
  // these arrays enumerate the data for a graph. PLOTELEMENTS are in mathml
  // When adding to this list, you must also add to the MathServiceRequest in the compute engine
  // Compute/iCmpIDs.h and Compute/MRequest.cpp::NameToPID()
  // PlotStatus: UI use only. New/Inited/Deleted.
  var i, list, length;
  this.plots = []; //an array of plots, see below
  this.modFlag = {}; //a Boolean for each attribute in GRAPHATTRIBUTES
  this.errStr = "";
  this.currentDisplayedPlot = -1;
  list = this.graphAttributeList();
  length = list.length;
  for (i = 0; i < length; i++) {
    this[list[i]] = this.getDefaultValue(list[i]);
  }
  this.frame = new Frame(this);
}
Graph.prototype = {
  COMPATTRIBUTES: ["ImageFile", "XAxisLabel", "YAxisLabel", "ZAxisLabel", "Width", "Height",
                              "Units", "AxesType", "EqualScaling", "EnableTicks", "XTickCount",
                              "YTickCount", "AxesTips", "GridLines", "BGColor", "Dimension",
                              "AxisScale", "CameraLocationX", "CameraLocationY", "CameraLocationZ",
                              "FocalPointX", "FocalPointY", "FocalPointZ", "UpVectorX", "UpVectorY",
                              "UpVectorZ", "ViewingAngle", "OrthogonalProjection", "KeepUp",
                              "OrientationTiltTurn"],
  GRAPHATTRIBUTES: ["Key", "Name", "CaptionPlace"],
  constructor: Graph,
  ser: new XMLSerializer(),
  addPlot: function (plot) {
    // Add a plot to a graph. Conceptually, a plot is a collection of attribute/value pairs
    this.plots.push(plot);
    plot.parent = this;
    return (this.plots.length - 1);
  },
  deletePlot: function (plotnum) {
    if (plotnum >= 0 && plotnum < this.plots.length) {
      this.plots.splice(plotnum, 1);
    }
    return (this.plots.length);
  },
  getNumPlots: function () {
    return (this.plots.length);
  },
  isModified: function (x) {
    return (this.modFlag[x]);
  },
  setModified: function (x) {
    this.modFlag[x] = true;
  },
  computeGraph: function (editorElement, filename) {
    // call the compute engine to create an image
    ComputeCursor(editorElement);
    var str = this.serializeGraph();
    if (this.errStr === "") {
      try {
        var topWin = msiGetTopLevelWindow();
        topWin.msiComputeLogger.Sent4("plotfuncCmd", filename, str, "");
        var out = GetCurrentEngine().plotfuncCmd(str);
        msiComputeLogger.Received(out);
      }
      catch (e) {
        alert("Computation Error", "Compute Graph: " + GetCurrentEngine().getEngineErrors());
        msiComputeLogger.Exception(e);
      }
    } else {
      msidump(this.errStr);
    }
    RestoreCursor(editorElement);
  },
  computeQuery: function (plot) {
    // call the compute engine to guess at graph attributes
    // If plot is not null, then we build a graph with a single plot to send to the engine.
    // Otherwise, all the plots that are children of the graph are included
    var str = this.serializeGraph(plot);
    var eng = GetCurrentEngine();
    var status, i;

    try {
      msiComputeLogger.Sent4("plotfuncQuery", "", str, "");
      var out = eng.plotfuncQuery(str);
      msidump("ComputeQuery plotfuncQuery returns " + out + "\n");
      msiComputeLogger.Received(out);
      parseQueryReturn(out, this, plot);
      status = "Inited";
    }
    catch (e) {
      status = "ERROR";
      dump("Computation Error", "Query Graph: " + eng.getEngineErrors() + "\n");
      msiComputeLogger.Exception(e);
    }
    finally {
      if (plot) {
        plot.attributes["PlotStatus"] = status;
      }
      else {
        for (i = 0; i < this.plots.length; i++) {
          this.plots[i].attributes["PlotStatus"] = status;
        }
      }
    }
    RestoreCursor();
  },
  init: function (graphObj) {
    // untested support for ctrl+V
    var graphController = {
      supportsCommand: function (cmd) {
        return (cmd === "cmd_paste");
      },
      isCommandEnabled: function (cmd) {
        if (cmd === "cmd_paste") {
          return true;
        }
        return false;
      },
      doCommand: function (cmd) {
        graphObj.addClipboardContents(graphObj.selectedIndex);
      },
      onEvent: function (evt) {}
    };
  },
  createGraphDOMElement: function (forComp, optplot) {
    // return a DOM <graph> node
    // if forComp, prepare the <graph> fragment to pass to the computation engine
    // Otherwise, create one suitable for putting into the document
    // An optional second argument is the number of the one plot to include for query
    var htmlns = "http://www.w3.org/1999/xhtml";
    var editorElement = msiGetActiveEditorElement();
//    var document = editorElement.contentDocument;
    var DOMGraph = document.createElementNS(htmlns, "graph");
    var DOMGs = document.createElementNS(htmlns, "graphSpec");
    var DOMFrame = document.createElementNS(htmlns, "msiframe");
    var DOMPw = document.createElementNS(htmlns, "plotwrapper");
    var DOMObj = document.createElementNS(htmlns, "object");
    var DOMCaption = document.createElementNS(htmlns,"imagecaption");
    DOMGraph.appendChild(DOMGs);
    DOMGraph.appendChild(DOMFrame);
    DOMFrame.appendChild(DOMPw);
    DOMFrame.appendChild(DOMCaption);
    DOMPw.appendChild(DOMObj);
    this.reviseGraphDOMElement(DOMGraph, editorElement);
    return DOMGraph;
  },
  reviseGraphDOMElement: function (DOMgraph, editorElement) {
    var htmlns = "http://www.w3.org/1999/xhtml";
    var editor = msiGetEditor(editorElement);
    var DOMGs = DOMgraph.getElementsByTagName("graphSpec")[0];
    var DOMPw = DOMgraph.getElementsByTagName("plotwrapper")[0];
    var DOMFrame = DOMgraph.getElementsByTagName("msiframe")[0];
    var DOMCaption = DOMFrame.getElementsByTagName("imagecaption")[0];
    var attr, value, alist, i, domPlots, plot, status, caption, captionloc, child;
    this.frame.reviseFrameDOMElement(DOMFrame, DOMPw, editorElement);

    // loop through graph attributes and insert them
    alist = this.graphAttributeList();
    for (i = 0; i < alist.length; i++) {
      attr = alist[i];
      value = this.getGraphAttribute(attr);
      if (value == null || value === "unspecified" || value === "undefined")
      {
        DOMGs.removeAttribute(attr);
      }
      else
      {
        DOMGs.setAttribute(attr, value);
      }
    }

    // if the optional plot number was specified, just include one plot
    // otherwise, for each plot, create a <plot> element
    // Clear out current plots first
    domPlots = DOMgraph.getElementsByTagName("plot");
    for (i = domPlots.length - 1; i >= 0; i--) {
      editor.deleteNode(domPlots[i]);
    }
    for (i = 0; i < this.plots.length; i++) {
      plot = this.plots[i];
      status = this.plots[i].attributes.PlotStatus;
      if (status === "ERROR") {
        this.errStr = "ERROR, Plot number " + i + " " + this.errStr;
      } else {
        DOMGs.appendChild(plot.createPlotDOMElement(document, false, plot));
      }
    }
    captionloc = this.getGraphAttribute("CaptionPlace");
    switch (captionloc) {
      case "labelabove": DOMFrame.setAttribute("captionloc","above"); break;
      case "labelbelow": DOMFrame.setAttribute("captionloc","below"); break;
      default: DOMFrame.removeAttribute("captionloc"); break;
    }
    caption = this.getGraphAttribute("Caption");
    if (caption && caption.length > 0) {
      if (DOMCaption) {
        while (DOMCaption.firstChild) {
          editor.deleteNode(DOMCaption.firstChild);
        }
      }
      else {
        DOMCaption = document.createElementNS(htmlns,"imagecaption");
        DOMFrame.appendChild(DOMCaption);
      }
      caption="<wrapper>"+caption+"</wrapper>";
      insertXML(editor, caption, DOMCaption, 0);
    }
  },
  extractGraphAttributes: function (DOMGraph) {
    var key, value, i, plot, plotno, DOMGs, DOMPlots, DOMFrame, DOMPw;
    DOMGs = DOMGraph.getElementsByTagName("graphSpec");
    if (DOMGs.length > 0) {
      DOMGs = DOMGs[0];
    }
    else return;
    for (i = 0; i < DOMGs.attributes.length; i++) {
      key = DOMGs.attributes[i].nodeName;
      value = DOMGs.attributes[i].nodeValue;
      this[key] = value;
    }
    DOMFrame = DOMGraph.getElementsByTagName("msiframe");
    if (DOMFrame.length > 0) {
      DOMFrame = DOMFrame[0];
    }
    DOMPw = DOMGraph.getElementsByTagName("plotwrapper");
    if (DOMPw.length > 0) {
      DOMPw = DOMPw[0];
    }
    DOMPlots = DOMGraph.getElementsByTagName("plot");
    for (i = 0; i < DOMPlots.length; i++) {
      plot = new Plot();
      plotno = this.addPlot(plot);
      plot.extractPlotAttributes(DOMPlots[i]);
      plot.attributes.PlotStatus = "Inited";
    }
    if (DOMFrame){
      this.frame.extractFrameAttributes(DOMFrame, DOMPw);
    }
    this.extractCaption(DOMGraph);
  },
  extractCaption: function (DOMGraph) {
    var DOMCaption, basenode, serialized;
    DOMCaption = DOMGraph.getElementsByTagName("imagecaption");
    if (DOMCaption.length > 0) {
      DOMCaption = DOMCaption[0];
    }
    else {
      return;
    }
    basenode = DOMCaption.getElementsByTagName("imagecaption");
    if (basenode.length > 0) {
      basenode = basenode[0];
    }
    else {
      return;
    }
    serialized = this.ser.serializeToString(basenode);
    this.setGraphAttribute("Caption", serialized);
  },
  getDefaultValue: function (key) {
    // get defaults from preference system, or if not there, from hardcoded list
    var prefs = Components.classes["@mozilla.org/preferences-service;1"].getService(Components.interfaces.nsIPrefBranch);
    var keyname = "swp.graph." + key;
    var value;
    if (prefs.getPrefType(keyname) === prefs.PREF_STRING) {
      try {
        value = prefs.getCharPref(keyname);
      }
      catch (e) {
        msidump("Preference Error", "Can't find preference for " + keyname + "\n");
        value = "";
      }
    }
    else if (prefs.getPrefType(keyname) === prefs.PREF_BOOL) {
      value = prefs.getBoolPref(keyname);
    }
    return value;
  },
  getGraphAttribute: function (name) {
    return (this[name]);
  },
  getValue: function (key) {
    var value = this.getGraphAttribute(key);
    if ((value != null) && (value !== "")) {
      return value;
    }
    return (this.getDefaultValue(key));
  },
  graphAttributeList: function () {
    return (this.GRAPHATTRIBUTES.concat(this.COMPATTRIBUTES));
  },
  graphCompAttributeList: function () {
    var NA = this.COMPATTRIBUTES;
    var dim = this.getGraphAttribute("Dimension");
    if (dim === "2") {
      NA = attributeArrayRemove(NA, "ZAxisLabel");
      NA = attributeArrayRemove(NA, "OrientationTiltTurn");
    }
    // TubeRadius?
    return NA;
  },
  serializeGraph: function (optionalplot) {
    var str;
    try {
      str = this.ser.serializeToString(this.createGraphDOMElement(true, optionalplot));
      // strip off the temporary namespace headers ...
      str = str.replace(/<[a-zA-Z0-9]{2}:/g, "<");
      str = str.replace(/<\/[a-zA-Z0-9]{2}:/g, "<\/");
    }
    catch (e) {
      dump("SMR GraphSerialize exception caught\n");
    }
    return str;
  },
  setGraphAttribute: function (name, value) {
    this[name] = value;
  },
  recomputeVCamImage: function (editorElement) {
    var filename = this.getGraphAttribute("ImageFile"); // the old name
    var file, match, filetype, newfilename, longnewfilename;
    match = /[a-zA-Z0-9]+\.(xv[cz]$)/.exec(filename);
    if (match.length > 0)
    {
      filetype = match[1];
    }
    newfilename = "plots/" + createUniqueFileName("plot", filetype);
    longnewfilename = makeRelPathAbsolute(newfilename, editorElement);
    this.setGraphAttribute("ImageFile", longnewfilename);
    // the filename that computeGraph uses is the one in the graph structure
    this.computeGraph(editorElement);
    // if the new file exists, delete the old one
    file = Components.classes["@mozilla.org/file/local;1"].
                         createInstance(Components.interfaces.nsILocalFile);
    file.initWithPath( makeRelPathAbsolute(filename, editorElement)) ;
    if (file.exists())
    {
      try {
        file.remove(false);
      }
      catch (e)
      {
      }
    }
    this.setGraphAttribute("ImageFile", newfilename);
    //  reset ImageFile property to relative path
  },
  scheduleNewPlotFromText: function (domgraph, expression, editorElement) {
    var intervalId;
    var __domgraph = domgraph;
    var __editorElement = editorElement;
    var __expression = expression;
    intervalId = setInterval(function () {
      newPlotFromText(__domgraph, __expression, __editorElement);
      clearInterval(intervalId);
    }, 200);
  },
  provideDragEnterHandler: function (editorElement, domGraph) {
    var thisGraph = this;
    var os = getOS(window);
    var __editorElement = editorElement;
    var __domGraph = domGraph;
    return function () {
      if (os === "osx") {
        var dropData, str;
        var dragService = Components.classes["@mozilla.org/widget/dragservice;1"].getService();
        dragService = dragService.QueryInterface(Components.interfaces.nsIDragService);
        netscape.security.PrivilegeManager.enablePrivilege('UniversalXPConnect');
        dropData = DNDUtils.getData("text/html", 0);
        str = dropData.QueryInterface(Components.interfaces.nsISupportsString);
        //      dragService.endDragSession(true);
        scheduleNewPlotFromText(__domGraph, str.data, __editorElement);
      }
      return 1;
    };
  },
  provideDropHandler: function (editorElement, domGraph) {
    var __domGraph = domGraph;
    var __editorElement = editorElement;
    return function () {
      var dropData, str;
      var dragService = Components.classes["@mozilla.org/widget/dragservice;1"].getService();
      dragService = dragService.QueryInterface(Components.interfaces.nsIDragService);
      netscape.security.PrivilegeManager.enablePrivilege('UniversalXPConnect');
      dropData = DNDUtils.getData("text/html", 0);
      str = dropData.QueryInterface(Components.interfaces.nsISupportsString);
      scheduleNewPlotFromText(__domGraph, str.data, __editorElement);
      //    dragService.endDragSession(true);
      return 1;
    };
  }
};

// ************ Plot section; a graph can have many plots ******

function Plot() {
  var attr, i;
  this.element = {};
  this.attributes = {};
  this.modFlag = {};
  this.parent = null;
  for (i = 0; i < this.PLOTATTRIBUTES.length; i++) {
    attr = this.PLOTATTRIBUTES[i];
    this.attributes[attr] = this.getDefaultPlotValue(attr);
    this.modFlag[attr] = false;
  }
  for (i = 0; i < this.PLOTELEMENTS.length; i++) {
    attr = this.PLOTELEMENTS[i];
    this.element[attr] = this.getDefaultPlotValue(attr);
    this.modFlag[attr] = false;
  }
}

Plot.prototype = {
  constructor: Plot,
  PLOTATTRIBUTES: ["PlotStatus", "PlotType",
                           "LineStyle", "PointStyle", "LineThickness", "LineColor",
                           "DiscAdjust", "DirectionalShading", "BaseColor", "SecondaryColor",
                           "PointSymbol", "SurfaceStyle", "IncludePoints",
                           "SurfaceMesh", "CameraLocationX", "CameraLocationY",
                           "CameraLocationZ", "FontFamily", "IncludeLines",
                           "AISubIntervals", "AIMethod", "AIInfo", "FillPattern",
                           "Animate", "AnimateStart", "AnimateEnd", "AnimateFPS",
                           "AnimateVisBefore", "AnimateVisAfter",
                           "ConfHorizontalPts", "ConfVerticalPts"],
  PLOTELEMENTS: ["Expression", "XMax", "XMin", "YMax", "YMin", "ZMax", "ZMin",
                            "XVar", "YVar", "ZVar", "XPts", "YPts", "ZPts", "TubeRadius"],
  // Plot elements are all MathML expressions.
  isModified: function (x) {
    return (this.modFlag[x]);
  },
  setModified: function (x) {
    this.modFlag[x] = true;
  },
  createPlotDOMElement: function (doc, forComp) {
     // return a DOM <plot> node. Unless forComp, only include non-default attributes and elements
     // do the plot attributes as DOM attributes of <plot>
    var status = this.PlotStatus;
    var attr;
    if (status != "Deleted") {
      var DOMPlot = doc.createElement("plot");
      var attrs = (forComp) ? this.plotCompAttributeList() : this.plotAttributeList();
      for (var i=0; i<attrs.length; i++) {
        attr = attrs[i];
  //      if (this.isModified[attr] || forComp) {
          var value = this.attributes[attr];
          if (value && (value !== "") && (value !== "unspecified"))
             DOMPlot.setAttribute (attr, value);
  //      }
      }
      // do the plot elements as document fragment children of <plot>
      attrs = (forComp) ? this.plotCompElementList() : this.plotElementList();
      for (i=0; i<attrs.length; i++) {
        attr = attrs[i];
        if (forComp || (this.element[attr])) {
          var DOMEnode = doc.createElement(attr);
          var textval = this.element[attr];
          if ((textval !=="") && (textval !== "unspecified")) {
            var tNode = (new DOMParser()).parseFromString (textval, "text/xml");
            DOMEnode.appendChild (tNode.documentElement);
            DOMPlot.appendChild (DOMEnode);
          }
        }
      }
      return DOMPlot;
    }
    return null;
  },
  revisePlotDOMElement: function (doc, forComp, domPlot) {
    // return a DOM <plot> node. Unless forComp, only include non-default attributes and elements
    // do the plot attributes as DOM attributes of <plot>
    var status = this.PlotStatus;
    var attr;
    if (status != "Deleted") {
      var DOMPlot = doc.createElement("plot");
      var attrs = (forComp) ? this.plotCompAttributeList() : this.plotAttributeList();
      for (var i = 0; i < attrs.length; i++) {
        attr = attrs[i];
        //      if (this.isModified[attr] || forComp) {
        var value = this.attributes[attr];
        if (value && (value !== "") && (value !== "unspecified")) DOMPlot.setAttribute(attr, value);
        //      }
      }
      // do the plot elements as document fragment children of <plot>
      attrs = (forComp) ? this.plotCompElementList() : this.plotElementList();
      for (i = 0; i < attrs.length; i++) {
        attr = attrs[i];
        if (forComp || (this.element[attr])) {
          var DOMEnode = doc.createElement(attr);
          var textval = this.element[attr];
          if ((textval !== "") && (textval !== "unspecified")) {
            var tNode = (new DOMParser()).parseFromString(textval, "text/xml");
            DOMEnode.appendChild(tNode.documentElement);
            DOMPlot.appendChild(DOMEnode);
          }
        }
      }
      return (DOMPlot);
    }
    return (NULL);
  },
  extractPlotAttributes: function (DOMPlot) {
    // get values from a DOM <plot> node
    var key;
    var value;
    var attr;
    var j;
    for (j = 0; j < DOMPlot.attributes.length; j++) {
      attr = DOMPlot.attributes[j];
      key = attr.nodeName;
      value = attr.nodeValue;
      this.attributes[key] = value;
    }

    // the children of <plot> are in general <plotelement> <mathml...> </plotelement>
    // We should get an ELEMENT_NODE for the <plotelement>, and the localName is the attribute name.
    // The data is either text or mathml. For text, grab the text value, which must be in the nodeValue
    // of the first child. For DOM fragment, store serialization of the fragment.
    //var children = DOMPlot.childNodes;
    var child = DOMPlot.firstChild;
    while (child) {
      key = child.localName;
      if (child.nodeType === Node.ELEMENT_NODE) {
        var mathnode = child.getElementsByTagName("math")[0];
        var serialized = this.parent.ser.serializeToString(mathnode);
        this.element[key] = serialized;
      }
      child = child.nextSibling;
    }
  },
  getPlotValue: function (key) {
    // look up the value key. If not found, look up the default value.
    var value = this[key];
    if ((value !== null) && (value !== "")) {
      return value;
    }
    var ptype = this.PlotType;
    if ((key === "XPts") || (key === "YPts") || (key === "ZPts")) {
      var dim = this.parent.getValue("Dimension");
      if (dim === "2") {
        switch (ptype) {
        case "implicit":
          value = GetNumAsMathML(20);
          break;
        case "gradient":
        case "vectorField":
          value = GetNumAsMathML(10);
          break;
        case "inequality":
          value = GetNumAsMathML(80);
          break;
        case "conformal":
          value = GetNumAsMathML(20);
          break;
        default:
          value = GetNumAsMathML(400);
          break;
        }
      } else {
        switch (ptype) {
        case "parametric":
        case "curve":
          value = GetNumAsMathML(80);
          break;
        case "implicit":
          value = GetNumAsMathML(6);
          break;
        case "gradient":
        case "vectorField":
          value = GetNumAsMathML(6);
          break;
        case "tube":
          value = GetNumAsMathML(40);
          break;
        case "approximateIntegral":
          value = GetNumAsMathML(400);
          break;
        default:
          value = GetNumAsMathML(20);
          break;
        }
      }
      return value;
    }
    return (plotGetDefaultPlotValue(key));
  },
  getDefaultPlotValue: function (key) {
    // get defaults from preference system, or if not there, from hardcoded list
    var math = '<math xmlns="http://www.w3.org/1998/Math/MathML">';
    var value;
    var prefs = Components.classes["@mozilla.org/preferences-service;1"].getService(Components.interfaces.nsIPrefBranch);
    var keyname = "swp.plot." + key;
    if (prefs.getPrefType(keyname) === prefs.PREF_STRING) {
      value = prefs.getCharPref(keyname);
    }
    else {
      switch (key) {
      case 'PlotStatus':
        value = "New"; // this should never be moved from here
        break;
      case 'ConfHorizontalPts':
        value = "15";
        break;
      case 'ConfVerticalPts':
        value = "15";
        break;
        //      default:
        //        value = math + "<mrow><mi tempinput=\"true\">()</mi></mrow></math>";
      case 'Expression':
        value = "<math xmlns='http://www.w3.org/1998/Math/MathML'><mi tempinput='true'></mi></math>";
        break;
      default:
        value = "";
      }
    }
    return value;
  },
  plotAttributeList: function () {
    return (this.PLOTATTRIBUTES);
  },
  plotElementList: function () {
    return (this.PLOTELEMENTS);
  },
  plotCompAttributeList: function () {
    var NA;
    var dim = this.parent["Dimension"];
    var ptype = this.attributes["PlotType"];
    var animate = this.attributes["Animate"];
    NA = attributeArrayRemove(this.PLOTATTRIBUTES, "PlotStatus");

    if (ptype != "approximateIntegral") {
      NA = attributeArrayRemove(NA, "AISubIntervals");
      NA = attributeArrayRemove(NA, "AIMethod");
      NA = attributeArrayRemove(NA, "AIInfo");
    }

    if ((ptype !== "approximateIntegral") && (ptype !== "inequality")) {
      NA = attributeArrayRemove(NA, "FillPattern");
    }

    if (ptype !== "conformal") {
      NA = attributeArrayRemove(NA, "ConfHorizontalPts");
      NA = attributeArrayRemove(NA, "ConfVerticalPts");
    }

    if (dim === 2) {
      NA = attributeArrayRemove(NA, "DirectionalShading");
      NA = attributeArrayRemove(NA, "SurfaceStyle");
      NA = attributeArrayRemove(NA, "SurfaceMesh");
      NA = attributeArrayRemove(NA, "CameraLocationX");
      NA = attributeArrayRemove(NA, "CameraLocationY");
      NA = attributeArrayRemove(NA, "CameraLocationZ");
    } else {
      NA = attributeArrayRemove(NA, "PointStyle");
      NA = attributeArrayRemove(NA, "AxisScale");
    }

    if ((ptype != "rectangular") && (ptype != "parametric") && (ptype != "implict")) {
      NA = attributeArrayRemove(NA, "DiscAdjust");
    }

    if (animate != "true") {
      NA = attributeArrayRemove(NA, "AnimateStart");
      NA = attributeArrayRemove(NA, "AnimateEnd");
      NA = attributeArrayRemove(NA, "AnimateFPS");
      NA = attributeArrayRemove(NA, "AnimateVisBefore");
      NA = attributeArrayRemove(NA, "AnimateVisAfter");
    }

    return NA;
  },
  plotCompElementList: function () {
    var dim = this.parent["Dimension"];
    var animate = this.attributes["Animate"];
    var ptype = this.attributes["PlotType"];
    var NA = this.PLOTELEMENTS;

    if (ptype != "tube") {
      NA = attributeArrayRemove(NA, "TubeRadius");
    }

    var nvars = plotVarsNeeded(dim, ptype, animate);

    if (nvars < 2) {
      NA = attributeArrayRemove(NA, "YPts");
      NA = attributeArrayRemove(NA, "YMax");
      NA = attributeArrayRemove(NA, "YMin");
    }

    if (nvars < 3) {
      NA = attributeArrayRemove(NA, "ZPts");
      NA = attributeArrayRemove(NA, "ZMax");
      NA = attributeArrayRemove(NA, "ZMin");
    }

    return NA;
  },
  getPlotAttribute: function (name) {
    return (this[name]);
  },
  setPlotAttribute: function (name, value) {
    this[name] = value;
    this.setModified(name);
  },
  computeQuery: function () {
    // call the compute engine to guess at graph attributes
    this.parent.computeQuery(this);
  }
};

// ************ Frame section; a graph has a single frame ******

function Frame(parent) {
  this.attributes = {};
  this.modFlag = {};
  this.parent = parent;
  var attr;
  for (var i = 0; i < this.FRAMEATTRIBUTES.length; i++) {
    attr = this.FRAMEATTRIBUTES[i];
    this.attributes[attr] = this.getDefaultFrameValue(attr);
    this.modFlag[attr] = false;
  }
}

Frame.prototype = {
  FRAMEATTRIBUTES: ["placement", "floatPlacement", "border",
          "HMargin", "VMargin", "padding", "BGColor", "borderColor",
          "placeLocation", "textalignment"],
  FRAMEDOMATTRIBUTES: ["units", "sidemargin", "topmargin", "pos", "placeLocation", "placement",
    "captionloc", "textalignment", "req"],
  WRAPPERATTRIBUTES: ["borderw", "padding", "imageheight", "imagewidth", "border-color", "background-color"],
  isModified: function (x) {
    return (this.modFlag[x]);
  },
  setModified: function (x) {
    this.modFlag[x] = true;
  },
  getDefaultFrameValue: function (key) {
    try {
      var value;
      var prefs = Components.classes["@mozilla.org/preferences-service;1"].getService(Components.interfaces.nsIPrefBranch);
      var keyname = "swp.graph." + key;
      if (prefs.getPrefType(keyname) === prefs.PREF_STRING) {
        value = prefs.getCharPref(keyname);
      }
      return value;
    }
    catch (e) {
      m = e.message;
      return "";
    }
  },
  frameAttributeList: function () {
    return (this.FRAMEATTRIBUTES.concat(this.FLOATLOCATIONS));
  },
  getFrameAttribute: function (name) {
    return (this.attributes[name]);
  },
  setFrameAttribute: function (name, value) {
    this.attributes[name] = value;
    this.setModified(name);
  },
  extractFrameAttributes: function (DOMFrame, DOMPw) {
    var i, att, attlist, len;
    var graph = this.parent;
    attlist = this.FRAMEDOMATTRIBUTES;
    len = attlist.length;
    for (i = 0; i < len; i++) {
      att = attlist[i];
      if (DOMFrame.hasAttribute(att)) {
        switch (att) {
          case "units":
            if (graph) {
              graph.setGraphAttribute("Units", DOMFrame.getAttribute(att));
            }
            break;
          case "sidemargin":
            this.setFrameAttribute("HMargin", DOMFrame.getAttribute(att));
            break;
          case "topmargin":
            this.setFrameAttribute("VMargin", DOMFrame.getAttribute(att));
            break;
          case "pos":
            this.setFrameAttribute("placement", DOMFrame.getAttribute(att));
            break;
          case "placeLocation":
            if (DOMFrame.getAttribute(att).length > 0) {
              this.setFrameAttribute("placeLocation", DOMFrame.getAttribute(att));
            }
            break;
          case "placement":
            if (DOMFrame.getAttribute("pos") === "float") {
              this.setFrameAttribute("floatPlacement", DOMFrame.getAttribute(att));
            }
            break;
          case "textalignment":
            if (DOMFrame.getAttribute(att)) {
              this.setFrameAttribute(att, DOMFrame.getAttribute(att));
            }
            break;
          case "captionloc":
            if (DOMFrame.hasAttribute("captionloc")) {
              graph.setGraphAttribute("CaptionPlace", "label"+DOMFrame.getAttribute("captionloc"));
            }
            else {
              graph.setGraphAttribute("CaptionPlace", "labelnone");
            }
            break;
          default: break;
        }
      }
    }
    attlist = this.WRAPPERATTRIBUTES;
    len = attlist.length;
    if (DOMPw){
    for (i = 0; i < len; i++) {
      att = attlist[i];
        if (DOMPw.hasAttribute(att)) {
          switch (att) {
            case "borderw":
              this.setFrameAttribute("border", DOMPw.getAttribute(att));
              break;
            case "padding":
              this.setFrameAttribute("padding", DOMPw.getAttribute(att));
              break;
//            case "imageheight":
//              graph.setGraphAttribute("Height", DOMPw.getAttribute(att));
//              break;
//            case "imagewidth":
//              this.setFrameAttribute("Width", DOMPw.getAttribute(att));
//              break;
            case "border-color":
              this.setFrameAttribute("borderColor", hexcolor(DOMPw.getAttribute(att)));
              break;
            case "background-color":
              this.setFrameAttribute("BGColor", hexcolor(DOMPw.getAttribute(att)));
              break;
            default: break;
          }
        }
      }
    }
  },
  reviseFrameDOMElement: function (DOMFrame, DOMPw, editorElement) {
    var editor = msiGetEditor(editorElement);
    var attributes, i, j, att, graph, units, height, width, heightinpx, widthinpx, placeLocation, floattts, fltatt, ch, captionlocation;
    var DOMObj = DOMPw.getElementsByTagName("object")[0];
    var frmStyle = "";
    var pwStyle = "";
    var objStyle = "";
    var unitHandler = new UnitHandler();
    var needsWrapfig = false;
    try {
      graph = this.parent;
      units = graph.getGraphAttribute("Units");
      unitHandler.initCurrentUnit(units);
      attributes = this.frameAttributeList().concat(this.parent.graphAttributeList());
      for (j = 0; j < attributes.length; j++) {
        att = attributes[j];
        switch (att) {
        case "HMargin":
          editor.setAttribute(DOMFrame, "sidemargin", this.getFrameAttribute(att));
          break;
        case "VMargin":
          editor.setAttribute(DOMFrame, "topmargin", this.getFrameAttribute(att));
          break;
        case "Height":
          height = Number(graph.getGraphAttribute(att));
          heightinpx = unitHandler.getValueAs(height, "px");
          editor.setAttribute(DOMPw, "height", height);
          pwStyle += "height: "+ heightinpx + "px; ";
          objStyle += "height: "+ heightinpx + "px; ";
          // dimensions of outer msiframe need to be adjusted for border and padding
          x = this.getFrameAttribute("border");
          if (x)
          {
            height += 2*(x-0);
          }
          x = this.getFrameAttribute("padding");
          if (x)
          {
            height += 2*(x-0);
          }
          editor.setAttribute(DOMFrame, "height", height);
          heightinpx = unitHandler.getValueAs(height, "px");
          frmStyle += "height: " + heightinpx + "px; ";
          break;
        case "Width":
          width = Number(graph.getGraphAttribute(att));
          widthinpx = unitHandler.getValueAs(width, "px");
          editor.setAttribute(DOMPw, "width", width);
          pwStyle += "width: "+ widthinpx + "px; ";
          objStyle += "width: "+ widthinpx + "px; ";
          x = this.getFrameAttribute("border");
          if (x) {
            width += 2*(x-0);
          }
          x = this.getFrameAttribute("padding");
          if (x) {
            width += 2*(x-0);
          }
          editor.setAttribute(DOMFrame, "width", width);
          widthinpx = unitHandler.getValueAs(width, "px");
          frmStyle += "width: " + widthinpx + "px; ";
          break;
        case "border":
          editor.setAttribute(DOMPw, "borderw", this.getFrameAttribute(att));
          pwStyle += "border: " + unitHandler.getValueStringAs(this.getFrameAttribute(att),"px") +
            " solid " + cssColor(this.getFrameAttribute("borderColor")) + "; ";
          break;
        case "padding":
          editor.setAttribute(DOMPw, "padding", this.getFrameAttribute(att));
          pwStyle += "padding: " + unitHandler.getValueStringAs(this.getFrameAttribute(att), "px") + "; ";
          break;
        case "Units":
          editor.setAttribute(DOMPw, "units", units);
          editor.setAttribute(DOMFrame, "units", units);
          break;
        case "BGColor":
          editor.setAttribute(DOMPw, "background-color", hexcolor(this.getFrameAttribute(att)));
          break;
        case "borderColor":
          editor.setAttribute(DOMPw, "border-color", hexcolor(this.getFrameAttribute(att)));
          break;
        case "placement":
          pos = this.getFrameAttribute(att);
          editor.setAttribute(DOMFrame, "pos", pos);
          if (pos === "float") {
            needsWrapfig = this.getFrameAttribute("floatPlacement") !== "none";
          }
          break;
        case "textalignment":
          editor.setAttribute(DOMFrame, att, this.getFrameAttribute(att));
          frmStyle += "text-align: "+ this.getFrameAttribute(att)+"; ";
          break;
        case "placeLocation":
          placeLocation = this.getFrameAttribute(att);
          editor.setAttribute(DOMFrame, "placeLocation", placeLocation);
          break;
        case "captionloc":
          captionlocation = this.getFrameAttribute(att);
          editor.setAttribute(DOMFrame, att, captionlocation);
          break;
        case "floatPlacement":
          if (this.getFrameAttribute("placement") === "float")
          {
            editor.setAttribute(DOMFrame, "placement", this.getFrameAttribute(att));
          }
          break;
        default:
          break;
        }
      }
      editor.setAttribute(DOMPw, "msi_resize", "true");

      // what about overhang?
      // Now we build the CSS style for the object and the plotwrapper
      var isfloat = this.getFrameAttribute("placement") === "float";
      var isdisplay = this.getFrameAttribute("placement") === "display";
      var lmargin = unitHandler.getValueStringAs(this.getFrameAttribute("HMargin"), "px");
      var rmargin = lmargin;
      var vmargin = unitHandler.getValueStringAs(this.getFrameAttribute("VMargin"), "px");
      if (isdisplay || (isfloat && this.getFrameAttribute("floatPlacement")==="full")) {
        frmStyle = "margin: " + vmargin + " auto; ";
      }
      else {
        if (isfloat) {
          switch (this.getFrameAttribute("floatPlacement")) {
            case "L":
            case "I":
            default:
              lmargin = "0px";
              break;
            case "R":
            case "O":
              rmargin = "0px";
              break;
          }
        }
        frmStyle += "margin: " + vmargin + " " + rmargin + " " + vmargin + " " + lmargin + "; ";
      }
      if (isfloat) {
        var floatParam = this.getFrameAttribute("floatPlacement");
        if (floatParam === "I" || floatParam === "L") {
          floatParam = "left";
        }
        else if (floatParam === "O" || floatParam === "R") {
          floatParam = "right";
        }
        else {
          floatParam = "none";
        }
        frmStyle += "float: " + floatParam + "; ";
      }
      var border = unitHandler.getValueStringAs(this.getFrameAttribute("border"), "px");
      // put the graph file in
      // resetting the data attribute seems to trigger loading a new VCam object. If it already exists, use
      // the load API
      if (false) { //DOMObj.load) {
        DOMObj.load(graph.getGraphAttribute("ImageFile"));
      }
      else {
        DOMObj.setAttribute("data", graph.getGraphAttribute("ImageFile"));
      }
      var filetype = graph.getDefaultValue("DefaultFileType");
      msidump("SMR file type is " + filetype + "\n");
      if (filetype === "xvz") {
        DOMObj.setAttribute("type", "application/x-mupad-graphics+gzip");
      } else if (filetype === "xvc") {
        DOMObj.setAttribute("type", "application/x-mupad-graphics+xml");
      }
      if (needsWrapfig) {
        editor.setAttribute(DOMFrame, "req", "wrapfig");
      }
      DOMObj.setAttribute("alt", "Generated Plot");
      DOMObj.setAttribute("msigraph", "true");
      DOMObj.setAttribute("data", graph.getGraphAttribute("ImageFile"));
      editor.setAttribute(DOMPw, "style", pwStyle);
      editor.setAttribute(DOMFrame, "style", frmStyle);
      editor.setAttribute(DOMObj, "style", objStyle);
      return (DOMPw);
    }
    catch (e) {
      msidump("reviseFrameDOMElement "+e.message);
    }
  }
};

function newPlotFromText(currentNode, expression, editorElement) {
  try {
    var editor = msiGetEditor(editorElement);
    var graph = new Graph();
    graph.extractGraphAttributes(currentNode);
    var plot = new Plot();
    var firstplot = graph.plots[0];
    plot.element["Expression"] = expression;
    plot.attributes["PlotType"] = firstplot.attributes["PlotType"];
    graph.addPlot(plot);
    graph.recomputeVCamImage(editorElement);
    graph.reviseGraphDOMElement(currentNode, editorElement);
    //    editor.replaceNode(domGraph, currentNode, currentNode.parentNode);
  }
  catch (e) {
    var m = e.message;
  }
}
function graphClickEvent(cmdstr, editorElement) {
  /**----------------------------------------------------------------------------------*/
  // Handle a mouse double click on a graph image in the document. This is bound in editor.js.
  // This function should be deprecated. It handled <img> elements inside <graph> elements
  try {
    if (!editorElement) editorElement = msiGetActiveEditorElement();
    var selection = msiGetEditor(editorElement).selection;
    if (selection) {
      var element = findtagparent(selection.focusNode, "graph");
      if (element) formatRecreateGraph(element, cmdstr, editorElement);
    }
  }
  catch (exc) {
    AlertWithTitle("Error in GraphOverlay.js", "Error in graphClickEvent: " + exc);
  }
}
function graphObjectClickEvent(cmdstr, element, editorElement) {
  /**----------------------------------------------------------------------------------*/
  // Handle a mouse double click on a <graph> <object> element. This is bound in msiEditor.js.
  dump("SMR In graphObjectClickEvent\n");
  try {
    if (element == null) {
      var selection = msiGetEditor(editorElement).selection;
      if (selection) element = selection.focusNode;
    }
    var graphelement = findtagparent(element, "graph");
    if (graphelement) {
      msidump("SMR found a <graph> element\n");
      // only open one dialog per graph element
//      var graph = new Graph();
//      graph.extractGraphAttributes(graphelement);
      // non-modal dialog, the return is immediate
      window.openDialog("chrome://prince/content/ComputeVcamSettings.xul", "vcamsettings", "chrome,close,titlebar,resizable, dependent", null, graphelement, null, element);
    }
  }
  catch (exc) {
    AlertWithTitle("Error in GraphOverlay.js", "Error in graphObjectClickEvent: " + exc);
  }
}
function addGraphElementToDocument(DOMGraphNode, siblingNode, editorElement) {
  /** ---------------------------------------------------------------------------------*/
  // insert the xml DOM fragment into the DOM
  if (siblingNode) {
    var idx;
    var parent = siblingNode.parentNode;
    if (!editorElement) editorElement = findEditorElementForDocument(element.ownerDocument);
    if (!editorElement) editorElement = msiGetActiveEditorElement();
    var editor = msiGetEditor(editorElement);
    for (idx = 0;
    (editor != null) && idx < parent.childNodes.length && siblingNode !== parent.childNodes[idx]; idx++);
    editor.insertNode(DOMGraphNode, parent, idx + 1);
  }
}
function createUniqueFileName(name, ext) {
  /**----------------------------------------------------------------------------------*/
  // Arguments: <name>: first characters of returned name
  //            <ext>:  file extension
  //            (out):  currdir -- an nsIFile for the current document directory
  //            (out):  docdir -- an nsIFile containing the document aux files
  // Returns <name><date>.<ext>
  //  where <path>, relative to the current document directory, is <docdir_files>/plots/
  //  There must be a current document directory
  //  <date> is the system time from the Date method.
  // now gin up a unique name
  var newdate = new Date();
  var fn = name + newdate.getTime() + "." + ext;
  return fn;
}
function formatRecreateGraph(DOMGraph, commandStr, editorElement) {
  /**----------------------------------------------------------------------------------*/
  // format and recreate a graph and replace the existing one
  // DOMGraph is the DOM graph element we are going to replace.
  // var extraArgsArray = new Array(DOMGraph);
  //  msiOpenModelessPropertiesDialog("chrome://prince/content/ComputeGraphSettings.xul",
  //                     "", "chrome,close,titlebar,dependent", editorElement, commandStr, DOMGraph, extraArgsArray);
  var dlgWindow = openDialog("chrome://prince/content/ComputeGraphSettings.xul", "Plot dialog", "chrome,close,titlebar,resizable, dependent", editorElement, commandStr, DOMGraph);
  return;
}
function nonmodalRecreateGraph(graph, DOMGraph, editorElement) {
  // this gets called only if the user clicked OK from ComputeGraphSettings.xul
  // In theory, graph is the graph object and DOMGraph is the DOM element
  // to be replaced. It is possible that DOMGraph has been removed from the
  // parent (i.e., deleted). No parent, no changes to the picture.
  try {

//    var parent = DOMGraph.parentNode;
//    var editor = msiGetEditor(editorElement);
    graph.reviseGraphDOMElement(DOMGraph, editorElement);
//    insertGraph(DOMGraph, graph, editorElement);
//    editor.deleteNode(DOMGraph);
  }
  catch (e) {
    dump("ERROR: Recreate Graph failed, line 715 in GraphOverlay.js\n");
  }
}
function makeRelPathAbsolute(relpath, editorElement) {
  var longfilename;
  var leaf;
  try {
    var documentfile;
    var docauxdirectory;
    var currdocdirectory;
    var urlstring = msiGetEditorURL(editorElement);
    var url = msiURIFromString(urlstring);
    documentfile = msiFileFromFileURL(url);

    currdocdirectory = documentfile.parent.clone();
    var pathParts = relpath.split("/");
    var i;
    for (i = 0; i < pathParts.length; i++) {
      currdocdirectory.append(pathParts[i]);
    }
    longfilename = currdocdirectory.path;
  } catch (e) {
    dump("Error: " + e + "\n");
  }
  return longfilename;
}
function insertGraph(siblingElement, graph, editorElement) {
  /**----------------------------------------------------------------------------------*/
  // compute a graph, create a <graph> element, insert it into DOM after siblingElement
  var filetype = graph.getDefaultValue("DefaultFileType");
  // May want to ensure file type is compatible with animated here
  //  var editorElement = null;
  if (!editorElement || editorElement == null) editorElement = findEditorElementForDocument(siblingElement.ownerDocument);
  var longfilename;
  var file;
  var leaf;
  var urlstring = msiGetEditorURL(editorElement);
  var url = msiURIFromString(urlstring);
  var documentfile = msiFileFromFileURL(url);
  var plotfile;
  var docauxdirectory;
  var currdocdirectory;
  try {
    leaf = createUniqueFileName("plot", filetype);
    plotfile = documentfile.clone();
    plotfile = plotfile.parent;
    plotfile.append("plots");
    if (!plotfile.exists()) plotfile.create(1, 0755);
    plotfile.append(leaf);
    longfilename = plotfile.path;
    graph.setGraphAttribute("ImageFile", longfilename);
    msiGetEditor(editorElement).setCaretAfterElement(siblingElement);
  } catch (e) {
    dump("Error: " + e + "\n");
  }
  //  dump("In insertGraph, about to computeGraph.\n");
  graph.computeGraph(editorElement, longfilename);
  graph.setGraphAttribute("ImageFile", "plots/" + leaf);
  var gDomElement = graph.createGraphDOMElement(false);

  addGraphElementToDocument(gDomElement, siblingElement, editorElement);
  var obj = gDomElement.getElementsByTagName("object")[0];
  doVCamPreInitialize(obj, graph);
  editorElement.focus();
}
function insertNewGraph(math, dimension, plottype, optionalAnimate, editorElement) {
  /**-----------------------------------------------------------------------------------------*/
  // Create the <graph> element and insert into the document following this math element
  // primary entry point
  if (!editorElement) editorElement = msiGetActiveEditorElement();
  var expr = runFixup(GetFixedMath(math));
  var graph = new Graph();
  var width = GetStringPref("swp.graph.HSize");
  var height = GetStringPref("swp.graph.VSize");
  var unit = GetStringPref("swp.graph.defaultUnits");
  graph.setGraphAttribute("Width", width);
  graph.setGraphAttribute("Height", height);
  graph.setGraphAttribute("Units", unit);


  var plot = new Plot();
  var plotnum = graph.addPlot(plot);
  plot.attributes["PlotStatus"] = "New";
  plot.element["Expression"] = expr;
  graph["Dimension"] = dimension;
  plot.attributes["PlotType"] = plottype;
  graph["plotnumber"] = plotnum;
  if ((arguments.length > 3) && (arguments[3] === true)) {
    plot.attributes["Animate"] = "true";
  }
  plot.computeQuery();
  insertGraph(math, graph, editorElement);
}
function recreateGraph(DOMGraph, editorElement) {
  /**----------------------------------------------------------------------------------*/
  // regenerate the graph  based on the contents of <graph>
  // DOMGraph is the DOM graph element we are going to replace.
  // called by the TestGraphScript() method in the debug menu.
  var graph = new Graph();
  graph.extractGraphAttributes(DOMGraph);
  var parent = DOMGraph.parentNode;
  insertGraph(DOMGraph, graph, editorElement);
  parent.removeChild(DOMGraph);
}
function wrapMath(thing) {
  /**----------------------------------------------------------------------------------*/
  // wrap this mathml fragment string inside a mathml <math> element
  return ("<math xmlns=\"http://www.w3.org/1998/Math/MathML\">" + thing + "</math>");
}
function wrapmi(thing) {
  // wrap this mathml fragment string inside a mathml <mi> element
  return ("<mi>" + thing + "</mi>");
}
function graphSaveVars(varList, plot) {
  if (varList[0] !== "") {
    plot.element["XVar"] = wrapMath(wrapmi(varList[0]));
  }
  if (varList[1] !== "") {
    plot.element["YVar"] = wrapMath(wrapmi(varList[1]));
  }
  if (varList[2] !== "") {
    plot.element["ZVar"] = wrapMath(wrapmi(varList[2]));
  }
}
function attributeArrayRemove(A, element) {
  //==========================================
  // return a list of attribute names that correspond to required or optional
  // data elements for the given type of graph. The only restrictions here are
  // based on the dimensions of the graph.
  // return a new array with all the elements of A preserved except element
  var NA1, NA2;
  var idx;
  idx = attributeArrayFind(A, element);
  if (idx === - 1) return A;
  NA1 = A.slice(0, idx);
  NA2 = A.slice(idx + 1);
  return (NA1.concat(NA2));
}
function attributeArrayFind(A, element) {
  // return the index of element in the array A, -1 if not found
  // ***** need to fix this 8/4/06 ****
  // get some funky error about A[idx] not having properties
  var idx;
  for (idx = 0; idx < A.length; idx++) {
    if (A[idx] === element) return idx;
  }
  return -1;
}
function plotVarsNeeded(dim, ptype, animate) {
  var nvars = 0;
  switch (ptype) {
  case "curve":
  case "explicitList":
  case "tube":
    nvars = 0; // gets set to 1 below
    break;
  case "rectangular":
  case "polar":
  case "parametric":
  case "approximateIntegral":
  case "spherical":
  case "cylindrical":
    nvars = 1;
    break;
  case "inequality":
  case "implicit":
  case "conformal":
  case "vectorField":
  case "gradient":
  case "ode":
    nvars = 2;
    break;
  default:
    alert("SMR ERROR in GraphOverlay line 932 unknown plot type " + ptype);
    break;
  }
  if (dim === "3") nvars++;
  if (animate === "true") nvars++;
  return (nvars);
}
function testQuery(domgraph) {
  var graph = new Graph();
  graph.extractGraphAttributes(domgraph);

  var expr = graph.plots[0].element["Expression"];
  var expr2 = runFixup(runFixup(expr));
  graph.plots[0].element["Expression"] = expr2;

  graph.computeQuery(graph.plots[0]);
}
function testQueryGraph(domgraph, editorElement) {
  var graph = new Graph();
  graph.extractGraphAttributes(domgraph);

  var expr = graph.plots[0].element["Expression"];
  var expr2 = runFixup(runFixup(expr));
  graph.plots[0].element["Expression"] = expr2;
  graph.computeQuery(plots[0]);

  var parent = domgraph.parentNode;
  insertGraph(domgraph, graph, editorElement);
  parent.removeChild(domgraph);
}
function parseQueryReturn(out, graph, plot) {
  //================================================================================
  // The engine has simply run eval(expression) and out is the mathml returned.
  // (1) identify the plot parameters by looking for <mi>s and assign them to
  //     the plot variables. Warn if there are problems
  // (2) Try to identify the type of plot
  var stack = [];
  var level = 0;
  var result;
  var variableList = [];
  var pt = plot.attributes["PlotType"];
  var dim = plot.parent["Dimension"];
  var animated = (plot.attributes["Animate"] === "true");
  var mathvars = false,
    commas = false;

  // (1) try to identify all of the variables
  //     Variables are indicated by <mi>, but these might also be mathnames of functions
  var count = 0;
  var index = 0;
  var start;
  while ((start = out.indexOf("<mi", index)) >= 0) {
    mathvars = true;
    start = out.indexOf(">", start);
    var stop = out.indexOf("</mi>", start) + 1; // find </mi>
    index = stop + 1;
    var v = out.slice(start + 1, stop - 1);
    if (nameNotIn(v, stack)) {
      stack.push(v);
    }
  }
  for (var i = 0; i < stack.length; i++) {
    if (varNotFun(stack[i])) {
      variableList.push(stack[i]);
    }
  }

  // check variables
  var varsNeeded = plotVarsNeeded(dim, pt, animated);

  // (2) Given a list of potential variables, match them to x,y,z, and t
  variableList = matchVarNames(variableList, animated);
  if (variableList) graphSaveVars(variableList, plot);

  // (3) identify explicitList and parametric types
  // graph.setPlotAttribute (PlotAttrName ("PlotType", plot_no), "explicitList");
  // Count the number of elements in the returned expression. If it's equal to the
  // dimension, set the plot type to parametric.
  var commalst = out.match(/<mo>,<\/mo>/g);
  if (commalst) {
    commas = true;
    var n = commalst.length + 1;
    // probably only want to do the following for "rectangular" plots
    if ((dim === n) && (pt != "vectorField") & (pt != "polar") && (pt != "cylindrical") && (pt != "spherical") && (pt != "tube")) {
      plot.attributes["PlotType"] = "parametric";
    }
  }

  // (4) try to identify explicit lists
  if ((!commas) && (!mathvars)) {
    if (out.indexOf("<mtable>") >= 0) {
      plot.attributes["PlotType"] = "explicitList";
    }
  }

  // (5) some special handling for polar, spherical, and cylindrical
  //     build an expression that looks like [r,t] for polar, [r,t,p] for spherical and cylindrical
  //     Allow constants. For animations, if there is only 1 var, it's the animation var.
  if ((pt === "polar") || (pt === "spherical") || (pt === "cylindrical")) {
    if (!commas) { // create [fn, xvar]
      if ((!mathvars) || ((animated) && (actualVarCount(variableList) === 1))) {
        // first arg is a constant, the rest are new
        var animvar = variableList[0];
        variableList[0] = newVar("1");
        plot.element["XVar"] = wrapMath(wrapmi(variableList[0]));
        variableList[1] = newVar("2");
        plot.element["YVar"] = wrapMath(wrapmi(variableList[1]));
        if (animated) {
          if (pt === "polar") {
            plot.element["YVar"] = wrapMath(wrapmi(animvar));
          } else {
            plot.element["ZVar"] = wrapMath(wrapmi(animvar));
          }
        }
      }
      //out = runFixup(createPolarExpr(out, variableList, pt));
      plot.element["Expression"] = out;
    }
  }

  // (6) add the ilk="enclosed-list" attribute to <mfenced>
  if (out) {
    if ((pt === "polar") || (pt === "spherical") || (pt === "cylindrical") || (pt === "parametric") || (pt === "gradient") || (pt === "vectorField")) {
      var hasilk = out.indexOf("ilk=", 0);
      if (hasilk === - 1) {
        var s = out.indexOf("<mfenced", 0);
        if (s > 0) {
          out = out.slice(0, s) + "<mfenced ilk=\"enclosed-list\" " + out.slice(s + 8);
        }
      }
      out = runFixup(out);
      plot.element["Expression"] = out;
    }
  }

}
function varNotFun(v) {
  // return true unless v is one of the function names
  // \u03c0 is the unicode for PI?
  var funNames = ["sin", "cos", "tan", "ln", "log", "exp", "PI", "\u03c0"];
  var ret = true;
  for (var i = 0; i < funNames.length; i++) {
    if (v === funNames[i]) {
      ret = false;
    }
  }
  return (ret);
}
function nameNotIn(vin, list) {
  // return true if vin is not in the list
  var ret = true;
  for (var i = 0; i < list.length; i++) {
    if (list[i] === vin) {
      ret = false;
    }
  }
  return ret;
}
function matchVarNames(variableList, animated) {
  // Given a list of variable names, assign them to the x, y, z, and animation
  // variables for a plot
  var newVarList = ["", "", "", ""];
  var animVar = "";
  var xvar = -1,
    yvar = -1,
    zvar = -1;

  // If variableList contains "x", "y", or "z", match them to position 0, 1,
  // or 2 respectively, but only if all of the predecessors are there.
  for (var i = 0; i < variableList.length; i++) {
    if ((variableList[i] === "x") || (variableList[i] === "X")) {
      xvar = i;
    } else if ((variableList[i] === "y") || (variableList[i] === "Y")) {
      yvar = i;
    } else if ((variableList[i] === "z") || (variableList[i] === "Z")) {
      zvar = i;
    }
  }
  var ptr = 0;
  if (xvar >= 0) {
    newVarList[ptr++] = variableList[xvar];
    variableList[xvar] = "";
    if (yvar >= 0) {
      newVarList[ptr++] = variableList[yvar];
      variableList[yvar] = "";
      if (zvar >= 0) {
        newVarList[ptr++] = variableList[zvar];
        variableList[zvar] = "";
      }
    }
  }

  // grab the animation variable ...
  if (animated) {
    for (i = 0; i < variableList.length; i++) {
      if ((variableList[i] === "t") || (variableList[i] === "T")) {
        animVar = variableList[i];
        variableList[i] = "";
      }
    }
  }
  // sort the remainder alphabetically
  variableList.sort();

  // assign remainder of variables in alphabetical order
  var oldptr, newptr;
  for (oldptr = 0;
  ((oldptr < variableList.length) && (variableList[oldptr] === "")); oldptr++);
  for (newptr = 0;
  ((newptr < newVarList.length) && (newVarList[newptr] !== "")); newptr++);
  while ((oldptr < variableList.length) && (newptr < newVarList.length)) {
    if (oldptr < variableList.length && variableList[oldptr] !== "") newVarList[newptr++] = variableList[oldptr++];
    for (;
    ((oldptr < variableList.length) && (variableList[oldptr] === "")); oldptr++);
    for (;
    ((newptr < newVarList.length) && (newVarList[newptr] !== "")); newptr++);
  }

  // insert animation var if it's there
  if ((animated) && (animVar !== "")) {
    newVarList[newptr] = animVar;
  }
  return newVarList;
}


function createPolarExpr(mathexp, variableList, plottype) {
  var newexp = stripMath(mathexp);
  newexp = "<mrow><mo>[</mo>" + newexp;
  newexp = newexp + "<mo>,</mo>" + wrapmi(variableList[0]);
  if (plottype != "polar") {
    newexp = newexp + "<mo>,</mo>" + wrapmi(variableList[1]);
  }
  newexp = newexp + "<mo>]</mo></mrow>";
  newexp = wrapMath(newexp);
  return newexp;
}
function newVar(x) {
  return "o" + x;
}

function stripMath(mathexp) {
  var start = mathexp.indexOf("<math", 0);
  start = mathexp.indexOf(">", start) + 1;
  var finish = mathexp.indexOf("</math", start);
  var newexp = mathexp.slice(start, finish);
  return newexp;
}

function actualVarCount(variableList) {
  var i = 0;
  for (var j = 0; j < variableList.length; j++)
  if (variableList[j] !== "") i++;
  return i;
}

function cssColor(hexColor) {
  var regex = /^[#A-F0-9]/;
  var R, G, B;
  if (regex.test(hexColor)) {
    if (hexColor.charAt(0) === "#") {
      hexColor = hexColor.slice(1);
    }
    R = parseInt(hexColor.slice(0,2), 16);
    G = parseInt(hexColor.slice(2,4), 16);
    B = parseInt(hexColor.slice(4,6), 16);
    return "rgb("+R+","+G+","+B+")";
  }
  return hexColor;
}

var plotObserver = {
  canHandleMultipleItems: function () {
    return true;
  },

  //  onDragStart: function (evt, transferData, action)
  //  {
  //    var tree = evt.currentTarget;
  //    var namecol = tree.columns.getNamedColumn('Name');
  //    var i = tree.currentIndex;
  //
  //    if (tree.view.isContainer(i)) return;
  //    var s = tree.view.getCellText( i,namecol);
  //    while (tree.view.getParentIndex(i) >= 0)
  //    {
  //      i = tree.view.getParentIndex(i);
  //      s = tree.view.getCellText(i,namecol)+ "/" + s;
  //    }
  //    // s is now the path of the clicked file relative to the fragment root.
  //    try
  //    {
  //      var request = Components.
  //                    classes["@mozilla.org/xmlextras/xmlhttprequest;1"].
  //                    createInstance();
  //      request.QueryInterface(Components.interfaces.nsIXMLHttpRequest);
  //      var urlstring = decodeURIComponent(tree.getAttribute("ref") + s);
  //      var path = msiPathFromFileURL( msiURIFromString(urlstring));
  //
  //      request.open("GET", urlstring, false);
  //      request.send(null);
  //
  //      var xmlDoc = request.responseXML;
  //      if (!xmlDoc && request.responseText)
  //        throw("fragment file exists but cannot be parsed as XML");
  //      if (xmlDoc)
  //      {
  //        var dataString="";
  //        var contextString="";
  //        var infoString="";
  //        var node;
  //        var nodelist;
  //        nodelist = xmlDoc.getElementsByTagName("data");
  //        if (nodelist.length > 0) node = nodelist.item(0);
  //        if (node)
  //        {
  //          dataString = decodeURIComponent(node.textContent);
  //        }
  //        if (dataString.length == 0) return;  // no point in going on in this case
  //        node = null;
  //        nodelist = xmlDoc.getElementsByTagName("context");
  //        if (nodelist.length > 0) node = nodelist.item(0);
  //        if (node)
  //        {
  //          contextString =  decodeURIComponent(node.textContent);
  //        }
  //        node = null;
  //        nodelist = xmlDoc.getElementsByTagName("info");
  //        if (nodelist.length > 0) node = nodelist.item(0);
  //        if (node)
  //        {
  //          infoString =  decodeURIComponent(node.textContent);
  //        }
  //        transferData.data = new TransferData();
  //        transferData.data.addDataForFlavour("privatefragmentfile", path);
  //        transferData.data.addDataForFlavour("text/html",dataString);
  //        transferData.data.addDataForFlavour("text/_moz_htmlcontext",contextString);
  //        transferData.data.addDataForFlavour("text/_moz_htmlinfo",infoString);
  //      }
  //    }
  //    catch(e) {
  //
  //      dump("Error: "+e.message+"\n");
  //
  //    }
  //    focusOnEditor();
  //  },
  canDrop: function (evt, session) {
    return true;
  },

  onDrop: function (evt, dropData, session) {
    if (session.isDataFlavorSupported("text/html")) {
      try {
        var trans = Components.classes["@mozilla.org/widget/transferable;1"].
        createInstance(Components.interfaces.nsITransferable);
        var value;
        if (!trans) return;
        var flavour = "text/html";
        trans.addDataFlavor(flavour);
        session.getData(trans, 0);
        var str = {};
        var strLength = {};
        try {
          trans.getTransferData(flavour, str, strLength);
          if (str) str = str.value.QueryInterface(Components.interfaces.nsISupportsString);
          if (str) value = str.data.substring(0, strLength.value / 2);
        }
        catch (e) {
          dump("  " + flavour + " not supported\n\n");
          value = "";
        }
        trans.removeDataFlavor(flavour);
      }
      catch (e) {
        msidump(e.message);
      }
    }
  },

  onDragOver: function (evt, flavour, session) {
    supported = session.isDataFlavorSupported("text/html");
    // we should look for math in the fragment
    if (supported) session.canDrop = true;
  },

  getSupportedFlavours: function () {
    var flavours = new FlavourSet();
    flavours.appendFlavour("text/html");
    return flavours;
  }
};

#endif

