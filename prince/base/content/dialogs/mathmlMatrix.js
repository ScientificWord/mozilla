// Copyright (c) 2004 MacKichan Software, Inc.  All Rights Reserved.
/*  Copyright 2006 by MacKichan Software, Inc. */

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

function nCharacters(n, c) {
  if ( n <= 0 || n > 50) return "";
  var i, theString="";
  for (i = 0; i < n; i++) theString += c;
  return theString;
}

// dialog initialization code
function Startup()
{
  var prefs = GetPrefs();
  var dialogFields = ["rowsInput","colsInput","sizeLabel","columnalign","baseline","flavor"];
  target=window.arguments[0];
  var node = target.node;
  setUpDialogObject(dialogFields, gDialog);  //avoids use of getElementById all the time

  gDialog.rowsInput.value = gRows = (node && node.getAttribute("rows")) || target.rows || prefs.getIntPref("swp.matrix.rows") || 3;
  gDialog.colsInput.value = gCols = (node && node.getAttribute("cols")) || target.cols || prefs.getIntPref("swp.matrix.cols") || 3;
  gDialog.columnalign.value = (node && node.getAttribute("colalign")) || target.columnalign || prefs.getCharPref('swp.matrixdef.colalign') || "c";
  gDialog.baseline.value = (node && node.getAttribute("baseline")) || target.baseline || prefs.getCharPref('swp.matrixdef.baseline') || "";
  gDialog.flavor.value = (node && node.getAttribute("flavor")) || target.flavor || prefs.getCharPref('swp.matrixdef.delim') || "";
  SelectSizeFromText();
  SetTextboxFocusById("rowsInput");
  SetWindowLocation();
}

function onOK() 
{
  target.baseline = gDialog.baseline.value;
  target.colalign = gDialog.columnalign.value;
  target.flavor = gDialog.flavor.value;
  target.cols = gDialog.colsInput.value;
  target.rows = gDialog.rowsInput.value;
  target.rowSignature = nCharacters(target.cols, target.colalign); 
  target.cancel = false;
  return(true);
}

function onCancel()
{
  target.cancel = true;
  return(true);
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