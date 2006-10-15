// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

#ifndef MUPD_ENG_H
#define MUPD_ENG_H

#include <ByteArry.h>
#include <TCIStrin.h>

class MTkernel;
class MuPADMssgHandler;

// It is understood that this is a singleton, but that's not enforced.
class MuPADEngine
{
public:
  MuPADEngine();
  ~MuPADEngine();

  bool LoadStrsAndDLL(const char *engpath, const char *libpath);
  int Initialize(const char *libpath, const char *champath);
  bool SendToProcessor(ByteArray * ba);
  bool SendPlotToProcessor(ByteArray * ba);
  int GetEngStrPtr(int str_ID, char **engstr, int *engstr_ln);

  void CleanupAndUnloadDLL();

private:
  int PrepareProcessor(void *dummy, const char *champath);
  bool SendLowLevel(ByteArray * ba, bool semicolon);
  bool PackageIDtoName(U32 package_ID, U8 * dest);
  void ReadLib(const char *name);
  void DoReadLibs(const U8 * packs, const U8 * name);
  void CollectGarbage();
  void ReportMemoryUse(U32 how);

  bool Exec(ByteArray & ba);
  void WaitToFinish();
  void WaitToStart();
  void HandleMuPADException();

  char *AppendBA2HeapStr(char *zheap_str, U32 & buffer_ln, ByteArray & ba);

private:
  bool m_useDLL;
  MTkernel *m_kernel;
  static MuPADMssgHandler *m_client;

  bool fcnsloaded;
  bool plot_status;
  char *zeng_instr;
  U32 zeng_inln;
  bool instr_sent;
  char *zeng_outstr;
  U32 zeng_outln;
  bool outstr_sent;
  char *zeng_errorstr;
  U32 zeng_errorln;
  bool errorstr_sent;

  static const TCICHAR *m_VCAMSWP_GUID_STR;
  static const TCICHAR *m_VCAMPRO_GUID_STR;
  static const TCICHAR *m_MuPADSWP_GUID_STR;
  static const TCICHAR *m_MuPADPRO_GUID_STR;

};

#endif
