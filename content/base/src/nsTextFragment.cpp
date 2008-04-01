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
 * The Original Code is Mozilla Communicator client code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
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
 * A class which represents a fragment of text (eg inside a text
 * node); if only codepoints below 256 are used, the text is stored as
 * a char*; otherwise the text is stored as a PRUnichar*
 */

#include "nsTextFragment.h"
#include "nsString.h"
#include "nsCRT.h"
#include "nsReadableUtils.h"
#include "nsMemory.h"
#include "nsBidiUtils.h"
#include "nsUnicharUtils.h"

#define TEXTFRAG_WHITE_AFTER_NEWLINE 50
#define TEXTFRAG_MAX_NEWLINES 7

// Static buffer used for common fragments
static char* sSpaceSharedString[TEXTFRAG_MAX_NEWLINES + 1];
static char* sTabSharedString[TEXTFRAG_MAX_NEWLINES + 1];
static char sSingleCharSharedString[256];

// static
nsresult
nsTextFragment::Init()
{
  // Create whitespace strings
  PRUint32 i;
  for (i = 0; i <= TEXTFRAG_MAX_NEWLINES; ++i) {
    sSpaceSharedString[i] = new char[1 + i + TEXTFRAG_WHITE_AFTER_NEWLINE];
    sTabSharedString[i] = new char[1 + i + TEXTFRAG_WHITE_AFTER_NEWLINE];
    NS_ENSURE_TRUE(sSpaceSharedString[i] && sTabSharedString[i],
                   NS_ERROR_OUT_OF_MEMORY);
    sSpaceSharedString[i][0] = ' ';
    sTabSharedString[i][0] = ' ';
    PRUint32 j;
    for (j = 1; j < 1 + i; ++j) {
      sSpaceSharedString[i][j] = '\n';
      sTabSharedString[i][j] = '\n';
    }
    for (; j < (1 + i + TEXTFRAG_WHITE_AFTER_NEWLINE); ++j) {
      sSpaceSharedString[i][j] = ' ';
      sTabSharedString[i][j] = '\t';
    }
  }

  // Create single-char strings
  for (i = 0; i < 256; ++i) {
    sSingleCharSharedString[i] = i;
  }

  return NS_OK;
}

// static
void
nsTextFragment::Shutdown()
{
  PRUint32  i;
  for (i = 0; i <= TEXTFRAG_MAX_NEWLINES; ++i) {
    delete [] sSpaceSharedString[i];
    delete [] sTabSharedString[i];
    sSpaceSharedString[i] = nsnull;
    sTabSharedString[i] = nsnull;
  }
}

nsTextFragment::~nsTextFragment()
{
  ReleaseText();
}

void
nsTextFragment::ReleaseText()
{
  if (mState.mLength && m1b && mState.mInHeap) {
    nsMemory::Free(m2b); // m1b == m2b as far as nsMemory is concerned
  }

  m1b = nsnull;

  // Set mState.mIs2b, mState.mInHeap, and mState.mLength = 0 with mAllBits;
  mAllBits = 0;
}

nsTextFragment&
nsTextFragment::operator=(const nsTextFragment& aOther)
{
  ReleaseText();

  if (aOther.mState.mLength) {
    if (!aOther.mState.mInHeap) {
      m1b = aOther.m1b; // This will work even if aOther is using m2b
    }
    else {
      m2b = static_cast<PRUnichar*>
                       (nsMemory::Clone(aOther.m2b, aOther.mState.mLength *
                                    (aOther.mState.mIs2b ? sizeof(PRUnichar) : sizeof(char))));
    }

    if (m1b) {
      mAllBits = aOther.mAllBits;
    }
  }

  return *this;
}

void
nsTextFragment::SetTo(const PRUnichar* aBuffer, PRInt32 aLength)
{
  ReleaseText();

  if (aLength == 0) {
    return;
  }
  
  PRUnichar firstChar = *aBuffer;
  if (aLength == 1 && firstChar < 256) {
    m1b = sSingleCharSharedString + firstChar;
    mState.mInHeap = PR_FALSE;
    mState.mIs2b = PR_FALSE;
    mState.mLength = 1;

    return;
  }

  const PRUnichar *ucp = aBuffer;
  const PRUnichar *uend = aBuffer + aLength;

  // Check if we can use a shared string
  if (firstChar == ' ' || firstChar == '\n' || firstChar == '\t') {
    if (firstChar == ' ') {
      ++ucp;
    }

    const PRUnichar* start = ucp;
    while (ucp < uend && *ucp == '\n') {
      ++ucp;
    }
    const PRUnichar* endNewLine = ucp;

    PRUnichar space = ucp < uend && *ucp == '\t' ? '\t' : ' ';
    while (ucp < uend && *ucp == space) {
      ++ucp;
    }

    if (ucp == uend &&
        endNewLine - start <= TEXTFRAG_MAX_NEWLINES &&
        ucp - endNewLine <= TEXTFRAG_WHITE_AFTER_NEWLINE) {
      char** strings = space == ' ' ? sSpaceSharedString : sTabSharedString;
      m1b = strings[endNewLine - start];

      // If we didn't find a space in the beginning, skip it now.
      if (firstChar != ' ') {
        ++m1b;
      }

      mState.mInHeap = PR_FALSE;
      mState.mIs2b = PR_FALSE;
      mState.mLength = aLength;

      return;        
    }
  }

  // See if we need to store the data in ucs2 or not
  PRBool need2 = PR_FALSE;
  while (ucp < uend) {
    PRUnichar ch = *ucp++;
    if (ch >= 256) {
      need2 = PR_TRUE;
      break;
    }
  }

  if (need2) {
    // Use ucs2 storage because we have to
    m2b = (PRUnichar *)nsMemory::Clone(aBuffer,
                                       aLength * sizeof(PRUnichar));
    if (!m2b) {
      return;
    }
  } else {
    // Use 1 byte storage because we can
    char* buff = (char *)nsMemory::Alloc(aLength * sizeof(char));
    if (!buff) {
      return;
    }

    // Copy data
    for (PRUint32 i = 0; i < (PRUint32)aLength; ++i) {
      buff[i] = (char)aBuffer[i];
    }
    m1b = buff;
  }

  // Setup our fields
  mState.mInHeap = PR_TRUE;
  mState.mIs2b = need2;
  mState.mLength = aLength;
}

void
nsTextFragment::AppendTo(nsAString& aString) const
{
  if (mState.mIs2b) {
    aString.Append(m2b, mState.mLength);
  } else {
    AppendASCIItoUTF16(Substring(m1b, m1b + mState.mLength),
                       aString);
  }
}

void
nsTextFragment::AppendTo(nsAString& aString, PRInt32 aOffset, PRInt32 aLength) const
{
  if (mState.mIs2b) {
    aString.Append(m2b + aOffset, aLength);
  } else {
    AppendASCIItoUTF16(Substring(m1b + aOffset, m1b + aOffset + aLength), aString);
  }
}

void
nsTextFragment::CopyTo(PRUnichar *aDest, PRInt32 aOffset, PRInt32 aCount)
{
  NS_ASSERTION(aOffset >= 0, "Bad offset passed to nsTextFragment::CopyTo()!");
  NS_ASSERTION(aCount >= 0, "Bad count passed to nsTextFragment::CopyTo()!");

  if (aOffset < 0) {
    aOffset = 0;
  }

  if (aOffset + aCount > GetLength()) {
    aCount = mState.mLength - aOffset;
  }

  if (aCount != 0) {
    if (mState.mIs2b) {
      memcpy(aDest, m2b + aOffset, sizeof(PRUnichar) * aCount);
    } else {
      const char *cp = m1b + aOffset;
      const char *end = cp + aCount;
      while (cp < end) {
        *aDest++ = (unsigned char)(*cp++);
      }
    }
  }
}

void
nsTextFragment::Append(const PRUnichar* aBuffer, PRUint32 aLength)
{
  // This is a common case because some callsites create a textnode
  // with a value by creating the node and then calling AppendData.
  if (mState.mLength == 0) {
    SetTo(aBuffer, aLength);

    return;
  }

  // Should we optimize for aData.Length() == 0?

  if (mState.mIs2b) {
    // Already a 2-byte string so the result will be too
    PRUnichar* buff = (PRUnichar*)nsMemory::Realloc(m2b, (mState.mLength + aLength) * sizeof(PRUnichar));
    if (!buff) {
      return;
    }
    
    memcpy(buff + mState.mLength, aBuffer, aLength * sizeof(PRUnichar));
    mState.mLength += aLength;
    m2b = buff;

    return;
  }

  // Current string is a 1-byte string, check if the new data fits in one byte too.

  const PRUnichar* ucp = aBuffer;
  const PRUnichar* uend = ucp + aLength;
  PRBool need2 = PR_FALSE;
  while (ucp < uend) {
    PRUnichar ch = *ucp++;
    if (ch >= 256) {
      need2 = PR_TRUE;
      break;
    }
  }

  if (need2) {
    // The old data was 1-byte, but the new is not so we have to expand it
    // all to 2-byte
    PRUnichar* buff = (PRUnichar*)nsMemory::Alloc((mState.mLength + aLength) *
                                                  sizeof(PRUnichar));
    if (!buff) {
      return;
    }

    // Copy data
    for (PRUint32 i = 0; i < mState.mLength; ++i) {
      buff[i] = (unsigned char)m1b[i];
    }
    
    memcpy(buff + mState.mLength, aBuffer, aLength * sizeof(PRUnichar));

    mState.mLength += aLength;
    mState.mIs2b = PR_TRUE;

    if (mState.mInHeap) {
      nsMemory::Free(m2b);
    }
    m2b = buff;

    mState.mInHeap = PR_TRUE;

    return;
  }

  // The new and the old data is all 1-byte
  char* buff;
  if (mState.mInHeap) {
    buff = (char*)nsMemory::Realloc(const_cast<char*>(m1b),
                                    (mState.mLength + aLength) * sizeof(char));
    if (!buff) {
      return;
    }
  }
  else {
    buff = (char*)nsMemory::Alloc((mState.mLength + aLength) * sizeof(char));
    if (!buff) {
      return;
    }

    memcpy(buff, m1b, mState.mLength);
    mState.mInHeap = PR_TRUE;
  }
    
  for (PRUint32 i = 0; i < aLength; ++i) {
    buff[mState.mLength + i] = (char)aBuffer[i];
  }

  m1b = buff;
  mState.mLength += aLength;

}

// To save time we only do this when we really want to know, not during
// every allocation
void
nsTextFragment::SetBidiFlag()
{
  if (mState.mIs2b && !mState.mIsBidi) {
    const PRUnichar* cp = m2b;
    const PRUnichar* end = cp + mState.mLength;
    while (cp < end) {
      PRUnichar ch1 = *cp++;
      PRUint32 utf32Char = ch1;
      if (NS_IS_HIGH_SURROGATE(ch1) &&
          cp < end &&
          NS_IS_LOW_SURROGATE(*cp)) {
        PRUnichar ch2 = *cp++;
        utf32Char = SURROGATE_TO_UCS4(ch1, ch2);
      }
      if (UTF32_CHAR_IS_BIDI(utf32Char) || IS_BIDI_CONTROL_CHAR(utf32Char)) {
        mState.mIsBidi = PR_TRUE;
        break;
      }
    }
  }
}
