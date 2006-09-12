// Copyright (c) 2004 MacKichan Software, Inc.  All Rights Reserved.

var data;

function Startup(){
  data = window.arguments[0];

  var ncols = document.getElementById("ncols");
  ncols.value = data.ncols;
}

function OK(){
  data.Cancel = false;

  //validate?
  var ncols = document.getElementById("ncols");
  data.ncols  = ncols.value;
}

function Cancel(){
  data.Cancel = true;
}



