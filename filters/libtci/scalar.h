
#ifndef SCALAR_H
#define SCALAR_H

#ifndef CHMTYPES_H
  #include "chmtypes.h"
#endif

typedef I32 Scalar;
//extern Scalar xn_over_d( I32 x, I32 n, I32 d );
  Scalar xn_over_d( I32 x, I32 n, I32 d );

void LongMult(Scalar x,Scalar y,U32& hi,U32& lo);
U32 LongSqRoot(U32 hi,U32 lo);
Scalar PolyInterpolate( Scalar t,U16 numpts,Scalar *xvalues,Scalar *yvalues );
Scalar TCISin(I16 degrees,U16 minutes=0);
Scalar TCICos(I16 degrees,U16 minutes=0);
Scalar TCIExp10( Scalar t );
Scalar TCILog10( Scalar t );

Scalar RoundToScale( Scalar nValue, Scalar scaledFactor, Scalar nScale=0x10000 );
Scalar RoundUpToScale( Scalar nValue, Scalar scaledFactor, Scalar nScale=0x10000 );
Scalar RoundDownToScale( Scalar nValue, Scalar scaledFactor, Scalar nScale=0x10000 );

#endif
