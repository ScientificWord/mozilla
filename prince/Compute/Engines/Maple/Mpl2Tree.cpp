// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

/*  Initial implementation July 23, 2003, JBM.

  Mpl2Tree inputs a zstring that contains a Maple output string.
  For example
                 "matrix([[q1x, 1, 2]])"

                  eval(surd(q1x,3)+sqrt(2));
                  <<surd(q1x,3)+2^(1/2)

  Its only task is to output a tree that reflects 1-to-1
  the math ojects in the Maple stream.

  All nodes in the output tree are of one type (struct MNODE)
  defined in fltutils.h.
  FltUtils.cpp provides a diagnostic function that dumps
  the output tree to a file - DumpTList( tree );
*/

/*
  ( I hope to parse a few Maple constructs on the first pass
    at this code -  Finish later. )
*/

#include "Mpl2Tree.h"
#include "../../CmpTypes.h"
#include <string.h>
#include <ctype.h>

Maple2Tree::Maple2Tree()
{
}

Maple2Tree::~Maple2Tree()
{
}

MNODE *Maple2Tree::MapleStr2Tree(const char *src)
{
  MNODE *rv = NULL;

  if (src && *src) {
    if (!strncmp(src, "proc ", 5)) {
      TCI_ASSERT(0);
    } else {
      start_ptr = src;
      src_line_number = 1;

      int advance = 0;          // a byte offset into src string
      rv = GetElementList(src, advance, NULL, NULL);
      rv = DoPrecedenceBinds(rv);
    }
  }

  return rv;
}

MNODE *Maple2Tree::GetElementList(const char *z_src,
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
               || (tok == '_' && next_tok >= 'A' && next_tok <= 'Z')
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

      if (*needle == '(') {     // arg list
        new_node = MakeTNode(needle - start_ptr, 0, src_line_number);
        new_node->parent = parent;
        strcpy(new_node->src_tok, "function");

        new_node->p_chdata = name;

        int bytes_spanned = 0;
        MNODE *args = GetDelimitedList(needle, bytes_spanned, ')',
                                       "arglist", "arg_bucket");

        if (!strcmp(name, "D")) { // derivative
          strcpy(new_node->src_tok, "derivative");

          MNODE *bucket = args->first_kid;
          MNODE *func = bucket->first_kid;
          bucket->first_kid = NULL;
          DisposeTList(args);

          strcpy(func->src_tok, "function");
          func->parent = new_node;
          new_node->first_kid = func;

          if (*(needle + bytes_spanned) == '(') {
            int adv = 0;
            MNODE *arg_val =
              GetDelimitedList(needle + bytes_spanned, adv, ')',
                               "arglist", "arg_bucket");
            arg_val->parent = func;
            func->first_kid = arg_val;
            loop_advance += adv;
          } else
            TCI_ASSERT(0);
        } else {
          args->parent = new_node;
          new_node->first_kid = args;
        }
        loop_advance += bytes_spanned;
      } else if (*needle == '[') {  // subscripted variable
        new_node = MakeTNode(needle - start_ptr, 0, src_line_number);
        new_node->parent = parent;
        strcpy(new_node->src_tok, "variable");

        new_node->p_chdata = name;

        int bytes_spanned = 0;
        MNODE *args = GetDelimitedList(needle, bytes_spanned, ']',
                                       "qlist", "q_bucket");
        args->parent = new_node;
        new_node->first_kid = args;
        loop_advance += bytes_spanned;
      } else {
        new_node = MakeTNode(needle - start_ptr, 0, src_line_number);
        new_node->parent = parent;

        if (!strcmp(name, "true"))
          strcpy(new_node->src_tok, "Boolean");
        else if (!strcmp(name, "false"))
          strcpy(new_node->src_tok, "Boolean");
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
            new_node = GetOperator(needle, 2);
            loop_advance += 2;
          } else {              // concatenation
            loop_advance++;
          }
        }
        break;

      case ',':{
          // this silly syntax is generated swfcns.mu
          // - joins the whole and fractional parts of a mixed number
          if (!strncmp(needle, ", `#`,", 6)) {
            new_node = GetOperator(needle, 6);
            loop_advance += 6;
          } else {
            new_node = GetOperator(needle, 1);
            loop_advance++;
          }
        }
        break;

      case '!':{
          new_node = GetOperator(needle, 1);
          loop_advance++;
        }
        break;

      case ':':{
          if (next_tok == '=') {  // assignment
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
          new_node = GetDelimitedList(needle, bytes_spanned, ']',
                                      "blist", "item_bucket");
          new_node->parent = parent;
          loop_advance += bytes_spanned;
        }
        break;

      case '{':{
          int bytes_spanned = 0;
          new_node = GetDelimitedList(needle, bytes_spanned, '}',
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
    }
    needle += loop_advance;
    bytes_done += loop_advance;
  }
  advance = bytes_done;
  return rv;
}

MNODE *Maple2Tree::GetNumber(const char *z_src, int &advance)
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

MNODE *Maple2Tree::GetOperator(const char *z_src, U32 n_chars)
{
  MNODE *rv = MakeTNode(z_src - start_ptr, n_chars, src_line_number);
  strcpy(rv->src_tok, "operator");

  char *tmp = new char[n_chars + 1];
  if (!strncmp(z_src, ", `#`,", 6))
    strncpy(tmp, "mnjoin", n_chars);
  else
    strncpy(tmp, z_src, n_chars);
  tmp[n_chars] = 0;
  rv->p_chdata = tmp;

  return rv;
}

char *Maple2Tree::GetName(const char *z_src, int &bytes_spanned)
{
  char *rv = NULL;
  bytes_spanned = 0;

  const char *ptr = z_src;

  if (*ptr == '_')
    ptr++;

  while (*ptr) {
    char ch = *ptr;
    if (ch >= 'a' && ch <= 'z') {
      ptr++;
    } else if (ch >= 'A' && ch <= 'Z') {
      ptr++;
    } else if (ch >= '0' && ch <= '9') {
      ptr++;
    } else
      break;
  }

  bytes_spanned = ptr - z_src;
  rv = new char[bytes_spanned + 1];
  strncpy(rv, z_src, bytes_spanned);
  rv[bytes_spanned] = 0;

  return rv;
}

char *Maple2Tree::GetQuotedString(const char *z_src, int &bytes_spanned)
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

MNODE *Maple2Tree::GetDelimitedList(const char *z_src, int &bytes_done,
                                    char ender, char *group_ilk,
                                    char *bucket_ilk)
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
      if (ch == ender) {
        done = true;
        needle++;
      } else if (ch == ',') {
        needle++;
      } else {
        char enders[3];
        enders[0] = ',';
        enders[1] = ender;
        enders[2] = 0;
        int ln_entry;
        MNODE *contents = GetElementList(needle, ln_entry, rv, enders);
        contents->parent = child_bucket;
        child_bucket->first_kid = contents;

        needle += ln_entry;
        if (*needle == ',')
          needle++;
        else if (*needle == ender) {
          needle++;
          done = true;
        }
      }

      if (child_bucket) {
        if (child_list) {
          child_bucket->prev = tail;
          tail->next = child_bucket;
        } else
          child_list = child_bucket;
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

MNODE *Maple2Tree::GetParenedGroup(const char *z_src, int &bytes_done)
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
      char enders[2];
      enders[0] = ')';
      enders[1] = 0;
      int ln_entry;
      contents = GetElementList(needle, ln_entry, rv, enders);
      needle += ln_entry;
      if (*needle == ')') {
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

MNODE *Maple2Tree::BindOperators(MNODE * src_tree, int targ_prec, OpIlk op_type)
{
  MNODE *rv = src_tree;

  int highest_prec = 0;
  int lowest_prec = 100;
  int op_tally = 0;
  MNODE *rover = src_tree;
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

  rover = src_tree;
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

MNODE *Maple2Tree::BindUnits(MNODE * src_tree)
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

int Maple2Tree::GetPrecedence(const char *targ_op)
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

    if (ch0 == '*' && ch1 == '*')
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
    if (!strcmp(targ_op, "intersection"))
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
    else if (!strcmp(targ_op, "mnjoin"))
      rv = 27;
    else
      TCI_ASSERT(0);
  }

  return rv;
}

MNODE *Maple2Tree::BindOp(MNODE * op_node, int targ_prec, OpIlk op_type)
{
  MNODE *rv = NULL;

  // "+" and "-" may be unary
  if (op_type == OP_infix && targ_prec == 27) {
    MNODE *left_operand = op_node->prev;
    if (left_operand) {
      if (!strcmp(left_operand->src_tok, "operator"))
        op_type = OP_prefix;
    } else
      op_type = OP_prefix;
  }

  if ((op_type != OP_prefix && op_node->prev)
      || (op_type != OP_postfix && op_node->next)) {
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
          && op_type == OP_infix && !strcmp(right_anchor->src_tok, "operator")) {
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
  }
  return rv;
}

MNODE *Maple2Tree::DoPrecedenceBinds(MNODE * src_list)
{
  MNODE *rv = src_list;

  // Operator precedence bindings
  rv = BindOperators(rv, 30, OP_postfix);  //"!"
  rv = BindOperators(rv, 29, OP_infix);  //"^" "**" "@@" 
  rv = BindUnits(rv);
  rv = BindOperators(rv, 28, OP_infix);  //"*" "&*" "/" "@" "intersection"
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

  JBM::DumpTList(rv, 0);
  return rv;
}

bool Maple2Tree::IsUnit(MNODE * src_node)
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

bool Maple2Tree::IsOperatorName(char *nom)
{
  bool rv = false;

  if (nom && nom[0]) {
    size_t zln = strlen(nom);
    switch (zln) {
    case 2:
      if (!strcmp(nom, "in"))
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
    case 12:
      if (!strcmp(nom, "intersection"))
        rv = true;
      break;
    default:
      break;
    }
  }

  return rv;
}
