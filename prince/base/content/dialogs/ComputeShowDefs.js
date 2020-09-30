// Copyright (c) 2004 MacKichan Software, Inc.  All Rights Reserved.


Components.utils.import("resource://app/modules/msiEditorDefinitions.jsm");

function Startup(){
  var data = window.arguments[0];
  var editElement = document.getElementById("content");

  msiInitializeEditorForElement(editElement, data.val, true, null, true);

  msiSetDocumentEditable(false, editElement);
//  dump("In ComputeShowDefs.js, whole src is is [" + src + "].\n");
}




