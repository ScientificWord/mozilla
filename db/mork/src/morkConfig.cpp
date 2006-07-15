/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-  */
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

#ifndef _MDB_
#include "mdb.h"
#endif

#ifndef _MORK_
#include "mork.h"
#endif

#ifndef _MORKCONFIG_
#include "morkConfig.h"
#endif

//3456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789

void mork_assertion_signal(const char* inMessage)
{
#if defined(MORK_WIN) || defined(MORK_MAC)
  // asm { int 3 }
  NS_ASSERTION(0, inMessage);
#endif /*MORK_WIN*/
}

#if defined(MORK_OS2)
#include <fcntl.h>
#include <sys/stat.h>
#include <share.h>
#include <io.h>

FILE* mork_fileopen(const char* name, const char* mode)
{
    int access = O_RDWR;
    int descriptor;
    int pmode = 0;

    /* Only possible options are wb+ and rb+ */
    MORK_ASSERT((mode[0] == 'w' || mode[0] == 'r') && (mode[1] == 'b') && (mode[2] == '+'));
    if (mode[0] == 'w') {
        access |= (O_TRUNC | O_CREAT);
        pmode = S_IREAD | S_IWRITE;
    }

    descriptor = sopen(name, access, SH_DENYNO, pmode);
    if (descriptor != -1) {
        return fdopen(descriptor, mode);
    }
    return NULL;
}
#endif

#ifdef MORK_PROVIDE_STDLIB

MORK_LIB_IMPL(mork_i4)
mork_memcmp(const void* inOne, const void* inTwo, mork_size inSize)
{
  register const mork_u1* t = (const mork_u1*) inTwo;
  register const mork_u1* s = (const mork_u1*) inOne;
  const mork_u1* end = s + inSize;
  register mork_i4 delta;
  
  while ( s < end )
  {
    delta = ((mork_i4) *s) - ((mork_i4) *t);
    if ( delta )
      return delta;
    else
    {
      ++t;
      ++s;
    }
  }
  return 0;
}

MORK_LIB_IMPL(void)
mork_memcpy(void* outDst, const void* inSrc, mork_size inSize)
{
  register mork_u1* d = (mork_u1*) outDst;
  mork_u1* end = d + inSize;
  register const mork_u1* s = ((const mork_u1*) inSrc);
  
  while ( inSize >= 8 )
  {
    *d++ = *s++;
    *d++ = *s++;
    *d++ = *s++;
    *d++ = *s++;
    
    *d++ = *s++;
    *d++ = *s++;
    *d++ = *s++;
    *d++ = *s++;
    
    inSize -= 8;
  }
  
  while ( d < end )
    *d++ = *s++;
}

MORK_LIB_IMPL(void)
mork_memmove(void* outDst, const void* inSrc, mork_size inSize)
{
  register mork_u1* d = (mork_u1*) outDst;
  register const mork_u1* s = (const mork_u1*) inSrc;
  if ( d != s && inSize ) // copy is necessary?
  {
    const mork_u1* srcEnd = s + inSize; // one past last source byte
    
    if ( d > s && d < srcEnd ) // overlap? need to copy backwards?
    {
      s = srcEnd; // start one past last source byte
      d += inSize; // start one past last dest byte
      mork_u1* dstBegin = d; // last byte to write is first in dest range
      while ( d - dstBegin >= 8 )
      {
        *--d = *--s;
        *--d = *--s;
        *--d = *--s;
        *--d = *--s;
        
        *--d = *--s;
        *--d = *--s;
        *--d = *--s;
        *--d = *--s;
      }
      while ( d > dstBegin )
        *--d = *--s;
    }
    else // can copy forwards without any overlap
    {
      mork_u1* dstEnd = d + inSize;
      while ( dstEnd - d >= 8 )
      {
        *d++ = *s++;
        *d++ = *s++;
        *d++ = *s++;
        *d++ = *s++;
        
        *d++ = *s++;
        *d++ = *s++;
        *d++ = *s++;
        *d++ = *s++;
      }
      while ( d < dstEnd )
        *d++ = *s++;
    }
  }
}

MORK_LIB_IMPL(void)
mork_memset(void* outDst, int inByte, mork_size inSize)
{
  register mork_u1* d = (mork_u1*) outDst;
  mork_u1* end = d + inSize;
  while ( d < end )
    *d++ = (mork_u1) inByte;
}

MORK_LIB_IMPL(void)
mork_strcpy(void* outDst, const void* inSrc)
{
  // back up one first to support preincrement
  register mork_u1* d = ((mork_u1*) outDst) - 1;
  register const mork_u1* s = ((const mork_u1*) inSrc) - 1;
  while ( ( *++d = *++s ) != 0 )
    /* empty */;
}

MORK_LIB_IMPL(mork_i4)
mork_strcmp(const void* inOne, const void* inTwo)
{
  register const mork_u1* t = (const mork_u1*) inTwo;
  register const mork_u1* s = ((const mork_u1*) inOne);
  register mork_i4 a;
  register mork_i4 b;
  register mork_i4 delta;
  
  do
  {
    a = (mork_i4) *s++;
    b = (mork_i4) *t++;
    delta = a - b;
  }
  while ( !delta && a && b );
  
  return delta;
}

MORK_LIB_IMPL(mork_i4)
mork_strncmp(const void* inOne, const void* inTwo, mork_size inSize)
{
  register const mork_u1* t = (const mork_u1*) inTwo;
  register const mork_u1* s = (const mork_u1*) inOne;
  const mork_u1* end = s + inSize;
  register mork_i4 delta;
  register mork_i4 a;
  register mork_i4 b;
  
  while ( s < end )
  {
    a = (mork_i4) *s++;
    b = (mork_i4) *t++;
    delta = a - b;
    if ( delta || !a || !b )
      return delta;
  }
  return 0;
}

MORK_LIB_IMPL(mork_size)
mork_strlen(const void* inString)
{
  // back up one first to support preincrement
  register const mork_u1* s = ((const mork_u1*) inString) - 1;
  while ( *++s ) // preincrement is cheapest
    /* empty */;
  
  return s - ((const mork_u1*) inString); // distance from original address
}

#endif /*MORK_PROVIDE_STDLIB*/

//3456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789
