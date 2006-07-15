/*
 *  des.h
 *
 *  header file for DES-150 library
 *
 * ***** BEGIN LICENSE BLOCK *****
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
 * The Original Code is the DES-150 library.
 *
 * The Initial Developer of the Original Code is
 * Nelson B. Bolyard, nelsonb@iname.com.
 * Portions created by the Initial Developer are Copyright (C) 1990
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
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

#ifndef _DES_H_
#define _DES_H_ 1

#include "blapi.h"

typedef unsigned char BYTE;
typedef unsigned int  HALF;

#define HALFPTR(x) ((HALF *)(x))
#define SHORTPTR(x) ((unsigned short *)(x))
#define BYTEPTR(x) ((BYTE *)(x))

typedef enum {
    DES_ENCRYPT = 0x5555,
    DES_DECRYPT = 0xAAAA
} DESDirection;

typedef void DESFunc(struct DESContextStr *cx, BYTE *out, const BYTE *in, 
                     unsigned int len);

struct DESContextStr {
    /* key schedule, 16 internal keys, each with 8 6-bit parts */
    HALF ks0 [32];
    HALF ks1 [32];
    HALF ks2 [32];
    HALF iv  [2];
    DESDirection direction;
    DESFunc  *worker;
};

void DES_MakeSchedule( HALF * ks, const BYTE * key,   DESDirection direction);
void DES_Do1Block(     HALF * ks, const BYTE * inbuf, BYTE * outbuf);

#endif
