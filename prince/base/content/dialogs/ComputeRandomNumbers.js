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

// sadly, this is all wrong because the strings can't be localized
function doSelect(){
  var dist = document.getElementById("dist");
  var p1   = document.getElementById("param1.text");
  var p2   = document.getElementById("param2.text");

  switch (dist.selectedIndex) {
  case 0:  // Beta
    p1.value = "Order";
    p2.value = "Order";
    break;
  case 1:  // Binomial
    p1.value = "Number of Trials";
    p2.value = "Probability of Success";
    break;
  case 2:  // Cauchy
    p1.value = "Median";
    p2.value = "Shape Parameter";
    break;
  case 3:  // Chi-Square
    p1.value = "Degrees of Freedom";
    p2.value = "";
    break;
  case 4:  // Exponential
    p1.value = "Mean Time Between Arrivals";
    p2.value = "";
    break;
  case 5:  // F
    p1.value = "Degrees of Freedom";
    p2.value = "Degrees of Freedom";
    break;
  case 6:  // Gamma
    p1.value = "Shape Parameter";
    p2.value = "Scale Parameter";
    break;
  case 7:  // Normal
    p1.value = "Mean";
    p2.value = "Standard Deviation";
    break;
  case 8:  // Poisson
    p1.value = "Mean Number of Occurences";
    p2.value = "";
    break;
  case 9:  // Student's t
    p1.value = "Degrees of Freedom";
    p2.value = "";
    break;
  case 10:  // Uniform
    p1.value = "Lower End of Range";
    p2.value = "Upper End of Range";
    break;
  case 11:  // Weibull
    p1.value = "Shape Parameter";
    p2.value = "Scale Parameter";
    break;
  default:
    dump("impossible index in doSelect!\n");
  }
}


