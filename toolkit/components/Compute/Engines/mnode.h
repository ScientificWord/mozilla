#ifndef MNODE_H
#define MNODE_H

#include "attriblist.h"
#include "logmsg.h"
#include "CmpTypes.h"



/*  As we parse LaTeX, a data structure called a "Parse Tree"
is generated. Each node in this tree is a "MNODE" struct
as defined below.
  Note that this one struct is used for two proposes:

  1. Each object that is represented explicitly by a token,
     or command, or an environment in the LaTeX file is represented
     in the parse tree by a MNODE.
  2. Many implicit higher level container objects are also
     represented in the parse tree by a MNODE.

  Example:  $x+\frac 12$

  Each node has a tag of the form <n.n.n>, identifying the object
  represented by the node.

<1.2.0>$ <-> <3.1.24>x <-> <3.13.60>+ <-> <5.1.0>\frac <-> <2.2.0>$
											  \
										      <5.1.1> <-> <5.1.2>
												 \			 \
									            <3.3.2>1    <3.3.3>2

  There are 7 LaTeX tokens in the examples - note that "\frac" is one
  token.  In the parse tree, a 2-node sublist appears below the \frac
  node.  These nodes represent the implicit parts of a fraction,
  numerator and denominator.  Sublists representing the explicit
  contents of these parts appear below.
*/




enum OpIlk {
  OP_none,
  OP_prefix,
  OP_infix,
  OP_postfix
};

const char* OpIlkToString(OpIlk ilk);
OpIlk StringToOpIlk(const char* form);


enum OpMatrixIntervalType {
    OMI_none,
    OMI_matrix,
    OMI_interval
  };





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

bool ElementNameIs(const MNODE* pNode, const char* str);
void SetElementName(MNODE* pNode, const char* str);

bool ContentIs(const MNODE* pNode, const char* str);
void SetContent(MNODE* pNode, const char* str);


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

void GetCurrAttribValue(const MNODE* mml_node, 
                        bool inherit,
                        const char* targ_attr, 
                        char* buffer, 
                        int lim);

void SemanticAttribs2Buffer(char* buffer, const MNODE* mml_node, int lim);

bool IsDIFFOP(MNODE* mml_frac_node,
              MNODE** m_num_operand, 
              MNODE** m_den_var_expr);


bool IsDDIFFOP(MNODE* mml_msub_node);

bool IsBesselFunc(MNODE* mml_msub_node);
bool IsApplyFunction(MNODE* next_elem);
bool IsLaplacian(MNODE* op_node);


void ChooseIndVar(MNODE* dMML_tree, char* buffer);
MNODE* Find_dx(MNODE* mrow, bool& is_nested);


bool IsUnitsFraction(MNODE * mml_node);
bool IsPositionalChild(MNODE * mml_node);
bool IsSUBSTITUTION(MNODE * mml_msub_node);
bool IsUSunit(const char *ptr);
bool IsWholeNumber(MNODE* mml_mn);
bool IsWholeFrac(MNODE* mml_frac);
bool IsOperator(MNODE* mml_node);

int  CountCols(MNODE* mml_mtr);
bool IsWhiteSpace(MNODE* mml_node);



OpMatrixIntervalType GetOpType(MNODE* mo);
bool OpArgIsMatrix(MNODE* mml_mi_node);
bool IsArgDelimitingFence(MNODE* mml_node);


MNODE* LocateOperator(MNODE* mml_list, OpIlk& op_ilk, int& advance);



bool CheckLinks(const MNODE* n);

#ifdef DEBUG
#define ALLOW_DUMPS
#endif

// dump utilities.  output sent to JBMline.out
namespace JBM {
  void DumpTNode(MNODE* t_node, int indent);
  void DumpTList(MNODE* t_list, int indent);
}

#endif
