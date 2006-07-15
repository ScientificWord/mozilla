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
// This is mostly a modified version of code in EdColorProps.xul

// Initialize in case we can't get them from prefs???
var defaultTextColor = "#000000";
var defaultLinkColor = "#000099";
var defaultActiveColor = "#000099";
var defaultVisitedColor = "#990099";
var defaultBackgroundColor = "#FFFFFF";

var customTextColor;
var customLinkColor;
var customActiveColor;
var customVisitedColor;
var customBackgroundColor;
var previewBGColor;
var backgroundImage = "";

// Strings we use often
const styleStr =       "style";
const textStr =        "text";
const linkStr =        "link";
const vlinkStr =       "vlink";
const alinkStr =       "alink";
const bgcolorStr =     "bgcolor";
const backgroundStr =  "background";
const colorStyle =     "color: ";
const backColorStyle = "background-color: ";
const backImageStyle = "; background-image: url(";

var browserColors;
var dialog;

function Startup()
{
  gDialog.ColorPreview = document.getElementById("ColorPreview");
  gDialog.NormalText = document.getElementById("NormalText");
  gDialog.LinkText = document.getElementById("LinkText");
  gDialog.ActiveLinkText = document.getElementById("ActiveLinkText");
  gDialog.VisitedLinkText = document.getElementById("VisitedLinkText");
  gDialog.DefaultColorsRadio = document.getElementById("DefaultColorsRadio");
  gDialog.CustomColorsRadio = document.getElementById("CustomColorsRadio");
  gDialog.BackgroundImageInput = document.getElementById("BackgroundImageInput");

  // The data elements that hold the pref values
  gDialog.NormalData = document.getElementById("textData");
  gDialog.LinkData = document.getElementById("linkData");
  gDialog.ActiveLinkData = document.getElementById("aLinkData");
  gDialog.VisitedLinkData = document.getElementById("fLinkData");
  gDialog.BackgroundColorData = document.getElementById("backgroundColorData");
  gDialog.BackgroundImageData = document.getElementById("backgroundImageData");

  browserColors = GetDefaultBrowserColors();

  // Use author's browser pref colors passed into dialog
  defaultTextColor = browserColors.TextColor;
  defaultLinkColor = browserColors.LinkColor;
  defaultActiveColor = browserColors.ActiveLinkColor;
  defaultVisitedColor =  browserColors.VisitedLinkColor;
  defaultBackgroundColor=  browserColors.BackgroundColor;

  // Get the colors and image set by prefs init code 
  customTextColor = gDialog.NormalData.getAttribute("value"); 
  customLinkColor = gDialog.LinkData.getAttribute("value");
  customActiveColor = gDialog.ActiveLinkData.getAttribute("value");
  customVisitedColor = gDialog.VisitedLinkData.getAttribute("value");
  customBackgroundColor = gDialog.BackgroundColorData.getAttribute("value");
  backgroundImage = gDialog.BackgroundImageData.getAttribute("value");
  if (backgroundImage)
    gDialog.BackgroundImageInput.value = backgroundImage;

  // "value" attribute value is a string conversion of boolean!
  if( document.getElementById( "useCustomColors" ).value == "true" )
    UseCustomColors();
  else
    UseDefaultColors();

  return true;
}

function GetColorAndUpdate(ColorWellID)
{
  // Only allow selecting when in custom mode
  if (!gDialog.CustomColorsRadio.selected) return;

  var colorWell = document.getElementById(ColorWellID);
  if (!colorWell) return;

  // Don't allow a blank color, i.e., using the "default"
  var colorObj = { NoDefault:true, Type:"", TextColor:0, PageColor:0, Cancel:false };

  switch( ColorWellID )
  {
    case "textCW":
      colorObj.Type = "Text";
      colorObj.TextColor = customTextColor;
      break;
    case "linkCW":
      colorObj.Type = "Link";
      colorObj.TextColor = customLinkColor;
      break;
    case "activeCW":
      colorObj.Type = "ActiveLink";
      colorObj.TextColor = customActiveColor;
      break;
    case "visitedCW":
      colorObj.Type = "VisitedLink";
      colorObj.TextColor = customVisitedColor;
      break;
    case "backgroundCW":
      colorObj.Type = "Page";
      colorObj.PageColor = customBackgroundColor;
      break;
  }

  window.openDialog("chrome://editor/content/EdColorPicker.xul", "_blank", "chrome,close,titlebar,modal", "", colorObj);

  // User canceled the dialog
  if (colorObj.Cancel)
    return;

  var color = "";
  switch( ColorWellID )
  {
    case "textCW":
      color = customTextColor = colorObj.TextColor;
      gDialog.NormalData.setAttribute("value", color); 
      break;
    case "linkCW":
      color = customLinkColor = colorObj.TextColor;
      gDialog.LinkData.setAttribute("value", color);
      break;
    case "activeCW":
      color = customActiveColor = colorObj.TextColor;
      gDialog.ActiveLinkData.setAttribute("value", color);
      break;
    case "visitedCW":
      color = customVisitedColor = colorObj.TextColor;
      gDialog.VisitedLinkData.setAttribute("value", color);
      break;
    case "backgroundCW":
      color = customBackgroundColor = colorObj.BackgroundColor;
      gDialog.BackgroundColorData.setAttribute("value", color);
      break;
  }
  setColorWell(ColorWellID, color); 
  SetColorPreview(ColorWellID, color);
}

function SetColorPreview(ColorWellID, color)
{
  switch( ColorWellID )
  {
    case "textCW":
      gDialog.NormalText.setAttribute(styleStr,colorStyle+color);
      break;
    case "linkCW":
      gDialog.LinkText.setAttribute(styleStr,colorStyle+color);
      break;
    case "activeCW":
      gDialog.ActiveLinkText.setAttribute(styleStr,colorStyle+color);
      break;
    case "visitedCW":
      gDialog.VisitedLinkText.setAttribute(styleStr,colorStyle+color);
      break;
    case "backgroundCW":
      // Must combine background color and image style values
      var styleValue = backColorStyle+color;
      if (backgroundImage)
        styleValue += ";"+backImageStyle+backgroundImage+");";

      gDialog.ColorPreview.setAttribute(styleStr,styleValue);
      previewBGColor = color;
      break;
  }
}

function UseCustomColors()
{
  SetElementEnabledById("TextButton", true);
  SetElementEnabledById("LinkButton", true);
  SetElementEnabledById("ActiveLinkButton", true);
  SetElementEnabledById("VisitedLinkButton", true);
  SetElementEnabledById("BackgroundButton", true);
  SetElementEnabledById("Text", true);
  SetElementEnabledById("Link", true);
  SetElementEnabledById("Active", true);
  SetElementEnabledById("Visited", true);
  SetElementEnabledById("Background", true);

  SetColorPreview("textCW",       customTextColor);
  SetColorPreview("linkCW",       customLinkColor);
  SetColorPreview("activeCW",     customActiveColor);
  SetColorPreview("visitedCW",    customVisitedColor);
  SetColorPreview("backgroundCW", customBackgroundColor);

  setColorWell("textCW",          customTextColor);
  setColorWell("linkCW",          customLinkColor);
  setColorWell("activeCW",        customActiveColor);
  setColorWell("visitedCW",       customVisitedColor);
  setColorWell("backgroundCW",    customBackgroundColor);

  gDialog.NormalData.setAttribute("value",          customTextColor); 
  gDialog.LinkData.setAttribute("value",            customLinkColor);
  gDialog.ActiveLinkData.setAttribute("value",      customActiveColor);
  gDialog.VisitedLinkData.setAttribute("value",     customVisitedColor);
  gDialog.BackgroundColorData.setAttribute("value", customBackgroundColor);
}

function UseDefaultColors()
{
  SetColorPreview("textCW",       defaultTextColor);
  SetColorPreview("linkCW",       defaultLinkColor);
  SetColorPreview("activeCW",     defaultActiveColor);
  SetColorPreview("visitedCW",    defaultVisitedColor);
  SetColorPreview("backgroundCW", defaultBackgroundColor);

  // Setting to blank color will remove color from buttons,
  setColorWell("textCW",       "");
  setColorWell("linkCW",       "");
  setColorWell("activeCW",     "");
  setColorWell("visitedCW",    "");
  setColorWell("backgroundCW", "");

  // Disable color buttons and labels
  SetElementEnabledById("TextButton", false);
  SetElementEnabledById("LinkButton", false);
  SetElementEnabledById("ActiveLinkButton", false);
  SetElementEnabledById("VisitedLinkButton", false);
  SetElementEnabledById("BackgroundButton", false);
  SetElementEnabledById("Text", false);
  SetElementEnabledById("Link", false);
  SetElementEnabledById("Active", false);
  SetElementEnabledById("Visited", false);
  SetElementEnabledById("Background", false);
  
  // Note that we leave custom colors set even if 
  //  custom colors pref is false (we just ignore the colors)
}
  
function ChooseImageFile()
{
  // Get a local image file, converted into URL format
  var fileName = GetLocalFileURL("img");
  if (fileName)
  {
    gDialog.BackgroundImageInput.value = fileName;
    ValidateAndPreviewImage(true);
  }
  SetTextboxFocus(gDialog.BackgroundImageInput);
}

function ChangeBackgroundImage()
{
  // Don't show error message for image while user is typing
  ValidateAndPreviewImage(false);
}

function ValidateAndPreviewImage(ShowErrorMessage)
{
  // First make a string with just background color
  var styleValue = backColorStyle+previewBGColor+";";

  var image = TrimString(gDialog.BackgroundImageInput.value);
  if (image)
  {
    backgroundImage = image;
    // Append image style
    styleValue += backImageStyle+backgroundImage+");";
  }
  else
    backgroundImage = "";

  // Set style on preview (removes image if not valid)
  gDialog.ColorPreview.setAttribute(styleStr, styleValue);
  
  // Set the pref data so pref code saves it 
  gDialog.BackgroundImageData.setAttribute("value", backgroundImage ? backgroundImage : "");
}

