// Copyright (c) 2006 MacKichan Software, Inc.  All Rights Reserved.


//const mmlns    = "http://www.w3.org/1998/Math/MathML";
const xhtmlns  = "http://www.w3.org/1999/xhtml";

var data;

//var nativeEntryTypes = ["article", "book", "booklet", "inbook", "incollection",
//                        "inproceedings", "manual", "mastersthesis", "misc",
//                        "phdthesis", "proceedings", "techreport", "unpublished"];
//var nativeKeyNames = ["address", "annote", "author", "booktitle", "chapter",
//                      "crossref", "edition", "editor", "howpublished", "institution",
//                      "journal", "key", "month", "note", "number", "organization",
//                      "pages", "publisher", "school", "series", "title", "type",
//                      "volume", "year"];

var bibTeXSplitExp = new RegExp("(\\\\[^a-zA-Z0-9])|([\$=@%{}\\\"\\[\\]\\(\\),])");

// dialog initialization code
function Startup()
{
  doSetOKCancel(onAccept, onCancel);
  data = window.arguments[0];
  data.Cancel = false;
  gDialog.databaseFile = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
  gDialog.databaseFile.initWithPath(data.databaseFile);
  gDialog.bibDir = data.baseDirectory;  //an nsILocalFile representing BibTeX directory
//  gDialog.bibTeXDir = null; //nsiLocalFile
  gDialog.key = "";
  if (data.key.length)
    gDialog.key = data.key;  //a string
//  gDialog.remark = data.remark;  //this should become arbitrary markup - a Document Fragment perhaps?
//  gDialog.bBibEntryOnly = data.bBibEntryOnly;
  gDialog.entryTypes = baseBibTeXData.nativeEntryTypes;
  gDialog.keyNames = baseBibTeXData.nativeKeyNames;
  gDialog.showEntryTypes = new Array();
  gDialog.showEntryTypes = gDialog.showEntryTypes.concat(gDialog.entryTypes);
  gDialog.displayValues = ["title"];
  gDialog.filters = new Array();
  gDialog.bSortEntriesAscending = true;
  gDialog.sortEntriesOn = "";

  InitDialog();

  document.getElementById("bibTeXEntriesListbox").focus();

  SetWindowLocation();
}

function InitDialog()
{
  gDialog.entriesListboxOriginalLabel = document.getElementById("entriesListboxDescription").getAttribute("value");
  gDialog.matchExpressionOriginalLabel = document.getElementById("matchDescription").getAttribute("value");
  readPrefs();

  readInBibTeXFile(gDialog.databaseFile);

  resetEntriesListbox();
  setDialogTitle();
  setControlLabels();
  checkDisableControls();

//  checkInaccessibleAcceleratorKeys(document.documentElement);

//  gDialog.tabOrderArray = new Array( gDialog.positionSpecGroup,
//                                       document.documentElement.getButton("accept"),
//                                       document.documentElement.getButton("cancel") );
  
  document.documentElement.getButton("accept").setAttribute("default", true);
}

function onAccept()
{
  data.key = gDialog.key;

  var prefs = Components.classes["@mozilla.org/preferences-service;1"].getService(Components.interfaces.nsIPrefBranch);
  var hideEntryTypes = new Array();
  hideEntryTypes = hideEntryTypes.concat(gDialog.entryTypes);
  for (var i = 0; i < gDialog.showEntryTypes.length; ++i)
  {
    var nIndex = findInArray(hideEntryTypes, gDialog.showEntryTypes[i]);
    if (nIndex > 0)
      hideEntryTypes.splice(nIndex, 1);
  }
  var hideTypesString = hideEntryTypes.join(",");
  var displayValuesStr = gDialog.displayValues.join(",");
  try
  {
    prefs.setCharPref("prince.bibtex.hideKeyEntryTypes", hideTypesString);
    prefs.setCharPref("prince.bibtex.displayKeyValues", displayValuesStr);
  }
  catch(exception) {}

  SaveWindowLocation();
  return true;
}

function onCancel()
{
  data.Cancel = true;
}

function doAccept()
{
  document.documentElement.getButton('accept').oncommand();
}

function readPrefs()
{
  var prefs = Components.classes["@mozilla.org/preferences-service;1"].getService(Components.interfaces.nsIPrefBranch);
  var hideTypesString = "";
  var displayKeyValuesStr = "";
  try
  {
    hideTypesString = prefs.getCharPref("prince.bibtex.hideKeyEntryTypes");
    displayKeyValuesStr = prefs.getCharPref("prince.bibtex.displayKeyValues");
  }
  catch(exception) {}
  var hideEntryTypes = hideTypesString.split(",");
  for (var i = 0; i < hideEntryTypes.length; ++i)
  {
    var nIndex = findInArray(gDialog.showEntryTypes, hideEntryTypes[i]);
    if (nIndex > 0)
      gDialog.showEntryTypes.splice(nIndex, 1);
  }
  if (displayKeyValuesStr.length > 0)
    gDialog.displayValues = displayKeyValuesStr.split(",");
}

//function selectDatabaseFile(selectedListboxItem)
//{
//  if (selectedListboxItem)
//    gDialog.databaseFile = selectedListboxItem.label;
//  else
//    gDialog.databaseFile = "";
//  checkDisableControls();
//}

function checkDisableControls()
{
//  var selDBase = document.getElementById("databaseFileListbox").selectedItem;
  if (gDialog.key.length > 0)
  {
    document.getElementById("viewEntryButton").removeAttribute("disabled");
    document.documentElement.getButton('accept').removeAttribute("disabled");
  }
  else
  {
    document.getElementById("viewEntryButton").setAttribute("disabled", "true");
    document.documentElement.getButton('accept').setAttribute("disabled", "true");
  }
}

function setDialogTitle()
{
  var theTitle = document.documentElement.getAttribute("title");
  var strBundle = document.getElementById("titleStrings");
  var replaceFileNameStr = strBundle.getString("FileNameReplaceTemplate");
  var ourFile = gDialog.databaseFile;
  var nameStr = ourFile.parent.leafName;
  var dotIndex  = ourFile.leafName.lastIndexOf('.');
  if (dotIndex >= 0)
    nameStr += "/" + ourFile.leafName.substring(0, dotIndex);
  else
    nameStr += "/" + ourFile.leafName;

  theTitle = theTitle.replace(replaceFileNameStr, nameStr);
  document.documentElement.setAttribute("title", theTitle);
}

function setControlLabels()
{
  var strBundle = document.getElementById("titleStrings");
  
  var listDescription = document.getElementById("entriesListboxDescription");
  var theString = gDialog.entriesListboxOriginalLabel;
  var numItems = document.getElementById("bibTeXEntriesListbox").getElementsByTagName("listitem").length;
  theString = theString.replace(strBundle.getString("NumberKeysReplaceTemplate"), numItems);
  listDescription.setAttribute("value", theString);

  var matchExpressionText = document.getElementById("matchDescription");
  var matchDescriptionStr = "";
  if (gDialog.filters.length > 0)
  {
    var regExpStr = "regular expression";
    var exactCaseStr = "exact case";
    for (var i = 0; i < gDialog.filters.length; ++i)
    {
      matchFieldDescription = gDialog.filters[i].keyName;
      var matchExpressionDescription = gDialog.filters[i].matchExpr;
      var thisString = gDialog.matchExpressionOriginalLabel.replace(strBundle.getString("FieldNameReplaceTemplate"), matchFieldDescription);
      thisString = thisString.replace(strBundle.getString("MatchExpressionReplaceTemplate"), matchExpressionDescription);
      if (gDialog.filters[i].bExactCase || gDialog.filters[i].bRegExp)
      {
        var modifiersArray = new Array();
        if (gDialog.filters[i].bExactCase)
          modifiersArray.push(exactCaseStr);
        if (gDialog.filters[i].bRegExp)
          modifiersArray.push(regExpStr);
        thisString += " (" + modifiersArray.join(",") + ")";
      }
      if (i > 0)
        matchDescriptionStr += "\n" + thisString;
      else
        matchDescriptionStr += thisString;
    }
  }
  else
    matchDescriptionStr = "{no filters}";

  matchExpressionText.setAttribute("value", matchDescriptionStr);
}

function selectDatabaseEntry(selectedItem)
{
  if (selectedItem)
  {
    var keyCell = selectedItem.getElementsByTagName("listcell")[0];
    gDialog.key = keyCell.getAttribute("label");
  }
  else
    gDialog.key = "";
  checkDisableControls();
}

//function checkColAttrModified(event)
//{
//  if (event.attrName == "width" || event.attrName == "left")
//  {
//    var theColumn = event.target;
//    var parent = theColumn.parentNode;
//    while (parent && parent.nodeName != "listbox")
//      parent = parent.parentNode;
//    if (parent)
//      syncListHeaderSizeToCol(parent);
//  }
//}
//
//function syncListHeaderSizeToCol(theListbox)
//{
//  var theCols = theListbox.getElementsByTagName("listcols")[0];
//  var cols = theCols.getElementsByTagName("listcol");
//  var theHead = theListbox.getElementsByTagName("listhead")[0];
//  var headers = theHead.getElementsByTagName("listheader");
//  for (var i = 0; i < headers.length; ++i)
//  {
//    if (!cols[i])
//    {
//      alert("Header column mismatch at index " + i + "!");
//      break;
//    }
//    headers[i].left = cols[i].left;
//    headers[i].width = cols[i].width;
//  }
//}

function resetEntriesListbox()
{
  var theListbox = document.getElementById("bibTeXEntriesListbox");
  //Clear the listbox
  var items = theListbox.getElementsByTagName("listitem");
  for (var i = items.length - 1; i >= 0; --i)
    theListbox.removeChild(items[i]);

  var theCols = theListbox.getElementsByTagName("listcols")[0];
  var cols = theCols.getElementsByTagName("listcol");
  var colSplitters = theCols.getElementsByTagName("splitter");
  var theHead = theListbox.getElementsByTagName("listhead")[0];
  var headers = theHead.getElementsByTagName("listheader");
  for (i = colSplitters.length - 1; i >= 0; --i)
    theCols.removeChild(colSplitters[i]);
  for (i = cols.length - 1; i > 0; --i)  //note: don't remove the 0th one (always want it)
    theCols.removeChild(cols[i]);
  for (i = headers.length - 1; i > 0; --i)
    theHead.removeChild(headers[i]);

  //Prepare the columns in the listbox (the header, and set the number of columns):
//  theCols.setAttribute("orient", "horizontal");
//  theHead.setAttribute("orient", "horizontal");
  for (i = 0; i < gDialog.displayValues.length; ++i)
  {
    var newHeader = document.createElement("listheader");
    newHeader.setAttribute("flex", "2");
    newHeader.setAttribute("label", gDialog.displayValues[i]);
    theHead.appendChild(newHeader);
    var colSplitter = document.createElement("splitter");
    theCols.appendChild( colSplitter );
    var newCol = document.createElement("listcol");
    newCol.setAttribute("flex", "2");
    theCols.appendChild(newCol);
  }
  //Finally, add the filtered entries:
  resetEntriesInEntryListbox();
}

function resetEntriesInEntryListbox()
{
  var theListbox = document.getElementById("bibTeXEntriesListbox");
  //Delete the existing entries (if called from "resetEntriesListbox", these should already be gone, but...
  var items = theListbox.getElementsByTagName("listitem");
  for (var i = items.length - 1; i >= 0; --i)
    theListbox.removeChild(items[i]);

  var nameList = null;
  if (gDialog.sortEntriesOn.length > 0 && gDialog.sortEntriesOn != "entryType")
  {
    if (findInArray(gDialog.displayValues, gDialog.sortEntriesOn) >= 0)
      nameList = new Array();
    else
      gDialog.sortEntriesOn = "";
  }

  var selItem = null;
  for (i = 0; i < gDialog.entryList.entries.length; ++i)
  {
    var theEntry = gDialog.entryList.entries[i];
    if (findInArray(gDialog.showEntryTypes, theEntry.entryType) >= 0)
    {
      if (checkFilters(gDialog.filters, theEntry))
      {
        var newItem = document.createElement("listitem");
        var newCell = document.createElement("listcell");
        newCell.setAttribute("label", theEntry.entryName);
        newItem.appendChild(newCell);
        for (var j = 0; j < gDialog.displayValues.length; ++j)
        {
          newCell = document.createElement("listcell");
          if (theEntry[gDialog.displayValues[j]] != null)
            newCell.setAttribute("label", theEntry[gDialog.displayValues[j]]);
          newItem.appendChild(newCell);
        }
        var insertPos = -1;
        if (nameList)
        {
          if (gDialog.bSortEntriesAscending)
          {
            for (var j = 0; insertPos < 0 && j < nameList.length; ++j)
            {
              if (theEntry[gDialog.sortEntriesOn])
              {
                if (nameList[j]!="" && theEntry[gDialog.sortEntriesOn] < nameList[j])
                  insertPos = j;
                else if (!nameList[j] || nameList[j] == "")
                  insertPos = j;
              }
            }
          }
          else
          {
            for (var j = 0; insertPos < 0 && j < nameList.length; ++j)
            {
              if (theEntry[gDialog.sortEntriesOn])
              {
                if (nameList[j]!="" && theEntry[gDialog.sortEntriesOn] > nameList[j])
                  insertPos = j;
              }
              else if (!nameList[j] || nameList[j] == "")
                insertPos = j;
            }
          }
        }
        if (insertPos >= 0)
        {
          items = theListbox.getElementsByTagName("listitem");
          theListbox.insertBefore(newItem, items[insertPos]);
          if (theEntry.entryName == gDialog.key)
            selItem = theListbox.getElementsByTagName("listitem")[insertPos];
          nameList.splice(insertPos, 0, theEntry[gDialog.sortEntriesOn]);
        }
        else
        {
          if (theEntry.entryName == gDialog.key)
            selItem = theListbox.appendChild(newItem);
          else
            theListbox.appendChild(newItem);
          if (nameList)
            nameList.push(theEntry[gDialog.sortEntriesOn]);
        }
      }
    }
  }
  if (selItem)
    theListbox.selectItem(selItem);
  else
    gDialog.key = "";
}


function checkFilters(filterList, anEntry)
{
  var bOkay = true;
  //Objects in filterList are of type "bibTeXFilter"; see typesetDialogUtils.js
  for (var i = 0; bOkay && i < filterList.length; ++i)
  {
    bOkay = false;
    var targExp = anEntry[filterList[i].keyName];
    var searchExp = null;
    if (targExp == null)
      bOkay = false;
    else
    {
      var matchExp = filterList[i].matchExpr;
      if (!filterList[i].bRegExp)
        matchExp = convertStringToRegExp(matchExp);
      if (!filterList[i].bExactCase)
        searchExp = new RegExp(matchExp, "i");
      else
        searchExp = new RegExp(matchExp);
      bOkay = searchExp.test(targExp);
    }
  }
  return bOkay;
}

function checkClickListHeader(event)
{
  var origTarget = event.originalTarget;
  if (!origTarget || origTarget.nodeName != "listheader")
  {
    origTarget = event.origExplicitTarget;
    if (!origTarget || origTarget.nodeName != "listheader")
      return;
  }
  //If we get here, we've got a listheader. Use it to reset the sort order:
  var newSortKey = origTarget.getAttribute("label");
  if (gDialog.sortEntriesOn == newSortKey)
    gDialog.bSortEntriesAscending = !gDialog.bSortEntriesAscending;
  else
  {
    gDialog.bSortEntriesAscending = true;
    gDialog.sortEntriesOn = newSortKey;
  }
  resetEntriesInEntryListbox();
}

function doViewKeyFilters()
{
  var filtersData = new Object();
  filtersData.entryTypes = new Array();
  filtersData.entryTypes = filtersData.entryTypes.concat(gDialog.entryTypes);
  filtersData.showEntryTypes = new Array();
  filtersData.showEntryTypes = filtersData.showEntryTypes.concat(gDialog.showEntryTypes);
  filtersData.fieldTypes = new Array();
  filtersData.fieldTypes = filtersData.fieldTypes.concat(gDialog.keyNames);
  filtersData.displayFields = new Array();
  filtersData.displayFields = filtersData.displayFields.concat(gDialog.displayValues);
  filtersData.filters = new Array();
  filtersData.filters = filtersData.filters.concat(gDialog.filters);
  window.openDialog("chrome://prince/content/typesetBibTeXFilters.xul", "_blank", "chrome,close,titlebar,modal", filtersData);
  if (!filtersData.Cancel)
  {
    gDialog.showEntryTypes = filtersData.showEntryTypes;
    gDialog.displayValues = filtersData.displayFields;
    gDialog.filters = filtersData.filters;
    resetEntriesListbox();
    setControlLabels();
    checkDisableControls();
  }
}


function doViewEntry()
{
  var entryData = new Object();
  entryData.theData = "";

  var theEntry = getBibItemEntryFromKey(gDialog.key);
  var startToken = theEntry.startLineOffset;
  var endToken = 0;
  for (var i = theEntry.startLine; i <= theEntry.endLine; ++i)
  {
    if (i > theEntry.startLine)
      entryData.theData += "\n";
    if (i == theEntry.endLine)
      endToken = theEntry.endLineOffset;
    else
      endToken = gDialog.theLines[i].tokens.length;
    for (var j = startToken; j < endToken; ++j)
      entryData.theData += gDialog.theLines[i].tokens[j];
  }

  window.openDialog("chrome://prince/content/typesetBibTeXEntry.xul", "_blank", "chrome,close,titlebar,modal", entryData);
  if (!entryData.Cancel)
  {
    alert("ViewEntry dialog should have returned Cancel but didn't.");
  }
}

function getBibItemEntryFromKey(theKey)
{
  var theEntry = null;
  for (var i = 0; i < gDialog.entryList.entries.length; ++i)
  {
    if (gDialog.entryList.entries[i].entryName == theKey)
    {
      theEntry = gDialog.entryList.entries[i];
      break;
    }
  }
  return theEntry;
}

//The LineTokens object will contain: (i) an Array of "tokens" (returned from split()); (ii) the stream offset
//  to the beginning of this line; (iii) the index of the token currently under consideration in the token list.
function LineTokens()
{
  this.tokens = new Array();
  this.nStreamStart = 0;
  this.nCurrToken = -1;
}

function bibItemEntry(entryType)
{
  this.entryType = entryType;
  this.entryName = "";
  this.startLine = 0;
  this.startLineOffset = 0;
  this.endLine = 0;
  this.endLineOffset = 0;
}

function stringEntry()
{
  this.stringKey = "";
  this.stringValue = "";
  this.startLine = 0;
  this.startLineOffset = 0;
  this.endLine = 0;
  this.endLineOffset = 0;
}

function preambleEntry()
{
  this.string = "";
  this.startLine = 0;
  this.startLineOffset = 0;
  this.endLine = 0;
  this.endLineOffset = 0;
}

function EntryList()
{
  this.entries = new Array();
  this.preamble = new Array();
  this.strings = new Array();
}

function readInBibTeXFile(theFile)  //theFile is a nsiLocalFile
{
  gDialog.entryList = new EntryList();
  gDialog.theLines = new Array();
  gDialog.fileAtEOF = false;

  var inputStream = Components.classes["@mozilla.org/network/file-input-stream;1"].createInstance(Components.interfaces.nsIFileInputStream);
  inputStream.init(theFile, -1, -1, false);  //these values for parameters taken from various examples...?
  var bDone = false;
  while (!bDone)
  {
    var newEntry = findNextEntry(inputStream);
    if (!newEntry)
      break;
  }
  inputStream.close();
}

function getOffsetToCurrentToken()
{
  var thisLine = null;
  var whichLine = -1;
  var nTokenOffset = -1;
  if (gDialog.theLines.length > 0)
  {
    whichLine = gDialog.theLines.length - 1;
    thisLine = gDialog.theLines[whichLine];
    if (thisLine.nCurrToken < 0)
    {
      if (whichLine > 0)
      {
        thisLine = gDialog.theLines[--whichLine];
        nTokenOffset = thisLine.length;
      }
      else
      {
        thisLine = null;
        whichLine = -1;
      }
    }
    else
      nTokenOffset = thisLine.nCurrToken;
  }
//  if (thisLine != null && nTokenOffset >= 0)
//  {
//    for (var i = 0; i < nTokenOffset; ++i)
//      nOffset += thisLine.tokens[i].length;
//  }
  return [whichLine, nTokenOffset];
}

function getCurrToken()
{
  var thisLine = null;
  var nTokenOffset = 0;
  if (gDialog.theLines.length > 0)
  {
    thisLine = gDialog.theLines[gDialog.theLines.length - 1];
    nTokenOffset = thisLine.nCurrToken;
    if (nTokenOffset < 0)
    {
      alert("Problem in getCurrToken!");
      if (gDialog.theLines.length > 1)
      {
        thisLine = gDialog.theLines[gDialog.theLines.length - 2];
        nTokenOffset = thisLine.tokens.length - 1;
      }
      else
        thisLine = null;
    }
  }
  if (thisLine && nTokenOffset < thisLine.tokens.length)
    return thisLine.tokens[nTokenOffset];
  return "";
}

function findNextEntry(inputStream)
{
  var nextToken = getNextToken(inputStream);
  var newEntry = null;
  while (nextToken != null && newEntry == null)
  {
    switch(nextToken)
    {
      case "@":
      {
        var currOffset = getOffsetToCurrentToken();
        var entryType = getNextToken(inputStream).toLowerCase();
        switch(entryType)
        {
          case "string":
            newEntry = parseStringEntry(inputStream);
            if (newEntry != null)
            {
              newEntry.startLine = currOffset[0];
              newEntry.startLineOffset = currOffset[1];
              currOffset = getOffsetToCurrentToken();
              newEntry.endLine = currOffset[0];
              newEntry.endLineOffset = currOffset[1] + 1;
              gDialog.entryList.strings.push(newEntry);
            }
          break;
          case "preamble":
            newEntry = parsePreambleEntry(inputStream);
            if (newEntry != null)
            {
              newEntry.startLine = currOffset[0];
              newEntry.startLineOffset = currOffset[1];
              currOffset = getOffsetToCurrentToken();
              newEntry.endLine = currOffset[0];
              newEntry.endLineOffset = currOffset[1] + 1;
              gDialog.entryList.preamble.push(newEntry);
            }
          break;
          case "{":  //anonymous entry - illegal?? parse but don't do anything with it?
            parseAnonymousEntry(inputStream);  //but don't do anything with it?
          break;
          default:
            newEntry = parseNormalEntry(inputStream, entryType);
            if (newEntry != null)
            {
              newEntry.startLine = currOffset[0];
              newEntry.startLineOffset = currOffset[1];
              currOffset = getOffsetToCurrentToken();
              newEntry.endLine = currOffset[0];
              newEntry.endLineOffset = currOffset[1] + getCurrToken().length;
              gDialog.entryList.entries.push(newEntry);
            }
          break;
        }
      }
      break;
      default:
      break;
    }
    if (newEntry != null)
      break;
    nextToken = getNextToken(inputStream);
  }
  return newEntry;
}

//This function breaks up the tokens following the opening '{' of an entry, or following a separating ','.
//It stops when it finds the next separator, or the closing '}'.
function parseField(inputStream)
{
  var bDone = false;
  var currToken = null;
  var bInValue = false;
  var keyString = "";
  var valString = "";
  var currStr = "";
  while (!bDone && (currToken = getNextToken(inputStream)) != null && currToken.length > 0)
  {
    switch(currToken)
    {
      case ",":
      case "":
      case "}":
        bDone = true;
      break;
      case "\"":
        var quotedStr = parseQuotedString(inputStream);
        if (getCurrToken() == "\"")
          currStr += "\"" + quotedStr + "\"";
        else
          bDone = true;
      break;
      case "{":
        var bracedStr = parseBracedString(inputStream, 0);
        if (getCurrToken() == "}")
          currStr += "{" + bracedStr + "}";
        else
          bDone = true;
      break;
      case "=":
        if (!bInValue)
        {
          keyString += currStr;
          currStr = "";
          bInValue = true;
        }
        else
          currStr += currToken;
      break;
      default:
        currStr += currToken;
      break;
    }
  }
  valString = currStr;
  return new Array(keyString, valString);
}


function parseNormalEntry(inputStream, entryType)
{
  var newEntry = null;
  var allDone = false;
  var currToken = null;
  var bFound = false;
  var keyString = "";
  var valueString = "";

  currToken = getNextToken(inputStream);
  if (currToken == "{") //we've hit the start
    bFound = true;
  else
    allDone = true;
//  do 
//  {
//    if (currToken == null)
//      allDone = true;
//    else
//    {
//      switch(currToken)
//      {
//        case "{": //we've hit the start
//          bFound = true;
//        break;
//        //case ",":
//        case "@":
//        case "":
//          allDone = true;
//        break;
//      }
//    }
//  } while (!bFound && !allDone);

  if (bFound)
  {
    newEntry = new bibItemEntry(entryType);
    if (findInArray(gDialog.entryTypes, entryType) < 0)
      gDialog.entryTypes.push(entryType);
  }

  while (bFound && !allDone)
  {
    var nextField = parseField(inputStream);
    if (!nextField[0].length && !nextField[1].length)
    {
      if (gDialog.fileAtEOF)
        allDone = true;
    }
    keyString = stripSurroundingWhitespace(nextField[0]);
    keyString = keyString.toLowerCase();
    valueString = stripSurroundingWhitespace(nextField[1]);
    if (newEntry.entryName == "" && keyString == "")
      newEntry.entryName = valueString;
    else if (keyString.length > 0)
    {
      newEntry[keyString] = valueString;
      if (findInArray(gDialog.keyNames, keyString) < 0)
        gDialog.keyNames.push(keyString);
    }
    if (getCurrToken() == "}")
      allDone = true;
  }

//    do
//    {
//      currToken = getNextToken(currTokens, inputStream);
//      switch(currToken)
//      {
//        case ",":
//          bFound = true;
//        break;
//        case "@":
//        case "":
//        case "}":
//          allDone = true;
//        break;
//        case "\"":
//          var quotedStr = parseQuotedString(currTokens, inputStream);
//          if (quotedStr == "")
//            allDone = true;
//          else
//            entryName += "\"" + quotedStr + "\"";
//        break;
//        case "{":
//          var bracedStr = parseBracedString(currTokens, inputStream);
//          if (bracedStr == "")
//            allDone = true;
//          else
//            entryName += "{" + bracedStr + "}";
//        break;
//        default:
//          entryName += currToken;
//        break;
//      }
//    } while (!bFound && !allDone);
//  }
//
//  if (bFound)
//  {
//    bFound = false;
//    do
//    {
//      currToken = getNextToken(currTokens, inputStream);
//      switch(currToken)
//      {
//        case "=":  //now we'll parse the value of the key
//          bFound = true;
//        break;
//        case ",":
//        case "@":
//        case "":
//        case "}":
//          allDone = true;
//        break;
//        default:
//          keyString += currToken;
//        break;
//      }
//    } while (!bFound && !allDone);
//  }
//  if (bFound)
//  {
//    bFound = false;
//    do
//    {
//      currToken = getNextToken(currTokens, inputStream);
//      switch(currToken)
//      {
//        case "}":
//          bFound = true;
//        break;
//        case "=":  //now we'll parse the value of the key
//        case ",":
//        case "@":
//        case "":
//          allDone = true;
//        break;
//        case "{":
//          valueString = stripOpeningWhitespace(valueString);
//          valueString += parseBracedString(currTokens, inputStream, 1);
//        break;
//        case "\"":
//          valueString = stripOpeningWhitespace(valueString);
//          valueString += parseQuotedString(currTokens, inputStream);
//        break;
//        default:
//          valueString += currToken;
//        break;
//      }
//    } while (!bFound && !allDone);
//  }
//  if (bFound)
//  {
//    newEntry = new stringEntry();
//    newEntry.stringKey = stripSurroundingWhitespace(keyString);
//    newEntry.stringValue = valueString;
//  }
  return newEntry;
}

function parseStringEntry(inputStream, theEntry)
{
  var newEntry = null;
  var allDone = false;
  var currToken = null;
  var bFound = false;
  var keyString = "";
  var valueString = "";

  currToken = getNextToken(inputStream);
  if (currToken == "{")
    bFound = true;
  else
    allDone = true;

//  do 
//  {
//    currToken = getNextToken(inputStream);
//    if (currToken == null)
//      allDone = true;
//    else
//    {
//      switch(currToken)
//      {
//        case "{": //we've hit the start
//          bFound = true;
//        break;
//        //case ",":
//        case "@":
//        case "":
//          allDone = true;
//        break;
//      }
//    }
//  } while (!bFound && !allDone);
  if (bFound)
  {
    newEntry = new stringEntry();
  }
  while (bFound && !allDone)
  {
    var nextField = parseField(inputStream);
    keyString = stripSurroundingWhitespace(nextField[0]);
    valueString = stripSurroundingWhitespace(nextField[1]);
    keyString = keyString.toLowerCase();
    if (keyString != "" && valueString != "")
    {
      newEntry.stringKey = keyString;
      newEntry.stringValue = valueString;
    }
    if (getCurrToken() == "}")
      allDone = true;
  }  
    
//    bFound = false;
//    do
//    {
//      currToken = getNextToken(currTokens, inputStream);
//      switch(currToken)
//      {
//        case "=":  //now we'll parse the value of the key
//          bFound = true;
//        break;
//        case ",":
//        case "@":
//        case "":
//        case "}":
//          allDone = true;
//        break;
//        default:
//          keyString += currToken;
//        break;
//      }
//    } while (!bFound && !allDone);
//  }
//  if (bFound)
//  {
//    bFound = false;
//    do
//    {
//      currToken = getNextToken(currTokens, inputStream);
//      switch(currToken)
//      {
//        case "}":
//          bFound = true;
//        break;
//        case "=":  //now we'll parse the value of the key
//        case ",":
//        case "@":
//        case "":
//          allDone = true;
//        break;
//        case "{":
//          valueString = stripOpeningWhitespace(valueString);
//          valueString += parseBracedString(currTokens, inputStream, 1);
//        break;
//        case "\"":
//          valueString = stripOpeningWhitespace(valueString);
//          valueString += parseQuotedString(currTokens, inputStream);
//        break;
//        default:
//          valueString += currToken;
//        break;
//      }
//    } while (!bFound && !allDone);
//  }
//  if (bFound)
//  {
//    newEntry = new stringEntry();
//    newEntry.stringKey = stripSurroundingWhitespace(keyString);
//    newEntry.stringValue = valueString;
//  }
  return newEntry;
}

function parsePreambleEntry(inputStream, theEntry)
{
  var newEntry = null;
  var allDone = false;
  var currToken = null;
  var bFound = false;
  var valueString = "";

  currToken = getNextToken(inputStream);
  if (currToken == "{")
    bFound = true;
  else
    allDone = true;

//  do 
//  {
//    currToken = getNextToken(inputStream);
//    if (currToken == null)
//      allDone = true;
//    else
//    {
//      switch(currToken)
//      {
//        case "{": //we've hit the start
//          bFound = true;
//        break;
//        //case ",":
//        case "@":
//        case "":
//          allDone = true;
//        break;
//      }
//    }
//  } while (!bFound && !allDone);
  if (bFound)
  {
    newEntry = new preambleEntry();
  }
  while (bFound && !allDone)
  {
    var theField = parseField(inputStream);
    keyString = stripSurroundingWhitespace(nextField[0]);
    valueString = stripSurroundingWhitespace(nextField[1]);
    if (keyString.length > 0)
      alert("Problem in parsePreambleEntry()");
    newEntry.string += valueString;
    if (getCurrToken() == "}")
      allDone = true;
  }
//    bFound = false;
//    do
//    {
//      currToken = getNextToken(currTokens, inputStream);
//      switch(currToken)
//      {
//        //case ",":
//        case "":
//        case "}":
//          allDone = true;
//        break;
//        case "{":
//          var bracedStr = parseBracedString(currTokens, inputStream, 0);
//          if (bracedStr == "")
//            allDone = true;
//          else
//           valueString += "{" + bracedStr + "}";
//        break;
//        case "\"":
//          var quotedStr = parseQuotedString(currTokens, inputStream);
//          if (quotedStr == "")
//            allDone = true;
//          else
//            valueString += "\"" + quotedStr + "\"";
//        break;
//        default:
//          valueString += currToken;
//        break;
//      }
//    } while (!bFound && !allDone);
//  }
//  if (bFound)
//  {
//    newEntry = new preambleEntry();
//    newEntry.string = valueStr;
//  }
  return newEntry;
}

function parseBracedString(inputStream, braceLevel)
{
  var allDone = false;
  var currToken = null;
  var bFound = false;
  var retString = "";
  do 
  {
    currToken = getNextToken(inputStream);
    if (currToken == null)
      allDone = true;
    else
    {
      switch(currToken)
      {
        case "}": //we've found the end
          bFound = true;
        break;
        case "{":
          var bracedStr = parseBracedString(inputStream, braceLevel + 1);
          if (getCurrToken() == "}")
          {
            retString += "{";
            retString += bracedStr;
            retString += "}";
          }
          else
            allDone = true;
        break;
  //      case "\"":  //ignore quotes inside braces??
  //        var quotedStr = parseQuotedString(currTokens, inputStream);
  //        if (quotedStr != "")
  //        {
  //          retString += "\"" + quotedStr + "\"";
  //        }
  //        else
  //          allDone = true;
  //      break;
        case "":
          allDone = true;
        break;
        default:
          retString += currToken;
        break;
      }
    }
  } while (!bFound && !allDone);
  if (bFound)
    return retString;
  return "";
}

function parseQuotedString(inputStream)
{
  var allDone = false;
  var currToken = null;
  var bFound = false;
  var retString = "";
  do 
  {
    currToken = getNextToken(inputStream);
    if (currToken == null)
      allDone = true;
    else
    {
      switch(currToken)
      {
        case "\"": //we've found the end
          bFound = true;
        break;
        case "{":
          var bracedStr = parseBracedString(inputStream, 0);
          if (getCurrToken() == "}")
          {
            retString += "{";
            retString += bracedStr;
            retString += "}";
          }
        break;
        case "":
          allDone = true;
        break;
        default:
          retString += currToken;
        break;
      }
    }
  } while (!bFound && !allDone);
  if (bFound)
    return retString;
  return "";
}


function getNextToken(inputStream)
{
  var thisLine = null;
  var nCurrToken = -1;
  if (gDialog.theLines.length > 0)
  {
    thisLine = gDialog.theLines[gDialog.theLines.length - 1];
    nCurrToken = thisLine.nCurrToken;
  }
  if (!thisLine || nCurrToken >= thisLine.tokens.length-1)
  {
    thisLine = readNextLine(inputStream);
  }
  if (thisLine.tokens.length > 0)
    return thisLine.tokens[++thisLine.nCurrToken];
  else
    gDialog.fileAtEOF = true;  //that's the only way readNextLine() will return an empty list
  return null;
}

function readNextLine(inputStream)
{
  var lineStream = inputStream.QueryInterface(Components.interfaces.nsILineInputStream);
  var seekableStream = inputStream.QueryInterface(Components.interfaces.nsISeekableStream);

  var currTokens = new LineTokens();
  currTokens.nStreamStart = seekableStream.tell();

  var theLine = { value: "" };
  while (lineStream.readLine(theLine))
  {
    var thePieces = theLine.value.split(bibTeXSplitExp);
    for (var i = 0; i < thePieces.length; ++i)
    {
      if (thePieces[i].length > 0)
        currTokens.tokens.push(thePieces[i]);
    }
    currTokens.nCurrToken = -1;
    gDialog.theLines.push(currTokens);
    if (currTokens.tokens.length > 0)
      break;
  }
  return currTokens;
}