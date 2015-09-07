#include "mnode.h"
#include "strutils.h"
#include "CmpTypes.h"
#include "dumputils.h"
#include "stdio.h"
#include <cstring>


bool ElementNameIs(const MNODE* pNode, const char* str)
{
  if (pNode == NULL)
    return false;
  else
    return StringEqual(pNode->src_tok, str);
}

void SetElementName(MNODE* pNode, const char* str)
{
  TCI_ASSERT( pNode != NULL );
  strcpy(pNode->src_tok, str);
}

bool ContentIs(const MNODE* pNode, const char* str)
{
  if (pNode == NULL)
    return false;
  else
    return StringEqual(pNode->p_chdata, str);
}

void SetContent(MNODE* pNode, const char* str)
{
  delete [] pNode -> p_chdata;
  pNode -> p_chdata = DuplicateString(str);
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



// Utility to make a translation node

MNODE* MakeTNode(U32 s_off, U32 s_len, U32 line_no)
{
  MNODE *rv = new MNODE(s_off, s_len, line_no);
  rv->src_tok[0] = 0;

  return rv;
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

void DisposeTList(MNODE* t_list)
{
  MNODE* del;
  while (t_list) {
    del = t_list;
    t_list = t_list->next;
    DisposeTNode(del);
  }
}

MNODE* JoinTLists(MNODE* list, MNODE* newtail)
{
  MNODE *rv;
  if (list) {
    rv = list;
    while (list -> next)
      list = list -> next;
    list -> next = newtail;
    if (newtail)
      newtail->prev = list;
  } else {
    rv = newtail;
  }

  return rv;
}


// Function to remove a MNODE's links to a tree.
// The delinked node can be moved or disposed by the caller.
// Note that we can't delink the first node of a first
//  level list here.

bool DelinkTNode(MNODE* elem)
{
  bool rv = true;           // assume all will be OK.

  // remove elem from it's present list:  prev <-> elem <-> next
  MNODE* el_prev = elem->prev;

  if (el_prev) {                // elem has a prev

    el_prev->next = elem->next;
    if (elem->next) {
      elem->next->prev = el_prev;
    }

  } else {                      // elem may head a sublist

    MNODE* el_owner = elem->parent;

    if (el_owner) {

      if (el_owner->first_kid == elem) {  // elem is head of sublist

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



void DetachTList(MNODE* elem)
{
  MNODE* prev_node = elem->prev;
  if (prev_node) {              // elem has a prev
    // remove elem from it's present list:  prev <-> elem
    prev_node->next = NULL;
    elem->prev = NULL;
  } else {                      // elem may head a sublist
    MNODE* owner_node = elem->parent;
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
bool HasPositionalChildren(MNODE* mml_node)
{
  if (mml_node) {
    const char* p_elem = mml_node -> src_tok;
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


void FUSetPrefix(const char* src_tok, char* prefix, char* buffer)
{
  buffer[0] = 0;
  if (src_tok) {
    const char* token_ptr = strchr(src_tok, ':');
    if (token_ptr)
      token_ptr++;
    else
      token_ptr = src_tok;

    if (prefix && *prefix)
      strcpy(buffer, prefix);

    strcat(buffer, token_ptr);
  }
}



char* TNodeToStr(MNODE * mml_node, char *prefix, int indent)
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
	// Take out pretty printing -- not useful at this stage
    // jcs while (ii < indent && ii < 127)
    // jcs   indent_str[ii++] = ' ';
    indent_str[ii] = 0;

    char element[80];
    char zzz[256];
    FUSetPrefix(mml_node->src_tok, l_prefix, element);
    sprintf(zzz, "%s<%s", indent_str, element);
    buffer = AppendStr2HeapStr(buffer, bln, zzz);

    ATTRIB_REC* rover = mml_node->attrib_list;
    while (rover) {
      sprintf(zzz, " %s=\"%s\"", rover->zattr_nom, rover->zattr_val);
      buffer = AppendStr2HeapStr(buffer, bln, zzz);
      rover = rover->GetNext();
    }

    if (mml_node->p_chdata && mml_node->first_kid) {
      TCI_ASSERT
        (!"I don't MML will ever allow both chdata and children on the same node");
      sprintf(zzz, " name=\"%s\">", mml_node->p_chdata);
      buffer = AppendStr2HeapStr(buffer, bln, zzz);
	
      MNODE *t_list = mml_node->first_kid;
      while (t_list) {
        char *tmp = TNodeToStr(t_list, l_prefix, indent + 2);
        buffer = AppendStr2HeapStr(buffer, bln, tmp);
        delete[] tmp;
        t_list = t_list->next;
      }
      FUSetPrefix(mml_node->src_tok, l_prefix, element);
	  
	  sprintf(zzz, "%s</%s>", indent_str, element);

      buffer = AppendStr2HeapStr(buffer, bln, zzz);
    } else if (mml_node->p_chdata) {
      buffer = AppendStr2HeapStr(buffer, bln, ">");
      buffer = AppendStr2HeapStr(buffer, bln, mml_node->p_chdata);
      FUSetPrefix(mml_node->src_tok, l_prefix, element);
	  sprintf(zzz, "</%s>", element);

      buffer = AppendStr2HeapStr(buffer, bln, zzz);
    } else if (mml_node->first_kid) {
	  
	  buffer = AppendStr2HeapStr(buffer, bln, ">");

      MNODE *t_list = mml_node->first_kid;
      while (t_list) {
        char *tmp = TNodeToStr(t_list, l_prefix, indent + 2);
        buffer = AppendStr2HeapStr(buffer, bln, tmp);
        delete[] tmp;
        t_list = t_list->next;
      }
      FUSetPrefix(mml_node->src_tok, l_prefix, element);
      sprintf(zzz, "%s</%s>", indent_str, element);
      buffer = AppendStr2HeapStr(buffer, bln, zzz);
    } else {
      if (!strcmp(mml_node->src_tok, "mspace")) {
	      buffer = AppendStr2HeapStr(buffer, bln, "/>");
	    
      } else {
        FUSetPrefix(mml_node->src_tok, l_prefix, element);
        sprintf(zzz, "></%s>", element);
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


// A function to enter debugger

void CallDebugger()
{
#ifdef _WINDOWS
  _asm{int 3};
#endif
}

// Follows links associated with the node and
// tries to crash if anyone has a bad pointer.

bool CheckLinks(const MNODE* me)
{
   if (me == 0) 
    return true;
   
   const MNODE* parent = me -> parent;    
   const MNODE* first = me -> first_kid;

   
   
   const MNODE* rover = first;
   while (rover != NULL){
     
     if (rover -> parent != me)	 
         CallDebugger();

     if (rover -> next){
	   if (rover -> next -> prev != rover)
	     CallDebugger();
	 }

     if (rover -> prev){
	   if (rover -> prev -> next != rover)
	     CallDebugger();
	 }
	 if (! CheckLinks(rover)){
	     CallDebugger();
	 } 
	 rover = rover -> next;
   }
    
   return true; 
}


void GetCurrAttribValue(const MNODE* mml_node, 
                        bool inherit,
                        const char* targ_attr_name, 
                        char* buffer, 
                        int lim)
{
  // Check the current node for the target attribute
  const char *attr_val = GetATTRIBvalue(mml_node->attrib_list, targ_attr_name);

  // Ascend the parent tree if necessary
  if (!attr_val && inherit) {
    MNODE *rover = mml_node->parent;
    while (rover && !attr_val) {
      if (!strcmp(rover->src_tok, "mstyle")) {
        attr_val = GetATTRIBvalue(rover->attrib_list, targ_attr_name);
      }
      rover = rover->parent;
    }
  }

  if (attr_val) {
    size_t curr_ln = strlen(buffer);
    size_t inc_ln = strlen(attr_val);
    if (curr_ln + inc_ln < lim)
      strcat(buffer, attr_val);
    else
      TCI_ASSERT(0);
  }
}


void SemanticAttribs2Buffer(char* buffer, const MNODE* mml_node, int lim)
{
  TCI_ASSERT(CheckLinks(mml_node));
  GetCurrAttribValue(mml_node, true, "mathvariant", buffer, lim);
  // May need to add more calls here
}


bool IsDDIFFOP(MNODE* mml_msub_node)
{
  bool rv = false;

  if (mml_msub_node && mml_msub_node->first_kid) {
    MNODE *base = mml_msub_node->first_kid;
    const char *base_elem = base->src_tok;
    const char *base_data = base->p_chdata;
    if (!strcmp(base_elem, "mo")
        && !strcmp(base_data, "&#x2145;"))  // &DD;
      rv = true;
  } else
    TCI_ASSERT(0);

  return rv;
}

bool IsDIFFOP(MNODE* mml_frac_node,
              MNODE** m_num_operand, 
              MNODE** m_den_var_expr)
{
  bool rv = false;
  *m_num_operand = NULL;
  *m_den_var_expr = NULL;

  if (mml_frac_node) {
    MNODE *num = mml_frac_node->first_kid;
    if (num) {
      MNODE *den = num->next;
      if (den) {

        if (!strcmp(num->src_tok, "mrow")) {  // dy OR d expr
          num = num->first_kid;
          *m_num_operand = num->next;
        }
        const char *num_elem = num->src_tok;  // mo OR msup
        const char *num_data = num->p_chdata; // d OR d^2

        if (!strcmp(num_elem, "msup")) {  // d^2
          MNODE *num_base = num->first_kid;
          num_elem = num_base->src_tok; // mo
          num_data = num_base->p_chdata;  // d
        }

        if (strcmp(num_elem, "mo")) // must be a diff op here
          return false;

        int diff_symbol = 0;
        if (!strcmp(num_data, "&#x2146;"))  // &dd;
          diff_symbol = 1;
        else if (!strcmp(num_data, "&#x2202;")) // &PartialD;
          diff_symbol = 2;

        if (diff_symbol) {
          const char *den_elem = den->src_tok;
          if (!strcmp(den_elem, "mrow")) {  // d * x
            MNODE *den1 = den->first_kid;
            if (den1) {
              // may have a product in the denom - dx^2 * dy^5
              if (!strcmp(den1->src_tok, "mrow"))
                den1 = den1->first_kid;

              const char *den1_elem = den1->src_tok;  // mo
              const char *den1_data = den1->p_chdata; // &dd;

              if (strcmp(den1_elem, "mo"))  // diff op
                return false;

              int diff1_symbol = 0;
              if (!strcmp(den1_data, "&#x2146;")) // &dd;
                diff1_symbol = 1;
              else if (!strcmp(den1_data, "&#x2202;"))  // &PartialD;
                diff1_symbol = 2;

              if (diff_symbol == diff1_symbol) {
                MNODE *den2 = den1->next;
                const char *den2_elem = den2->src_tok;
                if (!strcmp(den2_elem, "mi")  // dx{^2}
                    || !strcmp(den2_elem, "msup")) {
                  rv = true;
                  *m_den_var_expr = den;
                }
              }
            }
          }
        }
      }
    }
  }

  return rv;
}


bool IsBesselFunc(MNODE * mml_msub_node)
{
  bool rv = false;

  if (mml_msub_node && mml_msub_node->first_kid) {
    MNODE *base = mml_msub_node->first_kid;
    MNODE *sub = base->next;
    if (sub) {
      const char *base_elem = base->src_tok;
      if (!strcmp(base_elem, "mi") || !strcmp(base_elem, "mo")) {
        const char *ptr = base->p_chdata;
        if (ptr && !strncmp(ptr, "Bessel", 6)) {
          rv = true;
        }
      }
    }
  }

  return rv;
}


bool IsApplyFunction(MNODE* next_elem)
{
  bool rv = false;

  const char *next_elem_nom = next_elem -> src_tok;
  if (!strcmp(next_elem_nom, "mo")) {
    const char *ptr = strstr(next_elem -> p_chdata, "&#x");
    if (ptr) {
      U32 unicode = ASCII2U32(ptr + 3, 16);
      if (unicode == 0x2061)    // &Applyfunction;
        rv = true;
    }
  }
  return rv;
}


// Need to complete the following - find an unused var.

void ChooseIndVar(MNODE* dMML_tree, char *buffer)
{
  strcpy(buffer, "t");
}

// Locate the "dx".
// Note that the "mrow" passed into this function
//  may be the numerator of a fractional integrand.
// The function isn't recursive - I think the "dx"
//   should be the entire numerator as in "dx/x" or
//   should be a factor of the numerator as in "2xdx/sin(x)"

MNODE* Find_dx(MNODE * num_mrow, bool & is_nested)
{
  MNODE *rv = NULL;
  is_nested = false;

  if (num_mrow && !strcmp(num_mrow->src_tok, "mrow")) {
    MNODE *m_child = num_mrow->first_kid;
    if (m_child && !strcmp(m_child->src_tok, "mo")
        && !strcmp(m_child->p_chdata, "&#x2146;")) {  // "d"
      rv = num_mrow;
    } else if (m_child) {
      MNODE *m_rover = m_child;
      while (m_rover) {
        if (!strcmp(m_rover->src_tok, "mrow")) {
          MNODE *m_child2 = m_rover->first_kid;
          if (m_child2 && !strcmp(m_child2->src_tok, "mo")
              && !strcmp(m_child2->p_chdata, "&#x2146;")) { // "d"
            rv = m_rover;
            is_nested = true;
            break;
          }
        }
        m_rover = m_rover->next;
      }
    } else
      TCI_ASSERT(0);
  } else
    TCI_ASSERT(0);

  return rv;
}


bool IsUnitsFraction(MNODE* mml_frac)
{
  bool num_OK = false;
  bool den_OK = false;

  if (mml_frac && !strcmp(mml_frac->src_tok, "mfrac")) {
    MNODE *rover = mml_frac->first_kid;
    if (rover) {
      if (!strcmp(rover->src_tok, "mi")) {
        char zunit[256];
        zunit[0] = 0;
        GetCurrAttribValue(rover, false, "msiunit", zunit, 256);
        if (!strcmp(zunit, "true"))
          num_OK = true;
      }
      rover = rover->next;
      if (rover && !strcmp(rover->src_tok, "mi")) {
        char zunit[256];
        zunit[0] = 0;
        GetCurrAttribValue(rover, false, "msiunit", zunit, 256);
        if (!strcmp(zunit, "true"))
          den_OK = true;
      }
    }
  }

  return num_OK && den_OK;
}

bool IsPositionalChild(MNODE* mml_node)
{
  MNODE* the_parent = mml_node->parent;
  if (!the_parent && mml_node->prev) {
    MNODE* rover = mml_node->prev;
    while (rover->prev)
      rover = rover->prev;
    the_parent = rover->parent;
  }

  return HasPositionalChildren(the_parent);
}



// Some subscripted fences are intrepreted as "subs".
bool IsSUBSTITUTION(MNODE* mml_msub_node)
{
  bool rv = false;

  if (mml_msub_node) {
    MNODE* base = mml_msub_node->first_kid;
    if (base) {
      MNODE* sub = base->next;
      if (sub) {
        const char* base_elem = base->src_tok;
        if (StringEqual(base_elem, "mfenced")) {
          char zopen_attr_val[32];
          zopen_attr_val[0] = 0;
          GetCurrAttribValue(base, false, "open", zopen_attr_val, 256);

          char zclose_attr_val[32];
          zclose_attr_val[0] = 0;
          GetCurrAttribValue(base, false, "close", zclose_attr_val, 256);

          if (zopen_attr_val[0] == '[' && zclose_attr_val[0] == ']')
            rv = true;
          if (zopen_attr_val[0] == '[' && StringEqual(zclose_attr_val, "&#x250a;") )
            rv = true;
          if (zopen_attr_val[0] == 'I' && StringEqual(zclose_attr_val, "&#x250a;") )
            rv = true;
          if (zopen_attr_val[0] == 'I' && zclose_attr_val[0] == ']')
            rv = true;
          if (zopen_attr_val[0] == 'I' && zclose_attr_val[0] == '|')
            rv = true;
		      if (zopen_attr_val[0] == '\0' &&  zclose_attr_val[0] == '|')
		        rv = true;
   
          if (StringEqual(zopen_attr_val, "&#x250a;") && (zclose_attr_val[0] == ']') )
            rv = true;
          if (StringEqual(zopen_attr_val, "&#x250a;") && (zclose_attr_val[0] == 'I') )
            rv = true;
          if (StringEqual(zopen_attr_val, "&#x250a;") && (zclose_attr_val[0] == '|') )
            rv = true;

          
        }
      }
    }
  }

  return rv;
}

bool IsUSunit(const char *ptr)
{
  return false;
}

bool IsLaplacian(MNODE* op_node)
{
  bool rv = false;

  if (op_node && op_node->p_chdata) {
    const char *ptr = strstr(op_node->p_chdata, "&#x");
    if (ptr) {
      U32 unicode = ASCII2U32(ptr + 3, 16);
      if (unicode == 0x2207) {  // nabla
        MNODE *sup = op_node->next;
        if (sup && sup->p_chdata && !strcmp(sup->p_chdata, "2"))
          rv = true;
      }
    }
  }
  return rv;
}


// Function to decide if an operator may take matrix args
//  or interval args

OpMatrixIntervalType GetOpType(MNODE * mo)
{
  OpMatrixIntervalType rv = OMI_none;

  if (mo && mo->p_chdata) {
    const char *ptr = strstr(mo->p_chdata, "&#x");
    if (ptr) {
      U32 unicode = ASCII2U32(ptr + 3, 16);
      if (unicode == 0x2212     // &minus;
          || unicode == 0xd7    // &times;
          || unicode == 0x22c5  // DOT PRODUCT
          || unicode == 0x2062) { // Invisible times 
        rv = OMI_matrix;
      } else if (unicode == 0x2208  // &elem;
                 || unicode == 0x220a  // &elem;
                 || unicode == 0x2229  // &cap;
                 || unicode == 0x222a) {  // &cup;
        rv = OMI_interval;
      }
    } else {
      size_t zln = strlen(mo->p_chdata);
      if (zln == 1) {
        char ch = mo->p_chdata[0];
        if ( (ch == '+') || (ch == '-') )
          rv = OMI_matrix;
      }
    }
  }

  return rv;
}






bool IsWholeNumber(MNODE* mml_mn)
{
  bool rv = true;
  if (mml_mn && mml_mn->p_chdata) {
    const char* ptr = mml_mn->p_chdata;
    while (*ptr) {
      if (*ptr < '0' || *ptr > '9') {
        rv = false;
        break;
      }
      ptr++;
    }
  } else
    TCI_ASSERT(0);
  return rv;
}

bool IsOperator(MNODE* mml_node)
{
  bool rv = false;
  if (mml_node && ElementNameIs(mml_node, "mo")) {
    rv = true;
  }

  return rv;
}

bool IsWholeFrac(MNODE* mml_frac)
{
  bool num_OK = false;
  bool den_OK = false;

  if (mml_frac && ElementNameIs(mml_frac, "mfrac")) {
    MNODE *rover = mml_frac->first_kid;
    if (rover) {
      if (ElementNameIs(rover, "mn") && IsWholeNumber(rover))
        num_OK = true;

      rover = rover->next;

      if (rover) {
        if (ElementNameIs(rover, "mn") && IsWholeNumber(rover))
          den_OK = true;
      }
    }
  }
  return num_OK && den_OK;
}


int CountCols(MNODE* mml_mtr)
{
  int rv = 0;
  if (mml_mtr && mml_mtr->first_kid) {
    MNODE *rover = mml_mtr->first_kid;
    while (rover) {
      if (ElementNameIs(rover, "mtd"))
        rv++;
      rover = rover->next;
    }
  }
  return rv;
}

bool IsWhiteSpace(MNODE* mml_node)
{
  return false;
}



#ifdef ALLOW_DUMPS
void JBM::DumpTNode(MNODE* t_node, int indent)
{
  if (t_node) {
    // form the indentation string
    char indent_str[128];
    int ii = 0;
    while (ii < indent && ii < 127)
      indent_str[ii++] = ' ';
    indent_str[ii] = 0;

    char zzz[256];
    sprintf(zzz, "%4lu %s<%s", t_node->src_linenum, indent_str, t_node->src_tok);
    JBMLine(zzz);

    ATTRIB_REC* rover = t_node->attrib_list;
    while (rover) {
      sprintf(zzz, " %s=\"%s\"", rover->zattr_nom, rover->zattr_val);
      JBMLine(zzz);
      rover = rover->GetNext();
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



MNODE* LocateOperator(MNODE* mml_list, OpIlk &op_ilk, int& advance)
{
  MNODE* rv = NULL;
  op_ilk = OP_none;
  advance = 0;

  MNODE* rover = mml_list;
  while (rover) {
    const char* mml_element = rover->src_tok;

    size_t ln = strlen(mml_element);

    bool embellished = true;
    MNODE* key = NULL;
    switch (ln) {
    case 2:
      if (StringEqual(mml_element, "mi")) {
        if (ContentIs(rover, "mod")) {
           mml_element = "mo"; 
           key = rover;
           embellished = false;
        }

      } else if (StringEqual(mml_element, "mo")) {
        key = rover;
        embellished = false;
      }
      break;
    case 4:
      if (StringEqual(mml_element, "msup")) {
        key = rover->first_kid;
      } else if (StringEqual(mml_element, "msub")) {
        key = rover->first_kid;
      }
      break;
    case 5:
      if (StringEqual(mml_element, "mover")) {
        key = rover->first_kid;
      }
      break;
    case 6:
      if (StringEqual(mml_element, "munder")) {
        key = rover->first_kid;
      }
      break;
    case 7:
      if (StringEqual(mml_element, "msubsup")) {
        key = rover->first_kid;
      }
      break;
    case 10:
      if (StringEqual(mml_element, "munderover")) {
        key = rover->first_kid;
      }
      break;
    default:
      break;
    }

    // Check the current node for the target attribute
    if (ElementNameIs(key, "mo")) {

      const char* attr_val = GetATTRIBvalue(key->attrib_list, "form");
      if (attr_val)
        op_ilk = StringToOpIlk(attr_val);
      else
        op_ilk = OP_infix;

      rv = rover;
      if (embellished && op_ilk != OP_prefix)
        TCI_ASSERT(0);
      advance++;
      break;
    }
    advance++;
    rover = rover->next;
  }
  return rv;
}



bool OpArgIsMatrix(MNODE* mml_mi_node)
{
  bool rv = false;
  size_t ln = strlen(mml_mi_node->p_chdata);
  switch (ln) {
  case 3:
    if (StringEqual("div", mml_mi_node->p_chdata))
      rv = true;
    break;
  case 4:
    if (StringEqual("curl", mml_mi_node->p_chdata))
      rv = true;
    break;
  default:
    break;
  }

  return rv;
}


bool IsArgDelimitingFence(MNODE * mml_node)
{
  bool rv = false;

  if (ElementNameIs(mml_node, "mfenced")) {
    U32 l_unicode = '(';
    U32 r_unicode = ')';
    if (mml_node->attrib_list) {
      const char *open_value =
        GetATTRIBvalue(mml_node->attrib_list, "open");
      const char *close_value =
        GetATTRIBvalue(mml_node->attrib_list, "close");
      if (open_value) {
        if (*open_value == '&')
          l_unicode = ASCII2U32(open_value + 3, 16);
        else
          l_unicode = open_value[0];
      }
      if (close_value) {
        if (*close_value == '&')
          r_unicode = ASCII2U32(close_value + 3, 16);
        else
          r_unicode = close_value[0];
      }
    }
    if (l_unicode == '(' && r_unicode == ')')
      rv = true;
    else if (l_unicode == '[' && r_unicode == ']')
      rv = true;
    else if (l_unicode == '{' && r_unicode == '}')
      rv = true;

  }
  return rv;
}








