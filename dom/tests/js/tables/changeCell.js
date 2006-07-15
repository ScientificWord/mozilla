/* -*- Mode: js; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 * The Original Code is Mozilla Communicator client code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
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

function findBody(node)
{
  if (node.nodeType != Node.ELEMENT_NODE) {
    return null;
  }
  var children = node.childNodes;
  if (children == null) {
    return null;
  }
  var length = children.length;
  var child = null;
  var count = 0;
  while (count < length) {
    child = children[count];
    if (child.tagName == "BODY") {
      dump("BODY found");
      return child;
    }
    var body = findBody(child);
    if (null != body) {
      return body;
    }
    count++;
  }
  return null;
}

// Given the body element, find the first table element
function findTable(body)
{
  // XXX A better way to do this would be to use getElementsByTagName(), but
  // it isn't implemented yet...
  var children = body.childNodes
  if (children == null) {
    return null;
  }
  var length = children.length;
  var child = null;
  var count = 0;
  while (count < length) {
    child = children[count];
    if (child.nodeType == Node.ELEMENT_NODE) {
      if (child.tagName == "TABLE") {
        dump("TABLE found");
        break;
      }
    }
    count++;
  }

  return child;
}

// Given a table element, find the first table body
function findTableBody(table)
{
  // XXX A better way to do this would be to use getElementsByTagName(), but
  // it isn't implemented yet...
  var children = table.childNodes
  if (children == null) {
    return null;
  }
  var length = children.length;
  var child = null;
  var count = 0;
  while (count < length) {
    child = children[count];
    if (child.nodeType == Node.ELEMENT_NODE) {
      if (child.tagName == "TBODY") {
        break;
      }
    }
    count++;
  }

  return child;
}

// Change the text of the first table cell
function changeCell(table)
{
  // Get a table body
  var body = findTableBody(table)

  // The table body's first child is a table row
  var row = body.firstChild

  // The row's first child is a table cell
  var cell = row.firstChild

  // Get the cell's text
  var text = cell.firstChild

  // Append some text
  text.append(" NEW TEXT")
}

changeCell(findTable(findBody(document.documentElement)))

