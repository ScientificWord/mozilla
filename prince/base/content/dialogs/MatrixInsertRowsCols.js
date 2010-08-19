// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

//var gBodyElement;

const defaultMathColor="#000000";
//const cssBackgroundColorStr = "background-color";
//const emptyElementStr=" ";
//const colorStyle = cssColorStr + ": ";

//const mmlns    = "http://www.w3.org/1998/Math/MathML";
//const xhtmlns  = "http://www.w3.org/1999/xhtml";

//var customMathColor;

var data;

// dialog initialization code
function Startup()
{
  var editorElement = msiGetParentEditorElementForDialog(window);
  var editor = msiGetEditor(editorElement);
//  var editor = GetCurrentEditor();
  if (!editor) {
    window.close();
    return;
  }

  doSetOKCancel(onAccept, onCancel);
  data = window.arguments[0];
  data.Cancel = false;

  setDataFromReviseData(data.reviseData, data.reviseCommand);

  gDialog.theTable = document.getElementById("previewTable");
  gDialog.NumberInsertControl = document.getElementById("numberInsertControl");
  gDialog.PositionInsertControl = document.getElementById("positionInsertControl");
  gDialog.MatrixPreview = document.getElementById("insertPreview");
  gDialog.PreviewInitialRow = document.getElementById("previewInitialRow");
  gDialog.CurrMarkerCell = document.getElementById("initialMarkerCell");
  gDialog.PreserveInsertPos = document.getElementById("insertPositions");
  gDialog.currInsertPos = 0;  //Set to reflect the initial configuration of the sample

  InitDialog();

//  window.mMSIDlgManager = new msiDialogConfigManager(window);
//  if (bRevise)
//  {
//    window.mMSIDlgManager.mbIsRevise = true;
//    window.mMSIDlgManager.mbCloseOnAccept = true;
//  }
//  window.mMSIDlgManager.configureDialog();

//  gDialog.LeftBracketGroup.focus();
  msiSetInitialDialogFocus(gDialog.NumberInsertControl);

  SetWindowLocation();
}


function setDataFromReviseData(reviseData, reviseCommand)
{
  gDialog.mMatrixElement = reviseData.getReferenceNode();
  if ( (gDialog.mMatrixElement == null) || (gDialog.mMatrixElement.nodeName != "mtable") )
  {
    dump("Problem in MatrixInsertRowsCols.js, setDataFromReviseData - mtable node not found!\n");
    return;
  }
  gDialog.bInsertCols = (reviseCommand == "cmd_MSIaddMatrixColumnsCmd");
  var tableDims = data.reviseData.getTableDims();
  gDialog.nRows = tableDims.nRows;
  gDialog.nCols = tableDims.nCols;
}


function InitDialog()
{
  var startInsertPos = 0;

  gDialog.tabOrderArray = new Array( gDialog.NumberInsertControl, gDialog.PositionInsertControl,
                                       document.documentElement.getButton("accept"),
                                       document.documentElement.getButton("cancel") );

//  checkInaccessibleAcceleratorKeys(document.documentElement);
  document.documentElement.getButton("accept").setAttribute("default", true);

//  var sampleDependencies = new Array(gDialog.PositionInsertControl);
//  makeSampleWindowDependOn(gDialog.MatrixPreview, sampleDependencies);

  initialDrawSample();

  if (gDialog.bInsertCols)
  {
    startInsertPos = Number(gDialog.PreserveInsertPos.getAttribute("colInsert"));
    if (startInsertPos > gDialog.nCols)
      startInsertPos = gDialog.nCols;
    gDialog.PositionInsertControl.max = gDialog.nCols;
  }
  else
  {
    startInsertPos = Number(gDialog.PreserveInsertPos.getAttribute("rowInsert"));
    if (startInsertPos > gDialog.nRows)
      startInsertPos = gDialog.nRows;
    gDialog.PositionInsertControl.max = gDialog.nRows;
  }
//The following should cause redrawSample() to be called.
  gDialog.PositionInsertControl.value = startInsertPos;
  redrawSample();  //it seems that it didn't, so we'll call it.

//  var sampleStyle = document.defaultView.getComputedStyle(gDialog.BracketPreview, null);
//  var initHeight = sampleStyle.getPropertyValue("height");
//  gDialog.BracketPreview.style.maxHeight = initHeight;
}

function adjustTableFontSize()
{
  var ptSize = "8pt";
  var maxSize = Math.max(gDialog.nCols, gDialog.nRows);
  if (maxSize > 15)
    ptSize = "2pt";
  else if (maxSize > 10)
    ptSize = "3pt";
  else if (maxSize > 8)
    ptSize = "4pt";
  else if (maxSize > 5)
    ptSize = "6pt";
  gDialog.theTable.style.setProperty( "font-size", ptSize, "");
}

//Strategy: 
//  (i) For normal (not over-sized) matrices we size the table in the sample to the size of the matrix plus
//      two additional rows and one additional column (for insertRows) or two additional columns and one additional row (for insertColumns).
//      The extra rows are one each above and below (or just above in the insertColumns case) and the extra column is at the left 
//      (or on left and right in the insertColumns case). The outer cells all have a style showing no visible boundaries.
// (ii) We find the insert position
function initialDrawSample()
{
  var rowNodeToCopy = document.getElementById("previewInitialPlainRow");
  var borderCellNodeToCopy = document.getElementById("previewBorderCell");
  var cellNodeToCopy = document.getElementById("previewOrdinaryCell");

  var rowsList = msiNavigationUtils.getSignificantContents(gDialog.theTable);
  var cellList = null;
  var newCell = null;
  var insertBeforeCell = null;
  if (gDialog.bInsertCols)
  {
    var lastRow = rowsList.pop();
    //Now remove the extra bottom row and add an extra right column
    gDialog.theTable.removeChild( lastRow );
    for (var jx = 0; jx < rowsList.length; ++jx)
    {
      newCell = borderCellNodeToCopy.cloneNode(true);
      rowsList[jx].appendChild(newCell);
    }
    //Also need to move the second cell in the top row to be the first cell in the second row (which was missing on account of rowspan in the marker cell above it)
    insertBeforeCell = msiNavigationUtils.getFirstSignificantChild(rowsList[1]);
    rowsList[1].insertBefore( msiNavigationUtils.getIndexedSignificantChild(rowsList[0], 1), insertBeforeCell );
    gDialog.CurrMarkerCell.setAttribute("rowspan", "1");
    gDialog.CurrMarkerCell.setAttribute("colspan", "2");
    var theSpan = msiNavigationUtils.getFirstSignificantChild(gDialog.CurrMarkerCell);
    theSpan.textContent = String.fromCharCode(0x2193);
  }
  adjustTableFontSize();
  //Now adjust the size - first add needed cells to each row
  for (var jx = 0; jx < rowsList.length; ++jx)
  {
    insertBeforeCell = msiNavigationUtils.getIndexedSignificantChild(rowsList[jx], 1);
    for (var ix = 2; ix < gDialog.nCols; ++ix)
    {
      if ( (jx == 0) || (!gDialog.bInsertCols && (jx == rowsList.length - 1)) )
        newCell = borderCellNodeToCopy.cloneNode(true);
      else
        newCell = cellNodeToCopy.cloneNode(true);
      rowsList[jx].insertBefore(newCell, insertBeforeCell);
    }
    if (gDialog.nCols < 2)  //Must be 1
      rowsList[jx].removeChild(insertBeforeCell);
  }
    //Or remove them if need be:
  //Finally clone the first non-border row and adjust the row size
  for (var ix = 2; ix < gDialog.nRows; ++ix)
  {
    newRow = rowNodeToCopy.cloneNode(true);
    gDialog.theTable.insertBefore(newRow, rowNodeToCopy);
  }
  if (gDialog.nRows < 2)  //Must be 1
    gDialog.theTable.removeChild(rowNodeToCopy);
//  if (gDialog.nCols < 2)  //Must be 1
//  {
//    for (var jx = 0; jx < rowsList.length; ++jx)
//    {
//      cellList = msiNavigationUtils.getSignificantChildren(rowsList[jx]);
//      rowsList[jx].removeChild( cellList[cellList.length - 2] );
//    }   
//  }
}

function changeInsertionPosition()
{
  redrawSample();
}

function redrawSample()  //This function simply moves the pointer cell
{
  function findMarkerCellIn(parentNode, spanAttr)
  {
    if (!parentNode)
      return null;
    if (parentNode.getAttribute(spanAttr) == "2")
      return parentNode;
    var nodeContents = msiNavigationUtils.getSignificantContents(parentNode);
    var retVal = null;
    for (var kk = 0; (!retVal) && (kk < nodeContents.length); ++kk)
      retVal = goFindMarkerCell(nodeContents[kk], spanAttr);
    return retVal;
  }

  function goFindMarkerCell(listOfRows, spanAttr)
  {
    var theMarkerCell = null;
    var nWhichRow = -1;
    for (var jj = 0; (!theMarkerCell) && (jj < listOfRows.length); ++jj)
    {
      theMarkerCell = findMarkerCellIn(listOfRows[jj], spanAttr);
      if (theMarkerCell)
        nWhichRow = jj;
    }
    if (theMarkerCell)
      return {theCell : theMarkerCell, theRow : nWhichRow};
    return null;
  }

  var newMarkerCell, currMarkerCell, cellToMove;
  var cellsList, rowsList;
  var currRow, newRow, nextRow;
  var newInsertPos = gDialog.PositionInsertControl.valueNumber;
  var oldInsertPos = gDialog.currInsertPos;
  var foundMarkerData = null;
  var insertBeforeCell = null;
  if (newInsertPos != gDialog.currInsertPos)
  {
    rowsList = msiNavigationUtils.getSignificantContents(gDialog.theTable);
    currRow = rowsList[0];
    if (gDialog.bInsertCols)
    {
      if (rowsList && rowsList.length)
        cellsList = msiNavigationUtils.getSignificantContents(rowsList[0]);
      if (cellsList)
      {
        if (cellsList.length > gDialog.currInsertPos)
          currMarkerCell = cellsList[gDialog.currInsertPos];
        else
          currMarkerCell = cellsList[cellsList.length-1];
      }
      currMarkerCell = findMarkerCellIn(currMarkerCell, "colspan");
      if (!currMarkerCell)
        currMarkerCell = findMarkerCellIn(currRow, "colspan");
      oldInsertPos = cellsList.indexOf(currMarkerCell);
      if (newInsertPos > oldInsertPos)
        ++newInsertPos;  //Since currMarkerCell will first be deleted from the list, the cell to insert before is currently one further to the right than the target position
      if (newInsertPos >= cellsList.length)
        newInsertPos = cellsList.length;
      if (newInsertPos < cellsList.length)
        insertBeforeCell = cellsList[newInsertPos];  //otherwise it's still null, and the following call will append it (move it to the end)
//      msiKludgeLogNodeContentsAndAllAttributes(insertBeforeCell, ["tableEdit"], "Moving cell to change cols in sample, with oldInsertPos [" + oldInsertPos + "] and newInsertPos [" + newInsertPos + "]: insertBeforeCell ", true);
      currRow.insertBefore(currMarkerCell, insertBeforeCell);
//      currMarkerCell.setAttribute("colspan", "1");  //Would be better to remove this attribute?
//      currMarkerCell.setAttribute("class", "aBorderCell");
//      newMarkerCell.setAttribute("colspan", "2");
//      newMarkerCell.setAttribute("class", "markerBorderCell");
      gDialog.currInsertPos = gDialog.PositionInsertControl.valueNumber;
    }
    else
    {
      if (rowsList)
      {
        if (gDialog.currInsertPos < rowsList.length-1)
          currRow = rowsList[gDialog.currInsertPos]
        else
          currRow = rowsList[rowsList.length - 2];
        cellsList = msiNavigationUtils.getSignificantContents(currRow);
//        newRow = rowList[];
      }
      if (cellsList && cellsList.length)
        currMarkerCell = cellsList[0];
      currMarkerCell = findMarkerCellIn(currMarkerCell, "rowspan");
      if (!currMarkerCell)
      {
        foundMarkerData = goFindMarkerCell(rowsList, "rowspan");
        if (foundMarkerData)
        {
          oldInsertPos = foundMarkerData.theRow;
          currMarkerCell = foundMarkerData.theCell;
        }
      }
      newRow = rowsList[newInsertPos];
      insertBeforeCell = msiNavigationUtils.getFirstSignificantChild(rowsList[newInsertPos]);
//      msiKludgeLogString("Moving cells to change rows in sample, oldinsertPos is [" + oldInsertPos + "] and newInsertPos is [" + newInsertPos + "]\n", ["tableEdit"]);
//      msiKludgeLogNodeContentsAndAllAttributes(insertBeforeCell, ["tableEdit"], "First move: insertBeforeCell ", true);
//      msiKludgeLogNodeContentsAndAllAttributes(currMarkerCell, ["tableEdit"], "First move: currMarkerCell ", true);
      rowsList[newInsertPos].insertBefore(currMarkerCell, insertBeforeCell);
      if (oldInsertPos == newInsertPos - 1)
      {
        insertBeforeCell = msiNavigationUtils.getFirstSignificantChild(rowsList[oldInsertPos]);
        cellToMove = msiNavigationUtils.getFirstSignificantChild(rowsList[newInsertPos+1]);
//        msiKludgeLogNodeContentsAndAllAttributes(insertBeforeCell, ["tableEdit"], "Second move, oldInsertPos=newInsertPos-1 case: insertBeforeCell ", true);
//        msiKludgeLogNodeContentsAndAllAttributes(cellToMove, ["tableEdit"], "Second move, oldInsertPos=newInsertPos-1 case: cellToMove ", true);
        rowsList[oldInsertPos].insertBefore(cellToMove, insertBeforeCell);
      }
      else if (oldInsertPos == newInsertPos + 1)
      {
        cellToMove = insertBeforeCell;  //In this case the setting above is what we want
        insertBeforeCell = msiNavigationUtils.getFirstSignificantChild(rowsList[oldInsertPos+1]);
//        msiKludgeLogNodeContentsAndAllAttributes(insertBeforeCell, ["tableEdit"], "Second move, oldInsertPos=newInsertPos+1 case: insertBeforeCell ", true);
//        msiKludgeLogNodeContentsAndAllAttributes(cellToMove, ["tableEdit"], "Second move, oldInsertPos=newInsertPos+1 case: cellToMove ", true);
        rowsList[oldInsertPos+1].insertBefore(cellToMove, insertBeforeCell);
      }
      else
      {
        cellToMove = insertBeforeCell;  //In this case the setting above is what we want
        insertBeforeCell = msiNavigationUtils.getFirstSignificantChild(rowsList[oldInsertPos]);
//        msiKludgeLogNodeContentsAndAllAttributes(insertBeforeCell, ["tableEdit"], "Second move, all distinct case: insertBeforeCell ", true);
//        msiKludgeLogNodeContentsAndAllAttributes(cellToMove, ["tableEdit"], "Second move, all distinct case: cellToMove ", true);
        rowsList[oldInsertPos].insertBefore(cellToMove, insertBeforeCell);
        insertBeforeCell = msiNavigationUtils.getFirstSignificantChild(rowsList[oldInsertPos+1]);
        cellToMove = msiNavigationUtils.getFirstSignificantChild(rowsList[newInsertPos+1]);
//        msiKludgeLogNodeContentsAndAllAttributes(insertBeforeCell, ["tableEdit"], "Second move, all distinct case: insertBeforeCell ", true);
//        msiKludgeLogNodeContentsAndAllAttributes(cellToMove, ["tableEdit"], "Second move, all distinct case: cellToMove ", true);
        rowsList[oldInsertPos+1].insertBefore(cellToMove, insertBeforeCell);
      }
      gDialog.currInsertPos = newInsertPos;
    }
  }
}

function onAccept()
{
  data.numberToInsert = gDialog.NumberInsertControl.valueNumber;
  data.positionToInsert = gDialog.PositionInsertControl.valueNumber;
  if (gDialog.bInsertCols)
    gDialog.PreserveInsertPos.setAttribute("colInsert", String(data.positionToInsert));
  else
    gDialog.PreserveInsertPos.setAttribute("rowInsert", String(data.positionToInsert));

  var editorElement = msiGetParentEditorElementForDialog(window);
  var theWindow = window.opener;
  if (gDialog.bInsertCols)
  {
    if (!theWindow || !("insertMatrixColumns" in theWindow))
      theWindow = msiGetTopLevelWindow();
//    msiKludgeLogString("In MatrixInsertRowsCols.js, calling insertMatrixColumns with positionToInsert [" + data.positionToInsert + "] and numberToInsert [" + data.numberToInsert + "]\n", ["tableEdit"]);
    theWindow.insertMatrixColumns(gDialog.mMatrixElement, data.positionToInsert, data.numberToInsert, editorElement);
  }
  else
  {
    if (!theWindow || !("insertMatrixRows" in theWindow))
      theWindow = msiGetTopLevelWindow();
//    msiKludgeLogString("In MatrixInsertRowsCols.js, calling insertMatrixRows with positionToInsert [" + data.positionToInsert + "] and numberToInsert [" + data.numberToInsert + "]\n", ["tableEdit"]);
    theWindow.insertMatrixRows(gDialog.mMatrixElement, data.positionToInsert, data.numberToInsert, editorElement);
  }

  SaveWindowLocation();
  return true;
//  return false;
}

function onCancel()
{
  data.Cancel = true;
}

function doAccept()
{
  document.documentElement.getButton('accept').oncommand();
}