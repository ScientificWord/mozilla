/* See license.txt for terms of usage */
FBL.ns(function xpcomExplorer() { with (FBL) {

const Cc = Components.classes;
const Ci = Components.interfaces;

const reComponents = /:\/(.*)\/components\//; // chrome:/ or file:/
const reExtensionInFileURL = /file:.*\/extensions\/([^\/]*)/;
const reResource = /resource:\/\/([^\/]*)\//;
const reModules = /:\/\/(.*)\/modules\//; // chrome:// or file://
const reWeb = /(^http:|^ftp:|^mailto:|^https:|^ftps:)\//;
const reXUL = /\.xul$|\.xml$|^XStringBundle$/;

top.Chromebug = {}; // utility functions for Chromebug modules and ui

this.namespaceName = "ChromeBug";

//*******************************************************************************

Chromebug.parseWebURI = function(uri)
{
    var m = reWeb.exec(uri);
    if(m)
    {
        var split = FBL.splitURLBase(uri);
        return {path: m[1], name: split.path+'/'+split.name, kind:"web", pkgName: m[1]};
    }
}

Chromebug.parseSystemURI = function(uri)
{
    if (isSystemURL(uri))
    {
        var split =  FBL.splitURLBase(uri);
        return {path: split.path, name: split.name, kind: "system", pkgName: "system" }
    }
}

Chromebug.parseNoWindowURI = function(uri)
{
    if (uri.indexOf('noWindow')==0)
    {
        var sandbox = uri.indexOf('Sandbox');
        if (sandbox > 0)
            return {path: "Sandbox", name: uri.substr(11), kind: "sandbox", pkgName: "Sandbox"};

        return {path: uri.substr(0,9), name: uri.substr(11), kind: "noWindow", pkgName: "noWindow" }
    }
}

Chromebug.parseDataURI = function(URI)
{
    if (isDataURL(URI))
    {
        var split = splitURLBase(URI);

        if (FBTrace.DBG_LOCATIONS)
            FBTrace.sysout("parseDataURI "+URI, split);
        return {path: "data:", name: split.path+'/'+split.name, kind:"data", pkgName: "data:"};
    }
}

Chromebug.parseComponentURI = function(URIString)
{
    var m = reComponents.exec(URIString);
    if (m)
    {
        return { path: "components", pkgName: m[1], name: new String(URIString), href: URIString, kind: 'component' };
    }
    else
          return null;
};

Chromebug.parseModuleURI = function(URIString)
{
    if (Firebug.Chromebug.isChromebugURL(URIString))
        return null;

    var m = reModules.exec(URIString);
    if (m)
    {
           var module = m[1];
           //var remainder = m[0].length;
        return { path: "modules", name: new String(URIString), pkgName: "modules", href: URIString, kind: 'module' };
    }
    else
          return null;
};


Chromebug.parseExtensionURI = function(URIString)
{
    const appURLStem = Firebug.Chromebug.getPlatformStringURL("resource:app");

    var m = FBL.reChrome.exec(URIString) || reExtensionInFileURL.exec(URIString) || reResource.exec(URIString);
    var pkgName, remainder;
    if (m)
    {
        pkgName = m[1];
        remainder = m[0].length;
    }
    else
    {
        if (URIString && URIString.indexOf(appURLStem) == 0)
        {
            pkgName = "application";
            remainder = appURLStem.length;
        }
        // else not one of ours
        return null;
    }
    return {path: pkgName, name: new String(URIString.substr(remainder)), pkgName: pkgName, href: URIString, kind:'extension'};
};


Chromebug.parseURI = function(URI)
{
    if (!URI || Firebug.Chromebug.isChromebugURL(URI))
        return null;

    var description = null;
    if (!description)
        description = Chromebug.parseNoWindowURI(URI);
    if (!description)
        description = Chromebug.parseComponentURI(URI);
    if (!description)
        description = Chromebug.parseExtensionURI(URI);
    if (!description)
        description = Chromebug.parseModuleURI(URI);
    if (!description)
        description = Chromebug.parseSystemURI(URI);
    if (!description)
        description = Chromebug.parseWebURI(URI);
    if (!description)
        description = Chromebug.parseDataURI(URI);

    if (!description)
    {
        if (FBTrace.SOURCEFILES)
            FBTrace.sysout("Chromebug.parseURI: no match for "+URI);
        description = {path:"mystery", name:URI, kind: "mystery", pkgName: "unparsable"};
    }

    return description;
}


Chromebug.SourceFileListBase = function()
{
}

Chromebug.SourceFileListBase.prototype = extend(new Firebug.Listener(),
{
    parseURI: Chromebug.parseURI,

    getDescription: function(sourceFile)
    {
        var description = this.parseURI(sourceFile.href);
        if (description)
        {
            if (sourceFile.context)
            {
                description.context = sourceFile.context;
            }
            else
                FBTrace.sysout("ERROR in getDescription: sourceFile has no context");

            return description;
        }
        else
            return false;
    },

    supports: function(sourceFile)
    {
        return this.getDescription(sourceFile);
    },

    getPackageNames: function()
    {
        var slots = {};
        this.eachSourceFileDescription(function extractPackageNames(d)
        {
            slots[d.pkgName] = 1;
        });
        var list = [];
        for (var p in slots)
            if (slots.hasOwnProperty(p))
                list.push(p);
        return list;
    },

    eachSourceFileDescription: function(fnTakesSourceFileDescription)
    {
        var getDescription = bind(this.getDescription, this);
        var rc = Firebug.Chromebug.eachSourceFile(function visitSourceFiles(sourceFile)
        {
            var d = getDescription(sourceFile);
            if ( d )
            {
                var rc = fnTakesSourceFileDescription(d);
                if (rc)
                    return rc;
            }
        });
        return rc;
    },

    setFilter: function(fnTakesDescription)  // outsider can set filter, eg in onSetLocation
    {
        this.filter = fnTakesDescription;
    },

    isWantedDescription: function(description)
    {
        if (this.filter)
            return this.filter(description);
        else
            return true;
    },

    getLocationList: function()
    {
        var self = this;
        var list = [];
        this.eachSourceFileDescription(function joinSourceFileDescriptions(d)
        {
            if (self.isWantedDescription(d))
                list.push(d);
        } );
        return list;
    },

    getDefaultLocation: function()
    {
        var locations = this.getLocationList();
        if (locations && locations.length > 0) return locations[0];
    },

    getObjectLocation: function(sourceFileDescription)
    {
        if (sourceFileDescription)
            return sourceFileDescription.href;
        else
            return "no sourcefile:";
    },

    getObjectDescription: function(sourceFileDescription) // path: package name, name: remainder
    {
        if (sourceFileDescription)
        {
            var cn = sourceFileDescription.context.getName();
            if (cn)
            {
                cnParts = cn.split('/');
                cn = cnParts[cnParts.length - 1];
            }
            var nameParts = sourceFileDescription.name.split('/');
            var name = nameParts[nameParts.length - 1];
            var description =
            {
                name: cropString(name+(cn?" in "+cn:""), 120),
                path: cropString(sourceFileDescription.path, 120),
                label: cropString(name, 40),
            }
            return description;
        }
        return {path: "SourceFileListBase", name:"no sourceFileDescription"};
    },

    toString: function()
    {
        return "Source File List "+this.kind;
    },

    getCurrentLocation: function()
    {
        return this.elementBoundTo.repObject;
    },

    setCurrentLocation: function(description)
    {
        if (FBTrace.DBG_LOCATIONS)
            FBTrace.sysout("setCurrentLocation ", description);

        this.elementBoundTo.location = description;
        dispatch(this.fbListeners, "onSetLocation", [this, description]);
    },

    onSelectLocation: function(event)
    {
        var description = event.currentTarget.repObject;
        if (description)
            this.doSelect(description);
        else
            FBTrace.sysout("onSelectLocation FAILED, no repObject in currentTarget", event.currentTarget);
    },

    doSelect: function(description)
    {
        var context = description.context;
        if (context)
        {
            var sourceFile= context.sourceFileMap[description.href];
            if (FBTrace.DBG_LOCATIONS)
                FBTrace.sysout("AllFilesList.onSelectLocation context "+context.getName()+" url:"+description.href, description);
            Firebug.Chromebug.selectContext(context);

            FirebugChrome.select(sourceFile, "script", "watch", true);  // SourceFile
            this.setCurrentLocation(description);
        }
        else
            FBTrace.sysout("AllFilesList.onSelectLocation no context in description"+description, description);
    }

});

Chromebug.connectedList = function(xul_element, list)
{
    if (!list.elementBoundTo)
    {
        list.elementBoundTo = xul_element;
        xul_element.addEventListener("selectObject", bind(list.onSelectLocation, list), false);
        if (list.onPopUpShown)
            xul_element.addEventListener("popupshown", bind(list.onPopUpShown, list), false);
    }
    return list;
}



}});