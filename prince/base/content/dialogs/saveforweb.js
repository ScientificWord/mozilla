Components.utils.import("resource://app/modules/pathutils.jsm");

function getDocumentDirectoryFor(doc)
{
  var dir = msiFileFromFileURL(doc.documentURIObject);
  return dir.parent;
}

function createCopyOfDocument(doc)
{
  var doc2 = doc.implementation.createDocument("","",null);
  var newDocElement = doc2.importNode(doc.documentElement,true);
  doc2.appendChild(newDocElement);
  return doc2;
}

function createSubDirFromDocDir(dir, name)
{
  var subdir = dir.clone();
  subdir.append(name);
  if (!subdir.exists())
  {
    subdir.create(1, 0755);
  }
}

function rebaseStyleSheetHRef( sshref, documentDirectory )
{
  var reRes = new RegExp("resource://app/res/","i");
  var reDoc = new RegExp(msiFileURLStringFromFile(documentDirectory),"i");
  if (reRes.test(sshref))
    return sshref.replace(reRes,"");
  if (reDoc.test(sshref))
    return sshref.replace(reDoc,"");
  return sshref;  
}


function saveDocumentCopyAndSubDocs(docCopy, srcDir, destinationDir)
{
  var serializer = new XMLSerializer();
  var prettyString = serializer.serializeToString(docCopy);
// the source directory name is based on the document filename, so...
  var name = srcDir.leafName.replace("_work","");
  var destfile = destinationDir.clone();
  destfile.append(name+".xhtml");
  writeStringAsFile(prettyString, destfile);
// Now copy subdocuments -- all of these have an xml extension
  var entries = srcDir.directoryEntries;
  var isXml = /\.xml$/i;
  while(entries.hasMoreElements())
  {
    var entry = entries.getNext();
    entry.QueryInterface(Components.interfaces.nsIFile);
    if (entry.isFile && isXml.test(entry.path))
    {
      entry.copyTo(destinationDir,"");
    }
    else
    {
      if (entry.isDirectory && (entry.leafName == "graphics" || false /* BBM */))
      {
        entry.copyTo(destinationDir,"");
      }
    }
  }
}

function saveforweb( doc, usedirectory, dir )
{
  try {
    var dir1 = getDocumentDirectoryFor(doc);
    var doc2 = createCopyOfDocument(doc);
    var cssdir = createSubDirFromDocDir(dir1, "css");
    var xbldir = createSubDirFromDocDir(dir1, "xbl");
    var graphicsidr = createSubDirFromDocDir(dir1, "graphics");
    var SSs = doc.styleSheets; 
    var newHRef, oldHRef;
    var root = doc2.getElementsByTagName('html')[0];
    for (var i = 0; i <SSs.length; i++)
    {
      oldHRef = SSs[i].href;
      newHRef = rebaseStyleSheetHRef(oldHRef, dir1);
      var pi = doc2.createProcessingInstruction("xml-stylesheet", "href='"+newHRef+"'", "type='text/css'");
      root.parentNode.insertBefore(pi,root);
      if (oldHRef != newHRef) 
      {
        handleStyleSheet(oldHRef, newHRef, dir, 0 );  
      }
    }
    saveDocumentCopyAndSubDocs(doc2, dir1, dir);
    if (!usedirectory)
    {
      zipUpDir(dir,"zip");
      dir.remove(true);
      return true;
    }
    return true;
  }
  catch(e) {
    dump("Error in saveforweb: "+e.message+"\n");
    return false;
  }
}  

function zipUpDir(dir, extension)
{
  var zipfile = dir.parent.clone();
  var leafname = dir.leafName;  
  var compression = 0;
  var prefs = Components.classes["@mozilla.org/preferences-service;1"].getService(Components.interfaces.nsIPrefBranch);
  try
  {
    compression = prefs.getIntPref("swp.webzip.compression");
  }
  catch(e) {}
  
  zipfile.append(leafname + "." + extension); 

  try {
    var zw = Components.classes["@mozilla.org/zipwriter;1"]
                          .createInstance(Components.interfaces.nsIZipWriter);
    if (zipfile.exists()) zipfile.remove(0);
    zipfile.create(0,0755);
    zw.open( zipfile, PR_RDWR | PR_CREATE_FILE | PR_TRUNCATE);
    zipDirectory(zw, "", dir, compression); 
    zw.close();
    return true;
  }
  catch(e) {
    throw Components.results.NS_ERROR_UNEXPECTED;
  }
}


function appendRelativePath(dir, relPath)
// like nsifile append but the relPath can contain many directories
{
   var f = dir.clone();
   var arr = relPath.split("/");
   for  (var i = 0; i < arr.length; i++)
   {
     f.append(arr[i]);
     if (!f.exists())
     {
       if (i < arr.length - 1)
         f.create(1, 0755);
     }
   }
   return f;
}

function writeStringAsFile( str, file )
{
  var fos = Components.classes["@mozilla.org/network/file-output-stream;1"].createInstance(Components.interfaces.nsIFileOutputStream);
  fos.init(file, -1, -1, false);
  var os = Components.classes["@mozilla.org/intl/converter-output-stream;1"]
    .createInstance(Components.interfaces.nsIConverterOutputStream);
  os.init(fos, "UTF-8", 4096, "?".charCodeAt(0));
  os.writeString(str);
  os.close();
  fos.close();
}

function getFileAsString( url )
{
  var req = new XMLHttpRequest();
  req.overrideMimeType("text/css");
  req.open('GET', url, false);   
  req.send(null);  
  if (req.status == 0)
    return req.responseText;
  else
    return null;
}

function handleStyleSheet (oldHRef, newHRef, dir, depth)  // modify and copy style sheets and xbl files
{
  dump("handleStyleSheet:  "+ oldHRef +", "+newHRef + "\n");
  // if the parameters are different, copy the style sheet to newHRef (relative to dir)
  // while it is open, look for @import and -moz-binding commands that may need to be changed.
  try {
    var f = appendRelativePath(dir, newHRef);
//    if (f.exists()) return;
    var re = /resource:\/\/app\/res\/([a-zA-Z0-9_\/\.\-]+)/i;
//    var re2= /^(.*-moz-binding.*)$/mgi;

    var match;  
    var contents = getFileAsString( oldHRef );

    if (contents != null)
    {  
      while ((match = re.exec(contents))!= null)
      {
        handleStyleSheet(match[0],match[1],dir, ++depth);
        contents = contents.replace(match[0],match[1].replace(/^css\//i,""));
      }
      //contents = contents.replace(re2,"/*$1*/","gm");
    } 
    writeStringAsFile(contents, f);
  }
  catch(e)
  {
    dump("in handleStyleSheet(",oldHRef,",",newHRef,",",depth,"), "+e.message+"\n");
  }
}
