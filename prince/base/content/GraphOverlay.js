Components.utils.import("resource://app/modules/os.jsm");
#include productname.inc
#ifndef PROD_SW
//----------------------------------------------------------------------------------
// an array of DOM <graph> elements currently being reformatted
var currentDOMGs = new Array();

function Graph () {
  // these arrays enumerate the data for a graph. PLOTELEMENTS are in mathml
  // When adding to this list, you must also add to the MathServiceRequest in the compute engine
  // Compute/iCmpIDs.h and Compute/MRequest.cpp::NameToPID()
  // PlotStatus: UI use only. New/Inited/Deleted.
  this.plots            = [];  //an array of plots, see below
  this.modFlag          = {};  //a Boolean for each attribute in GRAPHATTRIBUTES
  this.errStr           = "";
  this.currentDisplayedPlot = -1;
  for (var i = 0; i < this.GRAPHATTRIBUTES.length; i++) {
    this[this.GRAPHATTRIBUTES[i]] = this.getDefaultValue(this.GRAPHATTRIBUTES[i]);
  }
}

Graph.prototype.GRAPHATTRIBUTES  = ["ImageFile",   "XAxisLabel",     "YAxisLabel",   "ZAxisLabel", "Width",
                          "Height",     "PrintAttribute",      
                          "PrintFrame", "Key",            "Name",         "CaptionText","CaptionPlace",
                          "Units",      "AxesType",       "EqualScaling", "EnableTicks","XTickCount",
                          "YTickCount", "AxesTips",       "GridLines",    "BGColor",    "Dimension", 
                          "AxisScale",  "CameraLocationX","CameraLocationY","CameraLocationZ",
                          "FocalPointX","FocalPointX",    "FocalPointZ",  "UpVectorX",  "UpVectorY", 
                          "UpVectorZ",  "ViewingAngle",   "OrthogonalProjection",       "KeepUp", 
                          " "];
  // Add a plot to a graph. Conceptually, a plot is a collection of attribute/value pairs
Graph.prototype.addPlot                = function (plot) {
  this.plots.push(plot);
  plot.parent = this;
  return (this.plots.length - 1);
}
Graph.prototype.deletePlot             = function (plotnum) {
   if (plotnum >= 0 && plotnum < this.plots.length) {
     this.plots.splice[plotnum,1];
   }
   return (this.plots.length);
}
Graph.prototype.getNumPlots            = function ()  { return (this.plots.length); };
Graph.prototype.isModified             = function (x) { return (this.modFlag[x]);};
Graph.prototype.setModified            = function (x) { this.modFlag[x] = true;};
Graph.prototype.ser                    = new XMLSerializer();
Graph.prototype.prompt                 = Components.classes["@mozilla.org/embedcomp/prompt-service;1"].getService(Components.interfaces.nsIPromptService);
  // call the compute engine to create an image
Graph.prototype.computeGraph           = function (editorElement, filename) {
  ComputeCursor(editorElement);
  var str = this.serializeGraph();
  if (this.errStr == "") {
    try {
      var topWin = msiGetTopLevelWindow();
      topWin.msiComputeLogger.Sent4 ("plotfuncCmd", filename, str, "");
      //msiComputeLogger.Sent4 ("plotfuncCmd", filename, "", "");
      var out=GetCurrentEngine().plotfuncCmd (str);
      msiComputeLogger.Received(out);                                                             
    }                                                                                             
    catch (e) {                                                                                    
      alert ("Computation Error", "Compute Graph: " + GetCurrentEngine().getEngineErrors());
      msiComputeLogger.Exception(e);                                                               
    } 
  } else {
    dump (this.errStr);
  }
  RestoreCursor(editorElement);
};
// call the compute engine to guess at graph attributes 
// If plot is not null, then we build a graph with a single plot to send to the engine.
// Otherwise, all the plots that are children of the graph are included
Graph.prototype.computeQuery           = function (plot) {    
  var str = this.serializeGraph (plot);
  var eng = GetCurrentEngine();
  
  try {           
    msiComputeLogger.Sent4 ("plotfuncQuery", "", str, "");
    var out = eng.plotfuncQuery (str);
    msidump ("ComputeQuery plotfuncQuery returns " + out + "\n");
    msiComputeLogger.Received(out); 
    parseQueryReturn (out, this, plot);
    status="Inited";
  }                                                                                             
  catch (e) {                                                                                    
    status = "ERROR";             
    //this.prompt.alert (null, "Computation Error", "Query Graph: " + eng.getEngineErrors());
    dump("Computation Error", "Query Graph: " + eng.getEngineErrors()+"\n");
    msiComputeLogger.Exception(e);                                                               
  } 
  finally {
    if (plot)
      plot.attributes["PlotStatus"] = status;
    else 
      for (var i = 0; i < this.plots.length; i++)   
        this.plots[i].attributes["PlotStatus"] = status;          
  }                                                                                             
  RestoreCursor();                                                                               
} ;
  // return a DOM <graph> node
  // if forComp, prepare the <graph> fragment to pass to the computation engine
  // Otherwise, create one suitable for putting into the document
  // An optional second argument is the number of the one plot to include for query
Graph.prototype.createGraphDOMElement  = function (forComp, optplot) {
  var htmlns="http://www.w3.org/1999/xhtml";
  var DOMGraph = document.createElementNS(htmlns, "graph");
  var DOMGs     = document.createElementNS(htmlns, "graphSpec");
  var DOMPw     = document.createElementNS(htmlns, "plotwrapper");
  init(DOMGraph);

  // set attributes that selection works. I think only one of these is necessary.
  
  DOMPw.setAttribute("msi_resize","true");
  var attributes;
  var att;
  if (this["plotwrapper"])
  { 
    attributes = this["plotwrapper"].getAttributes();
    for (var j=0; j < attributes.length; j++)
    {
      att = attributes[j];
      DOMPw.setAttribute(att.name, att.value);
    }
  }

  // loop through graph attributes and insert them
  var alist;
  if (forComp)
    alist = this.graphCompAttributeList();
  else
    alist = this.graphAttributeList();
  for (var i=0; i<alist.length; i++) {
    var attr = alist[i];
    var value = this.getGraphAttribute(attr);
    var defaultValue = this.getDefaultValue (attr);
    if ((value == "") || (value == null)) {
       value = defaultValue;
    }
    if (forComp || (value != defaultValue)) {
      if ((value != "") && (value != "unspecified")) {
        DOMGs.setAttribute (attr, value);
      }
    }
  }
  // Some plotwrapper attributes get promoted to the GraphSpec
  var value;
  var plotwrapper = this["plotwrapper"];
  if (plotwrapper)
  {
    if (value = plotwrapper.getAttribute("height"))                                                     
      DOMGs.setAttribute("Height") = value;
    if (value = plotwrapper.getAttribute("width"))                                                     
      DOMGs.setAttribute("Width") = value;
    if (value = plotwrapper.getAttribute("units"))                                                     
      DOMGs.setAttribute("Units") = value;
  }
  
  // if the optional plot number was specified, just include one plot
  // otherwise, for each plot, create a <plot> element
  if (optplot) {
    var status = optplot.attributes.PlotStatus;
    if (status == "ERROR") {
      this.errStr = "ERROR, Plot number " + this.plots.indexOf(optplot) + " " + this.errStr;
    } else if (status != "Deleted") {
      DOMGs.appendChild(optplot.createPlotDOMElement(document, forComp, optplot));
    }
  } else {
    var plot;
    for (var i=0; i<this.plots.length; i++) {
      plot = this.plots[i];
      var status = this.plots[i].attributes.PlotStatus;
      if (status == "ERROR") {
        this.errStr = "ERROR, Plot number " + i + " " + this.errStr;
      } else if (status != "Deleted") {
          DOMGs.appendChild(plot.createPlotDOMElement(document, forComp, plot));
      }
    }
  }
  DOMGraph.appendChild(DOMGs);
  var img;
  // put the image file in
  var filetype = this.getDefaultValue ("DefaultFileType");
  dump("SMR file type is " + filetype + "\n");
  if (filetype == "xvz") {
    img    = document.createElementNS(htmlns,"object");
    img.setAttribute("type","application/x-mupad-graphics+gzip"); 
    img.setAttribute("data", this.getGraphAttribute("ImageFile"));
	} else if (filetype == "xvc") {
    img    = document.createElementNS(htmlns,"object");
    img.setAttribute("type","application/x-mupad-graphics+xml"); 
    img.setAttribute("data", this.getGraphAttribute("ImageFile"));
	} else {
	  img    = document.createElementNS(htmlns,"img");
	  img.setAttribute("src", this.getGraphAttribute("ImageFile"));
	}    
  var unitHandler = new UnitHandler();
  unitHandler.initCurrentUnit(this["Units"]);
  var w = unitHandler.getValueStringAs(this["Width"],"px");
  var h = unitHandler.getValueStringAs(this["Height"],"px");
	img.setAttribute("alt", "Generated Plot");
	img.setAttribute("msigraph","true");
	img.setAttribute("width",w);
	img.setAttribute("height",h);

  DOMPw.appendChild(img);
	DOMGraph.appendChild(DOMPw);
  return(DOMGraph);
};
Graph.prototype.reviseGraphDOMElement  = function (DOMgraph, editorElement) 
{
  // like createGraphDOMElement, but doesn't delete or create a new VCam plugin instance
  var editor = msiGetEditor(editorElement);
  var DOMGs     = DOMgraph.getElementsByTagName("graphSpec")[0];
  var DOMPw     = DOMgraph.getElementsByTagName("plotwrapper")[0];
  DOMPw.setAttribute("msi_resize","true");
  var attributes;
  var att;
  if (this["plotwrapper"])
  { 
    attributes = this["plotwrapper"].getAttributes();
    for (var j=0; j < attributes.length; j++)
    {
      att = attributes[j];
      DOMPw.setAttribute(att.name, att.value);
    }
  }

  // loop through graph attributes and insert them
  var alist;

  alist = this.graphAttributeList();
  for (var i=0; i<alist.length; i++) {
    var attr = alist[i];
    var value = this.getGraphAttribute(attr);
    var defaultValue = this.getDefaultValue (attr);
    if ((value == "") || (value == null)) {
       value = defaultValue;
    }
    if (value != defaultValue) {
      if ((value != "") && (value != "unspecified")) {
        DOMGs.setAttribute (attr, value);
      }
    }
  }
  // Some plotwrapper attributes get promoted to the GraphSpec
  var value;
  var plotwrapper = this["plotwrapper"];
  if (plotwrapper)
  {
    if (value = plotwrapper.getAttribute("height"))                                                     
      DOMGs.setAttribute("Height") = value;
    if (value = plotwrapper.getAttribute("width"))                                                     
      DOMGs.setAttribute("Width") = value;
    if (value = plotwrapper.getAttribute("units"))                                                     
      DOMGs.setAttribute("Units") = value;
  }
  
  // if the optional plot number was specified, just include one plot
  // otherwise, for each plot, create a <plot> element
  // Clear out current plots first
  var i;
  var domPlots = DOMgraph.getElementsByTagName("plot");
  for (i=domPlots.length -1; i >= 0; i--) {
    editor.deleteNode(domPlots[i]);
  }
  var plot;
  for (i=0; i<this.plots.length; i++) {
    plot = this.plots[i];
    var status = this.plots[i].attributes.PlotStatus;
    if (status == "ERROR") {
      this.errStr = "ERROR, Plot number " + i + " " + this.errStr;
    } else if (status != "Deleted") {
        DOMGs.appendChild(plot.createPlotDOMElement(document, false, plot));
    }
  }
  var img;
  img = DOMgraph.getElementsByTagName("object")[0];
  img.load(this.getGraphAttribute("ImageFile"));
//  var unitHandler = new UnitHandler();
//  unitHandler.initCurrentUnit(this["Units"]);
//  var w = unitHandler.getValueStringAs(this["Width"],"px");
//  var h = unitHandler.getValueStringAs(this["Height"],"px");
//	img.setAttribute("width",w);
//	img.setAttribute("height",h);
//  doVCamPreInitialize(img,DOMgraph);
   return(DOMgraph);
};
Graph.prototype.extractGraphAttributes = function (DOMGraph) {
  var msins="http://www.sciword.com/namespaces/sciword";
  var DOMGs = DOMGraph.getElementsByTagName("graphSpec");
  var key;
  var value;
  if (DOMGs.length > 0) {
    DOMGs = DOMGs[0];
    for (i=0; i<DOMGs.attributes.length; i++) {
      key = DOMGs.attributes[i].nodeName;
      value = DOMGs.attributes[i].nodeValue;
      this[key] = value;
    }
    var DOMPlots = DOMGraph.getElementsByTagName("plot");
    for (var i = 0; i<DOMPlots.length; i++) {
      var plot = new Plot();
      var plotno = this.addPlot(plot);
      plot.readPlotAttributesFromDOM(DOMPlots[i]);
      plot.attributes.PlotStatus = "Inited";
    }
  }
};
 // get defaults from preference system, or if not there, from hardcoded list
Graph.prototype.getDefaultValue        = function graphGetDefaultGraphValue (key) {
  var prefs = Components.classes["@mozilla.org/preferences-service;1"].getService(Components.interfaces.nsIPrefBranch);
  var keyname = "swp.graph." + key;
  var value;
  if (prefs.getPrefType(keyname) == prefs.PREF_STRING) {
    try {
      value = prefs.getCharPref(keyname);
    }
    catch (e) {
      dump("Preference Error", "Can't find preference for " + keyname +"\n");
//      this.prompt.alert (null, "Preference Error", "Can't find preference for " + keyname);
      alert ("Preference Error", "Can't find preference for " + keyname);
      value = "";
    }
  } else {
    switch (key) {
      default:           value = "";
    }
  }
  return value;
};
Graph.prototype.getGraphAttribute      = function (name) { return (this[name]);};
Graph.prototype.getValue               = function (key) {
  var value = this.getGraphAttribute(key);
  if ((value != null) && (value != "")) {
    return value;
    }
  return (this.getDefaultValue(key));
};
Graph.prototype.graphAttributeList     = function () { return (Graph.prototype.GRAPHATTRIBUTES); };
Graph.prototype.graphCompAttributeList = function () {
  var NA;
  NA = attributeArrayRemove (this.GRAPHATTRIBUTES, "PrintAttribute");
  NA = attributeArrayRemove (NA,  "Placement");
  NA = attributeArrayRemove (NA,  "Offset");
  NA = attributeArrayRemove (NA,  "Float");
  NA = attributeArrayRemove (NA,  "PrintFrame");
  NA = attributeArrayRemove (NA,  "Key");
  NA = attributeArrayRemove (NA,  "Name");
  NA = attributeArrayRemove (NA,  "CaptionText");
  NA = attributeArrayRemove (NA,  "CaptionPlace");

  var dim = this.getGraphAttribute("Dimension");
  if (dim == "2") {
    NA = attributeArrayRemove (NA,  "ZAxisLabel");
    NA = attributeArrayRemove (NA,  "OrientationTiltTurn");
  }
  return NA;
};
Graph.prototype.serializeGraph         = function (optionalplot) {
  var str;
  try {
    str = this.ser.serializeToString(this.createGraphDOMElement(true,optionalplot));
    // strip off the temporary namespace headers ...
    str = str.replace (/<[a-zA-Z0-9]{2}:/g,"<");
    str = str.replace (/<\/[a-zA-Z0-9]{2}:/g,"<\/");
//    // and put in some line breaks so we humans can read the result
//    str = str.replace (/<graph/,"\n<graph");
//    str = str.replace (/<plot/,"\n<plot");
//    str = str.replace (/<\/graph>/,"</graph>\n");
  }
  catch (e) {
    dump("SMR GraphSerialize exception caught\n");
    // this.setPlotAttribute (PlotAttrName ("PlotStatus", plot_no), "ERROR");
    //msiComputeLogger.Exception(e);
  }
  return str;
};
Graph.prototype.setGraphAttribute      = function (name, value) { this[name] = value; };
Graph.prototype.provideDragEnterHandler= function (editorElement, domGraph) {
  var thisGraph = this;
  var os = getOS(window);
  var __editorElement = editorElement;
  var __domGraph = domGraph;
  return function () {
    if (os === "osx")
    {
      var dragService = Components.classes["@mozilla.org/widget/dragservice;1"].getService();
      dragService = dragService.QueryInterface(Components.interfaces.nsIDragService);
      netscape.security.PrivilegeManager.enablePrivilege('UniversalXPConnect');
      var dropData = DNDUtils.getData("text/html", 0);
      var str = dropData.QueryInterface(Components.interfaces.nsISupportsString);
//      dragService.endDragSession(true);
      scheduleNewPlotFromText(__domGraph, str.data, __editorElement);
    }
    return 1;
  }  
};
Graph.prototype.provideDropHandler     = function (editorElement, domGraph) {
  var __domGraph = domGraph;
  var __editorElement = editorElement;
  return function () {
    var dragService = Components.classes["@mozilla.org/widget/dragservice;1"].getService();
    dragService = dragService.QueryInterface(Components.interfaces.nsIDragService);
    netscape.security.PrivilegeManager.enablePrivilege('UniversalXPConnect');
    var dropData = DNDUtils.getData("text/html", 0);
    var str = dropData.QueryInterface(Components.interfaces.nsISupportsString);
    scheduleNewPlotFromText(__domGraph, str.data, __editorElement);
//    dragService.endDragSession(true);
    return 1;
  }  
};
Graph.prototype.recomputeVCamImage     = function (editorElement) {
  var filename = this.getGraphAttribute("ImageFile"); // the old name
  var match = /[a-zA-Z0-9]+\.(xv[cz]$)/.exec(filename);
  var filetype = match[1];
  var newfilename = "plots/" + createUniqueFileName("plot", filetype);
  var longnewfilename = makeRelPathAbsolute(newfilename, editorElement);
  this.setGraphAttribute("ImageFile", longnewfilename); 
  // the filename that computeGraph uses is the one in the graph structure
  this.computeGraph(editorElement);
  this.setGraphAttribute("ImageFile", newfilename); 
//    file.remove(false);
}
  
function scheduleNewPlotFromText(domgraph, expression, editorElement)
{
  var intervalId;
  var __domgraph = domgraph;
  var __editorElement = editorElement;
  var __expression = expression;
  intervalId = setInterval(function () {
    newPlotFromText(__domgraph, __expression, __editorElement);
//    var dragService = Components.classes["@mozilla.org/widget/dragservice;1"].getService();
//    dragService = dragService.QueryInterface(Components.interfaces.nsIDragService);
//    dragService.endDragSession(true);
    clearInterval(intervalId);
  },200);
}

function Plot () {
  this.element = {};
  this.attributes = {};
  this.modFlag = {};
  this.parent = null;
  var attr;
  for (var i = 0; i < this.PLOTATTRIBUTES.length; i++) {
    attr = this.PLOTATTRIBUTES[i];
    this.attributes[attr] = this.getDefaultPlotValue(attr);
    this.modFlag[attr]  = false;
  }
  for (var i = 0; i < this.PLOTELEMENTS.length; i++) {
    attr = this.PLOTELEMENTS[i];
    this.element[attr] = this.getDefaultPlotValue(attr);
    this.modFlag[attr] = false;
  }
}   

Plot.prototype.PLOTATTRIBUTES   = ["PlotStatus", "PlotType",
                         "LineStyle", "PointStyle", "LineThickness", "LineColor",
                         "DiscAdjust", "DirectionalShading", "BaseColor", "SecondaryColor",
								         "PointSymbol", "SurfaceStyle", "IncludePoints",
								         "SurfaceMesh", "CameraLocationX", "CameraLocationY",
								         "CameraLocationZ",	"FontFamily", "IncludeLines",
                         "AISubIntervals", "AIMethod", "AIInfo", "FillPattern",
                         "Animate", "AnimateStart", "AnimateEnd", "AnimateFPS",
                         "AnimateVisBefore", "AnimateVisAfter",
                         "ConfHorizontalPts", "ConfVerticalPts"];
Plot.prototype.PLOTELEMENTS     = ["Expression", "XMax", "XMin", "YMax", "YMin", "ZMax", "ZMin",
                        "XVar", "YVar", "ZVar", "XPts", "YPts", "ZPts", "TubeRadius"];
                        // Plot elements are all MathML expressions.
                        
Plot.prototype.isModified                 = function (x) { return (this.modFlag[x]);};
Plot.prototype.setModified                = function (x) { this.modFlag[x] = true;};
Plot.prototype.createPlotDOMElement       = function (doc, forComp) 
 {
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
        if (value && (value != "") && (value != "unspecified"))
           DOMPlot.setAttribute (attr, value);
//      }
    }
    // do the plot elements as document fragment children of <plot>
    attrs = (forComp) ? this.plotCompElementList() : this.plotElementList();
    for (var i=0; i<attrs.length; i++) {
      attr = attrs[i];
      if (forComp || (this.element[attr])) {
        var DOMEnode = doc.createElement(attr);
        var textval = this.element[attr];
        if ((textval != "") && (textval != "unspecified")) {
          var tNode = (new DOMParser()).parseFromString (textval, "text/xml");
          DOMEnode.appendChild (tNode.documentElement);
          DOMPlot.appendChild (DOMEnode);
        }
      }
    }
    return(DOMPlot);
  }
  return(NULL);
};
Plot.prototype.revisePlotDOMElement       = function (doc, forComp, domPlot) 
 {
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
        if (value && (value != "") && (value != "unspecified"))
           DOMPlot.setAttribute (attr, value);
//      }
    }
    // do the plot elements as document fragment children of <plot>
    attrs = (forComp) ? this.plotCompElementList() : this.plotElementList();
    for (var i=0; i<attrs.length; i++) {
      attr = attrs[i];
      if (forComp || (this.element[attr])) {
        var DOMEnode = doc.createElement(attr);
        var textval = this.element[attr];
        if ((textval != "") && (textval != "unspecified")) {
          var tNode = (new DOMParser()).parseFromString (textval, "text/xml");
          DOMEnode.appendChild (tNode.documentElement);
          DOMPlot.appendChild (DOMEnode);
        }
      }
    }
    return(DOMPlot);
  }
  return(NULL);
};
  // get values from a DOM <plot> node
Plot.prototype.readPlotAttributesFromDOM  = function (DOMPlot) {
  var key;
  var value;
  var attr;
  var j;
  for (j=0; j<DOMPlot.attributes.length; j++) {
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
  while (child)
  {
    key = child.localName;
    if (child.nodeType == Node.ELEMENT_NODE) 
    {
       var mathnode = child.getElementsByTagName("math")[0];
       var serialized = this.parent.ser.serializeToString(mathnode);
       this.element[key] = serialized;
    }
    child = child.nextSibling;
  }
};
  // look up the value key. If not found, look up the default value.
Plot.prototype.getPlotValue               = function (key) 
{
  var value = this[key];
  if ((value != null) && (value != "")) {
    return value;
  }
  var ptype = this.PlotType;
  if ((key == "XPts") || (key == "YPts") || (key == "ZPts")) {
    var dim   = this.parent.getValue ("Dimension");
    if (dim == "2") {
      switch (ptype) {
        case "implicit" :            value = GetNumAsMathML(20); break;
        case "gradient":
        case "vectorField":          value = GetNumAsMathML(10); break;
        case "inequality":           value = GetNumAsMathML(80); break;
        case "conformal":            value = GetNumAsMathML(20); break;
        default:                     value = GetNumAsMathML(400); break;
      }
    } else {
      switch (ptype) {
        case "parametric":
        case "curve" :               value = GetNumAsMathML(80); break;
        case "implicit" :            value = GetNumAsMathML(6); break;
        case "gradient":
        case "vectorField":          value = GetNumAsMathML(6); break;
        case "tube":                 value = GetNumAsMathML(40); break;
        case "approximateIntegral":  value = GetNumAsMathML(400); break;
        default:                     value = GetNumAsMathML(20); break;
      }
    }
    return value;
  }
  return (plotGetDefaultPlotValue (key));
};
  // get defaults from preference system, or if not there, from hardcoded list
Plot.prototype.getDefaultPlotValue        = function (key) {
  var math = '<math xmlns="http://www.w3.org/1998/Math/MathML">';
  var value;
  var prefs = Components.classes["@mozilla.org/preferences-service;1"].getService(Components.interfaces.nsIPrefBranch);
  var keyname = "swp.plot." + key;
  if (prefs.getPrefType(keyname) == prefs.PREF_STRING) {
    value = prefs.getCharPref(keyname);
  }
  else {
    switch (key) {
	  case 'PlotStatus':       value = "New";  // this should never be moved from here
         break;
	  case 'ConfHorizontalPts':     value = "15";
	  case 'ConfVerticalPts':       value = "15";
         break;
//      default:
//        value = math + "<mrow><mi tempinput=\"true\">()</mi></mrow></math>";
    case 'Expression': value = "<math xmlns='http://www.w3.org/1998/Math/MathML'><mi tempinput='true'></mi></math>";

    default: value = "";
    }
  }
  return value;
};
Plot.prototype.plotAttributeList          = function () { return (this.PLOTATTRIBUTES); };
Plot.prototype.plotElementList            = function () { return (this.PLOTELEMENTS); };
Plot.prototype.plotCompAttributeList      = function () {
  var NA;
  var dim = this.parent["Dimension"];
  var ptype = this.attributes["PlotType"];
  var animate = this.attributes["Animate"];
  NA = attributeArrayRemove (this.PLOTATTRIBUTES, "PlotStatus");

  if (ptype != "approximateIntegral") {
    NA = attributeArrayRemove (NA,  "AISubIntervals");
    NA = attributeArrayRemove (NA,  "AIMethod");
    NA = attributeArrayRemove (NA,  "AIInfo");
  }

  if ((ptype !== "approximateIntegral") && (ptype != "inequality")){
    NA = attributeArrayRemove (NA,  "FillPattern");
  }

  if (ptype !== "conformal") {
    NA = attributeArrayRemove (NA,  "ConfHorizontalPts");
    NA = attributeArrayRemove (NA,  "ConfVerticalPts");
  }

  if (dim == 2) {
    NA = attributeArrayRemove (NA,  "DirectionalShading");
    NA = attributeArrayRemove (NA,  "SurfaceStyle");
    NA = attributeArrayRemove (NA,  "SurfaceMesh");
    NA = attributeArrayRemove (NA,  "CameraLocationX");
    NA = attributeArrayRemove (NA,  "CameraLocationY");
    NA = attributeArrayRemove (NA,  "CameraLocationZ");
  } else {
    NA = attributeArrayRemove (NA,  "PointStyle");
    NA = attributeArrayRemove (NA,  "AxisScale");
  }

  if ((ptype != "rectangular") && (ptype != "parametric") && (ptype != "implict")) {
    NA = attributeArrayRemove (NA, "DiscAdjust");
  }

  if (animate != "true") {
    NA = attributeArrayRemove (NA, "AnimateStart");
    NA = attributeArrayRemove (NA, "AnimateEnd");
    NA = attributeArrayRemove (NA, "AnimateFPS");
    NA = attributeArrayRemove (NA, "AnimateVisBefore");
    NA = attributeArrayRemove (NA, "AnimateVisAfter");
  }

  return NA;
};
Plot.prototype.plotCompElementList        = function () {
  var dim     = this.parent["Dimension"];
  var animate = this.attributes["Animate"];
  var ptype   = this.attributes["PlotType"];
  var NA = this.PLOTELEMENTS;

  if (ptype != "tube") {
    NA = attributeArrayRemove (NA, "TubeRadius");
  }

  var nvars = plotVarsNeeded (dim, ptype, animate);

  if (nvars < 2) {
    NA = attributeArrayRemove (NA, "YPts");
    NA = attributeArrayRemove (NA, "YMax");
    NA = attributeArrayRemove (NA, "YMin");
  }

  if (nvars < 3) {
    NA = attributeArrayRemove (NA, "ZPts");
    NA = attributeArrayRemove (NA, "ZMax");
    NA = attributeArrayRemove (NA, "ZMin");
  }

  return NA;
};
Plot.prototype.getPlotAttribute           = function (name) { return (this[name]);};
Plot.prototype.setPlotAttribute           = function (name, value) 
{ this[name] = value;
  this.setModified(name);
};
  // call the compute engine to guess at graph attributes 
Plot.prototype.computeQuery               = function () {
  this.parent.computeQuery(this);
};

function init(graphObj)
{

  var graphController = {
    supportsCommand : function(cmd){ return (cmd == "cmd_paste"); },
    isCommandEnabled : function(cmd){
      if (cmd == "cmd_paste") return true;
      return false;
    },
    doCommand : function(cmd){
      graphObj.addClipboardContents(graphObj.selectedIndex);
    },
    onEvent : function(evt){ }
  };
}

function newPlotFromText(currentNode, expression, editorElement) {
  try {
    var editor = msiGetEditor(editorElement);
    var graph = new Graph();
    graph.extractGraphAttributes(currentNode);
    var plot = new Plot();
    var firstplot = graph.plots[0];
    plot.element["Expression"] = expression;
    plot.attributes["PlotType"]=firstplot.attributes["PlotType"];
    graph.addPlot(plot);
    graph.recomputeVCamImage(editorElement);
    graph.reviseGraphDOMElement(currentNode, editorElement);
//    editor.replaceNode(domGraph, currentNode, currentNode.parentNode);
  }
  catch(e)
  {
    var m = e.message;
  }
}

/**----------------------------------------------------------------------------------*/
// Handle a mouse double click on a graph image in the document. This is bound in editor.js.
// This function should be deprecated. It handled <img> elements inside <graph> elements
function graphClickEvent (cmdstr, editorElement)
{
//  dump("Entering graphClickEvent.\n");
  try
  {
    if (!editorElement)
      editorElement = msiGetActiveEditorElement();
    var selection = msiGetEditor(editorElement).selection;
    if (selection)
    {
      var element = findtagparent(selection.focusNode, "graph");
      if (element)
        formatRecreateGraph(element, cmdstr, editorElement);
    }
  }
  catch(exc) {AlertWithTitle("Error in GraphOverlay.js", "Error in graphClickEvent: " + exc);}
}

/**----------------------------------------------------------------------------------*/
// Handle a mouse double click on a <graph> <object> element. This is bound in msiEditor.js.
function graphObjectClickEvent(cmdstr,element, editorElement)
{
  dump("SMR In graphObjectClickEvent\n");
  try
  { 
    if (element === null)
    {
      var selection = msiGetEditor(editorElement).selection;
      if (selection) element = selection.focusNode;
    }
    var graphelement = findtagparent(element, "graph");
    if (graphelement) {
      dump ("SMR found a <graph> element\n");
      // only open one dialog per graph element
      var graph = new Graph();
      graph.extractGraphAttributes (graphelement);
      // non-modal dialog, the return is immediate
      window.openDialog ("chrome://prince/content/ComputeVcamSettings.xul",
                         "vcamsettings", "chrome,close,titlebar,resizable, dependent", graph, graphelement, currentDOMGs, element);
    }
  }
  catch(exc) {AlertWithTitle("Error in GraphOverlay.js", "Error in graphObjectClickEvent: " + exc);}
}

/** ---------------------------------------------------------------------------------*/
// insert the xml DOM fragment into the DOM
function addGraphElementToDocument(DOMGraphNode, siblingNode, editorElement)
{
  if (siblingNode)
  {
    var idx;
    var parent = siblingNode.parentNode;
    if (!editorElement)
      editorElement = findEditorElementForDocument(element.ownerDocument);
    if (!editorElement)
      editorElement = msiGetActiveEditorElement();
    var editor = msiGetEditor(editorElement);
    for (idx = 0; (editor != null) && idx < parent.childNodes.length && siblingNode!=parent.childNodes[idx]; idx++);
      editor.insertNode(DOMGraphNode, parent, idx+1);
  }
}

/**----------------------------------------------------------------------------------*/
// Arguments: <name>: first characters of returned name
//            <ext>:  file extension
//            (out):  currdir -- an nsIFile for the current document directory
//            (out):  docdir -- an nsIFile containing the document aux files
// Returns <name><date>.<ext>
//  where <path>, relative to the current document directory, is <docdir_files>/plots/
//  There must be a current document directory
//  <date> is the system time from the Date method.

function createUniqueFileName(name, ext) {
// now gin up a unique name  
  var newdate = new Date();
  var fn = name + newdate.getTime() + "." + ext;
  return fn;
}

/**----------------------------------------------------------------------------------*/
// format and recreate a graph and replace the existing one
// DOMGraph is the DOM graph element we are going to replace.
function formatRecreateGraph (DOMGraph, commandStr, editorElement) {
  var graph = new Graph();
  graph.extractGraphAttributes (DOMGraph);
  // non-modal dialog, the return is immediate

  var extraArgsArray = new Array(graph, DOMGraph, currentDOMGs);
//  msiOpenModelessPropertiesDialog("chrome://prince/content/ComputeGraphSettings.xul",
//                     "", "chrome,close,titlebar,dependent", editorElement, commandStr, DOMGraph, extraArgsArray);
var dlgWindow = msiDoModelessPropertiesDialog("chrome://prince/content/ComputeGraphSettings.xul", "Plot dialog", 
  "chrome,close,titlebar,resizable, dependent",
                                                     editorElement, commandStr, DOMGraph, graph, DOMGraph, currentDOMGs);
  return;
}

// this gets called only if the user clicked OK from ComputeGraphSettings.xul
// In theory, graph is the graph object and DOMGraph is the DOM element
// to be replaced. It is possible that DOMGraph has been removed from the
// parent (i.e., deleted). No parent, no changes to the picture.
function nonmodalRecreateGraph (graph, DOMGraph, editorElement) {
  try {
    
    var parent=window.arguments[1].parentNode;
    var editor = msiGetEditor(editorElement);
    var dg = graph.createGraphDOMElement(false);
    insertGraph(DOMGraph, graph, editorElement);
    editor.deleteNode(DOMGraph);
  }
  catch (e) {
    dump("ERROR: Recreate Graph failed, line 715 in GraphOverlay.js\n");
  }
}

function makeRelPathAbsolute(relpath, editorElement)
{
  var longfilename;
  var leaf;
  try
  {
    var documentfile;
    var docauxdirectory;
    var currdocdirectory;
    var urlstring = msiGetEditorURL(editorElement);
    var url = msiURIFromString(urlstring);
    var documentfile = msiFileFromFileURL(url);

    currdocdirectory = documentfile.parent.clone();
    var pathParts = relpath.split("/");
    var i;
    for (i = 0; i < pathParts.length; i++)
    {
      currdocdirectory.append(pathParts[i]);
    }
    longfilename = currdocdirectory.path;
  } catch (e) {
    dump ("Error: "+e+"\n");
  }
  return longfilename;
}


/**----------------------------------------------------------------------------------*/
// compute a graph, create a <graph> element, insert it into DOM after siblingElement
function insertGraph (siblingElement, graph, editorElement) {
  var filetype = graph.getDefaultValue ("DefaultFileType");
  // May want to ensure file type is compatible with animated here
  //  var editorElement = null;
  if (!editorElement || editorElement == null)
    editorElement = findEditorElementForDocument(siblingElement.ownerDocument);
  var longfilename;
  var file;
  var leaf;
  var urlstring = msiGetEditorURL(editorElement);
  var url = msiURIFromString(urlstring);
  var documentfile = msiFileFromFileURL(url);
  var plotfile;
  try
  {
    leaf = createUniqueFileName("plot", filetype);
    var documentfile;
    var docauxdirectory;
    var currdocdirectory;
    var urlstring = msiGetEditorURL(editorElement);
    var url = msiURIFromString(urlstring);
    var documentfile = msiFileFromFileURL(url);
    var plotfile = documentfile.clone();
    var plotfile = plotfile.parent;
    plotfile.append("plots");
    if (!plotfile.exists()) plotfile.create(1, 0755);
    plotfile.append(leaf);
    longfilename = plotfile.path;
    graph.setGraphAttribute("ImageFile", longfilename);
    msiGetEditor(editorElement).setCaretAfterElement (siblingElement);
  } catch (e) {
    dump ("Error: "+e+"\n");
  }
//  dump("In insertGraph, about to computeGraph.\n");
  graph.computeGraph (editorElement, longfilename);
  graph.setGraphAttribute("ImageFile", "plots/"+leaf);
  var gDomElement = graph.createGraphDOMElement(false);

  addGraphElementToDocument (gDomElement, siblingElement, editorElement);
  var obj = gDomElement.getElementsByTagName("object")[0];
  doVCamPreInitialize(obj,graph);
}

/**-----------------------------------------------------------------------------------------*/
// Create the <graph> element and insert into the document following this math element
// primary entry point
function insertNewGraph (math, dimension, plottype, optionalAnimate, editorElement) {
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var expr = runFixup(GetFixedMath(math));
  var graph = new Graph();
  //var prefs = GetPrefs();

  var width = GetStringPref("swp.graph.HSize");
  var height = GetStringPref("swp.graph.VSize");
  var unit = GetStringPref("swp.graph.defaultUnits");

  graph.setGraphAttribute("Width", width);
  graph.setGraphAttribute("Height", height);
  graph.setGraphAttribute("Units", unit);

  var plot = new Plot();
  var plotnum = graph.addPlot (plot);
  plot.attributes["PlotStatus"] = "New";
  plot.element["Expression"] = expr;
  graph["Dimension"] = dimension;
  plot.attributes["PlotType"] = plottype;
  graph["plotnumber"] = plotnum;
  if ((arguments.length > 3) && (arguments[3] == true)) {
    plot.attributes["Animate"] = "true";
  }
  plot.computeQuery();
  insertGraph (math, graph, editorElement);
}

/**----------------------------------------------------------------------------------*/
// regenerate the graph  based on the contents of <graph>
// DOMGraph is the DOM graph element we are going to replace.
// called by the TestGraphScript() method in the debug menu.
function recreateGraph (DOMGraph, editorElement) {
  var graph = new Graph();
  graph.extractGraphAttributes (DOMGraph);
  var parent = DOMGraph.parentNode;
  insertGraph (DOMGraph, graph, editorElement);
  parent.removeChild (DOMGraph);
}

/**----------------------------------------------------------------------------------*/
// wrap this mathml fragment string inside a mathml <math> element
function wrapMath (thing) {
  return ("<math xmlns=\"http://www.w3.org/1998/Math/MathML\">" + thing + "</math>");
}

// wrap this mathml fragment string inside a mathml <mi> element
function wrapmi (thing) {
  return ("<mi>" + thing + "</mi>");
}


/**----------------------------------------------------------------------------------*/

function graphSaveVars (varList, plot) {   
  if (varList[0] != "") {
    plot.element["XVar"] = wrapMath (wrapmi(varList[0]));         
  }  
  if (varList[1] != "") {
    plot.element["YVar"] = wrapMath (wrapmi(varList[1]));         
  }
  if (varList[2] != "") {
    plot.element["ZVar"] = wrapMath (wrapmi(varList[2]));         
  }
}


//==========================================
// return a list of attribute names that correspond to required or optional
// data elements for the given type of graph. The only restrictions here are
// based on the dimensions of the graph.


// return a new array with all the elements of A preserved except element
function attributeArrayRemove (A, element) {
  var NA1, NA2;
  var idx;
  idx = attributeArrayFind (A, element);
  if (idx == -1)
    return A;
  NA1 = A.slice(0,idx);
  NA2 = A.slice(idx+1);
  return (NA1.concat(NA2));
}

// return the index of element in the array A, -1 if not found
// ***** need to fix this 8/4/06 ****
// get some funky error about A[idx] not having properties
function attributeArrayFind (A, element) {
  var idx;
  for (idx = 0; idx < A.length; idx++) {
    if (A[idx] == element) return idx;
  }
  return -1;
}

function plotVarsNeeded (dim, ptype, animate) {
  var nvars = 0;  
  switch (ptype) {
    case "curve":
    case "explicitList":
    case "tube":                                                                     
       nvars = 0;            // gets set to 1 below
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
       alert ("SMR ERROR in GraphOverlay line 932 unknown plot type " + ptype);
//      this.prompt.alert (null, "Computation Error", "Unknown plot type <" + ptype +">");
       break;
  }   
  if (dim == "3") 
    nvars++;
  if (animate == "true")
    nvars++;
  return (nvars);
}  

function testQuery (domgraph) {
  var graph = new Graph();
  graph.extractGraphAttributes(domgraph);

  var expr = graph.plots[0].element["Expression"];
  var expr2 = runFixup(runFixup(expr));
  graph.plots[0].element["Expression"] = expr2;

  graph.computeQuery(graph.plots[0]);
}

function testQueryGraph (domgraph, editorElement) {
  var graph = new Graph();
  graph.extractGraphAttributes(domgraph);

  var expr = graph.plots[0].element["Expression"];
  var expr2 = runFixup(runFixup(expr));
  graph.plots[0].element["Expression"] = expr2;
  graph.computeQuery(plots[0]);

  var parent = domgraph.parentNode;
  insertGraph (domgraph, graph, editorElement);
  parent.removeChild (domgraph);
}


//================================================================================

// The engine has simply run eval(expression) and out is the mathml returned.
// (1) identify the plot parameters by looking for <mi>s and assign them to 
//     the plot variables. Warn if there are problems
// (2) Try to identify the type of plot 
function parseQueryReturn (out, graph, plot) {
  var stack = [];                                                             
  var level = 0;                                                              
  var result;                                                                 
  var variableList = [];                                                     
  var pt       = plot.attributes["PlotType"]; 
  var dim      = plot.parent["Dimension"]; 
  var animated = (plot.attributes["Animate"] === "true");
  var mathvars = false, commas = false;
  
  // (1) try to identify all of the variables 
  //     Variables are indicated by <mi>, but these might also be mathnames of functions
  var count = 0;   
  var index = 0;   
  var start;                                                        
  while ((start = out.indexOf ("<mi", index)) >= 0) {
    mathvars = true;
    start = out.indexOf(">", start);
    var stop  = out.indexOf ("</mi>", start) + 1;  // find </mi>                                              
    index = stop+1;
	var v = out.slice (start+1, stop-1);
    if (nameNotIn (v, stack)) {              
      stack.push (v);          
    }           
  }
  for (var i=0; i<stack.length; i++) {
    if (varNotFun(stack[i])) {
      variableList.push (stack[i]);
    }
  }
  
  // check variables
  var varsNeeded = plotVarsNeeded (dim, pt, animated);
  
  // (2) Given a list of potential variables, match them to x,y,z, and t
  variableList = matchVarNames (variableList, animated);
  if (variableList)
    graphSaveVars (variableList, plot);

  // (3) identify explicitList and parametric types
  // graph.setPlotAttribute (PlotAttrName ("PlotType", plot_no), "explicitList");             
  // Count the number of elements in the returned expression. If it's equal to the
  // dimension, set the plot type to parametric.
  var commalst = out.match (/<mo>,<\/mo>/g);
  if (commalst) {
    commas = true;
    var n = commalst.length + 1;
    // probably only want to do the following for "rectangular" plots
    if ((dim == n) && (pt != "vectorField") & (pt != "polar") && (pt != "cylindrical") && 
        (pt != "spherical") && (pt != "tube")) {
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
    if (!commas) 
    {        // create [fn, xvar]
      if ((!mathvars) || ((animated) && (actualVarCount(variableList) === 1)))
      {
            // first arg is a constant, the rest are new
        var animvar = variableList[0];
        variableList[0] = newVar ("1");
        plot.element["XVar"] = wrapMath (wrapmi(variableList[0]));         
        variableList[1] = newVar ("2");
        plot.element["YVar"] = wrapMath (wrapmi(variableList[1]));         
        if (animated) 
        {
          if (pt == "polar") {
            plot.element["YVar"] = wrapMath (wrapmi(animvar));         
          } else {
            plot.element["ZVar"] = wrapMath (wrapmi(animvar));         
          }
        }
      } 
      out = runFixup (createPolarExpr (out, variableList, pt));
      plot.element["Expression"] = out;             
    }  
  }
  
  // (6) add the ilk="enclosed-list" attribute to <mfenced>
  if (out) { 
    if ((pt == "polar") || (pt == "spherical") || (pt == "cylindrical") ||
        (pt == "parametric") || (pt == "gradient") || (pt == "vectorField")) {
      var hasilk = out.indexOf("ilk=",0);
      if (hasilk == -1) {
        var s = out.indexOf("<mfenced",0);
        if (s>0) {
          out = out.slice (0,s) + "<mfenced ilk=\"enclosed-list\" " + out.slice(s+8); 
        }
      }  
      out = runFixup(out);
      plot.element["Expression"] = out;             
    }          
  }
  
}


// return true unless v is one of the function names
// \u03c0 is the unicode for PI?
function varNotFun (v) {
  var funNames = ["sin", "cos", "tan", "ln", "log", "exp", "PI", "\u03c0"];
  var ret = true;
  for (var i=0; i<funNames.length; i++) {
    if (v == funNames[i]) {
      ret = false;
    }
  }
  return (ret);
}

// return true if vin is not in the list
function nameNotIn (vin, list) {
  var ret = true;
  for (var i=0; i<list.length; i++) {
    if (list[i] == vin) {
      ret = false;
    }
  }  
  return ret;
}  



// Given a list of variable names, assign them to the x, y, z, and animation 
// variables for a plot
function matchVarNames (variableList, animated) {
  var newVarList = ["", "", "", ""];
  var animVar = "";
  var xvar=-1, yvar=-1, zvar=-1;
  
  // If variableList contains "x", "y", or "z", match them to position 0, 1, 
  // or 2 respectively, but only if all of the predecessors are there.
  for (var i=0; i<variableList.length; i++) {
    if ((variableList[i] == "x") || (variableList[i] == "X")) {
       xvar = i;
    } else if ((variableList[i] == "y") || (variableList[i] == "Y")) {
       yvar = i;
    }  else if ((variableList[i] == "z") || (variableList[i] == "Z")) {
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
    for (var i=0; i<variableList.length; i++) {
      if ((variableList[i] == "t") || (variableList[i] == "T")) {
         animVar = variableList[i];
         variableList[i] = "";
      } 
    }  
  }
  // sort the remainder alphabetically
  variableList.sort();
  
  // assign remainder of variables in alphabetical order
  var oldptr, newptr;
  for (oldptr = 0; ((oldptr < variableList.length) && (variableList[oldptr] == "")); oldptr++) ;
  for (newptr = 0; ((newptr < newVarList.length) && (newVarList[newptr] != "")); newptr++) ;
  while ((oldptr < variableList.length) && (newptr < newVarList.length)) {
    if (oldptr < variableList.length && variableList[oldptr] != "")
      newVarList[newptr++] = variableList[oldptr++];
    for ( ; ((oldptr < variableList.length) && (variableList[oldptr] == "")); oldptr++) ;
    for ( ; ((newptr < newVarList.length) && (newVarList[newptr] != "")); newptr++) ;
  }

  // insert animation var if it's there 
  if ((animated) && (animVar != "")) {
     newVarList[newptr] = animVar;
  }
    
  return newVarList;
}
  
// create [fn, var]  
function createPolarExpr (mathexp, variableList, plottype) {
  var newexp = stripMath (mathexp);
  newexp = "<mrow><mo>[</mo>" + newexp;
  newexp = newexp + "<mo>,</mo>" + wrapmi(variableList[0]); 
  if (plottype != "polar") {
    newexp = newexp + "<mo>,</mo>" + wrapmi(variableList[1]); 
  }   
  newexp = newexp + "<mo>]</mo></mrow>";
  newexp = wrapMath (newexp);
  return newexp;
}

/* function createPolarExpr (mathexp, xvar) {                                                  */
/*   var newexp = stripMath (mathexp);                                                         */
/*   newexp = "<mrow><mo>[</mo>" + newexp + "<mo>,</mo>" + wrapmi(xvar) + "<mo>]</mo></mrow>"; */
/*   newexp = wrapMath (newexp);                                                               */
/*   return newexp;                                                                            */
/* }                                                                                           */


function newVar (x) {
  return "o"+x;
}
 

function stripMath (mathexp) {
  var start  = mathexp.indexOf ("<math", 0);
  start      = mathexp.indexOf (">", start) + 1;
  var finish = mathexp.indexOf ("</math", start);
  var newexp = mathexp.slice   (start, finish);
  return newexp;
}


function actualVarCount(variableList) {
  var i=0;
  for (var j=0; j<variableList.length; j++)
    if (variableList[j] != "") 
      i++;
  return i;    
}

var plotObserver = 
{ 
  canHandleMultipleItems: function ()
  {
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
  
  canDrop: function(evt, session)
  {
    return true;
  },
  
  onDrop: function(evt, dropData, session)
  {
    if (session.isDataFlavorSupported("text/html"))
    {
      try {
        var trans = Components.classes["@mozilla.org/widget/transferable;1"].
            createInstance(Components.interfaces.nsITransferable); 
        var value;
        if (!trans) return; 
        var flavour = "text/html";
        trans.addDataFlavor(flavour);
        session.getData(trans,0); 
        var str = new Object();
        var strLength = new Object();
        try
        {
          trans.getTransferData(flavour,str,strLength);
          if (str) str = str.value.QueryInterface(Components.interfaces.nsISupportsString); 
          if (str) value = str.data.substring(0,strLength.value / 2);
        }
        catch (e)
        {
          dump("  "+flavour+" not supported\n\n");
          value = "";
        }
        trans.removeDataFlavor(flavour);
      }
      catch(e)
      {
        msidump(e.message);
      }
    }
  },
  
  onDragOver: function(evt, flavour, session) 
  {
    supported = session.isDataFlavorSupported("text/html");
    // we should look for math in the fragment
    if (supported)
      session.canDrop = true;
  },
  
  getSupportedFlavours: function()
  {
    var flavours = new FlavourSet();
    flavours.appendFlavour("text/html");
    return flavours;
  }
}  

#endif