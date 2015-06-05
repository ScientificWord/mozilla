// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

#ifndef MuANALYZER_H
#define MuANALYZER_H
             
#include "CmpTypes.h"
#include "fltutils.h"
//#include "MuWrap.h"
#include <limits.h>

class MuPADWrapper;
class MNODE;

class MuAnalyzer
{
public:
  MuAnalyzer(Grammar* theGrammar);
  ~MuAnalyzer();

  SEMANTICS_NODE *BuildSemanticsTree(MNODE * dMML_tree, U32 cmd_ID,
                                     ClientAliases_REC * name_map);
  LOG_MSG_REC *GetMsgs();

private:
  enum {ALL_NODES = INT_MAX};
  SEMANTICS_NODE* GetSemanticsList(MNODE* eng_list, BUCKET_REC * parent, int mml_node_lim);
  SEMANTICS_NODE* GetSemanticsList(MNODE* eng_list, BUCKET_REC* parent);
  SEMANTICS_NODE *SNodeFromMNodes(MNODE * eng_node, int & eng_nodes_done);
  void SNodeFromOneMNode(SEMANTICS_NODE * snode,
                         MNODE * eng_node, int & local_nodes_done);

  MNODE *InfixToPrefix(MNODE * eng_tree, char *z_op);

  void AnalyzePARENGROUP(MNODE * eng_node, SEMANTICS_NODE * rv,
                         int & local_nodes_done);
  void AnalyzeBRACKETLIST(MNODE * eng_node, SEMANTICS_NODE * rv,
                          int & local_nodes_done);
  void AnalyzeNUMBER(MNODE * eng_node, SEMANTICS_NODE * rv,
                     int & local_nodes_done);
  void AnalyzeSTRING(MNODE * eng_node, SEMANTICS_NODE * rv,
                     int & local_nodes_done);
  void AnalyzeFUNCTION(MNODE * eng_node, SEMANTICS_NODE * rv,
                       int & local_nodes_done);
  void AnalyzeVARIABLE(MNODE * eng_node, SEMANTICS_NODE * rv,
                       int & local_nodes_done);
  void AnalyzeOPERATOR(MNODE * eng_node, SEMANTICS_NODE * rv,
                       int & local_nodes_done);
  void AnalyzeCONSTANT(MNODE * eng_node, SEMANTICS_NODE * rv,
                       int & local_nodes_done);

  void AnalyzeSUBSTITUTION(MNODE * eng_node, SEMANTICS_NODE * rv,
                       int & local_nodes_done);

  void AnalyzeBOOLEAN(MNODE * eng_node, SEMANTICS_NODE * rv,
                      int & local_nodes_done);
  void AnalyzeFRACTION(MNODE * eng_node, SEMANTICS_NODE * rv,
                       int & local_nodes_done);
  void AnalyzeSUPERSCRIPT(MNODE * eng_node, SEMANTICS_NODE * rv,
                          int & local_nodes_done);
  void AnalyzeMIXEDNUM(MNODE * eng_node, SEMANTICS_NODE * rv,
                       int & local_nodes_done);

  void AnalyzeSURD(MNODE * func_node, SEMANTICS_NODE * snode,
                   int & local_nodes_done);
  void AnalyzeDIFF(MNODE * func_node, SEMANTICS_NODE * snode,
                   int & local_nodes_done);
  void AnalyzeARRAY(MNODE * func_node, SEMANTICS_NODE * snode,
                    int & local_nodes_done);
  void AnalyzeVECTOR(MNODE * eng_node, SEMANTICS_NODE * rv,
                     int & local_nodes_done);

  void AnalyzeSET(MNODE * eng_node, SEMANTICS_NODE * rv,
                  int & local_nodes_done);
  void AnalyzeARGLIST(MNODE * eng_node, SEMANTICS_NODE * rv,
                      int & local_nodes_done);
  void AnalyzePRECGROUP(MNODE * eng_node, SEMANTICS_NODE * rv,
                        int & local_nodes_done);
  void AnalyzeCOMPONENT(MNODE * eng_node, SEMANTICS_NODE * snode,
                        int & local_nodes_done);
  void AnalyzeCONJUGATE(MNODE * func_node, SEMANTICS_NODE * snode,
                        int & local_nodes_done);
  void AnalyzeDERIVATIVE(MNODE * func_node, SEMANTICS_NODE * snode,
                         int & local_nodes_done);
  void AnalyzeEXPONENTIAL(MNODE * func_node, SEMANTICS_NODE * snode,
                          int & local_nodes_done);
  void AnalyzeINTEGRAL(MNODE * func_node, SEMANTICS_NODE * snode,
                       int & local_nodes_done);
  void AnalyzeSUM(MNODE * func_node, SEMANTICS_NODE * snode,
                  int & local_nodes_done);
  void AnalyzeNO_OP(MNODE * func_node, SEMANTICS_NODE * snode,
                    int & local_nodes_done);
  void AnalyzeBINOMIAL(MNODE * func_node, SEMANTICS_NODE * snode,
                       int & local_nodes_done);
  void AnalyzePIECEWISE(MNODE * func_node, SEMANTICS_NODE * snode,
                        int & local_nodes_done);
  void AnalyzeINTERVAL(MNODE * func_node, SEMANTICS_NODE * snode,
                       int & local_nodes_done);
  void AnalyzeLIMIT(MNODE * func_node, SEMANTICS_NODE * snode,
                    int & local_nodes_done);

  BUCKET_REC *GetCellsFromROW(MNODE * row_container, int & col_tally);

  NAME2AKA_REC *EngNameToCanonicalName(const char *targ_alias);

  void FixMapleCholesky(SEMANTICS_NODE * s_list);
  SEMANTICS_NODE *FixMapleEigenVect(SEMANTICS_NODE * s_tree);
  void FixMapleJordanForm(SEMANTICS_NODE * s_list);
  void FixMapleSimplexStandardize(SEMANTICS_NODE * s_list);
  SEMANTICS_NODE *FixMapleSimplexDual(SEMANTICS_NODE * s_list);
  SEMANTICS_NODE *FixQuantileResult(SEMANTICS_NODE * s_list);
  SEMANTICS_NODE *FixDefinitenessTests(SEMANTICS_NODE * s_list);

  SEMANTICS_NODE *RemoveParens(SEMANTICS_NODE * expr);
  SEMANTICS_NODE *RemoveRedundantPrecGroups(SEMANTICS_NODE * s_node);

  bool IsText(const char *var_nom);
  bool IsArrayDimen(MNODE * arg, U32 & n_start, U32 & n_end);

  bool IsEngineToken(const char *eng_token);

  void AddPrimesCount(SEMANTICS_NODE * snode, int n_primes);
  bool IsDerivative(MNODE * func_node, int & n_primes);
  MNODE *LocateDiffVar(MNODE * deriv_node);

  Grammar* myGrammar;
  ClientAliases_REC* mangle_info;
  LOG_MSG_REC* msg_list;
  U32 curr_cmd_ID;
};

#endif
