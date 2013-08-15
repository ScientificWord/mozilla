// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

/* Implementation Notes - JBM

  In the process of communicating presentation MathML to an engine,
  we must come up with a "meaning" for the mml elements that constitute
  the math input for computation.  For example we have to decide
  if a given <mi>, <munder>, <msup>, etc. represents a variable, 
  a function, a power form, etc.
  (The necessity of making these decisions design independent.)

  In this design, those decisions about the semantics of the source
  markup are made in the CompEngine object. ( Actually, a sub-object
  named "Analyzer" does most of this task exclusively. )

  In the initial implementation, only a few basic and fairly unambiguous
  cases will be coded.  The aim is to do enough so that the entire
  project can be coded fairly quickly and tested with a few simple
  examples.

  Later, a lot of work will be needed here to handle dozens of special
  cases.  Final code will require that a DefStore be in place, perhaps
  access to some user prefs and some history, and a mechanism to query
  the user.  

  The ability to assign appropriate meanings to a great variety of markup
  without bugging the user too often will be crucial to the success of
  of this project.
*/

#include "CompEngine.h"

#include "MRequest.h"
#include "MResult.h"
#include "DefStore.h"
#include "PrefStor.h"
#include "Grammar.h"
#include "MML2Tree.h"
#include "Analyzer.h"
#include "STree2MML.h"
#include "dumputils.h"
#include "DefInfo.h"

#include "xpcom-config.h"
#include "nsServiceManagerUtils.h"
#include "nsComponentManagerUtils.h"
#include "nsString.h"
#include "attriblist.h"
#include "strutils.h"


void FixIndexedVarsOfQualifiedVar(SEMANTICS_NODE* qv, BUCKET_REC * arg_bucket_list);
bool IdIsFuncArg(char *sub_canonical_ID, BUCKET_REC * arg_bucket_list);
bool IsCommaList(SEMANTICS_NODE * s_node);
bool ExprContainsFuncArg(SEMANTICS_NODE* expr, BUCKET_REC* args);



void AppendSubPath(nsILocalFile* file, const char* asciiPath )
{
  if (!asciiPath) return;
  size_t zln = strlen(asciiPath);
  char *tmp = new char[zln + 1];
  strcpy(tmp, asciiPath);
  char * pch = tmp;
  char * pchStart = tmp;
  nsAutoString pathStr;
  bool done = false;
  while (!done){
    while (*pch && *pch != '\\' && *pch != '/') pch++;
    done = (*pch == 0);
    (*pch) = 0;
    pathStr = NS_ConvertUTF8toUTF16(pchStart);
    file->Append(pathStr);
    if (!done) *pch++; // skip past the null we put in
    while (*pch && *pch == '\\' && *pch == '/') pch++;
    pchStart = pch;
  }
}
      
CompEngine::CompEngine(Grammar* ID_dBase, 
                       Grammar* NOM_dBase,
                       Grammar* mathml_grammar, 
					   PrefsStore * up_store)
{
  id_dBase = ID_dBase;
  nom_dBase = NOM_dBase;
  uprefs_store = up_store;

  semantic_analyzer = new Analyzer(mathml_grammar, up_store);
  mml_tree_gen = new MML2Tree();
  dMML_tree = NULL;
  StreeToMML = new STree2MML(mathml_grammar,up_store);

  curr_IDs_2mml = NULL;
  def_IDs_2mml = NULL;

  engine_attrs = NULL;
  mml_VecBasisVars = NULL;
}

CompEngine::~CompEngine()
{
  delete[] mml_VecBasisVars;

  DisposeTList(dMML_tree);
  delete mml_tree_gen;
  delete semantic_analyzer;
  DisposeIDsList(def_IDs_2mml);
  DisposeIDsList(curr_IDs_2mml);
  delete StreeToMML;

  if (wrapper)
    wrapper->Cleanup();

  ENG_ATTR_REC *a_rover = engine_attrs;
  while (a_rover) {
    ENG_ATTR_REC *del = a_rover;
    a_rover = a_rover->next;
    delete[] del->value;
    delete del;
  }
}

void CompEngine::StopProcessor()
{
   if (wrapper)
     wrapper->StopProcessor();
}

bool CompEngine::InitUnderlyingEngine(Grammar* install_dbase, nsILocalFile* baseDir, 
                                          MathResult & mr)
{

  bool rv = false;

  char path[500];
  path[0] = 0;
  nsresult res;
  nsCOMPtr<nsIFile> bd;
  nsCOMPtr<nsILocalFile> libFile;
  nsCOMPtr<nsILocalFile> modFile;
  nsCOMPtr<nsILocalFile> vcamFile;
  nsCOMPtr<nsILocalFile> engFile;
  
  const char* eng_name;
  const char* eRecord;
  if (install_dbase->
      GetRecordFromIDs("ENGINFO", ENG_wrapperDLL, 0, &eng_name, &eRecord))
    strcpy(path, eRecord);
    // the path now is actually "@mackichan.com/mupadenginewrapper;1"
    // so we don't need to make it absolute
  else
    TCI_ASSERT(!"Failed to lookup ENGINFO/wrapperDLL");

  printf("\n\n  jcs Loading enigne wrapper %s\n", path);

  if (LoadEngWrapperDLL(path)) {

    const char *dest_zname;
    const char *eRecord;

    if (install_dbase->
        GetRecordFromIDs("ENGINFO", ENG_engpath, 0, &dest_zname, &eRecord)) {
      res = baseDir->Clone(getter_AddRefs(bd));
      engFile = do_QueryInterface(bd);
      AppendSubPath(engFile,eRecord);
    } else {
      TCI_ASSERT(!"Failed to lookup ENGINFO/engpath");
    }

    if (install_dbase->
        GetRecordFromIDs("ENGINFO", ENG_libp, 0, &dest_zname, &eRecord)) {

      res = baseDir->Clone(getter_AddRefs(bd));
      libFile = do_QueryInterface(bd);
      AppendSubPath(libFile,eRecord);
      res = baseDir->Clone(getter_AddRefs(bd));
      modFile = do_QueryInterface(bd);
#ifdef XP_WIN
      AppendSubPath(modFile,"MuPAD/win32/modules");
#endif
#ifdef XP_MACOSX
      AppendSubPath(modFile,"MuPAD/maci64/modules");
#else 
#ifdef XP_UNIX
      AppendSubPath(modFile,"MuPAD/linux/modules");
#endif
#endif
    } else {
      TCI_ASSERT(!"Failed to lookup ENGINFO/libp");
      //libpath = "C:\\swp50\\Maple";
    }
   
    int inner_rv;
    res = wrapper->LoadStrsAndDLL(engFile, libFile, modFile, (void *)id_dBase, (void *)nom_dBase, &inner_rv);
   
    if (NS_SUCCEEDED(res) && inner_rv) {
      if (install_dbase->
          GetRecordFromIDs("ENGINFO", ENG_vcampath, 0, &dest_zname,
                           &eRecord)) {
        res = baseDir->Clone(getter_AddRefs(bd));
        vcamFile = do_QueryInterface(bd);
        AppendSubPath(vcamFile,eRecord);
      } else {
        TCI_ASSERT(!"Failed to lookup ENGINFO/vcampath");
        //vcampath =
        //  "C:\\Program Files\\SciFace\\MuPAD Pro 3.1\\bin\\VCamNG.exe";
      }
      printf("\n\n  jcs Calling wrapper->Initialize\n");
      res = wrapper->Initialize(libFile, baseDir, vcamFile, &inner_rv);
      if (NS_SUCCEEDED(res) && inner_rv) {
        rv = true;
        ResetState(true);
      } else {
        rv = false;
      }
    }
    RetrieveEngineStrs(mr);

    return rv;
  }

  return rv;
}

void CompEngine::ResetState(bool use_grammar_values)
{
  if (use_grammar_values) {
    U32 ID = 0;
    const char *dest_zname = NULL;
    const char *dest_ztemplate = NULL;

    // We iterate a section of the "install script" here.
    int OK = 1;
    while (OK) {
      ID++;
      if (id_dBase->
          GetRecordFromIDs("RESET", ID, 0, &dest_zname, &dest_ztemplate)) {
        if (!strcmp(dest_zname, "END_OF_LIST")) {
          break;
        } else {
          nsresult res = wrapper->SetEngineState(ID, dest_zname, &OK);
          if (NS_SUCCEEDED(res) && OK)
            RecordEngineAttr(ID, dest_zname);
        }
      }
    }
  } else {
    TCI_ASSERT(0);
  }
}

bool CompEngine::SetEngineAttr(int attr_ID, int i_val, const char *s_val)
{
  int rv = 0;

  char buffer[32];
  const char *new_value;
  if (s_val) {
    new_value = s_val;
  } else {
    StrFromInt(i_val, buffer);
    new_value = buffer;
  }

  nsresult res = wrapper->SetEngineState(attr_ID, new_value, &rv);
  if (NS_SUCCEEDED(res) && rv)
    RecordEngineAttr(attr_ID, new_value);

  return rv != 0;
}

void CompEngine::ReleaseClient(U32 client_ID, U32 engine_ID, DefStore * ds)
{
  char *def_list = ds->GetDefList(engine_ID);
  wrapper->ReleaseClient(client_ID, def_list);
  delete[] def_list;
  ds->ClearDefs(engine_ID);
}

void CompEngine::Execute(MathServiceRequest& msr, MathResult& mr)
{
  if (!wrapper) {
    mr.PutResultCode(CR_failure);
    TCI_ASSERT(!"No engine loaded.");
    return;
  }

  delete[] mml_VecBasisVars;
  mml_VecBasisVars = NULL;

  DefStore* ds = msr.GetDefStore();
  const char* defaultVecBasis = ds->GetPref(CLPF_Vector_basis, 0);
  
  if (!defaultVecBasis)
    defaultVecBasis = uprefs_store->GetPref(CLPF_Vector_basis);

  if (defaultVecBasis) {
    size_t zln = strlen(defaultVecBasis);
    mml_VecBasisVars = new char[zln + 1];
    strcpy(mml_VecBasisVars, defaultVecBasis);
  }

  semantic_analyzer->SetInputPrefs(msr.GetDefStore(), msr.GetEngineID());

  U32 UI_cmd_ID = msr.GetOpID();
  if (!UI_cmd_ID) {
    const char* cmd_template = NULL;
    U32 subID;
    if (GetdBaseNamedRecord("COMMANDS", msr.GetOpNamePtr(), &cmd_template, UI_cmd_ID, subID)) {
      msr.PutOpID(UI_cmd_ID);
    } else {
      TCI_ASSERT(0);
      mr.PutResultCode(CR_unsupported_command);
      return;
    }
  }
#ifdef DEBUG
  char start_msg[100];
  sprintf( start_msg, "\n\n========== Evaluate ==== Command = %d ============", UI_cmd_ID );
  JBM::JBMLine(start_msg);
#endif

  DisposeIDsList(curr_IDs_2mml);
  curr_IDs_2mml = NULL;
  // At this point, msr may be carrying "markup" parameters.
  // Typically, these parameters come from dialogs with "chambase" edits.
  // Here we add a semantics tree to all such parameters.
  // The underlying engine will need the semantics in order to form
  //  an appropriate engine string.

  int n_markups = msr.nMarkupParams();
  MarkupType m_type = msr.GetMarkupType();
  if (m_type != MT_GRAPH) {
    for (U32 p_ID = 1; p_ID < PID_last; p_ID++) {
      if (n_markups == 0)
        break;
      U32 p_type;
      const char *mk = msr.GetParam(p_ID, p_type);
      if (p_type == zPT_ASCII_mmlmarkup && mk && *mk) {
        MNODE *dMML_tree2 = mml_tree_gen->MMLstr2Tree(mk);
        JBM::DumpTList(dMML_tree2, 0);
        INPUT_NOTATION_REC *p_input_notation = NULL;
        SEMANTICS_NODE *semantics_tree =
          semantic_analyzer->BuildSemanticsTree(msr,
                                                mr, mk, dMML_tree2, UI_cmd_ID,
                                                p_input_notation);
        curr_IDs_2mml =
          JoinIDsLists(curr_IDs_2mml, semantic_analyzer->GetBackMap());
        msr.AddSemanticsToParam(p_ID, semantics_tree);
        // semantics_tree now owned by msr
        DisposeTList(dMML_tree2);
        n_markups--;
      }
    }  
  }

  // The math markup carried by the incoming service request is
  //  MathML or an engine string for now.  Add support for
  //  LaTeX/SWP internal formats later.
  switch (m_type) {

  case MT_GRAPH:{ 
      //U32 cmdID = msr.GetOpID ();
      //if (cmdID == CCID_PlotFuncQuery) {
      //}
      PlotServiceRequest *psr = static_cast<PlotServiceRequest*>(&msr);
      const U16 *wp = msr.GetWideMarkupPtr();
      char *str = WideToASCII (wp);
      const char *src = msr.GetASCIIMarkupPtr();
      MNODE *graphTree = mml_tree_gen->MMLstr2Tree (str);

      // code to convert the mathml into something more usable. The children of 
      // <plot> elements have mathml. For each, create a semantics node and
      // store in the psr
      DisposeIDsList(curr_IDs_2mml);
      curr_IDs_2mml = NULL;
      MNODE *ptr = graphTree;
      if (ptr) {       // ptr is <graph>
         ptr = ptr->first_kid;
      }   
      if (ptr && ((strncmp("graphSpec", ptr->src_tok, 9)) == 0)) {
        // grab and save the graph attributes
        for (ATTRIB_REC* aptr = ptr->attrib_list; aptr != NULL; aptr = aptr->GetNext()) {
          psr->StorePlotParam (aptr->zattr_nom, aptr->zattr_val, zPT_ASCII_text);
        }
        U32 plotno = 0;
        for (ptr = ptr->first_kid; ptr != NULL; ptr=ptr->next) {
           if ( (strcmp("plot", ptr->src_tok) == 0) || (strcmp("plotLabel", ptr->src_tok) == 0) ) {
             ++plotno;
             for (ATTRIB_REC *aptr = ptr->attrib_list; aptr != NULL; aptr = aptr->GetNext()) {
              psr->StorePlotParam (plotno, aptr->zattr_nom, aptr->zattr_val, zPT_ASCII_text);
             }
             for (MNODE *child = ptr->first_kid; child != NULL; child = child->next) {
               psr->StorePlotParam (plotno, child->src_tok, "", zPT_ASCII_mmlmarkup);
               MNODE *dMML_tree2 = child->first_kid;
               char *mmlstr = TNodeToStr(dMML_tree2, NULL, 0);
               INPUT_NOTATION_REC *p_input_notation = NULL;
               SEMANTICS_NODE *semantics_tree =
                   semantic_analyzer->BuildSemanticsTree (*psr, mr, mmlstr, dMML_tree2, 
                                                          CCID_PlotFuncCmd, p_input_notation);
               #ifdef DEBUG
                 JBM::DumpSList(semantics_tree);
               #endif
               curr_IDs_2mml =
                 JoinIDsLists(curr_IDs_2mml, semantic_analyzer->GetBackMap());
               psr->AddSemanticsToParam (plotno, child->src_tok, semantics_tree);
             }
           }
        }
      }  
      DisposeTList (graphTree);
      
      INPUT_NOTATION_REC *p_input_notation = CreateNotationRec();
      AddBasisVariables (msr);
      void * tmp;
      nsresult res = wrapper->ProcessRequest((void *) &msr, (void *) &mr, &tmp);
      SEMANTICS_NODE *res_tree = NS_SUCCEEDED(res) ? (SEMANTICS_NODE *) tmp : 0;
      RetrieveEngineStrs(mr);
      int result_code = mr.GetResultCode();
    
      if (result_code >= CR_success) {
        DefStore *ds = msr.GetDefStore();
        char *mml_str = StreeToMML->BackTranslate(res_tree, src, OUTFORMAT_MML,
                                                  curr_IDs_2mml,
                                                  def_IDs_2mml,
                                                  p_input_notation, ds,
                                                  &mr);
        if (mml_str) {
          mr.PutResultStr(mml_str, strlen(mml_str));
          delete[] mml_str;
        }
      }
      if (res_tree)
        wrapper->DisposeSList(res_tree);

    delete p_input_notation;
    mr.AttachMsgs(semantic_analyzer->GetMsgs());
  }
  break;


  case MT_UNDEFINED:         // Some computations don't carry any markup
  case MT_MATHML:{
      DisposeTList(dMML_tree);
      dMML_tree = NULL;
      SEMANTICS_NODE *semantics_tree = NULL;

      const char *src = msr.GetASCIIMarkupPtr();
      dMML_tree = mml_tree_gen->MMLstr2Tree(src);
      JBM::DumpTList(dMML_tree, 0);

      // As we build our semantics tree, info about the notation used
      //  in the input math is recorded.  Eventually, this info is passed
      //  to the object that generates output.
      INPUT_NOTATION_REC *p_input_notation = CreateNotationRec();

      if (UI_cmd_ID == CCID_Cleanup) {
        semantic_analyzer->TreeToCleanupForm(dMML_tree);
      } else if (UI_cmd_ID == CCID_Fixup) {
        semantic_analyzer->TreeToFixupForm(dMML_tree);
      } else {
        
        semantics_tree =
          semantic_analyzer->BuildSemanticsTree(msr,
                                                mr, src, dMML_tree, UI_cmd_ID,
                                                p_input_notation);
		    #ifdef DEBUG
          JBM::DumpSList(semantics_tree);
        #endif

        curr_IDs_2mml =
          JoinIDsLists(curr_IDs_2mml, semantic_analyzer->GetBackMap());

        if (mr.GetResultCode() == CR_baddefformat) {
          delete p_input_notation;
          DisposeSList(semantics_tree);
          return;
        }

        if (UI_cmd_ID == CCID_Factor) {
          if (IsMOD(semantics_tree)) {
            // Switch to "Factor in ring" command
            msr.PutOpID(CCID_Factor_in_ring);
          }
        }

      }

      if (UI_cmd_ID == CCID_Solve_Exact || UI_cmd_ID == CCID_Solve_Integer) {
        // In SWP, data for "Solve" commands may be entered in a matrix. We convert such
        //  matrices to a list type here.
        ConvertTreeToList(semantics_tree, UI_cmd_ID);
      } else if (UI_cmd_ID == CCID_Solve_Numeric) {
        // In SWP, a system of equations and solution intervals can be entered
        //  in a matrix. We convert such matrices to a dedicated SEM_TYP_NUMSYSTEM here.
        ConvertTreeToNumericSys(semantics_tree);
      } else if (UI_cmd_ID == CCID_Solve_System) {
        int res = ConvertTreeToSolveSys(semantics_tree);
        if (res == CR_badsolvesystem) {
          mr.PutResultCode(CR_badsolvesystem);
          delete p_input_notation;
          DisposeSList(semantics_tree);
          return;
        }
      } else if (UI_cmd_ID == CCID_Solve_Recursion) {

          // In SWP, a "recursion" and possibly a system of condition equations
          //  can be placed in a matrix as data to "Solve Recursion".
          // We convert such matrices to a dedicated SEM_TYP_RECURSION here.
          ConvertTreeToRecursion(semantics_tree, p_input_notation);

      } else if (UI_cmd_ID == CCID_Solve_PDE) {

          ConvertTreeToPDE(semantics_tree);

      } else if (UI_cmd_ID >= CCID_Solve_ODE_Exact && UI_cmd_ID <= CCID_Solve_ODE_Series) {

          // In SWP, ODEs are entered as matrices. We convert such
          //  matrices to a dedicated ODE type here.
          if (mr.GetResultCode() == CR_queryindepvar) {
            delete p_input_notation;
            return;
          }

          int n_boundary_conds;
          ConvertTreeToODE(semantics_tree, UI_cmd_ID, n_boundary_conds);
          if (UI_cmd_ID == CCID_Solve_ODE_Numeric && n_boundary_conds == 0) {
            mr.PutResultCode(CR_missingBoundaries);
            delete p_input_notation;
            DisposeSList(semantics_tree);
            return;
          }

      } else if (UI_cmd_ID == CCID_Moment || UI_cmd_ID == CCID_Quantile) {
        ConvertSetToPList(semantics_tree);
      } else if (UI_cmd_ID >= CCID_Simplex_Dual && UI_cmd_ID <= CCID_Simplex_Standardize) {
        // In SWP, simplexes are entered as matrices. We convert such
        //  matrices to a dedicated SEM_TYP_SIMPLEX here.
        ConvertTreeToSimplex(semantics_tree, UI_cmd_ID);
      }

      const char *def_canon_ID = NULL;
      bool generic_def = false;

      if (UI_cmd_ID == CCID_Define) {
        // Convert the semantics tree from Analyzer into a DEF form
        int convert_error;
        def_canon_ID =
          ConvertTreeToDef(msr, semantics_tree, generic_def, convert_error);
        if (convert_error != CR_success) {
          mr.PutResultCode(convert_error);
          delete p_input_notation;
          DisposeSList(semantics_tree);
          return;
        }
        if (!DefAllowed(semantics_tree)) {
          mr.PutResultCode(CR_noDefineReserved);
          delete p_input_notation;
          DisposeSList(semantics_tree);
          return;
        }
      } else if (UI_cmd_ID == CCID_DefineMupadName) {
         //
         U32 ptype;
         const char* p_mupadname = msr.GetParam(PID_mupname, ptype);
         U32 engineID = msr.GetEngineID();
         DefStore* ds = msr.GetDefStore();
         const char* ASCII_src = msr.GetASCIIMarkupPtr();
         const U16* WIDE_src = msr.GetWideMarkupPtr();
          
         // semantics_tree
         SEMANTICS_NODE* pst = semantics_tree;
         if (!pst) return;

         BUCKET_REC* pbuck = pst->bucket_list;
         if (!pbuck) return;

         SEMANTICS_NODE* pChild = pbuck->first_child;
         if (!pChild) return;

         BUCKET_REC* pChildBuck = pChild->bucket_list;
         if (!pChildBuck) return;

         SEMANTICS_NODE* pFunc = pChildBuck->first_child;
         if (!pFunc) return;


         const char* canonical = pFunc -> canonical_ID;
         ds ->PushDefInfo (engineID, 
                           canonical,
                           DT_MUPNAME,
                           DuplicateString(p_mupadname),
                           NULL,
                           0,
                           ASCII_src,
                           WIDE_src
                     );


      } else if (UI_cmd_ID == CCID_Undefine) {
        def_canon_ID = ConvertTreeToUnDef(semantics_tree);
        if (!def_canon_ID) {
          mr.PutResultCode(CR_baddefformat);
          delete p_input_notation;
          DisposeSList(semantics_tree);
          return;
        }
      }

      if (!(UI_cmd_ID == CCID_Interpret || UI_cmd_ID == CCID_Fixup || UI_cmd_ID == CCID_Cleanup)) {
        // Vector Calculus operations require "basis variables" to be specified
        // But almost any command could use vector calculus.
        AddBasisVariables(msr);
      }
      if (UI_cmd_ID == CCID_Divergence ||
          UI_cmd_ID == CCID_Curl ||
          UI_cmd_ID == CCID_Jacobian ||
          UI_cmd_ID == CCID_Wronskian ||
          UI_cmd_ID == CCID_Scalar_Potential ||
          UI_cmd_ID == CCID_Vector_Potential ||
          UI_cmd_ID == CCID_Norm) {
        // Some commands apply to matrices.  SWP allows "enclosed lists"
        //  as matrix input.
        if (semantics_tree->bucket_list)
          ListToMatrix(semantics_tree->bucket_list);
      } else if (UI_cmd_ID == CCID_Rewrite_Equations_as_Matrix) {
        if (semantics_tree->bucket_list)
          MatrixToList(semantics_tree->bucket_list);
      }

#ifdef DEBUG
      JBM::DumpSList(semantics_tree);
#endif

      if (UI_cmd_ID == CCID_Cleanup || UI_cmd_ID == CCID_Fixup) {
        // Commands w/o semantic analysis
        char *mml_str = TNodeToStr(dMML_tree, NULL, 0);
        if (mml_str) {
          mr.PutResultStr(mml_str, strlen(mml_str));
          delete[] mml_str;
          int res = mr.GetResultCode();
          if (!res)
            mr.PutResultCode(CR_success);
        }
      } else if (UI_cmd_ID == CCID_Interpret || UI_cmd_ID == CCID_Reshape ||
                 UI_cmd_ID == CCID_Concatenate || UI_cmd_ID == CCID_Stack) {
        // Commands that are NOT sent to the engine
        int error_code;
        bool OK =
          AdjustSemTree(msr, semantics_tree, UI_cmd_ID, error_code);
        if (OK) {
          // Currently, output routines take infix trees (as generated by engine wrappers)
          semantics_tree = PrefixToInfix(semantics_tree);
          // Source semantics trees don't have operator precedences set at this point.
          SetInfixPrecedences(semantics_tree);

          mr.PutSemanticsTree(semantics_tree);
          DefStore *ds = msr.GetDefStore();
          char *mml_str =
            StreeToMML->BackTranslate(semantics_tree, src, OUTFORMAT_MML,
                                      curr_IDs_2mml, def_IDs_2mml,
                                      p_input_notation, ds, &mr);
          if (mml_str) {
            mr.PutResultStr(mml_str, strlen(mml_str));
            delete[] mml_str;
            int res = mr.GetResultCode();
            if (!res)
              mr.PutResultCode(CR_success);
          }
        } else {
          TCI_ASSERT(!"AdjustSemTree failed");
          DisposeSList(semantics_tree);
          mr.PutResultCode(error_code);
        }
      } else {
        // Commands that are sent to the engine (or generic def)
        mr.PutSemanticsTree(semantics_tree);
        mr.PutResultCode(CR_success);
        nsresult res = NS_OK;
        SEMANTICS_NODE *res_tree = 0;
        if (!generic_def) {
          void * tmp;
          res = wrapper->ProcessRequest((void *)&msr, (void *)&mr, &tmp);
          if (NS_SUCCEEDED(res))
            res_tree = (SEMANTICS_NODE *) tmp;
          RetrieveEngineStrs(mr);
        } else {
          // A generic def. Delete the def and procede 
          void * tmp;

          msr.PutOpID( CCID_Undefine );
          res = wrapper->ProcessRequest((void *)&msr, (void *)&mr, &tmp);
        }
        int result_code = mr.GetResultCode();
        if (result_code >= CR_success) {
          if (UI_cmd_ID == CCID_Define) {
            SaveBackMap(curr_IDs_2mml);
            DefStore *ds = msr.GetDefStore();

            if (def_canon_ID) {
              U32 targ_engine_ID = msr.GetEngineID();
              ds->RemoveDef(targ_engine_ID, def_canon_ID);

              const char *LHS_markup = GetMarkupFromID(curr_IDs_2mml, def_canon_ID);
              char *arg_list;
              U32 n_subscripted_args;
              U32 def_type = GetDefType(semantics_tree, &arg_list, n_subscripted_args);
              const char *ASCII_src = msr.GetASCIIMarkupPtr();
              const U16 *WIDE_src = msr.GetWideMarkupPtr();
              ds->PushDefInfo(targ_engine_ID, def_canon_ID, def_type,
                              LHS_markup, arg_list, n_subscripted_args,
                              ASCII_src, WIDE_src);
              delete[] arg_list;
            } else {
              TCI_ASSERT(!"no def_canon_ID");
            }
          } else if (UI_cmd_ID == CCID_Undefine) {
            if (def_canon_ID) {
              U32 curr_client_ID = msr.GetClientHandle();
              U32 targ_engine_ID = msr.GetEngineID();
              RemoveBackMapEntry(curr_client_ID, def_canon_ID);
              DefStore *ds = msr.GetDefStore();
              ds->RemoveDef(targ_engine_ID, def_canon_ID);
            } else {
              TCI_ASSERT(!"no def_canon_ID");
            }
          } else if (UI_cmd_ID == CCID_Rewrite_Rational) {
            p_input_notation->nmixed_numbers = 0;
          }

          DefStore *ds = msr.GetDefStore();
          char *mml_str = StreeToMML->BackTranslate(res_tree, src, OUTFORMAT_MML,
                                                    curr_IDs_2mml,
                                                    def_IDs_2mml,
                                                    p_input_notation, ds,
                                                    &mr);
          if (mml_str) {
            mr.PutResultStr(mml_str, strlen(mml_str));
            delete[] mml_str;
          }
        }
        if (res_tree)
          wrapper->DisposeSList(res_tree);
      }
      delete p_input_notation;
      mr.AttachMsgs(semantic_analyzer->GetMsgs());
    }
    break;

  case MT_LATEX:
    break;

  case MT_MAPLEV_INPUT:
  case MT_MUPAD_INPUT:{
      const char *m = msr.GetASCIIMarkupPtr();
      int res;
      nsresult rv = wrapper->Execute(m,&res);
      if (NS_SUCCEEDED(rv) && res) {
        char *engstr;
        int engstr_ln;
        rv = wrapper->GetEngStrPtr(2, &engstr, &engstr_ln, &res);
        if (NS_SUCCEEDED(rv) && res)
          mr.PutResultStr(engstr, engstr_ln);
      }
      RetrieveEngineStrs(mr);
    }
    break;

  default:
    TCI_ASSERT(!"Unknown MarkupType");
    break;
  }
}

bool CompEngine::GetdBaseNamedRecord(const char *bin_name,
                                         const char *op_name,
                                         const char **eng_dbase_rec, U32 & ID,
                                         U32 & subID)
{
  if (op_name && *op_name) {
    const char *d_ztemplate;
    if (nom_dBase->
        GetRecordFromName(bin_name, op_name, strlen(op_name), ID, subID,
                          &d_ztemplate)) {
      *eng_dbase_rec = d_ztemplate;
      return true;
    }
  }

  return false;
}

bool CompEngine::GetdBaseIDRecord(const char *bin_name, U32 ID, U32 subID,
                                      const char **eng_dbase_rec,
                                      char *name_buffer, size_t lim)
{
  bool rv = false;
  *eng_dbase_rec = NULL;
  name_buffer[0] = 0;

  const char *dest_zname = NULL;
  const char *dest_ztemplate = NULL;
  if (id_dBase->
      GetRecordFromIDs(bin_name, ID, subID, &dest_zname, &dest_ztemplate)) {
    if (dest_zname && *dest_zname) {
      size_t zln = strlen(dest_zname);
      if (zln >= lim)
        zln = lim;
      strncpy(name_buffer, dest_zname, zln);
      name_buffer[zln] = 0;
    }
    *eng_dbase_rec = dest_ztemplate;
    rv = true;
  }

  return rv;
}

const char* CompEngine::GetEngineAttr(int targ_ID)
{
  const char *rv = NULL;

  ENG_ATTR_REC* a_rover = engine_attrs;
  while (a_rover) {
    if (a_rover->ID == targ_ID) {
      rv = a_rover->value;
      break;
    }
    a_rover = a_rover->next;
  }

  return rv;
}

void CompEngine::ConvertTreeToSimplex(SEMANTICS_NODE * semantics_tree,
                                      U32 cmd_ID)
{
  if (semantics_tree && semantics_tree->bucket_list) {
    BUCKET_REC *b_rover = semantics_tree->bucket_list;
    if (b_rover->first_child && b_rover->next == NULL) {
      SEMANTICS_NODE *c_content = b_rover->first_child;

      if (c_content->semantic_type == SEM_TYP_TABULATION) {
        if (c_content->ncols == 1 && c_content->nrows > 1) {
          c_content->semantic_type = SEM_TYP_SIMPLEX;
          BUCKET_REC *bl = c_content->bucket_list;
          char *obj_name;
          if (cmd_ID == CCID_Simplex_Dual) {
            obj_name = "Simplex Dual";
            bl->bucket_ID = MB_SIMPLEX_EXPR;
          } else if (cmd_ID == CCID_Simplex_Feasible) {
            obj_name = "Simplex Feasible";
          } else if (cmd_ID == CCID_Simplex_Maximize) {
            obj_name = "Simplex Maximize";
            bl->bucket_ID = MB_SIMPLEX_EXPR;
          } else if (cmd_ID == CCID_Simplex_Minimize) {
            obj_name = "Simplex Minimize";
            bl->bucket_ID = MB_SIMPLEX_EXPR;
          } else if (cmd_ID == CCID_Simplex_Standardize) {
            obj_name = "Simplex Standardize";
          }

          delete[] c_content->contents;
          size_t zln = strlen(obj_name);
          char *tmp = new char[zln + 1];
          strcpy(tmp, obj_name);
          c_content->contents = tmp;
        }
      }
    }
  }
}

void CompEngine::ConvertTreeToNumericSys(SEMANTICS_NODE * semantics_tree)
{
  if (semantics_tree && semantics_tree->bucket_list) {
    BUCKET_REC *b_rover = semantics_tree->bucket_list;
    if (b_rover->first_child && b_rover->next == NULL) {
      SEMANTICS_NODE *c_content = b_rover->first_child;
      if (c_content->semantic_type == SEM_TYP_PIECEWISE_FENCE
          || c_content->semantic_type == SEM_TYP_GENERIC_FENCE) {
        BUCKET_REC *b = c_content->bucket_list;
        if (b->first_child && b->next == NULL)
          c_content = b->first_child;
        else
          return;
      }
      if (c_content->semantic_type == SEM_TYP_TABULATION) {
        if (c_content->ncols == 1) {
          c_content->semantic_type = SEM_TYP_NUMSYSTEM;
          char *obj_name = "Numeric System";
          delete[] c_content->contents;
          size_t zln = strlen(obj_name);
          char *tmp = new char[zln + 1];
          strcpy(tmp, obj_name);
          c_content->contents = tmp;

          BUCKET_REC *b_rover = c_content->bucket_list;
          while (b_rover) {
            SEMANTICS_NODE *s_row = b_rover->first_child;
            b_rover->bucket_ID = MB_NSYS_EQUATION;
            if (s_row->semantic_type == SEM_TYP_INFIX_OP) {
              char *op_name = s_row->contents;
              if (op_name && op_name[0] == '&') {
                U32 op_unicode = NumericEntity2U32(s_row->contents);
                if (op_unicode == 0x2208 || op_unicode == 0x220a)
                  b_rover->bucket_ID = MB_NSYS_CONDITION;
              }
            }
            b_rover = b_rover->next;
          }
        }
      } else {                  // the math for "Solve Numeric" is not in a matrix
      }
    }
  } else {
    TCI_ASSERT(0);
  }
}

int CompEngine::ConvertTreeToSolveSys(SEMANTICS_NODE * semantics_tree)
{
  int rv = CR_badsolvesystem;

  if (semantics_tree && semantics_tree->bucket_list) {
    // Descend into the meat
    BUCKET_REC *b_rover = semantics_tree->bucket_list;

    SEMANTICS_NODE *c_content = b_rover->first_child;
    while (!b_rover->next
           && c_content->semantic_type == SEM_TYP_PRECEDENCE_GROUP
           && !c_content->next) {
      b_rover = c_content->bucket_list;
      if (b_rover->first_child) {
        c_content = b_rover->first_child;
      } else {
        TCI_ASSERT(0);
        return rv;
      }
    }

    int OK = 0;
    U32 lhs_nrows = 0;
    U32 rhs_nrows = 0;

    BUCKET_REC *assign_bucket = NULL;
    SEMANTICS_NODE *left_side = NULL;
    BUCKET_REC *left_arg_bucket = NULL;
    SEMANTICS_NODE *left_arg = NULL;

    // Examine the form of the input tree
    SEMANTICS_NODE *assign_op = c_content;
    if (assign_op->semantic_type == SEM_TYP_INFIX_OP
        && assign_op->bucket_list && assign_op->bucket_list->next) {
      if (!strcmp(assign_op->contents, "=")) {
        assign_bucket = assign_op->bucket_list;
        left_side = assign_bucket->first_child;
        SEMANTICS_NODE *right_side = assign_bucket->next->first_child;

        if (right_side->semantic_type == SEM_TYP_TABULATION) {
          if (right_side->ncols == 1) {
            OK++;
            rhs_nrows = right_side->nrows;
          }
        }

        if (left_side->semantic_type == SEM_TYP_INFIX_OP
            && left_side->bucket_list && left_side->bucket_list->next) {
          if (!strcmp(left_side->contents, "&#x2062;")) {
            left_arg_bucket = left_side->bucket_list;
            left_arg = left_arg_bucket->first_child;
            SEMANTICS_NODE *right_arg = left_arg_bucket->next->first_child;

            if (left_arg->semantic_type == SEM_TYP_TABULATION) {
              OK++;
              lhs_nrows = left_arg->nrows;
            }

            if (right_arg->semantic_type == SEM_TYP_TABULATION) {
              if (right_arg->ncols == 1) {
                OK++;
                if (left_arg->ncols == right_arg->nrows)
                  OK++;
              }
            }
          }
        }
      } else {
        TCI_ASSERT(0);
      }
    }

    if (OK == 4 && lhs_nrows == rhs_nrows) {
      left_arg_bucket->first_child = NULL;
      left_arg->parent = NULL;

      DisposeSList(left_side);

      assign_bucket->first_child = left_arg;
      left_arg->parent = assign_bucket;
      assign_bucket->bucket_ID = MB_SYSTEM_COEFFICIENTS;
      assign_bucket->next->bucket_ID = MB_SYSTEM_VALUES;

      assign_op->semantic_type = SEM_TYP_SOLVESYSTEM;

      char *obj_name = "Solve System";
      delete[] assign_op->contents;
      size_t zln = strlen(obj_name);
      char *tmp = new char[zln + 1];
      strcpy(tmp, obj_name);
      assign_op->contents = tmp;

      rv = 0;
    }
  } else {
    TCI_ASSERT(0);
  }
  return rv;
}

void CompEngine::ConvertTreeToPDE(SEMANTICS_NODE * semantics_tree)
{
  if (semantics_tree && semantics_tree->bucket_list) {

    BUCKET_REC *b_rover = semantics_tree->bucket_list;

    // Note that our math container carries an extra bucket here.
    //  It contains the name of the DE function.
    BUCKET_REC *b_odefunc = b_rover->next;
    if (b_odefunc)
      b_rover->next = NULL;

    if (b_rover->first_child && b_rover->next == NULL) {
      SEMANTICS_NODE* s_PDE = b_rover->first_child;

      SEMANTICS_NODE* s_save = NULL;
      if (s_PDE->semantic_type == SEM_TYP_TABULATION) {
        TCI_ASSERT(0);
      } else {                  //    if ( s_PDE->semantic_type == SEM_TYP_TABULATION )
        SEMANTICS_NODE* s_ODE = CreateSemanticsNode(SEM_TYP_PDE);
        //s_ODE->semantic_type = SEM_TYP_PDE;
        s_save = s_ODE;

        BUCKET_REC* b_list = MakeParentBucketRec(MB_UNNAMED, s_PDE);
        
        s_ODE->bucket_list = b_list;
        b_rover->first_child = s_ODE;

        if (b_odefunc)
          s_ODE->bucket_list = AppendBucketRec(s_ODE->bucket_list, b_odefunc);
      }

      if (s_save) {
        char *obj_name = "PDE";
        size_t zln = strlen(obj_name);
        char *tmp = new char[zln + 1];
        strcpy(tmp, obj_name);
        s_save->contents = tmp;
      }
    }
  } else
    TCI_ASSERT(0);
}

void CompEngine::ConvertTreeToODE(SEMANTICS_NODE* semantics_tree,
                                  U32 cmd_ID, 
                                  int& n_boundary_conditions)
{
  n_boundary_conditions = 0;

  if (semantics_tree && semantics_tree->bucket_list) {

    BUCKET_REC* b_rover = semantics_tree->bucket_list;

    // Note that our math container carries an extra bucket here.
    //  It contains the ODEfunc.
    BUCKET_REC* b_odefunc = b_rover->next;
    if (b_odefunc) {
      b_rover->next = NULL;
    }

    if (b_rover->first_child && b_rover->next == NULL) {

      SEMANTICS_NODE* s_ODEsystem = b_rover->first_child;
      SEMANTICS_NODE* s_save = NULL;

      if (s_ODEsystem->semantic_type == SEM_TYP_TABULATION) {

        if (s_ODEsystem->ncols == 1 && s_ODEsystem->nrows > 1) {

          s_ODEsystem->semantic_type = SEM_TYP_ODE_SYSTEM;
          delete[] s_ODEsystem->contents;
          s_save = s_ODEsystem;

          // loop thru lines in this tabulation,
          //   determine which lines are boundary conditions.
          BUCKET_REC* b_rover = s_ODEsystem->bucket_list;
          while (b_rover) {
            SEMANTICS_NODE* s_line = b_rover->first_child;
            if (IsBoundaryCondition(s_line)) {
              b_rover->bucket_ID = MB_BOUNDARY_CONDITION;
              n_boundary_conditions++;
            }
            b_rover = b_rover->next;
          }

          if (b_odefunc)
            s_ODEsystem->bucket_list = AppendBucketRec(s_ODEsystem->bucket_list, b_odefunc);

        } else
          TCI_ASSERT(0);

      } else {                  //    if ( s_ODEsystem->semantic_type == SEM_TYP_TABULATION )

        SEMANTICS_NODE* s_ODE = CreateSemanticsNode(SEM_TYP_ODE_SYSTEM);
        //s_ODE->semantic_type = SEM_TYP_ODE_SYSTEM;
        s_save = s_ODE;

        BUCKET_REC* b_list = MakeParentBucketRec(MB_UNNAMED, s_ODEsystem);
        
        s_ODE->bucket_list = b_list;
        b_rover->first_child = s_ODE;

        if (b_odefunc)
          s_ODE->bucket_list = AppendBucketRec(s_ODE->bucket_list, b_odefunc);
      }

      if (s_save) {
        char *obj_name = NULL;
        if (cmd_ID == CCID_Solve_ODE_Exact) {
          obj_name = "ODE Exact";
        } else if (cmd_ID == CCID_Solve_ODE_Laplace) {
          obj_name = "ODE Laplace";
        } else if (cmd_ID == CCID_Solve_ODE_Numeric) {
          obj_name = "ODE Numeric";
        } else if (cmd_ID == CCID_Solve_ODE_Series) {
          obj_name = "ODE Series";
        }
        size_t zln = strlen(obj_name);
        char *tmp = new char[zln + 1];
        strcpy(tmp, obj_name);
        s_save->contents = tmp;
      }
    }
  } else
    TCI_ASSERT(0);
}

void CompEngine::ConvertTreeToRecursion(SEMANTICS_NODE * semantics_tree,
                                        INPUT_NOTATION_REC * p_input_notation)
{
  if (semantics_tree && semantics_tree->bucket_list) {
    // Descend into the meat
    BUCKET_REC *b_rover = semantics_tree->bucket_list;
    SEMANTICS_NODE *s_child = b_rover->first_child;

    while (!b_rover->next
           && s_child->semantic_type == SEM_TYP_PRECEDENCE_GROUP
           && !s_child->next) {
      BUCKET_REC *b = s_child->bucket_list;
      if (b->first_child && b->next == NULL)
        s_child = b->first_child;
      else
        return;
    }

    SEMANTICS_NODE *s_save = NULL;
    if (s_child->semantic_type == SEM_TYP_INFIX_OP) {
      BUCKET_REC *parent_buc = s_child->parent;

      SEMANTICS_NODE *s_recur = CreateSemanticsNode(SEM_TYP_RECURSION);
      //s_recur->semantic_type = SEM_TYP_RECURSION;
      s_save = s_recur;

      BUCKET_REC* b_list = MakeParentBucketRec(MB_RECUR_EQN, s_child);
      s_recur->bucket_list = b_list;
      

      parent_buc->first_child = s_recur;
      s_recur->parent = parent_buc;

      BUCKET_REC* b = MakeBucketRec(MB_RECUR_FUNC, NULL);
      s_recur->bucket_list = AppendBucketRec(s_recur->bucket_list, b);
      AddRecurFuncNode(s_recur, b);
    } else if (s_child->semantic_type == SEM_TYP_TABULATION) {
      s_save = s_child;
      s_child->semantic_type = SEM_TYP_RECURSION;

      BUCKET_REC *b_rover = s_child->bucket_list;
      while (b_rover) {
        b_rover->bucket_ID = MB_RECUR_EQN;
        b_rover = b_rover->next;
      }

      BUCKET_REC *b = MakeBucketRec(MB_RECUR_FUNC, NULL);
      s_child->bucket_list = AppendBucketRec(s_child->bucket_list, b);
      AddRecurFuncNode(s_child, b);

    } else {
      TCI_ASSERT(0);
    }
    if (s_save) {
      delete[] s_save->contents;
      char *obj_name = "Recursion";
      size_t zln = strlen(obj_name);
      char *tmp = new char[zln + 1];
      strcpy(tmp, obj_name);
      s_save->contents = tmp;
    }
  } else {
    TCI_ASSERT(0);
  }
}

void CompEngine::ConvertTreeToList(SEMANTICS_NODE * semantics_tree,
                                   U32 cmd_ID)
{
  if (semantics_tree && semantics_tree->bucket_list) {

    BUCKET_REC *b_rover = semantics_tree->bucket_list;
    if (b_rover->first_child && b_rover->next == NULL) {
      SEMANTICS_NODE *c_content = b_rover->first_child;

      if (c_content->semantic_type == SEM_TYP_TABULATION) {
        if (c_content->ncols == 1) {
          c_content->semantic_type = SEM_TYP_LIST;
          char *obj_name = "Solve List";

          delete[] c_content->contents;
          size_t zln = strlen(obj_name);
          char *tmp = new char[zln + 1];
          strcpy(tmp, obj_name);
          c_content->contents = tmp;
        }
      }
    }
  } else {
    TCI_ASSERT(0);
  }
}

/* When Analyzer produces a semantics tree from an SWP equation
 the tree looks something like the following:

MATH_CONTAINER
Bucket list
  UNNAMED
    PRECEDENCE_GROUP
    Bucket list
      UNNAMED
        INFIX OPERATOR contents = "="
        VARIABLE canonical_ID = "mib" contents = "b"
        NUMBER canonical_ID = "mn5" contents = "5"
    End Bucket list
End Bucket list

 If the command being processed is "Compute:Definitions:New Definition",
  we re-cast the semantics tree from an equation to a definition.
*/

const char *CompEngine::ConvertTreeToDef(MathServiceRequest & msr,
                                         SEMANTICS_NODE * semantics_tree,
                                         bool & generic_def,
                                         int & error_code)
{
  const char *rv = NULL;
  generic_def = false;
  error_code = CR_success;

  if (semantics_tree && semantics_tree->bucket_list) {
    BUCKET_REC *outer_math_bucket = semantics_tree->bucket_list;
    SEMANTICS_NODE *c_content = outer_math_bucket->first_child;
    if (c_content) {
      // Typically we need to descend into a PRECEDENCE_GROUP here.
      if (c_content->semantic_type == SEM_TYP_PRECEDENCE_GROUP &&
          !c_content->next) {
        BUCKET_REC *b = c_content->bucket_list;
        if (b->first_child && b->next == NULL) {
          c_content = b->first_child;
        } else {
          TCI_ASSERT(0);
          error_code = CR_baddefformat;
          return rv;
        }
      }
      SEMANTICS_NODE *assign_op = c_content;
      if (assign_op->bucket_list && assign_op->bucket_list->next &&
          assign_op->semantic_type == SEM_TYP_INFIX_OP) {
        if (!strcmp(assign_op->contents, "=") ||
            !strcmp(assign_op->contents, ":=")) {
          // could see unicode 0x2254 here!
          BUCKET_REC *b = assign_op->bucket_list;
          SEMANTICS_NODE *left_side = b->first_child;
//          SEMANTICS_NODE *right_side = b->next->first_child;

          bool is_func_def = false;
          bool is_var_def = false;
          if (left_side &&
              left_side->semantic_type == SEM_TYP_QUALIFIED_VAR) {
            if (left_side->bucket_list) {
              BUCKET_REC *b =
                FindBucketRec(left_side->bucket_list, MB_SUB_QUALIFIER);
              if (b && b->first_child) {
                SEMANTICS_NODE *q_cont = b->first_child;
                bool sub_is_candidate = false;
                int n_args = 0;
                if (IsVariableInSubscript(q_cont)) {
                  sub_is_candidate = true;
                } else if (IsSubVariableList(q_cont, n_args)) {
                  sub_is_candidate = true;
                }
                if (sub_is_candidate) {
                  U32 p_type;
                  const char *p_str =
                    msr.GetParam(PID_subInterpretation, p_type);
                  if (!p_type) {
                    error_code = CR_NeedSubInterp;
                    return rv;
                  } else if (p_type == zPT_ASCII_natural) {
                    int interp_ilk = atoi(p_str);
                    if (interp_ilk == 1) {
                      is_var_def = true;
                    } else if (interp_ilk == 2) {
                      is_func_def = true;
                    } else {
                      TCI_ASSERT(!"Parameter must be 1 or 2.");
                      // error_code set below
                    }
                  } else {
                    TCI_ASSERT(!"Wrong parameter type.");
                    is_var_def = true;
                  }
                } else {
                  is_var_def = true;
                }
              } else {
                TCI_ASSERT(!"Looks like nonsense.");
                error_code = CR_baddefformat;
              }
            } else {
              TCI_ASSERT(!"Impossible.");
              error_code = CR_baddefformat;
            }
          } else {
            if (IsFunction(left_side))
              is_func_def = true;
            else if (IsVariable(left_side))
              is_var_def = true;
          }

          if (is_func_def) {
            rv = ConvertTreeToFuncDef(semantics_tree);
          } else if (is_var_def) {
            rv = ConvertTreeToSymbolAssign(semantics_tree);
          } else {
            // TCI_ASSERT(!"Bad parse of math.");
            error_code = CR_baddefformat;
          }
        } else {
          TCI_ASSERT(!"Op in definition not assignment.");
          error_code = CR_baddefformat;
        }
      } else if (assign_op->semantic_type == SEM_TYP_FUNCTION  && !assign_op->next && assign_op->bucket_list) {
        generic_def = true;
        rv = assign_op->canonical_ID;
      } else if (assign_op->semantic_type == SEM_TYP_VARIABLE &&  !assign_op->next) {
        generic_def = true;
        rv = assign_op->canonical_ID;
      } else if (assign_op->semantic_type == SEM_TYP_QUALIFIED_VAR && !assign_op -> next) {
        U32 p_type;
        const char *p_str = msr.GetParam(PID_subInterpretation, p_type);
        if (!p_type) {
          generic_def = true;
          error_code = CR_NeedSubInterp;
        }
        rv = assign_op->canonical_ID;
      } else {
        TCI_ASSERT(!"No content for definition.");
        error_code = CR_baddefformat;
      }
    } else {
      TCI_ASSERT(!"Empty definition.");
      error_code = CR_baddefformat;
    }
  }
  return rv;
}

// The input to an "Undefine" might be
//    "a"
//    "f(x)"
//    "a:=5"
//    "f(x)=x^{2} - 1"
// We only want the left side.

char *CompEngine::ConvertTreeToUnDef(SEMANTICS_NODE * semantics_tree)
{
  char *rv = NULL;

  if (semantics_tree && semantics_tree->bucket_list) {
    BUCKET_REC *outer_math_bucket = semantics_tree->bucket_list;
    SEMANTICS_NODE *c_content = outer_math_bucket->first_child;
    if (c_content) {
      // Typically we need to descend into a SEM_TYP_PRECEDENCE_GROUP here.
      if (c_content->semantic_type == SEM_TYP_PRECEDENCE_GROUP) {
        BUCKET_REC *b = c_content->bucket_list;
        if (b->first_child && b->next == NULL)
          c_content = b->first_child;
        else {
          TCI_ASSERT(0);
          return rv;
        }
      }

      SEMANTICS_NODE *assign_op = c_content;
      if (assign_op->semantic_type == SEM_TYP_INFIX_OP
          && assign_op->bucket_list && assign_op->bucket_list->next) {
        if (!strcmp(assign_op->contents, "=")
            || !strcmp(assign_op->contents, ":=")) {
          // could see unicode 0x2254 here!
          BUCKET_REC *b = assign_op->bucket_list;
          SEMANTICS_NODE *left_side = b->first_child;
          if (left_side) {
            b->first_child = NULL;
            DisposeSList(outer_math_bucket->first_child);
            outer_math_bucket->first_child = left_side;
            left_side->parent = outer_math_bucket;
            rv = left_side->canonical_ID;
          } else
            TCI_ASSERT(0);
        } else
          TCI_ASSERT(0);
      } else if (assign_op->semantic_type == SEM_TYP_FUNCTION
                 && !assign_op->next && assign_op->bucket_list) {
        rv = assign_op->canonical_ID;

      } else if (assign_op->semantic_type == SEM_TYP_VARIABLE
                 && !assign_op->next) {
        rv = assign_op->canonical_ID;
      } else {
        TCI_ASSERT(!"undefining something other than variable or func?");
      }
    } else {
      TCI_ASSERT(0);
    }
  }
  return rv;
}

void CompEngine::ConvertSetToPList(SEMANTICS_NODE * semantics_tree)
{
  if (semantics_tree && semantics_tree->bucket_list) {
    BUCKET_REC *outer_math_bucket = semantics_tree->bucket_list;
    SEMANTICS_NODE *c_content = outer_math_bucket->first_child;
    if (c_content) {
      if (c_content->semantic_type == SEM_TYP_SET) {
        c_content->semantic_type = SEM_TYP_PARENED_LIST;
      }
    }
  }
}

bool CompEngine::IsFunction(SEMANTICS_NODE * left_side)
{
  bool rv = false;

  SEMANTICS_NODE *s_key = left_side;
  if (s_key && s_key->semantic_type == SEM_TYP_PRECEDENCE_GROUP) {
    BUCKET_REC *bucket = s_key->bucket_list;
    s_key = bucket->first_child;
  }
  if (s_key)
    if (s_key->semantic_type == SEM_TYP_FUNCTION)
      rv = true;

  return rv;
}

bool CompEngine::IsVariable(SEMANTICS_NODE * left_side)
{
  bool rv = false;

  SEMANTICS_NODE *s_key = left_side;
  if (s_key && s_key->semantic_type == SEM_TYP_PRECEDENCE_GROUP) {
    BUCKET_REC *bucket = s_key->bucket_list;
    s_key = bucket->first_child;
  }
  if (s_key)
    if (s_key->semantic_type == SEM_TYP_VARIABLE)
      rv = true;

  return rv;
}

const char *CompEngine::ConvertTreeToSymbolAssign(SEMANTICS_NODE *
                                                  semantics_tree)
{
  char *rv = NULL;

  BUCKET_REC *outer_math_bucket = semantics_tree->bucket_list;
  SEMANTICS_NODE *morph = outer_math_bucket->first_child;

  if (morph->semantic_type == SEM_TYP_PRECEDENCE_GROUP && !morph->next) {
  } else {
    morph = InsertPGroup(semantics_tree);
  }

  // We must descend into a SEM_TYP_PRECEDENCE_GROUP here.
  SEMANTICS_NODE *c_content = morph;
  if (c_content->semantic_type == SEM_TYP_PRECEDENCE_GROUP) {
    BUCKET_REC *b = c_content->bucket_list;
    if (b->first_child && b->next == NULL)
      c_content = b->first_child;
    else {
      TCI_ASSERT(0);
      return rv;
    }
  } else
    TCI_ASSERT(0);

  SEMANTICS_NODE *assign_op = c_content;
  if (assign_op->semantic_type == SEM_TYP_INFIX_OP
      && assign_op->bucket_list) {
    if (!strcmp(assign_op->contents, ":="))
      morph->semantic_type = SEM_TYP_VARDEF;
    else
      morph->semantic_type = SEM_TYP_VARDEF_DEFERRED;

    BUCKET_REC *parts_list = assign_op->bucket_list;
    assign_op->bucket_list = NULL;

    BUCKET_REC *old_parent = assign_op->parent;

    parts_list->bucket_ID = MB_DEF_VARNAME;
    parts_list->next->bucket_ID = MB_DEF_VARVALUE;

    SEMANTICS_NODE *assigned_var = parts_list->first_child;
    assigned_var->owner_ID = 0;
    rv = assigned_var->canonical_ID;

    TCI_ASSERT(morph->bucket_list == old_parent);
    DisposeBucketList(old_parent);

    morph->bucket_list = parts_list;

    BUCKET_REC *bucket = parts_list->next;
    if (bucket && bucket->first_child) {
      SEMANTICS_NODE *s_orphan = bucket->first_child;
      if (!s_orphan->next) {
        if (s_orphan->semantic_type == SEM_TYP_BRACKETED_LIST
            || s_orphan->semantic_type == SEM_TYP_PARENED_LIST)
          ListToMatrix(bucket);
      }
    }
  } else
    TCI_ASSERT(0);

  return rv;
}

const char *CompEngine::ConvertTreeToFuncDef(SEMANTICS_NODE * semantics_tree)
{
  const char *rv = NULL;

  BUCKET_REC *outer_math_bucket = semantics_tree->bucket_list;
  SEMANTICS_NODE *morph = outer_math_bucket->first_child;

  // Generate a fixed form here - math containing a single precedence group
  if (morph->semantic_type == SEM_TYP_PRECEDENCE_GROUP && !morph->next) {
  } else {
    morph = InsertPGroup(semantics_tree);
  }

  // We must descend into a SEM_TYP_PRECEDENCE_GROUP here.
  SEMANTICS_NODE *c_content = morph;
  if (c_content->semantic_type == SEM_TYP_PRECEDENCE_GROUP) {
    BUCKET_REC *b = c_content->bucket_list;
    if (b->first_child && b->next == NULL)
      c_content = b->first_child;
    else {
      TCI_ASSERT(0);
      return rv;
    }
  } else
    TCI_ASSERT(0);

  SEMANTICS_NODE *assign_op = c_content;

  BUCKET_REC *del_bucket = assign_op->parent;

  morph->semantic_type = SEM_TYP_FUNCDEF;
  BUCKET_REC *morphs_bucket_list = assign_op->bucket_list;
  assign_op->bucket_list = NULL;
  morph->bucket_list = morphs_bucket_list;
  morphs_bucket_list->bucket_ID = MB_DEF_FUNCHEADER;

  SEMANTICS_NODE *f_header = morphs_bucket_list->first_child;
  f_header->owner_ID = 0;

  SEMANTICS_NODE *left_side = RemovePGroup(morphs_bucket_list->first_child);
  morphs_bucket_list->first_child = left_side;
  left_side->parent = morphs_bucket_list;

  if (left_side->semantic_type == SEM_TYP_QUALIFIED_VAR)
    rv = QVarToFuncHeader(left_side);
  else
    rv = left_side->canonical_ID;

  BUCKET_REC *func_val_bucket = morphs_bucket_list->next;
  func_val_bucket->bucket_ID = MB_DEF_FUNCVALUE;
  SEMANTICS_NODE *right_side = func_val_bucket->first_child;

  if (right_side->semantic_type == SEM_TYP_PIECEWISE_FENCE) {
    TCI_ASSERT(0);
  } else if (right_side->semantic_type == SEM_TYP_PIECEWISE_LIST) {
    func_val_bucket->first_child = right_side;
    right_side->parent = func_val_bucket;
  } else {
    right_side = RemovePGroup(right_side);
    func_val_bucket->first_child = right_side;
    right_side->parent = func_val_bucket;
  }

  DisposeBucketList(del_bucket);

  // "i" may be a formal variable (argument), as in "f(i,j)"
  // If so, we need to make sure that it hasn't been cast as "imaginaryi".

  FixiArgument(morphs_bucket_list);

  // In a def like a(i,j) = a_{i,j}, we change SEM_TYP_QUALIFIED_VAR
  //  to SEM_TYP_INDEXED_VAR.  ie. the subscript is taken to mean indexing.

  if (morphs_bucket_list
      && morphs_bucket_list->first_child
      && morphs_bucket_list->next && morphs_bucket_list->next->first_child) {
    SEMANTICS_NODE *f_header = morphs_bucket_list->first_child;

    BUCKET_REC *func_val_bucket = morphs_bucket_list->next;
    SEMANTICS_NODE *right_side = func_val_bucket->first_child;

    FixIndexedVars(right_side, f_header->bucket_list);
  }

  return rv;
}

SEMANTICS_NODE *CompEngine::RemovePGroup(SEMANTICS_NODE * s_node)
{
  SEMANTICS_NODE *rv = s_node;

  if (s_node && s_node->bucket_list &&
      s_node->semantic_type == SEM_TYP_PRECEDENCE_GROUP &&
      s_node->prev == NULL && s_node->next == NULL) {
    BUCKET_REC *bucket_list = s_node->bucket_list;
    if (bucket_list->next == NULL && bucket_list->first_child) {
      rv = bucket_list->first_child;
      bucket_list->first_child = NULL;
      rv->parent = NULL;
      DisposeSList(s_node);
    }
  }

  return rv;
}

bool CompEngine::AdjustSemTree(MathServiceRequest& msr,
                               SEMANTICS_NODE* s_tree,
                               U32 cmd_ID, 
                               int& error_code)
{
  bool rv = false;
  error_code = 0;

  if (s_tree) {
    if (cmd_ID == CCID_Interpret || cmd_ID == CCID_Cleanup || cmd_ID == CCID_Fixup)
      return true;

    BUCKET_REC *parent_bucket = NULL;

    SEMANTICS_NODE *s_rover = s_tree;

    if (s_rover->semantic_type == SEM_TYP_MATH_CONTAINER) {
      if (s_rover->bucket_list) {
        parent_bucket = s_rover->bucket_list;
        s_rover = s_rover->bucket_list->first_child;
      } else {
        TCI_ASSERT(0);
        error_code = CR_failure;
        return false;
      }
    }

    if (s_rover->semantic_type == SEM_TYP_PRECEDENCE_GROUP
        && !s_rover->next) {
      if (s_rover->bucket_list) {
        parent_bucket = s_rover->bucket_list;
        s_rover = s_rover->bucket_list->first_child;
      } else {
        TCI_ASSERT(0);
        error_code = CR_failure;
        return false;
      }
    }

    if (cmd_ID == CCID_Concatenate) {
      // When 2 consecutive matrices occur in the input,
      //  an invisible times or comma may be inserted between them.
      if (s_rover
          && s_rover->semantic_type == SEM_TYP_INFIX_OP
          && IsSeparatorOp(s_rover)) {
        int n_rows;
        if (OKtoConcat(s_rover, n_rows)) {
          ConcatMatrixITMatrix(s_rover, n_rows);
          rv = true;
        }
      } else if (s_rover
                 && s_rover->semantic_type == SEM_TYP_TABULATION
                 && s_rover->next
                 && s_rover->next->semantic_type == SEM_TYP_TABULATION) {

        if (s_rover->nrows == s_rover->next->nrows) {
          ConcatMatrixMatrix(s_rover);
          rv = true;
        } else {
          TCI_ASSERT(0);
          error_code = CR_failure;
        }
      } else {
        TCI_ASSERT(0);
        error_code = CR_failure;
      }
    } else if (cmd_ID == CCID_Stack) {
      if (s_rover
          && s_rover->semantic_type == SEM_TYP_INFIX_OP
          && IsSeparatorOp(s_rover)) {
        int n_cols;
        if (OKtoStack(s_rover, n_cols)) {
          StackMatrices(s_rover, n_cols);
          rv = true;
        }
      } else if (s_rover
                 && s_rover->semantic_type == SEM_TYP_TABULATION
                 && s_rover->next
                 && s_rover->next->semantic_type == SEM_TYP_TABULATION) {

        if (s_rover->ncols == s_rover->next->ncols) {
          // Implement this sometime!
          //        StackMatrixMatrix( s_rover );
          //         rv  =  true;
          TCI_ASSERT(0);
          error_code = CR_failure;
        } else {
          TCI_ASSERT(0);
          error_code = CR_failure;
        }
      } else {
        TCI_ASSERT(0);
        error_code = CR_failure;
      }
    } else if (cmd_ID == CCID_Reshape) {
      if (s_rover) {
        if (s_rover->semantic_type == SEM_TYP_BRACKETED_LIST
            || s_rover->semantic_type == SEM_TYP_PARENED_LIST
            || s_rover->semantic_type == SEM_TYP_SET) {
          ListToMatrix(parent_bucket);
        }

        if (s_rover->semantic_type == SEM_TYP_TABULATION) {
          U32 n_cells = s_rover->nrows * s_rover->ncols;
          int n_cols = GetNColumns(msr, error_code);
          if (!error_code) {
            U32 new_ncols = n_cols;
            U32 new_nrows = (n_cells + new_ncols - 1) / new_ncols;
            s_rover->ncols = new_ncols;
            s_rover->nrows = new_nrows;
            rv = true;
          } else {
            rv = false;
            TCI_ASSERT(0);
          }
        } else if (IsCommaList(s_rover)) {  // SWP will "Reshape" a comma separated list.
          //We create a table from a list here - major tree manipulation
          BUCKET_REC *b_l = ExtractCommaList(s_rover, NULL);
          if (b_l) {
            SEMANTICS_NODE *s_node = CreateSemanticsNode(SEM_TYP_TABULATION);
            //s_node->semantic_type = SEM_TYP_TABULATION;
            s_node->bucket_list = b_l;

            parent_bucket->first_child = s_node;
            s_node->parent = parent_bucket;

            U32 n_cells = 0;
            while (b_l) {
              n_cells++;
              b_l = b_l->next;
            }

            int n_cols = GetNColumns(msr, error_code);
            if (!error_code) {
              U32 new_ncols = n_cols;
              U32 new_nrows = (n_cells + new_ncols - 1) / new_ncols;
              s_node->ncols = new_ncols;
              s_node->nrows = new_nrows;
              rv = true;
            } else {
              s_node->ncols = n_cells;
              s_node->nrows = 1;
              rv = false;
            }
          } else {
            TCI_ASSERT(0);
          }
        } else {                // the input to Reshape is not a matrix or a comma list
          TCI_ASSERT(0);
        }
      } else {
        TCI_ASSERT(0);
      }
    } else {
      TCI_ASSERT(0);
    }
  }                             // if ( s_tree )

  return rv;
}

// Note that the following code looks for and handles pmatrix and bmatrix.
// Currently, this isn't necessary.  pmatrix and bmatrix are mapped
//  to matrix in Tree2StdMML, so they never get this far - generally
//  matrix delimiters are not semantic.  The few cases (like |m|) where
//  they are semantic are handled early in Analyzer.

void CompEngine::ConcatMatrixITMatrix(SEMANTICS_NODE * s_times_op, int n_rows)
{
  BUCKET_REC **bucket_lists = new BUCKET_REC *[n_rows];
  int i = 0;
  while (i < n_rows)
    bucket_lists[i++] = NULL;

  ExtractCells(s_times_op, bucket_lists, n_rows);

  s_times_op->semantic_type = SEM_TYP_TABULATION;
  DisposeBucketList(s_times_op->bucket_list);
  s_times_op->bucket_list = NULL;

  delete[] s_times_op->contents;
  char *obj_name = "matrix";
  size_t zln = strlen(obj_name);
  char *tmp = new char[zln + 1];
  strcpy(tmp, obj_name);
  s_times_op->contents = tmp;

  int total_cols = 0;
  BUCKET_REC *b_rover = bucket_lists[0];
  while (b_rover) {
    total_cols++;
    b_rover = b_rover->next;
  }

  for (i = 0;i < n_rows; i++) {
    s_times_op->bucket_list = AppendBucketRec(s_times_op->bucket_list,
                                              bucket_lists[i]);
  }

  s_times_op->ncols = total_cols;
  s_times_op->nrows = n_rows;
  delete[] bucket_lists;
}

void CompEngine::ConcatMatrixMatrix(SEMANTICS_NODE * s_matrix)
{
  U32 n_rows = s_matrix->nrows;

  BUCKET_REC **bucket_lists = new BUCKET_REC *[n_rows];
  U32 i = 0;
  while (i < n_rows)
    bucket_lists[i++] = NULL;

  SEMANTICS_NODE *curr_matrix = s_matrix;
  while (curr_matrix) {
    U32 n_cols = curr_matrix->ncols;
    BUCKET_REC *c_rover = curr_matrix->bucket_list;
    curr_matrix->bucket_list = NULL;

    U32 i = 0;
    while (i < n_rows) {
      bucket_lists[i] = AppendBucketRec(bucket_lists[i], c_rover);
      U32 j = 0;
      BUCKET_REC *prev = NULL;
      while (j < n_cols) {
        prev = c_rover;
        c_rover = c_rover->next;
        j++;
      }
      if (prev)
        prev->next = NULL;

      i++;
    }

    curr_matrix = curr_matrix->next;
    if (curr_matrix) {
      if (curr_matrix->semantic_type != SEM_TYP_TABULATION)
        break;
      else if (curr_matrix->nrows != n_rows)
        break;
    }
  }

  // Count columns in the output matrix
  U32 total_cols = 0;
  BUCKET_REC *b_rover = bucket_lists[0];
  while (b_rover) {
    total_cols++;
    b_rover = b_rover->next;
  }

  s_matrix->ncols = total_cols;
  s_matrix->nrows = n_rows;

  // Put buckets on the output matrix
  i = 0;
  while (i < n_rows) {
    s_matrix->bucket_list = AppendBucketRec(s_matrix->bucket_list,
                                            bucket_lists[i]);
    i++;
  }
  delete[] bucket_lists;

  // Dispose the s_matrices that have been cannibalized
  curr_matrix = s_matrix->next;
  while (curr_matrix) {
    SEMANTICS_NODE *new_right = curr_matrix->next;

    curr_matrix->next = NULL;
    DisposeSemanticsNode(curr_matrix);

    s_matrix->next = new_right;
    if (new_right) {
      new_right->prev = s_matrix;
      if (new_right->semantic_type != SEM_TYP_TABULATION)
        break;
      else if (new_right->nrows != n_rows)
        break;
      else
        curr_matrix = new_right;
    } else {
      break;
    }
  }
}

void CompEngine::StackMatrices(SEMANTICS_NODE * s_times_op, int n_cols)
{
  BUCKET_REC *b_rover = s_times_op->bucket_list;

  SEMANTICS_NODE *m1 = LocateMatrixInBucket(b_rover);
  SEMANTICS_NODE *m2 = LocateMatrixInBucket(b_rover->next);
  if (m1 && m2) {
    int rows1 = m1->nrows;
    int rows2 = m2->nrows;

    BUCKET_REC *cells1 = m1->bucket_list;
    m1->bucket_list = NULL;
    BUCKET_REC *cells2 = m2->bucket_list;
    m2->bucket_list = NULL;

    DisposeBucketList(s_times_op->bucket_list);
    s_times_op->bucket_list = NULL;

    delete[] s_times_op->contents;

    s_times_op->semantic_type = SEM_TYP_TABULATION;

    char *obj_name = "matrix";
    size_t zln = strlen(obj_name);
    char *tmp = new char[zln + 1];
    strcpy(tmp, obj_name);
    s_times_op->contents = tmp;

    s_times_op->bucket_list = cells1;
    s_times_op->bucket_list =
      AppendBucketRec(s_times_op->bucket_list, cells2);

    s_times_op->ncols = n_cols;
    s_times_op->nrows = rows1 + rows2;
  } else {
    TCI_ASSERT(0);
  }
}

void CompEngine::ExtractCells(SEMANTICS_NODE * s_times_op,
                              BUCKET_REC ** bucket_lists, U32 n_rows)
{
  BUCKET_REC *b_rover = s_times_op->bucket_list;
  while (b_rover) {
    SEMANTICS_NODE *s_matrix = b_rover->first_child;
    if (s_matrix->semantic_type == SEM_TYP_BRACKETED_LIST
        || s_matrix->semantic_type == SEM_TYP_PARENED_LIST) {
      BUCKET_REC *bucket = s_matrix->bucket_list;
      s_matrix = bucket->first_child;
    }

    if (s_matrix->semantic_type == SEM_TYP_TABULATION) {
      U32 n_cols = s_matrix->ncols;
      BUCKET_REC *c_rover = s_matrix->bucket_list;
      s_matrix->bucket_list = NULL;
      U32 i = 0;
      while (i < n_rows) {
        bucket_lists[i] = AppendBucketRec(bucket_lists[i], c_rover);
        U32 j = 0;
        BUCKET_REC *prev = NULL;
        while (j < n_cols) {
          prev = c_rover;
          c_rover = c_rover->next;
          j++;
        }
        if (prev)
          prev->next = NULL;
        i++;
      }
    } else if (s_matrix->semantic_type == SEM_TYP_INFIX_OP
               && IsSeparatorOp(s_matrix)) {
      ExtractCells(s_matrix, bucket_lists, n_rows);
    }
    b_rover = b_rover->next;
  }
}

SEMANTICS_NODE *CompEngine::LocateMatrixInBucket(BUCKET_REC * bucket)
{
  SEMANTICS_NODE *rv = NULL;

  if (bucket) {
    SEMANTICS_NODE *s_matrix = bucket->first_child;
    if (s_matrix->semantic_type == SEM_TYP_BRACKETED_LIST
        || s_matrix->semantic_type == SEM_TYP_PARENED_LIST) {
      BUCKET_REC *bucket1 = s_matrix->bucket_list;
      s_matrix = bucket1->first_child;
    }

    if (s_matrix->semantic_type == SEM_TYP_TABULATION)
      rv = s_matrix;
    else
      TCI_ASSERT(0);
  }

  return rv;
}

bool CompEngine::OKtoConcat(SEMANTICS_NODE * s_times_op, int & n_rows)
{
  bool rv = true;

  n_rows = 0;
  if (s_times_op
      && s_times_op->bucket_list
      && s_times_op->semantic_type == SEM_TYP_INFIX_OP
      && IsSeparatorOp(s_times_op)) {
    BUCKET_REC *b_rover = s_times_op->bucket_list;
    while (rv && b_rover) {
      SEMANTICS_NODE *s_matrix = b_rover->first_child;

      int loop_n_rows = 0;
      if (s_matrix->semantic_type == SEM_TYP_BRACKETED_LIST
          || s_matrix->semantic_type == SEM_TYP_PARENED_LIST) {
        BUCKET_REC *bucket = s_matrix->bucket_list;
        s_matrix = bucket->first_child;
      }
      if (s_matrix->semantic_type == SEM_TYP_TABULATION) {
        loop_n_rows = s_matrix->nrows;
      } else if (s_matrix->semantic_type == SEM_TYP_INFIX_OP
                 && IsSeparatorOp(s_matrix)) {
        if (!OKtoConcat(s_matrix, loop_n_rows))
          rv = false;
      } else
        rv = false;

      if (rv) {
        if (!n_rows)
          n_rows = loop_n_rows;
        else if (n_rows != loop_n_rows)
          rv = false;
      }
      b_rover = b_rover->next;
    }
  } else {
    rv = false;
  }
  return rv;
}

bool CompEngine::OKtoStack(SEMANTICS_NODE * s_times_op, int & n_cols)
{
  bool rv = true;

  n_cols = 0;
  if (s_times_op
      && s_times_op->bucket_list
      && s_times_op->semantic_type == SEM_TYP_INFIX_OP
      && IsSeparatorOp(s_times_op)) {
    BUCKET_REC *b_rover = s_times_op->bucket_list;
    while (rv && b_rover) {
      SEMANTICS_NODE *s_matrix = b_rover->first_child;

      int loop_n_cols = 0;
      if (s_matrix->semantic_type == SEM_TYP_BRACKETED_LIST
          || s_matrix->semantic_type == SEM_TYP_PARENED_LIST) {
        BUCKET_REC *bucket = s_matrix->bucket_list;
        s_matrix = bucket->first_child;
      }

      if (s_matrix->semantic_type == SEM_TYP_TABULATION) {
        loop_n_cols = s_matrix->ncols;
      } else if (s_matrix->semantic_type == SEM_TYP_INFIX_OP
                 && IsSeparatorOp(s_matrix)) {
        if (!OKtoStack(s_matrix, loop_n_cols))
          rv = false;
      } else {
        rv = false;
      }
      if (rv) {
        if (!n_cols)
          n_cols = loop_n_cols;
        else if (n_cols != loop_n_cols)
          rv = false;
      }
      b_rover = b_rover->next;
    }
  } else {
    rv = false;
  }
  return rv;
}

// s_tree is the semantic tree for one line of an ODE system
// I'm looking for    {y(0)}=1  OR {{y'(0)}=1}

bool CompEngine::IsBoundaryCondition(SEMANTICS_NODE * s_tree)
{
  bool rv = false;

  while (s_tree->semantic_type == SEM_TYP_PRECEDENCE_GROUP) {
    BUCKET_REC *bl = s_tree->bucket_list;
    if (bl && bl->first_child) {
      SEMANTICS_NODE *s_node = bl->first_child;
      s_tree = s_node;
    } else
      TCI_ASSERT(0);
  }

  if (s_tree->semantic_type == SEM_TYP_INFIX_OP) {

    if (s_tree->contents && !strcmp(s_tree->contents, "=")) {
      BUCKET_REC* bl_operands = s_tree->bucket_list;
      if (bl_operands && bl_operands->bucket_ID == MB_UNNAMED) {
        SEMANTICS_NODE* s_func = bl_operands->first_child;

        if (s_func->semantic_type == SEM_TYP_INFIX_OP) {
		   if (StringEqual(s_func->contents, "&#x2062;")){ // invisible times
		      //delete[] s_func -> contents;
		      //s_func->contents	= DuplicateString("&#x2061;"); // function app

		      BUCKET_REC* bl = s_func->bucket_list;
              if (bl && bl->first_child) {
                 s_func = bl->first_child;
				 
              } else
                 TCI_ASSERT(0);

		   } 
        }

        if (s_func->semantic_type == SEM_TYP_PRECEDENCE_GROUP) {
          BUCKET_REC *bl = s_func->bucket_list;
          if (bl && bl->first_child)
            s_func = bl->first_child;
          else
            TCI_ASSERT(0);
        }

        if (s_func->semantic_type == SEM_TYP_DERIVATIVE) {
          BUCKET_REC *b = FindBucketRec(s_func->bucket_list, MB_UNNAMED);
          if (b)
            s_func = b->first_child;
          else
            TCI_ASSERT(0);
        }

        if (s_func->semantic_type == SEM_TYP_FUNCTION) {
          BUCKET_REC* arg_b = s_func->bucket_list;
          if (arg_b && arg_b->bucket_ID == MB_UNNAMED && !arg_b->next) {
            SEMANTICS_NODE* s_num = arg_b->first_child;
            if (s_num->semantic_type == SEM_TYP_NUMBER)
              rv = true;
          }
        }                       // SEM_TYP_FUNCTION
      }
    }
  }

  return rv;
}

void CompEngine::GetODEFuncNames(SEMANTICS_NODE * s_line, char *zODE_names)
{
}

bool CompEngine::LoadEngWrapperDLL(const char *path)
{
  nsresult rv;
  wrapper = do_GetService(path, &rv);

  return NS_SUCCEEDED(rv);
}

// The underlying engine wrapper always records strings
//  going to and coming from the engine.  CompEngine
//  MUST retrieve this data - currently that's the only
//  way they get deleted.

void CompEngine::RetrieveEngineStrs(MathResult & mr)
{
  char *engstr;
  int engstr_ln;
  int res;
  nsresult rv = wrapper->GetEngStrPtr(1, &engstr, &engstr_ln, &res);
  if (NS_SUCCEEDED(rv) && res)
    mr.PutEngInStr(engstr, engstr_ln);

  rv = wrapper->GetEngStrPtr(2, &engstr, &engstr_ln, &res);
  if (NS_SUCCEEDED(rv) && res)
    mr.PutEngOutStr(engstr, engstr_ln);

  rv = wrapper->GetEngStrPtr(3, &engstr, &engstr_ln, &res);
  if (NS_SUCCEEDED(rv) && res)
    mr.PutEngErrorStr(engstr, engstr_ln);
}

void CompEngine::ClearEngineStrs()
{
  wrapper->ClearEngStrPtrs();
}

// Symbols that occur in a "definition" may occur in engine output
//  generated by future computations.  We must keep info required
//  to render such symbols.  
// Backmap records are copied from the current computation list, curr_IDs_2mml,
//  to a global permanent list, def_IDs_2mml.

void CompEngine::SaveBackMap(MIC2MMLNODE_REC * new_IDs)
{
  MIC2MMLNODE_REC *rover = new_IDs;
  while (rover) {
    char *obj_name = rover->canonical_name;
    U32 owner = rover->owner_ID;
    size_t zln = strlen(obj_name);

    bool do_it = true;
// obj_name may already be in the permanent list
    if (def_IDs_2mml) {
      MIC2MMLNODE_REC *rover1 = def_IDs_2mml;
      while (rover1) {
        if (rover1->canonical_name
            && rover1->owner_ID == owner
            && !strcmp(obj_name, rover1->canonical_name)) {
          do_it = false;
          break;
        }
        rover1 = rover1->next;
      }
    }

    if (do_it) {
      MIC2MMLNODE_REC *new_node = new MIC2MMLNODE_REC();
      new_node->next = NULL;
      new_node->owner_ID = rover->owner_ID;

      char *tmp = new char[zln + 1];
      strcpy(tmp, obj_name);
      new_node->canonical_name = tmp;

      if (rover->mml_markup) {
        size_t ln = strlen(rover->mml_markup);
        tmp = new char[ln + 1];
        strcpy(tmp, rover->mml_markup);
        new_node->mml_markup = tmp;
      } else
        TCI_ASSERT(0);

      // Append the new record to the global list
      if (!def_IDs_2mml)
        def_IDs_2mml = new_node;
      else {
        MIC2MMLNODE_REC *rover2 = def_IDs_2mml;
        while (rover2->next)
          rover2 = rover2->next;
        rover2->next = new_node;
      }
    }
    rover = rover->next;
  }
}

// SWP often accepts an "enclosed list" as input when a matrix or vector
//  is needed.

void CompEngine::ListToMatrix(BUCKET_REC * var_val_bucket)
{
  if (var_val_bucket && var_val_bucket->first_child) {
    SEMANTICS_NODE *cont = var_val_bucket->first_child;

    if (cont->semantic_type == SEM_TYP_BRACKETED_LIST
        || cont->semantic_type == SEM_TYP_PARENED_LIST
        || cont->semantic_type == SEM_TYP_SET) {

      int col_counter = 0;
      BUCKET_REC *b_rover = cont->bucket_list;
      while (b_rover) {
        col_counter++;
        b_rover = b_rover->next;
      }

      cont->semantic_type = SEM_TYP_TABULATION;
      cont->nrows = 1;
      cont->ncols = col_counter;

      delete[] cont->contents;

      char *obj_name = "matrix";
      size_t zln = strlen(obj_name);
      char *tmp = new char[zln + 1];
      strcpy(tmp, obj_name);
      cont->contents = tmp;
    }
  }
}

// SWP may accept a matrix as input when a "enclosed list"
//  is needed.

void CompEngine::MatrixToList(BUCKET_REC * math_bucket)
{
  if (math_bucket && math_bucket->first_child) {
    SEMANTICS_NODE *cont = math_bucket->first_child;
    if (cont->semantic_type == SEM_TYP_TABULATION) {
      cont->semantic_type = SEM_TYP_SET;
      cont->nrows = 0;
      cont->ncols = 0;
      delete[] cont->contents;
      cont->contents = NULL;
    }
  }
}

// Fix this!!

void CompEngine::GetBasisVariables(char *dest)
{
  if (dest)
    strcpy(dest, mml_VecBasisVars);
}

void CompEngine::AddBasisVariables(MathServiceRequest & msr)
{

  U32 p_type = zPT_ASCII_mmlmarkup;
  msr.PutParam(PID_VecBasisVariables, p_type, mml_VecBasisVars);

  MNODE *dMML_tree2 = mml_tree_gen->MMLstr2Tree(mml_VecBasisVars);
  MathResult mr;
  INPUT_NOTATION_REC *p_input_notation = NULL;
  SEMANTICS_NODE *semantics_tree =
    semantic_analyzer->BuildSemanticsTree(msr, mr, mml_VecBasisVars,
                                          dMML_tree2, CCID_Evaluate,
                                          p_input_notation);
  MIC2MMLNODE_REC *map = semantic_analyzer->GetBackMap();
  DisposeIDsList(map);
  DisposeTList(dMML_tree2);

  msr.AddSemanticsToParam(PID_VecBasisVariables, semantics_tree);
  // semantics_tree now owned by msr - disposed by msr dtor
}

// Search a semantics tree for "Solve Recursion".  Locate an instance
// of the recursive function, and append a node that gives the function name.

void CompEngine::AddRecurFuncNode(SEMANTICS_NODE* s_recur, BUCKET_REC* parent_b)
{

  if (s_recur && s_recur->bucket_list && s_recur->bucket_list->first_child) {
    SEMANTICS_NODE *s_equal = s_recur->bucket_list->first_child;

  // The following call may not locate the function call that we want.
  //  It just finds the first call.  Might add more here to insure
  //  that the recursion function is found.
    SEMANTICS_NODE *s_func = LocateRecurFuncInEqn(s_equal);
    if (s_func) {
      SEMANTICS_NODE *s_clone = CreateSemanticsNode(SEM_TYP_FUNCTION);
      //s_clone->semantic_type = SEM_TYP_FUNCTION;
      parent_b->first_child = s_clone;
      s_clone->parent = parent_b;

  //  FUNCTION canonical_ID = "miy" contents = "y"
      size_t zln = strlen(s_func->canonical_ID);
      char *tmp = new char[zln + 1];
      strcpy(tmp, s_func->canonical_ID);
      s_clone->canonical_ID = tmp;

      char *func_nom = s_func->canonical_ID;
      if (s_func->contents)
        func_nom = s_func->contents;

      zln = strlen(func_nom);
      tmp = new char[zln + 1];
      strcpy(tmp, func_nom);
      s_clone->contents = tmp;
    } else
      TCI_ASSERT(0);
  } else
    TCI_ASSERT(0);
}

// Traverse a semantic tree, return a pointer to the first function found
// Might amend this to return first function found twice.

SEMANTICS_NODE *CompEngine::LocateRecurFuncInEqn(SEMANTICS_NODE * s_tree)
{
  SEMANTICS_NODE *rv = NULL;

  if (s_tree) {
    if (s_tree->semantic_type == SEM_TYP_FUNCTION) {
      rv = s_tree;
    } else if (s_tree->bucket_list) {
      BUCKET_REC *b_r = s_tree->bucket_list;
      while (b_r && !rv) {
        if (b_r->first_child) {
          SEMANTICS_NODE *s_rover = b_r->first_child;
          while (s_rover) {
            if (s_rover->semantic_type == SEM_TYP_FUNCTION) {
              rv = s_rover;
              break;
            } else if (s_rover->bucket_list) {
              rv = LocateRecurFuncInEqn(s_rover);
              if (rv)
                break;
            }
            s_rover = s_rover->next;
          }
        }
        b_r = b_r->next;
      }
    }
  }

  return rv;
}

bool IsCommaList(SEMANTICS_NODE* s_node)
{
  bool rv = false;

  if (s_node
      && !s_node->next
      && s_node->semantic_type == SEM_TYP_INFIX_OP
      && s_node->contents && !strcmp(s_node->contents, ",")
      && s_node->bucket_list && s_node->bucket_list->next) {
    
    BUCKET_REC* b_left = s_node->bucket_list;
    BUCKET_REC* b_right = b_left->next;

    SEMANTICS_NODE* s_left = b_left->first_child;
    SEMANTICS_NODE* s_right = b_right->first_child;
    if (s_left && s_right) {
      if (!s_left->next
          && s_left->semantic_type == SEM_TYP_INFIX_OP
          && s_left->contents && !strcmp(s_left->contents, ",")
          && s_left->bucket_list && s_left->bucket_list->next)
        rv = IsCommaList(s_left);
      else
        rv = true;
    }
  }

  return rv;
}

// tree manipulation - the s_node tree is reduced to a bucket list here.

BUCKET_REC *CompEngine::ExtractCommaList(SEMANTICS_NODE * s_node,
                                         BUCKET_REC * b_list)
{
  BUCKET_REC *rv = b_list;

  if (s_node
      && !s_node->next
      && s_node->semantic_type == SEM_TYP_INFIX_OP
      && s_node->contents && !strcmp(s_node->contents, ",")
      && s_node->bucket_list && s_node->bucket_list->next) {
    BUCKET_REC *b_left = s_node->bucket_list;
    BUCKET_REC *b_right = b_left->next;
    b_left->next = NULL;

    SEMANTICS_NODE *s_left = b_left->first_child;
    SEMANTICS_NODE *s_right = b_right->first_child;
    if (s_left) {
      if (!s_left->next
          && s_left->semantic_type == SEM_TYP_INFIX_OP
          && s_left->contents && !strcmp(s_left->contents, ",")
          && s_left->bucket_list && s_left->bucket_list->next) {
        rv = ExtractCommaList(s_left, rv);
        b_left->first_child = NULL;
      } else {
        rv = AppendBucketRec(rv, b_left);
        s_node->bucket_list = NULL;
      }
    }
    if (s_right)
      rv = AppendBucketRec(rv, b_right);
    else
      TCI_ASSERT(0);

    DisposeSList(s_node);
  }

  return rv;
}

// Input data validation

bool CompEngine::IsNumericFrac(SEMANTICS_NODE * semantics_tree)
{
  bool rv = false;

  if (semantics_tree && semantics_tree->bucket_list) {
    // Descend into the meat
    BUCKET_REC *b_rover = semantics_tree->bucket_list;
    if (b_rover->first_child && b_rover->next == NULL) {
      SEMANTICS_NODE *s_child = b_rover->first_child;
      while (s_child->semantic_type == SEM_TYP_PRECEDENCE_GROUP) {
        BUCKET_REC *b = s_child->bucket_list;
        if (b->first_child && b->next == NULL)
          s_child = b->first_child;
        else
          return rv;
      }

      if (s_child->semantic_type == SEM_TYP_FRACTION
          && s_child->bucket_list) {
        BUCKET_REC *num = FindBucketRec(s_child->bucket_list, MB_NUMERATOR);
        BUCKET_REC *den = FindBucketRec(s_child->bucket_list, MB_DENOMINATOR);
        if (num && den) {
          SEMANTICS_NODE *s_num = num->first_child;
          SEMANTICS_NODE *s_den = den->first_child;
          if (s_num
              && s_num->semantic_type == SEM_TYP_NUMBER
              && s_num->next == NULL
              && s_den
              && s_den->semantic_type == SEM_TYP_NUMBER
              && s_den->next == NULL) {
            if (AllDigits(s_num->contents) &&
                AllDigits(s_den->contents))
              rv = true;
          }
        }
      }
    }
  } else {
    TCI_ASSERT(0);
  }
  return rv;
}

bool CompEngine::AllDigits(char *z_data)
{
  int n_bytes = 0;
  int n_digits = 0;

  char *p = z_data;
  while (*p) {
    n_bytes++;
    if (*p >= '0' && *p <= '9')
      n_digits++;
    p++;
  }

  if (n_digits && (n_digits == n_bytes))
    return true;
  else
    return false;
}

bool CompEngine::IsMOD(SEMANTICS_NODE * semantics_tree)
{
  bool rv = false;

  if (semantics_tree && semantics_tree->bucket_list) {
    BUCKET_REC *b_rover = semantics_tree->bucket_list;
    if (b_rover->first_child && b_rover->next == NULL) {
      SEMANTICS_NODE *s_child = b_rover->first_child;
      while (s_child->semantic_type == SEM_TYP_PRECEDENCE_GROUP) {
        BUCKET_REC *b = s_child->bucket_list;
        if (b->first_child && b->next == NULL)
          s_child = b->first_child;
        else
          return rv;
      }

      if (s_child->semantic_type == SEM_TYP_INFIX_OP
          && s_child->bucket_list
          && s_child->contents && !strcmp(s_child->contents, "mod")) {
        rv = true;
      }
    }
  }

  return rv;
}

// When a variable or function is "Undefined", we can remove it
//  from the back-mapping list "def_IDs_2mml".  Notice that
//  defs are associated with computation clients.

void CompEngine::RemoveBackMapEntry(U32 curr_client_ID,
                                    const char *targ_canon_ID)
{
  MIC2MMLNODE_REC *head = NULL;
  MIC2MMLNODE_REC *tail;

  MIC2MMLNODE_REC *rover = def_IDs_2mml;
  while (rover) {
    MIC2MMLNODE_REC *curr = rover;
    rover = rover->next;
    if (curr->owner_ID == curr_client_ID
        && curr->canonical_name
        && !strcmp(curr->canonical_name, targ_canon_ID)) {
      delete[] curr->canonical_name;
      delete[] curr->mml_markup;
      delete curr;
    } else {
      if (!head)
        head = curr;
      else
        tail->next = curr;

      tail = curr;
      tail->next = NULL;
    }
  }

  def_IDs_2mml = head;
}

U32 CompEngine::GetDefType(SEMANTICS_NODE * semantics_tree,
                           char **arg_list, U32 & n_subscripted_args)
{
  U32 rv = DT_NONE;
  *arg_list = NULL;
  n_subscripted_args = 0;

  if (semantics_tree
      && semantics_tree->bucket_list
      && semantics_tree->bucket_list->first_child) {
    BUCKET_REC *outer_math_bucket = semantics_tree->bucket_list;
    SEMANTICS_NODE *s_child = outer_math_bucket->first_child;

    if (s_child->semantic_type == SEM_TYP_VARDEF ||
        s_child->semantic_type == SEM_TYP_VARIABLE ||
        s_child->semantic_type == SEM_TYP_VARDEF_DEFERRED) {
      rv = DT_VARIABLE;
    } else if (s_child->semantic_type == SEM_TYP_FUNCDEF ||
               s_child->semantic_type == SEM_TYP_FUNCTION) {
      rv = DT_FUNCTION;

      BUCKET_REC *header_bucket = FindBucketRec(s_child->bucket_list,
                                                MB_DEF_FUNCHEADER);
      if (header_bucket && header_bucket->first_child) {
        SEMANTICS_NODE *s_header = header_bucket->first_child;
        if (s_header->semantic_type == SEM_TYP_PRECEDENCE_GROUP) {
          BUCKET_REC *bucket = s_header->bucket_list;
          s_header = bucket->first_child;
        }
        n_subscripted_args = s_header->n_sub_args;

        char *tmp = NULL;
        U32 tln = 0;

        int tally = 0;
        BUCKET_REC *arg_rover = s_header->bucket_list;
        while (arg_rover) {
          SEMANTICS_NODE *s_arg = arg_rover->first_child;
          if (s_arg && s_arg->contents && s_arg->canonical_ID) {
            if (tally)
              tmp = AppendStr2HeapStr(tmp, tln, ",");
            tmp = AppendStr2HeapStr(tmp, tln, s_arg->contents);
            tmp = AppendStr2HeapStr(tmp, tln, ",");
            tmp = AppendStr2HeapStr(tmp, tln, s_arg->canonical_ID);
            tally++;
          }
          arg_rover = arg_rover->next;
        }

        if (tmp && *tmp) {
          size_t zln = strlen(tmp);
          char *t1 = new char[zln + 1];
          strcpy(t1, tmp);
          *arg_list = t1;
          delete[] tmp;
        }
      }
    } else {
      TCI_ASSERT(!"unexpected semantic_type");
    }
  } else {
    TCI_ASSERT(0);
  }
  return rv;
}

void CompEngine::RecordEngineAttr(int targ_ID, const char *new_value)
{
  //#define EID_SeriesOrder         1
  //#define EID_Digits              2
  //#define EID_MaxDegree           3
  //#define EID_PvalOnly            4
  //#define EID_IgnoreSCases        5
  //#define EID_FloatFormat         6
  //#define EID_VarType             7

  bool done = false;

  ENG_ATTR_REC *a_rover = engine_attrs;
  while (a_rover) {
    if (a_rover->ID == targ_ID) {
      if (a_rover->value) {
        delete[] a_rover->value;
        a_rover->value = NULL;
        if (new_value) {
          size_t zln = strlen(new_value);
          char *tmp = new char[zln + 1];
          strcpy(tmp, new_value);
          a_rover->value = tmp;
        }
      }
      done = true;
      break;
    }
    a_rover = a_rover->next;
  }

  if (!done) {
    ENG_ATTR_REC *new_rec = new ENG_ATTR_REC();
    new_rec->next = engine_attrs;
    if (new_value) {
      size_t zln = strlen(new_value);
      char *tmp = new char[zln + 1];
      strcpy(tmp, new_value);
      new_rec->value = tmp;
    } else {
      new_rec->value = NULL;
    }
    new_rec->ID = targ_ID;
    engine_attrs = new_rec;
  }
}

void CompEngine::FixiArgument(BUCKET_REC * header_bucket)
{
  if (header_bucket && header_bucket->first_child) {
    SEMANTICS_NODE *f_header = header_bucket->first_child;

    bool i_is_arg = false;
    BUCKET_REC *arg_rover = f_header->bucket_list;
    while (arg_rover) {
      SEMANTICS_NODE *s_arg = arg_rover->first_child;
      if (s_arg
          && s_arg->semantic_type == SEM_TYP_UCONSTANT
          && s_arg->canonical_ID
          && !strcmp(s_arg->canonical_ID, "mii")) {
        s_arg->semantic_type = SEM_TYP_VARIABLE;
        i_is_arg = true;
      }
      arg_rover = arg_rover->next;
    }

    if (i_is_arg) {
      if (header_bucket->next) {
        BUCKET_REC *func_val_bucket = header_bucket->next;
        SEMANTICS_NODE *right_side = func_val_bucket->first_child;
        ImagineiToVari(right_side);
      } else
        TCI_ASSERT(0);
    }
  }
}

void CompEngine::ImagineiToVari(SEMANTICS_NODE * s_list)
{
  SEMANTICS_NODE *s_rover = s_list;
  while (s_rover) {
    if (s_rover->semantic_type == SEM_TYP_UCONSTANT) {
      if (s_rover->canonical_ID
          && !strcmp(s_rover->canonical_ID, "mii"))
        s_rover->semantic_type = SEM_TYP_VARIABLE;
    } else if (s_rover->bucket_list) {
      BUCKET_REC *b_rover = s_rover->bucket_list;
      while (b_rover) {
        SEMANTICS_NODE *s_node = b_rover->first_child;
        if (s_node)
          ImagineiToVari(s_node);
        b_rover = b_rover->next;
      }
    }
    s_rover = s_rover->next;
  }
}

void CompEngine::FixIndexedVars(SEMANTICS_NODE * s_list,
                                BUCKET_REC * arg_bucket_list)
{
  SEMANTICS_NODE* s_rover = s_list;
  while (s_rover) {
    if (s_rover->semantic_type == SEM_TYP_QUALIFIED_VAR) {

      FixIndexedVarsOfQualifiedVar(s_rover, arg_bucket_list);
      
    } else if (s_rover->bucket_list) {

      BUCKET_REC *b_rover = s_rover->bucket_list;
      while (b_rover) {
        if (b_rover->first_child)
          FixIndexedVars(b_rover->first_child, arg_bucket_list);
        b_rover = b_rover->next;
      }

    }
    s_rover = s_rover->next;
  }
}


void FixIndexedVarsOfQualifiedVar(SEMANTICS_NODE* qv, BUCKET_REC* arg_bucket_list)
{
  if (qv->bucket_list) {
     
     BUCKET_REC* b = FindBucketRec(qv->bucket_list, MB_SUB_QUALIFIER);
     if (b && b->first_child) {
        SEMANTICS_NODE* q_subscript = b->first_child;
        if (q_subscript->semantic_type == SEM_TYP_VARIABLE) {
            if (q_subscript->canonical_ID) {
              if (IdIsFuncArg(q_subscript->canonical_ID, arg_bucket_list))
                qv->semantic_type = SEM_TYP_INDEXED_VAR;
            }
        } else if (IsCommaList(q_subscript)) {
            // I'm only checking the first var in the list - might check all.
            if (q_subscript->bucket_list && q_subscript->bucket_list->first_child) {
              SEMANTICS_NODE* i_var = q_subscript->bucket_list->first_child;
              if (i_var->canonical_ID) {
                if (IdIsFuncArg(i_var->canonical_ID, arg_bucket_list))
                  qv->semantic_type = SEM_TYP_INDEXED_VAR;
              }
            }
        } else if (ExprContainsFuncArg(q_subscript, arg_bucket_list) ){
           qv->semantic_type = SEM_TYP_INDEXED_VAR;
        }
     }
  }
}

bool IdIsFuncArg(char* canonical_ID, BUCKET_REC* arg_bucket_list)
{
  bool rv = false;

  BUCKET_REC* arg_rover = arg_bucket_list;
  while (arg_rover) {
    SEMANTICS_NODE* s_arg = arg_rover->first_child;
    if (s_arg && 
         (s_arg->semantic_type == SEM_TYP_VARIABLE) &&
         s_arg->canonical_ID &&
         !strcmp(s_arg->canonical_ID, canonical_ID) ) {
      return true;
    }    
    arg_rover = arg_rover->next;
  }

  return false;
}

bool ExprContainsFuncArg(SEMANTICS_NODE* expr, BUCKET_REC* args)
{
  if (SEM_TYP_VARIABLE  ==  expr -> semantic_type ){
     // Need to look through list of args
     return true;
  } else {
     // Look through bucket list
     BUCKET_REC* b = expr->bucket_list;

     while (b) {
        SEMANTICS_NODE* s = b->first_child;
        if (ExprContainsFuncArg(s, args))
          return true;
        
        b = b -> next;
     }
     return false;
  }
}

bool CompEngine::DefAllowed(SEMANTICS_NODE * root)
{
  bool rv = true;

  SEMANTICS_NODE *child = NULL;
  if (root && root->bucket_list && root->bucket_list->first_child)
    child = root->bucket_list->first_child;

  if (child && child->semantic_type == SEM_TYP_FUNCDEF) {
    BUCKET_REC *header_bucket =
      FindBucketRec(child->bucket_list, MB_DEF_FUNCHEADER);
    if (header_bucket && header_bucket->first_child) {
      SEMANTICS_NODE *s_func = header_bucket->first_child;
      if (s_func->semantic_type == SEM_TYP_PRECEDENCE_GROUP) {
        BUCKET_REC *bucket = s_func->bucket_list;
        s_func = bucket->first_child;
      }
      char *key = s_func->contents;
      const char *db_production = NULL;
      U32 ID, subID;
      if (GetdBaseNamedRecord("FUNCTIONS", key, &db_production, ID, subID)) {
        rv = false;
      }
    } else {
      TCI_ASSERT(0);
    }
  }

  return rv;
}

// Interesting function - we ask the underlying engine
//  to evaluate a param.

int CompEngine::GetNColumns(MathServiceRequest & msr, int& error_code)
{
  int rv = 0;
  error_code = 0;

  U32 p_type;
  const char *p_str = msr.GetParam(PID_ncolsReshape, p_type);
  if (p_type == zPT_ASCII_natural) {
    rv = atoi(p_str);
  } else if (p_type == zPT_ASCII_mmlmarkup) {
    MathServiceRequest msr_tmp;
    MathResult mr_tmp;

    msr_tmp.PutClientHandle(msr.GetClientHandle());
    msr_tmp.PutEngineID(msr.GetEngineID());
    msr_tmp.PutDefStore(msr.GetDefStore());
    msr_tmp.PutOpID(CCID_Evaluate_Numerically);

    MNODE *dMML_tree2 = mml_tree_gen->MMLstr2Tree(p_str);

    INPUT_NOTATION_REC *p_input_notation = NULL;
    SEMANTICS_NODE *semantics_tree =
      semantic_analyzer->BuildSemanticsTree(msr_tmp, mr_tmp,
                                            p_str, dMML_tree2,
                                            CCID_Evaluate_Numerically,
                                            p_input_notation);
    curr_IDs_2mml =
      JoinIDsLists(curr_IDs_2mml, semantic_analyzer->GetBackMap());

    mr_tmp.PutSemanticsTree(semantics_tree);
    void * tmp;
    nsresult res = wrapper->ProcessRequest((void *)&msr_tmp, (void *)&mr_tmp, &tmp);
    SEMANTICS_NODE *res_tree = NS_SUCCEEDED(res) ? (SEMANTICS_NODE *) tmp : 0;
    RetrieveEngineStrs(mr_tmp);

    int result_code = mr_tmp.GetResultCode();
    if (result_code >= CR_success) {
      // res_tree should consist of a single node - a number
      if (res_tree && res_tree->semantic_type == SEM_TYP_NUMBER) {
        // Note that we may have "4.59" here.
        // The atoi call will truncate.
        rv = atoi(res_tree->contents);
      } else {
        TCI_ASSERT(0);
      }
    }

    DisposeTList(dMML_tree2);
    delete p_input_notation;
    wrapper->DisposeSList(res_tree);
  } else
    TCI_ASSERT(0);

  // rv is the number of columns in the reshaped matrix,
  //  it must be a positive integer.

  if (!rv)
    error_code = PID_first_badparam + PID_ncolsReshape;

  return rv;
}

bool CompEngine::IsSeparatorOp(SEMANTICS_NODE * s_node)
{
  bool rv = false;
  if (s_node && s_node->contents) {
    if (!strcmp(s_node->contents, "&#x2062;") // invisible times
        || !strcmp(s_node->contents, "&#x2063;")  // invisible comma
        || !strcmp(s_node->contents, ","))
      rv = true;
  }

  return rv;
}

SEMANTICS_NODE *CompEngine::InsertPGroup(SEMANTICS_NODE * s_math)
{
  SEMANTICS_NODE *s_pgroup = NULL;

  if (s_math) {
    s_pgroup = CreateSemanticsNode(SEM_TYP_PRECEDENCE_GROUP);
    //s_pgroup->semantic_type = SEM_TYP_PRECEDENCE_GROUP;
    if (s_math->bucket_list) {
      s_pgroup->bucket_list = s_math->bucket_list;
      s_math->bucket_list = NULL;
    }

    BUCKET_REC* b = MakeParentBucketRec(MB_UNNAMED, s_pgroup);
    
    s_math->bucket_list = b;
  }

  return s_pgroup;
}

void CompEngine::SetInfixPrecedences(SEMANTICS_NODE * semantics_list)
{
  SEMANTICS_NODE *s_rover = semantics_list;
  while (s_rover) {
    if (s_rover->semantic_type == SEM_TYP_INFIX_OP) {
      SetInfixPrecedence(s_rover);
    } else if (s_rover->bucket_list) {
      BUCKET_REC *b_rover = s_rover->bucket_list;
      while (b_rover) {
        if (b_rover->first_child)
          SetInfixPrecedences(b_rover->first_child);
        b_rover = b_rover->next;
      }
    }
    s_rover = s_rover->next;
  }
}

// Analyzer generates a SEM_TYP_QUALIFIED_VAR from f_{n}.
// During a Define command, we ask the user if the sub is part
//  of the name or a function argument.  If it's an argument,
//  we convert the SEM_TYP_QUALIFIED_VAR to SEM_TYP_FUNCTION here.

const char *CompEngine::QVarToFuncHeader(SEMANTICS_NODE * s_qvar)
{
  char *rv = NULL;

  if (s_qvar && s_qvar->bucket_list) {
    BUCKET_REC *b_args_list = NULL; // for the args in the call we're creating

    BUCKET_REC *b_rover = s_qvar->bucket_list;
    while (b_rover) {           // loop thru components of s_qvar

      SEMANTICS_NODE *s_node = b_rover->first_child;
      if (s_node) {
        if (b_rover->bucket_ID == MB_BASE_VARIABLE) {
          // convert s_qvar to a function call
          s_qvar->semantic_type = SEM_TYP_FUNCTION;
          delete s_qvar->canonical_ID;
          rv = s_node->canonical_ID;
          s_qvar->canonical_ID = rv;
          s_node->canonical_ID = NULL;

          delete s_qvar->contents;
          s_qvar->contents = s_node->contents;
          s_node->contents = NULL;
        } else if (b_rover->bucket_ID == MB_SUB_QUALIFIER) {
          // construct an args list from the qualifiers

          int n_args = 0;
          if (IsVariableInSubscript(s_node)) {
            s_qvar->n_sub_args = 1;
            b_args_list = MakeParentBucketRec(MB_UNNAMED, s_node);
            
            b_rover->first_child = NULL;
          } else if (IsSubVariableList(s_node, n_args)) {
            s_qvar->n_sub_args = n_args;
            b_args_list = ExtractCommaList(s_node, NULL);
            b_rover->first_child = NULL;
          } else
            TCI_ASSERT(0);
        } else
          TCI_ASSERT(0);
      } else
        TCI_ASSERT(0);
      b_rover = b_rover->next;
    }                           // loop thru components

    DisposeBucketList(s_qvar->bucket_list);

    s_qvar->bucket_list = b_args_list;
  } else
    TCI_ASSERT(0);

  return rv;
}

bool CompEngine::IsSubVariableList(SEMANTICS_NODE * s_node, int & n_args)
{
  bool rv = false;

  if (s_node
      && !s_node->next
      && s_node->semantic_type == SEM_TYP_INFIX_OP
      && s_node->contents && !strcmp(s_node->contents, ",")
      && s_node->bucket_list && s_node->bucket_list->next) {
    BUCKET_REC *b_left = s_node->bucket_list;
    BUCKET_REC *b_right = b_left->next;

    SEMANTICS_NODE *s_left = b_left->first_child;
    SEMANTICS_NODE *s_right = b_right->first_child;
    if (s_left && s_right) {
      if (!s_left->next
          && s_left->semantic_type == SEM_TYP_INFIX_OP
          && s_left->contents && !strcmp(s_left->contents, ",")
          && s_left->bucket_list && s_left->bucket_list->next) {

        if (IsVariableInSubscript(s_right)) {
          n_args++;
          rv = IsSubVariableList(s_left, n_args);
        }
      } else {
        if (IsVariableInSubscript(s_left)
            && IsVariableInSubscript(s_right)) {
          n_args += 2;
          rv = true;
        }
      }
    }
  }

  return rv;
}

bool CompEngine::IsVariableInSubscript(SEMANTICS_NODE * s_node)
{
  bool rv = false;

  if (s_node) {
    if (s_node->semantic_type == SEM_TYP_VARIABLE)
      rv = true;
    else if (s_node->semantic_type == SEM_TYP_UCONSTANT) {
      if (s_node->canonical_ID) {
        if (!strcmp(s_node->canonical_ID, "mii")
            || !strcmp(s_node->canonical_ID, "mie"))
          rv = true;
      } else
        TCI_ASSERT(0);
    }
  }

  return rv;
}
