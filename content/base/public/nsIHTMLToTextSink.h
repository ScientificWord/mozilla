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

#ifndef _nsIPlainTextSink_h__
#define _nsIPlainTextSink_h__

#include "nsISupports.h"
#include "nsStringGlue.h"

#define NS_PLAINTEXTSINK_CONTRACTID "@mozilla.org/layout/plaintextsink;1"

/* starting interface:    nsIContentSerializer */
#define NS_IHTMLTOTEXTSINK_IID_STR "b12b5643-07cb-401e-aabb-64b2dcd2717f"

#define NS_IHTMLTOTEXTSINK_IID \
  {0xb12b5643, 0x07cb, 0x401e, \
    { 0xaa, 0xbb, 0x64, 0xb2, 0xdc, 0xd2, 0x71, 0x7f }}


class nsIHTMLToTextSink : public nsISupports {
 public: 

  NS_DECLARE_STATIC_IID_ACCESSOR(NS_IHTMLTOTEXTSINK_IID)

  NS_IMETHOD Initialize(nsAString* aOutString,
                        PRUint32 aFlags, PRUint32 aWrapCol) = 0;
     // This function violates string ownership rules, see impl.
};

NS_DEFINE_STATIC_IID_ACCESSOR(nsIHTMLToTextSink, NS_IHTMLTOTEXTSINK_IID)

#endif
