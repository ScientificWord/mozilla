// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

#ifndef FilterUtils_h
#define FilterUtils_h

#include "../CmpTypes.h"
#include <stdlib.h>
#include <stdio.h>
#include <io.h>

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

typedef struct tagPARAM_REC
{                               // Struct to carry a command parameter
  tagPARAM_REC *next;
  U32 param_ID;
  U32 param_type;
  char *ztext;
} PARAM_REC;

PARAM_REC *AppendParam(PARAM_REC * curr_list, U32 p_ID, U32 p_type,
                       const char *zdata);
void DisposeParamList(PARAM_REC * p_list);

enum VarType {
  VT_None,  // unused
  VT_Normal,
  VT_Matrix
};

typedef struct tagVAR_REC
{
  tagVAR_REC *next;
  char *var_eng_name;
  VarType var_type;
} VAR_REC;

void AppendVarRec(VAR_REC * &curr_list, const char *v_nom, VarType v_type);
void DisposeVarList(VAR_REC * v_list);
VAR_REC *FindVarRec(VAR_REC * curr_list, const char *v_nom);

enum NameIlk {
  Ilk_Var,
  Ilk_Func,
  Ilk_ODE,
  Ilk_WasODE
};

enum OpIlk {
  OP_none,
  OP_prefix,
  OP_infix,
  OP_postfix
};

const char * OpIlkToString(OpIlk ilk);
OpIlk StringToOpIlk(const char * form);


typedef struct tagNAME2AKA_REC {	// name -> node mapping record
  tagNAME2AKA_REC*  next;
  char*             alias;      // engine name
  char*             src_canonical_name;
  char*             user_name;
  NameIlk           obj_ilk;
} NAME2AKA_REC;

typedef struct tagClientAliases_REC { 
  tagClientAliases_REC*  next;
  U32                    client_ID;
  NAME2AKA_REC*          AKAs_list;
  U32                    next_var_number;
  U32                    next_func_number;
} ClientAliases_REC;

typedef struct tagATTRIB_REC
{                               // Struct to define an attribute node.
  tagATTRIB_REC *next;          //  The MNODE struct defined below
  tagATTRIB_REC *prev;          //  can carry a list of attributes.
  char *zattr_nom;
  char *zattr_val;
} ATTRIB_REC;

// struct to carry info for a message to be written to the log file
typedef struct tagLOG_MSG_REC
{
  tagLOG_MSG_REC *next;
  char *msg;
} LOG_MSG_REC;

typedef struct tagMNODE
{
  tagMNODE *next;
  tagMNODE *prev;
  U32 src_linenum;
  U32 src_start_offset;
  U32 src_length;
  char src_tok[32];
  tagMNODE *parent;
  tagMNODE *first_kid;
  const char *p_chdata;
  int precedence;
  OpIlk form;
  ATTRIB_REC *attrib_list;
  LOG_MSG_REC *msg_list;
} MNODE;

typedef struct tagMIC2MMLNODE_REC
{                               // name -> node mapping record
  tagMIC2MMLNODE_REC *next;
  U32 owner_ID;
  char *canonical_name;
  char *mml_markup;
} MIC2MMLNODE_REC;

BUCKET_REC *MakeBucketRec(U32 which_bucket, SEMANTICS_NODE * sem_child);
BUCKET_REC *AppendBucketRec(BUCKET_REC * a_list, BUCKET_REC * new_a_rec);
BUCKET_REC *FindBucketRec(BUCKET_REC * a_list, U32 bucket_ID);
void DisposeBucketList(BUCKET_REC * arg_list);

SEMANTICS_NODE *CreateSemanticsNode();
void DisposeSemanticsNode(SEMANTICS_NODE * del);
void DisposeSList(SEMANTICS_NODE * s_list);
char *DumpSNode(const SEMANTICS_NODE * s_node, int indent);
char *DumpSList(const SEMANTICS_NODE * s_list, int indent);
SEMANTICS_NODE *AppendSLists(SEMANTICS_NODE * s_list,
                             SEMANTICS_NODE * new_tail);
int SemanticVariant2NumIntegrals(SemanticVariant var);

const char *GetMarkupFromID(MIC2MMLNODE_REC * node_IDs_list,
                            const char *targ_ID);
void DisposeIDsList(MIC2MMLNODE_REC * node_IDs_list);
MIC2MMLNODE_REC *JoinIDsLists(MIC2MMLNODE_REC * IDs_list,
                              MIC2MMLNODE_REC * appender);

enum LogMsgID
{
  MSG_UNSUPPORTED_NUMBER,
  MSG_UNSUPPORTED_OPERATOR,
  MSG_UNDEFINED_FUNCTION,
  MSG_UNDEFINED_INVERSE,
  MSG_UNDEFINED_LIMFUNC,
  MSG_TEXT_IN_MATH
};

void DisposeMsgs(LOG_MSG_REC* msg_list);
void RecordMsg(LOG_MSG_REC* &msg_list, LogMsgID id, const char* token);

MNODE *MakeTNode(U32 s_off, U32 e_off, U32 line_no);
void DisposeTNode(MNODE * del);
void DisposeTList(MNODE * t_list);
MNODE *JoinTLists(MNODE * list, MNODE * newtail);
char *TNodeToStr(MNODE * mml_node, char *prefix, int indent);

void CheckMNODETallies();

bool DelinkTNode(MNODE * elem);
void DetachTList(MNODE * elem);
bool HasPositionalChildren(MNODE * mml_node);
bool HasRequiredChildren(MNODE * mml_node);
bool HasScriptChildren(MNODE * mml_node);
bool HasInferedMROW(MNODE * mml_node);

ATTRIB_REC *MakeATTRIBNode(const char *attrib_name, const char *attrib_value);
const char *GetATTRIBvalue(ATTRIB_REC * a_list, const char *attrib_name);
void DisposeAttribs(ATTRIB_REC * alist);
ATTRIB_REC *ExtractAttrs(char *zattrsAndvals);
ATTRIB_REC *MergeAttrsLists(ATTRIB_REC * dest_list, ATTRIB_REC * new_attrs);
ATTRIB_REC *RemoveAttr(ATTRIB_REC * a_list, const char *attr_nom);

class Grammar;
bool IsTrigArgFuncName(Grammar *mml_entities, const char * f_nom);
bool IsReservedFuncName(const char * f_nom);

void StrReplace(char *line, size_t zln, char *tok, const char *sub);

U32 ASCII2U32(const char *p_digits, int place_val);
U32 NumericEntity2U32(const char *p_entity);
int CountSymbols(const char *p_chdata, int &n_entities);

char *AppendStr2HeapStr(char *zheap_str, U32 & buffer_ln,
                        const char *z_append_str);

INPUT_NOTATION_REC *CreateNotationRec();

SEMANTICS_NODE *PrefixToInfix(SEMANTICS_NODE * s_list);
void SetInfixPrecedence(SEMANTICS_NODE * snode);
SEMANTICS_NODE *NestInPGroup(SEMANTICS_NODE * s_list,
                             BUCKET_REC * parent_bucket);
void FunctionToInfix(SEMANTICS_NODE * snode, char *zop_str);
char *NestInParens(char *z_expr, bool forced);
char *NestInBrackets(char *z_expr);
char *RemovezStrParens(char *z_expr);

char *WideToASCII(const U16 * w_markup);
U16 *ASCIItoWide(const char *ascii, int &zlen);

#ifdef TESTING
#define ALLOW_DUMPS
#endif

// dump utilities.  output sent to JBMline.out
namespace JBM {
  void JBMLine(const char *line);
  void ClearLog();
  void DumpTNode(MNODE * t_node, int indent);
  void DumpTList(MNODE * t_list, int indent);
  void DumpSList(const SEMANTICS_NODE * s_list);
}

#endif
