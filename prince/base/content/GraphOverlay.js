
// Copyright (c) 2004-2013 MacFKichan Software, Inc. All rights reserved
/* jshint ignore:start */
// For JSHint documentation see  http://www.jshint.com/docs/
#include productname.inc
#ifdef PROD_COMPUTE
/* jshint ignore:end */
Components.utils.import("resource://app/modules/os.jsm");
//----------------------------------------------------------------------------------


// ************ Graph section ******

function Graph() {
  // these arrays enumerate the data for a graph. Expression is in mathml
  // When adding to this list, you must also add to the MathServiceRequest in the compute engine
  // Compute/iCmpIDs.h and Compute/MRequest.cpp::NameToPID()
  // PlotStatus: UI use only. New/Inited/Deleted.
  var i, list, length;
  this.plots = []; //an array of plots, see below
  this.modFlag = {}; //a Boolean for each attribute in GRAPHATTRIBUTES
  this.plotLabels = [];  //an array of plot labels
  this.userSetAttrs = [];  //a list of attributes by name which are not using default values
  this.errStr = "";
  this.currentDisplayedPlot = -1;
  list = this.graphAttributeList();
  length = list.length;
  for (i = 0; i < length; i++) {
    if (this.mapAttToPrefkey[list[i]])
      this[list[i]] = this.getDefaultValue(this.mapAttToPrefkey[list[i]]);
  }
  this.frame = new Frame(this);
}

Graph.prototype = {
  //  BBM: Some of the following attributes below really belong on the msiframe object or the VCam object
  // Namely: ImageFile====obj.data, Width, Height, Units, BGColor. Some of these probably should appear *only*
  // elsewhere: width, height, units
  COMPATTRIBUTES: ["ImageFile", "XAxisLabel", "YAxisLabel", "ZAxisLabel", "Width", "Height",
                              "Units", "AxesType", "EqualScaling", "EnableTicks", "XTickCount",
                              "YTickCount", "ZTickCount", "AxesTips", "GridLines", "BGColor", "Dimension",
                              "AxisScale", "CameraLocationX", "CameraLocationY", "CameraLocationZ",
                              "FocalPointX", "FocalPointY", "FocalPointZ", "UpVectorX", "UpVectorY",
                              "UpVectorZ", "ViewingAngle", "OrthogonalProjection", "KeepUp",
                              "OrientationTiltTurn", "ViewingBoxXMin", "ViewingBoxXMax",
                              "ViewingBoxYMin", "ViewingBoxYMax", "ViewingBoxZMin", "ViewingBoxZMax",
                              "AnimateStart", "AnimateEnd", "AnimateFPS", "AnimateCurrTime",
                              "AxisFontFamily", "AxisFontSize", "AxisFontColor", "AxisFontItalic", "AxisFontBold",
                              "TicksFontFamily", "TicksFontSize", "TicksFontColor", "TicksFontItalic", "TicksFontBold"],
  GRAPHATTRIBUTES: ["Key", "Name", "CaptionPlace"],
  omitAttributeIfDefaultList : //["XAxisLabel", "YAxisLabel", "ZAxisLabel",
                               ["CameraLocationX", "CameraLocationY", "CameraLocationZ",
                                "FocalPointX", "FocalPointY", "FocalPointZ",
                                "UpVectorX", "UpVectorY","UpVectorZ", "ViewingAngle", "OrientationTiltTurn",
                                "ViewingBoxXMin", "ViewingBoxXMax", "ViewingBoxYMin", "ViewingBoxYMax",
                                "ViewingBoxZMin", "ViewingBoxZMax", "AxisFontFamily", "AxisFontSize",
                                "AxisFontColor", "AxisFontItalic", "AxisFontBold", "TicksFontFamily",
                                "TicksFontSize", "TicksFontColor", "TicksFontItalic", "TicksFontBold"],
  plotVariablePropertyNames : ["XVar","YVar", "ZVar", "AnimVar"],
  graphAxesPropertyNames : ["XAxisLabel","YAxisLabel","ZAxisLabel"],


  constructor: Graph,
  ser: new XMLSerializer(),
  mapAttToPrefkey: {
    "ImageFile" : null,
    "XAxisLabel" : "xaxislabel",
    "YAxisLabel" : "yaxislabel",
    "ZAxisLabel" : "zaxislabel",
    "Width" : "hsize",
    "Height" : "vsize",
    "Units" : "units",
    "AxesType" : "AxisType",
    "EqualScaling" : "AxisEqualScaling",
    "EnableTicks" : null,
    "XTickCount" : "xtickcount",
    "YTickCount" : "ytickcount",
    "ZTickCount" : "ztickcount",
    "AxesTips" : "AxisTips",
    "GridLines" : "gridlines",
    "BGColor" : "bgcolor",
    "Dimension" : null,
    "AxisScale" : "AxisScaling",
    "CameraLocationX" : null,
    "CameraLocationY" : null,
    "CameraLocationZ" : null,
    "FocalPointX" : null,
    "FocalPointY" : null,
    "FocalPointZ" : null,
    "UpVectorX" : null,
    "UpVectorY" : null,
    "UpVectorZ" : null,
    "ViewingAngle" : null,
    "OrthogonalProjection" : "orthogproj",
    "KeepUp" : "keepupvector",
    "OrientationTiltTurn" : null,
    "ViewingBoxXMin" : null,
    "ViewingBoxXMax" : null,
    "ViewingBoxYMin" : null,
    "ViewingBoxYMax" : null,
    "ViewingBoxZMin" : null,
    "ViewingBoxZMax" : null,
    "AnimateStart" : null,
    "AnimateEnd" : null,
    "AnimateFPS" : null,
    "AnimateCurrTime" : null,
    "AxisFontFamily" : "AxisFontFamily",
    "AxisFontSize" : "AxisFontSize",
    "AxisFontColor" : "AxisFontColor",
    "AxisFontItalic" : "AxisFontItalic",
    "AxisFontBold" : "AxisFontBold",
    "TicksFontFamily" : "ticksfontfamily",
    "TicksFontSize" : "ticksfontsize",
    "TicksFontColor" : "ticksfontcolor",
    "TicksFontItalic" : "ticksfontitalic",
    "TicksFontBold" : "ticksfontbold",
  },
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
  addNewPlotLabel : function() {
    var plotLabel = new PlotLabel(this.getDimension());
    return this.addPlotLabel(plotLabel);
  },
  addPlotLabel : function(plotLabel) {
    this.plotLabels.push(plotLabel);
    plotLabel.parent = this;
    return (this.plotLabels.length - 1);
  },
  deletePlotLabel : function(labelNum) {
    if (labelNum >= 0 && labelNum < this.plotLabels.length) {
      this.plotLabels.splice(labelNum, 1);
    }
    return this.plotLabels.length;
  },
  getNumPlots: function () {
    return (this.plots.length);
  },
  getNumActivePlots : function() {
    var nCount = 0;
    for (var ii = 0; ii < this.plots.length; ++ii)
    {
      if (this.plots[ii].getPlotAttribute("PlotStatus") !== "Deleted") {
        ++nCount;
      }
    }
    return nCount;
  },
  plotFailed: function() {
    return (this.errStr && (this.errStr.length > 0));
  },
  isModified: function (x) {
    return (this.modFlag[x]);
  },
  setModified: function (x) {
    this.modFlag[x] = true;
  },
  isUserSet: function(attr) {
    return (this.userSetAttrs.indexOf(attr) >= 0);
  },
  markUserSet: function(attr, bUserSet) {
    var nIndex = this.userSetAttrs.indexOf(attr);
    if ((nIndex >= 0) && !bUserSet) {
      this.userSetAttrs.splice(nIndex,1);
    }
    else if ((nIndex < 0) && bUserSet) {
      this.userSetAttrs.push(attr);
    }
  },
  cameraValuesUserSet : function()
  {
    var retval = false;
    var camVals = ["CameraLocationX", "CameraLocationY", "CameraLocationZ",
                              "FocalPointX", "FocalPointY", "FocalPointZ", "UpVectorX", "UpVectorY",
                              "UpVectorZ", "ViewingAngle", "OrthogonalProjection", "KeepUp"];
    for (var ii = 0; !retval && (ii < camVals.length); ++ii)
    {
      retval = this.isUserSet(camVals[ii]);
    }
    return retval;
  },
  markCameraValuesUserSet : function(bSet)
  {
    var camVals = ["CameraLocationX", "CameraLocationY", "CameraLocationZ",
                              "FocalPointX", "FocalPointY", "FocalPointZ", "UpVectorX", "UpVectorY",
                              "UpVectorZ", "ViewingAngle", "OrthogonalProjection", "KeepUp"];
    for (var ii = 0; ii < camVals.length; ++ii)
    {
      this.markUserSet(camVals[ii], bSet);
    }
  },
  viewRangesUserSet : function()
  {
    var retval = false;
    var viewVals = ["ViewingBoxXMin", "ViewingBoxXMax", "ViewingBoxYMin", "ViewingBoxYMax",
                    "ViewingBoxZMin", "ViewingBoxZMax"];
    for (var ii = 0; !retval && (ii < viewVals.length); ++ii)
    {
      retval = this.isUserSet(viewVals[ii]);
    }
    return retval;
  },
  markViewRangesUserSet : function(bSet)
  {
    var viewVals = ["ViewingBoxXMin", "ViewingBoxXMax", "ViewingBoxYMin", "ViewingBoxYMax",
                    "ViewingBoxZMin", "ViewingBoxZMax"];
    for (var ii = 0; ii < viewVals.length; ++ii)
    {
      this.markUserSet(viewVals[ii], bSet);
    }
  },
  plotAttrIsUserSet: function(plotno, attr) {
    if (plotno < this.plots.length)
    {
      return this.plots[plotno].isUserSet(attr);
    }
    return true; //returning false would suggest there's a value to be used
  },
  markPlotAttrUserSet: function(plotno, attr, bUserSet) {
    if (plotno < this.plots.length)
    {
      this.plots[plotno].markUserSet(attr,bUserSet);
    }
  },
  varNameFromFoundVariable : function(plotno, whichVar)
  {
    if (plotno < this.plots.length) {
      return this.plots[plotno].varNameFromFoundVariable(whichVar);
    }
    return false;
  },
  computeGraph: function (editorElement, filename) {
    // call the compute engine to create an image
    ComputeCursor(editorElement);
    var oldError, newError;
    var str = this.serializeGraph();
    if (this.errStr === "") {
      try {
        oldError = GetCurrentEngine().getEngineErrors();  //to compare below
        var topWin = msiGetTopLevelWindow();
        topWin.msiComputeLogger.Sent4("plotfuncCmd", filename, str, "");
        var out = GetCurrentEngine().plotfuncCmd(str);
        msiComputeLogger.Received(out);
      }
      catch (e) {
//        alert("Computation Error", "Compute Graph: " + GetCurrentEngine().getEngineErrors());
        msiComputeLogger.Exception(e);
      }
      newError = GetCurrentEngine().getEngineErrors();
      if (newError && (newError.length > 0) && (newError !== oldError))
      {
        this.errStr = newError;
        alert("Computation Error", "Compute Graph: " + this.errStr);
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
    this.errStr = "";   //Reset this each time we start the plotting process
    var str = this.serializeGraph(plot);
    var eng = GetCurrentEngine();
    var status, i, end;

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
      this.errStr = eng.getEngineErrors();
      dump("Computation Error: " +  "Query Graph: " + this.errStr + "\n");
      msiComputeLogger.Exception(e);
    }
    finally {
      if (plot) {
        plot.attributes.PlotStatus = status;
      }
      else {
        msidump("In GraphOverlay.js, Graph.computeQuery, no plot was passed in!");
        //This really shouldn't happen - if it does, only plots[0] should be marked as "Inited"
        end = (status === "Inited") ? 1 : this.plots.length;
        for (i = 0; i < end; i++) {
          this.plots[i].attributes.PlotStatus = status;
        }
      }
    }
    RestoreCursor();
  },
  init: function (graphObj) {
    // untested support for ctrl+V
    var graphController = {
      supportsCommand: function (cmd) {
        return (cmd === "cmd_MSIpaste");
      },
      isCommandEnabled: function (cmd) {
        if (cmd === "cmd_MSIpaste") {
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
    var DOMGraph;
    var DOMGs;
    var DOMFrame;
    var DOMObj;
    DOMGraph = document.createElementNS(htmlns, "graph");
    DOMGs = document.createElementNS(htmlns, "graphSpec");
    DOMFrame = document.createElementNS(htmlns, "msiframe");
    DOMObj = null;
    if (!forComp) {
      DOMObj = document.createElementNS(htmlns, "object");
    }
    var DOMCaption = document.createElementNS(htmlns,"imagecaption");
    DOMGraph.appendChild(DOMGs);
    DOMGraph.appendChild(DOMFrame);
    DOMFrame.appendChild(DOMCaption);
    if (DOMObj) DOMFrame.appendChild(DOMObj);
    this.reviseGraphDOMElement(DOMGraph, forComp, editorElement, optplot);
    return DOMGraph;
  },

  reviseGraphDOMElement: function (DOMgraph, forComp, editorElement, optplot) {
    var htmlns = "http://www.w3.org/1999/xhtml";
    var editor = msiGetEditor(editorElement);
    var DOMGs = DOMgraph.getElementsByTagName("graphSpec")[0];
    var DOMFrame = DOMgraph.getElementsByTagName("msiframe")[0];
    var DOMCaption = DOMFrame.getElementsByTagName("imagecaption")[0];
    var attr, value, alist, i, domPlots, plot, domPlotLabels, status, caption, captionloc, child, optnum;
    var graphData = new graphVarData(this);
    var unitHandler = new UnitHandler(editor);
    var units = this.getValue("Units");
    unitHandler.initCurrentUnit(units);
    this.frame.reviseFrameDOMElement(DOMFrame, forComp, editorElement);

    // loop through graph attributes and insert them
    alist = this.graphAttributeList();
    for (i = 0; i < alist.length; i++) {
      attr = alist[i];
      value = this.getValue(attr);
      if (value == null || value === "unspecified" || value === "undefined")
      {
        if (forComp && (attr === "TicksFontSize"))  //if we've set axis font size but not tick font size, want to make tick font size proportional
        {
          value = Number(this.getValue("AxesFontSize"));
          if (value !== Number.NaN) {
            value = String( Math.round(4 * value/5) );
          }
        }
        else {
          DOMGs.removeAttribute(attr);
        }
      }
      else if (forComp && this.omitAttributeIfDefault(attr) && !this.isUserSet(attr))
      {
        DOMGs.removeAttribute(attr);
      }
      else
      {
        DOMGs.setAttribute(attr, value);
      }
    }
    DOMGs.setAttribute("errStr", this.errStr);  //preserve this so we'll know not to try activating VCam, etc.
    if (this.userSetAttrs.length) {
      DOMGs.setAttribute("userSetAttrs", this.userSetAttrs.join(","));
    }

    // if the optional plot number was specified, include that plot first (since the query routine only operates on the first plot)
    // otherwise, for each plot, create a <plot> element
    // Clear out current plots first
    domPlots = DOMgraph.getElementsByTagName("plot");
    for (i = domPlots.length - 1; i >= 0; i--) {
      editor.deleteNode(domPlots[i]);
    }
    optnum = -1;
    if (optplot)
    {
      status = optplot.attributes.PlotStatus;
      if (status === "ERROR") {
        for (i = 0; (optnum < 0) && (i < this.plots.length); ++i)
        {
          if (this.plots[i] === optplot) {
            optnum = i;
          }
        }
        this.errStr = "ERROR, Plot number " + optnum + " " + this.errStr;
      } else {
        DOMGs.appendChild(optplot.createPlotDOMElement(document, forComp, graphData));
      }
    }
    for (i = 0; i < this.plots.length; i++) {
      plot = this.plots[i];
      if (optplot && (plot === optplot)) {
        continue;   //we already got this one
      }
      status = this.plots[i].attributes.PlotStatus;
      if (status === "ERROR") {
        this.errStr = "ERROR, Plot number " + i + " " + this.errStr;
      } else {
        DOMGs.appendChild(plot.createPlotDOMElement(document, forComp, graphData));
      }
    }
    domPlotLabels = DOMgraph.getElementsByTagName("plotLabel");
    for (i = domPlotLabels.length - 1; i >= 0; i--) {
      editor.deleteNode(domPlotLabels[i]);
    }
    for (i = 0; i < this.plotLabels.length; ++i) {
      DOMGs.appendChild(this.plotLabels[i].createPlotLabelDOMElement(document, forComp));
    }

    captionloc = this.getValue("CaptionPlace");
    var captionNodes = DOMFrame.getElementsByTagName("imagecaption");
    var captionNode = null;
    if (captionNodes.length > 0) {
      captionNode = captionNodes[0];
      // while (captionNodes.length > 1) {
      //   editor.deleteNode(captionNodes[1]);       
      // }
    }
    if (captionloc && captionloc !== "none") {
      var tlm;
      tlm = editor.tagListManager;
      DOMFrame.setAttribute("captionloc", captionloc);
      setStyleAttributeOnNode(DOMFrame, "caption-side", captionloc || "", editor);
      // if there is no caption node, create an empty one.
      if (!captionNode) {
         captionNode = editor.createElementWithDefaults("imagecaption");
         var namespace = { value: null };
         captionNode.appendChild(tlm.getNewInstanceOfNode(tlm.getDefaultParagraphTag(namespace), null, captionNode.ownerDocument));
         DOMFrame.appendChild(captionNode);
      }
    } else
    {
      DOMFrame.removeAttribute("captionloc");
      removeStyleAttributeFamilyOnNode( DOMFrame, "caption-side", editor);
    }
    // caption="<imagecaption>"+ (caption || "") +"</imagecaption>";
    // insertXML(editor, caption, DOMCaption, 0);
  },

  extractGraphAttributes: function (DOMGraph) {
    var key, value, i, plot, plotno, plotLabel, DOMGs, DOMPlots, DOMFrame, DOMPlotLabels;
    if (!DOMGraph) {
      return;
    }
    DOMGs = DOMGraph.getElementsByTagName("graphSpec");
    if (DOMGs.length > 0) {
      DOMGs = DOMGs[0];
    }
    else return;
    for (i = 0; i < DOMGs.attributes.length; i++) {
      key = DOMGs.attributes[i].nodeName;
      value = DOMGs.attributes[i].nodeValue;
      if (key === "userSetAttrs")
        this[key] = value.split( /,\s*/);  //turn it into an array
      else
        this[key] = value;
    }
    DOMFrame = DOMGraph.getElementsByTagName("msiframe");
    if (DOMFrame.length > 0) {
      DOMFrame = DOMFrame[0];
    }
    DOMPlots = DOMGraph.getElementsByTagName("plot");
    for (i = 0; i < DOMPlots.length; i++) {
      plot = new Plot(this.getDimension(), DOMPlots[i].getAttribute("PlotType"));
      plotno = this.addPlot(plot);
      plot.extractPlotAttributes(DOMPlots[i]);
      plot.attributes.PlotStatus = "Inited";
    }
    DOMPlotLabels = DOMGraph.getElementsByTagName("plotLabel");
    for (i = 0; i < DOMPlotLabels.length; ++i)
    {
      plotLabel = new PlotLabel(this.getDimension());
      this.addPlotLabel(plotLabel);
      plotLabel.extractPlotLabelAttributes(DOMPlotLabels[i]);
    }
    if (DOMFrame){
      this.frame.extractFrameAttributes(DOMFrame);
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
    else if (prefs.getPrefType(keyname) === prefs.PREF_INT) {
      value = prefs.getIntPref(keyname);
    }
    if (!value)
    {
      switch(key)
      {
        case "AxisFontFamily":
        case "TicksFontFamily":
          value = "sans-serif";
        break;
        case "AxisFontSize":
          value = "10";
        break;
        case "TicksFontSize":
          value = "8";
        break;
        case "AxisFontColor":
        case "TicksFontColor":
          value = "#000000";
        break;
        case "AxisFontItalic":
        case "AxisFontBold":
        case "TicksFontItalic":
        case "TicksFontBold":
          value = "false";
        break;
        default:                       break;
      }
    }
    return value;
  },
  getGraphAttribute: function (name) {
    return (this[name] || null);
  },
  getValue: function (key) {
    return this.getGraphAttribute(key) || this.getDefaultValue(key);
  },
  getPlotValue : function(key, plotNum)
  {
    if (plotNum < this.plots.length)
    {
      return this.plots[plotNum].getPlotValue(key);
    }
    return "";
  },
  getDimension : function()
  {
    return Number(this.getValue("Dimension"));
  },
  graphAttributeList: function () {
    return (this.GRAPHATTRIBUTES.concat(this.COMPATTRIBUTES));
  },
  graphCompAttributeList: function () {
    var NA = this.COMPATTRIBUTES;
    var dim = getDimension();
    if (Number(dim) === 2) {
      NA = attributeArrayRemove(NA, "ZAxisLabel");
      NA = attributeArrayRemove(NA, "OrientationTiltTurn");
    }
    if (!this.isAnimated())
    {
      NA = attributeArrayRemove(NA, "AnimateStart");
      NA = attributeArrayRemove(NA, "AnimateEnd");
      NA = attributeArrayRemove(NA, "AnimateFPS");
      NA = attributeArrayRemove(NA, "AnimateCurrTime");
    }
    // TubeRadius?
    return NA;
  },
  isAnimated : function()
  {
    for (var jj = 0; (jj < this.plots.length); ++jj)
    {
      if (this.plots[jj].attributes.Animate === "true") {
        return true;
      }
    }
    return false;
  },
  omitAttributeIfDefault : function(attr) {
    return (this.omitAttributeIfDefaultList.indexOf(attr) >= 0);
  },
  plotAttributeList: function (plotnum) {
    if (plotnum < this.plots.length)
      return this.plots[plotnum].plotAttributeList();
    return [];
  },
  plotElementList: function (plotnum) {
    if (plotnum < this.plots.length)
      return this.plots[plotnum].plotElementList();
    return [];
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
    if (!value || !value.length)
      value = this.getDefaultValue(name);
    var bChanged = this.isValueDifferent(name, value);
    this[name] = value;
    return bChanged;
  },
  isValueDifferent : function(name, value) {
    var sigDigits = this.significantDigitsInVal(name);
    if ((!this[name] || !this[name].length) && (value && value.length))
      return true;
    if ((this[name] && this[name].length) && (!value || !value.length))
      return true;
    if (sigDigits)
      return ( useSignificantDigits(value, sigDigits) !== useSignificantDigits(this[name], sigDigits) );
    if (name === "userSetAttrs")
    {
      for (var ii = 0; ii < this.userSetAttrs.length; ++ii)
      {
        if (this.userSetAttrs[ii] !== value[ii])
          return true;
      }
      return false;
    }
    return (value !== this[name]);
  },
  significantDigitsInVal : function(attr)
  {
    switch(attr)
    {
      case "ViewingBoxXMin":
      case "ViewingBoxXMax":
      case "ViewingBoxYMin":
      case "ViewingBoxYMax":
      case "ViewingBoxZMin":
      case "ViewingBoxZMax":
        return 4;
      case "CameraLocationX":
      case "CameraLocationY":
      case "CameraLocationZ":
      case "FocalPointX":
      case "FocalPointY":
      case "FocalPointZ":
      case "UpVectorX":
      case "UpVectorY":
      case "UpVectorZ":
        return 4;
      case "ViewingAngle":
        return 3;
      default:           return 0;
    }
  },
  setPlotValue : function(name, plotnum, value) {
    if (plotnum < this.plots.length)
    {
      this.plots[plotnum].setPlotValue(name, value);
    }
  },
  mapVCamNameToAttr : function(aProp)
  {
    switch(aProp)
    {
      case("positionX"):            return "CameraLocationX";
      case("positionY"):            return "CameraLocationY";
      case("positionZ"):            return "CameraLocationZ";
      case("focalPointX"):          return "FocalPointX";
      case("focalPointY"):          return "FocalPointY";
      case("focalPointZ"):          return "FocalPointZ";
      case("upVectorX"):            return "UpVectorX";
      case("upVectorY"):            return "UpVectorY";
      case("upVectorZ"):            return "UpVectorZ";
      case("keepUpVector"):         return "KeepUp";
      case("viewingAngle"):         return "ViewingAngle";
      case("orthogonalProjection"): return "OrthogonalProjection";

      case("beginTime"):            return "AnimateStart";
      case("endTime"):              return "AnimateEnd";
      case("currentTime"):          return "AnimateCurrTime";
      case("framesPerSecond"):      return "AnimateFPS";

      case("XAxisTitle") :          return "XAxisLabel";
      case("YAxisTitle") :          return "YAxisLabel";
      case("ZAxisTitle") :          return "ZAxisLabel";
      case("ViewingBoxXMin") :
      case("ViewingBoxXMax") :
      case("ViewingBoxYMin") :
      case("ViewingBoxYMax") :
      case("ViewingBoxZMin") :
      case("ViewingBoxZMax") :      return aProp;

      default:  return null;
    }
  },
  setCameraValsFromVCam : function(cameraVals, domGraph, bUserSetIfChanged) {
    if (cameraVals !== null)
    {
      msidump("Got camera vals from VCam:\n");
      var attrName;
      var graphSpec;
      var aProp;
      var bChanged = false;
      var graphSpecList = domGraph.getElementsByTagName("graphSpec");
      if (graphSpecList && graphSpecList.length)
        graphSpec = graphSpecList[0];
      for (aProp in cameraVals)
      {
        msidump("  " + aProp + " = " + cameraVals[aProp] + "\n");
        attrName = this.mapVCamNameToAttr(aProp);
        if (bUserSetIfChanged || !this.isUserSet(attrName))
        {
          bChanged = this.setGraphAttribute(attrName, cameraVals[aProp]) || bChanged;
          if (graphSpec)
            graphSpec.setAttribute(attrName, cameraVals[aProp]);
        }
      }
      if (bUserSetIfChanged && bChanged)
      {
        for (aProp in cameraVals)
        {
          attrName = this.mapVCamNameToAttr(aProp);
          if (attrName)
            this.markUserSet(attrName, true);
        }
        if (graphSpec)
          graphSpec.setAttribute("userSetAttrs", this.userSetAttrs.join(","));
      }
    }
  },
  setAnimationValsFromVCam : function(animVals, domGraph, bUserSetIfChanged) {
    if (animVals !== null)
    {
      msidump("Got camera vals from VCam:\n");
      var attrName;
      var bChanged = false;
      var graphSpec;
      var aProp;
      var graphSpecList = domGraph.getElementsByTagName("graphSpec");
      if (graphSpecList && graphSpecList.length)
        graphSpec = graphSpecList[0];
      for (aProp in animVals)
      {
        msidump("  " + aProp + " = " + animVals[aProp] + "\n");
        attrName = this.mapVCamNameToAttr(aProp);
        if ( attrName && (bUserSetIfChanged || !this.isUserSet(attrName)) )
        {
          bChanged = this.setGraphAttribute(attrName, animVals[aProp]) || bChanged;
          if (graphSpec)
            graphSpec.setAttribute(attrName, animVals[aProp]);
        }
      }
      if (bUserSetIfChanged && bChanged)
      {
        for (aProp in animVals)
        {
          attrName = this.mapVCamNameToAttr(aProp);
          if (attrName)
            this.markUserSet(attrName, true);
        }
        if (graphSpec)
          graphSpec.setAttribute("userSetAttrs", this.userSetAttrs.join(","));
      }
    }
  },
  setCoordSysValsFromVCam : function(coordSysVals, domGraph, bUserSetIfChanged) {
    if (coordSysVals !== null)
    {
      msidump("Got coord sys vals from VCam:\n");
      var attrName;
      var bChanged = false;
      var graphSpec;
      var aProp;
      var graphSpecList = domGraph.getElementsByTagName("graphSpec");
      if (graphSpecList && graphSpecList.length)
        graphSpec = graphSpecList[0];
      for (aProp in coordSysVals)
      {
        msidump("  " + aProp + " = " + coordSysVals[aProp] + "\n");
        attrName = this.mapVCamNameToAttr(aProp);
        if (bUserSetIfChanged || !this.isUserSet(attrName))
        {
          bChanged = this.setGraphAttribute(attrName, coordSysVals[aProp]) || bChanged;
          if (graphSpec)
            graphSpec.setAttribute(attrName, coordSysVals[aProp]);
        }
      }
      if (bUserSetIfChanged && bChanged)
      {
        for (aProp in coordSysVals)
        {
          attrName = this.mapVCamNameToAttr(aProp);
          if (attrName)
            this.markUserSet(attrName, true);
        }
        if (graphSpec)
          graphSpec.setAttribute("userSetAttrs", this.userSetAttrs.join(","));
      }
    }
  },
  recomputeVCamImage: function (editorElement, graphNode) {
    var filename = this.getGraphAttribute("ImageFile"); // the old name
    var file, newfile, oldfilename, match, filetype, newfilename, longnewfilename, obj;
    match = /[a-zA-Z0-9]+\.(xv[cz]$)/.exec(filename);
    if (match.length > 0)
    {
      filetype = match[1];
    }
    newfilename = "plots/" + createUniqueFileName("plot", filetype);
    longnewfilename = makeRelPathAbsolute(newfilename, editorElement);
    this.setGraphAttribute("ImageFile", longnewfilename);
    try
    {
      GetCurrentEngine().clearEngineStrings();  //clear errors before starting a plot
      // the filename that computeGraph uses is the one in the graph structure
      this.computeGraph(editorElement);
      obj = graphNode.getElementsByTagName("object")[0];

      // if the new file exists, delete the old
      newfile = Components.classes["@mozilla.org/file/local;1"].
                           createInstance(Components.interfaces.nsILocalFile);
      newfile.initWithPath(longnewfilename) ;
      if (newfile.exists())
      {
        oldfilename = msiMakeRelativeUrl(obj.data);
        file = Components.classes["@mozilla.org/file/local;1"].
                             createInstance(Components.interfaces.nsILocalFile);
        file.initWithPath( makeRelPathAbsolute(oldfilename, editorElement)) ;
        if (file.exists())
        {
          try {
            file.remove(false);
          }
          catch (e)
          {
          }
        }
        newfile.moveTo(null, oldfilename.replace(/plots\//,''));
      }

      if (obj.load) {
        obj.load(oldfilename);
      }

      var parentWindow = editorElement.ownerDocument.defaultView;
      if (!obj.id) {
        editor = msiGetEditor(editorElement);
        editorDoc = editor.document;
        obj.id = findUnusedId(editorDoc, "plot");
        obj.setAttribute("id", obj.id); // BBM: unnecessary??
      }
      saveObj(obj);
    }
    catch(exc) {
      msidump("In GraphOverlay.js, recomputeVCamImage(), exception: " + exc + "\n");
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
        netscape.security.PrivilegeManager.enablePrivilege("UniversalXPConnect");
        dropData = DNDUtils.getData("text/html", 0);
        str = dropData.QueryInterface(Components.interfaces.nsISupportsString);
        //      dragService.endDragSession(true);
        thisGraph.scheduleNewPlotFromText(__domGraph, str.data, __editorElement);
      }
      return 1;
    };
  },
  provideDropHandler: function (editorElement, domGraph) {
    var __domGraph = domGraph;
    var __editorElement = editorElement;
    var thisGraph = this;
    return function () {
      var dropData, str;
      var dragService = Components.classes["@mozilla.org/widget/dragservice;1"].getService();
      dragService = dragService.QueryInterface(Components.interfaces.nsIDragService);
      netscape.security.PrivilegeManager.enablePrivilege("UniversalXPConnect");
      dropData = DNDUtils.getData("text/html", 0);
      str = dropData.QueryInterface(Components.interfaces.nsISupportsString);
      thisGraph.scheduleNewPlotFromText(__domGraph, str.data, __editorElement);
      //    dragService.endDragSession(true);
      return 1;
    };
  }
};

// ************ Plot section; a graph can have many plots ******

function Plot(dim, ptype) {
  var attr, i;
  this.element = {};
  this.attributes = {};
  this.modFlag = {};
  this.parent = null;
  this.dimen = dim;
  this.attributes.PlotType = ptype;
  for (i = 0; i < this.PLOTATTRIBUTES.length; i++) {
    attr = this.PLOTATTRIBUTES[i];
    if (attr !== "PlotType")
      this.attributes[attr] = this.getDefaultPlotValue(attr);
    this.modFlag[attr] = false;
  }
  for (i = 0; i < this.PLOTELEMENTS.length; i++) {
    attr = this.PLOTELEMENTS[i];
    // if (attr === "Expression") {
    //   this.element[attr] = this.getDefaultPlotValue(attr);
    // }
    // else
    // {
    //   this.element[attr] = mathify(this.getDefaultPlotValue(attr));
    // }
    this.element[attr] = this.getDefaultPlotValue(attr);
    this.modFlag[attr] = false;
  }
}

Plot.prototype =
{
  constructor: Plot,
  PLOTATTRIBUTES: ["PlotStatus", "PlotType",
                           "LineStyle",  "LineThickness", "LineColor",
                           "DiscAdjust", "DirectionalShading", "BaseColor", "SecondaryColor",
                           "PointStyle", "SurfaceStyle"/*,  "IncludePoints"*/,    //"IncludePoints" and "IncludeLines" are determined by LineStyle and/or SurfaceStyle
                           "SurfaceMesh"/*,  "IncludeLines"*/,
                           "AISubIntervals", "AIMethod", "AIInfo", "FillPattern",
                           "Animate", "AnimCommonOrCustomSettings", "AnimateStart", "AnimateEnd",
                           "AnimateFPS", "AnimateVisBefore", "AnimateVisAfter",
                           "ConfHorizontalPts", "ConfVerticalPts", "DefaultedVars"],
  PLOTELEMENTS: ["Expression","XMax", "XMin",  "YMax", "YMin", "ZMax", "ZMin", "AnimMin", "AnimMax",
                           "XVar", "YVar", "ZVar", "AnimVar", "XPts", "YPts", "ZPts", "TubeRadius", "TubeRadialPts"],
  // Plot elements are all MathML.
  userSetAttrs: [],

  expectedVariableLists : function(plottype, dim, isParametric)
  {
    if (isParametric)
    {
      if (Number(dim) === 3)
      {
        switch(plottype)
        {
          case "curve":
          case "tube":
            return [["t"],["s"],["u"],["x"]];
          case "cylindrical":
            return [["u","v",],["\u03b8","z"],["x","y"]];  //[u,v],[theta,z],[x,y]
          case "spherical":
            return [["u","v"],["\u03b8","\u03d5"],["x","y"]];  //[u,v],[theta,phi],[x,y]
          default:
            return [["u","v"],["x","y"]];
        }
      }
      else  //dim presumably 2
      {
        switch(plottype)
        {
          case "polar":
            return [["\u03b8"],["t"],["s"],["u"],["x"]];  //theta,t,s,u,x
          case "conformal":  //What would a user want from a conformal parametric plot??? Probably ends in an error anyway...
            return [["z"],["t"],["s"],["u"],["x"]];
          default:
            return [["t"],["s"],["u"],["x"]];
        }
      }
    }
    else
    {
      if (Number(dim) === 3)
      {
        switch(plottype)
        {
          case "curve":
          case "tube":   //What would a user want from a curve or tube plot NOT given parametrically? Again, probably isn't going to work anyway.
            return [["x","y"],["u","v"],["s","t"]];
          case "cylindrical":
            return [["\u03b8","z"],["x","y"]];  //[theta,z],[x,y] - Here "y" would be interpreted as the actual z-axis...
          case "spherical":
            return [["\u03b8","\u03d5"],["x","y"]];  //[theta,phi],[x,y]
          case "implicit":
          case "gradient":
          case "vectorField":
            return [["x","y","z"],["u","v","w"]];
          default:
            return [["x","y"],["u","v"],["s","t"]];
        }
      }
      else
      {
        switch(plottype)
        {
          case "polar":
            return [["\u03b8"],["x"],["u"]];  //theta,x,u
          case "implicit":
          case "inequality":
          case "gradient":
          case "vectorField":
            return [["x","y"],["u","v"],["s","t"]];
          case "conformal":
            return [["z"],["w"],["\u03b6"],["x"]];  //z,w,zeta,x
          case "ode":
            return [["x"],["t"],["s"]];
          default:
            return [["x"],["u"],["t"],["s"]];
        }
      }
    }
  },
  isModified: function (x) {
    return (this.modFlag[x]);
  },
  setModified: function (x) {
    this.modFlag[x] = true;
  },
  isUserSet: function(attr) {
    return (this.userSetAttrs.indexOf(attr) >= 0);
  },
  markUserSet: function(attr, bUserSet) {
    var nIndex = this.userSetAttrs.indexOf(attr);
    if ((nIndex >= 0) && !bUserSet)
      this.userSetAttrs.splice(nIndex,1);
    else if ((nIndex < 0) && bUserSet)
      this.userSetAttrs.push(attr);
  },
  getDimension : function()
  {
    if (this.parent)
      return this.parent.getDimension();
    if (this.dim)
      return this.dim;
    return 2;  //make a bad guess?
  },
  varNameFromFoundVariable : function(whichVar)
  {
    if (!this.hasVariable(whichVar))
      return false;
    return (this.attributes.DefaultedVars.indexOf(whichVar) < 0);
  },
  isAnimated : function()
  {
    return (this.attributes.Animate === "true");
  },
  hasVariable : function(whichVar)
  {
    if (whichVar === "AnimVar")
    {
      return (this.attributes.Animate === "true");
    }
    var plottype = this.attributes.PlotType;
    var nWhichVar = Graph.prototype.plotVariablePropertyNames.indexOf(whichVar);
    return ((nWhichVar >= 0) && (nWhichVar < plotVarsNeeded(this.getDimension(), plottype, false)) );
      //pass "false" for animated to plotVarsNeeded, as we don't care about animation here
  },
  createPlotDOMElement: function (doc, forComp, graphData) {
     // return a DOM <plot> node. Unless forComp, only include non-default attributes and elements
     // do the plot attributes as DOM attributes of <plot>
    var status = this.PlotStatus;
    var attr;
    var tNode;

    if (status !== "Deleted") {
      var plotData = graphData ? graphData.findPlotData(this) : null;
      if (plotData)
      {
        this.checkAnimationVar(plotData);
      }
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
      DOMPlot.setAttribute("PlotStatus", status);
      // do the plot elements as document fragment children of <plot>
      attrs = (forComp) ? this.plotCompElementList() : this.plotElementList();
      for (i=0; i<attrs.length; i++) {
        attr = attrs[i];
        if (forComp || (this.element[attr])) {
          var DOMEnode = doc.createElement(attr);
          var textval = this.element[attr];
          if ((attr === "Expression") && forComp)
            textval = this.adjustExpressionForComputation(plotData);
          if ((textval !== "") && (textval !== "unspecified")) {
              // textval = runFixup(textval); // runFixup(runFixup(textval));
              tNode = (new DOMParser()).parseFromString (textval, "text/xml");
              tNode = tNode.documentElement;
              removeInvisibles(tNode);
              textval =  GetMathAsString(tNode); //GetFixedMath(tNode);
            tNode = (new DOMParser()).parseFromString (textval, "text/xml");
            DOMEnode.appendChild (tNode.documentElement);
            DOMPlot.appendChild (DOMEnode);
          }
        }
      }      
      if (this.userSetAttrs.length)
        DOMPlot.setAttribute("userSetAttrs", this.userSetAttrs.join(","));
      return DOMPlot;
    }
    return null;
  },

  adjustExpressionForComputation : function(plotData) {
    switch(this.attributes.PlotStatus)
    {
      case "New":
      case "Deleted":
      case "ERROR":
        return this.element.Expression;
    }
    switch( plotData.plottype )
    {
      case "polar":
      case "spherical":
      case "cylindrical":
        if (!plotData.isParametric())
        {
          return runFixup(createPolarExpression(this));
        }
        break;
      default:
        return this.element.Expression;
    }
  },
  //the following is really only correcting for "old-style" animated plots which may have been saved - where the
  //  animation var isn't identified as such, but is just an extra variable
  checkAnimationVar : function(plotData) {
    if (this.attributes.Animate === "true")
    {
      if (!this.element.AnimVar || !this.element.AnimVar.length)
      {
        var numPlainVars = plotData.numPlainVariablesNeeded();
        var extraVar = plotData.getVariableByIndex(numPlainVars);
        if (extraVar && extraVar.length)
        {
          this.element.AnimVar= extraVar;
          plotData.setVariableByIndex(numPlainVars, "");
        }
      }
    }
  },
  revisePlotDOMElement: function (doc, forComp, domPlot) {
    // return a DOM <plot> node. Unless forComp, only include non-default attributes and elements
    // do the plot attributes as DOM attributes of <plot>
    var status = this.PlotStatus;
    var attr;
    if (status !== "Deleted") {
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
            textval = runFixup(runFixup(textval));
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
    var child;
    var mathnode;
    var serialized;
    for (j = 0; j < DOMPlot.attributes.length; j++) {
      attr = DOMPlot.attributes[j];
      key = attr.nodeName;
      value = attr.nodeValue;
      if (key === "userSetAttrs")
        this[key] = value.split( /,\s*/);  //turn it into an array
      else
        this.attributes[key] = value;
    }

    // the children of <plot> are in general <plotelement> <mathml...> </plotelement>
    // We should get an ELEMENT_NODE for the <plotelement>, and the localName is the attribute name.
    // The data is either text or mathml. For text, grab the text value, which must be in the nodeValue
    // of the first child. For DOM fragment, store serialization of the fragment.
    //var children = DOMPlot.childNodes;
    child = DOMPlot.firstChild;
    for (j = 0; j < DOMPlot.childNodes.length; ++j)
    {
      child = DOMPlot.childNodes[j];
//      key = child.localName;
      key = msiGetBaseNodeName(child);
      if (child.nodeType === Node.ELEMENT_NODE) {
        mathnode = child.getElementsByTagName("math")[0];
        serialized = this.parent.ser.serializeToString(mathnode);
        this.element[key] = serialized;
      }
       child = child.nextSibling;
    }
  },
  copyAttributes : function(otherPlot, attrArray)
  {
    var newVal;
    for (var jj = 0; jj < attrArray.length; ++jj)
    {
      newVal = otherPlot.getPlotValue(attrArray[jj]);
      if (newVal && newVal.length)
        this.setPlotValue(attrArray[jj], newVal);
    }
  },
  getPlotValue: function (key) {
    // look up the value key. If not found, look up the default value.
    var value;
    if (key in this)
      value = this[key];
    else if (key in this.attributes)
      value = this.attributes[key];
    else if (key in this.element)
      value = this.element[key];
    if ((value !== null) && (value !== "") && key === "Expression") {
      return dressUpMathString2(value);
    }
    switch(key)
    {
      case "XPts":
      case "YPts":
      case "ZPts":
        return this.getDefaultNumPlotPoints(key);
      case "AnimateStart":
      case "AnimateEnd":
      case "AnimateFPS":
        if (this.getPlotValue("AnimCommonOrCustomSettings") === "common")
          return this.parent.getValue(key);
        break;
      default:
      break;
    }
    return (this.getDefaultPlotValue(key));
  },
  getDefaultNumPlotPoints : function (key) {
    var ptype = this.attributes.PlotType;
    var dim = this.getDimension();
    var value = getPlotDefaultValue(dim, ptype, key);
    if (!value)
    {
      if (Number(dim) === 2) {
        switch (ptype) {
        case "implicit":
          value = 20;
          break;
        case "gradient":
        case "vectorField":
          value = 10;
          break;
        case "inequality":
          value = 80;
          break;
        case "conformal":
          value = 20;
          break;
        case "approximateIntegral":
          value = 400;
          break;
        default:
          value = 400;
          break;
        }
      } else {
        switch (ptype) {
        case "parametric":
        case "curve":
          value = 80;
          break;
        case "implicit":
          value = 6;
          break;
        case "gradient":
        case "vectorField":
          value = 6;
          break;
        case "tube":
          if (key === "TubeRadialPts")
            value = 9;
          else
            value = 40;
          break;
        default:
          value = 20;
          break;
        }
      }
    }
    if (value)
      return GetNumAsMathML(value);
    return value;
  },
  getDefaultPlotValue: function (key) {
    // get defaults from preference system, or if not there, from hardcoded list
    /* jshint ignore:start */
    var math = "<math xmlns='http://www.w3.org/1998/Math/MathML'>";
    /* jshint ignore:end */
    var ptype = this.attributes.PlotType;
    var dim = this.getDimension();
    switch(key)
    {
      case "XPts":
      case "YPts":
      case "ZPts":
      case "TubeRadialPts":
        return this.getDefaultNumPlotPoints(key);
    }
    var value = getPlotDefaultValue(dim, ptype, key);
    if (!value)
    {
      switch (key) {
        case "PlotStatus":
          value = "New"; // this should never be moved from here
          break;
        case "ConfHorizontalPts":
          value = "15";
          break;
        case "ConfVerticalPts":
          value = "15";
          break;
          //      default:
          //        value = math + "<mrow><mi tempinput=\"true\">()</mi></mrow></math>";
        case "Expression":
          value = "<math xmlns='http://www.w3.org/1998/Math/MathML'><mi tempinput='true'></mi></math>";
          break;
        case "TubeRadius":
          value = "<math xmlns='http://www.w3.org/1998/Math/MathML'><mn>1</mn></math>";
        break;
        case "AnimCommonOrCustomSettings":
          value = "common";
        break;
        case "AnimateVisBefore":
        case "AnimateVisAfter":
          value = "true";
        break;
        default:
          value = "";
      }
    }
    return value;
  },
  plotAttributeList: function () {
    var NA = this.plotCompAttributeList();
    NA.push("PlotStatus");  //this is the only thing removed in plotCompAttributeList() which should be returned!
    return NA;
//    return (this.PLOTATTRIBUTES);
  },
  plotElementList: function () {
    return this.plotCompElementList();
//    return (this.PLOTELEMENTS);
  },
  plotCompAttributeList: function () {
    var NA;
    var dim = this.getDimension();
    var ptype = this.attributes.PlotType;
    var animate = this.attributes.Animate;
    NA = attributeArrayRemove(this.PLOTATTRIBUTES, "PlotStatus");

    if (ptype !== "approximateIntegral") {
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

    if (Number(dim) === 2) {
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

    if ((ptype !== "rectangular") && (ptype !== "parametric") && (ptype !== "implicit")) {
      NA = attributeArrayRemove(NA, "DiscAdjust");
    }

    if (animate !== "true") {
      NA = attributeArrayRemove(NA, "AnimCommonOrCustomSettings");
      NA = attributeArrayRemove(NA, "AnimateStart");
      NA = attributeArrayRemove(NA, "AnimateEnd");
      NA = attributeArrayRemove(NA, "AnimateFPS");
      NA = attributeArrayRemove(NA, "AnimateVisBefore");
      NA = attributeArrayRemove(NA, "AnimateVisAfter");
    }

    return NA;
  },
  plotCompElementList: function () {
    var dim = this.getDimension();
    var animate = (this.attributes.Animate === "true");
    var ptype = this.attributes.PlotType;
    var NA = this.PLOTELEMENTS;

    if (ptype !== "tube") {
      NA = attributeArrayRemove(NA, "TubeRadius");
      NA = attributeArrayRemove(NA, "TubeRadialPts");
    }

    var nvars = plotVarsNeeded(dim, ptype, false);
//    var nvars = plotVarsNeeded(dim, ptype, animate);

    if (nvars < 2) {
      NA = attributeArrayRemove(NA, "YVar");
      NA = attributeArrayRemove(NA, "YPts");
      NA = attributeArrayRemove(NA, "YMax");
      NA = attributeArrayRemove(NA, "YMin");
    }

    if (nvars < 3) {
      NA = attributeArrayRemove(NA, "ZVar");
      NA = attributeArrayRemove(NA, "ZPts");
      NA = attributeArrayRemove(NA, "ZMax");
      NA = attributeArrayRemove(NA, "ZMin");
    }

    if (!animate)
    {
      NA = attributeArrayRemove(NA, "AnimVar");
      NA = attributeArrayRemove(NA, "AnimMax");
      NA = attributeArrayRemove(NA, "AnimMin");
    }

    return NA;
  },
  getPlotAttribute: function (name) {
    return (this.attributes[name]);
  },
  setPlotValue : function (name, value) {
    if (this.PLOTATTRIBUTES.indexOf(name) >= 0)
      this.setPlotAttribute(name, value);
    else if (this.PLOTELEMENTS.indexOf(name) >= 0)
      this.setPlotElement(name, value);
  },
  setPlotAttribute: function (name, value) {
    this.attributes[name] = value;
    this.setModified(name);
  },
  setPlotElement : function (name, value) {
    this.element[name] = value;
    this.setModified(name);
  },
  copyPlotAttributes : function(otherPlot, attrArray)
  {
    for (var jj = 0; jj < attrArray.length; ++jj)
      this.setPlotValue( attrArray[jj], otherPlot.getPlotValue(attrArray[jj]) );
  },
  computeQuery: function () {
    // call the compute engine to guess at graph attributes
    this.parent.computeQuery(this);
  }
};

// ************ Plot Labels; graph can have several plot labels ******
function PlotLabel(dim)
{
  if (!dim)
    dim = 2;
  this.dimension = dim;
  this.attributes = {};
  var attrs = this.PLOTLABELATTRIBUTES;
  for (var ii = 0; ii < attrs.length; ++ii)
    this.setPlotLabelAttribute( attrs[ii], this.getDefaultPlotLabelAttribute(attrs[ii]) );
}

PlotLabel.prototype =
{
  //The order of these IS significant
  PLOTLABELATTRIBUTES : ["PlotType", "PositionType", "OrientationType", "Text",
                         "PositionX", "PositionY", "PositionZ",
                         "OrientationX", "OrientationY", "OrientationZ", "Turn", "Tilt",
                         "HorizontalAlignment", "VerticalAlignment", "Billboarding",
                         "TextFontFamily", "TextFontSize", "TextFontColor", "TextFontBold", "TextFontItalic"],
  KEYATTRIBUTES: ["PositionType", "OrientationType","Billboarding"],
  getDimension : function()
  {
    if (this.parent)
      return this.parent.getDimension();
    return this.dimension;
  },
  plotLabelAttributeList : function(forComp)
  {
    var theList = this.PLOTLABELATTRIBUTES;
    var dim = this.getDimension();
    var bNonCartesianOrientation = ((this.getPlotLabelAttribute("OrientationType") === "angle") ||
      (this.getPlotLabelAttribute("OrientationType") === "tiltAndTurn"));
    var bNeedNonCartesianOrientation = bNonCartesianOrientation;
    if (this.isHeaderOrFooter())
    {
      theList = attributeArrayRemove(theList, "PositionX");
      theList = attributeArrayRemove(theList, "PositionY");
      theList = attributeArrayRemove(theList, "PositionZ");
      theList = attributeArrayRemove(theList, "OrientationX");
      theList = attributeArrayRemove(theList, "OrientationY");
      theList = attributeArrayRemove(theList, "OrientationZ");
      theList = attributeArrayRemove(theList, "Billboarding");
      theList = attributeArrayRemove(theList, "VerticalAlignment");
      theList = attributeArrayRemove(theList, "OrientationType");
      theList = attributeArrayRemove(theList, "Tilt");
      theList = attributeArrayRemove(theList, "Turn");
    }
    else
    {
      if (Number(dim) === 2)
      {
        if (forComp)
          bNeedNonCartesianOrientation = true;
        theList = attributeArrayRemove(theList, "PositionZ");
        theList = attributeArrayRemove(theList, "OrientationZ");
        theList = attributeArrayRemove(theList, "Tilt");
        theList = attributeArrayRemove(theList, "Billboarding");
      }
      else if (forComp)
        bNeedNonCartesianOrientation = false;
      if (bNeedNonCartesianOrientation !== bNonCartesianOrientation)
        this.convertOrientation(bNeedNonCartesianOrientation);
      if (bNeedNonCartesianOrientation)
      {
        theList = attributeArrayRemove(theList, "OrientationX");
        theList = attributeArrayRemove(theList, "OrientationY");
        theList = attributeArrayRemove(theList, "OrientationZ");
      }
      else
      {
        theList = attributeArrayRemove(theList, "Turn");
        theList = attributeArrayRemove(theList, "Tilt");
      }
    }
    if (forComp)
      theList = attributeArrayRemove(theList, "OrientationType");

    return theList;
  },
  plotLabelKeyAttributeList : function()
  {
    var theList = this.KEYATTRIBUTES;
    var dim = this.getDimension();
    if (Number(dim) === 2)
      theList = attributeArrayRemove(theList, "Billboarding");
    return theList;
  },
  extractPlotLabelAttributes : function(DOMPlotLabel)
  {
    var ii;
    for (ii = 0; ii < DOMPlotLabel.attributes.length; ++ii)
    {
      this.setPlotLabelAttribute(DOMPlotLabel.attributes[ii].nodeName, DOMPlotLabel.attributes[ii].nodeValue);
    }
  },
  getPlotLabelAttribute : function(key, bNoDefault)
  {
    if (key in this.attributes)
      return this.attributes[key];
    if (!bNoDefault)
      return this.getDefaultPlotLabelAttribute(key); //really?
    return null;
  },
  getDefaultPlotLabelAttribute : function(key)
  {
    switch(key)
    {
      case "PlotType":                  return "text";  //this is always the same for plot labels
      case "Text":                      return "";
      case "PositionX":                 return "0";
      case "PositionY":                 return "0";
      case "PositionZ":                 return "0";
      case "OrientationX":              return "1";
      case "OrientationY":              return "0";
      case "OrientationZ":              return "0";
      case "OrientationType":           return "cartesian";
      case "HorizontalAlignment":       return "Left";
      case "VerticalAlignment":         return "BaseLine";
      case "Billboarding":              return "true";
      //Note that the font specifications are by default empty strings - this should cause engine to use axes font
      case "TextFontName":              return "";
      case "TextFontSize":              return "";
      case "TextFontColor":             return "";
      case "TextFontBold":              return "";
      case "TextFontItalic":            return "";
      case "PositionType":              return "positioned";
      case "Turn":                      return "0";
      case "Tilt":                      return "0";
      default:
      break;
    }
    return null;
  },
  setPlotLabelAttribute : function(key, value)
  {
    var bConvertOrientation = ( (key === "OrientationType") && (this.attributes.OrientationType !== value) &&
      ((value === "cartesian") || (this.attributes.OrientationType === "cartesian")) );
    if (this.PLOTLABELATTRIBUTES.indexOf(key) >= 0)
    {
      if (!value || !value.length)
        value= this.getDefaultPlotLabelAttribute(key);
      this.attributes[key] = value;
    }
    if (bConvertOrientation)
      this.convertOrientation((value === "angle") || (value === "tiltAndTurn"));
  },
  createPlotLabelDOMElement: function (doc, forComp)
  {
     // return a DOM <plotLabel> node.
    var DOMPlotLabel = doc.createElement("plotLabel");
    this.revisePlotLabelDOMElement(doc, DOMPlotLabel, forComp);
    return DOMPlotLabel;
  },
  revisePlotLabelDOMElement: function (doc, domPlotLabel, forComp)
  {

    var attr, val;
    var attrs = this.plotLabelAttributeList(forComp);
    for (var i=0; i<attrs.length; i++)
    {
      attr = attrs[i];
      val = this.attributes[attr];
      if (val && (val !== "") && (val !== "unspecified"))
        domPlotLabel.setAttribute (attr, val);
      else
        domPlotLabel.removeAttribute(attr);
    }
  },
  isHeaderOrFooter : function()
  {
    var val = this.getPlotLabelAttribute("PositionType");
    return ((val === "header") || (val === "footer"));
  },
  clone : function()
  {
    var newLabel = new PlotLabel();
    newLabel.dimension = this.dimension;
    var attrs = this.plotLabelAttributeList();
    for (var ii = 0; ii < attrs.length; ++ii)
      newLabel.setPlotLabelAttribute( attrs[ii], this.getPlotLabelAttribute(attrs[ii]) );
    return newLabel;
  },
  convertOrientation : function(fromCartesian)
  {
    if (this.isHeaderOrFooter())
      return;
    if (fromCartesian)
      this.convertOrientationToTiltAndTurn();
    else
      this.convertOrientationToCartesian();
  },
  convertOrientationToTiltAndTurn : function()
  {
    var retVal;
    if (Number(this.getDimension()) === 3)
    {
      retVal = convertCartesianToSpherical(Number(this.getPlotLabelAttribute("OrientationX")),
                                           Number(this.getPlotLabelAttribute("OrientationY")),
                                           Number(this.getPlotLabelAttribute("OrientationZ")));
      this.setPlotLabelAttribute("Turn", String(retVal[1]));
      this.setPlotLabelAttribute("Tilt", String((Math.PI/2) - retVal[2]));
      //We do this transformation since a spherical coordinate of PI/2 should mean a tilt of 0
    }
    else
    {
      retVal = convertCartesianToPolar(Number(this.getPlotLabelAttribute("OrientationX")),
                                           Number(this.getPlotLabelAttribute("OrientationY")));
      this.setPlotLabelAttribute("Turn", String(retVal[1]));
    }
  },
  convertOrientationToCartesian : function()
  {
    //We use oldLength below so that if the user toggles from Cartesian to non-Cartesian and back she gets what she started with
    var oldLength, retVal;
    if (Number(this.getDimension()) === 3)
    {
      oldLength = this.getNorm([Number(this.getPlotLabelAttribute("OrientationX")), Number(this.getPlotLabelAttribute("OrientationY")), Number(this.getPlotLabelAttribute("OrientationZ"))]);
      if (!oldLength)
        oldLength = 1;
      retVal = convertSphericalToCartesian(oldLength, Number(this.getPlotLabelAttribute("Turn")),
                                              (Math.PI/2) - Number(this.getPlotLabelAttribute("Tilt")));
                                //We do this transformation since a spherical coordinate of PI/2 should mean a tilt of 0
      this.setPlotLabelAttribute("OrientationX", String(retVal[0]));
      this.setPlotLabelAttribute("OrientationY", String(retVal[1]));
      this.setPlotLabelAttribute("OrientationZ", String(retVal[2]));
    }
    else
    {
      oldLength = this.getNorm([Number(this.getPlotLabelAttribute("OrientationX")), Number(this.getPlotLabelAttribute("OrientationY"))]);
      if (!oldLength)
        oldLength = 1;
      retVal = convertPolarToCartesian(oldLength, Number(this.getPlotLabelAttribute("Turn")));
      this.setPlotLabelAttribute("OrientationX", String(retVal[0]));
      this.setPlotLabelAttribute("OrientationY", String(retVal[1]));
    }
  },
  getNorm : function(anArray)
  {
    var sqSum = 0;
    for (var ii = 0; ii < anArray.length; ++ii)
      sqSum += anArray[ii] * anArray[ii];
    return Math.sqrt(sqSum);
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
  FRAMEDOMATTRIBUTES: ["pos", "sidemargin", "topmargin", "width", "height",
  "units",  "borderw", "padding", "background-color", "border-color", "ltxfloat",
    "captionloc", "req", "floatlocation"],
  // WRAPPERATTRIBUTES: ["borderw", "padding", "ltx_height", "ltx_width", "border-color", "background-color"],
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
	if (name in this.attributes){
	    return (this.attributes[name]);
        } else {
	    return null;
	}
  },
  setFrameAttribute: function (name, value) {
    this.attributes[name] = value;
    this.setModified(name);
  },
  translateName: function(inName) {
    switch(inName) {
      case "units": return "Units";
      case "sidemargin": return "HMargin";
      case "topmargin": return "VMargin";
      case "width": return "Width";
      case "height": return "Height";
      case "captionloc": return "CaptionPlace";
      default: return inName;
    }
  },
  extractFrameAttributes: function (DOMFrame) {
    var graph = this.parent;
    if( DOMFrame.hasAttribute("units") && graph)
      graph.setGraphAttribute("Units", DOMFrame.getAttribute("units"));

    var i, att, attlist, len;
    graph = this.parent;
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
            this.setFrameAttribute("pos", DOMFrame.getAttribute(att));
            break;
          case "width":
            this.setFrameAttribute("Width", DOMFrame.getAttribute(att));
            break;
          case "height":
            this.setFrameAttribute("Height", DOMFrame.getAttribute(att));
            break;
          // case "ltx_height":
          //   this.setFrameAttribute("ltx_height", DOMFrame.getAttribute(att));
          //   break;
          // case "ltx_width":
          //   this.setFrameAttribute("ltx_width", DOMFrame.getAttribute(att));
          //   break;
          case "borderw":
            this.setFrameAttribute("borderw", DOMFrame.getAttribute(att));
            break;
          case "border-color":
            this.setFrameAttribute("border-color", DOMFrame.getAttribute(att));
            break;
          case "padding":
            this.setFrameAttribute("padding", DOMFrame.getAttribute(att));
            break;
          case "placeLocation":
            if (DOMFrame.getAttribute(att).length > 0) {
              this.setFrameAttribute("placeLocation", DOMFrame.getAttribute(att));
            }
            break;
          case "captionloc":
            graph.setGraphAttribute("CaptionPlace", DOMFrame.getAttribute(att));
            break;
          case "ltxfloat":
            if (DOMFrame.getAttribute("pos") === "floating") {
              this.setFrameAttribute("ltxfloat", DOMFrame.getAttribute(att));
            }
            break;
          // case "textalignment":
          //   if (DOMFrame.getAttribute(att)) {
          //     this.setFrameAttribute(att, DOMFrame.getAttribute(att));
          //   }
          //   break;
          case "captionloc":
            if (DOMFrame.hasAttribute("captionloc")) {
              graph.setGraphAttribute("CaptionPlace", DOMFrame.getAttribute("captionloc"));
            }
            else {
              graph.setGraphAttribute("CaptionPlace", "none");
            }
            break;
          default: break;
        }
      }
    }
  },

  reviseFrameDOMElement: function (DOMFrame, forComp, editorElement) {
    var  editor = msiGetEditor(editorElement);
    var attributes, i, j, att, graph, units, height, width, pos, placeLocation, captionlocation, x;
    var DOMObj = DOMFrame.getElementsByTagName("object")[0];
    var unitHandler = new UnitHandler(editor);
    var needsWrapfig = false;
    try {
      graph = this.parent;
      units = graph.getValue("Units");
      unitHandler.initCurrentUnit(units);
      attributes = this.frameAttributeList().concat(this.parent.graphAttributeList());
      for (j = 0; j < attributes.length; j++) {
        att = attributes[j];
        if (!att) continue;
        switch (att) {
          // case "ImageFile":
          //   editor.setAttribute(DOMObj, "data", graph.getValue("ImageFile"));
          //   break;
          case "HMargin":
            editor.setAttribute(DOMFrame, "sidemargin", this.getFrameAttribute(att));
            break;
          case "VMargin":
            editor.setAttribute(DOMFrame, "topmargin", this.getFrameAttribute(att));
            break;
          case "Height":
            height = Number(graph.getValue(att));
            // editor.setAttribute(DOMFrame, "ltx_height", height);
            editor.setAttribute(DOMFrame, "height", height);
            setStyleAttributeOnNode(DOMObj, "height", height + units, null);
            // dimensions of outer msiframe need to be adjusted for border and padding
            x = this.getFrameAttribute("border");
            if (x)
            {
  //            height += 2*(x-0);
            }
            x = this.getFrameAttribute("padding");
            if (x)
            {
  //            height += 2*(x-0);
            }
            // editor.setAttribute(DOMFrame, "height", height);
            setStyleAttributeOnNode( DOMFrame, "height", height + units, null);
            break;
          case "Width":
            width = Number(graph.getValue(att));
            editor.setAttribute(DOMFrame, "width", width);
            setStyleAttributeOnNode(DOMObj, "width", width + units, null);

            x = this.getFrameAttribute("border");
            if (x) {
  //   BBM: experimenting        width += 2*(x-0);
            }
            x = this.getFrameAttribute("padding");
            if (x) {
  //            width += 2*(x-0);
            }
            editor.setAttribute(DOMFrame, "width", width);
            setStyleAttributeOnNode( DOMFrame, "width", width + units, null);
            break;
          case "border":
          case "borderw":
            editor.setAttribute(DOMFrame, "borderw", this.getFrameAttribute("border"));
            break;
          case "padding":
            editor.setAttribute(DOMFrame, "padding", this.getFrameAttribute(att));
            setStyleAttributeOnNode(DOMFrame, "padding", this.getFrameAttribute(att), null);
            break;
          case "Units":
            editor.setAttribute(DOMFrame, "units", units);
            break;
          case "BGColor":
            editor.setAttribute(DOMFrame, "background-color", hexcolor(this.getFrameAttribute("BGColor")));
            setStyleAttributeOnNode(DOMFrame, "background-color", hexcolor(this.getFrameAttribute("BGColor")));
            break;
          case "borderColor":
            editor.setAttribute(DOMFrame, "border-color", hexcolor(this.getFrameAttribute(att)));
            setStyleAttributeOnNode(DOMFrame, "border-color", hexcolor(this.getFrameAttribute(att)));
            break;
          case "pos":
            editor.setAttribute(DOMFrame, "pos", this.getFrameAttribute(att));
            break;
          case "placement":
            pos = this.getFrameAttribute(att);
            editor.setAttribute(DOMFrame, "pos", pos);
            needsWrapfig = (pos === "L" || pos === "I" || pos === "R" || pos === "O" ||
                            pos === "left" || pos === "inside" || pos === "right" || pos === "outside");
            break;
          case "textalignment":
            editor.setAttribute(DOMFrame, att, this.getFrameAttribute(att));
            setStyleAttributeOnNode( DOMFrame, "text-align", this.getFrameAttribute(att) || "", null);
            break;
          case "placeLocation":
            placeLocation = this.getFrameAttribute(att);
            editor.setAttribute(DOMFrame, "placeLocation", placeLocation);
            break;
          case "CaptionPlace":
            captionlocation = this.getFrameAttribute("captionloc");
            editor.setAttribute(DOMFrame, "captionloc", captionlocation);
            break;
          case "floatPlacement":
            editor.setAttribute(DOMFrame, "floatlocation", this.getFrameAttribute("floatPlacement"));
            break;
          default:
  //          editor.setAttribute(DOMFrame, att, this.getFrameAttribute(att));
            break;
        }
      }
      editor.setAttribute(DOMFrame, "msi_resize", "true");
//      editor.setAttribute(DOMFrame, "style", this.getFrameAttribute("style"));

      // what about overhang?
      // Now we build the CSS style for the object and the frame
      var isfloat = this.getFrameAttribute("pos") === "floating";
      var isdisplay = this.getFrameAttribute("pos") === "center";
      var lmargin;
      var rmargin;
      if (isdisplay || (isfloat && this.getFrameAttribute("floatPlacement")==="center")) {
        lmargin = rmargin = "auto";
        removeStyleAttributeFamilyOnNode(DOMFrame, "float");
      }
      else {
        lmargin = unitHandler.getValueStringAs(this.getFrameAttribute("HMargin"), "px");
        rmargin = lmargin;
        editor.setAttribute(DOMFrame, "pos", this.getFrameAttribute("pos"));
        switch (this.getFrameAttribute("pos")) {
          case "left" :
          case "inside" :
            lmargin = "0px";
            setStyleAttributeOnNode(DOMFrame, "float", "left", null);
            break;
          case "right" :
          case "outside" :
            rmargin = "0px";
            setStyleAttributeOnNode(DOMFrame, "float", "right", null);
            break;
          default : removeStyleAttributeFamilyOnNode(DOMFrame, "float");
        }
      }
      var vmargin = unitHandler.getValueStringAs(this.getFrameAttribute("VMargin"), "px");
      setStyleAttributeOnNode( DOMFrame, "margin", vmargin + " " + rmargin + " " + vmargin + " " + lmargin, null);
      var border = unitHandler.getValueStringAs(this.getFrameAttribute("borderw"), "px");
      if (!forComp)  //don"t trigger any loading if we're only serializing - this isn't a "real" <object>
      {
        var existingObjFile = DOMObj.getAttribute("data");
        var newImageFile = graph.getGraphAttribute("ImageFile");
        var bLoadIt = false;
        if (newImageFile && newImageFile.length)
        {
          if (!existingObjFile || !existingObjFile.length)
            bLoadIt = true;
          else
            bLoadIt = isDifferentPlotFile(existingObjFile, newImageFile, editorElement);
        }
        if (bLoadIt)
        {
          var objfileShortName=  graph.getGraphAttribute("ImageFile");
          var vcamUri = msiMakeAbsoluteUrl(objfileShortName,editorElement);
          if (!existingObjFile || !existingObjFile.length)
          {
            msidump("In reviseDOMFrameElement, changing data attribute to [" + vcamUri + "]\n");
            DOMObj.setAttribute( "data", objfileShortName );
            //NOTE!!! You must set the vcam source file in the object before setting its type, or we don't seem to be able to get
      //  the scriptable vcam interface to work!
            // var filetype = graph.getDefaultValue("filetype");
            // msidump("SMR file type is " + filetype + "\n");
            // if (filetype === "xvz") {
            //   DOMObj.setAttribute("type", "application/x-mupad-graphics+gzip");
            // } else if (filetype === "xvc") {
            //   DOMObj.setAttribute("type", "application/x-mupad-graphics+xml");
            // }
          }
        }
      }
      if (needsWrapfig) {
        editor.setAttribute(DOMFrame, "req", "wrapfig");
      }
	if (DOMObj){
	    DOMObj.setAttribute("alt", "Generated Plot");
	    DOMObj.setAttribute("msigraph", "true");
	}
//      DOMObj.setAttribute("data", graph.getValue("ImageFile"));
      return (DOMFrame);
    }
    catch (e) {
      msidump("reviseFrameDOMElement "+e.message);
    }
  }
};

function dressUpMathString2(mathString) {
  mathString = mathString.replace('<math><math>','<math>'); // BBM: I don't know where the double math tags are coming from.
  mathString = mathString.replace('</math></math>','</math>');
  mathString = mathString.replace(/<math\s*>/g,'<math xmlns="http://www.w3.org/1998/Math/MathML">', "g");
  // mathString = mathString.replace(/<mi\s*>/,'<mi _moz-math-font-style="italic">', "g");
  return mathString;
}

function newPlotFromText(currentNode, expression, editorElement) {
  try {
    var editor = msiGetEditor(editorElement);
    var doc = editor.contentDocument;
    var graph = new Graph();
    var graphData;
    var DOMGs = currentNode.getElementsByTagName('graphSpec')[0];
    graph.extractGraphAttributes(currentNode);
    var firstplot = graph.plots[0];
    var plottype;
    if (firstplot)
      plottype = firstplot.attributes.PlotType;
    else
      plottype = "rectangular";
    var domParser = new DOMParser();
    var parsedExpr = domParser.parseFromString ("<body>" + expression + "</body>", "text/xml");
    parsedExpr = extractMathContent(parsedExpr.documentElement);
    var checkedExpr = preParsePlotExpression(parsedExpr, plottype, null, editorElement);
    var fixedExpr = GetFixedMath(checkedExpr);

    var plot = new Plot(graph.getDimension(), plottype);
    graph.addPlot(plot);
    plot.element.Expression = dressUpMathString2(fixedExpr);
    plot.attributes["PlotType"] = firstplot.attributes["PlotType"];
    graphData = new graphVarData(graph);

    DOMGs.appendChild(plot.createPlotDOMElement(document, true, graphData));

//    graph.computeQuery(plot);
    graph.recomputeVCamImage(editorElement, currentNode);
    // graph.reviseGraphDOMElement(currentNode, false, editorElement);
    // ensureVCamPreinitForPlot(currentNode, editorElement);
    // nonmodalRecreateGraph(graph, currentNode, editorElement);
  }
  catch (e) {
    var m = e.message;
  }
}
function graphClickEvent(cmdstr, editorElement, element) {
  /**----------------------------------------------------------------------------------*/
  // Handle a mouse double click on a graph image in the document. This is bound in editor.js.
  // This function should be deprecated. It handled <img> elements inside <graph> elements
  try {
    if (!editorElement) editorElement = msiGetActiveEditorElement();
    var selection = msiGetEditor(editorElement).selection;
    if (selection && !element) {
      element = findtagparent(selection.focusNode, "graph");
    }
    else
      element = findtagparent(element, "graph");
    if (element) formatRecreateGraph(element, cmdstr, editorElement);
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
    if (element === null) {
      var selection = msiGetEditor(editorElement).selection;
      if (selection) element = selection.focusNode;
    }
    var graphelement = findtagparent(element, "graph");
    if (graphelement) {
      msidump("SMR found a <graph> element\n");
      // non-modal dialog, the return is immediate
      window.openDialog("chrome://prince/content/computeVcamSettings.xul", "vcamsettings", "chrome,close,titlebar,resizable, dependent", null, graphelement, null, element);
    }
  }
  catch (exc) {
    AlertWithTitle("Error in GraphOverlay.js", "Error in graphObjectClickEvent: " + exc);
  }
  markDocumentChanged(editorElement);
}
function addGraphElementToDocument(DOMGraphNode, siblingNode, editorElement) {
  /** ---------------------------------------------------------------------------------*/
  // insert the xml DOM fragment into the DOM
  if (siblingNode) {
    var idx;
    var parent = siblingNode.parentNode;
    if (!editorElement) editorElement = findEditorElementForDocument(DOMGraphNode.ownerDocument);
    if (!editorElement) editorElement = msiGetActiveEditorElement();
    var editor = msiGetEditor(editorElement);
    for (idx = 0;
    (editor !== null) && idx < parent.childNodes.length && siblingNode !== parent.childNodes[idx]; idx++);
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
  //  msiOpenModelessPropertiesDialog("chrome://prince/content/computeGraphSettings.xul",
  //                     "", "chrome,close,titlebar,dependent", editorElement, commandStr, DOMGraph, extraArgsArray);
  var dlgWindow = openDialog("chrome://prince/content/computeGraphSettings.xul", "plotdialog", "chrome,close,titlebar,resizable,dependent,alwaysraised", editorElement, commandStr, DOMGraph);
  document.getElementById("vcamactive").setAttribute("hidden", "true");
  markDocumentChanged(editorElement);
  return;
}
function nonmodalRecreateGraph(graph, DOMGraph, editorElement) {
  // this gets called only if the user clicked OK from ComputeGraphSettings.xul
  // In theory, graph is the graph object and DOMGraph is the DOM element
  // to be replaced. It is possible that DOMGraph has been removed from the
  // parent (i.e., deleted). No parent, no changes to the picture.
  try {

    graph.reviseGraphDOMElement(DOMGraph, false, editorElement);
  }
  catch (e) {
    dump("ERROR: Recreate Graph failed, line 715 in GraphOverlay.js\n");
  }
}

function isDifferentPlotFile(oldpath, newpath, editorElement) {
  var oldURI = msiURIFromString(msiMakeAbsoluteUrl(oldpath, editorElement));
  var newURI = msiURIFromString(msiMakeAbsoluteUrl(newpath, editorElement));
  return (!oldURI.equals(newURI));
}

function insertGraph(siblingElement, graph, editorElement) {
  /**----------------------------------------------------------------------------------*/
  // compute a graph, create a <graph> element, insert it into DOM after siblingElement
  var filetype = graph.getDefaultValue("filetype");
  // May want to ensure file type is compatible with animated here
  //  var editorElement = null;
  if (!editorElement || editorElement === null) editorElement = findEditorElementForDocument(siblingElement.ownerDocument);
  var longfilename;
  var file;
  var leaf;
  var urlstring = msiGetEditorURL(editorElement);
  var url = msiURIFromString(urlstring);
  var documentfile = msiFileFromFileURL(url);
  var plotfile;
  var docauxdirectory;
  var currdocdirectory;
  var editor;
  var editorDoc;
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
  GetCurrentEngine().clearEngineStrings();  //clear errors before starting a plot
  //  dump("In insertGraph, about to computeGraph.\n");
  graph.computeGraph(editorElement, longfilename);
  graph.setGraphAttribute("ImageFile", "plots/" + leaf);
  var gDomElement = graph.createGraphDOMElement(false);

  addGraphElementToDocument(gDomElement, siblingElement, editorElement);
  var obj = gDomElement.getElementsByTagName("object")[0];
//  obj.setAttribute("data", "plots/"+leaf);  // this shouldn't be necessary. Someone is setting the attribute
  // to the absolute path, which won't work
  var parentWindow = editorElement.ownerDocument.defaultView;
//  parentWindow.ensureVCamPreinitForPlot(gDomElement, editorElement);
  if (!obj.id) {
    editor = msiGetEditor(editorElement);
    editorDoc = editor.document;
    obj.id = findUnusedId(editorDoc, "plot");
    obj.setAttribute("id", obj.id); // BBM: unnecessary??
  }
  saveObj(obj);
  editorElement.focus();
}

function insertNewGraph(math, dimension, plottype, optionalAnimate, editorElement, selection) {
  /**-----------------------------------------------------------------------------------------*/
  // Create the <graph> element and insert into the document following this math element
  // primary entry point
  if (!editorElement) editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);
  var checkedExpr = preParsePlotExpression(math, plottype, selection, editorElement);
  var expr = GetFixedMath(checkedExpr);

  var graph = new Graph();
  var frame = graph.frame;
  var width = GetStringPref("swp.graph.hsize");
  var height = GetStringPref("swp.graph.vsize");
  var unit = GetStringPref("swp.graph.units");
  var XTickCount = GetStringPref("swp.graph.xtickcount");
  var YTickCount = GetStringPref("swp.graph.ytickcount");
  var ZTickCount = GetStringPref("swp.graph.ztickcount");
  var XAxisLabel = GetStringPref("swp.graph.xaxislabel");
  var YAxisLabel = GetStringPref("swp.graph.yaxislabel");
  var ZAxisLabel = GetStringPref("swp.graph.zaxislabel");
  var plotcaptionpref = GetStringPref("swp.graph.captionplacement");
  var frmBorder = GetStringPref("swp.graph.borderwidth");
  var frmHMargin = GetStringPref("swp.graph.hmargin");
  var frmVMargin  = GetStringPref("swp.graph.vmargin");
  var frmPadding = GetStringPref("swp.graph.paddingwidth");
  var frmBGColor = GetStringPref("swp.graph.bgcolor");
  var frmBorderColor = GetStringPref("swp.graph.bordercolor");
  var frmPlacement = GetStringPref("swp.graph.placement");
  var frmFloatLocation_forceHere = GetBoolPref("swp.graph.floatlocation.forcehere");
  var frmFloatLocation_here = GetBoolPref("swp.graph.floatlocation.here");
  var frmFloatLocation_pageFloats = GetBoolPref("swp.graph.floatlocation.pagefloats");
  var frmFloatLocation_topPage = GetBoolPref("swp.graph.floatlocation.toppage");
  var frmFloatLocation_bottomPage = GetBoolPref("swp.graph.floatlocation.bottompage");

//  var frmFloatPlacement = GetStringPref("swp.graph.floatplacement");


  graph.setGraphAttribute("Width", width);
  graph.setGraphAttribute("Height", height);
  graph.setGraphAttribute("Units", unit);
  graph.Dimension = dimension;
  if (XTickCount) graph.setGraphAttribute("XTickCount", XTickCount);
  if (YTickCount) graph.setGraphAttribute("YTickCount", YTickCount);
  if (ZTickCount) graph.setGraphAttribute("ZTickCount", ZTickCount);
  if (XAxisLabel) graph.setGraphAttribute("XAxisLabel", XAxisLabel);
  if (YAxisLabel) graph.setGraphAttribute("YAxisLabel", YAxisLabel);
  if (ZAxisLabel) graph.setGraphAttribute("ZAxisLabel", ZAxisLabel);
  if (plotcaptionpref) graph.setGraphAttribute("CaptionPlace", plotcaptionpref);

  if (plotcaptionpref) frame.setFrameAttribute("plotcaptionpref", plotcaptionpref);
  if (frmBorder) frame.setFrameAttribute("borderw", frmBorder);
  if (frmHMargin) frame.setFrameAttribute("HMargin", frmHMargin);
  if (frmVMargin) frame.setFrameAttribute("VMargin", frmVMargin);
  if (frmPadding) frame.setFrameAttribute("padding", frmPadding);
  if (frmBGColor) frame.setFrameAttribute("BGColor", frmBGColor);
  if (frmBorderColor) frame.setFrameAttribute("borderColor", frmBorderColor);
  if (frmPlacement) frame.setFrameAttribute("placement", frmPlacement);
  var floatLoc = "";
  if (frmPlacement === "floating") {
    if (frmFloatLocation_forceHere) floatLoc += "H";
    if (frmFloatLocation_here) floatLoc += "h";
    if (frmFloatLocation_pageFloats) floatLoc += "p";
    if (frmFloatLocation_topPage) floatLoc += "t";
    if (frmFloatLocation_bottomPage) floatLoc += "b";
    if (floatLoc !== "") frame.setFrameAttribute(floatPlacement, floatLoc);
  }

  // BBM
  // setStyleAttributeOnNode(frame, "padding-top", "1.4em", null);


  var plot = new Plot(dimension, plottype);
  var plotnum = graph.addPlot(plot);
  plot.attributes.PlotStatus = "New";
    plot.element.Expression = dressUpMathString2(expr);
  graph.plotnumber = plotnum;
  if ((arguments.length > 3) && (arguments[3] === true)) {
    plot.attributes.Animate = "true";
  }
  plot.computeQuery();
  insertGraph(math, graph, editorElement);
  setStyleAttribute(math.nextSibling.getElementsByTagName("msiframe")[0], "padding-top", "1.4em", null);
  // if (frame) {
  //   if (frmBorder) frame.setAttribute("borderw", frmBorder);
  //   if (frmHMargin) frame.setAttribute("sidemargin", frmHMargin);
  //   if (frmVMargin) frame.setAttribute("topmargin", frmVMargin);
  //   if (frmPadding) frame.setAttribute("padding", frmPadding);
  //   if (frmBGColor) frame.setAttribute("background-color", frmBGColor);
  //   if (frmBorderColor) frame.setAttribute("border-color", frmBorderColor);
  //   if (frmPlacement) {
  //     frame.setAttribute("placement", frmPlacement);
  //     frame.setAttribute("pos", frmPlacement);
  //   }
  //   if (frmFloatLocation_forceHere !== null && frmFloatLocation_forceHere) frame.setAttribute("placeLocation", frmFloatLocation_forceHere);
  //   if (frmFloatLocation_here !== null && frmFloatLocation_here) frame.setAttribute("placeLocation", frmFloatLocation_here);
  //   if (frmFloatLocation_pageFloats !== null && frmFloatLocation_pageFloats) frame.setAttribute("placeLocation", frmFloatLocation_pageFloats);
  //   if (frmFloatLocation_topPage !== null && frmFloatLocation_topPage) frame.setAttribute("placeLocation", frmFloatLocation_topPage);
  //   if (frmFloatLocation_bottomPage !== null && frmFloatLocation_bottomPage) frame.setAttribute("placeLocation", frmFloatLocation_bottomPage);
  //   if (frmPlacement === "unspecified" && frmFloatPlacement) frame.setAttribute("placement", frmFloatPlacement);
  //   if (plotcaptionpref && plotcaptionpref !== "none") {
  //     frame.setAttribute("captionloc", plotcaptionpref);
  //   }

  //   removeStyleAttributeFamilyOnNode( frame, "margin", null);
  //   removeStyleAttributeFamilyOnNode( frame, "border", null);
  //   removeStyleAttributeFamilyOnNode( frame, "background", null);
  //   removeStyleAttributeFamilyOnNode( frame, "padding", null);
  //   removeStyleAttributeFamilyOnNode( frame, "caption-side", null);
  //   if ((frmHMargin !== null) || (frmVMargin !== null)) {
  //     setStyleAttributeOnNode(frame, "margin", (frmVMargin || "") + " " + (frmHMargin || ""), null);
  //   }
  //   if (frmPlacement === "display") {
  //     setStyleAttributeOnNode(frame, "margin", (frmVMargin || "0") + " auto", null);
  //   }
  //   if (frmBorder)
  //     setStyleAttributeOnNode(frame, "border-width", frmBorder, null);
  //   if (frmBorderColor)
  //     setStyleAttributeOnNode(frame, "border-color", frmBGColor, null);
  //   if (frmBGColor)
  //     setStyleAttributeOnNode(frame, "background-color", frmBGColor, null);
  //   if (frmPadding)
  //     setStyleAttributeOnNode(frame, "padding", frmPadding, null);
  //   if (width !== null) {
  //     setStyleAttributeOnNode(frame, "width", width+unit, null);
  //   }
  //   if (height !== null) {
  //     setStyleAttributeOnNode(frame, "height", height+unit, null);
  //   }
  //   // Handle the caption node
  //   var captionNodes = frame.getElementsByTagName("imagecaption");
  //   var captionNode = null;
  //   if (captionNodes.length > 0) {
  //     captionNode = captionNodes[0];
  //   }
  //   if (plotcaptionpref && (plotcaptionpref !== "none")) {
  //     var tlm;
  //     tlm = editor.tagListManager;
  //     setStyleAttributeOnNode(frame, "caption-side", plotcaptionpref, null);
  //     // if there is no caption node, create an empty one.
  //     if (!captionNode) {
  //        captionNode = editor.createElementWithDefaults("imagecaption");
  //        frame.appendChild(captionNode);
  //     }
  //   } else
  //   {
  //     frame.removeAttribute("captionloc");
  //     removeStyleAttributeFamilyOnNode( frame, "caption-side", null);
  //     if (captionNode)
  //       editor.deleteNode(captionNode);
  //   }
  // }
}

function preParsePlotExpression(math, plotType, selection, editorElement)
{
  var retMath = math;
  var retMathObj;
  if (selection)
  {
    if (!selection.isCollapsed)
    {
      retMathObj = extractSelectedMath(math, selection, editorElement);
      return extractPlottableExpression(retMathObj.mathObj, plotType, retMathObj.focusNode, retMathObj.focusOffset);
    }
    return extractPlottableExpression(retMath, plotType, selection.focusNode, selection.focusOffset);
  }
  return extractPlottableExpression(retMath, plotType);
}

function extractSelectedMath(math, selection, editorElement)
{
  var retMathObj = {};
  var topNode, startOffset, endOffset;
  var theRange = selection.getRangeAt(0);
  var bHasContentBefore = msiNavigationUtils.nodeHasContentBeforeRangeStart(theRange, math);
  var bHasContentAfter = msiNavigationUtils.nodeHasContentAfterRangeEnd(theRange, math);
  if (!bHasContentBefore && !bHasContentAfter)  //the whole math object is in the selection - just return what we"re given
    return { mathObj : math, focusNode : selection.focusNode, focusOffset : selection.focusOffset };
  //Otherwise
  var bFocusAtStart = (msiNavigationUtils.comparePositions(selection.anchorNode, selection.anchorOffset, selection.focusNode, selection.focusOffset) > 0);
  retMathObj.mathObj = CloneTheRange(math, theRange, editorElement);
  retMathObj.focusNode = retMathObj.mathObj;
//  if (bFocusAtStart)
//    retMathObj.focusOffset = 0;
//  else
  retMathObj.focusOffset = retMathObj.mathObj.childNodes.length;
  return retMathObj;
}

function extractMathContent(aNode)
{
  if (msiGetBaseNodeName(aNode) === "math")
    return aNode;
  var mathReturn;
  if (msiNavigationUtils.isMathNode(aNode))
  {
    mathReturn = document.createElementNS(mmlns, "math");
    mathReturn.appendChild(aNode);
    return mathReturn;
  }
  //Look to see if we have a string of math objects (may particularly happen if aNode is a documentFragment)
  var kids = msiNavigationUtils.getSignificantContents(aNode);
  var ii,
      bStarted = false,
      bFinished = false,
      isMath = false;
  for (ii = 0; (ii < kids.length) && (!bStarted || !bFinished); ++ii)
  {
    //if we find a <math> node, we just return the first one
    if (msiGetBaseNodeName(kids[ii]) === "math")
      return kids[ii];

    isMath = msiNavigationUtils.isMathNode(kids[ii]);
    if (!bStarted && isMath)
      mathReturn = document.createElementNS(mmlns, "math");
    bStarted = bStarted || isMath;
    bFinished = bStarted && !isMath;
    if (bStarted && !bFinished)
      mathReturn.appendChild(kids[ii]);
  }
  if (mathReturn)
    return mathReturn;
  //No top-level math, see if anything is nested?
  for (ii = 0; ii < kids.length; ++ii)
  {
    mathReturn = extractMathContent(kids[ii]);
    if (mathReturn)
      return mathReturn;
  }
  return null;
}

function extractPlottableExpression(mathNode, plotType, focusNode, focusOffset)
{
  var currNode;
  var tempNode = mathNode;
  do
  {
    currNode = tempNode;
    if (currNode === focusNode)
      break;
    tempNode = msiNavigationUtils.getSingleWrappedChild(currNode);
    if (!tempNode)
    {
      tempNode = msiNavigationUtils.getSingleSignificantChild(currNode, false);
      if (tempNode && !msiNavigationUtils.isOrdinaryMRow(tempNode))
        tempNode = null;
    }
  } while (tempNode);
  var topFocusNode, topFocusPos;
  if (focusNode)
    topFocusNode = msiNavigationUtils.findTopChildContaining(focusNode, currNode);
  else  //no focus node/offset passed in; we'll look for plottable expression starting from the end
  {
    topFocusNode = currNode;
    focusOffset = currNode.childNodes.length;
  }
  if (!topFocusNode)  //focusNode isn't in our mathNode at all?
  {
    if (msiNavigationUtils.comparePositions(focusNode, focusOffset, currNode, currNode.childNodes.length) > 0)
      topFocusPos = currNode.childNodes.length;
    else
      topFocusPos = 0;
  }
  else if (topFocusNode === currNode)
    topFocusPos = focusOffset;
  else
  {
    topFocusPos = msiNavigationUtils.offsetInParent(topFocusNode);
    if (focusOffset)
      ++topFocusPos;
  }
  var binOps = [];
  for (var ii = 0; ii < currNode.childNodes.length; ++ii)
  {
    if (msiNavigationUtils.isBinaryRelation(currNode.childNodes[ii]))
      binOps.push(ii);
  }
  var okayOps = 0;
  if ((plotType === "implicit") || (plotType === "inequality"))
    okayOps = 1;
  if (binOps.length === okayOps)
    return mathNode;
  var numOps = 0;
  var endPos, startPos;
  for (endPos = topFocusPos; endPos < currNode.childNodes.length; ++endPos)  //We start actually to the right of the focus in this loop
  {
    if (binOps.indexOf(endPos) >= 0)
      ++numOps;
    if (numOps > okayOps)
      break;
  }
  for (startPos = topFocusPos - 1; startPos >= 0; --startPos)
  {
    if (binOps.indexOf(startPos) >= 0)
      ++numOps;
    if (numOps > okayOps)
    {
      ++startPos;
      break;
    }
  }
  if (startPos < 0)
    startPos = 0;
  var newMathNode = document.createElementNS(mmlns, "math");
  for (ii = startPos; ii < endPos; ++ii)
    newMathNode.appendChild(currNode.childNodes[ii].cloneNode(true));
  return newMathNode;
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
function graphSaveVars(varList, plot, animated) {
  var nLast = varList.length - 1;
  while (nLast >= 0 && varList[nLast] === "")
    --nLast;
  if (nLast < 0)
  {
    msidump("In GraphOverlay.js, graphSaveVars(), only empty variables passed in!\n");
    return;
  }

  var varNames = ["XVar", "YVar", "ZVar"];
  if (animated) {
    plot.element.AnimVar = varList[nLast--];
  }
  for (; nLast >= 0; --nLast)
  {
    if (varList[nLast] !== "") {
      plot.element[varNames[nLast]] = varList[nLast];
    }
  }
}
function graphSaveVariablesAndDefaults(graph, plot, retVariables)
{
  var whichVar;
  for (whichVar in retVariables.found)
    plot.element[whichVar] = retVariables.found[whichVar];
  plot.attributes.DefaultedVars = "";
  for (whichVar in retVariables.defaulted)
  {
    plot.element[whichVar] = retVariables.defaulted[whichVar];
    if (plot.attributes.DefaultedVars.length)
      plot.attributes.DefaultedVars += "," + whichVar;
    else
      plot.attributes.DefaultedVars = whichVar;
  }
  if ("axes" in retVariables)
  {
    var currAxesLabels = {};
    var discarded = [];
    var needReplace = [];
    var axisName;
    var nIndex = -1;
    var jj, kk;
    for (jj = 0; jj < graph.graphAxesPropertyNames.length; ++jj)
    {
      axisName = graph.graphAxesPropertyNames[jj];
      currAxesLabels[axisName] = graph.getValue(axisName);
    }
    for (whichVar in retVariables.axes)
    {
      if (currAxesLabels[whichVar] === retVariables.axes[whichVar])
        continue;  //nothing to do
      for (axisName in currAxesLabels)
      {
        if (currAxesLabels[axisName] === retVariables.axes[whichVar])
          needReplace.push(axisName);
      }
      discarded.push(whichVar);
      graph[whichVar] = retVariables.axes[whichVar];
    }
    var bForward, replaceVal;
    for (jj = 0; jj < needReplace.length; ++jj)
    {
      bForward = (needReplace[jj] === graph.graphAxesPropertyNames[0]);
      if (bForward)
        replaceVal = discarded.shift();
      else
        replaceVal = discarded.pop();
      if (replaceVal)
        graph[needReplace[jj]] = currAxesLabels[replaceVal];
    }
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
    // alert("SMR ERROR in GraphOverlay line 932 unknown plot type " + ptype);
    break;
  }
  if ((dim === 3) && (ptype !== "explicitList"))
    nvars++;
  if (animate)
    nvars++;
  return (nvars);
}
function testQuery(domgraph) {
  var graph = new Graph();
  graph.extractGraphAttributes(domgraph);

  var expr = graph.plots[0].element.Expression;
  var expr2 = runFixup(runFixup(expr));
  graph.plots[0].element.Expression = dressUpMathString2(expr2);

  graph.computeQuery(graph.plots[0]);
}
function testQueryGraph(domgraph, editorElement) {
  var graph = new Graph();
  graph.extractGraphAttributes(domgraph);

  var expr = graph.plots[0].element.Expression;
  var expr2 = dressUpMathString2(runFixup(runFixup(expr)));
  graph.plots[0].element.Expression = expr2;
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
  var pt = plot.attributes.PlotType;
  var dim = graph.getDimension();
  var animated = (plot.attributes.Animate === "true");
//  var mathvars = false,
//    commas = false;
  var ii;

  plot.element.OriginalExpression = plot.element.Expression;  //store original form - should display it in dialog
  var fixedOut = dressUpMathString2(runFixup(out)); //runFixup(runFixup(out));
  plot.element.Expression = fixedOut;  //this may be sharpened below, but for now
  var varData = new graphVarData( graph );
  var retVariables;
  // (2) Given a list of potential variables, match them to x,y,z, and t
  try
  {
    retVariables = doAnalyzeVars(varData, plot);
  } catch(ex)
  {msidump("Error in GraphOverlay.js, parseQueryReturn: " + ex + "\n");}
  if (retVariables)
    graphSaveVariablesAndDefaults(graph, plot, retVariables);


  //rwa - following are now done during doAnalyzeVars() - if the plottype is to change, we should know it before
  //      setting the variables!
  // (3) identify explicitList and parametric types
  // (4) try to identify explicit lists
  var plotData = varData.findPlotData(plot);
  plot.attributes.PlotType = pt = plotData.plottype;

  //rwa - the adjustment of polar forms should only take place when building the computation input. We should
  //  leave the user's original form stored. Accordingly, this functionality is now called from plot.createDOMPlotElement
  //  (via a call to plot.adjustExpressionForComputation()).
  // (5) some special handling for polar, spherical, and cylindrical
  //     build an expression that looks like [r,t] for polar, [r,t,p] for spherical and cylindrical
  //     Allow constants. For animations, if there is only 1 var, it's the animation var.

  // (6) add the ilk="enclosed-list" attribute to <mfenced>
  if (fixedOut) {
    if ((pt === "polar") || (pt === "spherical") || (pt === "cylindrical") || (pt === "parametric") || (pt === "gradient") || (pt === "vectorField")) {
      var hasilk = fixedOut.indexOf("ilk=", 0);
      if (hasilk === - 1) {
        var s = fixedOut.indexOf("<mfenced", 0);
        if (s >= 0) {
          fixedOut = fixedOut.slice(0, s) + "<mfenced ilk=\"enclosed-list\" " + fixedOut.slice(s + 8);
        }
      }
      fixedOut = runFixup(fixedOut);
      plot.element.Expression = fixedOut;
    }
    else if (pt === "explicitList")
    {
      plot.element.Expression = fixExplicitList(plotData, dim);
    }
    else if ( (pt === "inequality") && (plotData.expressionAsList().length > 1) )
    {
      plot.element.Expression = fixInequalityList(plotData);
    }
  }
}

function fixExplicitList(aPlotData, dim)
{
  var cellList = aPlotData.expressionAsList();
  var tableInfo, tableNode;
  var rowNode, cellNode, kids, topNode, topName, nrows;
  var mathNode = document.createElementNS(mmlns, "math");
  var bOkay = false;
  if ((cellList.length === 1) && (msiGetBaseNodeName(cellList[0]) === "mtable"))
  {
    tableInfo = atomizeMTable(cellList[0]);
    if (tableInfo.nCols === dim)
    {
      tableNode = cellList[0];
      bOkay = true;
    }
    else
      cellList = tableInfo.cellContentsList;
  }

  if (!bOkay)
  {
    nrows = Math.floor(cellList.length / dim);
    tableNode = document.createElementNS(mmlns, "mtable");
    for (var ii = 0; ii < nrows; ++ii)
    {
      rowNode = document.createElementNS(mmlns, "mtr");
      for (var jj = 0; jj < dim; ++jj)
      {
        cellNode = document.createElementNS(mmlns, "mtd");
        topNode = cellList[(ii * dim) + jj];
        topName = msiGetBaseNodeName(topNode);
        if ( (topName === "math") || (topName === "mrow") )
          kids = msiNavigationUtils.getSignificantContents(topNode);
        else
          kids = [topNode];
        for (var kk = 0; kk < kids.length; ++kk)
        {
          cellNode.appendChild( kids[kk].cloneNode(true) );
        }
        rowNode.appendChild(cellNode);
      }
      tableNode.appendChild(rowNode);
    }
  }
  mathNode.appendChild(tableNode);
  return aPlotData.mPlot.parent.ser.serializeToString(mathNode);
}

function fixInequalityList(plotData)
{
  var exprList = plotData.expressionAsList();
  if (exprList.length <= 1)
    return plotData.mPlot.element.Expression;  //leave it unchanged

  var mathNode = document.createElementNS(mmlns, "math");
  var fenceNode = document.createElementNS(mmlns, "mfenced");
  var elemNode, listNode;
  var jj, kk;
  fenceNode.setAttribute("open", "(");
  fenceNode.setAttribute("close", ")");  //these are the defaults - shouldn't be necessary?
  for (jj = 0; jj < exprList.length; ++jj)
  {
    listNode = exprList[jj];
    if (msiGetBaseNodeName(listNode) === "math")
    {
      if (listNode.childNodes.length > 1)
      {
        elemNode = document.createElementNS(mmlns, "mrow");
        for (kk = 0; kk < listNode.childNodes.length; ++kk)
        {
          elemNode.appendChild(listNode.childNodes[kk].cloneNode(true));
        }
      }
      else
        elemNode = listNode.childNodes[0].cloneNode(true);
      fenceNode.appendChild(elemNode);
    }
    else
      fenceNode.appendChild(listNode.cloneNode(true));
  }
  mathNode.appendChild(fenceNode);
  return plotData.mPlot.parent.ser.serializeToString(mathNode);
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

//Here "expr" is a vector or set expressed as a MathML (fenced?) mrow or as an mfence.
//We return the elements of the set or vector connoted by "expr" as MathML DOM nodes.
//If "bFenced" is set to true, we insist on having opening and closing delimiters and separators, otherwise returning null.
//  (Also if "bFenced" is true, we don't dump the complaining messages - this allows function to be used to test for a list.)
function splitMathMLList(expr, bFenced)
{
  if (!bFenced)
    bFenced = false;  //if this wasn't passed in at all, set to false
  var retArray = [];

  var littleDoc = (new DOMParser()).parseFromString (expr, "text/xml");
  var topNode = littleDoc.documentElement;
  var childName;
  var bTopFound = false;
  while (!bTopFound && (topNode.childNodes.length === 1))
  {
    childName = msiGetBaseNodeName(topNode.childNodes[0]);
    switch(childName)
    {
      case "math":
      case "mrow":
      case "mstyle":
      case "mfenced":
      case "mtable":
        topNode = topNode.childNodes[0];
      break;
      default:
        bTopFound = true;
      break;
    }
  }
  var theNodes = msiNavigationUtils.getSignificantContents(topNode);
  if (theNodes.length === 0) return retArray;
  var topName = msiGetBaseNodeName(topNode);
  if (topName === "mfenced")
    return theNodes;
  else if (topName === "mtable")
    return getRowOrColumnVectorContentsAsList(topNode);
  var nStart = 0;
  var nEnd = theNodes.length - 1;
  var aChild;
  aChild = theNodes[nEnd];
  if (aChild.nodeName !== "mo")
  {
    if (bFenced)
      return [topNode];
    msidump("In GraphOverlay.js, splitMathMLList(), expression closes with non-delimiter: " + content + "\n");
  }
  else
  {
    if ((aChild.getAttribute("form") !== "postfix") && !bFenced)
      msidump("In GraphOverlay.js, splitMathMLList(), last <mo> isn't marked as postfix: " + content + "\n");  //But assume it is anyway
    --nEnd;  //if the last is a closing delimiter, stop before it
  }
  aChild = theNodes[0];
  if (aChild.nodeName !== "mo")
  {
    if (bFenced)
      return [topNode];
    msidump("In GraphOverlay.js, splitMathMLList(), expression opens with non-delimiter: " + content + "\n");
  }
  else
  {
   if ((aChild.getAttribute("form") !== "prefix") && !bFenced)
      msidump("In GraphOverlay.js, splitMathMLList(), opening <mo> isn't marked as prefix: " + content + "\n");  //But assume it is anyway
    ++nStart;  //if the last is a closing delimiter, stop before it
  }

  if (nStart === nEnd)  //Special code dealing with only one element - it may be an <mrow> or such simply containing the elements (though that isn't correct markup, really)
  {
    switch( msiGetBaseNodeName(theNodes[nStart]) )
    {
      case "mrow":
      case "mstyle":
      case "mphantom":
      case "mpadded":
        retArray = removeDelimitersFromNodeList( msiNavigationUtils.getSignificantContents(theNodes[nStart]) );
        if (retArray.length > 1)
        {
          if (msiGetBaseNodeName(retArray[0]) === "mo")
          {
            switch(retArray[0].textContent)
            {
              case "{":
              case "(":
              case "[":
                //don't want to use these contents if they're in a "fence inside a fence"
              break;
              default:
                return retArray;
            }
          }
          else
            return retArray;
        }
      break;
    }
  }

  retArray = removeDelimitersFromNodeList(theNodes.slice(nStart, nEnd+1));
  return retArray;
}

function removeDelimitersFromNodeList(nodeArray, delimiterList)
{
  if (!delimiterList)
    delimiterList = [","];
  var sep = 1;
  var entry = 2;
  var prevWas = sep;
  var retArray = [];

  var aChild;
  for (var nKid = 0; nKid < nodeArray.length; ++nKid)
  {
    aChild = nodeArray[nKid];
    if (aChild.nodeName === "mo")
    {
      var content = TrimString(aChild.textContent);
      if (delimiterList.indexOf(content) >= 0)
        prevWas = sep;
      else if (prevWas === sep)
      {
        msidump("In GraphOverlay.js, splitMathMLList(), found <mo> where expected entry: " + content + "\n");
        retArray.push(aChild);
        prevWas = entry;
      }
      else
        prevWas = sep;
    }
    else if (prevWas === sep)
    {
      prevWas = entry;
      retArray.push(aChild);
    }
  }
  return retArray;
}

function getRowOrColumnVectorContentsAsList(tableNode)
{
  var tableContents = atomizeMTable(tableNode);
  if ( (tableContents.nRows === 1) || (tableContents.nCols === 1) )
    return tableContents.cellContentsList;
  return [tableNode];
}

function atomizeMTable(tableNode)
{
  var retContents = {nRows : 0, nCols : 0, cellContentsList : []};
  if (msiGetBaseNodeName(tableNode) !== "mtable")
    return retContents;
  var ii, jj, nodeName;
  var kids = msiNavigationUtils.getSignificantContents(tableNode);
  var grandkids;
  var nRow = 0;
  var cellList = [];
  var nodeArray = [];

  function flushNodeArray()
  {
    if (nodeArray.length > 0)
    {
      cellList.push(encloseContentsInMathNode(nodeArray, true));
      nodeArray.splice(0, nodeArray.length);
    }
  }
  function flushCellArray()
  {
    if (cellList.length > 0)
    {
      if (retContents.nCols < cellList.length)
        retContents.nCols = cellList.length;
      retContents.cellContentsList = retContents.cellContentsList.concat(cellList);
      cellList.splice(0, cellList.length);
    }
  }
//  function checkNonVector()
//  {
//    if ((nRow === 2) && (retArray.length > 1))  //more than one row and the first row had more than one cell - not a vector
//      return true;
//    if ((nRow > 1) && (cellList.length > 1))  //more than one row and the row has more than one cell - not a vector
//      return true;
//    return false;
//  }

  for (jj = 0; jj < kids.length; ++jj)
  {
    nodeName = msiGetBaseNodeName(kids[jj]);
    if ((cellList.length === 0) && (nodeArray.length === 0) ) //starting a new row, whether <mtr> or not
    {
      ++retContents.nRows;
//      if (checkNonVector())
//        return [tableNode];
    }
    if (nodeName === "mtr")
    {
      flushNodeArray();
      flushCellArray();
      grandkids = msiNavigationUtils.getSignificantContents(kids[jj]);
      for (ii = 0; ii < grandkids.length; ++ii)
      {
        nodeName = msiGetBaseNodeName(grandkids[ii]);
        if (nodeName === "mtd")
        {
          flushNodeArray();
          cellList.push( encloseContentsInMathNode(msiNavigationUtils.getSignificantContents(grandkids[ii]),true) );
        }
        else
          nodeArray.push(grandkids[ii]);
      }
      flushNodeArray();
//      if (checkNonVector())
//        return [tableNode];
      flushCellArray();
    }
    else if (nodeName === "mtd")
    {
      flushNodeArray();
      cellList.push( encloseContentsInMathNode(msiNavigationUtils.getSignificantContents(kids[jj]),true) );
    }
    else
    {
      nodeArray.push(kids[jj]);
    }
  }
  flushNodeArray();
//  if (checkNonVector())
//    return [tableNode];
  flushCellArray();
  return retContents;
}

function encloseContentsInMathNode(childNodes, bCopy)
{
  var mathNode = document.createElementNS(mmlns, "math");
  var theChild;
  for (var jj = 0; jj < childNodes.length; ++jj)
  {
    if (bCopy)
      theChild = childNodes[jj].cloneNode(true);
    else
      theChild = childNodes[jj];
    mathNode.appendChild(theChild);
  }
  return mathNode;
}

function orderMathNodes(node1, node2)
{
  if (node1.textContent < node2.textContent)
    return -1;
  if (node2.textContent < node1.textContent)
    return 1;
  return 0;
}


var plotVarDataBase =
{
  checkPlotType : function()
  {
    if (this.isExplicitList())
      this.plottype = "explicitList";
    else if (this.isParametric())
    {
      switch(this.plottype)
      {
        case "vectorField":
        case "polar":
        case "cylindrical":
        case "spherical":
        case "tube":
        break;
        default:
          if ((Number(this.dim) === 3) && (this.numPlainVariablesFound() < 2))
            this.plottype = "curve";
          else
            this.plottype = "parametric";
        break;
      }
    }
    return this.plottype;
  },
  numPlainVariablesNeeded : function()
  {
    var nVars = plotVarsNeeded(this.dim, this.plottype, false);
    if (this.plottype === "conformal")
      --nVars;
    return nVars;
  },
  numPlainVariablesFound : function()
  {
    var foundVars = this.foundVarList();
    if ((foundVars.length > 0) && this.animated)
      return foundVars.length - 1;
    return foundVars.length;
  },
  plotVarName : function(nWhichVar)
  {
    return Graph.prototype.plotVariablePropertyNames[nWhichVar];
  },
  getVariableByIndex : function(nWhichVar)
  {
    var whichVar = this.plotVarName(nWhichVar);
    return this.mPlot.element[whichVar];
  },
  setVariableByIndex : function(nWhichVar, mathExpr)
  {
    var whichVar = this.plotVarName(nWhichVar);
    this.mPlot.element[whichVar] = mathExpr;
  },
  expressionAsList : function()
  {
    if (!this.mSplitExprList)
      this.mSplitExprList = splitMathMLList(dressUpMathString2(this.mPlot.element.Expression), true);
    return this.mSplitExprList;
  },
  varTeXForm : function(whichVar)
  {
    if (!this.texVars)
      this.texVars = {XVar : "", YVar : "", ZVar : "", AnimVar : ""};
    if (!this.texVars[whichVar].length)
    {
      this.texVars[whichVar] = this.parent.convertXMLFragToSimpleTeX(this.mPlot.element[whichVar]);
//      this.texVars[whichVar] = xmlFragToTeX(this.mPlot.element[whichVar]);
    }
    return this.texVars[whichVar];
  },
  foundVarList : function()
  {
    if (!this.mFoundVars)
    {
      var rawVars = GetCurrentEngine().getVariables(this.mPlot.element.Expression);
      var foundVarList = splitMathMLList(rawVars);
      //special check for returning the empty set symbol
      if ( (foundVarList.length === 1) && (msiGetBaseNodeName(foundVarList[0]) === "mi") &&
        (foundVarList[0].textContent === "\u2205") )
        foundVarList = [];
      foundVarList.sort(orderMathNodes);
      this.mFoundVars = [];
      var topVarNode;
      for (var ii = 0; ii < foundVarList.length; ++ii)
      {
        if (msiGetBaseNodeName(foundVarList[ii]) === "math")
          topVarNode = foundVarList[ii];
        else
        {
          topVarNode = document.createElementNS(mmlns, "math");
          topVarNode.appendChild(foundVarList[ii]);
        }
        this.mFoundVars.push( this.mPlot.parent.ser.serializeToString(topVarNode) );
      }
    }
    return this.mFoundVars;
  },
  foundVarTeXList : function()
  {
    if (!this.mTeXFoundVars || !this.mTeXFoundVars.length)
    {
      var foundVars = this.foundVarList();
      this.mTeXFoundVars = [];
      for (var ii = 0; ii < foundVars.length; ++ii)
        this.mTeXFoundVars.push( this.parent.convertXMLFragToSimpleTeX(foundVars[ii]) );
    }
    return this.mTeXFoundVars;
  },
  isParametric : function()
  {
    return ((this.expressionAsList().length === this.dim) && (this.plottype !== "inequality"));
  },
  isAnimated : function()
  {
    return this.animated;
  },
  isExplicitList : function()
  {
    if (this.foundVarList().length > 1)
      return false;
    if ((this.foundVarList().length > 0) && !this.animated)
      return false;
    if (this.expressionAsList().length === 1)
    {
      if (msiGetBaseNodeName( this.expressionAsList()[0] ) === "mtable")
        return true;
    }
    else
      return true;
    return false;
  },
  varWasFound : function(whichVar)
  {
    return this.mPlot.varNameFromFoundVariable(whichVar);
//    var foundList = this.foundVarTeXList();
//    var texVar = this.varTeXForm(whichVar);
//    for (var jj = 0; jj < foundList.length; ++jj)
//    {
//      if (foundList[jj] === texVar)
//        return true;
//    }
//    return false;
  },
  varIsUserSet : function(whichVar)
  {
    return this.mPlot.isUserSet(whichVar);
  },
  matchVariableTeXForm : function(texVar)
  {
    var varName;
    for (var ii = 0; ii < 4; ++ii)
    {
      varName = this.plotVarName(ii);
      if (this.varTeXForm(varName) === texVar)
        return varName;
    }
    return null;
  }
};

function plotVarData(plot, dim, graphData)
{
  this.mPlot = plot;
  this.parent = graphData;
  this.dim = dim;
  this.animated = (this.mPlot.attributes.Animate === "true");
  this.plottype = plot.attributes.PlotType;
}

plotVarData.prototype = plotVarDataBase;

var graphVarDataBase =
{
  graphAxisName : function(nWhichVar)
  {
    return this.mGraph.graphAxesPropertyNames[nWhichVar];
  },
  axisNameIsUserSet : function(whichVar)
  {
    return this.mGraph.isUserSet(whichVar);
  },
  findPlotData : function(aPlot)
  {
    for (var ii = 0; ii < this.plotData.length; ++ii)
    {
      if (this.plotData[ii].mPlot === aPlot)
      {
        return this.plotData[ii];
      }
    }
    return null;
  },
  plotVarIsUserSet : function(nWhichPlot, whichVar)
  {
    return this.plotData[nWhichPlot].varIsUserSet(whichVar);
  },
  matchAxisToTeXName : function(texName, matchArray)
  {
    var axisName;
    var matchIndex;
    for (var ii = 0; ii < this.dim; ++ii)
    {
      axisName = this.graphAxisName(ii);
      matchIndex = matchArray.indexOf(ii);
      if (matchIndex < 0)  //we're not looking for this one
        continue;
      if (this.mGraph.getValue(axisName) === texName)
        return Graph.prototype.plotVariablePropertyNames[matchIndex];
    }
    return null;
  },
  getAxisNameAsVariable : function(varName)
  {
    var index = Graph.prototype.plotVariablePropertyNames.indexOf(varName);
    var texForm;
    if (index < this.dim)
      texForm = this.mGraph.getValue(this.graphAxisName(index));
    if (texForm)
      return wrapMath(wrapmi(texForm));
    return null;
  },
  getXMLToTeXProcessor : function()
  {
    if (!this.xsltProcessor)
      this.xsltProcessor = setupXMLToTeXProcessor();
    return this.xsltProcessor;
  },
  getXMLToMdProcessor : function()
  {
    if (!this.xsltProcessor)
      this.xsltProcessor = setupXMLToMdProcessor();
    return this.xsltProcessor;
  },
  convertXMLFragToSimpleTeX : function(xmlStr)
  {
    var xsltProcessor = this.getXMLToTeXProcessor();
    var res = processXMLFragWithLoadedStylesheet(xsltProcessor, xmlStr);
    res = res.replace(/\\mathnormal/g, "");
    res = res.replace(/\\ensuremath/g, "");
    res = res.replace(/\\operatorname\*?/g, "");
    var bracesRE = /\{([^\{\}]*)\}/g;
    while (bracesRE.test(res))
      res = res.replace(bracesRE, "$1");
    res = res.replace(/[\$\\]/g,"");
    return res;
  },
  whichAxesTitlesSet : function()
  {
    var retArray = [];
    var axisName, varName;
    for (var ii = 0; ii < this.dim; ++ii)
    {
      axisName = this.graphAxisName(ii);
      if (this.mGraph.isUserSet(axisName))
      {
        retArray.push(axisName);
        continue;
      }
      for (var jj = 0; jj < this.plotData.length; ++jj)
      {
        switch(this.plotData[jj].mPlot.getPlotValue("PlotStatus"))
        {
          case "New":
          case "Deleted":
          case "ERROR":
            continue;  //don't want to do anything in these cases
        }
        varName = this.plotData[jj].matchVariableTeXForm(this.mGraph[axisName]);
        if (varName && this.plotData[jj].varWasFound(varName))
        {
          if (Graph.prototype.plotVariablePropertyNames.indexOf(varName) === ii)
          {
            retArray.push(axisName);
            break;
          }
        }
      }
    }
    return retArray;
  }
};

function graphVarData(graph)
{
  this.mGraph = graph;
  this.dim = graph.getDimension();
  this.animated = graph.isAnimated();
  this.plotData = [];
  for (var ii = 0; ii < graph.plots.length; ++ii)
  {
    if (graph.plots[ii].attributes.PlotStatus !== "Deleted")
      this.plotData.push( new plotVarData(graph.plots[ii], this.dim, this) );
  }
}

graphVarData.prototype = graphVarDataBase;

var varMatchDataBase =
{
  getPlotData : function()
  {
    if (this.whichPlot !== 0)
      return this.graphData.plotData[this.whichPlot - 1];
    return null;
  },
  getPlot : function()
  {
    var pdata = this.getPlotData();
    if (pdata)
      return pdata.mPlot;
    return null;
  },
  userSet : function()
  {
    if (this.whichPlot === 0)
      return this.graphData.axisNameIsUserSet(this.whichVar);
    return this.graphData.plotVarIsUserSet(this.whichPlot - 1, this.whichVar);
  },
  varWasFound : function()
  {
    var pdata = this.getPlotData();
    if (pdata)
      return pdata.varWasFound( this.whichVar );
    return true; //this is at global graph level - matching an axis name
  },
  preferToDefaults : function()
  {
    var pdata = this.getPlotData();
    if (!pdata)  //this is at global graph level - matching an axis name
      return true;
    if (this.whichVar === "AnimVar")
    {
      return this.targetAnimated;
    }
    else if ( Graph.prototype.plotVariablePropertyNames.indexOf(this.whichVar) >= this.targetNumPlainVars)
      return false;
    if (pdata.isParametric())
    {
      if (!this.targetParametric)
        return false;
    }
    else if (this.targetParametric)
      return false;
    var ptype = pdata.plottype;
    if (plotVarsShouldMatchAxes(this.targetPlotType, this.dim))
    {
      if (!plotVarsShouldMatchAxes(ptype, this.dim))
        return false;
    }
    else if (plotVarsShouldMatchAxes(ptype, this.dim))
      return false;

    switch( this.targetPlotType )
    {
      case "cylindrical":
      case "spherical":
        if (ptype === "cylindrical" || ptype === "spherical")
        {
          return (this.whichVar !== "YVar");
        }
        else
          return false;
      break;
    }
    return true;
  }
//  itemIsParametric : function()
//  {
//    if ("expressionParametric" in this)
//      return this.expressionParametric;
//    if (this.whichPlot !=== 0)
//    {
//      var expr = graph.getPlotValue("Expression", this.whichPlot - 1);
//      var exprListNodes = splitMathMLList(expr, false);
//      this.expressionParametric = (exprListNodes && exprListNodes.length === this.dim);
//      return this.expressionParametric;
//    }
//    return false;
//  }
};

function varMatchData(graphVarData, nPlot, nVarMatched, matchingVarName, targPlotType, dim, numPlainVars, animated, parametric)
{
  this.graphData = graphVarData;
  this.dim = dim;
  this.matched = nVarMatched;
  this.whichPlot = nPlot;
  this.whichVar = matchingVarName;
  this.targetAnimated = animated;
  this.targetPlotType = targPlotType;
  this.targetParametric = parametric;
  this.targetNumPlainVars = numPlainVars;
}

varMatchData.prototype = varMatchDataBase;

//New try.
//function doAnalyzeVars(foundVarList, graphData, plot, dim, isAnimated, isParametric)
function doAnalyzeVars(graphVarData, plot)
{
//First find matches with existing params in other plot items, if appropriate.
  var ii, jj, kk;
  var graph = graphVarData.mGraph;
  var ourPlotData = graphVarData.findPlotData(plot);

  var graphIsAnimated = graph.isAnimated();
  var foundVarList = ourPlotData.foundVarList();
  var isParametric = ourPlotData.isParametric();
  var isAnimated = ourPlotData.isAnimated();
  var dim = graphVarData.dim;
  var ourPlotType = ourPlotData.checkPlotType();
  var ourNumPlainVars = ourPlotData.numPlainVariablesNeeded();
  var nVarsFound = foundVarList.length;
  var nVarsAvailable = nVarsFound;
  var nVarsNeeded = -1;

  var matchData = [];
  var numVars, numPlainVars;
//  var nodeRE = /<\s*([^\s]+)\s([^>\/]+)(\/)?\s*>/g;

  var newVarList = {};
  for (ii = 0; ii < ourNumPlainVars; ++ii)
    newVarList[ Graph.prototype.plotVariablePropertyNames[ii] ] = -1;
  if (isAnimated)
    newVarList.AnimVar = -1;

//This should really be the key, then:
  function orderMatches(matchDataA, matchDataB)
  {
    if (matchDataA.preferToDefaults())
    {
      if (!matchDataB.preferToDefaults())
        return -1;
    }
    else if (matchDataB.preferToDefaults())
      return 1;
    if (matchDataA.userSet())
    {
      if (!matchDataB.userSet())
        return -1;
    }
    else if (matchDataB.userSet)
    {
      return 1;
    }
    if (matchDataA.varWasFound())
    {
      if (!matchDataB.varWasFound())
        return -1;
    }
    else if (matchDataB.varWasFound())
    {
      return 1;
    }
    if (matchDataA.targetParametric)
    {
      if (!matchDataB.targetParametric)
        return 1;
    }
    else if (matchDataB.targetParametric)
      return -1;
    if (matchDataA.whichPlot < matchDataB.whichPlot)
      return -1;
    else if (matchDataB.whichPlot < matchDataA.whichPlot)
      return 1;
    if (matchDataA.whichVar < matchDataB.whichVar)
      return -1;
    else if (matchDataB.whichVar < matchDataA.whichVar)
      return 1;
    if (matchDataA.matched < matchDataB.matched)
      return -1;
    else if (matchDataB.matched < matchDataA.matched)
      return 1;
    return 0;  //should never never happen...
  }

  function findPlaceForVariable(nFoundVar, prefVarName, usedFoundVarsList, assignedVars)
  {
    var ll;
    if (usedFoundVarsList.indexOf(nFoundVar) >= 0)
      return null;
    if (assignedVars[prefVarName] < 0)
      return prefVarName;
    for (ll in assignedVars)
    {
      if (ll === prefVarName)
        continue;
      if (assignedVars[ll] < 0)
        return ll;
    }
    return null;
  }

  var plotData;
  var matchingVarName;
  var foundVarsTeX = ourPlotData.foundVarTeXList();

  var axesMatch = plotVarsShouldMatchAxes(ourPlotType, dim);
  if ((!isParametric) && axesMatch)
  {
    for (kk = 0; kk < foundVarsTeX.length; ++kk)
    {
      matchingVarName = graphVarData.matchAxisToTeXName(foundVarsTeX[kk], axesMatch);
      if (matchingVarName)
      {
        matchData.push( new varMatchData(graphVarData, 0, kk, matchingVarName, ourPlotType, dim, ourNumPlainVars, false, isParametric) );
        //We pass "false" for "animated" for the global graph object; there's no axis string associated with animation variable
      }
    }
  }

  for (ii = 0; ii < graphVarData.plotData.length; ++ii)
  {
    if (graphVarData.plotData[ii] === ourPlotData)
      continue;
    plotData = graphVarData.plotData[ii];
//    numPlainVars = plotData.numPlainVars();
//    animated = plotData.isAnimated();
    for (kk = 0; kk < foundVarsTeX.length; ++kk)
    {
      matchingVarName = plotData.matchVariableTeXForm(foundVarsTeX[kk]);
      if (matchingVarName)
      {
        matchData.push( new varMatchData(graphVarData, ii+1, kk, matchingVarName, ourPlotType, dim, ourNumPlainVars, isAnimated, isParametric) );
      }
    }
  }

  matchData.sort(orderMatches);

  var expectedList, expectedTeXList;
  var usedFoundVars = [];
  var varName;
  var expectedVarLists = plot.expectedVariableLists(ourPlotType, dim, isParametric);
  var expectedTeXVarLists = [];
  var animatedVarList = ["t","s","\u03c4"];   //t,s,tau
  var animatedTeXList = [];
  for (ii = 0; isAnimated && (ii < animatedVarList.length); ++ii)
  {
    animatedTeXList.push( graphVarData.convertXMLFragToSimpleTeX( wrapMath(wrapmi(animatedVarList[ii])) ) );
  }
  var nIndex;

  for (ii = 0; (nVarsAvailable > 0) && (ii < matchData.length); ++ii)
  {
    if (!matchData[ii].preferToDefaults())
      break;
    //We split the matches between those "better" than using default variable names and those "worse".
    //When we get to the "worse" ones, break and process the defaults.

    if (usedFoundVars.indexOf(matchData[ii].matched) >= 0)
    {
      matchData[ii].bNoMatch = true;
      continue;
    }
    varName = matchData[ii].whichVar;
    varName = findPlaceForVariable(matchData[ii].matched, varName, usedFoundVars, newVarList);
    if (varName)
    {
      newVarList[varName] = matchData[ii].matched;
      matchData[ii].bUsed = true;
      usedFoundVars.push(matchData[ii].matched);
      --nVarsAvailable;
    }
  }

  //Now try the default ("expected") names
  if (isAnimated && (nVarsAvailable >= 0))
  {
    for (jj = 0; jj < animatedVarList.length; ++jj)
    {
      nIndex = foundVarsTeX.indexOf(animatedVarList[jj]);
      if ((nIndex >= 0) && (usedFoundVars.indexOf(nIndex) < 0))
      {
//        animatedIndex = nIndex;
        usedFoundVars.push(nIndex);
        newVarList.AnimVar = nIndex;
        --nVarsAvailable;
        break;
      }
    }
  }
  var defaultListUsed = -1;
  for (ii = 0; ii < expectedVarLists.length; ++ii)
  {
    expectedTeXList = [];
    expectedList = expectedVarLists[ii];
    for (jj = 0; jj < expectedList.length; ++jj)
      expectedTeXList.push( graphVarData.convertXMLFragToSimpleTeX( wrapMath(wrapmi(expectedList[jj])) ) );
    expectedTeXVarLists.push(expectedTeXList);
  }
  for (ii = 0; (nVarsAvailable > 0) && (ii < expectedVarLists.length); ++ii)
  {
    expectedList = expectedVarLists[ii];
    expectedTeXList = expectedTeXVarLists[ii];
    for (jj = 0; (nVarsAvailable > 0) && (jj < expectedList.length); ++jj)
    {
      nIndex = foundVarsTeX.indexOf(expectedTeXList[jj]);
      if ((nIndex >= 0) && (usedFoundVars.indexOf(nIndex) < 0))
      {
        if (defaultListUsed < 0)
          defaultListUsed = ii;
        varName = Graph.prototype.plotVariablePropertyNames[jj];
        varName = findPlaceForVariable(nIndex, varName, usedFoundVars, newVarList);
        if (varName)
        {
          newVarList[varName] = nIndex;
          usedFoundVars.push(nIndex);
          --nVarsAvailable;
        }
      }
    }
  }
  if (defaultListUsed < 0)
    defaultListUsed = 0;

  //If there are matches left over, resume the loop
  for (; (nVarsAvailable > 0) && (ii < matchData.length); ++ii)
  {
    if (usedFoundVars.indexOf(matchData[ii].matched) >= 0)
    {
      matchData[ii].bNoMatch = true;
      continue;
    }
    varName = matchData[ii].whichVar;
    varName = findPlaceForVariable(matchData[ii].matched, varName, usedFoundVars, newVarList);
    if (varName)
    {
      newVarList[varName] = matchData[ii].matched;
      matchData[ii].bUsed = true;
      usedFoundVars.push(matchData[ii].matched);
      --nVarsAvailable;
    }
  }

  //Apportion the rest of the unused found variables, starting with the animation variable if present
  if (isAnimated && (nVarsAvailable > 0) && (newVarList.AnimVar < 0) )
  {
    for (jj = 0; jj < foundVarsTeX.length; ++jj)
    {
      if (usedFoundVars.indexOf(jj) < 0)
      {
        newVarList.AnimVar = jj;
        usedFoundVars.push(jj);
        --nVarsAvailable;
        break;
      }
    }
  }
  for (ii = 0; (nVarsAvailable > 0) && (ii < ourNumPlainVars); ++ii)
  {
    varName = Graph.prototype.plotVariablePropertyNames[ii];
    if (newVarList[varName] < 0)
    {
      for (jj = 0; jj < foundVarsTeX.length; ++jj)
      {
        if (usedFoundVars.indexOf(jj) < 0)
        {
          newVarList[varName] = jj;
          usedFoundVars.push(jj);
          --nVarsAvailable;
          break;
        }
      }
    }
  }

  //Fill in any holes with default values
  var outList = {found : {}, defaulted : {}};
  for (varName in newVarList)
  {
    if (newVarList[varName] >= 0)
    {
      outList.found[varName] = foundVarList[ newVarList[varName] ];
    }
    else  //never found one - use a default?
    {
      for (jj = 0; jj < matchData.length; ++jj)
      {
        if (matchData.bUsed)
        {
          var aPlot = matchData.getPlot();
          if (!aPlot)
          {
            outList.defaulted[varName] = graphVarData.getAxisNameAsVariable(varName);
            if (outList.defaulted[varName])  //could have returned null
              break;
          }
          if (aPlot && aPlot.varNameFromFoundVariable(varName))
          {
            outList.defaulted[varName] = aPlot.element[varName];
            break;
          }
        }
      }
      if (!outList.defaulted[varName])  //fall back to expected list
      {
        if (varName === "AnimVar")
          outList.defaulted[varName] = wrapMath(wrapmi(animatedVarList[0]));
        else
        {
          nIndex = Graph.prototype.plotVariablePropertyNames.indexOf(varName);
          expectedList = expectedVarLists[defaultListUsed];
          outList.defaulted[varName] = wrapMath(wrapmi(expectedList[nIndex]));
        }
      }
    }
  }

//Now send back axis renaming suggestions:
  if ((!isParametric) && plotVarsShouldMatchAxes(ourPlotType, dim))
  {
    outList.axes = {};
    var alreadySetAxisNames = graphVarData.whichAxesTitlesSet();
    var axisName;
    for (ii = 0; ii < dim; ++ii)
    {
      varName = Graph.prototype.plotVariablePropertyNames[ii];
      axisName = Graph.prototype.graphAxesPropertyNames[ii];
      if ( (alreadySetAxisNames.indexOf(axisName) < 0) && (varName in newVarList) && (newVarList[varName] >= 0) )
        outList.axes[axisName] = foundVarsTeX[newVarList[varName]];
    }
  }

  return outList;
}


//function analyzeVars(variableList, plot, dim, animated, plotExp)
//{
//  var retObj = {plotType: plottype, varList : [], axesList : [], bParametric : false};
//
//  function compareMathNodes(node1, node2)
//  {
//    if (node1.textContent < node2.textContent)
//      return -1;
//    if (node2.textContent < node1.textContent)
//      return 1;
//    return 0;
//  }
//
//  if (variableList.length)
//    variableList.sort(compareMathNodes);
//  var plottype = plot.attributes.PlotType;
//  var textVarList = [];
//  var newVarList = [];
//  var ii, jj, kk;
//  for (ii = 0; ii < variableList.length; ++ii)
//    textVarList.push(variableList[ii].textContent);
//  var expectedVarLists, expectedList, newVarsList;
//  var expComponents;
//  var nIndex;
//  var animatedIndex = -1;
//  var nVarsFound = textVarList.length;
//  var nVarsNeeded = -1;
//  try
//  {
//    expComponents = splitMathMLList(plotExp, true);
//    retObj.bParametric = (expComponents && expComponents.length === dim);
//    expectedVarLists = plot.expectedVariableLists(plottype, dim, retObj.bParametric);
//    nVarsNeeded = expectedVarLists[0].length;
//    if (animated)
//      ++nVarsNeeded;
//    for (ii = 0; ii < nVarsNeeded; ++ii)
//      newVarList.push(-1);
//  }
//  catch(ex) {msidump("Error in GraphOverlay.js, analyzeVars: " + ex + "\n");}
//
//  if (animated)
//  {
//    var animatedVarList = ["t","s","\u03c4"];   //t,s,tau
//    for (jj = 0; jj < animatedVarList.length; ++jj)
//    {
//      nIndex = textVarList.indexOf(animatedVarList[jj]);
//      if (nIndex >= 0)
//      {
//        animatedIndex = nIndex;
//        textVarList[nIndex] = "";
//        --nVarsFound;
//      }
//    }
//  }
//  var nWhichFound = -1;
//  for (ii = 0; (nVarsFound > 0) && (ii < expectedVarLists.length); ++ii)
//  {
//    expectedList = expectedVarLists[ii];
//    for (jj = 0; (nVarsFound > 0) && (jj < expectedList.length); ++jj)
//    {
//      nIndex = textVarList.indexOf(expectedList[jj]);
//      if (nIndex >= 0)
//      {
//        if (nWhichFound < 0)
//          nWhichFound = ii;
//        if (newVarList[jj] < 0)
//          newVarList[jj] = nIndex;
//        else
//        {
//          for (kk = 0; kk < nVarsNeeded; ++kk)
//          {
//            if (newVarList[kk] < 0)
//            {
//              newVarList[kk] = nIndex;
//              break;
//            }
//          }
//        }
//        textVarList[nIndex] = "";
//        --nVarsFound;
//      }
//    }
//  }
//  if (animatedIndex >= 0)
//    newVarList[nVarsNeeded - 1] = animatedIndex;
//  if (nWhichFound < 0)
//    nWhichFound = 0;
//
//  jj = 0;
//  for (ii = 0; (nVarsFound > 0) && (ii < nVarsNeeded); ++ii)
//  {
//    for (; (newVarList[ii] < 0) && (jj < textVarList.length); ++jj)
//    {
//      if (textVarList[jj].length > 0)
//      {
//        newVarList[ii] = jj;
//        textVarList[jj] = "";
//        --nVarsFound;
//      }
//    }
//  }
//  var mathNode;
//  for (ii = 0; ii < variableList.length; ++ii)
//  {
//    if (msiGetBaseNodeName(variableList[ii]) !=== "math")
//    {
//      mathNode = document.createElementNS(mmlns, "math");
//      mathNode.appendChild(variableList[ii]);
//      variableList[ii] = mathNode;
//    }
//  }
//  for (ii = 0; ii < nVarsNeeded; ++ii)
//  {
//    if (newVarList[ii] < 0)
//      retObj.varList.push(wrapMath(wrapmi(expectedVarLists[nWhichFound][ii])));
//    else
//      retObj.varList.push( plot.parent.ser.serializeToString(variableList[newVarList[ii]]) );
//  }
//
//  if (!retObj.bParametric)  //send back a suggestion for the axes:
//  {
//    jj = 0;
//    if ((!variableList.length || (animated && variableList.length < 2)) && (plotExp.indexOf("<mtable>") >= 0))
//      retObj.plotType = "explicitList";
//
//    switch(plottype)
//    {
//      case "rectangular":
//      case "approximateIntegral":
//      case "inequality":
//      case "implicit":
//      case "vectorField":
//      case "gradient":
//      case "ode":
//        jj = retObj.varList.length;
//        if ((jj > 0) && animated)
//          --jj;
//      break;
//    }
//    for (ii = 0; ii < jj; ++ii)
//      retObj.axesList.push(retObj.varList[ii]);
//  }
//  else //it is parametric
//  {
//    switch(plottype)
//    {
//      case "vectorField":
//      case "polar":
//      case "cylindrical":
//      case "spherical":
//      case "tube":
//      break;
//      default:
//        retObj.plotType = "parametric";
//      break;
//    }
//  }
//
//  return retObj;
//}

function matchVarNames(variableList, animated)
{
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

function createPolarExpression(plot)
{
  var plottype = plot.attributes.PlotType;
  var newexp = stripMath(plot.element.Expression);
  var mrowIndex = newexp.indexOf("<mrow");
  var needsMRow = (mrowIndex !== 0);
//  var needsMRow = (!newexp.startsWith("<mrow"));
  if (needsMRow)
    newexp = "<mfenced open=\"(\" close=\")\" separators=\",\"><mrow>" + newexp + "</mrow>";
  else
    newexp = "<mfenced open=\"(\" close=\")\" separators=\",\">" + newexp;
  newexp += stripMath(plot.element.XVar);
  if (plottype !== "polar")
    newexp += stripMath(plot.element.YVar);
  newexp += "</mfenced>";
  return wrapMath(newexp);
}

function createPolarExpr(mathexp, variableList, plottype) {
  var newexp = stripMath(mathexp);
  newexp = "<mrow><mo>[</mo>" + newexp;
  newexp = newexp + "<mo>,</mo>" + wrapmi(variableList[0]);
  if (plottype !== "polar") {
    newexp = newexp + "<mo>,</mo>" + wrapmi(variableList[1]);
  }
  newexp = newexp + "<mo>]</mo></mrow>";
  newexp = wrapMath(newexp);
  return newexp;
}
function newVar(x) {
  return "o" + x;
}

function plotVarsShouldMatchAxes(plotType, dim)
{
  var retArray = [];
  var ii;
  switch(plotType)
  {
    case "rectangular":
    case "ode":
    case "approximateIntegral":
      for (ii = 0; ii < dim-1; ++ii)
        retArray[ii] = ii;  //says the 1st var should match 1st axis, or vars 0 and 1 should match axes 0 and 1 if dim===3
      return retArray;
    case "implicit":
    case "inequality":
    case "gradient":
    case "vectorField":
      for (ii = 0; ii < dim; ++ii)
        retArray[ii] = ii;
      return retArray;
//    case "polar":
//    case "parametric":
//    case "conformal":
//    case "curve":
//    case "cylindrical":
//    case "spherical":
//    case "tube":
//    default:
//    break;
  }
  return null;
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

function convertCartesianToPolar(xval, yval)
{
  var r = Math.sqrt((xval * xval) + (yval * yval));
  var theta = Math.atan2(yval, xval);
  return [r, theta];
}

function convertCartesianToSpherical(xval, yval, zval)
{
  var planeLength = Math.sqrt((xval * xval) + (yval * yval));
  var rho = Math.sqrt((planeLength * planeLength) + (zval * zval));
  if (rho === 0)
    return [0,0,0];
  var theta = Math.atan2(yval, xval);
  var phi = Math.acos(zval/rho);
  return [rho, theta, phi];
}

function convertPolarToCartesian(r, theta)
{
  return [Math.cos(theta) * r, Math.sin(theta) * r];
}

function convertSphericalToCartesian(rho, theta, phi)
{
  var planeLength = Math.sin(phi) * rho;
  var xval = Math.cos(theta) * planeLength;
  var yval = Math.sin(theta) * planeLength;
  var zval = Math.cos(phi) * rho;
  return [xval, yval, zval];
}

var numberRE = /(\-)?([0-9]+)?(\.[0-9]+)?/;  //Or delete the leading minus?
var sciNotationNumberRE = /(\-)?([0-9]+)?(\.[0-9]+)?e(\-?[0-9]+)/i;  //Or delete the leading minus?

function useSignificantDigits(aVal, sigDigits)
{
  var scinotation = sciNotationNumberRE.exec(aVal);
  var number;
  var neg, whole, dec, exp, theDigits, firstPos, jj;
  var retStr = "";
  if (!scinotation || !scinotation[0])
  {
    number = numberRE.exec(aVal);
    if (!number || !number[0])
      return aVal;  //can't do anything with it!?
    neg = (number[1]==="-") ? true : false;
    whole = number[2] ? number[2] : "";
    dec = number[3] ? number[3].substr(1) : ""; //skip the decimal point
    exp = 0;
  }
  else
  {
    neg = (scinotation[1]==="-") ? true : false;
    whole = scinotation[2] ? scinotation[2] : "";
    dec = scinotation[3] ? scinotation[3].substr(1) : ""; //skip the decimal point
    exp = (scinotation[4] && scinotation[4].length) ? Number(scinotation[4]) : 0;
    if (isNaN(exp))
      exp = 0;
  }
  if ((whole.length + dec.length) <= sigDigits)
    return aVal;
  theDigits = whole + dec;
  firstPos = theDigits.search(/[^0]/);
  if (firstPos < 0)
    return "0";
  var bCarry = false;
  if (firstPos + sigDigits < theDigits.length)
  {
    if (Number(theDigits[firstPos + sigDigits]) === 5)
    {
      if ( (firstPos + sigDigits + 1 < theDigits.length) && (theDigits.substr(firstPos + sigDigits + 1).match(/[^0]/)) )
        bCarry = true;
      else
        bCarry = !neg;
    }
    else if (Number(theDigits[firstPos + sigDigits]) > 5)
      bCarry = true;
    jj = 0;
    if (bCarry)
    {
      for (jj = firstPos + sigDigits - 1; bCarry && (jj >= 0); --jj)
      {
        bCarry = (Number(theDigits[jj]) === 9);
        retStr = (bCarry ? "0" : String(Number(theDigits[jj]) + 1) ) + retStr;
      }
      if (bCarry)
        theDigits = "1" + retStr.substr(0,retStr.length - 1);
      else
        theDigits = theDigits.substr(firstPos, jj+1-firstPos) + retStr;
    }
    else
      theDigits = theDigits.substr(firstPos, sigDigits);
  }
  else
    theDigits = theDigits.substr(firstPos);
  exp += whole.length - firstPos - 1;
  neg = (neg ? "-" : "");
  if ( (exp >= sigDigits) || (exp <= -(sigDigits)) )
  {
    if (theDigits.length <= 1)
      theDigits += "0";
    retStr = neg + theDigits[0] + "." + theDigits.substr(1) + "e" + exp;
  }
  else if (exp < 0)
  {
    retStr = neg + "0.";
    for (jj = 1; jj < (-exp); ++jj)
      retStr += "0";
    retStr += theDigits;
  }
  else
  {
    retStr = neg + theDigits.substr(0, exp + 1);
    if (exp + 1 < sigDigits)
      retStr += "." + theDigits.substr(exp+1);
//    for (jj = 0; jj < sigDigits - exp; ++jj)
//      retStr += "0";
  }
  return retStr;
}

function getPlotDefaultValue(dim, plotType, key)
{
  var prefs = Components.classes["@mozilla.org/preferences-service;1"].getService(Components.interfaces.nsIPrefBranch);
  var prefix = ["swp.plot."];
  var prefType, value;
  var prefElement;
  if (dim)
  {
    prefix.unshift("swp.plot." + dim + "d.");
    if (plotType)
      prefix.unshift("swp.plot." + dim + "d." + plotType + ".");
  }
  for (var ii = 0; !value && (ii < prefix.length); ++ii)
  {
    try
    {
      prefType = prefs.getPrefType(prefix[ii] + key);
      if (prefType === prefs.PREF_STRING) {
        value = prefs.getCharPref(prefix[ii] + key);
        prefElement = document.getElementById(prefix[ii] + key);
        if (prefElement && prefElement.hasAttribute('mml'))
          value = unmathify(value);        
      }
      else if (prefType === prefs.PREF_INT)
      {
        value = String(prefs.getIntPref(prefix[ii] + key));
      }
      else if (prefType === prefs.PREF_BOOL)
      {
        value = prefs.getBoolPref(prefix[ii] + key);
        if (value)
          value = "true";
        else
          value = "false";
      }
    } catch(e) {
      throw new MsiException('Getting default for '+key, e.message);
    }
  }
  return value;
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
  //        if (dataString.length === 0) return;  // no point in going on in this case
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
        var trans = Components.classes["@mozillarg/widget/transferable;1"].
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

/* jshint ignore:start */
#endif
/* jshint ignore:end */

