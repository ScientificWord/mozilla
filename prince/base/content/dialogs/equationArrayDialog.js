/* -*- Mode: Java; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * ***** BEGIN LICENSE BLOCK *****
Copyright 2007 MacKichan Software, Inc.
 * ***** END LICENSE BLOCK ***** */

var data;

var subEquationControls = ["wholeDisplayKeyLabel", "wholeDisplayKeyList", "subequationContinuationCheckbox"];
var alignmentControls = ["alignStandardRadio", "alignSingleEqnRadio", "alignCenteredRadio"];
var lineKeyControls = ["lineKeyLabel", "keyForLineAutoCompleteBox"];

function Startup()
{
//  dump("Entering equationArrayDialog.Startup\n");
  var editorElement = msiGetParentEditorElementForDialog(window);
  var editor = msiGetEditor(editorElement);
//  var editor = GetCurrentEditor();
  if (!editor) {
    window.close();
    return;
  }

  setDialogVars();
  doSetOKCancel(onAccept, onCancel);
  data = window.arguments[0];
//  dump("In equationArrayDialog.Startup, before setDataFromReviseData\n");
  if ((!data.reviseData) || !setDataFromReviseData( data.reviseData ))
  {
    dump("In equationArrayDialog.js, object to revise doesn't seem to be a display? Aborting...\n");
    window.close();
    return;
  }

  // Fill dialog.
//  dump("In equationArrayDialog.Startup, before new msiMarkerList\n");
  gDialog.markerList = new msiKeyMarkerList(window);
  gDialog.markerList.setUpTextBoxControl(gDialog.wholeDisplayKeyList);
  gDialog.markerList.setUpTextBoxControl(gDialog.lineKeyList);
//  var keyString = gDialog.markerList.getIndexString();
//  gDialog.wholeDisplayKeyList.setAttribute("autocompletesearchparam", keyString);
//  gDialog.lineKeyList.setAttribute("autocompletesearchparam", keyString);

  gDialog.lineNumberBox.setAttribute("min", 1);
  gDialog.lineNumberBox.setAttribute("max", data.numLines);

//  gDialog.tabOrderArray = new Array( gDialog.??,
//                                       document.documentElement.getButton("cancel") );

  gDialog.lineNumberBox.valueNumber = data.currLine;
  gDialog.enableSubeqNumberingCheckbox.checked = data.subEqnNumbersEnabled;
  gDialog.subequationContinuationCheckbox.checked = data.subEqnContinuation;
  if (data.reviseData.mTableElement.hasAttribute('alignment'))
  {
    gDialog.enableAlignmentCheckbox.checked = true;
    gDialog.alignmentRadioGroup.value = data.reviseData.mTableElement.getAttribute('alignment');
  }
  else {
    gDialog.enableAlignmentCheckbox.checked = false;
  }
  gDialog.wholeDisplayKeyList.value = data.wholeMarker;

  gDialog.spaceAfterLineUnitsController = new msiUnitsListbox(gDialog.spaceAfterLineUnits, [gDialog.spaceAfterLineBox], msiCSSUnitsList);
  var nCurrLine = data.currLine;
  if (nCurrLine > 0)
    --nCurrLine;
  var numAndUnit = msiCSSUnitsList.getNumberAndUnitFromString(data.lineData[nCurrLine].spaceAfterLineStr);
  var lineSpaceUnits = numAndUnit ? numAndUnit.unit : "pt";
  gDialog.spaceAfterLineUnitsController.setUp(lineSpaceUnits, [data.lineData[nCurrLine].spaceAfterLineStr]);

//  dump("In equationArrayDialog.Startup, before putLineDataToDialog\n");
  putLineDataToDialog(data.currLine);

  doEnabling();
  msiSetInitialDialogFocus(gDialog.lineNumberBox);
}

function setDialogVars()
{
  gDialog.lineNumberBox = document.getElementById("lineNumberBox");
  gDialog.lineKeyList = document.getElementById("keyForLineAutoCompleteBox");
  gDialog.lineNumberingRadioGroup = document.getElementById("lineNumberingRadioGroup");
  gDialog.suppressAnnotationCheckbox = document.getElementById("suppressAnnotationCheckbox");
  gDialog.customLineLabel = document.getElementById("customLineLabel");
  gDialog.spaceAfterLineBox = document.getElementById("spaceAfterLineBox");
  gDialog.spaceAfterLineUnits = document.getElementById("spaceAfterLineUnits");
  gDialog.enableSubeqNumberingCheckbox = document.getElementById("enableSubeqNumberingCheckbox");
  gDialog.subequationContinuationCheckbox = document.getElementById("subequationContinuationCheckbox");
  gDialog.wholeDisplayKeyList = document.getElementById("wholeDisplayKeyAutoCompleteBox");
  gDialog.enableAlignmentCheckbox = document.getElementById("enableAlignmentCheckbox");
  gDialog.alignmentRadioGroup = document.getElementById("alignmentRadioGroup");
}

function setDataFromReviseData( reviseData )
{
  data.currLine = reviseData.thisLine(); //Usually 1?
  data.numLines = reviseData.numberLines();
  data.wholeMarker = reviseData.getMarkerForWholeArray();
  data.subEqnNumbersEnabled = reviseData.subEquationNumbersEnabled();
  data.subEqnContinuation = reviseData.subEquationContinuation();
  data.enableAlignment = reviseData.getAlignmentEnabled();
  data.alignment = reviseData.getAlignment();
  data.lineData = new Array();
  for (var ix = 0; ix < data.numLines; ++ix)
  {
    data.lineData.push(getLineData(reviseData, ix));
  }
  return true;
}

//function putDataToReviseData( reviseData )
//{
//  reviseData.setMarkerForWholeArray(data.wholeMarker);
//  reviseData.setSubEquationNumbersEnabled(data.subEqnNumbersEnabled);
//  reviseData.setSubEquationContinuation(data.subEqnContinuation);
//  reviseData.setAlignmentEnabled(data.enableAlignment);
//  reviseData.setAlignment(data.alignment);
//  reviseData.getSuppressAnnotation(data.suppressAnnotation);
//  for (var ix = 0; ix < data.lineData.length; ++ix)
//  {
//    putLineDataToReviseData(reviseData, ix);
//  }
//}

function getLineData( reviseData, nLine )
{
  var bIsNumbered = reviseData.isLineNumbered(nLine);
  var customLabel = reviseData.getCustomLabelForLine(nLine); //Does this return a DOMFragment or just text????
  var theMarker = reviseData.getMarkerForLine(nLine);
  var spaceAfterLine = reviseData.getSpaceAfterLine(nLine);
  var suppressAnno = reviseData.getSuppressAnnotation(nLine);
  return { bNumbered : bIsNumbered, mCustomLabel : customLabel, mMarker : theMarker, suppressAnnotation : suppressAnno, spaceAfterLineStr : spaceAfterLine };
}

//function putLineDataToReviseData(reviseData, nLine)
//{
//  currData = data.lineData[nLine];
//  reviseData.setLineNumbered(nLine, currData.bNumbered);
//  reviseData.setCustomLabelForLine(nLine, currData.mCustomLabel);
//  reviseData.setMarkerForLine(nLine, currData.theMarker);
//  reviseData.getSpaceAfterLine(nLine, currData.spaceAfterLine);
//}

function lineNumberChanged()
{
  if (gDialog.ignoreLineChange)
    return;
  if (data.currLine == gDialog.lineNumberBox.valueNumber)
    return;

  var nLine = data.currLine;
  if (nLine > 0)
    --nLine;  //since currLine is 1-based
  var currMarker = data.lineData[nLine].mMarker;
  if ( (!currMarker && gDialog.lineKeyList.value.length) || (currMarker && (currMarker != gDialog.lineKeyList.value)) )
  {
    if (gDialog.lineKeyList.value.length && gDialog.lineKeyList.controller && (gDialog.lineKeyList.controller.searchStatus == 4))  //found a match!
    {
      gDialog.ignoreLineChange = true;
      gDialog.lineNumberBox.valueNumber = data.currLine;
      msiPostDialogMessage("dlgErrors.markerInUse", {markerString : gDialog.lineKeyList.value});
      gDialog.lineKeyList.select();
      gDialog.lineKeyList.focus();
      gDialog.ignoreLineChange = false;
      return;
    }
    gDialog.markerList.changeString(currMarker, gDialog.lineKeyList.value);
  }
  getLineDataFromDialog(data.currLine);
  data.currLine = gDialog.lineNumberBox.valueNumber;
  putLineDataToDialog(data.currLine);
  doEnabling();
}

function wholeMarkerChanged()
{
//  if (gDialog.ignoreWholeKeyChange)
//  {
//    gDialog.ignoreWholeKeyChange = false;
//    return;
//  }
//
//  gDialog.ignoreWholeKeyChange = true;
  if ( (!data.wholeMarker && gDialog.wholeDisplayKeyList.value.length) || (data.wholeMarker && (gDialog.wholeDisplayKeyList.value != data.wholeMarker)) )
  {
    if (gDialog.wholeDisplayKeyList.value.length && gDialog.wholeDisplayKeyList.controller && (gDialog.wholeDisplayKeyList.controller.searchStatus == 4))  //found a match!
    {
      msiPostDialogMessage("dlgErrors.markerInUse", {markerString : gDialog.wholeDisplayKeyList.value});
      gDialog.wholeDisplayKeyList.select();
      gDialog.wholeDisplayKeyList.focus();
      return;
    }
    gDialog.markerList.changeString(data.wholeMarker, gDialog.wholeDisplayKeyList.value);
    data.wholeMarker = gDialog.wholeDisplayKeyList.value;
  }
//  gDialog.ignoreWholeKeyChange = false;
}

function lineMarkerChanged()
{
//  if (gDialog.ignoreLineKeyChange)
//  {
//    gDialog.ignoreLineKeyChange = false;
//    return;
//  }
//
//  gDialog.ignoreLineKeyChange = true;
  var nLine = data.currLine;
  if (nLine > 0)
    --nLine;  //since currLine is 1-based
  var currMarker = data.lineData[nLine].mMarker;
  if ( (!currMarker && gDialog.lineKeyList.value.length) || (currMarker && (gDialog.lineKeyList.value != currMarker)) )
  {
    if (gDialog.lineKeyList.value.length && gDialog.lineKeyList.controller && (gDialog.lineKeyList.controller.searchStatus == 4))  //found a match!
    {
      msiPostDialogMessage("dlgErrors.markerInUse", {markerString : gDialog.lineKeyList.value});
      gDialog.lineKeyList.select();
      gDialog.lineKeyList.focus();
      return;
    }
    data.lineData[nLine].mMarker = gDialog.lineKeyList.value;
    gDialog.markerList.changeString(currMarker, gDialog.lineKeyList.value);
  }
//  gDialog.ignoreLineKeyChange = false;
}

function onAccept()
{
  gDialog.markerErrors = gDialog.markerList.checkAllChanges();
  if (gDialog.markerErrors.length > 0)
  {
    processMarkerErrors();
    return false;
  }

  data.subEqnNumbersEnabled = gDialog.enableSubeqNumberingCheckbox.checked;
  data.subEqnContinuation = gDialog.subequationContinuationCheckbox.checked;
  data.enableAlignment = gDialog.enableAlignmentCheckbox.checked;
  data.alignment = gDialog.alignmentRadioGroup.value;
  // BBM
  if (data.enableAlignment) {
    data.reviseData.mTableElement.setAttribute('alignment', data.alignment);
  }
  else {
    data.reviseData.mTableElement.removeAttribute('alignment');
  }
  data.wholeMarker = gDialog.wholeDisplayKeyList.value;
  data.numbering = gDialog.lineNumberingRadioGroup.value;
  getLineDataFromDialog(data.currLine);

//  putDataToReviseData( data.reviseData );

  var editorElement = msiGetParentEditorElementForDialog(window);
  var theWindow = window.opener;
  if (!theWindow || !("reviseEqnArray" in theWindow))
    theWindow = msiGetTopLevelWindow();
  theWindow.reviseEqnArray(data.reviseData, data, editorElement);

  SaveWindowLocation();
  return true;

}

function doTest()
{
  gDialog.markerErrors = gDialog.markerList.checkAllChanges();
  if (gDialog.markerErrors.length > 0)
  {
    processMarkerErrors();
    return false;
  }

  data.subEqnNumbersEnabled = gDialog.enableSubeqNumberingCheckbox.checked;
  data.subEqnContinuation = gDialog.subequationContinuationCheckbox.checked;
  data.enableAlignment = gDialog.enableAlignmentCheckbox.checked;
  data.alignment = gDialog.alignmentRadioGroup.value;
  data.wholeMarker = gDialog.wholeDisplayKeyList.value;
  getLineDataFromDialog(data.currLine);

//  putDataToReviseData( data.reviseData );

  var editorElement = msiGetParentEditorElementForDialog(window);
  var theWindow = window.opener;
  if (!theWindow || !("reviseEqnArray" in theWindow))
    theWindow = msiGetTopLevelWindow();
  theWindow.reviseEqnArray(data.reviseData, data, editorElement);

//  SaveWindowLocation();
//  return true;

}

function onCancel()
{
  data.Cancel = true;
}

function getLineDataFromDialog(nWhichLine)
{
  if (nWhichLine > 0)
    --nWhichLine;  //since line counter is 1-based
  var currData = data.lineData[nWhichLine];
  currData.bNumbered = (gDialog.lineNumberingRadioGroup.value != "numberingNone");
  if (gDialog.lineNumberingRadioGroup.value == "numberingCustom")
  {
//    if (gDialog.customContentFilter == null)
//      gDialog.customContentFilter = new msiDialogEditorContentFilter(gDialog.customLineLabel);
//    currData.mCustomLabel = gDialog.customContentFilter.getMarkupString();
    currData.mCustomLabel = gDialog.customLineLabel.value;
    currData.suppressAnnotation = gDialog.suppressAnnotationCheckbox.checked;
  }
  else
    currData.suppressAnnotation = false;
  currData.mMarker = gDialog.lineKeyList.value;
  currData.spaceAfterLineStr = gDialog.spaceAfterLineUnitsController.getValueString(gDialog.spaceAfterLineBox);
}

function putLineDataToDialog(nWhichLine)
{
  if (nWhichLine > 0)
    --nWhichLine;  //since line counter is 1-based
  var currData = data.lineData[nWhichLine];
  if (currData.bNumbered)
  {
    if (currData.mCustomLabel && currData.mCustomLabel!=null)
    {
      gDialog.lineNumberingRadioGroup.value = "numberingCustom";
      gDialog.customLineLabel.value = currData.mCustomLabel;
      gDialog.suppressAnnotationCheckbox.checked = currData.suppressAnnotation;
    }
    else
      gDialog.lineNumberingRadioGroup.value = "numberingAuto";
  }
  else
    gDialog.lineNumberingRadioGroup.value = "numberingNone";

  gDialog.lineKeyList.value = currData.mMarker;
  gDialog.spaceAfterLineUnitsController.setValue(gDialog.spaceAfterLineBox, currData.spaceAfterLineStr);
}

function findLineWithMarker(keyName)
{
  for (var ix = 0; ix < data.lineData.length; ++ix)
  {
    if (data.lineData[ix].mMarker && data.lineData[ix].mMarker == keyName)
      return ix;
  }
  return -1;
}

function processMarkerErrors()
{
  if (!gDialog.markerErrors || !gDialog.markerErrors.length)
    return;
  if (gDialog.markerErrors.indexOf(gDialog.wholeDisplayKeyList.value) >= 0)
  {
    msiPostDialogMessage("dlgErrors.markerInUse", {markerString : gDialog.wholeDisplayKeyList.value});
    gDialog.wholeDisplayKeyList.select();
    gDialog.wholeDisplayKeyList.focus();
    return;
  }
  var nLine = data.currLine;
  if (nLine > 0)
    --nLine;
  var currData = data.lineData[nLine];
  if (gDialog.markerErrors.indexOf(currData.mMarker) >= 0)
  {
    msiPostDialogMessage("dlgErrors.markerInUse", {markerString : gDialog.lineKeyList.value});
    gDialog.lineKeyList.select();
    gDialog.lineKeyList.focus();
    return;
  }

  for (var ix = 0; (ix < data.lineData.length) && (ix != data.currLine - 1); ++ix)
  {
    currData = data.lineData[ix];
    if (gDialog.markerErrors.indexOf(currData.mMarker) >= 0)
    {
      gDialog.ignoreLineChange = true;
      data.currLine = gDialog.lineNumberBox.valueNumber = ix + 1;
      putLineDataToDialog(ix + 1);
      msiPostDialogMessage("dlgErrors.markerInUse", {markerString : gDialog.lineKeyList.value});
      gDialog.lineKeyList.select();
      gDialog.lineKeyList.focus();
      gDialog.ignoreLineChange = false;
      return;
    }
  }
}

function checkEnableControls(changedControl)
{
  var theID = changedControl.getAttribute("id");
  switch(theID)
  {
    case "lineNumberingRadioGroup":
      enableControlsByID(["lineKeyList"], (gDialog.lineNumberingRadioGroup.value != "numberingNone"));
      enableControlsByID(["suppressAnnotationCheckbox"], (gDialog.lineNumberingRadioGroup.value == "numberingCustom"));
    break;
    case "enableSubeqNumberingCheckbox":
      enableControlsByID(subEquationControls, gDialog.enableSubeqNumberingCheckbox.checked);
    break;
    case "enableAlignmentCheckbox":
      enableControlsByID(alignmentControls, gDialog.enableAlignmentCheckbox.checked);
    break;
  }
}

function doEnabling()
{
  enableControlsByID(["lineNumberBox"], (data.numLines > 1));
  enableControlsByID(["lineKeyList"], (gDialog.lineNumberingRadioGroup.value != "numberingNone"));
  enableControlsByID(["enableAlignmentCheckbox"], (data.numLines > 1));
  enableControlsByID(["suppressAnnotationCheckbox"], (gDialog.lineNumberingRadioGroup.value == "numberingCustom"));
  enableControlsByID(subEquationControls, gDialog.enableSubeqNumberingCheckbox.checked);
  enableControlsByID(alignmentControls, gDialog.enableAlignmentCheckbox.checked);
  enableControlsByID(["spaceAfterLineBox", "spaceAfterLineUnits"], (data.numLines > data.currLine));
}


