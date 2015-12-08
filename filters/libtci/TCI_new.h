#ifndef TCI_NEW_H
#define TCI_NEW_H

//Everyone needs chamdefs -- TCIENV clauses will silently work without this.
#ifndef CHAMDEFS_H
  #include <chamdefs.h>
#endif
#ifndef CHMTYPES_H
  #include <chmtypes.h>
#endif

#ifdef __cplusplus
  #ifdef TESTING
    #ifdef NO_SMRTHEAP
	  // for file name/line number tracking
	  void* __cdecl operator new(unsigned int nSize, const char* lpszFileName, int nLine);
	  void __cdecl operator delete(void *p, const char* lpszFileName, int nLine);
	  #define TCI_NEW(a) new(THIS_FILE,__LINE__) a
    #else
	  #define TCI_NEW(a) new a
    #endif
  #else
	  #define TCI_NEW(a) new a
  #endif
#endif


#ifdef TESTING

#define TCIDebugBreak() _asm { int 3 }

TCI_BOOL _Assert(char*, unsigned); //Prototype

extern TCI_BOOL bSuppressAssertions;

#define TCI_ASSERT(f) do \
        { \
        if (!(f) && !(bSuppressAssertions) && _Assert(__FILE__, __LINE__)) \
                TCIDebugBreak(); \
        } while (0)

#define TCI_VERIFY(f) TCI_ASSERT(f)

#else        // not TESTING

#define TCI_VERIFY(f) (void)(f)
#define TCI_ASSERT(f) 0

#endif


#define HEAP_CHECK 
#define HEAP_SETDEBUG_MAX 
#define HEAP_SETDEBUG_NORMAL 

#endif
