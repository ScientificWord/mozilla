
#include "AnalyzerData.h"

#include "mnode.h"
#include "Grammar.h"
#include "DefInfo.h"
#include "DefStore.h"
#include "strutils.h"
#include <cstring>

char* GetCanonicalIDforMathNode(const MNODE* mml_node, const Grammar* mml_entities);


// Convenience method for Tree2StdMML to lookup a function.
bool AnalyzerData::IsDefinedFunction(MNODE* mnode)
{
  char* mi_canonical_str = GetCanonicalIDforMathNode(mnode, GetGrammar() );

  DefInfo* di = GetDI(mi_canonical_str);
  delete[] mi_canonical_str;
    
  return (di != NULL && (di->def_type == DT_FUNCTION || di->def_type == DT_MUPNAME));

}


DefInfo* AnalyzerData::GetDI(const char* canonical_id)
{
   if ( (this == NULL ) || (canonical_id == NULL) ) 
      return NULL;

    DefStore* ds = GetDefStore();

    if (ds == NULL)
	  return NULL;

    return  ds -> GetDefInfo(CurrEngineID(), canonical_id);
}




/* Generate canonical IDs (names) for mml nodes that represent variables and functions.

  In this design, these IDs are zstrings and are completely engine independent.

  They must meet the following criteria

  1. mml nodes that represent the same math object must be given
     the same canonical name.
     Note that in presentation MathML, the same math object can have
     many different markups.  For example, a Greek letter may be represented
     by a hex entity, a decimal entity, or one or more name entities.

  2. mml nodes that represent different math objects must be given
     different canonical names.
	 Note that <mi>x</mi> represents a different math object
	 when it is nested in an <mstyle mathvariant="fraktur"/>

  Finding an algorithm that produces names that meet the above criteria
  is not trivial.

  The most general algorithm would involve traversing the tree from
  the root to the node being named.  When schemata that affect the
  semantics of target node are encountered, add something to the ID string
  being created.  Example
  <mstyle mathvariant="fraktur">
    .
	.
	<subtree to be named>
	</subtree to be named>
    .
  </mstyle>

   When the <mstyle> is encountered, append something like mvFRAKTUR
   to the ID.
   Finally traverse the body of the subtree being named, appending
   semantically relevent attributes and contents to the ID.

   The following code doesn't implement the general algorithm completely.
   It can be made more general as required.

*/

char* GetCanonicalIDforMathNode(const MNODE* mml_node, const Grammar* mml_entities)
{
  char* rv = NULL;
  TCI_ASSERT(CheckLinks(mml_node));
  if (mml_node) {
    char buffer[1024];
    buffer[0] = 0;
    const char* mml_element = mml_node->src_tok;

    if (ElementNameIs(mml_node, "mi")) {

      strcat(buffer, mml_element);
      SemanticAttribs2Buffer(buffer, mml_node, 1024);
      Contents2Buffer(buffer, mml_node->p_chdata, 1024, mml_entities);

    } else if (ElementNameIs(mml_node, "mo")) {

      strcat(buffer, mml_element);
      Contents2Buffer(buffer, mml_node->p_chdata, 1024, mml_entities);

    } else if (ElementNameIs(mml_node, "mn")) {

      strcat(buffer, mml_element);
      Contents2Buffer(buffer, mml_node->p_chdata, 1024, mml_entities);

    } else if (ElementNameIs(mml_node, "mtext")) {

      strcat(buffer, mml_element);
      Contents2Buffer(buffer, mml_node->p_chdata, 1024, mml_entities);

    } else {

        if (mml_node->first_kid) {
          if (!StringEqual(mml_element, "mrow"))
            strcat(buffer, mml_element);

          MNODE* rover = mml_node->first_kid;
          while (rover) {
            char* tmp = GetCanonicalIDforMathNode(rover, mml_entities);
            if (tmp)
              strcat(buffer, tmp);
            delete[] tmp;
            rover = rover->next;
          }
        }

    }

    if (buffer[0] != 0)
	  rv = DuplicateString(buffer);
    

  }
  return rv;
}


