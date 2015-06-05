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

#include "Analyzer.h"
#include "Grammar.h"
#include "MML2Tree.h"
#include "Tree2StdMML.h"
#include "DefStore.h"
#include "PrefStor.h"
#include "attriblist.h"
#include "mnode.h"
#include "strutils.h"
#include "DefInfo.h"
#include "MResult.h"

#include <string.h>
#include <ctype.h>

#ifdef XP_WIN
#define _tcistricmp  stricmp
#define _tcistrnicmp _strnicmp
#else
#define _tcistricmp  strcasecmp
#define _tcistrnicmp _strncasecmp
#endif


static const int ALL_NODES = INT_MAX;








SEMANTICS_NODE* GetSemanticsFromNode(MNODE* mml_node, BUCKET_REC* bucket, Analyzer* pAnalyzer);


SEMANTICS_NODE* GetSemanticsList(MNODE* dMML_list, BUCKET_REC* parent_bucket, bool isLHSofDef, Analyzer* pAnalyzer);

SEMANTICS_NODE* GetSemanticsList(MNODE* dMML_list,
                                 BUCKET_REC* parent_bucket,
                                 int mml_node_lim,
                                 bool isLHSofDef,
                                 Analyzer* pAnalyzer);

SEMANTICS_NODE* GetDefSList(MNODE* dMML_list, 
                            BUCKET_REC* parent,
                            int mml_node_lim, 
                            int& error_code,
                            Analyzer* pAnalyzer);

SEMANTICS_NODE* SNodeFromMNodes(MNODE* mml_node, 
                                int& mml_nodes_done,
                                bool isLHSofDef, 
                                Analyzer* pAnalyzer);


void AnalyzeMN(MNODE* mml_mn, SEMANTICS_NODE* info, Analyzer* pAnalyzer);

void AnalyzeMI(MNODE* self, SEMANTICS_NODE* info, int& nodes_done, bool isLHSofDef, Analyzer* pAnalyzer);

void AnalyzeMO(MNODE* self, SEMANTICS_NODE* info, int& nodes_done, Analyzer* pAnalyzer);

void AnalyzeMTEXT(MNODE* mml_mtext_node, SEMANTICS_NODE* snode, int& nodes_done, Analyzer* pAnalyzer);

void AnalyzeMFRAC(MNODE* mml_node, SEMANTICS_NODE* info, int& nodes_done, Analyzer* pAnalyzer);

void AnalyzeMSQRT(MNODE* mml_node, SEMANTICS_NODE* info, int& nodes_done, Analyzer* pAnalyzer);

void AnalyzeMROOT(MNODE* mml_node, SEMANTICS_NODE* info, int& nodes_done, Analyzer* pAnalyzer);

void AnalyzeMFENCED(MNODE * mml_node, SEMANTICS_NODE* info, int& nodes_done, Analyzer* pAnalyzer);

void AnalyzeMSUP(MNODE* mml_msup_node, SEMANTICS_NODE* info, int& nodes_done, bool isLHSofDef, Analyzer* pAnalyzer);

void AnalyzeMSUB(MNODE* mml_msup_node, SEMANTICS_NODE* info, int& nodes_done, bool isLHSofDef, Analyzer* pAnalyzer);

void AnalyzeMUNDER(MNODE* mml_munder_node, SEMANTICS_NODE* info, int& nodes_done, bool isLHSofDef, Analyzer* pAnalyzer);

void AnalyzeMOVER(MNODE* mml_munder_node, SEMANTICS_NODE* info, int& nodes_done, bool isLHSofDef, Analyzer* pAnalyzer);

void AnalyzeMTABLE(MNODE* mml_mtable_node, SEMANTICS_NODE* info, int& nodes_done, Analyzer* pAnalyzer);

void AnalyzeMUNDEROVER(MNODE* mml_munder_node, SEMANTICS_NODE* info, int& nodes_done, bool isLHSofDef, Analyzer* pAnalyzer);

void AnalyzeMSUBSUP(MNODE* mml_munder_node, SEMANTICS_NODE* info,  int& nodes_done, bool isLHSofDef, Analyzer* pAnalyzer);

void AnalyzeMixedNum(MNODE* mml_mn, SEMANTICS_NODE* s_node, Analyzer* pAnalyzer);

void AnalyzePrimed(MNODE* mml_msup, SEMANTICS_NODE* s_node, int& nodes_done, Analyzer* pAnalyzer);

void AnalyzeDotDerivative(MNODE* mml_mover, int n_dots, SEMANTICS_NODE* s_node, int& nodes_done, Analyzer* pAnalyzer);

void AnalyzeSubscriptedFunc(MNODE* mml_msub_node, SEMANTICS_NODE* snode, int& nodes_done, Analyzer* pAnalyzer);

void AnalyzeSubscriptedArgFunc(MNODE* mml_msub_node, SEMANTICS_NODE* snode, Analyzer* pAnalyzer);

void AnalyzeSubscriptedFence(MNODE* mml_msub_node, SEMANTICS_NODE* snode, int& nodes_done, Analyzer* pAnalyzer);

void AnalyzeBesselFunc(MNODE* mml_msub_node, SEMANTICS_NODE* snode, int& nodes_done, Analyzer* pAnalyzer);



SEMANTICS_NODE* DefToSemanticsList(MNODE* dMML_tree, int& error_code, Analyzer* pAnalyzer);

void CreateSubstBucket(MNODE* subst, SEMANTICS_NODE* snode, bool is_lower, Analyzer* pAnalyzer);




void MSUB2FuncCall(MNODE* mml_msub_node, SEMANTICS_NODE * snode, Analyzer* pAnalyzer);



BUCKET_REC* ArgsToBucket(MNODE* func_node, int& nodes_done, Analyzer* pAnalyzer) ;


void OperandToBucketList(MNODE* mml_func_node, SemanticType bigop_type,
                         SEMANTICS_NODE* info, int& nodes_done, Analyzer* pAnalyzer);


BUCKET_REC* ArgBucketFromMROW(MNODE* mml_next, Analyzer* pAnalyzer);

  
BUCKET_REC* GetParenedArgs(MNODE* mml_mo, int& nodes_done, Analyzer* pAnalyzer);
BUCKET_REC* GetFencedArgs(MNODE* mml_fence, Analyzer* pAnalyzer);





SEMANTICS_NODE* DetermineIndepVar(MNODE* dMML_tree, Analyzer* pAnalyzer);
SEMANTICS_NODE* GetIndepVarFromSub(MNODE* msub, Analyzer* pAnalyzer);
SEMANTICS_NODE* GetIndVarFromFrac(MNODE* mfrac, Analyzer* pAnalyzer);

SEMANTICS_NODE* ExtractIndepVar(MNODE* rover, Analyzer* pAnalyzer);


SEMANTICS_NODE* QualifierToSNODE(MNODE* sub, Analyzer* pAnalyzer);



void CreateSubscriptedVar(MNODE* mml_msub_node,
                          bool remove_super, SEMANTICS_NODE * snode, Analyzer* pAnalyzer);

void CreatePowerForm(MNODE* base, MNODE* power, SEMANTICS_NODE* snode, Analyzer* pAnalyzer);

bool SetODEvars(MathServiceRequest& msr, MathResult& mr, MNODE * dMML_tree, U32 UI_cmd_ID, Analyzer* pAnalyzer);
void FixInvisibleTimesAfterFunction(MNODE* dMML_tree, AnalyzerData* pData);
void FixInvisibleTimesAfterFunction2(MNODE* dMML_tree, AnalyzerData* pData);



bool SetIMPLICITvars(MathServiceRequest& msr, MathResult& mr, Analyzer* pAnalyzer) ;

  
SEMANTICS_NODE* CreateSTreeFromMML(const char* mml_str, Analyzer* pAnalyzer);

void TranslateEmbellishedOp(MNODE* mml_embellop_node,
                            SEMANTICS_NODE* snode, 
                            int& nodes_done, 
                            Analyzer* pAnalyzer);



SEMANTICS_NODE* AppendNewBucketRecord(U32 which_bucket, SEMANTICS_NODE* sem_child, SEMANTICS_NODE* snode, MNODE* base, bool fromNode, Analyzer* pAnalyzer)
{

      BUCKET_REC* bucket = MakeBucketRec(which_bucket, sem_child);

      AppendBucketRecord(snode->bucket_list, bucket);

      SEMANTICS_NODE* contents = NULL;
      if (fromNode)
         contents = GetSemanticsFromNode(base, bucket, pAnalyzer);
	    else
	       contents = GetSemanticsList(base, bucket, false, pAnalyzer);


      bucket->first_child = contents;
      
      if (contents) 
        contents->parent = bucket;

      return contents;
}





Analyzer::Analyzer(const Grammar* mml_grammar, PrefsStore* up_store)
  : m_pData( new AnalyzerData )
{
  GetAnalyzerData()  -> SetGrammar(mml_grammar);
  SetScrStr( NULL );
  GetAnalyzerData() -> SetPrefsStore( up_store );

  GetAnalyzerData() -> SetDefStore( NULL );
  GetAnalyzerData() -> SetCurrClientID( 0 );
  GetAnalyzerData() -> SetCurrEngineID( 0 );

  GetAnalyzerData() -> SetLogMsgList( NULL );
  m_node_IDs_list = NULL;
  SetCanonicalTreeGen ( new Tree2StdMML(mml_grammar, GetAnalyzerData()) );
  GetAnalyzerData() -> SetDE_ind_vars( NULL );
  GetAnalyzerData() -> SetDE_FuncNames( NULL );
  SetIMPLDIFF_ind_var( NULL );
  GetAnalyzerData() -> SetIMPLDIFF_FuncNames( NULL );

  GetAnalyzerData()  -> SetMMLTreeGen( new MML2Tree() );

  GetAnalyzerData() -> Set_i_is_imaginary( true );
  GetAnalyzerData() -> Set_j_is_imaginary( false );
  GetAnalyzerData() -> Set_e_is_Euler( true );
}



Analyzer::~Analyzer()
{
  TCI_ASSERT(! GetAnalyzerData()  -> GetLogMsgList() );
  DisposeMsgs( GetAnalyzerData()  -> GetLogMsgList() );

  TCI_ASSERT(!NodeIDsList());
  DisposeIDsList( NodeIDsList() );
  delete GetAnalyzerData()  -> GetMMLTreeGen();
  delete CanonicalTreeGen;
  TCI_ASSERT( GetAnalyzerData() -> GetDE_ind_vars() == NULL );
  TCI_ASSERT( GetAnalyzerData() -> DE_FuncNames() == NULL );
  TCI_ASSERT( IMPLDIFF_ind_var == NULL );
  TCI_ASSERT( GetAnalyzerData() -> IMPLDIFF_FuncNames() == NULL );
}



void Analyzer::SetInputPrefs(DefStore* ds, U32 engine_ID)
{
  const char* pref_val = ds->GetPref(CLPF_log_is_base_e, 0);
  
  if (!pref_val)
    pref_val = GetAnalyzerData() -> GetPrefsStore() -> GetPref(CLPF_log_is_base_e);

  if (pref_val) {
    // log is base e<uID14.0>0
    int db_val = atoi(pref_val);
    Set_log_is_base10( db_val ? false : true );
  } else {
    TCI_ASSERT(0);
    Set_log_is_base10( false );
  }

  pref_val = ds->GetPref(CLPF_Dot_derivative, 0);
  if (!pref_val)
    pref_val = GetAnalyzerData() -> GetPrefsStore() -> GetPref(CLPF_Dot_derivative);
  if (pref_val) {
    // Dot accent is derivative<uID19.0>1
    int db_val = atoi(pref_val);
    Set_dot_is_derivative( db_val ? true : false );
  } else {
    TCI_ASSERT(0);
    Set_dot_is_derivative( false );
  }

  pref_val = ds->GetPref(CLPF_D_derivative, 0);
  if (!pref_val)
    pref_val = GetAnalyzerData() -> GetPrefsStore() -> GetPref(CLPF_D_derivative);
  if (pref_val) {
    // Dot accent is derivative<uID19.0>1
    int db_val = atoi(pref_val);
    Set_D_is_derivative( db_val ? true : false );
  } else {
    TCI_ASSERT(0);
    Set_D_is_derivative( true );
  }

  pref_val = ds->GetPref(CLPF_Prime_derivative, 0);
  if (!pref_val)
    pref_val = GetAnalyzerData() -> GetPrefsStore() -> GetPref(CLPF_Prime_derivative);
  if (pref_val) {
    int db_val = atoi(pref_val);
    Set_prime_is_derivative( db_val ? true : false );
  } else {
    TCI_ASSERT(0);
    Set_prime_is_derivative( false );
  }

  pref_val = ds->GetPref(CLPF_Overbar_conjugate, 0);
  if (!pref_val)
    pref_val = GetAnalyzerData() -> GetPrefsStore() -> GetPref(CLPF_Overbar_conjugate);
  if (pref_val) {
    // Overbar is conjugate<uID20.0>1
     int db_val = atoi(pref_val);
    Set_overbar_conj( db_val ? true : false );
  } else {
    TCI_ASSERT(0);
    Set_overbar_conj( false );
  }

  DefInfo* di = ds->GetDefInfo(engine_ID, "mii");
  if (di) {
    GetAnalyzerData() -> Set_i_is_imaginary( false );
  } else {
    pref_val = ds->GetPref(CLPF_Input_i_Imaginary, 0);
    if (!pref_val)
      pref_val = GetAnalyzerData() -> GetPrefsStore() -> GetPref(CLPF_Input_i_Imaginary);
    if (pref_val) {
      int db_val = atoi(pref_val);
      GetAnalyzerData() -> Set_i_is_imaginary( db_val ? true : false );
    } else {
      TCI_ASSERT(0);
      GetAnalyzerData() -> Set_i_is_imaginary( true );
    }
  }

  di = ds->GetDefInfo(engine_ID, "mij");
  if (di) {
    GetAnalyzerData() -> Set_j_is_imaginary( false );
  } else {
    pref_val = ds->GetPref(CLPF_Input_j_Imaginary, 0);
    if (!pref_val)
      pref_val = GetAnalyzerData() -> GetPrefsStore() -> GetPref(CLPF_Input_j_Imaginary);
    if (pref_val) {
      int db_val = atoi(pref_val);
      GetAnalyzerData() -> Set_j_is_imaginary( db_val ? true : false );
    } else {
      TCI_ASSERT(0);
      GetAnalyzerData() -> Set_j_is_imaginary( false );
    }
  }

  di = ds->GetDefInfo(engine_ID, "mie");
  if (di) {
    GetAnalyzerData() -> Set_e_is_Euler( false );
  } else {
    pref_val = ds->GetPref(CLPF_Input_e_Euler, 0);
    if (!pref_val)
      pref_val = GetAnalyzerData() -> GetPrefsStore() -> GetPref(CLPF_Input_e_Euler);
    if (pref_val) {
      int db_val = atoi(pref_val);
      GetAnalyzerData() -> Set_e_is_Euler( db_val ? true : false );
    } else {
      TCI_ASSERT(0);
      GetAnalyzerData() -> Set_e_is_Euler( false );
    }
  }

}



LOG_MSG_REC* Analyzer::GetMsgs()
{
  LOG_MSG_REC *rv = GetAnalyzerData() -> GetLogMsgList();
  GetAnalyzerData() -> SetLogMsgList( NULL );
  return rv;
}


void Analyzer::AppendIDList(char* mml_canonical_name, MNODE* mml_node){
	 SetNodeIDsList (AppendIDRec(NodeIDsList(),
	                             GetAnalyzerData() -> CurrClientID(),
								 mml_canonical_name,
 								 mml_node,
								 ScrStr()));
}


// WARNING: ownership of a list created in this object
//  is passed to another object here!

MIC2MMLNODE_REC *Analyzer::GetBackMap()
{
  MIC2MMLNODE_REC *rv = NodeIDsList();
  SetNodeIDsList( NULL );
  return rv;
}




// NOTE: The following function modifies it's dMML_tree parameter.
// It converts the tree to a canonical form before starting the
// semantic analysis.  This step is required so that we can avoid
// having to deal with multiple markups for the same object.
// For example, a construct bracketed by <mo>(</mo>..<mo>)</mo>
// might represent the same math object that an <mfenced> represents.

SEMANTICS_NODE* Analyzer::BuildSemanticsTree(MathServiceRequest& msr,
                                             MathResult& mr,
                                             const char* src_markup,
                                             MNODE* dMML_tree,
                                             U32 curr_cmd_ID,
                                             INPUT_NOTATION_REC* in_notation)
{
  SEMANTICS_NODE* rv = NULL;

  SetCmdID( curr_cmd_ID );
  SetScrStr( src_markup );
  GetAnalyzerData()  -> SetInputNotation( in_notation );

  switch (curr_cmd_ID) {
    case CCID_Cleanup:
    case CCID_Fixup:
      dMML_tree = CanonicalTreeGen->TreeToCleanupForm(dMML_tree);
      break;

    case CCID_Interpret:
      dMML_tree = CanonicalTreeGen->TreeToInterpretForm(dMML_tree, in_notation);
      break;

    case CCID_PassThru:
      break;

    default:
      dMML_tree = CanonicalTreeGen->TreeToCanonicalForm(dMML_tree, in_notation);
      break;
  }

  m_pData -> SetDefStore( msr.GetDefStore() );
  GetAnalyzerData() -> SetCurrClientID( msr.GetClientHandle() );
  GetAnalyzerData() -> SetCurrEngineID( msr.GetEngineID() );

  TCI_ASSERT(!IMPLDIFF_ind_var);

  if (curr_cmd_ID == CCID_Calculus_Implicit_Differentiation) {
    if (!SetIMPLICITvars(msr, mr, this)) {
      TCI_ASSERT(0);
      return rv;
    }
  }

  TCI_ASSERT(!GetAnalyzerData() -> GetDE_ind_vars());

  if (curr_cmd_ID >= CCID_Solve_ODE_Exact && curr_cmd_ID <= CCID_Solve_ODE_Series) {  // Solve ODE

     // ODEs are indeed special - we have to decide both the function
     //  that we're seeking to define, and the independent variable.
     if (!SetODEvars(msr, mr, dMML_tree, curr_cmd_ID, this))
       return rv;

  }

  if (curr_cmd_ID == CCID_Solve_PDE) {
    SEMANTICS_NODE* s_indvar = DetermineIndepVar(dMML_tree, this);
    TCI_ASSERT(s_indvar);
    DeterminePDEFuncNames(dMML_tree, GetAnalyzerData());
  }

  TCI_ASSERT(CheckLinks(dMML_tree));
  JBM::DumpTList(dMML_tree, 0);
  TCI_ASSERT(!GetAnalyzerData()  -> GetLogMsgList() );
  DisposeMsgs(GetAnalyzerData()  -> GetLogMsgList() );
  GetAnalyzerData()  -> SetLogMsgList( NULL );

  // We sometimes get here with ODEs.  Some analysis that generates
  //  name records may be done to determine the independent variable.
  DisposeIDsList( NodeIDsList() );
  SetNodeIDsList( NULL );

  // "i", "j" and "e" must be treated as formal args (ie. variables)
  //   if they occur on the left hand side of a definition
  if (curr_cmd_ID == CCID_Define) {
    int error_code;
    rv = DefToSemanticsList(dMML_tree, error_code, this);
    if (error_code || !rv) {
      mr.PutResultCode(CR_baddefformat);
    }
  } else {
    rv = GetSemanticsList(dMML_tree, NULL, false, this);
  }

  if (GetAnalyzerData() -> DE_FuncNames()) {
    AppendODEfuncs(rv, GetAnalyzerData() -> DE_FuncNames(), GetAnalyzerData());
    DisposeODEFuncNames( GetAnalyzerData() -> DE_FuncNames() );
    GetAnalyzerData() -> SetDE_FuncNames( NULL );
  }
  DisposeSList(GetAnalyzerData() -> GetDE_ind_vars() );
  GetAnalyzerData() -> SetDE_ind_vars( NULL );

  if (GetAnalyzerData() -> IMPLDIFF_FuncNames()) {
    DisposeODEFuncNames(GetAnalyzerData() -> IMPLDIFF_FuncNames());
    GetAnalyzerData() -> SetIMPLDIFF_FuncNames( NULL );
  }
  DisposeSList( GetIMPLDIFF_ind_var() );
  SetIMPLDIFF_ind_var( NULL );

  return rv;
}


void Analyzer::TreeToCleanupForm(MNODE* dMML_tree)
{
  CanonicalTreeGen->TreeToCleanupForm(dMML_tree);
  JBM::DumpTList(dMML_tree, 0);
}

void Analyzer::TreeToFixupForm(MNODE* dMML_tree)
{
  CanonicalTreeGen->TreeToFixupForm(dMML_tree, D_is_derivative);
  FixInvisibleTimesAfterFunction2(dMML_tree, GetAnalyzerData());
  JBM::DumpTList(dMML_tree, 0);
}






// <mi>x</mi>
// <mi>sin</mi>        <START_CONTEXT_FUNCTIONS>
// <mi>&theta;</mi>    <START_CONTEXT_VARIABLES>
// This is a preliminary implementation.  Ultimately
//  all the logical required to decide the meaning
//  an <mi> lies within this function.

void AnalyzeMI(MNODE* mml_mi_node,
               SEMANTICS_NODE* snode,
               int& nodes_done, 
               bool isLHSofDef,
               Analyzer* pAnalyzer)
{
  nodes_done = 1;

  char zclass[256];
  zclass[0] = 0;
  GetCurrAttribValue(mml_mi_node, false, "class", zclass, 256);

  char zmsi_class[256];
  zmsi_class[0] = 0;
  GetCurrAttribValue(mml_mi_node, false, "msiclass", zmsi_class, 256);

  char zmsi_unit[256];
  zmsi_unit[0] = 0;
  GetCurrAttribValue(mml_mi_node, false, "msiunit", zmsi_unit, 256);

  if (StringEqual(zmsi_unit, "true")) {

    SEMANTICS_NODE* s_target = snode;
    // Semantically, I'm treating units as "factors" joined to the expression
    //  that they qualify by an invisible times. If &it; is NOT present
    //  in the source MML, it's semantic equivalent is generated here.
    if (mml_mi_node->prev && !IsOperator(mml_mi_node->prev) && !IsPositionalChild(mml_mi_node)) {
      TCI_ASSERT(0);
      snode->semantic_type = SEM_TYP_INFIX_OP;
      
      snode->contents = DuplicateString("&#x2062;");

      s_target = CreateSemanticsNode();
      snode->next = s_target;
      s_target->prev = snode;
    }

    const char* ptr = mml_mi_node->p_chdata;

    s_target->contents = DuplicateString(mml_mi_node->p_chdata);

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

  char* mml_canonical_name = GetCanonicalIDforMathNode(mml_mi_node, pAnalyzer -> GetAnalyzerData() -> GetGrammar());

  if (!mml_canonical_name) {
    snode->error_flag = 1;
    TCI_ASSERT(0);
    return;
  }
  // Put the canonical name of the math object in "snode".
  snode->canonical_ID = mml_canonical_name;
  SetSnodeOwner(snode, pAnalyzer -> GetAnalyzerData());

  // Store a name-to-node back mapping record for this object.
  // pAnalyzer -> SetNodeIDsList( AppendIDRec(pAnalyzer -> NodeIDsList(), 
  //                              pAnalyzer -> GetAnalyzerData() -> CurrClientID(),
  //                              mml_canonical_name, 
  //                              mml_mi_node, 
  //                              pAnalyzer -> ScrStr()));

  pAnalyzer -> AppendIDList(mml_canonical_name, mml_mi_node);

  // This <mi> may be "defined" in the client's current context,
  //  or it may be a special predefined identifier.
  IdentIlk mi_ilk;
  if (zmsi_class[0]) {
    mi_ilk = GetMSIilk(zmsi_class);
    snode->msi_class = mi_ilk;
  } else {
    
    mi_ilk = GetMIilk(mml_canonical_name, 
                      pAnalyzer->GetAnalyzerData() ->GetDI ( mml_canonical_name), 
                      mml_mi_node, 
                      isLHSofDef, 
                      pAnalyzer-> GetAnalyzerData() -> GetGrammar(),
                      pAnalyzer-> GetAnalyzerData() -> Get_i_is_imaginary(),
                      pAnalyzer-> GetAnalyzerData() -> Get_j_is_imaginary(),
                      pAnalyzer-> GetAnalyzerData() -> Get_e_is_Euler());
  }
  if (mi_ilk) {
    if (IdentIsConstant(mi_ilk)) {

        const char* cont = mml_mi_node->p_chdata;
        if (mi_ilk == MI_imaginaryunit)
          cont = "i";

        snode->contents = DuplicateString(cont);
        snode->semantic_type = SEM_TYP_UCONSTANT;

    } else if (mi_ilk == MI_Laplace || mi_ilk == MI_Fourier || mi_ilk == MI_function) {
    	
        snode->contents = DuplicateString(mml_mi_node->p_chdata);;
        if (mi_ilk == MI_Laplace || mi_ilk == MI_Fourier)
          snode->semantic_type = SEM_TYP_TRANSFORM;
        else
          snode->semantic_type = SEM_TYP_FUNCTION;

        int local_nodes_done;
        BUCKET_REC* bucket = ArgsToBucket(mml_mi_node, local_nodes_done, pAnalyzer);
        nodes_done += local_nodes_done;

        bool arg_is_matrix = OpArgIsMatrix(mml_mi_node);
        if (arg_is_matrix)        // the args become entries in a row matrix
          ArgsToMatrix(snode, bucket);
        else
          AppendBucketRecord(snode->bucket_list, bucket);

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
      snode->contents = DuplicateString(varnom);
    } else {
      TCI_ASSERT(0);
    }
    bool is_function = false;
    bool is_ODE_func = false;
    bool is_IMPLDIFF_func = false;

    if (LocateFuncRec(pAnalyzer -> GetAnalyzerData() -> DE_FuncNames(), mml_canonical_name, NULL)) {
        is_ODE_func = true;
    } else if (LocateFuncRec(pAnalyzer -> GetAnalyzerData() -> IMPLDIFF_FuncNames(), NULL, mml_mi_node->p_chdata)) {
        is_IMPLDIFF_func = true;
    } else if (symbol_count >= 1) {
      // an entity, maybe a Greek letter
      // ASCII letter 
      if (mml_mi_node->next) {
        // Look for &ApplyFunction; after this <mi>
        MNODE* next_elem = mml_mi_node->next;

        if (!mml_mi_node->prev && mml_mi_node->parent) {

		  MNODE* parent = mml_mi_node->parent;

          if (ElementNameIs(parent,  "msub") ||
              ElementNameIs(parent, "msup")	 ||
              ElementNameIs(parent, "msubsup") ||
              ElementNameIs(parent, "munder") ||
              ElementNameIs(parent, "mover")  ||
              ElementNameIs(parent, "munderover"))
            next_elem = parent->next;
        }
        if (next_elem)
          if (IsApplyFunction(next_elem))
            is_function = true;
      }
    } else if (symbol_count > 1) {
        TCI_ASSERT((entity_count > 0)); // multiple entities?
        is_function = true;
    }

    if (is_function || is_ODE_func || is_IMPLDIFF_func) {
      snode->semantic_type = SEM_TYP_FUNCTION;

      if (ContentIs(mml_mi_node, "log")) {
        if (pAnalyzer -> GetAnalyzerData()  -> GetInputNotation()) {

           pAnalyzer -> GetAnalyzerData()  -> GetInputNotation() -> n_logs++;
        }
        AddDefaultBaseToLOG(snode, pAnalyzer->Get_log_is_base10());
      }
      
      int local_nodes_done;
      BUCKET_REC* bucket = ArgsToBucket(mml_mi_node, local_nodes_done, pAnalyzer);
      nodes_done += local_nodes_done;
      
      if (is_ODE_func && (!bucket || !bucket->first_child)) {
        if (pAnalyzer -> GetAnalyzerData() -> GetDE_ind_vars()) {
          if (bucket)
            DisposeBucketList(bucket);
          bucket = AddVarToBucket(MB_UNNAMED, pAnalyzer -> GetAnalyzerData() -> GetDE_ind_vars(), pAnalyzer -> GetAnalyzerData());
        } else {
          TCI_ASSERT(0);
          if (!bucket)
            bucket = MakeBucketRec(MB_UNNAMED, NULL);
        }
      }

      if (is_IMPLDIFF_func && (!bucket || !bucket->first_child)) {
        if (pAnalyzer -> GetIMPLDIFF_ind_var()) {
          if (bucket)
            DisposeBucketList(bucket);
          bucket = AddVarToBucket(MB_UNNAMED, pAnalyzer -> GetIMPLDIFF_ind_var(), pAnalyzer -> GetAnalyzerData());
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
        AppendBucketRecord(snode->bucket_list, bucket);
    } else {
     
       char isMathname[256];
       isMathname[0] = 0;
       GetCurrAttribValue(mml_mi_node, false, "msimathname", isMathname, 256);
       if (StringEqual(isMathname, "true") ) {
         if (StringEqual(mml_mi_node->p_chdata, "real") ){
           delete snode->contents;
           snode->contents = DuplicateString("Type::Real");
           snode->semantic_type = SEM_TYP_ENG_PASSTHRU;

         } else if (StringEqual(mml_mi_node->p_chdata, "complex") ){
           delete snode->contents;
           snode->contents = DuplicateString("Type::Complex");
           snode->semantic_type = SEM_TYP_ENG_PASSTHRU;
         } else if (StringEqual(mml_mi_node->p_chdata, "integer") ){
           delete snode->contents;
           snode->contents = DuplicateString("Type::Integer");
           snode->semantic_type = SEM_TYP_ENG_PASSTHRU;
         } else if (StringEqual(mml_mi_node->p_chdata, "positive") ){
           delete snode->contents;
           snode->contents = DuplicateString("Type::Positive");
           snode->semantic_type = SEM_TYP_ENG_PASSTHRU;
         } else if (StringEqual(mml_mi_node->p_chdata, "negative") ){
           delete snode->contents;
           snode->contents = DuplicateString("Type::Negative");
           snode->semantic_type = SEM_TYP_ENG_PASSTHRU;
         } else if (StringEqual(mml_mi_node->p_chdata, "nonzero") ){
           delete snode->contents;
           snode->contents = DuplicateString("Type::Nonzero");
           snode->semantic_type = SEM_TYP_ENG_PASSTHRU;
         } else {
           snode->semantic_type = SEM_TYP_VARIABLE;
         }
       } else
         snode->semantic_type = SEM_TYP_VARIABLE;
    }
  }
}



void AnalyzeMTEXT(MNODE* mml_mtext_node,
                  SEMANTICS_NODE* snode, 
                  int& nodes_done, 
                  Analyzer* pAnalyzer)
{
  nodes_done = 1;               // probably always doing 1 mml node here!

  if (mml_mtext_node->p_chdata) {
    if (!IsWhiteText(mml_mtext_node->p_chdata)) {

      snode->contents = DuplicateString(mml_mtext_node->p_chdata);

      if (pAnalyzer -> CmdID() == CCID_PassThru) {
        snode->semantic_type = SEM_TYP_ENG_PASSTHRU;
      } else {
        char* mml_canonical_name = GetCanonicalIDforMathNode(mml_mtext_node, pAnalyzer-> GetAnalyzerData() -> GetGrammar());
        snode->canonical_ID = mml_canonical_name;
        SetSnodeOwner(snode, pAnalyzer -> GetAnalyzerData());
        // Store a name-to-node back mapping record for this object.
        // pAnalyzer -> SetNodeIDsList( AppendIDRec(pAnalyzer -> NodeIDsList(), pAnalyzer ->GetAnalyzerData() -> CurrClientID(),
        //                             mml_canonical_name, mml_mtext_node,
        //                             pAnalyzer -> ScrStr()) );
		pAnalyzer -> AppendIDList(mml_canonical_name, mml_mtext_node);
        snode->semantic_type = SEM_TYP_TEXT;
      }
    }
  } else
    TCI_ASSERT(0);
}



void AnalyzeMO(MNODE* mml_mo_node,
               SEMANTICS_NODE* snode, int& nodes_done, Analyzer* pAnalyzer)
{
  nodes_done = 1;               // note that an operand may be added

  snode->semantic_type = SEM_TYP_INFIX_OP;  // assumed

  if (mml_mo_node->p_chdata) {
    SemanticVariant n_integs;
    const char *f_nom = mml_mo_node->p_chdata;
    PrefixOpIlk op_ilk = GetPrefixOpCode(f_nom, n_integs, pAnalyzer-> GetAnalyzerData() -> GetGrammar());

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
      case POI_function:
        snode->semantic_type = SEM_TYP_FUNCTION;
        break;
      case POI_gradient:
        snode->semantic_type = SEM_TYP_PREFIX_OP;
        if ( (mml_mo_node -> next) && (0 == strcmp(mml_mo_node -> next -> p_chdata, "&#xd7;") ) ){
           op_ilk = POI_curl;
           f_nom = "curl";
           nodes_done = 2;
        } else if ( (mml_mo_node -> next) && (0 == strcmp(mml_mo_node -> next -> p_chdata, "&#x22c5;") ) ){
           op_ilk = POI_divergence;
           f_nom = "div";
           nodes_done = 2;
        } else {
          f_nom = "grad";
        }
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
          OperandToBucketList(mml_mo_node, snode->semantic_type, snode, nodes_in_arg, pAnalyzer);
          nodes_done += nodes_in_arg;

      } else {

          // the right operand is nested, like a function argument
          int nodes_in_arg;
          BUCKET_REC *br = ArgsToBucket(mml_mo_node, nodes_in_arg, pAnalyzer);
          nodes_done += nodes_in_arg;
          if (br) {
            if (arg_is_matrix)    // the args become entries in a row matrix
              ArgsToMatrix(snode, br);
            else
              AppendBucketRecord(snode->bucket_list, br);
          }

      }
    } else {

        const char *attr_val = GetATTRIBvalue(mml_mo_node->attrib_list, "form");
        if (attr_val) {
          if (StringEqual(attr_val, "prefix")) {

              snode->semantic_type = SEM_TYP_PREFIX_OP;

          } else if (StringEqual(attr_val, "infix")) {

              snode->semantic_type = SEM_TYP_INFIX_OP;

          } else if (StringEqual(attr_val, "postfix")) {

              snode->semantic_type = SEM_TYP_POSTFIX_OP;

          } else {
              TCI_ASSERT(0);
          }
        }

    }

    // It's not clear that operators should ever be aliased.
    // Install our own standard semantic name for the operator.

    if (StringEqual(f_nom, "&#x5c;")) // bogus, backslash to setminus
      f_nom = "&#x2216;";

    delete[] snode->contents;
    snode->contents = DuplicateString(f_nom);

  } else {
    TCI_ASSERT(0);
  }
}



void AnalyzeMN(MNODE* mml_mn_node, SEMANTICS_NODE* snode, Analyzer* pAnalyzer)
{
  if (mml_mn_node && mml_mn_node->p_chdata) {
    size_t zln = strlen(mml_mn_node->p_chdata);
    char* num_str = new char[zln + 2];

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
    if (num_str[di-1] == '.'){
       num_str[di++] = '0';
    }
    num_str[di] = 0;

    if (n_digits != zln) {
      TCI_ASSERT(0);
      strcpy(num_str, mml_mn_node->p_chdata);

      // For <mn>s that carry base 10 digits only, we don't need mapping info.
      // Other <mn>'s will probably pass thru computation as a symbolic
      //  alias, and will require mapping info.
      char* mml_canonical_name = GetCanonicalIDforMathNode(mml_mn_node, pAnalyzer-> GetAnalyzerData() -> GetGrammar());
      snode->canonical_ID = mml_canonical_name;
      SetSnodeOwner(snode, pAnalyzer -> GetAnalyzerData());
      // Store a name-to-node back mapping record for this object.
      // pAnalyzer -> SetNodeIDsList( AppendIDRec(pAnalyzer -> NodeIDsList(), pAnalyzer ->GetAnalyzerData() -> CurrClientID(),
      //                             mml_canonical_name, mml_mn_node, pAnalyzer -> ScrStr()) );
	  pAnalyzer -> AppendIDList(mml_canonical_name, mml_mn_node);
    }
    snode->contents = num_str;
  } else
    TCI_ASSERT(0);

  snode->semantic_type = SEM_TYP_NUMBER;
}



void AnalyzeMFRAC(MNODE* mml_mfrac, SEMANTICS_NODE* snode, int& nodes_done, Analyzer* pAnalyzer)
{
  nodes_done = 1;

  // Look for "d^4(?)/d^{3}xdy"
  MNODE *m_num_operand;
  MNODE *m_den;
  if (IsDIFFOP(mml_mfrac, &m_num_operand, &m_den)) {
    snode->semantic_type = SEM_TYP_DERIVATIVE;
    snode->contents = DuplicateString("differentiate");

    // Handle the expression being differentiated
    if (m_num_operand) {
      // Here the expression being differentiated is in the numerator
      int nodes_in_arg;
      BUCKET_REC* br = ArgsToBucket(m_num_operand->prev, nodes_in_arg, pAnalyzer);
      if (br)
        AppendBucketRecord(snode->bucket_list, br);
    } else {
      // Here the expression being differentiated follows the fraction
      int nodes_in_arg;
      BUCKET_REC* br = ArgsToBucket(mml_mfrac, nodes_in_arg, pAnalyzer);
      nodes_done += nodes_in_arg;
      if (br)
        AppendBucketRecord(snode->bucket_list, br);
    }
    if (m_den) {                // denominator - "d^{3}xdy"
      SEMANTICS_NODE* s_indvar = GetIndVarFromFrac(mml_mfrac, pAnalyzer);
      if (s_indvar) {
        AppendBucketRecord(snode->bucket_list, MakeParentBucketRec(MB_DIFF_VAR, s_indvar));
      } else {
        TCI_ASSERT(0);
      }
    } else {
      TCI_ASSERT(0);
    }
    if (pAnalyzer -> GetAnalyzerData()  -> GetInputNotation()) {
       pAnalyzer -> GetAnalyzerData()  -> GetInputNotation() -> n_doverds++;
    }
    return;
  }

  SEMANTICS_NODE* s_target = snode;

  if (IsUnitsFraction(mml_mfrac)) {
    // Semantically, I'm treating units as "factors" joined to the expression
    //  that they qualify by an invisible times. If &it; is NOT present
    //  in the source MML, it's semantic equivalent is generated here.
    if (mml_mfrac->prev && !IsOperator(mml_mfrac->prev)) {
      snode->semantic_type = SEM_TYP_INFIX_OP;
      snode->contents = DuplicateString("&#x2062;");

      s_target = CreateSemanticsNode();
      snode->next = s_target;
      s_target->prev = snode;
    }
  }

  MNODE* rover = mml_mfrac->first_kid;
  
  if (rover) {
	  AppendNewBucketRecord(MB_NUMERATOR, NULL, s_target, rover, true, pAnalyzer);

      rover = rover->next;

      if (rover) {
	    AppendNewBucketRecord(MB_DENOMINATOR, NULL, s_target, rover, true, pAnalyzer);
      }
  }

  char buffer[256];
  buffer[0] = 0;
  GetCurrAttribValue(mml_mfrac, true, "linethickness", buffer, 256);
  if (!strcmp(buffer, "0"))
    s_target->semantic_type = SEM_TYP_BINOMIAL;
  else
    s_target->semantic_type = SEM_TYP_FRACTION;
}



void AnalyzeMSQRT(MNODE* mml_msqrt_node,
                  SEMANTICS_NODE* snode, 
                  int& nodes_done, 
                  Analyzer* pAnalyzer)
{
    nodes_done = 1;
	snode->semantic_type = SEM_TYP_SQRT;

    MNODE* rover = mml_msqrt_node->first_kid;
    if (rover) {
	  AppendNewBucketRecord(MB_ROOT_BASE, NULL, snode, rover, false, pAnalyzer);
    }    
}



void AnalyzeMROOT(MNODE* mml_mroot_node,
                  SEMANTICS_NODE* snode, 
                  int& nodes_done, 
                  Analyzer* pAnalyzer)
{
  snode->semantic_type = SEM_TYP_ROOT;
  nodes_done = 1;

  BUCKET_REC* parts_list = NULL;
  MNODE* rover = mml_mroot_node->first_kid;
  if (rover) {
    AppendNewBucketRecord(MB_ROOT_BASE, NULL, snode, rover, true, pAnalyzer); 

    rover = rover->next;

    if (rover) {
	  AppendNewBucketRecord(MB_ROOT_EXPONENT, NULL, snode, rover, true, pAnalyzer); 
    }
  }

  //snode->bucket_list = parts_list;
}

void AnalyzeMFENCED(MNODE* mml_mfenced_node,
                    SEMANTICS_NODE* snode, 
                    int& nodes_done, 
                    Analyzer* pAnalyzer)
{
  nodes_done = 1;
  U32 l_unicode = '(';
  U32 r_unicode = ')';

  if (mml_mfenced_node->attrib_list) {

    const char* ilk_value = GetATTRIBvalue(mml_mfenced_node->attrib_list, "ilk");
    const char* open_value = GetATTRIBvalue(mml_mfenced_node->attrib_list, "open");
    const char* close_value = GetATTRIBvalue(mml_mfenced_node->attrib_list, "close");

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

    char* key = NULL;
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
	  snode->contents = DuplicateString(key);
    }
  } else {
    TCI_ASSERT(!"No attribute list.");
  }
  //BUCKET_REC* parts_list = NULL;
  int num_children = 0;
  MNODE* rover = mml_mfenced_node->first_kid;
  while (rover) {
	AppendNewBucketRecord(MB_UNNAMED, NULL, snode, rover, true, pAnalyzer);

    num_children++;
    rover = rover->next;
  }

  // Here we're recording info re input notation
  if (num_children == 1 && snode->bucket_list && snode->bucket_list->first_child) {
    SEMANTICS_NODE* s_child = snode->bucket_list->first_child;
    if (s_child->semantic_type == SEM_TYP_TABULATION) {
      if (pAnalyzer -> GetAnalyzerData()  -> GetInputNotation()) {
        pAnalyzer -> GetAnalyzerData()  -> GetInputNotation() -> n_tables++;
        if (l_unicode == '[')
          pAnalyzer -> GetAnalyzerData()  -> GetInputNotation() -> nbracket_tables++;
        else if (l_unicode == '(')
          pAnalyzer -> GetAnalyzerData()  -> GetInputNotation() -> nparen_tables++;
        else if (l_unicode == '{')
          pAnalyzer -> GetAnalyzerData()  -> GetInputNotation() -> nbrace_tables++;
      }    

      // \ matrix \ -> det \ matrix \ .
      if (snode->semantic_type == SEM_TYP_ABS) {
        snode->semantic_type = SEM_TYP_FUNCTION;
        delete[] snode->contents;
        snode->contents = DuplicateString("det");
      }
    } else if (s_child->semantic_type == SEM_TYP_BINOMIAL) {
      if (snode->semantic_type == SEM_TYP_PARENED_LIST)
        snode->semantic_type = SEM_TYP_PRECEDENCE_GROUP;
    }
  }

  //snode->bucket_list = parts_list;
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

bool IsDoublePrime(MNODE* exp)
{
   // Look for either a doubleprime charcter, or an mrow with two primes
   
   if ( ElementNameIs(exp, "mi") || ElementNameIs(exp, "mo") )
     return ContentIs(exp, "&#x2033;");
      
   if (!ElementNameIs(exp, "mrow"))
     return false;

  

   MNODE* k = exp->first_kid;
   if (!(k && ContentIs(k, "&#x2032;") && k->next))
     return false;
   
   MNODE* kk = k->next;
   if (!(kk && ContentIs(kk, "&#x2032;") && (0 == kk->next)))
     return false;

   return true;
  
   
}

// Lot's more to do in the following

void AnalyzeMSUP(MNODE* mml_msup_node, 
                 SEMANTICS_NODE* snode,
                 int& nodes_done, 
                 bool isLHSofDef, 
                 Analyzer* pAnalyzer)
{
  nodes_done = 1;

  MNODE* base = mml_msup_node->first_kid;
  MNODE* exp = base -> next;
  if (base) {
    BaseType bt = GetBaseType(mml_msup_node, isLHSofDef, pAnalyzer-> GetAnalyzerData(), pAnalyzer-> GetAnalyzerData() -> GetGrammar());
    ExpType et = GetExpType(bt, exp, pAnalyzer-> GetAnalyzerData() -> GetGrammar());

    bool done = false;

    // First look for a superscript that dictates semantics
    switch (et) {
    case ET_DEGREE: {
      snode->semantic_type = SEM_TYP_INFIX_OP;
      snode->contents = DuplicateString("&#x2062;");
      SEMANTICS_NODE* new_contentA = AppendNewBucketRecord(MB_UNNAMED, NULL, snode, base, true, pAnalyzer);      
      SEMANTICS_NODE* new_contentB = AppendNewBucketRecord(MB_UNNAMED, NULL, snode, exp, true, pAnalyzer);
      new_contentB -> semantic_type = SEM_TYP_SIUNIT;
      new_contentB -> contents = DuplicateString("&#xb0;");

      // If the next is a prime, it is a minute
      bool haveMinutes = false;
      MNODE* node = mml_msup_node;
      if (node && node->next && node -> next -> next) {        
         MNODE* nxt = node->next;
         if (strcmp(nxt->src_tok, "mo") == 0 && ContentIs (nxt, "+") ) {
            MNODE* nxt2 = nxt->next;
            if (strcmp(nxt2->src_tok, "msup") == 0){
               MNODE* exp = nxt2->first_kid->next;
               if (exp &&  ContentIs (exp, "&#x2032;")){   // prime = 2032
                  haveMinutes = true;
                  // make an attribuate to use as a flag
                  ATTRIB_REC* ar = new ATTRIB_REC("msiangle", "true");
                  if (nxt2->attrib_list){
                    InsertAttribute(nxt2->attrib_list, ar);
                  } else {
                    nxt2->attrib_list = ar;
                  }
                  node = nxt2;
               }     
            }
         }
         // Then check for seconds
         if (node && node->next && node -> next -> next) {        
           MNODE* nxt = node->next;
           if (strcmp(nxt->src_tok, "mo") == 0 && ContentIs (nxt, "+") ) {
              MNODE* nxt2 = nxt->next;
              if (strcmp(nxt2->src_tok, "msup") == 0){
                 MNODE* exp = nxt2->first_kid->next;
         
                 if (IsDoublePrime(exp)){
                    // make an attribuate to use as a flag
                    ATTRIB_REC* ar = new ATTRIB_REC("msiangle", "true");
                    if (nxt2->attrib_list) {
                       InsertAttribute(nxt->attrib_list, ar);
                    } else {
                       nxt2->attrib_list = ar;
                    }
                    node = nxt2;
                   
                 }
              }
          }
        }
      }  
      
      done = true;
    }
    break;

    case ET_CONJUGATE_INDICATOR:{
	    snode->semantic_type = SEM_TYP_CONJUGATE;
		  SEMANTICS_NODE* new_content = AppendNewBucketRecord(MB_UNNAMED, NULL, snode, base, true, pAnalyzer);
        
      if (new_content->semantic_type == SEM_TYP_GENERIC_FENCE)
          new_content->semantic_type = SEM_TYP_PRECEDENCE_GROUP;

      done = true;
    }
    break;

    case ET_TRANSPOSE_INDICATOR:{
		   AppendNewBucketRecord(MB_UNNAMED, NULL, snode, base, true, pAnalyzer);
       snode->semantic_type = SEM_TYP_MTRANSPOSE;
       done = true;
    }
    break;

    case ET_HTRANSPOSE_INDICATOR: {
		   AppendNewBucketRecord(MB_UNNAMED, NULL, snode, base, true, pAnalyzer);
       snode->semantic_type = SEM_TYP_HTRANSPOSE;
       done = true;
      }
    break;

    case ET_PRIMES: {
       const char* v = GetATTRIBvalue(mml_msup_node->attrib_list, "msiangle");
       if (v || !(pAnalyzer -> Get_prime_is_derivative())){
          snode->semantic_type = SEM_TYP_INFIX_OP;
          snode->contents = DuplicateString("&#x2062;");
          SEMANTICS_NODE* new_contentA = AppendNewBucketRecord(MB_UNNAMED, NULL, snode, base, true, pAnalyzer);      
          SEMANTICS_NODE* new_contentB = AppendNewBucketRecord(MB_UNNAMED, NULL, snode, exp, true, pAnalyzer);
          new_contentB -> semantic_type = SEM_TYP_SIUNIT;
          
          MNODE* exp = mml_msup_node -> first_kid -> next;
          if (exp &&  ContentIs (exp, "&#x2032;")){
             new_contentB -> contents = DuplicateString("&#x2032;");
          } else if (exp && ContentIs (exp, "&#x2033;")) {
             new_contentB -> contents = DuplicateString("&#x2033;");
          }
       } else if (pAnalyzer -> Get_prime_is_derivative()) {
          AnalyzePrimed(mml_msup_node, snode, nodes_done, pAnalyzer );
          if (pAnalyzer -> GetAnalyzerData()  -> GetInputNotation()) {
             pAnalyzer -> GetAnalyzerData()  -> GetInputNotation() -> n_primes++;
          }
          done = true;
        } else {
          // Here I'm currently assuming that prime is just a decoration
        }
      }
      break;

    case ET_PARENED_PRIMES_COUNT:{
        AnalyzePrimed(mml_msup_node, snode, nodes_done, pAnalyzer);
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
        TranslateEmbellishedOp(mml_msup_node, snode, nodes_done, pAnalyzer);
        break;

      case BT_FUNCTION:
      case BT_SUBARG_FUNCTION:{
          if (et == ET_INVERSE_INDICATOR) {
            char* mml_canonical_name = GetCanonicalIDforMathNode(mml_msup_node, pAnalyzer-> GetAnalyzerData() -> GetGrammar());
            if (!mml_canonical_name) {
              snode->error_flag = 1;
              return;
            }
            // Put the canonical name of the math object in "snode".
            snode->canonical_ID = mml_canonical_name;
            SetSnodeOwner(snode, pAnalyzer -> GetAnalyzerData());
            // Store a name-to-node back mapping record for this object.
            // pAnalyzer -> SetNodeIDsList( AppendIDRec(pAnalyzer -> NodeIDsList(), pAnalyzer ->GetAnalyzerData()-> CurrClientID(),
            //                             mml_canonical_name, mml_msup_node,
            //                             pAnalyzer -> ScrStr()) );
			      pAnalyzer -> AppendIDList(mml_canonical_name, mml_msup_node);
            int local_nodes_done;
            BUCKET_REC* br = ArgsToBucket(mml_msup_node, local_nodes_done, pAnalyzer);
            nodes_done += local_nodes_done;
            if (br)
              AppendBucketRecord(snode->bucket_list, br);

            snode->contents = DuplicateString(base->p_chdata);
            snode->semantic_type = SEM_TYP_INVFUNCTION;
          } else if (et == ET_POWER || et == ET_DIRECTION) {
            char* mml_canonical_name = GetCanonicalIDforMathNode(base, pAnalyzer-> GetAnalyzerData() -> GetGrammar());
            if (!mml_canonical_name) {
              snode->error_flag = 1;
              return;
            }
            // Put the canonical name of the math object in "snode".
            snode->canonical_ID = mml_canonical_name;
            SetSnodeOwner(snode, pAnalyzer -> GetAnalyzerData());
            // Store a name-to-node back mapping record for this object.
            // pAnalyzer -> SetNodeIDsList(AppendIDRec(pAnalyzer -> NodeIDsList() , pAnalyzer->GetAnalyzerData() -> CurrClientID(),
            //                             mml_canonical_name, base, pAnalyzer -> ScrStr()) );

            pAnalyzer -> AppendIDList(mml_canonical_name, base);

            AppendNewBucketRecord(MB_FUNC_EXPONENT, NULL, snode, base->next, true, pAnalyzer);

            // We're handling something like "f^{2}(x)"
            // We must process function arguments here.
            int local_nodes_done;
            BUCKET_REC *br = ArgsToBucket(mml_msup_node, local_nodes_done, pAnalyzer);
            nodes_done += local_nodes_done;
            if (br)
              AppendBucketRecord(snode->bucket_list, br);
            else {
              // The function may have an implicit argument that must be generated here
              BUCKET_REC *bucket =
                FindBucketRec(snode->bucket_list, MB_UNNAMED);
              if (!bucket || !bucket->first_child) {
                if (bucket)
                  RemoveBucket(snode, bucket);
                SEMANTICS_NODE *s_var_list = NULL;
                if (LocateFuncRec (pAnalyzer -> GetAnalyzerData() -> DE_FuncNames(), mml_canonical_name, base->p_chdata))
                  s_var_list = pAnalyzer -> GetAnalyzerData() -> GetDE_ind_vars();
                else if (LocateFuncRec (pAnalyzer -> GetAnalyzerData() -> IMPLDIFF_FuncNames(), mml_canonical_name, base->p_chdata))
                  s_var_list = pAnalyzer -> GetIMPLDIFF_ind_var();
                if (s_var_list) {
                  BUCKET_REC *fvar_bucket = AddVarToBucket(MB_UNNAMED, s_var_list, pAnalyzer -> GetAnalyzerData());
                  
                  AppendBucketRecord(snode->bucket_list, fvar_bucket);
                }
              }
            }

            // Add remaining function info
            snode->contents = DuplicateString(base->p_chdata);
            snode->semantic_type = SEM_TYP_FUNCTION;
            if (base->p_chdata && !strcmp(base->p_chdata, "log"))
              AddDefaultBaseToLOG(snode, pAnalyzer->Get_log_is_base10());
          } else if (et == ET_PRIMES) {
            TCI_ASSERT(0);
          } else
            TCI_ASSERT(0);
        }
        break;

      case BT_VARIABLE:{
          if (et == ET_INVERSE_INDICATOR || et == ET_POWER || et == ET_DIRECTION) {
            CreatePowerForm(base, base->next, snode, pAnalyzer);
          } else if (et == ET_PRIMES) {

            // Here we have a decorated variable
            char* mml_canonical_name = GetCanonicalIDforMathNode(mml_msup_node, pAnalyzer-> GetAnalyzerData() -> GetGrammar());
            if (!mml_canonical_name) {
              snode->error_flag = 1;
              return;
            }
            snode->canonical_ID = mml_canonical_name;
            SetSnodeOwner(snode, pAnalyzer -> GetAnalyzerData());
            // pAnalyzer -> SetNodeIDsList( AppendIDRec(pAnalyzer -> NodeIDsList(), pAnalyzer ->GetAnalyzerData()-> CurrClientID(),
            //                             mml_canonical_name, mml_msup_node,
            //                             pAnalyzer -> ScrStr()) );

			      pAnalyzer -> AppendIDList(mml_canonical_name, mml_msup_node);
            snode->semantic_type = SEM_TYP_VARIABLE;
          } else
            TCI_ASSERT(0);
        }
        break;

      case BT_MOVER:
      case BT_MATRIX:
      case BT_UNIT:
      case BT_FENCED:
      case BT_NUMBER:
      case BT_SUB:
      case BT_SUP: {
          if (et == ET_POWER || et == ET_DIRECTION) {
            CreatePowerForm(base, base->next, snode, pAnalyzer);
          } else
            TCI_ASSERT(0);
        }
        break;

      case BT_TRANSFORM:{
          if (et == ET_INVERSE_INDICATOR) {
            char* mml_canonical_name = GetCanonicalIDforMathNode(mml_msup_node, pAnalyzer-> GetAnalyzerData() -> GetGrammar());
            if (!mml_canonical_name) {
              snode->error_flag = 1;
              return;
            }
            // Put the canonical name of the math object in "snode".
            snode->canonical_ID = mml_canonical_name;
            SetSnodeOwner(snode, pAnalyzer -> GetAnalyzerData());
            // Store a name-to-node back mapping record for this object.
            // pAnalyzer -> SetNodeIDsList( AppendIDRec(pAnalyzer -> NodeIDsList(), pAnalyzer ->GetAnalyzerData()-> CurrClientID(),
            //                             mml_canonical_name, mml_msup_node,
            //                             pAnalyzer -> ScrStr()) );
			      pAnalyzer -> AppendIDList(mml_canonical_name, mml_msup_node);
            int local_nodes_done;
            BUCKET_REC *br = ArgsToBucket(mml_msup_node, local_nodes_done, pAnalyzer);
            nodes_done += local_nodes_done;
            if (br)
              AppendBucketRecord(snode->bucket_list, br);
            snode->contents = DuplicateString(base->p_chdata);
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




void AnalyzeMSUB(MNODE* mml_msub_node, SEMANTICS_NODE* snode,
                 int& nodes_done, bool isLHSofDef, Analyzer* pAnalyzer)
{
  nodes_done = 1;

  MNODE* base = mml_msub_node->first_kid;
  if (IsSUBSTITUTION(mml_msub_node)) {
   	AppendNewBucketRecord(MB_UNNAMED, NULL, snode, base, true, pAnalyzer);

    MNODE *subst = base->next;
    CreateSubstBucket(subst, snode, true, pAnalyzer);

    snode->semantic_type = SEM_TYP_SUBSTITUTION;
    snode->contents = DuplicateString("substitution");
    return;
  }

  if (IsDDIFFOP(mml_msub_node)) { // D_{x^{5}y^{2}}
    MNODE *base = mml_msub_node->first_kid;
    if (base->next) {
      MNODE* sub = base->next;
      BUCKET_REC* var_bucket = MakeBucketRec(MB_DIFF_VAR, NULL);
      AppendBucketRecord(snode->bucket_list, var_bucket);
      MNODE* m_var = sub;
      if (ElementNameIs(m_var, "mrow"))
        m_var = sub->first_kid;

      SEMANTICS_NODE* s_var = GetSemanticsList(m_var, var_bucket, isLHSofDef, pAnalyzer);

      // Remove infix operators?
      s_var = RemoveInfixOps(s_var);
      var_bucket->first_child = s_var;
    } else {
      TCI_ASSERT(0);
    }

    // The expression being differentiated should follow.
    if (mml_msub_node->next) {
      int nodes_in_arg;
      BUCKET_REC *br = ArgsToBucket(mml_msub_node, nodes_in_arg, pAnalyzer);
      nodes_done += nodes_in_arg;
      if (br) {
        AppendBucketRecord(snode->bucket_list, br);
      }
    }

    snode->semantic_type = SEM_TYP_DERIVATIVE;
    snode->contents = DuplicateString("differentiate");
    if (pAnalyzer -> GetAnalyzerData()  -> GetInputNotation()) {
       pAnalyzer -> GetAnalyzerData()  -> GetInputNotation() -> n_Dxs++;
    }
    return;
  }

  if (IsBesselFunc(mml_msub_node)) {
    AnalyzeBesselFunc(mml_msub_node, snode, nodes_done, pAnalyzer);
    return;
  }

  if (base) {
    BaseType bt = GetBaseType(mml_msub_node, isLHSofDef, pAnalyzer-> GetAnalyzerData(), pAnalyzer-> GetAnalyzerData() -> GetGrammar());
    ExpType sub_type = GetSubScriptType(mml_msub_node, bt, base->next);
    if (pAnalyzer -> GetAnalyzerData() -> IsDefinedFunction (mml_msub_node)) {
       bt = BT_FUNCTION;
    }

    switch (bt) {
    case BT_OPERATOR:
      TranslateEmbellishedOp(mml_msub_node, snode, nodes_done, pAnalyzer);
      break;

    case BT_FUNCTION:{
        // Note that \log_{n} is handled below
        if (sub_type == ET_NUMBER || sub_type == ET_DECORATION  || sub_type == ET_VARIABLE) {
          char* mml_canonical_name = GetCanonicalIDforMathNode(mml_msub_node, pAnalyzer-> GetAnalyzerData() -> GetGrammar());
          if (!mml_canonical_name) {
            snode->error_flag = 1;
            return;
          }
          // Put the canonical name of the math object in "snode".
          snode->canonical_ID = mml_canonical_name;
          SetSnodeOwner(snode, pAnalyzer -> GetAnalyzerData());
          // Store a name-to-node back mapping record for this object.
          // pAnalyzer -> SetNodeIDsList( AppendIDRec(pAnalyzer -> NodeIDsList(), pAnalyzer ->GetAnalyzerData()-> CurrClientID(),
          //                             mml_canonical_name, mml_msub_node,
          //                             pAnalyzer -> ScrStr()) );
		  // 
		  pAnalyzer -> AppendIDList(mml_canonical_name, mml_msub_node);
          AnalyzeSubscriptedFunc(mml_msub_node, snode, nodes_done, pAnalyzer);
        } else {
          TCI_ASSERT(0);
        }
      }
      break;

    case BT_SUBARG_FUNCTION:{
        char* mml_canonical_name = GetCanonicalIDforMathNode(base, pAnalyzer-> GetAnalyzerData() -> GetGrammar());
        if (!mml_canonical_name) {
          snode->error_flag = 1;
          return;
        }
        // Put the canonical name of the math object in "snode".
        snode->canonical_ID = mml_canonical_name;
        SetSnodeOwner(snode, pAnalyzer -> GetAnalyzerData());
        // Store a name-to-node back mapping record for this object.
        // pAnalyzer -> SetNodeIDsList( AppendIDRec(pAnalyzer -> NodeIDsList(), pAnalyzer ->GetAnalyzerData()-> CurrClientID(),
        //                             mml_canonical_name, base, pAnalyzer -> ScrStr()) );
		// 
		pAnalyzer -> AppendIDList(mml_canonical_name, base);
        AnalyzeSubscriptedArgFunc(mml_msub_node, snode, pAnalyzer);
      }
      break;

    case BT_VARIABLE:{
        if (sub_type == ET_DECORATION) {
          char* mml_canonical_name = GetCanonicalIDforMathNode(mml_msub_node, pAnalyzer-> GetAnalyzerData() -> GetGrammar());
          if (!mml_canonical_name) {
            snode->error_flag = 1;
            return;
          }
          snode->canonical_ID = mml_canonical_name;
          SetSnodeOwner(snode, pAnalyzer -> GetAnalyzerData());
          // pAnalyzer -> SetNodeIDsList( AppendIDRec(pAnalyzer -> NodeIDsList(), pAnalyzer->GetAnalyzerData() -> CurrClientID(),
          //                             mml_canonical_name, mml_msub_node,
          //                             pAnalyzer -> ScrStr()) );
		  pAnalyzer -> AppendIDList(mml_canonical_name, mml_msub_node);
          snode->semantic_type = SEM_TYP_VARIABLE;
        } else if (sub_type == ET_VARIABLE || sub_type == ET_NUMBER ||
                   sub_type == ET_EXPRESSION) {
          if (pAnalyzer -> CmdID() == CCID_Solve_Recursion) { // Solve Recursion
            // Generate a function call
            MSUB2FuncCall(mml_msub_node, snode, pAnalyzer);
            if (pAnalyzer -> GetAnalyzerData()  -> GetInputNotation()) {
                pAnalyzer -> GetAnalyzerData()  -> GetInputNotation() -> funcarg_is_subscript++;
            }
          } else {
            // We generate a "qualified" variable here.  The engine is given enough info
            //  to treat this object as an ordinary variable, or something more complex
            //  like an indexed variable.
            CreateSubscriptedVar(mml_msub_node, false, snode, pAnalyzer);
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
      AnalyzeSubscriptedFence(mml_msub_node, snode, nodes_done, pAnalyzer);
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

void AnalyzeMSUBSUP(MNODE* mml_msubsup_node,
                   SEMANTICS_NODE* snode, 
                   int& nodes_done,
                   bool isLHSofDef, 
                   Analyzer* pAnalyzer)
{
  nodes_done = 1;

  MNODE* base = mml_msubsup_node->first_kid;
  if (IsSUBSTITUTION(mml_msubsup_node)) {


	SEMANTICS_NODE* s_base = AppendNewBucketRecord(MB_UNNAMED, NULL, snode, base, true, pAnalyzer);

    if (s_base->semantic_type == SEM_TYP_GENERIC_FENCE || s_base->semantic_type == SEM_TYP_BRACKETED_LIST)
      s_base->semantic_type = SEM_TYP_PRECEDENCE_GROUP;


    MNODE* lower = base->next;
    CreateSubstBucket(lower, snode, true, pAnalyzer);

    MNODE* upper = lower->next;
    CreateSubstBucket(upper, snode, false, pAnalyzer);

    snode->semantic_type = SEM_TYP_SUBSTITUTION;
    snode->contents = DuplicateString("doublesubstitution");

    return;
  }

  if (base) {
    BaseType bt = GetBaseType(mml_msubsup_node, isLHSofDef, pAnalyzer-> GetAnalyzerData(), pAnalyzer-> GetAnalyzerData() -> GetGrammar());
    ExpType sub_type = GetSubScriptType(mml_msubsup_node, bt, base->next);
    ExpType et = GetExpType(bt, base->next->next, pAnalyzer-> GetAnalyzerData() -> GetGrammar());

    bool done = false;

    // First look for a superscript that dictates semantics
    switch (et) {
    case ET_CONJUGATE_INDICATOR:{

        AppendNewBucketRecord(MB_UNNAMED, NULL, snode, base, true, pAnalyzer);
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
          snode->contents = DuplicateString("differentiate");

          MNODE *base = mml_msubsup_node->first_kid;
          SEMANTICS_NODE *s_func = CreateSemanticsNode();

          if (bt == BT_SUBARG_FUNCTION) {
            s_func->canonical_ID = GetCanonicalIDforMathNode(base, pAnalyzer-> GetAnalyzerData() -> GetGrammar());
            AnalyzeSubscriptedArgFunc(mml_msubsup_node, s_func, pAnalyzer);
            
            BUCKET_REC* base_bucket = MakeParentBucketRec(MB_UNNAMED, s_func);            
            AppendBucketRecord(snode->bucket_list, base_bucket);
            base_bucket->first_child = s_func;
            
          } else {
            char *mml_canonical_name = NULL;
            U32 zh_ln = 0;
            mml_canonical_name =
              AppendStr2HeapStr(mml_canonical_name, zh_ln, "msub");
            
            const char* tmp = GetCanonicalIDforMathNode(base, pAnalyzer-> GetAnalyzerData() -> GetGrammar());
            if (tmp) {
              mml_canonical_name = AppendStr2HeapStr(mml_canonical_name, zh_ln, tmp);
              delete[] tmp;
            }
            
            tmp = GetCanonicalIDforMathNode(base->next, pAnalyzer-> GetAnalyzerData() -> GetGrammar());
            if (tmp) {
              mml_canonical_name = AppendStr2HeapStr(mml_canonical_name, zh_ln, tmp);
              delete[] tmp;
            }

            // Put the canonical name of the math object in "snode".
            s_func->canonical_ID = mml_canonical_name;
            SetSnodeOwner(s_func, pAnalyzer -> GetAnalyzerData());
            // Store a name-to-node back mapping record for this object.
            // pAnalyzer -> SetNodeIDsList( AppendIDRec(pAnalyzer -> NodeIDsList(), pAnalyzer ->GetAnalyzerData()-> CurrClientID(),
            //                             mml_canonical_name, mml_msubsup_node,
            //                             pAnalyzer -> ScrStr()) );
			      pAnalyzer -> AppendIDList(mml_canonical_name, mml_msubsup_node);
            AnalyzeSubscriptedFunc(mml_msubsup_node, s_func, nodes_done, pAnalyzer);
            
            BUCKET_REC* base_bucket = MakeParentBucketRec(MB_UNNAMED, s_func);
            AppendBucketRecord(snode->bucket_list, base_bucket);
            base_bucket->first_child = s_func;
            

            if (pAnalyzer -> GetAnalyzerData() -> GetDE_ind_vars()) {
              BUCKET_REC *dvar_bucket =
                AddVarToBucket(MB_DIFF_VAR, pAnalyzer -> GetAnalyzerData() -> GetDE_ind_vars(), pAnalyzer -> GetAnalyzerData());
              AppendBucketRecord(snode->bucket_list, dvar_bucket);
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
        TranslateEmbellishedOp(mml_msubsup_node, snode, nodes_done, pAnalyzer);
        break;

      case BT_VARIABLE:{
          MNODE* mml_base = mml_msubsup_node->first_kid;
          MNODE* mml_sub = mml_msubsup_node->first_kid->next;
          MNODE* mml_power = mml_base->next->next;
          
          //BUCKET_REC* bucket = FindBucketRec(snode->bucket_list, MB_SCRIPT_BASE);


          //if (bucket) {
          //  DisposeSList(bucket->first_child);
          SEMANTICS_NODE* s_var = CreateSemanticsNode();
          CreateSubscriptedVar(mml_msubsup_node, true, s_var, pAnalyzer);

            //bucket->first_child = s_var;
            //s_var->parent = bucket;
          //} else
          //  TCI_ASSERT(0);
          //CreatePowerForm(mml_base, mml_power, snode, pAnalyzer);
          snode->semantic_type = SEM_TYP_POWERFORM;
          mml_base -> next = NULL;

          BUCKET_REC* base_bucket = MakeBucketRec(MB_SCRIPT_BASE, NULL);
          AppendBucketRecord(snode->bucket_list, base_bucket);
          base_bucket->first_child = s_var;
          s_var->parent = base_bucket;

         // AppendNewBucketRecord(MB_SCRIPT_BASE, NULL, snode, mml_base, true, pAnalyzer);
          AppendNewBucketRecord(MB_SCRIPT_UPPER, NULL, snode, mml_power, true, pAnalyzer);


        }
        break;

      case BT_SUBARG_FUNCTION:{
          MNODE* f_name = mml_msubsup_node->first_kid;
          MNODE* f_arg = f_name->next;
          MNODE* f_exp = f_arg->next;

          SEMANTICS_NODE* s_func = CreateSemanticsNode();

          // Create a canonical name for "s_func".
          s_func->canonical_ID = GetCanonicalIDforMathNode(f_name, pAnalyzer-> GetAnalyzerData() -> GetGrammar());
          AnalyzeSubscriptedArgFunc(mml_msubsup_node, s_func, pAnalyzer);
          SetSnodeOwner(s_func, pAnalyzer -> GetAnalyzerData());

          // Create a semantic power form
          BUCKET_REC* base_bucket = MakeBucketRec(MB_SCRIPT_BASE, NULL);
          AppendBucketRecord(snode->bucket_list, base_bucket);
          base_bucket->first_child = s_func;
          s_func->parent = base_bucket;

          BUCKET_REC *power_bucket = MakeBucketRec(MB_SCRIPT_UPPER, NULL);
          AppendBucketRecord(snode->bucket_list, power_bucket);
          SEMANTICS_NODE* s_power = GetSemanticsFromNode(f_exp, power_bucket, pAnalyzer);
          power_bucket->first_child = s_power;
          s_power->parent = power_bucket;

          snode->semantic_type = SEM_TYP_POWERFORM;
        }
        break;

      case BT_FENCED: {
          MNODE* f_fence = mml_msubsup_node->first_kid;
          MNODE* f_norm = f_fence->next;
          MNODE* f_exp = f_norm->next;

          SEMANTICS_NODE* s_fence = CreateSemanticsNode();
          AnalyzeSubscriptedFence(mml_msubsup_node, s_fence, nodes_done, pAnalyzer);

          // Create a semantic power form
          BUCKET_REC* base_bucket = MakeBucketRec(MB_SCRIPT_BASE, NULL);
          AppendBucketRecord(snode->bucket_list, base_bucket);
          base_bucket->first_child = s_fence;
          s_fence->parent = base_bucket;

          BUCKET_REC* power_bucket = MakeBucketRec(MB_SCRIPT_UPPER, NULL);
          AppendBucketRecord(snode->bucket_list, power_bucket);
          SEMANTICS_NODE* s_power = GetSemanticsFromNode(f_exp, power_bucket, pAnalyzer);
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



void AnalyzeMOVER(MNODE* mml_mover_node, 
                  SEMANTICS_NODE* snode,
                  int& nodes_done, 
                  bool isLHSofDef, 
                  Analyzer* pAnalyzer)
{
  nodes_done = 1;

  MNODE* base = mml_mover_node->first_kid;
  if (base) {
    BaseType bt = GetBaseType(mml_mover_node, isLHSofDef, pAnalyzer-> GetAnalyzerData(), pAnalyzer-> GetAnalyzerData() -> GetGrammar());
    AccentType top_type = GetAboveType(bt, base->next, pAnalyzer-> GetAnalyzerData() -> GetGrammar());

    if (top_type == OT_BAR && base->p_chdata && !strcmp(base->p_chdata, "lim")) {
     	TranslateEmbellishedOp(mml_mover_node, snode, nodes_done, pAnalyzer);
     	return;
    }

    bool done = false;

    // First look for a top decoration that dictates semantics
    switch (top_type) {
    case OT_DOT:
    case OT_DDOT:
    case OT_DDDOT:
    case OT_DDDDOT:
      if (pAnalyzer -> Get_dot_is_derivative()) {
        int n_dots = top_type - OT_DOT + 1;
        AnalyzeDotDerivative(mml_mover_node, n_dots, snode, nodes_done, pAnalyzer);
        if (pAnalyzer -> GetAnalyzerData()  -> GetInputNotation()) {
          pAnalyzer -> GetAnalyzerData()  -> GetInputNotation() -> n_dotaccents++;
        }
        done = true;
      }
      break;

    case OT_BAR:
      if (pAnalyzer -> Get_overbar_conj()) {
	    AppendNewBucketRecord(MB_UNNAMED, NULL, snode, base, true, pAnalyzer);

        snode->semantic_type = SEM_TYP_CONJUGATE;
        if (pAnalyzer -> GetAnalyzerData()  -> GetInputNotation()) {
          pAnalyzer ->  GetAnalyzerData()  -> GetInputNotation() -> n_overbars++;
        }
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
        char *mml_canonical_name = GetCanonicalIDforMathNode(mml_mover_node, pAnalyzer-> GetAnalyzerData() -> GetGrammar());
        if (!mml_canonical_name) {
          snode->error_flag = 1;
          return;
        }
        snode->canonical_ID = mml_canonical_name;
        SetSnodeOwner(snode, pAnalyzer -> GetAnalyzerData());
        // pAnalyzer -> SetNodeIDsList( AppendIDRec(pAnalyzer -> NodeIDsList(), pAnalyzer ->GetAnalyzerData()-> CurrClientID(),
        //                             mml_canonical_name, mml_mover_node,
        //                             pAnalyzer -> ScrStr()) );
		pAnalyzer -> AppendIDList(mml_canonical_name, mml_mover_node);
        snode->semantic_type = SEM_TYP_VARIABLE;
      }
    }
  } else
    TCI_ASSERT(0);
}



/* Much work remains in the following - currently it only handles
   something like "\limfunc{lim}_{x\rightarrow\infty}\frac{1}{x}"
*/

void AnalyzeMUNDER(MNODE * mml_munder_node, SEMANTICS_NODE * snode,
                             int& nodes_done, bool isLHSofDef, Analyzer* pAnalyzer)
{
  nodes_done = 1;

  MNODE *base = mml_munder_node->first_kid;
  if (base) {
    BaseType bt = GetBaseType(mml_munder_node, isLHSofDef, pAnalyzer-> GetAnalyzerData(), pAnalyzer-> GetAnalyzerData() -> GetGrammar());
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
        TranslateEmbellishedOp(mml_munder_node, snode, nodes_done, pAnalyzer);
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
        char *mml_canonical_name = GetCanonicalIDforMathNode(mml_munder_node, pAnalyzer-> GetAnalyzerData() -> GetGrammar());
        if (!mml_canonical_name) {
          snode->error_flag = 1;
          return;
        }
        snode->canonical_ID = mml_canonical_name;
        SetSnodeOwner(snode, pAnalyzer -> GetAnalyzerData());
        // pAnalyzer -> SetNodeIDsList( AppendIDRec(pAnalyzer -> NodeIDsList(), pAnalyzer ->GetAnalyzerData()-> CurrClientID(),
        //                             mml_canonical_name, mml_munder_node,
        //                             pAnalyzer -> ScrStr()) );
		pAnalyzer -> AppendIDList(mml_canonical_name, mml_munder_node);
        snode->semantic_type = SEM_TYP_VARIABLE;
      }
    }
  } else
    TCI_ASSERT(0);
}

void AnalyzeMUNDEROVER(MNODE * mml_munderover_node,
                                 SEMANTICS_NODE * snode, int& nodes_done,
                                 bool isLHSofDef, Analyzer* pAnalyzer)
{
  nodes_done = 1;

  MNODE *base = mml_munderover_node->first_kid;
  if (base) {
    BaseType bt = GetBaseType(mml_munderover_node, isLHSofDef, pAnalyzer-> GetAnalyzerData(), pAnalyzer-> GetAnalyzerData() -> GetGrammar());
    switch (bt) {

    case BT_OPERATOR:
      TranslateEmbellishedOp(mml_munderover_node, snode, nodes_done, pAnalyzer);
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

void AnalyzeMTABLE(MNODE* mml_mtable_node,
                   SEMANTICS_NODE* info, 
                   int& nodes_done, 
                   Analyzer* pAnalyzer)
{
  nodes_done = 1;

  int row_tally = 0;
  int max_cols = 0;
  BUCKET_REC* cell_list = NULL;

  if (mml_mtable_node->first_kid) {
    // count rows and columns
    MNODE* row_rover = mml_mtable_node->first_kid;
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
        MNODE* col_rover = row_rover->first_kid;

        int col_counter = 0;
        while (col_counter < max_cols) {
          MNODE* mml_cell = NULL;
          int nodes_in_arg = 0;
          if (col_rover) {
            mml_cell = col_rover;
            nodes_in_arg = 1;
            col_rover = col_rover->next;
          }

          BUCKET_REC* cell_bucket = MakeBucketRec(MB_UNNAMED, NULL);
          AppendBucketRecord(cell_list, cell_bucket);
          col_counter++;

          if (mml_cell) {
            if (ElementNameIs(mml_cell, "mtd")) {
              if (mml_cell->first_kid)
                mml_cell = mml_cell->first_kid;
              else
                mml_cell = NULL;
            }
            if (mml_cell) {
              SEMANTICS_NODE *s_cell = GetSemanticsList(mml_cell, cell_bucket, false, pAnalyzer);
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
  info->contents = DuplicateString("matrix");

  info->semantic_type = SEM_TYP_TABULATION;
  info->nrows = row_tally;
  info->ncols = max_cols;
  info->bucket_list = cell_list;
}



/*
       <mml:mi mathcolor="gray">sin</mml:mi>
       <mml:mo>&ApplyFunction;</mml:mo>
       <mml:mi>x</mml:mi>
*/

BUCKET_REC* ArgsToBucket(MNODE* func_node, int& nodes_done, Analyzer* pAnalyzer)
{
  nodes_done = 0;
  BUCKET_REC* a_rec = NULL;

  int local_nodes_done = 0;
  if (func_node && func_node->next) {
    MNODE* mml_rover = func_node->next;


    // step over any whitespace
    while (IsWhiteSpace(mml_rover)) {
      local_nodes_done++;
      mml_rover = mml_rover->next;
    }

    bool found_ap = false;  // Apply functrion
    bool found_it = false;  // Invisible times

    // look for <mo>&ApplyFunction;</mo>  or invisible times
    if (mml_rover && ElementNameIs(mml_rover, "mo")) {
      U32 unicodes[8];
      int content_tally = ChData2Unicodes(mml_rover->p_chdata, unicodes, 8, pAnalyzer-> GetAnalyzerData() -> GetGrammar());
      if (content_tally == 1 && (unicodes[0] == 0x2061 || unicodes[0] == 0x2062) ) {
        found_ap = (unicodes[0] == 0x2061);
        found_it = (unicodes[0] == 0x2062);
        local_nodes_done++;
        mml_rover = mml_rover->next;
      }
    }

    if (found_it) return NULL;

    // step over any whitespace
    while (IsWhiteSpace(mml_rover)) {
      local_nodes_done++;
      mml_rover = mml_rover->next;
    }

    if (mml_rover) {
      bool got_arg = false;
      if (ElementNameIs(mml_rover, "mrow")) {
        local_nodes_done++;
        a_rec = ArgBucketFromMROW(mml_rover, pAnalyzer);
        got_arg = true;
      } else if (ElementNameIs(mml_rover, "mo")) {
        int n_nodes;
        a_rec = GetParenedArgs(mml_rover, n_nodes, pAnalyzer);
        if (a_rec) {
          local_nodes_done += n_nodes;
          got_arg = true;
        }
      } else if (IsArgDelimitingFence(mml_rover)) {
        local_nodes_done++;
        a_rec = GetFencedArgs(mml_rover, pAnalyzer);
        got_arg = true;
      } else {                  // Here, the rest of the nodes in the list become the arg
        // sin &af; cos &af; x
        a_rec = MakeBucketRec(MB_UNNAMED, NULL);
        SEMANTICS_NODE* s_arg = GetSemanticsList(mml_rover, a_rec, false, pAnalyzer);
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



BUCKET_REC* ArgBucketFromMROW(MNODE* mml_mrow, Analyzer* pAnalyzer)
{
  BUCKET_REC *rv = NULL;

  if (mml_mrow && mml_mrow->first_kid) {
    MNODE *candidate = mml_mrow->first_kid;

    if (ElementNameIs(candidate, "mo")) {

        int nodes_done;
        rv = GetParenedArgs(candidate, nodes_done, pAnalyzer);

    } else if (ElementNameIs(candidate, "mfenced")) {

        rv = GetFencedArgs(candidate, pAnalyzer);

    } else {

        // we get here when processing trigargs, "sinh at"
        rv = MakeBucketRec(MB_UNNAMED, NULL);
        SEMANTICS_NODE* s_arg = GetSemanticsList(candidate, rv, false, pAnalyzer);
        rv->first_child = s_arg;

    }
  }
  return rv;
}



BUCKET_REC* GetParenedArgs(MNODE* mml_mo, int& nodes_done, Analyzer* pAnalyzer)
{
  BUCKET_REC* rv = NULL;
  nodes_done = 0;
  int local_nodes_done = 0;

  // Span to matching ")"
  bool matched_parens = false;
  int nodes_within_parens = 0;

  U32 unicodes[8];
  int content_tally = ChData2Unicodes(mml_mo->p_chdata, unicodes, 8, pAnalyzer-> GetAnalyzerData() -> GetGrammar());
  
  if (content_tally == 1 && unicodes[0] == '(') {
    local_nodes_done++;

    MNODE* rover = mml_mo->next;
    while (rover) {
      local_nodes_done++;
      int content_tally = ChData2Unicodes(rover->p_chdata, unicodes, 8, pAnalyzer-> GetAnalyzerData() -> GetGrammar());
      if (content_tally == 1 && unicodes[0] == ')') {
        matched_parens = true;
        break;
      }
      nodes_within_parens++;
      rover = rover->next;
    }
  }

  if (matched_parens) {

    MNODE* arg_ptr = mml_mo->next;

    // descend into an mrow here if necessary
    if (nodes_within_parens == 1) {
      if (ElementNameIs(arg_ptr, "mrow"))
        arg_ptr = arg_ptr->first_kid;
    }

    // traverse the first level nodes inside the parens
    bool done = false;
    while (!done) {
      // span the first arg
      MNODE* arg_first_obj = arg_ptr;
      int nodes_in_arg = 0;
      while (arg_ptr) {
        int content_tally = ChData2Unicodes(arg_ptr->p_chdata, unicodes, 8, pAnalyzer-> GetAnalyzerData() -> GetGrammar());
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

      BUCKET_REC* new_a_rec = MakeBucketRec(MB_UNNAMED, NULL);
      AppendBucketRecord(rv, new_a_rec);

      SEMANTICS_NODE* s_arg = GetSemanticsList(arg_first_obj, new_a_rec, nodes_in_arg, false, pAnalyzer);
      
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



BUCKET_REC* GetFencedArgs(MNODE* mml_fence, Analyzer* pAnalyzer)
{
  BUCKET_REC* rv = NULL;

  if (mml_fence && mml_fence->first_kid) {
    MNODE* rover = mml_fence->first_kid;
    while (rover) {
      BUCKET_REC* new_a_rec = MakeBucketRec(MB_UNNAMED, NULL);
      AppendBucketRecord(rv, new_a_rec);
      SEMANTICS_NODE *s_arg = GetSemanticsFromNode(rover, new_a_rec, pAnalyzer);
      new_a_rec->first_child = s_arg;
      if (s_arg)
        s_arg->parent = new_a_rec;

      rover = rover->next;
    }
  }

  return rv;
}

// Note that the MML list is traversed from left to right.
//  Operators are located and their operands are translated
//  and recorded as children of the generated semantic operator node.

SEMANTICS_NODE* GetSemanticsList(MNODE* dMML_list,
                                 BUCKET_REC* parent_bucket,
                                 bool isLHSofDef, 
                                 Analyzer* pAnalyzer)
{
  return GetSemanticsList(dMML_list, parent_bucket, ALL_NODES, isLHSofDef, pAnalyzer);
}

SEMANTICS_NODE* GetSemanticsList(MNODE* dMML_list,
                                 BUCKET_REC* parent_bucket,
                                 int mml_node_lim,
                                 bool isLHSofDef,
                                 Analyzer* pAnalyzer)
{
  TCI_ASSERT(CheckLinks(dMML_list));

  SEMANTICS_NODE* head = NULL;
  SEMANTICS_NODE* tail;

  int mml_nodes_done = 0;
  MNODE* rover = dMML_list;
  while (rover && mml_nodes_done < mml_node_lim) {

    SEMANTICS_NODE* new_node = NULL;
    // Look ahead in the MML list for an operator.
    OpIlk op_ilk;
    int advance;
    MNODE* mo = LocateOperator(rover, op_ilk, advance);

    int mml_nodes_remaining = mml_node_lim - mml_nodes_done;

    if (mo && mml_nodes_remaining > 1) {
        
        if (ContentIs(mo, "&#x2061;")) { // ApplyFunction - translate the function call

            int l_nodes_done = 0;
            new_node = SNodeFromMNodes (rover, l_nodes_done, isLHSofDef, pAnalyzer);
            
            while (l_nodes_done) {
              mml_nodes_done++;
              rover = rover->next;
              l_nodes_done--;
            }
		} else {
          if (ContentIs(mo, "&#x2062;")) {
             int break_here = 14;
          }

          SEMANTICS_NODE* l_operand = NULL;
          SEMANTICS_NODE* r_operand = NULL;

          if (op_ilk == OP_infix || op_ilk == OP_postfix) {
            if (head) {
              l_operand = head;
              // Here we're assuming ALL operators in source MML list have
              //  the same precedence (ie. well-formed MML), and are left-associative.
            } else {
              // translate the left operand
              int l_nodes_done = 0;
              l_operand = SNodeFromMNodes (rover, l_nodes_done, isLHSofDef, pAnalyzer);
              while (l_nodes_done) {
                mml_nodes_done++;
                rover = rover->next;
                l_nodes_done--;
              }
            }
          }
          // translate the operator
          int op_nodes_done = 0;
          new_node = SNodeFromMNodes(rover, op_nodes_done, isLHSofDef, pAnalyzer);
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
              r_operand = SNodeFromMNodes(rover, r_nodes_done, isLHSofDef, pAnalyzer);
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
          r_operand = SNodeFromMNodes(rover, r_nodes_done, isLHSofDef, pAnalyzer);
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
          BUCKET_REC* arg1_bucket = MakeParentBucketRec(MB_UNNAMED, l_operand);
          
          AppendBucketRecord(new_node->bucket_list, arg1_bucket);
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
      new_node = SNodeFromMNodes(rover, nodes_done, isLHSofDef, pAnalyzer);
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



SEMANTICS_NODE* SNodeFromMNodes(MNODE* mml_node,
                                int& mml_nodes_done,
                                bool isLHSofDef, 
                                Analyzer* pAnalyzer)
{
  SEMANTICS_NODE* rv = NULL;
  if (mml_node) {
    int local_nodes_done = 1;
    rv = CreateSemanticsNode();
    const char* mml_element = mml_node->src_tok;
    size_t ln = strlen(mml_element);

    switch (ln) {
    case 2:{
        if (StringEqual(mml_element, "mi")) {

            AnalyzeMI(mml_node, rv, local_nodes_done, isLHSofDef, pAnalyzer);

        } else if (StringEqual(mml_element, "mo")) {

            AnalyzeMO(mml_node, rv, local_nodes_done, pAnalyzer);

        } else if (StringEqual(mml_element, "mn")) {
             // Seems that mixed numbers should have been removed back in TreeToFixupForm()

             //bool do_mixed = false;

             //if (IsWholeNumber(mml_node) && IsWholeFrac(mml_node->next)) {
             //  do_mixed = !IsPositionalChild(mml_node);
             //}

             //if (do_mixed) {
             //  AnalyzeMixedNum(mml_node, rv, pAnalyzer);
             //  local_nodes_done++;
             //} else
               AnalyzeMN(mml_node, rv, pAnalyzer);

        } else {
          TCI_ASSERT(0);
        }
      }
      break;

    case 3:{
        if (StringEqual(mml_element, "mtd")) {

            TCI_ASSERT(0);

        } else if (StringEqual(mml_element, "mtr")) {
            
            TCI_ASSERT(0);

        } else {

            TCI_ASSERT(0);
        }
      }
      break;

    case 4:{
        if (StringEqual(mml_element, "mrow")) {

            rv->semantic_type = SEM_TYP_PRECEDENCE_GROUP;
            if (mml_node->first_kid) {

              BUCKET_REC* new_a_rec = MakeBucketRec(MB_UNNAMED, NULL);
              rv->bucket_list = AppendBucketRec(NULL, new_a_rec);
              SEMANTICS_NODE* s_node = GetSemanticsList(mml_node->first_kid, new_a_rec, isLHSofDef, pAnalyzer);

              new_a_rec->first_child = s_node;
              if (s_node) 
                s_node->parent = new_a_rec;
            }

        } else if (StringEqual(mml_element, "msup")) {

            AnalyzeMSUP(mml_node, rv, local_nodes_done, isLHSofDef, pAnalyzer);

        } else if (StringEqual(mml_element, "msub")) {

            AnalyzeMSUB(mml_node, rv, local_nodes_done, isLHSofDef, pAnalyzer);

        } else if (StringEqual(mml_element, "math")) {

            rv->semantic_type = SEM_TYP_MATH_CONTAINER;
            if (mml_node->first_kid) {
              MNODE* cont = mml_node->first_kid;
              // descend into a redundant mrow, if it exists
              while (cont && !cont->next && ElementNameIs(cont, "mrow"))
                cont = cont->first_kid;

              if (cont) {
                BUCKET_REC* new_a_rec = MakeBucketRec(MB_UNNAMED, NULL);
                rv->bucket_list = AppendBucketRec(NULL, new_a_rec);
                SEMANTICS_NODE* s_node = GetSemanticsList(cont, new_a_rec, ALL_NODES, isLHSofDef, pAnalyzer);
                new_a_rec->first_child = s_node;
                if (s_node)
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
        if (StringEqual(mml_element, "mfrac")) {

            AnalyzeMFRAC(mml_node, rv, local_nodes_done, pAnalyzer);

        } else if (StringEqual(mml_element, "msqrt")) {

            AnalyzeMSQRT(mml_node, rv, local_nodes_done, pAnalyzer);

        } else if (StringEqual(mml_element, "mroot")) {

            AnalyzeMROOT(mml_node, rv, local_nodes_done, pAnalyzer);

        } else if (StringEqual(mml_element, "mover")) {

            AnalyzeMOVER(mml_node, rv, local_nodes_done, isLHSofDef, pAnalyzer);

        } else if (StringEqual(mml_element, "mtext")) {

            AnalyzeMTEXT(mml_node, rv, local_nodes_done, pAnalyzer);

        } else {

            TCI_ASSERT(0);
        }
      }
      break;

    case 6:{
        if (StringEqual(mml_element, "munder")) {

            AnalyzeMUNDER(mml_node, rv, local_nodes_done, isLHSofDef, pAnalyzer);

        } else if (StringEqual(mml_element, "mstyle")) {

            TCI_ASSERT(0);

        } else if (StringEqual(mml_element, "mtable")) {

            AnalyzeMTABLE(mml_node, rv, local_nodes_done, pAnalyzer);

        } else if (StringEqual(mml_element, "mspace")) {

        } else {

            TCI_ASSERT(0);
        }
      }
      break;

    case 7:{
        if (!strcmp(mml_element, "mfenced")) {

            AnalyzeMFENCED(mml_node, rv, local_nodes_done, pAnalyzer);

        } else if (!strcmp(mml_element, "msubsup")) {

            AnalyzeMSUBSUP(mml_node, rv, local_nodes_done, isLHSofDef, pAnalyzer);

        } else if (!strcmp(mml_element, "mpadded")) {

            rv->semantic_type = SEM_TYP_PRECEDENCE_GROUP;
            if (mml_node->first_kid) {
			  AppendNewBucketRecord(MB_UNNAMED, NULL, rv, mml_node->first_kid, false, pAnalyzer);
            }

        } else {

            TCI_ASSERT(0);
        }
      }
      break;

    case 10:{
        if (StringEqual(mml_element, "munderover")) {

            AnalyzeMUNDEROVER(mml_node, rv, local_nodes_done, isLHSofDef, pAnalyzer);

        } else {

            TCI_ASSERT(0);
        }
      }
      break;

    case 11:{
        if (StringEqual(mml_element, "maligngroup")) {

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



SEMANTICS_NODE* GetSemanticsFromNode(MNODE* mml_node, BUCKET_REC* bucket, Analyzer* pAnalyzer)
{
  SEMANTICS_NODE *rv = NULL;
  if (mml_node) {
    if (ElementNameIs(mml_node, "mrow")) {
      if (mml_node->first_kid)
        rv = GetSemanticsList(mml_node->first_kid, bucket, false, pAnalyzer);
      else
        TCI_ASSERT(0);
    } else
      rv = GetSemanticsList(mml_node, bucket, 1, false, pAnalyzer);
  }

  return rv;
}

void AnalyzeMixedNum(MNODE* mml_mn, SEMANTICS_NODE* s_node, Analyzer* pAnalyzer)
{
  BUCKET_REC* whole_bucket = MakeBucketRec(MB_MN_WHOLE, NULL);
  if (s_node)
     AppendBucketRecord(s_node->bucket_list, whole_bucket);

  MNODE* save = mml_mn->next;
  mml_mn->next = NULL;
  SEMANTICS_NODE* s_whole = GetSemanticsList(mml_mn, whole_bucket, 1, false, pAnalyzer);
  mml_mn->next = save;

  whole_bucket->first_child = s_whole;
  s_whole->parent = whole_bucket;

  BUCKET_REC* frac_bucket = MakeBucketRec(MB_MN_FRACTION, NULL);
  if (s_node)
     AppendBucketRecord(s_node->bucket_list, frac_bucket);
  SEMANTICS_NODE* s_frac = GetSemanticsList(mml_mn->next, frac_bucket, 1, false, pAnalyzer);

  frac_bucket->first_child = s_frac;
  s_frac->parent = frac_bucket;
  if (s_node)
     s_node->semantic_type = SEM_TYP_MIXEDNUMBER;
}

////////////////////////////// START SCRIPT HANDLING


// When the "base" of a MML schemata like <msup>, <mover>, etc.
//  is an operator, the translation is handled here.

void TranslateEmbellishedOp(MNODE* mml_embellop_node,
                            SEMANTICS_NODE* snode,
                            int& nodes_done,
                            Analyzer* pAnalyzer)
{
  nodes_done = 1;
  
  TCI_ASSERT(CheckLinks(mml_embellop_node));
  MNODE* base = mml_embellop_node->first_kid;

  if (base) {                   // the underlying operator - \int, \sum, etc.

    char* tmp = DuplicateString(base->p_chdata);
    snode -> contents = tmp;
    // SEM_TYP_BIGOP_INTEGRAL, SEM_TYP_BIGOP_SUM, or something else
    SemanticVariant n_integs;
    SemanticType bigop_type = GetBigOpType(tmp, n_integs);

    if (bigop_type) {
      snode->semantic_type = bigop_type;
      if (base->next) {         // the lower limit

        MNODE *mml_ll = base->next;
        SEMANTICS_NODE *s_ll = GetSemanticsFromNode(mml_ll, NULL, pAnalyzer);

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
                
                AppendBucketRecord(snode->bucket_list, b_rover);

                // Might have mapped "i" to "imaginaryi".
                if (b_rover->first_child) {
                  SEMANTICS_NODE* s_var = b_rover->first_child;
                  if (s_var->semantic_type == SEM_TYP_UCONSTANT) {
                    
                    if (StringEqual(s_var->contents, "i")) {
                        
                        s_var->semantic_type = SEM_TYP_VARIABLE;
                        pAnalyzer -> GetAnalyzerData() -> Set_i_is_imaginary( false );

                    } else if (StringEqual(s_var->contents, "j")) {

                        s_var->semantic_type = SEM_TYP_VARIABLE;
                        pAnalyzer -> GetAnalyzerData() -> Set_j_is_imaginary( false );

                    } else if (StringEqual(s_var->contents, "e")) {

                        s_var->semantic_type = SEM_TYP_VARIABLE;
                        pAnalyzer -> GetAnalyzerData() -> Set_e_is_Euler( false );

                    }
                  }
                }

                b_expr->bucket_ID = MB_LOWERLIMIT;
                TCI_ASSERT(b_expr->next == NULL);
                
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
          BUCKET_REC* ll_bucket = MakeBucketRec(MB_LOWERLIMIT, NULL);
          AppendBucketRecord(snode->bucket_list, ll_bucket);
          ll_bucket->first_child = s_ll;
          s_ll->parent = ll_bucket;
        }
        if (mml_ll->next) {     // the upper limit
          MNODE* mml_ul = mml_ll->next;
          BUCKET_REC* ul_bucket = MakeBucketRec(MB_UPPERLIMIT, NULL);
          AppendBucketRecord(snode->bucket_list, ul_bucket);
          SEMANTICS_NODE* s_ul = GetSemanticsFromNode(mml_ul, ul_bucket, pAnalyzer);
          ul_bucket->first_child = s_ul;
          s_ul->parent = ul_bucket;
        }
      } else {
        TCI_ASSERT(0);
      }

      int nodes_in_arg;
      OperandToBucketList(mml_embellop_node, bigop_type, snode, nodes_in_arg, pAnalyzer);
      nodes_done += nodes_in_arg;
    } else {                    // Here, bigoptype is 0 - not sum or int
      // look for varinjlim, varliminf, varprojlim
      int var_lim_type = GetVarLimType(tmp, base);
      if (var_lim_type) {
        int nodes_in_arg;
        BUCKET_REC *br = ArgsToBucket(mml_embellop_node, nodes_in_arg, pAnalyzer);
        nodes_done += nodes_in_arg;
        if (br)
          AppendBucketRecord(snode->bucket_list, br);

        snode->semantic_type = SEM_TYP_PREFIX_OP;
        delete[] snode->contents;

        char* op_name = NULL;

        if (var_lim_type == 1)
          op_name = "injlim";
        else if (var_lim_type == 2)
          op_name = "liminf";
        else if (var_lim_type == 3)
          op_name = "projlim";
        else if (var_lim_type == 4)
          op_name = "limsup";

        if (op_name) {
          snode->contents = DuplicateString(op_name);
        } else {
          TCI_ASSERT(0);
        }
      } else if (IsLaplacian(base)) {
        int nodes_in_arg;
        BUCKET_REC *br = ArgsToBucket(mml_embellop_node, nodes_in_arg, pAnalyzer);
        nodes_done += nodes_in_arg;
        if (br)
          AppendBucketRecord(snode->bucket_list, br);

        snode->semantic_type = SEM_TYP_PREFIX_OP;

        delete[] snode->contents;
        snode->contents = DuplicateString("Laplacian");
      } else {                  // for now, I'm assuming a limfunc here!
        if (base->next) {
          MNODE *mml_ll = base->next;
          BUCKET_REC *ll_bucket = MakeBucketRec(MB_LOWERLIMIT, NULL);
          AppendBucketRecord(snode->bucket_list, ll_bucket);

          SEMANTICS_NODE* s_ll = GetSemanticsFromNode(mml_ll, ll_bucket, pAnalyzer);
          ll_bucket->first_child = s_ll;

          // The embellished operator MAY be a standard math operator
          //  that takes a limit.  If so, we can further decompose the limit.
          int req_limit_format = GetLimitFormat(snode->contents, pAnalyzer-> GetAnalyzerData() -> GetGrammar());
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
        BUCKET_REC *br = ArgsToBucket(mml_embellop_node, nodes_in_arg, pAnalyzer);
        nodes_done += nodes_in_arg;
        if (br)
          AppendBucketRecord(snode->bucket_list, br);
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

void OperandToBucketList(MNODE * big_op_node, SemanticType bigop_type,
                                   SEMANTICS_NODE * bigop_snode,
                                   int& nodes_done, Analyzer* pAnalyzer)
{
  TCI_ASSERT(CheckLinks(big_op_node));
  nodes_done = 0;

  int local_nodes_done = 0;
  if (big_op_node && big_op_node->next) {
    MNODE *mml_operand = big_op_node->next;
	  TCI_ASSERT(CheckLinks(mml_operand));

    // step over any whitespace
    while (IsWhiteSpace(mml_operand)) {
      local_nodes_done++;
      mml_operand = mml_operand->next;
    }
    TCI_ASSERT(CheckLinks(mml_operand));
    if (mml_operand) {
      local_nodes_done++;
      if (bigop_type == SEM_TYP_BIGOP_SUM) {
        BUCKET_REC *a_rec = MakeBucketRec(MB_OPERAND, NULL);
        
        AppendBucketRecord(bigop_snode->bucket_list, a_rec);
        SEMANTICS_NODE *s_arg = GetSemanticsFromNode(mml_operand, a_rec, pAnalyzer);
        a_rec->first_child = s_arg;
        s_arg->parent = a_rec;
      } else {                  // It's an integral
        bool nested_operand = true;
        MNODE *integrand_ender = NULL;
        bool frac_operand = false;
        bool dx_is_nested = false;

        if (ElementNameIs(mml_operand, "mfrac")) {
          nested_operand = true;
          frac_operand = true;

          MNODE *num = mml_operand->first_kid;
          if (num) {
            integrand_ender = Find_dx(num, dx_is_nested);
            TCI_ASSERT(integrand_ender);
          } else
            TCI_ASSERT(0);
        } else if (ElementNameIs(mml_operand, "mrow")) {
          integrand_ender = Find_dx(mml_operand, dx_is_nested);
          // SWP accepts indefinite integrals with no "dx" - so we're OK here.
        } else if (ElementNameIs(mml_operand, "mfenced")) {
          nested_operand = false;
          integrand_ender = mml_operand->next;
        } else {
          TCI_ASSERT(!"Integrand not nicely grouped.  What to do?");
        }
        if (integrand_ender) {
          // "dx" should be nested in an mrow of it's own
          if (ElementNameIs(integrand_ender, "mrow")) {
            int tally = 0;
            MNODE *s_rover = integrand_ender;
            while (1) {
              MNODE *var_rover = s_rover->first_kid;
              var_rover = var_rover->next;  // step over "d"

              BUCKET_REC* v_bucket = MakeBucketRec(MB_INTEG_VAR, NULL);
              AppendBucketRecord(bigop_snode->bucket_list, v_bucket);
              SEMANTICS_NODE* s_arg = GetSemanticsFromNode(var_rover, v_bucket, pAnalyzer);
              v_bucket->first_child = s_arg;
              s_arg->parent = v_bucket;
              if (!nested_operand)
                local_nodes_done++;
              tally++;

              if (frac_operand && !dx_is_nested)
                break;
              if (s_rover->next) {  // dx*dy...
                s_rover = s_rover->next;
                if (ElementNameIs(s_rover, "mo")) {
                  if (!nested_operand)
                    local_nodes_done++;
                  s_rover = s_rover->next;
                } else {
                  TCI_ASSERT(0);
                  break;
                }

                if (ElementNameIs(s_rover, "mrow")) {
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

            if (ElementNameIs(integrand_ender, "mo")) {
              const char *ptr = strstr(integrand_ender->p_chdata, "&#x");
              if (ptr) {
                U32 unicode = ASCII2U32(ptr + 3, 16);
                TCI_ASSERT(unicode == 0x2062);
              }
              integrand_ender = integrand_ender->prev;
              if (!nested_operand)
                local_nodes_done++;
            }

            integrand_starter = integrand_ender;
            while (integrand_starter && integrand_starter->prev) {
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
          BUCKET_REC *a_rec = MakeBucketRec(MB_OPERAND, NULL);
          AppendBucketRecord(bigop_snode->bucket_list, a_rec);
          SEMANTICS_NODE* node;

          if (integrand_ender){
            MNODE *save = integrand_ender->next;
            integrand_ender->next = NULL;            
            node = GetSemanticsList(integrand_starter, a_rec, false, pAnalyzer);
            integrand_ender->next = save;
         } else {
            // Have no integrand: \int dx
            node = CreateSemanticsNode(SEM_TYP_NUMBER);
            node -> contents = DuplicateString("1");
         }
         a_rec->first_child = node;
         node->parent = a_rec;

         if (frac_operand)
            Patchdx(node);

        } else {
          // Here, we don't have an integand ender - there's no "dx"
          // We still generate the operand - engine will do something without MB_INTEG_VAR
          MNODE *integrand_starter = mml_operand;

          BUCKET_REC *a_rec = MakeBucketRec(MB_OPERAND, NULL);
          AppendBucketRecord(bigop_snode->bucket_list, a_rec);
          SEMANTICS_NODE *contents = GetSemanticsList(integrand_starter, a_rec, false, pAnalyzer);
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

void AnalyzePrimed(MNODE * mml_msup,
                             SEMANTICS_NODE * s_node, int& nodes_done, Analyzer* pAnalyzer)
{
  nodes_done = 1;

  s_node->semantic_type = SEM_TYP_DERIVATIVE;

  s_node->contents = DuplicateString("differentiate");

  MNODE *base = mml_msup->first_kid;
  MNODE *primes = base->next;

  BUCKET_REC *base_bucket = MakeBucketRec(MB_UNNAMED, NULL);
  AppendBucketRecord(s_node->bucket_list, base_bucket);
  SEMANTICS_NODE *s_base = GetSemanticsFromNode(base, base_bucket, pAnalyzer);
  base_bucket->first_child = s_base;
  s_base->parent = base_bucket;



  if (s_base->semantic_type == SEM_TYP_FUNCTION
      || s_base->semantic_type == SEM_TYP_VARIABLE) {
    s_base->semantic_type = SEM_TYP_FUNCTION;
    // see if an argument bucket has been generated
    BUCKET_REC *bucket = FindBucketRec(s_base->bucket_list, MB_UNNAMED);
    // If arguments exist, process them
    int local_nodes_done;
    BUCKET_REC *br = ArgsToBucket(mml_msup, local_nodes_done, pAnalyzer);
    nodes_done += local_nodes_done;

    if (br) {
      if (bucket)
        RemoveBucket(s_base, bucket);
      AppendBucketRecord(s_base->bucket_list, br);
    } else if (pAnalyzer -> GetAnalyzerData() -> GetDE_ind_vars() && (!bucket || !bucket->first_child)) {
      if (bucket)
        RemoveBucket(s_base, bucket);
      BUCKET_REC *fvar_bucket = AddVarToBucket(MB_UNNAMED, pAnalyzer -> GetAnalyzerData() -> GetDE_ind_vars(), pAnalyzer -> GetAnalyzerData());
      AppendBucketRecord(s_base->bucket_list, fvar_bucket);
    } else {

      DefInfo* di = pAnalyzer->GetAnalyzerData() ->GetDI ( s_base->canonical_ID);
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

          SEMANTICS_NODE *s_var = CreateSemanticsNode(SEM_TYP_VARIABLE);
          //s_var->semantic_type = SEM_TYP_VARIABLE;

          if (canonical_ID) {
            s_var->canonical_ID = DuplicateString(canonical_ID);
            SetSnodeOwner(s_var, pAnalyzer -> GetAnalyzerData());
          }

          s_var->contents = DuplicateString(user_name);

          BUCKET_REC *dvar_bucket = AddVarToBucket(MB_DIFF_VAR, s_var, pAnalyzer -> GetAnalyzerData());
          
          AppendBucketRecord(s_node->bucket_list, dvar_bucket);

          if (bucket)
            RemoveBucket(s_base, bucket);
          BUCKET_REC *fvar_bucket = AddVarToBucket(MB_UNNAMED, s_var, pAnalyzer -> GetAnalyzerData());
          
          AppendBucketRecord(s_base->bucket_list, fvar_bucket);

          DisposeSList(s_var);
        } else
          TCI_ASSERT(0);
      }
    }
  } else {
    if (pAnalyzer -> GetAnalyzerData() -> GetDE_ind_vars()) {
      BUCKET_REC *dvar_bucket = AddVarToBucket(MB_DIFF_VAR, pAnalyzer -> GetAnalyzerData() -> GetDE_ind_vars(), pAnalyzer -> GetAnalyzerData());
      AppendBucketRecord(s_node->bucket_list, dvar_bucket);
    } else
      TCI_ASSERT(0);
  }

  AddPrimesCount(s_node, primes);
}




void AnalyzeDotDerivative(MNODE* mml_mover,
                          int n_dots,
                          SEMANTICS_NODE* s_node, 
                          int& nodes_done, 
                          Analyzer* pAnalyzer)
{
  nodes_done = 1;

  s_node->semantic_type = SEM_TYP_DERIVATIVE;
  s_node->contents = DuplicateString("differentiate");

  MNODE* base = mml_mover->first_kid;

  // BUCKET_REC* base_bucket = MakeBucketRec(MB_UNNAMED, NULL);
  // AppendBucketRecord(s_node->bucket_list, base_bucket);
  // SEMANTICS_NODE *s_base = GetSemanticsFromNode(base, base_bucket, pAnalyzer);
  // base_bucket->first_child = s_base;
  // s_base->parent = base_bucket;

  SEMANTICS_NODE* s_base = AppendNewBucketRecord(MB_UNNAMED, NULL, s_node, base, true, pAnalyzer);

  if (s_base->semantic_type == SEM_TYP_FUNCTION
      || s_base->semantic_type == SEM_TYP_VARIABLE) {

    s_base->semantic_type = SEM_TYP_FUNCTION;

    // see if an argument bucket has been generated
    BUCKET_REC* bucket = FindBucketRec(s_base->bucket_list, MB_UNNAMED);

    // If arguments exist, process them
    int local_nodes_done;
    BUCKET_REC* br = ArgsToBucket(mml_mover, local_nodes_done, pAnalyzer);
    nodes_done += local_nodes_done;

    if (br) {

        if (bucket)
          RemoveBucket(s_base, bucket);

        AppendBucketRecord(s_base->bucket_list, br);

    } else if (pAnalyzer -> GetAnalyzerData() -> GetDE_ind_vars() && (!bucket || !bucket->first_child)) {

        if (bucket)
          RemoveBucket(s_base, bucket);

        BUCKET_REC* fvar_bucket = AddVarToBucket(MB_UNNAMED, pAnalyzer -> GetAnalyzerData() -> GetDE_ind_vars(), pAnalyzer -> GetAnalyzerData());
        AppendBucketRecord(s_base->bucket_list, fvar_bucket);

    } else {

        DefInfo* di = pAnalyzer->GetAnalyzerData() ->GetDI ( s_base->canonical_ID);

        if (di && di->arg_list) {
          char user_name[128];
          char canonical_ID[128];
          char* ptr = strchr(di->arg_list, ',');
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

            SEMANTICS_NODE* s_var = CreateSemanticsNode(SEM_TYP_VARIABLE);
            //s_var->semantic_type = SEM_TYP_VARIABLE;

            if (canonical_ID) {
              s_var->canonical_ID = DuplicateString(canonical_ID);
              SetSnodeOwner(s_var, pAnalyzer -> GetAnalyzerData());
            }

            s_var->contents = DuplicateString(user_name);

            BUCKET_REC* dvar_bucket = AddVarToBucket(MB_DIFF_VAR, s_var, pAnalyzer -> GetAnalyzerData());
            AppendBucketRecord(s_node->bucket_list, dvar_bucket);

            if (bucket)
              RemoveBucket(s_base, bucket);

            BUCKET_REC* fvar_bucket = AddVarToBucket(MB_UNNAMED, s_var, pAnalyzer -> GetAnalyzerData());
            
            AppendBucketRecord(s_base->bucket_list, fvar_bucket);

            DisposeSList(s_var);
          } else
            TCI_ASSERT(0);
        }
    }
  } else {
    if (pAnalyzer -> GetAnalyzerData() -> GetDE_ind_vars()) {
        BUCKET_REC *dvar_bucket = AddVarToBucket(MB_DIFF_VAR, pAnalyzer -> GetAnalyzerData() -> GetDE_ind_vars(), pAnalyzer -> GetAnalyzerData());
        AppendBucketRecord(s_node->bucket_list, dvar_bucket);
    } else
       TCI_ASSERT(0);
  }

  AppendNumber(s_node, MB_NPRIMES, n_dots);
}

// Here the subscript is NOT treated as an argument

void AnalyzeSubscriptedFunc(MNODE * mml_msub_node,
                                      SEMANTICS_NODE * snode,
                                      int& nodes_done, Analyzer* pAnalyzer)
{
  nodes_done = 1;

  BUCKET_REC *bucket = FindBucketRec(snode->bucket_list, MB_UNNAMED);

  int local_nodes_done;
  BUCKET_REC *br = ArgsToBucket(mml_msub_node, local_nodes_done, pAnalyzer);
  nodes_done += local_nodes_done;

  if (br) {
    if (bucket)
      RemoveBucket(snode, bucket);
    AppendBucketRecord(snode->bucket_list, br);
  } else if (pAnalyzer -> GetAnalyzerData() -> GetDE_ind_vars() && (!bucket || !bucket->first_child)) {
    if (bucket)
      RemoveBucket(snode, bucket);
    BUCKET_REC *fvar_bucket = AddVarToBucket(MB_UNNAMED, pAnalyzer -> GetAnalyzerData() -> GetDE_ind_vars(), pAnalyzer -> GetAnalyzerData());
    AppendBucketRecord(snode->bucket_list, fvar_bucket);
  }

  MNODE* base = mml_msub_node->first_kid;

  snode->contents = DuplicateString(base->p_chdata);
  snode->semantic_type = SEM_TYP_FUNCTION;

  if (base->p_chdata && !strcmp(base->p_chdata, "log")) {
    if (base->next) {
      
      // BUCKET_REC *bucket = MakeBucketRec(MB_LOG_BASE, NULL);
      // AppendBucketRecord(snode->bucket_list, bucket);
      // SEMANTICS_NODE *log_base = GetSemanticsFromNode(base->next, bucket, pAnalyzer);
      // bucket->first_child = log_base;
      // log_base->parent = bucket;
	  AppendNewBucketRecord(MB_LOG_BASE, NULL, snode, base->next, true, pAnalyzer);

    } else
      TCI_ASSERT(0);
  }
}

void AnalyzeSubscriptedArgFunc(MNODE * mml_msub_node,
                                         SEMANTICS_NODE * snode, Analyzer* pAnalyzer)
{
  MNODE *f_nom = mml_msub_node->first_kid;
  MNODE *f_arg = f_nom->next;

  // Handle the function argument
  BUCKET_REC *arg_bucket = NULL;
  if (ElementNameIs(f_arg, "mrow")) {
    arg_bucket = ArgBucketFromMROW(f_arg, pAnalyzer);
  } else {
    arg_bucket = MakeBucketRec(MB_UNNAMED, NULL);
    SEMANTICS_NODE *s_arg = GetSemanticsFromNode(f_arg, arg_bucket, pAnalyzer);
    arg_bucket->first_child = s_arg;
  }

  if (arg_bucket) {
    BUCKET_REC *bucket = FindBucketRec(snode->bucket_list, MB_UNNAMED);
    if (bucket)
      RemoveBucket(snode, bucket);
    AppendBucketRecord(snode->bucket_list, arg_bucket);
  } else
    TCI_ASSERT(0);

  snode->contents = DuplicateString(f_nom->p_chdata);

  snode->semantic_type = SEM_TYP_FUNCTION;
}

void AnalyzeSubscriptedFence(MNODE* mml_msub_node,
                             SEMANTICS_NODE* snode,
                             int& nodes_done, 
                             Analyzer* pAnalyzer)
{
  nodes_done = 1;

  MNODE* base = mml_msub_node->first_kid;
  if (base && ElementNameIs(base, "mfenced")) {

    const char* open_value = GetATTRIBvalue(base->attrib_list, "open");
    const char* close_value = GetATTRIBvalue(base->attrib_list, "close");

    bool is_Vert = StringEqual(open_value, "&#x2016;") && StringEqual(close_value, "&#x2016;");
    bool is_abs = StringEqual(open_value, "|") && StringEqual(close_value, "|");
    
    if (is_Vert || is_abs) {
      // Handle ||...||_{norm} here
      // (note we don't have a different representation for |...|_{norm}
      int local_nodes_done;
      
      AnalyzeMFENCED(base, snode, local_nodes_done, pAnalyzer);

      if (snode->bucket_list) {
        BUCKET_REC* bucket = FindBucketRec(snode->bucket_list, MB_UNNAMED);
        if (bucket && bucket->first_child)
          FenceToMatrix(bucket->first_child);
      }
      if (base->next) {
        
        // BUCKET_REC* bucket = MakeBucketRec(MB_NORM_NUMBER, NULL);
        // AppendBucketRecord(snode->bucket_list, bucket);
        // SEMANTICS_NODE* norm_num = GetSemanticsFromNode(base->next, bucket, pAnalyzer);
        // bucket->first_child = norm_num;
        // norm_num->parent = bucket;
		AppendNewBucketRecord(MB_NORM_NUMBER, NULL, snode, base->next, true, pAnalyzer);

      } else {
        TCI_ASSERT(!"The subscript is missing? - bad MathML");
      }
    } else {
      TCI_ASSERT(!"Unknown fence type.");
    }
  }
}

////////////////////////////// END SCRIPT HANDLING



// Before we process a DE, it may be necessary to traverse the input.
// We look for things like "D_{x^2y}" and "d^{4}(?)/d^{2}x*d^{2}y"
// These constructs give us the independent variable(s).
// This function generates a list of SEMANTICS_NODEs.

SEMANTICS_NODE* DetermineIndepVar(MNODE * dMML_list, Analyzer* pAnalyzer)
{
  SEMANTICS_NODE *head = NULL;
  SEMANTICS_NODE *tail;

  MNODE *rover = dMML_list;
  while (rover) {
    SEMANTICS_NODE *s_indvar = NULL;
    if (ElementNameIs(rover, "mfrac")) {
      s_indvar = GetIndVarFromFrac(rover, pAnalyzer);
    } else if (ElementNameIs(rover, "msub")) {
      s_indvar = GetIndepVarFromSub(rover, pAnalyzer);
    }

    if (!s_indvar && rover->first_kid)
      s_indvar = DetermineIndepVar(rover->first_kid, pAnalyzer);

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

SEMANTICS_NODE* GetIndVarFromFrac(MNODE * mfrac, Analyzer* pAnalyzer)
{
  SEMANTICS_NODE *head = NULL;
  SEMANTICS_NODE *tail;

  MNODE *m_num_operand;
  MNODE *m_den;
  if (IsDIFFOP(mfrac, &m_num_operand, &m_den)) {
    if (m_den) {
      if (ElementNameIs(m_den, "mrow")) {
        MNODE* rover = m_den->first_kid;

        if (ElementNameIs(rover, "mrow")) {
          // denominator is a list of differentials
          while (rover) {
            if (ElementNameIs(rover, "mrow")) {
              MNODE* diff_op = rover->first_kid;
              MNODE* m_den_var_expr = diff_op->next;
              SEMANTICS_NODE* snode = ExtractIndepVar(m_den_var_expr, pAnalyzer);

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
          SEMANTICS_NODE *snode = ExtractIndepVar(m_den_var_expr, pAnalyzer);
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

SEMANTICS_NODE* GetIndepVarFromSub(MNODE * msub, Analyzer* pAnalyzer)
{
  SEMANTICS_NODE *head = NULL;
  SEMANTICS_NODE *tail;

  if (IsDDIFFOP(msub)) {
    MNODE *base = msub->first_kid;
    MNODE *sub = base->next;

    // subscript is a (list of) differentials
    MNODE *rover = sub;
    if (ElementNameIs(rover, "mrow"))
      rover = rover->first_kid;

    while (rover) {
      if (ElementNameIs(rover, "mi") || ElementNameIs(rover, "msup")) {
        SEMANTICS_NODE* snode = ExtractIndepVar(rover, pAnalyzer);
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


// \frac{df}{dx}


// Translation of a subscript has been placed in a separate function
//  just in case we want to do something special with it.
// Nothing fancy here yet.

SEMANTICS_NODE* QualifierToSNODE(MNODE* sub, Analyzer* pAnalyzer)
{
  return GetSemanticsFromNode(sub, NULL, pAnalyzer);
}






SEMANTICS_NODE* ExtractIndepVar(MNODE * rover, Analyzer* pAnalyzer)
{
  SEMANTICS_NODE *s_local = NULL;

  if (ElementNameIs(rover, "mi")) {

      s_local = CreateSemanticsNode();
      int nodes_done;
      AnalyzeMI(rover, s_local, nodes_done, false, pAnalyzer);

  } else if (ElementNameIs(rover, "msup")) {

      MNODE* base = rover->first_kid;
      s_local = CreateSemanticsNode(SEM_TYP_POWERFORM);
	  //s_local->semantic_type = SEM_TYP_POWERFORM;
      
      // BUCKET_REC* base_bucket = MakeBucketRec(MB_SCRIPT_BASE, NULL);
      // AppendBucketRecord(s_local->bucket_list, base_bucket);
      // SEMANTICS_NODE* s_base = GetSemanticsFromNode(base, base_bucket, pAnalyzer);
      // base_bucket->first_child = s_base;
      // s_base->parent = base_bucket;
	  AppendNewBucketRecord(MB_SCRIPT_BASE, NULL, s_local, base, true, pAnalyzer);

      // BUCKET_REC* power_bucket = MakeBucketRec(MB_SCRIPT_UPPER, NULL);
      // AppendBucketRecord(s_local->bucket_list, power_bucket);
      // SEMANTICS_NODE* s_power = GetSemanticsFromNode(base->next, power_bucket, pAnalyzer);
      // power_bucket->first_child = s_power;
      // s_power->parent = power_bucket;
      AppendNewBucketRecord(MB_SCRIPT_UPPER, NULL, s_local, base->next, true, pAnalyzer);

  } else if (ElementNameIs(rover, "mo") && StringEqual(rover->p_chdata, "&#x2062;")) {

  } else

      TCI_ASSERT(0);

  return s_local;
}




void CreateSubscriptedVar(MNODE* mml_msub_node,
                          bool remove_super,
                          SEMANTICS_NODE* snode, 
                          Analyzer* pAnalyzer)
{
  char *mml_canonical_name = NULL;
  if (remove_super) {
    char buffer[1024];
    buffer[0] = 0;
    strcat(buffer, "msub");

    MNODE *rover = mml_msub_node->first_kid;
    if (rover) {
      char *tmp = GetCanonicalIDforMathNode(rover, pAnalyzer-> GetAnalyzerData() -> GetGrammar());
      if (tmp) {
        strcat(buffer, tmp);
        delete[] tmp;
        tmp = GetCanonicalIDforMathNode(rover->next, pAnalyzer-> GetAnalyzerData() -> GetGrammar());
        if (tmp) {
          strcat(buffer, tmp);
          delete[] tmp;
        }
      }
    }
	mml_canonical_name = DuplicateString(buffer);
  } else
    mml_canonical_name = GetCanonicalIDforMathNode(mml_msub_node, pAnalyzer-> GetAnalyzerData() -> GetGrammar());
  if (!mml_canonical_name) {
    snode->error_flag = 1;
    return;
  }

  snode->canonical_ID = mml_canonical_name;
  SetSnodeOwner(snode, pAnalyzer -> GetAnalyzerData());
  // pAnalyzer -> SetNodeIDsList( AppendIDRec(pAnalyzer -> NodeIDsList(), pAnalyzer ->GetAnalyzerData()-> CurrClientID(),
  //                             mml_canonical_name, mml_msub_node, pAnalyzer -> ScrStr()) );
  pAnalyzer -> AppendIDList(mml_canonical_name, mml_msub_node);
  snode->semantic_type = SEM_TYP_QUALIFIED_VAR;

  // Extra info - first the base variable
  MNODE* base = mml_msub_node->first_kid;
  MNODE* sub = base->next;

  SEMANTICS_NODE *s_var = CreateSemanticsNode();
  BUCKET_REC* bucket = MakeParentBucketRec(MB_BASE_VARIABLE, s_var);
  AppendBucketRecord(snode->bucket_list, bucket);
  

  char *base_canonical_name = GetCanonicalIDforMathNode(base, pAnalyzer-> GetAnalyzerData() -> GetGrammar());
  if (!base_canonical_name) {
    snode->error_flag = 1;
    return;
  }

  s_var->canonical_ID = base_canonical_name;
  SetSnodeOwner(s_var, pAnalyzer -> GetAnalyzerData());
  // pAnalyzer -> SetNodeIDsList( AppendIDRec(pAnalyzer -> NodeIDsList(), pAnalyzer ->GetAnalyzerData()-> CurrClientID(),
  //                             base_canonical_name, base, pAnalyzer -> ScrStr()) );
  //
  pAnalyzer -> AppendIDList(base_canonical_name, base); 
  s_var->semantic_type = SEM_TYP_VARIABLE;
  s_var->contents = DuplicateString(base->p_chdata);

  // Extra info - the subscripted qualifier
  SEMANTICS_NODE *s_cont = QualifierToSNODE(sub, pAnalyzer);
  BUCKET_REC *sub_bucket = MakeParentBucketRec(MB_SUB_QUALIFIER, s_cont);
  AppendBucketRecord(snode->bucket_list, sub_bucket);
  
}

void CreatePowerForm(MNODE* mml_base, MNODE* mml_power, SEMANTICS_NODE* snode, Analyzer* pAnalyzer)
{
  // BUCKET_REC* base_bucket = MakeBucketRec(MB_SCRIPT_BASE, NULL);
  // AppendBucketRecord(snode->bucket_list, base_bucket);
  // SEMANTICS_NODE* s_base = GetSemanticsFromNode(mml_base, base_bucket, pAnalyzer);
  // base_bucket->first_child = s_base;
  // s_base->parent = base_bucket;



  // BUCKET_REC* power_bucket = MakeBucketRec(MB_SCRIPT_UPPER, NULL);
  // AppendBucketRecord(snode->bucket_list, power_bucket);
  // SEMANTICS_NODE *s_power = GetSemanticsFromNode(mml_power, power_bucket, pAnalyzer);
  // power_bucket->first_child = s_power;
  // s_power->parent = power_bucket;

  snode->semantic_type = SEM_TYP_POWERFORM;
  mml_base -> next = NULL;
  AppendNewBucketRecord(MB_SCRIPT_BASE, NULL, snode, mml_base, true, pAnalyzer);
  AppendNewBucketRecord(MB_SCRIPT_UPPER, NULL, snode, mml_power, true, pAnalyzer);

}




void AnalyzeBesselFunc(MNODE * mml_msub_node,
                                 SEMANTICS_NODE * snode, int& nodes_done, Analyzer* pAnalyzer)
{
  nodes_done = 1;
  char *mml_canonical_name = GetCanonicalIDforMathNode(mml_msub_node, pAnalyzer-> GetAnalyzerData() -> GetGrammar());

  if (!mml_canonical_name) {
    TCI_ASSERT(0);
    snode->error_flag = 1;
    return;
  }

  snode->canonical_ID = mml_canonical_name;
  SetSnodeOwner(snode, pAnalyzer -> GetAnalyzerData());
  // pAnalyzer -> SetNodeIDsList( AppendIDRec(pAnalyzer -> NodeIDsList(), pAnalyzer ->GetAnalyzerData()-> CurrClientID(),
  //                             mml_canonical_name, mml_msub_node, pAnalyzer -> ScrStr()) );
  pAnalyzer -> AppendIDList(mml_canonical_name, mml_msub_node);
  if (mml_msub_node && mml_msub_node->first_kid) {
    MNODE* base = mml_msub_node->first_kid;
    snode->contents = DuplicateString(base->p_chdata);
    snode->semantic_type = SEM_TYP_FUNCTION;

    MNODE *sub = base->next;
    if (sub) {
	  AppendNewBucketRecord(MB_UNNAMED, NULL, snode, sub, true, pAnalyzer);
      // BUCKET_REC *sub_bucket = MakeBucketRec(MB_UNNAMED, NULL);
      // AppendBucketRecord(snode->bucket_list, sub_bucket);
      // SEMANTICS_NODE *s_sub = GetSemanticsFromNode(sub, sub_bucket, pAnalyzer);
      // sub_bucket->first_child = s_sub;
      // s_sub->parent = sub_bucket;
    }

    int local_nodes_done;
    BUCKET_REC *bucket = ArgsToBucket(mml_msub_node, local_nodes_done, pAnalyzer);
    nodes_done += local_nodes_done;

    AppendBucketRecord(snode->bucket_list, bucket);
  }
}


bool SetODEvars(MathServiceRequest& msr, 
                MathResult& mr,
                MNODE* dMML_tree, 
                U32 UI_cmd_ID, 
                Analyzer* pAnalyzer)
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
      pAnalyzer -> GetAnalyzerData() -> SetDE_ind_vars( CreateSemanticsNode(SEM_TYP_VARIABLE) );
      pAnalyzer -> GetAnalyzerData() -> GetDE_ind_vars()->contents = DuplicateString(i_var);
    } else if (p_type == zPT_ASCII_mmlmarkup) {
      // If "i_var" comes from a chambase dialog (as it should)
      //   it will contain something like "<mi>x</mi>"
      pAnalyzer -> GetAnalyzerData() -> SetDE_ind_vars( CreateSTreeFromMML(i_var, pAnalyzer) );
    } else {
      TCI_ASSERT(0);
    }
    mr.PutResultCode(CR_undefined);
  } else {
      pAnalyzer -> GetAnalyzerData() -> SetDE_ind_vars( DetermineIndepVar(dMML_tree, pAnalyzer) );

      if (!pAnalyzer -> GetAnalyzerData() -> GetDE_ind_vars()) {
        mr.PutResultCode(CR_queryindepvar);
           return false;
        }

        // if (UI_cmd_ID == CCID_Solve_ODE_Numeric) {
        //   char buffer[80];
        //   ChooseIndVar(dMML_tree, buffer);
        // 
        //   pAnalyzer -> GetAnalyzerData() -> SetDE_ind_vars( CreateSemanticsNode(SEM_TYP_VARIABLE) );
        //   pAnalyzer -> GetAnalyzerData() -> GetDE_ind_vars()->contents = DuplicateString(buffer);
        // 
        //   mr.PutResultCode(CR_undefined);
        // } else {
        //   mr.PutResultCode(CR_queryindepvar);
        //   return false;
        // }
       //}
  }

  // Identify the function we're solving for in the ODE.
  DetermineODEFuncNames(dMML_tree, pAnalyzer -> GetAnalyzerData());


  FixInvisibleTimesAfterFunction(dMML_tree, pAnalyzer -> GetAnalyzerData());

  return true;
}



// A function to replace InvisibleTimes after known function  (Used for diff eqs)

void FixInvisibleTimesAfterFunction(MNODE* dMML_tree, AnalyzerData* pData)
{
   MNODE* rover = dMML_tree;

   // Look for <mi>'s
   // Use LocateFuncRec to see if they are function names
   // See if following operator is InvisibleTimes and replace with ApplyFunc

   while (rover) {
      
      if ( ElementNameIs(rover, "mi" ) ) {  // an <mi>
         
         char* mml_canonical_name = GetCanonicalIDforMathNode(rover, pData -> GetGrammar());

		 if (LocateFuncRec(pData -> DE_FuncNames(), mml_canonical_name, NULL)) {
             
             // Look ahead in the MML list for an operator.
             OpIlk op_ilk;
             int advance;
             rover = LocateOperator(rover, op_ilk, advance);
	        
			 if ( ContentIs(rover,"&#x2062;") ){
                delete rover->p_chdata;
                rover->p_chdata = DuplicateString("&#x2061;"); // ApplyFunction
			 }
			 
		 }
	  } else if (rover->first_kid) {
	     // recursive descent
		 FixInvisibleTimesAfterFunction(rover->first_kid, pData);
	  }

      if (rover) 
	     rover = rover -> next;
   }

}


// A function to replace InvisibleTimes after known function  (Used for diff eqs)

void FixInvisibleTimesAfterFunction2(MNODE* dMML_tree, AnalyzerData* pData)
{
   MNODE* rover = dMML_tree;

   // Look for <mi>'s
   // Use LocateFuncRec to see if they are function names
   // See if following operator is InvisibleTimes and replace with ApplyFunc

   while (rover) {
      
      if ( ElementNameIs(rover, "mi" ) ) {  // an <mi>
         
         char* mml_canonical_name = GetCanonicalIDforMathNode(rover, pData -> GetGrammar());

		     if ( (LocateFuncRec(pData -> DE_FuncNames(), mml_canonical_name, NULL)) || 
		          (pData -> IsDefinedFunction(rover) ) ) {
             
             // Look ahead in the MML list for an operator.
             OpIlk op_ilk;
             int advance;
             rover = LocateOperator(rover, op_ilk, advance);
	        
			 if ( ContentIs(rover,"&#x2062;") ){
             delete rover->p_chdata;
             rover->p_chdata = DuplicateString("&#x2061;"); // ApplyFunction
			 }
			 
		 }
	  } else if (rover->first_kid) {
	     // recursive descent
		 FixInvisibleTimesAfterFunction(rover->first_kid, pData);
	  }

      if (rover) 
	     rover = rover -> next;
   }

}






bool SetIMPLICITvars(MathServiceRequest & msr, MathResult & mr, Analyzer* pAnalyzer)
{
  bool rv = true;

  // Parameterized command - "variable of differentiation"
  U32 p_type;
  const char *i_var = msr.GetParam(PID_ImplDiffIndepVar, p_type);
  if (i_var) {
    if (p_type == zPT_ASCII_text) {
      // i_var entered as "x"
      TCI_ASSERT(0);
      TCI_ASSERT(!pAnalyzer -> GetIMPLDIFF_ind_var());  // global var - must be NULL here!
      pAnalyzer -> SetIMPLDIFF_ind_var( CreateSemanticsNode(SEM_TYP_VARIABLE) );
      //pAnalyzer -> GetIMPLDIFF_ind_var()->semantic_type = SEM_TYP_VARIABLE;
      pAnalyzer -> GetIMPLDIFF_ind_var()->contents = DuplicateString(i_var);
    } else if (p_type == zPT_ASCII_mmlmarkup) {
      // If "i_var" comes from a chambase dialog (as it should)
      //   it will contain something like "<mi>x</mi>"
      pAnalyzer -> SetIMPLDIFF_ind_var( CreateSTreeFromMML(i_var, pAnalyzer) );
    } else
      TCI_ASSERT(0);

    // Parameterized command - "list of dependent variables"
    // For now, I'm only handling a comma separated list of letters.
    const char *d_vars = msr.GetParam(PID_ImplDiffDepVars, p_type);
    if (p_type == zPT_ASCII_text) {
      TCI_ASSERT(0);
    } else if (p_type == zPT_ASCII_mmlmarkup) {
      SEMANTICS_NODE *s_tree = CreateSTreeFromMML(d_vars, pAnalyzer);
      if (s_tree) {
        ExtractVariables(s_tree, pAnalyzer -> GetAnalyzerData());
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

SEMANTICS_NODE* CreateSTreeFromMML(const char *mml_str, Analyzer* pAnalyzer)
{
  SEMANTICS_NODE *rv = NULL;

  if (mml_str) {
    MNODE *var = pAnalyzer -> GetAnalyzerData() -> GetMMLTreeGen() -> MMLstr2Tree(mml_str);
    if (var) {
      int nodes_done = 0;
      rv = SNodeFromMNodes(var, nodes_done, false, pAnalyzer);
      DisposeTNode(var);
    } else
      TCI_ASSERT(0);
  }

  return rv;
}

void MSUB2FuncCall(MNODE * mml_msub_node, SEMANTICS_NODE * snode, Analyzer* pAnalyzer)
{
  MNODE *base = mml_msub_node->first_kid;

  if (ElementNameIs(base, "mi")) {
    int nodes_done;
    AnalyzeMI(base, snode, nodes_done, false, pAnalyzer);
    if (snode->semantic_type == SEM_TYP_VARIABLE) {
      snode->semantic_type = SEM_TYP_FUNCTION;
      // process the argument
      int local_nodes_done;
      BUCKET_REC *br = ArgsToBucket (base, local_nodes_done, pAnalyzer);
      if (br)
        AppendBucketRecord(snode->bucket_list, br);
    }
  } else {
    TCI_ASSERT(!"Expected mi in base.");
  }
}



void CreateSubstBucket(MNODE* subst, SEMANTICS_NODE* snode, bool is_lower, Analyzer* pAnalyzer)
{
  U32 b_ID = is_lower ? MB_SUBST_LOWER : MB_SUBST_UPPER;

  if (subst) {
    bool is_list = false;
    MNODE* m_rover = subst;
    if (ElementNameIs(m_rover, "mrow")) {
      
        MNODE* child1 = m_rover->first_kid;
        if (child1 && child1->next && ContentIs(child1->next, ",")) {
          m_rover = child1;
          is_list = true;
        }

    }

    if (is_list) {

        while (m_rover) {
          BUCKET_REC* subst_bucket = MakeBucketRec(b_ID, NULL);
          AppendBucketRecord(snode->bucket_list, subst_bucket);
          SEMANTICS_NODE* sl_subst = GetSemanticsFromNode(m_rover, subst_bucket, pAnalyzer);
          subst_bucket->first_child = sl_subst;
          sl_subst->parent = subst_bucket;

          if (m_rover->next) {
            m_rover = m_rover->next;
            if (ElementNameIs(m_rover, "mo") && ContentIs(m_rover, ",")) {
              m_rover = m_rover->next;
            } else
              break;
          } else
            break;
        }

    } else {

        BUCKET_REC *subst_bucket = MakeBucketRec(b_ID, NULL);
        AppendBucketRecord(snode->bucket_list, subst_bucket);
        SEMANTICS_NODE *sl_subst = GetSemanticsFromNode(m_rover, subst_bucket, pAnalyzer);
        subst_bucket->first_child = sl_subst;
        sl_subst->parent = subst_bucket;

    }
  }
}




// Called only in BuildSemanticsTree

SEMANTICS_NODE* DefToSemanticsList(MNODE* dMML_tree, int& error_code, Analyzer* pAnalyzer)
{
  SEMANTICS_NODE* rv = NULL;
  error_code = 0;

  OverridePrefsOnLHS(dMML_tree, pAnalyzer -> GetAnalyzerData());
  OverrideInvisibleTimesOnLHS(dMML_tree);

  if (ElementNameIs(dMML_tree, "math")) {
    rv = CreateSemanticsNode(SEM_TYP_MATH_CONTAINER);
    //rv->semantic_type = SEM_TYP_MATH_CONTAINER;

    if (dMML_tree->first_kid) {
      MNODE *cont = dMML_tree->first_kid;
      // descend into a redundant mrow, if it exists
      while (cont && !cont->next && ElementNameIs(cont, "mrow"))
        cont = cont->first_kid;

      if (cont) {
        BUCKET_REC* new_a_rec = MakeBucketRec(MB_UNNAMED, NULL);
        rv->bucket_list = AppendBucketRec(NULL, new_a_rec);
        SEMANTICS_NODE* s_node = GetDefSList(cont, new_a_rec, ALL_NODES, error_code, pAnalyzer);

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

SEMANTICS_NODE* GetDefSList(MNODE* dMML_list,
                            BUCKET_REC* parent_bucket,
                            int mml_node_lim, 
                            int& error_code, 
                            Analyzer* pAnalyzer)
{
  SEMANTICS_NODE* head = NULL;
  SEMANTICS_NODE* tail;

  int mml_nodes_done = 0;
  MNODE* rover = dMML_list;

  while (rover && mml_nodes_done < mml_node_lim && !error_code) {
    SEMANTICS_NODE* new_node = NULL;
    // Look ahead in the MML list for an operator.
    OpIlk op_ilk;
    int advance;
    MNODE* mo = LocateOperator(rover, op_ilk, advance);

    bool opIsAF = false;
    if (ContentIs(mo,"&#x2061;"))
      opIsAF = true;

    int mml_nodes_remaining = mml_node_lim - mml_nodes_done;
    if (mo && mml_nodes_remaining > 1  && !opIsAF) {
      if ( (ContentIs(mo, "=") || ContentIs(mo, ":="))) {
        
        SEMANTICS_NODE* l_operand = NULL;
        SEMANTICS_NODE* r_operand = NULL;

        if (op_ilk == OP_infix || op_ilk == OP_postfix) {
          if (head) {
            l_operand = head;
            // Here we're assuming ALL operators in source MML list have
            //  the same precedence (ie. well-formed MML), and are left-associative.
          } else {
            // translate the left operand
            int l_nodes_done = 0;
            l_operand = SNodeFromMNodes(rover, l_nodes_done, true, pAnalyzer);
            while (l_nodes_done) {
              mml_nodes_done++;
              rover = rover->next;
              l_nodes_done--;
            }
          }
        }
        // translate the operator
        int op_nodes_done = 0;
        new_node = SNodeFromMNodes(rover, op_nodes_done, false, pAnalyzer);
        while (op_nodes_done) {
          mml_nodes_done++;
          rover = rover->next;
          op_nodes_done--;
        }
        if (op_ilk == OP_infix) {
          // translate the right operand
          int r_nodes_done = 0;
          r_operand = SNodeFromMNodes(rover, r_nodes_done, false, pAnalyzer);
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
      new_node = SNodeFromMNodes(rover, nodes_done, true, pAnalyzer);
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


bool NodeIsFunction(MNODE* mml_node, const Grammar* mml_entities, AnalyzerData* my_analyzer_data)
{
  // might be f^-1 or something, in which case the whole expr is the function
  if (HasScriptChildren(mml_node))
    return (my_analyzer_data->IsDefinedFunction(mml_node) || NodeIsFunction(mml_node->first_kid, mml_entities, my_analyzer_data));
  else if (ElementNameIs(mml_node, "mi")) {
    if (IsTrigArgFuncName(mml_entities, mml_node->p_chdata) ||
        IsReservedFuncName(mml_entities, mml_node->p_chdata) ||
        my_analyzer_data->IsDefinedFunction(mml_node))
      return true;
  } else if (ElementNameIs(mml_node, "mo") && mml_node->attrib_list) {
    const char* isMathname = GetATTRIBvalue(mml_node ->attrib_list, "msimathname");
    if ( isMathname  && strcmp(isMathname, "true") == 0 ){
      if (IsTrigArgFuncName(mml_entities, mml_node->p_chdata) ||
          IsReservedFuncName(mml_entities, mml_node->p_chdata) ||
          my_analyzer_data->IsDefinedFunction(mml_node))
        return true;
    }
    return false;
  }


  return false;
}

