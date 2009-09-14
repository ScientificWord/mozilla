// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

#ifndef CMPTYPES_H
#define CMPTYPES_H

#include "iCmpTypes.h"
#include "iCmpIDs.h"



enum MarkupType
{ MT_UNDEFINED, MT_MATHML, MT_LATEX,
  MT_MAPLEV_INPUT, MT_MUPAD_INPUT, MT_GRAPH
};

#define OUTFORMAT_MML                  1
#define OUTFORMAT_LATEX                2

class INPUT_NOTATION_REC
{
  public:
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
};

#endif
