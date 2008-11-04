#ifndef OUTTREEGEN_H
#define OUTTREEGEN_H

// Output Tree Generator class.  A wrapper class.

// A number of export filters (XML, the math component-MathML
//  of HTML, the math component-MTEF of RTF) all start with
//  a collection of objects that produce a parse tree from
//  our internal format, or external LaTeX.
// This object is one those that are common to all such filters.
// It hides the object that produces a particular output tree.


#include "flttypes.h"
#include "fltutils.h"

class TreeGenerator;
class Grammar;
class LogFiler;
class LaTeX2MTEFTree;
class LaTeX2MMLTree;


class OutTreeGenerator {
public:

  OutTreeGenerator( U16 output_ilk,TreeGenerator* parent,
                    Grammar* dest_xml_grammar,
                    Grammar* src_mtf_grammar,
  				 	Grammar* dest_mtf_grammar,
                    LogFiler* logger,
				    USERPREFS* preferences,
					const char** context_zstrs,
					int* context_ints );

  ~OutTreeGenerator();

  void* CreateDestTree( TNODE* src_tree,
                            U8& eqn_option,
                                ANOMALY_REC* anomalies,
                                    U16& result );
  void  AddNumbering( TNODE* mrow,TNODE* xml,
		  					                TCI_BOOL is_eqnarray );

  void  FixTCIDataInPreamble( TNODE* dst_parse_tree );
	void  RemovePatchComment( TNODE* dst_parse_tree );

private :

  U16 output_type;

  LaTeX2MTEFTree* pLaTeX2MTEF;

  LaTeX2MMLTree* pLaTeX2MML;
};

#endif

