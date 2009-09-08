// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

#ifndef ANALYZER_H
#define ANALYZER_H

#include "CmpTypes.h"
#include "fltutils.h"
#include "MRequest.h"
#include "MResult.h"

#include <limits.h>

class DefStore;
class Grammar;
class PrefsStore;
class Tree2StdMML;
class MML2Tree;
class MNODE;
class DefInfo;

class DE_FUNC_REC;


class AnalyzerData;



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
  bool IsDefinedFunction(MNODE* mnode);

  
// 

  MIC2MMLNODE_REC* NodeIDsList() const { return m_node_IDs_list; }
  void SetNodeIDsList( MIC2MMLNODE_REC* p) { m_node_IDs_list = p; }




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

class AnalyzerData {
  private:
    const Grammar* mml_entities;
    DefStore* m_defstore;
    U32 m_curr_engine_ID;
    U32 m_curr_client_ID;
    DE_FUNC_REC* m_DE_func_names;
    DE_FUNC_REC* m_IMPLDIFF_func_names;
    SEMANTICS_NODE* DE_ind_vars;

    
    bool i_is_imaginary;
    bool j_is_imaginary;
    bool e_is_Euler;





    PrefsStore* uprefs_store;

    LOG_MSG_REC* msg_list;
    INPUT_NOTATION_REC* p_input_notation;
    MML2Tree* mml_tree_gen;



   
  public:
    const Grammar* GetGrammar() const { return mml_entities; }
    void SetGrammar(const Grammar* g) { mml_entities = g; }

    DefStore* GetDefStore() { return m_defstore; }
    void SetDefStore( DefStore* ds) { m_defstore = ds; }


    PrefsStore* GetPrefsStore() { return uprefs_store; }
    void SetPrefsStore( PrefsStore* ps ) {uprefs_store = ps; }

    LOG_MSG_REC* GetLogMsgList() { return msg_list; }
    void SetLogMsgList( LOG_MSG_REC* ml ) { msg_list = ml; }
      
    INPUT_NOTATION_REC* GetInputNotation() { return p_input_notation; }
    void SetInputNotation(INPUT_NOTATION_REC* in) { p_input_notation = in; }

    MML2Tree* GetMMLTreeGen() { return mml_tree_gen; }
    void SetMMLTreeGen(MML2Tree* m) { mml_tree_gen = m; }
    
    U32 CurrEngineID() const { return m_curr_engine_ID; }
    void SetCurrEngineID(U32 id) { m_curr_engine_ID = id; }

    U32 CurrClientID() const { return m_curr_client_ID; }
    void SetCurrClientID(U32 id) { m_curr_client_ID = id; }
    
    DE_FUNC_REC* DE_FuncNames() const { return  m_DE_func_names; }
    void SetDE_FuncNames(DE_FUNC_REC* lis) { m_DE_func_names = lis; }
    

    DE_FUNC_REC* IMPLDIFF_FuncNames() const { return  m_IMPLDIFF_func_names; }
    void SetIMPLDIFF_FuncNames(DE_FUNC_REC* lis) { m_IMPLDIFF_func_names = lis; }

    bool Get_i_is_imaginary() { return i_is_imaginary; }
    void Set_i_is_imaginary(bool b) { i_is_imaginary = b; }

    bool Get_j_is_imaginary() { return j_is_imaginary; }
    void Set_j_is_imaginary(bool b) { j_is_imaginary = b; }

    bool Get_e_is_Euler() { return e_is_Euler; }
    void Set_e_is_Euler(bool b) { e_is_Euler = b; }

    SEMANTICS_NODE* GetDE_ind_vars() { return DE_ind_vars; }
    void SetDE_ind_vars(SEMANTICS_NODE* n) {DE_ind_vars = n; }
 
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
    BT_ROW
};

enum ExpType {
  ET_POWER,
  ET_PRIMES,
  ET_INVERSE_INDICATOR,
  ET_DECORATION,
  ET_DIRECTION,
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
  POI_sum
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





DefInfo* GetDI(AnalyzerData* pData, const char* canonical_id);
void SetSnodeOwner(SEMANTICS_NODE* snode, AnalyzerData* pData);
BaseType GetBaseType(MNODE* base, bool isLHSofDef, AnalyzerData* pData);
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










#endif
