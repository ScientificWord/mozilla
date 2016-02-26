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

//Cancel() is in msiEdDialogCommon.js
var gTableElement = null;
var gRows;
var gColumns;
var gActiveEditor;
var gCellID = 12;
var gPrefs;

// dialog initialization code
function Startup()
{
  gDialog.rowsInput      = document.getElementById("rowsInput");
  gDialog.columnsInput   = document.getElementById("columnsInput");
//  gDialog.widthInput     = document.getElementById("widthInput");
//  gDialog.borderInput    = document.getElementById("borderInput");
//  gDialog.horizAlignment = document.getElementById("horizAlignment");
//  gDialog.vertAlignment  = document.getElementById("vertAlignment");
//  gDialog.textWrapping   = document.getElementById("textWrapping");
//  gDialog.cellSpacing    = document.getElementById("cellSpacing");
//  gDialog.cellPadding    = document.getElementById("cellPadding");

//  gDialog.widthPixelOrPercentMenulist = document.getElementById("widthPixelOrPercentMenulist");
  gDialog.OkButton = document.documentElement.getButton("accept");
  gDialog.sizeLabel = document.getElementById("sizeLabel");
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
  /*gDialog.widthInput.value =*/ msiInitPixelOrPercentMenulist(globalElement, null, "width", "widthPixelOrPercentMenulist", gPercent);
  /*gDialog.borderInput.value = globalElement.getAttribute("border");*/

//  gDialog.horizAlignment.value = hAlign;
//  gDialog.vertAlignment.value  = vAlign;
//  gDialog.textWrapping.selectedItem = (wrapping == "nowrap") ?
//                                       document.getElementById("nowrapRadio") :
//                                       document.getElementById("wrapRadio");
//  gDialog.cellSpacing.value    = globalElement.getAttribute("cellspacing");
//  gDialog.cellPadding.value    = globalElement.getAttribute("cellpadding");
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
//  SetElementEnabledById("AdvancedEditButton1", enable);
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

//  // Set attributes: NOTE: These may be empty strings (last param = false)
//  msiValidateNumber(gDialog.borderInput, null, 0, gMaxPixels, globalElement, "border", false);
//  // TODO: Deal with "BORDER" without value issue
//  if (gValidationError) return false;
//
//  msiValidateNumber(gDialog.widthInput, gDialog.widthPixelOrPercentMenulist,
//                 1, gMaxTableSize, globalElement, "width", false);
//  gDialog.widthPixelOrPercentMenulist.value = gDialog.widthPixelOrPercentMenulist.selectedItem.value;
//
//  SetOrResetAttribute(globalElement, "cellspacing", gDialog.cellSpacing.value);
//  SetOrResetAttribute(globalElement, "cellpadding", gDialog.cellPadding.value);
//
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
        var align = gDialog.horizAlignment.value;
        var valign = gDialog.vertAlignment.value;
        var cellSpacing = globalElement.getAttribute("cellspacing");
        var cellPadding = globalElement.getAttribute("cellpadding");
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

    SaveWindowLocation();
    return true;
  }
  return false;
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
  var columns  = (gCellID % 10);
  var rows     = Math.ceil(gCellID / 10);

  gDialog.rowsInput.value    = rows;
  gDialog.columnsInput.value = columns;

  onAccept();
  window.close();
}
