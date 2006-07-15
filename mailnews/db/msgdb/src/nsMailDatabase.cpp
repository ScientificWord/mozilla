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
 * Portions created by the Initial Developer are Copyright (C) 1999
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

#include "msgCore.h"
#include "nsImapCore.h" // for imap flags
#include "nsMailDatabase.h"
#include "nsDBFolderInfo.h"
#include "nsMsgLocalFolderHdrs.h"
#include "nsFileStream.h"
#include "nsFileSpec.h"
#include "nsMsgOfflineImapOperation.h"
#include "nsMsgFolderFlags.h"
#include "prlog.h"
#include "prprf.h"
#include "nsIFileSpec.h"
#include "nsMsgUtils.h"
#ifdef PUTUP_ALERT_ON_INVALID_DB
#include "nsIPrompt.h"
#include "nsIWindowWatcher.h"
#endif

extern PRLogModuleInfo *IMAPOffline;

const char *kOfflineOpsScope = "ns:msg:db:row:scope:ops:all";	// scope for all offine ops table
const char *kOfflineOpsTableKind = "ns:msg:db:table:kind:ops";
struct mdbOid gAllOfflineOpsTableOID;


nsMailDatabase::nsMailDatabase()
    : m_reparse(PR_FALSE), m_folderSpec(nsnull), m_folderStream(nsnull), m_ownFolderStream(PR_FALSE)
{
  m_mdbAllOfflineOpsTable = nsnull;
}

nsMailDatabase::~nsMailDatabase()
{
  delete m_folderSpec;
}

NS_IMETHODIMP nsMailDatabase::SetFolderStream(nsIOFileStream *aFileStream)
{
  NS_ASSERTION(!m_folderStream || !aFileStream, "m_folderStream is not null and we are assigning a non null stream to it");
  m_folderStream = aFileStream; //m_folderStream is set externally, so m_ownFolderStream is false
  m_ownFolderStream = PR_FALSE;
  return NS_OK;
}

NS_IMETHODIMP nsMailDatabase::GetFolderStream(nsIOFileStream **aFileStream)
{
  NS_ENSURE_ARG_POINTER(aFileStream);
  if (!m_folderStream)
  {
    m_folderStream = new nsIOFileStream(nsFileSpec(*m_folderSpec));
    m_ownFolderStream = PR_TRUE;
  }
  // N.B. - not a ref-counted interface pointer
  *aFileStream = m_folderStream;
  return NS_OK;
}

static PRBool gGotGlobalPrefs = PR_FALSE;
static PRInt32 gTimeStampLeeway;

void nsMailDatabase::GetGlobalPrefs()
{
  if (!gGotGlobalPrefs)
  {
    nsMsgDatabase::GetGlobalPrefs();
    GetIntPref("mail.db_timestamp_leeway", &gTimeStampLeeway);
    gGotGlobalPrefs = PR_TRUE;
  }
}
// caller passes in upgrading==PR_TRUE if they want back a db even if the db is out of date.
// If so, they'll extract out the interesting info from the db, close it, delete it, and
// then try to open the db again, prior to reparsing.
NS_IMETHODIMP nsMailDatabase::Open(nsIFileSpec *aFolderName, PRBool aCreate, PRBool aUpgrading)
{
  nsFileSpec  folderName;
  aFolderName->GetFileSpec(&folderName);
  m_folderSpec = new nsFileSpec(folderName);
  nsresult rv = nsMsgDatabase::Open(aFolderName, aCreate, aUpgrading);
  return rv;
}

NS_IMETHODIMP nsMailDatabase::ForceClosed()
{
  m_mdbAllOfflineOpsTable = nsnull;
  return nsMsgDatabase::ForceClosed();
}

// get this on demand so that only db's that have offline ops will
// create the table.	
nsresult nsMailDatabase::GetAllOfflineOpsTable()
{
  nsresult rv = NS_OK;
  if (!m_mdbAllOfflineOpsTable)
    rv = GetTableCreateIfMissing(kOfflineOpsScope, kOfflineOpsTableKind, getter_AddRefs(m_mdbAllOfflineOpsTable), 
                                                m_offlineOpsRowScopeToken, m_offlineOpsTableKindToken) ;
  return rv;
}

// cache m_folderStream to make updating mozilla status flags fast
NS_IMETHODIMP nsMailDatabase::StartBatch()
{
  if (!m_folderStream && m_folder)  //only if we create a stream, set m_ownFolderStream to true.
  {
    PRBool isLocked;
    m_folder->GetLocked(&isLocked);
    if (isLocked)
    {
      NS_ASSERTION(PR_FALSE, "Some other operation is in progress");
      return NS_MSG_FOLDER_BUSY;
    }
    m_folderStream = new nsIOFileStream(nsFileSpec(*m_folderSpec));
    m_ownFolderStream = PR_TRUE;
  }

  return NS_OK;
}

NS_IMETHODIMP nsMailDatabase::EndBatch()
{
  if (m_ownFolderStream)   //only if we own the stream, then we should close it
  {
    if (m_folderStream)
    {
      m_folderStream->flush();
      m_folderStream->close();  
      delete m_folderStream;
    }
    m_folderStream = nsnull;
    m_ownFolderStream = PR_FALSE;
  }
  SetSummaryValid(PR_TRUE);
  Commit(nsMsgDBCommitType::kLargeCommit);
  return NS_OK;
}

NS_IMETHODIMP nsMailDatabase::DeleteMessages(nsMsgKeyArray* nsMsgKeys, nsIDBChangeListener *instigator)
{
  if (!m_folderStream && m_folder)
  {
    PRBool isLocked;
    m_folder->GetLocked(&isLocked);
    if (isLocked)
    {
      NS_ASSERTION(PR_FALSE, "Some other operation is in progress");
      return NS_MSG_FOLDER_BUSY;
    }
    m_folderStream = new nsIOFileStream(nsFileSpec(*m_folderSpec));
    m_ownFolderStream = PR_TRUE;
  }

  nsresult rv = nsMsgDatabase::DeleteMessages(nsMsgKeys, instigator);
  if (m_ownFolderStream)//only if we own the stream, then we should close it
  {
    if (m_folderStream)
    {
      m_folderStream->flush(); // this does a sync
      m_folderStream->close();
      delete m_folderStream;
    }
    m_folderStream = nsnull;
    m_ownFolderStream = PR_FALSE;
  }

  SetFolderInfoValid(m_folderSpec, 0, 0);
  return rv;
}

// Helper routine - lowest level of flag setting
PRBool nsMailDatabase::SetHdrFlag(nsIMsgDBHdr *msgHdr, PRBool bSet, MsgFlags flag)
{
  nsIOFileStream *fileStream = nsnull;
  PRBool ret = PR_FALSE;

  if (!m_folderStream && m_folder)  //we are going to create a stream, bail out if someone else has lock
  {
    PRBool isLocked;
    m_folder->GetLocked(&isLocked);
    if (isLocked)
    {
      NS_ASSERTION(PR_FALSE, "Some other operation is in progress");
      return PR_FALSE;
    }
  }
  if (nsMsgDatabase::SetHdrFlag(msgHdr, bSet, flag))
  {
    UpdateFolderFlag(msgHdr, bSet, flag, &fileStream);
    if (fileStream)
    {
      fileStream->flush();
      fileStream->close();
      delete fileStream;
      SetFolderInfoValid(m_folderSpec, 0, 0);
    }
    ret = PR_TRUE;
  }

  return ret;
}

#ifdef XP_MAC
extern PRFileDesc *gIncorporateFID;
extern const char* gIncorporatePath;
#endif // XP_MAC

// ### should move this into some utils class...
int msg_UnHex(char C)
{
	return ((C >= '0' && C <= '9') ? C - '0' :
			((C >= 'A' && C <= 'F') ? C - 'A' + 10 :
			 ((C >= 'a' && C <= 'f') ? C - 'a' + 10 : 0)));
}


// We let the caller close the file in case he's updating a lot of flags
// and we don't want to open and close the file every time through.
// As an experiment, try caching the fid in the db as m_folderFile.
// If this is set, use it but don't return *pFid.
void nsMailDatabase::UpdateFolderFlag(nsIMsgDBHdr *mailHdr, PRBool bSet, 
							  MsgFlags flag, nsIOFileStream **ppFileStream)
{
  static char buf[50];
  PRInt32 folderStreamPos = 0; //saves the folderStream pos in case we are sharing the stream with other code
  nsIOFileStream *fileStream = (m_folderStream) ? m_folderStream : *ppFileStream;
  //#ifdef GET_FILE_STUFF_TOGETHER
#ifdef XP_MAC
  /* ducarroz: Do we still need this ??
  // This is a horrible hack and we should make sure we don't need it anymore.
  // It has to do with multiple people having the same file open, I believe, but the
  // mac file system only has one handle, and they compete for the file position.
  // Prevent closing the file from under the incorporate stuff. #82785.
  int32 savedPosition = -1;
  if (!fid && gIncorporatePath && !strcmp(m_folderSpec, gIncorporatePath))
  {
		fid = gIncorporateFID;
                savedPosition = ftell(gIncorporateFID); // so we can restore it.
                }
  */
#endif // XP_MAC
  PRUint32 offset;
  (void)mailHdr->GetStatusOffset(&offset);
  if (offset > 0) 
  {
    
    if (fileStream == NULL) 
    {
      fileStream = new nsIOFileStream(nsFileSpec(*m_folderSpec));
    }
    else if (!m_ownFolderStream)
    {
      m_folderStream->flush();
      folderStreamPos = m_folderStream->tell();
    }
    if (fileStream) 
    {
      PRUint32 msgOffset;
      (void)mailHdr->GetMessageOffset(&msgOffset);
      PRUint32 statusPos = offset + msgOffset;
      PR_ASSERT(offset < 10000);
      fileStream->seek(PR_SEEK_SET, statusPos);
      buf[0] = '\0';
      if (fileStream->readline(buf, sizeof(buf))) 
      {
        if (strncmp(buf, X_MOZILLA_STATUS, X_MOZILLA_STATUS_LEN) == 0 &&
          strncmp(buf + X_MOZILLA_STATUS_LEN, ": ", 2) == 0 &&
          strlen(buf) >= X_MOZILLA_STATUS_LEN + 6) 
        {
          PRUint32 flags;
          (void)mailHdr->GetFlags(&flags);
          if (!(flags & MSG_FLAG_EXPUNGED))
          {
            int i;
            char *p = buf + X_MOZILLA_STATUS_LEN + 2;
            
            for (i=0, flags = 0; i<4; i++, p++)
            {
              flags = (flags << 4) | msg_UnHex(*p);
            }
            
            PRUint32 curFlags;
            (void)mailHdr->GetFlags(&curFlags);
            flags = (flags & MSG_FLAG_QUEUED) |
              (curFlags & ~MSG_FLAG_RUNTIME_ONLY);
          }
          else
          {
            flags &= ~MSG_FLAG_RUNTIME_ONLY;
          }
          fileStream->seek(statusPos);
          // We are filing out x-mozilla-status flags here
          PR_snprintf(buf, sizeof(buf), X_MOZILLA_STATUS_FORMAT,
            flags & 0x0000FFFF);
          PRInt32 lineLen = PL_strlen(buf);
          PRUint32 status2Pos = statusPos + lineLen + MSG_LINEBREAK_LEN;
          fileStream->write(buf, lineLen);
          
          // time to upate x-mozilla-status2
          fileStream->seek(status2Pos);
          if (fileStream->readline(buf, sizeof(buf))) 
          {
            if (strncmp(buf, X_MOZILLA_STATUS2, X_MOZILLA_STATUS2_LEN) == 0 &&
              strncmp(buf + X_MOZILLA_STATUS2_LEN, ": ", 2) == 0 &&
              strlen(buf) >= X_MOZILLA_STATUS2_LEN + 10) 
            {
              PRUint32 dbFlags;
              (void)mailHdr->GetFlags(&dbFlags);
              dbFlags &= 0xFFFF0000;
              fileStream->seek(status2Pos);
              PR_snprintf(buf, sizeof(buf), X_MOZILLA_STATUS2_FORMAT, dbFlags);
              fileStream->write(buf, PL_strlen(buf));
            }
          }
        } else 
        {
#ifdef DEBUG
          printf("Didn't find %s where expected at position %ld\n"
            "instead, found %s.\n",
            X_MOZILLA_STATUS, (long) statusPos, buf);
#endif
          SetReparse(PR_TRUE);
        }			
      } 
      else 
      {
#ifdef DEBUG
        printf("Couldn't read old status line at all at position %ld\n",
          (long) statusPos);
#endif
        SetReparse(PR_TRUE);
      }
#ifdef XP_MAC
      /* ducarroz: Do we still need this ??
      // Restore the file position
      if (savedPosition >= 0)
      XP_FileSeek(fid, savedPosition, SEEK_SET);
      */
#endif
    }
    else
    {
#ifdef DEBUG
      printf("Couldn't open mail folder for update%s!\n",
        (const char*)m_folderSpec);
#endif
      PR_ASSERT(PR_FALSE);
    }
  }
  //#endif // GET_FILE_STUFF_TOGETHER
  if (!m_folderStream)
    *ppFileStream = fileStream; // This tells the caller that we opened the file, and please to close it.
  else if (!m_ownFolderStream)
    m_folderStream->seek(PR_SEEK_SET, folderStreamPos);
}

PRUint32 nsMailDatabase::GetMailboxModDate()
{
  PRUint32 retModTime = 0;
  nsCOMPtr <nsILocalFile> localFile;
  PRInt64 lastModTime;
  nsresult rv = NS_FileSpecToIFile(m_folderSpec, getter_AddRefs(localFile));
  if (NS_SUCCEEDED(rv))
  {
    rv = localFile->GetLastModifiedTime(&lastModTime);
    if (NS_SUCCEEDED(rv))
    {

      PRTime  temp64;
      PRInt64 thousand;
      LL_I2L(thousand, PR_MSEC_PER_SEC);
      LL_DIV(temp64, lastModTime, thousand);
      LL_L2UI(retModTime, temp64);
    }
  }
  if (!retModTime)
    m_folderSpec->GetModDate(retModTime) ;

  return retModTime;
}

NS_IMETHODIMP nsMailDatabase::GetSummaryValid(PRBool *aResult)
{
  NS_ENSURE_ARG_POINTER(aResult);
  PRUint32 folderSize;
  PRUint32  folderDate;
  nsFileSpec::TimeStamp actualFolderTimeStamp;
  PRInt32 numUnreadMessages;
  nsAutoString errorMsg;

        
  *aResult = PR_FALSE;
  
  if (m_folderSpec && m_dbFolderInfo)
  {
    actualFolderTimeStamp = GetMailboxModDate();
  
    m_dbFolderInfo->GetNumUnreadMessages(&numUnreadMessages);
    m_dbFolderInfo->GetFolderSize(&folderSize);
    m_dbFolderInfo->GetFolderDate(&folderDate);

    // compare current version of db versus filed out version info, 
    // and file size in db vs file size on disk.
    PRUint32 version;

    m_dbFolderInfo->GetVersion(&version);
    if (folderSize == m_folderSpec->GetFileSize() &&
        numUnreadMessages >= 0 && GetCurVersion() == version)
    {
      GetGlobalPrefs();
      // if those values are ok, check time stamp
      if (gTimeStampLeeway == 0)
        *aResult = folderDate == actualFolderTimeStamp;
      else
        *aResult = PR_ABS((PRInt32) (actualFolderTimeStamp - folderDate)) <= gTimeStampLeeway;
#ifndef PUTUP_ALERT_ON_INVALID_DB
    }
  }
#else
      if (!*aResult)
      {
        errorMsg.AppendLiteral("time stamp didn't match delta = ");
        errorMsg.AppendInt(actualFolderTimeStamp - folderDate);
        errorMsg.AppendLiteral(" leeway = ");
        errorMsg.AppendInt(gTimeStampLeeway);
      }
    }
    else if (folderSize != m_folderSpec->GetFileSize())
    {
      errorMsg.AppendLiteral("folder size didn't match db size = ");
      errorMsg.AppendInt(folderSize);
      errorMsg.AppendLiteral(" actual size = ");
      errorMsg.AppendInt(m_folderSpec->GetFileSize());
    }
    else if (numUnreadMessages < 0)
    {
      errorMsg.AppendLiteral("numUnreadMessages < 0");
    }
  }
  if (errorMsg.Length())
  {
    nsCOMPtr<nsIPrompt> dialog;

    nsCOMPtr<nsIWindowWatcher> wwatch(do_GetService(NS_WINDOWWATCHER_CONTRACTID));
    if (wwatch)
      wwatch->GetNewPrompter(0, getter_AddRefs(dialog));
    if (dialog)
      dialog->Alert(nsnull, errorMsg.get());
  }
#endif // PUTUP_ALERT_ON_INVALID_DB
  return NS_OK;
}

NS_IMETHODIMP nsMailDatabase::SetSummaryValid(PRBool valid)
{
  nsresult ret = NS_OK;
  
  if (!m_folderSpec->Exists()) 
    return NS_MSG_ERROR_FOLDER_MISSING;
  
  if (m_dbFolderInfo)
  {
    if (valid)
    {
      nsFileSpec::TimeStamp actualFolderTimeStamp = GetMailboxModDate();
      
      m_dbFolderInfo->SetFolderSize(m_folderSpec->GetFileSize());
      m_dbFolderInfo->SetFolderDate(actualFolderTimeStamp);
    }
    else
    {
      m_dbFolderInfo->SetVersion(0);	// that ought to do the trick.
    }
  }
  return ret;
}

nsresult nsMailDatabase::GetFolderName(nsString &folderName)
{
	folderName.AssignWithConversion(NS_STATIC_CAST(const char*, *m_folderSpec));
	return NS_OK;
}


NS_IMETHODIMP  nsMailDatabase::RemoveOfflineOp(nsIMsgOfflineImapOperation *op)
{
  
  nsresult rv = GetAllOfflineOpsTable();
  NS_ENSURE_SUCCESS(rv, rv);
  
  if (!op || !m_mdbAllOfflineOpsTable)
    return NS_ERROR_NULL_POINTER;
  nsMsgOfflineImapOperation* offlineOp = NS_STATIC_CAST(nsMsgOfflineImapOperation*, op);  // closed system, so this is ok
  nsIMdbRow* row = offlineOp->GetMDBRow();
  rv = m_mdbAllOfflineOpsTable->CutRow(GetEnv(), row);
  row->CutAllColumns(GetEnv());
  return rv;
}

NS_IMETHODIMP nsMailDatabase::GetOfflineOpForKey(nsMsgKey msgKey, PRBool create, nsIMsgOfflineImapOperation **offlineOp)
{
  mdb_bool	hasOid;
  mdbOid		rowObjectId;
  mdb_err   err;
  
  if (!IMAPOffline)
    IMAPOffline = PR_NewLogModule("IMAPOFFLINE");
  nsresult rv = GetAllOfflineOpsTable();
  NS_ENSURE_SUCCESS(rv, rv);
  
  if (!offlineOp || !m_mdbAllOfflineOpsTable)
    return NS_ERROR_NULL_POINTER;
  
  *offlineOp = NULL;
  
  rowObjectId.mOid_Id = msgKey;
  rowObjectId.mOid_Scope = m_offlineOpsRowScopeToken;
  err = m_mdbAllOfflineOpsTable->HasOid(GetEnv(), &rowObjectId, &hasOid);
  if (err == NS_OK && m_mdbStore && (hasOid  || create))
  {
    nsCOMPtr <nsIMdbRow> offlineOpRow;
    err = m_mdbStore->GetRow(GetEnv(), &rowObjectId, getter_AddRefs(offlineOpRow));
    
    if (create)
    {
      if (!offlineOpRow)
      {
        err  = m_mdbStore->NewRowWithOid(GetEnv(), &rowObjectId, getter_AddRefs(offlineOpRow));
        NS_ENSURE_SUCCESS(err, err);
      }
      if (offlineOpRow && !hasOid)
        m_mdbAllOfflineOpsTable->AddRow(GetEnv(), offlineOpRow);
    }
    
    if (err == NS_OK && offlineOpRow)
    {
      *offlineOp = new nsMsgOfflineImapOperation(this, offlineOpRow);
      if (*offlineOp)
        (*offlineOp)->SetMessageKey(msgKey);
      NS_IF_ADDREF(*offlineOp);
    }
    if (!hasOid && m_dbFolderInfo)
    {
      // set initial value for flags so we don't lose them.
      nsCOMPtr <nsIMsgDBHdr> msgHdr;
      GetMsgHdrForKey(msgKey, getter_AddRefs(msgHdr));
      if (msgHdr)
      {
        PRUint32 flags;
        msgHdr->GetFlags(&flags);
        (*offlineOp)->SetNewFlags(flags);
      }
      PRInt32 newFlags;
      m_dbFolderInfo->OrFlags(MSG_FOLDER_FLAG_OFFLINEEVENTS, &newFlags);
    }
  }
  
  return (err == 0) ? NS_OK : NS_ERROR_FAILURE;

}

NS_IMETHODIMP nsMailDatabase::EnumerateOfflineOps(nsISimpleEnumerator **enumerator)
{
  NS_ASSERTION(PR_FALSE, "not impl yet");
  return NS_ERROR_NOT_IMPLEMENTED;
}


NS_IMETHODIMP nsMailDatabase::ListAllOfflineOpIds(nsMsgKeyArray *offlineOpIds)
{
  NS_ENSURE_ARG(offlineOpIds);
  nsresult rv = GetAllOfflineOpsTable();
  NS_ENSURE_SUCCESS(rv, rv);
  nsIMdbTableRowCursor *rowCursor;
  if (!IMAPOffline)
    IMAPOffline = PR_NewLogModule("IMAPOFFLINE");

  if (m_mdbAllOfflineOpsTable)
  {
    nsresult err = m_mdbAllOfflineOpsTable->GetTableRowCursor(GetEnv(), -1, &rowCursor);
    while (err == NS_OK && rowCursor)
    {
      mdbOid outOid;
      mdb_pos	outPos;
      
      err = rowCursor->NextRowOid(GetEnv(), &outOid, &outPos);
      // is this right? Mork is returning a 0 id, but that should valid.
      if (outPos < 0 || outOid.mOid_Id == (mdb_id) -1)	
        break;
      if (err == NS_OK)
      {
        offlineOpIds->Add(outOid.mOid_Id);
        if (PR_LOG_TEST(IMAPOffline, PR_LOG_ALWAYS))
        {
          nsCOMPtr <nsIMsgOfflineImapOperation> offlineOp;
          GetOfflineOpForKey(outOid.mOid_Id, PR_FALSE, getter_AddRefs(offlineOp));
          if (offlineOp)
          {
            nsMsgOfflineImapOperation *logOp = NS_STATIC_CAST(nsMsgOfflineImapOperation *, 
              NS_STATIC_CAST(nsIMsgOfflineImapOperation *, offlineOp.get()));
            if (logOp)
              logOp->Log(IMAPOffline);

          }
        }
      }
    }
    rv = (err == NS_OK) ? NS_OK : NS_ERROR_FAILURE;
    rowCursor->Release();
  }
  
  offlineOpIds->QuickSort();
  return rv;
}

NS_IMETHODIMP nsMailDatabase::ListAllOfflineDeletes(nsMsgKeyArray *offlineDeletes)
{
  if (!offlineDeletes)
    return NS_ERROR_NULL_POINTER;
  
  nsresult rv = GetAllOfflineOpsTable();
  NS_ENSURE_SUCCESS(rv, rv);
  nsIMdbTableRowCursor *rowCursor;
  if (m_mdbAllOfflineOpsTable)
  {
    nsresult err = m_mdbAllOfflineOpsTable->GetTableRowCursor(GetEnv(), -1, &rowCursor);
    while (err == NS_OK && rowCursor)
    {
      mdbOid outOid;
      mdb_pos	outPos;
      nsIMdbRow* offlineOpRow;
      
      err = rowCursor->NextRow(GetEnv(), &offlineOpRow, &outPos);
      // is this right? Mork is returning a 0 id, but that should valid.
      if (outPos < 0 || offlineOpRow == nsnull)	
        break;
      if (err == NS_OK)
      {
        offlineOpRow->GetOid(GetEnv(), &outOid);
        nsIMsgOfflineImapOperation *offlineOp = new nsMsgOfflineImapOperation(this, offlineOpRow);
        if (offlineOp)
        {
          NS_ADDREF(offlineOp);
          imapMessageFlagsType newFlags;
          nsOfflineImapOperationType opType;
          
          offlineOp->GetOperation(&opType);
          offlineOp->GetNewFlags(&newFlags);
          if (opType & nsIMsgOfflineImapOperation::kMsgMoved || 
            ((opType & nsIMsgOfflineImapOperation::kFlagsChanged) 
            && (newFlags & nsIMsgOfflineImapOperation::kMsgMarkedDeleted)))
            offlineDeletes->Add(outOid.mOid_Id);
          NS_RELEASE(offlineOp);
        }
        offlineOpRow->Release();
      }
    }
    rv = (err == NS_OK) ? NS_OK : NS_ERROR_FAILURE;
    rowCursor->Release();
  }
  return rv;
}

/* static */
nsresult nsMailDatabase::SetFolderInfoValid(nsFileSpec *folderName, int num, int numunread)
{
  nsresult err = NS_OK;
  PRBool bOpenedDB = PR_FALSE;
  nsFileSpec summaryPath;
  GetSummaryFileLocation(*folderName, &summaryPath);
  
  if (!folderName->Exists())
    return NS_MSG_ERROR_FOLDER_SUMMARY_MISSING;
  
  // should we have type safe downcast methods again?
  nsMailDatabase *pMessageDB = (nsMailDatabase *) nsMailDatabase::FindInCache(summaryPath);
  if (pMessageDB == nsnull)
  {
    pMessageDB = new nsMailDatabase();
    if(!pMessageDB)
      return NS_ERROR_OUT_OF_MEMORY;
    
    pMessageDB->m_folderSpec = new nsFileSpec();
    if(!pMessageDB->m_folderSpec)
    {
      delete pMessageDB;
      return NS_ERROR_OUT_OF_MEMORY;
    }
    
    *(pMessageDB->m_folderSpec) = summaryPath;
    // ### this does later stuff (marks latered messages unread), which may be a problem
    err = pMessageDB->OpenMDB(summaryPath, PR_FALSE);
    if (err != NS_OK)
    {
      delete pMessageDB;
      pMessageDB = nsnull;
    }
    bOpenedDB = PR_TRUE;
  }
  
  if (pMessageDB == nsnull)
  {
#ifdef DEBUG
    printf("Exception opening summary file\n");
#endif
    return NS_MSG_ERROR_FOLDER_SUMMARY_OUT_OF_DATE;
  }
  
  {
    pMessageDB->m_folderSpec = folderName;
    nsFileSpec::TimeStamp actualFolderTimeStamp = pMessageDB->GetMailboxModDate();
    pMessageDB->m_dbFolderInfo->SetFolderSize(folderName->GetFileSize());
    pMessageDB->m_dbFolderInfo->SetFolderDate(actualFolderTimeStamp);
    pMessageDB->m_dbFolderInfo->ChangeNumUnreadMessages(numunread);
    pMessageDB->m_dbFolderInfo->ChangeNumMessages(num);
  }
  // if we opened the db, then we'd better close it. Otherwise, we found it in the cache,
  // so just commit and release.
  if (bOpenedDB)
  {
    pMessageDB->Close(PR_TRUE);
  }
  else if (pMessageDB)
  { 
    err = pMessageDB->Commit(nsMsgDBCommitType::kLargeCommit);
    pMessageDB->Release();
  }
  return err;
}


// This is used to remember that the db is out of sync with the mail folder
// and needs to be regenerated.
void nsMailDatabase::SetReparse(PRBool reparse)
{
  m_reparse = reparse;
}


class nsMsgOfflineOpEnumerator : public nsISimpleEnumerator {
public:
  NS_DECL_ISUPPORTS

  // nsISimpleEnumerator methods:
  NS_DECL_NSISIMPLEENUMERATOR

  nsMsgOfflineOpEnumerator(nsMailDatabase* db);
  virtual ~nsMsgOfflineOpEnumerator();

protected:
  nsresult					GetRowCursor();
  nsresult					PrefetchNext();
  nsMailDatabase*              mDB;
  nsIMdbTableRowCursor*       mRowCursor;
  nsCOMPtr <nsIMsgOfflineImapOperation> mResultOp;
  PRBool            mDone;
  PRBool						mNextPrefetched;
};

nsMsgOfflineOpEnumerator::nsMsgOfflineOpEnumerator(nsMailDatabase* db)
    : mDB(db), mRowCursor(nsnull), mDone(PR_FALSE)
{
  NS_ADDREF(mDB);
  mNextPrefetched = PR_FALSE;
}

nsMsgOfflineOpEnumerator::~nsMsgOfflineOpEnumerator()
{
  NS_IF_RELEASE(mRowCursor);
  NS_RELEASE(mDB);
}

NS_IMPL_ISUPPORTS1(nsMsgOfflineOpEnumerator, nsISimpleEnumerator)

nsresult nsMsgOfflineOpEnumerator::GetRowCursor()
{
  nsresult rv = 0;
  mDone = PR_FALSE;

  if (!mDB || !mDB->m_mdbAllOfflineOpsTable)
    return NS_ERROR_NULL_POINTER;

  rv = mDB->m_mdbAllOfflineOpsTable->GetTableRowCursor(mDB->GetEnv(), -1, &mRowCursor);
  return rv;
}

NS_IMETHODIMP nsMsgOfflineOpEnumerator::GetNext(nsISupports **aItem)
{
  if (!aItem)
    return NS_ERROR_NULL_POINTER;
  nsresult rv=NS_OK;
  if (!mNextPrefetched)
    rv = PrefetchNext();
  if (NS_SUCCEEDED(rv))
  {
    if (mResultOp) 
    {
      *aItem = mResultOp;
      NS_ADDREF(*aItem);
      mNextPrefetched = PR_FALSE;
    }
  }
  return rv;
}

nsresult nsMsgOfflineOpEnumerator::PrefetchNext()
{
  nsresult rv = NS_OK;
  nsIMdbRow* offlineOpRow;
  mdb_pos rowPos;

  if (!mRowCursor)
  {
    rv = GetRowCursor();
    if (NS_FAILED(rv))
      return rv;
  }

  rv = mRowCursor->NextRow(mDB->GetEnv(), &offlineOpRow, &rowPos);
  if (!offlineOpRow) 
  {
    mDone = PR_TRUE;
    return NS_ERROR_FAILURE;
  }
  if (NS_FAILED(rv)) 
  {
    mDone = PR_TRUE;
    return rv;
  }
	//Get key from row
  mdbOid outOid;
  nsMsgKey key=0;
  if (offlineOpRow->GetOid(mDB->GetEnv(), &outOid) == NS_OK)
    key = outOid.mOid_Id;

  nsIMsgOfflineImapOperation *op = new nsMsgOfflineImapOperation(mDB, offlineOpRow);
  mResultOp = op;
  if (!op)
    return NS_ERROR_OUT_OF_MEMORY;

  if (mResultOp) 
  {
    mNextPrefetched = PR_TRUE;
    return NS_OK;
  }
  return NS_ERROR_FAILURE;
}

NS_IMETHODIMP nsMsgOfflineOpEnumerator::HasMoreElements(PRBool *aResult)
{
  if (!aResult)
    return NS_ERROR_NULL_POINTER;

  if (!mNextPrefetched)
    PrefetchNext();
  *aResult = !mDone;
  return NS_OK;
}
