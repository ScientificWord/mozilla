/* See license.txt for terms of usage */

const PrefService = Components.classes["@mozilla.org/preferences-service;1"];
const nsIPrefBranch2 = Components.interfaces.nsIPrefBranch2;
const prefs = PrefService.getService(nsIPrefBranch2);
const nsIPrefService = Components.interfaces.nsIPrefService;
const prefService = PrefService.getService(nsIPrefService);

var ChromeBugOpener =
{
    openNow: function()
    {
        var opener = this.getCommandLineHandler().wrappedJSObject;
        if (opener)
            return opener.openNow(window);
        else
            window.dump("ChromeBugOpener: no wrappedJSObject in command line handler\n");
    },

    getCommandLineHandler: function()
    {
        if (!this.ChromeBugCommandLineHandler)
        {
            this.ChromeBugCommandLineHandler = Components.classes['@mozilla.org/commandlinehandler/general-startup;1?type=chromebug'].
                getService(Components.interfaces.nsICommandLineHandler);
        }

        return this.ChromeBugCommandLineHandler;
    },

    setMenuByPref: function()
    {
        var menuitem = document.getElementById("menu_OpenChromeBugAlways");
        if (menuitem)
        {
            var openalways = Boolean(prefs.getBoolPref("extensions.chromebug.openalways"));
            menuitem.setAttribute("checked", openalways.toString() );
            //alert("set menuitem.checked to "+menuitem.getAttribute("checked"));
        }
        else
            window.dump("ChromeBugOpener: no element with id='menu_OpenChromeBugAlways'\n");
    },

    observe: function(subject, topic, data)
    {
        if (data == "extensions.chromebug.openalways")
            ChromeBugOpener.setMenuByPref();
    }
}

function ChromeBugOpenerOnLoad(event)
{
    ChromeBugOpener.setMenuByPref();
    prefs.addObserver("extensions.chromebug", ChromeBugOpener, false);
    window.removeEventListener("load", ChromeBugOpenerOnLoad, false);
}

window.addEventListener("load", ChromeBugOpenerOnLoad, false);
