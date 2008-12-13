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

// dialog initialization code

function Startup()
{
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
  gDialog.tabFrame          = document.getElementById( "msiFrameTab" );
  gDialog.tabPlacement      = document.getElementById( "msiPlacementTab" );
  gDialog.tabLabeling       = document.getElementById( "imageLabelingTab" );
  gDialog.tabLink           = document.getElementById( "imageLinkTab" );
  gDialog.srcInput          = document.getElementById( "srcInput" );
  gDialog.relativeURL       = document.getElementById( "makeRelativeCheckbox" ).checked;
 // gDialog.titleInput        = document.getElementById( "titleInput" );
 // gDialog.altTextInput      = document.getElementById( "altTextInput" );
 // gDialog.altTextRadioGroup = document.getElementById( "altTextRadioGroup" );
 // gDialog.altTextRadio      = document.getElementById( "altTextRadio" );
 // gDialog.noAltTextRadio    = document.getElementById( "noAltTextRadio" );
  gDialog.actualSizeRadio   = document.getElementById( "actualSizeRadio" );
  gDialog.iconicImageRadio  = document.getElementById( "iconicImageRadio" );
  gDialog.customSizeRadio   = document.getElementById( "customSizeRadio" );
  gDialog.constrainCheckbox = document.getElementById( "constrainCheckbox" );
  gDialog.unitMenulist      = document.getElementById( "unitMenulist" );
  gDialog.widthInput        = document.getElementById( "widthInput" );
  gDialog.heightInput       = document.getElementById( "heightInput" );
  gDialog.unitMenulist      = document.getElementById( "unitMenulist" );
 // frame and placement tabs
  initFrameTab(gDialog);
 // labeling tab
  gDialog.ImageHolder       = document.getElementById( "preview-image-holder" );
  gDialog.PreviewWidth      = document.getElementById( "PreviewWidth" );
  gDialog.PreviewHeight     = document.getElementById( "PreviewHeight" );
  gDialog.PreviewSize       = document.getElementById( "PreviewSize" );
  gDialog.PreviewImage      = null;
  gDialog.OkButton          = document.documentElement.getButton("accept");
  
  msiCSSUnitConversions.pica = 4.2333333; //mm per pica
  msiCSSUnitConversions.pixel = 0.26458342; //mm per pixel at 96 pixels/inch.
  // add more for the other tabs

  // Get a single selected image element
  var tagName = "img";
  if ("arguments" in window && window.arguments[0])
  {
    imageElement = window.arguments[0];
  }
  else
  {
    // First check for <input type="image">
    // Does this ever get run?
    try {
      imageElement = editor.getSelectedElement("input");

      if (!imageElement || imageElement.getAttribute("type") != "image") {
        // Get a single selected image element
        imageElement = editor.getSelectedElement(tagName);
        if (imageElement)
          gAnchorElement = editor.getElementOrParentByTagName("href", imageElement);
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

  InitDialog();
  if (gAnchorElement)
    gOriginalHref = gAnchorElement.getAttribute("href");
  gDialog.hrefInput.value = gOriginalHref;

//  FillLinkMenulist(gDialog.hrefInput, gHNodeArray);
  ChangeLinkLocation();

  // Save initial source URL
  gOriginalSrc = gDialog.srcInput.value;

  // By default turn constrain on, but both width and height must be in pixels
  gDialog.constrainCheckbox.checked = true;

  window.mMSIDlgManager = new msiDialogConfigManager(window);
  window.mMSIDlgManager.configureDialog();

  // Start in "Link" tab if 2nd arguement is true
  if (gDialog.linkTab && "arguments" in window && window.arguments[1])
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
  var border = TrimString(gDialog.border.value);
  gDialog.showLinkBorder.checked = border != "" && border > 0;
}

function ChangeLinkLocation()
{
  SetRelativeCheckbox(gDialog.makeRelativeLink);
  gDialog.showLinkBorder.disabled = !TrimString(gDialog.hrefInput.value);
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
//   accessible to AdvancedEdit() [in EdDialogCommon.js]
function ValidateData()
{
  return ValidateImage();
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
    if ("arguments" in window && window.arguments[0])
    {
      SaveWindowLocation();
      return true;
    }

    var editorElement = msiGetParentEditorElementForDialog(window);
    var editor = msiGetEditor(editorElement);
//    var editor = GetCurrentEditor();

    editor.beginTransaction();

    try
    {
      if (gRemoveImageMap)
      {
        globalElement.removeAttribute("usemap");
        if (gImageMap)
        {
          editor.deleteNode(gImageMap);
          gInsertNewIMap = true;
          gImageMap = null;
        }
      }
      else if (gImageMap)
      {
        // un-comment to see that inserting image maps does not work!
        /*
        gImageMap = editor.createElementWithDefaults("map");
        gImageMap.setAttribute("name", "testing");
        var testArea = editor.createElementWithDefaults("area");
        testArea.setAttribute("shape", "circle");
        testArea.setAttribute("coords", "86,102,52");
        testArea.setAttribute("href", "test");
        gImageMap.appendChild(testArea);
        */

        // Assign to map if there is one
        var mapName = gImageMap.getAttribute("name");
        if (mapName != "")
        {
          globalElement.setAttribute("usemap", ("#"+mapName));
          if (globalElement.getAttribute("border") == "")
            globalElement.setAttribute("border", 0);
        }
      }

      // Create or remove the link as appropriate
      var href = gDialog.hrefInput.value;
      if (href != gOriginalHref)
      {
        if (href && !gInsertNewImage)
          msiEditorSetTextProperty(editorElement, "a", "href", href);
        else
          msiEditorRemoveTextProperty(editorElement, "href", "");
      }

      // If inside a link, always write the 'border' attribute
      if (href)
      {
        if (gDialog.showLinkBorder.checked)
        {
          // Use default = 2 if border attribute is empty
          if (!globalElement.hasAttribute("border"))
            globalElement.setAttribute("border", "2");
        }
        else
          globalElement.setAttribute("border", "0");
      }

      if (gInsertNewImage)
      {
        if (href) {
          var linkElement = editor.createElementWithDefaults("a");
          linkElement.setAttribute("href", href);
          linkElement.appendChild(imageElement);
          editor.insertElementAtSelection(linkElement, true);
        }
        else
          // 'true' means delete the selection before inserting
          editor.insertElementAtSelection(imageElement, true);
      }

      // Check to see if the link was to a heading
      // Do this last because it moves the caret (BAD!)
      if (href in gHNodeArray)
      {
        var anchorNode = editor.createElementWithDefaults("a");
        if (anchorNode)
        {
          anchorNode.name = href.substr(1);
          // Remember to use editor method so it is undoable!
          editor.insertNode(anchorNode, gHNodeArray[href], 0, false);
        }
      }
      // All values are valid - copy to actual element in doc or
      //   element we just inserted
      editor.cloneAttributes(imageElement, globalElement);

      // If document is empty, the map element won't insert,
      //  so always insert the image first
      if (gImageMap && gInsertNewIMap)
      {
        // Insert the ImageMap element at beginning of document
        var body = editor.rootElement;
        editor.setShouldTxnSetSelection(false);
        editor.insertNode(gImageMap, body, 0);
        editor.setShouldTxnSetSelection(true);
      }
    }
    catch (e)
    {
      dump(e);
    }

    editor.endTransaction();

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