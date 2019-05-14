// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

/*
  For development purposes, these public functions are implemented
  by constructing an SWP "MuPADEngine" object and making calls to it.
*/

#ifdef XP_WIN
#define WIN32_LEAN_AND_MEAN     // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#endif
#ifdef XP_UNIX
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

#include "MuWrap.h"
#include "MuWrapPlotCmd.h"
#include "EngWrap.h"
#include "mupd_eng.h"
#include <tcistrin.h>
#include "MRequest.h"
#include "MResult.h"
#include "DefStore.h"
#include "MuP2Tree.h"
#include "MuAnalyzer.h"
#include "Grammar.h"
#include "CmpTypes.h"
#include "fltutils.h"
#include "mnode.h"

#include "xpcom-config.h"
#include "nsString.h"
#include "nsILocalFile.h"
#include "nsComponentManagerUtils.h"
#include "strutils.h"
#include "DefInfo.h"

MuPADEngine* mupd_eng = NULL;


MuPADWrapper::MuPADWrapper(void* ID_dBase, void* NOM_dBase)
{
  id_dBase = (Grammar *) ID_dBase;
  nom_dBase = (Grammar *) NOM_dBase;

  mupd_eng = new MuPADEngine();

  client_list = NULL;

  MuPAD_parser = new MuPAD2Tree();
  out_tree = NULL;
  engstr_analyzer = new MuAnalyzer(nom_dBase);

  msg_list = NULL;
  curr_msr = NULL;

  context_nmatrices = 0;
  context_variables = NULL;

  ODE_num_procnum = 0;
  vcam_path = NULL;
}

MuPADWrapper::~MuPADWrapper()
{
  ClientAliases_REC *rover = client_list;
  while (rover) {
    ClientAliases_REC *del = rover;
    rover = rover->next;
    delete del;
  }

  delete[] vcam_path;
  delete mupd_eng;

  delete MuPAD_parser;
  DisposeTList(out_tree);
  delete engstr_analyzer;
  TCI_ASSERT(!msg_list);
  DisposeMsgs(msg_list);
  TCI_ASSERT(!context_variables);
  DisposeVarList(context_variables);
}

void MuPADWrapper::iReleaseClient(U32 client_ID, const char *def_list)
{
  const char *undefine_template = NULL;
  U32 cmd_ID, subID;
  if (GetdBaseNamedRecord
      ("COMMANDS", "Undefine", &undefine_template, cmd_ID, subID)) {
    // Undefine<uID136.0>delete %s;
    ClientAliases_REC *client_aliases = GetClientAliasesRec(client_ID);

    if (def_list) {
      size_t zln = strlen(def_list);
      char *tmp = new char[zln + 1];
      strcpy(tmp, def_list);
      char *ptr = tmp;
      while (ptr && *ptr) {
        char *p_name = ptr;
        char *p_comma = strchr(ptr, ',');
        if (p_comma) {
          *p_comma = 0;
          ptr = p_comma + 1;
        } else
          ptr = NULL;

        if (*p_name) {
          NAME2AKA_REC *curr =
            GetAKARec(client_aliases->AKAs_list, p_name, NULL);
          if (curr) {
            char cmd[256];
            sprintf(cmd, undefine_template, curr->alias);
            iExecute(cmd);
          } else {
            TCI_ASSERT(0);
          }
        }
      }
      delete[] tmp;
    }

    NAME2AKA_REC *rover = client_aliases->AKAs_list;
    while (rover) {
      NAME2AKA_REC *curr = rover;
      rover = rover->next;
#ifdef WHY_DID_JACK_DO_THIS
      if (IsDefined(curr->alias)) {
        char cmd[256];
        sprintf(cmd, undefine_template, curr->alias);
        int res = iExecute(cmd);
      }
#endif
      delete[] curr->alias;
      delete[] curr->src_canonical_name;
      delete[] curr->user_name;
      delete curr;
    }

    client_aliases->AKAs_list = NULL;
  } else {
    TCI_ASSERT(0);
  }
}

void MuPADWrapper::iCleanupAndUnloadDLL()
{
  ClientAliases_REC *rover = client_list;
  while (rover) {
    iReleaseClient(rover->client_ID, NULL);
    rover = rover->next;
  }
  mupd_eng->CleanupAndUnloadDLL();
}

void MuPADWrapper::iClearEngStrPtrs()
{
  mupd_eng->ClearEngStrPtrs();
}

SEMANTICS_NODE *MuPADWrapper::iProcessRequest(MathServiceRequest * msr,
                                              MathResult * mr)
{
  SEMANTICS_NODE *s_tree = NULL;
  SEMANTICS_NODE *root = mr->GetSemanticsTree();
  U32 cmd_ID = msr->GetOpID();

  if (!root &&  cmd_ID == CCID_PassThru){
    mr->PutResultCode(CR_failure);
    return NULL;
  }
  curr_msr = msr;
  const char *plotfilename  = NULL;

  // We need the ID of the client we're serving for possible name mangling
  client_ID = msr->GetClientHandle();

  if (!cmd_ID) {
    const char *cmd_template = NULL;
    U32 subID;
    GetdBaseNamedRecord("COMMANDS", msr->GetOpNamePtr(), &cmd_template,
                        cmd_ID, subID);
  }
  if (!cmd_ID) {
    TCI_ASSERT(0);
    return s_tree;
  }                                

  // The math content of engine strings depend on the command
  //  being processed.  We'll need compound dBase records
  //  or more dBase sections to support this.
  //  Evaluate an integral                      Evaluate an integral Numerically
  //Maple  eval(   tciint(q1x^(2),q1x,1,2)  );  evalf(  Int(q1x^(2),q1x=1..2)        );
  //MuPAD  eval(   tciint(q1x^(2),q1x,1,2)  );  float(  hold(int)(q1x^(2),q1x=1..2)  );
  SetWrapperCmdContext(cmd_ID, NULL);
  context_nmatrices = 0;
  SetVarTypes(root, context_nmatrices);

  TCI_ASSERT(!msg_list);
  DisposeMsgs(msg_list);
  msg_list = NULL;

  SEMANTICS_NODE *children = NULL;
  if (root && root->bucket_list && root->bucket_list->first_child)
    children = root->bucket_list->first_child;

  bool allowed = CmdAllowed(msr, mr, cmd_ID, children);
  if (allowed && CmdGoesToEngine(cmd_ID)) {
    int error_code = 0;
    char *eng_zstr = NULL;
    
    if (cmd_ID == CCID_PassThru){
      eng_zstr = new char[strlen(root -> contents)+1];
      strcpy(eng_zstr, root -> contents);
      //eng_zstr = root -> contents;
    }

    if (children) {
      // Traverse the math object (a semantic tree), 
      //  building an engine command string as we go.

      if (cmd_ID == CCID_Polynomial_Divide) {
        if (children->semantic_type == SEM_TYP_FRACTION &&
            !children->next) {
          eng_zstr =
            FRACTIONtoEstr(children, "Polynomial Divide", error_code);
        } else {
          TCI_ASSERT(0);
        }
      } else {
        eng_zstr = ProcessSemList(children, NULL, error_code);
      }
    }

    if (context_cmdtype == CC_PLOTFUNC) {
      U32 p_type;
      PlotServiceRequest *psr = static_cast<PlotServiceRequest*>(msr);
      plotfilename = psr->RetrievePlotParam(PID_GraphImageFile, p_type);
    }

    char *zEngine_input = NULL;
    
    if (!error_code) {
      zEngine_input =
        CreateEngineCommandStr(*msr, cmd_ID, eng_zstr, plotfilename, error_code);
    }

    if (zEngine_input && !error_code) {
      // Send our command string to the underlying engine
      if (context_cmdtype == CC_DEFINITION) {
        // might send tcikill() here
      }

      if (cmd_ID >= CCID_Solve_PDE && cmd_ID <= CCID_Solve_ODE_Series) {  // Solve DEs
        // Make sure that the independent variable is NOT defined in the engine
        KillIndVar(children);
      }

      char *zeng_output = Execute(zEngine_input);

      if (zeng_output || (context_cmdtype == CC_PLOTFUNC) && (context_cmdtemplatetype != CT_PLOTQUERY)) {
        size_t numBytes;
        if (zeng_output) numBytes = strlen(zeng_output);
        else numBytes = 0;
        
        // let plotqueries be handled by the standard code
        if ((context_cmdtype == CC_PLOTFUNC) && (context_cmdtemplatetype != CT_PLOTQUERY)) {
#ifndef XM_MACOSX
          // convert VCam to filename
          // BBM. The file is already a vcam file. We will simply copy it -- a better longterm 
          //   solution is for the engine to save it in the right place with the right name 
          // copy the file zeng_output to the file plotfilename
          nsCOMPtr<nsILocalFile> engOutput, plotfile;
          nsCOMPtr<nsIFile> destdirectory;
          nsAutoString strLeafName;
          nsresult rv;
          bool plotOK;
          engOutput = do_CreateInstance("@mozilla.org/file/local;1", &rv);
          plotOK = (rv == NS_OK);
          if (plotOK)
          {
#ifdef XP_WIN
              // We need to make sure the filename zeng_output is ok for InitWithNativePath() below. 
              // In particular, don't allow forward slash as in c:/temp\mx27.
              // This is preventing debugging on windows.
              for (int i = 0; i<numBytes; ++i){
                if (zeng_output[i] == L'/'){
                  zeng_output[i] = L'\\';
                }
              }
#endif

              printf("plotOK is true\n");
              plotfile = do_CreateInstance("@mozilla.org/file/local;1", &rv);
//              for (int i =0; i<100000;i++);
              if (zeng_output && *zeng_output)
              {
                rv = engOutput->InitWithNativePath( nsDependentCString(zeng_output) );
                if (rv) printf("engOutput->InitWithNativePath failed: %h\n", rv);
                rv = plotfile->InitWithNativePath( nsDependentCString(plotfilename) );
                if (rv) printf("plotfile->InitWithNativePath failed\n");
                plotfile->GetParent( getter_AddRefs(destdirectory));
                plotfile->GetLeafName( strLeafName );
                rv = plotfile->Remove(PR_FALSE);
                if (rv) printf("plotfile->Remove failed\n");              
                rv = engOutput->CopyTo( destdirectory, strLeafName );
                if (rv != NS_OK) printf("File not ready to copy!\n");
                plotOK = (rv == NS_OK);
              }
          }   
#endif          
          // plotOK = RenderPlot(zeng_output, plotfilename);
          mr->PutResultCode(CR_success);

        } else if (cmd_ID == CCID_Define ||
                   cmd_ID == CCID_Undefine /*||   cmd_ID == CCID_PassThru */ ) {
          // Don't process the return here
          mr->PutResultCode(CR_success);
        } else {
          if (*zeng_output == '"') {  // We gin "  " around the math that Polynomial Sort returns.
            // The double quotes are stripped away here.
            TCI_ASSERT(cmd_ID == CCID_Polynomial_Sort || cmd_ID == CCID_Definiteness_Tests);
            size_t zln = numBytes;
            if (zeng_output[zln - 1] == '"') {
              char *tmp = new char[zln - 1];
              strncpy(tmp, zeng_output + 1, zln - 2);
              tmp[zln - 2] = 0;
              delete[] zeng_output;
              zeng_output = tmp;
            } else {
                    TCI_ASSERT(0);
            }
          }

          MNODE *eng_tree = MuPAD_parser->MuPADStr2Tree(zeng_output);

          if (eng_tree) {
            // Build a "semantics" tree from eng_tree
            ClientAliases_REC *name_map = GetClientAliasesRec(client_ID);
            s_tree = engstr_analyzer->BuildSemanticsTree(eng_tree, cmd_ID,
                                                         name_map);


            if (cmd_ID >= CCID_Solve_PDE && cmd_ID <= CCID_Solve_ODE_Series) {
              PurgeDEFuncNames();
              if (cmd_ID == CCID_Solve_ODE_Numeric){
                 // The names in eng_tree have been defined in the engine. We have to 
                 // announce to defstore
              
                 DefStore* defstore = curr_msr->GetDefStore();
                 U32 engine_ID = curr_msr->GetEngineID();
                 BUCKET_REC* b = children -> bucket_list;
                 while (b) {
                    if (b->bucket_ID == MB_ODEFUNC) {
                       if (b -> first_child && b -> first_child -> canonical_ID) {
                         defstore->PushDefInfo(engine_ID, b -> first_child -> canonical_ID, 2, NULL, NULL, 0, NULL, NULL);
                       }
                    }
                    b = b -> next;
                 }
              }
            }

            // s_tree is our return value - it may not contain math
            // ie. it may be an engine message like "needvars"
            int msg_ID = GetEngineMessageID(s_tree);
            if (msg_ID) {
              mr->PutResultCode(msg_ID);
            } else {
              if (numBytes > 0) {
                if (cmd_ID == CCID_Factor_in_ring)
                  mr->PutResultCode(CR_ringfactor);
                else
                  mr->PutResultCode(CR_success);
              }
            }
          }
        }
        delete[] zeng_output;
      } else {                  // there is no engine return string
        if (cmd_ID == CCID_Undefine || cmd_ID == CCID_PassThru) {
          mr->PutResultCode(CR_success);
        } else {
          TCI_ASSERT(!"No engine return string.");
          mr->PutResultCode(CR_failure);
        }
      }
      delete[] zEngine_input;
    } else if (error_code) {
    mupd_eng->ClearEngStrPtrs();
      mr->PutResultCode(error_code);
    }
    delete[] eng_zstr;
  }

  TCI_ASSERT(!msg_list);
  mr->AttachMsgs(msg_list);
  msg_list = NULL;

  DisposeVarList(context_variables);
  context_variables = NULL;

  curr_msr = NULL;
  return s_tree;
}

bool MuPADWrapper::iLoadStrsAndDLL(nsILocalFile *engp, nsILocalFile *libp, nsILocalFile *modulep)
{
//  TCIString engpath = engp;
//  TCIString libpath = libp;
  return mupd_eng->LoadStrsAndDLL(engp, libp, modulep);
}

void MuPADWrapper::iStopProcessor()
{
   mupd_eng->StopProcessor();
}

int MuPADWrapper::iInitialize(nsILocalFile *libp, nsILocalFile *champ,
                                   nsILocalFile *vcamp)
{
  nsAutoString vcampath;
  vcamp->GetPath(vcampath);
  nsCString vcampathstr = NS_LossyConvertUTF16toASCII(vcampath);
  if (vcampathstr.get()) {
    size_t zln = strlen(vcampathstr.get());
    vcam_path = new char[zln + 1];
    strcpy(vcam_path, vcampathstr.get());
  }
//  TCIString libpath = libp;
//  TCIString champath = champ;
  return mupd_eng->Initialize(libp, champ);
}

bool MuPADWrapper::SetEngineState(int attr_ID, const char *new_value)
{
  bool rv = false;

  const char *eng_dbase_rec;
  char p_name[80];
  if (GetdBaseIDRecord("RESET", attr_ID, 0, &eng_dbase_rec, p_name, 80)) {
    if (eng_dbase_rec && *eng_dbase_rec) {
      char cmd[256];
      sprintf(cmd, eng_dbase_rec, new_value);
      TCIString str(cmd);
      rv = mupd_eng->SendToProcessor(str);
    }
  }

  return rv;
}

// The interface version, iExecute, returns an int.
// The caller is expected to retrieve the engine return string
//  using  __cdecl iRetrieveStr( 2,char* buffer,int bln );
//  which uses
//  from Maple::int RetrieveStr( int str_ID,char* buffer,int bln );

int MuPADWrapper::iExecute(const char *cmd)
{
  TCIString str(cmd);
  bool res = mupd_eng->SendToProcessor(str);

  if (res && str.GetLength())
    return 1;
  else
    return 0;
}

char *MuPADWrapper::Execute(const char *cmd)
{
  TCIString str(cmd);
  bool res;
  if (context_cmdtype == CC_PLOTFUNC)
    res = mupd_eng->SendPlotToProcessor(str);
  else
    res = mupd_eng->SendToProcessor(str);

  U32 nBytes = str.GetLength();
  if (res && nBytes) {
    char *rv = new char[nBytes + 1];
    strcpy(rv, static_cast<const char *>(str));
    return rv;
  } else {
    return NULL;
  }
}

bool MuPADWrapper::RenderPlot(const char *VCamFile, const char *outFile)
{
  TCI_ASSERT(vcam_path);
  TCI_ASSERT(VCamFile && outFile);
#ifdef XP_WIN
  TCIString cmdLine;
  cmdLine += "VCamNG.exe /e ";
  TCIString out (outFile);  // if leading /, VCam fails
  out.Replace("/","\\");
  cmdLine += "\"";
  cmdLine += out;
  cmdLine += "\" \"";
  cmdLine += VCamFile;
  cmdLine += "\"";
  TCICHAR *msSucks =
    const_cast < TCICHAR * >(static_cast < const TCICHAR * >(cmdLine));

  STARTUPINFO si;
  PROCESS_INFORMATION pi;
  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));

  if (!CreateProcess
      (vcam_path, msSucks, NULL, NULL, false, 0, NULL, NULL, &si, &pi)) {
#ifdef DEBUG
    DWORD err = GetLastError();
    TCI_ASSERT(!"Unable to run VCam.  Check error code.");
#endif
    return false;
  }

  WaitForSingleObject(pi.hProcess, INFINITE);
  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);

  // we don't need this now
  DeleteFile(VCamFile);
#else
  // XP_UNIX
  pid_t pid = fork();
  
  if (pid == 0) { // we are the child
    if (execl(vcam_path, vcam_path, "-convert", outFile, VCamFile, (char *)NULL)) {
#ifdef DEBUG
      TCI_ASSERT(!"Unable to run MuPAD/VCam.  Check errno code.");
#endif
      return false;
    }
  } else {
    int status;
    waitpid(pid, &status, 0);
  
    // we don't need this now
    unlink(VCamFile);
  }
#endif

  return true;
}

// There are a number of different formats for commands to Maple.
// The basic lay out is  "cmd_name(math_expression);"
// Commands that are parameterized have more complex layouts.

char *MuPADWrapper::CreateEngineCommandStr(MathServiceRequest & msr,
                                           U32 cmd_ID, const char *eng_expr,
                                           const char* plotfilename,
                                           int& error_code)
{
  char *rv = NULL;
  error_code = 0;

  // Get the dBase record for the command to be processed
  char op_name[80];             // Evaluate
  const char *cmd_template = NULL;  // %eval%(%expr%);
  U32 data_type = context_datatype == CD_Real ? 0 : 1;

  bool got_template = false;
  if (cmd_ID) {
    if (data_type > 0) {
      if (GetdBaseIDRecord("COMMANDS", cmd_ID, data_type, &cmd_template, op_name, 80))
        got_template = true;
    }
    if (!got_template) {
      if (GetdBaseIDRecord("COMMANDS", cmd_ID, 0, &cmd_template, op_name, 80))
        got_template = true;
    }
  }

  if (!got_template) {
    if (context_cmdtemplatetype == CT_FITCURVE
        || context_cmdtemplatetype == CT_FILLMATRIX
        || context_cmdtemplatetype == CT_SOLVEEXACT
        || context_cmdtemplatetype == CT_POLYSORT
        || context_cmdtemplatetype == CT_EXPRandVAR
        || context_cmdtemplatetype == CT_POLYROOTS
        || context_cmdtemplatetype == CT_EXTREMA
        || context_cmdtemplatetype == CT_MATRIXMINPOL
        || context_cmdtemplatetype == CT_COMPANIONMAT
        || context_cmdtemplatetype == CT_WRONSKIAN
        || context_cmdtemplatetype == CT_CHARACTERISTICP
        || context_cmdtemplatetype == CT_PLOTFUNC 
        || context_cmdtemplatetype == CT_PLOTQUERY) {
      // Here, command production are looked up later.
    } else {
      TCI_ASSERT(0);
      error_code = CR_unsupported_command;
      return NULL;
    }
  }

  switch (context_cmdtemplatetype) {

  case CT_UNKNOWN:
    TCI_ASSERT(0);
    break;

  case CT_NORMAL:              // Evaluate<uID1.0>eval(%s);
    rv = CreateStandardeStr(eng_expr, cmd_template, error_code);
    break;

  case CT_EQN2MAT:             // tciequationstomatrix(%s,[%var_list%]);
    rv = CreateEQN2MATeStr(msr, eng_expr, cmd_template, error_code);
    break;
  case CT_COLLECT:             // Polynomial Collect...<uID29.0>collect(%expr&,{%var%});
    rv = CreateCOLLECTeStr(msr, eng_expr, cmd_template, error_code);
    break;
  case CT_RANDMATRIX:
    rv = CreateRANDMATRIXeStr(msr, eng_expr, cmd_template, error_code);
    break;
  case CT_RANDOMNUM:
    rv = CreateRANDOMNUMeStr(msr, cmd_template, error_code);
    break;
  case CT_RESHAPE:
    rv = CreateRESHAPEeStr(eng_expr, cmd_template, error_code);
    break;
  case CT_MAPENTRIES:
    rv = CreateMAPSeStr(msr, eng_expr, cmd_template, error_code);
    break;

  case CT_SOLVEEXACT:
  case CT_POLYSORT:            //  tcisort(%expr%,{%which_var%}); 
  case CT_EXPRandVAR:          //  tciparfrac(%expr%,%which_var%); 
  case CT_POLYROOTS:           //  tciroot(%expr%,%which_var%); 
  case CT_EXTREMA:
  case CT_MATRIXMINPOL:
  case CT_COMPANIONMAT:
  case CT_WRONSKIAN:
  case CT_CHARACTERISTICP:
    rv = CreateWHICHVAReStr(msr, cmd_ID, eng_expr, error_code);
    break;

  case CT_TRANSFORM:           // Fourier, Laplace
    rv = CreateTRANSFORMeStr(eng_expr, cmd_template, error_code);
    break;
  case CT_VCALC:               // Gradient, Divergence, Curl, etc.
    rv = CreateVCALCeStr(msr, eng_expr, cmd_template, error_code);
    break;

  case CT_PSERIES:             // Power Series...<uID44.0>series(%expr%,%var%=%num%,%order%);
    rv = CreatePSERIESeStr(msr, eng_expr, cmd_template, error_code);
    break;
  case CT_ODESERIES:           // Solve ODE Series<uID49.0>ode::series(%theODE_and_func%,%var%=%expand_at%,%order%);
    rv = CreateODESERIESeStr(msr, eng_expr, cmd_template, error_code);
    break;

  case CT_QUANTILE:            // 
    rv = CreateQUANTILEeStr(msr, eng_expr, cmd_template, error_code);
    break;
  case CT_MOMENT:              // 
    rv = CreateMOMENTeStr(msr, eng_expr, cmd_template, error_code);
    break;
  case CT_ITERATE:             // 
    rv = CreateITERATEeStr(msr, cmd_template, error_code);
    break;

  case CT_CALCBYPARTS:
  case CT_CALCCHANGEVAR:
    rv = CreateSIntegraleStr(msr, cmd_ID, eng_expr, cmd_template, error_code);
    break;
                                            
  case CT_CALCAPPROXINT:
    rv = CreateAPPROXINTeStr(msr, eng_expr, cmd_template, error_code);
    break;

  case CT_FITCURVE:
    rv = CreateFITCURVEeStr(msr, eng_expr, error_code);
    break;
  case CT_FILLMATRIX:
    rv = CreateFILLMATRIXeStr(msr, error_code);
    break;

  case CT_PLOTFUNC:
    rv = MuPADPlotWrapper::CreatePLOTFUNCeStr(this, static_cast<PlotServiceRequest*>(&msr), eng_expr, plotfilename, error_code);
    if (error_code == 0) 
      error_code = MuPADPlotWrapper::CheckCmdString (rv);
    if (error_code)
      RecordMsg(msg_list, MSG_UNSUPPORTED_OPERATOR, rv);
    break;

  case CT_PLOTQUERY:
    rv = MuPADPlotWrapper::CreatePLOTQueryeStr(this, static_cast<PlotServiceRequest*>(&msr), eng_expr, error_code);
    if (error_code == 0) 
      error_code = MuPADPlotWrapper::CheckCmdString (rv);
    if (error_code)
        RecordMsg(msg_list, MSG_UNSUPPORTED_OPERATOR, rv);
    break;

  default:
    TCI_ASSERT(0);
    break;
  }

  return rv;
}

// Iterate thru a list of math objects "x"  "+"  "-"
// Get the engine specific translation of each object
//  and catenate it to the returned string.

char *MuPADWrapper::ProcessSemList(SEMANTICS_NODE * head, const char *modulus,
                                   int& error_code)
{ 
  char *rv = NULL;
  error_code = 0;

  SEMANTICS_NODE *rover = head;
  while (rover) {
    SEMANTICS_NODE *next_node = rover->next;
    char *eng_zstr = ProcessSemNode(rover, modulus, error_code);
    if (error_code) {
      delete[] eng_zstr;
      delete[] rv;
      return NULL;
    }

    if (eng_zstr) {
      size_t zln = strlen(eng_zstr);
      if (rv) {
        char *save = rv;
        zln += strlen(save);
        rv = new char[zln + 1];
        strcpy(rv, save);
        strcat(rv, eng_zstr);
        delete[] save;
      } else {
        rv = new char[zln + 1];
        strcpy(rv, eng_zstr);
      }
      delete[] eng_zstr;
    }
    rover = next_node;
  }

  return rv;
}

// Generate the engine specific translation for a single math object.
// Just a switch on math object type.

char *MuPADWrapper::ProcessSemNode(SEMANTICS_NODE * s_node,
                                   const char *modulus, int& error_code)
{ 
  char *rv = NULL;

  if (s_node) {

    switch (s_node->semantic_type) {
    case SEM_TYP_VARIABLE:
      rv = VARIABLEtoEstr(s_node, error_code);
      break;
    case SEM_TYP_INDEXED_VAR:
    case SEM_TYP_QUALIFIED_VAR:
      rv = QVARIABLEtoEstr(s_node, error_code);
      break;

    case SEM_TYP_NUMBER:
      rv = NUMBERtoEstr(s_node, error_code);
      break;

    case SEM_TYP_MIXEDNUMBER:
      rv = MIXEDNUMBERtoEstr(s_node, error_code);
      break;

    case SEM_TYP_UCONSTANT:
      rv = UCONSTANTtoEstr(s_node, error_code);
      break;

    case SEM_TYP_ENG_PASSTHRU:
      if (s_node->contents) {
        size_t zln = strlen(s_node->contents);
        rv = new char[zln + 1];
        strcpy(rv, s_node->contents);
      }
      break;

    case SEM_TYP_LISTOPERATOR:
    case SEM_TYP_FUNCTION:
    case SEM_TYP_INVFUNCTION:
      rv = FUNCTIONtoEstr(s_node, error_code);
      break;

    case SEM_TYP_TRANSFORM:
    case SEM_TYP_INVTRANSFORM:
      rv = TRANSFORMtoEstr(s_node, NULL, NULL, error_code);
      break;

    case SEM_TYP_LIMFUNC:
      rv = LIMFUNCtoEstr(s_node, error_code);
      break;

    case SEM_TYP_DERIVATIVE:
      if (context_cmdtype == CC_ODE_CONDITION)
        rv = ODEBoundarytoEstr(s_node, error_code);
      else if (context_cmdtype == CC_ODE)
        rv = DERIVATIVEtoEstr(s_node, error_code);
      else {
        BUCKET_REC *nprimes_bucket = FindBucketRec(s_node->bucket_list,
                                                   MB_NPRIMES);
        if (nprimes_bucket)
          rv = PRIMEStoEstr(s_node, error_code);
        else
          rv = DERIVATIVEtoEstr(s_node, error_code);
      }
      break;

    case SEM_TYP_PIECEWISE_FENCE:
      //TCI_ASSERT(!"PIECEWISE_FENCE not supported by MuPADWrapper::ProcessSemNode");
      //break;

    case SEM_TYP_PIECEWISE_LIST:
      rv = Piecewise2Estr(s_node, error_code);
      break;

    case SEM_TYP_ODE_SYSTEM:
      if (!strcmp(s_node->contents, "ODE Numeric"))
        rv = ODENumerictoEstr(s_node, error_code);
      else
        rv = ODEtoEstr(s_node, error_code);
      break;

    case SEM_TYP_RECURSION:
      rv = Recursion2Estr(s_node, error_code);
      break;

    case SEM_TYP_SUBSTITUTION:
      rv = SUBSTITUTIONtoEstr(s_node, error_code);
      break;

    case SEM_TYP_MULTIARGFUNC:
      rv = MULTIARGFUNCtoEstr(s_node, error_code);
      break;

    case SEM_TYP_BIGOP_SUM:
      rv = BIGOPtoEstr(s_node, error_code);
      break;

    case SEM_TYP_BIGOP_INTEGRAL:
      rv = INTEGRALtoEstr(s_node, error_code);
      break;

    case SEM_TYP_PREFIX_OP:
    case SEM_TYP_INFIX_OP:
    case SEM_TYP_POSTFIX_OP:{
        bool is_infix = false;
        rv = OPERATORtoEstr(s_node, modulus, is_infix, error_code);
        if (rv && is_infix && s_node->bucket_list) {
          rv = NestInParens(rv, false);
        } else if (s_node->semantic_type == SEM_TYP_PREFIX_OP
                   && s_node->contents) {
          char ch = s_node->contents[0];
          if (!is_infix && (ch == '+' || !strcmp(s_node->contents, "&#x2212;") || !strcmp(s_node->contents, "-")) )
            rv = NestInParens(rv, false);
        }
      }
      break;

    case SEM_TYP_SPACE:
      // do nothing
      break;

    case SEM_TYP_TEXT:
      rv = TEXTtoEstr(s_node, error_code);
      break;

    case SEM_TYP_SIUNIT:
    case SEM_TYP_USUNIT:
      rv = UNITtoEstr(s_node, error_code);
      break;

    case SEM_TYP_MATH_CONTAINER:
      if (s_node->bucket_list) {
        BUCKET_REC *bucket_info = s_node->bucket_list;
        if (bucket_info->first_child) {
          rv = ProcessSemList(bucket_info->first_child, modulus, error_code);
    }
        else
          TCI_ASSERT(0);
      }
      break;

    case SEM_TYP_PRECEDENCE_GROUP:
      if (s_node->bucket_list) {
        BUCKET_REC *bucket_info = s_node->bucket_list;
        if (bucket_info->first_child){
          rv = ProcessSemList(bucket_info->first_child, modulus, error_code);
    }
        else
          TCI_ASSERT(0);
      }
      break;

    case SEM_TYP_PARENED_LIST:
    case SEM_TYP_BRACKETED_LIST:
    case SEM_TYP_GENERIC_FENCE: { 
       
       int length = 0;
       rv = LISTtoEstr(s_node, NULL, NULL, ",", modulus, error_code, length);
       if (length > 1){
         char lenStr[10];
         StrFromInt(length, lenStr);
         size_t zln = strlen(rv);
         zln += strlen("SWPmatrix(0,0,[])");
         char* newrv = new char[zln+10];
         strcpy(newrv, "SWPmatrix(1,");
         strcat(newrv, lenStr);
         strcat(newrv, ",[");
         strcat(newrv, rv);
         strcat(newrv, "])");
         delete rv;
         rv = newrv;
       } else {
         if (s_node->semantic_type == SEM_TYP_BRACKETED_LIST)
           rv = NestInBrackets(rv);
         else
           rv = NestInParens(rv, false);
       }
       break;
    }

    case SEM_TYP_LIST:
      rv = LISTtoEstr(s_node, NULL, NULL, ",", modulus, error_code);
      break;

    case SEM_TYP_INTERVAL:
      rv = INTERVALtoEstr(s_node, error_code);
      break;

    case SEM_TYP_FRACTION:
      rv = FRACTIONtoEstr(s_node, "fraction", error_code);
      break;

    case SEM_TYP_BINOMIAL:
      rv = FRACTIONtoEstr(s_node, "binomial", error_code);
      break;

    case SEM_TYP_POWERFORM:
      rv = POWERFORMtoEstr(s_node, true, modulus, error_code);
      break;

    case SEM_TYP_SQRT:
      rv = SQROOTtoEstr(s_node, false, error_code);
      break;

    case SEM_TYP_ROOT:
      rv = ROOTtoEstr(s_node, error_code);
      break;

    case SEM_TYP_TABULATION:
      rv = TABULATIONtoEstr(s_node, modulus, error_code);
      break;

    case SEM_TYP_SIMPLEX:
      rv = SIMPLEXtoEstr(s_node, error_code);
      break;

    case SEM_TYP_NUMSYSTEM:
      rv = NUMSYSTEMtoEstr(s_node, error_code);
      break;

    case SEM_TYP_SOLVESYSTEM:
      rv = SOLVESYSTEMtoEstr(s_node, error_code);
      break;

    case SEM_TYP_VARDEF:
    case SEM_TYP_VARDEF_DEFERRED:
      rv = VARASSIGNtoEstr(s_node, error_code);
      break;

    case SEM_TYP_FUNCDEF:
      rv = FUNCDEFtoEstr(s_node, error_code);
      break;

    case SEM_TYP_SET:
      rv = SETtoEstr(s_node, error_code);
      break;

    case SEM_TYP_MTRANSPOSE:
      rv = TRANSPOSEtoEstr(s_node, 'T', error_code);
      break;
    case SEM_TYP_HTRANSPOSE:
      rv = TRANSPOSEtoEstr(s_node, 'H', error_code);
      break;

    case SEM_TYP_CONJUGATE:
      rv = CONJUGATEtoEstr(s_node, error_code);
      context_datatype = CD_Complex;
      break;

    case SEM_TYP_ABS:
    case SEM_TYP_FLOOR:
    case SEM_TYP_CEILING:
      rv = FUNCTIONtoEstr(s_node, error_code);
      break;

    case SEM_TYP_NORM:
      rv = NORMtoEstr(s_node, error_code);
      break;

    default:
      TCI_ASSERT(0);
      break;
    }
  }

  return rv;
}


char *MuPADWrapper::VARIABLEtoEstr(SEMANTICS_NODE * snode, int& error_code)
{

  char *rv = NULL;
  error_code = 0;

  // This user variable is (or will be) defined within the scope of some client.
  // Analyzer has recorded the client ID of the owner in snode->owner_ID.

  U32 frame_ID = snode->owner_ID;
  
  if (!frame_ID) {              // give ownership to the client requesting
    frame_ID = client_ID;       //  the current computation
  }
  
  // See if we already have an engine name for this variable.

  ClientAliases_REC *client_aliases = GetClientAliasesRec(frame_ID);
  NAME2AKA_REC *map = GetAKARec(client_aliases->AKAs_list, snode->canonical_ID, snode->contents);

  if (map) {
    size_t zln = strlen(map->alias);
    char *tmp = new char[zln + 1];
    strcpy(tmp, map->alias);
    rv = tmp;

  } else {
    rv = GetEngNameForVar(frame_ID, snode->contents, snode->canonical_ID);
  }

  const char *db_production = NULL;
  U32 ID, subID;

  if (GetdBaseNamedRecord("MUPADSTRS", snode->contents, &db_production, ID, subID)){
     size_t zln = strlen(db_production);
     char *tmp = new char[zln + 1];
     strcpy(tmp, db_production);
     rv = tmp;
  }

////   if (snode->bucket_list && snode->bucket_list->first_child) {
//// //    SEMANTICS_NODE *s_qualifier = snode->bucket_list->first_child;
////     TCI_ASSERT(0);
////     // Qualified variables are now typed as SEM_TYP_QUALIFIED_VAR or SEM_TYP_INDEXED_VAR
////   }

  return rv;
}

// The engine wrapper will be required to convert the canonical
//  string to the format required by the underlying engine.
// In general, an engine wrapper may look for a few common
//  number formats.  Everything else will have to be rejected
//  as an error or aliased in the engine command string.

char *MuPADWrapper::NUMBERtoEstr(SEMANTICS_NODE * snode, int& error_code)
{
  char *rv = NULL;
  error_code = 0;

  if (snode->contents) {
    int local_err_code;
    rv = CanonicalNumberToEngineStr(snode->contents, local_err_code);
    if (local_err_code) {
      if (0) {
        RecordMsg(msg_list, MSG_UNSUPPORTED_NUMBER, snode->contents);
        error_code = CR_failure;
      } else {
        // Here the engine wrapper couldn't make sense of the data
        //  within a source <mn>.  We make a symbolic token to represent
        //  this "number", and incorporate it into the string going
        //  to the engine.  Generally, we expect the token to pass thru  
        //  the computation and appear in the engine output.

        char *base_name = NULL;
        bool use_alias = true;
        rv = CreateEngName(client_ID,
                           snode->canonical_ID, base_name, use_alias, Ilk_Var);
      }
    }
  } else
    TCI_ASSERT(0);

  return rv;
}

char *MuPADWrapper::MIXEDNUMBERtoEstr(SEMANTICS_NODE * s_mixed,
                                      int& error_code)
{
  char *zh_rv = NULL;
  error_code = 0;

  const char *key = "mixednumber";
  const char *db_production;
  U32 ID, subID;
  if (GetdBaseNamedRecord("SCHEMATA", key, &db_production, ID, subID)) {
    TCI_ASSERT(db_production);  // mixednumber<uID16.0>(%whole%+%frac%)
    char *whole_str = NULL;
    char *frac_str = NULL;

    BUCKET_REC *bucket = FindBucketRec(s_mixed->bucket_list, MB_MN_WHOLE);
    if (bucket && bucket->first_child) {
      whole_str = ProcessSemList(bucket->first_child, NULL, error_code);
      if (error_code) {
        delete[] whole_str;
        return NULL;
      }
    }

    bucket = FindBucketRec(s_mixed->bucket_list, MB_MN_FRACTION);
    if (bucket && bucket->first_child) {
      frac_str = ProcessSemList(bucket->first_child, NULL, error_code);
      if (error_code) {
        delete[] whole_str;
        delete[] frac_str;
        return NULL;
      }
    }

    if (whole_str && frac_str) {
      size_t bln =
        strlen(db_production) + strlen(whole_str) + strlen(frac_str);
      zh_rv = new char[bln];
      strcpy(zh_rv, db_production);
      StrReplace(zh_rv, bln, "%whole%", whole_str);
      StrReplace(zh_rv, bln, "%frac%", frac_str);
    }
    delete[] whole_str;
    delete[] frac_str;
  } else
    TCI_ASSERT(0);

  return zh_rv;
}

// Pi, etc.

char *MuPADWrapper::UCONSTANTtoEstr(SEMANTICS_NODE * snode, int& error_code)
{

  char *rv = NULL;
  error_code = 0;

  char *key = snode->contents;
  if (key) {
    const char *db_production;
    U32 ID, subID;
    if (GetdBaseNamedRecord("UCONSTANTS", key, &db_production, ID, subID)) {
      if (db_production && *db_production) {
        // the database for this engine contains a token for this constant
        size_t zln = strlen(db_production);
        rv = new char[zln + 1];
        strcpy(rv, db_production);

        if (zln == 1 && rv[0] == 'I')
          context_datatype = CD_Complex;
      }
    } else {
      // constant unknown to this engine - pass thru using an alias
      char *base_name = NULL;
      bool use_alias = true;
      rv = CreateEngName(client_ID,
                         snode->canonical_ID, base_name, use_alias, Ilk_Var);
    }
  } else
    TCI_ASSERT(0);

  return rv;
}

char *MuPADWrapper::TEXTtoEstr(SEMANTICS_NODE * snode, int& error_code)
{
  char *rv = NULL;
  error_code = 0;

  if (snode->contents) {
    if (0) {
      RecordMsg(msg_list, MSG_TEXT_IN_MATH, snode->contents);
      error_code = CR_failure;
    } else {
    }
  } else
    TCI_ASSERT(0);

  return rv;
}

char *MuPADWrapper::UNITtoEstr(SEMANTICS_NODE * snode, int& error_code)
{
  char *rv = NULL;

  char *key = snode->contents;
  if (key) {
    const char *db_production;
    U32 ID, subID;
    if (GetdBaseNamedRecord("SICONVERSIONS", key, &db_production, ID, subID)) {
      if (db_production && *db_production) {
        size_t zln = strlen(db_production);
        rv = new char[zln + 1];
        strcpy(rv, db_production);
      }
    } else
      TCI_ASSERT(0);
  } else
    TCI_ASSERT(0);

  return rv;
}

// The core side does a lot to support function handling.
// ie the "s_func" contains all the data needed to script
//  the engine function call in a direct manner.

char *MuPADWrapper::FUNCTIONtoEstr(SEMANTICS_NODE * s_func, int& error_code)
{
  char *rv = NULL;
  error_code = 0;

  char *new_production = NULL;
  U32 np_ln = 0;

  bool user_func = false;
  bool pass_thru = false;
  bool in_defstore = false;
  bool is_mupname = false;

  if (s_func->msi_class) {
    pass_thru = true;
  } else if (s_func->canonical_ID) {
    DefStore *defstore = curr_msr->GetDefStore();
    U32 engine_ID = curr_msr->GetEngineID();
    DefInfo *di = defstore->GetDefInfo(engine_ID, s_func->canonical_ID);
    if (di && di->def_type == DT_MUPNAME){
       is_mupname = true;
       char* mupname = DuplicateString(di->src_markup);
       char* parenloc = strchr(mupname, '(');
       if (parenloc != NULL) {
         int i = parenloc - mupname;
         mupname[i] = '\0';
       }
       new_production = AppendStr2HeapStr(new_production, np_ln, mupname);
       new_production = AppendStr2HeapStr(new_production, np_ln, "%arg_list%");
       in_defstore = true;

    }
    if (di && di->arg_list)
      in_defstore = true;
  }

  if (pass_thru) {
    //
  } else if (in_defstore) {
    user_func = true;
  } else {
    char *key = s_func->contents;
    const char *db_production = NULL;
    U32 ID, subID;

    char *db_section = "FUNCTIONS";
    if (s_func->semantic_type == SEM_TYP_INVFUNCTION)
      db_section = "INVERSES";
    else if (s_func->semantic_type == SEM_TYP_LISTOPERATOR)
      db_section = "LISTOPERATORS";
    else if (s_func->semantic_type == SEM_TYP_FUNCTION
             || s_func->semantic_type == SEM_TYP_PREFIX_OP) {
      if (!s_func->bucket_list)
        if (GetdBaseNamedRecord("MUPADSTRS", key, &db_production, ID, subID))
          db_section = "MUPADSTRS";
      // Real, Complex, Integer, Negative, Positive, NonZero
      // These items shouldn't be classified as functions??
    }

    if (GetdBaseNamedRecord(db_section, key, &db_production, ID, subID)) {
      // Special case - Probability functions - ProbDist( a;s1,s2.. );
      if (ID >= 8089 && ID <= 8127)
        return PROBFUNCtoEstr(s_func, db_production, ID, error_code);

      // Here, the user has entered a function that is known to the engine.
      // ie. a function that need not be defined by the user.
      // The production in the engine database is a template for
      //  a call to the function - it includes the engine's name
      //  for the function.
      new_production =
        AppendStr2HeapStr(new_production, np_ln, db_production);
    } else {
      // Not in maple.gmr AND not in the DefStore
      // An error? maybe a passthru?
      user_func = true;
    }
  }

  if (!is_mupname && (user_func || pass_thru) ){
    // Here, the user has entered a function that is NOT pre-defined
    //  on the engine.  We get a name for this function that can be sent
    //  to the engine.  The name-generating algorithm adds a namespace
    //  prefix and will produce an alias if the user's name is not
    //  a simple <mi>.  We keep a list of engine names generated for
    //  user functions for each client.  This list is used to assure
    //  that the same engine name is used for every instance of a 
    //  particular user function.  

    // Built a production of the form "%func_name%%arg_list%"

    if (pass_thru)
      new_production =
        AppendStr2HeapStr(new_production, np_ln, s_func->contents);
    else {
      char *f_nom_str = GetEngNameForUserFunc(s_func);
      new_production = AppendStr2HeapStr(new_production, np_ln, f_nom_str);
      delete[] f_nom_str;
    }

    new_production = AppendStr2HeapStr(new_production, np_ln, "%arg_list%");
  }

  char *base_str = NULL;
  char *args_str = NULL;

  // logs are a special case - carry a log base, so do bessel
  BUCKET_REC *lb_bucket = FindBucketRec(s_func->bucket_list, MB_LOG_BASE);
  if (lb_bucket && lb_bucket->first_child) {
      SEMANTICS_NODE *s_log_base = lb_bucket->first_child;
      if (s_log_base->semantic_type != SEM_TYP_UCONSTANT) {
        const char *db_production = NULL;
        U32 ID, subID;
        if (!strcmp("log", s_func->contents)){
          if (GetdBaseNamedRecord("FUNCTIONS", "logwithbase", &db_production, ID, subID)) {
          // logwithbase<uID8023.1>tcilog(%base%,%arg_list%)
          delete[] new_production;
          new_production = NULL;
          }
        }
        np_ln = 0;
        new_production =
            AppendStr2HeapStr(new_production, np_ln, db_production);

          SEMANTICS_NODE *curr_handle = lb_bucket->first_child;
          base_str = ProcessSemList(curr_handle, NULL, error_code);
          if (error_code) {
            delete[] base_str;
            return rv;
          }
        //} else {
        //  TCI_ASSERT(0);
        }
      }
    
  

  char *p2 = strstr(new_production, "%arg_list%");
  if (p2 && context_cmdtype != CC_UNDEFINE) {
    args_str = LISTtoEstr(s_func, NULL, NULL, ",", NULL, error_code);
    if (error_code) {
      delete[] base_str;
      delete[] args_str;
      return rv;
    } else {
      if (args_str)
        args_str = NestInParens(args_str, true);
    }
  }

  size_t bln = strlen(new_production) + 1;
  if (base_str)
    bln += strlen(base_str);
  if (args_str)
    bln += strlen(args_str);
  rv = new char[bln];
  strcpy(rv, new_production);
  StrReplace(rv, bln, "%base%", base_str);
  StrReplace(rv, bln, "%arg_list%", args_str);

  delete[] base_str;
  delete[] args_str;

  // this function may carry an exponent
  BUCKET_REC *power_bucket =
    FindBucketRec(s_func->bucket_list, MB_FUNC_EXPONENT);
  if (power_bucket) {
    char *sup_str = NULL;
    char *base_str = rv;
    base_str = NestInParens(base_str, false);

    int err_code = 0;
    if (power_bucket->first_child) {
      SEMANTICS_NODE *curr_handle = power_bucket->first_child;
      char *obj_str = ProcessSemList(curr_handle, NULL, err_code);
      if (obj_str) {
        if (err_code != 1) {
          sup_str = obj_str;
          sup_str = NestInParens(sup_str, false);
        } else
          delete[] obj_str;
      }
    }

    const char *db_production;
    U32 ID, subID;
    GetdBaseNamedRecord("SCHEMATA", "superscript", &db_production, ID, subID);
    // superscript<uID1.0>%base%^%superscript%
    size_t bln = strlen(db_production) + 1;
    if (base_str)
      bln += strlen(base_str);
    if (sup_str)
      bln += strlen(sup_str);
    rv = new char[bln];
    strcpy(rv, db_production);
    StrReplace(rv, bln, "%base%", base_str);
    StrReplace(rv, bln, "%superscript%", sup_str);

    delete[] base_str;
    delete[] sup_str;
  }

  delete[] new_production;

  return rv;
}

char *MuPADWrapper::SETtoEstr(SEMANTICS_NODE * set_info, int& error_code)
{
  char *zh_arglist = NULL;
  U32 aln = 0;
  error_code = 0;

  int err_code = 0;
  int elem_counter = 0;
  BUCKET_REC *bucket_rover = set_info->bucket_list;
  while (bucket_rover && !err_code) {
    if (bucket_rover->bucket_ID == MB_UNNAMED) {
      if (elem_counter == 0)
        zh_arglist = AppendStr2HeapStr(zh_arglist, aln, "{");
      else
        zh_arglist = AppendStr2HeapStr(zh_arglist, aln, ",");

      if (bucket_rover->first_child) {
        SEMANTICS_NODE *curr_handle = bucket_rover->first_child;
        char *obj_str = ProcessSemList(curr_handle, NULL, err_code);
        if (obj_str) {
          if (!err_code)
            zh_arglist = AppendStr2HeapStr(zh_arglist, aln, obj_str);
          delete[] obj_str;
        }
      }
      elem_counter++;
    }
    bucket_rover = bucket_rover->next;
  }
  if (err_code) {
    error_code = err_code;
    return NULL;
  }
  if (zh_arglist)
    zh_arglist = AppendStr2HeapStr(zh_arglist, aln, "}");

  return zh_arglist;
}

/*
LIMFUNC contents = "lim"
Bucket list
  LOWERLIMIT
    Bucket Parts
      LL VARIABLE
        VARIABLE canonical_ID = "mix" contents = "x"
      LL LIMIT EXPR
        NUMBER canonical_ID = "mn1" contents = "1"
      LL DIRECTION
        NUMBER contents = "0"
    End Bucket Parts
  UNNAMED
    VARIABLE canonical_ID = "mix" contents = "x"
End Bucket list
*/
// eval( limit((1/q2x),q2x=1{,right}) );

char *MuPADWrapper::LIMFUNCtoEstr(SEMANTICS_NODE * limfunc_info,
                                  int& error_code)
{
  char *rv = NULL;
  error_code = 0;

  char *key = limfunc_info->contents;

  const char *db_production;
  U32 ID, subID;
  if (GetdBaseNamedRecord("LIMFUNCS", key, &db_production, ID, subID)) {
    TCI_ASSERT(db_production);
    //lim<uID.1.1>limit(%arg_list%,%var%=%lim_exp%%direction%)
    //max<uID1.2>tcimax(%arg_list% $ %var%=%ll%..%ul%)

    char *arg_str = NULL;
    char *var_str = NULL;
    char *lim_exp_str = NULL;
    char *direction_str = NULL;
    char *ll_str = NULL;
    char *ul_str = NULL;

    const char *ptr = strstr(db_production, "%arg_list%");
    if (ptr) {
      // limfunc type operator takes an operand
      BUCKET_REC *arg_bucket =
        FindBucketRec(limfunc_info->bucket_list, MB_UNNAMED);
      if (arg_bucket && arg_bucket->first_child) {
        arg_str = ProcessSemList(arg_bucket->first_child, NULL, error_code);
      }
      if (error_code)
        return NULL;
    }

    const char *p_lim_exp = NULL;
    const char *p_ll = NULL;
    const char *p_ul = NULL;
    BUCKET_REC *l_bucket = FindBucketRec(limfunc_info->bucket_list,
                                         MB_LOWERLIMIT);
    if (l_bucket) {
      p_lim_exp = strstr(db_production, "%lim_exp%");

      p_ll = strstr(db_production, "%ll%");
      p_ul = strstr(db_production, "%ul%");

      SEMANTICS_NODE *s_lim = l_bucket->first_child;
      if (s_lim && s_lim->semantic_type == SEM_TYP_INTERVAL) {
        BUCKET_REC *var_bucket =
          FindBucketRec(s_lim->bucket_list, MB_INTERVAL_VAR);
        BUCKET_REC *ill_bucket =
          FindBucketRec(s_lim->bucket_list, MB_INTERVAL_START);
        BUCKET_REC *iul_bucket =
          FindBucketRec(s_lim->bucket_list, MB_INTERVAL_END);
        if (var_bucket) {
          SEMANTICS_NODE *var_snode = var_bucket->first_child;
          var_str = ProcessSemNode(var_snode, NULL, error_code);
          if (error_code) {
            delete[] var_str;
            delete[] arg_str;
            return NULL;
          }
        }
        if (p_ll && p_ul) {
          if (ill_bucket) {
            SEMANTICS_NODE *ll_snode = ill_bucket->first_child;
            ll_str = ProcessSemNode(ll_snode, NULL, error_code);
            if (error_code) {
              delete[] ll_str;
              delete[] lim_exp_str;
              delete[] var_str;
              delete[] arg_str;
              return NULL;
            }
          }
          if (iul_bucket) {
            SEMANTICS_NODE *ul_snode = iul_bucket->first_child;
            ul_str = ProcessSemNode(ul_snode, NULL, error_code);
            if (error_code) {
              delete[] ul_str;
              delete[] ll_str;
              delete[] lim_exp_str;
              delete[] var_str;
              delete[] arg_str;
              return NULL;
            }
          }
        } else {
          TCI_ASSERT(0);
        }
      } else {                  // lower limit is like "n->0"
        BUCKET_REC *expr_bucket = FindBucketRec(l_bucket->parts, MB_LL_EXPR);
	if (!expr_bucket){
	  delete[] lim_exp_str;
          delete[] var_str;
          delete[] arg_str;
	  return NULL;
	}

        if (p_lim_exp) {
          int direction = 0;    // non-directional limit
          BUCKET_REC *dir_bucket =
            FindBucketRec(l_bucket->parts, MB_LL_DIRECTION);
          if (dir_bucket) {
            SEMANTICS_NODE *snum = dir_bucket->first_child;
            if (snum
                && snum->contents
                && snum->semantic_type == SEM_TYP_NUMBER)
              direction = atoi(snum->contents);
          }
          SEMANTICS_NODE *lim_exp_snode = expr_bucket->first_child;
          if (lim_exp_snode) {
            lim_exp_str = ProcessSemNode(lim_exp_snode, NULL, error_code);
            if (error_code) {
              delete[] lim_exp_str;
              delete[] var_str;
              delete[] arg_str;
              return NULL;
            }
            if (direction == 1)
              direction_str = ",Right";
            else if (direction == 2)
              direction_str = ",Left";
          }

          BUCKET_REC *var_bucket = FindBucketRec(l_bucket->parts, MB_LL_VAR);
          if (var_bucket) {
            SEMANTICS_NODE *var_snode = var_bucket->first_child;
            var_str = ProcessSemNode(var_snode, NULL, error_code);
            if (error_code) {
              delete[] lim_exp_str;
              delete[] var_str;
              delete[] arg_str;
              return NULL;
            }
          } else
            TCI_ASSERT(0);
        }
      }
    } else
      TCI_ASSERT(0);

    size_t bln = strlen(db_production) + 1;
    if (arg_str)
      bln += strlen(arg_str);
    if (var_str)
      bln += strlen(var_str);
    if (lim_exp_str)
      bln += strlen(lim_exp_str);
    if (direction_str)
      bln += strlen(direction_str);
    if (ll_str)
      bln += strlen(ll_str);
    if (ul_str)
      bln += strlen(ul_str);
    rv = new char[bln];
    strcpy(rv, db_production);
    StrReplace(rv, bln, "%arg_list%", arg_str);
    StrReplace(rv, bln, "%var%", var_str);
    if (p_lim_exp) {
      StrReplace(rv, bln, "%lim_exp%", lim_exp_str);
      StrReplace(rv, bln, "%direction%", direction_str);
    }
    if (p_ll)
      StrReplace(rv, bln, "%ll%", ll_str);
    if (p_ul)
      StrReplace(rv, bln, "%ul%", ul_str);

    delete[] arg_str;
    delete[] var_str;
    delete[] lim_exp_str;
    delete[] ll_str;
    delete[] ul_str;
  } else                        // if ( db_production )
    TCI_ASSERT(0);

  return rv;
}

char *MuPADWrapper::SUBSTITUTIONtoEstr(SEMANTICS_NODE * subst_info,
                                       int& error_code)
{
  char *rv = NULL;
  error_code = 0;

  char *key = subst_info->contents; // "substitute";
  const char *db_production;
  U32 ID, subID;
  if (GetdBaseNamedRecord("SCHEMATA", key, &db_production, ID, subID)) {
    TCI_ASSERT(db_production);
    //substitution<uID18.0>subs(%expr%,%subst_list%)
    //doublesubstitution<uID19.0>subs(%expr%,%usubst_list%)-subs(%expr%,%subst_list%)

    char *expr_str = NULL;
    char *lower_str = NULL;
    char *upper_str = NULL;

    BUCKET_REC *expr_bucket =
      FindBucketRec(subst_info->bucket_list, MB_UNNAMED);
    BUCKET_REC *lower_bucket =
      FindBucketRec(subst_info->bucket_list, MB_SUBST_LOWER);
    BUCKET_REC *upper_bucket =
      FindBucketRec(subst_info->bucket_list, MB_SUBST_UPPER);
    if (expr_bucket && expr_bucket->first_child) {
      expr_str = ProcessSemList(expr_bucket->first_child, NULL, error_code);
      if (error_code) {
        delete[] expr_str;
        return NULL;
      }
    } else
      TCI_ASSERT(0);

    if (lower_bucket && lower_bucket->first_child) {
      lower_str =
        Subst2Estr(subst_info, true, expr_bucket, upper_bucket, error_code);
      if (error_code) {
        delete[] expr_str;
        return NULL;
      }
    } else
      TCI_ASSERT(0);

    if (upper_bucket && upper_bucket->first_child) {
      upper_str =
        Subst2Estr(subst_info, false, expr_bucket, lower_bucket, error_code);
      if (error_code) {
        delete[] expr_str;
        delete[] lower_str;
        return NULL;
      }
    }

    size_t bln = strlen(db_production) + 1;
    if (expr_str)
      bln += strlen(expr_str);
    if (lower_str)
      bln += strlen(lower_str);
    if (upper_str) {
      bln += strlen(expr_str);
      bln += strlen(upper_str);
    }
    rv = new char[bln];
    strcpy(rv, db_production);
    StrReplace(rv, bln, "%subst_list%", lower_str);
    StrReplace(rv, bln, "%expr%", expr_str);
    if (upper_str) {
      StrReplace(rv, bln, "%usubst_list%", upper_str);
      StrReplace(rv, bln, "%expr%", expr_str);
    }

    delete[] expr_str;
    delete[] lower_str;
    delete[] upper_str;
  } else                        // if ( db_production )
    TCI_ASSERT(0);

  return rv;
}

char *MuPADWrapper::MULTIARGFUNCtoEstr(SEMANTICS_NODE * subst_info,
                                       int& error_code)
{
  char *rv = NULL;
  error_code = 0;

  char *key = subst_info->contents; // "substitute";
  const char *db_production;
  U32 ID, subID;
  if (GetdBaseNamedRecord("SCHEMATA", key, &db_production, ID, subID)) {
    TCI_ASSERT(db_production);
    //substitute<uID17.0>(subs(%subst%,%expr%))

    char *subst_str = NULL;
    char *expr_str = NULL;

    BUCKET_REC *subst_bucket = FindBucketRec(subst_info->bucket_list,
                                             MB_SUBST_LOWER);
    if (subst_bucket && subst_bucket->first_child) {
      subst_str = ProcessSemList(subst_bucket->first_child, NULL, error_code);
      if (error_code) {
        delete[] subst_str;
        delete[] expr_str;
        return NULL;
      }
    } else
      TCI_ASSERT(0);

    BUCKET_REC *expr_bucket =
      FindBucketRec(subst_info->bucket_list, MB_UNNAMED);
    if (expr_bucket && expr_bucket->first_child) {
      expr_str = ProcessSemList(expr_bucket->first_child, NULL, error_code);
      if (error_code) {
        delete[] expr_str;
        return NULL;
      }
    } else
      TCI_ASSERT(0);

    size_t bln = strlen(db_production) + 1;
    if (subst_str)
      bln += strlen(subst_str);
    if (expr_str)
      bln += strlen(expr_str);
    rv = new char[bln];
    strcpy(rv, db_production);
    StrReplace(rv, bln, "%subst%", subst_str);
    StrReplace(rv, bln, "%expr%", expr_str);

    delete[] subst_str;
    delete[] expr_str;
  } else {
    TCI_ASSERT(!"db_production");
  }
  return rv;
}

char *MuPADWrapper::DERIVATIVEtoEstr(SEMANTICS_NODE * diff_info,
                                     int& error_code)
{
  char *rv = NULL;
  error_code = 0;

  char *key = diff_info->contents;  // "differentiate";
  const char *db_production;
  U32 ID, subID;
  if (GetdBaseNamedRecord("SCHEMATA", key, &db_production, ID, subID)) {
    TCI_ASSERT(db_production);
    //differentiate<uID16.0>tcidiff(%expr%,%var%)

    char *expr_str = NULL;
    char *var_str = NULL;
    U32 vs_ln = 0;

    BUCKET_REC *b_rover = diff_info->bucket_list;
    int tally = 0;
    while (b_rover) {
      if (b_rover->bucket_ID == MB_DIFF_VAR) {
        SEMANTICS_NODE *s_rover = b_rover->first_child;

        while (s_rover) {
          if (s_rover->semantic_type == SEM_TYP_VARIABLE
              || s_rover->semantic_type == SEM_TYP_POWERFORM) {
            char *tmp = GetDiffVarStr(s_rover, error_code);
            if (error_code) {
              delete[] expr_str;
              delete[] var_str;
              return NULL;
            }
            if (tmp) {
              if (tally)
                var_str = AppendStr2HeapStr(var_str, vs_ln, ",");
              var_str = AppendStr2HeapStr(var_str, vs_ln, tmp);
              delete[] tmp;
              tally++;
            }
          } else {
            TCI_ASSERT(0);
          }
          s_rover = s_rover->next;
        }
      }
      b_rover = b_rover->next;
    }
    if (vs_ln == 0) {
      BUCKET_REC *func_bucket = FindBucketRec(diff_info->bucket_list,
                                              MB_UNNAMED);
      if (func_bucket && func_bucket->first_child) {
        SEMANTICS_NODE *s_func = func_bucket->first_child;

        BUCKET_REC *var_bucket = FindBucketRec(s_func->bucket_list,
                                               MB_UNNAMED);
        if (var_bucket && var_bucket->first_child) {
          SEMANTICS_NODE *s_var = var_bucket->first_child;
          char *tmp = GetDiffVarStr(s_var, error_code);
          if (error_code) {
            delete[] expr_str;
            delete[] var_str;
            return NULL;
          }
          if (tmp) {
            var_str = AppendStr2HeapStr(var_str, vs_ln, tmp);
            delete[] tmp;
          }
        }
      } else {
        TCI_ASSERT(0);
      }
    }
    // the variable of differentiation looks like
    //  "q1x", "q1x$2", "q1x,q1x", "q1x,q1y$2", etc.

    BUCKET_REC *nprimes_bucket = FindBucketRec(diff_info->bucket_list,
                                               MB_NPRIMES);
    if (nprimes_bucket) {
      SEMANTICS_NODE *s_number = nprimes_bucket->first_child;

      var_str = AppendStr2HeapStr(var_str, vs_ln, "$");
      var_str = AppendStr2HeapStr(var_str, vs_ln, s_number->contents);
    }
    // The expression, function, etc., that is differentiated
    //  is held in a "MB_UNNAMED" bucket in SYM_TYP_DIFFERENTIATIONs
    //  bucket_list

    BUCKET_REC *expr_bucket =
      FindBucketRec(diff_info->bucket_list, MB_UNNAMED);
    if (expr_bucket && expr_bucket->first_child) {

      SEMANTICS_NODE *s_expr = expr_bucket->first_child;
      expr_str = ProcessSemList(s_expr, NULL, error_code);
      if (error_code) {
        return NULL;
      }
    } else {
      TCI_ASSERT(0);
    }
    size_t bln = strlen(db_production) + 1;
    if (expr_str)
      bln += strlen(expr_str);
    if (var_str)
      bln += strlen(var_str);
    rv = new char[bln];
    strcpy(rv, db_production);
    StrReplace(rv, bln, "%expr%", expr_str);
    StrReplace(rv, bln, "%var%", var_str);

    delete[] expr_str;
    delete[] var_str;
  } else
    TCI_ASSERT(0);

  return rv;
}

char *MuPADWrapper::ODEBoundarytoEstr(SEMANTICS_NODE * diff_info,
                                      int& error_code)
{
  char *rv = NULL;
  error_code = 0;

  // The expression, function, etc., that is differentiated
  //  is held in a "MB_UNNAMED" bucket in SYM_TYP_DERIVATIVE's
  //  bucket_list

  char *func_str = NULL;
  char *args_str = NULL;
  BUCKET_REC *expr_bucket = FindBucketRec(diff_info->bucket_list, MB_UNNAMED);
  if (expr_bucket->first_child) {
    SEMANTICS_NODE *s_func = expr_bucket->first_child;
    func_str = GetEngNameForUserFunc(s_func);

    args_str = LISTtoEstr(s_func, NULL, NULL, ",", NULL, error_code);
    args_str = NestInParens(args_str, false);
    if (error_code)
      return rv;
  } else
    TCI_ASSERT(0);

  int n_primes = 1;
  BUCKET_REC *nprimes_bucket = FindBucketRec(diff_info->bucket_list,
                                             MB_NPRIMES);
  if (nprimes_bucket) {
    SEMANTICS_NODE *s_number = nprimes_bucket->first_child;
    n_primes = atoi(s_number->contents);
  } else {
    TCI_ASSERT(0);
  }
  const char *db_production = "D(%func%)";
  int tally = 0;
  while (tally < n_primes) {
    size_t bln = strlen(db_production) + 1;
    if (func_str)
      bln += strlen(func_str);
    rv = new char[bln];
    strcpy(rv, db_production);
    StrReplace(rv, bln, "%func%", func_str);
    delete[] func_str;
    func_str = rv;
    tally++;
  }

  if (args_str) {
    size_t zln = strlen(rv) + strlen(args_str);
    char *tmp = new char[zln + 1];
    strcpy(tmp, rv);
    strcat(tmp, args_str);
    delete[] rv;
    delete[] args_str;
    rv = tmp;
  }

  return rv;
}

char *MuPADWrapper::PRIMEStoEstr(SEMANTICS_NODE * diff_info, int& error_code)
{
  char *rv = NULL;
  error_code = 0;

  const char *key = "tciprime";
  const char *db_production;
  U32 ID, subID;
  if (GetdBaseNamedRecord("SCHEMATA", key, &db_production, ID, subID)) {
    TCI_ASSERT(db_production);
    //tciprime<uID24.0>tciprime(%func%,[%var%],%numprimes%)

    char *func_str = NULL;
    char *var_str = NULL;
    char *num_str = NULL;

    // The expression, function, etc., that is differentiated
    //  is held in a "MB_UNNAMED" bucket in SYM_TYP_DIFFERENTIATIONs
    //  bucket_list
    SEMANTICS_NODE *s_expr = NULL;

    BUCKET_REC *func_bucket =
      FindBucketRec(diff_info->bucket_list, MB_UNNAMED);
    if (func_bucket && func_bucket->first_child) {
      s_expr = func_bucket->first_child;
      func_str = ProcessSemList(s_expr, NULL, error_code);
      if (error_code)
        return NULL;
    } else
      TCI_ASSERT(0);

    // func_str =  "q1f(q1g(q1x))" OR "q1f(q1x)"  OR a variable "q1y"
    char *ptr = strchr(func_str, '(');
    if (ptr) {
      size_t zln = strlen(func_str);
      if (func_str[zln - 1] == ')')
        func_str[zln - 1] = 0;
      else
        TCI_ASSERT(0);
      zln = strlen(ptr);
      var_str = new char[zln];
      strcpy(var_str, ptr + 1);
      *ptr = 0;
    } else {
      // Here, Analyzer hasn't seen a "variable of differentiation"
      // We may be able to get it from the defstore, or we can fake it.

      if (s_expr->semantic_type == SEM_TYP_FUNCTION
          || s_expr->semantic_type == SEM_TYP_VARIABLE) {
        DefStore *defstore = curr_msr->GetDefStore();
        U32 engine_ID = curr_msr->GetEngineID();
        DefInfo *di =
          defstore->GetDefInfo(engine_ID, s_expr->canonical_ID);
        if (di && di->arg_list) {
          char user_name[128];
          char canonical_ID[128];
          char *ptr = strchr(di->arg_list, ',');
          if (ptr) {            // 
            size_t zln = ptr - di->arg_list;
            strncpy(user_name, di->arg_list, zln);
            user_name[zln] = 0;

            ptr++;
            char *ptr1 = strchr(ptr, ',');
            if (ptr1) {         // more than 1 variable??
              TCI_ASSERT(0);
              size_t zln = ptr - ptr1;
              strncpy(canonical_ID, ptr, zln);
              canonical_ID[zln] = 0;
            } else
              strcpy(canonical_ID, ptr);
          } else {
            TCI_ASSERT(0);
          }
          U32 frame_ID = diff_info->owner_ID;
          if (!frame_ID)        // give ownership to the client requesting
            frame_ID = client_ID; //  the current computation

          ClientAliases_REC *client_aliases = GetClientAliasesRec(frame_ID);
          NAME2AKA_REC *map = GetAKARec(client_aliases->AKAs_list,
                                        canonical_ID, NULL);
          if (map) {
            size_t zln = strlen(map->alias);
            var_str = new char[zln + 1];
            strcpy(var_str, map->alias);
          } else {
            var_str = GetEngNameForVar(frame_ID, user_name, canonical_ID);
          }
        } else {                // fake it    
          var_str = DuplicateString("t");
        }
      } else
        TCI_ASSERT(0);
    }

    BUCKET_REC *nprimes_bucket = FindBucketRec(diff_info->bucket_list,
                                               MB_NPRIMES);
    if (nprimes_bucket) {
      SEMANTICS_NODE *s_number = nprimes_bucket->first_child;
      size_t zln = strlen(s_number->contents);
      num_str = new char[zln + 1];
      strcpy(num_str, s_number->contents);
    }

    size_t bln = strlen(db_production) + 1;
    if (func_str)
      bln += strlen(func_str);
    if (var_str)
      bln += strlen(var_str);
    if (num_str)
      bln += strlen(num_str);
    rv = new char[bln];
    strcpy(rv, db_production);
    StrReplace(rv, bln, "%func%", func_str);
    StrReplace(rv, bln, "%var%", var_str);
    StrReplace(rv, bln, "%numprimes%", num_str);

    delete[] func_str;
    delete[] var_str;
    delete[] num_str;
  } else
    TCI_ASSERT(0);

  return rv;
}

char *MuPADWrapper::BIGOPtoEstr(SEMANTICS_NODE * bigop_snode,
                                int& error_code)
{
  char *rv = NULL;
  error_code = 0;
  global_sum_var[0] = 0;

  U32 ID = 4;                   // summation
  char *key = bigop_snode->contents;
  if (!strcmp(key, "&#x220f;"))
    ID = 6;                     // product
  U32 subID = (context_cmdtype == CC_DEFINITION) ? 2 : 1;

  const char *db_production;
  char name_buffer[80];
  if (GetdBaseIDRecord("BIGOPS", ID, subID, &db_production, name_buffer, 80)) {
    if (db_production) {
      //&#x2211;<uID4.1>tcisum(%operand%,%variable%,%lower%,%upper%)
      //&#x2211;<uID4.2>tcisum(%operand%,%variable%,%lower%..%upper%)
      char *operand_str = NULL;
      char *var_str = NULL;
      char *lower_str = NULL;
      char *upper_str = NULL;

      BUCKET_REC *l_bucket =
        FindBucketRec(bigop_snode->bucket_list, MB_LOWERLIMIT);
      BUCKET_REC *u_bucket =
        FindBucketRec(bigop_snode->bucket_list, MB_UPPERLIMIT);
      BUCKET_REC *v_bucket =
        FindBucketRec(bigop_snode->bucket_list, MB_SUM_VAR);

      if (l_bucket) {
        SEMANTICS_NODE *lim_exp_snode = l_bucket->first_child;
        if (lim_exp_snode) {
          lower_str = ProcessSemNode(lim_exp_snode, NULL, error_code);
          if (error_code) {
            delete[] lower_str;
            return NULL;
          }
        }
      } else
        TCI_ASSERT(0);

      if (u_bucket) {
        SEMANTICS_NODE *lim_exp_snode = u_bucket->first_child;
        if (lim_exp_snode) {
          upper_str = ProcessSemNode(lim_exp_snode, NULL, error_code);
          if (error_code) {
            delete[] upper_str;
            delete[] lower_str;
            return NULL;
          }
        }
      } else
        TCI_ASSERT(0);

      if (v_bucket) {
        SEMANTICS_NODE *lim_exp_snode = v_bucket->first_child;
        if (lim_exp_snode) {
          var_str = ProcessSemNode(lim_exp_snode, NULL, error_code);
          if (error_code) {
            delete[] upper_str;
            delete[] lower_str;
            delete[] var_str;
            return NULL;
          }
        }
        if (var_str)
          strcpy(global_sum_var, var_str);
      } else
        TCI_ASSERT(0);

      const char *ptr = strstr(db_production, "%operand%");
      if (ptr) {
        // big prefix operator takes an operand
        BUCKET_REC *operand_bucket =
          FindBucketRec(bigop_snode->bucket_list, MB_OPERAND);
        if (operand_bucket->first_child)
          operand_str = ProcessSemList(operand_bucket->first_child,
                                       NULL, error_code);
        if (error_code) {
          delete[] upper_str;
          delete[] lower_str;
          delete[] var_str;
          delete[] operand_str;
          return NULL;
        }
      }

      size_t bln = strlen(db_production) + 1;
      if (operand_str)
        bln += strlen(operand_str);
      if (var_str)
        bln += strlen(var_str);
      if (lower_str)
        bln += strlen(lower_str);
      if (upper_str)
        bln += strlen(upper_str);
      rv = new char[bln];
      strcpy(rv, db_production);
      StrReplace(rv, bln, "%operand%", operand_str);
      StrReplace(rv, bln, "%variable%", var_str);
      StrReplace(rv, bln, "%lower%", lower_str);
      StrReplace(rv, bln, "%upper%", upper_str);
      //&#x2211;<uID5.2>tcisum(%operand%,%variable%,%lower%,%upper%)

      delete[] operand_str;
      delete[] var_str;
      delete[] lower_str;
      delete[] upper_str;
    }
  } else
    TCI_ASSERT(0);

  return rv;
}

char* MuPADWrapper::INTEGRALtoEstr(SEMANTICS_NODE* bigop_snode, int& error_code)
{
  char* rv = NULL;
  error_code = 0;

  //eval        integral<uID5.1>tciint(%integrand%,%variable%,%lower%,%upper%)
  //evalf       integral<uID5.2>hold(int)(%integrand%,%variable%=%lower%..%upper%)
  //indefinite  integral<uID5.3>tciint(%integrand%%comma%%variable%)
  //indefinite  integral<uID5.4>hold(int)(%integrand%%comma%%variable%)
  //approximate integral<uID5.5>%integrand%,%variable%,%lower%..%upper%

  char* lower_str = NULL;
  char* upper_str = NULL;
  char* integrand_str = NULL;
  char* var_str = NULL;

  BUCKET_REC* l_bucket = FindBucketRec(bigop_snode->bucket_list, MB_LOWERLIMIT);

  if (l_bucket && l_bucket->first_child) {
    
    lower_str = ProcessSemList(l_bucket->first_child, NULL, error_code);
    
    if (error_code) {
      delete[] lower_str;
      return NULL;
    }

  }

  BUCKET_REC* u_bucket = FindBucketRec(bigop_snode->bucket_list, MB_UPPERLIMIT);

  if (u_bucket && u_bucket->first_child) {

    upper_str = ProcessSemList(u_bucket->first_child, NULL, error_code);

    if (error_code) {
      delete[] upper_str;
      delete[] lower_str;
      return NULL;
    }
  }

  BUCKET_REC* operand_bucket = FindBucketRec(bigop_snode->bucket_list, MB_OPERAND);

  if (operand_bucket && operand_bucket->first_child) {

    integrand_str = ProcessSemList(operand_bucket->first_child, NULL, error_code);

    if (error_code) {
      delete[] upper_str;
      delete[] lower_str;
      delete[] integrand_str;
      return NULL;
    }
  }

//   BUCKET_REC* v_bucket = FindBucketRec(bigop_snode->bucket_list, MB_INTEG_VAR);
// 
//   if (v_bucket && v_bucket->first_child) {
//     var_str = ProcessSemList(v_bucket->first_child, NULL, error_code);
// 
//     if (error_code) {
//       delete[] upper_str;
//       delete[] lower_str;
//       delete[] integrand_str;
//       delete[] var_str;
//       return NULL;
//     }
//   }

  U32 ID = 5;
  U32 subID = 1;
  if (l_bucket || u_bucket) {   // definite integral
    if (context_cmdtype == CC_APPROXINTEGRAL)
      subID = 5;
    else if (context_cmdtype == CC_HOLDINTEGRAL || context_cmdtype == CC_DEFINITION)
      subID = 2;
    else
      subID = 1;
  } else {                      // indefinite integral
    if (context_cmdtype == CC_HOLDINTEGRAL || context_cmdtype == CC_DEFINITION)
      subID = 4;
    else
      subID = 3;
  }

  const char* db_production;
  char name_buffer[80];

  if (GetdBaseIDRecord("BIGOPS", ID, subID, &db_production, name_buffer, 80)) {

    TCI_ASSERT(db_production);

    size_t bln = strlen(db_production);
//    if (integrand_str)
//      bln += strlen(integrand_str);
//    if (lower_str)
//      bln += strlen(lower_str);
//    if (upper_str)
//      bln += strlen(upper_str);
//     if (var_str)
//       bln += strlen(var_str);

    rv = new char[bln + 1];

    strcpy(rv, db_production);

//    StrReplace(rv, bln, "%integrand%", integrand_str);

    if (subID == 3 || subID == 4) {
      
      //const char* comma = var_str ? "," : "";
      //StrReplace(rv, bln, "%comma%", comma);

    } else {

      //StrReplace(rv, bln, "%lower%", lower_str);
      //StrReplace(rv, bln, "%upper%", upper_str);
    }

    //StrReplace(rv, bln, "%variable%", var_str);
  }

  //delete[] integrand_str;
  //delete[] lower_str;
  //delete[] upper_str;
  delete[] var_str;

  // We may be processing a multiple integral
  // n_integs = # of integral signs
  int n_integs = SemanticVariant2NumIntegrals(bigop_snode->variant);

  if (n_integs > 0 ) {

    // nExplicitDiffs = # of explicit differentials    
    int nExplicitDiffs = 0;
  BUCKET_REC* v_bucket = FindBucketRec(bigop_snode->bucket_list, MB_INTEG_VAR);
  BUCKET_REC* tmp = v_bucket;
  while (tmp){
       tmp = FindBucketRec(tmp -> next, MB_INTEG_VAR);
     ++nExplicitDiffs;
  }
  //--nExplicitDiffs;

    bool used_x = false;
    bool used_y = false;
    bool used_z = false;

  int nTimes = n_integs;

    while (n_integs > 0) {
       //indefinite integral<uID5.3>tciint(%integrand%%comma%%variable%)
     char* var_str = NULL;

       if (nExplicitDiffs > 0){

         if (v_bucket->first_child) {
            var_str = ProcessSemList(v_bucket->first_child, NULL, error_code);

            if (error_code) {
              delete[] rv;
              delete[] var_str;
              return NULL;
            }

      v_bucket = FindBucketRec(v_bucket -> next, MB_INTEG_VAR);

            if (StringEqual(var_str, "q1x"))
           used_x = true;
        else if (StringEqual(var_str, "q1y"))
           used_y = true;
        else if (StringEqual(var_str, "q1z"))
           used_z = true;
     }
     } else {
       if (!used_x) {
        var_str = DuplicateString("q1x");
      used_x = true;
            ClientAliases_REC* curr_client = GetClientAliasesRec(client_ID);
            curr_client->AKAs_list = PushNamingRecord("q1x", "mix", "x", Ilk_Var, curr_client->AKAs_list);
                     
       } else if (!used_y){
        var_str = DuplicateString("q1y");
      used_y = true;
     } else if (!used_z) {
        var_str = DuplicateString("q1z");
      used_z = true;
     }
     }
       


     
       size_t bln = strlen(db_production);
       if (rv)
         bln += strlen(rv);
       if (var_str)
         bln += strlen(var_str);
       if (integrand_str)
         bln += strlen(integrand_str);
       if (lower_str)
         bln += strlen(lower_str);
       if (upper_str)
         bln += strlen(upper_str);


       char* new_rv = new char[bln + 1];
       strcpy(new_rv, db_production);

       if (nTimes == n_integs){
            // First time 

         StrReplace(new_rv, bln, "%integrand%", integrand_str);  
           StrReplace(new_rv, bln, "%lower%", lower_str);
           StrReplace(new_rv, bln, "%upper%", upper_str);

       delete[] integrand_str;
       integrand_str = 0;    
           delete[] lower_str;
       lower_str = 0;
           delete[] upper_str;
       upper_str = 0;


     } else {
         // Subsequent times...

           StrReplace(new_rv, bln, "%integrand%", rv);      
       }

       StrReplace(new_rv, bln, "%comma%", ",");
       StrReplace(new_rv, bln, "%variable%", var_str);

       delete[] rv;
       delete[] var_str;

       rv = new_rv;

       n_integs--;
     nExplicitDiffs--;

    }                           // loop thru multiple integral

  }

  return rv;
}

char* MuPADWrapper::OPERATORtoEstr(SEMANTICS_NODE * sem_op_node,
                                   const char *modulus, bool & is_infix,
                                   int& error_code)
{
  char *rv = NULL;

  if (sem_op_node->bucket_list) {
    // Reverse Polish forms carry operands as children
    rv = POLISHOPtoEstr(sem_op_node, modulus, is_infix, error_code);
  } else {
    // We still use infix in a few cases
    bool op_found = false;

    char *key = sem_op_node->contents;
    const char *db_maple_op;
    U32 ID, subID;

    if (GetdBaseNamedRecord("INFIXOPS", key, &db_maple_op, ID, subID)) {
      op_found = true;
      is_infix = true;
      if (db_maple_op && *db_maple_op) {
        size_t zln = strlen(db_maple_op);
        char ch = db_maple_op[0];
        if (ch >= 'a' && ch <= 'z') { // " union "
          zln += 2;
          rv = new char[zln + 1];
          rv[0] = ' ';
          strcpy(rv + 1, db_maple_op);
          rv[zln - 1] = ' ';
          rv[zln] = 0;
        } else {                // "+"
          rv = new char[zln + 1];
          strcpy(rv, db_maple_op);
        }
      }
    } else {
      if (GetdBaseNamedRecord("FUNCTIONS", key, &db_maple_op, ID, subID)) {
        op_found = true;
        TCI_ASSERT(0);
        // Operators should be distinguished from function by this point!
        const char *ptr = strstr(db_maple_op, "%val%");
        if (ptr)
          rv = PROBFUNCtoEstr(sem_op_node, db_maple_op, ID, error_code);
        else
          rv = FUNCTIONtoEstr(sem_op_node, error_code);

      } else
        // <mo>gcd</mo> - > tcigcd(35,15,65)
        TCI_ASSERT(0);
    }
    if (!op_found) {
      RecordMsg(msg_list, MSG_UNSUPPORTED_OPERATOR, key);
      error_code = CR_failure;
    }
  }

  return rv;
}

char *MuPADWrapper::INTERVALtoEstr(SEMANTICS_NODE * interval_info,
                                   int& error_code, bool bUseIntervalForm)
{
  char *zh_rv = NULL;
  error_code = 0;

  char *key = interval_info->contents;
  const char *db_production;
  const char *our_template = NULL;
  U32 ID, subID;
  GetdBaseNamedRecord("SCHEMATA", key, &db_production, ID, subID);
  if (bUseIntervalForm)
  {
    if (interval_info->variant == SNV_InclExclInterval)
      our_template = "Dom::Interval([%lower%],%upper%)";
    else if (interval_info->variant == SNV_InclInclInterval)
      our_template = "Dom::Interval([%lower%],[%upper%])";
    else if (interval_info->variant == SNV_ExclInclInterval)
      our_template = "Dom::Interval(%lower%,[%upper%])";
    else
      our_template = "Dom::Interval(%lower%,%upper%)";
  }
  else if (db_production)
  {
    our_template = db_production;
  }
//  if (db_production) {          // interval<uID33.0>%lower%..%upper%
  if (our_template) {
    char *lower_str = NULL;
    char *upper_str = NULL;

    BUCKET_REC *l_b =
      FindBucketRec(interval_info->bucket_list, MB_INTERVAL_START);
    if (l_b && l_b->first_child) {
      SEMANTICS_NODE *curr_handle = l_b->first_child;
      lower_str = ProcessSemList(curr_handle, NULL, error_code);
      if (error_code) {
        delete[] lower_str;
        return NULL;
      }
    }
    BUCKET_REC *u_b =
      FindBucketRec(interval_info->bucket_list, MB_INTERVAL_END);
    if (u_b && u_b->first_child) {
      SEMANTICS_NODE *curr_handle = u_b->first_child;
      upper_str = ProcessSemList(curr_handle, NULL, error_code);
      if (error_code) {
        delete[] lower_str;
        delete[] upper_str;
        return NULL;
      }
    }

    if (lower_str && upper_str) {
      size_t bln =
        strlen(our_template) + strlen(lower_str) + strlen(upper_str);
      zh_rv = new char[bln];
      strcpy(zh_rv, our_template);
      StrReplace(zh_rv, bln, "%lower%", lower_str);
      StrReplace(zh_rv, bln, "%upper%", upper_str);
    }
    delete[] lower_str;
    delete[] upper_str;
  }

  return zh_rv;
}

char *MuPADWrapper::FRACTIONtoEstr(SEMANTICS_NODE * frac_info,
                                   const char *key, int& error_code)
{
  char *rv = NULL;
  error_code = 0;

  const char *db_production;
  U32 ID, subID;
  GetdBaseNamedRecord("SCHEMATA", key, &db_production, ID, subID);

  if (db_production) {          // fraction<uID2.0>(%numerator%/%denominator%)
    char *num_str = NULL;
    char *den_str = NULL;

    BUCKET_REC *bucket = FindBucketRec(frac_info->bucket_list, MB_NUMERATOR);
    if (bucket && bucket->first_child) {
      num_str = ProcessSemList(bucket->first_child, NULL, error_code);
      if (error_code) {
        delete[] num_str;
        return NULL;
      }
      num_str = NestInParens(num_str, false);
    }

    bucket = FindBucketRec(frac_info->bucket_list, MB_DENOMINATOR);
    if (bucket && bucket->first_child) {
      den_str = ProcessSemList(bucket->first_child, NULL, error_code);
      if (error_code) {
        delete[] num_str;
        delete[] den_str;
        return NULL;
      }
      den_str = NestInParens(den_str, false);
    }

    if (num_str && den_str) {
      size_t bln = strlen(db_production) + strlen(num_str) + strlen(den_str);
      rv = new char[bln];
      strcpy(rv, db_production);
      StrReplace(rv, bln, "%numerator%", num_str);
      StrReplace(rv, bln, "%denominator%", den_str);
    }
    delete[] num_str;
    delete[] den_str;
  } else {
    TCI_ASSERT(0);
  }

  return rv;
}

char *MuPADWrapper::POWERFORMtoEstr(SEMANTICS_NODE * s_powerform,
                                    bool no_surds, const char *modulus,
                                    int& error_code)
{
  char *rv = NULL;
  error_code = 0;

  // look for special case where "exponential e" is the base

  bool done = false;
  BUCKET_REC *base_bucket = FindBucketRec(s_powerform->bucket_list,
                                          MB_SCRIPT_BASE);
  if (base_bucket && base_bucket->first_child) {
    SEMANTICS_NODE *s_base = base_bucket->first_child;
    if (s_base->semantic_type == SEM_TYP_UCONSTANT
        && !strcmp(s_base->contents, "e")) {
      const char *db_production;
      U32 ID, subID;
      if (GetdBaseNamedRecord("SCHEMATA", "exp", &db_production, ID, subID)) {
        if (db_production) {
          char *arg_str = NULL;
          BUCKET_REC *script_bucket = FindBucketRec(s_powerform->bucket_list,
                                                    MB_SCRIPT_UPPER);
          if (script_bucket && script_bucket->first_child) {
            arg_str = ProcessSemList(script_bucket->first_child,
                                     NULL, error_code);
            if (error_code) {
              delete[] arg_str;
              return NULL;
            }
          }

          size_t bln = strlen(db_production);
          if (arg_str)
            bln += strlen(arg_str);
          rv = new char[bln];
          strcpy(rv, db_production);
          arg_str = NestInParens(arg_str, false); 
          StrReplace(rv, bln, "%exponent%", arg_str);
          delete[] arg_str;
          done = true;
        } else
          TCI_ASSERT(0);
      } else
        TCI_ASSERT(0);
    }
  }

  bool base_is_matrix = false;
  if (base_bucket && base_bucket->first_child) {
    SEMANTICS_NODE *s_base = base_bucket->first_child;
    if (!s_base->next) {
      if (s_base->semantic_type == SEM_TYP_TABULATION)
        base_is_matrix = true;
      else
        base_is_matrix = QueryEngineIsMatrix(s_base);
    }
  }

  if (!done && !no_surds && !base_is_matrix) {
    BUCKET_REC *script_bucket = FindBucketRec(s_powerform->bucket_list,
                                              MB_SCRIPT_UPPER);
    if (script_bucket && script_bucket->first_child) {
      SEMANTICS_NODE *s_power = script_bucket->first_child;
      if (s_power->semantic_type == SEM_TYP_FRACTION
          && s_power->next == NULL && s_power->bucket_list) {
        BUCKET_REC *num_bucket = FindBucketRec(s_power->bucket_list,
                                               MB_NUMERATOR);
        if (num_bucket && num_bucket->first_child) {
          SEMANTICS_NODE *s_num = num_bucket->first_child;
          if (s_num->semantic_type == SEM_TYP_NUMBER
              && s_num->next == NULL && !strcmp(s_num->contents, "1")) {
            BUCKET_REC *den_bucket = FindBucketRec(s_power->bucket_list,
                                                   MB_DENOMINATOR);
            if (den_bucket && den_bucket->first_child) {
              SEMANTICS_NODE *s_den = den_bucket->first_child;
              if (s_den->semantic_type == SEM_TYP_NUMBER
                  && s_den->next == NULL) {
                rv = POWERasSURDtoEstr(s_powerform, error_code);
                done = true;
              }
            }
          }
        }
      }
    }
  }

  if (!done && modulus) {
    rv = POWERMODtoEstr(s_powerform, modulus, error_code);
    if (error_code) {
      delete[] rv;
      return NULL;
    }
    done = true;
  }

  if (!done && base_is_matrix) {
    BUCKET_REC *script_bucket = FindBucketRec(s_powerform->bucket_list,
                                              MB_SCRIPT_UPPER);
    if (script_bucket && script_bucket->first_child) {
      SEMANTICS_NODE *s_power = script_bucket->first_child;
      char TorH;
      if (IsTransposeIndicator(s_power, TorH)) {
        rv = POWERTRANSPOSEtoEstr(s_powerform, TorH, error_code);
        if (error_code) {
          delete[] rv;
          return NULL;
        }
        done = true;
      }
    }
  }

  if (!done) {
    const char *key = base_is_matrix ? "tcipwr" : "superscript";
    const char *db_production;
    U32 ID, subID;
    if (GetdBaseNamedRecord("SCHEMATA", key, &db_production, ID, subID)) {
      TCI_ASSERT(db_production);  // superscript<uID1.0>%base%^%superscript%
      char *base_str = NULL;
      char *sup_str = NULL;

      BUCKET_REC *bucket =
        FindBucketRec(s_powerform->bucket_list, MB_SCRIPT_BASE);
      if (bucket && bucket->first_child) {
        base_str = ProcessSemList(bucket->first_child, modulus, error_code);
        if (error_code) {
          delete[] base_str;
          return NULL;
        }
      }

      bucket = FindBucketRec(s_powerform->bucket_list, MB_SCRIPT_UPPER);
      if (bucket && bucket->first_child) {
        sup_str = ProcessSemList(bucket->first_child, modulus, error_code);
        if (error_code) {
          delete[] base_str;
          delete[] sup_str;
          return NULL;
        }
      }

      base_str = NestInParens(base_str, false);
      sup_str = NestInParens(sup_str, false);

      if (base_str && sup_str) {
        size_t bln = strlen(db_production)
          + strlen(base_str) + strlen(sup_str);
        rv = new char[bln];
        strcpy(rv, db_production);
        StrReplace(rv, bln, "%base%", base_str);
        StrReplace(rv, bln, "%superscript%", sup_str);
      }
      delete[] base_str;
      delete[] sup_str;
    }
  }

  return rv;
}

char *MuPADWrapper::ROOTtoEstr(SEMANTICS_NODE * root_info, int& error_code)
{

  char *zh_rv = NULL;
  error_code = 0;

  bool use_power_form = false;
  if (use_power_form) {
    zh_rv = SURDasPOWERtoEstr(root_info, error_code);
  } else {
    const char *key = "surd";
    const char *db_production;
    U32 ID, subID;
    if (GetdBaseNamedRecord("SCHEMATA", key, &db_production, ID, subID)) {
      TCI_ASSERT(db_production);  // surd<uID3.0>(%base%,%radicand%)
      char *base_str = NULL;
      char *radi_str = NULL;

      BUCKET_REC *bucket = FindBucketRec(root_info->bucket_list,
                                         MB_ROOT_BASE);
      if (bucket && bucket->first_child) {
        base_str = ProcessSemList(bucket->first_child, NULL, error_code);
        if (error_code) {
          delete[] base_str;
          return NULL;
        }
      }

      bucket = FindBucketRec(root_info->bucket_list, MB_ROOT_EXPONENT);
      if (bucket && bucket->first_child) {
        radi_str = ProcessSemList(bucket->first_child, NULL, error_code);
        if (error_code) {
          delete[] base_str;
          delete[] radi_str;
          return NULL;
        }
      }

      if (base_str && radi_str) {
        size_t bln =
          strlen(db_production) + strlen(base_str) + strlen(radi_str);
        zh_rv = new char[bln];
        strcpy(zh_rv, db_production);
        StrReplace(zh_rv, bln, "%base%", base_str);
        StrReplace(zh_rv, bln, "%radicand%", radi_str);
      }
      delete[] base_str;
      delete[] radi_str;
    }
  }

  return zh_rv;
}

char *MuPADWrapper::SQROOTtoEstr(SEMANTICS_NODE * sqrt_info,
                                 bool use_power_form, int& error_code)
{
  char *zh_rv = NULL;
  error_code = 0;

  if (use_power_form) {
    zh_rv = SURDasPOWERtoEstr(sqrt_info, error_code);
  } else {
    const char *key = "sqrt";
    const char *db_production;
    U32 ID, subID;
    if (GetdBaseNamedRecord("SCHEMATA", key, &db_production, ID, subID)) {
      TCI_ASSERT(db_production);  // sqrt<uID3.0>sqrt(%base%)
      char *base_str = NULL;

      BUCKET_REC *bucket =
        FindBucketRec(sqrt_info->bucket_list, MB_ROOT_BASE);
      if (bucket && bucket->first_child) {
        base_str = ProcessSemList(bucket->first_child, NULL, error_code);
        if (error_code) {
          delete[] base_str;
          return NULL;
        }
      }
      if (base_str) {
        size_t bln = strlen(db_production) + strlen(base_str);
        zh_rv = new char[bln];
        strcpy(zh_rv, db_production);
        StrReplace(zh_rv, bln, "%base%", base_str);
      }
      delete[] base_str;
    }
  }

  return zh_rv;
}

char *MuPADWrapper::SURDasPOWERtoEstr(SEMANTICS_NODE * surd_info,
                                      int& error_code)
{
  char *zh_rv = NULL;
  error_code = 0;

  char *sup_str = NULL;
  const char *db_production;
  U32 ID, subID;
  if (GetdBaseNamedRecord("SCHEMATA", "fraction", &db_production, ID, subID)) {
    TCI_ASSERT(db_production);  // fraction<uID2.0>(%numerator%/%denominator%)
    char *num_str = "1";
    char *den_str = NULL;

    BUCKET_REC *bucket =
      FindBucketRec(surd_info->bucket_list, MB_ROOT_EXPONENT);
    if (bucket && bucket->first_child) {
      den_str = ProcessSemList(bucket->first_child, NULL, error_code);
      if (error_code) {
        delete[] den_str;
        return NULL;
      }
      den_str = NestInParens(den_str, false);
    } else
      den_str = "2";

    if (den_str) {
      size_t bln = strlen(db_production)
        + strlen(num_str) + strlen(den_str);
      sup_str = new char[bln];
      strcpy(sup_str, db_production);
      StrReplace(sup_str, bln, "%numerator%", num_str);
      StrReplace(sup_str, bln, "%denominator%", den_str);
    }
    if (den_str && bucket)
      delete[] den_str;

  } else
    TCI_ASSERT(0);

  if (GetdBaseNamedRecord
      ("SCHEMATA", "superscript", &db_production, ID, subID)) {
    TCI_ASSERT(db_production);  // superscript<uID1.0>%base%^%superscript%
    char *base_str = NULL;

    BUCKET_REC *base_bucket =
      FindBucketRec(surd_info->bucket_list, MB_ROOT_BASE);
    if (base_bucket && base_bucket->first_child) {
      base_str = ProcessSemList(base_bucket->first_child, NULL, error_code);
      if (error_code) {
        delete[] base_str;
        return NULL;
      }
    }

    if (base_str && sup_str) {
      base_str = NestInParens(base_str, false);
      sup_str = NestInParens(sup_str, false);

      size_t bln = strlen(db_production) + strlen(base_str) + strlen(sup_str);
      zh_rv = new char[bln];
      strcpy(zh_rv, db_production);
      StrReplace(zh_rv, bln, "%base%", base_str);
      StrReplace(zh_rv, bln, "%superscript%", sup_str);
    }
    delete[] base_str;
    delete[] sup_str;
  } else
    TCI_ASSERT(0);

  return zh_rv;
}

char *MuPADWrapper::CONJUGATEtoEstr(SEMANTICS_NODE * snode, int& error_code)
{
  char *rv = NULL;
  error_code = 0;

  const char *key = "conjugate";
  const char *db_production;
  U32 ID, subID;
  if (GetdBaseNamedRecord("SCHEMATA", key, &db_production, ID, subID)) {
    if (db_production) {        // conjugate<uID48.0>conjugate%arg%
      char *base_str = NULL;

      if (snode && snode->bucket_list) {
        base_str =
          ProcessSemList(snode->bucket_list->first_child, NULL, error_code);
        if (error_code) {
          delete[] base_str;
          return NULL;
        }
      }

      if (base_str) {
        base_str = NestInParens(base_str, false);
        size_t bln = strlen(db_production) + strlen(base_str);
        rv = new char[bln];
        strcpy(rv, db_production);
        StrReplace(rv, bln, "%arg%", base_str);

        delete[] base_str;
      }
    }
  }
  TCI_ASSERT(rv);
  return rv;
}

char *MuPADWrapper::TRANSPOSEtoEstr(SEMANTICS_NODE * snode, char TorH,
                                    int& error_code)
{
  char *rv = NULL;
  error_code = 0;

  const char *key = (TorH == 'T') ? "Transpose" : "Hermitian Transpose";
  const char *db_production;
  U32 ID, subID;
  if (GetdBaseNamedRecord("SCHEMATA", key, &db_production, ID, subID)) {
    if (db_production && *db_production) {
      // Transpose<uID47.0>linalg::transpose(%matrix%)
      char *m_str = NULL;

      if (snode && snode->bucket_list && snode->bucket_list->first_child) {
        SEMANTICS_NODE *s_matrix = snode->bucket_list->first_child;

        m_str = ProcessSemList(s_matrix, NULL, error_code);
        if (error_code) {
          delete[] m_str;
          return NULL;
        }

        if (m_str) {
          size_t bln = strlen(db_production) + strlen(m_str);
          rv = new char[bln];
          strcpy(rv, db_production);
          StrReplace(rv, bln, "%matrix%", m_str);
          delete[] m_str;
        }
      } else
        TCI_ASSERT(0);
    } else
      TCI_ASSERT(0);
  } else
    TCI_ASSERT(0);

  return rv;
}

char *MuPADWrapper::TABULATIONtoEstr(SEMANTICS_NODE * table_info,
                                     const char *modulus, int& error_code)
{
  char *rv = NULL;
  error_code = 0;

  char *key = table_info->contents;

  const char *db_production;
  U32 ID, subID;
  if (GetdBaseNamedRecord("SCHEMATA", key, &db_production, ID, subID)) {
    TCI_ASSERT(db_production && *db_production);
    //matrix<uID6.0>SWPmatrix(%nrows%,%ncols%,[%row_list%])

    char *row_list_str = NULL;
    U32 cln = 0;

    const char *ptr = strstr(db_production, "%row_list%");
    if (ptr) {
      // For MuPAD, SWPmatrix takes a list of rows

      U32 nrows = table_info->nrows;
      U32 ncols = table_info->ncols;

      int err_code = 0;
      int row_counter = 0;
      BUCKET_REC *bucket_rover = table_info->bucket_list;
      while (row_counter < nrows) {
        if (row_counter)
          row_list_str = AppendStr2HeapStr(row_list_str, cln, ",");
        row_list_str = AppendStr2HeapStr(row_list_str, cln, "[");

        int col_counter = 0;
        while (col_counter < ncols) {
          if (col_counter)
            row_list_str = AppendStr2HeapStr(row_list_str, cln, ",");

          if (bucket_rover->first_child) {
            SEMANTICS_NODE *curr_handle = bucket_rover->first_child;
            char *obj_str = ProcessSemList(curr_handle, modulus, err_code);
            if (obj_str) {
              if (!err_code)
                row_list_str = AppendStr2HeapStr(row_list_str, cln, obj_str);
              delete[] obj_str;
            }
          }
          bucket_rover = bucket_rover->next;
          col_counter++;
        }
        row_list_str = AppendStr2HeapStr(row_list_str, cln, "]");
        row_counter++;
      }
      if (err_code) {
        error_code = err_code;
        delete[] row_list_str;
        return NULL;
      }
    }                           // end cells processing

    // Compose the return string
    size_t bln = strlen(db_production) + 1;
    if (row_list_str)
      bln += strlen(row_list_str);
    rv = new char[bln];
    strcpy(rv, db_production);
    char num_buffer[80];
    sprintf(num_buffer, "%d", table_info->nrows);
    StrReplace(rv, bln, "%nrows%", num_buffer);
    sprintf(num_buffer, "%d", table_info->ncols);
    StrReplace(rv, bln, "%ncols%", num_buffer);
    StrReplace(rv, bln, "%row_list%", row_list_str);

    delete[] row_list_str;
  } else {
    TCI_ASSERT(0);
  }
  return rv;
}

// Note that in SWP the data for a Simplex command is entered
//  into a matrix.  This matrix is re-typed as a simplex
//  in CompEngine.  The following functions handles all 5
//  Maple Simplex commands that are supported by SWP.

char *MuPADWrapper::SIMPLEXtoEstr(SEMANTICS_NODE * simplex_info,
                                  int& error_code)
{
  char *rv = NULL;
  error_code = 0;

  char *key = simplex_info->contents; // "Simplex Dual"
  const char *db_production;
  U32 ID, subID;
  if (!GetdBaseNamedRecord("SCHEMATA", key, &db_production, ID, subID)) {
    TCI_ASSERT(0);
    return rv;
  }
  if (db_production && *db_production) {
    // Simplex Dual<uID7.0>%expr%,{%condition_list%},%var%
    char *expr_str = NULL;
    char *conds_str = NULL;
    U32 conds_ln = 0;
    char *var_str = NULL;

    BUCKET_REC *cond_func_bucket =
      FindBucketRec(simplex_info->bucket_list, MB_SIMPLEX_EXPR);
    if (cond_func_bucket && cond_func_bucket->first_child) {
      expr_str =
        ProcessSemList(cond_func_bucket->first_child, NULL, error_code);
      if (error_code) {
        delete[] expr_str;
        return NULL;
      }
    }

    int err_code = 0;
    int condition_counter = 0;
    BUCKET_REC *bucket_rover = simplex_info->bucket_list;
    while (bucket_rover && !err_code) {
      if (bucket_rover->bucket_ID == MB_UNNAMED) {
        if (condition_counter != 0)
          conds_str = AppendStr2HeapStr(conds_str, conds_ln, ",");

        if (bucket_rover->first_child) {
          SEMANTICS_NODE *curr_handle = bucket_rover->first_child;
          char *obj_str = ProcessSemList(curr_handle, NULL, err_code);
          if (obj_str) {
            if (err_code != 1)
              conds_str = AppendStr2HeapStr(conds_str, conds_ln, obj_str);
            delete[] obj_str;
          }
        }
        condition_counter++;
      }
      bucket_rover = bucket_rover->next;
    }
    if (err_code) {
      error_code = err_code;
      return NULL;
    }
    const char *ptr = strstr(db_production, "%var%");
    if (ptr) {
      // Need to pass this in!
      var_str = new char[2];
      strcpy(var_str, "s");
    }
    // Compose the return string

    size_t bln = strlen(db_production);
    if (expr_str)
      bln += strlen(expr_str);
    if (conds_str)
      bln += strlen(conds_str);
    if (var_str)
      bln += strlen(var_str);
    rv = new char[bln + 1];
    strcpy(rv, db_production);

    StrReplace(rv, bln, "%expr%", expr_str);
    StrReplace(rv, bln, "%condition_list%", conds_str);
    StrReplace(rv, bln, "%var%", var_str);

    delete[] expr_str;
    delete[] conds_str;
    delete[] var_str;
  }

  return rv;
}

// SWP accepts a 1 column matrix with equations and conditions
//  like "x \in [1,2]" as input to Solve Numeric.
// I'm using SEM_TYP_NUMSYSTEM to carry this type of data.

char *MuPADWrapper::NUMSYSTEMtoEstr(SEMANTICS_NODE * numsys_info,
                                    int& error_code)
{
  char *rv = NULL;

  char *key = numsys_info->contents;  // "Numeric System"
  const char *db_production;
  U32 ID, subID;
  if (!GetdBaseNamedRecord("SCHEMATA", key, &db_production, ID, subID)) {
    TCI_ASSERT(0);
    error_code = CR_failure;
    return rv;
  }

  if (db_production && *db_production) {
    // Numeric System<uID35.0>%eqn_set%%comma%%cond_set%
    char *eqn_set_str = NULL;
    U32 eqns_ln = 0;
    char *cond_set_str = NULL;
    U32 conds_ln = 0;
    char *comma_str = NULL;

    int equation_counter = 0;
    BUCKET_REC *bucket_rover = numsys_info->bucket_list;
    while (bucket_rover) {
      if (bucket_rover->bucket_ID == MB_NSYS_EQUATION) {
        if (equation_counter == 0)
          eqn_set_str = AppendStr2HeapStr(eqn_set_str, eqns_ln, "{");
        else
          eqn_set_str = AppendStr2HeapStr(eqn_set_str, eqns_ln, ",");

        equation_counter++;
        if (bucket_rover->first_child) {
          char *row_str = ProcessSemList(bucket_rover->first_child,
                                         NULL, error_code);
          if (error_code) {
            delete[] row_str;
            delete[] eqn_set_str;
            return NULL;
          }
          eqn_set_str = AppendStr2HeapStr(eqn_set_str, eqns_ln, row_str);
          delete[] row_str;
        }
      }
      bucket_rover = bucket_rover->next;
    }
    if (equation_counter)
      eqn_set_str = AppendStr2HeapStr(eqn_set_str, eqns_ln, "}");

    int condition_counter = 0;
    bucket_rover = numsys_info->bucket_list;
    while (bucket_rover) {
      if (bucket_rover->bucket_ID == MB_NSYS_CONDITION) {
        if (condition_counter == 0)
          cond_set_str = AppendStr2HeapStr(cond_set_str, conds_ln, "{");
        else
          cond_set_str = AppendStr2HeapStr(cond_set_str, conds_ln, ",");

        condition_counter++;
        if (bucket_rover->first_child) {
          char *row_str = ProcessSemList(bucket_rover->first_child,
                                         NULL, error_code);
          if (error_code) {
            delete[] row_str;
            delete[] eqn_set_str;
            delete[] cond_set_str;
            return NULL;
          }
          cond_set_str = AppendStr2HeapStr(cond_set_str, conds_ln, row_str);
          delete[] row_str;
        }
      }
      bucket_rover = bucket_rover->next;
    }
    if (condition_counter) {
      comma_str = ",";
      cond_set_str = AppendStr2HeapStr(cond_set_str, conds_ln, "}");
    }

    // Compose the return string
    size_t bln = strlen(db_production);
    if (eqn_set_str)
      bln += strlen(eqn_set_str);
    if (comma_str)
      bln += strlen(comma_str);
    if (cond_set_str)
      bln += strlen(cond_set_str);
    rv = new char[bln + 1];
    strcpy(rv, db_production);

    StrReplace(rv, bln, "%eqn_set%", eqn_set_str);
    StrReplace(rv, bln, "%comma%", comma_str);
    StrReplace(rv, bln, "%cond_set%", cond_set_str);

    delete[] eqn_set_str;
    delete[] cond_set_str;
  } else {                      // no production for SEM_TYP_NUMSYSTEM
    TCI_ASSERT(0);
    error_code = CR_failure;
  }

  return rv;
}

char *MuPADWrapper::SOLVESYSTEMtoEstr(SEMANTICS_NODE * numsys_info,
                                      int& error_code)
{
  char *rv = NULL;
  error_code = 0;

  char *key = numsys_info->contents;  // "Solve System"
  const char *db_production;
  U32 ID, subID;
  if (!GetdBaseNamedRecord("SCHEMATA", key, &db_production, ID, subID)) {
    TCI_ASSERT(0);
    error_code = CR_failure;
    return rv;
  }

  if (db_production && *db_production) {
    // Solve System<uID60.0>%coeff_matrix%%comma%%col_matrix%
    char *coeff_matrix_str = NULL;
    char *comma_str = ",";
    char *col_matrix_str = NULL;

    BUCKET_REC *bucket =
      FindBucketRec(numsys_info->bucket_list, MB_SYSTEM_COEFFICIENTS);
    if (bucket && bucket->first_child) {
      coeff_matrix_str =
        ProcessSemList(bucket->first_child, NULL, error_code);
      if (error_code) {
        delete[] coeff_matrix_str;
        return NULL;
      }
    }

    bucket = FindBucketRec(numsys_info->bucket_list, MB_SYSTEM_VALUES);
    if (bucket && bucket->first_child) {
      col_matrix_str = ProcessSemList(bucket->first_child, NULL, error_code);
      if (error_code) {
        delete[] coeff_matrix_str;
        delete[] col_matrix_str;
        return NULL;
      }
    }
    // Compose the return string
    size_t bln = strlen(db_production);
    if (coeff_matrix_str)
      bln += strlen(coeff_matrix_str);
    if (comma_str)
      bln += strlen(comma_str);
    if (col_matrix_str)
      bln += strlen(col_matrix_str);
    rv = new char[bln + 1];
    strcpy(rv, db_production);

    StrReplace(rv, bln, "%coeff_matrix%", coeff_matrix_str);
    StrReplace(rv, bln, "%comma%", comma_str);
    StrReplace(rv, bln, "%col_matrix%", col_matrix_str);

    delete[] coeff_matrix_str;
    delete[] col_matrix_str;
  } else {                      // no production for SEM_TYP_NUMSYSTEM
    TCI_ASSERT(0);
    error_code = CR_failure;
  }

  return rv;
}

// I'm treating the inputs to "Solve ODE" as a special semantic type.
// In SWP an ODE command that includes boundary conditions is entered
//  in a 1 column matrix.  Such matrices are re-typed as ODEs in CompEngine.
// The engine database holds schemata that are used to construct
//  engine strings for these types.

char *MuPADWrapper::ODEtoEstr(SEMANTICS_NODE * ODE_info, int& error_code)
{
  char *rv = NULL;
  error_code = 0;

  char *key = ODE_info->contents; // "ODE EXACT", "ODE Laplace", etc.
  const char *db_production;
  U32 ID, subID;
  if (GetdBaseNamedRecord("SCHEMATA", key, &db_production, ID, subID)) {
    TCI_ASSERT(db_production && *db_production);
    //ODE Exact<uID20.0>{%exp_list%},{%sfor_func%}
    //ODE Laplace<uID21.0>{%exp_list%},{%sfor_func%},method=laplace
    //ODE Numeric<uID22.0>{%exp_list%},[%ODEfunc_list%],[%func_names%],%magic_num%
    //ODE Series<uID23.0>{%exp_list%},{%sfor_func%}

    char *expr_list_str = NULL;
    U32 expr_ln = 0;
    char *sfor_func_str = NULL;
    U32 sfor_ln = 0;

    int err_code = 0;
    int eqn_counter = 0;

    // Process all MB_ODEFUNCs first.  This generates the engine name
    //  for the ODEfunc, "q1yODE(q1x)", and pushes it onto the name stack
    //  as the name for "y".  As the engine string for the ODE is generated
    //  lookups for "y" will find q1yODE as required.
    BUCKET_REC *bucket_rover = ODE_info->bucket_list;
    while (bucket_rover && err_code != 1) {
      if (bucket_rover->first_child) {
//        SEMANTICS_NODE *s_eqn = bucket_rover->first_child;
        if (bucket_rover->bucket_ID == MB_ODEFUNC)
          ODEFuncName2eStr(bucket_rover->first_child, &sfor_func_str,
                           sfor_ln);
      }
      bucket_rover = bucket_rover->next;
    }

    bucket_rover = ODE_info->bucket_list;
    while (bucket_rover && err_code != 1) {
      if (bucket_rover->first_child) {
        if (bucket_rover->bucket_ID != MB_ODEFUNC) {
          if (bucket_rover->bucket_ID == MB_BOUNDARY_CONDITION)
            context_cmdtype = CC_ODE_CONDITION;
          else
            context_cmdtype = CC_ODE;

          SEMANTICS_NODE *s_eqn = bucket_rover->first_child;
          char *obj_str = ProcessSemList(s_eqn, NULL, err_code);
          if (obj_str) {
            if (err_code != 1) {
              if (eqn_counter != 0)
                expr_list_str =
                  AppendStr2HeapStr(expr_list_str, expr_ln, ",");
              expr_list_str =
                AppendStr2HeapStr(expr_list_str, expr_ln, obj_str);
              eqn_counter++;
            }
            delete[] obj_str;
          }
        }
      }

      bucket_rover = bucket_rover->next;
    }

    if (err_code) {
      error_code = err_code;
    } else {                    // Compose the return string
      size_t bln = strlen(db_production);
      if (expr_list_str)
        bln += strlen(expr_list_str);
      if (sfor_func_str)
        bln += strlen(sfor_func_str);
      rv = new char[bln + 1];
      strcpy(rv, db_production);

      StrReplace(rv, bln, "%exp_list%", expr_list_str);
      StrReplace(rv, bln, "%sfor_func%", sfor_func_str);
    }
    delete[] expr_list_str;
    delete[] sfor_func_str;
  } else
    TCI_ASSERT(0);

  return rv;
}

char *MuPADWrapper::ODENumerictoEstr(SEMANTICS_NODE * ODE_info,
                                     int& error_code)
{
  char *rv = NULL;
  error_code = 0;

  char *key = ODE_info->contents; // "ODE Numeric"
  const char *db_production;
  U32 ID, subID;
  if (GetdBaseNamedRecord("SCHEMATA", key, &db_production, ID, subID)) {
    TCI_ASSERT(db_production && *db_production);
    //ODE Numeric<uID22.0>{%exp_list%},[%ODEfunc_list%],[%func_names%],%magic_num%

    char *exp_list_str = NULL;
    U32 exp_ln = 0;
    char *ODEfunc_list_str = NULL;
    U32 ODEfunc_ln = 0;
    char *func_names_str = NULL;
    U32 names_ln = 0;
    char magic_num_str[80];
    magic_num_str[0] = 0;

    int err_code = 0;
    int eqn_counter = 0;
    int procs_defined = 0;

    // Process all MB_ODEFUNCs first.
    BUCKET_REC *bucket_rover = ODE_info->bucket_list;
    while (bucket_rover && err_code != 1) {
      if (bucket_rover->first_child) {
//        SEMANTICS_NODE *s_eqn = bucket_rover->first_child;
        if (bucket_rover->bucket_ID == MB_ODEFUNC) {
          ODEFuncName2eStr(bucket_rover->first_child,
                           &ODEfunc_list_str, ODEfunc_ln);
          if (NumericFuncName2eStr(bucket_rover->first_child,
                                   &func_names_str, names_ln))
            procs_defined++;
        }
      }
      bucket_rover = bucket_rover->next;
    }

    bucket_rover = ODE_info->bucket_list;
    while (bucket_rover && err_code != 1) {
      if (bucket_rover->first_child) {
        if (bucket_rover->bucket_ID != MB_ODEFUNC) {
          if (bucket_rover->bucket_ID == MB_BOUNDARY_CONDITION)
            context_cmdtype = CC_ODE_CONDITION;
          else
            context_cmdtype = CC_ODE;

          SEMANTICS_NODE *s_eqn = bucket_rover->first_child;
          char *obj_str = ProcessSemList(s_eqn, NULL, err_code);
          if (obj_str) {
            if (err_code != 1) {
              if (eqn_counter != 0)
                exp_list_str = AppendStr2HeapStr(exp_list_str, exp_ln, ",");
              exp_list_str = AppendStr2HeapStr(exp_list_str, exp_ln, obj_str);
              eqn_counter++;
            }
            delete[] obj_str;
          }
        }
      }
      bucket_rover = bucket_rover->next;
    }

    ODE_num_procnum += procs_defined;
    StrFromInt(ODE_num_procnum, magic_num_str);

    if (err_code) {
      error_code = err_code;
    } else {                    // Compose the return string
      size_t bln = strlen(db_production);

      if (exp_list_str)
        bln += strlen(exp_list_str);
      if (ODEfunc_list_str)
        bln += strlen(ODEfunc_list_str);
      if (func_names_str)
        bln += strlen(func_names_str);
      bln += strlen(magic_num_str);
      rv = new char[bln + 1];
      strcpy(rv, db_production);

      StrReplace(rv, bln, "%exp_list%", exp_list_str);
      StrReplace(rv, bln, "%ODEfunc_list%", ODEfunc_list_str);
      StrReplace(rv, bln, "%func_names%", func_names_str);
      StrReplace(rv, bln, "%magic_num%", magic_num_str);
    }
    delete[] exp_list_str;
    delete[] ODEfunc_list_str;
    delete[] func_names_str;
  } else
    TCI_ASSERT(0);

  return rv;
}

/************** Start Alias handling ****************************/

ClientAliases_REC *MuPADWrapper::CreateClientRecord(U32 client_num)
{
  ClientAliases_REC *rv = new ClientAliases_REC();
  rv->next = NULL;
  rv->client_ID = client_num;
  rv->AKAs_list = NULL;
  rv->next_var_number = 0x2000;
  rv->next_func_number = 0x2000;
  return rv;
}

// Clients have a list of "naming records", each holding
//  the source (or a generated) name for a variable or function
//  among with the engine name for this object.
// When we analyze engine return strings, the list is used
//  to determine the source name that will be passed back
//  to identify the object to the caller.

NAME2AKA_REC *MuPADWrapper::PushNamingRecord(const char *eng_name,
                                             const char *canon_name,
                                             const char *user_name,
                                             NameIlk obj_ilk,
                                             NAME2AKA_REC * aka_list)
{ 
  NAME2AKA_REC *rv = aka_list;

  if (eng_name) {
    NAME2AKA_REC *new_node = new NAME2AKA_REC();
    new_node->next = aka_list;

    size_t zln = strlen(eng_name);
    char *tmp = new char[zln + 1];
    strcpy(tmp, eng_name);
    new_node->alias = tmp;

    if (canon_name) {
      zln = strlen(canon_name);
      tmp = new char[zln + 1];
      strcpy(tmp, canon_name);
      new_node->src_canonical_name = tmp;
    } else
      new_node->src_canonical_name = NULL;

    if (user_name && *user_name) {
      zln = strlen(user_name);
      tmp = new char[zln + 1];
      strcpy(tmp, user_name);
      new_node->user_name = tmp;
    } else
      new_node->user_name = NULL;

    new_node->obj_ilk = obj_ilk;
    rv = new_node;
  } else
    TCI_ASSERT(0);

  return rv;
}

void MuPADWrapper::DisposeAKAsList(NAME2AKA_REC * aka_list)
{
  NAME2AKA_REC *rover = aka_list;
  while (rover) {
    NAME2AKA_REC *del = rover;
    rover = rover->next;
    delete[] del->alias;
    delete[] del->src_canonical_name;
    delete[] del->user_name;
    delete del;
  }
}

NAME2AKA_REC *MuPADWrapper::GetAKARec(NAME2AKA_REC * aka_list,
                                      const char *canonical_targ,
                                      const char *user_targ)
{
  NAME2AKA_REC *rover = aka_list;
  while (rover) {
    if (canonical_targ && rover->src_canonical_name &&
        !strcmp(rover->src_canonical_name, canonical_targ)) {
      return rover;
    }
    if (user_targ && rover->user_name && !strcmp(rover->user_name, user_targ)) {
      return rover;
    }
    rover = rover->next;
  }

  return NULL;
}

/************** End Alias handling *****************************/

bool MuPADWrapper::LoadPackages(char *new_packages)
{
  bool rv = true;
  TCI_ASSERT(!"We don't load packages in MuPAD.");
  return rv;
}

// Note that the following function creates and returns a new
//  "ClientAliases_REC" for a client that doesn't have one.

ClientAliases_REC *MuPADWrapper::GetClientAliasesRec(U32 targ_client_ID)
{
  ClientAliases_REC *rover = client_list;
  while (rover) {
    if (rover->client_ID == targ_client_ID)
      break;
    rover = rover->next;
  }

  if (!rover) {
    rover = CreateClientRecord(targ_client_ID);
    rover->next = client_list;
    client_list = rover;
  }

  return rover;
}

char *MuPADWrapper::CreateEngName(U32 client_ID,
                                  const char *obj_canonical_name,
                                  const char *preferred_eng_name,
                                  bool use_alias, NameIlk function_ilk)
{
  ClientAliases_REC *curr_client = GetClientAliasesRec(client_ID);

  char mangled_name[256];
  if (use_alias) {              // \alpha, a decorated letter, etc.
    if (preferred_eng_name && preferred_eng_name[0]) {
      if (function_ilk == Ilk_ODE) {
        sprintf(mangled_name, "q%d%sODE", client_ID, preferred_eng_name);
      } else
        sprintf(mangled_name, "q%d%s", client_ID, preferred_eng_name);
    } else {
      if (function_ilk == Ilk_Func) {
        sprintf(mangled_name, "q%df%d", client_ID,
                curr_client->next_func_number);
        curr_client->next_func_number++;
      } else if (function_ilk == Ilk_ODE) {
        sprintf(mangled_name, "q%d%sODE", client_ID, preferred_eng_name);
      } else {
        sprintf(mangled_name, "q%dv%d", client_ID,
                curr_client->next_var_number);
        curr_client->next_var_number++;
      }
    }
  } else {                      // x, a, s, u, etc.
    sprintf(mangled_name, "q%d%s", client_ID, preferred_eng_name);
  }

  size_t zln = strlen(mangled_name);
  char *eng_name = new char[zln + 1];
  strcpy(eng_name, mangled_name);

  curr_client->AKAs_list =
    PushNamingRecord(eng_name, obj_canonical_name, preferred_eng_name,
                     function_ilk, curr_client->AKAs_list);

  return eng_name;
}


// The following func needs more work.

char *MuPADWrapper::CanonicalNumberToEngineStr(const char *num_str,
                                               int& error_code)
{
  error_code = 0;

  size_t zln = strlen(num_str);
  char *rv = new char[zln + 1];
  int i = 0;

  const char *ptr = num_str;
  while (!error_code) {
    char ch = *ptr;
    if (!ch) {
      break;
    } else if (ch >= '0' && ch <= '9') {
      rv[i++] = ch;
    } else if (ch == '.') {
      rv[i++] = ch;
    } else if (ch == '-') {
      if (i == 0)
        rv[i++] = ch;
      else
        error_code = CR_failure;
    } else if (ch == '+') {
      if (i != 0)
        error_code = CR_failure;
    } else {
      error_code = CR_failure;
    }
    ptr++;
  }

  if (!error_code) {
    rv[i] = 0;
  } else {
    delete[] rv;
    rv = NULL;
  }

  return rv;
}

void MuPADWrapper::SetWrapperCmdContext(U32 cmd_ID, char *new_packages)
{
  context_cmdtype = CC_UNKNOWN;
  context_cmdtemplatetype = CT_UNKNOWN;
  context_datatype = CD_Real;

  switch (cmd_ID) {

  case CCID_Evaluate:
    context_cmdtype = CC_EVAL;
    context_cmdtemplatetype = CT_NORMAL;
    break;
  case CCID_Evaluate_Numerically:
    context_cmdtype = CC_EVAL_NUMERIC;
    context_cmdtemplatetype = CT_NORMAL;
    break;
  case CCID_Simplify:
    context_cmdtemplatetype = CT_NORMAL;
    break;

  case CCID_Combine_Exponentials:
  case CCID_Combine_Logs:
  case CCID_Combine_Powers:
  case CCID_Combine_Trig_Functions:
  case CCID_Combine_Arctan:
  case CCID_Combine_Hyp_Trig_Functions:
    context_cmdtemplatetype = CT_NORMAL;
    break;

  case CCID_Factor:
    context_cmdtemplatetype = CT_NORMAL;
    break;
  case CCID_Expand:
    context_cmdtemplatetype = CT_NORMAL;
    break;

  case CCID_Rewrite_Rational:
  case CCID_Rewrite_Float:
  case CCID_Rewrite_Mixed:
  case CCID_Rewrite_Exponential:
  case CCID_Rewrite_Factorial:
  case CCID_Rewrite_Gamma:
  case CCID_Rewrite_Logarithm:
  case CCID_Rewrite_sin_and_cos:
  case CCID_Rewrite_sinh_and_cosh:
  case CCID_Rewrite_sin:
  case CCID_Rewrite_cos:
  case CCID_Rewrite_tan:
  case CCID_Rewrite_arcsin:
  case CCID_Rewrite_arccos:
  case CCID_Rewrite_arctan:
  case CCID_Rewrite_arccot:
  case CCID_Rewrite_Polar:
  case CCID_Rewrite_Rectangular:
  case CCID_Rewrite_Normal_Form:
    context_cmdtemplatetype = CT_NORMAL;
    break;

  case CCID_Rewrite_Equations_as_Matrix:
    context_cmdtemplatetype = CT_EQN2MAT;
    break;
  case CCID_Rewrite_Matrix_as_Equations:
    context_cmdtemplatetype = CT_EQN2MAT;
    break;

  case CCID_Check_Equality:
    context_cmdtemplatetype = CT_NORMAL;
    break;
  case CCID_Solve_Exact:
    context_cmdtemplatetype = CT_SOLVEEXACT;
    break;
  case CCID_Solve_Integer:
    context_cmdtemplatetype = CT_NORMAL;
    break;
  case CCID_Solve_Numeric:
    context_cmdtemplatetype = CT_NORMAL;
    break;

  case CCID_Solve_Recursion:
    context_cmdtemplatetype = CT_NORMAL;
    break;

  case CCID_Polynomial_Collect:
    context_cmdtemplatetype = CT_COLLECT;
    break;

  case CCID_Polynomial_Divide: //Polynomial Divide tcidiv(%expr%);
    context_cmdtemplatetype = CT_EXPRandVAR;
    break;

  case CCID_Polynomial_Partial_Fractions:  //Polynomial Partial Fractions
    context_cmdtemplatetype = CT_EXPRandVAR;
    break;
  case CCID_Polynomial_Roots:  //Polynomial Roots
    context_cmdtemplatetype = CT_POLYROOTS;
    break;
  case CCID_Polynomial_Sort:   //Polynomial Sort
    context_cmdtemplatetype = CT_POLYSORT;
    break;
  case CCID_Polynomial_Companion_Matrix: //Polynomial Companion Matrix
    context_cmdtemplatetype = CT_COMPANIONMAT;
    break;

  case CCID_Calculus_Integrate_by_Parts: //Calculus Integrate by Parts...
    context_cmdtype = CC_HOLDINTEGRAL;
    context_cmdtemplatetype = CT_CALCBYPARTS;
    break;
  case CCID_Calculus_Change_Variable:  //Calculus Change Variable...
    context_cmdtype = CC_HOLDINTEGRAL;
    context_cmdtemplatetype = CT_CALCCHANGEVAR;
    break;

  case CCID_Calculus_Partial_Fractions:  //Calculus Partial Fractions
    context_cmdtemplatetype = CT_EXPRandVAR;
    break;
  case CCID_Calculus_Approximate_Integral: //Calculus Approximate Integral...
    context_cmdtype = CC_APPROXINTEGRAL;
    context_cmdtemplatetype = CT_CALCAPPROXINT;
    break;
  case CCID_Calculus_Plot_Approximate_Integral:  //Calculus Plot Approximate Integral
    context_cmdtemplatetype = CT_UNKNOWN;
    break;
  case CCID_Calculus_Find_Extrema: //Calculus Find Extrema
    context_cmdtemplatetype = CT_EXTREMA;
    break;
  case CCID_Calculus_Iterate:  //Calculus Iterate...
    context_cmdtemplatetype = CT_ITERATE;
    break;
  case CCID_Calculus_Implicit_Differentiation: //Calculus Implicit Differentiation...
    context_cmdtype = CC_IMPLICIT_DIFF;
    context_cmdtemplatetype = CT_EXPRandVAR;
    break;
  case CCID_Power_Series:      //Power Series...
    context_cmdtemplatetype = CT_PSERIES;
    break;

  case CCID_Solve_PDE:         //Solve PDE
    context_cmdtemplatetype = CT_UNKNOWN;
    break;

  case CCID_Solve_ODE_Exact:   //Solve ODE Exact
  case CCID_Solve_ODE_Laplace: //Solve ODE Laplace
  case CCID_Solve_ODE_Numeric: //Solve ODE Numeric
    context_cmdtype = CC_ODE;
    context_cmdtemplatetype = CT_NORMAL;
    break;

  case CCID_Solve_ODE_Series:  //Solve ODE Series
    context_cmdtype = CC_ODE;
    context_cmdtemplatetype = CT_ODESERIES;
    break;

  case CCID_Fourier_Transform: //Transforms Fourier
    context_cmdtemplatetype = CT_TRANSFORM;
    break;
  case CCID_Inverse_Fourier_Transform: //Transforms Inverse Fourier
    context_cmdtemplatetype = CT_TRANSFORM;
    break;
  case CCID_Laplace_Transform: //Transforms Laplace
    context_cmdtemplatetype = CT_TRANSFORM;
    break;
  case CCID_Inverse_Laplace_Transform: //Transforms Inverse Laplace
    context_cmdtemplatetype = CT_TRANSFORM;
    break;

  case CCID_Gradient:          //Gradient
    context_cmdtemplatetype = CT_VCALC;
    break;
  case CCID_Divergence:        //Divergence
    context_cmdtemplatetype = CT_VCALC;
    break;
  case CCID_Curl:              //Curl
    context_cmdtemplatetype = CT_VCALC;
    break;
  case CCID_Laplacian:         //Laplacian
    context_cmdtemplatetype = CT_VCALC;
    break;
  case CCID_Jacobian:          //Jacobian
    context_cmdtemplatetype = CT_VCALC;
    break;
  case CCID_Hessian:           //Hessian
    context_cmdtemplatetype = CT_VCALC;
    break;
  case CCID_Wronskian:         //Wronskian
    context_cmdtemplatetype = CT_WRONSKIAN;
    break;
  case CCID_Scalar_Potential:  //Scalar Potential
    context_cmdtemplatetype = CT_VCALC;
    break;
  case CCID_Vector_Potential:  //Vector Potential
    context_cmdtemplatetype = CT_VCALC;
    break;

  case CCID_Adjugate:          //Adjugate
    context_cmdtemplatetype = CT_NORMAL;
    break;
  case CCID_Characteristic_Polynomial: //Characteristic Polynomial
    context_cmdtemplatetype = CT_CHARACTERISTICP;
    break;
  case CCID_Cholesky_Decomposition:  //Cholesky Decomposition
    context_cmdtemplatetype = CT_NORMAL;
    break;
  case CCID_Column_Basis:      //Column Basis
    context_cmdtemplatetype = CT_NORMAL;
    break;
  case CCID_Concatenate:       //Concatenate
    context_cmdtemplatetype = CT_NORMAL;
    break;
  case CCID_Condition_Number:  //Condition Number
    context_cmdtemplatetype = CT_NORMAL;
    break;
  case CCID_Definiteness_Tests:  //Definiteness Tests
    context_cmdtemplatetype = CT_NORMAL;
    break;
  case CCID_Determinant:       //Determinant
    context_cmdtemplatetype = CT_NORMAL;
    break;
  case CCID_Eigenvalues:       //Eigenvalues
    context_cmdtemplatetype = CT_NORMAL;
    break;
  case CCID_Eigenvectors:      //Eigenvectors
    context_cmdtemplatetype = CT_NORMAL;
    break;

  case CCID_Fill_Matrix:       //Fill Matrix... randmatrix(%nrows%,%ncols%);
    context_cmdtemplatetype = CT_FILLMATRIX;
    break;

  case CCID_Fraction_Free_Gaussian_Elimination:  //Fraction-Free Gaussian Elimination
    context_cmdtemplatetype = CT_NORMAL;
    break;
  case CCID_Gaussian_Elimination:  //Gaussian Elimination
    context_cmdtemplatetype = CT_NORMAL;
    break;
  case CCID_Hermite_Normal_Form: //Hermite Normal Form
    context_cmdtemplatetype = CT_NORMAL;
    break;
  case CCID_Hermitian_Transpose: //Hermitian Transpose
    context_cmdtemplatetype = CT_NORMAL;
    break;
  case CCID_Inverse:           //Inverse
    context_cmdtemplatetype = CT_NORMAL;
    break;
  case CCID_Jordan_Form:       //Jordan Form
    context_cmdtemplatetype = CT_NORMAL;
    break;
  case CCID_Map_Entries:       //Map Entries...
    context_cmdtemplatetype = CT_MAPENTRIES;
    break;
  case CCID_Minimal_Polynomial:
    context_cmdtemplatetype = CT_MATRIXMINPOL;
    break;

  case CCID_Norm:              //Norm     norm(%s,2);
    context_cmdtemplatetype = CT_NORMAL;
    break;

  case CCID_Nullspace_Basis:   //Nullspace Basis
    context_cmdtemplatetype = CT_NORMAL;
    break;
  case CCID_Orthogonality_Test:  //Orthogonality Test
    context_cmdtemplatetype = CT_NORMAL;
    break;
  case CCID_Permanent:         //Permanent
    context_cmdtemplatetype = CT_NORMAL;
    break;
  case CCID_PLU_Decomposition: //PLU Decomposition
    context_cmdtemplatetype = CT_NORMAL;
    break;
  case CCID_QR_Decomposition:  //QR Decomposition
    context_cmdtemplatetype = CT_NORMAL;
    break;

  case CCID_Random_Matrix:     //Random Matrix... tcirandmat(5,5,1,-99..99);
    context_cmdtemplatetype = CT_RANDMATRIX;
    break;

  case CCID_Rank:              //Rank
    context_cmdtemplatetype = CT_NORMAL;
    break;
  case CCID_Rational_Canonical_Form: //Rational Canonical Form
    context_cmdtemplatetype = CT_NORMAL;
    break;
  case CCID_Reduced_Row_Echelon_Form:  //Reduced Row Echelon Form
    context_cmdtemplatetype = CT_NORMAL;
    break;

  case CCID_Reshape:           //Reshape...
    context_cmdtemplatetype = CT_RESHAPE;
    break;

  case CCID_Row_Basis:         //Row Basis
    context_cmdtemplatetype = CT_NORMAL;
    break;
  case CCID_Singular_Values:   //Singular Values
    context_cmdtemplatetype = CT_NORMAL;
    break;
  case CCID_Smith_Normal_Form: //Smith Normal Form
    context_cmdtemplatetype = CT_NORMAL;
    break;
  case CCID_Spectral_Radius:   //Spectral Radius
    context_cmdtemplatetype = CT_NORMAL;
    break;
  case CCID_Stack:             //Stack
    context_cmdtemplatetype = CT_NORMAL;
    break;
  case CCID_SVD:               //SVD
    context_cmdtemplatetype = CT_NORMAL;
    break;
  case CCID_Trace:             //Trace
    context_cmdtemplatetype = CT_NORMAL;
    break;
  case CCID_Transpose:         //Transpose
    context_cmdtemplatetype = CT_NORMAL;
    break;

  case CCID_Simplex_Dual:      //Simplex Dual
  case CCID_Simplex_Feasible:  //Simplex Feasible
  case CCID_Simplex_Maximize:  //Simplex Maximize
  case CCID_Simplex_Minimize:  //Simplex Minimize
  case CCID_Simplex_Standardize: //Simplex Standardize
    context_cmdtemplatetype = CT_NORMAL;
    break;

  case CCID_Fit_Curve_to_Data: //Fit Cucontext_cmdtemplatetypee to Data...
    context_cmdtemplatetype = CT_FITCURVE;
    break;

  case CCID_Random_Numbers:    //Random Numbers...
    context_cmdtemplatetype = CT_RANDOMNUM;
    break;

  case CCID_Mean:              //Mean
    context_cmdtemplatetype = CT_NORMAL;
    break;
  case CCID_Median:            //Median
    context_cmdtemplatetype = CT_NORMAL;
    break;
  case CCID_Mode:              //Mode
    context_cmdtemplatetype = CT_NORMAL;
    break;
  case CCID_Correlation:       //Correlation
    context_cmdtemplatetype = CT_NORMAL;
    break;
  case CCID_Covariance:        //Cov&ariance
    context_cmdtemplatetype = CT_NORMAL;
    break;
  case CCID_Geometric_Mean:    //Geometric Mean
    context_cmdtemplatetype = CT_NORMAL;
    break;
  case CCID_Harmonic_Mean:     //Harmonic Mean
    context_cmdtemplatetype = CT_NORMAL;
    break;
  case CCID_Mean_Deviation:    //Mean Deviation
    context_cmdtemplatetype = CT_NORMAL;
    break;

  case CCID_Moment:            //Moment...
    context_cmdtemplatetype = CT_MOMENT;
    break;

  case CCID_Quantile:          //Quantile...
    context_cmdtemplatetype = CT_QUANTILE;
    break;

  case CCID_Standard_Deviation:  //Standard Deviation
    context_cmdtemplatetype = CT_NORMAL;
    break;
  case CCID_Variance:          //Variance
    context_cmdtemplatetype = CT_NORMAL;
    break;

/*
    case 120 :  //Plot 2D Rectangular
    case 121 :  //Plot 2D Polar
    case 122 :  //Plot 2D Implicit
    case 123 :  //Plot 2D Parametric
    case 124 :  //Plot 2D Conformal
    case 125 :  //Plot 2D Gradient
    case 126 :  //Plot 2D Vector Field
    case 127 :  //Plot 2D ODE
    case 128 :  //Plot 2D Phase Plane
    case 129 :  //Plot 3D Rectangular
    case 130 :  //Plot 3D Cylindrical
    case 131 :  //Plot 3D Spherical
    case 132 :  //Plot 3D Implicit
    case 133 :  //Plot 3D Tube
    case 134 :  //Plot 3D Gradient
    case 135 :  //Plot 3D Vector Field
      context_cmdtemplatetype  =  CT_UNKNOWN;
  break;
*/

  case CCID_Define:            //DEFINE
    context_cmdtype = CC_DEFINITION;
    context_cmdtemplatetype = CT_NORMAL;
    break;
  case CCID_Undefine:          //UNDEFINE
    context_cmdtype = CC_UNDEFINE;
    context_cmdtemplatetype = CT_NORMAL;
    break;
/*
    case 138 :  //Show Definitions...
    case 139 :  //Clear Definitions
    case 140 :  //Save Definitions
    case 141 :  //Restore Definitions
    case 142 :  //Define Maple Name...
    case 143 :  //Settings...
    case 144 :  //Interpret
      context_cmdtemplatetype  =  CT_UNKNOWN;
  break;
*/
  case CCID_Factor_in_ring:    //Factor in ring<uID145.0>tcifactor(%s);
    context_cmdtype = CC_FACTOR_IN_RING;
    context_cmdtemplatetype = CT_NORMAL;
    break;

  case CCID_Solve_System:      //Solve System
    context_cmdtemplatetype = CT_NORMAL;
    break;

  case CCID_PassThru:
    context_cmdtemplatetype = CT_NORMAL;
    break;

  case CCID_PlotFuncCmd:
    context_cmdtype = CC_PLOTFUNC;
    context_cmdtemplatetype = CT_PLOTFUNC;
    break;

  case CCID_PlotFuncQuery:
    context_cmdtype = CC_PLOTFUNC;
    context_cmdtemplatetype = CT_PLOTQUERY;
    break;

  case CCID_GetVariables:
    context_cmdtemplatetype = CT_NORMAL;
  break;

  default:
    TCI_ASSERT(!"Unknown command ID.");
    break;
  }
}

//Evaluate<uID1.0>eval(%s);

char *MuPADWrapper::CreateStandardeStr(const char *eng_zstr,
                                       const char *cmd_template,
                                       int& error_code)
{
  char *rv = NULL;
  error_code = 0;

  if (eng_zstr) {
    if (cmd_template && *cmd_template) {
      size_t zln = strlen(cmd_template) + strlen(eng_zstr) + 10;
      rv = new char[zln];
      sprintf(rv, cmd_template, eng_zstr);
    } else {
      TCI_ASSERT(0);
      size_t zln = strlen(eng_zstr);
      rv = new char[zln + 1];
      strcpy(rv, eng_zstr);
    }
  }

  return rv;
}

// CT_SOLVEEXACT  :
//Solve Exact<uID26.0>tcisolve({%expr%});
//Solve Exact For<uID26.1>tcisolve({%expr%},{%which_var%});
// CT_POLYSORT    :
//Polynomial Sort<uID34.0>tcisort(%expr%);
//Polynomial Sort<uID34.1>tcisort(%expr%,{%which_var%});
// CT_EXPRandVAR  :
//Polynomial Partial Fractions<uID32.0>tciparfrac(%expr%);
//Polynomial Partial Fractions<uID32.1>tciparfrac(%expr%,%which_var%);
//Calculus Partial Fractions<uID38.0>tciparfrac(%expr%);
//Calculus Implicit Differentiation...<uID43.1>diff(%expr%,%which_var%);
// CT_POLYROOTS   :
//Polynomial Roots<uID33.0>tciroot(%expr%);
//Polynomial Roots<uID33.1>tciroot(%expr%,%which_var%);
// CT_MATRIXMINPOL:
//Minimal Polynomial<uID81.0>tciminpoly(%expr%);
//Minimal Polynomial<uID81.1>tciminpoly(%expr%,%which_var%);
// CT_COMPANIONMAT:
//Polynomial Companion Matrix<uID35.0>tcicompanion(%expr%);
//Polynomial Companion Matrix<uID35.1>tcicompanion(%expr%,%which_var%);

char *MuPADWrapper::CreateWHICHVAReStr(MathServiceRequest & msr, U32 cmd_ID,
                                       const char *eng_zstr, int& error_code)
{
  char *zh_rv = NULL;
  error_code = 0;

  if (eng_zstr) {
    if (cmd_ID) {
      const char *z_var = NULL;

      // Engine string formed here may require a subst for "%which_var%"
      // If msr is carrying "%which_var%" data in a param, we extract it here.
      U32 p_ID;
      U32 p_type;
      if (cmd_ID == CCID_Calculus_Implicit_Differentiation) {
        z_var = msr.GetParam(PID_ImplDiffIndepVar, p_type);
        p_ID = PID_ImplDiffIndepVar;
      } else {
        z_var = msr.GetParam(PID_needvarresult, p_type);
        p_ID = PID_needvarresult;
        if (!p_type) {
          z_var = msr.GetParam(PID_independentvar, p_type);
          p_ID = PID_independentvar;
        }
      }
      char op_name[80];         // Polynomial Partial Fractions
      const char *cmd_template = NULL;  // tciparfrac(%expr%,%which_var%);
      U32 sub_ID = p_type ? 1 : 0;
      //  what does the sub_ID do here??? 
      //       if (cmd_ID == CCID_PlotFunc2d) { //
      //         sub_ID = 0;                    //
     //       }                                //
      if (GetdBaseIDRecord("COMMANDS", cmd_ID, sub_ID, &cmd_template, op_name, 80)) {
        char *vars_str = NULL;
        U32 vars_ln = 0;

        if (p_type == zPT_ASCII_text) { // "a,b"
          // We shouldn't get here - params that carry variables should be markup
          TCI_ASSERT(0);
          int tally = 0;
          const char *ptr = z_var;
          while (*ptr) {

            if ((*ptr >= 'a' && *ptr <= 'z')
                || (*ptr >= 'A' && *ptr <= 'Z')) {
              // See if we already have an engine name for this variable.
              char var[2];
              var[0] = *ptr;
              var[1] = 0;
              ClientAliases_REC *client_aliases =
                GetClientAliasesRec(client_ID);
              NAME2AKA_REC *map =
                GetAKARec(client_aliases->AKAs_list, NULL, var);
              if (map) {
                if (IsDefined(map->alias))
                  error_code = CR_badindepvar;

                if (tally)
                  vars_str = AppendStr2HeapStr(vars_str, vars_ln, ",");
                vars_str = AppendStr2HeapStr(vars_str, vars_ln, map->alias);

              } else {
                // We get here when there is no variable in the math input
                // Minimal Polynomial<uID81.0>tciminpoly(%expr%);
                if (cmd_ID != CCID_Minimal_Polynomial)
                  TCI_ASSERT(0);
                if (tally)
                  vars_str = AppendStr2HeapStr(vars_str, vars_ln, ",");
                vars_str = AppendStr2HeapStr(vars_str, vars_ln, var);
              }
              tally++;
            }

            ptr++;
          }                     // loop thru variables
        } else if (p_type == zPT_ASCII_mmlmarkup) { // "<mi>x</mi>"

          SEMANTICS_NODE *sn = msr.GetSemanticsFromParam(p_ID);
          if (sn) {
            SEMANTICS_NODE *s_param = sn;

            if (cmd_ID == CCID_Calculus_Implicit_Differentiation)
              if (VarsAreDefined(s_param))
                error_code = CR_badindepvar;

            int err_code;
            int tally;
            char *v_str = VarsStrFromSNODE(s_param, err_code, tally);
            if (v_str) {
              vars_str = AppendStr2HeapStr(vars_str, vars_ln, v_str);
              delete[] v_str;
            }
          } else {
            TCI_ASSERT(0);
          }
        }
        // Form the returned engine string
        size_t zln = strlen(cmd_template) + strlen(eng_zstr);
        if (vars_str)
          zln += strlen(vars_str);
        zh_rv = new char[zln];
        strcpy(zh_rv, cmd_template);
        StrReplace(zh_rv, zln, "%expr%", eng_zstr);

        if (p_type) {
          if (vars_str) {
            StrReplace(zh_rv, zln, "%which_var%", vars_str);
            delete[] vars_str;
          } else {
            TCI_ASSERT(0);
            StrReplace(zh_rv, zln, "%which_var%", "x");
          }
        }
      } else {
        TCI_ASSERT(0);
      }
    } else {
      TCI_ASSERT(0);
    }
  }

  if (error_code && zh_rv) {
    delete[] zh_rv;
    zh_rv = NULL;
  }

  return zh_rv;
}

char *MuPADWrapper::CreateITERATEeStr(MathServiceRequest & msr,
                                      const char *cmd_template,
                                      int& error_code)
{
  char *zh_rv = NULL;
  error_code = 0;

  if (cmd_template) {
    // Calculus Iterate...<uID42.0>tciterate(%func%,%start%,%n_iters%);

    U32 p_type;
    const char *z_func = msr.GetParam(PID_iterfunc, p_type);  // chambase

    if (p_type == zPT_ASCII_mmlmarkup) {  // "<mi>x</mi>"
      char *z_func_nom = NULL;
      SEMANTICS_NODE *sn = msr.GetSemanticsFromParam(PID_iterfunc);
      if (sn) {
        SEMANTICS_NODE *s_param = sn;
        z_func_nom = ProcessSemList(s_param, NULL, error_code);
      } else {
        TCI_ASSERT(0);
      }
      char *start_str =
        CreateVParameStr(msr, PID_iterstart, zPT_ASCII_text, error_code);
      if (error_code)
        return NULL;

      char *tally_str =
        CreateVParameStr(msr, PID_itercount, zPT_ASCII_natural, error_code);
      if (error_code) {
        delete[] start_str;
        return NULL;
      }

      if (start_str && tally_str && z_func_nom) {
        size_t zln = strlen(cmd_template) + strlen(z_func_nom);
        zln += strlen(start_str);
        zln += strlen(tally_str);

        zh_rv = new char[zln];
        strcpy(zh_rv, cmd_template);
        StrReplace(zh_rv, zln, "%func%", z_func_nom);
        if (start_str) {
          StrReplace(zh_rv, zln, "%start%", start_str);
          delete[] start_str;
        }
        if (tally_str) {
          StrReplace(zh_rv, zln, "%n_iters%", tally_str);
          delete[] tally_str;
        }
      } else {
        TCI_ASSERT(0);
      }
      delete[] z_func_nom;
    } else {
      TCI_ASSERT(0);
      error_code = PID_first_badparam + PID_iterfunc;
    }
  }

  return zh_rv;
}

char *MuPADWrapper::CreateSIntegraleStr(MathServiceRequest & msr, U32 cmd_ID,
                                        const char *eng_zstr,
                                        const char *cmd_template,
                                        int& error_code)
{
  char *zh_rv = NULL;
  error_code = 0;

  if (cmd_template) {
    //Calculus Integrate by Parts...<uID36.0>tcibyparts(%integral%,%part%);
    //Calculus Change Variable...<uID37.0>tcichangevar(%change%,%integral%);

    U32 p_ID =
      (cmd_ID ==
       CCID_Calculus_Integrate_by_Parts) ? PID_CalcByPartsVar :
      PID_CalcChangeVar;
    char *part = NULL;

    U32 p_type;
    const char *z_part = msr.GetParam(p_ID, p_type);
    if (p_type == zPT_ASCII_mmlmarkup) {  // "<mi>x</mi>"
      SEMANTICS_NODE *sn = msr.GetSemanticsFromParam(p_ID);
      if (sn) {
        SEMANTICS_NODE *s_param = sn;
        part = ProcessSemList(s_param, NULL, error_code);;
      } else {
        TCI_ASSERT(0);
      }
      const char *ptr = strstr(eng_zstr, "hold(int)");
      if (ptr) {
        size_t zln = strlen(cmd_template) + strlen(eng_zstr) + strlen(part);
        zh_rv = new char[zln];
        strcpy(zh_rv, cmd_template);
        StrReplace(zh_rv, zln, "%integral%", eng_zstr);
        if (cmd_ID == CCID_Calculus_Integrate_by_Parts)
          StrReplace(zh_rv, zln, "%part%", part);
        else
          StrReplace(zh_rv, zln, "%change%", part);

      } else {
        TCI_ASSERT(0);
        error_code = CR_badintegral;
      }
      delete[] part;
    } else {
      TCI_ASSERT(0);
    }
  }

  return zh_rv;
}

char* MuPADWrapper::CreateAPPROXINTeStr(MathServiceRequest & msr,
                                        const char *eng_zstr,
                                        const char *cmd_template,
                                        int& error_code)
{
  char *zh_rv = NULL;
  error_code = 0;

  if (cmd_template && eng_zstr) {
    //Calculus Approximate Integral...<uID39.0>tciaprxarea(%integral%,%variable%,%lower%..%upper%,%subintervals%,%formula%);

    char* n_intervals =  CreateVParameStr(msr, PID_approxintnsubs, zPT_ASCII_natural, error_code);
    if (error_code) return NULL;
    

    U32 p_type;

    const char* lowerBound = msr.GetParam(PID_approxintlowerbound, p_type);
    if (lowerBound == NULL)
      return NULL;

    const char* upperBound = msr.GetParam(PID_approxintupperbound, p_type);
    if (upperBound == NULL)
      return NULL;
          
    const char* z_formula = msr.GetParam(PID_approxintform, p_type);  // 1..5

    if (z_formula) {
      size_t zln = strlen(cmd_template) + strlen(eng_zstr) + strlen(z_formula) + strlen(lowerBound) + strlen(upperBound);
      
      if (n_intervals)
        zln += strlen(n_intervals);


      zh_rv = new char[zln];
      strcpy(zh_rv, cmd_template);
      StrReplace(zh_rv, zln, "%integral%", eng_zstr);
      if (n_intervals) {
        StrReplace(zh_rv, zln, "%subintervals%", n_intervals);
        delete[] n_intervals;
      }
      StrReplace(zh_rv, zln, "%formula%", z_formula);
      StrReplace(zh_rv, zln, "%lower%", lowerBound);
      StrReplace(zh_rv, zln, "%upper%", upperBound);

      StrReplace(zh_rv, zln, "%variable%", "0");  // ? jcs -- I don't understand this value

    } else {
      TCI_ASSERT(0);
      delete[] n_intervals;
      error_code = PID_first_badparam + PID_approxintform;
    }
  } else
    TCI_ASSERT(0);

  return zh_rv;
}

char *MuPADWrapper::CreateFITCURVEeStr(MathServiceRequest & msr,
                                       const char *eng_zstr, int& error_code)
{
  char *zh_rv = NULL;
  error_code = 0;

  // Fit Curve to Data...<uID106.0>tcimregress(%data_matrix%,%regress_type%,%depend_column%);
  // Fit Curve to Data...<uID106.1>tcipolyfit(%data_matrix%,%degree%,%depend_column%);
  if (eng_zstr) {
    U32 p_type;
    const char *z_regress_code = msr.GetParam(PID_regresscode, p_type); // 0 or 1
    U32 subID = 1;              // assume polynomial fit

    // if PID_regresscode is 0 or 1, we use template 106.0
    if (p_type == zPT_ASCII_natural) {
      int regress_code = atoi(z_regress_code);
      if (regress_code == 0 || regress_code == 1)
        subID = 0;              // linear regression
    }

    const char *cmd_template;
    char p_name[80];
    if (GetdBaseIDRecord("COMMANDS", CCID_Fit_Curve_to_Data, subID, &cmd_template, p_name, 80)) {
      const char *z_depend_colmn = msr.GetParam(PID_dependcolumn, p_type);  // 0 or 1
      if (z_depend_colmn) {
        char *z_degree = NULL;
        size_t zln =
          strlen(cmd_template) + strlen(eng_zstr) + strlen(z_depend_colmn);
        if (subID == 0) {
          zln += strlen(z_regress_code);
        } else {
          z_degree =
            CreateVParameStr(msr, PID_regressdegree, zPT_ASCII_natural,
                             error_code);
          if (error_code) {
            return NULL;
          }
          zln += strlen(z_degree);
        }

        zh_rv = new char[zln];
        strcpy(zh_rv, cmd_template);
        StrReplace(zh_rv, zln, "%data_matrix%", eng_zstr);

        if (subID == 0)
          StrReplace(zh_rv, zln, "%regress_type%", z_regress_code);
        else {
          StrReplace(zh_rv, zln, "%degree%", z_degree);
          delete[] z_degree;
        }
        StrReplace(zh_rv, zln, "%depend_column%", z_depend_colmn);
      } else {
        TCI_ASSERT(0);
        error_code = PID_first_badparam + PID_dependcolumn;
      }
    } else
      TCI_ASSERT(0);
  } else
    TCI_ASSERT(0);

  return zh_rv;
}

char *MuPADWrapper::CreateFILLMATRIXeStr(MathServiceRequest & msr,
                                         int& error_code)
{
  char *zh_rv = NULL;
  error_code = 0;

  // FillMatrix zero<uID74.0>tcizmat(%nRows%,%nCols%);
  // FillMatrix Identity<uID74.1>tciimat(%nRows%,%nCols%);
  // FillMatrix random<uID74.2>linalg::randomMatrix(%nRows%,%nCols%,Dom::Integer);
  // FillMatrix Jordan block<uID74.3>tciJBmat(%diag_entry%,%nRows%,%nCols%);
  // FillMatrix by function<uID74.4>SWPmatrix(%nRows%,%nCols%,%func_nom%);
  // FillMatrix band<uID74.5>tciBand([%band_list%],%nRows%,%nCols%);
  U32 p_type;
  const char *z_ilk = msr.GetParam(PID_FillMatrixIlk, p_type);  // ASCII #
  if (p_type == zPT_ASCII_natural) {
    int ilk = atoi(z_ilk);
    U32 sub_ID = ilk - 1;
    const char *cmd_template;
    char p_name[80];
    if (GetdBaseIDRecord("COMMANDS", CCID_Fill_Matrix, sub_ID, &cmd_template, p_name, 80)) {
      char *extra = NULL;
      if (ilk >= 4) {
        SEMANTICS_NODE *sn = msr.GetSemanticsFromParam(PID_FillMatrixExtra);
        if (sn) {
          extra = ProcessSemNode(sn, NULL, error_code);
          if (error_code)
            return NULL;
        } else {
          TCI_ASSERT(0);
          error_code = PID_first_badparam + PID_FillMatrixExtra;
          return NULL;
        }
      }
      char *rows_str =
        CreateVParameStr(msr, PID_FillMatrixnRows, zPT_ASCII_natural,
                         error_code);
      if (error_code) {
        delete[] extra;
        return NULL;
      }
      char *cols_str =
        CreateVParameStr(msr, PID_FillMatrixnCols, zPT_ASCII_natural,
                         error_code);
      if (error_code) {
        delete[] extra;
        delete[] rows_str;
        return NULL;
      }
      size_t zln = strlen(cmd_template) + strlen(rows_str) + strlen(cols_str);
      if (extra)
        zln += strlen(extra);

      zh_rv = new char[zln];
      strcpy(zh_rv, cmd_template);

      StrReplace(zh_rv, zln, "%nRows%", rows_str);
      StrReplace(zh_rv, zln, "%nCols%", cols_str);
      if (extra) {
        char *subst_token = NULL;
        if (ilk == 4)
          subst_token = "%diag_entry%";
        else if (ilk == 5)
          subst_token = "%func_nom%";
        else if (ilk == 6)
          subst_token = "%band_list%";
        StrReplace(zh_rv, zln, subst_token, extra);
      }

      delete[] rows_str;
      delete[] cols_str;
      delete[] extra;
    } else {
      TCI_ASSERT(0);
    }
  } else {
    TCI_ASSERT(0);
    error_code = PID_first_badparam + PID_FillMatrixIlk;
  }

  return zh_rv;
}

char *MuPADWrapper::CreateTRANSFORMeStr(const char *eng_zstr,
                                        const char *cmd_template,
                                        int& error_code)
{
  if (eng_zstr)
    return TRANSFORMtoEstr(NULL, eng_zstr, cmd_template, error_code);
  else
    return NULL;
}

char *MuPADWrapper::CreateVCALCeStr(MathServiceRequest & msr,
                                    const char *eng_zstr,
                                    const char *cmd_template,
                                    int& error_code)
{
  if (eng_zstr)
    return VCALCOPtoEstr(&msr, NULL, 0, eng_zstr, cmd_template, error_code);
  else
    return NULL;
}

//Rewrite Equations as Matrix<uID23.0>tciequationstomatrix(%eqns_or_mat%,SWPmatrix(1,%nvars%,[%var_list%]));
//Rewrite Matrix as Equations<uID24.0>tcimatrixtoequations(%eqns_or_mat%,SWPmatrix(1,%nvars%,[%var_list%]));

char *MuPADWrapper::CreateEQN2MATeStr(MathServiceRequest & msr,
                                      const char *eng_zstr,
                                      const char *cmd_template,
                                      int& error_code)
{ 
  char *zh_rv = NULL;
  error_code = 0;

  if (eng_zstr && cmd_template) {
    U32 p_type;
    const char *z_var = msr.GetParam(PID_needvarresult, p_type);

    size_t zln = strlen(cmd_template) + strlen(eng_zstr);
    if (z_var)
      zln += strlen(z_var);

    zh_rv = new char[zln];
    strcpy(zh_rv, cmd_template);
    StrReplace(zh_rv, zln, "%eqns_or_mat%", eng_zstr);

    if (z_var) {                // "a,b"
      char *vars_str = NULL;
      U32 vars_ln = 0;
      int tally = 0;

      if (p_type == zPT_ASCII_text) { // "a,b"
        // We shouldn't get here - params that carry variables should be markup
        TCI_ASSERT(0);

        const char *ptr = z_var;
        while (*ptr) {
          if ((*ptr >= 'a' && *ptr <= 'z')
              || (*ptr >= 'A' && *ptr <= 'Z')) {
            // See if we already have an engine name for this variable.
            char var[2];
            var[0] = *ptr;
            var[1] = 0;
            ClientAliases_REC *client_aliases =
              GetClientAliasesRec(client_ID);
            NAME2AKA_REC *map =
              GetAKARec(client_aliases->AKAs_list, NULL, var);
            if (map) {
              if (IsDefined(map->alias))
                error_code = CR_badindepvar;

              if (tally)
                vars_str = AppendStr2HeapStr(vars_str, vars_ln, ",");
              vars_str = AppendStr2HeapStr(vars_str, vars_ln, map->alias);
            } else {
              bool use_alias = false;
              char *eng_var_str =
                CreateEngName(client_ID, NULL, var, use_alias, Ilk_Var);
              if (tally)
                vars_str = AppendStr2HeapStr(vars_str, vars_ln, ",");
              vars_str = AppendStr2HeapStr(vars_str, vars_ln, eng_var_str);
              delete[] eng_var_str;
            }
            tally++;
          }
          ptr++;
        }
      } else if (p_type == zPT_ASCII_mmlmarkup) { // "<mi>x</mi>"
        SEMANTICS_NODE *sn = msr.GetSemanticsFromParam(PID_needvarresult);
        if (sn) {
          SEMANTICS_NODE *s_param = sn;
          int err_code;
          char *v_str = VarsStrFromSNODE(s_param, err_code, tally);
          if (v_str) {
            vars_str = AppendStr2HeapStr(vars_str, vars_ln, v_str);
            delete[] v_str;
          }
        } else {
          TCI_ASSERT(0);
        }
      }

      char buffer[32];
      StrFromInt(tally, buffer);
      StrReplace(zh_rv, zln, "%nvars%", buffer);

      if (vars_str) {
        StrReplace(zh_rv, zln, "%var_list%", vars_str);
        delete[] vars_str;
      } else {
        TCI_ASSERT(0);
        StrReplace(zh_rv, zln, "%var_list%", "x");
      }
    } else {
      TCI_ASSERT(!"Need variable list.");
    }

  } else {
    TCI_ASSERT(0);
  }

  if (error_code && zh_rv) {
    delete[] zh_rv;
    zh_rv = NULL;
  }

  return zh_rv;
}

//Polynomial Collect...<uID29.0>collect(%expr&,{%var%});

char *MuPADWrapper::CreateCOLLECTeStr(MathServiceRequest & msr,
                                      const char *eng_zstr,
                                      const char *cmd_template,
                                      int& error_code)
{
  char *zh_rv = NULL;

  if (eng_zstr) {
    U32 p_type;
    char *var_str = NULL;
    const char *z_var = msr.GetParam(PID_PolyCollectVar, p_type);
    if (p_type == zPT_ASCII_mmlmarkup) {  // "<mi>x</mi>"
      SEMANTICS_NODE *sn = msr.GetSemanticsFromParam(PID_PolyCollectVar);
      if (sn) {
        SEMANTICS_NODE *s_param = sn;
        var_str = ProcessSemList(s_param, NULL, error_code);
      } else {
        TCI_ASSERT(0);
      }
    } else {
      TCI_ASSERT(0);
    }
    size_t zln = strlen(cmd_template) + strlen(eng_zstr);
    if (var_str)
      zln += strlen(var_str);
    zh_rv = new char[zln];
    strcpy(zh_rv, cmd_template);
    StrReplace(zh_rv, zln, "%expr%", eng_zstr);
    StrReplace(zh_rv, zln, "%var%", var_str);

    delete[] var_str;
  }

  return zh_rv;
}

//cmd_template = Power Series...<uID44.0>series(%expr%,%var%=%num%,%order%);

char *MuPADWrapper::CreatePSERIESeStr(MathServiceRequest & msr,
                                      const char *eng_zstr,
                                      const char *cmd_template,
                                      int& error_code)
{
  char *zh_rv = NULL;

  if (eng_zstr) {
    char *order_str =
      CreateVParameStr(msr, PID_seriesorder, zPT_ASCII_natural, error_code);
    if (error_code)
      return NULL;

    U32 p_type;
    char *var_str = NULL;
    const char *z_var = msr.GetParam(PID_seriesvar, p_type);
    if (p_type == zPT_ASCII_mmlmarkup) {  // "<mi>x</mi>"
      SEMANTICS_NODE *sn = msr.GetSemanticsFromParam(PID_seriesvar);
      if (sn) {
        SEMANTICS_NODE *s_param = sn;
        var_str = ProcessSemList(s_param, NULL, error_code);
      } else
        TCI_ASSERT(0);
    } else {
      TCI_ASSERT(0);
      error_code = PID_first_badparam + PID_seriesvar;
      delete[] order_str;
      return NULL;
    }

    char *num_str = NULL;
    const char *z_num = msr.GetParam(PID_seriesabout, p_type);
    if (p_type == zPT_ASCII_mmlmarkup) {  // "<mi>x</mi>"
      SEMANTICS_NODE *sn = msr.GetSemanticsFromParam(PID_seriesabout);
      if (sn) {
        SEMANTICS_NODE *s_param = sn;
        num_str = ProcessSemList(s_param, NULL, error_code);
      } else
        TCI_ASSERT(0);
    } else {
      TCI_ASSERT(0);
      error_code = PID_first_badparam + PID_seriesabout;
      delete[] order_str;
      delete[] var_str;
      return NULL;
    }

    size_t zln = strlen(cmd_template) + strlen(eng_zstr);
    if (var_str)
      zln += strlen(var_str);
    if (num_str)
      zln += strlen(num_str);
    if (order_str)
      zln += strlen(order_str);

    zh_rv = new char[zln];
    strcpy(zh_rv, cmd_template);
    StrReplace(zh_rv, zln, "%expr%", eng_zstr);
    StrReplace(zh_rv, zln, "%var%", var_str);
    StrReplace(zh_rv, zln, "%num%", num_str);
    StrReplace(zh_rv, zln, "%order%", order_str);

    delete[] var_str;
    delete[] num_str;
    delete[] order_str;
  }

  return zh_rv;
}

//cmd_template = Solve ODE Series<uID49.0>ode::series(%theODE_and_func%,%var%=%expand_at%,%order%);

char *MuPADWrapper::CreateODESERIESeStr(MathServiceRequest & msr,
                                        const char *eng_zstr,
                                        const char *cmd_template,
                                        int& error_code)
{
  char *zh_rv = NULL;

  if (eng_zstr) {
    char *order_str =
      CreateVParameStr(msr, PID_ODEseriesorder, zPT_ASCII_natural,
                       error_code);
    if (error_code)
      return NULL;

    U32 p_type;
    char *var_str = NULL;
    const char *z_var = msr.GetParam(PID_ODEIndepVar, p_type);
    if (p_type == zPT_ASCII_mmlmarkup) {  // "<mi>x</mi>"
      SEMANTICS_NODE *sn = msr.GetSemanticsFromParam(PID_ODEIndepVar);
      if (sn) {
        SEMANTICS_NODE *s_param = sn;
        var_str = ProcessSemList(s_param, NULL, error_code);
      } else
        TCI_ASSERT(0);
    } else {
      TCI_ASSERT(0);
      error_code = PID_first_badparam + PID_ODEIndepVar;
      delete[] order_str;
      return NULL;
    }

    char *num_str = NULL;
    const char *z_num = msr.GetParam(PID_ODEseriesabout, p_type);
    if (p_type == zPT_ASCII_mmlmarkup) {  // "<mi>x</mi>"
      SEMANTICS_NODE *sn = msr.GetSemanticsFromParam(PID_ODEseriesabout);
      if (sn) {
        SEMANTICS_NODE *s_param = sn;
        num_str = ProcessSemList(s_param, NULL, error_code);
      } else
        TCI_ASSERT(0);
    } else {
      TCI_ASSERT(0);
      error_code = PID_first_badparam + PID_ODEseriesabout;
      delete[] order_str;
      delete[] var_str;
      return NULL;
    }

    size_t zln = strlen(cmd_template) + strlen(eng_zstr);
    if (var_str)
      zln += strlen(var_str);
    if (num_str)
      zln += strlen(num_str);
    if (order_str)
      zln += strlen(order_str);

    zh_rv = new char[zln];
    strcpy(zh_rv, cmd_template);
    StrReplace(zh_rv, zln, "%theODE_and_func%", eng_zstr);
    StrReplace(zh_rv, zln, "%var%", var_str);
    StrReplace(zh_rv, zln, "%expand_at%", num_str);
    StrReplace(zh_rv, zln, "%order%", order_str);

    delete[] var_str;
    delete[] num_str;
    delete[] order_str;
  }

  return zh_rv;
}

//Quantile...<uID116.0>tciquantile(%stat_list%,%percent%);

char *MuPADWrapper::CreateQUANTILEeStr(MathServiceRequest & msr,
                                       const char *eng_zstr,
                                       const char *cmd_template,
                                       int& error_code)
{
  char *zh_rv = NULL;
  error_code = 0;

  if (eng_zstr) {
    char *percent_str =
      CreateVParameStr(msr, PID_quantile, zPT_ASCII_real, error_code);
    if (!error_code) {
      size_t zln = strlen(cmd_template) + strlen(eng_zstr);
      if (percent_str)
        zln += strlen(percent_str);
      zh_rv = new char[zln];
      strcpy(zh_rv, cmd_template);
      StrReplace(zh_rv, zln, "%stat_list%", eng_zstr);
      if (percent_str) {
        StrReplace(zh_rv, zln, "%percent%", percent_str);
        delete[] percent_str;
      }
    }
  }

  return zh_rv;
}

char *MuPADWrapper::CreateMOMENTeStr(MathServiceRequest & msr,
                                     const char *eng_zstr,
                                     const char *cmd_template,
                                     int& error_code)
{
  char *zh_rv = NULL;
  error_code = 0;

  if (eng_zstr) {
    char *mom_num_str = CreateVParameStr(msr, PID_moment_num,
                                         zPT_ASCII_natural, error_code);
    if (error_code)
      return NULL;

    char *about_str = NULL;
    U32 p_type;
    const char *meanisorigin_str =
      msr.GetParam(PID_moment_meanisorigin, p_type);
    if (p_type == zPT_ASCII_natural) {  // "0" or "1"

      if (meanisorigin_str && !strcmp(meanisorigin_str, "0")) {
        about_str =
          CreateVParameStr(msr, PID_moment_origin, zPT_ASCII_text,
                           error_code);
        if (error_code) {
          delete[] mom_num_str;
          return NULL;
        }
      } else {
        about_str = new char[4];
        strcpy(about_str, "M");
      }

    } else {
      TCI_ASSERT(0);
      error_code = PID_first_badparam + PID_moment_meanisorigin;
      delete[] mom_num_str;
      return NULL;
    }

    // might validate params here!
    //Moment...<uID118.0>tcimoment(%data_set%,%mom_num%,%about%);
    size_t zln = strlen(cmd_template) + strlen(eng_zstr);
    if (mom_num_str)
      zln += strlen(mom_num_str);
    if (about_str)
      zln += strlen(about_str);
    zh_rv = new char[zln];
    strcpy(zh_rv, cmd_template);
    StrReplace(zh_rv, zln, "%data_set%", eng_zstr);

    if (mom_num_str)
      StrReplace(zh_rv, zln, "%mom_num%", mom_num_str);
    if (about_str)
      StrReplace(zh_rv, zln, "%about%", about_str);

    delete[] mom_num_str;
    delete[] about_str;
  }

  return zh_rv;
}

char *MuPADWrapper::CreateMAPSeStr(MathServiceRequest & msr,
                                   const char *eng_zstr,
                                   const char *cmd_template, int& error_code)
{
  char *zh_rv = NULL;
  error_code = 0;

  if (eng_zstr) {
    U32 p_type;
    char *map_str = NULL;
    const char *z_num = msr.GetParam(PID_MapsFunction, p_type);
    if (p_type == zPT_ASCII_mmlmarkup) {  // "<mi>f</mi>"
      SEMANTICS_NODE *sn = msr.GetSemanticsFromParam(PID_MapsFunction);
      if (sn) {
        SEMANTICS_NODE *s_param = sn;
        map_str = ProcessSemList(s_param, NULL, error_code);
      } else
        TCI_ASSERT(0);
    } else {
      TCI_ASSERT(0);
      error_code = PID_first_badparam + PID_MapsFunction;
      return NULL;
    }

    if (!error_code) {
      size_t zln = strlen(cmd_template) + strlen(eng_zstr);
      if (map_str)
        zln += strlen(map_str);
      zh_rv = new char[zln];
      strcpy(zh_rv, cmd_template);
      StrReplace(zh_rv, zln, "%matrix%", eng_zstr);
      if (map_str) {
        StrReplace(zh_rv, zln, "%func%", map_str);
        delete[] map_str;
      }
    }
  }
  return zh_rv;
}

// Random Matrix...<uID88.0>tcirandmat(%nRows%,%nCols%,%matr_type%,%ll%..%ul%);

char *MuPADWrapper::CreateRANDMATRIXeStr(MathServiceRequest & msr,
                                         const char *eng_zstr,
                                         const char *cmd_template,
                                         int& error_code)
{
  char *zh_rv = NULL;
  error_code = 0;

  U32 p_type;
  const char *z_ilk = msr.GetParam(PID_RandMatrixType, p_type); // ASCII #
  if (p_type == zPT_ASCII_natural) {
    char *z_nRows =
      CreateVParameStr(msr, PID_RandMatrixnRows, zPT_ASCII_natural,
                       error_code);
    if (error_code)
      return NULL;

    char *z_nCols =
      CreateVParameStr(msr, PID_RandMatrixnCols, zPT_ASCII_natural,
                       error_code);
    if (error_code) {
      delete[] z_nRows;
      return NULL;
    }

    char *z_ll =
      CreateVParameStr(msr, PID_RandMatrixLLim, zPT_ASCII_real, error_code);
    if (error_code) {
      delete[] z_nRows;
      delete[] z_nCols;
      return NULL;
    }

    char *z_ul =
      CreateVParameStr(msr, PID_RandMatrixULim, zPT_ASCII_real, error_code);
    if (error_code) {
      delete[] z_nRows;
      delete[] z_nCols;
      delete[] z_ll;
      return NULL;
    }

    size_t zln = strlen(cmd_template) + strlen(z_ilk)
      + strlen(z_nRows) + strlen(z_nCols)
      + strlen(z_ll) + strlen(z_ul);

    zh_rv = new char[zln];
    strcpy(zh_rv, cmd_template);

    StrReplace(zh_rv, zln, "%nRows%", z_nRows);
    StrReplace(zh_rv, zln, "%nCols%", z_nCols);
    StrReplace(zh_rv, zln, "%matr_type%", z_ilk);
    StrReplace(zh_rv, zln, "%ll%", z_ll);
    StrReplace(zh_rv, zln, "%ul%", z_ul);

    delete[] z_nRows;
    delete[] z_nCols;
    delete[] z_ll;
    delete[] z_ul;
  } else {
    TCI_ASSERT(0);
    error_code = PID_first_badparam + PID_RandMatrixType;
  }

  return zh_rv;
}

// Random Numbers...<uID107.0>tcirandomnumbers(%distID%,%param1%%comma%%param2%,%tally%);
// tcirandomnumbers( 0,  3,  7, 10);
// tcirandomnumbers( 1,100,0.5, 10);
// tcirandomnumbers( 2, 50,0.5, 10);
// tcirandomnumbers( 3,  7,     10);
// tcirandomnumbers( 4,  7,     10);
// tcirandomnumbers( 5,  1,  3, 10);
// tcirandomnumbers( 6,  3,  1, 10);
// tcirandomnumbers( 7,  3,  7, 10);
// tcirandomnumbers( 8,  4,     10);
// tcirandomnumbers( 9,  7,     10);
// tcirandomnumbers(10,  0, 20, 10);
// tcirandomnumbers(11,  5,  3, 10);

char *MuPADWrapper::CreateRANDOMNUMeStr(MathServiceRequest & msr,
                                        const char *cmd_template,
                                        int& error_code)
{
  char *zh_rv = NULL;
  error_code = 0;
  /*
  #define PID_RandomNumTally      33  natural OR markup
  #define PID_RandomNumDist       34  natural
  #define PID_RandomNumParam1     35  real OR markup
  #define PID_RandomNumParam2     36  real OR markup
  */
  U32 p_type;
  const char *z_ilk = msr.GetParam(PID_RandomNumDist, p_type);  // ASCII #
  if (p_type == zPT_ASCII_natural) {

    int dist_ID = atoi(z_ilk);
    bool takes_p2 = true;
    if (dist_ID == 3 || dist_ID == 4 || dist_ID == 8 || dist_ID == 9)
      takes_p2 = false;

    char *tally_str = CreateVParameStr(msr, PID_RandomNumTally,
                                       zPT_ASCII_natural, error_code);
    if (error_code)
      return NULL;

    char *param1_str = CreateVParameStr(msr, PID_RandomNumParam1,
                                        zPT_ASCII_real, error_code);
    if (error_code) {
      delete[] tally_str;
      return NULL;
    }

    size_t zln = strlen(cmd_template) + strlen(z_ilk)
      + strlen(param1_str) + strlen(tally_str);

    char *param2_str = NULL;

    if (takes_p2) {
      param2_str = CreateVParameStr(msr, PID_RandomNumParam2,
                                    zPT_ASCII_real, error_code);
      if (error_code) {
        delete[] tally_str;
        delete[] param1_str;
        return NULL;
      }
      zln += strlen(param2_str);
    }

    zh_rv = new char[zln];
    strcpy(zh_rv, cmd_template);

    StrReplace(zh_rv, zln, "%distID%", z_ilk);
    StrReplace(zh_rv, zln, "%param1%", param1_str);
    if (takes_p2)
      StrReplace(zh_rv, zln, "%comma%", ",");
    else
      StrReplace(zh_rv, zln, "%comma%", NULL);
    StrReplace(zh_rv, zln, "%param2%", param2_str);

    StrReplace(zh_rv, zln, "%tally%", tally_str);

    delete[] tally_str;
    delete[] param1_str;
    delete[] param2_str;
  } else {
    TCI_ASSERT(0);
    error_code = PID_first_badparam + PID_RandomNumDist;
  }

  return zh_rv;
}

//Reshape...<uID91.0>tcisimplify(%s);
char *MuPADWrapper::CreateRESHAPEeStr(const char *eng_zstr,
                                      const char *cmd_template,
                                      int& error_code)
{
  // Implemented farther up in CompEngine
  TCI_ASSERT(0);
  return NULL;
}



bool MuPADWrapper::CmdGoesToEngine(U32 cmd_ID)
{
  if (cmd_ID == CCID_Reshape || cmd_ID == CCID_Concatenate || cmd_ID == CCID_Stack
      || cmd_ID == CCID_Interpret || cmd_ID == CCID_Fixup || cmd_ID == CCID_Cleanup)
    return false;
  else
    return true;
}

char *MuPADWrapper::LISTtoEstr(SEMANTICS_NODE * s_function,
                               const char *start_str, const char *end_str,
                               const char *join_str, const char *modulus,
                               int& error_code)
{ 
  char *zh_arglist = NULL;
  U32 aln = 0;

  int err_code = 0;
  int arg_counter = 0;
  BUCKET_REC *bucket_rover = s_function->bucket_list;
  while (bucket_rover && err_code != 1) {
    if (bucket_rover->bucket_ID == MB_UNNAMED) {
      if (arg_counter == 0) {
        if (start_str)
          zh_arglist = AppendStr2HeapStr(zh_arglist, aln, start_str);
      } else {
        zh_arglist = AppendStr2HeapStr(zh_arglist, aln, join_str);
      }
      if (bucket_rover->first_child) {
        SEMANTICS_NODE *curr_handle = bucket_rover->first_child;
        char *obj_str = ProcessSemList(curr_handle, modulus, err_code);
        if (obj_str) {
          if (err_code != 1)
            zh_arglist = AppendStr2HeapStr(zh_arglist, aln, obj_str);
          delete[] obj_str;
        }
      }
      arg_counter++;
    }
    bucket_rover = bucket_rover->next;
  }
  if (err_code) {
    error_code = err_code;
    return NULL;
  }
  if (zh_arglist && end_str)
    zh_arglist = AppendStr2HeapStr(zh_arglist, aln, end_str);

  return zh_arglist;
}



// same as above but sets a length

char *MuPADWrapper::LISTtoEstr(SEMANTICS_NODE * s_function,
                               const char *start_str, const char *end_str,
                               const char *join_str, const char *modulus,
                               int& error_code,
                               int& length)
{ 
  char *zh_arglist = NULL;
  U32 aln = 0;

  int err_code = 0;
  int arg_counter = 0;
  BUCKET_REC *bucket_rover = s_function->bucket_list;
  while (bucket_rover && err_code != 1) {
    if (bucket_rover->bucket_ID == MB_UNNAMED) {
      if (arg_counter == 0) {
        if (start_str)
          zh_arglist = AppendStr2HeapStr(zh_arglist, aln, start_str);
      } else {
        zh_arglist = AppendStr2HeapStr(zh_arglist, aln, join_str);
      }
      if (bucket_rover->first_child) {
        SEMANTICS_NODE *curr_handle = bucket_rover->first_child;
        char *obj_str = ProcessSemList(curr_handle, modulus, err_code);
        if (obj_str) {
          if (err_code != 1)
            zh_arglist = AppendStr2HeapStr(zh_arglist, aln, obj_str);
          delete[] obj_str;
        }
      }
      arg_counter++;
    }
    bucket_rover = bucket_rover->next;
  }
  length = arg_counter;
  if (err_code) {
    error_code = err_code;
    return NULL;
  }
  if (zh_arglist && end_str)
    zh_arglist = AppendStr2HeapStr(zh_arglist, aln, end_str);

  return zh_arglist;
}

bool MuPADWrapper::IsMatrix(SEMANTICS_NODE * s_node)
{
  if (s_node && s_node->semantic_type == SEM_TYP_TABULATION)
    return true;
  else
    return false;
}

// Assign Variable<uID13.0>%var%:='%expr%'

char *MuPADWrapper::VARASSIGNtoEstr(SEMANTICS_NODE * s_assign_var,
                                    int& error_code)
{ 
  char *rv = NULL;
  error_code = 0;

  char *key = "Assign Variable";
  if (s_assign_var->semantic_type == SEM_TYP_VARDEF_DEFERRED)
    key = "Assign Variable Deferred";

  const char *db_production;
  U32 ID, subID;
  if (GetdBaseNamedRecord("SCHEMATA", key, &db_production, ID, subID)) {

    TCI_ASSERT(db_production);
    char *nom_str = NULL;
    char *val_str = NULL;

    BUCKET_REC *bucket =
      FindBucketRec(s_assign_var->bucket_list, MB_DEF_VARNAME);
    if (bucket && bucket->first_child) {
      nom_str = ProcessSemList(bucket->first_child, NULL, error_code);
      if (error_code) {
        delete[] nom_str;
        return NULL;
      }
    }

    bucket = FindBucketRec(s_assign_var->bucket_list, MB_DEF_VARVALUE);
    if (bucket && bucket->first_child) {
      val_str = ProcessSemList(bucket->first_child, NULL, error_code);
      if (error_code) {
        delete[] nom_str;
        delete[] val_str;
        return NULL;
      }
    }

    if (nom_str && val_str) {
      size_t bln = strlen(db_production) + strlen(nom_str) + strlen(val_str);
      rv = new char[bln];
      strcpy(rv, db_production);
      StrReplace(rv, bln, "%var%", nom_str);
      StrReplace(rv, bln, "%expr%", val_str);
    }
    delete[] nom_str;
    delete[] val_str;
  } else
    TCI_ASSERT(0);

  return rv;
}

// tcikill('q2f');
// q2f:='(q2x)->q2x^(2)+2';
//Define Function<uID14.0>%f_nom%:=hold((%var_list%)->%expr%);
// piecewise<uID?.0>piecewise(%dr_list%)

char *MuPADWrapper::FUNCDEFtoEstr(SEMANTICS_NODE * s_funcdef,
                                  int& error_code)
{
  char *rv = NULL;

  bool do_kill = false;

  const char *db_killcmd;
  U32 ID, subID;
  if (GetdBaseNamedRecord("SCHEMATA", "Kill command", &db_killcmd, ID, subID)) {
    // Kill command<uID32.0>tcikill('%f_nom%');
    do_kill = true;
  }

  const char *db_production;
  if (GetdBaseNamedRecord
      ("SCHEMATA", "Define Function", &db_production, ID, subID)) {
    // Define Function<uID14.0>%f_nom%:=hold((%var_list%)->%expr%);
    TCI_ASSERT(db_production);

    char *f_nom_str = NULL;
    char *var_list_str = NULL;
    char *expr_str = NULL;

    BUCKET_REC *header_bucket =
      FindBucketRec(s_funcdef->bucket_list, MB_DEF_FUNCHEADER);

    if (header_bucket && header_bucket->first_child) {

      SEMANTICS_NODE *s_func = header_bucket->first_child;
      if (s_func->semantic_type == SEM_TYP_PRECEDENCE_GROUP) {
        BUCKET_REC *bucket = s_func->bucket_list;
        s_func = bucket->first_child;
      }
      // Handle the function name
      f_nom_str = GetEngNameForUserFunc(s_func);

      // Handle the function variables
      const char *ptr = strstr(db_production, "%var_list%");
      if (ptr) {
        int err_code = 0;
        var_list_str = LISTtoEstr(s_func, NULL, NULL, ",", NULL, err_code);

        // jcs -- this is actually an error
        if (!var_list_str)
          var_list_str = "";

        if (err_code) {
          TCI_ASSERT(0);
          error_code = err_code;
          return rv;
        }
      } else
        TCI_ASSERT(0);

    } else
      TCI_ASSERT(0);

    // Handle the defining expression
    BUCKET_REC *expr_bucket =
      FindBucketRec(s_funcdef->bucket_list, MB_DEF_FUNCVALUE);
    if (expr_bucket && expr_bucket->first_child) {

      SEMANTICS_NODE *s_content = expr_bucket->first_child;
      int err_code = 0;
      if (s_content->semantic_type == SEM_TYP_PIECEWISE_LIST) {
        expr_str = Piecewise2Estr(s_content, err_code);
      } else {
        expr_str = ProcessSemList(s_content, NULL, err_code);
      }
      if (err_code) {
        error_code = err_code;
        delete[] f_nom_str;
        delete[] var_list_str;
        return NULL;
      }
    } else
      TCI_ASSERT(0);

    // Compose the return string
    size_t bln = 0;
    if (do_kill)
      bln = strlen(db_killcmd) + strlen(f_nom_str);
    bln +=
      strlen(db_production) + strlen(f_nom_str) + strlen(var_list_str) +
      strlen(expr_str);
    rv = new char[bln];
    rv[0] = 0;

    if (do_kill) {
      strcpy(rv, db_killcmd);
      StrReplace(rv, bln, "%f_nom%", f_nom_str);
    }

    strcat(rv, db_production);
    StrReplace(rv, bln, "%f_nom%", f_nom_str);
    StrReplace(rv, bln, "%var_list%", var_list_str);
    StrReplace(rv, bln, "%expr%", expr_str);

    delete[] f_nom_str;
    delete[] var_list_str;
    delete[] expr_str;
  } else                        // no production for "define Functiom"
    TCI_ASSERT(0);

  return rv;
}


char *MuPADWrapper::GetEngNameForUserFunc(SEMANTICS_NODE * s_func)
{
  char *rv = NULL;

  // This user function is (or will be) defined within the scope of some client.
  // Analyzer has recorded the client ID of the owner in s_func->owner_ID.

  U32 frame_ID = s_func->owner_ID;
  if (!frame_ID) {              // give ownership to the client requesting
    frame_ID = client_ID;       //  the current computation
  }
  // See if we already have an engine name for this function.

  ClientAliases_REC *client_aliases = GetClientAliasesRec(frame_ID);
  NAME2AKA_REC *map = GetAKARec(client_aliases->AKAs_list,
                                s_func->canonical_ID,
                                s_func->contents);
  if (map) {
    size_t zln = strlen(map->alias);
    char *tmp = new char[zln + 1];
    strcpy(tmp, map->alias);
    rv = tmp;
  } else {
    bool use_alias = true;
    char base_name[128];
    base_name[0] = 0;
    char *user_name = s_func->contents; // myfunc
    if (user_name && *user_name) {
      size_t bln = strlen(user_name);
      if (s_func->canonical_ID) { // mimyfunc
        size_t cln = strlen(s_func->canonical_ID);
        if (cln == bln + 2) {
          // Is there an entity in the name?
          char *ptr = strchr(user_name, '&');
          if (!ptr) {
            use_alias = false;
            if (bln < 128)
              strcpy(base_name, user_name);
            else
              TCI_ASSERT(0);
          }
        }
        rv = CreateEngName(frame_ID, s_func->canonical_ID,
                           base_name, use_alias, Ilk_Func);
      } else {
        TCI_ASSERT(0);
      }
    } else {
      TCI_ASSERT(0);
    }
  }

  return rv;
}

char *MuPADWrapper::POWERasSURDtoEstr(SEMANTICS_NODE * s_pform,
                                      int& error_code)
{
  char *zh_rv = NULL;

  BUCKET_REC *script_bucket = FindBucketRec(s_pform->bucket_list,
                                            MB_SCRIPT_UPPER);
  if (script_bucket && script_bucket->first_child) {
    SEMANTICS_NODE *s_script = script_bucket->first_child;
    if (s_script->semantic_type == SEM_TYP_FRACTION
        && s_script->next == NULL && s_script->bucket_list) {
      SEMANTICS_NODE *s_exp = NULL;
      BUCKET_REC *num_bucket = FindBucketRec(s_script->bucket_list,
                                             MB_NUMERATOR);
      BUCKET_REC *den_bucket = FindBucketRec(s_script->bucket_list,
                                             MB_DENOMINATOR);
      if (num_bucket && den_bucket) {
        s_exp = den_bucket->first_child;

        bool is_sqrt;
        char *key = NULL;
        if (s_exp->semantic_type == SEM_TYP_NUMBER
            && !strcmp(s_exp->contents, "2")) {
          key = "sqrt";
          is_sqrt = true;
        } else {
          key = "surd";
          is_sqrt = false;
        }

        const char *db_production;
        U32 ID, subID;
        if (GetdBaseNamedRecord("SCHEMATA", key, &db_production, ID, subID)) {
          TCI_ASSERT(db_production);  // surd<uID3.0>(%base%,%radicand%)
          char *base_str = NULL;  // sqrt<uID3.0>sqrt(%base%)
          char *radi_str = NULL;

          BUCKET_REC *bucket =
            FindBucketRec(s_pform->bucket_list, MB_SCRIPT_BASE);
          if (bucket && bucket->first_child) {
            base_str = ProcessSemList(bucket->first_child, NULL, error_code);
            if (error_code) {
              delete[] base_str;
              return NULL;
            }
          }
          if (!is_sqrt) {
            if (den_bucket && den_bucket->first_child) {
              radi_str =
                ProcessSemList(den_bucket->first_child, NULL, error_code);
              if (error_code) {
                delete[] base_str;
                delete[] radi_str;
                return NULL;
              }
            }
          }
          if (base_str) {
            size_t bln = strlen(db_production) + strlen(base_str);
            if (radi_str)
              bln += strlen(radi_str);
            zh_rv = new char[bln];
            strcpy(zh_rv, db_production);
            StrReplace(zh_rv, bln, "%base%", base_str);
            if (!is_sqrt)
              StrReplace(zh_rv, bln, "%radicand%", radi_str);
          }
          delete[] base_str;
          delete[] radi_str;
        } else {
          TCI_ASSERT(0);
        }
      } else {
        TCI_ASSERT(0);
      }
    } else {
      TCI_ASSERT(0);
    }
  }

  return zh_rv;
}

char *MuPADWrapper::GetDiffVarStr(SEMANTICS_NODE * var_snode,
                                  int& error_code)
{
  char *zh_rv = NULL;
  error_code = 0;

  char *dollar_str = NULL;
  if (var_snode->semantic_type == SEM_TYP_POWERFORM) {
    BUCKET_REC *bucket =
      FindBucketRec(var_snode->bucket_list, MB_SCRIPT_UPPER);
    if (bucket && bucket->first_child) {
      dollar_str = ProcessSemList(bucket->first_child, NULL, error_code);
      if (error_code) {
        delete[] dollar_str;
        return NULL;
      }
    }
    bucket = FindBucketRec(var_snode->bucket_list, MB_SCRIPT_BASE);
    if (bucket && bucket->first_child)
      var_snode = bucket->first_child;
  }

  char *zh_var = ProcessSemNode(var_snode, NULL, error_code);
  if (error_code) {
    delete[] zh_var;
    delete[] dollar_str;
    return NULL;
  }
  if (zh_var) {
    U32 rv_ln = 0;
    zh_rv = AppendStr2HeapStr(zh_rv, rv_ln, zh_var);
    delete[] zh_var;
    if (dollar_str) {
      zh_rv = AppendStr2HeapStr(zh_rv, rv_ln, "$");
      zh_rv = AppendStr2HeapStr(zh_rv, rv_ln, dollar_str);
      delete[] dollar_str;
    }
  }
  return zh_rv;
}

SEMANTICS_NODE *MuPADWrapper::LocateVar(BUCKET_REC * expr_bucket)
{
  SEMANTICS_NODE *rv = NULL;

  while (expr_bucket) {
    if (expr_bucket->first_child) {
      SEMANTICS_NODE *s_rover = expr_bucket->first_child;
      while (s_rover) {
        if (s_rover->semantic_type == SEM_TYP_VARIABLE) {
          rv = s_rover;
          break;
        } else if (s_rover->bucket_list) {
          SEMANTICS_NODE *rv1 = LocateVar(s_rover->bucket_list);
          if (rv1) {
            rv = rv1;
            break;
          }
        }
        s_rover = s_rover->next;
      }
    } else if (expr_bucket->parts) {
      TCI_ASSERT(0);
    }
    expr_bucket = expr_bucket->next;
  }
  return rv;
}

// The following handles a limit on a substitution schemata - "x=1" OR "1"
//  or a list of limits - "x=0, y=0, .."

char *MuPADWrapper::Subst2Estr(SEMANTICS_NODE * subst_info,
                               bool is_lower,
                               BUCKET_REC * expr_bucket,
                               BUCKET_REC * other_subst_bucket,
                               int& error_code)
{
  char *zh_rv = NULL;
  U32 rv_ln = 0;
  error_code = 0;

  U32 b_ID = is_lower ? MB_SUBST_LOWER : MB_SUBST_UPPER;

  BUCKET_REC *b_rover = subst_info->bucket_list;
  while (b_rover) {
    if (b_rover->bucket_ID == b_ID && b_rover->first_child) {
      SEMANTICS_NODE *s_subst = b_rover->first_child;

      char *curr_subst = ProcessSemList(s_subst, NULL, error_code);
      if (error_code) {
        delete[] curr_subst;
        delete[] zh_rv;
        return NULL;
      }

      bool add_variable = true;
      SEMANTICS_NODE *s_rover = s_subst;
      if (s_rover->semantic_type == SEM_TYP_PRECEDENCE_GROUP
          && s_rover->bucket_list)
        s_rover = s_rover->bucket_list->first_child;

      if (s_rover->semantic_type == SEM_TYP_INFIX_OP
          && !strcmp(s_rover->contents, "="))
        add_variable = false;

      if (add_variable) {
        SEMANTICS_NODE *s_var = LocateVar(other_subst_bucket);
        if (!s_var)
          s_var = LocateVar(expr_bucket);
        if (s_var) {
          char *var_str = VARIABLEtoEstr(s_var, error_code);
          size_t v_ln = strlen(var_str) + 1;
          U32 temphack = v_ln;
          var_str = AppendStr2HeapStr(var_str, temphack, "=");
          var_str = AppendStr2HeapStr(var_str, temphack, curr_subst);
          delete[] curr_subst;
          curr_subst = var_str;
        } else {
          TCI_ASSERT(0);
        }
      }

      if (curr_subst) {
        if (zh_rv)
          zh_rv = AppendStr2HeapStr(zh_rv, rv_ln, ",");
        zh_rv = AppendStr2HeapStr(zh_rv, rv_ln, curr_subst);
        delete[] curr_subst;
      } else {
        TCI_ASSERT(0);
      }
    }
    b_rover = b_rover->next;
  }

  return zh_rv;
}

char *MuPADWrapper::Piecewise2Estr(SEMANTICS_NODE * pw_list, int& error_code)
{
  char *zh_rv = NULL;

  const char *db_production;
  U32 ID, subID;
  if (GetdBaseNamedRecord("SCHEMATA", "piecewise", &db_production, ID, subID)) {
    TCI_ASSERT(db_production);

    char *dr_list_str = NULL;
    U32 dr_ln = 0;
    U32 ow_ln = 0;
    U32 ow_obj_ln = 0;

    const char *ptr = strstr(db_production, "%dr_list%");
    if (ptr) {
      // MuPAD "piecewise" takes a list   "[domain,expr],[domain,expr],..[domain,expr]"
      // piecewise(
      //   [q1t<0           ,q1t  ],
      //   [0<=q1t and q1t<1,0    ],
      //   [1<=q1t and q1t<2,1    ],
      //   [2<=q1t and q1t<3,2    ],
      //   [3<=q1t          ,6-q1t]
      // )
      int err_code = 0;
      int tally = 0;

      BUCKET_REC *b_rover = pw_list->bucket_list;
      char *otherwise_str = NULL;
      char* otherwise_obj = NULL;
      bool bUseOtherwiseStr = false;
      while (b_rover) {
        SEMANTICS_NODE *s_one_piece = b_rover->first_child;
        BUCKET_REC *exp_bucket =
          FindBucketRec(s_one_piece->bucket_list, MB_PIECE_EXPR);
        BUCKET_REC *dom_bucket =
          FindBucketRec(s_one_piece->bucket_list, MB_UNNAMED);

        char * domain_str = NULL;
        bool bOtherwiseCase = false;
        if (dom_bucket && dom_bucket->first_child) {
          SEMANTICS_NODE *curr_handle = dom_bucket->first_child;
          domain_str = SemDomainToEstr(curr_handle, err_code);
          if (err_code) {
            error_code = err_code;
            delete[] dr_list_str;
            return NULL;
          }
        }
        if (domain_str && !strcmp(domain_str, "otherwise"))
          bOtherwiseCase = true;
        if (domain_str && !bOtherwiseCase) {
          if (tally) {
            dr_list_str = AppendStr2HeapStr(dr_list_str, dr_ln, ",");
            otherwise_str = AppendStr2HeapStr(otherwise_str, ow_ln, " and ");
          }
          dr_list_str = AppendStr2HeapStr(dr_list_str, dr_ln, "[");
          dr_list_str = AppendStr2HeapStr(dr_list_str, dr_ln, domain_str);
          otherwise_str = AppendStr2HeapStr(otherwise_str, ow_ln, "not(");
          otherwise_str = AppendStr2HeapStr(otherwise_str, ow_ln, domain_str);
          otherwise_str = AppendStr2HeapStr(otherwise_str, ow_ln, ")");
          tally++;
          delete[] domain_str;
        }
        if (exp_bucket && exp_bucket->first_child) {
          SEMANTICS_NODE *curr_handle = exp_bucket->first_child;
          char *obj_str = ProcessSemList(curr_handle, NULL, err_code);
          if (err_code) {
            error_code = err_code;
            delete[] dr_list_str;
            return NULL;
          }
          if (obj_str) {
            if (bOtherwiseCase) {
              otherwise_obj = AppendStr2HeapStr(otherwise_obj, ow_obj_ln, obj_str);
              bUseOtherwiseStr = true;
            } else {
              if (tally)
                dr_list_str = AppendStr2HeapStr(dr_list_str, dr_ln, ",");
              dr_list_str = AppendStr2HeapStr(dr_list_str, dr_ln, obj_str);
              dr_list_str = AppendStr2HeapStr(dr_list_str, dr_ln, "]");
              tally++;
            }
            delete[] obj_str;
          }
        }
        b_rover = b_rover->next;
      }
      if (bUseOtherwiseStr)
      {
        if (tally)
          dr_list_str = AppendStr2HeapStr(dr_list_str, dr_ln, ",");
        dr_list_str = AppendStr2HeapStr(dr_list_str, dr_ln, "[");
        dr_list_str = AppendStr2HeapStr(dr_list_str, dr_ln, otherwise_str);
        dr_list_str = AppendStr2HeapStr(dr_list_str, dr_ln, ",");
        dr_list_str = AppendStr2HeapStr(dr_list_str, dr_ln, otherwise_obj);
        dr_list_str = AppendStr2HeapStr(dr_list_str, dr_ln, "]");
      }
      delete[] otherwise_str;
      delete[] otherwise_obj;
    }

    // Compose the return string
    // piecewise<uID?.0>piecewise(%dr_list%)
    size_t bln = strlen(db_production) + 1;
    if (dr_list_str)
      bln += strlen(dr_list_str);
    zh_rv = new char[bln];
    strcpy(zh_rv, db_production);
    StrReplace(zh_rv, bln, "%dr_list%", dr_list_str);
    delete[] dr_list_str;
  } else {
    TCI_ASSERT(0);
  }
  return zh_rv;
}

// Solve Recursion<uID36.0>%eqns%,{%rfunc%}
// Solve CRecursion<uID36.1>{%eqns%},{%rfunc%}

char *MuPADWrapper::Recursion2Estr(SEMANTICS_NODE * recursion,
                                   int& error_code)
{
  char *zh_rv = NULL;
  error_code = 0;

  char *eqn_str = NULL;
  U32 eqn_ln = 0;
  char *rfunc_str = NULL;

  int tally = 0;
  if (recursion && recursion->bucket_list) {
    BUCKET_REC *b_rover = recursion->bucket_list;
    while (b_rover) {
      if (b_rover->bucket_ID == MB_RECUR_EQN) {
        SEMANTICS_NODE *curr_handle = b_rover->first_child;
        char *obj_str = ProcessSemList(curr_handle, NULL, error_code);
        if (error_code) {
          delete[] eqn_str;
          delete[] rfunc_str;
          return NULL;
        }
        if (obj_str) {
          if (tally)
            eqn_str = AppendStr2HeapStr(eqn_str, eqn_ln, ",");
          eqn_str = AppendStr2HeapStr(eqn_str, eqn_ln, obj_str);
          tally++;
          delete[] obj_str;
        }
      } else if (b_rover->bucket_ID == MB_RECUR_FUNC) {
        if (!rfunc_str) {
          SEMANTICS_NODE *curr_handle = b_rover->first_child;
          rfunc_str = ProcessSemList(curr_handle, NULL, error_code);
          if (error_code) {
            delete[] rfunc_str;
            delete[] eqn_str;
            return NULL;
          }
        } else {
          TCI_ASSERT(0);
        }
      } else {
        TCI_ASSERT(!"Unexpected bucket");
      }
      b_rover = b_rover->next;
    }                           // loop

    const char *key = (tally > 1) ? "Solve CRecursion" : "Solve Recursion";
    const char *db_production;
    U32 ID, subID;
    if (GetdBaseNamedRecord("SCHEMATA", key, &db_production, ID, subID)) {
      // Compose the return string
      size_t bln = strlen(db_production) + 1;
      if (eqn_str)
        bln += strlen(eqn_str);
      if (rfunc_str)
        bln += strlen(rfunc_str);
      zh_rv = new char[bln];
      strcpy(zh_rv, db_production);
      StrReplace(zh_rv, bln, "%eqns%", eqn_str);
      StrReplace(zh_rv, bln, "%rfunc%", rfunc_str);

      delete[] eqn_str;
      delete[] rfunc_str;
    } else {
      TCI_ASSERT(0);
    }
  } else {
    TCI_ASSERT(0);
  }

  return zh_rv;
}

int MuPADWrapper::GetEngineMessageID(SEMANTICS_NODE * s_tree)
{
  int rv = 0;

  if (s_tree && !s_tree->next) {
    if (s_tree->semantic_type == SEM_TYP_VARIABLE
        && s_tree->contents && !strcmp(s_tree->contents, "needvars"))
      rv = CR_needvars;
  }

  return rv;
}

/*
  NUMBER canonical_ID = "mn0" contents = "0"
  OPERATOR contents = "&#x2264;"
  VARIABLE canonical_ID = "mix" contents = "x"
  OPERATOR contents = "&#x3c;"
  NUMBER canonical_ID = "mn7" contents = "7"
*/

U32 MuPADWrapper::GetRelationExpressionOpType(SEMANTICS_NODE * s_node)
{
  if ((s_node->semantic_type == SEM_TYP_INFIX_OP) && s_node->contents)
  {
      //check for "=", "<", ">", "<=", ">=", "&leq;", "&geq;", "&NotEqual;"
    if (!strcmp(s_node->contents, "=") || !strcmp(s_node->contents, "&#x3c;") || !strcmp(s_node->contents, "&#x3e;") ||
//          !strcmp(s_node->contents, "<=") || !strcmp(s_node->contents, ">=") ||  //these two forms appear to always be translated previously?
          !strcmp(s_node->contents, "&#x2264;") || !strcmp(s_node->contents, "&#x2265;") || !strcmp(s_node->contents, "&#x2260;") ||
//         &#x2248 is the approx symbol. 
          !strcmp(s_node->contents, "&#x2248;") )
      return binOrderRelation;
      //check for "in", "Element", "sub", "sube", "sup", "supe"
    if ( !strcmp(s_node->contents, "&#x220a;") || !strcmp(s_node->contents, "&#x2208;") ||
         !strcmp(s_node->contents, "&#x2282;") || !strcmp(s_node->contents, "&#x2286;") ||
             !strcmp(s_node->contents, "&#x2283;") || !strcmp(s_node->contents, "&#x2287;") )
      return binInclusionRelation;
    if (!strcmp(s_node->contents, "&#x2227;") || !strcmp(s_node->contents, "&#x2228;") ) //"and", "or"
      return binConnectingOperation;
  }
  else if ((s_node->semantic_type == SEM_TYP_PREFIX_OP) && s_node->contents)
  {
    if (!strcmp(s_node->contents, "&#xac;")) //"not"
      return unaryNegationOperation;
  }
  return opTypeUnknown;
}

SEMANTICS_NODE * MuPADWrapper::FindRelationExpressionOperator(SEMANTICS_NODE * s_list, U32 whichType, bool bReturnLast)
{
  SEMANTICS_NODE * rv = NULL;
  SEMANTICS_NODE *s_rover = s_list;
  U32 findType = whichType;
  if (!findType)
    findType = anyType;
  U32 currType = opTypeUnknown;
  while (s_rover)
  {
    currType = GetRelationExpressionOpType(s_rover);
    if (currType & findType)
    {
      rv = s_rover;
      if (!bReturnLast)
        return rv;
    }
    s_rover = s_rover->next;
  }
  
  return rv;
}

//Utility for getting the string before the first or after the last nested relation inside a precedence group
char *MuPADWrapper::RetrieveNestedSemDomainOperandStr(SEMANTICS_NODE * s_node, bool bRightHand, int& error_code)
{
  char* zh_rv = NULL;
  U32 zh_ln = 0;
  if ( (s_node->semantic_type != SEM_TYP_PRECEDENCE_GROUP) || !s_node->bucket_list 
         || !s_node->bucket_list->first_child )
    return zh_rv;
  s_node = s_node->bucket_list->first_child;
  SEMANTICS_NODE *s_temp = FindRelationExpressionOperator(s_node, binOrderRelation | binInclusionRelation, bRightHand);   //get first or last binary comparison relation
  if (s_temp)
  {
    SEMANTICS_NODE *s_save = NULL;
    SEMANTICS_NODE *s_nested_op = NULL;
    SEMANTICS_NODE *s_nested_operand = NULL;
    s_nested_op = s_temp;
    if (bRightHand)
    {
      if (s_temp->next)
        s_nested_operand = s_temp->next;
    }
    else
    {
      if (s_temp->prev)
        s_nested_operand = s_temp->prev;
    }
    //Try recursing in case this operand is another nested relational expression
    zh_rv = RetrieveNestedSemDomainOperandStr(s_nested_operand, bRightHand, error_code);
    if (!zh_rv && s_nested_operand)
    {
      if (!bRightHand)
      {
        s_save = s_nested_operand;
        s_save->next = NULL;
        //if we're looking for the left-hand operand of the first relation, move back to the start of the list (shouldn't ever really do anything, but...)
        while (s_nested_operand->prev)
          s_nested_operand = s_nested_operand->prev;
      }
      zh_rv = ProcessSemList(s_nested_operand, NULL, error_code);  //recurse to get the string for relation to the left
      if (s_save)
        s_save->next = s_nested_op;
    }
  }
  return zh_rv;
}

//We assume here that precendence has been taken care of, so we don't worry about grouping per se. But we do have to
//  worry about things like 2<=x<=5, which will come in as <SEM_TYP_PRECEDENCE_GROUP><SEM_TYP_INFIX_OP><SEM_TYP_PRECEDENCE_GROUP>,
//  but we need to parse into "2<=x and x<=5", or even possibly <NUMBER><INFIX_OP><VARIABLE><INFIX_OP><NUMBER>.
char *MuPADWrapper::SemDomainToEstr(SEMANTICS_NODE * s_list, int& error_code)
{
  char *zh_rv = NULL;
  U32 zh_ln = 0;

  if (s_list && s_list->semantic_type == SEM_TYP_TEXT)
  {
    if (!strcmp(s_list->contents, "otherwise"))
    {
      zh_rv = AppendStr2HeapStr(zh_rv, zh_ln, "otherwise");  //this should act as "otherwise" if it's the last condition in the piecewise list
      return zh_rv;
    }
    s_list = s_list->next;
  }

  while (s_list
         && !s_list->next    // but s_list->next could be a precedence group??
         && s_list->semantic_type == SEM_TYP_PRECEDENCE_GROUP) {
    if (s_list->bucket_list && s_list->bucket_list->first_child)
      s_list = s_list->bucket_list->first_child;
    else
      TCI_ASSERT(0);
  }

  //First look for connecting operators
  char* zh_str = NULL;
  U32 zh_str_ln = 0;
  SEMANTICS_NODE* s_rover = s_list;
  SEMANTICS_NODE* s_save = NULL;
  SEMANTICS_NODE* s_temp = NULL;

  SEMANTICS_NODE * s_op = FindRelationExpressionOperator(s_list, binConnectingOperation, false);
  if (s_op)
  {
    while (s_rover)
    {
      //When we run out of logical connectors, we'll come through one last time without s_op and output the end of the list
      s_temp = (s_op && s_op != s_rover) ? s_op->prev : NULL;
      if (s_temp)
        s_temp->next = NULL;
      zh_str = SemDomainToEstr(s_rover, error_code);  //recurse - this time there'll be no connecting operators
      if (s_temp)
        s_temp->next = s_op;
      if (zh_str)
      {
        zh_rv = AppendStr2HeapStr(zh_rv, zh_ln, "(");
        zh_rv = AppendStr2HeapStr(zh_rv, zh_ln, zh_str);
        zh_rv = AppendStr2HeapStr(zh_rv, zh_ln, ")");
        delete[] zh_str;
        zh_str = NULL;
      }
      if (s_op)
      {
        s_rover = s_op->next;
        if (s_rover)
        {
          if (!strcmp(s_op->contents, "&#x2227;"))
            zh_rv = AppendStr2HeapStr(zh_rv, zh_ln, " and ");
          else if (!strcmp(s_op->contents, "&#x2228;"))
            zh_rv = AppendStr2HeapStr(zh_rv, zh_ln, " or ");
          else
            zh_rv = ProcessSemNode(s_op, NULL, error_code);
          s_op = FindRelationExpressionOperator(s_rover, binConnectingOperation, false);
        }
      }
      else
        s_rover = NULL;
    }
    return zh_rv;
  }

  s_op = FindRelationExpressionOperator(s_list, unaryNegationOperation, false);
  if (s_op)
  {
    TCI_ASSERT(s_op == s_list);
    s_rover = s_op->next;
    if (!strcmp(s_op->contents, "&#xac;")) //"not"
      zh_rv = AppendStr2HeapStr(zh_rv, zh_ln, "not");
    else
      zh_rv = ProcessSemNode(s_op, NULL, error_code);
    if (s_rover)
    {
      zh_str = SemDomainToEstr(s_rover, error_code);
      if (zh_str)
      {
        zh_rv = AppendStr2HeapStr(zh_rv, zh_ln, "(");
        zh_rv = AppendStr2HeapStr(zh_rv, zh_ln, zh_str);
        zh_rv = AppendStr2HeapStr(zh_rv, zh_ln, ")");
        delete[] zh_str;
      }
    }
    return zh_rv;
  }

  //Now we're in a (more or less) normal case. Just look for binary relations; if there are more than one (or if
  //  there are more nested in precedence group operands), we connect them with "and", repeating each right-hand
  //  operand as the left-hand operand in the next expression.
//  s_op = FindRelationExpressionOperator(s_list, binOrderRelation | binInclusionRelation, false);
//  if (s_op)
//  {
  SEMANTICS_NODE* s_nested_op = NULL;
  SEMANTICS_NODE* l_nested_operand = NULL;
  SEMANTICS_NODE* r_nested_operand = NULL;
  bool pastOp = false;
  zh_str = NULL;
  zh_str_ln = 0;
  char* zh_rh_str = NULL;
  U32 zh_rh_ln = 0;
  while (s_rover)
  {
    switch(s_rover->semantic_type)
    {
      case SEM_TYP_INFIX_OP:
        if ( !strcmp(s_rover->contents, "&#x220a;") || !strcmp(s_rover->contents, "&#x2208;") )
          zh_str = AppendStr2HeapStr(zh_str, zh_str_ln, " in ");
        else
          zh_str = ProcessSemNode(s_rover, NULL, error_code);
        if ( GetRelationExpressionOpType(s_rover) & (binOrderRelation | binInclusionRelation) )
        {
          if (zh_rh_str)  //this was the right-hand operand from previous op - we'll use it as left-hand
          {
            zh_rv = AppendStr2HeapStr(zh_rv, zh_ln, " and ");
            zh_rv = AppendStr2HeapStr(zh_rv, zh_ln, zh_rh_str);
            delete[] zh_rh_str;
            zh_rh_str = NULL;
            zh_rh_ln = 0;
          }
          pastOp = true;
        }
        zh_rv = AppendStr2HeapStr(zh_rv, zh_ln, zh_str);  //We do this here so zh_str won't be added to zh_rh_str below
        delete[] zh_str;
        zh_str = NULL;
        zh_str_ln = 0;
      break;

      case SEM_TYP_PRECEDENCE_GROUP:
        //Here, we check whether the precedence group contains one of our binary relations (at its top level). If so,
        //  we recursively use SemDomainToEstr() on the precedence group. If additionally it's adjacent to one of our 
        //  binary relations, we extract the left- and/or right-hand sides to be used as the right- and/or left-hand 
        //  operands of the adjacent relations. If it doesn't contain one of our binary relations, we just output it
        //  normally (with ProcessSemNode) and keep going.
        if (s_rover->bucket_list && s_rover->bucket_list->first_child)
        {
          //get first binary comparison relation
          s_temp = FindRelationExpressionOperator(s_rover->bucket_list->first_child, binOrderRelation | binInclusionRelation, false);  
          if (s_temp)
          {
            if ( s_rover->prev && (GetRelationExpressionOpType(s_rover->prev) & (binOrderRelation | binInclusionRelation)) )
            {
              if (zh_rh_str)
              {
                TCI_ASSERT(0);  //this shouldn't happen! we're just after a relation, so this should be NULL
                delete[] zh_rh_str;
              }
              zh_rh_ln = 0;
              zh_rh_str = RetrieveNestedSemDomainOperandStr(s_rover, false, error_code);
            }
            if (zh_rh_str)
            {
              zh_rv = AppendStr2HeapStr(zh_rv, zh_ln, zh_rh_str);
              zh_rv = AppendStr2HeapStr(zh_rv, zh_ln, " and ");
              delete[] zh_rh_str;
              zh_rh_str = NULL;
              zh_rh_ln = 0;
            }
            zh_str = SemDomainToEstr(s_rover->bucket_list->first_child, error_code);

            if ( s_rover->next && (GetRelationExpressionOpType(s_rover->next) & (binOrderRelation | binInclusionRelation)) )
            {
              if (zh_rh_str)
              {
                TCI_ASSERT(0);  //this shouldn't happen! we're just after a relation, so this should be NULL
                delete[] zh_rh_str;
              }
              zh_rh_str = RetrieveNestedSemDomainOperandStr(s_rover, true, error_code);
              zh_rh_ln = zh_rh_str ? strlen(zh_rh_str) : 0;
              zh_rv = AppendStr2HeapStr(zh_rv, zh_ln, zh_str);  //We do this here so zh_str won't be added to zh_rh_str below
              delete[] zh_str;
              zh_str = NULL;
              zh_str_ln = 0;
            }
          }
          else
            zh_str = ProcessSemNode(s_rover, NULL, error_code);
        }
      break;

      case SEM_TYP_INTERVAL:
        zh_str = INTERVALtoEstr(s_rover, error_code, true);
      break;

      case SEM_TYP_UCONSTANT:
      case SEM_TYP_VARIABLE:
        if (!strcmp(s_rover->contents, "&#x211d;"))
          zh_str = AppendStr2HeapStr(zh_str, zh_str_ln, "R_");
        else if (!strcmp(s_rover->contents, "&#x2102;"))
          zh_str = AppendStr2HeapStr(zh_str, zh_str_ln, "C_");
        else if (!strcmp(s_rover->contents, "&#x2124;"))
          zh_str = AppendStr2HeapStr(zh_str, zh_str_ln, "Z_");
        else if (!strcmp(s_rover->contents, "&#x211a;"))
          zh_str = AppendStr2HeapStr(zh_str, zh_str_ln, "Q_");
        else
          zh_str = ProcessSemNode(s_rover, NULL, error_code);
      break;
  
      default:
        zh_str = ProcessSemNode(s_rover, NULL, error_code);
      break;
    }
    if (zh_str)
    {
      zh_rv = AppendStr2HeapStr(zh_rv, zh_ln, zh_str);
      if (pastOp)
        zh_rh_str = AppendStr2HeapStr(zh_rh_str, zh_rh_ln, zh_str);  //We do this here so zh_str won't be added to zh_rh_str below
      delete[] zh_str;
      zh_str = NULL;
      zh_str_ln = 0;
    }
	s_rover = s_rover->next;
  }
  if (zh_rh_str)
    delete[] zh_rh_str;

  return zh_rv;
}

//char *MuPADWrapper::SemDomainToEstr(SEMANTICS_NODE * s_list, int& error_code)
//{
//  char *zh_rv = NULL;
//  U32 zh_ln = 0;
//
//  if (s_list && s_list->semantic_type == SEM_TYP_TEXT)
//  {
//    if (!strcmp(s_list->contents, "otherwise"))
//    {
//      zh_rv = AppendStr2HeapStr(zh_rv, zh_ln, "otherwise");  //this should act as "otherwise" if it's the last condition in the piecewise list
//      return zh_rv;
//    }
//    s_list = s_list->next;
//  }
//
//  while (s_list
//         && !s_list->next    // but s_list->next could be a precedence group??
//         && s_list->semantic_type == SEM_TYP_PRECEDENCE_GROUP) {
//    if (s_list->bucket_list && s_list->bucket_list->first_child)
//      s_list = s_list->bucket_list->first_child;
//    else
//      TCI_ASSERT(0);
//  }
//
//  int rel_pos = -1;
//  int rel_pos2 = -1;
//  int conn_pos = -1;
//  int conn_pos2 = -1;
//  int in_pos = -1;
//  int not_pos = -1;
//  int tally = 0;
//  SEMANTICS_NODE *s_lhs = NULL;
//  SEMANTICS_NODE *s_rhs = NULL;
//  SEMANTICS_NODE *s_lhs2 = NULL;
//  SEMANTICS_NODE *s_rhs2 = NULL;
//  SEMANTICS_NODE *s_rover = s_list;
//  while (s_rover) {
//    if (IsBinRelation(s_rover)) {
//      if (rel_pos < 0)
//        rel_pos = tally;
//      else if (rel_pos2 < 0)
//        rel_pos2 = tally;
//    } else if (IsLogicalConnectingOperator(s_rover)) {
//      if (conn_pos < 0)
//        conn_pos = tally;
//      else if (conn_pos2 < 0)
//        conn_pos2 = tally;
//    } else if (IsSetMembershipOperator(s_rover)) {
//      in_pos = tally;
//    } else if (IsNegationOperator(s_rover)) {
//      not_pos = tally;
//    }
//    tally++;
//    s_rover = s_rover->next;
//  }
//  if ((rel_pos >= 0) || (conn_pos >= 0) || (in_pos >= 0) || (not_pos >= 0)) //at least one relation or connecting operator
//  {
//    s_rover = s_list;
//    SEMANTICS_NODE *s_temp = NULL;
//    SEMANTICS_NODE *s_save = NULL;
//    tally = 0;
//    char *zh_lhs, *zh_rhs, *zh_str;
//    char *zh_lhs2, *zh_rhs2;
//    U32 zh_str_ln = 0;
//    while ( s_rover && ((rel_pos < 0) || (tally < rel_pos)) )
//    {
//      zh_str_ln = 0;
//      zh_str = NULL;
//      zh_lhs = NULL;
//      s_rhs = NULL;
//      //First check to see whether this operand is a nested binary relation
//      s_temp = FindBinRelation(s_rover, true);  //ask for the last binary relation inside this node - if it exists, we'll want its right-hand-side for the left-hand side of our relation
//      while (s_temp)
//      {
//        s_rhs = s_temp;
//        while (s_temp->next)
//          s_temp = s_temp->next;  //This should hopefully only be executed once at each level - otherwise there is more than one operand to the right of the relation?
//        s_temp = FindBinRelation(s_temp, true);
//      }
//      if (s_rhs)
//      {
//        s_rhs = s_rhs->next;  //move this pointer to after the nested relation
//        zh_lhs = ProcessSemList(s_rhs, NULL, error_code);  //
//      }
//
//      if ( s_rhs || ((conn_pos >= 0) && (tally==conn_pos-1 || tally==conn_pos+1)) || ((not_pos >= 0) && (tally==not_pos+1)) )  //if this is an operand of an "and" or "or", it should be a domain expression too
//      {
//        //Since this is reset each time through, it'll only be kept if the nested relation is the last thing before rel_pos
//        s_save = s_rover->next;
//        s_rover->next = NULL;
//        zh_str = SemDomainToEstr(s_rover, error_code);  //recurse to get the string for relation to the left
//        s_rover->next = s_save;
//        if ((not_pos >= 0) && (tally == not_pos+1))
//          zh_str = AppendStr2HeapStr(zh_str, zh_str_ln, ")");
//      }
//      else if (tally == in_pos)
//      {
//        zh_str = AppendStr2HeapStr(zh_str, zh_str_ln, " in ");
//      }
//      else if (tally == not_pos)
//      {
//        zh_str = AppendStr2HeapStr(zh_str, zh_str_ln, "not(");
//      }
//      else if (tally == conn_pos)
//      {
//        if (!strcmp(s_rover->contents, "&#x2227;"))
//          zh_str = AppendStr2HeapStr(zh_str, zh_str_ln, " and ");
//        else if (!strcmp(s_rover->contents, "&#x2228;"))
//          zh_str = AppendStr2HeapStr(zh_str, zh_str_ln, " or ");
//      }
//      else if (tally == in_pos+1)
//      {
//        if (s_rover->semantic_type == SEM_TYP_INTERVAL)
//          zh_str = INTERVALtoEstr(s_rover, error_code, true);
//        else if (s_rover->semantic_type == SEM_TYP_UCONSTANT || s_rover->semantic_type == SEM_TYP_VARIABLE)
//        {
//          if (!strcmp(s_rover->contents, "&#x211d;"))
//            zh_str = AppendStr2HeapStr(zh_str, zh_str_ln, "R_");
//          else if (!strcmp(s_rover->contents, "&#x2102;"))
//            zh_str = AppendStr2HeapStr(zh_str, zh_str_ln, "C_");
//          else if (!strcmp(s_rover->contents, "&#x2124;"))
//            zh_str = AppendStr2HeapStr(zh_str, zh_str_ln, "Z_");
//          else if (!strcmp(s_rover->contents, "&#x211a;"))
//            zh_str = AppendStr2HeapStr(zh_str, zh_str_ln, "Q_");
//          else
//            zh_str = ProcessSemNode(s_rover, NULL, error_code);
//        }
//        else
//          zh_str = ProcessSemNode(s_rover, NULL, error_code);
//      }
//      else
//      {
//        zh_str = ProcessSemNode(s_rover, NULL, error_code);
//      }
//      if (zh_str)
//      {
//        zh_rv = AppendStr2HeapStr(zh_rv, zh_ln, zh_str);
//        delete[] zh_str;
//      }
//      ++tally;
//      s_rover = s_rover->next;
//    }
//    char* zh_rel = NULL;
//    //Now we've reached our (first) binary relation
//    if (tally==rel_pos)     //There was a nested binary relation; s_rhs points to its right-hand argument, which will be our left-hand argument
//    {
//      if (zh_lhs)
//      {
//        zh_rv = AppendStr2HeapStr(zh_rv, zh_ln, " and ");  //we need this since we know there was a binary relation to our left
//        zh_rv = AppendStr2HeapStr(zh_rv, zh_ln, zh_lhs);
//      }
//      zh_rel = ProcessSemNode(s_rover, NULL, error_code);
//      zh_rv = AppendStr2HeapStr(zh_rv, zh_ln, zh_rel);
//      ++tally;
//      s_rover = s_rover->next;
//    }
//    zh_rhs = NULL;
//    while ( s_rover && ((rel_pos2 < 0) || (tally < rel_pos2)) )
//    {
//      zh_str = NULL;
//      zh_str_ln = 0;
//      zh_lhs2 = NULL;
//      s_lhs = NULL;
//      s_rhs = NULL;
//      s_lhs2 = NULL;
//      SEMANTICS_NODE *s_temp2 = FindBinRelation(s_rover, true);  //ask for the last binary relation inside this node
//      if (s_temp2)
//      {
//        s_save = s_rover->next;
//        s_rover->next = NULL;
//        zh_str = SemDomainToEstr(s_rover, error_code);  //recurse to get the string for child relation
//        s_rover->next = s_save;
//      }
//      else
//        zh_str = ProcessSemNode(s_rover, NULL, error_code);
//      if (s_temp2 && (rel_pos >= 0) && (tally==rel_pos + 1))
//      {
//        s_temp = FindBinRelation(s_rover, false);  //ask for the first binary relation inside this node unless we're looking for the LHS of a second relation - we'll want its right-hand-side for the left-hand side of our relation
//        while (s_temp)
//        {
//          s_lhs = s_temp;
//          while (s_temp->prev)
//            s_temp = s_temp->prev;  //This should hopefully only be executed once at each level - otherwise there is more than one operand to the left of the relation?
//          s_temp = FindBinRelation(s_temp, false);
//        }
//        if (s_lhs)
//        {
//          s_save = s_lhs;
//          s_lhs = s_lhs->prev;
//          s_lhs->next = NULL;
//          zh_rhs = ProcessSemList(s_lhs->parent->first_child, NULL, error_code);  //this is the right-hand side of the current relation
//          s_lhs->next = s_save;  //restore it
//        }
//      }
//      if (tally == rel_pos2 - 1)
//      {
//        while (s_temp2)  //now we'll want its right-hand-side for the left-hand side of our next relation
//        {
//          s_rhs = s_temp2;
//          while (s_temp2->next)
//            s_temp2 = s_temp2->next;  //This should hopefully only be executed once at each level - otherwise there is more than one operand to the left of the relation?
//          s_temp2 = FindBinRelation(s_temp, true);
//        }
//      }
//      if (s_rhs)
//      {
//        s_rhs = s_rhs->next;  //move to after the relation
//        zh_lhs2 = ProcessSemList(s_rhs, NULL, error_code);
//      }
//      if (zh_rhs)
//      {
//        zh_rv = AppendStr2HeapStr(zh_rv, zh_ln, zh_rhs);
//        zh_rv = AppendStr2HeapStr(zh_rv, zh_ln, " and ");  //we need this since we know there was a binary relation to our left
//      }
//      zh_rv = AppendStr2HeapStr(zh_rv, zh_ln, zh_str);
//      if (zh_lhs2)
//        zh_rv = AppendStr2HeapStr(zh_rv, zh_ln, zh_lhs2);
//      ++tally;
//      s_rover = s_rover->next;
//    }
//    if (s_rover)  //this is the (apparently rare) case where there is a second relation in the same list
//    {
//      zh_rel = ProcessSemNode(s_rover, NULL, error_code);
//      zh_rv = AppendStr2HeapStr(zh_rv, zh_ln, zh_rel);
//      ++tally;
//      s_rover = s_rover->next;
//    }
//    while (s_rover)
//    {
//      zh_rhs2 = NULL;
//      s_lhs2 = NULL;
//      s_temp = FindBinRelation(s_rover, false);  //ask for the first binary relation inside this node
//      if (s_temp)
//      {
//        s_save = s_rover->next;
//        s_rover->next = NULL;
//        zh_str = SemDomainToEstr(s_rover, error_code);  //recurse to get the string for child relation
//        s_rover->next = s_save;
//      }
//      else
//        zh_str = ProcessSemNode(s_rover, NULL, error_code);
//      while (s_temp)
//      {
//        s_lhs2 = s_temp;
//        while (s_temp->prev)
//          s_temp = s_temp->prev;  //This should hopefully only be executed once at each level - otherwise there is more than one operand to the left of the relation?
//        s_temp = FindBinRelation(s_temp, false);
//      }
//      if (s_lhs2 && (rel_pos >= 0) && (tally == rel_pos + 1))
//      {
//        s_save = s_lhs2;
//        s_lhs2 = s_lhs2->prev;
//        s_lhs2->next = NULL;
//        zh_rhs2 = ProcessSemList(s_lhs2->parent->first_child, NULL, error_code);  //this is the right-hand side of the current relation
//        s_lhs2->next = s_save;  //restore it
//      }
//      if (zh_rhs2)
//      {
//        zh_rv = AppendStr2HeapStr(zh_rv, zh_ln, zh_rhs2);
//        zh_rv = AppendStr2HeapStr(zh_rv, zh_ln, " and ");  //we need this since we know there was a binary relation to our left
//      }
//      if (zh_str)
//      {
//        zh_rv = AppendStr2HeapStr(zh_rv, zh_ln, zh_str);
//        delete[] zh_str;
//      }
//    }
//
////  if (s_var) {
////    if (tally == 3)
////      zh_rv = ProcessSemList(s_list, NULL, error_code);
////    else {
////      if (tally == 5 && var_pos == 2) {
////        SEMANTICS_NODE *s_save = s_var->next;
////        s_var->next = NULL;
////        char *zh_str = ProcessSemList(s_list, NULL, error_code);
////        zh_rv = AppendStr2HeapStr(zh_rv, zh_ln, zh_str);
////        delete[] zh_str;
////
////        s_var->next = s_save;
////        char *zh_str1 = ProcessSemList(s_var, NULL, error_code);
////        if (zh_str1) {
////          zh_rv = AppendStr2HeapStr(zh_rv, zh_ln, " and ");
////          zh_rv = AppendStr2HeapStr(zh_rv, zh_ln, zh_str1);
////          delete[] zh_str1;
////        }
////      } else {
////        TCI_ASSERT(0);
////      }
////    }
//  } else {
//    TCI_ASSERT(0);
//  }
//  return zh_rv;
//}

// The semantic analyzer identifies ODE funcs and records them
//  in a MB_ODEFUNC bucket in the bucket_list of an SEM_TYP_ODE_SYSTEM.

void MuPADWrapper::ODEFuncName2eStr(SEMANTICS_NODE * s_func,
                                    char **sfor_func_str, U32 & sfor_ln)
{
  if (s_func
      && s_func->semantic_type == SEM_TYP_FUNCTION
      && s_func->canonical_ID) {
    int err_code = 0;
    char *zh_nom = GetEngNameForODEfunc(s_func, err_code);
    if (zh_nom) {
      bool do_it = true;
      if (sfor_ln) {
        char *ptr = strstr(*sfor_func_str, zh_nom);
        if (ptr)
          do_it = false;
      }
      if (do_it) {
        if (sfor_ln)
          *sfor_func_str = AppendStr2HeapStr(*sfor_func_str, sfor_ln, ",");
        *sfor_func_str = AppendStr2HeapStr(*sfor_func_str, sfor_ln, zh_nom);
      }
      delete[] zh_nom;
    }
  } else
    TCI_ASSERT(0);
}

//  "hold(q1x),hold(q1y),hold(q1z)"

bool MuPADWrapper::NumericFuncName2eStr(SEMANTICS_NODE * s_func,
                                            char **names_str, U32 & names_ln)
{
  bool rv = false;

  if (s_func
      && s_func->semantic_type == SEM_TYP_FUNCTION
      && s_func->canonical_ID) {
    int err_code = 0;
    char *zh_nom = GetEngNameForODEfunc(s_func, err_code);
    if (zh_nom) {
      bool do_it = true;
      if (names_ln) {
        char *ptr = strstr(*names_str, zh_nom);
        if (ptr)
          do_it = false;
      }
      if (do_it) {
        char *ptr = strstr(zh_nom, "ODE");
        if (ptr)
          *ptr = 0;
        else
          TCI_ASSERT(0);

        char hold[256];
        strcpy(hold, "hold(");
        strcat(hold, zh_nom);
        strcat(hold, ")");

        if (names_ln)
          *names_str = AppendStr2HeapStr(*names_str, names_ln, ",");
        *names_str = AppendStr2HeapStr(*names_str, names_ln, hold);
        rv = true;
      }
      delete[] zh_nom;
    }
  } else
    TCI_ASSERT(0);

  return rv;
}

bool MuPADWrapper::GetdBaseNamedRecord(const char *bin_name,
                                           const char *op_name,
                                           const char **eng_dbase_rec,
                                           U32 & ID, U32 & subID)
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

bool MuPADWrapper::GetdBaseIDRecord(const char *bin_name, U32 ID,
                                        U32 subID, const char **eng_dbase_rec,
                                        char *name_buffer, size_t lim)
{
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
    return true;
  }

  return false;
}

void MuPADWrapper::iDisposeSList(SEMANTICS_NODE * slist)
{
  DisposeSList(slist);
}

int MuPADWrapper::iGetEngStrPtr(int str_ID, const char **engstr, int *engstr_ln)
{
  return mupd_eng->GetEngStrPtr(str_ID, engstr, engstr_ln);
}

/*
FUNCTION canonical_ID = "miNormalDist" contents = "NormalDist"
  UNNAMED
    INFIX OPERATOR contents = ";"
      UNNAMED
        NUMBER canonical_ID = "mn2.44" contents = "2.44"
      UNNAMED
        PRECEDENCE_GROUP
        Bucket list
          UNNAMED
            INFIX OPERATOR contents = ","
            Bucket list
              UNNAMED
                NUMBER canonical_ID = "mn0" contents = "0"
              UNNAMED
                NUMBER canonical_ID = "mn1" contents = "1"
            End Bucket list
        End Bucket list
*/

char *MuPADWrapper::PROBFUNCtoEstr(SEMANTICS_NODE * sem_func_node,
                                   const char *func_template, U32 ID,
                                   int& error_code)
{
  char *zh_rv = NULL;

  // func_template = tciuniforminv(%val%,%shape1%,%shape2%)
  char *str_val = NULL;
  char *str_s1 = NULL;
  char *str_s2 = NULL;
  char *str_s3 = NULL;

  if (sem_func_node && sem_func_node->bucket_list) {
    BUCKET_REC *bucket = sem_func_node->bucket_list;
    if (bucket && bucket->first_child) {
      //  INFIX OPERATOR contents = ";"
      SEMANTICS_NODE* s_semi = bucket->first_child;
      if (s_semi->bucket_list && StringEqual(s_semi->contents, ";")) {
        BUCKET_REC *op_rover = s_semi->bucket_list;

        SEMANTICS_NODE *s_var = op_rover->first_child;
        str_val = ProcessSemNode(s_var, NULL, error_code);
        if (error_code) {
          TCI_ASSERT(0);
          return NULL;
        }

        op_rover = op_rover->next;
        if (op_rover && op_rover->first_child) {
          SEMANTICS_NODE *s_op2 = op_rover->first_child;
          GetShapeArgs(s_op2, &str_s1, &str_s2, &str_s3, error_code);
          if (error_code) {
            delete[] str_val;
            return NULL;
          }
        } else {
          TCI_ASSERT(0);
        }
      } else {
        // Here the arg and any shape params are all separated by commas.
        //  They are carried in sequential unnamed buckets - like the
        //  semantic object for an ordinary function.

        str_val = ProcessSemNode(s_semi, NULL, error_code);
        if (error_code) {
          TCI_ASSERT(0);
          return NULL;
        }

        if (bucket->next && bucket->next->first_child) {
          SEMANTICS_NODE *s_op2 = bucket->next->first_child;
          GetShapeArgs(s_op2, &str_s1, &str_s2, &str_s3, error_code);
          if (error_code) {
            delete[] str_val;
            return NULL;
          }
        }
      }
    } else {
      TCI_ASSERT(0);
    }
    if (str_val) {
      if (!str_s1) {
        if (ID == 8113) {       // NormalDist
          const char *eng_dbase_rec;
          char p_name[80];
          if (GetdBaseIDRecord("FUNCTIONS", 8113, 1, &eng_dbase_rec, p_name, 80))
            func_template = eng_dbase_rec;
          else
            TCI_ASSERT(0);
        } else {
          TCI_ASSERT(0);
        }
      }

      size_t bln = strlen(func_template) + strlen(str_val);
      if (str_s1) {
        bln += strlen(str_s1);
        if (str_s2) {
          bln += strlen(str_s2);
          if (str_s3)
            bln += strlen(str_s3);
        }
      }
      zh_rv = new char[bln];
      strcpy(zh_rv, func_template);
      StrReplace(zh_rv, bln, "%val%", str_val);
      if (str_s1) {
        StrReplace(zh_rv, bln, "%shape1%", str_s1);
        if (str_s2) {
          StrReplace(zh_rv, bln, "%shape2%", str_s2);
          if (str_s3)
            StrReplace(zh_rv, bln, "%shape3%", str_s3);
        }
      }
      delete[] str_val;
      delete[] str_s1;
      delete[] str_s2;
      delete[] str_s3;
    }
  } else {
    TCI_ASSERT(0);
  }
  return zh_rv;
}

// transform::fourier(%expr%,%var%,%tvar%);

char *MuPADWrapper::TRANSFORMtoEstr(SEMANTICS_NODE * s_transform,
                                    const char *expr,
                                    const char *cmd_template,
                                    int& error_code)
{
  char *zh_rv = NULL;
  error_code = 0;

  const char *production = NULL;
  char *expr_str = NULL;
  char *var_str = NULL;
  char *tvar_str = NULL;

  if (s_transform) {
    bool is_Laplace = false;
    if (!strcmp(s_transform->contents, "&#x2112;"))
      is_Laplace = true;

    int arg_counter = 0;
    BUCKET_REC *bucket_rover = s_transform->bucket_list;
    while (bucket_rover && !error_code) {
      if (bucket_rover->bucket_ID == MB_UNNAMED) {
        if (bucket_rover->first_child) {
          SEMANTICS_NODE *curr_handle = bucket_rover->first_child;
          char *obj_str = ProcessSemList(curr_handle, NULL, error_code);
          if (obj_str && !error_code) {
            if (arg_counter == 0)
              expr_str = obj_str;
            else if (arg_counter == 1)
              var_str = obj_str;
            else if (arg_counter == 2)
              tvar_str = obj_str;
          }
        }
      }
      arg_counter++;
      bucket_rover = bucket_rover->next;
    }
    if (error_code) {
      delete[] expr_str;
      delete[] var_str;
      delete[] tvar_str;
      return NULL;
    }

    if (!expr_str) 
      return NULL;

    bool is_mapped = false;
    if (!strncmp(expr_str, "SWPmatrix", 9))
      is_mapped = true;

    const char *key = NULL;
    if (s_transform->semantic_type == SEM_TYP_TRANSFORM) {
      if (is_Laplace)
        key = is_mapped ? "LaplaceMapped" : "Laplace";
      else
        key = is_mapped ? "FourierMapped" : "Fourier";
    } else if (s_transform->semantic_type == SEM_TYP_INVTRANSFORM) {
      if (is_Laplace)
        key = is_mapped ? "InvLaplaceMapped" : "InvLaplace";
      else
        key = is_mapped ? "InvFourierMapped" : "InvFourier";
    } else {
      TCI_ASSERT(0);
    }
    const char *db_record;
    U32 cmd_ID, subID;
    if (GetdBaseNamedRecord("SCHEMATA", key, &db_record, cmd_ID, subID))
      production = db_record;
    else
      TCI_ASSERT(0);
  } else if (expr) {
    size_t bln = strlen(expr);
    expr_str = new char[bln + 1];
    strcpy(expr_str, expr);
    production = cmd_template;
  } else {
    TCI_ASSERT(0);
  }
  // Generally, the variable of integration is not specified
  //  in the input.  Here SWP assumes it is "t" and the transform
  //  variable is "s".

  if (!var_str) {
    const char *is_laplace = strstr(production, "laplace");

    const char *nom = is_laplace ? "s" : "w";
    const char *t_nom = is_laplace ? "t" : "x";

    char user_name[2];
    bool use_alias = false;
    strcpy(user_name, t_nom);
    var_str = CreateEngName(client_ID, NULL, user_name, use_alias, Ilk_Var);
    if (!tvar_str) {
      strcpy(user_name, nom);
      tvar_str = CreateEngName(client_ID, NULL, user_name, use_alias, Ilk_Var);
    }
    const char *ptr = strstr(production, "inv");
    if (ptr) {
      char *tmp = tvar_str;
      tvar_str = var_str;
      var_str = tmp;
    }
  }
  if (expr_str && var_str && tvar_str) {
    // e^{-5iw}Fourier(f)
    // 
    // e^{-5iw}Fourier(f(x))
    size_t bln = strlen(production) + strlen(expr_str)
      + strlen(var_str) + strlen(tvar_str);
    zh_rv = new char[bln];
    strcpy(zh_rv, production);
    StrReplace(zh_rv, bln, "%expr%", expr_str);
    StrReplace(zh_rv, bln, "%var%", var_str);
    StrReplace(zh_rv, bln, "%tvar%", tvar_str);
  }
  delete[] expr_str;
  delete[] var_str;
  delete[] tvar_str;

  return zh_rv;
}

// Gradient<uID53.0>tcigrad(%expr%,[%vector_basis%]);

char *MuPADWrapper::VCALCOPtoEstr(MathServiceRequest * msr,
                                  SEMANTICS_NODE * s_func, U32 func_ID,
                                  const char *expr, const char *cmd_template,
                                  int& error_code)
{
  char *zh_rv = NULL;
  error_code = 0;

  const char *production = NULL;

  char *expr_str = NULL;
  char *basis_str = NULL;

  if (s_func) {                 // Here, we're evaluating a vcalc operator
    U32 subID = 0;
    const char *eng_dbase_rec;
    char p_name[80];
    if (GetdBaseIDRecord("PREFIXOPS", func_ID, subID, &eng_dbase_rec, p_name, 80))
      production = eng_dbase_rec;
    else
      TCI_ASSERT(0);

    BUCKET_REC *bucket_rover = s_func->bucket_list;
    if (bucket_rover->bucket_ID == MB_UNNAMED && bucket_rover->first_child) {
      int err_code = 0;
      SEMANTICS_NODE *curr_handle = bucket_rover->first_child;
      char *obj_str = ProcessSemList(curr_handle, NULL, err_code);
      if (obj_str && err_code != 1)
        expr_str = obj_str;
    } else {
      TCI_ASSERT(0);
    }
  } else if (expr) {            // Here, we're we're responding to a vcalc menu command
    size_t bln = strlen(expr);
    expr_str = new char[bln + 1];
    strcpy(expr_str, expr);
    production = cmd_template;
  } else {
    TCI_ASSERT(0);
  }

  U32 p_type;
  const char *z_var = msr->GetParam(PID_VecBasisVariables, p_type);
  if (p_type == zPT_ASCII_mmlmarkup) {
    SEMANTICS_NODE *sn = msr->GetSemanticsFromParam(PID_VecBasisVariables);
    if (sn) {
      basis_str = ProcessSemNode(sn, NULL, error_code);
      if (error_code)
        return NULL;
    } else {
      TCI_ASSERT(0);
    }
  } else {
    TCI_ASSERT(0);
  }

  if (expr_str && basis_str) {
    size_t bln = strlen(production) + strlen(expr_str) + strlen(basis_str);
    zh_rv = new char[bln];
    strcpy(zh_rv, production);
    StrReplace(zh_rv, bln, "%expr%", expr_str);
    StrReplace(zh_rv, bln, "%vector_basis%", basis_str);

  }
  delete[] expr_str;
  delete[] basis_str;

  if (error_code && zh_rv) {
    delete[] zh_rv;
    zh_rv = NULL;
  }

  return zh_rv;
}

char *MuPADWrapper::POLISHOPtoEstr(SEMANTICS_NODE * s_rpolish_op,
                                   const char *modulus, bool & is_infix,
                                   int& error_code)
{
  char *rv = NULL;
  error_code = 0;
  is_infix = false;

  const char *production = NULL;
  U32 ID, subID;
  char *key = s_rpolish_op->contents;

  int req_eng_format;
  const char *db_section;
  if (s_rpolish_op->semantic_type == SEM_TYP_PREFIX_OP) {
    req_eng_format = 1;
    db_section = "PREFIXOPS";
    //grad<uID8176.0>tcigrad(%expr%,[%vector_basis%])

    if (!strcmp(key, "det"))
      req_eng_format = 2;
  } else if (s_rpolish_op->semantic_type == SEM_TYP_INFIX_OP) {
    // some semantic infix forms translate to prefix when the operands
    //  are vectors or matrices
    bool done = false;
    if (context_nmatrices) {
      if (GetdBaseNamedRecord
          ("MATRIXPREFIXFORMS", key, &production, ID, subID)) {
        int n_matrix_operands = CountMatrixOperands(s_rpolish_op);
        if (n_matrix_operands == 1) {
          req_eng_format = 2;
          db_section = "MATRIXPREFIXFORMS";
          done = true;
        } else if (n_matrix_operands == 2) {
          req_eng_format = 2;
          db_section = "DMATRIXPREFIXFORMS";
          done = true;
        }
      }
    }
    if (!done) {
      if (GetdBaseNamedRecord("PREFIXFORMS", key, &production, ID, subID)) {
        req_eng_format = 2;
        db_section = "PREFIXFORMS";
      } else {
        req_eng_format = 3;
        db_section = "INFIXOPS";
      }
    }
  } else if (s_rpolish_op->semantic_type == SEM_TYP_POSTFIX_OP) {
    req_eng_format = 4;
    db_section = "POSTFIXOPS";
  }

  if (GetdBaseNamedRecord(db_section, key, &production, ID, subID)) {
    if (production && production[0]) {
      if (req_eng_format == 1) {
        // gradient, divergence, curl , Laplacian
        if (ID == 8176 || ID == 8177 || ID == 8178 || ID == 8179) {
          return VCALCOPtoEstr(curr_msr, s_rpolish_op, ID, NULL, NULL,
                               error_code);
        }
        // prefix "+" or "-"
        char zoperator[80];
        char ch = production[0];
        if (ch >= 'a' && ch <= 'z') { // "union"
          zoperator[0] = ' ';
          strcpy(zoperator + 1, production);
          strcat(zoperator, " ");
        } else
          strcpy(zoperator, production);

        char *operand =
          LISTtoEstr(s_rpolish_op, NULL, NULL, NULL, modulus, error_code);
        if (error_code) {
          delete[] operand;
          return NULL;
        }

        size_t zln = strlen(zoperator) + strlen(operand);
        rv = new char[zln + 1];
        strcpy(rv, zoperator);
        strcat(rv, operand);
        delete[] operand;
      } else if (req_eng_format == 2) { // engine form is prefix
        // tcidprod(operand,operand)      
        const char *ptr = strstr(production, "%arg_list%");
        if (ptr) {
          int err_code = 0;

          char *args_str = NULL;
          if (!strcmp(key, "mod")) {
            args_str = MODtoEstr(s_rpolish_op, err_code);
            if (context_cmdtype == CC_FACTOR_IN_RING) {
              size_t bln = strlen(args_str) + 1;
              rv = new char[bln];
              strcpy(rv, args_str);
            } else {
              args_str = NestInParens(args_str, true);
              size_t bln = strlen(production) + 1;
              if (args_str)
                bln += strlen(args_str);
              rv = new char[bln];
              strcpy(rv, production);
              StrReplace(rv, bln, "%arg_list%", args_str);
            }
          } else {
            args_str =
              LISTtoEstr(s_rpolish_op, NULL, NULL, ",", modulus, err_code);
            args_str = NestInParens(args_str, true);
            size_t bln = strlen(production) + 1;
            if (args_str)
              bln += strlen(args_str);
            rv = new char[bln];
            strcpy(rv, production);
            StrReplace(rv, bln, "%arg_list%", args_str);
          }
          if (err_code) {
            error_code = err_code;
            return rv;
          }
          delete[] args_str;
        } else
          TCI_ASSERT(0);
      } else if (req_eng_format == 3) { // engine is infix
        // operand opname operand
        char op_str[80];
        char ch = production[0];
        if (ch >= 'a' && ch <= 'z') { // "union"
          op_str[0] = ' ';
          strcpy(op_str + 1, production);
          strcat(op_str, " ");
        } else
          strcpy(op_str, production);

        int err_code = 0;
        rv = LISTtoEstr(s_rpolish_op, NULL, NULL, op_str, modulus, err_code);
        if (err_code) {
          delete[] rv;
          error_code = err_code;
          return NULL;
        }
        is_infix = true;
      } else if (req_eng_format == 4) { // engine form is postfix
        char zoperator[80];
        char ch = production[0];
        if (ch >= 'a' && ch <= 'z') { // "union"
          zoperator[0] = ' ';
          strcpy(zoperator + 1, production);
          strcat(zoperator, " ");
        } else
          strcpy(zoperator, production);

        int err_code = 0;
        char *operand =
          LISTtoEstr(s_rpolish_op, NULL, NULL, NULL, modulus, err_code);
        if (err_code) {
          delete[] operand;
          error_code = err_code;
          return NULL;
        }

        size_t zln = strlen(operand) + strlen(zoperator);
        rv = new char[zln + 1];
        strcpy(rv, operand);
        strcat(rv, zoperator);
        delete[] operand;
      }
    } else {
      TCI_ASSERT(!"no db_production");
      error_code = CR_failure;
    }
  } else {
    TCI_ASSERT(!"no db entry");
    error_code = CR_failure;
  }
  return rv;
}

int MuPADWrapper::CountMatrixOperands(SEMANTICS_NODE * s_rpolish_op)
{
  int n_operands = 0;
  int n_matrices = 0;
  BUCKET_REC *b_rover = s_rpolish_op->bucket_list;
  while (b_rover) {
    n_operands++;
    if (b_rover->first_child) {
      SEMANTICS_NODE *s_operand = b_rover->first_child;
      if (s_operand->semantic_type == SEM_TYP_PRECEDENCE_GROUP
          && s_operand->bucket_list && !s_operand->next) {
        s_operand = s_operand->bucket_list->first_child;
      }
      if (s_operand->semantic_type == SEM_TYP_TABULATION) {
        n_matrices++;
      } else {
        if (context_nmatrices)
          if (QueryEngineIsMatrix(s_operand))
            n_matrices++;
      }
    } else {
      TCI_ASSERT(0);
    }
    b_rover = b_rover->next;
  }

  return n_matrices;
}

bool MuPADWrapper::QueryEngineIsMatrix(SEMANTICS_NODE * s_expr)
{
  bool rv = false;

  if (s_expr && s_expr->semantic_type != SEM_TYP_NUMBER) {

    bool done = false;

    if (s_expr->subtree_type == 1)
      return true;

    if (!s_expr->next
        && (s_expr->semantic_type == SEM_TYP_VARIABLE
            || s_expr->semantic_type == SEM_TYP_QUALIFIED_VAR)
        && context_variables) {
      ClientAliases_REC *client_aliases = GetClientAliasesRec(client_ID);
      NAME2AKA_REC *map = GetAKARec(client_aliases->AKAs_list,
                                    s_expr->canonical_ID,
                                    s_expr->contents);
      if (map) {
        VAR_REC *vr = FindVarRec(context_variables, map->alias);
        if (vr) {
          done = true;
          rv = (vr->var_type == VT_Matrix) ? true : false;
        }
      }
    }

//#ifdef DO_WE_REALLY_NEED_THIS_ON_MUPAD
//jcs -- apparently we do
    if (!done) {
      int err_code;
      char *v_str = ProcessSemList(s_expr, NULL, err_code);

      if (!err_code && v_str) {
        const char *db_production;  // testtypeMatrix<uID100.0>testtype(eval(%var%),Dom::Matrix);
        U32 ID, subID;
        if (GetdBaseNamedRecord
            ("SCHEMATA", "testtypeMatrix", &db_production, ID, subID)) {
          if (db_production && *db_production) {
            size_t bln = strlen(db_production) + strlen(v_str);
            char *cmd = new char[bln];
            strcpy(cmd, db_production);
            StrReplace(cmd, bln, "%var%", v_str);

            ContextType save_context = context_cmdtype;
            context_cmdtype = CC_EVAL;
            char *eng_ret = Execute(cmd);
            context_cmdtype = save_context;

            if (eng_ret) {
              if (!strcmp(eng_ret, "TRUE"))
                rv = true;
              delete[] eng_ret;
            }
            delete[] cmd;
          } else
            TCI_ASSERT(0);
        } else
          TCI_ASSERT(0);
      } else
        TCI_ASSERT(0);
      delete[] v_str;
    }
//#endif
  }

  if (rv)
    s_expr->subtree_type = 1;

  return rv;
}

// Extract the "shape" arguments from a semantic probability function
//  where the "," separated args are in prefix form.  Note that the first param,
//  separated by ";", has been handled before this function is called.

void MuPADWrapper::GetShapeArgs(SEMANTICS_NODE * s_op2,
                                char **str_s1, char **str_s2, char **str_s3,
                                int& error_code)
{
  SEMANTICS_NODE *s_comma = s_op2;
  error_code = 0;

  if (s_comma->semantic_type == SEM_TYP_PRECEDENCE_GROUP) {
    BUCKET_REC *bucket = s_comma->bucket_list;
    if (bucket && bucket->first_child)
      s_comma = bucket->first_child;
    else {
      TCI_ASSERT(0);
      return;
    }
  }

  if (s_comma->semantic_type == SEM_TYP_INFIX_OP) {
    //    INFIX OPERATOR contents = ","

    BUCKET_REC *op_rover = s_comma->bucket_list;

    SEMANTICS_NODE *s_child = op_rover->first_child;
    if (s_child->semantic_type == SEM_TYP_INFIX_OP) {
      // Here, 3 args occurs after the ";"
      BUCKET_REC *b_rover = s_comma->bucket_list;
      SEMANTICS_NODE *s_child1 = b_rover->first_child;

      *str_s1 = ProcessSemNode(s_child1, NULL, error_code);
      if (error_code) {
        return;
      }

      b_rover = b_rover->next;
      SEMANTICS_NODE *s_child2 = b_rover->first_child;
      *str_s2 = ProcessSemNode(s_child2, NULL, error_code);
      if (error_code) {
        delete[] *str_s1;
        return;
      }

      op_rover = op_rover->next;
      SEMANTICS_NODE *s_child3 = op_rover->first_child;
      *str_s3 = ProcessSemNode(s_child3, NULL, error_code);
      if (error_code) {
        delete[] *str_s1;
        delete[] *str_s2;
        return;
      }
    } else {
      // Here, only 2 args occurs after the ";"
      *str_s1 = ProcessSemNode(s_child, NULL, error_code);
      if (error_code) {
        return;
      }
      op_rover = op_rover->next;
      SEMANTICS_NODE *s_child2 = op_rover->first_child;
      *str_s2 = ProcessSemNode(s_child2, NULL, error_code);
      if (error_code) {
        delete[] *str_s1;
        return;
      }
    }
  } else {
    // Here, only 1 shape param occurs after the ";"
    //  OR there were only comma separators in the list
    *str_s1 = ProcessSemNode(s_comma, NULL, error_code);
    if (error_code) {
      return;
    }

    if (s_comma->parent) {
      BUCKET_REC *b_rover = s_comma->parent;
      b_rover = b_rover->next;
      if (b_rover) {
        SEMANTICS_NODE *s_child2 = b_rover->first_child;
        *str_s2 = ProcessSemNode(s_child2, NULL, error_code);
        if (error_code) {
          delete[] *str_s1;
          return;
        }

        b_rover = b_rover->next;
        if (b_rover) {
          SEMANTICS_NODE *s_child3 = b_rover->first_child;
          *str_s3 = ProcessSemNode(s_child3, NULL, error_code);
          if (error_code) {
            delete[] *str_s1;
            delete[] *str_s2;
            return;
          }
        }
      }
    }
  }
}

// Function to handle args of tcimod(arg1,modulus).
//  Special handling for POWERFORMS in the context of the first arg.

char *MuPADWrapper::MODtoEstr(SEMANTICS_NODE * s_mod, int& error_code)
{
  char *rv = NULL;
  error_code = 0;

  if (s_mod && s_mod->bucket_list && s_mod->bucket_list->first_child) {

    char *arg1_str = NULL;
    char *modulus_str = NULL;

    // Locate and translate the modulus
    BUCKET_REC *bucket = s_mod->bucket_list;
    if (bucket->next && bucket->next->first_child) {
      modulus_str = ProcessSemList(bucket->next->first_child,
                                   NULL, error_code);
      if (error_code) {
        delete[]modulus_str;
        return NULL;
      }

      SEMANTICS_NODE *s_mod_arg1 = s_mod->bucket_list->first_child;
      arg1_str = ProcessSemList(s_mod_arg1, modulus_str, error_code);
      if (error_code) {
        delete[] modulus_str;
        delete[] arg1_str;
        return NULL;
      }

      size_t rv_ln = strlen(arg1_str) + strlen(modulus_str);
      rv = new char[rv_ln + 2];
      strcpy(rv, arg1_str);
      strcat(rv, ",");
      strcat(rv, modulus_str);

      delete[] modulus_str;
      delete[] arg1_str;
    } else
      TCI_ASSERT(0);
  } else {
    TCI_ASSERT(0);
  }
  return rv;
}

char *MuPADWrapper::POWERMODtoEstr(SEMANTICS_NODE * s_powerform,
                                   const char *modulus, int& error_code)
{
  char *rv = NULL;
  error_code = 0;

  const char *db_production;    // Power Mod<uID37.0>tcipowermod(%base%,%superscript%,%modulus%)
  U32 ID, subID;
  if (GetdBaseNamedRecord("SCHEMATA", "Power Mod", &db_production, ID, subID)) {
    TCI_ASSERT(db_production && *db_production);

    char *base_str = NULL;
    char *sup_str = NULL;

    BUCKET_REC *bucket =
      FindBucketRec(s_powerform->bucket_list, MB_SCRIPT_BASE);
    if (bucket && bucket->first_child) {
      base_str = ProcessSemList(bucket->first_child, modulus, error_code);
      if (error_code) {
        delete[] base_str;
        return NULL;
      }
    }

    bucket = FindBucketRec(s_powerform->bucket_list, MB_SCRIPT_UPPER);
    if (bucket && bucket->first_child) {
      sup_str = ProcessSemList(bucket->first_child, NULL, error_code);
      if (error_code) {
        delete[] base_str;
        delete[] sup_str;
        return NULL;
      }
    }
    // Compose the return string
    if (base_str && sup_str && modulus) {
      size_t bln =
        strlen(db_production) + strlen(base_str) + strlen(sup_str) +
        strlen(modulus);
      rv = new char[bln];
      strcpy(rv, db_production);
      StrReplace(rv, bln, "%base%", base_str);
      StrReplace(rv, bln, "%superscript%", sup_str);
      StrReplace(rv, bln, "%modulus%", modulus);
    }

    delete[] base_str;
    delete[] sup_str;
  } else {
    TCI_ASSERT(0);
  }
  return rv;
}

char *MuPADWrapper::GetEngNameForODEfunc(SEMANTICS_NODE * s_func,
                                         int& error_code)
{
  char *rv = NULL;
  error_code = 0;

  char *f_nom_str = NULL;

  // See if we already have an engine name for this function.
  ClientAliases_REC *client_aliases = GetClientAliasesRec(client_ID);
  NAME2AKA_REC *map = GetAKARec(client_aliases->AKAs_list,
                                s_func->canonical_ID,
                                s_func->contents);
  if (map && map->obj_ilk == Ilk_ODE) {
    size_t zln = strlen(map->alias);
    f_nom_str = new char[zln + 1];
    strcpy(f_nom_str, map->alias);
  } else {
    char base_name[128];
    base_name[0] = 0;
    if (s_func->contents)
      strcpy(base_name, s_func->contents);
    bool use_alias = true;
    f_nom_str = CreateEngName(client_ID, s_func->canonical_ID,
                              base_name, use_alias, Ilk_ODE);
  }

  if (f_nom_str) {
    char *new_production = NULL;
    U32 np_ln = 0;
    new_production = AppendStr2HeapStr(new_production, np_ln, f_nom_str);
    delete[] f_nom_str;
    new_production = AppendStr2HeapStr(new_production, np_ln, "%arg_list%");

    char *base_str = NULL;
    char *args_str = NULL;

    args_str = LISTtoEstr(s_func, NULL, NULL, ",", NULL, error_code);
    args_str = NestInParens(args_str, false);
    if (error_code) {
      delete[] base_str;
      delete[] args_str;
      return rv;
    }

    size_t bln = strlen(new_production) + 1;
    if (base_str)
      bln += strlen(base_str);
    if (args_str)
      bln += strlen(args_str);
    rv = new char[bln];
    strcpy(rv, new_production);
    StrReplace(rv, bln, "%base%", base_str);
    StrReplace(rv, bln, "%arg_list%", args_str);

    delete[] new_production;
    delete[] base_str;
    delete[] args_str;
  }

  return rv;
}

// Engine generated solution(s) to ODE systems are sets
//  of function definitions.  The names of these functions
//  are relevent only within a single computation.
// PurgeDEFuncNames is called after each DE computation
//  at a point where aliasing info is no longer needed.

void MuPADWrapper::PurgeDEFuncNames()
{
  ClientAliases_REC *client_aliases = GetClientAliasesRec(client_ID);

  NAME2AKA_REC *head = NULL;
  NAME2AKA_REC *tail;

  NAME2AKA_REC *rover = client_aliases->AKAs_list;
  while (rover) {
    NAME2AKA_REC *curr = rover;
    rover = rover->next;

    curr->next = NULL;
    if (curr->obj_ilk == Ilk_ODE) {  // a DE function
      delete[] curr->alias;
      delete[] curr->src_canonical_name;
      delete[] curr->user_name;
      delete curr;
    } else {
      if (!head)
        head = curr;
      else
        tail->next = curr;
      tail = curr;
    }
  }

  client_aliases->AKAs_list = head;
}

// The following needs work.  We must check that independent variables
//  in DEs are un-encumbered ( ie. not defined on the engine ).
// If they are defined, the defs are deleted here.  Generally,
//  we don't want to delete data - set up a mechanism to notify
//  the user of the problem.  He can delete if that's what he wants.

void MuPADWrapper::KillIndVar(SEMANTICS_NODE * ODE_info)
{
  if (ODE_info) {
    const char *cmd_template = NULL;
    U32 cmd_ID, subID;
    if (!GetdBaseNamedRecord
        ("COMMANDS", "Undefine", &cmd_template, cmd_ID, subID)) {
      // Undefine<uID136.0>delete %s;
      return;
    }

    BUCKET_REC *bucket_rover = ODE_info->bucket_list;
    while (bucket_rover) {
      if (bucket_rover->first_child) {
//        SEMANTICS_NODE *s_eqn = bucket_rover->first_child;
        if (bucket_rover->bucket_ID == MB_ODEFUNC) {
          SEMANTICS_NODE *s_func = bucket_rover->first_child;
          if (s_func->semantic_type == SEM_TYP_FUNCTION) {
            BUCKET_REC *arg_rover = s_func->bucket_list;

            while (arg_rover) {
              SEMANTICS_NODE *s_arg = arg_rover->first_child;
              if (s_arg->semantic_type == SEM_TYP_VARIABLE) {

                ClientAliases_REC *client_aliases =
                  GetClientAliasesRec(client_ID);
                NAME2AKA_REC *map = GetAKARec(client_aliases->AKAs_list,
                                              s_arg->canonical_ID,
                                              s_arg->contents);
                if (map) {
                  char cmd[256];
                  sprintf(cmd, cmd_template, map->alias);
                  iExecute(cmd);
                }
              }
              arg_rover = arg_rover->next;
            }
          } else {
            TCI_ASSERT(0);
          }
        }
      }
      bucket_rover = bucket_rover->next;
    }
  }
}

char *MuPADWrapper::NORMtoEstr(SEMANTICS_NODE * s_norm, int& error_code)
{
  char *rv = NULL;
  error_code = 0;

  const char *key = "norm";
  const char *db_production;
  U32 ID, subID;
  if (GetdBaseNamedRecord("SCHEMATA", key, &db_production, ID, subID)) {
    if (db_production) {        // norm<uID49.0>tcinorm(%matrix%,%num%)
      char *vec_str = NULL;
      char *num_str = NULL;

      if (s_norm && s_norm->bucket_list) {
        BUCKET_REC *bucket = FindBucketRec(s_norm->bucket_list, MB_UNNAMED);
        if (bucket) {
          vec_str = ProcessSemList(bucket->first_child, NULL, error_code);
          if (error_code) {
            delete[] vec_str;
            return NULL;
          }
        }

        bucket = FindBucketRec(s_norm->bucket_list, MB_NORM_NUMBER);
        if (bucket) {
          SEMANTICS_NODE *s_num = bucket->first_child;
          if (s_num
              && s_num->contents && !strcmp(s_num->contents, "F")) {
            num_str = new char[10];
            strcpy(num_str, "Frobenius");
          } else {
            num_str = ProcessSemList(s_num, NULL, error_code);
            if (error_code) {
              delete[] num_str;
              delete[] vec_str;
              return NULL;
            }
            // MuPAD requires "Infinity" in this context
            // Need full data validation here - only positive integers
            //  or "Infinity" are allowed.

            char *ptr = strstr(num_str, "infinity");
            if (ptr)
              *ptr = 'I';
          }
        } else {                // norm not specified in input - defaults
          num_str = new char[2];
          num_str[0] = '2';
          num_str[1] = 0;
        }
      }

      size_t bln = strlen(db_production);
      if (vec_str)
        bln += strlen(vec_str);
      if (num_str)
        bln += strlen(num_str);
      rv = new char[bln];
      strcpy(rv, db_production);
      StrReplace(rv, bln, "%matrix%", vec_str);
      StrReplace(rv, bln, "%num%", num_str);

      delete[] num_str;
      delete[] vec_str;
    }
  } else
    TCI_ASSERT(0);

  return rv;
}

bool MuPADWrapper::IsDefined(const char *var_nom)
{
  bool rv = false;

  if (var_nom) {
    char cmd[256];
    sprintf(cmd, "eval(%s);", var_nom);
    char *eng_ret = Execute(cmd);
    if (eng_ret) {
      if (strcmp(eng_ret, var_nom))
        rv = true;
      delete[] eng_ret;
    }
  }
  return rv;
}

char *MuPADWrapper::QVARIABLEtoEstr(SEMANTICS_NODE * s_qvar, int& error_code)
{ 
  char *rv = NULL;
  error_code = 0;

  // SEM_TYP_QUALIFIED_VAR nodes carry a subtree, as in "x_{3+n}".
  // I regard the subtree as some kind of "qualifier".
  // If it has a meaning, it must be decided here.
  BUCKET_REC *b_bvar = FindBucketRec(s_qvar->bucket_list, MB_BASE_VARIABLE);
  BUCKET_REC *b_qual = FindBucketRec(s_qvar->bucket_list, MB_SUB_QUALIFIER);

  SEMANTICS_NODE *s_var = NULL;
  bool add_qualifier = false;

  if (b_bvar && b_bvar->first_child && b_qual && b_qual->first_child) {
    SEMANTICS_NODE *s_bvar = b_bvar->first_child;
    if (QueryEngineIsMatrix(s_bvar)) {
      s_var = s_bvar;
      add_qualifier = true;
    } else if (QualifierIsSumVar(s_qvar)) {
      s_var = s_bvar;
      add_qualifier = true;
    } else if (context_cmdtype == CC_DEFINITION
               && s_qvar->semantic_type == SEM_TYP_INDEXED_VAR) {
      s_var = s_bvar;
      add_qualifier = true;
    } else {
      s_var = s_qvar;
    }
  } else {
    TCI_ASSERT(0);
  }
  // See if we already have an engine name for this variable.
  ClientAliases_REC *client_aliases = GetClientAliasesRec(client_ID);
  NAME2AKA_REC *map = GetAKARec(client_aliases->AKAs_list,
                                s_var->canonical_ID, s_var->contents);
  if (map) {
    size_t zln = strlen(map->alias);
    char *tmp = new char[zln + 1];
    strcpy(tmp, map->alias);
    rv = tmp;
  } else {
    bool use_alias = true;
    rv = CreateEngName(client_ID, s_var->canonical_ID,
                       NULL, use_alias, Ilk_Var);
  }

  if (add_qualifier) {
    SEMANTICS_NODE *s_qualifier = b_qual->first_child;

    char *q_list = ProcessSemList(s_qualifier, NULL, error_code);
    if (!error_code) {
      const char *db_production;  // Qualified Variable<uID31.0>%var%[%q_list%]
      U32 ID, subID;
      if (GetdBaseNamedRecord
          ("SCHEMATA", "Qualified Variable", &db_production, ID, subID)) {
        size_t bln = strlen(db_production) + strlen(rv) + strlen(q_list);
        char *zh_rv = new char[bln];
        strcpy(zh_rv, db_production);
        StrReplace(zh_rv, bln, "%var%", rv);
        StrReplace(zh_rv, bln, "%q_list%", q_list);
        delete[] rv;
        rv = zh_rv;
      } else
        TCI_ASSERT(0);

      delete[] q_list;
    }
  }
  return rv;
}

bool MuPADWrapper::IsTransposeIndicator(SEMANTICS_NODE * s_power,
                                            char &which_one)
{
  bool rv = false;

  if (s_power
      && !s_power->next
      && s_power->semantic_type == SEM_TYP_VARIABLE
      && s_power->contents) {
    if (!strcmp(s_power->contents, "T")) {
      rv = true;
      which_one = 'T';
    } else if (!strcmp(s_power->contents, "H")) {
      rv = true;
      which_one = 'H';
    }
  }
  // The following code is optional.  If the potential indicator
  //   is defined, I disallow it as an indicator.

  if (rv) {
    char var[2];
    var[0] = which_one;
    var[1] = 0;
    ClientAliases_REC *client_aliases = GetClientAliasesRec(client_ID);
    NAME2AKA_REC *map = GetAKARec(client_aliases->AKAs_list, NULL, var);
    if (map && IsDefined(map->alias))
      rv = false;
  }

  return rv;
}

char *MuPADWrapper::POWERTRANSPOSEtoEstr(SEMANTICS_NODE * s_powerform,
                                         char which_one, int& error_code)
{
  char *rv = NULL;
  error_code = 0;

  const char *key = (which_one == 'T') ? "Transpose" : "Hermitian Transpose";
  const char *db_production;
  U32 ID, subID;
  if (GetdBaseNamedRecord("SCHEMATA", key, &db_production, ID, subID)) {
    if (db_production && *db_production) {
      // Transpose<uID47.0>linalg::transpose(%matrix%)
      char *m_str = NULL;

      BUCKET_REC *base_bucket =
        FindBucketRec(s_powerform->bucket_list, MB_SCRIPT_BASE);
      if (base_bucket && base_bucket->first_child) {
        SEMANTICS_NODE *s_matrix = base_bucket->first_child;
        m_str = ProcessSemList(s_matrix, NULL, error_code);
        if (error_code) {
          delete[] m_str;
          return NULL;
        }
        if (m_str) {
          size_t bln = strlen(db_production) + strlen(m_str);
          rv = new char[bln];
          strcpy(rv, db_production);
          StrReplace(rv, bln, "%matrix%", m_str);
          delete[] m_str;
        }
      } else
        TCI_ASSERT(0);
    } else
      TCI_ASSERT(0);
  } else
    TCI_ASSERT(0);

  return rv;
}

bool MuPADWrapper::DoEvals(ContextType curr_context)
{
  bool rv = true;
  if (curr_context == CC_DEFINITION
      || curr_context == CC_UNDEFINE)
    rv = false;

  return rv;
}

void MuPADWrapper::SetVarTypes(SEMANTICS_NODE * root, int & nmatrices)
{ 
  SEMANTICS_NODE *s_rover = root;
  while (s_rover) {
    bool descend = false;
    if (s_rover->semantic_type == SEM_TYP_VARIABLE) {
      int err_code;
      char *eng_nom = VARIABLEtoEstr(s_rover, err_code);
      // Is this variable already classified?
      VAR_REC *vr = FindVarRec(context_variables, eng_nom);
      if (!vr) {
        VarType v_type = VT_Normal;
        if (QueryEngineIsMatrix(s_rover)) {
          v_type = VT_Matrix;
          nmatrices++;
        }
        AppendVarRec(context_variables, eng_nom, v_type);
      }
      delete[] eng_nom;
    } else if (s_rover->semantic_type == SEM_TYP_QUALIFIED_VAR
               || s_rover->semantic_type == SEM_TYP_INDEXED_VAR) {
      char *eng_nom = NULL;

      ClientAliases_REC *client_aliases = GetClientAliasesRec(client_ID);
      NAME2AKA_REC *map = GetAKARec(client_aliases->AKAs_list,
                                    s_rover->canonical_ID,
                                    s_rover->contents);
      if (map) {
        size_t zln = strlen(map->alias);
        eng_nom = new char[zln + 1];
        strcpy(eng_nom, map->alias);
      } else {
        bool use_alias = true;
        eng_nom = CreateEngName(client_ID, s_rover->canonical_ID,
                                NULL, use_alias, Ilk_Var);
      }

      if (eng_nom) {
        // Is this variable already classified?
        VAR_REC *vr = FindVarRec(context_variables, eng_nom);
        if (!vr) {
          VarType v_type = VT_Normal;
          if (QueryEngineIsMatrix(s_rover)) {
            v_type = VT_Matrix;
            nmatrices++;
          }
          AppendVarRec(context_variables, eng_nom, v_type);
        }
        delete[] eng_nom;
      }
      descend = true;
    } else if (s_rover->semantic_type == SEM_TYP_TABULATION) {
      nmatrices++;
      descend = true;
    } else if (s_rover->bucket_list) {
      descend = true;
    }

    if (descend) {
      BUCKET_REC *b_rover = s_rover->bucket_list;
      while (b_rover) {
        if (b_rover->first_child)
          SetVarTypes(b_rover->first_child, nmatrices);
        b_rover = b_rover->next;
      }
    }

    s_rover = s_rover->next;
  }
}

char *MuPADWrapper::GetEngNameForVar(U32 frame_ID, const char *user_name,
                                     const char *canonical_ID)
{ 
  char preferred_eng_name[128];
  preferred_eng_name[0] = 0;

  bool use_alias = true;
  if (user_name) {
    const char *db_production;
    U32 ID, subID;
    if (GetdBaseNamedRecord
        ("VARIABLES", user_name, &db_production, ID, subID)) {
      if (db_production && *db_production) {
        // the database for this engine dictates an alias to be used for this variable
        size_t zln = strlen(db_production);
        if (zln < 128) {
          strcpy(preferred_eng_name, db_production);
        } else {
          TCI_ASSERT(0);
        }
      }
    } else {
      // To keep engine strings somewhat readable, we don't alias things like "x".
      //  We may add a prefix "q1x" - that's done in "CreateEngName".
      const char *pchdata = user_name;
      if (pchdata && *pchdata) {
        size_t zln = strlen(pchdata);
        if (zln < 128) {
          if (pchdata[0] != '&')
            strcpy(preferred_eng_name, pchdata);
        } else
          TCI_ASSERT(0);
        if (zln == 1) {
          if (*pchdata >= 'a' && *pchdata <= 'z')
            use_alias = false;
          if (*pchdata >= 'A' && *pchdata <= 'Z')
            use_alias = false;
        }
      } else {
        TCI_ASSERT(0);
      }
    }
  }

  return CreateEngName(frame_ID, canonical_ID, preferred_eng_name, use_alias,
                       Ilk_Var);
}

char *MuPADWrapper::VarsStrFromSNODE(SEMANTICS_NODE * s_param,
                                     int& error_code, int & tally)
{
  char *rv = NULL;
  U32 rv_ln = 0;

  tally = 0;

  // Generate a comma separated list of vars, no parens
  if (s_param) {
    SEMANTICS_NODE *s_var = s_param;
  // Descend into math container node
    while (s_var->semantic_type == SEM_TYP_MATH_CONTAINER
           || s_var->semantic_type == SEM_TYP_PRECEDENCE_GROUP) {
      if (s_var->bucket_list && s_var->bucket_list->first_child)
        s_var = s_var->bucket_list->first_child;
      else {
        TCI_ASSERT(0);
        break;
      }
    }

    if (s_var->semantic_type == SEM_TYP_VARIABLE) {
      rv = ProcessSemList(s_var, NULL, error_code);
      tally = 1;
      if (IsDefined(rv))
        error_code = CR_badindepvar;
    } else if (s_var->semantic_type == SEM_TYP_INFIX_OP) {
      BUCKET_REC *b_rover = s_var->bucket_list;
      if (b_rover->first_child) {
        SEMANTICS_NODE *s_child1 = b_rover->first_child;
        int t1 = 0;
        char *zh_child1 = VarsStrFromSNODE(s_child1, error_code, t1);

        int t2 = 0;
        char *zh_child2 = NULL;

        b_rover = b_rover->next;
        if (b_rover && b_rover->first_child) {
          SEMANTICS_NODE *s_child2 = b_rover->first_child;
          zh_child2 = VarsStrFromSNODE(s_child2, error_code, t2);
        } else
          TCI_ASSERT(0);

        if (zh_child1) {
          rv = AppendStr2HeapStr(rv, rv_ln, zh_child1);
          delete[] zh_child1;
        }
        if (zh_child2) {
          if (zh_child1)
            rv = AppendStr2HeapStr(rv, rv_ln, ",");
          rv = AppendStr2HeapStr(rv, rv_ln, zh_child2);
          delete[] zh_child2;
        }
        tally = t1 + t2;
      } else
        TCI_ASSERT(0);
    } else
      TCI_ASSERT(0);
  }

  return rv;
}

bool MuPADWrapper::QualifierIsSumVar(SEMANTICS_NODE * s_qvar)
{
  bool rv = false;

  if (s_qvar && s_qvar->bucket_list) {
    BUCKET_REC *b_qual = FindBucketRec(s_qvar->bucket_list, MB_SUB_QUALIFIER);
    SEMANTICS_NODE *s_qualifier = b_qual->first_child;
    int err_code;
    char *q_list = ProcessSemList(s_qualifier, NULL, err_code);

    if (q_list) {
      if (!strcmp(global_sum_var, q_list))
        rv = true;
      delete[] q_list;
    }
  } else
    TCI_ASSERT(0);

  return rv;
}

bool MuPADWrapper::CmdAllowed(MathServiceRequest * msr,
                                  MathResult * mr, U32 cmd_ID,
                                  SEMANTICS_NODE * child)
{
  return true;
}

// Search a semantics tree for a variable that is defined

bool MuPADWrapper::VarsAreDefined(SEMANTICS_NODE * s_tree)
{
  bool rv = false;

  ClientAliases_REC *client_aliases = GetClientAliasesRec(client_ID);

  SEMANTICS_NODE *s_rover = s_tree;
  while (s_rover && !rv) {
    if (s_rover->semantic_type == SEM_TYP_VARIABLE) {
      if (s_rover->contents) {
        NAME2AKA_REC *map =
          GetAKARec(client_aliases->AKAs_list, NULL, s_rover->contents);
        if (map)
          if (IsDefined(map->alias))
            rv = true;
      }
    } else if (s_rover->bucket_list) {
      BUCKET_REC *b_rover = s_rover->bucket_list;
      while (b_rover && !rv) {
        SEMANTICS_NODE *s_list = b_rover->first_child;
        if (s_list)
          rv = VarsAreDefined(s_list);

        b_rover = b_rover->next;
      }
    }

    s_rover = s_rover->next;
  }

  return rv;
}

// A number of command params can carry data as mml_markup
//  or as some other ASCII data - something like "10".
// Here we generate an engine sting from such a param.

char *MuPADWrapper::CreateVParameStr(MathServiceRequest & msr,
                                     U32 param_ID, U32 alt_ptype,
                                     int& error_code)
{ 
  char *zh_rv = NULL;
  error_code = 0;

  U32 p_type;
  const char *z_param = msr.GetParam(param_ID, p_type);
  if (p_type == zPT_ASCII_mmlmarkup) {
    SEMANTICS_NODE *sn = msr.GetSemanticsFromParam(param_ID);
    if (sn) {
      SEMANTICS_NODE *s_param = sn;
      zh_rv = ProcessSemList(s_param, NULL, error_code);
    } else {
      TCI_ASSERT(0);
      error_code = PID_first_badparam + param_ID;
    }
  } else if (p_type == alt_ptype) {
    size_t zln = strlen(z_param);
    zh_rv = new char[zln + 1];
    strcpy(zh_rv, z_param);
  } else {
    error_code = PID_first_badparam + param_ID;
  }
  return zh_rv;
}

