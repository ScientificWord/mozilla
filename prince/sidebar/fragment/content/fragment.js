Components.utils.import("resource://app/modules/macroArrays.jsm");

var pathSeparator = 
#ifdef XP_WIN32
    "\\";
#else
    "/"
#endif

function focusOnEditor()
{
  var editWindow = document.getElementById('content-frame');
  if (!editWindow && opener) editWindow = opener.document.getElementById('content-frame');
  if (editWindow) editWindow.contentWindow.focus();         
}

function refresh(tree)
{
  buildFragmentArray();
  tree.builder.rebuild();
  fixAttributes(tree);
}

function fixAttributes(tree)
{
//  var tree = document.getElementById("frag-tree");
  // strangely, attributes of the treechildren node are not preserved, so we have to set it now.
  var i;
  var  kids = tree.getElementsByTagName("treechildren");
  if (!kids || kids.length != 2) alert("Strange number of 'treechildren' in tree");
  var generated = kids[1];
  var attrs = kids[0].attributes;
  {
    for (i = 0; i < attrs.length; i++) 
    { 
      generated.setAttribute(attrs[i].name, attrs[i].value);
    }
  }
}

function initSidebar()
{
  // Since RDF file trees can't take RESOURCE:// path names, we need to convert the
  // fragmentsBaseDirectory to a FILE:// url.
  try {
    var dir1;
    dir1 = getUserResourceFile("fragments","");
    var dirpath = dir1.path + "/";
    var fragmentsBaseDirectory = msiFileURLFromAbsolutePath( dirpath );
    var tree = document.getElementById("frag-tree");
    tree.setAttribute("ref", fragmentsBaseDirectory);
    tree.builder.rebuild();
    fixAttributes(tree);
    //
  }
  catch(e) {
    dump("Exception in initSidebar() = "+e.toString());
  }
}

function descriptionOfItem( row )
{
  var i = row;
  var tree = document.getElementById("frag-tree");
  var namecol = tree.columns.getNamedColumn('Name');
  var s = tree.view.getCellText( i,namecol);
  while (tree.view.getParentIndex(i) >= 0)
    {           
      i = tree.view.getParentIndex(i);
      s = tree.view.getCellText(i,namecol)+ pathSeparator + s;
  }
  s = tree.getAttribute("ref") + "/" +s;
  // s is now the path of the clicked file relative to the fragment root.
  dump("showDescription: pathname = "+s+"\n");
  try 
  {
    var request = Components.
                  classes["@mozilla.org/xmlextras/xmlhttprequest;1"].
                  createInstance();
    request.QueryInterface(Components.interfaces.nsIXMLHttpRequest);
    request.open("GET", s, false);
    request.send(null);
                                
    var xmlDoc = request.responseXML; 
    if (xmlDoc)
    {
      var node;
      node = xmlDoc.getElementsByTagName("description").item(0);
      if (node)
      {
        node = node.firstChild;
        while (node && node.nodeType != node.CDATA_SECTION_NODE) node = node.nextSibling;
        if (node) return node.nodeValue;
      }
    }
  }
  catch (e)
  {
    dump("Error is fragments.js, showDescription: "+e.message+"\n");
  }
  return "";
}

function writeDescriptionOfItem(row, desc)
{
  var i = row;
  var tree = document.getElementById("frag-tree");
  var namecol = tree.columns.getNamedColumn('Name');
  var s = tree.view.getCellText( i,namecol);
  var outfile = getUserResourceFile("fragments","");
  while (tree.view.getParentIndex(i) >= 0)
  {           
    i = tree.view.getParentIndex(i);
    s = tree.view.getCellText(i,namecol)+pathSeparator+s;
  }
  // s is now the path of the clicked file relative to the fragment root.
  var pieces = s.split(pathSeparator);
  var j;
  for (j = 0; j < pieces.length; j++) outfile.append(pieces[j]);
  s = tree.getAttribute("ref") +s;
  dump("showDescription: pathname = "+s+"\n");
  try 
  {
    var request = Components.
                  classes["@mozilla.org/xmlextras/xmlhttprequest;1"].
                  createInstance();
    request.QueryInterface(Components.interfaces.nsIXMLHttpRequest);
    request.open("GET", s, false);
    request.send(null);
                                
    var xmlDoc = request.responseXML; 
    if (xmlDoc)
    {
      var node;
      node = xmlDoc.getElementsByTagName("description").item(0);
      if (!node)
      {
        node = xmlDoc.createElement("description");
        var cdata = xmlDoc.createCDATASection("");
        node.appendChild(cdata);
        node = xmlDoc.documentElement.appendChild(node);
      }
      if (node)
      {
        node = node.firstChild;
        while (node && node.nodeType != node.CDATA_SECTION_NODE) node = node.nextSibling;
        if (node && node.textContent != desc)
        {
          node.textContent = desc;
          // write the file again
          var ser = new XMLSerializer();
          var str = ser.serializeToString(xmlDoc);


          var fos = Components.classes["@mozilla.org/network/file-output-stream;1"].createInstance(Components.interfaces.nsIFileOutputStream);
          fos.init(outfile, -1, -1, false);
          var os = Components.classes["@mozilla.org/intl/converter-output-stream;1"]
            .createInstance(Components.interfaces.nsIConverterOutputStream);
          os.init(fos, "UTF-8", 4096, "?".charCodeAt(0));
          os.writeString(str);
          os.close();
          fos.close();
        }
      }
    }
  }
  catch (e)
  {
    dump("Error is fragments.js, writeDescriptionOfItem: "+e.message+"\n");
  }
}

function showDescription(event, tooltip)
{
  var tree = document.getElementById("frag-tree");
  tooltip.setAttribute("label","");
  var row = {};
  var col = {};
  var obj = {};
  var b = tree.treeBoxObject;
  b.getCellAt(event.clientX, event.clientY, row, col, obj);
//  if (!row.value) return false;
  var description = descriptionOfItem(row.value);
  tooltip.setAttribute("label",description);
  return description.length > 0;
}

function insertFragmentContents( pathname)
{
  // pathname is now the path of the clicked file relative to the fragment root.
  dump("insertFragmentContents: pathname = "+pathname+"\n");
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
    alert(e);
  }
  focusOnEditor();
}

function loadFragment(event,tree)
{
  var namecol = tree.columns.getNamedColumn('Name');
  var i = tree.currentIndex;
  var s = tree.view.getCellText( i,namecol);
  if (event.type=="keypress" && event.keyCode!=event.DOM_VK_RETURN) return;
    while (tree.view.getParentIndex(i) >= 0)
    {           
      i = tree.view.getParentIndex(i);
      s = tree.view.getCellText(i,namecol)+ pathSeparator + s;
    }
  s = tree.getAttribute("ref") + "/" +s;
  insertFragmentContents(s);
  focusOnEditor();
}

function deleteFragment(tree)
{
  var tree = document.getElementById("frag-tree");
  var namecol = tree.columns.getNamedColumn('Name');
  var i = tree.currentIndex;
  if (i < 0) return;
  var s = tree.view.getCellText( i,namecol);
  var file = getUserResourceFile("fragments","");
    while (tree.view.getParentIndex(i) >= 0)
    {           
      i = tree.view.getParentIndex(i);
      s = tree.view.getCellText(i,namecol)+pathSeparator+s;
    }
  var pieces = s.split(pathSeparator);
  var j;
  for (j = 0; j < pieces.length; j++) file.append(pieces[j]);
  dump('found file  '+file.path+'\n');
  try {
    file.remove(false);
  }
  catch (e) {
    dump(e.message+'\n');
  }
  refresh(tree);
}

function renameFragment(tree)
{
  var tree = document.getElementById("frag-tree");
  var namecol = tree.columns.getNamedColumn('Name');
  var i = tree.currentIndex;
  if (i < 0) return;
  var row = i;
  var s = tree.view.getCellText( i,namecol);
  var data = new Object();
  var savedata = new Object();
  savedata.filename = s;
  data.filename = s;
  data.role = "renamefrag";
  savedata.description = data.description = descriptionOfItem(row);
  dump('found item '+s+'\n');
  window.openDialog("chrome://prince/content/fragmentname.xul", "", "modal,chrome,resizable=yes", data);
  var nameChanged = ((data.filename.length > 0) && (savedata.filename != data.filename));
  var descChanged = (data.description != savedata.description);
  if (!nameChanged && !descChanged) return;
  if (descChanged) writeDescriptionOfItem(row, data.description);
  var file = getUserResourceFile("fragments","");
  var newname = data.filename;
  var oldname = s;
  if (nameChanged)
  {
    dump('new name is '+data.filename+'\n');
    var regexp = /\.frg$/i;
    newname = data.filename;
    if (!regexp.test(newname)) newname += '.frg';
  }
  while (tree.view.getParentIndex(i) >= 0)
  {           
    i = tree.view.getParentIndex(i);
    s = tree.view.getCellText(i,namecol)+pathSeparator+s;
  }
  var pieces = s.split(pathSeparator);
  var j;
  for (j = 0; j < pieces.length; j++) file.append(pieces[j]);
  if (nameChanged) {
    file.moveTo(null, newname);
    refresh(tree);
  }
}

function fixFragmentContextMenu()
{
  var tree = document.getElementById("frag-tree");
  var namecol = tree.columns.getNamedColumn('Name');
  var i = tree.currentIndex;
  if (i < 0) return;
  var s = tree.view.getCellText( i,namecol);
  var menu = document.getElementById("fragment_delete");
  menu.setAttribute("label", menu.getAttribute("label")+" "+s);
  var menu = document.getElementById("fragment_rename");
  menu.setAttribute("label", menu.getAttribute("label")+" "+s);
}

function restoreFragmentContextMenu()
{
  var menu = document.getElementById("fragment_delete");
  menu.setAttribute("label", menu.getAttribute("barelabel"));
  var menu = document.getElementById("fragment_rename");
  menu.setAttribute("label", menu.getAttribute("barelabel"));
}

function newFragmentFolder()
{
  var tree = document.getElementById("frag-tree");
  var namecol = tree.columns.getNamedColumn('Name');
  var i = tree.currentIndex;
  var s = "";
  if (i < 0) return;
  if (tree.view.isContainer(i))
    s = tree.view.getCellText( i,namecol);
  var file = getUserResourceFile("fragments","");
  while (tree.view.getParentIndex(i) >= 0)
  {           
    i = tree.view.getParentIndex(i);
    s = tree.view.getCellText(i,namecol)+pathSeparator+s;
  }
  var pieces = s.split(pathSeparator);
  var j;
  for (j = 0; j < pieces.length; j++) file.append(pieces[j]);
  var data = new Object();
  data.role = "newfolder";
  window.openDialog("chrome://prince/content/fragmentname.xul", "", "modal,chrome,resizable=yes", data);
  if (data.filename.length > 0)
  {
    dump('new name is '+data.filename+'\n');
    file.append(data.filename);
    try {
      file.create(1, 0755);
    }
    catch(e) {
      dump(e.message+'\n');
    }
    refresh(tree);
  }
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
  dump('onMacroOrFragmentEntered: '+aString+'\n');
  var s = getMacro(aString);
  if (s) 
  {
    insertDataAtCursor( s );
  }
  else
  {
    dump(aString+' is not a macro\n');
    s = getFragment(aString);
    dump(s+'\n');
    if (s)
    {
      var dirpath = s.dirpath;
#ifdef XP_WIN32
      dirpath = dirpath.replace("\\","/","g");
#endif
      dirpath = msiFileURLFromAbsolutePath(  dirpath ) + "/" + aString + ".frg";      
      dump('calling insertFragmentContents('+dirpath+'\n');
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
  canHandleMultipleItems: function ()
  {
    return true;
  },
  
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
      var path = tree.getAttribute("ref") + s;
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
  
  canDrop: function(evt, session)
  {
    return true;
  },
  
  onDrop: function(evt, dropData, session)
  {
    var tree = evt.currentTarget;
    var bo = tree.treeBoxObject;
    var namecol = tree.columns.getNamedColumn('Name');
    var saveref = tree.getAttribute("ref");
    var pathbase = saveref + "/";
#ifdef XP_WIN32
    pathbase = pathbase.substr(8); // omit "path:///" at the start
#else
    pathbase = pathbase.substr(7); // omit "path:///" at the start
#endif
    var path="";
    var row = new Object;
    var column = new Object;
    var part = new Object;
    bo.getCellAt(evt.clientX, evt.clientY, row, column, part);
    var i = row.value;
    if (i >= 0)
    {
      if (tree.view.isContainer(i)) 
        path = tree.view.getCellText(i,namecol);
      while (tree.view.getParentIndex(i) >= 0)
      {           
        i = tree.view.getParentIndex(i);
        path = tree.view.getCellText(i,namecol)+ "/" + path;
      }
    }
//    dump("New fragment file path is "+pathbase + path + "\n");
    if (session.isDataFlavorSupported("privatefragmentfile"))
    {
      var origPath = dropData.first.first.data.substr(8);  //This HAS to be fixed BBM:
#ifdef XP_WIN32
      origPath = origPath.replace("/","\\","g");
#endif
      // now move origPath to path
      var file = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
      file.QueryInterface(Components.interfaces.nsIFile);
      file.initWithPath( origPath );
      var dir = getUserResourceFile("fragments","");
      var pieces = path.split("/");
      var j;
      for (j = 0; j < pieces.length; j++) if (pieces[j].length > 0) dir.append(pieces[j]);
      
      file.moveTo(dir, "");
      refresh(tree);
    }
    else if (session.isDataFlavorSupported("text/html"))
    {
      var data = new Object();
      data.role = "newfrag";
      data.description = "Enter a short description of what the fragment does."
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
          ']]>\n  </info>\n  <description>\n    <![CDATA[' + data.description +
          ']]>\n  </description>\n</fragment>';
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
        refresh(tree);
      }
    }
  },
  
  onDragOver: function(evt, flavour, session) 
  {
    dump(flavour.contentType+"\n");
    var supported = session.isDataFlavorSupported("privatefragmentfile");
    if (!supported)
      supported = session.isDataFlavorSupported("text/html");

    if (supported)
      session.canDrop = true;
  },
  
  getSupportedFlavours: function()
  {
    var flavours = new FlavourSet();
    flavours.appendFlavour("privatefragmentfile");
    flavours.appendFlavour("text/html");
    return flavours;
  }
}  
