
// msiISimpleComputeEngine.idl

// how I get #include past the preprocessor
IDLinclude "nsISupports.idl"
IDLinclude "nsILocalFile.idl"

#include "iCmpIDs.h"

/*
 * msiISimpleComputeEngine - connection to computation engine
 *
 * Very much a work in progress. Emphasis is on prototyping.
 */

[scriptable, uuid(A3F85155-6CB7-46aa-9FA4-35108F696130)]
interface msiISimpleComputeEngine : nsISupports
{
  /**
   * some special error codes. see nsError.h and iCmpIDs.h
  */
  const unsigned long nosol     = 2155347978;
  const unsigned long needvars  = 2155348068;
  const unsigned long needivars = 2155348069;
  const unsigned long needsubinterp = 2155348074;
  /**
   * Start the engine
  */
  void startup(in nsILocalFile engFile);

  /**
   * Evaluate the input
  */
  void eval(in wstring expr, [retval] out wstring result);
  void evalNumeric(in wstring expr, [retval] out wstring result);

  /**
   * Command IDs.  Pass these to Perform()
  */
  const unsigned long Evaluate                    = CCID_Evaluate;            
  const unsigned long Evaluate_Numerically        = CCID_Evaluate_Numerically;
  const unsigned long Simplify				            = CCID_Simplify;							 
  const unsigned long Combine_Exponentials		    = CCID_Combine_Exponentials;
  const unsigned long Combine_Logs						    = CCID_Combine_Logs;
  const unsigned long Combine_Powers					    = CCID_Combine_Powers;
  const unsigned long Combine_Trig_Functions	    = CCID_Combine_Trig_Functions;
  const unsigned long Combine_Arctan					    = CCID_Combine_Arctan;
  const unsigned long Combine_Hyperbolic_Functions = CCID_Combine_Hyp_Trig_Functions;
  const unsigned long Factor					            = CCID_Factor;								 
  const unsigned long Expand					            = CCID_Expand;								 

  void equationsAsMatrix(in wstring expr, in wstring vars, [retval] out wstring result);
  void matrixAsEquations(in wstring expr, in wstring vars, [retval] out wstring result);
  const unsigned long Rewrite_Rational					  = CCID_Rewrite_Rational;
  const unsigned long Rewrite_Float						    = CCID_Rewrite_Float;
  const unsigned long Rewrite_Mixed						    = CCID_Rewrite_Mixed;
  const unsigned long Rewrite_Exponential				  = CCID_Rewrite_Exponential;
  const unsigned long Rewrite_Factorial					  = CCID_Rewrite_Factorial;
  const unsigned long Rewrite_Gamma						    = CCID_Rewrite_Gamma;
  const unsigned long Rewrite_Logarithm					  = CCID_Rewrite_Logarithm;
  const unsigned long Rewrite_sin_and_cos				  = CCID_Rewrite_sin_and_cos;
  const unsigned long Rewrite_sinh_and_cosh				= CCID_Rewrite_sinh_and_cosh;
  const unsigned long Rewrite_sin				          = CCID_Rewrite_sin;
  const unsigned long Rewrite_cos				          = CCID_Rewrite_cos;
  const unsigned long Rewrite_tan						      = CCID_Rewrite_tan;
  const unsigned long Rewrite_arcsin						  = CCID_Rewrite_arcsin;
  const unsigned long Rewrite_arccos						  = CCID_Rewrite_arccos;
  const unsigned long Rewrite_arctan						  = CCID_Rewrite_arctan;
  const unsigned long Rewrite_arccot						  = CCID_Rewrite_arccot;
  const unsigned long Rewrite_Polar						    = CCID_Rewrite_Polar;
  const unsigned long Rewrite_Rectangular				  = CCID_Rewrite_Rectangular;
  const unsigned long Rewrite_Normal_Form				  = CCID_Rewrite_Normal_Form;
  const unsigned long Rewrite_Equations_as_Matrix	= CCID_Rewrite_Equations_as_Matrix;
  const unsigned long Rewrite_Matrix_as_Equations	= CCID_Rewrite_Matrix_as_Equations;
  const unsigned long Check_Equality	            = CCID_Check_Equality;
  /**
   * solve commands
  */
  void solveExact(in wstring expr, in wstring vars, [retval] out wstring result);
  void solveInteger(in wstring expr, in wstring vars, [retval] out wstring result);

  const unsigned long Solve_Exact		   = CCID_Solve_Exact;		
  const unsigned long Solve_Integer	   = CCID_Solve_Integer;	
  const unsigned long Solve_Numeric	   = CCID_Solve_Numeric;	
  const unsigned long Solve_Recursion  = CCID_Solve_Recursion;
  // polynonomials
  void collect(in wstring expr, in wstring vars, [retval] out wstring result);
  void divide(in wstring expr, in wstring vars, [retval] out wstring result);
  void partialFractions(in wstring expr, in wstring vars, [retval] out wstring result);
  void sort(in wstring expr, in wstring vars, [retval] out wstring result);
  void roots(in wstring expr, in wstring vars, [retval] out wstring result);
  void companionMatrix(in wstring expr, in wstring vars, [retval] out wstring result);
  const unsigned long Polynomial_Collect   			   = CCID_Polynomial_Collect;   			 
  const unsigned long Polynomial_Divide					   = CCID_Polynomial_Divide;					 
  const unsigned long Polynomial_Partial_Fractions = CCID_Polynomial_Partial_Fractions;
  const unsigned long Polynomial_Roots					   = CCID_Polynomial_Roots;					   
  const unsigned long Polynomial_Sort					     = CCID_Polynomial_Sort;					   
  const unsigned long Polynomial_Companion_Matrix  = CCID_Polynomial_Companion_Matrix; 
  // calculus
  void byParts(in wstring expr, in wstring var, [retval] out wstring result);
  void changeVar(in wstring expr, in wstring var, [retval] out wstring result);
  void approxIntegral(in wstring expr, in wstring form, in wstring numintervals, in wstring lowerBound, in wstring upperBound, [retval] out wstring result);
  void iterate(in wstring expr, in wstring startval, in wstring count, [retval] out wstring result);
  void implicitDiff(in wstring expr, in wstring var, in wstring depvar, [retval] out wstring result);
  void findExtrema(in wstring expr, in wstring var, [retval] out wstring result);
  const unsigned long Calculus_Integrate_by_Parts   		 = CCID_Calculus_Integrate_by_Parts;   		
  const unsigned long Calculus_Change_Variable   		     = CCID_Calculus_Change_Variable;   		  
  const unsigned long Calculus_Partial_Fractions			   = CCID_Calculus_Partial_Fractions;			  
  const unsigned long Calculus_Approximate_Integral   	 = CCID_Calculus_Approximate_Integral;   	
  const unsigned long Calculus_Plot_Approximate_Integral = CCID_Calculus_Plot_Approximate_Integral;
  const unsigned long Calculus_Find_Extrema				       = CCID_Calculus_Find_Extrema;				    
  const unsigned long Calculus_Iterate   				         = CCID_Calculus_Iterate;   				      
  const unsigned long Calculus_Implicit_Differentiation  = CCID_Calculus_Implicit_Differentiation;
  // series
  void powerSeries(in wstring expr, in wstring var, in wstring about, in wstring order, [retval] out wstring result);
  const unsigned long Power_Series                       = CCID_Power_Series;
  // ODEs
  void solveODEExact(in wstring expr, in wstring var, [retval] out wstring result);
  void solveODELaplace(in wstring expr, in wstring var, [retval] out wstring result);
  void solveODENumeric(in wstring expr, in wstring var, [retval] out wstring result);
  void solveODESeries(in wstring expr, in wstring var, in wstring about, in wstring order, [retval] out wstring result);
  const unsigned long Solve_PDE				  = CCID_Solve_PDE;				
  const unsigned long Solve_ODE_Exact	  = CCID_Solve_ODE_Exact;	
  const unsigned long Solve_ODE_Laplace = CCID_Solve_ODE_Laplace;
  const unsigned long Solve_ODE_Numeric = CCID_Solve_ODE_Numeric;
  const unsigned long Solve_ODE_Series	= CCID_Solve_ODE_Series;
  // transforms	
  const unsigned long Fourier_Transform				  = CCID_Fourier_Transform;				
  const unsigned long Inverse_Fourier_Transform = CCID_Inverse_Fourier_Transform;
  const unsigned long Laplace_Transform				  = CCID_Laplace_Transform;				
  const unsigned long Inverse_Laplace_Transform = CCID_Inverse_Laplace_Transform;
  // vector calculus
  void wronskian(in wstring expr, in wstring var, [retval] out wstring result);
  const unsigned long Gradient				 = CCID_Gradient;				 
  const unsigned long Divergence			 = CCID_Divergence;			 
  const unsigned long Curl						 = CCID_Curl;						 
  const unsigned long Laplacian			   = CCID_Laplacian;			 
  const unsigned long Jacobian				 = CCID_Jacobian;				 
  const unsigned long Hessian				   = CCID_Hessian;				 
  const unsigned long C_Wronskian      = CCID_Wronskian;			 
  const unsigned long Scalar_Potential = CCID_Scalar_Potential;
  const unsigned long Vector_Potential = CCID_Vector_Potential;
  /**
   * Matrix commands
  */
  void characteristicPolynomial(in wstring expr, in wstring var, [retval] out wstring result);
  void matrixFill(in wstring type, in wstring rows, in wstring cols, in wstring expr, [retval] out wstring result);
  void map(in wstring expr, in wstring func, [retval] out wstring result);
  void minimalPolynomial(in wstring expr, in wstring var, [retval] out wstring result);
  void matrixRandom(in wstring type, in wstring rows, in wstring cols, in wstring llim, in wstring ulim, [retval] out wstring result);
  void matrixReshape(in wstring expr, in wstring ncols, [retval] out wstring result);
  const unsigned long Adjugate	                         = CCID_Adjugate;
  const unsigned long Characteristic_Polynomial	         = CCID_Characteristic_Polynomial;
  const unsigned long Cholesky_Decomposition	           = CCID_Cholesky_Decomposition;
  const unsigned long Column_Basis	                     = CCID_Column_Basis;
  const unsigned long Concatenate	                       = CCID_Concatenate;
  const unsigned long Condition_Number	                 = CCID_Condition_Number;
  const unsigned long Definiteness_Tests                 = CCID_Definiteness_Tests;
  const unsigned long Determinant			                   = CCID_Determinant;			                 
  const unsigned long Eigenvalues			                   = CCID_Eigenvalues;			                 
  const unsigned long Eigenvectors			                 = CCID_Eigenvectors;			                 
  const unsigned long Fill_Matrix   		                 = CCID_Fill_Matrix;   		                 
  const unsigned long Fraction_Free_Gaussian_Elimination = CCID_Fraction_Free_Gaussian_Elimination;
  const unsigned long Gaussian_Elimination               = CCID_Gaussian_Elimination;              
  const unsigned long Hermite_Normal_Form                = CCID_Hermite_Normal_Form;               
  const unsigned long Hermitian_Transpose                = CCID_Hermitian_Transpose;               
  const unsigned long Inverse					                   = CCID_Inverse;					                 
  const unsigned long Jordan_Form			                   = CCID_Jordan_Form;			                 
  const unsigned long Map_Entries			                   = CCID_Map_Entries;			                 
  const unsigned long Minimal_Polynomial                 = CCID_Minimal_Polynomial;                
  const unsigned long Norm							                 = CCID_Norm;							                 
  const unsigned long Nullspace_Basis	                   = CCID_Nullspace_Basis;	                 
  const unsigned long Orthogonality_Test                 = CCID_Orthogonality_Test;                
  const unsigned long Permanent				                   = CCID_Permanent;				                 
  const unsigned long PLU_Decomposition                  = CCID_PLU_Decomposition;                 
  const unsigned long QR_Decomposition	                 = CCID_QR_Decomposition;	                 
  const unsigned long Random_Matrix   	                 = CCID_Random_Matrix;   	                 
  const unsigned long Rank							                 = CCID_Rank;							                 
  const unsigned long Rational_Canonical_Form            = CCID_Rational_Canonical_Form;           
  const unsigned long Reduced_Row_Echelon_Form           = CCID_Reduced_Row_Echelon_Form;          
  const unsigned long Reshape   				                 = CCID_Reshape;   				                 
  const unsigned long Row_Basis				                   = CCID_Row_Basis;				                 
  const unsigned long Singular_Values	                   = CCID_Singular_Values;	                 
  const unsigned long Smith_Normal_Form                  = CCID_Smith_Normal_Form;                 
  const unsigned long Spectral_Radius	                   = CCID_Spectral_Radius;	                 
  const unsigned long Stack						                   = CCID_Stack;						                 
  const unsigned long SVD							                   = CCID_SVD;							                 
  const unsigned long Trace						                   = CCID_Trace;						                 
  const unsigned long Transpose				                   = CCID_Transpose;				                 
  // simplex
  const unsigned long Simplex_Dual				 = CCID_Simplex_Dual;				
  const unsigned long Simplex_Feasible		 = CCID_Simplex_Feasible;		
  const unsigned long Simplex_Maximize		 = CCID_Simplex_Maximize;		
  const unsigned long Simplex_Minimize		 = CCID_Simplex_Minimize;		
  const unsigned long Simplex_Standardize  = CCID_Simplex_Standardize;
  // statistics                                = 
  void fitCurve(in wstring expr, in wstring code, in wstring column, in wstring degree, [retval] out wstring result);
  void randomNumbers(in wstring type, in wstring tally, in wstring param1, in wstring param2, [retval] out wstring result);
  void moment(in wstring expr, in wstring num, in wstring origin, in wstring mean_is_origin, [retval] out wstring result);
  void quantile(in wstring expr, in wstring q, [retval] out wstring result);
  const unsigned long Fit_Curve_to_Data    = CCID_Fit_Curve_to_Data;  
  const unsigned long Random_Numbers   	   = CCID_Random_Numbers;   	
  const unsigned long Mean								 = CCID_Mean;								
  const unsigned long Median							 = CCID_Median;							
  const unsigned long Mode								 = CCID_Mode;								
  const unsigned long Correlation				   = CCID_Correlation;				
  const unsigned long Covariance					 = CCID_Covariance;					
  const unsigned long Geometric_Mean			 = CCID_Geometric_Mean;			
  const unsigned long Harmonic_Mean			   = CCID_Harmonic_Mean;			
  const unsigned long Mean_Deviation			 = CCID_Mean_Deviation;			
  const unsigned long C_Moment   					 = CCID_Moment;   					
  const unsigned long C_Quantile   				 = CCID_Quantile;   				
  const unsigned long Standard_Deviation	 = CCID_Standard_Deviation;	
  const unsigned long Variance						 = CCID_Variance;						

  const unsigned long PassThru						 = CCID_PassThru;						
  const unsigned long Cleanup						 = CCID_Cleanup;						
  const unsigned long Fixup  						 = CCID_Fixup;						
  const unsigned long Interpret						 = CCID_Interpret;						

  void getVariables (in wstring expr, [retval] out wstring result);
//  const unsigned long GetVariables         = CCID_GetVariables;

  /**
   * Plot the input.  A simple interface until we can figure out a better one.
  */
  void plotfuncCmd(in wstring graph, [retval] out wstring result);
  const unsigned long Plotfunc_cmd						 = CCID_PlotFuncCmd;						

  void plotfuncQuery(in wstring graph, [retval] out wstring result);
  const unsigned long Plotfunc_query						 = CCID_PlotFuncQuery;						

  /**
   * Make a definition
  */
  void define(in wstring expr, in wstring subinterp, [retval] out wstring result);
  /**
   * Remove a definition
  */
  void undefine(in wstring expr, [retval] out wstring result);
  /**
   * Get current definitions. Each definition is nested in a <p> tag.
  */
  void getDefinitions([retval] out wstring result);
  /**
   * Clear definitions
  */
  void clearDefinitions();

  /**
   * Define a mupad name
   */
  void defineMupadName(in wstring swpname, in wstring mupname, in wstring loc, [retval] out wstring result);
  

  /*
   * Vector Basis. Kind of a define, kind of a user preference.
  */
  void getVectorBasis([retval] out wstring result);
  void setVectorBasis(in wstring expr);
  
  /**
   * User Pref IDs.
  */
    const unsigned long SeriesOrder         = EID_SeriesOrder;
    const unsigned long Digits              = EID_Digits;
    const unsigned long MaxDegree           = EID_MaxDegree;
    const unsigned long PvalOnly            = EID_PvalOnly;
    const unsigned long IgnoreSCases        = EID_IgnoreSCases;
    const unsigned long FloatFormat         = EID_FloatFormat;
    const unsigned long VarType             = EID_VarType;
  /**                                                 
   * Set the attribute
  */
  void setEngineAttr(in unsigned long attrID, in long value);
  /**
   * Get the attribute
  */
  long getEngineAttr(in unsigned long attrID);

  /**
   * Get the string sent to/from the engine.
  */
  void getEngineSent([retval] out wstring result);
  void getEngineReceived([retval] out wstring result);
  void getEngineErrors([retval] out wstring result);

  /**
   * Clear previous error and return strings.
  */
  void clearEngineStrings();

  /**
   * User Pref IDs.
  */
    const unsigned long mml_version                = CLPF_mml_version;
    const unsigned long mml_prefix                 = CLPF_mml_prefix;
    const unsigned long math_node_attrs            = CLPF_math_node_attrs;
    const unsigned long use_mfenced                = CLPF_use_mfenced;
    const unsigned long clr_math_attr              = CLPF_clr_math_attr;
    const unsigned long clr_func_attr              = CLPF_clr_func_attr;
    const unsigned long clr_text_attr              = CLPF_clr_text_attr;
    const unsigned long clr_unit_attr              = CLPF_clr_unit_attr;
    const unsigned long Sig_digits_rendered        = CLPF_Sig_digits_rendered;
    const unsigned long SciNote_lower_thresh       = CLPF_SciNote_lower_thresh;
    const unsigned long SciNote_upper_thresh       = CLPF_SciNote_upper_thresh;
    const unsigned long Primes_as_n_thresh         = CLPF_Primes_as_n_thresh;
    const unsigned long Parens_on_trigargs         = CLPF_Parens_on_trigargs;
    const unsigned long log_is_base_e              = CLPF_log_is_base_e;
    const unsigned long Output_diffD_uppercase     = CLPF_Output_differentialD;
    const unsigned long Output_diffd_lowercase     = CLPF_Output_differentiald;
    const unsigned long Output_Euler_e             = CLPF_Output_Euler_e;
    const unsigned long Output_imaginaryi          = CLPF_Output_imaginaryi;
    const unsigned long Output_InvTrigFuncs_1      = CLPF_Output_InvTrigFuncs_1;
    const unsigned long Output_Mixed_Numbers       = CLPF_Output_Mixed_Numbers;
    const unsigned long Output_parens_mode         = CLPF_Output_parens_mode;
    const unsigned long Default_matrix_delims      = CLPF_Default_matrix_delims;
    const unsigned long Default_derivative_format  = CLPF_Default_derivative_format;
    const unsigned long Dot_derivative             = CLPF_Dot_derivative;
    const unsigned long Overbar_conjugate          = CLPF_Overbar_conjugate;
    const unsigned long Input_i_Imaginary          = CLPF_Input_i_Imaginary;
    const unsigned long Input_j_Imaginary          = CLPF_Input_j_Imaginary;
    const unsigned long Input_e_Euler              = CLPF_Input_e_Euler;
    const unsigned long Prime_means_derivative     = CLPF_Prime_derivative;
    const unsigned long D_is_derivative            = CLPF_D_derivative;
    const unsigned long Vector_basis               = CLPF_Vector_basis;
  /**
   * Set the user pref.  Some prefs need string values, but we don't use those yet.
  */
  void setUserPref(in unsigned long attrID, in long value);
  void setUserPrefByName(in string attrID, in long value);


  /**
   * Get the user pref.
  */
  long getUserPref(in unsigned long attrID);

  /**
   * Perform the operation on the input.  Pass in command ID.
  */
  void perform(in wstring expr, in unsigned long operation, [retval] out wstring result);

  /** 
   * Stop a computation
  */
  void stopProcessor();
  

  /**
   * Shutdown the engine
  */
  void shutdown();
};



