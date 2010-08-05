Components.utils.import("resource://app/modules/unitHandler.jsm"); 

var spaceElement;
var unit;
var unitHandler = new UnitHandler();

function startUp()
{
  spaceElement= window.arguments[0];
  if (!spaceElement) return;
  unit = window.arguments[1];
  var spaceHeight = ruleElement.getAttribute("tlheight"); 
  var ht;
  numberAndUnit = unitHandler.getNumberAndUnitFromString(spaceHeight);
  if (numberAndUnit)
    ht = unitHandler.getValueOf(numberAndUnit.number, numberAndUnit.unit);
  else ht = "";
  document.getElementById("spaceHeight").setAttribute("value",ht);
  document.getElementById("vspaceUnits").setAttribute("value",unit);
}


function onAccept()
{
  var n = document.getElementById("spaceHeight").value;
  var element = spaceElement;
  while (element && element.localName != "deck") element = element.parentNode;
    // element is now the deck.
  // should check for validity.
  if (n>0) {
    spaceElement.setAttribute("tlheight", n+unit);
    element.setAttribute("selectedIndex", "1");
  }
  return true;
}


function onCancel()
{
  return true;
}

function handleChar(a,b)
{
}