
#ifdef TESTING
  #undef THIS_FILE
  static char THIS_FILE[] = __FILE__;
#endif

#include "strcharr.h"
#include "TCI_new.h"
#include <string.h>

//#define STRETCHY_ARRAY_TEST


#define ARRAYSLOP   5

StretchyArray::StretchyArray() {

  highmark  =  0;
  pointers  =  0;
}


StretchyArray::StretchyArray( const StretchyArray& sa ) {

  highmark  =  sa.highmark;
  pointers  =  0;

  if ( highmark ) {
    void  **newptr  =  TCI_NEW( void*[highmark] );

    TCI_ASSERT( newptr );

    if ( !newptr ) {
      highmark  =  0;
    } else {
      memcpy( newptr, sa.pointers, sizeof(void*)*highmark );   // copy the old data
      pointers  =  newptr;
    }
  }
}


StretchyArray::~StretchyArray() {

  highmark  =  0;
  if ( pointers ) {
    delete pointers;
    pointers  =  0;
  }
}


voidPtr& StretchyArray::operator[] ( U32 ix ) {

  return Index( ix );
}


voidPtr StretchyArray::Get( U32 ix ) const {

  TCI_ASSERT( ix < highmark );
  if ( ix >= highmark )
    return 0;

  return pointers[ix];
}


void StretchyArray::operator=( const StretchyArray& sa ) {

  delete pointers;

  highmark  =  sa.highmark;
  pointers  =  0;

  if ( highmark ) {
    void  **newptr  =  TCI_NEW( void*[highmark] );

    TCI_ASSERT( newptr );

    if ( !newptr ) {
      highmark  =  0;
    } else {
      memcpy( newptr, sa.pointers, sizeof(void*)*highmark );   // copy the old data
      pointers  =  newptr;
    }
  }
}


voidPtr& StretchyArray::Index( U32 ix ) {

  if ( ix >= highmark )
    SetHighMark( ix );

  return pointers[ix];
}


TCI_BOOL StretchyArray::SetHighMark( U32 ix ) {

  // this function is only called if we need to allocate more memory
  U32 oldmark  =  highmark;
  ix  +=  ARRAYSLOP;
  void  **newptr  =  TCI_NEW( void*[ix] );

  TCI_ASSERT( newptr );

  if ( !newptr ) {
    highmark  =  oldmark;
    return FALSE;
  }

  memset( newptr, 0, sizeof(void*)*ix );    // initialize to NULL

  if ( pointers ) {
    memcpy( newptr, pointers, sizeof(void*)*oldmark );   // copy the old data
    delete pointers;
  }

  pointers  =  newptr;
  highmark  =  ix;

  return TRUE;
}


// ---------------------------------------------------------------------------------
#ifdef STRETCHY_ARRAY_TEST


#define MSG_WARNING 0
MSG_RESULT cdecl MessageString( U32, U8* b, ... ) {

  printf( "AIG: %s\n",b );
  return (MSG_RESULT)0;
}

void _Assert(char *strfile, unsigned uline){

  char buff[256];
  sprintf( buff,"Assertion failed: %s, line %u",strfile, uline);

  MessageString( MSG_WARNING,(U8*)buff );
}


//typedef struct _big {
//  int a;
//  int b;
//  int c;
//  int d;
//} big;
//
//
//void dumpbig(big *foo) {
//
//  if(foo){
//    printf("struct: %d %d %d %d\n",foo->a,foo->b,foo->c,foo->d);
//  } else {
//    printf("Struct: null\n");
//  }
//}


void setstuff(StretchyArray sa) {

  sa[4] = "Subroutine sets offset 4";
  sa[12] = "Subroutine sets offset 12";
  printf("Subroutine refs 3, 4, and 12: %s %s %s\n",
    (char *)sa[3], (char *)sa[4], (char *)sa[12]);
}


main() {

  StretchyArray foo;
  char *bar;

  foo[0] = "Value at zero";
  foo[7] = "Value at seven";
  foo[3] = "Value at three";
  bar = (char*) ((foo)[0]);

  printf("%s, %s, %s\n", bar,(char *) (foo[7]),
    (char *) (foo[3]));

  setstuff(foo);

  printf("%s, %s, %s\n", (char *)foo[0],(char *) (foo[7]),
    (char *) (foo[3]));

  printf("%s, %s, %s\n", (char *)foo[3],(char *) (foo[4]),
    (char *) (foo[12]));

  return(0);
}

/*

setstuff(StretchyArray<char *> sa)
{
  sa[4] = "Subroutine sets offset 4";
  sa[12] = "Subroutine sets offset 12";
  printf("Subroutine refs 3, 4, and 12: %s %s %s\n",
    (char *)sa[3], (char *)sa[4], (char *)sa[12]);
}


main()
{
  StretchyArray<char *> foo;
  char *bar;

  foo[0] = "Value at zero";
  foo[7] = "Value at seven";
  foo[3] = "Value at three";
  bar = foo[0];

  printf("%s, %s, %s\n", bar,(char *) (foo[7]),
    (char *) (foo[3]));

  setstuff(foo);

  printf("%s, %s, %s\n", (char *)foo[0],(char *) (foo[7]),
    (char *) (foo[3]));

  printf("%s, %s, %s\n", (char *)foo[3],(char *) (foo[4]),
    (char *) (foo[12]));

  return(0);

}

*/

#endif
