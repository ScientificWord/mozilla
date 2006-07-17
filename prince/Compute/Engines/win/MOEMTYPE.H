/* $Source$*/
/* $Revision$ */

/* Maple V Release 5 OEM Interface
   Basic Data Types
   Copyright (c) 1997 by Waterloo Maple, Inc.
   All rights reserved. Unauthorized duplication prohibited. */

#ifndef __MAPLE_MOEMTYPE_H__
#define __MAPLE_MOEMTYPE_H__

#ifdef __cplusplus
extern "C" {
#endif

/* M_INT: An integer that is the same size as a pointer. This is generally int,
	  but on a DEC-Alpha, it is a long int. */
#ifndef M_INT
#if defined __alpha
typedef long int M_INT;
#else
typedef int M_INT;
#endif
#endif

/* M_BOOL: An integer that can be either TRUE or FALSE. */
#ifndef M_BOOL
typedef M_INT M_BOOL;
#if !defined TRUE || !defined FALSE
#define FALSE 0
#define TRUE 1
#endif
#endif

/* M_EXPR: A Maple expression in native internal format. */
typedef M_INT ***M_EXPR;

#ifdef __cplusplus
}
#endif

#endif /* MAPLE_MOEMTYPE_H */
