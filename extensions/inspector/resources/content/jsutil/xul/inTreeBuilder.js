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
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2001
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Joe Hewitt <hewitt@netscape.com> (original author)
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
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

/***************************************************************
* inTreeBuilder -------------------------------------------------
*  Automatically builds up an tree so that it will display a tabular
*  set of data with titled columns and optionally an icon for each row.
*  The tree that is supplied must have an treechildren with an 
*  empty template inside of it.
* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
* REQUIRED IMPORTS:
*   chrome://inspector/content/jsutil/xul/DNDUtils.js
****************************************************************/

//////////// global variables /////////////////////

var inTreeBuilderPartCount = 0;

//////////// global constants ////////////////////

////////////////////////////////////////////////////////////////////////////
//// class inTreeBuilder

function inTreeBuilder(aTree, aNameSpace, aArcName)
{
  this.tree = aTree;
  this.nameSpace = aNameSpace;
  this.arcName = aArcName;

  this.mColumns = [];
  this.mExtras = [];
}

inTreeBuilder.prototype =
{
  mTree: null,
  mTreeBody: null,
  // datasource stuff
  mNameSpace: null,
  mArcName: null,
  mIsRefContainer: false,
  mIsContainer: true,

  // table structure stuff
  mIsIconic: false,
  mColumns: null,
  mSplitters: true,
  mRowFields: null,
  mRowAttrs: null,
  mAllowDND: false,
  mLastDragCol: null,
  mLastDragColWhere: null,

  ////////////////////////////////////////////////////////////////////////////
  //// properties

  // the xul tree node we will construct
  get tree() { return this.mTree },
  set tree(aVal) 
  { 
    this.mTree = aVal;
    if (aVal)
      this.mTreeBody = aVal.getElementsByTagName("treechildren")[0];
    aVal._treeBuilder = this 
  },

  // the namespace to use for all fields
  get nameSpace() { return this.mNameSpace },
  set nameSpace(aVal) { this.mNameSpace = aVal },

  // the name of the arc to the container
  get arcName() { return this.mArcName },
  set arcName(aVal) { this.mArcName = aVal },

  // is the datasource ref an arc to a container, or a container itself
  get isRefContainer() { return this.mIsRefContainer },
  set isRefContainer(aVal) { this.mIsRefContainer = aVal },

  // is each row a potential container?
  get isContainer() { return this.mIsContainer },
  set isContainer(aVal) { this.mIsContainer = aVal },

  // place an icon before the first column of each row
  get isIconic() { return this.mIsIconic },
  set isIconic(aVal) { this.mIsIconic = aVal },

  // put splitters between the columns?
  get useSplitters() { return this.mSplitters },
  set useSplitters(aVal) { this.mSplitters = aVal },

  // extra attributes to set on the treeitem
  get rowAttributes() { return this.mRowAttrs },
  set rowAttributes(aVal) { this.mRowAttrs = aVal },

  // extra data fields to set on the treeitem
  get rowFields() { return this.mRowFields },
  set rowFields(aVal) { this.mRowFields = aVal },

  // extra data fields to set on the treeitem
  get allowDragColumns() { return this.mAllowDND },
  set allowDragColumns(aVal) { this.mAllowDND = aVal },

  ////////////////////////////////////////////////////////////////////////////
  //// event handlers

  get onColumnAdd() { return this.mOnColumnAdd },
  set onColumnAdd(aFn) { this.mOnColumnAdd = aFn },

  get onColumnRemove() { return this.mOnColumnRemove },
  set onColumnRemove(aFn) { this.mOnColumnRemove = aFn },

  ////////////////////////////////////////////////////////////////////////////
  //// initialization

  initialize: function()
  {
    this.initColumns();
    this.initTemplate();
  },

  initColumns: function()
  {
    this.initDND();
  },

  initTemplate: function()
  {
    var template = this.mTree.getElementsByTagNameNS(kXULNSURI, "template")[0];
    this.mTemplate = template;
    this.clearChildren(template);

    var rule = document.createElementNS(kXULNSURI, "rule");
    template.appendChild(rule);
    this.mRule = rule;

    this.createDefaultConditions();

    var bindings = document.createElementNS(kXULNSURI, "bindings");
    rule.appendChild(bindings);
    this.mBindings = bindings;

    this.createDefaultAction();
  },

  createDefaultConditions: function()
  {
    var conditions = document.createElementNS(kXULNSURI, "conditions");
    this.mRule.appendChild(conditions);

    var content = document.createElementNS(kXULNSURI, "treerow");
    content.setAttribute("uri", "?uri");
    conditions.appendChild(content);

    var triple = this.createTriple("?uri", this.mNameSpace+this.mArcName,
      this.mIsRefContainer ? "?table" : "?row");
    conditions.appendChild(triple);

    if (this.mIsRefContainer) {
      var member = this.createMember("?table", "?row");
      conditions.appendChild(member);
    }
  },

  createDefaultAction: function()
  {
    var action = document.createElementNS(kXULNSURI, "action");
    this.mRule.appendChild(action);

    var orow = this.createTemplatePart("treerow");
    orow.setAttribute("uri", "?row");
    action.appendChild(orow);
    this.mTreeRow = orow;

    // assign the item attributes
    if (this.mRowAttrs)
      for (key in this.mRowAttrs)
        orow.setAttribute(key, this.mRowAttrs[key]);
  },

  createDefaultBindings: function()
  {
    // assign the item fields
    var binding;
    if (this.mRowFields) {
      var props = "";
      for (key in this.mRowFields) {
        binding = this.createBinding(this.mRowFields[key]);
        this.mBindings.appendChild(binding);
        props += key+"-?"+this.mRowFields[key]+" ";
      }
      this.mTreeRow.setAttribute("properties", props);
    }
  },

  createTriple: function(aSubject, aPredicate, aObject)
  {
    var triple = document.createElementNS(kXULNSURI, "triple");
    triple.setAttribute("subject", aSubject);
    triple.setAttribute("predicate", aPredicate);
    triple.setAttribute("object", aObject);

    return triple;
  },

  createMember: function(aContainer, aChild)
  {
    var member = document.createElementNS(kXULNSURI, "member");
    member.setAttribute("container", aContainer);
    member.setAttribute("child", aChild);

    return member;
  },

  reset: function()
  {
    this.mColumns = [];
    this.mIsIconic = false;

    this.resetTree();
  },

  resetTree: function()
  {
    var kids = this.mTree.childNodes;
    for (var i = kids.length - 1; i >= 0; --i)
      if (kids[i].localName != "treechildren")
        this.mTree.removeChild(kids[i]);
      
    this.clearChildren(this.mBindings);
    this.clearChildren(this.mTreeRow);
    this.createDefaultBindings();
  },

  clearChildren: function(aEl)
  {
    while (aEl.hasChildNodes())
      aEl.removeChild(aEl.lastChild);
  },

  ////////////////////////////////////////////////////////////////////////////
  //// column drag and drop

  initDND: function()
  {
    if (this.mAllowDND) {
      // addEventListener doesn't work for dnd events, apparently... so we use the attributes
      this.addDNDListener(this.mTree, "ondragenter");
      this.addDNDListener(this.mTree, "ondragover");
      this.addDNDListener(this.mTree, "ondragexit");
      this.addDNDListener(this.mTree, "ondraggesture");
      this.addDNDListener(this.mTree, "ondragdrop");
    }
  },

  onDragEnter: function(aEvent)
  {
  },

  onDragOver: function(aEvent)
  {
    if (!DNDUtils.checkCanDrop("TreeBuilder/column-add"))
      return;

    var idx = this.getColumnIndexFromX(aEvent.clientX, 0.5);
    this.mColumnInsertIndex = idx;
    var col = this.getColumnAt(idx);
    
    if (idx == -1)
      this.markColumnInsert(col, "after");
    else
      this.markColumnInsert(col, "before");
  },

  onDragExit: function(aEvent)
  {
  },

  onDragDrop: function(aEvent)
  {
    this.markColumnInsert(null);
    var dragService = XPCU.getService("@mozilla.org/widget/dragservice;1", "nsIDragService");
    var dragSession = dragService.getCurrentSession();

    if (!dragSession.isDataFlavorSupported("TreeBuilder/column-add"))
      return false;

    var trans = XPCU.createInstance("@mozilla.org/widget/transferable;1", "nsITransferable");
    trans.addDataFlavor("TreeBuilder/column-add");

    dragSession.getData(trans, 0);
    var data = {};
    trans.getAnyTransferData ({}, data, {});

    var string = XPCU.QI(data.value, "nsISupportsString");

    this.insertColumn(this.mColumnInsertIndex,
      {
        name: string.data,
        title: string.data,
        flex: 1
      });
    
    
    // if we rebuildContent during this thread, it will crash in the dnd service
    setTimeout(function(me) { me.build(); me.buildContent() }, 1, this);

    // bug 56270 - dragSession.sourceDocument is null --
    // causes me to code this very temporary, very nasty hack
    // to tell columnsDialog.js about the drop
    if (this.mTree.onClientDrop) {
      this.mTree.onClientDrop();
    }
  },

  markColumnInsert: function (aColumn, aWhere)
  {
    var col = this.mLastDragCol;
    var lastWhere = this.mLastDragColWhere;
    if (col)
      col.setAttribute("properties", "");

    if (aWhere != "before" && aWhere != "after") {
      this.mLastDragCol = null;
      this.mLastDragColWhere = null;
      return;
    }

    col = aColumn;
    if (col) {
      this.mLastDragCol = col;
      this.mLastDragColWhere = aWhere;
      col.setAttribute("properties", "dnd-insert-"+aWhere);
    }
    
    var bx = this.mTree.boxObject.QueryInterface(Components.interfaces.nsITreeBoxObject);
    bx.invalidate();
  },

  getColumnIndexFromX: function(aX, aThresh)
  {
    var width = 0;
    var col, cw;
    for (var i = 0; i < this.columnCount; ++i) {
      col = this.getColumnAt(i);
      cw = col.boxObject.width;
      width += cw;
      if (width-(cw*aThresh) > aX)
        return i;
    }

    return -1;
  },

  getColumnIndexFromHeader: function(aHeader)
  {
    var headers = this.mTree.getElementsByTagName("treecol");
    for (var i = 0; i < headers.length; ++i) {
      if (headers[i] == aHeader)
        return i;
    }
    return -1;
  },

  //// for drag-n-drop removal/arrangement of columns

  onDragGesture: function(aEvent)
  {
    var target = aEvent.target;
    if (target.parentNode == this.mTree) {
      var column = target.getAttribute("label");

      var idx = this.getColumnIndexFromHeader(target);
      if (idx == -1) return;
      this.mColumnDragging = idx;

      DNDUtils.invokeSession(target, ["TreeBuilder/column-remove"], [column]);
    }
  },

  addColumnDropTarget: function(aBox)
  {
    aBox._treeBuilderDropTarget = this;
    this.addDNDListener(aBox, "ondragover", "Target");
    this.addDNDListener(aBox, "ondragdrop", "Target");
  },

  removeColumnDropTarget: function(aBox)
  {
    aBox._treeBuilderDropTarget = this;
    this.removeDNDListener(aBox, "ondragover", "Target");
    this.removeDNDListener(aBox, "ondragdrop", "Target");
  },

  onDragOverTarget: function(aBox, aEvent)
  {
    DNDUtils.checkCanDrop("TreeBuilder/column-remove");
  },

  onDragDropTarget: function(aBox, aEvent)
  {
    this.removeColumn(this.mColumnDragging);
    this.build();
    // if we rebuildContent during this thread, it will crash in the dnd service
    setTimeout(function(aTreeBuilder) { aTreeBuilder.buildContent() }, 1, this);
  },

  // these are horrible hacks to compensate for the lack of true multiple
  // listener support for the DND events

  addDNDListener: function(aBox, aType, aModifier)
  {
   var js = "inTreeBuilder_"+aType+(aModifier?"_"+aModifier:"")+"(this, event);";

   var attr = aBox.getAttribute(aType);
   attr = attr ? attr : "";
   aBox.setAttribute(aType, attr + js);
  },

  removeDNDListener: function(aBox, aType, aModifier)
  {
   var js = "inTreeBuilder_"+aType+(aModifier?"_"+aModifier:"")+"(this, event);";

   var attr = aBox.getAttribute(aType);
   var idx = attr.indexOf(js);
   if (idx != -1)
     attr = attr.substr(0,idx) + attr.substr(idx+1);
   aBox.setAttribute(aType, attr);
  },

  ////////////////////////////////////////////////////////////////////////////
  //// column properties

  addColumn: function(aData)
  {
    this.mColumns.push(aData);

    if (this.mOnColumnAdd)
      this.mOnColumnAdd(this.mColumns.length-1)
  },

  insertColumn: function(aIndex, aData)
  {
    var idx;
    if (aIndex == -1) {
      this.mColumns.push(aData);
      idx = this.mColumns.length-1;
    } else {
      this.mColumns.splice(aIndex, 0, aData);
      idx = aIndex;
    }

    if (this.mOnColumnAdd)
      this.mOnColumnAdd(idx);
  },

  removeColumn: function(aIndex)
  {
    this.mColumns.splice(aIndex, 1);

    if (this.mOnColumnRemove)
      this.mOnColumnRemove(aIndex);
  },

  getColumnAt: function(aIndex)
  {
    var kids = this.mTree.getElementsByTagName("treecol");
    return aIndex < 0 || aIndex >= kids.length ? kids[kids.length-1] : kids[aIndex];
  },

  get columnCount() { return this.mColumns.length },

  getColumnName: function(aIndex) { return this.mColumns[aIndex].name  },
  setColumnName: function(aIndex, aValue) { this.mColumns[aIndex].name = aValue },

  getColumnTitle: function(aIndex) { return this.mColumns[aIndex].title  },
  setColumnTitle: function(aIndex, aValue) { this.mColumns[aIndex].title = aValue },

  getColumnClassName: function(aIndex) { return this.mColumns[aIndex].className  },
  setColumnClassName: function(aIndex, aValue) { this.mColumns[aIndex].className = aValue },

  getColumnFlex: function(aIndex) { return this.mColumns[aIndex].flex  },
  setColumnFlex: function(aIndex, aValue) { this.mColumns[aIndex].flex = aValue },

  getExtras: function(aIndex) { return this.mColumns[aIndex].extras  },
  setExtras: function(aIndex, aValue) { this.mColumns[aIndex].extras = aExtras },

  getAttrs: function(aIndex) { return this.mColumns[aIndex].attrs  },
  setAttrs: function(aIndex, aValue) { this.mColumns[aIndex].attrs = aExtras },

  ////////////////////////////////////////////////////////////////////////////
  //// template building

  build: function(aBuildContent)
  {
    try {
      this.resetTree();
      this.buildColumns();
      this.buildTemplate();
      if (aBuildContent)
        this.buildContent();
    } catch (ex) {
      debug("### ERROR - inTreeBuilder::build failed.\n" + ex);
    }

    //dumpDOM2(this.mTree);

  },

  buildContent: function()
  {
    this.mTreeBody.builder.rebuild();
  },

  buildColumns: function()
  {
    var cols = document.createElementNS(kXULNSURI, "treecols");
    var col, val, split;
    for (var i = 0; i < this.mColumns.length; i++) {
      col = document.createElementNS(kXULNSURI, "treecol");
      col.setAttribute("id", "treecol-"+this.mColumns[i].name);
      col.setAttribute("persist", "width");
      col.setAttribute("label", this.mColumns[i].title);

      // mark first node as primary, if necessary
      if (i == 0 && this.mIsContainer)
        col.setAttribute("primary", "true");

      val = cols[i].className;
      if (val)
        col.setAttribute("class", val);
      val = cols[i].flex;
      if (val)
        col.setAttribute("flex", val);

      cols.appendChild(col);

      if (this.mSplitters && i < this.mColumns.length-1) {
        split = document.createElementNS(kXULNSURI, "splitter");
        split.setAttribute("class", "tree-splitter");
        cols.appendChild(split);
      }
    }
    this.mTree.appendChild(cols);
  },

  buildTemplate: function()
  {
    var cols = this.mColumns;
    var bindings = this.mBindings;
    var row = this.mTreeRow;
    var cell, binding, val, extras, attrs, key, className;

    if (this.mIsIconic) {
      binding = this.createBinding("_icon");
      bindings.appendChild(binding);
    }

    for (var i = 0; i < cols.length; ++i) {
      val = cols[i].name;
      if (!val)
        throw "Column data is incomplete - missing name at index " + i + ".";

      cell = this.createTemplatePart("treecell");
      className = "";

      // build the default value data field
      binding = this.createBinding(val);
      bindings.appendChild(binding);
      
      cell.setAttribute("label", "?" + val);
      cell.setAttribute("ref", "treecol-"+cols[i].name);
      cell.setAttribute("class", className);
      
      var props = "";
      for (key in this.mRowFields)
        props += key+"-?"+this.mRowFields[key]+" ";
      cell.setAttribute("properties", props);

      row.appendChild(cell);
    }
  },

  createBinding: function(aName)
  {
    var binding = document.createElementNS(kXULNSURI, "binding");
    binding.setAttribute("subject", "?row");
    binding.setAttribute("predicate", this.mNameSpace+aName);
    binding.setAttribute("object", "?" + aName);
    return binding;
  },

  createTemplatePart: function(aTagName)
  {
    var el = document.createElementNS(kXULNSURI, aTagName);
    el.setAttribute("id", "templatePart"+inTreeBuilderPartCount);
    inTreeBuilderPartCount++;
    return el;
  }

};

function inTreeBuilder_ondraggesture(aTree, aEvent)
{
  return aTree._treeBuilder.onDragGesture(aEvent);
}

function inTreeBuilder_ondragenter(aTree, aEvent)
{
  return aTree._treeBuilder.onDragEnter(aEvent);
}

function inTreeBuilder_ondragover(aTree, aEvent)
{
  return aTree._treeBuilder.onDragOver(aEvent);
}

function inTreeBuilder_ondragexit(aTree, aEvent)
{
  return aTree._treeBuilder.onDragExit(aEvent);
}

function inTreeBuilder_ondragdrop(aTree, aEvent)
{
  return aTree._treeBuilder.onDragDrop(aEvent);
}

function inTreeBuilder_ondragover_Target(aBox, aEvent)
{
  return aBox._treeBuilderDropTarget.onDragOverTarget(aBox, aEvent);
}

function inTreeBuilder_ondragdrop_Target(aBox, aEvent)
{
  return aBox._treeBuilderDropTarget.onDragDropTarget(aBox, aEvent);
}
