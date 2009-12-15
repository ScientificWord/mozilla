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
 * The Original Code is Editor Image Properties Overlay.
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
 *   Neil Rashbrook <neil@parkwaycc.co.uk> (Separated from EdImageProps.js)
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

/*
 Note: We encourage non-empty alt text for images inserted into a page. 
 When there's no alt text, we always write 'alt=""' as the attribute, since "alt" is a required attribute.
 We allow users to not have alt text by checking a "Don't use alterate text" radio button,
 and we don't accept spaces as valid alt text. A space used to be required to avoid the error message
 if user didn't enter alt text, but is uneccessary now that we no longer annoy the user 
 with the error dialog if alt="" is present on an img element.
 We trim all spaces at the beginning and end of user's alt text
*/

var gInsertNewImage = true;
var gInsertNewIMap = true;
var gDoAltTextError = false;
var gConstrainOn = false;
// Note used in current version, but these are set correctly
//  and could be used to reset width and height used for constrain ratio
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

function ImageStartup()
{
  gDialog = new Object();
  gDialog.import            = document.getElementById( "importRefRadioGroup").selectedIndex == 0;
  gDialog.tabBox            = document.getElementById( "TabBox" );
  gDialog.tabLocation       = document.getElementById( "imageLocationTab" );
  gDialog.tabDimensions     = document.getElementById( "imageDimensionsTab" );
  gDialog.tabBorder         = document.getElementById( "imageBorderTab" );
  gDialog.srcInput          = document.getElementById( "srcInput" );
  gDialog.titleInput        = document.getElementById( "titleInput" );
  gDialog.altTextInput      = document.getElementById( "altTextInput" );
  gDialog.altTextRadioGroup = document.getElementById( "altTextRadioGroup" );
  gDialog.altTextRadio      = document.getElementById( "altTextRadio" );
  gDialog.noAltTextRadio    = document.getElementById( "noAltTextRadio" );
  gDialog.customSizeRadio   = document.getElementById( "customSizeRadio" );
  gDialog.actualSizeRadio   = document.getElementById( "actualSizeRadio" );
  gDialog.constrainCheckbox = document.getElementById( "constrainCheckbox" );
  gDialog.widthInput        = document.getElementById( "widthInput" );
  gDialog.heightInput       = document.getElementById( "heightInput" );
  gDialog.unitMenulist      = document.getElementById( "unitMenulist" );
  gDialog.imagelrInput      = document.getElementById( "imageleftrightInput" );
  gDialog.imagetbInput      = document.getElementById( "imagetopbottomInput" );
  gDialog.border            = document.getElementById( "border" );
  gDialog.alignTypeSelect   = document.getElementById( "alignTypeSelect" );
  gDialog.ImageHolder       = document.getElementById( "preview-image-holder" );
  gDialog.PreviewWidth      = document.getElementById( "PreviewWidth" );
  gDialog.PreviewHeight     = document.getElementById( "PreviewHeight" );
  gDialog.PreviewSize       = document.getElementById( "PreviewSize" );
  gDialog.PreviewImage      = null;
  gDialog.OkButton          = document.documentElement.getButton("accept");
  
  msiCSSUnitConversions.pica = 4.2333333; //mm per pica
  msiCSSUnitConversions.pixel = 0.26458342; //mm per pixel at 96 pixels/inch.
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
  gDialog.unitMenulist.value = "px";

  gDialog.widthInput.value  = gConstrainWidth = width ? width : (gActualWidth ? gActualWidth : "");
  gDialog.heightInput.value = gConstrainHeight = height ? height : (gActualHeight ? gActualHeight : "");

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

function  SetSizeWidgets(width, height)
{
  if (!(width || height) || (gActualWidth && gActualHeight && width == gActualWidth && height == gActualHeight))
    gDialog.actualSizeRadio.radioGroup.selectedItem = gDialog.actualSizeRadio;

  if (!gDialog.actualSizeRadio.selected)
  {
    gDialog.actualSizeRadio.radioGroup.selectedItem = gDialog.customSizeRadio;

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

// Disable alt text input when "Don't use alt" radio is checked
function SetAltTextDisabled(disable)
{
  gDialog.altTextInput.disabled = disable;
}

//function GetImageMap()
//{
//  var usemap = globalElement.getAttribute("usemap");
//  if (usemap)
//  {
//    gCanRemoveImageMap = true;
//    var mapname = usemap.substring(1, usemap.length);
//    var mapCollection;
//    try {
//      var editorElement = msiGetParentEditorElementForDialog(window);
//      var editor = msiGetEditor(editorElement);
//      mapCollection = editor.document.getElementsByName(mapname);
//    } catch (e) {}
//    if (mapCollection && mapCollection[0] != null)
//    {
//      gInsertNewIMap = false;
//      return mapCollection[0];
//    }
//  }
//  else
//  {
//    gCanRemoveImageMap = false;
//  }
//
//  gInsertNewIMap = true;
//  return null;
//}

function chooseFile()
{
  if (gTimerID)
    clearTimeout(gTimerID);
  // Get a local file, converted into URL format
  var fileName = GetLocalFileURL("img");
  if (fileName)
  {
    if (gDialog.import) // copy the file into the graphics directory
    {
      try {
        var file = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
        var path = GetFilepath(fileName);
#ifdef XP_WIN
        path=path.replace("/","\\","g");
#endif
        file.initWithPath(path);
        var docUrl = msiGetDocumentBaseUrl();
        var dir = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
        var dirpath = GetFilepath(docUrl);
#ifdef XP_WIN
        dirpath=dirpath.replace("/","\\","g");
#endif
        dir.initWithPath(dirpath);
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

      SetSizeWidgets(gDialog.widthInput.value, gDialog.heightInput.value);
    }

    if (gDialog.actualSizeRadio.selected)
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
  gDialog.unitMenulist.selectedIndex = 0;
  doDimensionEnabling();
}

function ChangeImageSrc()
{
  if (gTimerID)
    clearTimeout(gTimerID);

  gTimerID = setTimeout("LoadPreviewImage()", 800);

  InitImage();
  msiSetRelativeCheckbox();
  doOverallEnabling();
}

function doDimensionEnabling()
{
  // Enabled only if "Custom" is selected
  var enable = (gDialog.customSizeRadio.selected);

  // BUG 74145: After input field is disabled,
  //   setting it enabled causes blinking caret to appear
  //   even though focus isn't set to it.
  SetElementEnabledById( "heightInput", enable );
  SetElementEnabledById( "heightLabel", enable );

  SetElementEnabledById( "widthInput", enable );
  SetElementEnabledById( "widthLabel", enable);

  SetElementEnabledById( "unitMenulist", enable );

  var constrainEnable = enable ;
//         && ( gDialog.widthUnitsMenulist.selectedIndex == 0 )
//         && ( gDialog.heightUnitsMenulist.selectedIndex == 0 );

  SetElementEnabledById( "constrainCheckbox", constrainEnable );
  imageUnitHandler.setCurrentUnit(gDialog.unitMenulist.value);

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
  if (!gDialog.actualSizeRadio.selected)
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
