Components.utils.import("resource://app/modules/unitHandler.jsm");

var frameUnitHandler = new UnitHandler();
var sides = ["Top", "Right", "Bottom", "Left"]; // do not localize -- visible to code only
var gFrameTab={};
var scale = 0.25;
var scaledWidthDefault = 50; 
var scaledHeightDefault = 60;
var scaledHeight = scaledHeightDefault;
var scaledWidth = scaledWidthDefault; 
var position = 0;  // left = 1, right = 2, neither = 0
//var unit;




function initFrameTab(dg, element, newElement)
{
  var editorElement = msiGetParentEditorElementForDialog(window);
  var editor = msiGetEditor(editorElement);
  if (!editor) {
    window.close();
    return null;
  }
  // For convenience, map dialog elements to an object
//  currentFrame = element;
  dg.widthInput           = document.getElementById("frameWidthInput");
  dg.heightInput          = document.getElementById("frameHeightInput");
  dg.autoHeightCheck      = document.getElementById("autoHeight");
  dg.autoWidthCheck       = document.getElementById("autoWidth");
  dg.frameUnitMenulist    = document.getElementById("frameUnitMenulist");
  dg.unitList             = document.getElementById("unitList");
  dg.actual               = document.getElementById( "actual" );
  dg.iconic               = document.getElementById( "iconic" );
  dg.custom               = document.getElementById( "custom" );
  dg.constrainCheckbox    = document.getElementById( "constrainCheckbox" );
  dg.marginInput          = {left:   document.getElementById("marginLeftInput"),
                                    right: document.getElementById("marginRightInput"),
                                    top:   document.getElementById("marginTopInput"),
                                    bottom:document.getElementById("marginBottomInput")};
  dg.borderInput          = {left:   document.getElementById("borderLeftInput"),
                                    right: document.getElementById("borderRightInput"),
                                    top:   document.getElementById("borderTopInput"),
                                    bottom:document.getElementById("borderBottomInput")};
  dg.paddingInput         = {left:   document.getElementById("paddingLeftInput"),
                                    right: document.getElementById("paddingRightInput"),
                                    top:   document.getElementById("paddingTopInput"),
                                    bottom:document.getElementById("paddingBottomInput")};
  dg.cropInput            = {left:   document.getElementById("cropLeftInput"),
                                    right: document.getElementById("cropRightInput"),
                                    top:   document.getElementById("cropTopInput"),
                                    bottom:document.getElementById("cropBottomInput")};
  dg.colorWell            = document.getElementById("colorWell");
  dg.placementRadioGroup  = document.getElementById("placementRadioGroup");
  dg.placeForceHereCheck  = document.getElementById("placeForceHereCheck");
  dg.placeHereCheck       = document.getElementById("placeHereCheck");
  dg.placeFloatsCheck     = document.getElementById("placeFloatsCheck");
  dg.placeTopCheck        = document.getElementById("placeTopCheck");
  dg.placeBottomCheck     = document.getElementById("placeBottomCheck");
  dg.herePlacementRadioGroup  = document.getElementById("herePlacementRadioGroup");
  dg.OkButton             = document.documentElement.getButton("accept");
  var fieldList = [];
  var attrs = ["margin","border","padding","crop"];
  for (var side in sides)
  {
    for (var attr in attrs)
    {
      fieldList.push(dg[attrs[attr]+"Input"][sides[side].toLowerCase()]);
    }
  }
  fieldList.push(dg.heightInput);
  fieldList.push(dg.widthInput);
  frameUnitHandler.setEditFieldList(fieldList);
  var unit;
  initUnitList(dg.unitList);
// The defaults for the document are set by the XUL document, modified by persist attributes. If there is
// no pre-existing frame object, the dg is set to go.
  if (!newElement)
  {   // we need to initialize the dg from the frame element
    unit =  element.getAttribute("units");
    var i;
    var values = [0,0,0,0];
    for (i = 0; i < dg.unitList.itemCount; i++)
      if (dg.unitList.getItemAtIndex(i).value == unit)
      {
        dg.unitList.selectedIndex = i;
        break;
      }
    var width = 0;
    if (element.hasAttribute("width")) width = element.getAttribute("width");
    if (Number(width) > 0)
    {
      dg.widthInput.value = width;
      dg.autoWidthCheck.checked = false;
    }
    else dg.autoWidthCheck.checked = true;
    var height = 0;
    if (element.hasAttribute("height")) height = element.getAttribute("height");
    if (height > 0)
    {
      dg.heightInput.value = height;
      dg.autoHeightCheck.checked = false;
    }
    else dg.autoHeightCheck.checked = true;

    if (element.hasAttribute("margin"))
      { values = parseLengths(element.getAttribute("margin"));}
    for (i = 0; i<4; i++)
      { dg.marginInput[sides[i].toLowerCase()].value = values[i];}
    values = [0,0,0,0];
    if (element.hasAttribute("border"))
      { values = parseLengths(element.getAttribute("border"));}
    for (i = 0; i<4; i++)
      { dg.borderInput[sides[i].toLowerCase()].value = values[i];}
    values = [0,0,0,0];
    if (element.hasAttribute("padding"))
      { values = parseLengths(element.getAttribute("padding"));}
    for (i = 0; i<4; i++)
      { dg.paddingInput[sides[i].toLowerCase()].value = values[i];}
    if (element.hasAttribute("crop"))
      { values = parseLengths(element.getAttribute("crop"));}
    for (i = 0; i<4; i++)
      { dg.cropInput[sides[i].toLowerCase()].value = values[i];}
    var placeLocation = element.getAttribute("placeLocation");
    dg.placeForceHereCheck.checked = /H/.test(placeLocation);
    dg.placeHereCheck.checked = /h/.test(placeLocation);
    dg.placeFloatsCheck.checked = /p/.test(placeLocation);
    dg.placeTopCheck.checked = /t/.test(placeLocation);
    dg.placeBottomCheck.checked = /b/.test(placeLocation);

    dg.herePlacementRadioGroup.value = element.getAttribute("placement");
    var pos = element.getAttribute("pos");
    dg.placementRadioGroup.selectedIndex = (pos == "inline")?0:(pos == "display")?1:(pos == "float")?2:-1;

    // TODO: color
  }
// Now initialize the UI, including the diagram
  unit = 'px'; //dg.unitList.selectedItem.value;
  frameUnitHandler.initCurrentUnit(unit);
  var placement = 0;
  if (/left|inside/.test(placeLocation)) placement=1;
  else if (/right|outside/.test(placeLocation)) placement = 2;
  gFrameTab = dg;
  setAlignment(placement);
  enableHere(dg.herePlacementRadioGroup);
  enableFloating();
  updateDiagram("margin");
  updateDiagram("border");
  updateDiagram("padding");
  return dg;
  
}

function setNewUnit(element)
{
  frameUnitHandler.setCurrentUnit(element.value);
}

function initFrameSizePanel()
{
  document.getElementById("hasNaturalSize").removeAttribute("hidden");
}


function initUnitList(unitPopUp)
{
  var elements = unitPopUp.getElementsByTagName("menuitem");
  var i, len;
  for (i=0, len = elements.length; i<len; i++)
  {
    elements[i].label = frameUnitHandler.getDisplayString(elements[i].value);
    // dump("element with value "+elements[i].value+" has label "+elements[i].label+"\n");
  }
}
  
function parseLengths( str ) // correctly parse something like margin= 10px or margin = 10px 5px 3px 7px
// the inverse operation is going on in updateDiagram
{
  var values = str.split(" ");
  var length = values.length;
  if (length < 1) return [];
  // check to see if there is any empty string at the end.
  if (/^\s*$/.test(values[length-1]))
  {
    length--;
    values.pop();
  }
  switch (length)  // notice there are no breaks. Fall-through is intentional
  {
    case 1: values.push(values[0]);
    case 2: values.push(values[0]); 
    case 3: values.push(values[1]);
    default:
  }
  return values;
}


function update( anId )
{
  //parse the id
  if (anId.length == 0) return;
  var regexp=/(.*)(Left|Right|Top|Bottom)(.*)/;
  var result = regexp.exec(anId);
//  switch (result[2]) { // notice that there are no breaks in this switch statement -- intentional
//    case "Left":  document.getElementById(result[1]+"Left"+result[3]).value = document.getElementById(anId).value;
//    case "Right": document.getElementById(result[1]+"Right"+result[3]).value = document.getElementById(anId).value;
//    case "Top":   document.getElementById(result[1]+"Top"+result[3]).value = document.getElementById(anId).value;
//    case "Bottom": document.getElementById(result[1]+"Bottom"+result[3]).value = document.getElementById(anId).value;
//  }
  updateDiagram( result[1] );
}

function extendInput( anId )
{
  var input = "Input";
  var val = document.getElementById(anId+"Left"+input).value;
  document.getElementById(anId+"Right"+input).value = Math.max(0,val);
  document.getElementById(anId+"Top"+input).value = Math.max(0,val);
  document.getElementById(anId+"Bottom"+input).value = Math.max(0,val);
  updateDiagram( anId );
}

function toPixels( x )
{
  return Math.round(.4999 + frameUnitHandler.getValueAs(x,"px")*scale);
}

var color;
function getColorAndUpdate()
{
  var colorWell = document.getElementById("colorWell");
  if (!colorWell) return;

  // Don't allow a blank color, i.e., using the "default"
  var colorObj = { NoDefault:true, Type:"Rule", TextColor:color, PageColor:0, Cancel:false };

  window.openDialog("chrome://editor/content/EdColorPicker.xul", "colorpicker", "chrome,close,titlebar,modal", "", colorObj);

  // User canceled the gFrameTab
  if (colorObj.Cancel)
    return;

  color = colorObj.TextColor;
  setColorWell("colorWell", color); 
  setStyleAttributeByID("frame","border-color",color);
}

// come up with a four part attribute giving the four parts of the margin or padding or border, etc. using the same rules as CSS
function getCompositeMeasurement(attribute, unit, showUnit)
{
  var i;
  var values = [];
  dump("attribute = "+attribute+", unit = "+unit+"\n");
  for (i = 0; i<4; i++)
    { dump(i + "\n");
      values.push( Math.max(0,frameUnitHandler.getValueAs(Number(document.getElementById(attribute + sides[i] + "Input").value ),unit)));
    }
  if (values[1] == values[3])
  {
    values.splice(3,1);
    if (values[0] == values[2]) 
    {
      values.splice(2,1);
      if (values[0] == values[1]) {values.splice(1,1);}
    }
  }
//  if (unit == "px")
//  {
//    for (i=0; i<values.length; i++)
//    {
//      dump(i + "\n");
//      values[i] = Math.round(values[i]);
//    }
//  }
  var val;
  if (showUnit) {val = values.join(unit+" ")+unit;}
  else { val = values.join(" ");}
  dump("getCompositeMeasurement returning "+attribute+" = "+val+"\n");
  return val;
}

function updateDiagram( attribute ) //attribute = margin, border, padding; 
{
  var i;
  var values = [];
  for (i = 0; i<4; i++)
    { values.push( Math.max(0,toPixels(document.getElementById(attribute + sides[i] + "Input").value )));}
  if (values[1] == values[3])
  {
    values.splice(3,1);
    if (values[0] == values[2]) 
    {
      values.splice(2,1);
      if (values[0] == values[1]) values.splice(1,1);
    }
  }
  var val = values.join("px ")+"px";
  if (attribute=="border")  // add border color and border width
  {
    var bgcolor = gFrameTab.colorWell.getAttribute("style");
    var arr = bgcolor.match(/background-color\s*:([a-zA-Z\ \,0-9\(\)]+)\s*;\s*/,"");
    removeStyleAttributeFamilyOnNode(document.getElementById("frame"), "border");
    var style = document.getElementById("frame").getAttribute("style");
    style += " border-width: "+val+"; border-color: " + arr[1]+"; border-style: solid;";
    document.getElementById("frame").setAttribute("style", style);
  }
  else
    { setStyleAttributeByID("frame", attribute, val );}
  dump(document.getElementById("frame").getAttribute("style")+"\n");
  // dump(document.getElementById("frame").getAttribute("style"));
  // space flowing around the diagram is 150 - (lmargin + rmargin + lborder + lpadding + rborder + rpadding + imageWidth)*scale
  var hmargin = toPixels(Number(gFrameTab.marginInput.left.value)) + toPixels(Number(gFrameTab.marginInput.right.value));
  var hborder = toPixels(Number(gFrameTab.borderInput.left.value)) + toPixels(Number(gFrameTab.borderInput.right.value));
  var hpadding = toPixels(Number(gFrameTab.paddingInput.left.value)) + toPixels(Number(gFrameTab.paddingInput.right.value));
  switch(position)
  {
    case 1: document.getElementById("leftspacer").setAttribute("width", Number(60+ Math.min(toPixels(gFrameTab.marginInput.left.value),0)) + "px"); 
            document.getElementById("leftpage").setAttribute("width", "0px");    
            document.getElementById("frame").setAttribute("width", scaledWidth+hborder+"px");
            document.getElementById("frame").setAttribute("height", scaledHeight+hborder+"px");
            document.getElementById("rightpage").setAttribute("width", Math.min(150,150 - scaledWidth - (hmargin+hborder)) + "px");  
            document.getElementById("rightspace").setAttribute("width", Math.max(0, - scaledWidth - (hmargin+hborder)) + "px");  
            document.getElementById("leftspace").setAttribute("width", "0px");
            break;
    case 2: document.getElementById("leftspacer").setAttribute("width", Number(60 + Math.min(0,150 - scaledWidth - (hmargin + hborder))) + "px"); 
            document.getElementById("leftpage").setAttribute("width", Math.min(150,150 - scaledWidth - (hmargin + hborder)) + "px");    
            document.getElementById("frame").setAttribute("width", scaledHeight+hborder +"px");
            document.getElementById("frame").setAttribute("height", scaledWidth+hborder +"px");
            document.getElementById("rightpage").setAttribute("width", "0px");  
            document.getElementById("rightspace").setAttribute("width", "0px");
            document.getElementById("leftspace").setAttribute("width", Math.max(0, - scaledWidth - (hmargin + hborder)) + "px");  
            break;
    default: document.getElementById("leftspacer").setAttribute("width", Number(135 - (scaledWidth + hborder)/2) + "px");
            document.getElementById("leftpage").setAttribute("width", "0px");    
            document.getElementById("frame").setAttribute("width", scaledWidth+"px");
            document.getElementById("frame").setAttribute("height", scaledHeight+"px");
            document.getElementById("rightpage").setAttribute("width", "0px");  
            document.getElementById("rightspace").setAttribute("width", "0px");
            document.getElementById("leftspace").setAttribute("width", "0px");
            break;
  }  
  // dump(document.getElementById("frame").getAttribute("width"));
  // update currentElement also

}

function setStyleAttributeOnNode( node, att, value)
{
  var style="";
  removeStyleAttributeFamilyOnNode( node, att);
  if (node.hasAttribute("style")) style = node.getAttribute("style");
  dump("original style is '"+style+"'\n");
  style.replace("null","");
  style = style + " " + att +": " + value + "; ";
  dump("Setting style of node to "+style+"\n");
  node.setAttribute("style",style);
}

function removeStyleAttributeFamilyOnNode( node, att)
{
  dump("removeStyleAttributeFamilyOnNode( "+node.id+", "+att+");\n");
  var style="";
  if (node.hasAttribute("style")) style = node.getAttribute("style");
  dump("original style is '"+style+"'\n");
  style.replace("null","");
  var re = new RegExp("^|[^-]"+att + "[-a-zA-Z]*:[^;]*;","g");
  if (re.test(style))
  {
    style = style.replace(re, "");
    node.setAttribute("style",style);
    dump("Setting style of node to "+style+"\n");
  }
}


function setStyleAttributeByID( id, att, value)
{
  setStyleAttributeOnNode(document.getElementById(id), att, value);
}

function enableHere(radiogroup )
{
  var broadcaster = document.getElementById("herePlacement");
  var theValue = "true";
  var position;
  if (!radiogroup) radiogroup = document.getElementById("herePlacementRadioGroup");
  if (document.getElementById('placeHereCheck').checked)
  {
    theValue = "false";
    position = radiogroup.selectedItem.value;
    setAlignment((position==="left" || position==="inside")?1:((position==="right"||position=="outside")?2:0));
  }
  else
  {
    setAlignment(0);
  }
  broadcaster.setAttribute("disabled",theValue);
//  document.getElementById('herePlacementRadioGroup').value;
  updateDiagram("margin");
}


function enableFloating( )
{
  var broadcaster = document.getElementById("floatingPlacement");
  var theValue = "true";
  if (document.getElementById('float').selected) theValue = "false";
  broadcaster.setAttribute("disabled",theValue);
  if (theValue=="true") document.getElementById("herePlacement").setAttribute("disabled","true");
  else enableHere();
}

/************************************/
function handleChar(event, id)
{
  var element = event.originalTarget;
  if ((event.keyCode != event.DOM_VK_UP) && (event.keyCode != event.DOM_VK_DOWN))
    updateTextNumber(element, id, event);
  update(id);
  event.preventDefault();
}

function geomHandleChar(event, id, tbid)
{
  handleChar(event, id, tbid);
//  geomInputChar(event, id, tbid);
}


//function geomInputChar(event, id, tbid)
//{
//  update(id);  
//}


function unitRound( size, unit )
{
  var places;
  if (!unit) unit = frameUnitHandler.getCurrentUnit();
  // round off to a number of decimal places appropriate for the units
  switch (unit) {
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

function setContentSize(width, height)  // width and height are the size of the image in pixels
{
  scaledWidth = Math.round(scale*width);
  if (scaledWidth == 0) scaledWidth = 40;
  scaledHeight = Math.round(scale*height);
  if (scaledWidth == 0) scaledHeight = 60;
  setStyleAttributeByID("content", "width", scaledWidth + "px");
  setStyleAttributeByID("content", "height", scaledHeight + "px");
  updateDiagram("margin");
}

  
function setAlignment(alignment ) // alignment = 1 for left, 2 for right, 0 for neither
{
  position = alignment;
  if (position ==1|| position ==2)
  {
    gFrameTab.marginInput.left.removeAttribute("disabled");
    gFrameTab.marginInput.right.removeAttribute("disabled");
  }
  else
  {
    gFrameTab.marginInput.left.setAttribute("disabled", "true");
    gFrameTab.marginInput.left.setAttribute("value", "0.00");
    gFrameTab.marginInput.right.setAttribute("disabled", "true");
    gFrameTab.marginInput.right.setAttribute("value", "0.00");
  }
}

function setFrameAttributes(frameNode)
{
  var unit = frameUnitHandler.currentUnit;
  frameNode.setAttribute("units",unit);
  frameNode.setAttribute("msi_resize","true");
  frameNode.setAttribute("margin", getCompositeMeasurement("margin", unit, false));  
  frameNode.setAttribute("border", getCompositeMeasurement("border",unit, false));  
  frameNode.setAttribute("padding", getCompositeMeasurement("padding",unit, false));  
  frameNode.setAttribute("crop", getCompositeMeasurement("crop",unit, false));  
  if (gFrameTab.autoHeightCheck.checked)
  {
    if (frameNode.hasAttribute("height")) frameNode.removeAttribute("height");
  }
  else frameNode.setAttribute("height", gFrameTab.heightInput.value);
  if (gFrameTab.autoWidthCheck.checked)
  {
    if (frameNode.hasAttribute("width")) frameNode.removeAttribute("width");
  }
  else frameNode.setAttribute("width", gFrameTab.widthInput.value);
  frameNode.setAttribute("crop", getCompositeMeasurement("crop",unit, false));  
  var pos = document.getElementById("placementRadioGroup").selectedItem;
  frameNode.setAttribute("pos", pos?pos.getAttribute("id"):"");
  var bgcolor = gFrameTab.colorWell.getAttribute("style");
  var arr = bgcolor.match(/background-color\s*:([a-zA-Z\ \,0-9\(\)]+)\s*;\s*/,"");
  setStyleAttributeOnNode(frameNode, "border-color", arr[1]);
  frameNode.setAttribute("border-color", arr[1]);  
  if (document.getElementById("inline").selected)
    setStyleAttributeOnNode(frameNode, "display", "inline");
  else setStyleAttributeOnNode(frameNode, "display", "blcck");
  var placeLocation="";
  var isHere = false;
  if (gFrameTab.placeForceHereCheck.checked)
  {
    placeLocation += "H";
    isHere = true;
  }
  else
  {
    if (gFrameTab.placeHereCheck.checked)
    {
      placeLocation += "h";
      isHere = true;
    }
    if (gFrameTab.placeFloatsCheck.checked)
      { placeLocation += "p"}
    if (gFrameTab.placeTopCheck.checked)
      { placeLocation += "t"}
    if (gFrameTab.placeBottomCheck.checked)
      { placeLocation += "b" }
  }
  frameNode.setAttribute("placeLocation", placeLocation);
  if (isHere)
  {
    var floatparam = document.getElementById("herePlacementRadioGroup").value;
    frameNode.setAttribute("placement",floatparam); 
    if (floatparam == "inside") floatparam = "left";
    else if (floatparam == "outside") floatparam = "right";
    setStyleAttributeOnNode(frameNode, "float", floatparam);

    dump("Find parameters for here placement");
  }
  else {removeStyleAttributeFamilyOnNode(frameNode, "float");}
  // now set measurements in the style for frameNode
  var style = getCompositeMeasurement("margin","px", true);
  setStyleAttributeOnNode(frameNode, "margin", style);
  dump("Setting margin style to "+style+"\n");
  style = getCompositeMeasurement("padding","px", true);
  setStyleAttributeOnNode(frameNode, "padding", style);
  style = getCompositeMeasurement("border","px", true);
  setStyleAttributeOnNode(frameNode, "border-width", style);
  if (frameNode.hasAttribute("height") && Number(frameNode.getAttribute("height"))!= 0 )
    setStyleAttributeOnNode(frameNode, "height", frameUnitHandler.getValueAs(frameNode.getAttribute("height"),"px") + "px");
  else removeStyleAttributeFamilyOnNode(frameNode, "height");
  if (frameNode.hasAttribute("width") && Number(frameNode.getAttribute("width"))!= 0)
    setStyleAttributeOnNode(frameNode, "width", frameUnitHandler.getValueAs(frameNode.getAttribute("width"),"px") + "px");
  else removeStyleAttributeFamilyOnNode(frameNode, "width");
  if (style != "0px") { setStyleAttributeOnNode( frameNode, "border-style", "solid");}
}



function frameHeightChanged(input)
{
  if (input.value > 0)
  {
     scaledHeight = toPixels(input.value);
  }
  else scaledHeight = scaledHeightDefault;
  setStyleAttributeByID("content", "height", scaledHeight + "px");
}

function frameWidthChanged(input)
{
  if (input.value > 0) scaledWidth = toPixels(input.value);
    else scaledWidth = scaledWidthDefault;
  setStyleAttributeByID("content", "width", scaledWidth + "px");
}
