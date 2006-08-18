  
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the MMLDLL_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// MMLDLL_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef MMLDLL_EXPORTS
#define MMLDLL_API __declspec(dllexport)
#else
#define MMLDLL_API __declspec(dllimport)
#endif


extern "C" {
typedef BOOL (CALLBACK* LPRENDERTILE)(DWORD type, const char* data);

typedef HRESULT (__cdecl* LPINITIALIZE)( const char* basepath,
                                         const char* name_space,
                                         const char** context_zstrs );
typedef HRESULT (__cdecl* LPVERSION)(WORD major, WORD minor);
typedef HRESULT (__cdecl* LPCONVERT)(const char* latex, LPRENDERTILE renderfunc, int ambient_size);

MMLDLL_API HRESULT __cdecl Initialize( const char* basepath,
                                       const char* name_space,
                                       const char** context_zstrs );
MMLDLL_API HRESULT __cdecl Version(WORD major, WORD minor);
MMLDLL_API HRESULT __cdecl ConvertInline(const char* latex, LPRENDERTILE renderfunc, int ambient_size);
MMLDLL_API HRESULT __cdecl ConvertDisplay(const char* latex, LPRENDERTILE renderfunc, int ambient_size);


}