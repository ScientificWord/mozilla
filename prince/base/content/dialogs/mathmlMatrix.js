// Copyright (c) 2004 MacKichan Software, Inc.  All Rights Reserved.
/*  Copyright 2006 by MacKichan Software, Inc. */

var gRows;
var gColumns;
var gDialog;
var gCellID;
var target;

// dialog initialization code
function Startup()
{
  target=window.arguments[0];
  gDialog.rowsInput      = document.getElementById("rowsInput");
  if (target.rows > 0)
  {
   gRows = target.rows;
   dDialog.rowsInput.value = gRows;
  }
  gDialog.columnsInput   = document.getElementById("columnsInput");
  if (target.cols > 0)
  {
   gColumns = target.cols;
   dDialog.columnsInput.value = gColumns;
  }
  gDialog.OkButton = document.documentElement.getButton("accept");
  gDialog.sizeLabel = document.getElementById("sizeLabel");
  SelectSizeFromText();
  SetTextboxFocusById("rowsInput");
  SetWindowLocation();
}


function onOK() 
{
  target.rows = gRows;
  target.cols = gColumns;
  target.rowSignature = "";
  MakePersistsValue(gDialog.rowsInput);
  MakePersistsValue(gDialog.columnsInput);
  return(true);
}

function onCancel()
{
  target.rows = 0;
  target.cols = 0;
  target.rowSignature = "";
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
  gColumns  = (gCellID % 10);
  gRows     = Math.ceil(gCellID / 10);

  gDialog.rowsInput.value    = gRows;
  gDialog.columnsInput.value = gColumns;
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
  numCellID = Math.min(gRows-1,7)*10 + Math.min(gColumns,8);
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
  gColumns = Number(gDialog.columnsInput.value);
  gCellID = Math.min(gRows-1,7)*10 + Math.min(gColumns,8);
  DisplaySize();
}