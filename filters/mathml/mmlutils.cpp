
#include "MMLutils.h"
#include "tci_new.h"
#include <string.h>
#include <stdlib.h>

#ifdef TESTING
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern void   JBMLine( char* );

// WARNING - the following array MUST be kept in ono-to-one
//  correspondence with ELEM_ATTR_ defines in "attrman.h"
//  ie.		#define ELEM_ATTR_class 	1
//  Each attribute has 3 associated strings, name, type, and default.

char* elem_names[]  =  {
  "dummy",
  "mi",
  "mn",
  "mo",
  "mtext",
  "mspace",
  "ms",
  "mrow",
  "mfrac",
  "msqrt",
  "mroot",
  "mstyle",
  "merror",
  "mpadded",
  "mphantom",
  "mfenced",
  "msub",
  "msup",
  "msubsup",
  "munder",
  "mover",
  "munderover",
  "mprescripts",
  "mtd",
  "mtr",
  "mtable",
  ""
};



// Some utility functions to convert between names and IDs

U8* ElementIDtoName( U16 element_ID ) {

  U8* rv  =  NULL;
  if ( element_ID > 0 && element_ID <= EID_LAST ) {
    rv  =  (U8*)elem_names[element_ID];
  } else
    TCI_ASSERT(0);

  return rv;
}


U16 ElementNametoID( U8* targ_name ) {

  U16 rv  =  0;

  U16 i =  1;
  while ( i <= EID_LAST ) {
    if ( !strcmp((char*)targ_name,elem_names[i]) ) {
      rv  =  i;
	  break;
	} else
	  i++;
  }

  return rv;
}

U16 GetAtomicElementContents( U8* zsrc,U8* zdest,U16 lim,
											TCI_BOOL& is_entity ) {
//JBMLine( "MMLUtils::GetElementContents\n" );

  is_entity   =  FALSE;
  if ( zsrc && *zsrc=='&' ) {		// &entityname;
    is_entity =  TRUE;
	zsrc++;
  }

  U16 ni  =  0;
  if ( zsrc ) {
    while ( *zsrc && ni<lim-1 ) {
	  U8 chr  =  *zsrc;
	  if ( is_entity && chr==';' )	// named entity ender
	    break;
	  zdest[ni++]  =  chr;
	  zsrc++;
    }
  }
  zdest[ni] =  0;					// null terminator

  return ni;
}


// End utility functions


