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

#ifndef nsIDTD_h___
#define nsIDTD_h___

/**
 * MODULE NOTES:
 * @update  gess 7/20/98
 *
 * This interface defines standard interface for DTD's. Note that this
 * isn't HTML specific. DTD's have several functions within the parser
 * system:
 *      1) To coordinate the consumption of an input stream via the
 *      parser
 *      2) To serve as proxy to represent the containment rules of the
 *      underlying document
 *      3) To offer autodetection services to the parser (mainly for doc
 *      conversion)
 * */

#include "nsISupports.h"
#include "nsStringGlue.h"
#include "prtypes.h"
#include "nsITokenizer.h"

#define NS_IDTD_IID \
{ 0xcc374204, 0xcea2, 0x41a2, \
  { 0xb2, 0x7f, 0x83, 0x75, 0xe2, 0xcf, 0x97, 0xcf } }


enum eAutoDetectResult {
    eUnknownDetect,
    eValidDetect,
    ePrimaryDetect,
    eInvalidDetect
};

enum nsDTDMode {
    eDTDMode_unknown = 0,
    eDTDMode_quirks,        //pre 4.0 versions
    eDTDMode_almost_standards,
    eDTDMode_full_standards,
    eDTDMode_autodetect,
    eDTDMode_fragment
};


class nsIParser;
class CToken;
class nsIURI;
class nsIContentSink;
class CParserContext;

class nsIDTD : public nsISupports
{
public:

    NS_DECLARE_STATIC_IID_ACCESSOR(NS_IDTD_IID)

    NS_IMETHOD WillBuildModel(const CParserContext& aParserContext,
                              nsITokenizer* aTokenizer,
                              nsIContentSink* aSink) = 0;

    /**
     * Called by the parser after the parsing process has concluded
     * @update  gess5/18/98
     * @param   anErrorCode - contains error code resulting from parse process
     * @return
     */
    NS_IMETHOD DidBuildModel(nsresult anErrorCode, PRBool aNotifySink,
                             nsIParser* aParser,
                             nsIContentSink* aSink) = 0;

    /**
     * Called by the parser after the parsing process has concluded
     * @update  gess5/18/98
     * @param   anErrorCode - contains error code resulting from parse process
     * @return
     */
    NS_IMETHOD BuildModel(nsIParser* aParser, nsITokenizer* aTokenizer,
                          nsITokenObserver* anObserver,
                          nsIContentSink* aSink) = 0;

    /**
     * Called during model building phase of parse process. Each token
     * created during the parse phase is stored in a deque (in the
     * parser) and are passed to this method so that the DTD can
     * process the token. Ultimately, the DTD will transform given
     * token into calls onto a contentsink.
     * @update  gess 3/25/98
     * @param   aToken -- token object to be put into content model
     * @return error code (usually 0)
     */
    NS_IMETHOD HandleToken(CToken* aToken,nsIParser* aParser) = 0;

    /**
     * If the parse process gets interrupted midway, this method is
     * called by the parser prior to resuming the process.
     * @update  gess5/18/98
     * @return ignored
     */
    NS_IMETHOD WillResumeParse(nsIContentSink* aSink) = 0;

    /**
     * If the parse process gets interrupted, this method is called by
     * the parser to notify the DTD that interruption will occur.
     * @update  gess5/18/98
     * @return ignored
     */
    NS_IMETHOD WillInterruptParse(nsIContentSink* aSink) = 0;

    /**
     * This method is called to determine whether or not a tag of one
     * type can contain a tag of another type.
     *
     * @update  gess 3/25/98
     * @param   aParent -- int tag of parent container
     * @param   aChild -- int tag of child container
     * @return PR_TRUE if parent can contain child
     */
    NS_IMETHOD_(PRBool) CanContain(PRInt32 aParent,PRInt32 aChild) const = 0;

    /**
     * This method gets called to determine whether a given
     * tag is itself a container
     *
     * @update  gess 3/25/98
     * @param   aTag -- tag to test for containership
     * @return  PR_TRUE if given tag can contain other tags
     */
    NS_IMETHOD_(PRBool) IsContainer(PRInt32 aTag) const = 0;

    /**
     * Use this id you want to stop the building content model
     * --------------[ Sets DTD to STOP mode ]----------------
     * It's recommended to use this method in accordance with
     * the parser's terminate() method.
     *
     * @update  harishd 07/22/99
     * @param
     * @return
     */
    NS_IMETHOD_(void) Terminate() = 0;

    NS_IMETHOD_(PRInt32) GetType() = 0;
};

NS_DEFINE_STATIC_IID_ACCESSOR(nsIDTD, NS_IDTD_IID)

#define NS_DECL_NSIDTD \
    NS_IMETHOD WillBuildModel(  const CParserContext& aParserContext, nsITokenizer* aTokenizer, nsIContentSink* aSink);\
    NS_IMETHOD DidBuildModel(nsresult anErrorCode,PRBool aNotifySink,nsIParser* aParser,nsIContentSink* aSink);\
    NS_IMETHOD BuildModel(nsIParser* aParser,nsITokenizer* aTokenizer,nsITokenObserver* anObserver,nsIContentSink* aSink);\
    NS_IMETHOD HandleToken(CToken* aToken,nsIParser* aParser);\
    NS_IMETHOD WillResumeParse(nsIContentSink* aSink = 0);\
    NS_IMETHOD WillInterruptParse(nsIContentSink* aSink = 0);\
    NS_IMETHOD_(PRBool) CanContain(PRInt32 aParent,PRInt32 aChild) const;\
    NS_IMETHOD_(PRBool) IsContainer(PRInt32 aTag) const;\
    NS_IMETHOD_(void)  Terminate();\
    NS_IMETHOD_(PRInt32) GetType();
#endif /* nsIDTD_h___ */
