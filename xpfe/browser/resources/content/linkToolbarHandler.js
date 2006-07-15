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
 * The Original Code is Eric Hodel's <drbrain@segment7.net> code.
 *
 * The Initial Developer of the Original Code is
 * Eric Hodel.
 * Portions created by the Initial Developer are Copyright (C) 2001
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *      Christopher Hoess <choess@force.stwing.upenn.edu>
 *      Tim Taylor <tim@tool-man.org>
 *      Stuart Ballard <sballard@netreach.net>
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

/** 
 * LinkToolbarHandler is a Singleton that displays LINK elements 
 * and nodeLists of LINK elements in the Link Toolbar.  It
 * associates the LINK with a corresponding LinkToolbarItem based 
 * on it's REL attribute and the toolbar item's ID attribute.
 * LinkToolbarHandler is also a Factory and will create 
 * LinkToolbarItems as necessary.
 */
function LinkToolbarHandler()
{
  this.items = new Array();
  this.hasItems = false;
}

LinkToolbarHandler.prototype.handle =
function(element)
{
  // XXX: if you're going to re-enable handling of anchor elements,
  //    you'll want to change this to AnchorElementDecorator
  var linkElement = new LinkElementDecorator(element);

  if (linkElement.isIgnored()) return;

  for (var i = 0; i < linkElement.relValues.length; i++) {
    var linkType = LinkToolbarHandler.getLinkType(linkElement.relValues[i]);
    if (linkType) {
      if (!this.hasItems) {
        this.hasItems = true;
        linkToolbarUI.activate();
      }
      this.getItemForLinkType(linkType).displayLink(linkElement);
    }
  }
}

LinkToolbarHandler.getLinkType =
function(relAttribute)
{
  switch (relAttribute.toLowerCase()) {
    case "start":
    case "top":
    case "origin":
      return "top";

    case "up":
    case "parent":
      return "up";

    case "begin":
    case "first":
      return "first";

    case "next":
    case "child":
      return "next";

    case "prev":
    case "previous":
      return "prev";

    case "end":
    case "last":
      return "last";

    case "author":
    case "made":
      return "author";

    case "contents":
    case "toc":
      return "toc";

    case "prefetch":
      return null;

    default:
      return relAttribute.toLowerCase();
  }
}

LinkToolbarHandler.prototype.getItemForLinkType =
function(linkType) {
  if (!(linkType in this.items && this.items[linkType]))
    this.items[linkType] = LinkToolbarHandler.createItemForLinkType(linkType);

  return this.items[linkType];
}

LinkToolbarHandler.createItemForLinkType =
function(linkType)
{
  if (!document.getElementById("link-" + linkType))
    return new LinkToolbarTransientMenu(linkType);

  // XXX: replace switch with polymorphism
  switch(document.getElementById("link-" + linkType).localName) {
    case "toolbarbutton":
      return new LinkToolbarButton(linkType);

    case "menuitem":
      return new LinkToolbarItem(linkType);

    case "menu":
      return new LinkToolbarMenu(linkType);

    default:
      return new LinkToolbarTransientMenu(linkType);
  }
}

LinkToolbarHandler.prototype.clearAllItems =
function()
{
  // Hide the 'miscellaneous' separator
  document.getElementById("misc-separator").setAttribute("collapsed", "true");

  // Disable the individual items
  for (var linkType in this.items)
    this.items[linkType].clear();

  // Store the fact that the toolbar is empty
  this.hasItems = false;
}

const linkToolbarHandler = new LinkToolbarHandler();


function LinkElementDecorator(element) {
  /*
   * XXX: this is an incomplete decorator, because it doesn't implement
   *      the full Element interface.  If you need to use a method 
   *      or member in the Element interface, just add it here and
   *      have it delegate to this.element
   *
   * XXX: would rather add some methods to Element.prototype instead of
   *    using a decorator, but Element.prototype is no longer exposed 
   *      since the XPCDOM landing, see bug 83433
   */

  if (!element) return; // skip the rest on foo.prototype = new ThisClass calls
  
  this.element = element;
  
  this.rel = LinkElementDecorator.convertRevMade(element.rel, element.rev);
  if (this.rel)
    this.relValues = this.rel.split(" ");
  this.rev = element.rev;
  this.title = element.title;
  this.href = element.href;
  this.hreflang = element.hreflang;
  this.media = element.media;
  this.longTitle = null;
}

LinkElementDecorator.prototype.isIgnored =
function()
{
  if (!this.rel) return true;
  for (var i = 0; i < this.relValues.length; i++)
    if (/^stylesheet$|^icon$|^fontdef$|^p3pv|^schema./i.test(this.relValues[i]))
      return true;
  return false;
}

LinkElementDecorator.convertRevMade =
function(rel, rev) 
{
  if (!rel && rev && /\bmade\b/i.test(rev))
    return rev;
  else
    return rel;
}

LinkElementDecorator.prototype.getTooltip =
function() 
{
  return this.getLongTitle() != "" ? this.getLongTitle() : this.href;
}

LinkElementDecorator.prototype.getLabel =
function() 
{
  return this.getLongTitle() != "" ? this.getLongTitle() : this.rel;
}

LinkElementDecorator.prototype.getLongTitle =
function() 
{
  if (this.longTitle == null)
    this.longTitle = this.makeLongTitle();

  return this.longTitle;
}

LinkElementDecorator.prototype.makeLongTitle =
function()
{
  var prefix = "";

  // XXX: lookup more meaningful and localized version of media, 
  //   i.e. media="print" becomes "Printable" or some such
  // XXX: use localized version of ":" separator
  if (this.media && !/\ball\b|\bscreen\b/i.test(this.media))
    prefix += this.media + ": ";
  if (this.hreflang)
    prefix += languageDictionary.lookupLanguageName(this.hreflang)
          + ": ";

  return this.title ? prefix + this.title : prefix;
}

function AnchorElementDecorator(element) {
  this.constructor(element);
}
AnchorElementDecorator.prototype = new LinkElementDecorator;

AnchorElementDecorator.prototype.getLongTitle =
function() 
{
  return this.title ? this.__proto__.getLongTitle.apply(this) 
      : getText(this.element);
}

AnchorElementDecorator.prototype.getText =
function(element)
{
  return condenseWhitespace(getTextRecursive(element));
}

AnchorElementDecorator.prototype.getTextRecursive =
function(node) 
{
  var text = "";
  node.normalize();
  if (node.hasChildNodes()) {
    for (var i = 0; i < node.childNodes.length; i++) {
      if (node.childNodes.item(i).nodeType == Node.TEXT_NODE)
        text += node.childNodes.item(i).nodeValue;
      else if (node.childNodes.item(i).nodeType == Node.ELEMENT_NODE)
        text += getTextRecursive(node.childNodes.item(i));
    }
  }

  return text;
}

AnchorElementDecorator.prototype.condenseWhitespace =
function(text)
{
  return text.replace(/\W*$/, "").replace(/^\W*/, "").replace(/\W+/g, " ");
}