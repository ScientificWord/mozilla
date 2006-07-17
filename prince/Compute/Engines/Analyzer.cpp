// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

/* Implementation Notes - JBM

   This object examines MathML constructs ( <mi>,<mfrac>,<mover>, etc.) in the context
   of a <math> tree and assigns a "meaning" to each.  ( ie. classifies them as variables,
   operators, numbers, functions, power forms, fractions, binomials, etc. )

   In Prince, we want to give users some input into this semantic analysis process.
   For example, the user might want <mover><mi>x</mi><mo>&dot;</mo></mover> 
   to be interpreted as differentiation or as a decorated variable.

   To implement different interpretations, each client will need a "semantics" store.
   Before a meaning is assigned to a node using the default algorithm, we'll need
   to consult the "semantics" store for a possible override.  If we allow overrides
   on a per equation basis or a context-sensitive basis, we'll need a multi-level store.

   Every instance of CompEngine owns one "Analyzer".  Analyzer builds a "semantic" tree
   from a MathML tree, and passes it back to its parent CompEngine.

*/

#ifdef TESTING
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "Analyzer.h"
#include "Grammar.h"
#include "MML2Tree.h"
#include "Tree2StdMML.h"
#include "../DefStore.h"
#include "../PrefStor.h"

#include <string.h>
#include <ctype.h>

#include "tci_new.h"


Analyzer::Analyzer(Grammar * mml_grammar, PrefsStore * up_store)
{
  z_scr_str = NULL;
  mml_entities = mml_grammar;
  uprefs_store = up_store;

  defstore = NULL;
  curr_client_ID = 0;
  curr_engine_ID = 0;

  msg_list = NULL;
  node_IDs_list = NULL;
  CanonicalTreeGen = TCI_NEW(Tree2StdMML(mml_grammar,this));
  DE_ind_vars = NULL;
  DE_func_names = NULL;
  IMPLDIFF_ind_var = NULL;
  IMPLDIFF_func_names = NULL;

  mml_tree_gen = TCI_NEW(MML2Tree);

  i_is_imaginary = true;
  j_is_imaginary = false;
  e_is_Euler = true;
}

Analyzer::~Analyzer()
{
  TCI_ASSERT(!msg_list);
  DisposeMsgs(msg_list);
  TCI_ASSERT(!node_IDs_list);
  DisposeIDsList(node_IDs_list);
  delete mml_tree_gen;
  delete CanonicalTreeGen;
  TCI_ASSERT(DE_ind_vars == NULL);
  TCI_ASSERT(DE_func_names == NULL);
  TCI_ASSERT(IMPLDIFF_ind_var == NULL);
  TCI_ASSERT(IMPLDIFF_func_names == NULL);
}

void Analyzer::SetInputPrefs(DefStore * ds, U32 engine_ID)
{
  const char *pref_val = ds->GetPref(CLPF_log_is_base_e, 0);
  if (!pref_val)
    pref_val = uprefs_store->GetPref(CLPF_log_is_base_e);
  if (pref_val) {
    // log is base e<uID14.0>0
    int db_val = atoi(pref_val);
    log_is_base10 = db_val ? false : true;
  } else {
    TCI_ASSERT(0);
    log_is_base10 = false;
  }

  pref_val = ds->GetPref(CLPF_Dot_derivative, 0);
  if (!pref_val)
    pref_val = uprefs_store->GetPref(CLPF_Dot_derivative);
  if (pref_val) {
    // Dot accent is derivative<uID19.0>1
    int db_val = atoi(pref_val);
    dot_is_derivative = db_val ? true : false;
  } else {
    TCI_ASSERT(0);
    dot_is_derivative = false;
  }

  pref_val = ds->GetPref(CLPF_Prime_derivative, 0);
  if (!pref_val)
    pref_val = uprefs_store->GetPref(CLPF_Prime_derivative);
  if (pref_val) {
    int db_val = atoi(pref_val);
    prime_is_derivative = db_val ? true : false;
  } else {
    TCI_ASSERT(0);
    prime_is_derivative = false;
  }

  pref_val = ds->GetPref(CLPF_Overbar_conjugate, 0);
  if (!pref_val)
    pref_val = uprefs_store->GetPref(CLPF_Overbar_conjugate);
  if (pref_val) {
    // Overbar is conjugate<uID20.0>1
    int db_val = atoi(pref_val);
    overbar_conj = db_val ? true : false;
  } else {
    TCI_ASSERT(0);
    overbar_conj = false;
  }

  DefInfo *di = ds->GetDefInfo(engine_ID, "mii");
  if (di) {
    i_is_imaginary = false;
  } else {
    pref_val = ds->GetPref(CLPF_Input_i_Imaginary, 0);
    if (!pref_val)
      pref_val = uprefs_store->GetPref(CLPF_Input_i_Imaginary);
    if (pref_val) {
      int db_val = atoi(pref_val);
      i_is_imaginary = db_val ? true : false;
    } else {
      TCI_ASSERT(0);
      i_is_imaginary = true;
    }
  }

  di = ds->GetDefInfo(engine_ID, "mij");
  if (di) {
    j_is_imaginary = false;
  } else {
    pref_val = ds->GetPref(CLPF_Input_j_Imaginary, 0);
    if (!pref_val)
      pref_val = uprefs_store->GetPref(CLPF_Input_j_Imaginary);
    if (pref_val) {
      int db_val = atoi(pref_val);
      j_is_imaginary = db_val ? true : false;
    } else {
      TCI_ASSERT(0);
      j_is_imaginary = false;
    }
  }

  di = ds->GetDefInfo(engine_ID, "mie");
  if (di) {
    e_is_Euler = false;
  } else {
    pref_val = ds->GetPref(CLPF_Input_e_Euler, 0);
    if (!pref_val)
      pref_val = uprefs_store->GetPref(CLPF_Input_e_Euler);
    if (pref_val) {
      int db_val = atoi(pref_val);
      e_is_Euler = db_val ? true : false;
    } else {
      TCI_ASSERT(0);
      e_is_Euler = false;
    }
  }

}

// NOTE: The following function modifies it's dMML_tree parameter.
// It converts the tree to a canonical form before starting the
// semantic analysis.  This step is required so that we can avoid
// having to deal with multiple markups for the same object.
// For example, a construct bracketed by <mo>(</mo>..<mo>)</mo>
// might represent the same math object that an <mfenced> represents.

SEMANTICS_NODE *Analyzer::BuildSemanticsTree(MathServiceRequest & msr,
                                             MathResult & mr,
                                             const char *src_markup,
                                             MNODE * dMML_tree,
                                             U32 curr_cmd_ID,
                                             INPUT_NOTATION_REC * in_notation)
{ 
  SEMANTICS_NODE *rv = NULL;

  cmd_ID = curr_cmd_ID;
  z_scr_str = src_markup;
  p_input_notation = in_notation;

  switch (curr_cmd_ID) {
    case CCID_Cleanup:
    case CCID_Fixup:
      dMML_tree = CanonicalTreeGen->TreeToCleanupForm(dMML_tree, in_notation);
      break;
    case CCID_Interpret:
      dMML_tree = CanonicalTreeGen->TreeToInterpretForm(dMML_tree, in_notation);
      break;
    default:
      dMML_tree = CanonicalTreeGen->TreeToCanonicalForm(dMML_tree, in_notation);
      break;
  }

  defstore = msr.GetDefStore();
  curr_client_ID = msr.GetClientHandle();
  curr_engine_ID = msr.GetEngineID();

  TCI_ASSERT(!IMPLDIFF_ind_var);
  if (curr_cmd_ID == CCID_Calculus_Implicit_Differentiation) {
    if (!SetIMPLICITvars(msr, mr)) {
      TCI_ASSERT(0);
      return rv;
    }
  }

  TCI_ASSERT(!DE_ind_vars);
  if (curr_cmd_ID >= CCID_Solve_ODE_Exact && curr_cmd_ID <= CCID_Solve_ODE_Series) {  // Solve ODE
    // ODEs are indeed special - we have to decide both the function
    //  that we're seeking to define, and the independent variable.
    if (!SetODEvars(msr, mr, dMML_tree, curr_cmd_ID))
      return rv;
  }

  if (curr_cmd_ID == CCID_Solve_PDE) {
    SEMANTICS_NODE *s_indvar = DetermineIndepVar(dMML_tree);
    TCI_ASSERT(s_indvar);
    DeterminePDEFuncNames(dMML_tree);
  }

  JBM::DumpTList(dMML_tree, 0);
  TCI_ASSERT(!msg_list);
  DisposeMsgs(msg_list);
  msg_list = NULL;
  // We sometimes get here with ODEs.  Some analysis that generates
  //  name records may be done to determine the independent variable.
  DisposeIDsList(node_IDs_list);
  node_IDs_list = NULL;

  // "i", "j" and "e" must be treated as formal args (ie. variables)
  //   if they occur on the left hand side of a definition
  if (curr_cmd_ID == CCID_Define) {
    int error_code;
    rv = DefToSemanticsList(dMML_tree, error_code);
    if (error_code || !rv) {
      mr.PutResultCode(CR_baddefformat);
    }
  } else {
    rv = GetSemanticsList(dMML_tree, NULL);
  }

  if (DE_func_names) {
    AppendODEfuncs(rv, DE_func_names);
    DisposeODEFuncNames(DE_func_names);
    DE_func_names = NULL;
  }
  DisposeSList(DE_ind_vars);
  DE_ind_vars = NULL;

  if (IMPLDIFF_func_names) {
    DisposeODEFuncNames(IMPLDIFF_func_names);
    IMPLDIFF_func_names = NULL;
  }
  DisposeSList(IMPLDIFF_ind_var);
  IMPLDIFF_ind_var = NULL;

  return rv;
}

void Analyzer::TreeToCleanupForm(MNODE * dMML_tree,
                                  INPUT_NOTATION_REC * in_notation)
{
  CanonicalTreeGen->TreeToCleanupForm(dMML_tree, in_notation);
  JBM::DumpTList(dMML_tree, 0);
}

void Analyzer::TreeToFixupForm(MNODE * dMML_tree,
                               INPUT_NOTATION_REC * in_notation)
{
  CanonicalTreeGen->TreeToFixupForm(dMML_tree, in_notation);
  JBM::DumpTList(dMML_tree, 0);
}

// Convenience method for Tree2StdMML to lookup a function.
bool Analyzer::IsDefinedFunction(MNODE * mnode)
{
  char *mi_canonical_str = GetCanonicalIDforMathNode(mnode);
  if (defstore && mi_canonical_str) {
    DefInfo *di = defstore->GetDefInfo(curr_engine_ID, mi_canonical_str);
    delete[] mi_canonical_str;
    if (di && di->def_type == DT_FUNCTION)
      return true;
  }
  return false;
}

// <mi>x</mi>
// <mi>sin</mi>        <START_CONTEXT_FUNCTIONS>
// <mi>&theta;</mi>    <START_CONTEXT_VARIABLES>
// This is a preliminary implementation.  Ultimately
//  all the logical required to decide the meaning
//  an <mi> lies within this function.

void Analyzer::AnalyzeMI(MNODE * mml_mi_node,
                         SEMANTICS_NODE * snode,
                         int& nodes_done, bool isLHSofDef)
{
  nodes_done = 1;

  char zclass[256];
  zclass[0] = 0;
  GetCurrAttribValue(mml_mi_node, false, "class", zclass, 256);

  char zmsi_class[256];
  zmsi_class[0] = 0;
  GetCurrAttribValue(mml_mi_node, false, "msiclass", zmsi_class, 256);

  if (!strcmp(zclass, "msi_unit")) {
    SEMANTICS_NODE *s_target = snode;
    // Semantically, I'm treating units as "factors" joined to the expression
    //  that they qualify by an invisible times. If &it; is NOT present
    //  in the source MML, it's semantic equivalent is generated here.
    if (mml_mi_node->prev && !IsOperator(mml_mi_node->prev)
        && !IsPositionalChild(mml_mi_node)) {
      TCI_ASSERT(0);
      snode->semantic_type = SEM_TYP_INFIX_OP;
      char *tmp = TCI_NEW(char[10]);
      strcpy(tmp, "&#x2062;");
      snode->contents = tmp;

      s_target = CreateSemanticsNode();
      snode->next = s_target;
      s_target->prev = snode;
    }

    const char *ptr = mml_mi_node->p_chdata;
    size_t zln = strlen(ptr);
    char *tmp = TCI_NEW(char[zln + 1]);
    strcpy(tmp, ptr);
    s_target->contents = tmp;

    if (IsUSunit(ptr))
      s_target->semantic_type = SEM_TYP_USUNIT;
    else
      s_target->semantic_type = SEM_TYP_SIUNIT;

    return;
  }

  /* NON-UNIT CLAUSE

    We need to give the math object (mi) a standard engine-independent
    name here and record it in the SEMANTICS_NODE passed to the wrapper engine.

  ( This canonical name is seen by the engine wrapper but not by the
    underlying engine. The wrapper creates its own engine-specific name
    for the object to use in communications with the engine. )

    When processing returns froms the engine, the engine wrapper back-maps
    the engine-specific name to the canonical name before passing data
    back to this object.
  */

  char *mml_canonical_name = GetCanonicalIDforMathNode(mml_mi_node);

  if (!mml_canonical_name) {
    snode->error_flag = 1;
    TCI_ASSERT(0);
    return;
  }
  // Put the canonical name of the math object in "snode".
  snode->canonical_ID = mml_canonical_name;
  SetSnodeOwner(snode);

  // Store a name-to-node back mapping record for this object.
  node_IDs_list = AppendIDRec(node_IDs_list, curr_client_ID,
                              mml_canonical_name, mml_mi_node, z_scr_str);
  // This <mi> may be "defined" in the client's current context,
  //  or it may be a special predefined identifier.
  IdentIlk mi_ilk;
  if (zmsi_class[0]) {
    mi_ilk = GetMSIilk(zmsi_class);
    snode->msi_class = mi_ilk;
  } else {
    mi_ilk = GetMIilk(mml_canonical_name, mml_mi_node, isLHSofDef);
  }
  if (mi_ilk) {
    if (IdentIsConstant(mi_ilk)) {
      const char *cont = mml_mi_node->p_chdata;
      if (mi_ilk == MI_imaginaryunit)
        cont = "i";
      size_t ln = strlen(cont);
      char *tmp = TCI_NEW(char[ln + 1]);
      strcpy(tmp, cont);
      snode->contents = tmp;
      snode->semantic_type = SEM_TYP_UCONSTANT;
    } else if (mi_ilk == MI_Laplace || mi_ilk == MI_Fourier || mi_ilk == MI_function) {
      size_t ln = strlen(mml_mi_node->p_chdata);
      char *tmp = TCI_NEW(char[ln + 1]);
      strcpy(tmp, mml_mi_node->p_chdata);
      snode->contents = tmp;
      if (mi_ilk == MI_Laplace || mi_ilk == MI_Fourier)
        snode->semantic_type = SEM_TYP_TRANSFORM;
      else
        snode->semantic_type = SEM_TYP_FUNCTION;

      int local_nodes_done;
      BUCKET_REC *bucket = ArgsToBucket(mml_mi_node, local_nodes_done);
      nodes_done += local_nodes_done;

      bool arg_is_matrix = OpArgIsMatrix(mml_mi_node);
      if (arg_is_matrix)        // the args become entries in a row matrix
        ArgsToMatrix(snode, bucket);
      else
        snode->bucket_list = AppendBucketRec(snode->bucket_list, bucket);
    } else if (mi_ilk == MI_variable) {
      snode->semantic_type = SEM_TYP_VARIABLE;
    } else
      TCI_ASSERT(0);
  } else {                      // default semantics for <mi>
    int entity_count;
    int symbol_count = CountSymbols(mml_mi_node->p_chdata, entity_count);
    if (symbol_count) {
      char varnom[256];
      varnom[0] = 0;
      GetCurrAttribValue(mml_mi_node, false, "mathvariant", varnom, 256);
      strcat(varnom, mml_mi_node->p_chdata);

      size_t ln = strlen(varnom);
      char *tmp = TCI_NEW(char[ln + 1]);
      strcpy(tmp, varnom);
      snode->contents = tmp;
    } else {
      TCI_ASSERT(0);
    }
    bool is_function = false;
    bool is_ODE_func = false;
    bool is_IMPLDIFF_func = false;

    if (LocateFuncRec(DE_func_names, mml_canonical_name, NULL)) {
      is_ODE_func = true;
    } else
      if (LocateFuncRec(IMPLDIFF_func_names, NULL, mml_mi_node->p_chdata)) {
      is_IMPLDIFF_func = true;
    } else if (symbol_count == 1) {
      // an entity, maybe a Greek letter
      // ASCII letter 
      if (mml_mi_node->next) {
        // Look for &ApplyFunction; after this <mi>
        MNODE *next_elem = mml_mi_node->next;

        if (!mml_mi_node->prev && mml_mi_node->parent) {
          const char *p_elem = mml_mi_node->parent->src_tok;
          if (!strcmp(p_elem, "msub")
              || !strcmp(p_elem, "msup")
              || !strcmp(p_elem, "msubsup")
              || !strcmp(p_elem, "munder")
              || !strcmp(p_elem, "mover")
              || !strcmp(p_elem, "munderover"))
            next_elem = mml_mi_node->parent->next;
        }
        if (next_elem)
          if (IsApplyFunction(next_elem))
            is_function = true;
      }
    } else if (symbol_count > 1) {
      if (entity_count > 0)     // multiple entities?
        TCI_ASSERT(0);
      is_function = true;
    }

    if (is_function || is_ODE_func || is_IMPLDIFF_func) {
      snode->semantic_type = SEM_TYP_FUNCTION;

      if (mml_mi_node->p_chdata && !strcmp(mml_mi_node->p_chdata, "log")) {
        p_input_notation->n_logs++;
        AddDefaultBaseToLOG(snode);
      }
      int local_nodes_done;
      BUCKET_REC *bucket = ArgsToBucket(mml_mi_node, local_nodes_done);
      nodes_done += local_nodes_done;
      if (is_ODE_func && (!bucket || !bucket->first_child)) {
        if (DE_ind_vars) {
          if (bucket)
            DisposeBucketList(bucket);
          bucket = AddVarToBucket(MB_UNNAMED, DE_ind_vars);
        } else {
          TCI_ASSERT(0);
          if (!bucket)
            bucket = MakeBucketRec(MB_UNNAMED, NULL);
        }
      }

      if (is_IMPLDIFF_func && (!bucket || !bucket->first_child)) {
        if (IMPLDIFF_ind_var) {
          if (bucket)
            DisposeBucketList(bucket);
          bucket = AddVarToBucket(MB_UNNAMED, IMPLDIFF_ind_var);
        } else {
          TCI_ASSERT(0);
          if (!bucket)
            bucket = MakeBucketRec(MB_UNNAMED, NULL);
        }
      }

      bool arg_is_matrix = OpArgIsMatrix(mml_mi_node);
      if (arg_is_matrix)        // the args become entries in a row matrix
        ArgsToMatrix(snode, bucket);
      else
        snode->bucket_list = AppendBucketRec(snode->bucket_list, bucket);
    } else {
      snode->semantic_type = SEM_TYP_VARIABLE;
    }
  }
}

void Analyzer::AnalyzeMTEXT(MNODE * mml_mtext_node,
                            SEMANTICS_NODE * snode, int& nodes_done)
{
  nodes_done = 1;               // probably always doing 1 mml node here!

  if (mml_mtext_node->p_chdata) {
    if (!IsWhiteText(mml_mtext_node->p_chdata)) {

      size_t zln = strlen(mml_mtext_node->p_chdata);
      char *tmp = TCI_NEW(char[zln + 1]);
      strcpy(tmp, mml_mtext_node->p_chdata);
      snode->contents = tmp;

      if (cmd_ID == CCID_PassThru) {
        snode->semantic_type = SEM_TYP_ENG_PASSTHRU;
      } else {
        char *mml_canonical_name = GetCanonicalIDforMathNode(mml_mtext_node);
        snode->canonical_ID = mml_canonical_name;
        SetSnodeOwner(snode);
        // Store a name-to-node back mapping record for this object.
        node_IDs_list = AppendIDRec(node_IDs_list, curr_client_ID,
                                    mml_canonical_name, mml_mtext_node,
                                    z_scr_str);
        snode->semantic_type = SEM_TYP_TEXT;
      }
    }
  } else
    TCI_ASSERT(0);
}

void Analyzer::AnalyzeMO(MNODE * mml_mo_node,
                         SEMANTICS_NODE * snode, int& nodes_done)
{
  nodes_done = 1;               // note that an operand may be added

  snode->semantic_type = SEM_TYP_INFIX_OP;  // assumed

  if (mml_mo_node->p_chdata) {
    SemanticVariant n_integs;
    const char *f_nom = mml_mo_node->p_chdata;
    PrefixOpIlk op_ilk = GetPrefixOpCode(f_nom, n_integs);

    // Some <mo>s require special handling
    // These include all prefix operators, and operators that really
    //   should have been scripted as functions, ie <mi>s, in the source MML
    if (op_ilk != POI_none) {
      bool arg_is_matrix = false;
      switch (op_ilk) {
      case POI_listop:
        snode->semantic_type = SEM_TYP_LISTOPERATOR;
        break;
      case POI_det:
        snode->semantic_type = SEM_TYP_PREFIX_OP;
        f_nom = "det";
        break;
      case POI_distribution:
        snode->semantic_type = SEM_TYP_FUNCTION;
        break;
      case POI_Dirac:
        snode->semantic_type = SEM_TYP_FUNCTION;
        break;
      case POI_gradient:
        snode->semantic_type = SEM_TYP_PREFIX_OP;
        f_nom = "grad";
        break;
      case POI_divergence:
        snode->semantic_type = SEM_TYP_PREFIX_OP;
        f_nom = "div";
        arg_is_matrix = true;
        break;
      case POI_curl:
        snode->semantic_type = SEM_TYP_PREFIX_OP;
        f_nom = "curl";
        arg_is_matrix = true;
        break;
      case POI_Laplacian:
        snode->semantic_type = SEM_TYP_PREFIX_OP;
        f_nom = "Laplacian";
        break;
      case POI_integral:
        snode->semantic_type = SEM_TYP_BIGOP_INTEGRAL;
        snode->variant = n_integs;
        break;
      case POI_sum:
        snode->semantic_type = SEM_TYP_BIGOP_SUM;
        break;
      default:
        TCI_ASSERT(!"Unknown Prefix Op Ilk.");
      }

      if (op_ilk == POI_integral || op_ilk == POI_sum) {
        int nodes_in_arg;
        OperandToBucketList(mml_mo_node, snode->semantic_type,
                            snode, nodes_in_arg);
        nodes_done += nodes_in_arg;
      } else {
        // the right operand is nested, like a function argument
        int nodes_in_arg;
        BUCKET_REC *br = ArgsToBucket(mml_mo_node, nodes_in_arg);
        nodes_done += nodes_in_arg;
        if (br) {
          if (arg_is_matrix)    // the args become entries in a row matrix
            ArgsToMatrix(snode, br);
          else
            snode->bucket_list = AppendBucketRec(snode->bucket_list, br);
        }
      }
    } else {
      const char *attr_val = GetATTRIBvalue(mml_mo_node->attrib_list, "form");
      if (attr_val) {
        if (!strcmp(attr_val, "prefix")) {
          snode->semantic_type = SEM_TYP_PREFIX_OP;
        } else if (!strcmp(attr_val, "infix")) {
          snode->semantic_type = SEM_TYP_INFIX_OP;
        } else if (!strcmp(attr_val, "postfix")) {
          snode->semantic_type = SEM_TYP_POSTFIX_OP;
        } else {
          TCI_ASSERT(0);
        }
      }
    }

    // It's not clear that operators should ever be aliased.
    // Install our own standard semantic name for the operator.

    if (!strcmp(f_nom, "&#x5c;")) // bogus, backslash to setminus
      f_nom = "&#x2216;";

    delete[] snode->contents;
    size_t zln = strlen(f_nom);
    char *tmp = TCI_NEW(char[zln + 1]);
    strcpy(tmp, f_nom);
    snode->contents = tmp;
  } else {
    TCI_ASSERT(0);
  }
}

void Analyzer::AnalyzeMN(MNODE * mml_mn_node, SEMANTICS_NODE * snode)
{
  if (mml_mn_node && mml_mn_node->p_chdata) {
    size_t zln = strlen(mml_mn_node->p_chdata);
    char *num_str = TCI_NEW(char[zln + 1]);

    int n_digits = 0;
    int di = 0;
    const char *ptr = mml_mn_node->p_chdata;
    while (*ptr) {
      char ch = *ptr;
      if (ch >= '0' && ch <= '9') {
        num_str[di++] = ch;
        n_digits++;
      } else if (ch == '.') {
        num_str[di++] = ch;
        n_digits++;

      } else if (ch == '-') {
        if (di == 0) {
          num_str[di++] = ch;
          n_digits++;
        } else
          TCI_ASSERT(0);
      } else if (ch == '+') {
        if (di == 0)
          n_digits++;
        else
          TCI_ASSERT(0);
      } else if (ch == '&') {   // crude attempt to remove spacing entities
        while (*ptr && *ptr != ';') {
          ptr++;
          n_digits++;
        }
        if (*ptr == ';')
          n_digits++;
      } else {
        TCI_ASSERT(0);
        break;
      }
      ptr++;
    }
    num_str[di] = 0;

    if (n_digits != zln) {
      TCI_ASSERT(0);
      strcpy(num_str, mml_mn_node->p_chdata);

      // For <mn>s that carry base 10 digits only, we don't need mapping info.
      // Other <mn>'s will probably pass thru computation as a symbolic
      //  alias, and will require mapping info.
      char *mml_canonical_name = GetCanonicalIDforMathNode(mml_mn_node);
      snode->canonical_ID = mml_canonical_name;
      SetSnodeOwner(snode);
      // Store a name-to-node back mapping record for this object.
      node_IDs_list = AppendIDRec(node_IDs_list, curr_client_ID,
                                  mml_canonical_name, mml_mn_node, z_scr_str);
    }
    snode->contents = num_str;
  } else
    TCI_ASSERT(0);

  snode->semantic_type = SEM_TYP_NUMBER;
}

void Analyzer::AnalyzeMFRAC(MNODE * mml_mfrac, SEMANTICS_NODE * snode,
                            int& nodes_done)
{
  nodes_done = 1;

  // Look for "d^4(?)/d^{3}xdy"
  MNODE *m_num_operand;
  MNODE *m_den;
  if (IsDIFFOP(mml_mfrac, &m_num_operand, &m_den)) {
    snode->semantic_type = SEM_TYP_DERIVATIVE;
    char *tmp = TCI_NEW(char[14]);
    strcpy(tmp, "differentiate");
    snode->contents = tmp;

    // Handle the expression being differentiated
    if (m_num_operand) {
      // Here the expression being differentiated is in the numerator
      int nodes_in_arg;
      BUCKET_REC *br = ArgsToBucket(m_num_operand->prev, nodes_in_arg);
      if (br)
        snode->bucket_list = AppendBucketRec(snode->bucket_list, br);
    } else {
      // Here the expression being differentiated follows the fraction
      int nodes_in_arg;
      BUCKET_REC *br = ArgsToBucket(mml_mfrac, nodes_in_arg);
      nodes_done += nodes_in_arg;
      if (br)
        snode->bucket_list = AppendBucketRec(snode->bucket_list, br);
    }
    if (m_den) {                // denominator - "d^{3}xdy"
      SEMANTICS_NODE *s_indvar = GetIndVarFromFrac(mml_mfrac);
      if (s_indvar) {
        BUCKET_REC *fvar_bucket = MakeBucketRec(MB_DIFF_VAR, s_indvar);
        s_indvar->parent = fvar_bucket;
        snode->bucket_list = AppendBucketRec(snode->bucket_list, fvar_bucket);
      } else {
        TCI_ASSERT(0);
      }
    } else {
      TCI_ASSERT(0);
    }
    p_input_notation->n_doverds++;
    return;
  }

  SEMANTICS_NODE *s_target = snode;
  if (IsUnitsFraction(mml_mfrac)) {
    // Semantically, I'm treating units as "factors" joined to the expression
    //  that they qualify by an invisible times. If &it; is NOT present
    //  in the source MML, it's semantic equivalent is generated here.
    if (mml_mfrac->prev && !IsOperator(mml_mfrac->prev)) {
      snode->semantic_type = SEM_TYP_INFIX_OP;
      char *tmp = TCI_NEW(char[10]);
      strcpy(tmp, "&#x2062;");
      snode->contents = tmp;

      s_target = CreateSemanticsNode();
      snode->next = s_target;
      s_target->prev = snode;
    }
  }

  BUCKET_REC *parts_list = NULL;
  MNODE *rover = mml_mfrac->first_kid;
  if (rover) {
    BUCKET_REC *num_bucket = MakeBucketRec(MB_NUMERATOR, NULL);
    parts_list = AppendBucketRec(parts_list, num_bucket);
    SEMANTICS_NODE *numer = GetSemanticsFromNode(rover, num_bucket);
    num_bucket->first_child = numer;
    numer->parent = num_bucket;

    rover = rover->next;
    if (rover) {
      BUCKET_REC *denom_bucket = MakeBucketRec(MB_DENOMINATOR, NULL);
      parts_list = AppendBucketRec(parts_list, denom_bucket);
      SEMANTICS_NODE *denom = GetSemanticsFromNode(rover, denom_bucket);
      denom_bucket->first_child = denom;
      denom->parent = denom_bucket;
    }
  }
  s_target->bucket_list = parts_list;

  char buffer[256];
  buffer[0] = 0;
  GetCurrAttribValue(mml_mfrac, true, "linethickness", buffer, 256);
  if (!strcmp(buffer, "0"))
    s_target->semantic_type = SEM_TYP_BINOMIAL;
  else
    s_target->semantic_type = SEM_TYP_FRACTION;
}

void Analyzer::AnalyzeMSQRT(MNODE * mml_msqrt_node,
                            SEMANTICS_NODE * snode, int& nodes_done)
{
  nodes_done = 1;

  BUCKET_REC *parts_list = NULL;
  MNODE *rover = mml_msqrt_node->first_kid;
  if (rover) {
    BUCKET_REC *bucket = MakeBucketRec(MB_ROOT_BASE, NULL);
    parts_list = AppendBucketRec(parts_list, bucket);
    SEMANTICS_NODE *contents = GetSemanticsList(rover, bucket);
    bucket->first_child = contents;
    contents->parent = bucket;
  }

  snode->bucket_list = parts_list;
  snode->semantic_type = SEM_TYP_SQRT;
}

void Analyzer::AnalyzeMROOT(MNODE * mml_mroot_node,
                            SEMANTICS_NODE * snode, int& nodes_done)
{
  nodes_done = 1;

  BUCKET_REC *parts_list = NULL;
  MNODE *rover = mml_mroot_node->first_kid;
  if (rover) {
    BUCKET_REC *base_bucket = MakeBucketRec(MB_ROOT_BASE, NULL);
    parts_list = AppendBucketRec(parts_list, base_bucket);
    SEMANTICS_NODE *base = GetSemanticsFromNode(rover, base_bucket);
    base_bucket->first_child = base;
    base->parent = base_bucket;
    rover = rover->next;
    if (rover) {
      BUCKET_REC *power_bucket = MakeBucketRec(MB_ROOT_EXPONENT, NULL);
      parts_list = AppendBucketRec(parts_list, power_bucket);
      SEMANTICS_NODE *power = GetSemanticsFromNode(rover, power_bucket);
      power_bucket->first_child = power;
      power->parent = power_bucket;
    }
  }

  snode->bucket_list = parts_list;
  snode->semantic_type = SEM_TYP_ROOT;
}

void Analyzer::AnalyzeMFENCED(MNODE * mml_mfenced_node,
                              SEMANTICS_NODE * snode, int& nodes_done)
{
  nodes_done = 1;
  U32 l_unicode = '(';
  U32 r_unicode = ')';

  if (mml_mfenced_node->attrib_list) {
    const char *ilk_value =
      GetATTRIBvalue(mml_mfenced_node->attrib_list, "ilk");
    const char *open_value =
      GetATTRIBvalue(mml_mfenced_node->attrib_list, "open");
    const char *close_value =
      GetATTRIBvalue(mml_mfenced_node->attrib_list, "close");
    if (open_value) {
      if (*open_value == '&')
        l_unicode = ASCII2U32(open_value + 3, 16);
      else
        l_unicode = open_value[0];
    }
    if (close_value) {
      if (*close_value == '&')
        r_unicode = ASCII2U32(close_value + 3, 16);
      else
        r_unicode = close_value[0];
    }

    char *key = NULL;
    if (ilk_value && !strcmp(ilk_value, "enclosed-list")) {
      if (l_unicode == '(') {
        snode->semantic_type = SEM_TYP_PARENED_LIST;
      } else if (l_unicode == '[') {
        snode->semantic_type = SEM_TYP_BRACKETED_LIST;
      } else if (l_unicode == '{') {
        snode->semantic_type = SEM_TYP_SET;
      } else {
        TCI_ASSERT(!"Unknown enclosed-list");
      }
    } else if (l_unicode == '|' && r_unicode == '|') {
      snode->semantic_type = SEM_TYP_ABS;
      key = "abs";
    } else if (l_unicode == 0x2016 && r_unicode == 0x2016) {
      snode->semantic_type = SEM_TYP_NORM;
      key = "norm";
    } else if (l_unicode == 0x230A && r_unicode == 0x230B) {
      snode->semantic_type = SEM_TYP_FLOOR;
      key = "floor";
    } else if (l_unicode == 0x2308 && r_unicode == 0x2309) {
      snode->semantic_type = SEM_TYP_CEILING;
      key = "ceil";
    } else if (l_unicode == '{' && r_unicode == 'I') {
      snode->semantic_type = SEM_TYP_PIECEWISE_FENCE;
    } else if (l_unicode == '{' && r_unicode == '}') {
      snode->semantic_type = SEM_TYP_SET;
    } else if (l_unicode == '(' && r_unicode == ']') {
      snode->semantic_type = SEM_TYP_INTERVAL;
      snode->variant = SNV_ExclInclInterval;
    } else if (l_unicode == '[' && r_unicode == ')') {
      snode->semantic_type = SEM_TYP_INTERVAL;
      snode->variant = SNV_InclExclInterval;
    } else if (l_unicode == '{' && r_unicode == 0x250a) {
      snode->semantic_type = SEM_TYP_PIECEWISE_FENCE;
    } else {
      snode->semantic_type = SEM_TYP_GENERIC_FENCE;
    }

    if (key) {
      delete[] snode->contents;
      size_t zln = strlen(key);
      char *tmp = TCI_NEW(char[zln + 1]);
      strcpy(tmp, key);
      snode->contents = tmp;
    }
  } else {
    TCI_ASSERT(!"No attribute list.");
  }
  BUCKET_REC *parts_list = NULL;
  int num_children = 0;
  MNODE *rover = mml_mfenced_node->first_kid;
  while (rover) {
    BUCKET_REC *item_bucket = MakeBucketRec(MB_UNNAMED, NULL);
    parts_list = AppendBucketRec(parts_list, item_bucket);
    SEMANTICS_NODE *s_item = GetSemanticsFromNode(rover, item_bucket);
    item_bucket->first_child = s_item;
    if (s_item) {
      s_item->parent = item_bucket;
    }
    num_children++;
    rover = rover->next;
  }

  // Here we're recording info re input notation
  if (num_children == 1 && parts_list && parts_list->first_child) {
    SEMANTICS_NODE *s_child = parts_list->first_child;
    if (s_child->semantic_type == SEM_TYP_TABULATION) {
      if (p_input_notation) {
        p_input_notation->n_tables++;
        if (l_unicode == '[')
          p_input_notation->nbracket_tables++;
        else if (l_unicode == '(')
          p_input_notation->nparen_tables++;
        else if (l_unicode == '{')
          p_input_notation->nbrace_tables++;
      }    

      // \ matrix \ -> det \ matrix \ .
      if (snode->semantic_type == SEM_TYP_ABS) {
        snode->semantic_type = SEM_TYP_FUNCTION;
        delete[] snode->contents;
        char *tmp = TCI_NEW(char[4]);
        strcpy(tmp, "det");
        snode->contents = tmp;
      }
    } else if (s_child->semantic_type == SEM_TYP_BINOMIAL) {
      if (snode->semantic_type == SEM_TYP_PARENED_LIST)
        snode->semantic_type = SEM_TYP_PRECEDENCE_GROUP;
    }
  }

  snode->bucket_list = parts_list;
  if (snode->semantic_type == SEM_TYP_PIECEWISE_FENCE) {
    // If this type of fence contains a TABULATION with 2 or 3 columns,
    //  we ASSUME that it represents a PIECEWISE function.
    ConvertToPIECEWISElist(snode);
  }

  if (snode->semantic_type == SEM_TYP_NORM && snode->bucket_list) {
    BUCKET_REC *bucket = FindBucketRec(snode->bucket_list, MB_UNNAMED);
    if (bucket && bucket->first_child) {
      // If we've created a "SEM_TYP_NORM", we must guarantee that contents
      //  like [1,2,3] OR (4,5,6) are carried as 1 row SEM_TYP_TABULATIONs
      FenceToMatrix(bucket->first_child);
    }
  }
  if (snode->semantic_type == SEM_TYP_INTERVAL && snode->bucket_list) {
    FenceToInterval(snode);
  }
}

// Lot's more to do in the following

void Analyzer::AnalyzeMSUP(MNODE * mml_msup_node, SEMANTICS_NODE * snode,
                           int& nodes_done, bool isLHSofDef)
{
  nodes_done = 1;

  MNODE *base = mml_msup_node->first_kid;
  if (base) {
    BaseType bt = GetBaseType(mml_msup_node, isLHSofDef);
    ExpType et = GetExpType(bt, base->next);
    bool done = false;

    // First look for a superscript that dictates semantics
    switch (et) {
    case ET_CONJUGATE_INDICATOR:{
        BUCKET_REC *base_bucket = MakeBucketRec(MB_UNNAMED, NULL);
        snode->bucket_list = AppendBucketRec(snode->bucket_list, base_bucket);
        SEMANTICS_NODE *s_base = GetSemanticsFromNode(base, base_bucket);
        if (s_base->semantic_type == SEM_TYP_GENERIC_FENCE)
          s_base->semantic_type = SEM_TYP_PRECEDENCE_GROUP;
        base_bucket->first_child = s_base;
        s_base->parent = base_bucket;

        snode->semantic_type = SEM_TYP_CONJUGATE;
        done = true;
      }
      break;
    case ET_TRANSPOSE_INDICATOR:{
        BUCKET_REC *base_bucket = MakeBucketRec(MB_UNNAMED, NULL);
        snode->bucket_list = AppendBucketRec(snode->bucket_list, base_bucket);
        SEMANTICS_NODE *s_base = GetSemanticsFromNode(base, base_bucket);
        base_bucket->first_child = s_base;
        s_base->parent = base_bucket;

        snode->semantic_type = SEM_TYP_MTRANSPOSE;
        done = true;
      }
      break;
    case ET_HTRANSPOSE_INDICATOR:{
        BUCKET_REC *base_bucket = MakeBucketRec(MB_UNNAMED, NULL);
        snode->bucket_list = AppendBucketRec(snode->bucket_list, base_bucket);
        SEMANTICS_NODE *s_base = GetSemanticsFromNode(base, base_bucket);
        base_bucket->first_child = s_base;
        s_base->parent = base_bucket;

        snode->semantic_type = SEM_TYP_HTRANSPOSE;
        done = true;
      }
      break;
    case ET_PRIMES:{
        if (prime_is_derivative) {
          AnalyzePrimed(mml_msup_node, snode, nodes_done);
          p_input_notation->n_primes++;
          done = true;
        } else {
          // Here I'm currently assuming that prime is just a decoration
        }
      }
      break;

    case ET_PARENED_PRIMES_COUNT:{
        AnalyzePrimed(mml_msup_node, snode, nodes_done);
        done = true;
      }
      break;

    default:
      break;
    }

    // Look for a base/superscript pair that dictates semantics
    if (!done) {
      switch (bt) {

      case BT_OPERATOR:
        TranslateEmbellishedOp(mml_msup_node, snode, nodes_done);
        break;

      case BT_FUNCTION:
      case BT_SUBARG_FUNCTION:{
          if (et == ET_INVERSE_INDICATOR) {
            char *mml_canonical_name = GetCanonicalIDforMathNode(mml_msup_node);
            if (!mml_canonical_name) {
              snode->error_flag = 1;
              return;
            }
            // Put the canonical name of the math object in "snode".
            snode->canonical_ID = mml_canonical_name;
            SetSnodeOwner(snode);
            // Store a name-to-node back mapping record for this object.
            node_IDs_list = AppendIDRec(node_IDs_list, curr_client_ID,
                                        mml_canonical_name, mml_msup_node,
                                        z_scr_str);
            int local_nodes_done;
            BUCKET_REC *br = ArgsToBucket(mml_msup_node, local_nodes_done);
            nodes_done += local_nodes_done;
            if (br)
              snode->bucket_list = AppendBucketRec(snode->bucket_list, br);

            size_t ln = strlen(base->p_chdata);
            char *tmp = TCI_NEW(char[ln + 1]);
            strcpy(tmp, base->p_chdata);
            snode->contents = tmp;
            snode->semantic_type = SEM_TYP_INVFUNCTION;
          } else if (et == ET_POWER || et == ET_DIRECTION) {
            char *mml_canonical_name = GetCanonicalIDforMathNode(base);
            if (!mml_canonical_name) {
              snode->error_flag = 1;
              return;
            }
            // Put the canonical name of the math object in "snode".
            snode->canonical_ID = mml_canonical_name;
            SetSnodeOwner(snode);
            // Store a name-to-node back mapping record for this object.
            node_IDs_list = AppendIDRec(node_IDs_list, curr_client_ID,
                                        mml_canonical_name, base, z_scr_str);

            BUCKET_REC *power_bucket = MakeBucketRec(MB_FUNC_EXPONENT, NULL);
            snode->bucket_list =
              AppendBucketRec(snode->bucket_list, power_bucket);
            SEMANTICS_NODE *s_power =
              GetSemanticsFromNode(base->next, power_bucket);
            power_bucket->first_child = s_power;
            s_power->parent = power_bucket;

            // We're handling something like "f^{2}(x)"
            // We must process function arguments here.
            int local_nodes_done;
            BUCKET_REC *br = ArgsToBucket(mml_msup_node, local_nodes_done);
            nodes_done += local_nodes_done;
            if (br)
              snode->bucket_list = AppendBucketRec(snode->bucket_list, br);
            else {
              // The function may have an implicit argument that must be generated here
              BUCKET_REC *bucket =
                FindBucketRec(snode->bucket_list, MB_UNNAMED);
              if (!bucket || !bucket->first_child) {
                if (bucket)
                  RemoveBucket(snode, bucket);
                SEMANTICS_NODE *s_var_list = NULL;
                if (LocateFuncRec
                    (DE_func_names, mml_canonical_name, base->p_chdata))
                  s_var_list = DE_ind_vars;
                else
                  if (LocateFuncRec
                      (IMPLDIFF_func_names, mml_canonical_name,
                       base->p_chdata))
                  s_var_list = IMPLDIFF_ind_var;
                if (s_var_list) {
                  BUCKET_REC *fvar_bucket =
                    AddVarToBucket(MB_UNNAMED, s_var_list);
                  snode->bucket_list =
                    AppendBucketRec(snode->bucket_list, fvar_bucket);
                }
              }
            }

            // Add remaining function info
            size_t ln = strlen(base->p_chdata);
            char *tmp = TCI_NEW(char[ln + 1]);
            strcpy(tmp, base->p_chdata);
            snode->contents = tmp;
            snode->semantic_type = SEM_TYP_FUNCTION;
            if (base->p_chdata && !strcmp(base->p_chdata, "log"))
              AddDefaultBaseToLOG(snode);
          } else if (et == ET_PRIMES) {
            TCI_ASSERT(0);
          } else
            TCI_ASSERT(0);
        }
        break;

      case BT_VARIABLE:{
          if (et == ET_INVERSE_INDICATOR || et == ET_POWER) {
            CreatePowerForm(base, base->next, snode);
          } else if (et == ET_PRIMES) {

            // Here we have a decorated variable
            char *mml_canonical_name = GetCanonicalIDforMathNode(mml_msup_node);
            if (!mml_canonical_name) {
              snode->error_flag = 1;
              return;
            }
            snode->canonical_ID = mml_canonical_name;
            SetSnodeOwner(snode);
            node_IDs_list = AppendIDRec(node_IDs_list, curr_client_ID,
                                        mml_canonical_name, mml_msup_node,
                                        z_scr_str);
            snode->semantic_type = SEM_TYP_VARIABLE;
          } else
            TCI_ASSERT(0);
        }
        break;

      case BT_MOVER:
      case BT_MATRIX:
      case BT_UNIT:
      case BT_FENCED:
      case BT_NUMBER:{
          if (et == ET_POWER || et == ET_DIRECTION) {
            CreatePowerForm(base, base->next, snode);
          } else
            TCI_ASSERT(0);
        }
        break;

      case BT_TRANSFORM:{
          if (et == ET_INVERSE_INDICATOR) {
            char *mml_canonical_name = GetCanonicalIDforMathNode(mml_msup_node);
            if (!mml_canonical_name) {
              snode->error_flag = 1;
              return;
            }
            // Put the canonical name of the math object in "snode".
            snode->canonical_ID = mml_canonical_name;
            SetSnodeOwner(snode);
            // Store a name-to-node back mapping record for this object.
            node_IDs_list = AppendIDRec(node_IDs_list, curr_client_ID,
                                        mml_canonical_name, mml_msup_node,
                                        z_scr_str);
            int local_nodes_done;
            BUCKET_REC *br = ArgsToBucket(mml_msup_node, local_nodes_done);
            nodes_done += local_nodes_done;
            if (br)
              snode->bucket_list = AppendBucketRec(snode->bucket_list, br);
            size_t ln = strlen(base->p_chdata);
            char *tmp = TCI_NEW(char[ln + 1]);
            strcpy(tmp, base->p_chdata);
            snode->contents = tmp;
            snode->semantic_type = SEM_TYP_INVTRANSFORM;
          } else
            TCI_ASSERT(0);
        }
        break;

      default:
        TCI_ASSERT(0);
        break;
      }
    }
  } else
    TCI_ASSERT(0);
}

void Analyzer::AnalyzeMSUB(MNODE * mml_msub_node, SEMANTICS_NODE * snode,
                           int& nodes_done, bool isLHSofDef)
{
  nodes_done = 1;

  MNODE *base = mml_msub_node->first_kid;
  if (IsSUBSTITUTION(mml_msub_node)) {
    BUCKET_REC *base_bucket = MakeBucketRec(MB_UNNAMED, NULL);
    snode->bucket_list = AppendBucketRec(snode->bucket_list, base_bucket);
    SEMANTICS_NODE *s_base = GetSemanticsFromNode(base, base_bucket);
    base_bucket->first_child = s_base;
    s_base->parent = base_bucket;

    MNODE *subst = base->next;
    CreateSubstBucket(subst, snode, true);

    snode->semantic_type = SEM_TYP_SUBSTITUTION;
    char *tmp = TCI_NEW(char[13]);
    strcpy(tmp, "substitution");
    snode->contents = tmp;
    return;
  }

  if (IsDDIFFOP(mml_msub_node)) { // D_{x^{5}y^{2}}
    MNODE *base = mml_msub_node->first_kid;
    if (base->next) {
      MNODE *sub = base->next;
      BUCKET_REC *var_bucket = MakeBucketRec(MB_DIFF_VAR, NULL);
      snode->bucket_list = AppendBucketRec(snode->bucket_list, var_bucket);
      MNODE *m_var = sub;
      if (!strcmp(m_var->src_tok, "mrow"))
        m_var = sub->first_kid;

      SEMANTICS_NODE *s_var = GetSemanticsList(m_var, var_bucket);

      // Remove infix operators?
      s_var = RemoveInfixOps(s_var);
      var_bucket->first_child = s_var;
    } else {
      TCI_ASSERT(0);
    }

    // The expression being differentiated should follow.
    if (mml_msub_node->next) {
      int nodes_in_arg;
      BUCKET_REC *br = ArgsToBucket(mml_msub_node, nodes_in_arg);
      nodes_done += nodes_in_arg;
      if (br) {
        snode->bucket_list = AppendBucketRec(snode->bucket_list, br);
      }
    }

    snode->semantic_type = SEM_TYP_DERIVATIVE;
    char *tmp = TCI_NEW(char[14]);
    strcpy(tmp, "differentiate");
    snode->contents = tmp;
    p_input_notation->n_Dxs++;
    return;
  }

  if (IsBesselFunc(mml_msub_node)) {
    AnalyzeBesselFunc(mml_msub_node, snode, nodes_done);
    return;
  }

  if (base) {
    BaseType bt = GetBaseType(mml_msub_node, isLHSofDef);
    ExpType sub_type = GetSubScriptType(mml_msub_node, bt, base->next);

    switch (bt) {
    case BT_OPERATOR:
      TranslateEmbellishedOp(mml_msub_node, snode, nodes_done);
      break;

    case BT_FUNCTION:{
        // Note that \log_{n} is handled below
        if (sub_type == ET_NUMBER || sub_type == ET_DECORATION  || sub_type == ET_VARIABLE) {
          char *mml_canonical_name = GetCanonicalIDforMathNode(mml_msub_node);
          if (!mml_canonical_name) {
            snode->error_flag = 1;
            return;
          }
          // Put the canonical name of the math object in "snode".
          snode->canonical_ID = mml_canonical_name;
          SetSnodeOwner(snode);
          // Store a name-to-node back mapping record for this object.
          node_IDs_list = AppendIDRec(node_IDs_list, curr_client_ID,
                                      mml_canonical_name, mml_msub_node,
                                      z_scr_str);

          AnalyzeSubscriptedFunc(mml_msub_node, snode, nodes_done);
        } else {
          TCI_ASSERT(0);
        }
      }
      break;

    case BT_SUBARG_FUNCTION:{
        char *mml_canonical_name = GetCanonicalIDforMathNode(base);
        if (!mml_canonical_name) {
          snode->error_flag = 1;
          return;
        }
        // Put the canonical name of the math object in "snode".
        snode->canonical_ID = mml_canonical_name;
        SetSnodeOwner(snode);
        // Store a name-to-node back mapping record for this object.
        node_IDs_list = AppendIDRec(node_IDs_list, curr_client_ID,
                                    mml_canonical_name, base, z_scr_str);

        AnalyzeSubscriptedArgFunc(mml_msub_node, snode);
      }
      break;

    case BT_VARIABLE:{
        if (sub_type == ET_DECORATION) {
          char *mml_canonical_name = GetCanonicalIDforMathNode(mml_msub_node);
          if (!mml_canonical_name) {
            snode->error_flag = 1;
            return;
          }
          snode->canonical_ID = mml_canonical_name;
          SetSnodeOwner(snode);
          node_IDs_list = AppendIDRec(node_IDs_list, curr_client_ID,
                                      mml_canonical_name, mml_msub_node,
                                      z_scr_str);
          snode->semantic_type = SEM_TYP_VARIABLE;
        } else if (sub_type == ET_VARIABLE || sub_type == ET_NUMBER ||
                   sub_type == ET_EXPRESSION) {
          if (cmd_ID == CCID_Solve_Recursion) { // Solve Recursion
            // Generate a function call
            MSUB2FuncCall(mml_msub_node, snode);
            p_input_notation->funcarg_is_subscript++;
          } else {
            // We generate a "qualified" variable here.  The engine is given enough info
            //  to treat this object as an ordinary variable, or something more complex
            //  like an indexed variable.
            CreateSubscriptedVar(mml_msub_node, false, snode);
          }
        } else {
          TCI_ASSERT(0);
        }
      }
      break;

    case BT_NUMBER:{
        TCI_ASSERT(!"Don't know what to do with subscripted number.");
      }
      break;

    case BT_FENCED:
      AnalyzeSubscriptedFence(mml_msub_node, snode, nodes_done);
      break;

    default:
      TCI_ASSERT(!"Unexpected base type.");
      break;
    }
  } else {
    TCI_ASSERT(!"No base!");
  }
}

// Obviously, lots more to do here!

void Analyzer::AnalyzeMSUBSUP(MNODE * mml_msubsup_node,
                              SEMANTICS_NODE * snode, int& nodes_done,
                              bool isLHSofDef)
{
  nodes_done = 1;

  MNODE *base = mml_msubsup_node->first_kid;
  if (IsSUBSTITUTION(mml_msubsup_node)) {
    BUCKET_REC *base_bucket = MakeBucketRec(MB_UNNAMED, NULL);
    snode->bucket_list = AppendBucketRec(snode->bucket_list, base_bucket);
    SEMANTICS_NODE *s_base = GetSemanticsFromNode(base, base_bucket);
    if (s_base->semantic_type == SEM_TYP_GENERIC_FENCE || s_base->semantic_type == SEM_TYP_BRACKETED_LIST)
      s_base->semantic_type = SEM_TYP_PRECEDENCE_GROUP;
    base_bucket->first_child = s_base;
    s_base->parent = base_bucket;

    MNODE *lower = base->next;
    CreateSubstBucket(lower, snode, true);

    MNODE *upper = lower->next;
    CreateSubstBucket(upper, snode, false);

    snode->semantic_type = SEM_TYP_SUBSTITUTION;
    char *tmp = TCI_NEW(char[19]);
    strcpy(tmp, "doublesubstitution");
    snode->contents = tmp;

    return;
  }

  if (base) {
    BaseType bt = GetBaseType(mml_msubsup_node, isLHSofDef);
    ExpType sub_type = GetSubScriptType(mml_msubsup_node, bt, base->next);
    ExpType et = GetExpType(bt, base->next->next);

    bool done = false;

    // First look for a superscript that dictates semantics
    switch (et) {
    case ET_CONJUGATE_INDICATOR:{
        BUCKET_REC *base_bucket = MakeBucketRec(MB_UNNAMED, NULL);
        snode->bucket_list = AppendBucketRec(snode->bucket_list, base_bucket);
        SEMANTICS_NODE *s_base = GetSemanticsFromNode(base, base_bucket);
        base_bucket->first_child = s_base;
        s_base->parent = base_bucket;

        snode->semantic_type = SEM_TYP_CONJUGATE;
        done = true;
      }
      break;
    case ET_TRANSPOSE_INDICATOR:
    case ET_HTRANSPOSE_INDICATOR:{
        TCI_ASSERT(0);
      }
      break;
    case ET_PRIMES:{
        // For now, I'm assuming primes indicate differentiation
        if (bt == BT_VARIABLE
            || bt == BT_FUNCTION || bt == BT_SUBARG_FUNCTION) {

          snode->semantic_type = SEM_TYP_DERIVATIVE;
          char *key = "differentiate";
          size_t zln = strlen(key);
          char *tmp = TCI_NEW(char[zln + 1]);
          strcpy(tmp, key);
          snode->contents = tmp;

          MNODE *base = mml_msubsup_node->first_kid;
          SEMANTICS_NODE *s_func = CreateSemanticsNode();

          if (bt == BT_SUBARG_FUNCTION) {
            s_func->canonical_ID = GetCanonicalIDforMathNode(base);
            AnalyzeSubscriptedArgFunc(mml_msubsup_node, s_func);
            BUCKET_REC *base_bucket = MakeBucketRec(MB_UNNAMED, s_func);
            snode->bucket_list =
              AppendBucketRec(snode->bucket_list, base_bucket);
            base_bucket->first_child = s_func;
            s_func->parent = base_bucket;
          } else {
            char *mml_canonical_name = NULL;
            U32 zh_ln = 0;
            mml_canonical_name =
              AppendStr2HeapStr(mml_canonical_name, zh_ln, "msub");
            tmp = GetCanonicalIDforMathNode(base);
            if (tmp) {
              mml_canonical_name =
                AppendStr2HeapStr(mml_canonical_name, zh_ln, tmp);
              delete[] tmp;
            }
            tmp = GetCanonicalIDforMathNode(base->next);
            if (tmp) {
              mml_canonical_name =
                AppendStr2HeapStr(mml_canonical_name, zh_ln, tmp);
              delete[] tmp;
            }
            // Put the canonical name of the math object in "snode".
            s_func->canonical_ID = mml_canonical_name;
            SetSnodeOwner(s_func);
            // Store a name-to-node back mapping record for this object.
            node_IDs_list = AppendIDRec(node_IDs_list, curr_client_ID,
                                        mml_canonical_name, mml_msubsup_node,
                                        z_scr_str);

            AnalyzeSubscriptedFunc(mml_msubsup_node, s_func, nodes_done);
            BUCKET_REC *base_bucket = MakeBucketRec(MB_UNNAMED, s_func);
            snode->bucket_list =
              AppendBucketRec(snode->bucket_list, base_bucket);
            base_bucket->first_child = s_func;
            s_func->parent = base_bucket;

            if (DE_ind_vars) {
              BUCKET_REC *dvar_bucket =
                AddVarToBucket(MB_DIFF_VAR, DE_ind_vars);
              snode->bucket_list =
                AppendBucketRec(snode->bucket_list, dvar_bucket);
            } else {
              TCI_ASSERT(0);
            }
          }
          MNODE *primes = base->next->next;
          AddPrimesCount(snode, primes);
        } else
          TCI_ASSERT(0);
        done = true;
      }
    default:
      break;
    }

    // Look for a base that dictates semantics
    if (!done) {
      switch (bt) {

      case BT_OPERATOR:
        TranslateEmbellishedOp(mml_msubsup_node, snode, nodes_done);
        break;

      case BT_VARIABLE:{
          MNODE *mml_base = mml_msubsup_node->first_kid;
          MNODE *mml_power = mml_base->next->next;
          CreatePowerForm(mml_base, mml_power, snode);
          BUCKET_REC *bucket =
            FindBucketRec(snode->bucket_list, MB_SCRIPT_BASE);
          if (bucket) {
            DisposeSList(bucket->first_child);
            SEMANTICS_NODE *s_var = CreateSemanticsNode();
            CreateSubscriptedVar(mml_msubsup_node, true, s_var);
            bucket->first_child = s_var;
            s_var->parent = bucket;
          } else
            TCI_ASSERT(0);
        }
        break;

      case BT_SUBARG_FUNCTION:{
          MNODE *f_name = mml_msubsup_node->first_kid;
          MNODE *f_arg = f_name->next;
          MNODE *f_exp = f_arg->next;

          SEMANTICS_NODE *s_func = CreateSemanticsNode();

          // Create a canonical name for "s_func".
          s_func->canonical_ID = GetCanonicalIDforMathNode(f_name);
          AnalyzeSubscriptedArgFunc(mml_msubsup_node, s_func);
          SetSnodeOwner(s_func);

          // Create a semantic power form
          BUCKET_REC *base_bucket = MakeBucketRec(MB_SCRIPT_BASE, NULL);
          snode->bucket_list =
            AppendBucketRec(snode->bucket_list, base_bucket);
          base_bucket->first_child = s_func;
          s_func->parent = base_bucket;

          BUCKET_REC *power_bucket = MakeBucketRec(MB_SCRIPT_UPPER, NULL);
          snode->bucket_list =
            AppendBucketRec(snode->bucket_list, power_bucket);
          SEMANTICS_NODE *s_power = GetSemanticsFromNode(f_exp, power_bucket);
          power_bucket->first_child = s_power;
          s_power->parent = power_bucket;

          snode->semantic_type = SEM_TYP_POWERFORM;
        }
        break;

      case BT_FENCED: {
          MNODE *f_fence = mml_msubsup_node->first_kid;
          MNODE *f_norm = f_fence->next;
          MNODE *f_exp = f_norm->next;

          SEMANTICS_NODE *s_fence = CreateSemanticsNode();
          AnalyzeSubscriptedFence(mml_msubsup_node, s_fence, nodes_done);

          // Create a semantic power form
          BUCKET_REC *base_bucket = MakeBucketRec(MB_SCRIPT_BASE, NULL);
          snode->bucket_list =
            AppendBucketRec(snode->bucket_list, base_bucket);
          base_bucket->first_child = s_fence;
          s_fence->parent = base_bucket;

          BUCKET_REC *power_bucket = MakeBucketRec(MB_SCRIPT_UPPER, NULL);
          snode->bucket_list =
            AppendBucketRec(snode->bucket_list, power_bucket);
          SEMANTICS_NODE *s_power = GetSemanticsFromNode(f_exp, power_bucket);
          power_bucket->first_child = s_power;
          s_power->parent = power_bucket;

          snode->semantic_type = SEM_TYP_POWERFORM;
      }
      break;

      case BT_FUNCTION:
      case BT_NUMBER:
      case BT_UNIT:
      case BT_MATRIX:
      case BT_TRANSFORM:
      case BT_MOVER:
      default:
        TCI_ASSERT(0);
        break;
      }
    }
  } else
    TCI_ASSERT(0);
}

void Analyzer::AnalyzeMOVER(MNODE * mml_mover_node, SEMANTICS_NODE * snode,
                            int& nodes_done, bool isLHSofDef)
{
  nodes_done = 1;

  MNODE *base = mml_mover_node->first_kid;
  if (base) {
    BaseType bt = GetBaseType(mml_mover_node, isLHSofDef);
    AccentType top_type = GetAboveType(bt, base->next);

    if (top_type == OT_BAR
        && base->p_chdata && !strcmp(base->p_chdata, "lim")) {
      TranslateEmbellishedOp(mml_mover_node, snode, nodes_done);
      return;
    }

    bool done = false;

    // First look for a top decoration that dictates semantics
    switch (top_type) {
    case OT_DOT:
    case OT_DDOT:
    case OT_DDDOT:
    case OT_DDDDOT:
      if (dot_is_derivative) {
        int n_dots = top_type - OT_DOT + 1;
        AnalyzeDotDerivative(mml_mover_node, n_dots, snode, nodes_done);
        p_input_notation->n_dotaccents++;
        done = true;
      }
      break;
    case OT_BAR:
      if (overbar_conj) {
        BUCKET_REC *base_bucket = MakeBucketRec(MB_UNNAMED, NULL);
        snode->bucket_list = AppendBucketRec(snode->bucket_list, base_bucket);
        SEMANTICS_NODE *s_base = GetSemanticsFromNode(base, base_bucket);
        base_bucket->first_child = s_base;
        s_base->parent = base_bucket;

        snode->semantic_type = SEM_TYP_CONJUGATE;
        p_input_notation->n_overbars++;
        done = true;
      }
      break;

    default:
      break;
    }

    // Look for a base/decoration pair that dictates semantics
    if (!done) {
      switch (bt) {

      case BT_OPERATOR:
        TCI_ASSERT(0);
        break;

      case BT_FUNCTION:
      case BT_SUBARG_FUNCTION:{
          TCI_ASSERT(0);
        }
        break;

      case BT_VARIABLE:{
        }
        break;

      case BT_MATRIX:
      case BT_UNIT:
      case BT_FENCED:
      case BT_NUMBER:{
          TCI_ASSERT(0);
        }
        break;

      case BT_TRANSFORM:{
          TCI_ASSERT(0);
        }
        break;

      case BT_MOVER:{
          TCI_ASSERT(0);
        }
        break;

      default:
        TCI_ASSERT(0);
        break;
      }

      if (!done) {
        char *mml_canonical_name = GetCanonicalIDforMathNode(mml_mover_node);
        if (!mml_canonical_name) {
          snode->error_flag = 1;
          return;
        }
        snode->canonical_ID = mml_canonical_name;
        SetSnodeOwner(snode);
        node_IDs_list = AppendIDRec(node_IDs_list, curr_client_ID,
                                    mml_canonical_name, mml_mover_node,
                                    z_scr_str);
        snode->semantic_type = SEM_TYP_VARIABLE;
      }
    }
  } else
    TCI_ASSERT(0);
}

/* Much work remains in the following - currently it only handles
   something like "\limfunc{lim}_{x\rightarrow\infty}\frac{1}{x}"
*/

void Analyzer::AnalyzeMUNDER(MNODE * mml_munder_node, SEMANTICS_NODE * snode,
                             int& nodes_done, bool isLHSofDef)
{
  nodes_done = 1;

  MNODE *base = mml_munder_node->first_kid;
  if (base) {
    BaseType bt = GetBaseType(mml_munder_node, isLHSofDef);
    bool done = false;

    // First look for an under decoration that dictates semantics
    /*
        AccentType under_type = GetUnderType(bt, base->next);
        switch ( under_type ) {
	      case  UT_WHATEVER :
	      default :
	      break;
        }
    */
    // Look for a base/decoration pair that dictates semantics
    if (!done) {
      switch (bt) {
      case BT_OPERATOR:
        TranslateEmbellishedOp(mml_munder_node, snode, nodes_done);
        done = true;
        break;
      case BT_FUNCTION:
      case BT_SUBARG_FUNCTION:{
          TCI_ASSERT(0);
        }
        break;

      case BT_VARIABLE:{
        }
        break;

      case BT_MATRIX:
      case BT_UNIT:
      case BT_FENCED:
      case BT_NUMBER:{
          TCI_ASSERT(0);
        }
        break;

      case BT_TRANSFORM:{
          TCI_ASSERT(0);
        }
        break;

      case BT_MOVER:{
          TCI_ASSERT(0);
        }
        break;

      default:
        TCI_ASSERT(0);
        break;
      }

      if (!done) {
        char *mml_canonical_name = GetCanonicalIDforMathNode(mml_munder_node);
        if (!mml_canonical_name) {
          snode->error_flag = 1;
          return;
        }
        snode->canonical_ID = mml_canonical_name;
        SetSnodeOwner(snode);
        node_IDs_list = AppendIDRec(node_IDs_list, curr_client_ID,
                                    mml_canonical_name, mml_munder_node,
                                    z_scr_str);
        snode->semantic_type = SEM_TYP_VARIABLE;
      }
    }
  } else
    TCI_ASSERT(0);
}

void Analyzer::AnalyzeMUNDEROVER(MNODE * mml_munderover_node,
                                 SEMANTICS_NODE * snode, int& nodes_done,
                                 bool isLHSofDef)
{
  nodes_done = 1;

  MNODE *base = mml_munderover_node->first_kid;
  if (base) {
    BaseType bt = GetBaseType(mml_munderover_node, isLHSofDef);
    switch (bt) {

    case BT_OPERATOR:
      TranslateEmbellishedOp(mml_munderover_node, snode, nodes_done);
      break;

    case BT_VARIABLE:
    case BT_FUNCTION:
    case BT_SUBARG_FUNCTION:
    case BT_NUMBER:
    case BT_FENCED:
    case BT_MATRIX:
    default:
      TCI_ASSERT(0);
      break;
    }
  } else
    TCI_ASSERT(0);
}

// <mtable> (TABULATION) is used to carry a lot of different schemata

void Analyzer::AnalyzeMTABLE(MNODE * mml_mtable_node,
                             SEMANTICS_NODE * info, int& nodes_done)
{
  nodes_done = 1;

  int row_tally = 0;
  int max_cols = 0;
  BUCKET_REC *cell_list = NULL;

  if (mml_mtable_node->first_kid) {
    // count rows and columns
    MNODE *row_rover = mml_mtable_node->first_kid;
    while (row_rover) {
      int n_cols = CountCols(row_rover);
      if (n_cols > max_cols)
        max_cols = n_cols;

      row_tally++;
      row_rover = row_rover->next;
    }

    // locate the table cells
    row_rover = mml_mtable_node->first_kid;
    while (row_rover) {         // loop down thru rows
      if (row_rover->first_kid) {
        MNODE *col_rover = row_rover->first_kid;

        int col_counter = 0;
        while (col_counter < max_cols) {
          MNODE *mml_cell = NULL;
          int nodes_in_arg = 0;
          if (col_rover) {
            mml_cell = col_rover;
            nodes_in_arg = 1;
            col_rover = col_rover->next;
          }

          BUCKET_REC *cell_bucket = MakeBucketRec(MB_UNNAMED, NULL);
          cell_list = AppendBucketRec(cell_list, cell_bucket);
          col_counter++;

          if (mml_cell) {
            if (!strcmp(mml_cell->src_tok, "mtd")) {
              if (mml_cell->first_kid)
                mml_cell = mml_cell->first_kid;
              else
                mml_cell = NULL;
            }
            if (mml_cell) {
              SEMANTICS_NODE *s_cell = GetSemanticsList(mml_cell, cell_bucket);
              cell_bucket->first_child = s_cell;
              if (s_cell)
                s_cell->parent = cell_bucket;
            }
          }
        }
      }
      row_rover = row_rover->next;
    }
  } else {
    TCI_ASSERT(0);
  }
  char *tmp = TCI_NEW(char[7]);
  strcpy(tmp, "matrix");
  info->contents = tmp;

  info->semantic_type = SEM_TYP_TABULATION;
  info->nrows = row_tally;
  info->ncols = max_cols;
  info->bucket_list = cell_list;
}

Analyzer::IdentIlk Analyzer::GetMIilk(char *mi_canonical_str, MNODE * m_node,
                            bool isLHSofDef)
{
  IdentIlk rv = MI_none;

  if (defstore) {
    DefInfo *di = defstore->GetDefInfo(curr_engine_ID, mi_canonical_str);
    if (di) {
      if (isLHSofDef) {
        // Here, we're re-defining a symbol - the type of any previous def is irrelevent
      } else {
        if (di->def_type == DT_FUNCTION)
          rv = MI_function;
        else if (di->def_type == DT_VARIABLE)
          rv = MI_variable;
        else
          TCI_ASSERT(0);
        return rv;
      }
    }
  } else {
    TCI_ASSERT(!"No def store!");
    return rv;
  }
  if (!strcmp(mi_canonical_str, "mi&#x3c0;")) {
    rv = MI_pi;
  } else if (!strcmp(mi_canonical_str, "mii")) {  //  i
    if (i_is_imaginary)
      rv = MI_imaginaryunit;
  } else if (!strcmp(mi_canonical_str, "mi&#x2148;")) { // imaginary i
    rv = MI_imaginaryunit;
  } else if (!strcmp(mi_canonical_str, "mij")) {  //  j
    if (j_is_imaginary)
      rv = MI_imaginaryunit;
  } else if (!strcmp(mi_canonical_str, "mie")) {
    if (e_is_Euler)
      rv = MI_Eulere;
  } else if (!strcmp(mi_canonical_str, "mi&#x2147;")) {
    rv = MI_Eulere;
  } else if (!strcmp(mi_canonical_str, "mi&#x221e;")) {
    rv = MI_infinity;
  } else if (!strcmp(mi_canonical_str, "migamma")) {
    rv = MI_Eulergamma;
  } else if (!strcmp(mi_canonical_str, "mi&#x2112;")) {
    rv = MI_Laplace;
  } else if (!strcmp(mi_canonical_str, "mi&#x2131;")) {
    rv = MI_Fourier;
  }

  if (IdentIsConstant(rv)) {
    if (m_node->next && !strcmp(m_node->next->src_tok, "mo")) {
      U32 unicodes[8];
      int content_tally =
        ChData2Unicodes(m_node->next->p_chdata, unicodes, 8);
      if (content_tally == 1 && unicodes[0] == 0x2061)
        rv = MI_function;
    }
  }

  return rv;
}

Analyzer::IdentIlk Analyzer::GetMSIilk(char *msi_class)
{
  IdentIlk rv = MI_none;

  if (!stricmp(msi_class, "enginefunction")) {
    rv = MI_function;
  } else if (!stricmp(msi_class, "enginevariable")) {
    rv = MI_variable;
  } else {
    TCI_ASSERT(!"Unexpected msi_class value.");
  }
  return rv;
}

bool Analyzer::IdentIsConstant(IdentIlk ilk)
{
  switch (ilk) {
    case MI_pi:
    case MI_imaginaryunit:
    case MI_Eulere:
    case MI_infinity:
    case MI_Eulergamma:
      return true;
    default:
      return false;
  }
}

int Analyzer::CountCols(MNODE * mml_mtr)
{
  int rv = 0;
  if (mml_mtr && mml_mtr->first_kid) {
    MNODE *rover = mml_mtr->first_kid;
    while (rover) {
      if (!strcmp(rover->src_tok, "mtd"))
        rv++;
      rover = rover->next;
    }
  }
  return rv;
}

bool Analyzer::IsWhiteSpace(MNODE * mml_node)
{
  return false;
}

/*
       <mml:mi mathcolor="gray">sin</mml:mi>
       <mml:mo>&ApplyFunction;</mml:mo>
       <mml:mi>x</mml:mi>
*/

BUCKET_REC *Analyzer::ArgsToBucket(MNODE * func_node, int& nodes_done)
{
  nodes_done = 0;
  BUCKET_REC *a_rec = NULL;

  int local_nodes_done = 0;
  if (func_node && func_node->next) {
    MNODE *mml_rover = func_node->next;

    bool found_ap = false;
    bool found_args = false;

    // step over any whitespace
    while (IsWhiteSpace(mml_rover)) {
      local_nodes_done++;
      mml_rover = mml_rover->next;
    }

    // look for <mo>&ApplyFunction;</mo>
    if (mml_rover && !strcmp(mml_rover->src_tok, "mo")) {
      U32 unicodes[8];
      int content_tally = ChData2Unicodes(mml_rover->p_chdata, unicodes, 8);
      if (content_tally == 1 && unicodes[0] == 0x2061) {
        found_ap = true;
        local_nodes_done++;
        mml_rover = mml_rover->next;
      }
    }
    // step over any whitespace
    while (IsWhiteSpace(mml_rover)) {
      local_nodes_done++;
      mml_rover = mml_rover->next;
    }

    if (mml_rover) {
      bool got_arg = false;
      if (!strcmp(mml_rover->src_tok, "mrow")) {
        local_nodes_done++;
        a_rec = ArgBucketFromMROW(mml_rover);
        got_arg = true;
      } else if (!strcmp(mml_rover->src_tok, "mo")) {
        int n_nodes;
        a_rec = GetParenedArgs(mml_rover, n_nodes);
        if (a_rec) {
          local_nodes_done += n_nodes;
          got_arg = true;
        }
      } else if (IsArgDelimitingFence(mml_rover)) {
        local_nodes_done++;
        a_rec = GetFencedArgs(mml_rover);
        got_arg = true;
      } else {                  // Here, the rest of the nodes in the list become the arg
        // sin &af; cos &af; x
        a_rec = MakeBucketRec(MB_UNNAMED, NULL);
        SEMANTICS_NODE *s_arg = GetSemanticsList(mml_rover, a_rec);
        a_rec->first_child = s_arg;
        while (mml_rover) {
          local_nodes_done++;
          mml_rover = mml_rover->next;
        }
        got_arg = true;
      }

      if (got_arg && !a_rec)
        a_rec = MakeBucketRec(MB_UNNAMED, NULL);
    } else {                    // no node for arg candidate
      TCI_ASSERT(0);
    }
    if (a_rec)
      nodes_done = local_nodes_done;
  }
  return a_rec;
}

BUCKET_REC *Analyzer::ArgBucketFromMROW(MNODE * mml_mrow)
{
  BUCKET_REC *rv = NULL;

  if (mml_mrow && mml_mrow->first_kid) {
    MNODE *candidate = mml_mrow->first_kid;

    if (!strcmp(candidate->src_tok, "mo")) {
      int nodes_done;
      rv = GetParenedArgs(candidate, nodes_done);
    } else if (!strcmp(candidate->src_tok, "mfenced")) {
      rv = GetFencedArgs(candidate);
    } else {
      // we get here when processing trigargs, "sinh at"
      rv = MakeBucketRec(MB_UNNAMED, NULL);
      SEMANTICS_NODE *s_arg = GetSemanticsList(candidate, rv);
      rv->first_child = s_arg;
    }
  }
  return rv;
}

BUCKET_REC *Analyzer::GetParenedArgs(MNODE * mml_mo, int& nodes_done)
{
  BUCKET_REC *rv = NULL;
  nodes_done = 0;
  int local_nodes_done = 0;

  // Span to matching ")"
  bool matched_parens = false;
  int nodes_within_parens = 0;

  U32 unicodes[8];
  int content_tally = ChData2Unicodes(mml_mo->p_chdata, unicodes, 8);
  if (content_tally == 1 && unicodes[0] == '(') {
    local_nodes_done++;
    MNODE *rover = mml_mo->next;
    while (rover) {
      local_nodes_done++;
      int content_tally = ChData2Unicodes(rover->p_chdata, unicodes, 8);
      if (content_tally == 1 && unicodes[0] == ')') {
        matched_parens = true;
        break;
      }
      nodes_within_parens++;
      rover = rover->next;
    }
  }

  if (matched_parens) {
    MNODE *arg_ptr = mml_mo->next;

    // descend into an mrow here if necessary
    if (nodes_within_parens == 1) {
      if (!strcmp(arg_ptr->src_tok, "mrow"))
        arg_ptr = arg_ptr->first_kid;
    }
    // traverse the first level nodes inside the parens
    bool done = false;
    while (!done) {
      // span the first arg
      MNODE *arg_first_obj = arg_ptr;
      int nodes_in_arg = 0;
      while (arg_ptr) {
        int content_tally = ChData2Unicodes(arg_ptr->p_chdata, unicodes, 8);
        if (content_tally == 1 && unicodes[0] == ',')
          break;
        else if (content_tally == 1 && unicodes[0] == ')') {
          done = true;
          break;
        } else {
          nodes_in_arg++;
          arg_ptr = arg_ptr->next;
        }
      }

      BUCKET_REC *new_a_rec = MakeBucketRec(MB_UNNAMED, NULL);
      rv = AppendBucketRec(rv, new_a_rec);

      SEMANTICS_NODE *s_arg =
        GetSemanticsList(arg_first_obj, new_a_rec, nodes_in_arg, false);
      new_a_rec->first_child = s_arg;

      if (!done) {
        if (arg_ptr)
          arg_ptr = arg_ptr->next;
        else
          done = true;
      }
    }
  }

  // Report nodes_done back to caller
  if (rv)
    nodes_done = local_nodes_done;

  return rv;
}

BUCKET_REC *Analyzer::GetFencedArgs(MNODE * mml_fence)
{
  BUCKET_REC *rv = NULL;

  if (mml_fence && mml_fence->first_kid) {
    MNODE *rover = mml_fence->first_kid;
    while (rover) {
      BUCKET_REC *new_a_rec = MakeBucketRec(MB_UNNAMED, NULL);
      rv = AppendBucketRec(rv, new_a_rec);

      SEMANTICS_NODE *s_arg = GetSemanticsFromNode(rover, new_a_rec);
      new_a_rec->first_child = s_arg;
      if (s_arg)
        s_arg->parent = new_a_rec;

      rover = rover->next;
    }
  }

  return rv;
}

/* Generate canonical IDs (names) for mml nodes that represent variables and functions.

  In this design, these IDs are zstrings and are completely engine independent.

  They must meet the following criteria

  1. mml nodes that represent the same math object must be given
     the same canonical name.
     Note that in presentation MathML, the same math object can have
     many different markups.  For example, a Greek letter may be represented
     by a hex entity, a decimal entity, or one or more name entities.

  2. mml nodes that represent different math objects must be given
     different canonical names.
	 Note that <mi>x</mi> represents a different math object
	 when it is nested in an <mstyle mathvariant="fraktur"/>

  Finding an algorithm that produces names that meet the above criteria
  is not trivial.

  The most general algorithm would involve traversing the tree from
  the root to the node being named.  When schemata that affect the
  semantics of target node are encountered, add something to the ID string
  being created.  Example
  <mstyle mathvariant="fraktur">
    .
	.
	<subtree to be named>
	</subtree to be named>
    .
  </mstyle>

   When the <mstyle> is encountered, append something like mvFRAKTUR
   to the ID.
   Finally traverse the body of the subtree being named, appending
   semantically relevent attributes and contents to the ID.

   The following code doesn't implement the general algorithm completely.
   It can be made more general as required.

*/

char *Analyzer::GetCanonicalIDforMathNode(MNODE * mml_node)
{
  char *rv = NULL;

  if (mml_node) {
    char buffer[1024];
    buffer[0] = 0;
    const char *mml_element = mml_node->src_tok;

    if (!strcmp(mml_element, "mi")) {
      strcat(buffer, mml_element);
      SemanticAttribs2Buffer(buffer, mml_node, 1024);
      Contents2Buffer(buffer, mml_node->p_chdata, 1024);
    } else if (!strcmp(mml_element, "mo")) {
      strcat(buffer, mml_element);
      Contents2Buffer(buffer, mml_node->p_chdata, 1024);
    } else if (!strcmp(mml_element, "mn")) {
      strcat(buffer, mml_element);
      Contents2Buffer(buffer, mml_node->p_chdata, 1024);
    } else if (!strcmp(mml_element, "mtext")) {
      strcat(buffer, mml_element);
      Contents2Buffer(buffer, mml_node->p_chdata, 1024);
    } else {
      if (mml_node->first_kid) {
        if (strcmp(mml_element, "mrow"))
          strcat(buffer, mml_element);
        MNODE *rover = mml_node->first_kid;
        while (rover) {
          char *tmp = GetCanonicalIDforMathNode(rover);
          if (tmp)
            strcat(buffer, tmp);
          delete[] tmp;
          rover = rover->next;
        }
      }
    }
    if (buffer[0]) {
      size_t zln = strlen(buffer);
      rv = TCI_NEW(char[zln + 1]);
      strcpy(rv, buffer);
    }
  }
  return rv;
}

// Note that the MML list is traversed from left to right.
//  Operators are located and their operands are translated
//  and recorded as children of the generated semantic operator node.

SEMANTICS_NODE *Analyzer::GetSemanticsList(MNODE * dMML_list,
                                           BUCKET_REC * parent_bucket)
{
  return GetSemanticsList(dMML_list, parent_bucket, ALL_NODES, false);
}

SEMANTICS_NODE *Analyzer::GetSemanticsList(MNODE * dMML_list,
                                           BUCKET_REC * parent_bucket,
                                           int mml_node_lim,
                                           bool isLHSofDef)
{
  SEMANTICS_NODE *head = NULL;
  SEMANTICS_NODE *tail;

  int mml_nodes_done = 0;
  MNODE *rover = dMML_list;
  while (rover && mml_nodes_done < mml_node_lim) {
    SEMANTICS_NODE *new_node = NULL;
    // Look ahead in the MML list for an operator.
    OpIlk op_ilk;
    int advance;
    MNODE *mo = LocateOperator(rover, op_ilk, advance);

    int mml_nodes_remaining = mml_node_lim - mml_nodes_done;
    if (mo && mml_nodes_remaining > 1) {
      if (mo->p_chdata && !strcmp(mo->p_chdata, "&#x2061;")) {
        // ApplyFunction - translate the function call
        int l_nodes_done = 0;
        new_node = SNodeFromMNodes(rover, l_nodes_done, isLHSofDef);
        while (l_nodes_done) {
          mml_nodes_done++;
          rover = rover->next;
          l_nodes_done--;
        }
      } else {
        SEMANTICS_NODE *l_operand = NULL;
        SEMANTICS_NODE *r_operand = NULL;

        if (op_ilk == OP_infix || op_ilk == OP_postfix) {
          if (head) {
            l_operand = head;
            // Here we're assuming ALL operators in source MML list have
            //  the same precedence (ie. well-formed MML), and are left-associative.
          } else {
            // translate the left operand
            int l_nodes_done = 0;
            l_operand = SNodeFromMNodes(rover, l_nodes_done, isLHSofDef);
            while (l_nodes_done) {
              mml_nodes_done++;
              rover = rover->next;
              l_nodes_done--;
            }
          }
        }
        // translate the operator
        int op_nodes_done = 0;
        new_node = SNodeFromMNodes(rover, op_nodes_done, isLHSofDef);
        while (op_nodes_done) {
          mml_nodes_done++;
          rover = rover->next;
          op_nodes_done--;
        }
        if (op_ilk == OP_prefix) {
          TCI_ASSERT(l_operand == NULL);
          // Note that the operand of a prefix operator is handled
          //  when the operator is translated.
          if (!new_node->bucket_list) {
            // translate the right operand
            int r_nodes_done = 0;
            r_operand = SNodeFromMNodes(rover, r_nodes_done, isLHSofDef);
            while (r_nodes_done) {
              mml_nodes_done++;
              rover = rover->next;
              r_nodes_done--;
            }
            // We can re-work operands here as required.
            // For example, in (1,2,3)x(3,2,1) the parened fences
            // probably represent vectors ( 1 row matrices ).
            //In x \in (-1,3.5] the fence represents an interval
            if (r_operand) {
              OpMatrixIntervalType op_type = GetOpType(mo);
              if (op_type == OMI_matrix)
                FenceToMatrix(r_operand);
            }
            CreatePrefixForm(new_node, l_operand, r_operand);
          }
        } else if (op_ilk == OP_infix) {
          // translate the right operand
          int r_nodes_done = 0;
          r_operand = SNodeFromMNodes(rover, r_nodes_done, isLHSofDef);
          while (r_nodes_done) {
            mml_nodes_done++;
            rover = rover->next;
            r_nodes_done--;
          }
          // We can re-work operands here as required.
          // For example, in (1,2,3)x(3,2,1) the parened fences
          // probably represent vectors ( 1 row matrices ).
          OpMatrixIntervalType op_type = GetOpType(mo);
          if (op_type == OMI_matrix) {
            if (l_operand)
              FenceToMatrix(l_operand);
            if (r_operand)
              FenceToMatrix(r_operand);
          } else if (op_type == OMI_interval) {
            if (r_operand)
              FenceToInterval(r_operand);
            //TODO be more refined
            if (l_operand)
              FenceToInterval(l_operand);
          }
          CreatePrefixForm(new_node, l_operand, r_operand);
        } else if (op_ilk == OP_postfix) {
          // It remains to build a "reverse Polish" semantics node
          BUCKET_REC *arg1_bucket = MakeBucketRec(MB_UNNAMED, l_operand);
          new_node->bucket_list =
            AppendBucketRec(new_node->bucket_list, arg1_bucket);
          l_operand->parent = arg1_bucket;
          new_node->semantic_type = SEM_TYP_POSTFIX_OP;
        }
      }
      if (new_node) {
        head = new_node;
        tail = new_node;
        while (tail->next) {
          tail->parent = parent_bucket;
          tail = tail->next;
        }
        tail->parent = parent_bucket;
      }
    } else {
      // In this case, there are no operators in the source MML list.
      //  - probably a single node.
      int nodes_done = 0;
      new_node = SNodeFromMNodes(rover, nodes_done, isLHSofDef);
      // Advance thru the source list as required
      while (nodes_done) {
        mml_nodes_done++;
        rover = rover->next;
        nodes_done--;
      }
      // Append new_node to the list we're building
      if (new_node) {
        if (head) {
          tail->next = new_node;
          new_node->prev = tail;
        } else {
          head = new_node;
        }
        tail = new_node;
        while (tail->next) {
          tail->parent = parent_bucket;
          tail = tail->next;
        }
        tail->parent = parent_bucket;
      }
    }
  }
  return head;
}

SEMANTICS_NODE *Analyzer::SNodeFromMNodes(MNODE * mml_node,
                                          int& mml_nodes_done,
                                          bool isLHSofDef)
{
  SEMANTICS_NODE *rv = NULL;

  if (mml_node) {
    int local_nodes_done = 1;
    rv = CreateSemanticsNode();

    const char *mml_element = mml_node->src_tok;
    size_t ln = strlen(mml_element);
    switch (ln) {
    case 2:{
        if (!strcmp(mml_element, "mi")) {
          AnalyzeMI(mml_node, rv, local_nodes_done, isLHSofDef);
        } else if (!strcmp(mml_element, "mo")) {
          AnalyzeMO(mml_node, rv, local_nodes_done);
        } else if (!strcmp(mml_element, "mn")) {
          bool do_mixed = false;
          if (IsWholeNumber(mml_node)
              && IsWholeFrac(mml_node->next)) {
            do_mixed = !IsPositionalChild(mml_node);
          }
          if (do_mixed) {
            AnalyzeMixedNum(mml_node, rv);
            local_nodes_done++;
          } else
            AnalyzeMN(mml_node, rv);
        } else {
          TCI_ASSERT(0);
        }
      }
      break;

    case 3:{
        if (!strcmp(mml_element, "mtd")) {
          TCI_ASSERT(0);
        } else if (!strcmp(mml_element, "mtr")) {
          TCI_ASSERT(0);
        } else {
          TCI_ASSERT(0);
        }
      }
      break;

    case 4:{
        if (!strcmp(mml_element, "mrow")) {
          rv->semantic_type = SEM_TYP_PRECEDENCE_GROUP;
          if (mml_node->first_kid) {
            BUCKET_REC *new_a_rec = MakeBucketRec(MB_UNNAMED, NULL);
            rv->bucket_list = AppendBucketRec(NULL, new_a_rec);
            SEMANTICS_NODE *s_node = GetSemanticsList(mml_node->first_kid, new_a_rec);
            new_a_rec->first_child = s_node;
            s_node->parent = new_a_rec;
          }
        } else if (!strcmp(mml_element, "msup")) {
          AnalyzeMSUP(mml_node, rv, local_nodes_done, isLHSofDef);
        } else if (!strcmp(mml_element, "msub")) {
          AnalyzeMSUB(mml_node, rv, local_nodes_done, isLHSofDef);
        } else if (!strcmp(mml_element, "math")) {
          rv->semantic_type = SEM_TYP_MATH_CONTAINER;
          if (mml_node->first_kid) {
            MNODE *cont = mml_node->first_kid;
            // descend into a redundant mrow, if it exists
            while (cont && !cont->next && !strcmp(cont->src_tok, "mrow"))
              cont = cont->first_kid;
            if (cont) {
              BUCKET_REC *new_a_rec = MakeBucketRec(MB_UNNAMED, NULL);
              rv->bucket_list = AppendBucketRec(NULL, new_a_rec);
              SEMANTICS_NODE *s_node =
                GetSemanticsList(cont, new_a_rec, ALL_NODES, isLHSofDef);
              new_a_rec->first_child = s_node;
              s_node->parent = new_a_rec;
            }
          }
        } else {
          TCI_ASSERT(0);
        }
      }
      break;

    // integral<uID4.0>tciint(%integrand%,%variable%,%lowerlim%,%upperlim%),
    case 5:{
        if (!strcmp(mml_element, "mfrac")) {
          AnalyzeMFRAC(mml_node, rv, local_nodes_done);
        } else if (!strcmp(mml_element, "msqrt")) {
          AnalyzeMSQRT(mml_node, rv, local_nodes_done);
        } else if (!strcmp(mml_element, "mroot")) {
          AnalyzeMROOT(mml_node, rv, local_nodes_done);
        } else if (!strcmp(mml_element, "mover")) {
          AnalyzeMOVER(mml_node, rv, local_nodes_done, isLHSofDef);
        } else if (!strcmp(mml_element, "mtext")) {
          AnalyzeMTEXT(mml_node, rv, local_nodes_done);
        } else {
          TCI_ASSERT(0);
        }
      }
      break;

    case 6:{
        if (!strcmp(mml_element, "munder")) {
          AnalyzeMUNDER(mml_node, rv, local_nodes_done, isLHSofDef);
        } else if (!strcmp(mml_element, "mstyle")) {
          TCI_ASSERT(0);
        } else if (!strcmp(mml_element, "mtable")) {
          AnalyzeMTABLE(mml_node, rv, local_nodes_done);
        } else if (!strcmp(mml_element, "mspace")) {
        } else {
          TCI_ASSERT(0);
        }
      }
      break;

    case 7:{
        if (!strcmp(mml_element, "mfenced")) {
          AnalyzeMFENCED(mml_node, rv, local_nodes_done);
        } else if (!strcmp(mml_element, "msubsup")) {
          AnalyzeMSUBSUP(mml_node, rv, local_nodes_done, isLHSofDef);
        } else if (!strcmp(mml_element, "mpadded")) {
          rv->semantic_type = SEM_TYP_PRECEDENCE_GROUP;
          if (mml_node->first_kid) {
            BUCKET_REC *new_a_rec = MakeBucketRec(MB_UNNAMED, NULL);
            rv->bucket_list = AppendBucketRec(NULL, new_a_rec);

            SEMANTICS_NODE *s_node = GetSemanticsList(mml_node->first_kid, new_a_rec);
            new_a_rec->first_child = s_node;
            s_node->parent = new_a_rec;
          }
        } else {
          TCI_ASSERT(0);
        }
      }
      break;

    case 10:{
        if (!strcmp(mml_element, "munderover")) {
          AnalyzeMUNDEROVER(mml_node, rv, local_nodes_done, isLHSofDef);
        } else {
          TCI_ASSERT(0);
        }
      }
      break;

    case 11:{
        if (!strcmp(mml_element, "maligngroup")) {
        } else {
          TCI_ASSERT(0);
        }
      }
      break;

    default:
      TCI_ASSERT(0);
      break;
    }

    mml_nodes_done = local_nodes_done;

    if (!rv->semantic_type) {
      DisposeSemanticsNode(rv);
      rv = NULL;
    }
  }

  return rv;
}

LOG_MSG_REC *Analyzer::GetMsgs()
{
  LOG_MSG_REC *rv = msg_list;
  msg_list = NULL;
  return rv;
}

// WARNING: ownership of a list created in this object
//  is passed to another object here!

MIC2MMLNODE_REC *Analyzer::GetBackMap()
{
  MIC2MMLNODE_REC *rv = node_IDs_list;
  node_IDs_list = NULL;
  return rv;
}

SEMANTICS_NODE *Analyzer::GetSemanticsFromNode(MNODE * mml_node,
                                               BUCKET_REC * bucket)
{
  SEMANTICS_NODE *rv = NULL;
  if (mml_node) {
    if (!strcmp(mml_node->src_tok, "mrow")) {
      if (mml_node->first_kid)
        rv = GetSemanticsList(mml_node->first_kid, bucket);
      else
        TCI_ASSERT(0);
    } else
      rv = GetSemanticsList(mml_node, bucket, 1, false);
  }

  return rv;
}

void Analyzer::AnalyzeMixedNum(MNODE * mml_mn, SEMANTICS_NODE * s_node)
{
  BUCKET_REC *whole_bucket = MakeBucketRec(MB_MN_WHOLE, NULL);
  s_node->bucket_list = AppendBucketRec(s_node->bucket_list, whole_bucket);

  MNODE *save = mml_mn->next;
  mml_mn->next = NULL;
  SEMANTICS_NODE *s_whole = GetSemanticsList(mml_mn, whole_bucket, 1, false);
  mml_mn->next = save;

  whole_bucket->first_child = s_whole;
  s_whole->parent = whole_bucket;

  BUCKET_REC *frac_bucket = MakeBucketRec(MB_MN_FRACTION, NULL);
  s_node->bucket_list = AppendBucketRec(s_node->bucket_list, frac_bucket);
  SEMANTICS_NODE *s_frac =
    GetSemanticsList(mml_mn->next, frac_bucket, 1, false);
  frac_bucket->first_child = s_frac;
  s_frac->parent = frac_bucket;

  s_node->semantic_type = SEM_TYP_MIXEDNUMBER;
}

////////////////////////////// START SCRIPT HANDLING


Analyzer::BaseType Analyzer::GetBaseType(MNODE * mml_script_schemata, bool isLHSofDef)
{
  BaseType rv = BT_UNKNOWN;

  MNODE *base = mml_script_schemata->first_kid;
  const char *base_element = base->src_tok;

  if (!strcmp(base_element, "mi")) {
    if (!strcmp(base->p_chdata, "&#x2112;"))  // Laplace
      rv = BT_TRANSFORM;
    if (!strcmp(base->p_chdata, "&#x2131;"))  // Fourier
      rv = BT_TRANSFORM;
    if (!strcmp(base->p_chdata, "seq")) // sequence
      rv = BT_OPERATOR;
    if (!strcmp(base->p_chdata, "lim")) // varinjlim
      rv = BT_OPERATOR;

    if (!rv && !isLHSofDef) {
      char *mi_canonical_str = GetCanonicalIDforMathNode(base);
      if (defstore && mi_canonical_str) {
        DefInfo *di = defstore->GetDefInfo(curr_engine_ID, mi_canonical_str);
        if (di && di->def_type == DT_FUNCTION) {
          if (di->n_subscripted_args)
            rv = BT_SUBARG_FUNCTION;
          else
            rv = BT_FUNCTION;
        }
        delete[] mi_canonical_str;
      } else {
        TCI_ASSERT(0);
      }
    }
  }
  if (!rv && mml_script_schemata->next) {
    MNODE *nn = mml_script_schemata->next;
    const char *next_elem_nom = nn->src_tok;
    if (!strcmp(next_elem_nom, "mo")) {
      char *ptr = strstr(nn->p_chdata, "&#x");
      if (ptr) {
        U32 unicode = ASCII2U32(ptr + 3, 16);
        if (unicode == 0x2061) {  //&ApplyFunction;
          rv = BT_FUNCTION;
        }
      }
    }
  }
  if (!rv) {
    if (!strcmp(base_element, "mo")) {
      rv = BT_OPERATOR;
    } else if (!strcmp(base_element, "mn")) {
      rv = BT_NUMBER;
    } else if (!strcmp(base_element, "mfenced")) {
      rv = BT_FENCED;
    } else if (!strcmp(base_element, "mtable")) {
      rv = BT_MATRIX;
    } else if (!strcmp(base_element, "mover")) {
      rv = BT_MOVER;
    } else if (!strcmp(base_element, "mi")) {
      char zclass[256];
      zclass[0] = 0;
      GetCurrAttribValue(base, false, "class", zclass, 256);
      if (!strcmp(zclass, "msi_unit")) {
        rv = BT_UNIT;
      } else {
        int entity_count;
        int symbol_count = CountSymbols(base->p_chdata, entity_count);
        if (symbol_count == 1) {
          rv = BT_VARIABLE;
          if (LocateFuncRec(DE_func_names, NULL, base->p_chdata))
            rv = BT_FUNCTION;
          else if (LocateFuncRec(IMPLDIFF_func_names, NULL, base->p_chdata))
            rv = BT_FUNCTION;
        } else
          rv = BT_FUNCTION;
      }
    } else if (!strcmp(base_element, "mrow")) {
      rv = BT_ROW;
    } else {
      TCI_ASSERT(0);
    }
  }
  return rv;
}

// define ET_POWER                 1
// ..
// define ET_CONJUGATE_INDICATOR   5

Analyzer::ExpType Analyzer::GetExpType(BaseType base_type, MNODE * exp)
{
  ExpType rv = ET_POWER;

  const char *exp_element = exp->src_tok;
  const char *exp_contents = exp->p_chdata;
  if (!strcmp(exp_element, "mrow")) {
    exp_element = exp->first_kid->src_tok;
    exp_contents = exp->first_kid->p_chdata;
  }

  if (!strcmp(exp_element, "mi")) {
    if (base_type == BT_MATRIX) {
      if (!strcmp(exp_contents, "T"))
        rv = ET_TRANSPOSE_INDICATOR;
      else if (!strcmp(exp_contents, "H"))
        rv = ET_HTRANSPOSE_INDICATOR;
    }
  } else if (!strcmp(exp_element, "mn")) {
  } else if (!strcmp(exp_element, "mo")) {
    if (exp_contents) {
      int base = 16;
      int off = 3;
      char *ptr = strstr(exp_contents, "&#x");
      if (!ptr) {
        ptr = strstr(exp_contents, "&#");
        if (ptr) {
          base = 10;
          off = 2;
        }
      }
      if (ptr) {
        U32 unicode = ASCII2U32(ptr + off, base);
        if (unicode == 0x2217   // &midast;
            || unicode == 0x2a) // &ast;
          rv = ET_CONJUGATE_INDICATOR;
        else if (unicode == 0x2032) // &prime;
          rv = ET_PRIMES;
        else if (unicode == 0x2212) // &minus;
          rv = ET_DIRECTION;
        else
          TCI_ASSERT(!"Operator not in list.");
      } else if (exp_contents[0] == '*' && exp_contents[1] == 0) {
        rv = ET_CONJUGATE_INDICATOR;
      } else if (exp_contents[0] == '+' && exp_contents[1] == 0) {
        rv = ET_DIRECTION;
      } else if (exp_contents[0] == '-' && exp_contents[1] == 0) {
        rv = ET_DIRECTION;
      } else {
        TCI_ASSERT(0);
      }
    } else {
      TCI_ASSERT(!"operator with no contents????");
    }
  } else if (!strcmp(exp_element, "mfenced")) {
    bool inherit = false;

    char zopen_attr_val[32];
    zopen_attr_val[0] = 0;
    GetCurrAttribValue(exp, inherit, "open", zopen_attr_val, 32);

    char zclose_attr_val[32];
    zclose_attr_val[0] = 0;
    GetCurrAttribValue(exp, inherit, "close", zclose_attr_val, 32);

    if (zopen_attr_val[0] == '(' && zclose_attr_val[0] == ')') {
      if (base_type == BT_FUNCTION || base_type == BT_SUBARG_FUNCTION)
        rv = ET_PARENED_PRIMES_COUNT;
    }
  }
  if (rv == ET_DIRECTION && exp->first_kid && exp->first_kid->next)
    rv = ET_POWER;  // -x or +x
  if (IsInverseIndicator(exp))
    if (base_type == BT_FUNCTION
        || base_type == BT_SUBARG_FUNCTION || base_type == BT_TRANSFORM)
      rv = ET_INVERSE_INDICATOR;

  return rv;
}

//SLS seems like a risky heuristic...
Analyzer::ExpType Analyzer::GetSubScriptType(MNODE * script_schemata,
                               BaseType base_type, MNODE * sub)
{
  ExpType rv = ET_DECORATION;

  const char *sub_element = sub->src_tok;
  if (!strcmp(sub_element, "mn")) {
    rv = ET_NUMBER;
  } else if (!strcmp(sub_element, "mrow")) {
    rv = ET_EXPRESSION;
  } else if (!strcmp(sub_element, "msub")) {
    rv = ET_EXPRESSION;
  } else if (!strcmp(sub_element, "msup")) {
    rv = ET_EXPRESSION;
  } else if (!strcmp(sub_element, "mi")) {
    rv = ET_VARIABLE;

    if (base_type == BT_FUNCTION && script_schemata && script_schemata->next) {
      if (IsApplyFunction(script_schemata->next))
        rv = ET_DECORATION;
    }
  } else {
    TCI_ASSERT(0);
  }
  return rv;
}

SemanticType Analyzer::GetBigOpType(const char *op_chdata, SemanticVariant & n_integs)
{
  SemanticType rv = SEM_TYP_UNDEFINED;
  n_integs = SNV_None;

  if (op_chdata) {
    char *ptr = strstr(op_chdata, "&#x");
    if (ptr) {
      U32 unicode = ASCII2U32(ptr + 3, 16);
      switch (unicode) {
      case 0x222B:             //&int;<uID7.1.1>prefix,31,U0222B
        n_integs = SNV_singleint;
        rv = SEM_TYP_BIGOP_INTEGRAL;
        break;
      case 0x222C:             //&Int;<uID7.1.2>prefix,31,U0222C
        n_integs = SNV_doubleint;
        rv = SEM_TYP_BIGOP_INTEGRAL;
        break;
      case 0x222D:             //&tint;<uID7.1.3>prefix,31,U0222D
        n_integs = SNV_tripleint;
        rv = SEM_TYP_BIGOP_INTEGRAL;
        break;
      case 0x2A0C:             //&qint;<uID7.1.4>prefix,31,U02A0C
        n_integs = SNV_quadint;
        rv = SEM_TYP_BIGOP_INTEGRAL;
        break;
      //      case  0x2217  :     //&int;&ctdot;&int;<uID7.1.5>prefix,31,U0222B'U022EF'U0222B
      case 0x222E:             //&conint;<uID7.1.6>prefix,31,U0222E
        rv = SEM_TYP_BIGOP_INTEGRAL;
        break;
      case 0x2211:             //&sum;<uID7.1.7>prefix,29,    U02211
      case 0x220F:             //&prod;<uID7.1.8>prefix,35,   U0220F
      case 0x22C2:             //&xcap;<uID7.1.9>prefix,35,   U022C2
      case 0x22C0:             //&xwedge;<uID7.1.10>prefix,35,U022C0
      case 0x2295:             //&xoplus;<uID7.1.11>prefix,29,U02295
      case 0x2A00:             //&xodot;<uID7.1.12>prefix,38, U02A00
      case 0x2A06:             //&xsqcup;<uID7.1.13>prefix,29,U02A06
      case 0x2210:             //&coprod;<uID7.1.14>prefix,35,U02210
      case 0x22C3:             //&xcup;<uID7.1.15>prefix,29,  U022C3
      case 0x22C1:             //&xvee;<uID7.1.16>prefix,29,  U022C1
      case 0x2A02:             //&xotime;<uID7.1.17>prefix,35,U02A02
      case 0x2A04:             //&xuplus;<uID7.1.18>prefix,29,U02A04
        rv = SEM_TYP_BIGOP_SUM;
        break;
      case 0x2207:             //&nabla;
        break;
      default:
        break;
      }
    }
  }
  return rv;
}

bool Analyzer::IsLaplacian(MNODE * op_node)
{
  bool rv = false;

  if (op_node && op_node->p_chdata) {
    char *ptr = strstr(op_node->p_chdata, "&#x");
    if (ptr) {
      U32 unicode = ASCII2U32(ptr + 3, 16);
      if (unicode == 0x2207) {  // nabla
        MNODE *sup = op_node->next;
        if (sup && sup->p_chdata && !strcmp(sup->p_chdata, "2"))
          rv = true;
      }
    }
  }
  return rv;
}

// Just enough done to test the "true" return route.

bool Analyzer::IsInverseIndicator(MNODE * mml_exp_node)
{

  bool rv = false;
  /*
  <mml:msup>
    <mml:mi mathcolor="gray">sin</mml:mi>
    <mml:mrow>
      <mml:mo form="prefix">&minus;</mml:mo>   &minus;<uID3.13.61>multiform,111,U02212
      <mml:mn>1</mml:mn>
    </mml:mrow>
  </mml:msup>
  <mml:mo>&ApplyFunction;</mml:mo>
  <mml:mi>x</mml:mi>
  */
  if (!strcmp(mml_exp_node->src_tok, "mrow")) {
    if (mml_exp_node->first_kid) {
      MNODE *rover = mml_exp_node->first_kid;
      if (!strcmp(rover->src_tok, "mo")) {
        U32 unicodes[8];
        int content_tally = ChData2Unicodes(rover->p_chdata, unicodes, 8);
        if (content_tally == 1 &&
            (unicodes[0] == '-' || unicodes[0] == 0x2212)) {
          rover = rover->next;
          if (rover) {
            if (!strcmp(rover->src_tok, "mn")) {
              U32 unicodes[8];
              int content_tally =
                ChData2Unicodes(rover->p_chdata, unicodes, 8);
              if (content_tally == 1 && unicodes[0] == '1')
                rv = true;
            }
          }
        }
      }
    }
  }

  return rv;
}

// When the "base" of a MML schemata like <msup>, <mover>, etc.
//  is an operator, the translation is handled here.

void Analyzer::TranslateEmbellishedOp(MNODE * mml_embellop_node,
                                      SEMANTICS_NODE * snode,
                                      int& nodes_done)
{
  nodes_done = 1;

  MNODE *base = mml_embellop_node->first_kid;
  if (base) {                   // the underlying operator - \int, \sum, etc.
    size_t ln = strlen(base->p_chdata);
    char *tmp = TCI_NEW(char[ln + 1]);
    strcpy(tmp, base->p_chdata);
    snode->contents = tmp;

    // SEM_TYP_BIGOP_INTEGRAL, SEM_TYP_BIGOP_SUM, or something else
    SemanticVariant n_integs;
    SemanticType bigop_type = GetBigOpType(tmp, n_integs);

    if (bigop_type) {
      snode->semantic_type = bigop_type;
      if (base->next) {         // the lower limit

        MNODE *mml_ll = base->next;
        SEMANTICS_NODE *s_ll = GetSemanticsFromNode(mml_ll, NULL);

        bool done = false;
        if (bigop_type == SEM_TYP_BIGOP_SUM) {
          if (s_ll->semantic_type == SEM_TYP_INFIX_OP) {
            OpOrderIlk op_order = GetOpOrderIlk(s_ll);
            if (op_order == OOI_equal) {
              BUCKET_REC *b_rover = s_ll->bucket_list;
              if (b_rover && b_rover->next) {
                BUCKET_REC *b_expr = b_rover->next;
                b_rover->bucket_ID = MB_SUM_VAR;
                b_rover->next = NULL;
                snode->bucket_list =
                  AppendBucketRec(snode->bucket_list, b_rover);

                // Might have mapped "i" to "imaginaryi".
                if (b_rover->first_child) {
                  SEMANTICS_NODE *s_var = b_rover->first_child;
                  if (s_var->semantic_type == SEM_TYP_UCONSTANT) {
                    if (!strcmp(s_var->contents, "i")) {
                      s_var->semantic_type = SEM_TYP_VARIABLE;
                      i_is_imaginary = false;
                    } else if (!strcmp(s_var->contents, "j")) {
                      s_var->semantic_type = SEM_TYP_VARIABLE;
                      j_is_imaginary = false;
                    } else if (!strcmp(s_var->contents, "e")) {
                      s_var->semantic_type = SEM_TYP_VARIABLE;
                      e_is_Euler = false;
                    }
                  }
                }

                b_expr->bucket_ID = MB_LOWERLIMIT;
                TCI_ASSERT(b_expr->next == NULL);
                snode->bucket_list =
                  AppendBucketRec(snode->bucket_list, b_expr);
                s_ll->bucket_list = NULL;
                DisposeSList(s_ll);
                done = true;
              }
            } else {
              TCI_ASSERT(0);
            }
          } else {
            done = false;
          }
        }
        if (!done) {
          BUCKET_REC *ll_bucket = MakeBucketRec(MB_LOWERLIMIT, NULL);
          snode->bucket_list = AppendBucketRec(snode->bucket_list, ll_bucket);
          ll_bucket->first_child = s_ll;
          s_ll->parent = ll_bucket;
        }
        if (mml_ll->next) {     // the upper limit
          MNODE *mml_ul = mml_ll->next;
          BUCKET_REC *ul_bucket = MakeBucketRec(MB_UPPERLIMIT, NULL);
          snode->bucket_list = AppendBucketRec(snode->bucket_list, ul_bucket);
          SEMANTICS_NODE *s_ul = GetSemanticsFromNode(mml_ul, ul_bucket);
          ul_bucket->first_child = s_ul;
          s_ul->parent = ul_bucket;
        }
      } else {
        TCI_ASSERT(0);
      }

      int nodes_in_arg;
      OperandToBucketList(mml_embellop_node, bigop_type, snode, nodes_in_arg);
      nodes_done += nodes_in_arg;
    } else {                    // Here, bigoptype is 0 - not sum or int
      // look for varinjlim, varliminf, varprojlim
      int var_lim_type = GetVarLimType(tmp, base);
      if (var_lim_type) {
        int nodes_in_arg;
        BUCKET_REC *br = ArgsToBucket(mml_embellop_node, nodes_in_arg);
        nodes_done += nodes_in_arg;
        if (br)
          snode->bucket_list = AppendBucketRec(snode->bucket_list, br);

        snode->semantic_type = SEM_TYP_PREFIX_OP;
        delete[] snode->contents;

        char *op_name = NULL;
        char *tmp = TCI_NEW(char[8]);
        if (var_lim_type == 1)
          op_name = "injlim";
        else if (var_lim_type == 2)
          op_name = "liminf";
        else if (var_lim_type == 3)
          op_name = "projlim";
        else if (var_lim_type == 4)
          op_name = "limsup";

        if (op_name) {
          strcpy(tmp, op_name);
          snode->contents = tmp;
        } else {
          TCI_ASSERT(0);
        }
      } else if (IsLaplacian(base)) {
        int nodes_in_arg;
        BUCKET_REC *br = ArgsToBucket(mml_embellop_node, nodes_in_arg);
        nodes_done += nodes_in_arg;
        if (br)
          snode->bucket_list = AppendBucketRec(snode->bucket_list, br);

        snode->semantic_type = SEM_TYP_PREFIX_OP;

        delete[] snode->contents;
        char *tmp = TCI_NEW(char[12]);
        strcpy(tmp, "Laplacian");
        snode->contents = tmp;
      } else {                  // for now, I'm assuming a limfunc here!
        if (base->next) {
          MNODE *mml_ll = base->next;
          BUCKET_REC *ll_bucket = MakeBucketRec(MB_LOWERLIMIT, NULL);
          snode->bucket_list = AppendBucketRec(snode->bucket_list, ll_bucket);

          SEMANTICS_NODE *s_ll = GetSemanticsFromNode(mml_ll, ll_bucket);
          ll_bucket->first_child = s_ll;

          // The embellished operator MAY be a standard math operator
          //  that takes a limit.  If so, we can further decompose the limit.
          int req_limit_format = GetLimitFormat(snode->contents);
          switch (req_limit_format) {
          case 1:              // the limiting expression is an interval
            SetVarAndIntervalLimit(ll_bucket);
            break;
          case 2:
            SetVarArrowExprLimit(ll_bucket);
            break;
          default:
            break;
          }
        }

        int nodes_in_arg;
        BUCKET_REC *br = ArgsToBucket(mml_embellop_node, nodes_in_arg);
        nodes_done += nodes_in_arg;
        if (br)
          snode->bucket_list = AppendBucketRec(snode->bucket_list, br);
        snode->semantic_type = SEM_TYP_LIMFUNC;
      }
    }
  } else {
    TCI_ASSERT(0);
  }
}

/*
SEM_TYP_INTEGRAL
			MB_LOWERLIMIT	MB_UPPERLIMIT	MB_OPERAND	 MB_INTEG_VAR

         <mml:msubsup>
BIGOP
           <mml:mo form="prefix" largeop="true">&int;</mml:mo>
lower
           <mml:mn>1</mml:mn>
upper
           <mml:mrow>
             <mml:mi>a</mml:mi>
             <mml:mo form="infix">+</mml:mo>
             <mml:mn>1</mml:mn>
           </mml:mrow>
         </mml:msubsup>

operand
         <mml:mrow>                                <mfrac>
                                                     <mrow>
           <mml:whatever>                              <mo form="prefix">&#x2146;</mo>
           </mml:whatever>                             <mi>x</mi>
                                                     </mrow>
           <mml:mo>&InvisibleTimes;</mml:mo>         <mrow>

           <mml:mrow>                                  whatever
             <mml:mo form="prefix">&dd;</mml:mo>
variable
             <mml:mi>x</mml:mi>                      </mrow>
           </mml:mrow>                             </mfrac>

         </mml:mrow>
*/

void Analyzer::OperandToBucketList(MNODE * big_op_node, SemanticType bigop_type,
                                   SEMANTICS_NODE * bigop_snode,
                                   int& nodes_done)
{
  nodes_done = 0;

  int local_nodes_done = 0;
  if (big_op_node && big_op_node->next) {
    MNODE *mml_operand = big_op_node->next;

    // step over any whitespace
    while (IsWhiteSpace(mml_operand)) {
      local_nodes_done++;
      mml_operand = mml_operand->next;
    }

    if (mml_operand) {
      local_nodes_done++;
      if (bigop_type == SEM_TYP_BIGOP_SUM) {
        BUCKET_REC *a_rec = MakeBucketRec(MB_OPERAND, NULL);
        bigop_snode->bucket_list =
          AppendBucketRec(bigop_snode->bucket_list, a_rec);
        SEMANTICS_NODE *s_arg = GetSemanticsFromNode(mml_operand, a_rec);
        a_rec->first_child = s_arg;
        s_arg->parent = a_rec;
      } else {                  // It's an integral
        bool nested_operand = true;
        MNODE *integrand_ender = NULL;
        bool frac_operand = false;
        bool dx_is_nested = false;

        if (!strcmp(mml_operand->src_tok, "mfrac")) {
          nested_operand = true;
          frac_operand = true;

          MNODE *num = mml_operand->first_kid;
          if (num) {
            integrand_ender = Find_dx(num, dx_is_nested);
            TCI_ASSERT(integrand_ender);
          } else
            TCI_ASSERT(0);
        } else if (!strcmp(mml_operand->src_tok, "mrow")) {
          integrand_ender = Find_dx(mml_operand, dx_is_nested);
          // SWP accepts indefinite integrals with no "dx" - so we're OK here.
        } else if (!strcmp(mml_operand->src_tok, "mfenced")) {
          nested_operand = false;
          integrand_ender = mml_operand->next;
        } else {
          TCI_ASSERT(!"Integrand not nicely grouped.  What to do?");
        }
        if (integrand_ender) {
          // "dx" should be nested in an mrow of it's own
          if (!strcmp(integrand_ender->src_tok, "mrow")) {
            int tally = 0;
            MNODE *s_rover = integrand_ender;
            while (1) {
              MNODE *var_rover = s_rover->first_kid;
              var_rover = var_rover->next;  // step over "d"

              BUCKET_REC *v_bucket = MakeBucketRec(MB_INTEG_VAR, NULL);
              bigop_snode->bucket_list =
                AppendBucketRec(bigop_snode->bucket_list, v_bucket);
              SEMANTICS_NODE *s_arg =
                GetSemanticsFromNode(var_rover, v_bucket);
              v_bucket->first_child = s_arg;
              s_arg->parent = v_bucket;
              if (!nested_operand)
                local_nodes_done++;
              tally++;

              if (frac_operand && !dx_is_nested)
                break;
              if (s_rover->next) {  // dx*dy...
                s_rover = s_rover->next;
                if (!strcmp(s_rover->src_tok, "mo")) {
                  if (!nested_operand)
                    local_nodes_done++;
                  s_rover = s_rover->next;
                } else {
                  TCI_ASSERT(0);
                  break;
                }

                if (!strcmp(s_rover->src_tok, "mrow")) {
                  if (!nested_operand)
                    local_nodes_done++;
                } else {
                  TCI_ASSERT(0);
                  break;
                }
              } else
                break;
            }                   // loop thru "dx" * "dy" *..
          } else {
            TCI_ASSERT(0);
          }
          MNODE *integrand_starter = NULL;
          if (frac_operand) {
            integrand_starter = mml_operand;
            integrand_ender = mml_operand;
          } else {
            // back up to the integrand
            integrand_ender = integrand_ender->prev;

            if (!strcmp(integrand_ender->src_tok, "mo")) {
              char *ptr = strstr(integrand_ender->p_chdata, "&#x");
              if (ptr) {
                U32 unicode = ASCII2U32(ptr + 3, 16);
                TCI_ASSERT(unicode == 0x2062);
              }
              integrand_ender = integrand_ender->prev;
              if (!nested_operand)
                local_nodes_done++;
            }

            integrand_starter = integrand_ender;
            while (integrand_starter->prev) {
              integrand_starter = integrand_starter->prev;
              if (!nested_operand) {
                if (integrand_starter == big_op_node) {
                  integrand_starter = integrand_starter->next;
                  break;
                } else
                  local_nodes_done++;
              }
            }
          }

          // isolate the integrand
          MNODE *save = integrand_ender->next;
          integrand_ender->next = NULL;

          BUCKET_REC *a_rec = MakeBucketRec(MB_OPERAND, NULL);
          bigop_snode->bucket_list =
            AppendBucketRec(bigop_snode->bucket_list, a_rec);
          SEMANTICS_NODE *contents = GetSemanticsList(integrand_starter, a_rec);
          a_rec->first_child = contents;
          contents->parent = a_rec;

          integrand_ender->next = save;
          if (frac_operand)
            Patchdx(contents);
        } else {
          // Here, we don't have an integand ender - there's no "dx"
          // We still generate the operand - engine will do something without MB_INTEG_VAR
          MNODE *integrand_starter = mml_operand;

          BUCKET_REC *a_rec = MakeBucketRec(MB_OPERAND, NULL);
          bigop_snode->bucket_list =
            AppendBucketRec(bigop_snode->bucket_list, a_rec);
          SEMANTICS_NODE *contents = GetSemanticsList(integrand_starter, a_rec);
          a_rec->first_child = contents;
          contents->parent = a_rec;
        }
      }
    } else {
      TCI_ASSERT(0);
    }
  }

  nodes_done = local_nodes_done;
}

// whatever'''

void Analyzer::AnalyzePrimed(MNODE * mml_msup,
                             SEMANTICS_NODE * s_node, int& nodes_done)
{
  nodes_done = 1;

  s_node->semantic_type = SEM_TYP_DERIVATIVE;

  char *key = "differentiate";
  size_t zln = strlen(key);
  char *tmp = TCI_NEW(char[zln + 1]);
  strcpy(tmp, key);
  s_node->contents = tmp;

  MNODE *base = mml_msup->first_kid;
  MNODE *primes = base->next;

  BUCKET_REC *base_bucket = MakeBucketRec(MB_UNNAMED, NULL);
  s_node->bucket_list = AppendBucketRec(s_node->bucket_list, base_bucket);

  SEMANTICS_NODE *s_base = GetSemanticsFromNode(base, base_bucket);

  base_bucket->first_child = s_base;
  s_base->parent = base_bucket;

  if (s_base->semantic_type == SEM_TYP_FUNCTION
      || s_base->semantic_type == SEM_TYP_VARIABLE) {
    s_base->semantic_type = SEM_TYP_FUNCTION;
    // see if an argument bucket has been generated
    BUCKET_REC *bucket = FindBucketRec(s_base->bucket_list, MB_UNNAMED);
    // If arguments exist, process them
    int local_nodes_done;
    BUCKET_REC *br = ArgsToBucket(mml_msup, local_nodes_done);
    nodes_done += local_nodes_done;

    if (br) {
      if (bucket)
        RemoveBucket(s_base, bucket);
      s_base->bucket_list = AppendBucketRec(s_base->bucket_list, br);
    } else if (DE_ind_vars && (!bucket || !bucket->first_child)) {
      if (bucket)
        RemoveBucket(s_base, bucket);
      BUCKET_REC *fvar_bucket = AddVarToBucket(MB_UNNAMED, DE_ind_vars);
      s_base->bucket_list = AppendBucketRec(s_base->bucket_list, fvar_bucket);
    } else {
      DefInfo *di =
        defstore->GetDefInfo(curr_engine_ID, s_base->canonical_ID);
      if (di && di->arg_list) {
        char user_name[128];
        char canonical_ID[128];
        char *ptr = strchr(di->arg_list, ',');
        if (ptr) {              // 
          size_t zln = ptr - di->arg_list;
          strncpy(user_name, di->arg_list, zln);
          user_name[zln] = 0;

          ptr++;
          char *ptr1 = strchr(ptr, ',');
          if (ptr1) {           // more than 1 variable??
            TCI_ASSERT(0);
            size_t zln = ptr - ptr1;
            strncpy(canonical_ID, ptr, zln);
            canonical_ID[zln] = 0;
          } else
            strcpy(canonical_ID, ptr);

          SEMANTICS_NODE *s_var = CreateSemanticsNode();
          s_var->semantic_type = SEM_TYP_VARIABLE;

          if (canonical_ID) {
            size_t ln = strlen(canonical_ID);
            char *tmp = TCI_NEW(char[ln + 1]);
            strcpy(tmp, canonical_ID);
            s_var->canonical_ID = tmp;
            SetSnodeOwner(s_var);
          }

          size_t ln = strlen(user_name);
          char *tmp = TCI_NEW(char[ln + 1]);
          strcpy(tmp, user_name);
          s_var->contents = tmp;

          BUCKET_REC *dvar_bucket = AddVarToBucket(MB_DIFF_VAR, s_var);
          s_node->bucket_list =
            AppendBucketRec(s_node->bucket_list, dvar_bucket);

          if (bucket)
            RemoveBucket(s_base, bucket);
          BUCKET_REC *fvar_bucket = AddVarToBucket(MB_UNNAMED, s_var);
          s_base->bucket_list =
            AppendBucketRec(s_base->bucket_list, fvar_bucket);

          DisposeSList(s_var);
        } else
          TCI_ASSERT(0);
      }
    }
  } else {
    if (DE_ind_vars) {
      BUCKET_REC *dvar_bucket = AddVarToBucket(MB_DIFF_VAR, DE_ind_vars);
      s_node->bucket_list = AppendBucketRec(s_node->bucket_list, dvar_bucket);
    } else
      TCI_ASSERT(0);
  }

  AddPrimesCount(s_node, primes);
}

void Analyzer::AnalyzeDotDerivative(MNODE * mml_mover,
                                    int n_dots,
                                    SEMANTICS_NODE * s_node, int & nodes_done)
{
  nodes_done = 1;

  s_node->semantic_type = SEM_TYP_DERIVATIVE;

  char *key = "differentiate";
  size_t zln = strlen(key);
  char *tmp = TCI_NEW(char[zln + 1]);
  strcpy(tmp, key);
  s_node->contents = tmp;

  MNODE *base = mml_mover->first_kid;
  MNODE *dots = base->next;

  BUCKET_REC *base_bucket = MakeBucketRec(MB_UNNAMED, NULL);
  s_node->bucket_list = AppendBucketRec(s_node->bucket_list, base_bucket);

  SEMANTICS_NODE *s_base = GetSemanticsFromNode(base, base_bucket);

  base_bucket->first_child = s_base;
  s_base->parent = base_bucket;

  if (s_base->semantic_type == SEM_TYP_FUNCTION
      || s_base->semantic_type == SEM_TYP_VARIABLE) {
    s_base->semantic_type = SEM_TYP_FUNCTION;

    // see if an argument bucket has been generated
    BUCKET_REC *bucket = FindBucketRec(s_base->bucket_list, MB_UNNAMED);

    // If arguments exist, process them
    int local_nodes_done;
    BUCKET_REC *br = ArgsToBucket(mml_mover, local_nodes_done);
    nodes_done += local_nodes_done;

    if (br) {
      if (bucket)
        RemoveBucket(s_base, bucket);
      s_base->bucket_list = AppendBucketRec(s_base->bucket_list, br);
    } else if (DE_ind_vars && (!bucket || !bucket->first_child)) {
      if (bucket)
        RemoveBucket(s_base, bucket);
      BUCKET_REC *fvar_bucket = AddVarToBucket(MB_UNNAMED, DE_ind_vars);
      s_base->bucket_list = AppendBucketRec(s_base->bucket_list, fvar_bucket);
    } else {
      DefInfo *di =
        defstore->GetDefInfo(curr_engine_ID, s_base->canonical_ID);
      if (di && di->arg_list) {
        char user_name[128];
        char canonical_ID[128];
        char *ptr = strchr(di->arg_list, ',');
        if (ptr) {              // 
          size_t zln = ptr - di->arg_list;
          strncpy(user_name, di->arg_list, zln);
          user_name[zln] = 0;

          ptr++;
          char *ptr1 = strchr(ptr, ',');
          if (ptr1) {           // more than 1 variable??
            TCI_ASSERT(0);
            size_t zln = ptr - ptr1;
            strncpy(canonical_ID, ptr, zln);
            canonical_ID[zln] = 0;
          } else
            strcpy(canonical_ID, ptr);

          SEMANTICS_NODE *s_var = CreateSemanticsNode();
          s_var->semantic_type = SEM_TYP_VARIABLE;

          if (canonical_ID) {
            size_t ln = strlen(canonical_ID);
            char *tmp = TCI_NEW(char[ln + 1]);
            strcpy(tmp, canonical_ID);
            s_var->canonical_ID = tmp;
            SetSnodeOwner(s_var);
          }

          size_t ln = strlen(user_name);
          char *tmp = TCI_NEW(char[ln + 1]);
          strcpy(tmp, user_name);
          s_var->contents = tmp;

          BUCKET_REC *dvar_bucket = AddVarToBucket(MB_DIFF_VAR, s_var);
          s_node->bucket_list =
            AppendBucketRec(s_node->bucket_list, dvar_bucket);

          if (bucket)
            RemoveBucket(s_base, bucket);
          BUCKET_REC *fvar_bucket = AddVarToBucket(MB_UNNAMED, s_var);
          s_base->bucket_list =
            AppendBucketRec(s_base->bucket_list, fvar_bucket);

          DisposeSList(s_var);
        } else
          TCI_ASSERT(0);
      }
    }
  } else {
    if (DE_ind_vars) {
      BUCKET_REC *dvar_bucket = AddVarToBucket(MB_DIFF_VAR, DE_ind_vars);
      s_node->bucket_list = AppendBucketRec(s_node->bucket_list, dvar_bucket);
    } else
      TCI_ASSERT(0);
  }

  AppendNumber(s_node, MB_NPRIMES, n_dots);
}

// Here the subscript is NOT treated as an argument

void Analyzer::AnalyzeSubscriptedFunc(MNODE * mml_msub_node,
                                      SEMANTICS_NODE * snode,
                                      int& nodes_done)
{
  nodes_done = 1;

  BUCKET_REC *bucket = FindBucketRec(snode->bucket_list, MB_UNNAMED);

  int local_nodes_done;
  BUCKET_REC *br = ArgsToBucket(mml_msub_node, local_nodes_done);
  nodes_done += local_nodes_done;

  if (br) {
    if (bucket)
      RemoveBucket(snode, bucket);
    snode->bucket_list = AppendBucketRec(snode->bucket_list, br);
  } else if (DE_ind_vars && (!bucket || !bucket->first_child)) {
    if (bucket)
      RemoveBucket(snode, bucket);
    BUCKET_REC *fvar_bucket = AddVarToBucket(MB_UNNAMED, DE_ind_vars);
    snode->bucket_list = AppendBucketRec(snode->bucket_list, fvar_bucket);
  }

  MNODE *base = mml_msub_node->first_kid;
  size_t ln = strlen(base->p_chdata);
  char *tmp = TCI_NEW(char[ln + 1]);
  strcpy(tmp, base->p_chdata);
  snode->contents = tmp;
  snode->semantic_type = SEM_TYP_FUNCTION;

  if (base->p_chdata && !strcmp(base->p_chdata, "log")) {
    if (base->next) {
      BUCKET_REC *bucket = MakeBucketRec(MB_LOG_BASE, NULL);
      snode->bucket_list = AppendBucketRec(snode->bucket_list, bucket);
      SEMANTICS_NODE *log_base = GetSemanticsFromNode(base->next, bucket);
      bucket->first_child = log_base;
      log_base->parent = bucket;
    } else
      TCI_ASSERT(0);
  }
}

void Analyzer::AnalyzeSubscriptedArgFunc(MNODE * mml_msub_node,
                                         SEMANTICS_NODE * snode)
{
  MNODE *f_nom = mml_msub_node->first_kid;
  MNODE *f_arg = f_nom->next;

  // Handle the function argument
  BUCKET_REC *arg_bucket = NULL;
  if (!strcmp(f_arg->src_tok, "mrow")) {
    arg_bucket = ArgBucketFromMROW(f_arg);
  } else {
    arg_bucket = MakeBucketRec(MB_UNNAMED, NULL);
    SEMANTICS_NODE *s_arg = GetSemanticsFromNode(f_arg, arg_bucket);
    arg_bucket->first_child = s_arg;
  }

  if (arg_bucket) {
    BUCKET_REC *bucket = FindBucketRec(snode->bucket_list, MB_UNNAMED);
    if (bucket)
      RemoveBucket(snode, bucket);
    snode->bucket_list = AppendBucketRec(snode->bucket_list, arg_bucket);
  } else
    TCI_ASSERT(0);

  size_t ln = strlen(f_nom->p_chdata);
  char *tmp = TCI_NEW(char[ln + 1]);
  strcpy(tmp, f_nom->p_chdata);
  snode->contents = tmp;

  snode->semantic_type = SEM_TYP_FUNCTION;
}

void Analyzer::AnalyzeSubscriptedFence(MNODE * mml_msub_node,
                                       SEMANTICS_NODE * snode,
                                       int& nodes_done)
{
  nodes_done = 1;

  MNODE *base = mml_msub_node->first_kid;
  if (base && !strcmp(base->src_tok, "mfenced")) {
    const char *open_value = GetATTRIBvalue(base->attrib_list, "open");
    const char *close_value = GetATTRIBvalue(base->attrib_list, "close");
    bool is_Vert = !strcmp(open_value, "&#x2016;") && !strcmp(close_value, "&#x2016;");
    bool is_abs = !strcmp(open_value, "|") && !strcmp(close_value, "|");
    
    if (is_Vert || is_abs) {
      // Handle ||...||_{norm} here
      // (note we don't have a different representation for |...|_{norm}
      int local_nodes_done;
      AnalyzeMFENCED(base, snode, local_nodes_done);

      if (snode->bucket_list) {
        BUCKET_REC *bucket = FindBucketRec(snode->bucket_list, MB_UNNAMED);
        if (bucket && bucket->first_child)
          FenceToMatrix(bucket->first_child);
      }
      if (base->next) {
        BUCKET_REC *bucket = MakeBucketRec(MB_NORM_NUMBER, NULL);
        snode->bucket_list = AppendBucketRec(snode->bucket_list, bucket);
        SEMANTICS_NODE *norm_num = GetSemanticsFromNode(base->next, bucket);
        bucket->first_child = norm_num;
        norm_num->parent = bucket;
      } else {
        TCI_ASSERT(!"The subscript is missing? - bad MathML");
      }
    } else {
      TCI_ASSERT(!"Unknown fence type.");
    }
  }
}

////////////////////////////// END SCRIPT HANDLING

//////////////////// START UTILITIES

bool Analyzer::IsWhiteText(const char *z_text)
{
  bool rv = true;

  if (z_text && *z_text) {
    const char *ptr = z_text;
    char ch;
    while (rv && (ch = *ptr)) {
      if (ch == '&') {
        if (*(ptr + 1) == '#' && *(ptr + 2) == 'x') {
          U32 unicode = ASCII2U32(ptr + 3, 16);
          if (unicode != 0x200B)
            rv = false;
          // need more here!
        } else {
          TCI_ASSERT(0);
        }
        while (*ptr != ';')
          ptr++;
      } else if (ch > ' ' && ch <= '~') {
        rv = false;
      }
      ptr++;
    }                           // loop thru chars
  }

  return rv;
}

void Analyzer::SemanticAttribs2Buffer(char *buffer, MNODE * mml_node, int lim)
{
  GetCurrAttribValue(mml_node, true, "mathvariant", buffer, lim);
  // May need to add more calls here
}

void Analyzer::GetCurrAttribValue(MNODE * mml_node, bool inherit,
                                  char *targ_attr_name, char *buffer, int lim)
{
  // Check the current node for the target attribute
  const char *attr_val = GetATTRIBvalue(mml_node->attrib_list, targ_attr_name);

  // Ascend the parent tree if necessary
  if (!attr_val && inherit) {
    MNODE *rover = mml_node->parent;
    while (rover && !attr_val) {
      if (!strcmp(rover->src_tok, "mstyle")) {
        attr_val = GetATTRIBvalue(rover->attrib_list, targ_attr_name);
      }
      rover = rover->parent;
    }
  }

  if (attr_val) {
    size_t curr_ln = strlen(buffer);
    size_t inc_ln = strlen(attr_val);
    if (curr_ln + inc_ln < lim)
      strcat(buffer, attr_val);
    else
      TCI_ASSERT(0);
  }
}

void Analyzer::Contents2Buffer(char *zdest, const char *p_chdata, int lim)
{
  if (p_chdata) {
    U32 unicodes[128];
    int n_chars = ChData2Unicodes(p_chdata, unicodes, 128);
    if (n_chars < 128) {
      size_t i = strlen(zdest);
      int j = 0;
      while (i < lim && j < n_chars) {
        U32 ch = unicodes[j];
        if (ch >= ' ' && ch <= '~') {
          zdest[i] = ch;
          i++;
        } else {
          sprintf(zdest + i, "&#x%x;", ch);
          i = strlen(zdest);
        }
        j++;
      }
      zdest[i] = 0;
    } else {
      TCI_ASSERT(0);
    }
  }
}

bool Analyzer::IsWholeNumber(MNODE * mml_mn)
{
  bool rv = true;
  if (mml_mn && mml_mn->p_chdata) {
    const char *ptr = mml_mn->p_chdata;
    while (*ptr) {
      if (*ptr < '0' || *ptr > '9') {
        rv = false;
        break;
      }
      ptr++;
    }
  } else
    TCI_ASSERT(0);
  return rv;
}

bool Analyzer::IsOperator(MNODE * mml_node)
{
  bool rv = false;
  if (mml_node && !strcmp(mml_node->src_tok, "mo")) {
    rv = true;
  }

  return rv;
}

bool Analyzer::IsWholeFrac(MNODE * mml_frac)
{
  bool num_OK = false;
  bool den_OK = false;

  if (mml_frac && !strcmp(mml_frac->src_tok, "mfrac")) {
    MNODE *rover = mml_frac->first_kid;
    if (rover) {
      if (rover->src_tok && !strcmp(rover->src_tok, "mn")
          && IsWholeNumber(rover))
        num_OK = true;
      rover = rover->next;
      if (rover) {
        if (rover->src_tok && !strcmp(rover->src_tok, "mn")
            && IsWholeNumber(rover))
          den_OK = true;
      }
    }
  }
  return num_OK && den_OK;
}

bool Analyzer::IsUnitsFraction(MNODE * mml_frac)
{
  bool num_OK = false;
  bool den_OK = false;

  if (mml_frac && !strcmp(mml_frac->src_tok, "mfrac")) {
    MNODE *rover = mml_frac->first_kid;
    if (rover) {
      if (!strcmp(rover->src_tok, "mi")) {
        char zclass[256];
        zclass[0] = 0;
        GetCurrAttribValue(rover, false, "class", zclass, 256);
        if (!strcmp(zclass, "msi_unit"))
          num_OK = true;
      }
      rover = rover->next;
      if (rover && !strcmp(rover->src_tok, "mi")) {
        char zclass[256];
        zclass[0] = 0;
        GetCurrAttribValue(rover, false, "class", zclass, 256);
        if (!strcmp(zclass, "msi_unit"))
          den_OK = true;
      }
    }
  }

  return num_OK && den_OK;
}

bool Analyzer::IsPositionalChild(MNODE * mml_node)
{
  MNODE *the_parent = mml_node->parent;
  if (!the_parent && mml_node->prev) {
    MNODE *rover = mml_node->prev;
    while (rover->prev)
      rover = rover->prev;
    the_parent = rover->parent;
  }

  return HasPositionalChildren(the_parent);
}

bool Analyzer::IsDIFFOP(MNODE * mml_frac_node,
                            MNODE ** m_num_operand, MNODE ** m_den_var_expr)
{
  bool rv = false;
  *m_num_operand = NULL;
  *m_den_var_expr = NULL;

  if (mml_frac_node) {
    MNODE *num = mml_frac_node->first_kid;
    if (num) {
      MNODE *den = num->next;
      if (den) {

        if (!strcmp(num->src_tok, "mrow")) {  // dy OR d expr
          num = num->first_kid;
          *m_num_operand = num->next;
        }
        const char *num_elem = num->src_tok;  // mo OR msup
        const char *num_data = num->p_chdata; // d OR d^2

        if (!strcmp(num_elem, "msup")) {  // d^2
          MNODE *num_base = num->first_kid;
          num_elem = num_base->src_tok; // mo
          num_data = num_base->p_chdata;  // d
        }

        if (strcmp(num_elem, "mo")) // must be a diff op here
          return false;

        int diff_symbol = 0;
        if (!strcmp(num_data, "&#x2146;"))  // &dd;
          diff_symbol = 1;
        else if (!strcmp(num_data, "&#x2202;")) // &PartialD;
          diff_symbol = 2;

        if (diff_symbol) {
          const char *den_elem = den->src_tok;
          if (!strcmp(den_elem, "mrow")) {  // d * x
            MNODE *den1 = den->first_kid;
            if (den1) {
              // may have a product in the denom - dx^2 * dy^5
              if (!strcmp(den1->src_tok, "mrow"))
                den1 = den1->first_kid;

              const char *den1_elem = den1->src_tok;  // mo
              const char *den1_data = den1->p_chdata; // &dd;

              if (strcmp(den1_elem, "mo"))  // diff op
                return false;

              int diff1_symbol = 0;
              if (!strcmp(den1_data, "&#x2146;")) // &dd;
                diff1_symbol = 1;
              else if (!strcmp(den1_data, "&#x2202;"))  // &PartialD;
                diff1_symbol = 2;

              if (diff_symbol == diff1_symbol) {
                MNODE *den2 = den1->next;
                const char *den2_elem = den2->src_tok;
                if (!strcmp(den2_elem, "mi")  // dx{^2}
                    || !strcmp(den2_elem, "msup")) {
                  rv = true;
                  *m_den_var_expr = den;
                }
              }
            }
          }
        }
      }
    }
  }

  return rv;
}

bool Analyzer::IsDDIFFOP(MNODE * mml_msub_node)
{
  bool rv = false;

  if (mml_msub_node && mml_msub_node->first_kid) {
    MNODE *base = mml_msub_node->first_kid;
    const char *base_elem = base->src_tok;
    const char *base_data = base->p_chdata;
    if (!strcmp(base_elem, "mo")
        && !strcmp(base_data, "&#x2145;"))  // &DD;
      rv = true;
  } else
    TCI_ASSERT(0);

  return rv;
}

// Some subscripted fences are intrepreted as "subs".
bool Analyzer::IsSUBSTITUTION(MNODE * mml_msub_node)
{
  bool rv = false;

  if (mml_msub_node) {
    MNODE *base = mml_msub_node->first_kid;
    if (base) {
      MNODE *sub = base->next;
      if (sub) {
        const char *base_elem = base->src_tok;
        if (!strcmp(base_elem, "mfenced")) {
          char zopen_attr_val[32];
          zopen_attr_val[0] = 0;
          GetCurrAttribValue(base, false, "open", zopen_attr_val, 256);

          char zclose_attr_val[32];
          zclose_attr_val[0] = 0;
          GetCurrAttribValue(base, false, "close", zclose_attr_val, 256);

          if (zopen_attr_val[0] == '[' && zclose_attr_val[0] == ']')
            rv = true;
          if (zopen_attr_val[0] == 'I' && zclose_attr_val[0] == ']')
            rv = true;
          if (zopen_attr_val[0] == 'I' && zclose_attr_val[0] == '|')
            rv = true;
        }
      }
    }
  }

  return rv;
}

bool Analyzer::IsUSunit(const char *ptr)
{
  return false;
}

BUCKET_REC *Analyzer::AddVarToBucket(U32 bucket_ID,
                                     SEMANTICS_NODE * s_var_list)
{
  BUCKET_REC *head = NULL;
  BUCKET_REC *tail;

  char *buffer = NULL;
  U32 buffer_ln = 0;

  SEMANTICS_NODE *sv_rover = s_var_list;
  if (sv_rover->semantic_type == SEM_TYP_MATH_CONTAINER
      && sv_rover->bucket_list)
    sv_rover = sv_rover->bucket_list->first_child;

  while (sv_rover
         && sv_rover->semantic_type == SEM_TYP_PRECEDENCE_GROUP
         && !sv_rover->next)
    sv_rover = sv_rover->bucket_list->first_child;

  while (sv_rover) {
    SEMANTICS_NODE *s_curr_var = sv_rover;
    if (sv_rover->semantic_type == SEM_TYP_POWERFORM) {
      BUCKET_REC *bucket =
        FindBucketRec(sv_rover->bucket_list, MB_SCRIPT_BASE);
      s_curr_var = bucket->first_child;
    }

    if (s_curr_var->semantic_type == SEM_TYP_VARIABLE
        && s_curr_var->contents) {
      bool do_it = true;
      if (buffer) {
        char *ptr = strstr(buffer, s_curr_var->contents);
        if (ptr) {
          do_it = false;
        }
      }

      if (do_it) {
        SEMANTICS_NODE *s_var = CreateSemanticsNode();
        s_var->semantic_type = SEM_TYP_VARIABLE;

        if (s_curr_var->canonical_ID) {
          size_t ln = strlen(s_curr_var->canonical_ID);
          char *tmp = TCI_NEW(char[ln + 1]);
          strcpy(tmp, s_curr_var->canonical_ID);
          s_var->canonical_ID = tmp;
          SetSnodeOwner(s_var);
        }

        size_t ln = strlen(s_curr_var->contents);
        char *tmp = TCI_NEW(char[ln + 1]);
        strcpy(tmp, s_curr_var->contents);
        s_var->contents = tmp;
        buffer = AppendStr2HeapStr(buffer, buffer_ln, tmp);

        BUCKET_REC *fvar_bucket = MakeBucketRec(bucket_ID, s_var);
        s_var->parent = fvar_bucket;

        if (!head)
          head = fvar_bucket;
        else
          tail->next = fvar_bucket;
        tail = fvar_bucket;
      }
    } else
      TCI_ASSERT(0);
    sv_rover = sv_rover->next;
  }
  delete[] buffer;

  return head;
}

// Before we process a DE, it may be necessary to traverse the input.
// We look for things like "D_{x^2y}" and "d^{4}(?)/d^{2}x*d^{2}y"
// These constructs give us the independent variable(s).
// This function generates a list of SEMANTICS_NODEs.

SEMANTICS_NODE *Analyzer::DetermineIndepVar(MNODE * dMML_list)
{
  SEMANTICS_NODE *head = NULL;
  SEMANTICS_NODE *tail;

  MNODE *rover = dMML_list;
  while (rover) {
    SEMANTICS_NODE *s_indvar = NULL;
    if (!strcmp(rover->src_tok, "mfrac")) {
      s_indvar = GetIndVarFromFrac(rover);
    } else if (!strcmp(rover->src_tok, "msub")) {
      s_indvar = GetIndepVarFromSub(rover);
    }

    if (!s_indvar && rover->first_kid)
      s_indvar = DetermineIndepVar(rover->first_kid);

    if (s_indvar) {
      if (!head)
        head = s_indvar;
      else {
        tail->next = s_indvar;
        s_indvar->prev = tail;
      }
      tail = s_indvar;
    }
    rover = rover->next;
  }

  return head;
}

/*
<mrow>
  <mrow>
    <mo form="prefix">&#x2202;</mo>
    <mi>x</mi>
  </mrow>
  <mo>&#x2062;</mo>
  <mrow>
    <mo form="prefix">&#x2202;</mo>
    <mi>y</mi>
  </mrow>
  <mo>&#x2062;</mo>
  <mrow>
    <mo form="prefix">&#x2202;</mo>
    <msup>
      <mi>z</mi>
      <mn>2</mn>
    </msup>
  </mrow>
</mrow>
*/

SEMANTICS_NODE *Analyzer::GetIndVarFromFrac(MNODE * mfrac)
{
  SEMANTICS_NODE *head = NULL;
  SEMANTICS_NODE *tail;

  MNODE *m_num_operand;
  MNODE *m_den;
  if (IsDIFFOP(mfrac, &m_num_operand, &m_den)) {
    if (m_den) {
      if (!strcmp(m_den->src_tok, "mrow")) {
        MNODE *rover = m_den->first_kid;

        if (!strcmp(rover->src_tok, "mrow")) {
          // denominator is a list of differentials
          while (rover) {
            if (!strcmp(rover->src_tok, "mrow")) {
              MNODE *diff_op = rover->first_kid;
              MNODE *m_den_var_expr = diff_op->next;
              SEMANTICS_NODE *snode = ExtractIndepVar(m_den_var_expr);

              if (snode) {
                if (!head)
                  head = snode;
                else {
                  tail->next = snode;
                  snode->prev = tail;
                }
                tail = snode;
              }
            }
            rover = rover->next;
          }
        } else {
          // denominator is a single differential
          MNODE *diff_op = rover;
          MNODE *m_den_var_expr = diff_op->next;
          SEMANTICS_NODE *snode = ExtractIndepVar(m_den_var_expr);
          head = snode;
        }
      } else
        TCI_ASSERT(0);
    } else
      TCI_ASSERT(0);
  }

  return head;
}

// input comes from "D_{??}"

SEMANTICS_NODE *Analyzer::GetIndepVarFromSub(MNODE * msub)
{
  SEMANTICS_NODE *head = NULL;
  SEMANTICS_NODE *tail;

  if (IsDDIFFOP(msub)) {
    MNODE *base = msub->first_kid;
    MNODE *sub = base->next;

    // subscript is a (list of) differentials
    MNODE *rover = sub;
    if (!strcmp(rover->src_tok, "mrow"))
      rover = rover->first_kid;

    while (rover) {
      if (!strcmp(rover->src_tok, "mi")
          || !strcmp(rover->src_tok, "msup")) {
        SEMANTICS_NODE *snode = ExtractIndepVar(rover);
        if (snode) {
          if (!head)
            head = snode;
          else {
            tail->next = snode;
            snode->prev = tail;
          }
          tail = snode;
        }
      }
      rover = rover->next;
    }
  }

  return head;
}

// Here, snode is of type SEM_TYP_DERIVATIVE.
// Append a bucket of type MB_NPRIMES to snode->bucket_list.

void Analyzer::AddPrimesCount(SEMANTICS_NODE * snode, MNODE * primes)
{
  int n_primes = 0;

  if (!strcmp(primes->src_tok, "mfenced")) {
    MNODE *cont = primes->first_kid;
    if (!strcmp(cont->src_tok, "mn")) {
      n_primes = atoi(cont->p_chdata);
      AppendNumber(snode, MB_NPRIMES, n_primes);
    } else {
      TCI_ASSERT(0);
    }
  } else {
    MNODE *rover = primes;
    if (!strcmp(rover->src_tok, "mrow"))
      rover = rover->first_kid;
    // <mo form="postfix">&#x2032;</mo>
    while (rover) {
      if (!strcmp(rover->src_tok, "mo"))
        n_primes++;
      rover = rover->next;
    }
    AppendNumber(snode, MB_NPRIMES, n_primes);
  }
}

void Analyzer::AppendNumber(SEMANTICS_NODE * snode, U32 bucket_ID, int num)
{
  SEMANTICS_NODE *s_nprimes = CreateSemanticsNode();
  BUCKET_REC *pr_bucket = MakeBucketRec(bucket_ID, s_nprimes);
  snode->bucket_list = AppendBucketRec(snode->bucket_list, pr_bucket);
  pr_bucket->first_child = s_nprimes;
  s_nprimes->parent = pr_bucket;

  s_nprimes->semantic_type = SEM_TYP_NUMBER;
  char buffer[32];
  sprintf(buffer, "%d", num);
  size_t zln = strlen(buffer);
  char *tmp = TCI_NEW(char[zln + 1]);
  strcpy(tmp, buffer);
  s_nprimes->contents = tmp;
}

void Analyzer::RemoveBucket(SEMANTICS_NODE * s_base, BUCKET_REC * targ)
{
  BUCKET_REC *prev = NULL;
  BUCKET_REC *rover = s_base->bucket_list;
  while (rover) {
    BUCKET_REC *del = rover;
    rover = rover->next;
    if (del == targ) {
      if (del == s_base->bucket_list)
        s_base->bucket_list = rover;
      else
        prev->next = rover;
      del->next = NULL;
      DisposeBucketList(del);
      break;
    } else
      prev = del;
  }
}

// Traverse an mml tree that represents an ODE, look for constructs
// that represent derivatives.  Record the names of the functions
//  whose derivatives are encountered.

void Analyzer::DetermineODEFuncNames(MNODE * dMML_tree)
{
  MNODE *rover = dMML_tree;
  while (rover) {
    char *f_name = NULL;
    const char *src_name = NULL;
    if (!strcmp(rover->src_tok, "mfrac"))
      f_name = GetFuncNameFromFrac(rover, &src_name);
    else if (!strcmp(rover->src_tok, "msub"))
      f_name = GetFuncNameFromSub(rover, &src_name);
    else if (!strcmp(rover->src_tok, "msup"))
      f_name = GetFuncNameFromSup(rover, &src_name);
    else if (!strcmp(rover->src_tok, "msubsup"))
      f_name = GetFuncNameFromSubSup(rover, &src_name);
    if (f_name) {
      char *new_src_name = NULL;
      if (src_name) {
        size_t zln = strlen(src_name);
        char *tmp = TCI_NEW(char[zln + 1]);
        strcpy(tmp, src_name);
        new_src_name = tmp;
      }
      DE_func_names = AppendFuncName(DE_func_names, f_name, new_src_name);
    }
    rover = rover->next;
  }

  rover = dMML_tree;
  while (rover) {
    if (rover->first_kid)
      DetermineODEFuncNames(rover->first_kid);
    rover = rover->next;
  }
}

// Traverse an mml tree that represents an PDE, look for constructs
// that represent derivatives.  Record the names of the functions
//  whose derivatives are encountered.

void Analyzer::DeterminePDEFuncNames(MNODE * dMML_tree)
{
  MNODE *rover = dMML_tree;
  while (rover) {
    char *f_name = NULL;
    const char *src_name = NULL;
    if (!strcmp(rover->src_tok, "mfrac"))
      f_name = GetFuncNameFromFrac(rover, &src_name);
    if (f_name) {
      char *new_src_name = NULL;
      if (src_name) {
        size_t zln = strlen(src_name);
        char *tmp = TCI_NEW(char[zln + 1]);
        strcpy(tmp, src_name);
        new_src_name = tmp;
      }
      DE_func_names = AppendFuncName(DE_func_names, f_name, new_src_name);
    }
    rover = rover->next;
  }

  rover = dMML_tree;
  while (rover) {
    if (rover->first_kid)
      DeterminePDEFuncNames(rover->first_kid);
    rover = rover->next;
  }
}

void Analyzer::DisposeODEFuncNames(DE_FUNC_REC * func_names)
{
  DE_FUNC_REC *rover = func_names;
  while (rover) {
    DE_FUNC_REC *del = rover;
    rover = rover->next;
    delete[] del->zfunc_canon_name;
    delete[] del->zfunc_src_name;
    delete del;
  }
}

// Locate an DE_FUNC_REC by the function name it holds.
DE_FUNC_REC *Analyzer::LocateFuncRec(DE_FUNC_REC * f_list,
                                     const char *canon_name,
                                     const char *src_name)
{
  DE_FUNC_REC *rover = f_list;
  while (rover) {
    if (canon_name && rover->zfunc_canon_name &&
        !strcmp(rover->zfunc_canon_name, canon_name))
      break;
    if (src_name && rover->zfunc_src_name &&
        !strcmp(rover->zfunc_src_name, src_name))
      break;
    rover = rover->next;
  }

  return rover;
}

// Append a function record to DE_func_names, provided
//  that the function is not already in the list.
//  Note that these are mml canonical names here - miy, etc.

DE_FUNC_REC *Analyzer::AppendFuncName(DE_FUNC_REC * f_list, char *f_name,
                                      char *src_name)
{
  DE_FUNC_REC *rv = f_list;

  if (!LocateFuncRec(f_list, f_name, src_name)) {
    DE_FUNC_REC *new_rec = TCI_NEW(DE_FUNC_REC);
    new_rec->next = f_list;
    new_rec->zfunc_canon_name = f_name;
    new_rec->zfunc_src_name = src_name;
    rv = new_rec;
  } else {
    delete[] src_name;
    delete[] f_name;
  }

  return rv;
}

// \frac{df}{dx}

char *Analyzer::GetFuncNameFromFrac(MNODE * mfrac, const char **src_name)
{
  char *rv = NULL;

  MNODE *m_num_operand;
  MNODE *m_den_var_expr;
  if (IsDIFFOP(mfrac, &m_num_operand, &m_den_var_expr)) {
    if (m_num_operand) {
      if (!strcmp(m_num_operand->src_tok, "mi")) {
        rv = GetCanonicalIDforMathNode(m_num_operand);
        *src_name = m_num_operand->p_chdata;
      }
    }
  }
  return rv;
}

char *Analyzer::GetFuncNameFromSub(MNODE * msub, const char **src_name)
{
  char *rv = NULL;

  if (IsDDIFFOP(msub)) {
    MNODE *m_operand = msub->next;
    if (m_operand) {
      if (!strcmp(m_operand->src_tok, "mi")) {
        rv = GetCanonicalIDforMathNode(m_operand);
        *src_name = m_operand->p_chdata;
      } else if (!strcmp(m_operand->src_tok, "msub")) {
        // D_{x}y_{1}
        MNODE *base = m_operand->first_kid;
        if (!strcmp(base->src_tok, "mi")) {
          rv = GetCanonicalIDforMathNode(m_operand);
          *src_name = base->p_chdata;
        }
      }
    }
  }
  return rv;
}

char *Analyzer::GetFuncNameFromSup(MNODE * msup, const char **src_name)
{
  char *rv = NULL;

  MNODE *base = msup->first_kid;
  BaseType bt = GetBaseType(msup, false);
  ExpType et = GetExpType(bt, base->next);

  if (et == ET_PRIMES) {
    if (!strcmp(base->src_tok, "mi")) {
      rv = GetCanonicalIDforMathNode(base);
      *src_name = base->p_chdata;
    }
  }
  return rv;
}

char *Analyzer::GetFuncNameFromSubSup(MNODE * msubsup, const char **src_name)
{
  char *rv = NULL;

  MNODE *base = msubsup->first_kid;
  MNODE *sub = base->next;
  MNODE *exp = sub->next;
  BaseType bt = GetBaseType(msubsup, false);
  ExpType et = GetExpType(bt, exp);

  if (et == ET_PRIMES) {
    if (!strcmp(base->src_tok, "mi")) {
      U32 zh_ln = 0;
      rv = AppendStr2HeapStr(rv, zh_ln, "msub");
      char *tmp = GetCanonicalIDforMathNode(base);
      *src_name = base->p_chdata;
      if (tmp) {
        rv = AppendStr2HeapStr(rv, zh_ln, tmp);
        delete[] tmp;
      }
      tmp = GetCanonicalIDforMathNode(sub);
      if (tmp) {
        rv = AppendStr2HeapStr(rv, zh_ln, tmp);
        delete[] tmp;
      }
    }
  }
  return rv;
}

// Translation of a subscript has been placed in a separate function
//  just in case we want to do something special with it.
// Nothing fancy here yet.

SEMANTICS_NODE *Analyzer::QualifierToSNODE(MNODE * sub)
{
  return GetSemanticsFromNode(sub, NULL);
}

// The following bullshit arises because of the careless
//  use of \limfunc in SWP help documents when \func is needed

Analyzer::PrefixOpIlk Analyzer::GetPrefixOpCode(const char *op_name, SemanticVariant & n_integs)
{
  PrefixOpIlk rv = POI_none;
  n_integs = SNV_None;

  char *ptr = strchr(op_name, '&');
  if (ptr) {
    U32 unicodes[8];
    int char_tally = ChData2Unicodes(op_name, unicodes, 8);
    if (char_tally && unicodes[0] == 0x2207) {  // nabla
      if (char_tally == 1) {
        rv = POI_gradient;

      } else if (char_tally == 2) {
        if (unicodes[1] == 0x22C5)
          rv = POI_divergence;
        else if (unicodes[1] == 0xD7)
          rv = POI_curl;
      } else if (char_tally == 3) {
        if (unicodes[1] == 0x22C5 && unicodes[2] == 0x2207)
          rv = POI_Laplacian;
      }
    } else if (char_tally == 1) {
      SemanticType bo_type = GetBigOpType(op_name, n_integs);
      if (bo_type == SEM_TYP_BIGOP_INTEGRAL)
        rv = POI_integral;
      else if (bo_type == SEM_TYP_BIGOP_SUM)
        rv = POI_sum;
    }

    return rv;
  }

  size_t ln = strlen(op_name);
  switch (ln) {
  case 3:
    if (!strcmp(op_name, "gcd"))
      rv = POI_listop;
    else if (!strcmp(op_name, "lcm"))
      rv = POI_listop;
    else if (!strcmp(op_name, "max"))
      rv = POI_listop;
    else if (!strcmp(op_name, "min"))
      rv = POI_listop;
    else if (!strcmp(op_name, "det"))
      rv = POI_det;
    else if (!strcmp(op_name, "div"))
      rv = POI_divergence;
    break;

  case 4:
    if (!strcmp(op_name, "FDen"))
      rv = POI_distribution;
    else if (!strcmp(op_name, "FInv"))
      rv = POI_distribution;
    else if (!strcmp(op_name, "TDen"))
      rv = POI_distribution;
    else if (!strcmp(op_name, "TInv"))
      rv = POI_distribution;
    else if (!strcmp(op_name, "curl"))
      rv = POI_curl;
    else if (!strcmp(op_name, "grad"))
      rv = POI_distribution;
    break;

  case 5:
    if (!strcmp(op_name, "FDist"))
      rv = POI_distribution;
    else if (!strcmp(op_name, "TDist"))
      rv = POI_distribution;
    else if (!strcmp(op_name, "Dirac"))
      rv = POI_Dirac;
    break;

  case 7:
    if (!strcmp(op_name, "BetaDen"))
      rv = POI_distribution;
    else if (!strcmp(op_name, "BetaInv"))
      rv = POI_distribution;
    break;

  case 8:
    if (!strcmp(op_name, "GammaDen"))
      rv = POI_distribution;
    else if (!strcmp(op_name, "GammaInv"))
      rv = POI_distribution;
    else if (!strcmp(op_name, "BetaDist"))
      rv = POI_distribution;
    break;

  case 9:
    if (!strcmp(op_name, "GammaDist"))
      rv = POI_distribution;
    else if (!strcmp(op_name, "CauchyDen"))
      rv = POI_distribution;
    else if (!strcmp(op_name, "CauchyInv"))
      rv = POI_distribution;
    else if (!strcmp(op_name, "NormalDen"))
      rv = POI_distribution;
    else if (!strcmp(op_name, "NormalInv"))
      rv = POI_distribution;
    else if (!strcmp(op_name, "Heaviside"))
      rv = POI_Dirac;
    break;

  case 10:
    if (!strcmp(op_name, "CauchyDist"))
      rv = POI_distribution;
    else if (!strcmp(op_name, "NormalDist"))
      rv = POI_distribution;
    else if (!strcmp(op_name, "PoissonDen"))
      rv = POI_distribution;
    else if (!strcmp(op_name, "PoissonInv"))
      rv = POI_distribution;
    else if (!strcmp(op_name, "UniformDen"))
      rv = POI_distribution;
    else if (!strcmp(op_name, "UniformInv"))
      rv = POI_distribution;
    else if (!strcmp(op_name, "WeibullDen"))
      rv = POI_distribution;
    else if (!strcmp(op_name, "WeibullInv"))
      rv = POI_distribution;
    break;

  case 11:
    if (!strcmp(op_name, "PoissonDist"))
      rv = POI_distribution;
    else if (!strcmp(op_name, "UniformDist"))
      rv = POI_distribution;
    else if (!strcmp(op_name, "WeibullDist"))
      rv = POI_distribution;
    else if (!strcmp(op_name, "BinomialDen"))
      rv = POI_distribution;
    else if (!strcmp(op_name, "BinomialInv"))
      rv = POI_distribution;
    break;

  case 12:
    if (!strcmp(op_name, "BinomialDist"))
      rv = POI_distribution;
    else if (!strcmp(op_name, "ChiSquareDen"))
      rv = POI_distribution;
    else if (!strcmp(op_name, "ChiSquareInv"))
      rv = POI_distribution;
    else if (!strcmp(op_name, "HypergeomDen"))
      rv = POI_distribution;
    else if (!strcmp(op_name, "HypergeomInv"))
      rv = POI_distribution;
    break;

  case 13:
    if (!strcmp(op_name, "ChiSquareDist"))
      rv = POI_distribution;
    else if (!strcmp(op_name, "HypergeomDist"))
      rv = POI_distribution;
    break;

  case 14:
    if (!strcmp(op_name, "ExponentialDen"))
      rv = POI_distribution;
    else if (!strcmp(op_name, "ExponentialInv"))
      rv = POI_distribution;
    break;

  case 15:
    if (!strcmp(op_name, "ExponentialDist"))
      rv = POI_distribution;
    break;

  default:
    break;
  }

  return rv;
}

void Analyzer::ArgsToMatrix(SEMANTICS_NODE * snode, BUCKET_REC * b_list)
{
  int n_rows = 1;
  int n_cols = 0;
  BUCKET_REC *brover = b_list;
  while (brover) {
    n_cols++;
    brover = brover->next;
  }

  if (n_cols > 1) {
    SEMANTICS_NODE *s_matrix = CreateSemanticsNode();
    s_matrix->semantic_type = SEM_TYP_TABULATION;
    s_matrix->nrows = n_rows;
    s_matrix->ncols = n_cols;
    char *tmp = TCI_NEW(char[8]);
    strcpy(tmp, "matrix");
    s_matrix->contents = tmp;

    BUCKET_REC *bucket = MakeBucketRec(MB_UNNAMED, s_matrix);
    s_matrix->parent = bucket;
    s_matrix->bucket_list = AppendBucketRec(s_matrix->bucket_list, b_list);

    snode->bucket_list = AppendBucketRec(snode->bucket_list, bucket);
  } else
    snode->bucket_list = AppendBucketRec(snode->bucket_list, b_list);
}

bool Analyzer::OpArgIsMatrix(MNODE * mml_mi_node)
{
  bool rv = false;
  size_t ln = strlen(mml_mi_node->p_chdata);
  switch (ln) {
  case 3:
    if (!strcmp("div", mml_mi_node->p_chdata))
      rv = true;
    break;
  case 4:
    if (!strcmp("curl", mml_mi_node->p_chdata))
      rv = true;
    break;
  default:
    break;
  }

  return rv;
}

// Here we nest operands under their operators
//  ie. convert to a prefix (reverse Polish) data structure

void Analyzer::CreatePrefixForm(SEMANTICS_NODE * s_operator,
                                SEMANTICS_NODE * l_operand,
                                SEMANTICS_NODE * r_operand)
{
  if (l_operand) {
    l_operand = RemoveParens(l_operand);
    BUCKET_REC *arg1_bucket = MakeBucketRec(MB_UNNAMED, l_operand);
    s_operator->bucket_list =
      AppendBucketRec(s_operator->bucket_list, arg1_bucket);
    l_operand->parent = arg1_bucket;
    l_operand->prev = NULL;
    l_operand->next = NULL;
  }
  if (r_operand) {
    r_operand = RemoveParens(r_operand);
    BUCKET_REC *arg2_bucket = MakeBucketRec(MB_UNNAMED, r_operand);
    s_operator->bucket_list =
      AppendBucketRec(s_operator->bucket_list, arg2_bucket);
    r_operand->parent = arg2_bucket;
    r_operand->prev = NULL;
    r_operand->next = NULL;
  }
}

void Analyzer::SetVarAndIntervalLimit(BUCKET_REC * ll_bucket)
{
  SEMANTICS_NODE *s_ll;
  SEMANTICS_NODE *s_ul;
  bool ll_is_inclusive;
  bool ul_is_inclusive;
  SEMANTICS_NODE *s_var = LocateVarAndLimits(ll_bucket, &s_ll, &s_ul,
                                             ll_is_inclusive,
                                             ul_is_inclusive);
  if (s_ll || s_ul) {
    if (s_ll && s_ll->parent)
      s_ll->parent->first_child = NULL;
    if (s_ul && s_ul->parent)
      s_ul->parent->first_child = NULL;
    if (s_var && s_var->parent)
      s_var->parent->first_child = NULL;

    DisposeSList(ll_bucket->first_child);
    ll_bucket->first_child = NULL;

    SEMANTICS_NODE *interval = CreateSemanticsNode();
    interval->semantic_type = SEM_TYP_INTERVAL;
    char *tmp = TCI_NEW(char[10]);
    strcpy(tmp, "interval");
    interval->contents = tmp;

    SemanticVariant interval_type;
    if (ll_is_inclusive)
      interval_type =
        ul_is_inclusive ? SNV_InclInclInterval : SNV_InclExclInterval;
    else
      interval_type =
        ul_is_inclusive ? SNV_ExclInclInterval : SNV_ExclExclInterval;
    interval->variant = interval_type;

    if (s_ll) {
      BUCKET_REC *bucket = MakeBucketRec(MB_INTERVAL_START, s_ll);
      interval->bucket_list = AppendBucketRec(interval->bucket_list, bucket);
      s_ll->parent = bucket;
    }
    if (s_ul) {
      BUCKET_REC *bucket = MakeBucketRec(MB_INTERVAL_END, s_ul);
      interval->bucket_list = AppendBucketRec(interval->bucket_list, bucket);
      s_ul->parent = bucket;
    }

    if (s_var) {
      BUCKET_REC *bucket = MakeBucketRec(MB_INTERVAL_VAR, s_var);
      interval->bucket_list = AppendBucketRec(interval->bucket_list, bucket);
      s_var->parent = bucket;
    }
    ll_bucket->first_child = interval;
    interval->parent = ll_bucket;
  } else
    TCI_ASSERT(0);
}

void Analyzer::SetVarArrowExprLimit(BUCKET_REC * ll_bucket)
{
  SEMANTICS_NODE *s_expr;
  int direction;
  SEMANTICS_NODE *s_var = LocateVarAndExpr(ll_bucket, &s_expr,
                                           direction);
  if (s_var) {
    BUCKET_REC *b_list = NULL;
    BUCKET_REC *var_bucket = MakeBucketRec(MB_LL_VAR, s_var);
    b_list = AppendBucketRec(b_list, var_bucket);
    if (s_var->parent && s_var->parent->first_child == s_var)
      s_var->parent->first_child = NULL;
    s_var->parent = var_bucket;

    if (s_expr) {
      BUCKET_REC *bucket = MakeBucketRec(MB_LL_EXPR, s_expr);
      b_list = AppendBucketRec(b_list, bucket);
      if (s_expr->parent && s_expr->parent->first_child == s_expr)
        s_expr->parent->first_child = NULL;
      s_expr->parent = bucket;
    }

    SEMANTICS_NODE *s_direction_num = CreateSemanticsNode();
    s_direction_num->semantic_type = SEM_TYP_NUMBER;
    char *num_str = TCI_NEW(char[2]);
    num_str[0] = direction + '0'; // '+' -> 1, '-' -> 2
    num_str[1] = 0;
    s_direction_num->contents = num_str;

    BUCKET_REC *bucket = MakeBucketRec(MB_LL_DIRECTION,
                                       s_direction_num);
    s_direction_num->parent = bucket;
    b_list = AppendBucketRec(b_list, bucket);

    DisposeSList(ll_bucket->first_child);
    ll_bucket->first_child = NULL;

    ll_bucket->parts = b_list;
  }
}

int Analyzer::GetLimitFormat(char *op_name)
{
  int rv = 0;

  U32 ID, subID;
  const char *p_data;
  if (mml_entities->
      GetRecordFromName("LIMFORMS", op_name, strlen(op_name), ID, subID,
                        &p_data)) {
    if (p_data && *p_data)
      rv = atoi(p_data);
  }

  return rv;
}

SEMANTICS_NODE *Analyzer::LocateVarAndLimits(BUCKET_REC * l_bucket,
                                             SEMANTICS_NODE ** s_ll,
                                             SEMANTICS_NODE ** s_ul,
                                             bool & ll_is_inclusive,
                                             bool & ul_is_inclusive)
{
  SEMANTICS_NODE *rv = NULL;
  *s_ll = NULL;
  *s_ul = NULL;
  ll_is_inclusive = false;
  ul_is_inclusive = false;

  if (l_bucket && l_bucket->first_child) {
    SEMANTICS_NODE *s_node = l_bucket->first_child;
    if (s_node->semantic_type == SEM_TYP_INFIX_OP) {
      BUCKET_REC *b_rover = s_node->bucket_list;
      if (b_rover) {
        SEMANTICS_NODE *e1_list = NULL;
        SEMANTICS_NODE *relop1 = s_node;
        SEMANTICS_NODE *e2_list = NULL;
        SEMANTICS_NODE *relop2 = NULL;
        SEMANTICS_NODE *e3_list = NULL;

        OpOrderIlk op_order = GetOpOrderIlk(relop1);
        if (op_order == OOI_element) {
          e1_list = b_rover->first_child;
          if (e1_list && e1_list->semantic_type == SEM_TYP_VARIABLE) {
            rv = e1_list;

            b_rover = b_rover->next;
            s_node = b_rover->first_child;
            if (s_node && s_node->bucket_list) {
              if (s_node->semantic_type == SEM_TYP_BRACKETED_LIST) {
                b_rover = s_node->bucket_list;
                *s_ll = b_rover->first_child;
                *s_ul = b_rover->next->first_child;
                ll_is_inclusive = true;
                ul_is_inclusive = true;
              } else if (s_node->semantic_type == SEM_TYP_PARENED_LIST) {
                b_rover = s_node->bucket_list;
                *s_ll = b_rover->first_child;
                *s_ul = b_rover->next->first_child;
                ll_is_inclusive = false;
                ul_is_inclusive = false;
              } else if (s_node->semantic_type == SEM_TYP_INTERVAL) {
                BUCKET_REC *b_ll =
                  FindBucketRec(s_node->bucket_list, MB_INTERVAL_START);
                if (b_ll) {
                  if (s_node->variant == SNV_InclInclInterval
                      || s_node->variant == SNV_InclExclInterval)
                    ll_is_inclusive = true;
                  else
                    ll_is_inclusive = false;
                  *s_ll = b_ll->first_child;
                } else
                  TCI_ASSERT(0);
                BUCKET_REC *b_ul =
                  FindBucketRec(s_node->bucket_list, MB_INTERVAL_END);
                if (b_ul) {
                  if (s_node->variant == SNV_ExclInclInterval
                      || s_node->variant == SNV_InclInclInterval)
                    ul_is_inclusive = true;
                  else
                    ul_is_inclusive = false;
                  *s_ul = b_ul->first_child;
                } else
                  TCI_ASSERT(0);
              } else
                TCI_ASSERT(0);
            } else
              TCI_ASSERT(0);
          } else
            TCI_ASSERT(0);
        } else if (op_order != OOI_none) { // relational
          s_node = b_rover->first_child;
          if (s_node->semantic_type == SEM_TYP_INFIX_OP) {
            relop2 = s_node;
            e3_list = b_rover->next->first_child;
            b_rover = s_node->bucket_list;
          }
          e1_list = b_rover->first_child;
          e2_list = b_rover->next->first_child;

          if (e1_list && e1_list->semantic_type == SEM_TYP_VARIABLE) {
            rv = e1_list;
            if (op_order == OOI_lessthan || op_order == OOI_lessorequal) {
              *s_ul = e2_list;
              if (op_order == OOI_lessorequal)
                ul_is_inclusive = true;
            } else if (op_order == OOI_equal) { // equal
              if (e2_list->semantic_type == SEM_TYP_INFIX_OP) { // n=1..4
                BUCKET_REC *b_ll = e2_list->bucket_list;
                if (b_ll
                    && b_ll->first_child
                    && b_ll->next && b_ll->next->first_child) {
                  *s_ll = b_ll->first_child;
                  *s_ul = b_ll->next->first_child;
                } else
                  TCI_ASSERT(0);
              } else {
                *s_ll = e2_list;
                *s_ul = e2_list;
              }
            } else if (op_order == OOI_greaterthan || op_order == OOI_greaterorequal) {
              *s_ll = e2_list;
              if (op_order == OOI_greaterorequal)
                ll_is_inclusive = true;
            }
          } else if (e2_list
                     && e2_list->semantic_type == SEM_TYP_VARIABLE) {
            rv = e2_list;
            if (relop2) {
              OpOrderIlk op_order2 = GetOpOrderIlk(relop2);
              if (op_order == OOI_lessthan || op_order == OOI_lessorequal) {
                *s_ll = e1_list;
                if (op_order == OOI_lessorequal)
                  ll_is_inclusive = true;
                *s_ul = e3_list;
                if (op_order2 == OOI_lessorequal)
                  ul_is_inclusive = true;
              } else if (op_order == OOI_equal) {
                *s_ll = e1_list;
                *s_ul = e3_list;
              } else if (op_order == OOI_greaterthan || op_order == OOI_greaterorequal) {
                *s_ll = e3_list;
                if (op_order2 == OOI_greaterorequal)
                  ll_is_inclusive = true;
                *s_ul = e1_list;
                if (op_order == OOI_greaterorequal)
                  ul_is_inclusive = true;
              }
            } else {
              if (op_order == OOI_lessthan || op_order == OOI_lessorequal) {
                *s_ll = e1_list;
                if (op_order == OOI_lessorequal)
                  ll_is_inclusive = true;
              } else if (op_order == OOI_equal) {
                *s_ll = e1_list;
                *s_ul = e1_list;
              } else if (op_order == OOI_greaterthan || op_order == OOI_greaterorequal) {
                *s_ul = e1_list;
                if (op_order == OOI_greaterorequal)
                  ul_is_inclusive = true;
              }
            }
          } else {
            TCI_ASSERT(0);
          }
        } else {
          TCI_ASSERT(0);
        }
      } else {
        TCI_ASSERT(0);
      }
    } else {
      TCI_ASSERT(0);
    }
  } else {
    TCI_ASSERT(0);
  }
  return rv;
}

SEMANTICS_NODE *Analyzer::LocateVarAndExpr(BUCKET_REC * l_bucket,
                                           SEMANTICS_NODE ** s_expr,
                                           int & direction)
{
  SEMANTICS_NODE *rv = NULL;
  *s_expr = NULL;
  direction = 0;

  if (l_bucket && l_bucket->first_child) {
    SEMANTICS_NODE *s_node = l_bucket->first_child;
    if (s_node->semantic_type == SEM_TYP_INFIX_OP) {
      BUCKET_REC *b_rover = s_node->bucket_list;
      if (b_rover) {
        SEMANTICS_NODE *e1_list = NULL;
        SEMANTICS_NODE *relop = s_node;
        SEMANTICS_NODE *e2_list = NULL;
        SEMANTICS_NODE *expr = NULL;

        OpOrderIlk op_order = GetOpOrderIlk(relop);
        if (op_order == OOI_rightarrow) {
          // descend to operands
          e1_list = b_rover->first_child;
          if (b_rover->next)
            e2_list = b_rover->next->first_child;
          else
            TCI_ASSERT(0);

          if (e1_list && e1_list->semantic_type == SEM_TYP_UCONSTANT) {
            TCI_ASSERT(0);
            // Might have mapped "i" to "imaginaryi".
          }
          if (e2_list && e2_list->semantic_type == SEM_TYP_UCONSTANT) {
            // may be approaching \infty
          }

          if (e1_list && e1_list->semantic_type == SEM_TYP_VARIABLE) {
            rv = e1_list;
            expr = e2_list;
          } else if (e2_list
                     && e2_list->semantic_type == SEM_TYP_VARIABLE) {
            rv = e2_list;
            expr = e1_list;
          } else {
            TCI_ASSERT(0);
          }
          if (expr->semantic_type == SEM_TYP_POWERFORM) {
            BUCKET_REC *base_bucket = FindBucketRec(expr->bucket_list,
                                                    MB_SCRIPT_BASE);
            BUCKET_REC *script_bucket = FindBucketRec(expr->bucket_list,
                                                      MB_SCRIPT_UPPER);
            if (base_bucket && base_bucket->first_child) {
              if (script_bucket && script_bucket->first_child) {
                SEMANTICS_NODE *s_super = script_bucket->first_child;
                if (s_super->semantic_type == SEM_TYP_INFIX_OP
                    || s_super->semantic_type == SEM_TYP_POSTFIX_OP) {
                  char *key = s_super->contents;
                  if (!strcmp(key, "+"))
                    direction = 1;
                  else if (!strcmp(key, "&#x2212;"))
                    direction = 2;
                  else
                    TCI_ASSERT(0);
                }
              } else {
                TCI_ASSERT(0);
              }
              *s_expr = base_bucket->first_child;
            } else {
              TCI_ASSERT(0);
            }
          } else {
            *s_expr = expr;
            // x -> 0+
            if (expr->semantic_type == SEM_TYP_POSTFIX_OP) {
              char *key = expr->contents;
              if (!strcmp(key, "+"))
                direction = 1;
              else if (!strcmp(key, "&#x2212;"))
                direction = 2;
              else
                TCI_ASSERT(0);
              if (expr->bucket_list && expr->bucket_list->first_child) {
                *s_expr = expr->bucket_list->first_child;
              } else
                TCI_ASSERT(0);
            }
          }
        } else {
          TCI_ASSERT(0);
        }
      } else {
        TCI_ASSERT(0);
      }
    } else {
      TCI_ASSERT(0);
    }
  } else {
    TCI_ASSERT(0);
  }
  return rv;
}

Analyzer::OpOrderIlk Analyzer::GetOpOrderIlk(SEMANTICS_NODE * relop)
{
  OpOrderIlk rv = OOI_none;

  if (relop && relop->contents) {
    char *data = relop->contents;
    U32 unicode = 0;
    char *ptr = strstr(data, "&#x");
    if (ptr)
      unicode = ASCII2U32(ptr + 3, 16);
    else
      unicode = data[0];

    switch (unicode) {
    case 0x3c:
      rv = OOI_lessthan;
      break;
    case 0x2264:
      rv = OOI_lessorequal;
      break;
    case 0x3d:
      rv = OOI_equal;
      break;
    case 0x3e:
      rv = OOI_greaterthan;
      break;
    case 0x2265:
      rv = OOI_greaterorequal;
      break;
    case 0x2208:
      rv = OOI_element;
      break;
    case 0x2192:
      rv = OOI_rightarrow;
      break;
    default:
      break;
    }
  } else {
    TCI_ASSERT(!"No operator contents.");
  }
  return rv;
}

SEMANTICS_NODE *Analyzer::RemoveInfixOps(SEMANTICS_NODE * s_var)
{
  SEMANTICS_NODE *rv = s_var;

  SEMANTICS_NODE *s_rover = s_var;
  while (s_rover) {
    SEMANTICS_NODE *save_next = s_rover->next;

    if (s_rover->semantic_type == SEM_TYP_INFIX_OP
        && s_rover->bucket_list) {
      SEMANTICS_NODE *s_list = NULL;

      BUCKET_REC *b_rover = s_rover->bucket_list;
      while (b_rover) {
        SEMANTICS_NODE *s_operand = b_rover->first_child;
        if (s_operand) {
          s_list = AppendSLists(s_list, s_operand);
          b_rover->first_child = NULL;
        }
        b_rover = b_rover->next;
      }
      save_next = s_list;
      SEMANTICS_NODE *l_anchor = NULL;
      if (s_rover->prev)
        l_anchor = s_rover->prev;
      else
        rv = s_list;
      SEMANTICS_NODE *r_anchor = NULL;
      if (s_rover->next)
        r_anchor = s_rover->next;

      BUCKET_REC *new_parent = s_rover->parent;
      DisposeSemanticsNode(s_rover);

      SEMANTICS_NODE *tail = s_list;
      while (tail->next) {
        tail->parent = new_parent;
        tail = tail->next;
      }
      tail->parent = new_parent;

      if (l_anchor) {
        l_anchor->next = s_list;
        s_list->prev = l_anchor;
      }
      if (r_anchor) {
        tail->next = r_anchor;
        r_anchor->prev = tail;
      }
    } else {
    }
    s_rover = save_next;
  }

  return rv;
}

void Analyzer::ExtractVariables(SEMANTICS_NODE * s_tree)
{
  SEMANTICS_NODE *s_rover = s_tree;
  while (s_rover) {
    if (s_rover->semantic_type == SEM_TYP_VARIABLE) {
      if (s_rover->contents) {
        size_t zln = strlen(s_rover->contents);
        if (zln) {
          char *src_name = TCI_NEW(char[zln + 1]);
          strcpy(src_name, s_rover->contents);
          IMPLDIFF_func_names =
            AppendFuncName(IMPLDIFF_func_names, NULL, src_name);
        }
      }
    } else if (s_rover->bucket_list) {
      BUCKET_REC *b_rover = s_rover->bucket_list;
      while (b_rover) {
        SEMANTICS_NODE *s_list = b_rover->first_child;
        if (s_list) {
          ExtractVariables(s_list);
        }
        b_rover = b_rover->next;
      }
    }

    s_rover = s_rover->next;
  }

}

void Analyzer::ConvertToPIECEWISElist(SEMANTICS_NODE * s_fence)
{ 
  if (s_fence && s_fence->bucket_list && s_fence->bucket_list->first_child) {
    SEMANTICS_NODE *s_matrix = s_fence->bucket_list->first_child;

    if (s_matrix->semantic_type == SEM_TYP_TABULATION) {
      U32 nrows = s_matrix->nrows;
      U32 ncols = s_matrix->ncols;

      if (ncols == 2 || ncols == 3) {
        BUCKET_REC *cell_list = s_matrix->bucket_list;
        BUCKET_REC *b_list = NULL;

        int row_tally = 0;
        while (row_tally < nrows) {
          SEMANTICS_NODE *s_expression = NULL;
          SEMANTICS_NODE *s_domain = NULL;

          if (LocatePieces(cell_list, row_tally, ncols,
                           &s_expression, &s_domain)) {
            BUCKET_REC *new_b = MakeBucketRec(MB_UNNAMED, NULL);
            b_list = AppendBucketRec(b_list, new_b);
            SEMANTICS_NODE *s_onepiece =
              CreateOnePiece(s_expression, s_domain);
            new_b->first_child = s_onepiece;
            s_onepiece->parent = new_b;
          }
          row_tally++;
        }
        DisposeBucketList(s_fence->bucket_list);

        s_fence->bucket_list = b_list;
        s_fence->semantic_type = SEM_TYP_PIECEWISE_LIST;
      } else
        TCI_ASSERT(0);
    }
  } else
    TCI_ASSERT(0);
}

SEMANTICS_NODE *Analyzer::CreateOnePiece(SEMANTICS_NODE * s_expression,
                                         SEMANTICS_NODE * s_domain)
{
  SEMANTICS_NODE *rv = CreateSemanticsNode();
  rv->semantic_type = SEM_TYP_ONE_PIECE;

  BUCKET_REC *b_list = MakeBucketRec(MB_PIECE_EXPR, rv);
  b_list->first_child = s_expression;
  s_expression->parent = b_list;

  BUCKET_REC *new_b = MakeBucketRec(MB_UNNAMED, rv);
  b_list = AppendBucketRec(b_list, new_b);
  s_domain = PrefixToInfix(s_domain);
  new_b->first_child = s_domain;
  s_domain->parent = new_b;

  rv->bucket_list = b_list;

  return rv;
}

bool Analyzer::LocatePieces(BUCKET_REC * cell_list,
                                U32 row_tally, U32 ncols,
                                SEMANTICS_NODE ** s_expression,
                                SEMANTICS_NODE ** s_domain)
{
  bool rv = false;

  U32 targ = ncols * row_tally;
  BUCKET_REC *b_rover = cell_list;
  U32 tally = 0;
  while (b_rover && tally < targ) {
    tally++;
    b_rover = b_rover->next;
  }

  if (b_rover) {
    *s_expression = b_rover->first_child;
    if (b_rover->first_child)
      b_rover->first_child = NULL;
    (*s_expression)->parent = NULL;
    b_rover = b_rover->next;
    if (b_rover) {
      if (ncols == 3)
        b_rover = b_rover->next;
      if (b_rover) {
        *s_domain = b_rover->first_child;
        if (b_rover->first_child)
          b_rover->first_child = NULL;
        (*s_domain)->parent = NULL;
        rv = true;
      }
    }
  }
  return rv;
}

MNODE *Analyzer::LocateOperator(MNODE * mml_list, OpIlk &op_ilk, int & advance)
{
  MNODE *rv = NULL;
  op_ilk = OP_none;
  advance = 0;

  MNODE *rover = mml_list;
  while (rover) {
    const char *mml_element = rover->src_tok;
    size_t ln = strlen(mml_element);

    bool embellished = true;
    MNODE *key = NULL;
    switch (ln) {
    case 2:
      if (!strcmp(mml_element, "mi")) {

      } else if (!strcmp(mml_element, "mo")) {
        key = rover;
        embellished = false;
      }
      break;
    case 4:
      if (!strcmp(mml_element, "msup")) {
        key = rover->first_kid;
      } else if (!strcmp(mml_element, "msub")) {
        key = rover->first_kid;
      }
      break;
    case 5:
      if (!strcmp(mml_element, "mover")) {
        key = rover->first_kid;
      }
      break;
    case 6:
      if (!strcmp(mml_element, "munder")) {
        key = rover->first_kid;
      }
      break;
    case 7:
      if (!strcmp(mml_element, "msubsup")) {
        key = rover->first_kid;
      }
      break;
    case 10:
      if (!strcmp(mml_element, "munderover")) {
        key = rover->first_kid;
      }
      break;
    default:
      break;
    }

    // Check the current node for the target attribute
    if (key && !strcmp(key->src_tok, "mo")) {
      const char *attr_val = GetATTRIBvalue(key->attrib_list, "form");
      if (attr_val)
        op_ilk = StringToOpIlk(attr_val);
      else
        op_ilk = OP_infix;

      rv = rover;
      if (embellished && op_ilk != OP_prefix)
        TCI_ASSERT(0);
      break;
    }
    advance++;
    rover = rover->next;
  }
  return rv;
}

void Analyzer::FenceToMatrix(SEMANTICS_NODE * operand)
{
  if (operand->semantic_type == SEM_TYP_PARENED_LIST
      || operand->semantic_type == SEM_TYP_BRACKETED_LIST) {
    int n_rows = 1;
    int n_cols = 0;
    BUCKET_REC *brover = operand->bucket_list;
    while (brover) {
      n_cols++;
      brover = brover->next;
    }

    if (n_cols > 1) {
      operand->semantic_type = SEM_TYP_TABULATION;
      operand->nrows = n_rows;
      operand->ncols = n_cols;

      delete[] operand->contents;
      char *tmp = TCI_NEW(char[8]);
      strcpy(tmp, "matrix");
      operand->contents = tmp;
    }
  }
}

void Analyzer::FenceToInterval(SEMANTICS_NODE * s_fence)
{
  SemanticVariant interval_type = SNV_None;
  if (s_fence->semantic_type == SEM_TYP_PARENED_LIST) {
    interval_type = SNV_ExclExclInterval;
  } else if (s_fence->semantic_type == SEM_TYP_BRACKETED_LIST) {
    interval_type = SNV_InclInclInterval;
  } else if (s_fence->semantic_type == SEM_TYP_INTERVAL) {
    interval_type = s_fence->variant;
  } else if (s_fence->semantic_type == SEM_TYP_SET ||
             s_fence->semantic_type == SEM_TYP_GENERIC_FENCE ||
             s_fence->semantic_type == SEM_TYP_VARIABLE) {
    return;
  } else {
    TCI_ASSERT(!"Unexpected semantic type on fence.");
  }

  int n_rows = 1;
  int n_cols = 0;
  BUCKET_REC *brover = s_fence->bucket_list;
  while (brover) {
    n_cols++;
    brover = brover->next;
  }

  if (n_cols == 2) {
    s_fence->semantic_type = SEM_TYP_INTERVAL;
    s_fence->variant = interval_type;

    delete[] s_fence->contents;
    char *tmp = TCI_NEW(char[9]);
    strcpy(tmp, "interval");
    s_fence->contents = tmp;

    BUCKET_REC *brover = s_fence->bucket_list;
    brover->bucket_ID = MB_INTERVAL_START;
    brover = brover->next;
    brover->bucket_ID = MB_INTERVAL_END;
  }
}

// Function to decide if an operator may take matrix args
//  or interval args

Analyzer::OpMatrixIntervalType Analyzer::GetOpType(MNODE * mo)
{
  OpMatrixIntervalType rv = OMI_none;

  if (mo && mo->p_chdata) {
    char *ptr = strstr(mo->p_chdata, "&#x");
    if (ptr) {
      U32 unicode = ASCII2U32(ptr + 3, 16);
      if (unicode == 0x2212     // &minus;
          || unicode == 0xd7    // &times;
          || unicode == 0x22c5) { // DOT PRODUCT
        rv = OMI_matrix;
      } else if (unicode == 0x2208  // &elem;
                 || unicode == 0x220a  // &elem;
                 || unicode == 0x2229  // &cap;
                 || unicode == 0x222a) {  // &cup;
        rv = OMI_interval;
      }
    } else {
      size_t zln = strlen(mo->p_chdata);
      if (zln == 1) {
        char ch = mo->p_chdata[0];
        if (ch == '+')
          rv = OMI_matrix;
      }
    }
  }

  return rv;
}

bool Analyzer::IsApplyFunction(MNODE * next_elem)
{
  bool rv = false;

  const char *next_elem_nom = next_elem->src_tok;
  if (!strcmp(next_elem_nom, "mo")) {
    char *ptr = strstr(next_elem->p_chdata, "&#x");
    if (ptr) {
      U32 unicode = ASCII2U32(ptr + 3, 16);
      if (unicode == 0x2061)    // &Applyfunction;
        rv = true;
    }
  }
  return rv;
}

// The following must be called when the function "\log" is
//  encountered and an explicit base is not given as a subscript

void Analyzer::AddDefaultBaseToLOG(SEMANTICS_NODE * snode)
{
  BUCKET_REC *bucket = MakeBucketRec(MB_LOG_BASE, NULL);
  snode->bucket_list = AppendBucketRec(snode->bucket_list, bucket);

  SEMANTICS_NODE *log_base = CreateSemanticsNode();
  bucket->first_child = log_base;
  log_base->parent = bucket;

  char *ptr;
  if (log_is_base10) {
    ptr = "10";
    log_base->semantic_type = SEM_TYP_NUMBER;
  } else {
    ptr = "&#x2147;";
    log_base->semantic_type = SEM_TYP_UCONSTANT;
  }

  size_t zln = strlen(ptr);
  char *tmp = TCI_NEW(char[zln + 1]);
  strcpy(tmp, ptr);
  log_base->contents = tmp;
}

// In the course of analyzing an ODE or PDE, the function we're solving
//  for and the independent variable(s) are decided.  Here we add that
//  info to the semantic tree that we're building.

void Analyzer::AppendODEfuncs(SEMANTICS_NODE * rv, DE_FUNC_REC * ODE_fnames)
{
  DE_FUNC_REC *rover = ODE_fnames;
  while (rover) {
    SEMANTICS_NODE *s_odefunc = CreateSemanticsNode();
    s_odefunc->semantic_type = SEM_TYP_FUNCTION;

    if (rover->zfunc_src_name) {
      size_t ln = strlen(rover->zfunc_src_name);
      char *tmp = TCI_NEW(char[ln + 1]);
      strcpy(tmp, rover->zfunc_src_name);
      s_odefunc->contents = tmp;
    } else
      TCI_ASSERT(0);

    if (rover->zfunc_canon_name) {
      size_t ln = strlen(rover->zfunc_canon_name);
      char *tmp = TCI_NEW(char[ln + 1]);
      strcpy(tmp, rover->zfunc_canon_name);
      s_odefunc->canonical_ID = tmp;
      SetSnodeOwner(s_odefunc);
    } else
      TCI_ASSERT(0);

    if (DE_ind_vars) {
      BUCKET_REC *arg_bucket = AddVarToBucket(MB_UNNAMED, DE_ind_vars);
      s_odefunc->bucket_list =
        AppendBucketRec(s_odefunc->bucket_list, arg_bucket);
    } else
      TCI_ASSERT(0);

    BUCKET_REC *bucket = MakeBucketRec(MB_ODEFUNC, s_odefunc);
    s_odefunc->parent = bucket;
    rv->bucket_list = AppendBucketRec(rv->bucket_list, bucket);

    rover = rover->next;
  }
}

// Need to complete the following - find an unused var.

void Analyzer::ChooseIndVar(MNODE * dMML_tree, char *buffer)
{
  strcpy(buffer, "t");
}

// Locate the "dx".
// Note that the "mrow" passed into this function
//  may be the numerator of a fractional integrand.
// The function isn't recursive - I think the "dx"
//   should be the entire numerator as in "dx/x" or
//   should be a factor of the numerator as in "2xdx/sin(x)"

MNODE *Analyzer::Find_dx(MNODE * num_mrow, bool & is_nested)
{
  MNODE *rv = NULL;
  is_nested = false;

  if (num_mrow && !strcmp(num_mrow->src_tok, "mrow")) {
    MNODE *m_child = num_mrow->first_kid;
    if (m_child && !strcmp(m_child->src_tok, "mo")
        && !strcmp(m_child->p_chdata, "&#x2146;")) {  // "d"
      rv = num_mrow;
    } else if (m_child) {
      MNODE *m_rover = m_child;
      while (m_rover) {
        if (!strcmp(m_rover->src_tok, "mrow")) {
          MNODE *m_child2 = m_rover->first_kid;
          if (m_child2 && !strcmp(m_child2->src_tok, "mo")
              && !strcmp(m_child2->p_chdata, "&#x2146;")) { // "d"
            rv = m_rover;
            is_nested = true;
            break;
          }
        }
        m_rover = m_rover->next;
      }
    } else
      TCI_ASSERT(0);
  } else
    TCI_ASSERT(0);

  return rv;
}

void Analyzer::Patchdx(SEMANTICS_NODE * s_frac)
{
  if (s_frac
      && s_frac->semantic_type == SEM_TYP_FRACTION
      && s_frac->bucket_list && s_frac->bucket_list->first_child) {
    BUCKET_REC *b_num = FindBucketRec(s_frac->bucket_list, MB_NUMERATOR);
    if (b_num && b_num->first_child) {
      SEMANTICS_NODE *s_num = b_num->first_child;

      if (s_num->semantic_type == SEM_TYP_INFIX_OP && s_num->bucket_list) {
        BUCKET_REC *b_rover = s_num->bucket_list;
        while (b_rover) {
          SEMANTICS_NODE *s_operand = b_rover->first_child;
          if (s_operand
              && s_operand->semantic_type == SEM_TYP_PRECEDENCE_GROUP
              && s_operand->bucket_list) {
            SEMANTICS_NODE *s_nest = s_operand->bucket_list->first_child;
            if (s_nest
                && s_nest->semantic_type == SEM_TYP_PREFIX_OP
                && !strcmp(s_nest->contents, "&#x2146;")) {
              s_num = s_nest;
              break;
            }
          }
          b_rover = b_rover->next;
        }
      }

      if (s_num
          && s_num->semantic_type == SEM_TYP_PREFIX_OP
          && !strcmp(s_num->contents, "&#x2146;")) {
        delete[] s_num->contents;
        char *tmp = TCI_NEW(char[2]);
        strcpy(tmp, "1");
        s_num->contents = tmp;

        s_num->semantic_type = SEM_TYP_NUMBER;
        if (s_num->bucket_list) {
          DisposeBucketList(s_num->bucket_list);
          s_num->bucket_list = NULL;
        }
      } else
        TCI_ASSERT(0);
    } else
      TCI_ASSERT(0);
  } else
    TCI_ASSERT(0);
}

SEMANTICS_NODE *Analyzer::ExtractIndepVar(MNODE * rover)
{
  SEMANTICS_NODE *s_local = NULL;

  if (!strcmp(rover->src_tok, "mi")) {
    s_local = CreateSemanticsNode();
    int nodes_done;
    AnalyzeMI(rover, s_local, nodes_done, false);
  } else if (!strcmp(rover->src_tok, "msup")) {
    MNODE *base = rover->first_kid;
    s_local = CreateSemanticsNode();
    BUCKET_REC *base_bucket = MakeBucketRec(MB_SCRIPT_BASE, NULL);
    s_local->bucket_list = AppendBucketRec(s_local->bucket_list, base_bucket);
    SEMANTICS_NODE *s_base = GetSemanticsFromNode(base, base_bucket);
    base_bucket->first_child = s_base;
    s_base->parent = base_bucket;

    BUCKET_REC *power_bucket = MakeBucketRec(MB_SCRIPT_UPPER, NULL);
    s_local->bucket_list =
      AppendBucketRec(s_local->bucket_list, power_bucket);
    SEMANTICS_NODE *s_power = GetSemanticsFromNode(base->next, power_bucket);
    power_bucket->first_child = s_power;
    s_power->parent = power_bucket;

    s_local->semantic_type = SEM_TYP_POWERFORM;
  } else if (!strcmp(rover->src_tok, "mo")
             && rover->p_chdata && !strcmp(rover->p_chdata, "&#x2062;")) {
  } else
    TCI_ASSERT(0);
  return s_local;
}

// Currently not used

bool Analyzer::IsVarInSLIST(SEMANTICS_NODE * s_var_list, char *var_nom)
{
  bool rv = false;

  SEMANTICS_NODE *sv_rover = s_var_list;
  while (sv_rover) {
    SEMANTICS_NODE *s_var = sv_rover;
    if (sv_rover->semantic_type == SEM_TYP_POWERFORM) {
      BUCKET_REC *bucket =
        FindBucketRec(sv_rover->bucket_list, MB_SCRIPT_BASE);
      s_var = bucket->first_child;
    }

    if (s_var->semantic_type == SEM_TYP_VARIABLE && s_var->contents) {
      if (!strcmp(var_nom, s_var->contents)) {
        rv = true;
        break;
      }
    }
    sv_rover = sv_rover->next;
  }
  return rv;
}

SEMANTICS_NODE *Analyzer::RemoveParens(SEMANTICS_NODE * s_operand)
{
  SEMANTICS_NODE *rv = s_operand;
  if (s_operand
      && !s_operand->next
      && s_operand->bucket_list && !s_operand->bucket_list->next) {
    if (s_operand->semantic_type == SEM_TYP_GENERIC_FENCE
        || s_operand->semantic_type == SEM_TYP_PRECEDENCE_GROUP) {
      rv = s_operand->bucket_list->first_child;
      s_operand->bucket_list->first_child = NULL;

      BUCKET_REC *parent_bucket = s_operand->parent;
      s_operand->parent = NULL;

      DisposeSList(s_operand);

      if (rv)
        rv->parent = parent_bucket;
      if (parent_bucket)
        parent_bucket->first_child = rv;
    }
  }
  return rv;
}

bool Analyzer::IsArgDelimitingFence(MNODE * mml_node)
{
  bool rv = false;

  if (!strcmp(mml_node->src_tok, "mfenced")) {
    U32 l_unicode = '(';
    U32 r_unicode = ')';
    if (mml_node->attrib_list) {
      const char *open_value =
        GetATTRIBvalue(mml_node->attrib_list, "open");
      const char *close_value =
        GetATTRIBvalue(mml_node->attrib_list, "close");
      if (open_value) {
        if (*open_value == '&')
          l_unicode = ASCII2U32(open_value + 3, 16);
        else
          l_unicode = open_value[0];
      }
      if (close_value) {
        if (*close_value == '&')
          r_unicode = ASCII2U32(close_value + 3, 16);
        else
          r_unicode = close_value[0];
      }
    }
    if (l_unicode == '(' && r_unicode == ')')
      rv = true;
    else if (l_unicode == '[' && r_unicode == ']')
      rv = true;
  }
  return rv;
}

void Analyzer::CreateSubscriptedVar(MNODE * mml_msub_node,
                                    bool remove_super,
                                    SEMANTICS_NODE * snode)
{
  char *mml_canonical_name = NULL;
  if (remove_super) {
    char buffer[1024];
    buffer[0] = 0;
    strcat(buffer, "msub");

    MNODE *rover = mml_msub_node->first_kid;
    if (rover) {
      char *tmp = GetCanonicalIDforMathNode(rover);
      if (tmp) {
        strcat(buffer, tmp);
        delete[] tmp;
        tmp = GetCanonicalIDforMathNode(rover->next);
        if (tmp) {
          strcat(buffer, tmp);
          delete[] tmp;
        }
      }
    }
    size_t zln = strlen(buffer);
    mml_canonical_name = TCI_NEW(char[zln + 1]);
    strcpy(mml_canonical_name, buffer);
  } else
    mml_canonical_name = GetCanonicalIDforMathNode(mml_msub_node);
  if (!mml_canonical_name) {
    snode->error_flag = 1;
    return;
  }

  snode->canonical_ID = mml_canonical_name;
  SetSnodeOwner(snode);
  node_IDs_list = AppendIDRec(node_IDs_list, curr_client_ID,
                              mml_canonical_name, mml_msub_node, z_scr_str);
  snode->semantic_type = SEM_TYP_QUALIFIED_VAR;

  // Extra info - first the base variable
  MNODE *base = mml_msub_node->first_kid;
  MNODE *sub = base->next;

  SEMANTICS_NODE *s_var = CreateSemanticsNode();
  BUCKET_REC *bucket = MakeBucketRec(MB_BASE_VARIABLE, s_var);
  snode->bucket_list = AppendBucketRec(snode->bucket_list, bucket);
  s_var->parent = bucket;

  char *base_canonical_name = GetCanonicalIDforMathNode(base);
  if (!base_canonical_name) {
    snode->error_flag = 1;
    return;
  }
  s_var->canonical_ID = base_canonical_name;
  SetSnodeOwner(s_var);
  node_IDs_list = AppendIDRec(node_IDs_list, curr_client_ID,
                              base_canonical_name, base, z_scr_str);

  s_var->semantic_type = SEM_TYP_VARIABLE;

  size_t ln = strlen(base->p_chdata);
  char *tmp = TCI_NEW(char[ln + 1]);
  strcpy(tmp, base->p_chdata);
  s_var->contents = tmp;

  // Extra info - the subscripted qualifier
  SEMANTICS_NODE *s_cont = QualifierToSNODE(sub);
  BUCKET_REC *sub_bucket = MakeBucketRec(MB_SUB_QUALIFIER, s_cont);
  snode->bucket_list = AppendBucketRec(snode->bucket_list, sub_bucket);
  s_cont->parent = sub_bucket;
}

void Analyzer::CreatePowerForm(MNODE * mml_base, MNODE * mml_power,
                               SEMANTICS_NODE * snode)
{
  BUCKET_REC *base_bucket = MakeBucketRec(MB_SCRIPT_BASE, NULL);
  snode->bucket_list = AppendBucketRec(snode->bucket_list, base_bucket);
  SEMANTICS_NODE *s_base = GetSemanticsFromNode(mml_base, base_bucket);
  base_bucket->first_child = s_base;
  s_base->parent = base_bucket;

  BUCKET_REC *power_bucket = MakeBucketRec(MB_SCRIPT_UPPER, NULL);
  snode->bucket_list = AppendBucketRec(snode->bucket_list, power_bucket);
  SEMANTICS_NODE *s_power = GetSemanticsFromNode(mml_power, power_bucket);
  power_bucket->first_child = s_power;
  s_power->parent = power_bucket;

  snode->semantic_type = SEM_TYP_POWERFORM;
}

/* As a MathML tree is processed, nodes that must be given
  a canonical ID may be encountered.  We keep a list that maps
  a canonical ID back to the mml node.  The following 3 functions
  manage that list.
*/

MIC2MMLNODE_REC *Analyzer::AppendIDRec(MIC2MMLNODE_REC * node_IDs_list,
                                       U32 client_ID,
                                       char *obj_name, MNODE * mml_node,
                                       const char *src_markup)
{
  MIC2MMLNODE_REC *rv = node_IDs_list;

  if (obj_name && mml_node) {
    // obj_name may already be in the list
    if (node_IDs_list) {
      MIC2MMLNODE_REC *rover = node_IDs_list;
      while (rover) {
        if (rover->canonical_name && !strcmp(obj_name, rover->canonical_name))
          return rv;
        rover = rover->next;
      }
    }

    MIC2MMLNODE_REC *new_node = TCI_NEW(MIC2MMLNODE_REC);
    new_node->next = NULL;
    new_node->owner_ID = client_ID;

    size_t zln = strlen(obj_name);
    char *tmp = TCI_NEW(char[zln + 1]);
    strcpy(tmp, obj_name);
    new_node->canonical_name = tmp;

    new_node->mml_markup = TNodeToStr(mml_node, NULL, 0);
    // Append the new record to the global list
    if (!node_IDs_list)
      rv = new_node;
    else {
      MIC2MMLNODE_REC *rover = node_IDs_list;
      while (rover->next)
        rover = rover->next;
      rover->next = new_node;
    }
  } else
    TCI_ASSERT(0);

  return rv;
}

bool Analyzer::IsBesselFunc(MNODE * mml_msub_node)
{
  bool rv = false;

  if (mml_msub_node && mml_msub_node->first_kid) {
    MNODE *base = mml_msub_node->first_kid;
    MNODE *sub = base->next;
    if (sub) {
      const char *base_elem = base->src_tok;
      if (!strcmp(base_elem, "mi")) {
        const char *ptr = base->p_chdata;
        if (ptr && !strncmp(ptr, "Bessel", 6)) {
          rv = true;
        }
      }
    }
  }

  return rv;
}

void Analyzer::AnalyzeBesselFunc(MNODE * mml_msub_node,
                                 SEMANTICS_NODE * snode, int& nodes_done)
{
  nodes_done = 1;
  char *mml_canonical_name = GetCanonicalIDforMathNode(mml_msub_node);

  if (!mml_canonical_name) {
    TCI_ASSERT(0);
    snode->error_flag = 1;
    return;
  }

  snode->canonical_ID = mml_canonical_name;
  SetSnodeOwner(snode);
  node_IDs_list = AppendIDRec(node_IDs_list, curr_client_ID,
                              mml_canonical_name, mml_msub_node, z_scr_str);

  if (mml_msub_node && mml_msub_node->first_kid) {
    MNODE *base = mml_msub_node->first_kid;

    size_t ln = strlen(base->p_chdata);
    char *tmp = TCI_NEW(char[ln + 1]);
    strcpy(tmp, base->p_chdata);
    snode->contents = tmp;
    snode->semantic_type = SEM_TYP_FUNCTION;

    MNODE *sub = base->next;
    if (sub) {
      BUCKET_REC *sub_bucket = MakeBucketRec(MB_UNNAMED, NULL);
      snode->bucket_list = AppendBucketRec(snode->bucket_list, sub_bucket);
      SEMANTICS_NODE *s_sub = GetSemanticsFromNode(sub, sub_bucket);
      sub_bucket->first_child = s_sub;
      s_sub->parent = sub_bucket;
    }

    int local_nodes_done;
    BUCKET_REC *bucket = ArgsToBucket(mml_msub_node, local_nodes_done);
    nodes_done += local_nodes_done;

    snode->bucket_list = AppendBucketRec(snode->bucket_list, bucket);
  }
}

// Extract chars and unicodes from the ASCII string p_chdata.
// Returns a count of the symbols processed, up to "limit".

int Analyzer::ChData2Unicodes(const char *p_chdata, U32 * unicodes, int limit)
{
  int rv = 0;

  if (p_chdata && *p_chdata) {
    const char *ptr = p_chdata;
    while (*ptr && rv < limit) {
      if (*ptr == '&') {
        ptr++;
        if (*ptr == '#') {
          // numeric entity - &#x201a;, &#9876;. etc.
          ptr++;
          U32 place_val = 10;
          if (*ptr == 'x') {
            place_val = 16;
            ptr++;
          }
          unicodes[rv] = ASCII2U32(ptr, place_val);
          rv++;

          while (*ptr && *ptr != ';')
            ptr++;
        } else {
          // non-numeric entity - &theta;, &ApplyFunction;. etc.
          const char *entity = ptr - 1;
          while (*ptr && *ptr != ';')
            ptr++;
          U32 ID, subID;
          size_t zln = ptr - entity + 1;
          const char *p_data;
          if (mml_entities->
              GetRecordFromName("MATH", entity, zln, ID, subID, &p_data)) {
            if (p_data && *p_data) {
              //&ApplyFunction;<uID3.5.6>infix,65,U02061
              char *ptr = strstr(p_data, ",U");
              if (ptr) {
                unicodes[rv] = ASCII2U32(ptr + 2, 16);
                rv++;
              } else {
                TCI_ASSERT(0);
              }
            } else {
              TCI_ASSERT(0);
            }
          } else {
            while (entity <= ptr) {
              unicodes[rv] = *entity;
              rv++;
              entity++;
            }
          }
        }
      } else {
        unicodes[rv] = *ptr;
        rv++;
      }
      ptr++;
    }
  }
  return rv;
}

bool Analyzer::SetODEvars(MathServiceRequest & msr, MathResult & mr,
                              MNODE * dMML_tree, U32 UI_cmd_ID)
{
  U32 p_type;
  U32 p_ID;
  if (UI_cmd_ID == CCID_Solve_ODE_Series)
    p_ID = PID_ODEIndepVar;
  else
    p_ID = PID_independentvar;

  const char *i_var = msr.GetParam(p_ID, p_type);

  if (i_var) {
    if (p_type == zPT_ASCII_text) {
      // i_var entered as "x"
      TCI_ASSERT(0);
      DE_ind_vars = CreateSemanticsNode();
      DE_ind_vars->semantic_type = SEM_TYP_VARIABLE;

      size_t zln = strlen(i_var);
      char *tmp = TCI_NEW(char[zln + 1]);
      strcpy(tmp, i_var);
      DE_ind_vars->contents = tmp;
    } else if (p_type == zPT_ASCII_mmlmarkup) {
      // If "i_var" comes from a chambase dialog (as it should)
      //   it will contain something like "<mi>x</mi>"
      DE_ind_vars = CreateSTreeFromMML(i_var);
    } else {
      TCI_ASSERT(0);
    }
    mr.PutResultCode(CR_undefined);
  } else {
    DE_ind_vars = DetermineIndepVar(dMML_tree);

    if (!DE_ind_vars) {
      if (UI_cmd_ID == CCID_Solve_ODE_Numeric) {
        char buffer[80];
        ChooseIndVar(dMML_tree, buffer);

        DE_ind_vars = CreateSemanticsNode();
        DE_ind_vars->semantic_type = SEM_TYP_VARIABLE;
        size_t zln = strlen(buffer);
        char *tmp = TCI_NEW(char[zln + 1]);
        strcpy(tmp, buffer);
        DE_ind_vars->contents = tmp;

        mr.PutResultCode(CR_undefined);
      } else {
        mr.PutResultCode(CR_queryindepvar);
        return false;
      }
    }
  }

  // Identify the function we're solving for in the ODE.
  DetermineODEFuncNames(dMML_tree);

  return true;
}

bool Analyzer::SetIMPLICITvars(MathServiceRequest & msr, MathResult & mr)
{
  bool rv = true;

  // Parameterized command - "variable of differentiation"
  U32 p_type;
  const char *i_var = msr.GetParam(PID_ImplDiffIndepVar, p_type);
  if (i_var) {
    if (p_type == zPT_ASCII_text) {
      // i_var entered as "x"
      TCI_ASSERT(0);
      TCI_ASSERT(!IMPLDIFF_ind_var);  // global var - must be NULL here!
      IMPLDIFF_ind_var = CreateSemanticsNode();
      IMPLDIFF_ind_var->semantic_type = SEM_TYP_VARIABLE;

      size_t zln = strlen(i_var);
      char *tmp = TCI_NEW(char[zln + 1]);
      strcpy(tmp, i_var);
      IMPLDIFF_ind_var->contents = tmp;
    } else if (p_type == zPT_ASCII_mmlmarkup) {
      // If "i_var" comes from a chambase dialog (as it should)
      //   it will contain something like "<mi>x</mi>"
      IMPLDIFF_ind_var = CreateSTreeFromMML(i_var);
    } else
      TCI_ASSERT(0);

    // Parameterized command - "list of dependent variables"
    // For now, I'm only handling a comma separated list of letters.
    const char *d_vars = msr.GetParam(PID_ImplDiffDepVars, p_type);
    if (p_type == zPT_ASCII_text) {
      TCI_ASSERT(0);
    } else if (p_type == zPT_ASCII_mmlmarkup) {
      SEMANTICS_NODE *s_tree = CreateSTreeFromMML(d_vars);
      if (s_tree) {
        ExtractVariables(s_tree);
        DisposeSList(s_tree);
      } else
        TCI_ASSERT(0);
    } else {
      TCI_ASSERT(0);
      rv = false;
    }
  } else {
    TCI_ASSERT(0);
    rv = false;
  }
  return rv;
}

// Semantic nodes that can be defined ( variables and functions )
//  must carry the ID of their owning computation client.
// Note that CompEngine must override "snode->owner_ID"
//  for the snode on the left of a new definition.

void Analyzer::SetSnodeOwner(SEMANTICS_NODE * snode)
{
  if (snode
      && snode->canonical_ID
      && defstore && curr_client_ID && curr_engine_ID) {
    DefInfo *di = defstore->GetDefInfo(curr_engine_ID,
                                       snode->canonical_ID);
    if (di)
      snode->owner_ID = di->owner_ID;
    else
      snode->owner_ID = curr_client_ID;
  } else {
    TCI_ASSERT(0);
  }
}

Analyzer::AccentType Analyzer::GetAboveType(BaseType base_type, MNODE * accent)
{
  AccentType rv = OT_NONE;

  if (accent && !strcmp(accent->src_tok, "mo")) {
    U32 unicodes[8];
    int content_tally = ChData2Unicodes(accent->p_chdata, unicodes, 8);
    if (content_tally == 1) {
      switch (unicodes[0]) {
      case 0x0302:             // &Hat;    
        rv = OT_HAT;
        break;
      case 0x02C7:             // &caron;  
        rv = OT_CHECK;
        break;
      case 0x02DC:             // &tilde;  
        rv = OT_TILDE;
        break;
      case 0x00B4:             // &acute;  
        rv = OT_ACUTE;
        break;
      case 0x0060:             // &grave;  
        rv = OT_GRAVE;
        break;
      case 0x02D8:             // &breve;  
        rv = OT_BREVE;
        break;
      case 0x00AF:             // &macr;   
        rv = OT_BAR;
        break;
      case 0x02DA:             // &ring;   
        rv = OT_MATHRING;
        break;
      case 0x02D9:             // &dot;    
        rv = OT_DOT;
        break;
      case 0x00A8:             // &die;    
        rv = OT_DDOT;
        break;
      case 0x20DB:             // &tdot;   
        rv = OT_DDDOT;
        break;
      case 0x20DC:             // &DotDot; 
        rv = OT_DDDDOT;
        break;
      case 0x20D7:             // &#x20D7; 
        rv = OT_VEC;
        break;
      default:
        TCI_ASSERT(0);
        break;
      }
    } else
      TCI_ASSERT(0);
  } else
    TCI_ASSERT(0);

  return rv;
}

//Analyzer::AccentType Analyzer::GetUnderType(BaseType base_type, MNODE * accent)
//{
//  AccentType rv = OT_NONE;
//
//  if (accent && !strcmp(accent->src_tok, "mo")) {
//    U32 unicodes[8];
//    int content_tally = ChData2Unicodes(accent->p_chdata, unicodes, 8);
//    if (content_tally == 1) {
//      switch (unicodes[0]) {
//      case 0x0302:             // &Hat;    
//      //        rv  =  OT_HAT;        break;
//      case 0x0332:             // underset bar    
//      //        rv  =  OT_HAT;        break;
//      case 0x2190:             // underset left arrow    
//      //        rv  =  OT_HAT;        break;
//      case 0x2192:             // underset right arrow    
//      //        rv  =  OT_HAT;        break;
//      default:
//        TCI_ASSERT(0);
//        break;
//      }
//    } else
//      TCI_ASSERT(0);
//  } else {
//  }
//
//  return rv;
//}

SEMANTICS_NODE *Analyzer::CreateSTreeFromMML(const char *mml_str)
{
  SEMANTICS_NODE *rv = NULL;

  if (mml_str) {
    MNODE *var = mml_tree_gen->MMLstr2Tree(mml_str);
    if (var) {
      int nodes_done = 0;
      rv = SNodeFromMNodes(var, nodes_done, false);
      DisposeTNode(var);
    } else
      TCI_ASSERT(0);
  }

  return rv;
}

void Analyzer::MSUB2FuncCall(MNODE * mml_msub_node, SEMANTICS_NODE * snode)
{
  MNODE *base = mml_msub_node->first_kid;

  if (!strcmp(base->src_tok, "mi")) {
    int nodes_done;
    AnalyzeMI(base, snode, nodes_done, false);
    if (snode->semantic_type == SEM_TYP_VARIABLE) {
      snode->semantic_type = SEM_TYP_FUNCTION;
      // process the argument
      int local_nodes_done;
      BUCKET_REC *br = ArgsToBucket(base, local_nodes_done);
      if (br)
        snode->bucket_list = AppendBucketRec(snode->bucket_list, br);
    }
  } else {
    TCI_ASSERT(!"Expected mi in base.");
  }
}

void Analyzer::OverridePrefsOnLHS(MNODE * dMML_tree)
{
  if (dMML_tree) {
    MNODE *m_rover = dMML_tree;
    if (!strcmp(dMML_tree->src_tok, "math"))
      m_rover = dMML_tree->first_kid;

    if (!strcmp(m_rover->src_tok, "mrow")
        && !m_rover->next)
      m_rover = m_rover->first_kid;

    if (m_rover && !strcmp(m_rover->src_tok, "mi")
        && m_rover->next && !strcmp(m_rover->next->src_tok, "mo")) {
      const char *src_token = m_rover->p_chdata;
      if (!strcmp(src_token, "i"))
        i_is_imaginary = false;
      else if (!strcmp(src_token, "j"))
        j_is_imaginary = false;
      else if (!strcmp(src_token, "e"))
        e_is_Euler = false;
    }
  }
}

// Sometimes, Fixup will introduce InvisibleTimes when what was really meant is ApplyFunction.
// (But Fixup doesn't know the context so it actually did the right thing.)
void Analyzer::OverrideInvisibleTimesOnLHS(MNODE * dMML_tree)
{
  if (dMML_tree) {
    MNODE *m_rover = dMML_tree;
    if (!strcmp(dMML_tree->src_tok, "math"))
      m_rover = dMML_tree->first_kid;

    if (!strcmp(m_rover->src_tok, "mrow") && !m_rover->next)
      m_rover = m_rover->first_kid;

    if (!strcmp(m_rover->src_tok, "mrow"))
      m_rover = m_rover->first_kid;

    //TODO deal with embellished functions (non-SWP feature)
    if (m_rover && !strcmp(m_rover->src_tok, "mi")
        && m_rover->next && !strcmp(m_rover->next->src_tok, "mo")) {
      const char *src_token = m_rover->next->p_chdata;
      if (src_token && !strcmp(src_token, "&#x2062;") && m_rover->next->next) {
        if (IsArgDelimitingFence(m_rover->next->next)) {
          //super ugly, but what else to do?
          delete m_rover->next->p_chdata;
          char *tmp = TCI_NEW(char[9]);
          strcpy(tmp,"&#x2061;");  // ApplyFunction
          m_rover->next->p_chdata = tmp;
        }
      }
    }
  }
}

void Analyzer::CreateSubstBucket(MNODE * subst, SEMANTICS_NODE * snode,
                                 bool is_lower)
{
  U32 b_ID = is_lower ? MB_SUBST_LOWER : MB_SUBST_UPPER;

  if (subst) {
    bool is_list = false;
    MNODE *m_rover = subst;
    if (!strcmp(m_rover->src_tok, "mrow")) {
      MNODE *child1 = m_rover->first_kid;
      if (child1
          && child1->next
          && child1->next->p_chdata && !strcmp(child1->next->p_chdata, ",")) {
        m_rover = child1;
        is_list = true;
      }
    }

    if (is_list) {
      while (m_rover) {
        BUCKET_REC *subst_bucket = MakeBucketRec(b_ID, NULL);
        snode->bucket_list =
          AppendBucketRec(snode->bucket_list, subst_bucket);
        SEMANTICS_NODE *sl_subst =
          GetSemanticsFromNode(m_rover, subst_bucket);
        subst_bucket->first_child = sl_subst;
        sl_subst->parent = subst_bucket;

        if (m_rover->next) {
          m_rover = m_rover->next;
          if (!strcmp(m_rover->src_tok, "mo")
              && !strcmp(m_rover->p_chdata, ",")) {
            m_rover = m_rover->next;
          } else
            break;
        } else
          break;
      }
    } else {
      BUCKET_REC *subst_bucket = MakeBucketRec(b_ID, NULL);
      snode->bucket_list = AppendBucketRec(snode->bucket_list, subst_bucket);
      SEMANTICS_NODE *sl_subst = GetSemanticsFromNode(m_rover, subst_bucket);
      subst_bucket->first_child = sl_subst;
      sl_subst->parent = subst_bucket;
    }
  }
}

int Analyzer::GetVarLimType(char *op_name, MNODE * base)
{
  int rv = 0;

  if (!strcmp(op_name, "lim")
      && base && base->next && base->next->p_chdata) {
    MNODE *under_decoration = base->next;
    const char *decor = under_decoration->p_chdata;
    char *ptr = strstr(decor, "&#x");
    if (ptr) {
      U32 unicode = ASCII2U32(ptr + 3, 16);
      switch (unicode) {
      case 0x2192:             //&rarr;
        rv = 1;
        break;
      case 0x0332:             //&underbar;
        rv = 2;
        break;
      case 0x2190:             //&larr;
        rv = 3;
        break;
      case 0xaf:               //&overbar;
        rv = 4;
        break;
      default:
        break;
      }
    }
  }

  return rv;
}

SEMANTICS_NODE *Analyzer::DefToSemanticsList(MNODE * dMML_tree,
                                             int& error_code)
{
  SEMANTICS_NODE *rv = NULL;
  error_code = 0;

  OverridePrefsOnLHS(dMML_tree);
  OverrideInvisibleTimesOnLHS(dMML_tree);

  const char *mml_element = dMML_tree->src_tok;

  if (!strcmp(mml_element, "math")) {
    rv = CreateSemanticsNode();

    rv->semantic_type = SEM_TYP_MATH_CONTAINER;
    if (dMML_tree->first_kid) {
      MNODE *cont = dMML_tree->first_kid;
      // descend into a redundant mrow, if it exists
      while (cont && !cont->next && !strcmp(cont->src_tok, "mrow"))
        cont = cont->first_kid;

      if (cont) {
        BUCKET_REC *new_a_rec = MakeBucketRec(MB_UNNAMED, NULL);
        rv->bucket_list = AppendBucketRec(NULL, new_a_rec);

        SEMANTICS_NODE *s_node =
          GetDefSList(cont, new_a_rec, ALL_NODES, error_code);
        if (!error_code) {
          new_a_rec->first_child = s_node;
          s_node->parent = new_a_rec;
        }
      } else {
        error_code = 1;
      }
    } else {
      error_code = 1;
    }
  } else {
    TCI_ASSERT(!"Not in math.");
    error_code = 1;
  }

  return rv;
}

SEMANTICS_NODE *Analyzer::GetDefSList(MNODE * dMML_list,
                                      BUCKET_REC * parent_bucket,
                                      int mml_node_lim, int& error_code)
{
  SEMANTICS_NODE *head = NULL;
  SEMANTICS_NODE *tail;

  int mml_nodes_done = 0;
  MNODE *rover = dMML_list;
  while (rover && mml_nodes_done < mml_node_lim && !error_code) {
    SEMANTICS_NODE *new_node = NULL;
    // Look ahead in the MML list for an operator.
    OpIlk op_ilk;
    int advance;
    MNODE *mo = LocateOperator(rover, op_ilk, advance);
    bool opIsAF = false;
    if (mo && mo->p_chdata && !strcmp(mo->p_chdata,"&#x2061;"))
      opIsAF = true;

    int mml_nodes_remaining = mml_node_lim - mml_nodes_done;
    if (mo && mml_nodes_remaining > 1  && !opIsAF) {
      if (mo->p_chdata
          && (!strcmp(mo->p_chdata, "=") || !strcmp(mo->p_chdata, ":="))) {
        SEMANTICS_NODE *l_operand = NULL;
        SEMANTICS_NODE *r_operand = NULL;

        if (op_ilk == OP_infix || op_ilk == OP_postfix) {
          if (head) {
            l_operand = head;
            // Here we're assuming ALL operators in source MML list have
            //  the same precedence (ie. well-formed MML), and are left-associative.
          } else {
            // translate the left operand
            int l_nodes_done = 0;
            l_operand = SNodeFromMNodes(rover, l_nodes_done, true);
            while (l_nodes_done) {
              mml_nodes_done++;
              rover = rover->next;
              l_nodes_done--;
            }
          }
        }
        // translate the operator
        int op_nodes_done = 0;
        new_node = SNodeFromMNodes(rover, op_nodes_done, false);
        while (op_nodes_done) {
          mml_nodes_done++;
          rover = rover->next;
          op_nodes_done--;
        }
        if (op_ilk == OP_infix) {
          // translate the right operand
          int r_nodes_done = 0;
          r_operand = SNodeFromMNodes(rover, r_nodes_done, false);
          while (r_nodes_done) {
            mml_nodes_done++;
            rover = rover->next;
            r_nodes_done--;
          }
          // We can re-work operands here as required.
          // For example, in (1,2,3)x(3,2,1) the parened fences
          // probably represent vectors ( 1 row matrices ).
          OpMatrixIntervalType op_type = GetOpType(mo);
          if (op_type == OMI_matrix) {
            if (l_operand)
              FenceToMatrix(l_operand);
            if (r_operand)
              FenceToMatrix(r_operand);
          } else if (op_type == OMI_interval) {
            if (r_operand)
              FenceToInterval(r_operand);
            if (l_operand)
              FenceToInterval(l_operand);
          }
          CreatePrefixForm(new_node, l_operand, r_operand);
        } else {
          error_code = 1;
        }
      } else {
        error_code = 1;
      }
      if (new_node) {
        head = new_node;
        tail = new_node;
        while (tail->next) {
          tail->parent = parent_bucket;
          tail = tail->next;
        }
        tail->parent = parent_bucket;
      }
    } else {
      // In this case, there are no operators in the source MML list
      //  - probably a single node.
      int nodes_done = 0;
      new_node = SNodeFromMNodes(rover, nodes_done, true);
      // Advance thru the source list as required
      while (nodes_done) {
        mml_nodes_done++;
        rover = rover->next;
        nodes_done--;
      }
      // Append new_node to the list we're building
      if (new_node) {
        if (head) {
          tail->next = new_node;
          new_node->prev = tail;
        } else {
          head = new_node;
        }
        tail = new_node;
        while (tail->next) {
          tail->parent = parent_bucket;
          tail = tail->next;
        }
        tail->parent = parent_bucket;
      }
    }
  }
  return head;
}
