/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 * Portions created by the Initial Developer are Copyright (C) 2000
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Jan Varga <varga@ku.sk>
 *   Hakan Waara <hwaara@chello.se>
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

//NOTE: gMessengerBundle must be defined and set or this Overlay won't work

// Function to change the highlighted row back to the row that is currently
// outline/dotted without loading the contents of either rows.  This is
// triggered when the context menu for a given row is hidden/closed
// (onpopuphiding).
function RestoreSelectionWithoutContentLoad(tree)
{
    // If a delete or move command had been issued, then we should
    // reset gRightMouseButtonDown and gThreadPaneDeleteOrMoveOccurred
    // and return (see bug 142065).
    if(gThreadPaneDeleteOrMoveOccurred)
    {
      gRightMouseButtonDown = false;
      gThreadPaneDeleteOrMoveOccurred = false;
      return;
    }

    var treeSelection = tree.view.selection;

    // make sure that currentIndex is valid so that we don't try to restore
    // a selection of an invalid row.
    if((!treeSelection.isSelected(treeSelection.currentIndex)) &&
       (treeSelection.currentIndex >= 0))
    {
        treeSelection.selectEventsSuppressed = true;
        treeSelection.select(treeSelection.currentIndex);
        treeSelection.selectEventsSuppressed = false;

        // Keep track of which row in the thread pane is currently selected.
        // This is currently only needed when deleting messages.  See
        // declaration of var in msgMail3PaneWindow.js.
        if(tree.id == "threadTree")
          gThreadPaneCurrentSelectedIndex = treeSelection.currentIndex;
    }
    else if(treeSelection.currentIndex < 0)
        // Clear the selection in the case of when a folder has just been
        // loaded where the message pane does not have a message loaded yet.
        // When right-clicking a message in this case and dismissing the
        // popup menu (by either executing a menu command or clicking
        // somewhere else),  the selection needs to be cleared.
        // However, if the 'Delete Message' or 'Move To' menu item has been
        // selected, DO NOT clear the selection, else it will prevent the
        // tree view from refreshing.
        treeSelection.clearSelection();

    // Need to reset gRightMouseButtonDown to false here because
    // TreeOnMouseDown() is only called on a mousedown, not on a key down.
    // So resetting it here allows the loading of messages in the messagepane
    // when navigating via the keyboard or the toolbar buttons *after*
    // the context menu has been dismissed.
    gRightMouseButtonDown = false;
}

function threadPaneOnPopupHiding()
{
  RestoreSelectionWithoutContentLoad(GetThreadTree());
}

function fillThreadPaneContextMenu()
{
  var numSelected = GetNumSelectedMessages();

  var isNewsgroup = false;
  var selectedMessage = null;

  // Clear the global var used to keep track if a 'Delete Message' or 'Move
  // To' command has been triggered via the thread pane context menu.
  gThreadPaneDeleteOrMoveOccurred = false;

  if(numSelected >= 0) {
    selectedMessage = GetFirstSelectedMessage();
    isNewsgroup = IsNewsMessage(selectedMessage);
  }

  SetupNewMessageWindowMenuItem("threadPaneContext-openNewWindow", numSelected, false);
  SetupEditAsNewMenuItem("threadPaneContext-editAsNew", numSelected, false);

  ShowMenuItem("threadPaneContext-sep-open", (numSelected <= 1));

  SetupReplyToSenderMenuItem("threadPaneContext-replySender", numSelected, false);
  SetupReplyToNewsgroupMenuItem("threadPaneContext-replyNewsgroup", numSelected, isNewsgroup, false);
  SetupReplyAllMenuItem("threadPaneContext-replyAll", numSelected, false);
  SetupForwardMenuItem("threadPaneContext-forward", numSelected, false);
  SetupForwardAsAttachmentMenuItem("threadPaneContext-forwardAsAttachment", numSelected, false);

  ShowMenuItem("threadPaneContext-sep-reply", true);

  SetupCopyMessageUrlMenuItem("threadPaneContext-copyMessageUrl", numSelected, isNewsgroup, numSelected != 1); 
  SetupCopyMenuItem("threadPaneContext-copyMenu", numSelected, false);
  SetupMoveMenuItem("threadPaneContext-moveMenu", numSelected, isNewsgroup, false);
  EnableMenuItem("threadPaneContext-tags", (numSelected >= 1));
  EnableMenuItem("threadPaneContext-mark", (numSelected >= 1));
  SetupSaveAsMenuItem("threadPaneContext-saveAs", numSelected, false);
  SetupPrintPreviewMenuItem("threadPaneContext-printpreview", numSelected, false);
  SetupPrintMenuItem("threadPaneContext-print", numSelected, false);
  SetupDeleteMenuItem("threadPaneContext-delete", numSelected, false);
  SetupAddSenderToABMenuItem("threadPaneContext-addSenderToAddressBook", numSelected, false);
  SetupAddAllToABMenuItem("threadPaneContext-addAllToAddressBook", numSelected, false);

  ShowMenuItem("threadPaneContext-sep-edit", (numSelected <= 1));

  return(true);
}

function SetupNewMessageWindowMenuItem(menuID, numSelected, forceHide)
{
  ShowMenuItem(menuID, (numSelected <= 1) && !forceHide);
  EnableMenuItem(menuID, (numSelected == 1));
}

function SetupEditAsNewMenuItem(menuID, numSelected, forceHide)
{
  ShowMenuItem(menuID, (numSelected <= 1)&& !forceHide);
  EnableMenuItem(menuID, (numSelected == 1));
}

function SetupReplyToSenderMenuItem(menuID, numSelected, forceHide)
{
  ShowMenuItem(menuID, (numSelected <= 1)&& !forceHide);
  EnableMenuItem(menuID, (numSelected == 1));
}

function SetupReplyToNewsgroupMenuItem(menuID, numSelected, isNewsgroup, forceHide)
{
  ShowMenuItem(menuID, (numSelected <= 1) && isNewsgroup && !forceHide);
  EnableMenuItem(menuID,  (numSelected == 1));
}

function SetupReplyAllMenuItem(menuID, numSelected, forceHide)
{
  ShowMenuItem(menuID, (numSelected <= 1) && !forceHide);
  EnableMenuItem(menuID, (numSelected == 1));
}

function SetupForwardMenuItem(menuID, numSelected, forceHide)
{
  ShowMenuItem(menuID,  (numSelected <= 1) && !forceHide);
  EnableMenuItem(menuID, (numSelected > 0));
}

function SetupForwardAsAttachmentMenuItem(menuID, numSelected, forceHide)
{
  ShowMenuItem(menuID,  (numSelected > 1) && !forceHide);
  EnableMenuItem(menuID, (numSelected > 1));
}

function SetupMoveMenuItem(menuID, numSelected, isNewsgroup, forceHide)
{
  ShowMenuItem(menuID, !isNewsgroup && !forceHide);

  var msgFolder = GetLoadedMsgFolder();
  // disable move if we can't delete message(s) from this folder
  var enableMenuItem = (numSelected > 0) && msgFolder && msgFolder.canDeleteMessages;
  EnableMenuItem(menuID, enableMenuItem);
}

function SetupCopyMessageUrlMenuItem(menuID, numSelected, isNewsgroup, forceHide)
{
  ShowMenuItem(menuID, isNewsgroup && !forceHide);
  EnableMenuItem(menuID, (numSelected > 0));
}

function SetupCopyMenuItem(menuID, numSelected, forceHide)
{
  ShowMenuItem(menuID, !forceHide);
  EnableMenuItem(menuID, (numSelected > 0));
}

function SetupTagsMenuItem(menuID, numSelected, forceHide)
{
  ShowMenuItem(menuID, (numSelected <= 1) && !forceHide);
  EnableMenuItem(menuID, (numSelected == 1));
}

function SetupMarkMenuItem(menuID, numSelected, forceHide)
{
  ShowMenuItem(menuID, (numSelected <= 1) && !forceHide);
  EnableMenuItem(menuID, (numSelected == 1));
}

function SetupSaveAsMenuItem(menuID, numSelected, forceHide)
{
  ShowMenuItem(menuID, (numSelected <= 1) && !forceHide);
  EnableMenuItem(menuID, (numSelected == 1));
}

function SetupPrintPreviewMenuItem(menuID, numSelected, forceHide)
{
  ShowMenuItem(menuID, (numSelected <= 1) && !forceHide);
  EnableMenuItem(menuID, (numSelected == 1));
}

function SetupPrintMenuItem(menuID, numSelected, forceHide)
{
  ShowMenuItem(menuID, !forceHide);
  EnableMenuItem(menuID, (numSelected > 0));
}

function SetupAddSenderToABMenuItem(menuID, numSelected, forceHide)
{
  ShowMenuItem(menuID, (numSelected <= 1) && !forceHide);
  EnableMenuItem(menuID, false);
}

function SetupAddAllToABMenuItem(menuID, numSelected, forceHide)
{
  ShowMenuItem(menuID, (numSelected <= 1) && !forceHide);
  EnableMenuItem(menuID, false);
}

function SetupDeleteMenuItem(menuID, numSelected, forceHide)
{
  // This function is needed for the case where a folder is just loaded (while
  // there isn't a message loaded in the message pane), a right-click is done
  // in the thread pane.  This function will disable enable the 'Delete
  // Message' menu item.
  ShowMenuItem(menuID, !forceHide);
  EnableMenuItem(menuID, (numSelected > 0));
  goUpdateCommand('cmd_delete');
}

function folderPaneOnPopupHiding()
{
  RestoreSelectionWithoutContentLoad(GetFolderTree());
}

function fillFolderPaneContextMenu()
{
  if (IsFakeAccount())
    return false;

  var folderTree = GetFolderTree();
  var startIndex = {};
  var endIndex = {};
  folderTree.view.selection.getRangeAt(0, startIndex, endIndex);
  if (startIndex.value < 0)
    return false;
  var numSelected = endIndex.value - startIndex.value + 1;
  var folderResource = GetFolderResource(folderTree, startIndex.value);

  var isServer = GetFolderAttribute(folderTree, folderResource, "IsServer") == 'true';
  var serverType = GetFolderAttribute(folderTree, folderResource, "ServerType");
  var specialFolder = GetFolderAttribute(folderTree, folderResource, "SpecialFolder");
  var canSubscribeToFolder = (serverType == "nntp") || (serverType == "imap");
  var isNewsgroup = !isServer && serverType == 'nntp';
  var isMailFolder = !isServer && serverType != 'nntp';
  var isVirtualFolder = (specialFolder == "Virtual");
  var canGetMessages =  (isServer && (serverType != "nntp") && (serverType !="none")) || isNewsgroup;

  EnableMenuItem("folderPaneContext-properties", true);
  ShowMenuItem("folderPaneContext-getMessages", (numSelected <= 1) && canGetMessages);
  EnableMenuItem("folderPaneContext-getMessages", true);

  ShowMenuItem("folderPaneContext-openNewWindow", (numSelected <= 1) && !isServer);
  EnableMenuItem("folderPaneContext-openNewWindow", true);

  SetupRenameMenuItem(folderResource, numSelected, isServer, serverType, specialFolder);
  SetupRemoveMenuItem(folderResource, numSelected, isServer, serverType, specialFolder);
  SetupCompactMenuItem(folderResource, numSelected);

  ShowMenuItem("folderPaneContext-copy-location", !isServer && !isVirtualFolder);
  ShowMenuItem("folderPaneContext-emptyTrash", (numSelected <= 1) && (specialFolder == 'Trash'));
  EnableMenuItem("folderPaneContext-emptyTrash", true);

  var showSendUnsentMessages = (numSelected <= 1) && (specialFolder == 'Unsent Messages');
  ShowMenuItem("folderPaneContext-sendUnsentMessages", showSendUnsentMessages);
  if (showSendUnsentMessages) {
    EnableMenuItem("folderPaneContext-sendUnsentMessages", IsSendUnsentMsgsEnabled(folderResource));
  }

  ShowMenuItem("folderPaneContext-sep-edit", (numSelected <= 1));

  SetupNewMenuItem(folderResource, numSelected, isServer, serverType, specialFolder);

  ShowMenuItem("folderPaneContext-subscribe", (numSelected <= 1) && canSubscribeToFolder && !isVirtualFolder);
  EnableMenuItem("folderPaneContext-subscribe", true);

  ShowMenuItem("folderPaneContext-sep1", (numSelected <= 1) && !isServer);
// News folder context menu =============================================

  ShowMenuItem("folderPaneContext-newsUnsubscribe", (numSelected <= 1) && canSubscribeToFolder && isNewsgroup);
  EnableMenuItem("folderPaneContext-newsUnsubscribe", true);
  ShowMenuItem("folderPaneContext-markNewsgroupAllRead", (numSelected <= 1) && isNewsgroup);
  EnableMenuItem("folderPaneContext-markNewsgroupAllRead", true);

// End of News folder context menu =======================================

  ShowMenuItem("folderPaneContext-markMailFolderAllRead", (numSelected <= 1) && isMailFolder && !isVirtualFolder);
  EnableMenuItem("folderPaneContext-markMailFolderAllRead", true);

  ShowMenuItem("folderPaneContext-searchMessages", (numSelected <= 1) && !isVirtualFolder);
  goUpdateCommand('cmd_search');

  return(true);
}

function SetupRenameMenuItem(folderResource, numSelected, isServer, serverType, specialFolder)
{
  var msgFolder = folderResource.QueryInterface(Components.interfaces.nsIMsgFolder);
  var folderTree = GetFolderTree();
  var isSpecialFolder = !(specialFolder == "none" || (specialFolder == "Junk" && CanRenameDeleteJunkMail(msgFolder.URI))
                                                  || (specialFolder == "Virtual"));
  var canRename = GetFolderAttribute(folderTree, folderResource, "CanRename") == "true";

  ShowMenuItem("folderPaneContext-rename", (numSelected <= 1) && !isServer && !isSpecialFolder && canRename);
  var folder = GetMsgFolderFromResource(folderResource);
  EnableMenuItem("folderPaneContext-rename", !isServer && folder.isCommandEnabled("cmd_renameFolder"));

  if(canRename)
  {
    SetMenuItemLabel("folderPaneContext-rename", gMessengerBundle.getString("renameFolder"));
  }
}

function SetupRemoveMenuItem(folderResource, numSelected, isServer, serverType, specialFolder)
{
  var msgFolder = folderResource.QueryInterface(Components.interfaces.nsIMsgFolder);
  var isMail = serverType != 'nntp';
  var isSpecialFolder = !(specialFolder == "none" || (specialFolder == "Junk" && CanRenameDeleteJunkMail(msgFolder.URI))
                                                  || (specialFolder == "Virtual"));
  //Can't currently delete Accounts or special folders.
  var showRemove = (numSelected <=1) && (isMail && !isSpecialFolder) && !isServer;

  ShowMenuItem("folderPaneContext-remove", showRemove);
  if(showRemove)
  {
    var folder = GetMsgFolderFromResource(folderResource);
    EnableMenuItem("folderPaneContext-remove", folder.isCommandEnabled("cmd_delete"));
  }
  if(isMail && !isSpecialFolder)
  {
    SetMenuItemLabel("folderPaneContext-remove", gMessengerBundle.getString("removeFolder"));
  }
}

function SetupCompactMenuItem(folderResource, numSelected)
{
  var folderTree = GetFolderTree();
  var canCompact = GetFolderAttribute(folderTree, folderResource, "CanCompact") == "true";
  ShowMenuItem("folderPaneContext-compact", (numSelected <=1) && canCompact);
  var folder = GetMsgFolderFromResource(folderResource);
  EnableMenuItem("folderPaneContext-compact", folder.isCommandEnabled("cmd_compactFolder"));

  if(canCompact)
  {
    SetMenuItemLabel("folderPaneContext-compact", gMessengerBundle.getString("compactFolder"));
  }
}

function SetupNewMenuItem(folderResource, numSelected, isServer, serverType, specialFolder)
{
  var folderTree = GetFolderTree();
  var canCreateNew = GetFolderAttribute(folderTree, folderResource, "CanCreateSubfolders") == "true";
  var isInbox = specialFolder == "Inbox";

  var isIMAPFolder = GetFolderAttribute(folderTree, folderResource,
                       "ServerType") == "imap";

  var ioService = Components.classes["@mozilla.org/network/io-service;1"]
                         .getService(Components.interfaces.nsIIOService);

  var showNew = ((numSelected <=1) && (serverType != 'nntp') && canCreateNew) || isInbox;
  ShowMenuItem("folderPaneContext-new", showNew);
  EnableMenuItem("folderPaneContext-new", !isIMAPFolder || !ioService.offline);
  if(showNew)
  {
    if(isServer || isInbox)
      SetMenuItemLabel("folderPaneContext-new", gMessengerBundle.getString("newFolder"));
    else
      SetMenuItemLabel("folderPaneContext-new", gMessengerBundle.getString("newSubfolder"));
  }
}

function ShowMenuItem(id, showItem)
{
  var item = document.getElementById(id);
  if(item && item.hidden != "true") 
    item.hidden = !showItem;
}

function EnableMenuItem(id, enableItem)
{
  var item = document.getElementById(id);
  if(item)
  {
    var enabled = (item.getAttribute('disabled') !='true');
    if(enableItem != enabled)
    {
      item.setAttribute('disabled', enableItem ? '' : 'true');
    }
  }
}

function SetMenuItemLabel(id, label)
{
  var item = document.getElementById(id);
  if(item)
    item.setAttribute('label', label);
}

function SetMenuItemAccessKey(id, accessKey)
{
  var item = document.getElementById(id);
  if(item)
    item.setAttribute('accesskey', accessKey);
}

function fillMessagePaneContextMenu()
{
  var message = GetLoadedMessage();
  var numSelected = (message) ? 1 : 0;

  var isNewsgroup = false;

  if (numSelected == 1)
    isNewsgroup = IsNewsMessage(message);

  // don't show mail items for links/images, just show related items.
  var hideMailItems = gContextMenu.onImage || gContextMenu.onLink;

  SetupEditAsNewMenuItem("messagePaneContext-editAsNew", numSelected, (numSelected == 0 || hideMailItems));
  SetupReplyToSenderMenuItem("messagePaneContext-replySender", numSelected, (numSelected == 0 || hideMailItems));
  SetupReplyToNewsgroupMenuItem("messagePaneContext-replyNewsgroup", numSelected, isNewsgroup, (numSelected == 0 || hideMailItems));
  SetupReplyAllMenuItem("messagePaneContext-replyAll" , numSelected, (numSelected == 0 || hideMailItems));
  SetupForwardMenuItem("messagePaneContext-forward", numSelected, (numSelected == 0 || hideMailItems));
  SetupCopyMessageUrlMenuItem("messagePaneContext-copyMessageUrl", numSelected, isNewsgroup, (numSelected == 0 || hideMailItems)); 
  SetupCopyMenuItem("messagePaneContext-copyMenu", numSelected, (numSelected == 0 || hideMailItems));
  SetupMoveMenuItem("messagePaneContext-moveMenu", numSelected, isNewsgroup, (numSelected == 0 || hideMailItems));
  SetupTagsMenuItem("messagePaneContext-tags", numSelected, (numSelected == 0 || hideMailItems));
  SetupMarkMenuItem("messagePaneContext-mark", numSelected, (numSelected == 0 || hideMailItems));
  SetupSaveAsMenuItem("messagePaneContext-saveAs", numSelected, (numSelected == 0 || hideMailItems));
  SetupPrintPreviewMenuItem("messagePaneContext-printpreview", numSelected, (numSelected == 0 || hideMailItems));
  SetupPrintMenuItem("messagePaneContext-print", numSelected, (numSelected == 0 || hideMailItems));
  if (numSelected == 0 || hideMailItems)
    ShowMenuItem("messagePaneContext-delete", false)
  else {
    goUpdateCommand('cmd_delete');
    ShowMenuItem("messagePaneContext-delete", true)
  }
  SetupAddSenderToABMenuItem("messagePaneContext-addSenderToAddressBook", numSelected, (numSelected == 0 || hideMailItems));
  SetupAddAllToABMenuItem("messagePaneContext-addAllToAddressBook", numSelected, (numSelected == 0 || hideMailItems));

  ShowMenuItem("context-addemail", gContextMenu.onMailtoLink);
  ShowMenuItem("context-composeemailto", gContextMenu.onMailtoLink);
  ShowMenuItem("context-createfilterfrom", gContextMenu.onMailtoLink);

  //Figure out separators
  ShowMenuItem("messagePaneContext-sep-open", ShowSeparator("messagePaneContext-sep-open"));
  ShowMenuItem("messagePaneContext-sep-reply", ShowSeparator("messagePaneContext-sep-reply"));
  ShowMenuItem("messagePaneContext-sep-edit", ShowSeparator("messagePaneContext-sep-edit") || gContextMenu.onMailtoLink);
  ShowMenuItem("messagePaneContext-sep-link", ShowSeparator("messagePaneContext-sep-link"));
  ShowMenuItem("messagePaneContext-sep-image", ShowSeparator("messagePaneContext-sep-image"));
  ShowMenuItem("messagePaneContext-sep-copy", ShowSeparator("messagePaneContext-sep-copy"));
  ShowMenuItem("messagePaneContext-sep-tags", ShowSeparator("messagePaneContext-sep-tags"));
  ShowMenuItem("messagePaneContext-sep-mark", ShowSeparator("messagePaneContext-sep-mark"));
  
  // if we are on an non-mailto link, go ahead and hide this separator
  if (gContextMenu.onLink && !gContextMenu.onMailtoLink)
    ShowMenuItem("messagePaneContext-sep-edit", false);
}

function ShowSeparator(aSeparatorID)
{
  var separator = document.getElementById(aSeparatorID);
  var sibling = separator.previousSibling;
  var siblingID;
  var siblingNextHiddenAttrib = separator.nextSibling.getAttribute("hidden");

  while (sibling && sibling.localName != "menuseparator") {
    siblingID = sibling.getAttribute("id");
    // for some reason, context-blockimage and context-unblockimage is not
    // hidden on the very first time the context menu is invoked.  It's only
    // hidden on subsequent triggers of the context menu.  Since we're not
    // using these two menuitems in mailnews, we can ignore it if encountered.
    if ((sibling.getAttribute("hidden") != "true") && 
        (siblingNextHiddenAttrib != "true") &&
        (siblingID != "context-blockimage") &&
        (siblingID != "context-unblockimage"))
      return true;
    sibling = sibling.previousSibling;
  }
  return false;
}

function IsMenuItemShowing(menuID)
{
  var item = document.getElementById(menuID);
  if (item)
    return item.hidden != "true";
  return false;
}

// message pane context menu helper methods
function AddNodeToAddressBook(emailAddressNode)
{
  if (emailAddressNode)
    AddEmailToAddressBook(emailAddressNode.getAttribute("emailAddress"),
                          emailAddressNode.getAttribute("displayName"));
}

function AddEmailToAddressBook(primaryEmail, displayName)
{
    window.openDialog("chrome://messenger/content/addressbook/abNewCardDialog.xul",
                      "", "chrome,resizable=no,titlebar,modal,centerscreen",
                      {primaryEmail:primaryEmail, displayName:displayName});
}

// SendMailToNode takes the email address title button, extracts
// the email address we stored in there and opens a compose window
// with that address
function SendMailToNode(emailAddressNode)
{
  if (emailAddressNode)
    SendMailTo(emailAddressNode.getAttribute("fullAddress"));
}

function SendMailTo(fullAddress)
{
  var fields = Components.classes["@mozilla.org/messengercompose/composefields;1"].createInstance(Components.interfaces.nsIMsgCompFields);
  var params = Components.classes["@mozilla.org/messengercompose/composeparams;1"].createInstance(Components.interfaces.nsIMsgComposeParams);
  if (fields && params)
  {
    fields.to = fullAddress;
    params.type = Components.interfaces.nsIMsgCompType.New;
    params.format = Components.interfaces.nsIMsgCompFormat.Default;
    params.identity = accountManager.getFirstIdentityForServer(GetLoadedMsgFolder().server);
    params.composeFields = fields;
    msgComposeService.OpenComposeWindowWithParams(null, params);
  }
}

// CopyEmailAddress takes the email address title button, extracts
// the email address we stored in there and copies it to the clipboard
function CopyEmailAddress(emailAddressNode)
{
  if (emailAddressNode)
    CopyString(emailAddressNode.getAttribute("emailAddress"));
}

// CreateFilter opens the Message Filters and Filter Rules dialogs.
//The Filter Rules dialog has focus. The window is prefilled with filtername <email address>
//Sender condition is selected and the value is prefilled <email address>
function CreateFilter(emailAddressNode)
{
  if (emailAddressNode)
    CreateFilterFromMail(emailAddressNode.getAttribute("emailAddress"));
}

function CreateFilterFromMail(emailAddress)
{
  if (emailAddress)
    top.MsgFilters(emailAddress, GetFirstSelectedMsgFolder());
}

function CopyFolderUrl()
{
  try 
  {
    var folderResource = GetSelectedFolderResource();
    if (folderResource)
    {
      var msgFolder = folderResource.QueryInterface(Components.interfaces.nsIMsgFolder);
      CopyString(msgFolder.folderURL);
    }
  }
  catch (ex) 
  {
    dump("ex="+ex+"\n");
  }
}

function CopyMessageUrl()
{
  try {
    var hdr = gDBView.hdrForFirstSelectedMessage;
    var server = hdr.folder.server;

    var url;
    if (server.isSecure) {
      url = "snews://";
    }
    else {
      url = "news://"
    }
    url += server.hostName;
    url += ":";
    url += server.port;
    url += "/";
    url += hdr.messageId;
    CopyString(url);
  }
  catch (ex) {
    dump("ex="+ex+"\n");
  }
}

function CopyString(aString)
{
  Components.classes["@mozilla.org/widget/clipboardhelper;1"]
            .getService(Components.interfaces.nsIClipboardHelper)
            .copyString(aString);
}
