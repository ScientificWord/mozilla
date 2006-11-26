// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

#include <windows.h>

#include <chmtypes.h>
#include "mapl_eng.h"

#include <math.h>
#include <stdio.h>
#include <sys/stat.h>
#include <tcistrin.h>
#include <bytearry.h>

#include "../../CmpTypes.h"
#include "../fltutils.h"
#include "../../MResult.h"

#define MPL_STANDARD        0
#define MPL_READLIB         32

#include "MathIDs.h"

enum MType
{
  MT_UNDEF, MT_FUNCTION, MT_MATRIX, MT_OTHER, MT_ODEFUNC, MT_CASEFUNC,
  MT_MAPLENAME, MT_MAPLEFUNC, MT_DEFAULT, MT_VECTOR, MT_NUMBER,
  MT_GENFUNC, MT_SUBFUNC, MT_RECURSIVE, MT_SUBNAME, MT_CONSTANT,
  MT_VECFUNC, MT_MATFUNC, MT_MATVEC, MT_NOMAPFUNC
};

extern "C"
{
#include "../win/moemapi.h"
}

#define PLOTMAGIC  1234
#define PLOTBAD    4321

I16 LastPlotID;
#define NO_LAST_PLOT -1

long MapleSetCallBack(WORD msg, void *phWnd);

void M_DECL PlotCallBack(void *data, MPlot plot);
void M_DECL TextCallBack(void *data, int tag, char *output);
void M_DECL ErrorCallBack(void *data, M_INT offset, char *msg);
void M_DECL StatusCallBack(void *data, long bytesUsed, long bytesAlloc,
                           double cpuTime);
M_BOOL M_DECL QueryInterrupt(void *data);

typedef INT(M_DECL * FMP_INT_LPCSTRP_LPSTR) (INT, LPSTR *, LPSTR);
typedef VOID(M_DECL * FMP_EVAL) (INT, LPCSTR, MCallBack, VOID *);
typedef VOID(M_DECL * FMP_QUIT) (INT);
typedef VOID(M_DECL * FMP_PLOT_SET) (MPlot, MPlotAttrib, INT);
typedef LONG(M_DECL * FMP_PLOT_GET) (MPlot, MPlotAttrib);
typedef INT(M_DECL * FMP_I_MPLOT) (MPlot);
typedef VOID(M_DECL * FMP_V_MPLOT) (MPlot);
typedef VOID(M_DECL * FMP_V_MPLOTRANGE) (MPlot, MPlotAxisRanges *);
typedef LPCSTR(M_DECL * FMP_RENDER) (MPlot, MPlotDevice, VOID *);
typedef LONG(M_DECL * FMP_FONT) (MPlot, INT, LPSTR, INT, LPSTR, INT, INT *);

// Maple DLL function pointers
static FMP_INT_LPCSTRP_LPSTR MplInitialize = 0;
static FMP_EVAL MplEvaluate = 0;
static FMP_QUIT MplQuit = 0;
static FMP_PLOT_SET MplPlotSetAttribute = 0;
static FMP_PLOT_GET MplPlotGetAttribute = 0;
static FMP_I_MPLOT MplPlotGetDimensions = 0;
static FMP_V_MPLOT MplPlotDispose = 0;
static FMP_V_MPLOTRANGE MplPlotGetAxisRanges = 0;
static FMP_RENDER MplPlotRender = 0;
static FMP_FONT MplPlotGetFont = 0;

static int MapleID;

MapleEngine::MapleEngine():MapleH(0), bCorrupt(false)
{
  zeng_instr = NULL;
  zeng_inln = 0;
  instr_sent = false;

  zeng_outstr = NULL;
  zeng_outln = 0;
  outstr_sent = false;

  zeng_errorstr = NULL;
  zeng_errorln = 0;
  errorstr_sent = false;

  fcnsloaded = false;

  mcb = new MCallBackVec();
  mcb->textCallBack = TextCallBack;
  mcb->plotCallBack = PlotCallBack;
  mcb->errorCallBack = ErrorCallBack;
  mcb->statusCallBack = StatusCallBack;
  mcb->realMathCallBack = NULL;
  mcb->readLineCallBack = NULL;
  mcb->redirectCallBack = NULL;
  mcb->callBackCallBack = NULL;
  mcb->queryInterrupt = QueryInterrupt;
//  discont = true;

  PlotCallBacks = new MPlotDeviceVec();  // inited later
}

MapleEngine::~MapleEngine()
{
  delete[] zeng_instr;
  delete[] zeng_outstr;
  delete[] zeng_errorstr;

  delete mcb;
  delete PlotCallBacks;
}

int MapleEngine::GetEngStrPtr(int str_ID, char **engstr, int *engstr_ln)
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

void MapleEngine::ReportError(const TCICHAR * msg)
{
  //  parent->ReportError( msg );
  TCI_ASSERT(0);
  errorFlag = true;
}

void MapleEngine::ReportWarning(const TCICHAR * msg)
{
  //  parent->ReportWarning( msg );
  // eventually, these need to be logged
}

bool MapleEngine::LoadStrsAndDLL(const TCIString & engpath,
                                     const TCIString & libpath)
{
  bool rv = false;

  if (false) {
    HKEY mapleKey;
    if (::
        RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     "SOFTWARE\\Waterloo Maple\\Maple V R5.1", 0, KEY_READ,
                     &mapleKey) == ERROR_SUCCESS)
      ::RegCloseKey(mapleKey);
    else {
      return rv;
    }
  }

  LoadDLL(engpath);
  if (MapleH)
    rv = true;

  return rv;
}

BOOL FileSys_Exists(const char *fspec)
{
  struct stat fs;
  return (stat(fspec, &fs) == 0);
}

bool MapleEngine::Initialize(const TCIString & libpath,
                                 const TCIString & champath)
{
  bool rv = Init(libpath);

  if (!rv)
    UnloadDLL();

  InitPlots();
  TCIString fcnfile(champath);
  fcnfile += "/tcifcns.m";
  if (!FileSys_Exists(fcnfile)) {
    rv = false;
  } else
    PrepareProcessor(NULL, champath);

  return rv;
}

void MapleEngine::CleanupAndUnloadDLL()
{
  ClearPlots();
  if (MapleH != (HANDLE) NULL)
    MplQuit(MapleID);
  UnloadDLL();
}

void MapleEngine::HandleMapleException()
{
  bCorrupt = true;
}

void MapleEngine::PrepareProcessor(void *dummy, const TCIString & champath)
{
  //Do we need to load any packages?
  U32 numpackages = 0;
  if (!fcnsloaded && !numpackages)
    numpackages = 1;

  if (numpackages) {
    ByteArray packba;
    U32 package;
    if (!fcnsloaded) {
      TCIString fcnfile(champath);
      fcnfile += "/tcifcns.m";
      //    TCI_VERIFY( StrUtil::LoadStringA(fcnfile, STR_MapleFunctionsFile) );
      if (FileSys_Exists(fcnfile)) {
        TCIString fcnsPath(fcnfile);
        fcnsPath.Replace("\\", "/");
        packba += "read `";
        packba += fcnsPath;
        packba += "`";
        fcnsloaded = SendToProcessor(&packba, (U8) ':');
        packba.SetByteCount(0);
      } else {
      }
    }
    for (U32 i = 0; i < numpackages; i++) {
      package = 0;
      LoadPackage(package, NULL);
    }
  }
}

// The contents of ba are sent to Maple. If a valid return
//  is obtained, the result replaces the contents of ba.

bool MapleEngine::SendToProcessor(ByteArray * ba, U8 punct)
{
  bool rv = false;

  plot_status = false;
  rv = SendLowLevel(ba, punct);

  return rv;
}

bool MapleEngine::SendLowLevel(ByteArray * ba, U8 punct)
{
  if (bCorrupt)
    return false;

  bool rv = false;

  // Begin by adding a semicolon, and zero-terminating 
  //  the contents of the byte array.

  if (punct)
    (*ba) += punct;

  (*ba) += (U8) '\0';

  rv = Exec(*ba);

  return rv;
}

bool MapleEngine::PackageIDtoName(U32 ID, U8 * dest)
{
  bool rv = false;
  return rv;
}

bool MapleEngine::LoadPackage(U32 package, const U8 * engine_func_name)
{
  bool rv = false;

  if (package == MPL_STANDARD) {
  } else if (package == MPL_READLIB) {
    if (engine_func_name) {
      ReadLib((const TCICHAR *)engine_func_name);
      rv = true;
    }
  } else if (pkgsloaded[package]) {
    rv = true;
  } else {                      // load this package
    U8 package_name[80];
    if (PackageIDtoName(package, package_name)) {
      TCI_ASSERT(0);
    }
  }

  return rv;
}

void MapleEngine::ReadLib(const TCICHAR * name)
{
  TCI_ASSERT(name);
  TCIString tmp = "readlib(";
  tmp += name;
  tmp += ")";
  ByteArray ca(tmp);
  SendLowLevel(&ca, (U8) ';');
}

// Note gc() changed from R3 to R4

void MapleEngine::CollectGarbage()
{
  ByteArray ba("gc()");
  SendLowLevel(&ba, (U8) ';');
}

void MapleEngine::ReportMemoryUse(U32 how)
{
  CollectGarbage();
}

// Function to "unassign" a name in the Maple engine
bool MapleEngine::Undefine(const U8 * keyname, U32 mtype, U32 procnum)
{
  bool rv = true;           // assumed

  if (mtype == MT_ODEFUNC) {    //update ftpref in Maple
    ByteArray ca;
    char buff[64];
    sprintf(buff, "tcidelodeproc(%d)", procnum);
    ca += buff;
    rv = SendLowLevel(&ca, (U8) ';');
  }

  if (rv) {
    TCI_ASSERT(0);
  }

  return rv;
}

void MapleEngine::DoReadLibs(const U8 * packs, const U8 * name)
{
  // if Maplefunction requires 'readlib', do it
  bool needread = false;
  U32 len1 = strlen((char *)packs);
  while (len1 > 0) {
    U32 size = len1;
    char *mptr = strchr((char *)packs, ',');
    if (mptr)
      size = mptr - (char *)packs;
    char buff1[8];
    strncpy(buff1, (char *)packs, size);
    buff1[size] = 0;
    U32 package = atoi(buff1);
    if (package == (U32) MPL_READLIB)
      needread = true;
    packs += (size++);
    if (size < len1)
      len1 -= size;
    else
      len1 = 0;
  }
  if (needread)
    ReadLib((TCICHAR *) name);
}


void MapleEngine::SetSeriesOrder(I32 s)
{
  TCI_ASSERT(0);
}

// Note tri-valued logic.  Doesn't work for s=2, unfortunately.
void MapleEngine::SetSolveMaxDegree(I32 s)
{

  ByteArray ba;
  ba += "_EnvExplicit:=";
  if (s == 4)
    ba += "true";
  else if (s == 1)
    ba += "false";
  else
    ba += "NULL";

  SendToProcessor(&ba, (U8) ':');
}

void MapleEngine::SetSolvePvalOnly(I32 s)
{
  ByteArray ba;
  ba += "_EnvAllSolutions:=";
  if (s == 1)
    ba += "NULL";
  else
    ba += "true";

  SendToProcessor(&ba, (U8) ':');
}

void MapleEngine::SetSolveIgnoreSCases(I32 s)
{
  // NOP  -- not available in Maple
}

void MapleEngine::SetDigits(I32 d)
{
  TCI_ASSERT(0);
}

bool MapleEngine::GetVariableAliases(GraphicData & gData, int nItem,
                                         ByteArray & vars, ByteArray * expr,
                                         int numvars,
                                         const TCIString & prefixStr)
{
  TCI_ASSERT(0);
  return false;
}

bool MapleEngine::IsScalarValued(ByteArray & name)
{
  ByteArray tempba;
  tempba += "type(";
  tempba += name;
  tempba += ", scalar)";
  bool temprv = SendLowLevel(&tempba, (U8) ';');
  if (temprv && tempba.GetByteCount()) {
    const U8 *p = tempba.ReadOnlyLock();
    U8 first = *p;
    tempba.ReadOnlyUnlock();
    if (first == 't')           // true -- We have a matrix
      return true;
  }
  return false;
}

U32 MapleEngine::QueryMType(TCIString & mname)
{
  U32 rv = 0;

  ByteArray ba;
  ba = "type(eval(";
  ba += mname;
  ba += "),matrix)";
  bool temprv = SendLowLevel(&ba, (U8) ';');

  if (temprv && ba.GetByteCount()) {
    const U8 *p = ba.ReadOnlyLock();
    U8 first = *p;
    ba.ReadOnlyUnlock();
    if (first == 't') {         //true--we have a matrix
      ByteArray ca;
      ca = "testeq(rowdim(eval(";
      ca += mname;
      ca += "))=1) or testeq(coldim(eval(";
      ca += mname;
      ca += "))=1)";
      bool temprv = SendLowLevel(&ca, (U8) ';');
      if (temprv && ca.GetByteCount()) {
        const U8 *q = ca.ReadOnlyLock();
        first = *q;
        ca.ReadOnlyUnlock();
        if (first == 't')
          rv = MT_MATVEC;
        else
          rv = MT_MATRIX;
      }
    }
  }

  return rv;
}

// Initialize the Maple kernel.  The first parameter is the fully
// qualified pathname of the directory containing the Maple library
// (MAPLE.LIB and MAPLE.IND).  The second parameter is the size (in
// bytes) of the stack available to Maple.  This function returns
// non-zero on success, or zero on failure.  If MapleInit() is called
// more than once, subsequent calls issue a "restart" command to the
// kernel, rather than reinitialize it.

#define VERSION_HEADER_BYTES 5

bool MapleEngine::Init(const TCIString & libDir)
{
  bool rv = false;

  //Check for existence of MAPLE.LIB and MAPLE.IND
  TCIString f = libDir;
  f += "/maple.ind";
  BOOL exists = FileSys_Exists(f);

  if (exists) {
    f = libDir;
    f += "/maple.lib";
    exists = FileSys_Exists(f);
  }

  if (exists) {
    //Files are present. Now check for the validity of the library.
    //filename is ....\maple.lib from above

    char lib_buf[VERSION_HEADER_BYTES];
    FILE *stream = fopen(f, "rb");

    if (stream == NULL) {
      TCI_ASSERT(false);
      rv = false;
    } else if (fread(lib_buf, sizeof(char), VERSION_HEADER_BYTES, stream) !=
               VERSION_HEADER_BYTES) {
      TCI_ASSERT(false);
      rv = false;
    } else {
      // lib_buf[0] is ignored

      if (strncmp(&lib_buf[1], "MVR", 3)) {
        //MessageBox(NULL, "Not valid R4 or R5 library", "DLL", MB_OK);
        TCI_ASSERT(false);
        rv = false;
      } else if (lib_buf[4] == '4') {
        //MessageBox(NULL, "Valid R4 library", "DLL", MB_OK);
        TCI_ASSERT(false);
        rv = false;
      } else if (lib_buf[4] == '5') {
        //MessageBox(NULL, "Valid R5 library", "DLL", MB_OK);
        rv = true;
      } else {
        //MessageBox(NULL, "Invalid library", "DLL", MB_OK); 
        TCI_ASSERT(false);
        rv = false;
      }
    }
  } else {
    rv = false;
  }

  if (rv) {                     // Library valid, start up Maple
    char init_error[256];
    int argc = 3;
    char *argv[4];
    argv[0] = "maple";
    argv[1] = "-b";

    TCIString tmp(libDir);

    argv[2] = tmp.GetBuffer(0); //libDir;
    argv[3] = 0;

    MapleID = MplInitialize(argc, argv, init_error);

    if (MapleID > 0)
      rv = true;
    else {
      rv = false;
      TCI_ASSERT(0);
    }
  }

  return rv;
}

void MapleEngine::LoadDLL(const TCIString & path)
{
  TCI_ASSERT(MapleH == NULL);
  MapleH = LoadLibrary((const char *)path);

  TCI_ASSERT(MapleH);
  if (MapleH == NULL) {
    return;
  }

  MplInitialize =
    (FMP_INT_LPCSTRP_LPSTR) GetProcAddress((HINSTANCE) MapleH,
                                           "MapleInitialize");
  MplEvaluate =
    (FMP_EVAL) GetProcAddress((HINSTANCE) MapleH, "MapleEvaluate");
  MplQuit = (FMP_QUIT) GetProcAddress((HINSTANCE) MapleH, "MapleQuit");
  MplPlotSetAttribute =
    (FMP_PLOT_SET) GetProcAddress((HINSTANCE) MapleH, "PlotSetAttribute");
  MplPlotGetAttribute =
    (FMP_PLOT_GET) GetProcAddress((HINSTANCE) MapleH, "PlotGetAttribute");
  MplPlotGetDimensions =
    (FMP_I_MPLOT) GetProcAddress((HINSTANCE) MapleH, "PlotGetDimensions");
  MplPlotDispose =
    (FMP_V_MPLOT) GetProcAddress((HINSTANCE) MapleH, "PlotDispose");
  MplPlotGetAxisRanges =
    (FMP_V_MPLOTRANGE) GetProcAddress((HINSTANCE) MapleH,
                                      "PlotGetAxisRanges");
  MplPlotRender =
    (FMP_RENDER) GetProcAddress((HINSTANCE) MapleH, "PlotRender");
  MplPlotGetFont =
    (FMP_FONT) GetProcAddress((HINSTANCE) MapleH, "PlotGetFont");

  if (!MplInitialize || !MplEvaluate || !MplQuit ||
      !MplPlotSetAttribute || !MplPlotGetAttribute ||
      !MplPlotGetDimensions || !MplPlotDispose || !MplPlotGetAxisRanges ||
      !MplPlotRender || !MplPlotGetFont) {

    FreeLibrary((HINSTANCE) MapleH);
    MapleH = NULL;
  } else {
    for (int i = 0; i < ENGINE_PACKAGES; i++)
      pkgsloaded[i] = false;

    bCorrupt = false;
  }
}

// Unload the Maple DLL from memory. This should be called before the
//   application terminates

void MapleEngine::UnloadDLL(void)
{
  for (int i = 0; i < ENGINE_PACKAGES; i++)
    pkgsloaded[i] = false;

  if (MapleH != (HANDLE) NULL)
    FreeLibrary((HINSTANCE) MapleH);

  MplInitialize = 0;
  MplEvaluate = 0;
  MplPlotSetAttribute = 0;
  MplPlotGetAttribute = 0;
  MplPlotGetDimensions = 0;
  MplPlotDispose = 0;
  MplPlotRender = 0;
  MplPlotGetFont = 0;

  MapleH = 0;
}

// CallBack functions available to Maple
static void M_DECL TextCallBack(void *data, int tag, char *output)
{

  MapleEngine *ma = (MapleEngine *) data;
  switch (tag) {
  case M_OUTPUT:
  case M_MISC:                 // this one needed for about
    ma->returnStr += output;
    break;
  case M_QUIT:
    // do nothing
    break;
  case M_WARNING:
    ma->ReportWarning(output);
    break;
  default:
    TCI_ASSERT(false);          // some kind of garbage
  }
}

static void M_DECL ErrorCallBack(void *data, M_INT offset, char *msg)
{
  MapleEngine *ma = (MapleEngine *) data;
  ma->ReportError(msg);
}

static
  void M_DECL StatusCallBack(void *data, long bytesUsed, long bytesAlloc,
                             double cpuTime)
{
}

// Check if user wants to cancel operation.
// Should be handled at a higher level.
static M_BOOL M_DECL QueryInterrupt(void *data)
{
  return 0;
}

bool MapleEngine::Exec(ByteArray & ba)
{
  bool baLocked(false);

  if (ba.GetByteCount()) {
    if (instr_sent) {
      delete[] zeng_instr;
      zeng_instr = NULL;
      zeng_inln = 0;
      instr_sent = false;
    }
    zeng_instr = AppendBA2HeapStr(zeng_instr, zeng_inln, ba);
  }

  returnStr.SetByteCount(0);
  errorFlag = false;
  try {
    const char *cmd = (const char *)ba.ReadOnlyLock();
    baLocked = true;
    MplEvaluate(MapleID, cmd, mcb, this);
    ba.ReadOnlyUnlock();
    baLocked = false;
  }
  catch(...) {
    if (baLocked)
      ba.ReadOnlyUnlock();
    HandleMapleException();
    return false;
  }

  if (!errorFlag) {
    ba = returnStr;
    U32 numBytes = ba.GetByteCount();
    // Note that numBytes includes a zero terminator byte.
    if (numBytes > 0) {
      if (outstr_sent) {
        delete[] zeng_outstr;
        zeng_outstr = NULL;
        zeng_outln = 0;
        outstr_sent = false;
      }

      zeng_outstr = AppendBA2HeapStr(zeng_outstr, zeng_outln, ba);
    }
  }

  return !errorFlag;

  return false;
}

//                               ===============================
//                               -------------------------------
//                                 Maple Plot support
//                               _______________________________
//                               ===============================

/* How many simultaneous plots we support */

#define MAPLE_NUM_PLOTS 300

/* The canonical 'invalid' plot ID */

#define OUR_PS_LAST   5

#define ROUNDOFF(a) ((a)<0 ? (M_INT)((a)-0.5) : (M_INT)((a)+0.5))
#define DISPLAY_ROUNDOFF(a) ((a) < 0 ? (M_INT)(data_vec->OneWide * (M_INT)(((a)-(data_vec->OneWide/2))/data_vec->OneWide))\
                           : (M_INT)(data_vec->OneWide * (M_INT)(((a)+(data_vec->OneWide/2))/data_vec->OneWide)))

#define LINEWIDTH_PER_LOG_POINT     5 //This is determined experimentally! What else to do?? rwa, 6-15-99

static M_INT penStyles[OUR_PS_LAST] = {
  PS_SOLID, PS_DASH, PS_DOT, PS_DASHDOT, PS_DASHDOTDOT
};

typedef struct _win_data WIN_data;
struct _win_data
{
  M_INT x;                      //x-coord of plot rect in caller's window
  M_INT y;                      //y-coord of plot rect in caller's window
  HDC hdcInput;                 //HDC for calling Get functions---needed for metafiles
  HWND hwnd;
  HDC hdcDevice;                /* HDC for the device to which we wish to render. */
  HDC hdcMem;                   /* HDC for bitmaps. */
  HDC hdc;                      /* HDC currently being rendered to (one of above 2). */
  HPEN hpen;
  HBRUSH hbrush;
  HPALETTE hPal;

  M_INT Ascent;                 // Character ascent for plot font
  float OneWide;                // Width of one pixel
  M_INT pointSize;              // basic size of point symbols
  M_INT pointRoot;              // corners on circles
  M_INT lastStartX, lastStartY;
  M_INT lastEndX, lastEndY;
  M_INT lastStartPixel;         //starting pixel in line style patterns for last render
  M_INT lastEndPixel;           //ending pixel in line style patterns for last render

  HBITMAP hOrigBitmap;          /* Handle of the original bitmap in hdcMem. */

  MPlot plot;

  M_INT canvas_height, canvas_width;
};

WIN_data *plot_devs[MAPLE_NUM_PLOTS];

void M_DECL DrawPoint(void *data, MPlotDevice dev, MPlot plot,
                      M_INT x, M_INT y, M_INT attrib);
void M_DECL DrawLine(void *data, MPlotDevice dev, MPlot plot,
                     M_INT x1, M_INT y1, M_INT x2, M_INT y2, M_INT attrib);
void M_DECL DrawPolygon(void *data, MPlotDevice dev, MPlot plot,
                        M_INT numPoints, M_INT * x, M_INT * y, M_INT attrib);
void M_DECL DrawPlotText(void *data, MPlotDevice dev, MPlot plot,
                         M_INT x, M_INT y, char *text,
                         M_INT alignment, M_INT attrib, M_INT font);
void M_DECL DrawAxisText(void *data, MPlotDevice dev, MPlot plot,
                         M_INT x, M_INT y, char *text,
                         M_INT alignment, M_INT attrib, M_INT font);
void M_DECL DrawTitleText(void *data, MPlotDevice dev, MPlot plot,
                          char *text);
void M_DECL GetTextExtents(void *data, MPlotDevice dev, MPlot plot,
                           M_INT x, M_INT y, char *text,
                           M_INT alignment, M_INT attrib, M_INT font,
                           M_INT * llx, M_INT * lly, M_INT * urx,
                           M_INT * ury);
void M_DECL StartImage(void *data, MPlotDevice dev, MPlot plot);
void M_DECL SetColorMap(void *data, MPlotDevice dev, MPlot plot,
                        M_INT numColors, M_INT * rgb);
M_BOOL M_DECL EndImage(void *data, MPlotDevice dev, MPlot plot);
M_INT M_DECL QueryDevice(void *data, MPlotDevice dev, MPlot plot,
                         M_INT devQuery);
M_BOOL M_DECL PlotQueryInterrupt(void *data, MPlotDevice dev, MPlot plot);

void MapleEngine::InitPlots()
{
  LastPlotID = NO_LAST_PLOT;
  for (I16 i = 0; i < MAPLE_NUM_PLOTS; ++i)
    plot_devs[i] = 0;

  PlotCallBacks->drawPoint = DrawPoint;
  PlotCallBacks->drawLine = DrawLine;
  PlotCallBacks->drawPolygon = DrawPolygon;
  PlotCallBacks->drawText = DrawPlotText;
  PlotCallBacks->drawAxisLine = DrawLine;
  PlotCallBacks->drawAxisText = DrawAxisText;
  PlotCallBacks->drawTitleText = DrawTitleText;
  PlotCallBacks->getTextExtents = GetTextExtents;
  PlotCallBacks->startImage = StartImage;
  PlotCallBacks->setColorMap = SetColorMap;
  PlotCallBacks->endImage = EndImage;
  PlotCallBacks->queryDevice = QueryDevice;
  PlotCallBacks->plotQueryInterrupt = PlotQueryInterrupt;
  PlotCallBacks->outChar = NULL;
}

void MapleEngine::ClearPlots()
{
  for (I16 i = 0; i < MAPLE_NUM_PLOTS; ++i)
    if (plot_devs[i])
      KillPlot(i);
  LastPlotID = NO_LAST_PLOT;
}

static void M_DECL PlotCallBack(void *data, MPlot plot)
{
  MapleEngine *ma = (MapleEngine *) data;
  TCI_ASSERT(LastPlotID == NO_LAST_PLOT);

  /* Find a free plot device to use */
  for (I16 i = 0; i < MAPLE_NUM_PLOTS; i++) {
    if (plot_devs[i] == NULL)
      break;
  }

  TCI_ASSERT(i < MAPLE_NUM_PLOTS);
  TCI_ASSERT(!plot_devs[i]);

  /* We've got a plot device slot, fill it in and set up the plot */

  LastPlotID = i;

  plot_devs[i] = new WIN_data();

  WIN_data *data_vec = plot_devs[i];

  data_vec->plot = plot;

  data_vec->hdcMem = NULL;      /* We'll create the hdcMem elsewhere. */

  data_vec->hPal = NULL;
  data_vec->hpen = NULL;
  data_vec->hbrush = NULL;

  data_vec->hdcDevice = NULL;
}

static void winconfigure(WIN_data * data_vec, DWORD dimensions)
{
  if ((data_vec->canvas_width == (M_INT) LOWORD(dimensions)) &&
      (data_vec->canvas_height == (M_INT) HIWORD(dimensions)))
    return;

  data_vec->canvas_width = (M_INT) LOWORD(dimensions);
  data_vec->canvas_height = (M_INT) HIWORD(dimensions);

  SelectObject(data_vec->hdcMem, data_vec->hOrigBitmap);
}

void MapleEngine::SetPlotRect(U32 plotID, int left, int top, int right,
                              int bottom)
{
  WIN_data *d;
  DWORD dim;
  I16 width, height;
  width = right - left;
  height = bottom - top;

  if (plotID == NO_LAST_PLOT || !plot_devs[plotID]) {
    TCI_ASSERT(false);
    return;
  }

  d = plot_devs[plotID];
  d->x = left;
  d->y = top;
//  d->x = left + width/10;  // Go to center 80%
//  d->y = top  + height/10;

//  dim = xn_over_d( height, 8, 10 ); // Use center 80%
  dim = height;
  dim = dim << 16;
//  dim  |= xn_over_d( width, 8, 10 );
  dim |= width;
  winconfigure(plot_devs[plotID], dim);

}

/* The usual case is plotting in a window, this sets the window handle we're using. */

void MapleEngine::SetPlotWindow(U32 plotID, U32 w)
{
  WIN_data *d;

  if (plotID == NO_LAST_PLOT || !plot_devs[plotID]) {
    TCI_ASSERT(false);
    return;
  }

  d = plot_devs[plotID];

  d->hwnd = (HWND) w;
}

/* Returns the ID of the last plot rendered, and resets LastPlotID to -1,
   so you should only call this ONCE after sending Maple a plot(), to see
   if the plot succeeded. */

int MapleEngine::GetLastPlotID()
{
  I16 rv;

  rv = LastPlotID;
  LastPlotID = NO_LAST_PLOT;

  return (rv);
}

void MapleEngine::KillPlot(U32 plotID)
{
  if (plotID == -1)
    return;
  TCI_ASSERT(plot_devs[plotID]);
  if (!plot_devs[plotID])
    return;

#ifdef MAPLE_LOG
  TCIString logLine(StrUtil::
                    Format("Calling MplPlotDispose %x\n",
                           plot_devs[plotID]->plot));
  LogLine(logLine);
#endif

  try {
    MplPlotDispose(plot_devs[plotID]->plot);
  }
  catch(...) {
    HandleMapleException();
  }

  delete plot_devs[plotID];
  plot_devs[plotID] = 0;
}

/* This renders the entire plot onto a given display context. The idea is to use
   this to make a copy of the plot in, say, a metafile. */

void MapleEngine::DrawOnDC(U32 plotID, U32 hdc, U32 hAttrDC)
{

  RECT r;
  WIN_data *data_vec;
  HDC savedhdc, savedhdcDevice, savedhdcInput;
  HPEN origpen;
  HBRUSH origbrush;
  HDC hdcInput;

  if (plotID == NO_LAST_PLOT || !plot_devs[plotID]) {
    TCI_ASSERT(false);
    return;
  }

  data_vec = (WIN_data *) plot_devs[plotID];

  savedhdc = data_vec->hdc;
  savedhdcDevice = data_vec->hdcDevice;
  savedhdcInput = data_vec->hdcInput;

  //Get a DC for calling Get functions
  hdcInput = (HDC) hAttrDC;

  data_vec->hdcDevice = (HDC) hdc;
  data_vec->hdc = (HDC) hdc;
  data_vec->hdcInput = hdcInput;

  r.left = data_vec->x;
  r.right = data_vec->x + data_vec->canvas_width;
  r.top = data_vec->y;
  r.bottom = data_vec->y + data_vec->canvas_height;
  //Saving it means nothing here!
  origpen = (HPEN) SelectObject((HDC) hdc, GetStockObject(BLACK_PEN));
  origpen = (HPEN) SelectObject(hdcInput, GetStockObject(BLACK_PEN));
  //DeleteObject(SelectObject(hdc, origpen));

  //Saving the brush also means nothing
  origbrush = (HBRUSH) SelectObject((HDC) hdc, GetStockObject(BLACK_BRUSH));
  origbrush = (HBRUSH) SelectObject(hdcInput, GetStockObject(BLACK_BRUSH));
  //DeleteObject(SelectObject(hdc, origbrush));

  if (data_vec->hdcMem == NULL) {

    data_vec->hdcMem = CreateCompatibleDC((HDC) hdc);

    /* Determine the handle of the bitmap currently in hdcMem.
     * There must be a better way to do this. */

    data_vec->hOrigBitmap = (HBITMAP) SelectObject(data_vec->hdcMem,
                                                   CreateCompatibleBitmap
                                                   (data_vec->hdcMem, 3, 3));
    DeleteObject(SelectObject(data_vec->hdcMem, data_vec->hOrigBitmap));
  }
#ifdef MAPLE_LOG
  LogLine("Calling MplPlotRender from DrawOnDC\n");
  TCIString logLine(StrUtil::
                    Format
                    ("  plotID=%d, MPlot=%x; hdcInput=hdcDevice=hdc=%x; hdcMem=%x; hwnd=%x;\nhOrigBitmap=%x;",
                     plotID, data_vec->plot, hdc, data_vec->hdcMem,
                     data_vec->hwnd, data_vec->hOrigBitmap));
  LogLine(logLine);
#endif
  const char *res;
  bool handleException = false;
  try {
    res = MplPlotRender(plot_devs[plotID]->plot, PlotCallBacks, data_vec);
    if (res) {
//    parent->ReportError( res );
      TCI_ASSERT(0);
    }
  }
  catch(...) {
    handleException = true;
  }

  /* Restore original pen and brush, delete any we have left over */

  if (data_vec->hpen) {
    SelectObject(hdcInput, origpen);
    DeleteObject(data_vec->hpen);
    data_vec->hpen = NULL;
  }

  if (data_vec->hbrush) {
    SelectObject(hdcInput, origbrush);
    DeleteObject(data_vec->hbrush);
    data_vec->hbrush = NULL;
  }

  data_vec->hdc = savedhdc;
  data_vec->hdcDevice = savedhdcDevice;
  data_vec->hdcInput = savedhdcInput;
  //ReleaseDC(hwnd, hdcInput);

  if (handleException)
    HandleMapleException();

  return;
}

static int widthofonepixel(HDC hdc)
{
  POINT thickpoint[2];
  int one;
  thickpoint[0].x = 0;
  thickpoint[0].y = 0;
  thickpoint[1].x = 1;
  thickpoint[1].y = 1;
  DPtoLP(hdc, thickpoint, 2);
  one = (thickpoint[0].y - thickpoint[1].y);
  if (one < 0)
    one = -one;
  if (one < 1)
    one = 1;
  return one;
}

static void winsethpen(WIN_data * data_vec, M_INT attrib, BOOL ignore_style)
{
  HPEN hpenold, hpenlast;
  HPEN hpen;
  BYTE r, g, b;
  M_INT colour, style, width;

  hpenold = data_vec->hpen;

  colour = DECODE_COLOR(attrib);

  r = (BYTE) (colour >> 16);
  g = ((WORD) colour) >> 8;
  b = (BYTE) colour;

  style = PS_SOLID;
  if (!ignore_style) {
    style = DECODE_LINESTYLE(attrib);

    if (style)
      style = (style - 1) % sizeof(penStyles);
    style = penStyles[style];

    if (style != PS_SOLID)
      SetBkColor(data_vec->hdc, RGB(255, 255, 255));
  }

  width = DECODE_THICKNESS(attrib);

  width = ROUNDOFF(width * data_vec->OneWide);

  // "If the value specified by nWidth is greater than 1,
  // the fnPenStyle parameter must be PS_NULL, PS_SOLID, or PS_INSIDEFRAME."

  hpen = data_vec->hpen = CreatePen(style, width, RGB(r, g, b));
  hpenlast = (HPEN) SelectObject(data_vec->hdcInput, hpen);

  //If we're dealing with a MetaFile...
  if (data_vec->hdc != data_vec->hdcInput)
    SelectObject(data_vec->hdc, hpen);

  if (hpenold && hpenlast == hpenold)
    DeleteObject(hpenold);
}

#define CODE_COLOR(x)        (((M_INT)(x)) & 0xffffff)
#define CODE_LINESTYLE(x)   ((((M_INT)(x)) & 0x07)<<25)
#define CODE_SYMBOL(x)      ((((M_INT)(x)) & 0x0f)<<28)
#define CODE_THICKNESS(x)   ((((M_INT)(x)) & 0x0f)<<28)

static
  void M_DECL DrawPoint(void *data, MPlotDevice dev, MPlot plot,
                        M_INT x, M_INT y, M_INT attrib)
{
  WIN_data *data_vec = (WIN_data *) data;
  M_INT ix, iy;
  M_INT shape;
  POINT vertices[9];

  ix = x + data_vec->x;
  iy = data_vec->y + data_vec->canvas_height - y;

  M_INT pointThick = MPA_THICKNESS_THIN;
  M_INT pointSize = data_vec->pointSize;
  M_INT pointRoot = data_vec->pointRoot;

  M_INT ptAttrib = CODE_COLOR(DECODE_COLOR(attrib))
    | CODE_THICKNESS(pointThick)
    | CODE_LINESTYLE(MPA_LINESTYLE_SOLID);
  winsethpen(data_vec, ptAttrib, false);

  shape = DECODE_SYMBOL(attrib);
  switch (shape) {
  case MPA_SYMBOL_CROSS:
  case MPA_SYMBOL_DEFAULT:
  default:
    MoveToEx(data_vec->hdc, ix - pointSize, iy, NULL);
    LineTo(data_vec->hdc, ix + pointSize, iy);
    LineTo(data_vec->hdc, ix - pointSize, iy);  // Move pen back to get end
    MoveToEx(data_vec->hdc, ix, iy - pointSize, NULL);
    LineTo(data_vec->hdc, ix, iy + pointSize);
    LineTo(data_vec->hdc, ix, iy - pointSize);  // Move pen back to get end
    break;
  case MPA_SYMBOL_DIAMOND:
    MoveToEx(data_vec->hdc, ix, iy - pointSize, NULL);
    LineTo(data_vec->hdc, ix + pointSize, iy);
    LineTo(data_vec->hdc, ix, iy + pointSize);
    LineTo(data_vec->hdc, ix - pointSize, iy);
    LineTo(data_vec->hdc, ix, iy - pointSize);
    break;
  case MPA_SYMBOL_CIRCLE:
    vertices[0].x = ix - pointSize, vertices[0].y = iy;
    vertices[1].x = ix - pointRoot, vertices[1].y = iy - pointRoot;
    vertices[2].x = ix, vertices[2].y = iy - pointSize;
    vertices[3].x = ix + pointRoot, vertices[3].y = iy - pointRoot;
    vertices[4].x = ix + pointSize, vertices[4].y = iy;
    vertices[5].x = ix + pointRoot, vertices[5].y = iy + pointRoot;
    vertices[6].x = ix, vertices[6].y = iy + pointSize;
    vertices[7].x = ix - pointRoot, vertices[7].y = iy + pointRoot;
    vertices[8].x = ix - pointSize, vertices[8].y = iy;
    Polyline(data_vec->hdc, vertices, 9);
    break;
  case MPA_SYMBOL_BOX:
    vertices[0].x = ix - pointSize, vertices[0].y = iy - pointSize;
    vertices[1].x = ix + pointSize, vertices[1].y = iy - pointSize;
    vertices[2].x = ix + pointSize, vertices[2].y = iy + pointSize;
    vertices[3].x = ix - pointSize, vertices[3].y = iy + pointSize;
    vertices[4].x = ix - pointSize, vertices[4].y = iy - pointSize;
    Polyline(data_vec->hdc, vertices, 5);
    break;
  case MPA_SYMBOL_POINT:
    pointThick = ROUNDOFF(pointThick * data_vec->OneWide / 2);
    vertices[0].x = ix - pointThick, vertices[0].y = iy - pointThick;
    vertices[1].x = ix + pointThick, vertices[1].y = iy - pointThick;
    vertices[2].x = ix + pointThick, vertices[2].y = iy + pointThick;
    vertices[3].x = ix - pointThick, vertices[3].y = iy + pointThick;
    vertices[4].x = ix - pointThick, vertices[4].y = iy - pointThick;
    Polyline(data_vec->hdc, vertices, 5);
    break;
  }
}

#ifdef DEBUG
//  #define PLOT_LOGGING
#endif

#define MAX_LINESTYLE_SUBSEGMENTS    4

struct lineStyleDrawParams
{
  M_INT numpixels;
  M_INT numsegments;
  M_INT endpoints[2 * MAX_LINESTYLE_SUBSEGMENTS];
};

static lineStyleDrawParams lineDrawingParams[OUR_PS_LAST] = { {0, 0, {0, 1}}, //PS_SOLID - drawn directly as LineTo
{16, 1, {0, 12}},               //PS_DASH
{16, 2, {0, 2, 8, 10}},         //PS_DOT
{16, 2, {0, 6, 10, 12}},        //PS_DASHDOT
{16, 3, {0, 5, 8, 9, 12, 13}}   //PS_DASHDOTDOT
};

// Same function used for both DrawLine and DrawAxisLine

#ifdef PLOT_LOGGING
static TCICHAR *styleNames[OUR_PS_LAST] =
  { "Solid", "Dashed", "Dotted", "DashDot", "DashDotDot" };
#endif

static
  void M_DECL DrawLine(void *data, MPlotDevice dev, MPlot plot,
                       M_INT x1, M_INT y1, M_INT x2, M_INT y2, M_INT attrib)
{
  M_INT style;
  WIN_data *data_vec = (WIN_data *) data;

  //Translate to the right place */
  x1 += data_vec->x;
  x2 += data_vec->x;
  y1 = data_vec->y + data_vec->canvas_height - y1;
  y2 = data_vec->y + data_vec->canvas_height - y2;

  winsethpen(data_vec, attrib, true);

  style = DECODE_LINESTYLE(attrib);

  if (style)
    style = (style - 1) % sizeof(penStyles);
//   style = penStyles[style];
#ifdef PLOT_LOGGING
  TCICHAR msgbuff[256];
  sprintf(msgbuff, "\nDrawing with style %s from (%d,%d) to (%d,%d)\n",
          styleNames[style], x1, y1, x2, y2);
  MessageToFile(MSG_DIAGINFO, msgbuff);
#endif
  switch (penStyles[style]) {
  case PS_DASH:
  case PS_DOT:
  case PS_DASHDOT:
  case PS_DASHDOTDOT:
    {
      lineStyleDrawParams *drawparams = &lineDrawingParams[style];
      //scale our segs
      M_INT drawlen = ROUNDOFF(_hypot(x2 - x1, y2 - y1));
      M_INT dotSize = data_vec->pointSize / 2;
      M_INT nPixels = (drawlen + dotSize - 1) / dotSize;
      M_INT startx, starty;
      M_INT startpixel = data_vec->lastEndPixel;
      M_INT endpixel = 0;
      BOOL isreversed = false;
      M_INT whichcase = -1;
      if (abs(x2 - data_vec->lastStartX) < dotSize &&
          abs(y2 - data_vec->lastStartY) < dotSize)
        whichcase = 3;
      else if (abs(x1 - data_vec->lastEndX < dotSize) &&
               abs(y1 - data_vec->lastEndY) < dotSize)
        whichcase = 0;
      else if (abs(x1 - data_vec->lastStartX < dotSize) &&
               abs(y1 - data_vec->lastStartY) < dotSize)
        whichcase = 2;
      else if (abs(x2 - data_vec->lastEndX < dotSize) &&
               abs(y2 - data_vec->lastEndY) < dotSize)
        whichcase = 1;

      if (whichcase == -1)
        startpixel = 0;
      else {
        if (whichcase & 1)      //our end rather than our start matches previously drawn stuff
        {
          isreversed = true;
          endpixel = data_vec->lastEndPixel;
        }
        if (whichcase & 2)      //we match the previous start rather than the previous end
        {
          if (isreversed)
            endpixel = data_vec->lastStartPixel;
          else
            startpixel = data_vec->lastStartPixel;
        }
      }
      if (isreversed) {
        startpixel =
          (endpixel + drawparams->numpixels -
           (nPixels % drawparams->numpixels)) % drawparams->numpixels;
      } else {
        endpixel = (startpixel + nPixels) % drawparams->numpixels;
      }
#ifdef PLOT_LOGGING
      sprintf(msgbuff,
              "Last start (%d,%d), last end (%d,%d); last start pixel %d, last end pixel %d",
              data_vec->lastStartX, data_vec->lastStartY, data_vec->lastEndX,
              data_vec->lastEndY, data_vec->lastStartPixel,
              data_vec->lastEndPixel);

      MessageToFile(MSG_DIAGINFO, msgbuff);
      sprintf(msgbuff, "Whichcase was %d", whichcase);
      MessageToFile(MSG_DIAGINFO, msgbuff);
#endif
      data_vec->lastStartPixel = startpixel;
      data_vec->lastEndPixel = endpixel;

      float dx = (float)(x2 - x1) / nPixels;
      float dy = (float)(y2 - y1) / nPixels;

      data_vec->lastStartX = x1;
      data_vec->lastStartY = y1;
      data_vec->lastEndX = x2;
      data_vec->lastEndY = y2;

      endpixel = drawparams->numpixels;
      M_INT srcpixel = 0;
      M_INT targpixel = 0;
//       nPixels += startpixel;
      x1 -= ROUNDOFF(startpixel * dx);
      y1 -= ROUNDOFF(startpixel * dy);
      M_INT nRepetitions =
        (nPixels + drawparams->numpixels - 1) / drawparams->numpixels;
      if (startpixel != 0)
        nRepetitions++;
#ifdef PLOT_LOGGING
      sprintf(msgbuff, "Size of each full round is (%d,%d)",
              ROUNDOFF(drawparams->numpixels * dx),
              ROUNDOFF(drawparams->numpixels * dy));
      MessageToFile(MSG_DIAGINFO, msgbuff);
      sprintf(msgbuff,
              "Number of reps is %d, nPixels is %d, startpixel is %d",
              nRepetitions, nPixels, startpixel);
      MessageToFile(MSG_DIAGINFO, msgbuff);
#endif
      for (I32 ii = 0; ii < nRepetitions; ii++) {
        if (nPixels < drawparams->numpixels)
          endpixel = __min(startpixel + nPixels, drawparams->numpixels);

        startx = ROUNDOFF(x1 + ii * drawparams->numpixels * dx);
        starty = ROUNDOFF(y1 + ii * drawparams->numpixels * dy);

        for (U16 jj = 0; jj < 2 * drawparams->numsegments; jj += 2) {
          srcpixel = __max(drawparams->endpoints[jj], startpixel);
          targpixel = __min(drawparams->endpoints[jj + 1], endpixel);
#ifdef PLOT_LOGGING
          sprintf(msgbuff, "    Unrounded: from (%f,%f) to (%f,%f)",
                  startx + (srcpixel * dx), starty + (srcpixel * dy),
                  startx + (targpixel * dx), starty + (targpixel * dy));
          MessageToFile(MSG_DIAGINFO, msgbuff);
          sprintf(msgbuff,
                  "  Drawing segment from (%d,%d) (pixel %d) to (%d,%d) (pixel %d)",
                  DISPLAY_ROUNDOFF(startx + (srcpixel * dx)),
                  DISPLAY_ROUNDOFF(starty + (srcpixel * dy)), srcpixel,
                  DISPLAY_ROUNDOFF(startx + (targpixel * dx)),
                  DISPLAY_ROUNDOFF(starty + (targpixel * dy)), targpixel);
          MessageToFile(MSG_DIAGINFO, msgbuff);
#endif
          if (srcpixel < targpixel) {
            MoveToEx(data_vec->hdc,
                     DISPLAY_ROUNDOFF(startx + (srcpixel * dx)),
                     DISPLAY_ROUNDOFF(starty + (srcpixel * dy)), NULL);
            LineTo(data_vec->hdc, DISPLAY_ROUNDOFF(startx + (targpixel * dx)),
                   DISPLAY_ROUNDOFF(starty + (targpixel * dy)));
          }
#ifdef PLOT_LOGGING
          else {
            MessageToFile(MSG_DIAGINFO, "Not drawing this segment...");
          }
#endif
        }
        if (startpixel < endpixel)
          nPixels -= (endpixel - startpixel);
        if (nPixels <= 0)
          break;
        startpixel = 0;         //after the first piece, start the pattern at its beginning
      }
    }
    break;

  case PS_SOLID:
  default:
    MoveToEx(data_vec->hdc, x1, y1, NULL);
    LineTo(data_vec->hdc, x2, y2);
    break;
  }
}

static void winsethbrush(WIN_data * d, M_INT value)
{
  HBRUSH hbrush;
  WORD bDel = false;

  if (d->hbrush != NULL)
    bDel = true;

  BYTE r, g, b;

  r = (BYTE) (value >> 16);
  g = ((WORD) value) >> 8;
  b = (BYTE) value;
  d->hbrush = CreateSolidBrush(RGB(r, g, b));

  hbrush = (HBRUSH) SelectObject(d->hdcInput, d->hbrush);
  //Are we dealing with a metafile?
  if (d->hdc != d->hdcInput)
    SelectObject(d->hdc, d->hbrush);
  if (bDel)
    DeleteObject(hbrush);
}

static
  void M_DECL DrawPolygon(void *data, MPlotDevice dev, MPlot plot,
                          M_INT numPoints, M_INT * x, M_INT * y, M_INT attrib)
{
  HPEN oldpen;
  WIN_data *data_vec = (WIN_data *) data;
  M_INT i;
  LPPOINT points;

  oldpen = (HPEN) NULL;

  points = new POINT[numPoints];

  for (i = 0; i < numPoints; ++i) {
    points[i].x = x[i] + data_vec->x;
    points[i].y = data_vec->y + data_vec->canvas_height - y[i];
  }

  winsethbrush(data_vec, DECODE_COLOR(attrib));

  if (numPoints) {
    oldpen = (HPEN) SelectObject(data_vec->hdc, GetStockObject(NULL_PEN));
    Polygon(data_vec->hdc, points, (short)numPoints);
  }
  if (oldpen)
    (void)SelectObject(data_vec->hdc, oldpen);

  delete[] points;
}

// Should we draw this text?

static bool TCISuckText(TCICHAR * str)
{
  bool rv = true;

  if (strstr(str, "QSUB") && strstr(str, "ESUB"))
    rv = false;
  if (str[0] == 'Q' && str[1] == 'Q')
    rv = false;
  if (str[0] == 'v' && isdigit(str[1]))
    rv = false;
  if (strstr(str, "QVF")) {
    TCICHAR *ptr = strstr(str, "QVF");
    TCI_ASSERT(ptr + 3 == str + strlen(str));
    *ptr = 0;                   // slam in NULL termination
    rv = true;
  }
  if (strstr(str, "QLF")) {
    TCICHAR *ptr = strstr(str, "QLF");
    TCI_ASSERT(ptr + 3 == str + strlen(str));
    *ptr = 0;                   // slam in NULL termination
    rv = true;
  }

  return rv;
}

#define HORZ(r,n) (r).left += (n); (r).right += (n)
#define VERT(r,n) (r).top += (n); (r).bottom += (n)

static
  void M_DECL DrawTitleText(void *data, MPlotDevice dev, MPlot plot,
                            char *text)
{
}

static
  void M_DECL DrawAxisText(void *data, MPlotDevice dev, MPlot plot,
                           M_INT x, M_INT y, char *text,
                           M_INT alignment, M_INT attrib, M_INT font)
{
  char fname[100], sname[100];
  M_INT fsize;
  try {
    if (MplPlotGetFont(plot, font, fname, 100, sname, 100, &fsize)) {
      if (fsize == 0)
        return;
    }
  }
  catch(...) {
/* JBM
    Message( MSG_WARNING, MSG_GENERIC, "Maple kernel failure");
*/
    return;
  }
  DrawPlotText(data, dev, plot, x, y, text, alignment, attrib, font);
}

static
  void TCITextGetRect(WIN_data * data_vec,
                      M_INT x, M_INT y, char *text, M_INT alignment,
                      RECT & rect)
{
  int len;
  SIZE sz;
  BOOL res;
  rect.left = x + data_vec->x;
  rect.bottom = data_vec->y + data_vec->canvas_height - y;

  len = strlen(text);
  res = GetTextExtentPoint32(data_vec->hdcInput, text, len, &sz);
  sz.cy = data_vec->Ascent;     // better height!
  rect.right = rect.left + sz.cx;
  rect.top = rect.bottom - sz.cy;

  /* Do the alignment properly */
  if (alignment & ALIGN_TO_LEFT) {
    HORZ(rect, -sz.cx);
  } else if (alignment & ALIGN_TO_RIGHT) {
    /* do nothing */
  } else {                      /* Center it */
    HORZ(rect, -sz.cx / 2);
  }

  if (alignment & ALIGN_BELOW) {
    VERT(rect, +sz.cy);
  } else if (alignment & ALIGN_ABOVE) {

  } else {                      /* Center it */
    VERT(rect, +sz.cy / 2);
  }
}

static
  void M_DECL DrawPlotText(void *data, MPlotDevice dev, MPlot plot,
                           M_INT x, M_INT y, char *text,
                           M_INT alignment, M_INT attrib, M_INT font)
{
  int len;
  RECT rect;
  WIN_data *data_vec = (WIN_data *) data;

  TCITextGetRect(data_vec, x, y, text, alignment, rect);

  if (!TCISuckText(text))
    return;
  len = strlen(text);

// TextColor is set by those who control us.
//------------------------------------------//
//   M_INT colour = DECODE_COLOR(attrib);
//
//   r = (BYTE)(colour >> 16);
//   g = ((WORD) colour) >> 8;
//   b = (BYTE)colour;
//   dwColor = SetTextColor(data_vec->hdc, RGB(r,g,b));

  TextOut(data_vec->hdc, rect.left, rect.bottom, text, len);
}

static
  void M_DECL GetTextExtents(void *data, MPlotDevice dev, MPlot plot,
                             M_INT x, M_INT y, char *text,
                             M_INT alignment, M_INT attrib, M_INT font,
                             M_INT * llx, M_INT * lly, M_INT * urx,
                             M_INT * ury)
{
  RECT rect;
  WIN_data *data_vec = (WIN_data *) data;

  TCITextGetRect(data_vec, x, y, text, alignment, rect);

  *llx = rect.left;
  *lly = rect.bottom;

  *urx = rect.right;
  *ury = rect.top;
}

// Set here those things which are constant throughout the rendering session
static void M_DECL StartImage(void *data, MPlotDevice dev, MPlot plot)
{
}

static
  void M_DECL SetColorMap(void *data, MPlotDevice dev, MPlot plot,
                          M_INT numColors, M_INT * rgb)
{
}

static M_BOOL M_DECL EndImage(void *data, MPlotDevice dev, MPlot plot)
{
  return true;
}

static
  M_INT M_DECL QueryDevice(void *data, MPlotDevice dev, MPlot plot,
                           M_INT devQuery)
{
  WIN_data *data_vec = (WIN_data *) data;

  switch (devQuery) {
  case Q_WIDTH:
    return data_vec->canvas_width;
  case Q_HEIGHT:
    return data_vec->canvas_height;
  case Q_COLORMAP:
    return 0;                   // 24 bit mode
  case Q_BACKGROUND:
    return 0xFFFFFF;
  default:
    return 0;
  }
}

static
  M_BOOL M_DECL PlotQueryInterrupt(void *data, MPlotDevice dev, MPlot plot)
{

  return QueryInterrupt(data);
}

char *MapleEngine::AppendBA2HeapStr(char *zheap_str, U32 & buffer_ln,
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
        rv = new char[buffer_ln];
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
