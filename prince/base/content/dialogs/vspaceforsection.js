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
  // should check for validity.
  spaceElement.setAttribute("height", n);
}


function onCancel()
{
}

function handleChar(a,b)
{
}