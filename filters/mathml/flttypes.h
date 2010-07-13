
#ifndef FLTTYPES_H
#define FLTTYPES_H

#include <assert.h>
// debugging help, should switch each to NS_ASSERTION
//#define TCI_ASSERT(expr)  assert(expr)

#define TCI_ASSERT(expr)


typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned long U32;

typedef short I16;
typedef long  I32;

typedef bool TCI_BOOL;

#define FALSE false
#define TRUE true

#define TCI_NEW(T) new T



#endif