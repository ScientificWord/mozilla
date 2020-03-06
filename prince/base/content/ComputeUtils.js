// Copyright (c) 2004 MacKichan Software, Inc.  All Rights Reserved.
// BBM: This file is misnamed since these functions can get called from Scientific Word
#include productname.inc


function GetMathAsString(math)
{
  var ser = new XMLSerializer();
  var mathstr = ser.serializeToString(math);
  var nextNode, prevNode;
  var localname = math.localName;
  if (localname !== "math"){
//dump("###" + math.localName + " !== \"math\"");
    mathstr = "<math>" + mathstr + "</math>";
  }
  // prevNode = math.previousSibling;
  // while (prevNode && prevNode.nodeType != Node.ELEMENT_NODE)
  //   prevNode = prevNode.previousSibling;
  // if (prevNode && prevNode.localName == "math") {
  //   mathstr = ser.serializeToString(prevNode) + mathstr;
  // }
  // nextNode = math.nextSibling;
  // while (nextNode && nextNode.nodeType != Node.ELEMENT_NODE)
  //   nextNode = nextNode.nextSibling;
  // if (nextNode && nextNode.localName == "math") {
  //   mathstr = mathstr + ser.serializeToString(nextNode);
  // }

  // risky string surgery, but it works in simple cases
  mathstr = mathstr.replace(/ _moz_dirty=\"\"/g,"");
  mathstr = mathstr.replace(/<mi\/\>/g,"");
  // the following namespace problems happen with inserted computation results...need a better solution
  mathstr = mathstr.replace(/ xmlns(:a0)?=\"http:\/\/www.w3.org\/1998\/Math\/MathML\"/g,"");
  mathstr = mathstr.replace(/<a0:/g,"\<");
  mathstr = mathstr.replace(/<\/a0:/g,"\<\/");
  return mathstr;
}

// input:  mathml string possibly containing sequence of digits (i.e. <mn>4</mn><mn>5</mn>)
// output: mathml string with single number (i.e., <mn>45</mn>)
// Remove "</mn><mn>", "</mn><mo>.</mo>", and some other spurious tags
// Order is significant in the following set of transformations
// The result for error cases (e.g., "12..45") is undefined
function CleanMathString(mathstr)
{
  // handle tags inserted when user presses enter key
  mathstr = mathstr.replace(/<br xmlns:[\w]+=\"http:\/\/www\.w3\.org\/1999\/xhtml\"\/>/gi,"");
  // eliminate these useless tags
  mathstr = mathstr.replace(/<\/math><math\s*>/g, "");
  mathstr = mathstr.replace(/<mn\/>/g,"");
  mathstr = mathstr.replace(/<mo\/>/g,"");
  mathstr = mathstr.replace(/<mo>\s*<\/mo>/g,"");
  mathstr = mathstr.replace(/<(\w+:)*mo\/>/g,"");
  mathstr = mathstr.replace(/<mi\/>/g,"");
  mathstr = mathstr.replace(/<mi>\s*<\/mi>/g,"");
  mathstr = mathstr.replace(/<br\/>/g,"");
  // handle decimals: allow for decimal at start of number
  mathstr = mathstr.replace(/<\/mn><mo>\.<\/mo>/g, ".</mn>");
  mathstr = mathstr.replace(/<mo>\.<\/mo><mn>/g, "<mn>.");
  // after handling decimals, collect digits around it
  mathstr = mathstr.replace(/<\/mn>\.<mn>/g,".");
  // collect sequences of digits
  //mathstr = mathstr.replace(/<\/mn>\s*<mn>/g,"");
  mathstr = mergeNums(mathstr);
  // a decimal must have a digit after it
  mathstr = mathstr.replace(/\.<\/mn>/g,".0</mn>");

  // remove empty math operators:  <mo form="prefix"></mo>
  mathstr = mathstr.replace(/<mo form=\"prefix\">\s*<\/mo>/g,"");
  mathstr = mathstr.replace(/<mo form=\"prefix\"\s*\/>/g,"");

  // remove _moz_dirty=""
  mathstr = mathstr.replace (/_moz_dirty=\"\"/g, "");


  return mathstr;
}

function mergeNums(mathstr) {
  var returnstr = "";
  var mathstrcopy = mathstr;
  var regexp = /<\/mn>\s*<mn>/;
  var match;
  var longmatch;
  var longregexp = /<\/mn>\s*<mn>[^>]+<\/mn>/;  // use this to go over the second mn
  match = mathstr.match(regexp);
  longmatch = mathstr.match(longregexp);
  while (match && match.length > 0) {
    returnstr += mathstrcopy.substring(0, match.index);
    if (dontMergeNumbers(mathstrcopy, longmatch.index + longmatch[0].length)) {
      returnstr += match[0];
    }
    mathstrcopy = mathstrcopy.substring(match.index + match[0].length);
    match = mathstrcopy.match(regexp);
    longmatch = mathstrcopy.match(longregexp);
  }
  return returnstr + mathstrcopy;
}

  // we have found "</mn><mn>". Deleting this will merge the
  // two adjacent numbers, but we can't do this if we are in an object
  // with a fixed number of children, such as mfrac, mroot, etc. In these
  // cases, numbers adjacent in the source tree are not adjacent on-screen.
  // We need to find the common parent of these two nodes. We look for the
  // first end tag, skipping over matched pairs of tags if necessary.

function dontMergeNumbers(mathstring, index) {
  // we look for the first closing tag after index. If it is one that has a fixed number
  // of children, we can't merge the numbers.
  var regexp = /<\/[a-z]+/;
  var specialMLTagRe = /msubsup|msub|msup|mover|munder|mfrac|mroot|msqrt/;
  var match;
  match = mathstring.substring(index).match(regexp);
  if (match != null) {
    if (match[0].match(specialMLTagRe)) return true;
  }
  return false;
}


function GetNumAsMathML(num)
{
  var res;
  var base = num.toString();
  res  = '<math xmlns="http://www.w3.org/1998/Math/MathML"><mrow>';
  if (base.substr(0,1) === "-") {
    res += '<mo form="prefix">-</mo>';
    base = base.substr(1);
  }
  res += '<mn>' + base + '</mn>';
  res += '</mrow></math>';
  return res;
}

function WrapInMtext(text)
{
  var res = '<math xmlns="http://www.w3.org/1998/Math/MathML"><mrow>';
  res += '<mtext>' + text + '</mtext>';
  res += '</mrow></math>';
  return res;
}


function HasEmptyMath(element)
{
  var expr = GetMathAsString(element);
  return (expr.indexOf("tempinput=\"true\"") > -1);
}


