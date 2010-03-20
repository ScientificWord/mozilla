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

