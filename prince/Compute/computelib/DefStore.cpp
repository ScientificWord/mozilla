// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

#include "DefStore.h"
#include "iCmpTypes.h"
#include "DefInfo.h"
#include "strutils.h"
#include <string.h>

DefStore::DefStore(DefStore* parent_store, U32 client_handle) :
   parent_ds(parent_store), 
   client_ID(client_handle), 
   def_list(NULL), 
   prefs_list(NULL)
{
}

DefStore::~DefStore()
{
  ClearDefs(0);
  DisposeParamList(prefs_list);
}

void DefStore::SetParentDefStore(DefStore* parent_store)
{
  parent_ds = parent_store;
}

void DefStore::PushDefInfo(U32 engine_ID,
                           const char* def_name, 
                           U32 def_type,
                           const char* markup, 
                           const char *arg_list,
                           U32 n_sub_args,
                           const char* ASCII_src, 
                           const U16* WIDE_src)
{
  if (def_name && *def_name) {
    def_list = DeleteDefInfo(engine_ID, def_name);

    DefInfo* new_definfo = new DefInfo();
    new_definfo->next = def_list;
    new_definfo->engine_ID = engine_ID;
    new_definfo->owner_ID = client_ID;
    new_definfo->def_type = def_type;
    new_definfo->n_subscripted_args = n_sub_args;

    new_definfo->canonical_name = DuplicateString(def_name);
    new_definfo->src_markup = DuplicateString(markup);
    new_definfo->arg_list = DuplicateString(arg_list);
	new_definfo->ASCII_src = DuplicateString(ASCII_src);

    if (WIDE_src && *WIDE_src) {
      size_t zln = 0;
      const U16 *ptr = WIDE_src;
      while (*ptr) {
        ptr++;
        zln++;
      }
      U16 *tmp = new U16[zln + 1];
      size_t i = 0;
      ptr = WIDE_src;
      while (i <= zln) {
        tmp[i++] = *ptr;
        ptr++;
      }
      new_definfo->WIDE_src = tmp;
    } else
      new_definfo->WIDE_src = NULL;

    def_list = new_definfo;

  } else
    TCI_ASSERT(0);
}

DefInfo* DefStore::GetDefInfo(U32 engine_ID, const char* targ_canon_nom)
{
  DefInfo *rv = GetLocalDefInfo(engine_ID, targ_canon_nom);
  if (!rv && parent_ds) {
    // Here, "targ_canon_nom" is not in this DefStore
    // It may be in the DefStore of an ancestral client
    rv = parent_ds->GetDefInfo(engine_ID, targ_canon_nom);
  }

  return rv;
}

void DefStore::RemoveDef(U32 engine_ID, const char *targ_canon_nom)
{
  def_list = DeleteDefInfo(engine_ID, targ_canon_nom);
}

// Local utilities

DefInfo* DefStore::GetLocalDefInfo(U32 engine_ID, const char* targ_canon_nom)
{
  if (targ_canon_nom && *targ_canon_nom) {
    DefInfo* dl_rover = def_list;
    while (dl_rover) {
      if (dl_rover->engine_ID == engine_ID && StringEqual(dl_rover->canonical_name, targ_canon_nom))
        return dl_rover;
      dl_rover = dl_rover->next;
    }

  } else
    TCI_ASSERT(0);

  return NULL;
}

DefInfo* DefStore::DeleteDefInfo(U32 engine_ID, const char* targ_canon_nom)
{
  DefInfo* head = NULL;
  DefInfo* tail;
  DefInfo* dl_rover = def_list;

  while (dl_rover) {
    DefInfo* curr = dl_rover;
    dl_rover = dl_rover->next;
    if (curr->engine_ID == engine_ID && StringEqual(curr->canonical_name, targ_canon_nom)) {
      delete curr->canonical_name;
      delete curr->src_markup;
      delete curr->arg_list;
      delete curr->ASCII_src;
      delete curr->WIDE_src;
      delete curr;
    } else {
      if (!head)
        head = curr;
      else
        tail->next = curr;
      tail = curr;
      tail->next = NULL;
    }
  }

  return head;
}

char* DefStore::GetDefList(U32 targ_engine)
{
  char* tmp = NULL;
  U32 tln = 0;
  int tally = 0;
  DefInfo* dl_rover = def_list;

  while (dl_rover) {
    if (dl_rover->engine_ID == targ_engine) {
      if (dl_rover->canonical_name) {
        if (tally)
          tmp = AppendStr2HeapStr(tmp, tln, ",");
        tmp = AppendStr2HeapStr(tmp, tln, dl_rover->canonical_name);
        tally++;
      } else
        TCI_ASSERT(0);
    }

    dl_rover = dl_rover->next;
  }

  return DuplicateString(tmp);
}

// Clear all defs on a certain engine.
// If eng_ID is 0, clear all defs

void DefStore::ClearDefs(U32 eng_ID)
{
  DefInfo* head = NULL;
  DefInfo* tail;
  DefInfo* dl_rover = def_list;

  while (dl_rover) {
    DefInfo* del = dl_rover;
    dl_rover = dl_rover->next;
    if (!eng_ID || eng_ID == del->engine_ID) {
      delete[] del->canonical_name;
      delete[] del->src_markup;
      delete[] del->arg_list;
      delete del->ASCII_src;
      delete del->WIDE_src;
      delete del;
    } else {
      if (!head)
        head = del;
      else
        tail->next = del;

      tail = del;
      tail->next = NULL;
    }
  }

  def_list = head;
}

const DefInfo* DefStore::GetNextDef(U32 targ_engine, const DefInfo * curr_def)
{
  const DefInfo *rv = NULL;

  if (curr_def)
    rv = curr_def->next;
  else
    rv = def_list;

  while (rv) {
    if (!targ_engine || rv->engine_ID == targ_engine)
      break;
    rv = rv->next;
  }

  return rv;
}

int DefStore::SetPref(U32 pref_ID, const char *new_value)
{
  if (!new_value)
    return RemovePref(pref_ID);

  bool done = false;

  PARAM_REC* rover = prefs_list;
  while (rover) {
    if (rover->param_ID == pref_ID) {
      if (rover->ztext) {
        delete[] rover->ztext;
        rover->ztext = NULL;
      }
      if (new_value) {
        size_t zln = strlen(new_value);
        char *tmp = new char[zln + 1];
        strcpy(tmp, new_value);
        rover->ztext = tmp;
      }
      done = true;
      break;
    }
    rover = rover->next;
  }

  if (!done)
    prefs_list = AppendParam(prefs_list, pref_ID, 0, new_value);

  return 1;
}

int DefStore::RemovePref(U32 pref_ID)
{
  int rv = 0;

  PARAM_REC* new_list = NULL;
  PARAM_REC* tail;
  PARAM_REC* rover = prefs_list;

  while (rover) {
    PARAM_REC* curr = rover;
    rover = rover->next;
    if (curr->param_ID == pref_ID) {
      delete[] curr->ztext;
      delete curr;
      rv++;
    } else {
      if (!new_list)
        new_list = curr;
      else
        tail->next = curr;
      tail = curr;
      tail->next = NULL;
    }
  }

  prefs_list = new_list;
  return rv;
}

const char* DefStore::GetPref(U32 targ_ID, int no_inherit)
{
  const char* rv = NULL;
  PARAM_REC* rover = prefs_list;

  while (rover) {
    if (rover->param_ID == targ_ID) {
      if (rover->ztext)
        rv = rover->ztext;
      break;
    }
    rover = rover->next;
  }

  if (!rover && !no_inherit && parent_ds) {
    // Here, "targ_ID" is not in this DefStore
    // It may be in the DefStore of an ancestral client
    rv = parent_ds->GetPref(targ_ID, 0);
  }

  return rv;
}
