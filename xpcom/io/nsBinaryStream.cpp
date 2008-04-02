/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * ***** BEGIN LICENSE BLOCK *****
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
 * The Original Code is Mozilla Communicator client code, released
 * March 31, 1998.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998-1999
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

/**
 * This file contains implementations of the nsIBinaryInputStream and
 * nsIBinaryOutputStream interfaces.  Together, these interfaces allows reading
 * and writing of primitive data types (integers, floating-point values,
 * booleans, etc.) to a stream in a binary, untagged, fixed-endianness format.
 * This might be used, for example, to implement network protocols or to
 * produce architecture-neutral binary disk files, i.e. ones that can be read
 * and written by both big-endian and little-endian platforms.  Output is
 * written in big-endian order (high-order byte first), as this is traditional
 * network order.
 *
 * @See nsIBinaryInputStream
 * @See nsIBinaryOutputStream
 */
#include <string.h>
#include "nsBinaryStream.h"
#include "nsCRT.h"
#include "nsIStreamBufferAccess.h"
#include "nsMemory.h"
#include "prlong.h"
#include "nsGenericFactory.h"
#include "nsString.h"
#include "nsISerializable.h"
#include "nsIClassInfo.h"
#include "nsComponentManagerUtils.h"

NS_IMPL_ISUPPORTS3(nsBinaryOutputStream, nsIObjectOutputStream, nsIBinaryOutputStream, nsIOutputStream)

NS_IMETHODIMP
nsBinaryOutputStream::Flush() 
{ 
    NS_ENSURE_STATE(mOutputStream);
    return mOutputStream->Flush(); 
}

NS_IMETHODIMP
nsBinaryOutputStream::Close() 
{ 
    NS_ENSURE_STATE(mOutputStream);
    return mOutputStream->Close(); 
}

NS_IMETHODIMP
nsBinaryOutputStream::Write(const char *aBuf, PRUint32 aCount, PRUint32 *aActualBytes)
{
    NS_ENSURE_STATE(mOutputStream);
    return mOutputStream->Write(aBuf, aCount, aActualBytes);
}

NS_IMETHODIMP
nsBinaryOutputStream::WriteFrom(nsIInputStream *inStr, PRUint32 count, PRUint32 *_retval)
{
    NS_NOTREACHED("WriteFrom");
    return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
nsBinaryOutputStream::WriteSegments(nsReadSegmentFun reader, void * closure, PRUint32 count, PRUint32 *_retval)
{
    NS_NOTREACHED("WriteSegments");
    return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
nsBinaryOutputStream::IsNonBlocking(PRBool *aNonBlocking)
{
    NS_ENSURE_STATE(mOutputStream);
    return mOutputStream->IsNonBlocking(aNonBlocking);
}

nsresult
nsBinaryOutputStream::WriteFully(const char *aBuf, PRUint32 aCount)
{
    NS_ENSURE_STATE(mOutputStream);

    nsresult rv;
    PRUint32 bytesWritten;

    rv = mOutputStream->Write(aBuf, aCount, &bytesWritten);
    if (NS_FAILED(rv)) return rv;
    if (bytesWritten != aCount)
        return NS_ERROR_FAILURE;
    return NS_OK;
}

NS_IMETHODIMP
nsBinaryOutputStream::SetOutputStream(nsIOutputStream *aOutputStream)
{
    NS_ENSURE_ARG_POINTER(aOutputStream);
    mOutputStream = aOutputStream;
    mBufferAccess = do_QueryInterface(aOutputStream);
    return NS_OK;
}

NS_IMETHODIMP
nsBinaryOutputStream::WriteBoolean(PRBool aBoolean)
{
    return Write8(aBoolean);
}

NS_IMETHODIMP
nsBinaryOutputStream::Write8(PRUint8 aByte)
{
    return WriteFully((const char*)&aByte, sizeof aByte);
}

NS_IMETHODIMP
nsBinaryOutputStream::Write16(PRUint16 a16)
{
    a16 = NS_SWAP16(a16);
    return WriteFully((const char*)&a16, sizeof a16);
}

NS_IMETHODIMP
nsBinaryOutputStream::Write32(PRUint32 a32)
{
    a32 = NS_SWAP32(a32);
    return WriteFully((const char*)&a32, sizeof a32);
}

NS_IMETHODIMP
nsBinaryOutputStream::Write64(PRUint64 a64)
{
    nsresult rv;
    PRUint32 bytesWritten;

    a64 = NS_SWAP64(a64);
    rv = Write(reinterpret_cast<char*>(&a64), sizeof a64, &bytesWritten);
    if (NS_FAILED(rv)) return rv;
    if (bytesWritten != sizeof a64)
        return NS_ERROR_FAILURE;
    return rv;
}

NS_IMETHODIMP
nsBinaryOutputStream::WriteFloat(float aFloat)
{
    NS_ASSERTION(sizeof(float) == sizeof (PRUint32),
                 "False assumption about sizeof(float)");
    return Write32(*reinterpret_cast<PRUint32*>(&aFloat));
}

NS_IMETHODIMP
nsBinaryOutputStream::WriteDouble(double aDouble)
{
    NS_ASSERTION(sizeof(double) == sizeof(PRUint64),
                 "False assumption about sizeof(double)");
    return Write64(*reinterpret_cast<PRUint64*>(&aDouble));
}

NS_IMETHODIMP
nsBinaryOutputStream::WriteStringZ(const char *aString)
{
    PRUint32 length;
    nsresult rv;

    length = strlen(aString);
    rv = Write32(length);
    if (NS_FAILED(rv)) return rv;
    return WriteFully(aString, length);
}

NS_IMETHODIMP
nsBinaryOutputStream::WriteWStringZ(const PRUnichar* aString)
{
    PRUint32 length, byteCount;
    nsresult rv;

    length = nsCRT::strlen(aString);
    rv = Write32(length);
    if (NS_FAILED(rv)) return rv;

    if (length == 0)
        return NS_OK;
    byteCount = length * sizeof(PRUnichar);

#ifdef IS_BIG_ENDIAN
    rv = WriteBytes(reinterpret_cast<const char*>(aString), byteCount);
#else
    // XXX use WriteSegments here to avoid copy!
    PRUnichar *copy, temp[64];
    if (length <= 64) {
        copy = temp;
    } else {
        copy = reinterpret_cast<PRUnichar*>(nsMemory::Alloc(byteCount));
        if (!copy)
            return NS_ERROR_OUT_OF_MEMORY;
    }
    NS_ASSERTION((PRUptrdiff(aString) & 0x1) == 0, "aString not properly aligned");
    for (PRUint32 i = 0; i < length; i++)
        copy[i] = NS_SWAP16(aString[i]);
    rv = WriteBytes(reinterpret_cast<const char*>(copy), byteCount);
    if (copy != temp)
        nsMemory::Free(copy);
#endif

    return rv;
}

NS_IMETHODIMP
nsBinaryOutputStream::WriteUtf8Z(const PRUnichar* aString)
{
    return WriteStringZ(NS_ConvertUTF16toUTF8(aString).get());
}

NS_IMETHODIMP
nsBinaryOutputStream::WriteBytes(const char *aString, PRUint32 aLength)
{
    nsresult rv;
    PRUint32 bytesWritten;

    rv = Write(aString, aLength, &bytesWritten);
    if (NS_FAILED(rv)) return rv;
    if (bytesWritten != aLength)
        return NS_ERROR_FAILURE;
    return rv;
}

NS_IMETHODIMP
nsBinaryOutputStream::WriteByteArray(PRUint8 *aBytes, PRUint32 aLength)
{
    return WriteBytes(reinterpret_cast<char *>(aBytes), aLength);
}

NS_IMETHODIMP
nsBinaryOutputStream::WriteObject(nsISupports* aObject, PRBool aIsStrongRef)
{
    return WriteCompoundObject(aObject, NS_GET_IID(nsISupports),
                               aIsStrongRef);
}

NS_IMETHODIMP
nsBinaryOutputStream::WriteSingleRefObject(nsISupports* aObject)
{
    return WriteCompoundObject(aObject, NS_GET_IID(nsISupports),
                               PR_TRUE);
}

NS_IMETHODIMP
nsBinaryOutputStream::WriteCompoundObject(nsISupports* aObject,
                                          const nsIID& aIID,
                                          PRBool aIsStrongRef)
{
    // Can't deal with weak refs
    NS_ENSURE_TRUE(aIsStrongRef, NS_ERROR_UNEXPECTED);
    
    nsCOMPtr<nsIClassInfo> classInfo = do_QueryInterface(aObject);
    NS_ENSURE_TRUE(classInfo, NS_ERROR_NOT_AVAILABLE);

    nsCOMPtr<nsISerializable> serializable = do_QueryInterface(aObject);
    NS_ENSURE_TRUE(serializable, NS_ERROR_NOT_AVAILABLE);

    nsCID cid;
    classInfo->GetClassIDNoAlloc(&cid);

    nsresult rv = WriteID(cid);
    NS_ENSURE_SUCCESS(rv, rv);
    
    rv = WriteID(aIID);
    NS_ENSURE_SUCCESS(rv, rv);

    return serializable->Write(this);
}

NS_IMETHODIMP
nsBinaryOutputStream::WriteID(const nsIID& aIID)
{
    nsresult rv = Write32(aIID.m0);
    NS_ENSURE_SUCCESS(rv, rv);

    rv = Write16(aIID.m1);
    NS_ENSURE_SUCCESS(rv, rv);

    rv = Write16(aIID.m2);
    NS_ENSURE_SUCCESS(rv, rv);

    for (int i = 0; i < 8; ++i) {
        rv = Write8(aIID.m3[i]);
        NS_ENSURE_SUCCESS(rv, rv);
    }

    return NS_OK;
}

NS_IMETHODIMP_(char*)
nsBinaryOutputStream::GetBuffer(PRUint32 aLength, PRUint32 aAlignMask)
{
    if (mBufferAccess)
        return mBufferAccess->GetBuffer(aLength, aAlignMask);
    return nsnull;
}

NS_IMETHODIMP_(void)
nsBinaryOutputStream::PutBuffer(char* aBuffer, PRUint32 aLength)
{
    if (mBufferAccess)
        mBufferAccess->PutBuffer(aBuffer, aLength);
}

NS_IMPL_ISUPPORTS3(nsBinaryInputStream, nsIObjectInputStream, nsIBinaryInputStream, nsIInputStream)

NS_IMETHODIMP
nsBinaryInputStream::Available(PRUint32* aResult)
{
    NS_ENSURE_STATE(mInputStream);
    return mInputStream->Available(aResult);
}

NS_IMETHODIMP
nsBinaryInputStream::Read(char* aBuffer, PRUint32 aCount, PRUint32 *aNumRead)
{
    NS_ENSURE_STATE(mInputStream);

    // mInputStream might give us short reads, so deal with that.
    PRUint32 totalRead = 0;

    PRUint32 bytesRead;
    do {
        nsresult rv = mInputStream->Read(aBuffer, aCount, &bytesRead);
        if (rv == NS_BASE_STREAM_WOULD_BLOCK && totalRead != 0) {
            // We already read some data.  Return it.
            break;
        }
        
        if (NS_FAILED(rv)) {
            return rv;
        }

        totalRead += bytesRead;
        aBuffer += bytesRead;
        aCount -= bytesRead;
    } while (aCount != 0 && bytesRead != 0);

    *aNumRead = totalRead;
    
    return NS_OK;
}


// when forwarding ReadSegments to mInputStream, we need to make sure
// 'this' is being passed to the writer each time. To do this, we need
// a thunking function which keeps the real input stream around.

// the closure wrapper
struct ReadSegmentsClosure {
    nsIInputStream* mRealInputStream;
    void* mRealClosure;
    nsWriteSegmentFun mRealWriter;
    nsresult mRealResult;
    PRUint32 mBytesRead;  // to properly implement aToOffset
};

// the thunking function
static NS_METHOD
ReadSegmentForwardingThunk(nsIInputStream* aStream,
                           void *aClosure,
                           const char* aFromSegment,
                           PRUint32 aToOffset,
                           PRUint32 aCount,
                           PRUint32 *aWriteCount)
{
    ReadSegmentsClosure* thunkClosure =
        reinterpret_cast<ReadSegmentsClosure*>(aClosure);

    NS_ASSERTION(NS_SUCCEEDED(thunkClosure->mRealResult),
                 "How did this get to be a failure status?");

    thunkClosure->mRealResult =
        thunkClosure->mRealWriter(thunkClosure->mRealInputStream,
                                  thunkClosure->mRealClosure,
                                  aFromSegment,
                                  thunkClosure->mBytesRead + aToOffset,
                                  aCount, aWriteCount);

    return thunkClosure->mRealResult;
}


NS_IMETHODIMP
nsBinaryInputStream::ReadSegments(nsWriteSegmentFun writer, void * closure, PRUint32 count, PRUint32 *_retval)
{
    NS_ENSURE_STATE(mInputStream);

    ReadSegmentsClosure thunkClosure = { this, closure, writer, NS_OK, 0 };
    
    // mInputStream might give us short reads, so deal with that.
    PRUint32 bytesRead;
    do {
        nsresult rv = mInputStream->ReadSegments(ReadSegmentForwardingThunk,
                                                 &thunkClosure,
                                                 count, &bytesRead);

        if (rv == NS_BASE_STREAM_WOULD_BLOCK && thunkClosure.mBytesRead != 0) {
            // We already read some data.  Return it.
            break;
        }
        
        if (NS_FAILED(rv)) {
            return rv;
        }

        thunkClosure.mBytesRead += bytesRead;
        count -= bytesRead;
    } while (count != 0 && bytesRead != 0 &&
             NS_SUCCEEDED(thunkClosure.mRealResult));

    *_retval = thunkClosure.mBytesRead;

    return NS_OK;
}

NS_IMETHODIMP
nsBinaryInputStream::IsNonBlocking(PRBool *aNonBlocking)
{
    NS_ENSURE_STATE(mInputStream);
    return mInputStream->IsNonBlocking(aNonBlocking);
}

NS_IMETHODIMP
nsBinaryInputStream::Close() 
{ 
    NS_ENSURE_STATE(mInputStream);
    return mInputStream->Close(); 
}

NS_IMETHODIMP
nsBinaryInputStream::SetInputStream(nsIInputStream *aInputStream)
{
    NS_ENSURE_ARG_POINTER(aInputStream);
    mInputStream = aInputStream;
    mBufferAccess = do_QueryInterface(aInputStream);
    return NS_OK;
}

NS_IMETHODIMP
nsBinaryInputStream::ReadBoolean(PRBool* aBoolean)
{
    PRUint8 byteResult;
    nsresult rv = Read8(&byteResult);
    *aBoolean = !!byteResult;
    return rv;
}

NS_IMETHODIMP
nsBinaryInputStream::Read8(PRUint8* aByte)
{
    nsresult rv;
    PRUint32 bytesRead;

    rv = Read(reinterpret_cast<char*>(aByte), sizeof(*aByte), &bytesRead);
    if (NS_FAILED(rv)) return rv;
    if (bytesRead != 1)
        return NS_ERROR_FAILURE;
    return rv;
}

NS_IMETHODIMP
nsBinaryInputStream::Read16(PRUint16* a16)
{
    nsresult rv;
    PRUint32 bytesRead;

    rv = Read(reinterpret_cast<char*>(a16), sizeof *a16, &bytesRead);
    if (NS_FAILED(rv)) return rv;
    if (bytesRead != sizeof *a16)
        return NS_ERROR_FAILURE;
    *a16 = NS_SWAP16(*a16);
    return rv;
}

NS_IMETHODIMP
nsBinaryInputStream::Read32(PRUint32* a32)
{
    nsresult rv;
    PRUint32 bytesRead;

    rv = Read(reinterpret_cast<char*>(a32), sizeof *a32, &bytesRead);
    if (NS_FAILED(rv)) return rv;
    if (bytesRead != sizeof *a32)
        return NS_ERROR_FAILURE;
    *a32 = NS_SWAP32(*a32);
    return rv;
}

NS_IMETHODIMP
nsBinaryInputStream::Read64(PRUint64* a64)
{
    nsresult rv;
    PRUint32 bytesRead;

    rv = Read(reinterpret_cast<char*>(a64), sizeof *a64, &bytesRead);
    if (NS_FAILED(rv)) return rv;
    if (bytesRead != sizeof *a64)
        return NS_ERROR_FAILURE;
    *a64 = NS_SWAP64(*a64);
    return rv;
}

NS_IMETHODIMP
nsBinaryInputStream::ReadFloat(float* aFloat)
{
    NS_ASSERTION(sizeof(float) == sizeof (PRUint32),
                 "False assumption about sizeof(float)");
    return Read32(reinterpret_cast<PRUint32*>(aFloat));
}

NS_IMETHODIMP
nsBinaryInputStream::ReadDouble(double* aDouble)
{
    NS_ASSERTION(sizeof(double) == sizeof(PRUint64),
                 "False assumption about sizeof(double)");
    return Read64(reinterpret_cast<PRUint64*>(aDouble));
}

static NS_METHOD
WriteSegmentToCString(nsIInputStream* aStream,
                      void *aClosure,
                      const char* aFromSegment,
                      PRUint32 aToOffset,
                      PRUint32 aCount,
                      PRUint32 *aWriteCount)
{
    nsACString* outString = static_cast<nsACString*>(aClosure);

    outString->Append(aFromSegment, aCount);

    *aWriteCount = aCount;
    
    return NS_OK;
}

NS_IMETHODIMP
nsBinaryInputStream::ReadCString(nsACString& aString)
{
    nsresult rv;
    PRUint32 length, bytesRead;

    rv = Read32(&length);
    if (NS_FAILED(rv)) return rv;

    aString.Truncate();
    rv = ReadSegments(WriteSegmentToCString, &aString, length, &bytesRead);
    if (NS_FAILED(rv)) return rv;
    
    if (bytesRead != length)
        return NS_ERROR_FAILURE;

    return NS_OK;
}


// sometimes, WriteSegmentToString will be handed an odd-number of
// bytes, which means we only have half of the last PRUnichar
struct WriteStringClosure {
    PRUnichar *mWriteCursor;
    PRPackedBool mHasCarryoverByte;
    char mCarryoverByte;
};

// there are a few cases we have to account for here:
// * even length buffer, no carryover - easy, just append
// * odd length buffer, no carryover - the last byte needs to be saved
//                                     for carryover
// * odd length buffer, with carryover - first byte needs to be used
//                              with the carryover byte, and
//                              the rest of the even length
//                              buffer is appended as normal
// * even length buffer, with carryover - the first byte needs to be
//                              used with the previous carryover byte.
//                              this gives you an odd length buffer,
//                              so you have to save the last byte for
//                              the next carryover


// same version of the above, but with correct casting and endian swapping
static NS_METHOD
WriteSegmentToString(nsIInputStream* aStream,
                     void *aClosure,
                     const char* aFromSegment,
                     PRUint32 aToOffset,
                     PRUint32 aCount,
                     PRUint32 *aWriteCount)
{
    NS_PRECONDITION(aCount > 0, "Why are we being told to write 0 bytes?");
    NS_PRECONDITION(sizeof(PRUnichar) == 2, "We can't handle other sizes!");

    WriteStringClosure* closure = static_cast<WriteStringClosure*>(aClosure);
    PRUnichar *cursor = closure->mWriteCursor;

    // we're always going to consume the whole buffer no matter what
    // happens, so take care of that right now.. that allows us to
    // tweak aCount later. Do NOT move this!
    *aWriteCount = aCount;

    // if the last Write had an odd-number of bytes read, then 
    if (closure->mHasCarryoverByte) {
        // re-create the two-byte sequence we want to work with
        char bytes[2] = { closure->mCarryoverByte, *aFromSegment };
        *cursor = *(PRUnichar*)bytes;
        // Now the little endianness dance
#ifdef IS_LITTLE_ENDIAN
        *cursor = (PRUnichar) NS_SWAP16(*cursor);
#endif
        ++cursor;
        
        // now skip past the first byte of the buffer.. code from here
        // can assume normal operations, but should not assume aCount
        // is relative to the ORIGINAL buffer
        ++aFromSegment;
        --aCount;

        closure->mHasCarryoverByte = PR_FALSE;
    }
    
    // this array is possibly unaligned... be careful how we access it!
    const PRUnichar *unicodeSegment =
        reinterpret_cast<const PRUnichar*>(aFromSegment);

    // calculate number of full characters in segment (aCount could be odd!)
    PRUint32 segmentLength = aCount / sizeof(PRUnichar);

    // copy all data into our aligned buffer.  byte swap if necessary.
    memcpy(cursor, unicodeSegment, segmentLength * sizeof(PRUnichar));
    PRUnichar *end = cursor + segmentLength;
#ifdef IS_LITTLE_ENDIAN
    for (; cursor < end; ++cursor)
        *cursor = (PRUnichar) NS_SWAP16(*cursor);
#endif
    closure->mWriteCursor = end;

    // remember this is the modifed aCount and aFromSegment,
    // so that will take into account the fact that we might have
    // skipped the first byte in the buffer
    if (aCount % sizeof(PRUnichar) != 0) {
        // we must have had a carryover byte, that we'll need the next
        // time around
        closure->mCarryoverByte = aFromSegment[aCount - 1];
        closure->mHasCarryoverByte = PR_TRUE;
    }
    
    return NS_OK;
}


NS_IMETHODIMP
nsBinaryInputStream::ReadString(nsAString& aString)
{
    nsresult rv;
    PRUint32 length, bytesRead;

    rv = Read32(&length);
    if (NS_FAILED(rv)) return rv;

    if (length == 0) {
      aString.Truncate();
      return NS_OK;
    }

    // pre-allocate output buffer, and get direct access to buffer...
    if (!EnsureStringLength(aString, length))
        return NS_ERROR_OUT_OF_MEMORY;

    nsAString::iterator start;
    aString.BeginWriting(start);
    
    WriteStringClosure closure;
    closure.mWriteCursor = start.get();
    closure.mHasCarryoverByte = PR_FALSE;
    
    rv = ReadSegments(WriteSegmentToString, &closure,
                      length*sizeof(PRUnichar), &bytesRead);
    if (NS_FAILED(rv)) return rv;

    NS_ASSERTION(!closure.mHasCarryoverByte, "some strange stream corruption!");
    
    if (bytesRead != length*sizeof(PRUnichar))
        return NS_ERROR_FAILURE;

    return NS_OK;
}

NS_IMETHODIMP
nsBinaryInputStream::ReadBytes(PRUint32 aLength, char* *_rval)
{
    nsresult rv;
    PRUint32 bytesRead;
    char* s;

    s = reinterpret_cast<char*>(nsMemory::Alloc(aLength));
    if (!s)
        return NS_ERROR_OUT_OF_MEMORY;

    rv = Read(s, aLength, &bytesRead);
    if (NS_FAILED(rv)) {
        nsMemory::Free(s);
        return rv;
    }
    if (bytesRead != aLength) {
        nsMemory::Free(s);
        return NS_ERROR_FAILURE;
    }

    *_rval = s;
    return NS_OK;
}

NS_IMETHODIMP
nsBinaryInputStream::ReadByteArray(PRUint32 aLength, PRUint8* *_rval)
{
    return ReadBytes(aLength, reinterpret_cast<char **>(_rval));
}

NS_IMETHODIMP
nsBinaryInputStream::ReadObject(PRBool aIsStrongRef, nsISupports* *aObject)
{
    nsCID cid;
    nsIID iid;
    nsresult rv = ReadID(&cid);
    NS_ENSURE_SUCCESS(rv, rv);

    rv = ReadID(&iid);
    NS_ENSURE_SUCCESS(rv, rv);

    nsCOMPtr<nsISupports> object = do_CreateInstance(cid, &rv);
    NS_ENSURE_SUCCESS(rv, rv);

    nsCOMPtr<nsISerializable> serializable = do_QueryInterface(object);
    NS_ENSURE_TRUE(serializable, NS_ERROR_UNEXPECTED);

    rv = serializable->Read(this);
    NS_ENSURE_SUCCESS(rv, rv);    

    return object->QueryInterface(iid, reinterpret_cast<void**>(aObject));
}

NS_IMETHODIMP
nsBinaryInputStream::ReadID(nsID *aResult)
{
    nsresult rv = Read32(&aResult->m0);
    NS_ENSURE_SUCCESS(rv, rv);

    rv = Read16(&aResult->m1);
    NS_ENSURE_SUCCESS(rv, rv);

    rv = Read16(&aResult->m2);
    NS_ENSURE_SUCCESS(rv, rv);

    for (int i = 0; i < 8; ++i) {
        rv = Read8(&aResult->m3[i]);
        NS_ENSURE_SUCCESS(rv, rv);
    }

    return NS_OK;
}

NS_IMETHODIMP_(char*)
nsBinaryInputStream::GetBuffer(PRUint32 aLength, PRUint32 aAlignMask)
{
    if (mBufferAccess)
        return mBufferAccess->GetBuffer(aLength, aAlignMask);
    return nsnull;
}

NS_IMETHODIMP_(void)
nsBinaryInputStream::PutBuffer(char* aBuffer, PRUint32 aLength)
{
    if (mBufferAccess)
        mBufferAccess->PutBuffer(aBuffer, aLength);
}
