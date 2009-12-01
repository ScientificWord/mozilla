/* $Source$*/
/* $Revision$ */

/* Maple V Release 4 OEM Interface
   Sample Plot Device Driver - Character Plots
   Plotting API
   Copyright (c) 1996 by Waterloo Maple, Inc.
   All rights reserved. Unauthorized duplication prohibited. */

#ifndef __MAPLE_MOEMCHAR_H__
#define __MAPLE_MOEMCHAR_H__

#include "oemproto.h"
#include "moemtype.h"
#include "moemplot.h"

extern MPlotDeviceVec charPlotDeviceVec;

extern void SampleCharDriverPlotCallBack P(( void *data, MPlot plot ));

#ifdef __cplusplus
}
#endif

#endif /* MAPLE_MOEMCHAR_H */
