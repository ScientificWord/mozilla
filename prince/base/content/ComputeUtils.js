// Copyright (c) 2004 MacKichan Software, Inc.  All Rights Reserved.
#include productname.inc


function GetMathAsString(math)
{
  var ser = new XMLSerializer();
  var mathstr = ser.serializeToString(math);
  if (math.localName != "math")
    mathstr = "<math>" + mathstr + "</math>";
  // risky string surgery, but it works in simple cases
  mathstr = mathstr.replace(/ _moz_dirty=\"\"/g,"");
  mathstr = mathstr.replace(/\<mi\/\>/g,"");
  // the following namespace problems happen with inserted computation results...need a better solution
  mathstr = mathstr.replace(/ xmlns:a0=\"http:\/\/www.w3.org\/1998\/Math\/MathML\"/g,"");
  mathstr = mathstr.replace(/\<a0:/g,"\<");
  mathstr = mathstr.replace(/\<\/a0:/g,"\<\/");
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
  mathstr = mathstr.replace(/<mn\/>/g,"");
  mathstr = mathstr.replace(/<br\/>/g,"");
  // handle decimals: allow for decimal at start of number
  mathstr = mathstr.replace(/<\/mn><mo>\.<\/mo>/g, ".</mn>");
  mathstr = mathstr.replace(/<mo>\.<\/mo><mn>/g, "<mn>.");
  // after handling decimals, collect digits around it
  mathstr = mathstr.replace(/<\/mn>\.<mn>/g,".");
  // collect sequences of digits
  mathstr = mathstr.replace(/<\/mn><mn>/g,"");
  // a decimal must have a digit after it
  mathstr = mathstr.replace(/\.<\/mn>/g,".0</mn>");

  // remove empty math operators:  <mo form="prefix"></mo>
  mathstr = mathstr.replace(/<mo><\/mo>/g,"");
  mathstr = mathstr.replace(/<(\w+:)*mo\/>/g,"");
  mathstr = mathstr.replace(/<mo form=\"prefix\"><\/mo>/g,"");
  mathstr = mathstr.replace(/<mo form=\"prefix\"\/>/g,"");
  mathstr = mathstr.replace(/<mi><\/mi>/g,"");

  // remove _moz_dirty=""
  mathstr = mathstr.replace (/_moz_dirty=\"\"/g, "");


  return mathstr;
}

function GetNumAsMathML(num)
{
  var res;
  var base = num.toString();
  res  = '<math xmlns="http://www.w3.org/1998/Math/MathML"><mrow>';
  if (base.substr(0,1) == "-") {
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


