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
 * The Original Code is Unicode case conversion helpers.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corp..
 * Portions created by the Initial Developer are Copyright (C) 2002
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Alec Flett <alecf@netscape.com>
 *   Benjamin Smedberg <benjamin@smedbergs.us>
 *   Ben Turner <mozilla@songbirdnest.com>
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

#include "nsUnicharUtils.h"
#include "nsUnicharUtilCIID.h"

#include "nsCRT.h"
#include "nsICaseConversion.h"
#include "nsServiceManagerUtils.h"
#include "nsXPCOMStrings.h"

#include <ctype.h>

static nsICaseConversion* gCaseConv = nsnull;

nsICaseConversion*
NS_GetCaseConversion()
{
  if (!gCaseConv) {
    nsresult rv = CallGetService(NS_UNICHARUTIL_CONTRACTID, &gCaseConv);
    if (NS_FAILED(rv)) {
      NS_ERROR("Failed to get the case conversion service!");
      gCaseConv = nsnull;
    }
  }
  return gCaseConv;
}

void
ToLowerCase(nsAString& aString)
{
  nsICaseConversion* caseConv = NS_GetCaseConversion();
  if (caseConv) {
    PRUnichar *buf = aString.BeginWriting();
    caseConv->ToLower(buf, buf, aString.Length());
  }
  else
    NS_WARNING("No case converter: no conversion done");
}

void
ToLowerCase(const nsAString& aSource,
            nsAString& aDest)
{
  const PRUnichar *in;
  PRUint32 len = NS_StringGetData(aSource, &in);

  PRUnichar *out;
  NS_StringGetMutableData(aDest, len, &out);

  nsICaseConversion* caseConv = NS_GetCaseConversion();
  if (out && caseConv)
    caseConv->ToLower(in, out, len);
  else {
    NS_WARNING("No case converter: only copying");
    aDest.Assign(aSource);
  }
}

void
ToUpperCase(nsAString& aString)
{
  nsICaseConversion* caseConv = NS_GetCaseConversion();
  if (caseConv) {
    PRUnichar *buf = aString.BeginWriting();
    caseConv->ToUpper(buf, buf, aString.Length());
  }
  else
    NS_WARNING("No case converter: no conversion done");
}

void
ToUpperCase(const nsAString& aSource,
            nsAString& aDest)
{
  const PRUnichar *in;
  PRUint32 len = NS_StringGetData(aSource, &in);

  PRUnichar *out;
  NS_StringGetMutableData(aDest, len, &out);

  nsICaseConversion* caseConv = NS_GetCaseConversion();
  if (out && caseConv)
    caseConv->ToUpper(in, out, len);
  else {
    NS_WARNING("No case converter: only copying");
    aDest.Assign(aSource);
  }
}

#ifdef MOZILLA_INTERNAL_API

PRInt32
nsCaseInsensitiveStringComparator::operator()(const PRUnichar* lhs,
                                              const PRUnichar* rhs,
                                              PRUint32 aLength) const
{
  PRInt32 result;
  nsICaseConversion* caseConv = NS_GetCaseConversion();
  if (caseConv)
    caseConv->CaseInsensitiveCompare(lhs, rhs, aLength, &result);
  else {
    NS_WARNING("No case converter: using default");
    nsDefaultStringComparator comparator;
    result = comparator(lhs, rhs, aLength);
  }
  return result;
}

PRInt32
nsCaseInsensitiveStringComparator::operator()(PRUnichar lhs,
                                              PRUnichar rhs) const
{
  // see if they're an exact match first
  if (lhs == rhs)
    return 0;
  
  nsICaseConversion* caseConv = NS_GetCaseConversion();
  if (caseConv) {
    caseConv->ToLower(lhs, &lhs);
    caseConv->ToLower(rhs, &rhs);
  }
  else {
    if (lhs < 256)
      lhs = tolower(char(lhs));
    if (rhs < 256)
      rhs = tolower(char(rhs));
    NS_WARNING("No case converter: no conversion done");
  }
  
  if (lhs == rhs)
    return 0;
  else if (lhs < rhs)
    return -1;
  else
    return 1;
}

#else // MOZILLA_INTERNAL_API

PRInt32
CaseInsensitiveCompare(const PRUnichar *a,
                       const PRUnichar *b,
                       PRUint32 len)
{
  nsICaseConversion* caseConv = NS_GetCaseConversion();
  if (!caseConv)
    return NS_strcmp(a, b);

  PRInt32 result;
  caseConv->CaseInsensitiveCompare(a, b, len, &result);
  return result;
}

#endif // MOZILLA_INTERNAL_API

PRUnichar
ToLowerCase(PRUnichar aChar)
{
  PRUnichar result;
  nsICaseConversion* caseConv = NS_GetCaseConversion();
  if (caseConv)
    caseConv->ToLower(aChar, &result);
  else {
    NS_WARNING("No case converter: no conversion done");
    if (aChar < 256)
      result = tolower(char(aChar));
    else
      result = aChar;
  }
  return result;
}

PRUnichar
ToUpperCase(PRUnichar aChar)
{
  PRUnichar result;
  nsICaseConversion* caseConv = NS_GetCaseConversion();
  if (caseConv)
    caseConv->ToUpper(aChar, &result);
  else {
    NS_WARNING("No case converter: no conversion done");
    if (aChar < 256)
      result = toupper(char(aChar));
    else
      result = aChar;
  }
  return result;
}
