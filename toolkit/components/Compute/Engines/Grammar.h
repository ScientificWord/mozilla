// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

#ifndef Grammar_h
#define Grammar_h

//This object holds a .gmr file in a hash table.

#include "CmpTypes.h"
#include <stdio.h>

struct LINE_REC;
struct HASH_TABLE;
struct CONTEXT_ID;


class Grammar
{
public:
  Grammar(FILE* grammar_file, bool key_on_uids);
  ~Grammar();

  bool GetRecordFromIDs(const char*  zcurr_env, U32 uID, U32 usubID,
                        const char** dest_zname,
                        const char** dest_ztemplate) const;

  bool GetRecordFromName(const char* zcurr_env, const char* token, size_t ln,
                         U32& uID, U32& usubID,
                         const char** d_ztemplate) const;

  
  
  HASH_TABLE* LastTableHit() const { return m_last_table_hit;}
  void SetLastTableHit(HASH_TABLE* ht) { m_last_table_hit = ht; }

  HASH_TABLE* HashTables() const { return m_h_tables; }
  void SetHashTables(HASH_TABLE* ht) { m_h_tables = ht; }

  bool HashedOnUIDs() const { return m_hashed_on_uids;}
  void SetHashedOnUIDs(bool b) { m_hashed_on_uids = b; }

  CONTEXT_ID* ContextIDs() const { return m_pContextIDs; }
  void SetContextIDs( CONTEXT_ID* pid) {  m_pContextIDs = pid; }


private:
  HASH_TABLE* m_last_table_hit;
  HASH_TABLE* m_h_tables;
  bool m_hashed_on_uids;
  CONTEXT_ID* m_pContextIDs;
   
};

bool GetdBaseNamedRecord(const Grammar* gmr, 
                         const char* bin_name,
                         const char* op_name,
                         const char** eng_dbase_rec,
                         U32& ID, 
                         U32& subID);


int ChData2Unicodes(const char* p_chdata, U32* unicodes, int limit, const Grammar* mml_entities);
void Contents2Buffer(char* zdest, const char* p_chdata, int lim, const Grammar* mml_entities);
int GetLimitFormat(char* op_name, const Grammar* mml_entities);
bool IsTrigArgFuncName(const Grammar* mml_entities, const char* f_nom);
bool IsReservedFuncName(const Grammar* mml_entities, const char* f_nom);




#endif
