/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
#ifndef nsUnicodeEncodeHelper_h__
#define nsUnicodeEncodeHelper_h__

#include "nsIUnicodeEncoder.h"
#include "uconvutil.h"
//----------------------------------------------------------------------
// Class nsUnicodeEncodeHelper [declaration]

/**
 *
 * @created         22/Nov/1998
 * @author  Catalin Rotaru [CATA]
 */
class nsUnicodeEncodeHelper
{

public:
  //--------------------------------------------------------------------

  /**
   * Converts data using a lookup table and optional shift table.
   */
  static nsresult ConvertByTable(const PRUnichar * aSrc, PRInt32 * aSrcLength, 
      char * aDest, PRInt32 * aDestLength, uScanClassID aScanClass,
      uShiftOutTable * aShiftOutTable, uMappingTable  * aMappingTable);

  /**
   * Converts data using a set of lookup tables and optional shift tables.
   */
  static nsresult ConvertByMultiTable(const PRUnichar * aSrc, PRInt32 * aSrcLength,
      char * aDest, PRInt32 * aDestLength, PRInt32 aTableCount, 
      uScanClassID * aScanClassArray, 
      uShiftOutTable ** aShiftOutTable, uMappingTable  ** aMappingTable);

  /**
   * Create Char Representable Info
   */
  static nsresult FillInfo(PRUint32* aInfo, uMappingTable  * aMappingTable);
  static nsresult FillInfo(PRUint32* aInfo, PRInt32 aTableCount, uMappingTable  ** aMappingTable);
};

#endif // nsUnicodeEncodeHelper_h__


