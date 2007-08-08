// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

/*
  For development purposes, these public functions are implemented
  by contructing an SWP "MapleEngine" object and making calls to it.
*/

#include "MplWrap.h"
#include "../EngWrap.h"
#include <tcistrin.h>

#include "mapl_eng.h"
#include "../../CmpTypes.h"
#include "../../MRequest.h"
#include "../../MResult.h"
#include "../../DefStore.h"
#include "Mpl2Tree.h"
#include "eAnalyzer.h"
#include "../Grammar.h"
#include "nsILocalFile.h"

enum MaplePackages
{
  MPL_NOTHING,
  MPL_ALGCURVE,                 //  algcurves 
  MPL_COMBINAT,                 //  combinat  
  MPL_COMBSTRUCT,               //  combstruct  
  MPL_DETOOLS,                  //  DEtools   
  MPL_DIFFALG,                  //  diffalg   
  MPL_DIFFORMS,                 //  difforms  
  MPL_DOMAINS,                  //  Domains   
  MPL_FINANCE,                  //  finance   
  MPL_GAUSSINT,                 //  GaussInt  
  MPL_GENFUNC,                  //  genfunc   
  MPL_GEOMETRY,                 //  geometry  
  MPL_GEOM3D,                   //  geom3d    
  MPL_GF,                       //  GF      
  MPL_GROBNER,                  //  Groebner  
  MPL_GROUP,                    //  group   
  MPL_INTTRANS,                 //  inttrans  
  MPL_LIESYMM,                  //  liesymm   
  MPL_LINALG,                   //  linalg    
  MPL_LOGIC,                    //  logic   
  MPL_LRETOOLS,                 //  LREtools  
  MPL_NETWORKS,                 //  networks  
  MPL_NUMAPPROX,                //  numapprox 
  MPL_NUMTHEORY,                //  numtheory 
  MPL_ORTHOPOLY,                //  orthopoly 
  MPL_PADIC,                    //  padic   
  MPL_PDETOOLS,                 //  PDEtools  
  MPL_PLOTS,                    //  plots   
  MPL_PLOTTOOLS,                //  plottools 
  MPL_POWSERIES,                //  powseries 
  MPL_SIMPLEX,                  //  simplex   
  MPL_STATS,                    //  stats   
  MPL_STUDENT,                  //  student   
  MPL_SUMTOOLS,                 //  sumtools  
  MPL_TENSOR,                   //  tensor    
  MPL_TOTORDER,                 //  totorder  
  MPL_LASTPACKAGE
};

MapleEngine *mapl_eng = NULL;

MapleWrapper::MapleWrapper(void *ID_dBase, void *NOM_dBase)
{
  id_dBase = (Grammar *) ID_dBase;
  nom_dBase = (Grammar *) NOM_dBase;

  mapl_eng = new MapleEngine();

  client_list = NULL;

  loaded_packages = new char[MPL_LASTPACKAGE];
  new_packages = new char[MPL_LASTPACKAGE];

  for (int i = 0; i < MPL_LASTPACKAGE; i++) {
    loaded_packages[i] = 0;
  }

  maple_parser = new Maple2Tree();
  out_tree = NULL;
  engstr_analyzer = new eAnalyzer(this);

  msg_list = NULL;
  curr_msr = NULL;

  context_nmatrices = 0;
  context_variables = NULL;

  ODE_num_procnum = 0;
  matrix_form_required = false;
}

MapleWrapper::~MapleWrapper()
{
  ClientAliases_REC *rover = client_list;
  while (rover) {
    ClientAliases_REC *del = rover;
    rover = rover->next;
    delete del;
  }

  delete mapl_eng;
  delete[] loaded_packages;
  delete[] new_packages;

  delete maple_parser;
  DisposeTList(out_tree);
  delete engstr_analyzer;
  TCI_ASSERT(!msg_list);
  DisposeMsgs(msg_list);
  if (context_variables) {
    TCI_ASSERT(0);
    DisposeVarList(context_variables);
  }
}

void MapleWrapper::iReleaseClient(U32 client_ID, const char *def_list)
{
  const char *cmd_template = NULL;
  U32 cmd_ID, subID;
  if (GetdBaseNamedRecord
      ("COMMANDS", "Undefine", &cmd_template, cmd_ID, subID)) {
    // Undefine<uID136.0>tcikill('%s');

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
        } else {
          ptr = NULL;
        }
        if (*p_name) {
          NAME2AKA_REC *curr =
            GetAKARec(client_aliases->AKAs_list, p_name, NULL);
          if (curr) {
            char cmd[256];
            sprintf(cmd, cmd_template, curr->alias);
            int res = iExecute(cmd);
          } else
            TCI_ASSERT(0);
        }
      }
      delete[] tmp;
    }

    NAME2AKA_REC *rover = client_aliases->AKAs_list;
    while (rover) {
      NAME2AKA_REC *curr = rover;
      rover = rover->next;
      if (QueryEngineIsDefined(curr->alias)) {
        char cmd[256];
        sprintf(cmd, cmd_template, curr->alias);
        int res = iExecute(cmd);
      }
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

void MapleWrapper::iCleanupAndUnloadDLL()
{
  ClientAliases_REC *rover = client_list;
  while (rover) {
    iReleaseClient(rover->client_ID, NULL);
    rover = rover->next;
  }
  mapl_eng->CleanupAndUnloadDLL();
}

SEMANTICS_NODE *MapleWrapper::iProcessRequest(MathServiceRequest * msr,
                                              MathResult * mr)
{
  SEMANTICS_NODE *s_tree = NULL;
  SEMANTICS_NODE *root = mr->GetSemanticsTree();

  // During the phase when an engine command string is being generated,
  //  we need to identify any engine packages that may be needed.

  int i = 0;
  while (i < MPL_LASTPACKAGE) {
    new_packages[i] = 0;
    i++;
  }

  curr_msr = msr;

  // We need the ID of the client we're servicing for possible name mangling
  client_ID = msr->GetClientHandle();

  U32 cmd_ID = msr->GetOpID();
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
  // The math content of engine strings depends on the command
  //  being processed.  We'll need compound dBase records
  //  or more dBase sections to support this.
  //  Evaluate an integral                      Evaluate an integral Numerically
  //Maple  eval(   tciint(q1x^(2),q1x,1,2)  );  evalf(  Int(q1x^(2),q1x=1..2)        );
  //MuPAD  eval(   tciint(q1x^(2),q1x,1,2)  );  float(  hold(int)(q1x^(2),q1x=1..2)  );

  SetWrapperCmdContext(cmd_ID, new_packages);
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

    if (children) {
      // Traverse the math object (a semantic tree), 
      //  building an engine command string as we go.

      if (cmd_ID == CCID_Polynomial_Divide) { // Polynomial Divide
        if (children->semantic_type == SEM_TYP_FRACTION
            && !children->next) {
          eng_zstr =
            FRACTIONtoEstr(children, "Polynomial Divide", error_code);
        } else
          TCI_ASSERT(0);
      } else
        eng_zstr = ProcessSemList(children, error_code);
    }

    char *zEngine_input = NULL;
    if (!error_code)
      zEngine_input =
        CreateEngineCommandStr(*msr, cmd_ID, eng_zstr, error_code);

    // There will be cases where the previous call fails.
    // Eventually, these cases will be considered here.

    if (zEngine_input && !error_code) {
      // Send our command string to the underlying engine
      if (context_cmdtype == CC_DEFINITION) {
        // might send tcikill() here
      }

      if (cmd_ID >= CCID_Solve_PDE && cmd_ID <= CCID_Solve_ODE_Series) {  // Solve DEs
        // Make sure that the independent variable is NOT defined in the engine
        KillIndVar(children);
      }

      LoadPackages(new_packages);
      char *zeng_output = Execute(zEngine_input);

      if (zeng_output) {
        size_t numBytes = strlen(zeng_output);

        if (cmd_ID == CCID_Define || cmd_ID == CCID_Undefine ) {
          // Don't process the return here
          mr->PutResultCode(CR_success);

        } else {
          MNODE *eng_tree = maple_parser->MapleStr2Tree(zeng_output);

          if (eng_tree) {
            // Build a "semantics" tree from eng_tree
            ClientAliases_REC *name_map = GetClientAliasesRec(client_ID);
            s_tree = engstr_analyzer->BuildSemanticsTree(eng_tree, cmd_ID,
                                                         name_map);
            if (cmd_ID >= CCID_Solve_PDE && cmd_ID <= CCID_Solve_ODE_Series)
              PurgeDEFuncNames();

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
          } else {
          }
        }
        delete[] zeng_output;
      } else {                  // there is no engine return string
        if (cmd_ID == CCID_Undefine // Undefine
            || cmd_ID == CCID_PassThru) { // Pass Thru
          mr->PutResultCode(CR_success);
        } else {
          TCI_ASSERT(0);
          mr->PutResultCode(CR_failure);
        }
      }
      delete[] zEngine_input;
    } else if (error_code) {
      mr->PutResultCode(error_code);
    }
    delete[] eng_zstr;
  } else {                      // if ( CmdGoesToEngine(cmd_ID) )
  }

  TCI_ASSERT(!msg_list);
  mr->AttachMsgs(msg_list);
  msg_list = NULL;

  if (context_variables) {
    DisposeVarList(context_variables);
    context_variables = NULL;
  }

  curr_msr = NULL;
  return s_tree;
}

bool MapleWrapper::iLoadStrsAndDLL(nsILocalFile *engp, nsILocalFile *libp)
{
  bool rv = false;

//  TCIString engpath = engp;
//  TCIString libpath = libp;
  return mapl_eng->LoadStrsAndDLL(engp, libp);
}

bool MapleWrapper::iInitialize(nsILocalFile *libp, nsILocalFile *champ)
{
  bool rv = false;

//  TCIString libpath = libp;
//  TCIString champath = champ;
  return mapl_eng->Initialize(libp, champ);
}

bool MapleWrapper::SetEngineState(int attr_ID, const char *new_value)
{
  bool rv = false;

  const char *eng_dbase_rec;
  char p_name[80];
  if (GetdBaseIDRecord("RESET", attr_ID, 0, &eng_dbase_rec, p_name, 80)) {
    if (eng_dbase_rec && *eng_dbase_rec) {
      char cmd[256];
      sprintf(cmd, eng_dbase_rec, new_value);
      ByteArray ba(256);
      ba += cmd;
      rv = mapl_eng->SendToProcessor(&ba, 0);
    }
  }

  return rv;
}

// The interface version, iExecute, returns an int.
// The caller is expected to retrieve the engine return string
//  using  __cdecl iRetrieveStr( 2,char* buffer,int bln );
//  which uses
//  from Maple::int RetrieveStr( int str_ID,char* buffer,int bln );

int MapleWrapper::iExecute(const char *cmd)
{
  ByteArray ba(80);
  ba += cmd;
  bool res = mapl_eng->SendToProcessor(&ba, 0);

  U32 nBytes = ba.GetByteCount();
  if (res && nBytes)
    return 1;
  else
    return 0;
}

char *MapleWrapper::Execute(const char *cmd)
{
  ByteArray ba(80);
  ba += cmd;
  bool res = mapl_eng->SendToProcessor(&ba, 0);

  U32 nBytes = ba.GetByteCount();
  if (res && nBytes) {
    const char *ret = (const char *)ba.ReadOnlyLock();
    char *rv = new char[nBytes + 1];
    strncpy(rv, ret, nBytes);
    rv[nBytes] = 0;
    ba.ReadOnlyUnlock();
    return rv;

  } else
    return NULL;
}

// There are a number of different formats for commands to Maple.
// The basic lay out is  "cmd_name(math_expression);"
// Commands that are parameterized have more complex layouts.

char *MapleWrapper::CreateEngineCommandStr(MathServiceRequest & msr,
                                           U32 cmd_ID, const char *eng_expr,
                                           int & error_code)
{
  char *rv = NULL;
  error_code = 0;

  // Get the dBase record for the command to be processed

  char op_name[80];             // Evaluate
  const char *cmd_template = NULL;  // %eval%(%expr%);
  U32 data_type = context_datatype;

  bool got_template = false;
  if (cmd_ID) {
    if (data_type > 0) {
      if (GetdBaseIDRecord("COMMANDS", cmd_ID, data_type, &cmd_template, op_name, 80))
        got_template = true;
    }
    if (!got_template)
      if (GetdBaseIDRecord("COMMANDS", cmd_ID, 0, &cmd_template, op_name, 80))
        got_template = true;
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
        || context_cmdtemplatetype == CT_CHARACTERISTICP) {
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
    rv = CreateRANDOMNUMeStr(msr, error_code);
    break;

  case CT_RESHAPE:
    rv = CreateRESHAPEeStr(eng_expr, cmd_template, error_code);
    break;

  case CT_MAPENTRIES:
    rv = CreateMAPSeStr(msr, eng_expr, cmd_template, error_code);
    break;

  case CT_SOLVEEXACT:
  case CT_POLYSORT:            // 
  case CT_EXPRandVAR:          // 
  case CT_POLYROOTS:           // 
  case CT_EXTREMA:             // 
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

  case CT_PSERIES:             // Power series...<uID43.0>series(%expr%,%var%=0,%order%);
    rv = CreatePSERIESeStr(msr, eng_expr, cmd_template, error_code);
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

  default:
    TCI_ASSERT(0);
    break;
  }

  return rv;
}

// Iterate thru a list of semantic math objects.
// Get the engine specific translation of each object
//  and catenate it to the returned string.

char *MapleWrapper::ProcessSemList(SEMANTICS_NODE * head, int& error_code)
{
  char *rv = NULL;
  error_code = 0;

  SEMANTICS_NODE *rover = head;
  while (rover) {
    SEMANTICS_NODE *next_node = rover->next;
    char *eng_zstr = ProcessSemNode(rover, error_code);
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

char *MapleWrapper::ProcessSemNode(SEMANTICS_NODE * s_node, int & error_code)
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
      TCI_ASSERT(0);
      break;

    case SEM_TYP_PIECEWISE_LIST:
      rv = Piecewise2Estr(s_node, error_code);
      break;

    case SEM_TYP_PDE:
      rv = ODEtoEstr(s_node, error_code);
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
        bool is_infix;
        rv = OPERATORtoEstr(s_node, is_infix, error_code);
        if (rv && is_infix && s_node->bucket_list) {
          rv = NestInParens(rv, false);
        } else if (s_node->semantic_type == SEM_TYP_PREFIX_OP
                   && s_node->contents) {
          char ch = s_node->contents[0];
          if (ch == '+' || !strcmp(s_node->contents, "&#x2212;"))
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
        if (bucket_info->first_child)
          rv = ProcessSemList(bucket_info->first_child, error_code);
        else
          TCI_ASSERT(0);
      }
      break;

    case SEM_TYP_PRECEDENCE_GROUP:
      if (s_node->bucket_list) {
        BUCKET_REC *bucket_info = s_node->bucket_list;
        if (bucket_info->first_child)
          rv = ProcessSemList(bucket_info->first_child, error_code);
        else
          TCI_ASSERT(0);
      }
      break;

    case SEM_TYP_PARENED_LIST:
    case SEM_TYP_BRACKETED_LIST:
    case SEM_TYP_GENERIC_FENCE:
      rv = LISTtoEstr(s_node, "(", ")", ",", error_code);
      break;

    case SEM_TYP_LIST:
      rv = LISTtoEstr(s_node, NULL, NULL, ",", error_code);
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
      rv = POWERFORMtoEstr(s_node, false, error_code);
      break;

    case SEM_TYP_SQRT:
      rv = SQROOTtoEstr(s_node, error_code);
      break;

    case SEM_TYP_ROOT:
      rv = ROOTtoEstr(s_node, error_code);
      break;

    case SEM_TYP_TABULATION:
      rv = TABULATIONtoEstr(s_node, error_code);
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

// Return the engine string used to identify this variable.
// Generally, it consists of a client-specific qualifier, "q1",
//  followed by the variable name as found in the source, "x",
//  or some alias for the source name.

char *MapleWrapper::VARIABLEtoEstr(SEMANTICS_NODE * snode, int& error_code)
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
  NAME2AKA_REC *map = GetAKARec(client_aliases->AKAs_list,
                                snode->canonical_ID, snode->contents);
  if (map) {
    size_t zln = strlen(map->alias);
    char *tmp = new char[zln + 1];
    strcpy(tmp, map->alias);
    rv = tmp;

  } else {
    rv = GetEngNameForVar(frame_ID, snode->contents,
                          snode->canonical_ID);
  }

  if (snode->bucket_list && snode->bucket_list->first_child) {
    SEMANTICS_NODE *s_qualifier = snode->bucket_list->first_child;
    // Qualified variables are now typed as SEM_TYP_QUALIFIED_VAR or SEM_TYP_INDEXED_VAR
    TCI_ASSERT(0);

  }

  if (rv && DoEvals(context_cmdtype)
      && QueryEngineIsDefined(rv)) {
    size_t zln = strlen(rv);
    char *tmp = new char[zln + 8];
    strcpy(tmp, "eval(");
    strcat(tmp, rv);
    strcat(tmp, ")");
    delete[] rv;
    rv = tmp;
  }

  return rv;
}

// The engine wrapper will be required to convert the canonical
//  string to the format required by the underlying engine.
// In general, an engine wrapper may look for a few common
//  number formats.  Everything else will have to be rejected
//  as an error or aliased in the engine command string.

char *MapleWrapper::NUMBERtoEstr(SEMANTICS_NODE * snode, int& error_code)
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

char *MapleWrapper::MIXEDNUMBERtoEstr(SEMANTICS_NODE * s_mixed,
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
      whole_str = ProcessSemList(bucket->first_child, error_code);

      if (error_code) {
        delete[] whole_str;
        return NULL;
      }
    }

    bucket = FindBucketRec(s_mixed->bucket_list, MB_MN_FRACTION);
    if (bucket && bucket->first_child) {
      frac_str = ProcessSemList(bucket->first_child, error_code);

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
  } else {
    TCI_ASSERT(0);
  }

  return zh_rv;
}

// Pi, etc.

char *MapleWrapper::UCONSTANTtoEstr(SEMANTICS_NODE * snode, int& error_code)
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
      rv =
        CreateEngName(client_ID, snode->canonical_ID, base_name,
                      use_alias, Ilk_Var);
    }
  } else {
    TCI_ASSERT(0);
  }
  return rv;
}

char *MapleWrapper::TEXTtoEstr(SEMANTICS_NODE * snode, int& error_code)
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

char *MapleWrapper::UNITtoEstr(SEMANTICS_NODE * snode, int& error_code)
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
    } else {
      TCI_ASSERT(0);
    }
  } else {
    TCI_ASSERT(0);
  }
  return rv;
}

// The core side does a lot to support function handling.
// ie the "s_func" contains all the data needed to script
//  the engine function call in a direct manner.

char *MapleWrapper::FUNCTIONtoEstr(SEMANTICS_NODE * s_func, int& error_code)
{
  char *rv = NULL;
  error_code = 0;

  char *new_production = NULL;
  U32 np_ln = 0;

  bool user_func = false;
  bool pass_thru = false;
  bool in_defstore = false;

  if (s_func->msi_class) {
    pass_thru = true;

  } else if (s_func->canonical_ID) {
    DefStore *defstore = curr_msr->GetDefStore();
    U32 engine_ID = curr_msr->GetEngineID();
    DefInfo *di = defstore->GetDefInfo(engine_ID, s_func->canonical_ID);
    if (di && di->arg_list)
      in_defstore = true;
  }

  if (pass_thru) {

  } else if (in_defstore) {
    user_func = true;

  } else {

    char *key = s_func->contents;
    const char *db_production = NULL;
    U32 ID, subID;

    const char *db_section = "FUNCTIONS";
    if (s_func->semantic_type == SEM_TYP_INVFUNCTION)
      db_section = "INVERSES";
    if (s_func->semantic_type == SEM_TYP_LISTOPERATOR)
      db_section = "LISTOPERATORS";
    else if (s_func->semantic_type == SEM_TYP_FUNCTION
             || s_func->semantic_type == SEM_TYP_PREFIX_OP) {
      if (!s_func->bucket_list)
        if (GetdBaseNamedRecord("MAPLETOKS", key, &db_production, ID, subID))
          db_section = "MAPLETOKS";
      // Real, Complex, Integer, Negative, Positive, NonZero
    }

    if (GetdBaseNamedRecord(db_section, key, &db_production, ID, subID)) {
      // Probability functions
      if (ID >= 8089 && ID <= 8127)
        return PROBFUNCtoEstr(s_func, db_production, ID, error_code);

      // Here, the user has entered a function that is known to the engine.
      // ie. a function that need not be defined by the user.
      // The production in the engine database is a template for
      //  a call to the function - it includes the function name.

      new_production =
        AppendStr2HeapStr(new_production, np_ln, db_production);
    } else {
      // Not in maple.gmr AND not in the DefStore
      // An error? maybe a passthru?
      user_func = true;
    }
  }

  if (user_func || pass_thru) {
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
    if (context_cmdtype != CC_UNDEFINE)
      new_production = AppendStr2HeapStr(new_production, np_ln, "%arg_list%");
  }

  char *base_str = NULL;
  char *args_str = NULL;

  // logs are a special case - carry a log base
  BUCKET_REC *lb_bucket = FindBucketRec(s_func->bucket_list, MB_LOG_BASE);
  if (lb_bucket && lb_bucket->first_child) {
    SEMANTICS_NODE *s_log_base = lb_bucket->first_child;
    if (s_log_base->semantic_type != SEM_TYP_UCONSTANT) {
      const char *db_production = NULL;
      U32 ID, subID;
      if (GetdBaseNamedRecord
          ("FUNCTIONS", "logwithbase", &db_production, ID, subID)) {
        // logwithbase<uID8023.1>log[%base%]%arg_list%
        delete[] new_production;
        new_production = NULL;
        np_ln = 0;
        new_production =
          AppendStr2HeapStr(new_production, np_ln, db_production);

        SEMANTICS_NODE *curr_handle = lb_bucket->first_child;
        base_str = ProcessSemList(curr_handle, error_code);
        if (error_code) {
          delete[] base_str;
          return rv;
        }
      } else {
        TCI_ASSERT(0);
      }
    }
  }

  char *p2 = strstr(new_production, "%arg_list%");
  if (p2) {
    args_str = LISTtoEstr(s_func, NULL, NULL, ",", error_code);
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
      char *obj_str = ProcessSemList(curr_handle, err_code);
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
    // superscript<uID1.0>(%base%)^(%superscript%)
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

char *MapleWrapper::SETtoEstr(SEMANTICS_NODE * set_info, int& error_code)
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
        char *obj_str = ProcessSemList(curr_handle, err_code);
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

// eval( limit((1/q2x),q2x=1{,right}) );

char *MapleWrapper::LIMFUNCtoEstr(SEMANTICS_NODE * limfunc_info,
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
    //max<uID.1.2>tcimax(seq(%arg_list%,%var%=%ll%..%ul%))

    char *arg_str = NULL;
    char *var_str = NULL;
    char *lim_exp_str = NULL;
    char *direction_str = NULL;
    char *ll_str = NULL;
    char *ul_str = NULL;

    const char *ptr = strstr(db_production, "%arg_list%");
    if (ptr) {
      // function takes an argument
      BUCKET_REC *arg_bucket =
        FindBucketRec(limfunc_info->bucket_list, MB_UNNAMED);
      if (arg_bucket->first_child) {
        arg_str = ProcessSemList(arg_bucket->first_child, error_code);
      }

      if (error_code)
        return NULL;
    }

    const char *p_lim_exp = NULL;
    const char *p_ll = NULL;
    const char *p_ul = NULL;
    BUCKET_REC *l_bucket =
      FindBucketRec(limfunc_info->bucket_list, MB_LOWERLIMIT);
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
          var_str = ProcessSemNode(var_snode, error_code);
          if (error_code) {
            delete[] var_str;
            delete[] arg_str;
            return NULL;
          }
        }

        if (p_ll && p_ul) {
          if (ill_bucket) {
            SEMANTICS_NODE *ll_snode = ill_bucket->first_child;
            ll_str = ProcessSemNode(ll_snode, error_code);
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
            ul_str = ProcessSemNode(ul_snode, error_code);
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
      } else {
        BUCKET_REC *expr_bucket = FindBucketRec(l_bucket->parts, MB_LL_EXPR);
        if (p_lim_exp) {
          int direction = 0;    // non-directional limit
          BUCKET_REC *dir_bucket =
            FindBucketRec(l_bucket->parts, MB_LL_DIRECTION);
          if (dir_bucket) {
            SEMANTICS_NODE *snum = dir_bucket->first_child;
            if (snum && snum->contents &&
                snum->semantic_type == SEM_TYP_NUMBER)
              direction = atoi(snum->contents);
          }
          SEMANTICS_NODE *lim_exp_snode = expr_bucket->first_child;
          if (lim_exp_snode) {
            lim_exp_str = ProcessSemNode(lim_exp_snode, error_code);
            if (error_code) {
              delete[] lim_exp_str;
              delete[] var_str;
              delete[] arg_str;
              return NULL;
            }
            if (direction == 1)
              direction_str = ",right";
            else if (direction == 2)
              direction_str = ",left";
          }

          BUCKET_REC *var_bucket = FindBucketRec(l_bucket->parts, MB_LL_VAR);
          if (var_bucket) {
            SEMANTICS_NODE *var_snode = var_bucket->first_child;
            var_str = ProcessSemNode(var_snode, error_code);
            if (error_code) {
              delete[] lim_exp_str;
              delete[] var_str;
              delete[] arg_str;
              return NULL;
            }
          } else {
            TCI_ASSERT(0);
          }
        }
      }
    } else {
      TCI_ASSERT(0);
    }
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
  } else {
    TCI_ASSERT(0);
  }
  return rv;
}

char *MapleWrapper::SUBSTITUTIONtoEstr(SEMANTICS_NODE * subst_info,
                                       int& error_code)
{
  char *rv = NULL;
  error_code = 0;

  char *key = subst_info->contents; // "substitute";
  const char *db_production;
  U32 ID, subID;
  if (GetdBaseNamedRecord("SCHEMATA", key, &db_production, ID, subID)) {
    TCI_ASSERT(db_production);
    //substitution<uID17.0>(subs(%subst_list%,%expr%))
    //doublesubstitution<uID18.0>(subs(%usubst_list%,%expr%)-subs(%subst_list%,%expr%))

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
      expr_str = ProcessSemList(expr_bucket->first_child, error_code);
      if (error_code) {
        delete[] expr_str;
        return NULL;
      }
    } else {
      TCI_ASSERT(0);
    }
    if (lower_bucket && lower_bucket->first_child) {
      lower_str =
        Subst2Estr(subst_info, true, expr_bucket, upper_bucket, error_code);
      if (error_code) {
        delete[] expr_str;
        return NULL;
      }
    } else {
      TCI_ASSERT(0);
    }
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
  } else {
    TCI_ASSERT(0);
  }
  return rv;
}

char *MapleWrapper::MULTIARGFUNCtoEstr(SEMANTICS_NODE * subst_info,
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

    BUCKET_REC *subst_bucket =
      FindBucketRec(subst_info->bucket_list, MB_SUBST_LOWER);
    if (subst_bucket && subst_bucket->first_child) {
      subst_str = ProcessSemList(subst_bucket->first_child, error_code);

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
      expr_str = ProcessSemList(expr_bucket->first_child, error_code);

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

  } else
    TCI_ASSERT(0);

  return rv;
}

char *MapleWrapper::DERIVATIVEtoEstr(SEMANTICS_NODE * diff_info,
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
          } else
            TCI_ASSERT(0);
          s_rover = s_rover->next;
        }
      }
      b_rover = b_rover->next;
    }

    if (vs_ln == 0) {
      BUCKET_REC *func_bucket =
        FindBucketRec(diff_info->bucket_list, MB_UNNAMED);
      if (func_bucket && func_bucket->first_child) {
        SEMANTICS_NODE *s_func = func_bucket->first_child;

        BUCKET_REC *var_bucket =
          FindBucketRec(s_func->bucket_list, MB_UNNAMED);
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
      } else
        TCI_ASSERT(0);
    }
    // the variable of differentiation looks like
    //  "q1x", "q1x$2", "q1x,q1x", "q1x,q1y$2", etc.

    BUCKET_REC *nprimes_bucket =
      FindBucketRec(diff_info->bucket_list, MB_NPRIMES);
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
      expr_str = ProcessSemList(s_expr, error_code);
      if (error_code) {
        return NULL;
      }
    } else
      TCI_ASSERT(0);

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

  } else {
    TCI_ASSERT(0);
  }
  return rv;
}

char *MapleWrapper::ODEBoundarytoEstr(SEMANTICS_NODE * diff_info,
                                      int& error_code)
{
  char *rv = NULL;

  // The expression, function, etc., that is differentiated
  //  is held in a "MB_UNNAMED" bucket in SYM_TYP_DERIVATIVE's
  //  bucket_list

  char *func_str = NULL;
  char *args_str = NULL;
  BUCKET_REC *expr_bucket = FindBucketRec(diff_info->bucket_list, MB_UNNAMED);
  if (expr_bucket->first_child) {
    SEMANTICS_NODE *s_func = expr_bucket->first_child;
    func_str = GetEngNameForUserFunc(s_func);

    int err_code = 0;
    args_str = LISTtoEstr(s_func, NULL, NULL, ",", err_code);
    args_str = NestInParens(args_str, false);
    if (err_code) {
      error_code = err_code;
      return rv;
    }
  } else
    TCI_ASSERT(0);

  int n_primes = 1;
  BUCKET_REC *nprimes_bucket = FindBucketRec(diff_info->bucket_list,
                                             MB_NPRIMES);
  if (nprimes_bucket) {
    SEMANTICS_NODE *s_number = nprimes_bucket->first_child;
    n_primes = atoi(s_number->contents);
  } else
    TCI_ASSERT(0);

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

char *MapleWrapper::PRIMEStoEstr(SEMANTICS_NODE * diff_info, int& error_code)
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
      func_str = ProcessSemList(s_expr, error_code);
      if (error_code) {
        return NULL;
      }
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
          } else
            TCI_ASSERT(0);

          U32 frame_ID = diff_info->owner_ID;
          if (!frame_ID)        // give ownership to the client requesting
            frame_ID = client_ID; //  the current computation

          ClientAliases_REC *client_aliases = GetClientAliasesRec(frame_ID);
          NAME2AKA_REC *map =
            GetAKARec(client_aliases->AKAs_list, canonical_ID, NULL);
          if (map) {
            size_t zln = strlen(map->alias);
            var_str = new char[zln + 1];
            strcpy(var_str, map->alias);
          } else {
            var_str = GetEngNameForVar(frame_ID, user_name, canonical_ID);
          }
        } else {                // fake it
        }
      } else
        TCI_ASSERT(0);
    }

    BUCKET_REC *nprimes_bucket =
      FindBucketRec(diff_info->bucket_list, MB_NPRIMES);
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
  } else                        // if ( db_production )
    TCI_ASSERT(0);

  return rv;
}

char *MapleWrapper::BIGOPtoEstr(SEMANTICS_NODE * bigop_snode,
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
      //&#x2211;<uID4.0>tcisum('%operand%','%variable%',%lower%,%upper%)
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
          lower_str = ProcessSemNode(lim_exp_snode, error_code);
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
          upper_str = ProcessSemNode(lim_exp_snode, error_code);
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
          var_str = ProcessSemNode(lim_exp_snode, error_code);
          if (error_code) {
            delete[] upper_str;
            delete[] lower_str;
            delete[] var_str;
            return NULL;
          }
          if (var_str)
            strcpy(global_sum_var, var_str);
        }
      } else
        TCI_ASSERT(0);

      const char *ptr = strstr(db_production, "%operand%");
      if (ptr) {
        // big prefix operator takes an operand
        BUCKET_REC *operand_bucket =
          FindBucketRec(bigop_snode->bucket_list, MB_OPERAND);
        if (operand_bucket->first_child) {
          SEMANTICS_NODE *operand_snode = operand_bucket->first_child;

          operand_str = ProcessSemList(operand_snode, error_code);
        }
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
  } else                        // lookup failed
    TCI_ASSERT(0);

  return rv;
}

char *MapleWrapper::INTEGRALtoEstr(SEMANTICS_NODE * bigop_snode,
                                   int& error_code)
{
  char *rv = NULL;
  error_code = 0;

  //eval  integral<uID5.1>tciint(%operand%,%variable%  ,%lower%,%upper%)
  //evalf integral<uID5.2>Int(%operand%,%variable%   =%lower%..%upper%)
  //indefinite integral<uID5.3>tciint(%operand%%comma%%variable%)
  //indefinite integral<uID5.4>Int(%operand%%comma%%variable%)
  //approximate integral<uID5.5>%operand%,%variable%,%lower%..%upper%
  char *lower_str = NULL;
  char *upper_str = NULL;
  char *operand_str = NULL;
  char *var_str = NULL;

  BUCKET_REC *l_bucket =
    FindBucketRec(bigop_snode->bucket_list, MB_LOWERLIMIT);
  if (l_bucket && l_bucket->first_child) {
    lower_str = ProcessSemList(l_bucket->first_child, error_code);
    if (error_code) {
      delete[] lower_str;
      return NULL;
    }
  }

  BUCKET_REC *u_bucket =
    FindBucketRec(bigop_snode->bucket_list, MB_UPPERLIMIT);
  if (u_bucket && u_bucket->first_child) {
    upper_str = ProcessSemList(u_bucket->first_child, error_code);
    if (error_code) {
      delete[] upper_str;
      delete[] lower_str;
      return NULL;
    }
  }

  BUCKET_REC *operand_bucket =
    FindBucketRec(bigop_snode->bucket_list, MB_OPERAND);
  if (operand_bucket && operand_bucket->first_child) {
    operand_str = ProcessSemList(operand_bucket->first_child, error_code);
    if (error_code) {
      delete[] upper_str;
      delete[] lower_str;
      delete[] operand_str;
      return NULL;
    }
  }

  BUCKET_REC *v_bucket =
    FindBucketRec(bigop_snode->bucket_list, MB_INTEG_VAR);
  if (v_bucket && v_bucket->first_child) {
    var_str = ProcessSemList(v_bucket->first_child, error_code);
    if (error_code) {
      delete[] var_str;
      delete[] upper_str;
      delete[] lower_str;
      delete[] operand_str;
      return NULL;
    }
  }

  U32 ID = 5;
  U32 subID = 1;
  if (!l_bucket && !u_bucket) { // indefinite integral
    if (context_cmdtype == CC_HOLDINTEGRAL)
      subID = 4;
    else
      subID = 3;
  } else {
    if (context_cmdtype == CC_APPROXINTEGRAL)
      subID = 5;
    else if (context_cmdtype == CC_HOLDINTEGRAL)
      subID = 2;
    else
      subID = 1;
  }

  const char *db_production;
  char name_buffer[80];
  if (GetdBaseIDRecord("BIGOPS", ID, subID, &db_production, name_buffer, 80)) {
    TCI_ASSERT(db_production);

    size_t bln = strlen(db_production);
    if (operand_str)
      bln += strlen(operand_str);
    if (lower_str)
      bln += strlen(lower_str);
    if (upper_str)
      bln += strlen(upper_str);
    if (var_str)
      bln += strlen(var_str);
    rv = new char[bln + 1];
    strcpy(rv, db_production);
    StrReplace(rv, bln, "%operand%", operand_str);
    if (subID == 3 || subID == 4) {
      char *comma = var_str ? "," : "";
      StrReplace(rv, bln, "%comma%", comma);
    } else {
      StrReplace(rv, bln, "%lower%", lower_str);
      StrReplace(rv, bln, "%upper%", upper_str);
    }
    StrReplace(rv, bln, "%variable%", var_str);
  }

  delete[] operand_str;
  delete[] lower_str;
  delete[] upper_str;
  delete[] var_str;

  // We may be processing a multiple integral
  int n_integs = SemanticVariant2NumIntegrals(bigop_snode->variant);
  if (n_integs > 1 && v_bucket) {
    while (n_integs > 1) {
      v_bucket = FindBucketRec(v_bucket->next, MB_INTEG_VAR);
      //indefinite integral<uID5.3>tciint(%operand%%comma%%variable%)

      if (v_bucket && v_bucket->first_child) {
        char *var_str = ProcessSemList(v_bucket->first_child, error_code);
        if (error_code) {
          delete[] rv;
          delete[] var_str;
          return NULL;
        }

        size_t bln = strlen(db_production);
        if (rv)
          bln += strlen(rv);
        if (var_str)
          bln += strlen(var_str);
        char *new_rv = new char[bln + 1];
        strcpy(new_rv, db_production);
        StrReplace(new_rv, bln, "%operand%", rv);
        StrReplace(new_rv, bln, "%comma%", ",");
        StrReplace(new_rv, bln, "%variable%", var_str);

        delete[] rv;
        delete[] var_str;
        rv = new_rv;

        n_integs--;
      } else {
        break;
      }
    }
  }

  return rv;
}

char *MapleWrapper::OPERATORtoEstr(SEMANTICS_NODE * sem_op_node,
                                   bool & is_infix, int& error_code)
{
  char *rv = NULL;

  if (sem_op_node->bucket_list) {
    // Reverse Polish forms carry operands as children
    rv = POLISHOPtoEstr(sem_op_node, is_infix, error_code);
  } else {
    // We still use infix in a few cases

    bool op_found = false;

    char *key = sem_op_node->contents;
    const char *db_maple_op;
    U32 ID, subID;

    if (GetdBaseNamedRecord("INFIXOPS", key, &db_maple_op, ID, subID)) {
      op_found = true;
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
        if (ptr) {
          rv = PROBFUNCtoEstr(sem_op_node, db_maple_op, ID, error_code);
        } else
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

char *MapleWrapper::INTERVALtoEstr(SEMANTICS_NODE * interval_info,
                                   int& error_code)
{
  char *zh_rv = NULL;
  error_code = 0;

  char *key = interval_info->contents;
  const char *db_production;
  U32 ID, subID;
  GetdBaseNamedRecord("SCHEMATA", key, &db_production, ID, subID);
  if (db_production) {          // interval<uID33.0>%lower%..%upper%
    char *lower_str = NULL;
    char *upper_str = NULL;

    BUCKET_REC *l_b =
      FindBucketRec(interval_info->bucket_list, MB_INTERVAL_START);
    if (l_b && l_b->first_child) {
      SEMANTICS_NODE *curr_handle = l_b->first_child;
      lower_str = ProcessSemList(curr_handle, error_code);
      if (error_code) {
        delete[] lower_str;
        return NULL;
      }
    }
    BUCKET_REC *u_b =
      FindBucketRec(interval_info->bucket_list, MB_INTERVAL_END);
    if (u_b && u_b->first_child) {
      SEMANTICS_NODE *curr_handle = u_b->first_child;
      upper_str = ProcessSemList(curr_handle, error_code);
      if (error_code) {
        delete[] lower_str;
        delete[] upper_str;
        return NULL;
      }
    }

    if (lower_str && upper_str) {
      size_t bln =
        strlen(db_production) + strlen(lower_str) + strlen(upper_str);
      zh_rv = new char[bln];
      strcpy(zh_rv, db_production);
      StrReplace(zh_rv, bln, "%lower%", lower_str);
      StrReplace(zh_rv, bln, "%upper%", upper_str);
    }
    delete[] lower_str;
    delete[] upper_str;
  }

  return zh_rv;
}

char *MapleWrapper::FRACTIONtoEstr(SEMANTICS_NODE * frac_info,
                                   const char *key, int& error_code)
{
  char *rv = NULL;
  error_code = 0;

  const char *db_production;
  U32 ID;
  U32 subID;
  GetdBaseNamedRecord("SCHEMATA", key, &db_production, ID, subID);

  if (db_production) {          // fraction<uID2.0>(%numerator%/%denominator%)
    char *num_str = NULL;
    char *den_str = NULL;

    BUCKET_REC *bucket = FindBucketRec(frac_info->bucket_list, MB_NUMERATOR);
    if (bucket && bucket->first_child) {
      num_str = ProcessSemList(bucket->first_child, error_code);

      if (error_code) {
        delete[] num_str;
        return NULL;
      }
      num_str = NestInParens(num_str, false);
    }

    bucket = FindBucketRec(frac_info->bucket_list, MB_DENOMINATOR);
    if (bucket && bucket->first_child) {
      den_str = ProcessSemList(bucket->first_child, error_code);
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

char *MapleWrapper::POWERFORMtoEstr(SEMANTICS_NODE * s_powerform,
                                    bool no_surds, int& error_code)
{
  char *rv = NULL;
  error_code = 0;

  // look for case where "exponential e" is the base
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
            arg_str = ProcessSemList(script_bucket->first_child, error_code);
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
      else                      // if ( s_base->semantic_type == SEM_TYP_VARIABLE )
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
    char *key = base_is_matrix ? "tcipwr" : "superscript";
    const char *db_production;
    U32 ID, subID;
    if (GetdBaseNamedRecord("SCHEMATA", key, &db_production, ID, subID)) {

      TCI_ASSERT(db_production);  // superscript<uID1.0>%base%^(%superscript%)
      char *base_str = NULL;
      char *sup_str = NULL;

      BUCKET_REC *bucket =
        FindBucketRec(s_powerform->bucket_list, MB_SCRIPT_BASE);
      if (bucket && bucket->first_child) {
        base_str = ProcessSemList(bucket->first_child, error_code);

        if (error_code) {
          delete[] base_str;
          return NULL;
        }
      }

      bucket = FindBucketRec(s_powerform->bucket_list, MB_SCRIPT_UPPER);
      if (bucket && bucket->first_child) {
        sup_str = ProcessSemList(bucket->first_child, error_code);

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

char *MapleWrapper::ROOTtoEstr(SEMANTICS_NODE * root_info, int& error_code)
{
  char *zh_rv = NULL;
  error_code = 0;

  bool use_power_form = false;
  if (use_power_form) {
    zh_rv = SURDasPOWERtoEstr(root_info, error_code);
  } else {
    char *key = "surd";
    const char *db_production;
    U32 ID, subID;
    if (GetdBaseNamedRecord("SCHEMATA", key, &db_production, ID, subID)) {
      TCI_ASSERT(db_production);  // surd<uID3.0>(%base%,%radicand%)
      char *base_str = NULL;
      char *radi_str = NULL;

      BUCKET_REC *bucket =
        FindBucketRec(root_info->bucket_list, MB_ROOT_BASE);
      if (bucket && bucket->first_child) {
        base_str = ProcessSemList(bucket->first_child, error_code);
        if (error_code) {
          delete[] base_str;
          return NULL;
        }
      }

      bucket = FindBucketRec(root_info->bucket_list, MB_ROOT_EXPONENT);
      if (bucket && bucket->first_child) {
        radi_str = ProcessSemList(bucket->first_child, error_code);
        if (error_code) {
          delete[] base_str;
          delete[] radi_str;
          return NULL;
        }
      }

      if (base_str && radi_str) {
        size_t bln = strlen(db_production)
          + strlen(base_str) + strlen(radi_str);
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

char *MapleWrapper::SQROOTtoEstr(SEMANTICS_NODE * sqrt_info, int& error_code)
{
  char *zh_rv = NULL;
  error_code = 0;

  bool use_power_form = false;
  if (use_power_form) {
    zh_rv = SURDasPOWERtoEstr(sqrt_info, error_code);
  } else {
    char *key = "sqrt";
    const char *db_production;
    U32 ID, subID;
    if (GetdBaseNamedRecord("SCHEMATA", key, &db_production, ID, subID)) {
      TCI_ASSERT(db_production);  // sqrt<uID3.0>sqrt(%base%)
      char *base_str = NULL;

      BUCKET_REC *bucket =
        FindBucketRec(sqrt_info->bucket_list, MB_ROOT_BASE);
      if (bucket && bucket->first_child) {
        base_str = ProcessSemList(bucket->first_child, error_code);
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

char *MapleWrapper::SURDasPOWERtoEstr(SEMANTICS_NODE * surd_info,
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
      den_str = ProcessSemList(bucket->first_child, error_code);

      if (error_code) {
        delete[] den_str;
        return NULL;
      }
      den_str = NestInParens(den_str, false);
    } else
      den_str = "2";

    if (den_str) {
      size_t bln = strlen(db_production) + strlen(num_str) + strlen(den_str);
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
      base_str = ProcessSemList(base_bucket->first_child, error_code);

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

char *MapleWrapper::CONJUGATEtoEstr(SEMANTICS_NODE * snode, int& error_code)
{
  char *rv = NULL;
  error_code = 0;

  char *key = "conjugate";
  const char *db_production;
  U32 ID, subID;
  if (GetdBaseNamedRecord("SCHEMATA", key, &db_production, ID, subID)) {
    if (db_production) {        // conjugate<uID48.0>conjugate%arg%
      char *base_str = NULL;

      if (snode && snode->bucket_list) {
        base_str =
          ProcessSemList(snode->bucket_list->first_child, error_code);
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

char *MapleWrapper::TRANSPOSEtoEstr(SEMANTICS_NODE * snode, char TorH,
                                    int& error_code)
{
  char *rv = NULL;
  error_code = 0;

  char *key = (TorH == 'T') ? "Transpose" : "Hermitian Transpose";
  const char *db_production;
  U32 ID, subID;
  if (GetdBaseNamedRecord("SCHEMATA", key, &db_production, ID, subID)) {
    if (db_production && *db_production) {
      // Transpose<uID47.0>transpose(%matrix%)
      char *m_str = NULL;

      if (snode && snode->bucket_list && snode->bucket_list->first_child) {
        SEMANTICS_NODE *s_matrix = snode->bucket_list->first_child;
        m_str = ProcessSemList(s_matrix, error_code);
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

char *MapleWrapper::TABULATIONtoEstr(SEMANTICS_NODE * table_info,
                                     int& error_code)
{
  char *rv = NULL;
  error_code = 0;

  char *key;
  bool use_vector = false;
  if (table_info->nrows == 1 && !matrix_form_required) {
    use_vector = true;
    key = "vector";
  } else {
    key = "matrix";
  }
  const char *db_production;
  U32 ID, subID;
  if (GetdBaseNamedRecord("SCHEMATA", key, &db_production, ID, subID)) {
    TCI_ASSERT(db_production && *db_production);
    // matrix<uID6.0>matrix(%nrows%,%ncols%,[%cells%])
    // vector<uID51.0>vector([%cells%])

    char *cells_str = NULL;
    U32 cln = 0;

    const char *ptr = strstr(db_production, "%cells%");
    if (ptr) {
      // matrix takes a list of cells
      int err_code = 0;
      int cell_counter = 0;
      BUCKET_REC *bucket_rover = table_info->bucket_list;
      while (bucket_rover && !err_code) {
        if (cell_counter != 0)
          cells_str = AppendStr2HeapStr(cells_str, cln, ",");
        if (bucket_rover->first_child) {
          SEMANTICS_NODE *curr_handle = bucket_rover->first_child;
          char *obj_str = ProcessSemList(curr_handle, err_code);
          if (obj_str) {
            if (!err_code)
              cells_str = AppendStr2HeapStr(cells_str, cln, obj_str);
            delete[] obj_str;
          }
        }
        cell_counter++;
        bucket_rover = bucket_rover->next;
      }

      if (err_code) {
        error_code = err_code;
        delete[] cells_str;
        return NULL;
      }
    } else {
      TCI_ASSERT(0);
    }

    // Compose the return string
    size_t bln = strlen(db_production) + 1;
    if (cells_str)
      bln += strlen(cells_str);
    rv = new char[bln];
    strcpy(rv, db_production);
    char num_buffer[80];
    if (!use_vector) {
      sprintf(num_buffer, "%d", table_info->nrows);
      StrReplace(rv, bln, "%nrows%", num_buffer);
      sprintf(num_buffer, "%d", table_info->ncols);
      StrReplace(rv, bln, "%ncols%", num_buffer);
    }
    StrReplace(rv, bln, "%cells%", cells_str);
    delete[] cells_str;
    new_packages[MPL_LINALG] = 1;
  } else {
    TCI_ASSERT(0);
  }
  return rv;
}

// Note that in SWP the data for a Simplex command is entered
//  into a matrix.  This matrix is re-typed as a simplex
//  in CompEngine.  The following functions handles all 5
//  Maple Simplex commands that are supported by SWP.

char *MapleWrapper::SIMPLEXtoEstr(SEMANTICS_NODE * simplex_info,
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
      expr_str = ProcessSemList(cond_func_bucket->first_child, error_code);
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
          char *obj_str = ProcessSemList(curr_handle, err_code);
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
    new_packages[MPL_SIMPLEX] = 1;
  }

  return rv;
}

// SWP accepts a 1 column matrix with equations and conditions
//  like "x \in [1,2]" as input to Solve Numeric.
// I'm using SEM_TYP_NUMSYSTEM to carry this type of data.

char *MapleWrapper::NUMSYSTEMtoEstr(SEMANTICS_NODE * numsys_info,
                                    int& error_code)
{
  char *rv = NULL;

  char *key = numsys_info->contents;  // "Numeric System"
  const char *db_production;
  U32 ID, subID;
  if (!GetdBaseNamedRecord("SCHEMATA", key, &db_production, ID, subID)) {
    TCI_ASSERT(0);
    return rv;
  }

  if (db_production && *db_production) {
    // Numeric System<uID35.0>%eqn_set%,%cond_set%
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
          char *row_str =
            ProcessSemList(bucket_rover->first_child, error_code);
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
          char *row_str =
            ProcessSemList(bucket_rover->first_child, error_code);
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

char *MapleWrapper::SOLVESYSTEMtoEstr(SEMANTICS_NODE * numsys_info,
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
      coeff_matrix_str = ProcessSemList(bucket->first_child, error_code);
      if (error_code) {
        delete[] coeff_matrix_str;
        return NULL;
      }
    }

    bucket = FindBucketRec(numsys_info->bucket_list, MB_SYSTEM_VALUES);
    if (bucket && bucket->first_child) {
      col_matrix_str = ProcessSemList(bucket->first_child, error_code);
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
//  in a matrix.  Such matrices are re-typed as ODEs in CompEngine.
// The engine database holds schemata that are used to construct
//  engine strings for these types.

char *MapleWrapper::ODEtoEstr(SEMANTICS_NODE * ODE_info, int& error_code)
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

    char *expr_list_str = NULL;
    U32 expr_ln = 0;
    char *sfor_func_str = NULL;
    U32 sfor_ln = 0;

    int err_code = 0;
    int eqn_counter = 0;

    // Process all MB_ODEFUNCs first.
    BUCKET_REC *bucket_rover = ODE_info->bucket_list;
    while (bucket_rover && err_code != 1) {
      if (bucket_rover->first_child) {
        SEMANTICS_NODE *s_eqn = bucket_rover->first_child;
        if (bucket_rover->bucket_ID == MB_ODEFUNC)
          ODEFuncName2eStr(bucket_rover->first_child,
                           &sfor_func_str, sfor_ln);
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
          char *obj_str = ProcessSemList(s_eqn, err_code);
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
  } else {
    TCI_ASSERT(0);
  }
  return rv;
}

char *MapleWrapper::ODENumerictoEstr(SEMANTICS_NODE * ODE_info,
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
        SEMANTICS_NODE *s_eqn = bucket_rover->first_child;
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
          char *obj_str = ProcessSemList(s_eqn, err_code);
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
    if (procs_defined)
      ODE_num_procnum++;
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
  } else {
    TCI_ASSERT(0);
  }
  return rv;
}

/************** Start Alias handling ****************************/

ClientAliases_REC *MapleWrapper::CreateClientRecord(U32 client_num)
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

NAME2AKA_REC *MapleWrapper::PushNamingRecord(const char *eng_name,
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
    } else {
      new_node->user_name = NULL;
    }
    new_node->obj_ilk = obj_ilk;
    rv = new_node;
  } else {
    TCI_ASSERT(0);
  }
  return rv;
}

void MapleWrapper::DisposeAKAsList(NAME2AKA_REC * aka_list)
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

NAME2AKA_REC *MapleWrapper::GetAKARec(NAME2AKA_REC * aka_list,
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

bool MapleWrapper::LoadPackages(const char *new_packages)
{
  bool rv = true;

  U32 ID = 0;
  U32 subID = 0;

  const char *cmd_template;
  char p_name[80];
  if (GetdBaseIDRecord("MAPLEPACS", ID, subID, &cmd_template, p_name, 80)) {
    // with(%s):
  } else {
    TCI_ASSERT(0);
  }

  int i = 1;
  while (rv && i < MPL_LASTPACKAGE) {
    if (new_packages[i] && !loaded_packages[i]) {
      const char *eng_dbase_rec;
      char p_name[80];
      if (GetdBaseIDRecord("MAPLEPACS", i, subID, &eng_dbase_rec, p_name, 80)) {
        char cmd[512];
        sprintf(cmd, cmd_template, p_name);
        ByteArray ba(256);
        ba += cmd;
        rv = mapl_eng->SendToProcessor(&ba, 0);
        if (rv)
          loaded_packages[i] = 1;
      }
    }
    i++;
  }

  return rv;
}

ClientAliases_REC *MapleWrapper::GetClientAliasesRec(U32 targ_client_ID)
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

char *MapleWrapper::CreateEngName(U32 client_ID,
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
      } else {
        sprintf(mangled_name, "q%d%s", client_ID, preferred_eng_name);
      }
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

  curr_client->AKAs_list = PushNamingRecord(eng_name, obj_canonical_name,
                                            preferred_eng_name, function_ilk,
                                            curr_client->AKAs_list);

  return eng_name;
}

// The following func needs more work.

char *MapleWrapper::CanonicalNumberToEngineStr(const char *num_str,
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

void MapleWrapper::SetWrapperCmdContext(U32 cmd_ID, /* out */
                                        char *new_packages)
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
  // CCID_Combine_Arctan
  // CCID_Combine_Hyp_Trig_Functions
    context_cmdtemplatetype = CT_NORMAL;
    break;

  case CCID_Factor:
    context_cmdtemplatetype = CT_NORMAL;
    break;
  case CCID_Expand:
    context_cmdtemplatetype = CT_NORMAL;
    break;
  case CCID_Rewrite_Rational:
    context_cmdtemplatetype = CT_NORMAL;
    break;
  case CCID_Rewrite_Float:
    context_cmdtemplatetype = CT_NORMAL;
    break;
  case CCID_Rewrite_Mixed:
    context_cmdtemplatetype = CT_NORMAL;
    break;
  case CCID_Rewrite_Exponential:
    context_cmdtemplatetype = CT_NORMAL;
    break;
  case CCID_Rewrite_Factorial:
    context_cmdtemplatetype = CT_NORMAL;
    break;
  case CCID_Rewrite_Gamma:
    context_cmdtemplatetype = CT_NORMAL;
    break;
  case CCID_Rewrite_Logarithm:
    context_cmdtemplatetype = CT_NORMAL;
    break;
  case CCID_Rewrite_sin_and_cos:
    context_cmdtemplatetype = CT_NORMAL;
    break;
  case CCID_Rewrite_sinh_and_cosh:
    context_cmdtemplatetype = CT_NORMAL;
    break;
  case CCID_Rewrite_tan:
    context_cmdtemplatetype = CT_NORMAL;
    break;
  case CCID_Rewrite_Polar:
    context_cmdtemplatetype = CT_NORMAL;
    break;
  case CCID_Rewrite_Rectangular:
    context_cmdtemplatetype = CT_NORMAL;
    break;

  //  case 22 :  //Rewrite Normal Form

  case CCID_Rewrite_Equations_as_Matrix:
    context_cmdtemplatetype = CT_EQN2MAT;
    new_packages[MPL_LINALG] = 1;
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

  case CCID_Polynomial_Divide:
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
    new_packages[MPL_LINALG] = 1;
    break;

  case CCID_Calculus_Integrate_by_Parts: //Calculus Integrate by Parts...
    context_cmdtype = CC_HOLDINTEGRAL;
    context_cmdtemplatetype = CT_CALCBYPARTS;
    new_packages[MPL_STUDENT] = 1;
    break;
  case CCID_Calculus_Change_Variable:  //Calculus Change Variable...
    context_cmdtype = CC_HOLDINTEGRAL;
    context_cmdtemplatetype = CT_CALCCHANGEVAR;
    new_packages[MPL_STUDENT] = 1;
    break;
  case CCID_Calculus_Partial_Fractions:  //Calculus Partial Fractions
    context_cmdtemplatetype = CT_EXPRandVAR;
    break;
  case CCID_Calculus_Approximate_Integral: //Calculus Approximate Integral...
    context_cmdtype = CC_APPROXINTEGRAL;
    context_cmdtemplatetype = CT_CALCAPPROXINT;
    new_packages[MPL_STUDENT] = 1;
    break;
  case CCID_Calculus_Plot_Approximate_Integral:  //Calculus Plot Approximate Integral
    context_cmdtemplatetype = CT_UNKNOWN;
    break;
  case CCID_Calculus_Find_Extrema: //Calculus Find Extrema
    context_cmdtemplatetype = CT_EXTREMA;
    new_packages[MPL_STUDENT] = 1;
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
  case CCID_Solve_ODE_Exact:   //Solve ODE Exact
  case CCID_Solve_ODE_Laplace: //Solve ODE Laplace
  case CCID_Solve_ODE_Numeric: //Solve ODE Numeric
  case CCID_Solve_ODE_Series:  //Solve ODE Series
    context_cmdtype = CC_ODE;
    context_cmdtemplatetype = CT_NORMAL;
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
    new_packages[MPL_LINALG] = 1;
    break;
  case CCID_Characteristic_Polynomial: //Characteristic Polynomial
    context_cmdtemplatetype = CT_CHARACTERISTICP;
    new_packages[MPL_LINALG] = 1;
    break;
  case CCID_Cholesky_Decomposition:  //Cholesky Decomposition
    context_cmdtemplatetype = CT_NORMAL;
    new_packages[MPL_LINALG] = 1;
    break;
  case CCID_Column_Basis:      //Column Basis
    context_cmdtemplatetype = CT_NORMAL;
    new_packages[MPL_LINALG] = 1;
    break;
  case CCID_Concatenate:       //Concatenate
    context_cmdtemplatetype = CT_NORMAL;
    new_packages[MPL_LINALG] = 1;
    break;
  case CCID_Condition_Number:  //Condition Number
    context_cmdtemplatetype = CT_NORMAL;
    new_packages[MPL_LINALG] = 1;
    break;
  case CCID_Definiteness_Tests:  //Definiteness Tests
    context_cmdtemplatetype = CT_NORMAL;
    new_packages[MPL_LINALG] = 1;
    break;
  case CCID_Determinant:       //Determinant
    context_cmdtemplatetype = CT_NORMAL;
    new_packages[MPL_LINALG] = 1;
    break;
  case CCID_Eigenvalues:       //Eigenvalues
    context_cmdtemplatetype = CT_NORMAL;
    new_packages[MPL_LINALG] = 1;
    break;
  case CCID_Eigenvectors:      //Eigenvectors
    context_cmdtemplatetype = CT_NORMAL;
    new_packages[MPL_LINALG] = 1;
    break;

  case CCID_Fill_Matrix:       //Fill Matrix... randmatrix(%nrows%,%ncols%);
    context_cmdtemplatetype = CT_FILLMATRIX;
    new_packages[MPL_LINALG] = 1;
    break;

  case CCID_Fraction_Free_Gaussian_Elimination:  //Fraction-Free Gaussian Elimination
    context_cmdtemplatetype = CT_NORMAL;
    new_packages[MPL_LINALG] = 1;
    break;
  case CCID_Gaussian_Elimination:  //Gaussian Elimination
    context_cmdtemplatetype = CT_NORMAL;
    new_packages[MPL_LINALG] = 1;
    break;
  case CCID_Hermite_Normal_Form: //Hermite Normal Form
    context_cmdtemplatetype = CT_NORMAL;
    new_packages[MPL_LINALG] = 1;
    break;
  case CCID_Hermitian_Transpose: //Hermitian Transpose
    context_cmdtemplatetype = CT_NORMAL;
    new_packages[MPL_LINALG] = 1;
    break;
  case CCID_Inverse:           //Inverse
    context_cmdtemplatetype = CT_NORMAL;
    new_packages[MPL_LINALG] = 1;
    break;
  case CCID_Jordan_Form:       //Jordan Form
    context_cmdtemplatetype = CT_NORMAL;
    new_packages[MPL_LINALG] = 1;
    break;
  case CCID_Map_Entries:       //Map Entries...
    context_cmdtemplatetype = CT_MAPENTRIES;
    break;
  case CCID_Minimal_Polynomial:
    context_cmdtemplatetype = CT_MATRIXMINPOL;
    new_packages[MPL_LINALG] = 1;
    break;

  case CCID_Norm:              //Norm     norm(%s,2);
    context_cmdtemplatetype = CT_NORMAL;
    new_packages[MPL_LINALG] = 1;
    break;

  case CCID_Nullspace_Basis:   //Nullspace Basis
    context_cmdtemplatetype = CT_NORMAL;
    new_packages[MPL_LINALG] = 1;
    break;
  case CCID_Orthogonality_Test:  //Orthogonality Test
    context_cmdtemplatetype = CT_NORMAL;
    new_packages[MPL_LINALG] = 1;
    break;
  case CCID_Permanent:         //Permanent
    context_cmdtemplatetype = CT_NORMAL;
    new_packages[MPL_LINALG] = 1;
    break;
  case CCID_PLU_Decomposition: //PLU Decomposition
    context_cmdtemplatetype = CT_NORMAL;
    new_packages[MPL_LINALG] = 1;
    break;
  case CCID_QR_Decomposition:  //QR Decomposition
    context_cmdtemplatetype = CT_NORMAL;
    new_packages[MPL_LINALG] = 1;
    break;

  case CCID_Random_Matrix:     //Random Matrix... tcirandmat(5,5,1,-99..99);
    context_cmdtemplatetype = CT_RANDMATRIX;
    new_packages[MPL_LINALG] = 1;
    break;

  case CCID_Rank:              //Rank
    context_cmdtemplatetype = CT_NORMAL;
    new_packages[MPL_LINALG] = 1;
    break;
  case CCID_Rational_Canonical_Form: //Rational Canonical Form
    context_cmdtemplatetype = CT_NORMAL;
    new_packages[MPL_LINALG] = 1;
    break;
  case CCID_Reduced_Row_Echelon_Form:  //Reduced Row Echelon Form
    context_cmdtemplatetype = CT_NORMAL;
    new_packages[MPL_LINALG] = 1;
    break;

  case CCID_Reshape:           //Reshape...
    context_cmdtemplatetype = CT_RESHAPE;
    new_packages[MPL_LINALG] = 1;
    break;

  case CCID_Row_Basis:         //Row Basis
    context_cmdtemplatetype = CT_NORMAL;
    new_packages[MPL_LINALG] = 1;
    break;
  case CCID_Singular_Values:   //Singular Values
    context_cmdtemplatetype = CT_NORMAL;
    new_packages[MPL_LINALG] = 1;
    break;
  case CCID_Smith_Normal_Form: //Smith Normal Form
    context_cmdtemplatetype = CT_NORMAL;
    new_packages[MPL_LINALG] = 1;
    break;
  case CCID_Spectral_Radius:   //Spectral Radius
    context_cmdtemplatetype = CT_NORMAL;
    new_packages[MPL_LINALG] = 1;
    break;
  case CCID_Stack:             //Stack
    context_cmdtemplatetype = CT_NORMAL;
    new_packages[MPL_LINALG] = 1;
    break;
  case CCID_SVD:               //SVD
    context_cmdtemplatetype = CT_NORMAL;
    new_packages[MPL_LINALG] = 1;
    break;
  case CCID_Trace:             //Trace
    context_cmdtemplatetype = CT_NORMAL;
    new_packages[MPL_LINALG] = 1;
    break;
  case CCID_Transpose:         //Transpose
    context_cmdtemplatetype = CT_NORMAL;
    new_packages[MPL_LINALG] = 1;
    break;

  case CCID_Simplex_Dual:      //Simplex Dual
  case CCID_Simplex_Feasible:  //Simplex Feasible
  case CCID_Simplex_Maximize:  //Simplex Maximize
  case CCID_Simplex_Minimize:  //Simplex Minimize
  case CCID_Simplex_Standardize: //Simplex Standardize
    context_cmdtemplatetype = CT_NORMAL;
    break;

  case CCID_Fit_Curve_to_Data: //Fit Curve to Data...
    context_cmdtemplatetype = CT_FITCURVE;
    new_packages[MPL_STATS] = 1;
    break;

  case CCID_Random_Numbers:    //Random Numbers...
    context_cmdtemplatetype = CT_RANDOMNUM;
    new_packages[MPL_STATS] = 1;
    break;

  case CCID_Mean:              //Mean
    context_cmdtemplatetype = CT_NORMAL;
    new_packages[MPL_STATS] = 1;
    break;
  case CCID_Median:            //Median
    context_cmdtemplatetype = CT_NORMAL;
    new_packages[MPL_STATS] = 1;
    break;
  case CCID_Mode:              //Mode
    context_cmdtemplatetype = CT_NORMAL;
    new_packages[MPL_STATS] = 1;
    break;
  case CCID_Correlation:       //Correlation
    context_cmdtemplatetype = CT_NORMAL;
    new_packages[MPL_STATS] = 1;
    break;
  case CCID_Covariance:        //Cov&ariance
    context_cmdtemplatetype = CT_NORMAL;
    new_packages[MPL_STATS] = 1;
    break;
  case CCID_Geometric_Mean:    //Geometric Mean
    context_cmdtemplatetype = CT_NORMAL;
    new_packages[MPL_STATS] = 1;
    break;
  case CCID_Harmonic_Mean:     //Harmonic Mean
    context_cmdtemplatetype = CT_NORMAL;
    new_packages[MPL_STATS] = 1;
    break;
  case CCID_Mean_Deviation:    //Mean Deviation
    context_cmdtemplatetype = CT_NORMAL;
    new_packages[MPL_STATS] = 1;
    break;

  case CCID_Moment:            //Moment...
    context_cmdtemplatetype = CT_MOMENT;
    new_packages[MPL_STATS] = 1;
    break;

  case CCID_Quantile:          //Quantile...
    context_cmdtemplatetype = CT_QUANTILE;
    new_packages[MPL_STATS] = 1;
    break;

  case CCID_Standard_Deviation:  //Standard Deviation
    context_cmdtemplatetype = CT_NORMAL;
    new_packages[MPL_STATS] = 1;
    break;
  case CCID_Variance:          //Variance
    context_cmdtemplatetype = CT_NORMAL;
    new_packages[MPL_STATS] = 1;
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

  default:
    TCI_ASSERT(0);
    break;
  }
}

//Evaluate<uID1.0>eval(%s);

char *MapleWrapper::CreateStandardeStr(const char *eng_zstr,
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

//Polynomial Partial Fractions<uID31.0>tciparfrac(%expr%);
//Polynomial Partial Fractions<uID31.1>tciparfrac(%expr%,%which_var%);
//Minimal Polynomial<uID81.0>tciminpoly(%expr%);
//..Minimal Polynomial<uID81.1>tciminpoly(%expr%,%which_var%);
//Calculus Implicit Differentiation...<uID43.1>diff(%expr%,%which_var%);

char *MapleWrapper::CreateWHICHVAReStr(MathServiceRequest & msr, U32 cmd_ID,
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
                if (QueryEngineIsDefined(map->alias))
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

            if (cmd_ID == CCID_Calculus_Implicit_Differentiation) //Calculus Implicit Differentiation...
              if (VarsAreDefined(s_param))
                error_code = CR_badindepvar;

            int err_code;
            char *v_str = VarsStrFromSNODE(s_param, err_code);
            if (v_str) {
              vars_str = AppendStr2HeapStr(vars_str, vars_ln, v_str);
              delete[] v_str;
            }
          } else
            TCI_ASSERT(0);
        }

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

char *MapleWrapper::CreateITERATEeStr(MathServiceRequest & msr,
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
        z_func_nom = ProcessSemList(s_param, error_code);
      } else
        TCI_ASSERT(0);

      char *start_str = CreateVParameStr(msr, PID_iterstart,
                                         zPT_ASCII_text, error_code);
      if (error_code)
        return NULL;

      char *tally_str = CreateVParameStr(msr, PID_itercount,
                                         zPT_ASCII_natural, error_code);
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

char *MapleWrapper::CreateSIntegraleStr(MathServiceRequest & msr, U32 cmd_ID,
                                        const char *eng_zstr,
                                        const char *cmd_template,
                                        int& error_code)
{
  char *zh_rv = NULL;
  error_code = 0;

  if (cmd_template) {
    //Calculus Integrate by Parts...<uID36.0>intparts(%integral%,%part%);
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
        part = ProcessSemList(s_param, error_code);
      } else
        TCI_ASSERT(0);

      const char *ptr = strstr(eng_zstr, "Int");
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

char *MapleWrapper::CreateAPPROXINTeStr(MathServiceRequest & msr,
                                        const char *eng_zstr,
                                        const char *cmd_template,
                                        int& error_code)
{
  char *zh_rv = NULL;
  error_code = 0;

  if (cmd_template && eng_zstr) {
    //Calculus Approximate Integral...<uID39.0>tciaprxarea(%integral%,%subintervals%,%formula%);

    char *n_intervals = CreateVParameStr(msr, PID_approxintnsubs,
                                         zPT_ASCII_natural, error_code);
    if (error_code)
      return NULL;

    U32 p_type;
    const char *z_formula = msr.GetParam(PID_approxintform, p_type);  // 1..5

    if (z_formula) {
      size_t zln =
        strlen(cmd_template) + strlen(eng_zstr) + strlen(z_formula);
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
    } else {
      TCI_ASSERT(0);
      delete[] n_intervals;
      error_code = PID_first_badparam + PID_approxintform;
    }
  } else {
    TCI_ASSERT(0);
  }
  return zh_rv;
}

char *MapleWrapper::CreateFITCURVEeStr(MathServiceRequest & msr,
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
    if (GetdBaseIDRecord("COMMANDS", CCID_Fit_Curve_to_Data, subID,
                         &cmd_template, p_name, 80)) {
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
    } else {
      TCI_ASSERT(0);
    }
  } else {
    TCI_ASSERT(0);
  }
  return zh_rv;
}

char *MapleWrapper::CreateFILLMATRIXeStr(MathServiceRequest & msr,
                                         int& error_code)
{
  char *zh_rv = NULL;
  error_code = 0;

  // FillMatrix zero<uID74.0>tcizmat(%nRows%,%nCols%);
  // FillMatrix Identity<uID74.1>tciimat(%nRows%,%nCols%);
  // FillMatrix random<uID74.2>randmatrix(%nRows%,%nCols%);
  // FillMatrix Jordan block<uID74.3>tciJBmat(%diag_entry%,%nRows%,%nCols%);
  // FillMatrix by function<uID74.4>matrix(%nRows%,%nCols%,%func_nom%);
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
      if (ilk >= 4) {           // extra param required
        SEMANTICS_NODE *sn = msr.GetSemanticsFromParam(PID_FillMatrixExtra);
        if (sn) {
          extra = ProcessSemNode(sn, error_code);
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

char *MapleWrapper::CreateTRANSFORMeStr(const char *eng_zstr,
                                        const char *cmd_template,
                                        int& error_code)
{
  char *zh_rv = NULL;

  if (eng_zstr) {
    zh_rv = TRANSFORMtoEstr(NULL, eng_zstr, cmd_template, error_code);
  }

  return zh_rv;
}

char *MapleWrapper::CreateVCALCeStr(MathServiceRequest & msr,
                                    const char *eng_zstr,
                                    const char *cmd_template,
                                    int& error_code)
{
  char *zh_rv = NULL;

  if (eng_zstr)
    zh_rv = VCALCOPtoEstr(&msr, NULL, 0, eng_zstr, cmd_template, error_code);

  return zh_rv;
}

//Rewrite Equations as Matrix<uID23.0>tciequationstomatrix(%eqns_or_mat%,vector([%var_list%]));
//Rewrite Matrix as Equations<uID24.0>tcimatrixtoequations(%eqns_or_mat%,[%var_list%]);

char *MapleWrapper::CreateEQN2MATeStr(MathServiceRequest & msr,
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
              if (QueryEngineIsDefined(map->alias))
                error_code = CR_badindepvar;

              if (tally)
                vars_str = AppendStr2HeapStr(vars_str, vars_ln, ",");
              vars_str = AppendStr2HeapStr(vars_str, vars_ln, map->alias);
            } else {
              bool use_alias = false;
              char *eng_var_str = CreateEngName(client_ID, NULL, var,
                                                use_alias, Ilk_Var);
              if (tally)
                vars_str = AppendStr2HeapStr(vars_str, vars_ln, ",");
              vars_str = AppendStr2HeapStr(vars_str, vars_ln, eng_var_str);
              delete[] eng_var_str;
            }
            tally++;
          }

          ptr++;
        }                       // loop thru variables
      } else if (p_type == zPT_ASCII_mmlmarkup) { // "<mi>x</mi>"
        SEMANTICS_NODE *sn = msr.GetSemanticsFromParam(PID_needvarresult);
        if (sn) {
          SEMANTICS_NODE *s_param = sn;

          int err_code;
          char *v_str = VarsStrFromSNODE(s_param, err_code);
          if (v_str) {
            vars_str = AppendStr2HeapStr(vars_str, vars_ln, v_str);
            delete[] v_str;
          }
        } else
          TCI_ASSERT(0);
      }
      if (vars_str) {
        StrReplace(zh_rv, zln, "%var_list%", vars_str);
        delete[] vars_str;
      } else {
        TCI_ASSERT(0);
        StrReplace(zh_rv, zln, "%var_list%", "x");
      }
    } else {
      TCI_ASSERT(0);
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

char *MapleWrapper::CreateCOLLECTeStr(MathServiceRequest & msr,
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
        var_str = ProcessSemList(s_param, error_code);
      } else
        TCI_ASSERT(0);
    } else
      TCI_ASSERT(0);

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

char *MapleWrapper::CreatePSERIESeStr(MathServiceRequest & msr,
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
        var_str = ProcessSemList(s_param, error_code);
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
        num_str = ProcessSemList(s_param, error_code);
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

//Quantile...<uID116.0>tciquantile(%stat_list%,%percent%);

char *MapleWrapper::CreateQUANTILEeStr(MathServiceRequest & msr,
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

char *MapleWrapper::CreateMOMENTeStr(MathServiceRequest & msr,
                                     const char *eng_zstr,
                                     const char *cmd_template,
                                     int& error_code)
{
  char *zh_rv = NULL;
  error_code = 0;

  if (eng_zstr) {
    char *mom_num_str =
      CreateVParameStr(msr, PID_moment_num, zPT_ASCII_natural, error_code);
    if (error_code)
      return NULL;

    char *about_str = NULL;
    U32 p_type;
    const char *meanisorigin_str =
      msr.GetParam(PID_moment_meanisorigin, p_type);
    if (p_type == zPT_ASCII_natural) {  // "0" or "1"
      if (!strcmp(meanisorigin_str, "0")) {
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

    //Moment...<uID115.0>tcimoment(%data_set%,%mom_num%,%about%);

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

char *MapleWrapper::CreateMAPSeStr(MathServiceRequest & msr,
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
        map_str = ProcessSemList(s_param, error_code);
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

char *MapleWrapper::CreateRANDMATRIXeStr(MathServiceRequest & msr,
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

//Random Numbers...<uID107.0>random[%dist%[%param1%,%param2%]](%tally%);

char *MapleWrapper::CreateRANDOMNUMeStr(MathServiceRequest & msr,
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
    int distID = atoi(z_ilk);
    U32 ID = 200 + distID;
    const char *cmd_template;
    char p_name[80];
    if (GetdBaseIDRecord("SCHEMATA", ID, 0, &cmd_template, p_name, 80)) {
      // randombeta<uID200.0>random[beta[%param1%,%param2%]](%tally%);

      char *tally_str =
        CreateVParameStr(msr, PID_RandomNumTally, zPT_ASCII_natural,
                         error_code);
      if (error_code)
        return NULL;

      char *param1_str =
        CreateVParameStr(msr, PID_RandomNumParam1, zPT_ASCII_real,
                         error_code);
      if (error_code) {
        delete[] tally_str;
        return NULL;
      }

      size_t zln = strlen(cmd_template) + strlen(tally_str)
        + strlen(param1_str);

      char *param2_str = NULL;
      const char *p = strstr(cmd_template, "%param2%");
      if (p) {
        param2_str =
          CreateVParameStr(msr, PID_RandomNumParam2, zPT_ASCII_real,
                           error_code);
        if (error_code) {
          delete[] tally_str;
          delete[] param1_str;
          return NULL;
        }
        zln += strlen(param2_str);
      }

      zh_rv = new char[zln];
      strcpy(zh_rv, cmd_template);

      StrReplace(zh_rv, zln, "%param1%", param1_str);
      if (p) {
        StrReplace(zh_rv, zln, "%param2%", param2_str);
        delete[] param2_str;
      }
      StrReplace(zh_rv, zln, "%tally%", tally_str);

      delete[] tally_str;
      delete[] param1_str;
    } else {
      TCI_ASSERT(0);
    }
  } else {
    TCI_ASSERT(0);
    error_code = PID_first_badparam + PID_RandomNumDist;
  }

  return zh_rv;
}

//Reshape...<uID91.0>tcisimplify(%s);
char *MapleWrapper::CreateRESHAPEeStr(const char *eng_zstr,
                                      const char *cmd_template,
                                      int& error_code)
{
  // Implemented farther up in CompEngine
  TCI_ASSERT(0);
  return NULL;
}

bool MapleWrapper::CmdGoesToEngine(U32 cmd_ID)
{
  bool rv = true;
  if (cmd_ID == CCID_Reshape)
    rv = false;

  return rv;
}

char *MapleWrapper::LISTtoEstr(SEMANTICS_NODE * s_function,
                               char *start_str,
                               char *end_str,
                               char *join_str, int& error_code)
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
      } else
        zh_arglist = AppendStr2HeapStr(zh_arglist, aln, join_str);

      if (bucket_rover->first_child) {
        SEMANTICS_NODE *curr_handle = bucket_rover->first_child;
        char *obj_str = ProcessSemList(curr_handle, err_code);
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

bool MapleWrapper::IsMatrix(SEMANTICS_NODE * s_node)
{

  bool rv = false;

  if (s_node && s_node->semantic_type == SEM_TYP_TABULATION)
    rv = true;

  return rv;
}

// Assign Variable<uID13.0>%var%:='%expr%'

char *MapleWrapper::VARASSIGNtoEstr(SEMANTICS_NODE * s_assign_var,
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
      nom_str = ProcessSemList(bucket->first_child, error_code);
      if (error_code) {
        delete[] nom_str;
        return NULL;
      }
    }

    bucket = FindBucketRec(s_assign_var->bucket_list, MB_DEF_VARVALUE);
    if (bucket && bucket->first_child) {
      val_str = ProcessSemList(bucket->first_child, error_code);
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
  } else {
    TCI_ASSERT(0);
  }

  return rv;
}

// tcikill('q2f');
// q2f:='(q2x)->q2x^(2)+2';
// Define Function<uID14.0>%f_nom%:='(%var_list%)->%expr%'
// piecewise<uID?.0>piecewise(%dr_list%)

char *MapleWrapper::FUNCDEFtoEstr(SEMANTICS_NODE * s_funcdef,
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
    // Define Function<uID14.0>%f_nom%:='(%var_list%)->%expr%'
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
        var_list_str = LISTtoEstr(s_func, NULL, NULL, ",", err_code);
        if (err_code) {
          TCI_ASSERT(0);
          error_code = err_code;
          return rv;
        }
      } else {
        TCI_ASSERT(0);
      }
    } else {
      TCI_ASSERT(0);
    }
    // Handle the defining expression

    BUCKET_REC *expr_bucket =
      FindBucketRec(s_funcdef->bucket_list, MB_DEF_FUNCVALUE);
    if (expr_bucket && expr_bucket->first_child) {

      SEMANTICS_NODE *s_content = expr_bucket->first_child;
      int err_code = 0;
      if (s_content->semantic_type == SEM_TYP_PIECEWISE_LIST) {
        expr_str = Piecewise2Estr(s_content, err_code);
      } else {
        expr_str = ProcessSemList(s_content, err_code);
      }
      if (err_code) {
        error_code = err_code;
        delete[] f_nom_str;
        delete[] var_list_str;
        return NULL;
      }
    } else {
      TCI_ASSERT(0);
    }
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
  } else {
    TCI_ASSERT(0);
  }
  return rv;
}


char *MapleWrapper::GetEngNameForUserFunc(SEMANTICS_NODE * s_func)
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
      } else
        TCI_ASSERT(0);
    } else
      TCI_ASSERT(0);
  }

  return rv;
}

char *MapleWrapper::POWERasSURDtoEstr(SEMANTICS_NODE * s_pform,
                                      int& error_code)
{
  char *zh_rv = NULL;
  error_code = 0;

  BUCKET_REC *script_bucket =
    FindBucketRec(s_pform->bucket_list, MB_SCRIPT_UPPER);
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

          BUCKET_REC *bucket = FindBucketRec(s_pform->bucket_list,
                                             MB_SCRIPT_BASE);
          if (bucket && bucket->first_child) {
            base_str = ProcessSemList(bucket->first_child, error_code);
            if (error_code) {
              delete[] base_str;
              return NULL;
            }
          }

          if (!is_sqrt) {
            if (den_bucket && den_bucket->first_child) {
              radi_str = ProcessSemList(den_bucket->first_child, error_code);
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

char *MapleWrapper::GetDiffVarStr(SEMANTICS_NODE * var_snode,
                                  int& error_code)
{
  char *zh_rv = NULL;
  error_code = 0;

  char *dollar_str = NULL;
  if (var_snode->semantic_type == SEM_TYP_POWERFORM) {
    BUCKET_REC *bucket =
      FindBucketRec(var_snode->bucket_list, MB_SCRIPT_UPPER);
    if (bucket && bucket->first_child) {
      dollar_str = ProcessSemList(bucket->first_child, error_code);
      if (error_code) {
        delete[] dollar_str;
        return NULL;
      }
    }
    bucket = FindBucketRec(var_snode->bucket_list, MB_SCRIPT_BASE);
    if (bucket && bucket->first_child)
      var_snode = bucket->first_child;
  }

  char *zh_var = ProcessSemNode(var_snode, error_code);
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

SEMANTICS_NODE *MapleWrapper::LocateVar(BUCKET_REC * expr_bucket)
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

char *MapleWrapper::Subst2Estr(SEMANTICS_NODE * subst_info,
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

      char *curr_subst = ProcessSemList(s_subst, error_code);
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
        } else
          TCI_ASSERT(0);
      }

      if (curr_subst) {
        if (zh_rv)
          zh_rv = AppendStr2HeapStr(zh_rv, rv_ln, ",");
        zh_rv = AppendStr2HeapStr(zh_rv, rv_ln, curr_subst);
        delete[] curr_subst;
      } else
        TCI_ASSERT(0);
    }
    b_rover = b_rover->next;
  }

  return zh_rv;
}

char *MapleWrapper::Piecewise2Estr(SEMANTICS_NODE * pw_list, int& error_code)
{
  char *zh_rv = NULL;

  const char *db_production;
  U32 ID, subID;
  if (GetdBaseNamedRecord("SCHEMATA", "piecewise", &db_production, ID, subID)) {
    TCI_ASSERT(db_production);

    char *dr_list_str = NULL;
    U32 dr_ln = 0;

    const char *ptr = strstr(db_production, "%dr_list%");
    if (ptr) {

      // Maple "piecewise" takes a list   "{domain,expr,}expr"
      //                                  "x<0,    x^2,  x^3"

      int err_code = 0;
      int tally = 0;

      BUCKET_REC *b_rover = pw_list->bucket_list;
      while (b_rover) {
        SEMANTICS_NODE *s_one_piece = b_rover->first_child;
        BUCKET_REC *exp_bucket =
          FindBucketRec(s_one_piece->bucket_list, MB_PIECE_EXPR);
        BUCKET_REC *dom_bucket =
          FindBucketRec(s_one_piece->bucket_list, MB_UNNAMED);

        if (dom_bucket && dom_bucket->first_child && b_rover->next) {
          SEMANTICS_NODE *curr_handle = dom_bucket->first_child;
          char *obj_str = SemDomainToEstr(curr_handle, err_code);
          if (err_code) {
            error_code = err_code;
            delete[] dr_list_str;
            return NULL;
          }
          if (obj_str) {
            if (tally)
              dr_list_str = AppendStr2HeapStr(dr_list_str, dr_ln, ",");
            dr_list_str = AppendStr2HeapStr(dr_list_str, dr_ln, obj_str);
            tally++;
            delete[] obj_str;
          }
        }

        if (exp_bucket && exp_bucket->first_child) {
          SEMANTICS_NODE *curr_handle = exp_bucket->first_child;
          char *obj_str = ProcessSemList(curr_handle, err_code);
          if (err_code) {
            error_code = err_code;
            delete[] dr_list_str;
            return NULL;
          }
          if (obj_str) {
            if (tally)
              dr_list_str = AppendStr2HeapStr(dr_list_str, dr_ln, ",");
            dr_list_str = AppendStr2HeapStr(dr_list_str, dr_ln, obj_str);
            tally++;
            delete[] obj_str;
          }
        }
        b_rover = b_rover->next;
      }
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

char *MapleWrapper::Recursion2Estr(SEMANTICS_NODE * recursion,
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
        char *obj_str = ProcessSemList(curr_handle, error_code);
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
          rfunc_str = ProcessSemList(curr_handle, error_code);
          if (error_code) {
            delete[] rfunc_str;
            delete[] eqn_str;
            return NULL;
          }
        } else {
          TCI_ASSERT(0);
        }
      } else {
        TCI_ASSERT(0);
      }
      b_rover = b_rover->next;
    }                           // loop

    char *key = (tally > 1) ? "Solve CRecursion" : "Solve Recursion";
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

int MapleWrapper::GetEngineMessageID(SEMANTICS_NODE * s_tree)
{
  int rv = 0;

  if (s_tree && !s_tree->next) {
    if (s_tree->semantic_type == SEM_TYP_VARIABLE && s_tree->contents) {
      if (!strcmp(s_tree->contents, "needvars"))
        rv = CR_needvars;
      else if (!strcmp(s_tree->contents, "needconstrainedvars"))
        rv = CR_needvars;
    }
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

char *MapleWrapper::SemDomainToEstr(SEMANTICS_NODE * s_list, int& error_code)
{
  char *zh_rv = NULL;
  U32 zh_ln = 0;

  if (s_list && s_list->semantic_type == SEM_TYP_TEXT)
    s_list = s_list->next;

  while (s_list
         && !s_list->next
         && s_list->semantic_type == SEM_TYP_PRECEDENCE_GROUP) {
    if (s_list->bucket_list && s_list->bucket_list->first_child)
      s_list = s_list->bucket_list->first_child;
    else
      TCI_ASSERT(0);
  }

  int var_pos = 0;
  int tally = 0;
  SEMANTICS_NODE *s_var = NULL;
  SEMANTICS_NODE *s_rover = s_list;
  while (s_rover) {
    if (s_rover->semantic_type == SEM_TYP_VARIABLE) {
      s_var = s_rover;
      var_pos = tally;
    }
    tally++;
    s_rover = s_rover->next;
  }

  if (s_var) {
    if (tally == 3)
      zh_rv = ProcessSemList(s_list, error_code);
    else {
      if (tally == 5 && var_pos == 2) {
        SEMANTICS_NODE *s_save = s_var->next;
        s_var->next = NULL;
        char *zh_str = ProcessSemList(s_list, error_code);
        zh_rv = AppendStr2HeapStr(zh_rv, zh_ln, zh_str);
        delete[] zh_str;

        s_var->next = s_save;
        char *zh_str1 = ProcessSemList(s_var, error_code);
        if (zh_str1) {
          zh_rv = AppendStr2HeapStr(zh_rv, zh_ln, " and ");
          zh_rv = AppendStr2HeapStr(zh_rv, zh_ln, zh_str1);
          delete[] zh_str1;
        }
      } else
        TCI_ASSERT(0);
    }
  } else
    TCI_ASSERT(0);

  return zh_rv;
}

// The semantic analyzer identifies ODE funcs and scripts them
//  in a MB_ODEFUNC bucket in the bucket_list of an SEM_TYP_ODE_SYSTEM.

void MapleWrapper::ODEFuncName2eStr(SEMANTICS_NODE * s_func,
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
      delete[]zh_nom;
    }
  } else
    TCI_ASSERT(0);
}

//  'q1x','q1y','q1z'

bool MapleWrapper::NumericFuncName2eStr(SEMANTICS_NODE * s_func,
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
        strcpy(hold, "'");
        strcat(hold, zh_nom);
        strcat(hold, "'");

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

bool MapleWrapper::GetdBaseNamedRecord(const char *bin_name,
                                           const char *op_name,
                                           const char **eng_dbase_rec,
                                           U32 & ID, U32 & subID)
{
  bool rv = false;

  if (op_name && *op_name) {
    size_t zln = strlen(op_name);
    const char *d_ztemplate;
    if (nom_dBase->
        GetRecordFromName(bin_name, op_name, zln, ID, subID, &d_ztemplate)) {
      *eng_dbase_rec = d_ztemplate;
      rv = true;
    }
  }

  return rv;
}

bool MapleWrapper::GetdBaseIDRecord(const char *bin_name, U32 ID,
                                        U32 subID, const char **eng_dbase_rec,
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

void MapleWrapper::iDisposeSList(SEMANTICS_NODE * slist)
{
  DisposeSList(slist);
}

int MapleWrapper::iGetEngStrPtr(int str_ID, char **engstr, int *engstr_ln)
{
  return mapl_eng->GetEngStrPtr(str_ID, engstr, engstr_ln);
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

char *MapleWrapper::PROBFUNCtoEstr(SEMANTICS_NODE * sem_func_node,
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
      // Translate the first arg ( arg before ";" )
      SEMANTICS_NODE *s_semi = bucket->first_child;
      if (s_semi->bucket_list) {
        BUCKET_REC *op_rover = s_semi->bucket_list;

        SEMANTICS_NODE *s_var = op_rover->first_child;
        str_val = ProcessSemNode(s_var, error_code);
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
        str_val = ProcessSemNode(s_semi, error_code);
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

      new_packages[MPL_STATS] = 1;
    }
  } else {
    TCI_ASSERT(0);
  }
  return zh_rv;
}

char *MapleWrapper::TRANSFORMtoEstr(SEMANTICS_NODE * s_transform,
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
          char *obj_str = ProcessSemList(curr_handle, error_code);
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

    bool is_mapped = false;
    if (!strncmp(expr_str, "matrix", 6))
      is_mapped = true;

    char *key = NULL;
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
    } else
      TCI_ASSERT(0);

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

    char *nom = is_laplace ? "s" : "w";
    char *t_nom = is_laplace ? "t" : "x";

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
    new_packages[MPL_INTTRANS] = 1;
  }
  delete[] expr_str;
  delete[] var_str;
  delete[] tvar_str;

  return zh_rv;
}

// Gradient<uID53.0>tcigrad(%expr%,[%vector_basis%]);

char *MapleWrapper::VCALCOPtoEstr(MathServiceRequest * msr,
                                  SEMANTICS_NODE * s_func, U32 func_ID,
                                  const char *expr, const char *cmd_template,
                                  int& error_code)
{
  char *zh_rv = NULL;
  error_code = 0;

  const char *production = NULL;

  char *expr_str = NULL;
  char *basis_str = NULL;
  U32 zln = 0;

  if (s_func) {
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
      char *obj_str = ProcessSemList(curr_handle, err_code);
      if (obj_str && err_code != 1)
        expr_str = obj_str;
    } else
      TCI_ASSERT(0);

  } else if (expr) {
    size_t bln = strlen(expr);
    expr_str = new char[bln + 1];
    strcpy(expr_str, expr);
    production = cmd_template;
  } else
    TCI_ASSERT(0);

  U32 p_type;
  const char *z_var = msr->GetParam(PID_VecBasisVariables, p_type);
  if (p_type == zPT_ASCII_mmlmarkup) {
    SEMANTICS_NODE *sn = msr->GetSemanticsFromParam(PID_VecBasisVariables);
    if (sn) {
      basis_str = ProcessSemNode(sn, error_code);
      if (error_code)
        return NULL;
    } else
      TCI_ASSERT(0);
  } else
    TCI_ASSERT(0);

  if (expr_str && basis_str) {
    size_t bln = strlen(production) + strlen(expr_str) + strlen(basis_str);
    zh_rv = new char[bln];
    strcpy(zh_rv, production);

    if (strncmp(expr_str, "matrix", 6)) {
      size_t ln = strlen(expr_str) + 10;
      char *tmp = new char[ln];
      strcpy(tmp, "eval(%s%)");
      StrReplace(tmp, ln, "%s%", expr_str);
      delete[] expr_str;
      expr_str = tmp;
    }

    StrReplace(zh_rv, bln, "%expr%", expr_str);
    StrReplace(zh_rv, bln, "%vector_basis%", basis_str);
    new_packages[MPL_LINALG] = 1;
  }

  delete[] expr_str;
  delete[] basis_str;

  if (error_code && zh_rv) {
    delete[] zh_rv;
    zh_rv = NULL;
  }

  return zh_rv;
}

char *MapleWrapper::POLISHOPtoEstr(SEMANTICS_NODE * s_rpolish_op,
                                   bool & is_infix, int& error_code)
{
  char *rv = NULL;
  error_code = 0;
  is_infix = false;

  U32 ID, subID;
  const char *production = NULL;
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
        if (n_matrix_operands == 1 || n_matrix_operands == 2) {
          // Matrix subtraction is a special case in SWP Maple
          if (!strcmp(key, "&#x2212;"))
            req_eng_format = 5;
          else
            req_eng_format = 2;
          db_section = "MATRIXPREFIXFORMS";
          matrix_form_required = true;
          done = true;
        } else if (n_matrix_operands != 0)
          TCI_ASSERT(0);
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
          LISTtoEstr(s_rpolish_op, NULL, NULL, NULL, error_code);
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
            args_str = LISTtoEstr(s_rpolish_op, NULL, NULL, ",", err_code);
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

        rv = LISTtoEstr(s_rpolish_op, NULL, NULL, op_str, error_code);
        if (error_code) {
          delete[] rv;
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

        char *operand =
          LISTtoEstr(s_rpolish_op, NULL, NULL, NULL, error_code);
        if (error_code) {
          delete[] operand;
          return NULL;
        }

        size_t zln = strlen(operand) + strlen(zoperator);
        rv = new char[zln + 1];
        strcpy(rv, operand);
        strcat(rv, zoperator);
        delete[] operand;
      } else if (req_eng_format == 5) { // matrix subtraction
        rv = MATRIXMINUStoEstr(s_rpolish_op, production, error_code);
      }
    } else {                    // entry has no db_production
      TCI_ASSERT(0);
      error_code = CR_failure;
    }
  } else {                      // no db entry
    TCI_ASSERT(0);
    error_code = CR_failure;
  }

  matrix_form_required = false;
  return rv;
}

int MapleWrapper::CountMatrixOperands(SEMANTICS_NODE * s_rpolish_op)
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

// Extract the "shape" arguments from a semantic probability function
//  where the "," separated args are in prefix form.  Note that the first param,
//  separated by ";", has been handled before this function is called.

void MapleWrapper::GetShapeArgs(SEMANTICS_NODE * s_op2,
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

      *str_s1 = ProcessSemNode(s_child1, error_code);
      if (error_code) {
        return;
      }

      b_rover = b_rover->next;
      SEMANTICS_NODE *s_child2 = b_rover->first_child;
      *str_s2 = ProcessSemNode(s_child2, error_code);
      if (error_code) {
        delete[] *str_s1;
        return;
      }

      op_rover = op_rover->next;
      SEMANTICS_NODE *s_child3 = op_rover->first_child;
      *str_s3 = ProcessSemNode(s_child3, error_code);
      if (error_code) {
        if (*str_s1)
          delete[] * str_s1;
        if (*str_s2)
          delete[] * str_s2;
        return;
      }
    } else {
      // Here, only 2 args occurs after the ";"
      *str_s1 = ProcessSemNode(s_child, error_code);
      if (error_code) {
        return;
      }
      op_rover = op_rover->next;
      SEMANTICS_NODE *s_child2 = op_rover->first_child;
      *str_s2 = ProcessSemNode(s_child2, error_code);
      if (error_code) {
        if (*str_s1)
          delete[] * str_s1;
        return;
      }
    }
  } else {
    // Here, only 1 shape param occurs after the ";"
    //  OR there were only comma separators in the list
    *str_s1 = ProcessSemNode(s_comma, error_code);
    if (error_code) {
      return;
    }

    if (s_comma->parent) {
      BUCKET_REC *b_rover = s_comma->parent;
      b_rover = b_rover->next;
      if (b_rover) {
        SEMANTICS_NODE *s_child2 = b_rover->first_child;
        *str_s2 = ProcessSemNode(s_child2, error_code);
        if (error_code) {
          if (*str_s1)
            delete[] * str_s1;
          return;
        }

        b_rover = b_rover->next;
        if (b_rover) {
          SEMANTICS_NODE *s_child3 = b_rover->first_child;
          *str_s3 = ProcessSemNode(s_child3, error_code);
          if (error_code) {
            delete[] *str_s1;
            delete[] *str_s2;
            return;
          }
        }
      }
    }                           // if ( s_comma->parent )
  }
}

// Special handling for PowerMod here.

char *MapleWrapper::MODtoEstr(SEMANTICS_NODE * s_mod, int& error_code)
{
  char *rv = NULL;
  error_code = 0;

  if (s_mod && s_mod->bucket_list && s_mod->bucket_list->first_child) {

    SEMANTICS_NODE *s_mod_arg1 = s_mod->bucket_list->first_child;
    if (s_mod_arg1->semantic_type == SEM_TYP_POWERFORM) {
      // tcipwr<uID34.0>tcipwr(%base%,%superscript%)
      // Power Mod<uID37.0>%base%&^%superscript%
      char *key = "Power Mod";

      BUCKET_REC *bucket =
        FindBucketRec(s_mod_arg1->bucket_list, MB_SCRIPT_BASE);
      if (bucket && bucket->first_child) {
        SEMANTICS_NODE *s_base = bucket->first_child;
        if (s_base->semantic_type == SEM_TYP_TABULATION)
          key = "tcipwr";
      }

      const char *db_production;
      U32 ID, subID;
      if (GetdBaseNamedRecord("SCHEMATA", key, &db_production, ID, subID)) {
        TCI_ASSERT(db_production);

        char *base_str = NULL;
        char *sup_str = NULL;
        char *mod_str = NULL;

        BUCKET_REC *bucket =
          FindBucketRec(s_mod_arg1->bucket_list, MB_SCRIPT_BASE);
        if (bucket && bucket->first_child) {
          base_str = ProcessSemList(bucket->first_child, error_code);
          if (error_code) {
            delete[] base_str;
            return NULL;
          }
        }

        bucket = FindBucketRec(s_mod_arg1->bucket_list, MB_SCRIPT_UPPER);
        if (bucket && bucket->first_child) {
          sup_str = ProcessSemList(bucket->first_child, error_code);
          if (error_code) {
            delete[] base_str;
            delete[] sup_str;
            return NULL;
          }
        }

        bucket = s_mod->bucket_list;
        if (bucket->next && bucket->next->first_child) {
          mod_str = ProcessSemList(bucket->next->first_child, error_code);
          if (error_code) {
            delete[] base_str;
            delete[] sup_str;
            delete[] mod_str;
            return NULL;
          }
        } else
          TCI_ASSERT(0);

        if (base_str && sup_str && mod_str) {
          size_t bln =
            strlen(db_production) + strlen(base_str) + strlen(sup_str);
          char *tmp = new char[bln];
          strcpy(tmp, db_production);
          StrReplace(tmp, bln, "%base%", base_str);
          StrReplace(tmp, bln, "%superscript%", sup_str);

          size_t rv_ln = strlen(tmp) + strlen(mod_str);
          rv = new char[rv_ln + 2];
          strcpy(rv, tmp);
          strcat(rv, ",");
          strcat(rv, mod_str);
          delete[] tmp;
        }

        delete[] base_str;
        delete[] sup_str;
        delete[] mod_str;
      } else {
        TCI_ASSERT(0);
      }
    } else {
      rv = LISTtoEstr(s_mod, NULL, NULL, ",", error_code);
    }
  } else {
    TCI_ASSERT(0);
  }
  return rv;
}

char *MapleWrapper::GetEngNameForODEfunc(SEMANTICS_NODE * s_func,
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

    args_str = LISTtoEstr(s_func, NULL, NULL, ",", error_code);
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

void MapleWrapper::PurgeDEFuncNames()
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

void MapleWrapper::KillIndVar(SEMANTICS_NODE * ODE_info)
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
        SEMANTICS_NODE *s_eqn = bucket_rover->first_child;
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
                  int res = iExecute(cmd);
                }
              }
              arg_rover = arg_rover->next;
            }
          } else
            TCI_ASSERT(0);
        }
      }
      bucket_rover = bucket_rover->next;
    }
  }
}

char *MapleWrapper::NORMtoEstr(SEMANTICS_NODE * s_norm, int& error_code)
{
  char *rv = NULL;
  error_code = 0;

  char *key = "norm";
  const char *db_production;
  U32 ID, subID;
  if (GetdBaseNamedRecord("SCHEMATA", key, &db_production, ID, subID)) {
    if (db_production) {        // norm<uID49.0>tcinorm(%matrix%,%num%)
      char *vec_str = NULL;
      char *num_str = NULL;

      if (s_norm && s_norm->bucket_list) {
        BUCKET_REC *bucket = FindBucketRec(s_norm->bucket_list, MB_UNNAMED);
        if (bucket) {
          vec_str = ProcessSemList(bucket->first_child, error_code);
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
            strcpy(num_str, "frobenius");
          } else {
            num_str = ProcessSemList(s_num, error_code);
            if (error_code) {
              delete[] num_str;
              delete[] vec_str;
              return NULL;
            }
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
  } else {
    TCI_ASSERT(0);
  }
  new_packages[MPL_LINALG] = 1;
  return rv;
}

bool MapleWrapper::QueryEngineIsMatrix(SEMANTICS_NODE * s_expr)
{
  bool rv = false;

  if (s_expr && s_expr->semantic_type != SEM_TYP_NUMBER) {
    bool done = false;
    if (s_expr->subtree_type == 1)
      return true;

    if (!s_expr->next
        && (s_expr->semantic_type == SEM_TYP_VARIABLE
            || s_expr->semantic_type == SEM_TYP_INDEXED_VAR
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

    if (!done) {
      int err_code;
      char *v_str = ProcessSemList(s_expr, err_code);

      if (!err_code && v_str) {
        size_t zln = strlen(v_str);
        char *buffer = new char[zln + 8];
        strcpy(buffer, "eval(");
        strcat(buffer, v_str);
        strcat(buffer, ");");

        LoadPackages(new_packages);
        char *eng_ret = Execute(buffer);
        delete[] buffer;
        delete[] v_str;
        v_str = eng_ret;

        const char *db_production;  // testtypeMatrix<uID31.0>type(%var%,matrix);
        U32 ID, subID;
        if (GetdBaseNamedRecord
            ("SCHEMATA", "testtypeMatrix", &db_production, ID, subID)) {
          if (db_production && *db_production) {
            size_t bln = strlen(db_production) + strlen(v_str);
            char *cmd = new char[bln];
            strcpy(cmd, db_production);
            StrReplace(cmd, bln, "%var%", v_str);

            char *eng_ret = Execute(cmd);

            if (eng_ret) {
              if (!strcmp(eng_ret, "true"))
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
  }

  if (rv)
    s_expr->subtree_type = 1;

  return rv;
}

bool MapleWrapper::QueryEngineIsDefined(char *var_nom)
{
  bool rv = false;

  if (var_nom) {
    char cmd[256];
    sprintf(cmd, "assigned(%s);", var_nom);
    char *eng_ret = Execute(cmd);
    if (eng_ret) {
      if (!strcmp(eng_ret, "true"))
        rv = true;
      delete[] eng_ret;
    }
  }

  return rv;
}

char *MapleWrapper::QVARIABLEtoEstr(SEMANTICS_NODE * s_qvar, int& error_code)
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
    } else {
      s_var = s_qvar;
    }
  } else
    TCI_ASSERT(0);

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

    char *q_list = ProcessSemList(s_qualifier, error_code);

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

bool MapleWrapper::IsTransposeIndicator(SEMANTICS_NODE * s_power,
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
    if (map && QueryEngineIsDefined(map->alias))
      rv = false;
  }

  return rv;
}

char *MapleWrapper::POWERTRANSPOSEtoEstr(SEMANTICS_NODE * s_powerform,
                                         char which_one, int& error_code)
{
  char *rv = NULL;
  error_code = 0;

  char *key = (which_one == 'T') ? "Transpose" : "Hermitian Transpose";
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
        m_str = ProcessSemList(s_matrix, error_code);
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

bool MapleWrapper::DoEvals(ContextType curr_context)
{
  if (curr_context == CC_DEFINITION || curr_context == CC_UNDEFINE)
    return false;
  else
    return true;
}

char *MapleWrapper::MATRIXMINUStoEstr(SEMANTICS_NODE * s_minus,
                                      const char *production,
                                      int& error_code)
{
  char *zh_rv = NULL;
  error_code = 0;

  if (s_minus && s_minus->bucket_list && s_minus->bucket_list->next) {
    SEMANTICS_NODE *s_left = s_minus->bucket_list->first_child;
    SEMANTICS_NODE *s_right = s_minus->bucket_list->next->first_child;
    if (s_left && s_right) {
      char *arg1_str = ProcessSemList(s_left, error_code);
      if (error_code) {
        delete[] arg1_str;
        return NULL;
      }
      char *arg2_str = ProcessSemList(s_right, error_code);
      if (error_code) {
        delete[] arg1_str;
        delete[] arg2_str;
        return NULL;
      }
      if (arg1_str && arg2_str) {
        size_t bln = strlen(production) + strlen(arg1_str) + strlen(arg2_str);
        zh_rv = new char[bln];
        strcpy(zh_rv, production);
        StrReplace(zh_rv, bln, "%arg1%", arg1_str);
        StrReplace(zh_rv, bln, "%arg2%", arg2_str);
      }
      delete[] arg1_str;
      delete[] arg2_str;
    } else {
      TCI_ASSERT(0);
    }
  } else {
    TCI_ASSERT(0);
  }
  return zh_rv;
}

void MapleWrapper::SetVarTypes(SEMANTICS_NODE * root, int & nmatrices)
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

char *MapleWrapper::GetEngNameForVar(U32 frame_ID, const char *user_name,
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
        } else
          TCI_ASSERT(0);
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

/*
MATH_CONTAINER
  UNNAMED
    PRECEDENCE_GROUP
      UNNAMED
        INFIX OPERATOR contents = ","
          UNNAMED
            INFIX OPERATOR contents = ","
              UNNAMED
                VARIABLE canonical_ID = "mix" contents = "x"
              UNNAMED
                VARIABLE canonical_ID = "miy" contents = "y"
          UNNAMED
            VARIABLE canonical_ID = "miz" contents = "z"
*/

char *MapleWrapper::VarsStrFromSNODE(SEMANTICS_NODE * s_param,
                                     int& error_code)
{
  char *rv = NULL;
  U32 rv_ln = 0;

  // Generate a comma separated list of vars, no parens
  if (s_param) {
    SEMANTICS_NODE *s_var = s_param;
    // Descend into math container node
    while (s_var->semantic_type == SEM_TYP_MATH_CONTAINER
           || s_var->semantic_type == SEM_TYP_PRECEDENCE_GROUP) {
      if (s_var->bucket_list && s_var->bucket_list->first_child) {
        s_var = s_var->bucket_list->first_child;
      } else {
        TCI_ASSERT(0);
        break;
      }
    }

    if (s_var->semantic_type == SEM_TYP_VARIABLE) {
      rv = ProcessSemList(s_var, error_code);
      if (!error_code)
        if (QueryEngineIsDefined(rv))
          error_code = CR_badindepvar;
    } else if (s_var->semantic_type == SEM_TYP_INFIX_OP) {
      BUCKET_REC *b_rover = s_var->bucket_list;
      if (b_rover->first_child) {
        SEMANTICS_NODE *s_child1 = b_rover->first_child;
        char *zh_child1 = VarsStrFromSNODE(s_child1, error_code);
        char *zh_child2 = NULL;
        b_rover = b_rover->next;
        if (b_rover && b_rover->first_child) {
          SEMANTICS_NODE *s_child2 = b_rover->first_child;
          zh_child2 = VarsStrFromSNODE(s_child2, error_code);
        } else {
          TCI_ASSERT(0);
        }
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
      } else {
        TCI_ASSERT(0);
      }
    } else {
      TCI_ASSERT(!"unexpected semantic object");
    }
  } else {
    TCI_ASSERT(!"s_param is NULL");
  }
  return rv;
}


bool MapleWrapper::QualifierIsSumVar(SEMANTICS_NODE * s_qvar)
{
  bool rv = false;

  if (s_qvar && s_qvar->bucket_list) {
    BUCKET_REC *b_qual = FindBucketRec(s_qvar->bucket_list, MB_SUB_QUALIFIER);
    SEMANTICS_NODE *s_qualifier = b_qual->first_child;
    int err_code;
    char *q_list = ProcessSemList(s_qualifier, err_code);

    if (q_list) {
      if (!strcmp(global_sum_var, q_list))
        rv = true;
      delete[] q_list;
    }
  } else
    TCI_ASSERT(0);
  return rv;
}

bool MapleWrapper::CmdAllowed(MathServiceRequest * msr,
                                  MathResult * mr, U32 cmd_ID,
                                  SEMANTICS_NODE * child)
{
  bool rv = true;

  return rv;
}

// Search a semantics tree for a variable that is defined

bool MapleWrapper::VarsAreDefined(SEMANTICS_NODE * s_tree)
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
          if (QueryEngineIsDefined(map->alias))
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
// Here we generate an engine string from such a param.

char *MapleWrapper::CreateVParameStr(MathServiceRequest & msr,
                                     U32 param_ID,
                                     U32 alt_ptype, int& error_code)
{
  char *zh_rv = NULL;
  error_code = 0;

  U32 p_type;
  const char *z_param = msr.GetParam(param_ID, p_type);
  if (p_type == zPT_ASCII_mmlmarkup) {
    SEMANTICS_NODE *sn = msr.GetSemanticsFromParam(param_ID);
    if (sn) {
      SEMANTICS_NODE *s_param = sn;
      zh_rv = ProcessSemList(s_param, error_code);
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
