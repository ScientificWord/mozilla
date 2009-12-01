/* $Source$*/
/* $Revision$ */

/* Maple V Release 5 OEM Interface
   General API
   Copyright (c) 1997 by Waterloo Maple, Inc.
   All rights reserved. Unauthorized duplication prohibited. */

#ifndef __MAPLE_MOEMAPI_H__
#define __MAPLE_MOEMAPI_H__

#include "moemprot.h"
#include "moemtype.h"
#include "moemplot.h"

#ifdef __cplusplus
extern "C" {
#endif

/* The call back vector passed to the Maple function. */
typedef struct {
    void (M_DECL *textCallBack) PP(( void *data, int tag, char *output ));
    void (M_DECL *errorCallBack) PP(( void *data, M_INT offset, char *msg ));
    void (M_DECL *statusCallBack) PP(( void *data, long bytesUsed,
				       long bytesAlloc, double cpuTime ));
    void (M_DECL *plotCallBack) PP(( void *data, MPlot plot ));
    void (M_DECL *realMathCallBack) PP(( void *data, M_EXPR expr ));
    char * (M_DECL *readLineCallBack) PP(( void *data ));
    M_BOOL (M_DECL *redirectCallBack) PP(( void *data, char *name, char *mode ));
    char * (M_DECL *callBackCallBack) PP(( void *data, char *output ));
    M_BOOL (M_DECL *queryInterrupt) PP(( void *data ));
} MCallBackVec, *MCallBack;

/* Possible values for the tag parameter to the textCallBack function. */
#define M_DIAG 1
#define M_MISC 2
#define M_OUTPUT 3
#define M_QUIT 4
#define M_WARNING 5
#define M_ERROR 6
#define M_STATUS 7
#define M_PRETTY 8
#define M_HELP 9

extern int M_DECL MapleInitialize P(( int argc, char **argv, char *error ));

extern void M_DECL MapleEvaluate P(( int mapleID, char *expr, MCallBack cb,
				     void *data ));

extern void M_DECL MapleQuit P(( int mapleID ));

extern M_BOOL M_DECL MapleRestart P(( int mapleID, char *error ));

extern void M_DECL MapleError P(( char *message ));

extern M_EXPR M_DECL MapleParse P(( int mapleID, char *expr,
				    void (M_DECL *errorCallBack) PP(( void *data, M_INT offset, char *msg )),
				    void *data ));

#ifdef __cplusplus
}
#endif

#endif /* MAPLE_MOEMAPI_H */
