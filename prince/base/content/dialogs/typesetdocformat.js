 
var widthElements;
var heightElements;
var unitConversions;
var pagewidth;   // in mm
var pageheight;
var landscape;
var currentUnit;
var scale = 0.5;

var editor;

function Startup()
{
  editor = GetCurrentEditor();
  if (!editor) {
    window.close();
    return;
  }

  widthElements=new Array("lmargin","bodywidth","colsep", "mnsep","mnwidth","computedrmargin");
  heightElements=new Array("tmargin","header","headersep",
    "bodyheight","footer","footersep", "mnpush", "computedbmargin");
  currentUnit = document.getElementById("docformat.units").selectedItem.value;
  setDecimalPlaces();
  unitConversions = new Object;
  unitConversions.pt=.3514598;  //mm per pt
  unitConversions.in=25.4;  //mm per in
  unitConversions.mm=1; // mm per mm
  unitConversions.cm=10; // mm per cm
  setPageDimensions();
  setDefaults();
  getPageLayout();
  //now we can load the docformat information from the document to override 
  //all or part of the initial state
  OnWindowSizeReset(true);
  dump("current unit = "+currentUnit+"\n");
}

function savePageLayout(docFormatNode)
{
  var pfNode = editor.createNode('pagelayout', docFormatNode,0);
  var nodecounter = 0;
  var units;
  pfNode.setAttribute('latex',true);
  units=document.getElementById('docformat.units').value;
  pfNode.setAttribute('unit',units);
  var node = editor.createNode('requirespackage', pfNode, nodecounter++);
  node.nodeValue = "geometry";
  node = editor.createNode('page', pfNode, nodecounter++);
  node.setAttribute('twoside',document.getElementById('twosides').checked);
  node.setAttribute('paper',document.getElementById('docformat.papersize').value);
  node.setAttribute('width',pagewidth+units);
  node.setAttribute('height',pageheight+units);
  node.setAttribute('landscape',document.getElementById('landscape').checked);
  node = editor.createNode('textregion',pfNode,nodecounter++);
  node.setAttribute('width',document.getElementById('tbbodywidth').value+units);
  node.setAttribute('height', document.getElementById('tbbodyheight').value+units);
  node = editor.createNode('margin',pfNode,nodecounter++);
  node.setAttribute('left',document.getElementById('tblmargin').value+units);
  node.setAttribute('top', document.getElementById('tbtmargin').value+units);
  node = editor.createNode('columns',pfNode,nodecounter++);
  node.setAttribute('count',(document.getElementById('columns').checked?2:1));
  node.setAttribute('sep',document.getElementById('tbfootersep').value+units);
  node = editor.createNode('marginnote',pfNode,nodecounter++);
//  node.setAttribute('reversed',document.getElementById('*****').checked);
  node.setAttribute('width',document.getElementById('tbmnwidth').value+units);
  node.setAttribute('sep',document.getElementById('tbmnsep').value+units);
  node.setAttribute('push',document.getElementById('tbmnpush').value+units);
  node.setAttribute('hidden', document.getElementById('disablemn').checked);
  node = editor.createNode('header',pfNode,nodecounter++);
  node.setAttribute('height',document.getElementById('tbheader').value+units);
  node.setAttribute('sep',document.getElementById('tbheadersep').value+units);
  node = editor.createNode('footer',pfNode,nodecounter++);
  node.setAttribute('height',document.getElementById('tbfooter').value+units);
  node.setAttribute('sep',document.getElementById('tbfootersep').value+units);
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


function getPageLayout()
{
  var doc = editor.document;
  var preamble = doc.documentElement.getElementsByTagName('preamble')[0];
  if (!preamble) return;
  var docFormatNodeList = preamble.getElementsByTagName('docformat');
  if (!docFormatNodeList) return;
  var node;
  var subnode;
  var i;
  var value;
  for ( i = 0; i < docFormatNodeList.length; i++)
  {
    node = docFormatNodeList[i].getElementsByTagName('page')[0];
    if (node)
    {
      value = node.getAttribute('twoside');
      if (value=='true') document.getElementById('twosides').checked=true;
        else document.getElementById('twosides').checked=false;
      value = node.getAttribute('paper');
      if (value) document.getElementById('docformat.papersize').value = value;
      value = node.getAttribute('width');
      if (value) pagewidth = getNumberValue(value);
      value = node.getAttribute('height');
      if (value) pageheight = getNumberValue(value);
      value = node.getAttribute('landscape');
      if (value=='true') landscape = true; else landscape = false;
      document.getElementById('landscape').checked=landscape;
    }                                                    
    node = docFormatNodeList[i].getElementsByTagName('textregion')[0];
    if (node)
    {
      value = node.getAttribute('width');
      if (value) document.getElementById('tbbodywidth').value=getNumberValue(value);
      value = node.getAttribute('height');
      if (value) document.getElementById('tbbodyheight').value=getNumberValue(value);
    }
    node = docFormatNodeList[i].getElementsByTagName('margin')[0];
    if (node)
    {
      value = node.getAttribute('left');
      if (value) document.getElementById('tblmargin').value=getNumberValue(value);
      value = node.getAttribute('top');
      if (value) document.getElementById('tbtmargin').value=getNumberValue(value);
    }
    node = docFormatNodeList[i].getElementsByTagName('columns')[0];
    if (node)
    {
      value = node.getAttribute('count');
      if (value) document.getElementById('columns').checked=(value?2:1);
      value = node.getAttribute('sep');
      if (value) document.getElementById('tbcolsep').value=getNumberValue(value);
    }
    node = docFormatNodeList[i].getElementsByTagName('marginnote')[0];
    if (node)
    {
      value = node.getAttribute('width');
      if (value) document.getElementById('tbmnwidth').value=getNumberValue(value);
      value = node.getAttribute('sep');
      if (value) document.getElementById('tbmnsep').value=getNumberValue(value);
      value = node.getAttribute('push');
      if (value) document.getElementById('tbmnpush').value=getNumberValue(value);
      value = node.getAttribute('hidden');
      if (value) document.getElementById('disablemn').checked=value;
        else document.getElementById('disablemn').checked=false;
    }
    node = docFormatNodeList[i].getElementsByTagName('header')[0];
    if (node)
    {
      value = node.getAttribute('height');
      if (value) document.getElementById('tbheader').value=getNumberValue(value);
      value = node.getAttribute('sep');
      if (value) document.getElementById('tbheadersep').value=getNumberValue(value);
    }
    node = docFormatNodeList[i].getElementsByTagName('footer')[0];
    if (node)
    {
      value = node.getAttribute('height');
      if (value) document.getElementById('tbfooter').value=getNumberValue(value);
      value = node.getAttribute('sep');
      if (value) document.getElementById('tbfootersep').value=getNumberValue(value);
    }
  }
}

function onAccept()
{
// first check that something is set
  var doc = editor.document;
  var preamble = doc.documentElement.getElementsByTagName('preamble')[0];
// delete any current docformat nodes -- we are replacing them
  var docFormatNodeList = preamble.getElementsByTagName('docformat');
  var i;
  if (docFormatNodeList)
    for (i = docFormatNodeList.length - 1; i >= 0; i--)
      editor.deleteNode(docFormatNodeList[i])
  var newNode = editor.createNode('docformat',preamble,0);
// should check that the user wants to use geometry package.
  if (newNode) savePageLayout(newNode);
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
  var twosided = document.getElementById("twosides").checked;
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


// Page layout stuff  

function setPageDimensions( )
{
  var paper;
  paper = document.getElementById("docformat.papersize").selectedItem.value;
  landscape = document.getElementById("landscape").checked;
  switch (paper) {
    case "letter":   // these dimensions are in millimeters
      pagewidth = 215.9;
      pageheight = 279.4;
      break;
		case "a4":
      pagewidth = 210;
      pageheight = 297;
      break;
		case "screen":
      pagewidth = 225;
      pageheight = 180;
      break;
		case "other":
      pagewidth = 215.9;
      pageheight = 279.4;
      break;
		case "a0":
      pagewidth = 841;
      pageheight = 1189;
      break;
		case "a1":
      pagewidth = 594;
      pageheight = 841;
      break;
		case "a2":
      pagewidth = 420;
      pageheight = 594;
      break;
		case "a3":
      pagewidth = 297;
      pageheight = 420;
      break;
		case "a5":
      pagewidth = 148;
      pageheight = 210;
      break;
		case "a6":
      pagewidth = 105;
      pageheight = 148;
      break;
		case "b0":
      pagewidth = 1000;
      pageheight = 1414;
      break;
		case "b1":
      pagewidth = 707;
      pageheight = 1000;
      break;
		case "b2":
      pagewidth = 500;
      pageheight = 707;
      break;
		case "b3":
      pagewidth = 353;
      pageheight = 500;
      break;
		case "b4":
      pagewidth = 250;
      pageheight = 353;
      break;
		case "b5":
      pagewidth = 176;
      pageheight = 250;
      break;
		case "b6":
      pagewidth = 125;
      pageheight = 176;
      break;
		case "executive":
      pagewidth = 184;
      pageheight = 267;
      break;
		case "legal":
      pagewidth = 216;
      pageheight = 356;
    default:
      pagewidth = 215.9;
      pageheight = 279.4;
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
  document.getElementById("tbpagewidth").value = unitRound(pagewidth);
  document.getElementById("tbpageheight").value = unitRound(pageheight);
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
  var oldscale = scale;
  var width = 1.8*document.getElementById("gb.dimension").boxObject.width;
//  if (!initial) {
//    var winwidth = document.getElementById("docformat.dialog").boxObject.width-20;
//    if (winwidth < width) width = winwidth;
//  }
//  if (width < 2*twomargins) return;
  scale = width/pagewidth;
  if (oldscale != scale)
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
  // overflow occurs if an element goes beyond the edge of the paper.
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
  dump("factor is "+factor+"\n");
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
}

function handleBodyMouseClick(event)
{
  var element = event.target;
  if (event.clientY - element.boxObject.y < element.boxObject.height/2)
    document.getElementById("tbbodywidth").focus();
  else
    document.getElementById("tbbodyheight").focus();
}


function handleChar(event, id)
{
  var element = event.currentTarget;
  if (event.keyCode == event.DOM_VK_UP)
    goUp(id);
  else if (event.keyCode == event.DOM_VK_DOWN)
    goDown(id);
  if (id != "colums") layoutPage(id);
}

function goUp(id)
{
  var element = document.getElementById("tb"+id);
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
  var element = document.getElementById("tb"+id);
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
  var twocolumns = document.getElementById("columns").checked;
  document.getElementById("multicolumn").setAttribute("hidden", !twocolumns);
  document.getElementById("singlecolumn").setAttribute("hidden", twocolumns);
}

function setTwosidedState(elt)
{
  document.getElementById('oneside').hidden = elt.checked;
  document.getElementById('twoside').hidden = !elt.checked;
}