// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

/* Implementation Notes - JBM  August 1, 2003

  In the final stages of processing engine return strings, we must
  generate presentation MathML ( or LaTeX ) from the "semantic tree"
  created from the engine return string.  That work is done here.

    semantic tree -> MML stream

  In the initial pass, an MML string is generated directly from
  the semantics tree.  Templates with substitution tokens will
  be used.  Example - "<mi>%chdata%</mi>\n". This approach
  is fast, and probably adequate.

  An MML tree is generated from the initial serialized MML.
  Touch-ups can be done in this form before the final MML
  output string is generated.  These could include precedence binds,
  adding a namespace prefix, removing redundant mrows, etc.

  WARNING: This object produces a heap string.  Most member functions
  also produce heap strings.  Callers need to do delete[]s.

  Note that this object was designed and written to produce output
  for a computation testjig.
*/

#include "STree2MML.h"
#include "MML2Tree.h"
#include "MResult.h"
#include "DefStore.h"
#include "PrefStor.h"
#include "SNode.h"
#include "Grammar.h"
#include "fltutils.h"
#include "dumputils.h"
#include "attriblist.h"
#include "strutils.h"
#include <string.h>

// IDs and Templates for MathML generation.
// Make "mml:" into a substitution token - later.
// Add an equivalent set of templates for LaTeX generation.

enum TmplIDs
{                               // WARNING: The array below and this enum must be in sync!!
  TMPL_MATH,
  TMPL_MI,
  TMPL_UPRIGHTMI,
  TMPL_MN,
  TMPL_DECIMAL_NUM,
  TMPL_MO,
  TMPL_MTEXT,
  TMPL_MROW,
  TMPL_MFRAC,
  TMPL_BINOMIAL,
  TMPL_MSUP,
  TMPL_MOVER,
  TMPL_MSQRT,
  TMPL_MROOT,
  TMPL_MTABLE,
  TMPL_MTR,
  TMPL_MTD,
  TMPL_FENCEasOP,
  TMPL_MFENCED,
  TMPL_PARENS,
  TMPL_BRACKETS,
  TMPL_BRACES,
  TMPL_CONJUGATE,
  TMPL_BAR_CONJUGATE,
  TMPL_EIGENVECTORSET,
  TMPL_EIGENVECTOR,
  TMPL_INTEGRAL0,
  TMPL_INTEGRAL1,
  TMPL_INTEGRAL2,
  TMPL_SUM0,
  TMPL_SUM1,
  TMPL_SUM2,
  TMPL_MSUB,
  TMPL_COMMA,
  TMPL_FUNCARGS,
  TMPL_FUNCSUBARGS,
  TMPL_PFUNCARGS,
  TMPL_PFUNCSUBARGS,
  TMPL_DotFUNCARGS,
  TMPL_DotFUNCSUBARGS,
  TMPL_DxFUNCARGS,
  TMPL_DxFUNCSUBARGS,
  TMPL_MIXEDNUMBER,
  TMPL_UNIT,
  TMPL_UNIT_STYLE,
  TMPL_PIECEWISE_FENCE,
  TMPL_HAT_ACCENT,
  TMPL_FACTORIAL,
  TMPL_PRIMES,
  TMPL_INTERVAL,
  TMPL_LIMFUNC,
  TMPL_LOGWITHBASE,
  TMPL_IF,
  TMPL_PARTIALD,
  TMPL_PREFIXMO,
  TMPL_ACCENTMO,
  TMPL_BESSEL,
  TMPL_DD_HEADER,
  TMPL_QUANTILE,
  TMPL_SUBSTITUTION,
  TMPL_TRANSPOSE,
  TMPL_LAST
};

/* Note that the following templates contain numerous %color_attr% 
  substitution tokens.  In many cases, these tokens are replaced
  by an attribute/value string that is used by the renderer to get
  the ambient math color.  This string may be empty, in which case
  the color will come from a style sheet.  In fact, we may always 
  want the ambient math color to come from CSS.  If so, %color_attr%
  can be omitted in cases where only the ambient color is substituted.
  ( Most cases )
*/

char *MML1templates[] = {
  "<math%attrs%>\n%body%</math>\n",
  "<mi%color_attr%>%letters%</mi>\n",
  "<mi%color_attr% fontstyle=\"upright\">%letters%</mi>\n",
  "<mn%color_attr%>%digits%</mn>\n",
  "%num_base%%mult_op%<msup>\n%num_10%%num_exp%</msup>\n",
  "<mo%color_attr%>%chdata%</mo>\n",
  "<mtext%color_attr%>%text%</mtext>\n",
  "<mrow>\n%body%</mrow>\n",
  "<mfrac>\n%num%%den%</mfrac>\n",
  "<mfrac linethickness=\"0\">\n%num%%den%</mfrac>\n",
  "<msup>\n%base%%script%</msup>\n",
  "<mover accent=\"true\">\n%base%%top%</mover>\n",
  "<msqrt>\n%base%</msqrt>\n",
  "<mroot>\n%base%%exp%</mroot>\n",
  "<mtable>\n%rowlist%</mtable>\n",
  "<mtr>\n%celllist%</mtr>\n",
  "<mtd>\n%body%</mtd>\n",
  "<mrow>\n%mo_left%%body%%mo_right%</mrow>\n",
  "<mfenced open=\"%open%\" close=\"%close%\">\n%body%</mfenced>\n",
  "<mrow><mo%color_attr% form=\"prefix\" stretchy=\"true\" fence=\"true\" symmetric=\"true\" lspace=\"true\" rspace=\"rspace\" maxsize=\"maxsize\" minsize=\"minsize\">(</mo>\n%interior%<mo%color_attr2% form=\"postfix\" stretchy=\"true\" fence=\"true\" symmetric=\"true\" lspace=\"true\" rspace=\"rspace\" maxsize=\"maxsize\" minsize=\"minsize\">)</mo>\n</mrow>\n",
  "<mrow><mo%color_attr% form=\"prefix\" stretchy=\"true\" fence=\"true\" symmetric=\"true\" lspace=\"true\" rspace=\"rspace\" maxsize=\"maxsize\" minsize=\"minsize\">[</mo>\n%interior%<mo%color_attr2% form=\"postfix\" stretchy=\"true\" fence=\"true\" symmetric=\"true\" lspace=\"true\" rspace=\"rspace\" maxsize=\"maxsize\" minsize=\"minsize\">]</mo>\n</mrow>\n",
  "<mrow><mo%color_attr% form=\"prefix\" stretchy=\"true\" fence=\"true\" symmetric=\"true\" lspace=\"true\" rspace=\"rspace\" maxsize=\"maxsize\" minsize=\"minsize\">{</mo>\n%interior%<mo%color_attr2% form=\"postfix\" stretchy=\"true\" fence=\"true\" symmetric=\"true\" lspace=\"true\" rspace=\"rspace\" maxsize=\"maxsize\" minsize=\"minsize\">}</mo>\n</mrow>\n",
  "<msup>\n%expr%<mo%color_attr%>&#x2217;</mo>\n</msup>\n",
  "<mover>\n%body%<mo%color_attr% stretchy=\"true\" accent=\"true\">&#xaf;</mo>\n</mover>\n",
  "<mfenced class=\"msiEigenVectorSet\" open=\"{\" close=\"}\">\n%eigen_vector_list%</mfenced>\n",
  "<mfenced class=\"msiEigenTriple\" open=\"[\" close=\"]\">\n<mrow class= \"msiEigenValue\">\n%value%</mrow>\n<mrow class= \"msiMultiplicity\">\n%mult%</mrow>\n<mrow class= \"msiEigenVector\">\n%vector%</mrow>\n</mfenced>\n",
  "<mo form=\"prefix\" largeop=\"true\"%color_attr%>&#x222b;</mo>\n<mrow>\n%integrand%<mo>&#x2062;</mo>\n<mrow>\n%diffop%%intvar%</mrow>\n</mrow>\n",
  "<msub>\n<mo form=\"prefix\" largeop=\"true\"%color_attr%>&#x222b;</mo>\n%lowerlim%</msub>\n<mrow>\n%integrand%<mo>&#x2062;</mo>\n<mrow>\n%diffop%%intvar%</mrow>\n</mrow>\n",
  "<msubsup>\n<mo form=\"prefix\" largeop=\"true\"%color_attr%>&#x222b;</mo>\n%lowerlim%%upperlim%</msubsup>\n<mrow>\n%integrand%<mo>&#x2062;</mo>\n<mrow>\n%diffop%%intvar%</mrow>\n</mrow>\n",
  "<mo form=\"prefix\" largeop=\"true\"%color_attr%>%op%</mo>\n%sumand%",
  "<msub>\n<mo form=\"prefix\" largeop=\"true\"%color_attr%>%op%</mo>\n%lowerlim%</msub>\n%sumand%",
  "<msubsup>\n<mo form=\"prefix\" largeop=\"true\"%color_attr%>%op%</mo>\n%lowerlim%%upperlim%</msubsup>\n%sumand%",
  "<msub>\n%base%%script%</msub>\n",
  "<mo%color_attr%>,</mo>\n",
  "%fheader%<mo>&#x2061;</mo>\n%arglist%",
  "<msub>\n%fheader%%arglist%</msub>\n",
  "<msup>\n%fheader%%nprimes%</msup>\n<mo>&#x2061;</mo>\n%arglist%",
  "<msubsup>\n%fheader%%arglist%%nprimes%</msubsup>\n",
  "<mover>\n%fheader%%dots%</mover>\n<mo>&#x2061;</mo>\n%arglist%",
  "<msub>\n<mover>\n%fheader%%dots%</mover>\n%arglist%</msub>\n",
  "%Dheader%%fheader%<mo>&#x2061;</mo>\n%arglist%",
  "%Dheader%<msub>\n%fheader%%arglist%</msub>\n",
  "<mrow>\n<mn%color_attr%>%whole%</mn>\n<mfrac>\n<mn%color_attr2%>%numerator%</mn>\n<mn%color_attr3%>%denominator%</mn>\n</mfrac>\n</mrow>\n",
  "<mi%unit_attrs%>%nom%</mi>\n",
  "<mstyle%unit_attrs%>\n%body%</mstyle>\n",
//  "<mrow>\n<mo%color_attr%>{</mo>\n%interior%<mo>&#x250a;</mo>\n</mrow>\n",
  "<mfenced open=\"{\" close=\"&#x250a;\">\n%interior%\n</mfenced>\n",
  "<mover>\n%base%<mo form=\"postfix\" accent=\"true\"%color_attr%>&#x302;</mo>\n</mover>\n",
  "<mrow>\n%body%<mo form=\"postfix\"%color_attr%>!</mo>\n</mrow>\n",
  "<mo form=\"postfix\"%color_attr%>%primes%</mo>\n",
  "<mo%color_attr%>%l_delim%</mo>\n<mrow>\n%left%<mo%color_attr2%>,</mo>\n%right%</mrow>\n<mo%color_attr3%>%r_delim%</mo>\n",
  "<munder>\n<mo form=\"prefix\"%color_attr%>%opname%</mo>\n%llim%</munder>\n%arg%",
  "<msub>\n<mi%color_attr%>log</mi>\n%base%</msub>\n<mo>&#x2061;</mo>\n%arg%",
  "<mtext%color_attr%>if</mtext>\n",
  "<mfrac>\n<mrow>\n%d_op%%expr%</mrow>\n%diff_denom%</mfrac>\n",
  "<mo%color_attr% %slant_att% form=\"prefix\">%chdata%</mo>\n",
  "<mo%color_attr% form=\"postfix\" accent=\"true\">%chdata%</mo>\n",
  "<msub>\n<mi%color_attr%>Bessel%letter%</mi>\n%arg1%</msub>\n<mo>&#x2061;</mo>\n%arg2%",
  "<msub>\n%DiffOp%%vars%</msub>\n",
  "<mrow>\n%percent%%comma%%answer%</mrow>\n",
  "<msubsup>\n%FenceBody%\n%lower%\n%upper%\n</msubsup>\n",
  "<msup>\n%MatrixBody%\n%transpose%\n</msup>\n",
  0
};

STree2MML::STree2MML(Grammar* mml_grammar, PrefsStore* up_store) :
  mml_entities(mml_grammar), 
  uprefs_store(up_store)

{
  src_markup = NULL;
  canonicalID_mapper = NULL;
  def_IDs_mapper = NULL;
  input_notation = NULL;

  mml_tree_gen = new MML2Tree();

  // Some data used in selecting output formats
  last_matrix_delim_type = Delims_none;

  // Initialize user prefs
  up_math_attrs = NULL;
  up_clr_math_attr = NULL;
  up_clr_func_attr = NULL;
  up_clr_text_attr = NULL;
  up_unit_attrs = NULL;
}

STree2MML::~STree2MML()
{
  // STree2MML holds copies of pointers to data owned by CompEngine.
  // We don't dispose that data here.
  delete mml_tree_gen;
  delete[] up_math_attrs;
  delete[] up_clr_math_attr;
  delete[] up_clr_func_attr;
  delete[] up_clr_text_attr;
  delete[] up_unit_attrs;
}

// Note that nodes in the "semantic tree" may contain "canonical_IDs"
//  for objects originally encountered in the source MathML.
//  "ID_2_mmlnode" carries a mapping from these "canonical_IDs"
//  to pointers back to objects in the source MML tree.

char *STree2MML::BackTranslate(SEMANTICS_NODE* semantic_tree,
                               const char* src_MML_markup,
                               int output_markup_ID,
                               MIC2MMLNODE_REC* curr_IDs_2mml,
                               MIC2MMLNODE_REC* def_IDs_2mml,
                               INPUT_NOTATION_REC* notation_info,
                               DefStore* ds, 
                               MathResult* mr)
{
  char *zh_rv = NULL;

  output_markup = output_markup_ID;
  src_markup = src_MML_markup;
  canonicalID_mapper = curr_IDs_2mml;
  def_IDs_mapper = def_IDs_2mml;
  input_notation = notation_info;
  p_mr = mr;

  SetUserPrefs(ds);

// #ifdef DEBUG
//   JBM::DumpSList(semantic_tree);
// #endif

  int error_code = 0;
  int nodes_made, terms_made;
  bool is_signed = false;
  char *zh_mml = ProcessSemanticList(semantic_tree, error_code,
                                     nodes_made, terms_made,
                                     is_signed, NULL, NULL);
  if (zh_mml) {
    zh_rv = NestInMath(zh_mml);
  }
  input_notation = NULL;

  // The following code loads our serial mml output into a tree.
  if (zh_rv) {
    MNODE *tree = mml_tree_gen->MMLstr2Tree(zh_rv);
    delete[] zh_rv;
    zh_rv = TNodeToStr(tree, up_mml_prefix, 0);
    DisposeTNode(tree);
  }

  return zh_rv;
}

/* ProcessSemanticList is the basic recursion
  that generates MathML from math buckets.

  It returns data in 2 ways:

   1) Generally, a MathML zstr is returned.
      Here, the caller is told how many terms and factors
	  are in the return. This info is needed in cases
	  where the return may need to be nested in an <mrow>
	  or parened.  We may want to return a list of terms
	  instead of a zstr - this would allow term re-ordering.

   2) If the caller passes in non-NULL num_factors and 
      den_factors, and the input s_list contains only a single
	  fraction, ProcessSemanticList appends factors to
	  num_factors and den_factors.  The caller must compose
	  these factor lists to produce final MathML.
*/

char *STree2MML::ProcessSemanticList(SEMANTICS_NODE * s_list,
                                     int& error_code,
                                     int & nodes_last_term, int & terms_made,
                                     bool & is_signed,
                                     FACTOR_REC ** num_factors,
                                     FACTOR_REC ** den_factors)
{
  char *zh_rv = NULL;
  U32 buffer_ln = 0;

  nodes_last_term = 0;
  terms_made = 0;
  is_signed = false;
  bool signed_was_set = false;
  int sign_nodes = 0;

  // Basic algorithm - each s_node in the list we're traversing is
  //  translated producing a fragment of MathML (in the case of a fraction
  //  2 fragments are generated).  These fragments are stored in 2 lists
  //  of "factors", one for the numerator and one for the denominator.
  // When a term-ending s_node is encountered, final MathML is
  //  composed for the term using the factor lists.

  int nodes_in_last_term = 0;
  FACTOR_REC *loc_num_factors = NULL;
  FACTOR_REC *loc_den_factors = NULL;

  SEMANTICS_NODE *rover = s_list;
  while (rover) {
    bool is_term_separator = false;
    bool is_plusORminus = false;
    int curr_prec = 0;
    if (SNodeEndsTerm(rover, curr_prec, is_plusORminus)) {
      is_term_separator = true;
      if (loc_num_factors || loc_den_factors) {
        // In certain cases, it may be necessary to nest the term in parens
        bool add_parens = false;
        /*
        Currently, parens in multi-operator expressions are added at a lower level
        in SemanticPGROUP2MML.  It is logically better to handle parens here,
        but it's simpler to add them at the lower level.
        */

        // Factors may be moved - units shift right
        // Factors may be added or deleted - 1
        // Multiplicative operators may be inserted

        bool loc_is_signed = false;
        int loc_term;
        char *term_str = ComposeTerm(loc_num_factors, loc_den_factors,
                                     loc_is_signed, nodes_in_last_term, loc_term);

        loc_num_factors = NULL; // These lists are INVALID at this point
        loc_den_factors = NULL;

        if (term_str && add_parens) {
          if (nodes_in_last_term > 1)
            term_str = NestzMMLInMROW(term_str);
          int nodes_made;
          term_str = NestzMMLInPARENS(term_str, nodes_made);
        }

        if (term_str) {
          if (!signed_was_set) {
            signed_was_set = true;
            is_signed = loc_is_signed;
          }
          terms_made++;

          zh_rv = AppendStr2HeapStr(zh_rv, buffer_ln, term_str);
          delete[] term_str;
          nodes_last_term = nodes_in_last_term;
        }
      } else if (is_plusORminus) {
        if (!signed_was_set) {
          is_signed = true;
          signed_was_set = true;
        }
      }
    }

    ProcessSemanticNode(rover, error_code, &loc_num_factors, &loc_den_factors);
    if (error_code == 1) {
      delete[] zh_rv;
      DisposeFactorList(loc_num_factors);
      DisposeFactorList(loc_den_factors);
      return NULL;
    }

    if (is_term_separator) {
      // There should only be a single factor in the numerator here!
      if (loc_num_factors) {
        char *op_str = loc_num_factors->zh_fstr;
        if (op_str) {
          zh_rv = AppendStr2HeapStr(zh_rv, buffer_ln, op_str);
          delete[] op_str;
          loc_num_factors->zh_fstr = NULL;
          if (is_signed)
            sign_nodes = 1;
        }
        DisposeFactorList(loc_num_factors);
        loc_num_factors = NULL;
      } else {
        TCI_ASSERT(0);
      }
      if (loc_den_factors) {
        TCI_ASSERT(0);
        DisposeFactorList(loc_den_factors);
        loc_den_factors = NULL;
      }
    }
    rover = rover->next;
  }

  if (loc_num_factors || loc_den_factors) {
    bool extract_num_sign;
    bool extract_den_sign;
    loc_num_factors = AbsorbUnaryOps(loc_num_factors, extract_num_sign);
    loc_den_factors = AbsorbUnaryOps(loc_den_factors, extract_den_sign);

    if (!zh_rv && num_factors && den_factors) {
      // Here, the list generated is only 1 term and it's a fraction.
      // We pass the numerator and denominator out as "factors".
      // The caller must "ComposeTerm".

      *num_factors = JoinFactors(*num_factors, loc_num_factors);
      *den_factors = JoinFactors(*den_factors, loc_den_factors);
      nodes_last_term = 0;
    } else {
      if (extract_num_sign) {
        // If the term we're going to make is a fraction, we might want
        //  to put the sign that starts the numerator in front of the fraction.
      }

      bool loc_is_signed = false;
      int loc_terms;
      char *term_str = ComposeTerm(loc_num_factors, loc_den_factors,
                                   loc_is_signed, nodes_in_last_term, loc_terms);
      if (term_str) {
        if (!signed_was_set) {
          signed_was_set = true;
          is_signed = loc_is_signed;
        }
        terms_made += loc_terms;
        zh_rv = AppendStr2HeapStr(zh_rv, buffer_ln, term_str);
        delete[] term_str;
        nodes_last_term = nodes_in_last_term + sign_nodes;
      }
    }
  }

  return zh_rv;
}

// When MathML is generated from a single semantic node,
//  the MathML zstr and some auxilary info is stored in a
//  "FACTOR_REC".  Callers compose higher level MathML
//  from this data.  num_factors and den_factors are used
//  only when a fraction is processed.

void STree2MML::ProcessSemanticNode(SEMANTICS_NODE * snode,
                                    int& error_code,
                                    FACTOR_REC ** num_factors,
                                    FACTOR_REC ** den_factors)
{
  if (snode) {
    error_code = snode->error_flag;
    if (error_code == 1)
      return;

    FACTOR_REC *new_fr = CreateFactor();

    bool append_to_num = true;
    switch (snode->semantic_type) {
    case SEM_TYP_VARIABLE:
      SemanticVARIABLE2MML(snode, new_fr, error_code);
      break;
    case SEM_TYP_NUMBER:
      SemanticNUMBER2MML(snode, new_fr, error_code);
      break;
    case SEM_TYP_UCONSTANT:
      SemanticUCONSTANT2MML(snode, new_fr, error_code);
      break;
    case SEM_TYP_BOOLEAN:
      SemanticBOOLEAN2MML(snode, new_fr, error_code);
      break;
    case SEM_TYP_FUNCTION:
      SemanticFUNCTION2MML(snode, new_fr, error_code);
      break;
    case SEM_TYP_PIECEWISE_LIST:
      SemanticPIECEWISE2MML(snode, new_fr, error_code);
      break;
    case SEM_TYP_LIMFUNC:
      SemanticLIMFUNC2MML(snode, new_fr, error_code);
      break;
    case SEM_TYP_PREFIX_OP:
    case SEM_TYP_INFIX_OP:
    case SEM_TYP_POSTFIX_OP:
      SemanticOPERATOR2MML(snode, new_fr, error_code);
      break;
    case SEM_TYP_SPACE:
        // do nothing ??
      break;
    case SEM_TYP_TEXT:
      SemanticTEXT2MML(snode, new_fr, error_code);
      break;

    case SEM_TYP_INVFUNCTION:
      SemanticFUNCTION2MML(snode, new_fr, error_code);
      break;

    case SEM_TYP_EQCHECK_RESULT:
      SemanticEQRESULT2MML(snode, new_fr, error_code);
      break;

    case SEM_TYP_SIUNIT:
    case SEM_TYP_USUNIT:
      SemanticUNIT2MML(snode, new_fr, error_code);
      break;
    case SEM_TYP_MATH_CONTAINER:
      // SEM_TYP_MATH_CONTAINER can occur here - not from
      //      the engine, but from "computations" like Matrix::Reshape
      //      that don't go down to the engine.
      SemanticMATH2MML(snode, new_fr, error_code);
      break;
    case SEM_TYP_GENERIC_FENCE:
    case SEM_TYP_PRECEDENCE_GROUP:
      SemanticPGROUP2MML(snode, new_fr, error_code);
      break;

    case SEM_TYP_FRACTION:{
        bool done = false;
        if (up_output_mixed_nums
            && !snode->next && !snode->prev && !snode->parent) {
          BUCKET_REC *b_list = snode->bucket_list;
          BUCKET_REC *num_bucket = FindBucketRec(b_list, MB_NUMERATOR);
          BUCKET_REC *den_bucket = FindBucketRec(b_list, MB_DENOMINATOR);
          // This isn't a general algorithm.  If mixed numbers are seen in input
          //  and fractions like 5/2 occur in the output,
          //  then mixed number notation is produced.  Mixed numbers aren't
          //  generated for negatives or very large positives.
          U32 num, denom;
          if (IsSmallNaturalNumber(num_bucket->first_child, num)
              && IsSmallNaturalNumber(den_bucket->first_child, denom)) {
            if (num > denom) {
              SmallMixedNumber2MML(num, denom, new_fr);
              done = true;
            }
          }
        }
        if (!done) {
          SemanticFRACTION2MML(snode, error_code, num_factors, den_factors);
          append_to_num = false;
        }
      }
      break;

    case SEM_TYP_BINOMIAL:
      SemanticBINOMIAL2MML(snode, new_fr, error_code);
      break;
    case SEM_TYP_POWERFORM:
      SemanticPOWERFORM2MML(snode, new_fr, error_code);
      break;
    case SEM_TYP_SQRT:
      SemanticSQROOT2MML(snode, new_fr, error_code);
      break;
    case SEM_TYP_ROOT:
      SemanticROOT2MML(snode, new_fr, error_code);
      break;
    case SEM_TYP_BIGOP_SUM:
      SemanticBIGOPSUM2MML(snode, new_fr, error_code);
      break;
    case SEM_TYP_BIGOP_INTEGRAL:
      SemanticINTEGRAL2MML(snode, new_fr, error_code);
      break;
    case SEM_TYP_TABULATION:
      SemanticTABULATION2MML(snode, new_fr, error_code);
      break;
    case SEM_TYP_MTRANSPOSE:
    case SEM_TYP_HTRANSPOSE:
      SemanticTRANSPOSE2MML(snode, new_fr, error_code);
      break;
    case SEM_TYP_SIMPLEX:
      SemanticSIMPLEX2MML(snode, new_fr, error_code);
      break;

    case SEM_TYP_PARENED_LIST:{
        bool add_parens = true;
        SemanticFENCE2MML(snode, new_fr, add_parens, error_code);
      }
      break;

    case SEM_TYP_SET:
    case SEM_TYP_BRACKETED_LIST:
      SemanticFENCE2MML(snode, new_fr, true, error_code);
      break;

    case SEM_TYP_SUBSTITUTION:
      SemanticSUBS2MML(snode, new_fr, error_code);
      break;      

    case SEM_TYP_QSUB_LIST:
      SemanticFENCE2MML(snode, new_fr, false, error_code);
      break;

    case SEM_TYP_ABS:
    case SEM_TYP_FLOOR:
    case SEM_TYP_CEILING:
      SemanticFENCEOP2MML(snode, new_fr, error_code);
      break;
    case SEM_TYP_NORM:
      SemanticNORM2MML(snode, new_fr, error_code);
      break;
    case SEM_TYP_CONJUGATE:
      SemanticCONJUGATE2MML(snode, new_fr, error_code);
      break;
    case SEM_TYP_EIGENVECTORSET:
      SemanticEIGENVECTOR2MML(snode, new_fr, error_code);
      break;

    case SEM_TYP_QUANTILE_RESULT:
      SemanticQUANTILE2MML(snode, new_fr, error_code);
      break;

    case SEM_TYP_LISTOPERATOR:
      SemanticFUNCTION2MML(snode, new_fr, error_code);
      break;

    case SEM_TYP_INTERVAL:
      SemanticINTERVAL2MML(snode, new_fr, error_code);
      break;

    case SEM_TYP_MIXEDNUMBER:
      SemanticMIXEDNUM2MML(snode, new_fr, error_code);
      break;

    case SEM_TYP_DERIVATIVE:
    case SEM_TYP_PARTIALD:{    // d/d OR D_x
        bool do_d_over_dx = false;
        if (up_derivative_format) {
          if (up_derivative_format == 1)
            do_d_over_dx = (snode->semantic_type == SEM_TYP_PARTIALD);
        } else {
          if (!input_notation->n_Dxs)
            do_d_over_dx = (snode->semantic_type == SEM_TYP_PARTIALD);
        }
        if (do_d_over_dx)
          SemanticFracDerivative2MML(snode, new_fr, false, error_code);
        else
          SemanticDERIVATIVE2MML(snode, new_fr, error_code);
      }
      break;

    case SEM_TYP_TRANSFORM:
    case SEM_TYP_INVTRANSFORM:
      SemanticFUNCTION2MML(snode, new_fr, error_code);  // behave just like functions except for possible -1
      break;
    case SEM_TYP_QUALIFIED_VAR:  //SLS just guessing
      SemanticVARIABLE2MML(snode, new_fr, error_code);
      break;
    default:
      TCI_ASSERT(!"Semantic Type not handled.");
      break;
    }

    if (!error_code && append_to_num)
      *num_factors = JoinFactors(*num_factors, new_fr);
    else {
      DisposeFactorList(new_fr);
      if (error_code == 2)
        error_code = 0;
    }
  }
}

void STree2MML::SemanticVARIABLE2MML(SEMANTICS_NODE * s_variable,
                                     FACTOR_REC * factor, int& error_code)
{
  char *zh_rv = NULL;

  TCI_ASSERT(s_variable->contents || s_variable->canonical_ID);
  if (s_variable->canonical_ID) {
    //  for some mml input object
    if (canonicalID_mapper) {
      const char *mml_markup = GetMarkupFromID(canonicalID_mapper,
                                               s_variable->canonical_ID);
      if (!mml_markup && def_IDs_mapper)
        mml_markup = GetMarkupFromID(def_IDs_mapper,s_variable->canonical_ID);
      if (mml_markup) {
        // Construct a tree from the markup
        MNODE *var = mml_tree_gen->MMLstr2Tree(mml_markup);
        var = CleanupMMLsource(var);
        // Create markup from the modified tree
        zh_rv = TNodeToStr(var, NULL, 0);
        DisposeTNode(var);

        // User defined SEMANTIC variables can contain
        //  additional info, as in the maple variable "a_{ij}"
        if (s_variable->bucket_list && s_variable->bucket_list->first_child) {
          SEMANTICS_NODE *s_sub = s_variable->bucket_list->first_child;
          zh_rv = AddSubToVariable(s_sub, zh_rv);
        }
      } else {
        TCI_ASSERT(0);
      }
    }
  }
  if (s_variable->contents && !zh_rv) {
    // likely a generated variable?
    if (!strcmp(s_variable->contents, "x") ) {
        char *tmpl = GetTmplPtr(TMPL_HAT_ACCENT);
        char *mi_Z = MIfromCHDATA("Z");
        size_t zln = strlen(tmpl) + strlen(mi_Z);
        zh_rv = new char[zln + 1];
        strcpy(zh_rv, tmpl);
        StrReplace(zh_rv, zln, "%base%", mi_Z);
        zh_rv =
          ColorizeMMLElement(zh_rv, up_clr_math_attr, NULL, NULL);
        delete[] mi_Z;
    } else {
    

       char *tmpl = GetTmplPtr(TMPL_MI);
       char *z_variable = s_variable->contents;

       if (!strcmp(z_variable, "infinity"))
         z_variable = "&#x221e;";

       size_t ln = strlen(tmpl) + strlen(z_variable);
       zh_rv = new char[ln + 1];
       strcpy(zh_rv, tmpl);
       StrReplace(zh_rv, ln, "%letters%", z_variable);
       zh_rv = ColorizeMMLElement(zh_rv, up_clr_math_attr, NULL, NULL);
       // SEMANTIC variables that were generated by an engine can contain
       //  additional info, as in the maple variable "s21"
       if (s_variable->bucket_list && s_variable->bucket_list->first_child) {
         SEMANTICS_NODE *s_sub = s_variable->bucket_list->first_child;
         zh_rv = AddSubToVariable(s_sub, zh_rv);
       }
     }
  }

  if (zh_rv) {
    factor->zh_fstr = zh_rv;
    factor->mml_nodes_last_term = 1;
  } else {
    error_code = 1;
  }
}

// Most of the code in the following function cleans up
//  some sloppy formatting of decimal numbers by engines.
void STree2MML::SemanticNUMBER2MML(SEMANTICS_NODE * s_number,
                                   FACTOR_REC * factor, int& error_code)
{
  char *zh_rv = NULL;

  int nodes_last_term = 1;
  if (s_number->contents) {
    char *fixed_number = NumberToUserFormat(s_number);

    if (fixed_number && *fixed_number) {
      bool is_decimal_number = false;
      char *p_decimal = strchr(fixed_number, '.');
      char *p_expon = NULL;
      if (p_decimal) {
        p_expon = strchr(p_decimal + 1, 'e');
        if (p_expon)
          is_decimal_number = true;
      }

      if (is_decimal_number) {
        char *tmpl = GetTmplPtr(TMPL_DECIMAL_NUM);
        //  "%num_base%%mult_op%<msup>\n%num_10%%num_exp%</msup>\n",
        *p_expon = 0;
        p_expon++;

        char *znum_base = MNfromNUMBER(fixed_number);
        char *zmult_op = MOfromSTRING("&#xd7;", up_clr_math_attr);
        char *znum_10 = MNfromNUMBER("10");
        char *znum_exp = MNfromNUMBER(p_expon);

        size_t zln = strlen(tmpl) + strlen(znum_base) + strlen(zmult_op)
          + strlen(znum_10) + strlen(znum_exp);

        zh_rv = new char[zln + 1];
        strcpy(zh_rv, tmpl);
        StrReplace(zh_rv, zln, "%num_base%", znum_base);
        StrReplace(zh_rv, zln, "%mult_op%", zmult_op);
        StrReplace(zh_rv, zln, "%num_10%", znum_10);
        StrReplace(zh_rv, zln, "%num_exp%", znum_exp);

        delete[] znum_base;
        delete[] zmult_op;
        delete[] znum_10;
        delete[] znum_exp;
        nodes_last_term = 3;
      } else {
        if (p_decimal && *(p_decimal + 1) == 0) { // "1."
          size_t zln = strlen(fixed_number);
          char *tmp = new char[zln + 2];
          strcpy(tmp, fixed_number);
          strcat(tmp, "0");
          delete[] fixed_number;
          fixed_number = tmp;
        }
        zh_rv = MNfromNUMBER(fixed_number);
      }
    } else {
      TCI_ASSERT(0);
    }
    delete[] fixed_number;
    factor->is_number = IsExposedNumber(s_number);
  }

  if (zh_rv) {
    factor->zh_fstr = zh_rv;
    factor->mml_nodes_last_term = nodes_last_term;
  } else {
    error_code = 1;
  }
}

void STree2MML::SemanticUCONSTANT2MML(SEMANTICS_NODE * s_constant,
                                      FACTOR_REC * factor, int& error_code)
{
  char *zh_rv = NULL;

  if (s_constant->contents) {
    char *const_tok = s_constant->contents;

    bool done = false;
    size_t tln = strlen(const_tok);

    // Engine generated constants - following digits are subscripted here
    if (tln > 1 && (const_tok[0] == 'C' || const_tok[0] == 'X')
        && const_tok[1] >= '0' && const_tok[1] <= '9') {
      char letter[2];
      letter[0] = const_tok[0];
      letter[1] = 0;
      char *tmpl = GetTmplPtr(TMPL_MI);
      size_t zln = strlen(tmpl);
      char *zh_base = new char[zln + 1];
      strcpy(zh_base, tmpl);
      StrReplace(zh_base, zln, "%letters%", letter);
      zh_base = ColorizeMMLElement(zh_base, up_clr_math_attr, NULL, NULL);

      char *zh_sub = MNfromNUMBER(const_tok + 1);
      tmpl = GetTmplPtr(TMPL_MSUB);
      zln = strlen(tmpl) + strlen(zh_base) + strlen(zh_sub);
      zh_rv = new char[zln + 1];
      strcpy(zh_rv, tmpl);
      StrReplace(zh_rv, zln, "%base%", zh_base);
      StrReplace(zh_rv, zln, "%script%", zh_sub);

      delete[] zh_base;
      delete[] zh_sub;
      done = true;
    } else if (tln == 1) {
      char ch = const_tok[0];
      char *mml_symbol = NULL;

      if (ch == 'Z') {
        char *tmpl = GetTmplPtr(TMPL_HAT_ACCENT);
        char *mi_Z = MIfromCHDATA("Z");
        size_t zln = strlen(tmpl) + strlen(mi_Z);
        mml_symbol = new char[zln + 1];
        strcpy(mml_symbol, tmpl);
        StrReplace(mml_symbol, zln, "%base%", mi_Z);
        mml_symbol =
          ColorizeMMLElement(mml_symbol, up_clr_math_attr, NULL, NULL);
        delete[] mi_Z;
      } else if (ch == 'Q') {
        mml_symbol = MIfromCHDATA("Q");
      } else if (ch == 'R') {
        mml_symbol = MIfromCHDATA("R");
      } else if (ch == 'C') {
        mml_symbol = MIfromCHDATA("C");
      }
      if (mml_symbol) {
        zh_rv = mml_symbol;
        done = true;
      }
    }

    if (!done) {
      char symbol[80];

      // We allow output mappings for some constants thru userprefs
      if (!strcmp(const_tok, "&#x2145;")) {
        strcpy(symbol, up_differentialD);
      } else if (!strcmp(const_tok, "&#x2146;")) {
        strcpy(symbol, up_differentiald);
      } else if (!strcmp(const_tok, "&#x2147;")) {
        strcpy(symbol, up_nlogbase);
      } else if (!strcmp(const_tok, "&#x2148;")) {
        strcpy(symbol, up_imaginaryi);
      } else {
        strcpy(symbol, const_tok);
      }
      int tmpl_ID = TMPL_MI;
      char *repl = "%letters%";
      char *color_attr = up_clr_math_attr;
      if (!strcmp(symbol, "gamma")) {
        color_attr = up_clr_func_attr;
      } else if (!strcmp(symbol, "FAIL")) {
        tmpl_ID = TMPL_MTEXT;
        repl = "%text%";
        color_attr = up_clr_text_attr;
      } else if (!strcmp(symbol, "UNKNOWN")) {
        tmpl_ID = TMPL_MTEXT;
        repl = "%text%";
        color_attr = up_clr_text_attr;
      } else if (!strcmp(symbol, "undefined")) {
        tmpl_ID = TMPL_MTEXT;
        repl = "%text%";
        color_attr = up_clr_text_attr;
      } else if (!strcmp(symbol, "NoSol")) {
        tmpl_ID = TMPL_MTEXT;
        repl = "%text%";
        color_attr = up_clr_text_attr;
        p_mr->PutResultCode(CR_nosol);
      }

      char *tmpl = GetTmplPtr(tmpl_ID);
      size_t ln = strlen(tmpl) + strlen(symbol);
      zh_rv = new char[ln + 1];
      strcpy(zh_rv, tmpl);
      StrReplace(zh_rv, ln, repl, symbol);
      zh_rv = ColorizeMMLElement(zh_rv, color_attr, NULL, NULL);
    }
  } else {
    TCI_ASSERT(0);
  }
  if (zh_rv) {
    factor->zh_fstr = zh_rv;
    factor->is_constant = true;
    factor->mml_nodes_last_term = 1;
  } else {
    error_code = 1;
  }
}

void STree2MML::SemanticBOOLEAN2MML(SEMANTICS_NODE * s_boolean,
                                    FACTOR_REC * factor, int& error_code)
{
  char *zh_rv = NULL;

  if (s_boolean->contents) {  // "true" or "false"
    char *tmpl = GetTmplPtr(TMPL_MTEXT);
    char *unicode_entity = s_boolean->contents;
    size_t ln = strlen(tmpl) + strlen(unicode_entity);
    zh_rv = new char[ln + 1];
    strcpy(zh_rv, tmpl);
    StrReplace(zh_rv, ln, "%text%", unicode_entity);
    zh_rv = ColorizeMMLElement(zh_rv, up_clr_func_attr, NULL, NULL);

    if (s_boolean->variant == SNV_True)
      p_mr->PutResultCode(CR_bool_true);
    else if (s_boolean->variant == SNV_False)
      p_mr->PutResultCode(CR_bool_false);
    else
      TCI_ASSERT(0);
  }

  if (zh_rv) {
    factor->zh_fstr = zh_rv;
    factor->mml_nodes_last_term = 1;
  } else {
    error_code = 1;
  }
}

void STree2MML::SemanticCONJUGATE2MML(SEMANTICS_NODE * s_conjugate,
                                      FACTOR_REC * factor, int& error_code)
{
  char *zh_rv = NULL;

  if (s_conjugate && s_conjugate->bucket_list) {
    BUCKET_REC *arg_b = s_conjugate->bucket_list;
    if (arg_b->first_child) {
      int nodes_made, terms_made;
      bool is_signed = false;
      char *zh_arg = ProcessSemanticList(arg_b->first_child,
                                         error_code, nodes_made, terms_made,
                                         is_signed, NULL, NULL);

      if (up_overbar_is_conjugate && input_notation->n_overbars) {
        if (nodes_made > 1 || is_signed)
          zh_arg = NestzMMLInMROW(zh_arg);

        char *tmpl = GetTmplPtr(TMPL_BAR_CONJUGATE);
        //  "<mover>\n%body%<mo%color_attr% stretchy=\"true\" accent=\"true\">&#xaf;</mo>\n</mover>\n",
        size_t ln = strlen(tmpl);
        if (zh_arg)
          ln += strlen(zh_arg);
        zh_rv = new char[ln + 1];
        strcpy(zh_rv, tmpl);
        StrReplace(zh_rv, ln, "%body%", zh_arg);
        zh_rv = ColorizeMMLElement(zh_rv, up_clr_math_attr, NULL, NULL);
      } else {
        bool add_parens = false;
        if (zh_arg) {
          if (nodes_made > 1 || terms_made > 1 || is_signed)
            add_parens = true;
          else {
            SEMANTICS_NODE *s_base = arg_b->first_child;
            if (s_base->semantic_type == SEM_TYP_TABULATION) {
              add_parens = true;
              // if arg is single matrix and that matrix already added parens...
              if (s_base->prev == NULL && s_base->next == NULL && last_matrix_delim_type == Delims_parens)
                add_parens = false;
            } else if (s_base->semantic_type != SEM_TYP_VARIABLE) {
              add_parens = true;
            }
          }
        }
        if (add_parens) {
          if (nodes_made > 1 || is_signed)
            zh_arg = NestzMMLInMROW(zh_arg);
          zh_arg = NestzMMLInPARENS(zh_arg, nodes_made);
        }

        char *tmpl = GetTmplPtr(TMPL_CONJUGATE);
        //  "<msup>\n%expr%<mo%color_attr%>&#x2217;</mo>\n</msup>\n",
        size_t ln = strlen(tmpl);
        if (zh_arg)
          ln += strlen(zh_arg);
        zh_rv = new char[ln + 1];
        strcpy(zh_rv, tmpl);
        StrReplace(zh_rv, ln, "%expr%", zh_arg);
        zh_rv = ColorizeMMLElement(zh_rv, up_clr_math_attr, NULL, NULL);
      }
      delete[] zh_arg;
    }
  }

  if (zh_rv) {
    factor->zh_fstr = zh_rv;
    factor->mml_nodes_last_term = 1;
  } else {
    error_code = 1;
  }
}

void STree2MML::SemanticTEXT2MML(SEMANTICS_NODE * s_text,
                                 FACTOR_REC * factor, int& error_code)
{
  char *zh_rv = NULL;

  if (s_text->contents) {
    char *tmpl = GetTmplPtr(TMPL_MTEXT);
    char *text = s_text->contents;
    size_t ln = strlen(tmpl) + strlen(text);
    zh_rv = new char[ln + 1];
    strcpy(zh_rv, tmpl);
    StrReplace(zh_rv, ln, "%text%", text);
    zh_rv = ColorizeMMLElement(zh_rv, up_clr_text_attr, NULL, NULL);
  }
  if (zh_rv) {
    factor->zh_fstr = zh_rv;
    factor->is_text = true;
    factor->mml_nodes_last_term = 1;
  } else {
    error_code = 1;
  }
}

void STree2MML::SemanticEQRESULT2MML(SEMANTICS_NODE * s_eq_result,
                                     FACTOR_REC * factor, int& error_code)
{
  char *zh_rv = NULL;

  if (s_eq_result->contents) {
    char *tmpl = GetTmplPtr(TMPL_MTEXT);
    char *text = s_eq_result->contents;
    size_t ln = strlen(tmpl) + strlen(text);
    zh_rv = new char[ln + 1];
    strcpy(zh_rv, tmpl);
    StrReplace(zh_rv, ln, "%text%", text);
    zh_rv = ColorizeMMLElement(zh_rv, up_clr_text_attr, NULL, NULL);

    int result_code = CR_EqCheck_undecidable;
    if (s_eq_result->variant == SNV_True)
      result_code = CR_EqCheck_true;
    else if (s_eq_result->variant == SNV_False)
      result_code = CR_EqCheck_false;
    else if (s_eq_result->variant == SNV_Undecidable)
      result_code = CR_EqCheck_undecidable;
    else
      TCI_ASSERT(0);

    p_mr->PutResultCode(result_code);
  }

  if (zh_rv) {
    factor->zh_fstr = zh_rv;
    factor->is_text = true;
    factor->mml_nodes_last_term = 1;
  } else {
    error_code = 1;
  }
}

// Note that we handle base units here - not compound units.

void STree2MML::SemanticUNIT2MML(SEMANTICS_NODE * s_unit,
                                 FACTOR_REC * factor, int& error_code)
{
  char *zh_rv = NULL;

  if (s_unit->contents) {
    char *tmpl = GetTmplPtr(TMPL_UNIT);
    // "<mi %unit_attrs%>%nom%</mi>\n",
    char *text = s_unit->contents;
    size_t ln = strlen(tmpl) + strlen(text);
    if (up_unit_attrs)
      ln += strlen(up_unit_attrs);
    zh_rv = new char[ln + 1];
    strcpy(zh_rv, tmpl);
    StrReplace(zh_rv, ln, "%unit_attrs%", up_unit_attrs);
    StrReplace(zh_rv, ln, "%nom%", text);
  }
  if (zh_rv) {
    factor->zh_fstr = zh_rv;
    factor->mml_nodes_last_term = 1;
    SetUnitInfo(s_unit, factor);
  } else {
    error_code = 1;
  }
}

/*
FUNCTION contents = "sin"
  UNNAMED
    VARIABLE canonical_ID = "mix"
    INFIX OPERATOR contents = "&#xd7;"
    VARIABLE canonical_ID = "miy"
*/

void STree2MML::SemanticFUNCTION2MML(SEMANTICS_NODE * s_function,
                                     FACTOR_REC * factor, int& error_code)
{
  char *f_nom = s_function->contents;
  bool done = false;
  if (f_nom && !strcmp(f_nom, "factorial")) {
    done = true;
    BUCKET_REC *arg_bucket = s_function->bucket_list;
    if (arg_bucket->first_child) {
      SEMANTICS_NODE *operand = arg_bucket->first_child;

      int error_code = 0;
      int nodes_made, terms_made;
      bool is_signed = false;
      char *z_arg = ProcessSemanticList(operand,
                                        error_code, nodes_made,
                                        terms_made, is_signed, NULL, NULL);
      if (z_arg) {
        if (terms_made > 1 || nodes_made > 1) {
          if (is_signed || nodes_made > 1)
            z_arg = NestzMMLInMROW(z_arg);
          int new_nodes_made;
          z_arg = NestzMMLInPARENS(z_arg, new_nodes_made);
        }
        char *tmpl = GetTmplPtr(TMPL_FACTORIAL);
        // "<mml:mrow>\n%body%<mml:mo form=\"postfix\">!</mml:mo>\n</mml:mrow>\n",
        size_t ln = strlen(tmpl) + strlen(z_arg);
        char *zh_rv = new char[ln + 1];
        strcpy(zh_rv, tmpl);
        StrReplace(zh_rv, ln, "%body%", z_arg);
        zh_rv = ColorizeMMLElement(zh_rv, up_clr_math_attr, NULL, NULL);
        delete[] z_arg;
        factor->mml_nodes_last_term = nodes_made;
        factor->zh_fstr = zh_rv;
      }
    } else {
      TCI_ASSERT(0);
    }
  }

  if (f_nom && !strcmp(f_nom, "log")) {
    // Note that we may not always want to script the log's base here.
    // Users specify a default base ( e OR 10 ), and may want that
    // particular base to be implicit - ie. not scripted.

    bool script_base = true;
    BUCKET_REC *lb_bucket =
      FindBucketRec(s_function->bucket_list, MB_LOG_BASE);
    if (lb_bucket && lb_bucket->first_child) {
      SEMANTICS_NODE *log_base = lb_bucket->first_child;
      if (log_base->semantic_type == SEM_TYP_NUMBER) {
        if (!strcmp(log_base->contents, "10"))
          if (!up_log_is_base_e)
            script_base = false;
      } else if (log_base->semantic_type == SEM_TYP_UCONSTANT) {
        if (!strcmp(log_base->contents, "&#x2147;"))
          if (up_log_is_base_e)
            script_base = false;
      }
    }
    if (script_base) {
      LogWithBase2MML(s_function, factor, error_code);
      done = true;
    }
  }
  if (IsBesselFunc(f_nom)) {
    BesselFunc2MML(s_function, factor, error_code);
    done = true;
  }
  if (!done) {
    char *zh_header = FuncHeader2MML(s_function);
    if (zh_header) {
      char *zh_fcall = NULL;

      bool do_subscript_args = false;
      if (input_notation && input_notation->funcarg_is_subscript)
        do_subscript_args = true;

      int nodes_made = 1;
      if (s_function->bucket_list) {
        bool trigarg_enabled = IsTrigArgFunc(s_function);
        bool trigarg_scripted;
        int deriv_style = 0;
        zh_fcall = ComposeFuncCall(s_function, zh_header, trigarg_enabled,
                                   trigarg_scripted, do_subscript_args,
                                   0, deriv_style, NULL);
        if (!do_subscript_args)
          nodes_made += 2;      // &ApplyFunc; + the arg

        if (trigarg_scripted) {
          if (ParensRequired(s_function)) {
            zh_fcall = NestzMMLInMROW(zh_fcall);
            zh_fcall = NestzMMLInPARENS(zh_fcall, nodes_made);
          }
        }
      }
      delete[] zh_header;
      factor->mml_nodes_last_term = nodes_made;
      factor->zh_fstr = zh_fcall;
    }
  }
}

void STree2MML::LogWithBase2MML(SEMANTICS_NODE * s_function,
                                FACTOR_REC * factor, int& error_code)
{
  BUCKET_REC *arg_list = s_function->bucket_list;
  BUCKET_REC *lb_bucket = FindBucketRec(arg_list, MB_LOG_BASE);
  if (lb_bucket) {
    char *z_base = NULL;
    char *z_arg = NULL;

    BUCKET_REC *arg_bucket = FindBucketRec(arg_list, MB_UNNAMED);
    if (arg_bucket && arg_bucket->first_child) {
      SEMANTICS_NODE *operand = arg_bucket->first_child;
      int error_code = 0;
      int nodes_made, terms_made;
      bool is_signed = false;
      z_arg = ProcessSemanticList(operand, error_code, nodes_made,
                                  terms_made, is_signed, NULL, NULL);
      if (z_arg) {
        if (terms_made > 1 || nodes_made > 1 || is_signed)
          z_arg = NestzMMLInMROW(z_arg);
        int nodes_made;
        if (up_parens_on_trigargs) {
          z_arg = NestzMMLInPARENS(z_arg, nodes_made);
        } else {
          if (terms_made > 1)
            z_arg = NestzMMLInPARENS(z_arg, nodes_made);
        }
      }
    } else {
      TCI_ASSERT(0);
    }
    if (lb_bucket->first_child) {
      SEMANTICS_NODE *base = lb_bucket->first_child;
      int error_code = 0;
      int nodes_made, terms_made;
      bool is_signed = false;
      z_base = ProcessSemanticList(base, error_code, nodes_made,
                                   terms_made, is_signed, NULL, NULL);
      if (z_base) {
        if (terms_made > 1 || nodes_made > 1)
          z_base = NestzMMLInMROW(z_base);
      }
    } else {
      TCI_ASSERT(0);
    }

    char *tmpl = GetTmplPtr(TMPL_LOGWITHBASE);
    //  "<msub>\n<mi%color_attr%>log</mi>\n%base%</msub>\n<mo>&#x2061;</mo>\n%arg%",
    size_t ln = strlen(tmpl);
    if (z_base)
      ln += strlen(z_base);
    if (z_arg)
      ln += strlen(z_arg);
    char *zh_rv = new char[ln + 1];
    strcpy(zh_rv, tmpl);
    StrReplace(zh_rv, ln, "%base%", z_base);
    StrReplace(zh_rv, ln, "%arg%", z_arg);
    zh_rv = ColorizeMMLElement(zh_rv, up_clr_func_attr, NULL, NULL);

    delete[] z_base;
    delete[] z_arg;

    factor->mml_nodes_last_term = 3;
    factor->zh_fstr = zh_rv;
  }
}

void STree2MML::SemanticDERIVATIVE2MML(SEMANTICS_NODE * s_derivative,
                                       FACTOR_REC * factor, int& error_code)
{
  BUCKET_REC *diff_var_bucket = NULL;

  if (s_derivative
      && s_derivative->bucket_list
      && s_derivative->bucket_list->first_child) {
    diff_var_bucket = FindBucketRec(s_derivative->bucket_list, MB_DIFF_VAR);
    if (diff_var_bucket) {
      bool do_d_over_dx = false;
      if (up_derivative_format) {
        if (up_derivative_format == 1)
          do_d_over_dx = true;
      } else {
        if (input_notation
            && input_notation->n_doverds > input_notation->n_primes)
          do_d_over_dx = true;
      }
      if (do_d_over_dx) {
        // d/dx output form
        SemanticFracDerivative2MML(s_derivative, factor, true, error_code);
        return;
      }
    }
    // y" output form, or y, or D_x y
    SEMANTICS_NODE *s_function = s_derivative->bucket_list->first_child;
    SEMANTICS_NODE *s_expr = NULL;
    if (s_derivative->bucket_list->next)
      s_expr = s_derivative->bucket_list->next->first_child;

    int nprimes;
    BUCKET_REC *nprimes_bucket =
      FindBucketRec(s_derivative->bucket_list, MB_NPRIMES);
    if (nprimes_bucket && nprimes_bucket->first_child) {
      SEMANTICS_NODE *s_pcount = nprimes_bucket->first_child;
      nprimes = atoi(s_pcount->contents);
    } else {
      //TCI_ASSERT(!"No primes specified, assuming 1.");
      nprimes = 1;
    }
    int deriv_style = 1;        // Use primes
    if (up_derivative_format) {
      if (up_derivative_format == 2) {  // D_x
        if (diff_var_bucket)
          deriv_style = 3;      // Use D_x
      } else if (up_derivative_format == 3) { // primes
          deriv_style = 1;      // Use primes
      } else if (up_derivative_format == 4) { // over dots
        deriv_style = 2;        // Use over dots
      }
    } else {
      if (input_notation) {
        if (input_notation->n_dotaccents && nprimes < 5)
          deriv_style = 2;      // Use over dots
        else if (input_notation->n_Dxs)
          deriv_style = 3;      // Use D_x
      } else {
        TCI_ASSERT(!"Can't resolve derivative style.");
      }
    }
    char *zh_header = NULL;
    if (deriv_style != 3 || nprimes)
      zh_header = FuncHeader2MML(s_function);
    if (zh_header || deriv_style == 3) {
      char *zh_fcall = NULL;
      bool do_subscript_args = false;
      if (input_notation && input_notation->funcarg_is_subscript)
        do_subscript_args = true;
      int nodes_made = 1;
      if (zh_header && s_function->bucket_list) {
        bool trigarg_enabled = false;
        bool trigarg_scripted;
        zh_fcall = ComposeFuncCall(s_function, zh_header, trigarg_enabled,
                                   trigarg_scripted, do_subscript_args,
                                   nprimes, deriv_style, s_derivative);
        if (!do_subscript_args)
          nodes_made += 2;      // &ApplyFunc; + the arg
        delete[] zh_header;
      } else {
        if (nprimes) {
          if (deriv_style == 1) {
            char *primes_str = GetPrimesStr(nprimes);
            char *tmpl = GetTmplPtr(TMPL_MSUP); //  "<msup>\n%base%%script%</msup>\n"
            size_t zln =
              strlen(tmpl) + strlen(zh_header) + strlen(primes_str);
            zh_fcall = new char[zln + 1];
            strcpy(zh_fcall, tmpl);
            StrReplace(zh_fcall, zln, "%base%", zh_header);
            StrReplace(zh_fcall, zln, "%script%", primes_str);
            delete[] primes_str;
          } else if (deriv_style == 2) {
            char *dots_str = GetDotsStr(nprimes);
            char *tmpl = GetTmplPtr(TMPL_MOVER);  //  "<mover>\n%base%%top%</mover>\n"
            size_t zln = strlen(tmpl) + strlen(zh_header) + strlen(dots_str);
            zh_fcall = new char[zln + 1];
            strcpy(zh_fcall, tmpl);
            StrReplace(zh_fcall, zln, "%base%", zh_header);
            StrReplace(zh_fcall, zln, "%top%", dots_str);
            delete[] dots_str;
          } else if (deriv_style == 3) {  // Use D_x
            char *vars_str = GetVarsStr(s_derivative);
            if (!vars_str) {
              TCI_ASSERT(!"No vars_str");
              zh_header = FuncHeader2MML(s_function);
              char *primes_str = GetPrimesStr(nprimes);
              char *tmpl = GetTmplPtr(TMPL_MSUP); //  "<msup>\n%base%%script%</msup>\n"
              size_t zln =
                strlen(tmpl) + strlen(zh_header) + strlen(primes_str);
              zh_fcall = new char[zln + 1];
              strcpy(zh_fcall, tmpl);
              StrReplace(zh_fcall, zln, "%base%", zh_header);
              StrReplace(zh_fcall, zln, "%script%", primes_str);
              delete[] primes_str;
            } else {
              char *dop_str = PrefixMOfromSTRING(up_differentialD);
              char *tmpl = GetTmplPtr(TMPL_DD_HEADER);  //  "<msub>\n%DiffOp%%vars%</msub>\n",
              size_t zln = strlen(tmpl) + strlen(dop_str) + strlen(vars_str);
              char *zh_expr = NULL;
              if (s_expr) {
                int local_nodes, local_terms;
                bool is_signed = false;
                zh_expr = ProcessSemanticList(s_expr, error_code, local_nodes, local_terms,
                                              is_signed, NULL, NULL);
                //TODO fence?
                zln += strlen(zh_expr);
              }
              zh_fcall = new char[zln + 1];
              strcpy(zh_fcall, tmpl);
              StrReplace(zh_fcall, zln, "%DiffOp%", dop_str);
              StrReplace(zh_fcall, zln, "%vars%", vars_str);
              if (zh_expr)
                strcat(zh_fcall, zh_expr);
              delete[] dop_str;
              delete[] zh_expr;
            }
            delete[] vars_str;
          }
          delete[] zh_header;
        } else {
          zh_fcall = zh_header;
        }
      }
      factor->mml_nodes_last_term = nodes_made;
      factor->zh_fstr = zh_fcall;
    }
  }
}

void STree2MML::SemanticPIECEWISE2MML(SEMANTICS_NODE * s_piecewise,
                                      FACTOR_REC * factor, int& error_code)
{
  char *zh_rv = NULL;

  char *zh_line_list = NULL;
  U32 cln = 0;

  char *mtd_tmpl = GetTmplPtr(TMPL_MTD);
  size_t zln_mtd = strlen(mtd_tmpl);

  BUCKET_REC *b_rover = s_piecewise->bucket_list;
  while (b_rover) {             // loop thru UNNAMED buckets
    char *celllist = NULL;      // we make 3 mtd's per mtr
    U32 cl_ln = 0;

    if (b_rover->first_child) {
      SEMANTICS_NODE *s_onepiece = b_rover->first_child;
      BUCKET_REC *onep_b_list = s_onepiece->bucket_list;
      BUCKET_REC *expr_bucket = FindBucketRec(onep_b_list, MB_PIECE_EXPR);
      BUCKET_REC *domn_bucket = FindBucketRec(onep_b_list, MB_UNNAMED);

      if (expr_bucket) {
        SEMANTICS_NODE *s_expr = expr_bucket->first_child;
        int nodes_made, terms_made;
        bool is_signed = false;
        char *zh_expr = ProcessSemanticList(s_expr, error_code,
                                            nodes_made, terms_made,
                                            is_signed, NULL, NULL);
        // Nest the expression in an <mtd>
        size_t zln = zln_mtd;
        if (zh_expr)
          zln += strlen(zh_expr);
        char *zh_mtd = new char[zln + 1];
        strcpy(zh_mtd, mtd_tmpl);
        StrReplace(zh_mtd, zln, "%body%", zh_expr);
        delete[] zh_expr;

        celllist = AppendStr2HeapStr(celllist, cl_ln, zh_mtd);
        delete[] zh_mtd;
      } else {
        TCI_ASSERT(0);
      }
      // Nest "if" in an <mtd>
      char *if_tmpl = GetTmplPtr(TMPL_IF);
      size_t zln = zln_mtd + strlen(if_tmpl);
      char *zh_mtd = new char[zln + 1];
      strcpy(zh_mtd, mtd_tmpl);
      StrReplace(zh_mtd, zln, "%body%", if_tmpl);
      zh_mtd = ColorizeMMLElement(zh_mtd, up_clr_text_attr, NULL, NULL);

      celllist = AppendStr2HeapStr(celllist, cl_ln, zh_mtd);
      delete[] zh_mtd;

      if (domn_bucket) {
        SEMANTICS_NODE *s_cond = domn_bucket->first_child;
        int nodes_made, terms_made;
        bool is_signed = false;
        char *zh_cond = ProcessSemanticList(s_cond, error_code,
                                            nodes_made, terms_made,
                                            is_signed, NULL, NULL);
        // Nest the domain expression in an <mtd>
        size_t zln = zln_mtd;
        if (zh_cond)
          zln += strlen(zh_cond);
        char *zh_mtd = new char[zln + 1];
        strcpy(zh_mtd, mtd_tmpl);
        StrReplace(zh_mtd, zln, "%body%", zh_cond);
        delete[] zh_cond;

        celllist = AppendStr2HeapStr(celllist, cl_ln, zh_mtd);
        delete[] zh_mtd;
      } else {
        TCI_ASSERT(0);
      }
      // Nest the "celllist" in an <mtr>
      char *mtr_tmpl = GetTmplPtr(TMPL_MTR);
      zln = strlen(mtr_tmpl + 1);
      if (celllist)
        zln += strlen(celllist);
      char *zh_mtr = new char[zln + 1];
      strcpy(zh_mtr, mtr_tmpl);
      StrReplace(zh_mtr, zln, "%celllist%", celllist);
      delete[] celllist;

      zh_line_list = AppendStr2HeapStr(zh_line_list, cln, zh_mtr);
      delete[] zh_mtr;
    } else {
      TCI_ASSERT(0);
    }
    b_rover = b_rover->next;
  }                             // loop thru s_piecewise buckets

  // Nest zh_line_list in an <mtable>
  char *mtable_tmpl = GetTmplPtr(TMPL_MTABLE);
  size_t zln = strlen(mtable_tmpl + 1);
  zln += strlen(zh_line_list);
  zh_rv = new char[zln + 1];
  strcpy(zh_rv, mtable_tmpl);
  StrReplace(zh_rv, zln, "%rowlist%", zh_line_list);
  delete[] zh_line_list;

  // Nest mtable in a fence  {<mtable>|
  char *pw_fence_tmpl = GetTmplPtr(TMPL_PIECEWISE_FENCE);
  if (pw_fence_tmpl) {
    size_t ln = strlen(pw_fence_tmpl);
    char *tmp = zh_rv;
    if (tmp)
      ln += strlen(tmp);
    zh_rv = new char[ln + 1];
    strcpy(zh_rv, pw_fence_tmpl);

    StrReplace(zh_rv, ln, "%interior%", tmp);
    zh_rv = ColorizeMMLElement(zh_rv, up_clr_math_attr, NULL, NULL);
    delete[] tmp;
  }

  if (zh_rv) {
    factor->zh_fstr = zh_rv;
    factor->mml_nodes_last_term = 1;
  } else {
    error_code = 1;
  }
}

void STree2MML::SemanticLIMFUNC2MML(SEMANTICS_NODE * s_limfunc,
                                    FACTOR_REC * factor, int& error_code)
{
  char *zh_rv = NULL;
  error_code = 0;

  int rv_nodes_made = 0;
  if (s_limfunc && s_limfunc->bucket_list) {
    BUCKET_REC *bl = s_limfunc->bucket_list;
    BUCKET_REC *b_arg = FindBucketRec(bl, MB_UNNAMED);
    BUCKET_REC *b_llim = FindBucketRec(bl, MB_LOWERLIMIT);

    int nodes_made, terms_made;
    bool is_signed = false;
    char *z_arg = NULL;
    if (b_arg) {
      z_arg = ProcessSemanticList(b_arg->first_child,
                                  error_code, nodes_made, terms_made,
                                  is_signed, NULL, NULL);
      if (z_arg)
        if (terms_made > 1 || nodes_made > 1 || is_signed) {
          z_arg = NestzMMLInMROW(z_arg);
          z_arg = NestzMMLInPARENS(z_arg, rv_nodes_made);
        } else {
          rv_nodes_made = nodes_made;
        }
    }

    char *z_llim = NULL;
    if (b_llim) {
      if (b_llim->parts) {
        z_llim = LowerLim2MML(b_llim->parts);
      } else {
        SEMANTICS_NODE *s_ll_list = b_llim->first_child;
        z_llim = ProcessSemanticList(s_ll_list, error_code,
                                     nodes_made, terms_made,
                                     is_signed, NULL, NULL);
        if (z_llim)
          if (terms_made > 1 || nodes_made > 1 || is_signed)
            z_llim = NestzMMLInMROW(z_llim);
      }
    }

    char *opname = s_limfunc->contents;

    char *tmpl = GetTmplPtr(TMPL_LIMFUNC);
    //  "<munder>\n<mo form=\"prefix\"%color_attr%>%opname%</mo>\n%llim%</munder>\n%arg%",
    size_t zln = strlen(tmpl);
    if (opname)
      zln += strlen(opname);
    if (z_llim)
      zln += strlen(z_llim);
    if (z_arg)
      zln += strlen(z_arg);

    zh_rv = new char[zln + 1];
    strcpy(zh_rv, tmpl);

    StrReplace(zh_rv, zln, "%opname%", opname);
    StrReplace(zh_rv, zln, "%llim%", z_llim);
    StrReplace(zh_rv, zln, "%arg%", z_arg);
    zh_rv = ColorizeMMLElement(zh_rv, up_clr_func_attr, NULL, NULL);

    delete[] z_arg;
    delete[] z_llim;
  } else {
    TCI_ASSERT(0);
  }
  if (zh_rv) {
    factor->zh_fstr = zh_rv;
    factor->mml_nodes_last_term = 1 + rv_nodes_made;
  } else {
    error_code = 1;
  }
}

// Note that engine-specific analyzers (MuAnalyzer, etc.) produce
//  semantic nodes of type SEM_TYP_INFIX_OP, etc. with contents
//  that contain the MathML <mo> contents for that particular operator.

void STree2MML::SemanticOPERATOR2MML(SEMANTICS_NODE * s_operator,
                                     FACTOR_REC * factor, int& error_code)
{
  char *zh_rv = NULL;
  error_code = 0;

  bool omit = false;
  if (s_operator->contents) {
    char *fixed_op = s_operator->contents;
    char *ptr = strstr(s_operator->contents, "&#x");
    if (ptr) {
      U32 unicode = ASCII2U32(ptr + 3, 16);
      if (unicode == 0x2062)  // &invisibletimes;
        omit = true;
      else if (unicode == 0xd7       // &times;<uID3.13.3>infix,39,U000D7
        || unicode == 0x2217) { // &midast;
        if (IsExposedNumber(s_operator->prev) && IsExposedPositiveNumber(s_operator->next))
          omit = false;
        else
          omit = true;
      }
    }
    if (!omit) {
      zh_rv = MOfromSTRING(fixed_op, up_clr_math_attr);
      factor->zh_fstr = zh_rv;
      factor->is_operator = true;
    } else {
      error_code = 2;
    }
  } else {
    error_code = 1;
  }
}

// In the following, "FENCE" means a comma separated list
//  delimited by (), [] or {}.

void STree2MML::SemanticFENCE2MML(SEMANTICS_NODE * sem_set_node,
                                  FACTOR_REC * factor,
                                  bool add_delims, int& error_code)
{
  char *rv = NULL;

  char *zh_interior = NULL;
  U32 interior_ln = 0;

  if (sem_set_node && sem_set_node->bucket_list) {
    char *comma_markup = GetTmplPtr(TMPL_COMMA);
    size_t c_ln = strlen(comma_markup);
    char *z_comma = new char[c_ln + 1];
    strcpy(z_comma, comma_markup);
    z_comma = ColorizeMMLElement(z_comma, up_clr_math_attr, NULL, NULL);

    int nodes_made, terms_made;
    bool is_signed = false;

    BUCKET_REC *element_b_rover = sem_set_node->bucket_list;
    int node_counter = 0;
    while (element_b_rover) {
      if (node_counter) {
        if (!up_use_mfenced || !add_delims) {
          zh_interior = AppendStr2HeapStr(zh_interior, interior_ln, z_comma);
          node_counter++;
        }
      }
      if (element_b_rover->first_child) {
        SEMANTICS_NODE *head = element_b_rover->first_child;
        char *z_arg = NULL;
        if (head) {
          z_arg = ProcessSemanticList(head, error_code,
                                      nodes_made, terms_made, is_signed, NULL,
                                      NULL);
          if (z_arg)
            if (terms_made > 1 || nodes_made > 1 || is_signed) {
              z_arg = NestzMMLInMROW(z_arg);
              terms_made = 1;
              nodes_made = 1;
            }
        }
        if (z_arg) {
          zh_interior = AppendStr2HeapStr(zh_interior, interior_ln, z_arg);
          delete[] z_arg;
          node_counter++;
        }
      }
      element_b_rover = element_b_rover->next;
    }
    delete[] z_comma;
    char *tmpl = NULL;
    if (add_delims) {
      if (up_use_mfenced) {
        char *z_left_delim = NULL;
        char *z_right_delim = NULL;

        switch (sem_set_node->semantic_type) {
        case SEM_TYP_PARENED_LIST:
          z_left_delim = "(";
          z_right_delim = ")";
          break;
        case SEM_TYP_BRACKETED_LIST:
          z_left_delim = "[";
          z_right_delim = "]";
          break;
        case SEM_TYP_SET:
          z_left_delim = "{";
          z_right_delim = "}";
          break;
        default:
          TCI_ASSERT(0);
          break;
        }

        if (node_counter == 1)
          if (nodes_made > 1 || is_signed)
            zh_interior = NestzMMLInMROW(zh_interior);

        char *tmpl = GetTmplPtr(TMPL_MFENCED);
        size_t zln =
          strlen(tmpl) + strlen(z_left_delim) + strlen(z_right_delim);
        if (zh_interior)
          zln += strlen(zh_interior);
        rv = new char[zln + 1];
        strcpy(rv, tmpl);
        StrReplace(rv, zln, "%open%", z_left_delim);
        StrReplace(rv, zln, "%close%", z_right_delim);
        StrReplace(rv, zln, "%body%", zh_interior);

        delete[] zh_interior;
        factor->mml_nodes_last_term = 1;
      } else {
        if (sem_set_node->semantic_type == SEM_TYP_PARENED_LIST)
          tmpl = GetTmplPtr(TMPL_PARENS);
        else if (sem_set_node->semantic_type == SEM_TYP_BRACKETED_LIST)
          tmpl = GetTmplPtr(TMPL_BRACKETS);
        else if (sem_set_node->semantic_type == SEM_TYP_SET)
          tmpl = GetTmplPtr(TMPL_BRACES);
        else
          TCI_ASSERT(0);
        //  "<mrow>\n<mo%color_attr%>{</mo>\n%interior%<mo%color_attr2%>}</mo>\n</mrow>\n",

        if (node_counter > 1)
          zh_interior = NestzMMLInMROW(zh_interior);

        size_t ln = strlen(tmpl);
        if (zh_interior)
          ln += strlen(zh_interior);
        rv = new char[ln + 1];
        strcpy(rv, tmpl);
        StrReplace(rv, ln, "%interior%", zh_interior);
        rv = ColorizeMMLElement(rv, up_clr_math_attr, up_clr_math_attr, NULL);
        delete[] zh_interior;
        factor->mml_nodes_last_term = 3;
      }
    } else {
      // no delimiters clause
      factor->mml_nodes_last_term = node_counter;
      rv = zh_interior;
    }
  } else {                      // if ( sem_set_node && sem_set_node->bucket_list )
    // Here the set is empty
    if (sem_set_node->semantic_type == SEM_TYP_SET) {
      char *tmpl = GetTmplPtr(TMPL_UPRIGHTMI);  // "<mi%color_attr% fontstyle=\"upright\">%letters%</mi>\n",

      size_t ln = strlen(tmpl) + 8;
      rv = new char[ln + 1];
      strcpy(rv, tmpl);
      StrReplace(rv, ln, "%letters%", "&#x2205;");
      rv = ColorizeMMLElement(rv, up_clr_math_attr, NULL, NULL);

      factor->mml_nodes_last_term = 1;
    } else if (sem_set_node->semantic_type == SEM_TYP_BRACKETED_LIST) {
      char *tmpl = GetTmplPtr(TMPL_BRACKETS);
      //  "<mo%color_attr%>[</mo>\n%interior%<mo%color_attr2%>]</mo>\n",

      size_t ln = strlen(tmpl);
      rv = new char[ln + 1];
      strcpy(rv, tmpl);
      StrReplace(rv, ln, "%interior%", NULL);
      rv = ColorizeMMLElement(rv, up_clr_math_attr, up_clr_math_attr, NULL);

      factor->mml_nodes_last_term = 2;
    }
  }
  if (rv)
    factor->zh_fstr = rv;
  else
    error_code = 1;
}

void STree2MML::SemanticSUBS2MML(SEMANTICS_NODE * sem_set_node,
                                 FACTOR_REC * factor, int& error_code)
{
  char *rv = NULL;
  char *z_body = NULL;

  char *zh_interior = NULL;
  U32 interior_ln = 0;
  int nodes_made, terms_made;
  bool is_signed = false;

  BUCKET_REC *element_b_rover = NULL;
  if (sem_set_node && sem_set_node->bucket_list) {
    element_b_rover = sem_set_node->bucket_list;
    if (element_b_rover) {
      if (element_b_rover->first_child) {
        SEMANTICS_NODE *head = element_b_rover->first_child;
        char *z_arg = NULL;
        if (head) {
          z_arg = ProcessSemanticList(head, error_code,
                                      nodes_made, terms_made, is_signed, NULL,
                                      NULL);
          if (z_arg)
            if (terms_made > 1 || nodes_made > 1 || is_signed)
              z_arg = NestzMMLInMROW(z_arg);
        }
        if (z_arg) {
          zh_interior = AppendStr2HeapStr(zh_interior, interior_ln, z_arg);
          delete[] z_arg;
        }
      }
      element_b_rover = element_b_rover->next;
    }
    char *tmpl = NULL;
    if (up_use_mfenced) {
      char *z_left_delim = NULL;
      char *z_right_delim = NULL;

      z_left_delim = "[";
      z_right_delim = "]";

      if (nodes_made > 1 || is_signed)
        zh_interior = NestzMMLInMROW(zh_interior);

      tmpl = GetTmplPtr(TMPL_MFENCED);
      size_t zln =
        strlen(tmpl) + strlen(z_left_delim) + strlen(z_right_delim);
      if (zh_interior)
        zln += strlen(zh_interior);
      z_body = new char[zln + 1];
      strcpy(z_body, tmpl);
      StrReplace(z_body, zln, "%open%", z_left_delim);
      StrReplace(z_body, zln, "%close%", z_right_delim);
      StrReplace(z_body, zln, "%body%", zh_interior);

      delete[] zh_interior;
    } else {
      tmpl = GetTmplPtr(TMPL_BRACKETS);
      //  "<mrow>\n<mo%color_attr%>{</mo>\n%interior%<mo%color_attr2%>}</mo>\n</mrow>\n",

      size_t ln = strlen(tmpl);
      if (zh_interior)
        ln += strlen(zh_interior);
      z_body = new char[ln + 1];
      strcpy(z_body, tmpl);
      StrReplace(z_body, ln, "%interior%", zh_interior);
      z_body = ColorizeMMLElement(z_body, up_clr_math_attr, up_clr_math_attr, NULL);
      delete[] zh_interior;
    }
  }
  //TODO get sub & sup and attach
  char *z_sub = NULL;
  if (element_b_rover) {
    if (element_b_rover->first_child) {
      SEMANTICS_NODE *head = element_b_rover->first_child;
      if (head) {
        z_sub = ProcessSemanticList(head, error_code,
                                    nodes_made, terms_made, is_signed, NULL,
                                    NULL);
        if (z_sub && (terms_made > 1 || nodes_made > 1 || is_signed))
          z_sub = NestzMMLInMROW(z_sub);
      }
    }
    element_b_rover = element_b_rover->next;
  }
  char *z_sup = NULL;
  if (element_b_rover) {
    if (element_b_rover->first_child) {
      SEMANTICS_NODE *head = element_b_rover->first_child;
      if (head) {
        z_sup = ProcessSemanticList(head, error_code,
                                    nodes_made, terms_made, is_signed, NULL,
                                    NULL);
        if (z_sup && (terms_made > 1 || nodes_made > 1 || is_signed))
          z_sup = NestzMMLInMROW(z_sup);
      }
    }
  }
  //   "<msubsup>\n%FenceBody%\n%lower%\n%upper%\n</msubsup>\n",
  if (z_body && (z_sub || z_sup)) {
    char *tmpl = GetTmplPtr(TMPL_SUBSTITUTION);
    size_t zln =
      strlen(tmpl) + strlen(z_body) + (z_sub?strlen(z_sub):0) + (z_sup?strlen(z_sup):0);
    rv = new char[zln + 1];
    strcpy(rv, tmpl);
    if (!z_sup){
      StrReplace(rv, zln, "msubsup", "msub");
      StrReplace(rv, zln, "/msubsup", "/msub");
    } else if (!z_sup) {
      StrReplace(rv, zln, "msubsup", "msup");
      StrReplace(rv, zln, "/msubsup", "/msup");
    }

    StrReplace(rv, zln, "%FenceBody%", z_body);
    StrReplace(rv, zln, "%lower%", z_sub?z_sub:"");
    StrReplace(rv, zln, "%upper%", z_sup?z_sup:"");

    delete[] z_body;
    delete[] z_sub;
    delete[] z_sup;
    factor->mml_nodes_last_term = 1;   // right?
  } else {
    TCI_ASSERT(!"Missing term in substitution.  Possible to repair?");
  }

  if (rv)
    factor->zh_fstr = rv;
  else
    error_code = 1;
}

// precedence groups delimit operands
void STree2MML::SemanticPGROUP2MML(SEMANTICS_NODE * s_group,
                                   FACTOR_REC * factor, int& error_code)
{
  error_code = 1;
  int p_level = 0;

  if (s_group && s_group->bucket_list) {
    p_level = GetInfixPrecedence(s_group);
    int p_left = 0;
    int p_right = 0;
    bool after_minus = false;
    bool prev_is_minus = false;
    if (s_group->prev && s_group->prev->contents)
      prev_is_minus = strcmp(s_group->prev->contents,"&#x2212;") == 0
                    || strcmp(s_group->prev->contents,"-") == 0;
    if (s_group->prev && s_group->prev->semantic_type == SEM_TYP_INFIX_OP) {
      p_left = s_group->prev->infix_precedence;
      after_minus = prev_is_minus;
    }
    if (s_group->next && s_group->next->semantic_type == SEM_TYP_INFIX_OP)
      p_right = s_group->next->infix_precedence;

    bool nest_in_parens = false;
    bool force_parens = false;
    if (up_output_parens_mode) {
      nest_in_parens = true;
    } else if (s_group->next && s_group->next->semantic_type == SEM_TYP_POSTFIX_OP) { // '!'
      nest_in_parens = true;
    } else if (s_group->prev && s_group->prev->semantic_type == SEM_TYP_PREFIX_OP) { // 'grad'
      force_parens = !prev_is_minus;
    } else {
      if (p_left && (p_left > p_level))
        nest_in_parens = true;
      else if (p_right && (p_right > p_level))
        nest_in_parens = true;
      else
        nest_in_parens = after_minus;
    }
    if (s_group->semantic_type == SEM_TYP_GENERIC_FENCE)
      nest_in_parens = true;

    BUCKET_REC *b_list = s_group->bucket_list;
    if (b_list->first_child) {
      SEMANTICS_NODE *head = b_list->first_child;
      int nodes_made, terms_made;
      int err_code = 0;
      bool is_signed = false;
      char *zh_rv = ProcessSemanticList(head, err_code,
                                        nodes_made, terms_made,
                                        is_signed, NULL, NULL);
      if (!err_code && zh_rv) {
        error_code = 0;
        if ((nest_in_parens && nodes_made > 1) || force_parens) {
          int rv_nodes_made;
          zh_rv = NestzMMLInMROW(zh_rv);
          zh_rv = NestzMMLInPARENS(zh_rv, rv_nodes_made);
          factor->zh_fstr = zh_rv;
          factor->n_terms = 1;
          factor->mml_nodes_last_term = rv_nodes_made;
          factor->is_signed = false;
          factor->is_number = false;
        } else {
          factor->zh_fstr = zh_rv;
          factor->n_terms = terms_made;
          factor->mml_nodes_last_term = nodes_made;
          factor->is_signed = is_signed;
          factor->is_number = IsExposedNumber(head);
        }
      }
    } else {
      TCI_ASSERT(!"nothing in first bucket");
    }
  } else {
    TCI_ASSERT(!"no bucket list");
  }
}

void STree2MML::SemanticMATH2MML(SEMANTICS_NODE * s_math,
                                 FACTOR_REC * factor, int& error_code)
{
  char *zh_rv = NULL;

  if (s_math && s_math->bucket_list) {
    BUCKET_REC *b_list = s_math->bucket_list;
    if (b_list->first_child) {
      SEMANTICS_NODE *head = b_list->first_child;
      int nodes_made, terms_made;
      bool is_signed = false;
      zh_rv = ProcessSemanticList(head, error_code,
                                  nodes_made, terms_made,
                                  is_signed, NULL, NULL);
    }
  }
  if (zh_rv)
    factor->zh_fstr = zh_rv;
  else
    error_code = 1;
}

void STree2MML::SemanticFRACTION2MML(SEMANTICS_NODE * s_fraction,
                                     int& error_code,
                                     FACTOR_REC ** num_factors,
                                     FACTOR_REC ** den_factors)
{

  #ifdef DEBUG
  char start_msg[100];
  sprintf( start_msg, "\n\n========== STree2MML::SemanticFRACTION2MML ============");
  JBM::JBMLine(start_msg);
  #endif


  BUCKET_REC *b_list = s_fraction->bucket_list;
  BUCKET_REC *num_bucket = FindBucketRec(b_list, MB_NUMERATOR);
  BUCKET_REC *den_bucket = FindBucketRec(b_list, MB_DENOMINATOR);

  int nodes_made, terms_made;
  bool is_signed = false;
  char *z_num = NULL;
  if (num_bucket) {
    SEMANTICS_NODE *s_num = num_bucket->first_child;
    s_num = RemoveParens(s_num, num_bucket);
    z_num = ProcessSemanticList(s_num, error_code,
                                nodes_made, terms_made,
                                is_signed, num_factors, den_factors);
    if (z_num) {
      // #ifdef DEBUG
      // char msg[1000];
      // sprintf( msg, "\n\nNumerator = %s", z_num);
      // JBM::JBMLine(msg);
      // #endif

      FACTOR_REC *new_fr = CreateFactor();
      new_fr->zh_fstr = z_num;
      new_fr->n_terms = terms_made;
      new_fr->mml_nodes_last_term = nodes_made;
      new_fr->is_number = IsExposedNumber(s_num);
      new_fr->is_signed = is_signed;

      *num_factors = JoinFactors(*num_factors, new_fr);
    }
  }
  char *z_den = NULL;
  if (den_bucket) {
    SEMANTICS_NODE *s_den = den_bucket->first_child;
    s_den = RemoveParens(s_den, den_bucket);
    z_den = ProcessSemanticList(s_den, error_code,
                                nodes_made, terms_made,
                                is_signed, den_factors, num_factors);
    if (z_den) {
      // #ifdef DEBUG
      // char msg[1000];
      // sprintf( msg, "\n\nDenominator = %s", z_den);
      // JBM::JBMLine(msg);
      // #endif
      FACTOR_REC *new_fr = CreateFactor();
      new_fr->zh_fstr = z_den;
      new_fr->n_terms = terms_made;
      new_fr->mml_nodes_last_term = nodes_made;
      new_fr->is_number = IsExposedNumber(s_den);
      new_fr->is_signed = is_signed;

      *den_factors = JoinFactors(*den_factors, new_fr);
    }
  }
}

void STree2MML::SemanticBINOMIAL2MML(SEMANTICS_NODE * s_fraction,
                                     FACTOR_REC * factor, int& error_code)
{
  char *rv = NULL;

  BUCKET_REC *b_list = s_fraction->bucket_list;
  BUCKET_REC *num_bucket = FindBucketRec(b_list, MB_NUMERATOR);
  BUCKET_REC *den_bucket = FindBucketRec(b_list, MB_DENOMINATOR);

  int nodes_made, terms_made;
  bool is_signed = false;
  char *z_num = NULL;
  if (num_bucket) {
    z_num = ProcessSemanticList(num_bucket->first_child,
                                error_code, nodes_made, terms_made,
                                is_signed, NULL, NULL);
    if (z_num && nodes_made > 1)
      z_num = NestzMMLInMROW(z_num);
  }
  char *z_den = NULL;
  if (den_bucket) {
    z_den = ProcessSemanticList(den_bucket->first_child,
                                error_code, nodes_made, terms_made,
                                is_signed, NULL, NULL);
    if (z_den && nodes_made > 1)
      z_den = NestzMMLInMROW(z_den);
  }

  char *tmpl = GetTmplPtr(TMPL_BINOMIAL);
  size_t zln = strlen(tmpl);
  if (z_num)
    zln += strlen(z_num);
  if (z_den)
    zln += strlen(z_den);

  rv = new char[zln + 1];
  strcpy(rv, tmpl);
  StrReplace(rv, zln, "%num%", z_num);
  StrReplace(rv, zln, "%den%", z_den);

  delete[] z_num;
  delete[] z_den;

  int rv_nodes_made;
  rv = NestzMMLInPARENS(rv, rv_nodes_made);

  if (rv) {
    factor->zh_fstr = rv;
    factor->mml_nodes_last_term = rv_nodes_made;
  } else {
    error_code = 1;
  }
}

void STree2MML::SemanticINTERVAL2MML(SEMANTICS_NODE * s_interval,
                                     FACTOR_REC * factor, int& error_code)
{
  char *rv = NULL;

  BUCKET_REC *b_list = s_interval->bucket_list;
  BUCKET_REC *num_bucket = FindBucketRec(b_list, MB_INTERVAL_START);
  BUCKET_REC *den_bucket = FindBucketRec(b_list, MB_INTERVAL_END);

  int nodes_made, terms_made;
  bool is_signed = false;
  char *z_left = NULL;
  if (num_bucket) {
    z_left = ProcessSemanticList(num_bucket->first_child,
                                 error_code, nodes_made, terms_made,
                                 is_signed, NULL, NULL);
    if (z_left)
      if (nodes_made > 1 || terms_made > 1 || is_signed)
        z_left = NestzMMLInMROW(z_left);
  }

  char *z_right = NULL;
  if (den_bucket) {
    z_right = ProcessSemanticList(den_bucket->first_child,
                                  error_code, nodes_made, terms_made,
                                  is_signed, NULL, NULL);
    if (z_right)
      if (nodes_made > 1 || terms_made > 1 || is_signed)
        z_right = NestzMMLInMROW(z_right);
  }

  char *l_delim = "(";
  if (s_interval->variant == SNV_InclExclInterval
      || s_interval->variant == SNV_InclInclInterval)
    l_delim = "[";

  char *r_delim = ")";
  if (s_interval->variant == SNV_ExclInclInterval
      || s_interval->variant == SNV_InclInclInterval)
    r_delim = "]";

  if (up_use_mfenced) {
    char *tmpl = GetTmplPtr(TMPL_MFENCED);
    //  "<mfenced open=\"%open%\" close=\"%close%\">\n%body%</mfenced>\n",
    size_t zln = strlen(tmpl) + strlen(l_delim) + strlen(r_delim);
    if (z_left)
      zln += strlen(z_left);
    if (z_right)
      zln += strlen(z_right);

    rv = new char[zln + 1];
    strcpy(rv, tmpl);
    StrReplace(rv, zln, "%open%", l_delim);
    StrReplace(rv, zln, "%close%", r_delim);

    size_t ln = strlen(z_left) + strlen(z_right);
    char *tmp = new char[ln + 1];
    strcpy(tmp, z_left);
    strcat(tmp, z_right);

    StrReplace(rv, zln, "%body%", tmp);
    delete[] tmp;
    factor->mml_nodes_last_term = 1;
  } else {
    char *tmpl = GetTmplPtr(TMPL_INTERVAL);
    //  "<mo%color_attr%>%l_delim%</mo>\n<mrow>\n%left%<mo%color_attr2%>,</mo>\n%right%</mrow>\n<mo%color_attr3%>%r_delim%</mo>\n",
    size_t zln = strlen(tmpl) + strlen(l_delim) + strlen(r_delim);
    if (z_left)
      zln += strlen(z_left);
    if (z_right)
      zln += strlen(z_right);

    rv = new char[zln + 1];
    strcpy(rv, tmpl);
    StrReplace(rv, zln, "%l_delim%", l_delim);
    StrReplace(rv, zln, "%left%", z_left);
    StrReplace(rv, zln, "%right%", z_right);
    StrReplace(rv, zln, "%r_delim%", r_delim);
    rv =
      ColorizeMMLElement(rv, up_clr_math_attr, up_clr_math_attr,
                         up_clr_math_attr);
    factor->mml_nodes_last_term = 3;
  }

  delete[] z_left;
  delete[] z_right;

  if (rv) {
    factor->zh_fstr = rv;
  } else {
    error_code = 1;
  }
}

void STree2MML::SemanticPOWERFORM2MML(SEMANTICS_NODE * snode,
                                      FACTOR_REC * factor, int& error_code)
{
  char *rv = NULL;
  int nodes_made = 0;

  BUCKET_REC *b_list = snode->bucket_list;
  BUCKET_REC *base_bucket = FindBucketRec(b_list, MB_SCRIPT_BASE);
  BUCKET_REC *sup_bucket = FindBucketRec(b_list, MB_SCRIPT_UPPER);

  char *z_base = NULL;
  int base_nodes_made, base_terms_made;
  bool base_is_signed = false;
  if (base_bucket)
    z_base = ProcessSemanticList(base_bucket->first_child,
                                 error_code, base_nodes_made, base_terms_made,
                                 base_is_signed, NULL, NULL);

  bool is_sqrt;
  bool is_one_over_n = ScriptIsOneOverN(sup_bucket, is_sqrt);

  if (is_one_over_n) {          // use a surd form
    int tmpl_ID = is_sqrt ? TMPL_MSQRT : TMPL_MROOT;
    //"<mml:mroot>\n%base%%exp%</mml:mroot>\n",

    char *z_exp = NULL;
    if (!is_sqrt) {
      if (sup_bucket && sup_bucket->first_child) {
        SEMANTICS_NODE *script_cont = sup_bucket->first_child;
        SEMANTICS_NODE *fraction = NULL;
        if (script_cont->semantic_type == SEM_TYP_PARENED_LIST) {
          BUCKET_REC *b_list = script_cont->bucket_list;
          if (b_list && b_list->first_child)
            fraction = b_list->first_child;
        } else {
          fraction = script_cont;
        }
        if (fraction && fraction->semantic_type == SEM_TYP_FRACTION) {
          BUCKET_REC *denom_bucket =
            FindBucketRec(fraction->bucket_list, MB_DENOMINATOR);
          if (denom_bucket) {
            int local_nodes_made, local_terms_made;
            bool is_signed = false;
            SEMANTICS_NODE *s_head = denom_bucket->first_child;
            z_exp =
              ProcessSemanticList(s_head, error_code, local_nodes_made,
                                  local_terms_made, is_signed, NULL, NULL);
            if (z_exp) {
              if (local_terms_made > 1 || local_nodes_made > 1)
                z_exp = NestzMMLInMROW(z_exp);
              if (local_terms_made > 1) {
                int tmp_nodes_made;
                z_exp = NestzMMLInPARENS(z_exp, tmp_nodes_made);
                if (tmp_nodes_made > 1)
                  z_exp = NestzMMLInMROW(z_exp);
              }
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

      if (z_base)
        if (base_terms_made > 1 || base_nodes_made > 1 || base_is_signed)
          z_base = NestzMMLInMROW(z_base);
    }

    char *tmpl = GetTmplPtr(tmpl_ID);
    size_t zln = strlen(tmpl);
    if (z_base)
      zln += strlen(z_base);
    if (z_exp)
      zln += strlen(z_exp);
    rv = new char[zln + 1];
    strcpy(rv, tmpl);
    StrReplace(rv, zln, "%base%", z_base);
    if (!is_sqrt) {
      StrReplace(rv, zln, "%exp%", z_exp);
      delete[] z_exp;
    }
    nodes_made = 1;
  } else {                      // use a power form
    SEMANTICS_NODE *s_base = NULL;
    bool is_function = false;

    if (base_bucket && base_bucket->first_child) {
      s_base = base_bucket->first_child;
      if (s_base->semantic_type == SEM_TYP_FUNCTION && !s_base->next) {
        delete[] z_base;
        z_base = FuncHeader2MML(s_base);
        is_function = true;
      }
    }
    if (z_base && !is_function) {
      bool add_parens = false;
      if (base_terms_made > 1 || base_nodes_made > 1 || base_is_signed) {
        z_base = NestzMMLInMROW(z_base);
        add_parens = true;
      } else {
        SEMANTICS_NODE *s_base = base_bucket->first_child;
        switch (s_base->semantic_type) {
        case SEM_TYP_FRACTION:
        case SEM_TYP_POWERFORM:
        case SEM_TYP_SQRT:
        case SEM_TYP_ROOT:
        case SEM_TYP_BIGOP_SUM:
        case SEM_TYP_BIGOP_INTEGRAL:
        //        case SEM_TYP_TABULATION       :  handled in TABULATION2MML
        case SEM_TYP_BINOMIAL:
        case SEM_TYP_CONJUGATE:
          add_parens = true;
          break;
        default:
          break;
        }
      }
      if (add_parens) {
        int tmp_nodes_made;
        z_base = NestzMMLInPARENS(z_base, tmp_nodes_made);
        if (tmp_nodes_made > 1)
          z_base = NestzMMLInMROW(z_base);
      } else {
        factor->is_number = IsExposedNumber(snode);
      }
    }
    char *z_sup = NULL;
    if (sup_bucket && !is_sqrt) {
      int local_nodes_made, local_terms_made;
      bool is_signed = false;
      z_sup = ProcessSemanticList(sup_bucket->first_child,
                                  error_code, local_nodes_made,
                                  local_terms_made, is_signed, NULL, NULL);
      if (z_sup) {
        if (local_terms_made > 1 || local_nodes_made > 1 || is_signed) {
          z_sup = NestzMMLInMROW(z_sup);
        }
      }
    }

    char *tmpl = GetTmplPtr(TMPL_MSUP);
    size_t zln = strlen(tmpl);
    if (z_base)
      zln += strlen(z_base);
    if (z_sup)
      zln += strlen(z_sup);
    rv = new char[zln + 1];
    strcpy(rv, tmpl);

    StrReplace(rv, zln, "%base%", z_base);
    StrReplace(rv, zln, "%script%", z_sup);

    delete[] z_sup;
    nodes_made = 1;

    if (rv && is_function) {
      bool trigarg_enabled = IsTrigArgFunc(s_base);
      bool trigarg_scripted;
      int deriv_style = 0;
      char *zh_header = rv;
      rv = ComposeFuncCall(s_base, zh_header, trigarg_enabled,
                           trigarg_scripted, false, 0, deriv_style, NULL);
      nodes_made = 3;

      if (trigarg_scripted)
        if (ParensRequired(snode))
          rv = NestzMMLInPARENS(rv, nodes_made);
      delete[] zh_header;
    }
  }
  delete[] z_base;

  SetUnitInfo(snode, factor);
  if (rv) {
    factor->zh_fstr = rv;
    factor->mml_nodes_last_term = nodes_made;
  } else {
    error_code = 1;
  }
}

void STree2MML::SemanticSQROOT2MML(SEMANTICS_NODE * snode,
                                   FACTOR_REC * factor, int& error_code)
{
  char *rv = NULL;

  char *z_base = NULL;
  if (snode->bucket_list) {
    BUCKET_REC *b_list = snode->bucket_list;
    int nodes_made, terms_made;
    bool is_signed = false;
    if (b_list->first_child)
      z_base = ProcessSemanticList(b_list->first_child,
                                   error_code, nodes_made, terms_made,
                                   is_signed, NULL, NULL);
  }

  char *tmpl = GetTmplPtr(TMPL_MSQRT);
  size_t zln = strlen(tmpl);
  if (z_base)
    zln += strlen(z_base);

  rv = new char[zln + 1];
  strcpy(rv, tmpl);

  StrReplace(rv, zln, "%base%", z_base);
  delete[] z_base;

  if (rv) {
    factor->zh_fstr = rv;
    factor->mml_nodes_last_term = 1;
  } else {
    error_code = 1;
  }
}

void STree2MML::SemanticQUANTILE2MML(SEMANTICS_NODE * s_quant_result,
                                     FACTOR_REC * factor, int& error_code)
{
  char *rv = NULL;

  if (s_quant_result && s_quant_result->bucket_list) {
    BUCKET_REC *percent_bucket =
      FindBucketRec(s_quant_result->bucket_list, MB_WHICH_QUANTILE);
    if (percent_bucket && percent_bucket->first_child) {
      int nodes_made, terms_made;
      bool is_signed = false;
      char *z_percent = ProcessSemanticList(percent_bucket->first_child,
                                            error_code, nodes_made,
                                            terms_made,
                                            is_signed, NULL, NULL);
      if (z_percent)
        if (terms_made > 1 || nodes_made > 1)
          z_percent = NestzMMLInMROW(z_percent);

      char *z_result = NULL;
      BUCKET_REC *res_bucket =
        FindBucketRec(s_quant_result->bucket_list, MB_UNNAMED);
      if (res_bucket && res_bucket->first_child) {
        z_result = ProcessSemanticList(res_bucket->first_child,
                                       error_code, nodes_made, terms_made,
                                       is_signed, NULL, NULL);
        if (z_result)
          if (terms_made > 1 || nodes_made > 1)
            z_result = NestzMMLInMROW(z_result);
      } else {
        TCI_ASSERT(0);
      }
      char *comma_markup = GetTmplPtr(TMPL_COMMA);
      size_t c_ln = strlen(comma_markup);
      char *z_comma = new char[c_ln + 1];
      strcpy(z_comma, comma_markup);
      z_comma = ColorizeMMLElement(z_comma, up_clr_math_attr, NULL, NULL);

      char *tmpl = GetTmplPtr(TMPL_QUANTILE);
      size_t zln = strlen(tmpl) + strlen(z_comma);
      if (z_percent)
        zln += strlen(z_percent);
      if (z_result)
        zln += strlen(z_result);

      rv = new char[zln + 1];
      strcpy(rv, tmpl);

      StrReplace(rv, zln, "%percent%", z_percent);
      StrReplace(rv, zln, "%comma%", z_comma);
      StrReplace(rv, zln, "%answer%", z_result);

      if (z_percent) {
        z_percent = NestInMath(z_percent);
        p_mr->PutResultComponent(z_percent, CPID_WHICH_QUANTILE, 0);
        delete[] z_percent;
      }
      delete[] z_comma;
      if (z_result) {
        z_result = NestInMath(z_result);
        p_mr->PutResultComponent(z_result, CPID_QUANTILE_RESULT, 0);
        delete[] z_result;
      }
    } else {
      TCI_ASSERT(0);
    }
  } else {
    TCI_ASSERT(0);
  }
  if (rv) {
    factor->zh_fstr = rv;
    factor->mml_nodes_last_term = 1;
    p_mr->PutResultCode(CR_quantile);
  } else {
    error_code = 1;
  }
}

void STree2MML::SemanticROOT2MML(SEMANTICS_NODE * snode,
                                 FACTOR_REC * factor, int& error_code)
{
  char *rv = NULL;

  BUCKET_REC *b_list = snode->bucket_list;
  BUCKET_REC *base_bucket = FindBucketRec(b_list, MB_ROOT_BASE);
  BUCKET_REC *radi_bucket = FindBucketRec(b_list, MB_ROOT_EXPONENT);

  int nodes_made, terms_made;
  bool is_signed = false;
  char *z_base = NULL;
  if (base_bucket) {
    z_base = ProcessSemanticList(base_bucket->first_child,
                                 error_code, nodes_made, terms_made,
                                 is_signed, NULL, NULL);
    if (z_base)
      if (terms_made > 1 || nodes_made > 1)
        z_base = NestzMMLInMROW(z_base);
  }

  char *z_radi = NULL;
  if (radi_bucket) {
    z_radi = ProcessSemanticList(radi_bucket->first_child,
                                 error_code, nodes_made, terms_made,
                                 is_signed, NULL, NULL);
    if (z_radi)
      if (terms_made > 1 || nodes_made > 1)
        z_radi = NestzMMLInMROW(z_radi);
  }

  char *tmpl = GetTmplPtr(TMPL_MROOT);
  size_t zln = strlen(tmpl);
  if (z_base)
    zln += strlen(z_base);
  if (z_radi)
    zln += strlen(z_radi);

  rv = new char[zln + 1];
  strcpy(rv, tmpl);

  StrReplace(rv, zln, "%base%", z_base);
  StrReplace(rv, zln, "%exp%", z_radi);

  delete[] z_base;
  delete[] z_radi;

  if (rv) {
    factor->zh_fstr = rv;
    factor->mml_nodes_last_term = 1;
  } else {
    error_code = 1;
  }
}

void STree2MML::SemanticMIXEDNUM2MML(SEMANTICS_NODE * snode,
                                     FACTOR_REC * factor, int& error_code)
{
  char *rv = NULL;

  BUCKET_REC *b_list = snode->bucket_list;
  BUCKET_REC *quo_bucket = FindBucketRec(b_list, MB_MN_WHOLE);
  BUCKET_REC *rem_bucket = FindBucketRec(b_list, MB_MN_FRACTION);

  int nodes_made, terms_made;
  bool is_signed = false;
  char *z_base = NULL;
  if (quo_bucket) {
    z_base = ProcessSemanticList(quo_bucket->first_child,
                                 error_code, nodes_made, terms_made,
                                 is_signed, NULL, NULL);
    if (z_base)
      if (terms_made > 1 || nodes_made > 1)
        z_base = NestzMMLInMROW(z_base);
  }
  char *z_radi = NULL;
  if (rem_bucket) {
    z_radi = ProcessSemanticList(rem_bucket->first_child,
                                 error_code, nodes_made, terms_made,
                                 is_signed, NULL, NULL);
    if (z_radi)
      if (terms_made > 1 || nodes_made > 1)
        z_radi = NestzMMLInMROW(z_radi);
  }
  char *body = NULL;
  U32 b_ln = 0;
  body = AppendStr2HeapStr(body, b_ln, z_base);
  body = AppendStr2HeapStr(body, b_ln, z_radi);

  char *tmpl = GetTmplPtr(TMPL_MROW);
  // "<mml:mrow>\n%body%</mml:mrow>\n",
  size_t zln = strlen(tmpl);
  if (body)
    zln += strlen(body);
  rv = new char[zln + 1];
  strcpy(rv, tmpl);

  StrReplace(rv, zln, "%body%", body);

  delete[] z_base;
  delete[] z_radi;
  delete[] body;

  if (rv) {
    factor->zh_fstr = rv;
    factor->mml_nodes_last_term = 1;
  } else {
    error_code = 1;
  }
}

void STree2MML::SemanticBIGOPSUM2MML(SEMANTICS_NODE * s_sum,
                                     FACTOR_REC * factor, int& error_code)
{
  char *zh_rv = NULL;
  error_code = 0;

  int nodes_made, terms_made;
  bool is_signed = false;
  char *zh_op = "&#x2211;";  // default to &sum;
  char *zh_sumand = NULL;
  char *zh_lowerlim = NULL;
  char *zh_upperlim = NULL;
  if (s_sum && s_sum->bucket_list) {
    BUCKET_REC *b_list = s_sum->bucket_list;
    BUCKET_REC *sumand_bucket = FindBucketRec(b_list, MB_OPERAND);
    BUCKET_REC *ll_bucket = FindBucketRec(b_list, MB_LOWERLIMIT);
    BUCKET_REC *ul_bucket = FindBucketRec(b_list, MB_UPPERLIMIT);
    BUCKET_REC *sv_bucket = FindBucketRec(b_list, MB_SUM_VAR);

    if (sumand_bucket && sumand_bucket->first_child) {
      zh_sumand = ProcessSemanticList(sumand_bucket->first_child,
                                      error_code, nodes_made, terms_made,
                                      is_signed, NULL, NULL);
      if (zh_sumand)
        if (terms_made > 1 || nodes_made > 1 || is_signed)
          zh_sumand = NestzMMLInMROW(zh_sumand);
    }
    if (ll_bucket && ll_bucket->first_child) {
      zh_lowerlim = ProcessSemanticList(ll_bucket->first_child,
                                        error_code, nodes_made, terms_made,
                                        is_signed, NULL, NULL);
      if (zh_lowerlim)
        if (terms_made > 1 || nodes_made > 1 || is_signed)
          zh_lowerlim = NestzMMLInMROW(zh_lowerlim);

      if (sv_bucket && sv_bucket->first_child) {
        char *zh_sumvar = ProcessSemanticList(sv_bucket->first_child,
                                              error_code, nodes_made,
                                              terms_made,
                                              is_signed, NULL, NULL);
        if (zh_sumvar) {
          if (terms_made > 1 || nodes_made > 1 || is_signed)
            zh_sumvar = NestzMMLInMROW(zh_sumvar);

          char *z_equal = MOfromSTRING("=", up_clr_math_attr);

          char *z_ll = NULL;
          U32 ll_ln = 0;
          z_ll = AppendStr2HeapStr(z_ll, ll_ln, zh_sumvar);
          z_ll = AppendStr2HeapStr(z_ll, ll_ln, z_equal);
          z_ll = AppendStr2HeapStr(z_ll, ll_ln, zh_lowerlim);
          delete[] zh_sumvar;
          delete[] z_equal;
          delete[] zh_lowerlim;
          zh_lowerlim = NestzMMLInMROW(z_ll);
        }
      }
    }
    if (ul_bucket && ul_bucket->first_child) {
      zh_upperlim = ProcessSemanticList(ul_bucket->first_child,
                                        error_code, nodes_made, terms_made,
                                        is_signed, NULL, NULL);
      if (zh_upperlim)
        if (terms_made > 1 || nodes_made > 1 || is_signed)
          zh_upperlim = NestzMMLInMROW(zh_upperlim);
    }
  } else {
    TCI_ASSERT(0);
  }
  if (s_sum && s_sum->contents)
    zh_op = s_sum->contents;
  char *tmpl = NULL;
  if (zh_lowerlim && zh_upperlim)
    tmpl = GetTmplPtr(TMPL_SUM2);
  else if (zh_lowerlim)
    tmpl = GetTmplPtr(TMPL_SUM1);
  else
    tmpl = GetTmplPtr(TMPL_SUM0);

  if (tmpl) {
    size_t ln = strlen(tmpl) + strlen(zh_op);
    if (zh_sumand)
      ln += strlen(zh_sumand);
    if (zh_lowerlim)
      ln += strlen(zh_lowerlim);
    if (zh_upperlim)
      ln += strlen(zh_upperlim);
    zh_rv = new char[ln + 1];
    strcpy(zh_rv, tmpl);

    StrReplace(zh_rv, ln, "%op%", zh_op);
    StrReplace(zh_rv, ln, "%sumand%", zh_sumand);
    StrReplace(zh_rv, ln, "%lowerlim%", zh_lowerlim);
    StrReplace(zh_rv, ln, "%upperlim%", zh_upperlim);
    zh_rv = ColorizeMMLElement(zh_rv, up_clr_math_attr, NULL, NULL);

    delete[] zh_sumand;
    delete[] zh_lowerlim;
    delete[] zh_upperlim;
  }

  if (zh_rv)
    factor->zh_fstr = zh_rv;
  else
    error_code = 1;
}


void STree2MML::SemanticTABULATION2MML(SEMANTICS_NODE * s_table,
                                       FACTOR_REC * factor, int& error_code)
{
  char *zh_rv = NULL;

  char *zh_rowlist = NULL;
  U32 rln = 0;

  int row_counter = 0;
  BUCKET_REC *cell_b_rover = s_table->bucket_list;
  while (row_counter < s_table->nrows) {
    char *zh_celllist = NULL;
    U32 cln = 0;

    int col_counter = 0;
    while (col_counter < s_table->ncols) {
      char *zh_cell_contents = NULL;
      if (cell_b_rover) {
        int nodes_made, terms_made;
        bool is_signed = false;
        zh_cell_contents = ProcessSemanticList(cell_b_rover->first_child,
                                               error_code, nodes_made,
                                               terms_made, is_signed, NULL,
                                               NULL);
      }
      char *mtd_tmpl = GetTmplPtr(TMPL_MTD);
      size_t zln = strlen(mtd_tmpl);
      if (zh_cell_contents)
        zln += strlen(zh_cell_contents);
      char *zh_mtd = new char[zln + 1];
      strcpy(zh_mtd, mtd_tmpl);
      StrReplace(zh_mtd, zln, "%body%", zh_cell_contents);
      delete[] zh_cell_contents;
      zh_celllist = AppendStr2HeapStr(zh_celllist, cln, zh_mtd);
      delete[] zh_mtd;

      if (cell_b_rover)
        cell_b_rover = cell_b_rover->next;
      col_counter++;
    }

    char *mtr_tmpl = GetTmplPtr(TMPL_MTR);
    size_t zln = strlen(mtr_tmpl);
    if (zh_celllist)
      zln += strlen(zh_celllist);
    char *zh_mtr = new char[zln + 1];
    strcpy(zh_mtr, mtr_tmpl);
    StrReplace(zh_mtr, zln, "%celllist%", zh_celllist);
    zh_rowlist = AppendStr2HeapStr(zh_rowlist, rln, zh_mtr);
    delete[] zh_celllist;
    delete[] zh_mtr;
    row_counter++;
  }

  char *mtable_tmpl = GetTmplPtr(TMPL_MTABLE);
  size_t zln = strlen(mtable_tmpl);
  if (zh_rowlist)
    zln += strlen(zh_rowlist);
  zh_rv = new char[zln + 1];
  strcpy(zh_rv, mtable_tmpl);
  StrReplace(zh_rv, zln, "%rowlist%", zh_rowlist);
  delete[] zh_rowlist;

  // It remains to delimit the output matrix
  DelimsIlk delims = GetNotationVariant(SEM_TYP_TABULATION);
  if (delims != Delims_none) {
    if (up_use_mfenced) {
      char *z_left_delim = "[";
      char *z_right_delim = "]";
      if (delims == Delims_brackets) {
        z_left_delim = "[";
        z_right_delim = "]";
      } else if (delims == Delims_parens) {
        z_left_delim = "(";
        z_right_delim = ")";
      } else if (delims == Delims_braces) {
        z_left_delim = "{";
        z_right_delim = "}";
      }

      char *zh_interior = zh_rv;

      char *tmpl = GetTmplPtr(TMPL_MFENCED);
      size_t zln = strlen(tmpl);
      if (z_left_delim)
        zln += strlen(z_left_delim);
      if (z_right_delim)
        zln += strlen(z_right_delim);
      if (zh_interior)
        zln += strlen(zh_interior);
      zh_rv = new char[zln + 1];
      strcpy(zh_rv, tmpl);
      StrReplace(zh_rv, zln, "%open%", z_left_delim);
      StrReplace(zh_rv, zln, "%close%", z_right_delim);
      StrReplace(zh_rv, zln, "%body%", zh_interior);

      delete[] zh_interior;

      factor->mml_nodes_last_term = 1;
    } else {
      int delim_tmpl_ID = TMPL_BRACKETS;
      if (delims == Delims_brackets)
        delim_tmpl_ID = TMPL_BRACKETS;
      else if (delims == Delims_parens)
        delim_tmpl_ID = TMPL_PARENS;
      else if (delims == Delims_braces)
        delim_tmpl_ID = TMPL_BRACES;

      if (delim_tmpl_ID) {
        char *delim_tmpl = GetTmplPtr(delim_tmpl_ID);
        size_t zln = strlen(delim_tmpl);
        if (zh_rv)
          zln += strlen(zh_rv);
        char *zh_delimited_rv = new char[zln + 1];
        strcpy(zh_delimited_rv, delim_tmpl);
        StrReplace(zh_delimited_rv, zln, "%interior%", zh_rv);
        zh_delimited_rv = ColorizeMMLElement(zh_delimited_rv,
                                             up_clr_math_attr,
                                             up_clr_math_attr, NULL);
        delete[] zh_rv;
        zh_rv = zh_delimited_rv;
        factor->mml_nodes_last_term = 3;
      } else {
        TCI_ASSERT(0);
      }
    }
  }

  if (zh_rv) {
    factor->zh_fstr = zh_rv;
  } else {
    error_code = 1;
  }
}

//  SEM_TYP_MTRANSPOSE,
//  SEM_TYP_HTRANSPOSE,
//     SEM_TYP_MTRANSPOSE MB_UNNAMED {TABULATION}

void STree2MML::SemanticTRANSPOSE2MML(SEMANTICS_NODE * s_transpose,
                                    FACTOR_REC * factor, int& error_code)
{
  const char *transpose = s_transpose->semantic_type == SEM_TYP_HTRANSPOSE ? "H" : "T";
  char *zh_transpose = MIfromCHDATA(transpose);
  int nodes_made, terms_made;
  bool is_signed = false;
  char *zh_matrix = ProcessSemanticList(s_transpose->bucket_list->first_child,
                                        error_code, nodes_made,
                                        terms_made, is_signed, NULL,
                                        NULL);
  char *transpose_tmpl = GetTmplPtr(TMPL_TRANSPOSE);
  size_t zln = strlen(transpose_tmpl) + strlen(zh_matrix) + strlen(zh_transpose);
  char *zh_rv = new char[zln + 1];
  strcpy(zh_rv, transpose_tmpl);
  StrReplace(zh_rv, zln, "%MatrixBody%", zh_matrix);
  StrReplace(zh_rv, zln, "%transpose%", zh_transpose);
  delete[] zh_matrix;
  delete[] zh_transpose;

  if (zh_rv) {
    factor->zh_fstr = zh_rv;
  } else {
    error_code = 1;
  }
}

  // SEM_TYPE_SIMPLEX
//     MB_SIMPLEX_EXPR MB_UNNAMED {MB_UNNAMED}

// SWP stores the components of a simplex in a nx1 matric

void STree2MML::SemanticSIMPLEX2MML(SEMANTICS_NODE * s_simplex,
                                    FACTOR_REC * factor, int& error_code)
{
  char *zh_rv = NULL;

  char *zh_objective_func = NULL;
  char *zh_condition_list = NULL;
  U32 cln = 0;

  BUCKET_REC *b_rover = s_simplex->bucket_list;
  while (b_rover) {             // loop thru simplex buckets
    if (b_rover->first_child) {
      SEMANTICS_NODE *s_line = b_rover->first_child;
      int nodes_made, terms_made;
      bool is_signed = false;
      char *zh_line_contents = ProcessSemanticList(s_line,
                                                   error_code, nodes_made,
                                                   terms_made,
                                                   is_signed, NULL, NULL);
      // Nest the line contents in an <mtd>
      char *mtd_tmpl = GetTmplPtr(TMPL_MTD);
      size_t zln = strlen(mtd_tmpl);
      if (zh_line_contents)
        zln += strlen(zh_line_contents);
      char *zh_mtd = new char[zln + 1];
      strcpy(zh_mtd, mtd_tmpl);
      StrReplace(zh_mtd, zln, "%body%", zh_line_contents);
      delete[] zh_line_contents;

      // Nest the <mtd> in an <mtr>
      char *mtr_tmpl = GetTmplPtr(TMPL_MTR);
      zln = strlen(mtr_tmpl + 1);
      if (zh_mtd)
        zln += strlen(zh_mtd);
      char *zh_mtr = new char[zln + 1];
      strcpy(zh_mtr, mtr_tmpl);
      StrReplace(zh_mtr, zln, "%celllist%", zh_mtd);

      delete[] zh_mtd;

      if (b_rover->bucket_ID == MB_SIMPLEX_EXPR) {
        zh_objective_func = zh_mtr;
      } else {
        zh_condition_list = AppendStr2HeapStr(zh_condition_list, cln, zh_mtr);
        delete[] zh_mtr;
      }
    } else {
      TCI_ASSERT(0);
    }
    b_rover = b_rover->next;
  }                             // loop thru simplex buckets

  U32 rows_zln = 0;
  if (zh_objective_func)
    rows_zln += strlen(zh_objective_func);
  if (zh_condition_list)
    rows_zln += strlen(zh_condition_list);

  char *zh_rowlist = new char[rows_zln + 1];
  zh_rowlist[0] = 0;

  if (zh_objective_func) {
    strcpy(zh_rowlist, zh_objective_func);
    delete[] zh_objective_func;
  }
  if (zh_condition_list) {
    strcat(zh_rowlist, zh_condition_list);
    delete[] zh_condition_list;
  }
  // Nest zh_rowlist in an <mtable>

  char *mtable_tmpl = GetTmplPtr(TMPL_MTABLE);
  size_t zln = strlen(mtable_tmpl + 1);
  zln += strlen(zh_rowlist);
  zh_rv = new char[zln + 1];
  strcpy(zh_rv, mtable_tmpl);
  StrReplace(zh_rv, zln, "%rowlist%", zh_rowlist);
  delete[] zh_rowlist;

  if (zh_rv) {
    factor->zh_fstr = zh_rv;
    factor->mml_nodes_last_term = 1;
  } else {
    error_code = 1;
  }
}

// Some "operations" like "floor", "ceiling", "absolute value", etc.
//  are marked up as fences.

void STree2MML::SemanticFENCEOP2MML(SEMANTICS_NODE * s_fence,
                                    FACTOR_REC * factor, int& error_code)
{
  char *zh_rv = NULL;
  BUCKET_REC *fenced_bucket = s_fence->bucket_list;
  char *zh_contents = NULL;
  if (fenced_bucket) {
    int nodes_made, terms_made;
    bool is_signed = false;
    zh_contents = ProcessSemanticList(fenced_bucket->first_child,
                                      error_code, nodes_made, terms_made,
                                      is_signed, NULL, NULL);
    if (zh_contents)
      if (nodes_made > 1 || terms_made > 1 || is_signed)
        zh_contents = NestzMMLInMROW(zh_contents);
  }

  char *z_left_delim = NULL;
  char *z_right_delim = NULL;

  switch (s_fence->semantic_type) {
  case SEM_TYP_ABS:
    z_left_delim = "|";
    z_right_delim = "|";
    break;
  case SEM_TYP_FLOOR:
    z_left_delim = "&#x230a;";
    z_right_delim = "&#x230b;";
    break;
  case SEM_TYP_CEILING:
    z_left_delim = "&#x2308;";
    z_right_delim = "&#x2309;";
    break;

  default:
    TCI_ASSERT(!"more here later");
    break;
  }

  if (up_use_mfenced) {
    char *tmpl = GetTmplPtr(TMPL_MFENCED);
    size_t zln = strlen(tmpl) + strlen(z_left_delim) + strlen(z_right_delim);
    if (zh_contents)
      zln += strlen(zh_contents);
    zh_rv = new char[zln + 1];
    strcpy(zh_rv, tmpl);
    StrReplace(zh_rv, zln, "%open%", z_left_delim);
    StrReplace(zh_rv, zln, "%close%", z_right_delim);
    StrReplace(zh_rv, zln, "%body%", zh_contents);

    delete[] zh_contents;
  } else {
    char *tmpl = GetTmplPtr(TMPL_FENCEasOP);

    char *zmo_left = MOfromSTRING(z_left_delim, up_clr_math_attr);
    char *zmo_right = MOfromSTRING(z_right_delim, up_clr_math_attr);

    size_t zln = strlen(tmpl) + strlen(zmo_left) + strlen(zmo_right);
    if (zh_contents)
      zln += strlen(zh_contents);
    zh_rv = new char[zln + 1];
    strcpy(zh_rv, tmpl);
    StrReplace(zh_rv, zln, "%mo_left%", zmo_left);
    StrReplace(zh_rv, zln, "%body%", zh_contents);
    StrReplace(zh_rv, zln, "%mo_right%", zmo_right);

    delete[] zmo_left;
    delete[] zmo_right;
    delete[] zh_contents;
  }
  if (zh_rv) {
    factor->zh_fstr = zh_rv;
    factor->mml_nodes_last_term = 1;
  } else {
    error_code = 1;
  }
}

void STree2MML::SemanticNORM2MML(SEMANTICS_NODE * s_norm,
                                    FACTOR_REC * factor, int& error_code)
{
  char *zh_rv = NULL;
  BUCKET_REC *norm_bucket = s_norm->bucket_list;
  char *zh_contents = NULL;
  if (norm_bucket) {
    int nodes_made, terms_made;
    bool is_signed = false;
    zh_contents = ProcessSemanticList(norm_bucket->first_child,
                                      error_code, nodes_made, terms_made,
                                      is_signed, NULL, NULL);
    if (zh_contents)
      if (nodes_made > 1 || terms_made > 1 || is_signed)
        zh_contents = NestzMMLInMROW(zh_contents);
  }

  char *z_left_delim = "&#x2016;";
  char *z_right_delim = "&#x2016;";

  if (up_use_mfenced) {
    char *tmpl = GetTmplPtr(TMPL_MFENCED);
    size_t zln = strlen(tmpl) + strlen(z_left_delim) + strlen(z_right_delim);
    if (zh_contents)
      zln += strlen(zh_contents);
    zh_rv = new char[zln + 1];
    strcpy(zh_rv, tmpl);
    StrReplace(zh_rv, zln, "%open%", z_left_delim);
    StrReplace(zh_rv, zln, "%close%", z_right_delim);
    StrReplace(zh_rv, zln, "%body%", zh_contents);

    delete[] zh_contents;
  } else {
    char *tmpl = GetTmplPtr(TMPL_FENCEasOP);

    char *zmo_left = MOfromSTRING(z_left_delim, up_clr_math_attr);
    char *zmo_right = MOfromSTRING(z_right_delim, up_clr_math_attr);

    size_t zln = strlen(tmpl) + strlen(zmo_left) + strlen(zmo_right);
    if (zh_contents)
      zln += strlen(zh_contents);
    zh_rv = new char[zln + 1];
    strcpy(zh_rv, tmpl);
    StrReplace(zh_rv, zln, "%mo_left%", zmo_left);
    StrReplace(zh_rv, zln, "%body%", zh_contents);
    StrReplace(zh_rv, zln, "%mo_right%", zmo_right);

    delete[] zmo_left;
    delete[] zmo_right;
    delete[] zh_contents;
  }

  // put on the subscript
  BUCKET_REC *num_bucket = FindBucketRec(norm_bucket, MB_NORM_NUMBER);
  if (num_bucket && num_bucket->first_child) {
    zh_rv = AddSubToVariable(num_bucket->first_child,zh_rv);
  }

  if (zh_rv) {
    factor->zh_fstr = zh_rv;
    factor->mml_nodes_last_term = 1;
  } else {
    error_code = 1;
  }
}

char *STree2MML::NestzMMLInMROW(char *z_body)
{
  char *zh_rv = NULL;

  if (z_body) {
    char *tmpl = GetTmplPtr(TMPL_MROW);
    size_t zln = strlen(tmpl) + strlen(z_body);
    zh_rv = new char[zln + 1];
    strcpy(zh_rv, tmpl);
    StrReplace(zh_rv, zln, "%body%", z_body);
    delete[] z_body;
  }

  return zh_rv;
}

bool STree2MML::ScriptIsOneOverN(BUCKET_REC * script_upper_bucket,
                                     bool & is_sqrt)
{
  bool rv = false;
  is_sqrt = false;

  bool got1 = false;
  bool got2 = false;

  if (script_upper_bucket && script_upper_bucket->first_child) {
    SEMANTICS_NODE *script_cont = script_upper_bucket->first_child;
    SEMANTICS_NODE *fraction = NULL;

    if (script_cont->semantic_type == SEM_TYP_PARENED_LIST) {
      BUCKET_REC *b_list = script_cont->bucket_list;
      if (b_list && b_list->first_child)
        fraction = b_list->first_child;
    } else {
      fraction = script_cont;
    }
    if (fraction
        && fraction->semantic_type == SEM_TYP_FRACTION
        && !fraction->next) {

      BUCKET_REC *b_list = fraction->bucket_list;
      BUCKET_REC *num_bucket = FindBucketRec(b_list, MB_NUMERATOR);
      BUCKET_REC *den_bucket = FindBucketRec(b_list, MB_DENOMINATOR);

      if (num_bucket && num_bucket->first_child) {
        SEMANTICS_NODE *numerator = num_bucket->first_child;
        if (numerator->semantic_type == SEM_TYP_NUMBER) {
          if (!strcmp(numerator->contents, "1"))
            got1 = true;
        }
      }
      if (den_bucket && den_bucket->first_child) {
        SEMANTICS_NODE *denominator = den_bucket->first_child;
        if (denominator->semantic_type == SEM_TYP_NUMBER) {
          if (!strcmp(denominator->contents, "2"))
            got2 = true;
        }
      }
    }
  }

  if (got1) {
    rv = true;
    if (got2)
      is_sqrt = true;
  }

  return rv;
}

void STree2MML::SemanticEIGENVECTOR2MML(SEMANTICS_NODE * s_evector_set,
                                        FACTOR_REC * factor, int& error_code)
{
  char *zh_rv = NULL;

  char *vec_list = NULL;
  U32 vl_ln = 0;

  char *vec_tmpl = GetTmplPtr(TMPL_EIGENVECTOR);
  // <mfenced open=\"\" close=\"\">\n
  // <mrow class= \"msiEigenValue\">\n%value%</mrow>\n
  // <mrow class= \"msiMultiplicity\">\n%mult%</mrow>\n
  // <mrow class= \"msiEigenVector\">\n%vector%</mrow>\n
  // </mfenced>\n

  int index = 1;

  BUCKET_REC *b_rover = s_evector_set->bucket_list;
  while (b_rover) {
    if (b_rover->first_child) {
      SEMANTICS_NODE *s_evector = b_rover->first_child;

      if (s_evector) {
        int nodes_made, terms_made;
        bool is_signed = false;
        char *z_value = NULL;
        char *z_mult = NULL;
        char *z_vector = NULL;

        BUCKET_REC *b_list = s_evector->bucket_list;
        BUCKET_REC *value_bucket = FindBucketRec(b_list, MB_EIGENVALUE);
        BUCKET_REC *mult_bucket = FindBucketRec(b_list, MB_EV_MULTIPLICITY);
        BUCKET_REC *vector_bucket = FindBucketRec(b_list, MB_EIGENVECTOR);

        if (value_bucket && value_bucket->first_child) {
          z_value = ProcessSemanticList(value_bucket->first_child,
                                        error_code, nodes_made, terms_made,
                                        is_signed, NULL, NULL);
          if (z_value && nodes_made > 1)
            z_value = NestzMMLInMROW(z_value);
        }
        if (mult_bucket && mult_bucket->first_child) {
          z_mult = ProcessSemanticList(mult_bucket->first_child,
                                       error_code, nodes_made, terms_made,
                                       is_signed, NULL, NULL);
          if (z_mult && nodes_made > 1)
            z_mult = NestzMMLInMROW(z_mult);
        }
        if (vector_bucket && vector_bucket->first_child) {
          z_vector = ProcessSemanticList(vector_bucket->first_child,
                                         error_code, nodes_made, terms_made,
                                         is_signed, NULL, NULL);
          if (z_vector && nodes_made > 1)
            z_vector = NestzMMLInMROW(z_vector);
        }
        if (vec_tmpl) {
          size_t ln = strlen(vec_tmpl);
          if (z_value)
            ln += strlen(z_value);
          if (z_mult)
            ln += strlen(z_mult);
          if (z_vector)
            ln += strlen(z_vector);
          char *z_triple = new char[ln + 1];
          strcpy(z_triple, vec_tmpl);

          StrReplace(z_triple, ln, "%value%", z_value);
          StrReplace(z_triple, ln, "%mult%", z_mult);
          StrReplace(z_triple, ln, "%vector%", z_vector);

          if (z_value) {
            z_value = NestInMath(z_value);
            p_mr->PutResultComponent(z_value, CPID_EIGENVALUE, index);
            delete[] z_value;
          }
          if (z_mult) {
            z_mult = NestInMath(z_mult);
            p_mr->PutResultComponent(z_mult, CPID_EVAL_MULTIPLICITY, index);
            delete[] z_mult;
          }
          if (z_vector) {
            z_vector = NestInMath(z_vector);
            p_mr->PutResultComponent(z_vector, CPID_EIGENVECTOR, index);
            delete[] z_vector;
          }

          vec_list = AppendStr2HeapStr(vec_list, vl_ln, z_triple);
          delete[] z_triple;
        }
      } else {
        TCI_ASSERT(0);
      }
    }
    index++;
    b_rover = b_rover->next;
  }

  char *set_tmpl = GetTmplPtr(TMPL_EIGENVECTORSET);
  //  "<mfenced class= \"msiEigenVectorSet\" open=\"{\" close=\"}\">\n%eigen_vector_list%</mfenced>\n",

  if (set_tmpl) {
    size_t ln = strlen(set_tmpl);
    if (vec_list)
      ln += strlen(vec_list);

    zh_rv = new char[ln + 1];
    strcpy(zh_rv, set_tmpl);
    StrReplace(zh_rv, ln, "%eigen_vector_list%", vec_list);

    delete[] vec_list;
  }

  if (zh_rv) {
    factor->zh_fstr = zh_rv;
    p_mr->PutResultCode(CR_eigenvectors);
  } else {
    error_code = 1;
  }
}

void STree2MML::SemanticINTEGRAL2MML(SEMANTICS_NODE * s_integral,
                                     FACTOR_REC * factor, int& error_code)
{
  char *zh_rv = NULL;

  int nodes_made, terms_made;
  bool is_signed = false;
  char *zh_integrand = NULL;
  char *zh_lowerlim = NULL;
  char *zh_upperlim = NULL;
  char *zh_intvar = NULL;
  if (s_integral && s_integral->bucket_list) {
    BUCKET_REC *b_list = s_integral->bucket_list;
    BUCKET_REC *integrand_bucket = FindBucketRec(b_list, MB_OPERAND);
    BUCKET_REC *ll_bucket = FindBucketRec(b_list, MB_LOWERLIMIT);
    BUCKET_REC *ul_bucket = FindBucketRec(b_list, MB_UPPERLIMIT);
    BUCKET_REC *iv_bucket = FindBucketRec(b_list, MB_INTEG_VAR);

    if (integrand_bucket && integrand_bucket->first_child) {
      zh_integrand = ProcessSemanticList(integrand_bucket->first_child,
                                         error_code, nodes_made, terms_made,
                                         is_signed, NULL, NULL);
      if (zh_integrand && nodes_made > 1)
        zh_integrand = NestzMMLInMROW(zh_integrand);
    }
    if (ll_bucket && ll_bucket->first_child) {
      zh_lowerlim = ProcessSemanticList(ll_bucket->first_child,
                                        error_code, nodes_made, terms_made,
                                        is_signed, NULL, NULL);
      if (zh_lowerlim)
        if (nodes_made > 1 || terms_made > 1 || is_signed)
          zh_lowerlim = NestzMMLInMROW(zh_lowerlim);
    }
    if (ul_bucket && ul_bucket->first_child) {
      zh_upperlim = ProcessSemanticList(ul_bucket->first_child,
                                        error_code, nodes_made, terms_made,
                                        is_signed, NULL, NULL);
      if (zh_upperlim)
        if (nodes_made > 1 || terms_made > 1 || is_signed)
          zh_upperlim = NestzMMLInMROW(zh_upperlim);
    }
    if (iv_bucket && iv_bucket->first_child) {
      zh_intvar = ProcessSemanticList(iv_bucket->first_child,
                                      error_code, nodes_made, terms_made,
                                      is_signed, NULL, NULL);
      if (zh_intvar && nodes_made > 1)
        zh_intvar = NestzMMLInMROW(zh_intvar);
    }
  } else {
    TCI_ASSERT(0);
  }
  char *tmpl = NULL;
  if (zh_lowerlim && zh_upperlim)
    tmpl = GetTmplPtr(TMPL_INTEGRAL2);
  else if (zh_lowerlim)
    tmpl = GetTmplPtr(TMPL_INTEGRAL1);
  else
    tmpl = GetTmplPtr(TMPL_INTEGRAL0);

  if (tmpl) {
    char *zh_diffop = PrefixMOfromSTRING(up_differentiald);

    size_t ln = strlen(tmpl);
    if (zh_integrand)
      ln += strlen(zh_integrand);
    if (zh_lowerlim)
      ln += strlen(zh_lowerlim);
    if (zh_upperlim)
      ln += strlen(zh_upperlim);
    if (zh_diffop)
      ln += strlen(zh_diffop);
    if (zh_intvar)
      ln += strlen(zh_intvar);
    zh_rv = new char[ln + 1];
    strcpy(zh_rv, tmpl);

    StrReplace(zh_rv, ln, "%integrand%", zh_integrand);
    StrReplace(zh_rv, ln, "%lowerlim%", zh_lowerlim);
    StrReplace(zh_rv, ln, "%upperlim%", zh_upperlim);
    StrReplace(zh_rv, ln, "%diffop%", zh_diffop);
    StrReplace(zh_rv, ln, "%intvar%", zh_intvar);
    zh_rv =
      ColorizeMMLElement(zh_rv, up_clr_math_attr, up_clr_math_attr, NULL);

    delete[] zh_integrand;
    delete[] zh_lowerlim;
    delete[] zh_upperlim;
    delete[] zh_diffop;
    delete[] zh_intvar;
  }

  if (zh_rv)
    factor->zh_fstr = zh_rv;
  else
    error_code = 1;
}

char *STree2MML::NestzMMLInPARENS(char *zh_arg, int & nodes_made)
{
  char *zh_rv = NULL;

  if (up_use_mfenced) {
    char *tmpl = GetTmplPtr(TMPL_MFENCED);
    size_t zln = strlen(tmpl) + 1 + 1;
    if (zh_arg)
      zln += strlen(zh_arg);
    zh_rv = new char[zln + 1];
    strcpy(zh_rv, tmpl);
    StrReplace(zh_rv, zln, "%open%", "(");
    StrReplace(zh_rv, zln, "%close%", ")");
    StrReplace(zh_rv, zln, "%body%", zh_arg);
    nodes_made = 1;
  } else {
    char *tmpl = GetTmplPtr(TMPL_PARENS);
    size_t ln = strlen(tmpl);
    if (zh_arg)
      ln += strlen(zh_arg);
    zh_rv = new char[ln + 1];
    strcpy(zh_rv, tmpl);
    StrReplace(zh_rv, ln, "%interior%", zh_arg);
    zh_rv =
      ColorizeMMLElement(zh_rv, up_clr_math_attr, up_clr_math_attr, NULL);
    nodes_made = 3;
  }
  delete[] zh_arg;

  return zh_rv;
}

char *STree2MML::GetTmplPtr(int tmpl_ID)
{
  char *rv = NULL;

  if (tmpl_ID >= TMPL_MATH && tmpl_ID <= TMPL_LAST) {
    if (output_markup == OUTFORMAT_MML)
      rv = MML1templates[tmpl_ID];
    else if (output_markup == OUTFORMAT_LATEX)
      TCI_ASSERT(0);            //rv  =  LaTeXtemplates[tmpl_ID];
    else
      TCI_ASSERT(0);
  } else {
    TCI_ASSERT(0);
  }
  return rv;
}

// The following is only a crude beginning at an algorithm
//  to select notations for output.  Most semantic objects
//  have one common notation - no problem. However, some
//  have more.  Examples
//    \sin^{-1}     and   \arcsin
//    x^{\frac12}   and   \sqrt{x}
//    multiple notations for derivatives
//    complex conjugates x^{*}  and  \overbar{x}
//    floats can have more or digits after the decimal
//      - scientific notation is an alternative
//    the delimiters around matrices may vary.
//    various patterns may be used to order
//      the terms in a polynomial.
//    etc.
//
//   Things to consider in deciding output notation:
//   1. input notation
//   2. stated user preferences (a notation sheet?)
//   3. history - the notation seen in recent
//                inputs from use client.

//SLS Currently, this function only deals with fence delimiters and needs
// much work if it is to be extended.

STree2MML::DelimsIlk STree2MML::GetNotationVariant(SemanticType sem_type)
{
  DelimsIlk rv = Delims_none;

  switch (sem_type) {
  case SEM_TYP_FRACTION:
    break;
  case SEM_TYP_POWERFORM:
    break;
  case SEM_TYP_SQRT:
    break;
  case SEM_TYP_ROOT:
    break;
  case SEM_TYP_BIGOP_SUM:
    break;
  case SEM_TYP_BIGOP_INTEGRAL:
    break;

  case SEM_TYP_TABULATION:
    if (input_notation) {
      if (input_notation->nbracket_tables) {
        rv = Delims_brackets;
        last_matrix_delim_type = Delims_brackets;
      } else if (input_notation->nparen_tables) {
        rv = Delims_parens;
        last_matrix_delim_type = Delims_parens;
      } else if (input_notation->nbrace_tables) {
        rv = Delims_braces;
        last_matrix_delim_type = Delims_braces;
      } else
        rv = up_matrix_delims;
    } else {                    // use historic info
      rv = up_matrix_delims;
    }
    break;

  case SEM_TYP_LIMFUNC:
  case SEM_TYP_BINOMIAL:
  case SEM_TYP_SET:
  case SEM_TYP_ABS:
  case SEM_TYP_FLOOR:
  case SEM_TYP_CEILING:
  case SEM_TYP_CONJUGATE:
  case SEM_TYP_SIMPLEX:
    break;

  case SEM_TYP_EIGENVECTOR:
    break;

  default:
    TCI_ASSERT(0);
    break;
  }

  return rv;
}

bool STree2MML::IsExposedNumber(SEMANTICS_NODE * s_node)
{
  bool rv = false;

  // Descend to the left-most leaf
  SEMANTICS_NODE *s_parent = NULL;
  int n_ancestors = 0;
  SEMANTICS_NODE *s_rover = s_node;
  while (s_rover && s_rover->bucket_list) {
    s_parent = s_rover;
    n_ancestors++;
    BUCKET_REC *base_bucket = FindBucketRec(s_rover->bucket_list,
                                            MB_SCRIPT_BASE);
    if (!base_bucket)
      base_bucket = s_rover->bucket_list;

    s_rover = base_bucket->first_child;
  }
  if (s_rover) {
    if (s_rover->semantic_type == SEM_TYP_NUMBER) {
      rv = true;
      if (s_parent) {
        if (n_ancestors > 1)
          rv = false;
        else if (s_parent->semantic_type == SEM_TYP_SQRT
                || s_parent->semantic_type == SEM_TYP_FUNCTION)
          rv = false;
        else if (s_parent->semantic_type == SEM_TYP_POWERFORM) {
          BUCKET_REC *sup_bucket = FindBucketRec(s_parent->bucket_list,
                                                MB_SCRIPT_UPPER);
          bool is_sqrt;
          if (sup_bucket && ScriptIsOneOverN(sup_bucket, is_sqrt))
            rv = false;
        }
      }
    } else if (s_rover->semantic_type == SEM_TYP_PREFIX_OP  && !strcmp(s_rover->contents,"&#x2212;")) {
      rv = IsExposedNumber(s_rover->next);
    }
  }
  return rv;
}

// just like the above without the check for the &minus;
bool STree2MML::IsExposedPositiveNumber(SEMANTICS_NODE * s_node)
{
  bool rv = false;

  // Descend to the left-most leaf
  SEMANTICS_NODE *s_parent = NULL;
  int n_ancestors = 0;
  SEMANTICS_NODE *s_rover = s_node;
  while (s_rover && s_rover->bucket_list) {
    s_parent = s_rover;
    n_ancestors++;
    BUCKET_REC *base_bucket = FindBucketRec(s_rover->bucket_list,
                                            MB_SCRIPT_BASE);
    if (!base_bucket)
      base_bucket = s_rover->bucket_list;

    s_rover = base_bucket->first_child;
  }
  if (s_rover) {
    if (s_rover->semantic_type == SEM_TYP_NUMBER) {
      rv = true;
      if (s_parent) {
        if (n_ancestors > 1)
          rv = false;
        else if (s_parent->semantic_type == SEM_TYP_SQRT
                || s_parent->semantic_type == SEM_TYP_FUNCTION
                || s_parent->semantic_type == SEM_TYP_VARIABLE)
          rv = false;
        else if (s_parent->semantic_type == SEM_TYP_POWERFORM) {
          BUCKET_REC *sup_bucket = FindBucketRec(s_parent->bucket_list,
                                                MB_SCRIPT_UPPER);
          bool is_sqrt;
          if (sup_bucket && ScriptIsOneOverN(sup_bucket, is_sqrt))
            rv = false;
        }
      }
    }
  }
  return rv;
}

bool STree2MML::IsUnit(SEMANTICS_NODE * s_node)
{
  bool rv = false;

  // Descend to the left-most leaf
  SEMANTICS_NODE *s_parent = NULL;
  int n_ancestors = 0;
  SEMANTICS_NODE *s_rover = s_node;
  while (s_rover->bucket_list) {
    if (s_rover->semantic_type != SEM_TYP_PRECEDENCE_GROUP)
      n_ancestors++;
    s_parent = s_rover;
    BUCKET_REC *base_bucket = FindBucketRec(s_rover->bucket_list,
                                            MB_SCRIPT_BASE);
    if (!base_bucket)
      base_bucket = s_rover->bucket_list;
    s_rover = base_bucket->first_child;
  }
  if (s_rover) {
    if (s_rover->semantic_type == SEM_TYP_SIUNIT
        || s_rover->semantic_type == SEM_TYP_USUNIT) {
      rv = true;
      if (s_parent) {
        if (n_ancestors > 1)
          rv = false;
        else if (s_parent->semantic_type == SEM_TYP_SQRT
                 || s_parent->semantic_type == SEM_TYP_FUNCTION)
          rv = false;
        else if (s_parent->semantic_type == SEM_TYP_POWERFORM) {
          BUCKET_REC *sup_bucket = FindBucketRec(s_parent->bucket_list,
                                                 MB_SCRIPT_UPPER);
          bool is_sqrt;
          if (sup_bucket && ScriptIsOneOverN(sup_bucket, is_sqrt))
            rv = false;
        }
      }
    }
  }

  return rv;
}

bool STree2MML::IsSmallNaturalNumber(SEMANTICS_NODE * s_node, U32 & num)
{
  bool rv = false;

  U32 val = 0;
  if (s_node
      && s_node->next == NULL
      && s_node->semantic_type == SEM_TYP_NUMBER) {
    rv = true;
    char *ptr = s_node->contents;
    char ch;
    int n_digits = 0;
    while (ch = *ptr) {
      if ((ch < '0') || (ch > '9')) {
        rv = false;
        break;
      } else if (n_digits == 9) {
        rv = false;
        break;
      } else {
        val = 10 * val + ch - '0';
        n_digits++;
        ptr++;
      }
    }
  }

  num = val;
  return rv;
}

//<mml:mrow>\n
//  <mml:mn>%whole%</mml:mn>\n
//  <mml:mfrac>\n
//    <mml:mn>%numerator%</mml:mn>\n
//    <mml:mn>%denominator%</mml:mn>\n
//  </mml:mfrac>\n
//</mml:mrow>\n

void STree2MML::SmallMixedNumber2MML(U32 num, U32 denom, FACTOR_REC * factor)
{
  char *zh_rv = NULL;

  U32 whole = num / denom;
  num = num % denom;

  char *tmpl = GetTmplPtr(TMPL_MIXEDNUMBER);
  size_t ln = strlen(tmpl) + 64;

  zh_rv = new char[ln];
  strcpy(zh_rv, tmpl);

  char buffer[64];
  sprintf(buffer, "%lu", whole);
  StrReplace(zh_rv, ln, "%whole%", buffer);
  sprintf(buffer, "%lu", num);
  StrReplace(zh_rv, ln, "%numerator%", buffer);
  sprintf(buffer, "%lu", denom);
  StrReplace(zh_rv, ln, "%denominator%", buffer);
  zh_rv =
    ColorizeMMLElement(zh_rv, up_clr_math_attr, up_clr_math_attr,
                       up_clr_math_attr);

  factor->zh_fstr = zh_rv;
  factor->mml_nodes_last_term = 1;
  factor->is_number = true;
}

char *STree2MML::FuncHeader2MML(SEMANTICS_NODE * s_function)
{
  char *zh_rv = NULL;

  bool do_supneg1_inverse = false;
  if (s_function->contents) {
    int n_entities;
    int n_symbols = CountSymbols(s_function->contents, n_entities);
    char *tmpl = GetTmplPtr(TMPL_MI); // "<mml:mi>%letters%</mml:mi>\n",
    if (n_symbols == 1) {
      // Some single symbol function names are generally renderer upright
      //   Gamma is an example.
      char *ptr = strstr(s_function->contents, "&#");
      if (ptr) {
        U32 unicode = NumericEntity2U32(ptr);
        if (unicode >= 0x0380)
          tmpl = GetTmplPtr(TMPL_UPRIGHTMI);  // "<mi%color_attr% fontstyle=\"upright\">%letters%</mi>\n",
      }
    }
    // If the user has specified that "log" means "log base e" or natural log,
    //  we might want to substitute "log" for "ln" here.

    char *f_nom = s_function->contents;
    if (!strcmp(f_nom, "ln") && up_log_is_base_e && input_notation->n_logs) {
      f_nom = "log";
    }
    //need to do inverse as -1 in exponent?
    do_supneg1_inverse =
      (s_function->semantic_type == SEM_TYP_INVTRANSFORM || s_function->semantic_type == SEM_TYP_INVFUNCTION);
    if (up_InvTrigFuncs_1) {
      if (!strncmp(f_nom, "arc", 3)
          && IsTrigArgFunc(s_function)) {
        f_nom += 3;
        do_supneg1_inverse = true;
      }
    }

    size_t ln = strlen(tmpl) + strlen(f_nom);
    zh_rv = new char[ln + 1];
    strcpy(zh_rv, tmpl);
    StrReplace(zh_rv, ln, "%letters%", f_nom);

    char *color_attr = n_symbols > 1 ? up_clr_func_attr : up_clr_math_attr;
    zh_rv = ColorizeMMLElement(zh_rv, color_attr, NULL, NULL);

    if (do_supneg1_inverse) {
      char *tmpl = GetTmplPtr(TMPL_MSUP); //  "<msup>\n%base%%script%</msup>\n"
      char *minus1 = MNfromNUMBER("&#x2212;1");
      size_t zln = strlen(tmpl) + strlen(zh_rv) + strlen(minus1);
      char *tmp = new char[zln + 1];
      strcpy(tmp, tmpl);
      StrReplace(tmp, zln, "%base%", zh_rv);
      StrReplace(tmp, zln, "%script%", minus1);
      delete[] zh_rv;
      delete[] minus1;
      zh_rv = tmp;
    }
  } else if (s_function->canonical_ID) {  // a function that we created
    //  for some mml input object
    TCI_ASSERT(canonicalID_mapper);
    // Function occurred in the math we're processing currently 
    const char *mml_markup =
      GetMarkupFromID(canonicalID_mapper, s_function->canonical_ID);
    // Function was defined in math we processed earlier
    if (!mml_markup && def_IDs_mapper)
      mml_markup =
        GetMarkupFromID(def_IDs_mapper, s_function->canonical_ID);
    if (mml_markup) {
      size_t ln = strlen(mml_markup);
      zh_rv = new char[ln + 1];
      strcpy(zh_rv, mml_markup);
    } else {
      TCI_ASSERT(!"No markup for this function.");
    }
  } else {
    //TCI_ASSERT(!"No contents on function.");
  }
  if (!do_supneg1_inverse && zh_rv) {
    BUCKET_REC *exp_bucket = FindBucketRec(s_function->bucket_list, MB_FUNC_EXPONENT);
    if (exp_bucket && exp_bucket->first_child) {
      // put exponent which is not -1 on function
      char *tmpl = GetTmplPtr(TMPL_MSUP); //  "<msup>\n%base%%script%</msup>\n"
      int nodes_made, terms_made, error_code;
      bool is_signed = false;
      char *zh_exp = ProcessSemanticList(exp_bucket->first_child,
                                         error_code, nodes_made, terms_made,
                                         is_signed, NULL, NULL);
      TCI_ASSERT(!error_code); // don't know what could go wrong here
      if (zh_exp && nodes_made > 1)
        zh_exp = NestzMMLInMROW(zh_exp);
      size_t zln = strlen(tmpl) + strlen(zh_rv) + strlen(zh_exp);
      char *tmp = new char[zln + 1];
      strcpy(tmp, tmpl);
      StrReplace(tmp, zln, "%base%", zh_rv);
      StrReplace(tmp, zln, "%script%", zh_exp);
      delete[] zh_rv;
      delete[] zh_exp;
      zh_rv = tmp;
    }
  }

  return zh_rv;
}

char *STree2MML::ComposeFuncCall(SEMANTICS_NODE * s_function,
                                 char *zh_header,
                                 bool trigarg_enabled,
                                 bool & trigarg_scripted,
                                 bool subscripted_arg,
                                 int nprimes,
                                 int deriv_style, SEMANTICS_NODE * s_diff)
{
  char *zh_rv = NULL;
  trigarg_scripted = false;

  char *arglist = NULL;
  U32 arg_ln = 0;

  int n_arg_nodes = 0;

  char *comma_markup = GetTmplPtr(TMPL_COMMA);
  size_t c_ln = strlen(comma_markup);
  char *z_comma = new char[c_ln + 1];
  strcpy(z_comma, comma_markup);
  z_comma = ColorizeMMLElement(z_comma, up_clr_math_attr, NULL, NULL);

  int nodes_made, terms_made;
  bool is_signed = false;

  BUCKET_REC *arg_b_rover = s_function->bucket_list;
  int arg_counter = 0;
  while (arg_b_rover) {
    if (arg_counter && !up_use_mfenced) {
      arglist = AppendStr2HeapStr(arglist, arg_ln, z_comma);
    }
    if (arg_b_rover->bucket_ID == MB_UNNAMED && arg_b_rover->first_child) {
      SEMANTICS_NODE *head = arg_b_rover->first_child;
      char *z_arg = NULL;
      if (head) {
        int error_code = 0;
        z_arg = ProcessSemanticList(head,
                                    error_code, nodes_made,
                                    terms_made, is_signed, NULL, NULL);

        n_arg_nodes = nodes_made;
        if (nodes_made > 1 || terms_made > 1 || is_signed) {
          if (z_arg) {
            z_arg = NestzMMLInMROW(z_arg);
            nodes_made = 1;
          }
        }
      }
      if (z_arg) {
        arglist = AppendStr2HeapStr(arglist, arg_ln, z_arg);
        delete[] z_arg;
      }
      arg_counter++;
    }
    arg_b_rover = arg_b_rover->next;
  }

  delete[] z_comma;

  if (arglist) {
    bool nest_in_mrow = false;
    if (arg_counter > 1) {
      if (!up_use_mfenced)      // comma separated list
        nest_in_mrow = true;
    } else {
      // Single args are mrow nested as required in the loop above
    }
    if (nest_in_mrow)
      arglist = NestzMMLInMROW(arglist);
  }

  char *primes_str = NULL;
  char *dots_str = NULL;
  char *dhead_str = NULL;

  char *tmpl;
  if (nprimes) {
    if (deriv_style == 1) {     // y"
      primes_str = GetPrimesStr(nprimes);
      if (subscripted_arg)
        tmpl = GetTmplPtr(TMPL_PFUNCSUBARGS);
      else
        tmpl = GetTmplPtr(TMPL_PFUNCARGS);

    } else if (deriv_style == 2) {  // \dot{y}
      dots_str = GetDotsStr(nprimes);
      if (subscripted_arg)
        tmpl = GetTmplPtr(TMPL_DotFUNCSUBARGS);
      else
        tmpl = GetTmplPtr(TMPL_DotFUNCARGS);
    } else if (deriv_style == 3) {  // D_{x} y
      char *vars_str = GetVarsStr(s_diff);
      if (!vars_str) {
        primes_str = GetPrimesStr(nprimes);
        if (subscripted_arg)
          tmpl = GetTmplPtr(TMPL_PFUNCSUBARGS);
        else
          tmpl = GetTmplPtr(TMPL_PFUNCARGS);
      } else {
        char *dop_str = PrefixMOfromSTRING(up_differentialD);

        char *pattern = GetTmplPtr(TMPL_DD_HEADER); //  "<msub>\n%DiffOp%%vars%</msub>\n",
        size_t zln = strlen(pattern) + strlen(dop_str);
        if (vars_str)
          zln += strlen(vars_str);
        dhead_str = new char[zln + 1];
        strcpy(dhead_str, pattern);
        StrReplace(dhead_str, zln, "%DiffOp%", dop_str);
        StrReplace(dhead_str, zln, "%vars%", vars_str);
        delete[] dop_str;
        delete[] vars_str;

        //  "%Dheader%%fheader%<mo>&#x2061;</mo>\n%arglist%",
        //  "%Dheader%<msub>\n%fheader%%arglist%</msub>\n",

        if (subscripted_arg)
          tmpl = GetTmplPtr(TMPL_DxFUNCSUBARGS);
        else
          tmpl = GetTmplPtr(TMPL_DxFUNCARGS);
      }
    }
  } else {
    if (subscripted_arg)
      tmpl = GetTmplPtr(TMPL_FUNCSUBARGS);
    else
      tmpl = GetTmplPtr(TMPL_FUNCARGS);
  }

  // Handle parens re trigargs
  if (!subscripted_arg) {
    bool add_parens = true;
    if (trigarg_enabled         // passed in - func is trigarg candidate
        && !up_parens_on_trigargs // client preference
        && terms_made < 2 && !is_signed) {
      add_parens = false;
    }
    
    if (n_arg_nodes > 1)
      add_parens = true;

    BUCKET_REC *arg_bucket =
      FindBucketRec(s_function->bucket_list, MB_UNNAMED);
    if (arg_bucket && arg_bucket->first_child) {
      SEMANTICS_NODE *head = arg_bucket->first_child;
      if (head->semantic_type == SEM_TYP_FUNCTION) {
        add_parens = true;
      } else if (head->semantic_type == SEM_TYP_TABULATION) {
        // if arg is single matrix and that matrix already added parens...
        if (head->prev == NULL && head->next == NULL && last_matrix_delim_type == 2)
          add_parens = false;
      }
    }
    if (add_parens) {
      int tmp_nodes_made;
      arglist = NestzMMLInPARENS(arglist, tmp_nodes_made);
      if (tmp_nodes_made > 1)
        arglist = NestzMMLInMROW(arglist);
    } else {
      trigarg_scripted = true;
    }
  }
  // append mml for the function args to the output
  size_t aln = strlen(tmpl);
  if (zh_header)
    aln += strlen(zh_header);
  if (arglist)
    aln += strlen(arglist);
  if (primes_str)
    aln += strlen(primes_str);
  if (dots_str)
    aln += strlen(dots_str);
  if (dhead_str)
    aln += strlen(dhead_str);
  zh_rv = new char[aln + 1];
  strcpy(zh_rv, tmpl);

  StrReplace(zh_rv, aln, "%fheader%", zh_header);
  StrReplace(zh_rv, aln, "%arglist%", arglist);
  if (primes_str) {
    StrReplace(zh_rv, aln, "%nprimes%", primes_str);
    delete[] primes_str;
  }
  if (dots_str) {
    StrReplace(zh_rv, aln, "%dots%", dots_str);
    delete[] dots_str;
  }
  if (dhead_str) {
    StrReplace(zh_rv, aln, "%Dheader%", dhead_str);
    delete[] dhead_str;
  }
  delete[] arglist;

  return zh_rv;
}

void STree2MML::DisposeFactorList(FACTOR_REC * f_list)
{
  FACTOR_REC *rover = f_list;
  while (rover) {
    FACTOR_REC *del = rover;
    rover = rover->next;
    delete[] del->zh_fstr;
    delete del;
  }
}

void STree2MML::SplitFactors(FACTOR_REC * f_list,
                             FACTOR_REC ** factors, FACTOR_REC ** units)
{
  FACTOR_REC *f_tail = NULL;
  FACTOR_REC *u_tail = NULL;

  FACTOR_REC *rover = f_list;
  while (rover) {
    FACTOR_REC *curr = rover;
    rover = rover->next;

    curr->next = NULL;
    if (curr->is_unit) {
      if (*units == NULL)
        *units = curr;
      else
        u_tail->next = curr;
      u_tail = curr;
    } else {
      if (*factors == NULL)
        *factors = curr;
      else
        f_tail->next = curr;
      f_tail = curr;
    }
  }
}

// Catenate a list of factors into a product.
// Handle redundant "1" factors.

char *STree2MML::FactorsTozStr(FACTOR_REC * f_list,
                               int & mml_node_count, int & mml_term_count,
                               bool & is_signed, bool is_units)
{
  char *zh_rv = NULL;
  U32 zh_ln = 0;

  mml_node_count = 0;
  mml_term_count = 1;
  is_signed = false;
  int save_term_count = 0;
  int loop_count = 0;

  int n_ones_allowed = OnesAllowed(f_list);

  bool last_was_operator = false;
  bool last_was_number = false;
  bool last_was_text = false;
  bool last_was_parened = false;
  FACTOR_REC *rover = f_list;
  while (rover) {
    if (rover->zh_fstr) {
      bool do_it = true;
      if (rover->is_number && !strcmp(rover->zh_fstr, "<mn>1</mn>\n")) {
        if (n_ones_allowed)
          n_ones_allowed--;  //SLS super ugly.  Is this a counter or a bool?
        else
          do_it = false;
      }
      if (do_it) {
        // Determine which multiplication operator is appropriate
        if (mml_node_count) {
          if (rover->is_text || last_was_text) {
          } else if (!rover->is_operator
                     && !last_was_operator && !last_was_parened) {
            char *times_op = NULL;
            if (is_units) {
              times_op = MOfromSTRING("&#xb7;", up_unit_attrs);
            } else {
              if (last_was_number && rover->is_number && (rover->n_terms < 2)) {
                times_op = MOfromSTRING("&#xd7;", up_clr_math_attr);
              } else {
                //times_op = MOfromSTRING("&#x2062;", up_clr_math_attr);
              } 
            }

            zh_rv = AppendStr2HeapStr(zh_rv, zh_ln, times_op);
            delete[] times_op;

            mml_node_count++;
          }
        }

        char *z_factor = rover->zh_fstr;
        rover->zh_fstr = NULL;

        if (rover->n_terms > 1) {
          if (mml_node_count || rover->next) {
            int tmp_nodes_made;
            z_factor = NestzMMLInMROW(z_factor);
            z_factor = NestzMMLInPARENS(z_factor, tmp_nodes_made);
            last_was_parened = true;
            mml_node_count += tmp_nodes_made;
          } else {
            save_term_count = rover->n_terms;
            mml_node_count += rover->mml_nodes_last_term;
          }
        } else if (rover->mml_nodes_last_term > 1 && rover->is_signed) {
          if (mml_node_count || rover->next) {
            int tmp_nodes_made;
            if (z_factor && rover->mml_nodes_last_term > 1)
              z_factor = NestzMMLInMROW(z_factor);
            // don't need parens if we're starting with -n
            if (mml_node_count || rover->mml_nodes_last_term > 2) {
              z_factor = NestzMMLInPARENS(z_factor, tmp_nodes_made);
              last_was_parened = true;
              mml_node_count += tmp_nodes_made;
            }
          } else {
            mml_node_count += rover->mml_nodes_last_term;
            if (rover == f_list)
              is_signed = true;
          }
        } else if (rover->mml_nodes_last_term > 1) {
          mml_node_count += rover->mml_nodes_last_term;
        } else {
          mml_node_count++;
          if (rover == f_list && rover->is_signed)
            is_signed = true;
        }

        zh_rv = AppendStr2HeapStr(zh_rv, zh_ln, z_factor);
        delete[] z_factor;

        last_was_operator = rover->is_operator;
        last_was_number = rover->is_number;
        last_was_text = rover->is_text;
      }
    }
    rover = rover->next;
    ++loop_count;
  }

  if (mml_node_count == 1 && save_term_count > 1)
    mml_node_count = save_term_count;
  if (loop_count == 1 && save_term_count > 1)
    mml_term_count = save_term_count;

  return zh_rv;
}

// Separate factors that represent units, order factors,
//  numbers * constants * others

char *STree2MML::ComposeTerm(FACTOR_REC * num_factors,
                             FACTOR_REC * den_factors,
                             bool & is_signed, int & nodes_made, int & terms_made)
{
  char *rv = NULL;

  char *rv1 = NULL;             // non-units
  char *zmml_unit_str = NULL;   // units
  is_signed = false;

  // Here we separate a series of factors into non-units and units.
  //  Units are gathered into a separate factor that follows the term modified.
  FACTOR_REC *top_factors = NULL;
  FACTOR_REC *top_units = NULL;
  SplitFactors(num_factors, &top_factors, &top_units);

  FACTOR_REC *bot_factors = NULL;
  FACTOR_REC *bot_units = NULL;
  SplitFactors(den_factors, &bot_factors, &bot_units);

  top_factors = SetFactorOrder(top_factors);
  bot_factors = SetFactorOrder(bot_factors);

  int n_num_factors = 0;
  int n_den_factors = 0;
  bool num_is_signed = false;
  bool den_is_signed = false;

  if (top_factors || bot_factors) { // something other than units
    if (!bot_factors) {
      // Non-unit part is not a fraction
      int n_num_terms;
      rv1 = FactorsTozStr(top_factors, n_num_factors, n_num_terms, num_is_signed, false);
      terms_made = n_num_terms;
    } else {
      terms_made = 1;
      bool numeric_denom = DenomIsNumber(bot_factors);
      if (numeric_denom) {
        char *z_num;
        top_factors = ExtractNumNumber(top_factors, &z_num);

        int n_den_terms;
        char *z_den = FactorsTozStr(bot_factors, n_den_factors, n_den_terms,
                                    den_is_signed, false);
        char *tmpl = GetTmplPtr(TMPL_MFRAC);
        size_t zln = strlen(tmpl);
        if (z_num)
          zln += strlen(z_num);
        if (z_den)
          zln += strlen(z_den);
        char *mfrac = new char[zln + 1];
        strcpy(mfrac, tmpl);
        StrReplace(mfrac, zln, "%num%", z_num);
        StrReplace(mfrac, zln, "%den%", z_den);
        delete[] z_num;
        delete[] z_den;

        char *z_body = NULL;
        int n_factors = 0;
        if (top_factors) {
          int n_terms;
          z_body = FactorsTozStr(top_factors, n_factors, n_terms,
                                 num_is_signed, false);
        }
        if (z_body) {
          if (top_factors->is_operator) {
            size_t zln = strlen(mfrac) + strlen(z_body);
            rv1 = new char[zln + 1];
            strcpy(rv1, mfrac);
            delete[] mfrac;
            strcat(rv1, z_body);
            delete[] z_body;
            n_num_factors = n_factors + 1;
          } else {
            char *it = MOfromSTRING("&#x2062;", up_clr_math_attr);
            size_t zln = strlen(mfrac) + strlen(it) + strlen(z_body);
            rv1 = new char[zln + 1];
            strcpy(rv1, mfrac);
            delete[] mfrac;
            strcat(rv1, it);
            strcat(rv1, z_body);
            delete[] it;
            delete[] z_body;
            n_num_factors = n_factors + 2;
          }
        } else {
          rv1 = mfrac;
          n_num_factors = 1;
        }
      } else {
        // Generate the numerator
        char *z_num = NULL;
        if (top_factors) {
          int n_num_terms;
          z_num = FactorsTozStr(top_factors, n_num_factors, n_num_terms,
                                num_is_signed, false);
        } else {
          z_num = MNfromNUMBER("1");
          n_num_factors = 1;
        }

        // Generate the denominator
        int n_den_terms;
        char *z_den = FactorsTozStr(bot_factors, n_den_factors, n_den_terms,
                                    den_is_signed, false);
        // Nest num and denom in mrows, if necessary
        if (n_num_factors > 1)
          z_num = NestzMMLInMROW(z_num);
        if (n_den_factors > 1) {
          z_den = NestzMMLInMROW(z_den);
        }
        // Compose the mfrac return
        char *tmpl = GetTmplPtr(TMPL_MFRAC);
        size_t zln = strlen(tmpl);
        if (z_num)
          zln += strlen(z_num);
        if (z_den)
          zln += strlen(z_den);

        rv1 = new char[zln + 1];
        strcpy(rv1, tmpl);
        StrReplace(rv1, zln, "%num%", z_num);
        StrReplace(rv1, zln, "%den%", z_den);
        delete[] z_num;
        delete[] z_den;
      }
    }                           // has denominator clause
  }
  // It remains to handle units

  if (top_units || bot_units) {
    terms_made = 1;
    int n_num_factors = 0;
    int n_den_factors = 0;
    bool top_is_signed = false;
    bool bot_is_signed = false;

    int baseunit_powers[NUMBASEUNITS];
    for (int i = 0; i < NUMBASEUNITS; i++)
      baseunit_powers[i] = 0;
    GetBUnitPowers(top_units, baseunit_powers, true);
    GetBUnitPowers(bot_units, baseunit_powers, false);

    zmml_unit_str = BUnitsToCompoundUnit(baseunit_powers);
    bool add_unit_style = false;

    if (!zmml_unit_str) {
      // Here, we have unit(s) that don't map to a single derived unit.
      if (!bot_units) {
        int n_num_terms;
        zmml_unit_str = FactorsTozStr(top_units, n_num_factors, n_num_terms,
                                      top_is_signed, true);
        if (n_num_factors > 1)
          add_unit_style = true;
      } else {
        add_unit_style = true;

        char *z_num = NULL;
        if (top_units) {
          int n_num_terms;
          z_num = FactorsTozStr(top_units, n_num_factors, n_num_terms,
                                top_is_signed, true);
        } else {
          z_num = MNfromNUMBER("1");
          n_num_factors = 1;
        }
        int n_den_terms;
        char *z_den = FactorsTozStr(bot_units, n_den_factors, n_den_terms,
                                    bot_is_signed, true);

        if (n_num_factors > 1)
          z_num = NestzMMLInMROW(z_num);
        if (n_den_factors > 1)
          z_den = NestzMMLInMROW(z_den);

        char *tmpl = GetTmplPtr(TMPL_MFRAC);
        size_t zln = strlen(tmpl);
        if (z_num)
          zln += strlen(z_num);
        if (z_den)
          zln += strlen(z_den);

        zmml_unit_str = new char[zln + 1];
        strcpy(zmml_unit_str, tmpl);
        StrReplace(zmml_unit_str, zln, "%num%", z_num);
        StrReplace(zmml_unit_str, zln, "%den%", z_den);
        delete[] z_num;
        delete[] z_den;
      }
    }
    if (zmml_unit_str) {
      char *tmpl = GetTmplPtr(TMPL_UNIT_STYLE);
      // "<mstyle %unit_attrs%>\n%body%</mstyle>\n",
      size_t zln = strlen(tmpl) + strlen(zmml_unit_str);
      if (up_unit_attrs)
        zln += strlen(up_unit_attrs);
      char *body = zmml_unit_str;
      zmml_unit_str = new char[zln + 1];
      strcpy(zmml_unit_str, tmpl);
      StrReplace(zmml_unit_str, zln, "%unit_attrs%", up_unit_attrs);
      StrReplace(zmml_unit_str, zln, "%body%", body);
      delete[] body;
    }
  }

  DisposeFactorList(top_factors);
  DisposeFactorList(top_units);
  DisposeFactorList(bot_factors);
  DisposeFactorList(bot_units);

  if (rv1 && zmml_unit_str) {   // term + units
    TCI_ASSERT(rv == NULL);
    U32 rv_ln = 0;
    rv = AppendStr2HeapStr(rv, rv_ln, rv1);
    delete[] rv1;

    char *times = MOfromSTRING("&#x2062;", up_clr_math_attr);
    rv = AppendStr2HeapStr(rv, rv_ln, times);
    delete[] times;

    rv = AppendStr2HeapStr(rv, rv_ln, zmml_unit_str);
    delete[] zmml_unit_str;

    nodes_made = 3;
  } else if (rv1) {
    rv = rv1;
    nodes_made = n_num_factors;
    if (!bot_factors)
      is_signed = num_is_signed;
  } else if (zmml_unit_str) {
    rv = zmml_unit_str;
    nodes_made = n_den_factors;
  }

  return rv;
}

bool STree2MML::SNodeEndsTerm(SEMANTICS_NODE * s_node,
                                  int & curr_prec, bool & is_plusORminus)
{
  bool rv = false;
  is_plusORminus = false;
  curr_prec = 0;

  switch (s_node->semantic_type) {

  case SEM_TYP_VARIABLE:
  case SEM_TYP_QUALIFIED_VAR:
  case SEM_TYP_NUMBER:
  case SEM_TYP_MIXEDNUMBER:
  case SEM_TYP_FUNCTION:
  case SEM_TYP_TRANSFORM:
  case SEM_TYP_INVTRANSFORM:
    break;

  case SEM_TYP_PREFIX_OP:
  case SEM_TYP_INFIX_OP:
  case SEM_TYP_POSTFIX_OP:
    if (s_node->contents) {
      char *ptr = strstr(s_node->contents, "&#x");
      if (ptr) {
        U32 unicode = ASCII2U32(ptr + 3, 16);
        if (unicode == 0x2212 ||  // "-"
            unicode == 0xb1 ||    // plusminus
            unicode == 0x2213) {  // minusplus
          is_plusORminus = true;
          rv = true;
        } else if (unicode == 0x3c  // "<"
                   || unicode == 0x3d // "="
                   || unicode == 0x3e // ">"
                   || unicode == 0xac // not
                   || unicode == 0x2192 // "->"
                   || unicode == 0x220a // "\in"
                   || unicode == 0x2227 // and
                   || unicode == 0x2228 // or
                   || unicode == 0x2229 // "\cap" - intersect
                   || unicode == 0x222a // "\cup" - union
                   || unicode == 0x2264 // "<="
                   || unicode == 0x2265 // ">="
                   || unicode == 0x2260)  // "<>"
          rv = true;
      } else {
        size_t zln = strlen(s_node->contents);
        if (zln == 1) {
          char ch = s_node->contents[0];
          if (ch == '+' || ch == '-' || ch == '=' || ch == '<'
              || ch == '>' || ch == ',' || ch == '|')
            rv = true;
          if (ch == '+')
            is_plusORminus = true;
        } else if (zln == 2) {
          char ch0 = s_node->contents[0];
          char ch1 = s_node->contents[1];
          if (ch0 == '.' && ch1 == '.')
            rv = true;
          else if (ch0 == '!' && ch1 == '!')
            rv = true;
          else
            TCI_ASSERT(!"Unknown 2 letter operator.");
        } else if (zln == 3) {
          char ch0 = s_node->contents[0];
          char ch1 = s_node->contents[1];
          char ch2 = s_node->contents[2];
          if (ch0 == 'm' && ch1 == 'o' && ch2 == 'd')
            rv = true;
          else if (ch0 == 'd' && ch1 == 'i' && ch2 == 'v')
            rv = true;
          else
            TCI_ASSERT(!"Unknown 3 letter operator.");
		} else if (zln == 4) {
          char ch0 = s_node->contents[0];
          char ch1 = s_node->contents[1];
          char ch2 = s_node->contents[2];
          char ch3 = s_node->contents[3];
          if (ch0 == 'g' && ch1 == 'r' && ch2 == 'a' && ch3 == 'd')
            rv = true;
          else if (ch0 == 'c' && ch1 == 'u' && ch2 == 'r' && ch3 == 'l')
            rv = true;
          else
            TCI_ASSERT(!"Unknown 4 letter operator.");
		} else if (zln == 9) {
		  if (!strcmp(s_node->contents,"Laplacian"))
		    rv = true;
		  else
		    TCI_ASSERT(!"Unknown 9 letter operator.");
		} else {
	      TCI_ASSERT(!"Unknown operator");
		}
      }
    } else {
      TCI_ASSERT(0);
    }
    break;

  case SEM_TYP_UCONSTANT:
  case SEM_TYP_BOOLEAN:
  case SEM_TYP_SPACE:
  case SEM_TYP_TEXT:
  case SEM_TYP_INVFUNCTION:
  case SEM_TYP_SIUNIT:
  case SEM_TYP_USUNIT:
    break;

  // Schemata
  case SEM_TYP_MATH_CONTAINER:
    rv = true;
    break;

  case SEM_TYP_PRECEDENCE_GROUP:
  case SEM_TYP_PARENED_LIST:
  case SEM_TYP_BRACKETED_LIST:
  case SEM_TYP_PIECEWISE_FENCE:
  case SEM_TYP_GENERIC_FENCE:
  case SEM_TYP_QSUB_LIST:
  case SEM_TYP_INTERVAL:
    break;

  case SEM_TYP_FRACTION:
  case SEM_TYP_POWERFORM:
  case SEM_TYP_SQRT:
  case SEM_TYP_ROOT:
  case SEM_TYP_BIGOP_SUM:
  case SEM_TYP_BIGOP_INTEGRAL:
  case SEM_TYP_TABULATION:
  case SEM_TYP_LIMFUNC:
  case SEM_TYP_BINOMIAL:
  case SEM_TYP_SET:
  case SEM_TYP_ABS:
  case SEM_TYP_NORM:
  case SEM_TYP_FLOOR:
  case SEM_TYP_CEILING:
  case SEM_TYP_CONJUGATE:
  case SEM_TYP_MTRANSPOSE:
  case SEM_TYP_HTRANSPOSE:
  case SEM_TYP_SIMPLEX:
  case SEM_TYP_LIST:
  case SEM_TYP_PIECEWISE_LIST:
  case SEM_TYP_ONE_PIECE:
  case SEM_TYP_VARDEF_DEFERRED:
  case SEM_TYP_VARDEF:
  case SEM_TYP_FUNCDEF:
  case SEM_TYP_LISTOPERATOR:

  case SEM_TYP_DERIVATIVE:
  case SEM_TYP_ODE_SYSTEM:

  case SEM_TYP_SUBSTITUTION:
  case SEM_TYP_MULTIARGFUNC:

  case SEM_TYP_EIGENVECTORSET:
  case SEM_TYP_EIGENVECTOR:
  case SEM_TYP_PARTIALD:
  case SEM_TYP_QUANTILE_RESULT:
  case SEM_TYP_EQCHECK_RESULT:
    break;

  default:
    TCI_ASSERT(!"Unclassified semantic type.");
    break;
  }

  if (s_node->semantic_type == SEM_TYP_INFIX_OP && s_node->contents) {
    curr_prec = s_node->infix_precedence;
  }

  return rv;
}

FACTOR_REC *STree2MML::JoinFactors(FACTOR_REC * f_list,
                                   FACTOR_REC * more_factors)
{
  if (f_list) {
    FACTOR_REC *rover = f_list;
    while (rover->next)
      rover = rover->next;
    rover->next = more_factors;
    return f_list;
  } else
    return more_factors;
}

void STree2MML::GetBUnitPowers(FACTOR_REC * units_list,
                               int *baseunit_powers, bool is_num)
{
  FACTOR_REC *f_rover = units_list;
  while (f_rover) {
    if (f_rover->is_unit) {
      TCI_ASSERT(f_rover->unit_power);
      if (f_rover->unit_ilk > 0 && f_rover->unit_ilk <= NUMBASEUNITS) {
        int slot = f_rover->unit_ilk - 1;
        int power = f_rover->unit_power;
        if (!is_num)
          power = -1 * power;
        baseunit_powers[slot] += power;
      } else {
        TCI_ASSERT(!"Unit ilk out of range.");
      }
    } else {
      TCI_ASSERT(!"Non-unit in unit list.");
    }
    f_rover = f_rover->next;
  }
}

/*
Base Units
 length
 mass
 time
 electric current
 thermodynamic temperature
 amount of substance
 luminous intensity
                Radian<K1.6/>       rad

Derived Units

Activity	Becquerel<K1.2/>	Bq
            Curie<K1.3/>	    Ci

Amount of Substance<K1.1/>	Attomole<K1.2/>   amol
                            Examole<K1.3/>    Emol
                            Femtomole<K1.4/>  fmol
                            Gigamole<K1.5/>   Gmol
                            Kilomole<K1.6/>   kmol
                            Megamole<K1.7/>   Mmol
                            Micromole<K1.8/>  µmol
                            Millimole<K1.9/>  mmol
                            Mole<K1.10/>      mol
                            Nanomole<K1.11/>  nmol
                            Petamole<K1.12/>	Pmol 	uPmol
                            Picomole<K1.13/>	pmol 	upmol
                            Teramole<K1.14/>	Tmol 	uTmol

Area<K1.1/>	Acre<K1.2/>	        acre
            Hectare<K1.3/>	    hectare
            Square foot<K1.4/>	ft²
         	Square inch<K1.5/>	in²
        	Square meter<K1.6/>	m²

Current<K1.1/>	Ampere<K1.2/>	A 	uA
                Kiloampere<K1.3/>	kA 	ukA
                Microampere<K1.4/>	µA 	umcA
            	Milliampere<K1.5/>	mA 	umA
            	Nanoampere<K1.6/>	nA 	unA

Electric capacitance	Farad<K1.2/>	F 	uF
                        Microfarad<K1.3/>	µF 	umcF
                        Millifarad<K1.4/>	mF 	umF
                        Nanofarad<K1.5/>	nF 	unF
                        Picofarad<K1.6/>	pF 	upF

Electric charge	Coulomb<K1.2/>	C 	uCo
			
Electric conductance	Kilosiemens<K1.4/>	kS 	ukS
                        Microsiemens<K1.5/>	µS 	umcS
                        Millisiemens<K1.6/>	mS 	umS
                        Siemens<K1.7/>	S 	uS

Electrical potential difference	Kilovolt<K1.2/>	kV 	ukV
                                Megavolt<K1.3/>	MV 	uMV
                                Microvolt<K1.4/>	µV 	umcV
                                Millivolt<K1.5/>	mV 	umV
                                Nanovolt<K1.6/>	nV 	unV
                                Picovolt<K1.7/>	pV 	upV
                                Volt<K1.8/>	V 	uV

Electric resistance	Gigaohm<K1.2/>	GO 	uGohm
                                Kiloohm<K1.3/>	kO 	ukohm
                                Megaohm<K1.4/>	MO 	uMohm
                                Milliohm<K1.5/>	mO 	umohm
                                Ohm<K1.6/>	O 	uohm

Energy	British thermal unit<K1.2/>	Btu 	uBtu
                                    Calorie<K1.3/>	cal 	ucal
                                    Electron volt<K1.4/>	eV 	ueV
                                    Erg<K1.5/>	erg 	uerg
                                    Gigaelectronvolt<K1.6/>	GeV 	uGeV
                                    Gigajoule<K1.7/>	GJ 	uGJ
                                    Joule<K1.8/>	J 	uJ
                                    Kilocalorie<K1.9/>	kcal 	ukcal
                                    Kilojoule<K1.10/>	kJ 	ukJ
                                    Megaelectronvolt<K1.11/>	MeV 	uMeV
                                    Megajoule<K1.12/>	MJ 	uMJ
                                    Microjoule<K1.13/>	µJ 	umcJ
                                    Millijoule<K1.14/>	mJ 	umJ
                                    Nanojoule<K1.15/>	nJ 	unJ

Force	Dyne<K1.2/>	dyn 	udyn
	Kilonewton<K1.3/>	kN 	ukN
	Meganewton<K1.4/>	MN 	uMN
	Micronewton<K1.5/>	µN 	umcN
	Millinewton<K1.6/>	mN 	umN
	Newton<K1.7/>	N 	uN
	Ounce-force<K1.8/>	ozf 	uozf
	Pound-force<K1.9/>	lbf 	ulbf

Frequency	Exahertz<K1.2/>	EHz 	uEHz
	Gigahertz<K1.3/>	GHz 	uGHz
	Hertz<K1.4/>	Hz 	uHz
	Kilohertz<K1.5/>	kHz 	ukHz
	Megahertz<K1.6/>	MHz 	uMHz
	Petahertz<K1.7/>	PHz 	uPHz
	Terahertz<K1.8/>	THz 	uTHz

Illuminance	Footcandle<K1.2/>	fc 	ufc
	Lux<K1.3/>	lx 	ulx
	Phot<K1.4/>	phot 	uphot

Length	Angstrom<K1.2/>	¨A 	uan
	Attometer<K1.3/>	am 	uame
	Centimeter<K1.4/>	cm 	ucm
	Decimeter<K1.5/>	dm 	udme
	Femtometer<K1.6/>	fm 	ufme
	Foot<K1.7/>	ft 	uft
	Inch<K1.8/>	in 	uin
	Kilometer<K1.9/>	km 	ukme
	Meter<K1.10/>	m 	ume
	Micrometer<K1.11/>	µm 	umcme
	Mile<K1.12/>	mi 	umi
	Millimeter<K1.13/>	mm 	umme
	Nanometer<K1.14/>	nm 	unme
	Picometer<K1.15/>	pm 	upme

Magnetic flux	Maxwell<K1.2/>	Mx 	uMx
	Microweber<K1.3/>	µWb 	umcWb
	Milliweber<K1.4/>	mWb 	umWb
	Nanoweber<K1.5/>	nWb 	unWb
	Weber<K1.6/>	Wb 	uWb

Magnetic flux density	Gauss<K1.2/>	G 	uGa
	Microtesla<K1.3/>	µT 	umcT
	Millitesla<K1.4/>	mT 	umT
	Nanotesla<K1.5/>	nT 	unT
	Picotesla<K1.6/>	pT 	upT
	Tesla<K1.7/>	T 	uTe

Magnetic inductance     Henry<K1.2/>	H 	uHe
                        Microhenry<K1.3/>	µH 	umcH
                        Millihenry<K1.4/>	mH 	umH

Mass	Atomic mass unit<K1.2/>	u 	uu
        Centigram<K1.3/>	cg 	ucg
        Decigram<K1.4/>	dg 	udg
        Gram<K1.5/>	g 	ugr
        Kilogram<K1.6/>	kg 	ukg
        Microgram<K1.7/>	µg 	umcg
        Milligram<K1.8/>	mg 	umg
        Pound-mass<K1.9/>	lb 	ulbm
        Slug<K1.10/>	slug 	uslug

Plane angle     Degree<K1.2/>       °
                Microradian<K1.3/>  µrad
                Milliradian<K1.4/>  mrad
                Minute<K1.5/>       '
                Radian<K1.6/>       rad
                Second<K1.7/>	? 	uds

Power       Gigawatt<K1.2/>     GW
            Horsepower<K1.3/>   hp
            Kilowatt<K1.4/>     kW
            Megawatt<K1.5/>     MW
            Microwatt<K1.6/>    µW
            Milliwatt<K1.7/>    mW
            Nanowatt<K1.8/>     nW
            Watt<K1.9/>         W

Pressure    Atmosphere<K1.2/>       atm
            Bar<K1.3/>              bar
            Kilobar<K1.4/>          kbar
            Kilopascal<K1.5/>       kPa
            Megapascal<K1.6/>       MPa
            Micropascal<K1.7/>      µPa
            Millibar<K1.8/>         mbar
            Millimeters of Mercury (0? )<K1.9/>	mmHg
            Pascal<K1.10/>          Pa
            Torr<K1.11/>            torr

Solid angle	Steradian<K1.2/>    sr
			
Temperature	Celsius<K1.4/>	? 	ucel
	Fahrenheit<K1.5/>	? 	ufahr
	Kelvin<K1.6/>	K 	uK

Time      Attosecond<K1.2/>     as
          Day<K1.3/>            d
          Femtosecond<K1.4/>    fs
          Hour<K1.5/>           h
          Microsecond<K1.6/>    µs
          Millisecond<K1.7/>    ms
          Minute<K1.8/>         min
          Nanosecond<K1.9/>     ns
          Picosecond<K1.10/>    ps
          Second<K1.11/>        s
          Year<K1.12/>          y

Volume	Cubic foot<K1.2/>	ft³
        Cubic inch<K1.3/>	in³
        Cubic meter<K1.4/>	m³ 
        Gallon (US)<K1.5/>	gal
        Liter<K1.6/>        l
        Milliliter<K1.7/>	ml
        Pint<K1.8/>         pint
        Quart<K1.9/>        qt

*/

/*
             length(m) mass(kg) time(s) current(A) temp(K)  subst(mole)  candela(cd) solidangle(sr)
                 0        0        0        0	    0		   0	        0			  0
force(N)		 1        1		  -2
energy(J)        2        1       -2
power(W)         2        1       -3
pressure(Pa)	-1        1		  -2
elec charge(C)	 0		  0		   1		1
voltage(V)       2        1       -3	   -1
resistance(Ohm)  2        1       -3	   -2
*/

typedef struct tagCOMPUNIT_INFO
{
  char *symbol;
  int nfactors_kg;
  int nfactors_m;
  int nfactors_s;
  int nfactors_A;
  int nfactors_K;
  int nfactors_mol;
  int nfactors_cd;
  int nfactors_sr;
} COMPUNIT_INFO;

COMPUNIT_INFO cunit_data[] = {
  {"N", 1, 1, -2, 0, 0, 0, 0, 0},
  {"J", 1, 2, -2, 0, 0, 0, 0, 0},
  {"W", 1, 2, -3, 0, 0, 0, 0, 0},
  {"Pa", 1, -1, -2, 0, 0, 0, 0, 0},
  {"C", 0, 0, 1, 1, 0, 0, 0, 0},
  {"V", 1, 2, -3, -1, 0, 0, 0, 0},
  {"&#x3a9;", 1, 2, -3, -2, 0, 0, 0, 0},
  {"H", 1, 2, -2, -2, 0, 0, 0, 0},
  {"T", 1, 0, -2, -1, 0, 0, 0, 0},
  {"Wb", 1, 2, -2, -1, 0, 0, 0, 0},
  {"lx", 0, -2, 0, 0, 0, 0, 1, 1},
  {"lm", 0, 0, 0, 0, 0, 0, 1, 1},
  {"S", -1, -2, 3, 2, 0, 0, 0, 0},
  {"F", -1, -2, 4, 2, 0, 0, 0, 0},
  {"Bq", 0, 0, -1, 0, 0, 0, 0, 0},
  {"Gy", 0, 2, -2, 0, 0, 0, 0, 0}
};

// Return a derived unit from a corresponding group of base units
//   kg*m/s^2   -->  n

char *STree2MML::BUnitsToCompoundUnit(int *baseunit_powers)
{
  char *zh_rv = NULL;

  for (int i = 0; i < sizeof(cunit_data)/sizeof(COMPUNIT_INFO); i++) {
    COMPUNIT_INFO *ui = &cunit_data[i];
    if (ui->nfactors_kg == baseunit_powers[0]
        && ui->nfactors_m == baseunit_powers[1]
        && ui->nfactors_s == baseunit_powers[2]
        && ui->nfactors_A == baseunit_powers[3]
        && ui->nfactors_K == baseunit_powers[4]
        && ui->nfactors_mol == baseunit_powers[5]
        && ui->nfactors_cd == baseunit_powers[6]
        && ui->nfactors_sr == baseunit_powers[7]) {
      char *tmpl = GetTmplPtr(TMPL_UNIT);
      // "<mi %unit_attrs%>%nom%</mi>\n",
      char *text = ui->symbol;
      size_t ln = strlen(tmpl) + strlen(text);
      if (up_unit_attrs)
        ln += strlen(up_unit_attrs);
      zh_rv = new char[ln + 1];
      strcpy(zh_rv, tmpl);
      StrReplace(zh_rv, ln, "%nom%", text);
      StrReplace(zh_rv, ln, "%unit_attrs%", up_unit_attrs);
      break;
    }
  }

  return zh_rv;
}

// Utility to set info used to eliminate redundant 1's in a series
//  of factors.  ie.  1*x*1*y -> xy

int STree2MML::OnesAllowed(FACTOR_REC * f_list)
{
  int rv = 0;

  int n_factors = 0;
  int n_ones = 0;

  FACTOR_REC *rover = f_list;
  if (rover && rover->zh_fstr) {
    if (rover->is_operator)     // a sign?
      rover = rover->next;
  }

  while (rover) {
    if (rover->zh_fstr) {
      n_factors++;
      if (rover->is_number && !strcmp(rover->zh_fstr, "<mn>1</mn>\n"))
        n_ones++;
    }
    rover = rover->next;
  }

  if (n_ones) {
    rv = 1;
    if (n_factors > n_ones)
      rv = 0;
  }

  return rv;
}

// Order the factors within one term

FACTOR_REC *STree2MML::SetFactorOrder(FACTOR_REC * factors)
{
  FACTOR_REC *sign = NULL;
  FACTOR_REC *numbers = NULL;
  FACTOR_REC *constants = NULL;
  FACTOR_REC *others = NULL;

  FACTOR_REC *rover = factors;
  if (rover && rover->is_operator) {
    FACTOR_REC *curr = rover;
    rover = rover->next;
    curr->next = NULL;
    sign = AppendFactor(sign, curr);
  }

  FACTOR_REC *prev_list = NULL;
  while (rover) {
    FACTOR_REC *curr = rover;
    rover = rover->next;
    curr->next = NULL;
    if (curr->is_number) {
      numbers = AppendFactor(numbers, curr);
      prev_list = numbers;
    } else if (curr->is_constant) {
      constants = AppendFactor(constants, curr);
      prev_list = constants;
    } else if (curr->is_operator) {
      if (prev_list)
        prev_list = AppendFactor(prev_list, curr);
      else
        TCI_ASSERT(0);
    } else {
      others = AppendFactor(others, curr);
      prev_list = others;
    }
  }

  FACTOR_REC *rv = sign;
  rv = AppendFactor(rv, numbers);
  rv = AppendFactor(rv, constants);
  rv = AppendFactor(rv, others);

  return rv;
}

// Append a factor to the end of a list of factors

FACTOR_REC *STree2MML::AppendFactor(FACTOR_REC * f_list, FACTOR_REC * f_next)
{
  if (f_list) {
    FACTOR_REC *rover = f_list;
    while (rover->next)
      rover = rover->next;
    rover->next = f_next;
    return f_list;
  } else
    return f_next;
}

SEMANTICS_NODE *STree2MML::RemoveParens(SEMANTICS_NODE * s_factor,
                                        BUCKET_REC * parent_bucket)
{
  SEMANTICS_NODE *rv = s_factor;

  if (s_factor && s_factor->bucket_list && !s_factor->next
      && (s_factor->semantic_type == SEM_TYP_PRECEDENCE_GROUP
          || s_factor->semantic_type == SEM_TYP_PARENED_LIST)) {
    SEMANTICS_NODE *s_child = s_factor->bucket_list->first_child;
    if (s_child) {
      rv = s_child;
      rv->parent = NULL;
      s_factor->bucket_list->first_child = NULL;
      DisposeSList(s_factor);
      if (parent_bucket) {
        parent_bucket->first_child = rv;
        rv->parent = parent_bucket;
      }
    }
  }
  return rv;
}

void STree2MML::SetUnitInfo(SEMANTICS_NODE * snode, FACTOR_REC * new_fr)
{
  char *unit_symbol = NULL;
  bool is_unit = false;

  if (snode->semantic_type == SEM_TYP_SIUNIT
      || snode->semantic_type == SEM_TYP_USUNIT) {
    is_unit = true;
    new_fr->unit_power = 1;
    unit_symbol = snode->contents;
  } else if (snode->semantic_type == SEM_TYP_POWERFORM) {
    BUCKET_REC *base_bucket = FindBucketRec(snode->bucket_list,
                                            MB_SCRIPT_BASE);
    if (base_bucket && base_bucket->first_child) {
      SEMANTICS_NODE *s_cont = base_bucket->first_child;
      if (s_cont->semantic_type == SEM_TYP_SIUNIT
          || s_cont->semantic_type == SEM_TYP_USUNIT) {
        is_unit = true;
        unit_symbol = s_cont->contents;
        BUCKET_REC *sup_bucket =
          FindBucketRec(snode->bucket_list, MB_SCRIPT_UPPER);
        if (sup_bucket && sup_bucket->first_child) {
          SEMANTICS_NODE *s_cont = sup_bucket->first_child;
          if (s_cont->semantic_type == SEM_TYP_NUMBER)
            new_fr->unit_power = atoi(s_cont->contents);
          else
            TCI_ASSERT(0);
        }
      }
    }
  }
  if (is_unit) {
    new_fr->is_unit = true;
    if (unit_symbol) {
      size_t zln = strlen(unit_symbol);
      if (zln == 1) {
        if (unit_symbol[0] == 'm')
          new_fr->unit_ilk = 1;
        else if (unit_symbol[0] == 's')
          new_fr->unit_ilk = 3;
        else if (unit_symbol[0] == 'A')
          new_fr->unit_ilk = 4;
        else if (unit_symbol[0] == 'K')
          new_fr->unit_ilk = 5;
      } else if (zln == 2) {
        if (unit_symbol[0] == 'k' && unit_symbol[1] == 'g')
          new_fr->unit_ilk = 2;
        else if (unit_symbol[0] == 'c' && unit_symbol[1] == 'd')
          new_fr->unit_ilk = 7;
        else if (unit_symbol[0] == 's' && unit_symbol[1] == 'r')
          new_fr->unit_ilk = 8;
      } else if (zln == 3) {
        if (!strcmp(unit_symbol, "mol"))
          new_fr->unit_ilk = 6;
        else if (!strcmp(unit_symbol, "rad"))
          new_fr->unit_ilk = 9;
      }
    }
  }
}

FACTOR_REC *STree2MML::CreateFactor()
{
  FACTOR_REC *new_fr = new FACTOR_REC();
  new_fr->next = NULL;
  new_fr->zh_fstr = NULL;
  new_fr->n_terms = 0;
  new_fr->mml_nodes_last_term = 0;
  new_fr->unit_power = 0;
  new_fr->unit_ilk = 0;
  new_fr->is_unit = false;
  new_fr->is_operator = false;
  new_fr->is_number = false;
  new_fr->is_text = false;
  new_fr->is_constant = false;
  new_fr->is_signed = false;

  return new_fr;
}

char *STree2MML::AddSubToVariable(SEMANTICS_NODE * s_sub, char *zh_var)
{
  char *zh_rv = NULL;

  int error_code = 0;
  int nodes_made, terms_made;
  bool is_signed = false;
  char *zh_sub = ProcessSemanticList(s_sub, error_code,
                                     nodes_made, terms_made,
                                     is_signed, NULL, NULL);
  if (terms_made > 1 || nodes_made > 1 || is_signed)
    zh_sub = NestzMMLInMROW(zh_sub);

  // I'm mapping "s12" to "s_{21}" - not always correct in tensor notation.
  if (zh_sub) {
    char *tmpl = GetTmplPtr(TMPL_MSUB);
    size_t zln = strlen(tmpl) + strlen(zh_sub);
    if (zh_var)
      zln += strlen(zh_var);
    zh_rv = new char[zln + 1];
    strcpy(zh_rv, tmpl);
    StrReplace(zh_rv, zln, "%base%", zh_var);
    StrReplace(zh_rv, zln, "%script%", zh_sub);
    delete[] zh_sub;
  }
  delete[] zh_var;
  return zh_rv;
}

// A unary + or - that occurs infront of a single term
//  is catenated to the term here.  If the sign is in front
//  of a multi-term expression, we set the "extract_sign" flag.

FACTOR_REC *STree2MML::AbsorbUnaryOps(FACTOR_REC * factors,
                                      bool & extract_sign)
{
  FACTOR_REC *rv = factors;
  extract_sign = false;

  if (factors && factors->is_operator && factors->next) {
    if (factors->next->n_terms > 1) {
      extract_sign = true;
    } else {
      size_t ln1 = strlen(factors->zh_fstr);
      rv = factors->next;
      size_t ln2 = strlen(rv->zh_fstr);
      char *cat = new char[ln1 + ln2 + 1];
      strcpy(cat, factors->zh_fstr);
      strcat(cat, rv->zh_fstr);
      delete[] rv->zh_fstr;
      rv->zh_fstr = cat;;

      rv->mml_nodes_last_term++;
      rv->is_signed = true;

      factors->next = NULL;
      DisposeFactorList(factors);
    }
  }
  return rv;
}

/*
       <mml:mi mathvariant="double-struck">C</mml:mi>
       <mml:mo>&InvisibleTimes;</mml:mo>
       <mml:mi mathvariant="double-struck">N</mml:mi>
       <mml:mo>&InvisibleTimes;</mml:mo>
       <mml:mi mathvariant="double-struck">Q</mml:mi>
       <mml:mo>&InvisibleTimes;</mml:mo>
       <mml:mi mathvariant="double-struck">R</mml:mi>
       <mml:mo>&InvisibleTimes;</mml:mo>
       <mml:mi mathvariant="double-struck">Z</mml:mi>
*/

// We record the MML source for things like variables.
// When final output is generated, a "source tree" is generated.
// Here we clean up the source.

//  char          src_tok[32];
//  tagMNODE*     parent;
//  tagMNODE*     first_child;
//  const char*   p_chdata;
//  ATTRIB_REC*   attrib_list;

MNODE* STree2MML::CleanupMMLsource(MNODE* mml_var_node)
{
  MNODE* rv = mml_var_node;

  if (mml_var_node) {
    // Convert msubsup to msub - as required
    if (ElementNameIs(mml_var_node, "msubsup")) {
      MNODE* child = mml_var_node->first_kid;
      if (child && child->next) {
        MNODE* sub = child->next;
        if (sub->next) {        // we have a superscript
          MNODE* sup = sub->next;
          DelinkTNode(sup);
          DisposeTNode(sup);
		  SetElementName(mml_var_node, "msub");
          //strcpy(mml_var_node->src_tok, "msub");
        }
      }
    }
    // Fix up the attributes
    if (up_mml_version == 1 && mml_var_node->attrib_list) {
      const char *mv_val =
        GetATTRIBvalue(mml_var_node->attrib_list, "mathvariant");
      if (mv_val) {
        char suffix[8];
        ATTRIB_REC *a_list = VariantToStyleAtts(mv_val, suffix);
        if (suffix[0]) {
          if (ElementNameIs(mml_var_node, "mi") && mml_var_node->p_chdata) {
            if (strlen(mml_var_node->p_chdata) == 1) {
              bool do_it = false;
              char ch = mml_var_node->p_chdata[0];
              if (!strcmp(suffix, "scr")) {
                if (ch >= 'A' && ch <= 'Z')
                  do_it = true;
              } else if (!strcmp(suffix, "opf")) {
                if (ch >= 'A' && ch <= 'Z')
                  do_it = true;
              } else if (!strcmp(suffix, "fr")) {
                if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'))
                  do_it = true;
              } else {
                TCI_ASSERT(0);
              }
              if (do_it) {
                delete[] mml_var_node->p_chdata;
                size_t zln = strlen(suffix) + 3;
                char *tmp = new char[zln + 1];
                tmp[0] = '&';
                tmp[1] = ch;
                strcpy(tmp + 2, suffix);
                tmp[zln - 1] = ';';
                tmp[zln] = 0;
                mml_var_node->p_chdata = tmp;
              }
            }
          }
        }
        mml_var_node->attrib_list =
          RemoveAttr(mml_var_node->attrib_list, "mathvariant");

        if (a_list)
          mml_var_node->attrib_list =
            MergeAttrsLists(mml_var_node->attrib_list, a_list);
      }
    }
    // add color as required
    if (up_clr_math_attr && *up_clr_math_attr) {
      ATTRIB_REC *ar = ExtractAttrs(up_clr_math_attr);
      mml_var_node->attrib_list =
        MergeAttrsLists(mml_var_node->attrib_list, ar);
    }
  } else {
    TCI_ASSERT(0);
  }
  return rv;
}

// "tmpl" contains 1, 2, or 3 mathml atoms that carry chdata,
//  <mi>,<mn>,<mo>,<mtext> - ie <mi%color_attr%>%chdata%</mi>.
//  Here we replace instances of%color_attr% with something
//  like mathcolor="red"

char *STree2MML::ColorizeMMLElement(const char *tmpl,
                                    const char *color_attr1,
                                    const char *color_attr2, const char *color_attr3)
{
  char *rv = NULL;

  if (tmpl) {
    size_t ln = strlen(tmpl);
    if (color_attr1)
      ln += strlen(color_attr1);
    if (color_attr2)
      ln += strlen(color_attr2);
    if (color_attr3)
      ln += strlen(color_attr3);
    rv = new char[ln + 1];
    strcpy(rv, tmpl);
    if (color_attr1)
      StrReplace(rv, ln, "%color_attr%", color_attr1);
    if (color_attr2)
      StrReplace(rv, ln, "%color_attr2%", color_attr2);
    if (color_attr3)
      StrReplace(rv, ln, "%color_attr3%", color_attr3);

    delete[] tmpl;
  }
  return rv;
}

/*
<mfrac>
  <mrow>
    %d_op%
    %expr%
  </mrow>
  %diff_denom%
</mfrac>
*/

void STree2MML::SemanticFracDerivative2MML(SEMANTICS_NODE * snode,
                                           FACTOR_REC * factor,
                                           bool use_dd, int& error_code)
{
  char *rv = NULL;

// compose the numerator - d^{2}F

//  <msup>                                  <mo form="prefix">&#x2202;</mo>
//    <mo form="prefix">&#x2202;</mo>       %expr%
//    <mn>d</mn>                        
//  </msup>
//  %expr%

  BUCKET_REC *expr_bucket = FindBucketRec(snode->bucket_list, MB_UNNAMED);

  char *z_expr = NULL;
  if (expr_bucket) {
    int nodes_made, terms_made;
    bool is_signed = false;
    z_expr = ProcessSemanticList(expr_bucket->first_child,
                                 error_code, nodes_made, terms_made,
                                 is_signed, NULL, NULL);
    if (z_expr)
      if (terms_made > 1 || nodes_made > 1)
        z_expr = NestzMMLInMROW(z_expr);
  }
  // Count the number of differentiations

  int n_diffs = 0;
  BUCKET_REC *b_rover = snode->bucket_list;
  while (b_rover) {
    if (b_rover->bucket_ID == MB_DIFF_VAR && b_rover->first_child) {
      SEMANTICS_NODE *s_var = b_rover->first_child;
      if (s_var->semantic_type == SEM_TYP_VARIABLE)
        n_diffs++;
      else if (s_var->semantic_type == SEM_TYP_POWERFORM) {
        BUCKET_REC *num_bucket =
          FindBucketRec(s_var->bucket_list, MB_SCRIPT_UPPER);
        if (num_bucket && num_bucket->first_child) {
          SEMANTICS_NODE *s_num = num_bucket->first_child;
          if (s_num->semantic_type == SEM_TYP_NUMBER) {
            n_diffs += atoi(s_num->contents);
          } else {
            TCI_ASSERT(0);
          }
        } else {
          TCI_ASSERT(0);
        }
      }
    }
    b_rover = b_rover->next;
  }

  char *diff_op;
  if (use_dd)
    diff_op = PrefixMOfromSTRING(up_differentiald);
  else
    diff_op = PrefixMOfromSTRING("&#x2202;"); // \partial

  char *z_dop = NULL;
  if (n_diffs > 1) {
    char exponent[32];
    StrFromInt(n_diffs, exponent);
    char *script = MNfromNUMBER(exponent);

    char *tmpl = GetTmplPtr(TMPL_MSUP); //  "<msup>\n%base%%script%</msup>\n"
    size_t zln = strlen(tmpl) + strlen(diff_op) + strlen(script);
    z_dop = new char[zln + 1];
    strcpy(z_dop, tmpl);
    StrReplace(z_dop, zln, "%base%", diff_op);
    StrReplace(z_dop, zln, "%script%", script);
    delete[] script;
  } else {
    size_t zln = strlen(diff_op);
    z_dop = new char[zln + 1];
    strcpy(z_dop, diff_op);
  }
  delete[] diff_op;

  char *z_dvar = DiffDenom2MML(snode, use_dd, error_code);

  char *tmpl = GetTmplPtr(TMPL_PARTIALD);
  size_t zln = strlen(tmpl);
  if (z_dop)
    zln += strlen(z_dop);
  if (z_expr)
    zln += strlen(z_expr);
  if (z_dvar)
    zln += strlen(z_dvar);

  rv = new char[zln + 1];
  strcpy(rv, tmpl);

  StrReplace(rv, zln, "%d_op%", z_dop);
  StrReplace(rv, zln, "%expr%", z_expr);
  StrReplace(rv, zln, "%diff_denom%", z_dvar);

  delete[] z_dop;
  delete[] z_expr;
  delete[] z_dvar;

  if (rv) {
    factor->zh_fstr = rv;
    factor->mml_nodes_last_term = 1;
  } else {
    error_code = 1;
  }
}

//    VARIABLE OF DIFFERENTATION
//      POWERFORM
//        SCRIPT_BASE
//          VARIABLE canonical_ID = "mit" contents = "t"
//        SCRIPT_UPPER 
//          NUMBER canonical_ID = "mn3" contents = "3"

char *STree2MML::DiffDenom2MML(SEMANTICS_NODE * snode,
                               bool use_dd, int& error_code)
{
  char *zh_rv = NULL;
  U32 buffer_ln = 0;

  char *diff_op;
  if (use_dd)
    diff_op = PrefixMOfromSTRING(up_differentiald);
  else
    diff_op = PrefixMOfromSTRING("&#x2202;"); // \partial

  BUCKET_REC *b_rover = snode->bucket_list;
  while (b_rover) {
    if (b_rover->bucket_ID == MB_DIFF_VAR) {
      char *z_dvar = NULL;
      if (b_rover->first_child) {
        int nodes_made, terms_made;
        bool is_signed = false;
        z_dvar = ProcessSemanticList(b_rover->first_child,
                                     error_code, nodes_made, terms_made,
                                     is_signed, NULL, NULL);
        if (z_dvar) {
          zh_rv = AppendStr2HeapStr(zh_rv, buffer_ln, diff_op);
          zh_rv = AppendStr2HeapStr(zh_rv, buffer_ln, z_dvar);
          delete[] z_dvar;
        }
      } else {
        TCI_ASSERT(0);
      }
    }
    b_rover = b_rover->next;
  }
  delete[] diff_op;

  zh_rv = NestzMMLInMROW(zh_rv);
  return zh_rv;
}

// MathML 2.0 uses the attribute mathvariant="Bold", etc.
// Mathml 1.0 uses suffixed entity names
//  or the attribute fontweight="bold", etc.

ATTRIB_REC *STree2MML::VariantToStyleAtts(const char *mathvariant,
                                          char *suffix)
{
  ATTRIB_REC *rv = NULL;
  suffix[0] = 0;

  size_t attr_val_ln = strlen(mathvariant);

  switch (attr_val_ln) {
  case 4:{                     // mathvariant =  "bold";
      rv = new ATTRIB_REC("fontweight", "bold");
      ATTRIB_REC *ar2 = new ATTRIB_REC("fontstyle", "upright");
      InsertAttribute(rv, ar2);
      //rv->next = ar2;
      //ar2->prev = rv;
    }
    break;
  case 6:
    if (!strcmp(mathvariant, "script")) { //  Calligraphic
      strcpy(suffix, "scr");
      rv = new ATTRIB_REC("fontstyle", "upright");
    } else if (!strcmp(mathvariant, "normal")) {
      rv = new ATTRIB_REC("fontweight", "normal");
    } else if (!strcmp(mathvariant, "italic")) {

    }
    break;
  case 7:                      // mathvariant =  "fraktur";
    strcpy(suffix, "fr");
    rv = new ATTRIB_REC("fontstyle", "upright");
    break;
  case 9:{                     // mathvariant =  "monospace";
      rv = new ATTRIB_REC("fontstyle", "italic");
    }
    break;
  case 10:{                    // mathvariant =  "sans-serif";
      rv = new ATTRIB_REC("fontstyle", "italic");
    }
    break;
  case 11:{                    // mathvariant =  "bold-italic";
      rv = new ATTRIB_REC("fontweight", "bold");
      ATTRIB_REC *ar2 = new ATTRIB_REC("fontstyle", "italic");
      InsertAttribute(rv, ar2);

      // rv->next = ar2;
      // ar2->prev = rv;
    }
    break;
  case 13:                     // mathvariant =  "double-struck";
    strcpy(suffix, "opf");
    rv = new ATTRIB_REC("fontstyle", "upright");
    break;

  default:
    TCI_ASSERT(0);
    break;
  }

  return rv;
}

bool STree2MML::IsBesselFunc(char *f_nom)
{
  bool rv = false;

  if (f_nom) {
    size_t zln = strlen(f_nom);
    if (zln == 7) {
      if (!strncmp(f_nom, "bessel", 6)
          || !strncmp(f_nom, "Bessel", 6))
        rv = true;
    }
  }

  return rv;
}

void STree2MML::BesselFunc2MML(SEMANTICS_NODE * s_function,
                               FACTOR_REC * factor, int& error_code)
{
  error_code = 0;

  BUCKET_REC *arg_list = s_function->bucket_list;
  if (arg_list
      && arg_list->next
      && arg_list->first_child && arg_list->next->first_child) {
    SEMANTICS_NODE *arg1 = arg_list->first_child;
    SEMANTICS_NODE *arg2 = arg_list->next->first_child;

    int nodes_made, terms_made;
    bool is_signed = false;
    char *z_arg1 = ProcessSemanticList(arg1, error_code, nodes_made,
                                       terms_made, is_signed, NULL, NULL);
    if (z_arg1) {
      if (terms_made > 1 || nodes_made > 1)
        z_arg1 = NestzMMLInMROW(z_arg1);
      char *z_arg2 = ProcessSemanticList(arg2, error_code, nodes_made,
                                         terms_made, is_signed, NULL, NULL);
      if (z_arg2) {
        if (terms_made > 1 || nodes_made > 1) {
          z_arg2 = NestzMMLInMROW(z_arg2);
        }
        int tmp_nodes_made;
        z_arg2 = NestzMMLInPARENS(z_arg2, tmp_nodes_made);
        if (tmp_nodes_made > 1)
          z_arg2 = NestzMMLInMROW(z_arg2);

        char *f_nom = s_function->contents;
        char lstr[2];
        lstr[0] = f_nom[6];
        lstr[1] = 0;

        char *tmpl = GetTmplPtr(TMPL_BESSEL);
        //  "<msub>\n<mi%color_attr%>Bessel%letter%</mi>\n%arg1%</msub>\n<mo>&#x2061;</mo>\n%arg2%",
        size_t ln = strlen(tmpl) + strlen(z_arg1) + strlen(z_arg2);
        char *zh_rv = new char[ln + 1];
        strcpy(zh_rv, tmpl);
        StrReplace(zh_rv, ln, "%letter%", lstr);
        StrReplace(zh_rv, ln, "%arg1%", z_arg1);
        StrReplace(zh_rv, ln, "%arg2%", z_arg2);
        zh_rv = ColorizeMMLElement(zh_rv, up_clr_func_attr, NULL, NULL);
        delete[] z_arg1;
        delete[] z_arg2;
        factor->mml_nodes_last_term = nodes_made;
        factor->zh_fstr = zh_rv;
      }
    }
  } else {
    TCI_ASSERT(0);
  }
}

// {sign}00001234567890.0123456789e{sign}1234567890

char *STree2MML::NumberToUserFormat(SEMANTICS_NODE * s_number)
{
  char *rv = NULL;

  if (s_number->contents) {
    char *ptr = s_number->contents;
    size_t zln = strlen(ptr);

    char *p_decimal = strchr(ptr, '.');
    if (!p_decimal) {
      rv = new char[zln + 1];
      strcpy(rv, ptr);
      return rv;
    }
    // Here, we have a decimal number
    int sign = 0;               // 0 - not present, 1 - '+', 2 - '-'
    if (*ptr == '+' || *ptr == '-') {
      sign = (*ptr == '+') ? 1 : 2;
      ptr++;
    }

    while (*ptr == '0')
      ptr++;                    // step over lead 0's

    char *zsig_digits = new char[zln + 1];
    int bi = 0;
    int shift = 0;
    bool after_decimal = false;

    char ch;
    while (ch = *ptr) {
      if ((ch >= '0') && (ch <= '9')) {
        zsig_digits[bi++] = ch;
        if (after_decimal)
          shift--;
      } else if (ch == '.') {
        if (after_decimal)
          TCI_ASSERT(0);
        after_decimal = true;
      } else if (ch == 'e') {
        int exp = atol(ptr + 1);
        shift += exp;
        break;
      } else {
        TCI_ASSERT(0);
      }
      ptr++;
    }

    if (!bi) {
      char *rv = new char[4];
      strcpy(rv, "0.0");
      delete[] zsig_digits;
      return rv;
    } else {
      zsig_digits[bi] = 0;
    }
    int n_digits = static_cast < int >(strlen(zsig_digits));

    if (n_digits > up_signif_digits) {
      char key = zsig_digits[up_signif_digits];
      zsig_digits[up_signif_digits] = 0;
      shift += n_digits - up_signif_digits;
      if (key >= '5')
        Roundup(zsig_digits);
      n_digits = up_signif_digits;
    }

    int decimal_pos = n_digits + shift;

    bool do_scientific = false;
    if (decimal_pos > 0) {
      if (decimal_pos > up_sn_upperthreshold)
        do_scientific = true;
    } else {
      if (decimal_pos <= -up_sn_lowerthreshold)
        do_scientific = true;
    }
    if (do_scientific) {
      // Here we position the decimal after the first digit
      shift += n_digits - 1;
      char tmp[80];
      StrFromInt(shift, tmp);
      size_t zln = strlen(tmp) + n_digits + 2;
      if (sign)
        zln++;
      rv = new char[zln + 1];
      size_t i = 0;
      if (sign)
        rv[i++] = sign == 1 ? '+' : '-';
      rv[i++] = zsig_digits[0];
      rv[i++] = '.';
      strcpy(rv + i, zsig_digits + 1);
      i += n_digits - 1;
      rv[i++] = 'e';
      strcpy(rv + i, tmp);
    } else {
      if (shift >= 0) {
        size_t zln = n_digits + 1 + shift;
        if (sign)
          zln++;
        rv = new char[zln + 1];
        size_t i = 0;
        if (sign)
          rv[i++] = sign == 1 ? '+' : '-';
        strcpy(rv + i, zsig_digits);
        i += n_digits;
        int j = 0;
        while (j < shift) {
          rv[i++] = '0';
          j++;
        }
        rv[i++] = '.';
        rv[i] = 0;
      } else {                  // shift is negative - to the left
        shift = -shift;
        int zln = n_digits + 1;
        if (sign)
          zln++;
        if (shift >= n_digits)  // need 0's in front of decimal
          zln += shift - n_digits + 1;
        rv = new char[zln + 1];
        int i = 0;
        if (sign)
          rv[i++] = sign == 1 ? '+' : '-';

        if (shift >= n_digits) {  // need 0's in front of decimal
          rv[i++] = '0';
          rv[i++] = '.';
          int tally = shift - n_digits;
          while (tally) {
            rv[i++] = '0';
            tally--;
          }
          strcpy(rv + i, zsig_digits);
        } else {
          int n_lead_digits = n_digits - shift;
          int j = 0;
          while (j < n_digits) {
            rv[i++] = zsig_digits[j];
            j++;
            if (j == n_lead_digits)
              rv[i++] = '.';
          }
          rv[i] = 0;
        }
      }
    }

    delete[] zsig_digits;
  } else {
    TCI_ASSERT(0);
  }
  return rv;
}

// "299" -> "300"

void STree2MML::Roundup(char *z_numstr)
{
  if (z_numstr && *z_numstr) {
    size_t zln = strlen(z_numstr);
    char *tmp = new char[zln + 2];
    int ti = 0;

    bool carry_one = true;

    int si = zln - 1;
    while (si >= 0) {
      char ch = z_numstr[si];
      if (carry_one) {
        if (ch < '9') {
          tmp[ti++] = ch + 1;
          carry_one = false;
        } else if (ch == '9') {
          tmp[ti++] = '0';
        } else
          TCI_ASSERT(0);
      } else {
        tmp[ti++] = ch;
      }
      si--;
    }
    // Copy the modified digits back to the original string
    ti--;
    int j = 0;
    while (j < zln) {
      z_numstr[j++] = tmp[ti];
      ti--;
    }
    z_numstr[zln] = 0;          // not necessary
    delete[] tmp;
  }
}

char *STree2MML::MNfromNUMBER(char *z_number)
{
  char *tmpl = GetTmplPtr(TMPL_MN);
  size_t ln = strlen(tmpl) + strlen(z_number);
  char *zh_rv = new char[ln + 1];
  strcpy(zh_rv, tmpl);
  StrReplace(zh_rv, ln, "%digits%", z_number);
  zh_rv = ColorizeMMLElement(zh_rv, up_clr_math_attr, NULL, NULL);

  return zh_rv;
}

char *STree2MML::MOfromSTRING(const char *z_string, const char *color)
{
  char *tmpl = GetTmplPtr(TMPL_MO);
  size_t ln = strlen(tmpl) + strlen(z_string);
  char *zh_rv = new char[ln + 1];
  strcpy(zh_rv, tmpl);
  StrReplace(zh_rv, ln, "%chdata%", z_string);
  zh_rv = ColorizeMMLElement(zh_rv, color, NULL, NULL);

  return zh_rv;
}

char *STree2MML::PrefixMOfromSTRING(char *z_string)
{
  //  "<mo%color_attr% %slant_att% form=\"prefix\">%chdata%</mo>\n",

  size_t sln = strlen(z_string);
  char *slant_attr = NULL;
//SLS. Bogus; what is meant?  if (sln == 1 && z_string[0] < 128)
  if (sln == 1)
    slant_attr = "fontstyle=\"italic\"";

  char *tmpl = GetTmplPtr(TMPL_PREFIXMO);
  size_t ln = strlen(tmpl) + strlen(z_string);
  if (slant_attr)
    ln += strlen(slant_attr);
  char *zh_rv = new char[ln + 1];
  strcpy(zh_rv, tmpl);
  StrReplace(zh_rv, ln, "%slant_att%", slant_attr);
  StrReplace(zh_rv, ln, "%chdata%", z_string);
  zh_rv = ColorizeMMLElement(zh_rv, up_clr_math_attr, NULL, NULL);

  return zh_rv;
}

char *STree2MML::AccentMOfromSTRING(char *z_string)
{
  //  "<mo%color_attr% form=\"postfix\" accent=\"true\">%chdata%</mo>\n",

  char *tmpl = GetTmplPtr(TMPL_ACCENTMO);
  size_t ln = strlen(tmpl) + strlen(z_string);
  char *zh_rv = new char[ln + 1];
  strcpy(zh_rv, tmpl);
  StrReplace(zh_rv, ln, "%chdata%", z_string);
  zh_rv = ColorizeMMLElement(zh_rv, up_clr_math_attr, NULL, NULL);

  return zh_rv;
}

char *STree2MML::MIfromCHDATA(const char *z_chdata)
{
  char *tmpl = GetTmplPtr(TMPL_MI);
  size_t ln = strlen(tmpl) + strlen(z_chdata);
  char *zh_rv = new char[ln + 1];
  strcpy(zh_rv, tmpl);
  StrReplace(zh_rv, ln, "%letters%", z_chdata);
  zh_rv = ColorizeMMLElement(zh_rv, up_clr_math_attr, NULL, NULL);

  return zh_rv;
}

char *STree2MML::GetPrimesStr(int nprimes)
{
  char *rv = NULL;

  if (nprimes < up_primes_as_n_threshold) {
    char *primeslist = NULL;
    U32 primes_ln = 0;

    int tally = nprimes;
    while (tally) {
      primeslist = AppendStr2HeapStr(primeslist, primes_ln, "&#x2032;");
      tally--;
    }

    char *tmpl = GetTmplPtr(TMPL_PRIMES);
    //  <mo form="postfix">%primes%</mo>
    size_t aln = strlen(tmpl);
    if (primeslist)
      aln += strlen(primeslist);
    rv = new char[aln + 1];
    strcpy(rv, tmpl);

    StrReplace(rv, aln, "%primes%", primeslist);
    rv = ColorizeMMLElement(rv, up_clr_math_attr, NULL, NULL);
    delete[] primeslist;
  } else {
    char exponent[32];
    StrFromInt(nprimes, exponent);
    rv = MNfromNUMBER(exponent);
    int tmp_nodes_made;
    rv = NestzMMLInPARENS(rv, tmp_nodes_made);
    if (tmp_nodes_made > 1)
      rv = NestzMMLInMROW(rv);
  }

  return rv;
}

bool STree2MML::DenomIsNumber(FACTOR_REC * bot_factors)
{
  bool rv = false;

  if (bot_factors
      && !bot_factors->next
      && bot_factors->n_terms < 2 && bot_factors->mml_nodes_last_term < 2) {
    if (bot_factors->is_number || bot_factors->is_constant)
      rv = true;
    else if (bot_factors->zh_fstr && *(bot_factors->zh_fstr)) {
      if (IsSimpleDenom(bot_factors->zh_fstr))
        rv = true;
    }
  }

  return rv;
}

FACTOR_REC *STree2MML::ExtractNumNumber(FACTOR_REC * top_factors,
                                        char **mml_num_str)
{
  *mml_num_str = NULL;

  FACTOR_REC *head = NULL;
  FACTOR_REC *tail;

  bool got_number = false;
  if (top_factors) {
    FACTOR_REC *rover = top_factors;
    while (rover) {
      FACTOR_REC *curr = rover;
      rover = rover->next;
      if (curr->is_number && !got_number) {
        *mml_num_str = curr->zh_fstr;
        curr->zh_fstr = NULL;
        curr->next = NULL;
        DisposeFactorList(curr);
        got_number = true;
      } else {
        if (!head)
          head = curr;
        else
          tail->next = curr;
        tail = curr;
        tail->next = NULL;
      }
    }
  }

  if (!got_number)
    *mml_num_str = MNfromNUMBER("1");

  return head;
}

int STree2MML::GetInfixPrecedence(SEMANTICS_NODE * s_group)
{
  int rv = 0;

  if (s_group && s_group->bucket_list && s_group->bucket_list->first_child) {
    SEMANTICS_NODE *s_rover = s_group->bucket_list->first_child;
    while (s_rover) {
      if (s_rover->semantic_type == SEM_TYP_INFIX_OP
          && s_rover->contents) {
        rv = s_rover->infix_precedence;
        break;
      }
      s_rover = s_rover->next;
    }
  }

  return rv;
}

bool STree2MML::IsTrigArgFunc(SEMANTICS_NODE * s_function)
{
  if (s_function && s_function->contents)
    return IsTrigArgFuncName(mml_entities,s_function->contents);
  else
    return false;
}

// When un-delimited arguments are scripted following a function (trigargs),
//  we may need to paren the entire function call to separate the args from
//  factors that follow.
// ie "sin(x)y" with trigargs on is written as "(sinx)y"

bool STree2MML::ParensRequired(SEMANTICS_NODE * s_function)
{
  bool rv = false;

  if (s_function->next
      && s_function->next->semantic_type == SEM_TYP_INFIX_OP) {
    SEMANTICS_NODE *following_op = s_function->next;
    if (!strcmp(following_op->contents, "&#xd7;") ||
        !strcmp(following_op->contents, "&#x2062;")) {
      // Here, another factor follows the function call

      bool nest_in_parens = true;
      if (following_op->next) {
        SEMANTICS_NODE *following_oparg = following_op->next;
        if (following_oparg->semantic_type == SEM_TYP_POWERFORM) {
          BUCKET_REC *base_bucket =
            FindBucketRec(following_oparg->bucket_list, MB_SCRIPT_BASE);
          if (base_bucket && base_bucket->first_child) {
            following_oparg = base_bucket->first_child;
          } else
            TCI_ASSERT(0);
        }
        if (following_oparg->semantic_type == SEM_TYP_FUNCTION
            && IsTrigArgFunc(following_oparg))
          nest_in_parens = false;
      }
      // Might look for "cos^2", etc.

      if (nest_in_parens)
        rv = true;
    }
  }

  return rv;
}

//  up_clr_math_attr =  " mathcolor=\"red\"";

void STree2MML::UPrefStr2MMLattr(char **dest, const char *new_value)
{
  delete[] *dest;
  *dest = NULL;
  // Note that we can generate an empty string "" here.

  if (new_value) {
    size_t zln = strlen(new_value);
    if (new_value[0] != ' ' && new_value[0] != 0) {
      *dest = new char[zln + 2];
      strcpy(*dest, " ");
      strcat(*dest, new_value);
    } else {
      *dest = new char[zln + 1];
      strcpy(*dest, new_value);
    }
  }
}

// from user_prefs

void STree2MML::SetUserPrefs(DefStore * ds)
{
  U32 pref_ID = 1;
  while (pref_ID < CLPF_last) {
    const char *pref_val = ds->GetPref(pref_ID, 0);
    if (!pref_val)
      pref_val = uprefs_store->GetPref(pref_ID);

    if (pref_val && *pref_val)
      SetUserPref(pref_ID, pref_val);
    else
      SetUserPref(pref_ID, "");

    pref_ID++;
  }
}

void STree2MML::SetUserPref(U32 which_one, const char *new_value)
{
  switch (which_one) {

  case CLPF_mml_version:       // 1
    up_mml_version = atoi(new_value);
    break;
  case CLPF_mml_prefix:        // output node prefix<uID2.0>
    if (new_value && *new_value && strlen(new_value) < 32) {
      strcpy(up_mml_prefix, new_value);
    } else
      up_mml_prefix[0] = 0;
    break;
  case CLPF_math_node_attrs:   // xmlns="http://www.w3.org/1998/Math/MathML"
    UPrefStr2MMLattr(&up_math_attrs, new_value);
    break;
  case CLPF_use_mfenced:       // 1
    up_use_mfenced = (new_value[0] == '0') ? false : true;
    break;

  case CLPF_clr_math_attr:     // clr_math_attr<uID5.0>
    UPrefStr2MMLattr(&up_clr_math_attr, new_value);
    break;
  case CLPF_clr_func_attr:     // mathcolor="gray"
    UPrefStr2MMLattr(&up_clr_func_attr, new_value);
    break;
  case CLPF_clr_text_attr:     // mathcolor="black"
    UPrefStr2MMLattr(&up_clr_text_attr, new_value);
    break;
  case CLPF_clr_unit_attr:     // mathcolor="green"
    UPrefStr2MMLattr(&up_unit_attrs, new_value);
    break;

  case CLPF_Sig_digits_rendered: // 6
    up_signif_digits = atoi(new_value);
    break;
  case CLPF_SciNote_lower_thresh:  // 1
    up_sn_lowerthreshold = atoi(new_value);
    break;
  case CLPF_SciNote_upper_thresh:  // 5
    up_sn_upperthreshold = atoi(new_value);
    break;

  case CLPF_Primes_as_n_thresh:  // 2
    up_primes_as_n_threshold = atoi(new_value);
    break;

  case CLPF_Parens_on_trigargs:  // 0
    up_parens_on_trigargs = new_value[0] == '0' ? false : true;
    break;
  case CLPF_log_is_base_e:     // 0
    up_log_is_base_e = new_value[0] == '0' ? false : true;
    break;

  case CLPF_Output_differentialD:  // D
    if (new_value && *new_value && strlen(new_value) < 16) {
      strcpy(up_differentialD, new_value);
    } else
      up_differentialD[0] = 'D';
    break;
  case CLPF_Output_differentiald:  // d
    if (new_value && *new_value && strlen(new_value) < 16) {
      strcpy(up_differentiald, new_value);
    } else
      strcpy(up_differentiald, "&#x2146;");
      //up_differentiald[0] = 'd';
    break;
  case CLPF_Output_Euler_e:    // e
    if (new_value && *new_value && strlen(new_value) < 16) {
      strcpy(up_nlogbase, new_value);
    } else
      up_nlogbase[0] = 'e';
    break;
  case CLPF_Output_imaginaryi: // i
    if (new_value && *new_value && strlen(new_value) < 16) {
      strcpy(up_imaginaryi, new_value);
    } else
      up_imaginaryi[0] = 'i';
    break;

  case CLPF_Output_InvTrigFuncs_1:
    up_InvTrigFuncs_1 = (atoi(new_value) != 0);
    break;

  case CLPF_Output_Mixed_Numbers:
    up_output_mixed_nums = new_value[0] == '0' ? false : true;
    break;

  case CLPF_Output_parens_mode:
    up_output_parens_mode = (atoi(new_value) != 0);
    break;

  case CLPF_Default_matrix_delims: {
    int val = atoi(new_value);
    switch (val) {
      case 1:
        up_matrix_delims = Delims_brackets;
        break;
      case 2:
        up_matrix_delims = Delims_parens;
        break;
      case 3:
        up_matrix_delims = Delims_braces;
        break;
      case 0:
      default:
        up_matrix_delims = Delims_none;
        break;
    }
    break;
  }
  case CLPF_Default_derivative_format: // 0 - none, 1 - 
    up_derivative_format = atoi(new_value);
    break;

  case CLPF_Dot_derivative:    // 1
    up_dot_is_derivative = new_value[0] == '0' ? false : true;
    break;
  case CLPF_Overbar_conjugate: // 1
    up_overbar_is_conjugate = new_value[0] == '0' ? false : true;
    break;

  default:
    break;
  }
}

char *STree2MML::GetVarsStr(SEMANTICS_NODE * s_diff)
{
  char *zh_rv = NULL;
  U32 buffer_ln = 0;

  char *invis_comma = MOfromSTRING("&#x2063;", up_clr_math_attr);

  int tally = 0;
  int nodes_made = 0;
  int terms_made = 0;
  BUCKET_REC *b_rover = s_diff->bucket_list;
  while (b_rover) {
    if (b_rover->bucket_ID == MB_DIFF_VAR) {
      char *z_dvar = NULL;
      U32 dvar_ln = 0;
      if (b_rover->first_child) {
        SEMANTICS_NODE *s_var_node = b_rover->first_child;

        // s_var_node is of type SEM_TYP_VARIABLE or SEM_TYP_POWERFORM (x^2)
        // If it's a power form, there are 2 options for the output,
        //  <msup><mi>x</mi><mn>2</mn></msup>
        // or
        //  <mrow><mi>x</mi><mo>,</mo><mi>x</mi></mrow>

        bool is_signed = false;
        int error_code = 0;
        bool done = false;
        if (s_var_node->semantic_type == SEM_TYP_POWERFORM) {
          BUCKET_REC *sup_bucket =
            FindBucketRec(s_var_node->bucket_list, MB_SCRIPT_UPPER);
          if (sup_bucket && sup_bucket->first_child) {
            SEMANTICS_NODE *s_cont = sup_bucket->first_child;
            if (s_cont->semantic_type == SEM_TYP_NUMBER) {
              int exp = atoi(s_cont->contents);
              if (exp && exp < 4) {
                BUCKET_REC *base_bucket =
                  FindBucketRec(s_var_node->bucket_list, MB_SCRIPT_BASE);
                SEMANTICS_NODE *s_bv = base_bucket->first_child;
                char *z_bv = ProcessSemanticList(s_bv, error_code,
                                                 nodes_made, terms_made,
                                                 is_signed, NULL, NULL);
                while (exp) {
                  if (dvar_ln)
                    z_dvar = AppendStr2HeapStr(z_dvar, dvar_ln, invis_comma);
                  z_dvar = AppendStr2HeapStr(z_dvar, dvar_ln, z_bv);
                  exp--;
                }
                delete[] z_bv;
                z_dvar = NestzMMLInMROW(z_dvar);
                done = true;
              }
            }
          } else {
            TCI_ASSERT(0);
          }
        }
        if (!done)
          z_dvar = ProcessSemanticList(s_var_node, error_code,
                                       nodes_made, terms_made,
                                       is_signed, NULL, NULL);
        if (z_dvar) {
          if (tally)
            zh_rv = AppendStr2HeapStr(zh_rv, buffer_ln, invis_comma);
          zh_rv = AppendStr2HeapStr(zh_rv, buffer_ln, z_dvar);
          delete[] z_dvar;
          tally++;
        }
      } else {
        TCI_ASSERT(0);
      }
    }
    b_rover = b_rover->next;
  }
  delete[] invis_comma;

  if (tally > 1 || nodes_made > 1 || terms_made > 1)
    zh_rv = NestzMMLInMROW(zh_rv);

  return zh_rv;
}

// For derivatives scripted as dots above.
char *STree2MML::GetDotsStr(int ndots)
{
  char *rv = NULL;

  char *ptr = NULL;
  if (ndots == 1)
    ptr = "&#x02d9;";           // &dot;    
  else if (ndots == 2)
    ptr = "&#x00a8;";           // &die;    
  else if (ndots == 3)
    ptr = "&#x20db;";           // &tdot;   
  else if (ndots == 4)
    ptr = "&#x20dc;";           // &DotDot; 
  else {
    TCI_ASSERT(0);
  }

  if (ptr)
    rv = AccentMOfromSTRING(ptr);

  return rv;
}

/*
LIMFUNC contents = "lim"
  .
  .
  LL DIRECTION
    NUMBER contents = "1"
*/

int STree2MML::GetLimDirection(BUCKET_REC * b_direction)
{
  int rv = 0;

  if (b_direction && b_direction->first_child) {
    SEMANTICS_NODE *s_number = b_direction->first_child;
    if (s_number->semantic_type == SEM_TYP_NUMBER
        && s_number->contents) {
      if (!strcmp(s_number->contents, "0"))
        rv = 0;
      else if (!strcmp(s_number->contents, "1"))
        rv = 1;
      else if (!strcmp(s_number->contents, "2"))
        rv = 2;
      else
        TCI_ASSERT(0);
    } else {
      TCI_ASSERT(0);
    }
  } else {
    TCI_ASSERT(0);
  }
  return rv;
}

char *STree2MML::LowerLim2MML(BUCKET_REC * b_parts)
{
  char *zh_rv = NULL;
  U32 zh_ln = 0;

  if (b_parts) {
    BUCKET_REC *b_variable = FindBucketRec(b_parts, MB_LL_VAR);
    BUCKET_REC *b_expr = FindBucketRec(b_parts, MB_LL_EXPR);

    if (b_variable && b_expr) {
      char *var_str = NULL;
      char *arrow_str = NULL;
      char *expr_str = NULL;

      if (b_variable->first_child) {
        SEMANTICS_NODE *s_var = b_variable->first_child;
        int error_code;
        int nodes_made;
        int terms_made;
        bool is_signed = false;
        var_str = ProcessSemanticList(s_var, error_code, nodes_made,
                                      terms_made, is_signed, NULL, NULL);
        if (var_str) {
          if (nodes_made > 1 || terms_made > 1 || is_signed)
            var_str = NestzMMLInMROW(var_str);
        }
      }

      arrow_str = MOfromSTRING("&#x2192;", up_clr_math_attr);

      if (b_expr->first_child) {
        SEMANTICS_NODE *s_expr = b_expr->first_child;
        int error_code;
        int nodes_made;
        int terms_made;
        bool is_signed = false;
        expr_str = ProcessSemanticList(s_expr, error_code, nodes_made,
                                       terms_made, is_signed, NULL, NULL);
        if (expr_str)
          if (nodes_made > 1 || terms_made > 1 || is_signed)
            expr_str = NestzMMLInMROW(expr_str);

        BUCKET_REC *b_direction = FindBucketRec(b_parts, MB_LL_DIRECTION);
        int direction = 0;
        if (b_direction)
          direction = GetLimDirection(b_direction);
        if (direction) {
          const char *direction_str = (direction == 1) ? "+" : "&#x2212;";
          char *script_str = MOfromSTRING(direction_str, up_clr_math_attr);

          char *tmpl = GetTmplPtr(TMPL_MSUP); //  "<msup>\n%base%%script%</msup>\n"
          size_t zln = strlen(tmpl) + strlen(expr_str) + strlen(script_str);
          char *tmp = new char[zln + 1];
          strcpy(tmp, tmpl);
          StrReplace(tmp, zln, "%base%", expr_str);
          StrReplace(tmp, zln, "%script%", script_str);
          delete[] expr_str;
          delete[] script_str;
          expr_str = tmp;
        }
      }

      zh_rv = AppendStr2HeapStr(zh_rv, zh_ln, var_str);
      zh_rv = AppendStr2HeapStr(zh_rv, zh_ln, arrow_str);
      zh_rv = AppendStr2HeapStr(zh_rv, zh_ln, expr_str);

      zh_rv = NestzMMLInMROW(zh_rv);
      delete[] var_str;
      delete[] arrow_str;
      delete[] expr_str;
    } else
      TCI_ASSERT(0);
  }

  return zh_rv;
}

// Code to decide whether to use a/b or 1/b * a

bool STree2MML::IsSimpleDenom(char *mml_markup)
{
  bool rv = false;

  // Construct a tree from the markup
  MNODE *tree = mml_tree_gen->MMLstr2Tree(mml_markup);

  DisposeTNode(tree);

  return rv;
}

char *STree2MML::NestInMath(char *body)
{
  char *zh_rv = NULL;

  if (body) {
    // nest the output in an <mrow>, if necessary
    if (strncmp(body, "<mml:mrow", 9)
        && strncmp(body, "<mrow", 5))
      body = NestzMMLInMROW(body);

    // nest the output in an <math>
    char *tmpl = GetTmplPtr(TMPL_MATH);
    //  "<math%attrs%>\n%body%</math>\n",

    size_t ln = strlen(tmpl) + strlen(body);
    if (up_math_attrs)
      ln += strlen(up_math_attrs);
    zh_rv = new char[ln + 1];
    strcpy(zh_rv, tmpl);
    StrReplace(zh_rv, ln, "%attrs%", up_math_attrs);
    StrReplace(zh_rv, ln, "%body%", body);
    delete[] body;
  }

  return zh_rv;
}
