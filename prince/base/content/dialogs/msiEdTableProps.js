/* ***** BEGIN LICENSE BLOCK *****
 2009 MacKichan Software, Inc. *
 * ***** END LICENSE BLOCK ***** */

//Comments on restructuring for Prince:
//  We'll use three tabs in this dialog, changing the conceptual organization of the controls accordingly.
//  Tab 1 will be titled "Alignment". Here we should have controls to handle:
//         (i) Vertical alignment for cells or rows (Top/Middle/Bottom - or?)
//        (ii) Horizontal alignment for cells or columns (Left/Center/Right - or Justify? Character Align? [last not supported in Composer, so let's not])
//       (iii) Table baseline with respect to environment (Top row/Middle/Bottom row - or?)
//        (iv) (Maybe) Caption side selection
// Tab 2 will be titled "Borders and Backgrounds". Here we should set:
//         (i) Border styles/thicknesses for cells or cell groups (allow this to be set for columns, colgroups, rows, rowgroups? 
//               CSS/HTML gives explicit override instructions so it makes sense to allow setting it for explicit targets. But see comments below.) 
//        (ii) Border colors for cells or cell groups.
//       (iii) Border styles/thicknesses for entire table.
//        (iv) Border colors for entire table. (Note that the entire table settings are somewhat limited in terms of differing between
//               cells! Maybe we don't want this at all and just set everything on the cell/row/column level?)
//       NOTE that (i)-(iv) should be equipped with the kind of sample we had in SWP originally; Barry suggests that we use right-mouse
//         functionality to allow the sample drawing to be directly used for selection by the user. However, the structure should
//         probably change somewhat, so that the Top/Right/Bottom/Left/All/Outline choices will control the operation while the
//         None/Single/Double/whatever selections will show up as the choices to be made.
//         (v) Background colors for cells, rows, columns, row groups, col groups, table.
// Tab 3 will be titled "Size". Here we set:
//         (i) Width.
//        (ii) Height.
//        NOTE that these are problematic. If a rectangular block of cells is selected, in our current paradigm this could only
//          be used to set the width and height of all the cells in the selection. If some but not all of them have previously
//          been set, "Auto" would have to show up as the size when the dialog opens - and that might not be appealing.
//        I believe what should be done is:
//          (i) If the selection contains multiple cells, the width and height input boxes should be activated only if all the cells
//              in the selection share a common width/height.
//         (ii) If the selection spans columns, the "Column" item should be available in the drop-down listbox. If there is a common width
//              set for the column(s), the listbox should open with "Column" selected. Similarly for column groups, rows, and row groups.
//        (iii) Wrap/no-wrap behavior of cells
//         (iv) (Maybe) Header vs. regular cell? More generally, make a column group for a set of columns? Make a bottom row a footer?
//      Anything else? Maybe the size of margins or padding should be set here? We'll leave that for another day.
//    Actually, this tab seems to contain too little - particularly since the size controls may be often disabled.
//    It would be tempting to put the Wrap/no-wrap, Header Cell/Table Cell, and Caption placement controls here and entitle it
//      "Size and Layout". Then the first tab would be simply "Alignment".
// In any case the salient issue is how explicitly we should allow these settings for differing targets. They're available in some form
//   or other for individual cells, rows, columns, row groups (which only means Header/Body/Footer), column groups, or the entire table.
//   Ideally we'd like to offer for each a choice of what the user is setting the value for, and allow him to set for each available
//   choice each time he visits the dialog.
// Additionally, the Composer dialog offers the option of changing the selection while in the dialog and continuing to edit. This is
//   probably very helpful, but I don't know whether I want to try to implement it at this point.

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

//var gRowCount = 1;
//var gColCount = 1;
//var gLastRowIndex;
//var gLastColIndex;
//var gNewRowCount;
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

//var gSelectedCellCount = 0;
var gOurCellData;
var gInitialCellData;
var gCollatedCellData;  //This starts out as the same as the previous, but changes as the user makes selections in the dialog.
var gTableColor;
var gTableCaptionPlacement;
var gTableBaseline = "baseline";

var gCellChangeData;
var gTableChangeData;
var gCurrentSide = "";

var gSelectionTypeStr = "Table";
var gApplyUsed = false;
//var gSelection = null;
//var gCellDataChanged = false;
var gCanDelete = false;
var gPrefs = GetPrefs();
var gUseCSS = true;

var gActiveEditorElement;
var gActiveEditor;

var gCellWidthUnit = "pc";
var gCellHeightUnit = "pc";
var gWidthUnitsController;
var gHeightUnitsController;
var gCellFontSize = 12;

var data;

// dialog initialization code

function setVariablesForControls()
{
  // Get dialog widgets - Table Panel
//  gDialog.TableRowsInput = document.getElementById("TableRowsInput");
//  gDialog.TableColumnsInput = document.getElementById("TableColumnsInput");
//  gDialog.TableWidthInput = document.getElementById("TableWidthInput");
//  gDialog.TableWidthUnits = document.getElementById("TableWidthUnits");
//  gDialog.TableHeightInput = document.getElementById("TableHeightInput");
//  gDialog.TableHeightUnits = document.getElementById("TableHeightUnits");
//  try {
//    if (!gPrefs.getBoolPref("editor.use_css") || (gActiveEditor.flags & 1))
//    {
//      gUseCSS = false;
//      var tableHeightLabel = document.getElementById("TableHeightLabel");
//      tableHeightLabel.parentNode.removeChild(tableHeightLabel);
//      gDialog.TableHeightInput.parentNode.removeChild(gDialog.TableHeightInput);
//      gDialog.TableHeightUnits.parentNode.removeChild(gDialog.TableHeightUnits);
//    }
//  } catch (e) {}

//  gDialog.SpacingInput = document.getElementById("SpacingInput");
//  gDialog.PaddingInput = document.getElementById("PaddingInput");
//  gDialog.TableAlignList = document.getElementById("TableAlignList");

  gDialog.TabBox =  document.getElementById("TabBox");
  gDialog.AlignmentTab =  document.getElementById("AlignmentTab");
  gDialog.LinesTab =  document.getElementById("LinesTab");
  gDialog.LayoutTab =  document.getElementById("LayoutTab");

  gDialog.TableCaptionList = document.getElementById("TableCaptionList");
//  gDialog.TableInheritColor = document.getElementById("TableInheritColor");
//  gDialog.SelectionList = document.getElementById("SelectionList");
//  gDialog.PreviousButton = document.getElementById("PreviousButton");
//  gDialog.NextButton = document.getElementById("NextButton");
  // Currently, we always apply changes and load new attributes when changing selection
  // (Let's keep this for possible future use)
  //gDialog.ApplyBeforeMove =  document.getElementById("ApplyBeforeMove");
  //gDialog.KeepCurrentData = document.getElementById("KeepCurrentData");
  gDialog.ColAlignRadioGroup =  document.getElementById("ColumnAlignRadioGroup");
  gDialog.RowAlignRadioGroup =  document.getElementById("RowAlignRadioGroup");
  gDialog.TableBaselineRadioGroup =  document.getElementById("TableBaselineRadioGroup");

//  gDialog.BorderSelectionList =  document.getElementById("BorderSelectionList");
  gDialog.BorderSideSelectionList = document.getElementById("BorderSideSelectionList");
  gDialog.CellBorderStyleList = document.getElementById("cellBorderStyleList");
  gDialog.BordersPreview =  document.getElementById("BordersPreview");
  gDialog.BordersPreviewCenterCell = document.getElementById("BordersPreviewCenterCell");
  gDialog.CellBorderWidthList = document.getElementById("cellBorderWidthList");
//  gDialog.BorderWidthInput = document.getElementById("BorderWidthInput");
//  gDialog.BorderWidthUnits = document.getElementById("BorderWidthUnits");
//  gDialog.BackgroundColorCheckbox = document.getElementById("BackgroundColorCheckbox");
  gDialog.BackgroundSelectionRadioGroup = document.getElementById("BackgroundSelectionRadioGroup");

  gDialog.CellHeightInput = document.getElementById("CellHeightInput");
  gDialog.CellHeightUnits = document.getElementById("CellHeightUnits");
  gDialog.CellWidthInput = document.getElementById("CellWidthInput");
  gDialog.CellWidthUnits = document.getElementById("CellWidthUnits");
//  gDialog.CellHAlignList = document.getElementById("CellHAlignList");
//  gDialog.CellVAlignList = document.getElementById("CellVAlignList");
//  gDialog.CellInheritColor = document.getElementById("CellInheritColor");
  gDialog.CellStyleList = document.getElementById("CellStyleList");
//  gDialog.TextWrapList = document.getElementById("TextWrapList");

  // In cell panel, user must tell us which attributes to apply via checkboxes,
  //  else we would apply values from one cell to ALL in selection
  //  and that's probably not what they expect!
  //rwa Is this still valid to do?
  gDialog.CellHeightCheckbox = document.getElementById("CellHeightCheckbox");
  gDialog.CellWidthCheckbox = document.getElementById("CellWidthCheckbox");
//  gDialog.CellHAlignCheckbox = document.getElementById("CellHAlignCheckbox");
//  gDialog.CellVAlignCheckbox = document.getElementById("CellVAlignCheckbox");
//  gDialog.CellStyleCheckbox = document.getElementById("CellStyleCheckbox");
  gDialog.TextWrapCheckbox = document.getElementById("TextWrapCheckbox");
//  gDialog.CellColorCheckbox = document.getElementById("CellColorCheckbox");
//  gDialog.TableTab = document.getElementById("TableTab");
//  gDialog.CellTab = document.getElementById("CellTab");
//  gDialog.AdvancedEditCell = document.getElementById("AdvancedEditButton2");
  // Save "normal" tooltip message for Advanced Edit button
//  gDialog.AdvancedEditCellToolTipText = gDialog.AdvancedEditCell.getAttribute("tooltiptext");

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
  collatedCellData.size.bHeightSet = (initialCellData.size.bHeightSet != null) && (initialCellData.size.bHeightSet = true);
  collatedCellData.size.bWidthSet = (initialCellData.size.bWidthSet != null) && (initialCellData.size.bWidthSet = true);
}

function setDataFromReviseData(reviseData, commandStr)
{
  gSelectionTypeStr = reviseData.getSelectionType(commandStr);  //Do we even want to call this?
  gSelectedCellsType = translateSelectionTypeString(gSelectionTypeStr);
  gTableElement = reviseData.getReferenceNode();
  gIsMatrix = reviseData.isMatrix();
  //Another question - do we want to retain the set of selected cells? Or always use a iterator function similar to getNextSelectedCell?
  //NOTE that this will be either Cell, CellGroup, Rows, Columns, or Table. We should probably also add ColGroup and RowGroup, but...
  //Also, do we want to retain the dialog's use of a cloned table element to perform actions on? If so, we'd need to be using a cloned
  //  "objectData" object?
  gInitialCellData = getCellDataForSelection(reviseData);
  gCollatedCellData = createCellDataObject(gInitialCellData);
  setUpCollatedCellData(gCollatedCellData, gInitialCellData);
  setUpChangeData();
  EnableDisableControls();

  gTableBaseline = msiGetHTMLOrCSSStyleValue(gActiveEditorElement, gTableElement, "valign", "vertical-align");
  gTableColor = msiGetHTMLOrCSSStyleValue(gActiveEditorElement, gTableElement, bgcolor, cssBackgroundColorStr);
  gTableColor = ConvertRGBColorIntoHEXColor(gTableColor);
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
    baseline : false, background : false, caption : false
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
                     width : {top : "medium", right : "medium", bottom : "medium", left : "medium"},
                     color : {top : "black", right : "black", bottom : "black", left : "black"} };
  if (srcBorderData)
  {
    for (var aSide in ["top", "right", "bottom", "left"])
    {
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
  var retSizeData = { width : null, height : null };
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
  if ("halign" in srcAlignData)
    retAlignData.halign = srcAlignData.halign;
  if ("valign" in srcAlignData)
    retAlignData.valign = srcAlignData.valign;
  return retAlignData;
}

function getBorderSideAttrString(aSide, anAttr)
{
  var whichSideStr = (aSide == "all") ? "" : (gCurrentSide + "-");
  return ("border-" + whichSideStr + anAttr);
}

function UseCSSForCellProp(propName)
{
  switch(propName)
  {
    case "border-style":
    case "width":
      return (!gIsMatrix);
    break;
  }
  return true;
}

function ShouldSetWidthOnCols()
{
  return gIsMatrix;
}

function ShouldSetHeightOnRows()
{
  return gIsMatrix;
}

function translateSelectionTypeString(selTypeStr)
{
  switch(selTypeStr)
  {
    case "Row":
      return SELECT_ROW;
    break;
    case "Column":
      return SELECT_COLUMN;
    break;
    case "Table":  //For Table mode, doesn't seem to matter what we say the selection type is. On the other hand, our code will
                   //  pay attention to the selection type string rather than this variable.
    case "Cell":
    case "CellGroup":
    default:
      return SELECT_CELL;
    break;
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

  data = window.arguments[0];
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

  setVariablesForControls();

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

  SetWindowLocation();
}


function InitDialog()
{
  initAlignPanel();
  initBordersPanel();
  initSizeLayoutPanel();

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

//  InitCellPanel();
}

function initAlignPanel()
{
  gDialog.ColAlignRadioGroup.value = gCollatedCellData.align.halign;
  gDialog.RowAlignRadioGroup.value = gCollatedCellData.align.valign;
  gDialog.TableBaselineRadioGroup.value = gTableBaseline;
}

function initBordersPanel()
{
//  gDialog.BorderSelectionList.value = 
//  gDialog.BorderWidthInput.value = 
  var currSide = "all";  //may want this to be persistent
  gDialog.BorderSideSelectionList.value = currSide; //will trigger setCurrSide()

//  gTableColor = msiGetHTMLOrCSSStyleValue(globalTableElement, bgcolor, cssBackgroundColorStr);
  var backColor = gInitialCellData.background;
  if (!backColor || (backColor == "transparent"))
  {
    backColor = gTableColor;
    gDialog.BackgroundSelectionRadioGroup.value = "table";
  }
  else
    gDialog.BackgroundSelectionRadioGroup.value = "selection";
  setColorWell("BackgroundCW", backColor);
//  SetColor("BackgroundCW", backColor);
}

function setCurrSide(newSide)
{
  if (gCurrentSide == newSide)
    return;

  gCurrentSide = newSide;
  gDialog.CellBorderStyleList.value = gCollatedCellData.border.style[gCurrentSide];
  gDialog.CellBorderWidthList.value = gCollatedCellData.border.width[gCurrentSide];
  var cellBorderColor = gCollatedCellData.border.color[gCurrentSide];
  setColorWell("borderCW", cellBorderColor);
  //Note that this one shouldn't require redrawing sample
//  SetColor("borderCW", cellBorderColor);
}

function initSizeLayoutPanel()
{
//  var previousValue = gDialog.CellWidthInput.value;
  var theUnitsList = msiCSSWithFontUnitsList(gCellFontSize, "pt");
  gWidthUnitsController = new msiUnitsListbox(gDialog.CellWidthUnits, [gDialog.CellWidthInput], theUnitsList);
  gHeightUnitsController = new msiUnitsListbox(gDialog.CellHeightUnits, [gDialog.CellHeightInput], theUnitsList);
  var widthStr = String(gCollatedCellData.size.width) + gCellWidthUnit;
  var heightStr = String(gCollatedCellData.size.height) + gCellHeightUnit;
  gWidthUnitsController.setAdditionalCommand("CheckboxChanged('CellWidthCheckbox');");
  gHeightUnitsController.setAdditionalCommand("CheckboxChanged('CellHeightCheckbox');");
  gWidthUnitsController.setUp(gCellWidthUnit, [widthStr]);
  gHeightUnitsController.setUp(gCellHeightUnit, [heightStr]);
  gDialog.CellWidthCheckbox.checked = gCollatedCellData.size.bWidthSet;
  gDialog.CellHeightCheckbox.checked = gCollatedCellData.size.bHeightSet;
//  gDialog.CellWidthInput.value = InitPixelOrPercentMenulist(globalCellElement, gCellElement, "width", "CellWidthUnits", gPixel);
//  gDialog.CellHeightInput.value = InitPixelOrPercentMenulist(globalCellElement, gCellElement, "height", "CellHeightUnits", gPixel);
//  previousValue = gDialog.CellHeightInput.value;
//  gDialog.CellWidthCheckbox.checked = gAdvancedEditUsed && previousValue != gDialog.CellWidthInput.value;
//  gDialog.CellHeightCheckbox.checked = gAdvancedEditUsed && previousValue != gDialog.CellHeightInput.value;

//  var previousIndex = gDialog.TextWrapList.selectedIndex;
//  if (GetHTMLOrCSSStyleValue(globalCellElement, "nowrap", "white-space") == "nowrap")
//    gDialog.TextWrapList.value = "nowrap";
//  else
//    gDialog.TextWrapList.value = "wrap";
  gDialog.TextWrapCheckbox.checked = (gCollatedCellData.wrap != "nowrap");
  
  // Be sure to get caption from table in doc, not the copied "globalTableElement"
  gTableCaptionElement = gTableElement.caption;
  if (gTableCaptionElement)
  {
    gTableCaptionPlacement = msiGetHTMLOrCSSStyleValue(gActiveEditorElement, gTableCaptionElement, "align", "caption-side");
    if (gTableCaptionPlacement != "bottom" && gTableCaptionPlacement != "left" && gTableCaptionPlacement != "right")
      gTableCaptionPlacement = "top";
    gDialog.TableCaptionList.value = gTableCaptionPlacement;
  }
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
//    gDialog.TextWrapCheckbox.checked = gAdvancedEditUsed && previousIndex != gDialog.TextWrapList.selectedIndex;
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

  var bBackgroundIsSelection = (gDialog.BackgroundSelectionRadioGroup.value == "selection");
  var colorObj = { Type:"", TableColor:0, CellColor:0, NoDefault:false, Cancel:false, BackgroundColor:0 };

  switch( ColorWellID )
  {
    case "BackgroundCW":
      if (bBackgroundIsSelection)
      {
        colorObj.Type = "Cell";
        colorObj.CellColor = gCollatedCellData.background;
      }
      else
      {
        colorObj.Type = "Table";
        colorObj.TableColor = gTableColor;
      }
      break;
    case "borderCW":
      colorObj.Type = "Cell";
      colorObj.CellColor = gCollatedCellData.border.color[gCurrentSide];
      break;
  }

  window.openDialog("chrome://editor/content/EdColorPicker.xul", "_blank", "chrome,close,titlebar,modal", "", colorObj);

  // User canceled the dialog
  if (colorObj.Cancel)
    return;

  var changeArray = createPreviewChangeArray();
  var borderSideAttrStr = "";
  switch( ColorWellID )
  {
    case "BackgroundCW":
      if (bBackgroundIsSelection)
      {
        gCollatedCellData.background = colorObj.CellColor;
//        SetColor(ColorWellID, gCollatedCellData.background);  Do we really want to actually SetColor at this point?
        changeArray.style["background-color"] = colorObj.CellColor;
        gCellChangeData.background = true;
        setColorWell(ColorWellID, gCollatedCellData.background);
      }
      else
      {
        gTableColor = colorObj.BackgroundColor;
//        SetColor(ColorWellID, gTableColor);  Do we really want to actually SetColor at this point?
        changeArray.tableStyle["background-color"] = gTableColor;
        gTableChangeData.background = true;

        setColorWell(ColorWellID, gTableColor);
      }
    break;
    case "borderCW":
      gCollatedCellData.border.color[gCurrentSide] = colorObj.CellColor;
//      SetColor(ColorWellID, gCollatedCellData.border.color[gCurrentSide]);  Do we really want to actually SetColor at this point?
      borderSideAttrStr = getBorderSideAttrString(gCurrentSide, "color");
      changeArray.style[borderSideAttrStr] = colorObj.CellColor;
      gCellChangeData.border.color.push(gCurrentSide);
      setColorWell(ColorWellID, gCollatedCellData.border.color[gCurrentSide]);
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
//rwa //  if (gDialog.TextWrapCheckbox.checked)
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
//rwa function ValidateData()
//rwa {
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
//rwa   return true;
//rwa }

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
function SetCheckbox(checkboxID)
{
  if (checkboxID && checkboxID.length > 0)
  {
    // Set associated checkbox
    document.getElementById(checkboxID).checked = true;
  }
  gCellDataChanged = true;
}

//rwa function ChangeIntTextbox(textboxID, checkboxID)
//rwa {
//rwa   // Filter input for just integers
//rwa   forceInteger(textboxID);
//rwa 
//rwa   // Set associated checkbox
//rwa   SetCheckbox(checkboxID);
//rwa }

function CheckboxChanged(checkID)
{
  switch(checkID)
  {
    case "CellWidthCheckbox":
      gCellChangeData.size.width = true;
      gCollatedCellData.size.bWidthSet = gDialog.CellWidthCheckbox.checked;
    break;
    case "CellHeightCheckbox":
      gCellChangeData.size.height = true;
      gCollatedCellData.size.bHeightSet = gDialog.CellWidthCheckbox.checked;
    break;
    case "TextWrapCheckbox":
      gCellChangeData.wrap = true;
      gCollatedCellData.wrap = (gDialog.TextWrapCheckbox.checked ? "wrap" : "nowrap");
    break;
  }
}

function ChangeCellSize(textID)
{
  switch(textID)
  {
    case "CellWidthInput":
      gCellChangeData.size.width = true;
      gCollatedCellData.size.width = gWidthUnitsController.getValue(textID);
    break;
    case "CellHeightInput":
      gCellChangeData.size.height = true;
      gCollatedCellData.size.height = gHeightUnitsController.getValue(textID);
    break;
  }
}

function AlignmentChanged(radioGroupID)
{
  switch(radioGroupID)
  {
    case "ColumnAlignRadioGroup":
      gCellChangeData.align.halign = true;
      gCollatedCellData.align.halign = gDialog.ColAlignRadioGroup.value;
    break;
    case "RowAlignRadioGroup":
      gCellChangeData.align.valign = true;
      gCollatedCellData.align.valign = gDialog.RowAlignRadioGroup.value;
    break;
  }
}

function TablePropertyChanged(controlID)
{
  switch(controlID)
  {
    case "TableCaptionList":
      gTableChangeData.caption = true;
      gTableCaptionPlacement = gDialog.TableCaptionList.value;
    break;

    case "TableBaselineRadioGroup":
      gTableChangeData.baseline = true;
      gTableBaseline = gDialog.TableBaselineRadioGroup.value;
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

function EnableDisableControls()
{
  var bIsWholeCols = false;
  var bIsWholeRows = false;
  switch(gSelectionTypeStr)
  {
    case "Table":
      bIsWholeRows = true;
    case "Col":
      bIsWholeCols = true;
    break;
    case "Row":
      bIsWholeRows = true;
    break;
    case "Cell":
    case "CellGroup":
    default:
    break;
  }

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

    if (!bIsWholeCols)
    {
      DisableRadioGroup(gDialog.ColAlignRadioGroup);
      gDialog.CellWidthInput.disabled = true;
      gDialog.CellWidthUnits.disabled = true;
      gDialog.CellWidthCheckbox.disabled = true;
    }

    if (!bIsWholeRows)
    {
      DisableRadioGroup(gDialog.RowAlignRadioGroup);
      gDialog.CellHeightInput.disabled = true;
      gDialog.CellHeightUnits.disabled = true;
      gDialog.CellHeightCheckbox.disabled = true;
    }
  }
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

function SetAnAttribute(destElement, attr, attrValue)
{
  // Use editor methods since we are always
  //  modifying a table in the document and
  //  we need transaction system for undo
  try {
    if (!attrValue || attrValue.length == 0)
      gActiveEditor.removeAttributeOrEquivalent(destElement, attr, false);
    else
      gActiveEditor.setAttributeOrEquivalent(destElement, attr, attrValue, false);
  } catch(e) {}
}

function DoStyleChangesForACell(destCell)
{
  globalCellElement = destCell.cloneNode(false);
  globalCellElement.setAttribute("style", destElement.getAttribute("style"));
  var borderProps = ["style", "width", "color"];
  for (var nProp = 0; nProp < borderProps.length; ++nProp)
  {
    theProp = borderProps[nProp];
    if (UseCSSForCellProp("border-" + theProp))
    {
      for (var ix = 0; ix < gCellChangeData.border[theProp].length; ++ix)
      {
        theSide = gCellChangeData.border[theProp][ix];
        globalCellElement.style.setProperty( getBorderSideAttrString(theSide, theProp), gCollatedCellData.border[theProp][theSide]);
      }
    }
  }
  if (gCellChangeData.size.width)
  {
    if (gCollatedCellData.size.bWidthSet && UseCSSForCellProp("width"))
      globalCellElement.style.setProperty("width", String(gCollatedCellData.size.width) + gCellWidthUnit);
    else
      globalCellElement.style.removeProperty("width");
  }
  if (gCellChangeData.size.height)
  {
    if (gCollatedCellData.size.bHeightSet && UseCSSForCellProp("height"))
      globalCellElement.style.setProperty("height", String(gCollatedCellData.size.height) + gCellHeightUnit);
    else
      globalCellElement.style.removeProperty("height");
  }

  if (gCellChangeData.align.halign && UseCSSForCellProp("halign"))
    globalCellElement.style.setProperty("text-align", gCollatedCellData.align.halign);
  if (gCellChangeData.align.valign && UseCSSForCellProp("valign"))
    globalCellElement.style.setProperty("vertical-align", gCollatedCellData.align.valign);
  if (gCellChangeData.wrap && UseCSSForCellProp("wrap"))
  {
    if (gCollatedCellData.wrap == "nowrap")
      globalCellElement.style.setProperty("white-space", "nowrap?");
    else
    {
      var currWhiteSpace = globalCellElement.style.getProperty("white-space");
      if (currWhiteSpace && currWhiteSpace == "nowrap")
        globalCellElement.style.removeProperty("white-space");
    }
  }
  if (gCellChangeData.background)
  {
    if (gCollatedCellData.background == "transparent")
      globalCellElement.style.removeProperty("background-color");
    else
      globalCellElement.style.setProperty("background-color", gCollatedCellData.background);
  }
  var theStyle = globalCellElement.getAttribute("style");
  gActiveEditor.setAttribute(destCell, "style", theStyle);
}

function ApplyTableAttributes()
{
  var newAlign = gTableCaptionPlacement;
  if (!newAlign)
    newAlign = "";

  if (gTableCaptionElement)
  {
    // Get current alignment
    var align = GetHTMLOrCSSStyleValue(gTableCaptionElement, "align", "caption-side").toLowerCase();
    // This is the default
    if (!align) align = "top";

    if (newAlign == "")
    {
      // Remove existing caption
      try {
        gActiveEditor.deleteNode(gTableCaptionElement);
      } catch(e) {}
      gTableCaptionElement = null;
    }
    else if(newAlign != align)
    {
      try {
        if (newAlign == "top") // This is default, so don't explicitly set it
          gActiveEditor.removeAttributeOrEquivalent(gTableCaptionElement, "align", false);
        else
          gActiveEditor.setAttributeOrEquivalent(gTableCaptionElement, "align", newAlign, false);
      } catch(e) {}
    }
  }
  else if (newAlign != "")
  {
    // Create and insert a caption:
    try {
      gTableCaptionElement = gActiveEditor.createElementWithDefaults("caption");
    } catch (e) {}
    if (gTableCaptionElement)
    {
      if (newAlign != "top")
        gTableCaptionElement.setAttribute("align", newAlign);

      // Insert it into the table - caption is always inserted as first child
      try {
        gActiveEditor.insertNode(gTableCaptionElement, gTableElement, 0);
      } catch(e) {}

      // Put selecton back where it was
//      ChangeSelection(RESET_SELECTION);
    }
  }

  if (gTableChangeData.baseline)
    globalTableElement.style.setProperty("vertical-align", gTableBaseline);
  if (gTableChangeData.background)
    globalTableElement.style.setProperty("background-color", gTableColor);
//  if (gTableChangeData.size.width)
//    globalTableElement.style.setProperty("width", widthStr);
//  if (gTableChangeData.size.height)
//    globalTableElement.style.setProperty("height", heightStr);
  var theStyleString = globalTableElement.getAttribute("style");
  gActiveEditor.setAttribute(gTableElement, "style", theStyleString);

//rwa  var countDelta;
//rwa  var foundCell;
//rwa  var i;
//rwa
//rwa  if (gNewRowCount != gRowCount)
//rwa  {
//rwa    countDelta = gNewRowCount - gRowCount;
//rwa    if (gNewRowCount > gRowCount)
//rwa    {
//rwa      // Append new rows
//rwa      // Find first cell in last row
//rwa      if(GetCellData(gLastRowIndex, 0))
//rwa      {
//rwa        try {
//rwa          // Move selection to the last cell
//rwa          gSelection.collapse(gCellData.value,0);
//rwa          // Insert new rows after it
//rwa          gActiveEditor.insertTableRow(countDelta, true);
//rwa          gRowCount = gNewRowCount;
//rwa          gLastRowIndex = gRowCount - 1;
//rwa          // Put selecton back where it was
//rwa          ChangeSelection(RESET_SELECTION);
//rwa        }
//rwa        catch(ex) {
//rwa          dump("FAILED TO FIND FIRST CELL IN LAST ROW\n");
//rwa        }
//rwa      }
//rwa    }
//rwa    else
//rwa    {
//rwa      // Delete rows
//rwa      if (gCanDelete)
//rwa      {
//rwa        // Find first cell starting in first row we delete
//rwa        var firstDeleteRow = gRowCount + countDelta;
//rwa        foundCell = false;
//rwa        for ( i = 0; i <= gLastColIndex; i++)
//rwa        {
//rwa          if (!GetCellData(firstDeleteRow, i))
//rwa            break; // We failed to find a cell
//rwa
//rwa          if (gCellData.startRowIndex == firstDeleteRow)
//rwa          {
//rwa            foundCell = true;
//rwa            break;
//rwa          }
//rwa        };
//rwa        if (foundCell)
//rwa        {
//rwa          try {
//rwa            // Move selection to the cell we found
//rwa            gSelection.collapse(gCellData.value, 0);
//rwa            gActiveEditor.deleteTableRow(-countDelta);
//rwa            gRowCount = gNewRowCount;
//rwa            gLastRowIndex = gRowCount - 1;
//rwa            if (gCurRowIndex > gLastRowIndex)
//rwa              // We are deleting our selection
//rwa              // move it to start of table
//rwa              ChangeSelectionToFirstCell()
//rwa            else
//rwa              // Put selecton back where it was
//rwa              ChangeSelection(RESET_SELECTION);
//rwa          }
//rwa          catch(ex) {
//rwa            dump("FAILED TO FIND FIRST CELL IN LAST ROW\n");
//rwa          }
//rwa        }
//rwa      }
//rwa    }
//rwa  }
//rwa
//rwa  if (gNewColCount != gColCount)
//rwa  {
//rwa    countDelta = gNewColCount - gColCount;
//rwa
//rwa    if (gNewColCount > gColCount)
//rwa    {
//rwa      // Append new columns
//rwa      // Find last cell in first column
//rwa      if(GetCellData(0, gLastColIndex))
//rwa      {
//rwa        try {
//rwa          // Move selection to the last cell
//rwa          gSelection.collapse(gCellData.value,0);
//rwa          gActiveEditor.insertTableColumn(countDelta, true);
//rwa          gColCount = gNewColCount;
//rwa          gLastColIndex = gColCount-1;
//rwa          // Restore selection
//rwa          ChangeSelection(RESET_SELECTION);
//rwa        }
//rwa        catch(ex) {
//rwa          dump("FAILED TO FIND FIRST CELL IN LAST COLUMN\n");
//rwa        }
//rwa      }
//rwa    }
//rwa    else
//rwa    {
//rwa      // Delete columns
//rwa      if (gCanDelete)
//rwa      {
//rwa        var firstDeleteCol = gColCount + countDelta;
//rwa        foundCell = false;
//rwa        for ( i = 0; i <= gLastRowIndex; i++)
//rwa        {
//rwa          // Find first cell starting in first column we delete
//rwa          if (!GetCellData(i, firstDeleteCol))
//rwa            break; // We failed to find a cell
//rwa
//rwa          if (gCellData.startColIndex == firstDeleteCol)
//rwa          {
//rwa            foundCell = true;
//rwa            break;
//rwa          }
//rwa        };
//rwa        if (foundCell)
//rwa        {
//rwa          try {
//rwa            // Move selection to the cell we found
//rwa            gSelection.collapse(gCellData.value, 0);
//rwa            gActiveEditor.deleteTableColumn(-countDelta);
//rwa            gColCount = gNewColCount;
//rwa            gLastColIndex = gColCount-1;
//rwa            if (gCurColIndex > gLastColIndex)
//rwa              ChangeSelectionToFirstCell()
//rwa            else
//rwa              ChangeSelection(RESET_SELECTION);
//rwa          }
//rwa          catch(ex) {
//rwa            dump("FAILED TO FIND FIRST CELL IN LAST ROW\n");
//rwa          }
//rwa        }
//rwa      }
//rwa    }
//rwa  }

  // Clone all remaining attributes to pick up
  //  anything changed by Advanced Edit Dialog
  try {
    gActiveEditor.cloneAttributes(gTableElement, globalTableElement);
  } catch(e) {}
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

function ApplyColAndRowAttributes()
{
  if (gIsMatrix)
    ApplyMatrixColAndRowAttributes();
}

function ApplyMatrixColAndRowAttributes()
{
  var whiteSpace = /(^\s+)/;
  var defColWidth = "auto";
  var colWidths = [];
  var colWidthsStr = gTableElement.getAttribute("columnwidth");
  if (colWidthsStr)
    colWidths = colWidthsStr.split(whiteSpace);
  var tableDims = data.reviseData.getTableDims();
  var bAllAuto = false;
  if (colWidhts.length > 0)
    defColWidth = colWidths[colWidths.length - 1];
  for (var nCol = colWidths.length; nCol < tableDims.nCols; ++nCol)
    colWidths[nCol] = defColWidth;

  var colsInSelection = data.reviseData.getColsInSelection();
  var theWidth = "auto";
  if (gCollatedCellData.size.bWidthSet)
    theWidth = String(gCollatedCellData.size.width) + gCellWidthUnit;
  for (var ix = 0; ix < colsInSelection.length; ++ix)
    colWidths[colsInSelection[ix]] = theWidth;

  colWidthsStr = colWidths[0];
  bAllAuto = (colWidthsStr == "auto");
  for (nCol = 1; nCol < tableDims.nCols; ++nCol)
  {
    bAllAuto == bAllAuto && (colWidths[nCol] == "auto");
    colWidthsStr += " " + colWidths[nCol];
  }
  if (bAllAuto)
    gActiveEditor.removeAttribute(gTableElement, "columnwidth");
  else
    gActiveEditor.setAttribute(gTableElement, "columnwidth", colWidthsStr);

  //If there's a setting for height, it would have to be implemented via CSS(?)

  ApplyMatrixColAndRowLines();
  ApplyMatrixAlignment();
}

function ApplyMatrixColAndRowLines()
{
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

  var colsInSelection = data.reviseData.getColsInSelection();
  var rowsInSelection = data.reviseData.getRowsInSelection();
  var rowOffset = 0;
  var colOffset = 0;
  var bDoRows = false;
  var bDoCols = false;

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
    for (var ix = 1; ix < matrixRowLines.length; ++ix)
      matrixRowLinesStr += " " + matrixRowLines[ix];
    gActiveEditor.setAttribute(gTableElement, "rowlines", matrixRowLinesStr);
  }
  if (bDoCols)
  {
    matrixColLinesStr = matrixColLines[0];
    for (var ix = 1; ix < matrixColLines.length; ++ix)
      matrixColLinesStr += " " + matrixColLines[ix];
    gActiveEditor.setAttribute(gTableElement, "columnlines", matrixColLinesStr);
  }
}

function ApplyMatrixAlignment()
{
  var bDoHAlign = gCellChangeData.align.halign;
  var bDoVAlign = gCellChangeData.align.valign;
  if (!bDoHAlign && !bDoVAlign)
    return;

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

  var colsInSelection = data.reviseData.getColsInSelection();
  var rowsInSelection = data.reviseData.getRowsInSelection();

  if (bDoHAlign)
  {
    for (var jx = 0; jx < colsInSelection.length; ++jx)
      matrixHAlignVals[colsInSelection[jx]] = gCollatedCellData.align.halign;
    matrixHAlignStr = matrixHAlignVals[0];
    for (jx = 1; jx < matrixHAlignVals.length; ++jx)
      matrixHAlignStr += " " + matrixHAlignVals[jx];
    gActiveEditor.setAttribute(gTableElement, "columnalign", matrixHAlignStr);
  }
  if (bDoVAlign)
  {
    for (var ix = 0; ix < rowsInSelection.length; ++ix)
      matrixVAlignVals[rowsInSelection[ix]] = gCollatedCellData.align.valign;
    matrixVAlignStr = matrixVAlignVals[0];
    for (ix = 1; ix < matrixVAlignVals.length; ++ix)
      matrixVAlignStr += " " + matrixVAlignVals[ix];
    gActiveEditor.setAttribute(gTableElement, "rowalign", matrixVAlignStr);
  }
}

function ApplyCellAttributes()
{
  var cellIter = data.reviseData.beginSelectedCellIteration(gSelectionTypeStr);
  var currCell = data.reviseData.getNextSelectedCell(cellIter);
  while (currCell)
  {
    nRow = cellIter.nRow;
    nCol = cellIter.nCol;
    ApplyAttributesToOneCell(currCell, nRow, nCol);
    currCell = data.reviseData.getNextSelectedCell(cellIter);
  }
}

function ApplyAttributesToOneCell(destElement, nRow, nCol)
{
  var theValStr = "";
  if (gCellChangeData.size.height)
  {
    if (gCollatedCellData.size.bHeightSet && !UseCSSForCellProp("height") && !ShouldSetHeightOnRows())
      theValStr = String(gCollatedCellData.size.height) + gCellHeightUnit;
    SetAnAttribute(destElement, "height", theValStr);
  }

  if (gCellChangeData.size.width)
  {
    if (gCollatedCellData.size.bWidthSet && !UseCSSForCellProp("width") && !ShouldSetWidthOnCols())
      theValStr = String(gCollatedCellData.size.width) + gCellWidthUnit;
    SetAnAttribute(destElement, "width", theValStr);
  }

  DoStyleChangesForACell(destElement);

//  if (gCellChangeData.align.halign && UseCSSForCellProp("halign"))
//    globalCellElement.style.setProperty("text-align", gCollatedCellData.align.halign);
//  if (gCellChangeData.align.valign && UseCSSForCellProp("valign"))
//    globalCellElement.style.setProperty("vertical-align", gCollatedCellData.align.valign);

//  if (gCellChangeData.wrap))
//  {
//    if (!UseCSSForCellProp("wrap") && gCollatedCellData.wrap == "nowrap")
//      SetAnAttribute(destElement, "nowrap", "true");
//    else
//      SetAnAttribute(destElement, "nowrap", null);
//  }

//  if (gDialog.CellHAlignCheckbox.checked)
////  {
//    SetAnAttribute(destElement, "align", );
////    CloneAttribute(destElement, globalCellElement, charStr);
////  }
//
//  if (gDialog.CellVAlignCheckbox.checked)
//    SetAnAttribute(destElement, "valign", );
//
//  if (gDialog.TextWrapCheckbox.checked)
//    SetAnAttribute(destElement, "nowrap", );

//  if (gDialog.CellStyleCheckbox.checked)
//  {
//    var newStyleIndex = gDialog.CellStyleList.selectedIndex;
//    var currentStyleIndex = (destElement.nodeName.toLowerCase() == "th") ? 1 : 0;
//
//    if (newStyleIndex != currentStyleIndex)
//    {
//      // Switch cell types
//      // (replaces with new cell and copies attributes and contents)
//      try {
//        destElement = gActiveEditor.switchTableCellHeaderType(destElement);
//      } catch(e) {}
//    }
//  }

//  if (gDialog.CellColorCheckbox.checked)
//    CloneAttribute(destElement, globalCellElement, "bgcolor");
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

    ApplyColAndRowAttributes();

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

function onAccept()
{
  // Do same as Apply and close window if ValidateData succeeded
  var retVal = Apply();
  if (retVal)
    SaveWindowLocation();

  return retVal;
}

function createPreviewChangeArray()
{
  var theChanges = { obj : {}, style : {}, table : {}, tableStyle : {} };
  return theChanges;
}

function checkPreviewChanges(controlID)
{
  var theChanges = createPreviewChangeArray();
  var sideString = (gCurrentSide == "all") ? "" : gCurrentSide + "-";
  var bBackgroundIsSelection = (gDialog.BackgroundSelectionRadioGroup.value == "selection");
  switch(controlID)
  {
    case "CellBorderStyleList":
      if (gCollatedCellData.borderData.style[gCurrentSide] != gDialog.CellBorderStyleList.value)
      {
        gCollatedCellData.borderData.style[gCurrentSide] = gDialog.CellBorderStyleList.value;
        bChanged = true;
        theChanges.style["border-" + sideString + "style"] = gDialog.CellBorderStyleList.value;
        gCellChangeData.border.style.push(gCurrentSide);
      }
    break;
    case "CellBorderWidthList":
      if (gCollatedCellData.borderData.width[gCurrentSide] != gDialog.CellBorderWidthList.value)
      {
        gCollatedCellData.borderData.width[gCurrentSide] = gDialog.CellBorderWidthList.value;
        bChanged = true;
        theChanges.style["border-" + sideString + "width"] = gDialog.CellBorderWidthList.value;
        gCellChangeData.border.width.push(gCurrentSide);
      }
    break;

    default:
    break;
  }
  if (bChanged)
    updateSample(theChanges);
}

//Aren't all of these handled using style changes?
function updateSample(newValues)
{
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
    gDialog.BordersPreviewCenterCell.style.setProperty(styleChange, newValue);
  }
  for (var objChange in newValues.obj)
    gDialog.BordersPreviewCenterCell.setAttribute(objChange, newValues.obj[objChange]);
  for (var tableStyleChange in newValues.tableStyle)
    gDialog.BordersPreview.style.setProperty(tableStyleChange, newValues.tableStyle[tableStyleChange]);
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
  }
  return collateCellData(gOurCellData, reviseData);
}

function collateCellData(ourCellData, reviseData)
{
  var retCellData = new Object();
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
  var sortArray = new Array();
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
        for (whichSide in borderNames)
        {
          theStyle = styleData[whichSide];
          if (theStyle && (theStyle in styleCount[whichSide]))
            ++styleCount[whichSide][theStyle];
        }
      }
      var widthData = ourCellData[nRow][nCol].border.width;
      if (widthData)
      {
        for (whichSide in borderNames)
        {
          theWidth = interpretCSSBorderWidth(widthData[whichSide], ourCellData[nRow][nCol]);
          if (theWidth && (theWidth in widthCount[whichSide]))
            ++widthCount[whichSide][theWidth];
        }
      }
      var colorData = ourCellData[nRow][nCol].border.color;
      if (colorData)
      {
        for (whichSide in borderNames)
        {
          theColor = colorData[whichSide];
          if (theColor)
          {
            if (theColor in colorCount[whichSide])
              ++colorCount[nWhichSide][theColor];
            else
              colorCount[nWhichSide][theColor] = 1;
          }
        }
      }
    }
    currCell = reviseData.getNextSelectedCell(cellIter);
  }
  var borderData = createCellBorderData();
  for (var aSide in borderData.style)
  {
    borderData.style[aSide] = findPlurality( styleCount[aSide] );
    borderData.width[aSide] = findPlurality( widthCount[aSide] );
    borderData.color[aSide] = findPlurality( colorCount[aSide] );
  }
  return borderData;
}

//Need to adjust the following to deal with multi-row or multi-col cells!
function collateCellSizeData(ourCellData, reviseData)
{
  var rowHeights = new Array(ourCellData.length);
  for (var ix = 0; ix < rowHeights.length; ++ix)
    rowHeights[ix] = 0.0;
  var colWidths = new Array(ourCellData[0].length);
  for (var jx = 0; jx < colWidths.length; ++jx)
    colWidths[jx] = 0.0;
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
  var widthUnitsCount = { pt : 0, in : 0, mm : 0, cm : 0, pc : 0, em : 0, ex : 0, px : 0 };
  var heightUnitsCount = { pt : 0, in : 0, mm : 0, cm : 0, pc : 0, em : 0, ex : 0, px : 0 };
  var widthAutoCount = 0;
  var heightAutoCount = 0;
  var fontSizeCount = new Object();

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
    }
    else
      ++widthAutoCount;
    if (theCellData.size.height != null)
    {
      if (!heightUnitFound)
        heightUnitFound = theCellData.size.width.unit;
      if (theCellData.size.height.unit in heightUnitsCount)
        ++heightUnitsCount[theCellData.size.height.unit];
      theHeight = interpretCSSSizeValue(theCellData.size.height.number, theCellData.size.height.unit, heightUnitFound, theCellData);
    }
    else
      ++heightAutoCount;
    if (rowHeights[nRow] < theHeight)
      rowHeights[nRow] = theHeight;
    if (colWidths[nCol] < theWidth)
      colWidths[nCol] = theWidth;
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
  theWidth = findMedian(colWidths, 0.0);
  theHeight = findMedian(rowHeights, 0.0);
  gCellWidthUnit = findPlurality(widthUnitsCount);
  gCellHeightUnit = findPlurality(heightUnitsCount);
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
  var retAlign = new Object();
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
  var backColorCount = {black : 0};
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
      case "transparent":
      case "inherit":
        //do nothing
      break;
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
      ourCellData[nRow][nCol][aDatum].top = null;
    if (bNoRight)
      ourCellData[nRow][nCol][aDatum].right = null;
    if (bNoBottom)
      ourCellData[nRow][nCol][aDatum].bottom = null;
    if (bNoLeft)
      ourCellData[nRow][nCol][aDatum].left = null;
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
  
  cellData.border = getBorderDataForCell(aCell, nRow, nCol, elementUnitsList);
  cellData.size = getSizeDataForCell(aCell, nRow, nCol, elementUnitsList);

  if (gIsMatrix)
    cellData.align = getAlignDataForMatrixCell(aCell, nRow, nCol);
  else
  {
    cellData.align = { halign : "", valign : ""};
    aValue = computedStyle.getPropertyCSSValue("vertical-align");
    if (aValue)
      cellData.align.valign = aValue;
    aValue = computedStyle.getPropertyCSSValue("text-align");
    if (aValue)
      cellData.align.halign = aValue;
  }
  
  aValue = computedStyle.getPropertyCSSValue("white-space");
  if (aValue && aValue == "nowrap")
    cellData.wrap = "nowrap";
  else if (aCell.getAttributeValue("nowrap") == "true")
    cellData.wrap = "nowrap";
  
  aValue = computedStyle.getPropertyCSSValue("background-color");
  if (aValue)
    cellData.background = ConvertRGBColorIntoHEXColor(aValue);
  
  return cellData;
}

function getBorderDataForCell(aCell, nRow, nCol, elementUnitsList)
{
  var borderData = null;
  if (gIsMatrix)
    borderData = getBorderDataForMatrixCell(aCell, nRow, ncol, elementUnitsList);
  else
    borderData = createCellBorderData();
  var aValue, attrName;
  if (!gIsMatrix)
  {
    for (attrName in borderData.style)
    {
      aValue = computedStyle.getPropertyCSSValue("border-" + attrName + "-style");
      if (aValue)
        borderData.style[attrName] = aValue;
    }
  }
  for (attrName in borderData.color)
  {
    aValue = computedStyle.getPropertyCSSValue("border-" + attrName + "-color");
    if (aValue)
      borderData.color[attrName] = ConvertRGBColorIntoHEXColor(aValue);
  }
  for (attrName in borderData.width)
  {
    aValue = computedStyle.getPropertyCSSValue("border-" + attrName + "-width");
    if (aValue)
      borderData.width[attrName] = aValue;
  }
  return borderData;
}

function getBorderDataForMatrixCell(aCell, nRow, nCol, elementUnitsList)
{
  var whiteSpace = /(^\s+)/;
  var borderData = createCellBorderData();
  var matixColLines, matrixRowLines;
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
    return getSizeDataForMatrixCell(aCell, nRow, ncol);

  var sizeData = { width : null, height : null };
  var theValue = msiGetHTMLOrCSSStyleValue(gActiveEditorElement, gTableCaptionElement, "width", "width");
  if (theValue)
    sizeData.width = msiGetNumberAndLengthUnitFromString(theValue);
  theValue = msiGetHTMLOrCSSStyleValue(gActiveEditorElement, gTableCaptionElement, "height", "height");
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
