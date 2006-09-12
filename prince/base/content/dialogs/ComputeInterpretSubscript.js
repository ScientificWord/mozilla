// Copyright (c) 2004 MacKichan Software, Inc.  All Rights Reserved.

var data;

function Startup(){
  data = window.arguments[0];

  // subscript defaults to first choice according to XUL
}

function OK(){
  data.Cancel = false;

  var subscript = document.getElementById("subscript");
  var n = subscript.selectedIndex + 1;
  data.subscript = n.toString();
}

function Cancel(){
  data.Cancel = true;
}


