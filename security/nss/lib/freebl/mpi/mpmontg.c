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
 * The Original Code is the Netscape security libraries.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2000
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Sheueling Chang Shantz <sheueling.chang@sun.com>,
 *   Stephen Fung <stephen.fung@sun.com>, and
 *   Douglas Stebila <douglas@stebila.ca> of Sun Laboratories.
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
/* $Id$ */

/* This file implements moduluar exponentiation using Montgomery's
 * method for modular reduction.  This file implements the method
 * described as "Improvement 1" in the paper "A Cryptogrpahic Library for
 * the Motorola DSP56000" by Stephen R. Dusse' and Burton S. Kaliski Jr.
 * published in "Advances in Cryptology: Proceedings of EUROCRYPT '90"
 * "Lecture Notes in Computer Science" volume 473, 1991, pg 230-244,
 * published by Springer Verlag.
 */

#define MP_USING_CACHE_SAFE_MOD_EXP 1 
#include <string.h>
#include "mpi-priv.h"
#include "mplogic.h"
#include "mpprime.h"
#ifdef MP_USING_MONT_MULF
#include "montmulf.h"
#endif
#include <stddef.h> /* ptrdiff_t */

/* if MP_CHAR_STORE_SLOW is defined, we  */
/* need to know endianness of this platform. */
#ifdef MP_CHAR_STORE_SLOW
#if !defined(MP_IS_BIG_ENDIAN) && !defined(MP_IS_LITTLE_ENDIAN)
#error "You must define MP_IS_BIG_ENDIAN or MP_IS_LITTLE_ENDIAN\n" \
       "  if you define MP_CHAR_STORE_SLOW."
#endif
#endif

#define STATIC

#define MAX_ODD_INTS    32   /* 2 ** (WINDOW_BITS - 1) */

#if defined(_WIN32_WCE)
#define ABORT  res = MP_UNDEF; goto CLEANUP
#else
#define ABORT abort()
#endif

/* computes T = REDC(T), 2^b == R */
mp_err s_mp_redc(mp_int *T, mp_mont_modulus *mmm)
{
  mp_err res;
  mp_size i;

  i = MP_USED(T) + MP_USED(&mmm->N) + 2;
  MP_CHECKOK( s_mp_pad(T, i) );
  for (i = 0; i < MP_USED(&mmm->N); ++i ) {
    mp_digit m_i = MP_DIGIT(T, i) * mmm->n0prime;
    /* T += N * m_i * (MP_RADIX ** i); */
    MP_CHECKOK( s_mp_mul_d_add_offset(&mmm->N, m_i, T, i) );
  }
  s_mp_clamp(T);

  /* T /= R */
  s_mp_div_2d(T, mmm->b); 

  if ((res = s_mp_cmp(T, &mmm->N)) >= 0) {
    /* T = T - N */
    MP_CHECKOK( s_mp_sub(T, &mmm->N) );
#ifdef DEBUG
    if ((res = mp_cmp(T, &mmm->N)) >= 0) {
      res = MP_UNDEF;
      goto CLEANUP;
    }
#endif
  }
  res = MP_OKAY;
CLEANUP:
  return res;
}

#if !defined(MP_ASSEMBLY_MUL_MONT) && !defined(MP_MONT_USE_MP_MUL)
mp_err s_mp_mul_mont(const mp_int *a, const mp_int *b, mp_int *c, 
	           mp_mont_modulus *mmm)
{
  mp_digit *pb;
  mp_digit m_i;
  mp_err   res;
  mp_size  ib;
  mp_size  useda, usedb;

  ARGCHK(a != NULL && b != NULL && c != NULL, MP_BADARG);

  if (MP_USED(a) < MP_USED(b)) {
    const mp_int *xch = b;	/* switch a and b, to do fewer outer loops */
    b = a;
    a = xch;
  }

  MP_USED(c) = 1; MP_DIGIT(c, 0) = 0;
  ib = MP_USED(a) + MP_MAX(MP_USED(b), MP_USED(&mmm->N)) + 2;
  if((res = s_mp_pad(c, ib)) != MP_OKAY)
    goto CLEANUP;

  useda = MP_USED(a);
  pb = MP_DIGITS(b);
  s_mpv_mul_d(MP_DIGITS(a), useda, *pb++, MP_DIGITS(c));
  s_mp_setz(MP_DIGITS(c) + useda + 1, ib - (useda + 1));
  m_i = MP_DIGIT(c, 0) * mmm->n0prime;
  s_mp_mul_d_add_offset(&mmm->N, m_i, c, 0);

  /* Outer loop:  Digits of b */
  usedb = MP_USED(b);
  for (ib = 1; ib < usedb; ib++) {
    mp_digit b_i    = *pb++;

    /* Inner product:  Digits of a */
    if (b_i)
      s_mpv_mul_d_add_prop(MP_DIGITS(a), useda, b_i, MP_DIGITS(c) + ib);
    m_i = MP_DIGIT(c, ib) * mmm->n0prime;
    s_mp_mul_d_add_offset(&mmm->N, m_i, c, ib);
  }
  if (usedb < MP_USED(&mmm->N)) {
    for (usedb = MP_USED(&mmm->N); ib < usedb; ++ib ) {
      m_i = MP_DIGIT(c, ib) * mmm->n0prime;
      s_mp_mul_d_add_offset(&mmm->N, m_i, c, ib);
    }
  }
  s_mp_clamp(c);
  s_mp_div_2d(c, mmm->b); 
  if (s_mp_cmp(c, &mmm->N) >= 0) {
    MP_CHECKOK( s_mp_sub(c, &mmm->N) );
  }
  res = MP_OKAY;

CLEANUP:
  return res;
}
#endif

STATIC
mp_err s_mp_to_mont(const mp_int *x, mp_mont_modulus *mmm, mp_int *xMont)
{
  mp_err res;

  /* xMont = x * R mod N   where  N is modulus */
  MP_CHECKOK( mpl_lsh(x, xMont, mmm->b) );  		/* xMont = x << b */
  MP_CHECKOK( mp_div(xMont, &mmm->N, 0, xMont) );	/*         mod N */
CLEANUP:
  return res;
}

#ifdef MP_USING_MONT_MULF

/* the floating point multiply is already cache safe,
 * don't turn on cache safe unless we specifically
 * force it */
#ifndef MP_FORCE_CACHE_SAFE
#undef MP_USING_CACHE_SAFE_MOD_EXP
#endif

unsigned int mp_using_mont_mulf = 1;

/* computes montgomery square of the integer in mResult */
#define SQR \
  conv_i32_to_d32_and_d16(dm1, d16Tmp, mResult, nLen); \
  mont_mulf_noconv(mResult, dm1, d16Tmp, \
		   dTmp, dn, MP_DIGITS(modulus), nLen, dn0)

/* computes montgomery product of x and the integer in mResult */
#define MUL(x) \
  conv_i32_to_d32(dm1, mResult, nLen); \
  mont_mulf_noconv(mResult, dm1, oddPowers[x], \
		   dTmp, dn, MP_DIGITS(modulus), nLen, dn0)

/* Do modular exponentiation using floating point multiply code. */
mp_err mp_exptmod_f(const mp_int *   montBase, 
                    const mp_int *   exponent, 
		    const mp_int *   modulus, 
		    mp_int *         result, 
		    mp_mont_modulus *mmm, 
		    int              nLen, 
		    mp_size          bits_in_exponent, 
		    mp_size          window_bits,
		    mp_size          odd_ints)
{
  mp_digit *mResult;
  double   *dBuf = 0, *dm1, *dn, *dSqr, *d16Tmp, *dTmp;
  double    dn0;
  mp_size   i;
  mp_err    res;
  int       expOff;
  int       dSize = 0, oddPowSize, dTmpSize;
  mp_int    accum1;
  double   *oddPowers[MAX_ODD_INTS];

  /* function for computing n0prime only works if n0 is odd */

  MP_DIGITS(&accum1) = 0;

  for (i = 0; i < MAX_ODD_INTS; ++i)
    oddPowers[i] = 0;

  MP_CHECKOK( mp_init_size(&accum1, 3 * nLen + 2) );

  mp_set(&accum1, 1);
  MP_CHECKOK( s_mp_to_mont(&accum1, mmm, &accum1) );
  MP_CHECKOK( s_mp_pad(&accum1, nLen) );

  oddPowSize = 2 * nLen + 1;
  dTmpSize   = 2 * oddPowSize;
  dSize = sizeof(double) * (nLen * 4 + 1 + 
			    ((odd_ints + 1) * oddPowSize) + dTmpSize);
  dBuf   = (double *)malloc(dSize);
  dm1    = dBuf;		/* array of d32 */
  dn     = dBuf   + nLen;	/* array of d32 */
  dSqr   = dn     + nLen;    	/* array of d32 */
  d16Tmp = dSqr   + nLen;	/* array of d16 */
  dTmp   = d16Tmp + oddPowSize;

  for (i = 0; i < odd_ints; ++i) {
      oddPowers[i] = dTmp;
      dTmp += oddPowSize;
  }
  mResult = (mp_digit *)(dTmp + dTmpSize);	/* size is nLen + 1 */

  /* Make dn and dn0 */
  conv_i32_to_d32(dn, MP_DIGITS(modulus), nLen);
  dn0 = (double)(mmm->n0prime & 0xffff);

  /* Make dSqr */
  conv_i32_to_d32_and_d16(dm1, oddPowers[0], MP_DIGITS(montBase), nLen);
  mont_mulf_noconv(mResult, dm1, oddPowers[0], 
		   dTmp, dn, MP_DIGITS(modulus), nLen, dn0);
  conv_i32_to_d32(dSqr, mResult, nLen);

  for (i = 1; i < odd_ints; ++i) {
    mont_mulf_noconv(mResult, dSqr, oddPowers[i - 1], 
		     dTmp, dn, MP_DIGITS(modulus), nLen, dn0);
    conv_i32_to_d16(oddPowers[i], mResult, nLen);
  }

  s_mp_copy(MP_DIGITS(&accum1), mResult, nLen); /* from, to, len */

  for (expOff = bits_in_exponent - window_bits; expOff >= 0; expOff -= window_bits) {
    mp_size smallExp;
    MP_CHECKOK( mpl_get_bits(exponent, expOff, window_bits) );
    smallExp = (mp_size)res;

    if (window_bits == 1) {
      if (!smallExp) {
	SQR;
      } else if (smallExp & 1) {
	SQR; MUL(0); 
      } else {
	ABORT;
      }
    } else if (window_bits == 4) {
      if (!smallExp) {
	SQR; SQR; SQR; SQR;
      } else if (smallExp & 1) {
	SQR; SQR; SQR; SQR; MUL(smallExp/2); 
      } else if (smallExp & 2) {
	SQR; SQR; SQR; MUL(smallExp/4); SQR; 
      } else if (smallExp & 4) {
	SQR; SQR; MUL(smallExp/8); SQR; SQR; 
      } else if (smallExp & 8) {
	SQR; MUL(smallExp/16); SQR; SQR; SQR; 
      } else {
	ABORT;
      }
    } else if (window_bits == 5) {
      if (!smallExp) {
	SQR; SQR; SQR; SQR; SQR; 
      } else if (smallExp & 1) {
	SQR; SQR; SQR; SQR; SQR; MUL(smallExp/2);
      } else if (smallExp & 2) {
	SQR; SQR; SQR; SQR; MUL(smallExp/4); SQR;
      } else if (smallExp & 4) {
	SQR; SQR; SQR; MUL(smallExp/8); SQR; SQR;
      } else if (smallExp & 8) {
	SQR; SQR; MUL(smallExp/16); SQR; SQR; SQR;
      } else if (smallExp & 0x10) {
	SQR; MUL(smallExp/32); SQR; SQR; SQR; SQR;
      } else {
	ABORT;
      }
    } else if (window_bits == 6) {
      if (!smallExp) {
	SQR; SQR; SQR; SQR; SQR; SQR;
      } else if (smallExp & 1) {
	SQR; SQR; SQR; SQR; SQR; SQR; MUL(smallExp/2); 
      } else if (smallExp & 2) {
	SQR; SQR; SQR; SQR; SQR; MUL(smallExp/4); SQR; 
      } else if (smallExp & 4) {
	SQR; SQR; SQR; SQR; MUL(smallExp/8); SQR; SQR; 
      } else if (smallExp & 8) {
	SQR; SQR; SQR; MUL(smallExp/16); SQR; SQR; SQR; 
      } else if (smallExp & 0x10) {
	SQR; SQR; MUL(smallExp/32); SQR; SQR; SQR; SQR; 
      } else if (smallExp & 0x20) {
	SQR; MUL(smallExp/64); SQR; SQR; SQR; SQR; SQR; 
      } else {
	ABORT;
      }
    } else {
      ABORT;
    }
  }

  s_mp_copy(mResult, MP_DIGITS(&accum1), nLen); /* from, to, len */

  res = s_mp_redc(&accum1, mmm);
  mp_exch(&accum1, result);

CLEANUP:
  mp_clear(&accum1);
  if (dBuf) {
    if (dSize)
      memset(dBuf, 0, dSize);
    free(dBuf);
  }

  return res;
}
#undef SQR
#undef MUL
#endif

#define SQR(a,b) \
  MP_CHECKOK( mp_sqr(a, b) );\
  MP_CHECKOK( s_mp_redc(b, mmm) )

#if defined(MP_MONT_USE_MP_MUL)
#define MUL(x,a,b) \
  MP_CHECKOK( mp_mul(a, oddPowers + (x), b) ); \
  MP_CHECKOK( s_mp_redc(b, mmm) ) 
#else
#define MUL(x,a,b) \
  MP_CHECKOK( s_mp_mul_mont(a, oddPowers + (x), b, mmm) )
#endif

#define SWAPPA ptmp = pa1; pa1 = pa2; pa2 = ptmp

/* Do modular exponentiation using integer multiply code. */
mp_err mp_exptmod_i(const mp_int *   montBase, 
                    const mp_int *   exponent, 
		    const mp_int *   modulus, 
		    mp_int *         result, 
		    mp_mont_modulus *mmm, 
		    int              nLen, 
		    mp_size          bits_in_exponent, 
		    mp_size          window_bits,
		    mp_size          odd_ints)
{
  mp_int *pa1, *pa2, *ptmp;
  mp_size i;
  mp_err  res;
  int     expOff;
  mp_int  accum1, accum2, power2, oddPowers[MAX_ODD_INTS];

  /* power2 = base ** 2; oddPowers[i] = base ** (2*i + 1); */
  /* oddPowers[i] = base ** (2*i + 1); */

  MP_DIGITS(&accum1) = 0;
  MP_DIGITS(&accum2) = 0;
  MP_DIGITS(&power2) = 0;
  for (i = 0; i < MAX_ODD_INTS; ++i) {
    MP_DIGITS(oddPowers + i) = 0;
  }

  MP_CHECKOK( mp_init_size(&accum1, 3 * nLen + 2) );
  MP_CHECKOK( mp_init_size(&accum2, 3 * nLen + 2) );

  MP_CHECKOK( mp_init_copy(&oddPowers[0], montBase) );

  mp_init_size(&power2, nLen + 2 * MP_USED(montBase) + 2);
  MP_CHECKOK( mp_sqr(montBase, &power2) );	/* power2 = montBase ** 2 */
  MP_CHECKOK( s_mp_redc(&power2, mmm) );

  for (i = 1; i < odd_ints; ++i) {
    mp_init_size(oddPowers + i, nLen + 2 * MP_USED(&power2) + 2);
    MP_CHECKOK( mp_mul(oddPowers + (i - 1), &power2, oddPowers + i) );
    MP_CHECKOK( s_mp_redc(oddPowers + i, mmm) );
  }

  /* set accumulator to montgomery residue of 1 */
  mp_set(&accum1, 1);
  MP_CHECKOK( s_mp_to_mont(&accum1, mmm, &accum1) );
  pa1 = &accum1;
  pa2 = &accum2;

  for (expOff = bits_in_exponent - window_bits; expOff >= 0; expOff -= window_bits) {
    mp_size smallExp;
    MP_CHECKOK( mpl_get_bits(exponent, expOff, window_bits) );
    smallExp = (mp_size)res;

    if (window_bits == 1) {
      if (!smallExp) {
	SQR(pa1,pa2); SWAPPA;
      } else if (smallExp & 1) {
	SQR(pa1,pa2); MUL(0,pa2,pa1);
      } else {
	ABORT;
      }
    } else if (window_bits == 4) {
      if (!smallExp) {
	SQR(pa1,pa2); SQR(pa2,pa1); SQR(pa1,pa2); SQR(pa2,pa1);
      } else if (smallExp & 1) {
	SQR(pa1,pa2); SQR(pa2,pa1); SQR(pa1,pa2); SQR(pa2,pa1); 
	MUL(smallExp/2, pa1,pa2); SWAPPA;
      } else if (smallExp & 2) {
	SQR(pa1,pa2); SQR(pa2,pa1); SQR(pa1,pa2); 
	MUL(smallExp/4,pa2,pa1); SQR(pa1,pa2); SWAPPA;
      } else if (smallExp & 4) {
	SQR(pa1,pa2); SQR(pa2,pa1); MUL(smallExp/8,pa1,pa2); 
	SQR(pa2,pa1); SQR(pa1,pa2); SWAPPA;
      } else if (smallExp & 8) {
	SQR(pa1,pa2); MUL(smallExp/16,pa2,pa1); SQR(pa1,pa2); 
	SQR(pa2,pa1); SQR(pa1,pa2); SWAPPA;
      } else {
	ABORT;
      }
    } else if (window_bits == 5) {
      if (!smallExp) {
	SQR(pa1,pa2); SQR(pa2,pa1); SQR(pa1,pa2); SQR(pa2,pa1); 
	SQR(pa1,pa2); SWAPPA;
      } else if (smallExp & 1) {
	SQR(pa1,pa2); SQR(pa2,pa1); SQR(pa1,pa2); SQR(pa2,pa1); 
	SQR(pa1,pa2); MUL(smallExp/2,pa2,pa1);
      } else if (smallExp & 2) {
	SQR(pa1,pa2); SQR(pa2,pa1); SQR(pa1,pa2); SQR(pa2,pa1); 
	MUL(smallExp/4,pa1,pa2); SQR(pa2,pa1);
      } else if (smallExp & 4) {
	SQR(pa1,pa2); SQR(pa2,pa1); SQR(pa1,pa2); 
	MUL(smallExp/8,pa2,pa1); SQR(pa1,pa2); SQR(pa2,pa1);
      } else if (smallExp & 8) {
	SQR(pa1,pa2); SQR(pa2,pa1); MUL(smallExp/16,pa1,pa2); 
	SQR(pa2,pa1); SQR(pa1,pa2); SQR(pa2,pa1);
      } else if (smallExp & 0x10) {
	SQR(pa1,pa2); MUL(smallExp/32,pa2,pa1); SQR(pa1,pa2); 
	SQR(pa2,pa1); SQR(pa1,pa2); SQR(pa2,pa1);
      } else {
	ABORT;
      }
    } else if (window_bits == 6) {
      if (!smallExp) {
	SQR(pa1,pa2); SQR(pa2,pa1); SQR(pa1,pa2); SQR(pa2,pa1); 
	SQR(pa1,pa2); SQR(pa2,pa1);
      } else if (smallExp & 1) {
	SQR(pa1,pa2); SQR(pa2,pa1); SQR(pa1,pa2); SQR(pa2,pa1); 
	SQR(pa1,pa2); SQR(pa2,pa1); MUL(smallExp/2,pa1,pa2); SWAPPA;
      } else if (smallExp & 2) {
	SQR(pa1,pa2); SQR(pa2,pa1); SQR(pa1,pa2); SQR(pa2,pa1); 
	SQR(pa1,pa2); MUL(smallExp/4,pa2,pa1); SQR(pa1,pa2); SWAPPA;
      } else if (smallExp & 4) {
	SQR(pa1,pa2); SQR(pa2,pa1); SQR(pa1,pa2); SQR(pa2,pa1); 
	MUL(smallExp/8,pa1,pa2); SQR(pa2,pa1); SQR(pa1,pa2); SWAPPA;
      } else if (smallExp & 8) {
	SQR(pa1,pa2); SQR(pa2,pa1); SQR(pa1,pa2); 
	MUL(smallExp/16,pa2,pa1); SQR(pa1,pa2); SQR(pa2,pa1); 
	SQR(pa1,pa2); SWAPPA;
      } else if (smallExp & 0x10) {
	SQR(pa1,pa2); SQR(pa2,pa1); MUL(smallExp/32,pa1,pa2); 
	SQR(pa2,pa1); SQR(pa1,pa2); SQR(pa2,pa1); SQR(pa1,pa2); SWAPPA;
      } else if (smallExp & 0x20) {
	SQR(pa1,pa2); MUL(smallExp/64,pa2,pa1); SQR(pa1,pa2); 
	SQR(pa2,pa1); SQR(pa1,pa2); SQR(pa2,pa1); SQR(pa1,pa2); SWAPPA;
      } else {
	ABORT;
      }
    } else {
      ABORT;
    }
  }

  res = s_mp_redc(pa1, mmm);
  mp_exch(pa1, result);

CLEANUP:
  mp_clear(&accum1);
  mp_clear(&accum2);
  mp_clear(&power2);
  for (i = 0; i < odd_ints; ++i) {
    mp_clear(oddPowers + i);
  }
  return res;
}
#undef SQR
#undef MUL

#ifdef MP_USING_CACHE_SAFE_MOD_EXP
unsigned int mp_using_cache_safe_exp = 1;
#endif

mp_err mp_set_safe_modexp(int value) 
{
#ifdef MP_USING_CACHE_SAFE_MOD_EXP
 mp_using_cache_safe_exp = value;
 return MP_OKAY;
#else
 if (value == 0) {
   return MP_OKAY;
 }
 return MP_BADARG;
#endif
}

#ifdef MP_USING_CACHE_SAFE_MOD_EXP
#define WEAVE_WORD_SIZE 4

#ifndef MP_CHAR_STORE_SLOW
/*
 * mpi_to_weave takes MPI data and stores in into a byte array interleaved.
 *
 * The purpose of this interleaving is to hide our access to the array of
 * modulus powers from and attacker snooping on cache hits and misses. Because
 * the array is interleaved, each reference will cause exactly the same cache
 * lines to reload.
 *
 * There are 2 different implementations in this file, one which works with just
 * byte loads and stores, the second which works with mp_weave_word loads and
 * stores. These 2 implementations have DIFFERENT results in exactly which byte
 * of an mp_digit winds up in which location in the byte array. That is why
 * there are 2 sets of explanations for how the array is set up.
 *
 *
 *  a is an array of WEAVE_WORD_SIZE mp_ints (that is 4). 
 * It is a partial window into a logical array mp_int p[count] containing
 * the base to the 0 through count-1 powers. Ideally this code would be 
 * simpler if we stored one element of that array at a time, but on some 
 * platforms the cost of storing a byte incurs a full read modify write cycle
 * and increases the memory bandwidth cost by a factor of 4 or 8. By collecting
 * for mp_ints together, we can arrange to store all 4 values in a single
 * word write. 
 *
 *  b is the targeted weaved location. b[0] points to the first byte where
 * first byte of the a array needs to be stored. Each b is an offset into the 
 * weave array.
 * 
 *  count is 2^window size. 
 *
 *  b_size is the size in mp_digits of each mp_int in the array. mp_ints
 * with less than b_size elements are logically padded with zeros before
 * storing.
 *
 *
 * Data is stored as follows :
 * The mp_int array is treated as a byte array.
 *
 *
 * we want to eventually store the logical array mp_int p[count] into the
 * weave array as follows:

 * p[count].digit is treated as a byte array (rather than * an mp_digit array),
 * N is count, and n is b_size * *sizeof(mp_digit):
 *
 * p[0].digit[0]   p[1].digit[0]   ...... p[N-2].digit[0]   p[N-1].digit[0]
 * p[0].digit[1]   p[1].digit[1]   ...... p[N-2].digit[1]   p[N-1].digit[1]
 *            .                                         .
 *            .                                         .
 * p[0].digit[n-2] p[1].digit[n-2] ...... p[N-2].digit[n-2] p[N-1].digit[n-2]
 * p[0].digit[n-1] p[1].digit[n-1] ...... p[N-2].digit[n-1] p[N-1].digit[n-1] 
 *
 * This function stores that a window of p in each call.
 */
mp_err mpi_to_weave(const mp_int *a, unsigned char *b, 
			             mp_size b_size,  mp_size count)
{
  mp_size i, j;
  unsigned char *bsave = b;

  for (i=0; i < WEAVE_WORD_SIZE; i++) {
    unsigned char *pb = (unsigned char *)MP_DIGITS(&a[i]);
    mp_size useda = MP_USED(&a[i]);
    mp_size zero =  b_size - useda;
    unsigned char *end = pb+ (useda*sizeof(mp_digit));
    b = bsave+i;


    ARGCHK(MP_SIGN(&a[i]) == MP_ZPOS, MP_BADARG);
    ARGCHK(useda <= b_size, MP_BADARG);

    for (; pb < end; pb++) {
      *b = *pb;
      b += count;
    }
    for (j=0; j < zero; j++) {
      *b = 0;
      b += count;
    }
  }

  return MP_OKAY;
}

/* reverse the operation above for one entry.
 * b points to the offset into the weave array of the power we are
 * calculating */
mp_err weave_to_mpi(mp_int *a, const unsigned char *b, 
					mp_size b_size, mp_size count)
{
  unsigned char  *pb = (unsigned char *)MP_DIGITS(a);
  unsigned char *end = pb+ (b_size*sizeof(mp_digit));

  MP_SIGN(a) = MP_ZPOS;
  MP_USED(a) = b_size;

  for (; pb < end; b+=count, pb++) {
    *pb = *b;
  }
  s_mp_clamp(a);
  return MP_OKAY;
}
#else
/* Need a primitive that we know is 32 bits long... */
/* this is true on all modern processors we know of today*/
typedef unsigned int mp_weave_word;

/*
 * on some platforms character stores into memory is very expensive since they
 * generate a read/modify/write operation on the bus. On those platforms
 * we need to do integer writes to the bus. Because of some unrolled code,
 * in this current code the size of mp_weave_word must be four. The code that
 * makes this assumption explicity is called out. (on some platforms a write
 * of 4 bytes still requires a single read-modify-write operation.
 *
 * This function is takes the identical parameters as the function above, 
 * however it lays out the final array differently. Where the previous function
 * treats the mpi_int as an byte array, this function treats it as an array of
 * mp_digits where each digit is stored in big endian order.
 * 
 * since we need to interleave on a byte by byte basis, we need to collect 
 * several mpi structures together into a single uint32 before we write. We
 * also need to make sure the uint32 is arranged so that the first value of 
 * the first array winds up in b[0]. This means construction of that uint32
 * is endian specific (even though the layout of the mp_digits in the array 
 * is always big endian).
 *
 * The final data is stored as follows :
 *
 * Our same logical array p array, m is sizeof(mp_digit),
 * N is still count and n is now b_size. If we define p[i].digit[j]0 as the 
 * most significant byte of the word p[i].digit[j], p[i].digit[j]1 as 
 * the next most significant byte of p[i].digit[j], ...  and p[i].digit[j]m-1
 * is the least significant byte. 
 * Our array would look like:
 * p[0].digit[0]0     p[1].digit[0]0    ...  p[N-2].digit[0]0    p[N-1].digit[0]0
 * p[0].digit[0]1     p[1].digit[0]1    ...  p[N-2].digit[0]1    p[N-1].digit[0]1
 *                .                                         .
 * p[0].digit[0]m-1   p[1].digit[0]m-1  ...  p[N-2].digit[0]m-1  p[N-1].digit[0]m-1
 * p[0].digit[1]0     p[1].digit[1]0    ...  p[N-2].digit[1]0    p[N-1].digit[1]0
 *                .                                         .
 *                .                                         .
 * p[0].digit[n-1]m-2 p[1].digit[n-1]m-2 ... p[N-2].digit[n-1]m-2 p[N-1].digit[n-1]m-2
 * p[0].digit[n-1]m-1 p[1].digit[n-1]m-1 ... p[N-2].digit[n-1]m-1 p[N-1].digit[n-1]m-1 
 *
 */
mp_err mpi_to_weave(const mp_int *a, unsigned char *b, 
					mp_size b_size, mp_size count)
{
  mp_size i;
  mp_digit *digitsa0;
  mp_digit *digitsa1;
  mp_digit *digitsa2;
  mp_digit *digitsa3;
  mp_size   useda0;
  mp_size   useda1;
  mp_size   useda2;
  mp_size   useda3;
  mp_weave_word *weaved = (mp_weave_word *)b;

  count = count/sizeof(mp_weave_word);

  /* this code pretty much depends on this ! */
#if MP_ARGCHK == 2
  assert(WEAVE_WORD_SIZE == 4); 
  assert(sizeof(mp_weave_word) == 4);
#endif

  digitsa0 = MP_DIGITS(&a[0]);
  digitsa1 = MP_DIGITS(&a[1]);
  digitsa2 = MP_DIGITS(&a[2]);
  digitsa3 = MP_DIGITS(&a[3]);
  useda0 = MP_USED(&a[0]);
  useda1 = MP_USED(&a[1]);
  useda2 = MP_USED(&a[2]);
  useda3 = MP_USED(&a[3]);

  ARGCHK(MP_SIGN(&a[0]) == MP_ZPOS, MP_BADARG);
  ARGCHK(MP_SIGN(&a[1]) == MP_ZPOS, MP_BADARG);
  ARGCHK(MP_SIGN(&a[2]) == MP_ZPOS, MP_BADARG);
  ARGCHK(MP_SIGN(&a[3]) == MP_ZPOS, MP_BADARG);
  ARGCHK(useda0 <= b_size, MP_BADARG);
  ARGCHK(useda1 <= b_size, MP_BADARG);
  ARGCHK(useda2 <= b_size, MP_BADARG);
  ARGCHK(useda3 <= b_size, MP_BADARG);

#define SAFE_FETCH(digit, used, word) ((word) < (used) ? (digit[word]) : 0)

  for (i=0; i < b_size; i++) {
    mp_digit d0 = SAFE_FETCH(digitsa0,useda0,i);
    mp_digit d1 = SAFE_FETCH(digitsa1,useda1,i);
    mp_digit d2 = SAFE_FETCH(digitsa2,useda2,i);
    mp_digit d3 = SAFE_FETCH(digitsa3,useda3,i);
    register mp_weave_word acc;

/*
 * ONE_STEP takes the MSB of each of our current digits and places that
 * byte in the appropriate position for writing to the weaved array.
 *  On little endian:
 *   b3 b2 b1 b0
 *  On big endian:
 *   b0 b1 b2 b3
 *  When the data is written it would always wind up:
 *   b[0] = b0
 *   b[1] = b1
 *   b[2] = b2
 *   b[3] = b3
 *
 * Once we've written the MSB, we shift the whole digit up left one
 * byte, putting the Next Most Significant Byte in the MSB position,
 * so we we repeat the next one step that byte will be written.
 * NOTE: This code assumes sizeof(mp_weave_word) and MP_WEAVE_WORD_SIZE
 * is 4.
 */
#ifdef MP_IS_LITTLE_ENDIAN 
#define MPI_WEAVE_ONE_STEP \
    acc  = (d0 >> (MP_DIGIT_BIT-8))  & 0x000000ff; d0 <<= 8; /*b0*/ \
    acc |= (d1 >> (MP_DIGIT_BIT-16)) & 0x0000ff00; d1 <<= 8; /*b1*/ \
    acc |= (d2 >> (MP_DIGIT_BIT-24)) & 0x00ff0000; d2 <<= 8; /*b2*/ \
    acc |= (d3 >> (MP_DIGIT_BIT-32)) & 0xff000000; d3 <<= 8; /*b3*/ \
    *weaved = acc; weaved += count;
#else 
#define MPI_WEAVE_ONE_STEP \
    acc  = (d0 >> (MP_DIGIT_BIT-32)) & 0xff000000; d0 <<= 8; /*b0*/ \
    acc |= (d1 >> (MP_DIGIT_BIT-24)) & 0x00ff0000; d1 <<= 8; /*b1*/ \
    acc |= (d2 >> (MP_DIGIT_BIT-16)) & 0x0000ff00; d2 <<= 8; /*b2*/ \
    acc |= (d3 >> (MP_DIGIT_BIT-8))  & 0x000000ff; d3 <<= 8; /*b3*/ \
    *weaved = acc; weaved += count;
#endif 
   switch (sizeof(mp_digit)) {
   case 32:
    MPI_WEAVE_ONE_STEP
    MPI_WEAVE_ONE_STEP
    MPI_WEAVE_ONE_STEP
    MPI_WEAVE_ONE_STEP
    MPI_WEAVE_ONE_STEP
    MPI_WEAVE_ONE_STEP
    MPI_WEAVE_ONE_STEP
    MPI_WEAVE_ONE_STEP
    MPI_WEAVE_ONE_STEP
    MPI_WEAVE_ONE_STEP
    MPI_WEAVE_ONE_STEP
    MPI_WEAVE_ONE_STEP
    MPI_WEAVE_ONE_STEP
    MPI_WEAVE_ONE_STEP
    MPI_WEAVE_ONE_STEP
    MPI_WEAVE_ONE_STEP
   case 16:
    MPI_WEAVE_ONE_STEP
    MPI_WEAVE_ONE_STEP
    MPI_WEAVE_ONE_STEP
    MPI_WEAVE_ONE_STEP
    MPI_WEAVE_ONE_STEP
    MPI_WEAVE_ONE_STEP
    MPI_WEAVE_ONE_STEP
    MPI_WEAVE_ONE_STEP
   case 8:
    MPI_WEAVE_ONE_STEP
    MPI_WEAVE_ONE_STEP
    MPI_WEAVE_ONE_STEP
    MPI_WEAVE_ONE_STEP
   case 4:
    MPI_WEAVE_ONE_STEP
    MPI_WEAVE_ONE_STEP
   case 2:
    MPI_WEAVE_ONE_STEP
   case 1:
    MPI_WEAVE_ONE_STEP
    break;
   }
  }

  return MP_OKAY;
}

/* reverse the operation above for one entry.
 * b points to the offset into the weave array of the power we are
 * calculating */
mp_err weave_to_mpi(mp_int *a, const unsigned char *b, 
					mp_size b_size, mp_size count)
{
  mp_digit *pb = MP_DIGITS(a);
  mp_digit *end = &pb[b_size];

  MP_SIGN(a) = MP_ZPOS;
  MP_USED(a) = b_size;

  for (; pb < end; pb++) {
    register mp_digit digit;

    digit = *b << 8; b += count;
#define MPI_UNWEAVE_ONE_STEP  digit |= *b; b += count; digit = digit << 8;
    switch (sizeof(mp_digit)) {
    case 32:
	MPI_UNWEAVE_ONE_STEP 
	MPI_UNWEAVE_ONE_STEP 
	MPI_UNWEAVE_ONE_STEP 
	MPI_UNWEAVE_ONE_STEP 
	MPI_UNWEAVE_ONE_STEP 
	MPI_UNWEAVE_ONE_STEP 
	MPI_UNWEAVE_ONE_STEP 
	MPI_UNWEAVE_ONE_STEP 
	MPI_UNWEAVE_ONE_STEP 
	MPI_UNWEAVE_ONE_STEP 
	MPI_UNWEAVE_ONE_STEP 
	MPI_UNWEAVE_ONE_STEP 
	MPI_UNWEAVE_ONE_STEP 
	MPI_UNWEAVE_ONE_STEP 
	MPI_UNWEAVE_ONE_STEP 
	MPI_UNWEAVE_ONE_STEP 
    case 16:
	MPI_UNWEAVE_ONE_STEP 
	MPI_UNWEAVE_ONE_STEP 
	MPI_UNWEAVE_ONE_STEP 
	MPI_UNWEAVE_ONE_STEP 
	MPI_UNWEAVE_ONE_STEP 
	MPI_UNWEAVE_ONE_STEP 
	MPI_UNWEAVE_ONE_STEP 
	MPI_UNWEAVE_ONE_STEP 
    case 8:
	MPI_UNWEAVE_ONE_STEP 
	MPI_UNWEAVE_ONE_STEP 
	MPI_UNWEAVE_ONE_STEP 
	MPI_UNWEAVE_ONE_STEP 
    case 4:
	MPI_UNWEAVE_ONE_STEP 
	MPI_UNWEAVE_ONE_STEP 
    case 2:
	break;
    }
    digit |= *b; b += count; 

    *pb = digit;
  }
  s_mp_clamp(a);
  return MP_OKAY;
}
#endif


#define SQR(a,b) \
  MP_CHECKOK( mp_sqr(a, b) );\
  MP_CHECKOK( s_mp_redc(b, mmm) )

#if defined(MP_MONT_USE_MP_MUL)
#define MUL_NOWEAVE(x,a,b) \
  MP_CHECKOK( mp_mul(a, x, b) ); \
  MP_CHECKOK( s_mp_redc(b, mmm) ) 
#else
#define MUL_NOWEAVE(x,a,b) \
  MP_CHECKOK( s_mp_mul_mont(a, x, b, mmm) )
#endif

#define MUL(x,a,b) \
  MP_CHECKOK( weave_to_mpi(&tmp, powers + (x), nLen, num_powers) ); \
  MUL_NOWEAVE(&tmp,a,b)

#define SWAPPA ptmp = pa1; pa1 = pa2; pa2 = ptmp
#define MP_ALIGN(x,y) ((((ptrdiff_t)(x))+((y)-1))&(((ptrdiff_t)0)-(y)))

/* Do modular exponentiation using integer multiply code. */
mp_err mp_exptmod_safe_i(const mp_int *   montBase, 
                    const mp_int *   exponent, 
		    const mp_int *   modulus, 
		    mp_int *         result, 
		    mp_mont_modulus *mmm, 
		    int              nLen, 
		    mp_size          bits_in_exponent, 
		    mp_size          window_bits,
		    mp_size          num_powers)
{
  mp_int *pa1, *pa2, *ptmp;
  mp_size i;
  mp_size first_window;
  mp_err  res;
  int     expOff;
  mp_int  accum1, accum2, accum[WEAVE_WORD_SIZE];
  mp_int  tmp;
  unsigned char *powersArray;
  unsigned char *powers;

  MP_DIGITS(&accum1) = 0;
  MP_DIGITS(&accum2) = 0;
  MP_DIGITS(&accum[0]) = 0;
  MP_DIGITS(&accum[1]) = 0;
  MP_DIGITS(&accum[2]) = 0;
  MP_DIGITS(&accum[3]) = 0;
  MP_DIGITS(&tmp) = 0;

  powersArray = (unsigned char *)malloc(num_powers*(nLen*sizeof(mp_digit)+1));
  if (powersArray == NULL) {
    res = MP_MEM;
    goto CLEANUP;
  }

  /* powers[i] = base ** (i); */
  powers = (unsigned char *)MP_ALIGN(powersArray,num_powers);

  /* grab the first window value. This allows us to preload accumulator1
   * and save a conversion, some squares and a multiple*/
  MP_CHECKOK( mpl_get_bits(exponent, 
				bits_in_exponent-window_bits, window_bits) );
  first_window = (mp_size)res;

  MP_CHECKOK( mp_init_size(&accum1, 3 * nLen + 2) );
  MP_CHECKOK( mp_init_size(&accum2, 3 * nLen + 2) );
  MP_CHECKOK( mp_init_size(&tmp, 3 * nLen + 2) );

  /* build the first WEAVE_WORD powers inline */
  /* if WEAVE_WORD_SIZE is not 4, this code will have to change */
  if (num_powers > 2) {
    MP_CHECKOK( mp_init_size(&accum[0], 3 * nLen + 2) );
    MP_CHECKOK( mp_init_size(&accum[1], 3 * nLen + 2) );
    MP_CHECKOK( mp_init_size(&accum[2], 3 * nLen + 2) );
    MP_CHECKOK( mp_init_size(&accum[3], 3 * nLen + 2) );
    mp_set(&accum[0], 1);
    MP_CHECKOK( s_mp_to_mont(&accum[0], mmm, &accum[0]) );
    MP_CHECKOK( mp_copy(montBase, &accum[1]) );
    SQR(montBase, &accum[2]);
    MUL_NOWEAVE(montBase, &accum[2], &accum[3]);
    MP_CHECKOK( mpi_to_weave(accum, powers, nLen, num_powers) );
    if (first_window < 4) {
      MP_CHECKOK( mp_copy(&accum[first_window], &accum1) );
      first_window = num_powers;
    }
  } else {
      if (first_window == 0) {
        mp_set(&accum1, 1);
        MP_CHECKOK( s_mp_to_mont(&accum1, mmm, &accum1) );
      } else {
        /* assert first_window == 1? */
        MP_CHECKOK( mp_copy(montBase, &accum1) );
      }
  }

  /*
   * calculate all the powers in the powers array.
   * this adds 2**(k-1)-2 square operations over just calculating the
   * odd powers where k is the window size in the two other mp_modexpt
   * implementations in this file. We will get some of that
   * back by not needing the first 'k' squares and one multiply for the 
   * first window */ 
  for (i = WEAVE_WORD_SIZE; i < num_powers; i++) {
    int acc_index = i & (WEAVE_WORD_SIZE-1); /* i % WEAVE_WORD_SIZE */
    if ( i & 1 ) {
      MUL_NOWEAVE(montBase, &accum[acc_index-1] , &accum[acc_index]);
      /* we've filled the array do our 'per array' processing */
      if (acc_index == (WEAVE_WORD_SIZE-1)) {
        MP_CHECKOK( mpi_to_weave(accum, powers + i - (WEAVE_WORD_SIZE-1),
							 nLen, num_powers) );

        if (first_window <= i) {
          MP_CHECKOK( mp_copy(&accum[first_window & (WEAVE_WORD_SIZE-1)], 
								&accum1) );
          first_window = num_powers;
        }
      }
    } else {
      /* up to 8 we can find 2^i-1 in the accum array, but at 8 we our source
       * and target are the same so we need to copy.. After that, the
       * value is overwritten, so we need to fetch it from the stored
       * weave array */
      if (i > 2* WEAVE_WORD_SIZE) {
        MP_CHECKOK(weave_to_mpi(&accum2, powers+i/2, nLen, num_powers));
        SQR(&accum2, &accum[acc_index]);
      } else {
	int half_power_index = (i/2) & (WEAVE_WORD_SIZE-1);
	if (half_power_index == acc_index) {
	   /* copy is cheaper than weave_to_mpi */
	   MP_CHECKOK(mp_copy(&accum[half_power_index], &accum2));
	   SQR(&accum2,&accum[acc_index]);
	} else {
	   SQR(&accum[half_power_index],&accum[acc_index]);
	}
      }
    }
  }
  /* if the accum1 isn't set, Then there is something wrong with our logic 
   * above and is an internal programming error. 
   */
#if MP_ARGCHK == 2
  assert(MP_USED(&accum1) != 0);
#endif

  /* set accumulator to montgomery residue of 1 */
  pa1 = &accum1;
  pa2 = &accum2;

  for (expOff = bits_in_exponent - window_bits*2; expOff >= 0; expOff -= window_bits) {
    mp_size smallExp;
    MP_CHECKOK( mpl_get_bits(exponent, expOff, window_bits) );
    smallExp = (mp_size)res;

    /* handle unroll the loops */
    switch (window_bits) {
    case 1:
	if (!smallExp) {
	    SQR(pa1,pa2); SWAPPA;
	} else if (smallExp & 1) {
	    SQR(pa1,pa2); MUL_NOWEAVE(montBase,pa2,pa1);
	} else {
	    ABORT;
	}
	break;
    case 6:
	SQR(pa1,pa2); SQR(pa2,pa1); 
	/* fall through */
    case 4:
	SQR(pa1,pa2); SQR(pa2,pa1); SQR(pa1,pa2); SQR(pa2,pa1);
	MUL(smallExp, pa1,pa2); SWAPPA;
	break;
    case 5:
	SQR(pa1,pa2); SQR(pa2,pa1); SQR(pa1,pa2); SQR(pa2,pa1); 
	SQR(pa1,pa2); MUL(smallExp,pa2,pa1);
	break;
    default:
	ABORT; /* could do a loop? */
    }
  }

  res = s_mp_redc(pa1, mmm);
  mp_exch(pa1, result);

CLEANUP:
  mp_clear(&accum1);
  mp_clear(&accum2);
  mp_clear(&accum[0]);
  mp_clear(&accum[1]);
  mp_clear(&accum[2]);
  mp_clear(&accum[3]);
  mp_clear(&tmp);
  /* PORT_Memset(powers,0,num_powers*nLen*sizeof(mp_digit)); */
  free(powersArray);
  return res;
}
#undef SQR
#undef MUL
#endif

mp_err mp_exptmod(const mp_int *inBase, const mp_int *exponent, 
		  const mp_int *modulus, mp_int *result)
{
  const mp_int *base;
  mp_size bits_in_exponent, i, window_bits, odd_ints;
  mp_err  res;
  int     nLen;
  mp_int  montBase, goodBase;
  mp_mont_modulus mmm;
#ifdef MP_USING_CACHE_SAFE_MOD_EXP
  static unsigned int max_window_bits;
#endif

  /* function for computing n0prime only works if n0 is odd */
  if (!mp_isodd(modulus))
    return s_mp_exptmod(inBase, exponent, modulus, result);

  MP_DIGITS(&montBase) = 0;
  MP_DIGITS(&goodBase) = 0;

  if (mp_cmp(inBase, modulus) < 0) {
    base = inBase;
  } else {
    MP_CHECKOK( mp_init(&goodBase) );
    base = &goodBase;
    MP_CHECKOK( mp_mod(inBase, modulus, &goodBase) );
  }

  nLen  = MP_USED(modulus);
  MP_CHECKOK( mp_init_size(&montBase, 2 * nLen + 2) );

  mmm.N = *modulus;			/* a copy of the mp_int struct */
  i = mpl_significant_bits(modulus);
  i += MP_DIGIT_BIT - 1;
  mmm.b = i - i % MP_DIGIT_BIT;

  /* compute n0', given n0, n0' = -(n0 ** -1) mod MP_RADIX
  **		where n0 = least significant mp_digit of N, the modulus.
  */
  mmm.n0prime = 0 - s_mp_invmod_radix( MP_DIGIT(modulus, 0) );

  MP_CHECKOK( s_mp_to_mont(base, &mmm, &montBase) );

  bits_in_exponent = mpl_significant_bits(exponent);
#ifdef MP_USING_CACHE_SAFE_MOD_EXP
  if (mp_using_cache_safe_exp) {
    if (bits_in_exponent > 780)
	window_bits = 6;
    else if (bits_in_exponent > 256)
	window_bits = 5;
    else if (bits_in_exponent > 20)
	window_bits = 4;
       /* RSA public key exponents are typically under 20 bits (common values 
        * are: 3, 17, 65537) and a 4-bit window is inefficient
        */
    else 
	window_bits = 1;
  } else
#endif
  if (bits_in_exponent > 480)
    window_bits = 6;
  else if (bits_in_exponent > 160)
    window_bits = 5;
  else if (bits_in_exponent > 20)
    window_bits = 4;
  /* RSA public key exponents are typically under 20 bits (common values 
   * are: 3, 17, 65537) and a 4-bit window is inefficient
   */
  else 
    window_bits = 1;

#ifdef MP_USING_CACHE_SAFE_MOD_EXP
  /*
   * clamp the window size based on
   * the cache line size.
   */
  if (!max_window_bits) {
    unsigned long cache_size = s_mpi_getProcessorLineSize();
    /* processor has no cache, use 'fast' code always */
    if (cache_size == 0) {
      mp_using_cache_safe_exp = 0;
    } 
    if ((cache_size == 0) || (cache_size >= 64)) {
      max_window_bits = 6;
    } else if (cache_size >= 32) {
      max_window_bits = 5;
    } else if (cache_size >= 16) {
      max_window_bits = 4;
    } else max_window_bits = 1; /* should this be an assert? */
  }

  /* clamp the window size down before we caclulate bits_in_exponent */
  if (mp_using_cache_safe_exp) {
    if (window_bits > max_window_bits) {
      window_bits = max_window_bits;
    }
  }
#endif

  odd_ints = 1 << (window_bits - 1);
  i = bits_in_exponent % window_bits;
  if (i != 0) {
    bits_in_exponent += window_bits - i;
  } 

#ifdef MP_USING_MONT_MULF
  if (mp_using_mont_mulf) {
    MP_CHECKOK( s_mp_pad(&montBase, nLen) );
    res = mp_exptmod_f(&montBase, exponent, modulus, result, &mmm, nLen, 
		     bits_in_exponent, window_bits, odd_ints);
  } else
#endif
#ifdef MP_USING_CACHE_SAFE_MOD_EXP
  if (mp_using_cache_safe_exp) {
    res = mp_exptmod_safe_i(&montBase, exponent, modulus, result, &mmm, nLen, 
		     bits_in_exponent, window_bits, 1 << window_bits);
  } else
#endif
  res = mp_exptmod_i(&montBase, exponent, modulus, result, &mmm, nLen, 
		     bits_in_exponent, window_bits, odd_ints);

CLEANUP:
  mp_clear(&montBase);
  mp_clear(&goodBase);
  /* Don't mp_clear mmm.N because it is merely a copy of modulus.
  ** Just zap it.
  */
  memset(&mmm, 0, sizeof mmm);
  return res;
}
