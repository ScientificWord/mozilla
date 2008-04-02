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


#include "nsICharsetAlias.h"
#include "pratom.h"

// for NS_IMPL_IDS only
#include "nsIPlatformCharset.h"

#include "nsUConvDll.h"
#include "nsReadableUtils.h"
#include "nsUnicharUtils.h"
#include "nsGREResProperties.h"
#include "nsITimelineService.h"
#include "nsCharsetAlias.h"

//--------------------------------------------------------------
NS_IMPL_ISUPPORTS1(nsCharsetAlias2, nsICharsetAlias)

//--------------------------------------------------------------
nsCharsetAlias2::nsCharsetAlias2()
{
  mDelegate = nsnull; // delay the load of mDelegate untill we need it.
}
//--------------------------------------------------------------
nsCharsetAlias2::~nsCharsetAlias2()
{
  if(mDelegate)
     delete mDelegate;
}

// 
static const char* kAliases[][3] = {
  // Triple with { lower-case test string, out string, length of out string }
  { "iso-8859-1", "ISO-8859-1", (const char*)NS_INT32_TO_PTR(10) },
  { "utf-8",      "UTF-8",      (const char*)NS_INT32_TO_PTR(5)  },
  { "x-sjis",     "Shift_JIS",  (const char*)NS_INT32_TO_PTR(9)  },
  { "shift_jis",  "Shift_JIS",  (const char*)NS_INT32_TO_PTR(9)  }
};

//--------------------------------------------------------------
NS_IMETHODIMP nsCharsetAlias2::GetPreferred(const nsACString& aAlias,
                                            nsACString& oResult)
{
   if (aAlias.IsEmpty()) return NS_ERROR_NULL_POINTER;
   NS_TIMELINE_START_TIMER("nsCharsetAlias2:GetPreferred");


   // Delay loading charsetalias.properties by hardcoding the most
   // frequent aliases.  Note that it's possible to recur in to this
   // function *while loading* charsetalias.properties (see bug 190951),
   // so we might have an |mDelegate| already that isn't valid yet, but
   // the load is guaranteed to be "UTF-8" so things will be OK.
   for (PRUint32 index = 0; index < NS_ARRAY_LENGTH(kAliases); index++) {
     if (aAlias.LowerCaseEqualsASCII(kAliases[index][0])) {
       oResult.Assign(nsDependentCString(kAliases[index][1],
                                         NS_PTR_TO_UINT32(kAliases[index][2])));
       NS_TIMELINE_STOP_TIMER("nsCharsetAlias2:GetPreferred");
       return NS_OK;
     }
   }

   oResult.Truncate();

   if(!mDelegate) {
     //load charsetalias.properties string bundle with all remaining aliases
     // we may need to protect the following section with a lock so we won't call the
     // 'new nsGREResProperties' from two different threads
     mDelegate = new nsGREResProperties( NS_LITERAL_CSTRING("charsetalias.properties") );
     NS_ASSERTION(mDelegate, "cannot create nsGREResProperties");
     if(nsnull == mDelegate)
       return NS_ERROR_OUT_OF_MEMORY;
   }

   NS_TIMELINE_STOP_TIMER("nsCharsetAlias2:GetPreferred");
   NS_TIMELINE_MARK_TIMER("nsCharsetAlias2:GetPreferred");

   nsCAutoString key(aAlias);
   ToLowerCase(key);

   // hack for now, have to fix nsGREResProperties, but we can't until
   // string bundles use UTF8 keys
   nsAutoString result;
   nsresult rv = mDelegate->Get(NS_ConvertASCIItoUTF16(key), result);
   LossyAppendUTF16toASCII(result, oResult);
   return rv;
}

//--------------------------------------------------------------
NS_IMETHODIMP
nsCharsetAlias2::Equals(const nsACString& aCharset1,
                        const nsACString& aCharset2, PRBool* oResult)
{
   nsresult res = NS_OK;

   if(aCharset1.Equals(aCharset2, nsCaseInsensitiveCStringComparator())) {
      *oResult = PR_TRUE;
      return res;
   }

   if(aCharset1.IsEmpty() || aCharset2.IsEmpty()) {
      *oResult = PR_FALSE;
      return res;
   }

   *oResult = PR_FALSE;
   nsCAutoString name1;
   nsCAutoString name2;
   res = this->GetPreferred(aCharset1, name1);
   if(NS_SUCCEEDED(res)) {
      res = this->GetPreferred(aCharset2, name2);
      if(NS_SUCCEEDED(res)) {
        *oResult = name1.Equals(name2, nsCaseInsensitiveCStringComparator());
      }
   }
   
   return res;
}

