/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
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
 * The Original Code is Mozilla.
 *
 * The Initial Developer of the Original Code is IBM Corporation.
 * Portions created by IBM Corporation are Copyright (C) 2003
 * IBM Corporation. All Rights Reserved.
 *
 * Contributor(s):
 *   Darin Fisher <darin@meer.net>
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

#ifdef MOZ_V1_STRING_ABI
void
nsTDependentSubstring_CharT::Rebind( const abstract_string_type& readable, PRUint32 startPos, PRUint32 length )
  {
    // If we currently own a buffer, release it.
    Finalize();

    size_type strLength = readable.GetReadableBuffer((const char_type**) &mData);

    if (startPos > strLength)
      startPos = strLength;

    mData += startPos;
    mLength = NS_MIN(length, strLength - startPos);

    SetDataFlags(F_NONE);
  }
#endif

void
nsTDependentSubstring_CharT::Rebind( const substring_type& str, PRUint32 startPos, PRUint32 length )
  {
    // If we currently own a buffer, release it.
    Finalize();

    size_type strLength = str.Length();

    if (startPos > strLength)
      startPos = strLength;

    mData = NS_CONST_CAST(char_type*, str.Data()) + startPos;
    mLength = NS_MIN(length, strLength - startPos);

    SetDataFlags(F_NONE);
  }

void
nsTDependentSubstring_CharT::Rebind( const char_type* start, const char_type* end )
  {
    NS_ASSERTION(start && end, "nsTDependentSubstring must wrap a non-NULL buffer");

    // If we currently own a buffer, release it.
    Finalize();

    mData = NS_CONST_CAST(char_type*, start);
    mLength = end - start;
    SetDataFlags(F_NONE);
  }
