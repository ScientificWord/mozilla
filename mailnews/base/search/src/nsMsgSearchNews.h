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
 * Portions created by the Initial Developer are Copyright (C) 2000
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

#ifndef _nsMsgSearchNews_h__
#include "nsMsgSearchAdapter.h"

//-----------------------------------------------------------------------------
//---------- Adapter class for searching online (news) folders ----------------
//-----------------------------------------------------------------------------

class nsMsgSearchNews : public nsMsgSearchAdapter
{
public:
	nsMsgSearchNews (nsMsgSearchScopeTerm *scope, nsISupportsArray *termList);
	virtual ~nsMsgSearchNews ();

  NS_IMETHOD ValidateTerms ();
  NS_IMETHOD Search (PRBool *aDone);
  NS_IMETHOD GetEncoding (char **result);
  NS_IMETHOD AddHit(nsMsgKey key);
  NS_IMETHOD CurrentUrlDone(PRInt32 exitCode);

	virtual nsresult Encode (nsCString *outEncoding);
	virtual char *EncodeTerm (nsIMsgSearchTerm *);
	PRUnichar *EncodeToWildmat (const PRUnichar *);
	
  void ReportHits ();
    PRBool DuplicateHit(PRUint32 artNum) ;
    void CollateHits ();
    void ReportHit (nsIMsgDBHdr *pHeaders, nsIMsgFolder *folder);
    static int PR_CALLBACK CompareArticleNumbers (const void *v1, const void *v2, void *data);

protected:
	nsCString m_encoding;
	PRBool m_ORSearch; // set to true if any of the search terms contains an OR for a boolean operator. 

	nsMsgKeyArray m_candidateHits;
	nsMsgKeyArray m_hits;

	static const char *m_kNntpFrom;
	static const char *m_kNntpSubject;
	static const char *m_kTermSeparator;
	static const char *m_kUrlPrefix;
};

#endif

