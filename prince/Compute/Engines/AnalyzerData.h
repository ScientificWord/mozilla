#ifndef ANALYZERDATA_H
#define ANALYZERDATA_H


#include "iCmpTypes.h"

class Grammar;
class DefStore;
class DefInfo;
class DE_FUNC_REC;
class PrefsStore;
class SEMANTICS_NODE;
struct LOG_MSG_REC;
class INPUT_NOTATION_REC;
class MML2Tree;
class MNODE;


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


    bool IsDefinedFunction(MNODE* pNode);

    DefInfo* GetDI(const char* canonical_id);
    
 
};


#endif

