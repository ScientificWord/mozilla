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

/**
 * MODULE NOTES:
 * @update  jevering 6/17/98
 * 
 * This interface is not yet used; it was intended to allow an observer object
 * to "look at" the i/o stream coming into the parser before, during and after
 * the parser saw it. The intention of this was to allow an observer to modify
 * the stream at various stages.
 */

#ifndef  IPARSERFILTER
#define  IPARSERFILTER

#include "nsISupports.h"

class CToken;

#define NS_IPARSERFILTER_IID     \
  {0x14d6ff0,  0x0610,  0x11d2,  \
  {0x8c, 0x3f, 0x00,    0x80, 0x5f, 0x8a, 0x1d, 0xb7}}


class nsIParserFilter : public nsISupports {
  public:

   NS_DECLARE_STATIC_IID_ACCESSOR(NS_IPARSERFILTER_IID)
      
   NS_IMETHOD RawBuffer(const char * buffer, PRUint32 * buffer_length) = 0;

   NS_IMETHOD WillAddToken(CToken & token) = 0;

   NS_IMETHOD ProcessTokens( /* don't know what goes here yet */ void ) = 0;

   NS_IMETHOD Finish() = 0;

};

NS_DEFINE_STATIC_IID_ACCESSOR(nsIParserFilter, NS_IPARSERFILTER_IID)


#endif

