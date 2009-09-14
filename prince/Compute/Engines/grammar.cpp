// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

/* Given the name of a .gmr file, the ctor must load its contents
    into a set of internal stores.  Public functions on a Grammar
    object dispense this data to clients.
*/

#include "Grammar.h"
#include "strutils.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


struct HASH_REC
{
  char *zname;
  U32 ID;
  U32 subID;
  const char *ztemplate;
};

struct HASH_TABLE
{
  HASH_TABLE *next;
  char *zenv_names;
  U32 slot_count;
  HASH_REC *records;
};

struct LINE_REC
{
  LINE_REC *next;
  char *zline;
  HASH_TABLE *table_node;
};

#define TOK_LEN_LIM     40
#define CONTEXT_NOM_LIM 40
#define NUM_CONTEXTS    30


struct CONTEXT_ID
{
  char context_nom[CONTEXT_NOM_LIM];
  U32 context_uID;
};

#define UNKNOWN_CLASS   999


// struct CONTEXT_ID_ARRAY
// {
//   CONTEXT_ID m_array[NUM_CONTEXTS];
// };




LINE_REC* FileToLineList(FILE * grammar_file, Grammar*);
void FileLineToHashTables(const char *fline, HASH_TABLE * t_node, Grammar*);

void ExtractIDs(const char *num_str, U32 & rec_ID, U32 & rec_subID, Grammar* grammar);


U32 HashzNom(const char* znom)
{
  U32 rv = 0L;

  U32 fudge = 31L;
  while (*znom) {
    rv += (*znom - 31) * fudge;
    znom++;
    if (fudge == 7L)
      fudge = 31L;
    else
      fudge = fudge - 12L;
  }

  return rv;
}


U32 HashFromIDs(U32 ID, U32 subID)
{
  U32 rv = 0L;

  char buffer[32];
  sprintf(buffer, "%lu.%lu", ID, subID);

  int shift = 0;
  char *ptr = buffer;
  while (*ptr) {
    if (*ptr >= '0' && *ptr <= '9') {
      U32 tmp = *ptr;
      rv += (tmp << shift);
      shift++;
    } else {
      break;
    }
    ptr++;
  }

  return rv;
}




// Each hash table has a list of env names - a catenation of zstrs.
//  We check that the table identified by "tableID" is valid for "zcurr_env".
bool EnvOK(const char *zcurr_env, HASH_TABLE* table)
{
  bool rv = false;

  char *env_nom = table->zenv_names;

  if (env_nom) {
    if (!strcmp(zcurr_env, "GENERIC")) {
      rv = true;
    } else {
      while (*env_nom) {
        if (!strcmp(env_nom, zcurr_env)) {  // found
          rv = true;
          break;
        } else {
          env_nom += strlen(env_nom) + 1;
        }
      }
    }
  } else {
    //JBMLine( "Table slot has NO NAMES\n" );
  }

  return rv;
}

// fline = "\blah<uID2.10.43>remaining bytes are the template
//               ^   ^       ^
//               0   1       2

void LocateOffsets(const char *fline, int * offsets)
{
  const char *ptr = strstr(fline, "<uID");
  if (ptr) {
    offsets[0] = ptr - fline;
    offsets[1] = offsets[0] + 4;

    const char *p2 = ptr;
    while (*p2 && *p2 != '>')
      p2++;

    TCI_ASSERT(*p2 == '>');
    offsets[2] = p2 - fline + 1;
  } else {
    TCI_ASSERT(0);
  }
}

// <START_CONTEXT_TEXT,1.1.0>
//                ^

void GetContextNom(const char *nom, char *dest, U32 & the_uID)
{
  while (*nom != '>' && *nom != ',') {
    *dest = *nom;
    dest++;
    nom++;
  }
  *dest = 0;

  if (*nom == ',') {
    nom = strchr(nom, '.');
    nom++;
//    int the_usubtype = atoi(nom);  // subtype unused by current code
    nom = strchr(nom, '.');
    nom++;
    the_uID = atoi(nom);
  }
}

// Utility to put info that identifies a context encountered during
//   the read of .gmr file into contextIDs array.
void AddContext(const char *new_env_nom, char *env_list, U32 the_uID, CONTEXT_ID* pContextIDs)
{
  while (*env_list) {
    if (!strcmp(env_list, new_env_nom))
      return;
    env_list += strlen(env_list) + 1;
  }
  strcpy(env_list, new_env_nom);
  env_list += strlen(new_env_nom) + 1;
  *env_list = 0;

  int slot = 0;
  while (pContextIDs[slot].context_uID)
    slot++;
  TCI_ASSERT(slot < NUM_CONTEXTS);

  strcpy(pContextIDs[slot].context_nom, new_env_nom);
  pContextIDs[slot].context_uID = the_uID;
}

void RemoveContextName(const char *del_nom, char *env_list)
{
  char *delp = NULL;
  char *endp = env_list;

  while (*endp) {
    if (!strcmp(endp, del_nom))
      delp = endp;
    endp += strlen(endp) + 1;
  }

  if (delp) {
    char *rover = delp + strlen(delp) + 1;
    while (rover <= endp) {
      *delp = *rover;
      delp++;
      rover++;
    }
  }
}


// env_list_subset  =  "math0text00"
// env_list_superset=  "junk0text0math00"
bool IsSubset(const char *env_list_subset,
              const char *env_list_superset)
{
  while (*env_list_subset) {
    const char *test_names = env_list_superset;

    while (*test_names) {
      if (!strcmp(env_list_subset, test_names))
        break;
      else
        test_names += strlen(test_names) + 1;
    }
    if (*test_names)            // found
      env_list_subset += strlen(env_list_subset) + 1;
    else
      break;
  }

  return (*env_list_subset) ? false : true;
}


HASH_TABLE* MakeNextHNode(Grammar* grammar)
{
  HASH_TABLE *rv = new HASH_TABLE();

  rv->next = NULL;
  rv->zenv_names = NULL;        // context names
  rv->slot_count = 0;           // number of records in array
  rv->records = NULL;           // pointer to array of records

  // add this node to the "h_tables" list

  if (grammar -> HashTables()) {
    HASH_TABLE *rover = grammar -> HashTables();
    while (rover->next)
      rover = rover->next;
    rover->next = rv;
  } else {
    grammar -> SetHashTables(rv);
  }
  return rv;
}



HASH_TABLE* FindTableForEnv(const char *env_list, Grammar* grammar)
{
  if (*env_list == 0)
    return grammar -> HashTables() -> next;

  HASH_TABLE *rv = NULL;

  HASH_TABLE *rover = grammar -> HashTables();
  while (rover) {
    char *env_names = rover -> zenv_names;
    if (env_names) {
      if (IsSubset(env_names, env_list) && IsSubset(env_list, env_names)) {
        rv = rover;
        break;
      }
    }
    rover = rover->next;
  }

  if (!rv) {
    const char *tp = env_list;
    while (*tp)
      tp += strlen(tp) + 1;

    size_t zln = tp - env_list + 1;
    char *envs = new char[zln];
    memcpy(envs, env_list, zln);

    rv = MakeNextHNode(grammar);
    rv->zenv_names = envs;
  }

  return rv;
}



//zzzzzz

#define TOKENIZER_TEX     1
#define LINELIMIT         32767

/* A "Grammar" stores a collection of definitions of grammar elements.
    It implements functions that look up these definitions.  For Parsing,
    the lookup is hashed on the literal that identifies an instance of
    the grammar element.  For Translating, the elements are hashed on the
    uID of the element - in this case the client is going from a uID
    back to an element in a grammar.
*/

Grammar::Grammar(FILE * grammar_file, bool key_on_uids)
{
  SetHashedOnUIDs(key_on_uids);

  // Clear the list of hash tables for named grammar objects.
  //   Every set of names has its own hash table, and a list of "context"
  //   names in which the objects are allowed.
  //   Examples of contexts are "text" and "math" and the arguments of
  //   some special commands like format, which takes \l \c \r tokens only.

  SetHashTables( (HASH_TABLE *) NULL); // list of "hash table" nodes

  // the first node is reserved for "_DEFS_"
  MakeNextHNode(this);

  // the second node is reserved for the un-named context
  MakeNextHNode(this);

  SetContextIDs( new CONTEXT_ID[NUM_CONTEXTS] );

  // Store for the names and uIDs of the contexts in this grammar
  for (U32 slot = 0; slot < NUM_CONTEXTS; slot++) { // text,math,\format,etc
    m_pContextIDs[slot].context_nom[0] = 0;
    m_pContextIDs[slot].context_uID = 0;
  }

  // The following call reads in the grammar file.  As lines are read
  //   they are examined for environment switches.  Nodes are created
  //   in "h_tables" for sets of names encountered and are filled out
  //   re "zenv_names" and "slot_count"s.
  LINE_REC *line_list = FileToLineList(grammar_file, this);

  // Allocate arrays of hash slots for each "hash table" node.
  // The "slot_count" field gives the number of items to be placed
  //   in each hash table.
  HASH_TABLE *rover = HashTables();
  while (rover) {
    if (rover->slot_count) {
      U32 n_slots = (3 * rover->slot_count) / 2;
      n_slots++;                // assure at least 1 slot remains empty

      HASH_REC *h_array = new HASH_REC[n_slots];
      for (U32 si = 0; si < n_slots; si++) {
        h_array[si].zname = NULL;
        h_array[si].ztemplate = NULL;
      }
      rover->records = h_array;
      rover->slot_count = n_slots;
    }
    rover = rover->next;
  }

  // Move data from "line_list" to the hash tables
  while (line_list) {
    FileLineToHashTables(line_list->zline, line_list->table_node, this);

    LINE_REC *del = line_list;
    line_list = line_list->next;
    delete del->zline;
    delete del;
  }

  SetLastTableHit(NULL);        // last hash table in which an object was located

  // Use the following to dump this grammar
  //Dump( (FILE*)NULL  );   // this goes to "\jbm"
}

Grammar::~Grammar()
{
  HASH_TABLE* del;
  while (HashTables()) {
    delete HashTables() -> zenv_names;

    U32 lim_slot = HashTables() -> slot_count;
    if (lim_slot) {
      HASH_REC *slot_array = HashTables() -> records;
      U32 slot = 0;
      while (slot < lim_slot) {
        delete slot_array[slot].zname;
        delete slot_array[slot].ztemplate;
        slot++;
      }
      delete slot_array;
    }

    del = HashTables();
    SetHashTables( HashTables() -> next );
    delete del;
  }
}

#ifdef DEBUG_Dump
void Dump(FILE * df)
{
  char zzz[256];

  sprintf(zzz, "\nDumping Grammar hash tables:\n");
  if (df)
    fputs(zzz, df);
  else
    JBM::JBMLine(zzz);

  U32 tableID = 0;
  HASH_TABLE *curr_table = h_tables;
  while (curr_table) {
    U32 lim_slot = curr_table->slot_count;

    sprintf(zzz, "\nTable id = %d,Item count = %d\n", tableID, lim_slot);
    if (df)
      fputs(zzz, df);
    else
      JBM::JBMLine(zzz);

    if (lim_slot) {
      if (curr_table->zenv_names) {
        sprintf(zzz, "Contexts(s):\n");
        if (df)
          fputs(zzz, df);
        else
          JBM::JBMLine(zzz);
        char *envs = curr_table->zenv_names;
        while (*envs) {
          sprintf(zzz, "Context=%s\n", envs);
          if (df)
            fputs(zzz, df);
          else
            JBMLine(zzz);
          envs += strlen(envs) + 1;
        }
      } else {
        sprintf(zzz, "Context: Default\n");
        if (df)
          fputs(zzz, df);
        else
          JBM::JBMLine(zzz);
      }

      HASH_REC *rover = curr_table->records;
      int slot = 0;
      while (slot < lim_slot) {
        if (rover[slot].zname) {
          U32 hash = HashzNom(rover[slot].zname) % lim_slot;
          sprintf(zzz, "%d(%d),%s,%s,%s\n", slot, hash,
                  rover[slot].zname, rover[slot].zuID, rover[slot].ztemplate);
        } else
          sprintf(zzz, "0\n");

        if (df)
          fputs(zzz, df);
        else
          JBM::JBMLine(zzz);
        slot++;
      }
    }

    tableID++;
    curr_table = curr_table->next;
  }
}
#endif

// Given IDs, look up it's definition in the destination grammar

bool Grammar::GetRecordFromIDs(const char *zcurr_env, U32 uID, U32 usubID,
                                   const char **dest_zname,
                                   const char **dest_ztemplate)	const
                                  
{
  *dest_zname = NULL;
  *dest_ztemplate = NULL;

  bool rv = false;
  if ( !HashedOnUIDs() ) {
    TCI_ASSERT(0);
    return rv;
  }

  HASH_TABLE* start_table = LastTableHit() ? LastTableHit() : HashTables() -> next;
  HASH_TABLE* curr_table = start_table;

  U32 hash = HashFromIDs(uID, usubID);

  do {                          // loop thru the hash tables
    if (EnvOK(zcurr_env, curr_table)) {
      U32 slot_limit = curr_table->slot_count;
      if (slot_limit) {
        U32 curr_slot = hash % slot_limit;
        HASH_REC *h_array = curr_table->records;
        do {                    // loop down this array
          if (!h_array[curr_slot].zname)
            break;

          U32 curr_ID = h_array[curr_slot].ID;
          U32 curr_sub = h_array[curr_slot].subID;

          if (curr_ID == uID && curr_sub == usubID) {
            *dest_zname = h_array[curr_slot].zname;
            *dest_ztemplate = h_array[curr_slot].ztemplate;
            //SetLastTableHit(curr_table);
            rv = true;
          } else {
            curr_slot++;
            if (curr_slot == slot_limit)
              curr_slot = 0;
          }
        } while (!rv);
      }
    }

    curr_table = curr_table->next;
    if (!curr_table) {
      curr_table = HashTables() -> next;
    }
  } while (!rv && curr_table != start_table);

  return rv;
}

bool Grammar::GetRecordFromName(const char* zcurr_env, 
                                const char* token,
                                size_t ln, 
                                U32& uID, 
                                U32& usubID,
                                const char** d_ztemplate) const
{
  bool rv = false;
  if ( HashedOnUIDs() ) {
    TCI_ASSERT(0);
    return rv;
  }
  // Make a local copy of the token we're looking for
  char ztoken[TOK_LEN_LIM];
  if (ln + 1 < TOK_LEN_LIM) {
    strncpy(ztoken, token, ln);
    ztoken[ln] = 0;
  } else {
    return rv;
  }
  HASH_TABLE* start_table = LastTableHit() ? LastTableHit() : HashTables() -> next;
  HASH_TABLE* curr_table = start_table;

  U32 hash = HashzNom(ztoken);

  do {                          // loop thru the hash tables
    if (EnvOK(zcurr_env, curr_table)) {
      U32 slot_limit = curr_table -> slot_count;
      if (slot_limit) {
        U32 curr_slot = hash % slot_limit;
        HASH_REC* h_array = curr_table->records;

        do {                    // loop down this array
          const char* zcurr_nom = h_array[curr_slot].zname;

          if (!zcurr_nom) {     // lookup failed, empty table entry
            break;
          }

          if (strcmp(ztoken, zcurr_nom)) {
            curr_slot++;
            if (curr_slot == slot_limit)
              curr_slot = 0;
          } else {              // name match found

            // In MathML, there may be several definitions for one entity or symbol.
            //  ie '+' has 3 entries with differing form attributes (infix, prefix
            //  and postfix).
            // The caller of GetGrammarDataFromNameAndAttrs must set the zform
            //  param in order to get the required entry.
            //    ie zform  =  "prefix";
            //

            bool found = true;

            if (found) {
              uID = h_array[curr_slot].ID;
              usubID = h_array[curr_slot].subID;
              *d_ztemplate = h_array[curr_slot].ztemplate;
              //SetLastTableHit(curr_table);
              rv = true;
            } else {
              curr_slot++;
              if (curr_slot == slot_limit)
                curr_slot = 0;
            }
          }
        } while (!rv);
      }
    }

    curr_table = curr_table->next;
    if (!curr_table)
      curr_table = HashTables() -> next;
  } while (!rv && curr_table != start_table);

  return rv;
}



bool GetdBaseNamedRecord(const Grammar* gmr, 
                         const char* bin_name,
                         const char* op_name,
                         const char** eng_dbase_rec,
                         U32& ID, 
                         U32& subID)
{
  if (op_name && *op_name) {
    const char *d_ztemplate;
    if (gmr->
        GetRecordFromName(bin_name, op_name, strlen(op_name), ID, subID,
                          &d_ztemplate)) {
      *eng_dbase_rec = d_ztemplate;
      return true;
    }
  }
  return false;
}



// private functions

// The following call reads in the grammar file.  As lines are read
//   they are examined for context switches.  "h_tables" is filled
//   re "zenv_names" and "slot_count"s.

LINE_REC* FileToLineList(FILE * grammar_file, Grammar* grammar)
{
  char *t_ptr;

  LINE_REC *head = NULL;
  LINE_REC *tail;
  LINE_REC *new_node;

  HASH_TABLE *curr_table = NULL;  // this identifies the table for each line

  char env_list[256];
  env_list[0] = 0;
  U32 env_uID;

  char fline[256];
  while (fgets(fline, 256, grammar_file)) { // loop thru lines of grammar
    size_t ln = strlen(fline);  // remove trailing white chars
    while (ln) {
      if (fline[ln - 1] <= ' ') {
        ln--;
        fline[ln] = 0;
      } else {
        break;
      }
    }
    if (ln) {                   // process line
      if (fline[0] == ';' && (fline[1] != '<' || fline[2] != 'u')) {

        // comment line

      } else if ((t_ptr = strstr(fline, "<TOKENIZER_TEX>"))) {

        //      tokenizerID =  TOKENIZER_TEX;

      } else if ((t_ptr = strstr(fline, "<TOKENIZER_MML>"))) {

      } else if ((t_ptr = strstr(fline, "<DEFAULT_CONTEXT_"))) {

        //      GetContextNom( t_ptr+17,(char*)default_context,env_usubtype,env_uID ); 

      } else if ((t_ptr = strstr(fline, "<START_CONTEXT_"))) {

        char new_env_nom[CONTEXT_NOM_LIM];
        GetContextNom(t_ptr + 15, new_env_nom, env_uID);
        AddContext(new_env_nom, env_list, env_uID, grammar -> ContextIDs() );
        curr_table = FindTableForEnv(env_list, grammar);

      } else if ((t_ptr = strstr(fline, "<END_CONTEXT_"))) {

        char new_env_nom[CONTEXT_NOM_LIM];
        GetContextNom(t_ptr + 13, new_env_nom, env_uID);
        RemoveContextName(new_env_nom, env_list);
        curr_table = FindTableForEnv(env_list, grammar);

      } else {                  // put this line on the list

        new_node = new LINE_REC();
        new_node->next = NULL;
        new_node->zline = new char[ln + 1];
        strcpy(new_node->zline, fline);

        bool is_macro = false;
        if (fline[0] == '_')
          if (!strstr(fline, "<uID"))
            is_macro = true;

        if (is_macro) {
          new_node->table_node = grammar -> HashTables();
          grammar -> HashTables() -> slot_count = grammar -> HashTables() -> slot_count + 1;
        } else {
          new_node->table_node = curr_table;
          curr_table->slot_count = curr_table -> slot_count + 1;
        }

        if (!head)
          head = new_node;
        else
          tail->next = new_node;
        tail = new_node;
      }
    }                           // process line
  }                             // loop thru file lines

  return head;
}

// The following function uses the class variable "hashed_on_uids"
//  to decide which field keys each record.
// fline =  "\alpha<uID3.1.1>"
// fline =  "_BLAH_etc."

void FileLineToHashTables(const char *fline, HASH_TABLE * t_node, Grammar* grammar)
{
  U32 rec_ID = 0;
  U32 rec_subID = 0;

  // Locate the fields in this line
  // fline = "\blah<uID2.10.43>remaining bytes are the template
  //               ^   ^       ^
  //               0   1       2
  // fline = "_blah_remaining bytes are the template
  //                ^
  //                0
  //                2

  int offsets[4];

  if (t_node == grammar -> HashTables() ) {     // first table reserved for "_DEFS_"
    const char *p = strchr(fline + 1, '_');
    offsets[0] = offsets[2] = p - fline + 1;
  } else {
    LocateOffsets(fline, offsets);
    ExtractIDs(fline + offsets[1], rec_ID, rec_subID, grammar);
  }

  // Set up the name of this syntax element in a heap zstring
  size_t zln = offsets[0];
  char* znom = new char[zln + 1];
  strncpy(znom, fline, zln);
  znom[zln] = 0;

  // Find a slot for the element in the hash table
  HASH_REC* h_array = t_node->records;
  U32 slot_lim = t_node->slot_count;

  U32 slot;
  if (grammar -> HashedOnUIDs() && t_node != grammar -> HashTables() ) {
    U32 hash = HashFromIDs(rec_ID, rec_subID);
    slot = hash % slot_lim;
  } else {
    slot = HashzNom(znom) % slot_lim;
  }
  while (h_array[slot].zname) { // slot is already occupied
    slot++;
    if (slot == slot_lim)
      slot = 0;
  }

  // Assign field values for the hash table entry
  h_array[slot].zname = znom;
  h_array[slot].ID = rec_ID;
  h_array[slot].subID = rec_subID;

  // extract the template
  const char *ptr = fline + offsets[2];
  if (*ptr >= ' ') {
    zln = strlen(ptr);
    char *ztempl = new char[zln + 1];
    strcpy(ztempl, ptr);
    h_array[slot].ztemplate = ztempl;
  }
}




void ExtractIDs(const char *num_str, U32 & rec_ID, U32 & rec_subID, Grammar* grammar)
{
  rec_ID = 0;
  rec_subID = 0;

  bool got_ID = false;
  bool is_hex = false;
  U32 val = 0;

  while (1) {
    char curr_byte = *num_str;
    if (curr_byte == 'x') {
      is_hex = true;
    } else if (curr_byte == '.') {
      rec_ID = val;
      got_ID = true;
      val = 0;
      is_hex = false;
    } else if (curr_byte == ' ') {
      ; // hope for data later
    } else if (curr_byte >= '0' && curr_byte <= '9') {
      if (is_hex)
        val = (val * 16) + curr_byte - '0';
      else
        val = (val * 10) + curr_byte - '0';
    } else if (curr_byte >= 'A' && curr_byte <= 'F') {
      if (is_hex)
        val = (val * 16) + curr_byte - 'A' + 10;
      else
        break;
    } else if (curr_byte >= 'a' && curr_byte <= 'f') {
      if (is_hex)
        val = (val * 16) + curr_byte - 'a' + 10;
      else
        break;
    } else {
      break;
    }
    num_str++;
  }

  if (got_ID)
    rec_subID = val;
  else
    rec_ID = val;
}



// Extract chars and unicodes from the ASCII string p_chdata.
// Returns a count of the symbols processed, up to "limit".

int ChData2Unicodes(const char* p_chdata, U32* unicodes, int limit, const Grammar* mml_entities)
{
  int rv = 0;

  if (p_chdata && *p_chdata) {
    const char *ptr = p_chdata;
    while (*ptr && rv < limit) {
      if (*ptr == '&') {
        ptr++;
        if (*ptr == '#') {

            // numeric entity - &#x201a;, &#9876;. etc.
            ptr++;
            U32 place_val = 10;
            if (*ptr == 'x') {
              place_val = 16;
              ptr++;
            }
            unicodes[rv] = ASCII2U32(ptr, place_val);
            rv++;

            while (*ptr && *ptr != ';')
              ptr++;

        } else {

            // non-numeric entity - &theta;, &ApplyFunction;. etc.
            const char *entity = ptr - 1;
            while (*ptr && *ptr != ';')
              ptr++;

            U32 ID, subID;
            size_t zln = ptr - entity + 1;
            const char* p_data;
            if (mml_entities->GetRecordFromName("MATH", entity, zln, ID, subID, &p_data)) {

                if (p_data && *p_data) {
                  //&ApplyFunction;<uID3.5.6>infix,65,U02061
                  const char* ptr = strstr(p_data, ",U");
                  if (ptr) {
                      unicodes[rv] = ASCII2U32(ptr + 2, 16);
                      rv++;
                  } else {
                      TCI_ASSERT(0);
                  }
                } else {
                    TCI_ASSERT(0);
                }

            } else {

                while (entity <= ptr) {
                  unicodes[rv] = *entity;
                  rv++;
                  entity++;
                }
            }

        }

      } else {
        unicodes[rv] = *ptr;
        rv++;
      }
      ptr++;
    }
  }
  return rv;
}


void Contents2Buffer(char* zdest, const char* p_chdata, int lim, const Grammar* mml_entities)
{
  if (p_chdata) {
    U32 unicodes[128];
    int n_chars = ChData2Unicodes(p_chdata, unicodes, 128, mml_entities);
    if (n_chars < 128) {
      size_t i = strlen(zdest);
      int j = 0;
      while (i < lim && j < n_chars) {
        U32 ch = unicodes[j];
        if (ch >= ' ' && ch <= '~') {
          zdest[i] = ch;
          i++;
        } else {
          sprintf(zdest + i, "&#x%x;", ch);
          i = strlen(zdest);
        }
        j++;
      }
      zdest[i] = 0;
    } else {
      TCI_ASSERT(0);
    }
  }
}



int GetLimitFormat(char* op_name, const Grammar* mml_entities)
{
  int rv = 0;

  U32 ID, subID;
  const char* p_data;
  if (mml_entities -> GetRecordFromName("LIMFORMS", op_name, strlen(op_name), ID, subID, &p_data)) {
    if (p_data && *p_data)
      rv = atoi(p_data);
  }

  return rv;
}




bool IsTrigArgFuncName(const Grammar* gmr, const char* nom)
{
  U32 ID, subID;
  const char *p_data;
  if (gmr->GetRecordFromName("TRIGARGFUNCS", nom, strlen(nom), ID, subID, &p_data)) {
    if (p_data && *p_data)
      int rhs = atoi(p_data); // unused
    return true;
  }

  return false;
}
  
// See also FUNCTIONS section in engine grammar files.
bool IsReservedFuncName(const Grammar* gmr, const char *nom)
{
  U32 ID, subID;
  const char *p_data;
  if (gmr->GetRecordFromName("RESERVEDFUNCS", nom, strlen(nom), ID, subID, &p_data)) {
    if (p_data && *p_data)
      int rhs = atoi(p_data); // unused
    return true;
  }

  return false;
}
