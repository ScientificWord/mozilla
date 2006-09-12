// Copyright (c) 2004 MacKichan Software, Inc.  All Rights Reserved.

var data;

function Startup(){
  data = window.arguments[0];

  var digits = document.getElementById("digits");
  digits.value = data.digits.toString();

  var degree = document.getElementById("degree");
  degree.value = data.degree.toString();

  var principal = document.getElementById("principal");
  principal.setAttribute("checked", data.principal.toString());

  var special = document.getElementById("special");
  special.setAttribute("checked", data.special.toString());

  var logSent = document.getElementById("logSent");
  logSent.setAttribute("checked", data.logSent.toString());

  var engSent = document.getElementById("engSent");
  engSent.setAttribute("checked", data.engSent.toString());

  var engReceived = document.getElementById("engReceived");
  engReceived.setAttribute("checked", data.engReceived.toString());

  var logReceived = document.getElementById("logReceived");
  logReceived.setAttribute("checked", data.logReceived.toString());
}

function parseBool(s){
  if (s == "true")
    return true;
  else
    return false;
}

function OK(){
  data.Cancel = false;

  var digits = document.getElementById("digits");
  data.digits  = parseInt(digits.value,10);

  var degree = document.getElementById("degree");
  data.degree = parseInt(degree.value,10);

  var principal = document.getElementById("principal");
  data.principal = parseBool(principal.getAttribute("checked"));

  var special = document.getElementById("special");
  data.special = parseBool(special.getAttribute("checked"));

  var logSent = document.getElementById("logSent");
  data.logSent = parseBool(logSent.getAttribute("checked"));

  var engSent = document.getElementById("engSent");
  data.engSent = parseBool(engSent.getAttribute("checked"));

  var engReceived = document.getElementById("engReceived");
  data.engReceived = parseBool(engReceived.getAttribute("checked"));

  var logReceived = document.getElementById("logReceived");
  data.logReceived = parseBool(logReceived.getAttribute("checked"));
}

function Cancel(){
  data.Cancel = true;
}



