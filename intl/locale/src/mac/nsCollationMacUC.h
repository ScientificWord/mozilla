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
 *   Pierre Phaneuf <pp@ludusdesign.com>
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

#ifndef nsCollationMacUC_h__
#define nsCollationMacUC_h__

#include "nsICollation.h"
#include "nsCollation.h"  // static library
#include <MacLocales.h>
#include <UnicodeUtilities.h>

// Maximum number of characters for a buffer to remember 
// the generated collation key.
const PRUint32 kCacheSize = 128;
// According to the documentation, the length of the key should typically be
// at least 5 * textLength
const PRUint32 kCollationValueSizeFactor = 5;

class nsCollationMacUC : public nsICollation {

public: 
  nsCollationMacUC();
  ~nsCollationMacUC(); 

  // nsISupports interface
  NS_DECL_ISUPPORTS

  // nsICollation interface
  NS_DECL_NSICOLLATION

protected:
  nsresult ConvertLocale(nsILocale* aNSLocale, LocaleRef* aMacLocale);
  nsresult StrengthToOptions(const PRInt32 aStrength,
                             UCCollateOptions* aOptions);
  nsresult EnsureCollator(const PRInt32 newStrength);

private:
  PRPackedBool mInit;
  PRPackedBool mHasCollator;
  LocaleRef mLocale;
  PRInt32 mLastStrength;
  CollatorRef mCollator;
  void *mBuffer; // temporary buffer to generate collation keys
  PRUint32 mBufferLen; // byte length of buffer
};

#endif  /* nsCollationMacUC_h__ */
