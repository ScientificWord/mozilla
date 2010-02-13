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

// var gDialog;
var globalElement;
Components.utils.import("resource://app/modules/unitHandler.jsm");
var imageUnitHandler = new UnitHandler();
var frameTabDlg = new Object();

var gConstrainWidth  = 0;
var gConstrainHeight = 0;
var imageElement;
var gImageMap = 0;
//var gCanRemoveImageMap = false;
//var gRemoveImageMap = false;
//var gImageMapDisabled = false;
var gActualWidth = "";
var gActualHeight = "";
var gOriginalSrc = "";
var gHaveDocumentUrl = false;
var gTimerID;
var gValidateTab;

// These must correspond to values in EditorDialog.css for each theme
// (unfortunately, setting "style" attribute here doesn't work!)
var gPreviewImageWidth = 80;
var gPreviewImageHeight = 50;


// dialog initialization code

function Startup()
{
  debugger;
  var editorElement = msiGetParentEditorElementForDialog(window);
  var editor = msiGetEditor(editorElement);
  if (!editor)
  {
    window.close();
    return;
  }

  initKeyList();
  gDialog = new Object();
  gDialog.import            = document.getElementById( "importRefRadioGroup").selectedIndex == 0;
  gDialog.tabBox            = document.getElementById( "TabBox" );
  gDialog.tabPicture        = document.getElementById( "imagePictureTab" );
  gDialog.tabPlacement      = document.getElementById( "msiPlacementTab" );
  gDialog.tabFrame          = document.getElementById( "msiFrameTab" );
  gDialog.tabLabeling       = document.getElementById( "imageLabelingTab" );
  gDialog.tabLink           = document.getElementById( "imageLinkTab" );
  gDialog.srcInput          = document.getElementById( "srcInput" );
  gDialog.relativeURL       = document.getElementById( "makeRelativeCheckbox" ).checked;
  gDialog.altTextInput      = document.getElementById( "altTextInput" );
  gDialog.actual            = document.getElementById( "actual" );
  gDialog.iconic            = document.getElementById( "iconic" );
  gDialog.custom            = document.getElementById( "custom" );
  gDialog.constrainCheckbox = document.getElementById( "constrainCheckbox" );
 // frame and placement tabs
 // labeling tab
  gDialog.ImageHolder       = document.getElementById( "preview-image-holder" );
  gDialog.PreviewWidth      = document.getElementById( "PreviewWidth" );
  gDialog.PreviewHeight     = document.getElementById( "PreviewHeight" );
  gDialog.PreviewSize       = document.getElementById( "PreviewSize" );
  gDialog.PreviewImage      = null;
  gDialog.herePlacementRadioGroup   = document.getElementById("herePlacementRadioGroup");
  gDialog.OkButton          = document.documentElement.getButton("accept");
  
//  var imageSizeFieldList = [gFrameTab.widthInput, gFrameTab.heightInput];
//  imageUnitHandler.setEditFieldList(imageSizeFieldList);
//  imageUnitHandler.initCurrentUnit("px");
//  initImageUnitList(document.getElementById(gFrameTab.unitMenuList));
//  imageUnitHandler.setCurrentUnit("px");
  // Get a single selected image element
  var tagName = "object";
  imageElement = null;
  if (window.arguments && window.arguments.length > 0)
    imageElement = window.arguments[0];
  if (!imageElement)
  {
    // First check for <input type="image">
    // Does this ever get run?
    try {
      imageElement = editor.getSelectedElement("input");

      if (!imageElement || imageElement.getAttribute("type") != "image") {
        // Get a single selected image element
        imageElement = editor.getSelectedElement(tagName);
//        if (imageElement)
//          gAnchorElement = editor.getElementOrParentByTagName("href", imageElement);
      }
    } catch (e) {}

  }

  if (imageElement)
  {
    // We found an element and don't need to insert one
    if (imageElement.hasAttribute("src"))
    {
      gInsertNewImage = false;
      gActualWidth  = imageElement.naturalWidth;
      gActualHeight = imageElement.naturalHeight;
    }
  }
  else
  {
    gInsertNewImage = true;

    // We don't have an element selected,
    //  so create one with default attributes
    try {
      imageElement = editor.createElementWithDefaults(tagName);
    } catch(e) {}

    if (!imageElement)
    {
      dump("Failed to get selected element or create a new one!\n");
      window.close();
      return;
    }
    try {
      gAnchorElement = editor.getSelectedElement("href");
    } catch (e) {}
  }

  // Make a copy to use for AdvancedEdit
  globalElement = imageElement.cloneNode(false);

  // We only need to test for this once per dialog load
  gHaveDocumentUrl = msiGetDocumentBaseUrl();

  initFrameTab(frameTabDlg, imageElement, gInsertNewImage);
  InitDialog();
//  if (gAnchorElement)
//    gOriginalHref = gAnchorElement.getAttribute("href");
//  gDialog.hrefInput.value = gOriginalHref;

//  FillLinkMenulist(gDialog.hrefInput, gHNodeArray);
  ChangeLinkLocation();

  // Save initial source URL
  gOriginalSrc = gDialog.srcInput.value;

  // By default turn constrain on, but both width and height must be in pixels
  gDialog.constrainCheckbox.checked = true;

  window.mMSIDlgManager = new msiDialogConfigManager(window);
  window.mMSIDlgManager.configureDialog();

  // Start in "Link" tab if 2nd arguement is true
  if (gDialog.linkTab && (window.arguments.length > 1) && window.arguments[1])
  {
    document.getElementById("TabBox").selectedTab = gDialog.linkTab;
    SetTextboxFocus(gDialog.hrefInput);
  }
  else
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
    element= msiGetHTMLOrCSSStyleValue(null, globalElement, gBorderdesc[index], null);
}


// Set dialog widgets with attribute data
// We get them from globalElement copy so this can be used
//   by AdvancedEdit(), which is shared by all property dialogs
function InitImage()
{
  // Set the controls to the image's attributes
  gDialog.srcInput.value = globalElement.getAttribute("src");

  // Set "Relativize" checkbox according to current URL state
  msiSetRelativeCheckbox();

  // Force loading of image from its source and show preview image
  LoadPreviewImage();

  if (globalElement.hasAttribute("title"))
    gDialog.titleInput.value = globalElement.getAttribute("title");

  var hasAltText = globalElement.hasAttribute("alt");
  var altText;
  if (hasAltText)
  {
    altText = globalElement.getAttribute("alt");
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

  // setup the height and width widgets
  var width = msiInitPixelOrPercentMenulist(globalElement,
                    gInsertNewImage ? null : imageElement,
                    "width", "widthUnitsMenulist", gPixel);
  var height = msiInitPixelOrPercentMenulist(globalElement,
                    gInsertNewImage ? null : imageElement,
                    "height", "heightUnitsMenulist", gPixel);

  // Set actual radio button if both set values are the same as actual
  SetSizeWidgets(width, height);
  frameTabDlg.unitList.value = "px";

  frameTabDlg.widthInput.value  = gConstrainWidth = width ? width : (gActualWidth ? gActualWidth : "");
  frameTabDlg.heightInput.value = gConstrainHeight = height ? height : (gActualHeight ? gActualHeight : "");

  // set spacing editfields
  var herePlacement;
  var placement = globalElement.getAttribute("placement");
  if (placement == "here");
    gDialog.herePlacementRadioGroup.value=globalElement.getAttribute("herePlacement");

  // dialog.border.value       = globalElement.getAttribute("border");
  var bordervalues;
  var bv = msiGetHTMLOrCSSStyleValue(null, globalElement, "border", null);
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
    
//  gDialog.borderInput.top = bordervalues[0];
//  gDialog.borderInput.right = bordervalues[1];
//  gDialog.borderInput.bottom = bordervalues[2];
//  gDialog.borderInput.left = bordervalues[3];

  // Get alignment setting
//  var align = globalElement.getAttribute("align");
//  if (align)
//    align = align.toLowerCase();
//
//  var imgClass;
//  var textID;
//
//  switch ( align )
//  {
//    case "top":
//    case "middle":
//    case "right":
//    case "left":
//      gDialog.alignTypeSelect.value = align;
//      break;
//    default:  // Default or "bottom"
//      gDialog.alignTypeSelect.value = "bottom";
//  }
//
//  // Get image map for image
//  gImageMap = GetImageMap();
//
//  doOverallEnabling();
//  doDimensionEnabling();
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
    
  gDialog.PreviewImage = document.createElementNS("http://www.w3.org/1999/xhtml", "html:img");
  if (gDialog.PreviewImage)
  {
    // set the src before appending to the document -- see bug 198435 for why
    // this is needed.
    gDialog.PreviewImage.addEventListener("load", PreviewImageLoaded, true);
    gDialog.PreviewImage.src = imageSrc;
    gDialog.ImageHolder.appendChild(gDialog.PreviewImage);
  }
}



function  SetSizeWidgets(width, height)
{
  if (!(width || height) || (gActualWidth && gActualHeight && width == gActualWidth && height == gActualHeight))
    gDialog.actual.radioGroup.selectedItem = gDialog.actual;

  if (!gDialog.actual.selected)
  {
    gDialog.actual.radioGroup.selectedItem = gDialog.custom;

    // Decide if user's sizes are in the same ratio as actual sizes
    if (gActualWidth && gActualHeight)
    {
      if (gActualWidth > gActualHeight)
        gDialog.constrainCheckbox.checked = (Math.round(gActualHeight * width / gActualWidth) == height);
      else
        gDialog.constrainCheckbox.checked = (Math.round(gActualWidth * height / gActualHeight) == width);
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
  if (gDialog.showLinkBorder.checked)
  {
    var border = TrimString(gDialog.border.value);
    if (!border || border == "0")
      gDialog.border.value = "2";
  }
  else
  {
    gDialog.border.value = "0";
  }
}

// Get data from widgets, validate, and set for the global element
//   accessible to AdvancedEdit() [in msiEdDialogCommon.js]
function ValidateData()
{
  return ValidateImage();
}

// Get data from widgets, validate, and set for the global element
//   accessible to AdvancedEdit() [in msiEdDialogCommon.js]
function ValidateImage()
{
  var editorElement = msiGetParentEditorElementForDialog(window);
  var editor = msiGetEditor(editorElement);
//  var editor = GetCurrentEditor();
  if (!editor)
    return false;

//  gValidateTab = gDialog.tabLocation;
  if (!gDialog.srcInput.value)
  {
    AlertWithTitle(null, GetString("MissingImageError"));
    SwitchToValidatePanel();
    gDialog.srcInput.focus();
    return false;
  }

  //TODO: WE NEED TO DO SOME URL VALIDATION HERE, E.G.:
  // We must convert to "file:///" or "http://" format else image doesn't load!
  var src = TrimString(gDialog.srcInput.value);
  globalElement.setAttribute("src", src);

  var title = TrimString(gDialog.titleInput.value);
  if (title)
    globalElement.setAttribute("title", title);
  else
    globalElement.removeAttribute("title");

  alt = TrimString(gDialog.altTextInput.value);

  globalElement.setAttribute("alt", alt);

  var width = "";
  var height = "";

  gValidateTab = gDialog.tabDimensions;
  if (!gDialog.actual.selected)
  {
    // Get user values for width and height
    width = msiValidateNumber(gDialog.widthInput, gDialog.widthUnitsMenulist, 1, gMaxPixels, 
                           globalElement, "width", false, true);
    if (gValidationError)
      return false;

    height = msiValidateNumber(gDialog.heightInput, gDialog.heightUnitsMenulist, 1, gMaxPixels, 
                            globalElement, "height", false, true);
    if (gValidationError)
      return false;
  }

  // We always set the width and height attributes, even if same as actual.
  //  This speeds up layout of pages since sizes are known before image is loaded
  if (!width)
    width = gActualWidth;
  if (!height)
    height = gActualHeight;

  // Remove existing width and height only if source changed
  //  and we couldn't obtain actual dimensions
  var srcChanged = (src != gOriginalSrc);
  if (width)
    globalElement.setAttribute("width", width);
  else if (srcChanged)
    editor.removeAttributeOrEquivalent(globalElement, "width", true);

  if (height)
    globalElement.setAttribute("height", height);
  else if (srcChanged) 
    editor.removeAttributeOrEquivalent(globalElement, "height", true);

  // spacing attributes
//  gValidateTab = gDialog.tabBorder;
//  msiValidateNumber(gDialog.imagelrInput, null, 0, gMaxPixels, 
//                 globalElement, "hspace", false, true, true);
//  if (gValidationError)
//    return false;
//
//  msiValidateNumber(gDialog.imagetbInput, null, 0, gMaxPixels, 
//                 globalElement, "vspace", false, true);
//  if (gValidationError)
//    return false;

  // note this is deprecated and should be converted to stylesheets
//  msiValidateNumber(gDialog.border, null, 0, gMaxPixels, 
//                 globalElement, "border", false, true);
//  if (gValidationError)
//    return false;

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

  return true;
}

function chooseFile()
{
//  if (gTimerID)
//    clearTimeout(gTimerID);
  // Get a local file, converted into URL format
  var fileName = GetLocalFileURL("img"); // return a URLString
  if (fileName)
  {
    var url = msiURIFromString(fileName);
    if (gDialog.import) // copy the file into the graphics directory
    {
      try {
        var file = msiFileFromFileURL(url);
        var docUrlString = msiGetDocumentBaseUrl();
        var docurl = msiURIFromString(docUrlString);
        var dir = msiFileFromFileURL(docurl);
        dir = dir.parent;
        dir.append("graphics");
        if (!dir.exists()) dir.create(1, 0755);
        file.copyTo(dir,"");    // BBM todo: check for name clashes
        file.permissions = 0755;
        fileName = "graphics/"+file.leafName;
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

function PreviewImageLoaded()
{
  if (gDialog.PreviewImage)
  {
    // Image loading has completed -- we can get actual width
    gActualWidth  = gDialog.PreviewImage.naturalWidth;
    gActualHeight = gDialog.PreviewImage.naturalHeight;

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

      SetSizeWidgets(frameTabDlg.widthInput.value, frameTabDlg.heightInput.value);
    }

    if (frameTabDlg.actual.selected)
      SetActualSize();
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
    
  gDialog.PreviewImage = document.createElementNS("http://www.w3.org/1999/xhtml", "html:img");
  if (gDialog.PreviewImage)
  {
    // set the src before appending to the document -- see bug 198435 for why
    // this is needed.
    gDialog.PreviewImage.addEventListener("load", PreviewImageLoaded, true);
    gDialog.PreviewImage.src = imageSrc;
    gDialog.ImageHolder.appendChild(gDialog.PreviewImage);
  }
}

function SetActualSize()
{
  gDialog.widthInput.value = gActualWidth ? gActualWidth : "";
  gDialog.heightInput.value = gActualHeight ? gActualHeight : "";
  frameTabDlg.unitList.selectedIndex = 0;
  doDimensionEnabling();
}

function ChangeImageSrc()
{
//  if (gTimerID)
//    clearTimeout(gTimerID);
//
//  gTimerID = setTimeout("LoadPreviewImage()", 800);

  InitImage();
  msiSetRelativeCheckbox();
  doOverallEnabling();
}

function doDimensionEnabling()
{
  // Enabled only if "Custom" is selected
  var enable = (gDialog.custom.selected);

  // BUG 74145: After input field is disabled,
  //   setting it enabled causes blinking caret to appear
  //   even though focus isn't set to it.
  SetElementEnabledById( "heightInput", enable );
  SetElementEnabledById( "heightLabel", enable );

  SetElementEnabledById( "widthInput", enable );
  SetElementEnabledById( "widthLabel", enable);

  SetElementEnabledById( "unitList", enable );

  var constrainEnable = enable ;
//         && ( gDialog.widthUnitsMenulist.selectedIndex == 0 )
//         && ( gDialog.heightUnitsMenulist.selectedIndex == 0 );

  SetElementEnabledById( "constrainCheckbox", constrainEnable );
  imageUnitHandler.setCurrentUnit(frameTabDlg.unitList.value);

}

function doOverallEnabling()
{
  var enabled = TrimString(gDialog.srcInput.value) != "";

  SetElementEnabled(gDialog.OkButton, enabled);
  SetElementEnabledById("AdvancedEditButton1", enabled);
}

function ToggleConstrain()
{
  // If just turned on, save the current width and height as basis for constrain ratio
  // Thus clicking on/off lets user say "Use these values as aspect ration"
  if (gDialog.constrainCheckbox.checked && !gDialog.constrainCheckbox.disabled) ;
//     && (gDialog.widthUnitsMenulist.selectedIndex == 0)
//     && (gDialog.heightUnitsMenulist.selectedIndex == 0))
  {
    gConstrainWidth = Number(TrimString(gDialog.widthInput.value));
    gConstrainHeight = Number(TrimString(gDialog.heightInput.value));
  }
}

function constrainProportions( srcID, destID, event )
{
  var srcElement = document.getElementById(srcID);
  if (!srcElement)
    return;
  updateTextNumber(srcElement, srcID, event);
  var destElement = document.getElementById(destID);
  if (!destElement)
    return;

  // always force an integer (whether we are constraining or not)
  forceInteger(srcID);

  if (gActualWidth && gActualHeight &&
      (gDialog.constrainCheckbox.checked && !gDialog.constrainCheckbox.disabled))
  {
//  // double-check that neither width nor height is in percent mode; bail if so!
//  if ( (gDialog.widthUnitsMenulist.selectedIndex != 0)
//     || (gDialog.heightUnitsMenulist.selectedIndex != 0) )
//    return;

  // This always uses the actual width and height ratios
  // which is kind of funky if you change one number without the constrain
  // and then turn constrain on and change a number
  // I prefer the old strategy (below) but I can see some merit to this solution
    if (srcID == "widthInput")
      destElement.value = Math.round( srcElement.value * gActualHeight / gActualWidth );
    else
      destElement.value = Math.round( srcElement.value * gActualWidth / gActualHeight );
  }
/*
  // With this strategy, the width and height ratio
  //   can be reset to whatever the user entered.
  if (srcID == "widthInput")
    destElement.value = Math.round( srcElement.value * gConstrainHeight / gConstrainWidth );
  else
    destElement.value = Math.round( srcElement.value * gConstrainWidth / gConstrainHeight );
*/
  setContentSize(imageUnitHandler.getValueAs(gDialog.widthInput.value,"px"), imageUnitHandler.getValueAs(gDialog.heightInput.value,"px"));
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

function SwitchToValidatePanel()
{
  if (gDialog.tabBox && gValidateTab && gDialog.tabBox.selectedTab != gValidateTab)
    gDialog.tabBox.selectedTab = gValidateTab;
}

// Get data from widgets, validate, and set for the global element
//   accessible to AdvancedEdit() [in msiEdDialogCommon.js]
function ValidateImage()
{
  var editorElement = msiGetParentEditorElementForDialog(window);
  var editor = msiGetEditor(editorElement);
//  var editor = GetCurrentEditor();
  if (!editor)
    return false;

  gValidateTab = gDialog.tabLocation;
  if (!gDialog.srcInput.value)
  {
    AlertWithTitle(null, GetString("MissingImageError"));
    SwitchToValidatePanel();
    gDialog.srcInput.focus();
    return false;
  }

  //TODO: WE NEED TO DO SOME URL VALIDATION HERE, E.G.:
  // We must convert to "file:///" or "http://" format else image doesn't load!
  var src = TrimString(gDialog.srcInput.value);
  globalElement.setAttribute("src", src);

  var title = ""; //TrimString(gDialog.titleInput.value);
  if (title)
    globalElement.setAttribute("title", title);
  else
    globalElement.removeAttribute("title");

  alt = ""; //TrimString(gDialog.altTextInput.value);

  globalElement.setAttribute("alt", alt);

  var width = "";
  var height = "";

  gValidateTab = gDialog.tabDimensions;
  if (!gDialog.actual.selected)
  {
    // Get user values for width and height
//    width = msiValidateNumber(gDialog.widthInput, gDialog.widthUnitsMenulist, 1, gMaxPixels, 
//                           globalElement, "width", false, true);
//    if (gValidationError)
//      return false;
//
//    height = msiValidateNumber(gDialog.heightInput, gDialog.heightUnitsMenulist, 1, gMaxPixels, 
//                            globalElement, "height", false, true);
//    if (gValidationError)
//      return false;
  }

  // We always set the width and height attributes, even if same as actual.
  //  This speeds up layout of pages since sizes are known before image is loaded
  if (!width)
    width = gActualWidth;
  if (!height)
    height = gActualHeight;

  // Remove existing width and height only if source changed
  //  and we couldn't obtain actual dimensions
  var srcChanged = (src != gOriginalSrc);
  if (width)
    globalElement.setAttribute("width", width);
  else if (srcChanged)
    editor.removeAttributeOrEquivalent(globalElement, "width", true);

  if (height)
    globalElement.setAttribute("height", height);
  else if (srcChanged) 
    editor.removeAttributeOrEquivalent(globalElement, "height", true);

  // spacing attributes
  gValidateTab = gDialog.tabBorder;
  msiValidateNumber(gDialog.imagelrInput, null, 0, gMaxPixels, 
                 globalElement, "hspace", false, true, true);
  if (gValidationError)
    return false;

  msiValidateNumber(gDialog.imagetbInput, null, 0, gMaxPixels, 
                 globalElement, "vspace", false, true);
  if (gValidationError)
    return false;

  // note this is deprecated and should be converted to stylesheets
  msiValidateNumber(gDialog.border, null, 0, gMaxPixels, 
                 globalElement, "border", false, true);
  if (gValidationError)
    return false;

  // Default or setting "bottom" means don't set the attribute
  // Note that the attributes "left" and "right" are opposite
  //  of what we use in the UI, which describes where the TEXT wraps,
  //  not the image location (which is what the HTML describes)
  switch ( gDialog.alignTypeSelect.value )
  {
    case "top":
    case "middle":
    case "right":
    case "left":
      globalElement.setAttribute( "align", gDialog.alignTypeSelect.value );
      break;
    default:
      try {
        editor.removeAttributeOrEquivalent(globalElement, "align", true);
      } catch (e) {}
  }

  return true;
}

function doHelpButton()
{
  openHelp("image_properties");
  return true;
}

function onAccept()
{
  // Use this now (default = false) so Advanced Edit button dialog doesn't trigger error message
  gDoAltTextError = true;

  if (ValidateData())
  {
    if (window.arguments[0] != null)
    {
      SaveWindowLocation();
      return true;
    }

    var editorElement = msiGetParentEditorElementForDialog(window);
    var editor = msiGetEditor(editorElement);
//    var editor = GetCurrentEditor();

//    editor.beginTransaction();
//
//    try
//    {
//      if (gRemoveImageMap)
//      {
//        globalElement.removeAttribute("usemap");
//        if (gImageMap)
//        {
//          editor.deleteNode(gImageMap);
//          gInsertNewIMap = true;
//          gImageMap = null;
//        }
//      }
//      else if (gImageMap)
//      {
//        // un-comment to see that inserting image maps does not work!
//        /*
//        gImageMap = editor.createElementWithDefaults("map");
//        gImageMap.setAttribute("name", "testing");
//        var testArea = editor.createElementWithDefaults("area");
//        testArea.setAttribute("shape", "circle");
//        testArea.setAttribute("coords", "86,102,52");
//        testArea.setAttribute("href", "test");
//        gImageMap.appendChild(testArea);
//        */
//
//        // Assign to map if there is one
//        var mapName = gImageMap.getAttribute("name");
//        if (mapName != "")
//        {
//          globalElement.setAttribute("usemap", ("#"+mapName));
//          if (globalElement.getAttribute("border") == "")
//            globalElement.setAttribute("border", 0);
//        }
//      }
//
//      // Create or remove the link as appropriate
//      var href = gDialog.hrefInput.value;
//      if (href != gOriginalHref)
//      {
//        if (href && !gInsertNewImage)
//          msiEditorSetTextProperty(editorElement, "a", "href", href);
//        else
//          msiEditorRemoveTextProperty(editorElement, "href", "");
//      }
//
//      // If inside a link, always write the 'border' attribute
//      if (href)
//      {
//        if (gDialog.showLinkBorder.checked)
//        {
//          // Use default = 2 if border attribute is empty
//          if (!globalElement.hasAttribute("border"))
//            globalElement.setAttribute("border", "2");
//        }
//        else
//          globalElement.setAttribute("border", "0");
//      }
//
//      if (gInsertNewImage)
//      {
//        if (href) {
//          var linkElement = editor.createElementWithDefaults("a");
//          linkElement.setAttribute("href", href);
//          linkElement.appendChild(imageElement);
//          editor.insertElementAtSelection(linkElement, true);
//        }
//        else
//          // 'true' means delete the selection before inserting
//          editor.insertElementAtSelection(imageElement, true);
//      }
//
//      // Check to see if the link was to a heading
//      // Do this last because it moves the caret (BAD!)
//      if (href in gHNodeArray)
//      {
//        var anchorNode = editor.createElementWithDefaults("a");
//        if (anchorNode)
//        {
//          anchorNode.name = href.substr(1);
//          // Remember to use editor method so it is undoable!
//          editor.insertNode(anchorNode, gHNodeArray[href], 0, false);
//        }
//      }
//      // All values are valid - copy to actual element in doc or
//      //   element we just inserted
//      editor.cloneAttributes(imageElement, globalElement);
//
//      // If document is empty, the map element won't insert,
//      //  so always insert the image first
//      if (gImageMap && gInsertNewIMap)
//      {
//        // Insert the ImageMap element at beginning of document
//        var body = editor.rootElement;
//        editor.setShouldTxnSetSelection(false);
//        editor.insertNode(gImageMap, body, 0);
//        editor.setShouldTxnSetSelection(true);
//      }
//    }
//    catch (e)
//    {
//      dump(e);
//    }
//
//    editor.endTransaction();

    SaveWindowLocation();
    return true;
  }

  gDoAltTextError = false;

  return true;
//  return false;
}


var xsltSheet="<?xml version='1.0'?><xsl:stylesheet version='1.1' xmlns:xsl='http://www.w3.org/1999/XSL/Transform' xmlns:html='http://www.w3.org/1999/xhtml' ><xsl:output method='text' encoding='UTF-8'/> <xsl:template match='/'>  <xsl:apply-templates select='//*[@key]'/></xsl:template><xsl:template match='//*[@key]'>   <xsl:value-of select='@key'/><xsl:text> </xsl:text></xsl:template> </xsl:stylesheet>";

function initKeyList()
{
  var editorElement = msiGetActiveEditorElement();
  var editor;
  if (editorElement) editor = msiGetEditor(editorElement);
  var parser = new DOMParser();
  var dom = parser.parseFromString(xsltSheet, "text/xml");
  dump(dom.documentElement.nodeName == "parsererror" ? "error while parsing" + dom.documentElement.textContents : dom.documentElement.nodeName);
  var processor = new XSLTProcessor();
  processor.importStylesheet(dom.documentElement);
  var newDoc;
  if (editor) newDoc = processor.transformToDocument(editor.document, document);
  dump(newDoc.documentElement.localName+"\n");
  var keyString = newDoc.documentElement.textContent;
  var keys = keyString.split(/\s+/);
  var i;
  var len;
  keys.sort();
  var lastkey = "";
  for (i=keys.length-1; i >= 0; i--)
  {
    if (keys[i] == "" || keys[i] == lastkey) keys.splice(i,1);
    else lastkey = keys[i];
  }  
  var ACSA = Components.classes["@mozilla.org/autocomplete/search;1?name=stringarray"].getService();
  ACSA.QueryInterface(Components.interfaces.nsIAutoCompleteSearchStringArray);
  ACSA.resetArray("keys");
  for (i=0, len=keys.length; i<len; i++)
  {
    if (keys[i].length > 0) 
      ACSA.addString("keys",keys[i]);
  }
  dump("Keys are : "+keys.join()+"\n");    
}