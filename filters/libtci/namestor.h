#ifndef NAMESTOR_H
#define NAMESTOR_H

/*
  NameStore -  a hash table store for named objects.
    Names are case sensitive.  A 32-bit item is stored with each name.
*/

// Define this if you want monitoring enabled.
// #define NS_MONITOR

#ifndef CHMTYPES_H
  #include "chmtypes.h"
#endif

class TCIString;

#define NS_MATCH_ALL  3210


typedef struct tagNAME_REC {
  tagNAME_REC* alpha;           // next record in hash chain
  tagNAME_REC* next;            // next record chronologically
  U32 ntype;
  U32 info;
  U32 nlen;
  char name[2];
} NAME_REC;

typedef NAME_REC* NAME_RECP;



struct each_slot_info {
  U32 keys;
  U32 accesses;
};

typedef struct {
  U32 num_slots;
  U32 num_names;
  U32 name_bytes;
  U32 cur_depth_stack;
  U32 max_depth_stack;
  U32 min_keys_per_slot;
  U32 max_keys_per_slot;
  struct each_slot_info * slot_info;
} NSMonitorData;



class NameStore {

public:

  NameStore(U32 numchains, TCI_BOOL delrecs);
  ~NameStore();

  TCI_BOOL  PutName(const U8* newname, U32 newlen,
                        U32 newtype, U32 newinfo, TCI_BOOL is_global);
  TCI_BOOL  PutName(const TCIString& newname, U32 newtype, U32 newinfo, 
                                                      TCI_BOOL is_global);
  TCI_BOOL  PutDataWithName(const U8* newname, U32 newlen, U32 newtype, U32 newinfo);
  TCI_BOOL  GetName(const U8* targname, U32 targlen, U32 targtype, U32& theinfo);
  TCI_BOOL  GetName(const TCIString& targname, U32 targtype, U32& theinfo);
  TCI_BOOL  DeleteName(const U8* targname, U32 targlen, U32 targtype);

  void      PushName(const U8* newname, U32 newlen,
                        U32 newtype, U32 newinfo, TCI_BOOL is_global);

  U32       GetCurrTail();
  void      ReleaseTail(U32 newtail);
  U32       GetNext(U32 curr, U8* dname, U32& dlen, U32& dtype, U32& dinfo);
  U32       GetNext(U32 curr, TCIString& dname, U32& dtype, U32& dinfo);

  U32       GetNextRecord(U32 curri, U32 targtype,
                            U8* dname, U32& dlen, U32& dtype, U32& dinfo) const;

  TCI_BOOL  IsRedundant(const U8* targname, U32 targlen, U32 targtype);
  void      ChangeType(const U8* nom, U32 nomlen, U32 oldtype, U32 newtype);

  enum      NS_MODE{ NSM_CASE_SENSITIVE, NSM_CASE_INSENSITIVE, 
                  NSM_WIDE_CASE_SENSITIVE, NSM_WIDE_CASE_INSENSITIVE };

  void      SetCaseSensitivity(NS_MODE mode) { m_mode  =  mode; }

  TCI_BOOL  IsModified() const {return m_bModified;}
  void      ClearModified() {m_bModified = FALSE;}

// --- These are implemented only #ifdef NS_MONITOR
  void      MonitorOn();
  void      MonitorOff();
  NSMonitorData* MonitorData();
  void      PrintMonitorData(TCI_BOOL PrintSlotInfo = FALSE);
// --- #endif

// Dump is implemented only if CHAMNOWIN
  void      Dump();

private:    // functions
  U32         Hash(const U8* newname, U32 len);
  NAME_REC*   GetNameRec(const U8* targname, U32 targlen, U32 targtype);
  void        PopName(const U8* targname, U32 targlen, NAME_REC* poprec);


private:    // data
  NAME_RECP*  htable;
  U32         tablesize;
  TCI_BOOL    dodelete;
  NAME_REC*   head;
  NAME_REC*   tail;
  NS_MODE     m_mode;
  TCI_BOOL    m_bModified;
  
// --- These are implemented only #ifdef NS_MONITOR
  TCI_BOOL    domonitor;
  NSMonitorData * MonDataPtr;
// --- #endif
};

#endif

