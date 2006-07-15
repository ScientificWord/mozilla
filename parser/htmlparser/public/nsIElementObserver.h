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
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1999
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


/**
 * MODULE NOTES:
 * @update  rickg 03.23.2000  //removed unused NS_PARSER_SUBJECT and predecl of nsString
 * 
 */

#ifndef nsIElementObserver_h__
#define nsIElementObserver_h__

#include "nsISupports.h"
#include "prtypes.h"
#include "nsHTMLTags.h"
#include "nsVoidArray.h"


// {4672AA04-F6AE-11d2-B3B7-00805F8A6670}
#define NS_IELEMENTOBSERVER_IID      \
{ 0x4672aa04, 0xf6ae, 0x11d2, { 0xb3, 0xb7, 0x0, 0x80, 0x5f, 0x8a, 0x66, 0x70 } }


class nsIElementObserver : public nsISupports {
public:
  NS_DECLARE_STATIC_IID_ACCESSOR(NS_IELEMENTOBSERVER_IID)

  enum { IS_DOCUMENT_WRITE = 1U };
  /*
   *   Subject call observer when the parser hit the tag
   *   @param aDocumentID- ID of the document
   *   @param aTag- the tag
   *   @param numOfAttributes - number of attributes
   *   @param nameArray - array of name. 
   *   @param valueArray - array of value
   */
  NS_IMETHOD Notify(PRUint32 aDocumentID, eHTMLTags aTag, 
                    PRUint32 numOfAttributes, const PRUnichar* nameArray[], 
                    const PRUnichar* valueArray[]) = 0;

  NS_IMETHOD Notify(PRUint32 aDocumentID, const PRUnichar* aTag, 
                    PRUint32 numOfAttributes, const PRUnichar* nameArray[], 
                    const PRUnichar* valueArray[]) = 0;
  
  NS_IMETHOD Notify(nsISupports* aWebShell, 
                    nsISupports* aChannel,
                    const PRUnichar* aTag, 
                    const nsStringArray* aKeys, 
                    const nsStringArray* aValues,
                    const PRUint32 aFlags) = 0;

};

NS_DEFINE_STATIC_IID_ACCESSOR(nsIElementObserver, NS_IELEMENTOBSERVER_IID)

#define NS_HTMLPARSER_VALID_META_CHARSET NS_ERROR_GENERATE_SUCCESS( \
                                          NS_ERROR_MODULE_HTMLPARSER,3000)

#endif /* nsIElementObserver_h__ */

