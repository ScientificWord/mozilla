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
 * Netscape.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   jeroen.dobbelaere@acunia.com
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
#include "unicpriv.h"
/*=================================================================================

=================================================================================*/
typedef  PRBool (*uSubGeneratorFunc) (PRUint16 in, unsigned char* out);
/*=================================================================================

=================================================================================*/

typedef PRBool (*uGeneratorFunc) (
                                  PRInt32*    state,
                                  PRUint16    in,
                                  unsigned char*  out,
                                  PRUint32     outbuflen,
                                  PRUint32*    outlen
                                  );

MODULE_PRIVATE PRBool uGenerate(  
                                uScanClassID scanClass,
                                PRInt32*    state,
                                PRUint16    in,
                                unsigned char*  out,
                                PRUint32     outbuflen,
                                PRUint32*    outlen
                                );

#define uSubGenerator(sub,in,out) (* m_subgenerator[sub])((in),(out))

PRIVATE PRBool uCheckAndGenAlways1Byte(
                                       PRInt32*   state,
                                       PRUint16   in,
                                       unsigned char* out,
                                       PRUint32    outbuflen,
                                       PRUint32*   outlen
                                       );
PRIVATE PRBool uCheckAndGenAlways2Byte(
                                       PRInt32*   state,
                                       PRUint16   in,
                                       unsigned char* out,
                                       PRUint32    outbuflen,
                                       PRUint32*   outlen
                                       );
PRIVATE PRBool uCheckAndGenAlways2ByteShiftGR(
                                              PRInt32*    state,
                                              PRUint16    in,
                                              unsigned char*  out,
                                              PRUint32     outbuflen,
                                              PRUint32*    outlen
                                              );
MODULE_PRIVATE PRBool uGenerateShift(
                                     uShiftOutTable   *shift,
                                     PRInt32*   state,
                                     PRUint16   in,
                                     unsigned char* out,
                                     PRUint32    outbuflen,
                                     PRUint32*   outlen
                                     );
PRIVATE PRBool uCheckAndGen2ByteGRPrefix8F(
                                           PRInt32*   state,
                                           PRUint16   in,
                                           unsigned char* out,
                                           PRUint32    outbuflen,
                                           PRUint32*   outlen
                                           );
PRIVATE PRBool uCheckAndGen2ByteGRPrefix8EA2(
                                             PRInt32*   state,
                                             PRUint16   in,
                                             unsigned char* out,
                                             PRUint32    outbuflen,
                                             PRUint32*   outlen
                                             );

PRIVATE PRBool uCheckAndGen2ByteGRPrefix8EA3(
                                             PRInt32*   state,
                                             PRUint16   in,
                                             unsigned char* out,
                                             PRUint32    outbuflen,
                                             PRUint32*   outlen
                                             );

PRIVATE PRBool uCheckAndGen2ByteGRPrefix8EA4(
                                             PRInt32*   state,
                                             PRUint16   in,
                                             unsigned char* out,
                                             PRUint32    outbuflen,
                                             PRUint32*   outlen
                                             );

PRIVATE PRBool uCheckAndGen2ByteGRPrefix8EA5(
                                             PRInt32*   state,
                                             PRUint16   in,
                                             unsigned char* out,
                                             PRUint32    outbuflen,
                                             PRUint32*   outlen
                                             );

PRIVATE PRBool uCheckAndGen2ByteGRPrefix8EA6(
                                             PRInt32*   state,
                                             PRUint16   in,
                                             unsigned char* out,
                                             PRUint32    outbuflen,
                                             PRUint32*   outlen
                                             );

PRIVATE PRBool uCheckAndGen2ByteGRPrefix8EA7(
                                             PRInt32*   state,
                                             PRUint16   in,
                                             unsigned char* out,
                                             PRUint32    outbuflen,
                                             PRUint32*   outlen
                                             );
PRIVATE PRBool uCnGAlways8BytesDecomposedHangul(
                                              PRInt32*    state,
                                              PRUint16    in,
                                              unsigned char*  out,
                                              PRUint32     outbuflen,
                                              PRUint32*    outlen
                                              );

PRIVATE PRBool uCheckAndGenJohabHangul(
                                       PRInt32*   state,
                                       PRUint16   in,
                                       unsigned char* out,
                                       PRUint32    outbuflen,
                                       PRUint32*   outlen
                                       );

PRIVATE PRBool uCheckAndGenJohabSymbol(
                                       PRInt32*   state,
                                       PRUint16   in,
                                       unsigned char* out,
                                       PRUint32    outbuflen,
                                       PRUint32*   outlen
                                       );


PRIVATE PRBool uCheckAndGen4BytesGB18030(
                                         PRInt32*   state,
                                         PRUint16   in,
                                         unsigned char* out,
                                         PRUint32    outbuflen,
                                         PRUint32*   outlen
                                         );

PRIVATE PRBool uGenAlways2Byte(
                               PRUint16    in,
                               unsigned char* out
                               );
PRIVATE PRBool uGenAlways2ByteShiftGR(
                                      PRUint16     in,
                                      unsigned char*  out
                                      );
PRIVATE PRBool uGenAlways1Byte(
                               PRUint16    in,
                               unsigned char* out
                               );
PRIVATE PRBool uGenAlways1BytePrefix8E(
                                       PRUint16    in,
                                       unsigned char* out
                                       );
                                   /*=================================================================================
                                   
=================================================================================*/
PRIVATE const uGeneratorFunc m_generator[uNumOfCharsetType] =
{
    uCheckAndGenAlways1Byte,
    uCheckAndGenAlways2Byte,
    uCheckAndGenAlways2ByteShiftGR,
    uCheckAndGen2ByteGRPrefix8F,
    uCheckAndGen2ByteGRPrefix8EA2,
    uCheckAndGen2ByteGRPrefix8EA3,
    uCheckAndGen2ByteGRPrefix8EA4,
    uCheckAndGen2ByteGRPrefix8EA5,
    uCheckAndGen2ByteGRPrefix8EA6,
    uCheckAndGen2ByteGRPrefix8EA7,
    uCnGAlways8BytesDecomposedHangul,
    uCheckAndGenJohabHangul,
    uCheckAndGenJohabSymbol,
    uCheckAndGen4BytesGB18030,
    uCheckAndGenAlways2Byte   /* place-holder for GR128 */
};

/*=================================================================================

=================================================================================*/

PRIVATE const uSubGeneratorFunc m_subgenerator[uNumOfCharType] =
{
    uGenAlways1Byte,
    uGenAlways2Byte,
    uGenAlways2ByteShiftGR,
    uGenAlways1BytePrefix8E
        
};
/*=================================================================================

=================================================================================*/
MODULE_PRIVATE PRBool uGenerate(  
                                uScanClassID scanClass,
                                PRInt32*    state,
                                PRUint16    in,
                                unsigned char*  out,
                                PRUint32     outbuflen,
                                PRUint32*    outlen
                                )
{
    return (* m_generator[scanClass]) (state,in,out,outbuflen,outlen);
}
/*=================================================================================

=================================================================================*/
PRIVATE PRBool uGenAlways1Byte(
                               PRUint16    in,
                               unsigned char* out
                               )
{
    out[0] = (unsigned char)in;
    return PR_TRUE;
}

/*=================================================================================

=================================================================================*/
PRIVATE PRBool uGenAlways2Byte(
                               PRUint16    in,
                               unsigned char* out
                               )
{
    out[0] = (unsigned char)((in >> 8) & 0xff);
    out[1] = (unsigned char)(in & 0xff);
    return PR_TRUE;
}
/*=================================================================================

=================================================================================*/
PRIVATE PRBool uGenAlways2ByteShiftGR(
                                      PRUint16     in,
                                      unsigned char*  out
                                      )
{
    out[0] = (unsigned char)(((in >> 8) & 0xff) | 0x80);
    out[1] = (unsigned char)((in & 0xff) | 0x80);
    return PR_TRUE;
}
/*=================================================================================

=================================================================================*/
PRIVATE PRBool uGenAlways1BytePrefix8E(
                                       PRUint16    in,
                                       unsigned char* out
                                       )
{
    out[0] = 0x8E;
    out[1] = (unsigned char)(in  & 0xff);
    return PR_TRUE;
}
/*=================================================================================

=================================================================================*/
PRIVATE PRBool uCheckAndGenAlways1Byte(
                                       PRInt32*   state,
                                       PRUint16   in,
                                       unsigned char* out,
                                       PRUint32    outbuflen,
                                       PRUint32*   outlen
                                       )
{
    /* Don't check inlen. The caller should ensure it is larger than 0 */
    /*  Oops, I don't agree. Code changed to check every time. [CATA] */
    if(outbuflen < 1)
        return PR_FALSE;
    else
    {
        *outlen = 1;
        out[0] = in & 0xff;
        return PR_TRUE;
    }
}

/*=================================================================================

=================================================================================*/
PRIVATE PRBool uCheckAndGenAlways2Byte(
                                       PRInt32*   state,
                                       PRUint16   in,
                                       unsigned char* out,
                                       PRUint32    outbuflen,
                                       PRUint32*   outlen
                                       )
{
    if(outbuflen < 2)
        return PR_FALSE;
    else
    {
        *outlen = 2;
        out[0] = ((in >> 8 ) & 0xff);
        out[1] = in  & 0xff;
        return PR_TRUE;
    }
}
/*=================================================================================

=================================================================================*/
PRIVATE PRBool uCheckAndGenAlways2ByteShiftGR(
                                              PRInt32*    state,
                                              PRUint16    in,
                                              unsigned char*  out,
                                              PRUint32     outbuflen,
                                              PRUint32*    outlen
                                              )
{
    if(outbuflen < 2)
        return PR_FALSE;
    else
    {
        *outlen = 2;
        out[0] = ((in >> 8 ) & 0xff) | 0x80;
        out[1] = (in  & 0xff)  | 0x80;
        return PR_TRUE;
    }
}
/*=================================================================================

=================================================================================*/
MODULE_PRIVATE PRBool uGenerateShift(
                                   uShiftOutTable   *shift,
                                   PRInt32*   state,
                                   PRUint16   in,
                                   unsigned char* out,
                                   PRUint32    outbuflen,
                                   PRUint32*   outlen
                                   )
{
    PRInt16 i;
    const uShiftOutCell* cell = &(shift->shiftcell[0]);
    PRInt16 itemnum = shift->numOfItem;
    unsigned char inH, inL;
    inH = (in >> 8) & 0xff;
    inL = (in & 0xff );
    for(i=0;i<itemnum;i++)
    {
        if( ( inL >=  cell[i].shiftout_MinLB) &&
            ( inL <=  cell[i].shiftout_MaxLB) &&
            ( inH >=  cell[i].shiftout_MinHB) &&
            ( inH <=  cell[i].shiftout_MaxHB) )
        {
            if(outbuflen < cell[i].reserveLen)
              {
                return PR_FALSE;
              }
            else
            {
                *outlen = cell[i].reserveLen;
                return (uSubGenerator(cell[i].classID,in,out));
            }
        }
    }
    return PR_FALSE;
}
/*=================================================================================

=================================================================================*/
PRIVATE PRBool uCheckAndGen2ByteGRPrefix8F( PRInt32*   state,
                                           PRUint16   in,
                                           unsigned char* out,
                                           PRUint32    outbuflen,
                                           PRUint32*   outlen
                                           )
{
    if(outbuflen < 3)
        return PR_FALSE;
    else
    {
        *outlen = 3;
        out[0] = 0x8F;
        out[1] = ((in >> 8 ) & 0xff) | 0x80;
        out[2] = (in  & 0xff)  | 0x80;
        return PR_TRUE;
    }
}
/*=================================================================================

=================================================================================*/
PRIVATE PRBool uCheckAndGen2ByteGRPrefix8EA2( PRInt32*   state,
                                             PRUint16   in,
                                             unsigned char* out,
                                             PRUint32    outbuflen,
                                             PRUint32*   outlen
                                             )
{
    if(outbuflen < 4)
        return PR_FALSE;
    else
    {
        *outlen = 4;
        out[0] = 0x8E;
        out[1] = 0xA2;
        out[2] = ((in >> 8 ) & 0xff) | 0x80;
        out[3] = (in  & 0xff)  | 0x80;
        return PR_TRUE;
    }
}


/*=================================================================================

=================================================================================*/
PRIVATE PRBool uCheckAndGen2ByteGRPrefix8EA3( PRInt32*   state,
                                             PRUint16   in,
                                             unsigned char* out,
                                             PRUint32    outbuflen,
                                             PRUint32*   outlen
                                             )
{
    if(outbuflen < 4)
        return PR_FALSE;
    else
    {
        *outlen = 4;
        out[0] = 0x8E;
        out[1] = 0xA3;
        out[2] = ((in >> 8 ) & 0xff) | 0x80;
        out[3] = (in  & 0xff)  | 0x80;
        return PR_TRUE;
    }
}
/*=================================================================================

=================================================================================*/
PRIVATE PRBool uCheckAndGen2ByteGRPrefix8EA4( PRInt32*   state,
                                             PRUint16   in,
                                             unsigned char* out,
                                             PRUint32    outbuflen,
                                             PRUint32*   outlen
                                             )
{
    if(outbuflen < 4)
        return PR_FALSE;
    else
    {
        *outlen = 4;
        out[0] = 0x8E;
        out[1] = 0xA4;
        out[2] = ((in >> 8 ) & 0xff) | 0x80;
        out[3] = (in  & 0xff)  | 0x80;
        return PR_TRUE;
    }
}
/*=================================================================================

=================================================================================*/
PRIVATE PRBool uCheckAndGen2ByteGRPrefix8EA5( PRInt32*   state,
                                             PRUint16   in,
                                             unsigned char* out,
                                             PRUint32    outbuflen,
                                             PRUint32*   outlen
                                             )
{
    if(outbuflen < 4)
        return PR_FALSE;
    else
    {
        *outlen = 4;
        out[0] = 0x8E;
        out[1] = 0xA5;
        out[2] = ((in >> 8 ) & 0xff) | 0x80;
        out[3] = (in  & 0xff)  | 0x80;
        return PR_TRUE;
    }
}
/*=================================================================================

=================================================================================*/
PRIVATE PRBool uCheckAndGen2ByteGRPrefix8EA6( PRInt32*   state,
                                             PRUint16   in,
                                             unsigned char* out,
                                             PRUint32    outbuflen,
                                             PRUint32*   outlen
                                             )
{
    if(outbuflen < 4)
        return PR_FALSE;
    else
    {
        *outlen = 4;
        out[0] = 0x8E;
        out[1] = 0xA6;
        out[2] = ((in >> 8 ) & 0xff) | 0x80;
        out[3] = (in  & 0xff)  | 0x80;
        return PR_TRUE;
    }
}
/*=================================================================================

=================================================================================*/
PRIVATE PRBool uCheckAndGen2ByteGRPrefix8EA7( PRInt32*   state,
                                             PRUint16   in,
                                             unsigned char* out,
                                             PRUint32    outbuflen,
                                             PRUint32*   outlen
                                             )
{
    if(outbuflen < 4)
        return PR_FALSE;
    else
    {
        *outlen = 4;
        out[0] = 0x8E;
        out[1] = 0xA7;
        out[2] = ((in >> 8 ) & 0xff) | 0x80;
        out[3] = (in  & 0xff)  | 0x80;
        return PR_TRUE;
    }
}
/*=================================================================================

=================================================================================*/
#define SBase 0xAC00
#define LCount 19
#define VCount 21
#define TCount 28
#define NCount (VCount * TCount)
/*=================================================================================

=================================================================================*/
PRIVATE PRBool uCnGAlways8BytesDecomposedHangul(
                                              PRInt32*    state,
                                              PRUint16    in,
                                              unsigned char*  out,
                                              PRUint32     outbuflen,
                                              PRUint32*    outlen
                                              )
{
    static const PRUint8 lMap[LCount] = {
        0xa1, 0xa2, 0xa4, 0xa7, 0xa8, 0xa9, 0xb1, 0xb2, 0xb3, 0xb5,
            0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe
    };
    
    static const PRUint8 tMap[TCount] = {
        0xd4, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa9, 0xaa, 
            0xab, 0xac, 0xad, 0xae, 0xaf, 0xb0, 0xb1, 0xb2, 0xb4, 0xb5, 
            0xb6, 0xb7, 0xb8, 0xba, 0xbb, 0xbc, 0xbd, 0xbe
    };

    PRUint16 SIndex, LIndex, VIndex, TIndex;

    if(outbuflen < 8)
        return PR_FALSE;

    /* the following line are copy from Unicode 2.0 page 3-13 */
    /* item 1 of Hangul Syllabel Decomposition */
    SIndex =  in - SBase;
    
    /* the following lines are copy from Unicode 2.0 page 3-14 */
    /* item 2 of Hangul Syllabel Decomposition w/ modification */
    LIndex = SIndex / NCount;
    VIndex = (SIndex % NCount) / TCount;
    TIndex = SIndex % TCount;
    
    /* 
     * A Hangul syllable not enumerated in KS X 1001 is represented
     * by a sequence of 8 bytes beginning with Hangul-filler
     * (0xA4D4 in EUC-KR and 0x2454 in ISO-2022-KR) followed by three 
     * Jamos (2 bytes each the first of which is 0xA4 in EUC-KR) making 
     * up the syllable.  ref. KS X 1001:1998 Annex 3
     */
    *outlen = 8;
    out[0] = out[2] = out[4] = out[6] = 0xa4;
    out[1] = 0xd4;
    out[3] = lMap[LIndex] ;
    out[5] = (VIndex + 0xbf);
    out[7] = tMap[TIndex];

    return PR_TRUE;
}

PRIVATE PRBool uCheckAndGenJohabHangul(
                                       PRInt32*   state,
                                       PRUint16   in,
                                       unsigned char* out,
                                       PRUint32    outbuflen,
                                       PRUint32*   outlen
                                       )
{
    if(outbuflen < 2)
        return PR_FALSE;
    else
    {
    /*
    See Table 4-45 (page 183) of CJKV Information Processing
    for detail explanation of the following table.
        */
        /*
        static const PRUint8 lMap[LCount] = {
        2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20
        };
        Therefore lMap[i] == i+2;
        */
        
        static const PRUint8 vMap[VCount] = {
            /* no 0,1,2 */
            3,4,5,6,7,            /* no 8,9   */
                10,11,12,13,14,15,    /* no 16,17 */
                18,19,20,21,22,23,    /* no 24,25 */
                26,27,28,29
        };
        static const PRUint8 tMap[TCount] = {
            1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17, /* no 18 */
                19,20,21,22,23,24,25,26,27,28,29
        };
        PRUint16 SIndex, LIndex, VIndex, TIndex, ch;
        /* the following line are copy from Unicode 2.0 page 3-13 */
        /* item 1 of Hangul Syllabel Decomposition */
        SIndex =  in - SBase;
        
        /* the following lines are copy from Unicode 2.0 page 3-14 */
        /* item 2 of Hangul Syllabel Decomposition w/ modification */
        LIndex = SIndex / NCount;
        VIndex = (SIndex % NCount) / TCount;
        TIndex = SIndex % TCount;
        
        *outlen = 2;
        ch = 0x8000 | 
            ((LIndex+2)<<10) | 
            (vMap[VIndex]<<5)| 
            tMap[TIndex];
        out[0] = (ch >> 8);
        out[1] = ch & 0x00FF;
#if 0
        printf("Johab Hangul %x %x in=%x L=%d V=%d T=%d\n", out[0], out[1], in, LIndex, VIndex, TIndex); 
#endif 
        return PR_TRUE;
    }
}
PRIVATE PRBool uCheckAndGenJohabSymbol(
                                       PRInt32*   state,
                                       PRUint16   in,
                                       unsigned char* out,
                                       PRUint32    outbuflen,
                                       PRUint32*   outlen
                                       )
{
    if(outbuflen < 2)
        return PR_FALSE;
    else
    {
    /* The following code are based on the Perl code listed under
    * "ISO-2022-KR or EUC-KR to Johab Conversion" (page 1013)
    * in the book "CJKV Information Processing" by 
    * Ken Lunde <lunde@adobe.com>
    *
    * sub convert2johab($) { # Convert ISO-2022-KR or EUC-KR to Johab
    *  my @euc = unpack("C*", $_[0]);
    *  my ($fe_off, $hi_off, $lo_off) = (0,0,1);
    *  my @out = ();
    *  while(($hi, $lo) = splice(@euc, 0, 2)) {
    *    $hi &= 127; $lo &= 127;
    *    $fe_off = 21 if $hi == 73;
    *    $fe_off = 34 if $hi == 126;
    *    ($hi_off, $lo_off) = ($lo_off, $hi_off) if ($hi <74 or $hi >125);
    *    push(@out, ((($hi+$hi_off) >> 1)+ ($hi <74 ? 200:187)- $fe_off),
    *      $lo + ((($hi+$lo_off) & 1) ? ($lo > 110 ? 34:16):128));    
    *  }
    *  return pack("C*", @out);
        */
        
        unsigned char fe_off = 0;
        unsigned char hi_off = 0;
        unsigned char lo_off = 1;
        unsigned char hi = (in >> 8) & 0x7F;
        unsigned char lo = in & 0x7F;
        if(73 == hi)
            fe_off = 21;
        if(126 == hi)
            fe_off = 34;
        if( (hi < 74) || ( hi > 125) )
        {
            hi_off = 1;
            lo_off = 0;
        }
        *outlen = 2;
        out[0] =  ((hi+hi_off) >> 1) + ((hi<74) ? 200 : 187 ) - fe_off;
        out[1] =  lo + (((hi+lo_off) & 1) ? ((lo > 110) ? 34 : 16) : 
        128);
#if 0
        printf("Johab Symbol %x %x in=%x\n", out[0], out[1], in); 
#endif
        return PR_TRUE;
    }
}
PRIVATE PRBool uCheckAndGen4BytesGB18030(
                                         PRInt32*   state,
                                         PRUint16   in,
                                         unsigned char* out,
                                         PRUint32    outbuflen,
                                         PRUint32*   outlen
                                         )
{
    if(outbuflen < 4)
        return PR_FALSE;
    out[0] = (in / (10*126*10)) + 0x81;
    in %= (10*126*10);
    out[1] = (in / (10*126)) + 0x30;
    in %= (10*126);
    out[2] = (in / (10)) + 0x81;
    out[3] = (in % 10) + 0x30;
    *outlen = 4;
    return PR_TRUE;
}
