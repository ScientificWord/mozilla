/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=2 sw=2 et tw=80: */
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


#include "nsIAtom.h"
#include "CParserContext.h"
#include "nsToken.h"
#include "prenv.h"  
#include "nsIHTMLContentSink.h"
#include "nsHTMLTokenizer.h"

CParserContext::CParserContext(nsScanner* aScanner, 
                               void *aKey, 
                               eParserCommands aCommand,
                               nsIRequestObserver* aListener, 
                               nsIDTD *aDTD, 
                               eAutoDetectResult aStatus, 
                               PRBool aCopyUnused)
  : mDTD(aDTD),
    mListener(aListener),
    mKey(aKey),
    mPrevContext(nsnull),
    mScanner(aScanner),
    mDTDMode(eDTDMode_unknown),
    mStreamListenerState(eNone),
    mContextType(eCTNone),
    mAutoDetectStatus(aStatus),
    mParserCommand(aCommand),
    mMultipart(PR_TRUE),
    mCopyUnused(aCopyUnused),
    mTransferBufferSize(eTransferBufferSize)
{ 
  MOZ_COUNT_CTOR(CParserContext); 
} 

CParserContext::~CParserContext()
{
  // It's ok to simply ingore the PrevContext.
  MOZ_COUNT_DTOR(CParserContext);
}

void
CParserContext::SetMimeType(const nsACString& aMimeType)
{
  mMimeType.Assign(aMimeType);

  mDocType = ePlainText;

  if (mMimeType.EqualsLiteral(kHTMLTextContentType))
    mDocType = eHTML_Strict;
  else if (mMimeType.EqualsLiteral(kXMLTextContentType)          ||
           mMimeType.EqualsLiteral(kXMLApplicationContentType)   ||
           mMimeType.EqualsLiteral(kXHTMLApplicationContentType) ||
           mMimeType.EqualsLiteral(kXULTextContentType)          ||
#ifdef MOZ_SVG
           mMimeType.EqualsLiteral(kSVGTextContentType)          ||
#endif
           mMimeType.EqualsLiteral(kRDFApplicationContentType)   ||
           mMimeType.EqualsLiteral(kRDFTextContentType))
    mDocType = eXML;
}

nsresult
CParserContext::GetTokenizer(PRInt32 aType,
                             nsIContentSink* aSink,
                             nsITokenizer*& aTokenizer)
{
  nsresult result = NS_OK;
  
  if (!mTokenizer) {
    if (aType == NS_IPARSER_FLAG_HTML || mParserCommand == eViewSource) {
      nsCOMPtr<nsIHTMLContentSink> theSink = do_QueryInterface(aSink);
      PRUint16 theFlags = 0;

      if (theSink) {
        // XXX This code is repeated both here and in CNavDTD. Can the two
        // callsites be combined?
        PRBool enabled;
        theSink->IsEnabled(eHTMLTag_frameset, &enabled);
        if(enabled) {
          theFlags |= NS_IPARSER_FLAG_FRAMES_ENABLED;
        }
        
        theSink->IsEnabled(eHTMLTag_script, &enabled);
        if(enabled) {
          theFlags |= NS_IPARSER_FLAG_SCRIPT_ENABLED;
        }
      }

      mTokenizer = new nsHTMLTokenizer(mDTDMode, mDocType,
                                       mParserCommand, theFlags);
      if (!mTokenizer) {
        return NS_ERROR_OUT_OF_MEMORY;
      }

      // Make sure the new tokenizer has all of the necessary information.
      // XXX this might not be necessary.
      if (mPrevContext) {
        mTokenizer->CopyState(mPrevContext->mTokenizer);
      }
    }
    else if (aType == NS_IPARSER_FLAG_XML) {
      mTokenizer = do_QueryInterface(mDTD, &result);
    }
  }
  
  aTokenizer = mTokenizer;

  return result;
}
