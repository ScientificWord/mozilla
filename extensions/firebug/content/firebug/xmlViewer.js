/* See license.txt for terms of usage */

FBL.ns(function() { with (FBL) {

// ************************************************************************************************
// Constants

// List of XML related content types.
var xmlContentTypes =
[
    "text/xml",
    "application/xml",
    "application/xhtml+xml",
    "application/rdf+xml",
    "application/vnd.mozilla.xul+xml",
];

// ************************************************************************************************
// Model implementation

/**
 * @module Implements viewer for XML based network responses. In order to create a new
 * tab wihin network request detail, a listener is registered into
 * <code>Firebug.NetMonitor.NetInfoBody</code> object.
 */
Firebug.XMLViewerModel = extend(Firebug.Module,
{
    dispatchName: "xmlViewer",

    initialize: function()
    {
        Firebug.ActivableModule.initialize.apply(this, arguments);
        Firebug.NetMonitor.NetInfoBody.addListener(this);
    },

    shutdown: function()
    {
        Firebug.ActivableModule.shutdown.apply(this, arguments);
        Firebug.NetMonitor.NetInfoBody.removeListener(this);
    },

    /**
     * Check response's content-type and if it's a XML, create a new tab with XML preview.
     */
    initTabBody: function(infoBox, file)
    {
        if (this.isXML(safeGetContentType(file.request)))
        {
            Firebug.NetMonitor.NetInfoBody.appendTab(infoBox, "XML",
                $STR("xmlviewer.tab.XML"));

        }
    },

    isXML: function(contentType)
    {
        if (!contentType)
            return false;

        // Look if the response is XML based.
        for (var i=0; i<xmlContentTypes.length; i++)
        {
            if (contentType.indexOf(xmlContentTypes[i]) == 0)
                return true;
        }

        return false;
    },

    /**
     * Parse XML response and render pretty printed preview.
     */
    updateTabBody: function(infoBox, file, context)
    {
        var tab = infoBox.selectedTab;
        var tabBody = infoBox.getElementsByClassName("netInfoXMLText").item(0);
        if (!hasClass(tab, "netInfoXMLTab") || tabBody.updated)
            return;

        tabBody.updated = true;

        this.insertXML(tabBody, file.responseText);
    },

    insertXML: function(parentNode, text)
    {
        var parser = CCIN("@mozilla.org/xmlextras/domparser;1", "nsIDOMParser");
        var doc = parser.parseFromString(text, "text/xml");
        var root = doc.documentElement;

        // Error handling
        var nsURI = "http://www.mozilla.org/newlayout/xml/parsererror.xml";
        if (root.namespaceURI == nsURI && root.nodeName == "parsererror")
        {
            this.ParseError.tag.replace({error: {
                message: root.firstChild.nodeValue,
                source: root.lastChild.textContent
            }}, parentNode);
            return;
        }

        Firebug.HTMLPanel.CompleteElement.tag.replace({object: doc.documentElement}, parentNode);
    }
});

// ************************************************************************************************
// Domplate

/**
 * @domplate Represents a template for displaying XML parser errors. Used by
 * <code>Firebug.XMLViewerModel</code>.
 */
Firebug.XMLViewerModel.ParseError = domplate(Firebug.Rep,
{
    tag:
        DIV({"class": "xmlInfoError"},
            DIV({"class": "xmlInfoErrorMsg"}, "$error.message"),
            PRE({"class": "xmlInfoErrorSource"}, "$error|getSource")
        ),

    getSource: function(error)
    {
        var parts = error.source.split("\n");
        if (parts.length != 2)
            return error.source;

        var limit = 50;
        var column = parts[1].length;
        if (column >= limit) {
            parts[0] = "..." + parts[0].substr(column - limit);
            parts[1] = "..." + parts[1].substr(column - limit);
        }

        if (parts[0].length > 80)
            parts[0] = parts[0].substr(0, 80) + "...";

        return parts.join("\n");
    }
});

// ************************************************************************************************
// Registration

Firebug.registerModule(Firebug.XMLViewerModel);

}});
