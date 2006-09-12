// Copyright (c) 2006 MacKichan Software, Inc.  All Rights Reserved.

var data;

function Startup(){
  data = window.arguments[0];

  document.getElementById("engine").selectedIndex = data.engine - 1;
}

function OK(){
  data.Cancel = false;

  var subscript = document.getElementById("engine");
  var n = subscript.selectedIndex + 1;
  data.engine = n.toString();
}

function Cancel(){
  data.Cancel = true;
}


