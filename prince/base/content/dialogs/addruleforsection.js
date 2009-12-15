var color;
var ruleElement;
var ruleUnits;


function startUp()
{
  ruleElement= window.arguments[0];
  if (!ruleElement) return;
  color = window.arguments[1];
  if (!color) color = ruleElement.getAttribute("color");
  setColorWell("colorWell",color);
  ruleUnits = window.arguments[2];
  var size = ruleElement.getAttribute("size");
  document.getElementById("ruleUnits").setAttribute("value",ruleUnits);
  var size = ruleElement.getAttribute("size");
  if (size == "wide" || size == "narrow") document.getElementById("rulesize").selectedIndex=(size=="wide"?0:1);
  else {
    document.getElementById("rulesize").selectedIndex=2;
    document.getElementById("ruleWidth").setAttribute("value",size);
  }
}


function getColorAndUpdate()
{
  var colorWell = document.getElementById("colorWell");
  if (!colorWell) return;

  // Don't allow a blank color, i.e., using the "default"
  var colorObj = { NoDefault:true, Type:"Rule", TextColor:color, PageColor:0, Cancel:false };

  window.openDialog("chrome://editor/content/EdColorPicker.xul", "colorpicker", "chrome,close,titlebar,modal", "", colorObj);

  // User canceled the dialog
  if (colorObj.Cancel)
    return;

  color = colorObj.TextColor;
  setColorWell("colorWell", color); 
}


function onAccept()
{
  var n = document.getElementById("ruleHeight").value;
  // should check for validity.
  var element = ruleElement;
  while (element && element.localName != "deck") element = element.parentNode;
    // element is now the deck.
    // should check for validity.
  if (n>0) {
    ruleElement.setAttribute("height", n+ruleUnits);
    ruleElement.setAttribute("color", color);
    var style= "background-color:"+color +"; height:"+n+ruleUnits;
    var size = document.getElementById("rulesize").value+ruleUnits;
    if (size=="other") 
    {
      size = document.getElementById("ruleWidth").value+ruleUnits;
      style += ";width:"+size;
    }
    ruleElement.setAttribute("size",size);
    ruleElement.setAttribute("style", style);
    element.setAttribute("selectedIndex", "1");
  }
  else
    element.setAttribute("selectedIndex", "0");
}


function onCancel()
{
}

function handleChar(a,b)
{
}