// Copyright 2007 MacKichan Software, Inc.

// Constants

const msiIScriptRunner = Components.interfaces.msiIScriptRunner;
const nsISupports = Components.interfaces.nsISupports;

const CLASS_ID = Components.ID("{7BB5750B-BF2F-49d9-973E-403D489A602D}");
const CLASS_NAME = "Script runner XPCOM component";
const CONTRACT_ID = "@mackichan.com/scriptrunner;1";

// class definition

// constructor

function ScriptRunner()
{};

// class definition

ScriptRunner.prototype = {
  ctx: null,
  Eval: function( toExecute ) {
    try {
      eval( toExecute, this.ctx );
      return '';
    }
    catch( e )
    {
      return e.toString();
    }
  },

  setContext: function( newctx ) {
    // dump("set ctx to " + newctx + "\n");
    this.ctx = newctx;
  },

  QueryInterface: function(aIID)
  {
    // dump("ScriptRunner queryinterface\n");
    if (!aIID.equals(msiIScriptRunner) &&
      !aIID.equals(nsISupports))
      throw Components.results.NS_ERROR_NO_INTERFACE;
    return this;
  }
};

// class factory

//MyScriptRunner = Components.classes["@mackichan.com/scriptrunner;1"].
//  createInstance(Components.interfaces.msiIScriptRunner);

var ScriptRunnerFactory = {
  createInstance: function (aOuter, aIID)
  {
    // dump("ScriptRunnerFactory createInstance\n");
    if (aOuter != null)
      throw Components.results.NS_ERROR_NO_AGGREGATION;
    // dump("ScriptRunnerFactory createInstance returning valid object\n");
    return (new ScriptRunner()).QueryInterface(aIID);
  }
};

var ScriptRunnerModule = {
  registerSelf: function(aCompMgr, aFileSpec, aLocation, aType)
  {
    // dump("Registering ScriptRunnerModule\n");
    aCompMgr = aCompMgr.
        QueryInterface(Components.interfaces.nsIComponentRegistrar);
    aCompMgr.registerFactoryLocation(CLASS_ID, CLASS_NAME,
        CONTRACT_ID, aFileSpec, aLocation, aType);
  },

  unregisterSelf: function(aCompMgr, aLocation, aType)
  {
    // dump("Unregistering ScriptRunnerModule\n");
    aCompMgr = aCompMgr.
        QueryInterface(Components.interfaces.nsIComponentRegistrar);
    aCompMgr.unregisterFactoryLocation(CLASS_ID, aLocation);
  },

  getClassObject: function(aCompMgr, aCID, aIID)
  {
    // dump("GetClassObject for ScriptRunnerModule\n");
    if (!aIID.equals(Components.interfaces.nsIFactory))
      throw Components.results.NS_ERROR_NOT_IMPLEMENTED;

    if (aCID.equals(CLASS_ID))
      return ScriptRunnerFactory;

    throw Components.results.NS_ERROR_NO_INTERFACE;
  },

  canUnload: function(aCompMgr) { return true; }
};

/***********************************************************
module initialization

When the application registers the component, this function
is called.
***********************************************************/
function NSGetModule(aCompMgr, aFileSpec) { return ScriptRunnerModule; }






