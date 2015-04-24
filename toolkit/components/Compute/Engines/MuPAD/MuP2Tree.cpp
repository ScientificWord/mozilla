// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

/*  Initial implementation December, 2003, JBM.

  This module was created as a copy of Mpl2Tree - It will require
  modifications as MuPAD return strings are not quite the same
  as Maple return strings.

  MuP2Tree inputs a zstring that contains MuPAD output.
  For example

(5*q2x^2 + 3)/(q2x + 2*q2x^2 + 7) + 1/(8*q2x^2 + 7)*(3*q2x + 3*q2x^2)
OR
"``*(8*q2x^2 + 7)^(-1)``*(q2x + 2*q2x^2 + 7)^(-1)``*(21*q2x + 83*q2x^2 + 9*q2x^3 + 46*q2x^4 + 21)"

  Its only task is to output a tree that reflects 1-to-1
  the math ojects in the MuPAD return.

  All nodes in the output tree are of one type (struct MNODE)
  defined in fltutils.h.
  FltUtils.cpp provides a diagnostic function that dumps
  the output tree to a file - DumpTList( tree );
*/

/*
  Note that swfcnc.mu can and does dress up MuPAD returns
    in some of our own boiler plate.
*/

#include "MuP2Tree.h"
#include "CmpTypes.h"
#include "strutils.h"
#include "dumputils.h"
#include <string.h>
#include <ctype.h>

MuPAD2Tree::MuPAD2Tree()
{
}

MuPAD2Tree::~MuPAD2Tree()
{
}

MNODE* MuPAD2Tree::MuPADStr2Tree(const char* src)
{

  #ifdef DEBUG
     JBM::JBMLine("\n===== MuPADStr2Tree =====\n");
     JBM::JBMLine(src);
  #endif
  MNODE *rv = NULL;

  if (src && *src) {
    start_ptr = src;
    src_line_number = 1;

    int advance = 0;            // a byte offset into src string
    rv = GetElementList(src, advance, NULL, NULL);
    //JBM::DumpTList(rv,0);
    rv = DoPrecedenceBinds(rv);
  }
  //JBM::DumpTList(rv,0);
    #ifdef DEBUG
     JBM::JBMLine("\n===== End MuPADStr2Tree =====\n");
     JBM::DumpTList(rv,0);
  #endif
  return rv;
}

MNODE *MuPAD2Tree::GetElementList(const char *z_src,
                                  int &advance, MNODE * parent, char *enders)
{
  MNODE *rv = NULL;
  MNODE *tail;

  int bytes_done = 0;
  int loop_advance = 0;
  const char *needle = z_src;

  // loop thru consecutive elements   "2*x"   etc.
  while (*needle) {
    loop_advance = 0;

    // Advance over whitespace to next element?
    while (*needle && *needle <= ' ') {
      if (*needle == '\n')
        src_line_number++;
      needle++;
      bytes_done++;
    }
    char tok = *needle;
    char next_tok = *(needle + 1);

    if (enders) {
      char *p = strchr(enders, tok);
      if (p) {
        bytes_done += loop_advance;
        break;
      }
    }
    MNODE *new_node = NULL;
    if (tok == 0) {
      break;
    } else if ((tok >= 'a' && tok <= 'z')
               || (tok >= 'A' && tok <= 'Z')
               || (tok == '`')) {
      char *name = NULL;
      int bytes_spanned;
      if (tok == '`') {
        name = GetQuotedString(needle, bytes_spanned);
      } else {
        name = GetName(needle, bytes_spanned);
      }

      loop_advance += bytes_spanned;
      needle += bytes_spanned;

      bytes_done += loop_advance;
      loop_advance = 0;

      while (*needle && *needle <= ' ') {
        if (*needle == '\n')
          src_line_number++;
        needle++;
        loop_advance++;
      }
      bytes_done += loop_advance;
      loop_advance = 0;

      bool done = false;
      if (IsOperatorName(name)) {
      } else if (*needle == '(') {  // arg list
        new_node = MakeTNode(needle - start_ptr, 0, src_line_number);
        new_node->parent = parent;
        new_node->p_chdata = name;

        int bytes_spanned = 0;

        if (!strcmp(name, "D")) { // derivative - special notation
          strcpy(new_node->src_tok, "derivative");
          //D(D(D(q1m)))(q1x)
          const char *l_needle = needle + 1;
          int n_primes = 1;
          while (*l_needle == 'D' && *(l_needle + 1) == '(') {
            n_primes++;
            l_needle += 2;
          }

          char buffer[32];
          sprintf(buffer, "%d", n_primes);
          size_t zln = strlen(buffer);
          char *tmp = new char[zln + 1];
          strcpy(tmp, buffer);
          delete[] name;
          new_node->p_chdata = tmp;

          // process the "(q1m)" - I'm treating it like a func arg here
          MNODE *args = GetDelimitedList(l_needle - 1, bytes_spanned, ')', 0,
                                         "arglist", "arg_bucket");
          bytes_spanned += 3 * (n_primes - 1);

          // Extract the part we want from the "arglist" created
          MNODE *varbucket = args->first_kid;
          MNODE *funcbucket = varbucket->next;
          MNODE *var  = varbucket->first_kid;
          MNODE *func = funcbucket->first_kid;
          varbucket->first_kid = NULL;
          DisposeTList(args);

          strcpy(func->src_tok, "function");
          func->parent = new_node;
          new_node->first_kid = func;

          if (*(needle + bytes_spanned) == '(') {
            int adv = 0;
            MNODE *arg_val =
              GetDelimitedList(needle + bytes_spanned, adv, ')', 0,
                               "arglist", "arg_bucket");
            arg_val->parent = func;
            func->first_kid = arg_val;
            loop_advance += adv;
          } else
            TCI_ASSERT(0);
        } else {
          strcpy(new_node->src_tok, "function");
          MNODE *args = GetDelimitedList(needle, bytes_spanned, ')', 0,
                                         "arglist", "arg_bucket");
          args->parent = new_node;
          new_node->first_kid = args;
        }
        loop_advance += bytes_spanned;
        done = true;
      } else if (*needle == '[') {  // subscripted variable
        new_node = MakeTNode(needle - start_ptr, 0, src_line_number);
        new_node->parent = parent;
        strcpy(new_node->src_tok, "variable");

        new_node->p_chdata = name;

        int bytes_spanned = 0;
        MNODE *args = GetDelimitedList(needle, bytes_spanned, ']', 0,
                                       "qlist", "q_bucket");
        args->parent = new_node;
        new_node->first_kid = args;
        loop_advance += bytes_spanned;
        done = true;
      }
      if (!done) {
        new_node = MakeTNode(needle - start_ptr, 0, src_line_number);
        new_node->parent = parent;

        if (!strcmp(name, "TRUE"))
          strcpy(new_node->src_tok, "Boolean");
        else if (!strcmp(name, "FALSE"))
          strcpy(new_node->src_tok, "Boolean");
        else if (!strcmp(name, "undefined"))
          strcpy(new_node->src_tok, "constant");
        else {
          if ((tok >= 'A' && tok <= 'Z')
              || tok == '_')
            strcpy(new_node->src_tok, "constant");
          else {
            if (IsOperatorName(name))
              strcpy(new_node->src_tok, "operator");
            else
              strcpy(new_node->src_tok, "variable");
          }
        }
        new_node->p_chdata = name;
      }
    } else if (tok == '"') {
      int bytes_spanned = 0;
      new_node = GetString(needle, bytes_spanned);
      new_node->parent = parent;
      loop_advance += bytes_spanned;
    } else if (tok >= '0' && tok <= '9') {
      int bytes_spanned = 0;
      new_node = GetNumber(needle, bytes_spanned);
      new_node->parent = parent;
      loop_advance += bytes_spanned;
    } else if (tok == '.' && next_tok >= '0' && next_tok <= '9') {
      int bytes_spanned = 0;
      new_node = GetNumber(needle, bytes_spanned);
      new_node->parent = parent;
      loop_advance += bytes_spanned;
    } else {
      switch (tok) {
      case '+':
      case '-':
      case '/':
      case '^':{
          new_node = GetOperator(needle, 1);
          loop_advance++;
        }
        break;
      case '*':{               // &* non-commutative multiple
          if (next_tok == '*') {
            new_node = GetOperator(needle, 2);
            loop_advance += 2;
          } else {
            new_node = GetOperator(needle, 1);
            loop_advance++;
          }
        }
        break;
      case '&':{               // &* non-commutative multiple
          if (next_tok == '*') {
            loop_advance += 2;
          } else {              // neutral operator - &string
            loop_advance++;
          }
        }
        break;
      case '.':{
          if (next_tok == '.') {  // range or ellipsis
            if (*(needle + 2) == '.') {
              new_node = GetOperator(needle, 3);
              strcpy(new_node->src_tok, "constant");
              loop_advance += 3;
            } else {
              new_node = GetOperator(needle, 2);
              loop_advance += 2;
            }
          } else {              // concatenation
            loop_advance++;
          }
        }
        break;
      case ',':{
          // this silly syntax is generated swfcns.mu
          // - joins the whole and fractional parts of a mixed number
          if (!strncmp(needle, ", \"#\",", 6)) {
            new_node = GetOperator(needle, 6);
            loop_advance += 6;
          } else {
            new_node = GetOperator(needle, 1);
            loop_advance++;
          }
        }
        break;
      case '|':{
          new_node = GetOperator(needle, 1);
          loop_advance++;
        }
        break;
      case '!':{
          new_node = GetOperator(needle, 1);
          loop_advance++;
        }
        break;
      case '#':{
          TCI_ASSERT(0);
          new_node = GetOperator(needle, 1);
          loop_advance++;
        }
        break;
      case ':':{
          if (next_tok == '=') {  // assignment
            new_node = GetOperator(needle, 2);
            loop_advance += 2;
          } else if (next_tok == ':') { // ::
            new_node = GetOperator(needle, 2);
            loop_advance += 2;
          } else {              // statement separator
            TCI_ASSERT(0);
            loop_advance++;
          }
        }
        break;
      case '=':{               // equality relational
          new_node = GetOperator(needle, 1);
          loop_advance++;
        }
        break;
      case '<':{
          if (next_tok == '>') {  // does not equal
            new_node = GetOperator(needle, 2);
            loop_advance += 2;
          } else if (next_tok == '=') { // less than or equal
            new_node = GetOperator(needle, 2);
            loop_advance += 2;
          } else {              // less than
            new_node = GetOperator(needle, 1);
            loop_advance++;
          }
        }
        break;
      case '>':{
          if (next_tok == '=') {  // greater than or equal to
            new_node = GetOperator(needle, 2);
            loop_advance += 2;
          } else {              // greater than
            new_node = GetOperator(needle, 1);
            loop_advance++;
          }
        }
        break;
      case '\\':{              // continuation
          loop_advance++;
        }
        break;
      case '@':{
          if (next_tok == '@') {  // repeated composition (\ring)
            new_node = GetOperator(needle, 2);
            loop_advance += 2;
          } else {              // function composition (\ring)
            new_node = GetOperator(needle, 1);
            loop_advance++;
          }
        }
        break;
      case '(':{
          int bytes_spanned = 0;
          new_node = GetParenedGroup(needle, bytes_spanned);
          new_node->parent = parent;
          loop_advance += bytes_spanned;
        }
        break;
      case '[':{
          int bytes_spanned = 0;
          new_node = GetDelimitedList(needle, bytes_spanned, ']', ')',
                                      "blist", "item_bucket");
          new_node->parent = parent;
          loop_advance += bytes_spanned;
        }
        break;
      case '{':{
          int bytes_spanned = 0;
          new_node = GetDelimitedList(needle, bytes_spanned, '}', 0,
                                      "set", "set_element_bucket");
          new_node->parent = parent;
          loop_advance += bytes_spanned;
        }
        break;
      case '`':
        TCI_ASSERT(0);
        break;
      default:{
          TCI_ASSERT(0);
          loop_advance++;
        }
        break;
      }
    }
    if (new_node) {
      if (rv) {
        tail->next = new_node;
        new_node->prev = tail;
      } else
        rv = new_node;
      tail = new_node;
      while (tail->next)
        tail = tail->next;
    }
    needle += loop_advance;
    bytes_done += loop_advance;
  }
  advance = bytes_done;
  return rv;
}

MNODE *MuPAD2Tree::GetNumber(const char *z_src, int &advance)
{
  MNODE *rv = NULL;

  bool got_decimal = false;
  const char *needle = z_src;
  while (*needle) {
    char ch = *needle;
    if (ch >= '0' && ch <= '9')
      needle++;
    else if (ch == '.') {
      if (got_decimal || *(needle + 1) == '.')
        break;
      got_decimal = true;
      needle++;
    } else if (ch == 'e') {
      needle++;
      if (*needle == '-')
        needle++;
    } else
      break;
  }
  int data_len = needle - z_src;
  if (data_len > 0) {
    rv = MakeTNode(z_src - start_ptr, data_len, src_line_number);
    strcpy(rv->src_tok, "number");

    char *tmp = new char[data_len + 1];
    strncpy(tmp, z_src, data_len);
    tmp[data_len] = 0;
    rv->p_chdata = tmp;
  }
  advance = data_len;
  return rv;
}

MNODE *MuPAD2Tree::GetOperator(const char *z_src, U32 n_chars)
{
  MNODE *rv = MakeTNode(z_src - start_ptr, n_chars, src_line_number);
  strcpy(rv->src_tok, "operator");

  char *tmp = new char[n_chars + 1];
  if (!strncmp(z_src, ", \"#\",", 6))
    strncpy(tmp, "mnjoin", n_chars);
  else
    strncpy(tmp, z_src, n_chars);
  tmp[n_chars] = 0;
  rv->p_chdata = tmp;

  return rv;
}

MNODE *MuPAD2Tree::GetString(const char *z_src, int &advance)
{
  MNODE *rv = NULL;

  int tally = 0;
  const char *needle = z_src;
  while (*needle) {
    if (*needle == '"') {
      tally++;
      if (tally == 2)
        break;
    }
    needle++;
  }
  int data_len = needle - z_src + 1;
  if (data_len > 0) {
    rv = MakeTNode(z_src - start_ptr, data_len, src_line_number);
    strcpy(rv->src_tok, "string");

    char *tmp = new char[data_len + 1];
    strncpy(tmp, z_src, data_len);
    tmp[data_len] = 0;
    rv->p_chdata = tmp;
  }
  advance = data_len;
  return rv;
}

char *MuPAD2Tree::GetName(const char *z_src, int &bytes_spanned)
{
  char *rv = NULL;
  bytes_spanned = 0;

  const char *ptr = z_src;

  bool is_constant;
  if (*ptr >= 'A' && *ptr <= 'Z')
    is_constant = true;
  else
    is_constant = false;

  while (*ptr) {
    char ch = *ptr;
    if (ch >= 'a' && ch <= 'z') {
      ptr++;
    } else if (ch >= 'A' && ch <= 'Z') {
      ptr++;
    } else if (ch >= '0' && ch <= '9') {
      ptr++;
    } else if (ch == '_' && is_constant) {
      ptr++;
    } else if (ch == ':' && *(ptr + 1) == ':') {  // transform::laplace
      ptr += 2;
    } else
      break;
  }
  bytes_spanned = ptr - z_src;
  rv = new char[bytes_spanned + 1];
  strncpy(rv, z_src, bytes_spanned);
  rv[bytes_spanned] = 0;

  return rv;
}

char *MuPAD2Tree::GetQuotedString(const char *z_src, int &bytes_spanned)
{
  char *rv = NULL;
  bytes_spanned = 0;

  const char *ptr = z_src + 1;  // step over opening '`'
  while (*ptr) {
    if (*ptr == '`') {
      if (*(ptr + 1) == '`')
        ptr++;
      else {
        bytes_spanned = ptr - z_src + 1;

        TCI_ASSERT(bytes_spanned == 2);

        rv = new char[bytes_spanned + 1];
        strncpy(rv, z_src, bytes_spanned);
        rv[bytes_spanned] = 0;
        break;
      }
    }
    ptr++;
  }
  return rv;
}

MNODE *MuPAD2Tree::GetDelimitedList(const char *z_src, int &bytes_done,
                                    char ender, char ender2,
                                    char *group_ilk, char *bucket_ilk)
{
  MNODE *rv = MakeTNode(z_src - start_ptr, 0, src_line_number);
  strcpy(rv->src_tok, group_ilk);

  MNODE *child_list = NULL;
  MNODE *tail;

  bool done = false;
  const char *needle = z_src + 1;
  while (*needle && !done) {
    while (*needle && *needle <= ' ')
      needle++;
    if (*needle) {
      MNODE *child_bucket = MakeTNode(needle - start_ptr, 0, src_line_number);
      strcpy(child_bucket->src_tok, bucket_ilk);

      char ch = *needle;
      if (ch == ender || ch == ender2) {
        done = true;
        needle++;
      } else if (ch == ',') {
        needle++;
      } else {
        char enders[4];
        enders[0] = ',';
        enders[1] = ender;
        if (ender2) {
          enders[2] = ender2;
          enders[3] = 0;
        } else
          enders[2] = 0;
        int ln_entry;
        MNODE *contents = GetElementList(needle, ln_entry, rv, enders);
        contents->parent = child_bucket;
        child_bucket->first_kid = contents;

        needle += ln_entry;
        if (*needle == ',')
          needle++;
        else if (*needle == ender || *needle == ender2) {
          needle++;
          done = true;
        }
      }
      if (child_bucket) {
        if (child_list) {
          child_bucket->prev = tail;
          tail->next = child_bucket;
        } else {
          child_list = child_bucket;
        }
        tail = child_bucket;
      }
    }
  }
  if (child_list) {
    child_list->parent = rv;
    rv->first_kid = child_list;
  }
  bytes_done = needle - z_src;
  rv->src_length = bytes_done;
  return rv;
}

MNODE *MuPAD2Tree::GetParenedGroup(const char *z_src, int &bytes_done)
{
  MNODE *rv = MakeTNode(z_src - start_ptr, 0, src_line_number);
  strcpy(rv->src_tok, "pgroup");

  MNODE *contents = NULL;

  const char *needle = z_src + 1;
  while (*needle && *needle <= ' ')
    needle++;

  if (*needle) {
    char ch = *needle;
    if (ch == ')') {
      needle++;
    } else {
      char enders[3];
      enders[0] = ')';
      enders[1] = ']';
      enders[2] = 0;
      int ln_entry;
      contents = GetElementList(needle, ln_entry, rv, enders);

      needle += ln_entry;
      if (*needle == ')' || *needle == ']') {
        needle++;
      }
    }
  }
  if (contents) {
    contents->parent = rv;
    rv->first_kid = contents;
  }
  bytes_done = needle - z_src;
  rv->src_length = bytes_done;
  return rv;
}

// Traverse "src_list", bind operators with precedence "targ_prec"
//  to their operands.  "op_type" 1 - prefix, 2 - infix, 3 - postfix

MNODE *MuPAD2Tree::BindOperators(MNODE * src_list, int targ_prec, OpIlk op_type)
{
  MNODE *rv = src_list;

  int highest_prec = 0;         // strawman
  int lowest_prec = 100;        // strawman
  int op_tally = 0;
  MNODE *rover = src_list;
  while (rover) {
    if (!strcmp(rover->src_tok, "operator")) {
      op_tally++;
      int curr_prec = GetPrecedence(rover->p_chdata);
      if (lowest_prec > curr_prec)
        lowest_prec = curr_prec;
      if (highest_prec < curr_prec)
        highest_prec = curr_prec;
    }
    rover = rover->next;
  }
  rover = src_list;
  while (rover) {
    if (!strcmp(rover->src_tok, "operator")) {
      if (op_tally > 1 && highest_prec != lowest_prec) {
        int curr_prec = GetPrecedence(rover->p_chdata);
        if (curr_prec == targ_prec) {
          bool fix_rv = false;
          if (rover == rv)
            fix_rv = true;
          else if (rover->prev && rover->prev == rv)
            fix_rv = true;

          MNODE *tmp = BindOp(rover, targ_prec, op_type);
          if (fix_rv)
            rv = tmp;
          rover = tmp;
        }
      }
    } else if (rover->first_kid)
      BindOperators(rover->first_kid, targ_prec, op_type);
    if (rover)
      rover = rover->next;
  }
  return rv;
}

MNODE *MuPAD2Tree::BindUnits(MNODE * src_tree)
{
  MNODE *rv = src_tree;

  MNODE *rover = src_tree;
  while (rover) {
    if (!strcmp(rover->src_tok, "operator")
        && rover->p_chdata) {
      const char *zop_str = rover->p_chdata;
      size_t zop_ln = strlen(zop_str);
      if (zop_ln == 1) {
        if (zop_str[0] == '*' || zop_str[0] == '/') {
          if (IsUnit(rover->prev) && IsUnit(rover->next)) {
            bool fix_rv = false;
            if (rover->prev && rover->prev == rv)
              fix_rv = true;
            MNODE *tmp = BindOp(rover, 28, OP_infix);
            if (fix_rv)
              rv = tmp;
            rover = tmp;
          }
        }
      }
    } else if (rover->first_kid)
      BindUnits(rover->first_kid);
    if (rover)
      rover = rover->next;
  }
  return rv;
}

int MuPAD2Tree::GetPrecedence(const char *targ_op)
{
  int rv = 0;

  size_t zln = strlen(targ_op);
  if (zln == 1) {
    char ch0 = targ_op[0];
    if (ch0 == '!')
      rv = 30;
    else if (ch0 == '^')
      rv = 29;
    else if (ch0 == '*')
      rv = 28;
    else if (ch0 == '/')
      rv = 28;
    else if (ch0 == '@')
      rv = 28;
    else if (ch0 == '+')
      rv = 27;
    else if (ch0 == '-')
      rv = 27;
    else if (ch0 == '<')
      rv = 24;
    else if (ch0 == '>')
      rv = 24;
    else if (ch0 == '=')
      rv = 24;
    else if (ch0 == '$')
      rv = 23;
    else if (ch0 == ',')
      rv = 18;
    else if (ch0 == '|')
      rv = 18;
    else
      TCI_ASSERT(0);
  } else if (zln == 2) {
    char ch0 = targ_op[0];
    char ch1 = targ_op[1];

    if (ch0 == ':' && ch1 == ':')
      rv = 30;
    else if (ch0 == '*' && ch1 == '*')
      rv = 29;
    else if (ch0 == '@' && ch1 == '@')
      rv = 29;
    else if (ch0 == '&' && ch1 == '*')
      rv = 28;
    else if (ch0 == '.' && ch1 == '.')
      rv = 25;
    else if (ch0 == 'i' && ch1 == 'n')
      rv = 25;
    else if (ch0 == '<' && ch1 == '=')
      rv = 24;
    else if (ch0 == '>' && ch1 == '=')
      rv = 24;
    else if (ch0 == '<' && ch1 == '>')
      rv = 24;
    else if (ch0 == 'o' && ch1 == 'r')
      rv = 20;
    else if (ch0 == '-' && ch1 == '>')
      rv = 19;
    else if (ch0 == ':' && ch1 == '=')
      rv = 17;
    else
      TCI_ASSERT(0);
  } else {
    if (!strcmp(targ_op, "intersect"))
      rv = 28;
    else if (!strcmp(targ_op, "union"))
      rv = 27;
    else if (!strcmp(targ_op, "minus"))
      rv = 27;
    else if (!strcmp(targ_op, "mod"))
      rv = 26;
    else if (!strcmp(targ_op, "not"))
      rv = 22;
    else if (!strcmp(targ_op, "and"))
      rv = 21;
    else if (!strcmp(targ_op, "or"))
      rv = 20;
    else if (!strcmp(targ_op, "mnjoin"))  // mixed number join
      rv = 27;
    else
      TCI_ASSERT(0);
  }
  return rv;
}

MNODE *MuPAD2Tree::BindOp(MNODE * op_node, int targ_prec, OpIlk op_type)
{
  MNODE *rv = NULL;

  // In certain contexts, "+" and "-" are unary ( ie. prefix operators )
  if (op_type == OP_infix && targ_prec == 27) {
    MNODE *left_operand = op_node->prev;
    if (left_operand) {
      if (!strcmp(left_operand->src_tok, "operator"))
        op_type = OP_prefix;
    } else
      op_type = OP_prefix;
  }
  if ((op_type > 1 && op_node->prev)  // left required and present
      || (op_type < 3 && op_node->next)) {  // right required and present
    MNODE *left_operand = (op_type != OP_prefix) ? op_node->prev : NULL;
    MNODE *right_operand = (op_type != OP_postfix) ? op_node->next : NULL;

    if (op_type != OP_prefix) {
      if (left_operand && left_operand->first_kid)
        BindOperators(left_operand->first_kid, targ_prec, op_type);
    }
    if (op_type != OP_postfix) {
      if (right_operand && right_operand->first_kid)
        BindOperators(right_operand->first_kid, targ_prec, op_type);
    }
    MNODE *left_anchor = left_operand ? left_operand->prev : op_node->prev;
    MNODE *right_anchor = right_operand ? right_operand->next : op_node->next;

    // Gather a list like "a + b - c +1" into a single group
    bool go_on = true;
    while (go_on) {
      go_on = false;
      if (right_operand
          && right_anchor
          && right_anchor->next
          && op_type == 2 && !strcmp(right_anchor->src_tok, "operator")) {
        int rop_prec = GetPrecedence(right_anchor->p_chdata);
        if (rop_prec == targ_prec) {
          right_operand = right_anchor->next;
          right_anchor = right_operand->next;
          BindOperators(right_operand->first_kid, targ_prec, op_type);
          go_on = true;
        }
      }
    }
    MNODE *parent =
      left_anchor ? NULL : (left_operand ? left_operand->parent : op_node->
                            parent);
    rv = MakeTNode(0, 0, 0);
    strcpy(rv->src_tok, "precedence_group");

    char buffer[32];
    StrFromInt(targ_prec, buffer);
    size_t nln = strlen(buffer);
    char *tmp = new char[nln + 1];
    strcpy(tmp, buffer);
    rv->p_chdata = tmp;

    if (op_type == OP_prefix || op_type == OP_infix && !left_operand) {
      rv->first_kid = op_node;
      op_node->parent = rv;
      op_node->prev = NULL;
    } else {
      rv->first_kid = left_operand;
      left_operand->parent = rv;
      left_operand->prev = NULL;
    }
    if (right_operand)
      right_operand->next = NULL;
    else
      op_node->next = NULL;
    if (left_anchor) {
      left_anchor->next = rv;
      rv->prev = left_anchor;
    } else {
      rv->parent = parent;
      if (parent)
        parent->first_kid = rv;
    }
    if (right_anchor) {
      right_anchor->prev = rv;
      rv->next = right_anchor;
    }
  } else {
    TCI_ASSERT(0);
    // Here, there's no operand to bind!!
  }
  return rv;
}

MNODE *MuPAD2Tree::DoPrecedenceBinds(MNODE * src_list)
{
  MNODE *rv = src_list;

  // Operator precedence bindings
  rv = BindOperators(rv, 30, OP_postfix);  //"!"
  rv = BindOperators(rv, 29, OP_infix);  //"^" "**" "@@" 
  rv = BindUnits(rv);
  rv = BindOperators(rv, 28, OP_infix);  //"*" "&*" "/" "@" "intersect"

  rv = BindOperators(rv, 27, OP_infix);  //"+" "-" "union" "minus"
  rv = BindOperators(rv, 26, OP_infix);  //"mod"
  rv = BindOperators(rv, 25, OP_infix);  //".."
  rv = BindOperators(rv, 24, OP_infix);  //"<" "<=" ">" ">=" "=" "<>"
  rv = BindOperators(rv, 23, OP_infix);  //"$"
  rv = BindOperators(rv, 22, OP_prefix);  //"not"
  rv = BindOperators(rv, 21, OP_infix);  //"and"
  rv = BindOperators(rv, 20, OP_infix);  //"or"
  rv = BindOperators(rv, 19, OP_infix);  //"->"
  rv = BindOperators(rv, 18, OP_infix);  //","
  rv = BindOperators(rv, 17, OP_infix);  //":="

  //JBM::DumpTList(rv, 0);
  return rv;
}

bool MuPAD2Tree::IsUnit(MNODE * src_node)
{
  bool rv = false;

  if (src_node && src_node->p_chdata) {
    const char *z_nom = src_node->p_chdata;
    size_t zln = strlen(z_nom);
    if (zln > 2 && !strcmp(z_nom + zln - 2, "QU"))
      rv = true;
  } else {
    if (!strcmp(src_node->src_tok, "precedence_group")) {
      rv = IsUnit(src_node->first_kid);
    }
  }
  return rv;
}

bool MuPAD2Tree::IsOperatorName(const char *nom)
{
  bool rv = false;

  if (nom && nom[0]) {
    size_t zln = strlen(nom);
    switch (zln) {
    case 2:
      if (!strcmp(nom, "in"))
        rv = true;
      else if (!strcmp(nom, "or"))
        rv = true;
      break;
    case 3:
      if (!strcmp(nom, "mod"))
        rv = true;
      else if (!strcmp(nom, "not"))
        rv = true;
      else if (!strcmp(nom, "and"))
        rv = true;
      break;
    case 5:
      if (!strcmp(nom, "union"))
        rv = true;
      else if (!strcmp(nom, "minus"))
        rv = true;
      break;
    case 9:
      if (!strcmp(nom, "intersect"))
        rv = true;
      break;
    default:
      break;
    }
  }
  return rv;
}
