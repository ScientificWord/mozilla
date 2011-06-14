

var primary;
var prispec;
var prispecapp;
var secondary;
var secspec;
var secspecapp;
var tertiary;
var terspec;
var terspecapp;
var locator;
var format;
var thedeck;
var xreftext;
var node;
var isNewnode = false;
var activeEditor;

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
  prispecapp = document.getElementById("prispecapp");
  secondary = document.getElementById("secondary");
  secspec = document.getElementById("secspec");
  secspecapp = document.getElementById("secspecapp");
  tertiary = document.getElementById("tertiary");
  terspec = document.getElementById("terspec");
  terspecapp = document.getElementById("terspecapp");
  locator = document.getElementById("locator");
  format = document.getElementById("format");
  thedeck = document.getElementById("thedeck");
  xreftext = document.getElementById("xref");

  var specnode;
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
        prispecapp.value = specnode.textContent;
        prispec.checked = true;
      } else prispec.checked = false;
      prispecapp.setAttribute("hidden",!prispec.checked);
    }
    if (node.hasAttribute("sec")) {
      secondary.value = node.getAttribute("sec");
      secspec.disabled = secondary.value.length == 0;
      specnode = node.getElementsByTagName("secspec")[0];
      if (specnode && specnode.textContent.length >0) 
      {
         secspecapp.value = specnode.textContent;
         secspec.checked = true;
      } else secspec.checked = false;
      secspecapp.setAttribute("hidden",!secspec.checked);
    }
    if (node.hasAttribute("ter")) {
      tertiary.value = node.getAttribute("ter");
      terspec.disabled = tertiary.value.length == 0;
      specnode = node.getElementsByTagName("terspec")[0];
      if (specnode && specnode.textContent.length >0)
      { 
        terspecapp.value = specnode.textContent;
        terspec.checked = true;
      } else terspec.checked = false;
      terspecapp.setAttribute("hidden",!terspec.checked);
    }
    if (node.hasAttribute("xreftext") ){
      locator.selectedIndex = 1;
      thedeck.selectedIndex = 1;
      xreftext.value = node.getAttribute("xreftext");
    }
    else {
      locator.selectedIndex = 0;
      thedeck.selectedIndex = 0;
      if (node.hasAttribute("pnstyle")) {
        var st = node.getAttribute("pnstyle");
        if (st == "bold") format.selectedIndex = 1;
        else if (st="italics") format.selectedIndex =2;
        else format.selectedIndex = 0;
      }
    }
  }
  else {
    isNewnode = true;
    node = activeEditor.document.createElement("indexitem");
  }
}


function keypress(event, textbox, checkboxid)
{
  var cb = document.getElementById(checkboxid);
  if (cb) {
    cb.disabled = (textbox.value.length == 0);
    if (cb.disabled) document.getElementById(checkboxid+"app").setAttribute("hidden","true");
    else document.getElementById(checkboxid+"app").setAttribute("hidden",!(cb.checked));
  }
}


function checkboxchanged(event, checkbox, appearanceid)
{
  document.getElementById(appearanceid).setAttribute("hidden",!(checkbox.checked));
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
  dumpln(prispecapp.value);
  dumpln(secondary.value);
  dumpln(secspec.checked);
  dumpln(secspecapp.value);
  dumpln(tertiary.value);
  dumpln(terspec.checked);
  dumpln(terspecapp.value);
  dumpln(xreftext.value);
}


function onAccept()
{
  activeEditor.beginTransaction();
//  node.setAttribute("req","varioref");
  msiEditorEnsureElementAttribute(node, "req", "varioref", activeEditor);
  var v = primary.value;
//  if (v && v.length > 0)
//    node.setAttribute("pri",v);
//  else node.removeAttribute("pri");
  msiEditorEnsureElementAttribute(node, "pri", v, activeEditor);

  v = secondary.value;
//  if (v && v.length > 0)
//    node.setAttribute("sec",v);
//  else node.removeAttribute("sec");
  msiEditorEnsureElementAttribute(node, "sec", v, activeEditor);
  v = tertiary.value;
//  if (v && v.length > 0)
//    node.setAttribute("ter",v);
//  else node.removeAttribute("ter");
  msiEditorEnsureElementAttribute(node, "ter", v, activeEditor);

  if (node.parentNode) dump(node.parentNode.innerHTML+"\n");
  else dump(node.innerHTML+"\n");

  // remove subnodes, if any
  while(node.firstChild) 
    activeEditor.deleteNode(node.firstChild);
    //node.removeChild(node.firstChild);
  v = prispecapp.value;
  if (prispec.checked && v && v.length > 0) {
    var prispecnode;
    prispecnode = activeEditor.document.createElement("prispec");
    prispecnode.textContent = v;
    activeEditor.insertNode(prispecnode, node, node.childNodes.length);
//    node.appendChild(prispecnode);
  }
  v = secspecapp.value;
  if (secspec.checked && v && v.length > 0) {
    var secspecnode;
    secspecnode = activeEditor.document.createElement("secspec");
    secspecnode.textContent = v;
    activeEditor.insertNode(secspecnode, node, node.childNodes.length);
//    node.appendChild(secspecnode);
  }
  v = terspecapp.value;
  if (terspec.checked && v && v.length > 0) {
    var terspecnode;
    terspecnode = activeEditor.document.createElement("terspec");
    terspecnode.textContent = v;
    activeEditor.insertNode(terspecnode, node, node.childNodes.length);
//    node.appendChild(terspecnode);
  }
  if (isNewnode) activeEditor.insertElementAtSelection(node, true);

  activeEditor.endTransaction();
}


function onCancel()
{
  return true;
}