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
 * The Original Code is the Mozilla SVG project.
 *
 * The Initial Developer of the Original Code is
 * IBM Corporation
 * Portions created by the Initial Developer are Copyright (C) 2006
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

#include "nsSVGTransformListParser.h"
#include "prdtoa.h"
#include "nsSVGTransform.h"
#include "nsSVGMatrix.h"
#include "nsDOMError.h"
#include "nsGkAtoms.h"
#include "nsReadableUtils.h"
#include "nsCRT.h"
#include "nsCOMArray.h"
#include "nsContentUtils.h"
#include "nsIDOMClassInfo.h"

//----------------------------------------------------------------------
// public interface

nsSVGTransformListParser::nsSVGTransformListParser(nsCOMArray<nsIDOMSVGTransform>* aTransforms)
    : mTransform(aTransforms)
{
}

//----------------------------------------------------------------------
// private methods

nsresult
nsSVGTransformListParser::Match()
{
  return MatchTransformList();
}


nsresult
nsSVGTransformListParser::MatchTransformList()
{
  MatchWsp();

  if (IsTokenTransformStarter()) {
    ENSURE_MATCHED(MatchTransforms());
  }

  MatchWsp();

  return NS_OK;
}


nsresult
nsSVGTransformListParser::MatchTransforms()
{
  ENSURE_MATCHED(MatchTransform());

  while (mTokenType != END) {
    const char* pos = mTokenPos;

    /* Curiously the SVG BNF allows multiple comma-wsp between transforms */
    while (IsTokenCommaWspStarter()) {
      ENSURE_MATCHED(MatchCommaWsp());
    }

    if (IsTokenTransformStarter()) {
      ENSURE_MATCHED(MatchTransform());
    }
    else {
      if (pos != mTokenPos) RewindTo(pos);
      break;
    }
  }

  return NS_OK;
}


nsresult
nsSVGTransformListParser::GetTransformToken(nsIAtom** aKeyAtom,
                                            PRBool aAdvancePos)
{
  if (mTokenType != OTHER || *mTokenPos == '\0') {
     return NS_ERROR_FAILURE;
  }

  nsresult rv = NS_OK;

  const char* delimiters = "\x20\x9\xD\xA,(";
  char* delimiterStart = PL_strnpbrk(mTokenPos, delimiters, 11);
  if (delimiterStart != 0) {
    /* save this character and null it out */
    char holdingChar = *delimiterStart;
    *delimiterStart = '\0';

    if (mTokenPos != 0 && nsCRT::strlen(mTokenPos) > 0) {
      *aKeyAtom = NS_NewAtom(mTokenPos);

      if (aAdvancePos) {
         mInputPos = mTokenPos + nsCRT::strlen(mTokenPos);
         mTokenPos = mInputPos;
      }
    }
    else {
      rv = NS_ERROR_FAILURE;
    }
    /* reset character back to original */
    *delimiterStart = holdingChar;
  }
  else {
    rv = NS_ERROR_FAILURE;
  }

  return rv;
}


nsresult
nsSVGTransformListParser::MatchTransform()
{
  nsCOMPtr<nsIAtom> keyatom;

  nsresult rv = GetTransformToken(getter_AddRefs(keyatom), PR_TRUE);
  NS_ENSURE_SUCCESS(rv, rv);

  if (keyatom == nsGkAtoms::translate) {
     ENSURE_MATCHED(MatchTranslate());
  }
  else if (keyatom == nsGkAtoms::scale) {
     ENSURE_MATCHED(MatchScale());
  }
  else if (keyatom == nsGkAtoms::rotate) {
     ENSURE_MATCHED(MatchRotate());
  }
  else if (keyatom == nsGkAtoms::skewX) {
      ENSURE_MATCHED(MatchSkewX());
  }
  else if (keyatom == nsGkAtoms::skewY) {
     ENSURE_MATCHED(MatchSkewY());
   }
  else if (keyatom == nsGkAtoms::matrix) {
     ENSURE_MATCHED(MatchMatrix());
  }
  else {
     return NS_ERROR_FAILURE;
  }

  return NS_OK;
}


PRBool
nsSVGTransformListParser::IsTokenTransformStarter()
{
  nsCOMPtr<nsIAtom> keyatom;

  nsresult rv = GetTransformToken(getter_AddRefs(keyatom), PR_FALSE);
  NS_ENSURE_SUCCESS(rv, PR_FALSE);

  if (keyatom == nsGkAtoms::translate ||
      keyatom == nsGkAtoms::scale     ||
      keyatom == nsGkAtoms::rotate    ||
      keyatom == nsGkAtoms::skewX     ||
      keyatom == nsGkAtoms::skewY     ||
      keyatom == nsGkAtoms::matrix) {
      return PR_TRUE;
  }

  return PR_FALSE;
}

nsresult
nsSVGTransformListParser::MatchNumberArguments(float *aResult,
                                               PRUint32 aMaxNum,
                                               PRUint32 *aParsedNum)
{
  *aParsedNum = 0;

  MatchWsp();

  ENSURE_MATCHED(MatchLeftParen());

  MatchWsp();

  ENSURE_MATCHED(MatchNumber(&aResult[0]));
  *aParsedNum = 1;

  while (IsTokenCommaWspStarter()) {
    MatchWsp();
    if (mTokenType == RIGHT_PAREN) {
      break;
    }
    if (*aParsedNum == aMaxNum) {
      return NS_ERROR_FAILURE;
    }
    if (IsTokenCommaWspStarter()) {
      MatchCommaWsp();
    }
    ENSURE_MATCHED(MatchNumber(&aResult[(*aParsedNum)++]));
  }

  MatchWsp();

  ENSURE_MATCHED(MatchRightParen());

  return NS_OK;
}

nsIDOMSVGTransform *
nsSVGTransformListParser::AppendTransform()
{
  nsCOMPtr<nsIDOMSVGTransform> transform;
  NS_NewSVGTransform(getter_AddRefs(transform));
  if (transform) {
    mTransform->AppendObject(transform);
  }
  return transform;
}

nsresult
nsSVGTransformListParser::MatchTranslate()
{
  GetNextToken();

  float t[2];
  PRUint32 count;

  ENSURE_MATCHED(MatchNumberArguments(t, NS_ARRAY_LENGTH(t), &count));

  switch (count) {
    case 1:
      t[1] = 0.f;
      // fall-through
    case 2:
    {
      nsIDOMSVGTransform *transform = AppendTransform();
      NS_ENSURE_TRUE(transform, NS_ERROR_OUT_OF_MEMORY);
      transform->SetTranslate(t[0], t[1]);
      break;
    }
    default:
      return NS_ERROR_FAILURE;
  }

  return NS_OK;
}


nsresult
nsSVGTransformListParser::MatchScale()
{
  GetNextToken();

  float s[2];
  PRUint32 count;

  ENSURE_MATCHED(MatchNumberArguments(s, NS_ARRAY_LENGTH(s), &count));

  switch (count) {
    case 1:
      s[1] = s[0];
      // fall-through
    case 2:
    {
      nsIDOMSVGTransform *transform = AppendTransform();
      NS_ENSURE_TRUE(transform, NS_ERROR_OUT_OF_MEMORY);
      transform->SetScale(s[0], s[1]);
      break;
    }
    default:
      return NS_ERROR_FAILURE;
  }

  return NS_OK;
}


nsresult
nsSVGTransformListParser::MatchRotate()
{
  GetNextToken();

  float r[3];
  PRUint32 count;

  ENSURE_MATCHED(MatchNumberArguments(r, NS_ARRAY_LENGTH(r), &count));

  switch (count) {
    case 1:
      r[1] = r[2] = 0.f;
      // fall-through
    case 3:
    {
      nsIDOMSVGTransform *transform = AppendTransform();
      NS_ENSURE_TRUE(transform, NS_ERROR_OUT_OF_MEMORY);
      transform->SetRotate(r[0], r[1], r[2]);
      break;
    }
    default:
      return NS_ERROR_FAILURE;
  }

  return NS_OK;
}


nsresult
nsSVGTransformListParser::MatchSkewX()
{
  GetNextToken();

  float skew;
  PRUint32 count;

  ENSURE_MATCHED(MatchNumberArguments(&skew, 1, &count));

  if (count != 1) {
    return NS_ERROR_FAILURE;
  }

  nsIDOMSVGTransform *transform = AppendTransform();
  NS_ENSURE_TRUE(transform, NS_ERROR_OUT_OF_MEMORY);
  transform->SetSkewX(skew);

  return NS_OK;
}


nsresult
nsSVGTransformListParser::MatchSkewY()
{
  GetNextToken();

  float skew;
  PRUint32 count;

  ENSURE_MATCHED(MatchNumberArguments(&skew, 1, &count));

  if (count != 1) {
    return NS_ERROR_FAILURE;
  }

  nsIDOMSVGTransform *transform = AppendTransform();
  NS_ENSURE_TRUE(transform, NS_ERROR_OUT_OF_MEMORY);
  transform->SetSkewY(skew);

  return NS_OK;
}


nsresult
nsSVGTransformListParser::MatchMatrix()
{
  GetNextToken();

  float m[6];
  PRUint32 count;

  ENSURE_MATCHED(MatchNumberArguments(m, NS_ARRAY_LENGTH(m), &count));

  if (count != 6) {
    return NS_ERROR_FAILURE;
  }

  nsCOMPtr<nsIDOMSVGMatrix> matrix;
  NS_NewSVGMatrix(getter_AddRefs(matrix),
                  m[0], m[1], m[2], m[3], m[4], m[5]);
  NS_ENSURE_TRUE(matrix, NS_ERROR_OUT_OF_MEMORY);

  nsIDOMSVGTransform *transform = AppendTransform();
  NS_ENSURE_TRUE(transform, NS_ERROR_OUT_OF_MEMORY);
  transform->SetMatrix(matrix);

  return NS_OK;
}
