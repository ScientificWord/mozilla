#ifndef __MKT_linkage_h__
#define __MKT_linkage_h__

#ifdef WIN32

#ifdef MKT_DLL
#ifdef MKT_IMPLEMENT_DLL
#  define MKT_DECL_LIB __declspec( dllexport )
#else // import, ie. use DLL
#  define MKT_DECL_LIB __declspec( dllimport )
#endif
#else // static lib
#  define MKT_DECL_LIB
#endif // !MKT_DLL

#else // !WIN32

// support static linking only for now
#  define MKT_DECL_LIB

#endif // !WIN32

#endif // __MKT_linkage_h__
