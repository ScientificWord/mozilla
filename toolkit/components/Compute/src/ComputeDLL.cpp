// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

/* ComputeDLL.cpp : Codes the entry points for the ComputeDLL.

  The functionality provided by ComputeDLL is implemented using three
  underlying objects, MathWorkShop, MathServiceRequest, and MathResult.

  (MathServiceRequest and MathResult are bundled into a "transaction"
   in the following implementation.)

  These objects have a much larger interface than ComputeDLL currently
  exposes.  It's easy to add more functions to ComputeDLL.h if required.
*/

#include "ComputeDLL.h"

#include "CmpTypes.h"
#include "MRequest.h"
#include "MResult.h"
#include "WorkShop.h"
#include "fltutils.h"

// global
MathWorkShop *workshop;

typedef struct tagTRANS_REC
{                               // Struct to hold a "transaction record" 
  tagTRANS_REC *next;
  int transaction_ID;
  MathServiceRequest *msr;
  MathResult *mr;
} TRANS_REC;

// more globals
TRANS_REC *transactions_list;
int trans_counter;

TRANS_REC *LocateTransRecord(U32 trans_ID)
{
  TRANS_REC *rover = transactions_list;
  while (rover) {
    if (rover->transaction_ID == trans_ID)
      break;
    rover = rover->next;
  }
  return rover;
}


int ComputeDLL::InitCompDLL()
{
  workshop = new MathWorkShop();
  trans_counter = 0;
  transactions_list = NULL;

  return 1;
}

void ComputeDLL::TermCompDLL()
{
  delete workshop;
  workshop = NULL;

  TRANS_REC *rover = transactions_list;
  while (rover) {
    TRANS_REC *del = rover;
    rover = rover->next;
    delete del->msr;
    delete del->mr;
    delete del;
  }
  transactions_list = NULL;
}

void ComputeDLL::StopProcessor()
{
  workshop -> StopProcessor();
}

//  "C:\\xml\\compute\\testjig\\mplInstall.gmr"
//  "C:\\xml\\compute\\testjig\\mupInstall.gmr"
// returns an engine "ID"

U32 ComputeDLL::InstallEngine(U32 trans_ID, nsILocalFile *install_script, int *result_code)
{
  TRANS_REC *rover = LocateTransRecord(trans_ID);

  if (rover) {
    MathResult *mresult = rover->mr;
    U32 eng_handle = workshop->InstallCompEngine(install_script, *mresult);
    *result_code = mresult->GetResultCode();
    return eng_handle;
  } else {
    return 1;
  }
}

void ComputeDLL::UninstallEngine(U32 eng_ID)
{
  workshop->UninstallCompEngine(NULL, eng_ID);
}

const char *ComputeDLL::GetNextCommand(U32 eng_ID, U32 * curr_cmd_ID)
{
  return workshop->GetNextSupportedCommand(eng_ID, curr_cmd_ID);
}

U32 ComputeDLL::GetClientHandle(U32 parent_handle)
{
  return workshop->GetClientHandle(parent_handle);
}

void ComputeDLL::ReleaseClientHandle(U32 c_handle)
{
  workshop->ReleaseClientHandle(c_handle);
}

U32 ComputeDLL::CreateTransaction(U32 c_handle, const U16 * w_mml, U32 eng_ID, U32 cmd_ID)
{
  MathServiceRequest *msr;
  if ((cmd_ID == CCID_PlotFuncCmd) || (cmd_ID == CCID_PlotFuncQuery)) {
    PlotServiceRequest *psr = new PlotServiceRequest();
    psr->PutMarkupType (MT_GRAPH);
    msr = psr;
  } else {
    msr = new MathServiceRequest();
  }    
  MathResult *mr = new MathResult();
  msr->PutClientHandle(c_handle);
  msr->PutEngineID(eng_ID);
  msr->PutOpID(cmd_ID);
  msr->PutWideMarkup(w_mml);
  trans_counter++;
  TRANS_REC *new_trans = new TRANS_REC();
  new_trans->next = transactions_list;
  new_trans->transaction_ID = trans_counter;
  new_trans->msr = msr;
  new_trans->mr = mr;
  transactions_list = new_trans;
  return trans_counter;
}

void ComputeDLL::ReleaseTransaction(U32 targ_ID)
{
  TRANS_REC *head = NULL;
  TRANS_REC *tail;

  TRANS_REC *rover = transactions_list;
  while (rover) {
    TRANS_REC *curr = rover;
    rover = rover->next;
    if (curr->transaction_ID == targ_ID) {
      delete curr->msr;
      delete curr->mr;
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
  transactions_list = head;
}

bool ComputeDLL::CheckParam(MathServiceRequest * msr, U32 p_ID, U32 expected_type)
{
  U32 p_type;
  msr->GetParam(p_ID, p_type);
  if (p_type == expected_type)
    return true;
  else
    return false;
}

// For commands that require params from inception,
//  the existence and type of each parameter is checked here.
//  There is no attempt to check the "value" of any param.

int ComputeDLL::ValidateParams(U32 cmd_ID, MathServiceRequest * msr)
{
  int rv = 0;

  switch (cmd_ID) {
  case CCID_Polynomial_Collect:{
      if (!CheckParam(msr, PID_PolyCollectVar, zPT_ASCII_mmlmarkup))
        rv = PID_PolyCollectVar + PID_first_badparam;
    }
    break;
  case CCID_Calculus_Integrate_by_Parts:{
      if (!CheckParam(msr, PID_CalcByPartsVar, zPT_ASCII_mmlmarkup))
        rv = PID_CalcByPartsVar + PID_first_badparam;
    }
    break;
  case CCID_Calculus_Change_Variable:{
      if (!CheckParam(msr, PID_CalcChangeVar, zPT_ASCII_mmlmarkup))
        rv = PID_CalcChangeVar + PID_first_badparam;
    }
    break;
  case CCID_Calculus_Approximate_Integral:{
      if (!CheckParam(msr, PID_approxintform, zPT_ASCII_natural))
        rv = PID_approxintform + PID_first_badparam;
      else if (!CheckParam(msr, PID_approxintnsubs, zPT_ASCII_natural)
               && !CheckParam(msr, PID_approxintnsubs, zPT_ASCII_mmlmarkup))
        rv = PID_approxintnsubs + PID_first_badparam;
    }
    break;
  case CCID_Calculus_Iterate:{
      if (!CheckParam(msr, PID_iterfunc, zPT_ASCII_mmlmarkup))
        rv = PID_iterfunc + PID_first_badparam;
      else if (!CheckParam(msr, PID_iterstart, zPT_ASCII_text)
               && !CheckParam(msr, PID_iterstart, zPT_ASCII_mmlmarkup))
        rv = PID_iterstart + PID_first_badparam;
      else if (!CheckParam(msr, PID_itercount, zPT_ASCII_natural)
               && !CheckParam(msr, PID_itercount, zPT_ASCII_mmlmarkup))
        rv = PID_itercount + PID_first_badparam;
    }
    break;
  case CCID_Calculus_Implicit_Differentiation:{
      if (!CheckParam(msr, PID_ImplDiffIndepVar, zPT_ASCII_mmlmarkup))
        rv = PID_ImplDiffIndepVar + PID_first_badparam;
      else if (!CheckParam(msr, PID_ImplDiffDepVars, zPT_ASCII_mmlmarkup))
        rv = PID_ImplDiffDepVars + PID_first_badparam;
    }
    break;
  case CCID_Power_Series:{
      if (!CheckParam(msr, PID_seriesorder, zPT_ASCII_natural)
          && !CheckParam(msr, PID_seriesorder, zPT_ASCII_mmlmarkup))
        rv = PID_seriesorder + PID_first_badparam;
      else if (!CheckParam(msr, PID_seriesvar, zPT_ASCII_mmlmarkup))
        rv = PID_seriesvar + PID_first_badparam;
      else if (!CheckParam(msr, PID_seriesabout, zPT_ASCII_mmlmarkup))
        rv = PID_seriesabout + PID_first_badparam;
    }
    break;
  case CCID_Solve_ODE_Series:{
      if (!CheckParam(msr, PID_ODEseriesorder, zPT_ASCII_natural)
          && !CheckParam(msr, PID_ODEseriesorder, zPT_ASCII_mmlmarkup))
        rv = PID_ODEseriesorder + PID_first_badparam;
      else if (!CheckParam(msr, PID_ODEIndepVar, zPT_ASCII_mmlmarkup))
        rv = PID_ODEIndepVar + PID_first_badparam;
      else if (!CheckParam(msr, PID_ODEseriesabout, zPT_ASCII_mmlmarkup))
        rv = PID_ODEseriesabout + PID_first_badparam;
    }
    break;
  case CCID_Fill_Matrix:{
      if (!CheckParam(msr, PID_FillMatrixIlk, zPT_ASCII_natural))
        rv = PID_FillMatrixIlk + PID_first_badparam;
      else if (!CheckParam(msr, PID_FillMatrixnRows, zPT_ASCII_natural)
               && !CheckParam(msr, PID_FillMatrixnRows, zPT_ASCII_mmlmarkup))
        rv = PID_FillMatrixnRows + PID_first_badparam;
      else if (!CheckParam(msr, PID_FillMatrixnCols, zPT_ASCII_natural)
               && !CheckParam(msr, PID_FillMatrixnCols, zPT_ASCII_mmlmarkup))
        rv = PID_FillMatrixnCols + PID_first_badparam;

    }
    break;
  case CCID_Map_Entries:{
      if (!CheckParam(msr, PID_MapsFunction, zPT_ASCII_mmlmarkup))
        rv = PID_MapsFunction + PID_first_badparam;
    }
    break;
  case CCID_Random_Matrix:{
      if (!CheckParam(msr, PID_RandMatrixType, zPT_ASCII_natural))
        rv = PID_RandMatrixType + PID_first_badparam;
      else if (!CheckParam(msr, PID_RandMatrixnRows, zPT_ASCII_natural)
               && !CheckParam(msr, PID_RandMatrixnRows, zPT_ASCII_mmlmarkup))
        rv = PID_RandMatrixnRows + PID_first_badparam;
      else if (!CheckParam(msr, PID_RandMatrixnCols, zPT_ASCII_natural)
               && !CheckParam(msr, PID_RandMatrixnCols, zPT_ASCII_mmlmarkup))
        rv = PID_RandMatrixnCols + PID_first_badparam;
      else if (!CheckParam(msr, PID_RandMatrixLLim, zPT_ASCII_real)
               && !CheckParam(msr, PID_RandMatrixLLim, zPT_ASCII_mmlmarkup))
        rv = PID_RandMatrixLLim + PID_first_badparam;
      else if (!CheckParam(msr, PID_RandMatrixULim, zPT_ASCII_real)
               && !CheckParam(msr, PID_RandMatrixULim, zPT_ASCII_mmlmarkup))
        rv = PID_RandMatrixULim + PID_first_badparam;
    }
    break;
  case CCID_Reshape:{
      if (!CheckParam(msr, PID_ncolsReshape, zPT_ASCII_natural)
          && !CheckParam(msr, PID_ncolsReshape, zPT_ASCII_mmlmarkup))
        rv = PID_ncolsReshape + PID_first_badparam;
    }
    break;
  case CCID_Fit_Curve_to_Data:{
      if (!CheckParam(msr, PID_dependcolumn, zPT_ASCII_natural))
        rv = PID_dependcolumn + PID_first_badparam;

      U32 p_type;
      msr->GetParam(PID_regressdegree, p_type);
      if (p_type) {
        if (!CheckParam(msr, PID_regressdegree, zPT_ASCII_natural) &&
            !CheckParam(msr, PID_regressdegree, zPT_ASCII_mmlmarkup))
          rv = PID_regressdegree + PID_first_badparam;
      } else {
        if (!CheckParam(msr, PID_regresscode, zPT_ASCII_natural))
          rv = PID_regresscode + PID_first_badparam;
      }
    }
    break;
  case CCID_Random_Numbers:{
      if (!CheckParam(msr, PID_RandomNumTally, zPT_ASCII_natural)
          && !CheckParam(msr, PID_RandomNumTally, zPT_ASCII_mmlmarkup))
        rv = PID_RandomNumTally + PID_first_badparam;
      else if (!CheckParam(msr, PID_RandomNumDist, zPT_ASCII_natural))
        rv = PID_RandomNumDist + PID_first_badparam;
      else if (!CheckParam(msr, PID_RandomNumParam1, zPT_ASCII_real)
               && !CheckParam(msr, PID_RandomNumParam1, zPT_ASCII_mmlmarkup))
        rv = PID_RandomNumParam1 + PID_first_badparam;
      //    else if ( !CheckParam(msr,PID_RandomNumParam2,zPT_ASCII_real) )
      //    &&        !CheckParam(msr,PID_RandomNumParam2,zPT_ASCII_mmlmarkup) )
      //      rv  =  PID_RandomNumParam2 + PID_first_badparam;
    }
    break;
  case CCID_Moment:{
      if (!CheckParam(msr, PID_moment_num, zPT_ASCII_natural)
          && !CheckParam(msr, PID_moment_num, zPT_ASCII_mmlmarkup))
        rv = PID_moment_num + PID_first_badparam;
      else if (!CheckParam(msr, PID_moment_origin, zPT_ASCII_text)
               && !CheckParam(msr, PID_moment_origin, zPT_ASCII_mmlmarkup))
        rv = PID_moment_origin + PID_first_badparam;
      else if (!CheckParam(msr, PID_moment_meanisorigin, zPT_ASCII_natural))
        rv = PID_moment_meanisorigin + PID_first_badparam;
    }
    break;
  case CCID_Quantile:{
      if (!CheckParam(msr, PID_quantile, zPT_ASCII_real)
          && !CheckParam(msr, PID_quantile, zPT_ASCII_mmlmarkup))
        rv = PID_quantile + PID_first_badparam;
    }
    break;

  case CCID_PlotFuncCmd:
  case CCID_PlotFuncQuery:
    ///////////////////////////////////////////////////////////////////////////
    //  {
    //       if (!CheckParam(msr, PID_plotLowerX, zPT_ASCII_real) &&         //
    //           !CheckParam(msr, PID_plotLowerX, zPT_ASCII_mmlmarkup))      //
    //         rv = PID_plotLowerX + PID_first_badparam;                     //
    //       else if (!CheckParam(msr, PID_plotUpperX, zPT_ASCII_real) &&    //
    //                !CheckParam(msr, PID_plotUpperX, zPT_ASCII_mmlmarkup)) //
    //         rv = PID_plotUpperX + PID_first_badparam;                     //
    //       else if (!CheckParam(msr, PID_plotVar1, zPT_ASCII_real) &&      //
    //                !CheckParam(msr, PID_plotVar1, zPT_ASCII_mmlmarkup))   //
    //         rv = PID_plotVar1 + PID_first_badparam;                       //
    //     }                                                                 //
    ///////////////////////////////////////////////////////////////////////////
                                    
    break;

  default:
    break;
  }

  return rv;
}

int ComputeDLL::ExecuteTransaction(U32 trans_ID)
{
  int rv = 0;

  TRANS_REC *rover = LocateTransRecord(trans_ID);
  if (rover) {
    MathServiceRequest *mrequest = rover->msr;
    MathResult *mresult = rover->mr;
    U32 cmd_ID = mrequest->GetOpID();

    rv = ValidateParams(cmd_ID, mrequest);
    if (rv == 0) {
      workshop->ProcessRequest(*mrequest, *mresult);
      rv = mresult->GetResultCode();
    }
  }
  return rv;
}

int ComputeDLL::AddParam(U32 trans_ID, U32 p_ID, U32 p_type, const char *ascii_str)
{
  int rv = 0;

  TRANS_REC *rover = LocateTransRecord(trans_ID);
  if (rover)
    rover->msr->PutParam(p_ID, p_type, ascii_str);
  else
    rv = 1;

  return rv;
}

int ComputeDLL::AddWideParam(U32 trans_ID, U32 p_ID, U32 p_type, const U16 * wide_str)
{
  int rv = 0;

  TRANS_REC *rover = LocateTransRecord(trans_ID);
  if (rover) {
    rover->msr->PutWideParam(p_ID, p_type, wide_str);
  } else {
    rv = 1;
  }
  return rv;
}

const U16 *ComputeDLL::GetPtrWIDEresult(U32 trans_ID, int str_ID)
{
  const U16 *rv = NULL;

  TRANS_REC *rover = LocateTransRecord(trans_ID);
  if (rover) {
    if (str_ID == RES_RESULT) { // the computed result
      rv = rover->mr->GetWideResultStr();
    } else if (str_ID == RES_SENT) {  // the string that went to the engine
      rv = rover->mr->GetWideEngInStr();
    } else if (str_ID == RES_RECEIVED) {  // the string returned by the engine
      rv = rover->mr->GetWideEngOutStr();
    } else if (str_ID == RES_SEMANTICS) { // a dump of the semantics tree sent to the engine
      rv = rover->mr->GetWideSemanticsStr();
    } else if (str_ID == RES_ERROR) { // the error string returned by the engine
      rv = rover->mr->GetWideEngErrorStr();
    }
  }
  return rv;
}

const U16 *ComputeDLL::GetPtrWIDEresultPart(U32 trans_ID, U32 part, int index)
{
  TRANS_REC *rover = LocateTransRecord(trans_ID);

  if (rover) {
    return rover->mr->GetWideComponent(part, index);
  } else {
    return NULL;
  }
}

const char *ComputeDLL::GetPtrASCIIresult(U32 trans_ID, int str_ID)
{
  const char *rv = NULL;

  TRANS_REC *rover = LocateTransRecord(trans_ID);
  if (rover) {
    if (str_ID == RES_RESULT) {
      rv = rover->mr->GetResultStr();
    } else if (str_ID == RES_SENT) {
      rv = rover->mr->GetEngInStr();
    } else if (str_ID == RES_RECEIVED) {
      rv = rover->mr->GetEngOutStr();
    } else if (str_ID == RES_SEMANTICS) {
      rv = rover->mr->GetSemanticsStr();
    } else if (str_ID == RES_ERROR) {
      rv = rover->mr->GetEngErrorStr();
    }
  }
  return rv;
}

const char *ComputeDLL::GetPtrASCIIresultPart(U32 trans_ID, U32 part_ID, int index)
{
  TRANS_REC *rover = LocateTransRecord(trans_ID);
  if (rover) {
    return rover->mr->GetComponent(part_ID, index);
  } else {
    return NULL;
  }
}


DefStore* ComputeDLL::GetDefStore(U32 client_ID) {
  return workshop -> GetDefStore(client_ID);
}

const void *ComputeDLL::GetNextDef(U32 client_ID, U32 engine_ID, const void *curr_def)
{
  return workshop->GetNextDef(client_ID, engine_ID,
                              (const DefInfo *)curr_def);
}

void ComputeDLL::ClearDefs(U32 client_ID, U32 engine_ID)
{
  workshop->ClearDefs(client_ID, engine_ID);
}

int ComputeDLL::SetEngineStateAttr(U32 engine_ID, int attr_ID, const char *s_val)
{
  return workshop->SetEngineAttr(engine_ID, attr_ID, s_val);
}

const char *ComputeDLL::GetEngineStateAttr(U32 engine_ID, int attr_ID)
{
  return workshop->GetEngineAttr(engine_ID, attr_ID);
}

int ComputeDLL::SetUserPref(U32 client_ID, U32 pref_ID, const char *ascii_str)
{
  return workshop->SetClientPref(client_ID, pref_ID, ascii_str);
}

int ComputeDLL::SetWideUserPref(U32 client_ID, U32 pref_ID, const U16 * wide_str)
{
  return workshop->SetClientPrefWide(client_ID, pref_ID, wide_str);
}

const char *ComputeDLL::GetUserPref(U32 client_ID, U32 pref_ID, int no_inherit)
{
  return workshop->GetClientPref(client_ID, pref_ID, no_inherit);
}

const U16 *ComputeDLL::GetWideUserPref(U32 client_ID, U32 pref_ID, int no_inherit)
{
  return workshop->GetClientPrefWide(client_ID, pref_ID, no_inherit);
}
