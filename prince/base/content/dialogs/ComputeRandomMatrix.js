// Copyright (c) 2004 MacKichan Software, Inc.  All Rights Reserved.

var data;

function Startup(){
  data = window.arguments[0];

  var rows = document.getElementById("rows");
  rows.value = data.rows.toString();

  var cols = document.getElementById("cols");
  cols.value = data.cols.toString();

  var min = document.getElementById("min");
  min.value = data.min.toString();

  var max = document.getElementById("max");
  max.value = data.max.toString();

  // type defaults to first choice according to XUL
}

function OK(){
  data.Cancel = false;

  var rows = document.getElementById("rows");
  data.rows  = parseInt(rows.value,10);

  var cols = document.getElementById("cols");
  data.cols = parseInt(cols.value,10);

  var min = document.getElementById("min");
  data.min = parseInt(min.value,10);

  var max = document.getElementById("max");
  data.max = parseInt(max.value,10);

  var type = document.getElementById("type");
  data.type = type.selectedIndex + 1;
}

function Cancel(){
  data.Cancel = true;
}



