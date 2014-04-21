/* ***** BEGIN LICENSE BLOCK *****
 Copyright 2009 MacKichan Software, Inc. *
 * ***** END LICENSE BLOCK ***** */
"use strict";

Components.utils.import("resource://app/modules/unitHandler.jsm"); 


//Cancel() is in msiEdDialogCommon.js
var gTableElement;
//var gCellElement;
var gTableCaptionElement;
var globalCellElement;
var globalTableElement;
var gValidateTab;

const defHAlign =   "left";
const centerStr =   "center";  //Index=1
const rightStr =    "right";   // 2
const justifyStr =  "justify"; // 3
const charStr =     "char";    // 4
const defVAlign =   "middle";
const topStr =      "top";
const bottomStr =   "bottom";
const bgcolor = "bgcolor";

//var gCellColor;

const cssBackgroundColorStr = "background-color";

var gRowCount = 1;
var gColCount = 1;
var gLastRowIndex;
var gLastColIndex;
var gNewRowCount;
var unitHandler;
var newTable;
//var gNewColCount;
//var gCurRowIndex;
//var gCurColIndex;
//var gCurColSpan;
var gSelectedCellsType = 1;
const SELECT_CELL = 1;
const SELECT_ROW = 2;
const SELECT_COLUMN = 3;
const RESET_SELECTION = 0;
//var gCellData = { value:null, startRowIndex:0, startColIndex:0, rowSpan:0, colSpan:0,
//                 actualRowSpan:0, actualColSpan:0, isSelected:false
//               };
var gAdvancedEditUsed;
var gAlignWasChar = false;
var gIsMatrix = false;
var gColumns;
var gRows;

/*
From C++:
 0 TABLESELECTION_TABLE
 1 TABLESELECTION_CELL   There are 1 or more cells selected
                          but complete rows or columns are not selected
 2 TABLESELECTION_ROW    All cells are in 1 or more rows
                          and in each row, all cells selected
                          Note: This is the value if all rows (thus all cells) are selected
 3 TABLESELECTION_COLUMN All cells are in 1 or more columns
*/
var gCellID = 12;

//var gSelectedCellCount = 0;
var gOurCellData;
var gInitialCellData;
var gCollatedCellData = {};  //This starts out as the same as the previous, but changes as the user makes selections in the dialog.
var gColElementArray;
var gRowElementArray;

var gTableColor;
var gTableCaptionPlacement;
var gTableBaseline = "baseline";
var gBorderCollapse = "collapse";  //This should be the default???

var gTableChangeData;
var gCurrentSide = "";
var gBorderSides = ["top", "right", "bottom", "left"];

var gSelectionTypeStr = "Cell";
var gApplyUsed = false;
//var gSelection = null;
//var gCellDataChanged = false;
var gCanDelete = false;
var gPrefs = GetPrefs();
var gUseCSS = true;

var gActiveEditorElement;
var gActiveEditor;

var gCellWidthUnit = "pt";
var gCellHeightUnit = "pt";
var gCellFontSize = 10;

var rowHeightControlIDs = ["tableRowHeightLabel", "tableRowHeight"];

var data;

// dialog initialization code

function MakePersistsValue(elt)
{
  elt.setAttribute("value", elt.value);
}

function SelectSize(cell)
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

function SelectArea(cellID)
{
  var numCellID = Number(cellID.substr(1));

  // early way out if we can...
  if (gCellID == numCellID)
    return;

  gCellID = numCellID;

  var i, anyCell;
  for (i = 1; i < 60; i += 10)
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

function ShowSize()
{
  var columns  = (gCellID % 10);
  var rows     = Math.ceil(gCellID / 10);
  gDialog.sizeLabel.value = rows + " x " + columns;
}

function ChangeRowOrColumn(id)
{
  // Allow only integers
  forceInteger(id);

  // Enable OK only if both rows and columns have a value > 0
  var enable = gDialog.rowsInput.value.length > 0 && 
                              gDialog.rowsInput.value > 0 &&
                              gDialog.columnsInput.value.length > 0 &&
                              gDialog.columnsInput.value > 0;

  SetElementEnabled(gDialog.OkButton, enable);
  SetElementEnabledById("AdvancedEditButton1", enable);
  if (enable) 
  {  
    gRows = gDialog.rowsInput.value;  
    gColumns = gDialog.columnsInput.value;
    gCellID = 10*(gColumns) - - gRows;  //forces gRows to a number
    DisplaySize();
  }
}

function setVariablesForControls()
{
  // Get dialog widgets - Table Panel
  gDialog.TabBox =  document.getElementById("TabBox");
  gDialog.TableTab =  document.getElementById("TableTab");
  gDialog.CellsTab =  document.getElementById("CellsTab");
  gDialog.LinesTab =  document.getElementById("LinesTab");
  gDialog.sizeLabel = document.getElementById("sizeLabel");

//  gDialog.tableRowCount = document.getElementById("tableRowCount");
//  gDialog.tableColumnCount = document.getElementById("tableColumnCount");
  gDialog.rowsInput = document.getElementById("rowsInput");
  gDialog.columnsInput = document.getElementById("columnsInput");
  gDialog.widthInput     = document.getElementById("widthInput");

  gDialog.tableRowHeight =  document.getElementById("tableRowHeight");
  gDialog.tableWidthInput =  document.getElementById("tableWidthInput");
  gDialog.unitMenulist = document.getElementById("unitMenulist");
  gDialog.autoWidthCheckbox = document.getElementById("autoWidthCheckbox");

  gDialog.tableLocationList =  document.getElementById("tableLocationList");
  gDialog.floatLocationList =  document.getElementById("floatLocationList");
  gDialog.baselineList =  document.getElementById("baselineList");
  gDialog.captionLocation =  document.getElementById("captionLocation");
  // gDialog.tableBackgroundCW =  document.getElementById("tableBackgroundCW");
  
  // Cells Panel
  gDialog.CellHeightInput = document.getElementById("CellHeightInput");
  gDialog.CellHeightUnits = document.getElementById("CellHeightUnits");
  gDialog.CellWidthInput = document.getElementById("CellWidthInput");
  // gDialog.cellUnitsList = document.getElementById("cellUnitsList");

  gDialog.hAlignChoices = document.getElementById("hAlignChoices");
  gDialog.vAlignChoices = document.getElementById("vAlignChoices");

  gDialog.textwrapchoice = document.getElementById("textwrapchoice");
  // gDialog.cellBackgroundCW = document.getElementById("cellBackgroundCW");

  // Lines Panel
  gDialog.BordersPreviewCenterCell = document.getElementById("BordersPreviewCenterCell");
  gDialog.BorderSideSelectionList = document.getElementById("BorderSideSelectionList");
  gDialog.CellBorderStyleList = document.getElementById("cellBorderStyleList");
  gDialog.CellBorderWidthList = document.getElementById("cellBorderWidthList");
  gDialog.borderCW = document.getElementById("borderCW");
}

function getValueForAllSides(aData, defaultVal)
{
  if ( (aData.top == aData.right) || (aData.top == aData.bottom) || (aData.top == aData.left) )
    return aData.top;
  if ( (aData.right == aData.bottom) || (aData.right == aData.left) )
    return aData.right;
  if (aData.bottom == aData.left)
    return aData.bottom;
  return defaultVal;
}

function setValueForAllSides(aData, aValue)
{
  aData.top = aData.right = aData.bottom = aData.left = aValue;
}

function setUpCollatedCellData(collatedCellData, initialCellData)
{
  collatedCellData.border.style.defaultVal = "solid";
  collatedCellData.border.style.__defineGetter__( "all", function() {return getValueForAllSides(this, this.defaultVal);} );
  collatedCellData.border.style.__defineSetter__( "all", function(aVal) {setValueForAllSides(this, aVal);} );
  collatedCellData.border.width.defaultVal = "medium";
  collatedCellData.border.width.__defineGetter__( "all", function() {return getValueForAllSides(this, this.defaultVal);} );
  collatedCellData.border.width.__defineSetter__( "all", function(aVal) {setValueForAllSides(this, aVal);} );
  collatedCellData.border.color.defaultVal = "black";
  collatedCellData.border.color.__defineGetter__( "all", function() {return getValueForAllSides(this, this.defaultVal);} );
  collatedCellData.border.color.__defineSetter__( "all", function(aVal) {setValueForAllSides(this, aVal);} );
  collatedCellData.size.bHeightSet = (initialCellData.size.bHeightSet != null) && (initialCellData.size.bHeightSet === true);
  collatedCellData.size.bWidthSet = (initialCellData.size.bWidthSet != null) && (initialCellData.size.bWidthSet === true);
}

function setDataFromReviseData(reviseData, commandStr)
{
//  gSelectionTypeStr = reviseData.getSelectionType(commandStr);  //Do we even want to call this?
//  gSelectedCellsType = translateSelectionTypeString(gSelectionTypeStr);
  gTableElement = reviseData.getReferenceNode();
  gIsMatrix = reviseData.isMatrix();
  EnableDisableControls();

  var theBaseline = msiGetHTMLOrCSSStyleValue(gActiveEditorElement, gTableElement, "valign", "vertical-align");
  if (theBaseline && (theBaseline.length > 0) )
    gTableBaseline = theBaseline;
}

function setUpChangeData()
{
  gCellChangeData = 
  {
    border : { style : [], width : [], color : []},
    size : { width : false, height : false },
    align : { halign : false, valign : false },
    wrap : false, background : false, cellType : false
  };
  gTableChangeData = 
  {
    size : { width : false, height : false },
    baseline : false, background : false, caption : false, borderCollapse : false
  };
}

function createCellDataObject(srcData)
{
  var srcBorder = null;
  var srcSize = null;
  var srcAlign = null;
  var srcWrap = "wrap";
  var srcBackground = "transparent";
  var srcCellType = "data";
  if (srcData)
  {
    srcBorder = srcData.border;
    srcSize = srcData.size;
    srcAlign = srcData.align;
    srcWrap = srcData.wrap;
    srcBackground = srcData.background;
    srcCellType = srcData.cellType;
  }
  else
  {
    srcBorder = srcSize = srcAlign = srcWrap = srcBackground = srcCellType = null;
  }
  var retObj = { border      : createCellBorderData(srcBorder),
                 size        : createCellSizeData(srcSize),
                 align       : createCellAlignData(srcAlign),
                 wrap        : srcWrap,
                 background  : srcBackground,
                 cellType    : srcCellType };
  return retObj;
}

function createCellBorderData(srcBorderData)
{
  var borderData = { style : {top : "solid", right : "solid", bottom : "solid", left : "solid"},
                     width : {top : "thin", right : "thin", bottom : "thin", left : "thin"},
                     color : {top : "#000000", right : "#000000", bottom : "#000000", left : "#000000"} };
  var aSide;
  if (srcBorderData)
  {
    for (var ix = 0; ix < gBorderSides.length; ++ix)
    {
      aSide = gBorderSides[ix];
      if (aSide in srcBorderData.style)
        borderData.style[aSide] = srcBorderData.style[aSide];
      else
        delete borderData.style[aSide];
      if (aSide in srcBorderData.width)
        borderData.width[aSide] = srcBorderData.width[aSide];
      else
        delete borderData.width[aSide];
      if (aSide in srcBorderData.color)
        borderData.color[aSide] = srcBorderData.color[aSide];
      else
        delete borderData.color[aSide];
    }
  }
  return borderData;
}

function createCellSizeData(srcSizeData)
{
  var retSizeData = { width : 0.0, height : 0.0 };
  if (srcSizeData)
  {
    if (srcSizeData.width)
      retSizeData.width = srcSizeData.width;
    if (srcSizeData.height)
      retSizeData.height = srcSizeData.height;
  }
  return retSizeData;
}

function createCellAlignData(srcAlignData)
{
  var retAlignData = { halign : "left", valign : "middle" };
  if (srcAlignData) {
    if ("halign" in srcAlignData)
      retAlignData.halign = srcAlignData.halign;
    if ("valign" in srcAlignData)
      retAlignData.valign = srcAlignData.valign;
  }
  return retAlignData;
}

function getBorderSideAttrString(aSide, anAttr)
{
  var whichSideStr = (aSide == "all") ? "" : (aSide + "-");
  return ("border-" + whichSideStr + anAttr);
}

function borderStyleToBorderCollapse(aStyle)
{
      return "collapse";

}

function UseCSSForCellProp(propName)
{
  switch(propName)
  {
    case "border-style":
//      return (!gIsMatrix);
      return false;

    case "width":
    case "cellwidth":
      return (!gIsMatrix && !ShouldSetWidthOnCols());
    case "cellheight":
    case "height":
      return (gIsMatrix || !ShouldSetHeightOnRows());
//      return (!gIsMatrix && !ShouldSetHeightOnRows());
  }
  return true;
}

function ShouldSetWidthOnCols()
{
  return gIsMatrix;
//  return true;
}

function ShouldSetHeightOnRows()
{
//  return gIsMatrix;
  return true;
}

function translateSelectionTypeString(selTypeStr)
{
  switch(selTypeStr)
  {
    case "Row":
      return SELECT_ROW;
    case "Column":
      return SELECT_COLUMN;
//    case "Table":  //For Table mode, doesn't seem to matter what we say the //selection type is. On the other hand, our code will
//                   //  pay attention to the selection type string rather than this //variable.
//    case "Cell":
//    case "CellGroup":
    default:
      return SELECT_CELL;
  }
  return SELECT_CELL;  //put it out here just for redundancy's sake
}

function Startup()
{
  gActiveEditorElement = msiGetParentEditorElementForDialog(window);
  gActiveEditor = msiGetTableEditor(gActiveEditorElement);
//  gActiveEditor = GetCurrentTableEditor();
  if (!gActiveEditor)
  {
    window.close();
    return;
  }

  setVariablesForControls();
  initUnitHandler(null);
  
  data = window.arguments[3];
  if (!data) {
    newTable = true;
    try {
      gTableElement = gActiveEditor.createElementWithDefaults("table");
      gTableElement.removeAttribute("border");
      gTableElement.setAttribute("req","tabulary");
    } 
    catch (e) {

    }
  }
  else {
    newTable = false;
    gTableElement = data.reviseData.mTableElement;
    // We disable resetting row and column count -- the user has more direct ways of doing that.
    document.getElementById("QuicklyTab").setAttribute("collapsed", true);
    document.getElementById("tablegrid").setAttribute("collapsed", true);
    document.getElementById("rowsInput").disabled = true;
    document.getElementById("columnsInput").disabled = true;
    document.getElementById("mainTabBox").selectedTab = document.getElementById("TableTab");
  }

  var theCommand = "cmd_editTable";
  if (data)
  {
    if ("reviseCommand" in data)
      theCommand = data.theCommand;
    
    if ("reviseData" in data)
      setDataFromReviseData(data.reviseData, data.reviseCommand);
    else
    {
      window.close();
      return;
    }
  }


//  if (!gSelection)  //should get set in "setDataFromReviseData" call
//  {
//    try {
//      gSelection = gActiveEditor.selection;
//    } catch (e) {}
//    if (!gSelection) return;
//  }

  if (!gTableElement)
  {
    try {
      gTableElement = gActiveEditor.getElementOrParentByTagName("table", null);
   
    } catch (e) {}
  }
  if(!gTableElement)
  {
    dump("Failed to get table element!\n");
    window.close();
    return;
  }
  globalTableElement = gTableElement.cloneNode(false);

//RWA: All of our data setting should be done in the above call to setDataFromReviseData().
//  What remains is merely to set the controls to reflect values.

//  var tagNameObj = { value: "" };
//  var countObj = { value : 0 };
//  var tableOrCellElement;
//  try {
//   tableOrCellElement = gActiveEditor.getSelectedOrParentTableElement(tagNameObj, countObj);
//  } catch (e) {}

//  if (tagNameObj.value == "td")
//  {
//    // We are in a cell
//    gSelectedCellCount = countObj.value;
//    gCellElement = tableOrCellElement;
//  globalCellElement = gCellElement.cloneNode(false);
//
//    // Tells us whether cell, row, or column is selected
//    try {
//      gSelectedCellsType = gActiveEditor.getSelectedCellsType(gTableElement);
//    } catch (e) {}
//
//    // Ignore types except Cell, Row, and Column
//    if (gSelectedCellsType < SELECT_CELL || gSelectedCellsType > SELECT_COLUMN)
//      gSelectedCellsType = SELECT_CELL;
//
//    // Be sure at least 1 cell is selected.
//    // (If the count is 0, then we were inside the cell.)
////    if (gSelectedCellCount == 0)
////      DoCellSelection();
//
//    // Get location in the cell map
//    var rowIndexObj = { value: 0 };
//    var colIndexObj = { value: 0 };
//    try {
//      gActiveEditor.getCellIndexes(gCellElement, rowIndexObj, colIndexObj);
//    } catch (e) {}
//    gCurRowIndex = rowIndexObj.value;
//    gCurColIndex = colIndexObj.value;
//
//    // We save the current colspan to quickly
//    //  move selection from from cell to cell
//    if (GetCellData(gCurRowIndex, gCurColIndex))
//      gCurColSpan = gCellData.colSpan;
//
//    // Starting TabPanel name is passed in
//    if (window.arguments[1] == "CellPanel")
//      gDialog.TabBox.selectedTab = gDialog.CellTab;
//  }

//  if (gDialog.TabBox.selectedTab == gDialog.TableTab)
//  {
//    // We may call this with table selected, but no cell,
//    //  so disable the Cell Properties tab
//    if(!gCellElement)
//    {
//      // XXX: Disabling of tabs is currently broken, so for
//      //      now we'll just remove the tab completely.
//      //gDialog.CellTab.disabled = true;
//      gDialog.CellTab.parentNode.removeChild(gDialog.CellTab);
//    }
//  }

  // Note: we must use gTableElement, not globalTableElement for these,
  //  thus we should not put this in InitDialog.
  // Instead, monitor desired counts with separate globals
//  var rowCountObj = { value: 0 };
//  var colCountObj = { value: 0 };
//  try {
//    gActiveEditor.getTableSize(gTableElement, rowCountObj, colCountObj);
//  } catch (e) {}
//
//  gRowCount = rowCountObj.value;
//  gLastRowIndex = gRowCount-1;
//  gColCount = colCountObj.value;
//  gLastColIndex = gColCount-1;
//
//
//  // Set appropriate icons and enable state for the Previous/Next buttons
//  SetSelectionButtons();
//
//  // If only one cell in table, disable change-selection widgets
//  if (gRowCount == 1 && gColCount == 1)
//    gDialog.SelectionList.setAttribute("disabled", "true");
//
//  // User can change these via textboxes
//  gNewRowCount = gRowCount;
//  gNewColCount = gColCount;

  // This flag is used to control whether set check state
  //  on "set attribute" checkboxes
  // (Advanced Edit dialog use calls  InitDialog when done)
  gAdvancedEditUsed = false;
  InitDialog();
  gAdvancedEditUsed = true;

  // If first initializing, we really aren't changing anything
//  gCellDataChanged = false;
  setUpChangeData();  //This resets our record of what's changed.

//  if (gDialog.TabBox.selectedTab == gDialog.CellTab)
//    setTimeout("gDialog.SelectionList.focus()", 0);
//  else
//    SetTextboxFocus(gDialog.TableRowsInput);

  window.sizeToContent();

  SetWindowLocation();
}


function InitDialog()
{
  initTablePanel();
  initCellsPanel();
  initLinesPanel();

  // Get Table attributes
//  gDialog.TableRowsInput.value = gRowCount;
//  gDialog.TableColumnsInput.value = gColCount;

//STILL A question what to do with the whole table attributes - width and height, and the table background - and border?.??
//
//  gDialog.TableWidthInput.value = InitPixelOrPercentMenulist(globalTableElement, gTableElement, "width", "TableWidthUnits", gPercent);
//  if (gUseCSS) {
//    gDialog.TableHeightInput.value = InitPixelOrPercentMenulist(globalTableElement, gTableElement, "height",
//                                                                "TableHeightUnits", gPercent);
//  }
//  gDialog.BorderWidthInput.value = globalTableElement.border;

//  gDialog.SpacingInput.value = globalTableElement.cellSpacing;
//  gDialog.PaddingInput.value = globalTableElement.cellPadding;

//  var marginLeft  = GetHTMLOrCSSStyleValue(globalTableElement, "align", "margin-left");
//  var marginRight = GetHTMLOrCSSStyleValue(globalTableElement, "align", "margin-right");
//  var halign = marginLeft.toLowerCase() + " " + marginRight.toLowerCase();
//  if (halign == "center center" || halign == "auto auto")
//    gDialog.TableAlignList.value = "center";
//  else if (halign == "right right" || halign == "auto 0px")
//    gDialog.TableAlignList.value = "right";
//  else // Default = left
//    gDialog.TableAlignList.value = "left";

//  InitCellsPanel();
}


function initUnitHandler(unit)
{
  unitHandler = new UnitHandler();
  var fieldList = [];
  var initUnit = unit;
  fieldList.push(gDialog.CellHeightInput);
  fieldList.push(gDialog.CellWidthInput);
  fieldList.push(gDialog.tableWidthInput);
  unitHandler.setEditFieldList(fieldList);
  if (!unit && gPrefs)
    initUnit = gPrefs.getCharPref("swp.defaultTableUnits");
  gDialog.unitMenulist.setAttribute("value", initUnit);
  try {
    unitHandler.buildUnitMenu(gDialog.unitMenulist, initUnit);
    unitHandler.initCurrentUnit(initUnit);
  document.getElementById("currentunits").setAttribute("value", unitHandler.getDisplayString(initUnit));  }
  catch(e) {
    dump(e.message);
  }  
}


function initTablePanel()
{
  var rowCountObj = { value: 0 };
  var colCountObj = { value: 0 };
  var widthVal = 0;
  var heightVal = 0;
  var widthStr = "";
  var heightStr = "";
  var re;
  var match;
  var currUnit = gTableElement.getAttribute("unit");
  currUnit = currUnit || "pt";
  initUnitHandler(currUnit);
  var tableStyle = gTableElement.getAttribute("style");
  // The next 30 lines seem superfluous, but maybe harmless. Leave them for now.
  if (gTableElement.hasAttribute("width"))
  {
    widthStr = gTableElement.getAttribute("width");
    widthVal = unitHandler.getNumberAndUnitFromString(widthStr);
    if (!widthVal)
      widthVal = unitHandler.getNumberAndUnitFromString(widthStr + currUnit);
  }
  else if (gTableElement.hasAttribute("style"))
  {
    re = /width:\s*(\d*[^;]*)(;|$)/;
//    match = re.exec(gTableElement.getAttribute("style"));
    match = re.exec(tableStyle);
    if (match && match.length > 1) widthVal = unitHandler.getNumberAndUnitFromString(match[1]);
  }
  if (gTableElement.hasAttribute("height"))
  {
    heightStr = gTableElement.getAttribute("height");
    heightVal = unitHandler.getNumberAndUnitFromString(heightStr);
    if (!heightVal)
      heightVal = unitHandler.getNumberAndUnitFromString(heightStr + "pt");
  }
  else if (gTableElement.hasAttribute("style"))
  {
    re = /height:\s*(\d*[^;]*)(;|$)/;
//    match = re.exec(gTableElement.getAttribute("style"));
    match = re.exec(tableStyle);
    if (match && match.length > 1) heightVal = unitHandler.getNumberAndUnitFromString(match[1]);
  }

//  unitsHandler.setEditFieldList([gDialog.tableRowHeight,gDialog.tableWidth]);
  if (widthVal)
  {
    currUnit = widthVal.unit;
    gDialog.autoWidthCheckbox.checked = false;
  }
  else
    gDialog.autoWidthCheckbox.checked = true;
  // unitHandler.initCurrentUnit(currUnit);
  // unitHandler.buildUnitMenu(gDialog.unitMenulist, currUnit);
  checkEnableWidthControls();

  try {
    gActiveEditor.getTableSize(gTableElement, rowCountObj, colCountObj);
  } catch (e) {}

  gRowCount = rowCountObj.value;
  gLastRowIndex = gRowCount-1;
  gColCount = colCountObj.value;
  gLastColIndex = gColCount-1;
//  gDialog.tableRowCount.value = gRowCount;
//  gDialog.tableColumnCount.value = gColCount; 
  gDialog.rowsInput.value = " " + gRowCount;
  gDialog.columnsInput.value = " " + gColCount;
  if (widthVal && widthVal.number) gDialog.tableWidthInput.value = widthVal.number;
// if (heightVal && heightVal.number) gDialog.tableRowHeight.value = (gRowCount>0) ? (heightVal.number/gRowCount) : ""; 
  gDialog.baselineList.value = gTableBaseline;

  // Be sure to get caption from table in doc, not the copied "globalTableElement"
  gTableCaptionElement = gTableElement.caption;
  if (gTableCaptionElement)
  {
    gTableCaptionPlacement = msiGetHTMLOrCSSStyleValue(gActiveEditorElement, gTableCaptionElement, "align", "caption-side");
    if (gTableCaptionPlacement != "top")
      gTableCaptionPlacement = "bottom";
    gDialog.captionLocation.value = gTableCaptionPlacement;
  }

  var pos = gTableElement.getAttribute("pos");
  var placement = gTableElement.getAttribute("placement");

  var longPlacement;
  if (placement=="L") longPlacement = "left";
  else if (placement=="R") longPlacement = "right";
  else if (placement=="I") longPlacement = "inside";
  else if (placement=="O") longPlacement = "outside"; 
  else longPlacement = null;

  if (pos == "center") longPlacement="center";  // trumps
  else if (pos == "inline") longPlacement = null;


  var placeLocation = gTableElement.getAttribute("placeLocation");  
  
  if (longPlacement) {
//    if (placement == "I" || placement == "display") placeLocation = "";
    gDialog.tableLocationList.value = longPlacement;
    if (placeLocation) {
      gDialog.floatLocationList.value = placeLocation;
    }
    else 
    {
      gDialog.floatLocationList.value = "";
    }
  }
  else gDialog.tableLocationList.value = "unspecified";

  checkEnableFloatControl();
  checkEnableLocationControl();
  
}

function initLinesPanel()
{
//  gDialog.BorderSelectionList.value = 
//  gDialog.BorderWidthInput.value = 
  var currSide = "all";  //may want this to be persistent
  gDialog.BorderSideSelectionList.value = currSide; //will trigger setCurrSide()?
  setCurrSide(currSide);

//  gTableColor = msiGetHTMLOrCSSStyleValue(globalTableElement, bgcolor, cssBackgroundColorStr);
//  var backColor = gInitialCellData.background;
//  if (!backColor || (backColor == "transparent"))
//  {
//    backColor = gTableColor;
//    gDialog.BackgroundSelectionRadioGroup.value = "table";
//  }
//  else
//    gDialog.BackgroundSelectionRadioGroup.value = "selection";
//  doInitialPreviewSetup();
//  setColorWell("BackgroundCW", backColor);
//  SetColor("BackgroundCW", backColor);
}

function onChangeTableUnits()
{
  var val = gDialog.unitMenulist.value
  unitHandler.setCurrentUnit(val);
  document.getElementById("currentunits").setAttribute("value", unitHandler.getDisplayString(val));
}


function setCurrSide(newSide)
{
  if (gCurrentSide == newSide)
    return;

  gCurrentSide = newSide;
  // gDialog.CellBorderStyleList.value = gCollatedCellData.border.style[gCurrentSide];
  // gDialog.CellBorderWidthList.value = gCollatedCellData.border.width[gCurrentSide];
  // var cellBorderColor = gCollatedCellData.border.color[gCurrentSide];
  // setColorWell("borderCW", cellBorderColor);
  //Note that this one shouldn't require redrawing sample
//  SetColor("borderCW", cellBorderColor);
}


var cellUnitsHandler;

// function onChangeCellUnits()
// {
//   cellUnitsHandler.setCurrentUnit(gDialog.cellUnitsList.value);
// }

function initCellsPanel()
{
//  var previousValue = gDialog.CellWidthInput.value;
  cellUnitsHandler = unitHandler;
//  cellUnitsHandler.setEditFieldList([gDialog.CellWidthInput,gDialog.CellHeightInput]);
//  cellUnitsHandler.initCurrentUnit(gCellWidthUnit);
//  cellUnitsHandler.buildUnitMenu(gDialog.cellUnitsList, gCellWidthUnit);
//

//  var theUnitsList = new msiCSSWithFontUnitsList(gCellFontSize, "pt");
//  gWidthUnitsController = new msiUnitsListbox(gDialog.CellWidthUnits, [gDialog.CellWidthInput], theUnitsList);
//  gHeightUnitsController = new msiUnitsListbox(gDialog.CellHeightUnits, [gDialog.CellHeightInput], theUnitsList);
  // var widthStr = String(gCollatedCellData.size.width) + gCellWidthUnit;
  // var heightStr = String(gCollatedCellData.size.height) + gCellHeightUnit;

  gDialog.CellHeightInput.value = 0;
  gDialog.CellHeightUnits = unitHandler.getCurrentUnit();
  gDialog.CellWidthInput.value = 0;
  gDialog.textwrapchoice.value = true;
  
}

//function InitCellPanel()
//{
//  // Get cell attributes
//  if (globalCellElement)
//  {
//    // This assumes order of items is Cell, Row, Column
//    gDialog.SelectionList.value = gSelectedCellsType;
//
//    var previousValue = gDialog.CellHeightInput.value;
//    gDialog.CellHeightInput.value = InitPixelOrPercentMenulist(globalCellElement, gCellElement, "height", "CellHeightUnits", gPixel);
//    gDialog.CellHeightCheckbox.checked = gAdvancedEditUsed && previousValue != gDialog.CellHeightInput.value;
//
//    previousValue= gDialog.CellWidthInput.value;
//    gDialog.CellWidthInput.value = InitPixelOrPercentMenulist(globalCellElement, gCellElement, "width", "CellWidthUnits", gPixel);
//    gDialog.CellWidthCheckbox.checked = gAdvancedEditUsed && previousValue != gDialog.CellWidthInput.value;
//
//    var previousIndex = gDialog.CellVAlignList.selectedIndex;
//    var valign = GetHTMLOrCSSStyleValue(globalCellElement, "valign", "vertical-align").toLowerCase();
//    if (valign == topStr || valign == bottomStr)
//      gDialog.CellVAlignList.value = valign;
//    else // Default = middle
//      gDialog.CellVAlignList.value = defVAlign;
//
//    gDialog.CellVAlignCheckbox.checked = gAdvancedEditUsed && previousIndex != gDialog.CellVAlignList.selectedIndex;
//
//    previousIndex = gDialog.CellHAlignList.selectedIndex;
//
//    gAlignWasChar = false;
//
//    var halign = GetHTMLOrCSSStyleValue(globalCellElement, "align", "text-align").toLowerCase();
//    switch (halign)
//    {
//      case centerStr:
//      case rightStr:
//      case justifyStr:
//        gDialog.CellHAlignList.value = halign;
//        break;
//      case charStr:
//        // We don't support UI for this because layout doesn't work: bug 2212.
//        // Remember that's what they had so we don't change it
//        //  unless they change the alignment by using the menulist
//        gAlignWasChar = true;
//        // Fall through to use show default alignment in menu
//      default:
//        // Default depends on cell type (TH is "center", TD is "left")
//        gDialog.CellHAlignList.value =
//          (globalCellElement.nodeName.toLowerCase() == "th") ? "center" : "left";
//        break;
//    }
//
//    gDialog.CellHAlignCheckbox.checked = gAdvancedEditUsed &&
//      previousIndex != gDialog.CellHAlignList.selectedIndex;
//
//    previousIndex = gDialog.CellStyleList.selectedIndex;
//    gDialog.CellStyleList.value = globalCellElement.nodeName.toLowerCase();
//    gDialog.CellStyleCheckbox.checked = gAdvancedEditUsed && previousIndex != gDialog.CellStyleList.selectedIndex;
//
//    previousIndex = gDialog.TextWrapList.selectedIndex;
//    if (GetHTMLOrCSSStyleValue(globalCellElement, "nowrap", "white-space") == "nowrap")
//      gDialog.TextWrapList.value = "nowrap";
//    else
//      gDialog.TextWrapList.value = "wrap";
//    gDialog.textwrapchoice.checked = gAdvancedEditUsed && previousIndex != gDialog.TextWrapList.selectedIndex;
//
//    previousValue = gCellColor;
//    gCellColor = GetHTMLOrCSSStyleValue(globalCellElement, bgcolor, cssBackgroundColorStr);
//    gCellColor = ConvertRGBColorIntoHEXColor(gCellColor);
//    SetColor("cellBackgroundCW", gCellColor);
//    gDialog.CellColorCheckbox.checked = gAdvancedEditUsed && previousValue != gCellColor;
//
//    // We want to set this true in case changes came
//    //   from Advanced Edit dialog session (must assume something changed)
//    gCellDataChanged = true;
//  }
//}
//
//function GetCellData(rowIndex, colIndex)
//{
//  // Get actual rowspan and colspan
//  var startRowIndexObj = { value: 0 };
//  var startColIndexObj = { value: 0 };
//  var rowSpanObj = { value: 0 };
//  var colSpanObj = { value: 0 };
//  var actualRowSpanObj = { value: 0 };
//  var actualColSpanObj = { value: 0 };
//  var isSelectedObj = { value: false };
//
//  try {
//    gActiveEditor.getCellDataAt(gTableElement, rowIndex, colIndex,
//                         gCellData,
//                         startRowIndexObj, startColIndexObj,
//                         rowSpanObj, colSpanObj,
//                         actualRowSpanObj, actualColSpanObj, isSelectedObj);
//    // We didn't find a cell
//    if (!gCellData.value) return false;
//  }
//  catch(ex) {
//    return false;
//  }
//
//  gCellData.startRowIndex = startRowIndexObj.value;
//  gCellData.startColIndex = startColIndexObj.value;
//  gCellData.rowSpan = rowSpanObj.value;
//  gCellData.colSpan = colSpanObj.value;
//  gCellData.actualRowSpan = actualRowSpanObj.value;
//  gCellData.actualColSpan = actualColSpanObj.value;
//  gCellData.isSelected = isSelectedObj.value;
//  return true;
//}
//
//function SelectCellHAlign()
//{
//  SetCheckbox("CellHAlignCheckbox");
//  // Once user changes the alignment,
//  //  we lose their original "CharAt" alignment"
//  gAlignWasChar = false;
//}

function GetColorAndUpdate(ColorWellID)
{
  var colorWell = document.getElementById(ColorWellID);
  if (!colorWell) return;

  var colorObj = {Type: "TableOrCell", NoDefault:false, Cancel:false, BackgroundColor:0,
    SelectedType: "Table" };

  switch( ColorWellID )
  {
    case "tableBackgroundCW":
         colorObj.BackgroundColor = gTableColor;
      break;
    case "backgroundCW":
      colorObj.BackgroundColor = gCollatedCellData.background;
      colorObj.SelectedType = "Cell";
      break;
    case "borderCW":
      colorObj.CellColor = gCollatedCellData.border.color[gCurrentSide];
      break;
  }

  window.openDialog("chrome://editor/content/EdColorPicker.xul", "colorpicker", "chrome,close,titlebar,modal,resizable", "", colorObj);

  // User canceled the dialog
  if (colorObj.Cancel)
    return;
  else {
		gActiveEditor.incrementModificationCount(1);
	}

  var changeArray = createPreviewChangeArray();
  var borderSideAttrStr = "";
  var theColor;
  switch( ColorWellID )
  {
    case "backgroundCW":
      theColor = colorObj.BackgroundColor;  //this is where it's coming back
//      gCollatedCellData.background = theColor;
//      changeArray.style["background-color"] = theColor;  //this is where it's coming back
//      gCellChangeData.background = true;
      setColorWell(ColorWellID, theColor);
      break;
    case "tableBackgroundCW":
      gTableColor = colorObj.BackgroundColor;
      changeArray.tableStyle["background-color"] = gTableColor;
      gTableChangeData.background = true;
      setColorWell(ColorWellID, gTableColor);
      break;
    case "borderCW":
      theColor = colorObj.BackgroundColor;  //this is where it's coming back
      gCollatedCellData.border.color[gCurrentSide] = theColor;
//      gCollatedCellData.border.color[gCurrentSide] = colorObj.CellColor;
//      SetColor(ColorWellID, gCollatedCellData.border.color[gCurrentSide]);  Do we really want to actually SetColor at this point?
      borderSideAttrStr = getBorderSideAttrString(gCurrentSide, "color");
      changeArray.style[borderSideAttrStr] = theColor;
      if (!newTable) gCellChangeData.border.color.push(gCurrentSide);
      setColorWell(ColorWellID, theColor);
//      SetCheckbox('CellColorCheckbox');
    break;
  }
  updateSample(changeArray);
}

//function SetColor(ColorWellID, color)
//{
//  // Save the color
//  if (ColorWellID == "cellBackgroundCW")
//  {
//    if (color)
//    {
//      try {
//        gActiveEditor.setAttributeOrEquivalent(globalCellElement, bgcolor,
//                                               color, true);
//      } catch(e) {}
//      gDialog.CellInheritColor.collapsed = true;
//    }
//    else
//    {
//      try {
//        gActiveEditor.removeAttributeOrEquivalent(globalCellElement, bgcolor, true);
//      } catch(e) {}
//      // Reveal addition message explaining "default" color
//      gDialog.CellInheritColor.collapsed = false;
//    }
//  }
//  else
//  {
//    if (color)
//    {
//      try {
//        gActiveEditor.setAttributeOrEquivalent(globalTableElement, bgcolor,
//                                               color, true);
//      } catch(e) {}
//      gDialog.TableInheritColor.collapsed = true;
//    }
//    else
//    {
//      try {
//        gActiveEditor.removeAttributeOrEquivalent(globalTableElement, bgcolor, true);
//      } catch(e) {}
//      gDialog.TableInheritColor.collapsed = false;
//    }
//    SetCheckbox('CellColorCheckbox');
//  }
//
//  setColorWell(ColorWellID, color);
//}

//function ChangeSelectionToFirstCell()
//{
//  if (!GetCellData(0,0))
//  {
//    dump("Can't find first cell in table!\n");
//    return;
//  }
//  gCellElement = gCellData.value;
//  globalCellElement = gCellElement;
//
//  gCurRowIndex = 0;
//  gCurColIndex = 0;
//  ChangeSelection(RESET_SELECTION);
//}
//
//function ChangeSelection(newType)
//{
//  newType = Number(newType);
//
//  if (gSelectedCellsType == newType)
//    return;
//
//  if (newType == RESET_SELECTION)
//    // Restore selection to existing focus cell
//    gSelection.collapse(gCellElement,0);
//  else
//    gSelectedCellsType = newType;
//
//  // Keep the same focus gCellElement, just change the type
//  DoCellSelection();
//  SetSelectionButtons();
//
//  // Note: globalCellElement should still be a clone of gCellElement
//}
//
//function MoveSelection(forward)
//{
//  var newRowIndex = gCurRowIndex;
//  var newColIndex = gCurColIndex;
//  var focusCell;
//  var inRow = false;
//
//  if (gSelectedCellsType == SELECT_ROW)
//  {
//    newRowIndex += (forward ? 1 : -1);
//
//    // Wrap around if before first or after last row
//    if (newRowIndex < 0)
//      newRowIndex = gLastRowIndex;
//    else if (newRowIndex > gLastRowIndex)
//      newRowIndex = 0;
//    inRow = true;
//
//    // Use first cell in row for focus cell
//    newColIndex = 0;
//  }
//  else
//  {
//    // Cell or column:
//    if (!forward)
//      newColIndex--;
//
//    if (gSelectedCellsType == SELECT_CELL)
//    {
//      // Skip to next cell
//      if (forward)
//        newColIndex += gCurColSpan;
//    }
//    else  // SELECT_COLUMN
//    {
//      // Use first cell in column for focus cell
//      newRowIndex = 0;
//
//      // Don't skip by colspan,
//      //  but find first cell in next cellmap column
//      if (forward)
//        newColIndex++;
//    }
//
//    if (newColIndex < 0)
//    {
//      // Request is before the first cell in column
//
//      // Wrap to last cell in column
//      newColIndex = gLastColIndex;
//
//      if (gSelectedCellsType == SELECT_CELL)
//      {
//        // If moving by cell, also wrap to previous...
//        if (newRowIndex > 0)
//          newRowIndex -= 1;
//        else
//          // ...or the last row
//          newRowIndex = gLastRowIndex;
//
//        inRow = true;
//      }
//    }
//    else if (newColIndex > gLastColIndex)
//    {
//      // Request is after the last cell in column
//
//      // Wrap to first cell in column
//      newColIndex = 0;
//
//      if (gSelectedCellsType == SELECT_CELL)
//      {
//        // If moving by cell, also wrap to next...
//        if (newRowIndex < gLastRowIndex)
//          newRowIndex++;
//        else
//          // ...or the first row
//          newRowIndex = 0;
//
//        inRow = true;
//      }
//    }
//  }
//
//  // Get the cell at the new location
//  do {
//    if (!GetCellData(newRowIndex, newColIndex))
//    {
//      dump("MoveSelection: CELL NOT FOUND\n");
//      return;
//    }
//    if (inRow)
//    {
//      if (gCellData.startRowIndex == newRowIndex)
//        break;
//      else
//        // Cell spans from a row above, look for the next cell in row
//        newRowIndex += gCellData.actualRowSpan;
//    }
//    else
//    {
//      if (gCellData.startColIndex == newColIndex)
//        break;
//      else
//        // Cell spans from a Col above, look for the next cell in column
//        newColIndex += gCellData.actualColSpan;
//    }
//  }
//  while(true);
//
//  // Save data for current selection before changing
//  if (gCellDataChanged) // && gDialog.ApplyBeforeMove.checked)
//  {
//    if (!ValidateCellData())
//      return;
//
//    gActiveEditor.beginTransaction();
//    // Apply changes to all selected cells
//    ApplyCellAttributes();
//    gActiveEditor.endTransaction();
//
//    SetCloseButton();
//  }
//
//  // Set cell and other data for new selection
//  gCellElement = gCellData.value;
//
//  // Save globals for new current cell
//  gCurRowIndex = gCellData.startRowIndex;
//  gCurColIndex = gCellData.startColIndex;
//  gCurColSpan = gCellData.actualColSpan;
//
//  // Copy for new global cell
//  globalCellElement = gCellElement.cloneNode(false);
//
//  // Change the selection
//  DoCellSelection();
//
//  // Scroll page so new selection is visible
//  // Using SELECTION_ANCHOR_REGION makes the upper-left corner of first selected cell
//  //    the point to bring into view.
//  try {
//    var selectionController = gActiveEditor.selectionController;
//    selectionController.scrollSelectionIntoView(selectionController.SELECTION_NORMAL, selectionController.SELECTION_ANCHOR_REGION, true);
//  } catch (e) {}
//
//  // Reinitialize dialog using new cell
////  if (!gDialog.KeepCurrentData.checked)
//  // Setting this false unchecks all "set attributes" checkboxes
//  gAdvancedEditUsed = false;
//  InitCellPanel();
//  gAdvancedEditUsed = true;
//}
//
//
//function DoCellSelection()
//{
//  // Collapse selection into to the focus cell
//  //  so editor uses that as start cell
//  gSelection.collapse(gCellElement, 0);
//
//  var tagNameObj = { value: "" };
//  var countObj = { value: 0 };
//  try {
//    switch (gSelectedCellsType)
//    {
//      case SELECT_CELL:
//        gActiveEditor.selectTableCell();
//        break
//      case SELECT_ROW:
//        gActiveEditor.selectTableRow();
//        break;
//      default:
//        gActiveEditor.selectTableColumn();
//        break;
//    }
//    // Get number of cells selected
//    var tableOrCellElement = gActiveEditor.getSelectedOrParentTableElement(tagNameObj, countObj);
//  } catch (e) {}
//
//  if (tagNameObj.value == "td")
//    gSelectedCellCount = countObj.value;
//  else
//    gSelectedCellCount = 0;
//
//  // Currently, we can only allow advanced editing on ONE cell element at a time
//  //   else we ignore CSS, JS, and HTML attributes not already in dialog
//  SetElementEnabled(gDialog.AdvancedEditCell, gSelectedCellCount == 1);
//
//  gDialog.AdvancedEditCell.setAttribute("tooltiptext", 
//    gSelectedCellCount > 1 ? GetString("AdvancedEditForCellMsg") :
//                             gDialog.AdvancedEditCellToolTipText);
//}
//
//function SetSelectionButtons()
//{
//  if (gSelectedCellsType == SELECT_ROW)
//  {
//    // Trigger CSS to set images of up and down arrows
//    gDialog.PreviousButton.setAttribute("type","row");
//    gDialog.NextButton.setAttribute("type","row");
//  }
//  else
//  {
//    // or images of left and right arrows
//    gDialog.PreviousButton.setAttribute("type","col");
//    gDialog.NextButton.setAttribute("type","col");
//  }
//  DisableSelectionButtons((gSelectedCellsType == SELECT_ROW && gRowCount == 1) ||
//                          (gSelectedCellsType == SELECT_COLUMN && gColCount == 1) ||
//                          (gRowCount == 1 && gColCount == 1));
//}
//
//function DisableSelectionButtons( disable )
//{
//  gDialog.PreviousButton.setAttribute("disabled", disable ? "true" : "false");
//  gDialog.NextButton.setAttribute("disabled", disable ? "true" : "false");
//}

function SwitchToValidatePanel()
{
  if (gDialog.TabBox.selectedTab != gValidateTab)
    gDialog.TabBox.selectedTab = gValidateTab;
}

//function SetAlign(listID, defaultValue, element, attName)
//{
//  var value = document.getElementById(listID).value;
//  if (value == defaultValue)
//  {
//    try {
//      gActiveEditor.removeAttributeOrEquivalent(element, attName, true);
//    } catch(e) {}
//  }
//  else
//  {
//    try {
//      gActiveEditor.setAttributeOrEquivalent(element, attName, value, true);
//    } catch(e) {}
//  }
//}

//rwa function ValidateTableData()
//rwa {
//  gValidateTab = gDialog.TableTab;
//  gNewRowCount = Number(msiValidateNumber(gDialog.TableRowsInput, null, 1, gMaxRows, null, true, true));
//  if (gValidationError) return false;
//
//  gNewColCount = Number(msiValidateNumber(gDialog.TableColumnsInput, null, 1, gMaxColumns, null, true, true));
//  if (gValidationError) return false;

//  // If user is deleting any cells, get confirmation
//  // (This is a global to the dialog and we ask only once per dialog session)
//  if ( !gCanDelete &&
//        (gNewRowCount < gRowCount ||
//         gNewColCount < gColCount) ) 
//  {
//    if (ConfirmWithTitle(GetString("DeleteTableTitle"), 
//                         GetString("DeleteTableMsg"),
//                         GetString("DeleteCells")) )
//    {
//      gCanDelete = true;
//    }
//    else
//    {
//      SetTextboxFocus(gNewRowCount < gRowCount ? gDialog.TableRowsInput : gDialog.TableColumnsInput);
//      return false;
//    }
//  }

  //Very likely want to restore the following, but where to put it?
//rwa   ValidateNumber(gDialog.TableWidthInput, gDialog.TableWidthUnits,
//rwa                  1, gMaxTableSize, globalTableElement, "width");
//rwa   if (gValidationError) return false;
//rwa 
//rwa   if (gUseCSS) {
//rwa     ValidateNumber(gDialog.TableHeightInput, gDialog.TableHeightUnits,
//rwa                    1, gMaxTableSize, globalTableElement, "height");
//rwa     if (gValidationError) return false;
//rwa   }

//rwa   gValidateTab = gDialog.LinesTab;
//rwa   var border = ValidateNumber(gDialog.BorderWidthInput, null, 0, gMaxPixels, globalTableElement, "border");
//rwa   // TODO: Deal with "BORDER" without value issue
//rwa   if (gValidationError) return false;
//rwa 
//rwa //  ValidateNumber(gDialog.SpacingInput, null, 0, gMaxPixels, globalTableElement, "cellspacing");
//rwa //  if (gValidationError) return false;
//rwa //
//rwa //  ValidateNumber(gDialog.PaddingInput, null, 0, gMaxPixels, globalTableElement, "cellpadding");
//rwa //  if (gValidationError) return false;
//rwa 
//rwa   SetAlign("TableAlignList", defHAlign, globalTableElement, "align");

  // Color is set on globalCellElement immediately
//rwa   return true;
//rwa }

//rwa function ValidateCellData()
//rwa {
//rwa 
//rwa   gValidateTab = gDialog.LayoutTab;
//rwa 
//rwa   if (gDialog.CellHeightCheckbox.checked)
//rwa   {
//rwa     ValidateNumber(gDialog.CellHeightInput, gDialog.CellHeightUnits,
//rwa                     1, gMaxTableSize, globalCellElement, "height");
//rwa     if (gValidationError) return false;
//rwa   }
//rwa 
//rwa   if (gDialog.CellWidthCheckbox.checked)
//rwa   {
//rwa     ValidateNumber(gDialog.CellWidthInput, gDialog.CellWidthUnits,
//rwa                    1, gMaxTableSize, globalCellElement, "width");
//rwa     if (gValidationError) return false;
//rwa   }
//rwa 
//rwa   gValidateTab = gDialog.AlignTab;
//rwa   if (gDialog.CellHAlignCheckbox.checked)
//rwa   {
//rwa     var hAlign = gDialog.CellHAlignList.value;
//rwa 
//rwa     // Horizontal alignment is complicated by "char" type
//rwa     // We don't change current values if user didn't edit alignment
//rwa     if (!gAlignWasChar)
//rwa     {
//rwa       globalCellElement.removeAttribute(charStr);
//rwa 
//rwa       // Always set "align" attribute,
//rwa       //  so the default "left" is effective in a cell
//rwa       //  when parent row has align set.
//rwa       globalCellElement.setAttribute("align", hAlign);
//rwa     }
//rwa   }
//rwa 
//rwa   if (gDialog.CellVAlignCheckbox.checked)
//rwa   {
//rwa     // Always set valign (no default in 2nd param) so
//rwa     //  the default "middle" is effective in a cell
//rwa     //  when parent row has valign set.
//rwa     SetAlign("CellVAlignList", "", globalCellElement, "valign");
//rwa   }
//rwa 
//rwa //  if (gDialog.textwrapchoice.checked)
//rwa //  {
//rwa //    if (gDialog.TextWrapList.value == "nowrap")
//rwa //      try {
//rwa //        gActiveEditor.setAttributeOrEquivalent(globalCellElement, "nowrap",
//rwa //                                               "nowrap", true);
//rwa //      } catch(e) {}
//rwa //    else
//rwa //      try {
//rwa //        gActiveEditor.removeAttributeOrEquivalent(globalCellElement, "nowrap", true);
//rwa //      } catch(e) {}
//rwa //  }
//rwa 
//rwa   return true;
//rwa }
//rwa 
function ValidateData()
{
//rwa   var result;
//rwa 
//rwa   // Validate current panel first
//rwa   if (gDialog.TabBox.selectedTab == gDialog.TableTab)
//rwa   {
//rwa     result = ValidateTableData();
//rwa     if (result)
//rwa       result = ValidateCellData();
//rwa   } else {
//rwa     result = ValidateCellData();
//rwa     if (result)
//rwa       result = ValidateTableData();
//rwa   }
//rwa   if(!result) return false;
//rwa 
//rwa   // Set global element for AdvancedEdit
//rwa   if(gDialog.TabBox.selectedTab == gDialog.TableTab)
//rwa     globalElement = globalTableElement;
//rwa   else
//rwa     globalElement = globalCellElement;
//rwa
  // gCellWidthUnit = gDialog.cellUnitsList.value;
  // gCellHeightUnit = gCellWidthUnit;
  return true;
}

//function ChangeCellTextbox(textboxID)
//{
//  // Filter input for just integers
//  forceInteger(textboxID);
//
//  if (gDialog.TabBox.selectedTab == gDialog.CellTab)
//    gCellDataChanged = true;
//}

// Call this when a textbox or menulist is changed
//   so the checkbox is automatically set

//rwa function ChangeIntTextbox(textboxID, checkboxID)
//rwa {
//rwa   // Filter input for just integers
//rwa   forceInteger(textboxID);
//rwa 
//rwa   // Set associated checkbox
//rwa   SetCheckbox(checkboxID);
//rwa }

//function CheckboxChanged(checkID)
//{
//  switch(checkID)
//  {
//    case "CellWidthCheckbox":
//      gCellChangeData.size.width = true;
//      gCellWidthUnit = gDialog.CellWidthUnits.value;
//      gCollatedCellData.size.bWidthSet = gDialog.CellWidthCheckbox.checked;
//      if (gCollatedCellData.size.bWidthSet)
//        gCollatedCellData.size.width = gWidthUnitsController.getValue(gDialog.CellWidthInput, gCellWidthUnit);
//    break;
//    case "CellHeightCheckbox":
//      gCellChangeData.size.height = true;
//      gCellHeightUnit = gDialog.CellHeightUnits.value;
//      gCollatedCellData.size.bHeightSet = gDialog.CellHeightCheckbox.checked;
//      if (gCollatedCellData.size.bHeightSet)
//        gCollatedCellData.size.height = gHeightUnitsController.getValue(gDialog.CellHeightInput, gCellHeightUnit);
//    break;
//    case "textwrapchoice":
//      gCellChangeData.wrap = true;
//      gCollatedCellData.wrap = (gDialog.textwrapchoice.checked ? "wrap" : "nowrap");
//    break;
//  }
//}

function ChangeCellSize(textID)
{
  switch(textID)
  {
    case "CellWidthInput":
      gCellChangeData.size.width = true;
      gCellWidthUnit = unitHandler.getCurrentUnit();
      gCollatedCellData.size.width = unitHandler.getValueOf(gDialog.CellWidthInput.value, gCellWidthUnit);
    break;
    case "CellHeightInput":
      gCellChangeData.size.height = true;
      gCellHeightUnit = unitHandler.getCurrentUnit();
      gCollatedCellData.size.height = unitHandler.getValueOf(gDialog.CellHeightInput.value, gCellHeightUnit);
    break;
  }
}

function AlignmentChanged(radioGroupID)
{
  switch(radioGroupID)
  {
    case "ColumnAlignRadioGroup":
      gCellChangeData.align.halign = true;
      gCollatedCellData.align.halign = gDialog.hAlignChoices.value;
    break;
    case "RowAlignRadioGroup":
      gCellChangeData.align.valign = true;
      gCollatedCellData.align.valign = gDialog.vAlignChoices.value;
    break;
  }
}

function TablePropertyChanged(controlID)
{
  if (newTable) return;
  switch(controlID)
  {
    case "TableCaptionList":
      gTableChangeData.caption = true;
      gTableCaptionPlacement = gDialog.captionLocation.value;
    break;

    case "TableBaselineRadioGroup":
      gTableChangeData.baseline = true;
      gTableBaseline = gDialog.baselineList.value;
    break;
  }
}

function DisableRadioGroup(aGroup)
{
  var aNode = null;
  for (var ix = 0; ix < aGroup.childNodes.length; ++ix)
  {
    aNode = aGroup.childNodes[ix];
    if (aNode.getAttribute("group") == aGroup.id)
      aNode.disabled = true;
  }
  aGroup.disabled = true;
}

function checkEnableWidthControls()
{
  var bDisabled = gDialog.autoWidthCheckbox.checked;
  enableControlsByID(["tableWidthInput"], !bDisabled);
}

function checkEnableFloatControl()
{
  var sel = gDialog.tableLocationList.selectedItem.value;
  var bEnableFloat = (sel ===  "unspecified");
  enableControlsByID(["floatLocationList"], bEnableFloat);
}

function locationPropertyChanged()
{
  checkEnableFloatControl();
}


function checkEnableLocationControl()
{
  var sel = gDialog.floatLocationList.selectedItem.value;
  var bEnableLocation = (sel ===  "");
  enableControlsByID(["tableLocationList"], bEnableLocation);
}

function floatPropertyChanged()
{
  checkEnableLocationControl();
}




function EnableDisableControls()
{
  var bIsWholeCols = false;
  var bIsWholeRows = false;

  var anItem;
  if (gIsMatrix)
  {
    for (var ii = gDialog.CellBorderStyleList.itemCount-1; ii >= 0; --ii)
    {
      anItem = gDialog.CellBorderStyleList.getItemAtIndex(ii);
      switch(anItem.getAttribute("value"))
      {
        case "none":
        case "dotted":
        case "solid":
        break;
        case "hidden":
        case "dashed":
        case "double":
        case "groove":
        case "ridge":
          gDialog.CellBorderStyleList.removeItemAt(ii);
        break;
      }
    }

    enableControlsByID(rowHeightControlIDs, false);
//    if (!bIsWholeCols)
//    {
//      DisableRadioGroup(gDialog.ColAlignRadioGroup);
//      gDialog.CellWidthInput.disabled = true;
//      gDialog.CellWidthUnits.disabled = true;
//      gDialog.CellWidthCheckbox.disabled = true;
//    }
//
//    if (!bIsWholeRows)
//    {
//      DisableRadioGroup(gDialog.RowAlignRadioGroup);
//      gDialog.CellHeightInput.disabled = true;
//      gDialog.CellHeightUnits.disabled = true;
//      gDialog.CellHeightCheckbox.disabled = true;
//    }
  }
  checkEnableWidthControls();
  checkEnableFloatControl();
  checkEnableLocationControl();
}

//function CloneAttribute(destElement, srcElement, attr)
//{
//  var value = srcElement.getAttribute(attr);
//  // Use editor methods since we are always
//  //  modifying a table in the document and
//  //  we need transaction system for undo
//  try {
//    if (!value || value.length == 0)
//      gActiveEditor.removeAttributeOrEquivalent(destElement, attr, false);
//    else
//      gActiveEditor.setAttributeOrEquivalent(destElement, attr, value, false);
//  } catch(e) {}
//}

function SetAnAttribute(destElement, theAttr, theAttrValue)
{
  // Use editor methods since we are always
  //  modifying a table in the document and
  //  we need transaction system for undo
  try {
    if (!theAttrValue || theAttrValue.length === 0)
      gActiveEditor.removeAttribute(destElement, theAttr);
    else
//      gActiveEditor.setAttribute(destElement, theAttr, theAttrValue);
      gActiveEditor.setAttributeOrEquivalent(destElement, theAttr, theAttrValue, false);
    var logStr = "In msiEdTableProps.js, SetAnAttribute(); set attribute [" + theAttr + "] on [" + destElement.nodeName + "] element to [";
    if (theAttrValue && theAttrValue.length)
      logStr += theAttrValue;
    logStr += "]\n";
    msiKludgeLogString(logStr, ["tableEdit"]);
  } catch(e)
  {
    msidump(e.message);
  }
}



function GetSelectedCells(selection)
{
  var rangeCount = selection.rangeCount;
  var cells = [];
  var i;
  if (newTable || data.reviseCommand === "cmd_editTable") {
    // in effect all cells are selected
    var htmlcollection;
    htmlcollection = gTableElement.getElementsByTagName("td");
    for (i = 0; i < htmlcollection.length; i++) cells.push(htmlcollection[i]);
    htmlcollection = gTableElement.getElementsByTagName("th");
    for (i = 0; i < htmlcollection.length; i++) cells.push(htmlcollection[i]);
  }
  else {
    for (var i = 0; i < rangeCount; i++) {
      var range = selection.getRangeAt(i);
      var startContainer = range.startContainer;
      var startOffset    = range.startOffset;
      var endContainer   = range.endContainer;
      var endOffset      = range.endOffset;
      if (startContainer.nodeType == Node.ELEMENT_NODE)
        startContainer = startContainer.childNodes.item(startOffset);
      if (endContainer.nodeType == Node.ELEMENT_NODE)
        endContainer = endContainer.childNodes.item(endOffset -
                         (selection.isCollapsed ? 0 : 1));
    
      var node = startContainer;
      var direction = "down";
      var nextNode = node;
      do {
        node = nextNode;
        var tmp = node;
        // find a potential th/td ancestor
        while (tmp) {
          if (tmp instanceof Components.interfaces.nsIDOMHTMLTableCellElement) {
            if (cells.indexOf(tmp) == -1)
              cells.push(tmp);
            break;
          }
          tmp = tmp.parentNode;
        }
    
        // let's traverse the tree
        if (direction == "down") {
          if (node.firstChild)
            nextNode = node.firstChild
          else if (node.nextSibling)
            nextNode = node.nextSibling;
          else {
            direction = "up";
            nextNode = node.parentNode;
          }
        }
        else {
          if (node.nextSibling) {
            nextNode = node.nextSibling;
            direction = "down";
          }
          else
            nextNode = node.parentNode;
        }
      } while (node != endContainer);
    }
  }
  return cells;
}

function UpdateCells(editor)
{
  var selection = editor.selection;
  var cells = GetSelectedCells(selection);
  // at this points cells array contains all the cells to impact
  for (var i = 0; i < cells.length; i++) {
    var c = cells[i];
    ApplyAttributesToOneCell(c, makeSourceLineObject(document.getElementById("BordersPreviewCenterCell")));
  }
}

    // var txn = new diStyleAttrChangeTxn(c, "width", gDialog.cellsWidthMenulist.value, "");
    // editor.doTransaction(txn);
    // editor.incrementModificationCount(1);  

    // txn = new diStyleAttrChangeTxn(c, "height", gDialog.cellsHeightMenulist.value, "");
    // editor.doTransaction(txn);
    // editor.incrementModificationCount(1);  

    // txn = new diStyleAttrChangeTxn(c, "text-align", gDialog.cellsHAlignMenulist.value, "");
    // editor.doTransaction(txn);
    // editor.incrementModificationCount(1);  

    // txn = new diStyleAttrChangeTxn(c, "vertical-align", gDialog.cellsVAlignMenulist.value, "");
    // editor.doTransaction(txn);
    // editor.incrementModificationCount(1);  

    // txn = new diStyleAttrChangeTxn(c, "white-space", gDialog.cellsNoWrapCheckbox.checked ? "now-wrap" : "", "");
    // editor.doTransaction(txn);
    // editor.incrementModificationCount(1);  

    // txn = new diStyleAttrChangeTxn(c, "background-color", gDialog.bgColorColorpicker.color, "");
    // editor.doTransaction(txn);
    // editor.incrementModificationCount(1);  

    // if (c.nodeName.toLowerCase() != (gDialog.cellsHeadersCheckbox.checked ? "th" : "td"))
    //   editor.switchTableCellHeaderType(c);
    //}
    //}


function DoStyleChangesForACell(destCell)
{
  function setStyleAttribute( propName, value) {
    var j;
    try {
      j = propArray.indexOf(propName); 
      if (j >0) {
        styleArray[j][0] = propName;
        styleArray[j][1] = value;
      }
      else styleArray.push([propName, value]);
    }
    catch(e) {
      1+2;
    }
  }

  var style;
  var color;
  var styleArray, propArray;
  var i, j;
  var length;
  color = destCell.getAttribute("ccolor");  // this was set in the caller
  style = destCell.getAttribute("style")||"";
  styleArray = style.split(';');
  length = styleArray.length;
  for (i = length - 1; i >= 0; i--){
    if (styleArray[i].length > 0) {
      styleArray[i] = styleArray[i].split(':');
      for (j = 0; j < 2; j++) {
        styleArray[i][j] = styleArray[i][j].replace(/,|\s/,'');
      }
    } else styleArray.splice(i,1);
  }
  // rearrange styleArray into two parallel arrays, useful for indexOf
  length = styleArray.length;  // it may have changed
  propArray = [];
  for (i = 0; i < length; i++) {
    propArray.push(styleArray[i][0]);
  }
  if (gDialog.CellWidthInput.value > 0) {
    setStyleAttribute("width", gDialog.CellWidthInput.value + document.getElementById("unitMenulist").value);
  }
  if (gDialog.CellHeightInput.value > 0) {
    setStyleAttribute("height", gDialog.CellHeightInput.value + document.getElementById("unitMenulist").value );
  }
  if (color) {
    setStyleAttribute('background-color', color);
  }
  length = styleArray.length;
  for (i = 0; i < length; i++){
    styleArray[i] = styleArray[i].join(": ");
  }
  SetAnAttribute(destCell, "style", styleArray.join(";"));
//destCell.setAttribute("style", styleArray.join(";"));

}

function ApplyTableAttributes()
{
  function doSetStyleAttr(styleProp, styleVal)
  {
    var styleValStr;
    if (styleVal != null)
      styleValStr = String(styleVal);
    if (bEmptyStyle)
    {
      if ( (styleValStr != null) && styleValStr.length )
        theStyleString += styleProp + ": " + styleValStr + ";";
    }
    else if ((styleValStr!=null) && styleValStr.length)
      gTableElement.style.setProperty(styleProp, styleValStr, "");
    else
      gTableElement.style.removeProperty(styleProp);
  }

  try {
    var unit = unitHandler.getCurrentUnit();

    if (unit && gTableElement) 
      SetAnAttribute(gTableElement,"unit", unit);
    // var newAlign = gTableCaptionPlacement;
    // if (!newAlign)
    //   newAlign = "";
    var logStr;

  //   if (gTableCaptionElement)
  //   {
  //     // Get current alignment
  //     var align = msiGetHTMLOrCSSStyleValue(gActiveEditorElement, gTableCaptionElement, "align", "caption-side").toLowerCase();
  //     // This is the default
  //     if (!align) align = "top";

  //     if (newAlign === "")
  //     {
  //       // Remove existing caption
  //       try {
  //         gActiveEditor.deleteNode(gTableCaptionElement);
  //       } catch(e) {}
  //       gTableCaptionElement = null;
  //     }
  //     else if(newAlign != align)
  //     {
  //       try {
  //         logStr = "In msiEdTableProps.js, ApplyTableAttributes(); set attribute [align] on table caption element to [";
  //         if (newAlign == "top") // This is default, so don't explicitly set it
  //           gActiveEditor.removeAttributeOrEquivalent(gTableCaptionElement, "align", false);
  //         else
  //         {
  //           gActiveEditor.setAttributeOrEquivalent(gTableCaptionElement, "align", newAlign, false);
  //           logStr += newAlign;
  //         }
  //         logStr += "].\n";
  //         msiKludgeLogString(logStr, ["tableEdit"]);

  //       } catch(e) {}
  //     }
  //   }
  //   else if (newAlign !== "")
  //   {
  //     // Create and insert a caption:
  //     try {
  //       gTableCaptionElement = gActiveEditor.createElementWithDefaults("caption");
  //     } catch (e) {}
  //     if (gTableCaptionElement)
  //     {
  //       if (newAlign != "top")
  //       {
  //         SetAnAttribute(gTableCaptionElement,"align", newAlign);
  //         logStr = "In msiEdTableProps.js, ApplyTableAttributes(); set attribute [align] on table caption element to [" + newAlign + "].\n";
  //         msiKludgeLogString(logStr, ["tableEdit"]);
  //       }
  //       // Insert it into the table - caption is always inserted as first child
  //       try {
  //         gActiveEditor.insertNode(gTableCaptionElement, gTableElement, 0);
  //       } catch(e) {}

  //       // Put selecton back where it was
  // //      ChangeSelection(RESET_SELECTION);
  //     }
  //   }

    logStr = "";
    var bEmptyStyle = true; //(!gTableElement.style);
    var theStyleString = "";

    doSetStyleAttr("vertical-align", gDialog.baselineList.value);
    SetAnAttribute(gTableElement, "valign", gDialog.baselineList.value);

    doSetStyleAttr("border-collapse", gBorderCollapse);
    var pos = gDialog.tableLocationList.value;
    var float = gDialog.floatLocationList.value;  
    SetAnAttribute(gTableElement,"req","tabulary");
    if (gDialog.autoWidthCheckbox.checked)
      gActiveEditor.removeAttributeOrEquivalent(gTableElement, "width", false);
    else {
      SetAnAttribute(gTableElement,"width",gDialog.tableWidthInput.value );
      doSetStyleAttr("width", gDialog.tableWidthInput.value + unit);
    }
    if (pos && pos.length > 0) {
      if (pos == "inline" || pos == "center") float = "";
      if (float !== "") {
        msiRequirePackage(gActiveEditorElement, "wrapfig", "");
        SetAnAttribute(gTableElement,"placement",placementCodeFrom(pos));
        SetAnAttribute(gTableElement,"placeLocation", float);
        doSetStyleAttr("float", (pos=="left"||pos=="inside")?"left":"right");
      }
      else 
      {
        if (pos == "inline") doSetStyleAttr("display", "inline-table");
        else 
        {
          doSetStyleAttr("display", "table");
          if (pos == "center") {
            doSetStyleAttr("margin-left", "auto");
            doSetStyleAttr("margin-right", "auto");
          }
        }
      }
      SetAnAttribute(gTableElement, "pos", pos);
    }
    else doSetStyleAttr("display", "inline-table");
    if (!bEmptyStyle)
      theStyleString = gTableElement.getAttribute("style");
    if (theStyleString && theStyleString.length)
      SetAnAttribute(gTableElement, "style", theStyleString);
    else
      gActiveEditor.removeAttributeOrEquivalent(gTableElement, "style", false);
  }
  catch(e){
    msidump(e.message);
  }
}

function placementCodeFrom(pos)
{
  switch( pos )
  {
    case "left":
      return "L";
    case "right":
      return "R";
    case "inside":
      return "I";
    case "outside":
      return "O";
  }
}


//function ApplyCellAttributes()

//{
//  var rangeObj = { value: null };
//  var selectedCell;
//  try {
//    selectedCell = gActiveEditor.getFirstSelectedCell(rangeObj);
//  } catch(e) {}
//
//  if (!selectedCell)
//    return;
//
//  if (gSelectedCellCount == 1)
//  {
//    // When only one cell is selected, simply clone entire element,
//    //  thus CSS and JS from Advanced edit is copied
//    try {
//      gActiveEditor.cloneAttributes(selectedCell, globalCellElement);
//    } catch(e) {}
//
//    if (gDialog.CellStyleCheckbox.checked)
//    {
//      var currentStyleIndex = (selectedCell.nodeName.toLowerCase() == "th") ? 1 : 0;
//      if (gDialog.CellStyleList.selectedIndex != currentStyleIndex)
//      {
//        // Switch cell types
//        // (replaces with new cell and copies attributes and contents)
//        try {
//          selectedCell = gActiveEditor.switchTableCellHeaderType(selectedCell);
//        } catch(e) {}
//      }
//    }
//  }
//  else
//  {
//    // Apply changes to all selected cells
//    //XXX THIS DOESN'T COPY ADVANCED EDIT CHANGES!
//    try {
//      while (selectedCell)
//      {
//        ApplyAttributesToOneCell(selectedCell);
//        selectedCell = gActiveEditor.getNextSelectedCell(rangeObj);
//      }
//    } catch(e) {}
//  }
//  gCellDataChanged = false;
//}

function getColElements()
{
  if (!gColElementArray)
    gColElementArray = findColElementsInNode(gTableElement);
  return gColElementArray;
}

function getRowElements()
{
  if (!gRowElementArray)
    gRowElementArray = findRowElementsInNode(gTableElement);
  return gRowElementArray;
}

function getColNodeForColumn(nCol)
{
  var colArray = getColElements();
  if (colArray && colArray.length > nCol)
    return colArray[nCol].mNode;
  return null;
}

function getRowNodeForRow(nRow)
{
  var rowArray = getRowElements();
  if (rowArray && rowArray.length > nRow)
    return rowArray[nRow];
  return null;
}

//Here we don't rely on getElementsByTagName() since it would do the wrong thing in the case of tables inside table cells.
function findColElementsInNode(aNode)
{
  var colList = [];
  var subList = null;
  var childList = msiNavigationUtils.getSignificantContents(aNode);
  var bDone = false;
  var theSpan = 1;
  var spanAttr = "";
  for (var ix = 0; !bDone && ix < childList.length; ++ix)
  {
    theSpan = 1;
    switch(childList[ix].nodeName)
    {
      case "colgroup":
        subList = findColElementsInNode(childList[ix]);
        if (subList.length > 0)
        {
          colList = colList.concat(subList);
          break;  //otherwise continue to the "col" case
        }
      case "col":
        spanAttr = childList[ix].getAttribute("span");
        if (spanAttr && spanAttr.length)
          theSpan = Number(spanAttr);
        colList.push( {mNode : childList[ix], mSpan : theSpan} );
        for (var jx = 1; jx < theSpan; ++jx)
          colList.push( {mNode : childList[ix], mSpan : -jx} );
      break;
      case "tr":
      case "td":
      case "th":
        bDone = true;   //If we got this far without finding the cols, we aren't going to
      break;
      default: //just continue for anything else
      break;
    }
  }

  return colList;
}

//If there is no <colgroup> element and no <col> element, we want to insert them. If they exist, they must already be
//  the right number, when counted with spans, so we want to insert nothing.
//After insertion, the grouping may have to be altered according to the selected cells.
function insertColElementsInTable(tableElement, numCols)
{
  var colList = [];
  var insertPos = 0;
  var colGroupNode = null;
  for (var ix = 0; ix < tableElement.childNodes.length; ++ix)
  {
    switch(tableElement.childNodes[ix].nodeName)
    {
      case "caption":  //in this case just continue on
      break;

      case "colgroup":
        colGroupNode = tableElement.childNodes[ix];
      break;

      default:
        insertPos = ix;  //we found something not a caption, so we assume this is a good place to insert a <colgroup>?
      break;
    }
  }
  if (colGroupNode)
    return findColElementsInNode(colGroupNode);
 
  colGroupNode = gActiveEditor.createNode("colgroup", tableElement, insertPos);
  gActiveEditor.setAttribute(colGroupNode, "span", String(numCols));
  colList.push( {mNode : colGroupNode, mSpan : numCols} );
  for (ix = 1; ix < numCols; ++ix)
    colList.push( {mNode : colGroupNode, mSpan : -ix} );
  return colList;
}

function findRowElementsInNode(aNode)
{
  var rowList = [];
  var childList = msiNavigationUtils.getSignificantContents(aNode);
  var bDone = false;
  for (var ix = 0; !bDone && ix < childList.length; ++ix)
  {
    switch(childList[ix].nodeName)
    {
      case "thead":
      case "tbody":
      case "tfoot":
        rowList = rowList.concat(findRowElementsInNode(childList[ix]));
      break;
      case "tr":
        rowList.push(childList[ix]);
      break;
      case "td": 
      case "th":
         //What to do here? If we encounter a <td> where we're looking for a <tr>, it means there aren't any <tr>'s in this node. However,
        //  there is an implied one - if we are a <tbody>, <thead>, or <tfoot> we should probably return it??
        if (rowList.length === 0)
        {
          rowList.push(aNode);
          bDone = true;
        }
      break;
      default:
      break;
    }
  }
  return rowList;
}


//This function currently just applies column widths and row heights
function ApplyColAndRowAttributes()
{
  function getEndsOfSpanContaining(aColIndex, colElementArray)
  {
    var theSpan = {mStart : aColIndex, mEnd : aColIndex};
    if (colElementArray[aColIndex] == null)
    {
      dump("In msiEdTableProps.js, ApplyColAndRowAttributes(), getEndsOfSpanContaining() called with aColIndex = [" + aColIndex + "]; array element not found!\n");
    }  
    if (colElementArray[aColIndex].mSpan < 0)
      theSpan.mStart += colElementArray[aColIndex].mSpan;
    theSpan.mEnd = theSpan.mStart + colElementArray[theSpan.mStart].mSpan - 1;
    return theSpan;
  }

  function separateAColNode(firstCol, splitCol, colElementArray)
  {
    var originalCol = colElementArray[firstCol].mNode;
    var newCol = null;
    var oldStart = firstCol;
    var insertPos = 0;
    var kk;
    if (colElementArray[firstCol].mSpan < 0)
    {
      dump("In msiEdTableProps.js, ApplyColAndRowAttributes(), separateAColNode() called on middle of a col node!\n");
      return null;
    }
    var lastCol = firstCol + colElementArray[firstCol].mSpan - 1;
    var retVal = {mOriginalNode : originalCol, mNewNode : null};
    if (msiGetBaseNodeName(originalCol) != "col")
    {
      retVal.mOriginalNode = originalCol.ownerDocument.createElement("col");
      gActiveEditor.insertNode(retVal.mOriginalNode, originalCol, 0);
      gActiveEditor.removeAttribute(originalCol, "span");
      originalCol = retVal.mOriginalNode;
      gActiveEditor.setAttribute(originalCol, "span", String(splitCol - firstCol));
      colElementArray[firstCol].mNode = originalCol;
      for (var kk = firstCol + 1; kk < splitCol; ++kk)
      {
        colElements[kk].mNode = originalCol;
        colElements[kk].mSpan = firstCol - kk;
      }
    }

    if (data == null || data.reviseData == null) return;

    retVal.mNewNode = originalCol.ownerDocument.createElement("col");
    newCol = retVal.mNewNode;
    msiCopyElementAttributes(newCol, originalCol, gActiveEditor);
    insertPos = msiNavigationUtils.offsetInParent(originalCol) + 1;
    gActiveEditor.insertNode(newCol, originalCol.parentNode, insertPos);
    gActiveEditor.setAttribute( newCol, "span", String(lastCol - splitCol + 1) );
    colElementArray[firstCol].mSpan = splitCol - firstCol;
    colElementArray[splitCol].mNode = newCol;
    colElementArray[splitCol].mSpan = lastCol - splitCol + 1;
    for (kk = splitCol + 1; kk <= lastCol; ++kk)
    {
      colElementArray[kk].mNode = newCol;
      colElementArray[kk].mSpan = splitCol - kk;
    }
    return retVal;
  }

  if (data == null || data.reviseData == null) return;
  if (gIsMatrix)
  {
    ApplyMatrixColAndRowAttributes();
    return;
  }

  var tableDims = data.reviseData.getTableDims();

  var colsInSelection = data.reviseData.getColsInSelection(gSelectionTypeStr);
  var colElements = getColElements();

  if (!colElements.length && gCollatedCellData.size.bWidthSet && ShouldSetWidthOnCols())
    colElements = insertColElementsInTable(gTableElement, tableDims.nCols);
  msiKludgeLogString("In msiEdTableProps.js, ApplyColAndRowAttributes(), colElements contains [" + colElements.length + "] elements.\n", ["tableEdit"]);

  if (gCellChangeData.size.width && colElements && colElements.length)
  {
    var theWidth = "";
    var contiguousSelectedCols = 1;
    var currSelectedCol = -1;
//    theWidth = String(gCollatedCellData.size.width) + gCellWidthUnit;  we should be using something more like this...
    if (gCollatedCellData.size.bWidthSet)
      theWidth = unitHandler.getValueAs(gDialog.CellWidthInput.value, "px");


    var insertPos = 0;
    var kk = 0;
    var preCols, ourCols, postCols, splitCols;

    for (var ix = 0; ix < colsInSelection.length; ++ix)  //This loop should be thought of as operating over the contiguous blocks in colsInSelection.
    {
      startCol = colsInSelection[ix];
      endCol = startCol;
      for (ix = ix+1; ix < colsInSelection.length; ++ix)
      {
        if (colsInSelection[ix] != endCol + 1)  //Have we arrived at a gap in the selection?
        {
          --ix;  //set it back to the last value without a gap
          break;
        }
        ++endCol;
      }
      msiKludgeLogString("In msiEdTableProps.js, ApplyColAndRowAttributes(), startCol is [" + startCol + "] and endCol is [" + endCol + "].\n", ["tableEdit"]);

      //There are three things that must be done each time through:
      //  First, if startCol isn't the start of a <col> or <colgroup> - that is, is in the middle of one - we have to split it and insert a new <col>
      //    Note here that if colRecord.mNode is a <colgroup> (can anything else occur here?), it needs to be replaced by a new <col> child node.
      //  Then we apply the width attribute to (or remove it from) the newly created <col>.
      //  Finally, if endCol < endSpan, we have to create a new <col> to hold the remaining columns.
      //After this is done, we reset ix, and thus startCol and startSpan, and go through the outer loop again. Though there should
      //  only be multiple passes through the outer loop if the columns in the selection aren't contiguous. (Does <ctrl-click> work to select in tables?)
      colRecord = colElements[startCol];
      ourStart = startCol;

      //When we're done with the following loop, we'll have split the <col> nodes covering the contiguous selected columns 
      //  from startCol to endCol into appropriate ones and applied the width attribute (and whatever we need to apply).
      for (var ourStart = startCol; ourStart <= endCol; ++ourStart)
      {
        preCols = null;
        ourCols = null;
        postCols = null;
        theSpan = getEndsOfSpanContaining(ourStart, colElements);
        msiKludgeLogString("In msiEdTableProps.js, ApplyColAndRowAttributes(), inner loop, theSpan is [" + theSpan.mStart + "," + theSpan.mEnd + "].\n", ["tableEdit"]);
        if (theSpan.mStart < ourStart)  //We must split off the columns before ours
        {
          splitCols = separateAColNode(theSpan.mStart, ourStart, colElements);
          preCols = splitCols.mOriginalNode;
          ourCols = splitCols.mNewNode;

//          if (msiGetBaseNodeName(preCols) != "col")  //if we only have a <colgroup> we should split it up, so create <col> element
//          {
//            ourCols = preCols.ownerDocument.createElement("col");
//            gActiveEditor.insertNode(ourCols, preCols, 0);
//            preCols = ourCols;
//            colElements[theSpan.mStart].mNode = preCols;
//            colElements[theSpan.mStart].mSpan = ourStart - theSpan.mStart;
//            for (kk = theSpan.mStart + 1; kk < ourStart; ++kk)
//            {
//              colElements[kk].mNode = preCols;
//              colElements[kk].mSpan = theSpan.mStart - kk;
//            }
//          }
//          gActiveEditor.setAttribute(preCols, "span", String(ourStart - theSpan.mStart));
//          ourCols = preCols.ownerDocument.createElement("col");
//          msiCopyElementAttributes(ourCols, preCols, gActiveEditor);
//          insertPos = msiNavigationUtils.offsetInParent(preCols) + 1;
//          gActiveEditor.insertNode(ourCols, colRecord.mNode.parentNode, insertPos);
//          gActiveEditor.setAttribute(ourCols, "span", String(theSpan.mEnd - ourStart + 1));
//          colElements[ourStart].mNode = ourCols;
//          for (kk = ourStart + 1; kk <= theSpan.mEnd; ++kk)
//          {
//            colElements[kk].mNode = ourCols;
//            colElements[kk].mSpan = ourStart - kk;
//          }
        }
        else    //Otherwise, preCols is null and the first is ourCols.
          ourCols = colElements[theSpan.mStart].mNode;

        if (theSpan.mEnd > endCol)  //in this case we have to separate off the columns that come after
        {
          msiKludgeLogString("In msiEdTableProps.js, ApplyColAndRowAttributes(), inner loop, calling separateAColNode with ourStart [" + ourStart + "] and endCol [" + endCol + "].\n", ["tableEdit"]);
          splitCols = separateAColNode(ourStart, endCol + 1, colElements);
          ourCols = splitCols.mOriginalNode;
          ourStart = endCol;
          msiKludgeLogString("In msiEdTableProps.js, ApplyColAndRowAttributes(), inner loop, after calling separateAColNode; now ourStart is [" + ourStart + "]; splitCols.mNewNode has span [" + splitCols.mNewNode.getAttribute("span") + "].\n", ["tableEdit"]);
        }
        else
          ourStart = theSpan.mEnd;

        if (theWidth.length)
          gActiveEditor.setAttribute(ourCols, "width", theWidth);
        else
          gActiveEditor.removeAttribute(ourCols, "width");
      }
    }
  }

  var theRowElements = getRowElements();
  var ix;
  var insertPos;
  var rowsInSelection = data.reviseData.getRowsInSelection(gSelectionTypeStr);
  if (gCellChangeData.size.height && ShouldSetHeightOnRows())
  {
    var theHeight = "";
    if (gCollatedCellData.size.bHeightSet)
      theHeight = unitHandler.getValueOf(gDialog.CellHeightInput, "px");
    var theRowNode, newNode;

    for (ix = 0; ix < rowsInSelection.length; ++ix)
    {
      theRowNode = theRowElements[rowsInSelection[ix]];
      switch(msiGetBaseNodeName(theRowNode))
      {
        case "thead":
        case "tfoot":
        case "tbody":
          newNode = theRowNode.ownerDocument.createElement("tr");
          gActiveEditor.insertNode(newNode, theRowNode, theRowNode.childNodes.length);
          theRowNode = newNode;
        break;
        case "td":
        case "th":
          newNode = theRowNode.ownerDocument.createElement("tr");
          insertPos = msiNavigationUtils.offsetInParent(theRowNode);
          gActiveEditor.insertNode(newNode, theRowNode.parentNode, insertPos);
          gActiveEditor.deleteNode(theRowNode);
          gActiveEditor.insertNode(theRowNode, newNode, 0);
          theRowNode = newNode;
        break;
        default:
        break;
      }

      if (theHeight.length)
        gActiveEditor.setAttribute(theRowNode, "height", theHeight);
      else
        gActiveEditor.removeAttribute(theRowNode, "height");
    }
  }

}

function ApplyMatrixColAndRowAttributes()
{
  if (data == null || data.reviseData == null) return;
  var whiteSpace = /(^\s+)/;
  var defColWidth = "auto";
  var colWidths = [];
  var colWidthsStr = gTableElement.getAttribute("columnwidth");
  if (colWidthsStr)
    colWidths = colWidthsStr.split(whiteSpace);
  var tableDims = data.reviseData.getTableDims();
  var bAllAuto = false;
  if (colWidths.length > 0)
    defColWidth = colWidths[colWidths.length - 1];
  for (var nCol = colWidths.length; nCol < tableDims.nCols; ++nCol)
    colWidths[nCol] = defColWidth;

  var colsInSelection = data.reviseData.getColsInSelection(gSelectionTypeStr);
  var theWidth = "auto";
  if (gCollatedCellData.size.bWidthSet)
    theWidth = unitHandler.getValueAs(gDialog.CellWidthInput.value, "px");
//    theWidth = String(gCollatedCellData.size.width) + gCellWidthUnit;
  for (var ix = 0; ix < colsInSelection.length; ++ix)
    colWidths[colsInSelection[ix]] = theWidth;

  colWidthsStr = colWidths[0];
  bAllAuto = (colWidthsStr == "auto");
  for (nCol = 1; nCol < tableDims.nCols; ++nCol)
  {
    bAllAuto = bAllAuto && (colWidths[nCol] === "auto");
    colWidthsStr += " " + colWidths[nCol];
  }
  if (bAllAuto)
    gActiveEditor.removeAttribute(gTableElement, "columnwidth");
  else
    gActiveEditor.setAttribute(gTableElement, "columnwidth", colWidthsStr);

  var logStr;
  logStr = "In msiEdTableProps.js, ApplyMatrixColAndRowAttributes(); set attribute [columnwidth] on table element to [";
  if (!bAllAuto)
    logStr += colWidthsStr;
  logStr += "]\n";
  msiKludgeLogString(logStr, ["tableEdit"]);

  //If there's a setting for height, it would have to be implemented via CSS(?)

  ApplyMatrixColAndRowLines();
  ApplyMatrixAlignment();
}

function ApplyMatrixColAndRowLines()
{
  if (data == null || data.reviseData == null) return;
  var whiteSpace = /(^\s+)/;
  var matrixRowLines = [];
  var matrixColLines = [];
  var defRowLine = "solid";
  var defColLine = "solid";
  var tableDims = data.reviseData.getTableDims();
  var matrixColLinesStr = gTableElement.getAttribute("columnlines");
  if (matrixColLinesStr)
    matrixColLines = matrixColLinesStr.split(whiteSpace);
  if (matrixColLines.length > 0)
    defColLine = matrixColLines[matrixColLines.length - 1];
  for (var nCol = matrixColLines.length; nCol < tableDims.nCols-1; ++nCol)
    matrixColLines[nCol] = defColLine;

  var matrixRowLinesStr = gTableElement.getAttribute("rowlines");
  if (matrixRowLinesStr)
    matrixRowLines = matrixColLinesStr.split(whiteSpace);
  if (matrixRowLines.length > 0)
    defRowLine = matrixRowLines[matrixRowLines.length - 1];
  for (var nRow = matrixRowLines.length; nRow < tableDims.nRows-1; ++nRow)
    matrixRowLines[nRow] = defRowLine;

  var colsInSelection = data.reviseData.getColsInSelection(gSelectionTypeStr);
  var rowsInSelection = data.reviseData.getRowsInSelection(gSelectionTypeStr);
  var rowOffset = 0;
  var colOffset = 0;
  var bDoRows = false;
  var bDoCols = false;
  var logStr;
  var jx;

  for (var ix = 0; ix < gCellChangeData.border.style.length; ++ix)
  {
    theSide = gCellChangeData.border[theProp][ix];
    rowOffset = colOffset = 0;
    rowLineStr = "";
    colLineStr = "";
    switch(theSide)
    {
      case "top":
        rowOffset = -1;  //and fallthrough
      case "bottom":
        rowLineStr = gCollatedCellData.border.style[theSide];
      break;
      case "left":
        colOffset = -1;  //and fallthrough
      case "right":
        colLineStr = gCollatedCellData.border.style[theSide];
      break;
      case "all":
        colOffset = rowOffset =-1;
        rowLineStr = colLineStr = gCollatedCellData.border.style[theSide];
      break;
    }
    if (rowLineStr.length)
    {
      for (var jx = 0; jx < rowsInSelection.length; ++jx)
      {
        if ( (rowsInSelection[jx] + rowOffset >= 0) && (rowsInSelection[jx] + rowOffset < tableDims.nRows - 1) )
          matrixRowLines[rowsInSelection[jx] + rowOffset] = rowLineStr;
        if ( (theSide == "all") && (rowsInSelection[jx] < tableDims.nRows - 1) )
          matrixRowLines[rowsInSelection[jx]] = rowLineStr;
      }
      bDoRows = true;
    }
    if (colLineStr.length)
    {
      for (jx = 0; jx < colsInSelection.length; ++jx)
      {
        if ( (colsInSelection[jx] + colOffset >= 0) && (colsInSelection[jx] + colOffset < tableDims.nCols - 1) )
          matrixColLines[colsInSelection[jx] + colOffset] = colLineStr;
        if ( (theSide == "all") && (colsInSelection[jx] < tableDims.nCols - 1) )
          matrixColLines[colsInSelection[jx]] = colLineStr;
      }
      bDoCols = true;
    }
  }
  if (bDoRows)
  {
    matrixRowLinesStr = matrixRowLines[0];
    for (ix = 1; ix < matrixRowLines.length; ++ix)
      matrixRowLinesStr += " " + matrixRowLines[ix];
    gActiveEditor.setAttribute(gTableElement, "rowlines", matrixRowLinesStr);
    logStr = "In msiEdTableProps.js, ApplyMatrixColAndRowLines(); set attribute [rowlines] on table element to [" + matrixRowLinesStr + "]\n";
    msiKludgeLogString(logStr, ["tableEdit"]);
  }
  if (bDoCols)
  {
    matrixColLinesStr = matrixColLines[0];
    for (ix = 1; ix < matrixColLines.length; ++ix)
      matrixColLinesStr += " " + matrixColLines[ix];
    gActiveEditor.setAttribute(gTableElement, "columnlines", matrixColLinesStr);
    logStr = "In msiEdTableProps.js, ApplyMatrixColAndRowLines(); set attribute [columnlines] on table element to [" + matrixColLinesStr + "]\n";
    msiKludgeLogString(logStr, ["tableEdit"]);
  }
}

function ApplyMatrixAlignment()
{
  if (data == null || data.reviseData == null) return;
  var bDoHAlign = gCellChangeData.align.halign;
  var bDoVAlign = gCellChangeData.align.valign;
  if (!bDoHAlign && !bDoVAlign)
    return;

  var whiteSpace = /(^\s+)/;
  var tableDims = data.reviseData.getTableDims();
  var matrixHAlignVals = [];
  var matrixVAlignVals = [];
  var defaultHAlign = defHAlign;  //this was defined as a global at the top of the file, and I've left it there
  var defaultVAlign = defVAlign;  //this was defined as a global at the top of the file, and I've left it there

  var matrixHAlignStr = gTableElement.getAttribute("columnalign");
  if (matrixHAlignStr)
    matrixHAlignVals = matrixHAlignStr.split(whiteSpace);
  if (matrixHAlignVals.length)
    defaultHAlign = matrixHAlignVals[matrixHAlignVals.length - 1];
  for (var nCol = matrixHAlignVals.length; nCol < tableDims.nCols; ++nCol)
    matrixHAlignVals[nCol] = defaultHAlign;

  var matrixVAlignStr = gTableElement.getAttribute("rowalign");
  if (matrixVAlignStr)
    matrixVAlignVals = matrixVAlignStr.split(whiteSpace);
  if (matrixVAlignVals.length)
    defaultVAlign = matrixVAlignVals[matrixVAlignVals.length - 1];
  for (var nRow = matrixVAlignVals.length; nRow < tableDims.nRows; ++nRow)
    matrixVAlignVals[nRow] = defaultVAlign;

  var colsInSelection = data.reviseData.getColsInSelection(gSelectionTypeStr);
  var rowsInSelection = data.reviseData.getRowsInSelection(gSelectionTypeStr);
  var logStr;

  if (bDoHAlign)
  {
    for (var jx = 0; jx < colsInSelection.length; ++jx)
      matrixHAlignVals[colsInSelection[jx]] = gCollatedCellData.align.halign;
    matrixHAlignStr = matrixHAlignVals[0];
    for (jx = 1; jx < matrixHAlignVals.length; ++jx)
      matrixHAlignStr += " " + matrixHAlignVals[jx];
    gActiveEditor.setAttribute(gTableElement, "columnalign", matrixHAlignStr);
    logStr = "In msiEdTableProps.js, ApplyMatrixAlignment(); set attribute [columnalign] on table to [" + matrixHAlignStr + "]\n";
    msiKludgeLogString(logStr, ["tableEdit"]);
  }
  if (bDoVAlign)
  {
    for (var ix = 0; ix < rowsInSelection.length; ++ix)
      matrixVAlignVals[rowsInSelection[ix]] = gCollatedCellData.align.valign;
    matrixVAlignStr = matrixVAlignVals[0];
    for (ix = 1; ix < matrixVAlignVals.length; ++ix)
      matrixVAlignStr += " " + matrixVAlignVals[ix];
    gActiveEditor.setAttribute(gTableElement, "rowalign", matrixVAlignStr);
    logStr = "In msiEdTableProps.js, ApplyMatrixAlignment(); set attribute [rowalign] on table to [" + matrixVAlignStr + "]\n";
    msiKludgeLogString(logStr, ["tableEdit"]);
  }
}

function ApplyCellAttributes()
{
  UpdateCells(gActiveEditor);
// //  var cellIter = data.reviseData.beginSelectedCellIteration('Cell');
//   if (data == null || data.reviseData == null) return;
//   var cellIter = data.reviseData.beginSelectedCellIteration(gSelectionTypeStr);
//   var currCell = data.reviseData.getNextSelectedCell(cellIter);
//   while (currCell)
//   {
//     var nRow = cellIter.nRow;
//     var nCol = cellIter.nCol;
//     ApplyAttributesToOneCell(currCell, nRow, nCol);
//     currCell = data.reviseData.getNextSelectedCell(cellIter);
//   }
}


function translate(cssstyle) {
  // convert CSS-speak to LaTeX-speak
  switch (cssstyle) {
    case "none":
    case "unspec": return null;
    case "solid" : 
    case "double": return cssstyle;
    default:       return null;
  }
}

function makeSourceLineObject(cell)
{
  var lineobj = {left: null, right: null, top: null, bottom: null};
  var names = ['left','right','top','bottom'];
  var regex = /border-(left|right|top|bottom).*?(none|solid|double)/g;  // find items like 'border-left', etc
  var borderregex = /border:.*?(none|solid|double)/g;  // find items like border:
  var stylestring = cell.getAttribute("style");
  var borderDone = false;
  var matches;
  var match;
  var currLine;
  var i;
  if (stylestring) {
    matches = stylestring.match(borderregex);  //find 'border:'
    if (matches && matches.length > 0) {
      match = borderregex.exec(matches[0]);
      if (match.length > 1) {
        currLine = match[1];
        if (currLine !== "unspec") {
          lineobj.left = lineobj.right = lineobj.top = lineobj.bottom = currLine;
          borderDone = true;
        }
      }
    }
    if (!borderDone)
    {
      matches = stylestring.match(regex);
      if (matches) {
        for (i = 0; i<matches.length; i++) {
          match = regex.exec(matches[i]);
          if (match && match.length > 2) {
            lineobj[match[1]] = match[2];
          }
        }
      }
    }
  }
  return lineobj;
}

function makeTargetLineObject(cell)
{
  var lineobj = {left: null, right: null, top: null, bottom: null};
  var names = ['left','right','top','bottom'];
  var nm;
  var i;
  var val;
  if (cell.hasAttribute("lines")) {
    val = cell.getAttribute("lines");
    lineobj.left = lineobj.right = lineobj.top = lineobj.bottom = val;
    return lineobj;
  }
  for (i = 0; i < 4; i++) {
    nm = "line-" + names[i];
    if (cell.hasAttribute(nm)) {
      lineobj[names[i]] = cell.getAttribute(nm);
    }
  }
  return lineobj;
}


function allSame( lineobj ) // this is applied to targets, which have values of null, solid, and double only
{
  var allsame = true;
  var i;
  var nm;
  var names = ['left','right','top','bottom'];
  var firstVal = lineobj.left;
    //Check to see if all values are the same
  for (i = 0; i < 4; i++) {
    nm = names[i];
    if (lineobj[nm] != firstVal) {
      return false;
    }
  }
  return true;
}

function mergeLineObjects(target, source)
{
  var i;
  var nm;
  var names = ['left','right','top','bottom'];
  var val;
  for (i = 0; i < 4; i++) {
    nm = names[i];
    val = source[nm];
    if (val != null) {  // skip if val==null; remove from target if val=='none'; otherwise copy val to target
      if (val === "none") {
        target[nm] = null;
      }
      else target[nm] = val;
    }
  }
  return target;
}

function ApplyAttributesToOneCell(destElement, newLineObject)
{
  var source = makeSourceLineObject(document.getElementById("BordersPreviewCenterCell"));
  var target = makeTargetLineObject(destElement);
  var color;
  var i;
  var nm;
  var temp;
  var names = ['left','right','top','bottom'];
  target = mergeLineObjects(target, source);
  var allsame = allSame(target);
  if (allsame) {  // put in a lines attribute
    if (target.left != null) {
      
      gActiveEditor.setAttribute(destElement, "lines", target.left);
//      destElement.setAttribute("lines", target.left);
    }
    else gActiveEditor.removeAttribute(destElement,destElement,"lines");
    for (i = 0; i < 4; i++) {
      gActiveEditor.removeAttribute(destElement,"line-"+names[i]);
    }
  }
  else
  {
    for (i = 0; i < 4; i++) {
      nm = names[i];
      if (target[nm] == null) {
        gActiveEditor.removeAttribute(destElement,"line-" + nm);
      }
      else gActiveEditor.setAttribute(destElement, "line-"+nm, target[nm]);
    }
    gActiveEditor.removeAttribute(destElement,"lines");
  }
  if (Number(gDialog.CellHeightInput.value) !== 0)
  {
    aVal = unitHandler.getValueString(gDialog.CellHeightInput.value);
    SetAnAttribute(destElement, "cellheight", aVal);
  }
  else gActiveEditor.removeAttribute(destElement,"cellheight");

  if (Number(gDialog.CellWidthInput.value) !== 0)
  {
    aVal = unitHandler.getValueString(gDialog.CellWidthInput.value);
    SetAnAttribute(destElement, "cellwidth", aVal);
  }
  else gActiveEditor.removeAttribute(destElement,"cellwidth");


//  for (ix = 0; ix < gCellChangeData.border.style.length; ++ix)
//  {
//    theSide = gCellChangeData.border.style[ix];
//    if (theSide == "all")
//      SetAnAttribute(destElement, "lines", gCollatedCellData.border.style[theSide]);
//    else
//      SetAnAttribute(destElement, "line-" + theSide, gCollatedCellData.border.style[theSide]);
//  }
  temp = document.getElementById("hAlignChoices").value;  
  if (temp && temp.length > 0) {
    SetAnAttribute(destElement, "align", temp);
  }
  temp = document.getElementById("vAlignChoices").value;
  if (temp && temp.length > 0) {
    SetAnAttribute(destElement, "valign", temp);
  }
  color = document.getElementById('backgroundCW').getAttribute("color");
  if (color && (color.length > 0) )
  {
    SetAnAttribute(destElement, "ccolor", color);
  }
  DoStyleChangesForACell(destElement);

}

function SetCloseButton()
{
  // Change text on "Cancel" button after Apply is used
  if (!gApplyUsed)
  {
    document.documentElement.setAttribute("buttonlabelcancel",
      document.documentElement.getAttribute("buttonlabelclose"));
    gApplyUsed = true;
  }
}

function Apply()
{
  if (ValidateData())
  {
    gActiveEditor.beginTransaction();

    ApplyTableAttributes();

    // handle caption
    var captionloc = gDialog.captionLocation.value;
    var captiontext;
    var cap;
    var i;
    var caps = gTableElement.getElementsByTagName('caption');

    if (captionloc !== '') {
      if (caps.length > 0) {
        cap = caps[0];
      }
      else {  //create new caption node
        cap = gActiveEditor.createElementWithDefaults('caption');
        gTableElement.appendChild(cap);
        var namespace = { value: null };
        var tlm = gActiveEditor.tagListManager;
        captiontext = tlm.getNewInstanceOfNode(tlm.getDefaultParagraphTag(namespace), null, cap.ownerDocument);
        cap.appendChild(captiontext);

      }
      cap.setAttribute('style', 'caption-side: '+ captionloc +';');
      cap.setAttribute('align', captionloc);
    }
    else if (caps.length > 0) { // remove caption(s)
      for (i = caps.length -1; i >= 0; i--) {
        gActiveEditor.deleteNode(caps[i]);
      }
    }

//    ApplyColAndRowAttributes();

    // We may have just a table, so check for cell element
//    if (globalCellElement)
    ApplyCellAttributes();
    gActiveEditor.endTransaction();

    SetCloseButton();
    return true;
  }
  return false;
}

function doHelpButton()
{
  openHelp("table_properties");
}
function onAcceptNewTable()
{
//  if (ValidateData())
//  {
    var color;

    try 
    {
      var wrapping = (gDialog.textwrapchoice.value == "true");  // values are '','true','false'
      if (wrapping.length > 0) {
        gPrefs.setCharPref("editor.table.default_wrapping", wrapping);
      }

      var align = gDialog.hAlignChoices.value;
      if (align.length > 0) {
        gPrefs.setCharPref("editor.table.default_align", align);
      }

      var valign = gDialog.vAlignChoices.value;
      if (valign.length > 0) {
        gPrefs.setCharPref("editor.table.default_valign", valign);
      }
    }
    catch (e) {
      dump(e);
    }

//    try {
//      if (globalElement) gActiveEditor.cloneAttributes(gTableElement, globalElement);

// This is code for creating a new table, not for revising

      // Create necessary rows and cells for the table
    var tableBody = gActiveEditor.createElementWithDefaults("tbody");
    var style;
    ApplyTableAttributes();
    if (tableBody)
    {
      // check here for caption
      var captionloc = gDialog.captionLocation.value;
      var captiontext;
      if (captionloc !== '') {
        var cap = gActiveEditor.createElementWithDefaults('caption');
        var namespace = { value: null };
        var tlm = gActiveEditor.tagListManager;
        gTableElement.appendChild(cap);
        cap.setAttribute('style', 'caption-side: '+ captionloc +';');
        cap.setAttribute('align', captionloc);
        captiontext = tlm.getNewInstanceOfNode(tlm.getDefaultParagraphTag(namespace), null, cap.ownerDocument);
        cap.appendChild(captiontext);
      }
      gTableElement.appendChild(tableBody);
      color = document.getElementById('backgroundCW').getAttribute("color");

      // Create necessary rows and cells for the table
      for (var i = 0; i < gRows; i++)
      {
        var newRow = gActiveEditor.createElementWithDefaults("tr");
        if (newRow)
        {
          tableBody.appendChild(newRow);
          for (var j = 0; j < gColumns; j++)
          {
            var newCell = gActiveEditor.createElementWithDefaults("td");
            if (newCell)
            {
              newRow.appendChild(newCell);
              ApplyAttributesToOneCell(newCell, makeSourceLineObject(document.getElementById("BordersPreviewCenterCell")));              
            }
          }
        }
      }
    }
      // Detect when entire cells are selected:
        // Get number of cells selected
    var tagNameObj = { value: "" };
    var countObj = { value: 0 };
    var element = gActiveEditor.getSelectedOrParentTableElement(tagNameObj, countObj);
    var deletePlaceholder = false;

    if (tagNameObj.value == "table")
    {
      //Replace entire selected table with new table, so delete the table
      gActiveEditor.deleteTable();
    }
    else if (tagNameObj.value == "td")
    {
      if (countObj.value >= 1)
      {
        if (countObj.value > 1)
        {
          // Assume user wants to replace a block of
          //  contiguous cells with a table, so
          //  join the selected cells
          gActiveEditor.joinTableCells(false);
        
          // Get the cell everything was merged into
          element = gActiveEditor.getFirstSelectedCell();
        
          // Collapse selection into just that cell
          gActiveEditor.selection.collapse(element,0);
        }

        if (element)
        {
          // Empty just the contents of the cell
          gActiveEditor.deleteTableCellContents();
        
          // Collapse selection to start of empty cell...
          gActiveEditor.selection.collapse(element,0);
          // ...but it will contain a <br> placeholder
          deletePlaceholder = true;
        }
      }
    }

    // true means delete selection when inserting
    gActiveEditor.markNodeDirty(gTableElement);
    gActiveEditor.insertElementAtSelection(gTableElement, true);
    gTableElement.normalize();

//      if (deletePlaceholder && gTableElement && gTableElement.nextSibling)
//      {
//        // Delete the placeholder <br>
//        gActiveEditor.deleteNode(gTableElement.nextSibling);
//      }
  return true;
}

function onAccept()
{
  if (!data) {
    return onAcceptNewTable();
  }
  // Do same as Apply and close window if ValidateData succeeded
  var retVal = Apply();
//  if (gActiveEditor) {
//    gActiveEditor.deleteNode(gTableElement);
//    gActiveEditor.undo(1);
//  }
  gActiveEditor.markNodeDirty(gTableElement);

  if (retVal)
    SaveWindowLocation();
  return retVal;
}

function createPreviewChangeArray()
{
  var theChanges = { obj : {}, style : {}, table : {}, tableStyle : {} };
  return theChanges;
}

function doInitialPreviewSetup()
{
  var theChanges = createPreviewChangeArray();
  var borderProps = ["style", "width", "color"];
  var theSide, theProp;
  for (var nProp = 0; nProp < borderProps.length; ++nProp)
  {
    theProp = borderProps[nProp];
    for (var ix = 0; ix < gBorderSides.length; ++ix)
    {
      theSide = gBorderSides[ix];
      theChanges.style[getBorderSideAttrString(theSide, theProp)] = gCollatedCellData.border[theProp][theSide];
    }
  }
  theChanges.style["background-color"] = gCollatedCellData.background;
  theChanges.tableStyle["background-color"] = gTableColor;
  theChanges.tableStyle["border-collapse"] = gBorderCollapse;
  updateSample(theChanges);
}

function checkPreviewChanges(controlID)
{
  var sides = ["top","bottom","left","right"];
  var i;
  var style;
  var borderw
  var borderst;
  var bordercolor = "unspec";
  var bordervalues;
  var valuearray;

  style = gDialog.BordersPreviewCenterCell.getAttribute("style") || "";
  for (i = 0; i < 4; i++) {
    if (gDialog.BorderSideSelectionList.value ==="all" || gDialog.BorderSideSelectionList.value === sides[i]) {
      // remove part of style string
      borderst = gDialog.CellBorderStyleList.value;
      borderw = borderst==="double"?"medium":"thin";
      if (borderst != "unspec") {
        bordervalues = style;
        bordervalues.replace("/border-"+sides[i]+": ([^;]*);/","");
        bordervalues.replace("! important;", "");
        // bordervalues now looks like "thick solid red"
        valuearray = bordervalues.split(" ");
        valuearray = valuearray.slice(0,3);
        if (borderw !== "unspec") valuearray[0] = borderw==="none"?0:borderw;
        else valuearray[0]="";
        if (borderst !== "unspec") valuearray[1] = borderst;
        else valuearray[1] = "";
        if (bordercolor != "unspec") valuearray[2] = bordercolor;
        else valuearray[2] = "";
        style.replace("/border-"+sides[i]+": ([^;]*);/","");
        // sample style attribute: border-bottom: thick solid red
        style += 'border-' + sides[i] + ": " + valuearray.join(" ") + ' ! important; ';
      }
      else {
        style.replace("/border-"+sides[i]+": ([^;]*);/","");
      }
    }
  }
  gDialog.BordersPreviewCenterCell.setAttribute("style", style);
 //  var bChanged = false;
 //  var theChanges = createPreviewChangeArray();
 //  var sideString = (gCurrentSide == "all") ? "" : gCurrentSide + "-";
 // // var bBackgroundIsSelection = (gDialog.BackgroundSelectionRadioGroup.value == "selection");
 //  switch(controlID)
 //  {
 //    case "cellBorderStyleList":
 //      if (gCollatedCellData.border.style[gCurrentSide] != gDialog.CellBorderStyleList.value)
 //      {
 //        gCollatedCellData.border.style[gCurrentSide] = gDialog.CellBorderStyleList.value;
 //        bChanged = true;
 //        theChanges.style["border-" + sideString + "style"] = gDialog.CellBorderStyleList.value;
 //        gCellChangeData.border.style.push(gCurrentSide);
 //        if (borderStyleToBorderCollapse(gDialog.CellBorderStyleList.value) != gBorderCollapse)
 //        {
 //          gBorderCollapse = borderStyleToBorderCollapse(gDialog.CellBorderStyleList.value);
 //          gTableChangeData.borderCollapse = true;
 //        }
 //      }
 //    break;
 //    case "cellBorderWidthList":
 //      if (gCollatedCellData.border.width[gCurrentSide] != gDialog.CellBorderWidthList.value)
 //      {
 //        gCollatedCellData.border.width[gCurrentSide] = gDialog.CellBorderWidthList.value;
 //        bChanged = true;
 //        theChanges.style["border-" + sideString + "width"] = gDialog.CellBorderWidthList.value;
 //        gCellChangeData.border.width.push(gCurrentSide);
 //      }
 //    break;

 //    default:
 //    break;
 //  }
 //  if (bChanged)
 //    updateSample(theChanges);
}

//Aren't all of these handled using style changes?
function updateSample(newValues)
{
  //BBM
  return;
  var sizeStrings = {none : "1px", thin : "2px", medium : "3px", thick : "4px"};
  
  for (var styleChange in newValues.style)
  {
    var newValue = newValues.style[styleChange];
    switch(styleChange)
    {
      case "border-width":
      case "border-top-width":
      case "border-right-width":
      case "border-bottom-width":
      case "border-left-width":
        newValue = sizeStrings[newValue];
      break;
    }
    gDialog.BordersPreviewCenterCell.style.setProperty(styleChange, newValue, "important");
  }
  for (var objChange in newValues.obj)
    gDialog.BordersPreviewCenterCell.setAttribute(objChange, newValues.obj[objChange]);
  for (var tableStyleChange in newValues.tableStyle)
    gDialog.BordersPreview.style.setProperty(tableStyleChange, newValues.tableStyle[tableStyleChange], "important");
  for (var tableObjChange in newValues.table)
    gDialog.BordersPreview.setAttribute(tableObjChange, newValues.table[tableObjChange]);
}

//NOTES: The return from this will be an object describing the style, width, and color settings applicable to the borders of selected cells.
//  "style" will be one of the following: "none", "hidden", "dotted", "dashed", "solid", "double", "groove", "ridge"; 
//     "inset" and "outset" are legal CSS values, but since we're not currently looking at support for the "separated borders model", 
//     aren't available in our interface and are mapped if they occur to "ridge" and "groove" respectably. Since the combined attributes 
//     to be supplied must come from this list, we can just use a plurality count (after adjusting for border conflicts to indicate what
//     actually is showing up).
//  "width" is a typical "length" CSS measurement. But how should we handle it for purposes of creating a collated value? Could work
//    with a median or weighted average...but of course there's the difficulty of comparing apples to oranges here.
//    I believe the solution for width is to use just "thin", "medium", and "thick" as values, translating anything else to these.
//  "color" is a typical CSS color value. Again, we could use a weighted average on the RGB values - probably okay to do that.
//Some question can be raised for each of these regarding counting default values vs. explicit ones. My inclination is to treat them
//  equally - if one cell has a bottom border color of "red" and all the others don't specify one, the overall bottom border color
//  would then be treated as the default "black". However, it might be best to count only cells with visible borders in this calculation.
//  The other difficulty to be kept in mind is "conflicts" - according to the CSS spec, properties specified for, say, both the right
//  border of one cell and the left border of the next cell to the right should be resolved to the leftmost (and topmost) cells.

  //HOW TO PROCEED? THIS STUFF CAN BE SET JUST BY THE "border" ATTRIBUTE, OR BY "border-top" etc. or "border-top-style" etc.
  //Real question is what does msiGetHTMLOrCSSStyleValue do?? We probably want to use window.getComputedStyle(element) and then
  //  query it for each specific piece...
  //The other germane questions: where should we store this information? How to respond when the user changes a border style value? (Compare the old SWP code?)
  //  The "how to respond" here has to do with enforcing conflicts and ambiguities - when you change the "right border" attributes of a
  //  group of cells, it's likely to change the de facto values for left borders, for instance.
  //Perhaps we should make the reviseData to be held by the dialog a subclass of the normal table object properties data and have it
  //  keep the relevant stuff with each cell in its grid of cells?? No - we'll just create our own grid of style data and relevant attributes.

function initCellData(reviseData)
{
  if (!gOurCellData)
  {
    var tableDims = reviseData.getTableDims();
    gOurCellData = [];
    for (var ix = 0; ix < tableDims.nRows; ++ix)
      gOurCellData[ix] = new Array(tableDims.nCols);
  }
}

function getCellDataForSelection(reviseData)
{
  initCellData(reviseData);
  var cellIter = reviseData.beginSelectedCellIteration(gSelectionTypeStr);
  var currCell = reviseData.getNextSelectedCell(cellIter);
  while (currCell)
  {
    storeCellDataForCell(gOurCellData, currCell, cellIter, reviseData);
    currCell = reviseData.getNextSelectedCell(cellIter);
  }
  return collateCellData(gOurCellData, reviseData);
}

function collateCellData(ourCellData, reviseData)
{
  var retCellData = {};
  retCellData.border = collateCellBorderData(ourCellData, reviseData);
  retCellData.size = collateCellSizeData(ourCellData, reviseData);
  retCellData.align = collateCellAlignmentData(ourCellData, reviseData);
  retCellData.wrap = collateCellWrapData(ourCellData, reviseData);
  retCellData.background = collateCellBackgroundData(ourCellData, reviseData);
  retCellData.cellType = collateCellTypeData(ourCellData, reviseData);
  return retCellData;
}

function findPlurality(aCountArray)
{
  var maxCount = 0;
  var maxItem = null;
  for (var aData in aCountArray)
  {
    if (aCountArray[aData] > maxCount)
    {
      maxItem = aData;
      maxCount = aCountArray[aData];
    }
  }
  return maxItem;
}

function findMedian(anArray, defaultVal)
{
  if (!anArray.length)
    return defaultVal;
  var sortArray = [];
  sortArray = anArray.concat([]);  //is this really the best way to copy an array?
  sortArray.sort( function(a,b) {return (a-b);} );
  var nMid = Math.floor( (sortArray.length-1) / 2 );
  return sortArray[nMid];
}

function getTotalCount(aCountArray)
{
  var theTotal = 0;
  for (var aData in aCountArray)
    theTotal += aCountArray[aData];
  return theTotal;
}

//It's intended that "theCellData" being passed here is the value of "ourCellData[nRow][nCol]".
function interpretCSSSizeValue(inValue, inUnit, outUnit, theCellData)
{
  var retVal = inValue;
  if (inUnit != outUnit)
  {
    if (!("mUnitsList" in theCellData) || !theCellData.mUnitsList)
      theCellData.mUnitsList = msiCreateCSSUnitsListForElement(theCellData.mCell);
    retVal = theCellData.mUnitsList.convertUnits(inValue, inUnit, outUnit);
  }
  return retVal;
}

function interpretCSSBorderWidth(inValueStr, theCellData)
{
  var retVal = "medium";
  switch(inValueStr)
  {
    case "none":
    case "thin":
    case "medium":
    case "thick":
      retVal = inValueStr;
    break;
    default:
      var theWidth = msiGetNumberAndLengthUnitFromString (inValueStr);
      if (!theWidth)
        theWidth = {number : Number(inValueStr), unit : "px"};
      var pxWidth = interpretCSSSizeValue(theWidth.number, theWidth.unit, "px", theCellData);
      switch(pxWidth)
      {
        case 0:         retVal = "none";         break;
        case 1:         retVal = "thin";         break;
        case 2:         retVal = "medium";       break;
        default:
        case 3:         retVal = "thick";        break;
      }
    break;
  }
  return retVal;
}

function interpretBorderStyle(styleStr)
{
  switch(styleStr)
  {
    case "inset":
      return "ridge";
    case "outset":
      return "groove";
    default:
    break;
  }
  return styleStr;
}

function interpretCSSHAlignValue(hAlignStr)
{
  var retStr = hAlignStr;
  switch(hAlignStr)
  {
    case "start":                 retStr = "left";             break;
    case "end":                   retStr = "right";            break;
    default:                                                   break;
  }
  return retStr;
}

function getCellDataFontHeight(theCellData)
{
  var retVal = 12; //universal default...
  if (!("mUnitsList" in theCellData) || !theCellData.mUnitsList)
    theCellData.mUnitsList = msiCreateCSSUnitsListForElement(theCellData.mCell);
  retVal = theCellData.mUnitsList.fontSizeInPoints();
  return retVal;
}

function collateCellBorderData(ourCellData, reviseData)
{
  var styleCount = { top : {none : 0, hidden : 0, dotted : 0, dashed : 0, solid : 0, double : 0, groove : 0, ridge : 0, inset : 0, outset : 0},
                     right : {none : 0, hidden : 0, dotted : 0, dashed : 0, solid : 0, double : 0, groove : 0, ridge : 0, inset : 0, outset : 0},
                     bottom : {none : 0, hidden : 0, dotted : 0, dashed : 0, solid : 0, double : 0, groove : 0, ridge : 0, inset : 0, outset : 0},
                     left : {none : 0, hidden : 0, dotted : 0, dashed : 0, solid : 0, double : 0, groove : 0, ridge : 0, inset : 0, outset : 0} };
  var widthCount = { top : {none : 0, thin : 0, medium : 0, thick : 0},
                     right : {none : 0, thin : 0, medium : 0, thick : 0},
                     bottom : {none : 0, thin : 0, medium : 0, thick : 0},
                     left : {none : 0, thin : 0, medium : 0, thick : 0} };
  var colorCount = { top : {black : 0},
                     right : {black : 0},
                     bottom : {black : 0}, 
                     left : {black : 0} };

  var cellIter = reviseData.beginSelectedCellIteration(gSelectionTypeStr);
  var currCell = reviseData.getNextSelectedCell(cellIter);
  var currData = null;
  var borderNames = ["top", "right", "bottom", "left"];
  var nRow = 0;
  var nCol = 0;
  var ix;
  var theStyle, theWidth, theColor, whichSide;
  while (currCell)
  {
    nRow = cellIter.nRow;
    nCol = cellIter.nCol;
    if (ourCellData[nRow][nCol] && ourCellData[nRow][nCol].border)
    {
      var styleData = ourCellData[nRow][nCol].border.style;
      if (styleData)
      {
        for (ix = 0; ix < borderNames.length; ++ix)
        {
          whichSide = borderNames[ix];
          theStyle = styleData[whichSide];
          if (theStyle && (theStyle in styleCount[whichSide]))
            ++styleCount[whichSide][theStyle];
        }
      }
      var widthData = ourCellData[nRow][nCol].border.width;
      if (widthData)
      {
        for (ix = 0; ix < borderNames.length; ++ix)
        {
          whichSide = borderNames[ix];
          if (whichSide in widthData)
          {
            theWidth = interpretCSSBorderWidth(widthData[whichSide], ourCellData[nRow][nCol]);
            if (theWidth && (theWidth in widthCount[whichSide]))
              ++widthCount[whichSide][theWidth];
          }
        }
      }
      var colorData = ourCellData[nRow][nCol].border.color;
      if (colorData)
      {
        for (ix = 0; ix < borderNames.length; ++ix)
        {
          whichSide = borderNames[ix];
          theColor = colorData[whichSide];
          if (theColor)
          {
            if (theColor in colorCount[whichSide])
              ++colorCount[whichSide][theColor];
            else
              colorCount[whichSide][theColor] = 1;
          }
        }
      }
    }
    currCell = reviseData.getNextSelectedCell(cellIter);
  }
  var borderData = createCellBorderData();
  for (ix = 0; ix < borderNames.length; ++ix)
  {
    whichSide = borderNames[ix];
    borderData.style[whichSide] = findPlurality( styleCount[whichSide] );
    borderData.width[whichSide] = findPlurality( widthCount[whichSide] );
    borderData.color[whichSide] = findPlurality( colorCount[whichSide] );
  }
  return borderData;
}

//Need to adjust the following to deal with multi-row or multi-col cells!
function collateCellSizeData(ourCellData, reviseData)
{
  var cellHeights = [];
//  for (var ix = 0; ix < rowHeights.length; ++ix)
//    rowHeights[ix] = 0.0;
  var cellWidths = [];
//  for (var jx = 0; jx < colWidths.length; ++jx)
//    colWidths[jx] = 0.0;
  var nRow = 0;
  var nCol = 0;
  var theWidth = 0;
  var theHeight = 0;
  var theFontSize = 0;
  var theFontSizeStr = "";
  var widthUnitFound = null;
  var heightUnitFound = null;
  var theCellData = null;
  var cellIter = reviseData.beginSelectedCellIteration(gSelectionTypeStr);
  var currCell = reviseData.getNextSelectedCell(cellIter);
  var widthUnitsCount = { pt : 0, "in" : 0, mm : 0, cm : 0, pc : 0, em : 0, ex : 0, px : 0 };
  var heightUnitsCount = { pt : 0, "in" : 0, mm : 0, cm : 0, pc : 0, em : 0, ex : 0, px : 0 };
  var widthAutoCount = 0;
  var heightAutoCount = 0;
  var fontSizeCount = {};

  while (currCell)
  {
    theWidth = 0;
    theHeight = 0;
    nRow = cellIter.nRow;
    nCol = cellIter.nCol;
    theCellData = ourCellData[nRow][nCol];
    if (theCellData.size.width != null)
    {
      if (!widthUnitFound)
        widthUnitFound = theCellData.size.width.unit;
      if (theCellData.size.width.unit in widthUnitsCount)
        ++widthUnitsCount[theCellData.size.width.unit];
      theWidth = interpretCSSSizeValue(theCellData.size.width.number, theCellData.size.width.unit, widthUnitFound, theCellData);
      cellWidths.push( theWidth );
    }
    else
      ++widthAutoCount;
    if (theCellData.size.height != null)
    {
      if (!heightUnitFound)
        heightUnitFound = theCellData.size.height.unit;
      if (theCellData.size.height.unit in heightUnitsCount)
        ++heightUnitsCount[theCellData.size.height.unit];
      theHeight = interpretCSSSizeValue(theCellData.size.height.number, theCellData.size.height.unit, heightUnitFound, theCellData);
      cellHeights.push( theHeight );
    }
    else
      ++heightAutoCount;
//    if (rowHeights[nRow] < theHeight)
//      rowHeights[nRow] = theHeight;
//    if (colWidths[nCol] < theWidth)
//      colWidths[nCol] = theWidth;
    theFontSize = getCellDataFontHeight(theCellData);
    if (theFontSize)
    {
      theFontSizeStr = String(theFontSize);
      if (theFontSizeStr in fontSizeCount)
        ++fontSizeCount[theFontSizeStr];
      else
        fontSizeCount[theFontSizeStr] = 1;
    }
    currCell = reviseData.getNextSelectedCell(cellIter);
  }
  theWidth = findMedian(cellWidths, 0.0);
  theHeight = findMedian(cellHeights, 0.0);
  var newUnit = findPlurality(widthUnitsCount);
  if (newUnit != null)
    gCellWidthUnit = newUnit;
  newUnit = findPlurality(heightUnitsCount);
  if (newUnit != null)
    gCellHeightUnit = newUnit;
  gCellFontSize = findPlurality(fontSizeCount);
  var widthIsSet = (getTotalCount(widthUnitsCount) >= widthAutoCount);
  var heightIsSet = (getTotalCount(heightUnitsCount) >= heightAutoCount);
  return {width : theWidth, height : theHeight, bWidthSet : widthIsSet, bHeightSet : heightIsSet};
}

//The strategy here should be - what? Take the plurality?
function collateCellAlignmentData(ourCellData, reviseData)
{
  var nRow = 0;
  var nCol = 0;
  var hAlignCount = {left : 0, right : 0, center : 0, justify : 0};
  var vAlignCount = {baseline : 0, top : 0, bottom : 0, middle : 0};
  var cellIter = reviseData.beginSelectedCellIteration(gSelectionTypeStr);
  var currCell = reviseData.getNextSelectedCell(cellIter);
  var theHAlign, theVAlign;
  while (currCell)
  {
    nRow = cellIter.nRow;
    nCol = cellIter.nCol;
    if (ourCellData[nRow][nCol].align.halign != null)
      theHAlign = ourCellData[nRow][nCol].align.halign;
    if (theHAlign in hAlignCount)
      ++hAlignCount[theHAlign];
    if (ourCellData[nRow][nCol].align.valign != null)
      theVAlign = ourCellData[nRow][nCol].align.valign;
    if (theVAlign in vAlignCount)
      ++vAlignCount[theVAlign];
    currCell = reviseData.getNextSelectedCell(cellIter);
  }
  var retAlign = {};
  retAlign.halign = findPlurality(hAlignCount);
  retAlign.valign = findPlurality(vAlignCount);
  return retAlign;
}

function collateCellWrapData(ourCellData, reviseData)
{
  var nRow = 0;
  var nCol = 0;
  var wrapCount = {wrap : 0, nowrap : 0};
  var cellIter = reviseData.beginSelectedCellIteration(gSelectionTypeStr);
  var currCell = reviseData.getNextSelectedCell(cellIter);
  var theWrap;
  while (currCell)
  {
    nRow = cellIter.nRow;
    nCol = cellIter.nCol;
    if (ourCellData[nRow][nCol].wrap != null)
      theWrap = ourCellData[nRow][nCol].wrap;
    if (theWrap in wrapCount)
      ++wrapCount[theWrap];
    currCell = reviseData.getNextSelectedCell(cellIter);
  }
  return findPlurality(wrapCount);
}
 
function collateCellBackgroundData(ourCellData, reviseData)
{
  var nRow = 0;
  var nCol = 0;
  var backColorCount = {"transparent" : 0};
  var cellIter = reviseData.beginSelectedCellIteration(gSelectionTypeStr);
  var currCell = reviseData.getNextSelectedCell(cellIter);
  var theBackground;
  while (currCell)
  {
    nRow = cellIter.nRow;
    nCol = cellIter.nCol;
    if (ourCellData[nRow][nCol].background != null)
      theBackground = ourCellData[nRow][nCol].background;
    switch(theBackground)
    {
//      case "transparent":
//      case "inherit":
//        //do nothing
//      break;
      default:
        if (theBackground in backColorCount)
          ++backColorCount[theBackground];
        else if (theBackground.length)
          backColorCount[theBackground] = 1;
      break;
    }
    currCell = reviseData.getNextSelectedCell(cellIter);
  }
  return findPlurality(backColorCount);
}

////This one is really undecided....We won't do anything with it for now...
function collateCellTypeData(ourCellData, reviseData)
{
  var nRow = 0;
  var nCol = 0;
  var cellTypeCount = {data : 0, header : 0};
  var cellIter = reviseData.beginSelectedCellIteration(gSelectionTypeStr);
  var currCell = reviseData.getNextSelectedCell(cellIter);
  var theType;
  while (currCell)
  {
    nRow = cellIter.nRow;
    nCol = cellIter.nCol;
    if (ourCellData[nRow][nCol].cellType != null)
      theType = ourCellData[nRow][nCol].cellType;
    if (theType in cellTypeCount)
      ++cellTypeCount[theType];
    currCell = reviseData.getNextSelectedCell(cellIter);
  }
  return findPlurality(cellTypeCount);
}

//  Must also deal with the border stuff in the continuation case - do we want to make the
//  corresponding (missing) side data null rather than strings or numbers???
function storeCellDataForCell(ourCellData, aCell, cellIter, tableData)
{
  var nRow = cellIter.nRow;
  var nRefRow = nRow;
  var nCol = cellIter.nCol;
  var nRefCol = nCol;
  var bNoTop = false;
  var bNoRight = false;
  var bNoBottom = false;
  var bNoLeft = false;
  var cellExtents = tableData.getCellExtent(nRow, nCol);

  if (ourCellData[nRow][nCol] == null)
  {
    if ( (cellExtents.mRowContinuation < 0) || (cellExtents.nColContinuation < 0) )
    {
      if (cellExtents.mRowContinuation < 0)
      {
        nRefRow = nRow + cellExtents.mRowContinuation;
        bNoTop = true;
      }
      if (cellExtents.nColContinuation < 0)
      {
        nRefCol = nCol + cellExtents.mColContinuation;
        bNoLeft = true;
      }
      var refExtents = tableData.getCellExtent(nRefRow, nRefCol);
      if (nRefRow + refExtents.mRowContinuation > nRow)
        bNoBottom = true;
      if (nRefCol + refExtents.mColContinuation > nCol)
        bNoRight = true;
      if (ourCellData[nRefRow][nRefCol])
        ourCellData[nRow][nCol] = ourCellData[nRefRow][nRefCol];
      else
        ourCellData[nRow][nCol] = ourCellData[nRefRow][nRefCol] = getCellDataForCell(aCell, nRefRow, nRefCol);
    }
    else
    {
      ourCellData[nRow][nCol] = getCellDataForCell(aCell, nRow, nCol);
      if (cellExtents.mRowContinuation > 0)
        bNoBottom = true;
      if (cellExtents.mColContinuation > 0)
        bNoRight = true;
      for (var ix = 0; ix <= cellExtents.mRowContinuation; ++ix)
      {
        for (var jx = 0; jx <= cellExtents.mColContinuation; ++jx)
        {
          if ((ix > 0) || (jx > 0))
            ourCellData[nRow + ix][nCol + jx] = ourCellData[nRow][nCol];
        }
      }
    }
  }
  removeMissingSideInfo(ourCellData, nRow, nCol, bNoTop, bNoRight, bNoBottom, bNoLeft);
}

function removeMissingSideInfo(ourCellData, nRow, nCol, bNoTop, bNoRight, bNoBottom, bNoLeft)
{
  for (var aDatum in ourCellData[nRow][nCol].border)
  {
    if (bNoTop)
      ourCellData[nRow][nCol].border[aDatum].top = null;
    if (bNoRight)
      ourCellData[nRow][nCol].border[aDatum].right = null;
    if (bNoBottom)
      ourCellData[nRow][nCol].border[aDatum].bottom = null;
    if (bNoLeft)
      ourCellData[nRow][nCol].border[aDatum].left = null;
  }
}

function getCellDataForCell(aCell, nRow, nCol)
{
  var cellData = { mCell : aCell, border : null, size : null, align : null, wrap : "wrap", background : "transparent", cellType : "data" };
  var defView = aCell.ownerDocument.defaultView;
  var docCSS = defView.QueryInterface(Components.interfaces.nsIDOMViewCSS);
  var computedStyle = docCSS.getComputedStyle(aCell, "");
  var elementUnitsList = msiCreateCSSUnitsListForElement(aCell);

  var aValue, attrName;
  
  cellData.border = getBorderDataForCell(aCell, nRow, nCol, elementUnitsList, computedStyle);
  cellData.size = getSizeDataForCell(aCell, nRow, nCol, elementUnitsList);

  if (gIsMatrix)
    cellData.align = getAlignDataForMatrixCell(aCell, nRow, nCol);
  else
  {
    cellData.align = { halign : "", valign : ""};
    aValue = aCell.getAttribute("valign");
    if (aValue)
      cellData.align.valign = aValue;
    else
    {
      aValue = computedStyle.getPropertyCSSValue("vertical-align");
      if (aValue)
        cellData.align.valign = aValue.getStringValue();
    }
    aValue = aCell.getAttribute("align");
    if (aValue)
      cellData.align.halign = aValue;
    else
    {
      aValue = computedStyle.getPropertyCSSValue("text-align");
      if (aValue)
        cellData.align.halign = interpretCSSHAlignValue(aValue.getStringValue());
    }
  }
  
  aValue = computedStyle.getPropertyCSSValue("white-space");
  if (aValue && aValue.getStringValue() == "nowrap")
    cellData.wrap = "nowrap";
  else if (aCell.getAttribute("nowrap") == "true")
    cellData.wrap = "nowrap";
  
  aValue = computedStyle.getPropertyCSSValue("background-color");
  if (aValue)
    cellData.background = msiCSSUtils.getRGBColorValFromCSSPrimitive(aValue);
  
  return cellData;
}

function getBorderDataForCell(aCell, nRow, nCol, elementUnitsList, computedStyle)
{
  var borderData = null;
  var ix;
  if (gIsMatrix)
    borderData = getBorderDataForMatrixCell(aCell, nRow, nCol, elementUnitsList);
  else
    borderData = createCellBorderData();
  var aValue, attrName;
  if (!gIsMatrix)
  {
    for (var ix = 0; ix < gBorderSides.length; ++ix)
    {
      attrName = gBorderSides[ix];
      aValue = aCell.getAttribute("line-" + attrName);
      if (!aValue)
      {
        aValue = computedStyle.getPropertyCSSValue("border-" + attrName + "-style");
        if (aValue)
          borderData.style[attrName] = aValue.getStringValue();
      }
    }
  }
  for (ix = 0; ix < gBorderSides.length; ++ix)
  {
    attrName = gBorderSides[ix];
    aValue = computedStyle.getPropertyCSSValue("border-" + attrName + "-color");
    if (aValue)
      borderData.color[attrName] = msiCSSUtils.getRGBColorValFromCSSPrimitive(aValue);
  }
  for (ix = 0; ix < gBorderSides.length; ++ix)
  {
    attrName = gBorderSides[ix];
    aValue = computedStyle.getPropertyCSSValue("border-" + attrName + "-width");
    if (aValue)
      borderData.width[attrName] = msiCSSUtils.getLengthWithUnitsStringFromCSSPrimitiveValue(aValue);
  }
  return borderData;
}

function getBorderDataForMatrixCell(aCell, nRow, nCol, elementUnitsList)
{
  var whiteSpace = /(^\s+)/;
  var borderData = createCellBorderData();
  var matrixColLines, matrixRowLines;
  var matrixColLinesStr = gTableElement.getAttribute("columnlines");
  if (matrixColLinesStr)
    matrixColLines = matrixColLinesStr.split(whiteSpace);
  if (matrixColLines && matrixColLines.length)
  {
    if (nCol < matrixColLines.length)
      borderData.style.right = matrixColLines[nCol];
    else
      borderData.style.right = matrixColLines[matrixColLines.length - 1];
    if (nCol > 0)
    {
      if (nCol - 1 < matrixColLines.length)
        borderData.style.left = matrixColLines[nCol - 1];
      else
        borderData.style.left = matrixColLines[matrixColLines.length - 1];
    }
  }
  var matrixRowLinesStr = gTableElement.getAttribute("rowlines");
  if (matrixRowLinesStr)
    matrixRowLines = matrixRowLinesStr.split(whiteSpace);
  if (matrixRowLines && matrixRowLines.length)
  {
    if (nRow < matrixRowLines.length)
      borderData.style.bottom = matrixRowLines[nRow];
    else
      borderData.style.bottom = matrixRowLines[matrixRowLines.length - 1];
    if (nRow > 0)
    {
      if (nRow - 1 < matrixRowLines.length)
        borderData.style.top = matrixRowLines[nRow - 1];
      else
        borderData.style.top = matrixRowLines[matrixRowLines.length - 1];
    }
  }
  return borderData;
}

function getSizeDataForCell(aCell, nRow, nCol, elementUnitsList)
{
  if (gIsMatrix)
    return getSizeDataForMatrixCell(aCell, nRow, nCol);

  var sizeData = { width : null, height : null };
  var theValue = "";
  var anotherValue = "";

  var theCol = getColNodeForColumn(nCol);
  if (theCol)
  {
    theValue = msiGetHTMLOrCSSStyleValue(gActiveEditorElement, theCol, "width", "width");
    anotherValue = msiGetHTMLOrCSSStyleValue(gActiveEditorElement, theCol, "min-width", "width");
    if (anotherValue && anotherValue.length)
      theValue = anotherValue;
  }
  anotherValue = msiGetHTMLOrCSSStyleValue(gActiveEditorElement, aCell, "cellwidth", "width", true);
  if (anotherValue && anotherValue.length)
    theValue = anotherValue;
  anotherValue = msiGetHTMLOrCSSStyleValue(gActiveEditorElement, aCell, "cellwidth", "min-width", true);
  if (anotherValue && anotherValue.length)
    theValue = anotherValue;
  if (theValue)
    sizeData.width = msiGetNumberAndLengthUnitFromString(theValue);


  theValue = "";
  anotherValue = "";
  var theRow = getRowNodeForRow(nRow);
  if (theRow)
    theValue = msiGetHTMLOrCSSStyleValue(gActiveEditorElement, theRow, "height", "height");
  anotherValue = msiGetHTMLOrCSSStyleValue(gActiveEditorElement, aCell, "cellheight", "height", true);
  if (anotherValue && anotherValue.length)
    theValue = anotherValue;
  if (theValue)
    sizeData.height = msiGetNumberAndLengthUnitFromString(theValue);
  return sizeData;
}

function getSizeDataForMatrixCell(aCell, nRow, ncol, elementUnitsList)
{
  var whiteSpace = /(^\s+)/;
  var sizeData = { width : null, height : null };
  var colWidths, theValue;
  var colWidthsStr = gTableElement.getAttribute("columnwidth");
  if (colWidthsStr)
    colWidths = colWidthsStr.split(whiteSpace);
  if (colWidths && colWidths.length)
  {
    if (nCol < colWidths.length)
      theValue = colWidths[nCol];
    else
      theValue = colWidths[colWidths.length - 1];
    if ( (theValue != "auto") && (theValue != "fit") )
      sizeData.width = msiGetNumberAndLengthUnitFromString (theValue);
  }
  return sizeData;
}

function getAlignDataForMatrixCell(aCell, nRow, nCol)
{
  var whiteSpace = /(^\s+)/;
  var alignData = { halign : "", valign : ""};
  var halignVals, valignVals;
  var halignValsStr = gTableElement.getAttribute("columnalign");
  if (halignValsStr)
    halignVals = halignValsStr.split(whiteSpace);
  if (halignVals && halignVals.length)
  {
    if (nCol < halignVals.length)
      alignData.halign = halignVals[nCol];
    else
      alignData.halign = halignVals[halignVals.length - 1];
  }
  var valignValsStr = gTableElement.getAttribute("rowalign");
  if (valignVals && valignVals.length)
  {
    if (nCol < valignVals.length)
      alignData.valign = valignVals[nCol];
    else
      alignData.valign = valignVals[valignVals.length - 1];
  }
  return alignData;
}
