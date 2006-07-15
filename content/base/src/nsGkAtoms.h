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
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Mozilla Foundation.
 * Portions created by the Initial Developer are Copyright (C) 2006
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

/*
 * This class wraps up the creation (and destruction) of the standard
 * set of atoms used by gklayout; the atoms are created when gklayout
 * is loaded and they are destroyed when gklayout is unloaded.
 */

#ifndef nsGkAtoms_h___
#define nsGkAtoms_h___

#include "nsIAtom.h"

class nsGkAtoms {
public:

  static void AddRefAtoms();

  /* Declare all atoms

     The atom names and values are stored in nsGkAtomList.h and
     are brought to you by the magic of C preprocessing

     Add new atoms to nsGkAtomList and all support logic will be auto-generated
   */
#define GK_ATOM(_name, _value) static nsIAtom* _name;
#include "nsGkAtomList.h"
#undef GK_ATOM
};

#endif /* nsGkAtoms_h___ */
