// Copyright (c) 2004 MacKichan Software, Inc.  All Rights Reserved.

Components.utils.import("resource://app/modules/msiEditorDefinitions.jsm");

var data;

function Startup(){
  data = window.arguments[0];

  var intervals = document.getElementById("intervals");
  intervals.value = data.intervals.toString();

  var form = document.getElementById("form");
  form.selectedIndex = data.form - 1;

  //var lowerBound = document.getElementById("lowerBound");
  //lowerBound.value = data.lowerBound;

  //var upperBound = document.getElementById("upperBound");
  //upperBound.value = data.upperBound;
}

function OK(){
  data.Cancel = false;

  var intervals = document.getElementById("intervals");
  data.intervals = parseInt(intervals.value,10);

  var form = document.getElementById("form");
  data.form = form.selectedIndex + 1;

  var lowerBound = document.getElementById("lowerBound");
  data.lowerBound = parseInt(lowerBound.value, 10);

  var upperBound = document.getElementById("upperBound");
  data.upperBound = parseInt(upperBound.value, 10);

}

function Cancel(){
  data.Cancel = true;
}
