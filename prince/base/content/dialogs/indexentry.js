

var primary;
var prispec;
var prispecInputEditor;
var secondary;
var secspec;
var secspecInputEditor;
var tertiary;
var terspec;
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

function copyNodeContents(destNode, destOffset, srcNode, editorElement)
{
  var editor = msiGetEditor(editorElement);
  try {
    var dupNode = srcNode.cloneNode(true);  // deep clone
    var destChild = destNode;
    var inserted;
    destChild = destChild.firstChild;
    inserted = destNode.insertBefore(dupNode, destChild);
    editor.removeContainer(inserted);
  }
  catch( e ) {
    dumpln(e.message);
  }
}

function appendNodeContents(destNode, srcNode){
  var node = srcNode.firstChild;
  while (node) {
    destNode.appendChild(node);
    node = node.nextSibling;
  }
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
  prispecInputEditor = document.getElementById("prispecInputEditor");
  secondary = document.getElementById("secondary");
  secspec = document.getElementById("secspec");
//  secspecapp = document.getElementById("secspecapp");
  secspecInputEditor = document.getElementById("secspecInputEditor");
  tertiary = document.getElementById("tertiary");
  terspec = document.getElementById("terspec");
//  terspecapp = document.getElementById("terspecapp");
  terspecInputEditor = document.getElementById("terspecInputEditor");
  locator = document.getElementById("locator");
  format = document.getElementById("format");
  thedeck = document.getElementById("thedeck");
  xreftext = document.getElementById("xref");

  var specnode;
  var destNode;
  var origNode;
  var initStr = ''; //getFileAsString("chrome://prince/content/StdDialogShell.xhtml");
  var prispecStr = '';
  var secspecStr = '';
  var terspecStr = '';
  var specKids;
  var serializer = new XMLSerializer();
  var editors = [prispecInputEditor, secspecInputEditor, terspecInputEditor];
  var editorDocLoadedObserverData;
  var editorInitializer = new msiEditorArrayInitializer();

  if (("arguments" in window) && (window.arguments.length))
    origNode = window.arguments[0];
  if (!origNode)
    origNode = getSelectionParentByTag(activeEditor,"indexitem");
  if (!origNode) {
    isNewnode = true;
    node = activeEditor.document.createElement("indexitem");
  } else {
    node = origNode.cloneNode(true);
  }
  if (node)
  {
    editors.forEach(function(editor){
      editorDocLoadedObserverData = {mCommand : "obs_documentCreated", mObserver : new msiIndexEditorChangeObserver(editor)};
      editor.mInitialDocObserver = [editorDocLoadedObserverData];
      editorInitializer.addEditorInfo(editor, '', true);
    });
    editorInitializer.doInitialize();


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
}


function keypress(event, textbox, checkboxid)
{
  var cb = document.getElementById(checkboxid);
  if (cb) {
    cb.disabled = (textbox.value.length == 0);
    if (cb.disabled)
      msiEnableEditorControl(document.getElementById(checkboxid+"InputEditor"), false);
//      document.getElementById(checkboxid+"appInput").disabled = true;
//      document.getElementById(checkboxid+"appInput").setAttribute("hidden","true");
    else
      msiEnableEditorControl(document.getElementById(checkboxid+"InputEditor"), cb.checked);
//      document.getElementById(checkboxid+"appInput").disabled = (!cb.checked);
//      document.getElementById(checkboxid+"appInput").setAttribute("hidden",!(cb.checked));
  }
}


function checkboxchanged(event, checkbox, appearanceid)
{
//  document.getElementById(appearanceid).disabled = !(checkbox.checked);
  msiEnableEditorControl(document.getElementById(appearanceid), checkbox.checked);
  // document.getElementById(appearanceid).hidden = !(checkbox.checked);
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
  var priSpecNode;
  var secSpecNode;
  var terSpecNode;
  var srcNode;
  var hasPriSpec;
  var hasSecSpec;
  var hasTerSpec;

  // activeEditor.beginTransaction();
  if (!node) return 0;
  node.setAttribute("req", "varioref");
  var v = primary.value;
  node.setAttribute("pri", v);
  v = secondary.value;
  node.setAttribute("sec", v);
  v = tertiary.value;
  node.setAttribute("ter", v);

  // remove subnodes, if any
  while(node.firstChild) {
    node.firstChild.removeChild();
  }

  if (prispec.checked) {
    priSpecNode = activeEditor.document.createElement("prispec");
    node.appendChild(priSpecNode);
    srcNode = prispecInputEditor.contentDocument.documentElement.getElementsByTagName('dialogbase')[0];
    if (srcNode) {
      appendNodeContents(priSpecNode, srcNode);
    }
    hasPriSpec = "p";
  }
  if (secspec.checked) {
    secSpecNode = activeEditor.document.createElement("secspec");
    node.appendChild(secSpecNode);
    srcNode = secspecInputEditor.contentDocument.documentElement.getElementsByTagName('dialogbase')[0];
    if (srcNode) {
      appendNodeContents(secSpecNode, srcNode);
    }
    hasPriSpec += " s";
  }
  if (terspec.checked) {
    terSpecNode = activeEditor.document.createElement("terspec");
    node.appendChild(terSpecNode);
    srcNode = terspecInputEditor.contentDocument.documentElement.getElementsByTagName('dialogbase')[0];
    if (srcNode) {
      appendNodeContents(terSpecNode, srcNode);
    }
    hasPriSpec += " t";
  }

  hasPriSpec = TrimString(hasPriSpec);
  if (hasPriSpec.length > 0)
    node.setAttribute("specAppearance", hasPriSpec);
  else
    node.setAttribute("specAppearance", null);

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
    node.setAttribute("enc", enc);

  activeEditor.beginTransaction();
  if (!isNewnode){
    activeEditor.removeNode(origNode);
  }
  activeEditor.insertElementAtSelection(node, true);
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
            case "prispecInputEditor":
              checkboxID = "prispec";
              editorFlag = 1;    
              if (node.hasAttribute("pri")) {
                primary.value = node.getAttribute("pri");
                prispec.disabled = primary.value.length == 0;
                specnode = node.getElementsByTagName("prispec")[0];
                if (specnode && specnode.textContent.length >0)
                { 
                  destNode = prispecInputEditor.contentDocument.documentElement.getElementsByTagName('dialogbase')[0];
                  prispec.checked = true;
                  copyNodeContents(destNode, 0, specnode, prispecInputEditor);
                } else {
                  prispec.checked = false;
                }
              }
              else {
                prispec.checked = false;
              }


            break;
            case "secspecInputEditor":
              checkboxID = "secspec";
              editorFlag = 2;
              if (node.hasAttribute("sec")) {
                secondary.value = node.getAttribute("sec");
                secspec.disabled = secondary.value.length == 0;
                specnode = node.getElementsByTagName("secspec")[0];
                if (specnode && specnode.textContent.length >0)
                { 
                  destNode = secspecInputEditor.contentDocument.documentElement.getElementsByTagName('dialogbase')[0];
                  secspec.checked = true;
                  copyNodeContents(destNode, 0, specnode, secspecInputEditor);
                } else {
                  secspec.checked = false;
                }
              }
              else {
                secspec.checked = false;
              }

            break;
            case "terspecInputEditor":
              checkboxID = "terspec";
              editorFlag = 4;
              if (node.hasAttribute("ter")) {
                tertiary.value = node.getAttribute("ter");
                terspec.disabled = tertiary.value.length == 0;
                specnode = node.getElementsByTagName("terspec")[0];
                if (specnode && specnode.textContent.length >0)
                { 
                  destNode = terspecInputEditor.contentDocument.documentElement.getElementsByTagName('dialogbase')[0];
                  terspec.checked = true;
                  copyNodeContents(destNode, 0, specnode, terspecInputEditor);
                } else {
                  terspec.checked = false;
                }
              }
              else {
                terspec.checked = false;
              }

            break;
            default:
            break;
          }
          if (checkboxID && checkboxID.length)
          {
            var checkbox = document.getElementById(checkboxID);
//            if (!checkbox.checked)
            dump("In indexentry dialog, disabling editor " + this.mEditorElement.id + "\n");
            msiEnableEditorControl(this.mEditorElement, true);//checkbox.checked);
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
