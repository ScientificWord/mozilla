var graph;
var animAttributes = ["AnimateStart", "AnimateEnd", "AnimateFPS"];

function startup(){
//  var graph = window.arguments[0];
  var graph = window.arguments[0];
  for (var ii = 0; ii < animAttributes.length; ++ii)
    putValueToControl(animAttributes[ii], graph.getValue(animAttributes[ii]));
}

function onAccept() {
  for (var ii = 0; ii < animAttributes.length; ++ii)
    graph.setGraphAttribute(animAttributes[ii], getValueFromControl(animAttributes[ii]));
  return true;
}

function onCancel() {
  return true;
}


