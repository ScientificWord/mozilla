Components.utils.import("resource://app/modules/fontlist.jsm"); 
Components.utils.import("resource://app/modules/pathutils.jsm"); 

var node;
var initial;
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

