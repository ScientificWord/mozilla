/* Copyright 2008, MacKichan Software, Inc. */

#ifndef nsMathCursorUtils_h___
#define nsMathCursorUtils_h___

#include "nsIFrame.h"

/*
  This file contains a few utility routines that are called frequently from the code that controls
  cursor movement in math frames.
*/

/* Place the cursor at the beginning of a DOM node. There are several possible locations for the cursor, marked
   as #1 and ... #n in the following:
  <mfoo>
    #1
    <mbar>
	   	#2some text
	</mbar>		   
  </mfoo>

  If fInside is true, the cursor advances recursively until is precedes a character in a text frame.
  If fInside is false, the cursor goes just before the tag <mbar>. Which we choose depends on the quirks of
  displaying cursor positions in Mozilla.
*/

PRBool PlaceCursorAfter( nsIFrame * pFrame, PRBool fInside, nsPeekOffsetStruct** paPos, PRUint32& count);

PRBool PlaceCursorBefore( nsIFrame * pFrame, PRBool fInside, nsPeekOffsetStruct** paPos, PRUint32& count);

											   
#endif /* nsMathCursorUtils_h___ */
