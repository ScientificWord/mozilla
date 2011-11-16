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
var frameTabDlg = new Object();

//var gConstrainWidth  = 0;  //Moved to msiFrameOverlay
//var gConstrainHeight = 0;  //Moved to msiFrameOverlay
var imageElement;
var wrapperElement;
var gActualWidth = "";
var gActualHeight = "";
var gDefaultWidth = 200;
var gDefaultHeight = 100;
var gOriginalSrc = "";
var gHaveDocumentUrl = false;

// These must correspond to values in EditorDialog.css for each theme
// (unfortunately, setting "style" attribute here doesn't work!)
var gPreviewImageWidth = 80;
var gPreviewImageHeight = 50;

// These mode variables control the msiFrameOverlay code
var gFrameModeImage = true;
var gFrameModeTextFrame = false;

var gInsertNewImage;
var gCaptionData;

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
    wrapperElement = window.arguments[0];
    if (wrapperElement && (msiGetBaseNodeName(wrapperElement) == "msiframe"))
      imageElement = wrapperElement.getElementsByTagName("object")[0];
    else
      imageElement = wrapperElement;
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

  initFrameTab(frameTabDlg, wrapperElement, imageElement, gInsertNewImage);
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


// Set dialog widgets with attribute data
// We get them from globalElement copy so this can be used
//   by AdvancedEdit(), which is shared by all property dialogs
function InitImage()
{
  // Set the controls to the image's attributes
  if (imageElement.hasAttribute("src"))
    gDialog.srcInput.value = imageElement.getAttribute("src");
  else if (imageElement.hasAttribute("data"))
    gDialog.srcInput.value = imageElement.getAttribute("data");

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
  if (!gInsertNewImage)
  {
    // We found an element and don't need to insert one
  //trim off units at end
    var re = /[a-z]*/gi;
    var widthStr = "";
    var heightStr = "";
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
  frameTabDlg.unitList.value = "pt";

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
    
  gDialog.PreviewImage = document.createElementNS("http://www.w3.org/1999/xhtml", "html:object");
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

function getDocumentGraphicsDir()
{
  var docUrlString = msiGetDocumentBaseUrl();
  var docurl = msiURIFromString(docUrlString);
  var dir = msiFileFromFileURL(docurl);
  dir = dir.parent;
  dir.append("graphics");
  return dir;
}

var isSVGFile = false;
function chooseFile()
{
//  if (gTimerID)
//    clearTimeout(gTimerID);
  // Get a local file, converted into URL format
  var fileName = GetLocalFileURL(["img"]); // return a URLString
  if (fileName)
  {
    var url = msiURIFromString(fileName);
    if (gDialog.import) // copy the file into the graphics directory
    {
      try {
        var file = msiFileFromFileURL(url);
        isSVGFile = /\.svg$/.test(file.leafName);
        var dir = getDocumentGraphicsDir()
        if (!dir.exists()) dir.create(1, 0755);
        file.permissions = 0755;
        fileName = "graphics/"+file.leafName;
        var newFile = dir.clone();
        newFile.append(file.leafName);
        if (newFile.exists()) newFile.remove(false);
        file.copyTo(dir,"");    
      }
      catch(e)
      {
        dump("exception: e="+e.msg);
      }
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
  LoadPreviewImage();
  // copy to the graphics directory
  // Put focus into the input field
  SetTextboxFocus(gDialog.srcInput);
}

function SetImport(bSet)
{
  gDialog.import = bSet;
}

function PreviewImageLoaded()
{
  if (gDialog.PreviewImage)
  {
    dump("In PreviewImageLoaded! New offset size is [" + gDialog.PreviewImage.offsetWidth + "," + gDialog.PreviewImage.offsetHeight + "]\n");
    dump("  Existing actual size is [" + gActualWidth + "," + gActualHeight + "]\n");
    dump("  Current contents of size fields are [" + frameTabDlg.widthInput.value + "," + frameTabDlg.heightInput.value + "]\n");
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

      dump("Before setActualSize(), contents of size fields are [" + frameTabDlg.widthInput.value + "," + frameTabDlg.heightInput.value + "]\n");
      if (frameTabDlg.actual.selected || bReset)
        setActualSize();

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
    gDialog.ImageHolder.appendChild(gDialog.PreviewImage);
  }
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
  dump("in ValidateImage()\n");
//  var editorElement = msiGetParentEditorElementForDialog(window);
  var editor = msiGetEditor(gEditorElement);
//  var editor = GetCurrentEditor();
  if (!editor)
    return false;

//  gValidateTab = gDialog.tabLocation;
  dump("1\n");
  if (!gDialog.srcInput.value)
  {
    AlertWithTitle(null, GetString("MissingImageError"));
//    SwitchToValidatePanel();
    gDialog.srcInput.focus();
    return false;
  }

  //TODO: WE NEED TO DO SOME URL VALIDATION HERE, E.G.:
  // We must convert to "file:///" or "http://" format else image doesn't load!
  dump("2\n");
  var src = TrimString(gDialog.srcInput.value);
  globalImage.setAttribute("src", src);

//  var title = TrimString(gDialog.titleInput.value);
//  if (title)
//    globalElement.setAttribute("title", title);
//  else
//    globalElement.removeAttribute("title");

  var alt = TrimString(gDialog.altTextInput.value);

  dump("3\n");
  globalImage.setAttribute("alt", alt);

  if (gDialog.keyInput.controller)
  {
    dump("In msiEdImageProps.js, keyInput autosearch controller's status is [" + gDialog.keyInput.controller.searchStatus + "]\n");
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
  dump("4\n");

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
  dump("5\n");
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
  dump("6\n");

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
  dump("7\n");

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
  gDoAltTextError = true;
  dump("**************************************************************************\n");
  dump("in onAccept\n");
  if (ValidateData())
  {
//    var editorElement = msiGetParentEditorElementForDialog(window);
    var editor = msiGetEditor(gEditorElement);

    editor.beginTransaction();
    try
    {
      var tagname="object";
//      var frameElement = null;
      gCaptionData.m_captionStr = getCaptionEditContents();
      var bHasCaption = (gCaptionData.m_captionStr && gCaptionData.m_captionStr.length);
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
//  gDialog.markerList.setUpTextBoxControl(gDialog.linkKeyInput);

//  var keyString = gDialog.markerList.getIndexString();
//  gDialog.keyInput.setAttribute("autocompletesearchparam", keyString);
//  gDialog.linkKeyInput.setAttribute("autocompletesearchparam", keyString);

//  var editorElement = msiGetActiveEditorElement();
//  var editor;
//  if (editorElement) editor = msiGetEditor(editorElement);
//  var parser = new DOMParser();
//  var dom = parser.parseFromString(xsltSheet, "text/xml");
//  dump(dom.documentElement.nodeName == "parsererror" ? "error while parsing" + dom.documentElement.textContents : dom.documentElement.nodeName);
//  var processor = new XSLTProcessor();
//  processor.importStylesheet(dom.documentElement);
//  var newDoc;
//  if (editor) newDoc = processor.transformToDocument(editor.document, document);
//  dump(newDoc.documentElement.localName+"\n");
//  var keyString = newDoc.documentElement.textContent;
//  var keys = keyString.split(/\s+/);
//  var i;
//  var len;
//  keys.sort();
//  var lastkey = "";
//  for (i=keys.length-1; i >= 0; i--)
//  {
//    if (keys[i] == "" || keys[i] == lastkey) keys.splice(i,1);
//    else lastkey = keys[i];
//  }  
//  var ACSA = Components.classes["@mozilla.org/autocomplete/search;1?name=stringarray"].getService();
//  ACSA.QueryInterface(Components.interfaces.nsIAutoCompleteSearchStringArray);
//  ACSA.resetArray("keys");
//  for (i=0, len=keys.length; i<len; i++)
//  {
//    if (keys[i].length > 0) 
//      ACSA.addString("keys",keys[i]);
//  }
//  dump("Keys are : "+keys.join()+"\n");    
}