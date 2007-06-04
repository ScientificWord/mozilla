<?xml version="1.0"?>
<!-- Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved. -->

function onAccept()
{
  try {
    eval(document.getElementById("javascript-text").value);
  }
  catch(e) {
    dump("Execute JavaScript error: "+e+"\n");
  }
}

function onCancel()
{
}

