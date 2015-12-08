

#ifdef TESTING

#include <chmtypes.h>

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

#include <stdlib.h>


TCI_BOOL _Assert(char *strfile, unsigned uline){
  char buff[256];
  wsprintf(buff, "Assertion Failed: %s, line %u", strfile, uline);
  DWORD res =  MessageBox(NULL, buff, "Warning", MB_ABORTRETRYIGNORE | MB_APPLMODAL | MB_DEFBUTTON3 | MB_ICONSTOP);
  if (res == IDIGNORE)    //  ignore
    return FALSE;
   else if (res==IDRETRY)  // cause DebugBreak
    return TRUE;
   else     // IDABORT     // won't return
    abort();
  return FALSE;
}
#endif

