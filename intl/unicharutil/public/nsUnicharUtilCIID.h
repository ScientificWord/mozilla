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
#ifndef nsUnicharUtilCIID_h__
#define nsUnicharUtilCIID_h__


#include "nsISupports.h"
#include "nscore.h"

#define NS_UNICHARUTIL_CID \
{ 0xcc10c750, 0x9ec3, 0x11d2, \
  { 0xb3, 0xae, 0x0, 0x80, 0x5f, 0x8a, 0x66, 0x70 } }

#define NS_UNICHARUTIL_CONTRACTID "@mozilla.org/intl/unicharutil;1"

#define NS_UNICHARCATEGORY_CID \
{ 0x748a1132, 0x671a, 0x409a, \
  { 0x8d, 0x1d, 0xf1, 0xcd, 0xf6, 0xb3, 0xa6, 0xb4 } }

#define NS_UNICHARCATEGORY_CONTRACTID "@mozilla.org/intl/unicharcategory;1"

#endif
