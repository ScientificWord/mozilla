
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
    changeMIMEStuff(MIME_URI(aMIMETypeString), "value", aMIMETypeString.toLowerCase());
  },
  
  get description()
  {
    return getLiteralForContentType(this.URI, "description");
  },  
  
  set description(aDescriptionString)
  {
    changeMIMEStuff(MIME_URI(this.mimeType), "description", aDescriptionString);
  },
  
  get isEditable()
  {
    return getLiteralForContentType(this.URI, "editable");
  },
  
  set isEditable(aIsEditableString)
  {
    changeMIMEStuff(MIME_URI(this.mimeType), "editable", aIsEditableString);
  },

  get extensions()
  {
    var extensionResource = gRDF.GetUnicodeResource(NC_RDF("fileExtensions"));
    var contentTypeResource = gRDF.GetUnicodeResource(MIME_URI(this.mimeType));
    var extensionTargets = gDS.GetTargets(contentTypeResource, extensionResource, true);
    var extString = "";
    if (extensionTargets) {
      while (extensionTargets.hasMoreElements()) {
        var currentExtension = extensionTargets.getNext();
        if (currentExtension) {
          currentExtension = currentExtension.QueryInterface(Components.interfaces.nsIRDFLiteral);
          if (extString != "") {
            extString += " ";
          }
          extString += currentExtension.Value.toLowerCase();
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

  clearExtensions: function ()
  {
    var extArray = this.extensions.split(" ");
    for (i = extArray.length - 1; i >= 0; --i) {
      this.removeExtension(extArray[i]);
    }
  },
  
  // content handling
  get saveToDisk()
  {
    return getHandlerInfoForType(this.URI, "saveToDisk");
  },
  
  set saveToDisk(aSavedToDisk)
  {
    changeMIMEStuff(HANDLER_URI(this.mimeType), "saveToDisk", aSavedToDisk);
    this.setHandlerProcedure("handleInternal", "false");
    this.setHandlerProcedure("useSystemDefault", "false");
 },

  get useSystemDefault()
  {
    return getHandlerInfoForType(this.URI, "useSystemDefault");
  },

  set useSystemDefault(aUseSystemDefault)
  {
    changeMIMEStuff(HANDLER_URI(this.mimeType), "useSystemDefault", aUseSystemDefault);
    this.setHandlerProcedure("handleInternal", "false");
    this.setHandlerProcedure("saveToDisk", "false");
  },
  
  get handleInternal()
  {
    return getHandlerInfoForType(this.URI, "handleInternal");
  },
  
  set handleInternal(aHandledInternally)
  {
    changeMIMEStuff(HANDLER_URI(this.mimeType), "handleInternal", aHandledInternally);
    this.setHandlerProcedure("saveToDisk", "false");
    this.setHandlerProcedure("useSystemDefault", "false");
  },

  setHandlerProcedure: function (aHandlerProcedure, aValue)
  {
    var handlerSource = gRDF.GetUnicodeResource(HANDLER_URI(this.mimeType));
    var handlerProperty = gRDF.GetUnicodeResource(NC_RDF(aHandlerProcedure));
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
    changeMIMEStuff(HANDLER_URI(this.mimeType), "alwaysAsk", aAlwaysAsk);
  },
  
  // helper application
  get appDisplayName()
  {
    return getHelperAppInfoForType(this.URI, "prettyName");
  },
  
  set appDisplayName(aDisplayName)
  {
    changeMIMEStuff(APP_URI(this.mimeType), "prettyName", aDisplayName);
  },
  
  get appPath()
  {
    return getHelperAppInfoForType(this.URI, "path");
  },
  
  set appPath(aAppPath)
  {
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
    var mimeSource = gRDF.GetUnicodeResource(MIME_URI(this.mimeType));
    var handlerProperty = gRDF.GetUnicodeResource(NC_RDF("handlerProp"));
    var handlerResource = gRDF.GetUnicodeResource(HANDLER_URI(this.mimeType));
    gDS.Assert(mimeSource, handlerProperty, handlerResource, true);
    // assert the helper app resource
    var helperAppProperty = gRDF.GetUnicodeResource(NC_RDF("externalApplication"));
    var helperAppResource = gRDF.GetUnicodeResource(APP_URI(this.mimeType));
    gDS.Assert(handlerResource, helperAppProperty, helperAppResource, true);
    // add the mime type to the MIME types seq
    var container = Components.classes["@mozilla.org/rdf/container;1"].createInstance();
    if (container) {
      container = container.QueryInterface(Components.interfaces.nsIRDFContainer);
      if (container) {
        var containerRes = gRDF.GetUnicodeResource("urn:mimetypes:root");
        container.Init(gDS, containerRes);
        var element = gRDF.GetUnicodeResource(MIME_URI(this.mimeType));
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
  var contentTypeResource = gRDF.GetUnicodeResource(aURI);
  var propertyResource = gRDF.GetUnicodeResource(NC_RDF(aProperty));
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
  var source = gRDF.GetUnicodeResource(handler);
  var property = gRDF.GetUnicodeResource(NC_RDF(aPropertyString));
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
  var appRes      = gRDF.GetUnicodeResource(appURI);
  var appProperty = gRDF.GetUnicodeResource(NC_RDF(aPropertyString));
  return getLiteral(appRes, appProperty);
}

function mimeHandlerExists(aMIMEType)
{
  var valueProperty = gRDF.GetUnicodeResource(NC_RDF("value"));
  var mimeSource = gRDF.GetUnicodeResource(MIME_URI(aMIMEType));
  var mimeLiteral = gRDF.GetLiteral(aMIMEType);
  return gDS.HasAssertion(mimeSource, valueProperty, mimeLiteral, true);
}

// write to the ds
function assertMIMEStuff(aMIMEString, aPropertyString, aValueString)
{
  var mimeSource = gRDF.GetUnicodeResource(aMIMEString);
  var valueProperty = gRDF.GetUnicodeResource(NC_RDF(aPropertyString));
  var mimeLiteral = gRDF.GetLiteral(aValueString);
  gDS.Assert(mimeSource, valueProperty, mimeLiteral, true);
}

function changeMIMEStuff(aMIMEString, aPropertyString, aValueString)
{
  var mimeSource = gRDF.GetUnicodeResource(aMIMEString);
  var valueProperty = gRDF.GetUnicodeResource(NC_RDF(aPropertyString));
  var mimeLiteral = gRDF.GetLiteral(aValueString);
  var currentValue = gDS.GetTarget(mimeSource, valueProperty, true);
  if (currentValue) {
    gDS.Change(mimeSource, valueProperty, currentValue, mimeLiteral);
  } else {
    gDS.Assert(mimeSource, valueProperty, mimeLiteral, true);
  } 
}

function unassertMIMEStuff(aMIMEString, aPropertyString, aValueString)
{
  var mimeSource = gRDF.GetUnicodeResource(aMIMEString);
  var valueProperty = gRDF.GetUnicodeResource(NC_RDF(aPropertyString));
  var mimeLiteral = gRDF.GetLiteral(aValueString);
  gDS.Unassert(mimeSource, valueProperty, mimeLiteral, true);
}

function removeOverride(aMIMEType)
{
  // remove entry from seq
  var rdfc = Components.classes["@mozilla.org/rdf/container;1"].createInstance();
  if (rdfc) {
    rdfc = rdfc.QueryInterface(Components.interfaces.nsIRDFContainer);
    if (rdfc) {
      var containerRes = gRDF.GetUnicodeResource("urn:mimetypes:root");
      rdfc.Init(gDS, containerRes);
      var element = gRDF.GetUnicodeResource(MIME_URI(aMIMEType));
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
               [HANDLER_URI, ["handleInternal", "saveToDisk", "alwaysAsk", "useSystemDefault"], 
                          [APP_URI, "externalApplication"]],              
               [APP_URI, ["path", "prettyName"]] ];
  for (var i = 0; i < urns.length; i++) {
    var mimeRes = gRDF.GetUnicodeResource(urns[i][0](aMIMEType));
    // unassert the toplevel properties
    var properties = urns[i][1];
    for (var j = 0; j < properties.length; j++) {
      var propertyRes = gRDF.GetUnicodeResource(NC_RDF(properties[j]), true);
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
    if ("2" in urns[i] && urns[i][2]) {
      var linkRes = gRDF.GetUnicodeResource(NC_RDF(urns[i][2][1]), true);
      var linkTarget = gRDF.GetUnicodeResource(urns[i][2][0](aMIMEType), true);
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
