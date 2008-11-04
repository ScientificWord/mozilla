
#include "flttypes.h"
#include "OutWrap.h"


#include "ltex2mml.h"

/* This file implements an OutputWrapper by making calls
  to LaTeX2MMLTree.  The output_ilk arg is not necessary -
  if it is 1, we have confirmation that the caller wants
  MathML generated as output.
*/

#ifdef TESTING
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



OutTreeGenerator::OutTreeGenerator( U16 output_ilk,
                                TreeGenerator* parent,
                                Grammar* dest_xml_grammar,
                                Grammar* src_math_grammar,
  				 	            Grammar* dest_math_grammar,
                                LogFiler* logger,
							    USERPREFS* preferences,
							    const char** context_zstrs,
							    int* context_ints ) {

  pLaTeX2MML  =  NULL;
  output_type =  output_ilk;

  if ( output_ilk==1 ) {    // MathML
    pLaTeX2MML  =  TCI_NEW( LaTeX2MMLTree( parent,
                        src_math_grammar,dest_math_grammar,
                        logger,preferences,context_zstrs ) );
  } else
    TCI_ASSERT(0);

}



OutTreeGenerator::~OutTreeGenerator() {

  if ( output_type==1 ) {    // MathML
    if ( pLaTeX2MML )
      delete pLaTeX2MML;
  } else
    TCI_ASSERT(0);
}



void* OutTreeGenerator::CreateDestTree( TNODE* src_tree,
                                        U8& eqn_option,
                                        ANOMALY_REC* anomalies,
                                        U16& result ) {

  void* rv =  NULL;

  if ( output_type==1 ) {    // MathML
		MATH_CONTEXT mc;
    rv  =  (void*)pLaTeX2MML->NBLaTeXTreeToMML( src_tree,
                                    mc,anomalies,result );
  } else
    TCI_ASSERT(0);

  return rv;
}


void OutTreeGenerator::AddNumbering( TNODE* mrow,TNODE* xml,
		  					                TCI_BOOL is_eqnarray ) {

  if ( output_type==1 ) {    // MathML
  	pLaTeX2MML->AddNumbering( mrow,xml,is_eqnarray );
  } else
    TCI_ASSERT(0);
}



