// Copyright (c) 2004 MacKichan Software, Inc.  All Rights Reserved.
/*  Copyright 2006 by MacKichan Software, Inc. */

Components.utils.import("resource://app/modules/msiEditorDefinitions.jsm");

var gRows;
var gCols;
var gDialog;
var gCellID;
var target;

function setUpDialogObject(names, dialogobj) {   // this is a candidate for the utilities object
  for (nm in names) {
    dialogobj[names[nm]] = document.getElementById(names[nm]);
  }
}

function matrixRowsAndColumns(matrixNode) {
  var returnObject = {rows: null, columns: null};
  var rowlist;
  if (matrixNode) {
    rowlist = matrixNode.getElementsByTagName('mtr');
    returnObject.rows = rowlist.length;
    if (returnObject.rows > 0) returnObject.columns = 
      rowlist.item(0).getElementsByTagName('mtd').length;
  }
  return returnObject;
}

function nCharacters(n, c) {
  if ( n <= 0 || n > 50) return "";
  var i, theString="";
  for (i = 0; i < n; i++) theString += c;
  return theString;
}

// dialog initialization code
function Startup()
{
  try {
    var prefs = GetPrefs();
    var dialogFields = ["rowsInput","colsInput","sizeLabel","columnalign","baseline","flavor","matrixIsSmall","alignments"];
    var node;
    var rowsAndColumns;
    var flavorNull;
    target = window.arguments[0];
    node = target.node;


    setUpDialogObject(dialogFields, gDialog);  //avoids use of getElementById all the time

    rowsAndColumns = matrixRowsAndColumns(node);
    flavorNull = !(node && node.getAttribute("flv"));
    gDialog.rowsInput.value = gRows = (rowsAndColumns.rows) || target.rows || prefs.getIntPref("swp.matrix.rows",3);
    gDialog.colsInput.value = gCols = (rowsAndColumns.columns) || target.cols || prefs.getIntPref("swp.matrix.cols",3);
    gDialog.columnalign.value = (node && node.hasAttribute("rowSignature") ? node.getAttribute('rowSignature').slice(0,1) : null) 
      || target.columnalign || prefs.getCharPref('swp.matrixdef.colalign','c');
    gDialog.baseline.value = ((node && node.getAttribute("baseline") ? node.getAttribute("baseline") : "")) || target.baseline || prefs.getCharPref('swp.matrixdef.baseline','');
    gDialog.matrixIsSmall.checked = (node && node.getAttribute("flv") && node.getAttribute("flv").indexOf("small") >= 0) || (!node && prefs.getBoolPref("swp.matrixdef.small", false));
    if (flavorNull) {
      gDialog.flavor.value = "";
    }
    else {
      gDialog.flavor.value = node.getAttribute("flv") || target.flavor || prefs.getCharPref('swp.matrixdef.delim','').replace('small','');
    }

    SelectSizeFromText();
    SetTextboxFocusById("rowsInput");
    // Get disabling consistent with values
    onChangeData(gDialog.matrixIsSmall);
    onChangeData(gDialog.flavor);

    SetWindowLocation();
  }
  catch(e) {
    e.message;
  }
}

function onOK() 
{
  var isSmall;
  var isArray;
  isSmall = gDialog.matrixIsSmall.checked;
  target.baseline = gDialog.baseline.disabled ? null :gDialog.baseline.value;
  target.colalign = gDialog.columnalign.disabled ? null :gDialog.columnalign.value;;
  target.flavor = gDialog.flavor.value;
  target.cols = gDialog.colsInput.value;
  target.rows = gDialog.rowsInput.value;

  isArray = !isSmall && (!target.flavor || target.flavor === '');
  if (!(gDialog.columnalign.disabled)) {
    if (isArray) {
      target.rowSignature = nCharacters(target.cols, target.colalign); 
    }
    else {
      target.rowSignature = target.colalign;
    }    
  } else target.rowSignature = null;
  if (isSmall) {
    target.req = null;
    target.flavor = target.flavor + 'small';
  }
  else {
    target.req = 'mathtools';
  }
  target.cancel = false;
  return(true);
}

function onCancel()
{
  target.cancel = true;
  return(true);
}

// When 'element' is changed, redo any enabling or disabling that is required
function onChangeData(element) {
  try {
    switch (element.id) {
      case ('matrixIsSmall') :
        if (element.checked) { // disable items that can't be made small
          gDialog.columnalign.disabled = gDialog.baseline.disabled = 
            document.getElementById("lcases").disabled = 
            document.getElementById("rcases").disabled = true;
        }
        else {
          document.getElementById("lcases").disabled = document.getElementById("rcases").disabled = false;
          gDialog.columnalign.disabled = 
            (gDialog.flavor.value == 'cases' || gDialog.flavor.value == 'rcases');
          gDialog.baseline.disabled = gDialog.flavor.value !=='';
        }
        break;
      case('flavor') :
        gDialog.matrixIsSmall.disabled = 
          (gDialog.flavor.value == 'cases' || gDialog.flavor.value == 'rcases');
        gDialog.columnalign.disabled = (gDialog.flavor.value == 'cases' || gDialog.flavor.value == 'rcases');
        gDialog.baseline.disabled = (gDialog.flavor.value !=='');
        break;
    }
  }
  catch(e) {
    e.message;
  }
}

function SelectArea(cell)
{
  var cellID    = cell.id;
  var numCellID = Number(cellID.substr(1));

  // early way out if we can...
  if (gCellID == numCellID)
    return;

  gCellID = numCellID;

  var i, anyCell;
  for (i = 1; i < 80; i += 10)
  {
    anyCell = document.getElementById("c"+i);
    while (anyCell)
    {
      anyCell.removeAttribute("class");
      anyCell = anyCell.nextSibling;
    }
  }

  for (i = numCellID; i > 0; i -= 10)
  {
    anyCell = document.getElementById("c"+i);
    while (anyCell)
    {
      anyCell.setAttribute("class", "selected");
      anyCell = anyCell.previousSibling;
    }
  }
  ShowSize();
}


// this function shows the size of the displayed rectangle on the grid. 
function ShowSize()
{
  gDialog.sizeLabel.value = Math.ceil(gCellID / 10) + " x " + (gCellID % 10);
}

function MakePersistsValue(elt)
{
  elt.setAttribute("value", elt.value);
}

function SelectSize()
{
  gCols  = (gCellID % 10);
  gRows     = Math.ceil(gCellID / 10);

  gDialog.rowsInput.value    = gRows;
  gDialog.colsInput.value = gCols;
  DisplaySize();
}

function DisplaySize()
{
  var i, anyCell;
  for (i = 1; i < 80; i += 10)
  {
    anyCell = document.getElementById("c"+i);
    while (anyCell)
    {
      anyCell.removeAttribute("mark");
      anyCell = anyCell.nextSibling;
    }
  }

  var numCellID;
  numCellID = Math.min(gRows-1,7)*10 + Math.min(gCols,8);
  for (i = numCellID; i > 0; i -= 10)
  {
    anyCell = document.getElementById("c"+i);
    while (anyCell)
    {
      anyCell.setAttribute("mark", "true");
      anyCell = anyCell.previousSibling;
    }
  }
  ShowSize();
}



function SelectSizeFromText()
{
  gRows = Number(gDialog.rowsInput.value);
  gCols = Number(gDialog.colsInput.value);
  gCellID = Math.min(gRows-1,7)*10 + Math.min(gCols,8);
  DisplaySize();
}