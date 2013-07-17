// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

#ifndef COMPENGINE_H
#define COMPENGINE_H

#include "CmpTypes.h"
#include "fltutils.h"
#include "mnode.h"
#include "msiIEngineWrapper.h"
#include "nsCOMPtr.h"
#include "nsILocalFile.h"

struct ENG_ATTR_REC
{
  ENG_ATTR_REC* next;
  int ID;
  char* value;
};

class MathServiceRequest;
class MathResult;

class Grammar;
class MML2Tree;
class Analyzer;
class STree2MML;
class DefStore;
class PrefsStore;



class CompEngine
{
public:
  CompEngine(Grammar * ID_dBase, Grammar * NOM_dBase,
             Grammar * mathml_grammar, PrefsStore * up_store);
  ~CompEngine();

  void StopProcessor();

  bool InitUnderlyingEngine(Grammar * install_dbase, nsILocalFile * baseDir, MathResult & mr);

  void Execute(MathServiceRequest & msr, MathResult & mr);

  bool SetEngineAttr(int attr_ID, int i_val, const char* s_val);
  const char *GetEngineAttr(int attr_ID);
  void ClearEngineStrs();

  void ReleaseClient(U32 client_ID, U32 engine_ID, DefStore * defstore);

  // The following calls provide an interface for an embedded engine
  bool GetdBaseNamedRecord(const char *bin_name, const char *rec_name,
                               const char **production, U32 & ID,
                               U32 & subID);
  bool GetdBaseIDRecord(const char *bin_name, U32 ID, U32 subID,
                            const char **production, char *name_buffer,
                            size_t lim);

private:
  bool LoadEngWrapperDLL(const char *path);

  void ResetState(bool use_grammar_values);
  void RecordEngineAttr(int ID, const char *new_value);

  void ReportEngineMessage(const char *msg, U32 msg_id);

  void ConvertTreeToList(SEMANTICS_NODE * semantics_tree, U32 cmd_ID);
  void ConvertTreeToPDE(SEMANTICS_NODE * semantics_tree);
  void ConvertTreeToODE(SEMANTICS_NODE * semantics_tree,
                        U32 cmd_ID, int & n_boundary_conditions);
  void ConvertTreeToRecursion(SEMANTICS_NODE * semantics_tree,
                              INPUT_NOTATION_REC * p_input_notation);
  void ConvertTreeToSimplex(SEMANTICS_NODE * semantics_tree, U32 cmd_ID);
  void ConvertTreeToNumericSys(SEMANTICS_NODE * semantics_tree);
  int ConvertTreeToSolveSys(SEMANTICS_NODE * semantics_tree);
  const char *ConvertTreeToDef(MathServiceRequest & msr,
                               SEMANTICS_NODE * semantics_tree,
                               bool &generic_def, int & error_code);
  char *ConvertTreeToUnDef(SEMANTICS_NODE * semantics_tree);
  void ConvertSetToPList(SEMANTICS_NODE * semantics_tree);

  bool IsFunction(SEMANTICS_NODE * left_side);
  bool IsVariable(SEMANTICS_NODE * left_side);
  const char *ConvertTreeToFuncDef(SEMANTICS_NODE * semantics_tree);
  const char *ConvertTreeToSymbolAssign(SEMANTICS_NODE * semantics_tree);

  SEMANTICS_NODE *RemovePGroup(SEMANTICS_NODE * s_node);

  bool AdjustSemTree(MathServiceRequest & msr,
                         SEMANTICS_NODE * s_tree,
                         U32 cmd_ID, int & error_code);
  bool OKtoConcat(SEMANTICS_NODE * s_matrix, int & n_rows);
  void ConcatMatrixITMatrix(SEMANTICS_NODE * s_invistimes, int n_rows);
  void ConcatMatrixMatrix(SEMANTICS_NODE * s_matrix);
  bool OKtoStack(SEMANTICS_NODE * s_matrix, int & n_rows);
  void StackMatrices(SEMANTICS_NODE * s_matrix, int n_rows);
  void ExtractCells(SEMANTICS_NODE * s_times_op, BUCKET_REC ** bucket_lists,
                    U32 n_rows);
  SEMANTICS_NODE *LocateMatrixInBucket(BUCKET_REC * b_rover);

  bool IsBoundaryCondition(SEMANTICS_NODE * c_content);
  void GetODEFuncNames(SEMANTICS_NODE * s_line, char *zODE_names);

  void RetrieveEngineStrs(MathResult & mr);
  void SaveBackMap(MIC2MMLNODE_REC * new_IDs);
  void RemoveBackMapEntry(U32 curr_client_ID, const char *def_canon_ID);
  void ListToMatrix(BUCKET_REC * var_val_bucket);
  void MatrixToList(BUCKET_REC * math_bucket);

  void AddBasisVariables(MathServiceRequest & msr);

  void AddRecurFuncNode(SEMANTICS_NODE * s_recur, BUCKET_REC * b);
  SEMANTICS_NODE *LocateRecurFuncInEqn(SEMANTICS_NODE * s_equal);

  bool IsNumericFrac(SEMANTICS_NODE * semantics_tree);
  void GetBasisVariables(char *dest);

  bool IsSubVariableList(SEMANTICS_NODE * s_node, int & n_args);
  bool IsVariableInSubscript(SEMANTICS_NODE * s_node);

  BUCKET_REC *ExtractCommaList(SEMANTICS_NODE * s_node, BUCKET_REC * b_list);
  bool AllDigits(char *z_data);
  bool IsMOD(SEMANTICS_NODE * semantics_tree);

  U32 GetDefType(SEMANTICS_NODE * semantics_tree, char **arg_list,
                 U32 & n_subscripted_args);

  void FixiArgument(BUCKET_REC * header_bucket);
  void ImagineiToVari(SEMANTICS_NODE * s_list);
  void FixIndexedVars(SEMANTICS_NODE * s_list, BUCKET_REC * arg_bucket_list);
  
  bool DefAllowed(SEMANTICS_NODE * root);

  int GetNColumns(MathServiceRequest & msr, int & error_code);

  bool IsSeparatorOp(SEMANTICS_NODE * s_rover);
  SEMANTICS_NODE *InsertPGroup(SEMANTICS_NODE * s_math);

  void SetInfixPrecedences(SEMANTICS_NODE * semantics_list);

  const char *QVarToFuncHeader(SEMANTICS_NODE * left_side);

  Grammar *id_dBase;
  Grammar *nom_dBase;
  PrefsStore *uprefs_store;

  MML2Tree *mml_tree_gen;
  Analyzer *semantic_analyzer;
  MNODE *dMML_tree;
  STree2MML *StreeToMML;

  MIC2MMLNODE_REC *curr_IDs_2mml;
  MIC2MMLNODE_REC *def_IDs_2mml;

  ENG_ATTR_REC *engine_attrs;
  char *mml_VecBasisVars;

  nsCOMPtr<msiIEngineWrapper> wrapper;
};

#endif
