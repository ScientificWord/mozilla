// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

#ifdef XP_WIN
#define WIN32_LEAN_AND_MEAN     // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#endif

#include "mupd_eng.h"

#include "fltutils.h"
#include "CmpTypes.h"
#include "tcistrin.h"

#include <setjmp.h>
#include <sys/stat.h>

#include "mupkernel.h"
#include "xpcom-config.h"
#include "nsString.h"
#include "nsCOMPtr.h"

static jmp_buf mupad_mark;      // Address for long jump to jump to -- used in case of MuPAD DLL crash
static bool b_stop_mupad = false;    

// Our kernel message handler class.  A central, global data store for the MuPAD callbacks.
class MuPADMssgHandler
{
public:
  MuPADMssgHandler() : ready(false), cmd_cookie(0)
  {}

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

  bool Error()
  {
    return !errorStr.IsEmpty();
  }
  const TCIString & GetError()
  {
    return errorStr;
  }
  void ClearError()
  {
    errorStr.Empty();
  }

  bool Warning()
  {
    return !warningStr.IsEmpty();
  }
  const TCIString & GetWarning()
  {
    return warningStr;
  }
  void ClearWarning()
  {
    warningStr.Empty();
  }

  const TCIString & GetPlotFile()
  {
    return plotfileStr;
  }
  void ClearPlotFile()
  {
    plotfileStr.Empty();
  }

  MUP_Cookie Cookie()
  {
    return reinterpret_cast<MUP_Cookie>(cmd_cookie);
  }
  bool CurrentCookie(MUP_Cookie cookie)
  {
    return (reinterpret_cast<U32>(cookie) == cmd_cookie);
  }
  bool Finished(MUP_Cookie cookie)
  {
    TCI_ASSERT(CurrentCookie(cookie)); // this result is for some other command 
    if (!CurrentCookie(cookie)) {
      return false;
    } else {
      ++cmd_cookie;
      ready = true;
      return true;
    }
  }
  void Interrupted()
  {
    ++cmd_cookie;
  }
  void SetLibPath(const char * path)
  {
    libStr = path;
  }

  void SetModulePath(const char * mpath)
  {
    moduleStr = mpath;
  }

public:
  bool ready;
  TCIString returnStr;
  TCIString errorStr;
  TCIString warningStr;
  TCIString plotfileStr;
  U32 cmd_cookie;

  TCIString libStr;
  TCIString moduleStr;
};


// the kernel is ready to go
static void MUP_CALLBACK on_kernel_ready(MUP_Kernel kernel)
{
  MuPADMssgHandler* client = (MuPADMssgHandler*)MUP_get_client_data(kernel);
  client->ready = true;
}

static void MUP_CALLBACK on_init_wanted(MUP_Kernel kernel)
{
  MuPADMssgHandler* client = (MuPADMssgHandler*)MUP_get_client_data(kernel);

#ifdef XP_WIN
  
  char path[MAX_PATH];
  strcpy(path, static_cast<const TCICHAR *>(client->libStr));
  char modpath[MAX_PATH];
  strcpy(modpath, static_cast<const TCICHAR *>(client->moduleStr));

  const char *argv[4] = {
    "-l", path,
    "-m", modpath
  };
  MUP_send_init(kernel, 4, argv); 
#else

  char path[8000];
  strcpy(path, static_cast<const TCICHAR *>(client->libStr));

  const char *argv[4] = {
    "-l", path
  };
  MUP_send_init(kernel, 2, argv);
#endif

}

// an output from the kernel
static void MUP_CALLBACK on_output(MUP_Kernel kernel, MUP_Cookie cookie, const char *output_type,
                                 const char *output_format, const MUP_Char* output)
{
  MuPADMssgHandler* client = (MuPADMssgHandler*)MUP_get_client_data(kernel);
  if (!client->CurrentCookie(cookie))
    return;

  if (strcmp(MUP_ot_result, output_type) == 0)
    client->returnStr += output;
  else if (strcmp(MUP_ot_error, output_type) == 0) {
    client->errorStr += output;
  } else if (strcmp(MUP_ot_warning, output_type) == 0 ||
             strcmp(MUP_ot_other, output_type) == 0 ||
             strcmp(MUP_ot_print, output_type) == 0) {
    client->warningStr += output;
  } else {
    TCI_ASSERT(!"what is this?");
  }
}

// kernel finished command output
static void MUP_CALLBACK on_finished(MUP_Kernel kernel, MUP_Cookie cookie)
{
  MuPADMssgHandler* client = (MuPADMssgHandler*)MUP_get_client_data(kernel);
  client->Finished(cookie);
}

// kernel gave us status
static void MUP_CALLBACK on_status(MUP_Kernel kernel, MUP_Int used_mem, MUP_Int res_mem, MUP_Int sec)
{
  static MUP_Int s_used_mem;
  static MUP_Int s_res_mem;
  static MUP_Int s_sec;

  s_used_mem = used_mem;
  s_res_mem = res_mem;
  s_sec = sec;
}

// kernel generated a plot file
static void MUP_CALLBACK on_plot_result_file(MUP_Kernel kernel, MUP_Cookie cookie, const MUP_Char *fname)
{
  MuPADMssgHandler* client = (MuPADMssgHandler*)MUP_get_client_data(kernel);
  if (client->CurrentCookie(cookie))
    client->plotfileStr = fname;
}


bool Stop_Interrupt()
{
  return b_stop_mupad;
}

// MuPAD DLL is giving us a time slice during computation
static void MUP_CALLBACK Stop_MuPAD(MUP_Cookie data)
{
  //printf("\n *** Stop_MuPAD\n");
  MUP_Kernel ker = reinterpret_cast<MUP_Kernel>(data);
  if (ker && Stop_Interrupt())
    MUP_send_stop(ker);
}

// MuPAD DLL crashed and called fatal_exit_handler()
static void MUP_CALLBACK Cleanup_MuPAD()
{
  //printf("\n *** Cleanup_MuPAD\n");
  longjmp(mupad_mark, -1);
}

MuPADEngine::MuPADEngine() :
  m_kernel(0), m_client(0), fcnsloaded(false), plot_status(false), m_useDLL(false)
{
  instr_sent = false;
  outstr_sent = false;
  errorstr_sent = false;
}

MuPADEngine::~MuPADEngine()
{
  MUP_delete_kernel(m_kernel);
  m_kernel = 0;
  delete m_client;
  m_client = 0;
}

void MuPADEngine::ClearEngStrPtrs()
{
  eng_instr.Empty();
  eng_outstr.Empty();
  eng_errorstr.Empty();
}

int MuPADEngine::GetEngStrPtr(int str_ID, const char **engstr, int *engstr_ln)
{
  int rv = 0;

  if (str_ID == 1) {            // engine input string
    if (eng_instr.GetLength()) {
      *engstr = static_cast<const TCICHAR*>(eng_instr);
      *engstr_ln = eng_instr.GetLength();
      instr_sent = true;
      rv = 1;
    }
  } else if (str_ID == 2) {
    if (eng_outstr.GetLength()) {
      *engstr = static_cast<const TCICHAR*>(eng_outstr);
      *engstr_ln = eng_outstr.GetLength();
      outstr_sent = true;
      rv = 1;
    }
  } else if (str_ID == 3) {
    if (eng_errorstr.GetLength()) {
      *engstr = static_cast<const TCICHAR*>(eng_errorstr);
      *engstr_ln = eng_errorstr.GetLength();
      errorstr_sent = true;
      rv = 1;
    }
  } else {
    TCI_ASSERT(0);
  }
  return rv;
}

void MuPADEngine::StopProcessor()
{
  b_stop_mupad = true;
}

bool MuPADEngine::LoadStrsAndDLL(nsILocalFile *engpath, nsILocalFile *libpath, nsILocalFile *modulePath)
{
  //printf("\n\n  jcs mupd_eng.cpp LoadStrsAndDLL\n");
  bool rv = false;
  TCIString ErrorMessage;
  //TCIString pathLib;
  //TCIString pathEng;
  const char* pathLib;
  const char* pathEng;
  const char* pathModules;

  {
  	  nsAutoString temp1;
  	  engpath->GetPath(temp1);
  	  pathEng = NS_LossyConvertUTF16toASCII(temp1).get();
  	  char* cpy = new char[strlen(pathEng)+1];
  	  strcpy(cpy, pathEng);
  	  pathEng = cpy;
  }
  //printf("\n\n  jcs pathEng = %s\n", cstr);
  {
  	  nsAutoString temp2;
  	  libpath->GetPath(temp2);
  	  pathLib = NS_LossyConvertUTF16toASCII(temp2).get();
  	  char* cpy = new char[strlen(pathLib)+1];
  	  strcpy(cpy, pathLib);
  	  pathLib = cpy;
  }
  {
  	  nsAutoString temp3;
  	  modulePath->GetPath(temp3);
  	  pathModules = NS_LossyConvertUTF16toASCII(temp3).get();
  	  char* cpy = new char[strlen(pathModules)+1];
  	  strcpy(cpy, pathModules);
  	  pathModules = cpy;
  }
  //printf("\n\n  jcs pathLib = %s\n", pathLib);
  //fflush(NULL);
  m_client = new MuPADMssgHandler();
  if (!m_client)
    return rv;
  m_client->SetLibPath(pathLib);
  m_client->SetModulePath(pathModules);
  MUP_set_event_handler(Stop_MuPAD, 0);
  
  TCI_ASSERT(m_kernel == 0);
  
#ifdef XP_WIN
  //printf("\n\n  jcs Check 1\n");
  m_kernel = MUP_new_wmsg_kernel(pathEng, NULL);
  //printf("\n\n  jcs Check 2\n");
#else
  //printf("\n\n  jcs Check 1\n");
  m_kernel = MUP_get_uds_kernel(pathEng,NULL,1);
  //printf("\n\n  jcs Check 2\n");
#endif
  if (m_kernel) {
    //printf("\n\n  jcs Check 3\n");
    MUP_set_event_handler(Stop_MuPAD, m_kernel);
    MUP_set_fatal_exit_handler(Cleanup_MuPAD);
    // Set client data for kernel.
    MUP_set_client_data(m_kernel, m_client);

    // set kernel callbacks
    MUP_set_on_init_wanted_handler(m_kernel, &on_init_wanted);
    MUP_set_on_kernel_ready_handler(m_kernel, &on_kernel_ready);
    MUP_set_on_output_handler(m_kernel, &on_output);
    MUP_set_on_finished_handler(m_kernel, &on_finished);
    MUP_set_on_status_handler(m_kernel, &on_status);
    MUP_set_on_plot_result_file_handler(m_kernel, &on_plot_result_file);
    if (MUP_send_new_client(m_kernel, 0, 0, MUP_cf_can_plot)) {
      WaitToStart();
      
      if (m_client->Error()) {
        ErrorMessage = m_client->GetError();
        m_client->ClearError();
      } else {
        rv = true;
      }

      m_client->ClearReturn();
    }
  }
  if (!rv) {
    TCI_ASSERT(!"MSG_MuPADStartupError");
    MUP_delete_kernel(m_kernel);
    m_kernel = 0;
  }
  //printf("\n\n  jcs Check End, %d\n", rv);
  return rv;
}

int MuPADEngine::Initialize(nsILocalFile *libpath, nsILocalFile *champath)
{
  //printf("\n\n  jcs MuPADEngine::Initialize\n");
  const char* pathCham;
  nsAutoString temp;
  champath->GetPath(temp);
  pathCham = NS_LossyConvertUTF16toASCII(temp).get();

  // set output formatting
  MUP_send_pretty_print(m_kernel,0);
  MUP_send_textwidth(m_kernel,0x7fffffff); // 2^31-1
  
  return PrepareProcessor(pathCham);
}

void MuPADEngine::CleanupAndUnloadDLL()
{
  if (m_kernel) {
    MUP_send_quit(m_kernel);
    MUP_delete_kernel(m_kernel);
    m_kernel = 0;
  }
  delete m_client;
  m_client = 0;
}

void MuPADEngine::HandleMuPADException()
{
  //this kernel is corrupt, don't use it
  //TODO set an error code
  m_kernel = 0;
}

bool FileSys_Exists(const char *fspec)
{
  struct stat fs;
  return (stat(fspec, &fs) == 0);
}

int MuPADEngine::PrepareProcessor(const char *champath)
{
  // Do we need to load our special functions?
  if (!fcnsloaded) {
    TCIString packba;
    TCIString path (champath);
    path += "/swfcns.mu";
    if (FileSys_Exists(path)) {
      TCIString fcnsPath(path);
      fcnsPath.Replace("\\", "/");
      packba += "fread(\"";
      packba += fcnsPath;
      packba += "\"):";
      fcnsloaded = SendToProcessor(packba);
      return 1;
    } else {
      TCI_ASSERT(!"Unable to find swfcns.mu");
      return 0;
    }
  }
  return 1;
}

bool MuPADEngine::SendToProcessor(TCIString & str)
{
  // the result replaces the contents of ba.
  return Exec(str);
}

bool MuPADEngine::SendPlotToProcessor(TCIString & str)
{
  plot_status = true;
  bool rv = SendToProcessor(str);
  plot_status = false;
  return rv;
}

bool MuPADEngine::Exec(TCIString & cmd)
{
  bool rv = false;

  TCI_ASSERT(!m_client->Ready());

  if (instr_sent) {
    eng_instr.Empty();
    instr_sent = false;
  }
  eng_instr += cmd;

  const MUP_Char *p = static_cast<const MUP_Char*>(cmd);

  //printf("\n\n  jcs MuPADEngine::Exec(%s)\n", p);

  // send command
  int send_cmd_ret = setjmp(mupad_mark);
  if (send_cmd_ret == 0) {
    if (!MUP_send_cmd_ext(m_kernel, m_client->Cookie(), p, MUP_ef_no_history, 0)) {
      cmd.Empty();
      return rv;
    }
  } else {                      // MuPAD crashed via fatal_exit_handler()
    cmd.Empty();
    HandleMuPADException();
    return rv;
  }

  WaitToFinish();
  cmd.Empty();

  if (m_client->Error()) {
    TCI_ASSERT(!"Got an error from MuPAD.");
    cmd = m_client->GetError();
    if (errorstr_sent) {
      eng_errorstr.Empty();
      errorstr_sent = false;
    }
    eng_errorstr += cmd;
    const MUP_Char *p = static_cast<const MUP_Char*>(eng_errorstr);
    printf("\n\n  jcs MuPADEngine error(%s)\n", p);

  } else {
    if (m_client->Warning()) {
      TCI_ASSERT(!"No place for warning string.");
    }
    TCIString plotFile;
    if (plot_status) {
      plotFile = m_client->GetPlotFile();
      m_client->ClearPlotFile();
    }
    cmd = m_client->GetReturn();

    if (outstr_sent) {
      eng_outstr.Empty();
      outstr_sent = false;
    }
    eng_outstr += cmd;

    if (!plotFile.IsEmpty()) {
      cmd = plotFile;
      eng_outstr += cmd;
    }
    const MUP_Char *p = static_cast<const MUP_Char*>(eng_outstr);
    //printf("\n\n  jcs MuPADEngine result: %s\n", p);
    rv = true;
  }

  m_client->ClearReturn();
  m_client->ClearError();
  m_client->ClearWarning();
  return rv;
}

static void DispatchMuPADMessages()
{
#ifdef XP_WIN
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
#else
  MUP_handle_uds_input(true);
#endif
}

// Wait for a normal MuPAD command to finish.
void MuPADEngine::WaitToFinish()
{
  if (m_useDLL)
    return;
  while (!m_client->Ready()) {
    if (Stop_Interrupt()) {
      if (!MUP_send_stop(m_kernel)) { // tell MuPAD to stop
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
      if (!MUP_send_stop(m_kernel)) { // tell MuPAD to stop
        m_client->Interrupted();  // ?MuPAD dead?
        return;
      }
    }
    DispatchMuPADMessages();
  }
}
