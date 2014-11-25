//Following have been moved to GraphOverlay.js
//var numberRE = /(\-)?([0-9]+)?(\.[0-9]+)?/;  //Or delete the leading minus?
//var sciNotationNumberRE = /(\-)?([0-9]+)?(\.[0-9]+)?e(\-?[0-9]+)/i;  //Or delete the leading minus?

function getValueFromControlByID(anID)
{
  var theControl = document.getElementById(anID);
  return getValueFromControl(theControl);
}

//This function should return a string value
function getValueFromControl(theControl)
{
  if (theControl)
  {
    if ( (theControl.nodeName == "spacer") && (theControl.getAttribute("class") == "color-well") )
    {
      var theVal, alphaVal;
      var styleStr = theControl.getAttribute("style");
      var colorMatch = /background-color\:\s*([^;]+);?$/;
      if (styleStr && styleStr.length)
      {
        var matchRes = colorMatch.exec(styleStr);
        if (matchRes && matchRes[1])
          theVal = getColorFromCSSSpec(matchRes[1]);
        if (theControl.getAttribute("hasAlpha") == "true")
        {
          alphaVal = theControl.getAttribute("alpha");
          if (!alphaVal || alphaVal.length == 0)
            alphaVal = "ff";
          else if (alphaVal.length == 1)
            alphaVal += "0";
          else
            alphaVal = alphaVal.substr(0,2);
          theVal += alphaVal;
        }
      }
      return theVal;
    }
    else if (theControl.nodeName == "checkbox")
    {
      return theControl.checked ? "true" : "false";
    }
    else if ( (theControl.nodeName == "textbox") && (theControl.getAttribute("ilk") == "numberlike") && (theControl.getAttribute("displayDegrees") == "true") )
    {
      return String( Number(theControl.value) * Math.PI/180 );
    }
    else
      return theControl.value;
  }
  return "undefined";
}

function decToTwoHexDigits(decStr)
{
  var num = Number(decStr);
  var retStr = num.toString(16);
  while (retStr.length < 2)
    retStr = "0" + retStr;
  return retStr;
}

function twoHexDigitsToNumber(hexStr)  //of course, it could be just one hex digit
{
  hexStr = hexStr.toLowerCase();
  var hexDigits = "0123456789abcdef";
  var nStart = 0;
  if (hexStr[0] == "#")
    nStart = 1;
  var retval = hexDigits.indexOf(hexStr[nStart]);
  if (retval < 0)
    return Number.NaN;
  var digit2 = 0;
  if (hexStr.length > 1 + nStart)
  {
    digit2 = hexDigits.indexOf(hexStr[nStart + 1]);
    if (digit2 < 0)
      return Number.NaN;
    retval = 16 * retval + digit2;
  }
  return retval;
}

function getColorFromCSSSpec(cssStr)
{
  var rgbMatch = /rgb\(\s*([0-9]+)\s*,\s*([0-9]+)\s*,\s*([0-9]+)\s*\)/;
  var hexMatch = /#?([0-9a-fA-F]{1,6})/;
  var matchRes = rgbMatch.exec(cssStr);
  if (matchRes && matchRes[3])
  {
    return "#" + decToTwoHexDigits(matchRes[1]) + decToTwoHexDigits(matchRes[2]) + decToTwoHexDigits(matchRes[3]);
  }
  matchRes = hexMatch.exec(cssStr);
  if (matchRes && matchRes[1])
    return "#" + matchRes[1];
  return null;
}

function putValueToControlByID(anID, aVal)
{
  if (aVal == null) return;
  var theControl = document.getElementById(anID);
  putValueToControl(theControl, aVal);
}

function putValueToControl(theControl, aVal)
{
  if (theControl)
  {
    if ( (theControl.nodeName == "spacer") && (theControl.getAttribute("class") == "color-well") )
    {
      setColorWellControl(theControl, makeColorVal(aVal));
      theControl.color = aVal;
      var alphaVal;
      if (theControl.getAttribute("hasAlpha") == "true")
      {
        if (aVal[0] == "#")
          alphaVal = aVal.substr(7);  //start after # and 6 characters
        else
          alphaVal = aVal.substr(6);
        if (alphaVal.length == 0)
          alphaVal = "ff";
        else if (alphaVal.length == 1)
          alphaVal += "0";
        else
          alphaVal = alphaVal.substr(0,2);
        theControl.setAttribute("alpha", alphaVal);
      }
    }
    else if (theControl.nodeName == "checkbox")
    {
      theControl.checked = (aVal == "true");
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
      if (theControl.getAttribute("displayDegrees") == "true")
        aVal = String( Number(aVal) * 180/Math.PI );
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
  reportResults("0.0875631", 3, "0.0876");
}

function makeColorVal(attribStr)
{
  var retVal = attribStr.replace(/^#?([0-9a-fA-F]{1,6}).*$/,"#$1");
  return retVal;
}

var invisibleMathOpFilter =
{
  //parameters: in AString mimeType, in nsIURL contentSourceURL, in nsIDOMDocument sourceDocument,
  //               in PRBool willDeleteSelection, inout nsIDOMNode docFragment, inout nsIDOMNode contentStartNode,
  //               inout long contentStartOffset, inout nsIDOMNode contentEndNode, inout long contentEndOffset,
  //               inout nsIDOMNode insertionPointNode, inout long insertionPointOffset,
  //               out boolean continueWithInsertion);
  notifyOfInsertion : function(mimeType, contentSourceURL, sourceDocument, willDeleteSelection,
                               docFragment, contentStartNode, contentStartOffset,
                               contentEndNode, contentEndOffset, insertionPointNode,
                               insertionPointOffset, continueWithInsertion)
  {
    //Set up for the simple case, since that's all we should encounter in our context
    if ((contentEndNode.value != contentStartNode.value) || (contentStartNode.value != docFragment.value))
    {
      msidump("In plotDlgUtils.js, invisibleMathOpFilter, found different start and end nodes for fragment! Aborting...\n");
      return;
    }
    if ((contentStartOffset.value != 0) || (contentEndOffset.value < contentEndNode.value.childNodes.length))
    {
      msidump("In plotDlgUtils.js, invisibleMathOpFilter, startOffset not 0 or endOffset not at node end!\n");
      return;
    }
    var child, ii, jj;
    var opList, opNode, text;
    continueWithInsertion.value = true;
    for (jj = docFragment.value.childNodes.length - 1; jj >= 0; --jj)
    {
      child = docFragment.value.childNodes[jj];
      text = "";
      if (child.nodeName == "mo")
        text = child.textContent;
      if ((text == "\u2061") || (text == "\u2062") || (text == "\u2063"))
      {
        docFragment.removeChild(child);
        continue;
      }
      opList = child.getElementsByTagName("mo");
      for (ii = opList.length - 1; ii >= 0; --ii)
      {
        opNode = opList.item(ii);
        text = opNode.textContent;
        if ((text == "\u2061") || (text == "\u2062") || (text == "\u2063"))
        {
          if (opNode.parentNode)
            opNode.parentNode.removeChild(opNode);
          else if (opNode == contentStartNode)
            continueWithInsertion.value = false;
        }
      }
    }
  }
};

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
        editor.addInsertionListener(invisibleMathOpFilter);
        editor.insertHTMLWithContext(expr, "", "", "", null, topNode, 0, false);
        editor.removeInsertionListener(invisibleMathOpFilter);
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

function isEmptyMathEditControl(editCtrl)
{
  var exprDoc, topNode, kids, ii;
  var rv = true;
  if (editCtrl.getAttribute("editortype") == "html")
  {
    exprDoc = editCtrl.contentDocument;
    topNode = exprDoc.getElementsByTagName("math")[0];
    rv = isEmptyMathNode(topNode);
  }
  else
    rv = (editCtrl.value.length == 0);
  return rv;
}

function isEmptyMathNode(aNode)
{
  var rv = true;
  var ii;
  var kids;
  if (!aNode)
    return true;
  switch(msiGetBaseNodeName(aNode))
  {
    case "mi":
      rv = msiNavigationUtils.isEmptyInputBox(aNode) || (aNode.textContent.length = 0);
    break;

    case "math":
    case "mrow":
    case "mstyle":
    case "mphantom":
    case "maction":
      kids = msiNavigationUtils.getSignificantContents(aNode);
      for (ii = 0; rv && (ii < kids.length); ++ii)
      {
        rv = isEmptyMathNode(kids[ii]);
      }
    break;

    default:
      rv = (aNode.textContent.length == 0);
    break;
  }
  return rv;
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

function getPlotColorAndUpdate(id)
{
  var colorWell;
	colorWell = document.getElementById(id);
  if (!colorWell) return;
  var color = getValueFromControl(colorWell);
  // Don't allow a blank color, i.e., using the "default"
  var colorObj = { NoDefault:true, Type:"Rule", TextColor:color, PageColor:0, Cancel:false };
  var alphaVal;
  if (colorWell.getAttribute("hasAlpha") == "true")
  {
    alphaVal = colorWell.getAttribute("alpha");
    if (color && color.length > 6)
      colorObj.TextColor = color.substr(0, color.length - 2);
    if (alphaVal && alphaVal.length)
    {
      colorObj.alpha = twoHexDigitsToNumber(alphaVal);
      if (colorObj.alpha == Number.NaN)
        colorObj.alpha = 255;
    }
    else
      colorObj.alpha = 255;
  }

  window.openDialog("chrome://editor/content/EdColorPicker.xul", "colorpicker", "chrome,close,titlebar,modal,resizable", "", colorObj);

  // User canceled the gFrameTab
  if (colorObj.Cancel)
    return;

  if ("alpha" in colorObj)
    putValueToControl( colorWell, colorObj.TextColor + Number(colorObj.alpha).toString(16) );
  else
    putValueToControl( colorWell, colorObj.TextColor );
}
