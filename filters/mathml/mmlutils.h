
#ifndef MMLutils_h
#define MMLutils_h
																																																								
/*
*/

#include "flttypes.h"
#include <stdlib.h>


// IDs for all MathML elements

#define EID_mi				1
#define EID_mn			    2
#define EID_mo		    	3
#define EID_mtext		    4
#define EID_mspace	    	5
#define EID_ms			    6
#define EID_mrow	    	7
#define EID_mfrac		    8
#define EID_msqrt	    	9
#define EID_mroot		    10
#define EID_mstyle	    	11
#define EID_merror		    12
#define EID_mpadded	    	13
#define EID_mphantom	    14
#define EID_mfenced  		15
#define EID_msub	    	16
#define EID_msup		    17
#define EID_msubsup 		18
#define EID_munder	    	19
#define EID_mover		    20
#define EID_munderover	    21
#define EID_mprescripts     22
#define EID_mtd		    	23
#define EID_mtr			    24
#define EID_mtable		    25
#define EID_LAST 		    25

#define ESID_NUMERATOR		100
#define ESID_DENOMINATOR	101
#define ESID_SUBSCRIPT      102
#define ESID_SUPERSCRIPT  	103
#define ESID_SUBBASE		104
#define ESID_SUPERBASE		105
#define ESID_SUBSUPERBASE	106
#define ESID_UNDERSCRIPT	107
#define ESID_UNDERBASE		109
#define ESID_OVERSCRIPT		110
#define ESID_OVERBASE		111
#define ESID_UNDEROVERBASE	112


// IDs for all attributes of all elements

// attributes common to all elements

#define ELEM_ATTR_class				    1
#define ELEM_ATTR_style				    2
#define ELEM_ATTR_id				    3

// attributes of all token elements except <mspace/>

#define ELEM_ATTR_fontsize			    4
#define ELEM_ATTR_fontweight		    5
#define ELEM_ATTR_fontstyle			    6
#define ELEM_ATTR_fontfamily		    7
#define ELEM_ATTR_color				    8

// attributes of mo elements - operators

#define ELEM_ATTR_form				    9
#define ELEM_ATTR_fence				    10
#define ELEM_ATTR_separator			    11
#define ELEM_ATTR_lspace			    12
#define ELEM_ATTR_rspace			    13

#define ELEM_ATTR_stretchy			    14
#define ELEM_ATTR_symmetric			    15
#define ELEM_ATTR_maxsize			    16
#define ELEM_ATTR_minsize			    17

#define ELEM_ATTR_largeop			    18
#define ELEM_ATTR_movablelimits		    19
#define ELEM_ATTR_accent			    20

#define ELEM_ATTR_width 			    21
#define ELEM_ATTR_height			    22
#define ELEM_ATTR_depth 			    23

#define ELEM_ATTR_lquote		    	24
#define ELEM_ATTR_rquote			    25

#define ELEM_ATTR_linethickness		    26

#define ELEM_ATTR_scriptlevel		    27
#define ELEM_ATTR_displaystyle		    28
#define ELEM_ATTR_scriptsizemultiplier	29
#define ELEM_ATTR_scriptminsize  		30
#define ELEM_ATTR_background    		31

#define ELEM_ATTR_open    		        32
#define ELEM_ATTR_close    		        33
#define ELEM_ATTR_separators    		34

#define ELEM_ATTR_subscriptshift   		35

#define ELEM_ATTR_superscriptshift 		36

#define ELEM_ATTR_accentunder    		37

#define ELEM_ATTR_align         		38
#define ELEM_ATTR_rowalign      		39
#define ELEM_ATTR_columnalign    		40
#define ELEM_ATTR_groupalign    		41
#define ELEM_ATTR_alignmentscope   		42
#define ELEM_ATTR_rowspacing    		43
#define ELEM_ATTR_columnspacing    		44
#define ELEM_ATTR_rowlines      		45
#define ELEM_ATTR_columnlines    		46
#define ELEM_ATTR_frame         		47
#define ELEM_ATTR_framespacing    		48
#define ELEM_ATTR_equalrows     		49
#define ELEM_ATTR_equalcolumns          50

#define ELEM_ATTR_rowspan       		51
#define ELEM_ATTR_columnspan    		52

#define ELEM_ATTR_edge          		53

#define ELEM_ATTR_actiontype    		54
#define ELEM_ATTR_selection     		55

#define ELEM_ATTR_linebreak     		56

#define ELEM_ATTR_mathvariant           57

#define ELEM_ATTR_LAST				    57


U8* ElementIDtoName( U16 element_ID );
U16 ElementNametoID( U8* targ_name );

U16 GetAtomicElementContents( U8* zsrc,U8* zdest,U16 lim,
											TCI_BOOL& is_entity );
// End utility functions

#endif

