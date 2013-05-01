// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

#ifndef WORKSHOP_H
#define WORKSHOP_H

#include "CmpTypes.h"
#include "nsILocalFile.h"

class MathServiceRequest;
class MathResult;
class CompEngine;
class Grammar;
class DefStore;
class PrefsStore;
class DefInfo;

// Node for list of engines

typedef struct tagEngineInfo {
  tagEngineInfo*  next;
  char            eng_name[80];
  U32             eng_ID;
  Grammar*        eng_ID_dBase;
  Grammar*        eng_NOM_dBase;
  CompEngine*     comp_eng;
} EngineInfo;


// Node for list of clients

typedef struct tagClientInfo {
  tagClientInfo*  next;
  U32             ID;
  U32             parent_ID;
  DefStore*       defstore;
} ClientInfo;



class MathWorkShop {
public:

  MathWorkShop();
  ~MathWorkShop();

  // Clients are 1-to-1 with DefStores - they may or may not have a parent.
  //  parentID = 0 indicates no parent at all.
  U32   GetClientHandle( U32 parentID );
  void  ReleaseClientHandle( U32 targ_client_handle );

  U32   EngineNameToID( const char* engine_name );

  DefStore* GetDefStore(U32 client_ID);
  // Client DefSore iterater - start "curr_def" at NULL
  const DefInfo* GetNextDef( U32 client_ID,U32 engine_ID,const DefInfo* curr_def );
  void  ClearDefs( U32 client_ID,U32 engine_ID );


  U32   InstallCompEngine( nsILocalFile * install_script,MathResult& mr );
  void  UninstallCompEngine( char* eng_name,U32 eng_ID );

  const char* GetNextSupportedCommand( U32 engine_ID,U32* curr_cmd_ID );

  void  ProcessRequest( MathServiceRequest& msr,MathResult& mr );

  bool    SetEngineAttr( U32 engine_ID,int attr_ID,const char* s_val );
  const char* GetEngineAttr( U32 engine_ID,int attr_ID );

  int         SetClientPref( U32 client_ID,U32 pref_ID,const char* pref_value );
  const char* GetClientPref( U32 client_ID,U32 pref_ID,int no_inherit );
  int         SetClientPrefWide( U32 client_ID,U32 pref_ID,const U16* pref_value );
  const U16*  GetClientPrefWide( U32 client_ID,U32 pref_ID,int no_inherit );

private:
  EngineInfo* LocateEngineInfo( const char* targ_nom,U32 targ_engine_ID );
  ClientInfo* LocateClientRec( U32 targ_handle );

  U32         FinishInstall( nsILocalFile * eng_dbase_file,const char* eng_name,
                             Grammar* install_dBase,MathResult& mr );

  Grammar* mml_entities;
  PrefsStore* uprefs_store;

  EngineInfo* engine_list;
  CompEngine* curr_eng;

  U32   client_counter;
  U32   install_counter;

  ClientInfo* client_list;

  U16*  wide_pref;
};

#endif

