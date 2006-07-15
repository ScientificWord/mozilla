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
 * The Original Code is Mozilla
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corp.
 * Portions created by the Initial Developer are Copyright (C) 2001
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s): Krishna Mohan Khandrika (kkhandrika@netscape.com)
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

#ifndef MSG_MAPI_MAIN_H_
#define NSG_MAPI_MAIN_H_

#define        MAX_NAME_LEN    256
#define        MAX_PW_LEN      256
#define        MAX_SESSIONS    50
#define        MAPI_SENDCOMPLETE_EVENT   "SendCompletionEvent"

#define MAPI_PROPERTIES_CHROME "chrome://messenger-mapi/locale/mapi.properties"
#define PREF_MAPI_WARN_PRIOR_TO_BLIND_SEND "mapi.blind-send.warn"
#define PREF_MAPI_BLIND_SEND_ENABLED "mapi.blind-send.enabled"

#include "nsXPIDLString.h"
#include "nspr.h"
#include "nsString.h"
#include "nsHashtable.h"

class nsMAPIConfiguration
{
private :

    static PRUint32 session_generator;
    static PRUint32 sessionCount;
    static nsMAPIConfiguration *m_pSelfRef;
    PRLock *m_Lock;
    PRUint32  m_nMaxSessions;


    nsHashtable m_ProfileMap;
    nsHashtable m_SessionMap;
    nsMAPIConfiguration();

public :
    static nsMAPIConfiguration *GetMAPIConfiguration();
    void OpenConfiguration();
    PRInt16 RegisterSession(PRUint32 aHwnd, const PRUnichar *aUserName, \
                            const PRUnichar *aPassword, PRBool aForceDownLoad, \
                            PRBool aNewSession, PRUint32 *aSession, char *aIdKey);
    PRBool IsSessionValid(PRUint32 aSessionID);
    PRBool UnRegisterSession(PRUint32 aSessionID);
    PRUnichar *GetPassword(PRUint32 aSessionID);
    char *GetIdKey(PRUint32 aSessionID);
    void *GetMapiListContext(PRUint32 aSessionID);
    void SetMapiListContext(PRUint32 aSessionID, void *mapiListContext);
    ~nsMAPIConfiguration();

    // a util func
    static HRESULT GetMAPIErrorFromNSError (nsresult res) ;
};

class nsMAPISession
{
    friend class nsMAPIConfiguration;

    private :

        PRBool   m_bIsForcedDownLoad;
        PRBool   m_bApp_or_Service;
        PRUint32 m_hAppHandle;
        PRUint32 m_nShared;
        char     *m_pIdKey;
        nsString m_pProfileName;
        nsString m_pPassword;
        PRInt32 m_messageIndex;
        void   *m_listContext; // used by findNext

    public :

        nsMAPISession(PRUint32 aHwnd, const PRUnichar *aUserName, \
                      const PRUnichar *aPassword, \
                      PRBool aForceDownLoad, char *aKey);
        PRUint32 IncrementSession();
        PRUint32 DecrementSession();
        PRUint32 GetSessionCount();
        PRUnichar *nsMAPISession::GetPassword();
        char *nsMAPISession::GetIdKey();
        ~nsMAPISession();
        // For enumerating Messages...
        void SetMapiListContext( void *listContext) { m_listContext = listContext; } 
        void *GetMapiListContext( ) { return m_listContext; }
};

#endif    // MSG_MAPI_MAIN_H_
