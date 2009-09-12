// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

#ifndef FilterUtils_h
#define FilterUtils_h

#include "CmpTypes.h"
//#include "SNode.h"
#include <stdlib.h>
#include <stdio.h>

//#include "mnode.h"
#include "logmsg.h"

class Grammar;
class SEMANTICS_NODE;
struct BUCKET_REC;



struct PARAM_REC
{                       // Struct to carry a command parameter
  PARAM_REC* next;
  U32 param_ID;
  U32 param_type;
  char* ztext;
};

PARAM_REC* AppendParam(PARAM_REC* curr_list, U32 p_ID, U32 p_type, const char* zdata);

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

void AppendVarRec(VAR_REC* &curr_list, const char* v_nom, VarType v_type);
void DisposeVarList(VAR_REC* v_list);
VAR_REC* FindVarRec(VAR_REC* curr_list, const char* v_nom);


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
  MIC2MMLNODE_REC* next;
  U32 owner_ID;
  char* canonical_name;
  char* mml_markup;
};




BUCKET_REC* MakeBucketRec(U32 which_bucket, SEMANTICS_NODE* sem_child);
BUCKET_REC* MakeParentBucketRec(U32 which_bucket, SEMANTICS_NODE* sem_child);


BUCKET_REC* AppendBucketRec(BUCKET_REC* a_list, BUCKET_REC* new_a_rec);
void AppendBucketRecord(BUCKET_REC*& a_list, BUCKET_REC* new_a_rec);


BUCKET_REC* FindBucketRec(BUCKET_REC* a_list, U32 bucket_ID);
void DisposeBucketList(BUCKET_REC* arg_list);


const char* GetMarkupFromID(MIC2MMLNODE_REC* node_IDs_list, const char* targ_ID);
void DisposeIDsList(MIC2MMLNODE_REC* node_IDs_list);

MIC2MMLNODE_REC* JoinIDsLists(MIC2MMLNODE_REC* IDs_list, MIC2MMLNODE_REC* appender);



bool IsTrigArgFuncName(const Grammar* mml_entities, const char* f_nom);
bool IsReservedFuncName(const Grammar* mml_entities, const char* f_nom);


U32 NumericEntity2U32(const char *p_entity);
int CountSymbols(const char *p_chdata, int &n_entities);


INPUT_NOTATION_REC* CreateNotationRec();

SEMANTICS_NODE* PrefixToInfix(SEMANTICS_NODE* s_list);

void SetInfixPrecedence(SEMANTICS_NODE* snode);

SEMANTICS_NODE* NestInPGroup(SEMANTICS_NODE* s_list, BUCKET_REC* parent_bucket);

void FunctionToInfix(SEMANTICS_NODE* snode, char* zop_str);

char* NestInParens(char* z_expr, bool forced);
char* NestInBrackets(char* z_expr);
char* RemovezStrParens(char* z_expr);

char* WideToASCII(const U16* w_markup);
U16* ASCIItoWide(const char* ascii, int& zlen);


// dump utilities.  output sent to JBMline.out
namespace JBM {
  void DumpSList(const SEMANTICS_NODE* s_list);
}

#endif
