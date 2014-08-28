var EXPORTED_SYMBOLS = ["initializeFontFamilyList", "gSystemFonts",
  "addOTFontsToMenu", "getOTFontlist" ];
Components.utils.import("resource://app/modules/pathutils.jsm");
Components.utils.import("resource://app/modules/os.jsm");

// font section
var gSystemFonts = { init: false, list: [], count: 0};
const XUL_NS = "http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul";



function initializeFontFamilyList(force, window)
{
  dump ("==== initializeFontFamilyList\n");
  var prefs = GetPrefs();
  var dsprops = Components.classes["@mozilla.org/file/directory_service;1"].createInstance(Components.interfaces.nsIProperties);
  var dir = dsprops.get("ProfD", Components.interfaces.nsIFile);
  var texbindir;
  var outfile;
  var platform = getOS(window);
  dump("==== OS is "+platform+"\n");
  outfile = dir.clone();
  outfile.append("fontfamilies.txt");
  dump("==== Outfile: "+outfile.path+"\n");
  try { texbindir= prefs.getCharPref("swp.tex.bindir"); }
  catch(exc) {
    dump("texbindir not set in preferences\n");
    texbindir = '/usr/texbin';
  }
  if (!force)
  {
    if (outfile.exists()) return;
  }
  var listfile = dir.clone();
  listfile.append("bigfontlist.txt");
  if (listfile.exists()) listfile.remove(false);

  // Mac and Linux don't need the bigfontlist.txt intermediate file
  if ( platform != "win")
  {
    dump("==== BuildFonFamilyList on Mac or Linux\n");
    var exefile = dsprops.get("resource:app", Components.interfaces.nsIFile);;
    exefile.append("BuildFontFamilyList.bash");
    dump("==== Exe file is "+exefile.path+"\n");
    if (!exefile.exists()) return;
    try
    {
      var theProcess = Components.classes["@mozilla.org/process/util;1"].createInstance(Components.interfaces.nsIProcess);
      theProcess.init(exefile);
      var outpath=outfile.path;
      var args=[outpath];
      dump("==== BuildFontFamilyList args = "+args+"\n");
      theProcess.run(true, args, args.length);
    }
    catch (ex)
    {
      dump("\nUnable to run BuildFontFamilyList.bash\n");
      dump(ex+"\n");
      return;
    }
  }
  else {

//    dump("BuildFontFamilyList on Windows\n");
    var exefile = dsprops.get("resource:app", Components.interfaces.nsIFile);
    var useBash = false;
    exefile.append("BuildFontFamilyList.cmd");
    // if (!exefile.exists())
    // {
    //   exefile=exefile.parent;
    //   exefile.append("BuildFontFamilyList.bash");
    //   useBash = true;
    //   if (!exefile.exists()) return;
    // }

    try
    {
      var theProcess = Components.classes["@mozilla.org/process/util;1"].createInstance(Components.interfaces.nsIProcess);
      theProcess.init(exefile);
      dump("TexBinDir is "+texbindir+"\n");
      // for windows only -- we assume only windows will have %programfiles% in it
      // if (useBash)
      //   texbindir = texbindir.replace("%","!","g");
      var outpath=listfile.path;
  //    var opargs = outpath.split(/\s+/);  //Was this what was intended below? Neither version would seem to work...
  //    var opargs = outpath.split(/\w+/);

  //    var args =[texbindir].concat(opargs);
      var args = ['"'+texbindir+'"', outpath];       //Maybe just this?

      theProcess.run(true, args, args.length);
    }
    catch (ex)
    {
         dump("\nUnable to run OtfInfo.exe\n");
         dump(ex+"\n");
         return;
    }
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
  var str = "\n"+myXMLHTTPRequest.responseText;
  var lines = str.split(/[\n\r]+[^:]+:/);
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
      dump("Fontlistfile: "+fontlistfile.path+"\n");
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
  if (!gSystemFonts.init)
    getSysFontList();
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

//This is a fallback in case the otfont mechanism fails
function getSysFontList()
{
//following copied from editor/composer/content/editor.js, but not normally used in favor of otfonts
  var sysFontList;
  if (!gSystemFonts.init)
  {
    // Build list of all local fonts once per editor??
    try
    {
      var enumerator = Components.classes["@mozilla.org/gfx/fontenumerator;1"]
                                 .getService(Components.interfaces.nsIFontEnumerator);
      var localFontCount = { value: 0 }
      sysFontList = enumerator.EnumerateAllFonts(localFontCount);
    }
    catch(e) {dump("Error in fontlist.jsm, getSysFontList: " + e + "\n");}
  }
  if (sysFontList && sysFontList.length)
  {
    gSystemFonts.init = true;
    gSystemFonts.list = [];
    for (var i = 0; i < sysFontList.length; ++i)
      gSystemFonts.list.push(sysFontList[i]);
    gSystemFonts.count = gSystemFonts.list.length;
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
