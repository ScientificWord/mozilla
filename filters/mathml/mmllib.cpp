
#include "mmllib.h"

#include "flttypes.h"
//#include <filespec.h>
#include "fltutils.h"
#include "treegen.h"
#include "ufilter.h"
#include <string.h>

#ifdef TESTING
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

long int clients;
USERPREFS userprefs;
TreeGenerator* tree_generator;

//
//BOOL APIENTRY DllMain( HANDLE hModule, 
//                       DWORD  ul_reason_for_call, 
//                       LPVOID lpReserved
//					 )
//{
//  switch (ul_reason_for_call) {
//	case DLL_PROCESS_ATTACH:
//      if (InterlockedIncrement(&clients) > 1) {
//        InterlockedDecrement(&clients);
//        return FALSE;
//      }
//      break;
//	case DLL_PROCESS_DETACH:
//      delete tree_generator;
//
//      if ( userprefs.namespace_prefix )
//        delete userprefs.namespace_prefix;
//      if ( userprefs.mstyle_attrs )
//        delete userprefs.mstyle_attrs;
//      if ( userprefs.mathname_attrs )
//        delete userprefs.mathname_attrs;
//      if ( userprefs.unitname_attrs )
//        delete userprefs.unitname_attrs;
//      if ( userprefs.text_in_math_attrs )
//        delete userprefs.text_in_math_attrs;
//      if ( userprefs.link_attrs )
//        delete userprefs.link_attrs;
//      if ( userprefs.renderer_baselines )
//        delete userprefs.renderer_baselines;
//      if ( userprefs.eqn_tags_to_mlabeledtr )
//        delete userprefs.eqn_tags_to_mlabeledtr;
//      if ( userprefs.eqn_nums_to_mlabeledtr )
//        delete userprefs.eqn_nums_to_mlabeledtr;
//      if ( userprefs.entity_mode )
//        delete userprefs.entity_mode;
//      if ( userprefs.lr_spacing_mode )
//        delete userprefs.lr_spacing_mode;
//      if ( userprefs.lr_spacing_mode_in_scripts )
//        delete userprefs.lr_spacing_mode_in_scripts;
//      if ( userprefs.eqn_nums_format )
//        delete userprefs.eqn_nums_format;
//      if ( userprefs.long_arrows_are_stretched )
//        delete userprefs.long_arrows_are_stretched;
//      if ( userprefs.indent_increment )
//        delete userprefs.indent_increment;
//      if ( userprefs.adjust_output_for_IE_spacing_bug )
//        delete userprefs.adjust_output_for_IE_spacing_bug;
//
//      InterlockedDecrement(&clients);
//      break;
//
//	case DLL_THREAD_ATTACH:
//	case DLL_THREAD_DETACH:
//	return FALSE;
//  }
//
//  return TRUE;
//}


//FileSpec fsNBgmr;
//FileSpec fsMMLgmr;
//FileSpec log;

  const char* fsNBgmr = "NOTEBOOK.gmr";
  const char* fsMMLgmr = "MATHML.gmr";
  const char* log = "mmlDLL.log";

U8 n_space[32];


U8* GetZStr( const char* src_zstr ) {

  U8* rv  =  NULL;

  if ( src_zstr ) {
    U16 zln =  strlen( src_zstr );
    //if ( zln ) {
      rv  =  new U8[zln+1];
	  strcpy( (char*)rv,src_zstr );
   // }
  }

  return rv;
}


// Initialize the LaTeX - MathML converter
bool InitializeMML( const char* datapath,
                 const char* name_space,
                 const char** upref_zstrs )
{
  memset( &userprefs, 0, sizeof(USERPREFS) );

  userprefs.output_mode = 4;  // bare MathML
  
  userprefs.mml_version                 =  atoi(    upref_zstrs[0] );
  userprefs.start_display_math          =  GetZStr( upref_zstrs[1] );   //<mml:math class="displayedmathml" mode="display" display="block">
  userprefs.start_inline_math           =  GetZStr( upref_zstrs[2] );   //<mml:math class="inlinemathml" display="inline">
  userprefs.namespace_prefix            =  GetZStr( upref_zstrs[3] );   // "mml:",
  userprefs.mstyle_attrs                =  GetZStr( upref_zstrs[4] );   // "",
  userprefs.mathname_attrs              =  GetZStr( upref_zstrs[5] );   // "mathcolor=\"gray\"",
  userprefs.unitname_attrs              =  GetZStr( upref_zstrs[6] );   // "mathcolor=\"green\"",
  userprefs.text_in_math_attrs          =  GetZStr( upref_zstrs[7] );   // "mathcolor=\"black\"",
  userprefs.link_attrs                  =  GetZStr( upref_zstrs[8] );   // "mathcolor=\"green\"",
  userprefs.renderer_baselines          =  GetZStr( upref_zstrs[9] );   // "true",
  userprefs.eqn_tags_to_mlabeledtr      =  GetZStr( upref_zstrs[10] );   // "false",
  userprefs.eqn_nums_to_mlabeledtr      =  GetZStr( upref_zstrs[11] );  // "false",
  userprefs.entity_mode                 =  GetZStr( upref_zstrs[12] );  // "0",
  userprefs.lr_spacing_mode             =  GetZStr( upref_zstrs[13] );  // "1",
  userprefs.lr_spacing_mode_in_scripts  =  GetZStr( upref_zstrs[14] );  // "1",
  userprefs.eqn_nums_format             =  GetZStr( upref_zstrs[15] );  // "(%theequation%)",
  userprefs.long_arrows_are_stretched   =  GetZStr( upref_zstrs[16] );  // "false",
  userprefs.indent_increment            =  GetZStr( upref_zstrs[17] );  // "2",
  userprefs.adjust_output_for_IE_spacing_bug  =  GetZStr( upref_zstrs[18] );  // "true",
  userprefs.end_math                    =  GetZStr( upref_zstrs[19] );  // "true",


  //FileSpec DLLdir(datapath);
  
  //fsNBgmr   =  DLLdir;  fsNBgmr  += "NOTEBOOK.gmr";
  //fsMMLgmr  =  DLLdir;  fsMMLgmr += "MATHML.gmr";
  //log       =  DLLdir;  log      += "mmlDLL.log";
  
  tree_generator   =  new TreeGenerator( 1, fsNBgmr, NULL, fsMMLgmr );

  tree_generator->Reset( &userprefs,NULL,NULL );

  int do_xml  =  1;
  U16 entity_mode =  1;
  U16 result;
  tree_generator->OnStartNewFile( do_xml,entity_mode,result,upref_zstrs,0 );

  if ( name_space ) {
    if ( strlen(name_space) < 32 )
      strcpy( (char*)n_space,name_space );
    else
      TCI_ASSERT(0);
  } else
    n_space[0]  =  0;
  
  return true;
}


// Set the MathML version number
bool Version(U16 major, U16 minor)
{
  userprefs.mml_version = major;
	return true;
}


// Convert null-terminated LaTeX content to (inline) MathML

bool ConvertInline(const char* latex,
                   LPRENDERTILE renderfunc,
                   int start_indent ) {

  MMLFilter* filter =  TCI_NEW( MMLFilter(tree_generator) );
  filter->FInitialize( &userprefs, log, (char*)n_space, start_indent, NULL );

  U16 parse_result  =  filter->TranslateBuffer( latex, renderfunc );
  
  delete filter;
  return true;
}


// Convert null-terminated LaTeX content to (display) MathML

bool ConvertDisplay(const char* latex,
                    LPRENDERTILE renderfunc,
                    int start_indent ) {

  MMLFilter* filter =  TCI_NEW( MMLFilter(tree_generator) );
  filter->FInitialize( &userprefs, log, (char*)n_space, start_indent, NULL );

  U16 parse_result  =  filter->TranslateBuffer( latex, renderfunc );
  
  delete filter;
  return true;
}
