var EXPORTED_SYMBOLS = ["msiPathFromFileURL", "msiFileURLFromAbsolutePath",
	"msiFileURLFromChromeURI", "msiFileURLFromFile", "msiURIFromString",
	"msiFileURLStringFromFile", "msiPathFromFileURL" , "msiFileFromFileURL", "msiFileFromAbsolutePath"];

// msiFileURLFromAbsolutePath
// Takes an absolute path (the direction of the slashes is OS-dependent) and
// produces a file URL

function msiFileFromAbsolutePath( absPath ) {
  try {
    var file = Components.classes["@mozilla.org/file/local;1"].
                         createInstance(Components.interfaces.nsILocalFile);
    file.initWithPath( absPath );
    return file;
  }
  catch (e)
  {
    dump("//// error in msiFileFromAbsolutePath: "+e.message+"\n");
  }
}

function msiFileURLFromAbsolutePath( absPath )
{
  try {
    return msiFileURLFromFile( msiFileFromAbsolutePath( absPath ));
  }
  catch (e)
  {
    dump("//// error in msiFileURLFromAbsolutePath: "+e.message+"\n");
  }
}

function msiFileURLFromChromeURI( chromePath )  //chromePath is a nsURI
{
  var retPath;
  if (!chromePath || !(/^chrome:/.test(chromePath)))
  {
    dump("In msiFileURLFromChrome, path [" + chromePath + "] isn't a chrome URL! Returning null.\n");
    return retPath;
  }

  var ios = Components.classes['@mozilla.org/network/io-service;1'].getService(Components.interfaces.nsIIOService);
  var uri = ios.newURI(chromePath, "UTF-8", null);
  var cr = Components.classes['@mozilla.org/chrome/chrome-registry;1'].getService(Components.interfaces.nsIChromeRegistry);
  retPath = cr.convertChromeURL(uri);
  var pathStr = retPath.spec;

  if (/^jar:/.test(pathStr))
    pathStr = pathStr.substr(4);  //after the "jar:"
  if (!(/^file:/.test(pathStr)))
    pathStr = "file://" + pathStr;
  if (pathStr != retPath.spec)
    retPath = msiURIFromString(pathStr);
//  retPath = msiFileURLFromAbsolutePath(retPath.path);

  return retPath;
}

function msiFileURLFromFile( file )
{
  // file is nsIFile
  var ios = Components.classes["@mozilla.org/network/io-service;1"].
                      getService(Components.interfaces.nsIIOService);
  return ios.newFileURI(file);
}

function msiURIFromString(str)
{
  var ios = Components.classes["@mozilla.org/network/io-service;1"].
                      getService(Components.interfaces.nsIIOService);
  return ios.newURI(str, null, null);
}

function msiFileURLStringFromFile( file )
{
  return  msiFileURLFromFile( file ).spec;
}

function msiFileFromFileURL(url)
{
  try {
    // check for a possible chrome URL
    if (/^chrome:/.test(url.spec)) {
      return msiFileURLFromChromeURI(url.spec).QueryInterface(Components.interfaces.nsIFileURL).file;
    }
    return url.QueryInterface(Components.interfaces.nsIFileURL).file;
  }
  catch (e)
  {
    dump("Error in msiFileFromFileURL: url = "+url.spec+" "+e.message+"\n");
  }
}

function msiPathFromFileURL( url )     // redundant BBM: remove instances of this or of GetFilePath
{
  //return GetFilepath( url );
  // or
  return decodeURI(msiFileFromFileURL(url).path);
}

var extensionRE = /\.([^\.]+)$/;

function getExtension(aFilename)
{
  var extArray = extensionRE.exec(aFilename);
  var extension = "";
  if (extArray && extArray[1])
    extension = extArray[1];
  return extension;
}
