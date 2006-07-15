/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef nsBufferedStreams_h__
#define nsBufferedStreams_h__

#include "nsIBufferedStreams.h"
#include "nsIInputStream.h"
#include "nsIOutputStream.h"
#include "nsISafeOutputStream.h"
#include "nsISeekableStream.h"
#include "nsIStreamBufferAccess.h"
#include "nsCOMPtr.h"
#include "nsInt64.h"
////////////////////////////////////////////////////////////////////////////////

class nsBufferedStream : public nsISeekableStream
{
public:
    NS_DECL_ISUPPORTS
    NS_DECL_NSISEEKABLESTREAM

    nsBufferedStream();
    virtual ~nsBufferedStream();

    nsresult Close();

protected:
    nsresult Init(nsISupports* stream, PRUint32 bufferSize);
    NS_IMETHOD Fill() = 0;
    NS_IMETHOD Flush() = 0;

    PRUint32                    mBufferSize;
    char*                       mBuffer;

    // mBufferStartOffset is the offset relative to the start of mStream.
    nsInt64                     mBufferStartOffset;

    // mCursor is the read cursor for input streams, or write cursor for
    // output streams, and is relative to mBufferStartOffset.
    PRUint32                    mCursor;

    // mFillPoint is the amount available in the buffer for input streams,
    // or the high watermark of bytes written into the buffer, and therefore
    // is relative to mBufferStartOffset.
    PRUint32                    mFillPoint;

    nsISupports*                mStream;        // cast to appropriate subclass

    PRPackedBool                mBufferDisabled;
    PRUint8                     mGetBufferCount;
};

////////////////////////////////////////////////////////////////////////////////

class nsBufferedInputStream : public nsBufferedStream,
                              public nsIBufferedInputStream,
                              public nsIStreamBufferAccess
{
public:
    NS_DECL_ISUPPORTS_INHERITED
    NS_DECL_NSIINPUTSTREAM
    NS_DECL_NSIBUFFEREDINPUTSTREAM
    NS_DECL_NSISTREAMBUFFERACCESS

    nsBufferedInputStream() : nsBufferedStream() {}
    virtual ~nsBufferedInputStream() {}

    static NS_METHOD
    Create(nsISupports *aOuter, REFNSIID aIID, void **aResult);

    nsIInputStream* Source() { 
        return (nsIInputStream*)mStream;
    }

protected:
    NS_IMETHOD Fill();
    NS_IMETHOD Flush() { return NS_OK; } // no-op for input streams
};

////////////////////////////////////////////////////////////////////////////////

class nsBufferedOutputStream : public nsBufferedStream, 
                               public nsISafeOutputStream,
                               public nsIBufferedOutputStream,
                               public nsIStreamBufferAccess
{
public:
    NS_DECL_ISUPPORTS_INHERITED
    NS_DECL_NSIOUTPUTSTREAM
    NS_DECL_NSISAFEOUTPUTSTREAM
    NS_DECL_NSIBUFFEREDOUTPUTSTREAM
    NS_DECL_NSISTREAMBUFFERACCESS

    nsBufferedOutputStream() : nsBufferedStream() {}
    virtual ~nsBufferedOutputStream() { nsBufferedOutputStream::Close(); }

    static NS_METHOD
    Create(nsISupports *aOuter, REFNSIID aIID, void **aResult);

    nsIOutputStream* Sink() { 
        return (nsIOutputStream*)mStream;
    }

protected:
    NS_IMETHOD Fill() { return NS_OK; } // no-op for input streams

    nsCOMPtr<nsISafeOutputStream> mSafeStream; // QI'd from mStream
};

////////////////////////////////////////////////////////////////////////////////

#endif // nsBufferedStreams_h__
