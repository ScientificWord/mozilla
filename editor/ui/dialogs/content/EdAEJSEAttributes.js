/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Mozilla Communicator client code, released
 * March 31, 1998.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998-1999
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Ben "Count XULula" Goodger
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either of the GNU General Public License Version 2 or later (the "GPL"),
 * or the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

function BuildJSEAttributeNameList()
{
  gDialog.AddJSEAttributeNameList.removeAllItems();
  
  // Get events specific to current element
  var elementName = gElement.localName.toLowerCase();
  if (elementName in gJSAttr)
  {
    var attNames = gJSAttr[elementName];
    var i;
    var popup;
    var sep;

    if (attNames && attNames.length)
    {
      // Since we don't allow user-editable JS events yet (but we will soon)
      //  simply remove the JS tab to not allow adding JS events
      if (attNames[0] == "noJSEvents")
      {
        var tab = document.getElementById("tabJSE");
        if (tab)
          tab.parentNode.removeChild(tab);

        return;
      }

      for (i = 0; i < attNames.length; i++)
        gDialog.AddJSEAttributeNameList.appendItem(attNames[i], attNames[i]);

      popup = gDialog.AddJSEAttributeNameList.firstChild;
      if (popup)
      {
        sep = document.createElementNS(XUL_NS, "menuseparator");
        if (sep)
          popup.appendChild(sep);
      }        
    }
  }

  // Always add core JS events unless we aborted above
  for (i = 0; i < gCoreJSEvents.length; i++)
  {
    if (gCoreJSEvents[i] == "-")
    {
      if (!popup)
        popup = gDialog.AddJSEAttributeNameList.firstChild;

      sep = document.createElementNS(XUL_NS, "menuseparator");

      if (popup && sep)
        popup.appendChild(sep);
    }
    else
      gDialog.AddJSEAttributeNameList.appendItem(gCoreJSEvents[i], gCoreJSEvents[i]);
  }
  
  gDialog.AddJSEAttributeNameList.selectedIndex = 0;

  // Use current name and value of first tree item if it exists
  onSelectJSETreeItem();
}

// build attribute list in tree form from element attributes
function BuildJSEAttributeTable()
{
  var nodeMap = gElement.attributes;
  if (nodeMap.length > 0)
  {
    var added = false;
    for (var i = 0; i < nodeMap.length; i++)
    {
      if( CheckAttributeNameSimilarity( nodeMap[i].nodeName, JSEAttrs ) )
        continue;   // repeated or non-JS handler, ignore this one and go to next
      if( !IsEventHandler( nodeMap[i].nodeName ) )
        continue; // attribute isn't an event handler.
      var name  = nodeMap[i].nodeName.toLowerCase();
      var value = gElement.getAttribute(nodeMap[i].nodeName);
      if (AddTreeItem( name, value, "JSEAList", JSEAttrs )) // add item to tree
        added = true;
    }

    // Select first item
    if (added)
      gDialog.AddJSEAttributeTree.selectedIndex = 0;
  }
}

// check to see if given string is an event handler.
function IsEventHandler( which )
{
  var handlerName = which.toLowerCase();
  var firstTwo = handlerName.substring(0,2);
  if (firstTwo == "on")
    return true;
  else
    return false;
}

function onSelectJSEAttribute()
{
  if(!gDoOnSelectTree)
    return;

  gDialog.AddJSEAttributeValueInput.value = 
      GetAndSelectExistingAttributeValue(gDialog.AddJSEAttributeNameList.label, "JSEAList");
}

function onSelectJSETreeItem()
{
  var tree = gDialog.AddJSEAttributeTree;
  if (tree && tree.view.selection.count)
  {
    // Select attribute name in list
    gDialog.AddJSEAttributeNameList.value = GetTreeItemAttributeStr(getSelectedItem(tree));

    // Set value input to that in tree (no need to update this in the tree)
    gUpdateTreeValue = false;
    gDialog.AddJSEAttributeValueInput.value =  GetTreeItemValueStr(getSelectedItem(tree));
    gUpdateTreeValue = true;
  }
}

function onInputJSEAttributeValue()
{
  if (gUpdateTreeValue)
  {

    var name = TrimString(gDialog.AddJSEAttributeNameList.label);
    var value = TrimString(gDialog.AddJSEAttributeValueInput.value);

    // Update value in the tree list
    // Since we have a non-editable menulist, 
    //   we MUST automatically add the event attribute if it doesn't exist
    if (!UpdateExistingAttribute( name, value, "JSEAList" ) && value)
      AddTreeItem( name, value, "JSEAList", JSEAttrs );
  }
}

function editJSEAttributeValue(targetCell)
{
  if (IsNotTreeHeader(targetCell))
    gDialog.AddJSEAttributeValueInput.inputField.select();
}

function UpdateJSEAttributes()
{
  var JSEAList = document.getElementById("JSEAList");
  var i;

  // remove removed attributes
  for (i = 0; i < JSERAttrs.length; i++)
  {
    var name = JSERAttrs[i];

    if (gElement.hasAttribute(name))
      doRemoveAttribute(name);
  }

  // Add events
  for (i = 0; i < JSEAList.childNodes.length; i++)
  {
    var item = JSEAList.childNodes[i];

    // set the event handler
    doSetAttribute( GetTreeItemAttributeStr(item), GetTreeItemValueStr(item) );
  }
}

function RemoveJSEAttribute()
{
  var treechildren = gDialog.AddJSEAttributeTree.lastChild;

  // This differs from HTML and CSS panels: 
  // We reselect after removing, because there is not
  //  editable attribute name input, so we can't clear that
  //  like we do in other panels
  var newIndex = gDialog.AddJSEAttributeTree.selectedIndex;

  // We only allow 1 selected item
  if (gDialog.AddJSEAttributeTree.view.selection.count)
  {
    var item = getSelectedItem(gDialog.AddJSEAttributeTree);

    // Name is the text of the treecell
    var attr = GetTreeItemAttributeStr(item);

    // remove the item from the attribute array
    if (newIndex >= (JSEAttrs.length-1))
      newIndex--;

    // remove the item from the attribute array
    JSERAttrs[JSERAttrs.length] = attr;
    RemoveNameFromAttArray(attr, JSEAttrs);

    // Remove the item from the tree
    treechildren.removeChild (item);

    // Reselect an item
    gDialog.AddJSEAttributeTree.selectedIndex = newIndex;
  }
}
