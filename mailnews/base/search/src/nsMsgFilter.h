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
 *   David Bienvenu (bienvenu@nventure.com)
 *   Howard Chu <hyc@symas.com>
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

#ifndef _nsMsgFilter_H_
#define _nsMsgFilter_H_

#include "nscore.h"
#include "nsISupports.h"
#include "nsIMsgFilter.h"
#include "nsMsgSearchArray.h"
#include "nsIMsgSearchScopeTerm.h"
#include "nsMsgSearchBoolExpression.h"

class nsMsgRuleAction : public nsIMsgRuleAction
{
public:
  NS_DECL_ISUPPORTS

  nsMsgRuleAction();
  virtual ~nsMsgRuleAction();

  NS_DECL_NSIMSGRULEACTION

private:
    nsMsgRuleActionType      m_type;
		// this used to be a union - why bother?
    nsMsgPriorityValue	m_priority;  /* priority to set rule to */
    nsMsgLabelValue         m_label;  /* label to set rule to */
    nsCString		m_folderUri; 
    PRInt32             m_junkScore;  /* junk score (or arbitrary int value?) */
    // arbitrary string value. Currently, email address to forward to
    nsCString           m_strValue; 
} ;


class nsMsgFilter : public nsIMsgFilter
{
public:
  NS_DECL_ISUPPORTS

  nsMsgFilter();
  virtual ~nsMsgFilter ();

  NS_DECL_NSIMSGFILTER

  nsMsgFilterTypeType	GetType() {return m_type;}
  void	  SetType(nsMsgFilterTypeType	type) {m_type = type;}
  PRBool  GetEnabled() {return m_enabled;}
  void    SetFilterScript(nsCString *filterName) ;
  void    SetFilterList(nsIMsgFilterList* filterList);
  PRBool  IsRule() {return (m_type & (nsMsgFilterType::InboxRule |
                               nsMsgFilterType::NewsRule)) != 0;}

  PRBool  IsScript() {return (m_type &
                                  (nsMsgFilterType::InboxJavaScript |
                                   nsMsgFilterType::NewsJavaScript)) != 0;}

  // filing routines.
  nsresult  SaveToTextFile(nsIOFileStream *aStream);
  nsresult  SaveRule(nsIOFileStream *aStream);

  PRInt16   GetVersion();
#ifdef DEBUG
  void	    Dump();
#endif

  nsresult  ConvertMoveOrCopyToFolderValue(nsIMsgRuleAction *filterAction, nsCString &relativePath);
  static const char *GetActionStr(nsMsgRuleActionType action);
  static nsresult GetActionFilingStr(nsMsgRuleActionType action, nsCString &actionStr);
  static nsMsgRuleActionType GetActionForFilingStr(nsCString &actionStr);
  nsMsgRuleAction      m_action;
protected:
  nsMsgFilterTypeType m_type;
  nsString    m_filterName;
  nsCString   m_scriptFileName;	// iff this filter is a script.
  nsCString   m_description;
  nsCString   m_unparsedBuffer;

  PRPackedBool m_enabled;
  PRPackedBool m_temporary;
  PRPackedBool m_unparseable;
  nsIMsgFilterList *m_filterList;	/* owning filter list */
  nsCOMPtr<nsISupportsArray> m_termList;       /* linked list of criteria terms */
  nsCOMPtr<nsIMsgSearchScopeTerm> m_scope;         /* default for mail rules is inbox, but news rules could
                                                  have a newsgroup - LDAP would be invalid */
  nsCOMPtr<nsISupportsArray> m_actionList;
  nsMsgSearchBoolExpression *m_expressionTree;
};

#endif
