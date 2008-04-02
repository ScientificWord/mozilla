/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * ***** BEGIN LICENSE BLOCK *****
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
 * The Original Code is Novell code.
 *
 * The Initial Developer of the Original Code is Novell Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2006
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   robert@ocallahan.org
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

#include "nsTextFrameUtils.h"

#include "nsContentUtils.h"
#include "nsIWordBreaker.h"
#include "gfxFont.h"
#include "nsUnicharUtils.h"
#include "nsBidiUtils.h"

// XXX TODO implement transform of backslash to yen that nsTextTransform does
// when requested by PresContext->LanguageSpecificTransformType(). Do it with
// a new factory type that just munges the input stream. But first, check
// that we really still need this, it's only enabled via a hidden pref
// which defaults false...

#define UNICODE_ZWSP 0x200B
  
static PRBool IsDiscardable(PRUnichar ch, PRUint32* aFlags)
{
  // Unlike IS_DISCARDABLE, we don't discard \r. \r will be ignored by gfxTextRun
  // and discarding it would force us to copy text in many cases of preformatted
  // text containing \r\n.
  if (ch == CH_SHY) {
    *aFlags |= nsTextFrameUtils::TEXT_HAS_SHY;
    return PR_TRUE;
  }
  if ((ch & 0xFF00) != 0x2000) {
    // Not a Bidi control character
    return PR_FALSE;
  }
  return IS_BIDI_CONTROL_CHAR(ch);
}

static PRBool IsDiscardable(PRUint8 ch, PRUint32* aFlags)
{
  if (ch == CH_SHY) {
    *aFlags |= nsTextFrameUtils::TEXT_HAS_SHY;
    return PR_TRUE;
  }
  return PR_FALSE;
}

PRUnichar*
nsTextFrameUtils::TransformText(const PRUnichar* aText, PRUint32 aLength,
                                PRUnichar* aOutput,
                                PRBool aCompressWhitespace,
                                PRPackedBool* aIncomingWhitespace,
                                gfxSkipCharsBuilder* aSkipChars,
                                PRUint32* aAnalysisFlags)
{
  PRUint32 flags = 0;
  PRUnichar* outputStart = aOutput;

  if (!aCompressWhitespace) {
    // Skip discardables.
    PRUint32 i;
    for (i = 0; i < aLength; ++i) {
      PRUnichar ch = *aText++;
      if (IsDiscardable(ch, &flags)) {
        aSkipChars->SkipChar();
      } else {
        aSkipChars->KeepChar();
        if (ch == '\t') {
          flags |= TEXT_HAS_TAB;
        }
        *aOutput++ = ch;
      }
    }
    *aIncomingWhitespace = PR_FALSE;
  } else {
    PRBool inWhitespace = *aIncomingWhitespace;
    PRUint32 i;
    for (i = 0; i < aLength; ++i) {
      PRUnichar ch = *aText++;
      PRBool nowInWhitespace;
      if (ch == ' ' &&
          (i + 1 >= aLength ||
           !IsSpaceCombiningSequenceTail(aText, aLength - (i + 1)))) {
        nowInWhitespace = PR_TRUE;
      } else if (ch == '\n') {
        if (i > 0 && IS_CJ_CHAR(aText[-1]) &&
            i + 1 < aLength && IS_CJ_CHAR(aText[1])) {
          // Discard newlines between CJK chars.
          // XXX this really requires more context to get right!
          aSkipChars->SkipChar();
          continue;
        }
        nowInWhitespace = PR_TRUE;
      } else {
        nowInWhitespace = ch == '\t';
      }

      if (!nowInWhitespace) {
        if (IsDiscardable(ch, &flags)) {
          aSkipChars->SkipChar();
          nowInWhitespace = inWhitespace;
        } else {
          *aOutput++ = ch;
          aSkipChars->KeepChar();
        }
      } else {
        if (inWhitespace) {
          aSkipChars->SkipChar();
        } else {
          if (ch != ' ') {
            flags |= TEXT_WAS_TRANSFORMED;
          }
          *aOutput++ = ' ';
          aSkipChars->KeepChar();
        }
      }
      inWhitespace = nowInWhitespace;
    }
    *aIncomingWhitespace = inWhitespace;
  }

  if (outputStart + aLength != aOutput) {
    flags |= TEXT_WAS_TRANSFORMED;
  }
  *aAnalysisFlags = flags;
  return aOutput;
}

PRUint8*
nsTextFrameUtils::TransformText(const PRUint8* aText, PRUint32 aLength,
                                PRUint8* aOutput,
                                PRBool aCompressWhitespace,
                                PRPackedBool* aIncomingWhitespace,
                                gfxSkipCharsBuilder* aSkipChars,
                                PRUint32* aAnalysisFlags)
{
  PRUint32 flags = 0;
  PRUint8* outputStart = aOutput;

  if (!aCompressWhitespace) {
    // Skip discardables.
    PRUint32 i;
    for (i = 0; i < aLength; ++i) {
      PRUint8 ch = *aText++;
      if (IsDiscardable(ch, &flags)) {
        aSkipChars->SkipChar();
      } else {
        aSkipChars->KeepChar();
        if (ch == '\t') {
          flags |= TEXT_HAS_TAB;
        }
        *aOutput++ = ch;
      }
    }
    *aIncomingWhitespace = PR_FALSE;
  } else {
    PRBool inWhitespace = *aIncomingWhitespace;
    PRUint32 i;
    for (i = 0; i < aLength; ++i) {
      PRUint8 ch = *aText++;
      PRBool nowInWhitespace = ch == ' ' || ch == '\t' || ch == '\n' || ch == '\f';
      if (!nowInWhitespace) {
        if (IsDiscardable(ch, &flags)) {
          aSkipChars->SkipChar();
          nowInWhitespace = inWhitespace;
        } else {
          *aOutput++ = ch;
          aSkipChars->KeepChar();
        }
      } else {
        if (inWhitespace) {
          aSkipChars->SkipChar();
        } else {
          if (ch != ' ') {
            flags |= TEXT_WAS_TRANSFORMED;
          }
          *aOutput++ = ' ';
          aSkipChars->KeepChar();
        }
      }
      inWhitespace = nowInWhitespace;
    }
    *aIncomingWhitespace = inWhitespace;
  }

  if (outputStart + aLength != aOutput) {
    flags |= TEXT_WAS_TRANSFORMED;
  }
  *aAnalysisFlags = flags;
  return aOutput;
}

PRBool nsSkipCharsRunIterator::NextRun() {
  do {
    if (mRunLength) {
      mIterator.AdvanceOriginal(mRunLength);
      NS_ASSERTION(mRunLength > 0, "No characters in run (initial length too large?)");
      if (!mSkipped || mLengthIncludesSkipped) {
        mRemainingLength -= mRunLength;
      }
    }
    if (!mRemainingLength)
      return PR_FALSE;
    PRInt32 length;
    mSkipped = mIterator.IsOriginalCharSkipped(&length);
    mRunLength = PR_MIN(length, mRemainingLength);
  } while (!mVisitSkipped && mSkipped);

  return PR_TRUE;
}
