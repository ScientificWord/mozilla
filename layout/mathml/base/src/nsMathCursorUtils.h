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

PRBool PlaceCursorAfter( nsIFrame * pFrame, PRBool fInside, nsIFrame** aOutFrame, PRInt32* aOutOffset, PRInt32& count);

PRBool PlaceCursorBefore( nsIFrame * pFrame, PRBool fInside, nsIFrame** aOutFrame, PRInt32* aOutOffset, PRInt32& count);

nsIFrame * GetFirstTextFrame( nsIFrame * pFrame );

nsIFrame * GetFirstTextFramePastFrame( nsIFrame * pFrame );

nsIFrame * GetLastTextFrame( nsIFrame * pFrame );

nsIFrame * GetLastTextFrameBeforeFrame( nsIFrame * pFrame );
nsIFrame * GetLastChild(nsIFrame * pFrame);

/**
 * GetSignificantParent Sometimes a frame with have a parent frame with the same content pointer. In some cases we want
 *   to go up the parent tree until the content pointer is different. That is what this function does.
 * @param  pFrame A child frame
 * @return        The first ancestor with a different content pointer.
 *
 */
nsIFrame * GetSignificantParent( nsIFrame * pFrame);

nsIFrame * GetNextSignificantSibling( nsIFrame * pFrame);

nsIFrame * GetTopFrameForContent(nsIFrame * pFrame);

nsIFrame * GetPrevSib(nsIFrame * pFrame);

nsIFrame * GetLastChild(nsIFrame * pFrame);


// DOM tree navigation routines that pass over ignorable white space.
// See the "Whitespace in the DOM" article on the MDC

PRBool IsAllWS( nsIDOMNode * node);

PRBool IsIgnorable( nsIDOMNode * node);

nsIDOMNode * NodeBefore( nsIDOMNode * node);

nsIDOMNode * NodeAfter( nsIDOMNode * node);

nsIDOMNode * FirstChild( nsIDOMNode * parent);

nsIDOMNode * LastChild( nsIDOMNode * parent);

PRUint32 mmlFrameGetIndexInParent( nsIFrame * pF, nsIFrame * pParent);

PRUint32 GetFrameChildCount(nsIFrame * pParent);

#endif /* nsMathCursorUtils_h___ */
