
#include "scalar.h"
#include "TCI_new.h"


Scalar xn_over_d( I32 x, I32 n, I32 d ){

# define LO15(l) 	(l&0x7FFF)
# define HI17(l) 	(((U32)(l) >> 15) & 0x1FFFFL)

  TCI_ASSERT(d != 0);
  if (d == 0)
    return 0;

  Scalar temp;

  //TCI_ASSERT( !( HI16(n) || HI16(d))  ); //Bad parameters in xn_over_d
  if ( HI16(n) || HI16(d) ){  //the bad case; we'll just proceed bitwise:
    TCI_BOOL neg = FALSE;
    if ( x<0 ){ x = -x;neg = !neg; }
    if ( n<0 ){ n = -n;neg = !neg; }
    if ( d<0 ){ d = -d;neg = !neg; }
    temp = 0L;
    //now multiply:
    U32 hix = HI16( x );
    U32 lox = LO16( x );
    U32 hin = HI16( n );
    U32 lon = LO16( n );
    U32 hi = hix * hin;
    U32 lo = hix * lon;
    hi += HI16( lo );
    lo &= 0x0000ffff;
    U32 lotemp = lox * hin;
    hi += HI16( lotemp );
    lotemp &= 0x0000ffff;
    lotemp += lo;
    lo = lox * lon;
    lotemp += HI16( lo );
    lo &= 0x0000ffff;
    hi += HI16( lotemp );
    lo += ((lotemp&0x0000ffff) << 16);
    //finally, divide:
    if ( hi < (U32)d ){
      U32 dividend = hi;
      U32 mask = 0x80000000;
      for ( I16 jj=31;jj>=-1;jj-- ){
        dividend = dividend<<1;
        if ( lo & 0x80000000 )    dividend++;
        lo = lo<<1;
        if ( (U32)d<=dividend ){
          if (jj==-1)   temp++;  //round the last digit
          else{
            temp |= mask;
            dividend -= d;
          }
        }
        mask = (mask>>1);
      }     //division loop
    } else {  //if hi >= d:
      TCI_ASSERT( FALSE );
    }
    if ( neg )     temp = -temp;
  } else {  //if n and d are small enough, use original algorithm
  // use Knuth's algorithm (p.44 TeX: TheProgram) to avoid overflow
    TCI_BOOL positive = x>0;
    temp = positive ? x : -x;
    U32 t,u,v,rem;
    t = LO15(temp)*n;
    u = HI17(temp)*n + HI17(t);
    // check for errors
    TCI_ASSERT( !HI17(u/d) ); //scalar overflow: xn_over_d
    if (HI17(u/d)){
      temp = 0;
    } else {
      v = ((u%d)<<15) + LO15(t);      // ??!!??
      u = ((u/d)<<15) + v/d;
      rem = ((v%d)<<1);
      if (rem>=U32(d))      u++;
      temp  =  positive ?  u  :  -Scalar(u);
    }
  }

  return temp;

# undef LO15
# undef HI17

}


void LongMult(Scalar x,Scalar y,U32& hi,U32& lo){
  TCI_BOOL negx = x<0;
  TCI_BOOL negy = y<0;
  if (negx)
    x = -x;
  if (negy)
    y = -y;
  U32 hix = U32(x>>16);
  x &= 0xffff;
  U32 hiy = U32(y>>16);
  y &= 0xffff;
  hi = hix * hiy;
  U32 templo = hix * y;
  templo += x * hiy;           
  hi += HI16(templo);
  templo &= 0xffff;
  templo = templo<<16;
  lo = x * y;
  if (templo >  ~lo )
    hi++;
//  templo += lo;
  lo += templo;
//  if (lo & 0x80000000){
//    lo &= 0x7fffffff;
//    hi++;
//  }
  if ((negx && !negy) || (!negx && negy)){
    hi = (~hi);
    lo = -lo;
  }
}


//this is an integer square root function
U32 LongSqRoot(U32 hi,U32 lo){
  I16 ix = 30;              //our current position in the arguments 
  U32 hirv = 0L;
  U32 rv = 0L;
  U32 hidividend = hi;
  U32 lodividend = lo;

  U32 trial = 0xc0000000;
  while (ix>=0 && !(trial & hi)) {
    trial = trial>>2;
    ix -= 2;
  }
//  if (ix>=0)
//    hirv = 1<<ix;

  U32 addmask = 0;
  U32* whichrv;
  while ((hidividend || lodividend) && ix>=-32) {
    if (ix<0){
      addmask = 1L<<(ix+32);
      whichrv = &rv;
    } else {
      addmask = 1L<<ix;
      whichrv = &hirv;
    }
    *whichrv += addmask;

    if (hirv>hidividend) {
      *whichrv -= addmask;
    } else if (hirv==hidividend) {
      if (rv>lodividend){
        *whichrv -= addmask;
      } else {
        hidividend = 0L;
        lodividend -= rv;
        *whichrv += addmask;
      } 
    } else { //so hirv < hidividend
      if (rv>lodividend)
        hidividend--;
      hidividend -= hirv;
      lodividend = U32(lodividend - rv);
      *whichrv += addmask;
    }
    ix -= 2;
    //now shift our trial "divisor"
    rv = rv>>1;
    if (hirv & 1)
      rv |= 0x80000000;
    hirv = hirv>>1;
  }
  if (ix>=-32){   //need to add trailing zeros
    ix = (ix+34)>>1;  
    rv = ((rv>>(ix-1))+1)>>1; //this is a rounding algorithm
    rv += (hirv<<(32-ix));
    hirv = hirv>>(ix); 
  }

  TCI_ASSERT( !hirv );  //shouldn't exist by the time we're done!!
  return rv;
}


#define SINE_TABLE {0L,0x1650L,0x2c74L,0x4242L,0x578fL,0x6c31L,0x8000L,0x92d6L,0xa48eL,0xb505L,0xc41bL,0xd1b4L,0xddb4L,0xe804L,0xf090L,0xf747L,0xfc1cL,0xff07L,0x10000L}
#define LOG10_TABLE {0L,0x2d14L,0x4d10L,0x65dfL,0x7a25L,0x8b48L,0x9a21L,0xa739L,0xb2f0L,0xbd88L,0xc735L,0xd01bL,0xd858L,0xe004L,0xe731L,0xedeeL,0xf449L,0xfa4cL,0x10000L}
#define EXP10_TABLE {0x10000L,0x127a0L,0x15562L,0x18a39L,0x1c73dL,0x20db4L,0x25f12L,0x2bd09L,0x3298bL,0x3a6d9L,0x4378bL,0x4dea3L,0x59f98L,0x67e6bL,0x77fbbL,0x8a8deL,0xa0000L}                                                     

//Pass in angle measured in (degrees,minutes), return quadratic interpolation of cosine
//in TeX form (0x10000 corresponds to 1)
Scalar TCICos( I16 degrees,U16 minutes ){
  return minutes ? TCISin(89-degrees,60-minutes) : TCISin(90-degrees);
}

  
//Pass in angle measured in (degrees,minutes), return quadratic interpolation of sine
//in TeX form (0x10000 corresponds to 1)
Scalar TCISin( I16 degrees,U16 minutes ){
  TCI_BOOL neg = FALSE;
  Scalar rv = 0L;
  if (degrees<0){
    neg = TRUE;
    degrees = -degrees;
  }
  Scalar totalmins = degrees * 60;
  totalmins += minutes;
  totalmins = totalmins % 21600;
  if (totalmins<0)
    totalmins += 21600;
  if (totalmins>=10800){
    neg = !neg;
    totalmins -= 10800;
  }
  if (totalmins>5400)
    totalmins = 10800 - totalmins;
  I16 index = I16(totalmins/300);
  Scalar sinetable[19] = SINE_TABLE;
  if (totalmins==5400){
    rv = 0x10000;
  } else if (totalmins==index*300) {
    TCI_ASSERT(index<19);
    rv = (Scalar)(sinetable[index]);
  } else {
    if (index>0 && totalmins-300*index < 150)
      index--;
    if (index>0)
      index--;
    if (index>17)
      index = 17;
    Scalar startval = 300 * index;
    Scalar xvals[3] = {startval,startval+300,startval+600};
    Scalar yvals[3] = {sinetable[index],sinetable[index+1],sinetable[index+2]};
    rv = PolyInterpolate( totalmins,3,xvals,yvals );
  } 
  if (neg)
    rv = -rv;
  return rv;
}


// 94.1208 by ron - NOTE: t is assumed to be a "TeX form" number; i.e., it represents a value
// of t/0x10000. Ideally should be used just to get fractional part of log; thus t should be
// between 0x10000 and 0xa0000 (representing a number between 1 and 10). Similarly, we'll return
// a "TeX form" number.
Scalar TCILog10( Scalar t ){
  if (t<0)       //how do we return error here??
    return 0x7fff;
  Scalar rv(0L);
  Scalar tempt(t);
  while (tempt>=0xa0000) {
    rv += 0x10000;
    tempt /= 10;
  }
  while (tempt<=0x10000) {
    rv -= 0x10000;
    tempt *= 10;
  }
  I16 index = I16(tempt >> 15); //amounts to increments of 0x8000
  index -= 2;                   //since table starting argument is 0x10000
  if (index>0)
    index--;
  if (index>16)
    index = 16;
  Scalar logtable[19] = LOG10_TABLE;
  Scalar startval = (index+2) * 0x8000L;
  Scalar xvals[3] = {startval,startval+0x8000,startval+0x10000};
  Scalar yvals[3] = {logtable[index],logtable[index+1],logtable[index+2]};
  rv += PolyInterpolate( tempt,3,xvals,yvals );
  return rv;
  
}

// 94.1208 by ron - NOTE: t is assumed to be a "TeX form" number; i.e., it represents a value
// of t/0x10000. Ideally should be used to exponentiate fractional part of a number, so t should
// be between 0 and 0x10000. Similarly, we'll return a "TeX form" number.
Scalar TCIExp10( Scalar t ){
  if (t>0x40000L)
    return 0x7fffffffL;
  else if (t<-0x50000L)
    return 0L;
    
  Scalar rv(1L);
  Scalar tempt(t);
  while (tempt>=0x10000) {
    rv *= 10;
    tempt -= 0x10000;
  }
  U16 index = U16(tempt >> 12); //amounts to increments of 0x1000
  if (index>0)
    index--;
  if (index>15)
    index = 15;
  Scalar exptable[17] = EXP10_TABLE;
  Scalar startval = index * 0x1000L;
  Scalar xvals[3] = {startval,startval+0x1000,startval+0x2000};
  Scalar yvals[3] = {exptable[index],exptable[index+1],exptable[index+2]};
  rv += PolyInterpolate( tempt,3,xvals,yvals );
  return rv;
  
}

Scalar PolyInterpolate( Scalar t,U16 numpts,Scalar *xvalues,Scalar *yvalues ){
  Scalar rv = 0;
  Scalar temp = 0L;
  for (U16 ii=0;ii<numpts;ii++) {
    temp = yvalues[ii];
    for (U16 jj=0;jj<numpts;jj++) {
      if (jj==ii)     continue;
      temp = xn_over_d(temp,t-xvalues[jj],xvalues[ii]-xvalues[jj]);
    }
    rv += temp;
  }
  return rv;
  
}

//The following two functions are intended to answer the question:
// round (or round up) a value in logical coordinates to the next value
// that would correspond to an integer number of device coordinates,
// where there are "scaledFactor" over "nScale" logical pixels for
// each device pixel. This was previously done assuming (or rather "using",
// since it was known that the assumption was frequently wrong) that the
// ratio between the two systes was an integer, but this is no longer safe
// with print device contexts, particularly when zooming.
Scalar RoundToScale( Scalar nValue, Scalar scaledFactor, Scalar nScale )
{
  if (!scaledFactor || !nScale)
  {
    TCI_ASSERT(FALSE);
    return nValue;  //return it unchanged
  }
  Scalar convertedVal = xn_over_d(nValue, nScale, scaledFactor);
  return xn_over_d( convertedVal, scaledFactor, nScale );
}

Scalar RoundUpToScale( Scalar nValue, Scalar scaledFactor, Scalar nScale )
{
  if (!scaledFactor || !nScale)
  {
    TCI_ASSERT(FALSE);
    return nValue;  //return it unchanged
  }
  Scalar convertedVal = xn_over_d(nValue, nScale, scaledFactor);
  double actualValue = (double)nValue * (double)nScale/(double)scaledFactor;
  
  if (convertedVal < 0)
  {
    if (actualValue < convertedVal)
      convertedVal--;
  }    
  else
  {
    if (actualValue > convertedVal)
      convertedVal++;  
  }  
  return xn_over_d( convertedVal, scaledFactor, nScale ); 
}

Scalar RoundDownToScale( Scalar nValue, Scalar scaledFactor, Scalar nScale )
{
  if (!scaledFactor || !nScale)
  {
    TCI_ASSERT(FALSE);
    return nValue;  //return it unchanged
  }
  Scalar convertedVal = xn_over_d(nValue, nScale, scaledFactor);
  double actualValue = (double)nValue * (double)nScale/(double)scaledFactor;
  
  if (convertedVal < 0)
  {
    if (actualValue > convertedVal)
      convertedVal++;
  }    
  else
  {
    if (actualValue < convertedVal)
      convertedVal--;  
  }  
  return xn_over_d( convertedVal, scaledFactor, nScale ); 
}
