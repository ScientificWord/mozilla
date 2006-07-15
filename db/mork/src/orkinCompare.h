/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-  */
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
 * Portions created by the Initial Developer are Copyright (C) 1999
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

#ifndef _ORKINCOMPARE_
#define _ORKINCOMPARE_ 1

#ifndef _MDB_
#include "mdb.h"
#endif

#ifndef _MORK_
#include "mork.h"
#endif

//3456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789

/*| orkinCompare: 
|*/
class orkinCompare : public nsIMdbCompare { //
  
public:
  orkinCompare(); // does nothing
  virtual ~orkinCompare(); // does nothing
    
private: // copying is not allowed
  orkinCompare(const orkinCompare& other);
  orkinCompare& operator=(const orkinCompare& other);

public:

// { ===== begin nsIMdbCompare methods =====
  NS_IMETHOD Order(nsIMdbEnv* ev,      // compare first to second yarn
    const mdbYarn* inFirst,   // first yarn in comparison
    const mdbYarn* inSecond,  // second yarn in comparison
    mdb_order* outOrder);     // negative="<", zero="=", positive=">"
    
  NS_IMETHOD AddStrongRef(nsIMdbEnv* ev); // does nothing
  NS_IMETHOD CutStrongRef(nsIMdbEnv* ev); // does nothing
// } ===== end nsIMdbCompare methods =====

};

extern mdb_order // standard yarn comparison for nsIMdbCompare::ORder()
mdbYarn_Order(const mdbYarn* inSelf, morkEnv* ev, const mdbYarn* inSecond);

//3456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789

#endif /* _ORKINCOMPARE_ */
