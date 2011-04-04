Components.utils.import("resource://app/modules/fontlist.jsm"); 
Components.utils.import("resource://app/modules/pathutils.jsm"); 

var node;
var initial;
var data = { ruleColor: "#000000" };

function startup()
{
  if (window.arguments && window.arguments.length > 0)
    node = window.arguments[0];
  var menuObject = { };
  initializeFontFamilyList(false);
  menuObject.menulist = document.getElementById("otfontlist");
  addOTFontsToMenu(menuObject);
  if (node)
  {
    initial = node.getAttribute("fontname");
    document.getElementById("otfontlist").value = initial;
  }
}

function getColorAndUpdate()
{
  var colorWell = document.getElementById("colorWell");
  if (!colorWell) return;

  var colorObj = { NoDefault: false, Type: "Rule", TextColor: data.ruleColor, PageColor: 0, Cancel: false };

  window.openDialog("chrome://editor/content/EdColorPicker.xul", "colorpicker", "chrome,close,titlebar,modal", "", colorObj);

  // User canceled the dialog
  if (colorObj.Cancel)
    return;

  data.ruleColor = colorObj.TextColor;
  setColorWell("colorWell", data.ruleColor); 
}




function onAccept()
{
  var list = document.getElementById("otfontlist");
  var fontname;
  if (list.selectedIndex  > 0)
  {
    fontname = list.value;
    var editorElement = msiGetParentEditorElementForDialog(window);
    if (!editorElement)
    {
      AlertWithTitle("Error", "No editor in otfont.OnAccept!");
    }
  }
  if (node)
  {
    node.setAttribute("fontname", fontname);
    node.setAttribute("style","font-family: "+fontname+";");
  }
  else
  {
	  var theWindow = window.opener;
	  if (!theWindow || !("msiEditorSetTextProperty" in theWindow))
    {
	    theWindow = msiGetTopLevelWindow();
    }
    theWindow.msiRequirePackage(editorElement, "xltxtra", null);
    theWindow.msiEditorSetTextProperty(editorElement, "otfont", "fontname", fontname);
  }
  editorElement.contentWindow.focus();
}

function onCancel()
{
  return true;
}

