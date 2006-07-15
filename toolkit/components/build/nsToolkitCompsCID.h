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
 * Portions created by the Initial Developer are Copyright (C) 2001
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Joe Hewitt <hewitt@netscape.com> (Original Author)
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

#define NS_ALERTSERVICE_CONTRACTID \
  "@mozilla.org/alerts-service;1"

#define NS_AUTOCOMPLETECONTROLLER_CONTRACTID \
  "@mozilla.org/autocomplete/controller;1"

#define NS_AUTOCOMPLETECONTROLLER_CONTRACTID \
  "@mozilla.org/autocomplete/controller;1"

#define NS_AUTOCOMPLETESIMPLERESULT_CONTRACTID \
  "@mozilla.org/autocomplete/simple-result;1"

#define NS_AUTOCOMPLETEMDBRESULT_CONTRACTID \
  "@mozilla.org/autocomplete/mdb-result;1"

#define NS_DOWNLOADMANAGER_CONTRACTID \
  "@mozilla.org/download-manager;1"

#define NS_FORMHISTORY_CONTRACTID \
  "@mozilla.org/satchel/form-history;1"

#define NS_FORMHISTORYIMPORTER_CONTRACTID \
  "@mozilla.org/satchel/form-history-importer;1"

#define NS_FORMFILLCONTROLLER_CONTRACTID \
  "@mozilla.org/satchel/form-fill-controller;1"

#define NS_FORMHISTORYAUTOCOMPLETE_CONTRACTID \
  "@mozilla.org/autocomplete/search;1?name=form-history"

#define NS_GLOBALHISTORY_DATASOURCE_CONTRACTID \
  "@mozilla.org/rdf/datasource;1?name=history"

#define NS_GLOBALHISTORY_AUTOCOMPLETE_CONTRACTID \
    "@mozilla.org/autocomplete/search;1?name=history"

#define NS_TYPEAHEADFIND_CONTRACTID \
    "@mozilla.org/typeaheadfind;1"

#define NS_URLCLASSIFIERDBSERVICE_CONTRACTID \
    "@mozilla.org/url-classifier/dbservice;1"

#define NS_URLCLASSIFIERSTREAMUPDATER_CONTRACTID \
    "@mozilla.org/url-classifier/streamupdater;1"

#define NS_SCRIPTABLEUNESCAPEHTML_CONTRACTID "@mozilla.org/feed-unescapehtml;1"

/////////////////////////////////////////////////////////////////////////////

// {A0CCAAF8-09DA-44D8-B250-9AC3E93C8117}
#define NS_ALERTSSERVICE_CID \
{ 0xa0ccaaf8, 0x9da, 0x44d8, { 0xb2, 0x50, 0x9a, 0xc3, 0xe9, 0x3c, 0x81, 0x17 } }

// {F6D5EBBD-34F4-487d-9D10-3D34123E3EB9}
#define NS_AUTOCOMPLETECONTROLLER_CID \
{ 0xf6d5ebbd, 0x34f4, 0x487d, { 0x9d, 0x10, 0x3d, 0x34, 0x12, 0x3e, 0x3e, 0xb9 } }

// {2ee3039b-2de4-43d9-93b0-649beacff39a}
#define NS_AUTOCOMPLETESIMPLERESULT_CID \
{ 0x2ee3039b, 0x2de4, 0x43d9, { 0x93, 0xb0, 0x64, 0x9b, 0xea, 0xcf, 0xf3, 0x9a } }

// {7A6F70B6-2BBD-44b5-9304-501352D44AB5}
#define NS_AUTOCOMPLETEMDBRESULT_CID \
{ 0x7a6f70b6, 0x2bbd, 0x44b5, { 0x93, 0x4, 0x50, 0x13, 0x52, 0xd4, 0x4a, 0xb5 } }

#define NS_DOWNLOADMANAGER_CID \
    { 0xedb0490e, 0x1dd1, 0x11b2, { 0x83, 0xb8, 0xdb, 0xf8, 0xd8, 0x59, 0x06, 0xa6 } }

// {895DB6C7-DBDF-40ea-9F64-B175033243DC}
#define NS_FORMFILLCONTROLLER_CID \
{ 0x895db6c7, 0xdbdf, 0x40ea, { 0x9f, 0x64, 0xb1, 0x75, 0x3, 0x32, 0x43, 0xdc } }

// {A2059C0E-5A58-4c55-AB7C-26F0557546EF}
#define NS_FORMHISTORY_CID \
{ 0xa2059c0e, 0x5a58, 0x4c55, { 0xab, 0x7c, 0x26, 0xf0, 0x55, 0x75, 0x46, 0xef } }

// {db340cc2-7f50-4ea3-8427-f529daf6dc87}
#define NS_FORMHISTORYIMPORTER_CID \
{ 0xdb340cc2, 0x7f50, 0x4ea3, { 0x84, 0x27, 0xf5, 0x29, 0xda, 0xf6, 0xdc, 0x87 } }

// {59648a91-5a60-4122-8ff2-54b839c84aed}
#define NS_GLOBALHISTORY_CID \
{ 0x59648a91, 0x5a60, 0x4122, { 0x8f, 0xf2, 0x54, 0xb8, 0x39, 0xc8, 0x4a, 0xed} }

// {e7f70966-9a37-48d7-8aeb-35998f31090e}
#define NS_TYPEAHEADFIND_CID \
{ 0xe7f70966, 0x9a37, 0x48d7, { 0x8a, 0xeb, 0x35, 0x99, 0x8f, 0x31, 0x09, 0x0e} }

// {5eb7c3c1-ec1f-4007-87cc-eefb37d68ce6}
#define NS_URLCLASSIFIERDBSERVICE_CID \
{ 0x5eb7c3c1, 0xec1f, 0x4007, { 0x87, 0xcc, 0xee, 0xfb, 0x37, 0xd6, 0x8c, 0xe6} }

// {c2be6dc0-ef1e-4abd-86a2-4f864ddc57f6}
#define NS_URLCLASSIFIERSTREAMUPDATER_CID \
{ 0xc2be6dc0, 0xef1e, 0x4abd, { 0x86, 0xa2, 0x4f, 0x86, 0x4d, 0xdc, 0x57, 0xf6} }

// {10f2f5f0-f103-4901-980f-ba11bd70d60d}
#define NS_SCRIPTABLEUNESCAPEHTML_CID  \
{ 0x10f2f5f0, 0xf103, 0x4901, { 0x98, 0x0f, 0xba, 0x11, 0xbd, 0x70, 0xd6, 0x0d} }
