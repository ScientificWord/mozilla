/* 
 * The contents of this file are subject to the Netscape Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/NPL/
 *  
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *  
 * The Original Code is Mozilla Communicator client code, released
 * March 31, 1998.
 * 
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation. Portions created by Netscape are
 * Copyright (C) 1998-1999 Netscape Communications Corporation. All
 * Rights Reserved.
 * 
 * Contributor(s): 
 */
"use strict";

Components.utils.import("resource://app/modules/unitHandler.jsm");
//Cancel() is in EdDialogCommon.js
var gTableElement = null;
var gRows;
var gColumns;
var gActiveEditor;
var gCellID = 12;
var gPrefs;
var gDialog;
var globalElement;
var unitHandler;
var gOurCellData;
var gInitialCellData;
var gCollatedCellData;  //This starts out as the same as the previous, but changes as the user makes selections in the dialog.
var gColElementArray;
var gRowElementArray;

var gTableColor;
var gTableCaptionPlacement;
var gTableBaseline = "baseline";
var gBorderCollapse = "collapse";  //This should be the default???

var gCollatedCellData;
var gCellChangeData;
var gTableChangeData;
var gCurrentSide = "";
var gBorderSides = ["top", "right", "bottom", "left"];
var gSelectionTypeStr = "Cell";
var gApplyUsed = false;
var gIsMatrix = false; //We have to set this in the initialization code.
var data = null;

function ChangeCellSize(textID)
{
  // switch(textID)
  // {
  //   case "CellWidthInput":
  //     gCellChangeData.size.width = true;
  //     gCellWidthUnit = gDialog.cellUnitsList.value;
  //     gCollatedCellData.size.width = cellUnitsHandler.getValueOf(gDialog.CellWidthInput.value, gCellWidthUnit);
  //   break;
  //   case "CellHeightInput":
  //     gCellChangeData.size.height = true;
  //     gCellHeightUnit = gDialog.cellUnitsList.value;
  //     gCollatedCellData.size.height = cellUnitsHandler.getValueOf(gDialog.CellHeightInput.value, gCellHeightUnit);
  //   break;
  // }
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

function enableControlsByID(id)
{

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
  var bDisabled = gDialog.tableWidthAutoCheckbox.checked;
  enableControlsByID(["tableWidth"], !bDisabled);
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
//     if (!bIsWholeCols)
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
}

// dialog initialization code
function Startup()
{
  var defaultUnit;
  var hAlign ;  
  var vAlign;   
  var wrapping;
  gPrefs = GetPrefs();
  gActiveEditor = msiGetTableEditor();
  if (!gActiveEditor)
  {
    dump("Failed to get active table editor!\n");
    window.close();
    return;
  }

  try {
    gTableElement = gActiveEditor.createElementWithDefaults("table");
    gTableElement.setAttribute("req","tabulary");
  } catch (e) {}

  if(!gTableElement)
  {
    dump("Failed to create a new table!\n");
    window.close();
    return;
  }
  unitHandler = new UnitHandler();
  gDialog.rowsInput      = document.getElementById("rowsInput");
  gDialog.columnsInput   = document.getElementById("columnsInput");
  gDialog.widthInput     = document.getElementById("widthInput");
  gDialog.autoCheckbox   = document.getElementById("autoWidthCheckbox");
  gDialog.CellHeightInput= document.getElementById("CellHeightInput");
  gDialog.CellWidthInput = document.getElementById("CellWidthInput");
  gDialog.currentunits   = document.getElementById("currentunits");
  gDialog.unitMenulist   = document.getElementById("unitMenulist");
  gDialog.wrapping       = document.getElementById("TextWrapCheckbox");
  gDialog.vAlignChoices  = document.getElementById("vAlignChoices");
  gDialog.hAlignChoices  = document.getElementById("hAlignChoices");
  gDialog.BordersPreviewCenterCell = document.getElementById("BordersPreviewCenterCell");
  gDialog.BorderSideSelectionList = document.getElementById("BorderSideSelectionList");
  gDialog.CellBorderStyleList = document.getElementById("cellBorderStyleList");
  gDialog.CellBorderWidthList = document.getElementById("cellBorderWidthList");
  gDialog.borderCW = document.getElementById("borderCW");

  gDialog.OkButton = document.documentElement.getButton("accept");
  gDialog.sizeLabel = document.getElementById("sizeLabel");
  gRows = gDialog.rowsInput.value;
  gColumns = gDialog.columnsInput.value;

  var fieldList = [];
  fieldList.push(gDialog.CellHeightInput);
  fieldList.push(gDialog.CellWidthInput);
  fieldList.push(gDialog.widthInput);
  unitHandler.setEditFieldList(fieldList);
  if (gPrefs)
    defaultUnit = gPrefs.getCharPref("swp.defaultTableUnits");
  gDialog.currentunits.setAttribute("value", unitHandler.getDisplayString(defaultUnit));
  try {
    unitHandler.buildUnitMenu(gDialog.unitMenulist, defaultUnit);

  // need to check this and use default value from preferences if appropriate.
    unitHandler.initCurrentUnit(gDialog.unitMenulist.value, defaultUnit);
  }
  catch(e) {
    dump(e.message);
  }
  //setVariablesForControls();
//temp hack
      setDataFromReviseData(data.reviseData, data.reviseCommand);

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


  var cellid;
  if (gRows <= 6 && gColumns <= 6) cellid = 10*(Number(gRows) -1 )+Number(gColumns);
  else {
    cellid = 56;
    // Table is too big for the 'quickly' tab; activate the 'precisely' tab.
    document.getElementById("tabpanel-tabs").selectedIndex = 1;
  }

//  gTableElement.setAttribute("border", gDialog.borderInput.value);
  // if (gDialog.widthInput.value)
  //   gTableElement.setAttribute("width", Number(gDialog.widthInput.value) +
  //                                       (gDialog.widthPixelOrPercentMenulist.value == "pc" ? "%" : ""));
  
  // Make a copy to use for AdvancedEdit
  globalElement = gTableElement.cloneNode(false);
  try {
    if (IsHTMLEditor()
        && !(gActiveEditor.flags & Components.interfaces.nsIPlaintextEditor.eEditorMailMask))
    {
      if (gPrefs.getBoolPref("editor.use_css"))
      {
        // only for Composer and not for htmlmail
        globalElement.setAttribute("style", "text-align: left;");
      }

      hAlign = gPrefs.getCharPref("editor.table.default_align");
      vAlign = gPrefs.getCharPref("editor.table.default_valign");
      wrapping = gPrefs.getCharPref("editor.table.default_wrapping");

      var cellSpacing = gPrefs.getCharPref("editor.table.default_cellspacing");
      if (cellSpacing)
      {
        // only for Composer and not for htmlmail
        globalElement.setAttribute("cellspacing", cellSpacing);
      }

      var cellPadding= gPrefs.getCharPref("editor.table.default_cellpadding");
      if (cellPadding)
      {
        // only for Composer and not for htmlmail
        globalElement.setAttribute("cellpadding", cellPadding);
      }
    }
  } catch (e) {}

  // Initialize all widgets with image attributes
  SelectArea("c"+cellid);
  InitDialog(hAlign, vAlign, wrapping);

  SetTextboxFocusById("rowsInput");

  SetWindowLocation();
}

// Set dialog widgets with attribute data
// We get them from globalElement copy so this can be used
//   by AdvancedEdit(), which is shared by all property dialogs
function InitDialog(hAlign, vAlign, wrapping)
{  
  // Get default attributes set on the created table:
  // Get the width attribute of the element, stripping out "%"
  // This sets contents of menu combobox list
  // 2nd param = null: Use current selection to find if parent is table cell or window
  /*gDialog.widthInput.value =*/ 
  msiInitPixelOrPercentMenulist(globalElement, null, "width", "widthPixelOrPercentMenulist", gPercent);
  /*gDialog.borderInput.value = globalElement.getAttribute("border");*/

  gDialog.hAlignChoices.value = hAlign || "";
  gDialog.vAlignChoices.value  = vAlign || "";
  gDialog.wrapping.checked = wrapping;
  checkEnableWidthControls();
}

function onChangeTableUnits()
{
  var val = document.getElementById("unitMenulist").value;
  unitHandler.setCurrentUnit(val);
  document.getElementById("currentunits").setAttribute("value", unitHandler.getDisplayString(val));
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

function GetColorAndUpdate(ColorWellID)
{
  var colorWell = document.getElementById(ColorWellID);
  if (!colorWell) return;
  var colorObj= {
    TextColor: "#ffffff",
    alpha: 255,
    Cancel: false
  };
  if (ColorWellID === "borderCW") {
    colorObj.TextColor = "#000000";
  }


  window.openDialog("chrome://editor/content/EdColorPicker.xul", "colorpicker", "chrome,close,titlebar,modal,resizable", "", colorObj);

  // User canceled the dialog
  if (colorObj.Cancel)
    return;
  

  // var changeArray = createPreviewChangeArray();
  // var borderSideAttrStr = "";
  var theColor = colorObj.TextColor;
  setColorWell(ColorWellID, theColor);
}


function checkEnableWidthControls()
{
  var bIsAuto = gDialog.autoCheckbox.checked;
  enableControlsByID(["widthInput","widthPixelOrPercentMenulist"], !bIsAuto);
}

// Get and validate data from widgets.
// Set attributes on globalElement so they can be accessed by AdvancedEdit()
function ValidateData()
{
  gRows = msiValidateNumber(gDialog.rowsInput, null, 1, gMaxRows, null, null, true);
  if (gValidationError)
    return false;

  gColumns = msiValidateNumber(gDialog.columnsInput, null, 1, gMaxColumns, null, null, true)
  if (gValidationError)
    return false;

  // Set attributes: NOTE: These may be empty strings (last param = false)
  // msiValidateNumber(gDialog.borderInput, null, 0, gMaxPixels, globalElement, "border", false);
  // TODO: Deal with "BORDER" without value issue
  if (gValidationError) return false;

  if (gDialog.autoCheckbox.checked)
  {
    gActiveEditor.removeAttributeOrEquivalent(globalElement, "width", true);
  }
  else
  {
    msiValidateNumber(gDialog.widthInput, gDialog.widthPixelOrPercentMenulist,
                       1, gMaxTableSize, globalElement, "width", false);
    gDialog.widthPixelOrPercentMenulist.value = gDialog.widthPixelOrPercentMenulist.selectedItem.value;
  }

  // SetOrResetAttribute(globalElement, "cellspacing", gDialog.cellSpacing.value);
  // SetOrResetAttribute(globalElement, "cellpadding", gDialog.cellPadding.value);

  if (gValidationError)
    return false;

  return true;
}

function SetOrResetAttribute(element, attributeName, attributeValue)
{
  if (attributeValue)
    element.setAttribute(attributeName, attributeValue);
  else
    element.removeAttribute(attributeName);
}

function onAccept()
{
  if (ValidateData())
  {
    try 
    {
      var wrapping = gDialog.wrapping.checked;
      gPrefs.setCharPref("editor.table.default_wrapping", wrapping);

      var align = gDialog.hAlignChoices.value;
      gPrefs.setCharPref("editor.table.default_align", align);

      var valign = gDialog.vAlignChoices.value;
      gPrefs.setCharPref("editor.table.default_valign", valign);

      // var cellSpacing = globalElement.getAttribute("cellspacing");
      // gPrefs.setCharPref("editor.table.default_cellspacing", cellSpacing);

      // var cellPadding = globalElement.getAttribute("cellpadding");
      // gPrefs.setCharPref("editor.table.default_cellpadding", cellPadding);
    }
    catch (e) {
      dump(e);
    }

    gActiveEditor.beginTransaction();
    try {
      gActiveEditor.cloneAttributes(gTableElement, globalElement);

      // Create necessary rows and cells for the table
      var tableBody = gActiveEditor.createElementWithDefaults("tbody");
      if (tableBody)
      {
        gTableElement.appendChild(tableBody);

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
      gActiveEditor.insertElementAtSelection(gTableElement, true);

      if (deletePlaceholder && gTableElement && gTableElement.nextSibling)
      {
        // Delete the placeholder <br>
        gActiveEditor.deleteNode(gTableElement.nextSibling);
      }
    } catch (e) {}

    gActiveEditor.endTransaction();
    gDialog.rowsInput.value = gRows;
    gDialog.columnsInput.value = gColumns;
    MakePersistsValue(gDialog.rowsInput);
    MakePersistsValue(gDialog.columnsInput);
    SaveWindowLocation();
    return true;
  }
  return false;
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

function checkPreviewChanges(controlID)
{
  var bChanged = false;
  var theChanges = createPreviewChangeArray();
  var sideString = (gCurrentSide == "all") ? "" : gCurrentSide + "-";
 // var bBackgroundIsSelection = (gDialog.BackgroundSelectionRadioGroup.value == "selection");
  switch(controlID)
  {
    case "cellBorderStyleList":
      if (gCollatedCellData.border.style[gCurrentSide] != gDialog.CellBorderStyleList.value)
      {
        gCollatedCellData.border.style[gCurrentSide] = gDialog.CellBorderStyleList.value;
        bChanged = true;
        theChanges.style["border-" + sideString + "style"] = gDialog.CellBorderStyleList.value;
        gCellChangeData.border.style.push(gCurrentSide);
        if (borderStyleToBorderCollapse(gDialog.CellBorderStyleList.value) != gBorderCollapse)
        {
          gBorderCollapse = borderStyleToBorderCollapse(gDialog.CellBorderStyleList.value);
          gTableChangeData.borderCollapse = true;
        }
      }
    break;
    case "cellBorderWidthList":
      if (gCollatedCellData.border.width[gCurrentSide] != gDialog.CellBorderWidthList.value)
      {
        gCollatedCellData.border.width[gCurrentSide] = gDialog.CellBorderWidthList.value;
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
    gSelectionTypeStr = reviseData.getSelectionType(commandStr);  //Do we even want to call this?
//  gSelectedCellsType = translateSelectionTypeString(gSelectionTypeStr);
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

  var theBaseline = msiGetHTMLOrCSSStyleValue(gActiveEditorElement, gTableElement, "valign", "vertical-align");
  if (theBaseline && (theBaseline.length > 0) )
    gTableBaseline = theBaseline;
  var theTableColor = msiGetHTMLOrCSSStyleValue(gActiveEditorElement, gTableElement, bgcolor, cssBackgroundColorStr);
  if (theTableColor && (theTableColor.length > 0) )
    gTableColor = ConvertRGBColorIntoHEXColor(theTableColor);
  gBorderCollapse = borderStyleToBorderCollapse(gCollatedCellData.border.style["all"]);
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
  var borderData = { 
    style : {
      top : "solid", 
      right : "solid", 
      bottom : "solid", 
      left : "solid"
    },
    width : {
      top : "thin", 
      right : "thin", 
      bottom : "thin", 
      left : "thin"
    },
    color : {
      top : "#000000", 
      right : "#000000", 
      bottom : "#000000", 
      left : "#000000"} 
    };
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
  if ("halign" in srcAlignData)
    retAlignData.halign = srcAlignData.halign;
  if ("valign" in srcAlignData)
    retAlignData.valign = srcAlignData.valign;
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
    break;

    case "width":
    case "cellwidth":
      return (!gIsMatrix && !ShouldSetWidthOnCols());
    break;
    case "cellheight":
    case "height":
      return (gIsMatrix || !ShouldSetHeightOnRows());
//      return (!gIsMatrix && !ShouldSetHeightOnRows());
    break;
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
