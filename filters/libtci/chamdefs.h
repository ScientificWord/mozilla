#ifndef CHAMDEFS_H
#define CHAMDEFS_H

//***** Global flags which determine the OS, Windowing, and compiler
// These are defined:
//   CHAMPRODUCT --- Target Product (CHAMSWPPRO, CHAMSWPSE, CHAMSCIWORD)
//   CHAMWS      --- Windowing system (CHAMPM, CHAMWIN ...)
//   CHAMCC      --- Compiler (CHAMGLOCK   , CHAMZORTECH ...)
//   CHAMWP      --- Windowing package (CHAMXVT, CHAMWINNATIVE, etc.)
//   CHAMDOCSYS  --- Document system (CHAMDOCOLE, CHAMDOCNONE)
//   CHAMINTERFACE  --- OLE Interface (CHAM_AUTOSERVER,CHAM_OBJECTSERVER,CHAM_CONTAINER) 
// All Chameleon code should test the values of these variables, and
// not variables set by other compilers (such as __GWXX__, etc.)

//Target Product
#define CHAMSWPPRO      1001
#define CHAMSWPSE       1002
#define CHAMSCIWORD     1003
#define CHAMACTIVECALC  1004
#define CHAMMONARCH     1005
#define CHAMVIEWER      1006

//Operating systems
#define CHAMOS2         1012
#define CHAMDOS         1013
#define CHAMNT4         1014
#define CHAMNT5         1015
#define CHAMWIN95       1016
#define CHAMWIN32S      1017
#define CHAMWIN98       1018


//Base windowing systems
#define CHAMWIN         1101
#define CHAMPM          1102
#define CHAMWIN32       1103

//Windowing packages
#define CHAMWINNATIVE   1300
#define CHAMWINMFC      1301
#define CHAMPMNATIVE    1302
#define CHAMNOWIN       1303
#define ENV_CHAMNOWIN       CHAMNOWIN

//Document systems
#define CHAMDOCOLE      1700
#define CHAMDOCNONE     1701

#define CHAM_AUTOSERVER             1
#define CHAM_OBJECTSERVER           2
#define CHAM_CONTAINER              4

#define ENV_CHAM_AUTOSERVER             1401
#define ENV_CHAM_OBJECTSERVER           1402
#define ENV_CHAM_CONTAINER              1404

#define CHAM_NOCOMPUTE       0
#define CHAM_MAPLE           1
#define CHAM_MATHEMATICA     2
#define CHAM_MUPAD           4
#define CHAM_DERIVE          8
#define CHAM_REDUCE         16

// The COMPUTEPACKAGE macros can't be changed (e.g. CHAM_MAPLE must = 1)
// Therefore we use new names
#define NOCOMPUTE   2000
#define MAPLE       2001
#define MATHEMATICA 2002
#define MUPAD       2004
#define COMPUTE     2007

// Graphics filters constants:
#define GRFX_FILTERS_ACCUSOFT                          1
#define GRFX_FILTERS_INSO                              2
#define GRFX_FILTERS_LEADTOOLS_RASTER                  4
#define GRFX_FILTERS_LEADTOOLS_VECTOR_IMPORT           8
#define GRFX_FILTERS_LEADTOOLS_VECTOR_EXPORT        0x10 
#define GRFX_FILTERS_PDFXCHANGE                     0x20
#define GRFX_FILTERS_ACCUGIF                        0x40
// The ACCUGIF flag is a stopgap measure only to be used in the 5.0 Beta. We have to use
//   AccuSoft for GIFs for the time being until we get an independent Unisys (LZW-comnpression) agreement.

#define ENV_GRFX_FILTERS_ACCUSOFT                   (3000 + GRFX_FILTERS_ACCUSOFT)
#define ENV_GRFX_FILTERS_INSO                       (3000 + GRFX_FILTERS_INSO)
#define ENV_GRFX_FILTERS_LEADTOOLS_RASTER           (3000 + GRFX_FILTERS_LEADTOOLS_RASTER)
#define ENV_GRFX_FILTERS_LEADTOOLS_VECTOR_IMPORT    (3000 + GRFX_FILTERS_LEADTOOLS_VECTOR_IMPORT)
#define ENV_GRFX_FILTERS_LEADTOOLS_VECTOR_EXPORT    (3000 + GRFX_FILTERS_LEADTOOLS_VECTOR_EXPORT)
#define ENV_GRFX_FILTERS_PDFXCHANGE                 (3000 + GRFX_FILTERS_PDFXCHANGE)
#define ENV_GRFX_FILTERS_ACCUGIF                    (3000 + GRFX_FILTERS_ACCUGIF)

// Conditional compile scheme:

// The TCIENV macro produces a compile-time error if the 
// argument "a" is not defined. Other wise it expands the "helper
// macro _a_.

// The preprocessor for rc files is different and does not use
// short-circuit evaluation. Therefore this scheme does not work
// in rc files.  The TCIENV macro may be used in an rc file if the
// rc file defines TCI_RESOURCE_FILE. In that case no compile-time
// error is generated but we at least have a uniform way of writing 
// conditional compiles. -jon

#define TCIENV(a)   ( (a > 1000 || ENV_##a > 1000 || ENV_ERROR ) && _ENV_##a##_ )

#ifdef TCI_RESOURCE_FILE
  #define ENV_ERROR 0     // do not cause error
#else 
  #define ENV_ERROR 1/0   // cause compiler-time error
#endif

// Helpers for target products
#define _ENV_CHAMSWPSE_  (CHAMPRODUCT==CHAMSWPSE)
#define _ENV_CHAMSWPPRO_  (CHAMPRODUCT==CHAMSWPPRO)
#define _ENV_CHAMVIEWER_  (CHAMPRODUCT==CHAMVIEWER)
#define _ENV_CHAMSCIWORD_  (CHAMPRODUCT==CHAMSCIWORD)
#define _ENV_CHAMMONARCH_  (CHAMPRODUCT==CHAMMONARCH)
#define _ENV_CHAMACTIVECALC_ (CHAMPRODUCT==CHAMACTIVECALC)

// Helpers for windowing system
#define _ENV_CHAMWIN_      (CHAMWS==CHAMWIN)
#define _ENV_CHAMWIN32_    (CHAMWS==CHAMWIN32)

// Helpers for windowing package
#define _ENV_CHAMNOWIN_    (CHAMWP==CHAMNOWIN)
#define _ENV_CHAMWINMFC_   (CHAMWP==CHAMWINMFC)

// Helpers for docsys
#define _ENV_CHAMDOCOLE_         (CHAMDOCSYS==CHAMDOCOLE)
#define _ENV_CHAM_AUTOSERVER_    (CHAMINTERFACE & CHAM_AUTOSERVER)
#define _ENV_CHAM_OBJECTSERVER_  (CHAMINTERFACE & CHAM_OBJECTSERVER)
#define _ENV_CHAM_CONTAINER_     (CHAMINTERFACE & CHAM_CONTAINER)



#define _ENV_COMPUTE_      (COMPUTEPACKAGE != CHAM_NOCOMPUTE)
#define _ENV_MAPLE_        (COMPUTEPACKAGE & CHAM_MAPLE)
#define _ENV_MATHEMATICA_  (COMPUTEPACKAGE & CHAM_MATHEMATICA)
#define _ENV_MUPAD_        (COMPUTEPACKAGE & CHAM_MUPAD)
#define _ENV_NOCOMPUTE_    (COMPUTEPACKAGE == CHAM_NOCOMPUTE)

// Helpers for graphics filters
#define _ENV_GRFX_FILTERS_ACCUSOFT_                   (GRFX_FILTERS & GRFX_FILTERS_ACCUSOFT)
#define _ENV_GRFX_FILTERS_INSO_                       (GRFX_FILTERS & GRFX_FILTERS_INSO)
#define _ENV_GRFX_FILTERS_LEADTOOLS_RASTER_           (GRFX_FILTERS & GRFX_FILTERS_LEADTOOLS_RASTER)
#define _ENV_GRFX_FILTERS_LEADTOOLS_VECTOR_IMPORT_    (GRFX_FILTERS & GRFX_FILTERS_LEADTOOLS_VECTOR_IMPORT)
#define _ENV_GRFX_FILTERS_LEADTOOLS_VECTOR_EXPORT_    (GRFX_FILTERS & GRFX_FILTERS_LEADTOOLS_VECTOR_EXPORT)
#define _ENV_GRFX_FILTERS_PDFXCHANGE_                 (GRFX_FILTERS & GRFX_FILTERS_PDFXCHANGE)
#define _ENV_GRFX_FILTERS_ACCUGIF_                    (GRFX_FILTERS & GRFX_FILTERS_ACCUGIF)


//Now set the variables which can be tested

//The following should be defined for systems where sizeof(int)>=4
#define BIG_INTEGERS


// Assume _MSC_VER is defined.


#define CHAMWS CHAMWIN32
#define CHAMWP CHAMWINMFC


#ifdef NO_OLE
	#define CHAMDOCSYS     CHAMDOCNONE
#else
  #define CHAMDOCSYS     CHAMDOCOLE
  #define CHAMINTERFACE  CHAM_AUTOSERVER
//  #define CHAMINTERFACE  (CHAM_AUTOSERVER | CHAM_CONTAINER)
#endif



#ifdef NO_COMPUTE
  #define COMPUTEPACKAGE  CHAM_NOCOMPUTE
#else
  #define COMPUTEPACKAGE  (CHAM_MAPLE | CHAM_MUPAD)
#endif

//#ifndef GRFX_FILTERS
  #define GRFX_FILTERS GRFX_FILTERS_STR
//#endif  

#endif
