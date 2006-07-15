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

#ifndef nsPop3Protocol_h__
#define nsPop3Protocol_h__

#include "nsIStreamListener.h"
#include "nsIOutputStream.h"
#include "nsIInputStream.h"
#include "nsIPop3URL.h"
#include "nsIPop3Sink.h"
#include "nsMsgLineBuffer.h"
#include "nsMsgProtocol.h"
#include "nsIPop3Protocol.h"
#include "MailNewsTypes.h"
#include "nsLocalStringBundle.h"
#include "nsIMsgStatusFeedback.h"
#include "nsIMsgFolder.h" // TO include biffState enum. Change to bool later...
#include "nsIAuthModule.h"
#include "nsITimer.h"

#include "prerror.h"
#include "plhash.h"
#include "nsCOMPtr.h"

/* A more guaranteed way of making sure that we never get duplicate messages
is to always get each message's UIDL (if the server supports it)
and use these for storing up deletes which were not committed on the
server.  Based on our experience, it looks like we do NOT need to
do this (it has performance tradeoffs, etc.).  To turn it on, three
things need to happen: #define POP_ALWAYS_USE_UIDL_FOR_DUPLICATES, verify that
the uidl's are correctly getting added when the delete response is received,
and change the POP3_QUIT_RESPONSE state to flush the newly committed deletes. */

/* 
 * Cannot have the following line uncommented. It is defined.
 * #ifdef POP_ALWAYS_USE_UIDL_FOR_DUPLICATES will always be evaluated
 * as PR_TRUE.
 *
#define POP_ALWAYS_USE_UIDL_FOR_DUPLICATES 0
 *
 */

#define MK_OUT_OF_MEMORY -207
#define MK_POP3_OUT_OF_DISK_SPACE -321
#define MK_POP3_PASSWORD_UNDEFINED -313
#define XP_NO_ANSWER 14401
#define XP_THE_PREVIOUSLY_ENTERED_PASSWORD_IS_INVALID_ETC 14405
#define XP_PASSWORD_FOR_POP3_USER 14590

#define OUTPUT_BUFFER_SIZE 8192 // maximum size of command string

/* structure to hold data pertaining to the active state of
 * a transfer in progress.
 *
 */

enum Pop3CapabilityEnum {
    POP3_CAPABILITY_UNDEFINED   = 0x00000000,
    POP3_HAS_XSENDER            = 0x00000001,
    POP3_GURL_UNDEFINED         = 0x00000002,
    POP3_HAS_GURL               = 0x00000004,
    POP3_UIDL_UNDEFINED         = 0x00000008,
    POP3_HAS_UIDL               = 0x00000010,
    POP3_XTND_XLST_UNDEFINED    = 0x00000020,
    POP3_HAS_XTND_XLST          = 0x00000040,
    POP3_TOP_UNDEFINED          = 0x00000080,
    POP3_HAS_TOP                = 0x00000100,
    POP3_AUTH_MECH_UNDEFINED    = 0x00000200,
    POP3_HAS_AUTH_USER          = 0x00000400,
    POP3_HAS_AUTH_LOGIN         = 0x00000800,
    POP3_HAS_AUTH_PLAIN         = 0x00001000,
    POP3_HAS_AUTH_CRAM_MD5      = 0x00002000,
    POP3_HAS_AUTH_APOP          = 0x00004000,
    POP3_HAS_AUTH_NTLM          = 0x00008000,
    POP3_HAS_AUTH_MSN           = 0x00010000,
    POP3_HAS_RESP_CODES         = 0x00020000,
    POP3_HAS_AUTH_RESP_CODE     = 0x00040000,
    POP3_HAS_STLS               = 0x00080000,
    POP3_HAS_AUTH_GSSAPI        = 0x00100000
};

#define POP3_HAS_AUTH_ANY         0x00001C00
#define POP3_HAS_AUTH_ANY_SEC     0x0011E000

enum Pop3StatesEnum {
    POP3_READ_PASSWORD,                         // 0
                                                // 
    POP3_START_CONNECT,                         // 1
    POP3_FINISH_CONNECT,                        // 2
    POP3_WAIT_FOR_RESPONSE,                     // 3
    POP3_WAIT_FOR_START_OF_CONNECTION_RESPONSE, // 4
    POP3_SEND_USERNAME,                         // 5

    POP3_SEND_PASSWORD,                         // 6
    POP3_SEND_STAT,                             // 7
    POP3_GET_STAT,                              // 8
    POP3_SEND_LIST,                             // 9
    POP3_GET_LIST,                              // 10

    POP3_SEND_UIDL_LIST,                        // 11
    POP3_GET_UIDL_LIST,                         // 12
    POP3_SEND_XTND_XLST_MSGID,                  // 13
    POP3_GET_XTND_XLST_MSGID,                   // 14
    POP3_GET_MSG,                               // 15

    POP3_SEND_TOP,                              // 16
    POP3_TOP_RESPONSE,                          // 17
    POP3_SEND_RETR,                             // 18
    POP3_RETR_RESPONSE,                         // 19
    POP3_SEND_DELE,                             // 20

    POP3_DELE_RESPONSE,                         // 21
    POP3_SEND_QUIT,                             // 22
    POP3_DONE,                                  // 23
    POP3_ERROR_DONE,                            // 24
    POP3_FREE,                                  // 25
    /* The following 3 states support the use of the 'TOP' command instead of UIDL
       for leaving mail on the pop server -km */
    POP3_START_USE_TOP_FOR_FAKE_UIDL,           // 26
    POP3_SEND_FAKE_UIDL_TOP,                    // 27
    POP3_GET_FAKE_UIDL_TOP,                     // 28
    POP3_SEND_AUTH,                             // 29
    POP3_AUTH_RESPONSE,                         // 30
    POP3_SEND_CAPA,                             // 31
    POP3_CAPA_RESPONSE,                         // 32
    POP3_PROCESS_AUTH,                          // 33
    POP3_AUTH_FALLBACK,                         // 34

    POP3_AUTH_LOGIN,                            // 35
    POP3_AUTH_LOGIN_RESPONSE,                   // 36
    POP3_AUTH_NTLM,                             // 37
    POP3_AUTH_NTLM_RESPONSE,                    // 38
    POP3_SEND_XSENDER,                          // 39
    POP3_XSENDER_RESPONSE,                      // 40
    POP3_SEND_GURL,                             // 41

    POP3_GURL_RESPONSE,                         // 42
    POP3_QUIT_RESPONSE,                         // 43
    POP3_INTERRUPTED,                           // 44
    POP3_TLS_RESPONSE,                          // 45
    
    POP3_AUTH_GSSAPI,                           // 46
    POP3_AUTH_GSSAPI_FIRST,                     // 47
    POP3_AUTH_GSSAPI_STEP                       // 48
};


#define KEEP        'k'         /* If we want to keep this item on server. */
#define DELETE_CHAR 'd'         /* If we want to delete this item. */
#define TOO_BIG     'b'         /* item left on server because it was too big */
#define FETCH_BODY  'f'         /* Fetch full body of a partial msg */

typedef struct Pop3UidlEntry { /* information about this message */
    char* uidl;
    char  status; // KEEP=='k', DELETE='d' TOO_BIG='b' FETCH_BODY='f'
    PRInt32 dateReceived; // time message received, used for aging
} Pop3UidlEntry;

typedef struct Pop3UidlHost {
    char* host;
    char* user;
    PLHashTable * hash;
    Pop3UidlEntry* uidlEntries;
    struct Pop3UidlHost* next;
} Pop3UidlHost;

typedef struct Pop3MsgInfo {
    PRInt32 msgnum;
    PRInt32 size;
    char* uidl;
} Pop3MsgInfo;

typedef struct _Pop3ConData {
    PRBool leave_on_server;     /* Whether we're supposed to leave messages
                                   on server. */
    PRBool headers_only;        /* Whether to just fetch headers on initial
                                   downloads. */
    PRInt32 size_limit;         /* Leave messages bigger than this on the
                                   server and only download a partial
                                   message. */
    PRUint32 capability_flags; /* What capability this server has? */
    
    Pop3StatesEnum next_state;  /* the next state or action to be taken */
    Pop3StatesEnum next_state_after_response;  
    PRBool pause_for_read;   		/* Pause now for next read? */
    
    PRBool command_succeeded;   /* did the last command succeed? */
    PRBool list_done;		/* did we get the complete list of msgIDs? */
    PRInt32 first_msg;

    PRUint32 obuffer_size;
    PRUint32 obuffer_fp;
    
    PRInt32	really_new_messages;
    PRInt32	real_new_counter;
    PRInt32 number_of_messages;
    Pop3MsgInfo	*msg_info;      /* Message sizes and uidls (used only if we
                                   are playing games that involve leaving
                                   messages on the server). */
    PRInt32 last_accessed_msg;
    PRInt32 cur_msg_size;
    PRBool truncating_cur_msg;  /* are we using top and uidl? */
    PRBool msg_del_started;     /* True if MSG_BeginMailDel...
                                 * called
                                 */
    PRBool only_check_for_new_mail;
    nsMsgBiffState biffstate;     /* If just checking for, what the answer is. */
    
    void *msg_closure;
    
    PRBool graph_progress_bytes_p; /* whether we should display info about
                                      the bytes transferred (usually we
                                      display info about the number of
                                      messages instead.) */
    
    Pop3UidlHost *uidlinfo;
    PLHashTable *newuidl;
    char *only_uidl;            /* If non-NULL, then load only this UIDL. */
    
    /* the following three fields support the 
       use of the 'TOP' command instead of UIDL
       for leaving mail on the pop server -km */
    
    /* the current message that we are retrieving 
       with the TOP command */
    PRInt32 current_msg_to_top;	

    /* we will download this many in 
       POP3_GET_MSG */
    PRInt32 number_of_messages_not_seen_before;
    /* reached when we have TOPped 
       all of the new messages */
    PRBool found_new_message_boundary; 
    
    /* if this is true then we don't stop 
       at new message boundary, we have to 
       TOP em all to delete some */
    PRBool delete_server_message_during_top_traversal;
    PRBool get_url;
    PRBool seenFromHeader;
    PRInt32 parsed_bytes;
    PRInt32 pop3_size;
    PRBool dot_fix;
    PRBool assumed_end;
    PRInt32 logonFailureCount;
} Pop3ConData;

// State Flags (Note, I use the word state in terms of storing 
// state information about the connection (authentication, have we sent
// commands, etc. I do not intend it to refer to protocol state)
#define POP3_PAUSE_FOR_READ			0x00000001  /* should we pause for the next read */
#define POP3_PASSWORD_FAILED		0x00000002
#define POP3_STOPLOGIN              0x00000004  /* error loging in, so stop here */
#define POP3_AUTH_FAILURE           0x00000008  /* extended code said authentication failed */


class nsPop3Protocol : public nsMsgProtocol, public nsIPop3Protocol
{
public:
  nsPop3Protocol(nsIURI* aURL);  
  virtual ~nsPop3Protocol();
  
  NS_DECL_ISUPPORTS_INHERITED
  NS_DECL_NSIPOP3PROTOCOL

  nsresult Initialize(nsIURI * aURL);
  virtual nsresult LoadUrl(nsIURI *aURL, nsISupports * aConsumer = nsnull);

  const char* GetUsername() { return m_username.get(); };
  void SetUsername(const char* name);

  nsresult GetPassword(char ** aPassword, PRBool *okayValue);

  NS_IMETHOD OnTransportStatus(nsITransport *transport, nsresult status, PRUint32 progress, PRUint32 progressMax);
  NS_IMETHOD OnStopRequest(nsIRequest *request, nsISupports * aContext, nsresult aStatus);
  NS_IMETHOD Cancel(nsresult status);

  static void MarkMsgInHashTable(PLHashTable *hashTable, const Pop3UidlEntry *uidl, 
                                  PRBool *changed);

  static nsresult MarkMsgForHost(const char *hostName, const char *userName,
                                      nsIFileSpec *mailDirectory, 
                                      nsVoidArray  &UIDLArray);
private:
  nsCString m_ApopTimestamp;
  nsCOMPtr<nsIMsgStringService> mStringService;

  nsCString m_username;
  nsCString m_senderInfo;
  nsCString m_commandResponse;
  nsCOMPtr<nsIMsgStatusFeedback> m_statusFeedback;
  nsCString m_GSSAPICache;

  // progress state information
  void UpdateProgressPercent (PRUint32 totalDone, PRUint32 total);
  void UpdateStatus(PRInt32 aStatusID);
  void UpdateStatusWithString(const PRUnichar * aString);

  PRInt32	m_bytesInMsgReceived; 
  PRInt32	m_totalFolderSize;    
  PRInt32	m_totalDownloadSize; /* Number of bytes we're going to
                                    download.  Might be much less
                                    than the total_folder_size. */
  PRInt32 m_totalBytesReceived; // total # bytes received for the connection

  virtual nsresult ProcessProtocolState(nsIURI * url, nsIInputStream * inputStream, 
                                        PRUint32 sourceOffset, PRUint32 length);
  virtual nsresult CloseSocket();
  virtual PRInt32 SendData(nsIURI * aURL, const char * dataBuffer, PRBool aSuppressLogging = PR_FALSE);

  nsCOMPtr<nsIURI> m_url;
  nsCOMPtr<nsIPop3Sink> m_nsIPop3Sink;
  nsCOMPtr<nsIPop3IncomingServer> m_pop3Server;

  nsMsgLineStreamBuffer   * m_lineStreamBuffer; // used to efficiently extract lines from the incoming data stream
  Pop3ConData* m_pop3ConData;
  void FreeMsgInfo();
  void Abort();

  PRBool m_parsingMultiLineMessageId;

  PRBool m_tlsEnabled;
  PRInt32 m_socketType;
  PRBool m_useSecAuth;
  PRBool m_password_already_sent;

  void SetCapFlag(PRUint32 flag);
  void ClearCapFlag(PRUint32 flag);
  PRBool TestCapFlag(PRUint32 flag);

  void BackupAuthFlags();
  void RestoreAuthFlags();
  PRInt32 m_origAuthFlags;
  PRInt32 m_listpos;

  nsresult HandleLine(char *line, PRUint32 line_length);

  //////////////////////////////////////////////////////////////////////////////////////////
      // Begin Pop3 protocol state handlers
      //////////////////////////////////////////////////////////////////////////////////////////
  PRInt32 WaitForStartOfConnectionResponse(nsIInputStream* inputStream, 
                                           PRUint32 length);
  PRInt32 WaitForResponse(nsIInputStream* inputStream, 
                          PRUint32 length);
  PRInt32 Error(PRInt32 err_code);
  PRInt32 SendAuth();
  PRInt32 AuthResponse(nsIInputStream* inputStream, PRUint32 length);
  PRInt32 SendCapa();
  PRInt32 CapaResponse(nsIInputStream* inputStream, PRUint32 length);
  PRInt32 SendTLSResponse();
  PRInt32 ProcessAuth();
  PRInt32 AuthFallback();
  PRInt32 AuthLogin();
  PRInt32 AuthLoginResponse();
  PRInt32 AuthNtlm();
  PRInt32 AuthNtlmResponse();
  PRInt32 AuthGSSAPI();
  PRInt32 AuthGSSAPIResponse(PRBool first);
  PRInt32 SendUsername();
  PRInt32 SendPassword();
  PRInt32 SendStatOrGurl(PRBool sendStat);
  PRInt32 SendStat();
  PRInt32 GetStat();
  PRInt32 SendGurl();
  PRInt32 GurlResponse();
  PRInt32 SendList();
  PRInt32 GetList(nsIInputStream* inputStream, PRUint32 length);
  PRInt32 SendFakeUidlTop();
  PRInt32 StartUseTopForFakeUidl();
  PRInt32 GetFakeUidlTop(nsIInputStream* inputStream, PRUint32 length);
  PRInt32 SendXtndXlstMsgid();
  PRInt32 GetXtndXlstMsgid(nsIInputStream* inputStream, PRUint32 length);
  PRInt32 SendUidlList();
  PRInt32 GetUidlList(nsIInputStream* inputStream, PRUint32 length);
  PRInt32 GetMsg();
  PRInt32 SendTop();
  PRInt32 SendXsender();
  PRInt32 XsenderResponse();
  PRInt32 SendRetr();

  PRInt32 RetrResponse(nsIInputStream* inputStream, PRUint32 length);
  PRInt32 TopResponse(nsIInputStream* inputStream, PRUint32 length);
  PRInt32 SendDele();
  PRInt32 DeleResponse();
  PRInt32 CommitState(PRBool remove_last_entry);
};

#endif /* nsPop3Protocol_h__ */
