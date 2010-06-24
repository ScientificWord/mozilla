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
 * Portions created by the Initial Developer are Copyright (C) 1998-2003
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *    Charles Manske (cmanske@netscape.com)
 *    Neil Rashbrook (neil@parkwaycc.co.uk)
 *    Daniel Glazman (glazman@disruptive-innovations.com), on behalf of Linspire Inc.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
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

var gActiveEditor;
var anchorElement = null;
var imageElement = null;
var insertNew = false;
var replaceExistingLink = false;
var insertLinkAtCaret;
var needLinkText = false;
var href;
var newLinkText;
var gHNodeArray = {};
var gHaveNamedAnchors = false;
var gHaveHeadings = false;
var gCanChangeHeadingSelected = true;
var gCanChangeAnchorSelected = true;
var gHaveDocumentUrl = false;

var gCheckboxes = [ "met", "colleague", "co-worker", "muse", "crush", "date", "sweetheart" ];

// NOTE: Use "href" instead of "a" to distinguish from Named Anchor
// The returned node is has an "a" tagName
var tagName = "href";

// dialog initialization code
function Startup()
{
  var editorElement = msiGetParentEditorElementForDialog(window);
  gActiveEditor = msiGetEditor(editorElement);
  if (!gActiveEditor)
  {
    dump("Failed to get active editor!\n");
    window.close();
    return;
  }
  // Message was wrapped in a <label> or <div>, so actual text is a child text node
  gDialog.linkTextCaption     = document.getElementById("linkTextCaption");
  gDialog.linkTextMessage     = document.getElementById("linkTextMessage");
  gDialog.linkTextInput       = document.getElementById("linkTextInput");
  gDialog.hrefInput           = document.getElementById("hrefInput");
  gDialog.makeRelativeLink    = document.getElementById("MakeRelativeLink");
  gDialog.AdvancedEditSection = document.getElementById("AdvancedEdit");
//  gDialog.IsMailAddress       = document.getElementById("IsMailAddress");

  gDialog.MoreSection         = document.getElementById("MoreSection");
  gDialog.MoreFewerButton     = document.getElementById("MoreFewerButton");

  gDialog.userDefinedTargetCheckbox = document.getElementById("userDefinedTargetCheckbox");
  gDialog.targetRadiogroup        = document.getElementById("targetRadiogroup");
  gDialog.userDefinedTarget       = document.getElementById("userDefinedTarget");

  // See if we have a single selected image
  imageElement = gActiveEditor.getSelectedElement("img");

  if (imageElement)
  {
    // Get the parent link if it exists -- more efficient than GetSelectedElement()
    anchorElement = gActiveEditor.getElementOrParentByTagName("href", imageElement);
    if (anchorElement)
    {
      if (anchorElement.childNodes.length > 1)
      {
        // If there are other children, then we want to break
        //  this image away by inserting a new link around it,
        //  so make a new node and copy existing attributes
        anchorElement = anchorElement.cloneNode(false);
        //insertNew = true;
        replaceExistingLink = true;
      }
    }
  }
  else
  {
    // Get an anchor element if caret or
    //   entire selection is within the link.
    anchorElement = gActiveEditor.getSelectedElement(tagName);

    if (anchorElement)
    {
      // Select the entire link
      gActiveEditor.selectElement(anchorElement);
    }
    else
    {
      // If selection starts in a link, but extends beyond it,
      //   the user probably wants to extend existing link to new selection,
      //   so check if either end of selection is within a link
      // POTENTIAL PROBLEM: This prevents user from selecting text in an existing
      //   link and making 2 links. 
      // Note that this isn't a problem with images, handled above

      anchorElement = gActiveEditor.getElementOrParentByTagName("href", gActiveEditor.selection.anchorNode);
      if (!anchorElement)
        anchorElement = gActiveEditor.getElementOrParentByTagName("href", gActiveEditor.selection.focusNode);

      if (anchorElement)
      {
        // But clone it for reinserting/merging around existing
        //   link that only partially overlaps the selection
        anchorElement = anchorElement.cloneNode(false);
        //insertNew = true;
        replaceExistingLink = true;
      }
    }
  }

  if(!anchorElement)
  {
    // No existing link -- create a new one
    anchorElement = gActiveEditor.createElementWithDefaults(tagName);
    insertNew = true;
    // Hide message about removing existing link
    //document.getElementById("RemoveLinkMsg").hidden = true;
  }
  if(!anchorElement)
  {
    dump("Failed to get selected element or create a new one!\n");
    window.close();
    return;
  } 

  // We insert at caret only when nothing is selected
  insertLinkAtCaret = gActiveEditor.selection.isCollapsed;
  
  var selectedText;
  if (insertLinkAtCaret)
  {
    // Groupbox caption:
    gDialog.linkTextCaption.setAttribute("label", GetString("LinkText"));

    // Message above input field:
    gDialog.linkTextMessage.setAttribute("value", GetString("EnterLinkText"));
    gDialog.linkTextMessage.setAttribute("accesskey", GetString("EnterLinkTextAccessKey"));
  }
  else
  {
    if (!imageElement)
    {
      // We get here if selection is exactly around a link node
      // Check if selection has some text - use that first
      selectedText = msiGetSelectionAsText();
      if (!selectedText) 
      {
        // No text, look for first image in the selection
        var children = anchorElement.childNodes;
        if (children)
        {
          for(var i=0; i < children.length; i++) 
          {
            var nodeName = children.item(i).nodeName;
            if (nodeName == "img")
            {
              imageElement = children.item(i);
              break;
            }
          }
        }
      }
    }
    // Set "caption" for link source and the source text or image URL
    if (imageElement)
    {
      gDialog.linkTextCaption.setAttribute("label", GetString("LinkImage"));
      // Link source string is the source URL of image
      // TODO: THIS DOESN'T HANDLE MULTIPLE SELECTED IMAGES!
      gDialog.linkTextMessage.setAttribute("value", imageElement.src);
    } else {
      gDialog.linkTextCaption.setAttribute("label", GetString("LinkText"));
      if (selectedText) 
      {
        // Use just the first 60 characters and add "..."
        gDialog.linkTextMessage.setAttribute("value", TruncateStringAtWordEnd(ReplaceWhitespace(selectedText, " "), 60, true));
      } else {
        gDialog.linkTextMessage.setAttribute("value", GetString("MixedSelection"));
      }
    }
  }

  // Make a copy to use for AdvancedEdit and onSaveDefault
  globalElement = anchorElement.cloneNode(false);

  // Get the list of existing named anchors and headings
  initKeyList();

  // We only need to test for this once per dialog load
  gHaveDocumentUrl = msiGetDocumentBaseUrl();

  // Set data for the dialog controls
  InitDialog();
  
  // Search for a URI pattern in the selected text
  //  as candidate href
  selectedText = TrimString(selectedText); 
  if (!gDialog.hrefInput.value && TextIsURI(selectedText))
      gDialog.hrefInput.value = selectedText;

  // Set initial focus
  if (insertLinkAtCaret) {
    // We will be using the HREF inputbox, so text message
    SetTextboxFocus(gDialog.linkTextInput);
  } else {
    SetTextboxFocus(gDialog.hrefInput);

    // We will not insert a new link at caret, so remove link text input field
    gDialog.linkTextInput.hidden = true;
    gDialog.linkTextInput = null;
  }
    
  // This sets enable state on OK button
  doEnabling();

  InitMoreFewer();

  SetWindowLocation();
}

// Set dialog widgets with attribute data
// We get them from globalElement copy so this can be used
//   by AdvancedEdit(), which is shared by all property dialogs
function InitDialog()
{
  // Must use getAttribute, not "globalElement.href", 
  //  or foreign chars aren't coverted correctly!
  var value = globalElement.getAttribute("href");
  gDialog.hrefInput.value = value;

  // Set "Relativize" checkbox according to current URL state
  msiSetRelativeCheckbox(gDialog.makeRelativeLink);

  var relAttr = globalElement.getAttribute("rel");

  // init the target groupbox
  var targetValue = globalElement.getAttribute("target");
  if (targetValue)
  {
    gDialog.userDefinedTargetCheckbox.checked = true;
    switch (targetValue) {
      case "_top":
      case "_blank":
      case "_parent":
      case "_self":
        e = document.getElementById(targetValue.substr(1) + "Radio");
        gDialog.targetRadiogroup.selectedItem = e;
        break;
      default:
        e = document.getElementById("userdefRadio");
        gDialog.targetRadiogroup.selectedItem = e;
        gDialog.userDefinedTarget.value = targetValue;
        break;
    }
  }
}

function doEnabling()
{
  // We disable Ok button when there's no href text only if inserting a new link
  var href = TrimString(gDialog.hrefInput.value);
  var enable = insertNew ? (TrimString(gDialog.hrefInput.value).length > 0) : true;
  
  // anon. content, so can't use SetElementEnabledById here
  var dialogNode = document.getElementById("linkDlg");
  dialogNode.getButton("accept").disabled = !enable;

  SetElementEnabledById( "AdvancedEditButton1", enable);
  gDialog.targetRadiogroup.disabled = false;
}


function ChangeLinkLocation()
{
  msiSetRelativeCheckbox();
  // Set OK button enable state
  doEnabling();
}

// Get and validate data from widgets.
// Set attributes on globalElement so they can be accessed by AdvancedEdit()
function ValidateData()
{
  href = TrimString(gDialog.hrefInput.value);
  if (href)
  {
    // Set the HREF directly on the editor document's anchor node
    //  or on the newly-created node if insertNew is true
//    if (gDialog.IsMailAddress.checked)
//      href = "mailto:" + href;
    globalElement.setAttribute("href",href);
  }
  else if (insertNew)
  {
    // We must have a URL to insert a new link
    //NOTE: We accept an empty HREF on existing link to indicate removing the link
    ShowInputErrorMessage(GetString("EmptyHREFError"));
    return false;
  }
  if (gDialog.linkTextInput)
  {
    // The text we will insert isn't really an attribute,
    //  but it makes sense to validate it
    newLinkText = TrimString(gDialog.linkTextInput.value);
    if (!newLinkText)
    {
      if (href)
        newLinkText = href
      else
      {
        ShowInputErrorMessage(GetString("EmptyLinkTextError"));
        SetTextboxFocus(gDialog.linkTextInput);
        return false;
      }
    }
  }
  return true;
}

function doHelpButton()
{
  openHelp("link_properties");
  return true;
}

function dumpln(s) { dump(s+"\n"); }

function onAccept()
{
  dumpln("1");
  
  if (ValidateData())
  {
    dumpln("2");
    if (href.length > 0)
    {
      // Copy attributes to element we are changing or inserting
      dumpln(3);
      gActiveEditor.cloneAttributes(anchorElement, globalElement);

      // Coalesce into one undo transaction
      gActiveEditor.beginTransaction();

      // Get text to use for a new link
      if (insertLinkAtCaret)
      {
        // Append the link text as the last child node 
        //   of the anchor node
        dumpln(4);
        var textNode = gActiveEditor.document.createTextNode(newLinkText);
        dumpln(4.1);
        if (textNode)
          anchorElement.appendChild(textNode);
        try {
          dumpln(4.2);
          gActiveEditor.insertElementAtSelection(anchorElement, false);
        } catch (e) {
          dump("Exception occured in InsertElementAtSelection\n");
          return true;
        }
      } else if (insertNew || replaceExistingLink)
      {
        //  Link source was supplied by the selection,
        //  so insert a link node as parent of this
        //  (may be text, image, or other inline content)
        try {
          gActiveEditor.insertLinkAroundSelection(anchorElement);
        } catch (e) {
          dump("Exception occured in InsertElementAtSelection\n");
          return true;
        }
      }
      // Check if the link was to a heading 
      dumpln(6);
      if (href in gHNodeArray)
      {
        var anchorNode = gActiveEditor.createElementWithDefaults("a");
        if (anchorNode)
        {
          anchorNode.name = href.substr(1);

          // Insert the anchor into the document, 
          //  but don't let the transaction change the selection
          gActiveEditor.setShouldTxnSetSelection(false);
          gActiveEditor.insertNode(anchorNode, gHNodeArray[href], 0);
          gActiveEditor.setShouldTxnSetSelection(true);
        }
      }
      gActiveEditor.endTransaction();
    } 
    else if (!insertNew)
    {
      // We already had a link, but empty HREF means remove it
      EditorRemoveTextProperty("href", "");
    }
      dumpln(7);
    SaveWindowLocation();
    return true;
  }
  return false;
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
