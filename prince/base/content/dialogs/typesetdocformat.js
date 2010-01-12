 
var currentUnit;
var sectionUnit;
var unitConversions;
var widthElements;
var heightElements;
var pagewidth;   // in mm
var pageheight;                                                     
var finishpage;
var papertype;
var paperwidth;
var paperheight;
var landscape;
var scale = 0.5;

var editor;
var sectitleformat;
var sectScale = 1;


function InitializeUnits()
{
  currentUnit = document.getElementById("docformat.units").selectedItem.value;
	sectionUnit = currentUnit;
	document.getElementById("secoverlay.units").value = sectionUnit;
  setDecimalPlaces();
  unitConversions = new Object;
  unitConversions.pt=.3514598;  //mm per pt
  unitConversions.in=25.4;  //mm per in
  unitConversions.mm=1; // mm per mm
  unitConversions.cm=10; // mm per cm
  unitConversions.pica=4.2175176; // 12 * (mm per pt)
}

function newValue(element, index, array)
{
  if (!element.match(/\S/)) return false;
  if (index == 0) return true;
  return element != array[index-1];
}

var re = /:([^:]*)$/i;

function stripPath(element, index, array)
{
  array[index] = re.exec(element)[1];
}
 
function initializeFontFamilyList(force)
{
  var OTOk = document.getElementById("useOpenType").checked;
  changeOpenType();
  var prefs = GetPrefs();
  var dsprops = Components.classes["@mozilla.org/file/directory_service;1"].createInstance(Components.interfaces.nsIProperties);
  var dir = dsprops.get("ProfD", Components.interfaces.nsIFile);
  var texbindir;
  var outfile;
  outfile = dir.clone();
  outfile.append("fontfamilies.txt");
  try { texbindir= prefs.getCharPref("swp.tex.bindir"); }
  catch(exc) {dump("texbindir not set in preference\n");}
  if (!force)
  { 
    if (outfile.exists()) return;
  }
  var listfile = dir.clone(); 
  listfile.append("bigfontlist.txt");
  if (listfile.exists()) listfile.remove(false);
  var exefile = dsprops.get("resource:app", Components.interfaces.nsIFile);;
  exefile.append("BuildFontFamilyList.cmd");

  try 
  {
    var theProcess = Components.classes["@mozilla.org/process/util;1"].createInstance(Components.interfaces.nsIProcess);
    theProcess.init(exefile);
    dump("TexBinDir is "+texbindir+"\n");
    var args =[texbindir,listfile.parent.path];
    theProcess.run(true, args, args.length);
  } 
  catch (ex) 
  {
       dump("\nUnable to run OtfInfo.exe\n");
       dump(ex+"\n");
  }      
  if (!listfile.exists())
  {
    dump("Failed to create bigfontlist.txt\n");
    return;
  }
  var path = msiFileURLFromAbsolutePath( listfile.target )
  var myXMLHTTPRequest = new XMLHttpRequest();
  myXMLHTTPRequest.overrideMimeType("text/plain");
  myXMLHTTPRequest.open("GET", path, false);
  myXMLHTTPRequest.send(null);
  var str = myXMLHTTPRequest.responseText;
  var lines = str.split(/[\n\r]*[a-z]:[^:]*:/i);
  var i;
  var limit;
  lines = lines.sort();
  var unique = lines.filter(newValue);
  limit = unique.length;
// output the result
  str = "";
  if (outfile.exists()) outfile.remove(false);
  for (i =0; i < limit; i++)
    str += unique[i] + "\n";
  var fos = Components.classes["@mozilla.org/network/file-output-stream;1"].createInstance(Components.interfaces.nsIFileOutputStream);
  fos.init(outfile, -1, -1, false);
  var os = Components.classes["@mozilla.org/intl/converter-output-stream;1"]
    .createInstance(Components.interfaces.nsIConverterOutputStream);
  os.init(fos, "UTF-8", 4096, "?".charCodeAt(0));
  os.writeString(str);
  os.close();
  fos.close();
//  dump(str);
  initSystemFontMenu("mathfontlist", false);
  initSystemFontMenu("mainfontlist", OTOk);
  initSystemFontMenu("sansfontlist", OTOk);
  initSystemFontMenu("fixedfontlist", OTOk);
  initSystemFontMenu("x1fontlist", OTOk);
  initSystemFontMenu("x2fontlist", OTOk);
  initSystemFontMenu("x3fontlist", OTOk);
}

function Startup()
{
  initializeFontFamilyList(false);
  var editorElement = msiGetParentEditorElementForDialog(window);
  editor = msiGetEditor(editorElement);
  if (!editor) {
    window.close();                                               
    return;
  }
  widthElements=new Array("lmargin","bodywidth","colsep", "mnsep","mnwidth","computedrmargin",
    "sectrightheadingmargin", "sectleftheadingmargin", "pagewidth", "paperwidth");
  heightElements=new Array("tmargin","header","headersep",
    "bodyheight","footer","footersep", "mnpush", "computedbmargin", "pageheight", "paperheight");
  InitializeUnits();
  var doc = editor.document;
  var preamble = doc.documentElement.getElementsByTagName('preamble')[0];
  if (!preamble) {
    dump("No preamble in document\n");
    return;
  }
  var docFormatNodeList = preamble.getElementsByTagName('docformat');
  var node;
  if (!(docFormatNodeList && docFormatNodeList.length>=1)) node=null;
  else node = docFormatNodeList[0].getElementsByTagName('pagelayout')[0];
  getPageLayout(node);
  if (!(docFormatNodeList && docFormatNodeList.length>=1)) node=null;
  else node = docFormatNodeList[0].getElementsByTagName('crop')[0];
  getCropInfo(node);
  
  //now we can load the docformat information from the document to override 
  //all or part of the initial state
  OnWindowSizeReset(true);
  initSystemFontMenu("mathfontlist");
  initSystemFontMenu("mainfontlist");
  initSystemFontMenu("sansfontlist");
  initSystemFontMenu("fixedfontlist");
  initSystemFontMenu("x1fontlist");
  initSystemFontMenu("x2fontlist");
  initSystemFontMenu("x3fontlist");
  if (!(docFormatNodeList && docFormatNodeList.length>=1)) node=null;
  else node = docFormatNodeList[0].getElementsByTagName('fontchoices')[0];
  getFontSpecs(node);
  var sectitlenodelist;
  if (!(docFormatNodeList && docFormatNodeList.length>=1)) sectitlenodelist=null;
  else sectitlenodelist = docFormatNodeList[0].getElementsByTagName('sectitleformat');
  
//  var editElement = document.getElementById("sectiontitle-frame");
//  msiInitializeEditorForElement(editElement, "");
	dump ("initializing sectitleformat\n");
  sectitleformat = new Object();
  getSectionFormatting(sectitlenodelist, sectitleformat);
}


function lineend(node, spacecount)
{
  var spacer = "\n";
  for (var i = 0; i < spacecount; i++) 
    spacer = spacer + "  ";
  node.appendChild(node.ownerDocument.createTextNode(spacer));
}
  
function savePageLayout(docFormatNode)
{
  lineend(docFormatNode, 1);
  var pfNode = editor.createNode('pagelayout', docFormatNode,0);
  var nodecounter = 0;
  var units;
  pfNode.setAttribute('latex',true);
  units=document.getElementById('docformat.units').value;
  pfNode.setAttribute('unit',units);
  lineend(pfNode, 2);
  nodecounter++;
  node = editor.createNode('page', pfNode, nodecounter++);
  node.setAttribute('twoside',document.getElementById('twosides').checked);
  node.setAttribute('paper',document.getElementById('docformat.finishpagesize').value);
  node.setAttribute('width',pagewidth+units);
  node.setAttribute('height',pageheight+units);
  node.setAttribute('landscape',document.getElementById('landscape').checked);
  lineend(pfNode, 2);
  nodecounter++;
  node = editor.createNode('textregion',pfNode,nodecounter++);
  node.setAttribute('width',document.getElementById('tbbodywidth').value+units);
  node.setAttribute('height', document.getElementById('tbbodyheight').value+units);
  lineend(pfNode, 2);
  nodecounter++;
  node = editor.createNode('margin',pfNode,nodecounter++);
  node.setAttribute('left',document.getElementById('tblmargin').value+units);
  node.setAttribute('top', document.getElementById('tbtmargin').value+units);
  lineend(pfNode, 2);
  nodecounter++;
  node = editor.createNode('columns',pfNode,nodecounter++);
  node.setAttribute('count',(document.getElementById('columns').value));
  node.setAttribute('sep',document.getElementById('tbfootersep').value+units);
  lineend(pfNode, 2);
  nodecounter++;
  node = editor.createNode('marginnote',pfNode,nodecounter++);
//  node.setAttribute('reversed',document.getElementById('*****').checked);
  node.setAttribute('width',document.getElementById('tbmnwidth').value+units);
  node.setAttribute('sep',document.getElementById('tbmnsep').value+units);
  node.setAttribute('push',document.getElementById('tbmnpush').value+units);
  node.setAttribute('hidden', document.getElementById('disablemn').checked);
  lineend(pfNode, 2);
  nodecounter++;
  node = editor.createNode('header',pfNode,nodecounter++);
  node.setAttribute('height',document.getElementById('tbheader').value+units);
  node.setAttribute('sep',document.getElementById('tbheadersep').value+units);
  lineend(pfNode, 2);
  nodecounter++;
  node = editor.createNode('footer',pfNode,nodecounter++);
  node.setAttribute('height',document.getElementById('tbfooter').value+units);
  node.setAttribute('sep',document.getElementById('tbfootersep').value+units);
}

function saveCropMarks(docFormatNode)
{
  var pos;
  lineend(docFormatNode, 1);
  if (document.getElementById("useCropmarks").checked)
  {
    var cropNode = editor.createNode('crop', docFormatNode, 0);
    cropNode.setAttribute("unit", currentUnit);
    cropNode.setAttribute("type", document.getElementById("cropGroup").value);
    pos = document.getElementById("pageonpaperlayout").value;
    cropNode.setAttribute("pos", pos);
    if (pos == "center")
    {
      var paper = document.getElementById("docformat.papersize").value;
      cropNode.setAttribute("paper", paper);
      if (paper == "other")
      {
        cropNode.setAttribute("width", document.getElementById("tbpaperwidth").value);
        cropNode.setAttribute("height", document.getElementById("tbpaperheight").value);
      }
    }
  }
}

function centerCropmarks(value)
{
  var broadcaster = document.getElementById("pageonpapercentered");
  broadcaster.setAttribute("hidden", (value=="center"?"false":"true"));
}

function getNumberValue(numberwithunit)
{
  var reNum = /\d*\.?\d*/;
  var reUnit = /in|pt|mm|cm/;
  var num = reNum.exec(numberwithunit);
  if (num.length > 0)
  {
    dump(reUnit.exec()+"\n");
    return Number(num);
  }
  return 0;
}


function getPageLayout(node)
{
  //node is the pageformat node, or null
  var subnode;
  var i;
  var value;
  if (!node)
  {
    setPageDimensions();
    setPaperDimensions();
    setDefaults();
  }  
  if (node){                                                                             
    value = node.getAttribute('unit');
    if (value) {
      currentUnit = value;
      document.getElementById('docformat.units').value = currentUnit;
    }
    subnode = node.getElementsByTagName('page')[0];
    if (subnode)
    {
      value = subnode.getAttribute('twoside');
      document.getElementById('twosides').checked=(value=='true');
      setTwosidedState(document.getElementById('twosides'));
      value = subnode.getAttribute('paper');
      finishpage = value;
      if (value) document.getElementById('docformat.finishpagesize').value = value; 
      if (value != "other") document.getElementById("bc.finishpagesize").setAttribute("disabled","true");
      value = subnode.getAttribute('width');
      if (value) 
      {
        pagewidth = getNumberValue(value);
        document.getElementById("tbpagewidth").value = pagewidth;
      }
      value = subnode.getAttribute('height');
      if (value) 
      {
        pageheight = getNumberValue(value);
        document.getElementById("tbpageheight").value = pageheight;
      }
      value = subnode.getAttribute('landscape');
      if (value=='true') landscape = true; else landscape = false;
      document.getElementById('landscape').checked=landscape;
    }                                                    
    setPageDimensions();
    setPaperDimensions();
    setDefaults();
    subnode = node.getElementsByTagName('textregion')[0];
    if (subnode)
    {
      value = subnode.getAttribute('width');
      if (value) document.getElementById('tbbodywidth').value=getNumberValue(value);
      value = subnode.getAttribute('height');
      if (value) document.getElementById('tbbodyheight').value=getNumberValue(value);
    }
    subnode = node.getElementsByTagName('margin')[0];
    if (subnode)
    {
      value = subnode.getAttribute('left');
      if (value) document.getElementById('tblmargin').value=getNumberValue(value);
      value = subnode.getAttribute('top');
      if (value) document.getElementById('tbtmargin').value=getNumberValue(value);
    }
    subnode = node.getElementsByTagName('columns')[0];
    if (subnode)
    {
      value = subnode.getAttribute('count');
      document.getElementById('columns').value=value;
      broadcastColCount();
      value = subnode.getAttribute('sep');
      if (value) document.getElementById('tbcolsep').value=getNumberValue(value);
    }
    subnode = node.getElementsByTagName('marginnote')[0];
    if (subnode)
    {
      value = subnode.getAttribute('width');
      if (value) document.getElementById('tbmnwidth').value=getNumberValue(value);
      value = subnode.getAttribute('sep');
      if (value) document.getElementById('tbmnsep').value=getNumberValue(value);
      value = subnode.getAttribute('push');
      if (value) document.getElementById('tbmnpush').value=getNumberValue(value);
      value = subnode.getAttribute('hidden');
      if (value) document.getElementById('disablemn').checked=(value=='true');
        else document.getElementById('disablemn').checked=false;
    }
    subnode = node.getElementsByTagName('header')[0];
    if (subnode)
    {
      value = subnode.getAttribute('height');
      if (value) document.getElementById('tbheader').value=getNumberValue(value);
      value = subnode.getAttribute('sep');
      if (value) document.getElementById('tbheadersep').value=getNumberValue(value);
    }
    subnode = node.getElementsByTagName('footer')[0];
    if (subnode)
    {
      value = subnode.getAttribute('height');
      if (value) document.getElementById('tbfooter').value=getNumberValue(value);
      value = subnode.getAttribute('sep');
      if (value) document.getElementById('tbfootersep').value=getNumberValue(value);
    }
  }
}

      
function onAccept()
{
// first check that something is set
  var doc = editor.document;
  var preamble = doc.documentElement.getElementsByTagName('preamble')[0];
  var head;
  if (!preamble) { // create the preamble
    head = doc.documentElement.getElementsByTagName('head')[0];
    preamble = editor.createNode('preamble',head,head.childNodes.length);
  }
// delete any current docformat nodes -- we are replacing them
  var docFormatNodeList = preamble.getElementsByTagName('docformat');
  var i;
  if (docFormatNodeList)
    for (i = docFormatNodeList.length - 1; i >= 0; i--)
      editor.deleteNode(docFormatNodeList[i])
  var newNode = editor.createNode('docformat',preamble,0);
// should check that the user wants to use geometry package.
  if (newNode) 
  {
    saveCropMarks(newNode);
    savePageLayout(newNode);
    saveFontSpecs(newNode);
    saveSectionFormatting(newNode, sectitleformat);
  }
}  


function onCancel()
{
}

function disableMarginNotes()
{
  var elt = document.getElementById('disablemarginnotes');
  if (document.getElementById("disablemn").checked)
  { 
    elt.setAttribute('hidden','true');
    elt.setAttribute('disabled','true');
  }
  else
  { 
    elt.removeAttribute('hidden');
    elt.removeAttribute('disabled');
  }
  updateComputedMargins();
}


function convert(invalue, inunit, outunit) // Converts invalue inunits into x outunits
{
  if (inunit == outunit) return invalue;
  var outvalue = invalue*unitConversions[inunit];
  outvalue /= unitConversions[outunit];
  dump(invalue+inunit+" = "+outvalue+outunit+"\n");
  return outvalue;
}


// * Callback for spinbuttons. Increments of |increment| the length value of
//   the XUL element of ID |id|.
//   param integer increment
//   param String id
function Spinbutton(increment, id)
{
  var elt = document.getElementById(id);
  if (elt) {
    var value = elt.value;
    if (value + increment >= 1) value += increment;
    elt.oninput();
  }
}

// Set defaults roughly to what the geometry package uses
function setDefaults()
{
  var twosided = false;
  // set geometry package defaults in case there is nothing specified in our document
  var bodyheight = .7*pageheight;
  var bodywidth = .7*pagewidth;
  document.getElementById("tbbodyheight").value = unitRound(bodyheight);
  document.getElementById("tbbodywidth").value = unitRound(bodywidth);
  var bighmargin; // total (left+right) margin space assuming no margin notes
  var bigvmargin; // total (top+bottom) margin space assuming no header/footer
  bighmargin = pagewidth - bodywidth;
  bigvmargin = pageheight - bodyheight;
  // divide vertical margin by 2:3
  var oneline = convert(12,"pt",currentUnit);
  document.getElementById("tbtmargin").value = unitRound(.4*bigvmargin-2*oneline);
  var roundedOneline = unitRound(oneline);
  document.getElementById("tbfooter").value = roundedOneline;
  document.getElementById("tbheader").value = roundedOneline;
  document.getElementById("tbfootersep").value = roundedOneline;
  document.getElementById("tbheadersep").value = roundedOneline;
  // if twosided, divide horizontal margins 3:2, otherwise 1:1
  if (twosided)
  {
    document.getElementById("tblmargin").value = unitRound(.4*bighmargin);
    document.getElementById("tbmnwidth").value = unitRound(.8*(.6*bighmargin-oneline));
  }  
  else
  {
    document.getElementById("tblmargin").value = unitRound(.5*bighmargin);
    document.getElementById("tbmnwidth").value = unitRound(.8*(.5*bighmargin-oneline));
  }
  document.getElementById("tbmnsep").value = roundedOneline;
  document.getElementById("tbmnpush").value = roundedOneline;
  document.getElementById("tbcolsep").value = roundedOneline;
  // the margin note text is just for show, but it needs to be a reasonable size
  setHeight("topmarginnote", bodyheight*0.15);
  setHeight("bottommarginnote", bodyheight*.1);
}

function changefinishpageSize(menu)
{
  finishpage = menu.value;
  if (finishpage == "other")
  {
    //enable setting finishpage size
    document.getElementById("bc.finishpagesize").removeAttribute("disabled");
  }
  else
  {
    document.getElementById("bc.finishpagesize").setAttribute("disabled","true");
  }
  setPageDimensions();
}


// Page layout stuff  
function changePageDim(textbox)
{
  if (textbox.id == "tbpageheight") pageheight = convert(Number(textbox.value),currentUnit,"mm");
  else pagewidth = convert(Number(textbox.value),currentUnit,"mm");
  setPageDimensions();
}

//this gets the page dimensions for page type obj.pagename and returns them as obj.width 
// and obj.height. If the pagename is 'other', it does not set the heightand width and returns
// false. Otherwise it returns true.
function getPageDimensions( obj)
{
  switch (obj.pagename) {
    case "letter":   // these dimensions are in millimeters
      obj.width = 215.9;
      obj.height = 279.4;
      break;
		case "a4":
      obj.width = 210;
      obj.height = 297;
      break;
		case "other":
      return false;
    case "screen":
      obj.width = 225;
      obj.height = 180;
      break;
		case "a0":
      obj.width = 841;
      obj.height = 1189;
      break;
		case "a1":
      obj.width = 594;
      obj.height = 841;
      break;
		case "a2":
      obj.width = 420;
      obj.height = 594;
      break;
		case "a3":
      obj.width = 297;
      obj.height = 420;
      break;
		case "a5":
      obj.width = 148;
      obj.height = 210;
      break;
		case "a6":
      obj.width = 105;
      obj.height = 148;
      break;
		case "b0":
      obj.width = 1000;
      obj.height = 1414;
      break;
		case "b1":
      obj.width = 707;
      obj.height = 1000;
      break;
		case "b2":
      obj.width = 500;
      obj.height = 707;
      break;
		case "b3":
      obj.width = 353;
      obj.height = 500;
      break;
		case "b4":
      obj.width = 250;
      obj.height = 353;
      break;
		case "b5":
      obj.width = 176;
      obj.height = 250;
      break;
		case "b6":
      obj.width = 125;
      obj.height = 176;
      break;
		case "executive":
      obj.width = 184;
      obj.height = 267;
      break;
		case "legal":
      obj.width = 216;
      obj.height = 356;
      break;
    default:  // default to letter
      obj.width = 215.9;
      obj.height = 279.4;
      break;
  }
  return true;
}

function setPageDimensions()
{
//  finishpage = "letter";
//  currentUnit ="in"; 
//  landscape = false;
  var obj = new Object();
  obj.pagename = finishpage;
  
  if (!getPageDimensions(obj))
  { 
    pagewidth = convert(Number(document.getElementById("tbpagewidth").value), currentUnit,"mm");
    pageheight = convert(Number(document.getElementById("tbpageheight").value), currentUnit,"mm");
  } else
  {
    pagewidth = obj.width;
    pageheight = obj.height;
  }
  if (landscape) {
    var temp = pageheight;
    pageheight = pagewidth;
    pagewidth = temp;
  }
  if (currentUnit != "mm") {
    pagewidth = convert(pagewidth, "mm", currentUnit);
    pageheight = convert(pageheight, "mm", currentUnit);
  }
//  setHeight("pageh", pageheight);
//  setWidth("page", pagewidth);
  if (finishpage != "other")
  {
    document.getElementById("tbpagewidth").value = unitRound(pagewidth);
    document.getElementById("tbpageheight").value = unitRound(pageheight);
    document.getElementById("bc.finishpagesize").setAttribute("disabled","true");
  }
  else document.getElementById("bc.finishpagesize").removeAttribute("disabled");
}

function changepaperSize(menu)
{
  papertype = menu.value;
  if (papertype == "other")
  {
    //enable setting finishpage size
    document.getElementById("bc.papersize").removeAttribute("disabled");
  }
  else
  {
    document.getElementById("bc.papersize").setAttribute("disabled","true");
  }
  setPaperDimensions();
}

function getCropInfo(node)
{
  var broadcaster = document.getElementById("cropmarks");        
  var broadcaster2 = document.getElementById("pageonpapercentered");
  if (!node) {
    papertype="letter";
    document.getElementById("useCropmarks").checked=false;
    document.getElementById("cropGroup").value = "cam";
    document.getElementById("bc.papersize").setAttribute("disabled","true");
    broadcaster.setAttribute("hidden",true);
    broadcaster2.setAttribute("hidden",true);
  }
  else
  {
    document.getElementById("useCropmarks").checked=true;
    broadcaster.removeAttribute("hidden");
    var croptype = node.getAttribute("type");
    if (!croptype) croptype = "cam";
    document.getElementById("cropGroup").value = croptype;
    var pos = node.getAttribute("pos");
    document.getElementById("pageonpaperlayout").value=pos;
    if (pos == "center")
    {
      broadcaster2.removeAttribute("hidden");
      var paper = node.getAttribute("paper");
      if (!paper) paper="letter";
      document.getElementById("docformat.papersize").value = paper;
      papertype = paper;
      if (paper == "other")
      {
        paperheight = node.getAttribute("height");
        if (!paperheight) paperheight = document.getElementById("tbpageheight").value; 
        document.getElementById("tbpaperheight").value = paperheight;
        paperwidth = node.getAttribute("width");
        if (!paperwidth) paperwidth = document.getElementById("tbpaperwidth").value; 
        document.getElementById("tbpaperwidth").value = paperwidth;
      }
    }
  }
  setPaperDimensions();
}


function setPaperDimensions()
{
  var obj = new Object();
  obj.pagename = papertype;
  
  if (!getPageDimensions(obj))
  { 
    paperwidth = convert(Number(document.getElementById("tbpaperwidth").value), currentUnit,"mm");
    paperheight = convert(Number(document.getElementById("tbpaperheight").value), currentUnit,"mm");
  } else
  {
    paperwidth = obj.width;
    paperheight = obj.height;
  }
  if (landscape) {
    var temp = pageheight;
    paperheight = paperwidth;
    paperwidth = temp;
  }
  if (currentUnit != "mm") {
    paperwidth = convert(paperwidth, "mm", currentUnit);
    paperheight = convert(paperheight, "mm", currentUnit);
  }
  if (papertype != "other")
  {
    document.getElementById("tbpaperwidth").value = unitRound(paperwidth);
    document.getElementById("tbpaperheight").value = unitRound(paperheight);
    document.getElementById("bc.papersize").setAttribute("disabled","true");
  }
  else document.getElementById("bc.papersize").removeAttribute("disabled");
}

function unitRound( size )
{
  var places;
  // round off to a number of decimal places appropriate for the units
  switch (currentUnit) {
    case "mm" : places = 10;
      break;
    case "cm" : places = 100;
      break;
    case "pt" : places = 10;
      break;
    case "in" : places = 100;
      break;
    default   : places = 100;
  }
  return Math.round(size*places)/places;
}


function OnWindowSizeReset(initial)
{
  var twomargins = 60; //20 pixels per margin for the layout page
  //var oldscale = scale;
  var width = 1.8*document.getElementById("gb.dimension").boxObject.width;
//  if (!initial) {
//    var winwidth = document.getElementById("docformat.dialog").boxObject.width-20;
//    if (winwidth < width) width = winwidth;
//  }
//  if (width < 2*twomargins) return;
  scale = width/pagewidth;
  //if (oldscale != scale)
    layoutPage("all");
}

function setWidth(id, units)
{
  var elt;
  if (units < 0) units = 0;
  try {
    elt = document.getElementById(id);
    if (elt)
      elt.setAttribute("style","width:"+Math.round(units*scale)+"px;");
  }
  catch(e) {
    dump(e+"\n");
  }
}

function setHeight(id, units)  
{
  var elt;
  if (units < 0) units = 0;
  try {
    elt = document.getElementById(id);
    if (elt)
      elt.setAttribute("style","height:"+Math.round(units*scale)+"px");
  }
  catch(e) {
    dump(e+"\n");
  }
}

function setDecimalPlaces() // and increments
{
  var places;
  var increment;
  var elt;
  var i;
  var s;
  switch (currentUnit) {
    case "in" : increment = .1; places = 2; break;
    case "cm" : increment = .1; places = 2; break;
    case "mm" : increment = 1; places = 1; break;
    case "pt" : increment = 1; places = 1; break;
    default : increment = 1; places = 1; break;
  }
  for (i=0; i<widthElements.length; i++)
  {
    s=widthElements[i];
    elt = document.getElementById("tb"+s);
    elt.setAttribute("increment", increment);
    elt.setAttribute("decimalplaces", places);
  }
  for (i=0; i<heightElements.length; i++)
  {
    s=heightElements[i];
    elt = document.getElementById("tb"+s);
    elt.setAttribute("increment", increment);
    elt.setAttribute("decimalplaces", places);
  }
  dump("Setting 'increment' to "+increment+" and 'decimalplaces' to "+places+"\n");
}    

function containedin(anArray, aString)
{
  var i;
  for (i=0; i<anArray.length; i++) if (anArray[i] == aString) return true;
  return false;
}

function updateComputedMargins()
{
  // find computed margins
  var tbbodywidth = Number(document.getElementById("tbbodywidth").value);
  var tblmargin = Number(document.getElementById("tblmargin").value);
  var tbmnwidth = Number( document.getElementById("tbmnwidth").value);
  var tbmnsep = Number(document.getElementById("tbmnsep").value);
  var wtotal = tblmargin +  tbbodywidth;
  if (!(document.getElementById("disablemn").checked))
    wtotal += tbmnsep + tbmnwidth;
  var tbtopmargin = Number(document.getElementById("tbtmargin").value);
  var tbbody = Number(document.getElementById("tbbodyheight").value);
  var tbheader = Number(document.getElementById("tbheader").value);
  var tbheadersep = Number(document.getElementById("tbheadersep").value);
  var tbfootersep = Number(document.getElementById("tbfootersep").value);
  var tbfooter = Number(document.getElementById("tbfooter").value);
  var htotal = tbtopmargin + tbheader + tbheadersep + tbbody + tbfooter + tbfootersep;
  document.getElementById("tbcomputedrmargin").value = unitRound(pagewidth - wtotal);                
  document.getElementById("tbcomputedbmargin").value = unitRound(pageheight - htotal);
  // now check for overflow conditions
  // overflow occurs if an element goes beyond the edge of the finishpage.
  var vcenter = document.getElementById("vcenter").checked;
  var hcenter = document.getElementById("hcenter").checked;
  var overflow = false;
  if (vcenter)
  {
    var bigvmargin = (pageheight - tbbody)/2; // the margin including headers and footers
    if (tbheader + tbheadersep > bigvmargin) overflow = true;
    if (tbfooter + tbfootersep > bigvmargin) overflow = true;
  }
  if (hcenter && !(document.getElementById("disablemn").checked))
  {
    var bighmargin = (pagewidth - tbbodywidth)/2;
    if (tbmnwidth + tbmnsep > bighmargin) overflow = true;
  }
  // the basic overflow condition with no centering  
  if ((pagewidth < wtotal) || (pageheight < htotal))
    overflow = true;
  document.getElementById("page").setAttribute("danger", overflow);
}


function layoutPage(which)
{
  var s;
  var i;
  var elt;
  var eventsource = document.getElementById("tb"+which);
  if (eventsource) eventsource._validateValue(eventsource.inputField.value,false,true);
  var vcenter = document.getElementById("vcenter").checked;
  var hcenter = document.getElementById("hcenter").checked;
  if (hcenter)
  { 
    var lmargin = (pagewidth - document.getElementById("tbbodywidth").value)/2;
    // subtract marginnote width and separation for even pages.
    document.getElementById("tblmargin").value = lmargin;
  }
  if (vcenter)
  {
    var tmargin = (pageheight - document.getElementById("tbbodyheight").value)/2;
    tmargin -= (Number(document.getElementById("tbheader").value) + Number(document.getElementById("tbheadersep").value));
    document.getElementById("tbtmargin").value = tmargin;
  }
  if (which == "all")
  {
    elt = document.getElementById("page");
    var h;
    var w;
    h = Math.round(pageheight*scale)+2; // +2 accounts for two 1-pixel borders
    w = Math.round(pagewidth*scale)+2;  
    elt.setAttribute("style","height:"+h+"px;width:"+w+"px;"+
      "max-height:"+h+"px;max-width:"+w+"px;");
    elt.setAttribute("maxwidth",w);
    elt.setAttribute("maxheight",h);
    for (i=0; i<widthElements.length; i++)
    {
      s=widthElements[i];
      setWidth(s,document.getElementById("tb"+s).value);
      if (s == "mnbody" && document.getElementById("tb"+s).value == 0)
      {
        document.getElementById("disablemarginnotes").hidden=true;
        document.getElementById("disablemarginnotes").disable=true;
      }
    }
    for (i=0; i<heightElements.length; i++)
    {
      s=heightElements[i];
      setHeight(s,document.getElementById("tb"+s).value);
    }
  }
  else if (containedin(widthElements,which))
  { 
    setWidth(which,document.getElementById("tb"+which).value);
    if (hcenter) setWidth("lmargin", document.getElementById("tblmargin").value);
  }
  else if (containedin(heightElements,which))
  { 
    setHeight(which,document.getElementById("tb"+which).value);
    if (vcenter) setHeight("tmargin", document.getElementById("tbtmargin").value);
  }
  // now check for overflows and set computed values
  updateComputedMargins();
}

function activate(id)
{
    var elt = document.getElementById(id);
    if (elt)
      elt.setAttribute("active","1");
}     


function deactivate(id)
{
    var elt = document.getElementById(id);
    if (elt)
      elt.setAttribute("active","0");
}     

function switchUnits()
{
  var newUnit = document.getElementById("docformat.units").selectedItem.value;
  var factor = convert(1, currentUnit, newUnit);
  pagewidth *= factor;
  pageheight *= factor;
  scale = scale/factor;
  currentUnit = newUnit; // this has to be set before unitRound and setDecimalPlaces are called
  setDecimalPlaces();
  document.getElementById("tbbodyheight").value = unitRound(factor*document.getElementById("tbbodyheight").value);
  document.getElementById("tbbodywidth").value = unitRound(factor*document.getElementById("tbbodywidth").value); 
  document.getElementById("tblmargin").value = unitRound(factor*document.getElementById("tblmargin").value);
  document.getElementById("tbtmargin").value = unitRound(factor*document.getElementById("tbtmargin").value);
  document.getElementById("tbheader").value = unitRound(factor*document.getElementById("tbheader").value);
  document.getElementById("tbheadersep").value = unitRound(factor*document.getElementById("tbheadersep").value);
  document.getElementById("tbfooter").value = unitRound(factor*document.getElementById("tbfooter").value);
  document.getElementById("tbfootersep").value = unitRound(factor*document.getElementById("tbfootersep").value);
  document.getElementById("tbmnwidth").value = unitRound(factor*document.getElementById("tbmnwidth").value);
  document.getElementById("tbmnsep").value = unitRound(factor*document.getElementById("tbmnsep").value);
  document.getElementById("tbmnpush").value = unitRound(factor*document.getElementById("tbmnpush").value);
  document.getElementById("tbcomputedrmargin").value = unitRound(factor*document.getElementById("tbmnpush").value);
  document.getElementById("tbcomputedbmargin").value = unitRound(factor*document.getElementById("tbmnpush").value);
  document.getElementById("tbpagewidth").value = unitRound(pagewidth);
  document.getElementById("tbpageheight").value = unitRound(pageheight);
  updateComputedMargins();
  document.getElementById("tbpaperwidth").value = unitRound(factor*document.getElementById("tbpaperwidth").value);;
  document.getElementById("tbpaperheight").value = unitRound(factor*document.getElementById("tbpaperheight").value);
}

function cropmarkRequest(checkbox)
{
  var broadcaster = document.getElementById("cropmarks");        
  if (checkbox.checked)
    broadcaster.removeAttribute("hidden");
  else {
    broadcaster.setAttribute("hidden","true");
    centerCropmarks("");
  }
}
  
function handleBodyMouseClick(event)
{
  var element = event.target;
  if (event.clientY - element.boxObject.y < element.boxObject.height/2)
    document.getElementById("tbbodywidth").focus();
  else
    document.getElementById("tbbodyheight").focus();
}

// since the onkeypress event gets called *before* the value of a text box is updated,
// we handle the updating here. This function takes a textbox element and an event and returns sets
// the value of the text box

function updateTextNumber(textelement, event)
{
  var val = textelement.value;
  var selStart = textelement.selectionStart;
  var selEnd = textelement.selectionEnd;
  var keycode = event.keyCode;
  var charcode = event.charCode;
  
  if (keycode == event.DOM_VK_BACK_SPACE) {
    if (selStart > 0) {
      selStart--;
      val = val.slice(0,selStart)+val.slice(selEnd);
    }
  }
  else 
  if (keycode == event.DOM_VK_DELETE) {
    selEnd++;
    val = val.slice(0,selStart)+val.slice(selEnd);
  }
  else
  if (charcode == "-".charCodeAt(0) || charcode == "+".charCodeAt(0) || charcode == ".".charCodeAt(0) ||
    (charcode >= 48 && charcode <58))
  {
    if (selEnd >= selStart) val = val.slice(0,selStart)+ String.fromCharCode(charcode)+val.slice(selEnd);
    selStart++;
  }
  else return;
  // now check to see if we have a string
  try {
    if (!isNaN(Number(val))) 
    {
      textelement.value = val;
      textelement.setSelectionRange(selStart, selStart)
      event.preventDefault();
    }
  }
  catch(e)
  {
    dump(e.toString + "\n");
  }
}           
 

function handleChar(event, id, tbid)
{
  var element = event.originalTarget;
  if (event.keyCode == event.DOM_VK_UP)
    goUp(tbid);
  else if (event.keyCode == event.DOM_VK_DOWN)
    goDown(tbid);
  else
    updateTextNumber(element,event);
}

function geomHandleChar(event, id, tbid)
{
  handleChar(event, id, tbid);
  geomInputChar(event, id, tbid);
}

function sectHandleChar(event)
{
  var tbid = event.target.id;
  var id;
  if (tbid == "tbsectleftheadingmargin")
    id = "sectleftheadingmargin";
  else if (tbid == "tbsectrightheadingmargin")
    id = "sectrightheadingmargin";
  else return;  
  handleChar(event, id, tbid);
  sectInputChar(event, id, tbid);
}

function geomInputChar(event, id, tbid)
{
  if (id!="columns") layoutPage(id);
}

function sectInputChar(event, id, tbid)
{
  sectLayout(id, tbid);
}  

function goUp(id)
{
  var element = document.getElementById(id);
  var value = Number(element.value);
  if (value == NaN) return;
  var max = Number(element.getAttribute("max"));
  if ((max == 0)||(max == NaN)) max = Number.MAX_VALUE;
  value += Number(element.getAttribute("increment"));
  value = Math.min(max,value);
  element.value = unitRound(value);
}

function goDown(id)
{
  var element = document.getElementById(id);
  var value = Number(element.value);
  if (value == NaN) return;
  var min = Number(element.getAttribute("min"));
  if (min == NaN) min = 0;
  value -= Number(element.getAttribute("increment"));
  value = Math.max(min,value);
  element.value = unitRound(value);
}

function broadcastColCount()
{
  var multicolumns = document.getElementById("columncount").value > 1;
  document.getElementById("multicolumn").setAttribute("hidden", !multicolumns);
  document.getElementById("singlecolumn").setAttribute("hidden", multicolumns);
}

function setTwosidedState(elt)
{
  document.getElementById('oneside').hidden = elt.checked;
  document.getElementById('twoside').hidden = !elt.checked;
}

// font section

function putFontNode(fontname, fontNode, menuId, elementId, nodecounter)
{
  if (fontname && fontname.length>0) {
    var options;
    var node = editor.createNode(menuId, fontNode, nodecounter++);
    if (/{/.test(fontname))
    { // fontname in this case is something like "[options]{packagename} [options2]{package2}
      dump("Fontname is "+fontname+"\n");
      var arr = fontname.split(':');
      var match;
      var re = /\[?([\w\,\=\ ]*)\]?\{([\w\,\ ]+)\}/;
      var i;
      var pri;
      for (i = 0; i < arr.length; i++)
      {
        match = re.exec(arr[i]);
        dump(menuId+", pattern is "+arr[i]+"\n");//+" match is "+match?match.toString():"null"+"\n");
        if (match)
        {
          if (match[1])
          {
            node.setAttribute("options",match[1]);
            dump("options are "+match[1]+"\n");
          }
          if (match[2])
          {
            node.setAttribute("package",match[2]);
            dump("package name is "+match[2]+"\n");
          }
          if (menuId=="mathfont") pri = 50;
          else pri = 60;
          node.setAttribute("pri",pri);
        } else dump("Match is null\n");
      }
    }
    else if (elementId.length > 0) {
      node.setAttribute('name',fontname);
      options = document.getElementById(elementId).value;
      node.setAttribute('options',options)
      node.setAttribute('ot',"1");
    }
  }
}



function saveFontSpecs(docFormatNode)
{
  // we don't want to generate anything unless at least one font was defined.
  var fontspecList = docFormatNode.getElementsByTagName('fontchoices');
  var i;
  var node;
  var options;
  if (fontspecList) 
  {
    for (i = fontspecList.length - 1; i >= 0; i--)
      editor.deleteNode(fontspecList[i])
  }
  var fontids = ["mathfontlist","mainfontlist","sansfontlist","fixedfontlist","f1name","f2name","f3name"];
  var fontchoices = ["","","","","","",""];
  var goahead = false;
  for (i = 0; i < fontids.length; i++)
  {
    fontchoices[i] = document.getElementById(fontids[i]).value;
    if (fontchoices[i].length > 0) goahead = true;
  }
  if (!goahead) return;
  
  var fontNode = editor.createNode('fontchoices', docFormatNode,0);
  var nodecounter = 0;
  var internalname;
  var fontname;
  fontname = fontchoices[0];
// putFontNode(fontname,, menuId, elementId, nodecounter)  
  putFontNode(fontname, fontNode, "mathfont", "", nodecounter)
  fontname = fontchoices[1];
  putFontNode(fontname, fontNode, "mainfont", "romannative", nodecounter)
  fontname = fontchoices[2];
  putFontNode(fontname, fontNode, "sansfont", "sansnative", nodecounter)
  fontname = fontchoices[3];
  putFontNode(fontname, fontNode, "fixedfont", "fixednative", nodecounter)
  fontname = fontchoices[4];
  internalname = document.getElementById('f1name').value;
  if (fontname.length > 0 && internalname.length > 0)
  {
    node = editor.createNode('x1font', fontNode, nodecounter++);
    node.setAttribute('internalname', internalname);
    node.setAttribute('name', fontname);
    options = document.getElementById('x1native').value;
    node.setAttribute('options',options);
    node.setAttribute('ot',"1");
  }
  fontname = fontchoices[5];
  internalname = document.getElementById('f2name').value;
  if (fontname.length > 0 && internalname.length > 0)
  {
    node = editor.createNode('x2font', fontNode, nodecounter++);
    node.setAttribute('internalname', internalname);
    node.setAttribute('name', fontname);
    options = document.getElementById('x2native').value;
    node.setAttribute('options',options);
    node.setAttribute('ot',"1");
  }
  fontname = fontchoices[6];
  internalname = document.getElementById('f3name').value;
  if (fontname.length > 0 && internalname.length > 0)
  {
    node = editor.createNode('x3font', fontNode, nodecounter++);
    node.setAttribute('internalname', internalname);
    node.setAttribute('name', fontname);
    options = document.getElementById('x3native').value;
    node.setAttribute('options',options);
    node.setAttribute('ot',"1");
  }
}

function getFontSpecs(node)
{
// node is the fontspecs node, or null
  var textbox;
  var options;
  var subnode;
  if (node)
  {
    subnode = node.getElementsByTagName('mainfont')[0];
    document.getElementById('mainfontlist').value = subnode.getAttribute('name');
    options = subnode.getAttribute('options');
    if (options)
    {
      options = trimBlanks(options);
      if (options.length > 0)
      {
        textbox = document.getElementById('romannative');
        textbox.value = options;
        onOptionTextChanged( textbox );
      }
    }
    subnode = node.getElementsByTagName('sansfont')[0];
    if (subnode)
    {
      document.getElementById('sansfontlist').value = subnode.getAttribute('name');
      options = subnode.getAttribute('options');
      if (options)
      {
        options = trimBlanks(options);
        if (options.length > 0)
        {
          textbox = document.getElementById('sansnative');
          textbox.value = options;
          onOptionTextChanged( textbox );
        }
      }
    }   
    subnode = node.getElementsByTagName('fixedfont')[0];
    if (subnode)
    {
      document.getElementById('fixedfontlist').value = subnode.getAttribute('name');
      options = subnode.getAttribute('options');
      if (options)
      {
        options = trimBlanks(options);
        if (options.length > 0)
        {
          textbox = document.getElementById('fixednative');
          textbox.value = options;
          onOptionTextChanged( textbox );
        }
      }
    }   
    subnode = node.getElementsByTagName('x1font')[0];
    if (subnode  && subnode.getAttribute('internalname').length > 0)
    {
      document.getElementById('f1name').value = subnode.getAttribute('internalname');
      document.getElementById('x1fontlist').value = subnode.getAttribute('name');
      options = subnode.getAttribute('options');
      if (options)
      {
        options = trimBlanks(options);
        if (options.length > 0)
        {
          textbox = document.getElementById('x1native');
          textbox.value = options;
          onOptionTextChanged( textbox );
        }
      }
    }   
    subnode = node.getElementsByTagName('x2font')[0];
    if (subnode  && subnode.getAttribute('internalname').length > 0)
    {
      document.getElementById('f2name').value = subnode.getAttribute('internalname');
      document.getElementById('x2fontlist').value = subnode.getAttribute('name');
      options = subnode.getAttribute('options');
      if (options)
      {
        options = trimBlanks(options);
        if (options.length > 0)
        {
          textbox = document.getElementById('x2native');
          textbox.value = options;
          onOptionTextChanged( textbox );
        }
      }
    }   
    subnode = node.getElementsByTagName('x3font')[0];
    if (subnode  && subnode.getAttribute('internalname').length > 0)
    {
      document.getElementById('f3name').value = subnode.getAttribute('internalname');
      document.getElementById('x3fontlist').value = subnode.getAttribute('name');
      options = subnode.getAttribute('options');
      if (options)
      {
        options = trimBlanks(options);
        if (options.length > 0)
        {
          textbox = document.getElementById('x3native');
          textbox.value = options;
          onOptionTextChanged( textbox );
        }
      }
    }   
  } 
}


function trimBlanks( astring)    // this assumes blanks will appear only at the ends of the string, and it will
                                 // remove any blanks in the interior of the string
{
  return astring.replace(/\s/gm,"");
}

function parseOneOption( astring ) // the string is of the form XXXX=yyyy or XXXX={yyyy,zzzz}
{
  var dataObj = new Object;
  var arr = astring.split("=");
  if (arr.length <= 1) return null;
  dataObj.name = arr[0];
  var re = /\{|\}/gm
  var options = arr[1].replace(re,"");
  dataObj.options = options.split(",");
  return dataObj;
}


// parse the contents of a textbox. The menulist and checkbox to update are gotten by varying 
// the id of the textbox in predetermined ways. Returns an array of objects with members name (a string)
// and options, an array (often of length 1) of strings.                                      
function parseNativeFontString( instring )
{
  var i = 0; var ch;
  var nesting = 0;
  var pairlist = new Array();
  while (i < instring.length)
  {
    ch = instring[i];
    if (ch == "{"[0]) nesting++;
    if (ch == "}"[0]) --nesting;
    if (nesting == 0 && ch == ",")  // we have hit a comma at the outer level
    {
      pairlist.push(trimBlanks(instring.substr(0,i)));
      instring = instring.substr(i+1, instring.length-i);
      i = -1;
    }
    i++;
  }
  instring = trimBlanks(instring);
  if (instring.length >0) pairlist.push(instring);
  // now pairlist consists of a list of strings, presumably of the form 'AAAA=xxx' or 
  // 'AAAA={xxxx,yyyy}'
  pairlist = pairlist.map(trimBlanks);
  pairlist = pairlist.map(parseOneOption);
  return pairlist;
}

function onOptionTextChanged( textbox )
{
  var id = textbox.id;
  var base = id.substring(0,id.length - 6); // 6 is the length of 'native'
  var oldstylecheckbox = document.getElementById(base+'oldstylenums');
  var swashcheckbox = document.getElementById(base+'swash');
  var instring = textbox.value;
  var objarray = parseNativeFontString(instring);
  var i,j;
  var oldstyle = false;
  var swash = false;
  for (i = 0; i < objarray.length; i++)
  {
    if (objarray[i].name == "Numbers")
    {
      for (j = 0; j < objarray[i].options.length; j++)
      {
        if (objarray[i].options[j] == "OldStyle") oldstyle = true;
        break;
      }
    }
    else if (objarray[i].name == "Contextuals")
    {
      for (j = 0; j < objarray[i].options.length; j++)
      {
        if (objarray[i].options[j] == "Swash") swash = true;
        break;
      }
    }
  }
  try
  {
    swashcheckbox.checked = swash;
    oldstylecheckbox.checked = oldstyle;
  }
  catch(e) {
    dump(e+"\n");
  }
}     
    
function addOptionObject ( objarray, obj ) // objarray is an array of objects
                                      // with a name (string) and options (array of strings)
                                      // This adds the new object (with only one option in its array),
                                      // consolidating if necessary
{
  var i,j;
  var alreadythere = false;
  for (i = 0; i< objarray.length; i++)
  {
    if (objarray[i] && obj.name == objarray[i].name)
    {
      for (j = 0; j< objarray[i].options.length; j++)
      {
        if (objarray[i].options[j] == obj.options[0])
        {
         alreadythere = true;
         break;
        }
      }
      if (!alreadythere)
        objarray[i].options.push(obj.options[0]);
      return;
    }
  }
  // if we get here there is no current object with the same name
  // simply add this one to the list
  objarray.push(obj);
}

function removeOptionObject( objarray, obj)
{
  var i,j;
//  var alreadythere = false;
  for (i = 0; i< objarray.length; i++)
  {
    if (objarray[i] && obj.name == objarray[i].name)
    {
      for (j = 0; j< objarray[i].options.length; j++)
      {
        if (objarray[i].options[j] == obj.options[0])
        {
          objarray[i].options.splice(j,1);
          // check to see if we have deleted the last option associated to this name
          if (objarray[i].options.length == 0)
            objarray.splice(i,1);
          return;
        }
      }
    }
  }
} 

function stringFrom (objectArray)
{
  var returnValue = "";
  var needcomma = false;
  var i, j;
  var obj;
  for (i = 0; i < objectArray.length; i++)
  {
    obj = objectArray[i];
    if (obj)
    {
      if (needcomma) returnValue += ",\n";
      needcomma = true;
      returnValue += obj.name + "=";                 
      if (obj.options.length > 1) returnValue += "{";
      if (obj.options.length > 0) returnValue += obj.options[0];
      for (j = 1; j < obj.options.length; j++)
        returnValue += ","+obj.options[j];
      if (obj.options.length > 1) returnValue += "}";
    }
  }
  return returnValue;
}
         
   
function onCheck( checkbox ) // the checkbox is for old style nums or swashes
{
  var id = checkbox.id;
  var optionObj = new Object;
  var base;
  if (id[id.length-1] == "s")
  {
    base = id.substring(0,id.length - 12); // 6 is the length of 'oldstylenums'
    optionObj.name="Numbers";
    optionObj.options=["OldStyle"];
  }
  else
  {
    base = id.substring(0,id.length - 5); // 6 is the length of 'swash'
    optionObj.name="Contextuals";
    optionObj.options=["Swash"];
  }
  var objarray = parseNativeFontString(document.getElementById(base+"native").value);
  if (checkbox.checked) addOptionObject(objarray,optionObj);
  else removeOptionObject(objarray,optionObj);
  document.getElementById(base+"native").value = stringFrom(objarray);
}
  

// font section
var gFontMenuInitialized = new Object;
var gSystemFonts;
var gSystemFontCount;

function  getOTFontlist() 
{ 
  if (!gSystemFonts)
  {
    // Build list of all system fonts once per editor
    try 
    {
      var dsprops = Components.classes["@mozilla.org/file/directory_service;1"].createInstance(Components.interfaces.nsIProperties);
      var fontlistfile=dsprops.get("ProfD", Components.interfaces.nsIFile);
      fontlistfile.append("fontfamilies.txt");
      var stream;
      stream = Components.classes["@mozilla.org/network/file-input-stream;1"];
      stream = stream.createInstance(Components.interfaces.nsIFileInputStream);
      stream.init(fontlistfile,1,0,0);
      var s2 = Components.classes["@mozilla.org/scriptableinputstream;1"];
      s2 = s2.createInstance(Components.interfaces.nsIScriptableInputStream);
      s2.init(stream);
      var bytes = s2.available();
      var buffer = s2.read(bytes);
      gSystemFonts = buffer.split("\n");
      gSystemFontCount = gSystemFonts.length;
    }
    catch(e) { 
       3+5;
    }
  }
}

function initSystemFontMenu(menuPopupId, fUseOpenType)
{
  try
  {
    var menuPopup = document.getElementById(menuPopupId).firstChild;
    // fill in the menu only once...
    if (gFontMenuInitialized[menuPopupId ])
      return;
    if (fUseOpenType) gFontMenuInitialized[menuPopupId ] = menuPopupId;
    var fonttype = "textfonts";
    var filter;
  
    if (menuPopupId == "sansfontlist")
    {
      filter = "sans";
    } else if (menuPopupId == "fixedfontlist")
    {
      filter = "fixed";
    } else if (menuPopupId == "mathfontlist")
    {
      fonttype = "mathfonts";
      filter = "";
    } else filter = "main";
    initSystemOldFontMenu(menuPopup, fonttype, filter);
    if (fUseOpenType)
    {
      getOTFontlist();
      for (var i = 0; i < gSystemFontCount; ++i)
      {
        if (gSystemFonts[i] != "")
        {
          var itemNode = document.createElementNS(XUL_NS, "menuitem");
          itemNode.setAttribute("label", gSystemFonts[i]);
          itemNode.setAttribute("value", gSystemFonts[i]);
          menuPopup.appendChild(itemNode);
        }
      }
    }
  }
  catch(e)
  {
    dump(e + "\n");
  }
}
    
  
//function onMenulistFocus(menulist)
//{
//  var tempnodes = menulist.getElementsByTagName("menupopup");
//  var tempnode = tempnodes[0];
//  if (tempnode.id != "systemfontlist")
//  {
//    var savednode = tempnode.cloneNode(true);
//    var newnode = document.getElementById("systemfontlist");
//    newnode.parentNode.replaceChild(savednode, newnode);
//    menulist.replaceChild(newnode,tempnode);
//  }
//}

// functions for typesetsectionsoverlay

var sectionUnits;


function getSectionFormatting(sectitlenodelist, sectitleformat)
// sectitlenodelist is a list of nodes of sectitleformat nodes, and sectitleformat is a new object to hold the results from these nodes.
{
  var i;
  var node;
  var level;
  var dialogbase;
  var xmlcode;
  var sw = "http://www.sciword.com/namespaces/sciword";
  if (sectitlenodelist)
  {
    for (i=0; i< sectitlenodelist.length; i++)
    {
      node = sectitlenodelist[i];
      level = node.getAttribute("level");
      sectitleformat[level] = new Object();
      try {
        dialogbase = node.getElementsByTagNameNS(sw,"dialogbase")[0];
        if (dialogbase)
        {
          var ser = new XMLSerializer();
          xmlcode = ser.serializeToString(dialogbase);
          xmlcode = xmlcode.replace(/#1/,"#T");
          var pattern = "\\the"+level;
          xmlcode = xmlcode.replace(pattern, "#N");
        }
//        var re=/<sw:dialogbase[^>]*>/;
//        var l;
//        do
//        {
//          l = xmlcode.length;
//          xmlcode = xmlcode.replace(re,"").replace("</sw:dialogbase>",'');
//        } while (xmlcode.length > 0 && xmlcode.length < l);
        if (!xmlcode) xmlcode="";
        sectitleformat[level].proto = xmlcode;
        sectitleformat[level].newPage = (node.getAttribute("newPage")=="true");
        sectitleformat[level].sectStyle = node.getAttribute("sectStyle");
        sectitleformat[level].align = node.getAttribute("align");
        sectitleformat[level].units = node.getAttribute("units");
        sectitleformat[level].lhindent = node.getAttribute("lhindent");
        sectitleformat[level].rhindent = node.getAttribute("rhindent");
        // now check for rules and spaces
        var rulenodelist = node.getElementsByTagName("toprules");
        var color;
        if (rulenodelist && rulenodelist.length >0)
        {
          sectitleformat[level].toprules = new Array(rulenodelist.length);
          for (i>0; i < rulenodelist.length; i--)
          {
            sectitleformat[level].toprules[i].height = rulenodelist[i].getAttribute("height");
            sectitleformat[level].toprules[i].width = rulenodelist[i].getAttribute("width");
            sectitleformat[level].toprules[i].role = rulenodelist[i].getAttribute("role");
            color = rulenodelist[i].getAttribute("color");
            if (!color) color = "black";
            sectitleformat[level].toprules[i].color = color;
          }
        }
        rulenodelist = node.getElementsByTagName("bottomrules");
        if (rulenodelist && rulenodelist.length >0)
        {
          sectitleformat[level].bottomrules = new Array(rulenodelist.length);
          for (i>0; i < rulenodelist.length; i--)
          {
            sectitleformat[level].bottomrules[i].height = rulenodelist[i].getAttribute("height");
            sectitleformat[level].bottomrules[i].width = rulenodelist[i].getAttribute("width");
            color = rulenodelist[i].getAttribute("color");
            if (!color) color = "black";
            sectitleformat[level].bottomrules[i].color = color;
          }
        }
      }
      catch(e)
      {
        dump("exception in getSectionFormatting: "+e+"\n");
      }
    }
  }
  switchSectionTypeImp(null, document.getElementById("sections.name").label.toLowerCase());
  displayTextForSectionHeader();
}

var currentSectionType = "";
function switchSectionType()
{
  var from = currentSectionType;
  var to = document.getElementById("sections.name").label.toLowerCase();
  return switchSectionTypeImp(from, to);
}


function switchSectionTypeImp(from, to)
{
  var i;
  currentSectionType = to;
  var boxlist;
  if (from)
  {
    if (!sectitleformat[from]) sectitleformat[from] = new Object();
    var sec = sectitleformat[from];
    sec.newPage = document.getElementById("sectionstartnewpage").checked;
    sec.sectStyle = document.getElementById("sections.style").value;
    sec.align = document.getElementById("sections.align").value; 
    sec.units = document.getElementById("secoverlay.units").value; 
    sec.lhindent = document.getElementById("tbsectleftheadingmargin").value; 
    sec.rhindent = document.getElementById("tbsectrightheadingmargin").value; 
    //toprules, bottomrules, and proto, if they have been changed, have been updated by subdialogs 
  }
  //Now initialize for the new section type
  if (to)
  {
    if (from != to)
    {
      if (!sectitleformat[to])
      {
        sectitleformat[to] = new Object();
        return; // the dialog will default to the last sections headings. Can we do better? BBM
      }
      sec = sectitleformat[to];
      var newpage = sec.newPage
      if (!newpage) newpage = false;
      document.getElementById("sectionstartnewpage").checked = newpage;
      document.getElementById("sections.style").value = sec.sectStyle;
      document.getElementById("sections.align").value = sec.align; 
      document.getElementById("secoverlay.units").value = sec.units;
      sectionUnits = sec.units; 
      document.getElementById("tbsectleftheadingmargin").value = sec.lhindent; 
      document.getElementById("tbsectrightheadingmargin").value = sec.rhindent; 
      if (sec.toprules && sec.toprules.length > 0)
      {
        boxlist = document.getElementById("toprules").getElementsByTagName("vbox");
        for (i=0; i < Math.min(boxlist.length, sec.toprules.length); i++)
        {
          boxlist[i].setAttribute("hidden", "false");
          boxlist[i].setAttribute("role", sec.toprules[i].role);
          boxlist[i].setAttribute("width", sec.toprules[i].width);
          boxlist[i].setAttribute("height", sec.toprules[i].height);
          boxlist[i].setAttribute("color", sec.toprules[i].color);
        }
        for (i = sec.toprules.length; i<boxlist.length; i++);
          boxlist[i].setAttribute("hidden", "true");
      }
      if (sec.bottomrules && sec.bottomrules.length > 0)
      {
        boxlist = document.getElementById("bottomrules").getElementsByTagName("vbox");
        for (i=0; i < Math.min(boxlist.length, sec.bottomrules.length); i++)
        {
          boxlist[i].setAttribute("hidden", "false");
          boxlist[i].setAttribute("role", sec.bottomrules[i].role);
          boxlist[i].setAttribute("width", sec.bottomrules[i].width);
          boxlist[i].setAttribute("height", sec.bottomrules[i].height);
          boxlist[i].setAttribute("color", sec.bottomrules[i].color);
        }
        for (i = sec.bottomrules.length; i<boxlist.length; i++);
          boxlist[i].setAttribute("hidden", "true");
      }
      displayTextForSectionHeader();
      setalign(sec.align);
      settopofpage(document.getElementById("sectionstartnewpage"));
    }
  }
}

function saveSectionFormatting( docFormatNode, sectitleformat )
{
  // get the list of section-like objects
  var menulist = document.getElementById("sections.name");
  var itemlist = menulist.getElementsByTagName("menuitem");
  var sectiondata;
  // should actually look to see if the titlesec package is already in the document
  var bRequiresPackage = true;
  var reqpackageNode;
  switchSectionTypeImp(currentSectionType,null);
  for (var i = 0; i < itemlist.length; i++)
  {
    var name;
    try {
      name = itemlist[i].getAttribute("id").split(".")[1];
    }
    catch(e) {
      continue;
    }
    name = name.toLowerCase();
    if (sectitleformat[name])
    {
      if (bRequiresPackage)
      {
        bRequiresPackage = false;
        reqpackageNode = editor.createNode('requirespackage',docFormatNode, 0);
        reqpackageNode.setAttribute('package',"titlesec");
        reqpackageNode.setAttribute('options',"calcwidth");
      } 
      lineend(docFormatNode, 1);
      sectiondata = sectitleformat[name]; 
      var stNode = editor.createNode('sectitleformat', docFormatNode,0);
      stNode.setAttribute('level', name);
      stNode.setAttribute('sectStyle', sectiondata.sectStyle);
      stNode.setAttribute('align', sectiondata.align);
      stNode.setAttribute('units', sectiondata.units);
      stNode.setAttribute('newPage', sectiondata.newPage?"true":"false");
      stNode.setAttribute('lhindent', sectiondata.lhindent);
      stNode.setAttribute('rhindent', sectiondata.rhindent);
      lineend(stNode, 2);
      var proto = editor.createNode('titleprototype', stNode, 0);
      var fragment = sectiondata.proto;
      if (fragment)
      {
        // replace #N with \the(section, subsection, etc) and #T with #1
        fragment = fragment.replace(/#N/,"\\the"+name);
        fragment = fragment.replace(/#T/,"#1");
        dump("Contents being saved as section title prototype: "+fragment+"\n");
        var parser = new DOMParser();
        var doc = parser.parseFromString(fragment,"application/xhtml+xml");
        proto.appendChild(doc.documentElement);
      }
      // stNode.setAttribute( -- find the section format type: hang, runin, etc.
      docFormatNode.appendChild(stNode);
      // to do: the rule lists
    }
  }  
}


function getBaseNodeForIFrame( )
{
  var sw = "http://www.sciword.com/namespaces/sciword";
  var iframe = document.getElementById("sectiontextarea");
  if (!iframe) return;
  var doc = iframe.contentDocument;
  var theNodes = doc.getElementsByTagNameNS(sw,"dialogbase");
  var theNode;
  if (theNodes) theNode = theNodes[0]; 
  else 
  {
    theNodes = doc.getElementsByTagNameNS(sw,"para");
    if (theNodes) theNode = theNodes[0]; 
  }
  if (!theNode)
  {
    var bodies = doc.getElementsByTagName("body");
    if (bodies) for ( var i = 0; i < bodies.length; i++)
    {
      theNode = bodies[i];
      if (theNode.nodeType == theNode.ELEMENT_NODE)
        return theNode;
     }
  }
  return theNode;
}

function switchSectionUnits()
{
  var newUnit = document.getElementById("secoverlay.units").selectedItem.value;
  var factor = convert(1, sectionUnit, newUnit);
  dump("section factor is "+factor+"\n");
  sectScale = sectScale/factor;
  sectionUnit = newUnit; // this has to be set before unitRound and setDecimalPlaces are called
  sectSetDecimalPlaces();
  document.getElementById("tbsectrightheadingmargin").value = unitRound(factor*document.getElementById("tbsectrightheadingmargin").value);
  document.getElementById("tbsectleftheadingmargin").value = unitRound(factor*document.getElementById("tbsectleftheadingmargin").value); 
}


function sectSetDecimalPlaces() // and increments
{
  var places;
  var increment;
  var elt;
  var i;
  var s;
  switch (sectionUnit) {
    case "in" : increment = .1; places = 2; break;
    case "cm" : increment = .1; places = 2; break;
    case "mm" : increment = 1; places = 1; break;
    case "pt" : increment = 1; places = 1; break;
    default : increment = 1; places = 1; break;
  }
  elt = document.getElementById("tbsectrightheadingmargin");
  elt.setAttribute("increment", increment);
  elt.setAttribute("decimalplaces", places);
  elt = document.getElementById("tbsectleftheadingmargin");
  elt.setAttribute("increment", increment);
  elt.setAttribute("decimalplaces", places);
}    


function displayTextForSectionHeader()
{
  var secname = document.getElementById("sections.name").label.toLowerCase();
  var basepara;
//	var width;
	var height;
  basepara = getBaseNodeForIFrame();
  var strContents;
  if (sectitleformat[secname]) strContents = sectitleformat[secname].proto;
	if (strContents && strContents.length > 0)
	{
	  var parser = new DOMParser();
	  var doc = parser.parseFromString(strContents,"application/xhtml+xml");
	  basepara.parentNode.replaceChild(doc.documentElement, basepara);
		basepara = getBaseNodeForIFrame();
		var htmlNode=basepara.parentNode.parentNode;
		var boxObject=htmlNode.ownerDocument.getBoxObjectFor(htmlNode);
//		width = boxObject.width;
		height = boxObject.height;
    document.getElementById("tso_template").setAttribute("selectedIndex","1");
	}
	else
	{
	  height = 20;
    document.getElementById("tso_template").setAttribute("selectedIndex","0");
//		width = 200;
  }
	var iframecontainer = document.getElementById("sectiontextareacontainer");
//	setStyleAttributeOnNode(iframecontainer,"height",Number(height)+"px");
//	setStyleAttribute(iframecontainer,"width",width+sectionUnit);
	setStyleAttribute(iframecontainer,"height",Number(height)+"px");
}


function refresh() // a version of displayTextForSectionHeader designed to be used as a callback
{
//  var strContents = sectitleformat[sectitleformat.currentLevel];
//  var parser = new DOMParser();
//  var doc = parser.parseFromString(strContents,"application/xhtml+xml");
//	sectitleformat.destNode.parentNode.replaceChild(doc.documentElement, sectitleformat.destNode);
//  sectitleformat.destNode = null;
}
      
function textEditor()
{
  var secname=document.getElementById("sections.name").label;
  var units=document.getElementById("secoverlay.units").value;
  sectitleformat.refresh = refresh;
  sectitleformat.destNode = getBaseNodeForIFrame();
  sectitleformat.currentLevel = secname.toLowerCase();
  window.openDialog("chrome://prince/content/sectiontext.xul", "sectiontext", "modal,chrome,close,titlebar,alwaysRaised", sectitleformat,
      secname, units);																									 
  displayTextForSectionHeader();
}
      
function setalign(which)
{
  var element;
  var otherelement;
  var sectionparts = document.getElementById("sectionparts");
  if (which == "c")
  {
    sectionparts.setAttribute("pack","center");
    document.getElementById("leftalignment").setAttribute("msicollapsed","true");
		document.getElementById("leftalignment").setAttribute("width","1px");
    document.getElementById("rightalignment").setAttribute("msicollapsed","true");
		document.getElementById("rightalignment").setAttribute("width","1px");
    document.getElementById("notcenteralignment").setAttribute("msicollapsed","true");
    element = document.getElementById("sectleftheadingmargin");
    otherelement = document.getElementById('sectrightheadingmargin');
    //element.setAttribute("role", "spacer");
    document.getElementById("sectleftmargin").setAttribute("flex", "1");
    document.getElementById("sectleftmargin").setAttribute("width", "40px");
		document.getElementById("sectrightmargin").setAttribute("flex", "1");
		document.getElementById("sectrightmargin").setAttribute("width", "40px");
  }
  else
  {
    document.getElementById("notcenteralignment").setAttribute("msicollapsed","false");
    if (which == "l")
    {
      sectionparts.setAttribute("pack", "start");
      var width = document.getElementById("tbsectleftheadingmargin").value;
			if (Number(width) ==  NaN) width=30;                												 
      document.getElementById("leftalignment").setAttribute("msicollapsed","false");
      document.getElementById("leftalignment").setAttribute("width",width+"px");
      document.getElementById("rightalignment").setAttribute("msicollapsed","true");
		  document.getElementById("rightalignment").setAttribute("width","1px");
      element = document.getElementById("sectleftheadingmargin");
      otherelement = document.getElementById('sectrightheadingmargin');
	    document.getElementById("sectleftmargin").setAttribute("flex", "0");
	    document.getElementById("sectleftmargin").setAttribute("width", "40px");
			document.getElementById("sectrightmargin").setAttribute("flex", "1");
			document.getElementById("sectrightmargin").setAttribute("width", "40px");
      element.setAttribute("role", "sectionmargin");
    }
    else if (which == "r")
    {
      sectionparts.setAttribute("pack", "end");
      var rwidth = document.getElementById("tbsectrightheadingmargin").value;
			if (Number(rwidth) ==  NaN) width=30;                												 
      document.getElementById("leftalignment").setAttribute("msicollapsed","true");
		  document.getElementById("leftalignment").setAttribute("width","1px");
      document.getElementById("rightalignment").setAttribute("msicollapsed","false");
      document.getElementById("rightalignment").setAttribute("width",rwidth+"px");
      element = document.getElementById("sectrightheadingmargin");
      otherelement = document.getElementById('sectleftheadingmargin');
			document.getElementById("sectrightmargin").setAttribute("flex","0");
	    document.getElementById("sectleftmargin").setAttribute("flex","1");
	    document.getElementById("sectleftmargin").setAttribute("width", "40px");
			document.getElementById("sectrightmargin").setAttribute("width", "40px");
      element.setAttribute("role", "sectionmargin");
    }
  }
  //otherelement.setAttribute("role", "spacer");
}
    

function clearselection(element)
{
 if (element.nodeType != element.ELEMENT_NODE) return;
 if (element.getAttribute('sectionselected') != null)
    element.setAttribute('sectionselected', 'false');
  var node;
  for (var i = 0; i < element.childNodes.length; i++) 
  { 
    clearselection(element.childNodes[i]);
  }
} 
var lastselected = null;   
function toggleselection( element )
{
  lastselected = element;
  var root = document.getElementById("sectionparts");
  var NCAbroadcaster = document.getElementById("notcenteralignment");
  var TOBbroadcaster=document.getElementById("toporbottommargin");
  var NMbroadcaster=document.getElementById("notmargin");
  clearselection(root);
  element.setAttribute('sectionselected','true');
  if (element.id == "sectleftheadingmargin" || element.id=="sectrightheadingmargin")
  {
    NCAbroadcaster.setAttribute("disabled","false");
    TOBbroadcaster.setAttribute("disabled", "true");
    NMbroadcaster.setAttribute("disabled", "true");
  }
  else
  {
    NCAbroadcaster.setAttribute("disabled","true");
    NMbroadcaster.setAttribute("disabled", "false");
    if (element.id == "topmargin" || element.id == "bottommargin")
    {
      TOBbroadcaster.setAttribute("disabled", "false");
    }
    else 
      TOBbroadcaster.setAttribute("disabled", "true");
  } 
}

function settopofpage( checkbox )
{
  var broadcaster = document.getElementById("atpagetop");
  if (!broadcaster) return;
  if (checkbox.checked)
    broadcaster.hidden = true;
  else
    broadcaster.hidden=false;
}
    
function sectLayout(id, tbid)
{
  var textbox = document.getElementById(tbid);
  var box = document.getElementById(id);
  var dim = Number(textbox.value);
  secSetWidth(box,dim);
}		 

function secSetWidth(box, dim)
{
  // dim is in the current units -- add conversion to pixels
	var theotherbox;
	if (box.id == "sectleftheadingmargin")
	{
	  theotherbox = document.getElementById("sectleftmargin");
	  if (dim < 0) {
	    box.setAttribute("width", "0");
			theotherbox.setAttribute("width",40-Number(-dim));
		}
		else
		{
			box.setAttribute("width",Number(dim));
			theotherbox.setAttribute("width",40);
		}
  }
	else
	{
	  theotherbox = document.getElementById("sectrightmargin");
	  if (dim < 0) {
	    box.setAttribute("width", "0");
			theotherbox.setAttribute("width",40-Number(-dim));
		}
		else
		{
			box.setAttribute("width",Number(dim));
			theotherbox.setAttribute("width",40);
		}
  }
}		  

function addrule()
{
  // find which is selected
  if (!lastselected) return;
  var boxlist = lastselected.getElementsByTagName("vbox");
  var nextbox;
  for (var i = 1; i < boxlist.length; i++)    // we start at 1 because the first is the parent of the others
  {
    if (boxlist[i].getAttribute("hidden") == "true")
    {
      nextbox = boxlist[i];
      break;
    }
  }
  if (!nextbox) return;
  nextbox.setAttribute("role","rule");
  nextbox.setAttribute("hidden","false");
  nextbox.setAttribute("style","height:3px;background-color:black;");
  window.openDialog("chrome://prince/content/addruleforsection.xul", "addruleforsection", "chrome,close,titlebar,alwaysRaised",nextbox, "black", sectionUnit);
}


function addspace()
{
  // find which is selected
  if (!lastselected) return;
  var boxlist = lastselected.getElementsByTagName("vbox");
  var nextbox;
  for (var i = 1; i<boxlist.length; i++)
  {
    if (boxlist[i].getAttribute("hidden") == "true")
    {
      nextbox = boxlist[i];
      break;
    }
  }
  if (!nextbox) return;
  nextbox.setAttribute("role","vspace");
  nextbox.setAttribute("hidden","false");
  nextbox.setAttribute("style","height:6px;background-color:silver;");
	window.openDialog("chrome://prince/content/vspaceforsection.xul", 
	  "vspaceforsection", "chrome,close,titlebar,alwaysRaised",nextbox, sectionUnit);
}

function removeruleorspace()
{
  // find which is selected
  if (!lastselected) return;
  var boxlist = lastselected.getElementsByTagName("vbox");
  var lastbox;
  for (var i = 1; i<boxlist.length; i++)
  {
    if (boxlist[i].getAttribute("hidden") != "true")
      lastbox = boxlist[i];
    else break;
  }
  if (!lastbox) return;
  lastbox.setAttribute("hidden","true");
}

function reviseruleorspace(element)
{
  if (element.getAttribute("role")=="rule")
    window.openDialog("chrome://prince/content/addruleforsection.xul", 
      "addruleforsection", "chrome,close,titlebar,alwaysRaised",element, element.getAttribute("color"), sectionUnit);
  else if (element.getAttribute("role")=="vspace")
    window.openDialog("chrome://prince/content/vspaceforsection.xul", 
      "vspaceforsection", "chrome,close,titlebar,alwaysRaised",element, sectionUnit);
}                                

function initSystemOldFontMenu(menuPopup, fonttype, filter)
{    
  var path = "resource://app/res/xml/mathfonts.xml";
  var myXMLHTTPRequest = new XMLHttpRequest();
  myXMLHTTPRequest.open("GET", path, false);
  myXMLHTTPRequest.send(null);
  var doc = myXMLHTTPRequest.responseXML;
  if (!doc) return;
  var ptr = doc.documentElement.getElementsByTagName(fonttype)[0];
  if (!ptr) return;
  ptr = ptr.firstChild;
  while (ptr)
  {
    while (ptr && (ptr.nodeType != 1))
    {
      ptr = ptr.nextSibling;
    }
    if (!ptr) break;
    if (filter.length > 0 && !ptr.hasAttribute(filter))
    {
      ptr = ptr.nextSibling;
      continue;
    }                                                
    var itemNode = document.createElementNS(XUL_NS, "menuitem");
    itemNode.setAttribute("label", ptr.getAttribute("name"));
    itemNode.setAttribute("value", ptr.getAttribute("package"));
    itemNode.setAttribute("tooltip", ptr.getAttribute("description"));                                                                 
    menuPopup.appendChild(itemNode);
    ptr = ptr.nextSibling;
  }
  var separator = document.createElementNS(XUL_NS, "menuseparator");
  menuPopup.appendChild(separator);
}

function changeOpenType()
{
  document.getElementById("opentypeok").setAttribute("hidden", 
    document.getElementById("useOpenType").checked?"false":"true");                                                                      
}  
    