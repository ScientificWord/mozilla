var EXPORTED_SYMBOLS = ["initializeFontFamilyList", "gSystemFonts",
  "addOTFontsToMenu"];
Components.utils.import("resource://app/modules/pathutils.jsm");
Components.utils.import("resource://app/modules/os.jsm");

// font section
var gSystemFonts = { init: false, list: [], count: 0};
const XUL_NS = "http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul";



function initializeFontFamilyList(force)
{
  if (force) {
    gSystemFonts.init = false;
  }
  getSysFontList();
}

function addOTFontsToMenu(menu)
{
  try
  {
    var separator = menu.menulist.ownerDocument.createElementNS(XUL_NS, "menuseparator");
	  var popup = menu.menulist.getElementsByTagName("menupopup")[0];
    separator.setAttribute("id","startOpenType");
    popup.appendChild(separator);
    if (!gSystemFonts.init) {
      getSysFontList();
    }
    for (var i = 0; i < gSystemFonts.count; ++i)
    {
      if (gSystemFonts.list[i] != "")
      {
        var itemNode = menu.menulist.ownerDocument.createElementNS(XUL_NS, "menuitem");
        itemNode.setAttribute("label", gSystemFonts.list[i]);
        itemNode.setAttribute("value", gSystemFonts.list[i]);
        popup.appendChild(itemNode);
      }
    }
  }
  catch(e)
  {
    finalThrow(cmdFailString("buildfontmenus"), e.message);
  }
}

//This is a fallback in case the otfont mechanism fails
function getSysFontList()
{
  if (!gSystemFonts.init)
  {
    try
    {
      var enumerator = Components.classes["@mozilla.org/gfx/fontenumerator;1"]
                                 .getService(Components.interfaces.nsIFontEnumerator);
      var localFontCount = { value: 0 }
      gSystemFonts.list = enumerator.EnumerateAllFonts(localFontCount);
      gSystemFonts.init = true;
      gSystemFonts.count = localFontCount.value;
    }
    catch(e) {
      finalThrow(cmdFailString("getSystemFonts"), e.message);
    }
  }
}

