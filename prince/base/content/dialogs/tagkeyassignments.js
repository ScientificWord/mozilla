var keymapper;

function clear(event)
{
  document.getElementById("newkeys").value = "";
  event.preventDefault();
}

function handleChar(event, object)
{
  var key=event.keyCode;
  if (key < event.DOM_VK_F1){
    clear(event);
    return;
  }
  if (key > event.DOM_VK_F24){
    clear(event);
    return;
  }
  var display="";
  var keycode = "F"+(event.keyCode - event.DOM_VK_F1 +1);
  if (event.shiftKey) display += "Shift+";
  if (event.ctrlKey) display += "Ctrl+";
  if (event.altKey) display += "Alt+";
  if (event.metaKey) display += "Meta+";
  display += keycode;
  
  // F1 and F10 are reserved, plain and shifted
//   if ((key==event.DOM_VK_F1 || key==event.DOM_VK_F10) &&
//     !(event.ctrlKey | event.altKey | event.metaKey))
//   {
//     AlertWithTitle("Reserved function key","The "+display+" key is reserved");
//   }
//   else
  document.getElementById("newkeys").value = display;
  document.getElementById("assignkey").disabled = false;
  event.preventDefault();
}

function radioswitched( obj )
{
  var taglisttextbox=document.getElementById("alltaglist");
  var keyscript=document.getElementById("keyscript");
  if (obj.value == 'alltaglist') {
    taglisttextbox.disabled = false;
    keyscript.disabled = true;
  }
  else
  {
    taglisttextbox.disabled = true;
    keyscript.disabled = false;
  }
}

function startUp()
{
  document.getElementById("alltaglist").height = document.getElementById("keyscript").height;
 // Build the list of key combinations that have been assigned values or have been reserved
  keymapper =  Components.classes["@mackichan.com/keymap/keymap_service;1"]
                       .createInstance(Components.interfaces.msiIKeyMap);
  var strOfKeys = keymapper.getTableKeys("FKeys");
  var keylist = strOfKeys.split(" ");
  var currKey;
  var firstPart, secondPart;
  var len = keylist.length;
  var i;   
  var keyParts;
  var popup = document.getElementById("currentkeyslist");
  for (i = 0; i < len; i++)
  {
    currKey = keylist[i];
    if (currKey.length == 0) break;
    if (currKey[0] == "-"[0])
    {
      firstPart = "-";
      if (currKey[1] == "-"[0])
        secondPart = currKey.substr(2);
      else
        secondPart = "";
      secondPart = currKey[2];
    }
    else
    {
      keyParts = currKey.split("-");
      firstPart = keyParts[0];
      secondPart = keyParts[1];
    }
    var menuLabel = "";
    var fReserved = false;
    var j;
    for (j = 0; j<secondPart.length; j++)
    {
      if (secondPart[j] == "s") menuLabel += "Shift+";
      if (secondPart[j] == "a") menuLabel += "Alt+";
      if (secondPart[j] == "c") menuLabel += "Ctrl+";
      if (secondPart[j] == "m") menuLabel += "Meta+";
      if (secondPart[j] == "r") fReserved = true;
    }
    menuLabel += firstPart;
    var itemNode = document.createElementNS(XUL_NS, "listitem");
    itemNode.setAttribute("label", menuLabel);
    itemNode.setAttribute("value", currKey);
    if (fReserved) itemNode.setAttribute("reserved", "1");
    popup.appendChild(itemNode);
  }
}

function assignTag( obj )
{
  var editorElement = msiGetParentEditorElementForDialog(window);
  var editor = msiGetEditor(editorElement);
  var tag = obj.value;
  var tagmanager = editor.tagListManager;
  var data="";
  var tagclass = tagmanager.getClassOfTag(tag,null);
  if (tagclass == "texttag")
    data = "toggleTextTag('"+tag+"');";
  else if (tagclass == "paratag")
    data = "insertParaTag('"+tag+"');";
  else if (tagclass == "structtag")
    data = "insertSectionTag('"+tag+"');";
  else
    data = "insertTag('"+tag+"');";
  document.getElementById("keyscript").value = data;
}
  

function selectCurrentKey( amenulist )
{
  // this code strongly depends on the fact that the only keys allowed are function keys
  var keycodes = amenulist.value;
  var alt = false;
  var ctrl = false;
  var shift = false;
  var meta = false;
  var reserved = new Boolean(false);
  if (keycodes.length == 0 || keycodes[0] != "F"[0]) return;
  var num = parseInt(keycodes.substr(1,keycodes.indexOf("-")));
  if (isNaN(num) || num == 0) return;
  var keycode = 111 + num;
  var parts = keycodes.split("-");
  var secondPart = parts[1];
  var i;
  if (secondPart.length > 0)
  {
    for (i = 0; i<secondPart.length; i++)
    {
      if (secondPart[i] == "a"[0]) alt = true; 
      if (secondPart[i] == "c"[0]) ctrl = true; 
      if (secondPart[i] == "s"[0]) shift = true; 
      if (secondPart[i] == "m"[0]) meta = true;
    } 
    reserved.value = false;
    var mapsTo = keymapper.mapKeyToScript("FKeys", keycode, alt, ctrl, shift, meta, true, reserved);
    // now put the value in the text box
    document.getElementById("keyscript").value = mapsTo;
    // now if this is a command to change a tag, put the tag in the other text box and set the radio button
    // this needs to be made more robust. My regular expressions for finding the whole pattern didn't work.
    var button = document.getElementById("deleteassignment");
    button.disabled = false;
    button.label = "Delete assignment for " + amenulist.selectedItem.label;
    if (mapsTo.search("msiDoStatefulCommand") == 0)
    {
      var myArray = mapsTo.match("'([a-zA-Z()]+)'");
      if (myArray != null)
      {
        document.getElementById("tagorscript").value = "Key assigns the tag";
        document.getElementById("nameoftagorscript").value=myArray[1];
        document.getElementById("tagorscript").setAttribute("reserved","false");
        //document.getElementById("keyresult").selectedIndex = 0;
      }
    }
    else
    {
      document.getElementById("tagorscript").value = (reserved.value?"Key is reserved for operating system use":"Key runs the JavaScript");
      document.getElementById("tagorscript").setAttribute("reserved",reserved.value?"true":"false");
      document.getElementById("nameoftagorscript").value=reserved.value?"":mapsTo;
    }
  }
}    
    
function deleteAssignment()
{
  // find the relevant key
  var list = document.getElementById("currentkeyslist");
  var keyinfo = list.selectedItem.value;
  // parse the key info
  var alt = false;
  var ctrl = false;
  var shift = false;
  var meta = false;
  if (keyinfo.length == 0 || keyinfo[0] != "F"[0]) return;
  var num = parseInt(keyinfo.substr(1,keyinfo.indexOf("-")));
  if (isNaN(num) || num == 0) return;
  var keycode = 111 + num;
  var parts = keyinfo.split("-");
  var secondPart = parts[1];
  var i;
  if (secondPart.length > 0)
  {
    for (i = 0; i<secondPart.length; i++)
    {
      if (secondPart[i] == "a"[0]) alt = true; 
      if (secondPart[i] == "c"[0]) ctrl = true; 
      if (secondPart[i] == "s"[0]) shift = true; 
      if (secondPart[i] == "m"[0]) meta = true;
    } 
    keymapper.delKeyMapping("FKeys", keycode, alt, ctrl, shift, meta, true);
    keymapper.saveKeyMaps();
    list.removeItemAt(list.selectedIndex);
  }
}
    
 
function assignKey()
{
  return;
  // parse the contents of the text box with id="newkeys"
  var keyname = document.getElementById("newkeys").value;
  var alt = false;
  var ctrl = false;
  var shift = false;
  var meta = false;
  if (keyname.search(/Alt\+/) >= 0)
  {
    alt=true;
    keyname = keyname.Replace("Alt+","");
  }
  if (keyname.search(/Ctrl\+/) >= 0)
  {
    ctrl=true;
    keyname = keyname.Replace("Ctrl+","");
  }
  if (keyname.search(/Shift\+/) >= 0)
  {
    shift=true;
    keyname = keyname.Replace("Shift+","");
  }
  if (keyname.search(/Meta\+/) >= 0)
  {
    meta=true;
    keyname = keyname.Replace("Meta+","");
  }
  // now keyname should just be Fn
  if (keyname.length == 0 || keyname[0] != "F"[0]) return;
  var num = parseInt(keyname.substr(1));
  if (isNaN(num) || num == 0) return;
  var keycode = 111 + num;
  
  // get keycode, alt, ctrl, shift, meta
  var data;
  // get the radio value, either reservedforOS, keyscript, alltaglist
  var radiovalue = document.getElementById("keyresult").value;
  if (radiovalue == "reservedforOS")
  {
    data="reserved";
  }
  else data = document.getElementById("keyscript").value;
  keymapper.addScriptMapping("FKeys", keycode, alt, ctrl, shift, meta, true, data);
  keymapper.saveKeyMaps();
  startUp(); //reloads everything
}
  
   
