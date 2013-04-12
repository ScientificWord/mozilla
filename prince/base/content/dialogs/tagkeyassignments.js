var keymapper;

function clear(event)
{
  document.getElementById("newkeys").value = "";
  event.preventDefault();
}

function clearInputs()
{
// clear the values of the script input box and the tag selection
  document.getElementById("keyresult").selectedIndex = 0;
  document.getElementById("alltaglist").value = "";
  document.getElementById("alltaglist").disabled = false;
  document.getElementById("keyscript").value = "";
  document.getElementById("keyscript").disable = true;
  
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
  clearInputs();
  var display="";
  var keycode = "F"+(event.keyCode - event.DOM_VK_F1 +1);
  if (event.shiftKey) display += "Shift+";
  if (event.ctrlKey) display += "Ctrl+";
  if (event.altKey) display += "Alt+";
  if (event.metaKey) display += "Meta+";
  display += keycode;
  // look up the key to see if there is a current assignment
  var reserved = new Boolean(false);
  reserved.value = false;
  var mapsTo = keymapper.mapKeyToScript("FKeys", event.keyCode, event.altKey, event.ctrlKey, event.shiftKey, event.metaKey, true, reserved);
  if (reserved.value)
  {
    document.getElementById("keyresult").selectedIndex = 2;
  }
  if (mapsTo.length >0)
  {
    if (mapsTo.search(/toggleTextTag|insertParaTag|insertSectionTag|insertTag/) >= 0)
    {
      var myArray = mapsTo.match("'([a-zA-Z()]+)'");
      if (myArray != null)
      {
        document.getElementById("keyresult").selectedIndex = 0;
        document.getElementById("alltaglist").value=myArray[1];
        document.getElementById("keyscript").disabled=true;
      }
    }
    else
    {
      if (!reserved.value) document.getElementById("keyresult").selectedIndex = 1;
      document.getElementById("alltaglist").disabled=true;
    }
  }  
  document.getElementById("keyscript").value = mapsTo;
      
  
  // F1 and F10 are reserved, plain and shifted
//   if ((key==event.DOM_VK_F1 || key==event.DOM_VK_F10) &&
//     !(event.ctrlKey | event.altKey | event.metaKey))
//   {
//     AlertWithTitle("Reserved function key","The "+display+" key is reserved");
//   }
//   else
  document.getElementById("newkeys").value = display;
  document.getElementById("assignkey").disabled = display.length > 0?false:true;
  var templateStrings = document.getElementById("templateStrings");
  var assignTemplate = templateStrings.getString("tagKeyAssignmentDlg.assignTemplate");
//  document.getElementById("assignkey").label = "Assign action to "+display;
  document.getElementById("assignkey").label = assignTemplate.replace(/%keyDescriptor%/, display);
  event.preventDefault();
}

function radioswitched( event, obj )
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

function doTagListMouseDown( event )
{
  var tagListBox = document.getElementById("alltaglist");
  var theDropMarker = document.getAnonymousElementByAttribute(tagListBox, "anonid", "historydropmarker");
  var initObj = event.originalTarget;
  if (initObj == theDropMarker)
  {
    if (!tagListBox.disabled && ("showPopup" in theDropMarker))
      theDropMarker.showPopup();
    event.stopPropagation();
    event.preventDefault();
  }
}

function startUp()
{
  document.getElementById("alltaglist").height = document.getElementById("keyscript").height;
 // Build the list of key combinations that have been assigned values or have been reserved
  clearInputs();
  keymapper =  Components.classes["@mackichan.com/keymap;1"]
                       .createInstance(Components.interfaces.msiIKeyMap);
  var strOfKeys = keymapper.getTableKeys("FKeys");
  var keylist = strOfKeys.split(" ");
  var currKey;
  var firstPart, secondPart;
  var len = keylist.length;
  var i,j;   
  var keyParts;
  var popup = document.getElementById("currentkeyslist");
  var cnt = popup.getRowCount();
  for (j=cnt-1; j>= 0; j--)
    popup.removeItemAt(j);
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

////Debugging stuff:
  var theTagList = document.getElementById("alltaglist");
//  var theDropMarker = document.getAnonymousElementByAttribute(theTagList, "anonid", "historydropmarker");
  theTagList.addEventListener("mousedown", doTagListMouseDown, true);
//  var logStr = "The taglist has popup [" + theTagList.popup + "]; it has historydropmarker [";
//  if (theDropMarker)
//  {
//    logStr += theDropMarker + "]. Its members are:\n";
//    for (var aProp in theDropMarker)
//    {
//      logStr += "  " + aProp + ":      [" + theDropMarker[aProp] + "]\n";
//    } 
//  }
//  else
//    logStr += "void]";
//  dump(logStr + "\n");
}

function assignTag( obj )
{
  var editorElement = msiGetParentEditorElementForDialog(window);
  var editor = msiGetEditor(editorElement);
  var tag = obj.value;
  var tagmanager = editor.tagListManager;
  var command="";
  var data="";
  var tagclass = tagmanager.getRealClassOfTag(tag,null);
  if (tagclass == "texttag")
    data = "toggleTextTag('"+tag+"',false);";
  else if (tagclass == "paratag")
    data = "insertParaTag('"+tag+"',false);";
  else if (tagclass == "structtag")
    data = "insertSectionTag('"+tag+"',false);";
  else
    data = "insertTag('"+tag+"',false);";
  document.getElementById("keyscript").value = data;
  
}
  

function selectCurrentKey( amenulist )
{
  // this code strongly depends on the fact that the only keys allowed are function keys
  if (!amenulist) amenulist=document.getElementById("currentkeyslist");
  var keycodes = amenulist.value;
  if (!keycodes) return;
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
    // now if this is a command to change a tag, put the tag in the other text box and set the radio button
    // this needs to be made more robust. My regular expressions for finding the whole pattern didn't work.
    var button = document.getElementById("deleteassignment");
    button.disabled = false;
    var templateStrings = document.getElementById("templateStrings");
    var delTemplate = templateStrings.getString("tagKeyAssignmentDlg.deleteTemplate");
    button.label = delTemplate.replace(/%keyDescriptor%/, amenulist.selectedItem.label);
//    button.label = "Delete assignment for " + amenulist.selectedItem.label;
    if (mapsTo.search(/toggleTextTag|insertParaTag|insertSectionTag|insertTag/) >= 0)
    {
      var myArray = mapsTo.match("'([a-zA-Z()]+)'");
      if (myArray != null)
      {
//        document.getElementById("tagorscript").value = "Key assigns the tag";
        document.getElementById("tagorscript").value = document.getElementById("tagorscriptTag").value;
        document.getElementById("nameoftagorscript").value=myArray[1];
        document.getElementById("tagorscript").setAttribute("reserved","false");
        //document.getElementById("keyresult").selectedIndex = 0;
      }
    }
    else
    {
      if (reserved.value)
      {
        document.getElementById("tagorscript").value = document.getElementById("tagorscriptReserved").value;
        document.getElementById("tagorscript").setAttribute("reserved", "true");
        document.getElementById("nameoftagorscript").value = "";
      }
      else
      {
        document.getElementById("tagorscript").value = document.getElementById("tagorscriptScript").value;
        document.getElementById("tagorscript").setAttribute("reserved", "false");
        document.getElementById("nameoftagorscript").value = mapsTo;
      }
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
//    keymapper.saveKeyMaps();
    list.removeItemAt(list.selectedIndex);
  }
  selectCurrentKey(); // updates the other list box if necessary
}
    
 
function assignKey()
{
//  return;
  // parse the contents of the text box with id="newkeys"
  var keyname = document.getElementById("newkeys").value;
  var alt = false;
  var ctrl = false;
  var shift = false;
  var meta = false;
  if (keyname.search(/Alt\+/) >= 0)
  {
    alt=true;
    keyname = keyname.replace("Alt+","");
  }
  if (keyname.search(/Ctrl\+/) >= 0)
  {
    ctrl=true;
    keyname = keyname.replace("Ctrl+","");
  }
  if (keyname.search(/Shift\+/) >= 0)
  {
    shift=true;
    keyname = keyname.replace("Shift+","");
  }
  if (keyname.search(/Meta\+/) >= 0)
  {
    meta=true;
    keyname = keyname.replace("Meta+","");
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
//  keymapper.saveKeyMaps();
  selectCurrentKey(); // updates the other list box if necessary
  startUp(); //reloads everything
}
  
function onAccept()
{
  keymapper.saveKeyMaps();
//  acceptDialog();
  return true;
}   
