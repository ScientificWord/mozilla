
#ifdef TESTING
  #undef THIS_FILE
  static char THIS_FILE[] = __FILE__;
#endif

#include "namestor.h"
#include "tcistrin.h"
#include <string.h>
#include <ctype.h>


NameStore::NameStore(U32 numchains, TCI_BOOL delrecs) {

  tablesize =  numchains;
#ifdef TESTING
  // tablesize should be prime
  U32 i = 2;
  while (i < tablesize)
  {
    if (tablesize%i == 0)   // not prime!
    {
      TCI_ASSERT(FALSE);
      break;
    }
    if (i*i > tablesize)
      break;
    i++;
  }
#endif

  dodelete  =  delrecs;
  m_bModified =  FALSE;

  htable  =  TCI_NEW(NAME_RECP[tablesize]);
  memset(htable, 0, sizeof(NAME_RECP)*tablesize);

  head =  NULL;
  tail =  NULL;
  m_mode = NSM_CASE_SENSITIVE;

#ifdef NS_MONITOR
  MonDataPtr = (NSMonitorData *) NULL;
  domonitor  = FALSE;
#endif
}


NameStore::~NameStore() {

  NAME_REC* curr;
  NAME_REC* delrec;

  for (U32 i=0; i<tablesize; i++) {
    curr  =  htable[i];
    while (curr!=(NAME_REC*)NULL) {
      delrec  =  curr;
      curr    =  curr->alpha;
      if (dodelete && delrec->info)
        delete (void*)delrec->info;
      delete delrec;
    }
  }
  delete htable;

#ifdef NS_MONITOR
  if (MonDataPtr != (NSMonitorData *) NULL) {
    delete MonDataPtr->slot_info;
    delete MonDataPtr;
  }
#endif
}


TCI_BOOL NameStore::GetName(const U8* targname, U32 targlen, U32 targtype, U32& theinfo) {

  NAME_REC* namerec =  GetNameRec(targname, targlen, targtype);
  theinfo =  namerec ? namerec->info : 0;
  return (namerec != NULL);
}


TCI_BOOL NameStore::GetName(const TCIString& targname, U32 targtype, U32& theinfo)
{
  return GetName( (const U8*)targname, targname.GetLength(), targtype, theinfo);
}

NAME_REC* NameStore::GetNameRec(const U8* targname, U32 targlen, U32 targtype) {

  U32 h =  Hash(targname, targlen);
  NAME_REC* currrec  =  htable[h];

#ifdef NS_MONITOR
  if (domonitor)
    ++MonDataPtr->slot_info[h].accesses;
#endif

  while (currrec) {
    if (currrec->nlen == targlen) {
      if (targtype==NS_MATCH_ALL || currrec->ntype==targtype) {
        if (m_mode == NSM_CASE_SENSITIVE) {
          if (!strncmp(currrec->name, (char*)targname, targlen))
            return currrec;
        } else if (m_mode == NSM_CASE_INSENSITIVE){
          if (!_strnicmp(currrec->name, (char*)targname, targlen))
            return currrec;
        } else if (m_mode == NSM_WIDE_CASE_SENSITIVE){
          if (!wcsncmp((const wchar_t *)currrec->name, (const wchar_t *)targname, targlen/2))
            return currrec;
        } else if (m_mode == NSM_WIDE_CASE_INSENSITIVE){
          if (!_wcsnicmp((const wchar_t *)currrec->name, (const wchar_t *)targname, targlen/2))
            return currrec;
        }
      }
    }
    currrec =  currrec->alpha;
  }

  return (NAME_REC*)NULL;
}


TCI_BOOL NameStore::PutName(const U8* newname, U32 newlen,
                              U32 newtype, U32 newinfo, TCI_BOOL is_global) {

  NAME_REC* namerec =  GetNameRec(newname, newlen, newtype);

  TCI_BOOL rv =  TRUE;
  if (namerec == (NAME_REC*)NULL) {
    PushName(newname, newlen, newtype, newinfo, is_global);
    m_bModified =  TRUE;
  } else
    rv =  FALSE;

  return rv;
}

TCI_BOOL NameStore::PutName(const TCIString& newname, U32 newtype, U32 newinfo, 
                                                      TCI_BOOL is_global)
{
  return PutName((const U8*)newname, newname.GetLength(), newtype, newinfo, is_global);
}                                                      

U32 NameStore::Hash(const U8* newname, U32 len)
{
  U32 t(0);
  U32 i(0);
  if (m_mode == NSM_CASE_SENSITIVE) {
    for (i = 0; i < len; ++i)
      t = (t * 65599) + newname[i];
  } else if (m_mode == NSM_CASE_INSENSITIVE){
    for (i = 0; i <len; ++i)
      t = (t * 65599) + toupper(newname[i]);
  } else if (m_mode == NSM_WIDE_CASE_SENSITIVE){
    for (i = 0; i <len; i+=2)
      t = (t * 65599) + *(const wchar_t *)(newname+i);
  } else if (m_mode == NSM_WIDE_CASE_INSENSITIVE){
    for (i = 0; i <len/2; i+=2)
      t = (t * 65599) + towupper(*(const wchar_t *)(newname+i));
  }
  return t%tablesize;   // tablesize should be a prime
}


void NameStore::PushName(const U8* newname, U32 newlen,
                            U32 newtype, U32 newinfo, TCI_BOOL is_global) {

  I32 newsize =  sizeof(NAME_REC) + newlen - 2;
  NAME_REC* rec =  (NAME_REC*)TCI_NEW(TCICHAR[newsize]);
  rec->next   =  (NAME_REC*)NULL;
  rec->ntype  =  newtype;
  rec->info   =  newinfo;
  rec->nlen   =  newlen;

  if (m_mode == NSM_CASE_SENSITIVE || m_mode == NSM_CASE_INSENSITIVE)
    strncpy(rec->name, (char*)newname, newlen);
  else if (m_mode == NSM_WIDE_CASE_SENSITIVE || m_mode == NSM_WIDE_CASE_INSENSITIVE)
    wcsncpy((wchar_t *)rec->name, (const wchar_t *)newname, newlen/2);

  U32 chainno =  Hash(newname, newlen);
  rec->alpha  =  htable[ chainno ];
  htable[ chainno ] =  rec;           // install at head of chain
  m_bModified =  TRUE;

  if (!is_global) {
    if (head==(NAME_REC*)NULL) {
      head  =  rec;
      tail  =  rec;
    } else {
      tail->next  =  rec;
      tail  =  rec;
    }
  }
}


void NameStore::PopName(const U8* targname, U32 targlen, NAME_REC* poprec) {

/*
printf("popping %d ", targlen);
for (I32 zz=0; zz<targlen; zz++) printf("%c", targname[zz]);
printf("\n");
*/
  U32 chainno =  Hash(targname, targlen);
  NAME_REC* currrec =  htable[ chainno ];
  NAME_REC* prevrec =  (NAME_REC*)NULL;

  while (currrec!=(NAME_REC*)NULL) {
    if (currrec == poprec)
      break;

    prevrec =  currrec;
    currrec =  currrec->alpha;
  }

  if (currrec != (NAME_REC*)NULL) {
    if (prevrec==(NAME_REC*)NULL) {
      htable[ chainno ] =  currrec->alpha;
    } else {
      prevrec->alpha =  currrec->alpha;
    }
    if (dodelete && currrec->info != 0L) delete (void*)currrec->info;

    // zzzASC  search the other links for this record;
    NAME_REC* zzz1  =  head;
    if (zzz1 == currrec) {
      if (zzz1 == tail) {
        head  =  (NAME_REC*)NULL;
        tail  =  (NAME_REC*)NULL;
      } else
        head  =  head->next;
    } else if (head != tail) {
      NAME_REC* zzz2  =  head->next;
      while (zzz1 != tail) {      // equivalent to  (zzz2 != head) ??
        if (zzz2 == currrec) {
          zzz1->next  =  zzz2->next;
          if (zzz2 == tail) tail = zzz1;
          break;
        }
        zzz1  =  zzz2;
        zzz2  =  zzz1->next;
      }
    }

    delete currrec;
    m_bModified =  TRUE;
  }
}


U32 NameStore::GetCurrTail() {

  return (U32)tail;
}


void NameStore::ReleaseTail(U32 newtail) {

  tail  =  (NAME_REC*)newtail;
  NAME_REC* currrec;
  if (tail==(NAME_REC*)NULL) {
    currrec =  head;
    head    =  (NAME_REC*)NULL;
  } else {
    currrec =  tail->next;
    tail->next  =  (NAME_REC*)NULL;
  }

  NAME_REC* delrec;
  while (currrec!=(NAME_REC*)NULL) {
    delrec  =  currrec;
    currrec =  currrec->next;
    PopName((U8*)delrec->name, delrec->nlen, delrec);
  }
}


U32 NameStore::GetNext(U32 curr, U8* dname, U32& dlen, U32& dtype, U32& dinfo) {

  NAME_REC* nrp  =  (NAME_REC*)curr;
  if (nrp == (NAME_REC*)NULL) nrp =  head;
  else                          nrp =  nrp->next;
  if (nrp != (NAME_REC*)NULL) {
    dlen    =  nrp->nlen;
    if (m_mode == NSM_CASE_SENSITIVE || m_mode == NSM_CASE_INSENSITIVE)
      strncpy((char*)dname, nrp->name, dlen);
    else if (m_mode == NSM_WIDE_CASE_SENSITIVE || m_mode == NSM_WIDE_CASE_INSENSITIVE)
      wcsncpy((wchar_t *)dname, (const wchar_t *)(nrp->name), dlen/2);
    dtype   =  nrp->ntype;
    dinfo   =  nrp->info;
  }
  return (U32)nrp;
}


U32 NameStore::GetNext(U32 curr, TCIString& dname, U32& dtype, U32& dinfo) {

  NAME_REC* nrp  =  (NAME_REC*)curr;
  if (nrp == (NAME_REC*)NULL) nrp =  head;
  else                          nrp =  nrp->next;
  if (nrp != (NAME_REC*)NULL) {
    TCI_ASSERT (m_mode == NSM_CASE_SENSITIVE || m_mode == NSM_CASE_INSENSITIVE);
    dname.Empty();
    dname.AppendChars(nrp->name, nrp->nlen);
    dtype   =  nrp->ntype;
    dinfo   =  nrp->info;
  }
  return (U32)nrp;
}


U32 NameStore::GetNextRecord(U32 curri, U32 targtype,
                                U8* dname, U32& dlen, U32& dtype, U32& dinfo) const {

  NAME_REC* curr;
  U32 counter =  0;

  for (U32 i=0; i<tablesize; i++) {   // loop down table slots
    curr  =  htable[i];
    while (curr) {       // loop down chain in slot
      counter++;
      if (counter > curri) {
        if (!targtype || curr->ntype==targtype) {
          dlen    =  curr->nlen;
          if (m_mode == NSM_CASE_SENSITIVE || m_mode == NSM_CASE_INSENSITIVE)
            strncpy((char*)dname, curr->name, dlen);
          else if (m_mode == NSM_WIDE_CASE_SENSITIVE || m_mode == NSM_WIDE_CASE_INSENSITIVE)
            wcsncpy((wchar_t *)dname, (const wchar_t *)(curr->name), dlen/2);
          dtype   =  curr->ntype;
          dinfo   =  curr->info;
          return counter;
        }
      }
      curr =  curr->alpha;
    }
  }
  return 0;
}


void NameStore::ChangeType(const U8* nom, U32 nomlen, U32 oldtype, U32 newtype) {

  NAME_REC* namerec =  GetNameRec(nom, nomlen, oldtype);
  if (namerec != (NAME_REC*)NULL) {
    namerec->ntype  =  newtype;
    m_bModified =  TRUE;
  }
}


TCI_BOOL NameStore::PutDataWithName(const U8* newname, U32 newlen,
                                    U32 newtype, U32 newinfo) {

  NAME_REC* namerec =  GetNameRec(newname, newlen, newtype);

  TCI_BOOL rv =  TRUE;
  if (namerec == (NAME_REC*)NULL)
    PushName(newname, newlen, newtype, newinfo, TRUE);
  else {
    if (dodelete && namerec->info!=0L) delete (void*)namerec->info;
    namerec->info   =  newinfo;
    m_bModified =  TRUE;
  }
  return rv;
}


TCI_BOOL NameStore::IsRedundant(const U8* targname, U32 targlen, U32 targtype) {

  NAME_REC* currrec =  head;
  while (currrec!=(NAME_REC*)NULL) {
    if (currrec->nlen==targlen && currrec->ntype==targtype) {
      if (m_mode == NSM_CASE_SENSITIVE) {
        if (!strncmp(currrec->name, (char*)targname, targlen))
          return TRUE;
      } else if (m_mode == NSM_CASE_INSENSITIVE){
        if (!_strnicmp(currrec->name, (char*)targname, targlen))
          return TRUE;
      } else if (m_mode == NSM_WIDE_CASE_SENSITIVE){
        if (!wcsncmp((const wchar_t *)currrec->name, (const wchar_t *)targname, targlen/2))
          return TRUE;
      } else if (m_mode == NSM_WIDE_CASE_INSENSITIVE){
        if (!_wcsnicmp((const wchar_t *)currrec->name, (const wchar_t *)targname, targlen/2))
          return TRUE;
      }
    }
    currrec =  currrec->next;
  }

  return FALSE;
}


TCI_BOOL NameStore::DeleteName(const U8* targname, U32 targlen, U32 targtype) {

  TCI_BOOL rv  =  FALSE;

  NAME_REC* targrec =  GetNameRec(targname, targlen, targtype);
  if (targrec != (NAME_REC*)NULL) {
    PopName(targname, targlen, targrec);
    rv  =  TRUE;
  }

  return rv;
}


#if TCIENV(CHAMNOWIN)
void NameStore::Dump() {

  NAME_REC* currrec;

  for (U32 i=0; i<tablesize; i++) {
    printf("\nchain #%d: ", i);
    currrec =  htable[i];
    while (currrec!=(NAME_REC*)NULL) {
      for (I32 zz=0; zz<currrec->nlen; zz++)
        printf("%c", currrec->name[zz]);
      if (currrec->ntype==4)  printf("*");
      printf(", ");
      currrec =  currrec->alpha;
    }
  }

  printf("\nhead to tail ");
  currrec =  head;
  while (currrec!=(NAME_REC*)NULL) {
    for (I32 zz=0; zz<currrec->nlen; zz++)
      printf("%c", currrec->name[zz]);
    if (currrec->ntype==4)  printf("*");
    printf(", ");
    currrec =  currrec->next;
  }
  printf("\n\n");

}
#endif

#ifdef NS_MONITOR

void NameStore::MonitorOn() {
  domonitor = TRUE;
  if (MonDataPtr != (NSMonitorData *) NULL) {
    delete MonDataPtr->slot_info;
    delete MonDataPtr;
  }
  MonDataPtr = TCI_NEW(NSMonitorData);

  MonDataPtr->slot_info = TCI_NEW(each_slot_info[tablesize]);

  MonDataPtr->num_slots = tablesize;
  MonDataPtr->num_names = 0;
  MonDataPtr->cur_depth_stack = 1; // not maintained in this version
  MonDataPtr->max_depth_stack = 1; // not maintained in this version
  int slot;
  for (slot = 0; slot < tablesize; ++slot) {
    MonDataPtr->slot_info[slot].keys     = 0;
    MonDataPtr->slot_info[slot].accesses = 0;
  }
}


void NameStore::MonitorOff() {
  domonitor = FALSE;
}


U32 GetListLength(NAME_REC* rec) {
  U32 count = 0;

  while (rec!=(NAME_RECP) NULL) {       // use parameter variable!
    ++count;
    rec = rec->alpha;
  }

  return count;
}


U32 GetListBytes(NAME_REC* rec) {
  U32 count = 0;

  while (rec!=(NAME_RECP) NULL) {       // use parameter variable!
    count += rec->nlen;
    rec    = rec->alpha;
  }

  return count;
}


NSMonitorData* NameStore::MonitorData() {
  U32 slot;
  MonDataPtr->num_names = 0;
  MonDataPtr->name_bytes = 0;
  MonDataPtr->min_keys_per_slot = MonDataPtr->max_keys_per_slot = GetListLength(htable[0]);
  for (slot = 0; slot < tablesize; ++slot) {
    U32 len = GetListLength(htable[ slot ]);
    MonDataPtr->slot_info[slot].keys = len;
    MonDataPtr->min_keys_per_slot = CHAMmin(len, MonDataPtr->min_keys_per_slot);
    MonDataPtr->max_keys_per_slot = CHAMmax(len, MonDataPtr->max_keys_per_slot);
    MonDataPtr->num_names += len;
    MonDataPtr->name_bytes += GetListBytes(htable[ slot ]);
  }
  return MonDataPtr;
}


void NameStore::PrintMonitorData(TCI_BOOL PrintSlotInfo) {
  TCI_ASSERT(MonDataPtr);
  (void) MonitorData();

  printf("Number of hash slots   = %hu\n", MonDataPtr->num_slots);
  printf("Number of names stored = %u\n",  MonDataPtr->num_names);

  printf("Number of bytes stored = %u\n",  MonDataPtr->name_bytes);
  if (MonDataPtr->num_names)
    printf("Average bytes per name = %2.2f\n", (double) MonDataPtr->name_bytes / MonDataPtr->num_names);

  printf("Maximum names per slot = %hu\n", MonDataPtr->max_keys_per_slot);
  printf("Minimum names per slot = %hu\n", MonDataPtr->min_keys_per_slot);

  printf("Current stack depth = %hu\n", MonDataPtr->cur_depth_stack);
  printf("Maximum stack depth = %hu\n", MonDataPtr->max_depth_stack);

  if (PrintSlotInfo) {
    U32 slot;
    for (slot = 0; slot < MonDataPtr->num_slots; ++slot) {
      printf("\tSlot % 3hu:  names % 5hu, accesses % 5u\n", slot, MonDataPtr->slot_info[slot].keys, MonDataPtr->slot_info[slot].accesses);
    }
  }
}

#endif
