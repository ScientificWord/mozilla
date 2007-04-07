//----------------------------------------------------------------------------------
// an array of DOM <graph> elements currently being reformatted
var currentDOMGs = new Array();

function Graph () {
  // these arrays enumerate the data for a graph. PLOTELEMENTS are in mathml
  // When adding to this list, you must also add to the MathServiceRequest in the compute engine
  // Compute/iCmpIDs.h and Compute/MRequest.cpp::NameToPID()
  // PlotStatus: UI use only. New/Inited/Deleted.
  this.GRAPHATTRIBUTES  = new Array ("ImageFile", "XAxisLabel", "YAxisLabel", "ZAxisLabel", "Width",
                                     "Height", "PrintAttribute", "Placement", "Offset", "Float",
                                     "PrintFrame", "Key", "Name", "CaptionText", "CaptionPlace",
                                     "Units", "AxesType", "EqualScaling", "EnableTicks",
                                     "XTickCount", "YTickCount", "AxesTips", "GridLines", "BGColor",
									 "Dimension", "AxisScale",
                                     // added
									 "CameraLocationX", "CameraLocationY", "CameraLocationZ",
                                     "FocalPointX", "FocalPointY", "FocalPointZ",
                                     "UpVectorX", "UpVectorY", "UpVectorZ",
                                     "ViewingAngle", "OrthogonalProjection", "KeepUp",
                                     " ");
  this.PLOTATTRIBUTES   = new Array ("PlotStatus", "PlotType",
                                     "LineStyle", "PointStyle", "LineThickness", "LineColor",
                                     "DiscAdjust", "DirectionalShading", "BaseColor", "SecondaryColor",
									 "PointSymbol", "SurfaceStyle", "IncludePoints",
									 "SurfaceMesh", "CameraLocationX", "CameraLocationY",
									 "CameraLocationZ",	"FontFamily", "IncludeLines",
                                     "AISubIntervals", "AIMethod", "AIInfo", "FillPattern",
                                     "Animate", "AnimateStart", "AnimateEnd", "AnimateFPS",
                                     "AnimateVisBefore", "AnimateVisAfter",
                                     "ConfHorizontalPts", "ConfVerticalPts");
  this.PLOTELEMENTS     = new Array ("Expression", "XMax", "XMin", "YMax", "YMin", "ZMax", "ZMin",
                                     "XVar", "YVar", "ZVar", "XPts", "YPts", "ZPts", "TubeRadius");
  this.modFlag          = new Object ();
  this.plotCount        = 0;
  this.errStr           = "";
  this.addPlot          = GraphAddPlot;
  this.deletePlot       = GraphDeletePlot;
  this.getNumPlots      = function ()      { return (this.plotCount); };
  this.isModified       = function (x) { return (this.modFlag[x]);};
  this.setModified      = function (x) { this.modFlag[x] = true;};
  this.ser              = new XMLSerializer();
  this.prompt           = Components.classes["@mozilla.org/embedcomp/prompt-service;1"].getService(Components.interfaces.nsIPromptService);

  for (var i = 0; i < this.GRAPHATTRIBUTES.length; i++) {
    this[this.GRAPHATTRIBUTES[i]] = this.getDefaultValue(this.GRAPHATTRIBUTES[i]);
  }
}

Graph.prototype.computeGraph           = GraphComputeGraph;
Graph.prototype.computeQuery           = GraphComputeQuery;
Graph.prototype.createGraphDOMElement  = GraphMakeDOMGraphElement;
Graph.prototype.createPlotDOMElement   = PlotMakeDOMPlotElement;
Graph.prototype.extractGraphAttributes = GraphReadGraphAttributesFromDOM;
Graph.prototype.extractPlotAttributes  = PlotReadPlotAttributesFromDOM
Graph.prototype.getDefaultValue        = GraphGetDefaultGraphValue;
Graph.prototype.getGraphAttribute      = function (name) { return (this[name]);};
Graph.prototype.getPlotAttribute       = function (name) { return (this[name]);};
Graph.prototype.getPlotValue           = PlotGetPlotValue;
Graph.prototype.getValue               = GraphGetGraphValue;
Graph.prototype.graphAttributeList     = function () { return (this.GRAPHATTRIBUTES); };
Graph.prototype.graphCompAttributeList = GraphSelectCompAttributes;
Graph.prototype.plotAttributeList      = function () { return (this.PLOTATTRIBUTES); };
Graph.prototype.plotElementList        = function () { return (this.PLOTELEMENTS); };
Graph.prototype.plotCompAttributeList  = PlotSelectCompAttributes;
Graph.prototype.plotCompElementList    = PlotSelectCompElements;
Graph.prototype.serializeGraph         = GraphSerializeGraph;
Graph.prototype.setGraphAttribute      = function (name, value) { this[name] = value; };
Graph.prototype.setPlotAttribute       = function (name, value) { this[name] = value;
                                                                  this.setModified(name); };

// Add a plot to a graph. Conceptually, a plot is a collection of attribute/value pairs
function GraphAddPlot () {
  this.plotCount = this.plotCount + 1;
  for (var i = 0; i < this.PLOTATTRIBUTES.length; i++) {
    this[PlotAttrName(this.PLOTATTRIBUTES[i],this.plotCount)] = PlotGetDefaultPlotValue(this.PLOTATTRIBUTES[i]);
    this.modFlag[PlotAttrName(this.PLOTATTRIBUTES[i],this.plotCount)]  = false;
  }
  for (var i = 0; i < this.PLOTELEMENTS.length; i++) {
    this[PlotAttrName(this.PLOTELEMENTS[i],this.plotCount)]   = this.getDefaultValue(this.PLOTELEMENTS[i]);
    this.modFlag[PlotAttrName(this.PLOTELEMENTS[i],this.plotCount)] = false;
  }
  return (this.plotCount);
}

// just decrement the counter and let the GC handle the rest
function GraphDeletePlot () {
   if (this.plotCount > 0) {
     this.plotCount = this.plotCount - 1;
   }
}

// mangle plot attribute and element names so that multiple plots don't collide
// There should be a plot object that contains the elements for a plot, but
// I wasn't able to make JS behave in the expected way. This is a kludge-around.
function PlotAttrName (str, num) {
  return (str + "__" + num);
}


// call the compute engine to create an image
function GraphComputeGraph (editorElement) {
//  dump("Entering GraphComputeGraph.\n");
  var filename = this.getGraphAttribute ("ImageFile");
//  var dumpStr = "In GraphComputeGraph, about to put on ComputeCursor; editorElement is [";
//  if (editorElement)
//    dumpStr += editorElement.id;
//  dumpStr += "].\n";
//  dump(dumpStr);
  ComputeCursor(editorElement);
//  dump("In GraphComputeGraph, about to serializeGraph.\n");
  var str = this.serializeGraph();
//  dump("In GraphComputeGraph, after serializeGraph.\n");
  if (this.errStr == "") {
    try {
      var topWin = msiGetTopLevelWindow();
      topWin.msiComputeLogger.Sent4 ("plotfuncCmd", filename, str, "");
      //msiComputeLogger.Sent4 ("plotfuncCmd", filename, "", "");
      var out=GetCurrentEngine().plotfuncCmd (str);
      msiComputeLogger.Received(out);                                                             
    }                                                                                             
    catch (e) {                                                                                    
//      this.prompt.alert (null, "Computation Error", "Compute Graph: " + GetCurrentEngine().getEngineErrors());
      alert ("Computation Error", "Compute Graph: " + GetCurrentEngine().getEngineErrors());
      dump("Computation Error", "Compute Graph: " + GetCurrentEngine().getEngineErrors() + "\n");
      msiComputeLogger.Exception(e);                                                               
    } 
  } else {
    dump (this.errStr);
  }
  RestoreCursor(editorElement);
}

// call the compute engine to guess at graph attributes 
function GraphComputeQuery (plot_no) {    
  var str = this.serializeGraph (plot_no);
  var eng = GetCurrentEngine();
  
  try {           
    msiComputeLogger.Sent4 ("plotfuncQuery", "", str, "");
    //msiComputeLogger.Sent4 ("plotfuncQuery", "", "", "");

    var out=eng.plotfuncQuery (str);
    dump ("ComputQuery plotfuncQuery returns " + out + "\n");
    msiComputeLogger.Received(out); 
    parseQueryReturn (out, this, plot_no);
    this.setPlotAttribute (PlotAttrName ("PlotStatus", plot_no), "Inited");             
  }                                                                                             
  catch (e) {                                                                                    
    this.setPlotAttribute (PlotAttrName ("PlotStatus", plot_no), "ERROR");             
    //this.prompt.alert (null, "Computation Error", "Query Graph: " + eng.getEngineErrors());
    dump("Computation Error", "Query Graph: " + eng.getEngineErrors()+"\n");
    msiComputeLogger.Exception(e);                                                               
  }                                                                                              
  RestoreCursor();                                                                               
} 


// the optionalplot, if specified, indicates that this <graph> has only one plot
// If any of the plots has an ERROR status, return "ERROR" and a diagnostic
function GraphSerializeGraph (optionalplot) {
  var str;
  try {
    str = this.ser.serializeToString(this.createGraphDOMElement(true,optionalplot));
    // strip off the temporary namespace headers ...
    str = str.replace (/<[a-zA-Z0-9]{2}:/g,"<");
    str = str.replace (/<\/[a-zA-Z0-9]{2}:/g,"<\/");
  }
  catch (e) {
    // dump("SMR GraphSerialize exception caught\n");
    // this.setPlotAttribute (PlotAttrName ("PlotStatus", plot_no), "ERROR");
    msiComputeLogger.Exception(e);
  }
  return str;
}


// get defaults from preference system, or if not there, from hardcoded list
function GraphGetDefaultGraphValue (key) {
  var prefs = Components.classes["@mozilla.org/preferences-service;1"].getService(Components.interfaces.nsIPrefBranch);
  var keyname = "prince.graph." + key;
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
}


// look up the value of obj[key]. If not found, look up the default value.
function GraphGetGraphValue (key) {
  var value = this.getGraphAttribute(key);
  if ((value != null) && (value != "")) {
    return value;
    }
  return (this.getDefaultValue(key));
}


// return a DOM <graph> fragment
// if forComp, prepare the <graph> fragment to pass to the computation engine
// Otherwise, create one suitable for putting into the document
// An optional second argument is the number of the one plot to include for query
function GraphMakeDOMGraphElement (forComp, optplot) {
  var msins="http://www.sciword.com/namespaces/sciword";
  var DOMGraph  = document.createElementNS(msins,"graph");
  var DOMGs     = document.createElementNS(msins,"graphSpec");

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

  // if the optional plot number was specified, just include one plot
  // otherwise, for each plot, create a <plot> element
  if ((arguments.length > 1) && (arguments[1] <= this.getNumPlots())) {
    var status = this.getPlotAttribute (PlotAttrName ("PlotStatus", arguments[1]));
    if (status == "ERROR") {
      this.errStr = "ERROR, Plot number " + arguments[1] + " " + this.errStr;
    } else if (status != "Deleted") {
      DOMGs.appendChild(this.createPlotDOMElement(document, forComp, arguments[1]));
    }
  } else {
    for (var i=1; i<=this.getNumPlots(); i++) {
      var status = this.getPlotAttribute (PlotAttrName ("PlotStatus", i));
      if (status == "ERROR") {
        this.errStr = "ERROR, Plot number " + i + " " + this.errStr;
      } else if (status != "Deleted") {
          DOMGs.appendChild(this.createPlotDOMElement(document, forComp, i));
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
    img.setAttribute("data", "file://"+this.getGraphAttribute("ImageFile"));
    img.setAttribute("alt", "Generated Plot");
    img.setAttribute("msigraph","true");
    img.setAttribute("width","300");
    img.setAttribute("height","400");
  } else {
    img    = document.createElementNS(htmlns,"img");
    img.setAttribute("src", "file://"+this.getGraphAttribute("ImageFile"));
    img.setAttribute("alt", "Generated Plot");
    img.setAttribute("msigraph","true");
  }  
  DOMGraph.appendChild(img);
  return(DOMGraph);
}

// walk down <graphSpec> and extract attributes and elements from existing graph
function GraphReadGraphAttributesFromDOM (DOMGraph) {
  var msins="http://www.sciword.com/namespaces/sciword";
  var DOMGs = DOMGraph.getElementsByTagNameNS(msins, "graphSpec");
  if (DOMGs.length > 0) {
    DOMGs = DOMGs[0];
    for (i=0; i<DOMGs.attributes.length; i++) {
      var key = DOMGs.attributes[i].nodeName;
      var value = DOMGs.attributes[i].nodeValue;
//      dump("Adding key[" + key + "], value[" + value + "] to graph object.\n");  //rwa
      this.setGraphAttribute(key, value);
    }
    var DOMPlots = DOMGraph.getElementsByTagNameNS(msins, "plot");
    var debugStr = "Number of plot children of DOMGraph is [" + DOMPlots.length + "]";  //rwa
    if (DOMPlots.length <= 0)  //rwa
    {             //rwa
      DOMPlots = DOMGs.getElementsByTagNameNS(msins, "plot");   //rwa
      debugStr += "; number of plot children of graphSpec is [" + DOMPlots.length + "]";  //rwa
    }
    window.dump("Setting up graph dialog. " + debugStr + ".\n");  //rwa
    for (var i = 0; i<DOMPlots.length; i++) {
      var plotno = this.addPlot();
      this.extractPlotAttributes(DOMPlots[i], plotno);
      this.setPlotAttribute (PlotAttrName ("PlotStatus", plotno), "Inited");
    }
  }
}

/**----------------------------------------------------------------------------------*/

// get defaults from preference system, or if not there, from hardcoded list
function PlotGetDefaultPlotValue (key) {
  var math = '<math xmlns="http://www.w3.org/1998/Math/MathML">';
  var value;
  var prefs = Components.classes["@mozilla.org/preferences-service;1"].getService(Components.interfaces.nsIPrefBranch);
  var keyname = "prince.plot." + key;
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
      default: value = "";
    }
  }
  return value;
}


// look up the value key. If not found, look up the default value.
function PlotGetPlotValue (key, plotno) {
  var value = this.getPlotAttribute(PlotAttrName(key,plotno));
  if ((value != null) && (value != "")) {
    return value;
  }
  var ptype = this.getPlotAttribute(PlotAttrName("PlotType",plotno));
  if ((key == "XPts") || (key == "YPts") || (key == "ZPts")) {
    var dim   = this.getValue ("Dimension");
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
  return (PlotGetDefaultPlotValue (key));
}


// return a DOM <plot> node. Unless forComp, only include non-default attributes and elements
function PlotMakeDOMPlotElement (doc, forComp, plotno) {
  // do the plot attributes as DOM attributes of <plot>
  var status = this.getPlotValue("PlotStatus",plotno);
  if (status != "Deleted") {
    var msins="http://www.sciword.com/namespaces/sciword";
    var DOMPlot = doc.createElementNS(msins,"plot");
    var attributes = (forComp) ? this.plotCompAttributeList(plotno) : this.plotAttributeList();
    for (var i=0; i<attributes.length; i++) {
      if ((this.isModified(PlotAttrName(attributes[i],plotno))) || forComp) {
        var value = this.getPlotAttribute(PlotAttrName(attributes[i],plotno));
        if ((value != "") && (value != "unspecified"))
           DOMPlot.setAttribute (attributes[i], value);
      }
    }
    // do the plot elements as document fragment children of <plot>
    attributes = (forComp) ? this.plotCompElementList(plotno) : this.plotElementList();
    for (var i=0; i<attributes.length; i++) {
      if (forComp || (this.isModified(PlotAttrName(attributes[i],plotno)))) {
        var DOMEnode = doc.createElementNS(msins,attributes[i]);
        var textval = this.getPlotValue(attributes[i],plotno);
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
}


// get values from a DOM <plot> node
function PlotReadPlotAttributesFromDOM (DOMPlot, plotno) {
  for (var j=0; j<DOMPlot.attributes.length; j++) {
    var key = PlotAttrName(DOMPlot.attributes[j].nodeName, plotno);
    var value = DOMPlot.attributes[j].nodeValue;
    this.setPlotAttribute (key, value);
  }

  // the children of <plot> are in general <plotelement> <mathml...> </plotelement>
  // We should get an ELEMENT_NODE for the <plotelement>, and the localName is the attribute name.
  // The data is either text or mathml. For text, grab the text value, which must be in the nodeValue
  // of the first child. For DOM fragment, store serialization of the fragment.
  var children = DOMPlot.childNodes;
  for (var j=0; j<children.length; j++) {
    var key = PlotAttrName(children[j].localName, plotno);
    if (children[j].nodeType == Node.ELEMENT_NODE) {
      if ((children[j].childNodes.length > 0) && (children[j].childNodes[0].nodeType == Node.TEXT_NODE)) {
         this.setPlotAttribute (key, children[j].childNodes[0].nodeValue);
      }
      else {
//         var ser = new XMLSerializer();
         var serialized = this.ser.serializeToString(children[j].firstChild);
         this.setPlotAttribute (key, serialized);
      }
    }
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
function graphObjectClickEvent ()
{
  dump("SMR In graphObjectClickEvent\n");
  try
  { var editorElement = msiGetActiveEditorElement();
    var selection = msiGetEditor(editorElement).selection;
    if (selection)
    {
      var element = findtagparent(selection.focusNode, "graph");
      if (element) {
        dump ("SMR found a <graph> element\n");
        // only open one dialog per graph element
        if (DOMGListMemberP (element, currentDOMGs)) {
          return;
        }
        DOMGListAdd (element, currentDOMGs);

        var graph = new Graph();
        graph.extractGraphAttributes (element);
        // non-modal dialog, the return is immediate
        window.openDialog ("chrome://prince/content/ComputeVcamSettings.xul",
                           "", "chrome,close,titlebar,dependent", graph, element, currentDOMGs);
      }
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
// Returns <path><name><date>.<ext>
//  where <path> is the current document directory, or if there is no
//  current directory, the mozilla TmpD directory, or if we can't
//  find this, then return empty string
//  <date> is the system time from the Date method.
function createUniqueFileName(name, ext, editorElement) {
  var pathname = "";
  var urlstring = msiGetEditorURL(editorElement);
  var isBlank = (IsUrlAboutBlank(urlstring) || (urlstring == ""));

  // if no current doc path, get the mozilla tmp directory
  if (isBlank) {
    var dp = Components.classes["@mozilla.org/file/directory_service;1"];
    dp = dp.getService(Components.interfaces.nsIDirectoryService);
    dp = dp.QueryInterface(Components.interfaces.nsIProperties);
    pathname = dp.get("TmpD", Components.interfaces.nsIFile);
    pathname = pathname.path + "\/";
  } else {   // if there's a current dir, use it
    pathname = urlstring;
    var lastSlash = pathname.lastIndexOf("\/");
    if (lastSlash != -1) {
      pathname = urlstring.slice(0, lastSlash+1);
    }
    if (pathname.indexOf("file:///") != -1) {
      pathname = pathname.slice(8);
    }
  }

  if (pathname == "") {
    // should do something better here: file select dialog?
    AlertWithTitle(GetComputeString("Error.title"), GetComputeString("Error.notempdir"));
    return "";
  } else {
    var newdate = new Date();
    var fn = name + newdate.getTime() + "." + ext;
    return pathname + fn;
  }
}


/**----------------------------------------------------------------------------------*/
// handle the list operations for preventing multiple dialogs
function DOMGListMemberP (DOMGraph, thelist) {
  for (var i=0; i<thelist.length; i++) {
     if (thelist[i] == DOMGraph)
        return true;
  }
  return false;
}

// Remove this DOMGraph from the current list of DOMGraphs with open dialogs
// Call this for OK or Cancel, any time this dialog closes
function DOMGListRemove (DOMGraph, thelist) {
  for (var i=0; i<thelist.length; i++) {
     if (thelist[i] == DOMGraph) {
         thelist.splice(i,1);
         return;
     }
  }
}

function DOMGListAdd (DOMGraph, thelist) {
  thelist[thelist.length] = DOMGraph;
}

/**----------------------------------------------------------------------------------*/
// format and recreate a graph and replace the existing one
// DOMGraph is the DOM graph element we are going to replace.
function formatRecreateGraph (DOMGraph, commandStr, editorElement) {
  // only open one dialog per DOMGraph element
  if (DOMGListMemberP (DOMGraph, currentDOMGs)) {
    return;
  }
  DOMGListAdd (DOMGraph, currentDOMGs);

  var graph = new Graph();
  graph.extractGraphAttributes (DOMGraph);
  // non-modal dialog, the return is immediate

  var extraArgsArray = new Array(graph, DOMGraph, currentDOMGs);
  msiOpenModelessPropertiesDialog("chrome://prince/content/ComputeGraphSettings.xul",
                     "", "chrome,close,titlebar,dependent", editorElement, commandStr, DOMGraph, extraArgsArray);
//  window.openDialog ("chrome://prince/content/ComputeGraphSettings.xul",
//                     "", "chrome,close,titlebar,dependent", editorElement, DOMGraph, extraArgsArray);
  return;
}

// this gets called only if the user clicked OK from ComputeGraphSettings.xul
// In theory, graph is the graph object and DOMGraph is the DOM element
// to be replaced. It is possible that DOMGraph has been removed from the
// parent (i.e., deleted). No parent, no changes to the picture.
function nonmodalRecreateGraph (graph, DOMGraph, editorElement) {
  try {
    if (DOMGraph) {
      var parent = DOMGraph.parentNode;
      if (parent) {
        // parent.replaceChild() doesn't work. insert new, delete current
        insertGraph (DOMGraph, graph, editorElement);
        parent.removeChild (DOMGraph);
      }
    }
  }
  catch (e) {
    dump("ERROR: Recreate Graph failed, line 433 in GraphOverlay.js\n");
  }
}



/**----------------------------------------------------------------------------------*/
// compute a graph, create a <graph> element, insert it into DOM after siblingElement
function insertGraph (siblingElement, graph, editorElement) {
  var filetype = graph.getDefaultValue ("DefaultFileType");
  // May want to ensure file type is compatible with animated here
//  var editorElement = null;
  if (!editorElement || editorElement == null)
    editorElement = findEditorElementForDocument(siblingElement.ownerDocument);
  try
  {
    var filename = createUniqueFileName("plot", filetype, editorElement);
    graph.setGraphAttribute("ImageFile", filename);
    msiGetEditor(editorElement).setCaretAfterElement (siblingElement);
  } catch (e) {
    dump ("Warning: unable to setCaretAfterElement while inserting graph\n");
  }
//  dump("In insertGraph, about to computeGraph.\n");
  graph.computeGraph (editorElement);
  addGraphElementToDocument (graph.createGraphDOMElement(false), siblingElement, editorElement);
}

/**-----------------------------------------------------------------------------------------*/
// Create the <graph> element and insert into the document following this math element
// primary entry point
function insertNewGraph (math, dimension, plottype, optionalAnimate, editorElement) {
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var expr = runFixup(GetFixedMath(math));
  var graph = new Graph();
  graph.addPlot ();
  graph.setPlotAttribute (PlotAttrName ("PlotStatus",1), "New");
  graph.setPlotAttribute (PlotAttrName ("Expression",1), expr);
  graph.setGraphAttribute ("Dimension", dimension);
  graph.setPlotAttribute (PlotAttrName ("PlotType",1), plottype);
  graph.setGraphAttribute("plotnumber", "1");
  if ((arguments.length > 3) && (arguments[3] == true)) {
    graph.setPlotAttribute (PlotAttrName ("Animate",1), "true");
  }
  graph.computeQuery(1);
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

function graphSaveVars (varList, g, plot_no) {   
  if (varList[0] != "") {
    g.setPlotAttribute (PlotAttrName ("XVar", plot_no), wrapMath (wrapmi(varList[0])));         
  }  
  if (varList[1] != "") {
    g.setPlotAttribute (PlotAttrName ("YVar", plot_no), wrapMath (wrapmi(varList[1])));         
  }
  if (varList[2] != "") {
    g.setPlotAttribute (PlotAttrName ("ZVar", plot_no), wrapMath (wrapmi(varList[2])));         
  }
}


//==========================================
// return a list of attribute names that correspond to required or optional
// data elements for the given type of graph. The only restrictions here are
// based on the dimensions of the graph.
function GraphSelectCompAttributes() {
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
}


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



function PlotSelectCompAttributes (plotno) {
  var NA;
  var dim = this.getGraphAttribute("Dimension");
  var ptype = this.getPlotAttribute(PlotAttrName("PlotType",plotno));
  var animate = this.getPlotAttribute(PlotAttrName("Animate",plotno));
  NA = attributeArrayRemove (this.PLOTATTRIBUTES, "PlotStatus");

  if (ptype != "approximateIntegral") {
    NA = attributeArrayRemove (NA,  "AISubIntervals");
    NA = attributeArrayRemove (NA,  "AIMethod");
    NA = attributeArrayRemove (NA,  "AIInfo");
  }

  if ((ptype != "approximateIntegral") && (ptype != "inequality")){
    NA = attributeArrayRemove (NA,  "FillPattern");
  }

  if (ptype != "conformal") {
    NA = attributeArrayRemove (NA,  "ConfHorizontalPts");
    NA = attributeArrayRemove (NA,  "ConfVerticalPts");
  }

  if (dim == "2") {
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
}

function PlotSelectCompElements (plotno) {
  var dim     = this.getGraphAttribute("Dimension");
  var animate = this.getPlotAttribute(PlotAttrName("Animate",plotno));
  var ptype   = this.getPlotAttribute(PlotAttrName("PlotType",plotno));
  var NA = this.PLOTELEMENTS;

  if (ptype != "tube") {
    NA = attributeArrayRemove (this.PLOTELEMENTS, "TubeRadius");
  }

  var nvars = PlotVarsNeeded (dim, ptype, animate);

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
}


function PlotVarsNeeded (dim, ptype, animate) {
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

  var expr = graph.getPlotAttribute (PlotAttrName ("Expression",1));
  var expr2 = runFixup(runFixup(expr));
  graph.setPlotAttribute (PlotAttrName ("Expression",1), expr2);

  graph.computeQuery(1);
}

function testQueryGraph (domgraph, editorElement) {
  var graph = new Graph();
  graph.extractGraphAttributes(domgraph);

  var expr = graph.getPlotAttribute (PlotAttrName ("Expression",1));
  var expr2 = runFixup(runFixup(expr));
  graph.setPlotAttribute (PlotAttrName ("Expression",1), expr2);
  graph.computeQuery(1);

  var parent = domgraph.parentNode;
  insertGraph (domgraph, graph, editorElement);
  parent.removeChild (domgraph);
}


//================================================================================

// The engine has simply run eval(expression) and out is the mathml returned.
// (1) identify the plot parameters by looking for <mi>s and assign them to 
//     the plot variables. Warn if there are problems
// (2) Try to identify the type of plot 
function parseQueryReturn (out, graph, plot_no) {
  var stack = [];                                                             
  var level = 0;                                                              
  var result;                                                                 
  var variableList = [];                                                     
  var pt       = graph.getPlotAttribute (PlotAttrName ("PlotType", plot_no)); 
  var dim      = graph.getGraphAttribute ("Dimension"); 
  var animated = (graph.getPlotAttribute (PlotAttrName ("Animate", plot_no)) == "true")?true:false;
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
  var varsNeeded = PlotVarsNeeded (dim, pt, animated);
  
  // (2) Given a list of potential variables, match them to x,y,z, and t
  variableList = matchVarNames (variableList, animated);
  if (variableList)
    graphSaveVars (variableList, graph, plot_no);

  // (3) identify explicitList and parametric types
  // graph.setPlotAttribute (PlotAttrName ("PlotType", plot_no), "explicitList");             
  // Count the number of elements in the returned expression. If it's equal to the
  // dimension, set the plot type to parametric.
  var dim      = graph.getGraphAttribute("Dimension");
  var commalst = out.match (/<mo>,<\/mo>/g);
  if (commalst) {
    commas = true;
    var n = commalst.length + 1;
    // probably only want to do the following for "rectangular" plots
    if ((dim == n) && (pt != "vectorField") & (pt != "polar") && (pt != "cylindrical") && 
        (pt != "spherical") && (pt != "tube")) {
      graph.setPlotAttribute (PlotAttrName ("PlotType", plot_no), "parametric");   
    }  
  }  

  // (4) try to identify explicit lists
  if ((!commas) && (!mathvars)) {
    if (out.indexOf("<mtable>") >= 0) {
      graph.setPlotAttribute (PlotAttrName ("PlotType", plot_no), "explicitList");   
    }
  }

  // (5) some special handling for polar, spherical, and cylindrical
  //     build an expression that looks like [r,t] for polar, [r,t,p] for spherical and cylindrical
  //     Allow constants. For animations, if there is only 1 var, it's the animation var.
  if ((pt == "polar") || (pt == "spherical") || (pt == "cylindrical")) {
    if (!commas) {        // create [fn, xvar]
      if ((!mathvars) || ((animated) && (actualVarCount(variableList) == 1))){    // first arg is a constant, the rest are new
        var animvar = variableList[0];
        variableList[0] = newVar ("1");
        graph.setPlotAttribute (PlotAttrName ("XVar", plot_no), wrapMath (wrapmi(variableList[0])));         
        variableList[1] = newVar ("2");
        graph.setPlotAttribute (PlotAttrName ("YVar", plot_no), wrapMath (wrapmi(variableList[1])));         
        if (animated) {
          if (pt == "polar") {
            graph.setPlotAttribute (PlotAttrName ("YVar", plot_no), wrapMath (wrapmi(animvar)));         
          } else {
            graph.setPlotAttribute (PlotAttrName ("ZVar", plot_no), wrapMath (wrapmi(animvar)));         
          }  
        }
      } 
      out = runFixup (createPolarExpr (out, variableList, pt));
      graph.setPlotAttribute (PlotAttrName ("Expression", plot_no), out);             
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
      graph.setPlotAttribute (PlotAttrName ("Expression", plot_no), out);             
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
  var newexp = mathexp.slice   (start, finish-1);
  return newexp;
}


function actualVarCount(variableList) {
  var i=0;
  for (var j=0; j<variableList.length; j++)
    if (variableList[j] != "") 
      i++;
  return i;    
}