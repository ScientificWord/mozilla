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
#ifndef nsIFileStream_h___
#define nsIFileStream_h___

#include "xpcomobsolete.h"
#include "nsIInputStream.h"
#include "nsIOutputStream.h"
#include "nsISeekableStream.h"
#include "prio.h"

class nsFileSpec;

/* a6cf90e8-15b3-11d2-932e-00805f8add32 */
#define NS_IOPENFILE_IID \
{ 0xa6cf90e8, 0x15b3, 0x11d2, \
    {0x93, 0x2e, 0x00, 0x80, 0x5f, 0x8a, 0xdd, 0x32} }

//========================================================================================
class nsIOpenFile
// Represents a file, and supports Open.
//========================================================================================
: public nsISupports
{
public:
    NS_DECLARE_STATIC_IID_ACCESSOR(NS_IOPENFILE_IID)

	NS_IMETHOD                         Open(
                                           const nsFileSpec& inFile,
                                           int nsprMode,
                                           PRIntn accessMode) = 0;
                                           // Note: Open() is only needed after
                                           // an explicit Close().  All file streams
                                           // are automatically opened on construction.
    NS_IMETHOD                         GetIsOpen(PRBool* outOpen) = 0;

}; // class nsIOpenFile

NS_DEFINE_STATIC_IID_ACCESSOR(nsIOpenFile, NS_IOPENFILE_IID)

/* a6cf90e8-15b3-11d2-932e-00805f8add32 */
#define NS_IRANDOMACCESS_IID \
{  0xa6cf90eb, 0x15b3, 0x11d2, \
    {0x93, 0x2e, 0x00, 0x80, 0x5f, 0x8a, 0xdd, 0x32} }

//========================================================================================
class nsIRandomAccessStore
// Supports Seek, Tell etc.
//========================================================================================
: public nsISeekableStream
{
public:
    NS_DECLARE_STATIC_IID_ACCESSOR(NS_IRANDOMACCESS_IID)

/* "PROTECTED" */
    NS_IMETHOD                         GetAtEOF(PRBool* outAtEOF) = 0;
    NS_IMETHOD                         SetAtEOF(PRBool inAtEOF) = 0;
}; // class nsIRandomAccessStore

NS_DEFINE_STATIC_IID_ACCESSOR(nsIRandomAccessStore, NS_IRANDOMACCESS_IID)

#ifndef NO_XPCOM_FILE_STREAMS   // hack to work around duplicate class definitions in here
                                // and mozilla/netwerks/base/nsIFileStreams.idl

/* a6cf90e6-15b3-11d2-932e-00805f8add32 */
#define NS_IFILESPECINPUTSTREAM_IID \
{ 0xa6cf90e6, 0x15b3, 0x11d2, \
    {0x93, 0x2e, 0x00, 0x80, 0x5f, 0x8a, 0xdd, 0x32} }
    
//========================================================================================
class nsIFileSpecInputStream
// These are additional file-specific methods that files have, above what
// nsIInputStream supports.  The current implementation supports both
// interfaces.
//========================================================================================
: public nsIInputStream
{
public:
    NS_DECLARE_STATIC_IID_ACCESSOR(NS_IFILESPECINPUTSTREAM_IID)
}; // class nsIFileSpecInputStream

NS_DEFINE_STATIC_IID_ACCESSOR(nsIFileSpecInputStream,
                              NS_IFILESPECINPUTSTREAM_IID)

/* a6cf90e7-15b3-11d2-932e-00805f8add32 */
#define NS_IFILESPECOUTPUTSTREAM_IID \
{ 0xa6cf90e7, 0x15b3, 0x11d2, \
    {0x93, 0x2e, 0x00, 0x80, 0x5f, 0x8a, 0xdd, 0x32} }

//========================================================================================
class nsIFileSpecOutputStream
// These are additional file-specific methods that files have, above what
// nsIOutputStream supports.  The current implementation supports both
// interfaces.
//========================================================================================
: public nsIOutputStream
{
public:
    NS_DECLARE_STATIC_IID_ACCESSOR(NS_IFILESPECOUTPUTSTREAM_IID)
}; // class nsIFileSpecOutputStream

NS_DEFINE_STATIC_IID_ACCESSOR(nsIFileSpecOutputStream,
                              NS_IFILESPECOUTPUTSTREAM_IID)

#endif // NO_XPCOM_FILE_STREAMS

//----------------------------------------------------------------------------------------
nsresult NS_NewTypicalInputFileStream(
    nsISupports** aStreamResult,
    const nsFileSpec& inFile
    /*Default nsprMode == PR_RDONLY*/
    /*Default accessmode = 0700 (octal)*/);
// Factory method to get an nsInputStream from a file, using most common options

//----------------------------------------------------------------------------------------
nsresult NS_NewTypicalOutputFileStream(
    nsISupports** aStreamResult, // will implement all the above interfaces
    const nsFileSpec& inFile
    /*default nsprMode= (PR_WRONLY | PR_CREATE_FILE | PR_TRUNCATE)*/
    /*Default accessMode= 0700 (octal)*/);
    // Factory method to get an nsOutputStream to a file - most common case.

//----------------------------------------------------------------------------------------
extern "C" NS_COM_OBSOLETE nsresult NS_NewIOFileStream(
    nsISupports** aStreamResult, // will implement all the above interfaces
    const nsFileSpec& inFile,
    PRInt32 nsprMode,
    PRInt32 accessMode);
    // Factory method to get an object that implements both nsIInputStream
    // and nsIOutputStream, associated with a single file.

#endif /* nsIFileSpecStream_h___ */
