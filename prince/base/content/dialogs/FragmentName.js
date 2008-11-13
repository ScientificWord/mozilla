<?xml version="1.0"?>
<!-- Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved. -->

function onAccept()
{
  var data = window.arguments[0];
  data.filename = document.getElementById("fragment-name").value;
}

function onCancel()
{
  var data = window.arguments[0];
  data.filename = "";
}

function initialize()
{
  var data = window.arguments[0];
  if (data.filename && data.filename.length > 0) document.getElementById("fragment-name").value = data.filename; 
  var broadcasters = document.getElementsByTagName("broadcaster");
  var j;
  for (j=0; j< broadcasters.length; j++) broadcasters[j].setAttribute("hidden","true");
  if (data.role) document.getElementById(data.role).setAttribute("hidden","false");
}

