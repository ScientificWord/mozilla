/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * ***** BEGIN LICENSE BLOCK *****
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

function DoRDFCommand(dataSource, command, srcArray, argumentArray)
{
  var commandResource = RDF.GetResource(command);
  if (commandResource) {
    try {
      if (!argumentArray)
        argumentArray = Components.classes["@mozilla.org/supports-array;1"]
                        .createInstance(Components.interfaces.nsISupportsArray);

      if (argumentArray)
        argumentArray.AppendElement(msgWindow);
      dataSource.DoCommand(srcArray, commandResource, argumentArray);
    }
    catch(e) {
      if (command == "http://home.netscape.com/NC-rdf#NewFolder") {
        throw(e); // so that the dialog does not automatically close.
      }
      dump("Exception : In mail commands" + e + "\n");
    }
  }
}

function GetNewMessages(selectedFolders, server, compositeDataSource)
{
  var numFolders = selectedFolders.length;
  if (numFolders > 0)
  {
    var msgFolder = selectedFolders[0];

    // Whenever we do get new messages, clear the old new messages.
    if (msgFolder)
    {
      var nsIMsgFolder = Components.interfaces.nsIMsgFolder;
      msgFolder.biffState = nsIMsgFolder.nsMsgBiffState_NoMail;
      msgFolder.clearNewMessages();
    }

    if (compositeDataSource)
    {
      var folderResource = msgFolder.QueryInterface(Components.interfaces.nsIRDFResource);
      var folderArray = Components.classes["@mozilla.org/supports-array;1"].createInstance(Components.interfaces.nsISupportsArray);
      folderArray.AppendElement(folderResource);
      var serverArray = Components.classes["@mozilla.org/supports-array;1"].createInstance(Components.interfaces.nsISupportsArray);
      serverArray.AppendElement(server);
      DoRDFCommand(compositeDataSource, "http://home.netscape.com/NC-rdf#GetNewMessages", folderArray, serverArray);
    }
  }
  else {
    dump("Nothing was selected\n");
  }
}

function getBestIdentity(identities, optionalHint)
{
  var identity = null;

  var identitiesCount = identities.Count();

  try
  {
    // if we have more than one identity and a hint to help us pick one
    if (identitiesCount > 1 && optionalHint) {
      // normalize case on the optional hint to improve our chances of finding a match
      optionalHint = optionalHint.toLowerCase();

      var id;
      // iterate over all of the identities
      var tempID;

      for (id = 0; id < identitiesCount; ++id) {
        tempID = identities.GetElementAt(id).QueryInterface(Components.interfaces.nsIMsgIdentity);
        if (optionalHint.indexOf(tempID.email.toLowerCase()) >= 0) {
          identity = tempID;
          break;
        }
      }

      // if we could not find an exact email address match within the hint fields then maybe the message
      // was to a mailing list. In this scenario, we won't have a match based on email address.
      // Before we just give up, try and search for just a shared domain between the hint and
      // the email addresses for our identities. Hey, it is better than nothing and in the case
      // of multiple matches here, we'll end up picking the first one anyway which is what we would have done
      // if we didn't do this second search. This helps the case for corporate users where mailing lists will have the same domain
      // as one of your multiple identities.

      if (!identity) {
        for (id = 0; id < identitiesCount; ++id) {
          tempID = identities.GetElementAt(id).QueryInterface(Components.interfaces.nsIMsgIdentity);
          // extract out the partial domain
          var start = tempID.email.lastIndexOf("@"); // be sure to include the @ sign in our search to reduce the risk of false positives
          if (optionalHint.search(tempID.email.slice(start).toLowerCase()) >= 0) {
            identity = tempID;
            break;
          }
        }
      }
    }
  }
  catch (ex) {dump (ex + "\n");}

  // Still no matches ?
  // Give up and pick the first one (if it exists), like we used to.
  if (!identity && identitiesCount > 0)
    identity = identities.GetElementAt(0).QueryInterface(Components.interfaces.nsIMsgIdentity);

  return identity;
}

function getIdentityForServer(server, optionalHint)
{
    var identity = null;

    if (server) {
        // Get the identities associated with this server.
        var identities = accountManager.GetIdentitiesForServer(server);
        // dump("identities = " + identities + "\n");
        // Try and find the best one.
        identity = getBestIdentity(identities, optionalHint);
    }

    return identity;
}

function GetNextNMessages(folder)
{
  if (folder) {
    var newsFolder = folder.QueryInterface(Components.interfaces.nsIMsgNewsFolder);
    if (newsFolder) {
      newsFolder.getNextNMessages(msgWindow);
    }
  }
}

// type is a nsIMsgCompType and format is a nsIMsgCompFormat
function ComposeMessage(type, format, folder, messageArray)
{
  var msgComposeType = Components.interfaces.nsIMsgCompType;
  var identity = null;
  var newsgroup = null;
  var server;

  // dump("ComposeMessage folder=" + folder + "\n");
  try
  {
    if (folder)
    {
      // Get the incoming server associated with this uri.
      server = folder.server;

      // If they hit new or reply and they are reading a newsgroup,
      // turn this into a new post or a reply to group.
      if (!folder.isServer && server.type == "nntp" && type == msgComposeType.New)
      {
        type = msgComposeType.NewsPost;
        newsgroup = folder.folderURL;
      }

      identity = getIdentityForServer(server);
      // dump("identity = " + identity + "\n");
    }
  }
  catch (ex)
  {
    dump("failed to get an identity to pre-select: " + ex + "\n");
  }

  // dump("\nComposeMessage from XUL: " + identity + "\n");
  var uri = null;

  if (!msgComposeService)
  {
    dump("### msgComposeService is invalid\n");
    return;
  }

  if (type == msgComposeType.New)
  {
    // New message.

    // dump("OpenComposeWindow with " + identity + "\n");

    // If the addressbook sidebar panel is open and has focus, get
    // the selected addresses from it.
    if (document.commandDispatcher.focusedWindow.document.documentElement.hasAttribute("selectedaddresses"))
      NewMessageToSelectedAddresses(type, format, identity);
    else
      msgComposeService.OpenComposeWindow(null, null, type, format, identity, msgWindow);
    return;
  }
  else if (type == msgComposeType.NewsPost)
  {
    // dump("OpenComposeWindow with " + identity + " and " + newsgroup + "\n");
    msgComposeService.OpenComposeWindow(null, newsgroup, type, format, identity, msgWindow);
    return;
  }

  messenger.SetWindow(window, msgWindow);

  var object = null;

  if (messageArray && messageArray.length > 0)
  {
    uri = "";
    for (var i = 0; i < messageArray.length; ++i)
    {
      var messageUri = messageArray[i];

      var hdr = messenger.msgHdrFromURI(messageUri);
      // If we treat reply from sent specially, do we check for that folder flag here ?
      var hintForIdentity = (type == msgComposeType.Template) ? hdr.author : hdr.recipients + hdr.ccList;
      var accountKey = hdr.accountKey;
      if (accountKey.length > 0)
      {
        var account = accountManager.getAccount(accountKey);
        if (account)
          server = account.incomingServer;
      }

      if (server)
        identity = getIdentityForServer(server, hintForIdentity);

      if (folder && (!identity || hintForIdentity.search(identity.email) < 0))
      {
        server = folder.server;
        if (server)
        {
          var tmpIdentity = getIdentityForServer(server, hintForIdentity);
          if (tmpIdentity && hintForIdentity.search(tmpIdentity.email) >= 0)
            identity = tmpIdentity;
        }
      }

      if (type == msgComposeType.Reply ||
        type == msgComposeType.ReplyAll ||
        type == msgComposeType.ReplyToList ||
        type == msgComposeType.ForwardInline ||
        type == msgComposeType.ReplyToGroup ||
        type == msgComposeType.ReplyToSender ||
        type == msgComposeType.ReplyToSenderAndGroup ||
        type == msgComposeType.Template ||
        type == msgComposeType.Draft)
      {
        msgComposeService.OpenComposeWindow(null, messageUri, type, format, identity, msgWindow);
        // Limit the number of new compose windows to 8. Why 8 ? I like that number :-)
        if (i == 7)
          break;
      }
      else
      {
        if (i)
          uri += ","
        uri += messageUri;
      }
    }
    if (type == msgComposeType.ForwardAsAttachment)
      msgComposeService.OpenComposeWindow(null, uri, type, format, identity, msgWindow);
  }
  else
    dump("### nodeList is invalid\n");
}

function NewMessageToSelectedAddresses(type, format, identity) {
  var abSidebarPanel = document.commandDispatcher.focusedWindow;
  var abResultsTree = abSidebarPanel.document.getElementById("abResultsTree");
  var abResultsBoxObject = abResultsTree.treeBoxObject;
  var abView = abResultsBoxObject.view;
  abView = abView.QueryInterface(Components.interfaces.nsIAbView);
  var addresses = abView.selectedAddresses;
  var params = Components.classes["@mozilla.org/messengercompose/composeparams;1"].createInstance(Components.interfaces.nsIMsgComposeParams);
  if (params) {
    params.type = type;
    params.format = format;
    params.identity = identity;
    var composeFields = Components.classes["@mozilla.org/messengercompose/composefields;1"].createInstance(Components.interfaces.nsIMsgCompFields);
    if (composeFields) {
      var addressList = "";
      for (var i = 0; i < addresses.Count(); i++) {
        addressList = addressList + (i > 0 ? ",":"") + addresses.QueryElementAt(i,Components.interfaces.nsISupportsString).data;
      }
      composeFields.to = addressList;
      params.composeFields = composeFields;
      msgComposeService.OpenComposeWindowWithParams(null, params);
    }
  }
}

function CreateNewSubfolder(chromeWindowURL, preselectedMsgFolder,
                            dualUseFolders, callBackFunctionName)
{
  var preselectedURI;

  if (preselectedMsgFolder)
  {
    var preselectedFolderResource = preselectedMsgFolder.QueryInterface(Components.interfaces.nsIRDFResource);
    if (preselectedFolderResource)
      preselectedURI = preselectedFolderResource.Value;
    dump("preselectedURI = " + preselectedURI + "\n");
  }

  window.openDialog(chromeWindowURL,
                    "",
                    "chrome,titlebar,modal",
                    {preselectedURI:preselectedURI,
                      dualUseFolders:dualUseFolders,
                      okCallback:callBackFunctionName});
}

function NewFolder(name,uri)
{
  // dump("uri,name = " + uri + "," + name + "\n");
  if (uri && (uri != "") && name && (name != "")) {
    var selectedFolderResource = RDF.GetResource(uri);
    // dump("selectedFolder = " + uri + "\n");
    var compositeDataSource = GetCompositeDataSource("NewFolder");
    var folderArray = Components.classes["@mozilla.org/supports-array;1"].createInstance(Components.interfaces.nsISupportsArray);
    var nameArray = Components.classes["@mozilla.org/supports-array;1"].createInstance(Components.interfaces.nsISupportsArray);

    folderArray.AppendElement(selectedFolderResource);

    var nameLiteral = RDF.GetLiteral(name);
    nameArray.AppendElement(nameLiteral);
    DoRDFCommand(compositeDataSource, "http://home.netscape.com/NC-rdf#NewFolder", folderArray, nameArray);
  }
  else {
    dump("no name or nothing selected\n");
  }
}

function UnSubscribe(folder)
{
  // Unsubscribe the current folder from the newsserver, this assumes any confirmation has already
  // been made by the user  SPL

  var server = folder.server;
  var subscribableServer = server.QueryInterface(Components.interfaces.nsISubscribableServer);
  subscribableServer.unsubscribe(folder.name);
  subscribableServer.commitSubscribeChanges();
}

function Subscribe(preselectedMsgFolder)
{
  var preselectedURI;

  if (preselectedMsgFolder)
  {
    var preselectedFolderResource = preselectedMsgFolder.QueryInterface(Components.interfaces.nsIRDFResource);
    if (preselectedFolderResource)
      preselectedURI = preselectedFolderResource.Value;
    dump("preselectedURI = " + preselectedURI + "\n");
  }

  window.openDialog("chrome://messenger/content/subscribe.xul",
                    "subscribe", "chrome,modal,titlebar,resizable=yes",
                    {preselectedURI:preselectedURI,
                      okCallback:SubscribeOKCallback});
}

function SubscribeOKCallback(changeTable)
{
  for (var serverURI in changeTable) {
    var folder = GetMsgFolderFromUri(serverURI, true);
    var server = folder.server;
    var subscribableServer =
          server.QueryInterface(Components.interfaces.nsISubscribableServer);

    for (var name in changeTable[serverURI]) {
      if (changeTable[serverURI][name] == true) {
        try {
          subscribableServer.subscribe(name);
        }
        catch (ex) {
          dump("failed to subscribe to " + name + ": " + ex + "\n");
        }
      }
      else if (changeTable[serverURI][name] == false) {
        try {
          subscribableServer.unsubscribe(name);
        }
        catch (ex) {
          dump("failed to unsubscribe to " + name + ": " + ex + "\n");
        }
      }
      else {
        // no change
      }
    }

    try {
      subscribableServer.commitSubscribeChanges();
    }
    catch (ex) {
      dump("failed to commit the changes: " + ex + "\n");
    }
  }
}

function SaveAsFile(uri)
{
  if (uri) {
    var filename = null;
    try {
      var subject = messenger.msgHdrFromURI(uri).mime2DecodedSubject;
      filename = GenerateValidFilename(subject, ".eml");
    }
    catch (ex) {}
    messenger.saveAs(uri, true, null, filename);
  }
}

function SaveAsTemplate(uri, folder)
{
  if (uri) {
    var identity = getIdentityForServer(folder.server);
    messenger.saveAs(uri, false, identity, null);
  }
}

function MarkSelectedMessagesRead(markRead)
{
  gDBView.doCommand(markRead ? nsMsgViewCommandType.markMessagesRead : nsMsgViewCommandType.markMessagesUnread);
}

function MarkSelectedMessagesFlagged(markFlagged)
{
  gDBView.doCommand(markFlagged ? nsMsgViewCommandType.flagMessages : nsMsgViewCommandType.unflagMessages);
}

function MarkAllMessagesRead(compositeDataSource, folder)
{
  var folderResource = folder.QueryInterface(Components.interfaces.nsIRDFResource);
  var folderArray = Components.classes["@mozilla.org/supports-array;1"].createInstance(Components.interfaces.nsISupportsArray);
  folderArray.AppendElement(folderResource);

  DoRDFCommand(compositeDataSource, "http://home.netscape.com/NC-rdf#MarkAllMessagesRead", folderArray, null);
}

function DownloadFlaggedMessages(compositeDataSource, folder)
{
    dump("fix DownloadFlaggedMessages()\n");
}

function DownloadSelectedMessages(compositeDataSource, messages, markFlagged)
{
    dump("fix DownloadSelectedMessages()\n");
}

function ViewPageSource(messages)
{
  var numMessages = messages.length;

  if (numMessages == 0)
  {
    dump("MsgViewPageSource(): No messages selected.\n");
    return false;
  }

    try {
        // First, get the mail session
        const mailSessionContractID = "@mozilla.org/messenger/services/session;1";
        const nsIMsgMailSession = Components.interfaces.nsIMsgMailSession;
        var mailSession = Components.classes[mailSessionContractID].getService(nsIMsgMailSession);

        var mailCharacterSet = "charset=" + msgWindow.mailCharacterSet;

        for (var i = 0; i < numMessages; i++)
        {
            // Now, we need to get a URL from a URI
            var url = mailSession.ConvertMsgURIToMsgURL(messages[i], msgWindow);

            window.openDialog( "chrome://navigator/content/viewSource.xul",
                               "_blank",
                               "scrollbars,resizable,chrome,dialog=no",
                               url,
                               mailCharacterSet);
        }
        return true;
    } catch (e) {
        // Couldn't get mail session
        return false;
    }
}

function doHelpButton()
{
    openHelp("mail-offline-items");
}

// XXX The following junkmail code might be going away or changing

const NS_BAYESIANFILTER_CONTRACTID = "@mozilla.org/messenger/filter-plugin;1?name=bayesianfilter";
const nsIJunkMailPlugin = Components.interfaces.nsIJunkMailPlugin;
const nsIMsgDBHdr = Components.interfaces.nsIMsgDBHdr;

var gJunkmailComponent;

function determineActionsForJunkMsgs(aView, aIndices, aActionParams)
{

  // we use some arbitrary message to determine the
  // message server
  var msgURI = aView.getURIForViewIndex(aIndices[0]);
  var msgHdr = messenger.msgHdrFromURI(msgURI);
  var server = msgHdr.folder.server;

  var spamSettings = server.spamSettings;

  // note we will do moves/marking as read even if the spam
  // feature is disabled, since the user has asked to use it
  // despite the disabling

  // note also that we will only act on messages which
  // _the_current_run_ of the classifier has classified as
  // junk, rather than on all junk messages in the folder

  aActionParams.markRead = spamSettings.markAsReadOnSpam;
  aActionParams.junkTargetFolder = null;

  if (!spamSettings.moveOnSpam)
    return;

  // don't move if we are already in the junk folder
  if (msgHdr.folder.flags & MSG_FOLDER_FLAG_JUNK)
    return;

  var spamFolderURI = spamSettings.spamFolderURI;
  if (!spamFolderURI)
  {
    dump('no spam folder!');
    return;
  }

  aActionParams.junkTargetFolder = GetMsgFolderFromUri(spamFolderURI);
}

function performActionsOnJunkMsgs(aIndices)
{
  if (!aIndices.length)
    return;

  var treeView = gDBView.QueryInterface(Components.interfaces.nsITreeView);

  var treeSelection = treeView.selection;
  treeSelection.clearSelection();

  // select the messages
  for (i=0;i<aIndices.length;i++)
    treeSelection.rangedSelect(aIndices[i], aIndices[i], true /* augment */);

  var actionParams = { markRead: false, junkTargetFolder: null };
  determineActionsForJunkMsgs(treeView, aIndices, actionParams);
  if (actionParams.markRead)
    MarkSelectedMessagesRead(true);
  if (actionParams.junkTargetFolder)
  {
    SetNextMessageAfterDelete();
    gDBView.doCommandWithFolder(nsMsgViewCommandType.moveMessages, actionParams.junkTargetFolder);
  }

  treeSelection.clearSelection();
}

function getJunkmailComponent()
{
  if (!gJunkmailComponent) {
    gJunkmailComponent = Components.classes[NS_BAYESIANFILTER_CONTRACTID].getService(nsIJunkMailPlugin);
  }
}

function analyzeMessageForJunk(aMsgHdr, aMsgIndex, aJunkMsgIndices, aLastMessage, aWhiteListDirectory)
{
    var listener = {
        onMessageClassified: function(aClassifiedMsgURI, aClassification)
        {
            // XXX TODO
            // update status bar, or a progress dialog
            // running junk mail controls manually, on a large folder
            // can take a while, and the user doesn't know when we are done.

            // XXX TODO
            // make the cut off 50, like in nsMsgSearchTerm.cpp

            var score =
                aClassification == nsIJunkMailPlugin.JUNK ? "100" : "0";

            // set these props via the db (instead of the message header
            // directly) so that the nsMsgDBView knows to update the UI
            //
            var db = aMsgHdr.folder.getMsgDatabase(msgWindow);
            db.setStringProperty(aMsgHdr.messageKey, "junkscore", score);
            db.setStringProperty(aMsgHdr.messageKey, "junkscoreorigin",
                                 "plugin");

            if (aClassification == nsIJunkMailPlugin.JUNK)
              aJunkMsgIndices.push(aMsgIndex);

            if (aLastMessage)
              performActionsOnJunkMsgs(aJunkMsgIndices);
        }
    };

    // if we are whitelisting, check if the email address is in the whitelist addressbook.
    if (aWhiteListDirectory)
    {
      var headerParser = Components.classes["@mozilla.org/messenger/headerparser;1"].getService(Components.interfaces.nsIMsgHeaderParser);
      var authorEmailAddress = headerParser.extractHeaderAddressMailboxes(null, aMsgHdr.author);
      if (aWhiteListDirectory.hasCardForEmailAddress(authorEmailAddress))
        // skip over this message, like we do on incoming mail
        // the difference is it could be marked as junk from previous analysis
        // or from being manually marked by the user.
        return;
    }

    var messageURI = aMsgHdr.folder.generateMessageURI(aMsgHdr.messageKey)
        + "?fetchCompleteMessage=true";
    gJunkmailComponent.classifyMessage(messageURI, msgWindow, listener);
}

function filterFolderForJunk()
{
  MsgJunkMailInfo(true);
  var view = GetDBView();

  // need to expand all threads, so we analyze everything
  view.doCommand(nsMsgViewCommandType.expandAll);

  var treeView = view.QueryInterface(Components.interfaces.nsITreeView);
  var count = treeView.rowCount;
  if (!count)
    return;

  var junkMsgsArray = new Array;

  var tmpMsgURI = view.getURIForViewIndex(0);
  var tmpMsgHdr = messenger.msgHdrFromURI(tmpMsgURI);
  var spamSettings = tmpMsgHdr.folder.server.spamSettings;
  var whiteListDirectory = null;
  if (spamSettings.useWhiteList && spamSettings.whiteListAbURI)
    whiteListDirectory = RDF.GetResource(spamSettings.whiteListAbURI).QueryInterface(Components.interfaces.nsIAbMDBDirectory);

  getJunkmailComponent();

  for (var i = 0; i < count; i++) {
    var msgIndex = i;
    var msgURI = view.getURIForViewIndex(i);
    var msgHdr = messenger.msgHdrFromURI(msgURI);
    analyzeMessageForJunk(msgHdr,msgIndex,junkMsgsArray,(i == count-1),whiteListDirectory);
  }
}

function JunkSelectedMessages(setAsJunk)
{
  MsgJunkMailInfo(true);

  // When the user explicitly marks a message as junk, we can mark it as read,
  // too. This is independent of the "markAsReadOnSpam" pref, which applies
  // only to automatically-classified messages.
  // Note that this behaviour should match the one in the back end for marking
  // as junk via clicking the 'junk' column.

  if (setAsJunk && pref.getBoolPref("mailnews.ui.junk.manualMarkAsJunkMarksRead"))
    MarkSelectedMessagesRead(true);

  gDBView.doCommand(setAsJunk ? nsMsgViewCommandType.junk
                              : nsMsgViewCommandType.unjunk);
}

function deleteJunkInFolder()
{
  MsgJunkMailInfo(true);
  var view = GetDBView();

  // need to expand all threads, so we find everything
  view.doCommand(nsMsgViewCommandType.expandAll);

  var treeView = view.QueryInterface(Components.interfaces.nsITreeView);
  var count = treeView.rowCount;
  if (!count)
    return;

  var treeSelection = treeView.selection;

  var clearedSelection = false;

  // select the junk messages
  for (var i = 0; i < count; ++i)
  {
    var messageUri = view.getURIForViewIndex(i);
    var msgHdr = messenger.msgHdrFromURI(messageUri);
    var junkScore = msgHdr.getStringProperty("junkscore");
    var isJunk = ((junkScore != "") && (junkScore != "0"));
    // if the message is junk, select it.
    if (isJunk)
    {
      // only do this once
      if (!clearedSelection)
      {
        // clear the current selection
        // since we will be deleting all selected messages
        treeSelection.clearSelection();
        clearedSelection = true;
        treeSelection.selectEventsSuppressed = true;
      }
      treeSelection.rangedSelect(i, i, true /* augment */);
    }
  }

  // if we didn't clear the selection
  // there was no junk, so bail.
  if (!clearedSelection)
    return;

  treeSelection.selectEventsSuppressed = false;
  // delete the selected messages
  //
  // XXX todo
  // Should we try to set next message after delete
  // to the message selected before we did all this, if it was not junk?
  SetNextMessageAfterDelete();
  view.doCommand(nsMsgViewCommandType.deleteMsg);
  treeSelection.clearSelection();
}
