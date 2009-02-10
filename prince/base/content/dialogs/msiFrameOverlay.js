Components.utils.import("resource://app/modules/unitHandler.jsm");
var frameUnitHandler = new UnitHandler();
var sides = ["Top", "Right", "Bottom", "Left"];
var currentFrame;
var scale = 0.25;
var scaledWidth = 50; 
var scaledHeight = 40;
var position = 0;  // left = 1, right = 2, neither = 0




function initFrameTab(gDialog, element)
{
  var editorElement = msiGetParentEditorElementForDialog(window);
  var editor = msiGetEditor(editorElement);
  if (!editor) {
    window.close();
    return;
  }
  gDialog.frameUnitMenulist      = document.getElementById( "frameUnitMenulist");
  gDialog.marginInput = {left:   document.getElementById( "marginLeftInput"),
                         right:  document.getElementById( "marginRightInput"),
                         top:    document.getElementById( "marginTopInput"),
                         bottom: document.getElementById( "marginBottomInput")};
  gDialog.borderInput = {left:   document.getElementById( "borderLeftInput"),
                         right:  document.getElementById( "borderRightInput"),
                         top:    document.getElementById( "borderTopInput"),
                         bottom: document.getElementById( "borderBottomInput")};
  gDialog.paddingInput ={left:   document.getElementById( "paddingLeftInput"),
                         right:  document.getElementById( "paddingRightInput"),
                         top:    document.getElementById( "paddingTopInput"),
                         bottom: document.getElementById( "paddingBottomInput")};
  gDialog.cropInput   = {left:   document.getElementById( "cropLeftInput"),
                         right:  document.getElementById( "cropRightInput"),
                         top:    document.getElementById( "cropTopInput"),
                         bottom: document.getElementById( "cropBottomInput")};
  gDialog.colorWell   = document.getElementById("colorWell");
  gDialog.placementRadioGroup   = document.getElementById("placementRadioGroup");
  gDialog.placeForceHereCheck        = document.getElementById("placeForceHereCheck");
  gDialog.placeHereCheck        = document.getElementById("placeHereCheck");
  gDialog.placeFloatsCheck      = document.getElementById("placeFloatsCheck");
  gDialog.placeTopCheck         = document.getElementById("placeTopCheck");
  gDialog.placeBottomCheck      = document.getElementById("placeBottomCheck");
  gDialog.frameUnitMenuList     = document.getElementById("frameUnitMenulist");
  gDialog.OkButton          = document.documentElement.getButton("accept");
  var fieldList = [];
  var attrs = ["margin","border","padding","crop"];
  for (var side in sides)
  {
    for (var attr in attrs)
    {
      fieldList.push(gDialog[attrs[attr]+"Input"][sides[side].toLowerCase()]);
    }
  }
  frameUnitHandler.setEditFieldList(fieldList);
  frameUnitHandler.initCurrentUnit("in");
  initUnitList(document.getElementById("unitList"));
  frameUnitHandler.setCurrentUnit(gDialog.frameUnitMenuList.value);
  var color = getColorWell("colorWell");
  if (color.length > 0)
  { 
    setStyleAttribute("frame","border-color",color);
  }

  if (element && element.localName == "msiframe")
  {
    currentFrame = element;
    //read current units
    var values = [0,0,0,0];
    var i;
    if (element.hasAttribute("margin"))
      { values = parseLengths(element.getAttribute("margin"));}
    for (i = 0; i<4; i++)
      { gDialog.marginInput[toLowerCase(sides[i])].value = values[i];}
    values = [0,0,0,0];
    if (element.hasAttribute("border-width"))
      { values = parseLengths(element.getAttribute("border-width"));}
    for (i = 0; i<4; i++)
      { gDialog.borderInput[toLowerCase(sides[i])].value = values[i];}
    values = [0,0,0,0];
    if (element.hasAttribute("padding"))
      { values = parseLengths(element.getAttribute("padding"));}
    for (i = 0; i<4; i++)
      { gDialog.paddingInput[toLowerCase(sides[i])].value = values[i];}
  }
//  else
//    currentFrame = editor.createElementWithDefaults("msiframe"); 
}

function setNewUnit(element)
{
  frameUnitHandler.setCurrentUnit(element.value);
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
  return Math.round(frameUnitHandler.getValueAs(x,"px")*scale);
}

var color;
function getColorAndUpdate()
{
  var colorWell = document.getElementById("colorWell");
  if (!colorWell) return;

  // Don't allow a blank color, i.e., using the "default"
  var colorObj = { NoDefault:true, Type:"Rule", TextColor:color, PageColor:0, Cancel:false };

  window.openDialog("chrome://editor/content/EdColorPicker.xul", "_blank", "chrome,close,titlebar,modal", "", colorObj);

  // User canceled the dialog
  if (colorObj.Cancel)
    return;

  color = colorObj.TextColor;
  setColorWell("colorWell", color); 
  setStyleAttribute("frame","border-color",color);
}

// come up with a four part attribute giving the four parts of the margin or padding or border, etc. using the same rules as CSS
function getCompositeMeasurement(attribute, unit)
{
  var i;
  var values = [];
  dump("attribute = "+attribute+", unit = "+unit+"\n");
  for (i = 0; i<4; i++)
    { values.push( Math.max(0,frameUnitHandler.getValueAs(Number(document.getElementById(attribute + sides[i] + "Input").value ),unit)));}
  if (values[1] == values[3])
  {
    values.splice(3,1);
    if (values[0] == values[2]) 
    {
      values.splice(2,1);
      if (values[0] == values[1]) values.splice(1,1);
    }
  }
  if (unit == "px")
  {
    for (i=0; i<values.length; i++)
      values[i] = Math.round(values[i]);
  }
  var val = values.join(" ");
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
  var att = attribute + ((attribute === "border")?"-width":"");
  setStyleAttribute("frame", att, val );
  // dump(document.getElementById("frame").getAttribute("style"));
  // space flowing around the diagram is 150 - (lmargin + rmargin + lborder + lpadding + rborder + rpadding + imageWidth)*scale
  var hmargin = toPixels(Number(gDialog.marginInput.left.value)) + toPixels(Number(gDialog.marginInput.right.value));
  var hborder = toPixels(Number(gDialog.borderInput.left.value)) + toPixels(Number(gDialog.borderInput.right.value));
  var hpadding = toPixels(Number(gDialog.paddingInput.left.value)) + toPixels(Number(gDialog.paddingInput.right.value));
  switch(position)
  {
    case 1: document.getElementById("leftspacer").setAttribute("width", Number(60+ Math.min(toPixels(gDialog.marginInput.left.value),0)) + "px"); 
            document.getElementById("leftpage").setAttribute("width", "0px");    
            document.getElementById("rightpage").setAttribute("width", Math.min(150,150 - scaledWidth - (hmargin + hborder + hpadding)) + "px");  
            document.getElementById("rightspace").setAttribute("width", Math.max(0, - scaledWidth - (hmargin + hborder + hpadding)) + "px");  
            document.getElementById("leftspace").setAttribute("width", "0px");
            break;
    case 2: document.getElementById("leftspacer").setAttribute("width", Number(60 + Math.min(0,150 - scaledWidth - (hmargin + hborder + hpadding))) + "px"); 
            document.getElementById("leftpage").setAttribute("width", Math.min(150,150 - scaledWidth - (hmargin + hborder + hpadding)) + "px");    
            document.getElementById("rightpage").setAttribute("width", "0px");  
            document.getElementById("rightspace").setAttribute("width", "0px");
            document.getElementById("leftspace").setAttribute("width", Math.max(0, - scaledWidth - (hmargin + hborder + hpadding)) + "px");  
            break;
    default: document.getElementById("leftspacer").setAttribute("width", Number(135 - (scaledWidth + hborder + hpadding)/2) + "px");
            document.getElementById("leftpage").setAttribute("width", "0px");    
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
  var style = node.getAttribute("style");
  var re = new RegExp(att + ":[^;]*;","");
  if (re.test(style))
    style = style.replace(re, att + ": " +value+"; ");
  else
    style = style + " " + att +": " + value + "; ";
  node.setAttribute("style",style);
}

function setStyleAttribute( id, att, value)
{
  setStyleAttributeOnNode(document.getElementById(id));
}

function enableHere( )
{
  var broadcaster = document.getElementById("herePlacement");
  var theValue = "true";
  var position;
  if (document.getElementById('placeHereCheck').checked)
  {
    theValue = "false";
    position = document.getElementById('herePlacementRadioGroup').value;
    setAlignment( (position==="left" || position==="inside")?1:((position==="right"||position=="outside")?2:0));
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
  if (document.getElementById('floatingRadio').selected) theValue = "false";
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
  scaledHeight = Math.round(scale*height);
  setStyleAttribute("content", "width", scaledWidth + "px");
  setStyleAttribute("content", "height", scaledHeight + "px");
  updateDiagram("margin");
}

  
function setAlignment( alignment ) // alignment = 1 for left, 2 for right, 0 for neither
{
  position = alignment;
  if (position ==1|| position ==2)
  {
    gDialog.marginInput.left.removeAttribute("disabled");
    gDialog.marginInput.right.removeAttribute("disabled");
  }
  else
  {
    gDialog.marginInput.left.setAttribute("disabled", "true");
    gDialog.marginInput.left.setAttribute("value", "0.00");
    gDialog.marginInput.right.setAttribute("disabled", "true");
    gDialog.marginInput.right.setAttribute("value", "0.00");
  }
}

function setFrameAttributes(frameNode)
{
  var unit = frameUnitHandler.currentUnit;
//  frameNode.units = unit;
  dump(getCompositeMeasurement("margin",unit)+"\n");
  dump(getCompositeMeasurement("border",unit)+"\n");
  dump(getCompositeMeasurement("padding",unit)+"\n");
  dump(getCompositeMeasurement("crop",unit)+"\n");

  dump(getCompositeMeasurement("margin","px")+"\n");
  dump(getCompositeMeasurement("border","px")+"\n");
  dump(getCompositeMeasurement("padding","px")+"\n");
  dump(getCompositeMeasurement("crop","px")+"\n");

  var unit = frameNode.units;
  frameNode.setAttribute("margin", getCompositeMeasurement("margin",unit));  
  frameNode.setAttribute("border", getCompositeMeasurement("border",unit));  
  frameNode.setAttribute("padding", getCompositeMeasurement("padding",unit));  
  frameNode.setAttribute("crop", getCompositeMeasurement("crop",unit));  
  var bgcolor = document.getElementById("colorWell").getAttribute("style");
  bgcolor = bgcolor.replace(/background-color\s*:\s*/,"");
  setStyleAttributeOnNode(frameNode, "border-color", bgcolor);
  frameNode.setAttribute("border-color", bgcolor);  
  frameNode.setAttribute("placement", gDialog.placementRadioGroup.value);
  var placeLocation="";
  var isHere = false;
  if (gDialog.placeForceHereCheck.checked)
  {
    placeLocation += "H";
    isHere = true;
  }
  else
  {
    if (gDialog.placeHereCheck.checked)
    {
      placeLocation += "h";
      isHere = true;
    }
    if (gDialog.placeFloatsCheck.checked)
      { placeLocation += "p"}
    if (gDialog.placeTopCheck.checked)
      { placeLocation += "t"}
    if (gDialog.placeBottomCheck.checked)
      { placeLocation += "b" }
  }
  frameNode.setAttribute("placeLocation", placeLocation);
  if (isHere)
  {
    dump("Find parameters for here placement");
  }
  // now set measurements in the style for frameNode
  var style = getCompositeMeasurement("margin","px");
  setStyleAttributeOnNode(frameNode, "margin", style);
  style = getCompositeMeasurement("padding","px");
  setStyleAttributeOnNode(frameNode, "padding", style);
  style = getCompositeMeasurement("border","px");
  setStyleAttributeOnNode(frameNode, "border", style);
}
