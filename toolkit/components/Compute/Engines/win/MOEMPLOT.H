/* $Source$*/
/* $Revision$ */

/* Maple V Release 5 OEM Interface
   Plotting API
   Copyright (c) 1997 by Waterloo Maple, Inc.
   All rights reserved. Unauthorized duplication prohibited. */

#ifndef __MAPLE_MOEMPLOT_H__
#define __MAPLE_MOEMPLOT_H__

#include "moemprot.h"
#include "moemtype.h"

#ifdef __cplusplus
extern "C" {
#endif

/* A pointer to a plot structure. This structure is abstract, so it is
   declared here as a void pointer. */
typedef void *MPlot;

/* The device vector passed to the various plotting functions. */
typedef struct MPlotDeviceVec *MPlotDevice;
typedef struct MPlotDeviceVec {
    void (M_DECL *drawPoint) PP(( void *data, MPlotDevice dev, MPlot plot,
				  M_INT x, M_INT y, M_INT attrib ));
    void (M_DECL *drawLine) PP(( void *data, MPlotDevice dev, MPlot plot,
				 M_INT x1, M_INT y1, M_INT x2, M_INT y2,
				 M_INT attrib ));
    void (M_DECL *drawPolygon) PP(( void *data, MPlotDevice dev, MPlot plot,
				    M_INT numPoints, M_INT *x, M_INT *y,
				    M_INT attrib ));
    void (M_DECL *drawText) PP(( void *data, MPlotDevice dev,
				 MPlot plot, M_INT x, M_INT y, char *text,
				 M_INT alignment, M_INT attrib, M_INT font ));
    void (M_DECL *drawAxisLine) PP(( void *data, MPlotDevice dev, MPlot plot,
				     M_INT x1, M_INT y1, M_INT x2, M_INT y2,
				     M_INT attrib ));
    void (M_DECL *drawAxisText) PP(( void *data, MPlotDevice dev, MPlot plot,
				     M_INT x, M_INT y, char *text,
				     M_INT alignment, M_INT attrib, M_INT font ));
    void (M_DECL *drawTitleText) PP(( void *data, MPlotDevice dev, MPlot plot,
				      char *text ));
    void (M_DECL *getTextExtents) PP(( void *data, MPlotDevice dev,
				 MPlot plot, M_INT x, M_INT y, char *text,
				 M_INT alignment, M_INT attrib, M_INT font,
				 M_INT *llx, M_INT *lly,
				 M_INT *urx, M_INT *ury ));
    void (M_DECL *startImage) PP(( void *data, MPlotDevice dev, MPlot plot ));
    void (M_DECL *setColorMap) PP(( void *data, MPlotDevice dev, MPlot plot,
				    M_INT numColors, M_INT *rgb ));
    M_BOOL (M_DECL *endImage) PP(( void *data, MPlotDevice dev, MPlot plot ));
    M_INT (M_DECL *queryDevice) PP(( void *data, MPlotDevice dev, MPlot plot,
				     M_INT devQuery ));
    M_BOOL (M_DECL *plotQueryInterrupt) PP(( void *data, MPlotDevice dev,
					 MPlot plot ));
    void (M_DECL *outChar) PP(( void *data, int c ));
} MPlotDeviceVec;

/* This structure is used to return the axes range from PlotGetAxisRanges. */
typedef struct MPlotAxisRanges {
    double Xmin, Xmax,
	   Ymin, Ymax,
	   Zmin, Zmax;
} MPlotAxisRanges;

/* Values for the alignment parameter of drawText and drawAxisText. */
#define ALIGN_TO_LEFT 1
#define ALIGN_TO_RIGHT 2
#define ALIGN_BELOW 4
#define ALIGN_ABOVE 8

/* Possible values for the devQuery parameter of queryDevice. */
#define Q_RESOLUTION 1
#define Q_COLORMAP 2
#define Q_POLYGONS 3
#define Q_BACKGROUND 4
#define Q_WIDTH 5
#define Q_HEIGHT 6

/* Decoding macros for the attrib parameter of the various call backs. */
#define DECODE_COLOR(x) (((M_INT)(x)) & 0xffffff)
#define DECODE_LINESTYLE(x) ((((M_INT)(x))>>25) & 0x07)
#define DECODE_SYMBOL(x) ((((M_INT)(x))>>28) & 0x0f)
#define DECODE_THICKNESS(x) ((((M_INT)(x))>>28) & 0x0f)

/* Attributes to set/query with PlotSet/GetAttribute. */
typedef enum {
    MPA_AXES, MPA_COLOR, MPA_GRID, MPA_LIGHTING, MPA_LINESTYLE,
    MPA_ORIENTATION_PHI, MPA_ORIENTATION_THETA, MPA_PROJECTION,
    MPA_SCALING, MPA_STYLE, MPA_SYMBOL, MPA_THICKNESS
} MPlotAttrib;

/* Attributes to be used with MPA_AXES. */
#define MPA_AXES_DEFAULT 0
#define MPA_AXES_NONE 1
#define MPA_AXES_BOX 2
#define MPA_AXES_FRAME 3
#define MPA_AXES_NORMAL 4

/* Attributes to be used with MPA_COLOR. */
#define MPA_COLOR_DEFAULT 0
#define MPA_COLOR_XYZ 1
#define MPA_COLOR_XY 2
#define MPA_COLOR_Z 3
#define MPA_COLOR_Z_HUE 4
#define MPA_COLOR_Z_GRAY 5
#define MPA_COLOR_NONE 10

/* Attributes to be used with MPA_GRID. */
#define MPA_GRID_DEFAULT 0
#define MPA_GRID_TRIANGULAR 1
#define MPA_GRID_RECTANGULAR 2

/* Attributes to be used with MPA_LIGHTING. */
#define MPA_LIGHTING_DEFAULT 0
#define MPA_LIGHTING_NONE 1
#define MPA_LIGHTING_USER 2
#define MPA_LIGHTING_1 3
#define MPA_LIGHTING_2 4
#define MPA_LIGHTING_3 5
#define MPA_LIGHTING_4 6

/* Attributes to be used with MPA_LINESTYLE. These are also the values
   returned by the DECODE_LINESTYLE macro. User defined line styles may also
   be used; it is up to the rendering functions what to do with these. */
#define MPA_LINESTYLE_SOLID 1
#define MPA_LINESTYLE_DOTTED 2
#define MPA_LINESTYLE_DASHED 3
#define MPA_LINESTYLE_DASH_DOT 4
#define MPA_LINESTYLE_USER_MAX 7

/* Attributes to be used with MPA_PROJECTION. */
#define MPA_PROJECTION_DEFAULT 2
#define MPA_PROJECTION_ORTHOGONAL 3
#define MPA_PROJECTION_FAR_PERSPECTIVE 4
#define MPA_PROJECTION_MEDIUM_PERSPECTIVE 5
#define MPA_PROJECTION_NEAR_PERSPECTIVE 6

/* Attributes to be used with MPA_SCALING. */
#define MPA_SCALING_DEFAULT 0
#define MPA_SCALING_CONSTRAINED 1
#define MPA_SCALING_UNCONSTRAINED 2

/* Attributes to be used with MPA_STYLE. */
#define MPA_STYLE_DEFAULT 0
#define MPA_STYLE_HIDDEN 1
#define MPA_STYLE_PATCH 2
#define MPA_STYLE_PATCH_NO_GRID 3
#define MPA_STYLE_PATCH_CONTOUR 4
#define MPA_STYLE_POINT 5
#define MPA_STYLE_LINE 6
#define MPA_STYLE_CONTOUR 7
#define MPA_STYLE_FLAT_CONTOUR 8

/* Attributes to be used with MPA_SYMBOL. These are also the values returned
   by the DECODE_SYMBOL macro. User defined symbols may also be used; it is up
   to the rendering functions what to do with these. */
#define MPA_SYMBOL_DEFAULT 0
#define MPA_SYMBOL_CROSS 1
#define MPA_SYMBOL_DIAMOND 2
#define MPA_SYMBOL_BOX 3
#define MPA_SYMBOL_CIRCLE 4
#define MPA_SYMBOL_POINT 5
#define MPA_SYMBOL_USER_MAX 15

/* Attributes to be used with MPA_THICKNESS. These are also the values
   returned by the DECODE_THICKNESS macro. User defined thicknesses may also
   be used; it is up to the rendering functions what to do with these. */
#define MPA_THICKNESS_THIN 1
#define MPA_THICKNESS_MEDIUM 2
#define MPA_THICKNESS_THICK 3
#define MPA_THICKNESS_USER_MAX 15

extern char * M_DECL PlotRender P(( MPlot plot, MPlotDevice dev, void *data ));
extern void M_DECL PlotDispose P(( MPlot plot ));

extern void M_DECL PlotSetAttribute P(( MPlot plot, MPlotAttrib attr,
					int value ));
extern int M_DECL PlotGetAttribute P(( MPlot plot, MPlotAttrib attr ));

extern void M_DECL PlotGetAxisRanges P(( MPlot plot, MPlotAxisRanges *ar ));

extern MPlot M_DECL PlotGetBBox3D P(( int mapleID ));

extern M_BOOL M_DECL PlotGetFont P(( MPlot plot, M_INT font,
				     char *name, int nameSize,
				     char *style, int styleSize,
				     M_INT *size ));

extern M_INT M_DECL PlotGetFrameCount P(( MPlot plot ));
extern M_INT M_DECL PlotGetDimensions P(( MPlot plot ));

#ifdef __cplusplus
}
#endif

#endif /* MAPLE_MOEMPLOT_H */
