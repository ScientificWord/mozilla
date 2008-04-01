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
 * The Original Code is mozila.org code.
 *
 * The Initial Developer of the Original Code is
 * Mozilla Corporation
 * Portions created by the Initial Developer are Copyright (C) 2007
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Dave Camp <dcamp@mozilla.com>
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

#include "nsDOMFile.h"

#include "nsCExternalHandlerService.h"
#include "nsContentCID.h"
#include "nsContentUtils.h"
#include "nsDOMClassInfo.h"
#include "nsDOMError.h"
#include "nsICharsetAlias.h"
#include "nsICharsetDetector.h"
#include "nsICharsetConverterManager.h"
#include "nsIConverterInputStream.h"
#include "nsIDocument.h"
#include "nsIDOMDocument.h"
#include "nsIFile.h"
#include "nsIFileStreams.h"
#include "nsIInputStream.h"
#include "nsIMIMEService.h"
#include "nsIPlatformCharset.h"
#include "nsISeekableStream.h"
#include "nsIUnicharInputStream.h"
#include "nsIUnicodeDecoder.h"
#include "nsNetCID.h"
#include "nsNetUtil.h"

#include "plbase64.h"
#include "prmem.h"

// nsDOMFile implementation

NS_INTERFACE_MAP_BEGIN(nsDOMFile)
  NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, nsIDOMFile)
  NS_INTERFACE_MAP_ENTRY(nsIDOMFile)
  NS_INTERFACE_MAP_ENTRY(nsICharsetDetectionObserver)
  NS_DOM_INTERFACE_MAP_ENTRY_CLASSINFO(File)
NS_INTERFACE_MAP_END

NS_IMPL_ADDREF(nsDOMFile)
NS_IMPL_RELEASE(nsDOMFile)

static nsresult
DOMFileResult(nsresult rv)
{
  if (rv == NS_ERROR_FILE_NOT_FOUND) {
    return NS_ERROR_DOM_FILE_NOT_FOUND_ERR;
  }

  if (NS_ERROR_GET_MODULE(rv) == NS_ERROR_MODULE_FILES) {
    return NS_ERROR_DOM_FILE_NOT_READABLE_ERR;
  }

  return rv;
}

NS_IMETHODIMP
nsDOMFile::GetFileName(nsAString &aFileName)
{
  return mFile->GetLeafName(aFileName);
}

NS_IMETHODIMP
nsDOMFile::GetFileSize(PRUint64 *aFileSize)
{
  PRInt64 fileSize;
  nsresult rv = mFile->GetFileSize(&fileSize);
  NS_ENSURE_SUCCESS(rv, rv);

  if (fileSize < 0) {
    return NS_ERROR_FAILURE;
  }

  *aFileSize = fileSize;

  return NS_OK;
}

NS_IMETHODIMP
nsDOMFile::GetAsText(const nsAString &aCharset, nsAString &aResult)
{
  aResult.Truncate();

  nsCOMPtr<nsIInputStream> stream;
  nsresult rv = NS_NewLocalFileInputStream
                  (getter_AddRefs(stream),
                   mFile, -1, -1, 0);
  NS_ENSURE_SUCCESS(rv, DOMFileResult(rv));

  nsCAutoString charsetGuess;
  if (!aCharset.IsEmpty()) {
    CopyUTF16toUTF8(aCharset, charsetGuess);
  } else {
    rv = GuessCharset(stream, charsetGuess);
    NS_ENSURE_SUCCESS(rv, DOMFileResult(rv));

    nsCOMPtr<nsISeekableStream> seekable = do_QueryInterface(stream);
    if (!seekable) return NS_ERROR_FAILURE;
    rv = seekable->Seek(nsISeekableStream::NS_SEEK_SET, 0);
    NS_ENSURE_SUCCESS(rv, DOMFileResult(rv));
  }

  nsCAutoString charset;
  nsCOMPtr<nsICharsetAlias> alias =
    do_GetService(NS_CHARSETALIAS_CONTRACTID, &rv);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = alias->GetPreferred(charsetGuess, charset);
  NS_ENSURE_SUCCESS(rv, rv);

  return ConvertStream(stream, charset.get(), aResult);
}

NS_IMETHODIMP
nsDOMFile::GetAsDataURL(nsAString &aResult)
{
  aResult.AssignLiteral("data:");

  nsresult rv;
  nsCOMPtr<nsIMIMEService> mimeService =
    do_GetService(NS_MIMESERVICE_CONTRACTID, &rv);
  NS_ENSURE_SUCCESS(rv, rv);

  nsCAutoString contentType;
  rv = mimeService->GetTypeFromFile(mFile, contentType);
  if (NS_SUCCEEDED(rv)) {
    AppendUTF8toUTF16(contentType, aResult);
  } else {
    aResult.AppendLiteral("application/octet-stream");
  }
  aResult.AppendLiteral(";base64,");

  nsCOMPtr<nsIInputStream> stream;
  rv = NS_NewLocalFileInputStream(getter_AddRefs(stream),
                                  mFile, -1, -1,
                                  nsIFileInputStream::CLOSE_ON_EOF);
  NS_ENSURE_SUCCESS(rv, DOMFileResult(rv));

  char readBuf[4096];
  PRUint32 leftOver = 0;
  PRUint32 numRead;
  do {
    rv = stream->Read(readBuf + leftOver, sizeof(readBuf) - leftOver, &numRead);
    NS_ENSURE_SUCCESS(rv, DOMFileResult(rv));

    PRUint32 numEncode = numRead + leftOver;
    leftOver = 0;

    if (numEncode == 0) break;

    // unless this is the end of the file, encode in multiples of 3
    if (numRead > 0) {
      leftOver = numEncode % 3;
      numEncode -= leftOver;
    }

    // out buffer should be at least 4/3rds the read buf, plus a terminator
    char *base64 = PL_Base64Encode(readBuf, numEncode, nsnull);
    AppendASCIItoUTF16(base64, aResult);
    PR_Free(base64);

    if (leftOver) {
      memmove(readBuf, readBuf + numEncode, leftOver);
    }
  } while (numRead > 0);

  return NS_OK;
}

NS_IMETHODIMP
nsDOMFile::GetAsBinary(nsAString &aResult)
{
  aResult.Truncate();

  nsCOMPtr<nsIInputStream> stream;
  nsresult rv = NS_NewLocalFileInputStream
                  (getter_AddRefs(stream),
                   mFile, -1, -1,
                   nsIFileInputStream::CLOSE_ON_EOF);
  NS_ENSURE_SUCCESS(rv, DOMFileResult(rv));

  PRUint32 numRead;
  do {
    char readBuf[4096];
    rv = stream->Read(readBuf, sizeof(readBuf), &numRead);
    NS_ENSURE_SUCCESS(rv, DOMFileResult(rv));
    AppendASCIItoUTF16(Substring(readBuf, readBuf + numRead), aResult);
  } while (numRead > 0);

  return NS_OK;
}

nsresult
nsDOMFile::GuessCharset(nsIInputStream *aStream,
                        nsACString &aCharset)
{

  if (!mCharset.IsEmpty()) {
    aCharset = mCharset;
    return NS_OK;
  }

  // First try the universal charset detector
  nsCOMPtr<nsICharsetDetector> detector
    = do_CreateInstance(NS_CHARSET_DETECTOR_CONTRACTID_BASE
                        "universal_charset_detector");
  if (!detector) {
    // No universal charset detector, try the default charset detector
    const nsAdoptingString& detectorName =
      nsContentUtils::GetLocalizedStringPref("intl.charset.detector");
    if (!detectorName.IsEmpty()) {
      nsCAutoString detectorContractID;
      detectorContractID.AssignLiteral(NS_CHARSET_DETECTOR_CONTRACTID_BASE);
      AppendUTF16toUTF8(detectorName, detectorContractID);
      detector = do_CreateInstance(detectorContractID.get());
    }
  }

  nsresult rv;
  if (detector) {
    detector->Init(this);

    PRBool done;
    PRUint32 numRead;
    do {
      char readBuf[4096];
      rv = aStream->Read(readBuf, sizeof(readBuf), &numRead);
      NS_ENSURE_SUCCESS(rv, rv);

      rv = detector->DoIt(readBuf, numRead, &done);
      NS_ENSURE_SUCCESS(rv, rv);
    } while (!done && numRead > 0);

    rv = detector->Done();
    NS_ENSURE_SUCCESS(rv, rv);
  } else {
    // no charset detector available, check the BOM
    unsigned char sniffBuf[4];
    PRUint32 numRead;
    rv = aStream->Read(reinterpret_cast<char*>(sniffBuf),
                       sizeof(sniffBuf), &numRead);
    NS_ENSURE_SUCCESS(rv, rv);

    if (numRead >= 4 &&
        sniffBuf[0] == 0x00 &&
        sniffBuf[1] == 0x00 &&
        sniffBuf[2] == 0xfe &&
        sniffBuf[3] == 0xff) {
      mCharset = "UTF-32BE";
    } else if (numRead >= 4 &&
               sniffBuf[0] == 0xff &&
               sniffBuf[1] == 0xfe &&
               sniffBuf[2] == 0x00 &&
               sniffBuf[3] == 0x00) {
      mCharset = "UTF-32LE";
    } else if (numRead >= 2 &&
               sniffBuf[0] == 0xfe &&
               sniffBuf[1] == 0xff) {
      mCharset = "UTF-16BE";
    } else if (numRead >= 2 &&
               sniffBuf[0] == 0xff &&
               sniffBuf[1] == 0xfe) {
      mCharset = "UTF-16LE";
    } else if (numRead >= 3 &&
               sniffBuf[0] == 0xef &&
               sniffBuf[1] == 0xbb &&
               sniffBuf[2] == 0xbf) {
      mCharset = "UTF-8";
    }
  }

  if (mCharset.IsEmpty()) {
    // no charset detected, default to the system charset
    nsCOMPtr<nsIPlatformCharset> platformCharset =
      do_GetService(NS_PLATFORMCHARSET_CONTRACTID, &rv);
    if (NS_SUCCEEDED(rv)) {
      rv = platformCharset->GetCharset(kPlatformCharsetSel_PlainTextInFile,
                                       mCharset);
    }
  }

  if (mCharset.IsEmpty()) {
    // no sniffed or default charset, try UTF-8
    mCharset.AssignLiteral("UTF-8");
  }

  aCharset = mCharset;

  return NS_OK;
}

NS_IMETHODIMP
nsDOMFile::Notify(const char* aCharset, nsDetectionConfident aConf)
{
  mCharset.Assign(aCharset);

  return NS_OK;
}

nsresult
nsDOMFile::ConvertStream(nsIInputStream *aStream,
                         const char *aCharset,
                         nsAString &aResult)
{
  aResult.Truncate();

  nsCOMPtr<nsIConverterInputStream> converterStream =
    do_CreateInstance("@mozilla.org/intl/converter-input-stream;1");
  if (!converterStream) return NS_ERROR_FAILURE;

  nsresult rv = converterStream->Init
                  (aStream, aCharset,
                   8192,
                   nsIConverterInputStream::DEFAULT_REPLACEMENT_CHARACTER);
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<nsIUnicharInputStream> unicharStream =
    do_QueryInterface(converterStream);
  if (!unicharStream) return NS_ERROR_FAILURE;

  PRUint32 numChars;
  nsString result;
  rv = unicharStream->ReadString(8192, result, &numChars);
  while (NS_SUCCEEDED(rv) && numChars > 0) {
    aResult.Append(result);
    rv = unicharStream->ReadString(8192, result, &numChars);
  }

  return rv;
}

// nsDOMFileList implementation

NS_INTERFACE_MAP_BEGIN(nsDOMFileList)
  NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, nsIDOMFileList)
  NS_INTERFACE_MAP_ENTRY(nsIDOMFileList)
  NS_DOM_INTERFACE_MAP_ENTRY_CLASSINFO(FileList)
NS_INTERFACE_MAP_END

NS_IMPL_ADDREF(nsDOMFileList)
NS_IMPL_RELEASE(nsDOMFileList)

NS_IMETHODIMP
nsDOMFileList::GetLength(PRUint32* aLength)
{
  *aLength = mFiles.Count();

  return NS_OK;
}

NS_IMETHODIMP
nsDOMFileList::Item(PRUint32 aIndex, nsIDOMFile **aFile)
{
  NS_IF_ADDREF(*aFile = mFiles.SafeObjectAt(aIndex));

  return NS_OK;
}
