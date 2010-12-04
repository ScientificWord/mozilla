// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

function controlIsButton(aNode)
{
  return (aNode.nodeName == 'button' || aNode.nodeName == 'msibutton');
}

function getButtonValue(buttonNode)
{
  if (buttonNode.value && buttonNode.value.length > 0)
    return buttonNode.value;
  if (buttonNode.getAttribute("value"))
    return buttonNode.getAttribute("value");
  return buttonNode.label;
}

function getButtonGroupKids(parentNode)
{
  if ("getKids" in parentNode)
    return parentNode.getKids();
  dump("Problem in msiButtonGroup.js! getKids() not defined\n");
  return new Array(0);
}

function toggleSelection(parentNode, targetNode)
{
  if (targetNode.disabled)
    return -1;
  if (parentNode==targetNode)
    return -1;

  var kids = getButtonGroupKids(parentNode);
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

  var kids = getButtonGroupKids(parentNode);
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
  var kids = getButtonGroupKids(theGroup);
  if (nWhichButton >= 0 && nWhichButton < kids.length)
    setSelection(theGroup, kids[nWhichButton]);
}

function setSelectionByValue(parentNode,valueStr)
{
  var theNode = null;
  var theKids = getButtonGroupKids(parentNode);
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

  if (focusElement && controlIsButton(focusElement) && focusElement.getAttribute('group')==buttonGroup.id)
  {
    var kids = getButtonGroupKids(buttonGroup);
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
    return true;
  }
  return false;
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

function buttonWalkerCallback(aNode)
{
  if (controlIsButton(aNode))
    return NodeFilter.FILTER_ACCEPT;
  return NodeFilter.FILTER_SKIP;
}

function getButtonsInNode(aParent)
{
  var buttonArray = [];
  var treeWalker = document.createTreeWalker(aParent, NodeFilter.SHOW_ELEMENT, buttonWalkerCallback, true);
  if (treeWalker)
  {
    for (var currNode = treeWalker.nextNode(); currNode != null; currNode = treeWalker.nextNode())
    {
      buttonArray.append(currNode);
    }
  }
  return buttonArray;
}

function getRowAndColumnOfItem(rowArray, theItem)
{
  var rowColumn = new Array( -1, -1);
  var found = false;
  for (var i = 0; (found == false) && (i < rowArray.length); ++i)
  {
    var ourItems = getButtonsInNode(rowArray[i]);
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
  if (focusElement && controlIsButton(focusElement) && focusElement.getAttribute('group')==buttonGroup.id)
  {
    var theRows = getButtonRows(buttonGroup);
    var rowColumn = getRowAndColumnOfItem(theRows, focusElement);
    if (rowColumn[0] != -1 && rowColumn[1] != -1)
    {

      if (forward && rowColumn[0] < (theRows.length - 1))
      {
        var childButtons = getButtonsInNode(theRows[rowColumn[0] + 1]);
        if (rowColumn[1] < childButtons.length)
          moveTo = childButtons[ rowColumn[1] ];
        else
          moveTo = childButtons[ childButtons.length - 1 ];
      }
      else if (!forward && rowColumn[0] > 0)
      {
        var childButtons = getButtonsInNode(theRows[rowColumn[0] + 1]);
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
  return true;
}

function msiButtonGroupCheckEnter(buttonGroup, event)
{
//Removed the SelectsOnFocus condition - if you hit enter in button group in a dialog, it'll accept the dialog (if that's not disabled)
//Should be possible to do a more discriminating test here, but it's proving too hard to find where some of the behaviors are coming from.
//  if (buttonGroupSelectsOnFocus(buttonGroup))
  var dlg = window.document.documentElement;
  dlg._hitEnter(event);
  return true;
}

function msiButtonGroupCheckSpace(buttonGroup, event)
{
//  if ( ((event.keyCode == event.DOM_VK_RETURN) || (event.keyCode == event.DOM_VK_ENTER)) && (buttonGroupSelectsOnFocus(buttonGroup)) )
//  {
//    var dlg = window.document.documentElement;
//    dlg._hitEnter(event);
//    return false;
//  }

  var commandDispatcher = document.commandDispatcher;
  var focusElement = event.originalTarget;
  if (!focusElement)
    focusElement = commandDispatcher.focusedElement;
  if (focusElement && controlIsButton(focusElement) && focusElement.getAttribute('group')==buttonGroup.id)
  {
    toggleSelection(buttonGroup, focusElement);
    event.preventDefault();
    event.stopPropagation();
    return true;
  }
  return false;
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

  var kids = getButtonGroupKids(radioGroup);
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
  var kids = getButtonGroupKids(buttonGroup);
  for (var i = 0; i < kids.length; ++i)
  {
    kids[i].disabled = !doEnable;
  }
  buttonGroup.disabled = !doEnable;
}

function buttonGroupCheckKeys(buttonGroup, event)
{
//  var matchStr = "";
  var retVal = false;
  if (event.keyCode == event.DOM_VK_UP)
  {
//    matchStr = String(event.DOM_VK_UP);
    retVal = msiAdvanceButtonRow(buttonGroup, event, false);
  }
  else if (event.keyCode == event.DOM_VK_DOWN)
  {
//    matchStr = String(event.DOM_VK_DOWN);
    retVal = msiAdvanceButtonRow(buttonGroup, event, true);
  }
  else if (event.keyCode == event.DOM_VK_LEFT)
  {
//    matchStr = String(event.DOM_VK_LEFT);
    retVal = msiAdvanceButton(buttonGroup, event, false);
  }
  else if (event.keyCode == event.DOM_VK_RIGHT)
  {
//    matchStr = String(event.DOM_VK_RIGHT);
    retVal = msiAdvanceButton(buttonGroup, event, true);
  }
  else if ( (event.keyCode == event.DOM_VK_RETURN) || (event.keyCode == event.DOM_VK_ENTER) )
  {
//    if (event.keyCode == event.DOM_VK_RETURN)
//      matchStr = String(event.DOM_VK_RETURN);
//    if (event.keyCode == event.DOM_VK_ENTER)
//      matchStr += matchStr.length ? ("," + String(event.DOM_VK_ENTER)) : (String(event.DOM_VK_ENTER));
    retVal = msiButtonGroupCheckEnter(buttonGroup, event);
  }
  else if ( (event.keyCode == event.DOM_VK_SPACE) || ((event.keyCode == 0) && (event.charCode == ' ')) )
  {
//    if (event.keyCode == event.DOM_VK_SPACE)
//      matchStr = String(event.DOM_VK_SPACE);
//    else
//      matchStr = " ";
    retVal = msiButtonGroupCheckSpace(buttonGroup, event);
  }
//  else
//  {
//    matchStr = "not recognized";
//  }
//  msiDumpWithID("In msiButtonGroup's buttonGroupCheckKeys for buttongroup [@], keycode [" + event.keyCode + "]; reported match with [" + matchStr + "].\n", buttonGroup);
  return retVal;

//  switch(event.keyCode)
//  {
//    case event.DOM_VK_UP:          return msiAdvanceButtonRow(buttonGroup, event, false);    break;
//    case event.DOM_VK_DOWN:        return msiAdvanceButtonRow(buttonGroup, event, true);     break;
//    case event.DOM_VK_LEFT:        return msiAdvanceButton(buttonGroup, event, false);       break;
//    case event.DOM_VK_RIGHT:       return msiAdvanceButton(buttonGroup, event, true);        break;
//    case event.DOM_VK_SPACE:       return msiButtonGroupCheckSpace(buttonGroup, event);          break;
//    case event.DOM_VK_RETURN:
//    case event.DOM_VK_ENTER:       return msiButtonGroupCheckEnter(buttonGroup, event);          break;
//    default:
//      msiDumpWithID("In msiButtonGroup's buttonGroupCheckKeys for buttongroup [@], keycode [" + event.keyCode + "] unrecognized.\n", buttonGroup);
//      return false;
//    break;
//  }
}

function buttonGroupKeyHandler(event)
{
  return buttonGroupCheckKeys(this, event);
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
      kids = getButtonsInNode(this);
    }
    return kids;
  };
  element.isMSIButtonGroup = true;
}
