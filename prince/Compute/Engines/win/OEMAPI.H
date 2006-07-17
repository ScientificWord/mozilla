/* $Source$*/
/* $Revision$ */

/* Maple V Release 4 OEM Interface
   General API Private Header File
   Copyright (c) 1996 by Waterloo Maple, Inc.
   All rights reserved. Unauthorized duplication prohibited. */

#ifndef __MAPLE_OEMAPI_H__
#define __MAPLE_OEMAPI_H__

#include "moemtype.h"
#include "moemapi.h"

#ifdef __cplusplus
extern "C" {
#endif

/* This data structure is a wrapper around the OEM-supplied data argument to
   MapleEvaluate, a pointer to the current call back vector, and the current
   kernel instance ID. An instance of this structure is passed to EvaluateDAG
   so that the stream handler has access to both the OEM-supplied data and the
   call back vector. */

typedef struct {
    void *data;
    MCallBack callBack;
    int mapleID;
} MetaDataVec, *MetaData;

#ifdef __cplusplus
}
#endif

#endif /* MAPLE_OEMAPI_H */
