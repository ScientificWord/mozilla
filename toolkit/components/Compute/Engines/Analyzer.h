// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

#ifndef ANALYZER_H
#define ANA LYZER_H

#include "CmpTypes.h"
#include "MRequest.h"
#include "AnalyzerData.h"

#include <limits.h>

class DefStore;
class Grammar;
class PrefsStore;
class Tree2StdMML;
class MML2Tree;
class MNODE;
class DefInfo;

class DE_FUNC_REC;
struct LOG_MSG_REC;
class MIC2MMLNODE_REC;


class AnalyzerData;
class MathResult;



class Analyzer
{
public:
  Analyzer(const Grammar* mml_grammar, PrefsStore* up_store);
  ~Analyzer();

  void SetInputPrefs(DefStore* ds, U32 engine_ID);

  SEMANTICS_NODE* BuildSemanticsTree(MathServiceRequest& msr,
                                     MathResult& mr,
                                     const char* src_markup,
                                     MNODE* dMML_tree,
                                     U32 curr_cmd_ID,
                                     INPUT_NOTATION_REC* p_input_notation);

  LOG_MSG_REC* GetMsgs();
  MIC2MMLNODE_REC* GetBackMap();
  void TreeToCleanupForm(MNODE* dMML_tree);
  void TreeToFixupForm(MNODE* dMML_tree);

  
// 

  MIC2MMLNODE_REC* NodeIDsList() const { return m_node_IDs_list; }
  void SetNodeIDsList( MIC2MMLNODE_REC* p) { m_node_IDs_list = p; }

  void AppendIDList(char* mml_canonical_name, MNODE* mml_node);


  U32 CmdID() const { return m_cmd_ID; }
  void SetCmdID(U32 id) { m_cmd_ID = id; }

  const char* ScrStr() const { return m_z_scr_str; }
  void SetScrStr(const char* s) { m_z_scr_str = s; }

  
  Tree2StdMML* GetCanonicalTreeGen() { return CanonicalTreeGen; }
  void SetCanonicalTreeGen(Tree2StdMML* t) { CanonicalTreeGen = t; }


  SEMANTICS_NODE* GetIMPLDIFF_ind_var() { return IMPLDIFF_ind_var; }
  void SetIMPLDIFF_ind_var(SEMANTICS_NODE* n)  {IMPLDIFF_ind_var = n; }


  bool Get_log_is_base10() { return log_is_base10; }
  void Set_log_is_base10(bool b) { log_is_base10 = b; }

  bool Get_overbar_conj() { return overbar_conj; }
  void Set_overbar_conj(bool b) { overbar_conj = b; }

  bool Get_dot_is_derivative() { return dot_is_derivative; }
  void Set_dot_is_derivative(bool b) { dot_is_derivative = b; }

  bool Get_prime_is_derivative() { return prime_is_derivative; }
  void Set_prime_is_derivative(bool b) { prime_is_derivative = b; }

  bool Get_D_is_derivative() { return D_is_derivative; }
  void Set_D_is_derivative(bool b) { D_is_derivative = b; }

  AnalyzerData* GetAnalyzerData() { return m_pData; }

private:

  AnalyzerData* m_pData;



  Tree2StdMML* CanonicalTreeGen;

  SEMANTICS_NODE* IMPLDIFF_ind_var;


  MIC2MMLNODE_REC* m_node_IDs_list;

  const char* m_z_scr_str;

  U32 m_cmd_ID;

  bool log_is_base10;
  bool overbar_conj;
  bool dot_is_derivative;
  bool prime_is_derivative;
  bool D_is_derivative;

};



enum BaseType {
    BT_UNKNOWN,
    BT_VARIABLE,
    BT_FUNCTION,
    BT_NUMBER,
    BT_FENCED,
    BT_OPERATOR,
    BT_UNIT,
    BT_MATRIX,
    BT_TRANSFORM,
    BT_MOVER,
    BT_SUBARG_FUNCTION,
    BT_ROW,
    BT_SUB,
    BT_SUP
};

enum ExpType {
  ET_POWER,
  ET_PRIMES,
  ET_INVERSE_INDICATOR,
  ET_DECORATION,
  ET_DIRECTION,
  ET_DEGREE,
  ET_CONJUGATE_INDICATOR,
  ET_VARIABLE,
  ET_NUMBER,
  ET_EXPRESSION,
  ET_TRANSPOSE_INDICATOR,
  ET_HTRANSPOSE_INDICATOR,
  ET_PARENED_PRIMES_COUNT
};

enum IdentIlk {
  MI_none ,
  MI_pi,
  MI_imaginaryunit,
  MI_Eulere,
  MI_infinity,
  MI_Eulergamma,
  MI_Laplace,
  MI_Fourier,
  MI_function,
  MI_variable
};


enum PrefixOpIlk {
  POI_none,
  POI_listop,  //gcd,lcm,max,min
  POI_det,
  POI_distribution,
  POI_Dirac, // + Heaviside
  POI_gradient,
  POI_divergence,
  POI_curl,
  POI_Laplacian,
  POI_integral,
  POI_sum,
  POI_function
};


enum AccentType {
  OT_NONE,
  OT_HAT,
  OT_CHECK,
  OT_TILDE,
  OT_ACUTE,
  OT_GRAVE,
  OT_BREVE,
  OT_BAR,
  OT_MATHRING,
  OT_DOT,
  OT_DDOT,
  OT_DDDOT,
  OT_DDDDOT,
  OT_VEC
};










class DE_FUNC_REC
{
  public:
    DE_FUNC_REC *next;
    char *zfunc_canon_name;
    char *zfunc_src_name;

};





void SetSnodeOwner(SEMANTICS_NODE* snode, AnalyzerData* pData);
BaseType GetBaseType(MNODE* base, bool isLHSofDef, AnalyzerData* pData, const Grammar* mml_entities);
char* GetCanonicalIDforMathNode(const MNODE* mml_node, const Grammar* mml_entities);

bool  LocateFuncRec(const DE_FUNC_REC* f_list,
                    const char* canon_name,
                    const char* src_name);

DE_FUNC_REC* AppendFuncName(DE_FUNC_REC* f_list, 
                            char* f_name,
                            char* src_name);

ExpType GetExpType(BaseType base_type, MNODE* exp, const Grammar* mml_entities);

ExpType GetSubScriptType(MNODE* script_schemata, BaseType base_type, MNODE* exp);
bool IsInverseIndicator(MNODE* exp, const Grammar* mml_entities);
void OverrideInvisibleTimesOnLHS(MNODE* dMML_tree);
void OverridePrefsOnLHS(MNODE* dMML_tree, AnalyzerData* pData);


IdentIlk GetMIilk(char* mi_canonical_str, 
                  DefInfo* di, 
                  MNODE* m_node, 
                  bool isLHSofDef, 
                  const Grammar* mml_entities,
                  bool i_is_imaginary,
                  bool j_is_imaginary,
                  bool e_is_Euler);


bool IdentIsConstant(IdentIlk ilk);
IdentIlk GetMSIilk(char* msi_class);
bool IsWhiteText(const char* the_text);
int GetVarLimType(char* op_name, MNODE* base);
BUCKET_REC* AddVarToBucket(U32 bucket_ID, SEMANTICS_NODE* ind_var_list, AnalyzerData* pData);
void ExtractVariables(SEMANTICS_NODE * s_tree, AnalyzerData* pData);
PrefixOpIlk GetPrefixOpCode(const char* op_name, SemanticVariant& n_integs, const Grammar* mml_entities);
AccentType GetAboveType(BaseType base_type, MNODE* accent, const Grammar* mml_entities);

char* GetFuncNameFromSubSup(MNODE* msubsup, const char** src_name, AnalyzerData* pData);
char* GetFuncNameFromFrac(MNODE* mfrac, const char** src_name, const Grammar* mml_entities);
char* GetFuncNameFromSub(MNODE* msub, const char** src_name, const Grammar* mml_entities);

char* GetFuncNameFromSup(MNODE* msup, const char** src_name, AnalyzerData* pData);

void DetermineODEFuncNames(MNODE* dMML_tree, AnalyzerData* pData);
void DeterminePDEFuncNames(MNODE* dMML_tree, AnalyzerData* pData);
void DisposeODEFuncNames(DE_FUNC_REC* DE_func_names);

void AddPrimesCount(SEMANTICS_NODE* snode, MNODE* primes);

void AppendODEfuncs(SEMANTICS_NODE* rv, DE_FUNC_REC* DE_func_names, AnalyzerData* pData);

MIC2MMLNODE_REC* AppendIDRec(MIC2MMLNODE_REC* node_IDs_list,
                             U32 client_ID, 
                             char *obj_name,
                             MNODE* mml_node, 
                             const char *zsrc);

bool NodeIsFunction(MNODE* mml_node, const Grammar* mml_entities, AnalyzerData* my_analyzer_data);











#endif
