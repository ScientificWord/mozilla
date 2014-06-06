

var primary;
var prispec;
//var prispecapp;
var prispecInputEditor;
var secondary;
var secspec;
//var secspecapp;
var secspecInputEditor;
var tertiary;
var terspec;
//var terspecapp;
var terspecInputEditor;
var locator;
var format;
var thedeck;
var xreftext;
var node;
var isNewnode = false;
var activeEditor;
var gEditorsInitialized = 0;

function dumpln(s)
{
  dump(s+"\n");
}

function startup()
{
  var editorElement = msiGetParentEditorElementForDialog(window);
  activeEditor = msiGetEditor(editorElement);
  if (!activeEditor)
  {
    dump("Failed to get active editor!\n");
    window.close();
    return;
  }
  primary = document.getElementById("primary");
  prispec = document.getElementById("prispec");
//  prispecapp = document.getElementById("prispecapp");
  prispecInputEditor = document.getElementById("prispecappInput");
  secondary = document.getElementById("secondary");
  secspec = document.getElementById("secspec");
//  secspecapp = document.getElementById("secspecapp");
  secspecInputEditor = document.getElementById("secspecappInput");
  tertiary = document.getElementById("tertiary");
  terspec = document.getElementById("terspec");
//  terspecapp = document.getElementById("terspecapp");
  terspecInputEditor = document.getElementById("terspecappInput");
  locator = document.getElementById("locator");
  format = document.getElementById("format");
  thedeck = document.getElementById("thedeck");
  xreftext = document.getElementById("xref");

  var specnode;
  var prispecStr = "";
  var secspecStr = "";
  var terspecStr = "";
  var specKids;
  var serializer = new XMLSerializer();

  if (("arguments" in window) && (window.arguments.length))
    node = window.arguments[0];
  if (!node)
    node = getSelectionParentByTag(activeEditor,"indexitem");
  if (node)
  {
    if (node.hasAttribute("pri")) {
      primary.value = node.getAttribute("pri");
      prispec.disabled = primary.value.length == 0;
      specnode = node.getElementsByTagName("prispec")[0];
      if (specnode && specnode.textContent.length >0)
      { 
//        prispecapp.value = specnode.textContent;
        specKids = msiNavigationUtils.getSignificantContents(specnode);
        for (var jx = 0; jx < specKids.length; ++jx)
          prispecStr += serializer.serializeToString(specKids[jx]);
        prispec.checked = true;
      } else prispec.checked = false;
    }
    if (node.hasAttribute("sec")) {
      secondary.value = node.getAttribute("sec");
      secspec.disabled = secondary.value.length == 0;
      specnode = node.getElementsByTagName("secspec")[0];
      if (specnode && specnode.textContent.length >0) 
      {
//         secspecapp.value = specnode.textContent;
        specKids = msiNavigationUtils.getSignificantContents(specnode);
        for (var jx = 0; jx < specKids.length; ++jx)
          secspecStr += serializer.serializeToString(specKids[jx]);
        secspec.checked = true;
      } else secspec.checked = false;
    }
    if (node.hasAttribute("ter")) {
      tertiary.value = node.getAttribute("ter");
      terspec.disabled = tertiary.value.length == 0;
      specnode = node.getElementsByTagName("terspec")[0];
      if (specnode && specnode.textContent.length >0)
      { 
//        terspecapp.value = specnode.textContent;
        specKids = msiNavigationUtils.getSignificantContents(specnode);
        for (var jx = 0; jx < specKids.length; ++jx)
          terspecStr += serializer.serializeToString(specKids[jx]);
        terspec.checked = true;
      } else terspec.checked = false;
    }
    if (node.hasAttribute("enc") ){
       var enc=node.getAttribute("enc");
       var seepat=/see\{(.*)\}/;
       var result=seepat.exec(enc);
       if (result) {
          xreftext.value = result[1];
          locator.selectedIndex = 1;
          thedeck.selectedIndex = 1;
       } else {
          if (enc === "textbf") 
            format.selectedIndex = 1;
          else if (enc === "textit")
            format.selectedIndex =2;
          else 
            format.selectedIndex =0;
       }
    }

  }
  else {
    isNewnode = true;
    node = activeEditor.document.createElement("indexitem");
    prispec.checked = false;
    secspec.checked = false;
    terspec.checked = false;
  }

  var editorDocLoadedObserverData = {mCommand : "obs_documentCreated", mObserver : new msiIndexEditorChangeObserver(prispecInputEditor)};
  prispecInputEditor.mInitialDocObserver = [editorDocLoadedObserverData];
  editorDocLoadedObserverData = {mCommand : "obs_documentCreated", mObserver : new msiIndexEditorChangeObserver(secspecInputEditor)};
  secspecInputEditor.mInitialDocObserver = [editorDocLoadedObserverData];
  editorDocLoadedObserverData = {mCommand : "obs_documentCreated", mObserver : new msiIndexEditorChangeObserver(terspecInputEditor)};
  terspecInputEditor.mInitialDocObserver = [editorDocLoadedObserverData];

  var editorInitializer = new msiEditorArrayInitializer();
  editorInitializer.addEditorInfo(prispecInputEditor, prispecStr, true);
  editorInitializer.addEditorInfo(secspecInputEditor, secspecStr, true);
  editorInitializer.addEditorInfo(terspecInputEditor, terspecStr, true);
  editorInitializer.doInitialize();

//  prispecInputEditor.setAttribute("hidden",!prispec.checked);
//  secspecInputEditor.setAttribute("hidden",!secspec.checked);
//  terspecInputEditor.setAttribute("hidden",!terspec.checked);
}


function keypress(event, textbox, checkboxid)
{
  var cb = document.getElementById(checkboxid);
  if (cb) {
    cb.disabled = (textbox.value.length == 0);
    if (cb.disabled)
      msiEnableEditorControl(document.getElementById(checkboxid+"appInput"), false);
//      document.getElementById(checkboxid+"appInput").disabled = true;
//      document.getElementById(checkboxid+"appInput").setAttribute("hidden","true");
    else
      msiEnableEditorControl(document.getElementById(checkboxid+"appInput"), cb.checked);
//      document.getElementById(checkboxid+"appInput").disabled = (!cb.checked);
//      document.getElementById(checkboxid+"appInput").setAttribute("hidden",!(cb.checked));
  }
}


function checkboxchanged(event, checkbox, appearanceid)
{
//  document.getElementById(appearanceid).disabled = !(checkbox.checked);
  msiEnableEditorControl(document.getElementById(appearanceid), checkbox.checked);
//  document.getElementById(appearanceid).setAttribute("hidden",!(checkbox.checked));
}

function locatorchange(event, radiogroup)
{
  if (radiogroup.selectedIndex == 0) thedeck.selectedIndex = 0;
  else thedeck.selectedIndex = 1;
}

function stop()
{
  var x = 3;
  dumpln(document.getElementById("primary").value);
  dumpln(prispec.checked);
//  dumpln(prispecapp.value);
  dumpln(secondary.value);
  dumpln(secspec.checked);
//  dumpln(secspecapp.value);
  dumpln(tertiary.value);
  dumpln(terspec.checked);
//  dumpln(terspecapp.value);
  dumpln(xreftext.value);
}


function onAccept()
{
  activeEditor.beginTransaction();
  msiEditorEnsureElementAttribute(node, "req", "varioref", activeEditor);
  var v = primary.value;
  msiEditorEnsureElementAttribute(node, "pri", v, activeEditor);

  v = secondary.value;
  msiEditorEnsureElementAttribute(node, "sec", v, activeEditor);
  v = tertiary.value;
  msiEditorEnsureElementAttribute(node, "ter", v, activeEditor);

  if (node.parentNode) dump(node.parentNode.innerHTML+"\n");
  else dump(node.innerHTML+"\n");

  if (isNewnode)
    activeEditor.insertElementAtSelection(node, true);

  // remove subnodes, if any
  while(node.firstChild) 
    activeEditor.deleteNode(node.firstChild);
    //node.removeChild(node.firstChild);
  var specContentFilter;
  var hasPriSpec = "";
  if (prispec.checked) {
    specContentFilter = new msiDialogEditorContentFilter(prispecInputEditor);
    v = specContentFilter.getDocumentFragmentString();
//      dump("In indexentry dialog, pri special appearance is [" + v + "].\n");
    var prispecnode;
    if (v && v.length > 0)
    {
      prispecnode = activeEditor.document.createElement("prispec");
      activeEditor.insertNode(prispecnode, node, node.childNodes.length);
      prispecnode = node.getElementsByTagName("prispec")[0];
//    prispecnode.textContent = v;
      activeEditor.insertHTMLWithContext(v, "", "", "", null, prispecnode, 0, false);
      hasPriSpec = "p";
    }
  }
//  v = secspecapp.value;
  if (secspec.checked) {
    specContentFilter = new msiDialogEditorContentFilter(secspecInputEditor);
    v = specContentFilter.getDocumentFragmentString();
//      dump("In indexentry dialog, sec special appearance is [" + v + "].\n");
    var secspecnode;
    if (v && v.length > 0)
    {
      secspecnode = activeEditor.document.createElement("secspec");
      activeEditor.insertNode(secspecnode, node, node.childNodes.length);
//      secspecnode.textContent = v;
      secspecnode = node.getElementsByTagName("secspec")[0];
      activeEditor.insertHTMLWithContext(v, "", "", "", null, secspecnode, 0, false);
      hasPriSpec += " s";
    }
  }
//  v = terspecapp.value;
  if (terspec.checked) {
    specContentFilter = new msiDialogEditorContentFilter(terspecInputEditor);
    v = specContentFilter.getDocumentFragmentString();
    var terspecnode;
    if (v && v.length > 0)
    {
      terspecnode = activeEditor.document.createElement("terspec");
      activeEditor.insertNode(terspecnode, node, node.childNodes.length);
//      terspecnode.textContent = v;
      terspecnode = node.getElementsByTagName("terspec")[0];
      activeEditor.insertHTMLWithContext(v, "", "", "", null, terspecnode, 0, false);
      hasPriSpec += " t";
    }
  }
  hasPriSpec = TrimString(hasPriSpec);
  if (hasPriSpec.length > 0)
    msiEditorEnsureElementAttribute(node, "specAppearance", hasPriSpec, activeEditor);
  else
    msiEditorEnsureElementAttribute(node, "specAppearance", null, activeEditor);

  var enc;
  var loc = locator.selectedIndex; // 0 = page num, 1 = xref
  var fmt = format.selectedIndex;
  if (loc === 1) {
     enc = "see{" + xreftext.value + "}";
  } else {
     if (fmt === 0)
       enc = "";
     else if (fmt === 1)
       enc = "textbf";
     else
       enc = "textit";
  }
  if (enc === "" && node.hasAttribute("enc"))
    node.removeAttribute("enc");
  else
    msiEditorEnsureElementAttribute(node, "enc", enc, activeEditor);

  activeEditor.endTransaction();
}


function onCancel()
{
  return true;
}

function msiIndexEditorChangeObserver(editorElement)
{
  this.mEditorElement = editorElement;
  this.observe = function(aSubject, aTopic, aData)
  {
    switch(aTopic)
    {
//      case "cmd_bold":
//      case "cmd_setDocumentModified":
//      {
//        msiDumpWithID("In indexentry command observer [" + aTopic + "] for editor [@]; calling doEnabling().\n", this.mEditorElement);
//        doEnabling();
//      }
//      break;

      case "obs_documentCreated":
      {
        var bIsRealDocument = false;
        var currentURL = msiGetEditorURL(this.mEditorElement);
        msiDumpWithID("In indexentry documentCreated observer for editor element [@], currentURL is " + currentURL + "].\n", this.mEditorElement);
        if (currentURL != null)
        {
          var fileName = GetFilename(currentURL);
          bIsRealDocument = (fileName != null && fileName.length > 0);
        }
        if (bIsRealDocument)
        {
          var checkboxID;
          var editorFlag = 0;
          switch(this.mEditorElement.id)
          {
            case "prispecappInput":
              checkboxID = "prispec";
              editorFlag = 1;
            break;
            case "secspecappInput":
              checkboxID = "secspec";
              editorFlag = 2;
            break;
            case "terspecappInput":
              checkboxID = "terspec";
              editorFlag = 4;
            break;
            default:
            break;
          }
          if (checkboxID && checkboxID.length)
          {
            var checkbox = document.getElementById(checkboxID);
//            if (!checkbox.checked)
            dump("In indexentry dialog, disabling editor " + this.mEditorElement.id + "\n");
            msiEnableEditorControl(this.mEditorElement, checkbox.checked);
//              this.mEditorElement.disabled = true;
//              this.mEditorElement.setAttribute("hidden", "true");
          }
          gEditorsInitialized |= editorFlag;
        }
//        else
          msiDumpWithID("In indexentry documentCreated observer for editor [@], bIsRealDocument is false.\n", this.mEditorElement);
//        setControlsForSubstitution();
      }
      break;
    }
  };
}
