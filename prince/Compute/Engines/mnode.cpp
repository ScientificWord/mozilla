#include "mnode.h"
#include "strutils.h"
#include "CmpTypes.h"


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


void FUSetPrefix(const char* src_tok, char* prefix, char* buffer)
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



// Follows links associated with the node and
// tries to crash if anyone has a bad pointer.

bool CheckLinks(MNODE* n)
{
   if (n == 0) 
    return true;
    
   MNODE* p = n->parent;
   MNODE* m = n;
   while (m){
     m = m->parent;
   }
   m = n;
   while (m) {
     if (m -> parent != p){
       //_asm{int 3};
     } else {
       m = m-> next;
     }
   }
   
   m = n->first_kid;
   while (m){
     if (m->parent != n){
       //_asm{int 3}
    } else {
      m = m-> next;
    }
   } 
    
   return true; 
}


