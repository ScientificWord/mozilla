
#ifdef TESTING

#include <chmtypes.h>
#include <TCI_new.h>

#define NOGDICAPMASKS     
#define NOVIRTUALKEYCODES 
#define NOWINMESSAGES     
#define NOWINSTYLES       
#define NOSYSMETRICS      
#define NOMENUS           
#define NOICONS           
#define NOKEYSTATES       
#define NOSYSCOMMANDS     
#define NORASTEROPS       
#define NOSHOWWINDOW      
#define OEMRESOURCE       
#define NOATOM            
#define NOCLIPBOARD       
#define NOCOLOR           
#define NOCTLMGR          
#define NODRAWTEXT        
#define NOGDI             
#define NOKERNEL          
//#define NOUSER            
#define NONLS             
//#define NOMB              
#define NOMEMMGR          
#define NOMETAFILE        
#define NOMINMAX          
#define NOMSG             
#define NOOPENFILE        
#define NOSCROLL          
#define NOSERVICE         
#define NOSOUND           
#define NOTEXTMETRIC      
#define NOWH              
#define NOWINOFFSETS      
#define NOCOMM            
#define NOKANJI           
#define NOHELP            
#define NOPROFILER        
#define NODEFERWINDOWPOS  
#define NOMCX             
#include <windows.h>
#include <crtdbg.h>

#ifdef NO_SMRTHEAP
// for file name/line number tracking
void* __cdecl operator new(unsigned int nSize, const char* lpszFileName, int nLine)
{
		return( _malloc_dbg(nSize, _NORMAL_BLOCK, lpszFileName, nLine) );
}
void __cdecl operator delete(void *p, const char* lpszFileName, int nLine)
{
	::operator delete(p);
}
#endif

#endif