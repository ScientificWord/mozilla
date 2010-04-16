FBL.ns(function() { with (FBL) {

const Cc = Components.classes;
const Ci = Components.interfaces;


Firebug.Platform =
{
    Cc: function()
    {
        var theCrashers = Firebug.Platform.ComponentClassCrashers;
        var wrapped = {};
        for (var p in Cc)
        {
            if (Cc[p] instanceof Ci.nsIJSCID && theCrashers.indexOf(p) == -1)
                wrapped[p] = new Firebug.Platform.ComponentClass(Cc[p]);
        }

        FirebugChrome.select(wrapped);
    },

     showDOMWindowInZOrder: function()
    {
        var obj = new Firebug.Platform.RefreshableEnumerator(function getIWindowMediator()
        {
            var wm = Components.classes["@mozilla.org/appshell/window-mediator;1"]
                                        .getService(Components.interfaces.nsIWindowMediator);
            return wm.getZOrderDOMWindowEnumerator("", true);
        });
        FirebugChrome.select(obj);
    },

    //-----------------------------------------------------------------------------------------------
    //

};

Firebug.Platform.ComponentClassCrashers = ["@mozilla.org/generic-factory;1", "QueryInterface", "@mozilla.org/dom/storage;2"];
Firebug.Platform.ComponentInterfaceCrashers = ["IDispatch"];

const theCrashers = Firebug.Platform.ComponentClassCrashers;

Firebug.Platform.XPConnectWrappedRep = domplate(Firebug.Rep,
{
    tag:
        FirebugReps.OBJECTLINK(
                SPAN({class: "objectTitle", _repObject: "$object|getWrappedValue"}, "$object|getTitle")
            ),

    className: "XPConnectWrapper",

    reXPConnect: /\[xpconnect wrapped ([^\]]*)\]/,

    supportsObject: function(object, type)
    {
        try
        {
            var p = safeToString( object );
            if (p.indexOf('[xpconnect wrapped') == 0)
                return 2;
        }
        catch(exc)
        {
            FBTrace.sysout("XPConnectWrappedRep FAILS "+exc, exc);
        }
        return 0;
    },

    getTitle: function(object)
    {
        try
        {
            var p = safeToString(object);
            var m = this.reXPConnect.exec(p);
            return m[1];
        }
        catch(exc)
        {
            FBTrace.sysout("XPConnectWrappedRep FAILS "+exc, exc);
        }
    },

    getWrappedValue: function(object)
    {
        return object.wrappedJSObject ? object.wrappedJSObject : object;
    },

});

Firebug.Platform.XULWindowRep = domplate(Firebug.Rep,
{
    tag:
        FirebugReps.OBJECTLINK(
                SPAN({class: "objectTitle"}, "$object|getTitle")
            ),

    className: "XULWindow",

    supportsObject: function(object, type)
    {
         return (object instanceof Ci.nsIXULWindow);
    },

    getTitle: function(xul_window)
    {
        var domWindow = Chromebug.XULAppModule.getDOMWindowByXULWindow(xul_window);
        return domWindow.document.title;
    },

    getRealObject: function(object, context)
    {
        FBTrace.sysout("platform.getRealObject ");
        return object;
    },
});

Firebug.registerRep(Firebug.Platform.XULWindowRep);
Firebug.registerRep(Firebug.Platform.XPConnectWrappedRep);

Firebug.Platform.nsXPCComponents_ClassesRep = domplate(Firebug.Rep,
{
    tag:
        FirebugReps.OBJECTLINK(
                SPAN({class: "objectTitle"}, "$object|getTitle")
            ),

    className: "nsXPCComponents_Classes",

    supportsObject: function(object, type)
    {
         return (safeToString(object) == "[object nsXPCComponents_Classes]");
    },

    getTitle: function(aClass)
    {
        return aClass.toString();
    },
});

Firebug.registerRep(Firebug.Platform.nsXPCComponents_ClassesRep);

//----------------------------------
// How Cc[p] objects will be shown.

Firebug.Platform.ComponentClassRep = domplate(Firebug.Rep,
{
    tag:
        FirebugReps.OBJECTLINK({onclick: "$object|getOnClick"},
                SPAN({class: "objectTitle"}, "$object|getTitle")
            ),

    className: "ComponentClass",

    supportsObject: function(object, type)
    {
         return (object instanceof Ci.nsIJSCID) && (theCrashers.indexOf(object.name) == -1);
    },

    getTitle: function(aClass)
    {
        return "call getService or CreateInstance";
    },

    getRealObject: function(object, context)
    {
        FBTrace.sysout("platform.getRealObject ");
        return object;
    },

    getOnClick: function(object)
    {
        // This method is called as the tag is expanded
        var self = Firebug.Platform.ComponentClassRep;
        return function selectService(event)
        {
            var svcOrObj = self.getServiceOrCreateInstance(object);
            //FBTrace.sysout("platform.selectService "+object.name, svcOrObj);
            FirebugChrome.select(svcOrObj);
            FBL.cancelEvent(event);
        }
    },

    getServiceOrCreateInstance: function(object)
    {
        var aComponent = null;
        try
        {
            // These were needed because some Component access crash FF
            if (Firebug.Platform.ComponentClassCrashers.indexOf(object.name) != -1)
                return "crasher";

            //window.dump("getServiceOrCreateInstance "+object.name+"\n");

            aComponent = Cc[object.name].getService(Ci.nsISupports);
        }
        catch(exc) // not a service I guess
        {
            try
            {
                aComponent = Cc[object.name].createInstance(Ci.nsISupports);
            }
            catch(exc)
            {
                return "not a service or object";
            }
        }
        getInterfaces(aComponent);  // QI it
        return aComponent;
    },

});


Firebug.registerRep(Firebug.Platform.ComponentClassRep);

Firebug.Platform.ComponentClass = function(aClass)
{
    this.nsIJSCID = aClass;
},

Firebug.Platform.ComponentClass.prototype =
{
        // Use getters here to delay the calls until the user opens the class to look
    get serviceOrCreateInstance()
    {
        if (!this.obj)
        {
            try
            {
                // These were needed because some Component access crash FF window.dump("get service "+this.nsIJSCID.name+"\n");
                if (Firebug.Platform.ComponentClassCrashers.indexOf(this.nsIJSCID.name) != -1)
                {
                    //window.dump("skipping "+this.nsIJSCID.name+"\n");
                    return "crasher";
                }
                //window.dump("get serviceOrCreateInstance "+this.nsIJSCID.name+"\n");

                this.obj = Cc[this.nsIJSCID.name].getService(Ci.nsISupports);
                this.isA = 'service';
                //window.dump("got service "+this.nsIJSCID.name+"\n");
            }
            catch(exc) // not a service I guess
            {
                try
                {
                    if (Firebug.Platform.ComponentClassCrashers.indexOf(this.nsIJSCID.name) != -1)
                        return "crasher";

                    this.obj = Cc[this.nsIJSCID.name].createInstance(Ci.nsISupports);
                    this.isA = 'instance';
                }
                catch(exc)
                {
                    return "not a service or object";
                }
            }
        }
        this.ifaces = getInterfaces(this.obj);  // QI it
        return this.obj;
    },

    get interfaces()
    {
        if (!this.ifaces)
            this.ifaces = getInterfaces(this.obj);  // QI it
        return this.ifaces;
    },

    toString: function()
    {
        return (this.nsIJSCID.valid?"valid ":"invalid ")+"Component";
    },

};

function getInterfaces(obj)
{
    var ifaces = [];
    for (var iface in Ci)
    {
        if (Firebug.Platform.ComponentInterfaceCrashers.indexOf(iface) != -1)
            continue;

        // window.dump("getInterfaces "+iface+"\n");


        if (obj instanceof Ci[iface]) {
            var ifaceProps = ifaces[iface] = [];
            for (p in Ci[iface])
                ifaceProps[p] = obj[p];
        }
    }
    return ifaces;
}


//  A wrapper around nsISimpleEnumerator
  /*
Firebug.Platform.RefreshableEnumerator = function(refresher)
{
    this.refresher = refresher;
},

Firebug.Platform.RefreshableEnumerator.prototype =
{
    hasMoreElements: function()
    {
        return this.enumerator.hasMoreElements();
    },

    getNext: function()
    {
        return this.enumerator.getNext();
    },

    refresh: function()
    {
        return this.enumerator = this.refresher();
    },
}

FirebugReps.RefreshableEnumerator = domplate(FirebugReps.Arr,
{
    supportsObject: function(object, type)
    {
    FBTrace.sysout("RefreshableEnumerator supportsObject ", object);
    return object instanceof Firebug.Platform.RefreshableEnumerator;
    },

    arrayIterator: function(enumerator)
    {
        var items = [];
        enumerator.refresh();
        while (enumerator.hasMoreElements())
        {
            var value = enumerator.getNext();
            var rep = Firebug.getRep(value);
            var tag = rep.shortTag ? rep.shortTag : rep.tag;
            var delim = (enumerator.hasMoreElements() ? "" : ", ");

            items.push({object: value, tag: tag, delim: delim});
        }

        return items;
    },

    shortArrayIterator: function(enumerator)
    {
        var items = [];
        FBTrace.sysout("shortArrayIterator enumerator ", enumerator);
        enumerator.refresh();
        var cutoff = 3;
        while (enumerator.hasMoreElements() && cutoff > 0)
        {
            var value = enumerator.getNext();
            var rep = Firebug.getRep(value);
            var tag = rep.shortTag ? rep.shortTag : rep.tag;
            var delim = (enumerator.hasMoreElements() ? "" : ", ");

            items.push({object: value, tag: tag, delim: delim});
            cutoff--;
        }

        if (enumerator.hasMoreElements())
            items.push({object: " more...", tag: FirebugReps.Caption.tag, delim: ""});

        return items;
    },

});

Firebug.registerRep(FirebugReps.RefreshableEnumerator);
*/
}});