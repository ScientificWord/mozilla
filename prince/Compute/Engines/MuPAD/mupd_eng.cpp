// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

#ifdef TESTING
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include <windows.h>

#include <chmtypes.h>
#include "../fltutils.h"

#include "mupd_eng.h"

#include <setjmp.h>
#include <sys/stat.h>
#include <delayimp.h>
#include "OLEutils.h"
#include "unknwn.h"

#include "tcistrin.h"

#include "MKT_com_kernel.h"
#include "MKT_lib_kernel.h"

static jmp_buf mupad_mark;      // Address for long jump to jump to -- used in case of MuPAD DLL crash

#define USE_MuPAD_30 1
#ifdef USE_MuPAD_30
const TCICHAR *MuPADEngine::m_VCAMSWP_GUID_STR =
  "{1141E11F-C863-4EDF-AB2D-1031BE573CB3}";
const TCICHAR *MuPADEngine::m_VCAMPRO_GUID_STR =
  "{AD0749C0-BBB4-11D0-93DF-0000C059B7AD}";
const TCICHAR *MuPADEngine::m_MuPADSWP_GUID_STR =
  "{63680B2D-EE7E-42C8-AB45-AB46CD4D4360}";
const TCICHAR *MuPADEngine::m_MuPADPRO_GUID_STR =
  "{15922A38-0266-47bb-BE10-441A425607F0}";
#else
const TCICHAR *MuPADEngine::m_VCAMSWP_GUID_STR =
  "{1141E11F-C863-4EDF-AB2D-1031BE573CB3}";  // don't have this yet
const TCICHAR *MuPADEngine::m_VCAMPRO_GUID_STR =
  "{AD0749C0-BBB4-11D0-93DF-0000C059B7AD}";
const TCICHAR *MuPADEngine::m_MuPADSWP_GUID_STR =
  "{63680B2D-EE7E-42C8-AB45-AB46CD4D4360}";  // don't need this out of process kernel
const TCICHAR *MuPADEngine::m_MuPADPRO_GUID_STR =
  "{ED233FD4-C924-47b1-B855-4E305CF9706B}";
#endif

MuPADMssgHandler *MuPADEngine::m_client = 0;

// our kernel message handler class
class MuPADMssgHandler : public MTclientMessageHandler
{
public:
  MuPADMssgHandler() : ready(false), usingDLL(true), errorFlag(false),
    warningFlag(false), cmd_cookie(0)
  {
  }
  virtual ~MuPADMssgHandler()
  {
  }

  void on_kernel_ready();
  void on_init_wanted();
  void on_output(void *cmd_cookie, const char *output_type,
                 MTmemoryData * output);
  void on_finished(void *cmd_cookie);
  void on_status(size_t used_mem, size_t res_mem, size_t sec);  // called all the time
  void on_plot_result_file(void *cmd_cookie, const char *fname);

  bool Ready()
  {
    return ready;
  }
  const TCIString & GetReturn()
  {
    returnStr.TrimRight();
    return returnStr;
  }
  void ClearReturn()
  {
    ready = false;
    returnStr.Empty();
  }

  void UseDLL(bool useit)
  {
    usingDLL = useit;
  }

  bool Error()
  {
    return errorFlag;
  }
  const TCIString & GetError()
  {
    return errorStr;
  }
  void ClearError()
  {
    errorFlag = false;
    errorStr.Empty();
  }

  bool Warning()
  {
    return warningFlag;
  }
  const TCIString & GetWarning()
  {
    return warningStr;
  }
  void ClearWarning()
  {
    warningFlag = false;
    warningStr.Empty();
  }

  const TCIString & GetStatus()
  {
    return statusStr;
  }
  const TCIString & GetPlotFile()
  {
    return plotfileStr;
  }
  void ResetPlotFile()
  {
    plotfileStr.Empty();
  }

  U32 Cookie()
  {
    return cmd_cookie;
  }
  void Interrupted()
  {
    ++cmd_cookie;
  }                             // synchronization?

  void SetLibPath(const TCIString & path)
  {
    libStr = path;
  }

private:
  bool ready;
  bool usingDLL;
  bool errorFlag;
  bool warningFlag;
  TCIString returnStr;
  TCIString errorStr;
  TCIString warningStr;
  TCIString statusStr;
  TCIString plotfileStr;
  U32 cmd_cookie;

  TCIString libStr;
};

// the kernel is ready to go
void MuPADMssgHandler::on_kernel_ready()
{
  ready = true;
}

//   User name:    MacKichan
//   Key:          81000-57A6F-A4EDC-67BC6-45F97
/*
static
TCIString OEMname()
{
  return TCIString("MacKichan");
}

static
TCIString OEMkeyword()
{
  TCIString key;
  // reconstruct our key (so hacker can't see in string data)
  key  =       StrUtil::HexString(0x810000,6,false);
  key += "-" + StrUtil::HexString(0x57A6F,5,false);
  key += "-" + StrUtil::HexString(0xA4EDC,5,false);
  key += "-" + StrUtil::HexString(0x67BC6,5,false);
  key += "-" + StrUtil::HexString(0x45F97,5,false);
  return key;
}
*/
void MuPADMssgHandler::on_init_wanted()
{
  if (usingDLL) {
    char name[100], code[100];
    strcpy(name, "MacKichan");
    strcpy(code, "81000-57A6F-A4EDC-67BC6-45F97");
    char path[MAX_PATH], path2[MAX_PATH];
    strcpy(path, static_cast < const TCICHAR * >(libStr));
    strcpy(path2, static_cast < const TCICHAR * >(libStr));
    char *argv[8] = {
      "/N", name,
      "/K", code,
      "/l", path,
      "/m", path2
    };
    kernel()->send_init(8, argv);
  } else {
    kernel()->send_init(0, NULL);
  }
}

// an output from the kernel
void MuPADMssgHandler::on_output(void *cookie, const char *output_type,
                                 MTmemoryData * output)
{
  TCI_ASSERT(output->encoding() == MTformat::Ascii);
  if (output->encoding() != MTformat::Ascii)
    return;

  TCI_ASSERT((U32) cookie == cmd_cookie); // this result is for some other command 
  if ((U32) cookie != cmd_cookie)
    return;

  if (strcmp(MKot_result, output_type) == 0)
    returnStr += output->c_str();
  else if (strcmp(MKot_error, output_type) == 0 ||
           strcmp(MKot_SYSERROR, output_type) == 0) {
    errorFlag = true;
    errorStr += output->c_str();

  } else if (strcmp(MKot_warning, output_type) == 0 ||
             strcmp(MKot_print, output_type) == 0) {
    warningFlag = true;
    warningStr += output->c_str();
  } else
    TCI_ASSERT(false);          // what is this?
}

// kernel finished command output
void MuPADMssgHandler::on_finished(void *cookie)
{

  TCI_ASSERT((U32) cookie == cmd_cookie);
  if ((U32) cookie == cmd_cookie) {
    ++cmd_cookie;
    ready = true;
  }
}

// kernel gave us status
void MuPADMssgHandler::on_status(size_t used_mem, size_t res_mem, size_t sec)
{
//  TCIString buff;
//  StrUtil::LoadString(buff, MSG_COMPSTATUS);
//  statusStr =  StrUtil::Format(buff, used_mem, res_mem);
}

// kernel generated a plot file
void MuPADMssgHandler::on_plot_result_file(void *cookie, const char *fname)
{
  if ((U32) cookie == cmd_cookie)
    plotfileStr = fname;
  // else throw away
}

bool Stop_Interrupt()
{
  return false;
}

// MuPAD DLL is giving us a time slice during computation
static void Stop_MuPAD(void *data)
{
  MTlibKernel *ker = reinterpret_cast < MTlibKernel * >(data);
  if (ker && Stop_Interrupt())
    ker->send_stop();           // tell MuPAD to stop
}

// MuPAD DLL crashed and called fatal_exit_handler()
static void Cleanup_MuPAD()
{
  longjmp(mupad_mark, -1);
}

MuPADEngine::MuPADEngine() :
  m_useDLL(true), m_kernel(0)
{
  fcnsloaded = false;
  plot_status = false;

  zeng_instr = NULL;
  zeng_inln = 0;
  instr_sent = false;

  zeng_outstr = NULL;
  zeng_outln = 0;
  outstr_sent = false;

  zeng_errorstr = NULL;
  zeng_errorln = 0;
  errorstr_sent = false;
}

MuPADEngine::~MuPADEngine()
{
  delete[] zeng_instr;
  delete[] zeng_outstr;
  delete[] zeng_errorstr;

  if (!m_useDLL)
    delete m_kernel;
  delete m_client;
  m_client = 0;
}

int MuPADEngine::GetEngStrPtr(int str_ID, char **engstr, int *engstr_ln)
{
  int rv = 0;

  if (str_ID == 1) {            // engine input string
    if (zeng_instr) {
      *engstr = zeng_instr;
      *engstr_ln = strlen(zeng_instr);
      instr_sent = true;
      rv = 1;
    }
  } else if (str_ID == 2) {
    if (zeng_outstr) {
      *engstr = zeng_outstr;
      *engstr_ln = strlen(zeng_outstr);
      outstr_sent = true;
      rv = 1;
    }
  } else if (str_ID == 3) {
    if (zeng_errorstr) {
      *engstr = zeng_errorstr;
      *engstr_ln = strlen(zeng_errorstr);
      errorstr_sent = true;
      rv = 1;
    }
  } else {
    TCI_ASSERT(0);
  }
  return rv;
}

bool MuPADEngine::PackageIDtoName(U32 package_ID, U8 * dest)
{
  return false;
}

bool MuPADEngine::LoadStrsAndDLL(const char *engpath, const char *libpath)
{
  bool rv = false;
  TCIString ErrorMessage;

  // If our MuPAD is licensed, use it.  Otherwise use full version.
  // We let the user override this behavior with the following flag
  I32 PreferPro;
  //  inifiler->GetIniFileInt( Ini_MuPAD,Ini_PreferMuPADPro,PreferPro );
  //  bool LocalMuPAD = featureman->Have( FeatureMan::CAT_MATH, FeatureMan::MATH_MUPADLOCAL);
  PreferPro = 0;
  bool LocalMuPAD = true;
  CLSID MuPAD_clsid;
  TCI_VERIFY(SWOleUtils::TCIStringToCLSID(m_MuPADPRO_GUID_STR, &MuPAD_clsid));
  bool ProRegistered = SWOleUtils::CLSIDRegistered(&MuPAD_clsid);

  if (PreferPro && ProRegistered) {
    m_useDLL = false;
  } else if (!LocalMuPAD) {
    m_useDLL = false;
    if (!ProRegistered) {
      TCI_ASSERT(0);            // Message( MSG_WARNING,MSG_MuPADNotRegistered );
      return rv;
    }
  }
  // If our MuPAD is licensed, use it.  Otherwise use full version.
  CLSID VCamSWP_clsid, VCamPro_clsid;
  TCI_VERIFY(SWOleUtils::
             TCIStringToCLSID(m_VCAMSWP_GUID_STR, &VCamSWP_clsid));
  TCI_VERIFY(SWOleUtils::
             TCIStringToCLSID(m_VCAMPRO_GUID_STR, &VCamPro_clsid));
  ProRegistered = SWOleUtils::CLSIDRegistered(&VCamPro_clsid);

  if (LocalMuPAD && SWOleUtils::CLSIDRegistered(&VCamSWP_clsid) &&
      !(PreferPro && ProRegistered)) {
    //    m_pPlotStore = TCI_NEW(MuPlotStore(m_VCAMSWP_GUID_STR));
  } else {
    if (ProRegistered) {
      //      m_pPlotStore = TCI_NEW(MuPlotStore(m_VCAMPRO_GUID_STR));
    } else {
      //    Message( MSG_WARNING,MSG_MuPADNotRegistered );
      return rv;
    }
  }

  // start up kernel
  TCI_ASSERT(m_client == 0);
  m_client = TCI_NEW(MuPADMssgHandler);
  m_client->SetLibPath(libpath);
  m_client->UseDLL(m_useDLL);

  if (m_useDLL) {
    MTlibKernel::event_handler(Stop_MuPAD, 0);

    MTlibKernel *libKernel;
    if (MTlibKernel::get_kernel(m_client, libpath, &libKernel)) {
      m_kernel = libKernel;
      MTlibKernel::event_handler(Stop_MuPAD, libKernel);
      MTlibKernel::fatal_exit_handler(Cleanup_MuPAD);

      if (m_kernel->send_new_client(0, 0, MCstringLong, MCieeeFloat,
                                    MCflatCoding | MCidentCoding,
                                    MKcf_can_plot)) {
        if (m_client->Error()) {
          ErrorMessage = m_client->GetError();
          m_client->ClearError();
          rv = false;
        } else {
          rv = true;
        }
        m_client->ClearReturn();
      }
    }
  } else {
    // try for MuPAD Pro
    MTcomKernel *comKernel = 0;
    HRESULT hr = MTcomKernel::new_kernel(MuPAD_clsid, m_client, &comKernel);
    m_kernel = comKernel;
    if (hr == S_OK && comKernel) {
      // attach to kernel
      if (m_kernel->send_new_client(0, 0, MCstringLong, MCieeeFloat,
                                    MCflatCoding | MCidentCoding,
                                    MKcf_can_plot)) {
        WaitToStart();
        if (m_client->Error()) {
          ErrorMessage = m_client->GetError();
          m_client->ClearError();
          hr = comKernel->get_hresult();
          if (hr != 0 && ErrorMessage.IsEmpty())
            TCI_ASSERT(0);      // ErrorMessage = StrUtil::FormatLastError(hr);
          rv = false;
        } else {
          rv = true;
        }
        m_client->ClearReturn();
      }
    }
  }

  if (!rv) {
    TCI_ASSERT(0);              // Message( MSG_WARNING,MSG_MuPADStartupError,(const TCICHAR*)ErrorMessage );
    if (!m_useDLL)
      delete m_kernel;
    m_kernel = 0;
  }

  return rv;
}

int MuPADEngine::Initialize(const char *libpath, const char *champath)
{
  // set output formatting
  m_kernel->send_pretty_print(false);
  m_kernel->send_textwidth(0x7fffffff); // 2^31-1

  return PrepareProcessor(NULL, champath);
}

void MuPADEngine::CleanupAndUnloadDLL()
{

  if (m_kernel) {
    m_kernel->send_quit();
    if (!m_useDLL)
      delete m_kernel;
    m_kernel = 0;
  }
  delete m_client;
  m_client = 0;
  if (m_useDLL) {
    // name must match link line, exactly
    //    TCIString dll( StrUtil::GetResourceString( STR_MuPADDLL ) );
    //  __FUnloadDelayLoadedDLL( static_cast<const TCICHAR*>(dll) );
    //  __FUnloadDelayLoadedDLL( "mupkern.dll" );
  }
}

void MuPADEngine::HandleMuPADException()
{
  //  Message( MSG_WARNING,MSG_MuPADFailure );
  //  DelayLoadComputePackage( CHAM_MUPAD,true );
  //this kernel is corrupt, don't use it
  //TODO set an error code
  m_kernel = 0;
}

BOOL FileSys_Exists(const char *fspec)
{
  struct stat fs;
  return (stat(fspec, &fs) == 0);
}

int MuPADEngine::PrepareProcessor(void *dummy, const char *champath)
{
  // Do we need to load our special functions?
  if (!fcnsloaded) {
    ByteArray packba;
    TCIString path(champath);
    path += "/swfcns.mu";
    if (FileSys_Exists(path)) {
      TCIString fcnsPath(path);
      fcnsPath.Replace("\\", "/");
      packba += "fread(\"";
      packba += fcnsPath;
      packba += "\"):";
      fcnsloaded = SendToProcessor(&packba);
      packba.SetByteCount(0);
      return 1;
    } else {
      TCI_ASSERT(!"Unable to find swfcsn.mu");
      return 0;
    }
  }
  return 1;
}

bool MuPADEngine::SendToProcessor(ByteArray * ba)
{
  //The contents of ba are sent to MuPAD. If a valid return
  //is obtained, the result replaces the contents of ba.

  bool rv = false;

  rv = SendLowLevel(ba, false);

  return rv;
}

bool MuPADEngine::SendPlotToProcessor(ByteArray * ba)
{
  plot_status = true;
  bool rv = SendToProcessor(ba);
  plot_status = false;
  return rv;
}

bool MuPADEngine::SendLowLevel(ByteArray * ba, bool semicolon)
{

  bool rv = false;
  (*ba) += (U8) '\0';
  rv = Exec(*ba);

  return rv;
}

bool MuPADEngine::Exec(ByteArray & ba)
{

  bool rv = false;

  TCI_ASSERT(!m_client->Ready());
  //  parent->LogCmdStr( ba );

  if (instr_sent) {
    delete[] zeng_instr;
    zeng_instr = NULL;
    zeng_inln = 0;
    instr_sent = false;
  }

  zeng_instr = AppendBA2HeapStr(zeng_instr, zeng_inln, ba);

  const U8 *p = ba.ReadOnlyLock();

  // make a MuPAD language data from it
  MTbufferStream stream((unsigned char *)p, ba.GetByteCount());
  MTmemoryData cmd(MKfn_mupad_lang, MTformat::Ascii, &stream);

  // send command
  int send_cmd_ret = setjmp(mupad_mark);
  if (send_cmd_ret == 0) {
    if (!m_kernel->
        send_cmd(reinterpret_cast < void *>(m_client->Cookie()), &cmd))
    {
      ba.ReadOnlyUnlock();
      ba.SetByteCount(0);
      return rv;
    }
  } else {                      // MuPAD crashed via fatal_exit_handler()
    ba.ReadOnlyUnlock();
    ba.SetByteCount(0);
    HandleMuPADException();
    return rv;
  }

  WaitToFinish();
  ba.ReadOnlyUnlock();

  ba.SetByteCount(0);
  if (m_client->Error()) {
    TCI_ASSERT(0);
    //  parent->ReportError( m_client->GetError() );

    ba = m_client->GetError();
    if (errorstr_sent) {
      delete[] zeng_errorstr;
      zeng_errorstr = NULL;
      zeng_errorln = 0;
      errorstr_sent = false;
    }
    zeng_errorstr = AppendBA2HeapStr(zeng_errorstr, zeng_errorln, ba);
    rv = false;
  } else {
    if (m_client->Warning()) {
      TCI_ASSERT(0);
      //    parent->ReportWarning( m_client->GetWarning() );
    }
    TCIString plotFile;
    if (plot_status) {
      plotFile = m_client->GetPlotFile();
      m_client->ResetPlotFile();
    }
    ba = m_client->GetReturn();

    //  parent->LogReturnStr( ba );
    if (outstr_sent) {
      delete[] zeng_outstr;
      zeng_outstr = NULL;
      zeng_outln = 0;
      outstr_sent = false;
    }

    zeng_outstr = AppendBA2HeapStr(zeng_outstr, zeng_outln, ba);

    if (!plotFile.IsEmpty()) {
      ba = plotFile;
      //    parent->LogReturnStr( ba );
      zeng_outstr = AppendBA2HeapStr(zeng_outstr, zeng_outln, ba);
    }

    rv = true;
  }

  m_client->ClearReturn();
  m_client->ClearError();
  m_client->ClearWarning();
  return rv;
}

static void DispatchMuPADMessages()
{
  MSG msg;
  Sleep(0);                     // give up processor for rest of time slice if there are any other active threads
  // Avoid keyboard and mouse messages.
  if (::GetMessage(&msg, NULL, NULL, NULL)) {
    if ((WM_KEYFIRST <= msg.message && msg.message <= WM_KEYLAST) ||
        (WM_MOUSEFIRST <= msg.message && msg.message <= WM_MOUSELAST) ||
        (WM_PAINT == msg.message))
      ::PostMessage(msg.hwnd, msg.message, msg.wParam, msg.lParam);
    else
      ::DispatchMessage(&msg);
  }
}

// Wait for a normal MuPAD command to finish.
void MuPADEngine::WaitToFinish()
{
  if (m_useDLL)
    return;
  while (!m_client->Ready()) {
    if (Stop_Interrupt()) {
      if (!m_kernel->send_stop()) { // tell MuPAD to stop
        m_client->Interrupted();  // ?MuPAD dead?
        return;
      }
    }
    DispatchMuPADMessages();
  }
}

// Wait for the kernel to start up.  If there's a failure, we never get
// back a "Ready" signal, hence the different exit condition from above.
void MuPADEngine::WaitToStart()
{
  if (m_useDLL)
    return;
  while (!(m_client->Ready() || m_client->Error())) {
    if (Stop_Interrupt()) {
      if (!m_kernel->send_stop()) { // tell MuPAD to stop
        m_client->Interrupted();  // ?MuPAD dead?
        return;
      }
    }
    DispatchMuPADMessages();
  }
}

void MuPADEngine::ReadLib(const TCICHAR * name)
{
  TCI_ASSERT(false);
}

void MuPADEngine::CollectGarbage()
{
  // nothing to do
}

void MuPADEngine::DoReadLibs(const U8 * packs, const U8 * name)
{
}

void MuPADEngine::ReportMemoryUse(U32 how)
{
  // Kernel updates this every second, so don't need to ask separately
  if (how == 0) {
    //  WriteInfoString( (const U8 *)m_client->GetStatus() );
  } else {
    //  Message( MSG_INFORMATION, MSG_GENERIC, (const TCICHAR *)m_client->GetStatus() );
  }
}

char *MuPADEngine::AppendBA2HeapStr(char *zheap_str, U32 & buffer_ln,
                                    ByteArray & ba)
{
  char *rv = zheap_str;

  U32 delta_ln = ba.GetByteCount();
  if (delta_ln) {
    char *src = (char *)ba.Lock();
    if (src) {
      U32 curr_ln = 0;
      if (zheap_str)
        curr_ln = strlen(zheap_str);
      U32 bytes_needed = curr_ln + delta_ln + 1;
      if (zheap_str)
        bytes_needed++;
      if (bytes_needed > buffer_ln) {
        buffer_ln = bytes_needed + 200;
        rv = TCI_NEW(char[buffer_ln]);
        if (zheap_str) {
          strcpy(rv, zheap_str);
          delete[] zheap_str;
        } else
          rv[0] = 0;
      }
      if (zheap_str)
        rv[curr_ln++] = '\n';
      strncpy(rv + curr_ln, src, delta_ln);
      rv[curr_ln + delta_ln] = 0;
    }
    ba.Unlock();
  }

  return rv;
}
