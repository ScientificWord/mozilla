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

#ifndef _ORKINHEAP_
#define _ORKINHEAP_ 1

#ifndef _MDB_
#include "mdb.h"
#endif

#ifndef _MORK_
#include "mork.h"
#endif

//3456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789

#define orkinHeap_kTag 0x68456150 /* ascii 'hEaP' */

/*| orkinHeap: 
|*/
class orkinHeap : public nsIMdbHeap { //

#ifdef MORK_DEBUG_HEAP_STATS
protected:
  mork_num sHeap_AllocCount;  // number of times Alloc() is called
  mork_num sHeap_FreeCount;   // number of times Free() is called
  mork_num sHeap_BlockCount;  // number of outstanding blocks
  
  mork_num sHeap_BlockVolume; // sum of sizes for all outstanding blocks
  mork_num sHeap_HighWaterVolume;  // largest value of sHeap_BlockVolume seen
  mork_num sHeap_HighWaterTenKilo; // HighWaterVolume in 10K granularity
  mork_num sHeap_HighWaterHundredKilo; // HighWaterVolume in 100K granularity
  
public: // getters
  mork_num HeapAllocCount() const { return sHeap_AllocCount; }
  mork_num HeapFreeCount() const { return sHeap_FreeCount; }
  mork_num HeapBlockCount() const { return sHeap_AllocCount - sHeap_FreeCount; }
  
  mork_num HeapBlockVolume() const { return sHeap_BlockVolume; }
  mork_num HeapHighWaterVolume() const { return sHeap_HighWaterVolume; }
#endif /*MORK_DEBUG_HEAP_STATS*/
  
public:
  orkinHeap(); // does nothing
  virtual ~orkinHeap(); // does nothing
    
private: // copying is not allowed
  orkinHeap(const orkinHeap& other);
  orkinHeap& operator=(const orkinHeap& other);

public:

// { ===== begin nsIMdbHeap methods =====
  NS_IMETHOD Alloc(nsIMdbEnv* ev, // allocate a piece of memory
    mdb_size inSize,   // requested size of new memory block 
    void** outBlock);  // memory block of inSize bytes, or nil
    
  NS_IMETHOD Free(nsIMdbEnv* ev, // free block allocated earlier by Alloc()
    void* inBlock);
    
  NS_IMETHOD HeapAddStrongRef(nsIMdbEnv* ev); // does nothing
  NS_IMETHOD HeapCutStrongRef(nsIMdbEnv* ev); // does nothing
// } ===== end nsIMdbHeap methods =====

};

//3456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789

#endif /* _ORKINHEAP_ */
