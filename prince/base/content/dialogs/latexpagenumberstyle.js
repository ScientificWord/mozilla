// Copyright (c) 2006 MacKichan Software, Inc.  All Rights Reserved.

var data;

function trim( s ) // trim whitespace from the end of a string
{
  var l;
  l = s.length - 1;
  while (l>0 && s[l]<=32)
  {
    s=s.substr(0,l);
    l--;
  }
  return s;
}
// dialog initialization code
function Startup()
{
  doSetOKCancel(onAccept, onCancel);
  data = window.arguments[0];
  data.numstyle = trim( data.numstyle); 
  data.Cancel = false;
  list = document.getElementById("numberStyles");
  selected = list.getElementsByAttribute("latexvalue", 
      data.numstyle);
  if (selected && selected.length>0) {
    list.selectItem(selected[0]);
  }  
  InitDialog();
  SetWindowLocation();
}

function InitDialog()
{
  document.documentElement.getButton("accept").setAttribute("default", true);
}

function onAccept()
{
  var list;
  list = document.getElementById("numberStyles");
  data.numstyle = trim(list.selectedItem.getAttribute("latexvalue"));
  SaveWindowLocation();
  return true;
}

function onCancel()
{
  data.Cancel = true;
}

function doAccept()
{
  document.documentElement.getButton('accept').oncommand();
}
