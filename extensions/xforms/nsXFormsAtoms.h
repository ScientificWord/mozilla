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
 * The Original Code is Mozilla XForms support.
 *
 * The Initial Developer of the Original Code is
 * IBM Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2004
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *  Brian Ryner <bryner@brianryner.com>
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

#ifndef nsXFormsAtoms_h_
#define nsXFormsAtoms_h_

#include "nsIAtom.h"

// stolen from nsStaticAtom.h.  Not frozen though, so figured I should steal.
// Class for declaring a static list of atoms, for use with gperf
// Keep this VERY simple
// mString: the value of the atom - the policy is that this is only
//          ASCII, in order to avoid unnecessary conversions if
//          someone asks for this in unicode
// mAtom:   a convienience pointer - if you want to store the value of
//          the atom created by this structure somewhere, put its
//          address here
struct nsXFormsStaticAtom {
    const char* mString;
    nsIAtom ** mAtom;
};

/**
 * Class for singleton XForms nsIAtom pointers
 */
class nsXFormsAtoms
{
 public:
  static NS_HIDDEN_(nsIAtom *) src;
  static NS_HIDDEN_(nsIAtom *) bind;
  static NS_HIDDEN_(nsIAtom *) type;
  static NS_HIDDEN_(nsIAtom *) readonly;
  static NS_HIDDEN_(nsIAtom *) required;
  static NS_HIDDEN_(nsIAtom *) relevant;
  static NS_HIDDEN_(nsIAtom *) calculate;
  static NS_HIDDEN_(nsIAtom *) constraint;
  static NS_HIDDEN_(nsIAtom *) p3ptype;
  static NS_HIDDEN_(nsIAtom *) modelListProperty;
  static NS_HIDDEN_(nsIAtom *) uploadFileProperty;
  static NS_HIDDEN_(nsIAtom *) messageProperty;
  static NS_HIDDEN_(nsIAtom *) ref;
  static NS_HIDDEN_(nsIAtom *) nodeset;
  static NS_HIDDEN_(nsIAtom *) model;
  static NS_HIDDEN_(nsIAtom *) selected;
  static NS_HIDDEN_(nsIAtom *) appearance;
  static NS_HIDDEN_(nsIAtom *) incremental;
  static NS_HIDDEN_(nsIAtom *) value;
  static NS_HIDDEN_(nsIAtom *) clazz;
  static NS_HIDDEN_(nsIAtom *) deferredBindListProperty;
  static NS_HIDDEN_(nsIAtom *) readyForBindProperty;
  static NS_HIDDEN_(nsIAtom *) fatalError;
  static NS_HIDDEN_(nsIAtom *) isInstanceDocument;
  static NS_HIDDEN_(nsIAtom *) instanceDocumentOwner;
  static NS_HIDDEN_(nsIAtom *) externalMessagesProperty;
  static NS_HIDDEN_(nsIAtom *) deferredEventListProperty;
  static NS_HIDDEN_(nsIAtom *) attrBased;

  // xforms elements atoms (required for xforms utility service)
  static NS_HIDDEN_(nsIAtom *) choices;
  static NS_HIDDEN_(nsIAtom *) item;
  static NS_HIDDEN_(nsIAtom *) itemset;
  static NS_HIDDEN_(nsIAtom *) select;
  static NS_HIDDEN_(nsIAtom *) select1;

  static NS_HIDDEN_(void) InitAtoms();

 private:
  static NS_HIDDEN_(const nsXFormsStaticAtom) Atoms_info[];
};

#endif
