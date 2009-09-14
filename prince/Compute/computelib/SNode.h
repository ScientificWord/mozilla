#ifndef SNODE_H
#define SNODE_H

#include "iCmpTypes.h"


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



// structs for "semantic" trees

class SEMANTICS_NODE;

enum OpOrderIlk {
  OOI_none,
  OOI_lessthan,
  OOI_lessorequal,
  OOI_equal,
  OOI_greaterthan,
  OOI_greaterorequal,
  OOI_element,
  OOI_rightarrow
};

struct BUCKET_REC
{
  BUCKET_REC* next;
  BUCKET_REC* parts;
  U32 bucket_ID;
  SEMANTICS_NODE* first_child;
};

class SEMANTICS_NODE
{
  public:
    SEMANTICS_NODE* next;
    SEMANTICS_NODE* prev;
    BUCKET_REC* parent;
    U32 owner_ID;
    U32 error_flag;
    SemanticType semantic_type;
    char* canonical_ID;
    char* contents;
    U32 msi_class;
    SemanticVariant variant;
    int infix_precedence;
    U32 subtree_type;
    U32 nrows;
    U32 ncols;
    U32 n_sub_args;
    BUCKET_REC* bucket_list;
};



SEMANTICS_NODE* CreateSemanticsNode();
SEMANTICS_NODE* CreateSemanticsNode(SemanticType type);

void DisposeSemanticsNode(SEMANTICS_NODE* del);
void DisposeSList(SEMANTICS_NODE* s_list);

char* DumpSNode(const SEMANTICS_NODE* s_node, int indent);
char* DumpSList(const SEMANTICS_NODE* s_list, int indent);

SEMANTICS_NODE* AppendSLists(SEMANTICS_NODE* s_list, SEMANTICS_NODE* new_tail);

int SemanticVariant2NumIntegrals(SemanticVariant var);

SemanticType GetBigOpType(const char* op_chdata, SemanticVariant& n_integs);
SEMANTICS_NODE* LocateVarAndExpr(BUCKET_REC* l_bucket, SEMANTICS_NODE** s_expr, int & direction);

OpOrderIlk GetOpOrderIlk(SEMANTICS_NODE* relop);

void AddDefaultBaseToLOG(SEMANTICS_NODE * snode, bool log_is_base10);

void FenceToMatrix(SEMANTICS_NODE* operand);
void FenceToInterval(SEMANTICS_NODE* s_fence);
void Patchdx(SEMANTICS_NODE * s_frac);
bool LocatePieces(BUCKET_REC* cell_list,
                  U32 row_tally, U32 ncols,
                  SEMANTICS_NODE** s_expression,
                  SEMANTICS_NODE** s_domain);

SEMANTICS_NODE* CreateOnePiece(SEMANTICS_NODE* s_expression,
                               SEMANTICS_NODE* s_domain);

void ConvertToPIECEWISElist(SEMANTICS_NODE * s_fence);
SEMANTICS_NODE* RemoveInfixOps(SEMANTICS_NODE* s_var);
SEMANTICS_NODE* RemoveParens(SEMANTICS_NODE* s_operand);
SEMANTICS_NODE* LocateVarAndLimits(BUCKET_REC* l_bucket,
                                   SEMANTICS_NODE** s_ll,
                                   SEMANTICS_NODE** s_ul,
                                   bool& ll_is_inclusive,
                                   bool& ul_is_inclusive);
void SetVarAndIntervalLimit(BUCKET_REC* ll_bucket);
void SetVarArrowExprLimit(BUCKET_REC* ll_bucket);
bool IsVarInSLIST(SEMANTICS_NODE* s_var_list, char* var_nom);

void ArgsToMatrix(SEMANTICS_NODE* snode, BUCKET_REC* br);

void CreatePrefixForm(SEMANTICS_NODE* s_operator,
                      SEMANTICS_NODE* l_operand,
                      SEMANTICS_NODE* r_operand);

void AppendNumber(SEMANTICS_NODE* snode, U32 bucket_ID, int num);

void RemoveBucket(SEMANTICS_NODE* s_base, BUCKET_REC* targ);



BUCKET_REC* MakeBucketRec(U32 which_bucket, SEMANTICS_NODE* sem_child);
BUCKET_REC* MakeParentBucketRec(U32 which_bucket, SEMANTICS_NODE* sem_child);


BUCKET_REC* AppendBucketRec(BUCKET_REC* a_list, BUCKET_REC* new_a_rec);
void AppendBucketRecord(BUCKET_REC*& a_list, BUCKET_REC* new_a_rec);


BUCKET_REC* FindBucketRec(BUCKET_REC* a_list, U32 bucket_ID);
void DisposeBucketList(BUCKET_REC* arg_list);

SEMANTICS_NODE* PrefixToInfix(SEMANTICS_NODE* s_list);

SEMANTICS_NODE* NestInPGroup(SEMANTICS_NODE* s_list, BUCKET_REC* parent_bucket);

char* GetBucketName(U32 bucket_ID);




#endif