/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Mozilla Communicator client code, released
 * March 31, 1998.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998-1999
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Pete Collins
 *   Brian King
 *   Ben Goodger
 *   Charles Manske (cmanske@netscape.com)
 *   Neil Rashbrook (neil@parkwaycc.co.uk)
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either of the GNU General Public License Version 2 or later (the "GPL"),
 * or the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include ../productname.inc
///

Components.utils.import("resource://app/modules/pathutils.jsm"); 
Components.utils.import("resource://app/modules/unitHandler.jsm");
Components.utils.import("resource://app/modules/graphicsConverter.jsm");

var gDialog;
var gEditorElement;
var gEditor;
var imageElement;
var wrapperElement;
var gDefaultWidth = 200;
var gDefaultHeight = 100;
var gDefaultUnit = "pt";
var gInitialSrc = "";
var gHaveDocumentUrl = false;
var gOriginalSrcUrl = "";
var gSrcUrl;
var gPreviewImageWidth = 80;
var gPreviewImageHeight = 50;
var gPreviewSrc;
var gCopiedSrcUrl;
var gOriginalKey;
var gUnitHandler;

// These mode variables control the msiFrameOverlay code
var gFrameModeImage = true;
var gFrameModeTextFrame = false;

var gPreviewImageNeeded = false;
var gIsGoingAway = false;
var gInsertNewImage = true;
/* We save the caption node, since we might rewrite its ancestor nodes, but we won't change its contents. If it doesn't exist, we might create a new empty caption, or we might delete the current one, depending on the user's choice. */

var gVideo = false;
var isSVGFile = false;
var gErrorMessageShown = false;
var importTimer;
//var nativeGraphicTypes = ["png", "gif", "jpg", "jpeg", "pdf", "svg", "xvc","xvz"];
//var typesetGraphicTypes = ["eps", "pdf", "png", "jpg"];
var videoTypes = ["avi","mov","qt","mp4","mpeg","mpg","prc","rm","rv","swf","u3d","wmv"];

#ifdef PROD_SNB
var bNeedTypeset = false;
#else
var bNeedTypeset = true;
#endif

// dialog initialization code

/* An essay on the files in this module;
An image can be inserted as an <object> or as an <object> wrapped in a frame <msiframe>. The frame is necessary for placing captions correctly on the screen. When this dialog is created, it may be passed an <object> (when an image is being revised) or not (when a new image is inserted). Ron's code uses two temporary elements (an <object> and a <msiframe>) to collect the attributes for the final objects, although it really doesn't seem necessary, as the attributes are all applied inside the onAccept function, and take place inside a transaction.
imageElement: this is passed as a parameter in the revision case; in the new image case, it is null until it is assigned in onAccept.
wrapperElement: if imageElement is passed, and has an <msiframe> parent, the parent is assigned to wrapperElement. Otherwise it is null until it is assigned in onAccept.
*/

/*  We get the size from the object and its frame. It is fully populated by
the time we call this. There is no need to check the preferences here.*/
function setImageSizeFields(framenode, objectnode)
{
  var aspectRatio;
  var naturalwidth, naturalheight;
  var width, height;
  var units = framenode.getAttribute('units');
  var unithandler = new UnitHandler();
  var isActualSize;

  naturalwidth = framenode.getAttribute('naturalwidth');
  naturalheight = framenode.getAttribute('naturalheight');
  width = framenode.getAttribute('width');  
  height = framenode.getAttribute('height'); 
  // do a comparison to determine if the graphic is its actual size
  unithandler.setCurrentUnit(units);
  isActualSize = 
    (approxEqual(naturalwidth, width) &&
    approxEqual(naturalheight, height)) 
  if (isActualSize)
  {
    document.getElementById('sizeradio').setAttribute('value','actual');
  } else {
    document.getElementById('sizeradio').setAttribute('value','custom');
  }

  document.getElementById("frameWidthInput").value = 
    isActualSize ? naturalwidth : width;
  document.getElementById("frameHeightInput").value = 
    isActualSize ? naturalheight : height;
}
// function setFrameSizeFromExisting(node) is defined in msiFrameOverlay.js

function Startup()
{
  gEditorElement = msiGetParentEditorElementForDialog(window);
  gEditor = msiGetEditor(gEditorElement);
  if (!gEditor)
  {
    window.close();
    return;
  }

  var unitHandler = new UnitHandler(gEditor);

  var existingImage = false;
  var selection = gEditor.selection;
  var parent;

  gDialog = {};
  gDialog.isImport          = true;
  gDialog.tabBox            = document.getElementById( "TabBox" );
  gDialog.tabPicture        = document.getElementById( "imagePictureTab" );
  gDialog.tabPlacement      = document.getElementById( "msiPlacementTab" );
  gDialog.tabFrame          = document.getElementById( "msiFrameTab" );
  gDialog.srcInput          = document.getElementById( "srcInput" );
  gDialog.altTextInput      = document.getElementById( "altTextInput" );
  gDialog.ImageHolder       = document.getElementById( "preview-image-holder" );
  gDialog.PreviewWidth      = document.getElementById( "PreviewWidth" );
  gDialog.PreviewHeight     = document.getElementById( "PreviewHeight" );
  gDialog.PreviewSize       = document.getElementById( "PreviewSize" );
  gDialog.PreviewImage      = null;
  gDialog.OkButton          = document.documentElement.getButton("accept");
  gDialog.keyInput          = document.getElementById("keyInput");

  // Get a single selected image element
  imageElement = null;
  wrapperElement = null;

  if (window.arguments && window.arguments.length > 0)
  {
    gVideo = window.arguments[0].isVideo;
    var selectedElement = window.arguments[0].mNode;
    if (selectedElement){
      if (selectedElement.nodeName === "msiframe") {
         existingImage = true;
         wrapperElement = selectedElement;
         objList = msiNavigationUtils.getChildrenByTagName(wrapperElement, "object");
         imageElement = objList[0];
      } else if (selectedElement.nodeName === "object"){
         existingImage = true;
         imageElement = selectedElement;
         if (imageElement.parentNode.nodeName !== "msiframe"){
             wrapperElement = gEditor.createFrameWithDefaults("image", false, selection.anchorNode, selection.anchorOffset);
             wrapperElement.appendChild(imageElement);
         }
         else {
          wrapperElement = selectedElement.parentNode;
         }
      }
    } else { // a new insertion, rather than a revision
      try {
        wrapperElement = gEditor.createFrameWithDefaults("image", false, selection.anchorNode, selection.anchorOffset);
        if (wrapperElement == null) return; // should throw exception
        gEditor.getFrameStyleFromAttributes(wrapperElement);
        // parent = wrapperElement.parentNode;
        imageElement = wrapperElement.ownerDocument.createElement("object");
        gEditor.getGraphicsAttributesFromFrame(wrapperElement, imageElement);
      }
      catch(e) {
        dump(e);
      }
      
    }

    if (gVideo)
    {
      document.getElementById("isVideo").removeAttribute("hidden");
      document.title = document.getElementById("videoTitle").textContent;
      forceIsImport(false);  //Set import/reference to be reference by default for video files
    }
  }

  var tagName = gVideo ? "embed" : "object";

    // if a graphics file has been assigned, execute this
  if (imageElement.hasAttribute("src") || imageElement.hasAttribute("data"))
  {
    gInsertNewImage = false;
    tagName = imageElement.nodeName;
    if (imageElement.hasAttribute("src")) {
      gSrcUrl  = imageElement.getAttribute("src");
    } else {
      gSrcUrl = imageElement.getAttribute("data");
    }
    gDialog.srcInput.value = gSrcUrl;
    if (imageElement.hasAttribute("originalSrcUrl"))
      gOriginalSrcUrl = imageElement.getAttribute("originalSrcUrl");
    if (imageElement.hasAttribute("copiedSrcUrl"))
      gCopiedSrcUrl = imageElement.getAttribute("copiedSrcUrl");
    if (gCopiedSrcUrl && gCopiedSrcUrl.length > 0) {
      gDialog.srcInput.value = gCopiedSrcUrl;
      chooseImgFile(false);
    }
  }
  else
  {
    gInsertNewImage = true;
  }
  if (wrapperElement.hasAttribute("units")) {
     unitHandler.initCurrentUnit(wrapperElement.getAttribute("units"));
  } 
  // if inserting a new image, create elements now.
try {
  initFrameTab(gDialog, wrapperElement||imageElement, gInsertNewImage, imageElement);
}
catch(e) {
  msidump(e.message);
}
  initKeyList();

  // We only need to test for this once per dialog load
  gHaveDocumentUrl = msiGetDocumentBaseUrl();

  InitDialog(wrapperElement, imageElement);
  // if (!existingImage)
  //   setImageSizeFields(null,null, unitHandler.currentUnit);
  // Save initial source URL
  gInitialSrc = document.getElementById("srcInput").value || "";
  gCopiedSrcUrl = gInitialSrc;

  // By default turn constrain on, but both width and height must be in pixels
//  frameTabDlg.constrainCheckbox.checked = true;


  window.mMSIDlgManager = new msiDialogConfigManager(window);
  window.mMSIDlgManager.configureDialog();
  SetTextboxFocus(gDialog.srcInput);
  SetWindowLocation();
}

function InitDialog(wrapperElement, imageElement)
{
  InitImage(wrapperElement, imageElement);
}

function stripPx(s, index, array) // strip "px" from a string if it is there
{ if (/px/.test(s)) return RegExp.leftContext;
  else if (s=="thin") return 1;
  else if (s=="medium") return 3;
  else if (s=="thick") return 5;
}

var gBorderdesc = ["border-top-width","border-right-width","border-bottom-width","border-left-width"];
function fillInValue(element, index, array)
{
  if (!element || element.length == 0)
    element= msiGetHTMLOrCSSStyleValue(null, imageElement, gBorderdesc[index], null);
}

function getSourceLocationFromElement(imageNode)
{
  var fileSrc = "";
  if (imageElement.hasAttribute("src"))
    fileSrc = imageElement.getAttribute("src");
  else if (imageElement.hasAttribute("data"))
    fileSrc = imageElement.getAttribute("data");
  var lastDot = fileSrc.lastIndexOf(".");
  if (lastDot < 0)
    lastDot = 0;
  var extraStuff = fileSrc.indexOf("#", lastDot);
  if (extraStuff < 0)
    extraStuff = fileSrc.indexOf("?", lastDot);
  if (extraStuff >= 0)
    return fileSrc.substr(0, extraStuff);
  return fileSrc;
}

function loadDefaultsFromPrefs()
{
  try
  { gDefaultPlacement = GetStringPref("swp.graphics.placement"); }
  catch(ex) {gDefaultPlacement = "inline"; dump("Exception getting pref swp.graphics.placement: " + ex + "\n");}
  try
  { gDefaultInlineOffset = GetStringPref("swp.graphics.inlineoffset"); }
  catch(ex) {gDefaultInlineOffset = "0"; dump("Exception getting pref swp.graphics.inlineoffset: " + ex + "\n");}
}

var vidStartTimeControlIds = ["startFramesInput","startSecondsInput","startMinutesInput"];
var vidEndTimeControlIds = ["endFramesInput","endSecondsInput","endMinutesInput"];

function setVideoControlsFromElement(anElement)
{
  if (anElement && (msiGetBaseNodeName(anElement)=="object"))
    return setVideoControlsFromObjectElement(anElement);

  var attrStr;
  if (anElement)
    attrStr = anElement.getAttribute("autoplay");
  document.getElementById("vidAutoplayCheckbox").checked = (attrStr && (attrStr.toLowerCase() == "true")) ? true : false;
  if (anElement)
    attrStr = anElement.getAttribute("controller");
  document.getElementById("vidShowControlsCheckbox").checked = (attrStr && (attrStr.toLowerCase() == "true")) ? true : false;
  var valueStr = "False";
  if (anElement)
    attrStr = anElement.getAttribute("loop");
  if (attrStr && (attrStr.length > 0))
    valueStr = attrStr.substr(0,1).toUpperCase() + atrStr.subStr(1).toLowerCase();
  document.getElementById("vidLoopingList").value = valueStr;

  if (anElement)
    attrStr = anElement.getAttribute("starttime");
  setVideoStartEndTimeControls(true, attrStr);
  if (anElement)
    attrStr = anElement.getAttribute("endtime");
  setVideoStartEndTimeControls(false, attrStr);
}

function setVideoControlsFromObjectElement(anElement)
{
  var attrStr;
  var frameKids = [];
  var theAttrs = {autoplay : null, controller : null, loop : null, starttime : null, endtime : null};
  if (anElement)
    frameKids = anElement.getElementsByTagName("param");
  for (var ii = 0; ii < frameKids.length; ++ii)
  {
    theAttrs[frameKids[ii].getAttribute("name").toLowerCase()] = frameKids[ii].getAttribute("value");
  }

  document.getElementById("vidAutoplayCheckbox").checked = (theAttrs.autoplay && (theAttrs.autoplay == "true")) ? true : false;
  document.getElementById("vidShowControlsCheckbox").checked = (theAttrs.controller && (theAttrs.controller.toLowerCase() == "true")) ? true : false;

  var valueStr = "False";
  if (theAttrs.loop && (theAttrs.loop.length > 0))
    valueStr = theAttrs.loop.substr(0,1).toUpperCase() + theAttrs.loop.substr(1).toLowerCase();
  document.getElementById("vidLoopingList").value = valueStr;

  setVideoStartEndTimeControls(true, theAttrs.starttime);
  setVideoStartEndTimeControls(false, theAttrs.endtime);
}

function setVideoStartEndTimeControls(bStart, attrStr)
{
  var ourIds = (bStart ? vidStartTimeControlIds : vidEndTimeControlIds);
  var ourCheckboxId = (bStart ? "vidStartBeginningCheckbox" : "vidEndAtEndingCheckbox");
  var timePieces;
  var valNum = 0;
  if (attrStr && attrStr.length)
  {
    document.getElementById(ourCheckboxId).checked = false;
    timePieces = attrStr.split(":");
    for (var ii = 0; ii < timePieces.length; ++ii)
    {
      valNum = timePieces[timePieces.length - ii - 1];
      if (ii == 2)
      {
        if (timePieces.length > 3)
          valNum += 60 * timePieces[timePieces.length - 4];
        document.getElementById(ourIds[2]).valueNumber = valNum;
        break;
      }
      else
        document.getElementById(ourIds[ii]).valueNumber = valNum;
    }
  }
  else
  {
    document.getElementById(ourCheckboxId).checked = true;
  }
}

function setVideoSettingsToElement(anElement, editor)
{
  if (anElement && (msiGetBaseNodeName(anElement)=="object"))
    return setVideoSettingsToObjectElement(anElement);

  var attrStr = document.getElementById("vidAutoplayCheckbox").checked ? "true" : "false";
  msiEditorEnsureElementAttribute(anElement, "autoplay", attrStr, editor);

  var attrStr = document.getElementById("vidShowControlsCheckbox").checked ? "true" : "false";
  msiEditorEnsureElementAttribute(anElement, "controller", attrStr, editor);

  msiEditorEnsureElementAttribute(anElement, "loop", document.getElementById("vidLoopingList").value, editor);

  attrStr = readVideoStartEndControls(true);
  msiEditorEnsureElementAttribute(anElement, "starttime", attrStr, editor);
  attrStr = readVideoStartEndControls(false);
  msiEditorEnsureElementAttribute(anElement, "endtime", attrStr, editor);
}

function setVideoSettingsToObjectElement(anElement, editor)
{
  var attrStr = document.getElementById("vidAutoplayCheckbox").checked ? "true" : "false";
  msiEditorEnsureObjElementParam(anElement, "autoplay", attrStr, editor);

  var attrStr = document.getElementById("vidShowControlsCheckbox").checked ? "true" : "false";
  msiEditorEnsureObjElementParam(anElement, "controller", attrStr, editor);

  msiEditorEnsureObjElementParam(anElement, "loop", document.getElementById("vidLoopingList").value, editor);

  attrStr = readVideoStartEndControls(true);
  msiEditorEnsureObjElementParam(anElement, "starttime", attrStr, editor);
  attrStr = readVideoStartEndControls(false);
  msiEditorEnsureObjElementParam(anElement, "endtime", attrStr, editor);
}

function readVideoStartEndControls(bStart)
{
  var ourIds = (bStart ? vidStartTimeControlIds : vidEndTimeControlIds);
  var ourCheckboxId = (bStart ? "vidStartBeginningCheckbox" : "vidEndAtEndingCheckbox");
  var attrStr = null;
  if (!document.getElementById(ourCheckboxId).checked)
  {
    timePieces = attrStr.split(":");
    valNum = document.getElementById(ourIds[2]).valueNumber;
    attrStr = String( Math.floor(valNum/60) );
    attrStr += ":" + String(valNum % 60);
    attrStr += ":" + String( document.getElementById(ourIds[1]).valueNumber);
    attrStr += ":" + String( document.getElementById(ourIds[0]).valueNumber);
  }
  return attrStr;
}

function InitImage(wrapperElement, imageElement)
{
  try {
    var re = /[a-z]*/gi;
    var widthStr = "";
    var heightStr = "";
    var width;
    var height;
    var pixelWidth, pixelHeight;
    var unitHandler = new UnitHandler(null);
    var unit = wrapperElement.getAttribute('units');
    unitHandler.initCurrentUnit(unit);
    if (gCopiedSrcUrl && gCopiedSrcUrl.length > 0)
      gDialog.srcInput.value = gCopiedSrcUrl;
    // We found an element and don't need to insert one
    //trim off units at end
    gDialog.frameUnitMenulist.value = unit;
    if (wrapperElement.hasAttribute('width'))
    {
      width = wrapperElement.getAttribute('width');
      pixelWidth = Math.round(unitHandler.getValueAs(width,"px"));
    }
    if (wrapperElement.hasAttribute('height'))
    {
      height = wrapperElement.getAttribute('height');
      pixelHeight =  Math.round(unitHandler.getValueAs(height,"px"));
    }

    setWidthAndHeight(width, height, null);
    LoadPreviewImage(null);
    // caption and key info comes from frameOverlay
    document.getElementById("captionLocation").value = wrapperElement.getAttribute('captionloc');


    var hasAltText = imageElement.hasAttribute("alt");
    var altText;
    if (hasAltText)
    {
      altText = imageElement.getAttribute("alt");
      gDialog.altTextInput.value = altText;
    }

    SetSizeWidgets(pixelWidth, pixelHeight);
    if (!Number(gDialog.frameWidthInput.value))
      constrainProportions( "frameHeightInput", "frameWidthInput", null );
    else if (!Number(gDialog.frameHeightInput.value))
      constrainProportions( "frameWidthInput", "frameHeightInput", null );

    // set spacing editfields
    var element = wrapperElement || imageElement
    var pos = element.getAttribute("pos");
    gDialog.floatList.value=element.getAttribute("floatOption");

    var bordervalues;
    var bv = msiGetHTMLOrCSSStyleValue(null, element, "border", null);
    var i;
    if (bv.length > 0)
    {
      bordervalues = bv.split(/\s+/);
      if (bordervalues[3] && bordervalues[3].length > 0)
        ; /* all four values were given */
      else if (bordervalues[2] && bordervalues[2].length > 0)
        // only 3 values given. The last is the same as the second.
        bordervalues[3] = bordervalues[1];
      else if (bordervalues[1] && bordervalues[1].length > 0) {
        // only 2 values given; repeat
        bordervalues[2] = bordervalues[0]; bordervalues[3] = bordervalues[1];}
      else if (bordervalues[0] && bordervalues[0].length > 0)
        bordervalues[3] = bordervalues[2]  = bordervalues[1] = bordervalues[0];
    }
    else bordervalues = [0,0,0,0];
    bordervalues.forEach(fillInValue);
    bordervalues.forEach(stripPx);
  }
  catch(e) {
    dump(e.message);
  }
}



//This function assumes "width" and "height" are in pixels
function  SetSizeWidgets(width, height)
{
  if (!(width || height) || (gActualWidth && gActualHeight && width == gActualWidth && height == gActualHeight))
    document.getElementById("sizeRadio").value = "actual";
  else
    document.getElementById("sizeRadio").value = "custom";

  if (document.getElementById("sizeRadio").value !== "actual")
  {
    document.getElementById("sizeRadio").value = "custom";
    // Decide if user's sizes are in the same ratio as actual sizes
    if (gActualWidth && gActualHeight)
    {
      if (gActualWidth > gActualHeight)
        gDialog.constrainCheckbox.checked = (unitRound(gActualHeight * width / gActualWidth) == height);
      else
        gDialog.constrainCheckbox.checked = (unitRound(gActualWidth * height / gActualHeight) == width);
    }
  }
}

function initImageUnitList(unitPopUp)
{
  var elements = unitPopUp.getElementsByTagName("menuitem");
  var i, len;
  for (i=0, len = elements.length; i<len; i++)
  {
    elements[i].label = unitHandler.getDisplayString(elements[i].value);
    dump("element with value "+elements[i].value+" has label "+elements[i].label+"\n");
  }
}


function ChangeLinkLocation()
{
//  SetRelativeCheckbox(gDialog.makeRelativeLink);
//  gDialog.showLinkBorder.disabled = !TrimString(gDialog.hrefInput.value);
}

function ToggleShowLinkBorder()
{
//  if (gDialog.showLinkBorder.checked)
//  {
//    var border = TrimString(gDialog.border.value);
//    if (!border || border == "0")
//      gDialog.border.value = "2";
//  }
//  else
//  {
//    gDialog.border.value = "0";
//  }
}

function GetCaptionNode(parentNode)
{
  var nodes;
  if (parentNode) {
    nodes = parentNode.getElementsByTagName('imagecaption');
    if (nodes.length >= 1) {
      return nodes[0];
    }
  }
  return null;
}

function GetCaptionLoc(parentNode)
{
  if (parentNode && parentNode.hasAttribute("captionloc"))
    return parentNode.getAttribute("captionloc");
  else
    return "none";
}



function getNodeChildrenAsString(aNode)
{
  var retStr = "";
  var serializer = new XMLSerializer();
  var nodeKids = msiNavigationUtils.getSignificantContents(aNode);
  for (var jx = 0; jx < nodeKids.length; ++jx)
    retStr += serializer.serializeToString(nodeKids[jx]);
  return retStr;
}


// Get data from widgets, validate, and set for the global element
//   accessible to AdvancedEdit() [in msiEdDialogCommon.js]
function ValidateData()
{
  return ValidateImage();
}


function getDocumentGraphicsDir(mode)
{
  var docUrlString = msiGetDocumentBaseUrl();
  var docurl = msiURIFromString(docUrlString);
  var dir = msiFileFromFileURL(docurl);
  dir = dir.parent;
  if (mode == "tex")
    dir.append("tcache");
  else
    dir.append("graphics");
  return dir;
}

function makeImagePathRelative(filePath)
{
  var fileUrl = msiFileURLFromAbsolutePath(filePath);
  if (fileUrl)
    return msiMakeRelativeUrl(fileUrl.spec, gEditorElement);
  return filePath;  //if the above failed, was it already relative?
}

function chooseImgFile(fBrowsing)
{
  if (fBrowsing) {
    var fileFilterStr = "";
    var filterTitleStr = "";
    var fileTypeStr = "";

    if (gVideo)
    {
      fileFilterStr = getVideoImportFilterString();
      filterTitleStr = GetString("VideoFiles");
      fileTypeStr = "video";
    }
    else
    {
      fileFilterStr = getGraphicsImportFilterString();
      filterTitleStr = GetString("IMGFiles");
      fileTypeStr = "image";
    }
    var filterArray = [{filter : fileFilterStr, filterTitle : filterTitleStr}];
    var fileName = msiGetLocalFileURLSpecial(filterArray, fileTypeStr);

  }
  else
  {
    fileName = document.getElementById('srcInput').value;
  }
  if (fileName)
  {
    var url;
    var leafname;
    var file;
    var url = msiGetDocumentBaseUrl();

    try {
      arr = fileName.split('/');
      file = msiFileFromFileURL(url);
      file.append(arr[arr.length - 2]);
      file.append(arr[arr.length - 1]);

      gOriginalSrcUrl = file.path;
      leafname = file.leafName;
    }
    catch (e) {
      var arr = fileName.split('/');
      leafname = arr[arr.length - 1];
    }
    gPreviewImageNeeded = true;
    var importName;
    var internalFile;
    var internalName;
    var graphicDir = getDocumentGraphicsDir();
    var product;
#ifdef PROD_SWP
      product = "swp";
#endif
#ifdef PROD_SW
      product = "sw";
#endif
#ifdef PROD_SNB
      product = "snb";
#endif

    internalFile = graphicDir.clone(false);
    internalFile.append(leafname);
    if (!file) file = internalFile;
    internalName = graphicDir.leafName + "/" + internalFile.leafName;
    graphicsConverter.init(window, graphicDir.parent, product);
    importName = graphicsConverter.copyAndConvert(file, true);
    gSrcUrl = importName;
    if (!importName || !importName.length)
    {
      displayImportErrorMessage(fileName)
      fileName = "";
      return;
    }
    else {
      gCopiedSrcUrl = internalName;
    }

    if (gDialog.isImport) {
      gDialog.srcInput.value = internalName;
      gPreviewSrc = importName;
    }

    msiSetRelativeCheckbox();
    doOverallEnabling();
    LoadPreviewImage(gPreviewSrc, fileName);
  }
  SetTextboxFocus(gDialog.srcInput);
}


function getLoadableGraphicFile(inputURL)
{
  var importFiles = [];
  var typesetFiles = [];
  var fileName = "";
  var file = msiFileFromFileURL(inputURL);
  getGraphicsImportTargets(file, "import");
  var filedir = file.parent;
  var extension = getExtension(file.leafName);
  var dir = getDocumentGraphicsDir();
  var bDoImport = false;
  var bDoTypesetImport = false;
  if (!dir.exists())
    dir.create(1, 0755);

  if (extension)
  {
    var nNative = graphicsConverter.nativeGraphicTypes.indexOf(extension.toLowerCase());
    if ((nNative < 0) && !gVideo)
    {
      importFiles = getGraphicsImportTargets(file, "import");
      if (importFiles.length)
      {
        fileName = "graphics/" + importFiles[importFiles.length - 1].leafName;
        bDoImport = true;
      }
    }
    else if (gDialog.isImport) // copy the file into the graphics directory
    {
      if (dir.path != filedir.path)
      {
        var newFile = makeInternalCopy(file, dir);
        if (newFile)
        {
          importFiles.push( newFile );
          fileName = newFile.path;
        }
      }
  //    isSVGFile = /\.svg$/.test(fileName);
    }
    else
      fileName = file.path;
    if (bNeedTypeset)
    {
      if (graphicsConverter.typesetGraphicTypes.indexOf(extension.toLowerCase()) < 0)  //Need file in LaTeX-friendly format
      {
        var bCanUseImport = gVideo;  //if this is a video file, we don't consider converting it, otherwise check
        for (var kk = 0; !bCanUseImport && (kk < importFiles.length); ++kk)
        {
          var impExtension = getExtension(importFiles[kk].leafName).toLowerCase();
          if (graphicsConverter.typesetGraphicTypes.indexOf(impExtension) >= 0)
            bCanUseImport = true;
        }
        if (!bCanUseImport)
        {
          typesetFiles = getGraphicsImportTargets(file, "tex");
          if (typesetFiles.length)
            bDoTypesetImport = true;
        }
      }
    }
    if (bDoImport)
      doGraphicsImportToFile(file, importFiles[importFiles.length - 1], "import");
    if (bDoTypesetImport)
      doGraphicsImportToFile(file, typesetFiles[typesetFiles.length - 1], "tex");
  }
  return fileName;
}

function makeInternalCopy(file, dir)
{
  var fileName = "graphics/"+file.leafName;
  var importFile;
  try
  {
    file.permissions = 0755;
    importFile = dir.clone();
    importFile.append(file.leafName);
    if (importFile.exists())
      importFile.remove(false);
    file.copyTo(dir,"");
  }
  catch(e)
  {
    dump("exception: e="+e.msg);
  }
  return importFile;
}

function checkSourceAndImportSetting(src)
{
  try
  {
    var srcUrl = encodeURI(gOriginalSrcUrl);
    if (gDialog.isImport)
    {
      var dir = getDocumentGraphicsDir();
      var targFile = dir.clone();
      targFile.append(GetFilename(srcUrl));
      if (!targFile.exists())
      {
        var origSrc = msiURIFromString(encodeURI(srcUrl));
        var origSrcFile = msiFileFromFileURL(origSrc);
        targFile = makeInternalCopy(origSrcFile, dir);
        var targFileUrl = msiFileURLFromFile(targFile);
        src = msiMakeRelativeUrl(targFileUrl.spec, gEditorElement);
      }
    }
    else //if (!gDialog.relativeURL)  //if it isn't relative, we need to be using the URL form rather than the file form
    {
      src = encodeURI(gOriginalSrcUrl);
    }
  } catch(ex) {dump("Exception in msiEdImageProps, checkSourceAndImportSettings: " + ex + "\n");}
  return src;
}

var importTimerHandler =
{
  mSourceFile : null,
  mImportHandler : null,
  mTexHandler : null,
  failNoticePosted : false,

  terminalCallback : function(sourceFile, importTargFile, importStatus, errorString)
  {
    this.checkStatusLaunchDialog();
  },

  checkStatusLaunchDialog : function()
  {
    var bLaunchDialog = false;
    var bTimerRunning = false;
    var importData = {};
    if (this.mImportHandler && this.mImportHandler.isLoading())
    {
      if (!this.mImportHandler.timerStopped)
        bTimerRunning = true;
      bLaunchDialog = true;
      importData.mSourceFile = this.mImportHandler.sourceFile;
      importData.mImportProcess = this.mImportHandler.importProcess;
      importData.mImportStatus = this.mImportHandler.importStatus;
      importData.mImportTargFile = this.mImportHandler.importTargFile;
    }
    if (this.mTexHandler && this.mTexHandler.isLoading())
    {
      if (!this.mTexHandler.timerStopped)
        bTimerRunning = true;
      bLaunchDialog = true;
      importData.mSourceFile = this.mTexHandler.sourceFile;
      importData.mTexImportProcess = this.mTexHandler.importProcess,
      importData.mTexImportStatus = this.mTexHandler.importStatus;
      importData.mTexImportTargFile = this.mTexHandler.importTargFile;
    }
    if (bTimerRunning)
      return;

    if (bLaunchDialog)
    {
      dump("Launching ConvertGraphics dialog.\n");
      launchConvertingDialog(importData);
    }
    else
      this.checkFinalStatus();
  },

  checkStatus : function()
  {
    if (this.mImportHandler)
      this.mImportHandler.checkStatus();
    if (this.mTexHandler)
      this.mTexHandler.checkStatus();
  },

  reset : function()
  {
    if (this.mImportHandler)
      this.mImportHandler.reset();
    if (this.mTexHandler)
      this.mTexHandler.reset();
    this.failNoticePosted = false;
  },

  isLoading : function(mode)
  {
    if (mode == "import")
      return (this.mImportHandler && this.mImportHandler.isLoading());
    else if (mode == "tex")
      return (this.mTexHandler && this.mTexHandler.isLoading());
    else
      return ( (this.mImportHandler && this.mImportHandler.isLoading())
                || (this.mTexHandler && this.mTexHandler.isLoading()) );
  },

  didNotSucceed : function()
  {
    return ( (this.mImportHandler && this.mImportHandler.didNotSucceed())
             || (this.mTexHandler && this.mTexHandler.didNotSucceed()) );
  },

  //isActive means that an import process is in use, whether it's finished, failed, or still running
  isActive : function(mode)
  {
    if (this.mImportHandler && (!mode || (mode != "tex")))
      return true;
    else if (this.mTexHandler && (!mode || (mode != "import")))
      return true;
    else
      return false;
  },

  checkLogFileStatus : function()
  {
    if (this.mImportHandler)
      this.mImportHandler.checkLogFileStatus();
    if (this.mTexHandler)
      this.mTexHandler.checkLogFileStatus();
  },

  stopLoading : function()
  {
    if (this.mImportHandler)
      this.mImportHandler.stop();
    if (this.mTexHandler)
      this.mTexHandler.stop();
    this.checkFinalStatus();
  },

  checkFinalStatus : function()
  {
    if (this.failNoticePosted)
      return;
    this.checkStatus();
    if ((this.isLoading()))
      this.checkLogFileStatus();
    if (this.didNotSucceed())
    {
//      dump("In msiEdImageProps.js, checkFinalStatus, calling postFailedImportNotice.\n");
      this.postFailedImportNotice();
    }

    if (this.mImportHandler && (this.mImportHandler.importStatus == this.mImportHandler.statusSuccess))
      LoadPreviewImage();
  },

  sourceFile : function()
  {
    var theFile = null;
    if (this.mImportHandler)
      theFile = this.mImportHandler.sourceFile;
    if (!theFile && this.mTexHandler)
      theFile = this.mTexHandler.sourceFile;
    return theFile;
  },

  postFailedImportNotice : function()
  {
    if (this.failNoticePosted)
      return;
    var theMsg = "";
    var ext = "";
    var logInfo = "";
    if (this.mImportHandler && this.mImportHandler.processFailed())
    {
      theMsg = "imageProps.couldNotImport";
//      if ((this.mTexHandler == this.statusSuccess) || this.canTypesetSourceFile() )
      if (!this.mTexHandler || !this.mTexHandler.processFailed())
      {
        theMsg = "imageProps.couldNotImportButCanTeX";
        ext = getExtension(this.mImportHandler.importTargFile.leafName);
      }
      logInfo = this.mImportHandler.getImportLogInfo();
    }
    else if (this.mTexHandler && this.mTexHandler.processFailed())
    {
      theMsg = "imageProps.couldNotImportTeX";
      ext = getExtension(this.mTexHandler.importTargFile.leafName);
      logInfo = this.mTexHandler.getImportLogInfo();
    }
    var msgParams = {};
    if (this.sourceFile())
      msgParams.file = this.sourceFile().path;
    if (ext.length)
      msgParams.extension = ext;
    var msgString = msiGetDialogString(theMsg, msgParams);
    if (logInfo && (logInfo.length > 0))
      msgString += "\n" + logInfo;
    var titleStr = msiGetDialogString("imageProps.importErrorTitle", msgParams);
    this.failNoticePosted = true;
    AlertWithTitle(titleStr, msgString);
  },

  errorMessageShown : function()
  {
    return this.failNoticePosted;
  }

};


function launchConvertingDialog(importData)
{
  window.openDialog("chrome://prince/content/msiGraphicsConversionDlg.xul", "graphicsconversionrunning", "chrome,close,resizable,titlebar,modal", importData);
  //Then collect data the dialog may have changed (notably if the user cancelled the conversion)
  importTimerHandler.importStatus = importData.mImportStatus;
  importTimerHandler.texImportStatus = importData.mTexImportStatus;

  importTimerHandler.checkFinalStatus();
}

function getGraphicsImportTargets(inputFile, mode)
{
  var product;
#ifdef PROD_SWP
  product = "swp";
#endif
#ifdef PROD_SW
  product = "sw";
#endif
#ifdef PROD_SNB
  product = "snb";
#endif
  if (!mode || !mode.length)
    mode = "import";
  var graphicDir = getDocumentGraphicsDir(mode);
  if (!graphicDir.exists())
    graphicDir.create(1, 0755);
  graphicsConverter.init(window, graphicDir.parent, product);
  return graphicsConverter.copyAndConvert(inputFile);
//  return graphicsConverter.getTargetFilesForImport(inputFile, graphicDir, mode, window);
}

//  inputFile - an nsIFile, the graphic file being converted
//  mode - a string, either "tex" or "import" (defaults to "import")
//  Function initiates the import process and related timer and timer handlers, assuming the outputFile is the correct
//    target (that is, has been identified using the graphicsConverter.getTargetFilesForImport() function.
function doGraphicsImportToFile(inputFile, outputFile, mode)
{
  var graphicDir = outputFile.parent;
  var timerHandler = new graphicsTimerHandler(1600, importTimerHandler);
  if (mode=="tex")
    importTimerHandler.mTexHandler = timerHandler;
  else
    importTimerHandler.mImportHandler = timerHandler;
  graphicsConverter.doImportGraphicsToTarget(inputFile, outputFile, mode, window, timerHandler);
}


function SetImport(bSet)
{
}

function forceIsImport(bImport)
{
}

function PreviewImageLoaded()
{
  var unitHandler = new UnitHandler(gEditor);
  unitHandler.initCurrentUnit(gSizeState.sizeUnit);
  var previewActualWidth;
  var previewActualHeight;
  var docUrlString = msiGetDocumentBaseUrl();
  var docurl = msiURIFromString(docUrlString);
  var documentDir = msiFileFromFileURL(docurl);
  documentDir = documentDir.parent;

  if (gDialog.PreviewImage)
  {
    // get converted images if necessare
    // Image loading has completed -- we can get actual width
    previewActualWidth  = gDialog.PreviewImage.offsetWidth;
    previewActualHeight = gDialog.PreviewImage.offsetHeight;
    if (!gSizeState.actualSize.width || gSizeState.actualSize.width == 0)
      gSizeState.actualSize.width = unitHandler.getValueOf(previewActualWidth, 'px');
    if (!gSizeState.actualSize.height || gSizeState.actualSize.height == 0)
      gSizeState.actualSize.height = unitHandler.getValueOf(previewActualHeight, 'px');
    gSizeState.width = gSizeState.width || gSizeState.actualSize.width;
    gSizeState.height = gSizeState.height || gSizeState.actualSize.height;
    gSizeState.isCustomSize = false;
    gSizeState.update();
//    setImageSizeFields(previewActualWidth, previewActualHeight, unitHandler.currentUnit);
    if (gVideo)
      dump("PreviewImageLoaded reached for video file!\n");
    var bReset = false;

    if (previewActualWidth && previewActualHeight)
    {
      // Use actual size or scale to fit preview if either dimension is too large
      var width = previewActualWidth;
      var height = previewActualHeight;
      if (previewActualWidth > gPreviewImageWidth)
      {
          width = gPreviewImageWidth;
          height = previewActualHeight * (gPreviewImageWidth / previewActualWidth);
      }
      if (height > gPreviewImageHeight)
      {
        height = gPreviewImageHeight;
        width = previewActualWidth * (gPreviewImageHeight / previewActualHeight);
      }
      gDialog.PreviewImage.width = width;
      gDialog.PreviewImage.height = height;

      gDialog.PreviewWidth.setAttribute("value", Math.round(previewActualWidth)+" pixels");
      gDialog.PreviewHeight.setAttribute("value", Math.round(previewActualHeight)+" pixels");

      gDialog.PreviewSize.collapsed = false;
      gDialog.ImageHolder.collapsed = false;
      setContentSize(previewActualWidth, previewActualHeight);


      if (document.getElementById("actual").selected || bReset)
      {
        setActualOrDefaultSize();
      }

      doDimensionEnabling();
      SetSizeWidgets( Math.round(unitHandler.getValueAs(gDialog.frameWidthInput.value,"px")),
                      Math.round(unitHandler.getValueAs(gDialog.frameHeightInput.value,"px")) );
    }

    // if the image is svg, we need to modify it to make it resizable.
  //  if (isSVGFile)
     if (false)
    {
      var svg = gDialog.PreviewImage.contentDocument.documentElement;
      if (svg.hasAttribute("height") && svg.hasAttribute("width") && !svg.hasAttribute("viewBox"))
      {
        var currentUnit = unitHandler.currentUnit;
        var width = svg.getAttribute("height");
        var numregexp = /(\d+\.*\d*)([A-Za-z]*$)/;
        var arr = numregexp.exec(width);
        if (arr) {
          unitHandler.setCurrentUnit(arr[2]);
          width = unitHandler.getValueAs(arr[1],"px");
          width = Math.round(width);
        }
        var height = svg.getAttribute("width");
        var arr = numregexp.exec(height);
        if (arr)
        {
          unitHandler.setCurrentUnit(arr[2]);
          height = unitHandler.getValueAs(arr[1],"px");
          height = Math.round(height);
        }
        svg.setAttribute("viewBox", "0 0 "+height+" "+width);
        svg.setAttribute("width","100%");
        svg.setAttribute("height","100%");
      }
    }
  }
}

function isVideoSource(srcFile)
{
  var extension = getExtension(imageSrc);
  if (videoTypes.indexOf(extension) >= 0)
    return true;
  return false;
}

function LoadPreviewImage(importName, srcName)
{
  var unitHandler = new UnitHandler(gEditor);
  var unit = document.getElementById('frameUnitMenulist').value;
  unitHandler.initCurrentUnit(unit);
  if (!importName || !srcName) return;
  var imageSrc;
  var importSrc;
  if (!gPreviewImageNeeded || gIsGoingAway)
    return;


  gDialog.PreviewSize.collapsed = true;

  try {

    var IOService = msiGetIOService();
    if (IOService)
    {
      // We must have an absolute URL to preview it or remove it from the cache
      imageSrc = msiMakeAbsoluteUrl(importName);

      if (GetScheme(imageSrc))
      {
        var uri = IOService.newURI(imageSrc, null, null);
        if (uri)
        {
          var imgCacheService = Components.classes["@mozilla.org/image/cache;1"].getService();
          var imgCache = imgCacheService.QueryInterface(Components.interfaces.imgICache);

          // This returns error if image wasn't in the cache; ignore that
          imgCache.removeEntry(uri);
        }
      }
    }
  } catch(e) {}
    // We can't preview certain vector graphics, so build the bitmap equivalents first
  if (gDialog.PreviewImage)
    removeEventListener("load", PreviewImageLoaded, true);

  if (gDialog.ImageHolder.firstChild)
    gDialog.ImageHolder.removeChild(gDialog.ImageHolder.firstChild);

  var prevNodeName = "html:object";
  if (gVideo)
    prevNodeName = "html:embed";
  gDialog.PreviewImage = document.createElementNS("http://www.w3.org/1999/xhtml", prevNodeName);
  if (gDialog.PreviewImage) // && gInsertNewImage)
  {
    var docUrlString = msiGetDocumentBaseUrl();
    var docurl = msiURIFromString(docUrlString);
    var documentDir = msiFileFromFileURL(docurl);
    // set the src before appending to the document -- see bug 198435 for why
    // this is needed.
    gDialog.PreviewImage.setAttribute('copiedSrcUrl', importName);
    graphicsConverter.ensureTypesetGraphicForElement(gDialog.PreviewImage, documentDir);

    var eventStr = "load";
    gDialog.PreviewImage.addEventListener("load", PreviewImageLoaded, true);
    if (gVideo)
      gDialog.PreviewImage.src = imageSrc;
    else
      gDialog.PreviewImage.data = imageSrc;

    var extension = getExtension(srcName);
    var dimensions;
    var intermediate;
    var internalFile = msiFileFromFileURL(msiURIFromString(msiMakeAbsoluteUrl(srcName)));
    var leafname = internalFile.leafName;
    dimensions = graphicsConverter.readSizeFromVectorFile(internalFile, extension);
    if (dimensions == null) { 
      // if (extension === 'wmf' || extension === 'emf' || extension === 'pdf') {
      //   internalFile = internalFile.parent.parent;
      //   internalFile.append('gcache');
      //   internalFile.append((leafname.replace(extension,'png')));
      //   dimensions == graphicsConverter.readSizeFromVectorFile(internalFile, 'pdf');
      // }
      // if (dimensions == null) {
        dimensions = {width : gDialog.PreviewImage.offsetWidth, height : gDialog.PreviewImage.offsetHeight, units : 'px'};
      // }
      // else if (dimensions.unit === '') dimensions.unit = 'px';

    }
  
    intermediate = unitHandler.getValueOf(dimensions.width, dimensions.unit);
    gConstrainWidth = gActualWidth = unitHandler.getValueAs(intermediate, 'px');
    intermediate = unitHandler.getValueOf(dimensions.height, dimensions.unit);
    gConstrainHeight = gActualHeight = unitHandler.getValueAs(intermediate, 'px');
    adjustObjectForFileType(gDialog.PreviewImage, extension);
    if (gVideo)
    {
      gDialog.PreviewImage.setAttribute("controller", "false");
      gDialog.PreviewImage.setAttribute("showlogo", "true");
      gDialog.PreviewImage.setAttribute("autoplay", "false");
      gDialog.PreviewImage.setAttribute("postdomevents", "true");
    }
    gDialog.ImageHolder.appendChild(gDialog.PreviewImage);
    gPreviewImageNeeded = false;
  }
}

var pdfSetupStr = "#toolbar=0&statusbar=0&navpanes=0";

function adjustObjectForFileType(imageNode, extension)
{
  var ext = extension.toLowerCase();
  if (gVideo)
  {
    msiEditorEnsureAttributeOrParam(imageNode, "scale", "aspect", null);
    var mimeService = Components.classes["@mozilla.org/mime;1"].getService(Components.interfaces.nsIMIMEService);
    var mimeType = mimeService.getTypeFromExtension(ext);
    if (mimeType && (mimeType.length > 0))
      imageNode.setAttribute("type", mimeType);
  }
}


function setActualOrDefaultSize()
{
  var unitHandler = new UnitHandler(gEditor);
  var prefStr = "";
  var bUseDefaultWidth, bUseDefaultHeight;
  var width, height;
  try {
    bUseDefaultWidth = GetBoolPref("swp.graphics.usedefaultwidth");
  }
  catch(ex)
  {
    bUseDefaultWidth = false; dump("Exception getting pref swp.graphics.usedefaultwidth: " + ex + "\n");
  }

  try {
    bUseDefaultHeight = GetBoolPref("swp.graphics.usedefaultheight");
  }
  catch(ex) {
    bUseDefaultHeight = false; dump("Exception getting pref swp.graphics.usedefaultheight: " + ex + "\n");
  }

  if (bUseDefaultWidth || bUseDefaultHeight)
  {
    gDialog.sizeRadioGroup.selectedItem = document.getElementById("custom");
    try {prefStr = GetStringPref("swp.graphics.units");}
    catch(ex) {prefStr = ""; dump("Exception getting pref swp.graphics.units: " + ex + "\n");}
    if (prefStr.length)
      gDefaultUnit = prefStr;
    unitHandler.setCurrentUnit(gDefaultUnit);
    gDialog.frameUnitMenulist.value = gDefaultUnit;
  }
  if (bUseDefaultWidth)
  {
    gDialog.autoWidthCheck.checked = false;
    try {
      prefStr = GetStringPref("swp.graphics.hsize");
    }
    catch(ex) {
      prefStr = ""; 
      dump("Exception getting pref swp.graphics.hsize: " + ex + "\n");
    }
    if (prefStr.length)
    {
      width = unitHandler.getValueFromString( prefStr, gDefaultUnit );
      gDefaultWidth = Math.round(unitHandler.getValueAs(width, "px"));
      gDialog.frameWidthInput.value = width;
    }
    if (bUseDefaultHeight)
    {
      try {
        prefStr = GetStringPref("swp.graphics.vsize");
      }
      catch(ex) {
        prefStr = ""; dump("Exception getting pref swp.graphics.vsize: " + ex + "\n");
      }
      if (prefStr.length)
      {
        height = unitHandler.getValueFromString( prefStr, gDefaultUnit );
        gDefaultHeight = Math.round(unitHandler.getValueAs(height,"px"));
      }
      gDialog.autoHeightCheck.checked = false;
      gDialog.constrainCheckbox.checked = false;
      gDialog.frameHeightInput.value = height;
    }
    else
    {
      gDialog.autoHeightCheck.checked = true;
      gDialog.constrainCheckbox.checked = true;
      gDialog.constrainCheckbox.disabled = false;
      constrainProportions( "frameWidthInput", "frameHeightInput", null );
    }
  }
  else if (bUseDefaultHeight)
  {
    try {prefStr = GetStringPref("swp.graphics.vsize");}
    catch(ex) {prefStr = ""; dump("Exception getting pref swp.graphics.vsize: " + ex + "\n");}
    if (prefStr.length)
    {
      height = unitHandler.getValueFromString( prefStr, gDefaultUnit );
      gDefaultHeight = Math.round(unitHandler.getValueAs(height,"px"));
    }
    gDialog.autoWidthCheck.checked = true;
    gDialog.autoHeightCheck.checked = false;
    gDialog.constrainCheckbox.disabled = false;
    gDialog.constrainCheckbox.checked = true;
    gDialog.frameHeightInput.value = height;
    constrainProportions( "frameHeightInput", "frameWidthInput", null );
  }
  else
  {
    setActualSize();
  }
}

function addParamChild(imageNode, name, val)
{
  var param = document.createElementNS(xhtmlns, "param");
  param.setAttribute("name", name);
  param.setAttribute("value", val);
  param.setAttribute("valuetype", "data");
  imageNode.appendChild(param);
}


function readTotalExtraWidth(unit)
{
  var unitHandler = new UnitHandler(gEditor);
  var totWidth = 0;
//  totWidth += 2* unitHandler.getValueAs(Number(gDialog.marginInput.left.value), "px");
  totWidth += 2 * unitHandler.getValueAs(Number(gDialog.borderInput.left.value), "px");
  totWidth += 2 * unitHandler.getValueAs(Number(gDialog.paddingInput.left.value), "px");
  return totWidth;
}

function readTotalExtraHeight(unit)
{
  var unitHandler = new UnitHandler(gEditor);
  var totHeight = 0;
//  totHeight += 2 * unitHandler.getValueAs(Number(gDialog.marginInput.top.value), "px");
  totHeight += 2 * unitHandler.getValueAs(Number(gDialog.borderInput.left.value), "px");
  totHeight += 2 * unitHandler.getValueAs(Number(gDialog.paddingInput.left.value), "px");
  return totHeight;
}

function setContentSize(width, height)  // width and height are the size of the image in pixels
{
  scaledWidth = Math.round(scale*width);
  if (scaledWidth == 0) scaledWidth = scaledWidthDefault;
  scaledHeight = Math.round(scale*height);
  if (scaledHeight == 0) scaledHeight = scaledHeightDefault;
  setStyleAttributeByID("content", "width", scaledWidth + "px");
  setStyleAttributeByID("content", "height", scaledHeight + "px");
  updateDiagram("margin");
}


function doOverallEnabling()
{
  var enabled = TrimString(gDialog.srcInput.value) != "";

  SetElementEnabled(gDialog.OkButton, enabled);
  SetElementEnabledById("AdvancedEditButton1", enabled);
  // doDimensionEnabling();
}

function shouldShowErrorMessage()
{
  return (!gErrorMessageShown && !importTimerHandler.errorMessageShown());
}

function activateVideoStartControls(checkbox)
{
  var bEnable = !checkbox.checked;
  enableControlsByID(vidStartTimeControlIds, bEnable);
}

function activateVideoEndControls(checkbox)
{
  var bEnable = !checkbox.checked;
  enableControlsByID(vidEndTimeControlIds, bEnable);
}

function displayImportErrorMessage(fileName)
{
  if (!shouldShowErrorMessage())
    return;

  var theMsg = "imageProps.couldNotImport";
  var msgParams = {file: fileName};
  var msgString = msiGetDialogString(theMsg, msgParams);
  var titleStr = msiGetDialogString("imageProps.importErrorTitle", msgParams);
  gErrorMessageShown = true;
  AlertWithTitle(titleStr, msgString);
}


// Get data from widgets, validate, and set for the global element
//   accessible to AdvancedEdit() [in msiEdDialogCommon.js]
function ValidateImage()
{
  var editor = msiGetEditor(gEditorElement);
  if (!editor)
    return false;

  if (!gDialog.srcInput.value)
  {
    if (shouldShowErrorMessage())
      AlertWithTitle(null, GetString("MissingImageError"));
    gDialog.srcInput.focus();
    return false;
  }

  var alt = TrimString(gDialog.altTextInput.value);

  imageElement.setAttribute("alt", alt);

  if (gDialog.keyInput)
  {
    var bUnchanged = false;
    if (!gOriginalKey || !gOriginalKey.length)
      bUnchanged = (!gDialog.keyInput.value || !gDialog.keyInput.value.length);
    else if (gDialog.keyInput.value.length)
      bUnchanged = (gDialog.keyInput.value == gOriginalKey);
    if (!bUnchanged && gDialog.keyInput.value && gDialog.keyInput.value.length && (gDialog.keyInput.controller.searchStatus == 4))  //found a match!
    {
      msiPostDialogMessage("dlgErrors.markerInUse", {markerString : gDialog.keyInput.value});
      gDialog.keyInput.focus();
      return false;
    }
  }

  var width = "";
  var height = "";

  if (!(document.getElementById("actual").selected))
  {
    // Get user values for width and height
    width = msiValidateNumber(gDialog.frameWidthInput, gDialog.widthUnitsMenulist, 1, gMaxPixels,
                           imageElement, widthAtt, false, true);
    if (gValidationError)
      return false;

    height = msiValidateNumber(gDialog.frameHeightInput, gDialog.heightUnitsMenulist, 1, gMaxPixels,
                            imageElement, heightAtt, false, true);
    if (gValidationError)
      return false;
  }

  // We always set the width and height attributes, even if same as actual.
  //  This speeds up layout of pages since sizes are known before image is loaded
  if (!width)
    width = gActualWidth;
  if (!height)
    height = gActualHeight;

  var srcChanged = (gSrcUrl != gInitialSrc);
  if (width)
    imageElement.setAttribute(widthAtt, width);
  else if (srcChanged)
    editor.removeAttributeOrEquivalent(imageElement, widthAtt, true);

  if (height)
    imageElement.setAttribute(heightAtt, height);
  else if (srcChanged)
    editor.removeAttributeOrEquivalent(imageElement, heightAtt, true);
  return true;
}


  // var unitHandler = new UnitHandler(gEditor);
  // try {
  //   var isSVGFile = /\.svg$/.test(event.data);
  //   if (isSVGFile)
  //   {
  //     var svg = event.target.contentDocument.documentElement;
  //     if (svg.hasAttribute("height") && svg.hasAttribute("width") && !svg.hasAttribute("viewBox"))
  //     {
  //       var currentUnit = unitHandler.currentUnit;
  //       var width = svg.getAttribute("height");
  //       var numregexp = /(\d+\.*\d*)([A-Za-z]*$)/;
  //       var arr = numregexp.exec(width);
  //       if (arr) {
  //         unitHandler.setCurrentUnit(arr[2]);
  //         width = unitHandler.getValueAs(arr[1],"px");
  //         width = Math.round(width);
  //       }
  //       var height = svg.getAttribute("width");
  //       var arr = numregexp.exec(height);
  //       if (arr)
  //       {
  //         unitHandler.setCurrentUnit(arr[2]);
  //         height = unitHandler.getValueAs(arr[1],"px");
  //         height = Math.round(height);
  //       }
  //       svg.setAttribute("viewBox", "0 0 "+height+" "+width);
  //       svg.setAttribute("width","100%");
  //       svg.setAttribute("height","100%");
  //     }
  //   }
  // }
  // catch (e) {

//   // }
// }

function isEnabled(element)
{
  if (!element) return false;
  return(!element.hasAttribute("disabled"));
}


function onAccept()
{

  /* The variables imageElement and wrapperElement point to the image element 
    we are revising, and possibly the wrapper element if there was one. We want 
    to preserve these in the revising case because they might contain attributes 
    the user added. We can tell if we are in the revising case because then 
    gInsertNewImage is false. That there was a wrapper element originally does 
    not imply that there will or will not be one now, since the user may have 
    added or deleted a caption.
  */
  // Use this now (default = false) so Advanced Edit button dialog doesn't trigger error message
  var unitHandler = new UnitHandler();
  var brNode;
  gIsGoingAway = true;
  importTimerHandler.stopLoading();


  gDoAltTextError = true;
  if (ValidateData())
  {
    //var editorElement = msiGetParentEditorElementForDialog(window);
    //var editor = msiGetEditor(gEditorElement);
    var product;
#ifdef PROD_SWP
    product = "swp";
#endif
#ifdef PROD_SW
    product = "sw";
#endif
#ifdef PROD_SNB
    product = "snb";
#endif

    graphicsConverter.init(window, getDocumentGraphicsDir('').parent, product);
    var unit = document.getElementById("frameMenuUnitlist").value;
    var width = 0;
    var height = 0;
    var wInput = document.getElementById("frameWidthInput");
    var hInput = document.getElementById("frameHeightInput");
    // The next elements are from msiFrameOverlay
    if (!wInput.hasAttribute('disabled')) width = wInput.value;
    if (!hInput.hasAttribute('disabled')) height = hInput.value;
    unitHandler.initCurrentUnit(unit);

    gEditor.beginTransaction();
    try
    {
      var tagname = gVideo ? "embed" : "object";
      var captionloc = 'none';
      var gCaptionNode = GetCaptionNode(wrapperElement);
      if (isEnabled(gDialog.captionLocation)) captionloc = gDialog.captionLocation.value;
      if (captionloc === '') captionloc = 'none';
      var bHasCaption = (captionloc.length > 0 && captionloc !== 'none');

      // caption dance: If there was a caption and the user specifies none, delete the caption
      if (gCaptionNode && captionloc === 'none') {
        gEditor.deleteNode(gCaptionNode);
        gCaptionNode = null;
      }
      else if (!gCaptionNode && captionloc !== 'none') {
        gCaptionNode = gEditor.createElementWithDefaults('imagecaption');
        brNode = editor.document.createElement('br');
        gCaptionNode.appendChild(brNode);
      }
      if (gCaptionNode && isEnabled(document.getElementById("keyInput"))) {
        if (document.getElementById("keyInput").value != "")
          gCaptionNode.setAttribute("key", document.getElementById("keyInput").value);
        else gCaptionNode.removeAttribute("key");
      }
      else if (gCaptionNode) gCaptionNode.removeAttribute("key");

      var posInParent;

      
      if (gInsertNewImage)
      {
        if (imageElement == null) imageElement = gEditor.createElementWithDefaults(tagname);
//        imageElement.addEventListener("load", imageLoaded, true);
      }
      var src = gSrcUrl;
      imageElement.setAttribute("src", src);
      imageElement.setAttribute("data", src);
      if (wrapperElement == null) {
        wrapperElement = gEditor.createFrameWithDefaults("image", true, sel.anchorNode, sel.anchorOffset);
      }
      if (bHasCaption) {
        wrapperElement.appendChild(gCaptionNode);
        msiEditorEnsureElementAttribute(wrapperElement, "captionloc", captionloc, null);
      }

      if (gInsertNewImage)
        wrapperElement.appendChild(imageElement);
      else
      {
        // If we are revising an image element and adding a wrapper element where there wasn't one before, we
        // need to remove the margin, padding, border, background color style attributes from the image.
        var imageParent = imageElement.parentNode;
        var style;
        if (imageParent.tagName !== "msiframe") {
          gEditor.deleteNode(imageElement);
          wrapperElement.appendChild(imageElement);
          style = imageElement.getAttribute("style");
          style = style.replace(/margin[^;]+;/,'');
          style = style.replace(/border[^;]+;/,'');
          style = style.replace(/background[^;]+;/,'');
          style = style.replace(/padding[^;]+;/,'');
          style = style.replace(/caption-side[^;]+;/,'');
          if (captionloc != 'none')
            style = style + 'caption-side: ' + captionloc + ';';
          imageElement.setAttribute("style", style);
          gEditor.insertNode(wrapperElement, imageParent, posInParent);
        }
      }

      if (gInsertNewImage) {
        gEditor.insertElementAtSelection(wrapperElement, true);
      }

      var extension = getExtension(gDialog.srcInput.value).toLowerCase();
      adjustObjectForFileType(imageElement, extension);

      msiEnsureElementPackage(imageElement,"graphics",null);
      if (gVideo)
      {
        setVideoSettingsToElement(imageElement, null);
        msiEnsureElementPackage(imageElement,"hyperref",null);
        msiEnsureElementPackage(imageElement,"movie15",null);
      }

      if (gConstrainWidth > 0)
        msiEditorEnsureElementAttribute(imageParent, "naturalwidth", String(unitHandler.getValueOf(gActualWidth, "px")), null);
      if (gConstrainHeight > 0)
        msiEditorEnsureElementAttribute(imageParent, "naturalheight", String(unitHandler.getValueOf(gActualHeight, "px")), null);
      imageElement.setAttribute(widthAtt, unitHandler.getValueAs(width, 'px'));
      imageElement.setAttribute(heightAtt, unitHandler.getValueAs(height, 'px'));

      msiEditorEnsureElementAttribute(imageElement, "originalSrcUrl", gOriginalSrcUrl, null);
      msiEditorEnsureElementAttribute(imageElement, "copiedSrcUrl", gCopiedSrcUrl, null);

      setFrameAttributes(wrapperElement, imageElement, null, bHasCaption);
      if (bHasCaption) {
        if (captionloc && captionloc !== 'none') setStyleAttributeOnNode(wrapperElement||imageElement, "caption-side", captionloc, gEditor);
      }

      if (gVideo)
      {
        //msiEditorEnsureElementAttribute(imageElement, "autoplay", "false", null);
        //msiEditorEnsureElementAttribute(imageElement, "controller", "true", null);
        msiEditorEnsureAttributeOrParam(imageElement, "showlogo", "false", null);
        msiEditorEnsureAttributeOrParam(imageElement, "scale", "aspect", null);
        msiEditorEnsureElementAttribute(imageElement, "isVideo", "true", null);
      }
    }
    catch (e)
    {
      dump(e);
    }

    gEditor.endTransaction();

    SaveWindowLocation();
    return true;
  }
  else dump("Validation FAILED\n");
  gIsGoingAway = false;
  gDoAltTextError = false;
  return false;
}

function onCancel()
{
  gIsGoingAway = true;
  // if (imageElement != null) imageElement.remove();
  // if (wrapperElement != null) wrapperElement.remove();
  importTimerHandler.stopLoading();
  SaveWindowLocation();
  return true;
}

function initKeyList()
{

  gDialog.markerList = new msiKeyMarkerList(window);
  gDialog.markerList.setUpTextBoxControl(gDialog.keyInput);
}

function getGraphicsImportFilterString()
{
  return;
  var convertData = graphicsConverter.getConvertibleFileTypes(window);
  var typeArray = [];
  for (var jx = 0; jx < graphicsConverter.nativeGraphicTypes.length; ++jx)
    typeArray.push("*." + graphicsConverter.nativeGraphicTypes[jx]);
  var newType;
  for (var ix = 0; ix < convertData.length; ++ix)
  {
    newType = TrimString(convertData[ix]);
    if (newType.length > 0)
    {
      newType = "*." + newType;
      if (typeArray.indexOf(newType) < 0)
        typeArray.push(newType);
    }
  }
  return typeArray.join("; ");
}

function getVideoImportFilterString()
{
  var mimeService = Components.classes["@mozilla.org/mime;1"].getService(Components.interfaces.nsIMIMEService);
  var mimeType;
  var typeArray = [];
  var newType;
  for (var ii = 0; ii < videoTypes.length; ++ii)
  {
    try {mimeType = mimeService.getTypeFromExtension(videoTypes[ii]);}
    catch(exc) {mimeType = ""; dump("Exception calling nsIMIMEService.getTypeFromExtension for extension " + videoTypes[ii] + ": " + exc + "\n");}
    if (mimeType && (mimeType.length > 0) && (mimeType != "text/plain"))
    {
      //Now look to see if we have a registered plugin that can handle this mimetype
      newType = "*." + videoTypes[ii];
      if (typeArray.indexOf(newType) < 0)
        typeArray.push(newType);
    }
  }
  return typeArray.join("; ");
}

