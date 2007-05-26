// Copyright 2007 MacKichan Software, Inc.

// Constants

const nsISupports = Components.interfaces.nsISupports;
const msiIOTFontlist = Components.interfaces.msiIOTFontlist;

const CLASS_ID = Components.ID("{88D14045-E268-41a8-BE7F-B8C3DAEA5051}");
const CLASS_NAME = "Open Type font listing component";
const CONTRACT_ID = "@mackichan.com/otfontlist;1";

var gSystemFonts;
var gSystemFontCount;
// class definition

// constructor

function OTFontlist()
{};

// class definition

OTFontlist.prototype = {
  getOTFontlist: function( count ) { 
    if (!gSystemFonts)
    {
      gSystemFonts = Components.classes["@mozilla.org/array;1"].createInstance(Components.interfaces.nsIArray);
      // Build list of all system fonts once per editor
      try 
      {
        var enumerator = Components.classes["@mozilla.org/gfx/fontenumerator;1"]
                                   .getService(Components.interfaces.nsIFontEnumerator);
        gSystemFontCount = { value: 0 }
        gSystemFonts = enumerator.EnumerateAllFonts(gSystemFontCount);
      }
      catch(e) { }
    }
    count = gSystemFontCount;
    return gSystemFonts;
  },
    
  QueryInterface: function(aIID)
  {
    dump("Fontlist queryinterface\n");
    if (!aIID.equals(msiIOTFontlist) &&
      !aIID.equals(nsISupports))
      throw Components.results.NS_ERROR_NO_INTERFACE;
    return this;
  }
};

// class factory

  
var OTFontlistFactory = {
  createInstance: function (aOuter, aIID)
  {
    dump("FontlistFactory createInstance\n");
    if (aOuter != null)
      throw Components.results.NS_ERROR_NO_AGGREGATION;
    dump("FontlistFactory createInstance returning valid object\n");
    return (new OTFontlist()).QueryInterface(aIID);
  }
};

var OTFontlistModule = {
  registerSelf: function(aCompMgr, aFileSpec, aLocation, aType)
  {
    dump("Registering OTFontlistModule\n");
    aCompMgr = aCompMgr.
        QueryInterface(Components.interfaces.nsIComponentRegistrar);
    aCompMgr.registerFactoryLocation(CLASS_ID, CLASS_NAME, 
        CONTRACT_ID, aFileSpec, aLocation, aType);
  },

  unregisterSelf: function(aCompMgr, aLocation, aType)
  {
    dump("Unregistering OTFontlistModule\n");
    aCompMgr = aCompMgr.
        QueryInterface(Components.interfaces.nsIComponentRegistrar);
    aCompMgr.unregisterFactoryLocation(CLASS_ID, aLocation);        
  },
  
  getClassObject: function(aCompMgr, aCID, aIID)
  {
    dump("GetClassObject for OTFontlistModule\n");
    if (!aIID.equals(Components.interfaces.nsIFactory))
      throw Components.results.NS_ERROR_NOT_IMPLEMENTED;

    if (aCID.equals(CLASS_ID))
      return OTFontlistFactory;

    throw Components.results.NS_ERROR_NO_INTERFACE;
  },

  canUnload: function(aCompMgr) { return true; }
};

/***********************************************************
module initialization

When the application registers the component, this function
is called.
***********************************************************/
function NSGetModule(aCompMgr, aFileSpec) { return OTFontlistModule; }

  
  
  
  
  