/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
  Copyright 2007 MacKichan Software, Inc.
 * ***** END LICENSE BLOCK ***** */

const msiEditorContextMenuJS_duplicateTest = "Bad";

function msiEditorFillContextMenu(event, contextMenuNode)
{
  if ( event.target != contextMenuNode )
    return;
  // Setup object property menuitem
  var editorElement = msiGetCurrentEditorElementForWindow(window);
  var editor = msiGetEditor(editorElement);
  var dumpStr = "Curr context editor element is [";
  if (editorElement)
    dumpStr += editorElement.id;
  dump(dumpStr + "]\n");
  var objectName = msiInitObjectPropertiesMenuitem(editorElement, "propertiesMenu_cm");
  var isInLink = !!editor.getSelectedElement("href");
  // Special case of an image inside a link
  if (objectName == "img")
  try {
    var objData = msiGetObjectDataForProperties(editorElement);
    // if (objData != null && objData.getNode() != null)
      // isInLink = msiGetEditor(editorElement).getElementOrParentByTagName("href", objData.theNode());
  } catch (e) {}

  msiInitRemoveStylesMenuitems(editorElement, "removeStylesMenuitem_cm", "removeLinksMenuitem_cm", "removeNamedAnchorsMenuitem_cm");

  var inCell = msiIsInTableCell(editorElement);
  var inMatrixCell = msiIsInMatrixCell(editorElement);
// HACK alert
  if (inMatrixCell)
	{
	  inCell = false;
	}
  // Set appropriate text for join cells command
  msiInitJoinCellMenuitem("joinTableCells_cm", editorElement);

  // Update enable states for all table commands
  msiGoUpdateTableMenuItems(document.getElementById("composerTableMenuItems"), editorElement);

  // Loop through all children to hide disabled items
  var children = contextMenuNode.childNodes;
  if (children)
  {
    var count = children.length;
    for (var i = 0; i < count; i++)
      msiHideDisabledItem(children[i]);
  }

  // The above loop will always show all separators and the next two items
  // Hide "Create Link" if in a link
  msiShowMenuItem("createLink_cm", !isInLink);

  // Hide "Edit link in new Composer" unless in a link
  msiShowMenuItem("editLink_cm", isInLink);

  msiShowMenuItem("followLink_cm", isInLink);

  // Remove separators if all items in immediate group above are hidden
  // A bit complicated to account if multiple groups are completely hidden!
  var haveUndo =
    msiIsMenuItemShowing("menu_undo_cm") ||
    msiIsMenuItemShowing("menu_redo_cm");

  var haveEdit =
    msiIsMenuItemShowing("menu_cut_cm")   ||
    msiIsMenuItemShowing("menu_copy_cm")  ||
    msiIsMenuItemShowing("menu_paste_cm") ||
    msiIsMenuItemShowing("menu_pasteNoFormatting_cm") ||
    msiIsMenuItemShowing("menu_delete_cm");

  var haveStyle =
    msiIsMenuItemShowing("removeStylesMenuitem_cm") ||
    msiIsMenuItemShowing("createLink_cm") ||
    msiIsMenuItemShowing("removeLinksMenuitem_cm") ||
    msiIsMenuItemShowing("removeNamedAnchorsMenuitem_cm");

  var haveProps =
    msiIsMenuItemShowing("objectProperties_cm");

  msiShowMenuItem("undoredo-separator", haveUndo && haveEdit);

  msiShowMenuItem("edit-separator", haveEdit || haveUndo);

  // Note: Item "menu_selectAll_cm" and
  // following separator are ALWAYS enabled,
  // so there will always be 1 separator here

  var showStyleSep = haveStyle && (haveProps || inCell);
  msiShowMenuItem("styles-separator", showStyleSep);

  var showPropSep = (haveProps && inCell);
  msiShowMenuItem("property-separator", showPropSep);

  // Remove table submenus if not in table
  msiShowMenuItem("tableInsertMenu_cm",  inCell);
  msiShowMenuItem("tableSelectMenu_cm",  inCell);
  msiShowMenuItem("tableDeleteMenu_cm",  inCell);
	// Same for matrices
  msiShowMenuItem("matrixInsertMenu_cm",  inMatrixCell);
  msiShowMenuItem("matrixSelectMenu_cm",  inMatrixCell);
  msiShowMenuItem("matrixDeleteMenu_cm",  inMatrixCell);

//  addSectionProperties(editorElement, contextMenuNode);
}

function msiIsItemOrCommandEnabled( item )
{
  var command = item.getAttribute("command");
  if (command) {
    // If possible, query the command controller directly
    var controller = document.commandDispatcher.getControllerForCommand(command);
    if (controller)
      return controller.isCommandEnabled(command);
  }

  // Fall back on the inefficient observed disabled attribute
  return item.getAttribute("disabled") != "true";
}

function msiHideDisabledItem( item )
{
  item.hidden = !msiIsItemOrCommandEnabled(item);
}

function msiShowMenuItem(id, showItem)
{
  var item = document.getElementById(id);
  if (item && !showItem)
  {
    item.hidden = true;
  }
  // else HideDisabledItem showed the item anyway
}

function msiIsMenuItemShowing(menuID)
{
  var item = document.getElementById(menuID);
  if (item)
    return !item.hidden;

  return false;
}

function addSectionProperties(editorElement, contextMenuNode)
{
  var editor = msiGetEditor(editorElement);
  var parentTagString;
  var parentTagArray;
  var labelString;
  var tag;
  var i, length;
  removeSectionProperties(contextMenuNode);
  if (editor && editor.tagListManager)
  {
    parentTagString = editor.tagListManager.getParentTagList(",",false,false);
    parentTagArray = parentTagString.split(","); 
    labelString = GetString("TagPropertiesMenuLabel");
    for (i = 0, length = parentTagArray.length; i < length; i++)
    {
      tag = parentTagArray[i].replace(/\s-\s\w*$/,"");
      if (/\w/.test(tag) && editor.tagListManager.getTagInClass("paratag", tag, null))
      {
        labelString = labelString.replace(/%tagname%/, tag);
        createContextPropertiesItem(tag, labelString, tag, contextMenuNode,"paratag");
//        createContextPropertiesItem(tag,"Properties of "+tag,tag, contextMenuNode,"paratag");
      }
    }

    for (i = 0, length = parentTagArray.length; i < length; i++)
    {
      tag = parentTagArray[i].replace(/\s-\s\w*$/,"");
      dump("tag = "+tag+", element.localName = "+element.localName+"\n");
      if (/\w/.test(tag) && editor.tagListManager.getTagInClass("structtag", tag, null))
      {
        labelString = labelString.replace(/%tagname%/, tag);
        createContextPropertiesItem(tag, labelString, element, contextMenuNode, "structtag");
//        createContextPropertiesItem(tag,"Properties of "+tag,tag, contextMenuNode, "structtag");
      }
      element = element.parentNode;
    }
  }
}

function createContextPropertiesItem(id, label, target, contextMenuNode, tagfamily)
{
  var item = document.createElementNS("http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul",
                                         "menuitem");

  item.setAttribute("oncommand",tagfamily==="structtag"?"openStructureTagDialog('"+target+"');":"openParaTagDialog('"+target+"');");
  item.id = "contexttagitem_"+id;
  item.setAttribute("label",label);                                               
  item.setAttribute("value",target);
  
  return contextMenuNode.appendChild(item);
}

function removeSectionProperties(contextMenuNode)
{
  var item = document.getElementById("structure-properties-separator");
  item = item.nextSibling;
  var next;
  while (item && item.localName != "menuitem") item = item.nextSibling;
  while (item && item.localName === "menuitem")
  {                   
    next = item.nextSibling;
    item.parentNode.removeChild(item);
    item = next;
  }
}
    
  
   