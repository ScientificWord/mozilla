// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

#include "fltutils.h"
#include "attriblist.h"
#include "strutils.h"
#include "dumputils.h"
#include "SNode.h"
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

      SEMANTICS_NODE *s_op = CreateSemanticsNode(SEM_TYP_INFIX_OP);
      //s_op->semantic_type = SEM_TYP_INFIX_OP;
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


