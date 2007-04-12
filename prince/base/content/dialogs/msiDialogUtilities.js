// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

var specialAcceleratorList=null;

function inaccessibleAcceleratorFilter(anElement)
{
  if (!anElement.accessKey)
    return NodeFilter.FILTER_SKIP;
  switch(anElement.nodeName)
  {
    case 'button':
    case 'checkbox':
    case 'radio':
      return NodeFilter.FILTER_SKIP;
    break;
    default:  //all the rest are in the "don't normally take access keys" camp
      return NodeFilter.FILTER_ACCEPT;
    break;
  }
  return NodeFilter.FILTER_SKIP;
}

function addToSpecialAccList(newChar)
{
  if (specialAcceleratorList==null)
    specialAcceleratorList = new Array(newChar);
  else
    specialAcceleratorList.push(newChar);
}

function checkInaccessibleAcceleratorKeys(rootElement)
{
  var iterator = rootElement.ownerDocument.createTreeWalker(rootElement, NodeFilter.SHOW_ELEMENT, 
                                                         inaccessibleAcceleratorFilter, false);
  var foundSome = false;
  var isOkay = true;
  var started = false;
  while (isOkay) {
    try {
      isOkay = iterator.nextNode();
    }
    catch(anException) {
      isOkay = false;
    }
    if (!isOkay)
      break;
    started = true;
    var test = iterator.currentNode;
    if (test && test.accessKey)
    {
      foundSome = true;
      addToSpecialAccList( test.accessKey.toLowerCase() );
    }
    if (started && test == rootElement)
      break;
  }
  if (foundSome)
  {
    rootElement.ownerDocument.documentElement.addEventListener('keypress', specialAcceleratorKeysHandler, true);
    rootElement.ownerDocument.documentElement.addEventListener('keypress', arrowKeyHandler, true);
  }
}

function acceleratorKeyFilter(anElement)
{
//  alert('Checking node ' + anElement.nodeName + ' with access key [' + anElement.accessKey + '].');
  if ((anElement.accessKey!=null) && (anElement.accessKey.length>0) && !anElement.disabled && !anElement.collapsed && !anElement.hidden)
    return NodeFilter.FILTER_ACCEPT;
  return NodeFilter.FILTER_SKIP;
}

function shouldFireOnAccessKey(anElement)
{
  switch(anElement.localName)
  {
    case 'button':
    case 'checkbox':
    case 'radio':
      return true;
    break;
    default:
    break;
  }
  return false;
}

function canHandleKeys(anElement)
{
  switch(anElement.nodeName)
  {
    case 'textarea':
    case 'listbox':
    case 'listitem':
    case 'richlistbox':
    case 'richlistitem':
      return true;
    break;
    default:
    break;
  }
  return false;
}

function canHandleArrowKeys(anElement)
{
  if (canHandleKeys(anElement))
    return true;
  if (anElement.group != null && anElement.group.length > 0)
    return true;  //not that this element can, but its parent can?
  if (anElement.isMSIButtonGroup == true)
    return true;
  switch(anElement.localName)
  {
    case 'radiogroup':
    case 'grid':
      return true;
    break;
    default:
    break;
  }
  return false;
}

function arrowKeyHandler(event)
{
  var tabDirection = 0;
  switch(event.keyCode)
  {
    case KeyEvent.DOM_VK_UP:          tabDirection = -1;             break;
    case KeyEvent.DOM_VK_DOWN:        tabDirection = 1;              break;
    case KeyEvent.DOM_VK_LEFT:        tabDirection = -1;             break;
    case KeyEvent.DOM_VK_RIGHT:       tabDirection = 1;              break;
    default:
    break;
  }
  if (tabDirection == 0)
    return;

  var anElement = event.originalTarget;
  for (var currElement = anElement; (currElement != null) && !canHandleArrowKeys(currElement); currElement = currElement.parentNode)
  {
    if (currElement == anElement.ownerDocument.documentElement)
    {
      if (tabDirection == 1)
        msiTabForward(event);
      else if (tabDirection == -1)
        msiTabBack(event);
      break;
    }
  }
//Otherwise, do nothing - if we aborted out of the previous loop before reaching the document element, it means we
//found an element we claim can "handle the arrow keys".
}

function watchForKey(charPressed)
{
  var found = false;
  for (var i = 0; !found && (i < specialAcceleratorList.length); ++i)
  {
    if (specialAcceleratorList[i] == charPressed)
      found = true;
    if (specialAcceleratorList[i].charCodeAt(0) == charPressed.charCodeAt(0))
      found = true;
  }
//  alert("In watchForKey, checking character [" + charPressed + "]; found is [" + found + "]");
//  var specCharList = "";
//  for (var ix in specialAcceleratorList)
//  {
//    specCharList += "[" + ix + "=" + specialAcceleratorList[ix] + "]";
//  }
//  alert("specialAcceleratorList has [" + specialAcceleratorList.length + "] elements: " + specCharList);
  return found;
}

function specialAcceleratorKeysHandler(event)
{
  var commandDispatcher = this.ownerDocument.commandDispatcher;
  var currElement = event.originalTarget;
  if (!currElement)
    currElement = commandDispatcher.focusedElement;
  if (!event.altkey && canHandleKeys(currElement))
    return;
  if (!event.charCode)
    return;
  if (!watchForKey(String.fromCharCode(event.charCode).toLowerCase()))
    return;

  var charPressedLower = String.fromCharCode(event.charCode).toLowerCase();
  var iterator = this.ownerDocument.createTreeWalker(currElement, NodeFilter.SHOW_ELEMENT, 
                                                         acceleratorKeyFilter, false);
  var found = false;
  var isOkay = true;
  var startedOver = false;
  while (isOkay && !found)
  {
    try {
      isOkay = (iterator.nextNode()!=null);
    }
    catch(anException) {
      isOkay = false;
    }
    var test = iterator.currentNode;
    if (!test)
      isOkay = false;
//    alert('Checking node ' + test.nodeName + ' with access key [' + test.accessKey + ']; searching for [' + charPressedLower + '].');
    if (!isOkay)
    {
      if (!startedOver)
      {
//        alert('Starting over in specialAcceleratorKeysHandler.');
        iterator = this.ownerDocument.createTreeWalker(this.ownerDocument.documentElement, NodeFilter.SHOW_ELEMENT, 
                                                         acceleratorKeyFilter, false);
        startedOver = true;
        isOkay = true;
      }
      continue;
    }
    if (startedOver && test==currElement)
      break;
    if (!test.accessKey)
      continue;

    var testKey = test.accessKey.toLowerCase();
    if (test.nodeName == 'label')
    {
      test = this.ownerDocument.getElementById(test.control);
      if (!test)
        continue;
    }
    if (testKey == charPressedLower)
    {
      test.focus();
      if (shouldFireOnAccessKey(test))
        test.doCommand();
      found = true;
    }
  }
  if (found)
  {
    event.preventDefault();
    event.stopPropagation();
  }
}

//This silly function simply finds an object in an array and returns its index. Why isn't this part of the JavaScript
//basic language, or is it?
function lookUpInArray(theItem, theArray)
{
  if (theArray == null || theItem == null)
    return -1;

  var theIndex = -1;
  for (var i = 0; i < theArray.length; ++i)
  {
    if (theArray[i] == theItem)
    {
      theIndex = i;
      break;
    }
  }
  return theIndex;
}

//This function attempts to find the focus element or an ancestor in the tabOrderArray. If no tabOrderArray is
//given, it falls back on getting a suitable element to tab from based on the type of the focus element.
function findNearestAncestor(focusElement, tabOrderArray)
{
  var startTarget = null;
  if (tabOrderArray != null)
  {
    var currElement = focusElement;
    while (startTarget == null && currElement != null)
    {
      if (lookUpInArray(currElement, tabOrderArray) >= 0)
      {
        startTarget = currElement;
        break;
      }
      currElement = currElement.parentNode;
      if (currElement == document.documentElement)
        break;
    }
  }
  if (startTarget == null)  //failed to get anything from the above
  {
//    alert("Hit the null startTarget case!");
    var groupID = "";
    if (focusElement)
    {
      switch(focusElement.nodeName)
      {
        case 'hbox':
        case 'radiogroup':
        case 'checkbox':
          groupID = focusElement.id;
        break;
        case 'button':
        case 'radio':
          groupID = focusElement.getAttribute('group');
        break;

        default:
        break;
      }
    }
    if (groupID != "")
      startTarget = document.getElementById(groupID);
  }

  if (startTarget == null)
    startTarget = focusElement;
  return startTarget;
}

function msiTabForward(event)
{
  var commandDispatcher = document.commandDispatcher;
  var focusElement = event.originalTarget;
  if (!focusElement)
    focusElement = commandDispatcher.focusedElement;

  var startTarget = findNearestAncestor(focusElement, gDialog.tabOrderArray);

  var doDefault = true;
  var startItem = -1;
  if (gDialog.tabOrderArray != null)
  {
    startItem = lookUpInArray(startTarget, gDialog.tabOrderArray);
//    for (var i = 0; i < gDialog.tabOrderArray.length; ++i)
//    {
//      if (startTarget == gDialog.tabOrderArray[i])
//      {
//        startItem = i;
//        break;
//      }
//    }
    for (var i = 1; (startItem >= 0) && (i < gDialog.tabOrderArray.length); ++i)
    {
      var trialItem = startItem + i;
      if (trialItem >= gDialog.tabOrderArray.length)
        trialItem -= gDialog.tabOrderArray.length;
      if (!gDialog.tabOrderArray[trialItem].disabled)
      {
        doDefault = false;
        gDialog.tabOrderArray[trialItem].focus();
        break;
      }
    }
  }

  if (doDefault)
    commandDispatcher.advanceFocus();
  event.preventDefault();
  event.stopPropagation();
}

function msiTabBack(event)
{
  var commandDispatcher = document.commandDispatcher;
  var focusElement = event.originalTarget;
  if (!focusElement)
    focusElement = commandDispatcher.focusedElement;

  var startTarget = findNearestAncestor(focusElement, gDialog.tabOrderArray);

  var doDefault = true;
  var startItem = -1;
  if (gDialog.tabOrderArray != null)
  {
    startItem = lookUpInArray(startTarget, gDialog.tabOrderArray);
//    for (var i = 0; i < gDialog.tabOrderArray.length; ++i)
//    {
//      if (startTarget == gDialog.tabOrderArray[i])
//      {
//        startItem = i;
//        break;
//      }
//    }
    for (i = 1; (startItem >= 0) && (i < gDialog.tabOrderArray.length); ++i)
    {
      var trialItem = startItem - i;
      if (trialItem < 0)
        trialItem += gDialog.tabOrderArray.length;
      if (!gDialog.tabOrderArray[trialItem].disabled)
      {
        doDefault = false;
        gDialog.tabOrderArray[trialItem].focus();
        break;
      }
    }
  }

  if (doDefault)
    commandDispatcher.rewindFocus();

  event.preventDefault();
  event.stopPropagation();

}

function makeSampleWindowDependOn(sampleControl, controlArray)
{
  if (gDialog.sampleControlArray == null)
    gDialog.sampleControlArray = new Array(sampleControl);
  else
    gDialog.sampleControlArray[gDialog.sampleControlArray[length]] = sampleControl;
  sampleControl.sampleDependsOn = controlArray;
  document.documentElement.addEventListener('ButtonGroupSelectionChange', checkRedrawSample, false);
  document.documentElement.addEventListener('command', checkRedrawSample, false);
//  document.documentElement.addEventListener('RadioStateChange', checkRedrawSample, false);
//  document.documentElement.addEventListener('CheckboxStateChange', checkRedrawSample, false);
}

function checkRedrawSample(event)
{
  if (gDialog.sampleControlArray == null)
    return;

//  var bRedraw = false;
  for (var i = 0; i < gDialog.sampleControlArray.length; ++i)
  {
    var theTarget = null;
    switch(event.type)
    {
//      case "RadioStateChange":
      case "ButtonGroupSelectionChange":
      case "command":
        theTarget = findNearestAncestor(event.originalTarget, gDialog.sampleControlArray[i].sampleDependsOn);
        //We use the same sort of functionality as that in "tabPositionFromElement". Probably should rename that function.
      break;
//      case "CheckboxStateChange":
//        theTarget = event.originalTarget;
//      break;
    }

    if ((theTarget != null) && (gDialog.sampleControlArray[i].sampleDependsOn != null) && (lookUpInArray(theTarget, gDialog.sampleControlArray[i].sampleDependsOn) >= 0))
    {
      drawSample(gDialog.sampleControlArray[i]);
//      gDialog.sampleControlArray[i].sizeToContent();
//      bRedraw = true;
    }
  }

//  if (bRedraw)
//    window.sizeToContent();
//  try
//  {
//    var requestor = window.QueryInterface(Components.interfaces.nsIInterfaceRequestor);
//    if (!requestor && window.parent)
//      requestor = window.parent.QueryInterface(Components.interfaces.nsIInterfaceRequestor);
//    var webnavigation=requestor.getInterface(Components.interfaces.nsIWebNavigation);
//    var basewindow=webnavigation.QueryInterface(Components.interfaces.nsIBaseWindow);
//    basewindow.repaint(true);
//  }
//  catch(exc) {AlertWithTitle("Error", "Unable to get nsIBaseWindow interface in checkRedrawSample()");}
}

function findInArray(theArray, theItem)
{
  var theIndex = -1;
  for (var i = 0; i < theArray.length; ++i)
  {
    if (theArray[i] == theItem)
    {
      theIndex = i;
      break;
    }
  }
  return theIndex;
}

//The purpose of this function is to remove elements of "theArray" which aren't in "otherArray". In particular,
//it leaves the order of elements in "theArray" unchanged.
function intersectArrayWith(theArray, otherArray)
{
  for (var i = theArray.length - 1; i >= 0; --i)
  {
    if (findInArray(otherArray, theArray[i]) < 0)
      theArray.splice(i, 1);
  }
  return theArray;
}

//The purpose of this function is to add elements of "otherArray" which aren't in "theArray". In particular,
//it leaves the order of elements in "theArray" unchanged.
function unionArrayWith(theArray, otherArray)
{
  for (var i = 0; i < otherArray.length; ++i)
  {
    if (findInArray(theArray, otherArray[i]) < 0)
      theArray.push(otherArray[i]);
  }
  return theArray;
}


function enableControlsByID(theControls, bEnable)
{
  var theControl = null;
  for (var ix = 0; ix < theControls.length; ++ix)
  {
    theControl = document.getElementById(theControls[ix]);
    if (theControl != null)
    {
      if (!bEnable)
        theControl.disabled = true;
      else if (bEnable && theControl.disabled)
        theControl.disabled = false;
    }
  }
}