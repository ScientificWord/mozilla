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

#include "nscore.h"

#ifndef _MDB_
#include "mdb.h"
#endif

#ifndef _MORK_
#include "mork.h"
#endif

#ifndef _MORKNODE_
#include "morkNode.h"
#endif

#ifndef _MORKENV_
#include "morkEnv.h"
#endif

#ifndef _MORKARRAY_
#include "morkArray.h"
#endif

//3456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789

// ````` ````` ````` ````` ````` 
// { ===== begin morkNode interface =====

/*public virtual*/ void
morkArray::CloseMorkNode(morkEnv* ev) // CloseTable() only if open
{
  if ( this->IsOpenNode() )
  {
    this->MarkClosing();
    this->CloseArray(ev);
    this->MarkShut();
  }
}

/*public virtual*/
morkArray::~morkArray() // assert CloseTable() executed earlier
{
  MORK_ASSERT(this->IsShutNode());
  MORK_ASSERT(mArray_Slots==0);
}

/*public non-poly*/
morkArray::morkArray(morkEnv* ev, const morkUsage& inUsage,
    nsIMdbHeap* ioHeap, mork_size inSize, nsIMdbHeap* ioSlotHeap)
: morkNode(ev, inUsage, ioHeap)
, mArray_Slots( 0 )
, mArray_Heap( 0 )
, mArray_Fill( 0 )
, mArray_Size( 0 )
, mArray_Seed( (mork_u4)NS_PTR_TO_INT32(this) ) // "random" integer assignment
{
  if ( ev->Good() )
  {
    if ( ioSlotHeap )
    {
      nsIMdbHeap_SlotStrongHeap(ioSlotHeap, ev, &mArray_Heap);
      if ( ev->Good() )
      {
        if ( inSize < 3 )
          inSize = 3;
        mdb_size byteSize = inSize * sizeof(void*);
        void** block = 0;
        ioSlotHeap->Alloc(ev->AsMdbEnv(), byteSize, (void**) &block);
        if ( block && ev->Good() )
        {
          mArray_Slots = block;
          mArray_Size = inSize;
          MORK_MEMSET(mArray_Slots, 0, byteSize);
          if ( ev->Good() )
            mNode_Derived = morkDerived_kArray;
        }
      }
    }
    else
      ev->NilPointerError();
  }
}

/*public non-poly*/ void
morkArray::CloseArray(morkEnv* ev) // called by CloseMorkNode();
{
  if ( this )
  {
    if ( this->IsNode() )
    {
      if ( mArray_Heap && mArray_Slots )
        mArray_Heap->Free(ev->AsMdbEnv(), mArray_Slots);
        
      mArray_Slots = 0;
      mArray_Size = 0;
      mArray_Fill = 0;
      ++mArray_Seed;
      nsIMdbHeap_SlotStrongHeap((nsIMdbHeap*) 0, ev, &mArray_Heap);
      this->MarkShut();
    }
    else
      this->NonNodeError(ev);
  }
  else
    ev->NilPointerError();
}

// } ===== end morkNode methods =====
// ````` ````` ````` ````` ````` 

/*static*/ void
morkArray::NonArrayTypeError(morkEnv* ev)
{
  ev->NewError("non morkArray");
}

/*static*/ void
morkArray::IndexBeyondEndError(morkEnv* ev)
{
  ev->NewError("array index beyond end");
}

/*static*/ void
morkArray::NilSlotsAddressError(morkEnv* ev)
{
  ev->NewError("nil mArray_Slots");
}

/*static*/ void
morkArray::FillBeyondSizeError(morkEnv* ev)
{
  ev->NewError("mArray_Fill > mArray_Size");
}

mork_bool
morkArray::Grow(morkEnv* ev, mork_size inNewSize)
// Grow() returns true if capacity becomes >= inNewSize and ev->Good()
{
  if ( ev->Good() && inNewSize > mArray_Size ) // make array larger?
  {
    if ( mArray_Fill <= mArray_Size ) // fill and size fit the invariant?
    {
      if (mArray_Size <= 3)
        inNewSize = mArray_Size + 3;
      else
        inNewSize = mArray_Size  * 2;// + 3;  // try doubling size here - used to grow by 3
        
      mdb_size newByteSize = inNewSize * sizeof(void*);
      void** newBlock = 0;
      mArray_Heap->Alloc(ev->AsMdbEnv(), newByteSize, (void**) &newBlock);
      if ( newBlock && ev->Good() ) // okay new block?
      {
        void** oldSlots = mArray_Slots;
        void** oldEnd = oldSlots + mArray_Fill;
        
        void** newSlots = newBlock;
        void** newEnd = newBlock + inNewSize;
        
        while ( oldSlots < oldEnd )
          *newSlots++ = *oldSlots++;
          
        while ( newSlots < newEnd )
          *newSlots++ = (void*) 0;

        oldSlots = mArray_Slots;
        mArray_Size = inNewSize;
        mArray_Slots = newBlock;
        mArray_Heap->Free(ev->AsMdbEnv(), oldSlots);
      }
    }
    else
      this->FillBeyondSizeError(ev);
  }
  ++mArray_Seed; // always modify seed, since caller intends to add slots
  return ( ev->Good() && mArray_Size >= inNewSize );
}

void*
morkArray::SafeAt(morkEnv* ev, mork_pos inPos)
{
  if ( mArray_Slots )
  {
    if ( inPos >= 0 && inPos < (mork_pos) mArray_Fill )
      return mArray_Slots[ inPos ];
    else
      this->IndexBeyondEndError(ev);
  }
  else
    this->NilSlotsAddressError(ev);
    
  return (void*) 0;
}

void
morkArray::SafeAtPut(morkEnv* ev, mork_pos inPos, void* ioSlot)
{
  if ( mArray_Slots )
  {
    if ( inPos >= 0 && inPos < (mork_pos) mArray_Fill )
    {
      mArray_Slots[ inPos ] = ioSlot;
      ++mArray_Seed;
    }
    else
      this->IndexBeyondEndError(ev);
  }
  else
    this->NilSlotsAddressError(ev);
}

mork_pos
morkArray::AppendSlot(morkEnv* ev, void* ioSlot)
{
  mork_pos outPos = -1;
  if ( mArray_Slots )
  {
    mork_fill fill = mArray_Fill;
    if ( this->Grow(ev, fill+1) )
    {
      outPos = (mork_pos) fill;
      mArray_Slots[ fill ] = ioSlot;
      mArray_Fill = fill + 1;
      // note Grow() increments mArray_Seed
    }
  }
  else
    this->NilSlotsAddressError(ev);
    
  return outPos;
}

void
morkArray::AddSlot(morkEnv* ev, mork_pos inPos, void* ioSlot)
{
  if ( mArray_Slots )
  {
    mork_fill fill = mArray_Fill;
    if ( this->Grow(ev, fill+1) )
    {
      void** slot = mArray_Slots; // the slot vector
      void** end = slot + fill; // one past the last used array slot
      slot += inPos; // the slot to be added

      while ( --end >= slot ) // another slot to move upward?
        end[ 1 ] = *end;

      *slot = ioSlot;
      mArray_Fill = fill + 1;
      // note Grow() increments mArray_Seed
    }
  }
  else
    this->NilSlotsAddressError(ev);
}

void
morkArray::CutSlot(morkEnv* ev, mork_pos inPos)
{
  MORK_USED_1(ev);
  mork_fill fill = mArray_Fill;
  if ( inPos >= 0 && inPos < (mork_pos) fill ) // cutting slot in used array portion?
  {
    void** slot = mArray_Slots; // the slot vector
    void** end = slot + fill; // one past the last used array slot
    slot += inPos; // the slot to be cut
    
    while ( ++slot < end ) // another slot to move downward?
      slot[ -1 ] = *slot;
      
    slot[ -1 ] = 0; // clear the last used slot which is now unused
    
    // note inPos<fill implies fill>0, so fill-1 must be nonnegative:
    mArray_Fill = fill - 1;
    ++mArray_Seed;
  }
}

void
morkArray::CutAllSlots(morkEnv* ev)
{
  if ( mArray_Slots )
  {
    if ( mArray_Fill <= mArray_Size )
    {
      mdb_size oldByteSize = mArray_Fill * sizeof(void*);
      MORK_MEMSET(mArray_Slots, 0, oldByteSize);
    }
    else
      this->FillBeyondSizeError(ev);
  }
  else
    this->NilSlotsAddressError(ev);

  ++mArray_Seed;
  mArray_Fill = 0;
}

//3456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789
