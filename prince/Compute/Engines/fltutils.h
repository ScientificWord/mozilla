// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

#ifndef FilterUtils_h
#define FilterUtils_h

#include "CmpTypes.h"
#include <stdlib.h>
#include <stdio.h>

#include "mnode.h"
#include "logmsg.h"


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




struct PARAM_REC
{                               // Struct to carry a command parameter
  PARAM_REC *next;
  U32 param_ID;
  U32 param_type;
  char *ztext;
};

PARAM_REC* AppendParam(PARAM_REC * curr_list, U32 p_ID, U32 p_type,
                       const char *zdata);

void DisposeParamList(PARAM_REC * p_list);

enum VarType {
  VT_None,  // unused
  VT_Normal,
  VT_Matrix
};

struct VAR_REC
{
  VAR_REC *next;
  char* var_eng_name;
  VarType var_type;
};

void AppendVarRec(VAR_REC * &curr_list, const char *v_nom, VarType v_type);
void DisposeVarList(VAR_REC * v_list);
VAR_REC *FindVarRec(VAR_REC * curr_list, const char *v_nom);

enum NameIlk {
  Ilk_Var,
  Ilk_Func,
  Ilk_ODE,
  Ilk_WasODE
};


struct NAME2AKA_REC {	// name -> node mapping record
  NAME2AKA_REC*  next;
  char*          alias;      // engine name
  char*          src_canonical_name;
  char*          user_name;
  NameIlk        obj_ilk;
};

struct ClientAliases_REC { 
  ClientAliases_REC*  next;
  U32                 client_ID;
  NAME2AKA_REC*       AKAs_list;
  U32                 next_var_number;
  U32                 next_func_number;
};






struct MIC2MMLNODE_REC
{                               // name -> node mapping record
  MIC2MMLNODE_REC *next;
  U32 owner_ID;
  char *canonical_name;
  char *mml_markup;
};




BUCKET_REC* MakeBucketRec(U32 which_bucket, SEMANTICS_NODE* sem_child);
BUCKET_REC* MakeParentBucketRec(U32 which_bucket, SEMANTICS_NODE* sem_child);


BUCKET_REC *AppendBucketRec(BUCKET_REC * a_list, BUCKET_REC * new_a_rec);
BUCKET_REC *FindBucketRec(BUCKET_REC * a_list, U32 bucket_ID);
void DisposeBucketList(BUCKET_REC * arg_list);

SEMANTICS_NODE* CreateSemanticsNode();
void DisposeSemanticsNode(SEMANTICS_NODE* del);
void DisposeSList(SEMANTICS_NODE * s_list);
char* DumpSNode(const SEMANTICS_NODE * s_node, int indent);
char* DumpSList(const SEMANTICS_NODE * s_list, int indent);
SEMANTICS_NODE *AppendSLists(SEMANTICS_NODE * s_list,
                             SEMANTICS_NODE * new_tail);
int SemanticVariant2NumIntegrals(SemanticVariant var);

const char *GetMarkupFromID(MIC2MMLNODE_REC * node_IDs_list,
                            const char *targ_ID);
void DisposeIDsList(MIC2MMLNODE_REC * node_IDs_list);
MIC2MMLNODE_REC *JoinIDsLists(MIC2MMLNODE_REC * IDs_list,
                              MIC2MMLNODE_REC * appender);





class Grammar;
bool IsTrigArgFuncName(Grammar* mml_entities, const char* f_nom);
bool IsReservedFuncName(Grammar* mml_entities, const char* f_nom);


U32 NumericEntity2U32(const char *p_entity);
int CountSymbols(const char *p_chdata, int &n_entities);


INPUT_NOTATION_REC *CreateNotationRec();

SEMANTICS_NODE *PrefixToInfix(SEMANTICS_NODE * s_list);
void SetInfixPrecedence(SEMANTICS_NODE * snode);
SEMANTICS_NODE *NestInPGroup(SEMANTICS_NODE * s_list,
                             BUCKET_REC * parent_bucket);
void FunctionToInfix(SEMANTICS_NODE * snode, char *zop_str);

char* NestInParens(char *z_expr, bool forced);
char* NestInBrackets(char *z_expr);
char* RemovezStrParens(char *z_expr);

char* WideToASCII(const U16* w_markup);
U16* ASCIItoWide(const char* ascii, int& zlen);

#ifdef DEBUG
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
