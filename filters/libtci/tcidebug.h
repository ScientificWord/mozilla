
#ifndef TCIDEBUG_H
#define TCIDEBUG_H


#if 0
  #define TCI_TRACE(a) fprintf(stderr, a); \
                       fprintf(stderr, " (%s:%d)\n", __FILE__,__LINE__);
  #define TCI_TRACE1(a,b) fprintf(stderr, a, b); \
                       fprintf(stderr, " (%s:%d)\n", __FILE__,__LINE__);
  #define TCI_TRACE2(a,b,c) fprintf(stderr, a, b, c); \
                       fprintf(stderr, " (%s:%d)\n", __FILE__,__LINE__);
#else
  #define TCI_TRACE(a)
  #define TCI_TRACE1(a,b)
  #define TCI_TRACE2(a,b,c)
#endif


#endif
