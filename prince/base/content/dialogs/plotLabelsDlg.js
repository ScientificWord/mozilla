var graph;
var dim = 2;
var itemNumberControl;
var animAttributes = ["AnimateStart", "AnimateEnd", "AnimateFPS"];
var isCartesian = true;
var isHeaderFooter = false;
var isBillboarding = true;
var usingDummy = true;
var currLabelNum = 0;
var originalPlotLabels = [];

function startup(){
//  var graph = window.arguments[0];
  graph = window.arguments[0];
  var nLabels = graph.plotLabels.length;
  storeOriginalPlotLabels();  //in case the user cancels the dialog, want to be able to restore previous state
  itemNumberControl = document.getElementById("labelItemNumber");
  itemNumberControl.max = nLabels;
  if (nLabels > 0)
    itemNumberControl.valueNumber = 1;
  else
    itemNumberControl.valueNumber = 0;
  document.getElementById("hasLabels").setAttribute("disabled", ((nLabels > 0) ? "false" : "true") );
  dim = graph.getDimension();
  isBillboarding = (dim != 2);
  document.getElementById("is2D").hidden = (dim != 2);
  document.getElementById("is3D").hidden = (dim != 3);
  onChangeItem();
}

function mapAttributesToControls(attr)
{
  var ctrlID;
  switch(attr)
  {
    case "OrientationX":
    case "OrientationY":
    case "OrientationZ":
      if (isCartesian)
        ctrlID = attr;  //otherwise remains null
    break;

    case "Turn":
      if (!isCartesian)
        ctrlID = attr;  //otherwise remains null
//        ctrlID = "OrientationX";
    break;
    case "Tilt":
      if (!isCartesian && (dim == 3))
        ctrlID = attr;  //otherwise remains null
//        ctrlID = "OrientationY";  //otherwise remains null
    break;

    default:    ctrlID = attr;            break;
  } 
  if (ctrlID)
    return document.getElementById(ctrlID);
}

function onAddItem()
{
  var newNum;
  if (usingDummy)  //just to have something to display, we had a dummy item - now it'll be real
  {
    usingDummy = false;
    newNum = 1;
  }
  else
  {
//    var theLabel = new PlotLabel(dim);
    newNum = graph.addNewPlotLabel() + 1;
  }
  itemNumberControl.max = newNum;
  itemNumberControl.valueNumber = newNum;
  onChangeItem();
}

function onDeleteItem()
{
  var newNum = graph.plotLabels.length - 1;
  graph.deletePlotLabel(currLabelNum - 1);
  itemNumberControl.max = newNum;
  if (newNum == 0)
    itemNumberControl.min = 0;  //always move the min to 1 when items are present, so we have to reset now
  currLabelNum = 0;  //to avoid the getValuesFromDialog call in onChangeItem
  itemNumberControl.valueNumber = newNum;
  onChangeItem();
}

function onChangeItem()
{
  if (currLabelNum > 0)
    getValuesFromDialog();

  currLabelNum = itemNumberControl.valueNumber;
  var theLabel;
  if (currLabelNum > 0)
  {
    theLabel = graph.plotLabels[currLabelNum-1];
    usingDummy = false;
    itemNumberControl.min = 1;  //don't want the user to be able to select item 0
  }
  else
  {
//    theLabel = new PlotLabel(dim);
    graph.addNewPlotLabel();
    theLabel = graph.plotLabels[0];
    usingDummy = true;
  }
  var attrs = theLabel.plotLabelAttributeList();
  var value, ctrl;
  isCartesian = true;
  isHeaderFooter = false;
  setValuesToDialog(attrs);
  hideOrDisableControls();
}

function onChangeCoordinates()
{
  var newVal = document.getElementById("OrientationType").value;
  var plotLabel = getCurrentPlotLabel();
  var oldAttrArray = ["OrientationX","OrientationY"];
  var newAttrArray = ["Turn"];
  var temp;
  if (dim == 3)
  {
    oldAttrArray.push("OrientationZ");
    newAttrArray.push("Tilt");
  }
  if (newVal == "cartesian")
  {
    temp = newAttrArray;
    newAttrArray = oldAttrArray;
    oldAttrArray = temp;
  }
  oldAttrArray.push("OrientationType");  //after we get old coordinates, set orientation cartesian or not to force theLabel to do conversion
  getValuesFromDialog(oldAttrArray);
  isCartesian = (newVal == "cartesian");
  hideOrDisableControls();
  setValuesToDialog(newAttrArray);
  hideOrDisableControls();
}

function onPositionChoiceChange()
{
  var value = document.getElementById("PositionType").value;
  isHeaderFooter = ((value == "header") || (value == "footer"));
  hideOrDisableControls();
}

function changeBillboarding()
{
  isBillboarding = document.getElementById("Billboarding").checked;
  hideOrDisableControls();
}

function openFontSettingDlg()
{
  var attrMap = {face : "TextFontFamily", size : "TextFontSize", bold : "TextFontBold",
                  italic : "TextFontItalic", color : "TextFontColor"};
  var value, attrName;
  var theLabel = getCurrentPlotLabel();
  var fontObj = {whichFont : "TextLabel",
                 face : theLabel.getPlotLabelAttribute("TextFontFamily", true),
                 size : theLabel.getPlotLabelAttribute("TextFontSize", true),
                 bold : theLabel.getPlotLabelAttribute("TextFontBold", true),
                 italic : theLabel.getPlotLabelAttribute("TextFontItalic", true),
                 color : theLabel.getPlotLabelAttribute("TextFontColor", true),
                 Canceled : false};
  var fontDefaults = {face : graph.getValue("AxisFontFamily"),
                      size : graph.getValue("AxisFontSize"),
                      bold : graph.getValue("AxisFontBold"),
                      italic : graph.getValue("AxisFontItalic"),
                      color : graph.getValue("AxisFontColor")};
//  alert("Plot Font dialog not implemented!");
  openDialog('chrome://prince/content/plotFontSettings.xul', 'Axis Font Settings', 'chrome,close,titlebar,modal,resizable', fontObj, fontDefaults);
  if (!fontObj.Canceled)
  {
    for (var key in attrMap)
    {
      attrName = attrMap[key];
      value = fontObj[key];
      if (value && value != "")
        theLabel.setPlotLabelAttribute(attrName, value);
      else
        theLabel.setPlotLabelAttribute(attrName, "");
    }
  }  
}

function getValuesFromDialog(attrArray)
{
  var ii, value, ctrl;
  var theLabel = getCurrentPlotLabel();
  if (!theLabel)
  {
    msidump("Error in plotLabelsDlg.getValuesFromDialog! No item number " + currLabelNum + "!\n");
    return;
  }
  if (!attrArray)
  {
    getValuesFromDialog(theLabel.plotLabelKeyAttributeList());  //do this first so theLabel will produce the right list in the next call:
    getValuesFromDialog(theLabel.plotLabelAttributeList());
    return;
  }

  for (var ii = 0; ii < attrArray.length; ++ii)
  {
    ctrl = mapAttributesToControls(attrArray[ii]);
    if (ctrl && !ctrl.hidden)
    {
      value = getValueFromControl(ctrl);
      if (value && (value != "") && (value != "undefined"))
        theLabel.setPlotLabelAttribute(attrArray[ii], value);
      else
        theLabel.setPlotLabelAttribute(attrArray[ii], null);
    }
  }
}

function setValuesToDialog(attrArray)
{
  var theLabel = getCurrentPlotLabel();
  if (!theLabel)
  {
    msidump("Error in plotLabelsDlg.setValuesToDialog! No item number " + currLabelNum + "!\n");
    return;
  }
  if (!attrArray)
    attrArray = theLabel.plotLabelAttributeList();
  var ctrl, value;
  for (var ii = 0; ii < attrArray.length; ++ii)
  {
    ctrl = mapAttributesToControls(attrArray[ii]);
    if (ctrl && !ctrl.hidden)
    {
      value = theLabel.getPlotLabelAttribute(attrArray[ii]);
      // if (value)
        putValueToControl(ctrl, value);
      if (attrArray[ii] == "OrientationType")
        isCartesian = (value == "cartesian");
      else if (attrArray[ii] == "PositionType")
        isHeaderFooter = ((value == "header") || (value == "footer"));
      else if (attrArray[ii] == "Billboarding")
        isBillboarding = (value == "true");
    }
  }
}

function getCurrentPlotLabel()
{
  if (currLabelNum == 0)  //shouldn't really call for this if we're "usingDummy", but okay
    return graph.plotLabels[0];
  return graph.plotLabels[currLabelNum - 1];
}

function hideOrDisableControls()
{
  document.getElementById("notHeaderFooter").setAttribute( "disabled", (isHeaderFooter ? "true" : "false"));
//  document.getElementById("hasLabels").setAttribute( "disabled", (usingDummy ? "true" : "false"));
  if (usingDummy)
    document.getElementById("hasLabels").setAttribute("disabled", "true");
  else
    document.getElementById("hasLabels").removeAttribute("disabled");
  document.getElementById("orientationCartesian").hidden = !isCartesian;
  document.getElementById("orientationNotCartesian").hidden = isCartesian;
  if (isHeaderFooter || isBillboarding || usingDummy)
    document.getElementById("hasOrientation").setAttribute( "disabled", "true");
  else
    document.getElementById("hasOrientation").removeAttribute( "disabled");
  document.getElementById("orientation2Vals").hidden = ( (dim == 2) && (!isCartesian) );
  document.getElementById("orientation3Vals").hidden = ( (dim == 2) || (!isCartesian) );
  if (isHeaderFooter || isBillboarding || usingDummy)
    document.getElementById("hasOrientation").setAttribute( "disabled", "true");
  else
    document.getElementById("hasOrientation").removeAttribute( "disabled");
  if (isHeaderFooter || usingDummy)
    document.getElementById("hasVerticalAlignment").setAttribute( "disabled", "true");
  else
    document.getElementById("hasVerticalAlignment").removeAttribute( "disabled");
}

function onAccept() {
  if (usingDummy)
  {
    graph.deletePlotLabel(0);
    return true;
  }
  getValuesFromDialog();
  return true;
}

function onCancel() {
  restoreOriginalPlotLabels();
  return true;
}

function storeOriginalPlotLabels()
{
  for (var ix = 0; ix < graph.plotLabels.length; ++ix)
    originalPlotLabels.push( graph.plotLabels[ix].clone());
}

function restoreOriginalPlotLabels()
{
  for (var ix = graph.plotLabels.length-1; ix >= 0; --ix)
    graph.deletePlotLabel(ix);
  for (ix = 0; ix < originalPlotLabels.length; ++ix)
    graph.addPlotLabel(originalPlotLabels[ix]);
}
