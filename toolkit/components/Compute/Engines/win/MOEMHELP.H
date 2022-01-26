/* $Source$*/
/* $Revision$ */

/* Maple V Release 5 OEM Interface
   Help Database API
   Copyright (c) 1997 by Waterloo Maple, Inc.
   All rights reserved. Unauthorized duplication prohibited. */

#ifndef __MAPLE_MOEMHELP_H__
#define __MAPLE_MOEMHELP_H__

#include "moemprot.h"
#include "moemtype.h"

#ifdef __cplusplus
extern "C" {
#endif

extern char * M_DECL HelpSetPath P(( char *path ));
extern char * M_DECL HelpLookUp P(( char *topic, char *section,
				    M_BOOL (M_DECL *writeChar) PP(( void *data, int c )),
				    M_BOOL (M_DECL *writeAttrib) PP(( void *data, int a )),
				    int width, void *data ));

#ifndef FN_NORM
#define FN_NORM 0
#define FN_ITAL 1
#define FN_BOLD 3
#endif
#define FN_UNDER 4

#ifdef __cplusplus
}
#endif

#endif /* MAPLE_MOEMHELP_H */
