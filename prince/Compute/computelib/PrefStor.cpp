// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

#include "PrefStor.h"
#include "CmpTypes.h"
#include <string.h>

// WARNING: the following strings MUST be 1-to-1 with the CLPF_
//  constants defined in "iCmpTypes.h"

const char *default_prefs[] = {
  "",
  "1",                                            // CLPF_mml_version
  "",                                             // CLPF_mml_prefix
  "xmlns=\"http://www.w3.org/1998/Math/MathML\"", // CLPF_math_node_attrs
  "0",                                            // CLPF_use_mfenced
  "",                                             // CLPF_clr_math_attr
  "mathcolor=\"gray\"",                           // CLPF_clr_func_attr
  "mathcolor=\"black\"",                          // CLPF_clr_text_attr
  "mathcolor=\"green\"",                          // CLPF_clr_unit_attr
  "5",                                            // CLPF_Sig_digits_rendered
  "1",                                            // CLPF_SciNote_lower_thresh
  "5",                                            // CLPF_SciNote_upper_thresh
  "4",                                            // CLPF_Primes_as_n_thresh
  "0",                                            // CLPF_Parens_on_trigargs
  "1",                                            // CLPF_log_is_base_e
  "&#x2145;",                                     // CLPF_Output_differentialD
  "&#x2146;",                                     // CLPF_Output_differentiald
  "&#x2147;",                                     // CLPF_Output_Euler_e
  "&#x2148;",                                     // CLPF_Output_imaginaryi
  "0",                                            // CLPF_Output_InvTrigFuncs_1
  "0",                                            // CLPF_Output_Mixed_Numbers
  "0",                                            // CLPF_Output_parens_mode
  "1",                                            // CLPF_Default_matrix_delims
  "0",                                            // CLPF_Default_derivative_format
  "1",                                            // CLPF_Dot_derivative
  "1",                                            // CLPF_Overbar_conjugate
  "1",                                            // CLPF_Input_i_Imaginary
  "0",                                            // CLPF_Input_j_Imaginary
  "1",                                            // CLPF_Input_e_Euler
  "<math><mi>x</mi><mo>,</mo><mi>y</mi><mo>,</mo><mi>z</mi></math>", //CLPF_Vector_basis
  "1",                                            // CLPF_Prime_derivative
  "1",                                            // CLPF_D_derivative
  0                                               // CLPF_last
};


PrefsStore::PrefsStore()
{
  for (int i = 0; i < CLPF_last; i++)
    prefs[i] = NULL;
}

PrefsStore::~PrefsStore()
{
  for (int i = 0;i < CLPF_last; i++)
    delete[] prefs[i];
}

const char* PrefsStore::GetPref(U32 pref_ID)
{
  const char* rv = NULL;

  if (pref_ID && pref_ID < CLPF_last) {
    if (prefs[pref_ID])
      rv = prefs[pref_ID];
    else
      rv = default_prefs[pref_ID];
  } else {
    TCI_ASSERT(!"pref_ID out of range.");
  }
  return rv;
}

int PrefsStore::SetPref(U32 pref_ID, const char* new_value)
{
  int rv = 0;

  if (pref_ID && pref_ID < CLPF_last) {
    delete[] prefs[pref_ID];
    prefs[pref_ID] = NULL;
    if (new_value) {
      size_t zln = strlen(new_value);
      char *tmp = new char[zln + 1];
      strcpy(tmp, new_value);
      prefs[pref_ID] = tmp;
    }
    rv = 1;
  } else {
    TCI_ASSERT(!"pref_ID out of range.");
  }
  return rv;
}
