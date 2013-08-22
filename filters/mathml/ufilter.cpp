
#include "ufilter.h"
#include "logfiler.h"
#include "treegen.h"
#include "mmltiler.h"


#include <string.h>

#ifdef TESTING
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/*

This object is used in 2 distinct contexts.

1) In a testjig program that converts the math objects from a .tex file
   into a mathml.

2) As a component of a DLL used by "Export to HTML".
   In this context, a new MMLFilter is created for each
   internal math object encountered.

   
MMLFilter orchestrates
 the parsing of source LaTeX,
 the creation of a tree from source data,
 the creation of a corresponding MathML tree,
 and the creation of a list of tiles from the MML tree.

The code to generate lines of output from the tiles is in this file.

*/


//ctor
MMLFilter::MMLFilter( TreeGenerator* lparser ) {

  logfiler  =  NULL;
  converter =  lparser;
  mml_tiler =  TCI_NEW( MMLTiler(converter->GetMathGrammar(false) ) );
  start_indent  =  0;
  indent_inc    =  0;
}


//dtor
MMLFilter::~MMLFilter() {

  CheckTNODETallies();		// diagnostics in FLTUTILS

  if ( logfiler )   delete logfiler;
  if ( mml_tiler )  delete mml_tiler;
}


void MMLFilter::FInitialize( USERPREFS* userprefs,
							 const char* new_log_file,
							 char* name_space,
							 U16 starting_indent,
							 char* dump_file_spec ) {

  start_indent  =  starting_indent;

  if ( userprefs ) {
    indent_inc  =  atoi( (char*)userprefs->indent_increment );
    if ( mml_tiler ) {
      mml_tiler->SetNameSpace( userprefs->namespace_prefix );
      mml_tiler->SetEntityMode( userprefs->entity_mode );
    } else
      TCI_ASSERT(0);
  } else {
    TCI_ASSERT(0);
    mml_tiler->SetNameSpace( (U8*)name_space );
  }

  if ( logfiler ) {
    delete logfiler;
    logfiler  =  NULL;
  }
  if ( new_log_file )
    logfiler  =  TCI_NEW( LogFiler(new_log_file) );

  FUSetDumpFile( dump_file_spec );

  converter->Reset( userprefs,logfiler,dump_file_spec );

  if ( mml_tiler ) {
    mml_tiler->SetLogFiler( logfiler );
  }
}



U16 MMLFilter::TranslateBuffer( const char* srcbuffer,
							    FILTRENDERTILE renderfunc ) {

  U16 msgcount  =  0;
  ANOMALY_REC* anomaly_list =  TCI_NEW( ANOMALY_REC );
  anomaly_list->next   =  NULL;
  anomaly_list->ilk    =  0;
  anomaly_list->atext  =  NULL;
  anomaly_list->off1   =  0;
  U8 eqn_option;


  TNODE* XML_tree =  (TNODE*)converter->BufferToDestTree( 
	  			const_cast<U8*>(reinterpret_cast<const U8*>(srcbuffer)),
   				FALSE,msgcount,anomaly_list,eqn_option );

  // leave out top level <mrow>
  if (XML_tree && (NULL == strcmp((const char*) XML_tree->zuID, "5.750.1"))){
    XML_tree = XML_tree -> parts -> contents;
  }
    


  MMLTreeToRenderer( XML_tree,anomaly_list,
	  			const_cast<U8*>(reinterpret_cast<const U8*>(srcbuffer)),
  					renderfunc );

  DisposeTList( XML_tree );

// Dispose the anomaly list.

  ANOMALY_REC* a_rover  =  anomaly_list;
  while ( a_rover ) {
    ANOMALY_REC* del  =  a_rover;
	a_rover =  a_rover->next;
	if ( del->atext )
	  delete del->atext;
    delete del;
  }
  return 0;
}



void MMLFilter::MMLTreeToRenderer( TNODE* mml_tree,
  								ANOMALY_REC* anomaly_list,
								U8* src_LaTeX,
								FILTRENDERTILE renderfunc ) {
  TILE* tiles  =  mml_tiler->MMLtreeToTiles( mml_tree,0 );

  //JBMLine("End MMLTreeToRenderer\n");
//char* eoln_seq        =  "\n";
  char* eoln_seq        =  NULL;
  RenderMML( tiles,start_indent,indent_inc,
				  eoln_seq,anomaly_list,src_LaTeX,renderfunc );

  DisposeTILEs( (TILE*)tiles );
  //JBMLine("MMLTreeToRenderer\n");
}



// XML output to rendering function

void MMLFilter::RenderMML( TILE* tiles,U16 start_indent,
						    U16 indent_increment,char* eoln_seq,
							ANOMALY_REC* anomaly_list,U8* src_LaTeX,
						    FILTRENDERTILE renderfunc ) {

  //JBMLine("RenderMML\n");
  U32 dest_lineno =  0;

  char* line  =  TCI_NEW( char[32768] );

// send anomaly reports

  ANOMALY_REC* a_rover  =  anomaly_list;
  while ( a_rover ) {
    if ( a_rover->ilk ) {
	  line[0] =  0;
      if      ( a_rover->atext )
	    strcpy( line,(char*)a_rover->atext );
	  else if ( a_rover->off1 ) {
	    U32 ln  =  a_rover->off2 - a_rover->off1;
	    if ( ln < 32768 ) {
	      strncpy( line,(char*)src_LaTeX+a_rover->off1,ln );
	      line[ln] =  0;
	    }
	  }
      (*renderfunc)( a_rover->ilk,line );
	}

    a_rover =  a_rover->next;
  }


  if ( tiles ) {
    U16 li  =  0;

    // ambiant indent is passed in!
    U16 curr_indent =  start_indent;

    // indent the first line, if necessary
    if ( curr_indent )
		while ( li<curr_indent ) line[li++]  =  ' ';


    TCI_BOOL first_tile =  TRUE;
    TILE* rover =  tiles;
    while ( rover ) {				// loop thru tiles

      switch ( rover->ilk ) {

      case TT_VAR_VALUE :
      case TT_A_ELEM_HEADER	:			// tiles that start
		  case TT_S_ELEM_HEADER	: {			//  a new line
		    if ( li && !first_tile ) {		
      	  if ( eoln_seq ) strcat( line,eoln_seq );
            dest_lineno++;				// output current line
            (*renderfunc)( rover->ilk,line );
			  li =  0;				  		// indent next line
			  while ( li<curr_indent ) line[li++]  =  ' ';
			}
          strcpy( line+li,(char*)rover->zval );
          li  =  strlen( line );

          if ( rover->ilk == TT_S_ELEM_HEADER )
            curr_indent +=  indent_increment;
		  }
		  break;

        case TT_ATOM_BODY		: {
		    //line[li++]  =  ' ';		no space after headtag
          if ( rover->zval ) {
    // Implement line breaking here!
			  U16 vlen  =  strlen( (char*)rover->zval );
			  if ( vlen+li < 32767 )
              strcpy( line+li,(char*)rover->zval );
			  else
			    TCI_ASSERT(0);
			}
          li  =  strlen( line );
          //if ( rover->zval ) {
			//  (*renderfunc)( 2,(char*)rover->zval );
          //}
        }
		  break;

        case TT_A_ELEM_ENDER	: {
		    //line[li++]  =  ' ';		no space before tailtag
          strcpy( line+li,(char*)rover->zval );
          li  =  strlen( line );
          //(*renderfunc)( 3,(char*)rover->zval );
    	  }
    	  break;

        case TT_S_ELEM_ENDER	: {
		    if ( li ) {
		      if ( !first_tile ) {		// output current line
      	    if ( eoln_seq ) strcat( line,eoln_seq );
              dest_lineno++;
              (*renderfunc)( 4,line );
			  }
			  curr_indent -=  indent_increment;
			  li =  0;				  	// indent next line
			  while ( li<curr_indent ) line[li++]  =  ' ';
			}
          strcpy( line+li,(char*)rover->zval );
          li  =  strlen( line );
		  }
		  break;

        case TT_ALIGNGROUP	: {
		    if ( li && !first_tile ) {		// output current line
      	  if ( eoln_seq ) strcat( line,eoln_seq );
            dest_lineno++;
            (*renderfunc)( 5,line );
			  li =  0;				  		// indent next line
			  while ( li<curr_indent ) line[li++]  =  ' ';
			}
          strcpy( line+li,(char*)rover->zval );
          li  =  strlen( line );
    	  }
		  break;

		  default :
		    TCI_ASSERT(0);
		  break;
      }

      first_tile  =  FALSE;
      rover =  rover->next;
    }

    if ( li ) {
	  line[li]  =  0;
      if ( eoln_seq ) strcat( line,eoln_seq );
      dest_lineno++;
      (*renderfunc)( 5,line );
    }

  }
  //JBMLine("RenderMML\n");

  delete[] line;

}



// Call filter->FInitialize first.

U16 MMLFilter::TranslateFile( const char* srcfile_spec,
                                char* dstfile_spec,
                                char* srcstr,char* eoln ) {

  U16 rv  =  1;         // no errors logged (yet)

  if ( srcstr )
    srcstr[0]   =  0;         // This is a zstring that the user sees
  TCI_BOOL show_src =  TRUE;  //  as static text in our dialog app.
  U32 src_show_off  =  9999L; // Invalid starting value

  U16 result;
  converter->SetNewFile( srcfile_spec,0,1,result,NULL,0 );



  FILE* srcfile =  fopen( srcfile_spec,"r" );
  if ( srcfile ) {

    U16 msgcount  =  0;
    ANOMALY_REC* anomaly_list =  TCI_NEW( ANOMALY_REC );
    anomaly_list->next   =  NULL;
    anomaly_list->ilk    =  0;
    anomaly_list->atext  =  NULL;
    anomaly_list->off1   =  0;


    U8 srcbuffer[1024];
    fgets( (char*)srcbuffer,1000,srcfile );
    U8 eqn_option;

    TNODE* mml_tree =  (TNODE*)converter->BufferToDestTree( 
            const_cast<U8*>(reinterpret_cast<const U8*>(srcbuffer)),
   				  FALSE,msgcount,anomaly_list,eqn_option );
  


    FILE* dstfile =  fopen( dstfile_spec,"w" );
    if ( dstfile ) {
      U16 indent_increment  =  2;
      char eoln_seq[8];
      U16 di  =  0;
      if ( eoln ) {
        char* ptr =  eoln;
        char ch;
        while ( ch = *ptr ) {
          if      ( ch == 'n' )
            eoln_seq[di++]  =  '\n';
          else if ( ch == 'r' )
            eoln_seq[di++]  =  '\r';
          else
            TCI_ASSERT(0);
          ptr++;
        }
      }
      eoln_seq[di]  =  0;

      MMLTreeToDstfile( mml_tree,dstfile,indent_increment,eoln_seq );
      fclose( dstfile );
    }

    DisposeTList( mml_tree );

// Dispose the anomaly list.

    ANOMALY_REC* a_rover  =  anomaly_list;
    while ( a_rover ) {
      ANOMALY_REC* del  =  a_rover;
	    a_rover =  a_rover->next;
	    if ( del->atext )
	      delete del->atext;
      delete del;
    }

    fclose( srcfile );
  }


  return rv;
}


void MMLFilter::MMLTreeToDstfile( TNODE* mml_tree,
							   	    FILE* dstfile,
                                    U16 indent_increment,
							   	    char* eoln_seq ) {

  TILE* tiles  =  mml_tiler->MMLtreeToTiles( mml_tree,0 );


  char* line  =  TCI_NEW( char[32768] );
  U16 li  =  0;

  U16 curr_indent =  0;


  TCI_BOOL first_tile =  TRUE;
  TILE* rover =  tiles;
  while ( rover ) {				// loop thru tiles

    switch ( rover->ilk ) {

      case TT_A_ELEM_HEADER	:			// tiles that start
	  case TT_S_ELEM_HEADER	: {			//  a new line
	    if ( li && !first_tile ) {		
    	  if ( eoln_seq ) strcat( line,eoln_seq );
          fputs( line,dstfile );
		  li =  0;				  		// indent next line
		  while ( li<curr_indent ) line[li++]  =  ' ';
		}
        strcpy( line+li,(char*)rover->zval );
        li  =  strlen( line );

        if ( rover->ilk == TT_S_ELEM_HEADER )
		  curr_indent +=  indent_increment;
	  }
	  break;

      case TT_ATOM_BODY		: {
        if ( rover->zval ) {
// Implement line breaking here!
		  U16 vlen  =  strlen( (char*)rover->zval );
		  if ( vlen+li < 32767 )
            strcpy( line+li,(char*)rover->zval );
		  else
		    TCI_ASSERT(0);
		}
        li  =  strlen( line );
	  }
	  break;

      case TT_A_ELEM_ENDER	: {
        strcpy( line+li,(char*)rover->zval );
        li  =  strlen( line );
  	  }
  	  break;

      case TT_S_ELEM_ENDER	: {
	    if ( li ) {
	      if ( !first_tile ) {		// output current line
    	    if ( eoln_seq ) strcat( line,eoln_seq );
            fputs( line,dstfile );
		  }
		  curr_indent -=  indent_increment;
		  li =  0;				  	// indent next line
		  while ( li<curr_indent ) line[li++]  =  ' ';
		}
        strcpy( line+li,(char*)rover->zval );
        li  =  strlen( line );
	  }
	  break;

      case TT_ALIGNGROUP	: {
	    if ( li && !first_tile ) {		// output current line
    	  if ( eoln_seq ) strcat( line,eoln_seq );
          fputs( line,dstfile );
		  li =  0;				  		// indent next line
		  while ( li<curr_indent ) line[li++]  =  ' ';
		}
        strcpy( line+li,(char*)rover->zval );
        li  =  strlen( line );
  	  }
	  break;

	  default :
	    TCI_ASSERT(0);
	  break;
    }

    first_tile  =  FALSE;
    rover =  rover->next;
  }

  if ( li ) {
    if ( eoln_seq ) strcat( line,eoln_seq );
    fputs( line,dstfile );
  }

  DisposeTILEs( (TILE*)tiles );

  delete line;
}

