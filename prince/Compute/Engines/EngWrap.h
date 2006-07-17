// Copyright (c) 2006 MacKichan Software, Inc.  All Rights Reserved.

#ifndef EngWrap_h
#define EngWrap_h

//IMO MapleWrapper and MuPADWrapper should be derived from a base class (see all the duplicated code),
//    even if they are not meant to be used polymorphically.

//Anyway, the following capture some of the shared definitions used in the two implementations.

enum ContextType {
  CC_UNKNOWN,
  CC_EVAL,
  CC_EVAL_NUMERIC,
  CC_DEFINITION,
  CC_ODE,
  CC_ODE_CONDITION,
  CC_UNDEFINE,
  CC_IMPLICIT_DIFF,
  CC_FACTOR_IN_RING,
  CC_HOLDINTEGRAL,
  CC_APPROXINTEGRAL,
  CC_PLOTFUNC
};

enum ContextTemplateType {
  CT_UNKNOWN,
  CT_NORMAL,
  CT_EQN2MAT,
  // CT_SOLVE,
  // CT_PASSTHRU,
  CT_SOLVEEXACT,
  // CT_SOLVESYSTEM,
  // CT_RSOLVE,
  CT_COLLECT,
  // CT_POLYDIV,
  CT_RANDMATRIX,
  CT_RANDOMNUM,

  CT_RESHAPE,
  CT_EXPRandVAR,
  CT_POLYSORT,
  CT_POLYROOTS,
  CT_EXTREMA,
  CT_TRANSFORM,
  CT_VCALC,
  CT_PSERIES,

  CT_ODESERIES,
  CT_QUANTILE,
  CT_MOMENT,
  CT_MATRIXMINPOL,
  CT_COMPANIONMAT,
  CT_WRONSKIAN,
  CT_CHARACTERISTICP,
  CT_ITERATE,
  CT_CALCBYPARTS,
  CT_CALCCHANGEVAR,
  CT_CALCAPPROXINT,
  CT_FITCURVE,
  CT_FILLMATRIX,
  CT_MAPENTRIES,
  CT_PLOTFUNC,
  CT_PLOTQUERY
};

enum ContextDataType {
  CD_Real,
  CD_Complex
};

#endif
