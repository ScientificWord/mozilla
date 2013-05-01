Components.utils.import("resource://app/modules/fontlist.jsm"); 
//Components.utils.import("resource://app/modules/pathutils.jsm"); 
//Components.utils.import("resource://app/modules/unitHandler.jsm");

var fontData = {};
var fontDefaults = {};
var isAxesFont;
var isAxesTicksFont = false;
var usingDefault = true;
var fontFallback = {face : "sans-serif", size : "12", color : "#000000", bold : "false", italic : "false"};
var computebundle;
var webSafeFontList = ["arial", "arial black", "bookman old style", "comic sans ms", "courier", "courier new",
                       "garamond", "georgia", "impact", "lucida console", "lucida sans unicode",
                       "ms sans serif", "ms serif", "palatino linotype", "symbol", "tahoma",
                       "times new roman", "trebuchet ms", "verdana", "webdings", "wingdings"];

function startup()
{
  initializeFontFamilyList(false, window);

  if (window.arguments[0])
    fontData = window.arguments[0];
  if (window.arguments[1])
    fontDefaults = window.arguments[1];
  computebundle = document.getElementById("computeBundle");
  fillInMissingDefaults();
  try
  {
    setUpFontList();
  } catch(ex) {msidump("In plotFontSettings.js, startup(), exception setting up font list: " + ex + "\n");}
//  if (fontData.whichFont == "axesTicks")
//    isAxesTicksFont = true;
//  else if (fontData.whichFont == "axes")
//    isAxesFont = true;

  adjustLabels();
  putValuesToDialog();
  hideOrDisableControls();
  updateSample();
}

function adjustLabels()
{
  var labelText, prefixStr;
  var defCheckbox;
  if ((fontData.whichFont == "axesTicks") || (fontData.whichFont == "axes"))
  {
    prefixStr = "plotFontDlg." + fontData.whichFont + "Font.";
    labelText = lookUpString(prefixStr + "windowTitle");
    if (labelText)
      document.title = labelText;
    for (var attr in fontFallback)
    {
      if ( (attr != "bold") && (attr != "italic") )
      {
        labelText = lookUpString(prefixStr + attr + "Default");
        defCheckbox = mapAttributeToControl(attr, "defaultCheckbox");
        if (labelText && defCheckbox)
          defCheckbox.label = labelText;
      }
    }
    labelText = lookUpString(prefixStr + "styleDefault");
    defCheckbox = document.getElementById("styleUseDefault");
    if (labelText && defCheckbox)
      defCheckbox.label = labelText;
  }
}

function lookUpString(key)
{
  var retval;
  try
  {
    retval = computebundle.getString(key);
  } catch(ex) {} //don't even dump these - we expect some not to be there
  return retval;
}

function fillInMissingDefaults()
{
  for (var attr in fontFallback)
  {
    if (!(attr in fontDefaults) || (fontDefaults[attr] == null))
      fontDefaults[attr] = fontFallback[attr];
  }
}

function getColorAndUpdate()
{
  var colorWell = document.getElementById("color");
  var theColor = getValueFromControl(colorWell);

  var colorObj = { NoDefault: false, Type: "Rule", TextColor: theColor, PageColor: 0, Cancel: false };
  window.openDialog("chrome://editor/content/EdColorPicker.xul", "colorpicker", "chrome,close,titlebar,modal,resizable", "", colorObj);
  if (colorObj.Cancel)
    return;
  theColor = colorObj.TextColor;
  setColorWell("color", theColor); 
  colorWell.setAttribute("color",theColor);
  updateSample();
}

function mapAttributeToControl(attr, auxCtrl)
{
  var ctrlID;
  switch(attr)
  {
    default:      ctrlID = attr;     break;
  }
  if (auxCtrl == "defaultCheckbox")
    ctrlID += "UseDefault";
  else if (auxCtrl == "defaultBroadcaster")
    ctrlID += "UsingDefault";
  if (ctrlID)
    return document.getElementById(ctrlID);
}

function getValuesFromDialog(valArray)
{
  var ctrl, value;
  for (var aDatum in fontFallback)
  {
    if (valArray && valArray.indexOf(aDatum) < 0)
      continue;
    ctrl = mapAttributeToControl(aDatum);
    if (ctrl)
    {
      if (ctrl.disabled)
        fontData[aDatum] = "";
      else
      {
        value = getValueFromControl(ctrl);
        if ( (value != null) && (value != "undefined") && (value != "") )
          fontData[aDatum] = value;
        else
          fontData[aDatum] = "";
      }
    }
  }
}

function putValuesToDialog()
{
  var ctrl, defCheckbox, value;
  for (var attr in fontFallback)
  {
    ctrl = mapAttributeToControl(attr);
    defCheckbox = mapAttributeToControl(attr, "defaultCheckbox");
    if (ctrl)
    {
      if ( (attr in fontData) && (fontData[attr] != null) && fontData[attr].length )
      {
        putValueToControl(ctrl, fontData[attr]);
        if (defCheckbox)
          defCheckbox.checked = false;
      }
      else
      {
        putValueToControl(ctrl, fontDefaults[attr]);
        if (defCheckbox)
          defCheckbox.checked = true;
      }
    }
  }

  defCheckbox = document.getElementById("styleUseDefault");
  if ( ("bold" in fontData) && (fontData.bold != null) )
    defCheckbox.checked = false;
  else if ( ("italic" in fontData) && (fontData.italic != null) )
    defCheckbox.checked = false;
  else
    defCheckbox.checked = true;
}

function hideOrDisableControls()
{
  var defCheckbox, defBroadcaster;
  for (var attr in fontFallback)
  {
    defBroadcaster = mapAttributeToControl(attr, "defaultBroadcaster");
    defCheckbox = mapAttributeToControl(attr, "defaultCheckbox");
    if (defCheckbox)
    {
      if (defCheckbox.checked)
        defBroadcaster.setAttribute("disabled", "true");
      else
        defBroadcaster.removeAttribute("disabled");
    }
  }
  defCheckbox = document.getElementById("styleUseDefault");
  defBroadcaster = document.getElementById("styleUsingDefault");
  if (defCheckbox.checked)
    defBroadcaster.setAttribute("disabled", "true");
  else
    defBroadcaster.removeAttribute("disabled");
}

function updateSample()
{
  getValuesFromDialog();
  var cssDatum, value;
  var styleStr = "";
  for (var aDatum in fontFallback)
  {
    if (fontData[aDatum] && fontData[aDatum].length)
      value = fontData[aDatum];
    else
      value = fontDefaults[aDatum];
    cssDatum = translateForCSS(aDatum, value);
    if (cssDatum)
      styleStr += cssDatum.cssProp + ": " + cssDatum.cssValue + ";";
  }
  if (styleStr.length)
    document.getElementById("fontSample").setAttribute("style", styleStr);
}

function changeUseDefaults(ctrl)
{
  var prop;
  if (ctrl.checked)
  {
    prop = ctrl.id.substr(0, ctrl.id.indexOf("UseDefault"));
    if (prop in fontData)
      fontData[prop] = "";
    updateSample();
  } //unchecking a default doesn't change any values, so don't need redraw there
  hideOrDisableControls();
}

function onAccept()
{
  getValuesFromDialog();
  return true;
}

function onCancel()
{
  fontData.Canceled = true;
  return true;
}

function setUpFontList()
{
  var jj;
  var faceList = document.getElementById("face");
  var useWebFontList = null;
  if (document.getElementById("restrictToWebFonts").checked)
    useWebFontList = webSafeFontList;
//  var menuObject = { menulist: faceList};
//  addOTFontsToMenu(menuObject);

  function getFontName(fontEntry)
  {
    fontEntry = fontEntry.replace(/(^\s+)|(\s+$)/g, "");
    colonPos = fontEntry.lastIndexOf(":");
    return fontEntry.substr(colonPos + 1);  //if lastIndexOf() returned -1, this would be the whole string, as desired
  }
  function filterFontNames(fontEntry, ix, arry)
  {
    if (!fontEntry.length)
      return false;
    var lcFontEntry = fontEntry.toLowerCase();
    if (useWebFontList && (useWebFontList.indexOf(lcFontEntry) < 0))
      return false;
    if (lcFontEntry == "sans-serif")
      return false;
    if (lcFontEntry == "sans serif")
      return false;
    if (lcFontEntry == "serif")
      return false;
    if (lcFontEntry == "monospace")
      return false;
    if (ix == 0)
      return true;
    if (fontEntry == arry[ix-1])
      return false;
    return true;
  }
  var fontArray = [];
  getOTFontlist();
  if (gSystemFonts.count)
  {
    fontArray = gSystemFonts.list.map(getFontName);
    fontArray.sort();
    fontArray = fontArray.filter(filterFontNames);
  }

  faceList.removeAllItems();
  faceList.appendItem("Sans serif", "sans-serif");
  faceList.appendItem("Serif", "serif");
  faceList.appendItem("Monospace", "monospace");

  for (jj = 0; jj < fontArray.length; ++jj)
    faceList.appendItem(fontArray[jj], fontArray[jj]);
  
//  //following copied from editor/composer/content/editor.js, but not used in favor of fontlist.jsm (otfonts)
//  if (!gLocalFonts)
//  {
//    // Build list of all local fonts once per editor??
//    try 
//    {
//      var enumerator = Components.classes["@mozilla.org/gfx/fontenumerator;1"]
//                                 .getService(Components.interfaces.nsIFontEnumerator);
//      var localFontCount = { value: 0 }
//      gLocalFonts = enumerator.EnumerateAllFonts(localFontCount);
//    }
//    catch(e) { }
//  }
}

function findItemInMenulistNoCase(menulist, itemValue)
{
  var nIndex = -1;
  var val;
  for (var ii = 0; ii < menulist.itemCount; ++ii)
  {
    val = menulist.getitemAtIndex(ii).value;
    if (val.toLowerCase() == itemValue.toLowerCase())
    {
      nIndex = ii;
      break;
    }
  }
  return nIndex;
}

function translateForCSS(attr, value)
{
  var retVal = null;
  switch(attr)
  {
    case "face":
      retVal = {cssProp : "font-family", cssValue : value};
    break;
    case "size":
      retVal = {cssProp : "font-size", cssValue : value + "pt"};
    break;
    case "color":
      retVal = {cssProp : "color", cssValue : makeColorVal(value)};
    break;
    case "bold":
      if (value == "true")
        value = "bold";
      else
        value = "normal";
      retVal = {cssProp : "font-weight", cssValue : value};
    break;
    case "italic":
      if (value == "true")
        value = "italic";
      else
        value = "normal";
      retVal = {cssProp : "font-style", cssValue : value};
    break;
    default:                                       break;
  }
  return retVal;
}