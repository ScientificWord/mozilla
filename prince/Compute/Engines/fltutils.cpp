// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

#include "fltutils.h"
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

VAR_REC *FindVarRec(VAR_REC * v_list, const char *targ_nom)
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

#ifdef ALLOW_DUMPS
void JBM::DumpTNode(MNODE * t_node, int indent)
{
  if (t_node) {
    // form the indentation string
    char indent_str[128];
    int ii = 0;
    while (ii < indent && ii < 127)
      indent_str[ii++] = ' ';
    indent_str[ii] = 0;

    char zzz[256];
    sprintf(zzz, "%4lu %s<%s", t_node->src_linenum, indent_str,
            t_node->src_tok);
    JBMLine(zzz);

    ATTRIB_REC *rover = t_node->attrib_list;
    while (rover) {
      sprintf(zzz, " %s=\"%s\"", rover->zattr_nom, rover->zattr_val);
      JBMLine(zzz);
      rover = rover->next;
    }

    if (t_node->p_chdata && t_node->first_kid) {
      sprintf(zzz, " name=\"%s\">", t_node->p_chdata);
      JBMLine(zzz);

      JBMLine("\n");
      DumpTList(t_node->first_kid, indent + 2);
      sprintf(zzz, "%4lu %s</%s>\n", t_node->src_linenum, indent_str,
              t_node->src_tok);
      JBMLine(zzz);
    } else if (t_node->p_chdata) {
      JBMLine(">");
      JBMLine(t_node->p_chdata);
      sprintf(zzz, "</%s>\n", t_node->src_tok);
      JBMLine(zzz);
    } else if (t_node->first_kid) {
      JBMLine(">\n");
      DumpTList(t_node->first_kid, indent + 2);
      sprintf(zzz, "%4lu %s</%s>\n", t_node->src_linenum, indent_str,
              t_node->src_tok);
      JBMLine(zzz);
    } else {
      if (!strcmp(t_node->src_tok, "mspace")) {
        JBMLine(">\n");
      } else {
        sprintf(zzz, "></%s>\n", t_node->src_tok);
        JBMLine(zzz);
      }
    }
  } else {
    TCI_ASSERT(!"bad input t_node");
  }
}

void JBM::DumpTList(MNODE * t_list, int indent)
{
  if (!indent)
    JBMLine("\n");
  while (t_list) {
    DumpTNode(t_list, indent);
    t_list = t_list->next;
  }
}
#else
void JBM::DumpTNode(MNODE * t_node, int indent) {}
void JBM::DumpTList(MNODE * t_list, int indent) {}
#endif

#ifdef XP_WIN
#define JBM_FILENAME "/temp/JBMLine.out"
#else
#define JBM_FILENAME "/tmp/JBMLine.out"
#endif

#ifdef DEBUG
void JBM::JBMLine(const char *line)
{
  FILE *jbmfile = fopen(JBM_FILENAME, "a");
  if (jbmfile) {
    if (line) {
      fputs(line, jbmfile);
    }
    fclose(jbmfile);
  }
}

void JBM::DumpSList(const SEMANTICS_NODE * semantic_tree)
{
  char *sem_str = DumpSList(semantic_tree, 0);
  if (sem_str) {
    JBM::JBMLine(sem_str);
    delete[] sem_str;
  }
 }
#else
void JBM::JBMLine(const char *line) {}
void JBM::DumpSList(const SEMANTICS_NODE * semantics_tree) {}
#endif

void JBM::ClearLog()
{
  FILE *jbmfile = fopen(JBM_FILENAME, "w");
  if (jbmfile)
    fclose(jbmfile);
}


// Utility to make a translation node

MNODE *MakeTNode(U32 s_off, U32 s_len, U32 line_no)
{
  MNODE *rv = new MNODE();
  rv->next = NULL;
  rv->prev = NULL;
  rv->src_linenum = line_no;
  rv->src_start_offset = s_off;
  rv->src_length = s_len;

  rv->src_tok[0] = 0;

  rv->parent = NULL;
  rv->first_kid = NULL;
  rv->p_chdata = NULL;
  rv->precedence = 0;
  rv->form = OP_none;
  rv->attrib_list = NULL;
  rv->msg_list = NULL;

  return rv;
}

const char * OpIlkToString(OpIlk ilk)
{
  switch (ilk) {
    case OP_prefix:
      return "prefix";
    case OP_infix:
      return "infix";
      break;
    case OP_postfix:
      return "postfix";
      break;
    default:
      TCI_ASSERT(!"No string form.");
      return NULL;
  }
}

OpIlk StringToOpIlk(const char * form)
{
    if (!strcmp(form, "prefix")) {
    return OP_prefix;
  } else if (!strcmp(form, "infix")) {
    return OP_infix;
  } else if (!strcmp(form, "postfix")) {
    return OP_postfix;
  } else {
    TCI_ASSERT(!"bogus form attribute on operator");
    return OP_none;
  }
}

// message IDs and strings for development purposes only.
// final versions of strings will go in language dependent resources.

char *eMsgStrs[] = {
  "Unsupported number, %s\n",
  "Unsupported operator, %s\n",
  "Undefined function, %s\n",
  "No inverse defined for function, %s\n",
  "Undefined prefix operator with limits, %s\n",
  "Unexpected text in math, %s\n",
  0
};

// See enum LogMsgID in fltutils.h
LOG_MSG_REC *MakeLogMsg()
{
  LOG_MSG_REC *rv = new LOG_MSG_REC();
  rv->next = NULL;
  rv->msg = NULL;

  return rv;
}

void DisposeMsgs(LOG_MSG_REC * msg_list)
{
  LOG_MSG_REC *msg_rover = msg_list;
  while (msg_rover) {
    LOG_MSG_REC *del = msg_rover;
    msg_rover = msg_rover->next;
    delete[] del->msg;
    delete del;
  }
}

LOG_MSG_REC *AppendLogMsg(LOG_MSG_REC * msg_list, LOG_MSG_REC * new_msg_rec)
{
  if (!msg_list)
    return new_msg_rec;
  else {
    LOG_MSG_REC *rover = msg_list;
    while (rover->next)
      rover = rover->next;
    rover->next = new_msg_rec;
    return msg_list;
  }
}

void RecordMsg(LOG_MSG_REC * &msg_list, LogMsgID id, const char *token)
{
  char *msg_str = eMsgStrs[id];
  size_t zln = strlen(msg_str);
  if (token)
    zln += strlen(token);

  char *buffer = new char[zln];
  if (token)
    sprintf(buffer, msg_str, token);
  else
    strcpy(buffer, msg_str);

  LOG_MSG_REC *new_msg_rec = MakeLogMsg();
  new_msg_rec->msg = buffer;
  msg_list = AppendLogMsg(msg_list, new_msg_rec);
}

void DisposeTNode(MNODE * del)
{
  delete[] del->p_chdata;
  DisposeAttribs(del->attrib_list);

  LOG_MSG_REC *msg_rover = del->msg_list;
  while (msg_rover) {
    LOG_MSG_REC *msg_del = msg_rover;
    msg_rover = msg_rover->next;
    delete msg_del->msg;
    delete msg_del;
  }

  DisposeTList(del->first_kid);
  delete del;
}

void DisposeTList(MNODE * t_list)
{
  MNODE *del;
  while (t_list) {
    del = t_list;
    t_list = t_list->next;
    DisposeTNode(del);
  }
}

MNODE *JoinTLists(MNODE * list, MNODE * newtail)
{
  MNODE *rv;
  if (list) {
    rv = list;
    while (list->next)
      list = list->next;
    list->next = newtail;
    if (newtail)
      newtail->prev = list;
  } else {
    rv = newtail;
  }

  return rv;
}

void StrReplace(char *line, size_t zln, char *tok, const char *sub)
{
  char *ptr = strstr(line, tok);
  if (ptr) {
    char *buffer = new char[zln];

    *ptr = 0;
    strcpy(buffer, line);       // head
    if (sub)
      strcat(buffer, sub);      // substitution
    ptr += strlen(tok);
    strcat(buffer, ptr);        // tail

    strcpy(line, buffer);
    delete[] buffer;
  }
}

// itoa replacement
void StrFromInt(int val, char* buffer)
{
  sprintf(buffer, "%d", val);
}

// Function to remove a MNODE's links to a tree.
// The delinked node can be moved or disposed by the caller.
// Note that we can't delink the first node of a first
//  level list here.

bool DelinkTNode(MNODE * elem)
{
  bool rv = true;           // assume all will be OK.

  // remove elem from it's present list:  prev <-> elem <-> next
  MNODE *el_prev = elem->prev;
  if (el_prev) {                // elem has a prev
    el_prev->next = elem->next;
    if (elem->next) {
      elem->next->prev = el_prev;
    }
  } else {                      // elem may head a sublist
    MNODE *el_owner = elem->parent;
    if (el_owner) {
      if (el_owner->first_kid == elem) {
        el_owner->first_kid = elem->next;
        if (elem->next) {
          elem->next->parent = el_owner;
          elem->next->prev = NULL;
        }
      } else {
        TCI_ASSERT(0);
        rv = false;
      }
    } else {
      TCI_ASSERT(!"No sublist_owner");
      rv = false;
    }
  }
  if (rv) {
    elem->prev = NULL;
    elem->next = NULL;
    elem->parent = NULL;
  }
  return rv;
}

void DetachTList(MNODE * elem)
{
  MNODE *prev_node = elem->prev;
  if (prev_node) {              // elem has a prev
    // remove elem from it's present list:  prev <-> elem
    prev_node->next = NULL;
    elem->prev = NULL;
  } else {                      // elem may head a sublist
    MNODE *owner_node = elem->parent;
    if (owner_node) {
      if (owner_node->first_kid == elem) {
        owner_node->first_kid = NULL;
      } else {
        TCI_ASSERT(0);
      }
      elem->parent = NULL;
    }
    //TCI_ASSERT(0);
  }
}

// there is a question about whether mfenced should be in this list
bool HasPositionalChildren(MNODE * mml_node)
{
  if (mml_node) {
    const char* p_elem = mml_node->src_tok;
    if (!strcmp(p_elem, "mfrac")
        || !strcmp(p_elem, "msub")
        || !strcmp(p_elem, "msup")
        || !strcmp(p_elem, "msubsup")
        || !strcmp(p_elem, "munder")
        || !strcmp(p_elem, "mover")
        || !strcmp(p_elem, "munderover"))
      return true;
  }
  return false;
}

// there is a question about whether mfenced and mtable should be in this list
bool HasRequiredChildren(MNODE * mml_node)
{
  if (mml_node) {
    const char* p_elem = mml_node->src_tok;
    if (!strcmp(p_elem, "mfrac")
        || !strcmp(p_elem, "mroot")
        || !strcmp(p_elem, "msub")
        || !strcmp(p_elem, "msup")
        || !strcmp(p_elem, "msubsup")
        || !strcmp(p_elem, "munder")
        || !strcmp(p_elem, "mover")
        || !strcmp(p_elem, "munderover"))
      return true;
  }
  return false;
}

bool HasScriptChildren(MNODE * mml_node)
{
  if (mml_node) {
    const char* p_elem = mml_node->src_tok;
    if (!strcmp(p_elem, "msub")
        || !strcmp(p_elem, "msup")
        || !strcmp(p_elem, "msubsup")
        || !strcmp(p_elem, "munder")
        || !strcmp(p_elem, "mover")
        || !strcmp(p_elem, "munderover"))
      return true;
  }
  return false;
}

bool HasInferedMROW(MNODE * mml_node)
{
  if (mml_node) {
    const char* p_elem = mml_node->src_tok;
    if (!strcmp(p_elem, "math")
        || !strcmp(p_elem, "msqrt")
        || !strcmp(p_elem, "mstyle")
        || !strcmp(p_elem, "merror")
        || !strcmp(p_elem, "mpadded")
        || !strcmp(p_elem, "mphantom")
        || !strcmp(p_elem, "menclose")
        || !strcmp(p_elem, "mtd"))
      return true;
  }
  return false;
}

// Utility to make an attribute node

ATTRIB_REC *MakeATTRIBNode(const char *attr_nom, const char *attr_val)
{
  ATTRIB_REC *rv = new ATTRIB_REC();
  rv->next = NULL;
  rv->prev = NULL;

  if (attr_nom) {
    size_t zln = strlen(attr_nom);
    char *tmp = new char[zln + 1];
    strcpy(tmp, attr_nom);
    rv->zattr_nom = tmp;
  } else {
    rv->zattr_nom = NULL;
  }
  if (attr_val) {               // we have a STR value for the attribute
    size_t zln = strlen(attr_val);
    char *tmp = new char[zln + 1];
    strcpy(tmp, attr_val);
    rv->zattr_val = tmp;
  } else {
    rv->zattr_val = NULL;
  }

  return rv;
}

void DisposeAttribs(ATTRIB_REC * alist)
{
  ATTRIB_REC *arover = alist;
  while (arover) {
    ATTRIB_REC *adel = arover;
    arover = arover->next;
    delete[] adel->zattr_nom;
    delete[] adel->zattr_val;
    delete adel;
  }
}

const char *GetATTRIBvalue(ATTRIB_REC * a_list, const char *targ_name)
{
  ATTRIB_REC *rover = a_list;
  while (rover) {
    if (!strcmp(targ_name, rover->zattr_nom)) {
      return rover->zattr_val;
    }
    rover = rover->next;
  }

  return NULL;
}

// largeop="true" stretchy="true"  lspace="0em" rspace="0em"

ATTRIB_REC *ExtractAttrs(char *zattrs)
{
  ATTRIB_REC *head = NULL;      // We build an "attrib_list"
  ATTRIB_REC *tail;

  if (zattrs) {
    bool done = false;
    char *anom_ptr = zattrs;
    while (!done) {
      // Span spaces
      while (*anom_ptr && *anom_ptr <= ' ')
        anom_ptr++;
      if (*anom_ptr) {
        char *p_equal = strchr(anom_ptr, '=');
        if (p_equal) {
          char *left_delim = strchr(p_equal + 1, '"');
          if (left_delim) {
            left_delim++;
            char *right_delim = strchr(left_delim, '"');
            if (right_delim) {
              while (p_equal > anom_ptr && *(p_equal - 1) <= ' ')
                p_equal--;
              char znom[80];
              size_t nom_ln = p_equal - anom_ptr;
              if (nom_ln && nom_ln < 80) {
                strncpy(znom, anom_ptr, nom_ln);
                znom[nom_ln] = 0;

                size_t val_ln = right_delim - left_delim;
                if (val_ln && val_ln < 128) {
                  char zval[128];
                  strncpy(zval, left_delim, val_ln);
                  zval[val_ln] = 0;
                  ATTRIB_REC *arp = MakeATTRIBNode(znom, zval);
                  if (!head)
                    head = arp;
                  else
                    tail->next = arp;
                  tail = arp;
                } else {
                  TCI_ASSERT(0);
                }
              } else {
                TCI_ASSERT(!"attrib zname exceeds 80 bytes??");
              }
              anom_ptr = right_delim + 1;
            } else {            // no ending "
              done = true;
            }
          } else {              // no starting "
            done = true;
          }
        } else {                // no = 
          done = true;
        }
      } else {                  // nothing after spaces
        done = true;
      }
    }
  }

  return head;
}

ATTRIB_REC *MergeAttrsLists(ATTRIB_REC * dest_list, ATTRIB_REC * new_attrs)
{
  ATTRIB_REC *rv = NULL;

  if (!dest_list) {
    rv = new_attrs;
  } else if (!new_attrs) {
    rv = dest_list;
  } else {
    rv = dest_list;

    ATTRIB_REC *rover = new_attrs;
    while (rover) {
      ATTRIB_REC *d_rover = dest_list;
      while (d_rover) {
        if (!strcmp(d_rover->zattr_nom, rover->zattr_nom)) {
          delete[] d_rover->zattr_val;
          if (rover->zattr_val) {
            d_rover->zattr_val = rover->zattr_val;
            rover->zattr_val = NULL;
          } else {
            d_rover->zattr_val = NULL;
          }
          ATTRIB_REC *ender = rover;
          rover = rover->next;
          ender->next = NULL;
          ender->prev = NULL;
          if (rover)
            rover->prev = NULL;
          DisposeAttribs(ender);
          break;
        } else {
          if (d_rover->next) {
            d_rover = d_rover->next;
          } else {
            d_rover->next = rover;
            rover->prev = d_rover;
            ATTRIB_REC *ender = rover;
            rover = rover->next;
            ender->next = NULL;
            if (rover)
              rover->prev = NULL;
            break;
          }
        }
      }
    }
  }

  return rv;
}

ATTRIB_REC *RemoveAttr(ATTRIB_REC * a_list, const char *attr_nom)
{
  ATTRIB_REC *rv = NULL;
  ATTRIB_REC *tail;

  if (a_list) {
    ATTRIB_REC *a_rover = a_list;
    while (a_rover) {
      if (!strcmp(a_rover->zattr_nom, attr_nom)) {
        ATTRIB_REC *ender = a_rover;
        a_rover = a_rover->next;
        ender->next = NULL;
        ender->prev = NULL;
        DisposeAttribs(ender);
        if (a_rover)
          a_rover->prev = NULL;
      } else {
        if (!rv) {
          rv = a_rover;
        } else {
          tail->next = a_rover;
          a_rover->prev = tail;
        }
        tail = a_rover;
        a_rover = a_rover->next;
        tail->next = NULL;
        a_rover->prev = NULL;
      }
    }
  }

  return rv;
}

BUCKET_REC *MakeBucketRec(U32 which_bucket, SEMANTICS_NODE * sem_child)
{
  BUCKET_REC *rv = new BUCKET_REC();
  rv->next = NULL;
  rv->parts = NULL;
  rv->bucket_ID = which_bucket;
  rv->first_child = sem_child;

  return rv;
}

BUCKET_REC *AppendBucketRec(BUCKET_REC * a_list, BUCKET_REC * list_to_append)
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

BUCKET_REC *FindBucketRec(BUCKET_REC * a_list, U32 targ_bucket_ID)
{
  BUCKET_REC *rover = a_list;
  while (rover) {
    if (rover->bucket_ID == targ_bucket_ID)
      break;
    rover = rover->next;
  }
  return rover;
}

void DisposeBucketList(BUCKET_REC * b_list)
{
  BUCKET_REC *del;
  while (b_list) {
    if (b_list->parts) {
      DisposeBucketList(b_list->parts);
      b_list->parts = NULL;
    }
    del = b_list;
    b_list = b_list->next;
    DisposeSList(del->first_child);
    delete del;
  }
}

SEMANTICS_NODE *CreateSemanticsNode()
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

U32 ASCII2U32(const char *ptr, int place_val)
{
  U32 unicode = 0;

  if (place_val == 10) {
    while (*ptr) {
      if (isdigit(*ptr))
        unicode = place_val * unicode + *ptr - '0';
      else
        break;
      ptr++;
    }
  } else if (place_val == 16) {
    while (*ptr) {
      if (isdigit(*ptr))
        unicode = place_val * unicode + *ptr - '0';
      else if (*ptr >= 'A' && *ptr <= 'F')
        unicode = place_val * unicode + *ptr - 'A' + 10;
      else if (*ptr >= 'a' && *ptr <= 'f')
        unicode = place_val * unicode + *ptr - 'a' + 10;
      else
        break;
      ptr++;
    }
  }

  return unicode;
}

U32 NumericEntity2U32(const char *p_entity)
{
  U32 unicode = 0;

  if (p_entity && *p_entity == '&' && *(p_entity + 1) == '#') {
    const char *p = p_entity + 2;
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

PARAM_REC *AppendParam(PARAM_REC * curr_list, U32 p_ID, U32 p_type,
                       const char *zdata)
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

char *AppendStr2HeapStr(char *zheap_str, U32 & buffer_ln,
                        const char *z_append_str)
{
  char *rv = zheap_str;

  if (z_append_str && *z_append_str) {
    U32 curr_ln = 0;
    if (zheap_str)
      curr_ln += strlen(zheap_str);
    size_t delta_ln = strlen(z_append_str);
    U32 bytes_needed = curr_ln + delta_ln + 1;

    bool OK = true;
    if (bytes_needed > buffer_ln) {
      buffer_ln = bytes_needed + 512;
      char *tmp = new char[buffer_ln];
      if (tmp) {
        rv = tmp;
        if (zheap_str) {
          strcpy(rv, zheap_str);
          delete[] zheap_str;
        } else {
          rv[0] = 0;
        }
      } else {
        TCI_ASSERT(0);
        OK = false;
      }
    }

    if (OK) {
      if (zheap_str)
        strcat(rv, z_append_str);
      else
        strcpy(rv, z_append_str);
    }
  }

  return rv;
}

INPUT_NOTATION_REC *CreateNotationRec()
{
  INPUT_NOTATION_REC *new_struct = new INPUT_NOTATION_REC();

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

SEMANTICS_NODE *AppendSLists(SEMANTICS_NODE * s_list,
                             SEMANTICS_NODE * new_tail)
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

SEMANTICS_NODE *NestInPGroup(SEMANTICS_NODE * s_list,
                             BUCKET_REC * parent_bucket)
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

SEMANTICS_NODE *PrefixToInfix(SEMANTICS_NODE * s_list)
{
  SEMANTICS_NODE *rv = s_list;

  SEMANTICS_NODE *s_rover = s_list;
  while (s_rover) {
    SEMANTICS_NODE *save_next = s_rover->next;

    if (s_rover->bucket_list) {
      if (s_rover->semantic_type == SEM_TYP_INFIX_OP
          || s_rover->semantic_type == SEM_TYP_PREFIX_OP
          || s_rover->semantic_type == SEM_TYP_POSTFIX_OP) {
        BUCKET_REC *new_parent = s_rover->parent;
        SEMANTICS_NODE *l_anchor = s_rover->prev;
        SEMANTICS_NODE *r_anchor = s_rover->next;

        BUCKET_REC *b_node = s_rover->bucket_list;
        SEMANTICS_NODE *l_operand = NULL;
        SEMANTICS_NODE *r_operand = NULL;
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

        SEMANTICS_NODE *pg = NestInPGroup(new_list, new_parent);

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
        BUCKET_REC *b_node = s_rover->bucket_list;
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

char *NestInParens(char *z_expr, bool forced)
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

char *NestInBrackets(char *z_expr)
{
  char *rv = NULL;

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

char *RemovezStrParens(char *z_expr)
{
  char *rv = z_expr;

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

void FUSetPrefix(const char *src_tok, char *prefix, char *buffer)
{
  buffer[0] = 0;
  if (src_tok) {
    const char *token_ptr = strchr(src_tok, ':');
    if (token_ptr)
      token_ptr++;
    else
      token_ptr = src_tok;

    if (prefix && *prefix)
      strcpy(buffer, prefix);

    strcat(buffer, token_ptr);
  }
}

char *TNodeToStr(MNODE * mml_node, char *prefix, int indent)
{
  char *rv = NULL;
  char *buffer = NULL;
  U32 bln = 0;

  if (mml_node) {
    // make sure the prefix ends with ":"
    char l_prefix[32];
    l_prefix[0] = 0;
    if (prefix && *prefix) {
      size_t zln = strlen(prefix);
      if (zln < 32) {
        strcpy(l_prefix, prefix);
        if (prefix[zln - 1] != ':')
          strcat(l_prefix, ":");
      }
    }
    // form the indentation string
    char indent_str[128];
    int ii = 0;
    while (ii < indent && ii < 127)
      indent_str[ii++] = ' ';
    indent_str[ii] = 0;

    char element[80];
    char zzz[256];
    FUSetPrefix(mml_node->src_tok, l_prefix, element);
    sprintf(zzz, "%s<%s", indent_str, element);
    buffer = AppendStr2HeapStr(buffer, bln, zzz);

    ATTRIB_REC *rover = mml_node->attrib_list;
    while (rover) {
      sprintf(zzz, " %s=\"%s\"", rover->zattr_nom, rover->zattr_val);
      buffer = AppendStr2HeapStr(buffer, bln, zzz);
      rover = rover->next;
    }

    if (mml_node->p_chdata && mml_node->first_kid) {
      TCI_ASSERT
        (!"I don't MML will ever allow both chdata and children on the same node");
      sprintf(zzz, " name=\"%s\">", mml_node->p_chdata);
      buffer = AppendStr2HeapStr(buffer, bln, zzz);

      buffer = AppendStr2HeapStr(buffer, bln, "\n");
      MNODE *t_list = mml_node->first_kid;
      while (t_list) {
        char *tmp = TNodeToStr(t_list, l_prefix, indent + 2);
        buffer = AppendStr2HeapStr(buffer, bln, tmp);
        delete[] tmp;
        t_list = t_list->next;
      }
      FUSetPrefix(mml_node->src_tok, l_prefix, element);
      sprintf(zzz, "%s</%s>\n", indent_str, element);
      buffer = AppendStr2HeapStr(buffer, bln, zzz);
    } else if (mml_node->p_chdata) {
      buffer = AppendStr2HeapStr(buffer, bln, ">");
      buffer = AppendStr2HeapStr(buffer, bln, mml_node->p_chdata);
      FUSetPrefix(mml_node->src_tok, l_prefix, element);
      sprintf(zzz, "</%s>\n", element);
      buffer = AppendStr2HeapStr(buffer, bln, zzz);
    } else if (mml_node->first_kid) {
      buffer = AppendStr2HeapStr(buffer, bln, ">\n");
      MNODE *t_list = mml_node->first_kid;
      while (t_list) {
        char *tmp = TNodeToStr(t_list, l_prefix, indent + 2);
        buffer = AppendStr2HeapStr(buffer, bln, tmp);
        delete[] tmp;
        t_list = t_list->next;
      }
      FUSetPrefix(mml_node->src_tok, l_prefix, element);
      sprintf(zzz, "%s</%s>\n", indent_str, element);
      buffer = AppendStr2HeapStr(buffer, bln, zzz);
    } else {
      if (!strcmp(mml_node->src_tok, "mspace")) {
        buffer = AppendStr2HeapStr(buffer, bln, ">\n");
      } else {
        FUSetPrefix(mml_node->src_tok, l_prefix, element);
        sprintf(zzz, "></%s>\n", element);
        buffer = AppendStr2HeapStr(buffer, bln, zzz);
      }
    }
  } else {
    TCI_ASSERT(0);
  }
  if (buffer) {
    size_t zln = strlen(buffer);
    rv = new char[zln + 1];
    strcpy(rv, buffer);
    delete[] buffer;
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
char *WideToASCII(const U16 * w_markup)
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
U16 *ASCIItoWide(const char *ascii, int &zlen)
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


char *DumpSNode(const SEMANTICS_NODE * s_node, int indent)
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

char *DumpSList(const SEMANTICS_NODE * s_list, int indent)
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

#include "Grammar.h"

bool IsTrigArgFuncName(Grammar *gmr, const char * nom)
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
