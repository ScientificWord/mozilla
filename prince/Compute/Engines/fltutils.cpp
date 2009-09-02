// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

#include "fltutils.h"
#include "attriblist.h"
#include "strutils.h"
#include "dumputils.h"
#include <string.h>
#include <ctype.h>

// Global utility functions

void AppendVarRec(VAR_REC * &curr_list, const char *v_nom, VarType v_type)
{
  VAR_REC *new_rec = new VAR_REC();
  new_rec->next = curr_list;

  new_rec->var_type = v_type;
  if (v_nom) {
    size_t zln = strlen(v_nom);
    char *tmp = new char[zln + 1];
    strcpy(tmp, v_nom);
    new_rec->var_eng_name = tmp;
  } else
    new_rec->var_eng_name = NULL;

  curr_list = new_rec;
}

void DisposeVarList(VAR_REC * v_list)
{
  while (v_list) {
    VAR_REC *del = v_list;
    v_list = v_list->next;
    delete[] del->var_eng_name;
    delete del;
  }
}

VAR_REC* FindVarRec(VAR_REC * v_list, const char *targ_nom)
{
  VAR_REC *v_rover = v_list;
  while (v_rover) {
    if (v_rover->var_eng_name && !strcmp(v_rover->var_eng_name, targ_nom)) {
      return v_rover;
    }
    v_rover = v_rover->next;
  }

  return v_rover;
}








BUCKET_REC* MakeBucketRec(U32 which_bucket, SEMANTICS_NODE* sem_child)
{
  BUCKET_REC* rv = new BUCKET_REC();
  rv->next = NULL;
  rv->parts = NULL;
  rv->bucket_ID = which_bucket;
  rv->first_child = sem_child;

  return rv;
}


// jcs -- a common idiom
BUCKET_REC* MakeParentBucketRec(U32 which_bucket, SEMANTICS_NODE* sem_child)
{
  BUCKET_REC* rv = MakeBucketRec(which_bucket, sem_child);
  if (sem_child != NULL)
    sem_child -> parent = rv;
  return rv;

}



BUCKET_REC* AppendBucketRec(BUCKET_REC* a_list, BUCKET_REC* list_to_append)
{
  if (!a_list) {
    return list_to_append;
  } else {
    BUCKET_REC *rover = a_list;
    while (rover->next)
      rover = rover->next;
    rover->next = list_to_append;
    return a_list;
  }
}

BUCKET_REC* FindBucketRec(BUCKET_REC* a_list, U32 targ_bucket_ID)
{
  BUCKET_REC *rover = a_list;
  while (rover) {
    if (rover->bucket_ID == targ_bucket_ID)
      break;
    rover = rover->next;
  }
  return rover;
}

void DisposeBucketList(BUCKET_REC* b_list)
{
  
  while (b_list) {
    if (b_list->parts) {
      DisposeBucketList(b_list->parts);
      b_list->parts = NULL;
    }
    BUCKET_REC* del = b_list;
    b_list = b_list -> next;
    DisposeSList(del->first_child);
    delete del;
  }
}

SEMANTICS_NODE* CreateSemanticsNode()
{
  SEMANTICS_NODE *rv = new SEMANTICS_NODE;
  rv->next = NULL;
  rv->prev = NULL;
  rv->parent = NULL;

  rv->owner_ID = 0;
  rv->error_flag = 0;
  rv->semantic_type = SEM_TYP_UNDEFINED;
  rv->canonical_ID = NULL;
  rv->contents = NULL;
  rv->msi_class = 0;
  rv->variant = SNV_None;
  rv->infix_precedence = 0;
  rv->subtree_type = 0;
  rv->nrows = 0;
  rv->ncols = 0;
  rv->n_sub_args = 0;
  rv->bucket_list = NULL;

  return rv;
}

void DisposeSemanticsNode(SEMANTICS_NODE * del)
{
  delete[] del->canonical_ID;
  delete[] del->contents;
  DisposeBucketList(del->bucket_list);
  delete del;
}

void DisposeSList(SEMANTICS_NODE * s_list)
{
  SEMANTICS_NODE *del;
  while (s_list) {
    del = s_list;
    s_list = s_list->next;
    DisposeSemanticsNode(del);
  }
}

int SemanticVariant2NumIntegrals(SemanticVariant var)
{
  switch (var) {
  case SNV_singleint:
  default:
    return 1;
  case SNV_doubleint:
    return 2;
  case SNV_tripleint:
    return 3;
  case SNV_quadint:
    return 4;
  }
}

char *GetBucketName(U32 bucket_ID)
{
  char *bucket_name = NULL;

  switch (bucket_ID) {
  case MB_UNNAMED:
    bucket_name = "UNNAMED";
    break;
  case MB_NUMERATOR:
    bucket_name = "NUMERATOR";
    break;
  case MB_DENOMINATOR:
    bucket_name = "DENOMINATOR";
    break;
  case MB_SCRIPT_BASE:
    bucket_name = "SCRIPT_BASE";
    break;
  case MB_SCRIPT_UPPER:
    bucket_name = "SCRIPT_UPPER ";
    break;
  case MB_SCRIPT_LOWER:
    bucket_name = "SCRIPT_LOWER";
    break;
  case MB_ROOT_BASE:
    bucket_name = "ROOT_BASE";
    break;
  case MB_ROOT_EXPONENT:
    bucket_name = "ROOT_EXPONENT";
    break;
  case MB_LOWERLIMIT:
    bucket_name = "LOWERLIMIT";
    break;
  case MB_UPPERLIMIT:
    bucket_name = "UPPERLIMIT";
    break;
  case MB_OPERAND:
    bucket_name = "OPERAND";
    break;
  case MB_INTEG_VAR:
    bucket_name = "VARIABLE OF INTEGRATION";
    break;
  case MB_SUM_VAR:
    bucket_name = "VARIABLE OF SUMATION";
    break;
  case MB_EIGENVALUE:
    bucket_name = "EIGENVALUE";
    break;
  case MB_EV_MULTIPLICITY:
    bucket_name = "MULTIPLICITY";
    break;
  case MB_EIGENVECTOR:
    bucket_name = "EIGENVECTOR";
    break;
  case MB_SIMPLEX_EXPR:
    bucket_name = "OBJECTIVE FUNCTION";
    break;
  case MB_DEF_VARNAME:
    bucket_name = "DEF_VARNAME";
    break;
  case MB_DEF_VARVALUE:
    bucket_name = "DEF_VARVALUE";
    break;
  case MB_DEF_FUNCHEADER:
    bucket_name = "DEF_FUNCHEADER";
    break;
  case MB_DEF_FUNCVALUE:
    bucket_name = "DEF_FUNCVALUE";
    break;
  case MB_MN_WHOLE:
    bucket_name = "MIXEDNUM_WHOLE";
    break;
  case MB_MN_FRACTION:
    bucket_name = "MIXEDNUM_FRAC";
    break;
  case MB_DIFF_VAR:
    bucket_name = "VARIABLE OF DIFFERENTATION";
    break;
  case MB_BOUNDARY_CONDITION:
    bucket_name = "BOUNDARY CONDITION";
    break;
  case MB_NPRIMES:
    bucket_name = "nPRIMES";
    break;
  case MB_SUBST_LOWER:
    bucket_name = "SUBST_LOWER";
    break;
  case MB_SUBST_UPPER:
    bucket_name = "SUBST_UPPER";
    break;
  case MB_PIECE_EXPR:
    bucket_name = "PIECE_EXPR";
    break;
  case MB_LL_VAR:
    bucket_name = "LL VARIABLE";
    break;
  case MB_LL_EXPR:
    bucket_name = "LL LIMIT EXPR";
    break;
  case MB_LL_DIRECTION:
    bucket_name = "LL DIRECTION";
    break;
  case MB_LOG_BASE:
    bucket_name = "LOG BASE";
    break;
  case MB_NSYS_EQUATION:
    bucket_name = "NUMERIC SYSTEM EQN";
    break;
  case MB_NSYS_CONDITION:
    bucket_name = "NUMERIC SYSTEM COND";
    break;
  case MB_RECUR_EQN:
    bucket_name = "SOLVE RECURION EQN";
    break;
  case MB_RECUR_FUNC:
    bucket_name = "SOLVE RECURION FUNC";
    break;
  case MB_BASE_VARIABLE:
    bucket_name = "BASE VARIABLE";
    break;
  case MB_SUB_QUALIFIER:
    bucket_name = "SUBSCRIPTED QUALIFIER";
    break;
  case MB_ODEFUNC:
    bucket_name = "ODE FUNCTION";
    break;
  case MB_INTERVAL_START:
    bucket_name = "INTERVAL START";
    break;
  case MB_INTERVAL_END:
    bucket_name = "INTERVAL END";
    break;
  case MB_INTERVAL_VAR:
    bucket_name = "INTERVAL VAR";
    break;
  case MB_NORM_NUMBER:
    bucket_name = "NORM NUMBER";
    break;
  case MB_FUNC_EXPONENT:
    bucket_name = "FUNCTION EXPONENT";
    break;
  case MB_SYSTEM_COEFFICIENTS:
    bucket_name = "SYSTEM COEFFICIENTS";
    break;
  case MB_SYSTEM_VALUES:
    bucket_name = "SYSTEM VALUES";
    break;
  case MB_WHICH_QUANTILE:
    bucket_name = "WHICH QUANTILE";
    break;

  default:
    TCI_ASSERT(!"Unhandled bucket ID.");
    break;
  }

  return bucket_name;
}

void DisposeIDsList(MIC2MMLNODE_REC * node_IDs_list)
{
  MIC2MMLNODE_REC *rover = node_IDs_list;
  while (rover) {
    MIC2MMLNODE_REC *del = rover;
    rover = rover->next;
    delete[] del->canonical_name;
    delete[] del->mml_markup;
    delete del;
  }
}

MIC2MMLNODE_REC *JoinIDsLists(MIC2MMLNODE_REC * IDs_list,
                              MIC2MMLNODE_REC * appender)
{
  MIC2MMLNODE_REC *rv = NULL;
  if (IDs_list) {
    rv = IDs_list;
    MIC2MMLNODE_REC *rover = IDs_list;
    while (rover->next)
      rover = rover->next;
    rover->next = appender;
  } else {
    rv = appender;
  }
  return rv;
}

const char *GetMarkupFromID(MIC2MMLNODE_REC * node_IDs_list,
                            const char *targ_name)
{
  MIC2MMLNODE_REC *rover = node_IDs_list;
  while (rover) {
    if (rover->canonical_name && !strcmp(targ_name, rover->canonical_name)) {
      return rover->mml_markup;
      break;
    }
    rover = rover->next;
  }

  return NULL;
}



U32 NumericEntity2U32(const char* p_entity)
{
   
  U32 unicode = 0;

  if (p_entity && *p_entity == '&' && *(p_entity + 1) == '#') {
    const char* p = p_entity + 2;
    int place_val = 10;
    if (*p == 'x') {
      p++;
      place_val = 16;
    }
    unicode = ASCII2U32(p, place_val);
  } else {
    TCI_ASSERT(0);
  }
  return unicode;
}



PARAM_REC* AppendParam(PARAM_REC* curr_list, 
                       U32 p_ID, 
                       U32 p_type,
                       const char* zdata)
{
  PARAM_REC *new_rec = new PARAM_REC();
  new_rec->next = curr_list;

  new_rec->param_ID = p_ID;
  new_rec->param_type = p_type;
  if (zdata) {
    size_t zln = strlen(zdata);
    char *tmp = new char[zln + 1];
    strcpy(tmp, zdata);
    new_rec->ztext = tmp;
  } else {
    new_rec->ztext = NULL;
  }
  return new_rec;
}


void DisposeParamList(PARAM_REC * p_list)
{
  while (p_list) {
    PARAM_REC *del = p_list;
    p_list = p_list->next;
    delete[] del->ztext;
    delete del;
  }
}


INPUT_NOTATION_REC* CreateNotationRec()
{
  INPUT_NOTATION_REC* new_struct = new INPUT_NOTATION_REC();

  new_struct->nbracket_tables = 0;
  new_struct->nparen_tables = 0;
  new_struct->nbrace_tables = 0;
  new_struct->n_tables = 0;
  new_struct->nmixed_numbers = 0;
  new_struct->funcarg_is_subscript = 0;
  new_struct->n_primes = 0;
  new_struct->n_doverds = 0;
  new_struct->n_overbars = 0;
  new_struct->n_dotaccents = 0;
  new_struct->n_Dxs = 0;
  new_struct->n_logs = 0;

  return new_struct;
}

SEMANTICS_NODE* AppendSLists(SEMANTICS_NODE* s_list,
                             SEMANTICS_NODE* new_tail)
{
  if (!s_list) {
    return new_tail;
  } else {
    SEMANTICS_NODE *s_rover = s_list;
    while (s_rover->next)
      s_rover = s_rover->next;

    s_rover->next = new_tail;
    if (new_tail)
      new_tail->prev = s_rover;

    return s_list;
  }
}


SEMANTICS_NODE* NestInPGroup(SEMANTICS_NODE* s_list,
                             BUCKET_REC* parent_bucket)
{
  SEMANTICS_NODE *rv = CreateSemanticsNode();

  rv->semantic_type = SEM_TYP_PRECEDENCE_GROUP;
  rv->parent = parent_bucket;
  rv->bucket_list = MakeBucketRec(MB_UNNAMED, s_list);

  SEMANTICS_NODE *s_rover = s_list;
  while (s_rover) {
    s_rover->parent = rv->bucket_list;
    s_rover = s_rover->next;
  }

  return rv;
}

// Note that precedence values for SEMANTICS_NODEs are different from
// the MathML values from the Operator Dictionary.
void SetInfixPrecedence(SEMANTICS_NODE * snode)
{
  if (snode && snode->contents) {
    int p = 0;

    char *ptr = snode->contents;
    if (*ptr == '&' && *(ptr + 1) == '#') { // numeric entity
      int off = 2;
      int base = 10;            // "&#1234;"
      if (*(ptr + 2) == 'x') {
        base = 16;              // "&#x220a;"
        off++;
      }
      U32 unicode = ASCII2U32(ptr + off, base);
      switch (unicode) {
      case 0x2218:                  // "compfn"
        p = 31;                 //SLS just a guess
        break;
      case 0x2061:                  // "applyfunction"
        p = 30;                 //SLS just a guess
        break;
      case 0x2063:                  // "InvisibleComma"
        p = 18;
        break;
      case 0xd7  :                  // "times"
      case 0x2062:                  // "invisibletimes"
      case 0x2217:                  // "lowast"
      case 0x22c5:                  // "sdot"
      case 0x22c6:                  // "starf"
        p = 28;
        break;
      case 0x2228:                  // "or"
        p = 20;
        break;
      case 0x2227:                  // "and"
      case 0x26:                    // "amp"
        p = 21;
        break;
      case 0xac:                    // "not"
        p = 22;
        break;
      case 0x3c:                    // '<'
      case 0x3e:                    // '>'
      case 0x2260:                  // '!='
      case 0x2264:                  // '<='
      case 0x2265:                  // '>='
      case 0x2208:                  // "element"
        p = 24;
        break;
      case 0x222a:                  // "union"
      case 0x2216:                  // "minus"
      case 0x2212:                  // "-"
      case 0xb1:                    // "+-"
      case 0x2213:                  // "-+"
        p = 27;
        break;
      case 0x2229:                  // "intersect"
        p = 28;
        break;
      default:
        TCI_ASSERT(!"Unicode operator not found.");
        break;
      }
    } else {
      size_t zln = strlen(snode->contents);
      if (zln == 1) {
        char ch0 = snode->contents[0];

        if (ch0 == '+' || ch0 == '-') {
          p = 27;
        } else if (ch0 == '*' || ch0 == '/' || ch0 == '\\') {
          p = 28;
        } else if (ch0 == '=' || ch0 == '<' || ch0 == '>') {
          p = 24;
        } else if (ch0 == ',' || ch0 == '|') {
          p = 18;
        } else if (ch0 == '!') {
          p = 30;
        } else if (ch0 == '^') {
          p = 29;
        } else if (ch0 == ':') {
          p = 17;
        }
      } else if (zln == 2) {
        char ch0 = snode->contents[0];
        char ch1 = snode->contents[1];

        if (ch0 == '<' && ch1 == '=') {
          p = 24;
        } else if (ch0 == '>' && ch1 == '=') {
          p = 24;
        } else if (ch0 == '<' && ch1 == '>') {
          p = 24;
        } else if (ch0 == '.' && ch1 == '.') {
          p = 25;
        } else if (ch0 == ':' && ch1 == ':') {
          p = 30;
        } else if (ch0 == 'i' && ch1 == 'n') {
          p = 25;
        } else if (ch0 == 'o' && ch1 == 'r') {
          p = 20;
        }
      } else if (zln == 3) {
        if (!strcmp(snode->contents, "and")) {
          p = 21;
        } else if (!strcmp(snode->contents, "not")) {
          p = 22;
        } else if (!strcmp(snode->contents, "mod")) {
          p = 23;
        }
      } else if (zln == 5) {
        if (!strcmp(snode->contents, "union")) {
          p = 27;
        } else if (!strcmp(snode->contents, "minus")) {
          p = 27;
        }
      } else if (zln == 6) {
        if (!strcmp(snode->contents, "mnjoin"))
          p = 31;
      } else if (zln == 9) {
        if (!strcmp(snode->contents, "intersect"))
          p = 28;
      }
    }
    TCI_ASSERT(p);  // if p=0 then we don't have this operator in our list
    snode->infix_precedence = p;
  } else {
    TCI_ASSERT(!"Bad arg");
  }
}

// The initial output functions in STree2MML expect semantic trees
//  that contain the "infix" form for operators.  For some commands,
//  "prefix" semantic trees generated by Analyzer (NOT from an engine)
//  are used directly for output.  The following function converts
//  these "prefix" trees to "infix".

SEMANTICS_NODE* PrefixToInfix(SEMANTICS_NODE* s_list)
{
  SEMANTICS_NODE* rv = s_list;

  SEMANTICS_NODE* s_rover = s_list;
  while (s_rover) {
    SEMANTICS_NODE *save_next = s_rover->next;

    if (s_rover->bucket_list) {
      if (s_rover->semantic_type == SEM_TYP_INFIX_OP
          || s_rover->semantic_type == SEM_TYP_PREFIX_OP
          || s_rover->semantic_type == SEM_TYP_POSTFIX_OP) {
        BUCKET_REC* new_parent = s_rover->parent;
        SEMANTICS_NODE* l_anchor = s_rover->prev;
        SEMANTICS_NODE* r_anchor = s_rover->next;

        BUCKET_REC* b_node = s_rover->bucket_list;
        SEMANTICS_NODE* l_operand = NULL;
        SEMANTICS_NODE* r_operand = NULL;
        if (s_rover->semantic_type == SEM_TYP_INFIX_OP
            || s_rover->semantic_type == SEM_TYP_POSTFIX_OP) {
          l_operand = b_node->first_child;
          if (b_node->next) {
            r_operand = b_node->next->first_child;
            b_node->next->first_child = NULL;
          }
        } else {
          TCI_ASSERT(s_rover->semantic_type == SEM_TYP_PREFIX_OP);
          r_operand = b_node->first_child;
        }

        b_node->first_child = NULL;
        DisposeBucketList(b_node);
        s_rover->bucket_list = NULL;
        if (r_operand)
          r_operand->parent = NULL;
        if (l_operand)
          l_operand->parent = NULL;

        SEMANTICS_NODE *new_list = NULL;
        if (l_operand) {
          if (l_operand->bucket_list)
            new_list = PrefixToInfix(l_operand);
          else
            new_list = l_operand;
          new_list->next = s_rover;
          s_rover->prev = new_list;
        } else {
          new_list = s_rover;
        }
        if (r_operand) {
          if (r_operand->bucket_list)
            r_operand = PrefixToInfix(r_operand);
          s_rover->next = r_operand;
          r_operand->prev = s_rover;
        } else {
          s_rover->next = NULL;
        }

        SEMANTICS_NODE* pg = NestInPGroup(new_list, new_parent);

        if (rv == s_rover)
          rv = pg;

        if (l_anchor) {
          l_anchor->next = pg;
          pg->prev = l_anchor;
        } else {
          if (new_parent)
            new_parent->first_child = pg;
          pg->parent = new_parent;
        }
        if (r_anchor) {
          r_anchor->prev = pg;
          pg->next = r_anchor;
        }
      } else {                  // has children, but isn't an operator
        BUCKET_REC* b_node = s_rover->bucket_list;
        while (b_node) {
          PrefixToInfix(b_node->first_child);
          b_node = b_node->next;
        }
      }
    }
    s_rover = save_next;
  }

  return rv;
}



char* NestInParens(char* z_expr, bool forced)
{
  char *rv = NULL;

  if (z_expr && *z_expr) {
    bool do_it = true;

    if (*z_expr == '(') {
      // Find matching ')'
      char *p = z_expr + 1;
      int level = 1;
      while (*p) {
        if (*p == '(')
          level++;
        else if (*p == ')')
          level--;
        if (level == 0)
          break;
        else
          p++;
      }
      if (level == 0 && *(p + 1) == 0)
        do_it = false;
    }

    if (do_it) {
      size_t zln = strlen(z_expr);
      rv = new char[zln + 3];
      strcpy(rv, "(");
      strcat(rv, z_expr);
      strcat(rv, ")");
      delete[] z_expr;
    } else {
      rv = z_expr;
    }
  } else if (z_expr || forced) {
    rv = new char[3];
    strcpy(rv, "()");
    delete[] z_expr;
  }
  return rv;
}



char* NestInBrackets(char *z_expr)
{
  char* rv = NULL;

  if (z_expr && *z_expr) {
    size_t zln = strlen(z_expr);
    rv = new char[zln + 3];
    strcpy(rv, "[");
    strcat(rv, z_expr);
    strcat(rv, "]");
    delete[] z_expr;
  } else if (z_expr) {
    rv = new char[3];
    strcpy(rv, "[]");
    delete[] z_expr;
  }
  return rv;
}



char* RemovezStrParens(char *z_expr)
{
  char* rv = z_expr;

  if (z_expr && *z_expr) {
    bool do_it = false;
    if (*z_expr == '(') {       // Find matching ')'
      char *p = z_expr + 1;
      int level = 1;
      while (*p) {
        if (*p == '(')
          level++;
        else if (*p == ')')
          level--;
        if (level == 0)
          break;
        else
          p++;
      }
      if (level == 0 && *(p + 1) == 0)
        do_it = true;
    }
    if (do_it) {
      size_t zln = strlen(z_expr) - 2;
      rv = new char[zln + 1];
      strncpy(rv, z_expr + 1, zln);
      rv[zln - 1] = 0;
      delete[] z_expr;
    }
  }
  return rv;
}



void FunctionToInfix(SEMANTICS_NODE * s_func, char *zop_str)
{
  if (s_func && s_func->bucket_list) {
    BUCKET_REC *b1 = s_func->bucket_list;
    BUCKET_REC *b2 = b1->next;
    if (b2 && !b2->next) {
      s_func->semantic_type = SEM_TYP_PRECEDENCE_GROUP;
      delete[] s_func->contents;
      s_func->contents = NULL;

      SEMANTICS_NODE *s_left_op = b1->first_child;
      if (s_left_op && s_left_op->next) {
        s_left_op = NestInPGroup(s_left_op, b1);
        b1->first_child = s_left_op;
      }
      b1->next = NULL;

      SEMANTICS_NODE *s_op = CreateSemanticsNode();
      s_op->semantic_type = SEM_TYP_INFIX_OP;
      size_t zln = strlen(zop_str);
      char *tmp = new char[zln + 1];
      strcpy(tmp, zop_str);
      s_op->contents = tmp;
      s_left_op->next = s_op;
      s_op->prev = s_left_op;
      SEMANTICS_NODE *s_right_op = b2->first_child;
      b2->first_child = NULL;
      DisposeBucketList(b2);
      if (s_right_op && s_right_op->next) {
        s_right_op = NestInPGroup(s_right_op, b1);
      }

      s_op->next = s_right_op;
      s_right_op->prev = s_op;
    } else {
      TCI_ASSERT(0);
    }
  }
}

int CountSymbols(const char *p_chdata, int &n_entities)
{
  int rv = 0;
  n_entities = 0;

  if (p_chdata && *p_chdata) {
    const char *ptr = p_chdata;
    while (*ptr) {
      if (*ptr == '&') {
        while (*ptr && *ptr != ';')
          ptr++;
        if (*ptr == ';')
          n_entities++;
        else {
          TCI_ASSERT(!"Poorly formed entity.");
          break;
        }
      }
      rv++;
      ptr++;
    }
  }
  return rv;
}

// May need to reverse bytes in the following 2 functions
char* WideToASCII(const U16 * w_markup)
{
  char *rv = NULL;
  U32 zln = 0;

  const U16 *p = w_markup;
  while (*p) {
    char ascii[16];

    U16 wchar = *p;
    if (wchar < 0x80) {
      ascii[0] = wchar;
      ascii[1] = 0;
    } else {
      sprintf(ascii, "&#x%x;", wchar);
    }
    rv = AppendStr2HeapStr(rv, zln, ascii);
    p++;
  }
  return rv;
}

// To emulate Mozilla, we convert our input ASCII MathML
//  into a widechar string.
U16* ASCIItoWide(const char *ascii, int &zlen)
{
  U16 *rv = NULL;
  zlen = 0;

  if (ascii) {
    size_t zln = strlen(ascii);

    rv = new U16[zln + 1];
    U32 bi = 0;
    const char *p = ascii;
    while (*p) {
      U16 wide;
      char ch = *p;
      if (ch == '&' && *(p + 1) == '#') { // numeric entity
        int off = 2;
        int base = 10;          // "&#1234;"
        if (*(p + 2) == 'x') {
          base = 16;            // "&#x220a;"
          off++;
        }
        U32 unicode = ASCII2U32(p + off, base);
        if (unicode >= 0x20 && unicode < 0x10000) {
          if (unicode == 0x3c   // "&#x3c;"  "<"   
              || unicode == 0x3e  //       ">"
              || unicode == 0x26) { //       "&"
            // This is tricky - these entities are translated byte by byte
            wide = ch;
          } else {
            // Here the numeric entity is translated into a single widechar
            wide = unicode;
            p = strchr(p, ';');
          }
        } else {
          TCI_ASSERT(0);
        }
      } else {
        wide = ch;
      }
      rv[bi++] = wide;
      p++;
    }
    zlen = bi;
    rv[zlen] = 0;
  }
  return rv;
}


char* DumpSNode(const SEMANTICS_NODE* s_node, int indent)
{
  char *zheap_str = NULL;
  U32 buffer_ln = 0;

  if (s_node) {
    // form the indentation string
    char indent_str[128];
    int ii = 0;
    while (ii < indent && ii < 127)
      indent_str[ii++] = ' ';
    indent_str[ii] = 0;

    char line_buffer[512];

    char *t_name = NULL;
    switch (s_node->semantic_type) {
    case SEM_TYP_UNDEFINED:
      t_name = "UNDEFINED";
      break;
    case SEM_TYP_VARIABLE:
      t_name = "VARIABLE";
      break;
    case SEM_TYP_NUMBER:
      t_name = "NUMBER";
      break;
    case SEM_TYP_MIXEDNUMBER:
      t_name = "MIXED NUMBER";
      break;
    case SEM_TYP_FUNCTION:
      t_name = "FUNCTION";
      break;
    case SEM_TYP_PREFIX_OP:
      t_name = "PREFIX OPERATOR";
      break;
    case SEM_TYP_INFIX_OP:
      t_name = "INFIX OPERATOR";
      break;
    case SEM_TYP_POSTFIX_OP:
      t_name = "POSTFIX OPERATOR";
      break;
    case SEM_TYP_UCONSTANT:
      t_name = "UCONSTANT";
      break;
    case SEM_TYP_SPACE:
      t_name = "SPACE";
      break;
    case SEM_TYP_TEXT:
      t_name = "TEXT";
      break;
    case SEM_TYP_INVFUNCTION:
      t_name = "FUNCTION INVERSE";
      break;
    case SEM_TYP_SIUNIT:
      t_name = "SI UNIT";
      break;
    case SEM_TYP_USUNIT:
      t_name = "US UNIT";
      break;
    case SEM_TYP_QUALIFIED_VAR:
      t_name = "QUALIFIED_VAR";
      break;
    case SEM_TYP_INDEXED_VAR:
      t_name = "INDEXED_VAR";
      break;
    case SEM_TYP_ENG_PASSTHRU:
      t_name = "ENGINE PASS THRU";
      break;
    case SEM_TYP_EQCHECK_RESULT:
      t_name = "EQUALITY CHECK RESULT";
      break;

    case SEM_TYP_MATH_CONTAINER:
      t_name = "MATH_CONTAINER";
      break;
    case SEM_TYP_PRECEDENCE_GROUP:
      t_name = "PRECEDENCE_GROUP";
      break;
    case SEM_TYP_PARENED_LIST:
      t_name = "PARENED_LIST";
      break;
    case SEM_TYP_BRACKETED_LIST:
      t_name = "BRACKETED_LIST";
      break;
    case SEM_TYP_GENERIC_FENCE:
      t_name = "GENERIC_FENCE";
      break;
    case SEM_TYP_QSUB_LIST:
      t_name = "QUALIFIER_LIST";
      break;

    case SEM_TYP_COMMA_LIST:
      t_name = "COMMA LIST";
      break;
    case SEM_TYP_INTERVAL:
      t_name = "INTERVAL";
      break;

    case SEM_TYP_PIECEWISE_FENCE:
      t_name = "PIECEWISE_FENCE";
      break;
    case SEM_TYP_FRACTION:
      t_name = "FRACTION";
      break;
    case SEM_TYP_POWERFORM:
      t_name = "POWERFORM";
      break;
    case SEM_TYP_SQRT:
      t_name = "SQRT";
      break;
    case SEM_TYP_ROOT:
      t_name = "ROOT";
      break;
    case SEM_TYP_BIGOP_SUM:
      t_name = "BIG_OPERATOR";
      break;
    case SEM_TYP_BIGOP_INTEGRAL:
      t_name = "BIG_OPERATOR";
      break;
    case SEM_TYP_TABULATION:
      t_name = "TABULATION";
      break;
    case SEM_TYP_LIMFUNC:
      t_name = "LIMFUNC";
      break;
    case SEM_TYP_BINOMIAL:
      t_name = "BINOMIAL";
      break;
    case SEM_TYP_SET:
      t_name = "SET";
      break;
    case SEM_TYP_ABS:
      t_name = "ABSOLUTE VALUE";
      break;
    case SEM_TYP_NORM:
      t_name = "NORM";
      break;
    case SEM_TYP_FLOOR:
      t_name = "FLOOR";
      break;
    case SEM_TYP_CEILING:
      t_name = "CEILING";
      break;
    case SEM_TYP_CONJUGATE:
      t_name = "COMPLEX CONJUGATE";
      break;
    case SEM_TYP_MTRANSPOSE:
      t_name = "MATRIX TRANSPOSE";
      break;
    case SEM_TYP_HTRANSPOSE:
      t_name = "HERMITIAN TRANSPOSE";
      break;
    case SEM_TYP_EIGENVECTOR:
      t_name = "EIGEN VECTOR";
      break;
    case SEM_TYP_EIGENVECTORSET:
      t_name = "SET OF EIGEN VECTORS";
      break;
    case SEM_TYP_SIMPLEX:
      t_name = "SIMPLEX";
      break;
    case SEM_TYP_NUMSYSTEM:
      t_name = "NUMERIC SYSTEM";
      break;
    case SEM_TYP_SOLVESYSTEM:
      t_name = "SOLVE SYSTEM";
      break;
    case SEM_TYP_LIST:
      t_name = "LIST";
      break;
    case SEM_TYP_PIECEWISE_LIST:
      t_name = "PIECEWISE_LIST";
      break;
    case SEM_TYP_ONE_PIECE:
      t_name = "ONE_PIECE";
      break;
    case SEM_TYP_VARDEF_DEFERRED:
      t_name = "VARIABLE DEF DEFERRED";
      break;
    case SEM_TYP_VARDEF:
      t_name = "VARIABLE DEF";
      break;
    case SEM_TYP_FUNCDEF:
      t_name = "FUNCTION DEF";
      break;
    case SEM_TYP_TENSOR:
      t_name = "TENSOR";
      break;
    case SEM_TYP_QUANTILE_RESULT:
      t_name = "QUANTILE RESULT";
      break;
    case SEM_TYP_BOOLEAN:
      t_name = "BOOLEAN";
      break;
    case SEM_TYP_LISTOPERATOR:
      t_name = "LISTOPERATOR";
      break;
    case SEM_TYP_DERIVATIVE:
      t_name = "DERIVATIVE";
      break;
    case SEM_TYP_PARTIALD:
      t_name = "PARTIAL DERIVATIVE";
      break;
    case SEM_TYP_PDE:
      t_name = "PD";
      break;
    case SEM_TYP_ODE_SYSTEM:
      t_name = "ODE SYSTEM";
      break;
    case SEM_TYP_RECURSION:
      t_name = "RECURSION";
      break;
    case SEM_TYP_SUBSTITUTION:
      t_name = "SUBSTITUTION";
      break;
    case SEM_TYP_MULTIARGFUNC:
      t_name = "MULTIARGFUNC";
      break;
    case SEM_TYP_TRANSFORM:
      t_name = "TRANSFORM";
      break;
    case SEM_TYP_INVTRANSFORM:
      t_name = "INVERSE TRANSFORM";
      break;

    default:
      TCI_ASSERT(0);
      t_name = "NOT IMPLEMENTED";
      break;
    }

    if (t_name)
      sprintf(line_buffer, "%s%s", indent_str, t_name);
    else
      sprintf(line_buffer, "%sSem_ID = %d", indent_str,
              static_cast<int>(s_node->semantic_type));
    zheap_str = AppendStr2HeapStr(zheap_str, buffer_ln, line_buffer);

    if (s_node->canonical_ID) {
      sprintf(line_buffer, " canonical_ID = \"%s\"",
              s_node->canonical_ID);
      zheap_str = AppendStr2HeapStr(zheap_str, buffer_ln, line_buffer);
    }
    if (s_node->contents) {
      sprintf(line_buffer, " contents = \"%s\"", s_node->contents);
      zheap_str = AppendStr2HeapStr(zheap_str, buffer_ln, line_buffer);
    }
    zheap_str = AppendStr2HeapStr(zheap_str, buffer_ln, "\n");

    if (s_node->nrows || s_node->ncols) {
      sprintf(line_buffer, "%srows = %d, columns = %d\n", indent_str,
              s_node->nrows, s_node->ncols);
      zheap_str = AppendStr2HeapStr(zheap_str, buffer_ln, line_buffer);
    }

    if (s_node->bucket_list) {
      BUCKET_REC *bucket_rover = s_node->bucket_list;
      while (bucket_rover) {
        char *bucket_name = GetBucketName(bucket_rover->bucket_ID);
        if (bucket_name)
          sprintf(line_buffer, "%s  %s\n", indent_str, bucket_name);
        else
          sprintf(line_buffer, "%s  Bucket_ID = %d\n", indent_str,
                  bucket_rover->bucket_ID);
        zheap_str = AppendStr2HeapStr(zheap_str, buffer_ln, line_buffer);

        if (bucket_rover->first_child) {
          char *tmp = DumpSList(bucket_rover->first_child, indent + 4);
          if (tmp) {
            zheap_str = AppendStr2HeapStr(zheap_str, buffer_ln, tmp);
            delete[] tmp;
          }
        } else if (bucket_rover->parts) {
          sprintf(line_buffer, "%sBucket Parts\n", indent_str);
          zheap_str = AppendStr2HeapStr(zheap_str, buffer_ln, line_buffer);

          BUCKET_REC *b_rover = bucket_rover->parts;
          while (b_rover) {
            char *bucket_name = GetBucketName(b_rover->bucket_ID);
            if (bucket_name)
              sprintf(line_buffer, "%s  %s\n", indent_str, bucket_name);
            else
              sprintf(line_buffer, "%s  Bucket_ID = %d\n", indent_str,
                      b_rover->bucket_ID);
            zheap_str = AppendStr2HeapStr(zheap_str, buffer_ln, line_buffer);

            if (b_rover->first_child) {
              char *tmp = DumpSList(b_rover->first_child, indent + 4);
              if (tmp) {
                zheap_str = AppendStr2HeapStr(zheap_str, buffer_ln, tmp);
                delete[] tmp;
              }
            }
            b_rover = b_rover->next;
          }
          sprintf(line_buffer, "%sEnd Bucket Parts\n", indent_str);
          zheap_str = AppendStr2HeapStr(zheap_str, buffer_ln, line_buffer);
        }
        bucket_rover = bucket_rover->next;
      }
    }
  }
  return zheap_str;
}

char* DumpSList(const SEMANTICS_NODE* s_list, int indent)
{
  char *zheap_str = NULL;
  U32 buffer_ln = 0;

  if (indent == 0)
    zheap_str = AppendStr2HeapStr(zheap_str, buffer_ln, "\n");
  while (s_list) {
    char *tmp = DumpSNode(s_list, indent);
    if (tmp) {
      zheap_str = AppendStr2HeapStr(zheap_str, buffer_ln, tmp);
      delete[] tmp;
    }
    s_list = s_list->next;
  }
  if (indent == 0)
    zheap_str = AppendStr2HeapStr(zheap_str, buffer_ln, "\n");

  return zheap_str;
}

#ifdef ALLOW_DUMPS
#ifdef DEBUG
void JBM::DumpSList(const SEMANTICS_NODE* semantic_tree)
{
  char *sem_str = DumpSList(semantic_tree, 0);
  if (sem_str) {
    JBM::JBMLine(sem_str);
    delete[] sem_str;
  }
}
#else
void JBM::DumpSList(const SEMANTICS_NODE * semantics_tree) {}
#endif
#endif


#include "Grammar.h"

bool IsTrigArgFuncName(Grammar* gmr, const char* nom)
{
  U32 ID, subID;
  const char *p_data;
  if (gmr->GetRecordFromName("TRIGARGFUNCS", nom, strlen(nom), ID, subID, &p_data)) {
    if (p_data && *p_data)
      int rhs = atoi(p_data); // unused
    return true;
  }

  return false;
}
  
// See also FUNCTIONS section in engine grammar files.
bool IsReservedFuncName(Grammar *gmr, const char *nom)
{
  U32 ID, subID;
  const char *p_data;
  if (gmr->GetRecordFromName("RESERVEDFUNCS", nom, strlen(nom), ID, subID, &p_data)) {
    if (p_data && *p_data)
      int rhs = atoi(p_data); // unused
    return true;
  }

  return false;
}
