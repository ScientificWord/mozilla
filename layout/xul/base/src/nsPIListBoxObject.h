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
 * The Original Code is Mozilla.org code.
 *
 * The Initial Developer of the Original Code is Mozilla.org.
 * Portions created by the Initial Developer are Copyright (C) 2006
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *     Boris Zbarsky <bzbarsky@mit.edu> (Original Author)
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

#ifndef nsPIListBoxObject_h__
#define nsPIListBoxObject_h__

#define NS_PILISTBOXOBJECT_IID \
{ 0x294e0820, 0x5c40, 0x42a8, \
 { 0xb3, 0x2f, 0x6d, 0xcf, 0x05, 0xfb, 0xe1, 0xf3 } }

#include "nsIListBoxObject.h"

class nsPIListBoxObject : public nsIListBoxObject {
 public:
  NS_DECLARE_STATIC_IID_ACCESSOR(NS_PILISTBOXOBJECT_IID)

  /**
   * Clear the cached list box body frame from this box object.  This should be
   * called when the frame in question is destroyed.
  */
  virtual void ClearCachedListBoxBody() = 0;

  /*
   * Get the list box body.  This will search for it as needed.
   */
  virtual nsIListBoxObject* GetListBoxBody() = 0;
};

NS_DEFINE_STATIC_IID_ACCESSOR(nsPIListBoxObject, NS_PILISTBOXOBJECT_IID)

#endif // nsPIListBoxObject_h__
