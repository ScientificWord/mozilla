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

#ifndef _ORKINTABLEROWCURSOR_
#define _ORKINTABLEROWCURSOR_ 1

#ifndef _MDB_
#include "mdb.h"
#endif

#ifndef _MORK_
#include "mork.h"
#endif

#ifndef _MORKNODE_
#include "morkNode.h"
#endif

#ifndef _MORKHANDLE_
#include "morkHandle.h"
#endif

#ifndef _MORKTABLEROWCURSOR_
#include "morkTableRowCursor.h"
#endif

//3456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789

class morkTableRowCursor;
#define morkMagic_kTableRowCursor 0x54724375 /* ascii 'TrCu' */

/*| orkinTableRowCursor:
|*/
class orkinTableRowCursor :
  public morkHandle, public nsIMdbTableRowCursor { // nsIMdbCursor

// { ===== begin morkNode interface =====
public: // morkNode virtual methods
  // virtual void CloseMorkNode(morkEnv* ev); // morkHandle is fine
  virtual ~orkinTableRowCursor(); // morkHandle destructor does everything
  
protected: // construction is protected (use the static Make() method)
  orkinTableRowCursor(morkEnv* ev, // note morkUsage is always morkUsage_kPool
    morkHandleFace* ioFace,    // must not be nil, cookie for this handle
    morkTableRowCursor* ioObject); // must not be nil, object for this handle
    
  // void CloseHandle(morkEnv* ev); // don't need to specialize closing

private: // copying is not allowed
  orkinTableRowCursor(const morkHandle& other);
  orkinTableRowCursor& operator=(const morkHandle& other);

// public: // dynamic type identification
  // mork_bool IsHandle() const //
  // { return IsNode() && mNode_Derived == morkDerived_kHandle; }
// } ===== end morkNode methods =====

protected: // morkHandle memory management operators
  void* operator new(size_t inSize, morkPool& ioPool, morkZone& ioZone, morkEnv* ev) CPP_THROW_NEW
  { return ioPool.NewHandle(ev, inSize, &ioZone); }
  
  void* operator new(size_t inSize, morkPool& ioPool, morkEnv* ev) CPP_THROW_NEW
  { return ioPool.NewHandle(ev, inSize, (morkZone*) 0); }
  
  void* operator new(size_t inSize, morkHandleFace* ioFace) CPP_THROW_NEW
  { MORK_USED_1(inSize); return ioFace; }
  
  
public: // construction:

  static orkinTableRowCursor* MakeTableRowCursor(morkEnv* ev, 
    morkTableRowCursor* ioObject);

public: // utilities:

  morkEnv* CanUseTableRowCursor(nsIMdbEnv* mev, mork_bool inMutable,
    mdb_err* outErr) const;

public: // type identification
  mork_bool IsOrkinTableRowCursor() const
  { return mHandle_Magic == morkMagic_kTableRowCursor; }

  mork_bool IsOrkinTableRowCursorHandle() const
  { return this->IsHandle() && this->IsOrkinTableRowCursor(); }

  NS_DECL_ISUPPORTS
// { ===== begin nsIMdbObject methods =====

  // { ----- begin attribute methods -----
  NS_IMETHOD IsFrozenMdbObject(nsIMdbEnv* ev, mdb_bool* outIsReadonly);
  // same as nsIMdbPort::GetIsPortReadonly() when this object is inside a port.
  // } ----- end attribute methods -----

  // { ----- begin factory methods -----
  NS_IMETHOD GetMdbFactory(nsIMdbEnv* ev, nsIMdbFactory** acqFactory); 
  // } ----- end factory methods -----

  // { ----- begin ref counting for well-behaved cyclic graphs -----
  NS_IMETHOD GetWeakRefCount(nsIMdbEnv* ev, // weak refs
    mdb_count* outCount);  
  NS_IMETHOD GetStrongRefCount(nsIMdbEnv* ev, // strong refs
    mdb_count* outCount);

  NS_IMETHOD AddWeakRef(nsIMdbEnv* ev);
  NS_IMETHOD AddStrongRef(nsIMdbEnv* ev);

  NS_IMETHOD CutWeakRef(nsIMdbEnv* ev);
  NS_IMETHOD CutStrongRef(nsIMdbEnv* ev);
  
  NS_IMETHOD CloseMdbObject(nsIMdbEnv* ev); // called at strong refs zero
  NS_IMETHOD IsOpenMdbObject(nsIMdbEnv* ev, mdb_bool* outOpen);
  // } ----- end ref counting -----
  
// } ===== end nsIMdbObject methods =====

// { ===== begin nsIMdbCursor methods =====

  // { ----- begin attribute methods -----
  NS_IMETHOD GetCount(nsIMdbEnv* ev, mdb_count* outCount); // readonly
  NS_IMETHOD GetSeed(nsIMdbEnv* ev, mdb_seed* outSeed);    // readonly
  
  NS_IMETHOD SetPos(nsIMdbEnv* ev, mdb_pos inPos);   // mutable
  NS_IMETHOD GetPos(nsIMdbEnv* ev, mdb_pos* outPos);
  
  NS_IMETHOD SetDoFailOnSeedOutOfSync(nsIMdbEnv* ev, mdb_bool inFail);
  NS_IMETHOD GetDoFailOnSeedOutOfSync(nsIMdbEnv* ev, mdb_bool* outFail);
  // } ----- end attribute methods -----

// } ===== end nsIMdbCursor methods =====

// { ===== begin nsIMdbTableRowCursor methods =====

  // { ----- begin attribute methods -----
  NS_IMETHOD GetTable(nsIMdbEnv* ev, nsIMdbTable** acqTable);
  // } ----- end attribute methods -----

  // { ----- begin oid iteration methods -----
  NS_IMETHOD NextRowOid( // get row id of next row in the table
    nsIMdbEnv* ev, // context
    mdbOid* outOid, // out row oid
    mdb_pos* outRowPos);    // zero-based position of the row in table
  // } ----- end oid iteration methods -----

  // { ----- begin row iteration methods -----
  NS_IMETHOD NextRow( // get row cells from table for cells already in row
    nsIMdbEnv* ev, // context
    nsIMdbRow** acqRow, // acquire next row in table
    mdb_pos* outRowPos);    // zero-based position of the row in table
  // } ----- end row iteration methods -----

  // { ----- begin duplicate row removal methods -----
  NS_IMETHOD CanHaveDupRowMembers(nsIMdbEnv* ev, // cursor might hold dups?
    mdb_bool* outCanHaveDups);
    
  NS_IMETHOD MakeUniqueCursor( // clone cursor, removing duplicate rows
    nsIMdbEnv* ev, // context
    nsIMdbTableRowCursor** acqCursor);    // acquire clone with no dups
    // Note that MakeUniqueCursor() is never necessary for a cursor which was
    // created by table method nsIMdbTable::GetTableRowCursor(), because a table
    // never contains the same row as a member more than once.  However, a cursor
    // created by table method nsIMdbTable::FindRowMatches() might contain the
    // same row more than once, because the same row can generate a hit by more
    // than one column with a matching string prefix.  Note this method can
    // return the very same cursor instance with just an incremented refcount,
    // when the original cursor could not contain any duplicate rows (calling
    // CanHaveDupRowMembers() shows this case on a false return).  Otherwise
    // this method returns a different cursor instance.  Callers should not use
    // this MakeUniqueCursor() method lightly, because it tends to defeat the
    // purpose of lazy programming techniques, since it can force creation of
    // an explicit row collection in a new cursor's representation, in order to
    // inspect the row membership and remove any duplicates; this can have big
    // impact if a collection holds tens of thousands of rows or more, when
    // the original cursor with dups simply referenced rows indirectly by row
    // position ranges, without using an explicit row set representation.
    // Callers are encouraged to use nsIMdbCursor::GetCount() to determine
    // whether the row collection is very large (tens of thousands), and to
    // delay calling MakeUniqueCursor() when possible, until a user interface
    // element actually demands the creation of an explicit set representation.
  // } ----- end duplicate row removal methods -----

// } ===== end nsIMdbTableRowCursor methods =====
};

//3456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789

#endif /* _ORKINTABLEROWCURSOR_ */
