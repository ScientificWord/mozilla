// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

#ifndef TREE2STDMML_H
#define TREE2STDMML_H

#include "CmpTypes.h"
#include "mnode.h"
class Grammar;
class AnalyzerData;


typedef struct tagGROUP_INFO
{
  char opening_delim[32];
  char closing_delim[32];
  bool has_mtext;
  bool is_mod;
  int lowest_precedence;
  int operator_count;
  int separator_count;
  int n_interior_nodes;
} GROUP_INFO;



class Tree2StdMML
{
public:
  Tree2StdMML(const Grammar* mml_grammar, AnalyzerData* analyzer_data);
  ~Tree2StdMML();

  MNODE* TreeToCanonicalForm(MNODE * dMML_tree,
                             INPUT_NOTATION_REC * in_notation);
  MNODE* TreeToInterpretForm(MNODE * dMML_tree,
                             INPUT_NOTATION_REC * in_notation);
  MNODE* TreeToFixupForm(MNODE * dMML_tree, bool D_is_derivative);
  MNODE* TreeToCleanupForm(MNODE * dMML_tree);
protected:
  MNODE* ChDataToCanonicalForm(MNODE * MML_list);
  void RemoveMixedNumbers(MNODE * dMML_tree,
                          INPUT_NOTATION_REC * in_notation);
  void AddOperatorInfo(MNODE * dMML_list);
  void AddDDOperatorInfo(MNODE* dMML_list);

  void FixDotDotMN(MNODE* dMML_tree);

  MNODE* BindMixedNumbers(MNODE* dMML_list);
  MNODE* BindDegMinSec(MNODE* dMML_list);
  MNODE* BindDelimitedIntegrals(MNODE* dMML_list);
  MNODE* BindIntegral(MNODE* dMML_list);
  void FixupCapitalD(MNODE* dMML_list);
  void FixupSmalld(MNODE* dMML_list);
  MNODE* BindByOpPrecedence(MNODE* dMML_list, int high, int low);
  MNODE* BindApplyFunction(MNODE* dMML_list);

  void InsertInvisibleAddSigns(MNODE* dMML_list);
  void InsertApplyFunction(MNODE* dMML_list);
  void InsertInvisibleTimes(MNODE* dMML_list);
  void InsertAF(MNODE* dMML_list);
  void InsertIT(MNODE* dMML_list);
  MNODE *FixMFENCEDs(MNODE* dMML_tree);
  void FixInvisibleFences(MNODE* dMML_tree);
  MNODE *FixAdjacentMNs(MNODE* dMML_tree);
  void BindDelimitedGroups(MNODE* dMML_tree);
  void BindScripts(MNODE* dMML_tree);

  MNODE* FinishFixup(MNODE* dMML_tree);
  MNODE* InfixDivideToMFRAC(MNODE * dMML_tree);
  MNODE* RemoveRedundantMROWs(MNODE * MML_list);

  MNODE* RemoveEmptyTags(MNODE * MML_list);
  MNODE* RemoveMPADDEDs(MNODE * MML_list);
  MNODE* RemoveMatrixDelims(MNODE * MML_list,
                            INPUT_NOTATION_REC * in_notation);
  MNODE *ExtractItems(MNODE * body, int n_interior_nodes, int n_commas,
                      MNODE * new_parent);
  void RemoveMSTYLEs(MNODE* MML_list);
  void RemoveMSTYLEnode(MNODE* mstyle);
  void RemoveHSPACEs(MNODE* MML_list);
  void RemoveIT_and_AF(MNODE* MML_list);


  char *ChdataToString(const char *p_chdata);

  void GetFenceInfo(MNODE * MML_group_node, GROUP_INFO & gi);
  MNODE *GetMatchingFenceInfo(MNODE * MML_group_node, GROUP_INFO & gi);
  MNODE *GetScriptedFenceInfo(MNODE * MML_group_node, GROUP_INFO & gi);
  void GetGroupInsideInfo(MNODE * MML_cont, MNODE * rfence, GROUP_INFO & gi);

  bool FunctionHasArg(MNODE * mml_func_node, int & n_arg_nodes, bool & is_delimited);
  bool FuncTakesTrigArgs(MNODE * mml_func_node);

  bool IsFenceMO(MNODE * mml_node);
  bool IsLeftFenceMO(MNODE * mml_node);
  bool IsRightFenceMO(MNODE * mml_node);
  MNODE * GetBaseFence(MNODE * mml_node);
  bool IsScriptedFenceMO(MNODE * mml_node);
  bool IsEmptyMO(MNODE * mml_node);
  bool MMLDelimitersMatch(GROUP_INFO & gi);
  bool IsGroup(GROUP_INFO & gi);
  bool IsFence(GROUP_INFO & gi);
  bool IsEnclosedList(GROUP_INFO & gi);
  bool IsDelimitedGroup(MNODE * mml_node);
  void LookupMOInfo(MNODE * mml_node);
  void LookupEmbellishedMO(MNODE * mml_node);
  bool GetBasePrecedence(MNODE * mml_node, int &precedence, OpIlk &op_ilk);

  int CountTrigargNodes(MNODE * mnode);
  bool NodeInTrigargList(MNODE * mnode, bool & is_op);

  bool NodeIsNumber(MNODE * mml_node);
  bool NodeIsTrueNumber(MNODE * mml_node);
  bool NodeIsRationalFraction(MNODE * mml_node);
  
  MNODE *GetBaseFunction(MNODE * mml_node);
  bool NodeIsOperator(MNODE * mml_node);
  bool NodeIsIntegral(MNODE * mml_node);
  int GetIntegralCount(MNODE * mml_node);
  bool NodeIsDifferential(MNODE * mml_node, bool & nested);
  bool NodeIsCapitalDifferential(MNODE * mml_node);
  bool NodeIsVariableList(MNODE * mml_node);
  bool NodeIsFactor(MNODE * mml_node);
  bool NodeIsDiffNumerator(MNODE * mml_node);
  bool NodeIsDiffDenominator(MNODE * mml_node);

  MNODE *MakeItem(MNODE * l_anchor, MNODE * r_anchor);
  MNODE *MakeMROW(MNODE * l_anchor, MNODE * r_anchor);
  MNODE *MakeSCRIPT(MNODE * l_anchor, MNODE * r_anchor);

  MNODE *PermuteTreeToMFENCED(MNODE * opening_mo, GROUP_INFO & gi);
  bool IsAllDigits(MNODE * num, int & n_digits);
  bool IsNumberOrPeriod(MNODE * num);
  bool HasPeriod(MNODE * num);
  void PermuteMixedNumber(MNODE * num);
  void PermuteDifferential(MNODE * diff);
  void PermuteCapitalDifferential(MNODE * diff);
  void PermuteDiffNumerator(MNODE * diff);
  void PermuteDiffDenominator(MNODE * diff);

  bool NeedsInvisiblePlus(MNODE * dMML_mrow);

  ATTRIB_REC* StackAttr(ATTRIB_REC* attr_stack, const char* attr_nom,
                        const char* attr_val);
  ATTRIB_REC* UnstackAttr(ATTRIB_REC * attr_stack);

  void MoveAttrsToChildren(MNODE* mml_list);
  void InstallStackedAttr(MNODE* mml_node, ATTRIB_REC * attr_stack);

private:
  const Grammar* const mml_entities;
  AnalyzerData* my_analyzer_data;

  ATTRIB_REC *mv_stack;
  ATTRIB_REC *lt_stack;

  bool mDisDerivative;
};





#endif
