var keymapper = null;


function clear(event) {
    document.getElementById("newkeys").value = "";
    event.preventDefault();
}

function clearInputs() {
    // clear the values of the script input box and the tag selection
    document.getElementById("keyresult").selectedIndex = 0;
    document.getElementById("alltaglist").value = "";
    // document.getElementById("alltaglist").disable=false;
    document.getElementById("keyscript").value = "";
    // document.getElementById("keyscript").disable = true;

}

function handleTagListCharacter(event, obj) {
  var key = event.keyCode;
  if (key && (key === event.DOM_VK_ENTER || key === event.DOM_VK_RETURN)) {
    assignTag(obj);
    event.preventDefault();
    document.getElementById("tagkeyassignmentsDlg").getButton("cancel").focus();
  }
}

function tagToScript(data) { //Given a tag name, return the corresponding script
  var editorElement = msiGetParentEditorElementForDialog(window);
  var editor = msiGetEditor(editorElement);
  var tagmanager = editor.tagListManager;
  var tagclass = tagmanager.getRealClassOfTag(data, null);
  var script = '';
  if (tagclass === "texttag")
    script = "toggleTextTag('" + data + "',false);";
  else if (tagclass === "paratag")
    script = "insertParaTag('" + data + "',false);";
  else if (tagclass === "structtag")
    script = "insertSectionTag('" + data + "',false);";
  else if (tagclass === "envtag")
    script = "insertSectionTag('" + data + "',false);";
  else if (tagclass === "listtag")
    script = "insertListItem('" + data + "',false);";
  else if (tagclass === "frontmtag")
    script = "insertFrontMatterItem('" + data + "',false);";
  else if (/^\s*\(?[a-zA-Z]+\)?\s*$/.test(data)) 
      script = "insertTag('" + data + "',false);";
  return script;    
}

function scriptToTag(script) {  //Given a script, see if it just applies a tag. If so, return the tag; else return ''
  var searchRE = /^\s*(toggleTextTag|insertParaTag|insertSectionTag|insertTag|insertListItem|insertFrontMatterItem)/;
  var tag = '';
  var match;
  if (searchRE.test(script)) {  //starts with one of the above functions
    match= script.match("'([a-zA-Z()_0-9]+)'");
  }
  if (match)
    tag = match[1];
  return tag;
}

function longKeyToShort(keyname) { // takes long key name, e.g. F7+Shift+Alt, to short form, e.g. F7-sav
  var shift, alt, ctrl, meta;
  var functionkeyname = '';
  var match;
  shift = /\bShift\b/.test(keyname);
  alt = /\bAlt\b/.test(keyname);
  ctrl = /\Ctrl\b/.test(keyname);
  meta = /\Meta\b/.test(keyname);
  match = /F\d(\d)?\s*$/.exec(keyname);
  if (match) {
    functionkeyname = match[0];
  }
  return functionkeyname + '-' + (alt?'a':'') + (ctrl?'c':'') + (shift?'s':'') + (meta?'m':'') + 'v';
}

function buildKeysMenuFromStringOfKeys(strOfKeys) {
  var keylist = strOfKeys.split(" ");
  var currKey;
  var firstPart, secondPart;
  var len = keylist.length;
  var i, j;
  var keyParts;
  var popup = document.getElementById("currentkeyslist");
  var cnt = popup.getRowCount();
  for (j = cnt - 1; j >= 0; j--)
      popup.removeItemAt(j);
  for (i = 0; i < len; i++) {
      currKey = keylist[i];
      if (currKey.length == 0) break;
      if (currKey[0] == "-" [0]) {
          firstPart = "-";
          if (currKey[1] == "-" [0])
              secondPart = currKey.substr(2);
          else
              secondPart = "";
          secondPart = currKey[2];
      } else {
          keyParts = currKey.split("-");
          firstPart = keyParts[0];
          secondPart = keyParts[1];
      }
      var menuLabel = "";
      var j;
      for (j = 0; j < secondPart.length; j++) {
          if (secondPart[j] == "s") menuLabel += "Shift+";
          if (secondPart[j] == "a") menuLabel += "Alt+";
          if (secondPart[j] == "c") menuLabel += "Ctrl+";
          if (secondPart[j] == "m") menuLabel += "Meta+";
      }
      menuLabel += firstPart;
      var itemNode = document.createElementNS(XUL_NS, "listitem");
      itemNode.setAttribute("label", menuLabel);
      itemNode.setAttribute("value", currKey);
      popup.appendChild(itemNode);
  }  
}

function handleChar(event, object) // called when the user sets a function key combination
{
    var key = event.keyCode;
    if (key == event.DOM_VK_TAB) {
        document.getElementById("taglistradio").focus();
        return;
    }
    if (key < event.DOM_VK_F1) {
        clear(event);
        return;
    }
    if (key > event.DOM_VK_F24) {
        clear(event);
        return;
    }
    clearInputs();
    var display = "";
    var keycode = "F" + (event.keyCode - event.DOM_VK_F1 + 1);
    if (event.altKey) display  += "Alt+";
    if (event.ctrlKey) display += "Ctrl+";
    if (event.shiftKey) display += "Shift+";
    if (event.metaKey) display += "Meta+";
    display += keycode;
    // look up the key to see if there is a current assignment
    var reserved = new Boolean(false);
    // reserved.value = false;
    var mapsTo = keymapper.mapKeyToScript("FKeys", event.keyCode, event.altKey, event.ctrlKey, event.shiftKey, event.metaKey, true, reserved);
    var tag = scriptToTag(mapsTo);
    if (tag.length > 0) { //display the tag name
      document.getElementById("keyresult").selectedIndex = 0;
      document.getElementById("alltaglist").value = tag;
      document.getElementById("alltaglist").disabled=false;
      // document.getElementById("keyscript").value = '';
      document.getElementById("keyscript").disabled=true;
    }
    else { // appears to be a script
      document.getElementById("keyresult").selectedIndex = 1;
      // document.getElementById("alltaglist").value = '';
      document.getElementById("alltaglist").disabled = true;
      document.getElementById("keyscript").value = mapsTo;
      document.getElementById("keyscript").disabled = false;
    }


    // F1 and F10 are reserved, plain and shifted
    //   if ((key==event.DOM_VK_F1 || key==event.DOM_VK_F10) &&
    //     !(event.ctrlKey | event.altKey | event.metaKey))
    //   {
    //     AlertWithTitle("Reserved function key","The "+display+" key is reserved");
    //   }
    //   else
    document.getElementById("newkeys").value = display;
    //  document.getElementById("assignkey").disabled = display.length > 0?false:true;
    var templateStrings = document.getElementById("templateStrings");
    var assignTemplate = templateStrings.getString("tagKeyAssignmentDlg.assignTemplate");
    //  document.getElementById("assignkey").label = "Assign action to "+display;
    //  document.getElementById("assignkey").label = assignTemplate.replace(/%keyDescriptor%/, display);
    event.preventDefault();
}

function radioswitched(event, obj) {
    var taglisttextbox = document.getElementById("alltaglist");
    var keyscript = document.getElementById("keyscript");
    if (obj.value == 'alltaglist') {
        taglisttextbox.disabled=false;
        keyscript.disabled=true;
    } else {
        taglisttextbox.disabled=true;
        keyscript.disabled=false;
    }
}

function doTagListMouseDown(event) {
    var tagListBox = document.getElementById("alltaglist");
    var theDropMarker = document.getAnonymousElementByAttribute(tagListBox, "anonid", "historydropmarker");
    var initObj = event.originalTarget;
    if (initObj == theDropMarker) {
        if (!tagListBox.disabled && ("showPopup" in theDropMarker))
            theDropMarker.showPopup();
        event.stopPropagation();
        event.preventDefault();
    }
}

function startUp() {
  document.getElementById("alltaglist").height = document.getElementById("keyscript").height;
  // Build the list of key combinations that have been assigned values or have been reserved
  clearInputs();
  if (keymapper == null) keymapper = Components.classes["@mackichan.com/keymap;1"]
      .createInstance(Components.interfaces.msiIKeyMap);
  var strOfKeys = keymapper.getTableKeys("FKeys");
  buildKeysMenuFromStringOfKeys(strOfKeys);
  document.getElementById("taglistradio").disabled=false;
  document.getElementById("scriptradio").disabled=false;
  document.getElementById("alltaglist").disabled=false;
  document.getElementById("keyscript").disabled=false;
  ////Debugging stuff:
  var theTagList = document.getElementById("alltaglist");
  theTagList.addEventListener("mousedown", doTagListMouseDown, true);
}

function assignTag(obj) {
  var tagOrScript = obj.value;
  if (obj.id === "alltaglist") {
    document.getElementById("alltaglist").value = tagOrScript;
  }
  assignKey(obj.id === 'keyscript', tagOrScript);
}


function selectCurrentKey(amenulist) {
    // this code strongly depends on the fact that the only keys allowed are function keys
    if (!amenulist) amenulist = document.getElementById("currentkeyslist");
    var keycodes = amenulist.value;
    if (!keycodes) return;
    var alt = false;
    var ctrl = false;
    var shift = false;
    var meta = false;
    var tag = '';
    var reserved = new Boolean(false);  // not used, but required for call to mapKeyToScript
    if (keycodes.length == 0 || keycodes[0] != "F" [0]) return;
    var num = parseInt(keycodes.substr(1, keycodes.indexOf("-")));
    if (isNaN(num) || num == 0) return;
    var keycode = 111 + num;
    var parts = keycodes.split("-");
    var secondPart = parts[1];
    var i;
    if (secondPart.length > 0) {
        for (i = 0; i < secondPart.length; i++) {
            if (secondPart[i] == "a" [0]) alt = true;
            if (secondPart[i] == "c" [0]) ctrl = true;
            if (secondPart[i] == "s" [0]) shift = true;
            if (secondPart[i] == "m" [0]) meta = true;
        }
        var mapsTo = keymapper.mapKeyToScript("FKeys", keycode, alt, ctrl, shift, meta, true, reserved);
        // now if this is a command to change a tag, put the tag in the other text box and set the radio button
        // this needs to be made more robust. My regular expressions for finding the whole pattern didn't work.
        var button = document.getElementById("deleteassignment");
        button.disabled = false;
        var templateStrings = document.getElementById("templateStrings");
        var delTemplate = templateStrings.getString("tagKeyAssignmentDlg.deleteTemplate");
        button.label = delTemplate.replace(/%keyDescriptor%/, amenulist.selectedItem.label);
        //    button.label = "Delete assignment for " + amenulist.selectedItem.label;
        tag = scriptToTag(mapsTo);
        if (tag.length > 0)  {
           //        document.getElementById("tagorscript").value = "Key assigns the tag";
           document.getElementById("tagorscript").value = document.getElementById("tagorscriptTag").value;
           document.getElementById("nameoftagorscript").value = tag;
                // document.getElementById("tagorscript").setAttribute("reserved", "false");
                //document.getElementById("keyresult").selectedIndex = 0;

        } else {

          document.getElementById("tagorscript").value = document.getElementById("tagorscriptScript").value;
          document.getElementById("nameoftagorscript").value = mapsTo;
        }
    }
}

function deleteAssignment() {
    // find the relevant key
    var list = document.getElementById("currentkeyslist");
    var keyinfo = list.selectedItem.value;
    // parse the key info
    var alt = false;
    var ctrl = false;
    var shift = false;
    var meta = false;
    if (keyinfo.length == 0 || keyinfo[0] != "F" [0]) return;
    var num = parseInt(keyinfo.substr(1, keyinfo.indexOf("-")));
    if (isNaN(num) || num == 0) return;
    var keycode = 111 + num;
    var parts = keyinfo.split("-");
    var secondPart = parts[1];
    var i;
    if (secondPart.length > 0) {
        for (i = 0; i < secondPart.length; i++) {
            if (secondPart[i] == "a" [0]) alt = true;
            if (secondPart[i] == "c" [0]) ctrl = true;
            if (secondPart[i] == "s" [0]) shift = true;
            if (secondPart[i] == "m" [0]) meta = true;
        }
        keymapper.delKeyMapping("FKeys", keycode, alt, ctrl, shift, meta, true);
        //    keymapper.saveKeyMaps();
        list.removeItemAt(list.selectedIndex);
    }
 //   selectCurrentKey(); // updates the other list box if necessary
}


function assignKey(isScript, data) {
    var tagOrScript = isScript?data:tagToScript(data);
    // parse the contents of the text box with id="newkeys"
    var keyname = document.getElementById("newkeys").value;

    var alt = false;
    var ctrl = false;
    var shift = false;
    var meta = false;
    var shortName = longKeyToShort(keyname);
    if (keyname.search(/Alt\+/) >= 0) {
        alt = true;
        keyname = keyname.replace("Alt+", "");
    }
    if (keyname.search(/Ctrl\+/) >= 0) {
        ctrl = true;
        keyname = keyname.replace("Ctrl+", "");
    }
    if (keyname.search(/Shift\+/) >= 0) {
        shift = true;
        keyname = keyname.replace("Shift+", "");
    }
    if (keyname.search(/Meta\+/) >= 0) {
        meta = true;
        keyname = keyname.replace("Meta+", "");
    }
    // now keyname should just be Fn
    if (keyname.length == 0 || keyname[0] != "F" [0]) return;
    var num = parseInt(keyname.substr(1));
    if (isNaN(num) || num == 0) return;
    var keycode = 111 + num;

    // get keycode, alt, ctrl, shift, meta
    // get the radio value, either keyscript, alltaglist

    keymapper.addScriptMapping("FKeys", keycode, alt, ctrl, shift, meta, true, tagOrScript);
    // if (isScript) {
    //   document.getElementById('alltaglist').value = '';
    // }
    // else {
    //   document.getElementById('keyscript').value = '';
    // }
    var strOfKeys = keymapper.getTableKeys("FKeys");
    buildKeysMenuFromStringOfKeys(strOfKeys);
    // document.getElementById('currentkeyslist').currentIndex = -1;
    document.getElementById('currentkeyslist').value = shortName;
    // selectCurrentKey(); // updates the other list box if necessary
    //    startUp(); //reloads everything
}

function onAccept() {
    keymapper.saveKeyMaps();
    startUp();  // refresh
    return false;  // Do not exit dialog 
}

function onCancel() {
  return true;
}

