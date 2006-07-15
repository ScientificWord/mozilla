/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

/* Platform specific code to invoke XPCOM methods on native objects */

#include "xptcprivate.h"

#ifndef AIX
#error "This code is for PowerPC only"
#endif

extern "C" void
invoke_copy_to_stack(PRUint32* d, PRUint32 paramCount, nsXPTCVariant* s, double *fprData)
{
/*
    We need to copy the parameters for this function to locals and use them
    from there since the parameters occupy the same stack space as the stack
    we're trying to populate.
*/
    PRUint32 *l_d = d;
    nsXPTCVariant *l_s = s;
    PRUint32 l_paramCount = paramCount, fpCount = 0;
    double *l_fprData = fprData;

    typedef struct {
        uint32 hi;
        uint32 lo;
    } DU;               // have to move 64 bit entities as 32 bit halves since
                        // stack slots are not guaranteed 16 byte aligned

    for(uint32 i = 0; i < l_paramCount; i++, l_d++, l_s++)
    {
        if(l_s->IsPtrData())
        {
            *((void**)l_d) = l_s->ptr;
            continue;
        }
        switch(l_s->type)
        {
        case nsXPTType::T_I8     : *((int32*)  l_d) = l_s->val.i8;          break;
        case nsXPTType::T_I16    : *((int32*)  l_d) = l_s->val.i16;         break;
        case nsXPTType::T_I32    : *((int32*)  l_d) = l_s->val.i32;         break;
        case nsXPTType::T_I64    : 
        case nsXPTType::T_U64    :
            *((uint32*) l_d++) = ((DU *)l_s)->hi;
            *((uint32*) l_d) = ((DU *)l_s)->lo;
            break;
        case nsXPTType::T_DOUBLE :
            *((uint32*) l_d++) = ((DU *)l_s)->hi;
            *((uint32*) l_d) = ((DU *)l_s)->lo;
            if(fpCount < 13)
                l_fprData[fpCount++] = l_s->val.d;
            break;
        case nsXPTType::T_U8     : *((uint32*) l_d) = l_s->val.u8;          break;
        case nsXPTType::T_U16    : *((uint32*) l_d) = l_s->val.u16;         break;
        case nsXPTType::T_U32    : *((uint32*) l_d) = l_s->val.u32;         break;
        case nsXPTType::T_FLOAT  :
            *((float*)  l_d) = l_s->val.f;
            if(fpCount < 13)
                l_fprData[fpCount++] = l_s->val.f;
            break;
        case nsXPTType::T_BOOL   : *((PRBool*) l_d) = l_s->val.b;           break;
        case nsXPTType::T_CHAR   : *((uint32*) l_d) = l_s->val.c;           break;
        case nsXPTType::T_WCHAR  : *((int32*)  l_d) = l_s->val.wc;          break;
        default:
            // all the others are plain pointer types
            *((void**)l_d) = l_s->val.p;
            break;
        }
    }
}

