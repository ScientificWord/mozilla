// Copyright (c) 2004 MacKichan Software, Inc.  All Rights Reserved.


var data;

function setIndex(item,idx) {
  document.getElementById(item).selectedIndex = idx;
}

function setChecked(item,val) {
  document.getElementById(item).setAttribute("checked", val == 1 ? "true" : "false");
}

function setValue(item,val) {
  document.getElementById(item).value = val;
}

function Startup(){
  data = window.arguments[0];

  setIndex("fences", data.mfenced ? 0 : 1);

  setValue("digits", data.digits.toString());
  setValue("lower", data.lower.toString());
  setValue("upper", data.upper.toString());
  setValue("primesasn", data.primesasn.toString());

  setChecked("mixednum", data.mixednum);
  setChecked("trigargs", data.trigargs);
  setChecked("usearc", 1 - data.usearc);
  setChecked("logs", data.loge);
  setChecked("dots", data.dotderiv);
  setChecked("bar", data.barconj);
  setChecked("i_imaginary", data.i_imaginary);
  setChecked("j_imaginary", data.j_imaginary);
  setChecked("e_exp", data.e_exp);
  setChecked("primederiv", data.primederiv);
  
  setIndex("matrix_delim", data.matrix_delim);

  setIndex("derivformat", data.derivformat);
  setIndex("imagi", data.imaginaryi);
  setIndex("diffD", data.diffD);
  setIndex("diffd", data.diffd);
  setIndex("expe", data.expe);
}

function getIndex(item) {
  return document.getElementById(item).selectedIndex;
}

function getChecked(item) {
  return document.getElementById(item).getAttribute("checked") == "true" ? 1 : 0;
}

function getValue(item) {
  return document.getElementById(item).value;
}

function OK(){
  data.Cancel = false;

  data.mfenced = getIndex("fences") == 0 ? 1 : 0;

  data.digits  = parseInt(getValue("digits"),10);
  data.lower  = parseInt(getValue("lower"),10);
  data.upper  = parseInt(getValue("upper"),10);
  data.primesasn  = parseInt(getValue("primesasn"),10);

  data.mixednum = getChecked("mixednum");
  data.trigargs = getChecked("trigargs");
  data.usearc = 1 - getChecked("usearc");
  data.loge = getChecked("logs");
  data.dotderiv = getChecked("dots");
  data.barconj = getChecked("bar");
  data.i_imaginary = getChecked("i_imaginary");
  data.j_imaginary = getChecked("j_imaginary");
  data.e_exp = getChecked("e_exp");
  data.primederiv  = getChecked("primederiv");

  data.derivformat = getIndex("derivformat");
  data.imaginaryi = getIndex("imagi");
  data.diffD = getIndex("diffD");
  data.diffd = getIndex("diffd");
  data.expe = getIndex("expe");
  data.matrix_delim = getIndex("matrix_delim");
}

function Cancel(){
  data.Cancel = true;
}



