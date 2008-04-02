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

#ifndef nsIMenuFrame_h___
#define nsIMenuFrame_h___

#include "nsISupports.h"

// this interface exists solely because native themes need to call into it.
// Only menu frames should implement it

// {212521C8-1509-4F41-ADDB-6A0B9356770F}
#define NS_IMENUFRAME_IID \
{ 0x212521C8, 0x1509, 0x4F41, { 0xAD, 0xDB, 0x6A, 0x0B, 0x93, 0x56, 0x77, 0x0F } }

class nsIMenuFrame : public nsISupports {

public:

  NS_DECLARE_STATIC_IID_ACCESSOR(NS_IMENUFRAME_IID)

  virtual PRBool IsOpen() = 0;
  virtual PRBool IsMenu() = 0;
  virtual PRBool IsOnMenuBar() = 0;
  virtual PRBool IsOnActiveMenuBar() = 0;
};

NS_DEFINE_STATIC_IID_ACCESSOR(nsIMenuFrame, NS_IMENUFRAME_IID)

#endif

