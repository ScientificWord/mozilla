
const XUL_NS = "http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul";

function startup()
{
  var menuPopup = document.getElementById("systemfontlist");
  var fontlister = Components.classes["@mackichan.com/otfontlist;1"]
    .getService(Components.interfaces.msiIOTFontlist);
  var systemFontCount = new Object();
  var systemfonts = fontlister.getOTFontlist(systemFontCount);
  for (var i = 0; i < systemfonts.length; ++i)
  {
    if (systemfonts[i] != "")
    {
      var itemNode = document.createElementNS(XUL_NS, "menuitem");
      itemNode.setAttribute("label", systemfonts[i]);
      itemNode.setAttribute("value", systemfonts[i]);
      menuPopup.appendChild(itemNode);
    }
  }
}


function switchUnits()
{
  currentUnit = document.getElementById("otfont.units").value;
  document.getElementById('otfont.unit').setAttribute('value',currentUnit);
}


function handleChar(event, widget)
{
  var element = event.currentTarget;
  if (event.keyCode == event.DOM_VK_UP)
    goUp(widget);
  else if (event.keyCode == event.DOM_VK_DOWN)
    goDown(widget);
}

function goUp(widget)
{
  var value = Number(widget.value);
  if (value == NaN) return;
  var max = Number(widget.getAttribute("max"));
  if ((max == 0)||(max == NaN)) max = Number.MAX_VALUE;
  value += Number(widget.getAttribute("increment"));
  value = Math.min(max,value);
  widget.value = unitRound(value);
}

function goDown(widget)
{
  var value = Number(widget.value);
  if (value == NaN) return;
  var min = Number(widget.getAttribute("min"));
  if (min == NaN) min = 0;
  value -= Number(widget.getAttribute("increment"));
  value = Math.max(min,value);
  widget.value = unitRound(value);
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


