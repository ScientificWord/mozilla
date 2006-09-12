// Copyright (c) 2004 MacKichan Software, Inc.  All Rights Reserved.

var data;

function Startup(){
  data = window.arguments[0];

  var column = document.getElementById("column");
  column.selectedIndex = data.column;

  var code = document.getElementById("code");
  code.selectedIndex = data.code;

  var degree = document.getElementById("degree");
  degree.value = data.degree.toString();

  // type defaults to first choice according to XUL
}

function OK(){
  data.Cancel = false;

  var column = document.getElementById("column");
  data.column  = column.selectedIndex;

  var code = document.getElementById("code");
  data.code = code.selectedIndex;

  var degree = document.getElementById("degree");
  data.degree = parseInt(degree.value,10);
}

function Cancel(){
  data.Cancel = true;
}

function doSelect(){
  var code = document.getElementById("code");
  var deg = document.getElementById("degree");
  var text = document.getElementById("degree.text");
  if (code.selectedIndex == 2) {
    deg.disabled = false;
    text.disabled = false;
  } else {
    deg.disabled = true;
    text.disabled = true;
  }
}


