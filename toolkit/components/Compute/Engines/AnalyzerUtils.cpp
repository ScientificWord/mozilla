
#include "Analyzer.h"
#include "AnalyzerData.h"
#include "DefStore.h"
#include "DefInfo.h"
#include "mnode.h"
#include "strutils.h"
#include "Grammar.h"





void SetSnodeOwner(SEMANTICS_NODE* snode, AnalyzerData* pData)
{
  if (snode) {
    
    DefInfo* di = pData ->GetDI (snode->canonical_ID);

    if (di)
      snode->owner_ID = di->owner_ID;
    else
      snode->owner_ID = pData -> CurrClientID();
  } else {
    TCI_ASSERT(0);
  }
}

BaseType GetBaseType(MNODE* mml_script_schemata, bool isLHSofDef, AnalyzerData* pData, const Grammar* mml_entities)
{
  BaseType rv = BT_UNKNOWN;

  MNODE* base = mml_script_schemata->first_kid;

  if (ElementNameIs(base, "mi")) {
      if (ContentIs(base, "&#x2112;"))  // Laplace
        rv = BT_TRANSFORM;
      if (ContentIs(base, "&#x2131;"))  // Fourier
        rv = BT_TRANSFORM;
      if (ContentIs(base, "seq")) // sequence
        rv = BT_OPERATOR;
      if (ContentIs(base, "lim")) // varinjlim
        rv = BT_OPERATOR;
      if (ContentIs(base, "max")) 
        rv = BT_OPERATOR;
      if (ContentIs(base, "min")) 
        rv = BT_OPERATOR;

      if (rv == BT_UNKNOWN && !isLHSofDef) {

          if (NodeIsFunction(base, mml_entities, pData)) {
            
             char* mi_canonical_str = GetCanonicalIDforMathNode(base, pData -> GetGrammar());

             DefInfo* di = pData ->GetDI (mi_canonical_str);
             if (di && (di->def_type == DT_FUNCTION || di->def_type == DT_MUPNAME)) {
               if (di->n_subscripted_args)
                  rv = BT_SUBARG_FUNCTION;
               else
                  rv = BT_FUNCTION;
             } else {
                  rv = BT_FUNCTION; 
             }
             delete[] mi_canonical_str;
          } else 
             rv = BT_VARIABLE;
      }
  }

  if (rv == BT_UNKNOWN && mml_script_schemata->next) {
      MNODE* nn = mml_script_schemata->next;
      const char* next_elem_nom = nn->src_tok;
      if (StringEqual(next_elem_nom, "mo")) {
        const char *ptr = strstr(nn->p_chdata, "&#x");
        if (ptr) {
          U32 unicode = ASCII2U32(ptr + 3, 16);
          if (unicode == 0x2061) {  //&ApplyFunction;
            rv = BT_FUNCTION;
          }
        }
      }
  }

  if (rv == BT_UNKNOWN) {
      if (ElementNameIs(base, "mo")) {
          rv = BT_OPERATOR;
      } else if (ElementNameIs(base, "mn")) {
          rv = BT_NUMBER;
      } else if (ElementNameIs(base, "mfenced")) {
          rv = BT_FENCED;
      } else if (ElementNameIs(base, "mtable")) {
          rv = BT_MATRIX;
      } else if (ElementNameIs(base, "mover")) {
          rv = BT_MOVER;
      } else if (ElementNameIs(base, "msub")) {
          rv = BT_SUB;
      } else if (ElementNameIs(base, "msup")) {
          rv = BT_SUP;
      } else if (ElementNameIs(base, "mi")) {
          char zunit[256];
          zunit[0] = 0;
          GetCurrAttribValue(base, false, "msiunit", zunit, 256);
          if (StringEqual(zunit, "true")) {
            rv = BT_UNIT;
          } else {
              //int entity_count;
              //int symbol_count = CountSymbols(base->p_chdata, entity_count);
              //if (symbol_count == 1) {
                rv = BT_VARIABLE;
                if (LocateFuncRec(pData -> DE_FuncNames(), NULL, base->p_chdata))
                  rv = BT_FUNCTION;
                else if (LocateFuncRec(pData -> IMPLDIFF_FuncNames(), NULL, base->p_chdata))
                  rv = BT_FUNCTION;
              //} else
              //  rv = BT_FUNCTION;

          }
      } else if (ElementNameIs(base, "mrow")) {
          rv = BT_ROW;
      } else {
          TCI_ASSERT(0);
      }
  }

  return rv;
}



// Locate an DE_FUNC_REC by the function name it holds.
bool  LocateFuncRec(const DE_FUNC_REC* f_list,
                    const char* canon_name,
                    const char* src_name)
{
  const DE_FUNC_REC* rover = f_list;
  while (rover) {
    
    if (canon_name && rover->zfunc_canon_name &&
        StringEqual(rover->zfunc_canon_name, canon_name))
      break;

    if (src_name && rover->zfunc_src_name &&
        StringEqual(rover->zfunc_src_name, src_name))
      break;

    rover = rover->next;
  }

  return (rover != NULL);
}

// Append a function record to DE_func_names, provided
//  that the function is not already in the list.
//  Note that these are mml canonical names here - miy, etc.

DE_FUNC_REC* AppendFuncName(DE_FUNC_REC* f_list, 
                            char* f_name,
                            char* src_name)
{
  DE_FUNC_REC* rv = f_list;

  if (!LocateFuncRec(f_list, f_name, src_name)) {
    DE_FUNC_REC* new_rec = new DE_FUNC_REC();
    new_rec->next = const_cast<DE_FUNC_REC*> (f_list);
    new_rec->zfunc_canon_name = f_name;
    new_rec->zfunc_src_name = src_name;
    rv = new_rec;
  } else {
    delete[] src_name;
    delete[] f_name;
  }

  return rv;
}



// define ET_POWER                 1
// ..
// define ET_CONJUGATE_INDICATOR   5

ExpType GetExpType(BaseType base_type, MNODE* exp, const Grammar* mml_entities)
{
  ExpType rv = ET_POWER;

  MNODE* expOrig = exp;

  if (ElementNameIs(exp, "mrow")) {
	   exp = exp -> first_kid;
  }

  if (ElementNameIs(exp, "mi")) {
    if (base_type == BT_MATRIX) {
      if (ContentIs(exp, "T"))
        rv = ET_TRANSPOSE_INDICATOR;
      else if (ContentIs(exp, "H"))
        rv = ET_HTRANSPOSE_INDICATOR;
    } else if (ContentIs(exp, "&#x2032;") || ContentIs(exp, "&#x2033;") )
        rv = ET_PRIMES;
    
  } else if (ElementNameIs(exp, "mn")) {
  } else if (ElementNameIs(exp, "mo")) {
    const char* exp_contents = exp->p_chdata;
    if (exp_contents) {
      int base = 16;
      int off = 3;
      const char* ptr = strstr(exp_contents, "&#x");
      if (!ptr) {
        ptr = strstr(exp_contents, "&#");
        if (ptr) {
          base = 10;
          off = 2;
        }
      }
      if (ptr) {
        U32 unicode = ASCII2U32(ptr + off, base);
        if (unicode == 0x2217   // &midast;
            || unicode == 0x2a) // &ast;
          rv = ET_CONJUGATE_INDICATOR;
        else if (unicode == 0x2032) // &prime;
          rv = ET_PRIMES;
        else if (unicode == 0x2212) // &minus;
          rv = ET_DIRECTION;
        else if (unicode == 0x2218) // &minus;
          rv = ET_DEGREE;
        else
          TCI_ASSERT(!"Operator not in list.");
      } else if (exp_contents[0] == '*' && exp_contents[1] == 0) {
        rv = ET_CONJUGATE_INDICATOR;
      } else if (exp_contents[0] == '+' && exp_contents[1] == 0) {
        rv = ET_DIRECTION;
      } else if (exp_contents[0] == '-' && exp_contents[1] == 0) {
        rv = ET_DIRECTION;
      } else {
        TCI_ASSERT(0);
      }
    } else {
      TCI_ASSERT(!"operator with no contents????");
    }
  } else if (ElementNameIs(exp, "mfenced")) {
    bool inherit = false;

    char zopen_attr_val[32];
    zopen_attr_val[0] = 0;
    GetCurrAttribValue(exp, inherit, "open", zopen_attr_val, 32);

    char zclose_attr_val[32];
    zclose_attr_val[0] = 0;
    GetCurrAttribValue(exp, inherit, "close", zclose_attr_val, 32);

    if (zopen_attr_val[0] == '(' && zclose_attr_val[0] == ')') {
      if (base_type == BT_FUNCTION || base_type == BT_SUBARG_FUNCTION)
        rv = ET_PARENED_PRIMES_COUNT;
    }
  }
  if (rv == ET_DIRECTION && exp->first_kid && exp->first_kid->next)
    rv = ET_POWER;  // -x or +x

  if (IsInverseIndicator(expOrig, mml_entities))
    if (base_type == BT_FUNCTION || 
        base_type == BT_SUBARG_FUNCTION || 
        base_type == BT_TRANSFORM)
      rv = ET_INVERSE_INDICATOR;

  return rv;
}

//SLS seems like a risky heuristic...
ExpType GetSubScriptType(MNODE* script_schemata,
                         BaseType base_type, 
                         MNODE* sub) 
{
  ExpType rv = ET_DECORATION;

  //const char *sub_element = sub->src_tok;
  if (ElementNameIs(sub, "mn")) {
    rv = ET_NUMBER;
  } else if (ElementNameIs(sub, "mrow")) {
    rv = ET_EXPRESSION;
  } else if (ElementNameIs(sub, "msub")) {
    rv = ET_EXPRESSION;
  } else if (ElementNameIs(sub, "msup")) {
    rv = ET_EXPRESSION;
  } else if (ElementNameIs(sub, "mi")) {
    rv = ET_VARIABLE;

    if (base_type == BT_FUNCTION && script_schemata && script_schemata->next) {
      if (IsApplyFunction(script_schemata->next))
        rv = ET_DECORATION;
    }
  } else {
    TCI_ASSERT(0);
  }
  return rv;
}

// Just enough done to test the "true" return route.

bool IsInverseIndicator(MNODE* mml_exp_node, const Grammar* mml_entities)
{

  bool rv = false;
  /*
  <mml:msup>
    <mml:mi mathcolor="gray">sin</mml:mi>
    <mml:mrow>
      <mml:mo form="prefix">&minus;</mml:mo>   &minus;<uID3.13.61>multiform,111,U02212
      <mml:mn>1</mml:mn>
    </mml:mrow>
  </mml:msup>
  <mml:mo>&ApplyFunction;</mml:mo>
  <mml:mi>x</mml:mi>
  */
  if (ElementNameIs(mml_exp_node, "mrow")) {
    if (mml_exp_node->first_kid) {
      MNODE *rover = mml_exp_node->first_kid;
      if (ElementNameIs(rover, "mo")) {
        U32 unicodes[8];
        int content_tally = ChData2Unicodes(rover->p_chdata, unicodes, 8, mml_entities);
        if (content_tally == 1 &&
            (unicodes[0] == '-' || unicodes[0] == 0x2212)) {
          rover = rover->next;
          if (rover) {
            if (ElementNameIs(rover, "mn")) {
              U32 unicodes[8];
              int content_tally =
                ChData2Unicodes(rover->p_chdata, unicodes, 8, mml_entities);
              if (content_tally == 1 && unicodes[0] == '1')
                rv = true;
            }
          }
        }
      }
    }
  }

  return rv;
}


void OverridePrefsOnLHS(MNODE* dMML_tree, AnalyzerData* pData)
{
  if (dMML_tree) {
    
    MNODE* m_rover = dMML_tree;
    
    if (ElementNameIs(dMML_tree, "math"))
      m_rover = dMML_tree->first_kid;

    if (ElementNameIs(m_rover, "mrow") && !m_rover->next)
      m_rover = m_rover->first_kid;

    if (ElementNameIs(m_rover, "mi") && m_rover->next && ElementNameIs(m_rover->next, "mo")) {
      
      const char* src_token = m_rover->p_chdata;
      if (StringEqual(src_token, "i"))
        pData -> Set_i_is_imaginary( false );
      else if (StringEqual(src_token, "j"))
        pData -> Set_j_is_imaginary( false );
      else if (StringEqual(src_token, "e"))
        pData -> Set_e_is_Euler( false );
    }
  }
}



// Sometimes, Fixup will introduce InvisibleTimes when what was really meant is ApplyFunction.
// (But Fixup doesn't know the context so it actually did the right thing.)
void OverrideInvisibleTimesOnLHS(MNODE * dMML_tree)
{
  if (dMML_tree) {

    MNODE* m_rover = dMML_tree;

    if (ElementNameIs(dMML_tree, "math"))
      m_rover = dMML_tree->first_kid;

    if (ElementNameIs(m_rover, "mrow") && !m_rover->next)
      m_rover = m_rover->first_kid;

    if (ElementNameIs(m_rover, "mrow"))
      m_rover = m_rover->first_kid;

    //TODO deal with embellished functions (non-SWP feature)

    if ( (ElementNameIs(m_rover, "mi") || (ElementNameIs(m_rover, "msub") && 
                                           ElementNameIs(m_rover->first_kid, "mi") )) && 
           ElementNameIs(m_rover->next, "mo") ) {
      const char* src_token = m_rover->next->p_chdata;
      if (StringEqual(src_token, "&#x2062;") && m_rover->next->next) {
        
        MNODE* fence_or_mrow = m_rover->next->next;
        while (ElementNameIs(fence_or_mrow, "mrow")){
          fence_or_mrow = fence_or_mrow -> first_kid;
        }
          
        if (IsArgDelimitingFence(fence_or_mrow)) {
          //super ugly, but what else to do?
          delete m_rover->next->p_chdata;
          m_rover->next->p_chdata = DuplicateString("&#x2061;"); // ApplyFunction
        }
      }
    }
  }
}



IdentIlk GetMIilk(char* mi_canonical_str, 
                  DefInfo* di, 
                  MNODE* m_node, 
                  bool isLHSofDef, 
                  const Grammar* mml_entities,
                  bool i_is_imaginary,
                  bool j_is_imaginary,
                  bool e_is_Euler)
{
    IdentIlk rv = MI_none;

    if (di) {
      if (isLHSofDef) {
        // Here, we're re-defining a symbol - the type of any previous def is irrelevent
      } else {
        if (di->def_type == DT_FUNCTION || di->def_type == DT_MUPNAME)
          rv = MI_function;
        else if (di->def_type == DT_VARIABLE)
          rv = MI_variable;
        else
          TCI_ASSERT(0);
        return rv;
      }
    }
  

    if (StringEqual(mi_canonical_str, "mi&#x3c0;")) {
      rv = MI_pi;

    } else if (StringEqual(mi_canonical_str, "mii")) {  //  i
        if (i_is_imaginary)
          rv = MI_imaginaryunit;

    } else if (StringEqual(mi_canonical_str, "mi&#x2148;")) { // imaginary i
        rv = MI_imaginaryunit;

    } else if (StringEqual(mi_canonical_str, "mij")) {  //  j
        if (j_is_imaginary)
          rv = MI_imaginaryunit;

    } else if (StringEqual(mi_canonical_str, "mie")) {
      if (e_is_Euler)
        rv = MI_Eulere;

    } else if (StringEqual(mi_canonical_str, "mi&#x2147;")) {
        rv = MI_Eulere;
    } else if (StringEqual(mi_canonical_str, "mi&#x221e;")) {
        rv = MI_infinity;
    } else if (StringEqual(mi_canonical_str, "migamma")) {
        rv = MI_Eulergamma;
    } else if (StringEqual(mi_canonical_str, "mi&#x2112;")) {
        rv = MI_Laplace;
    } else if (StringEqual(mi_canonical_str, "mi&#x2131;")) {
        rv = MI_Fourier;
    }

    if (IdentIsConstant(rv)) {
      if (ElementNameIs(m_node->next, "mo")) {
        U32 unicodes[8];
        int content_tally = ChData2Unicodes(m_node->next->p_chdata, unicodes, 8, mml_entities);
        if (content_tally == 1 && unicodes[0] == 0x2061)
          rv = MI_function;
      }
    }

    return rv;
}



bool IdentIsConstant(IdentIlk ilk)
{
  switch (ilk) {
    case MI_pi:
    case MI_imaginaryunit:
    case MI_Eulere:
    case MI_infinity:
    case MI_Eulergamma:
      return true;
    default:
      return false;
  }
}




int GetVarLimType(char *op_name, MNODE * base)
{
  int rv = 0;

  if (!strcmp(op_name, "lim")
      && base && base->next && base->next->p_chdata) {
    MNODE *under_decoration = base->next;
    const char *decor = under_decoration->p_chdata;
    const char *ptr = strstr(decor, "&#x");
    if (ptr) {
      U32 unicode = ASCII2U32(ptr + 3, 16);
      switch (unicode) {
      case 0x2192:             //&rarr;
        rv = 1;
        break;
      case 0x0332:             //&underbar;
        rv = 2;
        break;
      case 0x2190:             //&larr;
        rv = 3;
        break;
      case 0xaf:               //&overbar;
        rv = 4;
        break;
      default:
        break;
      }
    }
  }

  return rv;
}



IdentIlk GetMSIilk(char* msi_class)
{
  IdentIlk rv = MI_none;

  if (StringEqual(msi_class, "enginefunction")) {
    rv = MI_function;
  } else if (StringEqual(msi_class, "enginevariable")) {
    rv = MI_variable;
  } else {
    TCI_ASSERT(!"Unexpected msi_class value.");
  }
  return rv;
}





bool IsWhiteText(const char *z_text)
{
  bool rv = true;

  if (z_text && *z_text) {
    const char *ptr = z_text;
    char ch;
    while (rv && (ch = *ptr)) {
      if (ch == '&') {
        if (*(ptr + 1) == '#' && *(ptr + 2) == 'x') {
          U32 unicode = ASCII2U32(ptr + 3, 16);
          if (unicode != 0x200B)
            rv = false;
          // need more here!
        } else {
          TCI_ASSERT(0);
        }
        while (*ptr != ';')
          ptr++;
      } else if (ch > ' ' && ch <= '~') {
        rv = false;
      }
      ptr++;
    }                           // loop thru chars
  }

  return rv;
}





BUCKET_REC* AddVarToBucket(U32 bucket_ID,
                           SEMANTICS_NODE* s_var_list, 
                           AnalyzerData* pData)
{
  BUCKET_REC* head = NULL;
  BUCKET_REC* tail;
  char* buffer = NULL;
  U32 buffer_ln = 0;
  SEMANTICS_NODE* sv_rover = s_var_list;

  if (sv_rover->semantic_type == SEM_TYP_MATH_CONTAINER && sv_rover->bucket_list)
    sv_rover = sv_rover->bucket_list->first_child;

  while (sv_rover
         && sv_rover->semantic_type == SEM_TYP_PRECEDENCE_GROUP
         && !sv_rover->next)
    sv_rover = sv_rover->bucket_list->first_child;

  while (sv_rover) {
    SEMANTICS_NODE* s_curr_var = sv_rover;
    if (sv_rover->semantic_type == SEM_TYP_POWERFORM) {
      BUCKET_REC* bucket = FindBucketRec(sv_rover->bucket_list, MB_SCRIPT_BASE);
      s_curr_var = bucket->first_child;
    }

    if (s_curr_var->semantic_type == SEM_TYP_VARIABLE && s_curr_var->contents) {
      
        bool do_it = true;
        if (buffer) {
          char *ptr = strstr(buffer, s_curr_var->contents);
          if (ptr) {
            do_it = false;
          }
        }

        if (do_it) {
          SEMANTICS_NODE* s_var = CreateSemanticsNode(SEM_TYP_VARIABLE);
          //s_var->semantic_type = SEM_TYP_VARIABLE;

          if (s_curr_var->canonical_ID) {
            s_var->canonical_ID = DuplicateString(s_curr_var->canonical_ID);
            SetSnodeOwner(s_var, pData);
          }

          char* tmp = DuplicateString(s_curr_var->contents);
	    	s_var -> contents = tmp;
          buffer = AppendStr2HeapStr(buffer, buffer_ln, tmp);

          BUCKET_REC* fvar_bucket = MakeParentBucketRec(bucket_ID, s_var);
          

          if (!head)
            head = fvar_bucket;
          else
            tail->next = fvar_bucket;
          tail = fvar_bucket;
        }

    } else
        TCI_ASSERT(0);

    sv_rover = sv_rover->next;
  }
  delete[] buffer;

  return head;
}


void ExtractVariables(SEMANTICS_NODE* s_tree, AnalyzerData* pData)
{
  SEMANTICS_NODE* s_rover = s_tree;

  while (s_rover) {

    if (s_rover->semantic_type == SEM_TYP_VARIABLE) {

        if ( StringNonEmpty(s_rover->contents) ){
            pData -> SetIMPLDIFF_FuncNames( AppendFuncName(pData -> IMPLDIFF_FuncNames(), NULL, DuplicateString(s_rover->contents) ) );
        }

    } else if (s_rover->bucket_list) {

        BUCKET_REC* b_rover = s_rover->bucket_list;
        while (b_rover) {
          SEMANTICS_NODE* s_list = b_rover->first_child;
          if (s_list) {
            ExtractVariables(s_list, pData);
          }
          b_rover = b_rover->next;
        }

    }

    s_rover = s_rover->next;
  }

}


char* GetFuncNameFromSubSup(MNODE* msubsup, const char** src_name, AnalyzerData* pData)
{
  char* rv = NULL;

  MNODE* base = msubsup->first_kid;
  MNODE* sub = base->next;
  MNODE* exp = sub->next;
  BaseType bt = GetBaseType(msubsup, false, pData, pData -> GetGrammar());
  ExpType et = GetExpType(bt, exp, pData -> GetGrammar());

  if (et == ET_PRIMES) {
    if (ElementNameIs(base, "mi")) {
      U32 zh_ln = 0;
      rv = AppendStr2HeapStr(rv, zh_ln, "msub");
      char* tmp = GetCanonicalIDforMathNode(base, pData -> GetGrammar());
      *src_name = base->p_chdata;
      if (tmp) {
        rv = AppendStr2HeapStr(rv, zh_ln, tmp);
        delete[] tmp;
      }
      tmp = GetCanonicalIDforMathNode(sub, pData -> GetGrammar());
      if (tmp) {
        rv = AppendStr2HeapStr(rv, zh_ln, tmp);
        delete[] tmp;
      }
    }
  }
  return rv;
}


char* GetFuncNameFromFrac(MNODE* mfrac, const char** src_name, const Grammar* mml_entities)
{
  char* rv = NULL;

  MNODE* m_num_operand;
  MNODE* m_den_var_expr;

  if (IsDIFFOP(mfrac, &m_num_operand, &m_den_var_expr)) {
    if (m_num_operand) {
      if (ElementNameIs(m_num_operand, "mi")) {
        rv = GetCanonicalIDforMathNode(m_num_operand, mml_entities);
        *src_name = m_num_operand->p_chdata;
      }
    }
  }
  return rv;
}

char* GetFuncNameFromSub(MNODE* msub, const char** src_name, const Grammar* mml_entities)
{
  char* rv = NULL;

  if (IsDDIFFOP(msub)) {
    MNODE* m_operand = msub->next;
    if (m_operand) {
      if (ElementNameIs(m_operand, "mi")) {
        rv = GetCanonicalIDforMathNode(m_operand, mml_entities);
        *src_name = m_operand->p_chdata;
      } else if (ElementNameIs(m_operand, "msub")) {
        // D_{x}y_{1}
        MNODE* base = m_operand->first_kid;
        if (ElementNameIs(base, "mi")) {
          rv = GetCanonicalIDforMathNode(m_operand, mml_entities);
          *src_name = base->p_chdata;
        }
      }
    }
  }
  return rv;
}



char* GetFuncNameFromSup(MNODE* msup, const char** src_name, AnalyzerData* pData)
{
  char* rv = NULL;

  MNODE* base = msup->first_kid;
  BaseType bt = GetBaseType(msup, false, pData, pData -> GetGrammar());
  ExpType et = GetExpType(bt, base->next, pData -> GetGrammar());

  if (et == ET_PRIMES) {
    if (ElementNameIs(base, "mi")) {
      rv = GetCanonicalIDforMathNode(base, pData -> GetGrammar());
      *src_name = base->p_chdata;
    }
  }
  return rv;
}


// Traverse an mml tree that represents an ODE, look for constructs
// that represent derivatives.  Record the names of the functions
//  whose derivatives are encountered.

void DetermineODEFuncNames(MNODE* dMML_tree, AnalyzerData* pData)
{
  MNODE* rover = dMML_tree;

  while (rover) {

    char* f_name = NULL;
    const char* src_name = NULL;

    if (ElementNameIs(rover, "mfrac"))

       f_name = GetFuncNameFromFrac(rover, &src_name, pData -> GetGrammar());

    else if (ElementNameIs(rover, "msub"))

      f_name = GetFuncNameFromSub(rover, &src_name, pData -> GetGrammar());

    else if (ElementNameIs(rover, "msup"))

      f_name = GetFuncNameFromSup(rover, &src_name, pData);

    else if (ElementNameIs(rover, "msubsup"))

      f_name = GetFuncNameFromSubSup(rover, &src_name, pData);

    if (f_name) {

      char* new_src_name = NULL;

      if (src_name) {
        new_src_name = DuplicateString(src_name);
      }

      pData -> SetDE_FuncNames( AppendFuncName(pData -> DE_FuncNames(), f_name, new_src_name) );

    }
    rover = rover->next;
  }

  rover = dMML_tree;
  while (rover) {
    if (rover->first_kid)
      DetermineODEFuncNames(rover->first_kid, pData);
    rover = rover->next;
  }
}


// Traverse an mml tree that represents an PDE, look for constructs
// that represent derivatives.  Record the names of the functions
//  whose derivatives are encountered.

void DeterminePDEFuncNames(MNODE* dMML_tree, AnalyzerData* pData)
{
  MNODE* rover = dMML_tree;
  while (rover) {
    char* f_name = NULL;
    const char* src_name = NULL;
    if (ElementNameIs(rover, "mfrac"))
      f_name = GetFuncNameFromFrac(rover, &src_name, pData -> GetGrammar());
    if (f_name) {
      char *new_src_name = NULL;
      if (src_name) {
        new_src_name = DuplicateString(src_name);
      }
      pData -> SetDE_FuncNames( AppendFuncName(pData-> DE_FuncNames(), f_name, new_src_name) );
    }
    rover = rover->next;
  }

  rover = dMML_tree;
  while (rover) {
    if (rover->first_kid)
      DeterminePDEFuncNames(rover->first_kid, pData);
    rover = rover->next;
  }
}



// The following bullshit arises because of the careless
//  use of \limfunc in SWP help documents when \func is needed

PrefixOpIlk GetPrefixOpCode(const char* op_name, SemanticVariant& n_integs, const Grammar* mml_entities)
{
  PrefixOpIlk rv = POI_none;
  n_integs = SNV_None;

  const char* ptr = strchr(op_name, '&');

  if (ptr) {
    U32 unicodes[8];
    int char_tally = ChData2Unicodes(op_name, unicodes, 8, mml_entities);
    if (char_tally && unicodes[0] == 0x2207) {  // nabla
      if (char_tally == 1) {
        rv = POI_gradient;

      } else if (char_tally == 2) {
        if (unicodes[1] == 0x22C5)
          rv = POI_divergence;
        else if (unicodes[1] == 0xD7)
          rv = POI_curl;
      } else if (char_tally == 3) {
        if (unicodes[1] == 0x22C5 && unicodes[2] == 0x2207)
          rv = POI_Laplacian;
      }
    } else if (char_tally == 1) {
      SemanticType bo_type = GetBigOpType(op_name, n_integs);
      if (bo_type == SEM_TYP_BIGOP_INTEGRAL)
        rv = POI_integral;
      else if (bo_type == SEM_TYP_BIGOP_SUM)
        rv = POI_sum;
    }

    return rv;
  }

  size_t ln = strlen(op_name);
  switch (ln) {
  case 2:
    if (StringEqual(op_name, "Si"))
      rv = POI_function;
    else if (StringEqual(op_name, "Ci"))
      rv = POI_function;
    else if (StringEqual(op_name, "Ei"))
      rv = POI_function;


    break;
  case 3:
    if (StringEqual(op_name, "gcd"))
      rv = POI_listop;
    else if (StringEqual(op_name, "lcm"))
      rv = POI_listop;
    else if (StringEqual(op_name, "max"))
      rv = POI_listop;
    else if (StringEqual(op_name, "min"))
      rv = POI_listop;
    else if (StringEqual(op_name, "det"))
      rv = POI_det;
    else if (StringEqual(op_name, "div"))
      rv = POI_divergence;
    else if (StringEqual(op_name, "erf"))
      rv = POI_function;
    else if (StringEqual(op_name, "Chi"))
      rv = POI_function;
    else if (StringEqual(op_name, "Shi"))
      rv = POI_function;
    else if (StringEqual(op_name, "Psi"))
      rv = POI_function;

    break;

  case 4:
    if (StringEqual(op_name, "FDen"))
      rv = POI_distribution;
    else if (StringEqual(op_name, "FInv"))
      rv = POI_distribution;
    else if (StringEqual(op_name, "TDen"))
      rv = POI_distribution;
    else if (StringEqual(op_name, "TInv"))
      rv = POI_distribution;
    else if (StringEqual(op_name, "curl"))
      rv = POI_curl;
    else if (StringEqual(op_name, "grad"))
      rv = POI_distribution;
    else if (StringEqual(op_name, "Beta"))
      rv = POI_function;
    else if (StringEqual(op_name, "csgn"))
      rv = POI_function;
    else if (StringEqual(op_name, "erfc"))
      rv = POI_function;

    break;

  case 5:
    if (StringEqual(op_name, "FDist"))
      rv = POI_distribution;
    else if (StringEqual(op_name, "TDist"))
      rv = POI_distribution;
    else if (StringEqual(op_name, "Dirac"))
      rv = POI_Dirac;
    else if (StringEqual(op_name, "dilog"))
      rv = POI_function;
    else if (StringEqual(op_name, "solve"))
      rv = POI_function;


    break;

  case 6:
    if (StringEqual(op_name, "signum"))
      rv = POI_function;
    else if (StringEqual(op_name, "factor"))
      rv = POI_function;
    else if (StringEqual(op_name, "expand"))
      rv = POI_function;

    break;

  case 7:
    if (StringEqual(op_name, "BetaDen"))
      rv = POI_distribution;
    else if (StringEqual(op_name, "BetaInv"))
      rv = POI_distribution;
    else if (StringEqual(op_name, "BesselI"))
      rv = POI_function;
    else if (StringEqual(op_name, "BesselJ"))
      rv = POI_function;
    else if (StringEqual(op_name, "BesselK"))
      rv = POI_function;
    else if (StringEqual(op_name, "BesselY"))
      rv = POI_function;
    
    break;

  case 8:
    if (StringEqual(op_name, "GammaDen"))
      rv = POI_distribution;
    else if (StringEqual(op_name, "GammaInv"))
      rv = POI_distribution;
    else if (StringEqual(op_name, "BetaDist"))
      rv = POI_distribution;
    break;

  case 9:
    if (StringEqual(op_name, "GammaDist"))
      rv = POI_distribution;
    else if (StringEqual(op_name, "CauchyDen"))
      rv = POI_distribution;
    else if (StringEqual(op_name, "CauchyInv"))
      rv = POI_distribution;
    else if (StringEqual(op_name, "NormalDen"))
      rv = POI_distribution;
    else if (StringEqual(op_name, "NormalInv"))
      rv = POI_distribution;
    else if (StringEqual(op_name, "Heaviside"))
      rv = POI_Dirac;
    else if (StringEqual(op_name, "bernoulli"))
      rv = POI_function;

    break;

  case 10:
    if (StringEqual(op_name, "CauchyDist"))
      rv = POI_distribution;
    else if (StringEqual(op_name, "NormalDist"))
      rv = POI_distribution;
    else if (StringEqual(op_name, "PoissonDen"))
      rv = POI_distribution;
    else if (StringEqual(op_name, "PoissonInv"))
      rv = POI_distribution;
    else if (StringEqual(op_name, "UniformDen"))
      rv = POI_distribution;
    else if (StringEqual(op_name, "UniformInv"))
      rv = POI_distribution;
    else if (StringEqual(op_name, "WeibullDen"))
      rv = POI_distribution;
    else if (StringEqual(op_name, "WeibullInv"))
      rv = POI_distribution;
    break;

  case 11:
    if (StringEqual(op_name, "PoissonDist"))
      rv = POI_distribution;
    else if (StringEqual(op_name, "UniformDist"))
      rv = POI_distribution;
    else if (StringEqual(op_name, "WeibullDist"))
      rv = POI_distribution;
    else if (StringEqual(op_name, "BinomialDen"))
      rv = POI_distribution;
    else if (StringEqual(op_name, "BinomialInv"))
      rv = POI_distribution;
    break;

  case 12:
    if (StringEqual(op_name, "BinomialDist"))
      rv = POI_distribution;
    else if (StringEqual(op_name, "ChiSquareDen"))
      rv = POI_distribution;
    else if (StringEqual(op_name, "ChiSquareInv"))
      rv = POI_distribution;
    else if (StringEqual(op_name, "HypergeomDen"))
      rv = POI_distribution;
    else if (StringEqual(op_name, "HypergeomInv"))
      rv = POI_distribution;
    break;

  case 13:
    if (StringEqual(op_name, "ChiSquareDist"))
      rv = POI_distribution;
    else if (StringEqual(op_name, "HypergeomDist"))
      rv = POI_distribution;
    break;

  case 14:
    if (StringEqual(op_name, "ExponentialDen"))
      rv = POI_distribution;
    else if (StringEqual(op_name, "ExponentialInv"))
      rv = POI_distribution;
    break;

  case 15:
    if (StringEqual(op_name, "ExponentialDist"))
      rv = POI_distribution;
    break;

  default:
    break;
  }

  return rv;
}



// Here, snode is of type SEM_TYP_DERIVATIVE.
// Append a bucket of type MB_NPRIMES to snode->bucket_list.

void AddPrimesCount(SEMANTICS_NODE* snode, MNODE* primes)
{
  int n_primes = 0;

  if (ElementNameIs(primes, "mfenced")) {

      MNODE* cont = primes->first_kid;
      if (ElementNameIs(cont, "mn")) {
        n_primes = atoi(cont->p_chdata);
        AppendNumber(snode, MB_NPRIMES, n_primes);
      } else {
        TCI_ASSERT(0);
      }

  } else {

      MNODE* rover = primes;
      if (ElementNameIs(rover, "mrow"))
        rover = rover->first_kid;
      // <mo form="postfix">&#x2032;</mo>
      while (rover) {
        if (ElementNameIs(rover, "mo"))
          n_primes++;
        rover = rover->next;
      }
      AppendNumber(snode, MB_NPRIMES, n_primes);

  }
}




void DisposeODEFuncNames(DE_FUNC_REC * func_names)
{
  DE_FUNC_REC *rover = func_names;
  while (rover) {
    DE_FUNC_REC *del = rover;
    rover = rover->next;
    delete[] del->zfunc_canon_name;
    delete[] del->zfunc_src_name;
    delete del;
  }
}


AccentType GetAboveType(BaseType base_type, MNODE* accent, const Grammar* mml_entities)
{
  AccentType rv = OT_NONE;

  if (accent && ElementNameIs(accent, "mo")) {
    U32 unicodes[8];
    int content_tally = ChData2Unicodes(accent->p_chdata, unicodes, 8, mml_entities);
    if (content_tally == 1) {
      switch (unicodes[0]) {
      case 0x0302:             // &Hat;    
        rv = OT_HAT;
        break;
      case 0x02C7:             // &caron;  
        rv = OT_CHECK;
        break;
      case 0x02DC:             // &tilde;  
        rv = OT_TILDE;
        break;
      case 0x00B4:             // &acute;  
        rv = OT_ACUTE;
        break;
      case 0x0060:             // &grave;  
        rv = OT_GRAVE;
        break;
      case 0x02D8:             // &breve;  
        rv = OT_BREVE;
        break;
      case 0x00AF:             // &macr;   
        rv = OT_BAR;
        break;
      case 0x02DA:             // &ring;   
        rv = OT_MATHRING;
        break;
      case 0x02D9:             // &dot;    
        rv = OT_DOT;
        break;
      case 0x00A8:             // &die;    
        rv = OT_DDOT;
        break;
      case 0x20DB:             // &tdot;   
        rv = OT_DDDOT;
        break;
      case 0x20DC:             // &DotDot; 
        rv = OT_DDDDOT;
        break;
      case 0x20D7:             // &#x20D7; 
        rv = OT_VEC;
        break;
      default:
        TCI_ASSERT(0);
        break;
      }
    } else
      TCI_ASSERT(0);
  } else
    TCI_ASSERT(0);

  return rv;
}



// In the course of analyzing an ODE or PDE, the function we're solving
//  for and the independent variable(s) are decided.  Here we add that
//  info to the semantic tree that we're building.

void AppendODEfuncs(SEMANTICS_NODE* rv, DE_FUNC_REC* ODE_fnames, AnalyzerData* pData)
{
  DE_FUNC_REC* rover = ODE_fnames;

  while (rover) {
    SEMANTICS_NODE* s_odefunc = CreateSemanticsNode(SEM_TYP_FUNCTION);
    //s_odefunc->semantic_type = SEM_TYP_FUNCTION;

    if (rover->zfunc_src_name) {
      s_odefunc->contents = DuplicateString(rover->zfunc_src_name);
    } else
      TCI_ASSERT(0);

    if (rover->zfunc_canon_name) {
      s_odefunc->canonical_ID = DuplicateString(rover->zfunc_canon_name);
      SetSnodeOwner(s_odefunc, pData);
    } else
      TCI_ASSERT(0);

    if (pData -> GetDE_ind_vars()) {

        BUCKET_REC* arg_bucket = AddVarToBucket(MB_UNNAMED, pData -> GetDE_ind_vars(), pData);
        AppendBucketRecord(s_odefunc->bucket_list, arg_bucket);

    } else
        TCI_ASSERT(0);

    BUCKET_REC* bucket = MakeParentBucketRec(MB_ODEFUNC, s_odefunc);
    
    AppendBucketRecord(rv->bucket_list, bucket);

    rover = rover->next;
  }
}






/* As a MathML tree is processed, nodes that must be given
  a canonical ID may be encountered.  We keep a list that maps
  a canonical ID back to the mml node.  The following 3 functions
  manage that list.
*/

MIC2MMLNODE_REC* AppendIDRec(MIC2MMLNODE_REC* node_IDs_list,
                             U32 client_ID,
                             char* obj_name, 
                             MNODE* mml_node,
                             const char* src_markup)
{
  MIC2MMLNODE_REC* rv = node_IDs_list;

  if (obj_name && mml_node) {
    // obj_name may already be in the list
    if (node_IDs_list) {
      MIC2MMLNODE_REC *rover = node_IDs_list;
      while (rover) {
        if (rover->canonical_name && !strcmp(obj_name, rover->canonical_name))
          return rv;
        rover = rover->next;
      }
    }

    MIC2MMLNODE_REC *new_node = new MIC2MMLNODE_REC();
    new_node->next = NULL;
    new_node->owner_ID = client_ID;
    new_node->canonical_name = DuplicateString(obj_name);
    new_node->mml_markup = TNodeToStr(mml_node, NULL, 0);

    // Append the new record to the global list
    if (!node_IDs_list)
      rv = new_node;
    else {
      MIC2MMLNODE_REC* rover = node_IDs_list;
      while (rover->next)
        rover = rover->next;
      rover->next = new_node;
    }
  } else
    TCI_ASSERT(0);

  return rv;
}


