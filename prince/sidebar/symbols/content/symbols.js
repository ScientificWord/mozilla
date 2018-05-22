function symbolStartup()
{
//  var editorElement = msiGetActiveEditorElement(window.parent);
//  window.msiSetupHTMLEditorCommands(editorElement);
  initUserSymbols();
}

function doSymbols(event)
{
  var w = window.parent;
  if (!("doParamCommand" in w)) w = w.opener;
  var editorElement = msiGetActiveEditorElement(w);
  editorElement.contentWindow.focus();
  // this will get to the right command when the tabs are in the sidebar or detached
  try
  {
    var cmdParams = newCommandParams();
    if (!cmdParams) return;

    cmdParams.setStringValue("value", event.target.getAttribute('msivalue'));
    msiGoDoCommandParams("cmd_MSIsymbolCmd", cmdParams);
  } catch(e) { dump("error thrown in doSymbols: "+e+"\n"); }
}


function doChar(event)
{
  var w = window.parent;
  if (!("doParamCommand" in w)) w = w.opener;
  var editorElement = msiGetActiveEditorElement(w);
  editorElement.contentWindow.focus();
  // this will get to the right command when the tabs are in the sidebar or detached
  var editor = msiGetEditor(editorElement);
  try
  {
    var s = event.target.getAttribute('msivalue');
    editor.insertText(s);
  } catch(e) { dump("error thrown in doSymbols: "+e+"\n"); }
}


function onHeightChange(h)
{
//  alert("foo! Height is "+h);
  var adjHeight = Number(h) - document.getElementById("tabs-box").boxObject.height;
  var broadcaster = document.getElementById("sidebar-symbol-tabbox");
  broadcaster.setAttribute("style","max-height: "+h+"px; overflow-y: auto;");
  broadcaster = document.getElementById("tabpanelstyle");
  broadcaster.setAttribute("style","max-height: "+adjHeight+"px; overflow-y: auto;");
}


function windowChanged()
{
  var h = innerHeight;
  onHeightChange(h);
}

function constructSymbolList(cachepanel) {
  var buttonlist = cachepanel.getElementsByTagName('toolbarbutton');
  var symbollist = ' ';
  for (var i = 0; i < buttonlist.length; i++) {
    symbollist = symbollist + buttonlist[i].id.replace(/^(cache_)*/,'')+ ' ';
  }
  cachepanel.setAttribute('userlistofsymbols', symbollist);
}


var symObserver =
{
  canHandleMultipleItems: function ()
  {
    return true;
  },

  onDragStart: function (evt, transferData, action)
  {
    var button = evt.originalTarget;
    if (button.nodeName !== "toolbarbutton") return;
    var buttonData =
    "id:"+ button.getAttribute("id") +" label:"+button.getAttribute("label")+" msivalue:"+
        button.getAttribute("msivalue")+" observes:"+ button.getAttribute("observes")+" tooltiptext:" +
        button.getAttribute("tooltiptext");
    transferData.data = new TransferData();
    transferData.data.addDataForFlavour("symbolbutton", buttonData);
  },

  canDrop: function(evt, session)
  {
    return true;
  },

  onDrop: function(evt, dropData, session)
  {
    if (evt.currentTarget.id === "cachebar" || evt.currentTarget.id === "cachepanel")
    {
      var supported = session.isDataFlavorSupported("symbolbutton");
      if (supported)
      {
        var cachepanel = document.getElementById("cachepanel");
        var data = dropData.first.first.data;
        var attArray = data.split(" ");
        var arr;
        var s;
        var id;
        var button = document.createElement("toolbarbutton");
        var targetButton;
        if (evt.currentTarget.id === "cachebar") {
          targetButton= cachepanel.firstChild;
        } else
        {
          targetButton = evt.explicitOriginalTarget;
        }
        if (targetButton != null) cachepanel.insertBefore(button, targetButton);
        else cachepanel.appendChild(button);
        for (var i = 0; i < attArray.length; i++)
        {
          arr = attArray[i].split(":");
          if (arr[0].length > 0)
          {
            if (arr[0] === "id")
            {
              id = arr[1];
              if (arr[1].indexOf('cache_') === 0) {
                // we are copying from the cache panel. We must delete the source button
                cachepanel.removeChild(document.getElementById(id));
                arr[1] = id;
              }
              else arr[1] = "cache_"+id;
            }
            button.setAttribute(arr[0],arr[1]);
          }
        }
        constructSymbolList(cachepanel);
      }
    }
  },

  onDragOver: function(evt, flavour, session)
  {
    if (evt.currentTarget.nodeName == "tab")
    {
      var supported = session.isDataFlavorSupported("symbolbutton");

      if (supported)
      {
        session.canDrop = true;
        evt.currentTarget.setAttribute("selected","1");
      }
    }
  },

  onDragEnter: function(evt, flavour, session)
  {
    var supported = session.isDataFlavorSupported("symbolbutton");

    if (supported)
      evt.currentTarget.setAttribute("selected", "1");
  },

  getSupportedFlavours: function()
  {
    var flavours = new FlavourSet();
    flavours.appendFlavour("symbolbutton");
//    flavours.appendFlavour("text/html");
    return flavours;
  }
}


function initUserSymbols()
{
  var cachepanel = document.getElementById("cachepanel");
  var usersymbols = cachepanel.getAttribute("userlistofsymbols");
  var symbols = usersymbols.split(" ");
  var symbolbutton;
  var i;
  for (i=symbols.length - 1; i >= 0; i-- )
  {
    try
    {
      if (symbols[i].length > 0)
      {
        symbolbutton = document.getElementById(symbols[i]).cloneNode(false);
        symbolbutton.setAttribute("id", "cache_"+document.getElementById(symbols[i]).getAttribute("id"));
        var firstTBButton = cachepanel.firstChild;
        if (firstTBButton != null) cachepanel.insertBefore(symbolbutton, firstTBButton);
        else cachepanel.appendChild(symbolbutton);
      }
    }
    catch(e)
    {}
  }
}

var button;
function initPopup(event)
{
  button = event.explicitOriginalTarget;
  var isCachePanel = (button.getAttribute('id').indexOf('cache_')===0);
  document.getElementById('context_remove').hidden = !isCachePanel;
  if (isCachePanel)
    document.getElementById('context_cache').label = "Move symbol to beginning of user palette";
  else
    document.getElementById('context_cache').label = "Add symbol to user palette";
}

function remove(event)
{
  if (!button || button.nodeName != "toolbarbutton") return;
  var cachepanel = document.getElementById("cachepanel");
  var usersymbols = cachepanel.getAttribute("userlistofsymbols");
  var id = button.getAttribute("id");
  if (id.indexOf("cache_") == 0) id=id.slice(6);
  cachepanel.removeChild(button);
  constructSymbolList(cachepanel);
}

function addSymbolToCachePanel(event) {
  if (!button || button.nodeName != "toolbarbutton") return;
  var cachepanel = document.getElementById("cachepanel");
  var usersymbols = cachepanel.getAttribute("userlistofsymbols");
  var id = button.getAttribute("id");
  var symbolbutton;
  var needToDelete = false;
  if (id.indexOf("cache_") == 0) {
    id=id.slice(6);
    needToDelete = true;
  }
  symbolbutton = button.cloneNode(false);
  if (needToDelete) cachepanel.removeChild(button);
  symbolbutton.setAttribute("id", "cache_"+button.getAttribute("id"));
  var firstTBButton = cachepanel.firstChild;
  if (firstTBButton == null) firstTBButton = null; //converts void to null if necessary
  cachepanel.insertBefore(symbolbutton, firstTBButton);
}
