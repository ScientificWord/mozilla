// Copyright (c) 2004 MacKichan Software, Inc.  All Rights Reserved.

//function Startup(){
//  var data = window.arguments[0];
//  var iframe = document.getElementById("content");
//
//  var doc   = '<?xml version="1.0"?>\n';
//  doc = doc + '<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1 plus MathML 2.0//EN" "http://www.w3.org/Math/DTD/mathml2/xhtml-math11-f.dtd">\n';
//
//  doc = doc + '<html xmlns="http://www.w3.org/1999/xhtml">\n';
//
//  // add math stylesheet?
//  var editorElement = msiGetParentEditorElementForDialog(window);
//  var mgMathStyleSheet = msiColorObj.FormatStyleSheet(editorElement, true); //"true" means standAlone - that is, no "data:text/css," at the beginning
////  mgMathStyleSheet = mgMathStyleSheet.replace("\"", "\'", "g");
//  dump("In ComputeShowDefs.js, stylesheet line is [" + mgMathStyleSheet + "].\n");
//
////  doc += "<?xml-stylesheet href=\"" + mgMathStyleSheet + "\" type=\"text/css\"?>";
//  doc += "<head>\n<style type=\"text/css\">\n";
//  doc += mgMathStyleSheet + "\n</style>\n</head>";
//
//  doc = doc + '<body>\n';
//  doc = doc + data.val;
//  doc = doc + '</body></html>\n';
//
//  var src = "data:application/xhtml+xml;charset=utf-8,";
//  src = src + encodeURIComponent(doc);  // data:URI must be UTF-8
//
////  dump("In ComputeShowDefs.js, whole src is is [" + src + "].\n");
//
//  iframe.setAttribute("src",src);
//}

function Startup(){
  var data = window.arguments[0];
  var editElement = document.getElementById("content");

  msiInitializeEditorForElement(editElement, data.val, true);

  msiSetDocumentEditable(false, editElement);
//  dump("In ComputeShowDefs.js, whole src is is [" + src + "].\n");
}




