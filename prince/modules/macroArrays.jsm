var EXPORTED_SYMBOLS = ["macroArray", "fragmentArray",
  "buildFragmentArray", "buildMacroArray", "initializeMacrosAndFragments",
  "getMacro", "getFragment"];

var macroArray = {};
var fragmentArray = {};

function initializeMacrosAndFragments()
{
  buildFragmentArray();
  buildMacroArray();
}

function buildFragmentArray()
{
  fragmentArray = {};
  var dir1;
  dir1 = getUserResourceFile("fragments","");
  var dirpath = dir1.path;
  var ACSA = Components.classes["@mozilla.org/autocomplete/search;1?name=stringarray"].getService();
  ACSA.QueryInterface(Components.interfaces.nsIAutoCompleteSearchStringArray);
  addFragmentsToList(ACSA, dir1);
}

function buildMacroArray()
{
  // Now load the macros file
  // We need to prebuild these so that the keyboard shortcut works
  // ACSA = autocomplete string array
  macroArray = {};
  var ACSA = Components.classes["@mozilla.org/autocomplete/search;1?name=stringarray"].getService();
  ACSA.QueryInterface(Components.interfaces.nsIAutoCompleteSearchStringArray);
  var macrofile;
  macrofile = getUserResourceFile("macros.xml","xml");
  var request = Components.
                classes["@mozilla.org/xmlextras/xmlhttprequest;1"].
                createInstance();
  request.QueryInterface(Components.interfaces.nsIXMLHttpRequest);
  var path = "file:///"+macrofile.target;
  path = path.replace("\\","/","g");
  request.open("GET", path, false);
  request.send(null);
                            
  var xmlDoc = request.responseXML; 
  if (!xmlDoc && request.responseText)
    throw("macros.js exists but cannot be parsed as XML");
  var nodeList;
  var node;
  var s;
  var arrayElement;
  {
    nodeList = xmlDoc.getElementsByTagName("m");
    for (var i = 0; i < nodeList.length; i++)
    {
      node = nodeList.item(i);
      s = node.getAttribute("nm");
      arrayElement = new Object();
      arrayElement.forcesMath = (node.getAttribute("fm") == "1");
      arrayElement.script = (node.getAttribute("tp") == "sc");
      arrayElement.data = node.getElementsByTagName("data").item(0).textContent;
      if (!arrayElement.script)
      {
        arrayElement.context = node.getElementsByTagName("context").item(0).textContent;
        arrayElement.info = node.getElementsByTagName("info").item(0).textContent;
      }
      macroArray[s] = arrayElement;
      ACSA.addString("macros",s);
    }
  }
}



// A recursively-called routine that does the adding of the file names to the list
function addFragmentsToList(autocomplete, dir)
{
  if (dir.isDirectory())
  {
    var enumerator = dir.directoryEntries.QueryInterface(Components.interfaces.nsIDirectoryEnumerator);
    var file;
    var dirpath = dir.path;
    var filename;
    while (enumerator.hasMoreElements())
    {
      file = enumerator.getNext();
      file.QueryInterface(Components.interfaces.nsIFile);
      if (file.isDirectory()) addFragmentsToList(autocomplete,file);
      else
      {
        filename = file.leafName;
        if (filename.length - filename.search(/\.frg$/)==4)
        {
          filename = filename.substring(0, filename.length-4);
          autocomplete.addString("fragments",filename);
          fragmentArray[filename.toLowerCase()] = new Object;
          fragmentArray[filename.toLowerCase()].dirpath = dirpath;
        }
      }
    }
    enumerator.close();
  }
}  
  
function getUserResourceFile( name, resdirname )
{
  var dsprops, userAreaFile, resdir, file, basedir;
  dsprops = Components.classes["@mozilla.org/file/directory_service;1"].getService(Components.interfaces.nsIProperties);
  basedir =dsprops.get("ProfD", Components.interfaces.nsIFile);
  userAreaFile = basedir.clone();
  userAreaFile.append(name);
  if (!userAreaFile.exists())
  { // copy from resource area
    resdir = dsprops.get("resource:app", Components.interfaces.nsIFile);
    if (resdir) resdir.append("res");
    if (resdir) {
      file = resdir.clone();
      if (resdirname && resdirname.length > 0) file.append(resdirname);
      file.append(name);
      try {
        if (file.exists()) file.copyTo(basedir,"");
      }
      catch(e) {
        dump("failed to copy: "+e.toString());
      }
    }
    userAreaFile = file.clone();
  }
  return userAreaFile;
}


function getMacro(key)
{
  return macroArray[key];
}

function getFragment(key)
{
  return fragmentArray[key.toLowerCase()];
}
