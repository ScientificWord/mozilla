Components.utils.import("resource://app/modules/unitHandler.jsm");

var color;
var ruleElement;
var ruleUnits;
var unitHandler;


function startUp()
{
  var editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);
  unitHandler = new UnitHandler(editor);
  ruleElement= window.arguments[0];
  if (!ruleElement) return;
//  color = window.arguments[1];
  color = ruleElement.getAttribute("color");
  if (!color) color = "black";
  setColorWell("colorWell",color);
  ruleUnits = window.arguments[1];
  unitHandler.initCurrentUnit(ruleUnits);
  document.getElementById("ruleUnits").setAttribute("value",unitHandler.currentUnit);
  var numberAndUnit;
  var tlheight = ruleElement.getAttribute("tlheight");
  var ht;
  numberAndUnit = unitHandler.getNumberAndUnitFromString(tlheight);
  if (numberAndUnit)
    ht = unitHandler.getValueOf(numberAndUnit.number, numberAndUnit.unit);
  else ht = "";
  document.getElementById("ruleHeight").setAttribute("value",ht);
  var tlwidth = ruleElement.getAttribute("tlwidth");
  var size; // tlwidth recoded for the dialog
  var radio = document.getElementById("rulelength");
  switch (tlwidth) {
    case "-": size = "wide";
      radio.selectedIndex = 0;
      break;
    case "*": size = "narrow";
      radio.selectedIndex = 1;
      break;
    default: numberAndUnit = unitHandler.getNumberAndUnitFromString(tlwidth);
      if (numberAndUnit)
        size = unitHandler.getValueOf(numberAndUnit.number, numberAndUnit.unit);
      else size = "";
      radio.selectedIndex = 2;
      document.getElementById("ruleWidth").setAttribute("value",size);
      break;
  }
  var tlalign = ruleElement.getAttribute("tlalign");
  var selectedIndex;
  switch (tlalign) {
    case "l": selectedIndex = 0; break;
    case "c": selectedIndex = 1; break;
    case "r": selectedIndex = 2; break;
    default: selectedIndex = 0; break;
  }
  document.getElementById("rulealign").selectedIndex = selectedIndex;
}


function getColorAndUpdate()
{
  var colorWell = document.getElementById("colorWell");
  if (!colorWell) return;

  // Don't allow a blank color, i.e., using the "default"
  var colorObj = { NoDefault:true, Type:"Rule", TextColor:color, PageColor:0, Cancel:false };

  window.openDialog("chrome://editor/content/EdColorPicker.xul", "colorpicker", "chrome,close,titlebar,modal,resizable", "", colorObj);

  // User canceled the dialog
  if (colorObj.Cancel)
    return;
  else {
		msiGetEditor(editorElement).incrementModificationCount(1);
	}
  color = colorObj.TextColor;
  setColorWell("colorWell", color);
}


function onAccept()
{
  var n = document.getElementById("ruleHeight").value;
  // should check for validity.
  var nInPixels = unitHandler.getValueOf(n, "px");
  if (nInPixels < 1 && n > 0) nInPixels = 1;
  var element = ruleElement;
  while (element && element.localName != "deck") element = element.parentNode;
    // element is now the deck.
    // should check for validity.
  if (n>0) {
    var style= "background-color:"+color +"; height:"+ nInPixels + "px; ";
    var tlwidth = document.getElementById("rulelength").value;
    switch (tlwidth) {
      case "other":
      {
        tlwidth = document.getElementById("ruleWidth").value+ruleUnits;
        break;
      }
      case "narrow":
        tlwidth = "*";
        break;
      case "wide":
        tlwidth = "-";
        break;
    }
    var align = document.getElementById("rulealign").selectedItem.value;
    ruleElement.setAttribute("tlalign",align);
    ruleElement.setAttribute("tlwidth",tlwidth);
    ruleElement.setAttribute("style", style);
    ruleElement.setAttribute("tlheight", n+ruleUnits);
    ruleElement.setAttribute("color", color);
    element.setAttribute("selectedIndex", "1");
  }
  else
    element.setAttribute("selectedIndex", "0");
}


function onCancel()
{
  return true;
}

function handleChar(a,b)
{
}
