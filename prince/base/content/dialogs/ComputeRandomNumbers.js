// Copyright (c) 2004 MacKichan Software, Inc.  All Rights Reserved.

var data;

function Startup(){
  data = window.arguments[0];

  var tally = document.getElementById("tally");
  tally.value = data.tally.toString();

  var dist = document.getElementById("dist");
  dist.selectedIndex = data.dist;
}

function OK(){
  data.Cancel = false;

  var tally = document.getElementById("tally");
  data.tally = parseInt(tally.value,10);
//JLF - Needed to get persistence to work.
  tally.setAttribute("value", tally.value);


  var dist = document.getElementById("dist");
  data.dist = dist.selectedIndex;

  var p1 = document.getElementById("param1");
  data.param1 = parseFloat(p1.value);

  var p2 = document.getElementById("param2");
  data.param2 = parseFloat(p2.value);

}

function Cancel(){
  data.Cancel = true;
}

function SetValue(p, str)
// sets the value of p2 and hides or shows it depending on whether the value
// is blank or not
{
  if (str.length > 0) p.parentNode.setAttribute("style", "visibility: visible;")
  else p.parentNode.setAttribute("style", "visibility: hidden;");
  p.value = str;
  return str;
}



// sadly, this is all wrong because the strings can't be localized
function doSelect(){
  var dist = document.getElementById("dist");
  var p1   = document.getElementById("label1");
  var p2   = document.getElementById("label2");

  switch (dist.selectedIndex) {
  case 0:  // Beta
    p1.value = SetValue(p2,GetComputeString("Order"));
    break;
  case 1:  // Binomial
    p1.value = GetComputeString("NumTrials");
    SetValue(p2,GetComputeString("ProbSuccess"));
    break;
  case 2:  // Cauchy
    p1.value = GetComputeString("Median");
    SetValue(p2,GetComputeString("Shape"));
    break;
  case 3:  // Chi-Square
    p1.value = GetComputeString("DegFreedom");
    SetValue(p2,"");
    break;
  case 4:  // Exponential
    p1.value = GetComputeString("MTBA");
    SetValue(p2,"");
    break;
  case 5:  // F
    p1.value = SetValue(p2,GetComputeString("DegFreedom"));
    break;
  case 6:  // Gamma
    p1.value = GetComputeString("Shape");
    SetValue(p2,GetComputeString("Scale"));
    break;
  case 7:  // Normal
    p1.value = GetComputeString("Mean");
    SetValue(p2,GetComputeString("StdDev"));
    break;
  case 8:  // Poisson
    p1.value = GetComputeString("MeanNoOfOccur");
    SetValue(p2,"");
    break;
  case 9:  // Student's t
    p1.value = GetComputeString("DegFreedom");
    SetValue(p2,"");
    break;
  case 10:  // Uniform
    p1.value = GetComputeString("Lower");
    SetValue(p2,GetComputeString("Upper"));
    break;
  case 11:  // Weibull
    p1.value = GetComputeString("Shape");
    SetValue(p2,GetComputeString("Scale"));
    break;
  default:
    dump("impossible index in doSelect!\n");
  }
}

var gComputeStringBundle;

function GetComputeString(name)
{
  if (!gComputeStringBundle)
  {
    try {
      var strBundleService =
          Components.classes["@mozilla.org/intl/stringbundle;1"].getService(); 
      strBundleService = 
          strBundleService.QueryInterface(Components.interfaces.nsIStringBundleService);

      gComputeStringBundle = strBundleService.createBundle("chrome://prince/locale/compute.properties"); 

    } catch (ex) {}
  }
  if (gComputeStringBundle)
  {
    try {
      return gComputeStringBundle.GetStringFromName(name);
    } catch (e) {}
  }
  return null;
}

