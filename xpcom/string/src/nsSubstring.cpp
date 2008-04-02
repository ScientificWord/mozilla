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

#ifdef DEBUG
#define ENABLE_STRING_STATS
#endif

#ifdef ENABLE_STRING_STATS
#include <stdio.h>
#endif

#include <stdlib.h>
#include "nsSubstring.h"
#include "nsString.h"
#include "nsStringBuffer.h"
#include "nsDependentString.h"
#include "nsMemory.h"
#include "pratom.h"

// ---------------------------------------------------------------------------

static const PRUnichar gNullChar = 0;

const char*      nsCharTraits<char>     ::sEmptyBuffer = (const char*) &gNullChar;
const PRUnichar* nsCharTraits<PRUnichar>::sEmptyBuffer =               &gNullChar;

// ---------------------------------------------------------------------------

#ifdef ENABLE_STRING_STATS
class nsStringStats
  {
    public:
      nsStringStats()
        : mAllocCount(0), mReallocCount(0), mFreeCount(0), mShareCount(0) {}

      ~nsStringStats()
        {
          // this is a hack to suppress duplicate string stats printing
          // in seamonkey as a result of the string code being linked
          // into seamonkey and libxpcom! :-(
          if (!mAllocCount && !mAdoptCount)
            return;

          printf("nsStringStats\n");
          printf(" => mAllocCount:     % 10d\n", mAllocCount);
          printf(" => mReallocCount:   % 10d\n", mReallocCount);
          printf(" => mFreeCount:      % 10d", mFreeCount);
          if (mAllocCount > mFreeCount)
            printf("  --  LEAKED %d !!!\n", mAllocCount - mFreeCount);
          else
            printf("\n");
          printf(" => mShareCount:     % 10d\n", mShareCount);
          printf(" => mAdoptCount:     % 10d\n", mAdoptCount);
          printf(" => mAdoptFreeCount: % 10d", mAdoptFreeCount);
          if (mAdoptCount > mAdoptFreeCount)
            printf("  --  LEAKED %d !!!\n", mAdoptCount - mAdoptFreeCount);
          else
            printf("\n");
        }

      PRInt32 mAllocCount;
      PRInt32 mReallocCount;
      PRInt32 mFreeCount;
      PRInt32 mShareCount;
      PRInt32 mAdoptCount;
      PRInt32 mAdoptFreeCount;
  };
static nsStringStats gStringStats;
#define STRING_STAT_INCREMENT(_s) PR_AtomicIncrement(&gStringStats.m ## _s ## Count)
#else
#define STRING_STAT_INCREMENT(_s)
#endif

// ---------------------------------------------------------------------------

inline void
ReleaseData( void* data, PRUint32 flags )
  {
    if (flags & nsSubstring::F_SHARED)
      {
        nsStringBuffer::FromData(data)->Release();
      }
    else if (flags & nsSubstring::F_OWNED)
      {
        nsMemory::Free(data);
        STRING_STAT_INCREMENT(AdoptFree);
#ifdef NS_BUILD_REFCNT_LOGGING
        // Treat this as destruction of a "StringAdopt" object for leak
        // tracking purposes.
        NS_LogDtor(data, "StringAdopt", 1);
#endif // NS_BUILD_REFCNT_LOGGING
      }
    // otherwise, nothing to do.
  }

// ---------------------------------------------------------------------------

// XXX or we could make nsStringBuffer be a friend of nsTAString

class nsAStringAccessor : public nsAString
  {
    private:
      nsAStringAccessor(); // NOT IMPLEMENTED

    public:
#ifdef MOZ_V1_STRING_ABI
      const void *vtable() const { return mVTable; }
#endif
      char_type  *data() const   { return mData; }
      size_type   length() const { return mLength; }
      PRUint32    flags() const  { return mFlags; }

      void set(char_type *data, size_type len, PRUint32 flags)
        {
          ReleaseData(mData, mFlags);
          mData = data;
          mLength = len;
          mFlags = flags;
        }
  };

class nsACStringAccessor : public nsACString
  {
    private:
      nsACStringAccessor(); // NOT IMPLEMENTED

    public:
#ifdef MOZ_V1_STRING_ABI
      const void *vtable() const { return mVTable; }
#endif
      char_type  *data() const   { return mData; }
      size_type   length() const { return mLength; }
      PRUint32    flags() const  { return mFlags; }

      void set(char_type *data, size_type len, PRUint32 flags)
        {
          ReleaseData(mData, mFlags);
          mData = data;
          mLength = len;
          mFlags = flags;
        }
  };

// ---------------------------------------------------------------------------

void
nsStringBuffer::AddRef()
  {
    PR_AtomicIncrement(&mRefCount);
    STRING_STAT_INCREMENT(Share);
    NS_LOG_ADDREF(this, mRefCount, "nsStringBuffer", sizeof(*this));
  }

void
nsStringBuffer::Release()
  {
    PRInt32 count = PR_AtomicDecrement(&mRefCount);
    NS_LOG_RELEASE(this, count, "nsStringBuffer");
    if (count == 0)
      {
        STRING_STAT_INCREMENT(Free);
        free(this); // we were allocated with |malloc|
      }
  }

  /**
   * Alloc returns a pointer to a new string header with set capacity.
   */
nsStringBuffer*
nsStringBuffer::Alloc(size_t size)
  {
    NS_ASSERTION(size != 0, "zero capacity allocation not allowed");

    nsStringBuffer *hdr =
        (nsStringBuffer *) malloc(sizeof(nsStringBuffer) + size);
    if (hdr)
      {
        STRING_STAT_INCREMENT(Alloc);

        hdr->mRefCount = 1;
        hdr->mStorageSize = size;
        NS_LOG_ADDREF(hdr, 1, "nsStringBuffer", sizeof(*hdr));
      }
    return hdr;
  }

nsStringBuffer*
nsStringBuffer::Realloc(nsStringBuffer* hdr, size_t size)
  {
    STRING_STAT_INCREMENT(Realloc);

    NS_ASSERTION(size != 0, "zero capacity allocation not allowed");

    // no point in trying to save ourselves if we hit this assertion
    NS_ASSERTION(!hdr->IsReadonly(), "|Realloc| attempted on readonly string");

    // Treat this as a release and addref for refcounting purposes, since we
    // just asserted that the refcound is 1.  If we don't do that, refcount
    // logging will claim we've leaked all sorts of stuff.
    NS_LOG_RELEASE(hdr, 0, "nsStringBuffer");
    
    hdr = (nsStringBuffer*) realloc(hdr, sizeof(nsStringBuffer) + size);
    if (hdr) {
      NS_LOG_ADDREF(hdr, 1, "nsStringBuffer", sizeof(*hdr));
      hdr->mStorageSize = size;
    }

    return hdr;
  }

nsStringBuffer*
nsStringBuffer::FromString(const nsAString& str)
  {
    const nsAStringAccessor* accessor =
        static_cast<const nsAStringAccessor*>(&str);

#ifdef MOZ_V1_STRING_ABI
    if (accessor->vtable() != nsObsoleteAString::sCanonicalVTable)
      return nsnull;
#endif
    if (!(accessor->flags() & nsSubstring::F_SHARED))
      return nsnull;

    return FromData(accessor->data());
  }

nsStringBuffer*
nsStringBuffer::FromString(const nsACString& str)
  {
    const nsACStringAccessor* accessor =
        static_cast<const nsACStringAccessor*>(&str);

#ifdef MOZ_V1_STRING_ABI
    if (accessor->vtable() != nsObsoleteACString::sCanonicalVTable)
      return nsnull;
#endif
    if (!(accessor->flags() & nsCSubstring::F_SHARED))
      return nsnull;

    return FromData(accessor->data());
  }

void
nsStringBuffer::ToString(PRUint32 len, nsAString &str)
  {
    PRUnichar* data = static_cast<PRUnichar*>(Data());

    nsAStringAccessor* accessor = static_cast<nsAStringAccessor*>(&str);
#ifdef MOZ_V1_STRING_ABI
    if (accessor->vtable() != nsObsoleteAString::sCanonicalVTable)
      {
        str.Assign(data, len);
        return;
      }
#endif
    NS_ASSERTION(data[len] == PRUnichar(0), "data should be null terminated");

    // preserve class flags
    PRUint32 flags = accessor->flags();
    flags = (flags & 0xFFFF0000) | nsSubstring::F_SHARED | nsSubstring::F_TERMINATED;

    AddRef();
    accessor->set(data, len, flags);
  }

void
nsStringBuffer::ToString(PRUint32 len, nsACString &str)
  {
    char* data = static_cast<char*>(Data());

    nsACStringAccessor* accessor = static_cast<nsACStringAccessor*>(&str);
#ifdef MOZ_V1_STRING_ABI
    if (accessor->vtable() != nsObsoleteACString::sCanonicalVTable)
      {
        str.Assign(data, len);
        return;
      }
#endif
    NS_ASSERTION(data[len] == char(0), "data should be null terminated");

    // preserve class flags
    PRUint32 flags = accessor->flags();
    flags = (flags & 0xFFFF0000) | nsCSubstring::F_SHARED | nsCSubstring::F_TERMINATED;

    AddRef();
    accessor->set(data, len, flags);
  }

// ---------------------------------------------------------------------------


  // define nsSubstring
#include "string-template-def-unichar.h"
#include "nsTSubstring.cpp"
#include "string-template-undef.h"

  // define nsCSubstring
#include "string-template-def-char.h"
#include "nsTSubstring.cpp"
#include "string-template-undef.h"
