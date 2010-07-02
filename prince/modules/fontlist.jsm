var EXPORTED_SYMBOLS = ["initializeFontFamilyList", "gSystemFonts", 
  "addOTFontsToMenu", "getOTFontlist" ];
Components.utils.import("resource://app/modules/pathutils.jsm"); 

// font section
var gSystemFonts = { init: false, list: [], count: 0};
const XUL_NS = "http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul";



function initializeFontFamilyList(force)
{
  var prefs = GetPrefs();
  var dsprops = Components.classes["@mozilla.org/file/directory_service;1"].createInstance(Components.interfaces.nsIProperties);
  var dir = dsprops.get("ProfD", Components.interfaces.nsIFile);
  var texbindir;
  var outfile;
  outfile = dir.clone();
  outfile.append("fontfamilies.txt");
  try { texbindir= prefs.getCharPref("swp.tex.bindir"); }
  catch(exc) {dump("texbindir not set in preference\n");}
  if (!force)
  { 
    if (outfile.exists()) return;
  }
  var listfile = dir.clone(); 
  listfile.append("bigfontlist.txt");
  if (listfile.exists()) listfile.remove(false);
  var exefile = dsprops.get("resource:app", Components.interfaces.nsIFile);;
  exefile.append("BuildFontFamilyList.cmd");

  try 
  {
    var theProcess = Components.classes["@mozilla.org/process/util;1"].createInstance(Components.interfaces.nsIProcess);
    theProcess.init(exefile);
    dump("TexBinDir is "+texbindir+"\n");
    var args =[listfile.parent.path, texbindir];
    theProcess.run(true, args, args.length);
  } 
  catch (ex) 
  {
       dump("\nUnable to run OtfInfo.exe\n");
       dump(ex+"\n");
  }      
  if (!listfile.exists())
  {
    dump("Failed to create bigfontlist.txt\n");
    return;
  }
  var uri = msiFileURLFromAbsolutePath( listfile.path )
  var myXMLHTTPRequest = Components.classes["@mozilla.org/xmlextras/xmlhttprequest;1"]  
                     .createInstance(Components.interfaces.nsIXMLHttpRequest);  
  myXMLHTTPRequest.overrideMimeType("text/plain");
  myXMLHTTPRequest.open("GET", uri.spec, false);
  myXMLHTTPRequest.send(null);
  var str = myXMLHTTPRequest.responseText;
  var lines = str.split(/[\n\r]*[a-z]:[^:]*:/i);
  var i;
  var limit;
  lines = lines.sort();
  var unique = lines.filter(newValue);
  limit = unique.length;
// output the result
  str = "";
  if (outfile.exists()) outfile.remove(false);
  for (i =0; i < limit; i++)
    str += unique[i] + "\n";
  var fos = Components.classes["@mozilla.org/network/file-output-stream;1"].createInstance(Components.interfaces.nsIFileOutputStream);
  fos.init(outfile, -1, -1, false);
  var os = Components.classes["@mozilla.org/intl/converter-output-stream;1"]
    .createInstance(Components.interfaces.nsIConverterOutputStream);
  os.init(fos, "UTF-8", 4096, "?".charCodeAt(0));
  os.writeString(str);
  os.close();
  fos.close();
//  dump(str);
}


function newValue(element, index, array)
{
  if (!element.match(/\S/)) return false;
  if (index == 0) return true;
  return element != array[index-1];
}

function  getOTFontlist() 
{ 
  if (!gSystemFonts.init)
  {
    // Build list of all system fonts once per editor
    try 
    {
      var dsprops = Components.classes["@mozilla.org/file/directory_service;1"].createInstance(Components.interfaces.nsIProperties);
      var fontlistfile=dsprops.get("ProfD", Components.interfaces.nsIFile);
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
      gSystemFonts.list = buffer.split("\n");
      gSystemFonts.count = gSystemFonts.list.length;
	  gSystemFonts.init = true; // we won't have to do this again.
    }
    catch(e) { 
       dump("Error in getOTFontList: "+e.message+"\n");
    }
  }
}

function addOTFontsToMenu(menu)
{
  try
  {
    getOTFontlist();
    var separator = menu.menulist.ownerDocument.createElementNS(XUL_NS, "menuseparator");
	var popup = menu.menulist.getElementsByTagName("menupopup")[0];
    separator.setAttribute("id","startOpenType");
    popup.appendChild(separator);
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
    dump(e + "\n");
  }
}
 
var gPrefsService;   
var gPrefsBranch;


function GetPrefsService()
{
  if (gPrefsService)
    return gPrefsService;

  try {
    gPrefsService = Components.classes["@mozilla.org/preferences-service;1"].getService(Components.interfaces.nsIPrefService);
  }
  catch(ex) {
    dump("failed to get prefs service!\n");
  }

  return gPrefsService;
}

function GetPrefs()
{
  if (gPrefsBranch)
    return gPrefsBranch;

  try {
    var prefService = GetPrefsService();
    if (prefService)
      gPrefsBranch = prefService.getBranch(null);

    if (gPrefsBranch)
      return gPrefsBranch;
    else
      dump("failed to get root prefs!\n");
  }
  catch(ex) {
    dump("failed to get root prefs!\n");
  }
  return null;
}
