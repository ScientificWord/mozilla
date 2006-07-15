/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
 * The Original Code is TransforMiiX XSLT processor code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2001
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Peter Van der Beken <peterv@propagandism.org>
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

#ifndef TRANSFRMX_ATOMS_H
#define TRANSFRMX_ATOMS_H

#ifndef TX_EXE

#include "nsGkAtoms.h"
typedef class nsGkAtoms txXPathAtoms;
typedef class nsGkAtoms txXMLAtoms;
typedef class nsGkAtoms txXSLTAtoms;
typedef class nsGkAtoms txHTMLAtoms;

#else

class nsIAtom;

/*
 * Declare all atoms
 *
 * The atom names and values are stored in tx*AtomList.h and
 * are brought to you by the magic of C preprocessing.
 * Add new atoms to tx*AtomList.h and all support logic will
 * be auto-generated.
 */

#define DOM_ATOMS                               \
TX_ATOM(comment, "#comment")                    \
TX_ATOM(document, "#document")                  \
TX_ATOM(text, "#text")

#define XML_ATOMS             \
TX_ATOM(_empty, "")           \
TX_ATOM(base, "base")         \
TX_ATOM(_default, "default")  \
TX_ATOM(lang, "lang")         \
TX_ATOM(preserve, "preserve") \
TX_ATOM(space, "space")       \
TX_ATOM(xml, "xml")           \
TX_ATOM(xmlns, "xmlns")       \
DOM_ATOMS

#define TX_ATOM(_name, _value) static nsIAtom* _name;

class txXMLAtoms
{
public:
    static void init();
XML_ATOMS
};

class txXPathAtoms
{
public:
    static void init();
#include "txXPathAtomList.h"
};

class txXSLTAtoms
{
public:
    static void init();
#include "txXSLTAtomList.h"
};

class txHTMLAtoms
{
public:
    static void init();
#include "txHTMLAtomList.h"
};

#undef TX_ATOM

#endif

#endif
