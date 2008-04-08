
function focusOnEditor()
{
  var editWindow = document.getElementById('content-frame');
  editWindow.contentWindow.focus();         
}

// we keep two objects that serve as associative arrays for the macros and fragments
var macroArray;
var fragmentArray;

// builds the directory tree of fragment files and builds the list of fragment files for
// calling by the keyboard. The boolean 'buildListOnly' is true when we don't need to rebuild
// the tree but do need to build the list, as, for example, right after the ref has been
// assigned.
function rebuildFragmentTree(tree, buildListOnly)
{
  if (!buildListOnly) tree.builder.rebuild();
  var ACSA = Components.classes["@mozilla.org/autocomplete/search;1?name=stringarray"].getService();
  ACSA.QueryInterface(Components.interfaces.nsIAutoCompleteSearchStringArray);
  var dir = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
  var dirpath = tree.getAttribute("ref");
  dirpath = dirpath.substring(8,dirpath.length-1);
    // take ref attribute and lop off "file:///" and last "/"
  // for Windows
#ifdef XP_WIN32
  dirpath = dirpath.replace("/","\\","g");
#endif
  dir.initWithPath(dirpath);
   addFragmentsToList(ACSA, dir);
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
          fragmentArray[filename] = new Object;
          fragmentArray[filename].dirpath = dirpath;
        }
      }
    }
    enumerator.close();
  }
}  
  

function initSidebar()
{
  // Since RDF file trees can't take RESOURCE:// path names, we need to convert the
  // fragmentsBaseDirectory to a FILE:// url.
  macroArray = new Object();
  fragmentArray = new Object();
  var dsprops = Components.classes["@mozilla.org/file/directory_service;1"].getService(Components.interfaces.nsIProperties);
  var basedir =dsprops.get("resource:app", Components.interfaces.nsIFile);
  basedir.append("res");
  var dir1 = basedir.clone();
  dir1.append("fragments");
  var dirpath = dir1.path + "/";
#ifdef XP_WIN32
  dirpath = dirpath.replace("\\","/","g");
#endif
  var fragmentsBaseDirectory = "file:///" + dirpath;
  document.getElementById("frag-tree").setAttribute("ref", fragmentsBaseDirectory);
  rebuildFragmentTree(document.getElementById("frag-tree"),true);
  //
  // Now load the macros file
  // We need to prebuild these so that the keyboard shortcut works
  // ACSA = autocomplete string array
  var ACSA = Components.classes["@mozilla.org/autocomplete/search;1?name=stringarray"].getService();
  ACSA.QueryInterface(Components.interfaces.nsIAutoCompleteSearchStringArray);
  var macrofile = basedir;
  macrofile.append("tagdefs");
  macrofile.append("macros.xml");
  var request = Components.
                classes["@mozilla.org/xmlextras/xmlhttprequest;1"].
                createInstance();
  request.QueryInterface(Components.interfaces.nsIXMLHttpRequest);
  var path = "file:///"+macrofile.target;
#ifdef XP_WIN32
  path = path.replace("\\","/","g");
#endif
  try {
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
  catch (e) {
    dump("exception: "+e+"\n");
  }
  
}

function insertFragmentContents( tree, pathname)
{
  // pathname is now the path of the clicked file relative to the fragment root.
  try 
  {
    var editorElement = document.getElementById("content-frame");
    var editor = msiGetEditor(editorElement);
    var xmlDoc = document.implementation.createDocument("", "frag", null);
    xmlDoc.async = false;
    if (xmlDoc.load(pathname))
    {
      var contextString="";
      var dataString=null;
      var infoString=null; 
      var node;
      node = xmlDoc.getElementsByTagName("data").item(0);
      if (node)
      {
        node = node.firstChild;
        while (node && node.nodeType != node.CDATA_SECTION_NODE) node = node.nextSibling;
        if (node) dataString = node.nodeValue;
      }
      if (dataString.length == 0) return;
      node = xmlDoc.getElementsByTagName("context").item(0);
      if (node) 
      {
        node = node.firstChild;
        while (node && node.nodeType != node.CDATA_SECTION_NODE) node = node.nextSibling;
        if (node) contextString = node.nodeValue;
      }
      node  = xmlDoc.getElementsByTagName("info").item(0);
      if (node)
      {
        node = node.firstChild;
        while (node && node.nodeType != node.CDATA_SECTION_NODE) node = node.nextSibling;
        if (node) infoString = node.nodeValue;
      }
      focusOnEditor();
      editor.insertHTMLWithContext(dataString,
                                   contextString, infoString, "text/html",
                                   null,null,0,true);
    }
  }
  catch(e)
  {
//    alert(e);
  }
  focusOnEditor();
}

function loadFragment(event,tree)
{
  var namecol = tree.columns.getNamedColumn('Name');
  var i = tree.currentIndex;
  var s = tree.view.getCellText( i,namecol);
  if (event.type=="keypress" && event.keyCode!=event.DOM_VK_RETURN) return;
  if (!tree.view.isContainer(i))
  {  
    while (tree.view.getParentIndex(i) >= 0)
    {           
      i = tree.view.getParentIndex(i);
      s = tree.view.getCellText(i,namecol)+ "/" + s;
    }
  }
  s = tree.getAttribute("ref") + "/" +s;
  insertFragmentContents(tree, s);
  focusOnEditor();
}

function insertDataAtCursor( arrayElement )
{
  try
  {
    focusOnEditor();
    if (arrayElement.script)
    {
      eval(arrayElement.data);
    }
    else
    {
      var editorElement = document.getElementById("content-frame");
      var editor = msiGetEditor(editorElement);
      editor.insertHTMLWithContext(arrayElement.data,
                                   arrayElement.context, arrayElement.info, "text/xml",
                                   null,null,0,true);
    }
  } catch(e) {dump("Error in insertDataAtCursor: "+e); }
}



function onMacroOrFragmentEntered( aString )
{
  var s = macroArray[aString];
  if (s) 
  {
    insertDataAtCursor( s );
  }
  else
  {
    s = fragmentArray[aString];
    if (s)
    {
      var tree = document.getElementById("frag-tree");
      if (!tree) return;
      var dirpath = s.dirpath;
#ifdef XP_WIN32
      dirpath = dirpath.replace("\\","/","g");
#endif
      dirpath = "file:///" + dirpath + "/" + aString + ".frg";      
      
      insertFragmentContents( tree, dirpath );
    }
  }
  var macrofragmentStatusPanel = document.getElementById('macroEntryPanel');
  if (macrofragmentStatusPanel)
    macrofragmentStatusPanel.setAttribute("hidden", "true");
  focusOnEditor();
}


var fragObserver = 
{ 
//    canHandleMultipleItems: function ()
//    {
//      return true;
//    },
  
  onDragStart: function (evt, transferData, action)
  {
    var tree = evt.currentTarget;
    var namecol = tree.columns.getNamedColumn('Name');
    var i = tree.currentIndex;
    var s = tree.view.getCellText( i,namecol);
    if (!tree.view.isContainer(i))
    {  
      while (tree.view.getParentIndex(i) >= 0)
      {           
        i = tree.view.getParentIndex(i);
        s = tree.view.getCellText(i,namecol)+ "/" + s;
      }
    }
    // s is now the path of the clicked file relative to the fragment root.
    try 
    {
      var xmlDoc = document.implementation.createDocument("", "frag", null);
      xmlDoc.async = false;
      var path = tree.getAttribute("ref") + "/" + s;
      if (xmlDoc.load(path))
      {
        var dataString="";
        var contextString="";
        var infoString="";
        var node;
        var nodelist;
        nodelist = xmlDoc.getElementsByTagName("data");
        if (nodelist.length > 0) node = nodelist.item(0);
        if (node)
        {
          node = node.firstChild;
          while (node && node.nodeType != node.CDATA_SECTION_NODE) node = node.nextSibling;
          if (node) dataString = node.nodeValue;
        }
        if (dataString.length == 0) return;  // no point in going on in this case
        node = null;
        nodelist = xmlDoc.getElementsByTagName("context");
        if (nodelist.length > 0) node = nodelist.item(0);
        if (node)
        {
          node = node.firstChild;
          while (node && node.nodeType != node.CDATA_SECTION_NODE) node = node.nextSibling;
          if (node) contextString = node.nodeValue;
        }
        node = null;
        nodelist = xmlDoc.getElementsByTagName("info");
        if (nodelist.length > 0) node = nodelist.item(0);
        if (node)
        {
          node = node.firstChild;
          while (node && node.nodeType != node.CDATA_SECTION_NODE) node = node.nextSibling;
          if (node) infoString = node.nodeValue;
        }
        transferData.data = new TransferData();
        transferData.data.addDataForFlavour("privatefragmentfile", path);
        transferData.data.addDataForFlavour("text/html",dataString);
        transferData.data.addDataForFlavour("text/_moz_htmlcontext",contextString);
        transferData.data.addDataForFlavour("text/_moz_htmlinfo",infoString);
      }
    }
    catch(e) {}
    focusOnEditor();
  },
  
  onDrop: function(evt, dropData, session)
  {
    var tree = evt.currentTarget;
    var bo = tree.treeBoxObject;
    var namecol = tree.columns.getNamedColumn('Name');
    var saveref = tree.getAttribute("ref");
    var pathbase = saveref + "/";
    pathbase = pathbase.substr(8); // omit "path:///" at the start
    var path="";
    var row = new Object;
    var column = new Object;
    var part = new Object;
    bo.getCellAt(evt.clientX, evt.clientY, row, column, part);
    var i = row.value;
    if (i >= 0)
    {
      if (tree.view.isContainer(i)) 
        path = tree.view.getCellText(i,namecol)+"/";
      while (tree.view.getParentIndex(i) >= 0)
      {           
        i = tree.view.getParentIndex(i);
        path = tree.view.getCellText(i,namecol)+ "/" + path;
      }
    }
//    dump("New fragment file path is "+pathbase + path + "\n");
    if (session.isDataFlavorSupported("privatefragmentfile"))
    {
      var origPath = dropData.data;
      // now move origPath to path
      var origfile = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
      file.QueryInterface(Components.interfaces.nsIFile);
      file.initWithPath( origPath );
      file.moveTo(path);
    }
    else if (session.isDataFlavorSupported("text/html"))
    {
      var data = new Object();
      window.openDialog("chrome://prince/content/fragmentname.xul", "", "modal,chrome,resizable=yes", data);
      if (data.filename.length > 0)
      {
// Now we collect several data formats
        var mimetypes = new Object();
        mimetypes.kHTMLMime                    = "text/html";
        mimetypes.kHTMLContext                 = "text/_moz_htmlcontext";
        mimetypes.kHTMLInfo                    = "text/_moz_htmlinfo";
        var trans = Components.classes["@mozilla.org/widget/transferable;1"].
          createInstance(Components.interfaces.nsITransferable); 
        if (!trans) return; 
        for (var i in mimetypes)
        {
          var flavour = mimetypes[i];
          trans.addDataFlavor(flavour);
          session.getData(trans,0); 
          var str = new Object();
          var strLength = new Object();
          try
          {
            trans.getTransferData(flavour,str,strLength);
            if (str) str = str.value.QueryInterface(Components.interfaces.nsISupportsString); 
            if (str) mimetypes[i] = str.data.substring(0,strLength.value / 2);
          }
          catch (e)
          {
            dump("  "+flavour+" not supported\n\n");
            mimetypes[i] = "";
          }
          trans.removeDataFlavor(flavour);
        }
        
        var sFileContent = '<?xml version="1.0"?>\n<fragment>\n  <data>\n    <![CDATA[' +mimetypes.kHTMLMime+
          ']]>\n  </data>\n  <context>\n    <![CDATA[' +mimetypes.kHTMLContext+
          ']]>\n  </context>\n  <info>\n    <![CDATA[' +mimetypes.kHTMLInfo+
          ']]>\n  </info>\n</fragment>';
        var filepath = pathbase + path + data.filename;
        if (filepath.search(/.frg/) == -1) filepath += ".frg";
#ifdef XP_WIN32
        filepath = filepath.replace("/","\\","g");
#endif
        try
        {
          var file = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
          file.QueryInterface(Components.interfaces.nsIFile);
          file.initWithPath( filepath );
          if( file.exists() == true ) file.remove( false );
          var strm = Components.classes["@mozilla.org/network/file-output-stream;1"].createInstance(Components.interfaces.nsIFileOutputStream);
          strm.QueryInterface(Components.interfaces.nsIOutputStream);
          strm.QueryInterface(Components.interfaces.nsISeekableStream);
          strm.init( file, 0x04 | 0x08, 420, 0 );
          strm.write( sFileContent, sFileContent.length );
          strm.flush();
          strm.close();
        }
        catch(ex)
        {
          window.alert(ex.message);
        } 
        rebuildFragmentTree(tree,false);
      }
    }
  },
  
  onDragOver: function(evt, flavour, session) 
  {
//    dump(flavour.contentType+"\n");
  },
  
  getSupportedFlavours: function()
  {
    var flavours = new FlavourSet();
    flavours.appendFlavour("privatefragmentfile");
    flavours.appendFlavour("text/html");
    return flavours;
  }
}  
