
Components.utils.import("resource://app/modules/msiEditorDefinitions.jsm");

var markerNode;
var markerKeyList;
var validkey;
var initialKeyValue = '';

function initKeyEntryOverlay(activeEditor, markerNode)
{
  validkey = false;
  // if (!markerNode)
  //   markerNode = getSelectionParentByTag(activeEditor, "a");
  if (markerNode)
  {
    if (markerNode.hasAttribute("key")) {
      initialKeyValue = markerNode.getAttribute("key");
      document.getElementById("keylist").value = initialKeyValue;
    }
    else if (markerNode.hasAttribute("name")) 
      document.getElementById("keylist").value = markerNode.getAttribute("name");
    else  if (markerNode.hasAttribute("id"))
      document.getElementById("keylist").value = markerNode.getAttribute("id");
  }
  initKeyList();
}
  
function markerlist()
{
  return markerKeyList.mKeyListManager.mDocumentArrays.StandardLATE.markerList;
}

function tagConflicts()
{
  var keyList = document.getElementById("keylist");
  var badkeys = {};
  var listOfKeys;
  var index;
  var currentName = keyList.value;
  var isInitialValue = (currentName === initialKeyValue);
  var end = currentName.length;
  if (!isInitialValue) {
    listOfKeys = markerlist();
    index = listOfKeys.indexOf(currentName);
    if (index >= 0) {
      document.getElementById("uniquekeywarning").hidden = false;
      document.getElementById("invalidkey").hidden = true;
      document.getElementById("badkeychar").value = '';
      document.getElementById("badkeychar").hidden = true;
      keyList.focus();
      keyList.setSelectionRange(end, end);
      // document.getElementById('markers').getButton('accept').setAttribute('disabled', 'true');
      validkey = false;
      return true;
    }
    // document.getElementById('markers').getButton('accept').removeAttribute('disabled');

    if (!isValidKey(currentName, badkeys)) {
      document.getElementById("uniquekeywarning").hidden = true;
      document.getElementById("invalidkey").hidden = false;
      document.getElementById("badkeychar").value = badkeys.value;
      document.getElementById("badkeychar").hidden = false;

      keyList.focus();
      keyList.setSelectionRange(end, end);

      // document.getElementById('markers').getButton('accept').setAttribute('disabled', 'true');
      validkey = false;
      return true;
    }
  }

  document.getElementById("uniquekeywarning").hidden = true;
  document.getElementById("invalidkey").hidden = true;
  document.getElementById("badkeychar").value = '';
  document.getElementById("badkeychar").hidden = true;
  // document.getElementById('markers').getButton('accept').removeAttribute('disabled');
  validkey = true;
  return false;
}

function saveKey(markerNode) 
{
  var keyList = document.getElementById("keylist");
  var currentName = keyList.value;
  markerNode.setAttribute("name", currentName);
  markerNode.setAttribute("key", currentName);
  markerNode.setAttribute("id", currentName);
}



function initKeyList()
{
  markerKeyList = new msiKeyMarkerList(window);
  markerKeyList.setUpTextBoxControl(document.getElementById("keylist"));
}
