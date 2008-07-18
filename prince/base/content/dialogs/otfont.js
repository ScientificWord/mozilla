
const XUL_NS = "http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul";

var gSystemFonts;
var gSystemFontCount;

function  getOTFontlist() 
{ 
  if (!gSystemFonts)
  {
    // Build list of all system fonts once per editor
    try 
    {
      var dsprops = Components.classes["@mozilla.org/file/directory_service;1"].createInstance(Components.interfaces.nsIProperties);
      var fontlistfile=dsprops.get("resource:app", Components.interfaces.nsIFile);
      fontlistfile.append("fontfamilies.txt");
      var stream;
      stream = Components.classes["@mozilla.org/network/file-input-stream;1"];
      stream = stream.createInstance(Components.interfaces.nsIFileInputStream);
      stream.init(fontlistfile,1,0,0);
      var s2 = Components.classes["@mozilla.org/scriptableinputstream;1"];
      s2 = s2.createInstance(Components.interfaces.nsIScriptableInputStream);
      s2.init(stream);
      var bytes = s2.available();
      var buffer = s2.read(bytes);
      gSystemFonts = buffer.split("\n");
      gSystemFontCount = gSystemFonts.length;
      for (var i = 0; i<gSystemFontCount; i++)
      {
        dump(gSystemFonts[i]+"\n");
      }
    }
    catch(e) { 
       3+5;
    }
  }
}
    

function startup()
{
  var menuPopup = document.getElementById("systemfontlist");
  getOTFontlist();
  for (var i = 0; i < gSystemFontCount; ++i)
  {
    if (gSystemFonts[i] != "")
    {
      var itemNode = document.createElementNS(XUL_NS, "menuitem");
      itemNode.setAttribute("label", gSystemFonts[i]);
      itemNode.setAttribute("value", gSystemFonts[i]);
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
      AlertWithTitle("Error", "No editor in otfont.OnAccept!");
    }
	var theWindow = window.opener;
	if (!theWindow || !("msiEditorSetTextProperty" in theWindow))
	  theWindow = msiGetTopLevelWindow();
    theWindow.msiRequirePackage(editorElement, "fontspec", null);
    theWindow.msiEditorSetTextProperty(editorElement, "otfont", "fontname", fontname);
  }
  editorElement.contentWindow.focus();
}

function onCancel()
{
  return true;
}

