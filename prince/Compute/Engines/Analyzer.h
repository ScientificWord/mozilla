// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

#ifndef ANALYZER_H
#define ANALYZER_H

#include "../CmpTypes.h"
#include "fltutils.h"
#include "../MRequest.h"
#include "../MResult.h"

#include <limits.h>

class DefStore;
class Grammar;
class PrefsStore;
class Tree2StdMML;
class MML2Tree;

typedef struct tagDE_FUNC_REC
{
  tagDE_FUNC_REC *next;
  char *zfunc_canon_name;
  char *zfunc_src_name;
} DE_FUNC_REC;

class Analyzer
{
public:
  Analyzer(Grammar * mml_grammar, PrefsStore * up_store);
  ~Analyzer();

  void SetInputPrefs(DefStore* ds, U32 engine_ID);
  SEMANTICS_NODE *BuildSemanticsTree(MathServiceRequest& msr,
                                     MathResult& mr,
                                     const char* src_markup,
                                     MNODE* dMML_tree,
                                     U32 curr_cmd_ID,
                                     INPUT_NOTATION_REC* p_input_notation);
  LOG_MSG_REC* GetMsgs();
  MIC2MMLNODE_REC* GetBackMap();
  void TreeToCleanupForm(MNODE * dMML_tree);
  void TreeToFixupForm(MNODE * dMML_tree);
  bool IsDefinedFunction(MNODE * mnode);

private:
  enum {ALL_NODES = INT_MAX};
  SEMANTICS_NODE* GetSemanticsList(MNODE* dMML_list, BUCKET_REC* parent,
                                   int mml_node_lim, bool isLHSofDef);
  SEMANTICS_NODE* GetSemanticsList(MNODE* dMML_list, BUCKET_REC* parent);
  SEMANTICS_NODE* GetDefSList(MNODE* dMML_list, BUCKET_REC* parent,
                              int mml_node_lim, int & error_code);
  SEMANTICS_NODE* SNodeFromMNodes(MNODE* mml_node, int& mml_nodes_done,
                                  bool isLHSofDef);

  void AnalyzeMN(MNODE* mml_mn, SEMANTICS_NODE* info);
  void AnalyzeMI(MNODE* self, SEMANTICS_NODE* info,
                 int& nodes_done, bool isLHSofDef);
  void AnalyzeMO(MNODE* self, SEMANTICS_NODE* info, int& nodes_done);
  void AnalyzeMTEXT(MNODE* mml_mtext_node, SEMANTICS_NODE * snode,
                    int& nodes_done);
  void AnalyzeMFRAC(MNODE* mml_node, SEMANTICS_NODE* info,
                    int& nodes_done);
  void AnalyzeMSQRT(MNODE* mml_node, SEMANTICS_NODE* info,
                    int& nodes_done);
  void AnalyzeMROOT(MNODE* mml_node, SEMANTICS_NODE* info,
                    int& nodes_done);
  void AnalyzeMFENCED(MNODE * mml_node, SEMANTICS_NODE* info,
                      int& nodes_done);
  void AnalyzeMSUP(MNODE* mml_msup_node, SEMANTICS_NODE* info,
                   int& nodes_done, bool isLHSofDef);
  void AnalyzeMSUB(MNODE * mml_msup_node, SEMANTICS_NODE* info,
                   int& nodes_done, bool isLHSofDef);
  void AnalyzeMUNDER(MNODE * mml_munder_node, SEMANTICS_NODE* info,
                     int& nodes_done, bool isLHSofDef);
  void AnalyzeMOVER(MNODE * mml_munder_node, SEMANTICS_NODE* info,
                    int& nodes_done, bool isLHSofDef);
  void AnalyzeMTABLE(MNODE* mml_mtable_node,
                     SEMANTICS_NODE* info, int& nodes_done);
  void AnalyzeMUNDEROVER(MNODE* mml_munder_node, SEMANTICS_NODE* info,
                         int& nodes_done, bool isLHSofDef);
  void AnalyzeMSUBSUP(MNODE * mml_munder_node, SEMANTICS_NODE* info,
                      int& nodes_done, bool isLHSofDef);
  void TranslateEmbellishedOp(MNODE* mml_embellop_node,
                              SEMANTICS_NODE* snode, int& nodes_done);
  void AnalyzeMixedNum(MNODE* mml_mn, SEMANTICS_NODE* s_node);
  void AnalyzePrimed(MNODE* mml_msup, SEMANTICS_NODE* s_node,
                     int& nodes_done);
  void AnalyzeDotDerivative(MNODE* mml_mover, int n_dots,
                            SEMANTICS_NODE* s_node, int& nodes_done);
  void AnalyzeSubscriptedFunc(MNODE* mml_msub_node,
                              SEMANTICS_NODE* snode, int& nodes_done);
  void AnalyzeSubscriptedArgFunc(MNODE* mml_msub_node,
                                 SEMANTICS_NODE* snode);
  void AnalyzeSubscriptedFence(MNODE* mml_msub_node,
                               SEMANTICS_NODE* snode, int& nodes_done);
  void AnalyzeBesselFunc(MNODE* mml_msub_node, SEMANTICS_NODE* snode,
                         int& nodes_done);

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
  IdentIlk GetMIilk(char *mml_content_str, MNODE * m_node, bool isLHSofDef);
  IdentIlk GetMSIilk(char *msi_class);
  bool IdentIsConstant(IdentIlk ilk);

  int CountCols(MNODE * mml_mtr);
  bool IsWhiteSpace(MNODE * mml_node);

  void OperandToBucketList(MNODE* mml_func_node, SemanticType bigop_type,
                           SEMANTICS_NODE* info, int& nodes_done);
  void LocateLimitBuckets(MNODE * mml_node, SEMANTICS_NODE * info);

  BUCKET_REC *ArgsToBucket(MNODE* mml_func_node, int& nodes_done);
  BUCKET_REC *ArgBucketFromMROW(MNODE* mml_next);
  BUCKET_REC *GetParenedArgs(MNODE* mml_mo, int& nodes_done);
  BUCKET_REC *GetFencedArgs(MNODE* mml_fence);

  bool IsInverseIndicator(MNODE * exp);
  char *GetCanonicalIDforMathNode(MNODE * contents);
  void SemanticAttribs2Buffer(char *buffer, MNODE * mml_node, int lim);
  void GetCurrAttribValue(MNODE * mml_node, bool inherit,
                          char *targ_attr, char *buffer, int lim);
  void Contents2Buffer(char *buffer, const char *p_chdata, int lim);

  bool IsWhiteText(const char *the_text);

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
  BaseType GetBaseType(MNODE * base, bool isLHSofDef);
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
  ExpType GetExpType(BaseType base_type, MNODE * exp);
  ExpType GetSubScriptType(MNODE * script_schemata, BaseType base_type, MNODE * exp);

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
  AccentType GetAboveType(BaseType base_type, MNODE * accent);
//  AccentType GetUnderType(BaseType base_type, MNODE * accent);

  SEMANTICS_NODE *GetSemanticsFromNode(MNODE * mml_node, BUCKET_REC * bucket);

  SemanticType GetBigOpType(const char *op_chdata, SemanticVariant & n_integs);
  bool IsWholeNumber(MNODE * mml_mn);
  bool IsWholeFrac(MNODE * mml_frac);
  bool IsOperator(MNODE * mml_node);
  bool IsUnitsFraction(MNODE * mml_node);
  bool IsPositionalChild(MNODE * mml_node);

  bool IsDIFFOP(MNODE * mml_frac_node,
                    MNODE ** m_num_operand, MNODE ** m_den_operand);
  bool IsDDIFFOP(MNODE * mml_msub_node);
  bool IsSUBSTITUTION(MNODE * mml_msub_node);
  bool IsUSunit(const char *ptr);
  bool IsBesselFunc(MNODE * mml_msub_node);

  void AppendNumber(SEMANTICS_NODE * snode, U32 bucket_ID, int num);
  void AddPrimesCount(SEMANTICS_NODE * snode, MNODE * primes);
  BUCKET_REC *AddVarToBucket(U32 bucket_ID, SEMANTICS_NODE * ind_var_list);
  SEMANTICS_NODE *DetermineIndepVar(MNODE * dMML_tree);
  SEMANTICS_NODE *GetIndepVarFromSub(MNODE * msub);
  SEMANTICS_NODE *GetIndVarFromFrac(MNODE * mfrac);
  bool IsVarInSLIST(SEMANTICS_NODE* s_var_list, char* var_nom);
  SEMANTICS_NODE *ExtractIndepVar(MNODE* rover);
  void DetermineODEFuncNames(MNODE* dMML_tree);
  void DeterminePDEFuncNames(MNODE* dMML_tree);
  void DisposeODEFuncNames(DE_FUNC_REC* DE_func_names);
  DE_FUNC_REC *LocateFuncRec(DE_FUNC_REC* f_list, const char* canon_name,
                             const char* src_name);
  DE_FUNC_REC *AppendFuncName(DE_FUNC_REC* f_list, char* canon_name,
                              char* src_name);
  char *GetFuncNameFromFrac(MNODE* mfrac, const char** src_name);
  char *GetFuncNameFromSub(MNODE* msub, const char** src_name);
  char *GetFuncNameFromSup(MNODE* msup, const char** src_name);
  char *GetFuncNameFromSubSup(MNODE* msubsup, const char** src_name);
  void AppendODEfuncs(SEMANTICS_NODE* rv, DE_FUNC_REC* DE_func_names);

  void RemoveBucket(SEMANTICS_NODE * s_base, BUCKET_REC * targ);
  SEMANTICS_NODE *QualifierToSNODE(MNODE * sub);
  
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
  PrefixOpIlk GetPrefixOpCode(const char* op_name, SemanticVariant & n_integs);
  
  bool IsLaplacian(MNODE* op_node);
  void ArgsToMatrix(SEMANTICS_NODE * snode, BUCKET_REC* br);
  bool OpArgIsMatrix(MNODE* mml_mi_node);

  void CreatePrefixForm(SEMANTICS_NODE* s_operator,
                        SEMANTICS_NODE* l_operand,
                        SEMANTICS_NODE* r_operand);
  
  enum OpOrderIlk {
    OOI_none,
    OOI_lessthan,
    OOI_lessorequal,
    OOI_equal,
    OOI_greaterthan,
    OOI_greaterorequal,
    OOI_element,
    OOI_rightarrow
  };
  OpOrderIlk GetOpOrderIlk(SEMANTICS_NODE * relop);
  
  SEMANTICS_NODE *LocateVarAndLimits(BUCKET_REC * l_bucket,
                                     SEMANTICS_NODE ** s_ll,
                                     SEMANTICS_NODE ** s_ul,
                                     bool & ll_is_inclusive,
                                     bool & ul_is_inclusive);
  SEMANTICS_NODE *LocateVarAndExpr(BUCKET_REC * l_bucket,
                                   SEMANTICS_NODE ** s_expr, int & direction);
  void SetVarAndIntervalLimit(BUCKET_REC * ll_bucket);
  void SetVarArrowExprLimit(BUCKET_REC * ll_bucket);
  int GetLimitFormat(char *op_name);

  SEMANTICS_NODE *RemoveInfixOps(SEMANTICS_NODE * s_var);
  void ExtractVariables(SEMANTICS_NODE * s_tree);

  void ConvertToPIECEWISElist(SEMANTICS_NODE * s_fence);
  SEMANTICS_NODE *CreateOnePiece(SEMANTICS_NODE * s_expression,
                                 SEMANTICS_NODE * s_domain);
  bool LocatePieces(BUCKET_REC * cell_list,
                        U32 row_tally, U32 ncols,
                        SEMANTICS_NODE ** s_expression,
                        SEMANTICS_NODE ** s_domain);
  enum OpMatrixIntervalType {
    OMI_none,
    OMI_matrix,
    OMI_interval
  };
  OpMatrixIntervalType GetOpType(MNODE * mo);
  MNODE *LocateOperator(MNODE * mml_list, OpIlk & op_ilk, int & advance);
  void FenceToMatrix(SEMANTICS_NODE * operand);
  void FenceToInterval(SEMANTICS_NODE * s_fence);

  void AddDefaultBaseToLOG(SEMANTICS_NODE * snode);
  bool IsApplyFunction(MNODE * next_elem);

  void ChooseIndVar(MNODE * dMML_tree, char *buffer);
  MNODE *Find_dx(MNODE * mrow, bool & is_nested);
  void Patchdx(SEMANTICS_NODE * s_frac);

  SEMANTICS_NODE *RemoveParens(SEMANTICS_NODE * s_operand);
  bool IsArgDelimitingFence(MNODE * mml_node);

  void CreateSubscriptedVar(MNODE * mml_msub_node,
                            bool remove_super, SEMANTICS_NODE * snode);
  void CreatePowerForm(MNODE * base, MNODE * power, SEMANTICS_NODE * snode);

  MIC2MMLNODE_REC *AppendIDRec(MIC2MMLNODE_REC * node_IDs_list,
                               U32 client_ID, char *obj_name,
                               MNODE * mml_node, const char *zsrc);
  int ChData2Unicodes(const char *p_chdata, U32 * unicodes, int n_unicodes);

  bool SetODEvars(MathServiceRequest & msr, MathResult & mr,
                      MNODE * dMML_tree, U32 UI_cmd_ID);
  bool SetIMPLICITvars(MathServiceRequest & msr, MathResult & mr);

  void SetSnodeOwner(SEMANTICS_NODE * snode);
  SEMANTICS_NODE *CreateSTreeFromMML(const char *mml_str);
  void MSUB2FuncCall(MNODE * mml_msub_node, SEMANTICS_NODE * snode);
  void OverridePrefsOnLHS(MNODE * dMML_tree);
  void OverrideInvisibleTimesOnLHS(MNODE * dMML_tree);
  void CreateSubstBucket(MNODE * subst, SEMANTICS_NODE * snode,
                         bool is_lower);
  int GetVarLimType(char *tmp, MNODE * base);
  SEMANTICS_NODE *DefToSemanticsList(MNODE * dMML_tree, int & error_code);

  Grammar *mml_entities;
  PrefsStore *uprefs_store;
  DefStore *defstore;
  U32 curr_client_ID;
  U32 curr_engine_ID;

  const char *z_scr_str;
  MIC2MMLNODE_REC *node_IDs_list;
  LOG_MSG_REC *msg_list;
  INPUT_NOTATION_REC *p_input_notation;

  MML2Tree *mml_tree_gen;
  Tree2StdMML *CanonicalTreeGen;

  SEMANTICS_NODE *DE_ind_vars;
  DE_FUNC_REC *DE_func_names;

  SEMANTICS_NODE *IMPLDIFF_ind_var;
  DE_FUNC_REC *IMPLDIFF_func_names;

  bool i_is_imaginary;
  bool j_is_imaginary;
  bool e_is_Euler;
  bool log_is_base10;
  bool overbar_conj;
  bool dot_is_derivative;
  bool prime_is_derivative;
  bool D_is_derivative;

  U32 cmd_ID;
};

#endif
