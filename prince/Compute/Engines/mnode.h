#ifndef MNODE_H
#define MNODE_H

#include "attriblist.h"
#include "logmsg.h"
#include "cmptypes.h"

enum OpIlk {
  OP_none,
  OP_prefix,
  OP_infix,
  OP_postfix
};

const char* OpIlkToString(OpIlk ilk);
OpIlk StringToOpIlk(const char* form);




class MNODE
{
public:
  MNODE(U32 s_off, U32 s_len, U32 line_no) :
     next(NULL), 
     prev(NULL),
	 src_linenum(line_no),
	 src_start_offset(s_off),
	 src_length(s_len),
	 parent(NULL),
	 first_kid(NULL),
	 p_chdata(NULL),
	 precedence(0),
	 form(OP_none),
	 attrib_list(NULL),
	 msg_list(NULL)
  {}


  MNODE* next;
  MNODE* prev;

  U32 src_linenum;
  U32 src_start_offset;
  U32 src_length;
  char src_tok[32];

  MNODE* parent;
  MNODE* first_kid;

  const char* p_chdata;
  int precedence;
  OpIlk form;
  ATTRIB_REC* attrib_list;
  LOG_MSG_REC* msg_list;
};


MNODE* MakeTNode(U32 s_off, U32 e_off, U32 line_no);
void DisposeTNode(MNODE* del);
void DisposeTList(MNODE* t_list);
MNODE *JoinTLists(MNODE* list, MNODE * newtail);

bool DelinkTNode(MNODE* elem);
void DetachTList(MNODE* elem);
bool HasPositionalChildren(MNODE* mml_node);
bool HasRequiredChildren(MNODE* mml_node);
bool HasScriptChildren(MNODE* mml_node);
bool HasInferedMROW(MNODE* mml_node);

char* TNodeToStr(MNODE * mml_node, char *prefix, int indent);

//void CheckMNODETallies();




bool CheckLinks(MNODE* n);



#endif
