// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

#ifndef MUPD_ENG_H
#define MUPD_ENG_H

#include <tcistrin.h>
#include "nsILocalFile.h"

typedef class MUP_KernelImpl * MUP_Kernel;
class MuPADMssgHandler;

// It is understood that this is a singleton, but that's not enforced.
class MuPADEngine
{
public:
  MuPADEngine();
  ~MuPADEngine();

  bool LoadStrsAndDLL(nsILocalFile *engpath, nsILocalFile *libpath);
  int Initialize(nsILocalFile *libpath, nsILocalFile *champath);
  bool SendToProcessor(TCIString &cmd);
  bool SendPlotToProcessor(TCIString &cmd);
  void ClearEngStrPtrs();
  int GetEngStrPtr(int str_ID, const char **engstr, int *engstr_ln);
  void CleanupAndUnloadDLL();

protected:
  int PrepareProcessor(const char *champath);

  bool Exec(TCIString &cmd);
  void HandleMuPADException();

  void WaitToFinish();
  void WaitToStart();
  
private:
  MUP_Kernel m_kernel;
  MuPADMssgHandler *m_client;

  bool fcnsloaded;
  bool plot_status;
  bool m_useDLL;

  TCIString eng_instr;
  bool instr_sent;
  TCIString eng_outstr;
  bool outstr_sent;
  TCIString eng_errorstr;
  bool errorstr_sent;
};

#endif
