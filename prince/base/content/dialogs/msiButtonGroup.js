// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

function getButtonValue(buttonNode)
{
  if (buttonNode.value && buttonNode.value.length > 0)
    return buttonNode.value;
  if (buttonNode.getAttribute("value"))
    return buttonNode.getAttribute("value");
  return buttonNode.label;
}

function toggleSelection(parentNode, targetNode)
{
  if (targetNode.disabled)
    return -1;
  if (parentNode==targetNode)
    return -1;

  var kids = parentNode.getKids();
  var nWhichButton = -1;
  var bSelect = true;
  for (var i = 0; i < kids.length; ++i)
  {
    if (kids[i].checked)
    {
      if (kids[i] == targetNode)
        bSelect = false;
      else
        kids[i].checked = false;
    }
    if (kids[i] == targetNode && bSelect)
      nWhichButton = i;
  }
  if (bSelect)
  {
    targetNode.setAttribute("checked", "true");
    targetNode.checked = true;
    parentNode.valueStr = getButtonValue(targetNode);
  }
  else
  {
    targetNode.setAttribute("checked", "false");
    targetNode.checked = false;
    parentNode.valueStr = "";
  }

  var myEvent = document.createEvent("Events");
  myEvent.initEvent("ButtonGroupSelectionChange", true, true);
//  alert("In setSelection, event created with type [" + myEvent.type + "].");
  parentNode.dispatchEvent(myEvent);
  return nWhichButton;
}

function setSelection(parentNode,targetNode)
{
  if (targetNode.disabled)
    return -1;
  if (parentNode==targetNode)
    return -1;

  var kids = parentNode.getKids();
  var nWhichButton = -1;
  for (var i = 0; i < kids.length; ++i)
  {
    if (kids[i].checked)
    {
      if (kids[i] != targetNode)
        kids[i].checked = false;
    }
    if (kids[i] == targetNode)
      nWhichButton = i;
  }
  targetNode.setAttribute("checked", "true");
  parentNode.valueStr = getButtonValue(targetNode);
  var myEvent = document.createEvent("Events");
  myEvent.initEvent("ButtonGroupSelectionChange", true, true);
//  alert("In setSelection, event created with type [" + myEvent.type + "].");
  parentNode.dispatchEvent(myEvent);
  return nWhichButton;
}

function setSelectionByIndex(nWhichButton, groupIDString)
{
  var theGroup = document.getElementById(groupIDString);
  var kids = theGroup.getKids();
  if (nWhichButton >= 0 && nWhichButton < kids.length)
    setSelection(theGroup, kids[nWhichButton]);
}

function setSelectionByValue(parentNode,valueStr)
{
  var theNode = null;
  var theKids = parentNode.getKids();
  if (!theNode)
  {
    for (var i = 0; i < theKids.length; ++i)
    {
      if (getButtonValue(theKids[i]) == valueStr)
      {
        theNode = theKids[i];
        break;
      }
    }
  }
  if (theNode != null)
    return setSelection(parentNode, theNode);
  return -1;
}

function msiAdvanceButton(buttonGroup, event, forward)
{
  var commandDispatcher = document.commandDispatcher;
  var focusElement = event.originalTarget;
  if (!focusElement)
    focusElement = commandDispatcher.focusedElement;

  if (focusElement && focusElement.nodeName == 'button' && focusElement.getAttribute('group')==buttonGroup.id)
  {
    var kids = buttonGroup.getKids();
    var nSelIndex = -1;
    for (var i = 0; i < (kids.length) && (nSelIndex < 0); ++i)
    {
      if (kids[i] == focusElement)
        nSelIndex = i;
    }
    var moveit = true;
    if (forward && nSelIndex < kids.length - 1)
      ++nSelIndex;
    else if (!forward && nSelIndex > 0)
      --nSelIndex;
    else
      moveit = false;
    if (moveit)
    {
      setButtonFocus(kids[nSelIndex]);
      if (buttonGroupSelectsOnFocus(buttonGroup))
      {
        setSelectionByIndex(nSelIndex, buttonGroup.id);
        if (buttonGroup.subIDStr != null && buttonGroup.subIDStr.length > 0)
          setSelectionByIndex(nSelIndex, buttonGroup.subIDStr);
      }
    }
    event.preventDefault();
    event.stopPropagation();
  }
}

function buttonGroupSelectsOnFocus(buttonGroup)
{
  var theAttr = buttonGroup.getAttribute("selectsOnFocus");
  if (theAttr == "false")
    return false;
  return true;
}

function getButtonRows(buttonGroup)
{
  var theRows = buttonGroup.getElementsByTagName("row");
  return theRows;
}

function getRowAndColumnOfItem(rowArray, theItem)
{
  var rowColumn = new Array( -1, -1);
  var found = false;
  for (var i = 0; (found == false) && (i < rowArray.length); ++i)
  {
    var ourItems = rowArray[i].getElementsByTagName("button");
    for (var j = 0; j < ourItems.length; ++j)
    {
      if (ourItems[j] == theItem)
      {
        rowColumn[0] = i;
        rowColumn[1] = j;
        found = true;
        break;
      }
    }
  }
  return rowColumn;
}

function msiAdvanceButtonRow(buttonGroup, event, forward)
{
  var commandDispatcher = document.commandDispatcher;
  var focusElement = event.originalTarget;
  if (!focusElement)
    focusElement = commandDispatcher.focusedElement;

  var moveTo = null;
  if (focusElement && focusElement.nodeName == 'button' && focusElement.getAttribute('group')==buttonGroup.id)
  {
    var theRows = getButtonRows(buttonGroup);
    var rowColumn = getRowAndColumnOfItem(theRows, focusElement);
    if (rowColumn[0] != -1 && rowColumn[1] != -1)
    {

      if (forward && rowColumn[0] < (theRows.length - 1))
      {
        var childButtons = theRows[rowColumn[0] + 1].getElementsByTagName("button");
        if (rowColumn[1] < childButtons.length)
          moveTo = childButtons[ rowColumn[1] ];
        else
          moveTo = childButtons[ childButtons.length - 1 ];
      }
      else if (!forward && rowColumn[0] > 0)
      {
        var childButtons = theRows[rowColumn[0] - 1].getElementsByTagName("button");
        if (rowColumn[1] < childButtons.length)
          moveTo = childButtons[ rowColumn[1] ];
        else
          moveTo = childButtons[ childButtons.length - 1 ];
      }
      if (moveTo != null)
      {
        setButtonFocus(moveTo);
        if (buttonGroupSelectsOnFocus(buttonGroup))
        {
          nSelIndex = setSelection(buttonGroup, moveTo);
          if (nSelIndex >= 0 && buttonGroup.subIDStr != null && buttonGroup.subIDStr.length > 0)
            setSelectionByIndex(nSelIndex, buttonGroup.subIDStr);
        }
      }
    }
  }

  if (moveTo == null)
  {
    if (forward)
      msiTabForward(event);
    else
      msiTabBack(event);
  }
  event.preventDefault();
  event.stopPropagation();
}

function msiCheckSpaceOrEnter(buttonGroup, event)
{
  if (!buttonGroupSelectsOnFocus(buttonGroup))
    return;

  var commandDispatcher = document.commandDispatcher;
  var focusElement = event.originalTarget;
  if (!focusElement)
    focusElement = commandDispatcher.focusedElement;
  if (focusElement && focusElement.nodeName == 'button' && focusElement.getAttribute('group')==buttonGroup.id)
  {
    toggleSelection(buttonGroup, focusElement);
  }

  event.preventDefault();
  event.stopPropagation();
}

//Following is provided as a function with the idea of changing it to use "phantom focus" a la <radiogroup> and <radio>.
function setButtonFocus(theButton)
{
  theButton.focus();
}

function radioGroupSetFocus(radioGroup,focusTarget)
{
  if (radioGroup != focusTarget)
    return;

  var kids = radioGroup.getKids();
  for (var i = 0; i < kids.length; ++i)
  {
    if (kids[i].checked)
    {
      kids[i].focus();
      return;
    }
  }
  kids[0].focus();
}

function elementIsDescendant(child, parent)
{
  for (var currElement = child; (currElement != null) && (currElement != parent.ownerDocument); currElement = currElement.parentNode)
  {
    if (currElement == parent)
      return true;
  }
  return false;
}

function buttonGroupEnable(buttonGroup, doEnable)
{
  var kids = buttonGroup.getKids();
  for (var i = 0; i < kids.length; ++i)
  {
    kids[i].disabled = !doEnable;
  }
  buttonGroup.disabled = !doEnable;
}

function buttonGroupCheckKeys(buttonGroup, event)
{
  switch(event.keyCode)
  {
    case KeyEvent.DOM_VK_UP:          msiAdvanceButtonRow(buttonGroup, event, false);    break;
    case KeyEvent.DOM_VK_DOWN:        msiAdvanceButtonRow(buttonGroup, event, true);     break;
    case KeyEvent.DOM_VK_LEFT:        msiAdvanceButton(buttonGroup, event, false);       break;
    case KeyEvent.DOM_VK_RIGHT:       msiAdvanceButton(buttonGroup, event, true);        break;
    case KeyEvent.DOM_VK_SPACE:
    case KeyEvent.DOM_VK_ENTER:       msiCheckSpaceOrEnter(buttonGroup, event);          break;
    default:
    break;
  }
}

function buttonGroupKeyHandler(event)
{
  buttonGroupCheckKeys(this, event);
}

function makeMSIButtonGroup(element, bSelectOnFocus, subordinateIDString)
{
  if (bSelectOnFocus == false)
    element.setAttribute("selectsOnFocus", "false");
  else
    element.setAttribute("selectsOnFocus", "true");
  element.addEventListener('keypress', buttonGroupKeyHandler, true);
  element.subIDStr = "";
  if (subordinateIDString && (subordinateIDString.length > 0))
    element.subIDStr = subordinateIDString;
  element.valueStr = "";
  element.getKids = function() {
    var kids = this.getElementsByAttribute("group", this.id);
    if (kids.length == 0)
    {
      kids = this.getButtonsByTagName("button");
    }
    return kids;
  };
  element.isMSIButtonGroup = true;
}
