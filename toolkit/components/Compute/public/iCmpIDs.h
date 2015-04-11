// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

#ifndef iCMPIDS_H
#define iCMPIDS_H

// C preprocessor trickery used in .gmx files
#define UID2(_str,_id) _str##_id
#define UID(_id,_subid) UID2(uID,_id)._subid

// IDs for compute commands

#define CCID_Evaluate                             1
#define CCID_Evaluate_Numerically				  2
#define CCID_Simplify							  3
#define CCID_Combine_Exponentials				  4
#define CCID_Combine_Logs						  5
#define CCID_Combine_Powers						  6
#define CCID_Combine_Trig_Functions				  7
#define CCID_Combine_Arctan                       8
#define CCID_Combine_Hyp_Trig_Functions           9
#define CCID_Factor								  10
#define CCID_Expand								  11
#define CCID_Rewrite_Rational					  12
#define CCID_Rewrite_Float						  13
#define CCID_Rewrite_Mixed						  14
#define CCID_Rewrite_Exponential				  15
#define CCID_Rewrite_Factorial					  16
#define CCID_Rewrite_Gamma						  17
#define CCID_Rewrite_Logarithm					  18
#define CCID_Rewrite_sin_and_cos				  19
#define CCID_Rewrite_sinh_and_cosh				  20
#define CCID_Rewrite_sin				          21
#define CCID_Rewrite_cos				          22
#define CCID_Rewrite_tan						  23
#define CCID_Rewrite_arcsin				          24
#define CCID_Rewrite_arccos				          25
#define CCID_Rewrite_arctan						  26
#define CCID_Rewrite_arccot						  27
#define CCID_Rewrite_Polar						  28
#define CCID_Rewrite_Rectangular				  29
#define CCID_Rewrite_Normal_Form				  30
#define CCID_Rewrite_Equations_as_Matrix		  31
#define CCID_Rewrite_Matrix_as_Equations		  32
#define CCID_Check_Equality						  33
#define CCID_Solve_Exact						  34
#define CCID_Solve_Integer						  35
#define CCID_Solve_Numeric						  36
#define CCID_Solve_Recursion					  37
#define CCID_Polynomial_Collect   				  38
#define CCID_Polynomial_Divide					  39
#define CCID_Polynomial_Partial_Fractions		  40
#define CCID_Polynomial_Roots					  41
#define CCID_Polynomial_Sort					  42
#define CCID_Polynomial_Companion_Matrix		  43
#define CCID_Calculus_Integrate_by_Parts   		  44
#define CCID_Calculus_Change_Variable   		  45
#define CCID_Calculus_Partial_Fractions			  46
#define CCID_Calculus_Approximate_Integral   	  47
#define CCID_Calculus_Plot_Approximate_Integral	  48
#define CCID_Calculus_Find_Extrema				  49
#define CCID_Calculus_Iterate   				  50
#define CCID_Calculus_Implicit_Differentiation    51
#define CCID_Power_Series   					  52
#define CCID_Solve_PDE							  53
#define CCID_Solve_ODE_Exact					  54
#define CCID_Solve_ODE_Laplace					  55
#define CCID_Solve_ODE_Numeric					  56
#define CCID_Solve_ODE_Series					  57
#define CCID_Fourier_Transform					  58
#define CCID_Inverse_Fourier_Transform			  59
#define CCID_Laplace_Transform					  60
#define CCID_Inverse_Laplace_Transform			  61
#define CCID_Gradient							  62
#define CCID_Divergence							  63
#define CCID_Curl								  64
#define CCID_Laplacian							  65
#define CCID_Jacobian							  66
#define CCID_Hessian							  67
#define CCID_Wronskian							  68
#define CCID_Scalar_Potential					  69
#define CCID_Vector_Potential					  70
#define CCID_Adjugate							  71
#define CCID_Characteristic_Polynomial			  72
#define CCID_Cholesky_Decomposition				  73
#define CCID_Column_Basis						  74
#define CCID_Concatenate						  75
#define CCID_Condition_Number					  76
#define CCID_Definiteness_Tests					  77
#define CCID_Determinant						  78
#define CCID_Eigenvalues						  79
#define CCID_Eigenvectors						  80
#define CCID_Fill_Matrix   						  81
#define CCID_Fraction_Free_Gaussian_Elimination	  82
#define CCID_Gaussian_Elimination				  83
#define CCID_Hermite_Normal_Form				  84
#define CCID_Hermitian_Transpose				  85
#define CCID_Inverse							  86
#define CCID_Jordan_Form						  87
#define CCID_Map_Entries						  88
#define CCID_Minimal_Polynomial					  89
#define CCID_Norm								  90
#define CCID_Nullspace_Basis					  91
#define CCID_Orthogonality_Test					  92
#define CCID_Permanent							  93
#define CCID_PLU_Decomposition					  94
#define CCID_QR_Decomposition					  95
#define CCID_Random_Matrix   					  96
#define CCID_Rank								  97
#define CCID_Rational_Canonical_Form			  98
#define CCID_Reduced_Row_Echelon_Form			  99
#define CCID_Reshape   							  100
#define CCID_Row_Basis							  101
#define CCID_Singular_Values					  102
#define CCID_Smith_Normal_Form					  103
#define CCID_Spectral_Radius					  104
#define CCID_Stack								  105
#define CCID_SVD								  106
#define CCID_Trace								  107
#define CCID_Transpose							  108
#define CCID_Simplex_Dual						  109
#define CCID_Simplex_Feasible					  110
#define CCID_Simplex_Maximize					  111
#define CCID_Simplex_Minimize					  112
#define CCID_Simplex_Standardize				  113
#define CCID_Fit_Curve_to_Data   				  114
#define CCID_Random_Numbers   					  115
#define CCID_Mean								  116
#define CCID_Median								  117
#define CCID_Mode								  118
#define CCID_Correlation						  119
#define CCID_Covariance							  120
#define CCID_Geometric_Mean						  121
#define CCID_Harmonic_Mean						  122
#define CCID_Mean_Deviation						  123
#define CCID_Moment   							  124
#define CCID_Quantile   						  125
#define CCID_Standard_Deviation					  126
#define CCID_Variance							  127
#define CCID_Factor_in_ring						  128
#define CCID_Solve_for_Subexpression   			  129
#define CCID_Solve_System						  130
#define CCID_PassThru                             131

#define CCID_Define								  150
#define CCID_Undefine							  151
#define CCID_Cleanup							  152
#define CCID_DefineMupadName        153
#define CCID_Fixup  							  154
#define CCID_Interpret							155
#define CCID_PlotFuncCmd            156
#define CCID_PlotFuncQuery          157
#define CCID_GetVariables           158
#define CCID_Last    							  159

// Result codes carried by a "MathResult" object.
// Make into a type at some point?

#define CR_undefined            0

#define CR_failure              1
#define CR_ScriptOpenFailed     2
#define CR_ScriptNoEngineDbase  3
#define CR_NoEntitiesDbase      4
#define CR_NoUserPrefsDbase     5
#define CR_EngDbaseOpenFailed   6
#define CR_EngInitFailed        7
#define CR_unsupported_command  8

#define CR_nosol                10
#define CR_noDefineReserved     11
#define CR_missingBoundaries    12

#define CR_needvars             100
#define CR_queryindepvar        101
#define CR_badindepvar          102
#define CR_undefinedfunc        103
#define CR_badintegral          104
#define CR_badsolvesystem       105
#define CR_NeedSubInterp        106
#define CR_baddefformat         107
#define CR_badplotformat        108

// results > 9000  and < 10000 indicate bad params

#define CR_success              10000
#define CR_numericint           10001
#define CR_ringfactor           10002
#define CR_bool_true            10003
#define CR_bool_false           10004
#define CR_EqCheck_true         10005
#define CR_EqCheck_false        10006
#define CR_EqCheck_undecidable  10007

// Successful results, caller can query for parts, if desired

#define CR_quantile             10100
#define CR_eigenvectors         10101

// enum the following?
#define PID_ncolsReshape        1
#define PID_needvarresult       2
#define PID_independentvar      3
#define PID_VecBasisVariables   4
#define PID_PolyCollectVar		5
#define PID_moment_num          6
#define PID_moment_origin       7
#define PID_moment_meanisorigin 8
#define PID_quantile            9
#define PID_dependentvars       10
#define PID_seriesorder         11
#define PID_seriesvar           12
#define PID_seriesabout         13
#define PID_ODEseriesorder      14
#define PID_ODEIndepVar         15
#define PID_ODEseriesabout      16
#define PID_iterfunc            17
#define PID_iterstart           18
#define PID_itercount           19
#define PID_SolveForSubExpr     20
#define PID_sem2                21
#define PID_approxintform       22
#define PID_approxintnsubs      23
#define PID_dependcolumn        24
#define PID_regresscode         25
#define PID_regressdegree       26
#define PID_CalcByPartsVar      27
#define PID_CalcChangeVar       28
#define PID_FillMatrixIlk       29
#define PID_FillMatrixnRows     30
#define PID_FillMatrixnCols     31
#define PID_FillMatrixExtra     32
#define PID_RandMatrixType      33
#define PID_RandMatrixnRows     34
#define PID_RandMatrixnCols     35
#define PID_RandMatrixLLim      36
#define PID_RandMatrixULim      37
#define PID_RandomNumTally      38
#define PID_RandomNumDist       39
#define PID_RandomNumParam1     40
#define PID_RandomNumParam2     41
#define PID_ImplDiffIndepVar    42
#define PID_ImplDiffDepVars     43
#define PID_MapsFunction        44
#define PID_subInterpretation   45
// Plot-specific types: Add entry to PlotServiceRequest::NameToPID
// For each image ...
#define PID_GraphFirstParam      46
#define PID_GraphImageFile       46
#define PID_GraphXAxisLabel      47
#define PID_GraphYAxisLabel      48
#define PID_GraphZAxisLabel      49
#define PID_GraphWidth           50
#define PID_GraphHeight          51
#define PID_GraphPrintAttribute  52
#define PID_GraphPlacement       53
#define PID_GraphOffset          54
#define PID_GraphFloat           55
#define PID_GraphPrintFrame      56
#define PID_GraphKey             57
#define PID_GraphName            58
#define PID_GraphCaptionText     59
#define PID_GraphCaptionPlace    60
#define PID_GraphUnits           61
#define PID_GraphAxesType	     62
#define PID_GraphEqualScaling    63
#define PID_GraphEnableTicks	 64
#define PID_GraphXTickCount		 65
#define PID_GraphYTickCount		 66
#define PID_GraphZTickCount		 67
#define PID_GraphAxesTips		 68
#define PID_GraphGridLines		 69
#define PID_GraphBGColor		 70
#define PID_GraphDimension       71
#define PID_GraphAxesScaling     72
#define PID_GraphViewBoxXMin     73
#define PID_GraphViewBoxXMax     74
#define PID_GraphViewBoxYMin     75
#define PID_GraphViewBoxYMax     76
#define PID_GraphViewBoxZMin     77
#define PID_GraphViewBoxZMax     78
#define PID_GraphCameraLocationX 79
#define PID_GraphCameraLocationY 80
#define PID_GraphCameraLocationZ 81
#define PID_GraphFocalPointX     82
#define PID_GraphFocalPointY     83
#define PID_GraphFocalPointZ     84
#define PID_GraphUpVectorX       85
#define PID_GraphUpVectorY       86
#define PID_GraphUpVectorZ       87
#define PID_GraphViewingAngle    88
#define PID_GraphOrthogonalProjection    89
#define PID_GraphKeepUp           90
#define PID_GraphFontFamily		    91
#define PID_GraphFontSize         92
#define PID_GraphFontColor        93
#define PID_GraphFontBold         94
#define PID_GraphFontItalic       95
#define PID_GraphTicksFontFamily  96
#define PID_GraphTicksFontSize    97
#define PID_GraphTicksFontColor   98
#define PID_GraphTicksFontBold    99
#define PID_GraphTicksFontItalic 100
// For each plot ...
#define PID_PlotFirstParam          110
//#define PID_PlotStatus              101
#define PID_PlotType                112
#define PID_PlotLineStyle           113
#define PID_PlotLineThickness       114
#define PID_PlotLineColor           115
#define PID_PlotExpression          116
#define PID_PlotXMin                117
#define PID_PlotXMax                118
#define PID_PlotYMin                119
#define PID_PlotYMax                120
#define PID_PlotZMin                121
#define PID_PlotZMax                122
#define PID_PlotAnimMin             123
#define PID_PlotAnimMax             124
#define PID_PlotXVar                125
#define PID_PlotYVar                126
#define PID_PlotZVar                127
#define PID_PlotAnimVar             128
#define PID_PlotXPts                129
#define PID_PlotYPts                130
#define PID_PlotZPts                131
#define PID_PlotDiscAdjust          132
#define PID_PlotDirectionalShading  133
#define PID_PlotBaseColor		        134
#define PID_PlotSecondaryColor	    135
#define PID_PlotPointSymbol		      136
#define PID_PlotIncludePoints	      137
#define PID_PlotSurfaceStyle	      138
#define PID_PlotSurfaceMesh		      139
#define PID_PlotAnimationVariable   140
#define PID_PlotCameraLocationX	    141
#define PID_PlotCameraLocationY	    142
#define PID_PlotCameraLocationZ	    143
#define PID_PlotIncludeLines	      144
#define PID_PlotTubeRadius  	      145
#define PID_PlotTubeRadialPts       146
#define PID_PlotAISubIntervals 	    147
#define PID_PlotAIMethod     	      148
#define PID_PlotAIInfo      	      149
#define PID_PlotFillPattern    	    150
#define PID_PlotAnimate             151 
#define PID_PlotAnimateStart   	    152
#define PID_PlotAnimateEnd   	      153
#define PID_PlotAnimateFPS  	      154
#define PID_PlotAnimateVisBefore    155
#define PID_PlotAnimateVisAfter     156
#define PID_PlotConfHorizontalPts   157
#define PID_PlotConfVerticalPts     158

#define PID_approxintlowerbound     159
#define PID_approxintupperbound     160

#define PID_textPlotPositionType             161
#define PID_textPlotOrientationCartesian     162
#define PID_textPlotText                     163
#define PID_textPlotPositionX                164
#define PID_textPlotPositionY                165
#define PID_textPlotPositionZ                166
#define PID_textPlotOrientationX             167
#define PID_textPlotOrientationY             168
#define PID_textPlotOrientationZ             169
#define PID_textPlotTurn                     170
#define PID_textPlotTilt                     171
#define PID_textPlotHorizontalAlignment      172
#define PID_textPlotVerticalAlignment        173
#define PID_textPlotBillboarding             174
#define PID_textPlotTextFontFamily           175
#define PID_textPlotTextFontSize             176
#define PID_textPlotTextFontColor            177
#define PID_textPlotTextFontBold             178
#define PID_textPlotTextFontItalic           179
#define PID_mupname                          180
#define PID_mupnameloc                       181
#define PID_last                    182
#define PID_first_badparam        9000

// Data types for strings passed into ComputeDLL

#define zPT_ASCII_natural       1
#define zPT_ASCII_mmlmarkup     2
#define zPT_ASCII_text          3
#define zPT_ASCII_real          4

#define zPT_WIDE_natural        11
#define zPT_WIDE_mmlmarkup      12
#define zPT_WIDE_text           13
#define zPT_WIDE_real           14

// stuff attached to a result
#define RES_RESULT              1
#define RES_SENT                2
#define RES_RECEIVED            3
#define RES_ERROR               4
#define RES_SEMANTICS           5

#define EID_SeriesOrder         1
#define EID_Digits              2
#define EID_MaxDegree           3
#define EID_PvalOnly            4
#define EID_IgnoreSCases        5
#define EID_FloatFormat         6
#define EID_VarType             7
#define EID_Last                8

#define ENG_Name                1
#define ENG_engpath             2
#define ENG_libp                3
#define ENG_champath            4
#define ENG_wrapperDLL          5
#define ENG_dbase               6
#define ENG_entity_dbase        7
#define ENG_vcampath            8

// IDs for client attributes

#define CLPF_mml_version                1
#define CLPF_mml_prefix                 2
#define CLPF_math_node_attrs            3
#define CLPF_use_mfenced                4
#define CLPF_clr_math_attr              5
#define CLPF_clr_func_attr              6
#define CLPF_clr_text_attr              7
#define CLPF_clr_unit_attr              8
#define CLPF_Sig_digits_rendered        9
#define CLPF_SciNote_lower_thresh       10
#define CLPF_SciNote_upper_thresh       11
#define CLPF_Primes_as_n_thresh         12
#define CLPF_Parens_on_trigargs         13
#define CLPF_log_is_base_e              14
#define CLPF_Output_differentialD       15
#define CLPF_Output_differentiald       16
#define CLPF_Output_Euler_e             17
#define CLPF_Output_imaginaryi          18
#define CLPF_Output_InvTrigFuncs_1      19
#define CLPF_Output_Mixed_Numbers       20
#define CLPF_Output_parens_mode         21
#define CLPF_Default_matrix_delims      22
#define CLPF_Default_derivative_format  23
#define CLPF_Dot_derivative             24
#define CLPF_Overbar_conjugate          25
#define CLPF_Input_i_Imaginary          26
#define CLPF_Input_j_Imaginary          27
#define CLPF_Input_e_Euler              28
#define CLPF_Vector_basis               29
#define CLPF_Prime_derivative           30
#define CLPF_D_derivative               31
#define CLPF_last                       32

// IDs for components of structured results

#define CPID_WHICH_QUANTILE             101
#define CPID_QUANTILE_RESULT            102

#define CPID_EIGENVALUE                 103
#define CPID_EVAL_MULTIPLICITY          104
#define CPID_EIGENVECTOR                105

#endif
