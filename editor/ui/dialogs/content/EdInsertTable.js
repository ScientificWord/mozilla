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

//Cancel() is in EdDialogCommon.js
var gTableElement = null;
var gRows;
var gColumns;
var gActiveEditor;
var gCellID = 12;
var gPrefs;

// dialog initialization code
function Startup()
{
  debugger;
  gActiveEditor = msiGetTableEditor();
  if (!gActiveEditor)
  {
    dump("Failed to get active table editor!\n");
    window.close();
    return;
  }

  try {
    gTableElement = gActiveEditor.createElementWithDefaults("table");
    gTableElement.setAttribute("req","tabulary")
  } catch (e) {}

  if(!gTableElement)
  {
    dump("Failed to create a new table!\n");
    window.close();
    return;
  }
  gDialog.rowsInput      = document.getElementById("rowsInput");
  gRows = gDialog.rowsInput.value;
  gDialog.columnsInput   = document.getElementById("columnsInput");
  gColumns = gDialog.columnsInput.value;
  gDialog.widthInput     = document.getElementById("widthInput");
  gDialog.borderInput    = document.getElementById("borderInput");
  gDialog.horizAlignment = document.getElementById("horizAlignment");
  gDialog.vertAlignment  = document.getElementById("vertAlignment");
  gDialog.textWrapping   = document.getElementById("textWrapping");
  gDialog.cellSpacing    = document.getElementById("cellSpacing");
  gDialog.cellPadding    = document.getElementById("cellPadding");
  gDialog.autoCheckbox   = document.getElementById("autoWidthCheckbox");

  var cellid;
  if (gRows <= 6 && gColumns <= 6) cellid = 10*(Number(gRows) -1 )+Number(gColumns);
  else {
    cellid = 56;
    // Table is too big for the 'quickly' tab; activate the 'precisely' tab.
    document.getElementById("tabpanel-tabs").selectedIndex = 1;
  }
  gDialog.widthPixelOrPercentMenulist = document.getElementById("widthPixelOrPercentMenulist");
  gDialog.OkButton = document.documentElement.getButton("accept");
  gDialog.sizeLabel = document.getElementById("sizeLabel");

  gTableElement.setAttribute("border", gDialog.borderInput.value);
  if (gDialog.widthInput.value)
    gTableElement.setAttribute("width", Number(gDialog.widthInput.value) +
                                        (gDialog.widthPixelOrPercentMenulist.value == "pc" ? "%" : ""));
  
  // Make a copy to use for AdvancedEdit
  globalElement = gTableElement.cloneNode(false);
  try {
    gPrefs = GetPrefs();
    if (IsHTMLEditor()
        && !(gActiveEditor.flags & Components.interfaces.nsIPlaintextEditor.eEditorMailMask))
    {
      if (gPrefs.getBoolPref("editor.use_css"))
      {
        // only for Composer and not for htmlmail
        globalElement.setAttribute("style", "text-align: left;");
      }

      var hAlign = gPrefs.getCharPref("editor.table.default_align");
      var vAlign = gPrefs.getCharPref("editor.table.default_valign");
      var wrapping = gPrefs.getCharPref("editor.table.default_wrapping");

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

  gDialog.horizAlignment.value = hAlign;
  gDialog.vertAlignment.value  = vAlign;
  gDialog.textWrapping.selectedItem = (wrapping == "nowrap") ?
                                       document.getElementById("nowrapRadio") :
                                       document.getElementById("wrapRadio");
  gDialog.cellSpacing.value    = globalElement.getAttribute("cellspacing");
  gDialog.cellPadding.value    = globalElement.getAttribute("cellpadding");
  checkEnableWidthControls();
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
  gRows = msiValidateNumber(gDialog.rowsInput, null, 1, gMaxRows, null, null, true)
  if (gValidationError)
    return false;

  gColumns = msiValidateNumber(gDialog.columnsInput, null, 1, gMaxColumns, null, null, true)
  if (gValidationError)
    return false;

  // Set attributes: NOTE: These may be empty strings (last param = false)
  msiValidateNumber(gDialog.borderInput, null, 0, gMaxPixels, globalElement, "border", false);
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

  SetOrResetAttribute(globalElement, "cellspacing", gDialog.cellSpacing.value);
  SetOrResetAttribute(globalElement, "cellpadding", gDialog.cellPadding.value);

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
    try {
      if (IsHTMLEditor()
          && !(gActiveEditor.flags & Components.interfaces.nsIPlaintextEditor.eEditorMailMask))
      {
        var wrapping = gDialog.textWrapping.selectedItem.value;
        gPrefs.setCharPref("editor.table.default_wrapping", wrapping);

        var align = gDialog.horizAlignment.value;
        gPrefs.setCharPref("editor.table.default_align", align);

        var valign = gDialog.vertAlignment.value;
        gPrefs.setCharPref("editor.table.default_valign", valign);

        var cellSpacing = globalElement.getAttribute("cellspacing");
        gPrefs.setCharPref("editor.table.default_cellspacing", cellSpacing);

        var cellPadding = globalElement.getAttribute("cellpadding");
        gPrefs.setCharPref("editor.table.default_cellpadding", cellPadding);

      }
    }
    catch (e) {};

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

