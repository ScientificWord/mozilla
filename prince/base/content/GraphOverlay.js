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
function GraphComputeGraph () {
  var filename = this.getGraphAttribute ("ImageFile");
  ComputeCursor();
  var str = this.serializeGraph();
  if (this.errStr == "") {
    try {
      msiComputeLogger.Sent4 ("plotfuncCmd", filename, str, "");
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
  RestoreCursor();
}

// call the compute engine to guess at graph attributes
function GraphComputeQuery (plot_no) {
  var str = this.serializeGraph (plot_no);
  try {
    msiComputeLogger.Sent4 ("plotfuncQuery", "", str, "");
    //msiComputeLogger.Sent4 ("plotfuncQuery", "", "", "");
    var out=GetCurrentEngine().plotfuncQuery (str);
    msiComputeLogger.Received(out);
    parseQueryReturn (out, this, plot_no);
    this.setPlotAttribute (PlotAttrName ("PlotStatus", plot_no), "Inited");             
  }                                                                                             
  catch (e) {                                                                                    
    this.setPlotAttribute (PlotAttrName ("PlotStatus", plot_no), "ERROR");             
//    this.prompt.alert (null, "Computation Error", "Query Graph: " + GetCurrentEngine().getEngineErrors());
    alert ("Computation Error", "Query Graph: " + GetCurrentEngine().getEngineErrors());
    dump("Computation Error", "Query Graph: " + GetCurrentEngine().getEngineErrors()+"\n");
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

  // put the image file in
  var img    = document.createElementNS(htmlns,"img");
  img.setAttribute("src", "file://"+this.getGraphAttribute("ImageFile"));
  img.setAttribute("alt", "Generated Plot");
  img.setAttribute("msigraph","true");
  DOMGraph.appendChild(img);
  return(DOMGraph);
}

// walk down <graphSpec> and extract attributes and elements from existing graph
function GraphReadGraphAttributesFromDOM (DOMGraph) {
  var DOMGs = DOMGraph.getElementsByTagName("graphSpec");
  if (DOMGs.length > 0) {
    DOMGs = DOMGs[0];
    for (i=0; i<DOMGs.attributes.length; i++) {
      var key = DOMGs.attributes[i].nodeName;
      var value = DOMGs.attributes[i].nodeValue;
      this.setGraphAttribute(key, value);
    }
    var DOMPlots = DOMGraph.getElementsByTagName("plot");
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
function graphClickEvent (cmdstr, editorElement)
{
  try
  {
    if (!editorElement)
      editorElement = msiGetActiveEditorElement();
    var selection = msiGetEditor(editorElement).selection;
    if (selection)
    {
      var element = findtagparent(selection.focusNode, "graph");
      if (element)
        formatRecreateGraph(element);
    }
  }
  catch(exc) {AlertWithTitle("Error in GraphOverlay.js", "Error in graphClickEvent: " + exc);}
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
function formatRecreateGraph (DOMGraph) {
  // only open one dialog per DOMGraph element
  if (DOMGListMemberP (DOMGraph, currentDOMGs)) {
    return;
  }
  DOMGListAdd (DOMGraph, currentDOMGs);

  var graph = new Graph();
  graph.extractGraphAttributes (DOMGraph);
  // non-modal dialog, the return is immediate
  window.openDialog ("chrome://editor/content/ComputeGraphSettings.xul",
                     "", "chrome,close,titlebar,dependent", graph, DOMGraph, currentDOMGs);
  return;
}

// this gets called only if the user clicked OK from ComputeGraphSettings.xul
// In theory, graph is the graph object and DOMGraph is the DOM element
// to be replaced. It is possible that DOMGraph has been removed from the
// parent (i.e., deleted). No parent, no changes to the picture.
function nonmodalRecreateGraph (graph, DOMGraph) {
  try {
    if (DOMGraph) {
      var parent = DOMGraph.parentNode;
      if (parent) {
        // parent.replaceChild() doesn't work. insert new, delete current
        insertGraph (DOMGraph, graph);
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
function insertGraph (siblingElement, graph) {
  var filetype = graph.getDefaultValue ("DefaultFileType");
  // need to walk down plot tree and see if any of the plots are animated ...
  if (graph.getPlotAttribute(PlotAttrName("Animate",1)) == "true")
     filetype = "gif";
  var editorElement = null;
  try
  {
    editorElement = findEditorElementForDocument(siblingElement.ownerDocument);
    var filename = createUniqueFileName("plot", filetype, editorElement);
    graph.setGraphAttribute("ImageFile", filename);
    msiGetEditor(editorElement).setCaretAfterElement (siblingElement);
  } catch (e) {
    dump ("Warning: unable to setCaretAfterElement while inserting graph\n");
  }
  graph.computeGraph ();
  addGraphElementToDocument (graph.createGraphDOMElement(false), siblingElement, editorElement);
}

/**-----------------------------------------------------------------------------------------*/
// Create the <graph> element and insert into the document following this math element
// primary entry point
function insertNewGraph (math, dimension, plottype, optionalAnimate) {
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
  insertGraph (math, graph);
}

/**----------------------------------------------------------------------------------*/
// regenerate the graph  based on the contents of <graph>
// DOMGraph is the DOM graph element we are going to replace.
// called by the TestGraphScript() method in the debug menu.
function recreateGraph (DOMGraph) {
  var graph = new Graph();
  graph.extractGraphAttributes (DOMGraph);
  var parent = DOMGraph.parentNode;
  insertGraph (DOMGraph, graph);
  parent.removeChild (DOMGraph);
}

/**----------------------------------------------------------------------------------*/
// wrap this mathml fragment string inside a mathml <math> element
function wrapMath (thing) {
  return ("<math xmlns=\"http://www.w3.org/1998/Math/MathML\">" + thing + "</math>");
}


/**----------------------------------------------------------------------------------*/
// extract the variable list and expression from the fragment created from
// a plot query return string. The format of the fragment is
// [<maybeExplicitList>, <outvarlist>, <invarlist>, <expression>, <optionaldatalist>]
// except the string is really mathml, not just text. So it looks like this:

/* <math xmlns="http://www.w3.org/1998/Math/M*//* <math xmlns="http://www.w3.org/1998/Math/MathML"> */
/*   <mrow>                                  *//*   <mrow>                                          */
/*     <mfenced open="[" close="]">          *//*     <mo>[</mo>                                    */
/*       <mtext>"functionImage"</mtext>      *//*     <mrow>                                        */
/*       <mfenced open="[" close="]">        *//*       <mtext>"functionImage"</mtext>              */
/*         <mi>θ</mi>                       *//*       <mo>,</mo>                                  */
/*       </mfenced>                          *//*       <mrow>                                      */
/*       <mfenced open="[" close="]">        *//*         <mo>[</mo>                                */
/*         <mi>x</mi>                        *//*         <mi>θ</mi>                               */
/*         <mi>θ</mi>                       *//*         <mo>]</mo>                                */
/*       </mfenced>                          *//*       </mrow>                                     */
/*       <mfenced open="[" close="]">        *//*       <mo>,</mo>                                  */
/*         <mrow>                            *//*       <mrow>                                      */
/*           <mn>2</mn>                      *//*         <mo>[</mo>                                */
/*           <mo>⁢</mo>                    *//*         <mrow>                                    */
/*           <mi msiMathname="true">sin</mi> *//*           <mi>x</mi>                              */
/*           <mo>⁡</mo>                    *//*           <mo>,</mo>                              */
/*           <mrow>                          *//*           <mi>θ</mi>                             */
/*             <mn>3</mn>                    *//*         </mrow>                                   */
/*             <mo>⁢</mo>                  *//*         <mo>]</mo>                                */
/*             <mi>θ</mi>                   *//*       </mrow>                                     */
/*           </mrow>                         *//*       <mo>,</mo>                                  */
/*           <mo>−</mo>                    *//*       <mrow>                                      */
/*           <mi msiMathname="true">sin</mi> *//*         <mo>[</mo>                                */
/*           <mo>⁡</mo>                    *//*         <mrow>                                    */
/*           <mi>θ</mi>                     *//*           <mrow>                                  */
/*           <mo>+</mo>                      *//*             <mn>2</mn>                            */
/*           <mn>1</mn>                      *//*             <mo>⁢</mo>                          */
/*         </mrow>                           *//*             <mi msiMathname="true">sin</mi>       */
/*         <mi>θ</mi>                       *//*             <mo>⁡</mo>                          */
/*       </mfenced>                          *//*             <mrow>                                */
/*       <mrow>                              *//*               <mn>3</mn>                          */
/*         <mo>[</mo>                        *//*               <mo>⁢</mo>                        */
/*         <mo>]</mo>                        *//*               <mi>θ</mi>                         */
/*       </mrow>                             *//*             </mrow>                               */
/*     </mfenced>                            *//*             <mo>−</mo>                          */
/*   </mrow>                                 *//*             <mi msiMathname="true">sin</mi>       */
/* </math>									 *//*             <mo>⁡</mo>                          */
                                               /*             <mi>θ</mi>                           */
                                               /*             <mo>+</mo>                            */
                                               /*             <mn>1</mn>                            */
                                               /*           </mrow>                                 */
                                               /*           <mo>,</mo>                              */
                                               /*           <mi>θ</mi>                             */
                                               /*         </mrow>                                   */
                                               /*         <mo>]</mo>                                */
                                               /*       </mrow>                                     */
                                               /*       <mo>,</mo>                                  */
                                               /*       <mrow>                                      */
                                               /*         <mo>[</mo>                                */
                                               /*         <mo>]</mo>                                */
                                               /*       </mrow>                                     */
                                               /*     </mrow>                                       */
                                               /*     <mo>]</mo>                                    */
                                               /*   </mrow>                                         */
                                               /* </math>                                           */



// Run along the string looKing for <mfenced> and </mfenced> pairs. We want
// the second-level of these (there may be many embedded in the expression.)
// Not and RE parsing problem: push each occurrence of <mfenced>, pop matched
// </mfenced>, and if the right level and count, save the needed data.
function parseQueryReturn (out, graph, plot_no) {
  var stack = [];                                                             
  var level = 0;                                                              
  var result;                                                                 
  var variableList, expr;                                                     
  var count = 0;   
  var index = 0;                                                           
  dump("SMR Query returned: " + out + "\n");
  var pt   = graph.getPlotAttribute (PlotAttrName ("PlotType", plot_no)); 
  // search for top level "mfenced" and the matching "/mfenced"                            
  // 4/11/06 this while loop should be unnecessary: the compute engine doesn't return
  // mfenced anymore.
  while ((result = out.indexOf ("mfenced", index)) >= 0) {
    index = result+1;
    if (out[result-1] == "/") { // end of mfence
	  var start = stack.pop();
      var stop  = out.indexOf (">", result) + 1;  // find </mfenced>
  	  level = level - 1;
      if (level == 1) {  // we have the end of a second-to-top-level mfence
	    count = count + 1;
	    if (count == 1)  // save the output variable list
	      variableList = out.slice (start, stop);
  	    else if (count == 3)  // save the returned expression
    	  expr = wrapMath (out.slice (start, stop));
      }
    } else { // it must be "mfenced"
      level = level + 1;
      stack.push (out.lastIndexOf("<", result)); // find <mfenced, even w/ namespace
    }
  }

  if (count < 3) {
    // search for top level "row" and the matching "/row"
    count = 0; index = 0; level = 0;
    while ((result = out.indexOf ("mrow", index)) >= 0) {
      index = result+1;
      if (out[result-1] == "/") { // end of mrow
		  var start = stack.pop();
          var stop  = out.indexOf (">", result) + 1;  // find </mrow>
    	  level = level - 1;
          if (level == 2) {  // we have the end of a second-to-top-level
		    count = count + 1;
		    if (count == 1)  // save the output variable list
		      variableList = out.slice (start, stop);
    	    else if (count == 3)  // save the returned expression
      	  expr = wrapMath (out.slice (start, stop));
        }
      } else { // it must be "mrow"
        level = level + 1;
        stack.push (out.lastIndexOf("<", result)); // find <mrow, even w/ namespace
      }
    }
    
    if ((pt != "gradient") && (count < 3)) {
//      this.prompt.alert (null, "Computation Warning", 
  //        "Warning: Prepare Plot was unable to identify dependent variables and prepare the expression");
      alert ("Warning: Prepare Plot was unable to identify dependent variables and prepare the expression");
      dump("Warning: Prepare Plot was unable to identify dependent variables and prepare the expression\n");
    }  
  }  
                                                                
  if (variableList)
    graphSaveVars (variableList, graph, plot_no);

  if (expr) {
    if ((pt == "polar") || (pt == "spherical") || (pt == "cylindrical") ||
        (pt == "parametric") || (pt == "gradient") || (pt == "vectorField")) {
      // insert ilk="enclosed-list" as attribute to <mfenced>
      var hasilk = expr.indexOf("ilk=",0);
      if (hasilk == -1) {
        var s = expr.indexOf("<mfenced",0);
        if (s>0) {
          expr = expr.slice (0,s) + "<mfenced ilk=\"enclosed-list\" " + expr.slice(s+8);
        }
      }
      expr = runFixup(expr);
      graph.setPlotAttribute (PlotAttrName ("Expression", plot_no), expr);
    }
  }

  // look for "explicitList" and "parametric"
  // Set the plot types if these are found
  var s = out.indexOf("explicitList",0);
  if (s > 0) {
    graph.setPlotAttribute (PlotAttrName ("PlotType", plot_no), "explicitList");
  } else {
    s = out.indexOf ("parametric");
    if (s > 0) {
      graph.setPlotAttribute (PlotAttrName ("PlotType", plot_no), "parametric");
    }
  }
}


// put the vars in the <graph>
//<mfenced open="[" close="]">
//        <mi>x</mi>
//        <mi>y</mi>
//        <mi>z</mi>
//      </mfenced>
function graphSaveVars (varList, g, plot_no) {
  var xyzvar, i;
  if (varList) {
    var index = 0;
    var stop = 0;
    var start = varList.indexOf ("<mi>", index);
    if (start >= 0) {
      stop = varList.indexOf ("</mi>", start+1);
      xyzvar = wrapMath (varList.slice ( start, stop+5));
      g.setPlotAttribute (PlotAttrName ("XVar", plot_no), xyzvar);
      index = stop+1;
    }
    start = varList.indexOf ("<mi>", index);
    if (start >= 0) {
      stop = varList.indexOf ("</mi>", start+1);
      xyzvar = wrapMath (varList.slice ( start, stop+5));
      g.setPlotAttribute (PlotAttrName ("YVar", plot_no), xyzvar);
      index = stop+1;
    }
    start = varList.indexOf ("<mi>", index);
    if (start >= 0) {
      stop = varList.indexOf ("</mi>", start+1);
      xyzvar = wrapMath (varList.slice ( start, stop+5));
      g.setPlotAttribute (PlotAttrName ("ZVar", plot_no), xyzvar);
      index = stop+1;
    }
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
//  for (idx = 0; idx < A.length && A[idx] != element; idx++) ;
//  if (A[idx] != element)
    idx = -1;
  return idx;
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

  var nvars = CountPlotVars (dim, ptype, animate);

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


function CountPlotVars (dim, ptype, animate) {
  var nvars = 0;
  switch (ptype) {
    case "curve":
    case "explicitList":
       nvars = 0;            // gets set to 1 below
       break;
    case "rectangular":
    case "polar":
    case "parametric":
    case "approximateIntegral":
    case "spherical":
    case "cylindrical":
    case "tube":
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
       dump ("SMR ERROR in GraphOverlay line 932 unknown plot type " + ptype+"\n");
//       alert ("SMR ERROR in GraphOverlay line 932 unknown plot type " + ptype);
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

function testQueryGraph (domgraph) {
  var graph = new Graph();
  graph.extractGraphAttributes(domgraph);

  var expr = graph.getPlotAttribute (PlotAttrName ("Expression",1));
  var expr2 = runFixup(runFixup(expr));
  graph.setPlotAttribute (PlotAttrName ("Expression",1), expr2);
  graph.computeQuery(1);

  var parent = domgraph.parentNode;
  insertGraph (domgraph, graph);
  parent.removeChild (domgraph);
}