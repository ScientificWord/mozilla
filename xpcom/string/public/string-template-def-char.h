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

#define CharT                               char
#define CharT_is_char                       1
#define nsTObsoleteAString_CharT            nsObsoleteACString
#define nsTObsoleteAStringThunk_CharT       nsObsoleteACStringThunk
#ifdef MOZ_V1_STRING_ABI
#define nsTAString_CharT                    nsACString
#else
#define nsTAString_CharT                    nsCSubstring_base
#endif
#define nsTAString_IncompatibleCharT        nsAString
#define nsTString_CharT                     nsCString
#define nsTFixedString_CharT                nsFixedCString
#define nsTAutoString_CharT                 nsCAutoString
#ifdef MOZ_V1_STRING_ABI
#define nsTSubstring_CharT                  nsCSubstring
#else
#define nsTSubstring_CharT                  nsACString
#endif
#define nsTSubstringTuple_CharT             nsCSubstringTuple
#define nsTStringComparator_CharT           nsCStringComparator
#define nsTDefaultStringComparator_CharT    nsDefaultCStringComparator
#define nsTDependentString_CharT            nsDependentCString
#define nsTDependentSubstring_CharT         nsDependentCSubstring
#define nsTXPIDLString_CharT                nsXPIDLCString
#define nsTGetterCopies_CharT               nsCGetterCopies
#define nsTAdoptingString_CharT             nsAdoptingCString
#define nsTPromiseFlatString_CharT          nsPromiseFlatCString
#define TPromiseFlatString_CharT            PromiseFlatCString
