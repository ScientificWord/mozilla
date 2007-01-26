/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
  Copyright 2007 MacKichan Software, Inc.
 * ***** END LICENSE BLOCK ***** */

function msiEditorFillContextMenu(event, contextMenuNode)
{
  if ( event.target != contextMenuNode )
    return;

  // Setup object property menuitem
  var editorElement = msiGetCurrentEditorElementForWindow(window);
  var dumpStr = "Curr context editor element is [";
  if (editorElement)
    dumpStr += editorElement.id;
  dump(dumpStr + "]\n");
  var objectName = msiInitObjectPropertiesMenuitem(editorElement, "objectProperties_cm");
  var isInLink = objectName == "href";

  // Special case of an image inside a link
  if (objectName == "img")
  try {
    isInLink = msiGetEditor(editorElement).getElementOrParentByTagName("href", msiGetObjectForProperties(editorElement));
  } catch (e) {}

  msiInitRemoveStylesMenuitems(editorElement, "removeStylesMenuitem_cm", "removeLinksMenuitem_cm", "removeNamedAnchorsMenuitem_cm");

  var inCell = msiIsInTableCell(editorElement);
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

