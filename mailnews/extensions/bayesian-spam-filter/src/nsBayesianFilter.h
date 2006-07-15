/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
 * Portions created by the Initial Developer are Copyright (C) 2002
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Patrick C. Beard <beard@netscape.com>
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

#ifndef nsBayesianFilter_h__
#define nsBayesianFilter_h__

#include "nsCOMPtr.h"
#include "nsIMsgFilterPlugin.h"
#include "nsISemanticUnitScanner.h"
#include "pldhash.h"
#include "nsITimer.h"

// XXX can't simply byte align arenas, must at least 2-byte align.
#define PL_ARENA_CONST_ALIGN_MASK 1
#include "plarena.h"

#define DEFAULT_MIN_INTERVAL_BETWEEN_WRITES             15*60*1000
#define DEFAULT_WRITE_TRAINING_DATA_MESSAGES_THRESHOLD  50

struct Token;
class TokenEnumeration;
class TokenAnalyzer;
class nsIMsgWindow;
class nsIMimeHeaders;
class nsIUTF8StringEnumerator;

/**
 * Helper class to enumerate Token objects in a PLDHashTable
 * safely and without copying (see bugzilla #174859). The
 * enumeration is safe to use until a PL_DHASH_ADD
 * or PL_DHASH_REMOVE is performed on the table.
 */
class TokenEnumeration {
public:
    TokenEnumeration(PLDHashTable* table);
    PRBool hasMoreTokens();
    Token* nextToken();
    
private:
    PRUint32 mEntrySize, mEntryCount, mEntryOffset;
    char *mEntryAddr, *mEntryLimit;
};

class Tokenizer {
public:
    Tokenizer();
    ~Tokenizer();

    operator int() { return mTokenTable.entryStore != NULL; }
    
    Token* get(const char* word);

    // The training set keeps an occurrence count on each word. This count 
    // is supposed to count the # of messsages it occurs in.
    // When add/remove is called while tokenizing a message and NOT the training set,
    // 
    Token* add(const char* word, PRUint32 count = 1);
    void remove(const char* word, PRUint32 count = 1);
    
    PRUint32 countTokens();
    Token* copyTokens();
    TokenEnumeration getTokens();

    /**
     * Clears out the previous message tokens.
     */
    nsresult clearTokens();

    /**
     * Assumes that text is mutable and
     * can be nsCRT::strtok'd.
     */
    void tokenize(char* text);
    
    /**
     * Copies the string before tokenizing.
     */
    void tokenize(const char* str);
    
    /**
     *  Creates specific tokens based on the mime headers for the message being tokenized
     */
    void tokenizeHeaders(nsIUTF8StringEnumerator * aHeaderNames, nsIUTF8StringEnumerator * aHeaderValues);

    void tokenizeAttachment(const char * aContentType, const char * aFileName);

    /**
     * Calls passed-in function for each token in the table.
     */
    void visit(PRBool (*f) (Token*, void*), void* data);

private:
    char* copyWord(const char* word, PRUint32 len);
    void tokenize_ascii_word(char * word);
    void tokenize_japanese_word(char* chunk);
    inline void addTokenForHeader(const char * aTokenPrefix, nsACString& aValue, PRBool aTokenizeValue = false);
    nsresult stripHTML(const nsAString& inString, nsAString& outString);

private:
    PLDHashTable mTokenTable;
    PLArenaPool mWordPool;
    nsCOMPtr<nsISemanticUnitScanner> mScanner;
};

class nsBayesianFilter : public nsIJunkMailPlugin {
public:
    NS_DECL_ISUPPORTS
    NS_DECL_NSIMSGFILTERPLUGIN
    NS_DECL_NSIJUNKMAILPLUGIN
    
    nsBayesianFilter();
    virtual ~nsBayesianFilter();
    
    nsresult tokenizeMessage(const char* messageURI, nsIMsgWindow *aMsgWindow, TokenAnalyzer* analyzer);
    void classifyMessage(Tokenizer& tokens, const char* messageURI, nsIJunkMailClassificationListener* listener);
    void observeMessage(Tokenizer& tokens, const char* messageURI, nsMsgJunkStatus oldClassification, nsMsgJunkStatus newClassification, 
                        nsIJunkMailClassificationListener* listener);

    void writeTrainingData();
    void readTrainingData();
    nsresult getTrainingFile(nsILocalFile ** aFile);
    
protected:

    static void TimerCallback(nsITimer* aTimer, void* aClosure);

    Tokenizer mGoodTokens, mBadTokens;
    double   mJunkProbabilityThreshold;
    PRUint32 mGoodCount, mBadCount;
    PRInt32 mDirtyingMessageWriteThreshold; // ... before flushing training data
    PRInt32 mNumDirtyingMessages; // must be positive
    PRInt32 mMinFlushInterval; // in miliseconds, must be positive
                               //and not too close to 0
    nsCOMPtr<nsITimer> mTimer;
    nsCOMPtr<nsILocalFile> mTrainingFile;
};

#endif // _nsBayesianFilter_h__
