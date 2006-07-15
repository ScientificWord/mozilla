# -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
# ***** BEGIN LICENSE BLOCK *****
# Version: MPL 1.1/GPL 2.0/LGPL 2.1
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Original Code is Mozilla.org Code.
#
# The Initial Developer of the Original Code is
# Doron Rosenberg.
# Portions created by the Initial Developer are Copyright (C) 2001
# the Initial Developer. All Rights Reserved.
#
# Contributor(s):
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 2 or later (the "GPL"), or
# the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
# in which case the provisions of the GPL or the LGPL are applicable instead
# of those above. If you wish to allow use of your version of this file only
# under the terms of either the GPL or the LGPL, and not to allow others to
# use your version of this file under the terms of the MPL, indicate your
# decision by deleting the provisions above and replace them with the notice
# and other provisions required by the GPL or the LGPL. If you do not delete
# the provisions above, a recipient may use your version of this file under
# the terms of any one of the MPL, the GPL or the LGPL.
#
# ***** END LICENSE BLOCK *****

/** 
 * Datasource initialization
 **/

var gRDF = Components.classes["@mozilla.org/rdf/rdf-service;1"]
                     .getService(Components.interfaces.nsIRDFService);

/**
 * Handler Override class
 **/
function HandlerOverride(aURI)
{
  this.URI = aURI;
  this.mUpdateMode = false;
}

HandlerOverride.prototype = {
  // general information
  get mimeType()
  {
    return getLiteralForContentType(this.URI, "value");
  },
  
  set mimeType(aMIMETypeString)
  {
    if (!this.mUpdateMode)
      assertMIMEStuff(MIME_URI(aMIMETypeString), "value", aMIMETypeString.toLowerCase());
    else
      changeMIMEStuff(MIME_URI(aMIMETypeString), "value", aMIMETypeString.toLowerCase());
  },
  
  get description()
  {
    return getLiteralForContentType(this.URI, "description");
  },  
  
  set description(aDescriptionString)
  {
    if (!this.mUpdateMode)
      assertMIMEStuff(MIME_URI(this.mimeType), "description", aDescriptionString);
    else
      changeMIMEStuff(MIME_URI(this.mimeType), "description", aDescriptionString);
  },
  
  get isEditable()
  {
    return getLiteralForContentType(this.URI, "editable");
  },
  
  set isEditable(aIsEditableString)
  {
    if (!this.mUpdateMode)
      assertMIMEStuff(MIME_URI(this.mimeType), "editable", aIsEditableString);
    else
      changeMIMEStuff(MIME_URI(this.mimeType), "editable", aIsEditableString);
  },

  get extensions()
  {
    var extensionResource = gRDF.GetResource(NC_RDF("fileExtensions"));
    var contentTypeResource = gRDF.GetResource(MIME_URI(this.mimeType));
    var extensionTargets  = gDS.GetTargets(contentTypeResource, extensionResource, true);
    var extString = "";
    if (extensionTargets) {
      extensionTargets = extensionTargets.QueryInterface(Components.interfaces.nsISimpleEnumerator);
      while (extensionTargets.hasMoreElements()) {
        var currentExtension = extensionTargets.getNext();
        if (currentExtension) {
          currentExtension = currentExtension.QueryInterface(Components.interfaces.nsIRDFLiteral);
          extString += currentExtension.Value.toLowerCase() + " ";
        }
      }
    }
    return extString;
  },
  
  addExtension: function (aExtensionString)
  {
    assertMIMEStuff(MIME_URI(this.mimeType), "fileExtensions", aExtensionString.toLowerCase());
  },
  
  removeExtension: function (aExtensionString)
  {
    unassertMIMEStuff(MIME_URI(this.mimeType), "fileExtensions", aExtensionString.toLowerCase());
  },
  
  // content handling
  get saveToDisk()
  {
    return getHandlerInfoForType(this.URI, "saveToDisk");
  },
  
  set saveToDisk(aSavedToDisk)
  {
    var handlerSource = gRDF.GetResource(HANDLER_URI(this.mimeType));
    var handlerProperty = gRDF.GetResource(NC_RDF("saveToDisk"));
    var trueLiteral = gRDF.GetLiteral("true");
    var hasSaveToDisk = gDS.HasAssertion(handlerSource, handlerProperty, trueLiteral, true);
    if (!hasSaveToDisk) {
      var falseLiteral = gRDF.GetLiteral("false");
      hasSaveToDisk = gDS.HasAssertion(handlerSource, handlerProperty, falseLiteral, true);
    }
    if (!this.mUpdateMode || !hasSaveToDisk)
      assertMIMEStuff(HANDLER_URI(this.mimeType), "saveToDisk", aSavedToDisk);
    else
      changeMIMEStuff(HANDLER_URI(this.mimeType), "saveToDisk", aSavedToDisk);
    this.setHandlerProcedure("handleInternal", "false");
 },
  
  get handleInternal()
  {
    return getHandlerInfoForType(this.URI, "handleInternal");
  },
  
  set handleInternal(aHandledInternally)
  {
    var handlerSource = gRDF.GetResource(HANDLER_URI(this.mimeType));
    var handlerProperty = gRDF.GetResource(NC_RDF("handleInternal"));
    var trueLiteral = gRDF.GetLiteral("true");
    var hasHandleInternal = gDS.HasAssertion(handlerSource, handlerProperty, trueLiteral, true);
    if (!hasHandleInternal) {
      var falseLiteral = gRDF.GetLiteral("false");
      hasHandleInternal = gDS.HasAssertion(handlerSource, handlerProperty, falseLiteral, true);
    }
    if (!this.mUpdateMode || !hasHandleInternal)
      assertMIMEStuff(HANDLER_URI(this.mimeType), "handleInternal", aHandledInternally);
    else
      changeMIMEStuff(HANDLER_URI(this.mimeType), "handleInternal", aHandledInternally);
    this.setHandlerProcedure("saveToDisk", "false");
  },

  setHandlerProcedure: function (aHandlerProcedure, aValue)
  {
    var handlerSource = gRDF.GetResource(HANDLER_URI(this.mimeType));
    var handlerProperty = gRDF.GetResource(NC_RDF(aHandlerProcedure));
    var oppositeValue = aValue == "false" ? "true" : "false";
    var trueLiteral = gRDF.GetLiteral(oppositeValue);
    var hasCounterpart = gDS.HasAssertion(handlerSource, handlerProperty, trueLiteral, true);
    if (hasCounterpart) {
      var falseLiteral = gRDF.GetLiteral(aValue);
      gDS.Change(handlerSource, handlerProperty, trueLiteral, falseLiteral);
    }
  },
  
  get alwaysAsk()
  {
    return getHandlerInfoForType(this.URI, "alwaysAsk");
  },
  
  set alwaysAsk(aAlwaysAsk)
  {
    if (!this.mUpdateMode)
      assertMIMEStuff(HANDLER_URI(this.mimeType), "alwaysAsk", aAlwaysAsk);
    else
      changeMIMEStuff(HANDLER_URI(this.mimeType), "alwaysAsk", aAlwaysAsk);
  },
  
  // helper application
  get appDisplayName()
  {
    return getHelperAppInfoForType(this.URI, "prettyName");
  },
  
  set appDisplayName(aDisplayName)
  {
    if (!this.mUpdateMode)
      assertMIMEStuff(APP_URI(this.mimeType), "prettyName", aDisplayName);
    else
      changeMIMEStuff(APP_URI(this.mimeType), "prettyName", aDisplayName);
  },
  
  get appPath()
  {
    return getHelperAppInfoForType(this.URI, "path");
  },
  
  set appPath(aAppPath)
  {
    if (!this.mUpdateMode)
      assertMIMEStuff(APP_URI(this.mimeType), "path", aAppPath);
    else
      changeMIMEStuff(APP_URI(this.mimeType), "path", aAppPath);
  },

  /**
   * After setting the various properties on this override, we need to
   * build the links between the mime type resource, the handler for that
   * resource, and the helper app (if any) associated with the resource.
   * We also need to add this mime type to the RDF seq (list) of types.
   **/
  buildLinks: function()
  {
    // assert the handler resource
    var mimeSource = gRDF.GetResource(MIME_URI(this.mimeType));
    var handlerProperty = gRDF.GetResource(NC_RDF("handlerProp"));
    var handlerResource = gRDF.GetResource(HANDLER_URI(this.mimeType));
    gDS.Assert(mimeSource, handlerProperty, handlerResource, true);
    // assert the helper app resource
    var helperAppProperty = gRDF.GetResource(NC_RDF("externalApplication"));
    var helperAppResource = gRDF.GetResource(APP_URI(this.mimeType));
    gDS.Assert(handlerResource, helperAppProperty, helperAppResource, true);
    // add the mime type to the MIME types seq
    var container = Components.classes["@mozilla.org/rdf/container;1"].createInstance();
    if (container) {
      container = container.QueryInterface(Components.interfaces.nsIRDFContainer);
      if (container) {
        var containerRes = gRDF.GetResource("urn:mimetypes:root");
        container.Init(gDS, containerRes);
        var element = gRDF.GetResource(MIME_URI(this.mimeType));
        if (container.IndexOf(element) == -1)
          container.AppendElement(element);
      }
    }
  }
 
};

/** 
 * Utility functions for building URIs easily
 **/
function NC_RDF(aProperty)
{
  return "http://home.netscape.com/NC-rdf#" + aProperty;
}

function HANDLER_URI(aHandler)
{
  return "urn:mimetype:handler:" + aHandler;
}

function APP_URI(aType)
{
  return "urn:mimetype:externalApplication:" + aType;
}

function MIME_URI(aType)
{
  return "urn:mimetype:" + aType;
}

/**
 * Utility functions for reading data from the RDF datasource
  **/
function getLiteralForContentType(aURI, aProperty)
{
  var contentTypeResource = gRDF.GetResource(aURI);
  var propertyResource = gRDF.GetResource(NC_RDF(aProperty));
  return getLiteral(contentTypeResource, propertyResource);
}

function getLiteral(aSource, aProperty)
{
  var node = gDS.GetTarget(aSource, aProperty, true);
  if (node) {
    node = node.QueryInterface(Components.interfaces.nsIRDFLiteral);
    return node.Value;
  }
  return "";
}

function getHandlerInfoForType(aURI, aPropertyString)
{
  // get current selected type
  var handler = HANDLER_URI(getLiteralForContentType(aURI, "value"));
  var source = gRDF.GetResource(handler);
  var property = gRDF.GetResource(NC_RDF(aPropertyString));
  var target = gDS.GetTarget(source, property, true);
  if (target) {
    target = target.QueryInterface(Components.interfaces.nsIRDFLiteral);
    return target.Value;
  }
  return "";
}

function getHelperAppInfoForType(aURI, aPropertyString)
{
  var appURI      = APP_URI(getLiteralForContentType(aURI, "value"));
  var appRes      = gRDF.GetResource(appURI);
  var appProperty = gRDF.GetResource(NC_RDF(aPropertyString));
  return getLiteral(appRes, appProperty);
}

function mimeHandlerExists(aMIMEType)
{
  var valueProperty = gRDF.GetResource(NC_RDF("value"));
  var mimeSource = gRDF.GetResource(MIME_URI(aMIMEType));
  var mimeLiteral = gRDF.GetLiteral(gMIMEField.value);
  return gDS.HasAssertion(mimeSource, valueProperty, mimeLiteral, true);
}

// write to the ds
function assertMIMEStuff(aMIMEString, aPropertyString, aValueString)
{
  var mimeSource = gRDF.GetResource(aMIMEString);
  var valueProperty = gRDF.GetResource(NC_RDF(aPropertyString));
  var mimeLiteral = gRDF.GetLiteral(aValueString);
  gDS.Assert(mimeSource, valueProperty, mimeLiteral, true);
}

function changeMIMEStuff(aMIMEString, aPropertyString, aValueString)
{
  var mimeSource = gRDF.GetResource(aMIMEString);
  var valueProperty = gRDF.GetResource(NC_RDF(aPropertyString));
  var mimeLiteral = gRDF.GetLiteral(aValueString);
  var currentValue = gDS.GetTarget(mimeSource, valueProperty, true);
  gDS.Change(mimeSource, valueProperty, currentValue, mimeLiteral);
}

function unassertMIMEStuff(aMIMEString, aPropertyString, aValueString)
{
  var mimeSource = gRDF.GetResource(aMIMEString);
  var valueProperty = gRDF.GetResource(NC_RDF(aPropertyString));
  var mimeLiteral = gRDF.GetLiteral(aValueString);
  gDS.Unassert(mimeSource, valueProperty, mimeLiteral, true);
}

function removeOverride(aMIMEType)
{
  dump("*** mimeType = " + aMIMEType + "\n");
  // remove entry from seq
  var rdfc = Components.classes["@mozilla.org/rdf/container;1"].createInstance();
  if (rdfc) {
    rdfc = rdfc.QueryInterface(Components.interfaces.nsIRDFContainer);
    if (rdfc) {
      var containerRes = gRDF.GetResource("urn:mimetypes:root");
      rdfc.Init(gDS, containerRes);
      var element = gRDF.GetResource(MIME_URI(aMIMEType));
      if (rdfc.IndexOf(element) != -1) {
        try {
          rdfc.RemoveElement(element, true);
        }
        catch(e) {
          // suppress (benign?) errors
        } 
      }
    }
  }
  
  // remove items from the graph  
  var urns = [ [MIME_URI, ["description", "editable", "value", "fileExtensions", "smallIcon", "largeIcon"], 
                          [HANDLER_URI, "handlerProp"]],               
               [HANDLER_URI, ["handleInternal", "saveToDisk", "alwaysAsk"], 
                          [APP_URI, "externalApplication"]],              
               [APP_URI, ["path", "prettyName"]] ];
  for (var i = 0; i < urns.length; i++) {
    var mimeRes = gRDF.GetResource(urns[i][0](aMIMEType));
    // unassert the toplevel properties
    var properties = urns[i][1];
    for (var j = 0; j < properties.length; j++) {
      var propertyRes = gRDF.GetResource(NC_RDF(properties[j]), true);
      if (properties[j] == "fileExtensions") {  // hacky. do it better next time. 
        var mimeValues = gDS.GetTargets(mimeRes, propertyRes, true);
        mimeValues = mimeValues.QueryInterface(Components.interfaces.nsISimpleEnumerator);
        while (mimeValues.hasMoreElements()) {
          var currItem = mimeValues.getNext();
          if (mimeRes && propertyRes && currItem) 
            gDS.Unassert(mimeRes, propertyRes, currItem, true);
        }
      }
      else {
        var mimeValue = gDS.GetTarget(mimeRes, propertyRes, true);
        if (mimeRes && propertyRes && mimeValue)
          gDS.Unassert(mimeRes, propertyRes, mimeValue, true);
      }
    }
    if (urns[i][2]) {
      var linkRes = gRDF.GetResource(NC_RDF(urns[i][2][1]), true);
      var linkTarget = gRDF.GetResource(urns[i][2][0](aMIMEType), true);
      gDS.Unassert(mimeRes, linkRes, linkTarget);
    }
  }
  try {
    gDS.QueryInterface(Components.interfaces.nsIRDFRemoteDataSource).Flush();
  } catch(e) {
  }
}

function checkInput() {
  var result = true;
  // Check for empty MIME type field.
  if ( gMIMEField.value.search(/\S/) == -1 ) {
    // Input is not OK.
    result = false;

    // Focus the mime type field.
    gMIMEField.focus();

    // Put up alert.  Title is same as parent dialog's.
    var title    = window.document.documentElement.getAttribute( "title" );
    var text     = gPrefApplicationsBundle.getString("emptyMIMEType");
    var prompter = Components.classes["@mozilla.org/embedcomp/prompt-service;1"].getService(Components.interfaces.nsIPromptService);
    prompter.alert(window, title, text);
  }
  return result;
}
