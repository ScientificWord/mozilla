#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>

#include "nsXPCOM.h"
#include "nsCOMPtr.h"
#include "xpcom-config.h"
#include "nsStringAPI.h"
#include "nsILocalFile.h"
#include "nsEmbedString.h"
#include "msiISimpleComputeEngine.h"
#include "nsIServiceManager.h"
#include "nsIComponentManager.h"
#include "nsComponentManagerUtils.h"
#include "nsIComponentRegistrar.h"


#include "CmpTypes.h"
#include "../Engines/fltutils.h"

#ifdef XP_WIN
#define tcistricmp stricmp
#else
#define tcistricmp strcasecmp
#endif

#define MSI_SIMPLECOMPUTEENGINE_CONTRACTID "@mackichan.com/simplecomputeengine;2"

// vestigal
U32 maple_eng_ID;
U32 MuPAD_eng_ID;

class CompEngine {
public:
  bool Init(nsCOMPtr<nsIComponentManager> aManager);
  bool SwitchEngine(U32 engine_ID, nsILocalFile * baseDir );
  bool Shutdown();

  void SetClientPref(U32 pref_ID, long val);
  void SetVectorBasis(const nsACString & src_markup);
  void SetEngineAttr(U32 attr_ID, long val);
  int DoComputation(FILE * outfile,
                  U32 client_ID, U32 engine_ID,
                  unsigned short cmd_ID, const nsACString & src_markup,
                  PARAM_REC * param_list);

private:
  nsCOMPtr<msiISimpleComputeEngine> mysample;
  nsString lastresult;
};

int ProcessTestScript(CompEngine &aEngine, const char *aFile);


int main(int argc, char** argv)
{
    nsresult rv;
    bool showUsage = false;

    const char *fname;
    if (argc == 2) {
      fname = argv[1];
    } else if (argc == 3) {
      if (strcmp(argv[1],"-clearlog") != 0) {
        showUsage = true;
      } else {
        JBM::ClearLog();
        printf("JBM::ClearLog() called.\n");
        fname = argv[2];
      }
    } else {
      showUsage = true;
    }
    if (showUsage) {
      printf("Usage: %s [-clearlog] testfile.tscript\n", argv[0]);
      return -1;
    }

#ifdef XPCOM_GLUE
    XPCOMGlueStartup(nsnull);
#endif

    nsCOMPtr<nsIServiceManager> servMan;
    rv = NS_InitXPCOM2(getter_AddRefs(servMan), nsnull, nsnull);
    if (NS_FAILED(rv)) {
      printf("ERROR: XPCOM initialization error [%x].\n", rv);
      return -1;
    }

    nsCOMPtr<nsIComponentRegistrar> registrar = do_QueryInterface(servMan);
    NS_ASSERTION(registrar, "Null nsIComponentRegistrar");
    registrar->AutoRegister(nsnull);
    
    nsCOMPtr<nsIComponentManager> manager = do_QueryInterface(registrar);
    NS_ASSERTION(manager, "Null nsIComponentManager");
    
    CompEngine myengine;
    if (!myengine.Init(manager))
      return -2;
    
    if (ProcessTestScript(myengine,fname) != 0) {
      printf("%s ERROR: ProcessTestScript failure\n", argv[0]);
      return -4;
    } else {
      printf("%s Test script ran to completion.\n", argv[0]);
    }
    
    if (!myengine.Shutdown())
      return -5;

    servMan = 0;
    registrar = 0;
    manager = 0;
    
    NS_ShutdownXPCOM(nsnull);

#ifdef XPCOM_GLUE
    XPCOMGlueShutdown();
#endif
    return 0;
}

bool CompEngine::Init(nsCOMPtr<nsIComponentManager> aManager)
{
  nsresult rv = aManager->CreateInstanceByContractID(MSI_SIMPLECOMPUTEENGINE_CONTRACTID,
                                                    nsnull,
                                                    NS_GET_IID(msiISimpleComputeEngine),
                                                    getter_AddRefs(mysample));
  if (NS_FAILED(rv)) {
    printf("ERROR: Cannot create instance of component " MSI_SIMPLECOMPUTEENGINE_CONTRACTID " [%x].\n"
           "Debugging hint:\n"
           "\tsetenv NSPR_LOG_MODULES nsComponentManager:5\n"
           "\tsetenv NSPR_LOG_FILE xpcom.log\n"
           "\t./Testjig2\n"
           "\t<check the contents for xpcom.log for possible cause of error>.\n",
           rv);
    return false;
  }

  nsCOMPtr<nsIFile> bd;
  nsCOMPtr<nsILocalFile> baseDir;
  PRBool fPersistent;
  rv = DSP->GetFile("GreD", &fPersistent, getter_AddRefs(bd));
  if (rv) return false;
  baseDir = do_QueryInterface(bd);
  nsAutoString temp;
  baseDir->Append(NS_LITERAL_STRING("mupInstall.gmr"));
  
  rv = mysample->Startup(baseDir);
  if (NS_FAILED(rv)) {
    printf("ERROR: Calling msiISimpleComputeEngine::Startup() [%x]\n", rv);
    return false;
  } else {
#ifdef DEBUG_CompEngine
    printf(" Startup OK\n");
#endif
  }
  return true;
}

bool CompEngine::SwitchEngine(U32 engine_ID, nsILocalFile * baseDir )
{
  NS_ASSERTION(engine_ID == 1 || engine_ID == 2, "engine ID out of range");
  nsresult rv = mysample->Shutdown();
  if (NS_FAILED(rv)) {
    printf("ERROR: Calling msiISimpleComputeEngine::Shutdown() in SwitchEngine() [%x]\n", rv);
    return false;
  } else {
#ifdef DEBUG_CompEngine
    printf(" Shutdown OK in SwitchEngine()\n");
#endif
  }
  nsAutoString temp;
  if (engine_ID == 2)
    baseDir->Append(NS_LITERAL_STRING("mupInstall.gmr"));
  else
    baseDir->Append(NS_LITERAL_STRING("mplInstall.gmr"));
    
//  baseDir->GetPath(temp);

  rv = mysample->Startup(baseDir);
  if (NS_FAILED(rv)) {
    printf("ERROR: Calling msiISimpleComputeEngine::Startup() during SwitchEngine() [%x]\n", rv);
    return false;
  } else {
#ifdef DEBUG_CompEngine
    printf(" Startup OK in SwitchEngine()\n");
#endif
  }
  return true;
}

bool CompEngine::Shutdown()
{
  nsresult rv = mysample->Shutdown();
  if (NS_FAILED(rv)) {
    printf("ERROR: Calling msiISimpleComputeEngine::Shutdown() [%x]\n", rv);
    return false;
  } else {
#ifdef DEBUG_CompEngine
    printf(" Shutdown OK\n");
#endif
  }
  mysample = 0;
  return true;
}


// I'm embedding the MathML generated from engine output in an xhtml file.
// The file can be opened by Netscape, thus providing a rendering of the output.

char *xhtml_header[] = {
  "<?xml version=\"1.0\"?>\n",
  "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1 plus MathML 2.0//EN\"\n",
  "\"http://www.w3.org/Math/DTD/mathml2/xhtml-math11-f.dtd\">\n",
  "<html xmlns=\"http://www.w3.org/1999/xhtml\"\n",
  "xmlns:mml=\"http://www.w3.org/1998/Math/MathML\">\n",
  "<head>\n",
  "<title>CompTest Script/Command Output</title>\n",
  "</head>\n",
  "<body>\n"
};

char *xhtml_footer[] = {
  "</body>\n",
  "</html>\n"
};

void WriteXHTMLHeader(FILE * fp)
{
  for (int i = 0; i < sizeof(xhtml_header) / sizeof(xhtml_header[0]); i++)
    fputs(xhtml_header[i], fp);
}

void WriteXHTMLFooter(FILE * fp)
{
  for (int i = 0; i < sizeof(xhtml_footer) / sizeof(xhtml_footer[0]); i++)
    fputs(xhtml_footer[i], fp);
}

void WriteTime(const char *msg, FILE * fp)
{
  time_t aclock;
  time(&aclock);
  char *p = asctime(localtime(&aclock));
  *(p + 24) = 0;                //HACK: remove \n
  fputs("<hr/>\n<p><b>", fp);
  fputs(msg, fp);
  fputs(":</b> ", fp);
  fputs(p, fp);
  fputs("</p>\n", fp);
}

const char *
GetParamVal(PARAM_REC * curr_list, U32 p_ID)
{
  for (PARAM_REC * p = curr_list; p; p = p->next)
    if (p->param_ID == p_ID) {
      NS_ASSERTION(p->param_type == zPT_ASCII_mmlmarkup, "Expecting MathML markup in ASCII.");
      return p->ztext;
    }
  return "";
}

const char *
GetNaturalParamVal(PARAM_REC * curr_list, U32 p_ID)
{
  for (PARAM_REC * p = curr_list; p; p = p->next)
    if (p->param_ID == p_ID) {
      NS_ASSERTION(p->param_type == zPT_ASCII_natural, "Expecting natural markup in ASCII.");
      return p->ztext;
    }
  return "";
}

void WideToASCII(const U16 * w_markup, nsACString& out)
{
  for (const U16 *p = w_markup; *p; p++) {
    char ascii[16];

    U16 wchar = *p;
    if (wchar < 0x80) {
      ascii[0] = static_cast<char>(wchar);
      ascii[1] = 0;
    } else {
      sprintf(ascii, "&#x%x;", wchar);
    }
    out += ascii;
  }
}

void CompEngine::SetClientPref(U32 pref_ID, long pref_val)
{
  nsresult res = mysample->SetUserPref(pref_ID,pref_val);
}

void CompEngine::SetVectorBasis(const nsACString & src_markup)
{
  mysample->SetVectorBasis(NS_ConvertUTF8toUTF16(src_markup).get());
}

void CompEngine::SetEngineAttr(U32 attr_ID, long val)
{
  mysample->SetEngineAttr(attr_ID,val);
}

int CompEngine::DoComputation(FILE * outfile,
                  U32 client_ID, U32 engine_ID,
                  unsigned short cmd_ID, const nsACString & src_markup,
                  PARAM_REC * param_list)
{
  NS_ASSERTION(client_ID == 1, "don't know how to switch clients");

  nsAutoString wide_markup;
  if (src_markup.Equals(NS_LITERAL_CSTRING("  <math>%</math>")))  // magic token
    wide_markup = lastresult;
  else
    wide_markup = NS_ConvertUTF8toUTF16(src_markup);
  nsAutoString arg, arg2, arg3, arg4, arg5;
  PRUnichar* res_str;
  int res = 0;
  nsresult rv = NS_OK;
  switch (cmd_ID) {
    case CCID_Evaluate:
      rv = mysample->Eval(wide_markup.get(),&res_str);
      break;
    case CCID_Evaluate_Numerically:
      rv = mysample->EvalNumeric(wide_markup.get(),&res_str);
      break;
    case CCID_Simplify:
    case CCID_Expand:
    case CCID_Combine_Exponentials:
    case CCID_Combine_Logs:
    case CCID_Combine_Powers:
    case CCID_Combine_Trig_Functions:
    case CCID_Combine_Arctan:
    case CCID_Combine_Hyp_Trig_Functions:
    case CCID_Factor:
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
    case CCID_Check_Equality:
    case CCID_Solve_Integer:
    case CCID_Solve_Numeric:
    case CCID_Solve_Recursion:
    case CCID_Polynomial_Roots:
    case CCID_Calculus_Partial_Fractions:
    case CCID_Solve_ODE_Numeric:
	case CCID_Fourier_Transform:
	case CCID_Inverse_Fourier_Transform:
	case CCID_Laplace_Transform:
	case CCID_Inverse_Laplace_Transform:
	case CCID_Gradient:
	case CCID_Divergence:
	case CCID_Curl:
	case CCID_Laplacian:
	case CCID_Jacobian:
    case CCID_Hessian:
    case CCID_Wronskian:
    case CCID_Scalar_Potential:
    case CCID_Vector_Potential:
    case CCID_Adjugate:
    case CCID_Cholesky_Decomposition:
    case CCID_Column_Basis:
    case CCID_Concatenate:
    case CCID_Condition_Number:
    case CCID_Definiteness_Tests:
    case CCID_Determinant:
    case CCID_Eigenvalues:
    case CCID_Eigenvectors:
    case CCID_Fraction_Free_Gaussian_Elimination:
    case CCID_Gaussian_Elimination:
    case CCID_Hermite_Normal_Form:
    case CCID_Hermitian_Transpose:
    case CCID_Inverse:
    case CCID_Jordan_Form:
    case CCID_Norm:
    case CCID_Nullspace_Basis:
    case CCID_Orthogonality_Test:
    case CCID_Permanent:
    case CCID_PLU_Decomposition:
    case CCID_QR_Decomposition:
    case CCID_Rank:
    case CCID_Rational_Canonical_Form:
    case CCID_Reduced_Row_Echelon_Form:
    case CCID_Row_Basis:
    case CCID_Singular_Values:
    case CCID_Smith_Normal_Form:
    case CCID_Spectral_Radius:
    case CCID_Stack:
    case CCID_SVD:
    case CCID_Trace:
    case CCID_Simplex_Dual:
    case CCID_Simplex_Feasible:
    case CCID_Simplex_Maximize:
    case CCID_Simplex_Minimize:
    case CCID_Simplex_Standardize:
    case CCID_Mean:
    case CCID_Median:
    case CCID_Mode:
    case CCID_Correlation:
    case CCID_Covariance:
    case CCID_Geometric_Mean:
    case CCID_Harmonic_Mean:
    case CCID_Mean_Deviation:
    case CCID_Standard_Deviation:
    case CCID_Variance:
    case CCID_Undefine:
    case CCID_Cleanup:
    case CCID_Fixup:
    case CCID_Interpret:
    case CCID_PassThru:
    case CCID_PlotFuncCmd:
      rv = mysample->Perform(wide_markup.get(),cmd_ID,&res_str);
      break;
    case CCID_Rewrite_Equations_as_Matrix:
      arg = NS_ConvertUTF8toUTF16(GetParamVal(param_list,PID_needvarresult));
      rv = mysample->EquationsAsMatrix(wide_markup.get(),arg.get(),&res_str);
      break;
    case CCID_Rewrite_Matrix_as_Equations:
      arg = NS_ConvertUTF8toUTF16(GetParamVal(param_list,PID_needvarresult));
      rv = mysample->MatrixAsEquations(wide_markup.get(),arg.get(),&res_str);
      break;
    case CCID_Solve_Exact:
      arg = NS_ConvertUTF8toUTF16(GetParamVal(param_list,PID_needvarresult));
      rv = mysample->SolveExact(wide_markup.get(),arg.get(),&res_str);
      break;
    case CCID_Power_Series:
      arg = NS_ConvertUTF8toUTF16(GetParamVal(param_list,PID_seriesvar));
      arg2 = NS_ConvertUTF8toUTF16(GetParamVal(param_list,PID_seriesabout));
      arg3 = NS_ConvertUTF8toUTF16(GetParamVal(param_list,PID_seriesorder));
      rv = mysample->PowerSeries(wide_markup.get(),arg.get(),arg2.get(),arg3.get(),&res_str);
      break;
    case CCID_Solve_ODE_Exact:
      arg = NS_ConvertUTF8toUTF16(GetParamVal(param_list,PID_independentvar));
      rv = mysample->SolveODEExact(wide_markup.get(),arg.get(),&res_str);
      break;
    case CCID_Solve_ODE_Laplace:
      arg = NS_ConvertUTF8toUTF16(GetParamVal(param_list,PID_independentvar));
      rv = mysample->SolveODELaplace(wide_markup.get(),arg.get(),&res_str);
      break;
    case CCID_Solve_ODE_Series:
      arg = NS_ConvertUTF8toUTF16(GetParamVal(param_list,PID_ODEIndepVar));
      arg2 = NS_ConvertUTF8toUTF16(GetParamVal(param_list,PID_ODEseriesabout));
      arg3 = NS_ConvertUTF8toUTF16(GetParamVal(param_list,PID_ODEseriesorder));
      rv = mysample->SolveODESeries(wide_markup.get(),arg.get(),arg2.get(),arg3.get(),&res_str);
      break;
    case CCID_Polynomial_Collect:
      arg = NS_ConvertUTF8toUTF16(GetParamVal(param_list,PID_PolyCollectVar));
      rv = mysample->Collect(wide_markup.get(),arg.get(),&res_str);
      break;
    case CCID_Polynomial_Divide:
      arg = NS_ConvertUTF8toUTF16(GetParamVal(param_list,PID_needvarresult));
      rv = mysample->Divide(wide_markup.get(),arg.get(),&res_str);
      break;
    case CCID_Polynomial_Partial_Fractions:
      arg = NS_ConvertUTF8toUTF16(GetParamVal(param_list,PID_needvarresult));
      rv = mysample->PartialFractions(wide_markup.get(),arg.get(),&res_str);
      break;
    case CCID_Polynomial_Sort:
      arg = NS_ConvertUTF8toUTF16(GetParamVal(param_list,PID_needvarresult));
      rv = mysample->Sort(wide_markup.get(),arg.get(),&res_str);
      break;
    case CCID_Polynomial_Companion_Matrix:
      arg = NS_ConvertUTF8toUTF16(GetParamVal(param_list,PID_needvarresult));
      rv = mysample->CompanionMatrix(wide_markup.get(),arg.get(),&res_str);
      break;
    // calculus commands
    case CCID_Calculus_Integrate_by_Parts:
      arg = NS_ConvertUTF8toUTF16(GetParamVal(param_list,PID_CalcByPartsVar));
      rv = mysample->ByParts(wide_markup.get(),arg.get(),&res_str);
      break;
    case CCID_Calculus_Change_Variable:
      arg = NS_ConvertUTF8toUTF16(GetParamVal(param_list,PID_CalcChangeVar));
      rv = mysample->ChangeVar(wide_markup.get(),arg.get(),&res_str);
      break;
    case CCID_Calculus_Approximate_Integral:
      arg = NS_ConvertUTF8toUTF16(GetNaturalParamVal(param_list,PID_approxintform));
      arg2 = NS_ConvertUTF8toUTF16(GetParamVal(param_list,PID_approxintnsubs));
      rv = mysample->ApproxIntegral(wide_markup.get(),arg.get(),arg2.get(),&res_str);
      break;
    case CCID_Calculus_Iterate:
      arg = NS_ConvertUTF8toUTF16(GetParamVal(param_list,PID_iterfunc));
      arg2 = NS_ConvertUTF8toUTF16(GetParamVal(param_list,PID_iterstart));
      arg3 = NS_ConvertUTF8toUTF16(GetParamVal(param_list,PID_itercount));
      rv = mysample->Iterate(arg.get(),arg2.get(),arg3.get(),&res_str);
      break;
    case CCID_Calculus_Implicit_Differentiation:
      arg = NS_ConvertUTF8toUTF16(GetParamVal(param_list,PID_ImplDiffIndepVar));
      arg2 = NS_ConvertUTF8toUTF16(GetParamVal(param_list,PID_ImplDiffDepVars));
      rv = mysample->ImplicitDiff(wide_markup.get(),arg.get(),arg2.get(),&res_str);
      break;
    // matrix commands
    case CCID_Characteristic_Polynomial:
      arg = NS_ConvertUTF8toUTF16(GetParamVal(param_list,PID_needvarresult));
      rv = mysample->CharacteristicPolynomial(wide_markup.get(),arg.get(),&res_str);
      break;
    case CCID_Fill_Matrix:
      arg = NS_ConvertUTF8toUTF16(GetNaturalParamVal(param_list,PID_FillMatrixIlk));
      arg2 = NS_ConvertUTF8toUTF16(GetParamVal(param_list,PID_FillMatrixnRows));
      arg3 = NS_ConvertUTF8toUTF16(GetParamVal(param_list,PID_FillMatrixnCols));
      arg4 = NS_ConvertUTF8toUTF16(GetParamVal(param_list,PID_FillMatrixExtra));
      rv = mysample->MatrixFill(arg.get(),arg2.get(),arg3.get(),arg4.get(),&res_str);
      break;
    case CCID_Map_Entries:
      arg = NS_ConvertUTF8toUTF16(GetParamVal(param_list,PID_MapsFunction));
      rv = mysample->Map(wide_markup.get(),arg.get(),&res_str);
      break;
    case CCID_Minimal_Polynomial:
      arg = NS_ConvertUTF8toUTF16(GetParamVal(param_list,PID_needvarresult));
      rv = mysample->MinimalPolynomial(wide_markup.get(),arg.get(),&res_str);
      break;
    case CCID_Random_Matrix:
      arg = NS_ConvertUTF8toUTF16(GetNaturalParamVal(param_list,PID_RandMatrixType));
      arg2 = NS_ConvertUTF8toUTF16(GetParamVal(param_list,PID_RandMatrixnRows));
      arg3 = NS_ConvertUTF8toUTF16(GetParamVal(param_list,PID_RandMatrixnCols));
      arg4 = NS_ConvertUTF8toUTF16(GetParamVal(param_list,PID_RandMatrixLLim));
      arg5 = NS_ConvertUTF8toUTF16(GetParamVal(param_list,PID_RandMatrixULim));
      rv = mysample->MatrixRandom(arg.get(),arg2.get(),arg3.get(),arg4.get(),arg5.get(),&res_str);
      break;
    case CCID_Reshape:
      arg = NS_ConvertUTF8toUTF16(GetParamVal(param_list,PID_ncolsReshape));
      rv = mysample->MatrixReshape(wide_markup.get(),arg.get(),&res_str);
      break;
    // statistics commands
    case CCID_Fit_Curve_to_Data:
      arg = NS_ConvertUTF8toUTF16(GetNaturalParamVal(param_list,PID_regresscode));
      arg2 = NS_ConvertUTF8toUTF16(GetNaturalParamVal(param_list,PID_dependcolumn));
      arg3 = NS_ConvertUTF8toUTF16(GetParamVal(param_list,PID_regressdegree));
      rv = mysample->FitCurve(wide_markup.get(),arg.get(),arg2.get(),arg3.get(),&res_str);
      break;
    case CCID_Moment:
      arg = NS_ConvertUTF8toUTF16(GetParamVal(param_list,PID_moment_num));
      arg2 = NS_ConvertUTF8toUTF16(GetParamVal(param_list,PID_moment_origin));
      arg3 = NS_ConvertUTF8toUTF16(GetNaturalParamVal(param_list,PID_moment_meanisorigin));
      rv = mysample->Moment(wide_markup.get(),arg.get(),arg2.get(),arg3.get(),&res_str);
      break;
    case CCID_Quantile:
      arg = NS_ConvertUTF8toUTF16(GetParamVal(param_list,PID_quantile));
      rv = mysample->Quantile(wide_markup.get(),arg.get(),&res_str);
      break;
    case CCID_Random_Numbers:
      arg = NS_ConvertUTF8toUTF16(GetNaturalParamVal(param_list,PID_RandomNumDist));
      arg2 = NS_ConvertUTF8toUTF16(GetParamVal(param_list,PID_RandomNumTally));
      arg3 = NS_ConvertUTF8toUTF16(GetParamVal(param_list,PID_RandomNumParam1));
      arg4 = NS_ConvertUTF8toUTF16(GetParamVal(param_list,PID_RandomNumParam2));
      rv = mysample->RandomNumbers(arg.get(),arg2.get(),arg3.get(),arg4.get(),&res_str);
      break;
    case CCID_Define:
      arg = NS_ConvertUTF8toUTF16(GetNaturalParamVal(param_list,PID_subInterpretation));
      rv = mysample->Define(wide_markup.get(),arg.get(),&res_str);
      break;
    default:
      NS_ASSERTION(0,"don't know how to do this command");
      break;
  }
  
  if (NS_ERROR_GET_MODULE(rv) == NS_ERROR_MODULE_GENERAL) {
    res = NS_ERROR_GET_CODE(rv);
    switch (res) {
      case CR_failure:
      case CR_undefined:
        fputs("<p>\nFailure or undefined\n</p>\n", outfile);
        return 1;
        break;
      case CR_nosol:
        fputs("<p>\nNo Solution Found\n</p>\n", outfile);
        break;
      case CR_noDefineReserved:
        fputs("<p>\nBad Definition - Function name is reserved\n</p>\n",
              outfile);
        break;
      case CR_missingBoundaries:
        fputs("<p>\nNo Initial Values - Boundary conditions are missing\n</p>\n",
          outfile);
        break;
      case CR_needvars:
        fputs("<p>\nneedvars\n</p>\n", outfile);
        break;
      case CR_queryindepvar:
        fputs("<p>\nqueryindepvar\n</p>\n", outfile);
        break;
      case CR_badindepvar:
        fputs("<p>\nIndependent variable(s) not free\n</p>\n", outfile);
        break;
      case CR_undefinedfunc:
        fputs("<p>\nFunction for Iteration not defined\n</p>\n", outfile);
        break;
      case CR_badintegral:
        fputs("<p>\nIntegral not found\n</p>\n", outfile);
        break;
      case CR_badsolvesystem:
        fputs("<p>\nSolve System - Bad format, [Mnxm][Mmx1]=[Mnx1] expected\n</p>\n",
          outfile);
        break;
      case CR_NeedSubInterp:
        fputs("<p>\nNeed interpretation of subscript in definition.\n</p>\n", outfile);
        break;
      case CR_baddefformat:
        fputs("<p>\nBad definition format\n</p>\n", outfile);
        break;

      case CR_success:
      case CR_numericint:
      case CR_ringfactor:
      case CR_bool_true:
      case CR_bool_false:
      case CR_EqCheck_true:
      case CR_EqCheck_false:
      case CR_EqCheck_undecidable:
      case CR_quantile:
      case CR_eigenvectors:
        if (res_str) {
          nsCAutoString res;
          WideToASCII(res_str,res);
          fputs("<p>\n", outfile);
          fputs(res.get(), outfile);
          fputs("\n</p>\n", outfile);
        }
        lastresult = res_str;
        break;

      default:
        NS_ASSERTION(0,"Unknown computation return code.");
        break;
    }
  } else if (NS_FAILED(rv)) {
    printf("ERROR: Calling msiISimpleComputeEngine::Eval function [%x]\n", rv);
    res = -4;
  } else if (res_str) {
    nsCAutoString res;
    WideToASCII(res_str,res);
    fputs("<p>\n", outfile);
    fputs(res.get(), outfile);
    fputs("\n</p>\n", outfile);
    lastresult = res_str;
  }
  return res;
}

unsigned short CmdName2cmdID(const char *cmd_name)
{
  unsigned short rv = 0;

  if (cmd_name && *cmd_name) {
    size_t zln = strlen(cmd_name);
    switch (zln) {
    case 3:
      if (!strcmp(cmd_name, "SVD"))
        rv = CCID_SVD;
      break;
    case 4:
      if (!strcmp(cmd_name, "Curl"))
        rv = CCID_Curl;
      else if (!strcmp(cmd_name, "Mean"))
        rv = CCID_Mean;
      else if (!strcmp(cmd_name, "Mode"))
        rv = CCID_Mode;
      else if (!strcmp(cmd_name, "Norm"))
        rv = CCID_Norm;
      else if (!strcmp(cmd_name, "Rank"))
        rv = CCID_Rank;
      break;
    case 5:
      if (!strcmp(cmd_name, "Fixup"))
        rv = CCID_Fixup;
      else if (!strcmp(cmd_name, "Stack"))
        rv = CCID_Stack;
      else if (!strcmp(cmd_name, "Trace"))
        rv = CCID_Trace;
      break;
    case 6:
      if (!strcmp(cmd_name, "Define"))
        rv = CCID_Define;
      else if (!strcmp(cmd_name, "Expand"))
        rv = CCID_Expand;
      else if (!strcmp(cmd_name, "Factor"))
        rv = CCID_Factor;
      else if (!strcmp(cmd_name, "Median"))
        rv = CCID_Median;
      break;
    case 7:
      if (!strcmp(cmd_name, "Cleanup"))
        rv = CCID_Cleanup;
      else if (!strcmp(cmd_name, "Hessian"))
        rv = CCID_Hessian;
      else if (!strcmp(cmd_name, "Inverse"))
        rv = CCID_Inverse;
      break;
    case 8:
      if (!strcmp(cmd_name, "Adjugate"))
        rv = CCID_Adjugate;
      else if (!strcmp(cmd_name, "Evaluate"))
        rv = CCID_Evaluate;
      else if (!strcmp(cmd_name, "Gradient"))
        rv = CCID_Gradient;
      else if (!strcmp(cmd_name, "Jacobian"))
        rv = CCID_Jacobian;
      else if (!strcmp(cmd_name, "Simplify"))
        rv = CCID_Simplify;
      else if (!strcmp(cmd_name, "Undefine"))
        rv = CCID_Undefine;
      else if (!strcmp(cmd_name, "Variance"))
        rv = CCID_Variance;
      break;
    case 9:
      if (!strcmp(cmd_name, "Interpret"))
        rv = CCID_Interpret;
      else if (!strcmp(cmd_name, "Laplacian"))
        rv = CCID_Laplacian;
      else if (!strcmp(cmd_name, "Moment..."))
        rv = CCID_Moment;
      else if (!strcmp(cmd_name, "Permanent"))
        rv = CCID_Permanent;
      else if (!strcmp(cmd_name, "Row Basis"))
        rv = CCID_Row_Basis;
      else if (!strcmp(cmd_name, "Solve PDE"))
        rv = CCID_Solve_PDE;
      else if (!strcmp(cmd_name, "Transpose"))
        rv = CCID_Transpose;
      else if (!strcmp(cmd_name, "Wronskian"))
        rv = CCID_Wronskian;
      else if (!strcmp(cmd_name, "Pass Thru"))
        rv = CCID_PassThru;
      break;
    case 10:
      if (!strcmp(cmd_name, "Covariance"))
        rv = CCID_Covariance;
      else if (!strcmp(cmd_name, "Divergence"))
        rv = CCID_Divergence;
      else if (!strcmp(cmd_name, "Reshape..."))
        rv = CCID_Reshape;
      break;
    case 11:
      if (!strcmp(cmd_name, "Concatenate"))
        rv = CCID_Concatenate;
      else if (!strcmp(cmd_name, "Correlation"))
        rv = CCID_Correlation;
      else if (!strcmp(cmd_name, "Determinant"))
        rv = CCID_Determinant;
      else if (!strcmp(cmd_name, "Eigenvalues"))
        rv = CCID_Eigenvalues;
      else if (!strcmp(cmd_name, "Jordan Form"))
        rv = CCID_Jordan_Form;
      else if (!strcmp(cmd_name, "Quantile..."))
        rv = CCID_Quantile;
      else if (!strcmp(cmd_name, "Rewrite sin"))
        rv = CCID_Rewrite_sin;
      else if (!strcmp(cmd_name, "Rewrite cos"))
        rv = CCID_Rewrite_cos;
      else if (!strcmp(cmd_name, "Rewrite tan"))
        rv = CCID_Rewrite_tan;
      else if (!strcmp(cmd_name, "Solve Exact"))
        rv = CCID_Solve_Exact;
      break;
    case 12:
      if (!strcmp(cmd_name, "Column Basis"))
        rv = CCID_Column_Basis;
      else if (!strcmp(cmd_name, "Combine Logs"))
        rv = CCID_Combine_Logs;
      else if (!strcmp(cmd_name, "Eigenvectors"))
        rv = CCID_Eigenvectors;
      else if (!strcmp(cmd_name, "Simplex Dual"))
        rv = CCID_Simplex_Dual;
      else if (!strcmp(cmd_name, "Solve System"))
        rv = CCID_Solve_System;
      break;
    case 13:
      if (!strcmp(cmd_name, "Harmonic Mean"))
        rv = CCID_Harmonic_Mean;
      else if (!strcmp(cmd_name, "Rewrite Float"))
        rv = CCID_Rewrite_Float;
      else if (!strcmp(cmd_name, "Rewrite Gamma"))
        rv = CCID_Rewrite_Gamma;
      else if (!strcmp(cmd_name, "Rewrite Mixed"))
        rv = CCID_Rewrite_Mixed;
      else if (!strcmp(cmd_name, "Rewrite Polar"))
        rv = CCID_Rewrite_Polar;
      else if (!strcmp(cmd_name, "Solve Integer"))
        rv = CCID_Solve_Integer;
      else if (!strcmp(cmd_name, "Solve Numeric"))
        rv = CCID_Solve_Numeric;
      break;
    case 14:
      if (!strcmp(cmd_name, "Check Equality"))
        rv = CCID_Check_Equality;
      else if (!strcmp(cmd_name, "Combine Powers"))
        rv = CCID_Combine_Powers;
      else if (!strcmp(cmd_name, "Combine Arctan"))
        rv = CCID_Combine_Arctan;
      else if (!strcmp(cmd_name, "Factor in ring"))
        rv = CCID_Factor_in_ring;
      else if (!strcmp(cmd_name, "Fill Matrix..."))
        rv = CCID_Fill_Matrix;
      else if (!strcmp(cmd_name, "Map Entries..."))
        rv = CCID_Map_Entries;
      else if (!strcmp(cmd_name, "Geometric Mean"))
        rv = CCID_Geometric_Mean;
      else if (!strcmp(cmd_name, "Mean Deviation"))
        rv = CCID_Mean_Deviation;
      else if (!strcmp(cmd_name, "Rewrite arcsin"))
        rv = CCID_Rewrite_arcsin;
      else if (!strcmp(cmd_name, "Rewrite arccos"))
        rv = CCID_Rewrite_arccos;
      else if (!strcmp(cmd_name, "Rewrite arctan"))
        rv = CCID_Rewrite_arctan;
      else if (!strcmp(cmd_name, "Rewrite arccot"))
        rv = CCID_Rewrite_arccot;
      break;
    case 15:
      if (!strcmp(cmd_name, "Nullspace Basis"))
        rv = CCID_Nullspace_Basis;
      else if (!strcmp(cmd_name, "Polynomial Sort"))
        rv = CCID_Polynomial_Sort;
      else if (!strcmp(cmd_name, "Power Series..."))
        rv = CCID_Power_Series;
      else if (!strcmp(cmd_name, "Singular Values"))
        rv = CCID_Singular_Values;
      else if (!strcmp(cmd_name, "Solve ODE Exact"))
        rv = CCID_Solve_ODE_Exact;
      else if (!strcmp(cmd_name, "Solve Recursion"))
        rv = CCID_Solve_Recursion;
      else if (!strcmp(cmd_name, "Spectral Radius"))
        rv = CCID_Spectral_Radius;
      break;
    case 16:
      if (!strcmp(cmd_name, "Condition Number"))
        rv = CCID_Condition_Number;
      else if (!strcmp(cmd_name, "Polynomial Roots"))
        rv = CCID_Polynomial_Roots;
      else if (!strcmp(cmd_name, "QR Decomposition"))
        rv = CCID_QR_Decomposition;
      else if (!strcmp(cmd_name, "Random Matrix..."))
        rv = CCID_Random_Matrix;
      else if (!strcmp(cmd_name, "Rewrite Rational"))
        rv = CCID_Rewrite_Rational;
      else if (!strcmp(cmd_name, "Scalar Potential"))
        rv = CCID_Scalar_Potential;
      else if (!strcmp(cmd_name, "Simplex Feasible"))
        rv = CCID_Simplex_Feasible;
      else if (!strcmp(cmd_name, "Simplex Maximize"))
        rv = CCID_Simplex_Maximize;
      else if (!strcmp(cmd_name, "Simplex Minimize"))
        rv = CCID_Simplex_Minimize;
      else if (!strcmp(cmd_name, "Solve ODE Series"))
        rv = CCID_Solve_ODE_Series;
      else if (!strcmp(cmd_name, "Vector Potential"))
        rv = CCID_Vector_Potential;
      else if (!strcmp(cmd_name, "Plot Function 2d"))
        rv = CCID_PlotFuncCmd;
      else if (!strcmp(cmd_name, "Plot Function 3d"))
        rv = CCID_PlotFuncCmd;
      break;
    case 17:
      if (!strcmp(cmd_name, "Fourier Transform"))
        rv = CCID_Fourier_Transform;
      else if (!strcmp(cmd_name, "Laplace Transform"))
        rv = CCID_Laplace_Transform;
      else if (!strcmp(cmd_name, "PLU Decomposition"))
        rv = CCID_PLU_Decomposition;
      else if (!strcmp(cmd_name, "Polynomial Divide"))
        rv = CCID_Polynomial_Divide;
      else if (!strcmp(cmd_name, "Random Numbers..."))
        rv = CCID_Random_Numbers;
      else if (!strcmp(cmd_name, "Rewrite Factorial"))
        rv = CCID_Rewrite_Factorial;
      else if (!strcmp(cmd_name, "Rewrite Logarithm"))
        rv = CCID_Rewrite_Logarithm;
      else if (!strcmp(cmd_name, "Smith Normal Form"))
        rv = CCID_Smith_Normal_Form;
      else if (!strcmp(cmd_name, "Solve ODE Laplace"))
        rv = CCID_Solve_ODE_Laplace;
      else if (!strcmp(cmd_name, "Solve ODE Numeric"))
        rv = CCID_Solve_ODE_Numeric;
      break;
    case 18:
      if (!strcmp(cmd_name, "Definiteness Tests"))
        rv = CCID_Definiteness_Tests;
      else if (!strcmp(cmd_name, "Minimal Polynomial"))
        rv = CCID_Minimal_Polynomial;
      else if (!strcmp(cmd_name, "Orthogonality Test"))
        rv = CCID_Orthogonality_Test;
      else if (!strcmp(cmd_name, "Standard Deviation"))
        rv = CCID_Standard_Deviation;
      break;
    case 19:
      if (!strcmp(cmd_name, "Calculus Iterate..."))
        rv = CCID_Calculus_Iterate;
      else if (!strcmp(cmd_name, "Hermite Normal Form"))
        rv = CCID_Hermite_Normal_Form;
      else if (!strcmp(cmd_name, "Hermitian Transpose"))
        rv = CCID_Hermitian_Transpose;
      else if (!strcmp(cmd_name, "Rewrite Exponential"))
        rv = CCID_Rewrite_Exponential;
      else if (!strcmp(cmd_name, "Rewrite Normal Form"))
        rv = CCID_Rewrite_Normal_Form;
      else if (!strcmp(cmd_name, "Rewrite Rectangular"))
        rv = CCID_Rewrite_Rectangular;
      else if (!strcmp(cmd_name, "Rewrite sin and cos"))
        rv = CCID_Rewrite_sin_and_cos;
      else if (!strcmp(cmd_name, "Simplex Standardize"))
        rv = CCID_Simplex_Standardize;
      break;
    case 20:
      if (!strcmp(cmd_name, "Combine Exponentials"))
        rv = CCID_Combine_Exponentials;
      else if (!strcmp(cmd_name, "Evaluate Numerically"))
        rv = CCID_Evaluate_Numerically;
      else if (!strcmp(cmd_name, "Fit Curve to Data..."))
        rv = CCID_Fit_Curve_to_Data;
      else if (!strcmp(cmd_name, "Gaussian Elimination"))
        rv = CCID_Gaussian_Elimination;
      break;
    case 21:
      if (!strcmp(cmd_name, "Calculus Find Extrema"))
        rv = CCID_Calculus_Find_Extrema;
      else if (!strcmp(cmd_name, "Polynomial Collect..."))
        rv = CCID_Polynomial_Collect;
      else if (!strcmp(cmd_name, "Rewrite sinh and cosh"))
        rv = CCID_Rewrite_sinh_and_cosh;
      break;
    case 22:
      if (!strcmp(cmd_name, "Cholesky Decomposition"))
        rv = CCID_Cholesky_Decomposition;
      else if (!strcmp(cmd_name, "Combine Trig Functions"))
        rv = CCID_Combine_Trig_Functions;
      break;
    case 23:
      if (!strcmp(cmd_name, "Rational Canonical Form"))
        rv = CCID_Rational_Canonical_Form;
      break;
    case 24:
      if (!strcmp(cmd_name, "Reduced Row Echelon Form"))
        rv = CCID_Reduced_Row_Echelon_Form;
      break;
    case 25:
      if (!strcmp(cmd_name, "Characteristic Polynomial"))
        rv = CCID_Characteristic_Polynomial;
      else if (!strcmp(cmd_name, "Inverse Fourier Transform"))
        rv = CCID_Inverse_Fourier_Transform;
      else if (!strcmp(cmd_name, "Inverse Laplace Transform"))
        rv = CCID_Inverse_Laplace_Transform;
      break;
    case 26:
      if (!strcmp(cmd_name, "Calculus Partial Fractions"))
        rv = CCID_Calculus_Partial_Fractions;
      else if (!strcmp(cmd_name, "Solve for Subexpression..."))
        rv = CCID_Solve_for_Subexpression;
      break;
    case 27:
      if (!strcmp(cmd_name, "Calculus Change Variable..."))
        rv = CCID_Calculus_Change_Variable;
      else if (!strcmp(cmd_name, "Polynomial Companion Matrix"))
        rv = CCID_Polynomial_Companion_Matrix;
      else if (!strcmp(cmd_name, "Rewrite Equations as Matrix"))
        rv = CCID_Rewrite_Equations_as_Matrix;
      else if (!strcmp(cmd_name, "Rewrite Matrix as Equations"))
        rv = CCID_Rewrite_Matrix_as_Equations;
      break;
    case 28:
      if (!strcmp(cmd_name, "Polynomial Partial Fractions"))
        rv = CCID_Polynomial_Partial_Fractions;
      break;
    case 30:
      if (!strcmp(cmd_name, "Calculus Integrate by Parts..."))
        rv = CCID_Calculus_Integrate_by_Parts;
      break;
    case 32:
      if (!strcmp(cmd_name, "Calculus Approximate Integral..."))
        rv = CCID_Calculus_Approximate_Integral;
      break;
    case 33:
      if (!strcmp(cmd_name, "Combine Hyperbolic Trig Functions"))
        rv = CCID_Combine_Hyp_Trig_Functions;
      break;
    case 34:
      if (!strcmp(cmd_name, "Calculus Plot Approximate Integral"))
        rv = CCID_Calculus_Plot_Approximate_Integral;
      else if (!strcmp(cmd_name, "Fraction-Free Gaussian Elimination"))
        rv = CCID_Fraction_Free_Gaussian_Elimination;
      break;
    case 36:
      if (!strcmp(cmd_name, "Calculus Implicit Differentiation..."))
        rv = CCID_Calculus_Implicit_Differentiation;
      break;
    default:
      NS_ASSERTION(0, "unrecognized name");
      break;
    }
  } else {
    NS_ASSERTION(0, "bad input");
  }
  return rv;
}

// Parameter IDs are scripted by name - here we return the corresponding ID.
// The master name <-> ID map is kept in "iCmpTypes.h".

U32 ParamNameToID(const char *p_name)
{
  U32 rv = 0;

  if (p_name && *p_name) {
    size_t zln = strlen(p_name);
    switch (zln) {
    case 12:
      if (!strcmp(p_name, "PID_quantile"))
        rv = PID_quantile;
      else if (!strcmp(p_name, "PID_iterfunc"))
        rv = PID_iterfunc;
      break;
    case 13:
      if (!strcmp(p_name, "PID_seriesvar"))
        rv = PID_seriesvar;
      else if (!strcmp(p_name, "PID_iterstart"))
        rv = PID_iterstart;
      else if (!strcmp(p_name, "PID_itercount"))
        rv = PID_itercount;
      break;
    case 14:
      if (!strcmp(p_name, "PID_moment_num"))
        rv = PID_moment_num;
      break;
    case 15:
      if (!strcmp(p_name, "PID_seriesorder"))
        rv = PID_seriesorder;
      else if (!strcmp(p_name, "PID_regresscode"))
        rv = PID_regresscode;
      else if (!strcmp(p_name, "PID_seriesabout"))
        rv = PID_seriesabout;
      else if (!strcmp(p_name, "PID_ODEIndepVar"))
        rv = PID_ODEIndepVar;
      break;
    case 16:
      if (!strcmp(p_name, "PID_ncolsReshape"))
        rv = PID_ncolsReshape;
      else if (!strcmp(p_name, "PID_dependcolumn"))
        rv = PID_dependcolumn;
      else if (!strcmp(p_name, "PID_MapsFunction"))
        rv = PID_MapsFunction;
      break;
    case 17:
      if (!strcmp(p_name, "PID_needvarresult"))
        rv = PID_needvarresult;
      else if (!strcmp(p_name, "PID_moment_origin"))
        rv = PID_moment_origin;
      else if (!strcmp(p_name, "PID_dependentvars"))
        rv = PID_dependentvars;
      else if (!strcmp(p_name, "PID_approxintform"))
        rv = PID_approxintform;
      else if (!strcmp(p_name, "PID_regressdegree"))
        rv = PID_regressdegree;
      else if (!strcmp(p_name, "PID_CalcChangeVar"))
        rv = PID_CalcChangeVar;
      else if (!strcmp(p_name, "PID_FillMatrixIlk"))
        rv = PID_FillMatrixIlk;
      else if (!strcmp(p_name, "PID_RandomNumDist"))
        rv = PID_RandomNumDist;
      break;
    case 18:
      if (!strcmp(p_name, "PID_independentvar"))
        rv = PID_independentvar;
      else if (!strcmp(p_name, "PID_PolyCollectVar"))
        rv = PID_PolyCollectVar;
      else if (!strcmp(p_name, "PID_approxintnsubs"))
        rv = PID_approxintnsubs;
      else if (!strcmp(p_name, "PID_CalcByPartsVar"))
        rv = PID_CalcByPartsVar;
      else if (!strcmp(p_name, "PID_RandMatrixType"))
        rv = PID_RandMatrixType;
      else if (!strcmp(p_name, "PID_RandMatrixLLim"))
        rv = PID_RandMatrixLLim;
      else if (!strcmp(p_name, "PID_RandMatrixULim"))
        rv = PID_RandMatrixULim;
      else if (!strcmp(p_name, "PID_RandomNumTally"))
        rv = PID_RandomNumTally;
      else if (!strcmp(p_name, "PID_ODEseriesorder"))
        rv = PID_ODEseriesorder;
      else if (!strcmp(p_name, "PID_ODEseriesabout"))
        rv = PID_ODEseriesabout;
      break;
    case 19:
      if (!strcmp(p_name, "PID_FillMatrixnRows"))
        rv = PID_FillMatrixnRows;
      else if (!strcmp(p_name, "PID_FillMatrixnCols"))
        rv = PID_FillMatrixnCols;
      else if (!strcmp(p_name, "PID_FillMatrixExtra"))
        rv = PID_FillMatrixExtra;
      else if (!strcmp(p_name, "PID_RandMatrixnRows"))
        rv = PID_RandMatrixnRows;
      else if (!strcmp(p_name, "PID_RandMatrixnCols"))
        rv = PID_RandMatrixnCols;
      else if (!strcmp(p_name, "PID_RandomNumParam1"))
        rv = PID_RandomNumParam1;
      else if (!strcmp(p_name, "PID_RandomNumParam2"))
        rv = PID_RandomNumParam2;
      else if (!strcmp(p_name, "PID_SolveForSubExpr"))
        rv = PID_SolveForSubExpr;
      else if (!strcmp(p_name, "PID_ImplDiffDepVars"))
        rv = PID_ImplDiffDepVars;
      break;
    case 20:
      if (!strcmp(p_name, "PID_ImplDiffIndepVar"))
        rv = PID_ImplDiffIndepVar;
      break;
    case 21:
      if (!strcmp(p_name, "PID_VecBasisVariables"))
        rv = PID_VecBasisVariables;
      else if (!strcmp(p_name, "PID_subInterpretation"))
        rv = PID_subInterpretation;
      break;
    case 23:
      if (!strcmp(p_name, "PID_moment_meanisorigin"))
        rv = PID_moment_meanisorigin;
      break;
    default:
      NS_ASSERTION(0, "unrecognized name");
      break;
    }
  } else {
    NS_ASSERTION(0, "bad input");
  }
  NS_ASSERTION(rv, "unrecognized name");
  return rv;
}

U32 PrefNameToID(char *p_name)
{
  U32 rv = 0;

  if (p_name && *p_name) {
    size_t zln = strlen(p_name);
    switch (zln) {
    case 10:
      if (!strcmp(p_name, "mml_prefix"))
        rv = CLPF_mml_prefix;
      break;
    case 11:
      if (!strcmp(p_name, "mml_version"))
        rv = CLPF_mml_version;
      else if (!strcmp(p_name, "use_mfenced"))
        rv = CLPF_use_mfenced;
      break;
    case 12:
      if (!strcmp(p_name, "Vector_basis"))
        rv = CLPF_Vector_basis;
      else if (!strcmp(p_name, "D_derivative"))
        rv = CLPF_D_derivative;
      break;
    case 13:
      if (!strcmp(p_name, "clr_math_attr"))
        rv = CLPF_clr_math_attr;
      else if (!strcmp(p_name, "clr_func_attr"))
        rv = CLPF_clr_func_attr;
      else if (!strcmp(p_name, "clr_text_attr"))
        rv = CLPF_clr_text_attr;
      else if (!strcmp(p_name, "clr_unit_attr"))
        rv = CLPF_clr_unit_attr;
      else if (!strcmp(p_name, "log_is_base_e"))
        rv = CLPF_log_is_base_e;
      else if (!strcmp(p_name, "Input_e_Euler"))
        rv = CLPF_Input_e_Euler;
      break;
    case 14:
      if (!strcmp(p_name, "Output_ln_base"))
        rv = CLPF_Output_Euler_e;
      else if (!strcmp(p_name, "Dot_derivative"))
        rv = CLPF_Dot_derivative;
      break;
    case 15:
      if (!strcmp(p_name, "math_node_attrs"))
        rv = CLPF_math_node_attrs;
      break;
    case 16:
      if (!strcmp(p_name, "Prime_derivative"))
        rv = CLPF_Prime_derivative;
      break;
    case 17:
      if (!strcmp(p_name, "Output_imaginaryi"))
        rv = CLPF_Output_imaginaryi;
      else if (!strcmp(p_name, "Overbar_conjugate"))
        rv = CLPF_Overbar_conjugate;
      else if (!strcmp(p_name, "Input_i_imaginary"))
        rv = CLPF_Input_i_Imaginary;
      else if (!strcmp(p_name, "Input_j_imaginary"))
        rv = CLPF_Input_j_Imaginary;
      break;
    case 18:
      if (!strcmp(p_name, "Parens_on_trigargs"))
        rv = CLPF_Parens_on_trigargs;
      else if (!strcmp(p_name, "Output_parens_mode"))
        rv = CLPF_Output_parens_mode;
      break;
    case 19:
      if (!strcmp(p_name, "Sig_digits_rendered"))
        rv = CLPF_Sig_digits_rendered;
      break;
    case 20:
      if (!strcmp(p_name, "Output_differentialD"))
        rv = CLPF_Output_differentialD;
      else if (!strcmp(p_name, "Output_differentiald"))
        rv = CLPF_Output_differentiald;
      else if (!strcmp(p_name, "SciNote_lower_thresh"))
        rv = CLPF_SciNote_lower_thresh;
      else if (!strcmp(p_name, "SciNote_upper_thresh"))
        rv = CLPF_SciNote_upper_thresh;
      else if (!strcmp(p_name, "Primes_as_(n)_thresh"))
        rv = CLPF_Primes_as_n_thresh;
      else if (!strcmp(p_name, "Output_Mixed_Numbers"))
        rv = CLPF_Output_Mixed_Numbers;
      break;
    case 21:
      if (!strcmp(p_name, "Default_matrix_delims"))
        rv = CLPF_Default_matrix_delims;
      else if (!strcmp(p_name, "Output_InvTrigFuncs_1"))
        rv = CLPF_Output_InvTrigFuncs_1;
      break;

    case 25:
      if (!strcmp(p_name, "Default_derivative_format"))
        rv = CLPF_Default_derivative_format;
      break;

    default:
      NS_ASSERTION(0, "unrecognized name");
      break;
    }
  } else {
    NS_ASSERTION(0, "bad input");
  }

  return rv;
}

// Parameter Types are scripted by name - here we return the corresponding ID.
// The master name <-> ID map is kept in "iCmpTypes.h".

U32 ParamTypeToID(char *p_type)
{
  U32 rv = 0;

  if (p_type && *p_type) {
    size_t zln = strlen(p_type);
    switch (zln) {
    case 13:
      NS_ASSERTION(0, "Wide chars not supported.");
      if (!strcmp(p_type, "zPT_WIDE_text"))
        rv = zPT_WIDE_text;
      else if (!strcmp(p_type, "zPT_WIDE_real"))
        rv = zPT_WIDE_real;
      break;
    case 14:
      if (!strcmp(p_type, "zPT_ASCII_text"))
        rv = zPT_ASCII_text;
      else if (!strcmp(p_type, "zPT_ASCII_real"))
        rv = zPT_ASCII_real;
      break;
    case 16:
      NS_ASSERTION(0, "Wide chars not supported.");
      if (!strcmp(p_type, "zPT_WIDE_natural"))
        rv = zPT_WIDE_natural;
      break;
    case 17:
      if (!strcmp(p_type, "zPT_ASCII_natural"))
        rv = zPT_ASCII_natural;
      break;
    case 18:
      NS_ASSERTION(0, "Wide chars not supported.");
      if (!strcmp(p_type, "zPT_WIDE_mmlmarkup"))
        rv = zPT_WIDE_mmlmarkup;
      break;
    case 19:
      if (!strcmp(p_type, "zPT_ASCII_mmlmarkup"))
        rv = zPT_ASCII_mmlmarkup;
      break;
    default:
      NS_ASSERTION(0, "unrecognized type string");
      break;
    }
  } else {
    NS_ASSERTION(0, "bad input");
  }
  return rv;
}

//param_str =  "PID_ncolsReshape,zPT_ASCII_natural,3"

char *GetParamInfo(char *param_str, U32 & p_ID, U32 & p_type)
{
  char *rv = NULL;
  p_ID = 0;
  p_type = 0;

  if (param_str && *param_str) {
    char *p1 = strstr(param_str, "PID_");
    if (p1) {
      char *p2 = strchr(p1, ',');
      if (p2) {
        *p2 = 0;
        p2++;
        char *p3 = strstr(p2, "zPT_ASCII_");
        if (p3) {
          char *p4 = strchr(p3, ',');
          if (p4) {
            *p4 = 0;
            p4++;
            while (*p4 && *p4 <= ' ')
              p4++;

            p_ID = ParamNameToID(p1);
            p_type = ParamTypeToID(p3);
            rv = p4;
          } else {
            NS_ASSERTION(0, "punt");
          }
        } else {
          NS_ASSERTION(0, "punt");
        }
      } else {
        NS_ASSERTION(0, "punt");
      }
    } else {
      NS_ASSERTION(0, "punt");
    }
  } else {
    NS_ASSERTION(0, "punt");
  }
  return rv;
}

// p_data = "MuPAD,SeriesOrder,5"

int GetEngStateParams(char *p_data, U32 & engine_ID, int &attr_ID,
                      nsACString & new_value)
{
  if (p_data && *p_data) {
    if (!strncmp(p_data, "Maple5", 6))
      engine_ID = maple_eng_ID;
    else if (!strncmp(p_data, "MuPAD", 5))
      engine_ID = MuPAD_eng_ID;
    else {
      NS_ASSERTION(0, "bad engine");
      return 1;
    }
    char *p = strchr(p_data, ',');
    if (p) {
      p++;
      while (*p && *p <= ' ')
        p++;
      if (!strncmp(p, "SeriesOrder", 11))
        attr_ID = 1;
      else if (!strncmp(p, "Digits", 6))
        attr_ID = 2;
      else if (!strncmp(p, "MaxDegree", 9))
        attr_ID = 3;
      else if (!strncmp(p, "PvalOnly", 8))
        attr_ID = 4;
      else if (!strncmp(p, "IgnoreSCases", 12))
        attr_ID = 5;
      else if (!strncmp(p, "FloatFormat", 11))
        attr_ID = 6;
      else if (!strncmp(p, "VarType", 7))
        attr_ID = 7;
      else {
        NS_ASSERTION(0, "bad state name");
        return 1;
      }
    } else {
      NS_ASSERTION(0, "no state name");
      return 1;
    }
    char *p1 = strchr(p, ',');
    if (p1) {
      p1++;
      while (*p1 && *p1 <= ' ')
        p1++;
      new_value = p1;
      return 0;
    } else {
      NS_ASSERTION(0, "bad format");
      return 1;
    }
  }
  NS_ASSERTION(0, "bad command format");
  return 1;
}

// p_data = "0,mml_version,1"

int GetClientPrefParams(char *p_data, U32 & client_ID, U32 & pref_ID,
                        nsACString & new_value)
{
  if (p_data && *p_data) {
    client_ID = atoi(p_data);

    char *p = strchr(p_data, ',');
    if (p) {
      p++;
      while (*p && *p <= ' ')
        p++;
      size_t zln = strlen(p);
      char *buffer = new char[zln + 1];
      strcpy(buffer, p);
      char *ptr = strchr(buffer, ',');
      if (ptr)
        *ptr = 0;
      pref_ID = PrefNameToID(buffer);
      delete[] buffer;
    } else {
      NS_ASSERTION(0, "bad command format");
      return 1;
    }
    char *p1 = strchr(p, ',');
    if (p1) {
      p1++;
      while (*p1 && *p1 <= ' ')
        p1++;
      new_value = p1;
      return 0;
    } else {
      NS_ASSERTION(0, "bad command format");
      return 1;
    }
  }
  NS_ASSERTION(0, "bad command format");
  return 1;
}

// Extract the data for 1 computation from srcfile.
// Pass required info back to the caller.
// Return an error_code
//    error_code  =  1; - EOF
//    error_code  =  6; - unexpected EOF
//    error_code  =  5; - error - bad format for "SetEngineState" or "InstallEngine" command
//    error_code  =  2; - error - 2 Test IDs - missing "End of test"?
//    error_code  =  3; - EOF within MathML
//    error_code  =  4; - Untagged line before state == 15

int GetComputationInfo(FILE * srcfile,
                       nsACString & passthru,
                       nsACString & test_ID,
                       nsACString & comment,
                       U32 & client_ID,
                       U32 & engine_ID,
                       unsigned short & cmd_ID,
                       int &set_ID,
                       U32 & pref_ID,
                       U32 & new_client,
                       nsACString & src_markup,
                       PARAM_REC ** param_list, 
                       U32 & result_code)
{
  int error_code = 0;

  passthru.Truncate();
  test_ID.Truncate();
  comment.Truncate();
  client_ID = 0;
  engine_ID = 0;
  cmd_ID = 0;
  set_ID = 0;
  pref_ID = 0;
  new_client = 0;
  src_markup.Truncate();
  *param_list = NULL;
  result_code = 0;

  char fline[8192];
  int state = 0;
  bool exit_outer_loop = false;
  while (!exit_outer_loop) {
    char *res = fgets(fline, 4096, srcfile);
    if (!res) {                 // end of file?
      error_code = 1;
      break;
    }
    size_t zln = strlen(fline);
    // remove ending '\n'.
    while (zln > 0 && fline[zln - 1] <= ' ') {
      zln -= 1;
      fline[zln] = 0;
    }
    if (zln > 0 && fline[0] != ';') { // non-empty and non-comment
      if (!strncmp(fline, "InstallEngine", 13)) {
        if (state) {
          NS_ASSERTION(!state, "error - bad format for Install command");
          error_code = 5;
        } else {
          char *p = fline + 13;
          while (*p <= ' ')
            p++;
          if (!strcmp(p, "Maple5")) {
            state = 15;
            engine_ID = 1;
          } else if (!strcmp(p, "MuPAD")) {
            state = 15;
            engine_ID = 2;
          } else {
            NS_ASSERTION(0, "unknown engine  name??");
          }
        }
        exit_outer_loop = true;
      } else if (!strncmp(fline, "SetEngineState", 14)) {
        if (state) {
          NS_ASSERTION(!state, "error - bad format for Set command");
          error_code = 5;
        } else {
          char *p = fline + 14;
          while (*p <= ' ')
            p++;
          int err = GetEngStateParams(p, engine_ID, set_ID, src_markup);
          if (err)
            error_code = 5;     // error - bad format for "Set" command
          else
            state = 15;
        }
        exit_outer_loop = true;
      } else if (!strncmp(fline, "SetClientPref", 13)) {
        if (state) {
          NS_ASSERTION(!state, "error - bad format for Set command");
          error_code = 5;
        } else {
          char *p = fline + 13;
          while (*p <= ' ')
            p++;
          int err = GetClientPrefParams(p, client_ID, pref_ID, src_markup);
          if (err)
            error_code = 5;     // error - bad format for "Set" command
          else
            state = 15;
        }
        exit_outer_loop = true;
      } else if (!strncmp(fline, "CreateNextClient", 16)) {
        if (state) {
          NS_ASSERTION(!state, "error - bad format for Set command");
          error_code = 5;
        } else {
          char *p = fline + 16;
          while (*p <= ' ')
            p++;
          if (*p && isdigit(*p)) {
            client_ID = atol(p);
            state = 15;
            new_client = 1;
          } else
            error_code = 5;
        }
        exit_outer_loop = true;
      } else if (!strncmp(fline, "Test ID", 7)) {
        if (state & 1) {
          NS_ASSERTION(!(state & 1), "error - missing End of test");
          error_code = 2;
          exit_outer_loop = true;
        } else {                // Record the Test ID string
          state += 1;
          char *p = fline + 7;
          while (*p <= ' ')
            p++;
          test_ID = p;
        }
      } else if (!strncmp(fline, "Passthru", 8)) {
        char *p = fline + 8;
        while (*p <= ' ')
          p++;
        passthru = p;
      } else if (!strncmp(fline, "Comment", 7)) {
        char *p = fline + 7;
        while (*p <= ' ')
          p++;
        comment = p;
      } else if (!strncmp(fline, "Client ID", 9)) {
        if (state & 2) {
          NS_ASSERTION(!(state & 2), "2 client IDs specified in 1 test?");
        } else {
          state += 2;
          char *p = fline + 9;
          while (*p <= ' ')
            p++;
          client_ID = atol(p);
        }
      } else if (!strncmp(fline, "Result", 6)) {
        char *p = fline + 6;
        while (*p <= ' ')
          p++;
        result_code = atol(p);
      } else if (!strncmp(fline, "Engine", 6)) {
        if (state & 4) {
          NS_ASSERTION(!(state & 4), "2 engines specified in 1 test?");
        } else {
          char *p = fline + 6;
          while (*p <= ' ')
            p++;
          if (!strcmp(p, "Maple5")) {
            state += 4;
            engine_ID = maple_eng_ID;
          } else if (!strcmp(p, "MuPAD")) {
            state += 4;
            engine_ID = MuPAD_eng_ID;
          } else {
            NS_ASSERTION(0, "unknown engine  name??");
          }
        }
      } else if (!strncmp(fline, "Command ID", 10)) {
        if (state & 8) {
          NS_ASSERTION(!(state & 8), "2 Command IDs specified in 1 test?");
        } else {
          char *p = fline + 10;
          while (*p <= ' ')
            p++;
          cmd_ID = CmdName2cmdID(p);
          if (!cmd_ID) {
            NS_ASSERTION(cmd_ID, "Unknown command?");
          } else
            state += 8;
        }
      } else if (!strncmp(fline, "Param", 5)) {
        char *p = fline + 5;
        while (*p <= ' ')
          p++;
        U32 p_ID;
        U32 p_type;
        char *zdata = GetParamInfo(p, p_ID, p_type);
        if (zdata)
          *param_list = AppendParam(*param_list, p_ID, p_type, zdata);
        else
          NS_ASSERTION(0, "GetParamInfo failed");
      } else if (!strncmp(fline, "End of test", 11)) {
        exit_outer_loop = true;
      } else {                  // semantic line has no tag
        if (state == 15) {
          do {
            if (fline[0] != ';') {  // non-comment
              src_markup += fline;
              char *ptr = strstr(fline, "</mml:math>");
              if (!ptr)
                ptr = strstr(fline, "</math>");
              if (!ptr)
                ptr = strstr(fline, "</graph>");
              if (ptr)
                break;
            }
            char *res = fgets(fline, 4096, srcfile);
            if (!res) {
              exit_outer_loop = true;
              error_code = 3;
              break;
            }
          } while (1);
        } else {
          NS_ASSERTION(0, "non-tagged line before required args are set - error");
          error_code = 4;
          exit_outer_loop = true;
        }
      }
    }
  }
  bool ok = false;
  if (error_code <= 1) {
    if (state == 15)            // Here we have all data for 1 transaction
      ok = true;
    else
      error_code = 6;           // unexpected EOF
  }
  return error_code;
}


int ProcessTestScript(CompEngine &aEngine, const char *aFile)
{
  if (!aFile || !*aFile) {
    NS_ASSERTION(0,"give me a file name");
    return 1;
  }
  // First, try to open the output file - for now it's "aFile"
  //  with the extension changed from ".tscript" to ".xhtml".

  size_t sf_ln = strlen(aFile);
  size_t dot_pos = sf_ln - 1;
  while (dot_pos) {
    if (aFile[dot_pos] == '.')
      break;
    dot_pos--;
  }
  if (aFile[dot_pos] != '.') {
    NS_ASSERTION(0,"could not find \'.\' in selected_file");
    return 1;
  }
  if (tcistricmp(".tscript", aFile + dot_pos)) {
    NS_ASSERTION(0,"Source File Extension Error (tscript required)");
    return 4;
  }
  FILE *srcfile = fopen(aFile, "r");
  if (!srcfile) {
    NS_ASSERTION(0,"Failed to read open source file");
    return 6;
  }
  char dest_file_spec[512];
  strcpy(dest_file_spec, aFile);
  strcpy(dest_file_spec + dot_pos, ".xhtml");
  FILE *outfile = fopen(dest_file_spec, "wt");
  if (!outfile) {
    NS_ASSERTION(0,"couldn't open output");
    return 2;
  }
  // If we get to this point, source and out files have been opened.

  WriteXHTMLHeader(outfile);

  fputs("<p><b>Test Script:</b> ", outfile);
  fputs(aFile, outfile);
  fputs("</p>\n", outfile);
  WriteTime("Test Start", outfile);

  // loop thru computations in "aFile"
  int error_code = 0;
  while (!error_code) {
    nsCAutoString passthru;
    nsCAutoString test_ID;
    nsCAutoString comment;
    nsCAutoString src_markup;
    U32 client_ID;
    U32 engine_ID;
    unsigned short cmd_ID;
    int set_attr_ID;
    U32 pref_ID;
    U32 new_client;
    PARAM_REC *param_list;
    U32 result_code;

    error_code = GetComputationInfo(srcfile, passthru, test_ID, comment, client_ID,
                                    engine_ID, cmd_ID, set_attr_ID, pref_ID,
                                    new_client, src_markup, &param_list, result_code);
    if (!error_code) {
      char buffer[1000]; //XXX tried to use nsPrintfCString but that crashed compiler
      if (new_client) {
        NS_ASSERTION(0, "msiSimpleComputeEngine can't switch clients.");
      } else if (set_attr_ID) {
        NS_ASSERTION(engine_ID == 0, "don't know how to switch engines");
        long stateVal = atol(src_markup.get());
        aEngine.SetEngineAttr(set_attr_ID,stateVal);
        sprintf(buffer,"<p>\nSetEngineState(%d,%d,%s)\n</p>\n", engine_ID,
                      set_attr_ID, src_markup.get());
        fputs(buffer, outfile);
      } else if (pref_ID) {
        NS_ASSERTION(client_ID == 0 || client_ID == 1, "don't know how to switch clients");
        if (pref_ID == CLPF_Vector_basis) {
          aEngine.SetVectorBasis(src_markup);
        } else {
          long prefVal = atol(src_markup.get());
          aEngine.SetClientPref(pref_ID,prefVal);
        }
        sprintf(buffer,"<p>\nSetClientPref(%d,%d,%s)\n</p>\n", client_ID,
                      pref_ID, src_markup.get());
        fputs(buffer, outfile);
      } else if (engine_ID != 0 && cmd_ID == 0) {
        if (aEngine.SwitchEngine(engine_ID)) {
          fprintf(outfile, "<p>\nInstallEngine(%d) succeeded\n</p>\n", engine_ID);
        } else {
          fprintf(outfile, "Engine failed to load\n");
          error_code = 1;
        }
      } else {
        if (!passthru.IsEmpty()) {
          fputs(passthru.get(), outfile);
          fputs("\n", outfile);
        }
        fputs("<p>\nTest ID ", outfile);
        fputs(test_ID.get(), outfile);
        fputs("\n</p>\n", outfile);
        if (!comment.IsEmpty()) {
          fputs("<p>", outfile);
          fputs(comment.get(), outfile);
          fputs("</p>\n", outfile);
        }
        error_code =
          aEngine.DoComputation(outfile, client_ID, engine_ID, cmd_ID,
                        src_markup, param_list);
        if (error_code != result_code) {
          fprintf(outfile, "Result code mismatch: expected %d, got %d\n", result_code, error_code);
          error_code = 1; // so get stop
        } else if (result_code && error_code == result_code) {
          fprintf(outfile, "Result code got %d as expected\n", error_code);
          error_code = 0; // so get keep running
        }
      }
    } else {
      if (error_code == 2)
        fputs("Missing \"End of test\"", outfile);
      else if (error_code == 3)
        fputs("Found EOF in MathML", outfile);
      else if (error_code == 4)
        fputs("Missing command data", outfile);
      else if (error_code == 5)
        fputs("Bad \"Set\" command", outfile);
    }
    DisposeParamList(param_list);
    fflush(outfile);
  }

  WriteTime("Test End", outfile);
  WriteXHTMLFooter(outfile);

  fclose(outfile);
  fclose(srcfile);
  return 0;
}
