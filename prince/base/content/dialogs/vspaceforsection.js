var spaceElement;
var unit;

function startUp()
{
  spaceElement= window.arguments[0];
  if (!spaceElement) return;
  unit = window.arguments[1];
  document.getElementById("spaceHeight").setAttribute("value",spaceElement.getAttribute("height"));
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
    spaceElement.setAttribute("height", n+unit);
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