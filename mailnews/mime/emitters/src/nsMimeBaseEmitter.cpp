/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 *   Pierre Phaneuf <pp@ludusdesign.com>
 *   Henrik Gemal <mozilla@gemal.dk>
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

#include "nsCOMPtr.h"
#include "nsXPIDLString.h"
#include "nsReadableUtils.h"
#include <stdio.h>
#include "nsMimeBaseEmitter.h"
#include "nsMailHeaders.h"
#include "nscore.h"
#include "nsIPrefService.h"
#include "nsIPrefBranch.h"
#include "nsIServiceManager.h"
#include "nsEscape.h"
#include "prmem.h"
#include "nsEmitterUtils.h"
#include "nsMimeStringResources.h"
#include "msgCore.h"
#include "nsIMsgHeaderParser.h"
#include "nsIComponentManager.h"
#include "nsEmitterUtils.h"
#include "nsIMimeStreamConverter.h"
#include "nsIMimeConverter.h"
#include "nsMsgMimeCID.h"
#include "prlog.h"
#include "prprf.h"
#include "nsIMimeHeaders.h"
#include "nsIMsgWindow.h"
#include "nsIMsgMailNewsUrl.h"

static PRLogModuleInfo * gMimeEmitterLogModule = nsnull;

#define   MIME_HEADER_URL      "chrome://messenger/locale/mimeheader.properties"
#define   MIME_URL             "chrome://messenger/locale/mime.properties"

NS_IMPL_THREADSAFE_ADDREF(nsMimeBaseEmitter)
NS_IMPL_THREADSAFE_RELEASE(nsMimeBaseEmitter)

NS_INTERFACE_MAP_BEGIN(nsMimeBaseEmitter)
   NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, nsIMimeEmitter)
   NS_INTERFACE_MAP_ENTRY(nsIMimeEmitter)
   NS_INTERFACE_MAP_ENTRY(nsIInterfaceRequestor)
NS_INTERFACE_MAP_END

nsMimeBaseEmitter::nsMimeBaseEmitter()
{
  // Initialize data output vars...
  mFirstHeaders = PR_TRUE;

  mBufferMgr = nsnull;
  mTotalWritten = 0;
  mTotalRead = 0;
  mInputStream = nsnull;
  mOutStream = nsnull;
  mOutListener = nsnull;
  mChannel = nsnull;

  // Display output control vars...
  mDocHeader = PR_FALSE;
  m_stringBundle = nsnull;
  mURL = nsnull;
  mHeaderDisplayType = nsMimeHeaderDisplayTypes::NormalHeaders;

  // Setup array for attachments
  mAttachCount = 0;
  mAttachArray = new nsVoidArray();
  mCurrentAttachment = nsnull;

  // Header cache...
  mHeaderArray = new nsVoidArray();

  // Embedded Header Cache...
  mEmbeddedHeaderArray = nsnull;

  // HTML Header Data...
//  mHTMLHeaders = "";
//  mCharset = "";

  // Init the body...
  mBodyStarted = PR_FALSE;
//  mBody = "";

  // This is needed for conversion of I18N Strings...
  mUnicodeConverter = do_GetService(NS_MIME_CONVERTER_CONTRACTID);

  if (!gMimeEmitterLogModule)
    gMimeEmitterLogModule = PR_NewLogModule("MIME");

  // Do prefs last since we can live without this if it fails...
  nsCOMPtr<nsIPrefBranch> pPrefBranch(do_GetService(NS_PREFSERVICE_CONTRACTID));
  if (pPrefBranch)
    pPrefBranch->GetIntPref("mail.show_headers", &mHeaderDisplayType);
}

nsMimeBaseEmitter::~nsMimeBaseEmitter(void)
{
  PRInt32 i;

  // Delete the buffer manager...
  if (mBufferMgr)
  {
    delete mBufferMgr;
    mBufferMgr = nsnull;
  }

  // Clean up the attachment array structures...
  if (mAttachArray)
  {
    for (i=0; i<mAttachArray->Count(); i++)
    {
      attachmentInfoType *attachInfo = (attachmentInfoType *)mAttachArray->ElementAt(i);
      if (!attachInfo)
        continue;
    
      PR_FREEIF(attachInfo->contentType);
      PR_FREEIF(attachInfo->displayName);
      PR_FREEIF(attachInfo->urlSpec);
      PR_FREEIF(attachInfo);
    }
    delete mAttachArray;
  }

  // Cleanup allocated header arrays...
  CleanupHeaderArray(mHeaderArray);
  mHeaderArray = nsnull;

  CleanupHeaderArray(mEmbeddedHeaderArray);
  mEmbeddedHeaderArray = nsnull;
}

NS_IMETHODIMP nsMimeBaseEmitter::GetInterface(const nsIID & aIID, void * *aInstancePtr)
{
  NS_ENSURE_ARG_POINTER(aInstancePtr);
  return QueryInterface(aIID, aInstancePtr);
}

void
nsMimeBaseEmitter::CleanupHeaderArray(nsVoidArray *aArray)
{
  if (!aArray)
    return;

  for (PRInt32 i=0; i<aArray->Count(); i++)
  {
    headerInfoType *headerInfo = (headerInfoType *)aArray->ElementAt(i);
    if (!headerInfo)
      continue;
    
    PR_FREEIF(headerInfo->name);
    PR_FREEIF(headerInfo->value);
    PR_FREEIF(headerInfo);
  }

  delete aArray;
}

static PRInt32 MapHeaderNameToID(const char *header)
{
  // emitter passes UPPERCASE for header names
  if (!strcmp(header, "DATE"))
    return MIME_MHTML_DATE;
  else if (!strcmp(header, "FROM"))
    return MIME_MHTML_FROM;
  else if (!strcmp(header, "SUBJECT"))
    return MIME_MHTML_SUBJECT;
  else if (!strcmp(header, "TO"))
    return MIME_MHTML_TO;
  else if (!strcmp(header, "SENDER"))
    return MIME_MHTML_SENDER;
  else if (!strcmp(header, "RESENT-TO"))
    return MIME_MHTML_RESENT_TO;
  else if (!strcmp(header, "RESENT-SENDER"))
    return MIME_MHTML_RESENT_SENDER;
  else if (!strcmp(header, "RESENT-FROM"))
    return MIME_MHTML_RESENT_FROM;
  else if (!strcmp(header, "RESENT-CC"))
    return MIME_MHTML_RESENT_CC;
  else if (!strcmp(header, "REPLY-TO"))
    return MIME_MHTML_REPLY_TO;
  else if (!strcmp(header, "REFERENCES"))
    return MIME_MHTML_REFERENCES;
  else if (!strcmp(header, "NEWSGROUPS"))
    return MIME_MHTML_NEWSGROUPS;
  else if (!strcmp(header, "MESSAGE-ID"))
    return MIME_MHTML_MESSAGE_ID;
  else if (!strcmp(header, "FOLLOWUP-TO"))
    return MIME_MHTML_FOLLOWUP_TO;
  else if (!strcmp(header, "CC"))
    return MIME_MHTML_CC;
  else if (!strcmp(header, "ORGANIZATION"))
    return MIME_MHTML_ORGANIZATION;
  else if (!strcmp(header, "BCC"))
    return MIME_MHTML_BCC;

  return 0;
}

char *
nsMimeBaseEmitter::MimeGetStringByName(const char *aHeaderName)
{
	nsresult res = NS_OK;

	if (!m_headerStringBundle)
	{
		static const char propertyURL[] = MIME_HEADER_URL;

		nsCOMPtr<nsIStringBundleService> sBundleService = 
		         do_GetService(NS_STRINGBUNDLE_CONTRACTID, &res); 
		if (NS_SUCCEEDED(res) && (nsnull != sBundleService)) 
		{
			res = sBundleService->CreateBundle(propertyURL, getter_AddRefs(m_headerStringBundle));
		}
	}

	if (m_headerStringBundle)
	{
    nsXPIDLString val;

    res = m_headerStringBundle->GetStringFromName(NS_ConvertASCIItoUTF16(aHeaderName).get(), 
                                                  getter_Copies(val));

    if (NS_FAILED(res)) 
      return nsnull;
    
    // Here we need to return a new copy of the string
    // This returns a UTF-8 string so the caller needs to perform a conversion 
    // if this is used as UCS-2 (e.g. cannot do nsString(utfStr);
    //
    return ToNewUTF8String(val);
	}
	else
	{
    return nsnull;
	}
}

char *
nsMimeBaseEmitter::MimeGetStringByID(PRInt32 aID)
{
  nsresult res = NS_OK;

  if (!m_stringBundle)
  {
    static const char propertyURL[] = MIME_URL;

    nsCOMPtr<nsIStringBundleService> sBundleService = 
                            do_GetService(NS_STRINGBUNDLE_CONTRACTID, &res); 
    if (NS_SUCCEEDED(res)) 
      res = sBundleService->CreateBundle(propertyURL, getter_AddRefs(m_stringBundle));
  }

  if (m_stringBundle)
  {
    nsXPIDLString val;

    res = m_stringBundle->GetStringFromID(aID, getter_Copies(val));

    if (NS_FAILED(res)) 
      return nsnull;

    return ToNewUTF8String(val);
  }
  else
    return nsnull;
}

// 
// This will search a string bundle (eventually) to find a descriptive header 
// name to match what was found in the mail message. aHeaderName is passed in
// in all caps and a dropback default name is provided. The caller needs to free
// the memory returned by this function.
//
char *
nsMimeBaseEmitter::LocalizeHeaderName(const char *aHeaderName, const char *aDefaultName)
{
  char *retVal = nsnull;

  // prefer to use translated strings if not for quoting
  if (mFormat != nsMimeOutput::nsMimeMessageQuoting &&
      mFormat != nsMimeOutput::nsMimeMessageBodyQuoting)
  {
    // map name to id and get the translated string
    PRInt32 id = MapHeaderNameToID(aHeaderName);
    if (id > 0)
      retVal = MimeGetStringByID(id);
  }
  
  // get the string from the other bundle (usually not translated)
  if (!retVal)
    retVal = MimeGetStringByName(aHeaderName);

  if (retVal)
    return retVal;
  else
    return nsCRT::strdup(aDefaultName);
}

///////////////////////////////////////////////////////////////////////////
// nsMimeBaseEmitter Interface
/////////////////////////////////////////////////////////////////////////// 
NS_IMETHODIMP
nsMimeBaseEmitter::SetPipe(nsIInputStream * aInputStream, nsIOutputStream *outStream)
{
  mInputStream = aInputStream;
  mOutStream = outStream;
  return NS_OK;
}

// Note - these is setup only...you should not write
// anything to the stream since these may be image data
// output streams, etc...
NS_IMETHODIMP       
nsMimeBaseEmitter::Initialize(nsIURI *url, nsIChannel * aChannel, PRInt32 aFormat)
{
  // set the url
  mURL = url;
  mChannel = aChannel;

  // Create rebuffering object
  if (mBufferMgr)
  {
    delete mBufferMgr;
  }
  mBufferMgr = new MimeRebuffer();

  // Counters for output stream
  mTotalWritten = 0;
  mTotalRead = 0;
  mFormat = aFormat;

  return NS_OK;
}

NS_IMETHODIMP
nsMimeBaseEmitter::SetOutputListener(nsIStreamListener *listener)
{
  mOutListener = listener;
  return NS_OK;
}

NS_IMETHODIMP
nsMimeBaseEmitter::GetOutputListener(nsIStreamListener **listener)
{
  if (listener)
  {
    *listener = mOutListener;
    NS_IF_ADDREF(*listener);
  }
  return NS_OK;
}


// Attachment handling routines
nsresult
nsMimeBaseEmitter::StartAttachment(const char *name, const char *contentType, const char *url,
                                   PRBool aIsExternalAttachment)
{
  // Ok, now we will setup the attachment info 
  mCurrentAttachment = (attachmentInfoType *) PR_NEWZAP(attachmentInfoType);
  if ( (mCurrentAttachment) && mAttachArray)
  {
    ++mAttachCount;

    mCurrentAttachment->displayName = nsCRT::strdup(name);
    mCurrentAttachment->urlSpec = nsCRT::strdup(url);
    mCurrentAttachment->contentType = nsCRT::strdup(contentType);
    mCurrentAttachment->isExternalAttachment = aIsExternalAttachment;
  }

  return NS_OK;
}

nsresult
nsMimeBaseEmitter::EndAttachment()
{
  // Ok, add the attachment info to the attachment array...
  if ( (mCurrentAttachment) && (mAttachArray) )
  {
    mAttachArray->AppendElement(mCurrentAttachment);
    mCurrentAttachment = nsnull;
  }

  return NS_OK;
}

nsresult
nsMimeBaseEmitter::EndAllAttachments()
{
	return NS_OK;
}

NS_IMETHODIMP
nsMimeBaseEmitter::AddAttachmentField(const char *field, const char *value)
{
  return NS_OK;
}

NS_IMETHODIMP
nsMimeBaseEmitter::UtilityWrite(const char *buf)
{
  PRInt32     tmpLen = strlen(buf);
  PRUint32    written;

  Write(buf, tmpLen, &written);

  return NS_OK;
}

NS_IMETHODIMP
nsMimeBaseEmitter::UtilityWriteCRLF(const char *buf)
{
  PRInt32     tmpLen = strlen(buf);
  PRUint32    written;

  Write(buf, tmpLen, &written);
  Write(CRLF, 2, &written);

  return NS_OK;
}

NS_IMETHODIMP
nsMimeBaseEmitter::Write(const char *buf, PRUint32 size, PRUint32 *amountWritten)
{
  unsigned int        written = 0;
  nsresult rv = NS_OK;
  PRUint32            needToWrite;

#ifdef DEBUG_BenB
  // If you want to see libmime output...
  printf("%s", buf);
#endif

  PR_LOG(gMimeEmitterLogModule, PR_LOG_ALWAYS, (buf));
  //
  // Make sure that the buffer we are "pushing" into has enough room
  // for the write operation. If not, we have to buffer, return, and get
  // it on the next time through
  //
  *amountWritten = 0;

  needToWrite = mBufferMgr->GetSize();
  // First, handle any old buffer data...
  if (needToWrite > 0)
  {
    rv = WriteHelper(mBufferMgr->GetBuffer(), needToWrite, &written);

    mTotalWritten += written;
    mBufferMgr->ReduceBuffer(written);
    *amountWritten = written;

    // if we couldn't write all the old data, buffer the new data
    // and return
    if (mBufferMgr->GetSize() > 0)
    {
      mBufferMgr->IncreaseBuffer(buf, size);
      return rv;
    }
  }


  // if we get here, we are dealing with new data...try to write
  // and then do the right thing...
  rv = WriteHelper(buf, size, &written);
  *amountWritten = written;
  mTotalWritten += written;

  if (written < size)
    mBufferMgr->IncreaseBuffer(buf+written, (size-written));

  return rv;
}

nsresult
nsMimeBaseEmitter::WriteHelper(const char *buf, PRUint32 count, PRUint32 *countWritten)
{
  nsresult rv;

  rv = mOutStream->Write(buf, count, countWritten);
  if (rv == NS_BASE_STREAM_WOULD_BLOCK) {
    // pipe is full, push contents of pipe to listener...
    PRUint32 avail;
    rv = mInputStream->Available(&avail);
    if (NS_SUCCEEDED(rv) && avail) {
      mOutListener->OnDataAvailable(mChannel, mURL, mInputStream, 0, avail);

      // try writing again...
      rv = mOutStream->Write(buf, count, countWritten);
    }
  }
  return rv;
}

//
// Find a cached header! Note: Do NOT free this value!
//
const char *
nsMimeBaseEmitter::GetHeaderValue(const char  *aHeaderName)
{
  PRInt32     i;
  char        *retVal = nsnull;
  nsVoidArray *array = mDocHeader? mHeaderArray : mEmbeddedHeaderArray;

  if (!array)
    return nsnull;

  for (i = 0; i < array->Count(); i++)
  {
    headerInfoType *headerInfo = (headerInfoType *)array->ElementAt(i);
    if ( (!headerInfo) || (!headerInfo->name) || (!(*headerInfo->name)) )
      continue;
    
    if (!nsCRT::strcasecmp(aHeaderName, headerInfo->name))
    {
      retVal = headerInfo->value;
      break;
    }
  }

  return retVal;
}

//
// This is called at the start of the header block for all header information in ANY
// AND ALL MESSAGES (yes, quoted, attached, etc...) 
//
// NOTE: This will be called even when headers are will not follow. This is
// to allow us to be notified of the charset of the original message. This is
// important for forward and reply operations
//
NS_IMETHODIMP
nsMimeBaseEmitter::StartHeader(PRBool rootMailHeader, PRBool headerOnly, const char *msgID,
                               const char *outCharset)
{
  mDocHeader = rootMailHeader;

  // If this is not the mail messages header, then we need to create 
  // the mEmbeddedHeaderArray structure for use with this internal header 
  // structure.
  if (!mDocHeader)
  {
    if (mEmbeddedHeaderArray)
      CleanupHeaderArray(mEmbeddedHeaderArray);

    mEmbeddedHeaderArray = new nsVoidArray();
    if (!mEmbeddedHeaderArray)
      return NS_ERROR_OUT_OF_MEMORY;
  }

  // If the main doc, check on updated character set
  if (mDocHeader)
    UpdateCharacterSet(outCharset);

  mCharset.AssignWithConversion(outCharset);
  return NS_OK; 
}

// Ok, if we are here, and we have a aCharset passed in that is not
// UTF-8 or US-ASCII, then we should tag the mChannel member with this
// charset. This is because replying to messages with specified charset's
// need to be tagged as that charset by default.
//
NS_IMETHODIMP
nsMimeBaseEmitter::UpdateCharacterSet(const char *aCharset)
{
  if ( (aCharset) && (PL_strcasecmp(aCharset, "US-ASCII")) &&
        (PL_strcasecmp(aCharset, "ISO-8859-1")) &&
        (PL_strcasecmp(aCharset, "UTF-8")) )
  {
    nsCAutoString contentType;
    
    if (NS_SUCCEEDED(mChannel->GetContentType(contentType)) && !contentType.IsEmpty())
    {
      char *cBegin = contentType.BeginWriting();

      const char *cPtr = PL_strcasestr(cBegin, "charset=");

      if (cPtr)
      {
        char  *ptr = cBegin;
        while (*ptr)
        {
          if ( (*ptr == ' ') || (*ptr == ';') ) 
          {
            if ((ptr + 1) >= cPtr)
            {
              *ptr = '\0';
              break;
            }
          }

          ++ptr;
        }
      }

      // have to set content-type since it could have an embedded null byte
      mChannel->SetContentType(nsDependentCString(cBegin));
      mChannel->SetContentCharset(nsDependentCString(aCharset));
    }
  }

  return NS_OK;
}

//
// This will be called for every header field regardless if it is in an
// internal body or the outer message. 
//
NS_IMETHODIMP
nsMimeBaseEmitter::AddHeaderField(const char *field, const char *value)
{
  if ( (!field) || (!value) )
    return NS_OK;

  nsVoidArray   *tPtr;
  if (mDocHeader)
    tPtr = mHeaderArray;
  else
    tPtr = mEmbeddedHeaderArray;

  // This is a header so we need to cache and output later.
  // Ok, now we will setup the header info for the header array!
  headerInfoType  *ptr = (headerInfoType *) PR_NEWZAP(headerInfoType);
  if ( (ptr) && tPtr)
  {
    ptr->name = nsCRT::strdup(field);
    ptr->value = nsCRT::strdup(value);
    tPtr->AppendElement(ptr);
  }

  return NS_OK;
}

NS_IMETHODIMP
nsMimeBaseEmitter::AddAllHeaders(const char *allheaders, 
                                 const PRInt32 allheadersize)
{
  if (mDocHeader) //We want to set only the main headers of a message, not the potentially embedded one
  {
    nsresult rv;
    nsCOMPtr<nsIMsgMailNewsUrl> msgurl (do_QueryInterface(mURL));
    if (msgurl)
    {
        nsCOMPtr<nsIMimeHeaders> mimeHeaders = do_CreateInstance(NS_IMIMEHEADERS_CONTRACTID, &rv);
        NS_ENSURE_SUCCESS(rv,rv);
        mimeHeaders->Initialize(allheaders, allheadersize);
        msgurl->SetMimeHeaders(mimeHeaders);
    }
  }
  return NS_OK;
}

////////////////////////////////////////////////////////////////////////////////
// The following code is responsible for formatting headers in a manner that is
// identical to the normal XUL output.
////////////////////////////////////////////////////////////////////////////////

nsresult
nsMimeBaseEmitter::WriteHeaderFieldHTML(const char *field, const char *value)
{
  char  *newValue = nsnull;

  if ( (!field) || (!value) )
    return NS_OK;

  //
  // This is a check to see what the pref is for header display. If
  // We should only output stuff that corresponds with that setting.
  //
  if (!EmitThisHeaderForPrefSetting(mHeaderDisplayType, field))
    return NS_OK;

  if ( (mUnicodeConverter) && (mFormat != nsMimeOutput::nsMimeMessageSaveAs) )
  {
    nsXPIDLCString tValue;

    // we're going to need a converter to convert
    nsresult rv = mUnicodeConverter->DecodeMimeHeader(value, getter_Copies(tValue));
    if (NS_SUCCEEDED(rv) && tValue)
    {
      newValue = nsEscapeHTML(tValue);
    }
    else
    {
      newValue = nsEscapeHTML(value);
    }
  }
  else
  {
    newValue = nsCRT::strdup(value);
  }

  if (!newValue)
    return NS_OK;

  mHTMLHeaders.Append("<tr>");
  mHTMLHeaders.Append("<td>");

  if (mFormat == nsMimeOutput::nsMimeMessageSaveAs)
    mHTMLHeaders.Append("<b>");
  else
    mHTMLHeaders.Append("<div class=\"headerdisplayname\" style=\"display:inline;\">");

  // Here is where we are going to try to L10N the tagName so we will always
  // get a field name next to an emitted header value. Note: Default will always
  // be the name of the header itself.
  //
  nsCAutoString  newTagName(field);
  newTagName.CompressWhitespace(PR_TRUE, PR_TRUE);
  ToUpperCase(newTagName);

  char *l10nTagName = LocalizeHeaderName(newTagName.get(), field);
  if ( (!l10nTagName) || (!*l10nTagName) )
    mHTMLHeaders.Append(field);
  else
  {
    mHTMLHeaders.Append(l10nTagName);
    PR_FREEIF(l10nTagName);
  }

  mHTMLHeaders.Append(": ");
  if (mFormat == nsMimeOutput::nsMimeMessageSaveAs)
    mHTMLHeaders.Append("</b>");
  else
    mHTMLHeaders.Append("</div>");

  // Now write out the actual value itself and move on!
  //
  mHTMLHeaders.Append(newValue);
  mHTMLHeaders.Append("</td>");

  mHTMLHeaders.Append("</tr>");

  PR_FREEIF(newValue);
  return NS_OK;
}

nsresult
nsMimeBaseEmitter::WriteHeaderFieldHTMLPrefix()
{
  if ( 
      ( (mFormat == nsMimeOutput::nsMimeMessageSaveAs) && (mFirstHeaders) ) ||
      ( (mFormat == nsMimeOutput::nsMimeMessagePrintOutput) && (mFirstHeaders) )
     )
     /* DO NOTHING */ ;   // rhp: Do nothing...leaving the conditional like this so its 
                          //      easier to see the logic of what is going on. 
  else
    mHTMLHeaders.Append("<br><hr width=\"90%\" size=4><br>");

  mFirstHeaders = PR_FALSE;
  return NS_OK;
}

nsresult
nsMimeBaseEmitter::WriteHeaderFieldHTMLPostfix()
{
  mHTMLHeaders.Append("<br>");
  return NS_OK;
}

NS_IMETHODIMP
nsMimeBaseEmitter::WriteHTMLHeaders()
{
  WriteHeaderFieldHTMLPrefix();

  // Start with the subject, from date info!
  DumpSubjectFromDate();

  // Continue with the to and cc headers
  DumpToCC();

  // Do the rest of the headers, but these will only be written if
  // the user has the "show all headers" pref set
  if (mHeaderDisplayType == nsMimeHeaderDisplayTypes::AllHeaders)
    DumpRestOfHeaders();

  WriteHeaderFieldHTMLPostfix();

  // Now, we need to either append the headers we built up to the 
  // overall body or output to the stream.
  UtilityWriteCRLF(mHTMLHeaders.get());

  mHTMLHeaders = "";

  return NS_OK;
}

nsresult
nsMimeBaseEmitter::DumpSubjectFromDate()
{
  mHTMLHeaders.Append("<table border=0 cellspacing=0 cellpadding=0 width=\"100%\" class=\"header-part1\">");

    // This is the envelope information
    OutputGenericHeader(HEADER_SUBJECT);
    OutputGenericHeader(HEADER_FROM);
    OutputGenericHeader(HEADER_DATE);

    // If we are Quoting a message, then we should dump the To: also
    if ( ( mFormat == nsMimeOutput::nsMimeMessageQuoting ) ||
         ( mFormat == nsMimeOutput::nsMimeMessageBodyQuoting ) )
      OutputGenericHeader(HEADER_TO);

  mHTMLHeaders.Append("</table>");
 
  return NS_OK;
}

nsresult
nsMimeBaseEmitter::DumpToCC()
{
  const char * toField = GetHeaderValue(HEADER_TO);
  const char * ccField = GetHeaderValue(HEADER_CC);
  const char * bccField = GetHeaderValue(HEADER_BCC);
  const char * newsgroupField = GetHeaderValue(HEADER_NEWSGROUPS);

  // only dump these fields if we have at least one of them! When displaying news
  // messages that didn't have a To or Cc field, we'd always get an empty box
  // which looked weird.
  if (toField || ccField || bccField || newsgroupField)
  {
    mHTMLHeaders.Append("<table border=0 cellspacing=0 cellpadding=0 width=\"100%\" class=\"header-part2\">");

    if (toField)
      WriteHeaderFieldHTML(HEADER_TO, toField);
    if (ccField)
      WriteHeaderFieldHTML(HEADER_CC, ccField);
    if (bccField)
      WriteHeaderFieldHTML(HEADER_BCC, bccField);
    if (newsgroupField)
      WriteHeaderFieldHTML(HEADER_NEWSGROUPS, newsgroupField);

    mHTMLHeaders.Append("</table>");
  }

  return NS_OK;
}

nsresult
nsMimeBaseEmitter::DumpRestOfHeaders()
{
  PRInt32     i;
  nsVoidArray *array = mDocHeader? mHeaderArray : mEmbeddedHeaderArray;

  mHTMLHeaders.Append("<table border=0 cellspacing=0 cellpadding=0 width=\"100%\" class=\"header-part3\">");
  
  for (i = 0; i < array->Count(); i++)
  {
    headerInfoType *headerInfo = (headerInfoType *)array->ElementAt(i);
    if ( (!headerInfo) || (!headerInfo->name) || (!(*headerInfo->name)) ||
      (!headerInfo->value) || (!(*headerInfo->value)))
      continue;
    
    if ( (!nsCRT::strcasecmp(HEADER_SUBJECT, headerInfo->name)) ||
      (!nsCRT::strcasecmp(HEADER_DATE, headerInfo->name)) ||
      (!nsCRT::strcasecmp(HEADER_FROM, headerInfo->name)) ||
      (!nsCRT::strcasecmp(HEADER_TO, headerInfo->name)) ||
      (!nsCRT::strcasecmp(HEADER_CC, headerInfo->name)) )
      continue;
    
    WriteHeaderFieldHTML(headerInfo->name, headerInfo->value);
  }
  
  mHTMLHeaders.Append("</table>");
  return NS_OK;
}

nsresult
nsMimeBaseEmitter::OutputGenericHeader(const char *aHeaderVal)
{
  const char *val = GetHeaderValue(aHeaderVal);

  if (val)
    return WriteHeaderFieldHTML(aHeaderVal, val);

  return NS_ERROR_FAILURE;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// These are the methods that should be implemented by the child class!
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//
// This should be implemented by the child class if special processing
// needs to be done when the entire message is read.
//
NS_IMETHODIMP
nsMimeBaseEmitter::Complete()
{
  // If we are here and still have data to write, we should try
  // to flush it...if we try and fail, we should probably return
  // an error!
  PRUint32      written;

  nsresult rv = NS_OK;
  while ( NS_SUCCEEDED(rv) && (mBufferMgr) && (mBufferMgr->GetSize() > 0))
    rv = Write("", 0, &written);

  if (mOutListener)
  {
    PRUint32 bytesInStream = 0;
    nsresult rv2 = mInputStream->Available(&bytesInStream);
	  NS_ASSERTION(NS_SUCCEEDED(rv2), "Available failed");
    if (bytesInStream)
    {
      nsCOMPtr<nsIRequest> request = do_QueryInterface(mChannel);
      rv2 = mOutListener->OnDataAvailable(request, mURL, mInputStream, 0, bytesInStream);
    }
  }

  return NS_OK;
}

//
// This needs to do the right thing with the stored information. It only 
// has to do the output functions, this base class will take care of the 
// memory cleanup
//
NS_IMETHODIMP
nsMimeBaseEmitter::EndHeader()
{
  return NS_OK;
}

// body handling routines
NS_IMETHODIMP
nsMimeBaseEmitter::StartBody(PRBool bodyOnly, const char *msgID, const char *outCharset)
{
  return NS_OK;
}

NS_IMETHODIMP
nsMimeBaseEmitter::WriteBody(const char *buf, PRUint32 size, PRUint32 *amountWritten)
{
  return NS_OK;
}

NS_IMETHODIMP
nsMimeBaseEmitter::EndBody()
{
  return NS_OK;
}
