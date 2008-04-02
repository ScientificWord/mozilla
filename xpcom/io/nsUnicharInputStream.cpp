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

#include "nsUnicharInputStream.h"
#include "nsIInputStream.h"
#include "nsIByteBuffer.h"
#include "nsIUnicharBuffer.h"
#include "nsIServiceManager.h"
#include "nsString.h"
#include "nsAutoPtr.h"
#include "nsCRT.h"
#include "nsUTF8Utils.h"
#include <fcntl.h>
#if defined(NS_WIN32)
#include <io.h>
#else
#include <unistd.h>
#endif

#define STRING_BUFFER_SIZE 8192

class StringUnicharInputStream : public nsIUnicharInputStream {
public:
  StringUnicharInputStream(const nsAString& aString) :
    mString(aString), mPos(0), mLen(aString.Length()) { }

  NS_DECL_ISUPPORTS
  NS_DECL_NSIUNICHARINPUTSTREAM

  nsString mString;
  PRUint32 mPos;
  PRUint32 mLen;

private:
  ~StringUnicharInputStream() { }
};

NS_IMETHODIMP
StringUnicharInputStream::Read(PRUnichar* aBuf,
                               PRUint32 aCount,
                               PRUint32 *aReadCount)
{
  if (mPos >= mLen) {
    *aReadCount = 0;
    return NS_OK;
  }
  nsAString::const_iterator iter;
  mString.BeginReading(iter);
  const PRUnichar* us = iter.get();
  PRUint32 amount = mLen - mPos;
  if (amount > aCount) {
    amount = aCount;
  }
  memcpy(aBuf, us + mPos, sizeof(PRUnichar) * amount);
  mPos += amount;
  *aReadCount = amount;
  return NS_OK;
}

NS_IMETHODIMP
StringUnicharInputStream::ReadSegments(nsWriteUnicharSegmentFun aWriter,
                                       void* aClosure,
                                       PRUint32 aCount, PRUint32 *aReadCount)
{
  PRUint32 bytesWritten;
  PRUint32 totalBytesWritten = 0;

  nsresult rv;
  aCount = PR_MIN(mString.Length() - mPos, aCount);
  
  nsAString::const_iterator iter;
  mString.BeginReading(iter);
  
  while (aCount) {
    rv = aWriter(this, aClosure, iter.get() + mPos,
                 totalBytesWritten, aCount, &bytesWritten);
    
    if (NS_FAILED(rv)) {
      // don't propagate errors to the caller
      break;
    }
    
    aCount -= bytesWritten;
    totalBytesWritten += bytesWritten;
    mPos += bytesWritten;
  }
  
  *aReadCount = totalBytesWritten;
  
  return NS_OK;
}

NS_IMETHODIMP
StringUnicharInputStream::ReadString(PRUint32 aCount, nsAString& aString,
                                     PRUint32* aReadCount)
{
  if (mPos >= mLen) {
    *aReadCount = 0;
    return NS_OK;
  }
  PRUint32 amount = mLen - mPos;
  if (amount > aCount) {
    amount = aCount;
  }
  aString = Substring(mString, mPos, amount);
  mPos += amount;
  *aReadCount = amount;
  return NS_OK;
}

nsresult StringUnicharInputStream::Close()
{
  mPos = mLen;
  return NS_OK;
}

NS_IMPL_ISUPPORTS1(StringUnicharInputStream, nsIUnicharInputStream)

//----------------------------------------------------------------------

class UTF8InputStream : public nsIUnicharInputStream {
public:
  UTF8InputStream();
  nsresult Init(nsIInputStream* aStream);

  NS_DECL_ISUPPORTS
  NS_DECL_NSIUNICHARINPUTSTREAM

private:
  ~UTF8InputStream();

protected:
  PRInt32 Fill(nsresult * aErrorCode);

  static void CountValidUTF8Bytes(const char *aBuf, PRUint32 aMaxBytes, PRUint32& aValidUTF8bytes, PRUint32& aValidUTF16CodeUnits);

  nsCOMPtr<nsIInputStream> mInput;
  nsCOMPtr<nsIByteBuffer> mByteData;
  nsCOMPtr<nsIUnicharBuffer> mUnicharData;
  
  PRUint32 mByteDataOffset;
  PRUint32 mUnicharDataOffset;
  PRUint32 mUnicharDataLength;
};

UTF8InputStream::UTF8InputStream() :
  mByteDataOffset(0),
  mUnicharDataOffset(0),
  mUnicharDataLength(0)
{
}

nsresult 
UTF8InputStream::Init(nsIInputStream* aStream)
{
  nsresult rv = NS_NewByteBuffer(getter_AddRefs(mByteData), nsnull,
                                 STRING_BUFFER_SIZE);
  if (NS_FAILED(rv)) return rv;
  rv = NS_NewUnicharBuffer(getter_AddRefs(mUnicharData), nsnull,
                           STRING_BUFFER_SIZE);
  if (NS_FAILED(rv)) return rv;

  mInput = aStream;

  return NS_OK;
}

NS_IMPL_ISUPPORTS1(UTF8InputStream,nsIUnicharInputStream)

UTF8InputStream::~UTF8InputStream()
{
  Close();
}

nsresult UTF8InputStream::Close()
{
  mInput = nsnull;
  mByteData = nsnull;
  mUnicharData = nsnull;

  return NS_OK;
}

nsresult UTF8InputStream::Read(PRUnichar* aBuf,
                               PRUint32 aCount,
                               PRUint32 *aReadCount)
{
  NS_ASSERTION(mUnicharDataLength >= mUnicharDataOffset, "unsigned madness");
  PRUint32 readCount = mUnicharDataLength - mUnicharDataOffset;
  nsresult errorCode;
  if (0 == readCount) {
    // Fill the unichar buffer
    readCount = Fill(&errorCode);
    if (readCount <= 0) {
      *aReadCount = 0;
      return errorCode;
    }
  }
  if (readCount > aCount) {
    readCount = aCount;
  }
  memcpy(aBuf, mUnicharData->GetBuffer() + mUnicharDataOffset,
         readCount * sizeof(PRUnichar));
  mUnicharDataOffset += readCount;
  *aReadCount = readCount;
  return NS_OK;
}

NS_IMETHODIMP
UTF8InputStream::ReadSegments(nsWriteUnicharSegmentFun aWriter,
                              void* aClosure,
                              PRUint32 aCount, PRUint32 *aReadCount)
{
  NS_ASSERTION(mUnicharDataLength >= mUnicharDataOffset, "unsigned madness");
  PRUint32 bytesToWrite = mUnicharDataLength - mUnicharDataOffset;
  nsresult rv = NS_OK;
  if (0 == bytesToWrite) {
    // Fill the unichar buffer
    bytesToWrite = Fill(&rv);
    if (bytesToWrite <= 0) {
      *aReadCount = 0;
      return rv;
    }
  }
  
  if (bytesToWrite > aCount)
    bytesToWrite = aCount;
  
  PRUint32 bytesWritten;
  PRUint32 totalBytesWritten = 0;

  while (bytesToWrite) {
    rv = aWriter(this, aClosure,
                 mUnicharData->GetBuffer() + mUnicharDataOffset,
                 totalBytesWritten, bytesToWrite, &bytesWritten);

    if (NS_FAILED(rv)) {
      // don't propagate errors to the caller
      break;
    }
    
    bytesToWrite -= bytesWritten;
    totalBytesWritten += bytesWritten;
    mUnicharDataOffset += bytesWritten;
  }

  *aReadCount = totalBytesWritten;
  
  return NS_OK;
}

NS_IMETHODIMP
UTF8InputStream::ReadString(PRUint32 aCount, nsAString& aString,
                            PRUint32* aReadCount)
{
  NS_ASSERTION(mUnicharDataLength >= mUnicharDataOffset, "unsigned madness");
  PRUint32 readCount = mUnicharDataLength - mUnicharDataOffset;
  nsresult errorCode;
  if (0 == readCount) {
    // Fill the unichar buffer
    readCount = Fill(&errorCode);
    if (readCount <= 0) {
      *aReadCount = 0;
      return errorCode;
    }
  }
  if (readCount > aCount) {
    readCount = aCount;
  }
  const PRUnichar* buf = reinterpret_cast<const PRUnichar*>(mUnicharData->GetBuffer() +
                                             mUnicharDataOffset);
  aString.Assign(buf, readCount);

  mUnicharDataOffset += readCount;
  *aReadCount = readCount;
  return NS_OK;
}


PRInt32 UTF8InputStream::Fill(nsresult * aErrorCode)
{
  if (nsnull == mInput) {
    // We already closed the stream!
    *aErrorCode = NS_BASE_STREAM_CLOSED;
    return -1;
  }

  NS_ASSERTION(mByteData->GetLength() >= mByteDataOffset, "unsigned madness");
  PRUint32 remainder = mByteData->GetLength() - mByteDataOffset;
  mByteDataOffset = remainder;
  PRInt32 nb = mByteData->Fill(aErrorCode, mInput, remainder);
  if (nb <= 0) {
    // Because we assume a many to one conversion, the lingering data
    // in the byte buffer must be a partial conversion
    // fragment. Because we know that we have received no more new
    // data to add to it, we can't convert it. Therefore, we discard
    // it.
    return nb;
  }
  NS_ASSERTION(remainder + nb == mByteData->GetLength(), "bad nb");

  // Now convert as much of the byte buffer to unicode as possible
  PRUint32 srcLen, dstLen;
  CountValidUTF8Bytes(mByteData->GetBuffer(),remainder + nb, srcLen, dstLen);

  // the number of UCS2 characters should always be <= the number of
  // UTF8 chars
  NS_ASSERTION( (remainder+nb >= srcLen), "cannot be longer than out buffer");
  NS_ASSERTION(PRInt32(dstLen) <= mUnicharData->GetBufferSize(),
               "Ouch. I would overflow my buffer if I wasn't so careful.");
  if (PRInt32(dstLen) > mUnicharData->GetBufferSize()) return 0;
  
  ConvertUTF8toUTF16 converter(mUnicharData->GetBuffer());
  
  nsASingleFragmentCString::const_char_iterator start = mByteData->GetBuffer();
  nsASingleFragmentCString::const_char_iterator end = mByteData->GetBuffer() + srcLen;
            
  copy_string(start, end, converter);
  NS_ASSERTION(converter.Length() == dstLen, "length mismatch");
               
  mUnicharDataOffset = 0;
  mUnicharDataLength = dstLen;
  mByteDataOffset = srcLen;
  
  return dstLen;
}

void
UTF8InputStream::CountValidUTF8Bytes(const char* aBuffer, PRUint32 aMaxBytes, PRUint32& aValidUTF8bytes, PRUint32& aValidUTF16CodeUnits)
{
  const char *c = aBuffer;
  const char *end = aBuffer + aMaxBytes;
  const char *lastchar = c;     // pre-initialize in case of 0-length buffer
  PRUint32 utf16length = 0;
  while (c < end && *c) {
    lastchar = c;
    utf16length++;
    
    if (UTF8traits::isASCII(*c))
      c++;
    else if (UTF8traits::is2byte(*c))
      c += 2;
    else if (UTF8traits::is3byte(*c))
      c += 3;
    else if (UTF8traits::is4byte(*c)) {
      c += 4;
      utf16length++; // add 1 more because this will be converted to a
                     // surrogate pair.
    }
    else if (UTF8traits::is5byte(*c))
      c += 5;
    else if (UTF8traits::is6byte(*c))
      c += 6;
    else {
      NS_WARNING("Unrecognized UTF8 string in UTF8InputStream::CountValidUTF8Bytes()");
      break; // Otherwise we go into an infinite loop.  But what happens now?
    }
  }
  if (c > end) {
    c = lastchar;
    utf16length--;
  }

  aValidUTF8bytes = c - aBuffer;
  aValidUTF16CodeUnits = utf16length;
}

NS_IMPL_QUERY_INTERFACE2(nsSimpleUnicharStreamFactory,
                         nsIFactory,
                         nsISimpleUnicharStreamFactory)

NS_IMETHODIMP_(nsrefcnt) nsSimpleUnicharStreamFactory::AddRef() { return 2; }
NS_IMETHODIMP_(nsrefcnt) nsSimpleUnicharStreamFactory::Release() { return 1; }

NS_IMETHODIMP
nsSimpleUnicharStreamFactory::CreateInstance(nsISupports* aOuter, REFNSIID aIID,
                                            void **aResult)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
nsSimpleUnicharStreamFactory::LockFactory(PRBool aLock)
{
  return NS_OK;
}

NS_IMETHODIMP
nsSimpleUnicharStreamFactory::CreateInstanceFromString(const nsAString& aString,
                                                      nsIUnicharInputStream* *aResult)
{
  StringUnicharInputStream* it = new StringUnicharInputStream(aString);
  if (!it) {
    return NS_ERROR_OUT_OF_MEMORY;
  }

  NS_ADDREF(*aResult = it);
  return NS_OK;
}

NS_IMETHODIMP
nsSimpleUnicharStreamFactory::CreateInstanceFromUTF8Stream(nsIInputStream* aStreamToWrap,
                                                           nsIUnicharInputStream* *aResult)
{
  *aResult = nsnull;

  // Create converter input stream
  nsRefPtr<UTF8InputStream> it = new UTF8InputStream();
  if (!it)
    return NS_ERROR_OUT_OF_MEMORY;

  nsresult rv = it->Init(aStreamToWrap);
  if (NS_FAILED(rv))
    return rv;

  NS_ADDREF(*aResult = it);
  return NS_OK;
}

nsSimpleUnicharStreamFactory*
nsSimpleUnicharStreamFactory::GetInstance()
{
  return const_cast<nsSimpleUnicharStreamFactory*>(&kInstance);
}

const nsSimpleUnicharStreamFactory
nsSimpleUnicharStreamFactory::kInstance;
