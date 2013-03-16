// Copyright (c) 2006 MacKichan Software, Inc.  All Rights Reserved.

#include "msiSimpleComputeEngine2.h"

#include "nsMemory.h"
#include "xpcom-config.h"
#include "nsIClassInfoImpl.h"
#include "nsString.h"
#include "nsIGenericFactory.h"
#include "nsIClassInfoImpl.h"


#include "ComputeDLL.h"
#include "iCmpTypes.h"
#include "iCmpIDs.h"
#include "DefInfo.h"
#include <stdio.h>
#include <stdarg.h>

//NS_IMPL_ISUPPORTS1(msiSimpleComputeEngine2, msiISimpleComputeEngine)


msiSimpleComputeEngine2* s_engine;

/* static */ 
msiSimpleComputeEngine2* msiSimpleComputeEngine2::GetInstance()
{
  if (!s_engine) {
    s_engine = new msiSimpleComputeEngine2();
    if (!s_engine)
      return nsnull;

    NS_ADDREF(s_engine);   // addref the global
  }

  NS_ADDREF(s_engine);   // addref the return result
  return s_engine;
}




// forward def.
static void SetupNodeAttrs();

PRUint32 wstrlen(const PRUnichar* s)
{
  PRUint32 len = 0;
  if (s) {
    while (*s++ != 0) {
      len++;
    }
  }
  return len;
}

////////////////////////////////////////////////////////////////////////

msiSimpleComputeEngine2::msiSimpleComputeEngine2() :
    didInit(false), sent_to_engine(NULL), received_from_engine(NULL), engine_errors(NULL)
{
}

msiSimpleComputeEngine2::~msiSimpleComputeEngine2()
{
  nsMemory::Free(engine_errors);
  nsMemory::Free(received_from_engine);
  nsMemory::Free(sent_to_engine);

  if (didInit)
    Shutdown();
}


NS_IMPL_ISUPPORTS1_CI(msiSimpleComputeEngine2, msiISimpleComputeEngine)

/* void startup (in nsILocalFile engFile); */
NS_IMETHODIMP msiSimpleComputeEngine2::Startup(nsILocalFile *engFile)
{
  nsresult rv = NS_OK;
  
  if (didInit)
    return rv;
  //  Shutdown();
  
  if (rv == NS_OK) {
    ComputeDLL::InitCompDLL();  //check return?

    client_handle =  ComputeDLL::GetClientHandle( 0 );
    if (client_handle == 0) {
	    Shutdown();
	    rv = NS_ERROR_FAILURE;
    }

    if (rv == NS_OK) {
      U32 trans_ID  =  ComputeDLL::CreateTransaction( client_handle,NULL,0,0 );

      int result_code = 0;
      MuPAD_eng_ID  =  ComputeDLL::InstallEngine( trans_ID, engFile, &result_code );
      if (MuPAD_eng_ID == 0
      ||  result_code == CR_ScriptOpenFailed
      ||  result_code == CR_ScriptNoEngineDbase
      ||  result_code == CR_NoEntitiesDbase
      ||  result_code == CR_NoUserPrefsDbase
      ||  result_code == CR_EngDbaseOpenFailed
      ||  result_code == CR_EngInitFailed ) {
	      Shutdown();
	      if (result_code == CR_ScriptOpenFailed
        ||  result_code == CR_ScriptNoEngineDbase
        ||  result_code == CR_NoEntitiesDbase
        ||  result_code == CR_NoUserPrefsDbase
        ||  result_code == CR_EngDbaseOpenFailed)
	        rv = NS_ERROR_FILE_NOT_FOUND;
	      else
	        rv = NS_ERROR_NOT_INITIALIZED;
      } else {
        ComputeDLL::ReleaseTransaction( trans_ID );
        SetupNodeAttrs();
        didInit = true;
      }
    }
  }
  return rv;
}

/* void shutdown (); */
NS_IMETHODIMP msiSimpleComputeEngine2::Shutdown()
{
  if (didInit) {
    ComputeDLL::ReleaseClientHandle( client_handle );
    ComputeDLL::TermCompDLL();
  }
  didInit = false;
  
  return NS_OK;
}

/* void evaluate (in wstring expr, [retval] out wstring result); */
NS_IMETHODIMP msiSimpleComputeEngine2::Eval(const PRUnichar *expr, PRUnichar **result)
{
  return BasicCommand(expr,result,CCID_Evaluate);
}

/* void evaluateNumeric (in wstring expr, [retval] out wstring result); */
NS_IMETHODIMP msiSimpleComputeEngine2::EvalNumeric(const PRUnichar *expr, PRUnichar **result)
{
  return BasicCommand(expr,result,CCID_Evaluate_Numerically);
}

/* void equationsAsMatrix (in wstring expr, [retval] out wstring result); */
NS_IMETHODIMP msiSimpleComputeEngine2::EquationsAsMatrix(const PRUnichar *expr, const PRUnichar *vars, PRUnichar **result)
{
  return CommandWithArgs( expr,result,CCID_Rewrite_Equations_as_Matrix, PID_needvarresult, vars, PID_last );
}

/* void matrixAsEquations (in wstring expr, [retval] out wstring result); */
NS_IMETHODIMP msiSimpleComputeEngine2::MatrixAsEquations(const PRUnichar *expr, const PRUnichar *vars, PRUnichar **result)
{
  return CommandWithArgs( expr,result,CCID_Rewrite_Matrix_as_Equations, PID_needvarresult, vars, PID_last );
}

/* void solveExact (in wstring expr, [retval] out wstring result); */
NS_IMETHODIMP msiSimpleComputeEngine2::SolveExact(const PRUnichar *expr, const PRUnichar *vars, PRUnichar **result)
{
  return CommandWithArgs( expr,result,CCID_Solve_Exact, PID_needvarresult, vars, PID_last );
}

/* void solveInteger (in wstring expr, [retval] out wstring result); */
NS_IMETHODIMP msiSimpleComputeEngine2::SolveInteger(const PRUnichar* expr, const PRUnichar* vars, PRUnichar** result)
{
  return CommandWithArgs( expr, result, CCID_Solve_Integer, PID_needvarresult, vars, PID_last );
}


/* void collect (in wstring expr, in wstring vars, [retval] out wstring result); */
NS_IMETHODIMP msiSimpleComputeEngine2::Collect(const PRUnichar *expr, const PRUnichar *vars, PRUnichar **result)
{
  return CommandWithArgs( expr,result,CCID_Polynomial_Collect, PID_PolyCollectVar, vars, PID_last );
}

/* void divide (in wstring expr, in wstring var[retval] out wstring result); */
NS_IMETHODIMP msiSimpleComputeEngine2::Divide(const PRUnichar *expr, const PRUnichar *var, PRUnichar **result)
{
  return CommandWithArgs( expr,result,CCID_Polynomial_Divide, PID_needvarresult, var, PID_last );
}

/* void partialFractions (in wstring expr, in wstring var[retval] out wstring result); */
NS_IMETHODIMP msiSimpleComputeEngine2::PartialFractions(const PRUnichar *expr, const PRUnichar *var, PRUnichar **result)
{
  return CommandWithArgs( expr,result,CCID_Polynomial_Partial_Fractions, PID_needvarresult, var, PID_last );
}

/* void sort (in wstring expr, in wstring var, [retval] out wstring result); */
NS_IMETHODIMP msiSimpleComputeEngine2::Sort(const PRUnichar *expr, const PRUnichar *var, PRUnichar **result)
{ 
  return CommandWithArgs( expr, result, CCID_Polynomial_Sort, PID_needvarresult, var, PID_last );
}

/* void roots(in wstring expr, in wstring vars, [retval] out wstring result); */
NS_IMETHODIMP msiSimpleComputeEngine2::Roots(const PRUnichar *expr, const PRUnichar *var, PRUnichar **result)
{
  return CommandWithArgs( expr, result, CCID_Polynomial_Roots, PID_needvarresult, var, PID_last );
}


/* void companionMatrix (in wstring expr, in wstring var, [retval] out wstring result); */
NS_IMETHODIMP msiSimpleComputeEngine2::CompanionMatrix(const PRUnichar *expr, const PRUnichar *var, PRUnichar **result)
{
  return CommandWithArgs( expr,result,CCID_Polynomial_Companion_Matrix, PID_needvarresult, var, PID_last );
}

/* void byParts (in wstring expr, in wstring var, [retval] out wstring result); */
NS_IMETHODIMP msiSimpleComputeEngine2::ByParts(const PRUnichar *expr, const PRUnichar *var, PRUnichar **result)
{
  return CommandWithArgs( expr,result,CCID_Calculus_Integrate_by_Parts, PID_CalcByPartsVar, var, PID_last );
}

/* void changeVar (in wstring expr, in wstring var, [retval] out wstring result); */
NS_IMETHODIMP msiSimpleComputeEngine2::ChangeVar(const PRUnichar *expr, const PRUnichar *var, PRUnichar **result)
{
  return CommandWithArgs( expr,result,CCID_Calculus_Change_Variable, PID_CalcChangeVar, var, PID_last );
}

/* void approxIntegral (in wstring expr, in wstring form, in wstring numintervals, [retval] out wstring result); */
NS_IMETHODIMP msiSimpleComputeEngine2::ApproxIntegral(const PRUnichar* expr, const PRUnichar* form, const PRUnichar* numintervals, const PRUnichar* lowerBound, const PRUnichar* upperBound, PRUnichar **result)
{
  // this one is special because form is an enum
  int cmdCode = CCID_Calculus_Approximate_Integral;
  NS_PRECONDITION(expr != nsnull && form != nsnull && numintervals != nsnull && result != nsnull, "null ptr");
  if ((!expr) || (!form) || (!numintervals) || (!result))
    return NS_ERROR_ILLEGAL_VALUE;
  if (!didInit)
    return NS_ERROR_FAILURE;

  nsresult rv = NS_OK;
  U32 trans_ID  =  ComputeDLL::CreateTransaction( client_handle,expr,MuPAD_eng_ID,cmdCode );
  ComputeDLL::AddWideParam( trans_ID, PID_approxintform,  zPT_WIDE_natural,   form );
  ComputeDLL::AddWideParam( trans_ID, PID_approxintnsubs, zPT_WIDE_mmlmarkup, numintervals );
  ComputeDLL::AddWideParam( trans_ID, PID_approxintlowerbound, zPT_WIDE_mmlmarkup, lowerBound );
  ComputeDLL::AddWideParam( trans_ID, PID_approxintupperbound, zPT_WIDE_mmlmarkup, upperBound );
  
  rv =  DoTransaction( trans_ID, result );
  
  ComputeDLL::ReleaseTransaction( trans_ID );
  return rv;
}

/* void iterate (in wstring expr, in wstring startval, in wstring startval, [retval] out wstring result); */
NS_IMETHODIMP msiSimpleComputeEngine2::Iterate(const PRUnichar *expr, const PRUnichar *startval, const PRUnichar *count, PRUnichar **result)
{
  // this one is special because there is no input expression
  int cmdCode = CCID_Calculus_Iterate;
  NS_PRECONDITION(expr != nsnull && startval != nsnull && count != nsnull && result != nsnull, "null ptr");
  if ((!expr) || (!startval) || (!count) || (!result))
    return NS_ERROR_ILLEGAL_VALUE;
  if (!didInit)
    return NS_ERROR_FAILURE;

  nsresult rv = NS_OK;
  U32 trans_ID  =  ComputeDLL::CreateTransaction( client_handle,nsnull,MuPAD_eng_ID,cmdCode );
  ComputeDLL::AddWideParam( trans_ID, PID_iterfunc, zPT_WIDE_mmlmarkup, expr );
  ComputeDLL::AddWideParam( trans_ID, PID_iterstart, zPT_WIDE_mmlmarkup, startval );
  ComputeDLL::AddWideParam( trans_ID, PID_itercount, zPT_WIDE_mmlmarkup, count );
  
  rv =  DoTransaction( trans_ID, result );

  ComputeDLL::ReleaseTransaction( trans_ID );
  return rv;
}

/* void implicitDiff (in wstring expr, in wstring var, in wstring depvar, [retval] out wstring result); */
NS_IMETHODIMP msiSimpleComputeEngine2::FindExtrema(const PRUnichar *expr, const PRUnichar *var, const PRUnichar *depvar, PRUnichar **result)
{
  return CommandWithArgs( expr,result,CCID_Calculus_Find_Extrema, PID_ImplDiffIndepVar, var, PID_ImplDiffDepVars, depvar, PID_last );
}

NS_IMETHODIMP msiSimpleComputeEngine2::ImplicitDiff(const PRUnichar *expr, const PRUnichar *var, const PRUnichar *depvar, PRUnichar **result)
{
  return CommandWithArgs( expr,result,CCID_Calculus_Implicit_Differentiation, PID_ImplDiffIndepVar, var, PID_ImplDiffDepVars, depvar, PID_last );
}

/* void solveODEExact (in wstring expr, in wstring var, [retval] out wstring result); */
NS_IMETHODIMP msiSimpleComputeEngine2::SolveODEExact(const PRUnichar *expr, const PRUnichar *var, PRUnichar **result)
{
  return CommandWithArgs( expr,result,CCID_Solve_ODE_Exact, PID_independentvar, var, PID_last );
}

/* void solveODELaplace (in wstring expr, in wstring var, [retval] out wstring result); */
NS_IMETHODIMP msiSimpleComputeEngine2::SolveODELaplace(const PRUnichar *expr, const PRUnichar *var, PRUnichar **result)
{
  return CommandWithArgs(expr, result, CCID_Solve_ODE_Laplace, PID_independentvar, var, PID_last );
}

/* void solveODENumeric (in wstring expr, in wstring var, [retval] out wstring result); */
NS_IMETHODIMP msiSimpleComputeEngine2::SolveODENumeric(const PRUnichar* expr, const PRUnichar* var, PRUnichar** result)
{
  return CommandWithArgs(expr, result, CCID_Solve_ODE_Numeric, PID_independentvar, var, PID_last );
}

/* void solveODESeries (in wstring expr, in wstring var, in wstring order, [retval] out wstring result); */
NS_IMETHODIMP msiSimpleComputeEngine2::SolveODESeries(const PRUnichar *expr, const PRUnichar *var, const PRUnichar *about, const PRUnichar *order, PRUnichar **result)
{
  return CommandWithArgs( expr, result, CCID_Solve_ODE_Series,
    PID_ODEIndepVar, var,
    PID_ODEseriesabout, about,
    PID_ODEseriesorder, order,
    PID_last );
}

/* void powerSeries (in wstring expr, in wstring var, in wstring about, [retval] out wstring result); */
NS_IMETHODIMP msiSimpleComputeEngine2::PowerSeries(const PRUnichar *expr, const PRUnichar *var, const PRUnichar *about, const PRUnichar *order, PRUnichar **result)
{
  return CommandWithArgs( expr,result,CCID_Power_Series,
    PID_seriesvar, var,
    PID_seriesabout, about,
    PID_seriesorder, order,
    PID_last );
}

/* void wronskian (in wstring expr, in wstring var, [retval] out wstring result); */
NS_IMETHODIMP msiSimpleComputeEngine2::Wronskian(const PRUnichar *expr, const PRUnichar *var, PRUnichar **result)
{
  return CommandWithArgs( expr,result,CCID_Wronskian, PID_needvarresult, var, PID_last );
}

/* void characteristicPolynomial (in wstring expr, in wstring var, [retval] out wstring result); */
NS_IMETHODIMP msiSimpleComputeEngine2::CharacteristicPolynomial(const PRUnichar *expr, const PRUnichar *var, PRUnichar **result)
{
  return CommandWithArgs( expr,result,CCID_Characteristic_Polynomial, PID_needvarresult, var, PID_last );
}

/*  void matrixFill(in wstring expr, in wstring type, in wstring rows, in wstring cols, in wstring expr, [retval] out wstring result); */
NS_IMETHODIMP msiSimpleComputeEngine2::MatrixFill(const PRUnichar *type, const PRUnichar *rows, const PRUnichar *cols, const PRUnichar *expr, PRUnichar **result)
{
  // this one is special because there is no input expression and the type arg is an enum
  int cmdCode = CCID_Fill_Matrix;
  NS_PRECONDITION(type != nsnull && rows != nsnull && cols != nsnull && result != nsnull, "null ptr");
  if ((!type) || (!rows) || (!cols) || (!result))
    return NS_ERROR_ILLEGAL_VALUE;
  if (!didInit)
    return NS_ERROR_FAILURE;

  nsresult rv = NS_OK;
  U32 trans_ID  =  ComputeDLL::CreateTransaction( client_handle,nsnull,MuPAD_eng_ID,cmdCode );
  ComputeDLL::AddWideParam( trans_ID, PID_FillMatrixIlk,   zPT_WIDE_natural,   type );
  ComputeDLL::AddWideParam( trans_ID, PID_FillMatrixnRows, zPT_WIDE_mmlmarkup, rows );
  ComputeDLL::AddWideParam( trans_ID, PID_FillMatrixnCols, zPT_WIDE_mmlmarkup, cols );
  ComputeDLL::AddWideParam( trans_ID, PID_FillMatrixExtra, zPT_WIDE_mmlmarkup, expr );
  
  rv =  DoTransaction( trans_ID, result );

  ComputeDLL::ReleaseTransaction( trans_ID );
  return rv;
}

/* void map (in wstring expr, in wstring func, [retval] out wstring result); */
NS_IMETHODIMP msiSimpleComputeEngine2::Map(const PRUnichar *expr, const PRUnichar *func, PRUnichar **result)
{
  return CommandWithArgs( expr,result,CCID_Map_Entries, PID_MapsFunction, func, PID_last );
}

/* void minimalPolynomial (in wstring expr, in wstring var, [retval] out wstring result); */
NS_IMETHODIMP msiSimpleComputeEngine2::MinimalPolynomial(const PRUnichar *expr, const PRUnichar *var, PRUnichar **result)
{
  return CommandWithArgs( expr,result,CCID_Minimal_Polynomial, PID_needvarresult, var, PID_last );
}

/*  void matrixRandom(in wstring expr, in wstring type, in wstring rows, in wstring cols, in wstring llim, in wstring ulim, [retval] out wstring result); */
NS_IMETHODIMP msiSimpleComputeEngine2::MatrixRandom(const PRUnichar *type, const PRUnichar *rows, const PRUnichar *cols, const PRUnichar *llim, const PRUnichar *ulim, PRUnichar **result)
{
  // this one is special because there is no input expression and the type arg is an enum
  int cmdCode = CCID_Random_Matrix;
  NS_PRECONDITION(type != nsnull && rows != nsnull && cols != nsnull && llim != nsnull && ulim != nsnull && result != nsnull, "null ptr");
  if ((!type) || (!rows) || (!cols) || (!llim) || (!ulim) || (!result))
    return NS_ERROR_ILLEGAL_VALUE;
  if (!didInit)
    return NS_ERROR_FAILURE;

  nsresult rv = NS_OK;
  U32 trans_ID  =  ComputeDLL::CreateTransaction( client_handle,nsnull,MuPAD_eng_ID,cmdCode );
  ComputeDLL::AddWideParam( trans_ID, PID_RandMatrixType,  zPT_WIDE_natural, type );
  ComputeDLL::AddWideParam( trans_ID, PID_RandMatrixnRows, zPT_WIDE_mmlmarkup, rows );
  ComputeDLL::AddWideParam( trans_ID, PID_RandMatrixnCols, zPT_WIDE_mmlmarkup, cols );
  ComputeDLL::AddWideParam( trans_ID, PID_RandMatrixLLim,  zPT_WIDE_mmlmarkup, llim );
  ComputeDLL::AddWideParam( trans_ID, PID_RandMatrixULim,  zPT_WIDE_mmlmarkup, ulim );
  
  rv =  DoTransaction( trans_ID, result );

  ComputeDLL::ReleaseTransaction( trans_ID );
  return rv;
}

/* void matrixReshape (in wstring expr, in wstring ncols, [retval] out wstring result); */
NS_IMETHODIMP msiSimpleComputeEngine2::MatrixReshape(const PRUnichar *expr, const PRUnichar *ncols, PRUnichar **result)
{
  return CommandWithArgs( expr,result,CCID_Reshape, PID_ncolsReshape, ncols, PID_last );
}

/* void fitCurve (in wstring expr, in wstring code, in wstring column, in wstring degree, [retval] out wstring result); */
NS_IMETHODIMP msiSimpleComputeEngine2::FitCurve(const PRUnichar *expr, const PRUnichar *code, const PRUnichar *column, const PRUnichar *degree, PRUnichar **result)
{
  // this one is special because the type arg is an enum
  int cmdCode = CCID_Fit_Curve_to_Data;
  NS_PRECONDITION(code != nsnull && column != nsnull && degree != nsnull && result != nsnull, "null ptr");
  if ((!code) || (!column) || (!degree) || (!result))
    return NS_ERROR_ILLEGAL_VALUE;
  if (!didInit)
    return NS_ERROR_FAILURE;

  nsresult rv = NS_OK;
  U32 trans_ID  =  ComputeDLL::CreateTransaction( client_handle,expr,MuPAD_eng_ID,cmdCode );
  ComputeDLL::AddWideParam( trans_ID, PID_regresscode,   zPT_WIDE_natural,   code );
  ComputeDLL::AddWideParam( trans_ID, PID_dependcolumn,  zPT_WIDE_natural, column );
  ComputeDLL::AddWideParam( trans_ID, PID_regressdegree, zPT_WIDE_natural, degree );
  
  rv =  DoTransaction( trans_ID, result );

  ComputeDLL::ReleaseTransaction( trans_ID );
  return rv;
}

/* void randomNumbers (in wstring type, in wstring tally, in wstring param1, in wstring param2, [retval] out wstring result); */
NS_IMETHODIMP msiSimpleComputeEngine2::RandomNumbers(const PRUnichar *type, const PRUnichar *tally, const PRUnichar *param1, const PRUnichar *param2, PRUnichar **result)
{
  // this one is special because there is no input expression and the type arg is an enum
  int cmdCode = CCID_Random_Numbers;
  NS_PRECONDITION(type != nsnull && tally != nsnull && param1 != nsnull && param2 != nsnull && result != nsnull, "null ptr");
  if ((!type) || (!tally) || (!param1) || (!param2) || (!result))
    return NS_ERROR_ILLEGAL_VALUE;
  if (!didInit)
    return NS_ERROR_FAILURE;

  nsresult rv = NS_OK;
  U32 trans_ID  =  ComputeDLL::CreateTransaction( client_handle,nsnull,MuPAD_eng_ID,cmdCode );
  ComputeDLL::AddWideParam( trans_ID, PID_RandomNumDist,   zPT_WIDE_natural,   type );
  ComputeDLL::AddWideParam( trans_ID, PID_RandomNumTally,  zPT_WIDE_mmlmarkup, tally );
  ComputeDLL::AddWideParam( trans_ID, PID_RandomNumParam1, zPT_WIDE_mmlmarkup, param1 );
  ComputeDLL::AddWideParam( trans_ID, PID_RandomNumParam2, zPT_WIDE_mmlmarkup, param2 );
  
  rv =  DoTransaction( trans_ID, result );

  ComputeDLL::ReleaseTransaction( trans_ID );
  return rv;
}

/* void moment (in wstring expr, in wstring num, in wstring origin, in wstring mean_is_origin, [retval] out wstring result); */
NS_IMETHODIMP msiSimpleComputeEngine2::Moment(const PRUnichar *expr, const PRUnichar *num, const PRUnichar *origin, const PRUnichar *mean_is_origin, PRUnichar **result)
{
  // this one is special because the mean_is_origin arg is an enum
  int cmdCode = CCID_Moment;
  NS_PRECONDITION(expr != nsnull && num != nsnull && origin != nsnull && mean_is_origin != nsnull && result != nsnull, "null ptr");
  if ((!expr) || (!num) || (!origin) || (!mean_is_origin) || (!result))
    return NS_ERROR_ILLEGAL_VALUE;
  if (!didInit)
    return NS_ERROR_FAILURE;

  nsresult rv = NS_OK;
  U32 trans_ID  =  ComputeDLL::CreateTransaction( client_handle,expr,MuPAD_eng_ID,cmdCode );
  ComputeDLL::AddWideParam( trans_ID, PID_moment_num, zPT_WIDE_mmlmarkup, num );
  ComputeDLL::AddWideParam( trans_ID, PID_moment_origin, zPT_WIDE_mmlmarkup, origin );
  ComputeDLL::AddWideParam( trans_ID, PID_moment_meanisorigin, zPT_WIDE_natural, mean_is_origin );
  
  rv =  DoTransaction( trans_ID, result );

  ComputeDLL::ReleaseTransaction( trans_ID );
  return rv;
}

/* void quantile (in wstring expr, in wstring q, [retval] out wstring result); */
NS_IMETHODIMP msiSimpleComputeEngine2::Quantile(const PRUnichar *expr, const PRUnichar *q, PRUnichar **result)
{
  return CommandWithArgs( expr,result,CCID_Quantile, PID_quantile, q, PID_last );
}

/* void getVariables (in wstring expr, [retval] out wstring result); */
NS_IMETHODIMP msiSimpleComputeEngine2::GetVariables(const PRUnichar *expr, PRUnichar **result)
{
  return BasicCommand(expr, result, CCID_GetVariables);
}



/* void perform (in string expr, in unsigned long operation, [retval] out string result); */
NS_IMETHODIMP msiSimpleComputeEngine2::Perform(const PRUnichar *expr, PRUint32 operation, PRUnichar **result)
{   
  NS_PRECONDITION(operation >= CCID_Evaluate && operation, "operation out of range");
  if (operation >= CCID_Evaluate && operation < CCID_Last)
    return BasicCommand(expr,result,operation);
  else
    return NS_ERROR_ILLEGAL_VALUE;
}

/* void define (in wstring expr, in wstring subinterp, [retval] out wstring result); */
NS_IMETHODIMP msiSimpleComputeEngine2::Define(const PRUnichar *expr, const PRUnichar *subinterp, PRUnichar **result)
{
  int cmdCode = CCID_Define;
  NS_PRECONDITION(expr != nsnull && result != nsnull, "null ptr");
  if (!expr)
    return NS_ERROR_ILLEGAL_VALUE;
  if (!didInit)
    return NS_ERROR_FAILURE;

  nsresult rv = NS_OK;
  U32 trans_ID  =  ComputeDLL::CreateTransaction( client_handle,expr,MuPAD_eng_ID,cmdCode );
  if (subinterp && wstrlen(subinterp)>0 ) 
    ComputeDLL::AddWideParam( trans_ID, PID_subInterpretation, zPT_WIDE_natural, subinterp );
  
  rv =  DoTransaction( trans_ID, result );

  ComputeDLL::ReleaseTransaction( trans_ID );
  return rv;
}

/* void undefine (in wstring expr, [retval] out wstring result); */
NS_IMETHODIMP msiSimpleComputeEngine2::Undefine(const PRUnichar *expr, PRUnichar **result)
{
  return BasicCommand(expr,result,CCID_Undefine);
}

/* void defineMupadName(in wstring swpname, in wstring mupname, in wstring loc, [retval] out wstring result); */
NS_IMETHODIMP msiSimpleComputeEngine2::DefineMupadName(const PRUnichar* swpname, const PRUnichar* mupname, const PRUnichar* loc, PRUnichar** result)
{
  return NS_ERROR_FAILURE;
}

/* void getDefinitions ([retval] out wstring result); */
NS_IMETHODIMP msiSimpleComputeEngine2::GetDefinitions(PRUnichar **result)
{
  NS_PRECONDITION(result != nsnull, "null ptr");
  if (!result)
    return NS_ERROR_ILLEGAL_VALUE;
  if (!didInit)
    return NS_ERROR_FAILURE;

  NS_NAMED_LITERAL_STRING(openp,"<p>");
  const PRSize openp_len = openp.Length();
  NS_NAMED_LITERAL_STRING(closep,"</p>");
  const PRSize closep_len = closep.Length();

  const DefInfo* curr_def =  NULL;
  curr_def = (DefInfo*)ComputeDLL::GetNextDef( client_handle,MuPAD_eng_ID,curr_def );
  if (!curr_def) {
    *result = (PRUnichar*) nsMemory::Clone(L"", sizeof(PRUnichar));
    return NS_OK;
  }
  // loop through defs to get size
  PRSize total_len = 1; // for trailing NULL
  while (curr_def) {
    total_len += openp_len + wstrlen(curr_def->WIDE_src) + closep_len;
    curr_def = (DefInfo*)ComputeDLL::GetNextDef( client_handle,MuPAD_eng_ID,curr_def );
  }

  *result = (PRUnichar*) nsMemory::Alloc(total_len*sizeof(PRUnichar));
  if (!*result) {
    return NS_ERROR_FAILURE;
  }
  
  curr_def = NULL;
  curr_def = (DefInfo*)ComputeDLL::GetNextDef( client_handle,MuPAD_eng_ID,curr_def );

  // now, loop through defs to get the data
  PRUnichar *rover = *result;
  while (curr_def) {
    memcpy(rover, openp.get(), openp_len*sizeof(PRUnichar));
    rover += openp_len;
    int zln = wstrlen(curr_def->WIDE_src);
    memcpy(rover, curr_def->WIDE_src, zln*sizeof(PRUnichar));
    rover += zln;
    memcpy(rover, closep.get(), closep_len*sizeof(PRUnichar));
    rover += closep_len;
    curr_def = (DefInfo*)ComputeDLL::GetNextDef( client_handle,MuPAD_eng_ID,curr_def );
  }
  *rover = 0;
  return NS_OK;
}

/* void clearDefinitions (); */
NS_IMETHODIMP msiSimpleComputeEngine2::ClearDefinitions()
{
  ComputeDLL::ClearDefs(client_handle,MuPAD_eng_ID);
  return NS_OK;
}

/*  void plotfuncCmd(in wstring cmd, [retval] out wstring result); */
/*  cmd is XML, not MathML.    */
NS_IMETHODIMP msiSimpleComputeEngine2::PlotfuncCmd(const PRUnichar *graph, PRUnichar **result)
{
  int cmdCode = CCID_PlotFuncCmd;
  nsresult rv = NS_OK;
  U32 trans_ID = ComputeDLL::CreateTransaction (client_handle, graph, MuPAD_eng_ID, cmdCode);
  rv = DoTransaction (trans_ID, result);
  ComputeDLL::ReleaseTransaction (trans_ID);
  return rv;
}

/*  void plotfuncCmd(in wstring cmd, [retval] out wstring result); */
/*  cmd is XML, not MathML.    */
NS_IMETHODIMP msiSimpleComputeEngine2::PlotfuncQuery(const PRUnichar *graph, PRUnichar **result)
{
  int cmdCode = CCID_PlotFuncQuery;
  nsresult rv = NS_OK;
  U32 trans_ID = ComputeDLL::CreateTransaction (client_handle, graph, MuPAD_eng_ID, cmdCode);
  rv = DoTransaction (trans_ID, result);
  ComputeDLL::ReleaseTransaction (trans_ID);
  return rv;
}

/* void setEngineAttr (in unsigned long attrID, in long value); */
NS_IMETHODIMP msiSimpleComputeEngine2::SetEngineAttr(PRUint32 attrID, PRInt32 value)
{
  char buf[100];
  switch (attrID) {
  case EID_SeriesOrder: 
  case EID_Digits:      
  case EID_MaxDegree:   
    sprintf( buf, "%d", value );
    break;
  case EID_PvalOnly:    
  case EID_IgnoreSCases:
    sprintf( buf, "%s", value ? "TRUE" : "FALSE" );  // note unfortunate use of engine syntax
    break;
  default:
    return NS_ERROR_ILLEGAL_VALUE;
  }
  ComputeDLL::SetEngineStateAttr( MuPAD_eng_ID, attrID, buf); // check return?
  return NS_OK;
}

/* long getEngineAttr (in unsigned long attrID); */
NS_IMETHODIMP msiSimpleComputeEngine2::GetEngineAttr(PRUint32 attrID, PRInt32 *_retval)
{
  switch (attrID) {
  case EID_SeriesOrder: 
  case EID_Digits:      
  case EID_MaxDegree:   
  case EID_PvalOnly:    
  case EID_IgnoreSCases:
    break;
  default:
    return NS_ERROR_ILLEGAL_VALUE;
  }
  const char* str_value = ComputeDLL::GetEngineStateAttr( MuPAD_eng_ID, attrID);
  if (!str_value)
    return NS_ERROR_FAILURE;

  switch (attrID) {
  case EID_SeriesOrder: 
  case EID_Digits:      
  case EID_MaxDegree:   
    if (sscanf( str_value, "%d", _retval ) < 1)
      return NS_ERROR_FAILURE;
    break;
  case EID_PvalOnly:    
  case EID_IgnoreSCases:
    if (strcmp(str_value,"TRUE") == 0)
      *_retval = 1;
    else
      *_retval = 0;
    break;
  }
  return NS_OK;
}

/* void getVectorBasis ([retval] out wstring result); */
NS_IMETHODIMP msiSimpleComputeEngine2::GetVectorBasis(PRUnichar **result)
{
  const U16* res_mml = ComputeDLL::GetWideUserPref( 0/*client_handle*/, CLPF_Vector_basis, 0 /* no_inherit */);
  if (res_mml)
    *result = (PRUnichar*) nsMemory::Clone(res_mml, (wstrlen(res_mml)+1)*sizeof(PRUnichar));
  else
    *result = nsnull;
  return NS_OK;
}

/* void setVectorBasis (in wstring result); */
NS_IMETHODIMP msiSimpleComputeEngine2::SetVectorBasis(const PRUnichar *expr)
{
  ComputeDLL::SetWideUserPref( 0/*client_handle*/, CLPF_Vector_basis, expr);  // check return?
  // check res
  return NS_OK;
}

/* void getEngineSent ([retval] out wstring result); */
NS_IMETHODIMP msiSimpleComputeEngine2::GetEngineSent(PRUnichar **result)
{
  if (sent_to_engine)
    *result = (PRUnichar*) nsMemory::Clone(sent_to_engine, (wstrlen(sent_to_engine)+1)*sizeof(PRUnichar));
  else
    *result = nsnull;
  return NS_OK;
}

/* void getEngineReceived ([retval] out wstring result); */
NS_IMETHODIMP msiSimpleComputeEngine2::GetEngineReceived(PRUnichar **result)
{
  if (received_from_engine)
    *result = (PRUnichar*) nsMemory::Clone(received_from_engine, (wstrlen(received_from_engine)+1)*sizeof(PRUnichar));
  else
    *result = nsnull;
  return NS_OK;
}

/* void getEngineErrors ([retval] out wstring result); */
NS_IMETHODIMP msiSimpleComputeEngine2::GetEngineErrors(PRUnichar **result)
{
  if (engine_errors)
    *result = (PRUnichar*) nsMemory::Clone(engine_errors, (wstrlen(engine_errors)+1)*sizeof(PRUnichar));
  else
    *result = nsnull;
  return NS_OK;
}

static
void SetupNodeAttrs()
{
  int res;
  res = ComputeDLL::SetUserPref( 0/*client_handle*/, CLPF_clr_func_attr, "msimathname=\"true\"");
  res = ComputeDLL::SetUserPref( 0/*client_handle*/, CLPF_clr_text_attr, "");
  res = ComputeDLL::SetUserPref( 0/*client_handle*/, CLPF_clr_unit_attr, "msiunit=\"true\"");
}

/* void setUserPref (in unsigned long attrID, in long value); */
NS_IMETHODIMP msiSimpleComputeEngine2::SetUserPref(PRUint32 attrID, PRInt32 value)
{
  char buf[100];
  switch (attrID) {
  case CLPF_use_mfenced:         
  case CLPF_Sig_digits_rendered:
  case CLPF_SciNote_lower_thresh: 
  case CLPF_SciNote_upper_thresh: 
  case CLPF_Primes_as_n_thresh:  
  case CLPF_Prime_derivative:  
  case CLPF_Parens_on_trigargs: 
  case CLPF_log_is_base_e:      
  case CLPF_Dot_derivative:     
  case CLPF_Overbar_conjugate:
  case CLPF_Default_matrix_delims:  
  case CLPF_Output_InvTrigFuncs_1:
  case CLPF_Output_Mixed_Numbers:
  case CLPF_Default_derivative_format:
  case CLPF_Input_i_Imaginary:
  case CLPF_Input_j_Imaginary:
  case CLPF_Input_e_Euler:
  case CLPF_D_derivative:     
    sprintf( buf, "%d", value );
    break;
  case CLPF_Output_imaginaryi:
    if (value == 0)
      strcpy(buf,"i");
    else if (value == 1)
      strcpy(buf,"j");
    else
      strcpy(buf,"&#x2148;");
    break;
  case CLPF_Output_differentialD:
    if (value == 0)
      strcpy(buf,"D");
    else
      strcpy(buf,"&#x2145;");
    break;
  case CLPF_Output_differentiald:
    if (value == 0)
      strcpy(buf,"d");
    else
      strcpy(buf,"&#x2146;");
    break;
  case CLPF_Output_Euler_e:
    if (value == 0)
      strcpy(buf,"e");
    else
      strcpy(buf,"&#x2147;");
    break;
       
  case CLPF_mml_version:         
  case CLPF_mml_prefix:          
  case CLPF_math_node_attrs:     
  case CLPF_clr_math_attr:       
  case CLPF_clr_func_attr:       
  case CLPF_clr_text_attr:       
  case CLPF_clr_unit_attr:       
  default:
    return NS_ERROR_ILLEGAL_VALUE;
  }

  ComputeDLL::SetUserPref( 0/*client_handle*/, attrID, buf);  // check return?
  return NS_OK;
}

/* long getUserPref (in unsigned long attrID); */
NS_IMETHODIMP msiSimpleComputeEngine2::GetUserPref(PRUint32 attrID, PRInt32 *_retval)
{
  switch (attrID) {
  case CLPF_use_mfenced:
  case CLPF_Sig_digits_rendered:
  case CLPF_SciNote_lower_thresh: 
  case CLPF_SciNote_upper_thresh: 
  case CLPF_Primes_as_n_thresh:
  case CLPF_Prime_derivative:  
  case CLPF_Parens_on_trigargs: 
  case CLPF_log_is_base_e:      
  case CLPF_Dot_derivative:     
  case CLPF_Overbar_conjugate:  
  case CLPF_Output_differentialD:
  case CLPF_Output_differentiald:
  case CLPF_Output_Euler_e:
  case CLPF_Output_imaginaryi:
  case CLPF_Default_matrix_delims:
  case CLPF_Output_InvTrigFuncs_1:     
  case CLPF_Output_Mixed_Numbers:
  case CLPF_Default_derivative_format:
  case CLPF_Input_i_Imaginary:
  case CLPF_Input_j_Imaginary:
  case CLPF_Input_e_Euler:
  case CLPF_D_derivative:     
    break;
  default:
    return NS_ERROR_ILLEGAL_VALUE;
  }
  const char* str_value = ComputeDLL::GetUserPref( 0/*client_handle*/, attrID, 0 /* no_inherit */);
  if (!str_value)
    return NS_ERROR_FAILURE;

  switch (attrID) {
  case CLPF_use_mfenced:
  case CLPF_Sig_digits_rendered:
  case CLPF_SciNote_lower_thresh: 
  case CLPF_SciNote_upper_thresh: 
  case CLPF_Primes_as_n_thresh:  
  case CLPF_Prime_derivative:  
  case CLPF_Parens_on_trigargs: 
  case CLPF_log_is_base_e:      
  case CLPF_Dot_derivative:     
  case CLPF_Overbar_conjugate:
  case CLPF_Default_matrix_delims:
  case CLPF_Output_InvTrigFuncs_1:  
  case CLPF_Output_Mixed_Numbers:
  case CLPF_Default_derivative_format:
  case CLPF_Input_i_Imaginary:
  case CLPF_Input_j_Imaginary:
  case CLPF_Input_e_Euler:
  case CLPF_D_derivative:     
    if (sscanf( str_value, "%d", _retval ) < 1)
      return NS_ERROR_FAILURE;
    else
      return NS_OK;
    break;

  case CLPF_Output_imaginaryi:
    if (strcmp(str_value,"i") == 0) {
      *_retval = 0;
      return NS_OK;
    } else if (strcmp(str_value,"j") == 0) {
      *_retval = 1;
      return NS_OK;
    } else {
      *_retval = 2;
      return NS_OK;
    }
    break;   
  case CLPF_Output_differentialD:
    if (strcmp(str_value,"D") == 0) {
      *_retval = 0;
      return NS_OK;
    } else {
      *_retval = 1;
      return NS_OK;
    }
    break;   
  case CLPF_Output_differentiald:
    if (strcmp(str_value,"d") == 0) {
      *_retval = 0;
      return NS_OK;
    } else {
      *_retval = 1;
      return NS_OK;
    }
    break;   
  case CLPF_Output_Euler_e:
    if (strcmp(str_value,"e") == 0) {
      *_retval = 0;
      return NS_OK;
    } else {
      *_retval = 1;
      return NS_OK;
    }
    break;   

  default:
    return NS_ERROR_ILLEGAL_VALUE;
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private helpers
NS_IMETHODIMP msiSimpleComputeEngine2::BasicCommand(const PRUnichar *expr, PRUnichar **result, int cmdCode)
{
  NS_PRECONDITION(expr != nsnull && result != nsnull, "null ptr");
  if ((!expr) || (!result))
    return NS_ERROR_ILLEGAL_VALUE;
  if (!didInit)
    return NS_ERROR_FAILURE;

  nsresult rv = NS_OK;
  if (expr) {
    U32 trans_ID  =  ComputeDLL::CreateTransaction( client_handle,expr,MuPAD_eng_ID,cmdCode );
    rv =  DoTransaction( trans_ID, result );
    ComputeDLL::ReleaseTransaction( trans_ID );
  } else {
    rv = NS_ERROR_FAILURE;
  }
  return rv;
}

// The variable arguments come in pairs <PARAM_ID,PARAM>
// until a final PID_LAST.  Unfortunately, C++ makes it hard to validate this format.
// The natural thing would be to use an STL object, but we don't do that in Mozilla.
//
NS_IMETHODIMP msiSimpleComputeEngine2::CommandWithArgs(const PRUnichar *expr, PRUnichar **result, int cmdCode, ...)
{
  NS_PRECONDITION(expr != nsnull && result != nsnull, "null ptr");
  if (!expr || !result)
    return NS_ERROR_ILLEGAL_VALUE;
  if (!didInit)
    return NS_ERROR_FAILURE;

  nsresult rv = NS_OK;
  if (expr) {
    U32 trans_ID  =  ComputeDLL::CreateTransaction( client_handle,expr,MuPAD_eng_ID,cmdCode );

	  // put args on transaction
	  va_list cArgs;
	  va_start(cArgs, cmdCode);
    while (PR_TRUE) {
      int paramID = va_arg(cArgs,int);
      if (paramID == PID_last) {
        break;
      } else {
        const PRUnichar *arg = va_arg(cArgs,PRUnichar*);
        if (arg && wstrlen(arg) > 0) 
          ComputeDLL::AddWideParam( trans_ID, paramID, zPT_WIDE_mmlmarkup, arg );
      }
    }
	  va_end(cArgs);

    if (rv == NS_OK) {
      rv =  DoTransaction( trans_ID, result );
    }
    ComputeDLL::ReleaseTransaction( trans_ID );
  } else {
    rv = NS_ERROR_FAILURE;
  }
  return rv;
}

nsresult   msiSimpleComputeEngine2::DoTransaction(PRUint32 trans_ID, PRUnichar **result)
{
  nsresult rv = NS_OK;
  int res =  ComputeDLL::ExecuteTransaction( trans_ID );

  if (res > PID_first_badparam && res < PID_first_badparam + PID_last) {
#ifdef DEBUG
    printf("msiSimpleComputeEngine::DoTransaction() missing arg: %d\n", res);
#endif
    rv = NS_ERROR_INVALID_ARG;
  } else if (res >= CR_success) {
    rv = NS_OK;
  } else {
#ifdef DEBUG
    printf("msiSimpleComputeEngine::DoTransaction() some other error: %d\n", res);
#endif
    if (res == 1) {
      rv = NS_OK; // why is this necessary?
    }
    else
      rv = NS_ERROR_GENERATE_FAILURE(NS_ERROR_MODULE_GENERAL, res);
  }
  *result = GetResultStrs(trans_ID);

  return rv;
}

// note interesting side effects.  GetEngineSent/GetEngineReceived return other values
PRUnichar *msiSimpleComputeEngine2::GetResultStrs(PRUint32 trans_ID)
{
  const U16* res_mml =  ComputeDLL::GetPtrWIDEresult( trans_ID, RES_RESULT );
  PRUnichar *res;
  if (res_mml)
    res = (PRUnichar*) nsMemory::Clone(res_mml, (wstrlen(res_mml)+1)*sizeof(PRUnichar));
  else
    res = nsnull;

  nsMemory::Free(sent_to_engine);
  const U16* sent =  ComputeDLL::GetPtrWIDEresult( trans_ID, RES_SENT );
  if (sent)
    sent_to_engine = (PRUnichar*) nsMemory::Clone(sent, (wstrlen(sent)+1)*sizeof(PRUnichar));
  else
    sent_to_engine = nsnull;

  nsMemory::Free(received_from_engine);
  const U16* received =  ComputeDLL::GetPtrWIDEresult( trans_ID, RES_RECEIVED );
  if (received)
    received_from_engine = (PRUnichar*) nsMemory::Clone(received, (wstrlen(received)+1)*sizeof(PRUnichar));
  else
    received_from_engine = nsnull;

  nsMemory::Free(engine_errors);
  const U16* errors =  ComputeDLL::GetPtrWIDEresult( trans_ID, RES_ERROR );
  if (errors)
    engine_errors = (PRUnichar*) nsMemory::Clone(errors, (wstrlen(errors)+1)*sizeof(PRUnichar));
  else
    engine_errors = nsnull;

  return res;
}


//NS_GENERIC_FACTORY_SINGLETON_CONSTRUCTOR(msiSimpleComputeEngine2, msiSimpleComputeEngine2::GetInstance)
NS_DECL_CLASSINFO(msiSimpleComputeEngine2)



