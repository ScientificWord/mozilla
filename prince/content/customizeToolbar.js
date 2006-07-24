// Copyright (c) 2004 MacKichan Software, Inc.  All Rights Reserved.

//const kRowMax = 7;
const kWindowWidth = 600;
const kWindowHeight = 400;
const kAnimateIncrement = 50;
const kAnimateSteps = kWindowHeight / kAnimateIncrement - 1;

var gToolboxDocument = null;
var gToolbox = null;
var gToolbars = null;
var gCurrentDragOverItem = null;
var gToolboxChanged = false;

function onLoad()
{
  gToolbox = window.arguments[0];
  if (gToolbox) gToolbars = gToolbox.getElementsByTagName("toolbar");
  gToolboxDocument = gToolbox.ownerDocument;
  
  gToolbox.addEventListener("draggesture", onToolbarDragGesture, false);
  gToolbox.addEventListener("dragover", onToolbarDragOver, false);
  gToolbox.addEventListener("dragexit", onToolbarDragExit, false);
  gToolbox.addEventListener("dragdrop", onToolbarDragDrop, false);

  document.documentElement.setAttribute("hidechrome", "true");

  repositionDialog();
  window.outerWidth = kWindowWidth;
  window.outerHeight = 50;
  slideOpen(0);
}

function onUnload(aEvent)
{
  removeToolboxListeners();
  unwrapToolbarItems();
  persistCurrentSets();
  
  notifyParentComplete();
}

function onAccept(aEvent)
{
  document.getElementById("main-box").collapsed = true;
  slideClosed(0);
}

function initDialog()
{
  document.getElementById("main-box").collapsed = false;
  
  var mode = gToolbox.getAttribute("mode");
  document.getElementById("modelist").value = mode;
  var iconSize = gToolbox.getAttribute("iconsize");
  var smallIconsCheckbox = document.getElementById("smallicons");
  if (smallIconsCheckbox)
  {
    if ( mode == "text")
      smallIconsCheckbox.disabled = true;
    else
      smallIconsCheckbox.checked = iconSize == "small"; 
  }

  // Build up the palette of other items.
  buildPalette();

  // Wrap all the items on the toolbar in toolbarpaletteitems.
  wrapToolbarItems();
}

function slideOpen(aStep)
{
  if (aStep < kAnimateSteps) {
    window.outerHeight += kAnimateIncrement;
    setTimeout(slideOpen, 20, ++aStep);
  } else {
    initDialog();
  }
}

function slideClosed(aStep)
{
  if (aStep < kAnimateSteps) {
    window.outerHeight -= kAnimateIncrement;
    setTimeout(slideClosed, 10, ++aStep);
  } else {
    window.close();
  }
}

function repositionDialog()
{
  // Position the dialog touching the bottom of the toolbox and centered with it
  var screenX = gToolbox.boxObject.screenX + ((gToolbox.boxObject.width - kWindowWidth) / 2);
  var screenY = gToolbox.boxObject.screenY + gToolbox.boxObject.height;
  window.moveTo(screenX, screenY);
}

function removeToolboxListeners()
{
  gToolbox.removeEventListener("draggesture", onToolbarDragGesture, false);
  gToolbox.removeEventListener("dragover", onToolbarDragOver, false);
  gToolbox.removeEventListener("dragexit", onToolbarDragExit, false);
  gToolbox.removeEventListener("dragdrop", onToolbarDragDrop, false);
}

/**
 * Invoke a callback on the toolbox to notify it that the dialog is done
 * and going away.
 */
function notifyParentComplete()
{
  if ("customizeDone" in gToolbox)
    gToolbox.customizeDone(gToolboxChanged);
}

function getToolbarAt(i)
{
  return gToolbars[i];
}

/**
 * Persist the current set of buttons in all customizable toolbars to
 * localstore.
 */
function persistCurrentSets()
{
  if (!gToolboxChanged)
    return;

  var customCount = 0;
  for (var i = 0; i < gToolbars.length; ++i) {
    // Look for customizable toolbars that need to be persisted.
    var toolbar = getToolbarAt(i);
    if (isCustomizableToolbar(toolbar)) {
      // Calculate currentset and store it in the attribute.
      var currentSet = toolbar.currentSet;
      toolbar.setAttribute("currentset", currentSet);
      
      var customIndex = toolbar.hasAttribute("customindex");
      if (customIndex) {
        if (!toolbar.firstChild) {
          // Remove custom toolbars whose contents have been removed.
          gToolbox.removeChild(toolbar);
          --i;
        } else {
          // Persist custom toolbar info on the <toolbarset/>
          gToolbox.toolbarset.setAttribute("toolbar"+(++customCount),
                                           toolbar.toolbarName + ":" + currentSet);
          gToolboxDocument.persist(gToolbox.toolbarset.id, "toolbar"+customCount);
        }
      }

      if (!customIndex) {
        // Persist the currentset attribute directly on hardcoded toolbars.
        gToolboxDocument.persist(toolbar.id, "currentset");
      }
    }
  }
  
  // Remove toolbarX attributes for removed toolbars.
  while (gToolbox.toolbarset.hasAttribute("toolbar"+(++customCount))) {
    gToolbox.toolbarset.removeAttribute("toolbar"+customCount);
    gToolboxDocument.persist(gToolbox.toolbarset.id, "toolbar"+customCount);
  }
}

/**
 * Wraps all items in all customizable toolbars in a toolbox.
 */
function wrapToolbarItems()
{
  for (var i = 0; i < gToolbars.length; ++i) {
    var toolbar = getToolbarAt(i);
    if (isCustomizableToolbar(toolbar)) {
      for (var k = 0; k < toolbar.childNodes.length; ++k) {
        var item = toolbar.childNodes[k];
        if (isToolbarItem(item)) {
          var nextSibling = item.nextSibling;
          
          var wrapper = wrapToolbarItem(item);     
          if (nextSibling)
            toolbar.insertBefore(wrapper, nextSibling);
          else
            toolbar.appendChild(wrapper);
        }
      }
    }
  }
}

/**
 * Unwraps all items in all customizable toolbars in a toolbox.
 */
 function unwrapToolbarItems()
{
  for (var i = 0; i < gToolbars.length; ++i) {
    var toolbar = getToolbarAt(i);
    if (isCustomizableToolbar(toolbar)) {
      for (var k = 0; k < toolbar.childNodes.length; ++k) {
        var paletteItem = toolbar.childNodes[k];
        var toolbarItem = paletteItem.firstChild;

        if (isToolbarItem(toolbarItem)) {
          var nextSibling = paletteItem.nextSibling;

          if (paletteItem.hasAttribute("itemcommand"))
            toolbarItem.setAttribute("command", paletteItem.getAttribute("itemcommand"));
          if (paletteItem.hasAttribute("itemobserves"))
            toolbarItem.setAttribute("observes", paletteItem.getAttribute("itemobserves"));

          paletteItem.removeChild(toolbarItem); 
          paletteItem.parentNode.removeChild(paletteItem);

          if (nextSibling)
            toolbar.insertBefore(toolbarItem, nextSibling);
          else
            toolbar.appendChild(toolbarItem);
        }
      }
    }
  }
}


/**
 * Creates a wrapper that can be used to contain a toolbaritem and prevent
 * it from receiving UI events.
 */
function createWrapper(aId)
{
  var wrapper = document.createElementNS("http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul",
                                         "toolbarpaletteitem");

  wrapper.id = "wrapper-"+aId;  
  return wrapper;
}

/**
 * Wraps an item that has been cloned from a template and adds
 * it to the end of a row in the palette.
 */
function wrapPaletteItem(aPaletteItem, aCurrentRow, aSpacer)
{
  var wrapper = createWrapper(aPaletteItem.id);

  wrapper.setAttribute("flex", 1);
  wrapper.setAttribute("align", "center");
  wrapper.setAttribute("pack", "center");
  wrapper.setAttribute("minheight", "0");
  wrapper.setAttribute("minwidth", "0");

  wrapper.appendChild(aPaletteItem);
  
  // XXX We need to call this AFTER the palette item has been appended
  // to the wrapper or else we crash dropping certain buttons on the 
  // palette due to removal of the command and disabled attributes - JRH
  cleanUpItemForPalette(aPaletteItem, wrapper);

  if (aSpacer)
    aCurrentRow.insertBefore(wrapper, aSpacer);
  else
    aCurrentRow.appendChild(wrapper);

}

/**
 * Wraps an item that is currently on a toolbar and replaces the item
 * with the wrapper. This is not used when dropping items from the palette,
 * only when first starting the dialog and wrapping everything on the toolbars.
 */
function wrapToolbarItem(aToolbarItem)
{
  var wrapper = createWrapper(aToolbarItem.id);
  
  cleanupItemForToolbar(aToolbarItem, wrapper);
  wrapper.flex = aToolbarItem.flex;

  if (aToolbarItem.parentNode)
    aToolbarItem.parentNode.removeChild(aToolbarItem);
  
  wrapper.appendChild(aToolbarItem);
  
  return wrapper;
}

/**
 * Get the list of ids for the current set of items on each toolbar.
 */
function getCurrentItemIds()
{
  var currentItems = {};
  for (var i = 0; i < gToolbars.length; ++i) {
    var toolbar = getToolbarAt(i);
    if (isCustomizableToolbar(toolbar)) {
      var child = toolbar.firstChild;
      while (child) {
        if (isToolbarItem(child))
          currentItems[child.id] = 1;
        child = child.nextSibling;
      }
    }
  }
  return currentItems;
}

/**
 * Builds the palette of draggable items that are not yet in a toolbar.
 */
function buildPalette()
{
  // Empty the palette first.
  var paletteBox = document.getElementById("palette-box");
  while (paletteBox.lastChild)
    paletteBox.removeChild(paletteBox.lastChild);

  var currentRow = document.createElementNS("http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul",
                                            "hbox");
  currentRow.setAttribute("class", "paletteRow");

  // Add the toolbar separator item.
  var templateNode = document.createElementNS("http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul",
                                              "toolbarseparator");
  templateNode.id = "separator";
  wrapPaletteItem(templateNode, currentRow, null);

  // Add the toolbar spring item.
  templateNode = document.createElementNS("http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul",
                                              "toolbarspring");
  templateNode.id = "spring";
  templateNode.flex = 1;
  wrapPaletteItem(templateNode, currentRow, null);

  // Add the toolbar spacer item.
  templateNode = document.createElementNS("http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul",
                                              "toolbarspacer");
  templateNode.id = "spacer";
  templateNode.flex = 1;
  wrapPaletteItem(templateNode, currentRow, null);

  var rowSlot = 3;

  var currentItems = getCurrentItemIds();
  var toolbox;
  var i;
  for (i = 0; i < window.arguments.length; i++)
  {
    var toolbox = window.arguments[i];
    if (!toolbox) break;
    templateNode = toolbox.palette.firstChild;
    while (templateNode) {
      // Check if the item is already in a toolbar before adding it to the palette.
      if (!(templateNode.id in currentItems)) {
        var paletteItem = templateNode.cloneNode(true);

        if (rowSlot == kRowMax) {
          // Append the old row.
          paletteBox.appendChild(currentRow);

          // Make a new row.
          currentRow = document.createElementNS("http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul",
                                                "hbox");
          currentRow.setAttribute("class", "paletteRow");
          rowSlot = 0;
        }

        ++rowSlot;
        wrapPaletteItem(paletteItem, currentRow, null);
      }
    
      templateNode = templateNode.nextSibling;
    }

    if (currentRow) { 
      fillRowWithFlex(currentRow);
      paletteBox.appendChild(currentRow);
    }
  }
}

/**
 * Creates a new palette item for a cloned template node and
 * adds it to the last slot in the palette.
 */
function appendPaletteItem(aItem)
{
  var paletteBox = document.getElementById("palette-box");
  var lastRow = paletteBox.lastChild;
  var lastSpacer = lastRow.lastChild;
   
  if (lastSpacer.localName != "spacer") {
    // The current row is full, so we have to create a new row.
    lastRow = document.createElementNS("http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul",
                                        "hbox");
    lastRow.setAttribute("class", "paletteRow");
    paletteBox.appendChild(lastRow);
    
    wrapPaletteItem(aItem, lastRow, null);

    fillRowWithFlex(lastRow);
  } else {
    // Decrement the flex of the last spacer or remove it entirely.
    var flex = lastSpacer.getAttribute("flex");
    if (flex == 1) {
      lastRow.removeChild(lastSpacer);
      lastSpacer = null;
    } else
      lastSpacer.setAttribute("flex", --flex);

    // Insert the wrapper where the last spacer was.
    wrapPaletteItem(aItem, lastRow, lastSpacer);
  }
}

function fillRowWithFlex(aRow)
{
  var remainingFlex = kRowMax - aRow.childNodes.length;
  if (remainingFlex > 0) {
    var spacer = document.createElementNS("http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul",
                                          "spacer");
    spacer.setAttribute("flex", remainingFlex);
    aRow.appendChild(spacer);
  }
}

/**
 * Makes sure that an item that has been cloned from a template
 * is stripped of all properties that may adversely affect it's
 * appearance in the palette.
 */
function cleanUpItemForPalette(aItem, aWrapper)
{
  aWrapper.setAttribute("place", "palette");
  setWrapperType(aItem, aWrapper);

  if (aItem.hasAttribute("title"))
    aWrapper.setAttribute("title", aItem.getAttribute("title"));
  else if (isSpecialItem(aItem)) {
    var stringBundle = document.getElementById("stringBundle");
    var title = stringBundle.getString(aItem.id + "Title");
    aWrapper.setAttribute("title", title);
  }
  
  // Remove attributes that screw up our appearance.
  aItem.removeAttribute("command");
  aItem.removeAttribute("observes");
  aItem.removeAttribute("disabled");
  aItem.removeAttribute("type");
  
  if (aItem.localName == "toolbaritem" && aItem.firstChild) {
    aItem.firstChild.removeAttribute("observes");

    // So the throbber doesn't throb in the dialog,
    // cute as that may be...
    aItem.firstChild.removeAttribute("busy");
  }
}

/**
 * Makes sure that an item that has been cloned from a template
 * is stripped of all properties that may adversely affect it's
 * appearance in the toolbar.  Store critical properties on the 
 * wrapper so they can be put back on the item when we're done.
 */
function cleanupItemForToolbar(aItem, aWrapper)
{
  setWrapperType(aItem, aWrapper);
  aWrapper.setAttribute("place", "toolbar");

  if (aItem.hasAttribute("command")) {
    aWrapper.setAttribute("itemcommand", aItem.getAttribute("command"));
    aItem.removeAttribute("command");
  }

  if (aItem.hasAttribute("observes"))
  {
    aWrapper.setAttribute("itemobserves", aItem.getAttribute("observes"));
    aItem.removeAttribute("observes");
  }

  if (aItem.disabled) {
    aWrapper.setAttribute("itemdisabled", "true");
    aItem.disabled = false;
  }
}

function setWrapperType(aItem, aWrapper)
{
  if (aItem.localName == "toolbarseparator") {
    aWrapper.setAttribute("type", "separator");
  } else if (aItem.localName == "toolbarspring") {
    aWrapper.setAttribute("type", "spring");
  } else if (aItem.localName == "toolbarspacer") {
    aWrapper.setAttribute("type", "spacer");
  } else if (aItem.localName == "toolbaritem" && aItem.firstChild) {
    aWrapper.setAttribute("type", aItem.firstChild.localName);
  }
}

function setDragActive(aItem, aValue)
{
  var node = aItem;
  var value = "left";
  if (aItem.localName == "toolbar") {
    node = aItem.lastChild;
    value = "right";
  }
  
  if (!node)
    return;
  
  if (aValue) {
    if (!node.hasAttribute("dragover"))
      node.setAttribute("dragover", value);
  } else {
    node.removeAttribute("dragover");
  }
}


/**
 * Restore the default set of buttons to fixed toolbars,
 * remove all custom toolbars, and rebuild the palette.
 */
function restoreDefaultSet()
{
  // Restore the defaultset for fixed toolbars.
  var toolbar = gToolbox.firstChild;
  while (toolbar) {
    if (isCustomizableToolbar(toolbar)) {
      if (!toolbar.hasAttribute("customindex")) {
        var defaultSet = toolbar.getAttribute("defaultset");
        if (defaultSet)
        {
          toolbar.currentSet = defaultSet;
        }
      }
    }
    toolbar = toolbar.nextSibling;
  }

  // Remove all of the customized toolbars.
  var child = gToolbox.lastChild;
  while (child) {
    if (child.hasAttribute("customindex")) {
      var thisChild = child;
      child = child.previousSibling;
      gToolbox.removeChild(thisChild);
    } else {
      child = child.previousSibling;
    }
  }
  
  // Now rebuild the palette.
  buildPalette();

  // Now re-wrap the items on the toolbar.
  wrapToolbarItems();

  repositionDialog();
  gToolboxChanged = true;
}

function updateIconSize(aUseSmallIcons)
{
  var val = aUseSmallIcons ? "small" : null;
  
  setAttribute(gToolbox, "iconsize", val);
  gToolboxDocument.persist(gToolbox.id, "iconsize");
  
  for (var i = 0; i < gToolbox.childNodes.length; ++i) {
    var toolbar = getToolbarAt(i);
    if (isCustomizableToolbar(toolbar)) {
      setAttribute(toolbar, "iconsize", val);
      gToolboxDocument.persist(toolbar.id, "iconsize");
    }
  }

  repositionDialog();
}

function updateToolbarMode(aModeValue)
{
  setAttribute(gToolbox, "mode", aModeValue);
  gToolboxDocument.persist(gToolbox.id, "mode");

  for (var i = 0; i < gToolbox.childNodes.length; ++i) {
    var toolbar = getToolbarAt(i);
    if (isCustomizableToolbar(toolbar)) {
      setAttribute(toolbar, "mode", aModeValue);
      gToolboxDocument.persist(toolbar.id, "mode");
    }
  }

  var iconSizeCheckbox = document.getElementById("smallicons");
  if (iconSizeCheckbox)
  {
    if (aModeValue == "text") {
      iconSizeCheckbox.disabled = true;
      iconSizeCheckbox.checked = false;
      updateIconSize(false);
    }
    else {
      iconSizeCheckbox.disabled = false;
    }
  }

  repositionDialog();
}


function setAttribute(aElt, aAttr, aVal)
{
 if (aVal)
    aElt.setAttribute(aAttr, aVal);
  else
    aElt.removeAttribute(aAttr);
}

function isCustomizableToolbar(aElt)
{
  return aElt.localName == "toolbar" &&
         aElt.getAttribute("customizable") == "true";
}

function isSpecialItem(aElt)
{
  return aElt.localName == "toolbarseparator" ||
         aElt.localName == "toolbarspring" ||
         aElt.localName == "toolbarspacer";
}

function isToolbarItem(aElt)
{
  return aElt.localName == "toolbarbutton" ||
         aElt.localName == "toolbaritem" ||
         aElt.localName == "toolbarseparator" ||
         aElt.localName == "toolbarspring" ||
         aElt.localName == "toolbarspacer" ||
         aElt.localName == "textbox";
}

///////////////////////////////////////////////////////////////////////////
//// Drag and Drop observers

function onToolbarDragGesture(aEvent)
{
  nsDragAndDrop.startDrag(aEvent, dragStartObserver);
}

function onToolbarDragOver(aEvent)
{
  nsDragAndDrop.dragOver(aEvent, toolbarDNDObserver);
}

function onToolbarDragDrop(aEvent)
{
  nsDragAndDrop.drop(aEvent, toolbarDNDObserver);
}

function onToolbarDragExit(aEvent)
{
  if (gCurrentDragOverItem)
    setDragActive(gCurrentDragOverItem, false);
}

var dragStartObserver =
{
  onDragStart: function (aEvent, aXferData, aDragAction) {
    var documentId = gToolboxDocument.documentElement.id;
    
    var item = aEvent.target;
    while (item && item.localName != "toolbarpaletteitem")
      item = item.parentNode;
    
    item.setAttribute("dragactive", "true");
    
    aXferData.data = new TransferDataSet();
    var data = new TransferData();
    data.addDataForFlavour("text/toolbarwrapper-id/"+documentId, item.firstChild.id);
    aXferData.data.push(data);
  }
}

var toolbarDNDObserver =
{
  onDragOver: function (aEvent, aFlavour, aDragSession)
  {
    var toolbar = aEvent.target;
    var dropTarget = aEvent.target;
    while (toolbar && toolbar.localName != "toolbar") {
      dropTarget = toolbar;
      toolbar = toolbar.parentNode;
    }
    
    var previousDragItem = gCurrentDragOverItem;

    // Make sure we are dragging over a customizable toolbar.
    if (!isCustomizableToolbar(toolbar)) {
      gCurrentDragOverItem = null;
      return;
    }
    
    if (dropTarget.localName == "toolbar") {
      gCurrentDragOverItem = dropTarget;
    } else {
      var dropTargetWidth = dropTarget.boxObject.width;
      var dropTargetX = dropTarget.boxObject.x;

      gCurrentDragOverItem = null;
      if (aEvent.clientX > (dropTargetX + (dropTargetWidth / 2))) {
        gCurrentDragOverItem = dropTarget.nextSibling;
        if (!gCurrentDragOverItem)
          gCurrentDragOverItem = toolbar;
      } else
        gCurrentDragOverItem = dropTarget;
    }    

    if (previousDragItem && gCurrentDragOverItem != previousDragItem) {
      setDragActive(previousDragItem, false);
    }
    
    setDragActive(gCurrentDragOverItem, true);
    
    aDragSession.canDrop = true;
  },
  
  onDrop: function (aEvent, aXferData, aDragSession)
  {
    if (!gCurrentDragOverItem)
      return;
    
    setDragActive(gCurrentDragOverItem, false);

    var draggedItemId = aXferData.data;
    if (gCurrentDragOverItem.id == draggedItemId)
      return;

    var toolbar = aEvent.target;
    while (toolbar.localName != "toolbar")
      toolbar = toolbar.parentNode;

    var draggedPaletteWrapper = document.getElementById("wrapper-"+draggedItemId);       
    if (!draggedPaletteWrapper) {
      // The wrapper has been dragged from the toolbar.
      
      // Get the wrapper from the toolbar document and make sure that
      // it isn't being dropped on itself.
      var wrapper = gToolboxDocument.getElementById("wrapper-"+draggedItemId);
      if (wrapper == gCurrentDragOverItem)
        return;

      // Don't allow static kids (e.g., the menubar) to move.
      if (wrapper.parentNode.firstPermanentChild && wrapper.parentNode.firstPermanentChild.id == wrapper.firstChild.id)
        return;
      if (wrapper.parentNode.lastPermanentChild && wrapper.parentNode.lastPermanentChild.id == wrapper.firstChild.id)
        return;

      // Remove the item from it's place in the toolbar.
      wrapper.parentNode.removeChild(wrapper);

      // Determine which toolbar we are dropping on.
      var dropToolbar = null;
      if (gCurrentDragOverItem.localName == "toolbar")
        dropToolbar = gCurrentDragOverItem;
      else
        dropToolbar = gCurrentDragOverItem.parentNode;
      
      // Insert the item into the toolbar.
      if (gCurrentDragOverItem != dropToolbar)
        dropToolbar.insertBefore(wrapper, gCurrentDragOverItem);
      else
        dropToolbar.appendChild(wrapper);
    } else {
      // The item has been dragged from the palette
      
      // Create a new wrapper for the item. We don't know the id yet.
      var wrapper = createWrapper("");

      // Ask the toolbar to clone the item's template, place it inside the wrapper, and insert it in the toolbar.
      var newItem = toolbar.insertItem(draggedItemId, gCurrentDragOverItem == toolbar ? null : gCurrentDragOverItem, wrapper);
      
      // Prepare the item and wrapper to look good on the toolbar.
      cleanupItemForToolbar(newItem, wrapper);
      wrapper.id = "wrapper-"+newItem.id;
      wrapper.flex = newItem.flex;

      // Remove the wrapper from the palette.
      var currentRow = draggedPaletteWrapper.parentNode;
      if (draggedItemId != "separator" &&
          draggedItemId != "spring" &&
          draggedItemId != "spacer")
      {
        currentRow.removeChild(draggedPaletteWrapper);

        while (currentRow) {
          // Pull the first child of the next row up
          // into this row.
          var nextRow = currentRow.nextSibling;
          
          if (!nextRow) {
            var last = currentRow.lastChild;
            var first = currentRow.firstChild;
            if (first == last) {
              // Kill the row.
              currentRow.parentNode.removeChild(currentRow);
              break;
            }

            if (last.localName == "spacer") {
              var flex = last.getAttribute("flex");
              last.setAttribute("flex", ++flex);
              // Reflow doesn't happen for some reason.  Trigger it with a hide/show. ICK! -dwh
              last.hidden = true;
              last.hidden = false;
              break;
            } else {
              // Make a spacer and give it a flex of 1.
              var spacer = document.createElementNS("http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul",
                                                    "spacer");
              spacer.setAttribute("flex", "1");
              currentRow.appendChild(spacer);
            }
            break;
          }
          
          currentRow.appendChild(nextRow.firstChild);
          currentRow = currentRow.nextSibling;
        }
      }
    }
    
    gCurrentDragOverItem = null;

    repositionDialog();
    gToolboxChanged = true;
  },
  
  _flavourSet: null,
  
  getSupportedFlavours: function ()
  {
    if (!this._flavourSet) {
      this._flavourSet = new FlavourSet();
      var documentId = gToolboxDocument.documentElement.id;
      this._flavourSet.appendFlavour("text/toolbarwrapper-id/"+documentId);
    }
    return this._flavourSet;
  }
}

var paletteDNDObserver =
{
  onDragOver: function (aEvent, aFlavour, aDragSession)
  {
    aDragSession.canDrop = true;
  },
  
  onDrop: function(aEvent, aXferData, aDragSession)
  {
    var itemId = aXferData.data;
    
    var wrapper = gToolboxDocument.getElementById("wrapper-"+itemId);
    if (wrapper) {
      // Don't allow static kids (e.g., the menubar) to move.
      if (wrapper.parentNode.firstPermanentChild && wrapper.parentNode.firstPermanentChild.id == wrapper.firstChild.id)
        return;
      if (wrapper.parentNode.lastPermanentChild && wrapper.parentNode.lastPermanentChild.id == wrapper.firstChild.id)
        return;

      // The item was dragged out of the toolbar.
      wrapper.parentNode.removeChild(wrapper);
      
      var wrapperType = wrapper.getAttribute("type");
      if (wrapperType != "separator" && wrapperType != "spacer" && wrapperType != "spring") {
        // Find the template node in the toolbox palette
        var templateNode = gToolbox.palette.firstChild;
        while (templateNode) {
          if (templateNode.id == itemId)
            break;
          templateNode = templateNode.nextSibling;
        }
        if (!templateNode)
          return;
        
        // Clone the template and add it to our palette.
        var paletteItem = templateNode.cloneNode(true);
        appendPaletteItem(paletteItem);
      }
    }
    
    repositionDialog();
    gToolboxChanged = true;
  },
  
  _flavourSet: null,
  
  getSupportedFlavours: function ()
  {
    if (!this._flavourSet) {
      this._flavourSet = new FlavourSet();
      var documentId = gToolboxDocument.documentElement.id;
      this._flavourSet.appendFlavour("text/toolbarwrapper-id/"+documentId);
    }
    return this._flavourSet;
  }
}

