
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
      AlertWithTitle("Error", "No editor in color.OnAccept!");
    }
	var theWindow = window.opener;
	if (!theWindow || !("msiInsertHorizontalSpace" in theWindow))
	  theWindow = msiGetTopLevelWindow();
    theWindow.msiRequirePackage(editorElement, "xcolor");
    theWindow.msiEditorSetTextProperty(editorElement, "otfont", "fontname", fontname);
  }
  editorElement.contentWindow.focus();
}

function onCancel()
{
  return true;
}

