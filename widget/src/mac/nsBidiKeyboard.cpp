/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
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
 * The Original Code is IBM code.
 *
 * The Initial Developer of the Original Code is
 * IBM. Portions created by IBM are Copyright (C) International Business Machines Corporation, 2000.  All Rights Reserved.
 * Portions created by the Initial Developer are Copyright (C) 2001
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Simon Montagu
 *   Asaf Romano <mozilla.mano@sent.com>
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

#include "nsBidiKeyboard.h"

NS_IMPL_ISUPPORTS1(nsBidiKeyboard, nsIBidiKeyboard)

nsBidiKeyboard::nsBidiKeyboard() : nsIBidiKeyboard()
{
}

nsBidiKeyboard::~nsBidiKeyboard()
{
}

NS_IMETHODIMP nsBidiKeyboard::IsLangRTL(PRBool *aIsRTL)
{
  *aIsRTL = PR_FALSE;
  nsresult rv = NS_ERROR_FAILURE;

  OSStatus err;
  KeyboardLayoutRef currentKeyboard;

  err = ::KLGetCurrentKeyboardLayout(&currentKeyboard);
  if (err == noErr)
  {
    const void* currentKeyboardResID;
    err = ::KLGetKeyboardLayoutProperty(currentKeyboard, kKLIdentifier,
                                        &currentKeyboardResID);
    if (err == noErr)
    {
      rv = NS_OK;
      *aIsRTL = IsRTLLanguage((SInt32)currentKeyboardResID);
    }
  }

  return rv;
}

NS_IMETHODIMP nsBidiKeyboard::SetLangFromBidiLevel(PRUint8 aLevel)
{
  // XXX Insert platform specific code to set keyboard language
  return NS_OK;
}

PRBool nsBidiKeyboard::IsRTLLanguage(SInt32 aKeyboardResID)
{
  // Check if the resource id is BiDi associated (Arabic, Persian, Hebrew)
  // (Persian is included in the Arabic range)
  // http://developer.apple.com/documentation/mac/Text/Text-534.html#HEADING534-0
  // Note: these ^^ values are negative on Mac OS X
  return (aKeyboardResID >= -18943 && aKeyboardResID <= -17920);
}
