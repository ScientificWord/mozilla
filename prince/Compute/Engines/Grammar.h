// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

#ifndef Grammar_h
#define Grammar_h

//This object holds a .gmr file in a hash table.

#include "../cmptypes.h"
#include <stdio.h>

typedef struct tagHASH_REC
{
  char *zname;
  U32 ID;
  U32 subID;
  const char *ztemplate;
} HASH_REC;

typedef struct tagHASH_TABLE
{
  tagHASH_TABLE *next;
  char *zenv_names;
  U32 slot_count;
  HASH_REC *records;
} HASH_TABLE;

typedef struct tagLINE_REC
{
  tagLINE_REC *next;
  char *zline;
  HASH_TABLE *table_node;
} LINE_REC;

#define TOK_LEN_LIM     40
#define CONTEXT_NOM_LIM 40

typedef struct tagCONTEXT_ID
{
  char context_nom[CONTEXT_NOM_LIM];
  U32 context_uID;
} CONTEXT_ID;

#define NUM_CONTEXTS    30
#define UNKNOWN_CLASS   999

class Grammar
{
public:
  Grammar(FILE * grammar_file, bool key_on_uids);
  ~Grammar();

  bool GetRecordFromIDs(const char *zcurr_env, U32 uID, U32 usubID,
                            const char **dest_zname,
                            const char **dest_ztemplate);
  bool GetRecordFromName(const char *zcurr_env, const char *token, size_t ln,
                             U32 & uID, U32 & usubID,
                             const char **d_ztemplate);
private:
  LINE_REC * FileToLineList(FILE * grammar_file);
  void FileLineToHashTables(const char *fline, HASH_TABLE * t_node);
  void ExtractIDs(const char *num_str, U32 & rec_ID, U32 & rec_subID);
  U32 HashzNom(const char *znom);
  U32 HashFromIDs(U32 uID, U32 subID);
  bool EnvOK(const char *zcurr_env, HASH_TABLE * table);
  void LocateOffsets(const char *fline, int * offsets);

  void GetContextNom(const char *nom, char *dest, U32 & the_uID);
  void AddContext(const char *new_env_nom, char *env_list, U32 the_uID);
  void RemoveContextName(const char *del_nom, char *env_list);

  HASH_TABLE *FindTableForEnv(const char *env_list);
  bool IsSubset(const char *env_list_subset,
                    const char *env_list_superset);

  HASH_TABLE *MakeNextHNode();

#ifdef DEBUG_Dump
  void Dump(FILE * df);
#endif
  bool hashed_on_uids;

  HASH_TABLE *h_tables;
  HASH_TABLE *last_table_hit;

  CONTEXT_ID contextIDs[NUM_CONTEXTS];
};

#endif
