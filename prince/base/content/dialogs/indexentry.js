

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
var xreftext

function dumpln(s)
{
  dump(s+"\n");
}

function startup()
{
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
  var node = window.arguments[0]; //current index entry if not void or null
  if (node)
  {
    if (node.tagName != "indexitem") alert("Wrong node passed to indexentry.js!");
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
  var node =  window.arguments[0];
  var editorElement = msiGetParentEditorElementForDialog(window);
  var editor = msiGetEditor(editorElement);
  var domdoc = editor.document;
  var newnode = !node; 
  if (newnode) 
  {
    node = domdoc.createElement("indexitem");
  }
  var v = primary.value;
  if (v && v.length > 0)
    node.setAttribute("pri",v);
  else node.removeAttribute("pri");
  v = secondary.value;
  if (v && v.length > 0)
    node.setAttribute("sec",v);
  else node.removeAttribute("sec");
  v = tertiary.value;
  if (v && v.length > 0)
    node.setAttribute("ter",v);
  else node.removeAttribute("ter");
  if (node.parentNode) dump(node.parentNode.innerHTML+"\n");
  else dump(node.innerHTML+"\n");
  // remove subnodes, if any
  while(node.firstChild) node.removeChild(node.firstChild);
  v = prispecapp.value;
  if (prispec.checked && v && v.length > 0) {
    var prispecnode;
    prispecnode = domdoc.createElement("prispec");
    prispecnode.textContent = v;
    node.appendChild(prispecnode);
  }
  v = secspecapp.value;
  if (secspec.checked && v && v.length > 0) {
    var secspecnode;
    secspecnode = domdoc.createElement("secspec");
    secspecnode.textContent = v;
    node.appendChild(secspecnode);
  }
  v = terspecapp.value;
  if (terspec.checked && v && v.length > 0) {
    var terspecnode;
    terspecnode = domdoc.createElement("terspec");
    terspecnode.textContent = v;
    node.appendChild(terspecnode);
  }
  if (newnode) editor.insertElementAtSelection(node, true);
}


function onCancel()
{
  return true;
}