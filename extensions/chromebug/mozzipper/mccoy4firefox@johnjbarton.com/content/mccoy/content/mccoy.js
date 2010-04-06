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
 * The Original Code is McCoy.
 *
 * The Initial Developer of the Original Code is
 * the Mozilla Foundation <http://www.mozilla.org/>.
 * Portions created by the Initial Developer are Copyright (C) 2007
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Dave Townsend <dtownsend@oxymoronical.com>
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

const XULNS = "http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul";
const Cc = Components.classes;
const Ci = Components.interfaces;

const PREFIX_NS_EM      = "http://www.mozilla.org/2004/em-rdf#";
const PREFIX_ITEM_URI   = "urn:mozilla:item:";
const PREFIX_EXTENSION  = "urn:mozilla:extension:";
const PREFIX_THEME      = "urn:mozilla:theme:";
const HELP_URL          = "http://developer.mozilla.org/en/docs/McCoy";

var gKS = null;
var gList = null;
var gRDF = null;
var gRDFUtils = null;
var gStrings = null;

function EM_NS(prop)
{
  return gRDF.GetResource(PREFIX_NS_EM + prop);
}

var mainController = {
  supportsCommand: function(cmd)
  {
    switch (cmd) {
      case "cmd_changepassword":
      case "cmd_createkey":
      case "cmd_renamekey":
      case "cmd_deletekey":
      case "cmd_copypublic":
      case "cmd_signupdate":
      case "cmd_addtoinstall":
      case "cmd_verifykey":
      case "cmd_verifyinstall":
      case "cmd_about":
      case "cmd_help":
        return true;
    }
    return false;
  },
  
  isCommandEnabled: function(cmd)
  {
    switch (cmd) {
      case "cmd_changepassword":
      case "cmd_createkey":
      case "cmd_verifyinstall":
      case "cmd_about":
      case "cmd_help":
        return true;
      case "cmd_renamekey":
      case "cmd_deletekey":
      case "cmd_copypublic":
      case "cmd_signupdate":
      case "cmd_addtoinstall":
      case "cmd_verifykey":
        return gList.selectedIndex>=0;
    }
    return false;
  },
  
  doCommand: function(cmd)
  {
    if (this.isCommandEnabled(cmd)) {
      switch (cmd) {
        case "cmd_changepassword":
          gKS.changePassword();
          break;
        case "cmd_createkey":
          var promptSvc = Cc["@mozilla.org/embedcomp/prompt-service;1"].
                          getService(Ci.nsIPromptService);
          var name = { value: "" };
          var title = gStrings.getString("createkey.title");
          var text = gStrings.getString("createkey.text");
          if (promptSvc.prompt(window, title, text, name, null, {})) {
            if (name.value) {
              var key = gKS.createKeyPair(Ci.nsIKeyPair.KEYTYPE_RSA);
              key.name = name.value;
              var item = createItem(key);
              insertItem(item);
              gList.selectedItem = item;
            }
            else {
              alert(gStrings.getString("createkey.noname"));
            }
          }
          break;
        case "cmd_renamekey":
          var promptSvc = Cc["@mozilla.org/embedcomp/prompt-service;1"].
                          getService(Ci.nsIPromptService);
          var name = { value: gList.selectedItem.key.name };
          var title = gStrings.getString("renamekey.title");
          var text = gStrings.getString("renamekey.text");
          if (promptSvc.prompt(window, title, text, name, null, {})) {
            var item = gList.selectedItem;
            item.key.name=name.value;
            item.setAttribute("label", name.value);
            insertItem(item);
            gList.selectedItem = item;
          }
          break;
        case "cmd_deletekey":
          var promptSvc = Cc["@mozilla.org/embedcomp/prompt-service;1"].
                          getService(Ci.nsIPromptService);
          var title = gStrings.getString("deletekey.title");
          var text = gStrings.getString("deletekey.text");
          if (promptSvc.confirm(window, title, text)) {
            gList.selectedItem.key.delete();
            gList.removeChild(gList.selectedItem);
          }
          break;
        case "cmd_copypublic":
          var clipboard = Cc["@mozilla.org/widget/clipboardhelper;1"].
                          getService(Ci.nsIClipboardHelper);
          clipboard.copyString(gList.selectedItem.key.exportPublicKey());
          break;
        case "cmd_signupdate":
          var rdf = loadRDF("Select Update Manifest", "update.rdf");
          if (rdf) {
            signUpdate(rdf, gList.selectedItem.key);
            rdf.Flush();
            gRDF.UnregisterDataSource(rdf);
          }
          break;
        case "cmd_addtoinstall":
          var rdf = loadRDF("Select Install Manifest", "install.rdf");
          if (rdf) {
            var mftRes = gRDF.GetResource("urn:mozilla:install-manifest");
            var keyArc = EM_NS("updateKey");
            var key = gRDF.GetLiteral(gList.selectedItem.key.exportPublicKey());
            unassertAll(rdf, mftRes, keyArc);
            rdf.Assert(mftRes, keyArc, key, true);
            rdf.Flush();
            gRDF.UnregisterDataSource(rdf);
          }
          break;
        case "cmd_verifykey":
          var rdf = loadRDF("Select Update Manifest", "update.rdf");
          if (rdf) {
            verifyUpdateManifest(rdf, gList.selectedItem.key);
            gRDF.UnregisterDataSource(rdf);
          }
          break;
        case "cmd_verifyinstall":
          var install = loadRDF("Select Install Manifest", "install.rdf");
          if (install) {
            var update = loadRDF("Select Update Manifest", "update.rdf");
            if (update) {
              verufyFullUpdate(install, update);
              gRDF.UnregisterDataSource(update);
            }
            gRDF.UnregisterDataSource(install);
          }
          break;
        case "cmd_about":
          var wm = Cc["@mozilla.org/appshell/window-mediator;1"].
                   getService(Ci.nsIWindowMediator);
          var win = wm.getMostRecentWindow("McCoy:About");
          if (win)
            win.focus();
          else
            window.openDialog("chrome://mccoy/content/about.xul", "",
                              "chrome,dialog,centerscreen");
          break;
        case "cmd_help":
          openURL(HELP_URL);
          break;
      }
    }
  },
  
  /**
   * Updates all of the commands in the commandset
   */
  onCommandUpdate: function()
  {
    var commands = document.getElementById("mainCommands")
                           .getElementsByTagName("command");
    for (var i = 0; i < commands.length; i++)
      goSetCommandEnabled(commands[i].id, this.isCommandEnabled(commands[i].id));
  },
  
  onEvent: function(evt) { }
}

/**
 * Creates a richlistboxitem for the key.
 * @param key an nsIKeyPair
 * @returns a richlistitem element
 */
function createItem(key)
{
  var item = document.createElementNS(XULNS, "richlistitem");
  item.setAttribute("label", key.name);
  switch (key.type)
  {
    case Ci.nsIKeyPair.KEYTYPE_RSA:
      item.setAttribute("type", "RSA");
      break;
    case Ci.nsIKeyPair.KEYTYPE_DSA:
      item.setAttribute("type", "DSA");
      break;
  }
  item.setAttribute("context", "menu-keycontext");
  item.key = key;
  return item;
}

/**
 * Inserts a key richlistitem into the list in the appropriate place
 * @param item the richlistitem to be inserted
 */
function insertItem(item)
{
  var name = item.key.name.toLowerCase();
  var items = gList.children;

  for (var i = 0; i < items.length; i++) {
    // Might be moving an item, skip over ourself
    if (items[i] == item)
      continue;
    
    // This matches the insertion point
    if (items[i].key.name.toLowerCase() > name) {
      gList.insertBefore(item, items[i]);
      return;
    }
  }
  // Nothing later found, put at the end
  gList.appendChild(item);
}

/**
 * Loads and rdf file into a datasource
 * @param title the title for the file picker dialog.
 * @param filename the default filename for hte file picker dialog
 * @returns an nsIRDFDataSource for the seelcted file or null if cancelled.
 */
function loadRDF(title, filename)
{
  var fp = Cc["@mozilla.org/filepicker;1"].
           createInstance(Ci.nsIFilePicker);
  fp.init(window, title, Ci.nsIFilePicker.modeOpen);
  fp.appendFilter("RDF Files", "*.rdf");
  fp.appendFilters(Ci.nsIFilePicker.filterAll);
  fp.filterIndex = 0;
  fp.defaultString = filename;
  if (fp.show() == Ci.nsIFilePicker.returnOK)
    return gRDF.GetDataSourceBlocking(fp.fileURL.spec)
               .QueryInterface(Ci.nsIRDFRemoteDataSource);
  return null;
}

/**
 * Removes any assertions with a given source and property.
 * @param rdf the nsIRDFDataSource
 * @param source the nsIRDFResource source of the assertion
 * @param property the nsIRDFResource property of the assertion
 */
function unassertAll(rdf, source, property)
{
  var targets = rdf.GetTargets(source, property, true);
  while (targets.hasMoreElements()) {
    var target = targets.getNext().QueryInterface(Ci.nsIRDFNode);
    rdf.Unassert(source, property, target, true);
  }
}

/**
 * Gets an rdf assertion target as a string
 * @param rdf the nsIRDFDataSource
 * @param source the nsIRDFResource source of the assertion
 * @param property the nsIRDFResource property of the assertion
 * @returns a string if the assertion exists and pointed at an nsIRDFLiteral,
 *          null otherwise.
 */
function getString(rdf, source, property)
{
  var prop = rdf.GetTarget(source, EM_NS(property), true);
  if (prop && prop instanceof Ci.nsIRDFLiteral)
    return prop.Value;
  return null;
}

/**
 * Gets an rdf assertion target as an integer
 * @param rdf the nsIRDFDataSource
 * @param source the nsIRDFResource source of the assertion
 * @param property the nsIRDFResource property of the assertion
 * @returns a number if the assertion exists and pointed at an nsIRDFInt,
 *          or a parsable nsIRDFLiteral, null otherwise.
 */
function getInt(rdf, source, property)
{
  var prop = rdf.GetTarget(source, EM_NS(property), true);
  if (prop) {
    if (prop instanceof Ci.nsIRDFInt)
      return prop.Value;
    if (prop instanceof Ci.nsIRDFLiteral)
      return parseInt(prop.Value);
  }
  return null;
}

/**
 * Gets the resource used in an update manifest for an add-on.
 * @param id the id of the add-on
 * @param type the type of the add-on
 * @returns an nsIRDFResource
 */
function getUpdateResource(id, type)
{
  switch (type) {
  case Ci.nsIUpdateItem.TYPE_EXTENSION:
    return gRDF.GetResource(PREFIX_EXTENSION + id);
  case Ci.nsIUpdateItem.TYPE_THEME:
    return gRDF.GetResource(PREFIX_THEME + id);
  default:
    return gRDF.GetResource(PREFIX_ITEM_URI + id);
  }
}

/**
 * Selects which add-ons from a given update manifest the user
 * wishes to sign. Currently returns all found add-ons.
 * TODO throw up a list box allowing user to select ids
 * @param rdf an nsIRDFDataSource for the update.rdf
 * @returns an array of nsIRDFResources to be signed
 */
function selectAddons(rdf)
{
  var addons = [];
  var resources = rdf.GetAllResources();
  while (resources.hasMoreElements()) {
    var res = resources.getNext().QueryInterface(Ci.nsIRDFResource);
    if (res.Value.substring(0, PREFIX_ITEM_URI.length) == PREFIX_ITEM_URI)
      addons.push(res);
    if (res.Value.substring(0, PREFIX_EXTENSION.length) == PREFIX_EXTENSION)
      addons.push(res);
    if (res.Value.substring(0, PREFIX_THEME.length) == PREFIX_THEME)
      addons.push(res);
  }
  return addons;
}

/**
 * Generates the string to sign from an update manifest.
 * @param rdf an nsIRDFDataSource for the update.rdf
 * @param resource an nsIRDFResource for the extension to be signed.
 */
function getUpdateData(rdf, resource)
{
  var serializer = new RDFSerializer();
  return serializer.serializeResource(rdf, resource, "");
}

/**
 * Signs the update manifest given with the key provided.
 * @param rdf an nsIRDFDataSource for the update.rdf
 * @param key an nsIKeyPair to use for testing
 */
function signUpdate(rdf, key)
{
  var addons = selectAddons(rdf);
  for (var i = 0; i < addons.length; i++) {
    var resource = addons[i];
    var data = getUpdateData(rdf, resource);
    var signature = gRDF.GetLiteral(key.signData(data, Ci.nsIKeyPair.HASHTYPE_SHA512));
    unassertAll(rdf, resource, EM_NS("signature"));
    rdf.Assert(resource, EM_NS("signature"), signature, true);
  }
}

/**
 * Verifies that the update manifest given was signed by the key provided.
 * @param rdf an nsIRDFDataSource for the update.rdf
 * @param key an nsIKeyPair to use for testing
 */
function verifyUpdateManifest(rdf, key)
{
  var addons = selectAddons(rdf);
  var pass = true;
  for (var i = 0; i < addons.length; i++) {
    var resource = addons[i];
    var sig = getString(rdf, resource, "signature");
    if (sig) {
      var data = getUpdateData(rdf, resource);
      if (!key.verifyData(data, sig)) {
        alert(gStrings.getFormattedString("verification.badsignature", [addons[i].Value]));
        pass = false;
      }
    }
    else {
      alert(gStrings.getFormattedString("verification.nosignature", [addons[i].Value]));
      pass = false;
    }
  }
  if (pass)
    alert(gStrings.getString("verification.passed"));
  else
    alert(gStrings.getString("verification.failed"));
}

/**
 * Verifies that the update manifest is signed correctly for the given install
 * manifest.
 * TODO let this pull the install manifest out of an xpi
 * @param install the install manifest of the add-on
 * @param update the update manifest to test
 */
function verifyFullUpdate(install, update)
{
  verifyFullUpdate(install, update);
  var mftRes = gRDF.GetResource("urn:mozilla:install-manifest");
  var id = getString(install, mftRes, "id");
  var key = getString(install, mftRes, "updateKey");
  if (id && key) {
    // Finds the type of add-on, defaults to extension
    var type = getInt(install, mftRes, "type");
    if (!type)
      type = Ci.nsIUpdateItem.TYPE_EXTENSION;

    var resource = getUpdateResource(id, type);
    var sig = getString(update, resource, "signature");
    if (sig) {
      // Verify the data
      var data = getUpdateData(update, resource);
      var dsv = Cc["@mozilla.org/security/datasignatureverifier;1"].
                getService(Ci.nsIDataSignatureVerifier);
      if (dsv.verifyData(data, sig, key))
        alert(gStrings.getString("fullverify.goodsignature"));
      else
        alert(gStrings.getString("fullverify.badsignature"));
    }
    else {
      alert(gStrings.getString("fullverify.nosignature"));
    }
  }
  else {
    alert(gStrings.getString("fullverify.invalidrdf"));
  }
}

/**
 * Refreshes the list of keys in the listbox.
 */
function readKeys()
{
  // Clear out any exiting list
  while (gList.lastChild)
    gList.removeChild(gList.lastChild);

  var keyObjects = {};
  var keyNames = [];
  
  // Update the key hashes
  var keys = gKS.enumerateKeys();
  while (keys.hasMoreElements()) {
    var key = keys.getNext().QueryInterface(Ci.nsIKeyPair);
    var name = key.name.toLowerCase();
    keyObjects[name] = key;
    keyNames.push(name);
  }
  keyNames.sort();
  
  // Generate the list
  for (var i = 0; i < keyNames.length; i++) {
    key = keyObjects[keyNames[i]];
    var item = createItem(key);
    gList.appendChild(item);
  }
}

/**
 * This adjusts the default protocol handler actions to just open with the
 * system default app
 */
function fixProtocolHandlers()
{
  var hs = Cc["@mozilla.org/uriloader/handler-service;1"].
           getService(Ci.nsIHandlerService);
  var extps = Cc["@mozilla.org/uriloader/external-protocol-service;1"].
              getService(Ci.nsIExternalProtocolService);

  var httpHandler = extps.getProtocolHandlerInfo("http");
  httpHandler.preferredAction = Ci.nsIHandlerInfo.useSystemDefault;
  httpHandler.alwaysAskBeforeHandling = false;
  hs.store(httpHandler);
}

/**
 * Initialise the window
 */
function startup()
{
  fixProtocolHandlers();

  try {
    gKS = Cc["@toolkit.mozilla.org/keyservice;1"].
          getService(Ci.nsIKeyService);
  }
  catch (e) {
    // Chances are the user cancelled the password dialog
  }
  
  // Set up some globals
  gRDF = Cc["@mozilla.org/rdf/rdf-service;1"].
         getService(Ci.nsIRDFService);
  gRDFUtils = Cc["@mozilla.org/rdf/container-utils;1"].
              getService(Ci.nsIRDFContainerUtils);
  gList = document.getElementById("keylist");
  gStrings = document.getElementById("strings");
  
  window.controllers.appendController(mainController);
  mainController.onCommandUpdate();

  // Build the initial key list
  readKeys();
}
