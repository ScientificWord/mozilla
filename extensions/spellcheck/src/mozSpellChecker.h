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
 * The Original Code is Mozilla Spellchecker Component.
 *
 * The Initial Developer of the Original Code is
 * David Einstein.
 * Portions created by the Initial Developer are Copyright (C) 2001
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s): David Einstein Deinst@world.std.com
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

#ifndef mozSpellChecker_h__
#define mozSpellChecker_h__

#include "nsCOMPtr.h"
#include "nsISpellChecker.h"
#include "nsString.h"
#include "nsITextServicesDocument.h"
#include "mozIPersonalDictionary.h"
#include "mozISpellCheckingEngine.h"
#include "nsVoidArray.h"
#include "mozISpellI18NUtil.h"

class mozSpellChecker : public nsISpellChecker
{
public:
  NS_DECL_ISUPPORTS

  mozSpellChecker();
  virtual ~mozSpellChecker();

  nsresult Init();

  // nsISpellChecker
  NS_IMETHOD SetDocument(nsITextServicesDocument *aDoc, PRBool aFromStartofDoc);
  NS_IMETHOD NextMisspelledWord(nsAString &aWord, nsStringArray *aSuggestions);
  NS_IMETHOD CheckWord(const nsAString &aWord, PRBool *aIsMisspelled, nsStringArray *aSuggestions);
  NS_IMETHOD Replace(const nsAString &aOldWord, const nsAString &aNewWord, PRBool aAllOccurrences);
  NS_IMETHOD IgnoreAll(const nsAString &aWord);

  NS_IMETHOD AddWordToPersonalDictionary(const nsAString &aWord);
  NS_IMETHOD RemoveWordFromPersonalDictionary(const nsAString &aWord);
  NS_IMETHOD GetPersonalDictionary(nsStringArray *aWordList);

  NS_IMETHOD GetDictionaryList(nsStringArray *aDictionaryList);
  NS_IMETHOD GetCurrentDictionary(nsAString &aDictionary);
  NS_IMETHOD SetCurrentDictionary(const nsAString &aDictionary);

protected:
  nsCOMPtr<mozISpellI18NUtil> mConverter;
  nsCOMPtr<nsITextServicesDocument> mTsDoc;
  nsCOMPtr<mozIPersonalDictionary> mPersonalDictionary;
  nsString mDictionaryName;
  nsCOMPtr<mozISpellCheckingEngine>  mSpellCheckingEngine;
  PRBool mFromStart;
  nsStringArray mIgnoreList;

  nsresult SetupDoc(PRUint32 *outBlockOffset);

  nsresult GetCurrentBlockIndex(nsITextServicesDocument *aDoc, PRInt32 *outBlockIndex);
};
#endif // mozSpellChecker_h__
