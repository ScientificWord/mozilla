
function saveforweb( doc, usedirectory, dir )
{
  try {
    var doc2 = document.implementation.createDocument ("", "", null);
    var node;
    node = doc2.importNode(doc.documentElement,true);
    doc2.appendChild(node);
    var cssdir = dir.clone();
    var xbldir = dir.clone();
    cssdir.append('css');
    xbldir.append('xbl');
    if (!cssdir.exists()) cssdir.create(1, 0755);
    if (!xbldir.exists()) xbldir.create(1, 0755);
    var SSs = doc.styleSheets; 
    var i;
    var pi;
    var href;
    var hrefModified, newHRef, oldHRef;
    var root = doc2.getElementsByTagName('html')[0];
    var re = new RegExp("resource://app/res/","");
    for (i = 0; i <SSs.length; i++)
    {
      oldHRef = SSs[i].href;
      if (re.test(oldHRef)) 
      {
        hrefModified = true;
        newHRef = oldHRef.replace(re,"");
      }
      else newHRef = oldHRef;
      var pi = doc2.createProcessingInstruction("xml-stylesheet", "href='"+newHRef+"'", "type='text/css'");
      root.parentNode.insertBefore(pi,root);
      handleStyleSheet(oldHRef, newHRef, dir );  
    }
    // now save doc2. Use the current base of the .sci name for the xhtml file generated.
    // Subdocuments are copied over with changes.  
    var serializer = new XMLSerializer();
    var prettyString = serializer.serializeToString(doc2);
    // find the name of the starting .sci file. Start with the working directory.
    var docurlstring = doc.URL;
    var url = msiURIFromString(docurlstring);
    var srcfile = msiFileFromFileURL(url);
    var srcdir = srcfile.parent.clone();
    var name = srcdir.leafName.replace("_work","");
    var destfile = dir.clone();
    destfile.append(name+".xhtml");
    var fos = Components.classes["@mozilla.org/network/file-output-stream;1"].createInstance(Components.interfaces.nsIFileOutputStream);
    fos.init(destfile,0x02 | 0x08 | 0x20, 0666, false);
// write, create, truncate
    var os = Components.classes["@mozilla.org/intl/converter-output-stream;1"]
      .createInstance(Components.interfaces.nsIConverterOutputStream);
    os.init(fos, "UTF-8", 4096, "?".charCodeAt(0));
    os.writeString(prettyString);
    os.close();
    // Now copy subdocuments
    var entries = srcdir.directoryEntries;
    var isXml = /\.xml$/i;
    while(entries.hasMoreElements())
    {
      var entry = entries.getNext();
      entry.QueryInterface(Components.interfaces.nsIFile);
      if (entry.isFile && isXml.test(entry.path))
      {
        entry.copyTo(dir,"");
      }
    }
    return true;
  }
  catch(e) {
    dump("Error in saveforweb: "+e.message+"\n");
    return false;
  }
}  

function handleStyleSheet (oldHRef, newHRef, dir)  // modify and copy style sheets and xbl files
{
  // if the parameters are different, copy the style sheet to newHRef (relative to dir)
  // while it is open, look for @import and -moz-binding commands that may need to be changed.
  try {
    var f = dir.clone();
    var arr = newHRef.split("/");
    var i;
    var re = /resource:\/\/app\/res\/([a-zA-Z0-9_\/\.]+)/ig;
    var re2= /^(.*-moz-binding.*)$/mgi;
    for (i = 0; i < arr.length; i++) f.append(arr[i]);
    if (f.exists()) return;
    var req = new XMLHttpRequest();
    var contents;
    var match;  
    req.open('GET', oldHRef, false);   
    req.send(null);  
    if(req.status == 0)
    {  
      match = re.exec(req.responseText);
      if (match)
      {
        handleStyleSheet(match[0],match[1],dir);
      }
      contents = req.responseText.replace("resource://app/res/","","g");
      contents = contents.replace(re2,"/*$1*/","gm");
    }  
    var fos = Components.classes["@mozilla.org/network/file-output-stream;1"].createInstance(Components.interfaces.nsIFileOutputStream);
    fos.init(f, -1, -1, false);
    var os = Components.classes["@mozilla.org/intl/converter-output-stream;1"]
      .createInstance(Components.interfaces.nsIConverterOutputStream);
    os.init(fos, "UTF-8", 4096, "?".charCodeAt(0));
    os.writeString(contents);
    os.close();
    fos.close();
  }
  catch(e)
  {
    dump("in handleStyleSheet(",oldHRef,",",newHRef,"), "+e.message+"\n");
  }
}
