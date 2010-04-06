/* See license.txt for terms of usage */

// Our global trace object.

var FBTrace = Components.classes["@joehewitt.com/firebug-trace-service;1"]
                 .getService(Components.interfaces.nsISupports).wrappedJSObject.getTracer("extensions.firebug");
FBTrace.setScope(window);
function clearFBTraceScope()
{
    window.removeEventListener('unload', clearFBTraceScope, true);
    FBTrace.setScope(null);
}
window.addEventListener('unload', clearFBTraceScope, true);
// ************************************************************************************************
// Some examples of tracing APIs

// 1) Log "Hello World!" into the console.
//    FBTrace.sysout("Hello World!")
//
// 2) Log "Hello World!" if the DBG_ERRORS option is true.
//    if (FBTrace.DBG_ERRORS)
//       FBTrace.sysout("Hello World!");
//
// 3) Log "Hello World!" and various info about 'world' object.
//    FBTrace.sysout("Hello World!", world);
//
// 4) Log into specific console (created by Firebug extension).
//    FBTrace.dump("firebug.extensions", "Hello World!", world);
//    FBTrace.dump("chromebug.extensions", "Hello World!", world);
//
// TODO: how to open another console.
