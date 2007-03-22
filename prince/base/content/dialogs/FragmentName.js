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

