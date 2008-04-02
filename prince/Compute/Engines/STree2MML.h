// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

#ifndef STREE2MML_H
#define STREE2MML_H

#include "CmpTypes.h"
#include "fltutils.h"

// See SIBASEUNITS in .gmr file
typedef int UnitIlk;
#define NUMBASEUNITS 9

typedef struct tagFACTOR_REC
{                               // We build terms from factors
  tagFACTOR_REC *next;
  char *zh_fstr;
  int n_terms;
  int mml_nodes_last_term;
  int unit_power;
  UnitIlk unit_ilk;
  bool is_unit;
  bool is_operator;
  bool is_number;
  bool is_text;
  bool is_constant;
  bool is_signed;
} FACTOR_REC;

class DefStore;
class PrefsStore;
class Grammar;
class MML2Tree;
class MathResult;

class STree2MML
{
public:
  STree2MML(Grammar * mml_grammar, PrefsStore * up_store);
  ~STree2MML();

  char *BackTranslate(SEMANTICS_NODE * root_handle,
                      const char *src,
                      int output_markup_ID,
                      MIC2MMLNODE_REC * curr_IDs_2mml,
                      MIC2MMLNODE_REC * def_IDs_2mml,
                      INPUT_NOTATION_REC * in_notation,
                      DefStore * ds, MathResult * mr);
private:
  void SetUserPrefs(DefStore * ds);
  void SetUserPref(U32 which_one, const char *new_value);

  char *ProcessSemanticList(SEMANTICS_NODE * s_list,
                            int & error_code,
                            int & nodes_made, int & terms_made,
                            bool & is_signed,
                            FACTOR_REC ** num_factors,
                            FACTOR_REC ** den_factors);
  void ProcessSemanticNode(SEMANTICS_NODE * snode,
                           int& error_code,
                           FACTOR_REC ** num_factors,
                           FACTOR_REC ** den_factors);

  void SemanticVARIABLE2MML(SEMANTICS_NODE * snode, FACTOR_REC * factor,
                            int& error_code);
  void SemanticNUMBER2MML(SEMANTICS_NODE * snode, FACTOR_REC * factor,
                          int& error_code);
  void SemanticTEXT2MML(SEMANTICS_NODE * snode, FACTOR_REC * factor,
                        int& error_code);
  void SemanticEQRESULT2MML(SEMANTICS_NODE * snode, FACTOR_REC * factor,
                            int& error_code);
  void SemanticUNIT2MML(SEMANTICS_NODE * snode, FACTOR_REC * factor,
                        int& error_code);
  void SemanticFUNCTION2MML(SEMANTICS_NODE * snode, FACTOR_REC * factor,
                            int& error_code);
  void SemanticLIMFUNC2MML(SEMANTICS_NODE * snode, FACTOR_REC * factor,
                           int& error_code);
  void SemanticOPERATOR2MML(SEMANTICS_NODE * snode, FACTOR_REC * factor,
                            int& error_code);
  void SemanticUCONSTANT2MML(SEMANTICS_NODE * snode, FACTOR_REC * factor,
                             int& error_code);
  void SemanticBOOLEAN2MML(SEMANTICS_NODE * snode, FACTOR_REC * factor,
                           int& error_code);
  void SemanticMATH2MML(SEMANTICS_NODE * snode, FACTOR_REC * factor,
                        int& error_code);
  void SemanticPGROUP2MML(SEMANTICS_NODE * snode, FACTOR_REC * factor,
                          int& error_code);
  void SemanticFENCE2MML(SEMANTICS_NODE * snode, FACTOR_REC * factor,
                         bool add_delims, int& error_code);
  void SemanticSUBS2MML(SEMANTICS_NODE * snode, FACTOR_REC * factor, int& error_code);
  void SemanticFRACTION2MML(SEMANTICS_NODE * snode, int& error_code,
                            FACTOR_REC ** num_factors,
                            FACTOR_REC ** den_factors);
  void SemanticBINOMIAL2MML(SEMANTICS_NODE * snode, FACTOR_REC * factor,
                            int& error_code);
  void SemanticPOWERFORM2MML(SEMANTICS_NODE * snode, FACTOR_REC * factor,
                             int& error_code);
  void SemanticSQROOT2MML(SEMANTICS_NODE * snode, FACTOR_REC * factor,
                          int& error_code);
  void SemanticROOT2MML(SEMANTICS_NODE * snode, FACTOR_REC * factor,
                        int& error_code);
  void SemanticQUANTILE2MML(SEMANTICS_NODE * snode, FACTOR_REC * factor,
                            int& error_code);
  void SemanticBIGOPSUM2MML(SEMANTICS_NODE * snode, FACTOR_REC * factor,
                            int& error_code);
  void SemanticTABULATION2MML(SEMANTICS_NODE * snode, FACTOR_REC * factor,
                              int& error_code);
  void SemanticTRANSPOSE2MML(SEMANTICS_NODE * snode, FACTOR_REC * factor,
                             int& error_code);
  void SemanticSIMPLEX2MML(SEMANTICS_NODE * snode, FACTOR_REC * factor,
                           int& error_code);
  void SemanticFENCEOP2MML(SEMANTICS_NODE * snode, FACTOR_REC * factor,
                           int& error_code);
  void SemanticNORM2MML(SEMANTICS_NODE * snode, FACTOR_REC * factor,
                           int& error_code);
  void SemanticCONJUGATE2MML(SEMANTICS_NODE * snode, FACTOR_REC * factor,
                             int& error_code);
  void SemanticEIGENVECTOR2MML(SEMANTICS_NODE * snode, FACTOR_REC * factor,
                               int& error_code);
  void SemanticINTEGRAL2MML(SEMANTICS_NODE * snode, FACTOR_REC * factor,
                            int& error_code);
  void SemanticPIECEWISE2MML(SEMANTICS_NODE * snode, FACTOR_REC * factor,
                             int& error_code);
  void SemanticDERIVATIVE2MML(SEMANTICS_NODE * snode, FACTOR_REC * factor,
                              int& error_code);
  void SemanticINTERVAL2MML(SEMANTICS_NODE * snode, FACTOR_REC * factor,
                            int& error_code);
  void SemanticMIXEDNUM2MML(SEMANTICS_NODE * snode, FACTOR_REC * factor,
                            int& error_code);
  void SemanticFracDerivative2MML(SEMANTICS_NODE * snode, FACTOR_REC * factor,
                                  bool use_dd, int& error_code);

  void LogWithBase2MML(SEMANTICS_NODE * s_function,
                       FACTOR_REC * factor, int& error_code);
  void SmallMixedNumber2MML(U32 num, U32 denom, FACTOR_REC * factor);
  void BesselFunc2MML(SEMANTICS_NODE * s_function,
                      FACTOR_REC * factor, int& error_code);
  bool IsBesselFunc(char *f_nom);

  char *GetTmplPtr(int tmpl_ID);

  char *NestInMath(char *body);
  char *NestzMMLInMROW(char *z_body);
  char *NestzMMLInPARENS(char *zh_arg, int & nodes_made);

  bool ScriptIsOneOverN(BUCKET_REC * script_upper_bucket,
                            bool & is_sqrt);
  bool IsExposedNumber(SEMANTICS_NODE * s_node);
  bool IsExposedPositiveNumber(SEMANTICS_NODE * s_node);
  bool IsSmallNaturalNumber(SEMANTICS_NODE * s_node, U32 & num);
  bool IsUnit(SEMANTICS_NODE * s_node);

  FACTOR_REC *CreateFactor();
  void GetFactors(BUCKET_REC * parent_bucket,
                  FACTOR_REC ** num_factors, FACTOR_REC ** den_factors);
  char *FuncHeader2MML(SEMANTICS_NODE * s_function);
  char *ComposeFuncCall(SEMANTICS_NODE * s_function,
                        char *zh_header,
                        bool is_trigarg_func,
                        bool & trigarg_scripted,
                        bool subscripted_arg,
                        int nprimes,
                        int deriv_style, SEMANTICS_NODE * s_derivative);

  void DisposeFactorList(FACTOR_REC * f_list);
  void SplitFactors(FACTOR_REC * num_factors,
                    FACTOR_REC ** top_factors, FACTOR_REC ** top_units);
  char *FactorsTozStr(FACTOR_REC * f_list, int & tally, int & terms,
                      bool & is_signed, bool is_units);
  char *ComposeTerm(FACTOR_REC * num_factors,
                    FACTOR_REC * den_factors,
                    bool & is_signed, int & nodes_made, int & terms_made);
  bool SNodeEndsTerm(SEMANTICS_NODE * s_node,
                         int & curr_prec, bool & is_plusORminus);
  FACTOR_REC *JoinFactors(FACTOR_REC * f_list, FACTOR_REC * more_factors);
  FACTOR_REC *SetFactorOrder(FACTOR_REC * factors);
  FACTOR_REC *AppendFactor(FACTOR_REC * f_list, FACTOR_REC * f_next);
  FACTOR_REC *AbsorbUnaryOps(FACTOR_REC * loc_num_factors,
                             bool & separate_sign);

  void GetBUnitPowers(FACTOR_REC * bot_units, int *baseunit_powers,
                      bool is_num);
  void SetUnitInfo(SEMANTICS_NODE * snode, FACTOR_REC * new_fr);

  char *BUnitsToCompoundUnit(int *baseunit_powers);
  int OnesAllowed(FACTOR_REC * f_list);
  SEMANTICS_NODE *RemoveParens(SEMANTICS_NODE * s_factor,
                               BUCKET_REC * parent_bucket);

  char *AddSubToVariable(SEMANTICS_NODE * s_sub, char *zh_var);

  MNODE *CleanupMMLsource(MNODE * mml_var_tree);

  char *ColorizeMMLElement(const char *tmpl, const char *color_attr1,
                           const char *color_attr2, const char *color_attr3);
  char *DiffDenom2MML(SEMANTICS_NODE * snode,
                      bool use_dd, int& error_code);

  ATTRIB_REC *VariantToStyleAtts(const char *mathvariant, char *suffix);

  enum DelimsIlk {
    Delims_none,
    Delims_brackets = 1,
    Delims_parens = 2,
    Delims_braces = 3
  };
  DelimsIlk GetNotationVariant(SemanticType sem_type);
  
  char *NumberToUserFormat(SEMANTICS_NODE * s_number);
  void Roundup(char *buffer);

  char *MIfromCHDATA(const char *z_chdata);
  char *MNfromNUMBER(char *z_number);
  char *MOfromSTRING(const char *z_string, const char *color);
  char *PrefixMOfromSTRING(char *z_string);
  char *AccentMOfromSTRING(char *z_string);

  char *GetPrimesStr(int nprimes);
  char *GetDotsStr(int nprimes);
  char *GetVarsStr(SEMANTICS_NODE * s_diff);

  bool DenomIsNumber(FACTOR_REC * bot_factors);
  FACTOR_REC *ExtractNumNumber(FACTOR_REC * top_factors, char **mml_num_str);
  int GetInfixPrecedence(SEMANTICS_NODE * s_node);
  bool IsTrigArgFunc(SEMANTICS_NODE * s_function);
  bool ParensRequired(SEMANTICS_NODE * s_function);
  void UPrefStr2MMLattr(char **dest, const char *src);

  char *LowerLim2MML(BUCKET_REC * b_parts);
  int GetLimDirection(BUCKET_REC * b_direction);

  bool IsSimpleDenom(char *mml_markup);

  Grammar *mml_entities;
  PrefsStore *uprefs_store;
  const char *src_markup;
  MIC2MMLNODE_REC *canonicalID_mapper;
  MIC2MMLNODE_REC *def_IDs_mapper;

  MML2Tree *mml_tree_gen;

  int output_markup;            // MML or LaTeX

  INPUT_NOTATION_REC *input_notation;
  MathResult *p_mr;

  // notational history
  DelimsIlk last_matrix_delim_type;

  // client prefs
  int up_mml_version;
  char up_mml_prefix[32];
  char *up_math_attrs;
  bool up_use_mfenced;
  char *up_clr_math_attr;
  char *up_clr_func_attr;
  char *up_clr_text_attr;
  char *up_unit_attrs;
  int up_signif_digits;
  int up_sn_lowerthreshold;
  int up_sn_upperthreshold;
  int up_primes_as_n_threshold;
  bool up_parens_on_trigargs;
  bool up_log_is_base_e;
  char up_differentialD[16];
  char up_differentiald[16];
  char up_nlogbase[16];
  char up_imaginaryi[16];
  bool up_InvTrigFuncs_1;
  bool up_output_mixed_nums;
  bool up_output_parens_mode;
  DelimsIlk up_matrix_delims;
  int up_derivative_format;

  bool up_dot_is_derivative;
  bool up_overbar_is_conjugate;
};

#endif
