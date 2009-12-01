// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

#define  FCN_FIRST_FUNCTION         8000 //dup
#define  FCN_arccos                 8000
#define  FCN_arcsin                 8001
#define  FCN_arctan                 8002
#define  FCN_arg                    8003
#define  FCN_cos                    8004
#define  FCN_cosh                   8005
#define  FCN_cot                    8006
#define  FCN_coth                   8007
#define  FCN_csc                    8008
#define  FCN_deg                    8009
#define  FCN_det                    8010
#define  FCN_dim                    8011
#define  FCN_exp                    8012
#define  FCN_gcd                    8013
#define  FCN_hom                    8014
#define  FCN_inf                    8015
#define  FCN_injlim                 8016
#define  FCN_ker                    8017
#define  FCN_lg                     8018
#define  FCN_lim                    8019
#define  FCN_liminf                 8020
#define  FCN_limsup                 8021
#define  FCN_ln                     8022
#define  FCN_log                    8023
#define  FCN_max                    8024
#define  FCN_min                    8025
#define  FCN_Pr                     8026
#define  FCN_projlim                8027
#define  FCN_sec                    8028
#define  FCN_sin                    8029
#define  FCN_sinh                   8030
#define  FCN_sup                    8031
#define  FCN_tan                    8032
#define  FCN_tanh                   8033
#define  FCN_varinjlim              8034
#define  FCN_varliminf              8035
#define  FCN_varlimsup              8036
#define  FCN_varprojlim             8037 //dup
#define  FCN_LAST_TEXFUNC           8037

#define  FCN_arccosh                8038
#define  FCN_arccot                 8039
#define  FCN_arccoth                8040
#define  FCN_arccsc                 8041
#define  FCN_arccsch                8042
#define  FCN_arcsec                 8043
#define  FCN_arcsech                8044
#define  FCN_arcsinh                8045
#define  FCN_arctanh                8046
#define  FCN_beta                   8047
#define  FCN_csch                   8048
#define  FCN_gamma                  8049
#define  FCN_sech                   8050
#define  FCN_zeta                   8051
#define  FCN_binomial               8052
#define  FCN_Si                     8053
#define  FCN_Ci                     8054
#define  FCN_Ei                     8055
#define  FCN_dilog                  8056
#define  FCN_abs                    8057

#define  FCN_norm                   8058
#define  FCN_erf                    8059
#define  FCN_erfc                   8060
#define  FCN_euler                  8061
#define  FCN_lcm                    8062
#define  FCN_mod                    8063
#define  FCN_signum                 8064
#define  FCN_csgn                   8065
#define  FCN_hypergeom              8066
#define  FCN_about                  8067
#define  FCN_additionally           8068
#define  FCN_assume                 8069
#define  FCN_unassume               8070
#define  FCN_AndProp                8071
#define  FCN_Re                     8072
#define  FCN_Im                     8073
#define  FCN_floor                  8074
#define  FCN_ceil                   8075
#define  FCN_pow                    8076
#define  FCN_DirInf                 8077
#define  FCN_rank                   8078

#define  FCN_FIRST_STATOP           8079 //dup
#define  FCN_tcicorr                8079
#define  FCN_tcicovar               8080
#define  FCN_tcimean                8081
#define  FCN_tcimedian              8082
#define  FCN_tcimode                8083
#define  FCN_tcigeometricmean       8084
#define  FCN_tciharmonicmean        8085
#define  FCN_tcisdev                8086
#define  FCN_tcimdev                8087
#define  FCN_tcivariance            8088 //dup
#define  FCN_LAST_STATOP            8088

#define  FCN_FIRST_STAT             8089 //dup
#define  FCN_BetaDist               8089
#define  FCN_BetaDen                8090
#define  FCN_BetaInv                8091
#define  FCN_BinomialDist           8092
#define  FCN_BinomialDen            8093
#define  FCN_BinomialInv            8094
#define  FCN_CauchyDist             8095
#define  FCN_CauchyDen              8096
#define  FCN_CauchyInv              8097
#define  FCN_ChiSquareDist          8098
#define  FCN_ChiSquareDen           8099
#define  FCN_ChiSquareInv           8100
#define  FCN_ExponentialDist        8101
#define  FCN_ExponentialDen         8102
#define  FCN_ExponentialInv         8103
#define  FCN_FDist                  8104
#define  FCN_FDen                   8105
#define  FCN_FInv                   8106
#define  FCN_GammaDist              8107
#define  FCN_GammaDen               8108
#define  FCN_GammaInv               8109
#define  FCN_HypergeomDist          8110
#define  FCN_HypergeomDen           8111
#define  FCN_HypergeomInv           8112
#define  FCN_NormalDist             8113
#define  FCN_NormalDen              8114
#define  FCN_NormalInv              8115
#define  FCN_PoissonDist            8116
#define  FCN_PoissonDen             8117
#define  FCN_PoissonInv             8118
#define  FCN_TDist                  8119
#define  FCN_TDen                   8120
#define  FCN_TInv                   8121
#define  FCN_UniformDist            8122
#define  FCN_UniformDen             8123
#define  FCN_UniformInv             8124
#define  FCN_WeibullDist            8125
#define  FCN_WeibullDen             8126
#define  FCN_WeibullInv             8127 //dup
#define  FCN_LAST_STAT              8127

#define  FCN_Ai                     8128
#define  FCN_Bi                     8129
#define  FCN_BesselI                8130
#define  FCN_BesselK                8131
#define  FCN_BesselJ                8132
#define  FCN_BesselY                8133
#define  FCN_FresnelC               8134
#define  FCN_FresnelS               8135
#define  FCN_Fresnelf               8136
#define  FCN_Fresnelg               8137
#define  FCN_EllipticE              8138
#define  FCN_EllipticF              8139
#define  FCN_MeijerG                8140
#define  FCN_Psi                    8141
#define  FCN_LT                     8142
#define  FCN_ILT                    8143
#define  FCN_FT                     8144
#define  FCN_IFT                    8145
#define  FCN_order                  8146
#define  FCN_comp                   8147
#define  FCN_rand                   8148
#define  FCN_setseed                8149
#define  FCN_setassigns             8150
#define  FCN_setconditions          8151
#define  FCN_satisfy                8152
#define  FCN_randmat                8153
#define  FCN_LambertW               8154
#define  FCN_FTcos                  8155
#define  FCN_FTsin                  8156
#define  FCN_conjugate              8157
#define  FCN_distinct               8158
#define  FCN_LommelS1               8159
#define  FCN_LommelS2               8160
#define  FCN_bernoulli              8161
#define  FCN_Chi                    8162
#define  FCN_Shi                    8163
#define  FCN_polylog                8164
#define  FCN_Dirac                  8165
#define  FCN_Heaviside              8166
#define  FCN_seq                    8167

// put here to not interfere with next group
#define  CONST_real                 8168
#define  CONST_complex              8169
#define  CONST_integer              8170
#define  CONST_negative             8171
#define  CONST_positive             8172
#define  CONST_nonzero              8173
#define  CONST_EULERgamma           8174
//                                    STOP

#define  FCN_SPECIALCATEGORIES      8994  //dup
#define  FCN_MATFUNC                8994
#define  FCN_VECFUNC                8995
#define  FCN_MAPLEFUNC              8996
#define  FCN_GENFUNC                8997
#define  FCN_USERFUNCTION           8998
#define  FCN_LAST_FUNCTION          8999

#define  CONST_pi                   9000
#define  CONST_infinity             9001
#define  CONST_true                 9002
#define  CONST_false                9003
#define  CONST_twodots              9004
#define  CONST_at                   9005
#define  CONST_proc                 9006
#define  CONST_fail                 9007
#define  CONST_QSUB                 9008
#define  CONST_ESUB                 9009
#define  CONST_posdef               9010
#define  CONST_negdef               9011
#define  CONST_psdef                9012
#define  CONST_nsdef                9013
#define  CONST_indef                9014

#define  MPL_FIRST_DEFINITION      10000 //dup
#define  MPL_Float                 10000
#define  MPL_product               10001
#define  MPL_sum                   10002
#define  MPL_diff                  10003
#define  MPL_int                   10004
#define  MPL_Int                   10005
#define  MPL_RootOf                10006
#define  MPL_MATRIX                10007
#define  MPL_matrix                10008
#define  MPL_singularvals          10009
#define  MPL_simplify              10010
#define  MPL_factor                10011
#define  MPL_randmatrix            10012
#define  MPL_Sum                   10013
#define  MPL_simplex               10014
#define  MPL_lim                   10015
#define  MPL_factorial             10016
#define  MPL_series                10017
#define  MPL_surd                  10018
#define  MPL_arg                   10019
#define  MPL_array                 10020
#define  MPL_rewrite               10021
#define  MPL_interval              10022
#define  MPL_expand                10023
#define  MPL_solve                 10024
#define  MPL_istrue                10025
#define  MPL_row                   10026
#define  MPL_col                   10027
//                                   STOP

#define  MPL_tcipwr                11000
#define  MPL_tciadd                11001
#define  MPL_tcimpy                11002
#define  MPL_add                   11003
#define  MPL_multiply              11004
#define  MPL_scalarmul             11005
#define  MPL_subs                  11006
#define  MPL_eval                  11007
#define  MPL_VECTOR                11008
#define  MPL_vector                11009
#define  MPL_curl                  11010
#define  MPL_diverge               11011
#define  MPL_grad                  11012
#define  MPL_tcixprod              11013
#define  MPL_tcidprod              11014
#define  MPL_crossprod             11015
#define  MPL_dotprod               11016
#define  MPL_tcikill               11017
#define  MPL_tciarg                11018
#define  MPL_claplace              11019
#define  MPL_cnumeric              11020
#define  MPL_cseries               11021
#define  MPL_ode                   11022
#define  MPL_cpolar                11023
#define  MPL_evalf                 11024
#define  MPL_setdigits             11025
#define  MPL_setseriesorder        11026
#define  MPL_startquote            11027
#define  MPL_endquote              11028
#define  MPL_rsolve                11029
#define  MPL_badplot               11030
#define  MPL_function              11031
#define  MPL_tcigrad               11032
#define  MPL_laplacian             11033
#define  MPL_intersect             11034
#define  MPL_minus                 11035
#define  MPL_union                 11036
#define  MPL_PIECEWISE             11037
#define  MPL_left                  11038
#define  MPL_right                 11039
#define  MPL_frobenius             11040
#define  MPL_infinitynorm          11041
#define  MPL_exponential           11042
#define  MPL_floatformat           11043
#define  MPL_in                    11044
#define  MPL_and                   11045
#define  MPL_or                    11046
#define  MPL_not                   11047
#define  MPL_integers              11048
#define  MPL_rationalnumbers       11049
#define  MPL_realnumbers           11050
#define  MPL_complexnumbers        11051
#define  MPL_withpackage           11052
#define  MPL_powermod              11053
//                                   STOP               

#define COMPUTE_SENDSTRING                  20000
#define COMPUTE_EVALUATE                    20001
#define COMPUTE_EVALUATENUM                 20002
#define COMPUTE_SIMPLIFY                    20003
#define COMPUTE_EXPAND                      20004
#define COMPUTE_FACTOR                      20005
#define COMPUTE_REWRITE                     20006
#define COMPUTE_SUBSTITUTE                  20007
#define COMPUTE_ROOTS                       20008
#define COMPUTE_LAPLACE                     20009
#define COMPUTE_MATRIX_ADJOINT              20010
#define COMPUTE_MATRIX_BASIS                20011
#define COMPUTE_MATRIX_EIGENVALS            20012
#define COMPUTE_MATRIX_EIGENVECTS           20013
#define COMPUTE_MATRIX_INVERSE              20014
#define COMPUTE_MATRIX_RANK                 20015
#define COMPUTE_MATRIX_DETERMINANT          20016
#define COMPUTE_MATRIX_TRANSPOSE            20017
#define COMPUTE_MATRIX_CHOLESKY             20018
#define COMPUTE_MATRIX_COLBASIS             20019
#define COMPUTE_MATRIX_CONDNUM              20020
#define COMPUTE_MATRIX_DEFTESTS             20021
#define COMPUTE_MATRIX_FFGAUSS              20022
#define COMPUTE_MATRIX_FILLMATRIX           20023
#define COMPUTE_MATRIX_FROBENIUS            20024
#define COMPUTE_MATRIX_GAUSS                20025
#define COMPUTE_MATRIX_HERMITE              20026
#define COMPUTE_MATRIX_HERMITIANTRANSPOSE   20027
#define COMPUTE_MATRIX_JORDAN               20028
#define COMPUTE_MATRIX_LU                   20029
#define COMPUTE_MATRIX_MINPOLY              20030
#define COMPUTE_MATRIX_NORM                 20031
#define COMPUTE_MATRIX_NULLSPACEBASIS       20032
#define COMPUTE_MATRIX_ORTHOGTEST           20033
#define COMPUTE_MATRIX_PERMANENT            20034
#define COMPUTE_MATRIX_CONCATENATE          20035
#define COMPUTE_MATRIX_STACK                20036
#define COMPUTE_MATRIX_CHARPOLY             20037
#define COMPUTE_MATRIX_REDROWECHELON        20038
#define COMPUTE_MATRIX_ROWBASIS             20039
#define COMPUTE_MATRIX_SINGULARVALUES       20040
#define COMPUTE_MATRIX_SMITH                20041
#define COMPUTE_MATRIX_SPECTRAL             20042
#define COMPUTE_MATRIX_ISMITH               20043
#define COMPUTE_MATRIX_TRACE                20044
#define COMPUTE_MATRIX_SVD                  20045
#define COMPUTE_MATRIX_RANDOM               20046
#define COMPUTE_MATRIX_RESHAPE              20047
#define COMPUTE_MATRIX_QR                   20048
#define COMPUTE_EVALUATECPLX                20049

#define COMPUTE_COMBINE                     20050
#define COMPUTE_COMBINE_EXPONENTIALS        20051
#define COMPUTE_COMBINE_LOGS                20052
#define COMPUTE_COMBINE_POWERS              20053
#define COMPUTE_COMBINE_PSIFNS              20054
#define COMPUTE_COMBINE_TRIGFNS             20055

#define COMPUTE_REWRITE_EQNSTOMATRIX        20056
#define COMPUTE_REWRITE_MATRIXTOEQNS        20057

#define COMPUTE_POLY_CONTENT                20058
#define COMPUTE_POLY_DEGREE                 20059
#define COMPUTE_POLY_QUOTIENT               20060
#define COMPUTE_POLY_REMAINDER              20061
#define COMPUTE_POLY_ROOTS                  20062
#define COMPUTE_POLY_DIVIDE                 20063
#define COMPUTE_POLY_SORT                   20064
#define COMPUTE_POLY_COLLECT                20065
#define COMPUTE_POLY_COMPANION              20066

#define COMPUTE_CHECKEQUALITY               20067

#define COMPUTE_SOLVE_EXACT                 20068
#define COMPUTE_SOLVE_INTEGER               20069
#define COMPUTE_SOLVE_NUMERIC               20070

#define COMPUTE_SOLVEODE_EXACT              20071
#define COMPUTE_SOLVEODE_LAPLACE            20072
#define COMPUTE_SOLVEODE_NUMERIC            20073
#define COMPUTE_SOLVEODE_SERIES             20074
#define COMPUTE_SOLVEPDE_EXACT              20075

#define COMPUTE_SOLVEREC_EXACT              20076
#define COMPUTE_SOLVEREC_ZTRANS             20077
#define COMPUTE_SOLVEREC_ITERATE            20078
#define COMPUTE_SOLVEREC_ASYMP              20079

#define COMPUTE_DEFINE                      20080
#define COMPUTE_UNDEFINE                    20081
#define COMPUTE_ASSUME                      20082
#define COMPUTE_ACTIVATE_DEFS               20083
#define COMPUTE_DEFINE_SAVEDEFS             20084
#define COMPUTE_DEFINE_CLEARDEFS            20085
#define COMPUTE_DEFINE_MAPLENAME            20086

#define COMPUTE_SERIES                      20087

#define COMPUTE_CALCULUS_INTPARTS           20088
#define COMPUTE_CALCULUS_CHANGEVAR          20089
#define COMPUTE_CALCULUS_PARFRAC            20090
#define COMPUTE_CALCULUS_EXTREMA            20091
#define COMPUTE_CALCULUS_AREA               20092
#define COMPUTE_CALCULUS_AREA_LEFTBOXES     20093
#define COMPUTE_CALCULUS_AREA_MIDPOINT      20094
#define COMPUTE_CALCULUS_AREA_RIGHTBOXES    20095
#define COMPUTE_CALCULUS_AREA_SIMPSON       20096
#define COMPUTE_CALCULUS_AREA_TRAPEZOID     20097
#define COMPUTE_CALCULUS_ITERATE            20098
#define COMPUTE_CALCULUS_DIFFIMP            20099

#define COMPUTE_SIMPLEX_BASIS               20100
#define COMPUTE_SIMPLEX_HULL                20101
#define COMPUTE_SIMPLEX_DUAL                20102
#define COMPUTE_SIMPLEX_FEASIBLE            20103
#define COMPUTE_SIMPLEX_MAXIMIZE            20104
#define COMPUTE_SIMPLEX_MINIMIZE            20105
#define COMPUTE_SIMPLEX_STANDARD            20106

#define COMPUTE_FIRST_STAT                  20107 //dup
#define COMPUTE_STAT_CORRELATION            20107
#define COMPUTE_STAT_COVARIANCE             20108
#define COMPUTE_STAT_MEAN                   20109
#define COMPUTE_STAT_MEDIAN                 20110
#define COMPUTE_STAT_MODE                   20111
#define COMPUTE_STAT_GEOMETRICMEAN          20112
#define COMPUTE_STAT_HARMONICMEAN           20113
#define COMPUTE_STAT_MULTIREGRESS           20114
#define COMPUTE_STAT_MULTIREGRESSNOC        20115
#define COMPUTE_STAT_RANDOM                 20116
#define COMPUTE_STAT_REGRESSION             20117
#define COMPUTE_STAT_STDDEV                 20118
#define COMPUTE_STAT_MEANDEV                20119
#define COMPUTE_STAT_VARIANCE               20120
#define COMPUTE_STAT_MOMENT                 20121
#define COMPUTE_STAT_QUANTILE               20122
#define COMPUTE_STAT_POLYFIT                20123
#define COMPUTE_STAT_SPECIALFIT             20124 //dup
#define COMPUTE_LAST_STAT                   20124

#define COMPUTE_TRANS_FOURIER               20125
#define COMPUTE_TRANS_INVFOURIER            20126
#define COMPUTE_TRANS_FFT                   20127
#define COMPUTE_TRANS_INVFFT                20128
#define COMPUTE_TRANS_LAPLACE               20129
#define COMPUTE_TRANS_INVLAPLACE            20130
#define COMPUTE_TRANS_MELLIN                20131
#define COMPUTE_TRANS_Z                     20132
#define COMPUTE_TRANS_INVZ                  20133

#define FIRST_VEC_OP                        20134 //dup
#define COMPUTE_VEC_GRADIENT                20134
#define COMPUTE_VEC_DIVERGENCE              20135
#define COMPUTE_VEC_CURL                    20136
#define COMPUTE_VEC_LAPLACIAN               20137
#define COMPUTE_VEC_JACOBIAN                20138
#define COMPUTE_VEC_HESSIAN                 20139
#define COMPUTE_VEC_SCALARPOTENT            20140
#define COMPUTE_VEC_VECTORPOTENT            20141 //dup
#define LAST_VEC_OP                         20141
// Wronskian doesn't use the basis variables
#define COMPUTE_VEC_WRONSKIAN               20142
#define COMPUTE_VEC_SETBASIS                20143

#define FIRST_TRIG_OP                       20144 //dup
#define COMPUTE_TRIG_EXPAND                 20144
#define COMPUTE_TRIG_FACTOR                 20145
#define COMPUTE_TRIG_REDUCE                 20146
#define COMPUTE_TRIG_SIMPLIFY               20147
#define COMPUTE_TRIG_REWRITE                20148
#define COMPUTE_TRIGTOCOMPLEX               20149
#define COMPUTE_COMPLEXTOTRIG               20150 //dup
#define LAST_TRIG_OP                        20150

// Plots
#define COMPUTE_FIRST_PLOT                  20151 //dup
# define COMPUTE_PLOT_LBOX                  20151
# define COMPUTE_PLOT_RBOX                  20152
# define COMPUTE_PLOT_MBOX                  20153
# define COMPUTE_PLOT_2D                    20154
# define COMPUTE_PLOT_3D                    20155
# define COMPUTE_PLOT_ODE                   20156
# define COMPUTE_PLOT_PHASE                 20157
# define COMPUTE_PLOT_POLAR                 20158
# define COMPUTE_PLOT_CONFORMAL             20159
# define COMPUTE_PLOT_GRADIENT              20160
# define COMPUTE_PLOT_VECTORFIELD           20161
# define COMPUTE_PLOT_IMPLICIT              20162
# define COMPUTE_PLOT_TUBE                  20163
# define COMPUTE_PLOT_CYLINDER              20164
# define COMPUTE_PLOT_SPHERE                20165
# define COMPUTE_PLOT_GRADIENT3D            20166
# define COMPUTE_PLOT_FIELD3D               20167
# define COMPUTE_PLOT_IMPLICIT3D            20168
# define COMPUTE_PLOT_PARAMETRIC            20169
# define COMPUTE_PLOT_LOG                   20170
# define COMPUTE_PLOT_LOGLOG                20171
# define COMPUTE_PLOT_PARAMPOLAR            20172
# define COMPUTE_PLOT_SPACECURVE            20173
# define COMPUTE_PLOT_PARAMSURFACE          20174
# define COMPUTE_PLOT_LISTPLOT              20175 //dup
#define COMPUTE_LAST_PLOT                   20175

#define COMPUTE_ABOUT                       20176
#define COMPUTE_NOTHINGKNOWN                20177
#define INTERPRET_MATHFORM                  20178

#define COMPUTE_SETPLOTVARS                 20179
#define COMPUTE_PARSEONLY                   20180
//                                            STOP

//Preserve the ordering and spacing of the following ids
//which have corresponding ids in unitids.h

#define UNIT_FIRST_FACTOR                   24500 //dup
#define UNIT_FACTOR_in                      24500
#define UNIT_FACTOR_ft                      24501
#define UNIT_FACTOR_mi                      24502
#define UNIT_FACTOR_km                      24503
#define UNIT_FACTOR_m                       24504
#define UNIT_FACTOR_cm                      24505
#define UNIT_FACTOR_mm                      24506
#define UNIT_FACTOR_micro_m                 24507
#define UNIT_FACTOR_nano_m                  24508
#define UNIT_FACTOR_angstrom                24509
#define UNIT_FACTOR_pico_m                  24510
#define UNIT_FACTOR_femto_m                 24511
#define UNIT_FACTOR_atto_m                  24512
#define UNIT_FACTOR_dm                      24513

#define UNIT_FACTOR_g                       24600
#define UNIT_FACTOR_kg                      24601
#define UNIT_FACTOR_mg                      24602
#define UNIT_FACTOR_micro_g                 24603
#define UNIT_FACTOR_amu                     24604
#define UNIT_FACTOR_cg						24605
#define UNIT_FACTOR_dg                      24606
#define UNIT_FACTOR_lb                      24607
#define UNIT_FACTOR_slug                    24608

#define UNIT_FACTOR_d                       24700
#define UNIT_FACTOR_h                       24701
#define UNIT_FACTOR_mn                      24702
#define UNIT_FACTOR_s                       24703
#define UNIT_FACTOR_y                       24704
#define UNIT_FACTOR_ms                      24705
#define UNIT_FACTOR_micro_s                 24706
#define UNIT_FACTOR_nano_s                  24707
#define UNIT_FACTOR_pico_s                  24708
#define UNIT_FACTOR_femto_s                 24709
#define UNIT_FACTOR_atto_s                  24710

#define UNIT_FACTOR_lbf                     24800
#define UNIT_FACTOR_N                       24801
#define UNIT_FACTOR_dyne                    24802
#define UNIT_FACTOR_micro_N                 24803
#define UNIT_FACTOR_mN                      24804
#define UNIT_FACTOR_kN                      24805
#define UNIT_FACTOR_MN                      24806
#define UNIT_FACTOR_ozf                     24807

#define UNIT_FACTOR_J                       24900
#define UNIT_FACTOR_kcal                    24901
#define UNIT_FACTOR_Btu                     24902
#define UNIT_FACTOR_cal                     24903
#define UNIT_FACTOR_eV                      24904
#define UNIT_FACTOR_MeV                     24905
#define UNIT_FACTOR_GeV                     24906
#define UNIT_FACTOR_erg                     24907
#define UNIT_FACTOR_mJ                      24908
#define UNIT_FACTOR_micro_J                 24909
#define UNIT_FACTOR_nano_J                  24910
#define UNIT_FACTOR_kJ     					24911
#define UNIT_FACTOR_MJ						24912
#define UNIT_FACTOR_GJ                      24913

#define UNIT_FACTOR_W                       25000
#define UNIT_FACTOR_hp                      25001
#define UNIT_FACTOR_mW                      25002
#define UNIT_FACTOR_micro_W                 25003
#define UNIT_FACTOR_kW                      25004
#define UNIT_FACTOR_MW                      25005
#define UNIT_FACTOR_GW                      25006
#define UNIT_FACTOR_nano_W                  25007

#define UNIT_FACTOR_atm                     25100
#define UNIT_FACTOR_Pa                      25101
#define UNIT_FACTOR_bar                     25102
#define UNIT_FACTOR_torr                    25103
#define UNIT_FACTOR_mbar                    25104
#define UNIT_FACTOR_micro_Pa				25105
#define UNIT_FACTOR_MPa						25106
#define UNIT_FACTOR_kPa                     25107
#define UNIT_FACTOR_kbar                    25108
#define UNIT_FACTOR_mmHg                    25109

#define UNIT_FACTOR_A                       25200
#define UNIT_FACTOR_mA                      25201
#define UNIT_FACTOR_micro_A                 25202
#define UNIT_FACTOR_nano_A                  25203
#define UNIT_FACTOR_kA                      25204      

#define UNIT_FACTOR_F                       25300
#define UNIT_FACTOR_mF                      25301
#define UNIT_FACTOR_micro_F                 25302
#define UNIT_FACTOR_nano_F                  25303
#define UNIT_FACTOR_pico_F                  25304

#define UNIT_FACTOR_C                       25400

#define UNIT_FACTOR_H                       25500
#define UNIT_FACTOR_mH						25501
#define UNIT_FACTOR_micro_H					25502

#define UNIT_FACTOR_T                       25600
#define UNIT_FACTOR_G                       25601
#define UNIT_FACTOR_mT                      25602
#define UNIT_FACTOR_micro_T                 25603
#define UNIT_FACTOR_nano_T                  25604
#define UNIT_FACTOR_pico_T                  25605

#define UNIT_FACTOR_V                       25700
#define UNIT_FACTOR_mV                      25701
#define UNIT_FACTOR_micro_V                 25702
#define UNIT_FACTOR_nano_V                  25703
#define UNIT_FACTOR_pico_V                 	25704
#define UNIT_FACTOR_kV                      25705
#define UNIT_FACTOR_MV						25706

#define UNIT_FACTOR_DEG_C                   25800
#define UNIT_FACTOR_DEG_F                  	25801
#define UNIT_FACTOR_DEG_K                 	25802

#define UNIT_FACTOR_RADIAN                  25900
#define UNIT_FACTOR_MILLIRAD                25901
#define UNIT_FACTOR_MICRORAD                25902
#define UNIT_FACTOR_DEGREE                  25903
#define UNIT_FACTOR_ARCMIN                  25904
#define UNIT_FACTOR_ARCSEC                  25905

#define UNIT_FACTOR_STERAD                  26000 

#define UNIT_FACTOR_BECQUER                 26100
#define UNIT_FACTOR_CURIE                   26101 
                                             
#define UNIT_FACTOR_HZ                      26200
#define UNIT_FACTOR_KHZ                     26201
#define UNIT_FACTOR_MHZ                     26202
#define UNIT_FACTOR_GHZ                     26203
#define UNIT_FACTOR_THZ                     26204
#define UNIT_FACTOR_PHZ                     26205
#define UNIT_FACTOR_EHZ                     26206        
                                             
#define UNIT_FACTOR_WEBER                   26300
#define UNIT_FACTOR_MAXWELL                 26301
#define UNIT_FACTOR_MILLIWEBER              26302
#define UNIT_FACTOR_MICROWEBER              26303
#define UNIT_FACTOR_NANOWEBER               26304                                             
#define UNIT_FACTOR_CANDELA                 26400
                                             
#define UNIT_FACTOR_LUX                     26500
#define UNIT_FACTOR_FTCANDLE                26501
#define UNIT_FACTOR_PHOT                    26502
                                             
#define UNIT_FACTOR_LUMEN                   26600
                                             
#define UNIT_FACTOR_STILB                   26700
                                             
#define UNIT_FACTOR_OHM                     26800
#define UNIT_FACTOR_MILLIOHM                26801
#define UNIT_FACTOR_KOHM                    26802
#define UNIT_FACTOR_MOHM                    26803
#define UNIT_FACTOR_GOHM                    26804   
                                             
#define UNIT_FACTOR_MICROSIEMENS            26900
#define UNIT_FACTOR_MILLISIEMENS            26901
#define UNIT_FACTOR_KILOSIEMENS             26902
#define UNIT_FACTOR_SIEMENS                 26903

#define UNIT_FACTOR_MOLE                    27000
#define UNIT_FACTOR_amol                    27001
#define UNIT_FACTOR_pmol                    27002
#define UNIT_FACTOR_nmol                    27003
#define UNIT_FACTOR_mmol                    27004
#define UNIT_FACTOR_kmol                    27005
#define UNIT_FACTOR_Mmol                    27006
#define UNIT_FACTOR_Gmol                    27007
#define UNIT_FACTOR_Tmol                    27008
#define UNIT_FACTOR_Emol                    27009
#define UNIT_FACTOR_micro_mol               27010 
#define UNIT_FACTOR_fmol                    27011
#define UNIT_FACTOR_Pmol                    27012  

#define UNIT_FACTOR_SQUARE_METERS           27100
#define UNIT_FACTOR_SQUARE_FEET             27101
#define UNIT_FACTOR_ACRE                    27102
#define UNIT_FACTOR_SQUARE_INCHES           27103
#define UNIT_FACTOR_HECTARE                 27104

#define UNIT_FACTOR_CUBIC_METERS            27200
#define UNIT_FACTOR_CUBIC_FEET              27201
#define UNIT_FACTOR_CUBIC_INCHES            27202
#define UNIT_FACTOR_LITERS                  27203
#define UNIT_FACTOR_GALLONS                 27204
#define UNIT_FACTOR_QUART                   27205
#define UNIT_FACTOR_MILLILITERS             27206
#define UNIT_FACTOR_PINT					27207 //dup

#define UNIT_LAST_FACTOR                    27207



