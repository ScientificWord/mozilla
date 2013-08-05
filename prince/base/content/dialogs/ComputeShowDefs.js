// Copyright (c) 2004 MacKichan Software, Inc.  All Rights Reserved.

function Startup(){
  var data = window.arguments[0];
  var editElement = document.getElementById("content");

  msiInitializeEditorForElement(editElement, data.val, true);

  msiSetDocumentEditable(false, editElement);
//  dump("In ComputeShowDefs.js, whole src is is [" + src + "].\n");
}




