// Copyright (c) 2004 MacKichan Software, Inc.  All Rights Reserved.

var target;

function parseBool(s){
  if (s == "true")
    return true;
  else
    return false;
}

function onOK() {
  var t = document.getElementById("dialog.input");
  target.val = t.value;

  var enginefunction = document.getElementById("enginefunction");
  target.enginefunction = parseBool(enginefunction.getAttribute("checked"));

  return(true);
}

function onCancel() {
  target.val = "";
  target.enginefunction = false;
  return(true);
}

function Startup() {
  target = window.arguments[0];
}

