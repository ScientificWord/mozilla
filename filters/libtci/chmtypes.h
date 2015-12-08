#ifndef CHMTYPES_H
#define CHMTYPES_H

//Everyone needs chamdefs -- TCIENV clauses will silently work without this.
#ifndef CHAMDEFS_H
  #include "chamdefs.h"
#endif

// BOOLEANS
typedef bool  TCI_BOOL;
#ifdef FALSE
  #undef FALSE
#endif
#ifdef TRUE
  #undef TRUE
#endif
#define FALSE  false
#define TRUE   true

// CHARACTERS
#define TCICHAR  char
#define TCIWCHAR short


// NULL
#undef NULL
#define NULL  0


// limits
//#define CHAM_MAXPARASIZE   (30000-1)
#define CHAM_MAXPARASIZE   0x40000
#define CHAM_MAXFILTER        32
#define CHAM_MAXPATH     260

typedef signed char     I8;
typedef signed short    I16;
typedef signed long     I32;
typedef unsigned char   U8;
typedef U8*             U8Ptr;
typedef unsigned short  U16;
typedef unsigned long   U32;
//typedef float           F32;

#define CHAMmax(a,b)  ((a) > (b) ? (a) : (b))
#define CHAMmin(a,b)  ((a) < (b) ? (a) : (b))

#define CHAMMAKELONG(h,l)   ((long)(((unsigned long)((unsigned)(h))) << 16 | ((unsigned)(l))))
#define LO16(l)   ((U16)(l))
#define HI16(l)   ((U16)(((U32)(l) >> 16) & 0xFFFF))
#define LO8(w)    ((U8)(w))
#define HI8(w)    ((U8)(((U16)(w) >> 8) & 0xff))

#endif
