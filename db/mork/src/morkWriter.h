/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-  */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1999
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either of the GNU General Public License Version 2 or later (the "GPL"),
 * or the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef _MORKWRITER_
#define _MORKWRITER_ 1

#ifndef _MORK_
#include "mork.h"
#endif

#ifndef _MORKNODE_
#include "morkNode.h"
#endif

#ifndef _MORKMAP_
#include "morkMap.h"
#endif

#ifndef _MORKROWMAP_
#include "morkRowMap.h"
#endif

#ifndef _MORKTABLE_
#include "morkTable.h"
#endif

#ifndef _MORKATOMMAP_
#include "morkAtomMap.h"
#endif

#ifndef _MORKATOMSPACE_
#include "morkAtomSpace.h"
#endif

#ifndef _MORKROWSPACE_
#include "morkRowSpace.h"
#endif

#ifndef _MORKSTREAM_
#include "morkStream.h"
#endif

//3456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789


#define morkWriter_kStreamBufSize /*i*/ (16 * 1024) /* buffer size for stream */ 

#define morkDerived_kWriter  /*i*/ 0x5772 /* ascii 'Wr' */

#define morkWriter_kPhaseNothingDone          0 /* nothing has yet been done */
#define morkWriter_kPhaseDirtyAllDone         1 /* DirtyAll() is done */
#define morkWriter_kPhasePutHeaderDone        2 /* PutHeader() is done */

#define morkWriter_kPhaseRenumberAllDone      3 /* RenumberAll() is done */

#define morkWriter_kPhaseStoreAtomSpaces      4 /*mWriter_StoreAtomSpacesIter*/
#define morkWriter_kPhaseAtomSpaceAtomAids    5 /*mWriter_AtomSpaceAtomAidsIter*/

#define morkWriter_kPhaseStoreRowSpacesTables 6 /*mWriter_StoreRowSpacesIter*/
#define morkWriter_kPhaseRowSpaceTables       7 /*mWriter_RowSpaceTablesIter*/
#define morkWriter_kPhaseTableRowArray        8 /*mWriter_TableRowArrayPos */

#define morkWriter_kPhaseStoreRowSpacesRows   9 /*mWriter_StoreRowSpacesIter*/
#define morkWriter_kPhaseRowSpaceRows        10 /*mWriter_RowSpaceRowsIter*/

#define morkWriter_kPhaseContentDone         11 /* all content written */
#define morkWriter_kPhaseWritingDone         12 /* everthing has been done */

#define morkWriter_kCountNumberOfPhases      13 /* part of mWrite_TotalCount */

#define morkWriter_kMaxColumnNameSize        128 /* longest writable col name */

#define morkWriter_kMaxIndent 66 /* default value for mWriter_MaxIndent */
#define morkWriter_kMaxLine   78 /* default value for mWriter_MaxLine */

#define morkWriter_kYarnEscapeSlop  4 /* guess average yarn escape overhead */

#define morkWriter_kTableMetaCellDepth 4 /* */
#define morkWriter_kTableMetaCellValueDepth 6 /* */

#define morkWriter_kDictMetaCellDepth 4 /* */
#define morkWriter_kDictMetaCellValueDepth 6 /* */

#define morkWriter_kDictAliasDepth 2 /* */
#define morkWriter_kDictAliasValueDepth 4 /* */

#define morkWriter_kRowDepth 2 /* */
#define morkWriter_kRowCellDepth 4 /* */
#define morkWriter_kRowCellValueDepth 6 /* */

#define morkWriter_kGroupBufSize 64 /* */

// v=1.1 retired on 23-Mar-99 (for metainfo one char column names)
// v=1.2 retired on 20-Apr-99 (for ":c" suffix on table kind hex refs)
// v=1.3 retired on 20-Apr-99 (for 1CE:m instead of ill-formed 1CE:^6D)
#define morkWriter_kFileHeader "// <!-- <mdb:mork:z v=\"1.4\"/> -->"

class morkWriter : public morkNode { // row iterator

// public: // slots inherited from morkObject (meant to inform only)
  // nsIMdbHeap*     mNode_Heap;
  // mork_able    mNode_Mutable; // can this node be modified?
  // mork_load    mNode_Load;    // is this node clean or dirty?
  // mork_base    mNode_Base;    // must equal morkBase_kNode
  // mork_derived mNode_Derived; // depends on specific node subclass
  // mork_access  mNode_Access;  // kOpen, kClosing, kShut, or kDead
  // mork_usage   mNode_Usage;   // kHeap, kStack, kMember, kGlobal, kNone
  // mork_uses    mNode_Uses;    // refcount for strong refs
  // mork_refs    mNode_Refs;    // refcount for strong refs + weak refs

public: // state is public because the entire Mork system is private

  morkStore*   mWriter_Store;      // weak ref to committing store
  nsIMdbFile*  mWriter_File;       // strong ref to store's file
  nsIMdbFile*  mWriter_Bud;        // strong ref to bud of mWriter_File
  morkStream*  mWriter_Stream;     // strong ref to stream on bud file
  nsIMdbHeap*  mWriter_SlotHeap;   // strong ref to slot heap

  // GroupIdentity should be based on mStore_CommitGroupIdentity:
  mork_gid     mWriter_CommitGroupIdentity; // transaction ID number
  
  // GroupBuf holds a hex version of mWriter_CommitGroupIdentity:
  char         mWriter_GroupBuf[ morkWriter_kGroupBufSize ];
  mork_fill    mWriter_GroupBufFill; // actual bytes in GroupBuf
  
  mork_count   mWriter_TotalCount;  // count of all things to be written
  mork_count   mWriter_DoneCount;   // count of things already written
  
  mork_size    mWriter_LineSize;  // length of current line being written
  mork_size    mWriter_MaxIndent; // line size forcing a line break
  mork_size    mWriter_MaxLine;   // line size forcing a value continuation
  
  mork_cscode  mWriter_TableForm;     // current charset metainfo
  mork_scope   mWriter_TableAtomScope;   // current atom scope
  mork_scope   mWriter_TableRowScope;    // current row scope
  mork_kind    mWriter_TableKind;        // current table kind
  
  mork_cscode  mWriter_RowForm;         // current charset metainfo
  mork_scope   mWriter_RowAtomScope;    // current atom scope
  mork_scope   mWriter_RowScope;        // current row scope
  
  mork_cscode  mWriter_DictForm;      // current charset metainfo
  mork_scope   mWriter_DictAtomScope;    // current atom scope
 
  mork_bool    mWriter_NeedDirtyAll;  // need to call DirtyAll()
  mork_bool    mWriter_Incremental;   // opposite of mWriter_NeedDirtyAll
  mork_bool    mWriter_DidStartDict;  // true when a dict has been started
  mork_bool    mWriter_DidEndDict;    // true when a dict has been ended

  mork_bool    mWriter_SuppressDirtyRowNewline; // for table meta rows
  mork_bool    mWriter_DidStartGroup; // true when a group has been started
  mork_bool    mWriter_DidEndGroup;    // true when a group has been ended
  mork_u1      mWriter_Phase;         // status of writing process

  mork_bool    mWriter_BeVerbose; // driven by env and table verbose settings:
  // mWriter_BeVerbose equals ( ev->mEnv_BeVerbose || table->IsTableVerbose() )
  
  mork_u1      mWriter_Pad[ 3 ];  // for u4 alignment

  mork_pos     mWriter_TableRowArrayPos;  // index into mTable_RowArray
   
  char         mWriter_SafeNameBuf[ (morkWriter_kMaxColumnNameSize * 2) + 4 ];
  // Note: extra four bytes in ColNameBuf means we can always append to yarn

  char         mWriter_ColNameBuf[ morkWriter_kMaxColumnNameSize + 4 ];
  // Note: extra four bytes in ColNameBuf means we can always append to yarn
  
  mdbYarn      mWriter_ColYarn; // a yarn to describe space in ColNameBuf:
  // mYarn_Buf == mWriter_ColNameBuf, mYarn_Size == morkWriter_kMaxColumnNameSize
  
  mdbYarn      mWriter_SafeYarn; // a yarn to describe space in ColNameBuf:
  // mYarn_Buf == mWriter_SafeNameBuf, mYarn_Size == (kMaxColumnNameSize * 2)

  morkAtomSpaceMapIter  mWriter_StoreAtomSpacesIter;   // for mStore_AtomSpaces
  morkAtomAidMapIter  mWriter_AtomSpaceAtomAidsIter; // for AtomSpace_AtomAids
  
  morkRowSpaceMapIter  mWriter_StoreRowSpacesIter;    // for mStore_RowSpaces
  morkTableMapIter  mWriter_RowSpaceTablesIter;    // for mRowSpace_Tables
  
#ifdef MORK_ENABLE_PROBE_MAPS
  morkRowProbeMapIter  mWriter_RowSpaceRowsIter; // for mRowSpace_Rows
#else /*MORK_ENABLE_PROBE_MAPS*/
  morkRowMapIter  mWriter_RowSpaceRowsIter;      // for mRowSpace_Rows
#endif /*MORK_ENABLE_PROBE_MAPS*/
   
// { ===== begin morkNode interface =====
public: // morkNode virtual methods
  virtual void CloseMorkNode(morkEnv* ev); // CloseWriter()
  virtual ~morkWriter(); // assert that close executed earlier
  
public: // morkWriter construction & destruction
  morkWriter(morkEnv* ev, const morkUsage& inUsage,
    nsIMdbHeap* ioHeap, morkStore* ioStore, nsIMdbFile* ioFile,
    nsIMdbHeap* ioSlotHeap);
  void CloseWriter(morkEnv* ev); // called by CloseMorkNode();

private: // copying is not allowed
  morkWriter(const morkWriter& other);
  morkWriter& operator=(const morkWriter& other);

public: // dynamic type identification
  mork_bool IsWriter() const
  { return IsNode() && mNode_Derived == morkDerived_kWriter; }
// } ===== end morkNode methods =====

public: // typing & errors
  static void NonWriterTypeError(morkEnv* ev);
  static void NilWriterStoreError(morkEnv* ev);
  static void NilWriterBudError(morkEnv* ev);
  static void NilWriterStreamError(morkEnv* ev);
  static void NilWriterFileError(morkEnv* ev);
  static void UnsupportedPhaseError(morkEnv* ev);

public: // utitlities
  void ChangeRowForm(morkEnv* ev, mork_cscode inNewForm);
  void ChangeDictForm(morkEnv* ev, mork_cscode inNewForm);
  void ChangeDictAtomScope(morkEnv* ev, mork_scope inScope);

public: // inlines
  mork_bool DidStartDict() const { return mWriter_DidStartDict; }
  mork_bool DidEndDict() const { return mWriter_DidEndDict; }
  
  void IndentAsNeeded(morkEnv* ev, mork_size inDepth)
  { 
    if ( mWriter_LineSize > mWriter_MaxIndent )
      mWriter_LineSize = mWriter_Stream->PutIndent(ev, inDepth);
  }
  
  void IndentOverMaxLine(morkEnv* ev,
    mork_size inPendingSize, mork_size inDepth)
  { 
    if ( mWriter_LineSize + inPendingSize > mWriter_MaxLine )
      mWriter_LineSize = mWriter_Stream->PutIndent(ev, inDepth);
  }

public: // delayed construction

  void MakeWriterStream(morkEnv* ev); // give writer a suitable stream

public: // iterative/asynchronous writing
  
  mork_bool WriteMore(morkEnv* ev); // call until IsWritingDone() is true
  
  mork_bool IsWritingDone() const // don't call WriteMore() any longer?
  { return mWriter_Phase == morkWriter_kPhaseWritingDone; }

public: // marking all content dirty
  mork_bool DirtyAll(morkEnv* ev);
  // DirtyAll() visits every store sub-object and marks 
  // them dirty, including every table, row, cell, and atom.  The return
  // equals ev->Good(), to show whether any error happened.  This method is
  // intended for use in the beginning of a "compress commit" which writes
  // all store content, whether dirty or not.  We dirty everything first so
  // that later iterations over content can mark things clean as they are
  // written, and organize the process of serialization so that objects are
  // written only at need (because of being dirty).  Note the method can 
  // stop early when any error happens, since this will abort any commit.

public: // group commit transactions

  mork_bool StartGroup(morkEnv* ev);
  mork_bool CommitGroup(morkEnv* ev);
  mork_bool AbortGroup(morkEnv* ev);

public: // phase methods
  mork_bool OnNothingDone(morkEnv* ev);
  mork_bool OnDirtyAllDone(morkEnv* ev);
  mork_bool OnPutHeaderDone(morkEnv* ev);

  mork_bool OnRenumberAllDone(morkEnv* ev);

  mork_bool OnStoreAtomSpaces(morkEnv* ev);
  mork_bool OnAtomSpaceAtomAids(morkEnv* ev);

  mork_bool OnStoreRowSpacesTables(morkEnv* ev);
  mork_bool OnRowSpaceTables(morkEnv* ev);
  mork_bool OnTableRowArray(morkEnv* ev);

  mork_bool OnStoreRowSpacesRows(morkEnv* ev);
  mork_bool OnRowSpaceRows(morkEnv* ev);

  mork_bool OnContentDone(morkEnv* ev);
  mork_bool OnWritingDone(morkEnv* ev);

public: // writing dict items first pass
  mork_bool PutTableDict(morkEnv* ev, morkTable* ioTable);
  mork_bool PutRowDict(morkEnv* ev, morkRow* ioRow);

public: // writing node content second pass
  mork_bool PutTable(morkEnv* ev, morkTable* ioTable);
  mork_bool PutRow(morkEnv* ev, morkRow* ioRow);
  mork_bool PutRowCells(morkEnv* ev, morkRow* ioRow);
  mork_bool PutVerboseRowCells(morkEnv* ev, morkRow* ioRow);
  
  mork_bool PutCell(morkEnv* ev, morkCell* ioCell, mork_bool inWithVal);
  mork_bool PutVerboseCell(morkEnv* ev, morkCell* ioCell, mork_bool inWithVal);
  
  mork_bool PutTableChange(morkEnv* ev, const morkTableChange* inChange);

public: // other writer methods

  mork_bool IsYarnAllValue(const mdbYarn* inYarn);

  mork_size WriteYarn(morkEnv* ev, const mdbYarn* inYarn);
  // return number of atom bytes written on the current line (which
  // implies that escaped line breaks will make the size value smaller
  // than the entire yarn's size, since only part goes on a last line).

  mork_size WriteAtom(morkEnv* ev, const morkAtom* inAtom);
  // return number of atom bytes written on the current line (which
  // implies that escaped line breaks will make the size value smaller
  // than the entire atom's size, since only part goes on a last line).

  void WriteAllStoreTables(morkEnv* ev);
  void WriteAtomSpaceAsDict(morkEnv* ev, morkAtomSpace* ioSpace);
  
  void WriteTokenToTokenMetaCell(morkEnv* ev, mork_token inCol,
    mork_token inValue);
  void WriteStringToTokenDictCell(morkEnv* ev, const char* inCol, 
    mork_token inValue);
  // Note inCol should begin with '(' and end with '=', with col in between.

  void StartDict(morkEnv* ev);
  void EndDict(morkEnv* ev);

  void StartTable(morkEnv* ev, morkTable* ioTable);
  void EndTable(morkEnv* ev);

public: // typesafe refcounting inlines calling inherited morkNode methods
  static void SlotWeakWriter(morkWriter* me,
    morkEnv* ev, morkWriter** ioSlot)
  { morkNode::SlotWeakNode((morkNode*) me, ev, (morkNode**) ioSlot); }
  
  static void SlotStrongWriter(morkWriter* me,
    morkEnv* ev, morkWriter** ioSlot)
  { morkNode::SlotStrongNode((morkNode*) me, ev, (morkNode**) ioSlot); }
};

//3456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789

#endif /* _MORKTABLEROWCURSOR_ */
