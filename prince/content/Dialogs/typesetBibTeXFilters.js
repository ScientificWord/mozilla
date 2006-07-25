// Copyright (c) 2006 MacKichan Software, Inc.  All Rights Reserved.


//const mmlns    = "http://www.w3.org/1998/Math/MathML";
const xhtmlns  = "http://www.w3.org/1999/xhtml";

var data;

var bibTeXSplitExp = new RegExp("(\\\\[^a-zA-Z0-9])|([\$=@%{}\\\"\\[\\]\\(\\),])");

// dialog initialization code
function Startup()
{
  var editor = GetCurrentEditor();
  if (!editor) {
    window.close();
    return;
  }

  doSetOKCancel(onAccept, onCancel);
  data = window.arguments[0];
  data.Cancel = false;

  gDialog.entryTypes = data.entryTypes;
  gDialog.showEntryTypes = data.showEntryTypes;
  gDialog.fieldTypes = data.fieldTypes;
  gDialog.displayFields = data.displayFields;
  gDialog.filters = data.filters;

  InitDialog();

  document.getElementById("entryTypesListbox").focus();

  SetWindowLocation();
}

function InitDialog()
{
  fillEntryTypesListbox();
  fillFiltersFieldsListbox();

//  checkInaccessibleAcceleratorKeys(document.documentElement);

//  gDialog.tabOrderArray = new Array( gDialog.positionSpecGroup,
//                                       document.documentElement.getButton("accept"),
//                                       document.documentElement.getButton("cancel") );
  
  document.documentElement.getButton("accept").setAttribute("default", true);
}

function onAccept()
{
  readEntryTypesListbox();
  readFiltersFieldsListbox();

  data.showEntryTypes = gDialog.showEntryTypes;
  data.fieldTypes = gDialog.fieldTypes;
  data.displayFields = gDialog.displayFields;
  data.filters = gDialog.filters;

  SaveWindowLocation();
  return true;
}

function onCancel()
{
  data.Cancel = true;
}

function doAccept()
{
  document.documentElement.getButton('accept').oncommand();
}

function fillEntryTypesListbox()
{
  var theListbox = document.getElementById("entryTypesListbox");
  for (var i = 0; i < gDialog.entryTypes.length; ++i)
  {
    var newItem = theListbox.appendItem(gDialog.entryTypes[i]);
    if (findInArray( gDialog.showEntryTypes, gDialog.entryTypes[i] ) >= 0)
    {
      theListbox.addItemToSelection(newItem);
      //The following explicit settings attempt to get around bugginess in scrolling multi-select listboxes.
      newItem.setAttribute("selected", "true");
//      newItem.selected = true;
      newItem.label = gDialog.entryTypes[i];
    }
  }
}

function readEntryTypesListbox()
{
  gDialog.showEntryTypes.splice(0, gDialog.showEntryTypes.length);
  var theListbox = document.getElementById("entryTypesListbox");
  var selItems = theListbox.selectedItems;
  for (var i = 0; i < selItems.length; ++i)
    gDialog.showEntryTypes.push(selItems[i].getAttribute("label"));
}

function fillFiltersFieldsListbox()
{
  var theListbox = document.getElementById("filtersFieldsListbox");
  //Clear the listbox
  var items = theListbox.getElementsByTagName("listitem");
  for (var i = items.length - 1; i >= 0; --i)
    theListbox.removeChild(items[i]);

  for (i = 0; i < gDialog.fieldTypes.length; ++i)
  {
    var theField = gDialog.fieldTypes[i];

    var newItem = document.createElement("listitem");
    newItem.setAttribute("allowevents", "true");  //Necessary to get checkboxes to work - from xulplanet listcell Notes
    
    //First, the field name column:
    var newCell = document.createElement("listcell");
    newCell.setAttribute("label", theField);
    newItem.appendChild(newCell);
    
    //Now the "displayed" checkbox column:
    newCell = document.createElement("listcell");
    var checkBox = document.createElement("checkbox");
    if (findInArray(gDialog.displayFields, theField) >= 0)
      checkBox.setAttribute("checked", "true");
    else
      checkBox.setAttribute("checked", "false");
    newCell.appendChild(checkBox);
    newItem.appendChild(newCell);

    //Now look for a filter:
    var aFilter = findFilterForField(gDialog.filters, theField);
    var filterPattern = "";
    if (aFilter != null)
      filterPattern = aFilter.matchExpr;

    //Set the "filter" column:
    var newCell = document.createElement("listcell");
    var newTextbox = document.createElement("textbox");
    newTextbox.setAttribute("allowevents", true);
//    newTextbox.value = filterPattern;
    newTextbox.setAttribute("value", filterPattern);
    newCell.appendChild(newTextbox);
    newItem.appendChild(newCell);
    
    //Now the "exact case" checkbox column:
    newCell = document.createElement("listcell");
//    newCell.setAttribute("type", "checkbox");
    checkBox = document.createElement("checkbox");
    if (aFilter && aFilter.bExactCase)
      checkBox.setAttribute("checked", "true");
    else
      checkBox.setAttribute("checked", "false");
    if (aFilter)
      newCell.setAttribute("disabled", "false");
    else
      newCell.setAttribute("disabled", "true");
    newCell.appendChild(checkBox);
    newItem.appendChild(newCell);

    //Now the "regular expression" checkbox column:
    newCell = document.createElement("listcell");
//    newCell.setAttribute("type", "checkbox");
    checkBox = document.createElement("checkbox");
    if (aFilter && aFilter.bRegExp)
      checkBox.setAttribute("checked", "true");
    else
      checkBox.setAttribute("checked", "false");
    if (aFilter)
      newCell.setAttribute("disabled", "false");
    else
        newCell.setAttribute("disabled", "true");
    newCell.appendChild(checkBox);
    newItem.appendChild(newCell);

    theListbox.appendChild(newItem);
  }
}


function readFiltersFieldsListbox()
{
//  gDialog.displayFields.splice(0, gDialog.displayFields.length);
  var newFields = new Array();
  gDialog.filters.splice(0, gDialog.filters.length);

  var theListbox = document.getElementById("filtersFieldsListbox");

  var theItems = theListbox.getElementsByTagName("listitem");
  for (i = 0; i < theItems.length; ++i)
  {
    var theItem = theItems[i];
    var theColumns = theItem.getElementsByTagName("listcell");
    var theField = theColumns[0].getAttribute("label");
//    if (1 < theColumns.length && theColumns[1].getAttribute("checked") == "true")
    if (1 < theColumns.length && getEmbeddedCheckboxValue(theColumns[1]))
      newFields.push(theField);
//      gDialog.displayFields.push(theField);
    var matchExpr = "";
    if (2 < theColumns.length && (matchExpr = getEmbeddedTextboxValue(theColumns[2])) && matchExpr.length > 0)
    {
      aFilter = new bibTeXFilter();
      aFilter.keyName = theField;
      aFilter.matchExpr = matchExpr;
      if (3 < theColumns.length && getEmbeddedCheckboxValue(theColumns[3]))
        aFilter.bExactCase = true;
      else
        aFilter.bExactCase = false;
      if (4 < theColumns.length && getEmbeddedCheckboxValue(theColumns[4]))
        aFilter.bRegExp = true;
      else
        aFilter.bRegExp = false;
      gDialog.filters.push(aFilter);
    }
  }

  intersectArrayWith(gDialog.displayFields, newFields);  //this call will remove all members of gDialog.displayFields which aren't in newFields, but without changing order
  unionArrayWith(gDialog.displayFields, newFields);  //this call will add all members of newFields which aren't in gDialog.displayFields, but without changing order
}

function selectEntryType(event)
{
  var theListbox = document.getElementById("entryTypesListbox");
  var origTarget = event.explicitOriginalTarget;
  if (origTarget.nodeName == "listitem")
  {
    if (origTarget.selected)
      origTarget.setAttribute("selected", "true");
    else
      origTarget.setAttribute("selected", "false");
  }
}

function getEmbeddedCheckboxValue(aListCell)
{
  var kids = aListCell.getElementsByTagName("checkbox");
  if (kids.length)
    return (kids[0].getAttribute("checked") == "true");
  return false;
}

function getEmbeddedTextboxValue(aListCell)
{
  var kids = aListCell.getElementsByTagName("textbox");
//  if (kids.length)
//    return kids[0].value;
  if (kids.length && kids[0].inputField)
    return kids[0].inputField.value;
  return "";
}

function findFilterForField(filters, theField)
{
  var theFilter = null;
  for (var i = 0; i < filters.length; ++i)
  {
    if (filters[i].keyName == theField)
    {
      theFilter = filters[i];
      break;
    }
  }
  return theFilter;
}
