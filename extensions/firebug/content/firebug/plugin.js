/* See license.txt for terms of usage */

FBL.ns(function() { with (FBL) {

// ************************************************************************************************

Firebug.PluginPanel = function() {};

Firebug.PluginPanel.prototype = extend(Firebug.Panel,
{
    createBrowser: function()
    {
        var doc = Firebug.chrome.window.document;
        this.browser = doc.createElement("browser");
        this.browser.addEventListener("DOMContentLoaded", this.browserReady, false);
        this.browser.className = "pluginBrowser";
        this.browser.setAttribute("src", this.url);  // see tabContext.createPanelType
    },

    destroyBrowser: function()
    {
        if (this.browser)
        {
            this.browser.parentNode.removeChild(this.browser);
            delete this.browser;
        }
    },

    browserReady: function()
    {
        this.browser.removeEventListener("DOMContentLoaded", this.browserReady, false);
        this.innerPanel = this.browser.contentWindow.FirebugPanel; // XXXjjb ?
        if (this.visible)
        {
            if (this.innerPanel)
                innerCall(this.innerPanel, "initialize", [this.context.window]);
            this.updateSelection(this.selection);
        }
    },

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
    // extends Panel

    initialize: function()
    {
        this.browserReady = bindFixed(this.browserReady, this);
        Firebug.Panel.initialize.apply(this, arguments);
    },

    destroy: function(state)
    {
        this.destroyBrowser();
        Firebug.Panel.destroy.apply(this, arguments);
    },

    reattach: function(doc)
    {
        this.destroyBrowser();
        this.createBrowser();
    },

    show: function(state)
    {
        if (!this.browser)
            this.createBrowser();
    },

    hide: function()
    {
    },

    supportsObject: function(object)
    {
        if (this.innerPanel)
            return innerCall(this.innerPanel, "supportsObject", [object]);
        else
            return 0;
    },

    updateSelection: function(object)
    {
        if (!this.innerPanel)
            return;

        innerCall(this.innerPanel, "select", [object]);
    },

    getObjectPath: function(object)
    {
    },

    getDefaultSelection: function()
    {
    },

    updateOption: function(name, value)
    {
    },

    getOptionsMenuItems: function()
    {
    },

    getContextMenuItems: function(object, target)
    {
    },

    getEditor: function(target, value)
    {
    }
});

// ************************************************************************************************

function innerCall(innerPanel, name, args)
{
    try
    {
        innerPanel[name].apply(innerPanel, args);
    }
    catch (exc)
    {
        ERROR(exc);
    }
}

// ************************************************************************************************

}});
