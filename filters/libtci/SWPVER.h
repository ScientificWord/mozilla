#define SWPVERSION 5,50,0,2960
#define SWPVERSIONSTRING "5.50.0.2960\0"
#define SWPVERSION_DLLSTRING "550\0"
#define SWPVERSION_DIRSTRING "55"

#define LICENSE_VERSION "5.5"

#define SWP_COMPANYNAME "MacKichan Software\0"
#define SWP_COPYRIGHT "MacKichan Software, 2005\0"
#define SWP_LEGALCOPYRIGHT  "Copyright \251 " SWP_COMPANYNAME
#define SWP_TRADEMARKS "Scientific Word (R)     Scientific WorkPlace (R)     Scientific Notebook (R)\0"

#ifndef SWPVERS_CONSTDEFSONLY

#include <chamdefs.h>
#if CHAMPRODUCT == CHAMSWPPRO
  #define SWP_PRODUCTNAME   "Scientific Workplace"
#elif CHAMPRODUCT == CHAMSCIWORD
  #define SWP_PRODUCTNAME   "Scientific Word"
#elif CHAMPRODUCT == CHAMMONARCH
  #define SWP_PRODUCTNAME     "Scientific Notebook"
#elif CHAMPRODUCT == CHAMVIEWER
  #define SWP_PRODUCTNAME     "Scientific Viewer"
#else
  // this is an environment error, use default
  #define SWP_PRODUCTNAME     "Scientific Notebook"
#endif

#define SWP_DLLPREFIX     "sw"

#endif
