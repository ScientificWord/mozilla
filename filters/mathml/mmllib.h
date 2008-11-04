  
#include "flttypes.h"

typedef bool (* LPRENDERTILE)(U32 type, const char* data);

typedef bool (* LPINITIALIZE)( const char* basepath,
                                         const char* name_space,
                                         const char** context_zstrs );
typedef U32 (* LPVERSION)(U16 major, U16 minor);
typedef U32 (* LPCONVERT)(const char* latex, LPRENDERTILE renderfunc, int ambient_size);

bool InitializeMML( const char* basepath,
                 const char* name_space,
                 const char** context_zstrs );
                 
bool Version(U16 major, U16 minor);
bool ConvertInline(const char* latex, LPRENDERTILE renderfunc, int ambient_size);
bool ConvertDisplay(const char* latex, LPRENDERTILE renderfunc, int ambient_size);


