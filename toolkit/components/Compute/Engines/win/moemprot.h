/* $Source$*/
/* $Revision$ */

/* Maple V Release 5 OEM Interface
   Function Prototype Activation
   Copyright (c) 1997 by Waterloo Maple, Inc.
   All rights reserved. Unauthorized duplication prohibited. */

#ifndef __MAPLE_OEMPROTO_H__
#define __MAPLE_OEMPROTO_H__

/* Maple API functions and user supplied callbacks use stdcall conventions
   for compatibility with RAD environments like Visual BASIC and Delphi. */
#ifdef _MAPLE_WINDOWS_
#define MAPLE_WINDOWS
#endif

#ifdef MAPLE_WINDOWS
#define M_DECL __stdcall
#define USE_PROTO              /* stdcall requires prototypes */
#else
#define M_DECL
#endif

/* Determine whether or not to use function prototypes. The P() macro is
   used for function declarations. The PP() macro is used for function pointer
   declarations (eg. callbacks). */
#undef P
#undef PP
#if defined __STDC__ || defined __cplusplus || defined USE_PROTO
#define P(x) x
#define PP(x) x
#else
#define P(x) ()
#define PP(x) ()
#endif

#endif /* MAPLE_OEMPROTO_H */
