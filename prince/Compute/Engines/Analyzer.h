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

struct DE_FUNC_REC;

enum OpMatrixIntervalType;
enum IdentIlk;
enum BaseType;
enum ExpType;
enum AccentType;
enum PrefixOpIlk;
enum OpOrderIlk;





class Analyzer
{
public:
  Analyzer(Grammar * mml_grammar, PrefsStore * up_store);
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



  const char* ScrStr() const { return m_z_scr_str; }
  void SetScrStr(const char* s) { m_z_scr_str = s; }

  
 
  DE_FUNC_REC* DE_FuncNames() const { return  m_DE_func_names; }
  void SetDE_FuncNames(DE_FUNC_REC* lis) { m_DE_func_names = lis; }

  DE_FUNC_REC* IMPLDIFF_FuncNames() const { return  m_IMPLDIFF_func_names; }
  void SetIMPLDIFF_FuncNames(DE_FUNC_REC* lis) { m_IMPLDIFF_func_names = lis; }


  Grammar* mml_entities;
  PrefsStore* uprefs_store;
  DefStore* defstore;
  
  LOG_MSG_REC *msg_list;
  INPUT_NOTATION_REC *p_input_notation;

  MML2Tree* mml_tree_gen;
  Tree2StdMML* CanonicalTreeGen;

  SEMANTICS_NODE* DE_ind_vars;
  SEMANTICS_NODE* IMPLDIFF_ind_var;
  

  bool i_is_imaginary;
  bool j_is_imaginary;
  bool e_is_Euler;
  bool log_is_base10;
  bool overbar_conj;
  bool dot_is_derivative;
  bool prime_is_derivative;
  bool D_is_derivative;

  U32 cmd_ID;

private:

  MIC2MMLNODE_REC* m_node_IDs_list;
  U32 m_curr_client_ID;
  U32 m_curr_engine_ID;

  const char* m_z_scr_str;

  DE_FUNC_REC* m_DE_func_names;
  DE_FUNC_REC* m_IMPLDIFF_func_names;


};

#endif
