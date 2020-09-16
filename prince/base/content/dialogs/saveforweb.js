"use strict";
/* global Components: true */
/* global msiFileURLStringFromFile: true */
/* global GetPrefs: true */
/* global msiFileFromFileURL: true */

Components.utils.import("resource://app/modules/pathutils.jsm");

function getDocumentDirectoryFor(doc)
{
  var dir = msiFileFromFileURL(doc.documentURIObject);
  return dir.parent;
}

function createCopyOfDocument(doc, srcDir, destDir)
{
  var contents;
  var serializer = new XMLSerializer();
  var prettyString = serializer.serializeToString(doc);
  var parser = new DOMParser();
  var docCopy = parser.parseFromString(prettyString, 'application/xhtml+xml');
// the source directory name is based on the document filename, so...
  var name = srcDir.leafName.replace("_work","");
  if (!destDir.exists()) destDir.create(1, 493); //0755
  var destfile = destDir.clone();
  destfile.append(name+".xhtml");
  writeStringAsFile(prettyString, destfile);
  return docCopy;
}

function createSubDirFromDocDir(dir, name)
{
  var subdir = dir.clone();
  subdir.append(name);
  return subdir;
}

function rebaseStyleSheetHRef( oldHRef, srcDir, fUseWebCss, cssurl )
{
  var reRes = new RegExp("resource://app/res/","i");
  var reDoc = new RegExp(msiFileURLStringFromFile(srcDir),"i");
  if (reRes.test(oldHRef))
    return oldHRef.replace(reRes, fUseWebCss ? cssurl : "");
  if (reDoc.test(oldHRef))
    return oldHRef.replace(reDoc,"");
  return oldHRef;
}

function isXML(name) {
  var regexp = /\.xml$/i;
  return regexp.test(name);
}


function saveDocumentCopyAndSubDocs(docCopy, srcDir, destinationDir)
{
  var entries;
  var entry;
  var cssDir = destinationDir.clone();
  cssDir.append('css');
  if (!cssDir.exists())
  {
    cssDir.create(1, 493); // = 0755
  }
// Now copy subdocuments -- all of these have an xml extension
  entries = srcDir.directoryEntries;
  while ( entries.hasMoreElements() )
  {
    entry = entries.getNext().QueryInterface(Components.interfaces.nsIFile);
    if (entry.isFile() && isXML(entry.path))
    {
      entry.copyTo(destinationDir,"");
    }
    else
    {
      if (entry.isDirectory() && (entry.leafName !== "css" && entry.leafName !== "plots" && entry.leafName !== "tcache" &&
        entry.leafName !== "tex"))
      {
        entry.copyTo(destinationDir,"");
      }
    }
  }
  if (!cssDir.exists())
  {
    cssDir.create(1, 493); // = 0755
  }
}

function insertMathJaxScript(doc,mathjax)
{
  var head = doc.getElementsByTagName('head')[0];
  var script = doc.createElementNS('http://www.w3.org/1999/xhtml','script');
  script.setAttribute("type", "text/javascript");
  script.setAttribute("src", mathjax);
  head.insertBefore(script, head.firstChild);
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
  catch(e) {
    3; // you can put a breakpoint here
  }
  zipfile.append(leafname + "." + extension);

  try {
    var zw = Components.classes["@mozilla.org/zipwriter;1"]
                          .createInstance(Components.interfaces.nsIZipWriter);
    if (zipfile.exists()) zipfile.remove(0);
    zipfile.create(0,493);
    zw.open( zipfile, PR_RDWR | PR_CREATE_FILE | PR_TRUNCATE);
    zipDirectory(zw, "", dir, compression);
    zw.close();
    return true;
  }
  catch(e) {
    throw Components.results.NS_ERROR_UNEXPECTED;
  }
}

// function writeStringAsFile( str, file )
// {
//   var fos = Components.classes["@mozilla.org/network/file-output-stream;1"].createInstance(Components.interfaces.nsIFileOutputStream);
//   fos.init(file, 0x02 | 0x08 | 0x20, 0664, 0);
//   var os = Components.classes["@mozilla.org/intl/converter-output-stream;1"]
//     .createInstance(Components.interfaces.nsIConverterOutputStream);
//   os.init(fos, "UTF-8", 4096, "?".charCodeAt(0));
//   os.writeString(str);
//   os.close();
//   fos.close();
// }

function getFileAsString_local( url, basepath )  // url is the text representation of a URL
{
  if (isRelativePath(url)) url = 'file://' + basepath + '/' + url;
  var req = new XMLHttpRequest();
  req.overrideMimeType("text/css");
  req.open('GET', url, false);
  req.send(null);
  if (req.status === 0)
    return req.responseText;
  else
    return null;
}

function isRelativePath( url ) {  // the text representation of a URL
  return (url.indexOf('://') < 0);
}

function handleStyleSheet (oldHRef, newHRef, srcDir, destDir, depth, fUseWebCss, cssurl)  // modify and copy style sheets and xbl files
{
  // dump("handleStyleSheet:  "+ oldHRef +", "+newHRef + "\n");
  // if the parameters are different, copy the style sheet to newHRef (relative to dir)
  // while it is open, look for @import commands that may need to be changed.
  // -moz-binding commands are not portable, so they are left untouched; these should occur
  // only in XUL style sheets.
  try {
    var newFile;
    var re = /resource:\/\/app\/res\/(css\/[a-zA-Z0-9_\.-]+)/i;
    if (fUseWebCss && re.test(oldHRef)) {
      // BBM -- work required here
      return;
    }

    if (isRelativePath(newHRef))
    {
      newFile = appendRelativePath(destDir, newHRef);  // in pathutils.jsm
      var match;
      var contents = getFileAsString_local( oldHRef, srcDir.path );

      if (contents != null)
      {
        while ((match = re.exec(contents))!= null && (/css$/.test(match[1])))
        {
          handleStyleSheet(match[0], match[1], srcDir, destDir, ++depth, fUseWebCss, cssurl);
          if (fUseWebCss) {
            contents = contents.replace(match[0], cssurl+match[1].replace(/^css\//i,""));
          }
          else {
            contents = contents.replace(match[0],match[1].replace(/^css\//i,""));
          }
        }
      } 

      // sneak in a new rule that hides tex buttons
      if (newHRef == 'css/my.css') {
        contents = contents + '\ntexb { display: none; }';
      }
      writeStringAsFile(contents, newFile);
    }
  }
  catch(e)
  {
   1;// dump("in handleStyleSheet(",oldHRef,",",newHRef,",",depth,"), "+e.message+"\n");
  }
}

function saveforweb( doc, filterindex, destDir )
// Filter index is 0 for zip file containing css
//                 1 for zip file without css but refs to our domain or the one in the preferences
//                 2 like 0 but with MathJax
//                 3 like 1 but with MathJax
{

  try {
    // Preferences are swp.savetoweb.cssurl (defaults to www.mackichan/supportfiles/6.0/css)
    // and swp.savetoweb.mathjaxsrc (defaults to ")
    var prefs = GetPrefs();
    var mathjax;
    var fUseMathJax = (filterindex >= 2);
    if (fUseMathJax)
      mathjax = prefs.getCharPref("swp.savetoweb.mathjaxsrc");
    var cssurl;
    var fUseWebCss = (filterindex === 1 || filterindex === 3);
    if (fUseWebCss) {
      cssurl = prefs.getCharPref("swp.savetoweb.cssurl");
      if (!cssurl) {
        cssurl = "https://www.mackichan.com/supportfiles/6.0/css/"
      }
    }
    if (!destDir.exists())
    {
      destDir.create(1, 493); // = 0755
    }

    var srcDir = getDocumentDirectoryFor(doc);
    var destDoc = createCopyOfDocument(doc, srcDir, 
      destDir);
//    var cssdir = createSubDirFromDocDir(dir, "css"); // there is always, at least, the 'my.css' file
//    var xbldir = createSubDirFromDocDir(dir, "xbl");
//    var graphicsdir = createSubDirFromDocDir(dir, "graphics");
    var root = destDoc.getElementsByTagName('html')[0];

    if (fUseMathJax)
      insertMathJaxScript(destDoc,mathjax);

// Now handle the CSS files. There is at least one (my.css) referenced in the xhtml file, but each 
// css file can include others with the @import directive. These files can live in several places:
// 1. the css directory in the document,
// 2. in the resource area of the code, and accessed with the resource:// URI,
// 3. elsewhere on the web, accessed with http:// or https:// URIs,
// 4. elsewhere in the file system (this cannot occur with our user interface)

// The my.css file is always copied to the destination zip file. The resource:// files are copied or
// not, depending on the user's choice. If the filterindex is 0 or 2, they are copied to the css directory,
// and in all cases the links are changed to point to the css directory, or corresponding files at 
// the mackichan.com website, or to the corresponding files at the location given by the
// swp.savetoweb.cssurl preference.

// saveDocumentCopyAndSubDocs copies the various document files, but not main.xhtml since we
// need to change parts of it.
    saveDocumentCopyAndSubDocs(destDoc, srcDir, destDir);

    var SSs = destDoc.childNodes;  // SSs is list of top level nodes (siblings of <html>)
    var length = SSs.length;
    var newHRef, oldHRef;
    var match;
    for (var i = 0; i < length; i++)
    {
      if (SSs[i].nodeName !== 'xml-stylesheet') break;
      match = /href\s*=\s*["']([/\\-_a-zA-ZS0..9]+)['"]/.exec(SSs[i].data);
      if (match && match.length >= 2) {
        oldHRef = match[1];
        // oldHRef = appendRelativePath(srcDir,oldHRef).path;
        newHRef = rebaseStyleSheetHRef(oldHRef, srcDir, fUseWebCss, cssurl);
        var pi = destDoc.createProcessingInstruction("xml-stylesheet", "href='"+newHRef+"'", "type='text/css'");
        destDoc.removeChild(SSs[i]);
        root.parentNode.insertBefore(pi,root);
        handleStyleSheet(oldHRef, newHRef, srcDir, destDir, 0, fUseWebCss, cssurl );
      }
    }
    var serializer = new XMLSerializer();
    var prettyString = serializer.serializeToString(destDoc);
    // the source directory name is based on the document filename, so...
    var name = srcDir.leafName.replace("_work","");
    if (!destDir.exists()) destDir.create(1, 493); //0755
    var destfile = destDir.clone();
    destfile.append(name+".xhtml");
    writeStringAsFile(prettyString, destfile);
    zipUpDir(destDir,"zip");
    destDir.remove(true);
    return true;
  }
  catch(e) {
    return false;
  }
}

