// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

//const mmlns    = "http://www.w3.org/1998/Math/MathML";
//const xhtmlns  = "http://www.w3.org/1999/xhtml";

//var data;

//This function assumes an array of package objects - each of which should contain a string "packageName"
//and a comma-separated list of package options "packageOption".
function Package()
{
  this.packageName = "";
  this.packageOptions = new Array();
  this.addOptions = function(optionArray)
    {
      for (var i = 0; i < optionArray.length; ++i)
      {
        if (findOption(optionArray[i]) < 0)
          this.packageOptions.push(optionArray[i]);
      }
    };
  this.setOptions = function(optionArray)
    {
      this.opt = optionArray;
    };
  this.getOptionsStr = function()
    {
      return this.packageOptions.join(",");
    };
  this.setOptionsFromStr = function(optionStr)
    {
      if (optionStr && optionStr.length > 0)
        this.setOptions(optionStr.split(","));
      else if (this.opt.length)
        this.opt = new Array();
    };
  this.findOption = function(theOption)
    {
      var retVal = -1;
      for (var i = 0; (retVal < 0) && (i < this.packageOptions.length); ++i)
      {
        if (this.packageOptions[i] == theOption)
          retVal = i;
      }
      return retVal;
    };
}

function packageListToString(packageList)
{
  var packagesStr = "";
  for (var i = 0; i < packageList.length; ++i)
  {
    if (i > 0)
      packagesStr += "\n";
    if (packageList[i].opt.length > 0)
      packagesStr += "[" + packageList[i].opt.join() + "]";
    packagesStr += "{" + packageList[i].pkg + "}";
  }
  return packagesStr;
}

function packageAndOptionsStringToPackageList(packagesStr)
{
  var packageList = new Array();
  if (packagesStr.length > 0)
  {
    var theStrings = packagesStr.split("\n");
    for (var i = 0; i < theStrings.length; ++i)
    {
      packageList[i] = new Package();
      var regExp = /(\[([^\]]*)\])?\{([^\}]+)\}/;
      var optionsAndName = regExp.exec(theStrings[i]);
      if (optionsAndName[2] && optionsAndName[2].length > 0)
      {
        packageList[i].setOptionsFromStr(optionsAndName[2]);
      }
      packageList[i].pkg = optionsAndName[3];
    }
  }
  return packageList;
}

function findPackageByName(packageList, packageName)
{
  var theIndex = -1;
  for (var i = 0; i < packageList.length; ++i)
  {
    if (packageList[i].pkg == packageName)
    {
      theIndex = i;
      break;
    }
  }
  return theIndex;
}

function sortFileEntries(firstEntry, secondEntry)
{
  var lcFirst = firstEntry.fileName.toLowerCase();
  var slashPos = lcFirst.lastIndexOf('/');
  var lcShortFirst = lcFirst;
  if (slashPos >= 0)
    lcShortFirst = lcFirst.substring(slashPos+1);
  var lcSecond = secondEntry.fileName.toLowerCase();
  var lcShortSecond = lcSecond;
  slashPos = lcSecond.lastIndexOf('/');
  if (slashPos >= 0)
    lcShortSecond = lcSecond.substring(slashPos+1);
  if (lcShortFirst < lcShortSecond)
    return -1;
  else if (lcShortFirst == lcShortSecond)
  {
    if (lcFirst < lcSecond)
      return -1;
    else if (lcFirst == lcSecond)
      return 0;
    else
      return 1;
  }
  else
    return 1;
}

function addDirToSortedFilesList(theList, theDir, extensionList, bRecursive)
{
  var enumer = theDir.directoryEntries;
  while (enumer.hasMoreElements())
  {
    var theEntry = enumer.getNext();
    theEntry = theEntry.QueryInterface(Components.interfaces.nsILocalFile);
    if (theEntry.isDirectory() && bRecursive)
      addDirToSortedFilesList(theList, theEntry, extensionList, true);
    else
    {
      var fileName  = theEntry.leafName;
      if (extensionList.length > 0)
      {
        var dotIndex  = fileName.lastIndexOf('.');
        if (dotIndex >= 0 && findInArray(extensionList, fileName.substring(dotIndex+1)) >= 0)
          fileName = fileName.substring(0, dotIndex);
        else
          fileName = "";
      }
      if (fileName.length > 0)
      {
        var nFoundAt = findItemInFilesList(theList, fileName);
        if (nFoundAt >= 0)
        {
          fileName = theEntry.parent.leafName + "/" + fileName;
          var oldPath = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
          oldPath.initWithPath(theList[nFoundAt].filePath);
          theList[nFoundAt].fileName = oldPath.parent.leafName + "/" + theList[nFoundAt].fileName;
        }
        var newEntry = new Object();
        newEntry.fileName = fileName;
        newEntry.filePath = theEntry.path;
        theList.push(newEntry);
//        addItemToSortedFilesList(theList, fileName, theEntry.path);
      }
    }
  }
}

function findItemInFilesList(theList, theName)
{
  for (var i = 0; i < theList.length; ++i)
  {
    if (theList[i].fileName == theName)
      return i;
  }
  return -1;
}

function addItemToSortedFilesList(theList, theFileEntry)
{
  var nCount = theList.length;
  var lcName = theFileEntry.fileName.toLowerCase();
  var currItem = null;
  for ( var i = 0; i < nCount; ++i)
  {
    currItem = theList[i];
    if (currItem.fileName.toLowerCase() > lcName)
    {
      theList.splice( i, 0, theFileEntry );
      return;
    }
    else if (currItem.fileName.toLowerCase() == lcName)
      return;  //don't want to add duplicate items!
  }
  theList.push(theFileEntry);
}

//Returns a nsiLocalFile
function lookUpBibTeXDirectory()
{
  var bibPath = null;
  var prefs = Components.classes["@mozilla.org/preferences-service;1"].getService(Components.interfaces.nsIPrefBranch);
  try
  {
    bibPath = prefs.getComplexValue("swp.bibtex.dir", Components.interfaces.nsILocalFile);
    if (!bibPath || !bibPath.exists())
    {
      var dsprops = Components.classes["@mozilla.org/file/directory_service;1"].createInstance(Components.interfaces.nsIProperties);
      bibPath = dsprops.get("CurProcD", Components.interfaces.nsIFile);

    }
  } 
  catch(exception) { }

  return bibPath;
}


function stripSurroundingWhitespace(aString)
{
  if (aString.length == 0)
    return aString;
  var regExp = /^(\s*)(\S(?:.*\S)?)(\s*)$/;
  var stringPieces = aString.match(regExp);
  if (!stringPieces || stringPieces.length < 3)
    return "";
  return stringPieces[2];
}

function stripOpeningWhitespace(aString)
{
  if (aString.length == 0)
    return aString;
  var regExp = /^(\s*)(\S.*)$/;
  var stringPieces = aString.match(regExp);
  if (!stringPieces || stringPieces.length < 3)
    return "";
  return stringPieces[2];
}

function bibTeXFilter(keyName, matchExpr)
{
  this.keyName = keyName;
  this.matchExpr = matchExpr;
  this.bExactCase = false;
  this.bRegExp = false;
}

function convertStringToRegExp(stringExp)
{
  var regExp = stringExp.replace(".", "\\.");
  regExp = regExp.replace("\\", "\\\\");
  regExp = regExp.replace("(", "\\(");
  regExp = regExp.replace(")", "\\)");
  regExp = regExp.replace("[", "\\[");
  regExp = regExp.replace("]", "\\]");
  regExp = regExp.replace("?", "\\?");
  regExp = regExp.replace("^", "\\^");
  regExp = regExp.replace("?", "\\?");
  return regExp;
}

var baseBibTeXData = 
  {
    nativeEntryTypes : ["article", "book", "booklet", "inbook", "incollection",
                        "inproceedings", "manual", "mastersthesis", "misc",
                        "phdthesis", "proceedings", "techreport", "unpublished"],
    nativeKeyNames : ["address", "annote", "author", "booktitle", "chapter",
                      "crossref", "edition", "editor", "howpublished", "institution",
                      "journal", "key", "month", "note", "number", "organization",
                      "pages", "publisher", "school", "series", "title", "type",
                      "volume", "year"]
  };

