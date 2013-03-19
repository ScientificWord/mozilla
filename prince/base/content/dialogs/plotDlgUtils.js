var numberRE = /(\-)?([0-9]+)?(\.[0-9]+)?/;  //Or delete the leading minus?
var sciNotationNumberRE = /(\-)?([0-9]+)?(\.[0-9]+)?e(\-?[0-9]+)/i;  //Or delete the leading minus?

function getValueFromControl(anID)
{
  var theControl = document.getElementById(anID);
  if (theControl)
  {
    if ( (theControl.nodeName == "spacer") && (theControl.getAttribute("class") == "color-well") )
    {
      var styleStr = theControl.getAttribute("style");
      var colorMatch = /background-color\:\s*([^;]+);?$/;
      if (styleStr && styleStr.length)
      {
        var matchRes = colorMatch.exec(styleStr);
        if (matchRes && matchRes[1])
          return getColorFromCSSSpec(matchRes[1]);
      }
    }
    else if (theControl.nodeName == "checkbox")
    {
      return theControl.checked ? "true" : "false";
    }
    else
      return theControl.value;
  }
  return "undefined";
}

function getColorFromCSSSpec(cssStr)
{
  function decToTwoHexDigits(decStr)
  {
    var num = Number(decStr);
    var retStr = num.toString(16);
    while (retStr.length < 2)
      retStr = "0" + retStr;
    return retStr;
  }
  var rgbMatch = /rgb\(\s*([0-9]+)\s*,\s*([0-9]+)\s*,\s*([0-9]+)\s*\)/;
  var hexMatch = /#?([0-9a-fA-F]{1,6})/;
  var matchRes = rgbMatch.exec(cssStr);
  if (matchRes && matchRes[3])
  {
    return decToTwoHexDigits(matchRes[1]) + decToTwoHexDigits(matchRes[2]) + decToTwoHexDigits(matchRes[3]);
  }
  matchRes = hexMatch.exec(cssStr);
  if (matchRes && matchRes[1])
    return matchRes[1];
  return null;
}

function putValueToControl(anID, aVal)
{
  var theControl = document.getElementById(anID);
  if (theControl)
  {
    if ( (theControl.nodeName == "spacer") && (theControl.getAttribute("class") == "color-well") )
    {
      setColorWell(anID, makeColorVal(aVal));
    }
    else if (theControl.nodeName == "checkbox")
    {
      theControl.checked = aVal ? "true" : "false";
    }
    else if ( (theControl.nodeName == "textbox") && (theControl.getAttribute("ilk") == "numberlike") )
    {
      var sigDigits = 5;
      if (theControl.hasAttribute("significantDigits"))
      {
        sigDigits = Number(theControl.getAttribute("significantDigits"));
        if (isNaN(sigDigits))
          sigDigits = 5;
      }     
      theControl.value = useSignificantDigits(aVal, sigDigits);
    }
    else
      theControl.value = aVal;
  }
}

function testUseSignificantDigits()
{
  var bPassed;
  function reportResults(val, sdig, correctRes)
  {
    var res = useSignificantDigits(val, sdig);
    if (res == correctRes)
      msidump("useSignificantDigits worked for (" + val + ", " + sdig + ")\n");
    else
      msidump("useSignificantDigits failed for (" + val + ", " + sdig + "); produced [" + res + "]\n");
  }
  reportResults("156893", 3, "1.57e5");
  reportResults("333.0099999", 5, "333.01");
  reportResults("-.00000001", 4, "-1.0e-8");
  reportResults("-0.4499e-4", 3, "-4.50e-5");
  reportResults("11.99997",3,"12.0");
  reportResults("1.234789e4", 5, "12348");
  reportResults("-6.77775e4", 5, "-67777");
}

function useSignificantDigits(aVal, sigDigits)
{
//var numberRE = /(\-)?([0-9]+)?(\.[0-9]+)?/;  //Or delete the leading minus?
//var sciNotationNumberRE = /(\-)?([0-9]+)?(\.[0-9]+)?e(\-?[0-9]+)/i;  //Or delete the leading minus?
  var scinotation = sciNotationNumberRE.exec(aVal);
  var number;
  var neg, whole, dec, exp, theDigits, firstPos, jj;
  var retStr = "";
  if (!scinotation || !scinotation[0])
  {
    number = numberRE.exec(aVal);
    if (!number || !number[0])
      return aVal;  //can't do anything with it!?
    neg = (number[1]=="-") ? true : false;
    whole = number[2] ? number[2] : "";
    dec = number[3] ? number[3].substr(1) : ""; //skip the decimal point
    exp = 0;
  }
  else
  {
    neg = (scinotation[1]=="-") ? true : false;
    whole = scinotation[2] ? scinotation[2] : "";
    dec = scinotation[3] ? scinotation[3].substr(1) : ""; //skip the decimal point
    exp = (scinotation[4] && scinotation[4].length) ? Number(scinotation[4]) : 0;
    if (isNaN(exp))
      exp = 0;
  }
  if ((whole.length + dec.length) <= sigDigits)
    return aVal;
  theDigits = whole + dec;
  firstPos = theDigits.search(/[^0]/);
  if (firstPos < 0)
    return "0";
  var bCarry = false;
  if (firstPos + sigDigits < theDigits.length)
  {
    if (theDigits[firstPos + sigDigits] == '5')
    {
      if ( (firstPos + sigDigits + 1 < theDigits.length) && (theDigits.substr(firstPos + sigDigits + 1).match(/[^0]/)) )
        bCarry = true;
      else
        bCarry = !neg;
    }
    else if (Number(theDigits[firstPos + sigDigits]) > 5)
      bCarry = true;
    jj = 0;
    if (bCarry)
    {
      for (jj = firstPos + sigDigits - 1; bCarry && (jj >= 0); --jj)
      {
        bCarry = (theDigits[jj] == '9');
        retStr = (bCarry ? '0' : String(Number(theDigits[jj]) + 1) ) + retStr;
      }
      if (bCarry)
        theDigits = "1" + retStr.substr(0,retStr.length - 1);
      else
        theDigits = theDigits.substr(firstPos, jj+1-firstPos) + retStr;
    }
    else
      theDigits = theDigits.substr(firstPos, sigDigits);
  }
  else
    theDigits = theDigits.substr(firstPos);
  exp += whole.length - firstPos - 1;
  neg = (neg ? "-" : "");
  if ( (exp >= sigDigits) || (exp <= -(sigDigits)) )
  {
    if (theDigits.length <= 1)
      theDigits += "0";
    retStr = neg + theDigits[0] + "." + theDigits.substr(1) + "e" + exp;
  }
  else if (exp < 0)
  {
    retStr = neg + "0.";
    for (jj = 0; jj < sigDigits + exp; ++jj)
      retStr += "0";
    retStr += theDigits;
  }
  else
  {
    retStr = neg + theDigits.substr(0, exp + 1);
    if (exp + 1 < sigDigits)
      retStr += "." + theDigits.substr(exp+1);
//    for (jj = 0; jj < sigDigits - exp; ++jj)
//      retStr += "0";
  }
  return retStr;
}

function makeColorVal(attribStr)
{
  var retVal = attribStr.replace(/^([0-9a-fA-F]+)$/,"#$1");
  return retVal; 
}

function putMathMLExpressionToControlByID(ctrlID, expr)
{
  var ctrl = document.getElementById(ctrlID);
  if (!ctrl)
  {
    msidump("Trying to write expression " + expr + " to missing control with ID " + ctrlID + "!\n");
    return;
  }

  return putMathMLExpressionToControl(ctrl, expr);
}

function putMathMLExpressionToControl(ctrl, expr)
{
  var editor, parser;
  var nodeList, topNode;
  var exprDoc;
  var theValue;
  var mathExpr = expr;
  if (expr.substr(0,6) != "<math ")
    expr = "<math xmlns=\"http://www.w3.org/1998/Math/MathML\">" + expr + "</math>";

  switch(ctrl.nodeName)
  {
    case "editor":
      if (ctrl.getAttribute("editortype") == "html")
      {
        editor = msiGetEditor(ctrl);
        topNode = msiNavigationUtils.getFirstParagraphDescendant(editor.rootElement, editor);
        if (!topNode)
          return;
        while (topNode.nextSibling)
          topNode.parentNode.removeChild(topNode.nextSibling);
        while (topNode.firstChild)
          topNode.removeChild(topNode.firstChild);
        editor.insertHTMLWithContext(expr, "", "", "", null, topNode, 0, false);
      }
    break;

    case "textbox":
      parser = new DOMParser();
      exprDoc = parser.parseFromString(expr, "text/xml");
      theValue = mathNodeToNumericText(exprDoc.documentElement);
      if (ctrl.getAttribute("ilk") == "numberlike")
      {
        var sigDigits = 5;
        if (theControl.hasAttribute("significantDigits"))
        {
          sigDigits = Number(theControl.getAttribute("significantDigits"));
          if (isNaN(sigDigits))
            sigDigits = 5;
        }
        theValue = useSignificantDigits(theValue, sigDigits);
      }       
      ctrl.value = theValue;
    break;

    case "label":
    case "description":
      parser = new DOMParser();
      exprDoc = parser.parseFromString (expr, "text/xml");
      while (ctrl.firstChild)
        ctrl.removeChild(ctrl.firstChild);
      ctrl.appendChild(exprDoc.documentElement);
    break;
  }
}

function mathNodeToNumericText(aNode, bNumberType)
{
  var kids, numRes;
  var jj;
  var retval = null;
  var kid = msiNavigationUtils.getSingleWrappedChild(aNode);
  if (kid)
    retval = mathNodeToControlText(kid);
  else
  {
    switch( msiGetBaseNodeName(aNode) )
    {
      case "math":
      case "mrow":
      case "mstyle":
        kids = msiNavigationUtils.getSignificantContents(aNode);
        for (jj = 0; !retval && (jj < kids.length); ++jj)
          retval = mathNodeToNumericText(kids[jj], bNumberType);
      break;
      case "mn":
        retval = aNode.textContent;
      break;
      case "mi":
        if (!bNumberType)
          retval = aNode.textContent;
      break;
    }
  }
  if (retval && retval.length)
  {
    if (!bNumberType)
      numRes = sciNotationNumberRE.exec(retval);
    if (!numRes)
      numRes = numberRE.exec(retval);
    if (numRes && numRes[0])
      retval = numRes[0];
  }
  return retval;
}

function getMathMLExpressionFromControlByID(ctrlID, ser)
{
  var ctrl = document.getElementById(ctrlID);
  if (!ctrl)
  {
    msidump("Trying to get expression from missing control with ID " + ctrlID + "!\n");
    return;
  }

  return getMathMLExpressionFromControl(ctrl, ser);
}

function getMathMLExpressionFromControl(ctrl, ser)
{
  var editor;
  var nodeList, topNode;
  var exprDoc, retval;
//  var mathExpr = expr;
//  if (expr.substr(0,6) != "<math ")
//    expr = "<math xmlns=\"http://www.w3.org/1998/Math/MathML\">" + expr + "</math>";

  switch(ctrl.nodeName)
  {
    case "editor":
      if (ctrl.getAttribute("editortype") == "html")
      {
        exprDoc = ctrl.contentDocument;
        topNode = exprDoc.getElementsByTagName("math")[0];
        retval = ser.serializeToString(topNode);
      }
    break;

    case "textbox":
      retval = mathNodeFromNumericText(ctrl.value);
    break;

    case "description":
    case "label":
      topNode = ctrl.getElementsByTagName("math")[0];
      retval = ser.serializeToString(topNode);
    break;
  }
  return retval;
}

function mathNodeFromNumericText(text)
{
  var numRes = numberRE.exec(text);
  if (numRes)
    return wrapMath("<mn>" + numRes[0] + "</mn>");
  numRes = sciNotationNumberRE.exec(text);
  if (numRes)
    return wrapMath("<mn>" + numRes[0] + "</mn>");
  return wrapMath("<mi>" + text + "</mi>");
}
