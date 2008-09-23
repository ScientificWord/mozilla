Components.utils.import("resource://app/modules/macroArrays.jsm");

function focusOnEditor()
{
  var editWindow = document.getElementById('content-frame');
  if (!editWindow && opener) editWindow = opener.document.getElementById('content-frame');
  if (editWindow) editWindow.contentWindow.focus();         
}



function initSidebar()
{
  // Since RDF file trees can't take RESOURCE:// path names, we need to convert the
  // fragmentsBaseDirectory to a FILE:// url.
  try {
    var dir1;
    dir1 = getUserResourceFile("fragments","");
    var dirpath = dir1.path + "/";
#ifdef XP_WIN32
    dirpath = dirpath.replace("\\","/","g");
#endif
    var fragmentsBaseDirectory = "file:///" + dirpath;
    document.getElementById("frag-tree").setAttribute("ref", fragmentsBaseDirectory);
    document.getElementById("frag-tree").builder.rebuild();
    //
  }
  catch(e) {
    dump("Exception in initSidebar() = "+e.toString());
  }
}

function insertFragmentContents( pathname)
{
  // pathname is now the path of the clicked file relative to the fragment root.
  try 
  {
    var editorElement = document.getElementById("content-frame");
    var editor = msiGetEditor(editorElement);
    var request = Components.
                  classes["@mozilla.org/xmlextras/xmlhttprequest;1"].
                  createInstance();
    request.QueryInterface(Components.interfaces.nsIXMLHttpRequest);
    request.open("GET", pathname, false);
    request.send(null);
                                
    var xmlDoc = request.responseXML; 
    if (xmlDoc)
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
  var s = getMacro(aString);
  if (s) 
  {
    insertDataAtCursor( s );
  }
  else
  {
    s = getFragment(aString);
    if (s)
    {
      var dirpath = s.dirpath;
      dirpath = dirpath.replace("\\","/","g");
      dirpath = "file:///" + dirpath + "/" + aString + ".frg";      
      
      insertFragmentContents( dirpath );
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
      var request = Components.
                    classes["@mozilla.org/xmlextras/xmlhttprequest;1"].
                    createInstance();
      request.QueryInterface(Components.interfaces.nsIXMLHttpRequest);
      var path = tree.getAttribute("ref") + "/" + s;
#ifdef XP_WIN32
      path = path.replace("\\","/","g");
#endif
      request.open("GET", path, false);
      request.send(null);
                          
      var xmlDoc = request.responseXML; 
      if (!xmlDoc && request.responseText)
        throw("fragment file exists but cannot be parsed as XML");
      if (xmlDoc)
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
        buildFragmentArray();
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
