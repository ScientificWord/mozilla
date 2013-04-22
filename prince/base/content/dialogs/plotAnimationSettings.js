var graph;
var animAttributes = ["AnimateStart", "AnimateEnd", "AnimateFPS", "AnimateCurrTime"];

function startup(){
//  var graph = window.arguments[0];
  graph = window.arguments[0];
  for (var ii = 0; ii < animAttributes.length; ++ii)
    putValueToControlByID(animAttributes[ii], graph.getValue(animAttributes[ii]));
}

function onAccept() {
  for (var ii = 0; ii < animAttributes.length; ++ii)
    graph.setGraphAttribute(animAttributes[ii], getValueFromControlByID(animAttributes[ii]));
  return true;
}

function onCancel() {
  return true;
}


