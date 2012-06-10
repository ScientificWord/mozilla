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

var gDialog;
var globalElement;
var globalImage;
var gEditorElement;
Components.utils.import("resource://app/modules/unitHandler.jsm");
Components.utils.import("resource://app/modules/graphicsConverter.jsm");
var frameTabDlg = new Object();

var imageElement;
var wrapperElement;
var gDefaultWidth = 200;
var gDefaultHeight = 100;
var gDefaultUnit = "pt";
var gOriginalSrc = "";
var gHaveDocumentUrl = false;

var gPreviewImageWidth = 80;
var gPreviewImageHeight = 50;

// These mode variables control the msiFrameOverlay code
var gFrameModeImage = true;
var gFrameModeTextFrame = false;

var gInsertNewImage = true;
var gCaptionData;

var importTimer;
var nativeGraphicTypes = ["png", "gif", "jpg", "jpeg", "pdf", "xvc","xvz"];
var typesetGraphicTypes = ["eps", "pdf", "png", "jpg"];

// dialog initialization code
function Startup()
{
  gEditorElement = msiGetParentEditorElementForDialog(window);
  var editor = msiGetEditor(gEditorElement);
  if (!editor)
  {
    window.close();
    return;
  }

  gDialog = new Object();
  gDialog.import            = document.getElementById( "importRefRadioGroup").selectedIndex == 0;
  gDialog.tabBox            = document.getElementById( "TabBox" );
  gDialog.tabPicture        = document.getElementById( "imagePictureTab" );
  gDialog.tabPlacement      = document.getElementById( "msiPlacementTab" );
  gDialog.tabFrame          = document.getElementById( "msiFrameTab" );
  gDialog.tabLabeling       = document.getElementById( "imageLabelingTab" );
//  gDialog.tabLink           = document.getElementById( "imageLinkTab" );
  gDialog.srcInput          = document.getElementById( "srcInput" );
  gDialog.relativeURL       = document.getElementById( "makeRelativeCheckbox" ).checked;
  gDialog.altTextInput      = document.getElementById( "altTextInput" );
  gDialog.ImageHolder       = document.getElementById( "preview-image-holder" );
  gDialog.PreviewWidth      = document.getElementById( "PreviewWidth" );
  gDialog.PreviewHeight     = document.getElementById( "PreviewHeight" );
  gDialog.PreviewSize       = document.getElementById( "PreviewSize" );
  gDialog.PreviewImage      = null;
  gDialog.captionEdit       = document.getElementById( "captionTextInput" );
  gDialog.captionPlacementGroup = document.getElementById("captionPlacementRadioGroup");
  gDialog.herePlacementRadioGroup   = document.getElementById("herePlacementRadioGroup");
  gDialog.OkButton          = document.documentElement.getButton("accept");
  gDialog.keyInput          = document.getElementById( "keyInput" );
//  gDialog.linkKeyInput      = document.getElementById( "linkKeyInput" );
  
  // Get a single selected image element
  var tagName = "object";
  imageElement = null;
  wrapperElement = null;
  if (window.arguments && window.arguments.length >0)
  {
    imageElement = window.arguments[0];
    if (imageElement && (imageElement.nodeName !== "msiframe"))
    {
      if (imageElement.parentNode.nodeName === "msiframe")
      {
        wrapperElement = imageElement.parentNode;
      }
      else wrapperElement = imageElement;  //shouldn't happen
    } 
    else
    {
      wrapperElement = imageElement;
      imageElement = wrapperElement.getElementsByTagName("object")[0];
    }
  }
  if (!imageElement)
  {
    // Does this ever get run?
    try {
      imageElement = getSelectionParentByTag(editor,"input");
      if (!imageElement || imageElement.getAttribute("type") != "image") {
        // Get a single selected image element
        imageElement = getSelectionParentByTag(editor,tagName);
      }
    } catch (e) {}
  }
  if (imageElement)
  {
    // We found an element and don't need to insert one
    if (imageElement.hasAttribute("src") || imageElement.hasAttribute("data"))
      gInsertNewImage = false;
  }
  else
  {
    gInsertNewImage = true;

    // We don't have an element selected,
    //  so create one with default attributes
    try {
      imageElement = editor.createElementWithDefaults(tagName);
      wrapperElement = imageElement;
    } catch(e) {}

    if (!imageElement)
    {
      dump("Failed to get selected element or create a new one!\n");
      window.close();
      return;
    }
  }

  initKeyList();

  // Make a copy to use for AdvancedEdit
  globalElement = wrapperElement.cloneNode(true);
  globalImage = globalElement;
  if (msiGetBaseNodeName(globalElement) == "msiframe")
    globalImage = globalElement.getElementsByTagName("object")[0];

  // We only need to test for this once per dialog load
  gHaveDocumentUrl = msiGetDocumentBaseUrl();

  initFrameTab(frameTabDlg, wrapperElement, gInsertNewImage, imageElement);
  InitDialog();
//  ChangeLinkLocation();

  // Save initial source URL
  gOriginalSrc = gDialog.srcInput.value;

  // By default turn constrain on, but both width and height must be in pixels
//  frameTabDlg.constrainCheckbox.checked = true;

  window.mMSIDlgManager = new msiDialogConfigManager(window);
  window.mMSIDlgManager.configureDialog();

  // Start in "Link" tab if 2nd arguement is true
//  if (gDialog.linkTab && (window.arguments.length > 1) && window.arguments[1])
//  {
//    document.getElementById("TabBox").selectedTab = gDialog.linkTab;
//    SetTextboxFocus(gDialog.hrefInput);
//  }
//  else
  SetTextboxFocus(gDialog.srcInput);

  SetWindowLocation();
}

// Set dialog widgets with attribute data
// We get them from globalElement copy so this can be used
//   by AdvancedEdit(), which is shared by all property dialogs
function InitDialog()
{
  InitImage();
//  var border = TrimString(gDialog.border.value);
//  gDialog.showLinkBorder.checked = border != "" && border > 0;
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
    element= msiGetHTMLOrCSSStyleValue(null, globalImage, gBorderdesc[index], null);
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
  { gDefaultPlacement = GetStringPref("swp.defaultGraphicsPlacement"); }
  catch(ex) {gDefaultPlacement = "inline"; dump("Exception getting pref swp.defaultGraphicsPlacement: " + ex + "\n");}
  try
  { gDefaultInlineOffset = GetStringPref("swp.defaultGraphicsInlineOffset"); }
  catch(ex) {gDefaultInlineOffset = "0"; dump("Exception getting pref swp.defaultGraphicsInlineOffset: " + ex + "\n");}
}

// Set dialog widgets with attribute data
// We get them from globalElement copy so this can be used
//   by AdvancedEdit(), which is shared by all property dialogs
function InitImage()
{
  // Set the controls to the image's attributes
  gDialog.srcInput.value = getSourceLocationFromElement(imageElement);

  // Set "Relativize" checkbox according to current URL state
  msiSetRelativeCheckbox();

  // setup the height and width values
  var width = 0;
  var height = 0;
  var pixelWidth = 0;
  var pixelHeight = 0;
//  var width = msiInitPixelOrPercentMenulist(globalImage,
//                    gInsertNewImage ? null : imageElement,
//                    widthAtt, "widthUnitsMenulist", gPixel);
//  var height = msiInitPixelOrPercentMenulist(globalImage,
//                    gInsertNewImage ? null : imageElement,
//                    heightAtt, "heightUnitsMenulist", gPixel);

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
      frameTabDlg.frameUnitMenulist.value = unit;
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
    if (imageElement.hasAttribute("src"))
    {
      gActualWidth  = gConstrainWidth = imageElement.naturalWidth;
      gActualHeight = gConstrainHeight = imageElement.naturalHeight;
    } else if (imageElement.hasAttribute("data"))
    {
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
  else
  {
    var defaultUnitStr, defaultWidthStr, defaultHeightStr;
    try
    {defaultUnitStr = GetStringPref("swp.defaultGraphicsSizeUnits");}
    catch(ex) {defaultUnitStr = ""; dump("Exception getting pref swp.defaultGraphicsSizeUnits: " + ex + "\n");}
    if (defaultUnitStr.length)
    {
      gDefaultUnit = defaultUnitStr;
      frameUnitHandler.setCurrentUnit(gDefaultUnit);
      frameTabDlg.frameUnitMenulist.value = gDefaultUnit;
    }

    try
    {defaultWidthStr = GetStringPref("swp.defaultGraphicsHSize");}
    catch(ex) {defaultWidthStr = ""; dump("Exception getting pref swp.defaultGraphicsHSize: " + ex + "\n");}
    if (defaultWidthStr.length)
      width = frameUnitHandler.getValueFromString( defaultWidthStr, gDefaultUnit );
    gDefaultWidth = Math.round(frameUnitHandler.getValueAs(width, "px"));

    try
    {defaultHeightStr = GetStringPref("swp.defaultGraphicsVSize");}
    catch(ex) {defaultHeightStr = ""; dump("Exception getting pref swp.defaultGraphicsVSize: " + ex + "\n");}
    if (defaultHeightStr.length)
      height = frameUnitHandler.getValueFromString( defaultHeightStr, gDefaultUnit );
    gDefaultHeight = Math.round(frameUnitHandler.getValueAs(height,"px"));
    
  }

  if ((width > 0) || (height > 0))
    setWidthAndHeight(width, height, null);
  else if ((gActualHeight > 0)||(gActualWidth > 0)) 
    setWidthAndHeight(unitRound(frameUnitHandler.getValueOf(gActualWidth,"px")), 
                      unitRound(frameUnitHandler.getValueOf(gActualHeight,"px")), null);
  else
    setWidthAndHeight(unitRound(frameUnitHandler.getValueOf(gDefaultWidth,"pt")),
                      unitRound(frameUnitHandler.getValueOf(gDefaultHeight,"pt")), null);

  // Force loading of image from its source and show preview image
  LoadPreviewImage();

//  if (globalElement.hasAttribute("title"))
//    gDialog.titleInput.value = globalElement.getAttribute("title");

  if (!gCaptionData)
    gCaptionData = {m_position : "below", m_captionStr : ""};
  var position = "below";
  var capData = findCaptionNodes(wrapperElement);
  if (capData.belowCaption)
  {
    gCaptionData.m_captionStr = getNodeChildrenAsString(capData.belowCaption);
    position = "below";
  }
  else if (capData.aboveCaption)
  {
    gCaptionData.m_captionStr = getNodeChildrenAsString(capData.aboveCaption);
    position = "above";
  }
  gCaptionData.m_position = position;
  msiInitializeEditorForElement(gDialog.captionEdit, gCaptionData.m_captionStr);
  gDialog.captionPlacementGroup.value = gCaptionData.m_position;
  
  var imageKey = "";
//  if (globalElement.hasAttribute("key"))
//    imageKey = globalElement.getAttribute("key");
  if (imageElement.hasAttribute("key"))
    imageKey = imageElement.getAttribute("key");
  else if (imageElement.hasAttribute("id"))
    imageKey = imageElement.getAttribute("id");
//  else if (globalElement.hasAttribute("id"))
//    imageKey = globalElement.getAttribute("id");
  gDialog.keyInput.value = imageKey;

  var hasAltText = imageElement.hasAttribute("alt");
  var altText;
  if (hasAltText)
  {
    altText = imageElement.getAttribute("alt");
    gDialog.altTextInput.value = altText;
  }

  // Initialize altText widgets during dialog startup 
  //   or if user enterred altText in Advanced Edit dialog
  //  (this preserves "Don't use alt text" radio button state)
//  if (!gDialog.altTextRadioGroup.selectedItem || altText)
//  {
//    if (gInsertNewImage || !hasAltText || (hasAltText && gDialog.altTextInput.value))
//    {
//      SetAltTextDisabled(false);
//      gDialog.altTextRadioGroup.selectedItem = gDialog.altTextRadio;
//    }
//    else
//    {
//      SetAltTextDisabled(true);
//      gDialog.altTextRadioGroup.selectedItem = gDialog.noAltTextRadio;
//    }
//  }

  // Set actual radio button if both set values are the same as actual
  SetSizeWidgets(pixelWidth, pixelHeight);
//  frameTabDlg.unitList.value = "pt";

//  if ((width > 0) || (height > 0))
//  {
//    frameTabDlg.widthInput.value  = frameUnitHandler.getValueOf(width,"px");
//    frameTabDlg.heightInput.value = frameUnitHandler.getValueOf(height,"px");
//  }
//  else if ((gActualHeight > 0)||(gActualWidth > 0)) 
//  {
//    frameTabDlg.widthInput.value  = frameUnitHandler.getValueOf(gActualWidth,"px");
//    frameTabDlg.heightInput.value = frameUnitHandler.getValueOf(gActualHeight,"px");
//  }
//  else
//  {
//    frameTabDlg.widthInput.value  = frameUnitHandler.getValueOf(gDefaultWidth,"pt");
//    frameTabDlg.heightInput.value = frameUnitHandler.getValueOf(gDefaultHeight,"pt");
//  }
  if (!Number(frameTabDlg.widthInput.value))
    constrainProportions( "frameHeightInput", "frameWidthInput", null );
  else if (!Number(frameTabDlg.heightInput.value))
    constrainProportions( "frameWidthInput", "frameHeightInput", null );
  // set spacing editfields
  var herePlacement;
  var placement = globalElement.getAttribute("placement");
  if (placement == "here");
    gDialog.herePlacementRadioGroup.value=globalElement.getAttribute("herePlacement");

  // dialog.border.value       = globalElement.getAttribute("border");
  var bordervalues;
  var bv = msiGetHTMLOrCSSStyleValue(null, globalImage, "border", null);
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



function LoadPreviewImage()
{
  gDialog.PreviewSize.collapsed = true;

  var imageSrc = TrimString(gDialog.srcInput.value);
  if (!imageSrc)
    return;

  try {
    // Remove the image URL from image cache so it loads fresh
    //  (if we don't do this, loads after the first will always use image cache
    //   and we won't see image edit changes or be able to get actual width and height)
    
    var IOService = msiGetIOService();
    if (IOService)
    {
      // We must have an absolute URL to preview it or remove it from the cache
      imageSrc = msiMakeAbsoluteUrl(imageSrc);

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
    
  gDialog.PreviewImage = document.createElementNS("http://www.w3.org/1999/xhtml", "object");
  if (gDialog.PreviewImage)
  {
    // set the src before appending to the document -- see bug 198435 for why
    // this is needed.
    gDialog.PreviewImage.addEventListener("load", PreviewImageLoaded, true);
    gDialog.PreviewImage.src = imageSrc;
    gDialog.ImageHolder.appendChild(gDialog.PreviewImage);
  }
}


//This function assumes "width" and "height" are in pixels
function  SetSizeWidgets(width, height)
{
  if (!(width || height) || (gActualWidth && gActualHeight && width == gActualWidth && height == gActualHeight))
    frameTabDlg.sizeRadioGroup.value = "actual";
  else
    frameTabDlg.sizeRadioGroup.value = "custom";

  if (!frameTabDlg.actual.selected)
  {
    frameTabDlg.actual.radioGroup.selectedItem = frameTabDlg.custom;

    // Decide if user's sizes are in the same ratio as actual sizes
    if (gActualWidth && gActualHeight)
    {
      if (gActualWidth > gActualHeight)
        frameTabDlg.constrainCheckbox.checked = (unitRound(gActualHeight * width / gActualWidth) == height);
      else
        frameTabDlg.constrainCheckbox.checked = (unitRound(gActualWidth * height / gActualHeight) == width);
    }
  }
}

// These mode variables control the msiFrameOverlay code
//var gFrameModeImage = true;
//var gFrameModeTextFrame = false;

//var gInsertNewImage;

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

//function onChangeCaptionPlacement()
//{
//  var newPosition = gDialog.captionPlacementGroup.value;
//  if (newPosition != gCaptionData.m_position)
//  {
//    gCaptionData.m_captionStr[gCaptionData.m_position] = getCaptionEditContents();
//    var editor = msiGetEditor(gDialog.captionEdit);
//    msiDeleteBodyContents(editor);
//    editor.insertHTMLWithContext(gCaptionData.m_captionStr[newPosition], "", "", "", null, null, 0, true);
//  }
//}

function getCaptionEditContents()
{
  if (!gCaptionData.contentFilter)
    gCaptionData.contentFilter = new msiDialogEditorContentFilter(gDialog.captionEdit);
  return gCaptionData.contentFilter.getDocumentFragmentString();
}

function findCaptionNodes(parentNode)
{
  var retData = {belowCaption : null, aboveCaption : null};
  var theChildren = msiNavigationUtils.getSignificantContents(parentNode);
  var position;
  var parentPos = "below";
  if (parentNode.hasAttribute("captionloc"))
    parentPos = parentNode.getAttribute("captionloc");
  for (var ii = 0; ii < theChildren.length; ++ii)
  {
    if (msiGetBaseNodeName(theChildren[ii]) == "imagecaption")
    {
      if (theChildren[ii].hasAttribute("position"))
        position = theChildren[ii].getAttribute("position");
      else
        position = parentPos;
      if (!position || (position != "above"))
        position = parentPos;
      position += "Caption";
      retData[position] = theChildren[ii];
    }
  }
  return retData;
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

function syncCaptionAndExisting(dlgCaptionStr, editor, imageObj, positionStr)
{
  var bChange = false;
  var existingStr = "";
  var currCaptionNode;
  var currLoc = imageObj.getAttribute("captionloc");
  if (!currLoc || !currLoc.length)
    currLoc = "below";
  var captionNodes = imageObj.getElementsByTagName("imagecaption");
   if (captionNodes && captionNodes.length)
    currCaptionNode = captionNodes[0];
  if (currCaptionNode)
    existingStr = getNodeChildrenAsString(currCaptionNode);
  if (dlgCaptionStr.length)
    msiEditorEnsureElementAttribute(imageObj, "captionloc", positionStr, editor);
  else
    msiEditorEnsureElementAttribute(imageObj, "captionloc", null, editor);  //This will remove the captionLoc attribute

  bChange = (existingStr != dlgCaptionStr);
  if (!bChange)
  {
    return;
  }
  if (dlgCaptionStr.length)
  {
    if (!currCaptionNode)
    {
      currCaptionNode = editor.document.createElementNS(xhtmlns, "imagecaption");
//      currCaptionNode.setAttribute("position", positionStr);
      editor.insertNode(currCaptionNode, imageObj, imageObj.childNodes.length);
    }
    else if (existingStr.length)
    {
      for (var jx = currCaptionNode.childNodes.length - 1; jx >= 0; --jx)
        editor.deleteNode(currCaptionNode.childNodes[jx]);
    }
    editor.insertHTMLWithContext(dlgCaptionStr, "", "", "", null, currCaptionNode, 0, false);
    msiEditorEnsureElementAttribute(imageObj, "captionloc", positionStr, editor);
  }
  else if (currCaptionNode)
    editor.deleteNode(currCaptionNode);
}

// Get data from widgets, validate, and set for the global element
//   accessible to AdvancedEdit() [in msiEdDialogCommon.js]
function ValidateData()
{
  return ValidateImage();
}

//// Get data from widgets, validate, and set for the global element
////   accessible to AdvancedEdit() [in msiEdDialogCommon.js]
//function ValidateImage()
//{
//  dump("in ValidateImage\n");
//  var editorElement = msiGetParentEditorElementForDialog(window);
//  var editor = msiGetEditor(editorElement);
////  var editor = GetCurrentEditor();
//  if (!editor)
//    return false;
//
////  gValidateTab = gDialog.tabLocation;
//  if (!gDialog.srcInput.value)
//  {
//    AlertWithTitle(null, GetString("MissingImageError"));
////    SwitchToValidatePanel();
//    gDialog.srcInput.focus();
//    return false;
//  }
//
//  //TODO: WE NEED TO DO SOME URL VALIDATION HERE, E.G.:
//  // We must convert to "file:///" or "http://" format else image doesn't load!
//  var src = TrimString(gDialog.srcInput.value);
//  globalElement.setAttribute("src", src);
//
//  var title = TrimString(gDialog.titleInput.value);
//  if (title)
//    globalElement.setAttribute("title", title);
//  else
//    globalElement.removeAttribute("title");
//
//  alt = TrimString(gDialog.altTextInput.value);
//
//  globalElement.setAttribute("alt", alt);
//
//  var width = "";
//  var height = "";
//
////  gValidateTab = gDialog.tabDimensions;
//  if (!frameTabDlg.actual.selected)
//  {
//    // Get user values for width and height
//    width = msiValidateNumber(frameTabDlg.widthInput, gDialog.widthUnitsMenulist, 1, gMaxPixels, 
//                           globalElement, "width", false, true);
//    if (gValidationError)
//      return false;
//
//    height = msiValidateNumber(frameTabDlg.heightInput, gDialog.heightUnitsMenulist, 1, gMaxPixels, 
//                            globalElement, "height", false, true);
//    if (gValidationError)
//      return false;
//  }
//
//  // We always set the width and height attributes, even if same as actual.
//  //  This speeds up layout of pages since sizes are known before image is loaded
//  if (!width)
//    width = gActualWidth;
//  if (!height)
//    height = gActualHeight;
//
//  // Remove existing width and height only if source changed
//  //  and we couldn't obtain actual dimensions
//  var srcChanged = (src != gOriginalSrc);
//  if (width)
//    globalElement.setAttribute("width", width);
//  else if (srcChanged)
//    editor.removeAttributeOrEquivalent(globalElement, "width", true);
//
//  if (height)
//    globalElement.setAttribute("height", height);
//  else if (srcChanged) 
//    editor.removeAttributeOrEquivalent(globalElement, "height", true);
//
//
//  // spacing attributes
////  gValidateTab = gDialog.tabBorder;
////  msiValidateNumber(gDialog.imagelrInput, null, 0, gMaxPixels, 
////                 globalElement, "hspace", false, true, true);
////  if (gValidationError)
////    return false;
////
////  msiValidateNumber(gDialog.imagetbInput, null, 0, gMaxPixels, 
////                 globalElement, "vspace", false, true);
////  if (gValidationError)
////    return false;
//
//  // note this is deprecated and should be converted to stylesheets
////  msiValidateNumber(gDialog.border, null, 0, gMaxPixels, 
////                 globalElement, "border", false, true);
////  if (gValidationError)
////    return false;
//
//  // Default or setting "bottom" means don't set the attribute
//  // Note that the attributes "left" and "right" are opposite
//  //  of what we use in the UI, which describes where the TEXT wraps,
//  //  not the image location (which is what the HTML describes)
////  switch ( gDialog.alignTypeSelect.value )
////  {
////    case "top":
////    case "middle":
////    case "right":
////    case "left":
////      globalElement.setAttribute( "align", gDialog.alignTypeSelect.value );
////      break;
////    default:
////      try {
////        editor.removeAttributeOrEquivalent(globalElement, "align", true);
////      } catch (e) {}
////  }
//
//  return true;
//}
//

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

var isSVGFile = false;
function chooseFile()
{
//  if (gTimerID)
//    clearTimeout(gTimerID);

  // Get a local file, converted into URL format
//  var fileName = GetLocalFileURL(["img"]); // return a URLString
  var graphicsFileFilterStr = getGraphicsImportFilterString();
  var fileName = msiGetLocalFileURLSpecial([{filter : graphicsFileFilterStr, filterTitle : GetString("IMGFiles")}], "image");
  if (fileName)
  {
    importTimerHandler.reset();
    var url = msiURIFromString(fileName);

    if (gDialog.import) // copy the file into the graphics directory
    {
      fileName = getLoadableGraphicFile(url);
    }
    else
    {
    // Always try to relativize local file URLs
      if (gHaveDocumentUrl)
        fileName = msiMakeRelativeUrl(fileName);
    }

    gDialog.srcInput.value = fileName;

    msiSetRelativeCheckbox();
    doOverallEnabling();
  }
  if (!importTimerHandler.isLoading("import"))
  {
//  if (!gfxImportProcess || !gfxImportProcess.isRunning)
    dump("Calling LoadPreviewImage after importTimerHandler.isLoading returned false.\n");
    LoadPreviewImage();
  }
  // Put focus into the input field
  SetTextboxFocus(gDialog.srcInput);
}

function getLoadableGraphicFile(inputURL)
{
  var newFile;
  var fileName = "";
  var file = msiFileFromFileURL(inputURL);
  var filedir = file.parent;
  var extension = getExtension(file.leafName);
  var dir = getDocumentGraphicsDir();
  if (dir.path === filedir.path) {
    return file.path;
  }
  if (!dir.exists())
    dir.create(1, 0755);
  if (extension)  //if not, should we just do the copy and hope for the best? Or forget it?
  {
    var nNative = nativeGraphicTypes.indexOf(extension);
    if (nNative < 0)
    {
      newFile = doGraphicsImport(file, "import");
      fileName = "graphics/" + newFile.leafName;
    }
    else  //ordinary import
    {
      try
      {
        isSVGFile = /\.svg$/.test(file.leafName);
        file.permissions = 0755;
        fileName = "graphics/"+file.leafName;
        newFile = dir.clone();
        newFile.append(file.leafName);
        if (newFile.exists()) newFile.remove(false);
        file.copyTo(dir,"");
      }
      catch(e)
      {
        dump("exception: e="+e.msg);
      }
    }
    if (typesetGraphicTypes.indexOf(extension.toLowerCase()) < 0)  //Need file in LaTeX-friendly format
    {
      var bCanUseImport = false;
      if (newFile)
      {
        var impExtension = getExtension(newFile.leafName).toLowerCase();
        if (typesetGraphicTypes.indexOf(impExtension) >= 0)
          bCanUseImport = true;
      }
      if (!bCanUseImport)
        doGraphicsImport(file, "tex");
    }
  }
  return fileName;
}

var importTimerHandler =
{
  mSourceFile : null,
  mImportHandler : null,
  mTexHandler : null,

//  this.startLoading = function(sourceFile, targFile, process, mode)
//  {
//  };

  terminalCallback : function(sourceFile, importTargFile, importStatus, errorString)
  {
//    if (this.texImportFile.path == importTargFile.path)
//    {
//      this.texImportStatus = importStatus;
//      this.texImportError = errorString;
//    }
//    else
//    {
//      this.importStatus = importStatus;
//      this.importError = errorString;
//    }
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
//      importData.mImportSentinelFile = this.getFailureSentinelFile("import");
//      importData.mImportLogFile = this.getImportLogFile("import");
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
//      importData.mTexSentinelFile = this.getFailureSentinelFile("tex");
//      importData.mTexImportLogFile = this.getImportLogFile("tex");
    }
    if (bTimerRunning)
      return;

    if (bLaunchDialog)
    {
      dump("Launching ConvertGraphics dialog.\n");
      launchConvertingDialog(importData);
      //Put up a dialog asking if user wants to cancel?
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

    if (this.importStatus == this.statusSuccess)
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
  }

};

//rwa 5-19-12var importTimerHandler = 
//rwa 5-19-12{  
//rwa 5-19-12  statusNone : 0,
//rwa 5-19-12  statusRunning : 1,
//rwa 5-19-12  statusSuccess : 2,
//rwa 5-19-12  statusFailed : 3,
//rwa 5-19-12//  this.timercopy = timer;
//rwa 5-19-12  sourceFile : null,
//rwa 5-19-12  importTargFile : null,
//rwa 5-19-12  texTargFile : null,
//rwa 5-19-12  importProcess : null,
//rwa 5-19-12  texImportProcess : null,
//rwa 5-19-12  importStatus : this.statusNone,
//rwa 5-19-12  texImportStatus : this.statusNone,
//rwa 5-19-12  timerCount : 0,
//rwa 5-19-12
//rwa 5-19-12  notify : function(timer)
//rwa 5-19-12  { 
//rwa 5-19-12//    if (timer)
//rwa 5-19-12//      this.timercopy = timer;
//rwa 5-19-12    this.checkStatus();
//rwa 5-19-12
//rwa 5-19-12    if (this.importStatus == this.statusRunning || this.texImportStatus == this.statusRunning)
//rwa 5-19-12    {
//rwa 5-19-12      ++this.timerCount;
//rwa 5-19-12//      var exitVal = "[" + (this.importProcess ? this.importProcess.exitValue : "none");
//rwa 5-19-12//      exitVal += "," + (this.texImportProcess ? this.texImportProcess.exitValue : "none") + "]";
//rwa 5-19-12//      dump("In graphics loading timerCallback, timer count is [" + this.timerCount + "], exit vals are " + exitVal + ".\n");
//rwa 5-19-12      if (this.timerCount < 8)  //keep waiting
//rwa 5-19-12        return;
//rwa 5-19-12
//rwa 5-19-12//      if (gfxImportProcess && !gfxImportProcess.isRunning)
//rwa 5-19-12//      {
//rwa 5-19-12//        stopImportTimer("import");
//rwa 5-19-12//      }
//rwa 5-19-12      timer.cancel();
//rwa 5-19-12      this.checkLogFileStatus();
//rwa 5-19-12      var importData = {mSourceFile : this.sourceFile, 
//rwa 5-19-12                        mImportProcess : this.importProcess, mTexImportProcess : this.texImportProcess,
//rwa 5-19-12                        mImportStatus : this.importStatus, mTexImportStatus: this.texImportStatus,
//rwa 5-19-12                        mImportTargFile : this.importTargFile, mTexImportTargFile : this.texTargFile,
//rwa 5-19-12                        mImportSentinelFile : this.getFailureSentinelFile("import"),
//rwa 5-19-12                        mTexSentinelFile : this.getFailureSentinelFile("tex"),
//rwa 5-19-12                        mImportLogFile : this.getImportLogFile("import"), 
//rwa 5-19-12                        mTexImportLogFile : this.getImportLogFile("tex")};
//rwa 5-19-12      launchConvertingDialog(importData);
//rwa 5-19-12      //Put up a dialog asking if user wants to cancel?
//rwa 5-19-12      return;
//rwa 5-19-12    }
//rwa 5-19-12     
//rwa 5-19-12    //Otherwise, we're finished
//rwa 5-19-12    timer.cancel();
//rwa 5-19-12    this.checkFinalStatus();
//rwa 5-19-12  },
//rwa 5-19-12
//rwa 5-19-12  reset : function()
//rwa 5-19-12  {
//rwa 5-19-12    if (this.importTargFile)
//rwa 5-19-12    {
//rwa 5-19-12      var sentFile = this.getFailureSentinelFile("import");
//rwa 5-19-12      if (sentFile.exists())
//rwa 5-19-12        sentFile.remove(false);
//rwa 5-19-12      this.importTargFile = null;
//rwa 5-19-12      var logFile = this.getImportLogFile("import");
//rwa 5-19-12      if (logFile.exists())
//rwa 5-19-12        logFile.remove(false);
//rwa 5-19-12    }
//rwa 5-19-12    if (this.texTargFile)
//rwa 5-19-12    {
//rwa 5-19-12      var texSentFile = this.getFailureSentinelFile("tex");
//rwa 5-19-12      if (texSentFile.exists())
//rwa 5-19-12        texSentFile.remove(false);
//rwa 5-19-12      this.texTargFile = null;
//rwa 5-19-12      var texLogFile = this.getImportLogFile("tex");
//rwa 5-19-12      if (texLogFile.exists())
//rwa 5-19-12        texLogFile.remove(false);
//rwa 5-19-12    }
//rwa 5-19-12    this.timerCount = 0;
//rwa 5-19-12    this.sourceFile = null;
//rwa 5-19-12    this.importProcess = null;
//rwa 5-19-12    this.texTargFile = null;
//rwa 5-19-12    this.texImportProcess = null;
//rwa 5-19-12    this.importStatus = this.statusNone;
//rwa 5-19-12    this.texImportStatus = this.statusNone;
//rwa 5-19-12  },
//rwa 5-19-12
//rwa 5-19-12  startLoading : function(srcFile, targFile, process, mode)
//rwa 5-19-12  {
//rwa 5-19-12    this.timerCount = 0;  //always restart timer count with second load if two are converting?
//rwa 5-19-12    this.sourceFile = srcFile;
//rwa 5-19-12    if (mode == "tex")
//rwa 5-19-12    {
//rwa 5-19-12      this.texTargFile = targFile;
//rwa 5-19-12      this.texImportProcess = process;
//rwa 5-19-12      this.texImportStatus = this.statusRunning;
//rwa 5-19-12    }
//rwa 5-19-12    else
//rwa 5-19-12    {
//rwa 5-19-12      this.importTargFile = targFile;
//rwa 5-19-12      this.importProcess = process;
//rwa 5-19-12      this.importStatus = this.statusRunning;
//rwa 5-19-12    }
//rwa 5-19-12  },
//rwa 5-19-12
//rwa 5-19-12  //Assumptions about status:
//rwa 5-19-12  //  (i)If no import conversion process is launched, it is assumed that the file is of an importable type as is.
//rwa 5-19-12  // (ii)If no tex import conversion process is launched, either the result of the import process or the native file must be
//rwa 5-19-12  //       typesettable. Which is the case should be noted somewhere here (using this.canTypesetSourceFile()).
//rwa 5-19-12  checkStatus : function()
//rwa 5-19-12  {
//rwa 5-19-12    if (this.texImportStatus == this.statusRunning)
//rwa 5-19-12    {
//rwa 5-19-12      if (this.texTargFile.exists())
//rwa 5-19-12        this.texImportStatus = this.statusSuccess;
//rwa 5-19-12      else if (!this.texImportProcess || this.processFailed("tex"))
//rwa 5-19-12        this.texImportStatus = this.statusFailed;
//rwa 5-19-12    }
//rwa 5-19-12    if (this.importStatus == this.statusRunning)
//rwa 5-19-12    {
//rwa 5-19-12      if (this.importTargFile.exists())
//rwa 5-19-12        this.importStatus = this.statusSuccess;
//rwa 5-19-12      else if (!this.importProcess || this.processFailed("import"))
//rwa 5-19-12        this.importStatus = this.statusFailed;
//rwa 5-19-12    }
//rwa 5-19-12  },
//rwa 5-19-12
//rwa 5-19-12  checkLogFileStatus : function()
//rwa 5-19-12  {
//rwa 5-19-12    var errorRE = /(error)|(fail)/i;
//rwa 5-19-12    if (this.texImportStatus == this.statusRunning)
//rwa 5-19-12    {
//rwa 5-19-12      if (this.getImportLogFile("tex") && this.getImportLogFile("tex").exists())
//rwa 5-19-12      {
//rwa 5-19-12        var logStr;
//rwa 5-19-12        try
//rwa 5-19-12        {
//rwa 5-19-12          logStr = this.getImportLogInfo("tex");
//rwa 5-19-12        } catch(ex) {}
//rwa 5-19-12        if (logStr)
//rwa 5-19-12        {
//rwa 5-19-12          if (errorRE.exec(logStr))
//rwa 5-19-12            this.texImportStatus = this.statusFailed;
//rwa 5-19-12        }
//rwa 5-19-12      }
//rwa 5-19-12    }
//rwa 5-19-12    if (this.importStatus == this.statusRunning)
//rwa 5-19-12    {
//rwa 5-19-12      if (this.getImportLogFile("import") && this.getImportLogFile("import").exists())
//rwa 5-19-12      {
//rwa 5-19-12        var logStr;
//rwa 5-19-12        try
//rwa 5-19-12        {
//rwa 5-19-12          logStr = this.getImportLogInfo("import");
//rwa 5-19-12        } catch(ex) {}
//rwa 5-19-12        if (logStr)
//rwa 5-19-12        {
//rwa 5-19-12          if (errorRE.exec(logStr))
//rwa 5-19-12            this.importStatus = this.statusFailed;
//rwa 5-19-12        }
//rwa 5-19-12      }
//rwa 5-19-12    }
//rwa 5-19-12  },
//rwa 5-19-12
//rwa 5-19-12  checkFinalStatus : function()
//rwa 5-19-12  {
//rwa 5-19-12    this.checkStatus();
//rwa 5-19-12    if ((this.importStatus == this.statusRunning) || (this.texImportStatus == this.statusRunning))
//rwa 5-19-12      this.checkLogFileStatus();
//rwa 5-19-12    if (this.importStatus == this.statusFailed || this.texImportStatus == this.statusFailed)
//rwa 5-19-12      this.postFailedImportNotice();
//rwa 5-19-12
//rwa 5-19-12    if (this.importStatus == this.statusSuccess)
//rwa 5-19-12      LoadPreviewImage();
//rwa 5-19-12  },
//rwa 5-19-12
//rwa 5-19-12  isLoading : function(mode)
//rwa 5-19-12  {
//rwa 5-19-12    this.checkStatus();
//rwa 5-19-12    if (mode == "import")
//rwa 5-19-12      return (this.importStatus == this.statusRunning);
//rwa 5-19-12    if (mode == "tex")
//rwa 5-19-12      return (this.texImportStatus == this.statusRunning);
//rwa 5-19-12    return ((this.importStatus == this.statusRunning) || (this.texImportStatus == this.statusRunning));
//rwa 5-19-12  },
//rwa 5-19-12
//rwa 5-19-12  getFailureSentinelFile : function(mode)
//rwa 5-19-12  {
//rwa 5-19-12    var sentinelFile = (mode == "tex") ? this.texTargFile.clone() : this.importTargFile.clone();
//rwa 5-19-12    if (!sentinelFile || !sentinelFile.path)
//rwa 5-19-12      return null;
//rwa 5-19-12    var leaf = sentinelFile.leafName;
//rwa 5-19-12    leaf += ".txt";
//rwa 5-19-12    sentinelFile = sentinelFile.parent;
//rwa 5-19-12    sentinelFile.append(leaf);
//rwa 5-19-12    return sentinelFile;
//rwa 5-19-12  },
//rwa 5-19-12
//rwa 5-19-12  getImportLogFile : function(mode)
//rwa 5-19-12  {
//rwa 5-19-12    var logFile = (mode == "tex") ? this.texTargFile.clone() : this.importTargFile.clone();
//rwa 5-19-12    if (!logFile || !logFile.path)
//rwa 5-19-12      return null;
//rwa 5-19-12    var leaf = logFile.leafName;
//rwa 5-19-12    leaf += ".log";
//rwa 5-19-12    logFile = logFile.parent;
//rwa 5-19-12    logFile.append(leaf);
//rwa 5-19-12    return logFile;
//rwa 5-19-12  },
//rwa 5-19-12
//rwa 5-19-12  processFailed: function(mode)
//rwa 5-19-12  {
//rwa 5-19-12    var sentFile = this.getFailureSentinelFile(mode);
//rwa 5-19-12    return (sentFile && sentFile.exists());
//rwa 5-19-12  },
//rwa 5-19-12
//rwa 5-19-12  postFailedImportNotice : function()
//rwa 5-19-12  {
//rwa 5-19-12    var theMsg = "";
//rwa 5-19-12    var ext = "";
//rwa 5-19-12    var logInfo = "";
//rwa 5-19-12    if (this.importStatus == this.statusFailed)
//rwa 5-19-12    {
//rwa 5-19-12      theMsg = "imageProps.couldNotImport";
//rwa 5-19-12      if ((this.texImportStatus == this.statusSuccess) || this.canTypesetSourceFile() )
//rwa 5-19-12      {
//rwa 5-19-12        theMsg = "imageProps.couldNotImportButCanTeX";
//rwa 5-19-12        ext = getExtension(this.importTargFile.leafName);
//rwa 5-19-12      }
//rwa 5-19-12      logInfo = this.getImportLogInfo("import");
//rwa 5-19-12    }
//rwa 5-19-12    else if (this.texImportStatus == this.statusFailed)
//rwa 5-19-12    {
//rwa 5-19-12      theMsg = "imageProps.couldNotImportTeX";
//rwa 5-19-12      ext = getExtension(this.texTargFile.leafName);
//rwa 5-19-12      logInfo = this.getImportLogInfo("tex");
//rwa 5-19-12    }
//rwa 5-19-12    var msgParams = {file : this.sourceFile.path};
//rwa 5-19-12    if (ext.length)
//rwa 5-19-12      msgParams.extension = ext;
//rwa 5-19-12    var msgString = msiGetDialogString(theMsg, msgParams);
//rwa 5-19-12    if (logInfo && (logInfo.length > 0))
//rwa 5-19-12      msgString += "\n" + logInfo;
//rwa 5-19-12    var titleStr = msiGetDialogString("imageProps.importErrorTitle", msgParams);
//rwa 5-19-12    AlertWithTitle(titleStr, msgString);
//rwa 5-19-12  },
//rwa 5-19-12
//rwa 5-19-12  getImportLogInfo : function(mode)
//rwa 5-19-12  {
//rwa 5-19-12    var logFile = this.getImportLogFile(mode);
//rwa 5-19-12    var logUrl = msiFileURLFromFile( logFile );
//rwa 5-19-12    return getFileAsString(logUrl.spec);
//rwa 5-19-12  },
//rwa 5-19-12
//rwa 5-19-12  canTypesetSourceFile : function()
//rwa 5-19-12  {
//rwa 5-19-12    var ret = false;
//rwa 5-19-12    var ext = "";
//rwa 5-19-12    if (this.sourceFile)
//rwa 5-19-12      ext = getExtension(this.sourceFile.leafName);
//rwa 5-19-12    switch(ext)
//rwa 5-19-12    {
//rwa 5-19-12      case "eps":
//rwa 5-19-12      case "pdf":
//rwa 5-19-12      case "png":
//rwa 5-19-12      case "jpg":
//rwa 5-19-12        ret = true;
//rwa 5-19-12      break;
//rwa 5-19-12    }
//rwa 5-19-12    return ret;
//rwa 5-19-12  }
//rwa 5-19-12
//rwa 5-19-12};

function launchConvertingDialog(importData)
{
  window.openDialog("chrome://prince/content/msiGraphicsConversionDlg.xul", "graphicsConversionRunning", "chrome,close,resizable,titlebar,modal", importData);
  //Then collect data the dialog may have changed (notably if the user cancelled the conversion)
//  importTimerHandler.importProcess = importData.mImportProcess;
//  importTimerHandler.texImportProcess = importData.mTexImportProcess;
  importTimerHandler.importStatus = importData.mImportStatus;
  importTimerHandler.texImportStatus = importData.mTexImportStatus;

  importTimerHandler.checkFinalStatus();
}

//  inputFile - an nsIFile, the graphic file being converted
//  mode - a string, either "tex" or "import" (defaults to "import")
//  Returns nsIFile representing the graphic file being created.
function doGraphicsImport(inputFile, mode)
{
  if (!mode || !mode.length)
    mode = "import";
  var graphicDir = getDocumentGraphicsDir(mode);
  if (!graphicDir.exists())
    graphicDir.create(1, 0755);

  var timerHandler = new graphicsTimerHandler(1600, importTimerHandler);
  var theTimer = Components.classes["@mozilla.org/timer;1"].createInstance(Components.interfaces.nsITimer);
//  timerHandler.startLoading(inputFile, targFile, process, theTimer);
  if (mode=="tex")
    importTimerHandler.mTexHandler = timerHandler;
  else
    importTimerHandler.mImportHandler = timerHandler;
//  dump("\nIn msiEdImageProps.js, doGraphicsImport; calling graphicsConverterin mode " + mode + ".\n");
  var outputFile = graphicsConverter.doGraphicsImport(inputFile, graphicDir, mode, window, timerHandler);
  return outputFile;
}

//rwa 5-19-12var replaceableValues = ["targDirectory", "exepath", "inputFile", "outputFile", "commandLine"];
//rwa 5-19-12
//rwa 5-19-12//  aLine - string
//rwa 5-19-12//  substitutions - importLineSubstitutions or texLineSubstitutions object
//rwa 5-19-12function fixCommandFileLine(aLine, substitutions, bIsUnix)
//rwa 5-19-12{
//rwa 5-19-12  var retLine = aLine;
//rwa 5-19-12  var theSub = "";
//rwa 5-19-12  var aSub;
//rwa 5-19-12  var subRE;
//rwa 5-19-12  var subREStr="(%sub%)|(\\$\\{sub\\})|(\\$sub)";
//rwa 5-19-12  var unixRE=/%([^%]+)%/g;
//rwa 5-19-12  if ((retLine.indexOf("%") >= 0) || (retLine.indexOf("$") >= 0))
//rwa 5-19-12  {
//rwa 5-19-12    for (var ix = 0; ix < replaceableValues.length; ++ix)
//rwa 5-19-12    {
//rwa 5-19-12      aSub = replaceableValues[ix];
//rwa 5-19-12      subRE = new RegExp(subREStr.replace("sub",aSub,"g"),"g");
//rwa 5-19-12      if (subRE.test(retLine))
//rwa 5-19-12//      if (retLine.indexOf("%" + aSub + "%") >= 0)
//rwa 5-19-12      {
//rwa 5-19-12        theSub = "";
//rwa 5-19-12        if (substitutions[aSub] && substitutions[aSub].length)
//rwa 5-19-12          theSub = substitutions[aSub];
//rwa 5-19-12//        retLine = retLine.replace("%" + aSub + "%", theSub, "g");
//rwa 5-19-12        retLine = retLine.replace(subRE, theSub);
//rwa 5-19-12      }
//rwa 5-19-12    }
//rwa 5-19-12    if (bIsUnix)
//rwa 5-19-12      retLine = retLine.replace(unixRE,"$${$1}");
//rwa 5-19-12  }
//rwa 5-19-12  return retLine;
//rwa 5-19-12};
//rwa 5-19-12
//rwa 5-19-12//  fileTypeData - an object in the array returned from readInGraphicsFileData
//rwa 5-19-12//  inputNSFile - an nsIFile
//rwa 5-19-12function importLineSubstitutions(fileTypeData, inputNSFile, bIsUnix)
//rwa 5-19-12{
//rwa 5-19-12  this.targDirectory = getDocumentGraphicsDir().path;
//rwa 5-19-12  this.exepath = fileTypeData.exepath;
//rwa 5-19-12  this.inputFile = inputNSFile.path;
//rwa 5-19-12  this.outputExtension = fileTypeData.output;
//rwa 5-19-12  var extRE = new RegExp("\\." + fileTypeData.inFileType + "$", "i"); 
//rwa 5-19-12  this.outputFile = inputNSFile.leafName.replace(extRE, "." + fileTypeData.output);
//rwa 5-19-12  this.commandLine = "";
//rwa 5-19-12  this.commandLine = fixCommandFileLine(fileTypeData.commandLine, this, bIsUnix);
//rwa 5-19-12  if (this.commandLine.indexOf('>') < 0)
//rwa 5-19-12    this.commandLine +=  " >" + this.outputFile + ".log 2>&1";
//rwa 5-19-12  else
//rwa 5-19-12    this.commandLine += " 2>" + this.outputFile + ".log";
//rwa 5-19-12}
//rwa 5-19-12
//rwa 5-19-12//  fileTypeData - an object in the array returned from readInGraphicsFileData
//rwa 5-19-12//  inputNSFile - an nsIFile
//rwa 5-19-12function texLineSubstitutions(fileTypeData, inputNSFile, bIsUnix)
//rwa 5-19-12{
//rwa 5-19-12  this.targDirectory = getDocumentGraphicsDir("tex").path;
//rwa 5-19-12  this.exepath = fileTypeData.texexepath;
//rwa 5-19-12  this.inputFile = inputNSFile.path;
//rwa 5-19-12  this.outputExtension = fileTypeData.texoutput;
//rwa 5-19-12  var extRE = new RegExp("\\." + fileTypeData.inFileType + "$", "i");
//rwa 5-19-12  this.outputFile = inputNSFile.leafName.replace(extRE, "." + this.outputExtension);
//rwa 5-19-12  this.commandLine = "";
//rwa 5-19-12  this.commandLine = fixCommandFileLine(fileTypeData.texCommandLine, this, bIsUnix);
//rwa 5-19-12  if (this.commandLine.indexOf('>') < 0)
//rwa 5-19-12    this.commandLine +=  " >" + this.outputFile + ".log 2>&1";
//rwa 5-19-12  else
//rwa 5-19-12    this.commandLine += " 2>" + this.outputFile + ".log";
//rwa 5-19-12}
//rwa 5-19-12
//rwa 5-19-12//  commandFile - an nsILocalFile; the command file being created
//rwa 5-19-12//  templateFileLines - an array of strings, each representing a line from the template command file
//rwa 5-19-12//  fileTypeData - an object in the array returned from readInGraphicsFileData
//rwa 5-19-12//  inputFile - an nsIFile; the graphic file being converted
//rwa 5-19-12//  mode - a string, either "tex" or "import" (defaults to "import")
//rwa 5-19-12//Returns an nsILocalFile giving the graphics file to be created.
//rwa 5-19-12function prepareImport(commandFile, templateFileLines, fileTypeData, inputFile, mode, bIsUnix)
//rwa 5-19-12{
//rwa 5-19-12  var newLine = "";
//rwa 5-19-12  var outStr = "";
//rwa 5-19-12  var theSubs;
//rwa 5-19-12  try
//rwa 5-19-12  {
//rwa 5-19-12    if (mode == "tex")
//rwa 5-19-12      theSubs = new texLineSubstitutions(fileTypeData, inputFile, bIsUnix);
//rwa 5-19-12    else
//rwa 5-19-12      theSubs = new importLineSubstitutions(fileTypeData, inputFile, bIsUnix);
//rwa 5-19-12    for (var ix = 0; ix < templateFileLines.length; ++ix)
//rwa 5-19-12    {
//rwa 5-19-12      newLine = fixCommandFileLine(templateFileLines[ix], theSubs, bIsUnix);
//rwa 5-19-12      //now output newLine to the file we're going to run
//rwa 5-19-12      outStr += "\n" + newLine;
//rwa 5-19-12    }
//rwa 5-19-12  //  var commandFile = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
//rwa 5-19-12  //  commandFile.initWithPath(commandFilePath);
//rwa 5-19-12    if (commandFile.exists()) commandFile.remove(false);
//rwa 5-19-12    writeStringAsFile( outStr, commandFile, 0755 );
//rwa 5-19-12  }
//rwa 5-19-12  catch(exc) {dump("Exception in msiEdImageProps.js, prepareImport: [" + exc + "]\n"); return null;}
//rwa 5-19-12  if (!commandFile.exists())
//rwa 5-19-12    return null;
//rwa 5-19-12
//rwa 5-19-12  var outFile = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
//rwa 5-19-12  outFile.initWithPath(theSubs.targDirectory);
//rwa 5-19-12  outFile.append(theSubs.outputFile);
//rwa 5-19-12  return outFile;
//rwa 5-19-12}
//rwa 5-19-12
//rwa 5-19-12//  filterSourceFile - an nsIFile; the command file generated for this import
//rwa 5-19-12//  graphicsInFile - an nsIFile
//rwa 5-19-12//  graphicsOutFile - an nsILocalFile
//rwa 5-19-12//  mode - a string, either "tex" or "import" (defaults to "import")
//rwa 5-19-12//  Returns void
//rwa 5-19-12function runGraphicsFilter(filterSourceFile, graphicsInFile, graphicsOutFile, mode)
//rwa 5-19-12{
//rwa 5-19-12  var theProcess = Components.classes["@mozilla.org/process/util;1"].createInstance(Components.interfaces.nsIProcess);
//rwa 5-19-12//  if (mode == "tex") 
//rwa 5-19-12//    texImportProcess = theProcess;
//rwa 5-19-12//  else
//rwa 5-19-12//    gfxImportProcess = theProcess;
//rwa 5-19-12//
//rwa 5-19-12//  if (mode == "tex")
//rwa 5-19-12//  {
//rwa 5-19-12//    texImportTargFile = graphicsOutFile;
//rwa 5-19-12//    texImportTimer = theTimer;
//rwa 5-19-12//    theCallback = texImportTimerCallback;
//rwa 5-19-12//  }
//rwa 5-19-12//  else
//rwa 5-19-12//  {
//rwa 5-19-12//    importTargFile = graphicsOutFile;
//rwa 5-19-12//    importTimer = theTimer;
//rwa 5-19-12//    theCallback = importTimerCallback;
//rwa 5-19-12//  }
//rwa 5-19-12  importTimerHandler.startLoading(graphicsInFile, graphicsOutFile, theProcess, mode);
//rwa 5-19-12
//rwa 5-19-12  var outLogFile = importTimerHandler.getImportLogFile(mode);
//rwa 5-19-12//  var nDot = outLogFile.lastIndexOf(".");
//rwa 5-19-12//  outLogFile = outLogFile.substr(0,nDot) + ".log";
//rwa 5-19-12  theProcess.init(filterSourceFile);
//rwa 5-19-12//  theProcess.run(false, [">" + outLogFile + " 2>&1"], 1);
//rwa 5-19-12  theProcess.run(false, [], 0);
//rwa 5-19-12  var theTimer = Components.classes["@mozilla.org/timer;1"].createInstance(Components.interfaces.nsITimer);
//rwa 5-19-12  theTimer.initWithCallback( importTimerHandler, 200, Components.interfaces.nsITimer.TYPE_REPEATING_SLACK);
//rwa 5-19-12}
//rwa 5-19-12
//rwa 5-19-12//Returns a string (the .path of a file)
//rwa 5-19-12function getTemplateFileURI()
//rwa 5-19-12{
//rwa 5-19-12  var os = getOS(window);
//rwa 5-19-12  var extension = "";
//rwa 5-19-12  if (os == "win") extension = "cmd";
//rwa 5-19-12  else extension = "bash";
//rwa 5-19-12  var dsprops = Components.classes["@mozilla.org/file/directory_service;1"].createInstance(Components.interfaces.nsIProperties);
//rwa 5-19-12  var templateFile = dsprops.get("resource:app", Components.interfaces.nsILocalFile);
//rwa 5-19-12  templateFile.append("rungfxconv." + extension);
//rwa 5-19-12  var theURL = msiFileURLFromFile( templateFile );
//rwa 5-19-12  return theURL.spec;
//rwa 5-19-12}

function SetImport(bSet)
{
  gDialog.import = bSet;
}

function PreviewImageLoaded()
{
  if (gDialog.PreviewImage)
  {
//    dump("In PreviewImageLoaded! New offset size is [" + gDialog.PreviewImage.offsetWidth + "," + gDialog.PreviewImage.offsetHeight + "]\n");
//    dump("  Existing actual size is [" + gActualWidth + "," + gActualHeight + "]\n");
//    dump("  Current contents of size fields are [" + frameTabDlg.widthInput.value + "," + frameTabDlg.heightInput.value + "]\n");
    // Image loading has completed -- we can get actual width
    var bReset = false;
    if (gDialog.PreviewImage.offsetWidth && (gActualWidth != gDialog.PreviewImage.offsetWidth))
    {
      gConstrainWidth = gActualWidth  = gDialog.PreviewImage.offsetWidth;
      bReset = true;
    }
    if (gDialog.PreviewImage.offsetHeight && (gActualHeight != gDialog.PreviewImage.offsetHeight))
    {
      gConstrainHeight = gActualHeight = gDialog.PreviewImage.offsetHeight;
      bReset = true;
    }

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

      gDialog.PreviewWidth.setAttribute("value", gActualWidth+" pixels");
      gDialog.PreviewHeight.setAttribute("value", gActualHeight+" pixels");

      gDialog.PreviewSize.collapsed = false;
      gDialog.ImageHolder.collapsed = false;
      setContentSize(gActualWidth, gActualHeight);

//      dump("Before setActualOrDefaultSize(), contents of size fields are [" + frameTabDlg.widthInput.value + "," + frameTabDlg.heightInput.value + "]\n");

      if (frameTabDlg.actual.selected || bReset)
      {
        setActualOrDefaultSize();
      }

      doDimensionEnabling();
      SetSizeWidgets( Math.round(frameUnitHandler.getValueAs(frameTabDlg.widthInput.value,"px")), 
                      Math.round(frameUnitHandler.getValueAs(frameTabDlg.heightInput.value,"px")) );
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

function LoadPreviewImage()
{
  gDialog.PreviewSize.collapsed = true;

  var imageSrc = TrimString(gDialog.srcInput.value);
  if (!imageSrc)
    return;

  try {
    // Remove the image URL from image cache so it loads fresh
    //  (if we don't do this, loads after the first will always use image cache
    //   and we won't see image edit changes or be able to get actual width and height)
    
    var IOService = msiGetIOService();
    if (IOService)
    {
      // We must have an absolute URL to preview it or remove it from the cache
      imageSrc = msiMakeAbsoluteUrl(imageSrc);

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
    
  gDialog.PreviewImage = document.createElementNS("http://www.w3.org/1999/xhtml", "html:object");
  if (gDialog.PreviewImage)
  {
    // set the src before appending to the document -- see bug 198435 for why
    // this is needed.
    gDialog.PreviewImage.addEventListener("load", PreviewImageLoaded, true);
    gDialog.PreviewImage.data = imageSrc;
    var extension = getExtension(imageSrc);
    if (extension == "pdf")
      readSizeFromPDFFile(imageSrc);
    adjustObjectForFileType(gDialog.PreviewImage, extension);
    gDialog.ImageHolder.appendChild(gDialog.PreviewImage);
  }
}

//var pdfSetupStr = "#toolbar=0&statusbar=0&message=0&navpanes=0";
var pdfSetupStr = "#toolbar=0&statusbar=0&navpanes=0";

function adjustObjectForFileType(imageNode, extension)
{
  switch(extension.toLowerCase())
  {
    case "pdf":
      var theSrc = getSourceLocationFromElement(imageNode);
      if (theSrc.indexOf(pdfSetupStr) < 0)
        imageNode.setAttribute("data", theSrc + pdfSetupStr);
    break;
  }
}

function readSizeFromPDFFile(pdfSrc)
{
  var pdfSrcUrl = msiMakeAbsoluteUrl(pdfSrc);
  var theText = getFileAsString(pdfSrcUrl.spec);

//  var wdthRE = /(\/Width)\s+([0-9]+)/i;
//  var htRE = /(\/Height\s+([0-9]+)/i;
  var BBRE = /\/BBox\s*\[([0-9\.]+)\s+([0-9\.]+)\s+([0-9\.]+)\s+([0-9\.]+)\s*\]/;
  var mediaBoxRE = /\/MediaBox\s*\[([0-9\.]+)\s+([0-9\.]+)\s+([0-9\.]+)\s+([0-9\.]+)\s*\]/;

  function checkBoundingBox( objString )
  {
    var res = null;
    var bbArray = BBRE.exec(objString);
    if (bbArray && bbArray[1] && bbArray[2])
    {
      if (bbArray[3] && bbArray[4])
      {
        res = {wdth : Math.round(9.6 * (Number(bbArray[3]) - Number(bbArray[1]))),
               ht : Math.round(9.6 * (Number(bbArray[4]) - Number(bbArray[2]))) };   //9.6 here is 96 pts-per-inch/10 since these are given in tenths of an inch
      }
      else  //this shouldn't happen, though
      {
        res = {wdth : Math.round(9.6 * Number(bbArray[1])),
               ht : Math.round(9.6 * Number(bbArray[2])) };   //9.6 here is 96 pts-per-inch/10 since these are given in tenths of an inch
      }
    }
    return res;
  }

  function checkMediaBox( objString )
  {
    var res = null;
    var dimsArray = mediaBoxRE.exec(objString);
    if (dimsArray && dimsArray[1] && dimsArray[2])
    {
      if (dimsArray[3] && dimsArray[4])
      {
        res = { wdth : Math.round(Number(dimsArray[3]) - Number(dimsArray[1])),
                ht : Math.round(Number(dimsArray[4]) - Number(dimsArray[2])) };
      }
      else  //this shouldn't happen, though
      {
        res = { wdth : Math.round(Number(dimsArray[1])),
                ht : Math.round(Number(dimsArray[2])) };
      }
    }
    return res;
  }

  var htWdth = null;
  var pdfStart = theText.indexOf("%PDF");
  if (pdfStart < 0)
    return;
  var xObj = theText.indexOf("/XObject", pdfStart);
  var objEnd = pdfStart;
  while (!htWdth && (xObj > 0))  //look for /XObject spec first
  {
    objEnd = theText.indexOf("endobj", xObj);
    htWdth = checkBoundingBox( theText.substring(xObj, objEnd) );
    if (!htWdth)
      xObj = theText.indexOf("/XObject", objEnd);
  }
  if (!htWdth)
  {
    var pageDesc = theText.indexOf("/Page", pdfStart);
    objEnd = pdfStart;
    while (!htWdth && (pageDesc > 0))  //look for /XObject spec first
    {
      objEnd = theText.indexOf("endobj", pageDesc);
      htWdth = checkMediaBox( theText.substring(pageDesc, objEnd) );
      if (!htWdth)
        pageDesc = theText.indexOf("/Page", objEnd);
    }
  }

  if (htWdth)
  {
    var bReset = false;
    if (gActualWidth != htWdth.wdth)
    {
      gConstrainWidth = gActualWidth  = htWdth.wdth;
      bReset = true;
    }
    if (gActualHeight != htWdth.ht)
    {
      gConstrainHeight = gActualHeight = htWdth.ht;
      bReset = true;
    }
    if (frameTabDlg.actual.selected || bReset)
      setActualOrDefaultSize();

    SetSizeWidgets( Math.round(frameUnitHandler.getValueAs(frameTabDlg.widthInput.value,"px")), 
                    Math.round(frameUnitHandler.getValueAs(frameTabDlg.heightInput.value,"px")) );
  }
}

function setActualOrDefaultSize()
{
  var prefStr = "";
  var bUseDefaultWidth, bUseDefaultHeight;
  var width, height;
  try {bUseDefaultWidth = GetBoolPref("swp.graphicsUseDefaultWidth");}
  catch(ex) 
  {bUseDefaultWidth = false; dump("Exception getting pref swp.graphicsUseDefaultWidth: " + ex + "\n");}

  try {bUseDefaultHeight = GetBoolPref("swp.graphicsUseDefaultHeight");}
  catch(ex) {bUseDefaultHeight = false; dump("Exception getting pref swp.graphicsUseDefaultHeight: " + ex + "\n");}

  if (bUseDefaultWidth || bUseDefaultHeight)
  {
    frameTabDlg.sizeRadioGroup.selectedItem = frameTabDlg.custom;
    try {prefStr = GetStringPref("swp.defaultGraphicsSizeUnits");}
    catch(ex) {prefStr = ""; dump("Exception getting pref swp.defaultGraphicsSizeUnits: " + ex + "\n");}
    if (prefStr.length)
      gDefaultUnit = prefStr;
    frameUnitHandler.setCurrentUnit(gDefaultUnit);
    frameTabDlg.frameUnitMenulist.value = gDefaultUnit;
  }
  if (bUseDefaultWidth)
  {
    frameTabDlg.autoWidthCheck.checked = false;
    try {prefStr = GetStringPref("swp.defaultGraphicsHSize");}
    catch(ex) {prefStr = ""; dump("Exception getting pref swp.defaultGraphicsHSize: " + ex + "\n");}
    if (prefStr.length)
    {
      width = frameUnitHandler.getValueFromString( prefStr, gDefaultUnit );
      gDefaultWidth = Math.round(frameUnitHandler.getValueAs(width, "px"));
      frameTabDlg.widthInput.value = width;
    }
    if (bUseDefaultHeight)
    {
      try {prefStr = GetStringPref("swp.defaultGraphicsVSize");}
      catch(ex) {prefStr = ""; dump("Exception getting pref swp.defaultGraphicsVSize: " + ex + "\n");}
      if (prefStr.length)
      {
        height = frameUnitHandler.getValueFromString( prefStr, gDefaultUnit );
        gDefaultHeight = Math.round(frameUnitHandler.getValueAs(height,"px"));
      }
      frameTabDlg.autoHeightCheck.checked = false;
      frameTabDlg.constrainCheckbox.checked = false;
      frameTabDlg.heightInput.value = height;
    }
    else
    {
      frameTabDlg.autoHeightCheck.checked = true;
      frameTabDlg.constrainCheckbox.checked = true;
      frameTabDlg.constrainCheckbox.disabled = false;
      constrainProportions( "frameWidthInput", "frameHeightInput", null );
    }
  }
  else if (bUseDefaultHeight)
  {
    try {prefStr = GetStringPref("swp.defaultGraphicsVSize");}
    catch(ex) {prefStr = ""; dump("Exception getting pref swp.defaultGraphicsVSize: " + ex + "\n");}
    if (prefStr.length)
    {
      height = frameUnitHandler.getValueFromString( prefStr, gDefaultUnit );
      gDefaultHeight = Math.round(frameUnitHandler.getValueAs(height,"px"));
    }
    frameTabDlg.autoWidthCheck.checked = true;
    frameTabDlg.autoHeightCheck.checked = false;
    frameTabDlg.constrainCheckbox.disabled = false;
    frameTabDlg.constrainCheckbox.checked = true;
    frameTabDlg.heightInput.value = height;
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
//  totWidth += 2* frameUnitHandler.getValueAs(Number(frameTabDlg.marginInput.left.value), "px");
  totWidth += 2 * frameUnitHandler.getValueAs(Number(frameTabDlg.borderInput.left.value), "px");
  totWidth += 2 * frameUnitHandler.getValueAs(Number(frameTabDlg.paddingInput.left.value), "px");
  return totWidth;
}

function readTotalExtraHeight(unit)
{
  var totHeight = 0;
//  totHeight += 2 * frameUnitHandler.getValueAs(Number(frameTabDlg.marginInput.top.value), "px");
  totHeight += 2 * frameUnitHandler.getValueAs(Number(frameTabDlg.borderInput.left.value), "px");
  totHeight += 2 * frameUnitHandler.getValueAs(Number(frameTabDlg.paddingInput.left.value), "px");
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

//function ChangeImageSrc()
//{
//  if (gTimerID)
//    clearTimeout(gTimerID);
//
//  gTimerID = setTimeout("LoadPreviewImage()", 800);
//
//  InitImage();
//  msiSetRelativeCheckbox();
//  doOverallEnabling();
//}

//function doDimensionEnabling()
//{
//  // Enabled only if "Custom" is selected
//  var enable = (frameTabDlg.custom.selected);
//
//  // BUG 74145: After input field is disabled,
//  //   setting it enabled causes blinking caret to appear
//  //   even though focus isn't set to it.
//  SetElementEnabledById( "heightInput", enable );
//  SetElementEnabledById( "heightLabel", enable );
//
//  SetElementEnabledById( "widthInput", enable );
//  SetElementEnabledById( "widthLabel", enable);
//
//  SetElementEnabledById( "unitList", enable );
//
//  var constrainEnable = enable ;
////         && ( gDialog.widthUnitsMenulist.selectedIndex == 0 )
////         && ( gDialog.heightUnitsMenulist.selectedIndex == 0 );
//
//  SetElementEnabledById( "constrainCheckbox", constrainEnable );
//
//}
//
function doOverallEnabling()
{
  var enabled = TrimString(gDialog.srcInput.value) != "";

  SetElementEnabled(gDialog.OkButton, enabled);
  SetElementEnabledById("AdvancedEditButton1", enabled);
  doDimensionEnabling();
}

//function editImageMap()
//{
//  // Create an imagemap for image map editor
//  if (gInsertNewIMap)
//  {
//    try {
//      var editorElement = msiGetParentEditorElementForDialog(window);
//      var editor = msiGetEditor(editorElement);
//      gImageMap = editor.createElementWithDefaults("map");
//    } catch (e) {}
//  }
//
//  // Note: We no longer pass in a copy of the global ImageMap. ImageMap editor should create a copy and manage onOk and onCancel behavior
//  window.openDialog("chrome://editor/content/EdImageMap.xul", "imagemap", "chrome,close,titlebar,modal", globalElement, gImageMap);
//}
//
//function removeImageMap()
//{
//  gRemoveImageMap = true;
//  gCanRemoveImageMap = false;
//  SetElementEnabledById("removeImageMap", false);
//}

//function SwitchToValidatePanel()
//{
//  if (gDialog.tabBox && gDialog.tabBox.selectedTab != gValidateTab)
//    gDialog.tabBox.selectedTab = gValidateTab;
//}

// Get data from widgets, validate, and set for the global element
//   accessible to AdvancedEdit() [in msiEdDialogCommon.js]
function ValidateImage()
{
//  dump("in ValidateImage()\n");
//  var editorElement = msiGetParentEditorElementForDialog(window);
  var editor = msiGetEditor(gEditorElement);
//  var editor = GetCurrentEditor();
  if (!editor)
    return false;

//  gValidateTab = gDialog.tabLocation;
//  dump("1\n");
  if (!gDialog.srcInput.value)
  {
    AlertWithTitle(null, GetString("MissingImageError"));
//    SwitchToValidatePanel();
    gDialog.srcInput.focus();
    return false;
  }

  //TODO: WE NEED TO DO SOME URL VALIDATION HERE, E.G.:
  // We must convert to "file:///" or "http://" format else image doesn't load!
//  dump("2\n");
  var src = TrimString(gDialog.srcInput.value);
  globalImage.setAttribute("src", src);

//  var title = TrimString(gDialog.titleInput.value);
//  if (title)
//    globalElement.setAttribute("title", title);
//  else
//    globalElement.removeAttribute("title");

  var alt = TrimString(gDialog.altTextInput.value);

//  dump("3\n");
  globalImage.setAttribute("alt", alt);

  if (gDialog.keyInput.controller)
  {
//    dump("In msiEdImageProps.js, keyInput autosearch controller's status is [" + gDialog.keyInput.controller.searchStatus + "]\n");
    var oldKey = imageElement.getAttribute("key");
    var bUnchanged = false;
    if (!oldKey || !oldKey.length)
      bUnchanged = (!gDialog.keyInput.value || !gDialog.keyInput.value.length);
    else if (gDialog.keyInput.value.length)
      bUnchanged = (gDialog.keyInput.value == oldKey);
    if (!bUnchanged && gDialog.keyInput.value && gDialog.keyInput.value.length && (gDialog.keyInput.controller.searchStatus == 4))  //found a match!
    {
      msiPostDialogMessage("dlgErrors.markerInUse", {markerString : gDialog.keyInput.value});
      gDialog.keyInput.focus();
      return false;
    }
  }

  var width = "";
  var height = "";

//  gValidateTab = gDialog.tabDimensions;
  if (!frameTabDlg.actual.selected)
  {
    // Get user values for width and height
    width = msiValidateNumber(frameTabDlg.widthInput, gDialog.widthUnitsMenulist, 1, gMaxPixels, 
                           globalImage, "width", false, true);
    if (gValidationError)
      return false;

    height = msiValidateNumber(frameTabDlg.heightInput, gDialog.heightUnitsMenulist, 1, gMaxPixels, 
                            globalImage, "height", false, true);
    if (gValidationError)
      return false;
  }

  // We always set the width and height attributes, even if same as actual.
  //  This speeds up layout of pages since sizes are known before image is loaded
  if (!width)
    width = gActualWidth;
  if (!height)
    height = gActualHeight;
//  dump("4\n");

  // Remove existing width and height only if source changed
  //  and we couldn't obtain actual dimensions
  var srcChanged = (src != gOriginalSrc);
  if (width)
    globalImage.setAttribute("width", width);
  else if (srcChanged)
    editor.removeAttributeOrEquivalent(globalImage, "width", true);

  if (height)
    globalImage.setAttribute("height", height);
  else if (srcChanged) 
    editor.removeAttributeOrEquivalent(globalImage, "height", true);

  // spacing attributes
//  gValidateTab = gDialog.tabBorder;
//  dump("5\n");
//  msiValidateNumber(gDialog.imagelrInput, null, 0, gMaxPixels, 
//                 globalElement, "hspace", false, true, true);
//  if (gValidationError)
//    return false;

//  msiValidateNumber(gDialog.imagetbInput, null, 0, gMaxPixels, 
//                 globalElement, "vspace", false, true);
//  if (gValidationError)
//    return false;

  // note this is deprecated and should be converted to stylesheets
//  msiValidateNumber(gDialog.border, null, 0, gMaxPixels, 
//                 globalElement, "border", false, true);
//  if (gValidationError)
//    return false;
//  dump("6\n");

  // Default or setting "bottom" means don't set the attribute
  // Note that the attributes "left" and "right" are opposite
  //  of what we use in the UI, which describes where the TEXT wraps,
  //  not the image location (which is what the HTML describes)
//  switch ( gDialog.alignTypeSelect.value )
//  {
//    case "top":
//    case "middle":
//    case "right":
//    case "left":
//      globalElement.setAttribute( "align", gDialog.alignTypeSelect.value );
//      break;
//    default:
//      try {
//        editor.removeAttributeOrEquivalent(globalElement, "align", true);
//      } catch (e) {}
//  }
//  dump("7\n");

  return true;
}

//function doHelpButton()
//{
//  openHelp("image_properties");
//  return true;
//}

function imageLoaded(event)
{
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

function onAccept()
{
  // Use this now (default = false) so Advanced Edit button dialog doesn't trigger error message
  importTimerHandler.stopLoading();

  gDoAltTextError = true;
//  dump("**************************************************************************\n");
//  dump("in onAccept\n");
  if (ValidateData())
  {
    var editorElement = msiGetParentEditorElementForDialog(window);
    var editor = msiGetEditor(gEditorElement);

    editor.beginTransaction();
    try
    {
      var tagname="object";
//      var frameElement = null;
      gCaptionData.m_captionStr = getCaptionEditContents();
      var bHasCaption = (gCaptionData.m_captionStr && gCaptionData.m_captionStr.length);
      var bHasCaption = true;
      var posInParent;
      if (imageElement)
      {
        imgExists = true;
//        frameElement = imageElement.parentNode;
//        if (!frameElement || frameElement.localName != 'msiframe')
//        {
//          var newframe = editor.createElementWithDefaults("msiframe");
//          if (frameElement) frameElement.removeChild(imageElement); 
//          newframe.appendChild(imageElement);
//          if (frameElement) frameElement.appendChild(newframe);
//          frameElement = newframe;
//        }
      }
      else
      {      
        imageElement = editor.createElementWithDefaults(tagname);
        imageElement.addEventListener("load", imageLoaded, true);
        wrapperElement = imageElement;
      }

      if (bHasCaption)
      {
        if (wrapperElement == imageElement)
        {
          wrapperElement = editor.createElementWithDefaults("msiframe");
          wrapperElement.setAttribute("frametype", "image");
          if (gInsertNewImage)
            wrapperElement.appendChild(imageElement);
          else
          {
            posInParent = msiNavigationUtils.offsetInParent(imageElement);
            var imageParent = imageElement.parentNode;
            editor.deleteNode(imageElement);
            wrapperElement.appendChild(imageElement);
//            editor.insertNode(imageElement, wrapperElement, 0);
            editor.insertNode(wrapperElement, imageParent, posInParent);
          }
        }
      }
      else if (wrapperElement != imageElement)  //Can only happen in the !gInsertNewImage case, if there was a caption previously
      {
        posInParent = msiNavigationUtils.offsetInParent(wrapperElement);
        editor.insertNode(imageElement, wrapperElement.parentNode, posInParent);
        editor.deleteNode(wrapperElement);
        wrapperElement = imageElement;
//        frameElement = editor.createElementWithDefaults("msiframe");
//        frameElement.appendChild(imageElement);
      }

      var capData = findCaptionNodes(wrapperElement);
      var capPosition = gDialog.captionPlacementGroup.value;
      if (!capPosition || !capPosition.length)
        capPosition = "below";
//      syncCaptionAndExisting(gCaptionData.m_captionStr.below, capData.belowCaption, editor, wrapperElement, "below");
//      var capAttrStr = "";
//      if (capData.aboveCaption && capData.aboveCaption.length)
//        capAttrStr = "above";
//      if (capData.belowCaption && capData.belowCaption.length)
//        capAttrStr += "below";
//      if (!capAttrStr.length)
//        capAttrStr = null;  //since EnsureElementAttribute will remove the attribute if a null is passed in.
      
          // 'true' means delete the selection before inserting
//      var oldWrapper = wrapperElement;
//      var oldImage = imageElement;
      if (gInsertNewImage)
        editor.insertElementAtSelection(wrapperElement, true);
//      if (msiGetBaseNodeName(wrapperElement) == "msiframe")
//      {
//        wrapperElement = msiEditorFindJustInsertedElement("msiframe", editor);
//        imageElement = msiNavigationUtils.getChildrenByTagName(wrapperElement, "object")[0];
//      }
//      else
//        wrapperElement = imageElement = msiEditorFindJustInsertedElement("object", editor);
//      dump("In msiEdImageProps.onAccept(), oldWrapper is [" + ((wrapperElement == oldWrapper) ? "same as" : "different from") + "] one in document.\n");
//      dump("In msiEdImageProps.onAccept(), oldImage is [" + ((imageElement == oldImage) ? "same as" : "different from") + "] one in document.\n");
      syncCaptionAndExisting(gCaptionData.m_captionStr, editor, wrapperElement, capPosition);
    
      var attrList = ["src,data,title,alt"];
      msiCopySpecifiedElementAttributes(imageElement, globalElement, editor, attrList);
      imageElement.setAttribute("data",gDialog.srcInput.value);
      var extension = getExtension(gDialog.srcInput.value);
      adjustObjectForFileType(imageElement, extension);
      msiEditorEnsureElementAttribute(imageElement, "req", "graphicx", editor);
      msiEditorEnsureElementAttribute(imageElement, "naturalWidth", frameUnitHandler.getValueOf(gConstrainWidth, "px"), editor);
      msiEditorEnsureElementAttribute(imageElement, "naturalHeight", frameUnitHandler.getValueOf(gConstrainHeight, "px"), editor);
      
      setFrameAttributes(wrapperElement, imageElement);
//      msiEditorEnsureElementAttribute(wrapperElement, "captionLoc", capAttrStr, editor);  //Now taken care of above

      var theKey = gDialog.keyInput.value;
      if (!theKey.length)
        theKey = null;
      msiEditorEnsureElementAttribute(imageElement, "key", theKey, editor);
      msiEditorEnsureElementAttribute(imageElement, "id", theKey, editor);
    }
    catch (e)
    {
      dump(e);
    }

    editor.endTransaction();

    SaveWindowLocation();
    return true;
  }
  else dump("Validation FAILED\n");

  gDoAltTextError = false;

  return true;
}

function onCancel()
{
  importTimerHandler.stopLoading();
  SaveWindowLocation();
  return true;
}


//// These mode variables control the msiFrameOverlay code
//var gFrameModeImage = true;
//var gFrameModeTextFrame = false;
//
//var gInsertNewImage;
//
//var xsltSheet="<?xml version='1.0'?><xsl:stylesheet version='1.1' xmlns:xsl='http://www.w3.org/1999/XSL/Transform' xmlns:html='http://www.w3.org/1999/xhtml' ><xsl:output method='text' encoding='UTF-8'/> <xsl:template match='/'>  <xsl:apply-templates select='//*[@key]'/></xsl:template><xsl:template match='//*[@key]'>   <xsl:value-of select='@key'/><xsl:text> </xsl:text></xsl:template> </xsl:stylesheet>";

function initKeyList()
{

  gDialog.markerList = new msiKeyMarkerList(window);
  gDialog.markerList.setUpTextBoxControl(gDialog.keyInput);
}

function getGraphicsImportFilterString()
{
  var convertData = graphicsConverter.getConvertibleFileTypes(window);
  var typeArray = [];
  for (var jx = 0; jx < nativeGraphicTypes.length; ++jx)
    typeArray.push("*." + nativeGraphicTypes[jx]);
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

//rwa5-19-12function getImportDataForGraphicType(extension)
//rwa5-19-12{
//rwa5-19-12  var convertData = getGraphicsFileTypesData();
//rwa5-19-12  for (var ix = 0; ix < convertData.length; ++ix)
//rwa5-19-12  {
//rwa5-19-12    if (convertData[ix].inFileType == extension.toLowerCase())
//rwa5-19-12      return convertData[ix];
//rwa5-19-12  }
//rwa5-19-12  return null;
//rwa5-19-12}
//rwa5-19-12
//rwa5-19-12var sectionRE = /^\s*\[([^\]]+)\]/;
//rwa5-19-12var keyValueRE = /^([^=]+)=(.*)$/;
//rwa5-19-12
//rwa5-19-12function getGraphicsFileTypesData(bForceReload)
//rwa5-19-12{
//rwa5-19-12  if (!gDialog.graphicsConvertData || bForceReload)
//rwa5-19-12    gDialog.graphicsConvertData = readInGraphicsFileData("chrome://user/content/gfximport.ini");
//rwa5-19-12  return gDialog.graphicsConvertData;
//rwa5-19-12}
//rwa5-19-12
//rwa5-19-12function readInGraphicsFileData(fileURI)
//rwa5-19-12{
//rwa5-19-12  var theLines = GetLinesFromFile(fileURI);
//rwa5-19-12  var gfxList = [];
//rwa5-19-12  var sectionData = {mFileType : "", mStart : 0, mEnd : 0};
//rwa5-19-12  var found = null;
//rwa5-19-12  var theKey, theValue;
//rwa5-19-12  var newData;
//rwa5-19-12  while (FindNextBracketDelimitedSection(theLines, sectionData))
//rwa5-19-12  {
//rwa5-19-12    if (sectionData.mFileType.length)
//rwa5-19-12    {
//rwa5-19-12      newData = {inFileType : sectionData.mFileType.toLowerCase()};
//rwa5-19-12      for (var ix = sectionData.mStart+1; ix <= sectionData.mEnd; ++ix)
//rwa5-19-12      {
//rwa5-19-12        found = keyValueRE.exec(TrimString(theLines[ix]));
//rwa5-19-12        if (found && found[1] && found[1].length && found[2] && found[2].length)
//rwa5-19-12        {
//rwa5-19-12          theKey = TrimString(found[1]);
//rwa5-19-12          theValue = TrimString(found[2]);
//rwa5-19-12          newData[theKey] = theValue;
//rwa5-19-12        }
//rwa5-19-12      }
//rwa5-19-12      gfxList.push(newData);
//rwa5-19-12    }
//rwa5-19-12    sectionData.mStart = sectionData.mEnd + 1;
//rwa5-19-12  }
//rwa5-19-12  return gfxList;
//rwa5-19-12}
//rwa5-19-12
//rwa5-19-12function GetLinesFromFile(aFileUrl)
//rwa5-19-12{
//rwa5-19-12  var theText = getFileAsString(aFileUrl);
//rwa5-19-12  if (!theText)
//rwa5-19-12    theText = "";
//rwa5-19-12  // read lines into array
//rwa5-19-12  var lines = theText.split("\n");
//rwa5-19-12  return lines;
//rwa5-19-12}
//rwa5-19-12
//rwa5-19-12function FindNextBracketDelimitedSection(lineList, sectionData)
//rwa5-19-12{
//rwa5-19-12  var found = null, foundLine = null;
//rwa5-19-12  var foundStart = false;
//rwa5-19-12  var foundEnd = false;
//rwa5-19-12  for (var ix = sectionData.mStart; ix < lineList.length; ++ix)
//rwa5-19-12  {
//rwa5-19-12    if (lineList[ix].charAt(0) == ";")  //skip comment lines!
//rwa5-19-12      continue;
//rwa5-19-12    foundLine = TrimString(lineList[ix]);
//rwa5-19-12    if (!foundLine || !foundLine.length)
//rwa5-19-12      continue;
//rwa5-19-12    if (found = sectionRE.exec(foundLine))
//rwa5-19-12    {
//rwa5-19-12      if (!foundStart)
//rwa5-19-12      {
//rwa5-19-12        sectionData.mStart = ix;
//rwa5-19-12        sectionData.mFileType = found[1];
//rwa5-19-12        foundStart = true;
//rwa5-19-12      }
//rwa5-19-12      else
//rwa5-19-12      {
//rwa5-19-12        foundEnd = true;
//rwa5-19-12        sectionData.mEnd = ix-1;
//rwa5-19-12        break;
//rwa5-19-12      }
//rwa5-19-12    }
//rwa5-19-12  }
//rwa5-19-12  if (!foundEnd)
//rwa5-19-12      sectionData.mEnd = ix-1;
//rwa5-19-12  return foundStart;
//rwa5-19-12}
