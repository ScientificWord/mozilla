<?xml version="1.0"?>
<!-- Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved. -->


function onAccept()
{
  var data = window.arguments[0];
  data.filename = document.getElementById("fragment-name").value;
  data.description = document.getElementById("fragment-description").value;
}

function onCancel()
{
  var data = window.arguments[0];
  data.filename = "";
}

function initialize()
{
  var data = window.arguments[0];
  if (!(data.description && data.description.length > 0))
    data.description = document.getElementById("descriptionprompt").getAttribute('value');
  if (data.filename && data.filename.length > 0) document.getElementById("fragment-name").value = data.filename; 
  var broadcasters = document.getElementsByTagName("broadcaster");
  var j;
  for (j=0; j< broadcasters.length; j++) broadcasters[j].setAttribute("hidden","true");
  if (data.role) document.getElementById(data.role).setAttribute("hidden","false"); 
  document.getElementById("fragment-description").value = data.description;
}

