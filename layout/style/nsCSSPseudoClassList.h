/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 * The Original Code is atom lists for CSS pseudos.
 *
 * The Initial Developer of the Original Code is 
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   L. David Baron <dbaron@dbaron.org>
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

/* atom list for CSS pseudo-classes */

/*
 * This file contains the list of nsIAtoms and their values for CSS
 * pseudo-classes.  It is designed to be used as inline input to
 * nsCSSPseudoClasses.cpp *only* through the magic of C preprocessing.
 * All entries must be enclosed in the macro CSS_PSEUDO_CLASS which will
 * have cruel and unusual things done to it.  The entries should be kept
 * in some sort of logical order.  The first argument to
 * CSS_PSEUDO_CLASS is the C++ identifier of the atom.  The second
 * argument is the string value of the atom.
 */

// OUTPUT_CLASS=nsCSSPseudoClasses
// MACRO_NAME=CSS_PSEUDO_CLASS

CSS_PSEUDO_CLASS(empty, ":empty")
CSS_PSEUDO_CLASS(mozOnlyWhitespace, ":-moz-only-whitespace")
CSS_PSEUDO_CLASS(mozEmptyExceptChildrenWithLocalname, ":-moz-empty-except-children-with-localname")
CSS_PSEUDO_CLASS(lang, ":lang")
CSS_PSEUDO_CLASS(notPseudo, ":not")
CSS_PSEUDO_CLASS(mozBoundElement, ":-moz-bound-element")
CSS_PSEUDO_CLASS(root, ":root")

CSS_PSEUDO_CLASS(link, ":link")
CSS_PSEUDO_CLASS(mozAnyLink, ":-moz-any-link") // what matches :link or :visited
CSS_PSEUDO_CLASS(visited, ":visited")

CSS_PSEUDO_CLASS(active, ":active")
CSS_PSEUDO_CLASS(checked, ":checked")
CSS_PSEUDO_CLASS(disabled, ":disabled")
CSS_PSEUDO_CLASS(enabled, ":enabled")
CSS_PSEUDO_CLASS(focus, ":focus")
CSS_PSEUDO_CLASS(hover, ":hover")
CSS_PSEUDO_CLASS(mozDragOver, ":-moz-drag-over")
CSS_PSEUDO_CLASS(target, ":target")

CSS_PSEUDO_CLASS(firstChild, ":first-child")
CSS_PSEUDO_CLASS(firstNode, ":-moz-first-node")
CSS_PSEUDO_CLASS(lastChild, ":last-child")
CSS_PSEUDO_CLASS(lastNode, ":-moz-last-node")
CSS_PSEUDO_CLASS(onlyChild, ":only-child")

// Image, object, etc state pseudo-classes
CSS_PSEUDO_CLASS(mozBroken, ":-moz-broken")
CSS_PSEUDO_CLASS(mozUserDisabled, ":-moz-user-disabled")
CSS_PSEUDO_CLASS(mozSuppressed, ":-moz-suppressed")
CSS_PSEUDO_CLASS(mozLoading, ":-moz-loading")
CSS_PSEUDO_CLASS(mozTypeUnsupported, ":-moz-type-unsupported")

CSS_PSEUDO_CLASS(mozHasHandlerRef, ":-moz-has-handlerref")

// Match nodes that are HTML but not XHTML
CSS_PSEUDO_CLASS(mozIsHTML, ":-moz-is-html")

// Matches anything when the specified look-and-feel metric is set
CSS_PSEUDO_CLASS(mozSystemMetric, ":-moz-system-metric")

#ifdef MOZ_MATHML
CSS_PSEUDO_CLASS(mozMathIncrementScriptLevel, ":-moz-math-increment-script-level")
#endif

// CSS 3 UI
// http://www.w3.org/TR/2004/CR-css3-ui-20040511/#pseudo-classes
CSS_PSEUDO_CLASS(required, ":required")
CSS_PSEUDO_CLASS(optional, ":optional")
CSS_PSEUDO_CLASS(valid, ":valid")
CSS_PSEUDO_CLASS(invalid, ":invalid")
CSS_PSEUDO_CLASS(inRange, ":in-range")
CSS_PSEUDO_CLASS(outOfRange, ":out-of-range")
CSS_PSEUDO_CLASS(defaultPseudo, ":default")
CSS_PSEUDO_CLASS(mozReadOnly, ":-moz-read-only")
CSS_PSEUDO_CLASS(mozReadWrite, ":-moz-read-write")
