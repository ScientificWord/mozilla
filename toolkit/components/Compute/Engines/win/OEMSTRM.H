/* $Source$*/
/* $Revision$ */

/* Maple V Release 4 OEM Interface
   Stream Handlers
   Copyright (c) 1996 by Waterloo Maple, Inc.
   All rights reserved. Unauthorized duplication prohibited.
   Written Jan-Feb 1996 by Stefan Vorkoetter. */

#ifndef __MAPLE_OEMSTRM_H__
#define __MAPLE_OEMSTRM_H__

#include "proto.h"
#include "mtypes.h"
#include "ifvars.h"

#ifdef __cplusplus
extern "C" {
#endif

extern ifvars_p GetInterfaceVariables P(( int mapleid ));
extern ALGEB KernelResultHandler P(( void *data, ALGEB dag ));
extern void InitializeKernelResultHandler P(( int mapleid ));

#ifdef __cplusplus
}
#endif

#endif /* MAPLE_OEMSTRM_H */
