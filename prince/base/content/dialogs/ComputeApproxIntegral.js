// Copyright (c) 2004 MacKichan Software, Inc.  All Rights Reserved.

var data;

function Startup(){
  data = window.arguments[0];

  var intervals = document.getElementById("intervals");
  intervals.value = data.intervals.toString();

  var form = document.getElementById("form");
  form.selectedIndex = data.form - 1;
}

function OK(){
  data.Cancel = false;

  var intervals = document.getElementById("intervals");
  data.intervals = parseInt(intervals.value,10);

  var form = document.getElementById("form");
  data.form = form.selectedIndex + 1;
}

function Cancel(){
  data.Cancel = true;
}
