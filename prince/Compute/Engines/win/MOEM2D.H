/* $Source$*/
/* $Revision$ */

/* Maple V Release 5 OEM Interface
   Two-Dimensional Math Notation API
   Copyright (c) 1997 by Waterloo Maple, Inc.
   All rights reserved. Unauthorized duplication prohibited. */

#ifndef __MAPLE_MOEM2D_H__
#define __MAPLE_MOEM2D_H__

#include "moemprot.h"
#include "moemtype.h"

#ifdef MAPLE_WINDOWS
#include <windows.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct RealMathOptionsVec *RealMathOptions;
typedef struct RealMathOptionsVec {
    char *fontName;
    int fontSize;
    int numFontSizes;
    int deviceWidth;
    int deviceHeight;
    int tabWidth;
    int firstIndent;
    int subsequentIndent;
    int lineSpace;
    M_BOOL labelling;
    int minLabelWidth;
    int verboseProc;
    int showAssumed;
} RealMathOptionsVec;

extern RealMathOptions M_DECL DefaultRealMathOptions P(( int mapleID ));

extern char * M_DECL RealMathText P(( M_EXPR expr, RealMathOptions fmtOpts,
				      M_BOOL (M_DECL *writeChar) PP(( void *data, int c )),
				      M_BOOL (M_DECL *writeAttrib) PP(( void *data, int a )),
				      void *data ));

/* Possible values of the attribute passed to the writeAttrib call back. */
#ifndef FN_NORM
#define FN_NORM 0
#define FN_ITAL 1
#define FN_BOLD 3
#endif

extern char * M_DECL RealMathLaTeX P(( M_EXPR expr, RealMathOptions fmtOpts,
				      M_BOOL (M_DECL *writeChar) PP(( void *data, int c )),
				      M_BOOL in_line,
				      void *data ));

#ifdef MAPLE_WINDOWS

typedef struct {
    int metaFileType;
    int currentNumber, maximumNumber;
    int resolution;
    M_INT width, height;
    union {
	char *fileName;
	HMETAFILE standard;
	HENHMETAFILE enhanced;
    } metaFile;
} RealMathMFVec, *RealMathMF;

/* Possible values of the metaFileType field in a RealMathMFVec, and the
   metaFileType argument to the RealMathMetaFile function. */
#define MF_STANDARD 0                   /* Standard 16-bit. */
#define MF_ALDUS 1                      /* Aldus placeable 16-bit enhanced. */
#define MF_ENHANCED 2                   /* MS enhanced 32-bit (Win32). */

extern char * M_DECL RealMathMetaFile P(( M_EXPR expr, RealMathOptions fmtopts,
					  M_BOOL (M_DECL *writemetafile) PP(( void *data, RealMathMF mf )),
					  int metafiletype, char *filename,
					  HDC devicecontext, M_BOOL multipage,
					  void *data ));

#endif /* MAPLE_WINDOWS */

#ifdef MAPLE_MACINTOSH

extern HANDLE M_ DECL RealMathPict P(( M_EXPR expr, RealMathOptions fmtOpts ));

#endif

#ifdef __cplusplus
}
#endif

#endif /* MAPLE_MOEM2D_H */
