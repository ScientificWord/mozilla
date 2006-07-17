/* $Source$*/
/* $Revision$ */

/* Maple V Release 4 OEM Interface
   Plotting API - Private Header File
   Copyright (c) 1996 by Waterloo Maple, Inc.
   All rights reserved. Unauthorized duplication prohibited. */

#ifndef __MAPLE_OEMPLOT_H__
#define __MAPLE_OEMPLOT_H__

#include "mtypes.h"

#include "blkalloc.h"
#include "plot.h"

#include "oemproto.h"
#include "moemtype.h"
#include "moemapi.h"

#ifdef __cplusplus
extern "C" {
#endif

/* This is really the MPlot structure that OEM applications pass around, but
   we can't call it that, since MPlot is already declared as a void *, since
   the OEM should not know what's in it. */
typedef struct {
    Plot *plot;                  /* Maple's plot data structure. */
    MemArea *memArea;            /* Place to build and use plot structure. */
} OEMPlotVec, *OEMPlot;

extern void PlotAndPlot3DHandler P(( void *data, ALGEB args, int dim,
				     MCallBack cb ));

#ifdef __cplusplus
}
#endif

#endif /* MAPLE_OEMPLOT_H */
