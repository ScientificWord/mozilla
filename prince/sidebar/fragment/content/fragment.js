Components.utils.import("resource://app/modules/macroArrays.jsm");



function focusOnEditor()
{
  var editWindow = msiGetActiveEditorElement();
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
    var dirspec = msiFileURLFromFile(dir1).spec;
    var tree = document.getElementById("frag-tree");
    dump("Setting 'ref' attribute to "+dirspec+"\n");
    tree.setAttribute("ref", dirspec);
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
  if (row < 0)
  {
    dump("Calling descriptionOfItem with row ="+row+"\n");
    return "";
  }
  var i = row;
  var tree = document.getElementById("frag-tree");
  var namecol = tree.columns.getNamedColumn('Name');
  var s = tree.view.getCellText( i,namecol);
  while (tree.view.getParentIndex(i) >= 0)
    {           
      i = tree.view.getParentIndex(i);
      s = tree.view.getCellText(i,namecol)+ "/" + s;
  }
  s = decodeURIComponent(tree.getAttribute("ref")+s);
  // s is now the path of the clicked file relative to the fragment root.
  dump("showDescription: pathname = "+s+"\n");
  try 
  {
    var request = new XMLHttpRequest();
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
        return decodeURIComponent(node.textContent);
      }
    }
  }
  catch (e)
  {
    dump("Error in fragments.js, showDescription: "+e.message+"\n");
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
    s = tree.view.getCellText(i,namecol)+"/"+s;
  }
  // s is now the path of the clicked file relative to the fragment root.
  var pieces = s.split("/");
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
        node = xmlDoc.documentElement.appendChild(node);
      }
      if (node)
      {
        if (node.textContent != desc)
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
  var row = {};
  var col = {};
  var obj = {};
  var b = tree.treeBoxObject;
  b.getCellAt(event.clientX, event.clientY, row, col, obj);
  if (!row.value) return false;
  if (tree.view.isContainer(row.value)) return false;                                                       
  tooltip.setAttribute("label","");
  var description = descriptionOfItem(row.value);
  tooltip.setAttribute("label",description);
  return description.length > 0;
}

function insertFragmentContents( fileurl )
{
  // fileurl is now the absolute file url of the clicked file relative to the fragment root.
  fileurl.spec = decodeURIComponent(fileurl.spec);
  dump("insertFragmentContents: fileurl.spec = "+fileurl.spec+"\n");
  try 
  {
    var editorElement = msiGetActiveEditorElement();
    var editor = msiGetEditor(editorElement);
    var request = Components.
                  classes["@mozilla.org/xmlextras/xmlhttprequest;1"].
                  createInstance();
    request.QueryInterface(Components.interfaces.nsIXMLHttpRequest);
    dump("insertFragmentContents: "+fileurl.spec+"\n");
    request.open("GET", fileurl.spec, false);
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
        dataString =  decodeURIComponent(node.textContent);
      }
      if (dataString.length == 0) return;
      node = xmlDoc.getElementsByTagName("context").item(0);
      if (node) 
      {
        contextString =  decodeURIComponent(node.textContent);
      }
      node  = xmlDoc.getElementsByTagName("info").item(0);
      if (node)
      {
        infoString =  decodeURIComponent(node.textContent);
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
  if (tree.view.isContainer(i)) return;
  var s = tree.view.getCellText( i,namecol);
  if (event.type=="keypress" && event.keyCode!=event.DOM_VK_RETURN) return;
    while (tree.view.getParentIndex(i) >= 0)
    {           
      i = tree.view.getParentIndex(i);
      s = tree.view.getCellText(i,namecol)+ "/" + s;
    }
  if (!tree.hasAttribute("ref")) return;
  s = tree.getAttribute("ref") +s ;
  var url = msiURIFromString(s);
  insertFragmentContents(url);
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
      s = tree.view.getCellText(i,namecol)+"/"+s;
    }
  var pieces = s.split("/");
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
  var regexp = /\.frg$/i;
  savedata.filename = s;
  data.filename = s;
  if (!regexp.test(s))
    data.role="folder";
  else
    data.role = "frag";
  savedata.description = data.description = descriptionOfItem(row);
  dump('found item '+s+'\n');
  window.openDialog("chrome://prince/content/fragmentname.xul", "fragmentname", "modal,chrome,resizable", data);
  var nameChanged = ((data.filename.length > 0) && (savedata.filename != data.filename));
  var descChanged = (data.description != savedata.description);
  if (!nameChanged && !descChanged) return;
  if (descChanged) writeDescriptionOfItem(row, data.description);
  var file = getUserResourceFile("fragments","");
  var newname = data.filename;
  var oldname = s;
  if (nameChanged && data.role == "frag")
  {
    dump('new name is '+data.filename+'\n');
    newname = data.filename;
    if (!regexp.test(newname)) newname += '.frg';
  }
  while (tree.view.getParentIndex(i) >= 0)
  {           
    i = tree.view.getParentIndex(i);
    s = tree.view.getCellText(i,namecol)+"/"+s;
  }
  var pieces = s.split("/");
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
    s = tree.view.getCellText(i,namecol)+"/"+s;
  }
  var pieces = s.split("/");
  var j;
  for (j = 0; j < pieces.length; j++) file.append(pieces[j]);
  var data = new Object();
  data.role = "folder";
  window.openDialog("chrome://prince/content/fragmentname.xul", "fragmentname", "modal,chrome,resizable", data);
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
      var editorElement = msiGetActiveEditorElement();
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
      var dirurlstring = msiFileURLFromAbsolutePath(dirpath).spec + aString + ".frg";      
      dump('calling insertFragmentContents('+dirurlstring+'\n');
      var url = msiURIFromString(dirurlstring);
      insertFragmentContents( url );
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
    if (tree.view.isContainer(i)) return;
    var s = tree.view.getCellText( i,namecol);
    while (tree.view.getParentIndex(i) >= 0)
    {           
      i = tree.view.getParentIndex(i);
      s = tree.view.getCellText(i,namecol)+ "/" + s;
    }
    // s is now the path of the clicked file relative to the fragment root.
    try 
    {
      var request = Components.
                    classes["@mozilla.org/xmlextras/xmlhttprequest;1"].
                    createInstance();
      request.QueryInterface(Components.interfaces.nsIXMLHttpRequest);
      var urlstring = decodeURIComponent(tree.getAttribute("ref") + s);
      var path = msiPathFromFileURL( msiURIFromString(urlstring));
      request.open("GET", urlstring, false);
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
          dataString = decodeURIComponent(node.textContent);
        }
        if (dataString.length == 0) return;  // no point in going on in this case
        node = null;
        nodelist = xmlDoc.getElementsByTagName("context");
        if (nodelist.length > 0) node = nodelist.item(0);
        if (node)
        {
          contextString =  decodeURIComponent(node.textContent);
        }
        node = null;
        nodelist = xmlDoc.getElementsByTagName("info");
        if (nodelist.length > 0) node = nodelist.item(0);
        if (node)
        {
          infoString =  decodeURIComponent(node.textContent);
        }
        transferData.data = new TransferData();
        transferData.data.addDataForFlavour("privatefragmentfile", path);
        transferData.data.addDataForFlavour("text/html",dataString);
        transferData.data.addDataForFlavour("text/_moz_htmlcontext",contextString);
        transferData.data.addDataForFlavour("text/_moz_htmlinfo",infoString);
      }
    }
    catch(e) {
      dump("Error: "+e.message+"\n");
    }
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
    var saveurlstring = tree.getAttribute("ref");
    var path = "";
    var urlbasestring = tree.getAttribute("ref");
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
//    dump("New fragment file URL is " + urlbasestring + path + "\n");
    if (session.isDataFlavorSupported("privatefragmentfile"))
    {
      var origPath = dropData.first.first.data;
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
      data.role = "frag";
//      data.description = "Enter a short description of what the fragment does."
      window.openDialog("chrome://prince/content/fragmentname.xul", "fragmentname", "modal,chrome,resizable", data);
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
        
        var sFileContent = '<?xml version="1.0"?>\n<fragment>\n  <data>' + encodeURIComponent(mimetypes.kHTMLMime) +
          '</data>\n  <context>' + encodeURIComponent(mimetypes.kHTMLContext) +
          '</context>\n  <info>' + encodeURIComponent(mimetypes.kHTMLInfo) +
          '</info>\n  <description>' +  encodeURIComponent(data.description) +
          '</description>\n</fragment>';
        var urlstring = urlbasestring + path + "/" + data.filename;
        var filepath = msiPathFromFileURL( msiURIFromString(urlstring));        
        if (filepath.search(/.frg/) == -1) filepath += ".frg";
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

function createFragmentFromClip()
{
    var tree = document.getElementById("frag-tree");
    var namecol = tree.columns.getNamedColumn('Name');
    var saveurlstring = tree.getAttribute("ref");
    var path = "";
    var urlbasestring = tree.getAttribute("ref");
    var row = new Object;
    row.value = 0;
    var column = new Object;
    var part = new Object;
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
//    dump("New fragment file URL is " + urlbasestring + path + "\n");
    var clip = Components.classes["@mozilla.org/widget/clipboard;1"].
      getService(Components.interfaces.nsIClipboard); 
    if (!clip) return false; 
    var trans = Components.classes["@mozilla.org/widget/transferable;1"].
      createInstance(Components.interfaces.nsITransferable); 
    if (!trans) return false; 
    var data = new Object();
    data.role = "frag";
//    data.description = "Enter a short description of what the fragment does."
    window.openDialog("chrome://prince/content/fragmentname.xul", "fragmentname", "modal,chrome,resizable", data);
    if (data.filename.length > 0)
    {
// Now we collect several data formats
      var mimetypes = new Object();
      mimetypes.kHTMLMime                    = "text/html";
      mimetypes.kHTMLContext                 = "text/_moz_htmlcontext";
      mimetypes.kHTMLInfo                    = "text/_moz_htmlinfo";
      for (var i in mimetypes)
      {
        var flavour = mimetypes[i];
        trans.addDataFlavor(flavour);
        clip.getData(trans,clip.kGlobalClipboard); 
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
      
      var sFileContent = '<?xml version="1.0"?>\n<fragment>\n  <data>' + encodeURIComponent(mimetypes.kHTMLMime) +
        '</data>\n  <context>' + encodeURIComponent(mimetypes.kHTMLContext) +
        '</context>\n  <info>' + encodeURIComponent(mimetypes.kHTMLInfo) +
        '</info>\n  <description>' + encodeURIComponent(data.description) +
        '</description>\n</fragment>';
      var urlstring = urlbasestring + path + "/" + data.filename;
      var filepath = msiPathFromFileURL( msiURIFromString(urlstring));        
      if (filepath.search(/.frg/) == -1) filepath += ".frg";
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