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
 * The Original Code is Zip Writer Component.
 *
 * The Initial Developer of the Original Code is
 * Dave Townsend <dtownsend@oxymoronical.com>.
 *
 * Portions created by the Initial Developer are Copyright (C) 2007
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *      Mook <mook.moz+random.code@gmail.com>
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
 * ***** END LICENSE BLOCK *****
 */

#include "StreamFunctions.h"
#include "nsZipWriter.h"
#include "nsZipDataStream.h"
#include "nsISeekableStream.h"
#include "nsIAsyncStreamCopier.h"
#include "nsIStreamListener.h"
#include "nsIInputStreamPump.h"
#include "nsComponentManagerUtils.h"
#include "nsMemory.h"
#include "nsNetError.h"
#include "nsStreamUtils.h"
#include "nsThreadUtils.h"
#include "nsNetUtil.h"
#include "prio.h"

#define ZIP_EOCDR_HEADER_SIZE 22
#define ZIP_EOCDR_HEADER_SIGNATURE 0x06054b50

/**
 * nsZipWriter is used to create and add to zip files.
 * It is based on the spec available at
 * http://www.pkware.com/documents/casestudies/APPNOTE.TXT.
 * 
 * The basic structure of a zip file created is slightly simpler than that
 * illustrated in the spec because certain features of the zip format are
 * unsupported:
 * 
 * [local file header 1]
 * [file data 1]
 * . 
 * .
 * .
 * [local file header n]
 * [file data n]
 * [central directory]
 * [end of central directory record]
 */
NS_IMPL_ISUPPORTS2(nsZipWriter, nsIZipWriter,
                                nsIRequestObserver)

nsZipWriter::nsZipWriter()
{
    mEntryHash.Init();
    mInQueue = PR_FALSE;
}

nsZipWriter::~nsZipWriter()
{
    if (mStream && !mInQueue)
        Close();
}

/* attribute AString comment; */
NS_IMETHODIMP nsZipWriter::GetComment(nsACString & aComment)
{
    if (!mStream)
        return NS_ERROR_NOT_INITIALIZED;

    aComment = mComment;
    return NS_OK;
}

NS_IMETHODIMP nsZipWriter::SetComment(const nsACString & aComment)
{
    if (!mStream)
        return NS_ERROR_NOT_INITIALIZED;

    mComment = aComment;
    mCDSDirty = PR_TRUE;
    return NS_OK;
}

/* readonly attribute boolean inQueue; */
NS_IMETHODIMP nsZipWriter::GetInQueue(PRBool *aInQueue)
{
    *aInQueue = mInQueue;
    return NS_OK;
}

/* readonly attribute nsIFile file; */
NS_IMETHODIMP nsZipWriter::GetFile(nsIFile **aFile)
{
    nsCOMPtr<nsIFile> file;
    nsresult rv = mFile->Clone(getter_AddRefs(file));
    NS_ENSURE_SUCCESS(rv, rv);

    NS_ADDREF(*aFile = file);
    return NS_OK;
}

/*
 * Reads file entries out of an existing zip file.
 */
nsresult nsZipWriter::ReadFile(nsIFile *aFile)
{
    PRInt64 size;
    nsresult rv = aFile->GetFileSize(&size);
    NS_ENSURE_SUCCESS(rv, rv);

    nsCOMPtr<nsIInputStream> inputStream;
    rv = NS_NewLocalFileInputStream(getter_AddRefs(inputStream), aFile);
    NS_ENSURE_SUCCESS(rv, rv);

    char buf[1024];
    PRInt64 seek = size - 1024;
    PRUint32 length = 1024;

    if (seek < 0) {
        length += seek;
        seek = 0;
    }

    PRUint32 pos;
    PRUint32 sig = 0;
    nsCOMPtr<nsISeekableStream> seekable = do_QueryInterface(inputStream);

    while (true) {
        rv = seekable->Seek(nsISeekableStream::NS_SEEK_SET, seek);
        if (NS_FAILED(rv)) {
            inputStream->Close();
            return rv;
        }
        rv = ZW_ReadData(inputStream, buf, length);
        if (NS_FAILED(rv)) {
            inputStream->Close();
            return rv;
        }

        /*
         * We have to backtrack from the end of the file until we find the
         * CDS signature
         */
        // We know it's at least this far from the end
        pos = length - ZIP_EOCDR_HEADER_SIZE;
        sig = READ32(buf, &pos);
        pos -= 4;
        while (pos >=0) {
            if (sig == ZIP_EOCDR_HEADER_SIGNATURE) {
                // Skip down to entry count
                pos += 10;
                PRUint32 entries = READ16(buf, &pos);
                // Skip past CDS size
                pos += 4;
                mCDSOffset = READ32(buf, &pos);
                PRUint32 commentlen = READ16(buf, &pos);

                if (commentlen == 0)
                    mComment.Truncate();
                else if (pos + commentlen <= length)
                    mComment.Assign(buf + pos, commentlen);
                else {
                    if ((seek + pos + commentlen) > size) {
                        inputStream->Close();
                        return NS_ERROR_FILE_CORRUPTED;
                    }
                    nsAutoArrayPtr<char> field(new char[commentlen]);
                    NS_ENSURE_TRUE(field, NS_ERROR_OUT_OF_MEMORY);
                    rv = seekable->Seek(nsISeekableStream::NS_SEEK_SET,
                                        seek + pos);
                    if (NS_FAILED(rv)) {
                        inputStream->Close();
                        return rv;
                    }
                    rv = ZW_ReadData(inputStream, field.get(), length);
                    if (NS_FAILED(rv)) {
                        inputStream->Close();
                        return rv;
                    }
                    mComment.Assign(field.get(), commentlen);
                }

                rv = seekable->Seek(nsISeekableStream::NS_SEEK_SET,
                                    mCDSOffset);
                if (NS_FAILED(rv)) {
                    inputStream->Close();
                    return rv;
                }

                for (PRUint32 entry = 0; entry < entries; entry++) {
                    nsZipHeader* header = new nsZipHeader();
                    if (!header) {
                        inputStream->Close();
                        mEntryHash.Clear();
                        mHeaders.Clear();
                        return NS_ERROR_OUT_OF_MEMORY;
                    }
                    rv = header->ReadCDSHeader(inputStream);
                    if (NS_FAILED(rv)) {
                        inputStream->Close();
                        mEntryHash.Clear();
                        mHeaders.Clear();
                        return rv;
                    }
                    if (!mEntryHash.Put(header->mName, mHeaders.Count()))
                        return NS_ERROR_OUT_OF_MEMORY;
                    if (!mHeaders.AppendObject(header))
                        return NS_ERROR_OUT_OF_MEMORY;
                }

                return inputStream->Close();
            }
            sig = sig << 8;
            sig += buf[--pos];
        }

        if (seek == 0) {
            // We've reached the start with no signature found. Corrupt.
            inputStream->Close();
            return NS_ERROR_FILE_CORRUPTED;
        }

        // Overlap by the size of the end of cdr
        seek -= (1024 - ZIP_EOCDR_HEADER_SIZE);
        if (seek < 0) {
            length += seek;
            seek = 0;
        }
    }
    // Will never reach here in reality
    NS_NOTREACHED("Loop should never complete");
    return NS_ERROR_UNEXPECTED;
}

/* void open (in nsIFile aFile, in PRInt32 aIoFlags); */
NS_IMETHODIMP nsZipWriter::Open(nsIFile *aFile, PRInt32 aIoFlags)
{
    if (mStream)
        return NS_ERROR_ALREADY_INITIALIZED;

    NS_ENSURE_ARG_POINTER(aFile);

    // Need to be able to write to the file
    if (aIoFlags & PR_RDONLY)
        return NS_ERROR_FAILURE;
    
    nsresult rv = aFile->Clone(getter_AddRefs(mFile));
    NS_ENSURE_SUCCESS(rv, rv);

    PRBool exists;
    rv = mFile->Exists(&exists);
    NS_ENSURE_SUCCESS(rv, rv);
    if (!exists && !(aIoFlags & PR_CREATE_FILE))
        return NS_ERROR_FILE_NOT_FOUND;

    if (exists && !(aIoFlags & (PR_TRUNCATE | PR_WRONLY))) {
        rv = ReadFile(mFile);
        NS_ENSURE_SUCCESS(rv, rv);
        mCDSDirty = PR_FALSE;
    }
    else {
        mCDSOffset = 0;
        mCDSDirty = PR_TRUE;
        mComment.Truncate();
    }

    // Silently drop PR_APPEND
    aIoFlags &= 0xef;

    nsCOMPtr<nsIOutputStream> stream;
    rv = NS_NewLocalFileOutputStream(getter_AddRefs(stream), mFile, aIoFlags);
    if (NS_FAILED(rv)) {
        mHeaders.Clear();
        mEntryHash.Clear();
        return rv;
    }

    rv = NS_NewBufferedOutputStream(getter_AddRefs(mStream), stream, 0x800);
    if (NS_FAILED(rv)) {
        stream->Close();
        mHeaders.Clear();
        mEntryHash.Clear();
        return rv;
    }

    if (mCDSOffset > 0) {
        rv = SeekCDS();
        NS_ENSURE_SUCCESS(rv, rv);
    }

    return NS_OK;
}

/* nsIZipEntry getEntry (in AString aZipEntry); */
NS_IMETHODIMP nsZipWriter::GetEntry(const nsACString & aZipEntry,
                                    nsIZipEntry **_retval)
{
    PRInt32 pos;
    if (mEntryHash.Get(aZipEntry, &pos))
        NS_ADDREF(*_retval = mHeaders[pos]);
    else
        *_retval = nsnull;

    return NS_OK;
}

/* boolean hasEntry (in AString aZipEntry); */
NS_IMETHODIMP nsZipWriter::HasEntry(const nsACString & aZipEntry,
                                    PRBool *_retval)
{
    *_retval = mEntryHash.Get(aZipEntry, nsnull);

    return NS_OK;
}

/* void addEntryDirectory (in AUTF8String aZipEntry, in PRTime aModTime,
 *                         in boolean aQueue); */
NS_IMETHODIMP nsZipWriter::AddEntryDirectory(const nsACString & aZipEntry,
                                             PRTime aModTime, PRBool aQueue)
{
    if (!mStream)
        return NS_ERROR_NOT_INITIALIZED;

    if (aQueue) {
        nsZipQueueItem item;
        item.mOperation = OPERATION_ADD;
        item.mZipEntry = aZipEntry;
        item.mModTime = aModTime;
        if (!mQueue.AppendElement(item))
            return NS_ERROR_OUT_OF_MEMORY;
        return NS_OK;
    }

    if (mInQueue)
        return NS_ERROR_IN_PROGRESS;
    return InternalAddEntryDirectory(aZipEntry, aModTime);
}

/* void addEntryFile (in AUTF8String aZipEntry, in PRInt32 aCompression,
 *                    in nsIFile aFile, in boolean aQueue); */
NS_IMETHODIMP nsZipWriter::AddEntryFile(const nsACString & aZipEntry,
                                        PRInt32 aCompression, nsIFile *aFile,
                                        PRBool aQueue)
{
    NS_ENSURE_ARG_POINTER(aFile);
    if (!mStream)
        return NS_ERROR_NOT_INITIALIZED;

    nsresult rv;
    if (aQueue) {
        nsZipQueueItem item;
        item.mOperation = OPERATION_ADD;
        item.mZipEntry = aZipEntry;
        item.mCompression = aCompression;
        rv = aFile->Clone(getter_AddRefs(item.mFile));
        NS_ENSURE_SUCCESS(rv, rv);
        if (!mQueue.AppendElement(item))
            return NS_ERROR_OUT_OF_MEMORY;
        return NS_OK;
    }

    if (mInQueue)
        return NS_ERROR_IN_PROGRESS;

    PRBool exists;
    rv = aFile->Exists(&exists);
    NS_ENSURE_SUCCESS(rv, rv);
    if (!exists)
        return NS_ERROR_FILE_NOT_FOUND;

    PRBool isdir;
    rv = aFile->IsDirectory(&isdir);
    NS_ENSURE_SUCCESS(rv, rv);

    PRInt64 modtime;
    rv = aFile->GetLastModifiedTime(&modtime);
    NS_ENSURE_SUCCESS(rv, rv);
    modtime *= PR_USEC_PER_MSEC;

    if (isdir)
        return InternalAddEntryDirectory(aZipEntry, modtime);

    if (mEntryHash.Get(aZipEntry, nsnull))
        return NS_ERROR_FILE_ALREADY_EXISTS;

    nsCOMPtr<nsIInputStream> inputStream;
    rv = NS_NewLocalFileInputStream(getter_AddRefs(inputStream),
                                    aFile);
    NS_ENSURE_SUCCESS(rv, rv);

    rv = AddEntryStream(aZipEntry, modtime, aCompression, inputStream,
                        PR_FALSE);
    NS_ENSURE_SUCCESS(rv, rv);

    return inputStream->Close();
}

/* void addEntryChannel (in AUTF8String aZipEntry, in PRTime aModTime,
 *                       in PRInt32 aCompression, in nsIChannel aChannel,
 *                       in boolean aQueue); */
NS_IMETHODIMP nsZipWriter::AddEntryChannel(const nsACString & aZipEntry,
                                           PRTime aModTime,
                                           PRInt32 aCompression,
                                           nsIChannel *aChannel,
                                           PRBool aQueue)
{
    NS_ENSURE_ARG_POINTER(aChannel);
    if (!mStream)
        return NS_ERROR_NOT_INITIALIZED;

    if (aQueue) {
        nsZipQueueItem item;
        item.mOperation = OPERATION_ADD;
        item.mZipEntry = aZipEntry;
        item.mModTime = aModTime;
        item.mCompression = aCompression;
        item.mChannel = aChannel;
        if (!mQueue.AppendElement(item))
            return NS_ERROR_OUT_OF_MEMORY;
        return NS_OK;
    }

    if (mInQueue)
        return NS_ERROR_IN_PROGRESS;
    if (mEntryHash.Get(aZipEntry, nsnull))
        return NS_ERROR_FILE_ALREADY_EXISTS;

    nsCOMPtr<nsIInputStream> inputStream;
    nsresult rv = aChannel->Open(getter_AddRefs(inputStream));
    NS_ENSURE_SUCCESS(rv, rv);

    rv = AddEntryStream(aZipEntry, aModTime, aCompression, inputStream,
                        PR_FALSE);
    NS_ENSURE_SUCCESS(rv, rv);

    return inputStream->Close();
}

/* void addEntryStream (in AUTF8String aZipEntry, in PRTime aModTime,
 *                      in PRInt32 aCompression, in nsIInputStream aStream,
 *                      in boolean aQueue); */
NS_IMETHODIMP nsZipWriter::AddEntryStream(const nsACString & aZipEntry,
                                          PRTime aModTime,
                                          PRInt32 aCompression,
                                          nsIInputStream *aStream,
                                          PRBool aQueue)
{
    NS_ENSURE_ARG_POINTER(aStream);
    if (!mStream)
        return NS_ERROR_NOT_INITIALIZED;

    if (aQueue) {
        nsZipQueueItem item;
        item.mOperation = OPERATION_ADD;
        item.mZipEntry = aZipEntry;
        item.mModTime = aModTime;
        item.mCompression = aCompression;
        item.mStream = aStream;
        if (!mQueue.AppendElement(item))
            return NS_ERROR_OUT_OF_MEMORY;
        return NS_OK;
    }

    if (mInQueue)
        return NS_ERROR_IN_PROGRESS;
    if (mEntryHash.Get(aZipEntry, nsnull))
        return NS_ERROR_FILE_ALREADY_EXISTS;

    nsRefPtr<nsZipHeader> header = new nsZipHeader();
    NS_ENSURE_TRUE(header, NS_ERROR_OUT_OF_MEMORY);
    header->Init(aZipEntry, aModTime, ZIP_ATTRS_FILE, mCDSOffset);
    nsresult rv = header->WriteFileHeader(mStream);
    if (NS_FAILED(rv)) {
        SeekCDS();
        return rv;
    }

    nsRefPtr<nsZipDataStream> stream = new nsZipDataStream();
    if (!stream) {
        SeekCDS();
        return NS_ERROR_OUT_OF_MEMORY;
    }
    rv = stream->Init(this, mStream, header, aCompression);
    if (NS_FAILED(rv)) {
        SeekCDS();
        return rv;
    }

    rv = stream->ReadStream(aStream);
    if (NS_FAILED(rv))
        SeekCDS();
    return rv;
}

/* void removeEntry (in AUTF8String aZipEntry, in boolean aQueue); */
NS_IMETHODIMP nsZipWriter::RemoveEntry(const nsACString & aZipEntry,
                                       PRBool aQueue)
{
    if (!mStream)
        return NS_ERROR_NOT_INITIALIZED;

    if (aQueue) {
        nsZipQueueItem item;
        item.mOperation = OPERATION_REMOVE;
        item.mZipEntry = aZipEntry;
        if (!mQueue.AppendElement(item))
            return NS_ERROR_OUT_OF_MEMORY;
        return NS_OK;
    }

    if (mInQueue)
        return NS_ERROR_IN_PROGRESS;

    PRInt32 pos;
    if (mEntryHash.Get(aZipEntry, &pos)) {
        // Flush any remaining data before we seek.
        nsresult rv = mStream->Flush();
        NS_ENSURE_SUCCESS(rv, rv);
        if (pos < mHeaders.Count() - 1) {
            // This is not the last entry, pull back the data.
            nsCOMPtr<nsISeekableStream> seekable = do_QueryInterface(mStream);
            rv = seekable->Seek(nsISeekableStream::NS_SEEK_SET,
                                mHeaders[pos]->mOffset);
            NS_ENSURE_SUCCESS(rv, rv);

            nsCOMPtr<nsIInputStream> inputStream;
            rv = NS_NewLocalFileInputStream(getter_AddRefs(inputStream),
                                            mFile);
            NS_ENSURE_SUCCESS(rv, rv);
            seekable = do_QueryInterface(inputStream);
            rv = seekable->Seek(nsISeekableStream::NS_SEEK_SET,
                                mHeaders[pos + 1]->mOffset);
            if (NS_FAILED(rv)) {
                inputStream->Close();
                return rv;
            }

            PRUint32 count = mCDSOffset - mHeaders[pos + 1]->mOffset;
            PRUint32 read = 0;
            char buf[4096];
            while (count > 0) {
                if (count < sizeof(buf))
                    read = count;
                else
                    read = sizeof(buf);

                rv = inputStream->Read(buf, read, &read);
                if (NS_FAILED(rv)) {
                    inputStream->Close();
                    Cleanup();
                    return rv;
                }

                rv = ZW_WriteData(mStream, buf, read);
                if (NS_FAILED(rv)) {
                    inputStream->Close();
                    Cleanup();
                    return rv;
                }

                count -= read;
            }
            inputStream->Close();

            // Rewrite header offsets and update hash
            PRUint32 shift = (mHeaders[pos + 1]->mOffset -
                              mHeaders[pos]->mOffset);
            mCDSOffset -= shift;
            PRInt32 pos2 = pos + 1;
            while (pos2 < mHeaders.Count()) {
                if (!mEntryHash.Put(mHeaders[pos2]->mName, pos2-1)) {
                    Cleanup();
                    return NS_ERROR_OUT_OF_MEMORY;
                }
                mHeaders[pos2]->mOffset -= shift;
                pos2++;
            }
        }
        else {
            // Remove the last entry is just a case of moving the CDS
            mCDSOffset = mHeaders[pos]->mOffset;
            rv = SeekCDS();
            NS_ENSURE_SUCCESS(rv, rv);
        }

        mEntryHash.Remove(mHeaders[pos]->mName);
        mHeaders.RemoveObjectAt(pos);
        mCDSDirty = PR_TRUE;

        return NS_OK;
    }

    return NS_ERROR_FILE_NOT_FOUND;
}

/* void processQueue (in nsIRequestObserver aObserver,
 *                    in nsISupports aContext); */
NS_IMETHODIMP nsZipWriter::ProcessQueue(nsIRequestObserver *aObserver,
                                        nsISupports *aContext)
{
    if (!mStream)
        return NS_ERROR_NOT_INITIALIZED;
    if (mInQueue)
        return NS_ERROR_IN_PROGRESS;

    mProcessObserver = aObserver;
    mProcessContext = aContext;
    mInQueue = PR_TRUE;

    if (mProcessObserver)
        mProcessObserver->OnStartRequest(nsnull, mProcessContext);

    BeginProcessingNextItem();

    return NS_OK;
}

/* void close (); */
NS_IMETHODIMP nsZipWriter::Close()
{
    if (!mStream)
        return NS_ERROR_NOT_INITIALIZED;
    if (mInQueue)
        return NS_ERROR_IN_PROGRESS;

    if (mCDSDirty) {
        PRUint32 size = 0;
        for (PRInt32 i = 0; i < mHeaders.Count(); i++) {
            nsresult rv = mHeaders[i]->WriteCDSHeader(mStream);
            if (NS_FAILED(rv)) {
                Cleanup();
                return rv;
            }
            size += mHeaders[i]->GetCDSHeaderLength();
        }

        char buf[ZIP_EOCDR_HEADER_SIZE];
        PRUint32 pos = 0;
        WRITE32(buf, &pos, ZIP_EOCDR_HEADER_SIGNATURE);
        WRITE16(buf, &pos, 0);
        WRITE16(buf, &pos, 0);
        WRITE16(buf, &pos, mHeaders.Count());
        WRITE16(buf, &pos, mHeaders.Count());
        WRITE32(buf, &pos, size);
        WRITE32(buf, &pos, mCDSOffset);
        WRITE16(buf, &pos, mComment.Length());

        nsresult rv = ZW_WriteData(mStream, buf, pos);
        if (NS_FAILED(rv)) {
            Cleanup();
            return rv;
        }

        rv = ZW_WriteData(mStream, mComment.get(), mComment.Length());
        if (NS_FAILED(rv)) {
            Cleanup();
            return rv;
        }

        nsCOMPtr<nsISeekableStream> seekable = do_QueryInterface(mStream);
        rv = seekable->SetEOF();
        if (NS_FAILED(rv)) {
            Cleanup();
            return rv;
        }
    }

    nsresult rv = mStream->Close();
    mStream = nsnull;
    mHeaders.Clear();
    mEntryHash.Clear();
    mQueue.Clear();

    return rv;
}

// Our nsIRequestObserver monitors removal operations performed on the queue
/* void onStartRequest (in nsIRequest aRequest, in nsISupports aContext); */
NS_IMETHODIMP nsZipWriter::OnStartRequest(nsIRequest *aRequest,
                                          nsISupports *aContext)
{
    return NS_OK;
}

/* void onStopRequest (in nsIRequest aRequest, in nsISupports aContext,
 *                                             in nsresult aStatusCode); */
NS_IMETHODIMP nsZipWriter::OnStopRequest(nsIRequest *aRequest,
                                         nsISupports *aContext,
                                         nsresult aStatusCode)
{
    if (NS_FAILED(aStatusCode)) {
        FinishQueue(aStatusCode);
        Cleanup();
    }

    nsresult rv = mStream->Flush();
    if (NS_FAILED(rv)) {
        FinishQueue(rv);
        Cleanup();
        return rv;
    }
    rv = SeekCDS();
    if (NS_FAILED(rv)) {
        FinishQueue(rv);
        return rv;
    }

    BeginProcessingNextItem();

    return NS_OK;
}

nsresult nsZipWriter::InternalAddEntryDirectory(const nsACString & aZipEntry,
                                                PRTime aModTime)
{
    nsRefPtr<nsZipHeader> header = new nsZipHeader();
    NS_ENSURE_TRUE(header, NS_ERROR_OUT_OF_MEMORY);

    if (aZipEntry.Last() != '/') {
        nsCString dirPath;
        dirPath.Assign(aZipEntry + NS_LITERAL_CSTRING("/"));
        header->Init(dirPath, aModTime, ZIP_ATTRS_DIRECTORY, mCDSOffset);
    }
    else
        header->Init(aZipEntry, aModTime, ZIP_ATTRS_DIRECTORY, mCDSOffset);

    if (mEntryHash.Get(header->mName, nsnull))
        return NS_ERROR_FILE_ALREADY_EXISTS;

    nsresult rv = header->WriteFileHeader(mStream);
    if (NS_FAILED(rv)) {
        Cleanup();
        return rv;
    }

    mCDSDirty = PR_TRUE;
    mCDSOffset += header->GetFileHeaderLength();
    if (!mEntryHash.Put(header->mName, mHeaders.Count())) {
        Cleanup();
        return NS_ERROR_OUT_OF_MEMORY;
    }
    if (!mHeaders.AppendObject(header)) {
        Cleanup();
        return NS_ERROR_OUT_OF_MEMORY;
    }

    return NS_OK;
}

/*
 * Recovering from an error while adding a new entry is simply a case of
 * seeking back to the CDS. If we fail trying to do that though then cleanup
 * and bail out.
 */
nsresult nsZipWriter::SeekCDS()
{
    nsresult rv;
    nsCOMPtr<nsISeekableStream> seekable = do_QueryInterface(mStream, &rv);
    if (NS_FAILED(rv)) {
        Cleanup();
        return rv;
    }
    rv = seekable->Seek(nsISeekableStream::NS_SEEK_SET, mCDSOffset);
    if (NS_FAILED(rv))
        Cleanup();
    return rv;
}

/*
 * In a bad error condition this essentially closes down the component as best
 * it can.
 */
void nsZipWriter::Cleanup()
{
    mHeaders.Clear();
    mEntryHash.Clear();
    if (mStream)
        mStream->Close();
    mStream = nsnull;
    mFile = nsnull;
}

/*
 * Called when writing a file to the zip is complete.
 */
nsresult nsZipWriter::EntryCompleteCallback(nsZipHeader* aHeader,
                                            nsresult aStatus)
{
    if (NS_SUCCEEDED(aStatus)) {
        if (!mEntryHash.Put(aHeader->mName, mHeaders.Count())) {
            SeekCDS();
            return NS_ERROR_OUT_OF_MEMORY;
        }
        if (!mHeaders.AppendObject(aHeader)) {
            mEntryHash.Remove(aHeader->mName);
            SeekCDS();
            return NS_ERROR_OUT_OF_MEMORY;
        }
        mCDSDirty = PR_TRUE;
        mCDSOffset += aHeader->mCSize + aHeader->GetFileHeaderLength();

        if (mInQueue)
            BeginProcessingNextItem();

        return NS_OK;
    }

    nsresult rv = SeekCDS();
    if (mInQueue)
        FinishQueue(aStatus);
    return rv;
}

inline nsresult nsZipWriter::BeginProcessingAddition(nsZipQueueItem* aItem,
                                                     PRBool* complete)
{
    if (aItem->mFile) {
        PRBool exists;
        nsresult rv = aItem->mFile->Exists(&exists);
        NS_ENSURE_SUCCESS(rv, rv);

        if (!exists) return NS_ERROR_FILE_NOT_FOUND;

        PRBool isdir;
        rv = aItem->mFile->IsDirectory(&isdir);
        NS_ENSURE_SUCCESS(rv, rv);

        rv = aItem->mFile->GetLastModifiedTime(&aItem->mModTime);
        NS_ENSURE_SUCCESS(rv, rv);
        aItem->mModTime *= PR_USEC_PER_MSEC;

        if (!isdir) {
            // Set up for fall through to stream reader
            rv = NS_NewLocalFileInputStream(getter_AddRefs(aItem->mStream),
                                            aItem->mFile);
            NS_ENSURE_SUCCESS(rv, rv);
        }
        // If a dir then this will fall through to the plain dir addition
    }

    if (aItem->mStream) {
        nsRefPtr<nsZipHeader> header = new nsZipHeader();
        NS_ENSURE_TRUE(header, NS_ERROR_OUT_OF_MEMORY);

        header->Init(aItem->mZipEntry, aItem->mModTime, ZIP_ATTRS_FILE,
                     mCDSOffset);
        nsresult rv = header->WriteFileHeader(mStream);
        NS_ENSURE_SUCCESS(rv, rv);

        nsRefPtr<nsZipDataStream> stream = new nsZipDataStream();
        rv = stream->Init(this, mStream, header, aItem->mCompression);
        NS_ENSURE_SUCCESS(rv, rv);

        nsCOMPtr<nsIInputStreamPump> pump;
        rv = NS_NewInputStreamPump(getter_AddRefs(pump), aItem->mStream, -1,
                                   -1, 0, 0, PR_TRUE);
        NS_ENSURE_SUCCESS(rv, rv);

        rv = pump->AsyncRead(stream, nsnull);
        NS_ENSURE_SUCCESS(rv, rv);

        return NS_OK;
    }

    if (aItem->mChannel) {
        nsRefPtr<nsZipHeader> header = new nsZipHeader();
        NS_ENSURE_TRUE(header, NS_ERROR_OUT_OF_MEMORY);

        header->Init(aItem->mZipEntry, aItem->mModTime, ZIP_ATTRS_FILE,
                     mCDSOffset);

        nsRefPtr<nsZipDataStream> stream = new nsZipDataStream();
        NS_ENSURE_TRUE(stream, NS_ERROR_OUT_OF_MEMORY);
        nsresult rv = stream->Init(this, mStream, header, aItem->mCompression);
        NS_ENSURE_SUCCESS(rv, rv);
        rv = aItem->mChannel->AsyncOpen(stream, nsnull);
        NS_ENSURE_SUCCESS(rv, rv);

        return NS_OK;
    }

    // Must be plain directory addition
    *complete = PR_TRUE;
    return InternalAddEntryDirectory(aItem->mZipEntry, aItem->mModTime);
}

inline nsresult nsZipWriter::BeginProcessingRemoval(PRInt32 aPos)
{
    // Open the zip file for reading
    nsCOMPtr<nsIInputStream> inputStream;
    nsresult rv = NS_NewLocalFileInputStream(getter_AddRefs(inputStream),
                                             mFile);
    NS_ENSURE_SUCCESS(rv, rv);
    nsCOMPtr<nsIInputStreamPump> pump;
    rv = NS_NewInputStreamPump(getter_AddRefs(pump), inputStream, -1, -1, 0,
                               0, PR_TRUE);
    if (NS_FAILED(rv)) {
        inputStream->Close();
        return rv;
    }
    nsCOMPtr<nsIStreamListener> listener;
    rv = NS_NewSimpleStreamListener(getter_AddRefs(listener), mStream, this);
    if (NS_FAILED(rv)) {
        inputStream->Close();
        return rv;
    }

    nsCOMPtr<nsISeekableStream> seekable = do_QueryInterface(mStream);
    rv = seekable->Seek(nsISeekableStream::NS_SEEK_SET,
                        mHeaders[aPos]->mOffset);
    if (NS_FAILED(rv)) {
        inputStream->Close();
        return rv;
    }

    PRUint32 shift = (mHeaders[aPos + 1]->mOffset -
                      mHeaders[aPos]->mOffset);
    mCDSOffset -= shift;
    PRInt32 pos2 = aPos + 1;
    while (pos2 < mHeaders.Count()) {
        mEntryHash.Put(mHeaders[pos2]->mName, pos2 - 1);
        mHeaders[pos2]->mOffset -= shift;
        pos2++;
    }

    mEntryHash.Remove(mHeaders[aPos]->mName);
    mHeaders.RemoveObjectAt(aPos);
    mCDSDirty = PR_TRUE;

    rv = pump->AsyncRead(listener, nsnull);
    if (NS_FAILED(rv)) {
        inputStream->Close();
        Cleanup();
        return rv;
    }
    return NS_OK;
}

/*
 * Starts processing on the next item in the queue.
 */
void nsZipWriter::BeginProcessingNextItem()
{
    while (!mQueue.IsEmpty()) {

        nsZipQueueItem next = mQueue[0];
        mQueue.RemoveElementAt(0);

        if (next.mOperation == OPERATION_REMOVE) {
            PRInt32 pos = -1;
            if (mEntryHash.Get(next.mZipEntry, &pos)) {
                if (pos < mHeaders.Count() - 1) {
                    nsresult rv = BeginProcessingRemoval(pos);
                    if (NS_FAILED(rv)) FinishQueue(rv);
                    return;
                }

                mCDSOffset = mHeaders[pos]->mOffset;
                nsresult rv = SeekCDS();
                if (NS_FAILED(rv)) {
                    FinishQueue(rv);
                    return;
                }
                mEntryHash.Remove(mHeaders[pos]->mName);
                mHeaders.RemoveObjectAt(pos);
            }
            else {
                FinishQueue(NS_ERROR_FILE_NOT_FOUND);
                return;
            }
        }
        else if (next.mOperation == OPERATION_ADD) {
            if (mEntryHash.Get(next.mZipEntry, nsnull)) {
                FinishQueue(NS_ERROR_FILE_ALREADY_EXISTS);
                return;
            }

            PRBool complete = PR_FALSE;
            nsresult rv = BeginProcessingAddition(&next, &complete);
            if (NS_FAILED(rv)) {
                SeekCDS();
                FinishQueue(rv);
                return;
            }
            if (!complete)
                return;
        }
    }

    FinishQueue(NS_OK);
}

/*
 * Ends processing with the given status.
 */
void nsZipWriter::FinishQueue(nsresult aStatus)
{
    nsCOMPtr<nsIRequestObserver> observer = mProcessObserver;
    nsCOMPtr<nsISupports> context = mProcessContext;
    // Clean up everything first in case the observer decides to queue more
    // things
    mProcessObserver = nsnull;
    mProcessContext = nsnull;
    mInQueue = PR_FALSE;

    if (observer)
        observer->OnStopRequest(nsnull, context, aStatus);
}
