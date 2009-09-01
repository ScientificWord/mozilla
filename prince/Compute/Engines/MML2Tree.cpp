// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

#include "MML2Tree.h"
#include "CmpTypes.h"
#include "attriblist.h"
#include <string.h>
#include <ctype.h>

/*  Initial implementation June 10, 2003, JBM.

  MML2Tree inputs a zstring that contains a presentation MathML.
  For example
" <mml:math attrnom="attr_val">\n  <mml:mrow>\n<mml:mi>x</mml:mi>  </mml:mrow> </mml:math>"

  Its only task is to output a tree that reflects 1-to-1 the nesting
  the relationships, chdata and attributes of all elements in the MathML.
  White space between elements is discarded as it is not semantically
  relevent to clients.

       1 <math>
        attrnom="attr_val"
       2   <mrow>
       3     <mi>
x
       3     </mi>
       2   </mrow>
       1 </math>

  To construct a simple DOM from this tree, embed it in an object
   that provides some access functions thru its interface.

  All nodes in the output tree are of one type (struct MNODE)
  defined in mnode.h.
  FltUtils.cpp provides a diagnostic function that dumps
  the output tree to a file - DumpTList( tree );
*/

MML2Tree::MML2Tree()
{
}

MML2Tree::~MML2Tree()
{
}

MNODE* MML2Tree::MMLstr2Tree(const char* src)
{
  MNODE* rv = NULL;

  if (src && *src) {
    start_ptr = src;
    src_line_number = 1;
    int advance = 0;            // a byte offset into src string

    // I'm calling GetElementList here, but a single element is expected.
    // "    <mml:math>..</mml:math>"
    rv = GetElementList(src, advance, NULL);

    // Diagnostics - there should be single root node, <math>..</math>
    if (rv) {
      TCI_ASSERT(rv->prev == NULL);
      TCI_ASSERT(rv->next == NULL);
    } else {
      TCI_ASSERT(0);
    }
  }

  return rv;
}

MNODE* MML2Tree::GetElementList(const char* z_src,
                                int& advance, 
                                MNODE* parent)
{
  MNODE* rv = NULL;
  MNODE* tail;

  int local_advance = 0;
  const char* needle = z_src;

  // loop thru consecutive elements   <mrow>..</mrow>  <mstyle>..</mstyle> etc.
  while (1) {
    // Advance over whitespace to next element <mml:math..
    while (*needle && *needle <= ' ') {
      if (*needle == '\n')
        src_line_number++;
      needle++;
      local_advance++;
    }

    if (*needle == '<') {       // found a token start
      if (*(needle + 1) == '/') { // we're at the end of the parent element
        if (!VerifyElementEnder(parent, needle + 1))
          TCI_ASSERT(0);
        break;
      } else {                  
        // we're at the start of one of the siblings in the current list
        int len_element = 0;
        MNODE* new_node = GetElement(needle, len_element);
        new_node->parent = parent;
        local_advance += len_element;
        needle += len_element;

        if (rv) {
          tail->next = new_node;
          new_node->prev = tail;
        } else {
          rv = new_node;
        }
        tail = new_node;
      }
    } else {                    // can't find '<'
      TCI_ASSERT(*needle == 0);
      break;
    }
  }
  advance = local_advance;
  return rv;
}

MNODE* MML2Tree::GetElement(const char* z_src, int& advance)
{
  MNODE* rv = NULL;

  int local_advance = 0;
  const char* needle = z_src;

  if (*needle == '<') {
    rv = MakeTNode(z_src - start_ptr, 0, src_line_number);

    int element_has_no_contents;
    int len_header = GetElementHeader(needle, rv, element_has_no_contents);
    local_advance = len_header;

    if (element_has_no_contents) {
      local_advance = len_header;
    } else {
      // step over element header
      needle += len_header;
      // step over white space at start of contents
      while (*needle && *needle <= ' ') {
        if (*needle == '\n')
          src_line_number++;
        needle++;
        local_advance++;
      }
      if (*needle) {
        if (*needle == '<') {
          if (*(needle + 1) == '/') {
            // this element has the form <e></e>  There's nothing in it.
            //  read off ender
            while (*needle && *needle != '>') {
              needle++;
              local_advance++;
            }
            local_advance++;
          } else {
            int len_of_children = 0;
            rv->first_kid = GetElementList(needle, len_of_children, rv);
            local_advance += len_of_children;

            needle += len_of_children;
            if (*needle == '<') {
              TCI_ASSERT(*(needle + 1) == '/');
              //  read off ender
              while (*needle && *needle != '>') {
                needle++;
                local_advance++;
              }
              local_advance++;
            }
          }
        } else {
          const char *p_end_chdata = strchr(needle, '<');
          if (p_end_chdata) {
            const char *p_tmp = p_end_chdata;
            while (*(p_tmp - 1) <= ' ') {
              if (*(p_tmp - 1) == '\n')
                src_line_number++;
              p_tmp--;
            }
            int data_len = p_tmp - needle;
            if (data_len > 0) {
              char *tmp = new char[data_len + 1];
              strncpy(tmp, needle, data_len);
              tmp[data_len] = 0;
              rv->p_chdata = tmp;
            }

            local_advance += p_end_chdata - needle;
            needle = p_end_chdata;

            if (*(needle + 1) == '/') {
              // read off ender
              while (*needle && *needle != '>') {
                needle++;
                local_advance++;
              }
              local_advance++;
            } else {
              TCI_ASSERT(0);
            }
          } else {
            TCI_ASSERT(0);
          }
        }
      } else {
        TCI_ASSERT(0);
      }
    }
  } else {
    TCI_ASSERT(!"can't find '<'");
  }
  if (rv)
    rv->src_length = local_advance;

  advance = local_advance;
  return rv;
}

int MML2Tree::GetElementHeader(const char* p_header,
                               MNODE* new_node, 
                               int& is_empty)
{
  is_empty = 0;

  TCI_ASSERT(*p_header == '<');
  TCI_ASSERT(*(p_header + 1) != '/');

  const char* p_nom_left_delim = p_header;

  const char* needle = p_header + 1;
  while (isalpha(*needle))
    needle++;

  if (*needle == ':') {
    p_nom_left_delim = needle;
    needle++;
    while (isalpha(*needle))
      needle++;
  }

  p_nom_left_delim++;
  int nom_len = needle - p_nom_left_delim;

  if (nom_len < 32) {
    strncpy(new_node->src_tok, p_nom_left_delim, nom_len);
    new_node->src_tok[nom_len] = 0;
  } else {
    TCI_ASSERT(0);
  }
  
  // handle attributes
  ATTRIB_REC* a_list = NULL;
  ATTRIB_REC* tail;

  while (1) {
    while (*needle && *needle <= ' ') {
      if (*needle == '\n')
        src_line_number++;
      needle++;
    }

    if (isalpha(*needle)) {
      int bytesdone = 0;
      ATTRIB_REC* new_attr = GetAttribute(needle, bytesdone);
      needle += bytesdone;

      if (new_attr) {
        if (a_list) {
          tail->next = new_attr;
          new_attr->prev = tail;
        } else
          a_list = new_attr;
        tail = new_attr;
      } else {
        TCI_ASSERT(0);
      }
    } else {
      if (*needle == '/') {
        is_empty = 1;
        needle++;
      }
      if (*needle == '>')
        needle++;
      else
        TCI_ASSERT(0);
      break;
    }
  }

  if (a_list)
    new_node->attrib_list = a_list;

  return needle - p_header;
}


ATTRIB_REC* MML2Tree::GetAttribute(const char* p_attr, int& bytesdone)
{
  ATTRIB_REC *rv = NULL;

  char attrib_name[32];
  char attrib_value[256];

  const char* needle = p_attr;
  int di = 0;
  while (isalpha(*needle)) {
    attrib_name[di++] = *needle;
    needle++;
  }
  attrib_name[di] = 0;

  while (*needle != '"')
    needle++;
  needle++;

  di = 0;
  while (*needle != '"') {
    attrib_value[di++] = *needle;
    needle++;
  }
  attrib_value[di] = 0;

  rv = new ATTRIB_REC(attrib_name, attrib_value);

  needle++;
  bytesdone = needle - p_attr;

  return rv;
}


// "...<mml:mrow>......</mml:mrow>..."
// needle =            ^ 

int MML2Tree::VerifyElementEnder(MNODE* mml_element, const char* needle)
{
  int rv = 1;                   // assume ending token OK

  const char *p_nom_left_delim = needle;

  const char *ptr = p_nom_left_delim + 1;
  while (*ptr && isalpha(*ptr))
    ptr++;

  if (*ptr == ':') {            // we have a namespace prefix
    p_nom_left_delim = ptr;
    ptr++;
    while (*ptr && isalpha(*ptr))
      ptr++;
  }

  if (*ptr == '>') {
    if (mml_element) {
      p_nom_left_delim++;
      size_t nom_len = ptr - p_nom_left_delim;
      size_t parent_nom_len = strlen(mml_element->src_tok);
      if (parent_nom_len == nom_len) {
        if (strncmp(mml_element->src_tok, p_nom_left_delim, nom_len))
          rv = 0;
      } else
        rv = 0;
    }
  } else {
    rv = 0;
  }
  return rv;
}
