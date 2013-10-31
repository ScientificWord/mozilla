// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

/* Implementation Notes - JBM

 This object massages a presentation MathML tree into a canonical
 form which becomes the starting point for Analyzer's semantic
 analysis.

 Here's what happens.

  1) All chdata is scanned.  Named entities are converted
     to hex - &alpha; -> &#x03B1;

  2) Enclosed lists are converted to fences.

     <mo>(</mo>
     <mrow>				   <mfenced ilk="enclosed-list">
       <mi>x</mi>            <mi>x</mi>
       <mo>,</mo>      =>    <mi>y</mi>
       <mi>y</mi>		   </mfenced>
     </mrow>
     <mo>)</mo>

     (Converting implied objects to explicit objects makes
     semantic analysis easier.)

     Enclosed lists start with a left fence delimiter,
     contain a single item or a comma separated list of items,
     and end with the matching right fence delimiter.

  3) <mrow>s that contain a single element are removed.

 In future, more may need to be done here.  In particular,
 if MathWorkShop is expected to handle poorly formed MathML
 we'll have to convert it to well form markup here.
 (Well formed markup has invisible operators where required
  and <mrow>s are used to make operator precedence explicit.)
 (See LaTeX2MMLTree::FinishMMLBindings() in old MathML export
  filter.)

 NOTE: Precedence bindings are part of semantic analysis.
  If they are ever done here, we may want user controlled input.
*/

#include "Tree2StdMML.h"
#include "Analyzer.h" // jcs bad dependency
#include "Grammar.h"
#include "AnalyzerData.h"
#include "attriblist.h"
#include "strutils.h"
#include <string.h>


Tree2StdMML::Tree2StdMML(const Grammar* mml_grammar, AnalyzerData* analyzer_data) :
     mml_entities(mml_grammar), 
     my_analyzer_data(analyzer_data), 
     mv_stack(NULL), 
     lt_stack(NULL), 
     mDisDerivative(true)
{
}


Tree2StdMML::~Tree2StdMML()
{
  TCI_ASSERT(mv_stack == NULL);
  TCI_ASSERT(lt_stack == NULL);
}


void Tree2StdMML::FixDotDotMN(MNODE* dMML_tree)
{
  if (dMML_tree == NULL)
    return;

  MNODE* rover = dMML_tree;
  while (rover) {
    MNODE* curr = rover;
    rover = rover -> next;
    if (ElementNameIs(curr, "mn")) {
       char* mn = const_cast<char*>(curr -> p_chdata);
       char* p0 = strchr(mn, '.');
       bool found = false;
       while (p0) {
         if (*(p0+1) == '.') {
           // Found ..
           found = true;
           break;
         }
         p0 = strchr(p0+1, '.');          
       }
       if (found){
          // Copy Original data after the ..
          char* rem_content = DuplicateString(p0 + 2);
          
//          if (p0 == curr -> p_chdata){
//             MNODE* prev = curr -> prev;
//            DelinkTNode(curr);
//             DisposeTNode(curr);
//             curr = prev;
//          } else {
             *p0 = '\0';  // Truncate m_pchData
//          }
          // p0 points just after ..
          
          // Create a <mo> with ..
          MNODE* dotdot = MakeTNode(0,0,0);
          SetElementName(dotdot, "mo");
          SetContent(dotdot, "..");

          MNODE* fin = curr->next;
          curr->next = dotdot;
          dotdot->prev = curr;
          
          // Create a remainder <mn>
          MNODE* rem = NULL;
          //if (rem_content && (strlen(rem_content) != 0)){             
             rem = MakeTNode(0,0,0);
             SetElementName(rem, "mn");
             SetContent(rem, rem_content);
          //}

          // link it together
                    
          rem->next = fin;
          if (fin) 
            fin->prev = rem;

          rem->prev = dotdot;
          dotdot->next = rem;

          MNODE* new_row = MakeMROW(curr, rem);
       }
    } else {
       FixDotDotMN(curr -> first_kid);
    }
  } 
}


MNODE* Tree2StdMML::TreeToCanonicalForm(MNODE* dMML_tree,
                                        INPUT_NOTATION_REC* in_notation)
{
  TCI_ASSERT(CheckLinks(dMML_tree));

  MNODE* rv = ChDataToCanonicalForm(dMML_tree);
  // This is too late for removing mixed numbers. We've already removed commas, so 4,1/2 is now a mixed number.
  // RemoveMixedNumbers(rv, NULL);
  rv = FixMFENCEDs(rv);
  rv = InfixDivideToMFRAC(rv);
  rv = RemoveMatrixDelims(rv, in_notation);
  RemoveMSTYLEs(rv);
  rv = RemoveMPADDEDs(rv);
  rv = RemoveRedundantMROWs(rv);
  //  The following is NOT a mistake!
  rv = RemoveRedundantMROWs(rv);
  InsertInvisibleAddSigns(rv);

  TCI_ASSERT(CheckLinks(rv));
  return rv;
}



MNODE* Tree2StdMML::TreeToInterpretForm(MNODE* dMML_tree,
                                        INPUT_NOTATION_REC* in_notation)
{
  TCI_ASSERT(CheckLinks(dMML_tree));
  MNODE* rv = ChDataToCanonicalForm(dMML_tree);
  BindDelimitedGroups(rv);
  BindScripts(rv);
  rv = FixMFENCEDs(rv);
  RemoveMixedNumbers(rv, in_notation);
  FinishFixup(rv);
  rv = InfixDivideToMFRAC(rv);
  rv = RemoveMatrixDelims(rv, in_notation);
  RemoveMSTYLEs(rv);
  rv = RemoveMPADDEDs(rv);
  rv = RemoveRedundantMROWs(rv);
  rv = RemoveRedundantMROWs(rv);

  return rv;
}

// The input to CCID_Fixup may be less well-formed than that for other commands.  Resolve that here.
//XXX second arg is in place of general user prefs parameter
MNODE* Tree2StdMML::TreeToFixupForm(MNODE* dMML_tree, bool D_is_derivative)
{
  MNODE* rv = dMML_tree;
  FixDotDotMN(rv);
  RemoveMSTYLEs(rv);
  RemoveHSPACEs(rv);
  RemoveEmptyTags(dMML_tree);
  // jcs This was put in for the case of edited math having two adjacent MNs.
  // jcs In that case we want them merged. But there are cases when we really 
  // jcs want a list of MNs. See bug 1753. We'll probably want two versions
  // jcs of fixup. One for repairing edited text, and one for internals.
  //FixAdjacentMNs(dMML_tree);
  RemoveMixedNumbers(rv, NULL);

  rv = ChDataToCanonicalForm(rv);
  BindDelimitedGroups(rv);
  BindScripts(rv);
  rv = FixMFENCEDs(rv);
  mDisDerivative = D_is_derivative;
  FinishFixup(rv);
  rv = RemoveRedundantMROWs(rv);
  rv = RemoveRedundantMROWs(rv);
  FixInvisibleFences(rv);
  
  return rv;
}

// The input to CCID_Cleanup may be ugly.  Resolve that here.
MNODE* Tree2StdMML::TreeToCleanupForm(MNODE* dMML_tree)
{
  TCI_ASSERT( ElementNameIs(dMML_tree, "math") );

  RemoveEmptyTags(dMML_tree);
  FixAdjacentMNs(dMML_tree);
  RemoveRedundantMROWs(dMML_tree);

  return dMML_tree;
}

MNODE* Tree2StdMML::ChDataToCanonicalForm(MNODE* dMML_tree)
{
  MNODE* rv = dMML_tree;

  MNODE* rover = dMML_tree;
  while (rover) {
    MNODE* curr = rover;
    rover = rover -> next;

    if (curr -> first_kid) {      // rover is a schemata
      MNODE* c_list = ChDataToCanonicalForm(curr -> first_kid);
      if (c_list != curr -> first_kid) {
        curr -> first_kid = c_list;
        c_list -> parent = curr;
      }
    } else {                    // rover is elementary
      if (curr -> p_chdata) {
        char* tmp = ChdataToString( curr -> p_chdata );
        if (tmp) {
          delete[] curr->p_chdata;
          curr -> p_chdata = tmp;
        } else {
          TCI_ASSERT(!"Unknown character entity or out of memory.");
        }
      } else {
        if (ElementNameIs(curr , "mspace") || ElementNameIs(curr, "maligngroup")) {
          if (curr == rv)
            rv = curr -> next;
          DelinkTNode(curr);
          DisposeTNode(curr);
        }
      }
    }
  }
  return rv;
}

void Tree2StdMML::RemoveMixedNumbers(MNODE* dMML_tree,
                                     INPUT_NOTATION_REC* in_notation)
{
  MNODE* rover = dMML_tree;
  while (rover) {
    if ( rover -> first_kid ) {     // rover is a schemata
      RemoveMixedNumbers( rover -> first_kid, in_notation );
    } else {                    // rover is elementary
      int whole_digits;
      if (ElementNameIs(rover, "mn") && 
          IsAllDigits(rover, whole_digits) && 
          rover->next && 
          ElementNameIs(rover->next, "mfrac") ) {

        MNODE* frac = rover->next;
        if (frac->first_kid) {
          MNODE* numerator = frac -> first_kid;
          MNODE* denominator = numerator -> next;
          if (denominator) {
            int num_digits;
            int den_digits;
            if (IsAllDigits(numerator, num_digits) && IsAllDigits(denominator, den_digits)) 
            {
              bool do_it = !HasPositionalChildren( rover->parent );
              if (whole_digits + den_digits > 9)  // arbitrary complexity limit...note use of U32 in PermutMixedNumbers()
                do_it = false;
              if (do_it) {
                PermuteMixedNumber(rover);
                if (in_notation) 
                  in_notation -> nmixed_numbers++;
              }
            }
          } else {
            TCI_ASSERT(!"Malformed mfrac");
          }
        } else {
          TCI_ASSERT(!"No children of mfrac");
        }
      }
    }
    rover = rover->next;
  }
}



MNODE* Tree2StdMML::FixMFENCEDs(MNODE* dMML_tree)
{
  MNODE* rv = dMML_tree;
  MNODE* rover = dMML_tree;

  while (rover) {

    MNODE* the_next = rover->next;

    if (rover->first_kid) {     // rover is a schemata

        MNODE* c_list = FixMFENCEDs(rover->first_kid);
        if (c_list != rover->first_kid) {
          rover->first_kid = c_list;
          c_list->parent = rover;
        }

    } else {                    // rover is elementary
    
        if (ElementNameIs(rover, "mo") && !HasRequiredChildren(rover->parent)) {
           if (IsFenceMO(rover)) {
             GROUP_INFO gi;
             GetFenceInfo(rover, gi);

             if (IsGroup(gi)) {
               MNODE* mfenced = PermuteTreeToMFENCED(rover, gi);
               if (dMML_tree == rover)
                 rv = mfenced;
               the_next = mfenced;
               if (IsEnclosedList(gi)) {
                 ATTRIB_REC* ar_ilk = new ATTRIB_REC("ilk", "enclosed-list");
                 InsertAttribute(ar_ilk, mfenced->attrib_list);

                 //ar_ilk->next = mfenced->attrib_list;
                 //mfenced->attrib_list->prev = ar_ilk;
                 mfenced->attrib_list = ar_ilk;
               }
             }
           }
        }

    }
    rover = the_next;
  }
  return rv;
}


// Analyzer wants to see invisible fences marked as "I", but that's the wrong MathML
void Tree2StdMML::FixInvisibleFences(MNODE* dMML_tree)
{
  MNODE* rover = dMML_tree;

  while (rover) {

    if (rover->first_kid) {     // rover is a schemata
      FixInvisibleFences(rover->first_kid);
    }

    if (ElementNameIs(rover, "mfenced")) {

        const char* opener = GetATTRIBvalue(rover->attrib_list, "open");
        if (opener && opener[0] == 'I' && opener[1] == 0) {
          ATTRIB_REC* ar_open = new ATTRIB_REC("open", "");
          rover->attrib_list = MergeAttrsLists(rover->attrib_list, ar_open);
        }

    }
    rover = rover->next;
  }
}

void Tree2StdMML::AddOperatorInfo(MNODE* dMML_tree)
{
  MNODE* rover = dMML_tree;

  while (rover) {
    LookupMOInfo(rover);
    LookupEmbellishedMO(rover);
    rover = rover->next;
  }

}

// return precedence of mo according to the MathML Operator Dictionary
void Tree2StdMML::LookupMOInfo(MNODE* mml_node)
{
  if (! ElementNameIs(mml_node, "mo"))
    return;

  int precedence = 0;
  const char* accent_val = GetATTRIBvalue(mml_node->attrib_list, "accent");
  const char* form_val = GetATTRIBvalue(mml_node->attrib_list, "form");
  OpIlk op_ilk = OP_none;

  if (form_val)
    op_ilk = StringToOpIlk(form_val);

  // look up via mml_entities
  const char* entity = mml_node->p_chdata;
  TCI_ASSERT(entity); //mo tags should always have character data!
  bool found_one = false;
  bool set_form = false;
  U32 ID, subID;
  const char* p_data;

  if (mml_entities && entity && mml_entities->GetRecordFromName("MATH", entity, strlen(entity), ID, subID, &p_data)) {
    if (p_data && *p_data) {
      if (strstr(p_data,"multiform,")) {
        set_form = true;
        const char* str_ilk;
        
        if ( (mml_node->prev == 0) || 
             ((mml_node->prev->form != OP_none) && (mml_node->prev->precedence <= 26)) ){
            op_ilk = OP_prefix;
            set_form = true;
        } else if (op_ilk == OP_none && ((ContentIs(mml_node, "&#x2212;") || ContentIs(mml_node, "-")))) {
          if (mml_node->prev == 0 || (ContentIs(mml_node->prev, "&#x2212;") || ContentIs(mml_node->prev, "-")
                                        || ContentIs(mml_node->prev, "+"))) {  
            op_ilk = OP_prefix;
            set_form = true;
          }
        }

        
        if (op_ilk == OP_none)
          str_ilk = "default";
        else
          str_ilk = OpIlkToString(op_ilk);

        size_t new_ln = strlen(entity) + 1 + strlen(str_ilk);
        char *tmp = new char[new_ln + 1];
        strcpy(tmp, entity);
        strcat(tmp, ",");
        strcat(tmp, str_ilk);
        if (mml_entities->GetRecordFromName("MATH", tmp, strlen(tmp), ID, subID, &p_data)) {
          if (p_data && *p_data) {
            found_one = true;
          }
        } else {
          TCI_ASSERT(!"multiform entry not found.");
        }
        delete tmp;
      } else {
        found_one = true;
      }
    }
  }
  if (found_one) {
    const char* str_ilk = p_data;
    const char* str_prec = p_data;

    while (str_prec && *str_prec != ',')
      str_prec++;

    if (*str_prec)
      str_prec++;

    TCI_ASSERT(*str_prec);
    if (!*str_prec)
      return;

    const char* str_unicode = str_prec;

    while (*str_unicode && *str_unicode != ',')
      str_unicode++;

    if (*str_unicode)
      str_unicode++;

    TCI_ASSERT(*str_unicode);
    if (!*str_unicode)
      return;

    precedence = atoi(str_prec);

    if (op_ilk == OP_none) {
      // convert str_ilk .. str_prec to the code
      size_t ilk_len = str_prec - str_ilk - 1;
      char *tmp = new char[ilk_len+1];
      strncpy(tmp,str_ilk,ilk_len);
      tmp[ilk_len] = 0;
      OpIlk new_ilk = StringToOpIlk(tmp);
      if (new_ilk != OP_infix && new_ilk != OP_none && !accent_val)
        set_form = true;
      op_ilk = new_ilk;
      delete tmp;
    }

    if (set_form) {
      const char* str_ilk = OpIlkToString(op_ilk);
      mml_node->attrib_list = StackAttr(mml_node->attrib_list,"form",str_ilk);  // put form on node
    }

  } else {
    TCI_ASSERT(!"Lookup failed.");
  }

  mml_node->precedence = precedence;
  mml_node->form = op_ilk;
}


//embellished operators need to participate in precedence resolution
//we assume here that LookMOInfor has been performed on all subtrees
void Tree2StdMML::LookupEmbellishedMO(MNODE* mml_node)
{
  if (HasScriptChildren(mml_node)) {
    int precedence = 0;
    OpIlk op_ilk = OP_none;
    if (GetBasePrecedence(mml_node->first_kid,precedence,op_ilk)) {
      mml_node->precedence = precedence;
      mml_node->form = op_ilk;
    }
  }
}

bool Tree2StdMML::GetBasePrecedence(MNODE* mml_node, int& precedence, OpIlk& op_ilk)
{
  if (ElementNameIs(mml_node, "mo")) {
    precedence = mml_node->precedence;
    op_ilk = mml_node->form;
    return true;
  } else if (HasScriptChildren(mml_node->first_kid)) {
    return GetBasePrecedence(mml_node->first_kid, precedence, op_ilk);
  } else {
    return false;  // not an embellished operator
  }
}


//CapitalDifferentialD is a special kind of operator
void Tree2StdMML::AddDDOperatorInfo(MNODE* dMML_tree)
{
  MNODE* rover = dMML_tree;
  while (rover) {
    if (ElementNameIs(rover, "msub")) {
      MNODE *base = rover->first_kid;
      if (ElementNameIs(base, "mo") && ContentIs(base, "&#x2145;")) { // &DD;
        LookupMOInfo(base);  //probably not necessary
        rover->precedence = base->precedence;
        rover->form = base->form;
      }
    }
    rover = rover->next;
  }
}

void Tree2StdMML::InsertApplyFunction(MNODE* dMML_list)
{
  MNODE* rover = dMML_list;

  while (rover) {
    bool do_insert = false;
    bool is_delimited = false;
    int n_arg_nodes;

    if (NodeIsFunction(rover, mml_entities, my_analyzer_data)) {
      if (FunctionHasArg(rover,n_arg_nodes,is_delimited)) {
        do_insert = true;
      } else {
        // function name followed by other stuff
      }
    } else if (NodeIsOperator(rover)) {
    } else {
      // implicit function?
    }
    if (do_insert) {
      if (n_arg_nodes > 1) {
        MNODE* end_arg;
        for (end_arg = rover->next; n_arg_nodes > 1 && end_arg ; n_arg_nodes--)
          end_arg = end_arg->next;
        TCI_ASSERT(end_arg);
        MNODE* new_row = MakeMROW(rover->next, end_arg);
        FinishFixup(new_row->first_kid);
      }
      InsertAF(rover);
      AddOperatorInfo(rover);
      rover = rover->next;  // points to af
      rover = rover->next;  // points to argument
    }
    rover = rover->next;
  }
}

void Tree2StdMML::InsertInvisibleTimes(MNODE* dMML_list)
{
  MNODE* rover = dMML_list;
  if (rover && rover->parent) {
      if (ElementNameIs(rover->parent, "mfenced") ||
          ElementNameIs(rover->parent, "mtable") ||
          HasRequiredChildren(rover->parent))
    return;
  }

  while (rover) {
    bool do_insert = false;
    MNODE* candidate = rover;
    rover = rover->next;
          
   
    if (NodeIsNumber(candidate) || NodeIsFactor(candidate)) {
      if (!rover)
        break;
      if (NodeIsTrueNumber(candidate) && NodeIsRationalFraction(rover))
        do_insert = false;  // mixed number
      else if (NodeIsNumber(rover) || NodeIsFactor(rover))
        do_insert = true;
      // LTeX2MML also deals with Qualifiers (x{x!=0}), vectors, DegMinSec
    }
    if (do_insert) {
      InsertIT(candidate);
      AddOperatorInfo(candidate->next);
    }
  }
}

MNODE* Tree2StdMML::BindByOpPrecedence(MNODE* dMML_list, int high, int low)
{
  TCI_ASSERT(high>=low && low>0);
  MNODE* rv = dMML_list;
  bool skipped_an_infix_op = false;

  for (int curr_prec = high; curr_prec >= low; curr_prec--) {
    MNODE* rover = rv;
    while (rover) {
      MNODE* the_next = rover->next;
      if (rover->form != OP_none && rover->precedence == curr_prec) {
        
        switch (rover->form) {

            case OP_prefix:
              if (rover->next) {
                // Check nabla times special case
                if ( ( (rover -> p_chdata) && 0 == strcmp(rover -> p_chdata , "&#x2207;") ) &&
                     ( rover -> next -> p_chdata ) &&
                     ( ( 0 == strcmp(rover -> next -> p_chdata , "&#xd7;") ) || 
                       ( 0 == strcmp(rover -> next -> p_chdata , "&#x22c5;") ) ) )
                {
                   the_next = rover -> next -> next;
                   break;
                }
            	  // If the next thing is also an infix op, skip this one till later
            	  // pass through loop.
            	  if (the_next->form != OP_none && the_next->precedence == curr_prec){
            	    skipped_an_infix_op = true;
            		rover = the_next;
            	  } else{
                	MNODE* new_row = MakeMROW(rover, rover->next);
                	if (new_row != rover) {
                  	the_next = new_row;
                  	if (rover == rv)
                    	  rv = the_next;
            		}
                }
              }
              break;

            case OP_infix: {
              MNODE * end = 0;
              if (rover->prev && rover->next) {
                end = rover->next;
                // scan forward until we hit one of different precedence
                MNODE * inner = end;
                while (inner) {
                  if (inner->form == OP_infix) {
                    if (inner->precedence == curr_prec) {
                      if (inner->next)
                        inner = end = inner->next;
                      else
                        break;
                    } else {
                      break;
                    }
                  } else if (inner->form != OP_none) {
                    break;
                  } else {
                    inner = inner->next;
                  }
                }
                the_next = end;
              } else if (rover->prev){
                end = rover;
              } else {
                  break;
              }

              MNODE* new_row = MakeMROW(rover->prev, end);
              if (new_row != rover->prev) {
                 the_next = new_row;
                 if (rover->prev == rv)
                   rv = the_next;
              }
            }
            break;


            case OP_postfix:
              if (rover->prev && rover->prev->form != OP_postfix && rover->prev->precedence != curr_prec) {
                MNODE* new_row = MakeMROW(rover->prev, rover);
                if (new_row != rover->prev) {
                  the_next = new_row;
                  if (rover->prev == rv)
                    rv = the_next;
                }
              }
              break;

          default:
            TCI_ASSERT(!"This is impossible.");
        }
      }
      rover = the_next;
    }
  }
  return rv;
}


//ApplyFunction binds right to left, unlike any other operator
MNODE* Tree2StdMML::BindApplyFunction(MNODE* dMML_list)
{
  MNODE* rv = dMML_list;
  int curr_prec = 65;
  MNODE* rover = rv;

  // find end of list
  while (rover && rover->next) {
    if (rover->next)
      rover = rover->next;
  }

  // scan for ApplyFunction, backwards
  while (rover) {
    MNODE* the_next = rover->prev;
    if (rover->form != OP_none && rover->precedence == curr_prec) {
      
      switch (rover->form) {
        
        case OP_infix:
          if (rover->prev && rover->next) {
            MNODE* new_row = MakeMROW(rover->prev, rover->next);
            if (new_row != rover->prev) {
              the_next = new_row;
              if (rover->prev == rv)
                rv = the_next;
            }
          }
          break;

        default:
          TCI_ASSERT(!"Non-infix ApplyFunction is impossible.");
      }
    }
    rover = the_next;
  }
  return rv;
}



// change $3 . 4$ to $3.4$ as single mn
MNODE* Tree2StdMML::FixAdjacentMNs(MNODE* dMML_tree)
{
  MNODE* rv = dMML_tree;
  MNODE* rover = dMML_tree;

  while (rover) {
    
    MNODE* the_next = rover->next;
    
    if (rover->first_kid) {     // rover is a schemata

        FixAdjacentMNs(rover->first_kid);

    } else {                    // rover is elementary

        if (IsNumberOrPeriod(rover) && !HasRequiredChildren(rover->parent)) {

          MNODE* right = rover->next;

          if (right && IsNumberOrPeriod(right)) {
            
            // check to make sure we don't get two periods
            if (!(HasPeriod(rover) && HasPeriod(right))) {
              the_next = rover;
              // glue together
              if ( ! ElementNameIs(rover, "mn") )
                SetElementName(rover, "mn");  // might have been mo

              size_t ln = strlen(rover->p_chdata) + strlen(right->p_chdata);
              char *tmp = new char[ln + 1];
              strcpy(tmp, rover->p_chdata);
              strcat(tmp, right->p_chdata);
              delete rover->p_chdata;
              rover->p_chdata = tmp;

              rover->next = right->next;
              if (right->next) {
                right->next->prev = rover;
                right->next = 0;
              }
              DisposeTNode(right);
            }
          }
        }

    }
    rover = the_next;
  }
  return rv;
}


// change $( .. ) ..$ to $<mrow>( <mrow>..</mrow> )</mrow> ..$ so fixMFENCED can work
void Tree2StdMML::BindDelimitedGroups(MNODE* dMML_tree)
{
  MNODE* rover = dMML_tree;

  while (rover) {
    MNODE* the_next = rover->next;
    if (rover->first_kid) {     // rover is a schemata
        BindDelimitedGroups(rover->first_kid);
    } else {                    // rover is elementary
        if (ElementNameIs(rover, "mo") && !HasRequiredChildren(rover->parent)) {
          if (IsLeftFenceMO(rover)) {
            MNODE* lfence = rover;
            GROUP_INFO gi;
            MNODE* rfence = GetMatchingFenceInfo(lfence, gi);
            if (IsGroup(gi)) {
              
              MNODE* new_row = MakeMROW(lfence, rfence);
              if (new_row != lfence)
                the_next = new_row;  // tree structure morphed
              
              if (lfence->next != rfence->prev && lfence->next != rfence)  // atomic content should be OK
                MakeMROW(lfence->next, rfence->prev);
            }
          }
        }
    }
    rover = the_next;
  }

}

// change $( .. )^? ..$ to $<msup><mrow>( .. )</mrow>?</msup> ..$ so fixMFENCED can work
void Tree2StdMML::BindScripts(MNODE* dMML_tree)
{
  MNODE* rover = dMML_tree;

  while (rover) {
    MNODE* the_next = rover->next;
    if (rover->first_kid) {     // rover is a schemata
        BindScripts(rover->first_kid);
    } else {                    // rover is elementary
        if (ElementNameIs(rover, "mo") && !HasRequiredChildren(rover->parent)) {
          if (IsLeftFenceMO(rover)) {
            MNODE* lfence = rover;
            GROUP_INFO gi;
            MNODE* rscript = GetScriptedFenceInfo(lfence, gi);
            if (IsFence(gi)) {
              MNODE* rfence = GetBaseFence(rscript);
              MNODE* new_script = MakeSCRIPT(lfence, rscript);
              if (new_script != lfence)
                the_next = new_script;  // tree structure morphed
              if (lfence->next != rfence->prev)  // atomic content should be OK
                MakeMROW(lfence->next, rfence->prev);
            }
          }
        }
    }
    rover = the_next;
  }
}

MNODE* Tree2StdMML::BindMixedNumbers(MNODE * dMML_list)
{
  MNODE* rv = dMML_list;
  MNODE* rover = rv;

  while (rover && rover->next) {
    MNODE* the_next = rover->next;
    if (NodeIsTrueNumber(rover) && NodeIsRationalFraction(rover->next)) {
      MNODE * new_row = MakeMROW(rover, rover->next);
      if (new_row != rover) {
        the_next = new_row;
        if (rover == rv)
          rv = the_next;
      }
    }
    rover = the_next;
  }
  return rv;
}


MNODE* Tree2StdMML::BindDegMinSec(MNODE * dMML_list)
{
  MNODE* rv = dMML_list;
  MNODE* rover = rv;

  while (rover && rover->next) {
    MNODE* the_next = rover->next;
    bool bb = NeedsInvisiblePlus(rover);
    if (bb) {
        MNODE* l_anchor = rover;
        MNODE* r_anchor = rover->next;
        MNODE* plus = MakeTNode(r_anchor->src_start_offset, 0, r_anchor->src_linenum);
          
        SetElementName(plus, "mo");
        SetContent(plus, "+");

        plus->prev = l_anchor;
        plus->next = r_anchor;

        l_anchor->next = plus;
        r_anchor->prev = plus;

        if (l_anchor->parent)
          plus->parent = l_anchor->parent;
      
    }  
    
    rover = the_next;
  }
  return rv;
}






// \int ... dx

MNODE* Tree2StdMML::BindDelimitedIntegrals(MNODE* dMML_list)
{
  MNODE* rv = dMML_list;
  MNODE* rover = rv;
  while (rover) {
    MNODE* the_next = rover->next;
    if (NodeIsIntegral(rover)) {
      MNODE * new_row = BindIntegral(rover);
      if (new_row != rover) {
        the_next = new_row;
        if (rover == rv)
          rv = the_next;
      }
    }
    rover = the_next;
  }
  return rv;
}


MNODE* Tree2StdMML::BindIntegral(MNODE* dMML_list)
{
  MNODE* rv = dMML_list;
  int n_integrals = GetIntegralCount(dMML_list);
  bool is_delimited = false;
  bool done = false;
  MNODE* i_rover = dMML_list;
  MNODE* i_end = dMML_list;
  
  while (i_rover && !done) {
    i_rover = i_rover->next;
    if (!i_rover)
      break;

    if (NodeIsIntegral(i_rover)) { // Handle nested integrals recursively

        MNODE* tmp = BindIntegral(i_rover);
          if (tmp)
            i_rover = tmp;
        i_end = i_rover;

    } else {	// non-nested integral clause

        bool in_row;
        if (NodeIsDifferential(i_rover,in_row)) {
            PermuteDifferential(i_rover);
            if (!in_row)
            	i_rover = i_rover->next;
            i_end = i_rover;
            

            n_integrals--;
  	        while (i_rover && n_integrals) {

              i_rover = i_rover->next;
              if (!i_rover)
                break;

              if (NodeIsDifferential(i_rover,in_row)) {
            	  PermuteDifferential(i_rover);
            	  if (!in_row)
            		  i_rover = i_rover->next;
                i_end = i_rover;
            	  n_integrals--;
              }
            }

            is_delimited = true;
            done = true;
        }
    }
  }

  // At this point, we've spanned \int ... dx
  if (is_delimited && i_end) {		// we nest
    MNODE* arg = dMML_list->next;
    MNODE* new_row = MakeMROW(arg, i_end);  // bind integrand
    if (new_row != arg) {
      FinishFixup(new_row->first_kid);
    }
    new_row = MakeMROW(dMML_list, new_row);  // bind integral
    if (new_row != dMML_list)
      rv = new_row;
  }

  return rv;
}



// Recognize D_x as differential operator
void Tree2StdMML::FixupCapitalD(MNODE* dMML_list)
{
  MNODE* rover = dMML_list;
  while (rover) {
    if (NodeIsCapitalDifferential(rover) && rover->next)
      PermuteCapitalDifferential(rover);
    rover = rover->next;
  }
}


// Recognize d/dx as differential operator
void Tree2StdMML::FixupSmalld(MNODE* dMML_list)
{
  MNODE* rover = dMML_list;
  while (rover) {
    if (ElementNameIs(rover, "mfrac")) {
      if ( NodeIsDiffNumerator(rover->first_kid) )
        PermuteDiffNumerator(rover->first_kid);

	    if ( NodeIsDiffDenominator(rover->first_kid->next) )
        PermuteDiffDenominator(rover->first_kid->next);
      
    }
    rover = rover->next;
  }
}



// finish MathML bindings
MNODE* Tree2StdMML::FinishFixup(MNODE* dMML_tree)
{  
  if (!dMML_tree)
    return dMML_tree;

  MNODE* rv = dMML_tree;

  if (mDisDerivative)
    FixupCapitalD(rv);  // D_x

  FixupSmalld(rv);  // d/dx

  // depth first recursion
  MNODE* rover = dMML_tree;
  while (rover) {
    MNODE *the_next = rover->next;
    FinishFixup(rover->first_kid);
    rover = the_next;
  }

  // Now, resolve bindings for this list.
  AddOperatorInfo(rv);
  
  if (rv->parent && HasPositionalChildren(rv->parent))
    return rv;

  //rv = BindMixedNumbers(rv);
  //TODO: BindUnits

  //InsertInvisibleAddSigns(rv);  // binds deg/min/sec
  rv = BindDegMinSec(rv);
  rv = BindDelimitedIntegrals(rv);
  
  AddDDOperatorInfo(rv);

  rv = BindByOpPrecedence(rv, 68, 66);
  InsertApplyFunction(rv);  //65
  rv = BindApplyFunction(rv);
  rv = BindByOpPrecedence(rv, 64, 54);
  rv = BindByOpPrecedence(rv, 53, 40);
  InsertInvisibleTimes(rv);  // 39
  rv = BindByOpPrecedence(rv, 39, 39);
  rv = BindByOpPrecedence(rv, 29, 29);
  InsertInvisibleTimes(rv);  // 39
  rv = BindByOpPrecedence(rv, 39, 28);
  rv = BindByOpPrecedence(rv, 27, 2);
  return rv;
}



MNODE* Tree2StdMML::InfixDivideToMFRAC(MNODE* dMML_list)
{
  MNODE* rv = dMML_list;

  MNODE* rover = dMML_list;
  while (rover) {
    MNODE* the_next = rover->next;

    if (ElementNameIs(rover, "mo") && 
         (ContentIs(rover, "/") || ContentIs(rover, "&#xf7;"))) {

      MNODE* l_operand = rover->prev;
      MNODE* r_operand = rover->next;

      if (l_operand && r_operand) {
        MNODE* mfrac = rover;
        SetElementName(rover, "mfrac");

        MNODE* l_anchor = l_operand->prev;
        MNODE* r_anchor = r_operand->next;

        mfrac->prev = l_anchor;
        mfrac->next = r_anchor;
        l_operand->prev = NULL;
        l_operand->next = r_operand;
        r_operand->prev = l_operand;
        r_operand->next = NULL;

        mfrac->first_kid = l_operand;
        
        l_operand->parent = mfrac;
        r_operand->parent = mfrac;


        if (r_anchor)
          r_anchor->prev = mfrac;
        if (l_anchor)
          l_anchor->next = mfrac;
        else
          rv = mfrac;

        the_next = r_anchor;
      }
    }
    rover = the_next;
  }

  rover = rv;
  while (rover) {
    if (rover->first_kid) {     // rover is a schemata
      MNODE *c_list = InfixDivideToMFRAC(rover->first_kid);
      if (c_list != rover->first_kid) {
        rover->first_kid = c_list;
        c_list->parent = rover;
      }
    }
    rover = rover->next;
  }
  return rv;
}


// f(x), sin 2x, g(f(x)), etc.

bool Tree2StdMML::FunctionHasArg(MNODE* mml_func_node, int& n_arg_nodes, bool& is_delimited)
{
  bool rv = false;

  n_arg_nodes = 1;              // assumed, set if otherwise
  is_delimited = false;
  MNODE* base_node = GetBaseFunction(mml_func_node);
  bool do_trigargs = IsTrigArgFuncName(mml_entities,base_node->p_chdata);

  MNODE* arg = mml_func_node->next;
  if (!arg)
    return false;

  if (ElementNameIs(arg, "mo") && ContentIs(arg, "&#x2061;"))
    return false;  // OK, it has an arg but already has ApplyFunction

  if (IsDelimitedGroup(arg)) {
    is_delimited = true;
    return true;
  } else if (ElementNameIs(arg, "mrow") && IsDelimitedGroup(arg->first_kid)) {
    is_delimited = true;
    return true;
  } else if (do_trigargs && FuncTakesTrigArgs(base_node)) {
    n_arg_nodes = CountTrigargNodes(mml_func_node->next);
    return true;
  }

  return rv;
}

// Function to determine if the delimiters on a group
//  are canonically matched. (a) {a} [a] |a| ||a|| <a> etc. 

bool Tree2StdMML::MMLDelimitersMatch(GROUP_INFO& gi)
{
  bool rv = false;

  const char* opener = gi.opening_delim;
  const char* closer = gi.closing_delim;

  if (!opener[0] || !closer[0])
    return false;

  size_t zln_left = strlen(opener);
  size_t zln_right = strlen(closer);

  if (zln_left == 1) {
    if (zln_right == 1) {
      char l_char = opener[0];
      char r_char = closer[0];
      if (l_char == '(') {
        if (r_char == ')')
          rv = true;
      } else if (l_char == '[') {
        if (r_char == ']')
          rv = true;
      } else if (l_char == '{') {
        if (r_char == '}')
          rv = true;
      } else if (l_char == '|') {
        if (r_char == '|')
          rv = true;
      } else if (l_char == '\\') {
        if (r_char == '\\')
          rv = true;
      } else if (l_char == '/') {
        if (r_char == '/')
          rv = true;
      } else if (l_char == 'I') { // invisible

      } else {
        printf("MMLDelimitersMatch() botch: %s vs. %s\n", opener, closer);
        TCI_ASSERT(0);
      }
    }

  } else if (zln_left == 2) {

      if (zln_right == 2) {
        if (opener[0] == '|' && closer[0] == '|')
          if (opener[1] == '|' && closer[1] == '|')
            rv = true;
      }

  } else if (opener[0] == '&' && closer[0] == '&') {

      U32 l_unicode = ASCII2U32(opener + 3, 16);
      U32 r_unicode = ASCII2U32(closer + 3, 16);

      if (l_unicode == 0x2308 && r_unicode == 0x2309)
        rv = true;
      else if (l_unicode == 0x2329 && r_unicode == 0x232A)
        rv = true;
      else if (l_unicode == 0x230A && r_unicode == 0x230B)
        rv = true;
      else if (l_unicode == 0x2191 && r_unicode == 0x2191)
        rv = true;
      else if (l_unicode == 0x2216 && r_unicode == 0x2216)
        rv = true;
      else if (l_unicode == 0xE850 && r_unicode == 0xE851)
        rv = true;
      else if (l_unicode == 0x21D1 && r_unicode == 0x21D1)
        rv = true;
      else if (l_unicode == 0x2018 && r_unicode == 0x2019)
        rv = true;
      else if (l_unicode == 0x21D3 && r_unicode == 0x21D3)
        rv = true;
      else if (l_unicode == 0x3008 && r_unicode == 0x3009)
        rv = true;
      else if (l_unicode == 0xF603 && r_unicode == 0xF604)
        rv = true;
      else if (l_unicode == 0x301A && r_unicode == 0x301B)
        rv = true;
      else if (l_unicode == 0x2016 && r_unicode == 0x2016)
        rv = true;
      else if (l_unicode == 0x201C && r_unicode == 0x201D)
        rv = true;
      else if (l_unicode == 0xF605 && r_unicode == 0xF606)
        rv = true;
      else
        TCI_ASSERT(0);

  } else {
    TCI_ASSERT(0);
  }

  return rv;
}



// f(z), R(R<=5), d(a,b), etc.
// We often need to know what's in a group
//  in order to decide what it represents 
//    1. the argument list of a function  f(x,y)
//    2. an operand               2(x+1)
//    3. parenthetic info         R(R>0)
//    4. an interval            [0,10)

void Tree2StdMML::GetFenceInfo(MNODE* MML_opener, GROUP_INFO& gi)
{
  gi.opening_delim[0] = 0;
  gi.closing_delim[0] = 0;
  gi.has_mtext = false;
  gi.is_mod = false;
  gi.lowest_precedence = 999;
  gi.operator_count = 0;
  gi.separator_count = 0;
  gi.n_interior_nodes = 0;

  MNODE* rover = MML_opener;
  if (rover) {
    MNODE* lfence = rover;
    // Extract the left delimiter - if it exists
    if (IsFenceMO(lfence)) {
      if (lfence->p_chdata)
        strcpy(gi.opening_delim, lfence->p_chdata);
      else
        strcpy(gi.opening_delim, "I");
      rover = rover->next;
    }

    MNODE* body = rover;
    int n_interior_nodes = 0;
    MNODE* rfence = rover;
    
    if (rfence) {
      if (IsFenceMO(rfence)) {
        body = NULL;
      } else {
        while (rfence->next) {
          n_interior_nodes++;
          rfence = rfence->next;
        }
      }
      if (IsFenceMO(rfence)) {
        // Extract the right delimiter
        if (rfence->p_chdata)
          strcpy(gi.closing_delim, rfence->p_chdata);
        else
          strcpy(gi.closing_delim, "I");
      } else {
        rfence = NULL;
      }
    }

    if (body) {                 // there's an interior
      if (n_interior_nodes == 1 && ElementNameIs(body, "mrow"))
        body = body->first_kid;
      GetGroupInsideInfo(body, rfence, gi);
    }

  }
}

MNODE *Tree2StdMML::GetMatchingFenceInfo(MNODE* MML_opener, GROUP_INFO& gi)
{
  gi.opening_delim[0] = 0;
  gi.closing_delim[0] = 0;
  gi.has_mtext = false;
  gi.is_mod = false;
  gi.lowest_precedence = 999;
  gi.operator_count = 0;
  gi.separator_count = 0;
  gi.n_interior_nodes = 0;

  MNODE* rover = MML_opener;
  MNODE* rfence = NULL;
  if (rover) {
    MNODE* lfence = rover;
    // Extract the left delimiter - if it exists
    if (IsLeftFenceMO(lfence)) {
      if (lfence->p_chdata)
        strcpy(gi.opening_delim, lfence->p_chdata);
      else
        strcpy(gi.opening_delim, "I");
      rover = rover->next;
    }

    MNODE* body = rover;
    int n_interior_nodes = 0;
    rfence = rover;
    if (rfence) {
      int nest = 1;  // we need to skip over nested fences
      if (IsRightFenceMO(rfence)) {
        body = NULL;
        nest = 0;
      } else {
        if (IsLeftFenceMO(rfence))
          nest++;
        while (rfence->next) {
          n_interior_nodes++;
          rfence = rfence->next;
          if (IsRightFenceMO(rfence) || IsScriptedFenceMO(rfence)) {
            nest--;
            if (nest == 0)
              break;
          } else if (IsLeftFenceMO(rfence)) {
            nest++;
          }
        }
      }
      if (IsRightFenceMO(rfence) && nest == 0) {
        // Extract the right delimiter
        if (rfence->p_chdata)
          strcpy(gi.closing_delim, rfence->p_chdata);
        else
          strcpy(gi.closing_delim, "I");
      } else {
        rfence = NULL;
      }
    }

    if (body) {                 // there's an interior
      if (n_interior_nodes == 1 && ElementNameIs(body, "mrow"))
        body = body->first_kid;
      GetGroupInsideInfo(body, rfence, gi);
    }
  }
  return rfence;
}

MNODE* Tree2StdMML::GetScriptedFenceInfo(MNODE* MML_opener, GROUP_INFO& gi)
{
  gi.opening_delim[0] = 0;
  gi.closing_delim[0] = 0;
  gi.has_mtext = false;
  gi.is_mod = false;
  gi.lowest_precedence = 999;
  gi.operator_count = 0;
  gi.separator_count = 0;
  gi.n_interior_nodes = 0;

  MNODE* rover = MML_opener;
  MNODE* rfence = NULL;
  if (rover) {
    MNODE *lfence = rover;
    // Extract the left delimiter - if it exists
    if (IsLeftFenceMO(lfence)) {
      if (lfence->p_chdata)
        strcpy(gi.opening_delim, lfence->p_chdata);
      else
        strcpy(gi.opening_delim, "I");
      rover = rover->next;
    }

    MNODE* body = rover;
    int n_interior_nodes = 0;
    rfence = rover;
    if (rfence) {
      int nest = 1;  // we need to skip over nested fences
      if (IsScriptedFenceMO(rfence)) {
        body = NULL;
      } else {
        while (rfence->next) {
          n_interior_nodes++;
          rfence = rfence->next;
          if (IsScriptedFenceMO(rfence)) {
            nest--;
            if (nest == 0)
              break;
          } else if (IsLeftFenceMO(rfence)) {
            nest++;
          }
        }
      }
      if (IsScriptedFenceMO(rfence) && nest == 0) {
        // Extract the right delimiter
        MNODE *basefence = GetBaseFence(rfence);
        if (basefence->p_chdata)
          strcpy(gi.closing_delim, basefence->p_chdata);
        else
          strcpy(gi.closing_delim, "I");
      } else {
        rfence = NULL;
      }
    }

    if (body) {                 // there's an interior
      if (n_interior_nodes == 1 && ElementNameIs(body, "mrow"))
        body = body->first_kid;
      GetGroupInsideInfo(body, rfence, gi);
    }
  }
  return rfence;
}

void Tree2StdMML::GetGroupInsideInfo(MNODE* MML_cont, MNODE* rfence, GROUP_INFO& gi)
{
  // Iterate MML_list to see what's in this delimited group.
  MNODE* rover = MML_cont;
  while (rover) {
    if (rfence && rover == rfence)
      break;
    gi.n_interior_nodes++;

    if (ElementNameIs(rover, "mo")) {
      gi.operator_count++;

      if (rover->p_chdata)
        if (rover->p_chdata[0] == ',')
          gi.separator_count++;

      if (gi.n_interior_nodes == 1 && gi.operator_count == 1)
        if (rover->p_chdata)
          if (ContentIs(rover, "mod"))
            gi.is_mod = true;
    } else if (ElementNameIs(rover, "mi")) {
      if (gi.n_interior_nodes == 1)
        if (rover->p_chdata)
          if (ContentIs(rover, "mod"))
            gi.is_mod = true;
    } else if (ElementNameIs(rover, "mtext")) {
      gi.has_mtext = true;
    }
    rover = rover->next;
  }
}

// Sadly, fence information isn't in our operator dictionary, so
// we hardcode it here.
bool Tree2StdMML::IsFenceMO(MNODE* mml_node)
{
  bool rv = false;

  if (ElementNameIs(mml_node, "mo")) {
    ATTRIB_REC *arover = mml_node->attrib_list;
    while (arover) {
      if (!StringEqual(arover->zattr_nom, "fence")) {
        arover = arover->GetNext();
      } else {
        if (StringEqual("true", arover->zattr_val))
          rv = true;
        break;
      }
    }
    if (!rv) {
      if (mml_node->p_chdata) {
        // look for <mo>s that are fencing ops by default
        const char* delim_str = mml_node->p_chdata;
        size_t zln = strlen(delim_str);
        if (zln == 1) {
          char ch = delim_str[0];
          if (ch == '(' || ch == ')' || ch == '[' || ch == ']' ||
              ch == '{' || ch == '}' || ch == '|')
            rv = true;
        } else if (zln == 8) {
          if (StringEqual("&#x230a;", delim_str))   // LeftFloor
            rv = true;
          else if (StringEqual("&#x230b;", delim_str))   // RightFloor
            rv = true;
          else if (StringEqual("&#x2308;", delim_str))   // LeftCeiling
            rv = true;
          else if (StringEqual("&#x2309;", delim_str))   // RightCeiling
            rv = true;
          else if (StringEqual("&#x2016;", delim_str))   // Verbar
            rv = true;
        }
      } else {
        rv = true;
      }
    }
  }
  return rv;
}

bool Tree2StdMML::IsLeftFenceMO(MNODE* mml_node)
{
  bool rv = false;
  bool fence_true = false;
  bool prefix_true = false;
  bool fence_false = false;
  bool prefix_false = false;

  if (ElementNameIs(mml_node, "mo")) {
    ATTRIB_REC* arover = mml_node->attrib_list;
    while (arover) {
      if (StringEqual(arover->zattr_nom, "fence")) {
        if (StringEqual("true", arover->zattr_val))
          fence_true = true;
        else if (StringEqual("false", arover->zattr_val))
          fence_false = true;
      }
      else if (StringEqual(arover->zattr_nom, "form")) {
        if (StringEqual("prefix", arover->zattr_val))
          prefix_true = true;
        else
          prefix_false = true;
      }
      arover = arover->GetNext();
    }
    if (fence_true && prefix_true)
      return true;
    if (fence_false || prefix_false)
      return false;

    // look for <mo>s that are left fencing ops by default
    if (mml_node->p_chdata) {
      const char* delim_str = mml_node->p_chdata;
      size_t zln = strlen(delim_str);
      if (zln == 1) {
        char ch = delim_str[0];
        if (ch == '(' || ch == '[' || ch == '{' || ch == '|')
          rv = true;
      } else if (zln == 8) {
        if (StringEqual("&#x230a;", delim_str))   // LeftFloor
          rv = true;
        else if (StringEqual("&#x2308;", delim_str))   // LeftCeiling
          rv = true;
        else if (StringEqual("&#x2016;", delim_str))   // Verbar
          rv = true;
      }
    }
  }
  return rv;
}

bool Tree2StdMML::IsRightFenceMO(MNODE* mml_node)
{
  bool rv = false;
  bool fence_true = false;
  bool postfix_true = false;
  bool fence_false = false;
  bool postfix_false = false;

  if (ElementNameIs(mml_node, "mo")) {
    ATTRIB_REC *arover = mml_node->attrib_list;
    while (arover) {
      if (StringEqual(arover->zattr_nom, "fence")) {
        if (StringEqual("true", arover->zattr_val))
          fence_true = true;
        else if (StringEqual("false", arover->zattr_val))
          fence_false = true;
      }
      else if (StringEqual(arover->zattr_nom, "form")) {
        if (StringEqual("postfix", arover->zattr_val))
          postfix_true = true;
        else
          postfix_false = true;
      }
      arover = arover->GetNext();
    }
    if (fence_true && postfix_true)
      return true;
    if (fence_false || postfix_false)
      return false;

    // look for <mo>s that are right fencing ops by default
    if (mml_node->p_chdata) {
      const char* delim_str = mml_node->p_chdata;
      size_t zln = strlen(delim_str);
      if (zln == 1) {
        char ch = delim_str[0];
        if (ch == ')' || ch == ']' || ch == '}' || ch == '|')
          rv = true;
      } else if (zln == 8) {
        if (StringEqual("&#x230b;", delim_str))   // RightFloor
          rv = true;
        else if (StringEqual("&#x2309;", delim_str))   // RightCeiling
          rv = true;
        else if (StringEqual("&#x2016;", delim_str))   // Verbar
          rv = true;
      }
    }
  }
  return rv;
}

// Note: we don't use the concept of embellished fence from the MathML spec.
bool Tree2StdMML::IsScriptedFenceMO(MNODE* mml_node)
{
  MNODE *fence = GetBaseFence(mml_node);
  if (fence)
    return IsRightFenceMO(fence);
  else
    return false;
}

bool Tree2StdMML::IsDelimitedGroup(MNODE* mml_node)
{
  return ElementNameIs(mml_node, "mfenced");
}

bool Tree2StdMML::IsEmptyMO(MNODE* mml_node)
{
  if (! ElementNameIs(mml_node, "mo") )
    return false;

  if (mml_node->p_chdata && *mml_node->p_chdata)
    return false;

  //ATTRIB_REC* arover = mml_node->attrib_list;
  //while (arover) {
  //  if (StringEqual(arover->zattr_nom, "fence"))
  //    if (StringEqual("true", arover->zattr_val))
  //      return false;
  //  arover = arover->GetNext();
  //}
  return true;
}

bool Tree2StdMML::NodeIsNumber(MNODE* mml_node)
{
  if (ElementNameIs(mml_node, "mn"))
    return true;

  if (HasScriptChildren(mml_node))
    return NodeIsNumber(mml_node->first_kid);
  else
    return false;
}

// Check for an integer w/o structure
bool Tree2StdMML::NodeIsTrueNumber(MNODE* mml_node)
{
  int dummy;
  if (ElementNameIs(mml_node, "mn"))
    return IsAllDigits(mml_node,dummy);
  else
    return false;
}

// Check for n/m where n,m integers
bool Tree2StdMML::NodeIsRationalFraction(MNODE* mml_node)
{
  if (ElementNameIs(mml_node, "mfrac")) {
    MNODE * rover = mml_node->first_kid;
    if (rover) {
      if (NodeIsTrueNumber(rover)) {
        rover = rover->next;
        if (rover && NodeIsTrueNumber(rover))
          return true;
      }
    }
  }
  return false;
}





//assuming NodeIsFunction() true, find the actual function name node
MNODE* Tree2StdMML::GetBaseFunction(MNODE* mml_node)
{
  if (ElementNameIs(mml_node, "mi") || ElementNameIs(mml_node, "mo"))
    return mml_node;
  if (HasScriptChildren(mml_node))
    return GetBaseFunction(mml_node->first_kid);

  TCI_ASSERT(!"Shouldn't get here.  This wasn't an embellished function.");
  return 0;
}



bool Tree2StdMML::FuncTakesTrigArgs(MNODE* mml_node)
{
  return IsTrigArgFuncName(mml_entities, mml_node->p_chdata);
}



bool Tree2StdMML::NodeIsOperator(MNODE* mml_node)
{
  
  if (ElementNameIs(mml_node, "mo")) {

        return true;

  } else if (ElementNameIs(mml_node, "mi")) {

      //XXX  might be max/lim/lcm etc.
	  if ( ContentIs(mml_node, "lim") || ContentIs(mml_node, "max") ||
	       ContentIs(mml_node, "min") ||	ContentIs(mml_node, "lcm") ||
		     ContentIs(mml_node, "gcd") || ContentIs(mml_node, "seq") ) {
	    return true;
	  }	else {
	    //TODO look up in def. store and other places

        return false;
	  }

  } else if (HasScriptChildren(mml_node) || ElementNameIs(mml_node, "mfrac")) {
	  
      return NodeIsOperator(mml_node->first_kid);

  } else if (ElementNameIs(mml_node, "mrow")) {

      if (mml_node->first_kid && mml_node->first_kid->next)
        return false;
      else
        return NodeIsOperator(mml_node->first_kid);

  } else {

    return false;

  }
}



bool Tree2StdMML::NodeIsIntegral(MNODE* mml_node)
{
  return GetIntegralCount(mml_node) > 0;
}



// This looks a lot like Analyzer::GetBigOpType()
int Tree2StdMML::GetIntegralCount(MNODE* mml_node)
{
  if (ElementNameIs(mml_node, "msubsup") ||
      ElementNameIs(mml_node, "munderover"))
    return GetIntegralCount(mml_node->first_kid);

  if (! ElementNameIs(mml_node, "mo"))
    return 0;

  //TODO study Unicode list (this list from SWP)
  if (! (mml_node->p_chdata)) 
    return 0;

  const char *ptr = strstr(mml_node->p_chdata, "&#x");
  if (ptr) {
    U32 unicode = ASCII2U32(ptr + 3, 16);
    switch (unicode) {
      case 0x222b: //&int;<uID7.1.1>prefix,31,U0222B
        return 1;
      case 0x222c: //&Int;<uID7.1.2>prefix,31,U0222C
        return 2;
      case 0x222d: //&tint;<uID7.1.3>prefix,31,U0222D
        return 3;
      case 0x2a0c: //&qint;<uID7.1.4>prefix,31,U02A0C
        return 4;
      case 0x222e: //&conint;<uID7.1.6>prefix,31,U0222E
        return 1;
    }
  }
  return 0;
}



//TODO check that it's followed by a variable

bool Tree2StdMML::NodeIsDifferential(MNODE* mml_node, bool& nested)
{
  nested = false;
  if (ElementNameIs(mml_node, "mrow")) {

      nested = true;
      bool dummy;
      return NodeIsDifferential(mml_node->first_kid, dummy);

  } else if (ElementNameIs(mml_node, "mo")) {

      return ContentIs(mml_node, "&#x2146;") || // differentiald
             ContentIs(mml_node, "d");

  } else if (ElementNameIs(mml_node, "mi")) {

      return ContentIs(mml_node, "d");

  } else {

      return false;
  }
}



// Assuming NodeIsDifferential() is true, change d to &dd;
void Tree2StdMML::PermuteDifferential(MNODE* mml_node)
{
  if ((ElementNameIs(mml_node, "mi") && ContentIs(mml_node,"d")) ||
      (ElementNameIs(mml_node, "mo") && ContentIs(mml_node,"d"))) {

    SetElementName(mml_node, "mo");
    SetContent(mml_node, "&#x2146;");
    LookupMOInfo(mml_node);

  }
}



bool Tree2StdMML::NodeIsCapitalDifferential(MNODE* mml_node)
{
  if (! ElementNameIs(mml_node, "msub")) {
    return false;
  }
  MNODE* base = mml_node->first_kid;

  if ((ElementNameIs(base, "mi") && ContentIs(base, "D")) ||
      (ElementNameIs(base, "mo") && ContentIs(base, "D"))) {

     return NodeIsVariableList(base->next);

  } else {
     return false;
  }
}



// Assuming NodeIsCapitalDifferential() is true, change D to &DD;
void Tree2StdMML::PermuteCapitalDifferential(MNODE* mml_node)
{
  if (! ElementNameIs(mml_node, "msub"))
    return;  //shouldn't get here
  
  MNODE* theD = mml_node->first_kid;
  if ((ElementNameIs(theD, "mi") && ContentIs(theD, "D")) ||
      (ElementNameIs(theD, "mo") && ContentIs(theD, "D"))) {

    SetElementName(theD, "mo");
    SetContent(theD, "&#x2145;"); 
    LookupMOInfo(theD);
  }
}



// Assuming NodeIsDiffNumerator() is true, change d to &dd;
void Tree2StdMML::PermuteDiffNumerator(MNODE* mml_node)
{
  MNODE* rover = mml_node;

  if (ElementNameIs(rover, "mrow"))
    rover = rover->first_kid;

  if (ElementNameIs(rover, "msup"))
    rover = rover->first_kid;

  PermuteDifferential(rover);
}



// Assuming NodeIsDiffDenominator() is true, change d to &dd;
void Tree2StdMML::PermuteDiffDenominator(MNODE* mml_node)
{
  MNODE* rover = mml_node;

  if (ElementNameIs(rover, "mrow"))
    PermuteDiffDenominator(rover->first_kid);

  if (ElementNameIs(rover, "msup"))
    PermuteDifferential(rover->first_kid);
  else
    PermuteDifferential(rover);

  rover = rover->next;

  if (rover)
    rover = rover->next;

  if (rover)
    PermuteDiffDenominator(rover);
}



bool Tree2StdMML::NodeIsDiffNumerator(MNODE* mml_node)
{
  if (ElementNameIs(mml_node, "mrow") || ElementNameIs(mml_node, "msup")) {
      return NodeIsDiffNumerator(mml_node->first_kid);
  } else {
      bool nested; // unused
      return NodeIsDifferential(mml_node, nested);
  }
}



// dx dy or dx^3 or d^3x
bool Tree2StdMML::NodeIsDiffDenominator(MNODE* mml_node)
{
  if (ElementNameIs(mml_node, "mrow")) {
      
      return NodeIsDiffNumerator(mml_node->first_kid);

  } else if (ElementNameIs(mml_node, "msup")) {
      
      if (!NodeIsDiffNumerator(mml_node->first_kid))
        return false;

  } else {
      
      bool nested; // unused
      if (!NodeIsDifferential(mml_node,nested))
        return false;
  }

  MNODE* rover = mml_node->next;
  if (!rover)
    return false;

  if (!NodeIsVariableList(rover))
    return false;

  rover = rover->next;

  if (!rover)
    return true;
  else
    return NodeIsDiffDenominator(rover);
}



//TODO: check the end of the list
//TODO: decorated variables?
bool Tree2StdMML::NodeIsVariableList(MNODE* mml_node)
{
  if (ElementNameIs(mml_node, "mi"))
    return true;
  else if (ElementNameIs(mml_node, "mrow") || ElementNameIs(mml_node, "msup"))
    return NodeIsVariableList(mml_node->first_kid);
  else
    return false;
}



// Quite different from the corresponding function in LTeX2MML.cpp
bool Tree2StdMML::NodeIsFactor(MNODE* mml_node)
{
  //NOTE assuming fences are all mfenced
  //if (NodeIsOperator(mml_node) || NodeIsFunction(mml_node, mml_entities, my_analyzer_data))
  if (NodeIsOperator(mml_node))
    return false;

  if (ElementNameIs(mml_node, "mtr") || ElementNameIs(mml_node, "mtd"))
    return false;
 
  return true;
}



// return first leaf child at base of msup
MNODE* Tree2StdMML::GetBaseFence(MNODE* mml_node)
{
  if (ElementNameIs(mml_node, "msup")) {
    return mml_node->first_kid;
  } else if (ElementNameIs(mml_node, "msub")) {
    return mml_node->first_kid;
  } else if (ElementNameIs(mml_node, "msubsup")) {
    return mml_node->first_kid;
  } else {
    return NULL;
  }
}



char* Tree2StdMML::ChdataToString(const char* p_chdata)
{
  char* rv = NULL;
  U32 rln = 0;

  if (p_chdata && *p_chdata) {
    
    const char* ptr = p_chdata;
    while (*ptr) {
      U32 unicode = 0;
      char ch = 0;
      if (*ptr == '&') {
        ptr++;
        if (*ptr == '#') {
          // numeric entity - &#x201a;, &#9876;. etc.
          ptr++;
          U32 place_val = 10;
          if (*ptr == 'x') {
            place_val = 16;
            ptr++;
          }
          unicode = ASCII2U32(ptr, place_val);
          while (*ptr && *ptr != ';')
            ptr++;
        } else {
          // non-numeric entity - &theta;, &ApplyFunction;. etc.
          const char* entity = ptr - 1;
          while (*ptr && *ptr != ';')
            ptr++;
          U32 ID, subID;
          size_t zln = ptr - entity + 1;
          const char* p_data;
          if (mml_entities->GetRecordFromName("MATH", entity, zln, ID, subID, &p_data)) {
            if (p_data && *p_data) {
              //&ApplyFunction;<uID3.5.6>infix,65,U02061
              const char* ptr = strstr(p_data, ",U");
              if (ptr) {
                unicode = ASCII2U32(ptr + 2, 16);
              } else {
                TCI_ASSERT(!"No Unicode value found for this entity.");
              }
            } else {
              TCI_ASSERT(!"No grammar file entry for this entity.");
            }
          } else {
            char buffer[80];
            strncpy(buffer, entity, zln);
            buffer[zln] = 0;
            rv = AppendStr2HeapStr(rv, rln, buffer);
          }
        }
      } else {
        // not an &entity; - ASCII character
        ch = *ptr;
      }

      if (ch == '<' || ch == '>' || ch == '&') {
        unicode = ch;
        ch = 0;
      }
      // A bit of a hack.  Only known usage is in separation of function args
      if (ch == ';') {
        ch = ',';
      }

      char buffer[32];
      if (ch) {
        buffer[0] = ch;
        buffer[1] = 0;
      } else if (unicode) {
        sprintf(buffer, "&#x%x;", unicode);
      } else {
        buffer[0] = 0;
      }
      rv = AppendStr2HeapStr(rv, rln, buffer);
      ptr++;
    }
  }

  return rv;
}



bool Tree2StdMML::IsGroup(GROUP_INFO& gi)
{
  return (gi.opening_delim[0] && gi.closing_delim[0]);
}



bool Tree2StdMML::IsFence(GROUP_INFO& gi)
{
  if (gi.opening_delim[0] && gi.closing_delim[0]) {
    if (gi.separator_count) {
      if (gi.operator_count == gi.separator_count)
        return true;
    } else {
      return true;
    }
  }

  return false;
}



bool Tree2StdMML::IsEnclosedList(GROUP_INFO& gi)
{
  bool rv = false;
  if (MMLDelimitersMatch(gi)) {
    char ch0 = gi.opening_delim[0];

    if (ch0 == '(' || ch0 == '[' || ch0 == '{') {
      if (gi.n_interior_nodes > 1 && 
          gi.operator_count == gi.separator_count) {
        rv = true;
      } else if (gi.n_interior_nodes > 2 && gi.separator_count > 0) {
        rv = true;
        // may need more logic here!
      }
    }
  }
  return rv;
}



int Tree2StdMML::CountTrigargNodes(MNODE* mml_node)
{
  int rv = 1;
  MNODE* rover = mml_node;
  if (rover)
    rover = rover->next;  // first one always OK
  else
    rv = 0;
  bool last_was_op = false;
  while (rover) {
    bool is_op;
    if (!NodeInTrigargList(rover, is_op)) {
      if (last_was_op)
        rv--;
      break;
    } else {
      last_was_op = is_op;
      rv++;
      rover = rover->next;
    }
  }
  return rv;
}



// TODO review this list carefully
bool Tree2StdMML::NodeInTrigargList(MNODE* mml_node, bool& is_op)
{
  is_op = false;
  if (mml_node) {
    //const char* p_elem = mml_node->src_tok;
    if (ElementNameIs(mml_node, "mrow") || 
        ElementNameIs(mml_node, "mroot")|| 
        ElementNameIs(mml_node, "msqrt")) {

      return true;

    } else if (ElementNameIs(mml_node, "msub") || 
               ElementNameIs(mml_node, "msup") || 
               ElementNameIs(mml_node, "msubsup")) {

      bool dummy;
      return NodeInTrigargList(mml_node->first_kid, dummy);

    } else if (ElementNameIs(mml_node, "mo")) {
      is_op = true;
      // true if multiplicative (this needs to be refactored)
      const char *ptr = mml_node->p_chdata;
      if (ptr && *ptr == '&' && *(ptr + 1) == '#') { // numeric entity
        int off = 2;
        int base = 10;            // "&#1234;"
        if (*(ptr + 2) == 'x') {
          base = 16;              // "&#x220a;"
          off++;
        }
        U32 unicode = ASCII2U32(ptr + off, base);
        switch (unicode) {
          case 0xd7  :                  // "times"
          case 0x2062:                  // "invisibletimes"
          case 0x2217:                  // "lowast"
          case 0x22c5:                  // "sdot"
          case 0x22c6:                  // "starf"
            return true;
        }
      } else if (ptr) {
        size_t zln = strlen(ptr);
        if (zln == 1) {
          char ch0 = ptr[0];
          if (ch0 == '*' || ch0 == '/' || ch0 == '\\')
            return true;
        }
      }
    } else if (ElementNameIs(mml_node, "mi")) {

      return (!IsTrigArgFuncName(mml_entities, mml_node->p_chdata));
    }
  }
  return false;
}



MNODE* Tree2StdMML::RemoveRedundantMROWs(MNODE* MML_list)
{
  MNODE* rv = MML_list;

  MNODE* rover = MML_list;
  while (rover) {
    MNODE* the_next = rover->next;
    if (ElementNameIs(rover, "mrow")) {
      MNODE* eldest = rover->first_kid;
      bool zerokids = !eldest;
      bool onekid = eldest && !eldest->next;
      if ((zerokids && !HasRequiredChildren(rover->parent)) || onekid) {
        if (rover == MML_list)
          rv = eldest;
        MNODE* parent = rover->parent;
        MNODE* left_anchor = rover->prev;
        MNODE* right_anchor = rover->next;

        rover->first_kid = NULL;
        DelinkTNode(rover);
        DisposeTNode(rover);

        if (eldest) {
          eldest->parent = parent;
          if (left_anchor) {
            left_anchor->next = eldest;
            eldest->prev = left_anchor;
          } else {
            parent->first_kid = eldest;
          }
          if (right_anchor) {
            right_anchor->prev = eldest;
            eldest->next = right_anchor;
          }
          the_next = eldest;
        } else {
          if (left_anchor) {
            left_anchor->next = right_anchor;
            if (right_anchor)
              right_anchor->prev = left_anchor;
          } else {
            parent->first_kid = right_anchor;
          }
          the_next = right_anchor;
        }
      } else {
        RemoveRedundantMROWs(rover->first_kid);
      }
    } else if (rover->first_kid) {  // rover is a schemata
      RemoveRedundantMROWs(rover->first_kid);
    }
    rover = the_next;
  }
  return rv;
}



MNODE* Tree2StdMML::RemoveEmptyTags(MNODE* MML_list)
{
  MNODE* rv = MML_list;
  MNODE* rover = MML_list;
  while (rover) {
    MNODE* the_next = rover->next;
    if (rover->first_kid) {  // rover is a schemata
        RemoveEmptyTags(rover->first_kid);
    } else {
        if (IsEmptyMO(rover)
            || ElementNameIs(rover, "mn")
            || (ElementNameIs(rover, "mi") && (0 == GetATTRIBvalue(rover->attrib_list, "tempinput"))) ) {
          if (!rover->p_chdata || !*rover->p_chdata) {
            if (!HasRequiredChildren(rover->parent)) {
              DelinkTNode(rover);
              DisposeTNode(rover);
            } else {
              // Replace with the node with a tempinput mi
              MNODE* inputbox = MakeTNode(0,0,0);
              SetElementName(inputbox, "mi");
              ATTRIB_REC* ar = new ATTRIB_REC("tempinput", "true");
              inputbox->attrib_list = ar;

              inputbox->next = rover->next;
              inputbox->prev = rover->prev;
              inputbox->parent = rover->parent;

              inputbox->p_chdata = DuplicateString("&#x200B;"); 

              if (rover -> prev)
                rover->prev->next = inputbox;
              if (rover -> next)
                rover->next->prev = inputbox;
              //DelinkTNode(rover);
              DisposeTNode(rover);
            }
          }
        }
    }
    rover = the_next;
  }
  return rv;
}



MNODE* Tree2StdMML::RemoveMPADDEDs(MNODE* MML_list)
{
  MNODE* rv = MML_list;

  MNODE* rover = MML_list;
  while (rover) {
    MNODE* the_next = rover->next;
    if (ElementNameIs(rover, "mpadded")) {
      MNODE *eldest = rover->first_kid;
      if (!eldest || !eldest->next) {
        MNODE* parent = rover->parent;
        MNODE* left_anchor = rover->prev;
        MNODE* right_anchor = rover->next;

        if (rover == MML_list)
          rv = eldest;

        rover->first_kid = NULL;
        DelinkTNode(rover);
        DisposeTNode(rover);

        if (eldest) {
          eldest->parent = parent;
          if (left_anchor) {
            left_anchor->next = eldest;
            eldest->prev = left_anchor;
          } else {
            parent->first_kid = eldest;
          }
          if (right_anchor) {
            right_anchor->prev = eldest;
            eldest->next = right_anchor;
          }
          the_next = eldest;
        } else {
          if (left_anchor) {
            left_anchor->next = right_anchor;
            if (right_anchor)
              right_anchor->prev = left_anchor;
          } else {
            parent->first_kid = right_anchor;
          }
          the_next = right_anchor;
        }
      } else {
        RemoveMPADDEDs(rover->first_kid);
      }
    } else if (rover->first_kid) {  // rover is a schemata
      RemoveMPADDEDs(rover->first_kid);
    }
    rover = the_next;
  }

  return rv;
}



void Tree2StdMML::RemoveMSTYLEs(MNODE* MML_list)
{
  TCI_ASSERT(CheckLinks(MML_list));

  MNODE* rover = MML_list;
  while (rover) {
    MNODE* the_next = rover->next;
    if (ElementNameIs(rover, "mstyle")) {

        RemoveMSTYLEnode(rover);

    } else if (rover->first_kid) {  // rover is a schemata

        if (ElementNameIs(rover, "mfrac"))
          if (lt_stack)
            InstallStackedAttr(rover, lt_stack);
        RemoveMSTYLEs(rover->first_kid);

    } else {                    // rover is atomic

        if (ElementNameIs(rover, "mi"))
          if (mv_stack)
            InstallStackedAttr(rover, mv_stack);

    }
    rover = the_next;
  }

  
}

void Tree2StdMML::RemoveHSPACEs(MNODE* MML_list)
{
  TCI_ASSERT(CheckLinks(MML_list));

  MNODE* rover = MML_list;
  while (rover) {
    MNODE* the_next = rover->next;
    if (ElementNameIs(rover, "hspace")) {

       DelinkTNode(rover);
       DisposeTNode(rover);

    } else if (rover->first_kid) {  // rover is a schemata

        RemoveHSPACEs(rover->first_kid);

    }
    rover = the_next;
  }  
}


// Remove Invisible times and apply function
void Tree2StdMML::RemoveIT_and_AF(MNODE* MML_list)
{
  TCI_ASSERT(CheckLinks(MML_list));

  MNODE* rover = MML_list;
  while (rover) {
    MNODE* the_next = rover->next;
    if (ElementNameIs(rover, "mo") && (ContentIs(rover, "&#x2062;") || (ContentIs(rover, "&#x2061;")))){

       DelinkTNode(rover);
       DisposeTNode(rover);

    } else if (rover->first_kid) {  // rover is a schemata

        RemoveIT_and_AF(rover->first_kid);

    }
    rover = the_next;
  }  
}

MNODE* Tree2StdMML::RemoveMatrixDelims(MNODE* MML_list,
                                       INPUT_NOTATION_REC* in_notation)
{
  MNODE* rv = MML_list;
  MNODE* rover = MML_list;
  while (rover) {
    MNODE* the_next = rover->next;
    if (ElementNameIs(rover, "mfenced")) {
      bool is_parens = false;
      bool is_brackets = false;
      bool is_braces = false;

      bool do_it = false;
      ATTRIB_REC *a_list = rover->attrib_list;
      const char *open_val = GetATTRIBvalue(a_list, "open");
      const char *close_val = GetATTRIBvalue(a_list, "close");
      if (open_val && close_val) {
        if (open_val[0] == '[' && close_val[0] == ']') {
          do_it = true;
          is_brackets = true;
        } else if (open_val[0] == '(' && close_val[0] == ')') {
          do_it = true;
          is_parens = true;
        } else if (open_val[0] == '{' && close_val[0] == '}') {
          do_it = true;
          is_braces = true;
        }
      } else {
        do_it = true;
      }

      MNODE* eldest = rover->first_kid;
      if (do_it &&
          eldest && !eldest->next && ElementNameIs(eldest, "mtable")) {
        if (in_notation){
          if (is_brackets)
            in_notation->nbracket_tables++;
          else if (is_parens)
            in_notation->nparen_tables++;
          else if (is_braces)
            in_notation->nbrace_tables++;
        }

        MNODE* parent = rover->parent;
        MNODE* left_anchor = rover->prev;
        MNODE* right_anchor = rover->next;

        if (rover == MML_list)
          rv = eldest;

        rover->first_kid = NULL;
        DelinkTNode(rover);
        DisposeTNode(rover);

        eldest->parent = parent;
        if (left_anchor) {
          left_anchor->next = eldest;
          eldest->prev = left_anchor;
        } else {
          parent->first_kid = eldest;
        }
        if (right_anchor) {
          right_anchor->prev = eldest;
          eldest->next = right_anchor;
        }
        the_next = eldest;
      } else {
        RemoveMatrixDelims(rover->first_kid, in_notation);
      }
    } else if (rover->first_kid) {  // rover is a schemata
      RemoveMatrixDelims(rover->first_kid, in_notation);
    }
    rover = the_next;
  }

  return rv;
}



MNODE* Tree2StdMML::PermuteTreeToMFENCED(MNODE* opening_mo, GROUP_INFO& gi)
{
  MNODE *mfenced = opening_mo;

  SetElementName(mfenced, "mfenced");
  SetContent(mfenced, NULL);

  if (mfenced->attrib_list) {
    DisposeAttribs(mfenced->attrib_list);
    mfenced->attrib_list = NULL;
  }

  MNODE* body = opening_mo->next;
  int n_interior_nodes = 0;
  MNODE* closer = body;
  while (closer) {
    if (ElementNameIs(closer, "mo")) {
      if (closer->p_chdata) {
        if (ContentIs(closer, gi.closing_delim))
          break;
      } else {
        if (StringEqual("I", gi.closing_delim))
          break;
      }
    }
    n_interior_nodes++;
    closer = closer->next;
  }

  if (closer) {
    mfenced->next = closer->next;
    if (closer->next)
      closer->next->prev = mfenced;

    U32 end_offset = closer->src_start_offset + closer->src_length;
    mfenced->src_length = end_offset - mfenced->src_start_offset;

    DelinkTNode(closer);
    DisposeTNode(closer);

    if (n_interior_nodes) {
      MNODE *item_list = ExtractItems(body, n_interior_nodes,
                                      gi.separator_count, mfenced);
      mfenced->first_kid = item_list;
      if (mfenced->first_kid)
        mfenced->first_kid->prev = NULL;
    }

    ATTRIB_REC* ar_open = new ATTRIB_REC("open", gi.opening_delim);
    ATTRIB_REC* ar_close = new ATTRIB_REC("close", gi.closing_delim);
    InsertAttribute(ar_open, ar_close);
    //ar_open->next = ar_close;
    //ar_close->prev = ar_open;
    mfenced->attrib_list = ar_open;
  } else {
    TCI_ASSERT(0);
  }
  return mfenced;
}

/*				  OR
<mo>(</mo>
<mrow>							<mo>(</mo>
  a								a
  ,								,
  b								b
</mrow>							<mo>)</mo>
<mo>)</mo>
*/

MNODE* Tree2StdMML::ExtractItems(MNODE* body,
                                 int n_interior_nodes,
                                 int n_commas, 
                                 MNODE* parent_mfenced)
{
  MNODE* rv = NULL;

  if (body && n_commas) {
    MNODE* new_list = NULL;
    MNODE* new_tail;
    MNODE* old_list = body;

    // strip away possible enclosing <mrow>
    if (!body->next && ElementNameIs(body, "mrow")) {
      old_list = body->first_kid;
      body->first_kid = NULL;
      DelinkTNode(body);
      DisposeTNode(body);
    }
    // Loop thru list entries, removing commas separators, etc.
    MNODE* rover = old_list;
    while (rover) {
      MNODE* l_anchor = rover;
      MNODE* r_anchor = rover;

      while (r_anchor) {
        if (ElementNameIs(r_anchor, "mo") && ContentIs(r_anchor, ","))
          break;
        else
          r_anchor = r_anchor->next;
      }
      
      if (r_anchor)
        rover = r_anchor->next;
      else
        rover = NULL;

      MNODE* item = MakeItem(l_anchor, r_anchor);
      item->parent = parent_mfenced;
      if (new_list) {
        new_tail->next = item;
        item->prev = new_tail;
      } else {
        new_list = item;
      }
      new_tail = item;
    }
    rv = new_list;
  } else if (body) {
    rv = body;
    // reparent body
    for (MNODE* rover = body; rover; rover = rover->next) {
      rover->parent = parent_mfenced;
    }
  }
  return rv;
}



MNODE* Tree2StdMML::MakeItem(MNODE* l_anchor, MNODE* r_anchor)
{
  MNODE* rv = l_anchor;

  if (l_anchor == r_anchor) {
      rv = MakeTNode(l_anchor->src_start_offset, 0, l_anchor->src_linenum);
      SetElementName(rv, "mrow");
  } else {
      if (r_anchor) {             // , - remove
        if (r_anchor->prev)
          r_anchor->prev->next = NULL;
        if (r_anchor->next)
          r_anchor->next->prev = NULL;
        r_anchor->prev = NULL;
        r_anchor->next = NULL;
        DisposeTNode(r_anchor);
      }

      if (l_anchor->next) {
          rv = MakeTNode(l_anchor->src_start_offset, l_anchor->src_length, l_anchor->src_linenum);
          SetElementName(rv, "mrow");
          rv->first_kid = l_anchor;
        
          while (l_anchor) {
            l_anchor->parent = rv;
            l_anchor = l_anchor->next;
          }

      } else {
          l_anchor->prev = NULL;
          l_anchor->next = NULL;
          rv = l_anchor;
      }
  }
  return rv;
}

// Put stuff between l_anchor and r_anchor inclusive into an mrow
MNODE* Tree2StdMML::MakeMROW(MNODE* l_anchor, MNODE* r_anchor)
{
  MNODE* rover = l_anchor;
  while (rover && rover != r_anchor)
    rover = rover->next;

  if (!rover) {
    TCI_ASSERT(!"Anchors are not siblings!");
    return NULL;
  }

  bool inMrow = ElementNameIs(l_anchor->parent, "mrow")
                || HasInferedMROW(l_anchor->parent);
  
  if (inMrow && l_anchor->prev == NULL && r_anchor->next == NULL)
    return l_anchor;  // already is in mrow
  
  MNODE* rv = MakeTNode(l_anchor->src_start_offset, l_anchor->src_length, l_anchor->src_linenum);
  SetElementName(rv, "mrow");

  rv->first_kid = l_anchor;
  rv->parent = l_anchor->parent; // sew together LHS of mrow
  if (l_anchor->parent && (l_anchor->parent->first_kid == l_anchor))
    l_anchor->parent->first_kid = rv;
  if (l_anchor->prev)
    l_anchor->prev->next = rv;
  rv->prev = l_anchor->prev;
  l_anchor->prev = NULL;

  rover = l_anchor;
  while (rover && rover != r_anchor) {
    rover->parent = rv;
    rover = rover->next;
  }
  r_anchor->parent = rv;

  rv->next = r_anchor->next;  // sew together RHS of mrow
  if (r_anchor->next)
    r_anchor->next->prev = rv;
  r_anchor->next = NULL;

  return rv;
}

// Like above, but r_anchor has a script
MNODE* Tree2StdMML::MakeSCRIPT(MNODE* l_anchor, MNODE* r_anchor)
{
  MNODE* rover = l_anchor;
  while (rover && rover != r_anchor)
    rover = rover->next;

  if (!rover) {
    TCI_ASSERT(!"Anchors are not siblings!");
    return NULL;
  }
  
  MNODE* rv = MakeTNode(l_anchor->src_start_offset, l_anchor->src_length, l_anchor->src_linenum);
  TCI_ASSERT(r_anchor->src_tok);
  SetElementName(rv, r_anchor->src_tok);  // msup, msub or msubsup

  MNODE* base = MakeTNode(l_anchor->src_start_offset, l_anchor->src_length, l_anchor->src_linenum);
  SetElementName(base, "mrow");

  base->first_kid = l_anchor;
  base->parent = rv;
  rv->parent = l_anchor->parent; // sew together LHS of msup
  if (l_anchor->parent && (l_anchor->parent->first_kid == l_anchor))
    l_anchor->parent->first_kid = rv;
  if (l_anchor->prev)
    l_anchor->prev->next = rv;
  rv->prev = l_anchor->prev;
  l_anchor->prev = NULL;

  rover = l_anchor;
  while (rover && rover != r_anchor) {
    rover->parent = base;
    rover = rover->next;
  }

  rv->next = r_anchor->next;  // sew together RHS of msup
  if (r_anchor->next)
    r_anchor->next->prev = rv;

  // recall, r_anchor is an msup
  MNODE* oldbase = r_anchor->first_kid;
  MNODE* oldscript = oldbase->next;
  
  oldbase->parent = base;
  oldbase->prev = r_anchor->prev;
  oldbase->next = NULL;
  r_anchor->prev->next = oldbase;

  rv->first_kid = base;
  base->next = oldscript;
  oldscript->prev = base;
  oldscript->parent = rv;
  if (oldscript->next)
    oldscript->next->parent = rv;

  r_anchor->first_kid = NULL;
  DisposeTNode(r_anchor);

  return rv;
}

bool Tree2StdMML::IsAllDigits(MNODE* num, int& n_digits)
{
  bool rv = true;
  n_digits = 0;
  if (num && num->p_chdata) {
    const char *ptr = num->p_chdata;
    while (*ptr) {
      if (*ptr < '0' || *ptr > '9') {
        rv = false;
        break;
      }
      n_digits++;
      ptr++;
    }
  } else {
    rv = false;
  }
  return rv;
}



bool Tree2StdMML::IsNumberOrPeriod(MNODE* num)
{
  const char* p = num->p_chdata;
  if (ElementNameIs(num, "mn") )
    return p != 0;
  else if (ElementNameIs(num, "mo") ) {
    return p != 0 && StringEqual(p,".");
  } else {
    return false;
  }
}

bool Tree2StdMML::HasPeriod(MNODE* num)
{
  TCI_ASSERT(IsNumberOrPeriod(num));
  return num->p_chdata != 0 && strstr(num->p_chdata,".");
}



void Tree2StdMML::PermuteMixedNumber(MNODE* num)
{
  U32 whole_num = ASCII2U32(num->p_chdata, 10);
  MNODE* frac = num->next;
  MNODE* numerator = frac->first_kid;
  MNODE* denominator = numerator->next;
  U32 numerator_num = ASCII2U32(numerator->p_chdata, 10);
  U32 denominator_num = ASCII2U32(denominator->p_chdata, 10);
  U32 new_numerator = whole_num * denominator_num + numerator_num;

  MNODE* right_anchor = frac->next;

  num->next = right_anchor;
  if (right_anchor)
    right_anchor->prev = num;
  frac->prev = NULL;
  frac->next = NULL;

  TCI_ASSERT( num->first_kid == NULL );

  num->first_kid = frac->first_kid;
  frac->first_kid = NULL;
  // reparent
  MNODE* rover = num->first_kid;
  while (rover) {
    rover->parent = num;
    rover = rover->next;
  }

  num->src_length += frac->src_length;
  SetElementName(num, "mfrac");
  SetContent(num, NULL);

  DisposeTNode(frac);

  char buffer[32];
  sprintf(buffer, "%lu", new_numerator);

  SetContent(numerator, buffer);
}


// These should move to attriblist.
ATTRIB_REC* Tree2StdMML::StackAttr(ATTRIB_REC* attr_stack,
                                   const char* attr_nom, 
                                   const char* attr_val)
{
  attr_stack = RemoveAttr(attr_stack, attr_nom);
  ATTRIB_REC* a_new = new ATTRIB_REC(attr_nom, attr_val);
  InsertAttribute(a_new, attr_stack);
  //  a_new->next = attr_stack;
 //   if (attr_stack)
 //     attr_stack->prev = a_new;

  return a_new;
}



ATTRIB_REC* Tree2StdMML::UnstackAttr(ATTRIB_REC* attr_stack)
{
  ATTRIB_REC* rv = NULL;

  if (attr_stack) {
    ATTRIB_REC* del = attr_stack;
    rv = attr_stack->GetNext();
    del->SetNext(NULL);
    DisposeAttribs(del);
  }

  return rv;
}



void Tree2StdMML::MoveAttrsToChildren(MNODE* mml_list)
{
  MNODE* rover = mml_list;
  while (rover) {
    TCI_ASSERT(CheckLinks(rover));
    MNODE* the_next = rover->next;

    if (ElementNameIs(rover, "mstyle")) {

        RemoveMSTYLEnode(rover);

    } else if (ElementNameIs(rover, "mi")) {

        if (mv_stack)
          InstallStackedAttr(rover, mv_stack);

    } else if (ElementNameIs(rover, "mfrac")) {

        if (lt_stack)
          InstallStackedAttr(rover, lt_stack);

        MNODE* num = rover->first_kid;
        if (num) {
          RemoveMSTYLEs(num);
          MNODE* den = num->next;
          if (den)
            RemoveMSTYLEs(den);
        }

    } else if (rover->first_kid) {

        RemoveMSTYLEs(rover->first_kid);
    }
    rover = the_next;
  }
}



void Tree2StdMML::RemoveMSTYLEnode(MNODE* mstyle)
{
  TCI_ASSERT(CheckLinks(mstyle));

  const char* mv_val = GetATTRIBvalue(mstyle->attrib_list, "mathvariant");
  const char* lt_val = GetATTRIBvalue(mstyle->attrib_list, "linethickness");
  const char* fw_val = GetATTRIBvalue(mstyle->attrib_list, "fontweight");

  if (mv_val)  
    mv_stack = StackAttr(mv_stack, "mathvariant", mv_val);

  if (lt_val)  
    lt_stack = StackAttr(lt_stack, "linethickness", lt_val);

  if (fw_val && StringEqual(fw_val, "bold"))  
    mv_stack = StackAttr(mv_stack, "mathvariant", "Bold");

  //  If we don't descend into children here, nested mstyles
  //    will not be processed.
  
  if (mstyle->first_kid){
    MoveAttrsToChildren(mstyle->first_kid);
  }

  if (mv_val)
    mv_stack = UnstackAttr(mv_stack);

  if (lt_val)
    lt_stack = UnstackAttr(lt_stack);

  if (fw_val && StringEqual(fw_val, "bold"))
    mv_stack = UnstackAttr(mv_stack);
  
  MNODE* eldest = mstyle->first_kid;
  MNODE* parent = mstyle->parent;
  MNODE* left_anchor = mstyle->prev;
  MNODE* right_anchor = mstyle->next;

  MNODE* right_end = eldest;
  if (right_end) {
    while (right_end->next)
      right_end = right_end->next;
  }
  
  mstyle->first_kid = NULL;
  
  DelinkTNode(mstyle);
  DisposeTNode(mstyle);

  MNODE* p = eldest;
  while (p){
      p->parent = parent;
      p = p->next;
  }


  if (left_anchor) {
    left_anchor->next = eldest;
    if (eldest)
      eldest->prev = left_anchor;
  } else {
    parent->first_kid = eldest;
  }

  if (right_anchor) {
    right_anchor->prev = right_end;
    if (eldest)
      right_end->next = right_anchor;
  }

  TCI_ASSERT(CheckLinks(parent));
}



void Tree2StdMML::InstallStackedAttr(MNODE * mml_node,
                                     ATTRIB_REC * attr_stack)
{
  char* targ_attr = attr_stack->zattr_nom;
//  char *targ_aval = attr_stack->zattr_val;

  bool done = false;
  ATTRIB_REC* arover = mml_node->attrib_list;
  
  while (arover) {
    if (!StringEqual(arover->zattr_nom, targ_attr)) {
      arover = arover->GetNext();
    } else {
      done = true;
      break;
    }
  }

  if (!done) {
    ATTRIB_REC* a_new = new ATTRIB_REC(attr_stack->zattr_nom, attr_stack->zattr_val);
    a_new->SetNext(mml_node->attrib_list);
    if (mml_node->attrib_list)
      mml_node->attrib_list->SetPrev(a_new);
    mml_node->attrib_list = a_new;
  }

}



void Tree2StdMML::InsertAF(MNODE* func)
{
  MNODE* af = MakeTNode(func->src_start_offset, 0, func->src_linenum);
  SetElementName(af, "mo");
  SetContent(af, "&#x2061;");  // ApplyFunction

  af->prev = func;
  af->next = func->next;
  func->next = af;
  if (af->next)
    af->next->prev = af;
  af->parent = func->parent;
}


          
void Tree2StdMML::InsertIT(MNODE* term)
{
  MNODE* it = MakeTNode(term->src_start_offset, 0, term->src_linenum);
  SetElementName(it, "mo");
  SetContent(it, "&#x2062;");  // InvisibleTimes

  it->prev = term;
  it->next = term->next;
  term->next = it;
  if (it->next)
    it->next->prev = it;
  it->parent = term->parent;
}


          
// deg/min/sec are additive units (sort of)
void Tree2StdMML::InsertInvisibleAddSigns(MNODE* dMML_list)
{
  MNODE* rover = dMML_list;
  while (rover) {
    if (rover->first_kid) {     // rover is a schemata
      if (ElementNameIs(rover, "mrow")) {

        if (NeedsInvisiblePlus(rover)) {
          MNODE* l_anchor = rover;
          MNODE* r_anchor = rover->next;
          MNODE* plus = MakeTNode(r_anchor->src_start_offset, 0, r_anchor->src_linenum);
          
          SetElementName(plus, "mo");
          SetContent(plus, "+");

          plus->prev = l_anchor;
          plus->next = r_anchor;

          l_anchor->next = plus;
          r_anchor->prev = plus;

          if (l_anchor->parent)
            plus->parent = l_anchor->parent;
        } else {
          InsertInvisibleAddSigns(rover->first_kid);
        }

      } else if (HasPositionalChildren(rover)) {
        MNODE* child_list = rover->first_kid;
        // recurse into children
        InsertInvisibleAddSigns(child_list);
        child_list = child_list->next;
        if (child_list) {
          InsertInvisibleAddSigns(child_list);
          if (ElementNameIs(rover, "msubsup") || ElementNameIs(rover, "munderover")) {
            child_list = child_list->next;
            if (child_list)
              InsertInvisibleAddSigns(child_list);
          }
        }
      } else {
        InsertInvisibleAddSigns(rover->first_kid);
      }
    }
    rover = rover->next;
  }
}




bool Tree2StdMML::NeedsInvisiblePlus(MNODE* dMML_mrow)
{
  bool rv = false;

  if (dMML_mrow && dMML_mrow->next) {
    char* t1 = NULL;
    char* t2 = NULL;
    MNODE* c1 = NULL;
    MNODE* c2 = NULL;

    if (ElementNameIs(dMML_mrow, "mrow")) {
      c1 = dMML_mrow->first_kid;
      if (c1)
        c1 = c1->next;
      if (c1)
        c1 = c1->next;
      t1 = "mi";
    } else if (ElementNameIs(dMML_mrow, "msup")) {
      c1 = dMML_mrow->first_kid;
      if (c1)
        c1 = c1->next;
      t1 = "mo";
    }
    if (ElementNameIs(dMML_mrow->next, "mrow")) {
      c2 = dMML_mrow->next->first_kid;
      if (c2)
        c2 = c2->next;
      if (c2)
        c2 = c2->next;
      t2 = "mi";
    } else if (ElementNameIs(dMML_mrow->next, "msup")) {
      c2 = dMML_mrow->next->first_kid;
      if (c2)
        c2 = c2->next;
      if (ElementNameIs(c2, "mrow")) {
        c2 = c2 -> first_kid;
      }
      t2 = "mo";
    }
    //if (c1 && c2 && ElementNameIs(c1, t1) && ElementNameIs(c2, t2)) {

      if (ContentIs(c1, "&#xb0;") || ContentIs(c1, "&#x2218;")) {  // degree
        if (ContentIs(c2, "&#x2032;") || // minute
            ContentIs(c2, "&#x2033;"))   // second
          rv = true;
      } else if (ContentIs(c1, "&#x2032;")) {
        if (ContentIs(c2, "&#x2033;") || ContentIs(c2, "&#x2032;"))
          rv = true;
      }
    //}
  }
  return rv;
}
