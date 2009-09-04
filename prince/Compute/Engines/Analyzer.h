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

class DE_FUNC_REC;






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


  U32 CurrClientID() const { return m_curr_client_ID; }
  void SetCurrClientID(U32 id) { m_curr_client_ID = id; }


  U32 CurrEngineID() const { return m_curr_engine_ID; }
  void SetCurrEngineID(U32 id) { m_curr_engine_ID = id; }

  U32 CmdID() const { return m_cmd_ID; }
  void SetCmdID(U32 id) { m_cmd_ID = id; }

  const char* ScrStr() const { return m_z_scr_str; }
  void SetScrStr(const char* s) { m_z_scr_str = s; }

 
  DE_FUNC_REC* DE_FuncNames() const { return  m_DE_func_names; }
  void SetDE_FuncNames(DE_FUNC_REC* lis) { m_DE_func_names = lis; }

  DE_FUNC_REC* IMPLDIFF_FuncNames() const { return  m_IMPLDIFF_func_names; }
  void SetIMPLDIFF_FuncNames(DE_FUNC_REC* lis) { m_IMPLDIFF_func_names = lis; }

  DefStore* GetDefStore() { return m_defstore; }
  void SetDefStore( DefStore* ds) { m_defstore = ds; }

  const Grammar* GetGrammar() { return mml_entities; }
  //Grammar is only set at construction... void SetGrammar(Grammar* g) { mml_entities = g; }


  PrefsStore* GetPrefsStore() { return uprefs_store; }
  void SetPrefsStore( PrefsStore* ps ) {uprefs_store = ps; }

  LOG_MSG_REC* GetLogMsgList() { return msg_list; }
  void SetLogMsgList( LOG_MSG_REC* ml ) { msg_list = ml; }
    
  INPUT_NOTATION_REC* GetInputNotation() { return p_input_notation; }
  void SetInputNotation(INPUT_NOTATION_REC* in) { p_input_notation = in; }

  MML2Tree* GetMMLTreeGen() { return mml_tree_gen; }
  void SetMMLTreeGen(MML2Tree* m) { mml_tree_gen = m; }

  
  Tree2StdMML* GetCanonicalTreeGen() { return CanonicalTreeGen; }
  void SetCanonicalTreeGen(Tree2StdMML* t) { CanonicalTreeGen = t; }

  SEMANTICS_NODE* GetDE_ind_vars() { return DE_ind_vars; }
  void SetDE_ind_vars(SEMANTICS_NODE* n) {DE_ind_vars = n; }

  SEMANTICS_NODE* GetIMPLDIFF_ind_var() { return IMPLDIFF_ind_var; }
  void SetIMPLDIFF_ind_var(SEMANTICS_NODE* n)  {IMPLDIFF_ind_var = n; }

  bool Get_i_is_imaginary() { return i_is_imaginary; }
  void Set_i_is_imaginary(bool b) { i_is_imaginary = b; }

  bool Get_j_is_imaginary() { return j_is_imaginary; }
  void Set_j_is_imaginary(bool b) { j_is_imaginary = b; }

  bool Get_e_is_Euler() { return e_is_Euler; }
  void Set_e_is_Euler(bool b) { e_is_Euler = b; }

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


private:
  const Grammar* const mml_entities;
  DefStore* m_defstore;
  PrefsStore* uprefs_store;

  LOG_MSG_REC* msg_list;
  INPUT_NOTATION_REC* p_input_notation;
  MML2Tree* mml_tree_gen;

  Tree2StdMML* CanonicalTreeGen;

  SEMANTICS_NODE* DE_ind_vars;
  SEMANTICS_NODE* IMPLDIFF_ind_var;


  MIC2MMLNODE_REC* m_node_IDs_list;
  U32 m_curr_client_ID;
  U32 m_curr_engine_ID;

  const char* m_z_scr_str;

  DE_FUNC_REC* m_DE_func_names;
  DE_FUNC_REC* m_IMPLDIFF_func_names;
  U32 m_cmd_ID;

  bool i_is_imaginary;
  bool j_is_imaginary;
  bool e_is_Euler;
  bool log_is_base10;
  bool overbar_conj;
  bool dot_is_derivative;
  bool prime_is_derivative;
  bool D_is_derivative;

};

#endif
