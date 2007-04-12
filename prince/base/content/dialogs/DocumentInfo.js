// Copyright (c) 2007 MacKichan Software, Inc.  All Rights Reserved.

//var gBodyElement;

//const emptyElementStr=" ";

//const mmlns    = "http://www.w3.org/1998/Math/MathML";
//const xhtmlns  = "http://www.w3.org/1999/xhtml";

var data;
//The "data" passed in to this dialog is assumed to be complex, containing:
//  data.documentUri - an nsiUri, we hope
//  data.created - a string
//  data.lastRevised - a string
//  data.language - a string
//  data.documentShell - a string
//  data.documentTitle - a string
//  data.comments - an object containing:
//    data.comments.comment - an object as in the "metadata" description below
//    data.comments.description - an object as in the "metadata" description below
//  data.printOptions - an object containing:
//    data.printOptions.theOptions - an msiPrintOptions object (in msiEditorUtilities.js), which contains flags:
//      data.printOptions.theOptions.useDefaultPrintOptions - a boolean
//      data.printOptions.theOptions.useCurrViewSettings - a boolean
//      data.printOptions.theOptions.useCurrViewInvisibles - a boolean
//      data.printOptions.theOptions.useCurrViewHelperLines - a boolean
//      data.printOptions.theOptions.useCurrViewInputBoxes - a boolean
//      data.printOptions.theOptions.useCurrViewMarkers - a boolean
//      data.printOptions.theOptions.useCurrViewIndexEntries - a boolean
//      data.printOptions.theOptions.allTextInBlack - a boolean
//      data.printOptions.theOptions.allLinesInBlack - a boolean
//      data.printOptions.theOptions.backgroundsTransparent - a boolean
//      data.printOptions.theOptions.suppressGrayBoxes - a boolean
//      data.printOptions.theOptions.useCurrViewZoom - a boolean
//    data.printOptions.zoomPercentage - a number (perhaps as a string, doesn't matter)
//  data.metadata - an object containing members indexed by (our internal) metadatum name. Each member is an object
//    containing either a "uri" member or a "contents" member, depending on whether it represents a <meta> or a <link>.
//    The "internal metadatum name" is taken from the keys in the file docInfoDialog.properties, with the prefix (before
//    the dot) removed. It will also contain a member "name" which is the full and properly capitalized name (or "rel" in
//    the case of <link>) of the property as it appears in the document (and hopefully the listbox); a member "type" which
//    identifies the type of node the datum is stored in in the document (should be either "title" for <title>, or one of
//    the preset strings "meta", "link", "comment-meta", "comment-link", "comment-meta-alt", "comment-link-alt", or
//    "comment-key-value"); and a "status" variable which we set to "changed" or "deleted" if the user changes or "unsets"
//    it (or deletes the data) in the dialog.
//    E.g.:
//      data.metadata.author.contents = "Noam Chomsky"
//      data.metadata.previous.uri = "../siblingDirectory/documentBeforeThisOne.xhtml"
//      data.metadata.chapter.uri = "http://junk.mackichan.com/internal/sillyDocuments/chapterTwo.xhtml"
//  data.saveOptions - an object containing:
//    saveData.useRelativeGraphicsPaths - a boolean
//    saveData.storeViewSettings - a boolean
//    saveData.storeViewPercent - a boolean
//    saveData.storeNoteViewSettings - a boolean
//    saveData.storeNoteViewPercent - a boolean
//Of course, any of these members may be absent from "data", but this is the structure.

// dialog initialization code
function Startup()
{
  var editorElement = msiGetParentEditorElementForDialog(window);
  var editor = msiGetEditor(editorElement);
  if (!editor) {
    window.close();
    return;
  }

  doSetOKCancel(onAccept, onCancel);
  data = window.arguments[0];
  
//  var logStr = "Data in DocumentInfo.js: "
//  for (var loggingData in data)
//    logStr += loggingData + ", ";
//  logStr += "\n";
//  dump(logStr);
  
  data.cancel = false;

  InitDialog();

  document.getElementById("docTitleBox").focus();

  SetWindowLocation();
}

function InitDialog()
{
  var editorElement = msiGetParentEditorElementForDialog(window);
  var editor = msiGetEditor(editorElement);
  dump("In DocumentInfo.initDialog, documentUri is " + data.general.documentUri + "\n");
//  var ourLocalFile = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
//  ourLocalFile.initWithPath(data.general.documentUri);
  var docUriObject = msiGetIOService().newURI(data.general.documentUri, editor.documentCharacterSet, null);
  dump("In DocumentInfo.initDialog, ourDocUri has spec [" + docUriObject.spec + "].\n");
  docUriObject.QueryInterface(Components.interfaces.nsIURL);
  document.getElementById('filenameBox').value = docUriObject.fileName;
  document.getElementById('directoryBox').value = docUriObject.directory;

  setDataToTextbox(data.general, "created", "", "createdBox");
  setDataToTextbox(data.general, "lastRevised", "", "lastRevisedBox");
  setDataToTextbox(data.general, "language", "", "languageBox");
  setDataToTextbox(data.general, "documentShell", "", "docShellBox");
  setDataToTextbox(data.general, "documentTitle", "", "docTitleBox");

  getComments(data);
  getPrintOptions(data);
  getMetadataOptions(data);
  getSaveOptions(data);
  
  checkEnablePrintControls();
  checkEnableMetadataControls();

  document.documentElement.getButton("accept").setAttribute("default", true);
}

function onAccept()
{
//Stuff it in the "data" - leave document storage up to the code invoking the dialog.

  data.general.documentTitle = document.getElementById('docTitleBox').value;
  storeComments(data);
  storePrintOptions(data);
  storeMetadataOptions(data);
  storeSaveOptions(data);

  var editorElement = msiGetParentEditorElementForDialog(window);
  var theWindow = window.opener;
  if (!theWindow || !("msiFinishDocumentInfoDialog" in theWindow))
    theWindow = msiGetTopLevelWindow(window);
  try
  {
    theWindow.msiFinishDocumentInfoDialog(editorElement, data);
  }
  catch(exc) {dump("Error in DocumentInfo dialog in onAccept, calling msiFinishDocumentInfoDialog: " + exc);}

  SaveWindowLocation();

  return true;
}

function getComments(theData)
{
  var commentData = null;
  if ("comments" in theData)
    commentData = theData.comments;
  if (commentData != null && "comment" in commentData)
    document.getElementById('commentsBox').value = commentData.comment;
  if (commentData != null && "description" in commentData)
    document.getElementById('descriptionBox').value = commentData.description;
}

function getPrintOptions(theData)
{
  var printOptionsData = null;
  var printOptionsFlags = null;
  if ("printOptions" in theData)
    printOptionsData = theData.printOptions;
  if ((printOptionsData != null) && ("theOptions" in theData.printOptions))
    printOptionsFlags = theData.printOptions.theOptions;

  setDataToCheckbox(printOptionsFlags, "useDefaultPrintOptions", true, "defaultPrintOptionsCheckbox");
  setDataToCheckbox(printOptionsFlags, "useCurrViewSettings", true, "printUseCurrViewSettings");
  setDataToCheckbox(printOptionsFlags, "printInvisibles", true, "printInvisibles");
  setDataToCheckbox(printOptionsFlags, "printHelperLines", true, "printHelperLines");
  setDataToCheckbox(printOptionsFlags, "printInputBoxes", true, "printInputBoxes");
  setDataToCheckbox(printOptionsFlags, "printMarkers", true, "printMarkers");
  setDataToCheckbox(printOptionsFlags, "printIndexEntries", true, "printIndexEntries");
  setDataToCheckbox(printOptionsFlags, "allTextInBlack", true, "printAllTextInBlack");
  setDataToCheckbox(printOptionsFlags, "allLinesInBlack", true, "printAllLinesInBlack");
  setDataToCheckbox(printOptionsFlags, "backgroundsTransparent", true, "printBackgroundsTransparent");
  setDataToCheckbox(printOptionsFlags, "grayButtonsTransparent", true, "printGrayButtonsTransparent");
  setDataToCheckbox(printOptionsFlags, "suppressGrayBoxes", true, "printSuppressGrayBoxes");
  setDataToCheckbox(printOptionsFlags, "useCurrViewZoom", true, "printUseCurrViewZoom");

  setDataToTextbox(printOptionsData, "zoomPercentage", "100", "printZoomPercentageBox");
}

//the data.metadata member of passed-in data is assumed to store information about the links and metadata indexed by the
//same string as appears in the linkRelationStrings (see docInfoDialog.properties). Thus, for instance,
//the value of data.metadata.creator should be an object describing this link, with in particular a member data.metadata.creator.uri
//giving the URI of it.
//We place the strings in the listbox, but make any changes directly to data.metadata. At Accept() time, the only thing to be
//done is to delete elements of data.links which have an empty URI.
function getMetadataOptions(theData)
{
  var theMetaData = null;
  if ("metadata" in theData)
    theMetaData = theData.metadata;
  var metaRelStrings = document.getElementById("metadataRelationStrings");
  var metaRelListbox = document.getElementById("metadataRelationsListbox");
  var relIter = metaRelStrings.strings;
  var currItem = null;
  while (relIter.hasMoreElements())
  {
    currItem = relIter.getNext().QueryInterface(Components.interfaces.nsIPropertyElement);
    
    if (currItem != null)
    {
//      var currValue = null;
//      if (linkData != null && (currItem.key in linkData) && linkData[currItem.key] != null)
//        currValue = linkData[currItem.key];
      var typeStr = getTypeStrFromKeyString(currItem.key);
      if (typeStr != "DC")  //exclude the Dublin Core ones for now
      {
        var theName = getRelationStrFromKeyString(currItem.key);
        var itemInfo = null;
        if (theName in theMetaData)
          itemInfo = theMetaData[theName];
        if (itemInfo == null)
        {
          itemInfo = new Object();
          itemInfo.name = currItem.value;
          itemInfo.type = typeStr;
//          itemInfo.status = "empty";
          theMetaData[theName] = itemInfo;
        }
        metaRelListbox.appendItem(currItem.value, theName);
      }
    }
  }
  for (var datum in theMetaData)
  {
    if (!findRelationInListbox(metaRelListbox, datum))
    {
      var theName = datum;
      if (("name" in theMetaData[datum]) && (theMetaData[datum].name != null))
        theName = theMetaData[datum].name;
      metaRelListbox.appendItem(theName, datum);
    }
  }
}

function getSaveOptions(theData)
{
  var saveData = null;
  if ("saveOptions" in theData)
    saveData = theData.saveOptions;
  setDataToCheckbox(saveData, "useRelativeGraphicsPaths", true, "relativeGraphicsPathsCheckbox");
  setDataToCheckbox(saveData, "storeViewSettings", true, "storeViewSettingsCheckbox");
  setDataToCheckbox(saveData, "storeViewPercent", true, "storeViewPercentCheckbox");
  setDataToCheckbox(saveData, "storeNoteViewSettings", true, "storeNoteViewSettingsCheckbox");
  setDataToCheckbox(saveData, "storeNoteViewPercent", true, "storeNoteViewPercentCheckbox");
}

function storeComments(theData)
{
  if (!("comments" in theData))
    theData.comments = new Object();
  getDataFromTextbox(theData.comments, "comment", "", "commentsBox");
  getDataFromTextbox(theData.comments, "description", "", "descriptionBox");
}

function storePrintOptions(theData)
{
  if (!("printOptions" in theData))
    theData.printOptions = new Object();
  if (!("theOptions" in theData.printOptions))
    theData.printOptions.theOptions = new Object();

  getDataFromCheckbox(theData.printOptions.theOptions, "useDefaultPrintOptions", true, "defaultPrintOptionsCheckbox");
  getDataFromCheckbox(theData.printOptions.theOptions, "useCurrViewSettings", true, "printUseCurrViewSettings");
  getDataFromCheckbox(theData.printOptions.theOptions, "printInvisibles", true, "printInvisibles");
  getDataFromCheckbox(theData.printOptions.theOptions, "printHelperLines", true, "printHelperLines");
  getDataFromCheckbox(theData.printOptions.theOptions, "printInputBoxes", true, "printInputBoxes");
  getDataFromCheckbox(theData.printOptions.theOptions, "printMarkers", true, "printMarkers");
  getDataFromCheckbox(theData.printOptions.theOptions, "printIndexEntries", true, "printIndexEntries");
  getDataFromCheckbox(theData.printOptions.theOptions, "allTextInBlack", true, "printAllTextInBlack");
  getDataFromCheckbox(theData.printOptions.theOptions, "allLinesInBlack", true, "printAllLinesInBlack");
  getDataFromCheckbox(theData.printOptions.theOptions, "backgroundsTransparent", true, "printBackgroundsTransparent");
  getDataFromCheckbox(theData.printOptions.theOptions, "grayButtonsTransparent", true, "printGrayButtonsTransparent");
  getDataFromCheckbox(theData.printOptions.theOptions, "suppressGrayBoxes", true, "printSuppressGrayBoxes");
  getDataFromCheckbox(theData.printOptions.theOptions, "useCurrViewZoom", true, "printUseCurrViewZoom");

  getDataFromTextbox(theData.printOptions, "zoomPercentage", "100", "printZoomPercentageBox");
}

//Only thing to do here is to delete members of theData.metadata which have empty content. The rest of the data
//is updated as it's "Link"ed or "Unlink"ed.
function storeMetadataOptions(theData)
{
  if ("metadata" in theData)
  {
    for (var datum in theData.metadata)
    {
      if ( ( !("uri" in theData.metadata[datum]) || (theData.metadata[datum].uri.length == 0) )
            && ( !("contents" in theData.metadata[datum]) || (theData.metadata[datum].contents.length == 0) ) )
        theData.metadata[datum].status = "deleted";
      if (theData.metadata[datum].status == "uninitialized")  //just added for the dialog, not present originally, and not set
        delete theData.metadata[datum];
    }
  }
}

function storeSaveOptions(theData)
{
  if (!("saveOptions" in theData))
    theData.saveOptions = new Object();
  getDataFromCheckbox(theData.saveOptions, "useRelativeGraphicsPaths", true, "relativeGraphicsPathsCheckbox");
  getDataFromCheckbox(theData.saveOptions, "storeViewSettings", true, "storeViewSettingsCheckbox");
  getDataFromCheckbox(theData.saveOptions, "storeViewPercent", true, "storeViewPercentCheckbox");
  getDataFromCheckbox(theData.saveOptions, "storeNoteViewSettings", true, "storeNoteViewSettingsCheckbox");
  getDataFromCheckbox(theData.saveOptions, "storeNoteViewPercent", true, "storeNoteViewPercentCheckbox");
}

function onCancel()
{
  data.Cancel = true;
}

function doAccept()
{
  document.documentElement.getButton('accept').oncommand();
}

function doBrowseFileLinks()
{
  var dirPicker = Components.classes["@mozilla.org/filepicker;1"].createInstance(Components.interfaces.nsIFilePicker);
  dirPicker.init(window, "Select file", Components.interfaces.nsIFilePicker.modeOpen);
  dirPicker.appendFilters(Components.interfaces.nsIFilePicker.filterHTML|Components.interfaces.nsIFilePicker.filterXML);
  var pathTextbox = document.getElementById("metadataValueBox");
  var ioService = msiGetIOService();
  var baseUri = getBasePath();
  if (pathTextbox.value.length > 0)
  {
    var theUri = ioservice.newURI(pathTextbox.value, null, null);

    theUri = theURI.resolve(baseUri);
    dirPicker.displayDirectory = baseUri.directory;
//    dirPicker.displayDirectory.initWithPath(

  }
  var res = dirPicker.show();
  if (res == Components.interfaces.nsIFilePicker.returnOK)
  {
    var displayUri = dirPicker.fileURL.spec;
    if (baseUri.length > 0 && shouldDisplayRelativeLinkPaths())
    {
      var editorElement = msiGetParentEditorElementForDialog(window);
//      var editor = msiGetEditor(editorElement);
//      var baseUriObject = msiGetIOService().newURI(baseUri, editor.documentCharacterSet, null);
//      baseUriObject.QueryInterface(Components.interfaces.nsIURL);
//      displayUri = dirPicker.fileURL.getRelativeSpec(baseUriObject);
      displayUri = msiMakeUrlRelativeTo(displayUri, baseUri, editorElement);
    }
    pathTextbox.value = displayUri;
    checkEnableMetadataControls();
  }
}

function shouldDisplayRelativeLinkPaths()
{
  return true;  //Why not? Until and unless we add a checkbox for this...
}

function getBasePath()
{
  var baseUri = data.general.documentUri;
  if (("baseUri" in data.general) && (data.general.baseUri != null) && (data.general.baseUri.length > 0))
    baseUri = data.general.baseUri;
  return baseUri;
}

function getTypeStrFromKeyString(theStr)
{
  if (!theStr || theStr.length == 0)
  {
    dump("Empty or null string passed into getTypeStrFromKeyString in DocumentInfo dialog!\n"); 
    return "";
  }
  var nDot = theStr.indexOf(".");
  if (nDot > -1)
    return theStr.substring(0, nDot);
  return "";
}

function getRelationStrFromKeyString(theStr)
{
  var nDot = theStr.indexOf(".");
  return theStr.substring(nDot+1);
}

function findRelationInListbox(theListbox, theRelation)
{
  var nIndex = -1;
  var nItems = theListbox.getRowCount();
  for (var ii = 0; ii < nItems; ++ii)
  {
    var listItem = theListbox.getItemAtIndex(ii);
    if (listItem.value != null)
    {
      if ( listItem.value.toLowerCase() == theRelation.toLowerCase())
      {
        nIndex = ii;
        break;
      }
    }
  }
  return nIndex;
}

var printSettingsControlIDs = ["printUseCurrViewInvisibles", "printHelperLines",
                               "printInputBoxes", "printMarkers",
                               "printIndexEntries"];
var printViewSettingsControlIDs = ["printAllTextInBlack", "printAllLinesInBlack",
                                   "printBackgroundsTransparent", "printGrayButtonsTransparent",
                                   "printSuppressGrayBoxes"];

function checkEnablePrintControls()
{
  var defaultCheckbox = document.getElementById("defaultPrintOptionsCheckbox");
  var useViewSettingsCheckbox = document.getElementById("printUseCurrViewSettings");
  var useViewZoomCheckbox = document.getElementById("printUseCurrViewZoom");
//  if (defaultCheckbox.checked)
  var bEnableAll = !(defaultCheckbox.checked);
  var bEnableView = bEnableAll && !(useViewSettingsCheckbox.checked);
  var bEnableZoom = bEnableAll && !(useViewZoomCheckbox.checked);
  enableControlsByID(printSettingsControlIDs, bEnableAll);  //function is in msiDialogUtilities.js
  enableControlsByID(printViewSettingsControlIDs, bEnableView);  //function is in msiDialogUtilities.js
  enableControlsByID(["printZoomPercentageBox"], bEnableZoom);  //function is in msiDialogUtilities.js
  enableControlsByID(["makePrintOptionsDefault"], !printSettingsMatchDefaults());
}

function printSettingsMatchDefaults()
{
  return false;  //Need to re-examine this - for now we'll just allow the "Set as Default" button to always be enabled.
}

//This enables the three button controls in the Metadata page ("Set", "Unset", "Browse"), plus the "Type" radio group.
function checkEnableMetadataControls()  
{
  var bEnableBrowse = false;
  var bEnableSet = false;
  var bEnableUnset = false;
  var bEnableTypes = false;
  var bShouldBeLink = false;

  var contentTextbox = document.getElementById("metadataValueBox");
  var theValue = contentTextbox.value;
  var metaRelListbox = document.getElementById("metadataRelationsListbox");
  var theItem = metaRelListbox.selectedItem;
  var bTypeIsLink = (document.getElementById("metadataTypeRadioGroup").value == "link");
  if (theItem != null && theItem.value != null)
  {
    bEnableBrowse = true;
    var metaObj = null;
    if (theItem.value in data.metadata)
      metaObj = data.metadata[theItem.value];
    if (metaObj == null)
    {
      dump("In DocumentInfo dialog, in checkEnableMetadataControls, relation " + theItem.label + " has no associated data object in metadata!\n");
      metaObj = new Object();
      metaObj.name = theItem.label;
      metaObj.type = "meta";  //should we go looking about otherwise for it?
      data.metadata[theItem.value] = metaObj;
    }
    switch (metaObj.type)
    {
      case "DC":
        alert("DC metadata not supposed to appear in listbox yet!");
      case "meta":
      default:
        bEnableTypes = true;
      break;
      case "link":
        bShouldBeLink = true;
      break;
    }
//    if (theItem.value.name in data.metadata && data.metadata[theItem.value.name] != null)
//    {
      if (theValue.length > 0)
      {
        if ( bTypeIsLink && (!metaObj.uri || (metaObj.uri != theValue)) )
          bEnableSet = true;
        else if ( !bTypeIsLink && (!metaObj.contents || (metaObj.contents != theValue)) )
          bEnableSet = true;
      }
      if ( ((metaObj.uri != null) && (metaObj.uri.length > 0)) || ((metaObj.contents != null) && (metaObj.contents.length > 0)) )
        bEnableUnset = true;
//    }
//    else if (theValue.length > 0)
//      bEnableSet = true;
  }
  if (bShouldBeLink && !bTypeIsLink) //force it
    document.getElementById("metadataTypeRadioGroup").selectedItem = document.getElementById("metadataTypeLinkRadio");
  else if (!bShouldBeLink && bTypeIsLink)
    document.getElementById("metadataTypeRadioGroup").selectedItem = document.getElementById("metadataTypeDataRadio");
    
  enableControlsByID(["setMetadataSelectionButton"], bEnableSet);
  enableControlsByID(["unsetMetadataSelectionButton"], bEnableUnset);
  enableControlsByID(["linksFileBrowseButton"], bEnableBrowse);
  enableControlsByID(["metadataTypeRadioGroup","metadataTypeDataRadio","metadataTypeLinkRadio"], bEnableTypes); 
}

function setMetadataType(whichType)
{
  checkEnableMetadataControls();
}

function setMetadataSelection()
{
  if (!("metadata" in data))
  {
    AlertWithTitle("Error in DocumentInfo.js", "In setMetadataSelection, dialogInfo has no metadata member!?");
    data.metadata = new Object();
  }

  var theValue = document.getElementById("metadataValueBox").value;
  var theItem = document.getElementById("metadataRelationsListbox").selectedItem;

  if (theItem != null && theItem.value != null && theValue != null && theValue.length > 0)
  {
    if (!(theItem.value in data.metadata))
    {
      data.metadata[theItem.value] = new Object();
      dump("In DocumentInfo dialog, in checkEnableMetadataControls, relation " + theItem.label + " has no associated data object in metadata!\n");
      data.metadata[theItem.value].name = theItem.label;
      data.metadata[theItem.value].type = "meta";  //should we go looking about otherwise for it?
    }
    var theObj = data.metadata[theItem.value];
    var typeStr = document.getElementById("metadataTypeRadioGroup").value;
    if (typeStr == "link")
    {
      theObj.uri = theValue;
      theObj.type = "link";
    }
    else
      theObj.contents = theValue;
    theObj.status = "changed";
  }
}

function unsetMetadataSelection()
{
  if (!("metadata" in data))
    return;  //nothing to do - except perhaps disable the "Unset" button...

  var metaRelListbox = document.getElementById("metadataRelationsListbox");
  var theItem = metaRelListbox.selectedItem;

  if (theItem != null && theItem.value != null)
  {
    if (theItem.value in data.metadata && data.metadata[theItem.value] != null)
    {
      var theObj = data.metadata[theItem.value];
      if ("uri" in theObj)
        theObj.uri = "";
      if ("contents" in theObj)
        theObj.contents = "";
      theObj.status = "deleted";
    }
  }
}

function changeMetadataRelation(selItem)
{
  var valStr = "";
  var theObj = null;
  var currType = "meta";
  if (selItem.value in data.metadata)
  {
    theObj = data.metadata[selItem.value];
    if (("type" in theObj) && (theObj.type != null))
      currType = theObj.type;
    else if ( ("uri" in theObj) && theObj.uri != null)
    {
      valStr = theObj.uri;
      currType = "link";
    }
    else if ( ("contents" in theObj) && theObj.contents != null)
      valStr = theObj.contents;
  }
  var valueTextbox = document.getElementById("metadataValueBox");
  valueTextbox.value = valStr;
  if (valStr.length > 0)
    document.getElementById("unsetMetadataSelectionButton").disabled = false;
  else
    document.getElementById("unsetMetadataSelectionButton").disabled = true;
  document.getElementById("setMetadataSelectionButton").disabled = true;  //Unless something is changed, this would do nothing.
  document.getElementById("metadataTypeRadioGroup").value = currType;
  checkEnableMetadataControls();
}

function setDataToCheckbox(sourceData, dataName, bDefault, checkboxID)
{
  var boolVal = bDefault;
  if (sourceData != null && (dataName in sourceData))
    boolVal = sourceData[ dataName ];
  document.getElementById(checkboxID).checked = boolVal;
}

function setDataToTextbox(sourceData, dataName, defaultStr, textboxID)
{
  var strVal = defaultStr;
  if (sourceData != null && (dataName in sourceData))
    strVal = sourceData[ dataName ];
  document.getElementById(textboxID).value = strVal;
}

function getDataFromCheckbox(sourceData, dataName, bDefault, checkboxID)
{
  var boolVal = document.getElementById(checkboxID).checked;
  if ((dataName in sourceData) || boolVal != bDefault)
    sourceData[ dataName ] = boolVal;
}

function getDataFromTextbox(sourceData, dataName, defaultStr, textboxID)
{
  var strVal = document.getElementById(textboxID).value;
  if ((dataName in sourceData) || strVal != defaultStr)
    sourceData[ dataName ] = strVal;
}

