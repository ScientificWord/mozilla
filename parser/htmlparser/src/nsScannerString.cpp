/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
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
 * The Original Code is Mozilla.
 *
 * The Initial Developer of the Original Code is IBM Corporation.
 * Portions created by IBM Corporation are Copyright (C) 2003
 * IBM Corporation. All Rights Reserved.
 *
 * Contributor(s):
 *   Darin Fisher <darin@meer.net>
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

#include <stdlib.h>
#include "nsScannerString.h"


  /**
   * nsScannerBufferList
   */

nsScannerBufferList::Buffer*
nsScannerBufferList::AllocBufferFromString( const nsAString& aString )
  {
    PRUint32 len = aString.Length();

    Buffer* buf = (Buffer*) malloc(sizeof(Buffer) + (len + 1) * sizeof(PRUnichar));
    if (buf)
      {
        // leave PRCList members of Buffer uninitialized

        buf->mUsageCount = 0;
        buf->mDataEnd = buf->DataStart() + len;

        nsAString::const_iterator source;
        aString.BeginReading(source);
        nsCharTraits<PRUnichar>::copy(buf->DataStart(), source.get(), len);

        // XXX null terminate.  this shouldn't be required, but we do it because
        // nsScanner erroneously thinks it can dereference DataEnd :-(
        *buf->mDataEnd = PRUnichar(0);
      }
    return buf;
  }

nsScannerBufferList::Buffer*
nsScannerBufferList::AllocBuffer( PRUint32 capacity )
  {
    Buffer* buf = (Buffer*) malloc(sizeof(Buffer) + (capacity + 1) * sizeof(PRUnichar));
    if (buf)
      {
        // leave PRCList members of Buffer uninitialized

        buf->mUsageCount = 0;
        buf->mDataEnd = buf->DataStart() + capacity;

        // XXX null terminate.  this shouldn't be required, but we do it because
        // nsScanner erroneously thinks it can dereference DataEnd :-(
        *buf->mDataEnd = PRUnichar(0);
      }
    return buf;
  }

void
nsScannerBufferList::ReleaseAll()
  {
    while (!PR_CLIST_IS_EMPTY(&mBuffers))
      {
        PRCList* node = PR_LIST_HEAD(&mBuffers);
        PR_REMOVE_LINK(node);
        //printf(">>> freeing buffer @%p\n", node);
        free(NS_STATIC_CAST(Buffer*, node));
      }
  }

void
nsScannerBufferList::SplitBuffer( const Position& pos )
  {
    // splitting to the right keeps the work string and any extant token
    // pointing to and holding a reference count on the same buffer.

    Buffer* bufferToSplit = pos.mBuffer;
    NS_ASSERTION(bufferToSplit, "null pointer");

    PRUint32 splitOffset = pos.mPosition - bufferToSplit->DataStart();
    NS_ASSERTION(pos.mPosition >= bufferToSplit->DataStart() &&
                 splitOffset <= bufferToSplit->DataLength(),
                 "split offset is outside buffer");
    
    PRUint32 len = bufferToSplit->DataLength() - splitOffset;
    Buffer* new_buffer = AllocBuffer(len);
    if (new_buffer)
      {
        nsCharTraits<PRUnichar>::copy(new_buffer->DataStart(),
                                      bufferToSplit->DataStart() + splitOffset,
                                      len);
        InsertAfter(new_buffer, bufferToSplit);
        bufferToSplit->SetDataLength(splitOffset);
      }
  }

void
nsScannerBufferList::DiscardUnreferencedPrefix( Buffer* aBuf )
  {
    if (aBuf == Head())
      {
        while (!PR_CLIST_IS_EMPTY(&mBuffers) && !Head()->IsInUse())
          {
            Buffer* buffer = Head();
            PR_REMOVE_LINK(buffer);
            free(buffer);
          }
      }
  }

size_t
nsScannerBufferList::Position::Distance( const Position& aStart, const Position& aEnd )
  {
    size_t result = 0;
    if (aStart.mBuffer == aEnd.mBuffer)
      {
        result = aEnd.mPosition - aStart.mPosition;
      }
    else
      {
        result = aStart.mBuffer->DataEnd() - aStart.mPosition;
        for (Buffer* b = aStart.mBuffer->Next(); b != aEnd.mBuffer; b = b->Next())
          result += b->DataLength();
        result += aEnd.mPosition - aEnd.mBuffer->DataStart();
      }
    return result;
  }


/**
 * nsScannerSubstring
 */

nsScannerSubstring::nsScannerSubstring()
  : mStart(nsnull, nsnull)
  , mEnd(nsnull, nsnull)
  , mBufferList(nsnull)
  , mLength(0)
  , mIsDirty(PR_TRUE)
  {
  }

nsScannerSubstring::nsScannerSubstring( const nsAString& s )
  : mBufferList(nsnull)
  , mIsDirty(PR_TRUE)
  {
    Rebind(s);
  }

nsScannerSubstring::~nsScannerSubstring()
  {
    release_ownership_of_buffer_list();
  }

PRInt32
nsScannerSubstring::CountChar( PRUnichar c ) const
  {
      /*
        re-write this to use a counting sink
       */

    size_type result = 0;
    size_type lengthToExamine = Length();

    nsScannerIterator iter;
    for ( BeginReading(iter); ; )
      {
        PRInt32 lengthToExamineInThisFragment = iter.size_forward();
        const PRUnichar* fromBegin = iter.get();
        result += size_type(NS_COUNT(fromBegin, fromBegin+lengthToExamineInThisFragment, c));
        if ( !(lengthToExamine -= lengthToExamineInThisFragment) )
          return result;
        iter.advance(lengthToExamineInThisFragment);
      }
      // never reached; quiets warnings
    return 0;
  }

void
nsScannerSubstring::Rebind( const nsScannerSubstring& aString,
                            const nsScannerIterator& aStart, 
                            const nsScannerIterator& aEnd )
  {
    // allow for the case where &aString == this

    aString.acquire_ownership_of_buffer_list();
    release_ownership_of_buffer_list();

    mStart      = aStart;
    mEnd        = aEnd;
    mBufferList = aString.mBufferList;
    mLength     = Distance(aStart, aEnd);
    mIsDirty    = PR_TRUE;
  }

void
nsScannerSubstring::Rebind( const nsAString& aString )
  {
    release_ownership_of_buffer_list();

    mBufferList = new nsScannerBufferList(AllocBufferFromString(aString));
    mIsDirty    = PR_TRUE;

    init_range_from_buffer_list();
    acquire_ownership_of_buffer_list();
  }

const nsSubstring&
nsScannerSubstring::AsString() const
  {
    if (mIsDirty)
      {
        nsScannerSubstring* mutable_this = NS_CONST_CAST(nsScannerSubstring*, this);

        if (mStart.mBuffer == mEnd.mBuffer) {
          // We only have a single fragment to deal with, so just return it
          // as a substring.
          mutable_this->mFlattenedRep.Rebind(mStart.mPosition, mEnd.mPosition);
        } else {
          // Otherwise, we need to copy the data into a flattened buffer.
          nsScannerIterator start, end;
          CopyUnicodeTo(BeginReading(start), EndReading(end), mutable_this->mFlattenedRep);
        }

        mutable_this->mIsDirty = PR_FALSE;
      }

    return mFlattenedRep;
  }

nsScannerIterator&
nsScannerSubstring::BeginReading( nsScannerIterator& iter ) const
  {
    iter.mOwner = this;

    iter.mFragment.mBuffer = mStart.mBuffer;
    iter.mFragment.mFragmentStart = mStart.mPosition;
    if (mStart.mBuffer == mEnd.mBuffer)
      iter.mFragment.mFragmentEnd = mEnd.mPosition;
    else
      iter.mFragment.mFragmentEnd = mStart.mBuffer->DataEnd();

    iter.mPosition = mStart.mPosition;
    iter.normalize_forward();
    return iter;
  }

nsScannerIterator&
nsScannerSubstring::EndReading( nsScannerIterator& iter ) const
  {
    iter.mOwner = this;

    iter.mFragment.mBuffer = mEnd.mBuffer;
    iter.mFragment.mFragmentEnd = mEnd.mPosition;
    if (mStart.mBuffer == mEnd.mBuffer)
      iter.mFragment.mFragmentStart = mStart.mPosition;
    else
      iter.mFragment.mFragmentStart = mEnd.mBuffer->DataStart();

    iter.mPosition = mEnd.mPosition;
    // must not |normalize_backward| as that would likely invalidate tests like |while ( first != last )|
    return iter;
  }

PRBool
nsScannerSubstring::GetNextFragment( nsScannerFragment& frag ) const
  {
    // check to see if we are at the end of the buffer list
    if (frag.mBuffer == mEnd.mBuffer)
      return PR_FALSE;

    frag.mBuffer = NS_STATIC_CAST(const Buffer*, PR_NEXT_LINK(frag.mBuffer));

    if (frag.mBuffer == mStart.mBuffer)
      frag.mFragmentStart = mStart.mPosition;
    else
      frag.mFragmentStart = frag.mBuffer->DataStart();

    if (frag.mBuffer == mEnd.mBuffer)
      frag.mFragmentEnd = mEnd.mPosition;
    else
      frag.mFragmentEnd = frag.mBuffer->DataEnd();

    return PR_TRUE;
  }

PRBool
nsScannerSubstring::GetPrevFragment( nsScannerFragment& frag ) const
  {
    // check to see if we are at the beginning of the buffer list
    if (frag.mBuffer == mStart.mBuffer)
      return PR_FALSE;

    frag.mBuffer = NS_STATIC_CAST(const Buffer*, PR_PREV_LINK(frag.mBuffer));

    if (frag.mBuffer == mStart.mBuffer)
      frag.mFragmentStart = mStart.mPosition;
    else
      frag.mFragmentStart = frag.mBuffer->DataStart();

    if (frag.mBuffer == mEnd.mBuffer)
      frag.mFragmentEnd = mEnd.mPosition;
    else
      frag.mFragmentEnd = frag.mBuffer->DataEnd();

    return PR_TRUE;
  }


  /**
   * nsScannerString
   */

nsScannerString::nsScannerString( Buffer* aBuf )
  {
    mBufferList = new nsScannerBufferList(aBuf);

    init_range_from_buffer_list();
    acquire_ownership_of_buffer_list();
  }

void
nsScannerString::AppendBuffer( Buffer* aBuf )
  {
    mBufferList->Append(aBuf);
    mLength += aBuf->DataLength();

    mEnd.mBuffer = aBuf;
    mEnd.mPosition = aBuf->DataEnd();

    mIsDirty = PR_TRUE;
  }

void
nsScannerString::DiscardPrefix( const nsScannerIterator& aIter )
  {
    Position old_start(mStart);
    mStart = aIter;
    mLength -= Position::Distance(old_start, mStart);
    
    mStart.mBuffer->IncrementUsageCount();
    old_start.mBuffer->DecrementUsageCount();

    mBufferList->DiscardUnreferencedPrefix(old_start.mBuffer);

    mIsDirty = PR_TRUE;
  }

void
nsScannerString::UngetReadable( const nsAString& aReadable, const nsScannerIterator& aInsertPoint )
    /*
     * Warning: this routine manipulates the shared buffer list in an unexpected way.
     *  The original design did not really allow for insertions, but this call promises
     *  that if called for a point after the end of all extant token strings, that no token string
     *  or the work string will be invalidated.
     *
     *  This routine is protected because it is the responsibility of the derived class to keep those promises.
     */
  {
    Position insertPos(aInsertPoint);

    mBufferList->SplitBuffer(insertPos);
      // splitting to the right keeps the work string and any extant token pointing to and
      //  holding a reference count on the same buffer

    Buffer* new_buffer = AllocBufferFromString(aReadable);
      // make a new buffer with all the data to insert...
      //  BULLSHIT ALERT: we may have empty space to re-use in the split buffer, measure the cost
      //  of this and decide if we should do the work to fill it

    Buffer* buffer_to_split = insertPos.mBuffer;
    mBufferList->InsertAfter(new_buffer, buffer_to_split);
    mLength += aReadable.Length();

    mEnd.mBuffer = mBufferList->Tail();
    mEnd.mPosition = mEnd.mBuffer->DataEnd();

    mIsDirty = PR_TRUE;
  }

void
nsScannerString::ReplaceCharacter(nsScannerIterator& aPosition, PRUnichar aChar)
  {
    // XXX Casting a const to non-const. Unless the base class
    // provides support for writing iterators, this is the best
    // that can be done.
    PRUnichar* pos = NS_CONST_CAST(PRUnichar*, aPosition.get());
    *pos = aChar;

    mIsDirty = PR_TRUE;
  }


  /**
   * nsScannerSharedSubstring
   */

void
nsScannerSharedSubstring::Rebind(const nsScannerIterator &aStart,
                              const nsScannerIterator &aEnd)
{
  // If the start and end positions are inside the same buffer, we must
  // acquire ownership of the buffer.  If not, we can optimize by not holding
  // onto it.

  Buffer *buffer = NS_CONST_CAST(Buffer*, aStart.buffer());
  PRBool sameBuffer = buffer == aEnd.buffer();

  nsScannerBufferList *bufferList;

  if (sameBuffer) {
    bufferList = aStart.mOwner->mBufferList;
    bufferList->AddRef();
    buffer->IncrementUsageCount();
  }

  if (mBufferList)
    ReleaseBuffer();

  if (sameBuffer) {
    mBuffer = buffer;
    mBufferList = bufferList;
    mString.Rebind(aStart.mPosition, aEnd.mPosition);
  } else {
    mBuffer = nsnull;
    mBufferList = nsnull;
    CopyUnicodeTo(aStart, aEnd, mString);
  }
}

void
nsScannerSharedSubstring::ReleaseBuffer()
{
  NS_ASSERTION(mBufferList, "Should only be called with non-null mBufferList");
  mBuffer->DecrementUsageCount();
  mBufferList->DiscardUnreferencedPrefix(mBuffer);
  mBufferList->Release();
}

void
nsScannerSharedSubstring::MakeMutable()
{
  nsString temp(mString); // this will force a copy of the data
  mString.Assign(temp);   // mString will now share the just-allocated buffer

  ReleaseBuffer();

  mBuffer = nsnull;
  mBufferList = nsnull;
}

  /**
   * utils -- based on code from nsReadableUtils.cpp
   */

void
CopyUnicodeTo( const nsScannerIterator& aSrcStart,
               const nsScannerIterator& aSrcEnd,
               nsAString& aDest )
  {
    nsAString::iterator writer;
    if (!EnsureStringLength(aDest, Distance(aSrcStart, aSrcEnd))) {
      aDest.Truncate();
      return; // out of memory
    }
    aDest.BeginWriting(writer);
    nsScannerIterator fromBegin(aSrcStart);
    
    copy_string(fromBegin, aSrcEnd, writer);
  }

void
AppendUnicodeTo( const nsScannerIterator& aSrcStart,
                 const nsScannerIterator& aSrcEnd,
                 nsScannerSharedSubstring& aDest )
  {
    // Check whether we can just create a dependent string.
    if (aDest.str().IsEmpty()) {
      // We can just make |aDest| point to the buffer.
      // This will take care of copying if the buffer spans fragments.
      aDest.Rebind(aSrcStart, aSrcEnd);
    } else {
      // The dest string is not empty, so it can't be a dependent substring.
      AppendUnicodeTo(aSrcStart, aSrcEnd, aDest.writable());
    }
  }

void
AppendUnicodeTo( const nsScannerIterator& aSrcStart,
                 const nsScannerIterator& aSrcEnd,
                 nsAString& aDest )
  {
    nsAString::iterator writer;
    PRUint32 oldLength = aDest.Length();
    if (!EnsureStringLength(aDest, oldLength + Distance(aSrcStart, aSrcEnd)))
      return; // out of memory
    aDest.BeginWriting(writer).advance(oldLength);
    nsScannerIterator fromBegin(aSrcStart);
    
    copy_string(fromBegin, aSrcEnd, writer);
  }

PRBool
FindCharInReadable( PRUnichar aChar,
                    nsScannerIterator& aSearchStart,
                    const nsScannerIterator& aSearchEnd )
  {
    while ( aSearchStart != aSearchEnd )
      {
        PRInt32 fragmentLength;
        if ( SameFragment(aSearchStart, aSearchEnd) ) 
          fragmentLength = aSearchEnd.get() - aSearchStart.get();
        else
          fragmentLength = aSearchStart.size_forward();

        const PRUnichar* charFoundAt = nsCharTraits<PRUnichar>::find(aSearchStart.get(), fragmentLength, aChar);
        if ( charFoundAt ) {
          aSearchStart.advance( charFoundAt - aSearchStart.get() );
          return PR_TRUE;
        }

        aSearchStart.advance(fragmentLength);
      }

    return PR_FALSE;
  }

PRBool
FindInReadable( const nsAString& aPattern,
                nsScannerIterator& aSearchStart,
                nsScannerIterator& aSearchEnd,
                const nsStringComparator& compare )
  {
    PRBool found_it = PR_FALSE;

      // only bother searching at all if we're given a non-empty range to search
    if ( aSearchStart != aSearchEnd )
      {
        nsAString::const_iterator aPatternStart, aPatternEnd;
        aPattern.BeginReading(aPatternStart);
        aPattern.EndReading(aPatternEnd);

          // outer loop keeps searching till we find it or run out of string to search
        while ( !found_it )
          {
              // fast inner loop (that's what it's called, not what it is) looks for a potential match
            while ( aSearchStart != aSearchEnd &&
                    compare(*aPatternStart, *aSearchStart) )
              ++aSearchStart;

              // if we broke out of the `fast' loop because we're out of string ... we're done: no match
            if ( aSearchStart == aSearchEnd )
              break;

              // otherwise, we're at a potential match, let's see if we really hit one
            nsAString::const_iterator testPattern(aPatternStart);
            nsScannerIterator testSearch(aSearchStart);

              // slow inner loop verifies the potential match (found by the `fast' loop) at the current position
            for(;;)
              {
                  // we already compared the first character in the outer loop,
                  //  so we'll advance before the next comparison
                ++testPattern;
                ++testSearch;

                  // if we verified all the way to the end of the pattern, then we found it!
                if ( testPattern == aPatternEnd )
                  {
                    found_it = PR_TRUE;
                    aSearchEnd = testSearch; // return the exact found range through the parameters
                    break;
                  }

                  // if we got to end of the string we're searching before we hit the end of the
                  //  pattern, we'll never find what we're looking for
                if ( testSearch == aSearchEnd )
                  {
                    aSearchStart = aSearchEnd;
                    break;
                  }

                  // else if we mismatched ... it's time to advance to the next search position
                  //  and get back into the `fast' loop
                if ( compare(*testPattern, *testSearch) )
                  {
                    ++aSearchStart;
                    break;
                  }
              }
          }
      }

    return found_it;
  }

  /**
   * This implementation is simple, but does too much work.
   * It searches the entire string from left to right, and returns the last match found, if any.
   * This implementation will be replaced when I get |reverse_iterator|s working.
   */
PRBool
RFindInReadable( const nsAString& aPattern,
                 nsScannerIterator& aSearchStart,
                 nsScannerIterator& aSearchEnd,
                 const nsStringComparator& aComparator )
  {
    PRBool found_it = PR_FALSE;

    nsScannerIterator savedSearchEnd(aSearchEnd);
    nsScannerIterator searchStart(aSearchStart), searchEnd(aSearchEnd);

    while ( searchStart != searchEnd )
      {
        if ( FindInReadable(aPattern, searchStart, searchEnd, aComparator) )
          {
            found_it = PR_TRUE;

              // this is the best match so far, so remember it
            aSearchStart = searchStart;
            aSearchEnd = searchEnd;

              // ...and get ready to search some more
              //  (it's tempting to set |searchStart=searchEnd| ... but that misses overlapping patterns)
            ++searchStart;
            searchEnd = savedSearchEnd;
          }
      }

      // if we never found it, return an empty range
    if ( !found_it )
      aSearchStart = aSearchEnd;

    return found_it;
  }
