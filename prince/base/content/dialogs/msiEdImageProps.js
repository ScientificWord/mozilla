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

var gDialog;
var gEditorElement;
var gEditor;
Components.utils.import("resource://app/modules/unitHandler.jsm");
Components.utils.import("resource://app/modules/graphicsConverter.jsm");
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

/* Finding the initial graphics size and other properties can be rather complex.
The input, in order of increasing usefulness, is:

1. Base defaults built into the program. These are used only if no other information is available. It makes sense for this to be null -- there is nothing in any of the fields.

2. The preferences allow the user to set default width and/or default height, and a boolean for each of these to determine if the default is to be used.

3. Once an image has been chosen, we can find the width, height, and aspect ratio.

4. If we are revising an <img> or <object>, then we an read the width and/or height if they have been set, and if we should preserve the aspect ratio. Also, frame characteristics are then known.

For finding the custom width and height and auto- checkboxes and the preserve-aspect-ratio checkbox, proceed as follows.

Case I. New image.
  1. Set all of these to blank. This step is skipped in the drag-drop case.
  2. Once an image is chosen and its size is known
    a. Set size to 'Custom'. If the preferences height and width are both given and enabled, then set those, turn off Auto for each, and set Preserve aspect ratio to false.
    b, One of height or width is given and enabled, then fill that field and set the other to Auto, and set Preserve aspect ratio to true. Compute the missing height or width using the actual image's aspect ratio and the given width or height.
    c. If neither height nor width is given and enabled, then set Actual size to true.

    The user can now change any of these settings.
Case II. Revising an image.
  1. In this case current settings for the object trump the defaults.
  2. If only one of the width or height is given as an attribute, follow step 2 above.
  3. If both are given, set the size to 'Custom' and fill in the width and height and set auto to false for both. The preserve aspect ration should be set if there is an attribute for it.

For the frame properties, the values should come from the defaults in the preferences, unless we are revising, in which case they come from the object being revised.
*/


function setImageSizeFields(imageWidth, imageHeight, dialogUnits)
{
  var prefWidth, prefHeight, prefWidthEnabled, prefHeightEnabled;
  var aspectRatio, prefUnits;
  var autoWidth = true;
  var autoHeight = true;
  var prefBranch = GetPrefs();
  var customValue = false;
  var w = null;
  var h = null;
  var unitHandler = new  UnitHandler();

  prefUnits = prefBranch.getCharPref("swp.graphics.units");
  unitHandler.initCurrentUnit(dialogUnits);

  prefWidth = prefBranch.getCharPref("swp.graphics.hsize");
  prefWidthEnabled = prefBranch.getBoolPref("swp.graphics.usedefaultwidth");
  if (!prefWidthEnabled) prefWidth = null;
  prefHeight = prefBranch.getCharPref("swp.graphics.vsize");
  prefHeightEnabled = prefBranch.getBoolPref("swp.graphics.usedefaultheight");
  if (!prefHeightEnabled) prefHeight = null;
  if (prefHeight && prefWidth) aspectRatio = prefHeight/prefWidth;
  document.getElementById("unitList").value = prefUnits;
//    customValue = true;
  if (prefWidth) {
    autoWidth = false;
    w = unitHandler.getValueOf(prefWidth, prefUnits);
  }
  document.getElementById("frameWidthInput").value = w;
  if (prefHeight) {
    autoHeight = false;
    h = unitHandler.getValueOf(prefHeight, prefUnits);
  }
  document.getElementById("frameHeightInput").value = h;
  if (imageWidth && imageHeight) {
    aspectRatio = imageHeight/imageWidth;
    if (!prefWidth) {
      if (prefHeight) {
        // set using aspectRatio
        document.getElementById("frameWidthInput").value = h/aspectRatio;
      }
    }
    if (!prefHeight) {
      if (prefWidth) {
        // set using aspectRatio
        document.getElementById("frameHeightInput").value = w*aspectRatio;
      }
    }
  }
  document.getElementById("autoHeight").checked = autoHeight;
  document.getElementById("autoWidth").checked = autoWidth;
  if (autoWidth) document.getElementById("frameWidthInput").setAttribute("disabled", true)
  else document.getElementById("frameWidthInput").removeAttribute("disabled");
  if (autoHeight) document.getElementById("frameHeightInput").setAttribute("disabled", true)
  else document.getElementById("frameHeightInput").removeAttribute("disabled");
}
// function setFrameSizeFromExisting(node) is defined in msiFrameOverlay.js

function Startup()
{
  //gActiveEditorElement = msiGetParentEditorElementForDialog(window);
  //gActiveEditor = msiGetTableEditor(gActiveEditorElement);
  var existingImage = false;
  gEditorElement = msiGetParentEditorElementForDialog(window);
  gEditor = msiGetEditor(gEditorElement);
  if (!gEditor)
  {
    window.close();
    return;
  }
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
    imageElement = window.arguments[0].mNode;
    if (imageElement) existingImage = true;
    if (gVideo)
    {
      document.getElementById("isVideo").removeAttribute("hidden");
      document.title = document.getElementById("videoTitle").textContent;
      forceIsImport(false);  //Set import/reference to be reference by default for video files
    }
  }

  var tagName = gVideo ? "embed" : "object";
  if (imageElement)  // if the image already exists, search for a frame.
  {
    if (imageElement.nodeName !== "msiframe")
    {
      if (imageElement.parentNode.nodeName === "msiframe")
      {
        wrapperElement = imageElement.parentNode;
      }
      else
        wrapperElement = null;  // no frame
    }
    else
    {
      var tagsToTry = gVideo ? ["embed", "object"] : ["object", "img", "embed"];
      wrapperElement = imageElement;
      var imgList = null;
      for (var iii = 0; (!imgList || !imgList.length) && (iii < tagsToTry.length); ++iii)
      {
        imgList = msiNavigationUtils.getChildrenByTagName(wrapperElement, tagsToTry[iii]);
      }
      if (!imgList || !imgList.length)
      {
        for (iii = 0; (!imgList || !imgList.length) && (iii < tagsToTry.length); ++iii)
        {
          imgList = wrapperElement.getElementsByTagName(tagsToTry[iii]);
        }
      }
      if (imgList && imgList.length)
        imageElement = imgList[0];
    }
  }
  if (!gVideo && !imageElement)
  {
    try {
      imageElement = getSelectionParentByTag(gEditor,"input");
      if (!imageElement || imageElement.getAttribute("type") != "image") {
        // Get a single selected image element
        imageElement = getSelectionParentByTag(gEditor,tagName);
      }
    } catch (e) {}
  }
  if (imageElement)
  {
    // We found an element and don't need to insert one
    if (imageElement.hasAttribute("src") || imageElement.hasAttribute("data"))
    {
      gInsertNewImage = false;
      tagName = msiGetBaseNodeName(imageElement);
      if (imageElement.hasAttribute("src")) {
        gSrcUrl  = imageElement.getAttribute("src");
      } else {
        gDialog.srcInput.value = imageElement.getAttribute("data");
      }
    }
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
  // if inserting a new image, create elements now.
  if (imageElement == null) {
    imageElement = gEditor.createElementWithDefaults(tagName);
    // Don't create a wrapper element until needed
  }

  initFrameTab(gDialog, wrapperElement||imageElement, gInsertNewImage, imageElement);

  initKeyList();



  // We only need to test for this once per dialog load
  gHaveDocumentUrl = msiGetDocumentBaseUrl();

  InitDialog();
  if (!existingImage)
    setImageSizeFields(null,null, frameUnitHandler.currentUnit);
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

function InitDialog()
{
  InitImage();
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

function InitImage()
{
  gDialog.srcInput.value = gCopiedSrcUrl || "";
  var width = 0;
  var height = 0;
  var pixelWidth = 0;
  var pixelHeight = 0;

  loadDefaultsFromPrefs();

  if (!gInsertNewImage)
  {
    // We found an element and don't need to insert one
    //trim off units at end
    var re = /[a-z]*/gi;
    var widthStr = "";
    var heightStr = "";
    if (imageElement.hasAttribute("units"))
    {
      var unit = imageElement.getAttribute("units");
      frameUnitHandler.setCurrentUnit(unit);
      gDialog.frameUnitMenulist.value = unit;
    }
    if (imageElement.hasAttribute(widthAtt))
    {
      widthStr = imageElement.getAttribute(widthAtt);
      width = frameUnitHandler.getValueFromString(widthStr);
      pixelWidth = Math.round(frameUnitHandler.getValueAs(width,"px"));
      width = unitRound(width);
    }
    if (!width)
    {
      widthStr = msiGetHTMLOrCSSStyleValue(gEditorElement, imageElement, "width", "width");
      width = unitRound(frameUnitHandler.getValueFromString(widthStr, "px"));
      widthStr = widthStr.replace(re,"");
      pixelWidth = Math.round(Number(widthStr));
    }
    if (imageElement.hasAttribute(heightAtt))
    {
      heightStr = imageElement.getAttribute(heightAtt);
      height = frameUnitHandler.getValueFromString(heightStr);
      pixelHeight = Math.round(frameUnitHandler.getValueAs(height,"px"));
      height = unitRound(height);
    }
    if (!height)
    {
      heightStr = msiGetHTMLOrCSSStyleValue(gEditorElement, imageElement, "height", "height");
      height = unitRound(frameUnitHandler.getValueFromString(heightStr, "px"));
      heightStr = heightStr.replace(re,"");
      pixelHeight = Math.round(Number(heightStr));
    }
    if (imageElement.hasAttribute("src") || imageElement.hasAttribute("data"))
    {
      gPreviewImageNeeded = true;
      var natWidth = imageElement.getAttribute("naturalWidth");
      var natHeight = imageElement.getAttribute("naturalHeight");
      if (natWidth)
        gActualWidth = gConstrainWidth = Math.round(frameUnitHandler.getValueAs(frameUnitHandler.getValueFromString(natWidth), "px"));
      if (!gActualWidth)
        gActualWidth  = gConstrainWidth = imageElement.offsetWidth - Math.round(readTotalExtraWidth("px"));
      if (natHeight)
        gActualHeight = gConstrainHeight = Math.round(frameUnitHandler.getValueAs(frameUnitHandler.getValueFromString(natHeight), "px"));
      if (!gActualHeight)
        gActualHeight = gConstrainHeight = imageElement.offsetHeight - Math.round(readTotalExtraHeight("px"));
    }
  }
  else  // existing image
  {
    // var defaultUnitStr, defaultWidthStr, defaultHeightStr;
    // try
    // {defaultUnitStr = GetStringPref("swp.graphics.units");}
    // catch(ex) {defaultUnitStr = ""; dump("Exception getting pref swp.graphics.units: " + ex + "\n");}
    // if (defaultUnitStr.length)
    // {
    //   gDefaultUnit = defaultUnitStr;
    //   frameUnitHandler.setCurrentUnit(gDefaultUnit);
    //   gDialog.frameUnitMenulist.value = gDefaultUnit;
    // }

    // try
    // {defaultWidthStr = GetStringPref("swp.graphics.hsize");}
    // catch(ex) {defaultWidthStr = ""; dump("Exception getting pref swp.graphics.hsize: " + ex + "\n");}
    // if (defaultWidthStr.length)
    //   width = frameUnitHandler.getValueFromString( defaultWidthStr, gDefaultUnit );
    // gDefaultWidth = Math.round(frameUnitHandler.getValueAs(width, "px"));

    // try
    // {defaultHeightStr = GetStringPref("swp.graphics.vsize");}
    // catch(ex) {defaultHeightStr = ""; dump("Exception getting pref swp.graphics.vsize: " + ex + "\n");}
    // if (defaultHeightStr.length)
    //   height = frameUnitHandler.getValueFromString( defaultHeightStr, gDefaultUnit );
    // gDefaultHeight = Math.round(frameUnitHandler.getValueAs(height,"px"));

  }

  if ((width > 0) || (height > 0))
    setWidthAndHeight(width, height, null);
  else if ((gActualHeight > 0)||(gActualWidth > 0))
    setWidthAndHeight(unitRound(frameUnitHandler.getValueOf(gActualWidth,"px")),
                      unitRound(frameUnitHandler.getValueOf(gActualHeight,"px")), null);
  else
    setWidthAndHeight(unitRound(frameUnitHandler.getValueOf(gDefaultWidth,"pt")),
                      unitRound(frameUnitHandler.getValueOf(gDefaultHeight,"pt")), null);

  LoadPreviewImage(null);
  // caption and key info comes from frameOverlay
  document.getElementById("captionLocation").value = gCaptionLoc;


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

  // dialog.border.value       = globalElement.getAttribute("border");
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
    elements[i].label = frameUnitHandler.getDisplayString(elements[i].value);
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
    nodes = parentNode.getElementsByTagName('caption');
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

// function findCaptionNodes(parentNode)
// {
//   var retData = {belowCaption : null, aboveCaption : null};
//   var theChildren = msiNavigationUtils.getSignificantContents(parentNode);
//   var position;
//   var parentPos = "below";
//   if (parentNode.hasAttribute("captionloc"))
//     parentPos = parentNode.getAttribute("captionloc");
//   for (var ii = 0; ii < theChildren.length; ++ii)
//   {
//     if (msiGetBaseNodeName(theChildren[ii]) == "imagecaption")
//     {
//       if (theChildren[ii].hasAttribute("position"))
//         position = theChildren[ii].getAttribute("position");
//       else
//         position = parentPos;
//       if (!position || (position != "above"))
//         position = parentPos;
//       position += "Caption";
//       retData[position] = theChildren[ii];
//     }
//   }
//   return retData;
// }

function getNodeChildrenAsString(aNode)
{
  var retStr = "";
  var serializer = new XMLSerializer();
  var nodeKids = msiNavigationUtils.getSignificantContents(aNode);
  for (var jx = 0; jx < nodeKids.length; ++jx)
    retStr += serializer.serializeToString(nodeKids[jx]);
  return retStr;
}

// function syncCaptionAndExisting(dlgCaptionStr, editor, imageObj, positionStr)
// {
//   var bChange = false;
//   var existingStr = "";
//   var currCaptionNode;
//   var currLoc = imageObj.getAttribute("captionloc");
//   if (!currLoc || !currLoc.length)
//     currLoc = "below";
//   var captionNodes = imageObj.getElementsByTagName("imagecaption");
//    if (captionNodes && captionNodes.length)
//     currCaptionNode = captionNodes[0];
//   if (currCaptionNode)
//     existingStr = getNodeChildrenAsString(currCaptionNode);
//   if (dlgCaptionStr.length)
//     msiEditorEnsureElementAttribute(imageObj, "captionloc", positionStr, editor);
//   else
//     msiEditorEnsureElementAttribute(imageObj, "captionloc", null, editor);  //This will remove the captionLoc attribute

//   bChange = (existingStr != dlgCaptionStr);
//   if (!bChange)
//   {
//     return;
//   }
//   if (dlgCaptionStr.length)
//   {
//     if (!currCaptionNode)
//     {
//       currCaptionNode = editor.document.createElementNS(xhtmlns, "imagecaption");
// //      currCaptionNode.setAttribute("position", positionStr);
//       editor.insertNode(currCaptionNode, imageObj, imageObj.childNodes.length);
//     }
//     else if (existingStr.length)
//     {
//       for (var jx = currCaptionNode.childNodes.length - 1; jx >= 0; --jx)
//         editor.deleteNode(currCaptionNode.childNodes[jx]);
//     }
//     editor.insertHTMLWithContext(dlgCaptionStr, "", "", "", null, currCaptionNode, 0, false);
//     msiEditorEnsureElementAttribute(imageObj, "captionloc", positionStr, editor);
//   }
//   else if (currCaptionNode)
//     editor.deleteNode(currCaptionNode);
// }

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
    try {
      url = msiURIFromString(fileName);
      gOriginalSrcUrl = decodeURI(url.spec);
      leafname = msiFileFromFileURL(url).leafName;
      file = msiFileFromFileURL(url);
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
    internalFile = graphicDir.clone(false);
    internalFile.append(leafname);
    if (!file) file = internalFile;
    internalName = graphicDir.leafName + "/" + internalFile.leafName;
    graphicsConverter.init(window, graphicDir.parent);
    importName = graphicsConverter.copyAndConvert(file, true);
    gSrcUrl = importName;
    if (!importName || !importName.length)
    {
      displayImportErrorMessage(fileName)
      fileName = "";
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
  window.openDialog("chrome://prince/content/msiGraphicsConversionDlg.xul", "graphicsConversionRunning", "chrome,close,resizable,titlebar,modal", importData);
  //Then collect data the dialog may have changed (notably if the user cancelled the conversion)
  importTimerHandler.importStatus = importData.mImportStatus;
  importTimerHandler.texImportStatus = importData.mTexImportStatus;

  importTimerHandler.checkFinalStatus();
}

function getGraphicsImportTargets(inputFile, mode)
{
  if (!mode || !mode.length)
    mode = "import";
  var graphicDir = getDocumentGraphicsDir(mode);
  if (!graphicDir.exists())
    graphicDir.create(1, 0755);
  graphicsConverter.init(window, graphicDir.parent);
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
  if (gDialog.PreviewImage)
  {
    // Image loading has completed -- we can get actual width
    gActualWidth  = gDialog.PreviewImage.offsetWidth;
    gActualHeight = gDialog.PreviewImage.offsetHeight;
    setImageSizeFields(gActualWidth, gActualHeight, frameUnitHandler.currentUnit);
    if (gVideo)
      dump("PreviewImageLoaded reached for video file!\n");
    var bReset = false;

    if (gActualWidth && gActualHeight)
    {
      // Use actual size or scale to fit preview if either dimension is too large
      var width = gActualWidth;
      var height = gActualHeight;
      if (gActualWidth > gPreviewImageWidth)
      {
          width = gPreviewImageWidth;
          height = gActualHeight * (gPreviewImageWidth / gActualWidth);
      }
      if (height > gPreviewImageHeight)
      {
        height = gPreviewImageHeight;
        width = gActualWidth * (gPreviewImageHeight / gActualHeight);
      }
      gDialog.PreviewImage.width = width;
      gDialog.PreviewImage.height = height;

      gDialog.PreviewWidth.setAttribute("value", Math.round(gActualWidth)+" pixels");
      gDialog.PreviewHeight.setAttribute("value", Math.round(gActualHeight)+" pixels");

      gDialog.PreviewSize.collapsed = false;
      gDialog.ImageHolder.collapsed = false;
      setContentSize(gActualWidth, gActualHeight);


      if (document.getElementById("actual").selected || bReset)
      {
        setActualOrDefaultSize();
      }

      doDimensionEnabling();
      SetSizeWidgets( Math.round(frameUnitHandler.getValueAs(gDialog.frameWidthInput.value,"px")),
                      Math.round(frameUnitHandler.getValueAs(gDialog.frameHeightInput.value,"px")) );
    }

    // if the image is svg, we need to modify it to make it resizable.
    if (isSVGFile)
    {
      var svg = gDialog.PreviewImage.contentDocument.documentElement;
      if (svg.hasAttribute("height") && svg.hasAttribute("width") && !svg.hasAttribute("viewBox"))
      {
        var currentUnit = frameUnitHandler.currentUnit;
        var width = svg.getAttribute("height");
        var numregexp = /(\d+\.*\d*)([A-Za-z]*$)/;
        var arr = numregexp.exec(width);
        if (arr) {
          frameUnitHandler.setCurrentUnit(arr[2]);
          width = frameUnitHandler.getValueAs(arr[1],"px");
          width = Math.round(width);
        }
        var height = svg.getAttribute("width");
        var arr = numregexp.exec(height);
        if (arr)
        {
          frameUnitHandler.setCurrentUnit(arr[2]);
          height = frameUnitHandler.getValueAs(arr[1],"px");
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

  if (gDialog.PreviewImage)
    removeEventListener("load", PreviewImageLoaded, true);

  if (gDialog.ImageHolder.firstChild)
    gDialog.ImageHolder.removeChild(gDialog.ImageHolder.firstChild);

  var prevNodeName = "html:object";
  if (gVideo)
    prevNodeName = "html:embed";
  gDialog.PreviewImage = document.createElementNS("http://www.w3.org/1999/xhtml", prevNodeName);
  if (gDialog.PreviewImage)
  {
    // set the src before appending to the document -- see bug 198435 for why
    // this is needed.
    var eventStr = "load";
    gDialog.PreviewImage.addEventListener("load", PreviewImageLoaded, true);
    if (gVideo)
      gDialog.PreviewImage.src = imageSrc;
    else
      gDialog.PreviewImage.data = imageSrc;
    var extension = getExtension(srcName);
    var dimensions;
    var intermediate;
    if (extension == "pdf") {
      dimensions = graphicsConverter.readSizeFromPDFFile(msiFileFromFileURL(msiURIFromString(msiMakeAbsoluteUrl(srcName))));
      // convert bp to px
      intermediate = frameUnitHandler.getValueOf(dimensions.width, 'bp');
      gConstrainWidth = gActualWidth = frameUnitHandler.getValueAs(intermediate, 'px');
      intermediate = frameUnitHandler.getValueOf(dimensions.height, 'bp');
      gConstrainHeight = gActualHeight = frameUnitHandler.getValueAs(intermediate, 'px');
    }
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
  var prefStr = "";
  var bUseDefaultWidth, bUseDefaultHeight;
  var width, height;
  try {bUseDefaultWidth = GetBoolPref("swp.graphics.usedefaultwidth");}
  catch(ex)
  {bUseDefaultWidth = false; dump("Exception getting pref swp.graphics.usedefaultwidth: " + ex + "\n");}

  try {bUseDefaultHeight = GetBoolPref("swp.graphics.usedefaultheight");}
  catch(ex) {bUseDefaultHeight = false; dump("Exception getting pref swp.graphics.usedefaultheight: " + ex + "\n");}

  if (bUseDefaultWidth || bUseDefaultHeight)
  {
    gDialog.sizeRadioGroup.selectedItem = document.getElementById("custom");
    try {prefStr = GetStringPref("swp.graphics.units");}
    catch(ex) {prefStr = ""; dump("Exception getting pref swp.graphics.units: " + ex + "\n");}
    if (prefStr.length)
      gDefaultUnit = prefStr;
    frameUnitHandler.setCurrentUnit(gDefaultUnit);
    gDialog.frameUnitMenulist.value = gDefaultUnit;
  }
  if (bUseDefaultWidth)
  {
    gDialog.autoWidthCheck.checked = false;
    try {prefStr = GetStringPref("swp.graphics.hsize");}
    catch(ex) {prefStr = ""; dump("Exception getting pref swp.graphics.hsize: " + ex + "\n");}
    if (prefStr.length)
    {
      width = frameUnitHandler.getValueFromString( prefStr, gDefaultUnit );
      gDefaultWidth = Math.round(frameUnitHandler.getValueAs(width, "px"));
      gDialog.frameWidthInput.value = width;
    }
    if (bUseDefaultHeight)
    {
      try {prefStr = GetStringPref("swp.graphics.vsize");}
      catch(ex) {prefStr = ""; dump("Exception getting pref swp.graphics.vsize: " + ex + "\n");}
      if (prefStr.length)
      {
        height = frameUnitHandler.getValueFromString( prefStr, gDefaultUnit );
        gDefaultHeight = Math.round(frameUnitHandler.getValueAs(height,"px"));
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
      height = frameUnitHandler.getValueFromString( prefStr, gDefaultUnit );
      gDefaultHeight = Math.round(frameUnitHandler.getValueAs(height,"px"));
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

var extensionRE = /\.([^\.]+)$/;

function getExtension(aFilename)
{
  var extArray = extensionRE.exec(aFilename);
  var extension = "";
  if (extArray && extArray[1])
    extension = extArray[1];
  return extension;
}

function readTotalExtraWidth(unit)
{
  var totWidth = 0;
//  totWidth += 2* frameUnitHandler.getValueAs(Number(gDialog.marginInput.left.value), "px");
  totWidth += 2 * frameUnitHandler.getValueAs(Number(gDialog.borderInput.left.value), "px");
  totWidth += 2 * frameUnitHandler.getValueAs(Number(gDialog.paddingInput.left.value), "px");
  return totWidth;
}

function readTotalExtraHeight(unit)
{
  var totHeight = 0;
//  totHeight += 2 * frameUnitHandler.getValueAs(Number(gDialog.marginInput.top.value), "px");
  totHeight += 2 * frameUnitHandler.getValueAs(Number(gDialog.borderInput.left.value), "px");
  totHeight += 2 * frameUnitHandler.getValueAs(Number(gDialog.paddingInput.left.value), "px");
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
  doDimensionEnabling();
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


function imageLoaded(event)
{
  try {
    var isSVGFile = /\.svg$/.test(event.data);
    if (isSVGFile)
    {
      var svg = event.target.contentDocument.documentElement;
      if (svg.hasAttribute("height") && svg.hasAttribute("width") && !svg.hasAttribute("viewBox"))
      {
        var currentUnit = frameUnitHandler.currentUnit;
        var width = svg.getAttribute("height");
        var numregexp = /(\d+\.*\d*)([A-Za-z]*$)/;
        var arr = numregexp.exec(width);
        if (arr) {
          frameUnitHandler.setCurrentUnit(arr[2]);
          width = frameUnitHandler.getValueAs(arr[1],"px");
          width = Math.round(width);
        }
        var height = svg.getAttribute("width");
        var arr = numregexp.exec(height);
        if (arr)
        {
          frameUnitHandler.setCurrentUnit(arr[2]);
          height = frameUnitHandler.getValueAs(arr[1],"px");
          height = Math.round(height);
        }
        svg.setAttribute("viewBox", "0 0 "+height+" "+width);
        svg.setAttribute("width","100%");
        svg.setAttribute("height","100%");
      }
    }
  }
  catch (e) {

  }
}

function isEnabled(element)
{
  if (!element) return false;
  return(!element.hasAttribute("disabled"));
}


function onAccept()
{
  /* The variables imageElement and wrapperElement point to the image element we are revising, and possibly the wrapper element if there was one. We want to preserve these in the revising case because they might contain attributes the user added. We can tell if we are in the revising case because then gInsertNewImage is false. That there was a wrapper element originally does not imply that there will or will not be one now, since the user may have added or deleted a caption.
  */
  // Use this now (default = false) so Advanced Edit button dialog doesn't trigger error message
  gIsGoingAway = true;
  importTimerHandler.stopLoading();

  gDoAltTextError = true;
  if (ValidateData())
  {
    //var editorElement = msiGetParentEditorElementForDialog(window);
    //var editor = msiGetEditor(gEditorElement);
    graphicsConverter.init(window, getDocumentGraphicsDir('').parent);
    var unit = document.getElementById("unitList").value;
    var width = 0;
    var height = 0;
    var wInput = document.getElementById("frameWidthInput");
    var hInput = document.getElementById("frameHeightInput");
    // The next elements are from msiFrameOverlay
    if (!wInput.hasAttribute('disabled')) width = wInput.value;
    if (!hInput.hasAttribute('disabled')) height = hInput.value;
//    frameUnitHandler.setCurrentUnit(unit);
    // srcurl is for debugging purposes
    // BBM: use gOriginalSrcUrl or gCopiedSrcUrl
    var srcurl = graphicsConverter.copyAndConvert(msiFileFromFileURL(msiURIFromString(gOriginalSrcUrl)), false,
      frameUnitHandler.getValueAs(width, 'px'), frameUnitHandler.getValueAs(height, 'px') );


    gEditor.beginTransaction();
    try
    {
      var tagname = gVideo ? "embed" : "object";
            // handle caption
      var captionloc = 'none';
      var gCaptionNode = GetCaptionNode(wrapperElement);

      if (isEnabled(gDialog.captionLocation)) captionloc = gDialog.captionLocation.value;
      var bHasCaption = (captionloc && captionloc !== 'none');
      var tlm = gEditor.tagListManager;

      var i;

      // caption dance: If there was a caption and the user specifies none, delete the caption
      if (gCaptionNode && captionloc === 'none') {
        gCaptionNode = null;
      }
      else if (!gCaptionNode && captionloc !== 'none') {
        gCaptionNode = gEditor.createElementWithDefaults('caption');
        var namespace = { value: null };
        gCaptionNode.appendChild(tlm.getNewInstanceOfNode(tlm.getDefaultParagraphTag(namespace), null, gCaptionNode.ownerDocument));
      }
      if (gCaptionNode && isEnabled(document.getElementById("keyInput"))) {
        if (document.getElementById("keyInput").value != "")
          gCaptionNode.setAttribute("key", document.getElementById("keyInput").value);
        else gCaptionNode.removeAttribute("key");
      }
      else if (gCaptionNode) gCaptionNode.removeAttribute("key");

//      cap.setAttribute('style', 'caption-side: '+ captionloc +';');
//      cap.setAttribute('align', captionloc);
      var posInParent;

      imgExists = !gInsertNewImage;
      if (!imgExists)
      {
        if (imageElement == null) imageElement = gEditor.createElementWithDefaults(tagname);
        imageElement.addEventListener("load", imageLoaded, true);
      }
      var src = gSrcUrl;
      //  src = checkSourceAndImportSetting(src, gDialog.isImport);
      if (/\.svg$/.test(src)) {
        imageElement.setAttribute("isSVG", "1");
        msiRequirePackage(gEditorElement,"svg","");
      }
      else
      {
        imageElement.removeAttribute("isSVG");
      }
      imageElement.setAttribute("src", src);
      imageElement.setAttribute("data", src);
      if (bHasCaption)  // we need to set up the wrapper element
      {
        if (wrapperElement == null)
          wrapperElement = gEditor.createElementWithDefaults("msiframe");
        wrapperElement.setAttribute("frametype", "image");
        wrapperElement.appendChild(gCaptionNode);
        msiEditorEnsureElementAttribute(wrapperElement, "captionloc", captionloc, null);
        // if (gDialog.wrapOptionRadioGroup.value != "full"){
        //   msiEnsureElementPackage(wrapperElement,"wrapfig",null);
        // }
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
      }
      if (wrapperElement && wrapperElement.parentNode && wrapperElement != imageElement && !bHasCaption)  //Can only happen in the !gInsertNewImage case, if there was a caption previously
      {
        posInParent = msiNavigationUtils.offsetInParent(wrapperElement);
        gEditor.insertNode(imageElement, wrapperElement.parentNode, posInParent);
        gEditor.deleteNode(wrapperElement);
        wrapperElement = imageElement;
      }

      if (gInsertNewImage) {
        if (!bHasCaption)
          wrapperElement = imageElement;
        gEditor.insertElementAtSelection(wrapperElement, true);
      }

      var extension = getExtension(gDialog.srcInput.value).toLowerCase();
      adjustObjectForFileType(imageElement, extension);

      msiEnsureElementPackage(imageElement,"graphics",null);
      if (extension === "eps") msiEnsureElementPackage(imageElement, "epstopdf", null);
      // BBM: add package for svg
      if (gVideo)
      {
        setVideoSettingsToElement(imageElement, null);
        msiEnsureElementPackage(imageElement,"hyperref",null);
        msiEnsureElementPackage(imageElement,"movie15",null);
      }

      if (gConstrainWidth > 0)
        msiEditorEnsureElementAttribute(imageElement, "naturalWidth", String(frameUnitHandler.getValueOf(gActualWidth, "px")), null);
      if (gConstrainHeight > 0)
        msiEditorEnsureElementAttribute(imageElement, "naturalHeight", String(frameUnitHandler.getValueOf(gActualHeight, "px")), null);
      if (width > 0) imageElement.setAttribute(widthAtt, width);
      else imageElement.removeAttribute(widthAtt);
      if (height > 0) imageElement.setAttribute(heightAtt, height);
      else imageElement.removeAttribute(heightAtt);

      msiEditorEnsureElementAttribute(imageElement, "originalSrcUrl", gOriginalSrcUrl, null);
      msiEditorEnsureElementAttribute(imageElement, "copiedSrcUrl", gCopiedSrcUrl, null);
      // if (!gDialog.isImport)
      //   msiEditorEnsureElementAttribute(imageElement, "byReference", "true", null);

      setFrameAttributes(wrapperElement||imageElement, imageElement, null, bHasCaption);
      if (bHasCaption) {
        if (captionloc && captionloc !== 'none') setStyleAttributeOnNode(wrapperElement||imageElement, "caption-side", captionloc, gEditor);
       // if there is no frame, globalElement == globalImage
      }

      msiSetGraphicFrameAttrsFromGraphic(imageElement, null);  //unless we first end the transaction, this seems to have trouble!

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

