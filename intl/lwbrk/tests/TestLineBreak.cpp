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
#include <stdio.h>
#include "nsXPCOM.h"
#include "nsIComponentManager.h"
#include "nsISupports.h"
#include "nsServiceManagerUtils.h"
#include "nsILineBreaker.h"
#include "nsIWordBreaker.h"
#include "nsLWBrkCIID.h"
#include "nsStringAPI.h"
#include "nsEmbedString.h"

#define WORK_AROUND_SERVICE_MANAGER_ASSERT

NS_DEFINE_CID(kLBrkCID, NS_LBRK_CID);
NS_DEFINE_CID(kWBrkCID, NS_WBRK_CID);


static char teng1[] = 
//          1         2         3         4         5         6         7
//01234567890123456789012345678901234567890123456789012345678901234567890123456789
 "This is a test to test(reasonable) line    break. This 0.01123 = 45 x 48.";

static PRUint32 exp1[] = {
  4,5,7,8,9,10,14,15,17,18,22,34,35,39,40,41,42,43,49,50,54,55,62,63,64,65,
  67,68,69,70
};

static PRUint32 wexp1[] = {

  4,5,7,8,9,10,14,15,17,18,22,23,33,34,35,39,43,48,49,50,54,55,56,57,62,63,
  64,65,67,68,69,70,72
};
//          1         2         3         4         5         6         7
//01234567890123456789012345678901234567890123456789012345678901234567890123456789
static char teng2[] = 
 "()((reasonab(l)e) line  break. .01123=45x48.";

static PRUint32 lexp2[] = {
  2,12,15,17,18,22,23,24,30,31,37,38,
};
static PRUint32 wexp2[] = {
  4,12,13,14,15,16,17,18,22,24,29,30,31,32,37,38,43
};

//          1         2         3         4         5         6         7
//01234567890123456789012345678901234567890123456789012345678901234567890123456789
static char teng3[] = 
 "It's a test to test(ronae ) line break....";
static PRUint32 exp3[] = {
  4, 5, 6,7,11,12,14,15,19,25,27,28,32,33
};
static PRUint32 wexp3[] = {
  4,5,6,7,11,12,14,15,19,20,25,26,27,28,32,33,38
};

static char ruler1[] =
"          1         2         3         4         5         6         7  ";
static char ruler2[] =
"0123456789012345678901234567890123456789012345678901234567890123456789012";


PRBool TestASCIILB(nsILineBreaker *lb,
                 const char* in, const PRUint32 len, 
                 const PRUint32* out, PRUint32 outlen)
{
         NS_ConvertASCIItoUTF16 eng1(in);
         PRUint32 i,j;
         PRUint32 res[256];
         PRBool ok = PR_TRUE;
         PRInt32 curr;
         for(i = 0, curr = 0; (curr != NS_LINEBREAKER_NEED_MORE_TEXT) && 
             (i < 256); i++)
         {
            curr = lb->Next(eng1.get(), eng1.Length(), curr);
            res [i] = curr != NS_LINEBREAKER_NEED_MORE_TEXT ? curr : eng1.Length();
    
         }
         if (i != outlen)
         {
            ok = PR_FALSE;
            printf("WARNING!!! return size wrong, expect %d but got %d \n",
                   outlen, i);
         }
         printf("string  = \n%s\n", in);
         printf("%s\n", ruler1);
         printf("%s\n", ruler2);
         printf("Expect = \n");
         for(j=0;j<outlen;j++)
         {
            printf("%d,", out[j]);
         }
         printf("\nResult = \n");
         for(j=0;j<i;j++)
         {
            printf("%d,", res[j]);
         }
         printf("\n");
         for(j=0;j<i;j++)
         {
            if(j < outlen)
            {
                if (res[j] != out[j])
                {
                   ok = PR_FALSE;
                   printf("[%d] expect %d but got %d\n", j, out[j], res[j]);
                }
            } else {
                   ok = PR_FALSE;
                   printf("[%d] additional %d\n", j, res[j]);
            }
         }
         return ok;
}

PRBool TestASCIIWB(nsIWordBreaker *lb,
                 const char* in, const PRUint32 len, 
                 const PRUint32* out, PRUint32 outlen)
{
         NS_ConvertASCIItoUTF16 eng1(in);

         PRUint32 i,j;
         PRUint32 res[256];
         PRBool ok = PR_TRUE;
         PRInt32 curr = 0;

         for(i = 0, curr = lb->NextWord(eng1.get(), eng1.Length(), curr);
                    (curr != NS_WORDBREAKER_NEED_MORE_TEXT) && (i < 256);
                    curr = lb->NextWord(eng1.get(), eng1.Length(), curr), i++)
         {
            res [i] = curr != NS_WORDBREAKER_NEED_MORE_TEXT ? curr : eng1.Length();
         }
         if (i != outlen)
         {
            ok = PR_FALSE;
            printf("WARNING!!! return size wrong, expect %d but got %d\n",
                   outlen, i);
         }
         printf("string  = \n%s\n", in);
         printf("%s\n", ruler1);
         printf("%s\n", ruler2);
         printf("Expect = \n");
         for(j=0;j<outlen;j++)
         {
            printf("%d,", out[j]);
         }
         printf("\nResult = \n");
         for(j=0;j<i;j++)
         {
            printf("%d,", res[j]);
         }
         printf("\n");
         for(j=0;j<i;j++)
         {
            if(j < outlen)
            {
                if (res[j] != out[j])
                {
                   ok = PR_FALSE;
                   printf("[%d] expect %d but got %d\n", j, out[j], res[j]);
                }
            } else {
                   ok = PR_FALSE;
                   printf("[%d] additional %d\n", j, res[j]);
            }
         }
         return ok;
}
     
     
PRBool TestLineBreaker()
{
   printf("===========================\n");
   printf("Finish nsILineBreaker Test \n");
   printf("===========================\n");
   nsILineBreaker *t = NULL;
   nsresult res;
   PRBool ok = PR_TRUE;
   res = CallGetService(kLBrkCID, &t);
           
   printf("Test 1 - GetService():\n");
   if(NS_FAILED(res) || ( t == NULL ) ) {
     printf("\t1st GetService failed\n");
     ok = PR_FALSE;
   }

   NS_IF_RELEASE(t);

   res = CallGetService(kLBrkCID, &t);
 
   if(NS_FAILED(res) || ( t == NULL ) ) {
     printf("\t2nd GetService failed\n");
     ok = PR_FALSE;
   } else {
     printf("Test 4 - {First,Next}ForwardBreak():\n");
     if( TestASCIILB(t, teng1, sizeof(teng1)/sizeof(char), 
              exp1, sizeof(exp1)/sizeof(PRUint32)) )
     {
       printf("Test 4 Passed\n\n");
     } else {
       ok = PR_FALSE;
       printf("Test 4 Failed\n\n");
     }

     printf("Test 5 - {First,Next}ForwardBreak():\n");
     if(TestASCIILB(t, teng2, sizeof(teng2)/sizeof(char), 
               lexp2, sizeof(lexp2)/sizeof(PRUint32)) )
     {
       printf("Test 5 Passed\n\n");
     } else {
       ok = PR_FALSE;
       printf("Test 5 Failed\n\n");
     }

     printf("Test 6 - {First,Next}ForwardBreak():\n");
     if(TestASCIILB(t, teng3, sizeof(teng3)/sizeof(char), 
               exp3, sizeof(exp3)/sizeof(PRUint32)) )
     {
       printf("Test 6 Passed\n\n");
     } else {
       ok = PR_FALSE;
       printf("Test 6 Failed\n\n");
     }


     NS_RELEASE(t);

   }

   printf("===========================\n");
   printf("Finish nsILineBreaker Test \n");
   printf("===========================\n");

 return ok;
}

PRBool TestWordBreaker()
{
   printf("===========================\n");
   printf("Finish nsIWordBreaker Test \n");
   printf("===========================\n");
   nsIWordBreaker *t = NULL;
   nsresult res;
   PRBool ok = PR_TRUE;
   res = CallGetService(kWBrkCID, &t);
           
   printf("Test 1 - GetService():\n");
   if(NS_FAILED(res) || ( t == NULL ) ) {
     printf("\t1st GetService failed\n");
     ok = PR_FALSE;
   } else {
     NS_RELEASE(t);
   }

   res = CallGetService(kWBrkCID, &t);
           
   if(NS_FAILED(res) || ( t == NULL ) ) {
     printf("\t2nd GetService failed\n");
     ok = PR_FALSE;
   } else {

     printf("Test 4 - {First,Next}ForwardBreak():\n");
     if( TestASCIIWB(t, teng1, sizeof(teng1)/sizeof(char), 
               wexp1, sizeof(wexp1)/sizeof(PRUint32)) )
     {
        printf("Test 4 Passed\n\n");
     } else {
       ok = PR_FALSE;
       printf("Test 4 Failed\n\n");
     }

     printf("Test 5 - {First,Next}ForwardBreak():\n");
     if(TestASCIIWB(t, teng2, sizeof(teng2)/sizeof(char), 
               wexp2, sizeof(wexp2)/sizeof(PRUint32)) )
     {
       printf("Test 5 Passed\n\n");
     } else {
       ok = PR_FALSE;
       printf("Test 5 Failed\n\n");
     }

     printf("Test 6 - {First,Next}ForwardBreak():\n");
     if(TestASCIIWB(t, teng3, sizeof(teng3)/sizeof(char), 
               wexp3, sizeof(wexp3)/sizeof(PRUint32)) )
     {
       printf("Test 6 Passed\n\n");
     } else {
       ok = PR_FALSE;
       printf("Test 6 Failed\n\n");
     }


     NS_RELEASE(t);
   }

   printf("===========================\n");
   printf("Finish nsIWordBreaker Test \n");
   printf("===========================\n");

   return ok;
}

void   SamplePrintWordWithBreak();
void   SampleFindWordBreakFromPosition(PRUint32 fragN, PRUint32 offset);
// Sample Code

//                          012345678901234
static const char wb0[] =  "T";
static const char wb1[] =  "h";
static const char wb2[] =  "is   is a int";
static const char wb3[] =  "ernationali";
static const char wb4[] =  "zation work.";

static const char* wb[] = {wb0,wb1,wb2,wb3,wb4};
void SampleWordBreakUsage()
{
   SamplePrintWordWithBreak();
   SampleFindWordBreakFromPosition(0,0); // This
   SampleFindWordBreakFromPosition(1,0); // This
   SampleFindWordBreakFromPosition(2,0); // This
   SampleFindWordBreakFromPosition(2,1); // This
   SampleFindWordBreakFromPosition(2,9); // [space]
   SampleFindWordBreakFromPosition(2,10); // internationalization
   SampleFindWordBreakFromPosition(3,4);  // internationalization
   SampleFindWordBreakFromPosition(3,8);  // internationalization
   SampleFindWordBreakFromPosition(4,6);  // [space]
   SampleFindWordBreakFromPosition(4,7);  // work
}
 

void SamplePrintWordWithBreak()
{
   PRUint32 numOfFragment = sizeof(wb) / sizeof(char*);
   nsIWordBreaker *wbk = NULL;

   CallGetService(kWBrkCID, &wbk);

   nsAutoString result;

   for(PRUint32 i = 0; i < numOfFragment; i++)
   {
      NS_ConvertASCIItoUTF16 fragText(wb[i]);

      PRInt32 cur = 0;
      cur = wbk->NextWord(fragText.get(), fragText.Length(), cur);
      PRUint32 start = 0;
      for(PRUint32 j = 0; cur != NS_WORDBREAKER_NEED_MORE_TEXT ; j++)
      {
            result.Append(Substring(fragText, start, cur - start));
            result.Append('^');
            start = (cur >= 0 ? cur : cur - start);
            cur = wbk->NextWord(fragText.get(), fragText.Length(), cur);
      }

      result.Append(Substring(fragText, fragText.Length() - start));

      if( i != (numOfFragment -1 ))
      {
        NS_ConvertASCIItoUTF16 nextFragText(wb[i+1]);
 
        PRBool canBreak = PR_TRUE;
        canBreak = wbk->BreakInBetween( fragText.get(), 
                                        fragText.Length(),
                                        nextFragText.get(), 
                                        nextFragText.Length());
        if(canBreak)
            result.Append('^');

        fragText.Assign(nextFragText);
      }
   }
   printf("Output From  SamplePrintWordWithBreak() \n\n");
   printf("[%s]\n", NS_ConvertUTF16toUTF8(result).get());

   NS_IF_RELEASE(wbk);
}

void SampleFindWordBreakFromPosition(PRUint32 fragN, PRUint32 offset)
{
   PRUint32 numOfFragment = sizeof(wb) / sizeof(char*);
   nsIWordBreaker *wbk = NULL;

   CallGetService(kWBrkCID, &wbk);

   NS_ConvertASCIItoUTF16 fragText(wb[fragN]); 
   
   nsWordRange res = wbk->FindWord(fragText.get(), fragText.Length(), offset);

   PRBool canBreak;
   nsAutoString result(Substring(fragText, res.mBegin, res.mEnd-res.mBegin));

   if((PRUint32)fragText.Length() == res.mEnd) // if we hit the end of the fragment
   {
     nsAutoString curFragText = fragText;
     for(PRUint32  p = fragN +1; p < numOfFragment ;p++)
     {
        NS_ConvertASCIItoUTF16 nextFragText(wb[p]);
        canBreak = wbk->BreakInBetween(curFragText.get(), 
                                       curFragText.Length(),
                                       nextFragText.get(), 
                                       nextFragText.Length());
        if(canBreak)
           break;
 
        nsWordRange r = wbk->FindWord(nextFragText.get(), nextFragText.Length(),
                                      0);

        result.Append(Substring(nextFragText, r.mBegin, r.mEnd - r.mBegin));

        if((PRUint32)nextFragText.Length() != r.mEnd)
          break;

        nextFragText.Assign(curFragText);
     }
   }
   
   if(0 == res.mBegin) // if we hit the beginning of the fragment
   {
     nsAutoString curFragText = fragText;
     for(PRUint32  p = fragN ; p > 0 ;p--)
     {
        NS_ConvertASCIItoUTF16 prevFragText(wb[p-1]); 
        canBreak = wbk->BreakInBetween(prevFragText.get(), 
                                       prevFragText.Length(),
                                       curFragText.get(), 
                                       curFragText.Length());
        if(canBreak)
           break;
 
        nsWordRange r = wbk->FindWord(prevFragText.get(), prevFragText.Length(), 
                                      prevFragText.Length());

        result.Insert(Substring(prevFragText, r.mBegin, r.mEnd - r.mBegin), 0);

        if(0 != r.mBegin)
          break;

        prevFragText.Assign(curFragText);
     }
   }
   
   printf("Output From  SamplePrintWordWithBreak() \n\n");
   printf("[%s]\n", NS_ConvertUTF16toUTF8(result).get());

   NS_IF_RELEASE(wbk);
}

// Main

int main(int argc, char** argv) {

   NS_InitXPCOM2(nsnull, nsnull, nsnull);
   
   // --------------------------------------------
   printf("Test Line Break\n");

   PRBool lbok ; 
   PRBool wbok ; 
   lbok =TestWordBreaker();
   if(lbok)
      printf("Line Break Test\nOK\n");
   else
      printf("Line Break Test\nFailed\n");

   wbok = TestLineBreaker();
   if(wbok)
      printf("Word Break Test\nOK\n");
   else
      printf("Word Break Test\nFailed\n");

   SampleWordBreakUsage();
   

   // --------------------------------------------
   printf("Finish All The Test Cases\n");

   if(lbok && wbok)
      printf("Line/Word Break Test\nOK\n");
   else
      printf("Line/Word Break Test\nFailed\n");
   return 0;
}
