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
 * The Original Code is Mozilla Communicator client code.
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

#ifndef nsPIBoxObject_h___
#define nsPIBoxObject_h___

// {91f2b229-b139-460e-a508-db9d52e9ed87}
#define NS_PIBOXOBJECT_IID \
{ 0x91f2b229, 0xb139, 0x460e, \
 { 0xa5, 0x08, 0xdb, 0x9d, 0x52, 0xe9, 0xed, 0x87 } }

class nsIPresShell;
class nsIContent;
class nsIDocument;

class nsPIBoxObject : public nsIBoxObject
{
public:
  NS_DECLARE_STATIC_IID_ACCESSOR(NS_PIBOXOBJECT_IID)

  virtual void Init(nsIContent* aContent) = 0;

  // Drop the weak ref to the content node as needed
  virtual void Clear() = 0;
};

NS_DEFINE_STATIC_IID_ACCESSOR(nsPIBoxObject, NS_PIBOXOBJECT_IID)

#endif

