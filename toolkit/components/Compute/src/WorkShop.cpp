// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

#include "WorkShop.h"

#include "CmpTypes.h"
#include "MRequest.h"
#include "MResult.h"
#include "DefStore.h"
#include "PrefStor.h"
#include "CompEngine.h"
#include "Grammar.h"
#include "DefInfo.h"
#include "fltutils.h"
#include "xpcom-config.h"
#include "nsString.h"

// start with a utility function with nsILocalFile

void AppendSubpath( nsILocalFile * file, const char * asciiPath )
{
  if (!asciiPath) return;
  size_t zln = strlen(asciiPath);
  char *tmp = new char[zln + 1];
  strcpy(tmp, asciiPath);
  char * pch = tmp;
  char * pchStart = tmp;
  nsAutoString pathStr;
  bool done = false;
  while (!done){
    while (*pch && *pch != '\\' && *pch != '/') pch++;
    done = (*pch == 0);
    (*pch) = 0;
    pathStr = NS_ConvertUTF8toUTF16(pchStart);
    file->Append(pathStr);
    if (!done) *pch++; // skip past the null we put in
    while (*pch && *pch == '\\' && *pch == '/') pch++;
    pchStart = pch;
  }
}
      
MathWorkShop::MathWorkShop()
{
  engine_list = NULL;
  curr_eng = NULL;

  client_list = NULL;
  client_counter = 0;
  install_counter = 0;

  mml_entities = NULL;
  uprefs_store = new PrefsStore();

  wide_pref = NULL;
}

MathWorkShop::~MathWorkShop()
{
  delete mml_entities;
  delete uprefs_store;
  delete[] wide_pref;

  EngineInfo *el_rover = engine_list;
  while (el_rover) {
    EngineInfo *del = el_rover;
    el_rover = el_rover->next;
    delete del->comp_eng;
    delete del->eng_ID_dBase;
    delete del->eng_NOM_dBase;
    delete del;
  }

  ClientInfo *cl_rover = client_list;
  while (cl_rover) {
    ClientInfo *del = cl_rover;
    cl_rover = cl_rover->next;
    delete del->defstore;
    delete del;
  }
}

U32 MathWorkShop::GetClientHandle(U32 parentID)
{
  client_counter++;

  DefStore *parent_ds = NULL;
  if (parentID) {
    ClientInfo *p_rec = LocateClientRec(parentID);
    if (!p_rec) {
      TCI_ASSERT(0);
      parentID = 0;
    } else
      parent_ds = p_rec->defstore;
  }

  ClientInfo *new_client = new ClientInfo();
  new_client->next = client_list;
  new_client->ID = client_counter;
  new_client->parent_ID = parentID;
  new_client->defstore = new DefStore(parent_ds, client_counter);

  client_list = new_client;
  return client_counter;
}

void MathWorkShop::ReleaseClientHandle(U32 targ_handle)
{
  ClientInfo *head = NULL;
  ClientInfo *tail;

  U32 targs_parent = 0;

  ClientInfo *cl_rover = client_list;
  while (cl_rover) {
    ClientInfo *curr = cl_rover;
    cl_rover = cl_rover->next;
    if (curr->ID == targ_handle) {

      targs_parent = curr->parent_ID;

      // Found the record of the client to be released
      if (curr->defstore) {
        // Loop thru installed engines - release cliet on all
        EngineInfo *el_rover = engine_list;
        while (el_rover) {
          if (el_rover->comp_eng)
            el_rover->comp_eng->ReleaseClient(targ_handle,
                                              el_rover->eng_ID,
                                              curr->defstore);
          el_rover = el_rover->next;
        }
        delete curr->defstore;
      }
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

  client_list = head;

  // Children of the client being deleted no longer have a parent.
  DefStore *new_parent_ds = NULL;

  if (targs_parent) {
    ClientInfo *cr_parent = LocateClientRec(targs_parent);
    if (cr_parent)
      new_parent_ds = cr_parent->defstore;
    else
      TCI_ASSERT(0);
  }

  cl_rover = client_list;
  while (cl_rover) {
    if (cl_rover->parent_ID == targ_handle) {
      cl_rover->parent_ID = targs_parent;
      if (new_parent_ds)
        cl_rover->defstore->SetParentDefStore(new_parent_ds);
    }
    cl_rover = cl_rover->next;
  }

}

// For now, "install_script" is a grammar file (mplInstall.gmr)
// The return value here is a handle

U32 MathWorkShop::InstallCompEngine(nsILocalFile *install_script,
                                    MathResult & mr)
{
  printf("\n\n  jcs InstallCompEngine\n");
  U32 rv = 0;
  nsCOMPtr<nsILocalFile> workingDir;
  nsCOMPtr<nsIFile> parentDir, wd;
  install_script->GetParent(getter_AddRefs(parentDir));
  parentDir->Clone(getter_AddRefs(wd));
  workingDir = do_QueryInterface(wd);
  FILE *fp;
  install_script->OpenANSIFileDesc("r", &fp);
  if (fp) {
    // Lookup is on IDs
    Grammar *install_dBase = new Grammar(fp, true);
    fclose(fp);

    const char *db_zname;
    const char *db_ztemplate;
    install_dBase -> GetRecordFromIDs("ENGINFO", ENG_Name, 0, &db_zname,
                                    &db_ztemplate);
    //Maple5<uID1.0>  MuPAD<uID1.0>
    if (db_zname && *db_zname) {
      EngineInfo *ei = LocateEngineInfo(db_zname, 0);
      if (ei) {
        rv = ei->eng_ID;
        delete install_dBase;
        // this engine is already installed!!
        mr.PutResultCode(CR_success);
        return rv;
      }
    } else {                    // engine name not found in "install_script"
      printf("\n\n  jcs No engine name in install script");
      //TCI_ASSERT(0);
    }
    // The "install_script" should name the database file with all the info
    //  on this engine;

    const char *db_dummy;
    const char *db_eng_dbase;
    install_dBase->GetRecordFromIDs("ENGINFO", ENG_dbase, 0, &db_dummy,
                                    &db_eng_dbase);
    // eng_dbase<uID6.0>C:\xml\compute\engines\MuPAD.gmr
    if (db_eng_dbase && *db_eng_dbase) {
      if (!mml_entities) {
        const char *db_dummy;
        const char *db_ztemplate;
        install_dBase->GetRecordFromIDs("ENGINFO", ENG_entity_dbase, 0,
                                        &db_dummy, &db_ztemplate);
        // entity_dbase<uID7.0>C:\xml\compute\MMLents.gmr
        if (db_ztemplate && *db_ztemplate) {
          nsCOMPtr<nsILocalFile> entityFile;
          nsCOMPtr<nsIFile> tempFile;
          workingDir->Clone(getter_AddRefs(tempFile));
          entityFile = do_QueryInterface(tempFile);
          AppendSubpath(entityFile,db_ztemplate);
          FILE *fp;
          entityFile->OpenANSIFileDesc("r",&fp);
          if (fp) {
            mml_entities = new Grammar(fp, false);
            fclose(fp);
          } else {
            TCI_ASSERT(0);
            mr.PutResultCode(CR_NoEntitiesDbase);
          }
        }
      }
      nsCOMPtr<nsILocalFile> eng_dbaseFile;
      nsCOMPtr<nsIFile> temp2File;
      workingDir->Clone(getter_AddRefs(temp2File));
      eng_dbaseFile = do_QueryInterface(temp2File);
      AppendSubpath(eng_dbaseFile,db_eng_dbase);
      rv = FinishInstall(eng_dbaseFile, db_zname, install_dBase, mr);
    } else {
      TCI_ASSERT(0);
      mr.PutResultCode(CR_ScriptNoEngineDbase);
    }
    delete install_dBase;
  } else {
    TCI_ASSERT(0);
    mr.PutResultCode(CR_ScriptOpenFailed);
  }

  return rv;
}

// Callers can identify the engine to be uninstalled by name OR id.

void MathWorkShop::UninstallCompEngine(char *targ_eng_name,
                                       U32 targ_engine_ID)
{
  EngineInfo *head = NULL;
  EngineInfo *tail;

  EngineInfo *el_rover = engine_list;
  while (el_rover) {
    EngineInfo *curr = el_rover;
    el_rover = el_rover->next;

    bool do_uninstall = false;
    if (targ_engine_ID && curr->eng_ID == targ_engine_ID) {
      do_uninstall = true;
    } else if (targ_eng_name && !strcmp(curr->eng_name, targ_eng_name)) {
      do_uninstall = true;
      targ_engine_ID = curr->eng_ID;
    }

    if (do_uninstall) {
      // Found the record of the engine to be released

      // Loop thru clients - release defs on targ_engine_ID
      ClientInfo *cl_rover = client_list;
      while (cl_rover) {
        if (cl_rover->defstore)
          curr->comp_eng->ReleaseClient(cl_rover->ID,
                                        targ_engine_ID, cl_rover->defstore);

        cl_rover = cl_rover->next;
      }

      delete curr->comp_eng;
      delete curr->eng_ID_dBase;
      delete curr->eng_NOM_dBase;
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

  engine_list = head;
}

// rv is a handle

U32 MathWorkShop::FinishInstall(nsILocalFile *eng_dbase_file,
                                const char *eng_name, Grammar * install_dBase,
                                MathResult & mr)
{
  printf("\n\n  jcs FinishInstall\n");
  U32 rv = 0;

  CompEngine *comp_engine = NULL;

  // Note that the following dBases are created here,
  //  and are passed down to the CompEngine created below.
  //  They are owned here.
  Grammar *ID_dBase = NULL;
  Grammar *NOM_dBase = NULL;

  // Note that the engine database file is processed twice
  //  We're creating 2 database objects, one has the records
  //  keyed by number, the other is keyed by names.
  FILE *fp;
  eng_dbase_file->OpenANSIFileDesc("r",&fp);
  if (fp) {
    ID_dBase = new Grammar(fp, true);
    fclose(fp);

    eng_dbase_file->OpenANSIFileDesc("r",&fp);
    NOM_dBase = new Grammar(fp, false);
    fclose(fp);
  } else {
    TCI_ASSERT(0);
    printf("\n\n  jcs Could not open\n");
    mr.PutResultCode(CR_EngDbaseOpenFailed);
    return rv;
  }

  comp_engine =
    new CompEngine(ID_dBase, NOM_dBase, mml_entities, uprefs_store);
  TCI_ASSERT(comp_engine);
  nsCOMPtr<nsIFile> temp;
  nsCOMPtr<nsILocalFile> baseDir;
  eng_dbase_file->GetParent(getter_AddRefs(temp));
  baseDir = do_QueryInterface(temp);
  bool result = comp_engine->InitUnderlyingEngine(install_dBase, baseDir, mr);
  if (result) {
    // Add this engine to WorkShop's current engines list
    install_counter++;
    rv = install_counter;
    mr.PutResultCode(CR_success);

    EngineInfo *eng_info = new EngineInfo();
    eng_info->eng_ID = install_counter;
    eng_info->eng_ID_dBase = ID_dBase;
    eng_info->eng_NOM_dBase = NOM_dBase;
    eng_info->comp_eng = comp_engine;

    if (eng_name && *eng_name) {
      size_t zln = strlen(eng_name);
      if (zln < 80)
        strcpy(eng_info->eng_name, eng_name);
      else
        TCI_ASSERT(0);
    }
    // Add the new Engine info to the front of the list
    eng_info->next = engine_list;
    engine_list = eng_info;

    // Set the engine that we are installing to be the curr_eng
    curr_eng = comp_engine;
  } else {
    TCI_ASSERT(!"failed to initialize underlying engine");
    delete ID_dBase;
    delete NOM_dBase;
    delete comp_engine;
    curr_eng = 0;
    mr.PutResultCode(CR_EngInitFailed);
  }

  return rv;
}

EngineInfo *MathWorkShop::LocateEngineInfo(const char *targ_nom,
                                           U32 targ_engine_ID)
{
  EngineInfo *rover = engine_list;
  while (rover) {
    if (targ_engine_ID && rover->eng_ID == targ_engine_ID)
      break;
    if (targ_nom && *targ_nom && !strcmp(rover->eng_name, targ_nom))
      break;
    rover = rover->next;
  }
  return rover;
}

const char *MathWorkShop::GetNextSupportedCommand(U32 targ_engine_ID,
                                                  U32 * cmd_number)
{
  const char *rv = NULL;

  EngineInfo *rover = engine_list;
  while (rover) {
    if (rover->eng_ID == targ_engine_ID)
      break;
    rover = rover->next;
  }

  if (rover) {                  // found the target engine
    Grammar *ID_dBase = rover->eng_ID_dBase;
    const char *dest_zname;
    const char *dest_ztemplate;

    int n_fails = 0;
    U32 targ_ID = *cmd_number + 1;
    while (n_fails < 1000) {
      if (ID_dBase->
          GetRecordFromIDs("COMMANDS", targ_ID, 0, &dest_zname,
                           &dest_ztemplate)) {
        if (dest_zname && *dest_zname) {
          if (strcmp("END_OF_LIST", dest_zname)) {
            rv = dest_zname;
            *cmd_number = targ_ID;
          }
        }
        break;
      } else {
        targ_ID++;
        n_fails++;
      }
    }
  } else {
    TCI_ASSERT(0);
  }
  return rv;
}

void MathWorkShop::ProcessRequest(MathServiceRequest & msr, MathResult & mr)
{
  EngineInfo *ei = NULL;
  U32 targ_engine_ID = msr.GetEngineID();
  if (!targ_engine_ID) {
    ei = LocateEngineInfo(msr.GetEngineNamePtr(), 0);
    if (ei)
      msr.PutEngineID(ei->eng_ID);
  } else {
    ei = LocateEngineInfo(NULL, targ_engine_ID);
  }

  if (ei) {
    curr_eng = ei->comp_eng;
    //enum MarkupType { MT_UNDEFINED, MT_MATHML, MT_LATEX,
    //                  MT_MAPLEV_INPUT, MT_MUPAD_INPUT };

    // Make sure we have ASCII markup
    const char *a_markup = msr.GetASCIIMarkupPtr();
    if (!a_markup) {
      const U16 *w_m = msr.GetWideMarkupPtr();
      if (w_m) {
        char *zascii = WideToASCII(w_m);
        msr.PutASCIIMarkup(zascii);
        delete[] zascii;
        a_markup = msr.GetASCIIMarkupPtr();
      }
    }

    MarkupType mt = msr.GetMarkupType();
    if (mt == MT_UNDEFINED) {
      // temporary hack to get a markup type.
      // Eventually it has to come from the caller
      //   or a settable Shop default.
      if (a_markup) {
        if (strstr(a_markup, "math>"))
          msr.PutMarkupType(MT_MATHML);
        else 
          msr.PutMarkupType(MT_UNDEFINED);
        //if (strstr(a_markup, "tci"))
        //  msr.PutMarkupType(MT_MUPAD_INPUT);
      } else {
        // Some command don't carry any markup
      }
    }
    // msr carries a pointer to a DefStore
    U32 client_ID = msr.GetClientHandle();
    ClientInfo *ci = LocateClientRec(client_ID);
    if (ci && ci->defstore)
      msr.PutDefStore(ci->defstore);
    else
      TCI_ASSERT(0);
    curr_eng->Execute(msr, mr);
  } else {
    TCI_ASSERT(0);
  }
}

ClientInfo *MathWorkShop::LocateClientRec(U32 targ_handle)
{
  ClientInfo *cl_rover = client_list;
  while (cl_rover) {
    if (cl_rover->ID == targ_handle)
      return cl_rover;
    cl_rover = cl_rover->next;
  }

  return NULL;
}

const DefInfo *MathWorkShop::GetNextDef(U32 client_ID, U32 engine_ID,
                                        const DefInfo * curr_def)
{
  const DefInfo *rv = NULL;

  ClientInfo *ci = LocateClientRec(client_ID);
  if (ci) {
    rv = ci->defstore->GetNextDef(engine_ID, curr_def);
  } else {
    TCI_ASSERT(0);
  }
  return rv;
}

void MathWorkShop::ClearDefs(U32 client_ID, U32 engine_ID)
{
  ClientInfo *ci = LocateClientRec(client_ID);
  if (ci) {
    DefStore *ds = ci->defstore;
    if (ds) {
      EngineInfo *el_rover = engine_list;
      while (el_rover) {
        if (el_rover->comp_eng)
          el_rover->comp_eng->ReleaseClient(client_ID, engine_ID, ds);
        el_rover = el_rover->next;
      }
    } else {
      TCI_ASSERT(0);
    }
  } else {
    TCI_ASSERT(0);
  }
}

U32 MathWorkShop::EngineNameToID(const char *targ_name)
{
  U32 rv = 0;

  EngineInfo *el_rover = engine_list;
  while (el_rover) {
    if (!strcmp(el_rover->eng_name, targ_name)) {
      rv = el_rover->eng_ID;
      break;
    }
    el_rover = el_rover->next;
  }

  return rv;
}

bool MathWorkShop::SetEngineAttr(U32 engine_ID, int attr_ID,
                                     const char *new_val)
{
  bool rv = false;

  EngineInfo *ei = LocateEngineInfo(NULL, engine_ID);
  if (ei) {
    CompEngine *eng = ei->comp_eng;
    rv = eng->SetEngineAttr(attr_ID, 0, new_val);
  }

  return rv;
}

const char *MathWorkShop::GetEngineAttr(U32 engine_ID, int attr_ID)
{
  const char *rv = NULL;

  EngineInfo *ei = LocateEngineInfo(NULL, engine_ID);
  if (ei) {
    CompEngine *eng = ei->comp_eng;
    rv = eng->GetEngineAttr(attr_ID);
  }

  return rv;
}

int MathWorkShop::SetClientPref(U32 client_ID, U32 pref_ID,
                                const char *pref_value)
{
  if (client_ID) {
    ClientInfo *ci = LocateClientRec(client_ID);
    if (ci) {
      if (ci->defstore) {
        ci->defstore->SetPref(pref_ID, pref_value);
      } else
        TCI_ASSERT(0);
    } else {
      TCI_ASSERT(0);
    }
  } else {
    uprefs_store->SetPref(pref_ID, pref_value);
  }
  return 1;
}

const char *MathWorkShop::GetClientPref(U32 client_ID, U32 pref_ID,
                                        int no_inherit)
{
  const char *rv = NULL;

  if (client_ID) {
    ClientInfo *ci = LocateClientRec(client_ID);
    if (ci) {
      if (ci->defstore)
        rv = ci->defstore->GetPref(pref_ID, no_inherit);
      else
        TCI_ASSERT(0);
    } else {
      TCI_ASSERT(0);
    }
  } else {
    rv = uprefs_store->GetPref(pref_ID);
  }

  return rv;
}

int MathWorkShop::SetClientPrefWide(U32 client_ID, U32 pref_ID,
                                    const U16 * pref_value)
{
  char *zascii = NULL;
  if (pref_value)
    zascii = WideToASCII(pref_value);

  int rv = SetClientPref(client_ID, pref_ID, zascii);
  delete[] zascii;

  return rv;
}

const U16 *MathWorkShop::GetClientPrefWide(U32 client_ID, U32 pref_ID,
                                           int no_inherit)
{
  const char *zascii = GetClientPref(client_ID, pref_ID, no_inherit);
  if (zascii) {
    int zlen;
    delete[] wide_pref;
    wide_pref = ASCIItoWide(zascii, zlen);
    return wide_pref;
  }

  return NULL;
}
