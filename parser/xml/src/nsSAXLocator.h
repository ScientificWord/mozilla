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
 * The Initial Developer of the Original Code is Robert Sayre.
 *
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
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

#ifndef nsSAXLocator_h__
#define nsSAXLocator_h__

#include "nsISAXLocator.h"
#include "nsString.h"

#define NS_SAXLOCATOR_CONTRACTID "@mozilla.org/saxparser/locator;1"
#define NS_SAXLOCATOR_CLASSNAME "SAX Locator"
#define NS_SAXLOCATOR_CID  \
{/* {c1cd4045-846b-43bb-a95e-745a3d7b40e0}*/ \
0xc1cd4045, 0x846b, 0x43bb, \
{ 0xa9, 0x5e, 0x74, 0x5a, 0x3d, 0x7b, 0x40, 0xe0} }

class nsSAXLocator : public nsISAXLocator
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSISAXLOCATOR

  nsSAXLocator();

  NS_IMETHOD SetColumnNumber(PRInt32 aColumnNumber);
  NS_IMETHOD SetLineNumber(PRInt32 aLineNumber);
  NS_IMETHOD SetSystemId(const nsAString & aSystemId);
  NS_IMETHOD SetPublicId(const nsAString & aPublicId);

private:
  nsString mPublicId;
  nsString mSystemId;
  PRInt32 mLineNumber;
  PRInt32 mColumnNumber;
};

#endif //nsSAXLocator_h__
