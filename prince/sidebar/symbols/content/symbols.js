function startup()
{
//  var editorElement = msiGetActiveEditorElement(window.parent);
//  window.msiSetupHTMLEditorCommands(editorElement);
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
  var adjHeight = Number(h) - 31 - 31 - 4 - 2;
  var broadcaster = document.getElementById("sidebar-symbol-tabbox");
  broadcaster.setAttribute("style","height: "+adjHeight+"px;");
  broadcaster = document.getElementById("tabpanelstyle");
  broadcaster.setAttribute("style","height: "+adjHeight+"px; overflow-y: auto;");
}


function windowChanged()
{
  var h = innerHeight;
  onHeightChange(h);
}


var symObserver = 
{ 
  canHandleMultipleItems: function ()
  {
    return true;
  },
  
  onDragStart: function (evt, transferData, action)
  {
    var button = evt.currentTarget;
    var buttonData = {label: button.getAttribute("label"),
                      msivalue: button.getAttribute("msivalue"),
                      observes: button.getAttribute("observes"),
                      tooltiptext: button.getAttribute("tooltiptext")};
    transferData.data = new TransferData();
    transferData.data.addDataForFlavour("symbolbutton", buttonData);
  },
  
  canDrop: function(evt, session)
  {
    return true;
  },
  
  onDrop: function(evt, dropData, session)
  {
  },
  
  onDragOver: function(evt, flavour, session) 
  {
    dump(flavour.contentType+"\n");
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
    return flavours;
  }
}  
