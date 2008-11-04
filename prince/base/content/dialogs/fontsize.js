
const XUL_NS = "http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul";

function onAccept()
{
  var size=Number(document.getElementById("otfont.fontsize").value);
  if (size==NaN) return;
  var units = document.getElementById("otfont.units").value;
  var leading=Number(document.getElementById("leading").value);
  var sizewithunits = size+"/"+leading+" "+units;
  var editorElement = msiGetParentEditorElementForDialog(window);
  if (!editorElement)
  {
    AlertWithTitle("Error", "No editor in otfont.OnAccept!");
  }
	var theWindow = window.opener;
	if (!theWindow || !("msiEditorSetFontSize" in theWindow))
	  theWindow = msiGetTopLevelWindow();
  theWindow.msiEditorSetFontSize(sizewithunits, editorElement);
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


