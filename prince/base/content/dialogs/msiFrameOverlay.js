Components.utils.import("resource://app/modules/unitHandler.jsm");
var frameUnitHandler = new UnitHandler();
var sides = ["Top", "Right", "Bottom", "Left"];
var currentFrame;
var scale = 0.25;

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
  gDialog.placeHereCheck        = document.getElementById("placeHereCheck");
  gDialog.placeFloatsCheck      = document.getElementById("placeFloatsCheck");
  gDialog.placeTopCheck         = document.getElementById("placeTopCheck");
  gDialog.placeBottomCheck      = document.getElementById("placeBottomCheck");
  gDialog.herePlacementRadioGroup   = document.getElementById("herePlacementRadioGroup");
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

  if (element && element.localName == "msiframe")
  {
    currentFrame = element;
    //read current units
    var values = [0,0,0,0];
    var i;
    if (element.hasAttribute("margin"))
      values = parseLengths(element.getAttribute("margin"));
    for (i = 0; i<4; i++)
      gDialog.marginInput[toLowerCase(sides[i])].value = values[i];
    values = [0,0,0,0];
    if (element.hasAttribute("border-width"))
      values = parseLengths(element.getAttribute("border-width"));
    for (i = 0; i<4; i++)
      gDialog.borderInput[toLowerCase(sides[i])].value = values[i];
    values = [0,0,0,
    0];
    if (element.hasAttribute("padding"))
      values = parseLengths(element.getAttribute("padding"));
    for (i = 0; i<4; i++)
      gDialog.paddingInput[toLowerCase(sides[i])].value = values[i];
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
    dump("element with value "+elements[i].value+" has label "+elements[i].label+"\n");
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


function extendInput( anId )
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
  var eventsource = document.getElementById(anId);
  if (eventsource) eventsource._validateValue(eventsource.inputField.value,false,true);
  updateDiagram( result[1] );
}

function toPixels( x )
{
  return frameUnitHandler.getValueAs(x,"px")*scale;
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
  currentFrame.setAttribute("border-color",color);
}



function updateDiagram( attribute ) //attribute = margin, border, padding; 
{
  var i;
  var values = [];
  for (i = 0; i<4; i++)
    values.push( toPixels(document.getElementById(attribute + sides[i] + "Input").value ));
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
  dump(document.getElementById("frame").getAttribute("style"));
// assume for now that the picture width (scaled) is 40px high and 50px wide
  var imageWidth = 200*scale;
  var imageHeight = 160*scale;
  // space flowing around the diagram is 150 - (lmargin + rmargin + lborder + lpadding + rborder + rpadding + imageWidth)*scale
  var hmargin = toPixels(Number(gDialog.marginInput.left.value) + Number(gDialog.marginInput.right.value));
  var hborder = toPixels(Number(gDialog.borderInput.left.value) + Number(gDialog.borderInput.right.value));
  var hpadding = toPixels(Number(gDialog.paddingInput.left.value) + Number(gDialog.paddingInput.right.value));
  setStyleAttribute("leftpage", "width", 150 - imageWidth - (hmargin + hborder + hpadding)*scale + "px");  

  // update currentElement also

}

function setStyleAttribute( id, att, value)
{
  var style = document.getElementById(id).getAttribute("style");
  var re = new RegExp(att + ":[^;]*;","");
  if (re.test(style))
    style = style.replace(re, att + ": " +value+"; ");
  else
    style = style + " " + att +": " + value + "; ";
  document.getElementById(id).setAttribute("style",style);
}

function enableHere( )
{
  var broadcaster = document.getElementById("herePlacement");
  var theValue = "true";
  if (document.getElementById('placeHereCheck').checked) theValue = "false";
  broadcaster.setAttribute("disabled",theValue);
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

function handleChar(event, id)
{
  alert("event = , id = "+id);
}

/************************************/
function handleChar(event, id)
{
  var element = event.originalTarget;
  if ((event.keyCode != event.DOM_VK_UP) && (event.keyCode != event.DOM_VK_DOWN))
    updateTextNumber(element, id, event);
  extendInput(id);
  event.preventDefault();
}

function geomHandleChar(event, id, tbid)
{
  handleChar(event, id, tbid);
  geomInputChar(event, id, tbid);
}


// since the onkeypress event gets called *before* the value of a text box is updated,
// we handle the updating here. This function takes a textbox element and an event and returns sets
// the value of the text box

function updateTextNumber(textelement, id, event)
{
  var val = textelement.value;
  var textbox = document.getElementById(id);
  //textbox is the text box; textelement in the underlying html:input
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


function geomInputChar(event, id, tbid)
{
  extendInput(id);  
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

