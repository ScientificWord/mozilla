
var data;
var widthElements;
var heightElements;
var widthTextElements;
var heightTextElements;
var color;

var unitConversions=           
{
  mm: 1, // mm per mm
  pt: .3514598,  //mm per pt
  in: 25.4,  //mm per in
  cm: 10 // mm per cm
};
var scale= .75; /*pixels per mm in reduced view*/

var unitsList;
var currentUnit;

function startUp()
{
  var element;
  var i;
  data = window.arguments[0];
  if (data.units) // we assume if units is set, all the data is set
  {
    document.getElementById('units').value = data.units;
    document.getElementById('frame.position').value = data.position;
    document.getElementById('width').value = data.width;
    document.getElementById('height').value = data.height;
    document.getElementById('overhang').value = data.overhang;
    document.getElementById('marginTop').value = data.topmargin;
    document.getElementById('marginSide').value = data.sidemargin;
    document.getElementById('border').value = data.border;
    document.getElementById('padding').value = data.padding;
    color = data.color;
    document.getElementById('textalignment').value = data.textalignment;
//    document.getElementById('borderstyle').value = data.borderstyle;
    document.getElementById('rotation').value = data.rotation;
    setPosition(data.position);
  }
  else
  {
    color="#000000";
  }
  widthTextElements = new Array (
    "width",
    "overhang",
    "marginSide",
    "border",
    "padding"
  );
  widthElements = new Array (
    "notedata",
     null,
    "sidemargin",
    "border",
    "padding"
  );
  heightTextElements = new Array (
    "height",
    "marginTop"
  );
  heightElements = new Array (
    "noteheight",
    "topmargin"
  );
  unitsList=new msiUnitsList(unitConversions);
  currentUnit = document.getElementById('units').value;
  setDecimalPlaces();
  for (i=0; i<widthElements.length; i++)
  {
    element = document.getElementById(widthTextElements[i]);
    if (element.value != "auto")
      updateParameter(widthElements[i],widthTextElements[i]);
  }

  for (i=0; i<heightElements.length; i++)
  {
    element = document.getElementById(heightTextElements[i]);
    if (element.value != "auto")
      updateParameter(heightElements[i],heightTextElements[i]);   
  }
  setColorWell("colorWell", color);
  updateBorderAndPadding();
}

function switchUnits(newUnit)
{
  var i;
  var element;
  var saveUnit = currentUnit;
  currentUnit = newUnit;
  
  if (newUnit != saveUnit)
  {
    for (i=0; i<widthElements.length; i++)
    {
      element = document.getElementById(widthTextElements[i]);
      if (element.value != "auto")
      {
        element.value = unitRound(unitsList.convertUnits(element.value,saveUnit,currentUnit));
        updateParameter(widthElements[i],widthTextElements[i]);
      }
    }

    for (i=0; i<heightElements.length; i++)
    {
      element = document.getElementById(heightTextElements[i]);
      if (element.value != "auto")
      {
        element.value = unitRound(unitsList.convertUnits(element.value,saveUnit,currentUnit));
        updateParameter(heightElements[i],heightTextElements[i]);   
      }
    }
  }
  setDecimalPlaces();
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
  for (i=0; i<widthTextElements.length; i++)
  {
    s=widthTextElements[i];
    elt = document.getElementById(s);
    elt.setAttribute("increment", increment);
    elt.setAttribute("decimalplaces", places);
  }
  for (i=0; i<heightTextElements.length; i++)
  {
    s=heightTextElements[i];
    elt = document.getElementById(s);
    elt.setAttribute("increment", increment);
    elt.setAttribute("decimalplaces", places);
  }
  dump("Setting 'increment' to "+increment+" and 'decimalplaces' to "+places+"\n");
}    


function onOK() {
  // add return data to the data object. 
  data.units = currentUnit;
  data.position = document.getElementById('frame.position').value;
  var s = document.getElementById('width').value;
  if (s=="auto") data.width = s;
  else data.width = Number(s);
  var s = document.getElementById('height').value;
  if (s=="auto") data.height = s;
  else data.height = Number(s);
  data.overhang = Number(document.getElementById('overhang').value);
  data.topmargin = Number(document.getElementById('marginTop').value);
  data.sidemargin = Number(document.getElementById('marginSide').value);
  data.border = Number(document.getElementById('border').value);
  data.padding = Number(document.getElementById('padding').value);
//  data.color = color;
  data.textalignment = document.getElementById('textalignment').value;
//   data.borderstyle = document.getElementById('borderstyle').value;
  data.rotation = document.getElementById('rotation').value;
  close();
  return (false);
}

function onCancel() {
  close();
  return(true);
}


function handleChar(event, id, tbid)
{
  var element = event.currentTarget;
  if (event.keyCode == event.DOM_VK_UP)
    goUp(tbid);
  else if (event.keyCode == event.DOM_VK_DOWN)
    goDown(tbid);
  updateParameter(id,tbid);
}

function toPixels(value)
{
  return Math.round(scale*unitsList.convertUnits(value,currentUnit, 'mm'));
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


function updateParameter(id, tbid)
{
  var textbox = document.getElementById(tbid);
  var target;
  if (id) target = document.getElementById(id);
  if (target)
  {
    if (target.hasAttribute('height')) target.height=toPixels(textbox.value);
    else if (target.hasAttribute('width')) target.width=toPixels(textbox.value);
  }
  var overhang; 
  var notewidth;
  var sidemargin; 
  if (tbid == "overhang") // set dependent parameters
  {                                       
    overhang = toPixels(Number(document.getElementById('overhang').value));
    notewidth = Number(document.getElementById('notedata').width);
    sidemargin = Number(document.getElementById('sidemargin').width);
    var textwidth =  100 + overhang - notewidth - sidemargin;
    document.getElementById('textwidth').width = textwidth;
    var reducedleftmargin = 100 - overhang;
    document.getElementById('reducedleftmargin').width = reducedleftmargin;
  }
  else if (tbid == "width")
  {
    notewidth = Number(document.getElementById('notedata').width);
    sidemargin = Number(document.getElementById('sidemargin').width);
    overhang = toPixels(Number(document.getElementById('overhang').value));
    document.getElementById('textwidth').width = 100 + overhang - notewidth - sidemargin;
  }
  else if (tbid == "marginSide")
  {
    sidemargin = Number(document.getElementById('sidemargin').width);

    notewidth = Number(document.getElementById('notedata').width);
    overhang = toPixels(Number(document.getElementById('overhang').value))
    ;
    document.getElementById('textwidth').width = 100+overhang-notewidth-sidemargin;;
  }
}

function setPosition(value)
{
  var diagram = document.getElementById('illustration');
  if (diagram)
  {
    if (value=='inner') diagram.setAttribute("outer","false");
    else if (value=='outer') diagram.setAttribute("outer","true");
    else if (value=='inline') /* do something */;
    else if (value=='display') /* do something */;
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

function updateBorderAndPadding()
{
  var borderdata = document.getElementById('notedata');
  var notebox = document.getElementById('thenote');
  var borderinfo = borderdata.getAttribute('bwidth')+"px "+borderdata.getAttribute('bstyle')+" black";
  setStyleAttribute(notebox, 'border', borderinfo);
  setStyleAttribute(notebox, 'padding', borderdata.getAttribute('pwidth')+"px");
  setStyleAttribute(notebox, 'border-color', 'black');
}

function updatePadding(event,textbox)
{
//  var element = event.currentTarget;
  if (event.keyCode == event.DOM_VK_UP)
    goUp(textbox.id);
  else if (event.keyCode == event.DOM_VK_DOWN)
    goDown(textbox.id);
  var borderdata = document.getElementById('notedata');
  borderdata.setAttribute('pwidth',toPixels(textbox.value));
  updateBorderAndPadding();
}


function updateBorderWidth( event,textbox )
{
  if (event.keyCode == event.DOM_VK_UP)
    goUp(textbox.id);
  else if (event.keyCode == event.DOM_VK_DOWN)
    goDown(textbox.id);
  var borderdata = document.getElementById('notedata');
  borderdata.setAttribute('bwidth',toPixels(textbox.value));
  updateBorderAndPadding();
}
//


//function getColorAndUpdate()
//{
//  var colorWell = document.getElementById("colorWell");
//  if (!colorWell) return;
//  var colorObj = { NoDefault:true, Type:"Rule", TextColor:color, PageColor:0, Cancel:false };
//  window.openDialog("chrome://editor/content/EdColorPicker.xul", "_blank", "chrome,close,titlebar,modal", "", colorObj);
//  // User canceled the dialog
//  if (colorObj.Cancel)
//    return;
//  color = colorObj.TextColor;
//  setColorWell("colorWell", color);
//  var borderdata = document.getElementById('notedata');
//  borderdata.setAttribute('color',color);
//  updateBorderAndPadding();
//}
//

