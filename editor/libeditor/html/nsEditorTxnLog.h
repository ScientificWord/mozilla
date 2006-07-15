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

#ifndef nsEditorTxnLog_h__
#define nsEditorTxnLog_h__

#include "nsITransaction.h"
#include "nsITransactionManager.h"
#include "nsITransactionListener.h"

class nsHTMLEditorLog;

/** implementation of a transaction listener object.
 *
 */
class nsEditorTxnLog : public nsITransactionListener
{
private:

  nsHTMLEditorLog *mEditorLog;
  PRInt32 mIndentLevel;
  PRInt32 mBatchCount;

public:

  /** The default constructor.
   */
  nsEditorTxnLog(nsHTMLEditorLog *aEditorLog=0);

  /** The default destructor.
   */
  virtual ~nsEditorTxnLog();

  /* Macro for AddRef(), Release(), and QueryInterface() */
  NS_DECL_ISUPPORTS

  /* nsITransactionListener method implementations. */
  NS_DECL_NSITRANSACTIONLISTENER

private:

  /* nsEditorTxnLog private methods. */
  nsresult PrintIndent(PRInt32 aIndentLevel);
  nsresult Write(const char *aBuffer);
  nsresult WriteInt(PRInt32 aInt);
  nsresult WriteTransaction(nsITransaction *aTransaction);
  nsresult Flush();
};

#endif // nsEditorTxnLog_h__
