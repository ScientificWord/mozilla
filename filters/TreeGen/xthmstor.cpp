
#include "nthmstor.h"
#include "grammar.h"

#include "tci_new.h"
#include <string.h>
#include <stdlib.h>

#ifdef TESTING
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/* 
  Notebook.gmr has some built-in \newtheorem's.  When this object
is constructed, we put these notebook environments in the store.
They are objects with uIDs starting at 5.309.0 and going to 5.328.0
that are defined in notebook.gmr.

  Note that in LaTeX, \newtheorem defs are global.
*/

NewObjectStore::NewObjectStore( Grammar* s_grammar ) {
//JBMLine( "NewObjectStore::ctor\n" );

  new_theorems  =  (THEOREM_REC*)NULL;
  new_environs  =  (ENVIRON_REC*)NULL;

// Initialize the newtheorem style stack

  ts_sp   =  0;
  U16 sp  =  0;
  while ( sp<4 ) {
    theo_stack[sp].ts_style     =  NULL;
    theo_stack[sp].ts_bodyfont  =  NULL;
	sp++;
  }
  headerfont  =  NULL;

// s_grammar may predefine some "\newtheorem", as does notebook.gmr.
// We push the built-in newtheorems at scope NT_SCOPE_GRAMMAR, ie.
//  defined at the level of the grammar.

  U8  zname[80];			// note that this name is itself a zuID
  U8* zcurr_env =  (U8*)"TEXT";

  U16 uobjtype  =  5;
  U16 usubtype  =  309;
  U16 uID       =  0;
  while ( usubtype < 329 ) {    // built-in newtheorem ID range, 309<=ID<=328

// notebook.gmr has lines as follows:
// 5.309.0<uID5.309.0>theorem,,Theorem,

    UidsTozuID( uobjtype,usubtype,uID,zname );
    U16 name_ln =  strlen( (char*)zname );

    U8* zuID;
    U8* ztemplate;
    if ( s_grammar->GetGrammarDataFromNameAndAttrs(
    			zname,name_ln,NULL,zcurr_env,&zuID,&ztemplate) ) {
/*
char zzz[256];
sprintf( zzz,"zname=%s -> zuID=%s, ztemplate=%s\n",
(char*)zname,(char*)zuID,(char*)ztemplate );
JBMLine( zzz );
*/
      U8* nom     =  NULL;
      U8* n_like  =  NULL;
      U8* caption =  NULL;
      U8* in      =  NULL;

      U8 buffer[128];
      buffer[0] =  0;
      strcpy( (char*)buffer,(char*)ztemplate );

      U8* ptr =  buffer;
      if ( *ptr != 0 ) {
        if ( *ptr != ',' ) {
          nom =  ptr;
          ptr =  (U8*)strchr( (char*)ptr,',' );
        }
        if ( ptr && *ptr != 0 ) {
          *ptr  =  0;
          ptr++;
          if ( *ptr != ',' ) {
            n_like  =  ptr;
            ptr =  (U8*)strchr( (char*)ptr,',' );
          }
          if ( ptr && *ptr != 0 ) {
            *ptr  =  0;
            ptr++;
            if ( *ptr != ',' ) {
              caption =  ptr;
              ptr =  (U8*)strchr( (char*)ptr,',' );
            }
            if ( ptr && *ptr != 0 ) {
              *ptr  =  0;
              ptr++;
              if ( *ptr != 0 )
                in  =  ptr;
            }
          }
        }
      }
      AddNewTheorem( NT_SCOPE_GRAMMAR,nom,caption,n_like,in,FALSE );
    }

    usubtype++;
  }			//  while ( usubtype < 329 )


// LaTeX has many built-in environments. quote verse center etc.
// We treat these the same as envs defined by \newenvironment,
//  except that they stored at scope NT_SCOPE_GRAMMAR. 
// Their names and zuIDs come from notebook.gmr.

  uobjtype  =  5;
  usubtype  =  509;
  uID       =  1;
  while ( usubtype < 529 ) {    // built-in environs ID range, 509<=ID<=528

// notebook.gmr has lines as follows
// 5.509.1<uID5.509.1>abstract,0,,,

    UidsTozuID( uobjtype,usubtype,uID,zname );
    U16 name_ln =  strlen( (char*)zname );

    U8* zuID;
    U8* ztemplate;
    if ( s_grammar->GetGrammarDataFromNameAndAttrs(
    			zname,name_ln,NULL,zcurr_env,&zuID,&ztemplate) ) {
/*
char zzz[256];
sprintf( zzz,"zname=%s -> zuID=%s, ztemplate=%s\n",
(char*)zname,(char*)zuID,(char*)ztemplate );
JBMLine( zzz );
*/
      U8* nom     =  NULL;
      U8* n_args  =  NULL;
      U8* opt_def =  NULL;
      U8* begdef  =  NULL;
      U8* enddef  =  NULL;

      U8 buffer[256];
      buffer[0] =  0;
      strcpy( (char*)buffer,(char*)ztemplate );

      U8* ptr =  buffer;
      if ( *ptr != 0 ) {
        if ( *ptr != ',' ) {
          nom =  ptr;
          ptr =  (U8*)strchr( (char*)ptr,',' );
        }
        if ( ptr && *ptr != 0 ) {
          *ptr  =  0;
          ptr++;
          if ( *ptr != ',' ) {
            n_args  =  ptr;
            ptr =  (U8*)strchr( (char*)ptr,',' );
          }
          if ( ptr && *ptr != 0 ) {
            *ptr  =  0;
            ptr++;
            if ( *ptr != ',' ) {
              opt_def =  ptr;
              ptr =  (U8*)strchr( (char*)ptr,',' );
            }
            if ( ptr && *ptr != 0 ) {
              *ptr  =  0;
              ptr++;
              if ( *ptr != 0 ) {
                begdef  =  ptr;
                ptr =  (U8*)strchr( (char*)ptr,',' );
              }
              if ( ptr && *ptr != 0 ) {
                *ptr  =  0;
                ptr++;
                if ( *ptr != 0 )
                  enddef  =  ptr;
              }
            }
          }
        }
      }

      AddNewEnviron( NT_SCOPE_GRAMMAR,nom,n_args,
      					opt_def,begdef,enddef );

    }		// s_grammar lookup OK

    usubtype++;
  }			//  while ( usubtype < 329 )


  counters  =  (COUNTER_INFO*)NULL;
  InitCounters();
}


NewObjectStore::~NewObjectStore() {

  DisposeCounters();

  ClearTheorems( NT_SCOPE_GRAMMAR );	// clears everything
  ClearEnvirons( NT_SCOPE_GRAMMAR );

  // Clear the newtheorem style stack

  TCI_ASSERT( ts_sp == 0 );
  U16 sp  =  0;
  while ( sp<4 ) {
    if ( theo_stack[sp].ts_style )    delete theo_stack[sp].ts_style;
    if ( theo_stack[sp].ts_bodyfont ) delete theo_stack[sp].ts_bodyfont;
	sp++;
  }
  if ( headerfont )
    delete headerfont;

}


U8* NewObjectStore::GetTheoremInfo( U8* targ_env,U16 which_info ) {
//char zzz[80];
//sprintf( zzz,"GetTheoremInfo, targ_env=%s\n",targ_env );
//JBMLine( zzz );

  U8* rv  =  NULL;

  THEOREM_REC*  rover =  new_theorems;
  while ( rover ) {
/*
char zzz[80];
sprintf( zzz,"  rover_env=%s\n",rover->env_name );
JBMLine( zzz );
*/
    if ( !strcmp((char*)targ_env,(char*)rover->env_name) ) {

      if      ( which_info == TRI_envname   )
        rv  =  rover->env_name;
      else if ( which_info == TRI_caption   )
        rv  =  rover->caption;
      else if ( which_info == TRI_nums_like )
        rv  =  rover->nums_like;
      else if ( which_info == TRI_within    )
        rv  =  rover->within;
      else if ( which_info == TRI_tmpl      )
        rv  =  rover->tmpl;
      else if ( which_info == TRI_style     )
        rv  =  rover->style;
      else if ( which_info == TRI_bodyfont  )
        rv  =  rover->bodyfont;
      else if ( which_info == TRI_starred  )
        rv  =  rover->is_starred ? (U8*)"T" : (U8*)"F";
      else 
        TCI_ASSERT( 0 );

      break;
    } else
      rover =  rover->next;
  }

  return rv;
}

// We maintain a list "new_theorems" of all envs defined by
//  \newtheorem.  Each \newtheorem cmd pushes an entry on the
//  front of the list.  We don't care if the same env is defined
//  more than once.  Info is retrieved from the list by linear
//  search most-recent to least-recent.  Thus we always get
//  info from the most-recent definition.

void NewObjectStore::AddNewTheorem( U16 scope,U8* nom,U8* def,
                                    	U8* n_like,U8* in,
                                    		TCI_BOOL isstarred ) {

  THEOREM_REC* n_theorem  =
    	(THEOREM_REC*)TCI_NEW(char[sizeof(THEOREM_REC)] );

  n_theorem->scope  =  scope;
  n_theorem->is_starred =  isstarred;

  U16 nom_len =  strlen( (char*)nom );
  U8* tmp =  (U8*)TCI_NEW(char[ nom_len+1 ] );
  strcpy( (char*)tmp,(char*)nom );
  n_theorem->env_name =  tmp;

/*
char zzz[80];
sprintf( zzz,"AddNewTheorem, scope=%d\n",scope );
JBMLine( zzz );
JBMLine( (char*)tmp );
JBMLine( "\n" );
*/

  U16 def_len =  strlen( (char*)def );
  tmp =  (U8*)TCI_NEW(char[ def_len+1 ] );
  strcpy( (char*)tmp,(char*)def );
  n_theorem->caption  =  tmp;

//JBMLine( (char*)tmp );
//JBMLine( "\n" );

  if ( n_like ) {
    U16 like_len  =  strlen( (char*)n_like );
    tmp =  (U8*)TCI_NEW(char[ like_len+1 ] );
    strcpy( (char*)tmp,(char*)n_like );
    n_theorem->nums_like  =  tmp;

//JBMLine( (char*)tmp );
//JBMLine( "\n" );

  } else
    n_theorem->nums_like  =  NULL;

  if ( in ) {
    U16 in_len  =  strlen( (char*)in );
    tmp =  (U8*)TCI_NEW(char[ in_len+1 ] );
    strcpy( (char*)tmp,(char*)in );
    n_theorem->within =  tmp;

//JBMLine( (char*)tmp );
//JBMLine( "\n" );

  } else
    n_theorem->within =  NULL;

  n_theorem->tmpl =  MakeThmTemplate( nom );

  n_theorem->style    =  GetTheoremStyleAttr( TA_style );
  n_theorem->bodyfont =  GetTheoremStyleAttr( TA_bodyfont );

// Add at head of list - we are pushing onto a stack here!

  n_theorem->next =  new_theorems;
  new_theorems    =  n_theorem;
}


/* The following public function allows the parent Grammar to clear
  newtheorem defs from NewObjectStore.  Scope 0 NT_SCOPE_GRAMMAR
  holds grammar level newtheorem defs (as in notebook.gmr)
  and scope 1 NT_SCOPE_DOCUMENT holds document level newtheorem
  defs as are possible in a LaTeX document.  Note that we are
  creating a new instance of NewObjectStore for each file filtered
  in the present implementation.  Hence we don't really need
  this as public.
*/

void NewObjectStore::ClearTheorems( U16 scope ) {

  THEOREM_REC*  new_list  =  NULL;
  THEOREM_REC*  tail;

  THEOREM_REC*  del;
  THEOREM_REC*  rover =  new_theorems;
  while ( rover ) {
    del   =  rover;
    rover =  rover->next;
    if ( del->scope >= scope ) {
      if ( del->env_name ) {
//JBMLine( (char*)del->env_name );
//JBMLine( "\n" );
        delete del->env_name;
      }
      if ( del->caption ) {
//JBMLine( (char*)del->caption );
//JBMLine( "\n" );
        delete del->caption;
      }
      if ( del->nums_like ) {
//JBMLine( (char*)del->nums_like );
//JBMLine( "\n" );
        delete del->nums_like;
	  }
      if ( del->within ) {
//JBMLine( (char*)del->within );
//JBMLine( "\n" );
        delete del->within;
	  }
      if ( del->tmpl ) {
//JBMLine( (char*)del->tmpl );
//JBMLine( "\n" );
        delete del->tmpl;
	  }
      if ( del->style ) {
//JBMLine( (char*)del->style );
//JBMLine( "\n" );
        delete del->style;
	  }
      if ( del->bodyfont ) {
//JBMLine( (char*)del->bodyfont );
//JBMLine( "\n" );
        delete del->bodyfont;
	  }
      delete del;

    } else {            // add this node to the new list
      if ( !new_list )  new_list    =  del;
      else              tail->next  =  del;

      tail  =  del;
      tail->next  =  NULL;
    }

  }		// while ( rover )

  new_theorems  =  new_list;
}


void NewObjectStore::ClearEnvirons( U16 scope ) {

  ENVIRON_REC*  new_list  =  NULL;
  ENVIRON_REC*  tail;

  ENVIRON_REC*  del;
  ENVIRON_REC*  rover =  new_environs;
  while ( rover ) {
    del   =  rover;
    rover =  rover->next;
    if ( del->scope >= scope ) {
      if ( del->env_name ) {
//JBMLine( (char*)del->env_name );
//JBMLine( "\n" );
        delete del->env_name;
      }
      if ( del->arg_count ) {
//JBMLine( (char*)del->arg_count );
//JBMLine( "\n" );
        delete del->arg_count;
      }
      if ( del->begdef )  delete del->begdef;
      if ( del->enddef )  delete del->enddef;
      if ( del->tmpl )    delete del->tmpl;
      delete del;

    } else {            // add this node to the new list
      if ( !new_list )  new_list    =  del;
      else              tail->next  =  del;

      tail  =  del;
      tail->next  =  NULL;
    }

  }		// while ( rover )

  new_environs  =  new_list;
}


// Private function to build a template ( grammar production )
//  for a newtheorem.

U8* NewObjectStore::MakeThmTemplate( U8* nom ) {

  U8 buffer[256];

  strcpy( (char*)buffer,"!\\begin{" );
  strcat( (char*)buffer,(char*)nom );
  strcat( (char*)buffer,"}!OPTPARAM(5.329.2,TEXT)BUCKET(5.329.3,TEXT,,,\\end{" );
  strcat( (char*)buffer,(char*)nom );
  strcat( (char*)buffer,"},)!\\end{" );
  strcat( (char*)buffer,(char*)nom );
  strcat( (char*)buffer,"}!" );

  U16 len =  strlen( (char*)buffer );
  U8* rv  =  (U8*)TCI_NEW(char[ len+1 ] );
  strcpy( (char*)rv,(char*)buffer );

/*
JBMLine( (char*)rv );
JBMLine( "\n" );
*/

  return rv;
}


U8* NewObjectStore::GetEnvironInfo( U8* targ_env,U16 which_info ) {
/*
char zzz[80];
sprintf( zzz,"GetEnvironInfo, targ_env=%s\n",targ_env );
JBMLine( zzz );
*/

  U8* rv  =  NULL;

  ENVIRON_REC*  rover =  new_environs;
  while ( rover ) {
/*
char zzz[80];
sprintf( zzz,"  rover_env=%s\n",rover->env_name );
JBMLine( zzz );
*/
    if ( !strcmp((char*)targ_env,(char*)rover->env_name) ) {

      if      ( which_info == ERI_num_args  )
        rv  =  rover->arg_count;
      else if ( which_info == ERI_optdef    )
        rv  =  rover->opt_def;
      else if ( which_info == ERI_begdef    )
        rv  =  rover->begdef;
      else if ( which_info == ERI_enddef    )
        rv  =  rover->enddef;
      else if ( which_info == ERI_tmpl      )
        rv  =  rover->tmpl;
      else if ( which_info == ERI_is_newenv )
        rv  =  rover->scope > 0 ? (U8*)"t" : (U8*)"f";
      else 
        TCI_ASSERT( 0 );
      break;
    } else
      rover =  rover->next;
  }

  // This is the last resort for finding a definition of the object
  //  encountered in the input stream.  If "\begin{whatever}" is not
  //  defined, we gin a definition here!

  if ( rv == NULL && which_info == ERI_tmpl ) {

// logfiler	 - should log an error here!

    U16 scope   =  NT_SCOPE_DOCUMENT;
    U8* n_args  =  (U8*)"1";			// I'm allowing for an optional
  	U8* opt_def =  (U8*)"dummy";		//  param here - no harm.
  	U8* begdef  =  NULL;
  	U8* enddef  =  NULL;
    AddNewEnviron( scope,targ_env,n_args,opt_def,begdef,enddef );

    rv  =  GetEnvironInfo( targ_env,ERI_tmpl );
  }

  return rv;
}


void NewObjectStore::AddNewEnviron( U16 scope,U8* env_nom,U8* n_args,
  									U8* opt_def,U8* begdef,U8* enddef ) {
/*
JBMLine( "AddNewEnviron\n" );
JBMLine( (char*)env_nom );
JBMLine( ", " );
if ( n_args ) JBMLine( (char*)n_args );
JBMLine( ", " );
if ( opt_def ) JBMLine( (char*)opt_def );
JBMLine( ", " );
if ( begdef )  JBMLine( (char*)begdef );
JBMLine( ", " );
if ( enddef ) JBMLine( (char*)enddef );
JBMLine( "\n" );
*/
  ENVIRON_REC* n_environ  =
    			(ENVIRON_REC*)TCI_NEW(char[sizeof(ENVIRON_REC)] );

  n_environ->scope  =  scope;

  U16 nom_len =  strlen( (char*)env_nom );
  U8* tmp =  (U8*)TCI_NEW(char[ nom_len+1 ] );
  strcpy( (char*)tmp,(char*)env_nom );
  n_environ->env_name =  tmp;

/*
char zzz[80];
sprintf( zzz,"AddNewEnviron, scope=%d\n",scope );
JBMLine( zzz );
JBMLine( (char*)tmp );
JBMLine( "\n" );
*/

  if ( n_args ) {
    U16 args_len  =  strlen( (char*)n_args );
    tmp =  (U8*)TCI_NEW(char[ args_len+1 ] );
    strcpy( (char*)tmp,(char*)n_args );
    n_environ->arg_count  =  tmp;
  } else
    n_environ->arg_count  =  NULL;

  if ( opt_def ) {
    U16 opt_def_len  =  strlen( (char*)opt_def );
    tmp =  (U8*)TCI_NEW(char[ opt_def_len+1 ] );
    strcpy( (char*)tmp,(char*)opt_def );
    n_environ->opt_def  =  tmp;
  } else
    n_environ->opt_def  =  NULL;

  if ( begdef ) {
    U16 beg_len =  strlen( (char*)begdef );
    tmp =  (U8*)TCI_NEW(char[ beg_len+1 ] );
    strcpy( (char*)tmp,(char*)begdef );
    n_environ->begdef =  tmp;
  } else {
    n_environ->begdef =  NULL;
	//TCI_ASSERT(0);
  }

  if ( enddef ) {
    U16 end_len =  strlen( (char*)enddef );
    tmp =  (U8*)TCI_NEW(char[ end_len+1 ] );
    strcpy( (char*)tmp,(char*)enddef );
    n_environ->enddef =  tmp;
  } else {
    n_environ->enddef =  NULL;
	//TCI_ASSERT(0);
  }

  n_environ->tmpl =  MakeEnvTemplate( env_nom,n_args,opt_def );

// Add at head of list - we are pushing onto a stack here!

  n_environ->next =  new_environs;
  new_environs    =  n_environ;
}


// Private function to build a template ( grammar production )
//  for a newenvironment.
// \newenvironment<uID5.154.0>!\newenvironment!REQPARAM(5.154.1,NONLATEX)
//	OPTPARAM(5.154.2,NONLATEX)OPTPARAM(5.154.3,NONLATEX)
//	REQPARAM(5.154.4,NONLATEX)REQPARAM(5.154.5,NONLATEX)
//
// \begin{myenv}[op]{rp1}{rp2}...
// <uID5.529.0>

U8* NewObjectStore::MakeEnvTemplate( U8* env_nom,
										U8* n_args,U8* opt_def ) {
/*
char zzz[256];
sprintf( zzz,"MakeEnvTemplate, nom=%s, zn_args=%s, opt_def=%s\n",
env_nom,n_args,opt_def );
JBMLine( zzz );
*/
  U8 buffer[1024];

  strcpy( (char*)buffer,"!\\begin{" );
  strcat( (char*)buffer,(char*)env_nom );
  strcat( (char*)buffer,"}!" );

  U16 nargs =  0;
  if ( n_args )
    nargs   =  atoi( (char*)n_args );

  if ( nargs ) {		// this environ takes args

	U16 tally =  0;
	if ( opt_def ) {	// the first arg is optional
      strcat( (char*)buffer,"OPTPARAM(5.529.1,NONLATEX)" );
      tally++;
    }

	char zid[16];
    U16 rp_ID =  2;
    while ( tally < nargs ) {
      strcat( (char*)buffer,"REQPARAM(5.529." );
      // itoa( rp_ID,zid,10 );
      sprintf(zid, "%d", rp_ID);
      strcat( (char*)buffer,zid );
      strcat( (char*)buffer,",NONLATEX)" );
      rp_ID++;
      tally++;
    }
  }

  strcat( (char*)buffer,"BUCKET(5.529.10,TEXT,,,\\end{" );
  strcat( (char*)buffer,(char*)env_nom );
  strcat( (char*)buffer,"},)!\\end{" );
  strcat( (char*)buffer,(char*)env_nom );
  strcat( (char*)buffer,"}!" );

  U16 len =  strlen( (char*)buffer );
  U8* rv  =  (U8*)TCI_NEW(char[ len+1 ] );
  strcpy( (char*)rv,(char*)buffer );

/*
JBMLine( "MakeEnvTemplate\n" );
JBMLine( (char*)rv );
JBMLine( "\n" );
*/

  return rv;
}


void NewObjectStore::EnterExitGroup( TCI_BOOL do_enter ) {

  if ( do_enter ) {
	TCI_ASSERT( ts_sp < 4 );
	ts_sp++;
  } else {						// hit "}" - exit a group
	TCI_ASSERT( ts_sp > 0 );
    if ( theo_stack[ts_sp].ts_style ) {
      delete theo_stack[ts_sp].ts_style;
      theo_stack[ts_sp].ts_style =  NULL;
	}
    if ( theo_stack[ts_sp].ts_bodyfont ) {
      delete theo_stack[ts_sp].ts_bodyfont;
      theo_stack[ts_sp].ts_bodyfont =  NULL;
	}
	ts_sp--;
  }
}


void NewObjectStore::SetTheoremStyleAttr( U16 which_attr,
											U8* new_val ) {
/*
char zzz[128];
sprintf( zzz,"SetTheoStyAttr, attr=%d, val=%s\n",which_attr,new_val );
JBMLine( zzz );
*/

  U8* tmp =  NULL;
  if ( new_val ) {
    U16 ln  =  strlen( (char*)new_val );
    tmp =  (U8*)TCI_NEW(char[ln+1] );
	strcpy( (char*)tmp,(char*)new_val );
  }

  if      ( which_attr ==TA_style ) {
    if ( theo_stack[ts_sp].ts_style ) {
	  //TCI_ASSERT( 0 );
      delete theo_stack[ts_sp].ts_style;
	}
    theo_stack[ts_sp].ts_style  =  tmp;

  } else if ( which_attr ==TA_bodyfont ) {

    if ( theo_stack[ts_sp].ts_bodyfont ) {
	  TCI_ASSERT( 0 );
      delete theo_stack[ts_sp].ts_bodyfont;
	}
    theo_stack[ts_sp].ts_bodyfont =  tmp;

  } else if ( which_attr ==TA_headerfont ) {

    if ( headerfont ) {
	  TCI_ASSERT( 0 );
      delete headerfont;
	}
    headerfont =  tmp;

  } else
	TCI_ASSERT( 0 );
    
}


U8* NewObjectStore::GetTheoremStyleAttr( U16 which_attr ) {

  U8* rv  =  NULL;
  U8* attr_val  =  NULL;

  U16 sp  =  ts_sp;
  while ( TRUE ) {			// loop down thru stack
    if        ( which_attr==TA_style ) {
      if ( theo_stack[sp].ts_style ) {
        attr_val  =  theo_stack[sp].ts_style;
		break;
	  }
    } else if ( which_attr==TA_bodyfont ) {
      if ( theo_stack[sp].ts_bodyfont ) {
        attr_val  =  theo_stack[sp].ts_bodyfont;
		break;
	  }
    } else
	  TCI_ASSERT(0);

    if ( sp > 0 ) sp--;
	else          break;
  }

  if ( attr_val ) {
    U16 ln  =  strlen( (char*)attr_val );
    rv  =  (U8*)TCI_NEW(char[ln+1] );
	strcpy( (char*)rv,(char*)attr_val );
  }

  return rv;
}



/*
    This object maintains a collection of counters similar to those
  that LaTeX manages during a run.  Each counter has a list of its
  first level sub-counters.  When a counter is incremented, all its
  sub-counters are reset.  Whenever a counter is reset all
  sub-counters are also reset.

LaTeX level numbers, and subcounter relationships

\newcounter{part}						% (-1) parts
\newcounter{chapter}					% (0)  chapters
\newcounter{section}[chapter]			% (1)  sections
\newcounter{subsection}[section]		% (2)  subsections
\newcounter{subsubsection}[subsection] 	% (3)  subsubsections
\newcounter{paragraph}[subsubsection] 	% (4)  paragraph
\newcounter{subparagraph}[paragraph] 	% (5)  subparagraph

For every LaTeX counter, there is a corresponding command
of the form \thecounter that typesets the current value of the counter.

\thepart
\thechapter
\thesection
etc.

*/


U8* zpart           =  (U8*)"part";
U8* zchapter        =  (U8*)"chapter";
U8* zsection        =  (U8*)"section";
U8* zsubsection     =  (U8*)"subsection";
U8* zsubsubsection  =  (U8*)"subsubsection";
U8* zparagraph      =  (U8*)"paragraph";
U8* zsubparagraph   =  (U8*)"subparagraph";
U8* zpage           =  (U8*)"page";
U8* zequation       =  (U8*)"equation";
U8* zfigure         =  (U8*)"figure";
U8* ztable          =  (U8*)"table";
U8* zfootnote       =  (U8*)"footnote";
U8* zmpfootnote     =  (U8*)"mpfootnote";
U8* zenumi          =  (U8*)"enumi";
U8* zenumii         =  (U8*)"enumii";
U8* zenumiii        =  (U8*)"enumiii";
U8* zenumiv         =  (U8*)"enumiv";
U8* zsecnumdepth	=  (U8*)"secnumdepth";
U8* ztocdepth       =  (U8*)"tocdepth";


//\newtheorem cmds, etc., also may create counters


void NewObjectStore::InitCounters() {
//JBMLine( "InitCounters\n" );

// WARNING: the following calls establish a set of counter/subcounters
//  relationships.  We may want these relationships to come from style
//  considerations or the inifile - they are hardcoded here!

  CreateCounter( zfigure,0,NULL );
  CreateCounter( ztable,0,NULL );

  CreateCounter( zpart,0,NULL );
  CreateCounter( zchapter,0,NULL );
  CreateCounter( zsection,0,zchapter );
  CreateCounter( zsubsection,0,zsection );
  CreateCounter( zsubsubsection,0,zsubsection );
  CreateCounter( zparagraph,0,zsubsubsection );
  CreateCounter( zsubparagraph,0,zparagraph );

  CreateCounter( zpage,1,NULL );

  CreateCounter( zequation,0,NULL );

// LaTeX assigns levels to sectioning commands
// part           = -1  ( 0 in article styls )
// chapter		  = 0
// section        = 1
// subsection 	  = 2
// subsubsection  = 3

  CreateCounter( zsecnumdepth,2,NULL );
  CreateCounter( ztocdepth,2,NULL );

  list_sp =  0;
  list_counter_vals[0]  =  0;
  list_counter_vals[1]  =  0;
  list_counter_vals[2]  =  0;
  list_counter_vals[3]  =  0;
}


void NewObjectStore::DisposeCounters() {
//JBMLine( "DisposeCounters\n" );

  COUNTER_INFO* del;
  COUNTER_INFO* c_rover =  counters;
  while ( c_rover ) {
    del =  c_rover;
    c_rover =  c_rover->next;

    SUBCOUNTER_REC* sdel;
    SUBCOUNTER_REC* s_rover =  del->subcounters;
    while ( s_rover ) {
      sdel    =  s_rover;
      s_rover =  s_rover->next;
      delete sdel;
    }

    delete del;
  }

  counters  =  NULL;
}


// When certain objects are encountered in a parse tree generated
//  from LaTeX source, we need to create counters for new objects
//  and/or record changes in certain counters.
// We need to duplicate the behavior of LaTeX's counters so that
//  we can access any numbers generated from them.
//  Page, section, theorem, equation numbers etc.

// \newcounter{ctr}[within]
// \setcounter{ctr}{val} - no subcounters are affected
// \addtocounter{ctr}{val} - no subcounters are affected
// \stepcounter{ctr} - increment AND reset subcounters
// \refstepcounter{ctr} - increment AND reset subcounters

void NewObjectStore::ProcessCounterCmd( TNODE* src_node ) {

  U16 uobjtype,usubtype,uID;
  GetUids( src_node->zuID,uobjtype,usubtype,uID );

  switch ( usubtype ) {

    case  34 : {		// \begin{equation}<uID5.34.41>
	  if ( uID==41 ) {
        U8* nom =  uIDtoName( usubtype,uID );
        if ( nom )
          IncCounter( nom );
      }
    }
    break;

    case 121 : {		// eqnarray<uID5.121.0>
	  if ( uID==9 ) {			// EQNLINESListEntry<uID5.121.86>
        U8* nom =  uIDtoName( 34,41 );
        if ( nom )
          IncCounter( nom );
      }
    }
    break;

    case 200 : {        // \sectioning cmd - increment it's counter
      U8* nom =  uIDtoName( usubtype,uID );
      if ( nom )
        IncCounter( nom );
    }
    break;

// \Qcb<uID5.399.1>!\Qcb!OPTPARAM(5.399.2,TEXT)REQPARAM(5.399.3,TEXT)
// \Qct<uID5.405.1>!\Qct!OPTPARAM(5.405.2,TEXT)REQPARAM(5.405.3,TEXT)

	case 399  :			// \Qcb[]{}
	case 405  : {		// \Qct[]{}
	  if ( uID == 1 )
        IncCounter( zfigure );
    }
    break;

    case 329 : {        // \begin{my_new_theorem}

    // extract the type name of this newtheorem instance -
    //  theorem, lemma, proposition, corollary, etc.

      U8 env_nom[64];
      char* ptr1  =  strchr( (char*)src_node->src_tok,'{' );
      if ( ptr1 ) {
        char* ptr2  =  strchr( (char*)ptr1,'}' );
        if ( ptr2 ) {
		  U16 ln  =  ptr2 - ptr1 - 1;
		  TCI_ASSERT( ln < 64 );
          strncpy( (char*)env_nom,ptr1+1,ln );
          env_nom[ln] =  0;
        } else
          TCI_ASSERT( 0 );
      } else
      	TCI_ASSERT( 0 );

      U8* starred  =  GetTheoremInfo( env_nom,TRI_starred );
      if ( !strcmp((char*)starred,"T") ) {
	    //TCI_ASSERT(0);
		break;
	  }

    // We need to know the name of the counter
    //  that counts instances of this newtheorem type.

	  U8* counter_nom =  GetTheoremInfo( env_nom,TRI_nums_like );
      if ( !counter_nom ) counter_nom =  env_nom;

      COUNTER_INFO* theo_ci =  FindCounterInfo( counter_nom );
	  if ( !theo_ci ) {		// the counter does not yet exist
        U8* within  =  GetTheoremInfo( env_nom,TRI_within );
        if ( within ) {
    // Here our newtheorem type has it's own counter, but it is
	//  a sub-counter of some larger division of the document.
          COUNTER_INFO* parenti =  FindCounterInfo( within );
          if ( !parenti )
            CreateCounter( within,0,NULL );
        }
        CreateCounter( counter_nom,0,within );
      }			// counter does not exist clause

/*
char zzz[256];
sprintf( zzz,"theo_env_name = %s, counter_name = %s\n",
		env_nom,counter_nom );
JBMLine( zzz );
*/
      IncCounter( counter_nom );
    }
    break;

    case 344 : {          // src_node == \caption[]{}
// \caption<uID5.344.1>!\caption!OPTPARAM(5.344.2,TEXT)REQPARAM(5.344.3,TEXT)
      TCI_ASSERT( uID==1 );
	  U16 parent_ID  =  GetCaptionOwnerID( src_node );
	  if      ( parent_ID == 1 )
        IncCounter( zfigure );
	  else if ( parent_ID == 2 )
        IncCounter( ztable );
	  else
        TCI_ASSERT( 0 );
    }
    break;

    case 350 : {          // \pagebreak
      U8* nom =  uIDtoName( usubtype,uID );
      TNODE* param  =  src_node->parts;
      if ( param ) {
        if ( param->contents ) {
          I16 val   =  atoi( (char*)param->contents->var_value );
          SetCounter( nom,val );
        } else
          IncCounter( nom );
      } else
        IncCounter( nom );
    }
    break;

    case 352 :          // \newpage
    case 353 :          // \clearpage
    case 354 : {        // \cleardoublepage
      U8* nom =  uIDtoName( usubtype,uID );
      IncCounter( nom );
    }
    break;

    case 351 : {        // \nopagebreak
    }
    break;

    case 355 : {        // \setcounter

/*
\setcounter<uID5.355.0>
  REQPARAM(5.355.1,TEXT)        // counter name
  REQPARAM(5.355.2,TEXT)        // new value for counter
*/
      TNODE* p_rover  =  src_node->parts;

      U8* c_name    =  NULL;
      I16 new_val   =  0;

      while ( p_rover ) {       // loop thru 2 params
/*
char zzz[80];
sprintf( zzz,"part ID = %s\n",p_rover->zuID );
JBMLine( zzz );
*/        
        U16 uobjtype,usubtype,uID;
        GetUids( p_rover->zuID,uobjtype,usubtype,uID );
        if      ( uID == 1 && p_rover->contents )
          c_name  =  p_rover->contents->var_value;
        else if ( uID == 2 && p_rover->contents )
          new_val =  atoi( (char*)p_rover->contents->var_value );

        p_rover =  p_rover->next;
      }  
/*
char zzz[80];
sprintf( zzz,"c_name = %s, new_val = %d\n",c_name,new_val );
JBMLine( zzz );
*/
      SetCounter( c_name,new_val );
    }
    break;

    case 356 : {        // \addtocounter
      U8* nom =  uIDtoName( usubtype,uID );
      TCI_ASSERT( 0 );
    }
    break;

    case 357 :          // \arabic
    case 358 :          // \roman
    case 359 :          // \Roman
    case 360 :          // \alph
    case 361 : {        // \Alph
	  TCI_ASSERT(0);
    }
    break;

    case 362 : {        // \newcounter
/*
\newcounter<uID5.362.1>!\newcounter!
  REQPARAM(5.362.2,TEXT)
  OPTPARAM(5.362.3,TEXT)
*/
      U8* c_name  =  NULL;
      U8* parent  =  NULL;		// within

      TNODE* p_rover  =  src_node->parts;
      while ( p_rover ) {       // loop thru 2 params
/*
char zzz[80];
sprintf( zzz,"part ID = %s\n",p_rover->zuID );
JBMLine( zzz );
*/        
        U16 uobjtype,usubtype,uID;
        GetUids( p_rover->zuID,uobjtype,usubtype,uID );
        if      ( uID == 2 && p_rover->contents )
          c_name  =  p_rover->contents->var_value;
        else if ( uID == 3 && p_rover->contents )
          parent  =  p_rover->contents->var_value;

        p_rover =  p_rover->next;
      }  
/*
char zzz[80];
sprintf( zzz,"c_name = %s, parent = %s\n",c_name,parent );
JBMLine( zzz );
*/
      CreateCounter( c_name,0,parent );
    }
    break;

// \stepcounter<uID5.367.1>!\stepcounter!REQPARAM(5.367.2,NONLATEX)
// \refstepcounter<uID5.368.1>!\refstepcounter!REQPARAM(5.368.2,NONLATEX)

    case 367 :
    case 368 : {
      U8* c_name  =  NULL;
      TNODE* p_rover  =  src_node->parts;
      while ( p_rover ) {       // loop thru 2 params
        U16 uobjtype,usubtype,uID;
        GetUids( p_rover->zuID,uobjtype,usubtype,uID );
        if ( uobjtype==5 && uID == 2 )
          if ( p_rover->contents )
            c_name  =  p_rover->contents->var_value;

        p_rover =  p_rover->next;
      }  

      if ( c_name )
        IncCounter( c_name );
    }
    break;

    default :
      TCI_ASSERT( 0 );  
    break;

  }

}


// Given the name of a counter, set it's value to 0.
// Recurse thru subcounters, setting the value of each to 0.

void NewObjectStore::ResetCounter( U8* nom ) {
/*
char zzz[80];
sprintf( zzz,"\nResetCounter, %s\n",nom );
JBMLine( zzz );
*/
  COUNTER_INFO* ci  =  FindCounterInfo( nom );
  if ( ci ) {
    ci->value =  0;

    SUBCOUNTER_REC* s_rover =  ci->subcounters;
    while ( s_rover ) {
/*
char zzz[80];
sprintf( zzz,"  SubCounter, %s\n",s_rover->name );
JBMLine( zzz );
*/
      ResetCounter( s_rover->name );
      s_rover =  s_rover->next;
    }

  } else
    TCI_ASSERT( 0 );

}

// Given a uID from Notebook.gmr, return a pointer
//  to the name of the corresonding counter.

U8* NewObjectStore::uIDtoName( U16 usubtype,U16 uID ) {

  U8* rv  =  NULL;

  if        ( usubtype == 34 ) {
    if ( uID==41 )					// \begin{equation}<uID5.34.41>
      rv  =  zequation;

  } else if ( usubtype == 200 ) {
    switch ( uID ) {
      case 0  :   rv  =  zpart;           break;
      case 1  :   rv  =  zchapter;        break;
      case 2  :   rv  =  zsection;        break;
      case 3  :   rv  =  zsubsection;     break;
      case 4  :   rv  =  zsubsubsection;  break;
      case 5  :   rv  =  zparagraph;      break;
      case 6  :   rv  =  zsubparagraph;   break;

      case 20 : 	// \part*<uID5.200.20>
      case 21 : 	// \chapter*<uID5.200.21>
      case 22 : 	// \section*<uID5.200.22>
      case 23 :  	// \subsection*<uID5.200.23>
      case 24 :  	// \subsubsection*<uID5.200.24>
      case 25 :  	// \paragraph*<uID5.200.25>
      case 26 :  	// \subparagraph*<uID5.200.26>
	  break;

      default :
        TCI_ASSERT( 0 );
      break;
    }

  } else {

    switch ( usubtype ) {

      case 350 :        // \pagebreak
      case 352 :        // \newpage
      case 353 :        // \clearpage
      case 354 : {      // \cleardoublepage
        rv  =  zpage;
      }
      break;

      default :
        TCI_ASSERT( 0 );
      break;
    }
  }

  return rv;
}


U16 NewObjectStore::uIDtoEnumID( U16 usubtype,U16 uID ) {

  U16 rv  =  0;

  if ( usubtype == 200 ) {
    switch ( uID ) {
      case 0  :         rv  =  LTX_part;            break;
      case 1  :         rv  =  LTX_chapter;         break;
      case 2  :         rv  =  LTX_section;         break;
      case 3  :         rv  =  LTX_subsection;      break;
      case 4  :         rv  =  LTX_subsubsection;   break;
      case 5  :         rv  =  LTX_paragraph;       break;
      case 6  :         rv  =  LTX_subparagraph;    break;

      default :
        TCI_ASSERT( 0 );
      break;
    }

  } else {

    switch ( usubtype ) {

      case 350 :        // \pagebreak
      case 352 :        // \newpage
      case 353 :        // \clearpage
      case 354 : {      // \cleardoublepage
        rv  =  LTX_page;
      }
      break;

/*
LTX_equation
LTX_figure
LTX_table
LTX_footnote
LTX_mpfootnote
LTX_enumi
LTX_enumii
LTX_enumiii
LTX_enumiv
LTX_secnumdepth
LTX_tocdepth
*/

      default :
        TCI_ASSERT( 0 );
      break;
    }
  }			// non-sectioning clause

  return rv;
}


U8* NewObjectStore::EnumIDtoName( U16 enumID ) {

  U8* rv  =  NULL;

  switch ( enumID ) {
    case LTX_part           :   rv  =  zpart;           break;
    case LTX_chapter        :   rv  =  zchapter;        break;
    case LTX_section        :   rv  =  zsection;        break;
    case LTX_subsection     :   rv  =  zsubsection;     break;
    case LTX_subsubsection  :   rv  =  zsubsubsection;  break;
    case LTX_paragraph      :   rv  =  zparagraph;      break;
    case LTX_subparagraph   :   rv  =  zsubparagraph;   break;
    case LTX_page           :   rv  =  zpage;           break;
    case LTX_equation       :   rv  =  zequation;       break;
    case LTX_figure         :   rv  =  zfigure;         break;
    case LTX_table          :   rv  =  ztable;          break;
    case LTX_footnote       :   rv  =  zfootnote;       break;
    case LTX_mpfootnote     :   rv  =  zmpfootnote;     break;
    case LTX_enumi          :   rv  =  zenumi;          break;
    case LTX_enumii         :   rv  =  zenumii;         break;
    case LTX_enumiii        :   rv  =  zenumiii;        break;
    case LTX_enumiv         :   rv  =  zenumiv;         break;
    case LTX_secnumdepth    :   rv  =  zsecnumdepth;    break;
    case LTX_tocdepth       :   rv  =  ztocdepth;		break;
			 
    default :
      TCI_ASSERT( 0 );
    break;
  }

  return rv;
}


// WARNING: the action here emulates LaTeX's \setcounter.
//  Subcounters are NOT reset by this function.

void NewObjectStore::SetCounter( U8* nom,I16 new_val ) {
//JBMLine( "NewObjectStore::SetCounter\n" );

  if ( !strncmp((char*)nom,(char*)zenumi,5) ) {

    U16 ix  =  0;
    if      ( !strcmp((char*)nom,(char*)zenumi)   ) ix  =  0;
    else if ( !strcmp((char*)nom,(char*)zenumii)  ) ix  =  1;
    else if ( !strcmp((char*)nom,(char*)zenumiii) ) ix  =  2;
    else if ( !strcmp((char*)nom,(char*)zenumiv)  ) ix  =  3;
    else TCI_ASSERT(0);

    list_counter_vals[ix] =  new_val+1;

  } else {		// not a list counter

    COUNTER_INFO* ci  =  FindCounterInfo( nom );
    if ( !ci ) {
      CreateCounter( nom,0,NULL );
      ci  =  counters;
    }
    ci->value =  new_val;
  }
}



I16 NewObjectStore::GetCounterValue( U16 enumID ) {

  U8* nom   =  EnumIDtoName( enumID );
  COUNTER_INFO* ci  =  FindCounterInfo( nom );
  if ( !ci ) {
    CreateCounter( nom,0,NULL );
    ci  =  counters;
  }

  return ci->value;
}


I16 NewObjectStore::GetCounterValue( U8* nom ) {

  COUNTER_INFO* ci  =  FindCounterInfo( nom );
  if ( !ci ) {
    CreateCounter( nom,0,NULL );
	// creating a new counter installs it at the head
    ci  =  counters;
  }

  return ci->value;
}


void NewObjectStore::IncCounter( U8* nom ) {

  COUNTER_INFO* ci  =  FindCounterInfo( nom );
  if ( !ci ) {
    CreateCounter( nom,0,NULL );
	// creating a new counter installs it at the head
    ci  =  counters;
  }

// the equation counter is a special case - it is global and has
//  no real subcounters.  However, it does freeze and run a pseudo
//  subcounter within a "mathletters" environment.

  if ( !strcmp((char*)nom,(char*)zequation) ) {
    SUBCOUNTER_REC* mlc =  ci->subcounters;
	if ( mlc )
      mlc->value  =  mlc->value + 1;
	else
      ci->value   =  ci->value + 1;

  } else {
    ci->value =  ci->value + 1;

// If the counter that was just incremented has any subcounters
//  they must be reset.

    SUBCOUNTER_REC* s_rover =  ci->subcounters;
    while ( s_rover ) {
      ResetCounter( s_rover->name );
      s_rover =  s_rover->next;
    }
  }
}


void NewObjectStore::CreateCounter( U8* nom,I16 val,U8* within ) {
/*
char xxx[80];
sprintf( xxx,"CreateCounter %s, within = %s\n",nom,within );
JBMLine( xxx );
*/
  COUNTER_INFO* ci  =  FindCounterInfo( nom );

  if ( ci ) {		// This counter already exists!
    // TCI_ASSERT(0);
  } else {
    ci  =  (COUNTER_INFO*)TCI_NEW( char[ sizeof(COUNTER_INFO) ] );

    ci->next  =  counters;        // new counter is head of list
    counters  =  ci;
  
    if ( strlen((char*)nom) > 31 ) {
      TCI_ASSERT( 0 );
      strncpy( (char*)ci->name,(char*)nom,31 );
      ci->name[31]  =  0;
    } else {
      strcpy( (char*)ci->name,(char*)nom );
    }

    //ci->style =  ;
    ci->value =  val;
    ci->subcounters =  NULL;

    if ( within ) {
/*
char zzz[128];
sprintf( zzz,"Create::Add subcounter %s, to parent %s\n",nom,within );
JBMLine( zzz );
*/
      COUNTER_INFO* parentci  =  FindCounterInfo( within );
	  if ( parentci )
        parentci->subcounters =  AddSubCounter( parentci->subcounters,nom );
	  else
	    TCI_ASSERT(0);
    }
  }

}


// Locate a counter record by name.

COUNTER_INFO* NewObjectStore::FindCounterInfo( U8* nom ) {

  COUNTER_INFO* rv  =  NULL;

  U16 nlen  =  strlen( (char*)nom );
  COUNTER_INFO* c_rover =  counters;
  while ( c_rover ) {				// loop thru list of counters

    if ( nlen > 31 ) {
      if ( !strncmp((char*)c_rover->name,(char*)nom,31) ) {
        rv  =  c_rover;
        break;
      }  
    } else {
      if ( !strcmp((char*)c_rover->name,(char*)nom) ) {
        rv  =  c_rover;
        break;
      }  
    }
    c_rover =  c_rover->next;
  }

  return rv;
}


void NewObjectStore::Dump() {

  char zzz[80];

  COUNTER_INFO* c_rover =  counters;
  while ( c_rover ) {
    sprintf( zzz,"Counter name=%s, value=%d\n",c_rover->name,c_rover->value );
    DumpLine( zzz );

    SUBCOUNTER_REC* s_rover =  c_rover->subcounters;
    while ( s_rover ) {
      sprintf( zzz,"  Subcounter=%s\n",s_rover->name );
      DumpLine( zzz );
      s_rover =  s_rover->next;
    }

    c_rover =  c_rover->next;
  }
}


void NewObjectStore::ResetAllCounters() {
//JBMLine( "NewObjectStore::ResetAllCounters\n" );

  COUNTER_INFO* c_rover =  counters;
  while ( c_rover ) {
    if ( strcmp((char*)c_rover->name,(char*)zpage) )
      c_rover->value =  0;
    else
      c_rover->value =  1;
    c_rover =  c_rover->next;
  }

  list_sp =  0;
  list_counter_vals[0]  =  0;
  list_counter_vals[1]  =  0;
  list_counter_vals[2]  =  0;
  list_counter_vals[3]  =  0;
}


SUBCOUNTER_REC* NewObjectStore::AddSubCounter( SUBCOUNTER_REC* list,
                                                    U8* new_name ) {

  SUBCOUNTER_REC* rv  =  list;

// search "list" to see if "new_name" is already there

  TCI_BOOL found =  FALSE;
  SUBCOUNTER_REC* s_rover =  list;
  while ( s_rover ) {
/*
char zzz[80];
sprintf( zzz,"checking %s, %s\n",(char*)new_name,(char*)s_rover->name );
JBMLine( zzz );
*/
    if ( !strcmp((char*)new_name,(char*)s_rover->name) ) {
//JBMLine( "AddSubCounter, already done\n" );
      found =  TRUE;
      break;
    }
    s_rover =  s_rover->next;
  }

  if ( !found ) {       // add "new_name" to head of list of subcounters

//JBMLine( "  AddSub, new sub not found in current sub list\n" );

    rv  =  (SUBCOUNTER_REC*)TCI_NEW( char[ sizeof(SUBCOUNTER_REC) ] );

    if ( new_name ) {
      if ( strlen((char*)new_name) > 31 ) {
        TCI_ASSERT( 0 );
        strncpy( (char*)rv->name,(char*)new_name,31 );
        rv->name[31]  =  0;
      } else {
        strcpy( (char*)rv->name,(char*)new_name );
      }
    } else
      rv->name[0] =  0;

    rv->next  =  list;
  }

  return rv;
}


// List counters are handled as follows

void NewObjectStore::PushListCounter() {

  if ( list_sp < 4 ) {
    list_sp++;
  } else
    TCI_ASSERT(0);
}


void NewObjectStore::PopListCounter() {

  if ( list_sp > 0 ) {
    list_sp--;
    list_counter_vals[list_sp]  =  0;
  } else
    TCI_ASSERT(0);
}


void NewObjectStore::IncListCounter() {

  if ( 0<list_sp && list_sp<=4 ) {
    list_counter_vals[list_sp-1]++;
  } else
    TCI_ASSERT(0);
}


I16 NewObjectStore::GetListCounterVal() {

  I16 rv  =  0;

  if ( 0<list_sp && list_sp<=4 ) {
    rv  =  list_counter_vals[list_sp-1];
  } else
    TCI_ASSERT(0);

  return rv;
}


TCI_BOOL NewObjectStore::HasCounter( U8* nom ) {

  COUNTER_INFO* ci  =  FindCounterInfo( nom );
  return ci ? TRUE : FALSE;
}


// \begin{figure}<uID5.342.1>
// \begin{table}<uID5.343.1>

U16 NewObjectStore::GetCaptionOwnerID( TNODE* caption_node ) {

  U16 rv  =  0;

  TNODE* t_rover  =  caption_node;
  while ( t_rover->prev )
	t_rover =  t_rover->prev;

  if ( t_rover->sublist_owner ) {
	TNODE* parent  =  t_rover->sublist_owner;
    U16 uobjtype,usubtype,uID;
    GetUids( parent->zuID,uobjtype,usubtype,uID );
    if ( uobjtype==5 && uID==1 ) {
      if      ( usubtype==342 )		// figure
	    rv  =  1;
      else if ( usubtype==343 )		// table
	    rv  =  2;
	}
	if ( !rv )
      rv  =  GetCaptionOwnerID( parent );
  }

  return rv;
}


void NewObjectStore::EnterMathLetters() {
//JBMLine( "EnterMathLetters\n" );

  COUNTER_INFO* eqn_counter =  FindCounterInfo( zequation );
  TCI_ASSERT( eqn_counter );

// increment
  eqn_counter->value  =  eqn_counter->value + 1;

  SUBCOUNTER_REC* mlc =  (SUBCOUNTER_REC*)TCI_NEW( char[ sizeof(SUBCOUNTER_REC) ] );
  mlc->name[0]  =  0;
  mlc->value    =  0;

  mlc->next =  eqn_counter->subcounters;
  eqn_counter->subcounters  =  mlc;
}


void NewObjectStore::ExitMathLetters() {
//JBMLine( "ExitMathLetters\n" );

  COUNTER_INFO* eqn_counter =  FindCounterInfo( zequation );
  TCI_ASSERT( eqn_counter );

  SUBCOUNTER_REC* del =  eqn_counter->subcounters;
  TCI_ASSERT( del );

  eqn_counter->subcounters  =  del->next;
  delete del;
}

// The "equation" counter is modified by the "mathletters" environment.
// Within a mathletters, equation numbers take values like 2a, 2b, 2c

void NewObjectStore::GetEquationCounterValue( U8* dest ) {

  COUNTER_INFO* eqn_counter =  FindCounterInfo( zequation );
  TCI_ASSERT( eqn_counter );
  //itoa( eqn_counter->value,(char*)dest,10 );
  sprintf((char*)dest, "%d", eqn_counter->value);
  U16 off =  strlen( (char*)dest );

  I16 ml_vals[16];		// storage for values of mathletters
  U16 i =  0;			//  subcounters

  SUBCOUNTER_REC* ml_rover  =  eqn_counter->subcounters;
  while ( ml_rover ) {
	ml_vals[i++]  =  ml_rover->value;
	ml_rover  =  ml_rover->next;
  }

// mathletters subcounters values are stored in ml_vals,
//  with the least significant value in slot 0.

  while ( i > 0 ) {
	I16 val0  =  ml_vals[i-1] - 1;	// 0 based 

	if ( i == 1 && val0 == -1 ) {

	} else {
	  U16 p1  =  val0 / 26;
	  if ( p1 ) {						// a counter value of 27
	    U16 p =  (p1 - 1) % 26;       //  yields "aa"
	    dest[off++] =  'a' + p;
	  }
	  U16 p0  =  val0 % 26;
	  dest[off++] =  'a' + p0;
	}
	i--;
  }
  dest[off] =  0;
}


