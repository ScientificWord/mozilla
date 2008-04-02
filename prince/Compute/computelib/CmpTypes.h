// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

#ifndef CMPTYPES_H
#define CMPTYPES_H

#include "iCmpTypes.h"
#include "iCmpIDs.h"

#include "nsDebug.h"
// debugging help, should switch each to NS_ASSERTION
#define TCI_ASSERT(expr)  NS_ASSERTION((expr),"TCI_ASSERT")


enum MarkupType
{ MT_UNDEFINED, MT_MATHML, MT_LATEX,
  MT_MAPLEV_INPUT, MT_MUPAD_INPUT, MT_GRAPH
};

// Identifiers for the "semantic type" that we assign
//  to each node that we put in our "semantics" trees.
enum SemanticType {
  SEM_TYP_UNDEFINED,
  // Atomic
  SEM_TYP_VARIABLE,
  SEM_TYP_NUMBER,
  SEM_TYP_MIXEDNUMBER,
  SEM_TYP_FUNCTION,
  SEM_TYP_PREFIX_OP,
  SEM_TYP_INFIX_OP,
  SEM_TYP_POSTFIX_OP,
  SEM_TYP_UCONSTANT,
  SEM_TYP_BOOLEAN,
  SEM_TYP_SPACE,
  SEM_TYP_TEXT,
  SEM_TYP_INVFUNCTION,
  SEM_TYP_SIUNIT,
  SEM_TYP_USUNIT,
  SEM_TYP_QUALIFIED_VAR,
  SEM_TYP_INDEXED_VAR,
  SEM_TYP_ENG_PASSTHRU,
  SEM_TYP_EQCHECK_RESULT,
  // Schemata
  SEM_TYP_MATH_CONTAINER,
  SEM_TYP_PRECEDENCE_GROUP,
  SEM_TYP_PARENED_LIST,
  SEM_TYP_BRACKETED_LIST,
  SEM_TYP_PIECEWISE_FENCE,
  SEM_TYP_GENERIC_FENCE,
  SEM_TYP_QSUB_LIST,
  SEM_TYP_COMMA_LIST,
  SEM_TYP_INTERVAL,
  SEM_TYP_FRACTION,
  SEM_TYP_POWERFORM,
  SEM_TYP_SQRT,
  SEM_TYP_ROOT,
  SEM_TYP_BIGOP_SUM,
  SEM_TYP_BIGOP_INTEGRAL,
  SEM_TYP_TABULATION,
  SEM_TYP_LIMFUNC,
  SEM_TYP_BINOMIAL,
  SEM_TYP_SET,
  SEM_TYP_ABS,
  SEM_TYP_NORM,
  SEM_TYP_FLOOR,
  SEM_TYP_CEILING,
  SEM_TYP_CONJUGATE,
  SEM_TYP_MTRANSPOSE,
  SEM_TYP_HTRANSPOSE,
  SEM_TYP_SIMPLEX,
  SEM_TYP_NUMSYSTEM,
  SEM_TYP_SOLVESYSTEM,
  SEM_TYP_LIST,
  SEM_TYP_PIECEWISE_LIST,
  SEM_TYP_ONE_PIECE,
  SEM_TYP_VARDEF_DEFERRED,
  SEM_TYP_VARDEF,
  SEM_TYP_FUNCDEF,
  SEM_TYP_TENSOR,
  SEM_TYP_QUANTILE_RESULT,
  // "listoperators" take a variable length list of arguments - gcd, lcm, min, max
  SEM_TYP_LISTOPERATOR,
  SEM_TYP_DERIVATIVE,
  SEM_TYP_PARTIALD,
  SEM_TYP_PDE,
  SEM_TYP_ODE_SYSTEM,
  SEM_TYP_RECURSION,
  SEM_TYP_TRANSFORM,
  SEM_TYP_INVTRANSFORM,
  // Completely bogus types to handle contrived SWP notations
  //  that have "special" meaning for computations
  SEM_TYP_SUBSTITUTION,
  SEM_TYP_MULTIARGFUNC,
  // Output only types
  SEM_TYP_EIGENVECTORSET,
  SEM_TYP_EIGENVECTOR
};

enum SemanticVariant {
SNV_None,
SNV_ExclExclInterval,    //was 1
SNV_ExclInclInterval,    //was 2
SNV_InclExclInterval,    //was 3
SNV_InclInclInterval,    //was 4
SNV_True,
SNV_False,
SNV_Undecidable,
SNV_singleint,
SNV_doubleint,
SNV_tripleint,
SNV_quadint,
SNV_numericint          //was 100
};

// structs for "semantic" trees

typedef struct tagSEMANTICS_NODE SEMANTICS_NODE;

typedef struct tagBUCKET_REC
{
  tagBUCKET_REC *next;
  tagBUCKET_REC *parts;
  U32 bucket_ID;
  SEMANTICS_NODE *first_child;
} BUCKET_REC;

typedef struct tagSEMANTICS_NODE
{
  tagSEMANTICS_NODE *next;
  tagSEMANTICS_NODE *prev;
  BUCKET_REC *parent;
  U32 owner_ID;
  U32 error_flag;
  SemanticType semantic_type;
  char *canonical_ID;
  char *contents;
  U32 msi_class;
  SemanticVariant variant;
  int infix_precedence;
  U32 subtree_type;
  U32 nrows;
  U32 ncols;
  U32 n_sub_args;
  BUCKET_REC *bucket_list;
} SEMANTICS_NODE;


// Identifiers for Math buckets that occur within schemata
// In most cases Math buckets don't require an ID -
// there's only one or positional order is enough.

#define	MB_UNNAMED              1

#define	MB_NUMERATOR            2
#define	MB_DENOMINATOR          3
#define	MB_SCRIPT_BASE          4
#define	MB_SCRIPT_UPPER         5
#define	MB_SCRIPT_LOWER         6
#define	MB_ROOT_BASE            7
#define	MB_ROOT_EXPONENT        8

#define MB_LOWERLIMIT		    9
#define MB_UPPERLIMIT		    10
#define MB_OPERAND		        11
#define MB_INTEG_VAR		    12
#define MB_SUM_VAR              13

#define MB_EIGENVALUE		    14
#define MB_EV_MULTIPLICITY	    15
#define MB_EIGENVECTOR		    16
#define MB_SIMPLEX_EXPR         17

#define	MB_DEF_VARNAME          20
#define	MB_DEF_VARVALUE         21
#define	MB_DEF_FUNCHEADER       22
#define	MB_DEF_FUNCVALUE        23

#define	MB_MN_WHOLE             25
#define	MB_MN_FRACTION          26

#define MB_DIFF_VAR		        27
#define MB_NPRIMES		        28
#define MB_BOUNDARY_CONDITION   29

#define MB_SUBST_LOWER	        31
#define MB_SUBST_UPPER	        32

#define MB_PIECE_EXPR           33

#define MB_LL_VAR               34
#define MB_LL_EXPR              39
#define MB_LL_DIRECTION         40

#define MB_LOG_BASE             41

#define MB_NSYS_EQUATION        42
#define MB_NSYS_CONDITION       43

#define MB_RECUR_EQN            44
#define MB_RECUR_FUNC           45

#define MB_BASE_VARIABLE        46
#define MB_SUB_QUALIFIER        47
#define MB_ODEFUNC              48

#define MB_INTERVAL_START       49
#define MB_INTERVAL_END         50
#define MB_INTERVAL_VAR         51

#define MB_NORM_NUMBER          52

#define MB_FUNC_EXPONENT	    53

#define MB_SYSTEM_COEFFICIENTS  54
#define MB_SYSTEM_VALUES        55

#define MB_WHICH_QUANTILE       56

#define OUTFORMAT_MML                  1
#define OUTFORMAT_LATEX                2

typedef struct tagINPUT_NOTATION_REC
{
  int nbracket_tables;
  int nparen_tables;
  int nbrace_tables;
  int n_tables;
  int nmixed_numbers;
  int funcarg_is_subscript;
  int n_primes;
  int n_doverds;                // d/dx
  int n_overbars;
  int n_dotaccents;
  int n_Dxs;
  int n_logs;
} INPUT_NOTATION_REC;

#endif
