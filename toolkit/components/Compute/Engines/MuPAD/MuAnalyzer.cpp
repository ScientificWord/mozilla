// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

/* Implementation Notes - JBM

   In the initial implementation ( JBM - July 2003 ) computation results
   from engines are ASCII strings.  The Maple2Tree object generates a
   tree of MNODES from the engine return string. This object, eAnalyzer,
   generates a semantics tree from the MNODEs tree.  When CompEngine
   back-translates the reply from the engine, it uses the semantics
   tree produced here.

   Note that engine return strings contain math names that are
   1. generated aliases for entities and structures that the
      engine can't handle directly but can pass thru as atoms.
   2. prefixed with something like "q1" to distinguish one
      clients variables from anothers.

   As SEMANTICS_NODEs are generated, math names are translated
   back to names originally sent by CompEngine.
*/

#include "MuAnalyzer.h"
//#include "MuWrap.h"

#include "CmpTypes.h"
#include "fltutils.h"
#include "Grammar.h"
#include "mnode.h"
#include "SNode.h"

#include "strutils.h"
#include <string.h>
#include <ctype.h>

MuAnalyzer::MuAnalyzer(Grammar* the_grammar)
{
  myGrammar = the_grammar;
  msg_list = NULL;
  mangle_info = NULL;
}

MuAnalyzer::~MuAnalyzer()
{
  TCI_ASSERT(!msg_list);
  DisposeMsgs(msg_list);
}

LOG_MSG_REC *MuAnalyzer::GetMsgs()
{
  LOG_MSG_REC *rv = msg_list;
  msg_list = NULL;
  return rv;
}

SEMANTICS_NODE *MuAnalyzer::BuildSemanticsTree(MNODE * eng_tree, U32 cmd_ID,
                                               ClientAliases_REC * name_map)
{
  SEMANTICS_NODE *rv = NULL;

  curr_cmd_ID = cmd_ID;
  mangle_info = name_map;

  TCI_ASSERT(!msg_list);
  DisposeMsgs(msg_list);
  msg_list = NULL;

  MNODE *e_tree = InfixToPrefix(eng_tree, "^");
  e_tree = InfixToPrefix(e_tree, "/");
  e_tree = InfixToPrefix(e_tree, "mnjoin");
  e_tree = InfixToPrefix(e_tree, "|");  

  // Generate the semantics tree that is returned
  rv = GetSemanticsList(e_tree, NULL);

  // Some Maple return strings don't translate directly into Math.
  // ie. some Maple commands generate returns that are specially formatted
  //  and require special processing to produce a meaningful semantics tree.

  switch (cmd_ID) {

  //Cholesky Decomposition<uID65.0>tcicholesky(%s); returns matrix, matrix
  //  We need matrix &invisibleTimes; matrix

  // jcs Same for PLU , etc
  case CCID_Cholesky_Decomposition:
  case CCID_PLU_Decomposition:
  case CCID_QR_Decomposition:
  case CCID_Singular_Values:
  case CCID_SVD:
  case CCID_Rational_Canonical_Form: 
    FixMapleCholesky(rv);
    break;

  case CCID_Eigenvectors:
    rv = FixMapleEigenVect(rv);
    break;

  //Jordan Form<uID79.0>tcijordan(%s);
  case CCID_Jordan_Form:
    FixMapleJordanForm(rv);
    break;

  //Simplex Dual<uID100.0>dual(%s);
  case CCID_Simplex_Dual:
    rv = FixMapleSimplexDual(rv);
    break;

  //Simplex Standardize<uID104.0>standardize(%s);
  case CCID_Simplex_Standardize:
    FixMapleSimplexStandardize(rv);
    break;

  case CCID_Quantile:
    rv = FixQuantileResult(rv);
    break;

  case CCID_Definiteness_Tests:
    rv = FixDefinitenessTests(rv);

  default:
    break;
  }

  DisposeTList(e_tree);
  return rv;
}

SEMANTICS_NODE *MuAnalyzer::GetSemanticsList(MNODE * eng_list,
                                             BUCKET_REC * parent)
{
  return GetSemanticsList(eng_list, parent, ALL_NODES);
}

SEMANTICS_NODE *MuAnalyzer::GetSemanticsList(MNODE * eng_list,
                                             BUCKET_REC * parent,
                                             int mml_node_lim)
{
  SEMANTICS_NODE *head = NULL;
  SEMANTICS_NODE *tail;

  int tally = 0;
  MNODE *rover = eng_list;
  while (rover && tally < mml_node_lim) {
    int eng_nodes_done = 0;
    SEMANTICS_NODE *new_node = SNodeFromMNodes(rover, eng_nodes_done);
    if (new_node) {
      new_node->parent = parent;
      if (head) {
        tail->next = new_node;
        new_node->prev = tail;
      } else {
        head = new_node;
      }
      tail = new_node;
    }
    if (eng_nodes_done) {
      while (eng_nodes_done) {
        tally++;
        rover = rover->next;
        eng_nodes_done--;
      }
    } else
      break;
  }
  return head;
}

void MuAnalyzer::SNodeFromOneMNode(SEMANTICS_NODE * snode,
                                   MNODE * eng_node, int & local_nodes_done)
{
  local_nodes_done = 1;

  const char *eng_element = eng_node->src_tok;
  size_t ln = strlen(eng_element);
  
  switch (ln) {

  case 3:{
      if (ElementNameIs(eng_node, "set")) {
        AnalyzeSET(eng_node, snode, local_nodes_done);
      } else
        TCI_ASSERT(0);
    }
    break;

  case 5:{
      if (ElementNameIs(eng_node, "blist")) {
        AnalyzeBRACKETLIST(eng_node, snode, local_nodes_done);
      } else
        TCI_ASSERT(0);
    }
    break;

  case 6:{
      if (ElementNameIs(eng_node, "number")) {
          AnalyzeNUMBER(eng_node, snode, local_nodes_done);
      } else if (ElementNameIs(eng_node, "pgroup")) {
          AnalyzePARENGROUP(eng_node, snode, local_nodes_done);
      } else if (ElementNameIs(eng_node, "string")) {
          AnalyzeSTRING(eng_node, snode, local_nodes_done);
      } else
        TCI_ASSERT(0);
    }
    break;

  case 7:{
      if (ElementNameIs(eng_node, "arglist")) {
          TCI_ASSERT(0);
          AnalyzeARGLIST(eng_node, snode, local_nodes_done);
      } else if (ElementNameIs(eng_node, "Boolean")) {
          AnalyzeBOOLEAN(eng_node, snode, local_nodes_done);
      } else
          TCI_ASSERT(0);
    }
    break;

  case 8:{
      if (ElementNameIs(eng_node, "function")) {
          AnalyzeFUNCTION(eng_node, snode, local_nodes_done);
      } else if (ElementNameIs(eng_node, "variable")) {
          AnalyzeVARIABLE(eng_node, snode, local_nodes_done);
      } else if (ElementNameIs(eng_node, "operator")) {
          AnalyzeOPERATOR(eng_node, snode, local_nodes_done);
      } else if (ElementNameIs(eng_node, "fraction")) {
          AnalyzeFRACTION(eng_node, snode, local_nodes_done);
      } else if (ElementNameIs(eng_node, "constant")) {
          AnalyzeCONSTANT(eng_node, snode, local_nodes_done);
      } else
          TCI_ASSERT(0);
    }
    break;

  case 9:{
      if (ElementNameIs(eng_node, "mixed_num")) {
          AnalyzeMIXEDNUM(eng_node, snode, local_nodes_done);
      } else
          TCI_ASSERT(0);
    }
    break;

  case 10:{
      if (ElementNameIs(eng_node, "power_form")) {
          AnalyzeSUPERSCRIPT(eng_node, snode, local_nodes_done);
      } else if (ElementNameIs(eng_node, "derivative")) {
          AnalyzeDERIVATIVE(eng_node, snode, local_nodes_done);
      } else
          TCI_ASSERT(0);
    }
    break;

  case 11:{
      if (ElementNameIs(eng_node, "item_bucket")) {
          AnalyzeCOMPONENT(eng_node, snode, local_nodes_done);
      } else
          TCI_ASSERT(0);
    }
    break;

  case 12: {
    if (ElementNameIs(eng_node, "substitution")) {
        AnalyzeSUBSTITUTION(eng_node, snode, local_nodes_done);
    }
    break;
  }

  case 16:{
      if (ElementNameIs(eng_node, "precedence_group")) {
          AnalyzePRECGROUP(eng_node, snode, local_nodes_done);
      } else
          TCI_ASSERT(0);
    }
    break;

  default:
    TCI_ASSERT(0);
    break;
  }

  TCI_ASSERT(local_nodes_done == 1);
}

SEMANTICS_NODE* MuAnalyzer::SNodeFromMNodes(MNODE* eng_node,
                                            int& eng_nodes_done)
{
  SEMANTICS_NODE* rv = NULL;

  if (eng_node) {
    rv = CreateSemanticsNode();

    int local_nodes_done;
    SNodeFromOneMNode(rv, eng_node, local_nodes_done);
    eng_nodes_done = local_nodes_done;
  }
  return rv;
}

void MuAnalyzer::AnalyzePARENGROUP(MNODE* eng_node,
                                   SEMANTICS_NODE* snode,
                                   int& local_nodes_done)
{
  local_nodes_done = 1;
  snode->semantic_type = SEM_TYP_PRECEDENCE_GROUP;

  BUCKET_REC* item_list = NULL;
  if (eng_node->first_kid) {

    MNODE* contents = eng_node->first_kid;
    BUCKET_REC* obj_bucket = MakeBucketRec(MB_UNNAMED, NULL);
    AppendBucketRecord(item_list, obj_bucket);

    SEMANTICS_NODE* s_cell = GetSemanticsList(contents, obj_bucket);
    obj_bucket->first_child = s_cell;
    s_cell->parent = obj_bucket;
  }
  snode->bucket_list = item_list;
}

void MuAnalyzer::AnalyzeNUMBER(MNODE* eng_node,
                               SEMANTICS_NODE* snode, 
                               int& local_nodes_done)
{
  local_nodes_done = 1;
  snode->semantic_type = SEM_TYP_NUMBER;
  if (eng_node->p_chdata) {
    //size_t zln = strlen( );
    //char *tmp = new char[zln + 1];
    //strcpy(tmp, eng_node->p_chdata);
    snode->contents = DuplicateString(eng_node->p_chdata);
  }
}

void MuAnalyzer::AnalyzeSTRING(MNODE * eng_node,
                               SEMANTICS_NODE * snode, int & local_nodes_done)
{
  local_nodes_done = 1;
  snode->semantic_type = SEM_TYP_TEXT;
  if (eng_node->p_chdata) {
    size_t zln = strlen(eng_node->p_chdata);
    char *tmp = new char[zln + 1];
    strcpy(tmp, eng_node->p_chdata);
    snode->contents = tmp;
  }
}

// second pass required
// add a bunch of special cases here matrix(), vector(), int(), etc.
/*
       1 <function>0.0.0
surd
       1   <fence>0.0.0
       1     <mrow>0.0.0
       1       <variable>0.0.0
q1x
       1       </variable>
       1     </mrow>
       1     <mrow>0.0.0
       1       <number>0.0.0
3
       1       </number>
       1     </mrow>
       1   </fence>
       1 </function>
*/

void MuAnalyzer::AnalyzeSURD(MNODE * func_node,
                             SEMANTICS_NODE * snode, int & local_nodes_done)
{
  local_nodes_done = 1;
  snode->semantic_type = SEM_TYP_ROOT;
  MNODE* args_node = func_node->first_kid;
  if (ElementNameIs(args_node, "arglist")) {
    BUCKET_REC* item_list = NULL;
    if (args_node->first_kid) {

      // locate items in this fence
      MNODE *item_rover = args_node->first_kid;
      if (item_rover->first_kid) {
        MNODE *contents = item_rover->first_kid;
        BUCKET_REC *item_bucket = MakeBucketRec(MB_ROOT_BASE, NULL);
        AppendBucketRecord(item_list, item_bucket);
        SEMANTICS_NODE *s_cell = GetSemanticsList(contents, item_bucket);
        item_bucket->first_child = s_cell;
      }

      if (item_rover->next) {
        item_rover = item_rover->next;
        if (item_rover->first_kid) {
          MNODE *contents = item_rover->first_kid;
          BUCKET_REC *item_bucket = MakeBucketRec(MB_ROOT_EXPONENT, NULL);
          AppendBucketRecord(item_list, item_bucket);
          SEMANTICS_NODE *s_cell = GetSemanticsList(contents, item_bucket);
          item_bucket->first_child = s_cell;
        }
      }
    }
    snode->bucket_list = item_list;
  }
}

/*
<function name="binomial">
  <arglist>
    <arg_bucket>
      <number>8</number>
    </arg_bucket>
    <arg_bucket>
      <variable>q1n</variable>
    </arg_bucket>
  </arglist>
</function>
*/

void MuAnalyzer::AnalyzeBINOMIAL(MNODE * func_node,
                                 SEMANTICS_NODE * snode,
                                 int & local_nodes_done)
{
  local_nodes_done = 1;
  snode->semantic_type = SEM_TYP_BINOMIAL;
  MNODE *args_node = func_node->first_kid;
  if (ElementNameIs(args_node, "arglist")) {
    BUCKET_REC* item_list = NULL;
    if (args_node->first_kid) {
      MNODE* item_rover = args_node->first_kid;
      if (item_rover->first_kid) {
        MNODE *contents = item_rover->first_kid;
        BUCKET_REC *item_bucket = MakeBucketRec(MB_NUMERATOR, NULL);
        AppendBucketRecord(item_list, item_bucket);
        SEMANTICS_NODE *s_cell = GetSemanticsList(contents, item_bucket);
        item_bucket->first_child = s_cell;
      }

      if (item_rover->next) {
        item_rover = item_rover->next;
        if (item_rover->first_kid) {
          MNODE *contents = item_rover->first_kid;
          BUCKET_REC *item_bucket = MakeBucketRec(MB_DENOMINATOR, NULL);
          AppendBucketRecord(item_list, item_bucket);

          SEMANTICS_NODE *s_cell = GetSemanticsList(contents, item_bucket);
          item_bucket->first_child = s_cell;
        }
      }
    }
    snode->bucket_list = item_list;
  }
}

// Sample string from MuPad - "Interval(2, [7/2])"

void MuAnalyzer::AnalyzeINTERVAL(MNODE * func_node,
                                 SEMANTICS_NODE * snode,
                                 int & local_nodes_done)
{
  local_nodes_done = 1;
  snode->semantic_type = SEM_TYP_INTERVAL;

  SemanticVariant interval_ilk = SNV_None;
  bool left_open = true;
  bool right_open = true;

  MNODE *args_list = func_node->first_kid;
  if (ElementNameIs(args_list, "arglist")) {
    if (args_list->first_kid) {
      MNODE *arg1 = args_list->first_kid;
      if (arg1->first_kid) {
        BUCKET_REC *bucket = MakeBucketRec(MB_INTERVAL_START, NULL);
        AppendBucketRecord(snode->bucket_list, bucket);

        MNODE* contents = arg1->first_kid;
        if (ElementNameIs(contents, "blist")) {
          left_open = false;
          contents = contents->first_kid;
          if (contents)
            contents = contents->first_kid;
          else
            TCI_ASSERT(0);
        }
        SEMANTICS_NODE *s_expr1 = GetSemanticsList(contents, bucket);
        bucket->first_child = s_expr1;

        if (arg1->next) {
          MNODE *arg2 = arg1->next;
          if (arg2->first_kid) {
            BUCKET_REC *bucket = MakeBucketRec(MB_INTERVAL_END, NULL);
            AppendBucketRecord(snode->bucket_list, bucket);

            MNODE* contents = arg2->first_kid;
            if (ElementNameIs(contents, "blist")) {
              right_open = false;
              contents = contents->first_kid;
              if (contents)
                contents = contents->first_kid;
              else
                TCI_ASSERT(0);
            }
            SEMANTICS_NODE *s_expr2 = GetSemanticsList(contents, bucket);
            bucket->first_child = s_expr2;

            if (left_open) {
              interval_ilk =
                right_open ? SNV_ExclExclInterval : SNV_ExclInclInterval;
            } else {
              interval_ilk =
                right_open ? SNV_InclExclInterval : SNV_InclInclInterval;
            }
          } else
            TCI_ASSERT(0);
        } else
          TCI_ASSERT(0);
      } else
        TCI_ASSERT(0);
    } else
      TCI_ASSERT(0);
  } else
    TCI_ASSERT(0);

  snode->variant = interval_ilk;
}

void MuAnalyzer::AnalyzeNO_OP(MNODE * func_node,
                              SEMANTICS_NODE * snode, int & local_nodes_done)
{
  local_nodes_done = 1;

  MNODE *args_node = func_node->first_kid;
  if (ElementNameIs(args_node, "arglist")) {
    if (args_node->first_kid) {
      MNODE *arg_bucket = args_node->first_kid;
      if (arg_bucket->first_kid) {
        int eng_nodes_done = 0;
        SEMANTICS_NODE *new_node = SNodeFromMNodes(arg_bucket->first_kid,
                                                   eng_nodes_done);
        // transcribe fields from new_node to snode
        snode->error_flag = new_node->error_flag;
        snode->semantic_type = new_node->semantic_type;
        snode->canonical_ID = new_node->canonical_ID;
        new_node->canonical_ID = NULL;
        snode->contents = new_node->contents;
        new_node->contents = NULL;
        snode->nrows = new_node->nrows;
        snode->ncols = new_node->ncols;
        snode->bucket_list = new_node->bucket_list;
        new_node->bucket_list = NULL;
        DisposeSemanticsNode(new_node);
      }
    }
  }
}

/*
<function name="Int">               MATH_CONTAINER
  <arglist>							Bucket list
    <arg_bucket>					  UNNAMED
      <number>.50</number>			    PRECEDENCE_GROUP
      <operator>*</operator>		    Bucket list
      <precedence_group>			      UNNAMED
        <variable>q1x</variable>	        BIG_OPERATOR contents = "&#x222b;"
        <operator>^</operator>		        Bucket list
        <number>2</number>			          LOWERLIMIT
      </precedence_group>			            NUMBER canonical_ID = "mn1" contents = "1"
    </arg_bucket>					          UPPERLIMIT
    <arg_bucket>					            VARIABLE canonical_ID = "mia" contents = "a"
      <variable>q1x</variable>		            OPERATOR contents = "+"
      <operator>=</operator>		            NUMBER canonical_ID = "mn1" contents = "1"
      <precedence_group>			          OPERAND
        <number>1</number>			            FRACTION
        <operator>..</operator>		            Bucket list
        <precedence_group>			              NUMERATOR
          <variable>q1a</variable>	                SUPERSCRIPT
          <operator>+</operator>	                Bucket list
          <number>1</number>		                  SCRIPT_BASE
        </precedence_group>			                    VARIABLE canonical_ID = "mix" contents = "x"
      </precedence_group>			                  SCRIPT_UPPER 
    </arg_bucket>					                    NUMBER canonical_ID = "mn2" contents = "2"
  </arglist>						                End Bucket list
</function>							              DENOMINATOR
									                NUMBER canonical_ID = "mn2" contents = "2"
									            End Bucket list
									          VARIABLE OF INTEGRATION
									            VARIABLE canonical_ID = "mix" contents = "x"
									        End Bucket list
									    End Bucket list
									End Bucket list

*/

void MuAnalyzer::AnalyzeINTEGRAL(MNODE * func_Int_node,
                                 SEMANTICS_NODE * snode,
                                 int & local_nodes_done)
{
  local_nodes_done = 1;
  snode->semantic_type = SEM_TYP_BIGOP_INTEGRAL;
  MNODE *args_list = func_Int_node->first_kid;
  if (args_list) {
    // MuPAD format is   int( integrand,var{=ll..ul} )
    BUCKET_REC *item_list = NULL;
    if (args_list->first_kid) { // integrand
      MNODE *arg_rover = args_list->first_kid;
      if (arg_rover->first_kid) {
        MNODE *contents = arg_rover->first_kid;
        BUCKET_REC *item_bucket = MakeBucketRec(MB_OPERAND, NULL);
        AppendBucketRecord(item_list, item_bucket);

        SEMANTICS_NODE *s_cell = GetSemanticsList(contents, item_bucket);
        item_bucket->first_child = s_cell;
      } else
        TCI_ASSERT(0);
      if (arg_rover->next) {
        arg_rover = arg_rover->next;
        if (arg_rover->first_kid) {
          MNODE *contents_rover = arg_rover->first_kid;
          /*
                <variable>q1x</variable>
                <operator>=</operator>
                <precedence_group>
                  <number>1</number>
                  <operator>..</operator>
                  <precedence_group>
                    <variable>q1a</variable>
                    <operator>+</operator>
                    <number>1</number>
                  </precedence_group>
                </precedence_group>
			          MB_LOWERLIMIT	MB_UPPERLIMIT	MB_OPERAND	 MB_INTEG_VAR
          */
          if (contents_rover) {
            BUCKET_REC* item_bucket = MakeBucketRec(MB_INTEG_VAR, NULL);
            AppendBucketRecord(item_list, item_bucket);

            SEMANTICS_NODE* s_cell = GetSemanticsList(contents_rover, item_bucket, 1);
            item_bucket->first_child = s_cell;
            contents_rover = contents_rover->next;
          } else
            TCI_ASSERT(0);

          if (contents_rover)
            //      <operator>=</operator>
            contents_rover = contents_rover->next;
          if (contents_rover && contents_rover->first_kid) {
            contents_rover = contents_rover->first_kid;

            BUCKET_REC *item_bucket = MakeBucketRec(MB_LOWERLIMIT, NULL);
            AppendBucketRecord(item_list, item_bucket);

            SEMANTICS_NODE *s_cell = GetSemanticsList(contents_rover, item_bucket, 1);
            item_bucket->first_child = s_cell;
            contents_rover = contents_rover->next;

            if (contents_rover)
              contents_rover = contents_rover->next;
            else
              TCI_ASSERT(0);

            if (contents_rover) {
              BUCKET_REC *item_bucket = MakeBucketRec(MB_UPPERLIMIT, NULL);
              AppendBucketRecord(item_list, item_bucket);

              SEMANTICS_NODE *s_cell = GetSemanticsList(contents_rover, item_bucket, 1);
              item_bucket->first_child = s_cell;
            } else
              TCI_ASSERT(0);
          }
        }
      } else
        TCI_ASSERT(0);
    }
    snode->bucket_list = item_list;
  }
}

void MuAnalyzer::AnalyzeSUM(MNODE * func_sum_node,
                            SEMANTICS_NODE * snode, int & local_nodes_done)
{
  local_nodes_done = 1;
  snode->semantic_type = SEM_TYP_BIGOP_SUM;
  MNODE *args_list = func_sum_node->first_kid;
  if (args_list) {
    // MuPAD format is   sum(sumand, i8 = 0..9)
    if (args_list->first_kid) { // sumand
      MNODE *arg_rover = args_list->first_kid;
      if (arg_rover->first_kid) {
        MNODE *sumand = arg_rover->first_kid;
        BUCKET_REC* bucket = MakeBucketRec(MB_OPERAND, NULL);
        AppendBucketRecord(snode->bucket_list, bucket);

        SEMANTICS_NODE *s_sumand = GetSemanticsList(sumand, bucket);
        bucket->first_child = s_sumand;
        s_sumand->parent = bucket;
      } else
        TCI_ASSERT(0);

      if (arg_rover->next) {    // limits arg  i= 1..10
        arg_rover = arg_rover->next;
        if (arg_rover->first_kid) {
          MNODE *limits_rover = arg_rover->first_kid;
          if (limits_rover) {
            BUCKET_REC *bucket = MakeBucketRec(MB_SUM_VAR, NULL);
            AppendBucketRecord(snode->bucket_list, bucket);
            SEMANTICS_NODE *s_var = GetSemanticsList(limits_rover, bucket, 1);
            bucket->first_child = s_var;
            s_var->parent = bucket;
            limits_rover = limits_rover->next;
          } else
            TCI_ASSERT(0);

          if (limits_rover)
            // <operator>=</operator>
            limits_rover = limits_rover->next;

          if (limits_rover && limits_rover->first_kid) {
            // descend into a precedence group
            limits_rover = limits_rover->first_kid;

            BUCKET_REC *bucket = MakeBucketRec(MB_LOWERLIMIT, NULL);
            AppendBucketRecord(snode->bucket_list, bucket);
            SEMANTICS_NODE *s_ll = GetSemanticsList(limits_rover, bucket, 1);
            bucket->first_child = s_ll;
            s_ll->parent = bucket;

            limits_rover = limits_rover->next;
            if (limits_rover)
              // <operator>..</operator>
              limits_rover = limits_rover->next;
            else
              TCI_ASSERT(0);

            if (limits_rover) {
              BUCKET_REC *bucket = MakeBucketRec(MB_UPPERLIMIT, NULL);
              
              AppendBucketRecord(snode->bucket_list, bucket);
              SEMANTICS_NODE *s_ul = GetSemanticsList(limits_rover, bucket, 1);
              bucket->first_child = s_ul;
              s_ul->parent = bucket;
            } else
              TCI_ASSERT(0);
          } else
            TCI_ASSERT(0);
        }
      } else
        TCI_ASSERT(0);
    }
  }
}

void MuAnalyzer::AnalyzeCONJUGATE(MNODE * func_node,
                                  SEMANTICS_NODE * snode,
                                  int & local_nodes_done)
{
  local_nodes_done = 1;
  snode->semantic_type = SEM_TYP_CONJUGATE;
  MNODE *args_node = func_node->first_kid;
  if (ElementNameIs(args_node, "arglist")) {
    BUCKET_REC *item_list = NULL;
    if (args_node->first_kid) {
      MNODE *item_rover = args_node->first_kid;
      if (item_rover->first_kid) {
        MNODE *contents = item_rover->first_kid;
        BUCKET_REC *item_bucket = MakeBucketRec(MB_UNNAMED, NULL);
        AppendBucketRecord(item_list, item_bucket);

        SEMANTICS_NODE *s_cell = GetSemanticsList(contents, item_bucket);
        item_bucket->first_child = s_cell;
      }
    }
    snode->bucket_list = item_list;
  }
}

// Engine output - "D(D(q1y(q1x)))(q1x)" for y"(x) OR d^2y/dx^2

void MuAnalyzer::AnalyzeDERIVATIVE(MNODE* deriv_node,
                                   SEMANTICS_NODE* snode,
                                   int& local_nodes_done)
{
  local_nodes_done = 1;
  snode->semantic_type = SEM_TYP_DERIVATIVE;

  MNODE *func_node = deriv_node->first_kid;
  if (ElementNameIs(func_node, "function")) {
    SEMANTICS_NODE* s_func = CreateSemanticsNode(SEM_TYP_FUNCTION);
    //s_func->semantic_type = SEM_TYP_FUNCTION;

    AnalyzeFUNCTION(func_node, s_func, local_nodes_done);

    BUCKET_REC* expr_bucket = MakeParentBucketRec(MB_UNNAMED, s_func);
    AppendBucketRecord(snode->bucket_list, expr_bucket);
    

    int n_primes = 1;
    if (deriv_node->p_chdata)
      n_primes = atoi(deriv_node->p_chdata);
    else
      TCI_ASSERT(0);

    MNODE* m_diff_var = LocateDiffVar(deriv_node);
    if (m_diff_var) {
      BUCKET_REC* bucket = MakeBucketRec(MB_DIFF_VAR, NULL);
      AppendBucketRecord(snode->bucket_list, bucket);
      SEMANTICS_NODE *s_var = GetSemanticsList(m_diff_var, bucket);
      bucket->first_child = s_var;
      s_var->parent = bucket;

    }
    AddPrimesCount(snode, n_primes);
  } else
    TCI_ASSERT(0);
}

void MuAnalyzer::AnalyzeEXPONENTIAL(MNODE * func_node,
                                    SEMANTICS_NODE * snode,
                                    int & local_nodes_done)
{
  local_nodes_done = 1;
  snode->semantic_type = SEM_TYP_POWERFORM;
  MNODE *args_node = func_node->first_kid;
  if (ElementNameIs(args_node, "arglist")) {
    // Get the value of the argument
    if (args_node->first_kid) {
      MNODE *exponent = args_node->first_kid;
      if (exponent->first_kid) {
        MNODE *exp_contents = exponent->first_kid;

        bool is_one = false;
        if (exp_contents && !exp_contents->next)
          if (ElementNameIs(exp_contents, "number"))
            if (exp_contents->p_chdata)
              if (!strcmp(exp_contents->p_chdata, "1"))
                is_one = true;

        if (is_one) {
          snode->semantic_type = SEM_TYP_UCONSTANT;
          char *tmp = new char[10];
          strcpy(tmp, "&#x2147;");
          snode->contents = tmp;
        } else {
          BUCKET_REC *script_bucket = MakeBucketRec(MB_SCRIPT_UPPER, NULL);
          SEMANTICS_NODE *s_exp_list = GetSemanticsList(exp_contents, script_bucket);
          script_bucket->first_child = s_exp_list;

          SEMANTICS_NODE *s_base = CreateSemanticsNode(SEM_TYP_UCONSTANT);
          //s_base->semantic_type = SEM_TYP_UCONSTANT;
          char *tmp = new char[10];
          strcpy(tmp, "&#x2147;");
          s_base->contents = tmp;

          BUCKET_REC* b_list = NULL;
          BUCKET_REC* base_bucket = MakeParentBucketRec(MB_SCRIPT_BASE, s_base);
          AppendBucketRecord(b_list, base_bucket);
          AppendBucketRecord(b_list, script_bucket);
          snode->bucket_list = b_list;
        }
      }
    } else
      TCI_ASSERT(0);
  } else
    TCI_ASSERT(0);
}

/*
MNODE tree for a MuPAD return array
array(1..2, 1..2, (1,1) = q1x, (1,2) = - q1x^2 + 1, (2,1) = - q1x^2 + 1, (2,2) = q1x)

<function name="array">
  <arglist>
    <arg_bucket>
    </arg_bucket>
    <arg_bucket>
    </arg_bucket>
	.
	.
  </arglist>
</function>
*/

void MuAnalyzer::AnalyzeARRAY(MNODE * array_node,
                              SEMANTICS_NODE * snode, int & local_nodes_done)
{
  local_nodes_done = 1;
  snode->semantic_type = SEM_TYP_TABULATION;

  MNODE *arg_rover = NULL;
  if (array_node && array_node->first_kid) {
    MNODE *arglist = array_node->first_kid;
    arg_rover = arglist->first_kid;
  }

  U32 dimens[16];               // I don't know how many dimensions
  int di = 0;                   //  a MuPAD "array" can contain

  BUCKET_REC *cells_list = NULL;
  while (arg_rover) {
    U32 n_start, n_end;
    if (IsArrayDimen(arg_rover, n_start, n_end)) {
      if (di < 16)
        dimens[di++] = n_end - n_start + 1;
    } else {
      MNODE* c_rover = arg_rover->first_kid;
      MNODE* cell_content = NULL;
      if (ElementNameIs(c_rover, "pgroup")) { // (1,1)
        c_rover = c_rover->next;
        if (ElementNameIs(c_rover, "operator")) { // =
          if (c_rover->p_chdata && c_rover->p_chdata[0] == '=') {
            cell_content = c_rover->next;
            // The contents a matrix cell doesn't need an enclosing precedence group
            if (ElementNameIs(cell_content, "precedence_group")  && !cell_content->next)
              cell_content = cell_content->first_kid;
          }
        }
      }
      if (cell_content) {       // q1x
        BUCKET_REC* cell_bucket = MakeBucketRec(MB_UNNAMED, NULL);
        AppendBucketRecord(cells_list, cell_bucket);
        SEMANTICS_NODE* curr_cell = GetSemanticsList(cell_content, cell_bucket);
        cell_bucket->first_child = curr_cell;
        curr_cell->parent = cell_bucket;
      } else
        TCI_ASSERT(0);
    }
    arg_rover = arg_rover->next;
  }
  if (cells_list) {
    snode->bucket_list = cells_list;
    TCI_ASSERT(di == 2);
    snode->nrows = dimens[0];
    snode->ncols = dimens[1];
  }
}

void MuAnalyzer::AnalyzeLIMIT(MNODE * func_node,
                              SEMANTICS_NODE * snode, int & local_nodes_done)
{
  local_nodes_done = 1;
  snode->semantic_type = SEM_TYP_LIMFUNC;
  char *tmp = new char[4];
  strcpy(tmp, "lim");
  snode->contents = tmp;

  if (func_node && func_node->first_kid) {
    MNODE *arglist = func_node->first_kid;
    if (arglist->first_kid) {
      MNODE *arg1 = arglist->first_kid;
      if (arg1 && arg1->next) {
        MNODE *arg2 = arg1->next;
        if (arg1->first_kid) {
          MNODE* contents = arg1->first_kid;
          BUCKET_REC* bucket = MakeBucketRec(MB_UNNAMED, NULL);
          SEMANTICS_NODE* s_cell = GetSemanticsList(contents, bucket);
          AppendBucketRecord(snode->bucket_list, bucket);
          bucket->first_child = s_cell;
        }
        if (arg2->first_kid) {
          BUCKET_REC *bucket = MakeBucketRec(MB_LOWERLIMIT, NULL);
          BUCKET_REC *parts_list = NULL;
          // MuPad generates "x=infinity"
          int tally = 0;
          MNODE *m_rover = arg2->first_kid;
          while (m_rover) {
            if (tally == 0) {
              BUCKET_REC *bucket = MakeBucketRec(MB_LL_VAR, NULL);
              SEMANTICS_NODE *s_var = GetSemanticsList(m_rover, bucket, 1);
              parts_list = AppendBucketRec(parts_list, bucket);
              bucket->first_child = s_var;
            } else if (tally == 1) {
            } else if (tally == 2) {
              BUCKET_REC *bucket = MakeBucketRec(MB_LL_EXPR, NULL);
              SEMANTICS_NODE *s_expr = GetSemanticsList(m_rover, bucket);
              parts_list = AppendBucketRec(parts_list, bucket);
              bucket->first_child = s_expr;
            }
            tally++;
            m_rover = m_rover->next;
          }
          if (arg2->next) {
            MNODE *arg3 = arg2->next;
            if (arg3->first_kid) {
              MNODE *variable = arg3->first_kid;

              SEMANTICS_NODE* s_num = CreateSemanticsNode(SEM_TYP_NUMBER);
              //s_num->semantic_type = SEM_TYP_NUMBER;
              char *tmp = new char[2];
              if (!strcmp(variable->p_chdata, "Right"))
                strcpy(tmp, "1");
              else if (!strcmp(variable->p_chdata, "Left"))
                strcpy(tmp, "2");
              else
                TCI_ASSERT(0);
              s_num->contents = tmp;

              BUCKET_REC* bucket = MakeParentBucketRec(MB_LL_DIRECTION, s_num);
              parts_list = AppendBucketRec(parts_list, bucket);
              
            }
          }
          bucket->parts = parts_list;
          snode->bucket_list = AppendBucketRec(snode->bucket_list, bucket);
        }
      } else
        TCI_ASSERT(0);
    } else
      TCI_ASSERT(0);
  } else
    TCI_ASSERT(0);
}

void MuAnalyzer::AnalyzeVECTOR(MNODE * func_vector_node,
                               SEMANTICS_NODE * snode, int & local_nodes_done)
{
  local_nodes_done = 1;
  snode->semantic_type = SEM_TYP_TABULATION;
  MNODE *vector = NULL;
  if (func_vector_node && func_vector_node->first_kid) {
    MNODE *args_node = func_vector_node->first_kid;
    MNODE *arg_bucket = args_node->first_kid;
    if (arg_bucket && arg_bucket->first_kid)
      vector = arg_bucket->first_kid;
  }

  if (vector && vector->first_kid) {
    BUCKET_REC *cells_head = NULL;
    BUCKET_REC *cells_tail;
    int row_tally = 0;
    int col_tally = 0;
    MNODE *component_rover = vector->first_kid;
    while (component_rover) {   // loop thru rows
      BUCKET_REC *s_list = GetCellsFromROW(component_rover,
                                           col_tally);
      if (!cells_head)
        cells_head = s_list;
      else
        cells_tail->next = s_list;
      cells_tail = s_list;
      while (cells_tail->next)
        cells_tail = cells_tail->next;
      row_tally++;
      component_rover = component_rover->next;
    }
    snode->bucket_list = cells_head;
    snode->nrows = row_tally;
    snode->ncols = col_tally;
  } else
    TCI_ASSERT(0);
}

void MuAnalyzer::AnalyzeFUNCTION(MNODE * func_node, SEMANTICS_NODE * snode,
                                 int & local_nodes_done)
{
  local_nodes_done = 1;
  SemanticType sem_type = SEM_TYP_FUNCTION;
  bool done = false;

  const char *func_name = func_node->p_chdata;
  if (func_name) {
    size_t zln = strlen(func_name);

    switch (zln) {
    case 1:{
        if (!strcmp(func_name, "D")) {
          TCI_ASSERT(0);
          //        AnalyzeDERIVATIVE( func_node,snode,local_nodes_done );
          done = true;
        }
      }
      break;
    case 2:{
        if (!strcmp(func_name, "``")) {
          AnalyzeNO_OP(func_node, snode, local_nodes_done);
          done = true;
        }
      }
      break;
    case 3:{
        if (!strcmp(func_name, "abs")) {
          sem_type = SEM_TYP_ABS;
        } else if (!strcmp(func_name, "int")) {
          AnalyzeINTEGRAL(func_node, snode, local_nodes_done);
          done = true;
        } else if (!strcmp(func_name, "sum")) {
          AnalyzeSUM(func_node, snode, local_nodes_done);
          done = true;
        } else if (!strcmp(func_name, "exp")) {
          AnalyzeEXPONENTIAL(func_node, snode, local_nodes_done);
          done = true;
        } else if (!strcmp(func_name, "psi")) {
          //          func_node->p_chdata[0]  =  'P';
          delete[] func_node->p_chdata;
          char *tmp = new char[4];
          strcpy(tmp, "Psi");
          func_node->p_chdata = tmp;
        }
      }
      break;
    case 4:{
        if (!strcmp(func_name, "surd")) {
          AnalyzeSURD(func_node, snode, local_nodes_done);
          done = true;
        } else if (!strcmp(func_name, "diff")) {
          AnalyzeDIFF(func_node, snode, local_nodes_done);
          done = true;
        } else if (!strcmp(func_name, "ceil")) {
          sem_type = SEM_TYP_CEILING;
        } else if (!strcmp(func_name, "fact")) {
          delete[] func_node->p_chdata;
          char *tmp = new char[10];
          strcpy(tmp, "factorial");
          func_node->p_chdata = tmp;
        } else if (!strcmp(func_name, "zeta")) {
          delete[] func_node->p_chdata;
          char *tmp = new char[10];
          strcpy(tmp, "&#x3b6;");
          func_node->p_chdata = tmp;
        } else if (!strcmp(func_name, "beta")) {
          //          func_node->p_chdata[0]  =  'B';
          delete[] func_node->p_chdata;
          char *tmp = new char[5];
          strcpy(tmp, "Beta");
          func_node->p_chdata = tmp;
        }
      }
      break;
    case 5:{
        if (!strcmp(func_name, "array")) {
          AnalyzeARRAY(func_node, snode, local_nodes_done);
          done = true;
        } else if (!strcmp(func_name, "limit")) {
          AnalyzeLIMIT(func_node, snode, local_nodes_done);
          done = true;
        } else if (!strcmp(func_name, "floor")) {
          sem_type = SEM_TYP_FLOOR;
        } else if (!strcmp(func_name, "dirac")) {
          //          func_node->p_chdata[0]  =  'D';
          delete[] func_node->p_chdata;
          char *tmp = new char[6];
          strcpy(tmp, "Dirac");
          func_node->p_chdata = tmp;
        } else if (!strcmp(func_name, "gamma")) {
          delete[] func_node->p_chdata;
          char *tmp = new char[8];
          strcpy(tmp, "&#x393;");
          func_node->p_chdata = tmp;
        }
      }
      break;
    case 6:{
        if (!strcmp(func_name, "vector")) {
          AnalyzeVECTOR(func_node, snode, local_nodes_done);
          done = true;
        } else if (!strcmp(func_name, "igamma")) {
          delete[] func_node->p_chdata;
          char *tmp = new char[8];
          strcpy(tmp, "&#x393;");
          func_node->p_chdata = tmp;
        } else if (!strcmp(func_name, "airyAi")) {
          //          func_node->p_chdata[0]  =  'A';
          delete[] func_node->p_chdata;
          char *tmp = new char[7];
          strcpy(tmp, "AiryAi");
          func_node->p_chdata = tmp;
        } else if (!strcmp(func_name, "airyBi")) {
          //          func_node->p_chdata[0]  =  'A';
          delete[] func_node->p_chdata;
          char *tmp = new char[7];
          strcpy(tmp, "AiryBi");
          func_node->p_chdata = tmp;
        }
      }
      break;
    case 8:{
        if (!strcmp(func_name, "binomial")) {
          AnalyzeBINOMIAL(func_node, snode, local_nodes_done);
          done = true;
        } else if (!strcmp(func_name, "Interval")) {
          AnalyzeINTERVAL(func_node, snode, local_nodes_done);
          done = true;
        } else if (!strcmp(func_name, "lambertW")) {
          //          func_node->p_chdata[0]  =  'L';
          delete[] func_node->p_chdata;
          char *tmp = new char[9];
          strcpy(tmp, "LambertW");
          func_node->p_chdata = tmp;
        }
      }
      break;
    case 9:{
        if (!strcmp(func_name, "conjugate")) {
          AnalyzeCONJUGATE(func_node, snode, local_nodes_done);
          done = true;
        } else if (!strcmp(func_name, "piecewise")) {
          AnalyzePIECEWISE(func_node, snode, local_nodes_done);
          done = true;
        } else if (!strcmp(func_name, "heaviside")) {
          //          func_node->p_chdata[0]  =  'H';
          delete[] func_node->p_chdata;
          char *tmp = new char[10];
          strcpy(tmp, "Heaviside");
          func_node->p_chdata = tmp;
        }
      }
      break;

    case 12:{
        if (!strcmp(func_name, "numeric::int")) {
          AnalyzeINTEGRAL(func_node, snode, local_nodes_done);
          snode->variant = SNV_numericint;
          done = true;
        }
      }
      break;

    case 13:{
        if (!strcmp(func_name, "Dom::Interval")) {
          AnalyzeINTERVAL(func_node, snode, local_nodes_done);
          done = true;
        }
      }
      break;

    case 18:{
        if (!strcmp(func_name, "transform::laplace")) {
          delete[] func_node->p_chdata;
          char *tmp = new char[9];
          strcpy(tmp, "&#x2112;");
          func_node->p_chdata = tmp;
          sem_type = SEM_TYP_TRANSFORM;
        } else if (!strcmp(func_name, "transform::fourier")) {
          delete[] func_node->p_chdata;
          char *tmp = new char[9];
          strcpy(tmp, "&#x2131;");
          func_node->p_chdata = tmp;
          sem_type = SEM_TYP_TRANSFORM;
        }
      }
      break;

    case 21:{
        if (!strcmp(func_name, "transform::invlaplace")) {
          delete[] func_node->p_chdata;
          char *tmp = new char[9];
          strcpy(tmp, "&#x2112;");
          func_node->p_chdata = tmp;
          sem_type = SEM_TYP_INVTRANSFORM;
        } else if (!strcmp(func_name, "transform::invfourier")) {
          delete[] func_node->p_chdata;
          char *tmp = new char[9];
          strcpy(tmp, "&#x2131;");
          func_node->p_chdata = tmp;
          sem_type = SEM_TYP_INVTRANSFORM;
        }
      }
      break;

    default:
      break;
    }

  } else
    TCI_ASSERT(0);

  if (!done && func_name) {
    snode->semantic_type = sem_type;
    if (func_node->p_chdata) {
      NAME2AKA_REC *back_map = EngNameToCanonicalName(func_node->p_chdata);
      if (back_map) {           // user function seen in input
        if (back_map->src_canonical_name) {
          size_t zln = strlen(back_map->src_canonical_name);
          char *tmp = new char[zln + 1];
          strcpy(tmp, back_map->src_canonical_name);
          snode->canonical_ID = tmp;
        }
        if (back_map->user_name) {
          size_t zln = strlen(back_map->user_name);
          char *tmp = new char[zln + 1];
          strcpy(tmp, back_map->user_name);
          snode->contents = tmp;
        }
      } else {
        size_t zln = strlen(func_node->p_chdata);
        char *tmp = new char[zln + 1];
        strcpy(tmp, func_node->p_chdata);
        snode->contents = tmp;
      }
    }

    MNODE* args_node = func_node->first_kid;
    if (ElementNameIs(args_node, "arglist")) {
      BUCKET_REC* item_list = NULL;
      if (args_node->first_kid) {
        // locate items in this fence
        MNODE* item_rover = args_node->first_kid;
        while (item_rover) {
          if (item_rover->first_kid) {
            MNODE* contents = item_rover->first_kid;
            BUCKET_REC* item_bucket = MakeBucketRec(MB_UNNAMED, NULL);
            item_list = AppendBucketRec(item_list, item_bucket);

            SEMANTICS_NODE* s_cell = GetSemanticsList(contents, item_bucket);
            item_bucket->first_child = s_cell;
          }
          item_rover = item_rover->next;
        }
      }
      snode->bucket_list = item_list;
      if (!strcmp(func_name, "log")
          && snode->bucket_list) {
        // MuPAD scripts the log base as the first arg - "log(10,x)"
        snode->bucket_list->bucket_ID = MB_LOG_BASE;
      }
    }
  }
}

void MuAnalyzer::AnalyzePIECEWISE(MNODE * PIECEWISE_node,
                                  SEMANTICS_NODE * snode,
                                  int & local_nodes_done)
{
  local_nodes_done = 1;
  snode->semantic_type = SEM_TYP_PIECEWISE_LIST;
  if (PIECEWISE_node->p_chdata) {
    size_t zln = strlen(PIECEWISE_node->p_chdata);
    char *tmp = new char[zln + 1];
    strcpy(tmp, PIECEWISE_node->p_chdata);
    snode->contents = tmp;
  }

  MNODE *pieceslist = PIECEWISE_node->first_kid;
  if (ElementNameIs(pieceslist, "arglist")) {
    BUCKET_REC *item_list = NULL;
    if (pieceslist->first_kid) {
      MNODE *arg_rover = pieceslist->first_kid;
      while (arg_rover) {
        if (arg_rover->first_kid) {
          MNODE *contents = arg_rover->first_kid;
          BUCKET_REC *item_bucket = MakeBucketRec(MB_UNNAMED, NULL);
          item_list = AppendBucketRec(item_list, item_bucket);

          SEMANTICS_NODE *s_item = GetSemanticsList(contents, item_bucket);
          item_bucket->first_child = s_item;
          /*
            <arg_bucket>					-> arg_rover
              <blist>									ONE_PIECE
                <item_bucket>							Bucket list
                  <variable>q1a</variable>			  UNNAMED
                  <operator>=</operator>				    VARIABLE contents = "a"
                  <number>0</number>					    OPERATOR contents = "="
                  <variable>and</variable>			    NUMBER contents = "0"
                  <variable>q1b</variable>			    TEXT contents = "and"
                  <operator>=</operator>				    VARIABLE contents = "b"
                  <number>0</number>					    OPERATOR contents = "="
                  <variable>and</variable>			    NUMBER contents = "0"
                  <variable>q1c</variable>			    TEXT contents = "and"
                  <operator>=</operator>				    VARIABLE contents = "c"
                  <number>0</number>					    OPERATOR contents = "="
                </item_bucket>						    NUMBER contents = "0"
                <item_bucket>							  PIECE_EXPR
                  <constant>C_</constant>				    UCONSTANT contents = "C"
                </item_bucket>						End Bucket list
              </blist>
            </arg_bucket>
          */
          if (s_item->semantic_type == SEM_TYP_BRACKETED_LIST) {
            s_item->semantic_type = SEM_TYP_ONE_PIECE;
            if (s_item->bucket_list) {
              BUCKET_REC *item_bucket = s_item->bucket_list;
              item_bucket->next->bucket_ID = MB_PIECE_EXPR;
            } else
              TCI_ASSERT(0);
          } else
            TCI_ASSERT(0);
        }
        arg_rover = arg_rover->next;
      }
    }
    snode->bucket_list = item_list;
  }
}

// (numerator)/(denominator) already converted to frac{num}{den}

void MuAnalyzer::AnalyzeFRACTION(MNODE * fraction_node,
                                 SEMANTICS_NODE * snode,
                                 int & local_nodes_done)
{
  MNODE* numerator_node = fraction_node->first_kid;
  SEMANTICS_NODE* numerator = CreateSemanticsNode();
  SNodeFromOneMNode(numerator, numerator_node, local_nodes_done);
  numerator = RemoveRedundantPrecGroups(numerator);

  SEMANTICS_NODE* denominator = CreateSemanticsNode();
  if (numerator_node->next) {
    SNodeFromOneMNode(denominator, numerator_node->next, local_nodes_done);
    denominator = RemoveRedundantPrecGroups(denominator);
  } else
    TCI_ASSERT(0);

  BUCKET_REC *num_bucket = MakeBucketRec(MB_NUMERATOR, NULL);
  snode->bucket_list = AppendBucketRec(snode->bucket_list, num_bucket);
  num_bucket->first_child = numerator;
  numerator->parent = num_bucket;
  numerator = RemoveParens(numerator);

  BUCKET_REC *den_bucket = MakeBucketRec(MB_DENOMINATOR, NULL);
  snode->bucket_list = AppendBucketRec(snode->bucket_list, den_bucket);
  den_bucket->first_child = denominator;
  denominator->parent = den_bucket;
  denominator = RemoveParens(denominator);

  local_nodes_done = 1;
  snode->semantic_type = SEM_TYP_FRACTION;
}

void MuAnalyzer::AnalyzeMIXEDNUM(MNODE * mixnum_node,
                                 SEMANTICS_NODE * snode,
                                 int & local_nodes_done)
{

  MNODE* whole_node = mixnum_node->first_kid;
  SEMANTICS_NODE* s_whole = CreateSemanticsNode();
  SNodeFromOneMNode(s_whole, whole_node, local_nodes_done);
  s_whole = RemoveRedundantPrecGroups(s_whole);

  SEMANTICS_NODE* s_fraction = CreateSemanticsNode();
  if (whole_node->next) {
    SNodeFromOneMNode(s_fraction, whole_node->next, local_nodes_done);
    s_fraction = RemoveRedundantPrecGroups(s_fraction);
  } else
    TCI_ASSERT(0);

  BUCKET_REC* num_bucket = MakeParentBucketRec(MB_MN_WHOLE, s_whole);
  s_whole = RemoveParens(s_whole);

  BUCKET_REC* den_bucket = MakeParentBucketRec(MB_MN_FRACTION, s_fraction);
  
  s_fraction = RemoveParens(s_fraction);

  snode->bucket_list = AppendBucketRec(snode->bucket_list, num_bucket);
  snode->bucket_list = AppendBucketRec(snode->bucket_list, den_bucket);

  local_nodes_done = 1;
  snode->semantic_type = SEM_TYP_MIXEDNUMBER;
}

// (numerator)^(denominator) already converted to msup{base}{power}

void MuAnalyzer::AnalyzeSUPERSCRIPT(MNODE * msup_node,
                                    SEMANTICS_NODE * snode,
                                    int & local_nodes_done)
{
  MNODE *base_node = msup_node->first_kid;
  SEMANTICS_NODE *base = CreateSemanticsNode();
  SNodeFromOneMNode(base, base_node, local_nodes_done);
  base = RemoveRedundantPrecGroups(base);

  SEMANTICS_NODE *superscript = CreateSemanticsNode();
  if (base_node->next) {
    SNodeFromOneMNode(superscript, base_node->next, local_nodes_done);
    superscript = RemoveRedundantPrecGroups(superscript);
  }

  BUCKET_REC *base_bucket = MakeBucketRec(MB_SCRIPT_BASE, NULL);
  snode->bucket_list = AppendBucketRec(snode->bucket_list, base_bucket);
  base_bucket->first_child = base;
  base->parent = base_bucket;
  base = RemoveParens(base);

  BUCKET_REC *script_bucket = MakeBucketRec(MB_SCRIPT_UPPER, NULL);
  snode->bucket_list = AppendBucketRec(snode->bucket_list, script_bucket);
  script_bucket->first_child = superscript;
  superscript->parent = script_bucket;
  superscript = RemoveParens(superscript);

  local_nodes_done = 1;
  snode->semantic_type = SEM_TYP_POWERFORM;
}

void MuAnalyzer::AnalyzeVARIABLE(MNODE * eng_node, SEMANTICS_NODE * snode,
                                 int & local_nodes_done)
{
  local_nodes_done = 1;

  const char *v_str = eng_node->p_chdata;
  if (!v_str) {
    TCI_ASSERT(0);
    return;
  }

  bool done = false;
  size_t vln = strlen(v_str);
  if (vln > 2 && !strcmp(v_str + vln - 2, "QU")) {
    const char *db_production;
    U32 ID, subID;
    if (
        GetdBaseNamedRecord(myGrammar, "SIBASEUNITS", v_str, &db_production, ID,
                            subID)) {
      if (db_production && *db_production) {
        size_t zln = strlen(db_production);
        char *tmp = new char[zln + 1];
        strcpy(tmp, db_production);
        snode->contents = tmp;

        snode->semantic_type = SEM_TYP_SIUNIT;
        done = true;
      }
    }
  }

  if (!done) {
    snode->semantic_type = SEM_TYP_VARIABLE;

    NAME2AKA_REC *back_map = EngNameToCanonicalName(eng_node->p_chdata);

    if (back_map) {             // variable directly from input

      if (back_map->src_canonical_name && back_map->src_canonical_name[0]) {
        size_t zln = strlen(back_map->src_canonical_name);
        char *tmp = new char[zln + 1];
        strcpy(tmp, back_map->src_canonical_name);
        snode->canonical_ID = tmp;
      } else if (back_map->user_name && back_map->user_name[0]) {
        size_t zln = strlen(back_map->user_name);
        char *tmp = new char[zln + 1];
        strcpy(tmp, back_map->user_name);
        snode->contents = tmp;
      }
      // A "qlist" child on a "variable" indicates that the variable
      //  is qualified, as in "a_{1,2}"

      if (eng_node->first_kid && eng_node->first_kid->first_kid) {
        SEMANTICS_NODE *s_qual = CreateSemanticsNode();
        int local_nodes_done;
        AnalyzeBRACKETLIST(eng_node->first_kid, s_qual, local_nodes_done);

        BUCKET_REC* bucket = MakeParentBucketRec(MB_UNNAMED, s_qual);
        snode->bucket_list = AppendBucketRec(snode->bucket_list, bucket);
        bucket->first_child = s_qual;
        
      }
    } else {                    // variable generated by Maple, "u1"
      //  may be numbered
      const char *var_nom = eng_node->p_chdata;
      size_t zln = strlen(var_nom);

      if (IsText(var_nom)) {
        snode->semantic_type = SEM_TYP_TEXT;

        char *tmp = new char[zln + 1];
        strcpy(tmp, var_nom);
        snode->contents = tmp;
      } else {
        char *tmp_nom = new char[zln + 1];
        if (zln > 1 && var_nom[1] >= '0' && var_nom[1] <= '9') {
          BUCKET_REC* obj_bucket = MakeBucketRec(MB_UNNAMED, snode);
          snode->bucket_list = AppendBucketRec(snode->bucket_list, obj_bucket);

          SEMANTICS_NODE *s_sub = CreateSemanticsNode(SEM_TYP_NUMBER);
          //s_sub->semantic_type = SEM_TYP_NUMBER;
          char *tmp = new char[zln];
          strcpy(tmp, var_nom + 1);
          s_sub->contents = tmp;

          obj_bucket->first_child = s_sub;
          s_sub->parent = obj_bucket;
          //          var_nom[1]  =  0;
          tmp_nom[0] = var_nom[0];
          tmp_nom[1] = 0;
        } else {
          strcpy(tmp_nom, var_nom);
        }
        snode->contents = tmp_nom;
      }
    }
  }
}

// Change the following so that snode->contents receives unicode entities

void MuAnalyzer::AnalyzeOPERATOR(MNODE * eng_node, SEMANTICS_NODE * snode,
                                 int & local_nodes_done)
{
  local_nodes_done = 1;

  snode->semantic_type = SEM_TYP_INFIX_OP;
  if (eng_node->p_chdata) {
    size_t zln = strlen(eng_node->p_chdata);
    int p = 0;
    char buffer[80];
    buffer[0] = 0;
    if (zln == 1) {
      char ch0 = eng_node->p_chdata[0];

      if (ch0 == '+') {
        strcpy(buffer, "+");
        p = 27;
      }                         // 0x002b
      else if (ch0 == '-') {
        strcpy(buffer, "&#x2212;");
        p = 27;
      }                         // 0x2212
      else if (ch0 == '*') {
        strcpy(buffer, "&#xd7;");
        p = 28;
      }                         // 0x00d7
      else if (ch0 == '/') {
        TCI_ASSERT(0);
        p = 28;
      }                         // 0x00f7  strcpy( buffer,"&divide;")
      else if (ch0 == '=') {
        strcpy(buffer, "=");
        p = 24;
      }                         // 0x002b
      else if (ch0 == '<') {
        strcpy(buffer, "&#x3c;");
        p = 24;
      }                         // 0x003c
      else if (ch0 == '>') {
        strcpy(buffer, ">");
        p = 24;
      }                         // 0x003e
      else if (ch0 == ',') {
        strcpy(buffer, ",");
        p = 18;
      } else if (ch0 == '|') {
        strcpy(buffer, "|");
        p = 18;
      } else if (ch0 == '!') {
        strcpy(buffer, "!");
        p = 30;
      } else if (ch0 == '^') {
        TCI_ASSERT(0);
        p = 29;
      }                         // 0x005e  strcpy( buffer,"&;")
      if (ch0 == '!')
        snode->semantic_type = SEM_TYP_POSTFIX_OP;

      if (!eng_node->prev)
        if (ch0 == '+' || ch0 == '-') {
          snode->semantic_type = SEM_TYP_PREFIX_OP;
          p = 0;
        }
    } else if (zln == 2) {
      char ch0 = eng_node->p_chdata[0];
      char ch1 = eng_node->p_chdata[1];

      if (ch0 == '<' && ch1 == '=') {
        strcpy(buffer, "&#x2264;");
        p = 24;
      } else if (ch0 == '>' && ch1 == '=') {
        strcpy(buffer, "&#x2265;");
        p = 24;
      } else if (ch0 == '<' && ch1 == '>') {
        strcpy(buffer, "&#x2260;");
        p = 24;
      } else if (ch0 == '.' && ch1 == '.') {
        strcpy(buffer, "..");
        p = 25;
      } else if (ch0 == ':' && ch1 == ':') {
        strcpy(buffer, "::");
        p = 30;
      } else if (ch0 == 'i' && ch1 == 'n') {
        strcpy(buffer, "&#x220a;");
        p = 25;
      } else if (ch0 == 'o' && ch1 == 'r') {
        strcpy(buffer, "&#x2228;");
        p = 20;
      }
    } else if (zln == 3) {
      if (!strcmp(eng_node->p_chdata, "and")) {
        strcpy(buffer, "&#x2227;");
        p = 21;
      } else if (!strcmp(eng_node->p_chdata, "not")) {
        strcpy(buffer, "&#xac;");
        snode->semantic_type = SEM_TYP_PREFIX_OP;
        p = 22;
      }
    } else if (zln == 5) {
      if (!strcmp(eng_node->p_chdata, "union")) {
        strcpy(buffer, "&#x222a;");
        p = 27;
      } else if (!strcmp(eng_node->p_chdata, "minus")) {
        strcpy(buffer, "&#x2216;");
        p = 27;
      }
    } else if (zln == 6) {
      if (!strcmp(eng_node->p_chdata, "mnjoin")) {
        strcpy(buffer, "mnjoin");
        p = 31;
      } else if (!strcmp(eng_node->p_chdata, "approx")){
        strcpy(buffer, "&#x2248;");
        p = 24;
      }
    } else if (zln == 9) {
      if (!strcmp(eng_node->p_chdata, "intersect")) {
        strcpy(buffer, "&#x2229;");
        p = 28;
      }
    }
    if (!buffer[0]) {
      TCI_ASSERT(0);
      if (zln < 80)
        strcpy(buffer, eng_node->p_chdata);
    }
    size_t bln = strlen(buffer);
    char *tmp = new char[bln + 1];
    strcpy(tmp, buffer);
    snode->contents = tmp;
    if (p)
      snode->infix_precedence = p;
  } else
    TCI_ASSERT(0);
}

void MuAnalyzer::AnalyzeCONSTANT(MNODE * eng_node,
                                 SEMANTICS_NODE * snode,
                                 int & local_nodes_done)
{
  local_nodes_done = 1;
  snode->semantic_type = SEM_TYP_UCONSTANT;
  if (eng_node->p_chdata) {
    const char *c_str = eng_node->p_chdata;
    size_t zln = strlen(c_str);
    // Look for SWP units
    if (zln > 2 && !strcmp(c_str + zln - 2, "QU")) {
      const char *db_production;
      U32 ID, subID;
      if (
          GetdBaseNamedRecord(myGrammar,"SIBASEUNITS", c_str, &db_production, ID,
                              subID)) {
        if (db_production && *db_production) {
          size_t zln = strlen(db_production);
          char *tmp = new char[zln + 1];
          strcpy(tmp, db_production);
          snode->contents = tmp;

          snode->semantic_type = SEM_TYP_SIUNIT;
        }
      }
    } else if (zln) {
      char buffer[80];
      buffer[0] = 0;
      // engine generated constants and variables
      if (c_str[0] == 'C' || c_str[0] == 'X') {
        char ch1 = c_str[1];
        if (ch1 >= '0' && ch1 <= '9')
          strcpy(buffer, c_str);
      }
      if (!buffer[0]) {
        if (zln == 1) {
          char ch0 = c_str[0];
          if (ch0 == 'E')
            strcpy(buffer, "&#x2147;"); // exponential e
          else if (ch0 == 'I')
            strcpy(buffer, "&#x2148;"); // imaginary i
          else {
            if (ch0 != 'X')
              TCI_ASSERT(0);
            strcpy(buffer, c_str);
          }
        } else if (zln == 2) {
          char ch0 = c_str[0];
          char ch1 = c_str[1];

          if (ch0 == 'P' && ch1 == 'I')
            strcpy(buffer, "&#x3c0;");  // &pi;

          else if (ch1 == '_') {  // Z_, Q_, R_, and C_
            if (ch0 == 'C') {
              strcpy(buffer, "&#x2102;"); // bbC
            } else if (ch0 == 'Q') {
              strcpy(buffer, "&#x211a;"); // bbQ
            } else if (ch0 == 'R') {
              strcpy(buffer, "&#x211d;"); // bbR
            } else if (ch0 == 'Z') {
              strcpy(buffer, "&#x2124;"); // bbZ
            } else {
              TCI_ASSERT(0);
              buffer[0] = ch0;
              buffer[1] = 0;
            }
          } else
            TCI_ASSERT(0);
        } else if (zln == 4) {
          if (!strcmp(c_str, "Zeta"))
            strcpy(buffer, "&#x3b6;");  // &Zeta;
          else if (!strcmp(c_str, "FAIL"))
            strcpy(buffer, "FAIL"); // undecidable
        } else if (zln == 5) {
          if (!strcmp(c_str, "EULER"))
            strcpy(buffer, "gamma");  // Euler gamma constant
        } else if (zln == 6) {
          if (!strcmp(c_str, "RD_INF"))
            strcpy(buffer, "&#x221e;");  // infinity from interval calculus
        } else if (zln == 7) {
          if (!strcmp(c_str, "UNKNOWN")) {
            strcpy(buffer, "UNKNOWN");  // 
            if (curr_cmd_ID == CCID_Check_Equality) {
              snode->semantic_type = SEM_TYP_EQCHECK_RESULT;
              snode->variant = SNV_Undecidable;
            }
          } if (!strcmp(c_str,"RD_NINF")) {
            strcpy(buffer, "-&#x221e;");  // wrong, if not super-ugly
          }
        } else if (zln == 9) {
          if (!strcmp(c_str, "undefined"))
            strcpy(buffer, "undefined");  // undefined value
        }
      }
      if (!buffer[0]) {
        if (zln < 80)
          strcpy(buffer, c_str);
      }
      size_t bln = strlen(buffer);
      char *tmp = new char[bln + 1];
      strcpy(tmp, buffer);
      snode->contents = tmp;
    }
  }
}

void MuAnalyzer::AnalyzeSUBSTITUTION(MNODE* eng_node,
                                SEMANTICS_NODE* snode,
                                int& local_nodes_done)
{
   BUCKET_REC* main_bucket_list = NULL;

   local_nodes_done = 1;

   snode->semantic_type = SEM_TYP_SUBSTITUTION;
   snode->contents = DuplicateString("substitution");
   
   MNODE* contents = eng_node->first_kid;
   MNODE* subst = eng_node->first_kid->next;

   // de-link the two parts
   contents->next = NULL;
   subst->prev = NULL;

   // Deal with contents. 
   //  1. Put the contents sem list into an unnamed bucket
   //  2. Put that bucket into a fence
   //  3. Put the fence into an unnamed bucket
   BUCKET_REC* fence_item_bucket_list = NULL;
   BUCKET_REC* fence_item_bucket = MakeBucketRec(MB_UNNAMED, NULL);
   fence_item_bucket_list = AppendBucketRec(fence_item_bucket_list, fence_item_bucket);
   SEMANTICS_NODE* element = GetSemanticsList(contents, fence_item_bucket);
   fence_item_bucket->first_child = element;
   element->parent = fence_item_bucket;

   // Now put element bucket in a generic fence
   SEMANTICS_NODE* fence = CreateSemanticsNode();
   fence -> semantic_type = SEM_TYP_GENERIC_FENCE;  
   fence->bucket_list = fence_item_bucket_list;

   // Put the fence in an unnamed bucket
   BUCKET_REC* fence_bucket = MakeBucketRec(MB_UNNAMED, NULL);
   fence_bucket->first_child = fence;
   main_bucket_list = AppendBucketRec(main_bucket_list, fence_bucket);

   // Process subst
   BUCKET_REC* item_bucket = MakeBucketRec(MB_SUBST_LOWER, NULL);
   main_bucket_list = AppendBucketRec(main_bucket_list, item_bucket);

   SEMANTICS_NODE* element2 = GetSemanticsList(subst, item_bucket);
   item_bucket->first_child = element2;
   element2->parent = item_bucket;
   
   snode->bucket_list = main_bucket_list;


   //if (eng_node->p_chdata) {
   // size_t zln = strlen(eng_node->p_chdata);
   // char *tmp = new char[zln + 1];
   // strcpy(tmp, eng_node->p_chdata);
   // snode->contents = tmp;
   //}



}

void MuAnalyzer::AnalyzeBOOLEAN(MNODE * eng_node,
                                SEMANTICS_NODE * snode,
                                int & local_nodes_done)
{
  local_nodes_done = 1;

  snode->semantic_type = SEM_TYP_BOOLEAN;
  if (eng_node->p_chdata) {
    size_t zln = strlen(eng_node->p_chdata);
    char *tmp = new char[zln + 1];
    strcpy(tmp, eng_node->p_chdata);
    snode->contents = tmp;

    if (!strcmp(tmp, "TRUE"))
      snode->variant = SNV_True;
    else if (!strcmp(tmp, "FALSE"))
      snode->variant = SNV_False;
    if (curr_cmd_ID == CCID_Check_Equality)
      snode->semantic_type = SEM_TYP_EQCHECK_RESULT;
  } else
    TCI_ASSERT(0);
}

NAME2AKA_REC *MuAnalyzer::EngNameToCanonicalName(const char *targ_alias)
{
  NAME2AKA_REC *rv = NULL;
  if (mangle_info) {
    NAME2AKA_REC *rover = mangle_info->AKAs_list;
    while (rover) {
      if (rover->alias && !strcmp(targ_alias, rover->alias)) {
        rv = rover;
        break;
      }
      rover = rover->next;
    }

    if (!rv) {
      char ODEtarg[128];
      strcpy(ODEtarg, targ_alias);
      strcat(ODEtarg, "ODE");
      rover = mangle_info->AKAs_list;
      while (rover) {
        if (rover->alias && !strcmp(ODEtarg, rover->alias)) {
          rv = rover;
          break;
        }
        rover = rover->next;
      }
      if (rv) {
        if (rover->obj_ilk == Ilk_ODE) {
          char *ptr = strstr(rover->alias, "ODE");
          *ptr = 0;
          rover->obj_ilk = Ilk_WasODE;
        } else
          TCI_ASSERT(0);
      } else {
        // Here, the function is NOT user defined - it's "sin", etc.
      }
    }
  } else
    TCI_ASSERT(0);

  return rv;
}

void MuAnalyzer::AnalyzeSET(MNODE * eng_node, SEMANTICS_NODE * snode,
                            int & local_nodes_done)
{
  local_nodes_done = 1;
  BUCKET_REC *item_bucket_list = NULL;
  if (eng_node && eng_node->first_kid) {  // should be a "set_element_bucket"
    // locate items in this set bucket
    MNODE *item_rover = eng_node->first_kid;
    while (item_rover) {
      if (item_rover->first_kid) {
        MNODE *contents = item_rover->first_kid;
        BUCKET_REC *item_bucket = MakeBucketRec(MB_UNNAMED, NULL);
        item_bucket_list = AppendBucketRec(item_bucket_list, item_bucket);

        SEMANTICS_NODE *s_element = GetSemanticsList(contents, item_bucket);
        item_bucket->first_child = s_element;
        s_element->parent = item_bucket;
      }
      item_rover = item_rover->next;
    }
  }
  snode->bucket_list = item_bucket_list;
  snode->semantic_type = SEM_TYP_SET;
}

/* The following is a generic translation of [blah,blah,...]
  When it occurs in Maple output, the function that invokes
  semantic analysis (BuildSemanticsList) must post-process output
  from this function and put meaningful types on the snodes.
  For example, [ , , ] occurs in the engine output for matrix:Eigenvectors.
  After "BuildSemanticsList"  has generated the initial semantics tree,
  FixMapleEigenVect is called to assign meaning to the blists.
  In this case, each blist specifies an eigenvector - using Maple
  it contains an eigenvalue, a multiplicity number, and an eigenvector.
*/

void MuAnalyzer::AnalyzeBRACKETLIST(MNODE * eng_blist_node,
                                    SEMANTICS_NODE * snode,
                                    int & local_nodes_done)
{
  local_nodes_done = 1;
  snode->semantic_type = SEM_TYP_BRACKETED_LIST;
  if (ElementNameIs(eng_blist_node, "qlist"))
    snode->semantic_type = SEM_TYP_QSUB_LIST;

  BUCKET_REC *item_list = NULL;
  if (eng_blist_node->first_kid) {
    MNODE *item_rover = eng_blist_node->first_kid;
    while (item_rover) {        // loop thru items in list
      if (item_rover->first_kid) {
        MNODE *contents = item_rover->first_kid;
        BUCKET_REC *item_bucket = MakeBucketRec(MB_UNNAMED, NULL);
        item_list = AppendBucketRec(item_list, item_bucket);

        SEMANTICS_NODE *s_cell = GetSemanticsList(contents, item_bucket);
        item_bucket->first_child = s_cell;
      }
      item_rover = item_rover->next;
    }
  }
  snode->bucket_list = item_list;
}

void MuAnalyzer::AnalyzeARGLIST(MNODE * eng_node,
                                SEMANTICS_NODE * snode,
                                int & local_nodes_done)
{
  TCI_ASSERT(0);
}

// After a tree of MNODEs has been generated from a Maple return string
//  the tree is modified to bind operands to operators by precedence.
//  The generated MNODEs nodes are named "precedence_group"s.
//  Sometimes the contents of a "precedence_group" produce a single
//  SEMANTICS_NODE. In this case the pgroup is redundant and is discarded.

void MuAnalyzer::AnalyzePRECGROUP(MNODE * prec_node,
                                  SEMANTICS_NODE * snode, int & nodes_done)
{
  nodes_done = 1;
  snode->semantic_type = SEM_TYP_PRECEDENCE_GROUP;
  if (prec_node->p_chdata)
    snode->infix_precedence = atoi(prec_node->p_chdata);

  if (prec_node->first_kid) {
    MNODE *contents = prec_node->first_kid;
    SEMANTICS_NODE *s_cell = GetSemanticsList(contents, NULL);

    if (s_cell && s_cell->next) {
      BUCKET_REC *obj_bucket = MakeBucketRec(MB_UNNAMED, NULL);
      snode->bucket_list = AppendBucketRec(snode->bucket_list, obj_bucket);
      obj_bucket->first_child = s_cell;
      s_cell->parent = obj_bucket;
    } else if (s_cell) {
      // transcribe fields from s_cell to snode
      snode->error_flag = s_cell->error_flag;
      snode->semantic_type = s_cell->semantic_type;
      snode->canonical_ID = s_cell->canonical_ID;
      s_cell->canonical_ID = NULL;
      snode->contents = s_cell->contents;
      s_cell->contents = NULL;
      snode->nrows = s_cell->nrows;
      snode->ncols = s_cell->ncols;
      snode->bucket_list = s_cell->bucket_list;
      s_cell->bucket_list = NULL;
      DisposeSemanticsNode(s_cell);
    }
  }
}

void MuAnalyzer::AnalyzeCOMPONENT(MNODE * eng_node,
                                  SEMANTICS_NODE * snode,
                                  int & local_nodes_done)
{
  local_nodes_done = 1;
  BUCKET_REC *comp_bucket = NULL;
  if (eng_node->first_kid) {
    MNODE *contents = eng_node->first_kid;
    BUCKET_REC *item_bucket = MakeBucketRec(MB_UNNAMED, NULL);
    comp_bucket = AppendBucketRec(comp_bucket, item_bucket);

    SEMANTICS_NODE *s_cell = GetSemanticsList(contents, item_bucket);
    item_bucket->first_child = s_cell;
    s_cell->parent = item_bucket;
  }
  snode->bucket_list = comp_bucket;
  snode->semantic_type = SEM_TYP_PRECEDENCE_GROUP;
}

/*
        <component_bucket>
        </component_bucket>
*/

BUCKET_REC *MuAnalyzer::GetCellsFromROW(MNODE * row_container,
                                        int & col_tally)
{
  BUCKET_REC *rv = NULL;
  col_tally = 0;

  if (row_container && row_container->first_kid) {
    MNODE *contents = row_container->first_kid;
    if (ElementNameIs(contents, "blist")) {
      MNODE *col_rover = contents->first_kid;
      while (col_rover) {
        // col_rover should be a "item" node
        BUCKET_REC *cell_bucket = MakeBucketRec(MB_UNNAMED, NULL);
        rv = AppendBucketRec(rv, cell_bucket);
        SEMANTICS_NODE *curr_cell = GetSemanticsList(col_rover, cell_bucket, 1);
        cell_bucket->first_child = curr_cell;
        curr_cell->parent = cell_bucket;

        col_tally++;
        col_rover = col_rover->next;
      }
    } else {
      BUCKET_REC *cell_bucket = MakeBucketRec(MB_UNNAMED, NULL);
      rv = AppendBucketRec(rv, cell_bucket);

      SEMANTICS_NODE *curr_cell = GetSemanticsList(contents, cell_bucket);
      cell_bucket->first_child = curr_cell;
      curr_cell->parent = cell_bucket;
      col_tally = 1;
    }
  } else
    TCI_ASSERT(0);

  return rv;
}

// s_list -> matrix , matrix

void MuAnalyzer::FixMapleCholesky(SEMANTICS_NODE* s_list)
{
  while (s_list && s_list->next) {
    SEMANTICS_NODE* comma = s_list->next;
    if (comma->semantic_type == SEM_TYP_INFIX_OP && !strcmp(comma->contents, ",")) {
      delete[] comma->contents;
      char *chdata = new char[9];
      strcpy(chdata, "&#x2062;");
      comma->contents = chdata;
    }
    s_list = s_list -> next;
  }
}

// Change "," to "&InvisibleTimes;"

void MuAnalyzer::FixMapleJordanForm(SEMANTICS_NODE * s_list)
{
  SEMANTICS_NODE *rover = s_list;
  while (rover) {
    if (rover->semantic_type == SEM_TYP_INFIX_OP
        && !strcmp(rover->contents, ",")) {
      delete[] rover->contents;
      char *chdata = new char[9];
      strcpy(chdata, "&#x2062;");
      rover->contents = chdata;
    }
    rover = rover->next;
  }
}

// Initial 
// [q1x+q1x^2-1, 1, {vector([1, 1])}]   ,   [q1x-q1x^2+1, 1, {vector([-1, 1])}]

SEMANTICS_NODE *MuAnalyzer::FixMapleEigenVect(SEMANTICS_NODE * s_list)
{
  SEMANTICS_NODE *rv = CreateSemanticsNode(SEM_TYP_EIGENVECTORSET);
  //rv->semantic_type = SEM_TYP_EIGENVECTORSET;

  SEMANTICS_NODE *rover = s_list;
  while (rover) {
    SEMANTICS_NODE *s_curr = rover;
    rover = rover->next;
    if (s_curr->semantic_type == SEM_TYP_BRACKETED_LIST) {
      s_curr->semantic_type = SEM_TYP_EIGENVECTOR;

      s_curr->next = NULL;
      s_curr->prev = NULL;

      BUCKET_REC* bucket = MakeParentBucketRec(MB_UNNAMED, s_curr);
      rv->bucket_list = AppendBucketRec(rv->bucket_list, bucket);
      
      int bucket_num = 0;
      BUCKET_REC *bucket_rover = s_curr->bucket_list;
      while (bucket_rover) {
        if (bucket_num == 0)
          bucket_rover->bucket_ID = MB_EIGENVALUE;
        else if (bucket_num == 1)
          bucket_rover->bucket_ID = MB_EV_MULTIPLICITY;
        else if (bucket_num == 2)
          bucket_rover->bucket_ID = MB_EIGENVECTOR;

        bucket_num++;
        bucket_rover = bucket_rover->next;
      }
    } else {
      s_curr->next = NULL;
      s_curr->prev = NULL;
      DisposeSemanticsNode(s_curr);
    }
  }
  return rv;
}

/*
<precedence_group>          SIMPLEX contents = "Simplex Dual"
  expr						rows = 5, columns = 1
</precedence_group>			Bucket list
							  OBJECTIVE FUNCTION
<operator>,</operator>
							  UNNAMED
<set>
  <set_element_bucket>		  UNNAMED

  </set_element_bucket>		  UNNAMED
  <set_element_bucket>		    TABLE_CELL
							    Bucket list
  </set_element_bucket>		      UNNAMED
</set>						        VARIABLE canonical_ID = "mix" contents = "x"
							        OPERATOR contents = "&#x2265;"
							        NUMBER canonical_ID = "mn0" contents = "0"
							    End Bucket list
        					  UNNAMED
        
        					End Bucket list
        
*/

SEMANTICS_NODE *MuAnalyzer::FixMapleSimplexDual(SEMANTICS_NODE * s_list)
{
  SEMANTICS_NODE *s_simplex = s_list;

  if (s_list && s_list->next && s_list->next->next) {
    SEMANTICS_NODE *s_expr = s_list;
    SEMANTICS_NODE *s_comma = s_list->next;
    SEMANTICS_NODE *s_set = s_comma->next;

    s_simplex = CreateSemanticsNode(SEM_TYP_SIMPLEX);
    //s_simplex->semantic_type = SEM_TYP_SIMPLEX;
    // char *name = "Simplex Dual";
    // size_t zln = strlen(name);
    // char *tmp = new char[zln + 1];
    // strcpy(tmp, name);
    s_simplex->contents = DuplicateString("Simplex Dual");

    BUCKET_REC* expr_bucket = MakeBucketRec(MB_SIMPLEX_EXPR, s_simplex);
    s_simplex->bucket_list = AppendBucketRec(s_simplex->bucket_list, expr_bucket);
    expr_bucket->first_child = s_expr;
    s_expr->parent = expr_bucket;

    s_expr->next = NULL;
    s_comma->prev = NULL;

    if (strcmp(s_comma->contents, ","))
      TCI_ASSERT(0);

    s_comma->next = NULL;
    s_set->prev = NULL;
    DisposeSList(s_comma);

    BUCKET_REC *set_bucket = s_set->bucket_list;
    s_set->bucket_list = NULL;
    DisposeSList(s_set);

    s_simplex->bucket_list =
      AppendBucketRec(s_simplex->bucket_list, set_bucket);
    while (set_bucket) {
      set_bucket->bucket_ID = MB_UNNAMED;
      set_bucket = set_bucket->next;
    }
  }
  return s_simplex;
}

void MuAnalyzer::FixMapleSimplexStandardize(SEMANTICS_NODE * s_list)
{
  SEMANTICS_NODE *rover = s_list;
  while (rover) {
    rover = rover->next;
  }
}

SEMANTICS_NODE *MuAnalyzer::FixQuantileResult(SEMANTICS_NODE * s_list)
{
  SEMANTICS_NODE* s_qresult = s_list;

  if (s_list && s_list->next && s_list->next->next) {
    SEMANTICS_NODE* s_percent = s_list;
    SEMANTICS_NODE* s_comma = s_list->next;
    SEMANTICS_NODE* s_answer = s_comma->next;

    s_qresult = CreateSemanticsNode(SEM_TYP_QUANTILE_RESULT);
    //s_qresult->semantic_type = SEM_TYP_QUANTILE_RESULT;
    // char *name = "Quantile Result";
    // size_t zln = strlen(name);
    // char *tmp = new char[zln + 1];
    // strcpy(tmp, name);
    s_qresult->contents = DuplicateString("Quantile Result");

    BUCKET_REC* percent_bucket = MakeBucketRec(MB_WHICH_QUANTILE, NULL);
    s_qresult->bucket_list =  AppendBucketRec(s_qresult->bucket_list, percent_bucket);
    percent_bucket->first_child = s_percent;
    s_percent->parent = percent_bucket;

    s_percent->next = NULL;
    s_comma->prev = NULL;

    if (!StringEqual(s_comma->contents, ","))
      TCI_ASSERT(0);

    s_comma->next = NULL;
    s_answer->prev = NULL;
    DisposeSList(s_comma);

    BUCKET_REC *answer_bucket = MakeBucketRec(MB_UNNAMED, NULL);
    s_qresult->bucket_list =  AppendBucketRec(s_qresult->bucket_list, answer_bucket);
    answer_bucket->first_child = s_answer;
    s_answer->parent = answer_bucket;
  }

  return s_qresult;
}



SEMANTICS_NODE *MuAnalyzer::FixDefinitenessTests(SEMANTICS_NODE * s_list)
{
  char* cont  = s_list->contents;
  if (cont){
    if (0 == strcmp(cont, "indef")) {
       s_list -> contents = DuplicateString("indefinite");
    } else if (0 == strcmp(cont, "posdef")) {
       s_list -> contents = DuplicateString("positive definite");
    }
  }
  return s_list;
}


SEMANTICS_NODE* MuAnalyzer::RemoveParens(SEMANTICS_NODE * expr)
{
  SEMANTICS_NODE *rv = expr;

  if (expr
      && expr->semantic_type == SEM_TYP_PARENED_LIST
      && expr->bucket_list && expr->next == NULL) {
    BUCKET_REC *save_parent = expr->parent;
    rv = expr->bucket_list->first_child;
    expr->bucket_list->first_child = NULL;
    rv->parent = save_parent;
    save_parent->first_child = rv;

    DisposeSList(expr);
  }

  return rv;
}

MNODE *MuAnalyzer::InfixToPrefix(MNODE * eng_tree, char *z_op)
{
  MNODE *rv = eng_tree;

  MNODE *rover = eng_tree;
  while (rover) {
    if (ElementNameIs(rover, "operator")
        && rover->p_chdata && !strcmp(rover->p_chdata, z_op)) {
      MNODE *left_operand = rover->prev;
      MNODE *right_operand = rover->next;
      if (left_operand && right_operand) {
        MNODE *left_anchor = left_operand->prev;
        MNODE *right_anchor = right_operand->next;

        left_operand->next = right_operand;
        right_operand->prev = left_operand;

        MNODE *schemata = MakeTNode(0, 0, 0);
        if (rover->p_chdata[0] == '/')
		      SetElementName(schemata, "fraction");
        else if (rover->p_chdata[0] == '^')
          SetElementName(schemata, "power_form");
        else if (rover->p_chdata[0] == 'm')
          SetElementName(schemata, "mixed_num");
        else if (rover->p_chdata[0] == '|')
          SetElementName(schemata, "substitution");
        else
          TCI_ASSERT(0);

        rover->next = NULL;
        rover->prev = NULL;
        DisposeTNode(rover);
        rover = schemata;

        schemata->first_kid = left_operand;
        schemata->parent = left_operand->parent;

        if (left_anchor) {
          left_anchor->next = schemata;
          schemata->prev = left_anchor;
        } else {
          if (left_operand->parent)
            left_operand->parent->first_kid = schemata;
          else
            rv = schemata;
        }

        if (right_anchor) {
          right_anchor->prev = schemata;
          schemata->next = right_anchor;
        }
        right_operand->next = NULL;

        InfixToPrefix(left_operand, z_op);
        InfixToPrefix(right_operand, z_op);
      } else
        TCI_ASSERT(0);
    } else if (rover->first_kid) {
      InfixToPrefix(rover->first_kid, z_op);
    }
    rover = rover->next;
  }

  return rv;
}

SEMANTICS_NODE *MuAnalyzer::RemoveRedundantPrecGroups(SEMANTICS_NODE * s_node)
{
  SEMANTICS_NODE *rv = s_node;

  if (s_node && s_node->bucket_list
      && (s_node->semantic_type == SEM_TYP_PRECEDENCE_GROUP
          || s_node->semantic_type == SEM_TYP_PARENED_LIST)) {
    SEMANTICS_NODE *s_child = s_node->bucket_list->first_child;
    if (s_child) {
      rv = s_child;
      rv->parent = NULL;
      s_node->bucket_list->first_child = NULL;
      DisposeSList(s_node);
    }
  }

  return rv;
}

bool MuAnalyzer::IsText(const char *var_nom)
{
  bool rv = false;

  if (var_nom) {
    if (!strcmp(var_nom, "and"))
      rv = true;
    else if (!strcmp(var_nom, "otherwise"))
      rv = true;
  }

  return rv;
}

// array(1..2, 1..2, (1,1) = q1x, (1,2) = - q1x^2 + 1, (2,1) = - q1x^2 + 1, (2,2) = q1x)

bool MuAnalyzer::IsArrayDimen(MNODE * arg, U32 & n_start, U32 & n_end)
{
  bool rv = false;

  if (arg && arg->first_kid) {
    MNODE* rover = arg->first_kid;
    if (ElementNameIs(rover, "number")) {
      n_start = atoi(rover->p_chdata);
      rover = rover->next;
      if (ElementNameIs(rover, "operator") && ContentIs(rover, "..")) {
        rover = rover->next;
        if (ElementNameIs(rover, "number")) {
          rv = true;
          if (rover->p_chdata)
            n_end = atoi(rover->p_chdata);
        }
      }
    }
  }
  return rv;
}

bool MuAnalyzer::IsEngineToken(const char *eng_token)
{
  bool rv = false;

  size_t zln = strlen(eng_token);
  switch (zln) {
  case 3:{
      if (!strcmp(eng_token, "..."))
        rv = true;
    }
    break;
  case 4:{
      if (!strcmp(eng_token, "Type"))
        rv = true;
      else if (!strcmp(eng_token, "Real"))
        rv = true;
    }
    break;
  case 5:{
      if (!strcmp(eng_token, "NoSol"))
        rv = true;
    }
    break;
  case 7:{
      if (!strcmp(eng_token, "Complex"))
        rv = true;
    }
    break;
  default:
    TCI_ASSERT(0);
    break;
  }

  return rv;
}

// Here, snode is of type SEM_TYP_DERIVATIVE.
// Append a bucket of type MB_NPRIMES to snode->bucket_list.

void MuAnalyzer::AddPrimesCount(SEMANTICS_NODE * snode, int n_primes)
{
  SEMANTICS_NODE* s_nprimes = CreateSemanticsNode();
  BUCKET_REC* pr_bucket = MakeParentBucketRec(MB_NPRIMES, s_nprimes);
  snode->bucket_list = AppendBucketRec(snode->bucket_list, pr_bucket);
  

  s_nprimes->semantic_type = SEM_TYP_NUMBER;
  char buffer[32];
  sprintf(buffer, "%d", n_primes);
  size_t zln = strlen(buffer);
  char *tmp = new char[zln + 1];
  strcpy(tmp, buffer);
  s_nprimes->contents = tmp;
}

// Engine output - "diff(q1yDIM(q1x), q1x)"
// this could be a partial derivative

void MuAnalyzer::AnalyzeDIFF(MNODE * func_node,
                             SEMANTICS_NODE * snode, int & local_nodes_done)
{
  local_nodes_done = 1;

  int n_primes;
  if (IsDerivative(func_node, n_primes))
    snode->semantic_type = SEM_TYP_DERIVATIVE;
  else
    snode->semantic_type = SEM_TYP_PARTIALD;

  MNODE *args_list = func_node->first_kid;
  if (ElementNameIs(args_list, "arglist")) {
    if (args_list->first_kid) {
      // First arg is the function being differentiated
      MNODE *arg_rover = args_list->first_kid;
      if (arg_rover->first_kid) {
        BUCKET_REC *item_bucket = MakeBucketRec(MB_UNNAMED, NULL);
        snode->bucket_list = AppendBucketRec(snode->bucket_list, item_bucket);

        MNODE *contents = arg_rover->first_kid;
        SEMANTICS_NODE *s_cell = GetSemanticsList(contents, item_bucket);
        item_bucket->first_child = s_cell;
      }
      arg_rover = arg_rover->next;
      // Remaining args are variables of differentiation
      //   diff(q1u(q1t), q1t, q1t, q1t)
      while (arg_rover) {
        if (arg_rover->first_kid) {
          BUCKET_REC *item_bucket = MakeBucketRec(MB_DIFF_VAR, NULL);
          snode->bucket_list =
            AppendBucketRec(snode->bucket_list, item_bucket);

          MNODE *contents = arg_rover->first_kid;
          SEMANTICS_NODE *s_curr_var = GetSemanticsList(contents, NULL);
          // Look ahead for more instances of the same variable
          int cv_tally = 1;
          while (arg_rover) {
            if (arg_rover->next
                && arg_rover->first_kid
                && arg_rover->next->first_kid
                && arg_rover->first_kid->p_chdata
                && arg_rover->next->first_kid->p_chdata
                && !strcmp(arg_rover->first_kid->p_chdata,
                           arg_rover->next->first_kid->p_chdata)) {
              cv_tally++;
              arg_rover = arg_rover->next;
            } else
              break;
          }
          if (cv_tally == 1) {
            // single instance of the variable of differentiation
            item_bucket->first_child = s_curr_var;
            s_curr_var->parent = item_bucket;
          } else {
            // multiple instances of the variable of differentiation
            SEMANTICS_NODE *s_power = CreateSemanticsNode(SEM_TYP_POWERFORM);
            //s_power->semantic_type = SEM_TYP_POWERFORM;
            item_bucket->first_child = s_power;
            s_power->parent = item_bucket;

            BUCKET_REC* base_bucket = MakeParentBucketRec(MB_SCRIPT_BASE, s_curr_var);
            s_power->bucket_list = AppendBucketRec(s_power->bucket_list, base_bucket);
            
            SEMANTICS_NODE* s_sup = CreateSemanticsNode(SEM_TYP_NUMBER);
            //s_sup->semantic_type = SEM_TYP_NUMBER;
            char buffer[32];
            StrFromInt(cv_tally, buffer);
            // size_t zln = strlen(buffer);
            // char *tmp = new char[zln + 1];
            // strcpy(tmp, buffer);
            s_sup->contents = DuplicateString(buffer);

            BUCKET_REC* script_bucket = MakeParentBucketRec(MB_SCRIPT_UPPER, s_sup);
            s_power->bucket_list = AppendBucketRec(s_power->bucket_list, script_bucket);
            
          }
        }
        arg_rover = arg_rover->next;
      }
    } else
      TCI_ASSERT(0);

    if (snode->semantic_type == SEM_TYP_DERIVATIVE) {
      TCI_ASSERT(n_primes);
      AddPrimesCount(snode, n_primes);
    }
  } else
    TCI_ASSERT(0);
}

/*
<function name="diff">
  <arglist>
    <arg_bucket>
      <function name="q1y">
        <arglist>
          <arg_bucket>
            <variable>q1x</variable>
          </arg_bucket>
        </arglist>
      </function>
    </arg_bucket>
    <arg_bucket>
      <variable>q1x</variable>
    </arg_bucket>
  </arglist>
</function>
*/

bool MuAnalyzer::IsDerivative(MNODE * func_node, int & n_primes)
{
  bool rv = false;
  n_primes = 0;

  if (func_node && func_node->first_kid) {
    MNODE *args_list = func_node->first_kid;
    if (args_list->first_kid) {
      MNODE *arg_rover = args_list->first_kid;
      // First arg is the function or expression being differentiated
      const char *the_var = NULL;
      if (arg_rover->first_kid) { // enter first argument
        MNODE* cont = arg_rover->first_kid;
        if (ElementNameIs(cont, "function")) {
          MNODE* a_list = cont->first_kid;
          if (ElementNameIs(a_list, "arglist")) {
            if (a_list->first_kid) {
              MNODE *a_rover = a_list->first_kid;
              if (a_rover && a_rover->first_kid && !a_rover->next) {
                // func of single variable
                MNODE *var = a_rover->first_kid;
                if (ElementNameIs(var, "variable"))
                  the_var = var->p_chdata;
              }
            }
          }
        }
      }

      if (the_var) {
        arg_rover = arg_rover->next;
        // Remaining args are variables of differentiation
        //   diff(q1u(q1t), q1t, q1t, q1t)
        int tally = 0;
        int cv_tally = 0;
        while (arg_rover) {
          tally++;
          if (arg_rover->first_kid
              && arg_rover->first_kid->p_chdata
              && !strcmp(arg_rover->first_kid->p_chdata, the_var))
            cv_tally++;

          arg_rover = arg_rover->next;
        }
        if (cv_tally && cv_tally == tally) {
          // Only 1 variable of differentiation
          rv = true;
          n_primes = tally;
        }
      }
    }
  } else {
    TCI_ASSERT(0);
  }
  return rv;
}

/*
   1 <derivative name="1">
   1   <function name="q1f">
   1     <arglist>
   1       <arg_bucket>
   1         <function name="q1g">
   1           <arglist>
   1             <arg_bucket>
   1               <variable>q1x</variable>
   1             </arg_bucket>
   1           </arglist>
   1         </function>
   1       </arg_bucket>
   1     </arglist>
   1   </function>
   1 </derivative>
*/

MNODE *MuAnalyzer::LocateDiffVar(MNODE * deriv_node)
{
  MNODE *rv = NULL;
  if (deriv_node && deriv_node->first_kid) {
    MNODE *rover = deriv_node->first_kid;
    while (rover
           && rover->first_kid
           && rover->first_kid->first_kid
           && rover->first_kid->first_kid->first_kid) {
      MNODE *cont = rover->first_kid->first_kid->first_kid;
      if (ElementNameIs(cont, "variable")) {
        rv = cont;
        break;
      } else if (ElementNameIs(cont, "function")) {
        rover = cont;
      } else {
        break;
      }
    }
  } else
    TCI_ASSERT(0);
  return rv;
}
