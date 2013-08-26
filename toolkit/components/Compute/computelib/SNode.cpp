#include "SNode.h"
//#include "fltutils.h"
#include "strutils.h"
#include <stdio.h>
#include <cstring>



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


SEMANTICS_NODE* CreateSemanticsNode(SemanticType type)
{
  SEMANTICS_NODE *rv = CreateSemanticsNode();
  rv -> semantic_type = type;
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


SemanticType GetBigOpType(const char* op_chdata, SemanticVariant& n_integs)
{
  SemanticType rv = SEM_TYP_UNDEFINED;
  n_integs = SNV_None;

  if (op_chdata) {
    const char *ptr = strstr(op_chdata, "&#x");
    if (ptr) {
      U32 unicode = ASCII2U32(ptr + 3, 16);
      switch (unicode) {
      case 0x222B:             //&int;<uID7.1.1>prefix,31,U0222B
        n_integs = SNV_singleint;
        rv = SEM_TYP_BIGOP_INTEGRAL;
        break;
      case 0x222C:             //&Int;<uID7.1.2>prefix,31,U0222C
        n_integs = SNV_doubleint;
        rv = SEM_TYP_BIGOP_INTEGRAL;
        break;
      case 0x222D:             //&tint;<uID7.1.3>prefix,31,U0222D
        n_integs = SNV_tripleint;
        rv = SEM_TYP_BIGOP_INTEGRAL;
        break;
      case 0x2A0C:             //&qint;<uID7.1.4>prefix,31,U02A0C
        n_integs = SNV_quadint;
        rv = SEM_TYP_BIGOP_INTEGRAL;
        break;
      //      case  0x2217  :     //&int;&ctdot;&int;<uID7.1.5>prefix,31,U0222B'U022EF'U0222B
      case 0x222E:             //&conint;<uID7.1.6>prefix,31,U0222E
        rv = SEM_TYP_BIGOP_INTEGRAL;
        break;
      case 0x2211:             //&sum;<uID7.1.7>prefix,29,    U02211
      case 0x220F:             //&prod;<uID7.1.8>prefix,35,   U0220F
      case 0x22C2:             //&xcap;<uID7.1.9>prefix,35,   U022C2
      case 0x22C0:             //&xwedge;<uID7.1.10>prefix,35,U022C0
      case 0x2295:             //&xoplus;<uID7.1.11>prefix,29,U02295
      case 0x2A00:             //&xodot;<uID7.1.12>prefix,38, U02A00
      case 0x2A06:             //&xsqcup;<uID7.1.13>prefix,29,U02A06
      case 0x2210:             //&coprod;<uID7.1.14>prefix,35,U02210
      case 0x22C3:             //&xcup;<uID7.1.15>prefix,29,  U022C3
      case 0x22C1:             //&xvee;<uID7.1.16>prefix,29,  U022C1
      case 0x2A02:             //&xotime;<uID7.1.17>prefix,35,U02A02
      case 0x2A04:             //&xuplus;<uID7.1.18>prefix,29,U02A04
        rv = SEM_TYP_BIGOP_SUM;
        break;
      case 0x2207:             //&nabla;
        break;
      default:
        break;
      }
    }
  }
  return rv;
}



OpOrderIlk GetOpOrderIlk(SEMANTICS_NODE* relop)
{
  OpOrderIlk rv = OOI_none;

  if (relop && relop->contents) {
    char *data = relop->contents;
    U32 unicode = 0;
    char *ptr = strstr(data, "&#x");
    if (ptr)
      unicode = ASCII2U32(ptr + 3, 16);
    else
      unicode = data[0];

    switch (unicode) {
    case 0x3c:
      rv = OOI_lessthan;
      break;
    case 0x2264:
      rv = OOI_lessorequal;
      break;
    case 0x3d:
      rv = OOI_equal;
      break;
    case 0x3e:
      rv = OOI_greaterthan;
      break;
    case 0x2265:
      rv = OOI_greaterorequal;
      break;
    case 0x2208:
      rv = OOI_element;
      break;
    case 0x2192:
    case 0x27f6:
      rv = OOI_rightarrow;
      break;
    default:
      break;
    }
  } else {
    TCI_ASSERT(!"No operator contents.");
  }
  return rv;
}



SEMANTICS_NODE* LocateVarAndExpr(BUCKET_REC* l_bucket,
                                 SEMANTICS_NODE ** s_expr,
                                 int & direction)
{
  SEMANTICS_NODE* rv = NULL;
  *s_expr = NULL;
  direction = 0;

  if (l_bucket && l_bucket->first_child) {
    SEMANTICS_NODE* s_node = l_bucket->first_child;
    if (s_node->semantic_type == SEM_TYP_INFIX_OP) {
      BUCKET_REC* b_rover = s_node->bucket_list;
      if (b_rover) {
        SEMANTICS_NODE* e1_list = NULL;
        SEMANTICS_NODE* relop = s_node;
        SEMANTICS_NODE* e2_list = NULL;
        SEMANTICS_NODE* expr = NULL;

        OpOrderIlk op_order = GetOpOrderIlk(relop);
        if (op_order == OOI_rightarrow) {
          // descend to operands
          e1_list = b_rover->first_child;
          if (b_rover->next)
            e2_list = b_rover->next->first_child;
          else
            TCI_ASSERT(0);

          if (e1_list && e1_list->semantic_type == SEM_TYP_UCONSTANT) {
            TCI_ASSERT(0);
            // Might have mapped "i" to "imaginaryi".
          }
          if (e2_list && e2_list->semantic_type == SEM_TYP_UCONSTANT) {
            // may be approaching \infty
          }

          if (e1_list && e1_list->semantic_type == SEM_TYP_VARIABLE) {
            rv = e1_list;
            expr = e2_list;
          } else if (e2_list
                     && e2_list->semantic_type == SEM_TYP_VARIABLE) {
            rv = e2_list;
            expr = e1_list;
          } else {
            TCI_ASSERT(0);
          }
          if (expr->semantic_type == SEM_TYP_POWERFORM) {
            BUCKET_REC *base_bucket = FindBucketRec(expr->bucket_list,
                                                    MB_SCRIPT_BASE);
            BUCKET_REC *script_bucket = FindBucketRec(expr->bucket_list,
                                                      MB_SCRIPT_UPPER);
            if (base_bucket && base_bucket->first_child) {
              if (script_bucket && script_bucket->first_child) {
                SEMANTICS_NODE *s_super = script_bucket->first_child;
                if (s_super->semantic_type == SEM_TYP_INFIX_OP
                    || s_super->semantic_type == SEM_TYP_POSTFIX_OP) {
                  char *key = s_super->contents;
                  if (!strcmp(key, "+"))
                    direction = 1;
                  else if ( (!strcmp(key, "&#x2212;")) || (!strcmp(key, "-")) )
                    direction = 2;
                  else
                    TCI_ASSERT(0);
                }
              } else {
                TCI_ASSERT(0);
              }
              *s_expr = base_bucket->first_child;
            } else {
              TCI_ASSERT(0);
            }
          } else {
            *s_expr = expr;
            // x -> 0+
            if (expr->semantic_type == SEM_TYP_POSTFIX_OP) {
              char *key = expr->contents;
              if (!strcmp(key, "+"))
                direction = 1;
              else if (!strcmp(key, "&#x2212;"))
                direction = 2;
              else
                TCI_ASSERT(0);
              if (expr->bucket_list && expr->bucket_list->first_child) {
                *s_expr = expr->bucket_list->first_child;
              } else
                TCI_ASSERT(0);
            }
          }
        } else {
          TCI_ASSERT(0);
        }
      } else {
        TCI_ASSERT(0);
      }
    } else {
      TCI_ASSERT(0);
    }
  } else {
    TCI_ASSERT(0);
  }
  return rv;
}


// The following must be called when the function "\log" is
//  encountered and an explicit base is not given as a subscript

void AddDefaultBaseToLOG(SEMANTICS_NODE* snode, bool log_is_base10)
{
  BUCKET_REC* bucket = MakeBucketRec(MB_LOG_BASE, NULL);
  snode->bucket_list = AppendBucketRec(snode->bucket_list, bucket);

  SEMANTICS_NODE* log_base = CreateSemanticsNode();
  bucket->first_child = log_base;
  log_base->parent = bucket;

  char *ptr;
  if (log_is_base10) {
    ptr = "10";
    log_base->semantic_type = SEM_TYP_NUMBER;
  } else {
    ptr = "&#x2147;";
    log_base->semantic_type = SEM_TYP_UCONSTANT;
  }

  log_base->contents = DuplicateString(ptr);
}


void FenceToMatrix(SEMANTICS_NODE * operand)
{
  if (operand->semantic_type == SEM_TYP_PARENED_LIST
      || operand->semantic_type == SEM_TYP_BRACKETED_LIST) {
    int n_rows = 1;
    int n_cols = 0;
    BUCKET_REC *brover = operand->bucket_list;
    while (brover) {
      n_cols++;
      brover = brover->next;
    }

    if (n_cols > 1){
      operand->semantic_type = SEM_TYP_TABULATION;
      operand->nrows = n_rows;
      operand->ncols = n_cols;

      delete[] operand->contents;
      operand->contents = DuplicateString("matrix");
    }
  }
}



void FenceToInterval(SEMANTICS_NODE * s_fence)
{
  SemanticVariant interval_type = SNV_None;
  if (s_fence->semantic_type == SEM_TYP_PARENED_LIST) {
    interval_type = SNV_ExclExclInterval;
  } else if (s_fence->semantic_type == SEM_TYP_BRACKETED_LIST) {
    interval_type = SNV_InclInclInterval;
  } else if (s_fence->semantic_type == SEM_TYP_INTERVAL) {
    interval_type = s_fence->variant;
  } else if (s_fence->semantic_type == SEM_TYP_SET ||
             s_fence->semantic_type == SEM_TYP_GENERIC_FENCE ||
             s_fence->semantic_type == SEM_TYP_VARIABLE) {
    return;
  } else {
    TCI_ASSERT(!"Unexpected semantic type on fence.");
    return;
  }

  int n_cols = 0;
  BUCKET_REC *brover = s_fence->bucket_list;
  while (brover) {
    n_cols++;
    brover = brover->next;
  }

  if (n_cols == 2) {
    s_fence->semantic_type = SEM_TYP_INTERVAL;
    s_fence->variant = interval_type;

    delete[] s_fence->contents;
    s_fence->contents = DuplicateString("interval");

    BUCKET_REC *brover = s_fence->bucket_list;
    brover->bucket_ID = MB_INTERVAL_START;
    brover = brover->next;
    brover->bucket_ID = MB_INTERVAL_END;
  }
}



void Patchdx(SEMANTICS_NODE * s_frac)
{
  if (s_frac
      && s_frac->semantic_type == SEM_TYP_FRACTION
      && s_frac->bucket_list && s_frac->bucket_list->first_child) {
    BUCKET_REC *b_num = FindBucketRec(s_frac->bucket_list, MB_NUMERATOR);
    if (b_num && b_num->first_child) {
      SEMANTICS_NODE *s_num = b_num->first_child;

      if (s_num->semantic_type == SEM_TYP_INFIX_OP && s_num->bucket_list) {
        BUCKET_REC *b_rover = s_num->bucket_list;
        while (b_rover) {
          SEMANTICS_NODE *s_operand = b_rover->first_child;
          if (s_operand
              && s_operand->semantic_type == SEM_TYP_PRECEDENCE_GROUP
              && s_operand->bucket_list) {
            SEMANTICS_NODE *s_nest = s_operand->bucket_list->first_child;
            if (s_nest
                && s_nest->semantic_type == SEM_TYP_PREFIX_OP
                && !strcmp(s_nest->contents, "&#x2146;")) {
              s_num = s_nest;
              break;
            }
          }
          b_rover = b_rover->next;
        }
      }

      if (s_num
          && s_num->semantic_type == SEM_TYP_PREFIX_OP
          && !strcmp(s_num->contents, "&#x2146;")) {
        delete[] s_num->contents;
        s_num->contents = DuplicateString("1");

        s_num->semantic_type = SEM_TYP_NUMBER;
        if (s_num->bucket_list) {
          DisposeBucketList(s_num->bucket_list);
          s_num->bucket_list = NULL;
        }
      } else
        TCI_ASSERT(0);
    } else
      TCI_ASSERT(0);
  } else
    TCI_ASSERT(0);
}


SEMANTICS_NODE* CreateOnePiece(SEMANTICS_NODE * s_expression,
                                         SEMANTICS_NODE * s_domain)
{
  SEMANTICS_NODE* rv = CreateSemanticsNode();
  rv->semantic_type = SEM_TYP_ONE_PIECE;

  BUCKET_REC* b_list = MakeParentBucketRec(MB_PIECE_EXPR, rv);
  b_list->first_child = s_expression;
  

  BUCKET_REC* new_b = MakeBucketRec(MB_UNNAMED, rv);
  b_list = AppendBucketRec(b_list, new_b);
  s_domain = PrefixToInfix(s_domain);
  new_b->first_child = s_domain;
  s_domain->parent = new_b;

  rv->bucket_list = b_list;

  return rv;
}


bool LocatePieces(BUCKET_REC * cell_list,
                                U32 row_tally, U32 ncols,
                                SEMANTICS_NODE ** s_expression,
                                SEMANTICS_NODE ** s_domain)
{
  bool rv = false;

  U32 targ = ncols * row_tally;
  BUCKET_REC *b_rover = cell_list;
  U32 tally = 0;
  while (b_rover && tally < targ) {
    tally++;
    b_rover = b_rover->next;
  }

  if (b_rover) {
    *s_expression = b_rover->first_child;
    if (b_rover->first_child)
      b_rover->first_child = NULL;
    (*s_expression)->parent = NULL;
    b_rover = b_rover->next;
    if (b_rover) {
      if (ncols == 3)
        b_rover = b_rover->next;
      if (b_rover) {
        *s_domain = b_rover->first_child;
        if (b_rover->first_child)
          b_rover->first_child = NULL;
        (*s_domain)->parent = NULL;
        rv = true;
      }
    }
  }
  return rv;
}




void ConvertToPIECEWISElist(SEMANTICS_NODE* s_fence)
{ 
  if (s_fence && s_fence->bucket_list && s_fence->bucket_list->first_child) {
    SEMANTICS_NODE* s_matrix = s_fence->bucket_list->first_child;

    if (s_matrix->semantic_type == SEM_TYP_TABULATION) {
      U32 nrows = s_matrix->nrows;
      U32 ncols = s_matrix->ncols;

      if (ncols == 2 || ncols == 3) {
        BUCKET_REC *cell_list = s_matrix->bucket_list;
        BUCKET_REC *b_list = NULL;

        int row_tally = 0;
        while (row_tally < nrows) {
          SEMANTICS_NODE *s_expression = NULL;
          SEMANTICS_NODE *s_domain = NULL;

          if (LocatePieces(cell_list, row_tally, ncols,
                           &s_expression, &s_domain)) {
            BUCKET_REC *new_b = MakeBucketRec(MB_UNNAMED, NULL);
            b_list = AppendBucketRec(b_list, new_b);
            SEMANTICS_NODE *s_onepiece =
              CreateOnePiece(s_expression, s_domain);
            new_b->first_child = s_onepiece;
            s_onepiece->parent = new_b;
          }
          row_tally++;
        }
        DisposeBucketList(s_fence->bucket_list);

        s_fence->bucket_list = b_list;
        s_fence->semantic_type = SEM_TYP_PIECEWISE_LIST;
      } else
        TCI_ASSERT(0);
    }
  } else
    TCI_ASSERT(0);
}


SEMANTICS_NODE* RemoveParens(SEMANTICS_NODE * s_operand)
{
  SEMANTICS_NODE *rv = s_operand;
  if (s_operand
      && !s_operand->next
      && s_operand->bucket_list && !s_operand->bucket_list->next) {
    if (s_operand->semantic_type == SEM_TYP_GENERIC_FENCE
        || s_operand->semantic_type == SEM_TYP_PRECEDENCE_GROUP) {
      rv = s_operand->bucket_list->first_child;
      s_operand->bucket_list->first_child = NULL;

      BUCKET_REC *parent_bucket = s_operand->parent;
      s_operand->parent = NULL;

      DisposeSList(s_operand);

      if (rv)
        rv->parent = parent_bucket;
      if (parent_bucket)
        parent_bucket->first_child = rv;
    }
  }
  return rv;
}




SEMANTICS_NODE* RemoveInfixOps(SEMANTICS_NODE* s_var)
{
  SEMANTICS_NODE *rv = s_var;

  SEMANTICS_NODE *s_rover = s_var;
  while (s_rover) {
    SEMANTICS_NODE *save_next = s_rover->next;

    if (s_rover->semantic_type == SEM_TYP_INFIX_OP
        && s_rover->bucket_list) {
      SEMANTICS_NODE *s_list = NULL;

      BUCKET_REC *b_rover = s_rover->bucket_list;
      while (b_rover) {
        SEMANTICS_NODE *s_operand = b_rover->first_child;
        if (s_operand) {
          s_list = AppendSLists(s_list, s_operand);
          b_rover->first_child = NULL;
        }
        b_rover = b_rover->next;
      }
      save_next = s_list;
      SEMANTICS_NODE *l_anchor = NULL;
      if (s_rover->prev)
        l_anchor = s_rover->prev;
      else
        rv = s_list;
      SEMANTICS_NODE *r_anchor = NULL;
      if (s_rover->next)
        r_anchor = s_rover->next;

      BUCKET_REC *new_parent = s_rover->parent;
      DisposeSemanticsNode(s_rover);

      SEMANTICS_NODE *tail = s_list;
      while (tail->next) {
        tail->parent = new_parent;
        tail = tail->next;
      }
      tail->parent = new_parent;

      if (l_anchor) {
        l_anchor->next = s_list;
        s_list->prev = l_anchor;
      }
      if (r_anchor) {
        tail->next = r_anchor;
        r_anchor->prev = tail;
      }
    } else {
    }
    s_rover = save_next;
  }

  return rv;
}


void SetVarAndIntervalLimit(BUCKET_REC * ll_bucket)
{
  SEMANTICS_NODE *s_ll;
  SEMANTICS_NODE *s_ul;
  bool ll_is_inclusive;
  bool ul_is_inclusive;
  SEMANTICS_NODE *s_var = LocateVarAndLimits(ll_bucket, &s_ll, &s_ul,
                                             ll_is_inclusive,
                                             ul_is_inclusive);
  if (s_ll || s_ul) {
    if (s_ll && s_ll->parent)
      s_ll->parent->first_child = NULL;
    if (s_ul && s_ul->parent)
      s_ul->parent->first_child = NULL;
    if (s_var && s_var->parent)
      s_var->parent->first_child = NULL;

    DisposeSList(ll_bucket->first_child);
    ll_bucket->first_child = NULL;

    SEMANTICS_NODE *interval = CreateSemanticsNode();
    interval->semantic_type = SEM_TYP_INTERVAL;
    interval->contents = DuplicateString("interval");

    SemanticVariant interval_type;
    if (ll_is_inclusive)
      interval_type =
        ul_is_inclusive ? SNV_InclInclInterval : SNV_InclExclInterval;
    else
      interval_type =
        ul_is_inclusive ? SNV_ExclInclInterval : SNV_ExclExclInterval;
    interval->variant = interval_type;

    if (s_ll) {
      BUCKET_REC* bucket = MakeParentBucketRec(MB_INTERVAL_START, s_ll);
      interval->bucket_list = AppendBucketRec(interval->bucket_list, bucket);
      
    }
    if (s_ul) {
      BUCKET_REC *bucket = MakeParentBucketRec(MB_INTERVAL_END, s_ul);
      interval->bucket_list = AppendBucketRec(interval->bucket_list, bucket);
      
    }

    if (s_var) {
      BUCKET_REC* bucket = MakeParentBucketRec(MB_INTERVAL_VAR, s_var);
      interval->bucket_list = AppendBucketRec(interval->bucket_list, bucket);
      
    }
    ll_bucket->first_child = interval;
    interval->parent = ll_bucket;
  } else
    TCI_ASSERT(0);
}

void SetVarArrowExprLimit(BUCKET_REC * ll_bucket)
{
  SEMANTICS_NODE *s_expr;
  int direction;
  SEMANTICS_NODE *s_var = LocateVarAndExpr(ll_bucket, &s_expr,
                                           direction);
  if (s_var) {
    BUCKET_REC* b_list = NULL;
    BUCKET_REC* var_bucket = MakeBucketRec(MB_LL_VAR, s_var);
    b_list = AppendBucketRec(b_list, var_bucket);
    if (s_var->parent && s_var->parent->first_child == s_var)
      s_var->parent->first_child = NULL;
    s_var->parent = var_bucket;

    if (s_expr) {
      BUCKET_REC* bucket = MakeBucketRec(MB_LL_EXPR, s_expr);
      b_list = AppendBucketRec(b_list, bucket);
      if (s_expr->parent && s_expr->parent->first_child == s_expr)
        s_expr->parent->first_child = NULL;
      s_expr->parent = bucket;
    }

    SEMANTICS_NODE *s_direction_num = CreateSemanticsNode();
    s_direction_num->semantic_type = SEM_TYP_NUMBER;
    char* num_str = new char[2];
    num_str[0] = direction + '0'; // '+' -> 1, '-' -> 2
    num_str[1] = 0;
    s_direction_num->contents = num_str;

    BUCKET_REC* bucket = MakeParentBucketRec(MB_LL_DIRECTION, s_direction_num);
    
    b_list = AppendBucketRec(b_list, bucket);

    DisposeSList(ll_bucket->first_child);
    ll_bucket->first_child = NULL;

    ll_bucket->parts = b_list;
  }
}


SEMANTICS_NODE* LocateVarAndLimits(BUCKET_REC * l_bucket,
                                             SEMANTICS_NODE ** s_ll,
                                             SEMANTICS_NODE ** s_ul,
                                             bool & ll_is_inclusive,
                                             bool & ul_is_inclusive
                                             )
{
  SEMANTICS_NODE *rv = NULL;
  *s_ll = NULL;
  *s_ul = NULL;
  ll_is_inclusive = false;
  ul_is_inclusive = false;

  if (l_bucket && l_bucket->first_child) {
    SEMANTICS_NODE *s_node = l_bucket->first_child;
    if (s_node->semantic_type == SEM_TYP_INFIX_OP) {
      BUCKET_REC *b_rover = s_node->bucket_list;
      if (b_rover) {
        SEMANTICS_NODE *e1_list = NULL;
        SEMANTICS_NODE *relop1 = s_node;
        SEMANTICS_NODE *e2_list = NULL;
        SEMANTICS_NODE *relop2 = NULL;
        SEMANTICS_NODE *e3_list = NULL;

        OpOrderIlk op_order = GetOpOrderIlk(relop1);
        if (op_order == OOI_element) {
          e1_list = b_rover->first_child;
          if (e1_list && e1_list->semantic_type == SEM_TYP_VARIABLE) {
            rv = e1_list;

            b_rover = b_rover->next;
            s_node = b_rover->first_child;
            if (s_node && s_node->bucket_list) {
              if (s_node->semantic_type == SEM_TYP_BRACKETED_LIST) {
                b_rover = s_node->bucket_list;
                *s_ll = b_rover->first_child;
                *s_ul = b_rover->next->first_child;
                ll_is_inclusive = true;
                ul_is_inclusive = true;
              } else if (s_node->semantic_type == SEM_TYP_PARENED_LIST) {
                b_rover = s_node->bucket_list;
                *s_ll = b_rover->first_child;
                *s_ul = b_rover->next->first_child;
                ll_is_inclusive = false;
                ul_is_inclusive = false;
              } else if (s_node->semantic_type == SEM_TYP_INTERVAL) {
                BUCKET_REC *b_ll =
                  FindBucketRec(s_node->bucket_list, MB_INTERVAL_START);
                if (b_ll) {
                  if (s_node->variant == SNV_InclInclInterval
                      || s_node->variant == SNV_InclExclInterval)
                    ll_is_inclusive = true;
                  else
                    ll_is_inclusive = false;
                  *s_ll = b_ll->first_child;
                } else
                  TCI_ASSERT(0);
                BUCKET_REC *b_ul =
                  FindBucketRec(s_node->bucket_list, MB_INTERVAL_END);
                if (b_ul) {
                  if (s_node->variant == SNV_ExclInclInterval
                      || s_node->variant == SNV_InclInclInterval)
                    ul_is_inclusive = true;
                  else
                    ul_is_inclusive = false;
                  *s_ul = b_ul->first_child;
                } else
                  TCI_ASSERT(0);
              } else
                TCI_ASSERT(0);
            } else
              TCI_ASSERT(0);
          } else
            TCI_ASSERT(0);
        } else if (op_order != OOI_none) { // relational
          s_node = b_rover->first_child;
          if (s_node->semantic_type == SEM_TYP_INFIX_OP) {
            relop2 = s_node;
            e3_list = b_rover->next->first_child;
            b_rover = s_node->bucket_list;
          }
          e1_list = b_rover->first_child;
          e2_list = b_rover->next->first_child;

          if (e1_list && e1_list->semantic_type == SEM_TYP_VARIABLE) {
            rv = e1_list;
            if (op_order == OOI_lessthan || op_order == OOI_lessorequal) {
              *s_ul = e2_list;
              if (op_order == OOI_lessorequal)
                ul_is_inclusive = true;
            } else if (op_order == OOI_equal) { // equal
              if (e2_list->semantic_type == SEM_TYP_INFIX_OP) { // n=1..4
                BUCKET_REC *b_ll = e2_list->bucket_list;
                if (b_ll
                    && b_ll->first_child
                    && b_ll->next && b_ll->next->first_child) {
                  *s_ll = b_ll->first_child;
                  *s_ul = b_ll->next->first_child;
                } else
                  TCI_ASSERT(0);
              } else {
                *s_ll = e2_list;
                *s_ul = e2_list;
              }
            } else if (op_order == OOI_greaterthan || op_order == OOI_greaterorequal) {
              *s_ll = e2_list;
              if (op_order == OOI_greaterorequal)
                ll_is_inclusive = true;
            }
          } else if (e2_list
                     && e2_list->semantic_type == SEM_TYP_VARIABLE) {
            rv = e2_list;
            if (relop2) {
              OpOrderIlk op_order2 = GetOpOrderIlk(relop2);
              if (op_order == OOI_lessthan || op_order == OOI_lessorequal) {
                *s_ll = e1_list;
                if (op_order == OOI_lessorequal)
                  ll_is_inclusive = true;
                *s_ul = e3_list;
                if (op_order2 == OOI_lessorequal)
                  ul_is_inclusive = true;
              } else if (op_order == OOI_equal) {
                *s_ll = e1_list;
                *s_ul = e3_list;
              } else if (op_order == OOI_greaterthan || op_order == OOI_greaterorequal) {
                *s_ll = e3_list;
                if (op_order2 == OOI_greaterorequal)
                  ll_is_inclusive = true;
                *s_ul = e1_list;
                if (op_order == OOI_greaterorequal)
                  ul_is_inclusive = true;
              }
            } else {
              if (op_order == OOI_lessthan || op_order == OOI_lessorequal) {
                *s_ll = e1_list;
                if (op_order == OOI_lessorequal)
                  ll_is_inclusive = true;
              } else if (op_order == OOI_equal) {
                *s_ll = e1_list;
                *s_ul = e1_list;
              } else if (op_order == OOI_greaterthan || op_order == OOI_greaterorequal) {
                *s_ul = e1_list;
                if (op_order == OOI_greaterorequal)
                  ul_is_inclusive = true;
              }
            }
          } else {
            TCI_ASSERT(0);
          }
        } else {
          TCI_ASSERT(0);
        }
      } else {
        TCI_ASSERT(0);
      }
    } else {
      TCI_ASSERT(0);
    }
  } else {
    TCI_ASSERT(0);
  }
  return rv;
}



// Currently not used

bool IsVarInSLIST(SEMANTICS_NODE * s_var_list, char *var_nom)
{
  bool rv = false;

  SEMANTICS_NODE *sv_rover = s_var_list;
  while (sv_rover) {
    SEMANTICS_NODE *s_var = sv_rover;
    if (sv_rover->semantic_type == SEM_TYP_POWERFORM) {
      BUCKET_REC *bucket =
        FindBucketRec(sv_rover->bucket_list, MB_SCRIPT_BASE);
      s_var = bucket->first_child;
    }

    if (s_var->semantic_type == SEM_TYP_VARIABLE && s_var->contents) {
      if (!strcmp(var_nom, s_var->contents)) {
        rv = true;
        break;
      }
    }
    sv_rover = sv_rover->next;
  }
  return rv;
}



void ArgsToMatrix(SEMANTICS_NODE * snode, BUCKET_REC * b_list)
{
  int n_rows = 1;
  int n_cols = 0;
  BUCKET_REC *brover = b_list;
  while (brover) {
    n_cols++;
    brover = brover->next;
  }

  if (n_cols > 1) {
    SEMANTICS_NODE *s_matrix = CreateSemanticsNode();
    s_matrix->semantic_type = SEM_TYP_TABULATION;
    s_matrix->nrows = n_rows;
    s_matrix->ncols = n_cols;
    s_matrix->contents = DuplicateString("matrix");

    BUCKET_REC* bucket = MakeParentBucketRec(MB_UNNAMED, s_matrix);
    
    s_matrix->bucket_list = AppendBucketRec(s_matrix->bucket_list, b_list);

    snode->bucket_list = AppendBucketRec(snode->bucket_list, bucket);
  } else
    snode->bucket_list = AppendBucketRec(snode->bucket_list, b_list);
}


// Here we nest operands under their operators
//  ie. convert to a prefix (reverse Polish) data structure

void CreatePrefixForm(SEMANTICS_NODE * s_operator,
                      SEMANTICS_NODE * l_operand,
                      SEMANTICS_NODE * r_operand)
{
  if (l_operand) {
    l_operand = RemoveParens(l_operand);
    BUCKET_REC* arg1_bucket = MakeParentBucketRec(MB_UNNAMED, l_operand);
    if (s_operator){
       s_operator->bucket_list = AppendBucketRec(s_operator->bucket_list, arg1_bucket);
    }
    if (l_operand){
      l_operand->prev = NULL;
      l_operand->next = NULL;
    }
  }
  if (r_operand) {
    r_operand = RemoveParens(r_operand);
    if (s_operator)
       AppendBucketRecord(s_operator->bucket_list, MakeParentBucketRec(MB_UNNAMED, r_operand));
    if (r_operand){
      r_operand->prev = NULL;
      r_operand->next = NULL;
    }
  }
}



void AppendNumber(SEMANTICS_NODE * snode, U32 bucket_ID, int num)
{
  SEMANTICS_NODE *s_nprimes = CreateSemanticsNode();
  BUCKET_REC* pr_bucket = MakeParentBucketRec(bucket_ID, s_nprimes);
  snode->bucket_list = AppendBucketRec(snode->bucket_list, pr_bucket);
  pr_bucket->first_child = s_nprimes;
  

  s_nprimes->semantic_type = SEM_TYP_NUMBER;
  char buffer[32];
  sprintf(buffer, "%d", num);
  s_nprimes->contents = DuplicateString(buffer);
}

void RemoveBucket(SEMANTICS_NODE * s_base, BUCKET_REC * targ)
{
  BUCKET_REC *prev = NULL;
  BUCKET_REC *rover = s_base->bucket_list;
  while (rover) {
    BUCKET_REC *del = rover;
    rover = rover->next;
    if (del == targ) {
      if (del == s_base->bucket_list)
        s_base->bucket_list = rover;
      else
        prev->next = rover;
      del->next = NULL;
      DisposeBucketList(del);
      break;
    } else
      prev = del;
  }
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



void AppendBucketRecord(BUCKET_REC*& a_list, BUCKET_REC* new_a_rec)
{
  a_list = 	AppendBucketRec(a_list, new_a_rec);
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



SEMANTICS_NODE* NestInPGroup(SEMANTICS_NODE* s_list,
                             BUCKET_REC* parent_bucket)
{
  SEMANTICS_NODE *rv = CreateSemanticsNode(SEM_TYP_PRECEDENCE_GROUP);

  //rv->semantic_type = SEM_TYP_PRECEDENCE_GROUP;
  rv->parent = parent_bucket;
  rv->bucket_list = MakeBucketRec(MB_UNNAMED, s_list);

  SEMANTICS_NODE *s_rover = s_list;
  while (s_rover) {
    s_rover->parent = rv->bucket_list;
    s_rover = s_rover->next;
  }

  return rv;
}







