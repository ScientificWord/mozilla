/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set sw=2 ts=2 et tw=80: */
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
 * The Original Code is mozilla.com code.
 *
 * The Initial Developer of the Original Code is
 * Mozilla Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2006
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Johnny Stenback <jst@mozilla.com> (original author)
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

#ifndef __nsPIDOMStorage_h_
#define __nsPIDOMStorage_h_

#include "nsISupports.h"
#include "nsTArray.h"

class nsIDOMStorage;
class nsIURI;

#define NS_PIDOMSTORAGE_IID                                 \
  { 0x2fdbb82e, 0x4b47, 0x406a,                             \
      { 0xb1, 0x17, 0x6d, 0x67, 0x58, 0xc1, 0xee, 0x6b } }

class nsPIDOMStorage : public nsISupports
{
public:
  NS_DECLARE_STATIC_IID_ACCESSOR(NS_PIDOMSTORAGE_IID)

  virtual void Init(nsIURI* aURI, const nsAString &aDomain, PRBool aUseDB) = 0;

  virtual already_AddRefed<nsIDOMStorage> Clone(nsIURI* aURI) = 0;

  virtual nsTArray<nsString> *GetKeys() = 0;
};

NS_DEFINE_STATIC_IID_ACCESSOR(nsPIDOMStorage, NS_PIDOMSTORAGE_IID)

#endif // __nsPIDOMStorage_h_
