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
#define CHK_GR94(b) ( (PRUint8) 0xa0 < (PRUint8) (b) && (PRUint8) (b) < (PRUint8) 0xff )
#define CHK_GR94_2Byte(b1,b2) (CHK_GR94(b1) && CHK_GR94(b2))
/*=================================================================================

=================================================================================*/
typedef  PRBool (*uSubScannerFunc) (unsigned char* in, PRUint16* out);
/*=================================================================================

=================================================================================*/

typedef PRBool (*uScannerFunc) (
                                PRInt32*    state,
                                unsigned char  *in,
                                PRUint16    *out,
                                PRUint32     inbuflen,
                                PRUint32*    inscanlen
                                );

MODULE_PRIVATE PRBool uScan(  
                            uScanClassID scanClass,
                            PRInt32*    state,
                            unsigned char  *in,
                            PRUint16    *out,
                            PRUint32     inbuflen,
                            PRUint32*    inscanlen
                            );

#define uSubScanner(sub,in,out) (* m_subscanner[sub])((in),(out))

PRIVATE PRBool uCheckAndScanAlways1Byte(
                                        PRInt32*    state,
                                        unsigned char  *in,
                                        PRUint16    *out,
                                        PRUint32     inbuflen,
                                        PRUint32*    inscanlen
                                        );
PRIVATE PRBool uCheckAndScanAlways2Byte(
                                        PRInt32*    state,
                                        unsigned char  *in,
                                        PRUint16    *out,
                                        PRUint32     inbuflen,
                                        PRUint32*    inscanlen
                                        );
PRIVATE PRBool uCheckAndScanAlways2ByteShiftGR(
                                               PRInt32*    state,
                                               unsigned char  *in,
                                               PRUint16    *out,
                                               PRUint32     inbuflen,
                                               PRUint32*    inscanlen
                                               );
PRIVATE PRBool uCheckAndScanAlways2ByteGR128(
                                               PRInt32*    state,
                                               unsigned char  *in,
                                               PRUint16    *out,
                                               PRUint32     inbuflen,
                                               PRUint32*    inscanlen
                                               );
MODULE_PRIVATE PRBool uScanShift(  
                                 uShiftInTable    *shift,
                                 PRInt32*    state,
                                 unsigned char  *in,
                                 PRUint16    *out,
                                 PRUint32     inbuflen,
                                 PRUint32*    inscanlen
                                 );

PRIVATE PRBool uCheckAndScan2ByteGRPrefix8F(
                                            PRInt32*    state,
                                            unsigned char  *in,
                                            PRUint16    *out,
                                            PRUint32     inbuflen,
                                            PRUint32*    inscanlen
                                            );
PRIVATE PRBool uCheckAndScan2ByteGRPrefix8EA2(
                                              PRInt32*    state,
                                              unsigned char  *in,
                                              PRUint16    *out,
                                              PRUint32     inbuflen,
                                              PRUint32*    inscanlen
                                              );
PRIVATE PRBool uCheckAndScan2ByteGRPrefix8EA3(
                                              PRInt32*    state,
                                              unsigned char  *in,
                                              PRUint16    *out,
                                              PRUint32     inbuflen,
                                              PRUint32*    inscanlen
                                              );
PRIVATE PRBool uCheckAndScan2ByteGRPrefix8EA4(
                                              PRInt32*    state,
                                              unsigned char  *in,
                                              PRUint16    *out,
                                              PRUint32     inbuflen,
                                              PRUint32*    inscanlen
                                              );
PRIVATE PRBool uCheckAndScan2ByteGRPrefix8EA5(
                                              PRInt32*    state,
                                              unsigned char  *in,
                                              PRUint16    *out,
                                              PRUint32     inbuflen,
                                              PRUint32*    inscanlen
                                              );
PRIVATE PRBool uCheckAndScan2ByteGRPrefix8EA6(
                                              PRInt32*    state,
                                              unsigned char  *in,
                                              PRUint16    *out,
                                              PRUint32     inbuflen,
                                              PRUint32*    inscanlen
                                              );
PRIVATE PRBool uCheckAndScan2ByteGRPrefix8EA7(
                                              PRInt32*    state,
                                              unsigned char  *in,
                                              PRUint16    *out,
                                              PRUint32     inbuflen,
                                              PRUint32*    inscanlen
                                              );
PRIVATE PRBool uCnSAlways8BytesDecomposedHangul(
                                              PRInt32*    state,
                                              unsigned char  *in,
                                              PRUint16    *out,
                                              PRUint32     inbuflen,
                                              PRUint32*    inscanlen
                                              );
PRIVATE PRBool uCheckAndScanJohabHangul(
                                        PRInt32*    state,
                                        unsigned char  *in,
                                        PRUint16    *out,
                                        PRUint32     inbuflen,
                                        PRUint32*    inscanlen
                                        );
PRIVATE PRBool uCheckAndScanJohabSymbol(
                                        PRInt32*    state,
                                        unsigned char  *in,
                                        PRUint16    *out,
                                        PRUint32     inbuflen,
                                        PRUint32*    inscanlen
                                        );

PRIVATE PRBool uCheckAndScan4BytesGB18030(
                                          PRInt32*    state,
                                          unsigned char  *in,
                                          PRUint16    *out,
                                          PRUint32     inbuflen,
                                          PRUint32*    inscanlen
                                          );

PRIVATE PRBool uScanAlways2Byte(
                                unsigned char*  in,
                                PRUint16*    out
                                );
PRIVATE PRBool uScanAlways2ByteShiftGR(
                                       unsigned char*  in,
                                       PRUint16*    out
                                       );
PRIVATE PRBool uScanAlways1Byte(
                                unsigned char*  in,
                                PRUint16*    out
                                );
PRIVATE PRBool uScanAlways1BytePrefix8E(
                                        unsigned char*  in,
                                        PRUint16*    out
                                        );
                                    /*=================================================================================
                                    
=================================================================================*/
PRIVATE const uScannerFunc m_scanner[uNumOfCharsetType] =
{
    uCheckAndScanAlways1Byte,
    uCheckAndScanAlways2Byte,
    uCheckAndScanAlways2ByteShiftGR,
    uCheckAndScan2ByteGRPrefix8F,
    uCheckAndScan2ByteGRPrefix8EA2,
    uCheckAndScan2ByteGRPrefix8EA3,
    uCheckAndScan2ByteGRPrefix8EA4,
    uCheckAndScan2ByteGRPrefix8EA5,
    uCheckAndScan2ByteGRPrefix8EA6,
    uCheckAndScan2ByteGRPrefix8EA7,
    uCnSAlways8BytesDecomposedHangul,
    uCheckAndScanJohabHangul,
    uCheckAndScanJohabSymbol,
    uCheckAndScan4BytesGB18030,
    uCheckAndScanAlways2ByteGR128
};

/*=================================================================================

=================================================================================*/

PRIVATE const uSubScannerFunc m_subscanner[uNumOfCharType] =
{
    uScanAlways1Byte,
    uScanAlways2Byte,
    uScanAlways2ByteShiftGR,
    uScanAlways1BytePrefix8E
};
/*=================================================================================

=================================================================================*/
MODULE_PRIVATE PRBool uScan(  
                            uScanClassID scanClass,
                            PRInt32*    state,
                            unsigned char  *in,
                            PRUint16    *out,
                            PRUint32     inbuflen,
                            PRUint32*    inscanlen
                            )
{
  return (* m_scanner[scanClass]) (state,in,out,inbuflen,inscanlen);
}
/*=================================================================================

=================================================================================*/
PRIVATE PRBool uScanAlways1Byte(
                                unsigned char*  in,
                                PRUint16*    out
                                )
{
  *out = (PRUint16) in[0];
  return PR_TRUE;
}

/*=================================================================================

=================================================================================*/
PRIVATE PRBool uScanAlways2Byte(
                                unsigned char*  in,
                                PRUint16*    out
                                )
{
  *out = (PRUint16) (( in[0] << 8) | (in[1]));
  return PR_TRUE;
}
/*=================================================================================

=================================================================================*/
PRIVATE PRBool uScanAlways2ByteShiftGR(
                                       unsigned char*  in,
                                       PRUint16*    out
                                       )
{
  *out = (PRUint16) ((( in[0] << 8) | (in[1])) &  0x7F7F);
  return PR_TRUE;
}

/*=================================================================================

=================================================================================*/
PRIVATE PRBool uScanAlways1BytePrefix8E(
                                        unsigned char*  in,
                                        PRUint16*    out
                                        )
{
  *out = (PRUint16) in[1];
  return PR_TRUE;
}
/*=================================================================================

=================================================================================*/
PRIVATE PRBool uCheckAndScanAlways1Byte(
                                        PRInt32*    state,
                                        unsigned char  *in,
                                        PRUint16    *out,
                                        PRUint32     inbuflen,
                                        PRUint32*    inscanlen
                                        )
{
  /* Don't check inlen. The caller should ensure it is larger than 0 */
  *inscanlen = 1;
  *out = (PRUint16) in[0];
  
  return PR_TRUE;
}

/*=================================================================================

=================================================================================*/
PRIVATE PRBool uCheckAndScanAlways2Byte(
                                        PRInt32*    state,
                                        unsigned char  *in,
                                        PRUint16    *out,
                                        PRUint32     inbuflen,
                                        PRUint32*    inscanlen
                                        )
{
  if(inbuflen < 2)
    return PR_FALSE;
  else
  {
    *inscanlen = 2;
    *out = ((in[0] << 8) | ( in[1])) ;
    return PR_TRUE;
  }
}
/*=================================================================================

=================================================================================*/
PRIVATE PRBool uCheckAndScanAlways2ByteShiftGR(
                                               PRInt32*    state,
                                               unsigned char  *in,
                                               PRUint16    *out,
                                               PRUint32     inbuflen,
                                               PRUint32*    inscanlen
                                               )
{
  /*
   * Both bytes should be in the range of [0xa1,0xfe] for 94x94 character sets
   * invoked on GR. No encoding implemented in Mozilla uses 96x96 char. sets.
   * Only 2nd byte range needs to be checked because 
   * 1st byte is checked before calling this in nsUnicodeDecoerHelper.cpp 
   */
  if(inbuflen < 2)    /* will lead to NS_OK_UDEC_MOREINPUT */
    return PR_FALSE;
  else if (! CHK_GR94(in[1]))  
  {
    *inscanlen = 2; 
    *out = 0xFF;  /* for 2-byte table, uMap() is guaranteed to fail for 0xFF. */
    return PR_TRUE;
  }
  else
  {
    *inscanlen = 2;
    *out = (((in[0] << 8) | ( in[1]))  & 0x7F7F);
    return PR_TRUE;
  }
}
/*=================================================================================

=================================================================================*/
PRIVATE PRBool uCheckAndScanAlways2ByteGR128(
                                               PRInt32*    state,
                                               unsigned char  *in,
                                               PRUint16    *out,
                                               PRUint32     inbuflen,
                                               PRUint32*    inscanlen
                                               )
{
  /*
   * The first byte should be in  [0xa1,0xfe] 
   * and the second byte can take any value with MSB = 1.
   * Used by CP949 -> Unicode converter.
   * Only 2nd byte range needs to be checked because 
   * 1st byte is checked before calling this in nsUnicodeDecoerHelper.cpp 
   */
  if(inbuflen < 2)    /* will lead to NS_OK_UDEC_MOREINPUT */
    return PR_FALSE;
  else if (! in[1] & 0x80)     /* 2nd byte range check */
  {
    *inscanlen = 2; 
    *out = 0xFF;  /* for 2-byte table, uMap() is guaranteed to fail for 0xFF. */
    return PR_TRUE;
  }
  else
  {
    *inscanlen = 2;
    *out = (in[0] << 8) |  in[1];
    return PR_TRUE;
  }
}
/*=================================================================================

=================================================================================*/
PRIVATE PRBool uScanShift(
                                    uShiftInTable    *shift,
                                    PRInt32*    state,
                                    unsigned char  *in,
                                    PRUint16    *out,
                                    PRUint32     inbuflen,
                                    PRUint32*    inscanlen
                                    )
{
  PRInt16 i;
  const uShiftInCell* cell = &(shift->shiftcell[0]);
  PRInt16 itemnum = shift->numOfItem;
  for(i=0;i<itemnum;i++)
  {
    if( ( in[0] >=  cell[i].shiftin_Min) &&
      ( in[0] <=  cell[i].shiftin_Max))
    {
      if(inbuflen < cell[i].reserveLen)
        return PR_FALSE;
      else
      {
        *inscanlen = cell[i].reserveLen;
        return (uSubScanner(cell[i].classID,in,out));
      }
    }
  }
  return PR_FALSE;
}
/*=================================================================================

=================================================================================*/
PRIVATE PRBool uCheckAndScan2ByteGRPrefix8F(
                                            PRInt32*    state,
                                            unsigned char  *in,
                                            PRUint16    *out,
                                            PRUint32     inbuflen,
                                            PRUint32*    inscanlen
                                            )
{
  if((inbuflen < 3) ||(in[0] != 0x8F)) 
    return PR_FALSE;
  else if (! CHK_GR94(in[1]))  /* 2nd byte range check */
  {
    *inscanlen = 2; 
    *out = 0xFF;  /* for 2-byte table, uMap() is guaranteed to fail for 0xFF. */
    return PR_TRUE;
  }
  else if (! CHK_GR94(in[2]))  /* 3rd byte range check */
  {
    *inscanlen = 3; 
    *out = 0xFF;  /* for 2-byte table, uMap() is guaranteed to fail for 0xFF. */
    return PR_TRUE;
  }
  else
  {
    *inscanlen = 3;
    *out = (((in[1] << 8) | ( in[2]))  & 0x7F7F);
    return PR_TRUE;
  }
}
/*=================================================================================

=================================================================================*/

/* Macro definition to use for uCheckAndScan2ByteGRPrefix8EAX()
 * where X is 2,3,4,5,6,7 
 */
#define CNS_8EAX_4BYTE(PREFIX)                    \
  if((inbuflen < 4) || (in[0] != 0x8E))           \
    return PR_FALSE;                              \
  else if((in[1] != (PREFIX)))                    \
  {                                               \
    *inscanlen = 2;                               \
    *out = 0xFF;                                  \
    return PR_TRUE;                               \
  }                                               \
  else if(! CHK_GR94(in[2]))                      \
  {                                               \
    *inscanlen = 3;                               \
    *out = 0xFF;                                  \
    return PR_TRUE;                               \
  }                                               \
  else if(! CHK_GR94(in[3]))                      \
  {                                               \
    *inscanlen = 4;                               \
    *out = 0xFF;                                  \
    return PR_TRUE;                               \
  }                                               \
  else                                            \
  {                                               \
    *inscanlen = 4;                               \
    *out = (((in[2] << 8) | ( in[3]))  & 0x7F7F); \
    return PR_TRUE;                               \
  }    

PRIVATE PRBool uCheckAndScan2ByteGRPrefix8EA2(
                                              PRInt32*    state,
                                              unsigned char  *in,
                                              PRUint16    *out,
                                              PRUint32     inbuflen,
                                              PRUint32*    inscanlen
                                              )
{
  CNS_8EAX_4BYTE(0xA2)
}

/*=================================================================================

=================================================================================*/
PRIVATE PRBool uCheckAndScan2ByteGRPrefix8EA3(
                                              PRInt32*    state,
                                              unsigned char  *in,
                                              PRUint16    *out,
                                              PRUint32     inbuflen,
                                              PRUint32*    inscanlen
                                              )
{
  CNS_8EAX_4BYTE(0xA3)
}
/*=================================================================================

=================================================================================*/
PRIVATE PRBool uCheckAndScan2ByteGRPrefix8EA4(
                                              PRInt32*    state,
                                              unsigned char  *in,
                                              PRUint16    *out,
                                              PRUint32     inbuflen,
                                              PRUint32*    inscanlen
                                              )
{
  CNS_8EAX_4BYTE(0xA4)
}
/*=================================================================================

=================================================================================*/
PRIVATE PRBool uCheckAndScan2ByteGRPrefix8EA5(
                                              PRInt32*    state,
                                              unsigned char  *in,
                                              PRUint16    *out,
                                              PRUint32     inbuflen,
                                              PRUint32*    inscanlen
                                              )
{
  CNS_8EAX_4BYTE(0xA5)
}
/*=================================================================================

=================================================================================*/
PRIVATE PRBool uCheckAndScan2ByteGRPrefix8EA6(
                                              PRInt32*    state,
                                              unsigned char  *in,
                                              PRUint16    *out,
                                              PRUint32     inbuflen,
                                              PRUint32*    inscanlen
                                              )
{
  CNS_8EAX_4BYTE(0xA6)
}
/*=================================================================================

=================================================================================*/
PRIVATE PRBool uCheckAndScan2ByteGRPrefix8EA7(
                                              PRInt32*    state,
                                              unsigned char  *in,
                                              PRUint16    *out,
                                              PRUint32     inbuflen,
                                              PRUint32*    inscanlen
                                              )
{
  CNS_8EAX_4BYTE(0xA7)
}
/*=================================================================================

=================================================================================*/
#define SBase 0xAC00
#define SCount 11172
#define LCount 19
#define VCount 21
#define TCount 28
#define NCount (VCount * TCount)

PRIVATE PRBool uCnSAlways8BytesDecomposedHangul(
                                              PRInt32*    state,
                                              unsigned char  *in,
                                              PRUint16    *out,
                                              PRUint32     inbuflen,
                                              PRUint32*    inscanlen
                                              )
{
  
  PRUint16 LIndex, VIndex, TIndex;
  /* no 8 bytes, not in a4 range, or the first 2 byte are not a4d4 */
  if((inbuflen < 8) || (0xa4 != in[0]) || (0xd4 != in[1]) ||
    (0xa4 != in[2] ) || (0xa4 != in[4]) || (0xa4 != in[6]))
    return PR_FALSE;
  
  /* Compute LIndex  */
  if((in[3] < 0xa1) && (in[3] > 0xbe)) { /* illegal leading consonant */
    return PR_FALSE;
  } 
  else {
    static const PRUint8 lMap[] = {
      /*        A1   A2   A3   A4   A5   A6   A7  */
      0,   1,0xff,   2,0xff,0xff,   3,
        /*   A8   A9   AA   AB   AC   AD   AE   AF  */
        4,   5,0xff,0xff,0xff,0xff,0xff,0xff,
        /*   B0   B1   B2   B3   B4   B5   B6   B7  */
        0xff,   6,   7,   8,0xff,   9,  10,  11,
        /*   B8   B9   BA   BB   BC   BD   BE       */
        12,  13,  14,  15,  16,  17,  18     
    };
    
    LIndex = lMap[in[3] - 0xa1];
    if(0xff == (0xff & LIndex))
      return PR_FALSE;
  }
  
  /* Compute VIndex  */
  if((in[5] < 0xbf) && (in[5] > 0xd3)) { /* illegal medial vowel */
    return PR_FALSE;
  } 
  else {
    VIndex = in[5] - 0xbf;
  }
  
  /* Compute TIndex  */
  if(0xd4 == in[7])  
  {
    TIndex = 0;
  } 
  else if((in[7] < 0xa1) && (in[7] > 0xbe)) {/* illegal trailling consonant */
    return PR_FALSE;
  } 
  else {
    static const PRUint8 tMap[] = {
      /*        A1   A2   A3   A4   A5   A6   A7  */
      1,   2,   3,   4,   5,   6,   7,
        /*   A8   A9   AA   AB   AC   AD   AE   AF  */
        0xff,   8,   9,  10,  11,  12,  13,  14,
        /*   B0   B1   B2   B3   B4   B5   B6   B7  */
        15,  16,  17,0xff,  18,  19,  20,  21,
        /*   B8   B9   BA   BB   BC   BD   BE       */
        22,0xff,  23,  24,  25,  26,  27     
    };
    TIndex = tMap[in[7] - 0xa1];
    if(0xff == (0xff & TIndex))
      return PR_FALSE;
  }
  
  *inscanlen = 8;
  /* the following line is from Unicode 2.0 page 3-13 item 5 */
  *out = ( LIndex * VCount + VIndex) * TCount + TIndex + SBase;
  
  return PR_TRUE;
}
/*=================================================================================

=================================================================================*/

PRIVATE PRBool uCheckAndScanJohabHangul(
                                        PRInt32*    state,
                                        unsigned char  *in,
                                        PRUint16    *out,
                                        PRUint32     inbuflen,
                                        PRUint32*    inscanlen
                                        )
{
/* since we don't have code to convert Johab to Unicode right now     *
  * make this part of code #if 0 to save space untill we fully test it */
  if(inbuflen < 2)
    return PR_FALSE;
  else {
  /*
  * See Table 4-45 Johab Encoding's Five-Bit Binary Patterns in page 183
  * of "CJKV Information Processing" for details
    */
    static const PRUint8 lMap[32]={ /* totaly 19  */
      0xff,0xff,0,   1,   2,   3,   4,   5,    /* 0-7    */
        6,   7,   8,   9,   10,  11,  12,  13,   /* 8-15   */
        14,  15,  16,  17,  18,  0xff,0xff,0xff, /* 16-23  */
        0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff  /* 24-31  */
    };
    static const PRUint8 vMap[32]={ /* totaly 21 */
      0xff,0xff,0xff,0,   1,   2,   3,   4,    /* 0-7   */
        0xff,0xff,5,   6,   7,   8,   9,   10,   /* 8-15  */
        0xff,0xff,11,  12,  13,  14,  15,  16,   /* 16-23 */
        0xff,0xff,17,  18,  19,  20,  0xff,0xff  /* 24-31 */
    };
    static const PRUint8 tMap[32]={ /* totaly 29 */
      0xff,0,   1,   2,   3,   4,   5,   6,    /* 0-7   */
        7,   8,   9,   10,  11,  12,  13,  14,   /* 8-15  */
        15,  16,  0xff,17,  18,  19,  20,  21,   /* 16-23 */
        22,  23,  24,  25,  26,  27,  0xff,0xff  /* 24-31 */
    };
    PRUint16 ch = (in[0] << 8) | in[1];
    PRUint16 LIndex, VIndex, TIndex;
    if(0 == (0x8000 & ch))
      return PR_FALSE;
    LIndex=lMap[(ch>>10)& 0x1F];
    VIndex=vMap[(ch>>5) & 0x1F];
    TIndex=tMap[(ch>>0) & 0x1F];
    if((0xff==(LIndex)) || 
      (0xff==(VIndex)) || 
      (0xff==(TIndex)))
      return PR_FALSE;
    /* the following line is from Unicode 2.0 page 3-13 item 5 */
    *out = ( LIndex * VCount + VIndex) * TCount + TIndex + SBase;
    *inscanlen = 2;
    return PR_TRUE;
  }
}
PRIVATE PRBool uCheckAndScanJohabSymbol(
                                        PRInt32*    state,
                                        unsigned char  *in,
                                        PRUint16    *out,
                                        PRUint32     inbuflen,
                                        PRUint32*    inscanlen
                                        )
{
  if(inbuflen < 2)
    return PR_FALSE;
  else {
  /*
  * The following code are based on the Perl code lised under
  * "Johab to ISO-2022-KR or EUC-KR Conversion" in page 1014 of
  * "CJKV Information Processing" by Ken Lunde <lunde@adobe.com>
  *
  * sub johab2ks ($) { # Convert Johab to ISO-2022-KR
  *   my @johab = unpack("C*", $_[0]);
  *   my ($offset, $d8_off) = (0,0);
  *   my @out = ();
  *   while(($hi, $lo) = splice($johab, 0, 2)) {
  *     $offset = 1 if ($hi > 223 and $hi < 250);
  *     $d8_off = ($hi == 216 and ($lo > 160 ? 94 : 42));
  *     push (@out, (((($hi - ($hi < 223 ? 200 : 187)) << 1) -
  *            ($lo < 161 ? 1 : 0) + $offset) + $d8_off),
  *            $lo - ($lo < 161 ? ($lo > 126 ? 34 : 16) : 128 ));
  *   }
  *   return pack ("C*", @out);
  * }
  * additional comments from Ken Lunde
  * $d8_off = ($hi == 216 and ($lo > 160 ? 94 : 42));
  * has three possible return values:
  * 0  if $hi is not equal to 216
  * 94 if $hi is euqal to 216 and if $lo is greater than 160
  * 42 if $hi is euqal to 216 and if $lo is not greater than 160
    */ 
    unsigned char hi = in[0];
    unsigned char lo = in[1];
    PRUint16 offset = (( hi > 223 ) && ( hi < 250)) ? 1 : 0;
    PRUint16 d8_off = 0;
    if(216 == hi) {
      if( lo > 160)
        d8_off = 94;
      else
        d8_off = 42;
    }
    
    *out = (((((hi - ((hi < 223) ? 200 : 187)) << 1) -
      (lo < 161 ? 1 : 0) + offset) + d8_off) << 8 ) |
      (lo - ((lo < 161) ? ((lo > 126) ? 34 : 16) : 
    128));
    *inscanlen = 2;
    return PR_TRUE;
  }
}
PRIVATE PRBool uCheckAndScan4BytesGB18030(
                                          PRInt32*    state,
                                          unsigned char  *in,
                                          PRUint16    *out,
                                          PRUint32     inbuflen,
                                          PRUint32*    inscanlen
                                          )
{
  PRUint32  data;
  if(inbuflen < 4) 
    return PR_FALSE;
  
  if((in[0] < 0x81 ) || (0xfe < in[0])) 
    return PR_FALSE;
  if((in[1] < 0x30 ) || (0x39 < in[1])) 
    return PR_FALSE;
  if((in[2] < 0x81 ) || (0xfe < in[2])) 
    return PR_FALSE;
  if((in[3] < 0x30 ) || (0x39 < in[3])) 
    return PR_FALSE;
  
  data = (((((in[0] - 0x81) * 10 + (in[1] - 0x30)) * 126) + 
    (in[2] - 0x81)) * 10 ) + (in[3] - 0x30);
  
  *inscanlen = 4;
  *out = (data < 0x00010000) ? data : 0xFFFD;
  return PR_TRUE;
}
