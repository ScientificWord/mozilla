Components.utils.import("resource://app/modules/fontlist.jsm"); 
Components.utils.import("resource://app/modules/pathutils.jsm"); 


function startup()
{
  var menuObject = { };
  initializeFontFamilyList(false);
  menuObject.menulist = document.getElementById("otfontlist");
  addOTFontsToMenu(menuObject);
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
	var theWindow = window.opener;
	if (!theWindow || !("msiEditorSetTextProperty" in theWindow))
  {
	  theWindow = msiGetTopLevelWindow();
  }
  theWindow.msiRequirePackage(editorElement, "xltxtra", null);
  theWindow.msiEditorSetTextProperty(editorElement, "otfont", "fontname", fontname);
  editorElement.contentWindow.focus();
}

function onCancel()
{
  return true;
}

