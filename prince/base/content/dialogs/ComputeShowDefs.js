// Copyright (c) 2004 MacKichan Software, Inc.  All Rights Reserved.

function Startup(){
  var data = window.arguments[0];
  var iframe = document.getElementById("content");

  var doc   = '<?xml version="1.0"?>\n';
  doc = doc + '<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1 plus MathML 2.0//EN" "http://www.w3.org/Math/DTD/mathml2/xhtml-math11-f.dtd">\n';
  // add math stylesheet?
  doc = doc + '<html xmlns="http://www.w3.org/1999/xhtml">\n';
  doc = doc + '<body>\n';
  doc = doc + data.val;
  doc = doc + '</body></html>\n';

  var src = "data:application/xhtml+xml;charset=utf-8,";
  src = src + encodeURIComponent(doc);  // data:URI must be UTF-8

  iframe.setAttribute("src",src);
}




