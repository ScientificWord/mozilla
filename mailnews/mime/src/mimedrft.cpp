/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
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
 * ***** END LICENSE BLOCK *****
 * This Original Code has been modified by IBM Corporation. Modifications made by IBM 
 * described herein are Copyright (c) International Business Machines Corporation, 2000.
 * Modifications to Mozilla code or documentation identified per MPL Section 3.3
 *
 * Date             Modified by     Description of modification
 * 04/20/2000       IBM Corp.      OS/2 VisualAge build.
 */
#include "nsCOMPtr.h"
#include "modmimee.h"
#include "mimeobj.h"
#include "modlmime.h"
#include "mimei.h"
#include "mimebuf.h"
#include "mimemoz2.h"
#include "mimemsg.h"
#include "nsMimeTypes.h"

#include "prmem.h"
#include "plstr.h"
#include "prprf.h"
#include "prio.h"
#include "nsIPrefService.h"
#include "nsIPrefBranch.h"
#include "msgCore.h"
#include "nsCRT.h"
#include "nsEscape.h"
#include "nsIFileSpec.h"
#include "nsIMsgSend.h"
#include "nsFileStream.h"
#include "nsMimeStringResources.h"
#include "nsIIOService.h"
#include "comi18n.h"
#include "nsIMsgCompFields.h"
#include "nsMsgCompCID.h"
#include "nsIMsgComposeService.h"
#include "nsMsgI18N.h"
#include "nsNativeCharsetUtils.h"
#include "nsSpecialSystemDirectory.h"
#include "nsIMsgMessageService.h"
#include "nsMsgUtils.h"
#include "nsXPIDLString.h"
#include "nsReadableUtils.h"
#include "nsCExternalHandlerService.h"
#include "nsIMIMEService.h"
#include "nsIMsgHeaderParser.h"
#include "nsIMsgAccountManager.h"
#include "nsMsgBaseCID.h"

//
// Header strings...
//
#define HEADER_NNTP_POSTING_HOST      "NNTP-Posting-Host"
#define MIME_HEADER_TABLE             "<TABLE CELLPADDING=0 CELLSPACING=0 BORDER=0 class=\"moz-email-headers-table\">"
#define HEADER_START_JUNK             "<TR><TH VALIGN=BASELINE ALIGN=RIGHT NOWRAP>"
#define HEADER_MIDDLE_JUNK            ": </TH><TD>"
#define HEADER_END_JUNK               "</TD></TR>"

//
// Forward declarations...
//
extern "C" char     *MIME_StripContinuations(char *original);
nsresult            mime_decompose_file_init_fn ( void *stream_closure, MimeHeaders *headers );
nsresult            mime_decompose_file_output_fn ( const char *buf, PRInt32 size, void *stream_closure );
nsresult            mime_decompose_file_close_fn ( void *stream_closure );
extern int          MimeHeaders_build_heads_list(MimeHeaders *hdrs);

// CID's
static NS_DEFINE_CID(kCMsgComposeServiceCID,  NS_MSGCOMPOSESERVICE_CID);       

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
// THIS SHOULD ALL MOVE TO ANOTHER FILE AFTER LANDING!
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

// safe filename for all OSes
#define SAFE_TMP_FILENAME "nsmime.tmp"

//
// Create a file spec for the a unique temp file
// on the local machine. Caller must free memory
//
nsFileSpec * 
nsMsgCreateTempFileSpec(const char *tFileName)
{
  //Calling NS_MsgHashIfNecessary so that when Replies are forwarded - the ':' in the subject line doesn't cause problems
  nsFileSpec *tmpSpec = new nsFileSpec(nsSpecialSystemDirectory(nsSpecialSystemDirectory::OS_TemporaryDirectory));
  NS_ASSERTION(tmpSpec, "out of memory");
  if (!tmpSpec)
    return nsnull;

  nsresult rv = NS_OK;
  nsCAutoString tempName;
  if ((!tFileName) || (!*tFileName)) {
    tempName = SAFE_TMP_FILENAME;
  }
  else {
    nsAutoString tempNameUni; 
    if (NS_FAILED(NS_CopyNativeToUnicode(nsDependentCString(tFileName),
                                         tempNameUni))) {
      tempName = SAFE_TMP_FILENAME;
      goto fallback;
    }

    PRInt32 dotChar = tempNameUni.RFindChar('.');
    if (dotChar == kNotFound) {
       rv = NS_MsgHashIfNecessary(tempNameUni);
    }
    else {
      // this scary code will not lose any 3 or 4 character normal extensions, 
      // like foo.eml, foo.txt, foo.html
      // this code is to turn arbitrary attachment file names to ones 
      // that are safe for the underlying OS 
      // eventually, this code will go away once bug #86089 is fixed
      nsAutoString extension;
      tempNameUni.Right(extension, tempNameUni.Length() - dotChar - 1);
      tempNameUni.Truncate(dotChar);
      rv = NS_MsgHashIfNecessary(tempNameUni);
      if (NS_SUCCEEDED(rv)) {
        rv = NS_MsgHashIfNecessary(extension);    // should be a NOOP for normal 3 or 4 char extension
        if (NS_SUCCEEDED(rv)) {
          tempNameUni += '.';
          tempNameUni += extension;
          // hash the combinded string, so that filenames like "<giant string>.<giant string>" are made safe
          rv = NS_MsgHashIfNecessary(tempNameUni);   
        }
      }
    } 
    rv = NS_CopyUnicodeToNative(tempNameUni, tempName);
    NS_ASSERTION(NS_SUCCEEDED(rv), "UTF-16 to native filename failed"); 
  }

fallback:
  NS_ASSERTION(NS_SUCCEEDED(rv), "hash failed");
  if (NS_FAILED(rv)) {
    tempName = SAFE_TMP_FILENAME;
  }
  // we are guaranteed that tempName is safe for the underlying OS
  *tmpSpec += tempName.get();

  tmpSpec->MakeUnique();
  return tmpSpec;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
// END OF - THIS SHOULD ALL MOVE TO ANOTHER FILE AFTER LANDING!
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

typedef enum {
  nsMsg_RETURN_RECEIPT_BOOL_HEADER_MASK = 0,
  nsMsg_ENCRYPTED_BOOL_HEADER_MASK,
  nsMsg_SIGNED_BOOL_HEADER_MASK,
  nsMsg_UUENCODE_BINARY_BOOL_HEADER_MASK,
  nsMsg_ATTACH_VCARD_BOOL_HEADER_MASK,
  nsMsg_LAST_BOOL_HEADER_MASK     // last boolean header mask; must be the last one 
                                  // DON'T remove.
} nsMsgBoolHeaderSet;

#ifdef NS_DEBUG
extern "C" void
mime_dump_attachments ( nsMsgAttachmentData *attachData )
{
  PRInt32     i = 0;
  struct nsMsgAttachmentData  *tmp = attachData;

  while ( (tmp) && (tmp->real_name) )
  {
    printf("Real Name         : %s\n", tmp->real_name);

    if ( tmp->url ) 
    {
      nsCAutoString spec;
      tmp->url->GetSpec(spec);
      printf("URL               : %s\n", spec.get());
    }

    printf("Desired Type      : %s\n", tmp->desired_type ? tmp->desired_type : "nsnull");
    printf("Real Type         : %s\n", tmp->real_type ? tmp->real_type : "nsnull");
    printf("Real Encoding     : %s\n", tmp->real_encoding ? tmp->real_encoding : "nsnull"); 
    printf("Description       : %s\n", tmp->description) ? tmp->description : "nsnull";
    printf("Mac Type          : %s\n", tmp->x_mac_type ? tmp->x_mac_type : "nsnull");
    printf("Mac Creator       : %s\n", tmp->x_mac_creator ? tmp->x_mac_creator : "nsnull");
    i++;
    tmp++;
  }
}
#endif

nsresult
CreateTheComposeWindow(nsIMsgCompFields *   compFields,
                       nsMsgAttachmentData *attachmentList,
                       MSG_ComposeType      composeType,
                       MSG_ComposeFormat    composeFormat,
                       nsIMsgIdentity *     identity,
                       const char *         originalMsgURI
                       )
{
  nsresult            rv;
  MSG_ComposeFormat   format = nsIMsgCompFormat::Default;

#ifdef NS_DEBUG
  mime_dump_attachments ( attachmentList );
#endif

  nsMsgAttachmentData *curAttachment = attachmentList;
  if (curAttachment)
  {
    nsCAutoString spec;

    while (curAttachment && curAttachment->real_name) //JFD: Why do we check for real_name?
    {
      rv = curAttachment->url->GetSpec(spec);
      if (NS_SUCCEEDED(rv))
      {
        nsCOMPtr<nsIMsgAttachment> attachment = do_CreateInstance(NS_MSGATTACHMENT_CONTRACTID, &rv);
        if (NS_SUCCEEDED(rv) && attachment)
        {
          nsAutoString nameStr;
          rv = ConvertToUnicode("UTF-8", curAttachment->real_name, nameStr);
          if (NS_FAILED(rv))
            CopyASCIItoUTF16(curAttachment->real_name, nameStr);
          attachment->SetName(nameStr);
          attachment->SetUrl(spec.get());
          attachment->SetTemporary(PR_TRUE);
          attachment->SetContentType(curAttachment->real_type);
          attachment->SetMacType(curAttachment->x_mac_type);
          attachment->SetMacCreator(curAttachment->x_mac_creator);
          compFields->AddAttachment(attachment);
        }
      }
      curAttachment++;
    }
  }

  nsCOMPtr<nsIMsgComposeService> msgComposeService = 
           do_GetService(kCMsgComposeServiceCID, &rv);
  if ((NS_FAILED(rv)) || (!msgComposeService))
    return rv; 
  
  if (identity && composeType == nsIMsgCompType::ForwardInline)
  {
    PRBool composeHtml = PR_FALSE;
    identity->GetComposeHtml(&composeHtml);
    if (composeHtml)
      format = nsIMsgCompFormat::HTML;
    else
    {
      format = nsIMsgCompFormat::PlainText;
      /* do we we need to convert the HTML body to plain text? */
      if (composeFormat == nsIMsgCompFormat::HTML)
        compFields->ConvertBodyToPlainText();
    }
  }
  else
    format = composeFormat;
  
  nsCOMPtr<nsIMsgComposeParams> pMsgComposeParams (do_CreateInstance(NS_MSGCOMPOSEPARAMS_CONTRACTID, &rv));
  if (NS_SUCCEEDED(rv) && pMsgComposeParams)
  {
    pMsgComposeParams->SetType(composeType);
    pMsgComposeParams->SetFormat(format);
    pMsgComposeParams->SetIdentity(identity);
    pMsgComposeParams->SetComposeFields(compFields);
    if (originalMsgURI)
      pMsgComposeParams->SetOriginalMsgURI(originalMsgURI);
    
    rv = msgComposeService->OpenComposeWindowWithParams(nsnull /* default chrome */, pMsgComposeParams);
  }
  return rv;
}

nsresult
CreateCompositionFields(const char        *from,
                        const char        *reply_to,
                        const char        *to,
                        const char        *cc,
                        const char        *bcc,
                        const char        *fcc,
                        const char        *newsgroups,
                        const char        *followup_to,
                        const char        *organization,
                        const char        *subject,
                        const char        *references,
                        const char        *other_random_headers,
                        const char        *priority,
                        const char        *newspost_url,
                        PRBool            xlate_p,
                        PRBool            sign_p,
                        char              *charset,
                        nsIMsgCompFields  **_retval)
{
  NS_ENSURE_ARG_POINTER(_retval);
  
  nsresult rv;
  *_retval = nsnull;
  
  nsCOMPtr<nsIMsgCompFields> cFields = do_CreateInstance(NS_MSGCOMPFIELDS_CONTRACTID, &rv);
  NS_ENSURE_SUCCESS(rv, rv);
  NS_ENSURE_TRUE(cFields, NS_ERROR_OUT_OF_MEMORY);
  
  // Now set all of the passed in stuff...
  cFields->SetCharacterSet(!nsCRT::strcasecmp("us-ascii", charset) ? "ISO-8859-1" : charset);
  
  char *val;
  
  if (from) {
    val = MIME_DecodeMimeHeader(from, charset, PR_FALSE, PR_TRUE);
    cFields->SetFrom(NS_ConvertUTF8toUTF16(val ? val : from));
    PR_FREEIF(val);
  }
  
  if (subject) {
    val = MIME_DecodeMimeHeader(subject, charset, PR_FALSE, PR_TRUE);
    cFields->SetSubject(NS_ConvertUTF8toUTF16(val ? val : subject));
    PR_FREEIF(val);
  }
  
  if (reply_to) {
    val = MIME_DecodeMimeHeader(reply_to, charset, PR_FALSE, PR_TRUE);
    cFields->SetReplyTo(NS_ConvertUTF8toUTF16(val ? val : reply_to));
    PR_FREEIF(val);
  }
  
  if (to) {
    val = MIME_DecodeMimeHeader(to, charset, PR_FALSE, PR_TRUE);
    cFields->SetTo(NS_ConvertUTF8toUTF16(val ? val : to));
    PR_FREEIF(val);
  }
  
  if (cc) {
    val = MIME_DecodeMimeHeader(cc, charset, PR_FALSE, PR_TRUE);
    cFields->SetCc(NS_ConvertUTF8toUTF16(val ? val : cc));
    PR_FREEIF(val);
  }
  
  if (bcc) {
    val = MIME_DecodeMimeHeader(bcc, charset, PR_FALSE, PR_TRUE);
    cFields->SetBcc(NS_ConvertUTF8toUTF16(val ? val : bcc));
    PR_FREEIF(val);
  }
  
  if (fcc) {
    val = MIME_DecodeMimeHeader(fcc, charset, PR_FALSE, PR_TRUE);
    cFields->SetFcc(NS_ConvertUTF8toUTF16(val ? val : fcc));
    PR_FREEIF(val);
  }
  
  if (newsgroups) {
    // fixme: the newsgroups header had better be decoded using the server-side
    // character encoding,but this |charset| might be different from it.
    val = MIME_DecodeMimeHeader(newsgroups, charset, PR_FALSE, PR_TRUE);
    cFields->SetNewsgroups(NS_ConvertUTF8toUTF16(val ? val : newsgroups));
    PR_FREEIF(val);
  }
  
  if (followup_to) {
    val = MIME_DecodeMimeHeader(followup_to, charset, PR_FALSE, PR_TRUE);
    cFields->SetFollowupTo(val ? val : followup_to);
    PR_FREEIF(val);
  }
  
  if (organization) {
    val = MIME_DecodeMimeHeader(organization, charset, PR_FALSE, PR_TRUE);
    cFields->SetOrganization(NS_ConvertUTF8toUTF16(val ? val : organization));
    PR_FREEIF(val);
  }
  
  if (references) {
    val = MIME_DecodeMimeHeader(references, charset, PR_FALSE, PR_TRUE);
    cFields->SetReferences(val ? val : references);
    PR_FREEIF(val);
  }
  
  if (other_random_headers) {
    val = MIME_DecodeMimeHeader(other_random_headers, charset, PR_FALSE, PR_TRUE);
    cFields->SetOtherRandomHeaders(NS_ConvertUTF8toUTF16(val ? val : other_random_headers));
    PR_FREEIF(val);
  }
  
  if (priority) {
    val = MIME_DecodeMimeHeader(priority, charset, PR_FALSE, PR_TRUE);
    cFields->SetPriority(val ? val : priority);
    PR_FREEIF(val);
  }
  
  if (newspost_url) {
    val = MIME_DecodeMimeHeader(newspost_url, charset, PR_FALSE, PR_TRUE);
    cFields->SetNewspostUrl(val ? val : newspost_url);
    PR_FREEIF(val);
  }
  
  *_retval = cFields;
  NS_IF_ADDREF(*_retval);
  
  return rv;
}

// RICHIE:
// Not sure what this preference is in the new world.
PRBool
GetMailXlateionPreference(void)
{
  nsresult res;
  PRBool   xlate = PR_FALSE;

  nsCOMPtr<nsIPrefBranch> prefBranch(do_GetService(NS_PREFSERVICE_CONTRACTID, &res));
  if(NS_SUCCEEDED(res))
    prefBranch->GetBoolPref("mail.unknown", &xlate);

  return xlate;
}

// RICHIE:
// Not sure what this preference is in the new world.
PRBool
GetMailSigningPreference(void)
{
  nsresult  res;
  PRBool    signit = PR_FALSE;

  nsCOMPtr<nsIPrefBranch> prefBranch(do_GetService(NS_PREFSERVICE_CONTRACTID, &res));
  if(NS_SUCCEEDED(res))
    prefBranch->GetBoolPref("mail.unknown", &signit);

  return signit;
}
  
static int
dummy_file_write( char *buf, PRInt32 size, void *fileHandle )
{
  if (!fileHandle)
    return NS_ERROR_FAILURE;

  nsOutputFileStream  *tStream = (nsOutputFileStream *) fileHandle;
  return tStream->write(buf, size);
}

static int PR_CALLBACK
mime_parse_stream_write ( nsMIMESession *stream, const char *buf, PRInt32 size )
{
  struct mime_draft_data *mdd = (struct mime_draft_data *) stream->data_object;  
  NS_ASSERTION ( mdd, "null mime draft data!" );

  if ( !mdd || !mdd->obj ) 
    return -1;

  return mdd->obj->clazz->parse_buffer ((char *) buf, size, mdd->obj);
}

static void
mime_free_attach_data ( nsMsgAttachmentData *attachData)
{
  struct nsMsgAttachmentData  *tmp = attachData;

  while (tmp && tmp->real_name) 
  {
    if (tmp->url)
    {
      delete tmp->url;
      tmp->url = nsnull;
    }
    PR_FREEIF(tmp->real_name);

    PR_FREEIF(tmp->desired_type);
    PR_FREEIF(tmp->real_type);
    PR_FREEIF(tmp->real_encoding); 
    PR_FREEIF(tmp->description);
    PR_FREEIF(tmp->x_mac_type);
    PR_FREEIF(tmp->x_mac_creator);

    tmp++;
  }
}

static void
mime_free_attachments ( nsMsgAttachedFile *attachments, int count )
{
  int               i;
  nsMsgAttachedFile *cur = attachments;

  NS_ASSERTION ( attachments && count > 0, "freeing attachments but there aren't any");
  if ( !attachments || count <= 0 ) 
    return;

  for ( i = 0; i < count; i++, cur++ ) 
  {
    cur->orig_url=nsnull;

    PR_FREEIF ( cur->type );
    PR_FREEIF ( cur->encoding );
    PR_FREEIF ( cur->description );
    PR_FREEIF ( cur->x_mac_type );
    PR_FREEIF ( cur->x_mac_creator );
    if ( cur->file_spec ) 
    {
      cur->file_spec->Delete(PR_FALSE);
      delete ( cur->file_spec );
    }
  }

  PR_FREEIF( attachments );
}

static nsMsgAttachmentData *
mime_draft_process_attachments(mime_draft_data *mdd) 
{
  if (!mdd)
    return nsnull;

  nsMsgAttachmentData         *attachData = NULL, *tmp = NULL;
  nsMsgAttachedFile           *tmpFile = NULL;
  int                         i;

  //It's possible we must treat the message body as attachment!
  PRBool bodyAsAttachment = PR_FALSE;
  if (  mdd->messageBody &&
        mdd->messageBody->type && *mdd->messageBody->type &&
        ( PL_strcasestr(mdd->messageBody->type, "text/html") == nsnull ) &&
        ( PL_strcasestr(mdd->messageBody->type, "text/plain") == nsnull ) &&
        ( PL_strcasecmp(mdd->messageBody->type, "text") != 0 )
     )
     bodyAsAttachment = PR_TRUE;

  if ( (!mdd->attachments || !mdd->attachments_count) && !bodyAsAttachment)
    return nsnull;

  PRInt32 totalCount = mdd->attachments_count;
  if (bodyAsAttachment)
    totalCount ++;
  attachData = (nsMsgAttachmentData *) PR_CALLOC( ( (totalCount + 1) * sizeof (nsMsgAttachmentData) ) );
  if ( !attachData ) 
    return nsnull;

  //mdd->messageBody and mdd->attachments are the same type!
  if (bodyAsAttachment)
    tmpFile = mdd->messageBody;
  else
    tmpFile = mdd->attachments;
  tmp = attachData;

  for ( i=0; i < totalCount; i++, tmp++) 
  {
    if (tmpFile->type) 
    {
      if (nsCRT::strcasecmp ( tmpFile->type, "text/x-vcard") == 0)
        NS_MsgSACopy (&(tmp->real_name), tmpFile->description);
    }

    if ( tmpFile->orig_url ) 
    {
      nsCAutoString tmpSpec;
      if (NS_FAILED(tmpFile->orig_url->GetSpec(tmpSpec)))
        goto FAIL;

      if (NS_FAILED(nsMimeNewURI(&(tmp->url), tmpSpec.get(), nsnull)))
      {
        goto FAIL; 
      }

      NS_ADDREF(tmp->url);
      if (!tmp->real_name)
      {
        if (tmpFile->real_name)
          NS_MsgSACopy ( &(tmp->real_name), tmpFile->real_name );
        else
          NS_MsgSACopy ( &(tmp->real_name), tmpSpec.get() );
      }
    }

    if ( tmpFile->type ) 
    {
      NS_MsgSACopy ( &(tmp->desired_type), tmpFile->type );
      NS_MsgSACopy ( &(tmp->real_type), tmpFile->type );
    }

    if ( tmpFile->encoding ) 
    {
      NS_MsgSACopy ( &(tmp->real_encoding), tmpFile->encoding );
    }

    if ( tmpFile->description ) 
    {
      NS_MsgSACopy ( &(tmp->description), tmpFile->description );
    }

    if ( tmpFile->x_mac_type ) 
    {
      NS_MsgSACopy ( &(tmp->x_mac_type), tmpFile->x_mac_type );
    }

    if ( tmpFile->x_mac_creator ) 
    {
      NS_MsgSACopy ( &(tmp->x_mac_creator), tmpFile->x_mac_creator );
    }

    if (bodyAsAttachment && (i == 0))
      tmpFile = mdd->attachments;
    else
      tmpFile++;
  }

  return (attachData);

FAIL:
  mime_free_attach_data (attachData);
  PR_FREEIF(attachData);
  return nsnull;
}

static void 
mime_fix_up_html_address( char **addr)
{
  //
  // We need to replace paired <> they are treated as HTML tag 
  //
  if (addr && *addr && PL_strchr(*addr, '<') && PL_strchr(*addr, '>'))
  {
    char *lt = NULL;
    PRInt32 newLen = 0;
    do 
    {
      newLen = strlen(*addr) + 3 + 1;
      *addr = (char *) PR_REALLOC(*addr, newLen);
      NS_ASSERTION (*addr, "out of memory fixing up html address");
      lt = PL_strchr(*addr, '<');
      NS_ASSERTION(lt, "couldn't find < char in address");
      memmove(lt+4, lt+1, newLen - 4 - (lt - *addr));
      *lt++ = '&';
      *lt++ = 'l';
      *lt++ = 't';
      *lt = ';';
    } while (PL_strchr(*addr, '<'));
  }
}

static void 
mime_intl_insert_message_header_1(char        **body, 
                                  char        **hdr_value,
                                  char        *hdr_str, 
                                  const char  *html_hdr_str,
                                  char        *mailcharset,
                                  PRBool      htmlEdit)
{
  if (!body || !hdr_value || !hdr_str)
    return;

  if (htmlEdit)
  {
    NS_MsgSACat(body, HEADER_START_JUNK);
  }
  else
  {
    NS_MsgSACat(body, MSG_LINEBREAK);
  }
  if (!html_hdr_str)
    html_hdr_str = hdr_str;
  NS_MsgSACat(body, html_hdr_str);
  if (htmlEdit)
  {
    NS_MsgSACat(body, HEADER_MIDDLE_JUNK);
  }
  else
    NS_MsgSACat(body, ": ");

    // MIME decode header
    char* utf8 = MIME_DecodeMimeHeader(*hdr_value, mailcharset, PR_FALSE,
                                       PR_TRUE);
    if (NULL != utf8) {
        NS_MsgSACat(body, utf8);
        PR_Free(utf8);
    } else {
        NS_MsgSACat(body, *hdr_value); // raw MIME encoded string
    }

  if (htmlEdit)
    NS_MsgSACat(body, HEADER_END_JUNK);
}

char *
MimeGetNamedString(PRInt32 id)
{
  static char   retString[256];

  retString[0] = '\0';
  char *tString = MimeGetStringByID(id);
  if (tString)
  {
    PL_strncpy(retString, tString, sizeof(retString)); 
    PR_Free(tString);
  }
  return retString;
}

/* given an address string passed though parameter "address", this one will be converted
   and returned through the same parameter. The original string will be destroyed
*/
static void UnquoteMimeAddress(nsIMsgHeaderParser* parser, char** address)
{
  if (parser && address && *address && **address)
  {
    char *result;
    if (NS_SUCCEEDED(parser->UnquotePhraseOrAddr(*address, PR_FALSE, &result)))
    {
      if (result && *result)
      {
        PR_Free(*address);
        *address = result;
        return;
      }
      PR_FREEIF(result);
    }
  }

  return;
}

static void 
mime_insert_all_headers(char            **body,
                        MimeHeaders     *headers,
                        MSG_ComposeFormat composeFormat,
                        char            *mailcharset)
{
  nsCOMPtr<nsIMsgHeaderParser> parser = do_GetService(NS_MAILNEWS_MIME_HEADER_PARSER_CONTRACTID);

  PRBool htmlEdit = (composeFormat == nsIMsgCompFormat::HTML);
  char *newBody = NULL;
  char *html_tag = nsnull;
  if (*body)
    html_tag = PL_strcasestr(*body, "<HTML>");
  int i;

  if (!headers->done_p)
  {
    MimeHeaders_build_heads_list(headers);
    headers->done_p = PR_TRUE;
  }

  if (htmlEdit)
  {
    NS_MsgSACopy(&(newBody), "<HTML><BODY><BR><BR>");

    NS_MsgSACat(&newBody, MimeGetNamedString(MIME_FORWARDED_MESSAGE_HTML_USER_WROTE));
    NS_MsgSACat(&newBody, MIME_HEADER_TABLE);
  }
  else
  {
    NS_MsgSACopy(&(newBody), MSG_LINEBREAK MSG_LINEBREAK);
    NS_MsgSACat(&newBody, MimeGetNamedString(MIME_FORWARDED_MESSAGE_HTML_USER_WROTE));
  }

  for (i = 0; i < headers->heads_size; i++)
  {
    char *head = headers->heads[i];
    char *end = (i == headers->heads_size-1
           ? headers->all_headers + headers->all_headers_fp
           : headers->heads[i+1]);
    char *colon, *ocolon;
    char *contents;
    char *name = 0;
    char *c2 = 0;

    // Hack for BSD Mailbox delimiter. 
    if (i == 0 && head[0] == 'F' && !strncmp(head, "From ", 5))
    {
      colon = head + 4;
      contents = colon + 1;
    }
    else
    {
      /* Find the colon. */
      for (colon = head; colon < end; colon++)
      if (*colon == ':') break;

      if (colon >= end) continue;   /* junk */

      /* Back up over whitespace before the colon. */
      ocolon = colon;
      for (; colon > head && nsCRT::IsAsciiSpace(colon[-1]); colon--)
      ;

      contents = ocolon + 1;
    }

    /* Skip over whitespace after colon. */
    while (contents <= end && nsCRT::IsAsciiSpace(*contents))
    contents++;

    /* Take off trailing whitespace... */
    while (end > contents && nsCRT::IsAsciiSpace(end[-1]))
    end--;

    name = (char *)PR_MALLOC(colon - head + 1);
    if (!name) 
      return /* MIME_OUT_OF_MEMORY */;
    memcpy(name, head, colon - head);
    name[colon - head] = 0;

    c2 = (char *)PR_MALLOC(end - contents + 1);
    if (!c2)
    {
      PR_Free(name);
      return /* MIME_OUT_OF_MEMORY */;
    }
    memcpy(c2, contents, end - contents);
    c2[end - contents] = 0;
    
    /* Do not reveal bcc recipients when forwarding a message!
       See http://bugzilla.mozilla.org/show_bug.cgi?id=41150
    */
    if (nsCRT::strcasecmp(name, "bcc") != 0)
    {
      if (htmlEdit)
        mime_fix_up_html_address(&c2);
        
      if (!nsCRT::strcasecmp(name, "resent-from") || !nsCRT::strcasecmp(name, "from") ||
          !nsCRT::strcasecmp(name, "resent-to") || !nsCRT::strcasecmp(name, "to") ||
          !nsCRT::strcasecmp(name, "resent-cc") || !nsCRT::strcasecmp(name, "cc") ||
          !nsCRT::strcasecmp(name, "reply-to"))
        UnquoteMimeAddress(parser, &c2);

      mime_intl_insert_message_header_1(&newBody, &c2, name, name, mailcharset, htmlEdit);
    }
    PR_Free(name);
    PR_Free(c2);
  }

  if (htmlEdit)
  {
    NS_MsgSACat(&newBody, "</TABLE>");
    NS_MsgSACat(&newBody, MSG_LINEBREAK "<BR><BR>");
    if (html_tag)
      NS_MsgSACat(&newBody, html_tag+6);
    else if (*body)
        NS_MsgSACat(&newBody, *body);
  }
  else
  {
    NS_MsgSACat(&newBody, MSG_LINEBREAK MSG_LINEBREAK);
    if (*body)
      NS_MsgSACat(&newBody, *body);
  }

  if (newBody)
  {
    PR_FREEIF(*body);
    *body = newBody;
  }
}

static void 
mime_insert_normal_headers(char             **body,
                           MimeHeaders      *headers,
                           MSG_ComposeFormat  composeFormat,
                           char             *mailcharset)
{
  char *newBody = nsnull;
  char *subject = MimeHeaders_get(headers, HEADER_SUBJECT, PR_FALSE, PR_FALSE);
  char *resent_comments = MimeHeaders_get(headers, HEADER_RESENT_COMMENTS, PR_FALSE, PR_FALSE);
  char *resent_date = MimeHeaders_get(headers, HEADER_RESENT_DATE, PR_FALSE, PR_TRUE); 
  char *resent_from = MimeHeaders_get(headers, HEADER_RESENT_FROM, PR_FALSE, PR_TRUE);
  char *resent_to = MimeHeaders_get(headers, HEADER_RESENT_TO, PR_FALSE, PR_TRUE);
  char *resent_cc = MimeHeaders_get(headers, HEADER_RESENT_CC, PR_FALSE, PR_TRUE);
  char *date = MimeHeaders_get(headers, HEADER_DATE, PR_FALSE, PR_TRUE);
  char *from = MimeHeaders_get(headers, HEADER_FROM, PR_FALSE, PR_TRUE);
  char *reply_to = MimeHeaders_get(headers, HEADER_REPLY_TO, PR_FALSE, PR_TRUE);
  char *organization = MimeHeaders_get(headers, HEADER_ORGANIZATION, PR_FALSE, PR_FALSE);
  char *to = MimeHeaders_get(headers, HEADER_TO, PR_FALSE, PR_TRUE);
  char *cc = MimeHeaders_get(headers, HEADER_CC, PR_FALSE, PR_TRUE);
  char *newsgroups = MimeHeaders_get(headers, HEADER_NEWSGROUPS, PR_FALSE, PR_TRUE);
  char *followup_to = MimeHeaders_get(headers, HEADER_FOLLOWUP_TO, PR_FALSE, PR_TRUE);
  char *references = MimeHeaders_get(headers, HEADER_REFERENCES, PR_FALSE, PR_TRUE);
  const char *html_tag = nsnull;
  if (*body)
    html_tag = PL_strcasestr(*body, "<HTML>");
  PRBool htmlEdit = composeFormat == nsIMsgCompFormat::HTML;
  
  if (!from)
    from = MimeHeaders_get(headers, HEADER_SENDER, PR_FALSE, PR_TRUE);
  if (!resent_from)
    resent_from = MimeHeaders_get(headers, HEADER_RESENT_SENDER, PR_FALSE,
                    PR_TRUE); 
  
  nsCOMPtr<nsIMsgHeaderParser> parser = do_GetService(NS_MAILNEWS_MIME_HEADER_PARSER_CONTRACTID);
  UnquoteMimeAddress(parser, &resent_from);
  UnquoteMimeAddress(parser, &resent_to);
  UnquoteMimeAddress(parser, &resent_cc);
  UnquoteMimeAddress(parser, &reply_to);
  UnquoteMimeAddress(parser, &from);
  UnquoteMimeAddress(parser, &to);
  UnquoteMimeAddress(parser, &cc);

  if (htmlEdit)
  {
    NS_MsgSACopy(&(newBody), "<HTML><BODY><BR><BR>");
    NS_MsgSACat(&newBody, MimeGetNamedString(MIME_FORWARDED_MESSAGE_HTML_USER_WROTE));
    NS_MsgSACat(&newBody, MIME_HEADER_TABLE);
  }
  else
  {
    NS_MsgSACopy(&(newBody), MSG_LINEBREAK MSG_LINEBREAK);
    NS_MsgSACat(&newBody, MimeGetNamedString(MIME_FORWARDED_MESSAGE_HTML_USER_WROTE));
  }
  if (subject)
    mime_intl_insert_message_header_1(&newBody, &subject, HEADER_SUBJECT,
                      MimeGetNamedString(MIME_MHTML_SUBJECT),
                      mailcharset, htmlEdit);
  if (resent_comments)
    mime_intl_insert_message_header_1(&newBody, &resent_comments,
                      HEADER_RESENT_COMMENTS, 
                      MimeGetNamedString(MIME_MHTML_RESENT_COMMENTS),
                      mailcharset, htmlEdit);
  if (resent_date)
    mime_intl_insert_message_header_1(&newBody, &resent_date,
                      HEADER_RESENT_DATE,
                      MimeGetNamedString(MIME_MHTML_RESENT_DATE),
                      mailcharset, htmlEdit);
  if (resent_from)
  {
    if (htmlEdit) mime_fix_up_html_address(&resent_from);
    mime_intl_insert_message_header_1(&newBody, &resent_from,
                      HEADER_RESENT_FROM,
                      MimeGetNamedString(MIME_MHTML_RESENT_FROM),
                      mailcharset, htmlEdit);
  }
  if (resent_to)
  {
    if (htmlEdit) mime_fix_up_html_address(&resent_to);
    mime_intl_insert_message_header_1(&newBody, &resent_to,
                      HEADER_RESENT_TO,
                      MimeGetNamedString(MIME_MHTML_RESENT_TO),
                      mailcharset, htmlEdit);
  }
  if (resent_cc)
  {
    if (htmlEdit) mime_fix_up_html_address(&resent_cc);
    mime_intl_insert_message_header_1(&newBody, &resent_cc,
                      HEADER_RESENT_CC,
                      MimeGetNamedString(MIME_MHTML_RESENT_CC),
                      mailcharset, htmlEdit);
  }
  if (date)
    mime_intl_insert_message_header_1(&newBody, &date, HEADER_DATE,
                      MimeGetNamedString(MIME_MHTML_DATE),
                      mailcharset, htmlEdit);
  if (from)
  {
    if (htmlEdit) mime_fix_up_html_address(&from);
    mime_intl_insert_message_header_1(&newBody, &from, HEADER_FROM,
                      MimeGetNamedString(MIME_MHTML_FROM),
                      mailcharset, htmlEdit);
  }
  if (reply_to)
  {
    if (htmlEdit) mime_fix_up_html_address(&reply_to);
    mime_intl_insert_message_header_1(&newBody, &reply_to, HEADER_REPLY_TO,
                      MimeGetNamedString(MIME_MHTML_REPLY_TO),
                      mailcharset, htmlEdit);
  }
  if (organization)
    mime_intl_insert_message_header_1(&newBody, &organization,
                      HEADER_ORGANIZATION,
                      MimeGetNamedString(MIME_MHTML_ORGANIZATION),
                      mailcharset, htmlEdit);
  if (to)
  {
    if (htmlEdit) mime_fix_up_html_address(&to);
    mime_intl_insert_message_header_1(&newBody, &to, HEADER_TO,
                      MimeGetNamedString(MIME_MHTML_TO),
                      mailcharset, htmlEdit);
  }
  if (cc)
  {
    if (htmlEdit) mime_fix_up_html_address(&cc);
    mime_intl_insert_message_header_1(&newBody, &cc, HEADER_CC,
                      MimeGetNamedString(MIME_MHTML_CC),
                      mailcharset, htmlEdit);
  }
    /*
      Do not reveal bcc recipients when forwarding a message!
      See http://bugzilla.mozilla.org/show_bug.cgi?id=41150
    */
    if (newsgroups)
    mime_intl_insert_message_header_1(&newBody, &newsgroups, HEADER_NEWSGROUPS,
                      MimeGetNamedString(MIME_MHTML_NEWSGROUPS),
                      mailcharset, htmlEdit);
  if (followup_to)
  {
    if (htmlEdit) mime_fix_up_html_address(&followup_to);
    mime_intl_insert_message_header_1(&newBody, &followup_to,
                      HEADER_FOLLOWUP_TO,
                      MimeGetNamedString(MIME_MHTML_FOLLOWUP_TO),
                      mailcharset, htmlEdit);
  }
  if (references)
  {
    if (htmlEdit) mime_fix_up_html_address(&references);
    mime_intl_insert_message_header_1(&newBody, &references,
                      HEADER_REFERENCES,
                      MimeGetNamedString(MIME_MHTML_REFERENCES),
                      mailcharset, htmlEdit);
  }
  if (htmlEdit)
  {
    NS_MsgSACat(&newBody, "</TABLE>");
    NS_MsgSACat(&newBody, MSG_LINEBREAK "<BR><BR>");
    if (html_tag)
      NS_MsgSACat(&newBody, html_tag+6);
    else if (*body)
        NS_MsgSACat(&newBody, *body);
  }
  else
  {
    NS_MsgSACat(&newBody, MSG_LINEBREAK MSG_LINEBREAK);
    if (*body)
      NS_MsgSACat(&newBody, *body);
  }
  if (newBody)
  {
    PR_FREEIF(*body);
    *body = newBody;
  }
  PR_FREEIF(subject);
  PR_FREEIF(resent_comments);
  PR_FREEIF(resent_date);
  PR_FREEIF(resent_from);
  PR_FREEIF(resent_to);
  PR_FREEIF(resent_cc);
  PR_FREEIF(date);
  PR_FREEIF(from);
  PR_FREEIF(reply_to);
  PR_FREEIF(organization);
  PR_FREEIF(to);
  PR_FREEIF(cc);
  PR_FREEIF(newsgroups);
  PR_FREEIF(followup_to);
  PR_FREEIF(references);
}

static void 
mime_insert_micro_headers(char            **body,
                          MimeHeaders     *headers,
                          MSG_ComposeFormat composeFormat,
                          char            *mailcharset)
{
  char *newBody = NULL;
  char *subject = MimeHeaders_get(headers, HEADER_SUBJECT, PR_FALSE, PR_FALSE);
  char *from = MimeHeaders_get(headers, HEADER_FROM, PR_FALSE, PR_TRUE);
  char *resent_from = MimeHeaders_get(headers, HEADER_RESENT_FROM, PR_FALSE,
                    PR_TRUE); 
  char *date = MimeHeaders_get(headers, HEADER_DATE, PR_FALSE, PR_TRUE);
  char *to = MimeHeaders_get(headers, HEADER_TO, PR_FALSE, PR_TRUE);
  char *cc = MimeHeaders_get(headers, HEADER_CC, PR_FALSE, PR_TRUE);
  char *newsgroups = MimeHeaders_get(headers, HEADER_NEWSGROUPS, PR_FALSE,
                     PR_TRUE);
  const char *html_tag = nsnull;
  if (*body)
    html_tag = PL_strcasestr(*body, "<HTML>");
  PRBool htmlEdit = composeFormat == nsIMsgCompFormat::HTML;
  
  if (!from)
    from = MimeHeaders_get(headers, HEADER_SENDER, PR_FALSE, PR_TRUE);
  if (!resent_from)
    resent_from = MimeHeaders_get(headers, HEADER_RESENT_SENDER, PR_FALSE,
                    PR_TRUE); 
  if (!date)
    date = MimeHeaders_get(headers, HEADER_RESENT_DATE, PR_FALSE, PR_TRUE);
  
  nsCOMPtr<nsIMsgHeaderParser> parser = do_GetService(NS_MAILNEWS_MIME_HEADER_PARSER_CONTRACTID);
  UnquoteMimeAddress(parser, &resent_from);
  UnquoteMimeAddress(parser, &from);
  UnquoteMimeAddress(parser, &to);
  UnquoteMimeAddress(parser, &cc);

  if (htmlEdit)
  {
    NS_MsgSACopy(&(newBody), "<HTML><BODY><BR><BR>");
    NS_MsgSACat(&newBody, MimeGetNamedString(MIME_FORWARDED_MESSAGE_HTML_USER_WROTE));
    NS_MsgSACat(&newBody, MIME_HEADER_TABLE);
  }
  else
  {
    NS_MsgSACopy(&(newBody), MSG_LINEBREAK MSG_LINEBREAK);
    NS_MsgSACat(&newBody, MimeGetNamedString(MIME_FORWARDED_MESSAGE_HTML_USER_WROTE));
  }

  if (from)
  {
    if (htmlEdit) 
      mime_fix_up_html_address(&from);
    mime_intl_insert_message_header_1(&newBody, &from, HEADER_FROM,
                    MimeGetNamedString(MIME_MHTML_FROM),
                    mailcharset, htmlEdit);
  }
  if (subject)
    mime_intl_insert_message_header_1(&newBody, &subject, HEADER_SUBJECT,
                    MimeGetNamedString(MIME_MHTML_SUBJECT),
                      mailcharset, htmlEdit);
/*
  if (date)
    mime_intl_insert_message_header_1(&newBody, &date, HEADER_DATE,
                    MimeGetNamedString(MIME_MHTML_DATE),
                    mailcharset, htmlEdit);
*/
  if (resent_from)
  {
    if (htmlEdit) mime_fix_up_html_address(&resent_from);
    mime_intl_insert_message_header_1(&newBody, &resent_from,
                    HEADER_RESENT_FROM,
                    MimeGetNamedString(MIME_MHTML_RESENT_FROM),
                    mailcharset, htmlEdit);
  }
  if (to)
  {
    if (htmlEdit) mime_fix_up_html_address(&to);
    mime_intl_insert_message_header_1(&newBody, &to, HEADER_TO,
                    MimeGetNamedString(MIME_MHTML_TO),
                    mailcharset, htmlEdit);
  }
  if (cc)
  {
    if (htmlEdit) mime_fix_up_html_address(&cc);
    mime_intl_insert_message_header_1(&newBody, &cc, HEADER_CC,
                    MimeGetNamedString(MIME_MHTML_CC),
                    mailcharset, htmlEdit);
  }
  /*
    Do not reveal bcc recipients when forwarding a message!
    See http://bugzilla.mozilla.org/show_bug.cgi?id=41150
  */
  if (newsgroups)
    mime_intl_insert_message_header_1(&newBody, &newsgroups, HEADER_NEWSGROUPS,
                    MimeGetNamedString(MIME_MHTML_NEWSGROUPS),
                    mailcharset, htmlEdit);
  if (htmlEdit)
  {
    NS_MsgSACat(&newBody, "</TABLE>");
    NS_MsgSACat(&newBody, MSG_LINEBREAK "<BR><BR>");
    if (html_tag)
      NS_MsgSACat(&newBody, html_tag+6);
    else if (*body)
        NS_MsgSACat(&newBody, *body);
  }
  else
  {
    NS_MsgSACat(&newBody, MSG_LINEBREAK MSG_LINEBREAK);
    if (*body)
      NS_MsgSACat(&newBody, *body);
  }
  if (newBody)
  {
    PR_FREEIF(*body);
    *body = newBody;
  }
  PR_FREEIF(subject);
  PR_FREEIF(from);
  PR_FREEIF(resent_from);
  PR_FREEIF(date);
  PR_FREEIF(to);
  PR_FREEIF(cc);
  PR_FREEIF(newsgroups);

}

// body has to be encoded in UTF-8
static void 
mime_insert_forwarded_message_headers(char            **body, 
                                      MimeHeaders     *headers,
                                      MSG_ComposeFormat composeFormat,
                                      char            *mailcharset)
{
  if (!body || !headers)
    return;

  PRInt32     show_headers = 0;
  nsresult    res;

  nsCOMPtr<nsIPrefBranch> prefBranch(do_GetService(NS_PREFSERVICE_CONTRACTID, &res));
  if (NS_SUCCEEDED(res))
    prefBranch->GetIntPref("mail.show_headers", &show_headers);

  switch (show_headers)
  {
  case 0:
    mime_insert_micro_headers(body, headers, composeFormat, mailcharset);
    break;
  default:
  case 1:
    mime_insert_normal_headers(body, headers, composeFormat, mailcharset);
    break;
  case 2:
    mime_insert_all_headers(body, headers, composeFormat, mailcharset);
    break;
  }
}

static void PR_CALLBACK
mime_parse_stream_complete (nsMIMESession *stream)
{
  struct mime_draft_data *mdd = (struct mime_draft_data *) stream->data_object;
  nsCOMPtr<nsIMsgCompFields> fields;
  int htmlAction = 0;
  int lineWidth = 0;
  
  char *host = 0;
  char *news_host = 0;
  char *to_and_cc = 0;
  char *re_subject = 0;
  char *new_refs = 0;
  char *from = 0;
  char *repl = 0;
  char *subj = 0;
  char *id = 0;
  char *refs = 0;
  char *to = 0;
  char *cc = 0;
  char *bcc = 0; 
  char *fcc = 0; 
  char *org = 0; 
  char *grps = 0;
  char *foll = 0;
  char *priority = 0;
  char *draftInfo = 0;
  char *identityKey = 0;
  
  PRBool xlate_p = PR_FALSE;  /* #### how do we determine this? */
  PRBool sign_p = PR_FALSE;   /* #### how do we determine this? */
  PRBool forward_inline = PR_FALSE;
  PRBool bodyAsAttachment = PR_FALSE;
  PRBool charsetOverride = PR_FALSE;
  
  NS_ASSERTION (mdd, "null mime draft data");
  
  if (!mdd) return;
  
  if (mdd->obj) 
  {
    int status;
    
    status = mdd->obj->clazz->parse_eof ( mdd->obj, PR_FALSE );
    mdd->obj->clazz->parse_end( mdd->obj, status < 0 ? PR_TRUE : PR_FALSE );
    
    xlate_p = mdd->options->decrypt_p;
    sign_p = mdd->options->signed_p;
    
    // RICHIE
    // We need to figure out how to pass the forwarded flag along with this
    // operation.
    
    //forward_inline = (mdd->format_out != FO_CMDLINE_ATTACHMENTS);    
    forward_inline = mdd->forwardInline;
    if (forward_inline)
    {
#ifdef MOZ_SECURITY
      HG88890
#endif
    }
    
    NS_ASSERTION ( mdd->options == mdd->obj->options, "mime draft options not same as obj->options" );
    mime_free (mdd->obj);
    mdd->obj = 0;
    if (mdd->options) 
    {
      // save the override flag before it's unavailable
      charsetOverride = mdd->options->override_charset;
      if ((!mdd->mailcharset || charsetOverride) && mdd->options->default_charset) 
      {
        PR_Free(mdd->mailcharset);
        mdd->mailcharset = nsCRT::strdup(mdd->options->default_charset);
      }

      // mscott: aren't we leaking a bunch of strings here like the charset strings and such?
      delete mdd->options;
      mdd->options = 0;
    }
    if (mdd->stream) 
    {
      mdd->stream->complete ((nsMIMESession *)mdd->stream->data_object);
      PR_Free( mdd->stream );
      mdd->stream = 0;
    }
  }
  
  //
  // Now, process the attachments that we have gathered from the message
  // on disk
  //
  nsMsgAttachmentData *newAttachData = mime_draft_process_attachments(mdd);
  
  //
  // time to bring up the compose windows with all the info gathered 
  //
  if ( mdd->headers )
  {
    subj = MimeHeaders_get(mdd->headers, HEADER_SUBJECT,  PR_FALSE, PR_FALSE);
    if (forward_inline)
    {
      if (subj)
      {
        char *newSubj = PR_smprintf("[Fwd: %s]", subj);
        if (newSubj)
        {
          PR_Free(subj);
          subj = newSubj;
        }
      }
    } 
    else
    {
      repl = MimeHeaders_get(mdd->headers, HEADER_REPLY_TO, PR_FALSE, PR_FALSE);
      to   = MimeHeaders_get(mdd->headers, HEADER_TO,       PR_FALSE, PR_TRUE);
      cc   = MimeHeaders_get(mdd->headers, HEADER_CC,       PR_FALSE, PR_TRUE);
      bcc   = MimeHeaders_get(mdd->headers, HEADER_BCC,       PR_FALSE, PR_TRUE);
      
      /* These headers should not be RFC-1522-decoded. */
      grps = MimeHeaders_get(mdd->headers, HEADER_NEWSGROUPS,  PR_FALSE, PR_TRUE);
      foll = MimeHeaders_get(mdd->headers, HEADER_FOLLOWUP_TO, PR_FALSE, PR_TRUE);
      
      host = MimeHeaders_get(mdd->headers, HEADER_X_MOZILLA_NEWSHOST, PR_FALSE, PR_FALSE);
      if (!host)
        host = MimeHeaders_get(mdd->headers, HEADER_NNTP_POSTING_HOST, PR_FALSE, PR_FALSE);
      
      id   = MimeHeaders_get(mdd->headers, HEADER_MESSAGE_ID,  PR_FALSE, PR_FALSE);
      refs = MimeHeaders_get(mdd->headers, HEADER_REFERENCES,  PR_FALSE, PR_TRUE);
      priority = MimeHeaders_get(mdd->headers, HEADER_X_PRIORITY, PR_FALSE, PR_FALSE);
      
      
      if (host) 
      {
        char *secure = NULL;
        
        secure = PL_strcasestr(host, "secure");
        if (secure) 
        {
          *secure = 0;
          news_host = PR_smprintf ("snews://%s", host);
        }
        else 
        {
          news_host = PR_smprintf ("news://%s", host);
        }
      }
    }
    
    
    CreateCompositionFields( from, repl, to, cc, bcc, fcc, grps, foll,
      org, subj, refs, 0, priority, news_host,
      xlate_p, sign_p, mdd->mailcharset,
      getter_AddRefs(fields));
    
    draftInfo = MimeHeaders_get(mdd->headers, HEADER_X_MOZILLA_DRAFT_INFO, PR_FALSE, PR_FALSE);
    if (draftInfo && fields && !forward_inline) 
    {
      char *parm = 0;
      parm = MimeHeaders_get_parameter(draftInfo, "vcard", NULL, NULL);
      fields->SetAttachVCard(parm && !nsCRT::strcmp(parm, "1"));
   
      fields->SetMessageId(id); // keep same message id for editing template.
      PR_FREEIF(parm);
      parm = MimeHeaders_get_parameter(draftInfo, "receipt", NULL, NULL);
      if (parm && !nsCRT::strcmp(parm, "0"))
        fields->SetReturnReceipt(PR_FALSE); 
      else
      {
        int receiptType = 0;
        fields->SetReturnReceipt(PR_TRUE); 
        sscanf(parm, "%d", &receiptType);
        // slight change compared to 4.x; we used to use receipt= to tell
        // whether the draft/template has request for either MDN or DNS or both
        // return receipt; since the DNS is out of the picture we now use the
        // header type - 1 to tell whether user has requested the return receipt
        fields->SetReceiptHeaderType(((PRInt32)receiptType) - 1);
      }
      PR_FREEIF(parm);
      parm = MimeHeaders_get_parameter(draftInfo, "uuencode", NULL, NULL);
      fields->SetUuEncodeAttachments(parm && !nsCRT::strcmp(parm, "1"));
      PR_FREEIF(parm);
      parm = MimeHeaders_get_parameter(draftInfo, "html", NULL, NULL);
      if (parm)
        sscanf(parm, "%d", &htmlAction);
      PR_FREEIF(parm);
      parm = MimeHeaders_get_parameter(draftInfo, "linewidth", NULL, NULL);
      if (parm)
        sscanf(parm, "%d", &lineWidth);
      PR_FREEIF(parm);
      
    }
    
	// identity to prefer when opening the message in the compose window?
    identityKey = MimeHeaders_get(mdd->headers, HEADER_X_MOZILLA_IDENTITY_KEY, PR_FALSE, PR_FALSE);
    if ( identityKey && *identityKey )
    {
        nsresult rv = NS_OK;
        nsCOMPtr< nsIMsgAccountManager > accountManager = 
                do_GetService( NS_MSGACCOUNTMANAGER_CONTRACTID, &rv );
        if ( NS_SUCCEEDED(rv) && accountManager )
        {
            nsCOMPtr< nsIMsgIdentity > overrulingIdentity;
            rv = accountManager->GetIdentity( identityKey, getter_AddRefs( overrulingIdentity ) );

            if ( NS_SUCCEEDED(rv) && overrulingIdentity )
                mdd->identity = overrulingIdentity;
        }
    }

    if (mdd->messageBody) 
    {
      MSG_ComposeFormat composeFormat = nsIMsgCompFormat::Default;
      if (mdd->messageBody->type && *mdd->messageBody->type)
      {
        if( PL_strcasestr(mdd->messageBody->type, "text/html"))
          composeFormat = nsIMsgCompFormat::HTML;
        else if ( PL_strcasestr(mdd->messageBody->type, "text/plain") ||
                  !PL_strcasecmp(mdd->messageBody->type, "text") )
          composeFormat = nsIMsgCompFormat::PlainText;
        else
        {
          //We cannot use this kind of data for the message body! Therefore, move it as attachment
          bodyAsAttachment = PR_TRUE;
        }
      }
      else
        composeFormat = nsIMsgCompFormat::PlainText;

      char *body = nsnull;
      PRUint32 bodyLen = 0;
      
      if (!bodyAsAttachment)
      {
        bodyLen = mdd->messageBody->file_spec->GetFileSize();
        body = (char *)PR_MALLOC (bodyLen + 1);
        if (body)
        {
          memset (body, 0, bodyLen+1);
        
          nsInputFileStream inputFile(*(mdd->messageBody->file_spec));
          if (inputFile.is_open())
            inputFile.read(body, bodyLen);
        
          inputFile.close();

          // Convert the body to UTF-8
          char *mimeCharset = nsnull;
          // Get a charset from the header if no override is set.
          if (!charsetOverride)
            mimeCharset = MimeHeaders_get_parameter (mdd->messageBody->type, "charset", nsnull, nsnull);
          // If no charset is specified in the header then use the default.
          char *bodyCharset = mimeCharset ? mimeCharset : mdd->mailcharset;
          if (bodyCharset)
          {
            nsAutoString tempUnicodeString;
            if (NS_SUCCEEDED(ConvertToUnicode(bodyCharset, body, tempUnicodeString)))
            {
              char *newBody = ToNewUTF8String(tempUnicodeString);
              if (newBody) 
              {
                PR_Free(body);
                body = newBody;
                bodyLen = strlen(newBody);
              }
            }
          }
          PR_FREEIF(mimeCharset);
        }
      }

      // Since we have body text, then we should set the compose fields with
      // this data.      
      //       if (composeFormat == nsIMsgCompFormat::PlainText)
      //         fields->SetTheForcePlainText(PR_TRUE);
      
      if (forward_inline)
      {
        if (mdd->identity)
        {
          PRBool bFormat;
          mdd->identity->GetComposeHtml(&bFormat);
          if (bFormat)
          {
            if (body && composeFormat == nsIMsgCompFormat::PlainText)
            {
              //We need to convert the plain/text to HTML in order to escape any HTML markup
              char *escapedBody = nsEscapeHTML(body);
              if (escapedBody)
              {
                PR_Free(body);
                body = escapedBody;
                bodyLen = strlen(body);
              }

              char* newbody = (char *)PR_MALLOC (bodyLen + 12); //+11 chars for <pre> & </pre> tags
              if (newbody)
              {
                *newbody = 0;
                PL_strcat(newbody, "<PRE>");
                PL_strcat(newbody, body);
                PL_strcat(newbody, "</PRE>");
                PR_Free(body);
                body = newbody;
              }
            }
            composeFormat = nsIMsgCompFormat::HTML;
          }
        }
        
        mime_insert_forwarded_message_headers(&body, mdd->headers, composeFormat,
          mdd->mailcharset);
      }
      // setting the charset while we are creating the composition fields
      //fields->SetCharacterSet(NS_ConvertASCIItoUTF16(mdd->mailcharset));

      // convert from UTF-8 to UTF-16
      if (body)
      {
        fields->SetBody(NS_ConvertUTF8toUTF16(body));
        PR_Free(body);
      }

      //
      // At this point, we need to create a message compose window or editor
      // window via XP-COM with the information that we have retrieved from 
      // the message store.
      //
      if (mdd->format_out == nsMimeOutput::nsMimeMessageEditorTemplate)
      {
#ifdef NS_DEBUG
        printf("RICHIE: Time to create the EDITOR with this template - HAS a body!!!!\n");
#endif
        fields->SetDraftId(mdd->url_name);
        CreateTheComposeWindow(fields, newAttachData, nsIMsgCompType::Template, composeFormat, mdd->identity, nsnull);
      }
      else
      {
#ifdef NS_DEBUG
        printf("Time to create the composition window WITH a body!!!!\n");
#endif
        if (mdd->forwardInline)
          CreateTheComposeWindow(fields, newAttachData, nsIMsgCompType::ForwardInline, composeFormat, mdd->identity, mdd->originalMsgURI);
          else
        {
          fields->SetDraftId(mdd->url_name);
          CreateTheComposeWindow(fields, newAttachData, nsIMsgCompType::Draft, composeFormat, mdd->identity, nsnull);
        }
      }
    }
    else
    {
      //
      // At this point, we need to create a message compose window via
      // XP-COM with the information that we have retrieved from the message store.
      //
      if (mdd->format_out == nsMimeOutput::nsMimeMessageEditorTemplate)
      {
#ifdef NS_DEBUG
        printf("RICHIE: Time to create the EDITOR with this template - NO body!!!!\n");
#endif
        CreateTheComposeWindow(fields, newAttachData, nsIMsgCompType::Template, nsIMsgCompFormat::Default, mdd->identity, nsnull);
      }
      else
      {
#ifdef NS_DEBUG
        printf("Time to create the composition window WITHOUT a body!!!!\n");
#endif
        if (mdd->forwardInline)
          CreateTheComposeWindow(fields, newAttachData, nsIMsgCompType::ForwardInline, nsIMsgCompFormat::Default, mdd->identity, mdd->originalMsgURI);
        else
        {
          fields->SetDraftId(mdd->url_name);
          CreateTheComposeWindow(fields, newAttachData, nsIMsgCompType::Draft, nsIMsgCompFormat::Default, mdd->identity, nsnull);
        }
      }
    }    
  }
  else
  {
    CreateCompositionFields( from, repl, to, cc, bcc, fcc, grps, foll,
      org, subj, refs, 0, priority, news_host,
      GetMailXlateionPreference(), 
      GetMailSigningPreference(),
      mdd->mailcharset,
      getter_AddRefs(fields));
    if (fields)
      CreateTheComposeWindow(fields, newAttachData, nsIMsgCompType::New, nsIMsgCompFormat::Default, mdd->identity, nsnull);
  }
  
  if ( mdd->headers )
    MimeHeaders_free ( mdd->headers );
  
  // 
  // Free the original attachment structure...
  // Make sure we only cleanup the local copy of the memory and not kill 
  // files we need on disk
  //
  if (bodyAsAttachment)
  {
    if ( mdd->messageBody->file_spec ) 
    {
      delete ( mdd->messageBody->file_spec );
      mdd->messageBody->file_spec = nsnull;
    }
  }
  mime_free_attachments (mdd->messageBody, 1);

  if (mdd->attachments)
  {
    int               i;
    nsMsgAttachedFile *cur = mdd->attachments;
    
    for ( i = 0; i < mdd->attachments_count; i++, cur++ ) 
    {
      if ( cur->file_spec ) 
      {
        delete ( cur->file_spec );
        cur->file_spec = nsnull;
      }
    }
    
    mime_free_attachments( mdd->attachments, mdd->attachments_count );
  }
  PR_FREEIF(mdd->mailcharset);
  
  mdd->identity = nsnull;
  PR_Free(mdd->url_name);
  PR_Free(mdd->originalMsgURI);
  PR_Free(mdd);
  
  PR_FREEIF(host);
  PR_FREEIF(to_and_cc);
  PR_FREEIF(re_subject);
  PR_FREEIF(new_refs);
  PR_FREEIF(from);
  PR_FREEIF(repl);
  PR_FREEIF(subj);
  PR_FREEIF(id);
  PR_FREEIF(refs);
  PR_FREEIF(to);
  PR_FREEIF(cc);
  PR_FREEIF(grps);
  PR_FREEIF(foll);
  PR_FREEIF(priority);
  PR_FREEIF(draftInfo);
  PR_Free(identityKey);

  mime_free_attach_data(newAttachData);
}

static void PR_CALLBACK
mime_parse_stream_abort (nsMIMESession *stream, int status )
{
  struct mime_draft_data *mdd = (struct mime_draft_data *) stream->data_object;  
  NS_ASSERTION (mdd, "null mime draft data");

  if (!mdd) 
    return;
  
  if (mdd->obj) 
  {
    int status=0;

    if ( !mdd->obj->closed_p )
      status = mdd->obj->clazz->parse_eof ( mdd->obj, PR_TRUE );
    if ( !mdd->obj->parsed_p )
      mdd->obj->clazz->parse_end( mdd->obj, PR_TRUE );
    
    NS_ASSERTION ( mdd->options == mdd->obj->options, "draft display options not same as mime obj" );
    mime_free (mdd->obj);
    mdd->obj = 0;
    if (mdd->options) 
    {
      delete mdd->options;
      mdd->options = 0;
    }

    if (mdd->stream) 
    {
      mdd->stream->abort ((nsMIMESession *)mdd->stream->data_object, status);
      PR_Free( mdd->stream );
      mdd->stream = 0;
    }
  }

  if ( mdd->headers )
    MimeHeaders_free (mdd->headers);


  if (mdd->attachments)
    mime_free_attachments( mdd->attachments, mdd->attachments_count );

  PR_FREEIF(mdd->mailcharset);

  PR_Free (mdd);
}

static int
make_mime_headers_copy ( void *closure, MimeHeaders *headers )
{
  struct mime_draft_data *mdd = (struct mime_draft_data *) closure;

  NS_ASSERTION ( mdd && headers, "null mime draft data and/or headers" );
  
  if ( !mdd || ! headers ) 
    return 0;

  NS_ASSERTION ( mdd->headers == NULL , "non null mime draft data headers");

  mdd->headers = MimeHeaders_copy ( headers );
  mdd->options->done_parsing_outer_headers = PR_TRUE;

  return 0;
}

nsresult 
mime_decompose_file_init_fn ( void *stream_closure, MimeHeaders *headers )
{
  struct mime_draft_data *mdd = (struct mime_draft_data *) stream_closure;
  nsMsgAttachedFile *attachments = 0, *newAttachment = 0;
  int nAttachments = 0;
  //char *hdr_value = NULL;
  char *parm_value = NULL;
  PRBool needURL = PR_FALSE;
  PRBool creatingMsgBody = PR_TRUE;
  PRBool bodyPart = PR_FALSE;
  
  NS_ASSERTION (mdd && headers, "null mime draft data and/or headers");
  if (!mdd || !headers) 
    return -1;

  if (mdd->options->decompose_init_count) 
  {
    mdd->options->decompose_init_count++;
    NS_ASSERTION(mdd->curAttachment, "missing attachment in mime_decompose_file_init_fn");
    if (mdd->curAttachment) {
      char *ct = MimeHeaders_get(headers, HEADER_CONTENT_TYPE, PR_TRUE, PR_FALSE);
      if (ct)
        NS_MsgSACopy(&(mdd->curAttachment->type), ct);
      PR_FREEIF(ct);
    }
    return 0;
  }
  else
    mdd->options->decompose_init_count++;

  nAttachments = mdd->attachments_count;

  if (!nAttachments && !mdd->messageBody) 
  {
    // if we've been told to use an override charset then do so....otherwise use the charset
    // inside the message header...
    if (mdd->options && mdd->options->override_charset)
        mdd->mailcharset = nsCRT::strdup(mdd->options->default_charset);
    else
    {
      char *contentType = NULL;
      contentType = MimeHeaders_get(headers, HEADER_CONTENT_TYPE, PR_FALSE, PR_FALSE);
      if (contentType) 
      {
        mdd->mailcharset = MimeHeaders_get_parameter(contentType, "charset", NULL, NULL);
        PR_FREEIF(contentType);
      }
    }
    
    mdd->messageBody = PR_NEWZAP (nsMsgAttachedFile);
    NS_ASSERTION (mdd->messageBody, "missing messageBody in mime_decompose_file_init_fn");
    if (!mdd->messageBody) 
      return MIME_OUT_OF_MEMORY;
    newAttachment = mdd->messageBody;
    creatingMsgBody = PR_TRUE;
    bodyPart = PR_TRUE;
  }
  else 
  {
    /* always allocate one more extra; don't ask me why */
    needURL = PR_TRUE;
    if ( nAttachments ) 
    {
      NS_ASSERTION (mdd->attachments, "no attachments");
      
      attachments = (nsMsgAttachedFile *)PR_REALLOC(mdd->attachments, 
                                                    sizeof (nsMsgAttachedFile) * 
                                                    (nAttachments + 2));
    if (!attachments)
        return MIME_OUT_OF_MEMORY;
      mdd->attachments = attachments;
      mdd->attachments_count++;
    }
    else {
      NS_ASSERTION (!mdd->attachments, "have attachments but count is 0");
      
      attachments = (nsMsgAttachedFile *) PR_MALLOC ( sizeof (nsMsgAttachedFile) * 2);
      if (!attachments) 
        return MIME_OUT_OF_MEMORY;
      mdd->attachments_count++;
      mdd->attachments = attachments;
    }
    
    newAttachment = attachments + nAttachments;
    memset ( newAttachment, 0, sizeof (nsMsgAttachedFile) * 2 );
  }

  char *workURLSpec = nsnull;
  char *contLoc = nsnull;

  newAttachment->real_name = MimeHeaders_get_name ( headers, mdd->options );
  contLoc = MimeHeaders_get( headers, HEADER_CONTENT_LOCATION, PR_FALSE, PR_FALSE );
  if (!contLoc)
      contLoc = MimeHeaders_get( headers, HEADER_CONTENT_BASE, PR_FALSE, PR_FALSE );

  if ( (!contLoc) && (newAttachment->real_name) )
    workURLSpec = nsCRT::strdup(newAttachment->real_name);
  if ( (contLoc) && (!workURLSpec) )
    workURLSpec = nsCRT::strdup(contLoc);

  PR_FREEIF(contLoc);

  mdd->curAttachment = newAttachment;  
  newAttachment->type =  MimeHeaders_get ( headers, HEADER_CONTENT_TYPE, PR_FALSE, PR_FALSE );

  //
  // This is to handle the degenerated Apple Double attachment.
  //
  parm_value = MimeHeaders_get( headers, HEADER_CONTENT_TYPE, PR_FALSE, PR_FALSE );
  if (parm_value) 
  {
    char *boundary = NULL;
    char *tmp_value = NULL;
    boundary = MimeHeaders_get_parameter(parm_value, "boundary", NULL, NULL);
    if (boundary)
      tmp_value = PR_smprintf("; boundary=\"%s\"", boundary);
    if (tmp_value)
      NS_MsgSACat(&(newAttachment->type), tmp_value);
    newAttachment->x_mac_type = MimeHeaders_get_parameter(parm_value, "x-mac-type", NULL, NULL);
    newAttachment->x_mac_creator = MimeHeaders_get_parameter(parm_value, "x-mac-creator", NULL, NULL);
    PR_FREEIF(parm_value);
    PR_FREEIF(boundary);
    PR_FREEIF(tmp_value);
  }
  newAttachment->encoding = MimeHeaders_get ( headers, HEADER_CONTENT_TRANSFER_ENCODING,
                                              PR_FALSE, PR_FALSE );
  newAttachment->description = MimeHeaders_get( headers, HEADER_CONTENT_DESCRIPTION,
                                                PR_FALSE, PR_FALSE );
  // 
  // If we came up empty for description or the orig URL, we should do something about it.
  //
  if  ( ( (!newAttachment->description) || (!*newAttachment->description) ) && (workURLSpec) )
    newAttachment->description = nsCRT::strdup(workURLSpec);

  nsFileSpec *tmpSpec = nsnull;
  {
    // Let's build a temp file with an extension based on the content-type: nsmail.<extension>

    nsCAutoString  newAttachName ("nsmail");
    PRBool extensionAdded = PR_FALSE;
    // the content type may contain a charset. i.e. text/html; ISO-2022-JP...we want to strip off the charset
    // before we ask the mime service for a mime info for this content type.
    nsCAutoString contentType (newAttachment->type);
    PRInt32 pos = contentType.FindChar(';');
    if (pos > 0)
      contentType.Truncate(pos);
    nsresult  rv = NS_OK;
    nsCOMPtr<nsIMIMEService> mimeFinder (do_GetService(NS_MIMESERVICE_CONTRACTID, &rv));
    if (NS_SUCCEEDED(rv) && mimeFinder) 
    {
      nsCAutoString fileExtension;
      rv = mimeFinder->GetPrimaryExtension(contentType, EmptyCString(), fileExtension);

      if (NS_SUCCEEDED(rv) && !fileExtension.IsEmpty())
      {
        newAttachName.Append(".");
        newAttachName.Append(fileExtension);
        extensionAdded = PR_TRUE;
      }
    }

    if (!extensionAdded)
    {
      newAttachName.Append(".tmp");
    }

    tmpSpec = nsMsgCreateTempFileSpec(newAttachName.get());
  }

  // This needs to be done so the attachment structure has a handle 
  // on the temp file for this attachment...
  // if ( (tmpSpec) && (!bodyPart) )
  if (tmpSpec)
  {
      nsFileURL fileURL(*tmpSpec);
      const char * tempSpecStr = fileURL.GetURLString();

      nsMimeNewURI(getter_AddRefs(newAttachment->orig_url), tempSpecStr, nsnull);
  }

  PR_FREEIF(workURLSpec);
  if (!tmpSpec)
    return MIME_OUT_OF_MEMORY;

  NS_NewFileSpecWithSpec(*tmpSpec, &mdd->tmpFileSpec);
  if (!mdd->tmpFileSpec)
    return MIME_OUT_OF_MEMORY;
  
  newAttachment->file_spec = tmpSpec;

  mdd->tmpFileStream = new nsOutputFileStream(*tmpSpec, PR_WRONLY | PR_CREATE_FILE, 00600);
  if (!mdd->tmpFileStream)
    return MIME_UNABLE_TO_OPEN_TMP_FILE;
    
  // For now, we are always going to decode all of the attachments 
  // for the message. This way, we have native data 
  if (creatingMsgBody) 
  {
    MimeDecoderData *(*fn) (nsresult (*) (const char*, PRInt32, void*), void*) = 0;
    
    //
    // Initialize a decoder if necessary.
    //
    if (!newAttachment->encoding)
      ;
    else if (!nsCRT::strcasecmp(newAttachment->encoding, ENCODING_BASE64))
      fn = &MimeB64DecoderInit;
    else if (!nsCRT::strcasecmp(newAttachment->encoding, ENCODING_QUOTED_PRINTABLE))
    {
      mdd->decoder_data = MimeQPDecoderInit (/* The (nsresult (*) ...) cast is to turn the `void' argument into `MimeObject'. */
                              ((nsresult (*) (const char *, PRInt32, void *))
                              dummy_file_write), mdd->tmpFileStream);
      if (!mdd->decoder_data)
        return MIME_OUT_OF_MEMORY;
    }
    else if (!nsCRT::strcasecmp(newAttachment->encoding, ENCODING_UUENCODE) ||
             !nsCRT::strcasecmp(newAttachment->encoding, ENCODING_UUENCODE2) ||
             !nsCRT::strcasecmp(newAttachment->encoding, ENCODING_UUENCODE3) ||
             !nsCRT::strcasecmp(newAttachment->encoding, ENCODING_UUENCODE4))
      fn = &MimeUUDecoderInit;
    else if (!nsCRT::strcasecmp(newAttachment->encoding, ENCODING_YENCODE))
      fn = &MimeYDecoderInit;
    
    if (fn) 
    {
      mdd->decoder_data = fn (/* The (nsresult (*) ...) cast is to turn the `void' argument into `MimeObject'. */
                              ((nsresult (*) (const char *, PRInt32, void *))
                              dummy_file_write), mdd->tmpFileStream);
      if (!mdd->decoder_data)
        return MIME_OUT_OF_MEMORY;
    }
  }

  return 0;
}

nsresult 
mime_decompose_file_output_fn (const char     *buf,
                               PRInt32  size,
                               void     *stream_closure )
{
  struct mime_draft_data *mdd = (struct mime_draft_data *) stream_closure;
  int ret = 0;
  
  NS_ASSERTION (mdd && buf, "missing mime draft data and/or buf");
  if (!mdd || !buf) return -1;
  if (!size) return NS_OK;
  
  if ( !mdd->tmpFileStream ) 
    return NS_OK;
  
  if (mdd->decoder_data) {
    ret = MimeDecoderWrite(mdd->decoder_data, buf, size);
    if (ret == -1) return -1;
  }
  else 
  {
    ret = mdd->tmpFileStream->write(buf, size);
    if (ret < size)
      return MIME_ERROR_WRITING_FILE;
  }
  
  return NS_OK;
}

nsresult
mime_decompose_file_close_fn ( void *stream_closure )
{
  struct mime_draft_data *mdd = (struct mime_draft_data *) stream_closure;
  
  if ( !mdd || !mdd->tmpFileStream )
    return -1;
  
  if ( --mdd->options->decompose_init_count > 0 )
      return 0;
  
  if (mdd->decoder_data) {
    MimeDecoderDestroy(mdd->decoder_data, PR_FALSE);
    mdd->decoder_data = 0;
  }

  if (mdd->tmpFileStream->GetIStream())
    mdd->tmpFileStream->close();

  delete mdd->tmpFileStream;
  mdd->tmpFileStream = nsnull;

  delete mdd->tmpFileSpec;
  mdd->tmpFileSpec = nsnull;
  
  return 0;
}

extern "C" void  *
mime_bridge_create_draft_stream(
                          nsIMimeEmitter      *newEmitter,
                          nsStreamConverter   *newPluginObj2,
                          nsIURI              *uri,
                          nsMimeOutputType    format_out)
{
  int                     status = 0;
  nsMIMESession           *stream = nsnull;
  struct mime_draft_data  *mdd = nsnull;
  MimeObject              *obj = nsnull;

  if ( !uri ) 
    return nsnull;

  mdd = PR_NEWZAP(struct mime_draft_data);
  if (!mdd) 
    return nsnull;

  nsCAutoString turl;
  nsCOMPtr <nsIMsgMessageService> msgService;
  nsCOMPtr<nsIURI> aURL;
  nsCAutoString urlString;
  nsresult rv;

  // first, convert the rdf msg uri into a url that represents the message...
  if (NS_FAILED(uri->GetSpec(turl)))
    goto FAIL;

  rv = GetMessageServiceFromURI(turl.get(), getter_AddRefs(msgService));
  if (NS_FAILED(rv)) 
    goto FAIL;

  rv = msgService->GetUrlForUri(turl.get(), getter_AddRefs(aURL), nsnull);
  if (NS_FAILED(rv)) 
    goto FAIL;

  if (NS_SUCCEEDED(aURL->GetSpec(urlString)))
  {
    PRInt32 typeIndex = urlString.Find("&type=application/x-message-display");
    if (typeIndex != kNotFound)
      urlString.Cut(typeIndex, sizeof("&type=application/x-message-display") - 1);

    mdd->url_name = ToNewCString(urlString);
    if (!(mdd->url_name))
      goto FAIL;
  }

  newPluginObj2->GetForwardInline(&mdd->forwardInline);
  newPluginObj2->GetIdentity(getter_AddRefs(mdd->identity));
  newPluginObj2->GetOriginalMsgURI(&mdd->originalMsgURI);
  mdd->format_out = format_out;
  mdd->options = new  MimeDisplayOptions ;
  if (!mdd->options)
    goto FAIL;

  mdd->options->url = nsCRT::strdup(mdd->url_name);
  mdd->options->format_out = format_out;     // output format
  mdd->options->decompose_file_p = PR_TRUE; /* new field in MimeDisplayOptions */
  mdd->options->stream_closure = mdd;
  mdd->options->html_closure = mdd;
  mdd->options->decompose_headers_info_fn = make_mime_headers_copy;
  mdd->options->decompose_file_init_fn = mime_decompose_file_init_fn;
  mdd->options->decompose_file_output_fn = mime_decompose_file_output_fn;
  mdd->options->decompose_file_close_fn = mime_decompose_file_close_fn;

  mdd->options->m_prefBranch = do_GetService(NS_PREFSERVICE_CONTRACTID, &rv);
  if (NS_FAILED(rv))
    goto FAIL;

#ifdef ENABLE_SMIME
  /* If we're attaching a message (for forwarding) then we must eradicate all
   traces of xlateion from it, since forwarding someone else a message
   that wasn't xlated for them doesn't work.  We have to dexlate it
   before sending it.
   */
  mdd->options->decrypt_p = PR_TRUE;
#endif /* ENABLE_SMIME */

  obj = mime_new ( (MimeObjectClass *) &mimeMessageClass, (MimeHeaders *) NULL, MESSAGE_RFC822 );
  if ( !obj ) 
    goto FAIL;
  
  obj->options = mdd->options;
  mdd->obj = obj;

  stream = PR_NEWZAP ( nsMIMESession );
  if ( !stream ) 
    goto FAIL;

  stream->name = "MIME To Draft Converter Stream";
  stream->complete = mime_parse_stream_complete;
  stream->abort = mime_parse_stream_abort;
  stream->put_block = mime_parse_stream_write;
  stream->data_object = mdd;

  status = obj->clazz->initialize ( obj );
  if ( status >= 0 )
    status = obj->clazz->parse_begin ( obj );
  if ( status < 0 )
    goto FAIL;

  return stream;

FAIL:
  if (mdd)
  {
    PR_Free(mdd->url_name);
    PR_Free(mdd->originalMsgURI);
    if (mdd->options)
      delete mdd->options;
    PR_Free ( mdd );
  }
  PR_Free ( stream );
  PR_Free ( obj );

  return nsnull;
}
