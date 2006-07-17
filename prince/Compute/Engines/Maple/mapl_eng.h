// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

#ifndef MAPL_ENG_H
#define MAPL_ENG_H

#if TCIENV(MAPLE)

#include <ByteArry.h>
#include <TCIStrin.h>

extern "C"
{
#define _MAPLE_WINDOWS_
#include "../win/moemapi.h"
}

#define ENGINE_PACKAGES  35

class GraphicData;

class MapleEngine
{
public:
  MapleEngine();
  ~MapleEngine();

  void ReportError(const TCICHAR *);
  void ReportWarning(const TCICHAR *);

  bool LoadStrsAndDLL(const TCIString & engpath,
                          const TCIString & libpath);
  bool Initialize(const TCIString & libpath, const TCIString & champath);
  int GetEngStrPtr(int str_ID, char **engstr, int *engstr_ln);

  void CleanupAndUnloadDLL();

  void PrepareProcessor(void *dummy, const TCIString & champath);
  bool SendToProcessor(ByteArray * ba, U8 punct);
  bool SendLowLevel(ByteArray * ba, U8 punct);

  bool PackageIDtoName(U32 ID, U8 * dest);
  //bool  LoadPackage( U32 );
  bool LoadPackage(U32 package_ID, const U8 * engine_func_name);
  void ReadLib(const TCICHAR * name);
  void CollectGarbage();
  void ReportMemoryUse(U32 how);

  bool Undefine(const U8 * keyname, U32 mtype, U32 procnum);
  bool GetVariableAliases(GraphicData & gData, int nItem,
                              ByteArray & vars, ByteArray * expr, int numVars,
                              const TCIString & prefixStr);
  void DoReadLibs(const U8 * packs, const U8 * name);
  bool IsScalarValued(ByteArray & name);
  U32 QueryMType(TCIString & mname);

/*
  void      PlotGraphic(ImageId imid, const PlotData&, GraphicData& gd,
                          Renderer* r, ByteArray* plotfcns, U16 docid);
  void      PlotGraphicInImage(ImageId imid, const PlotData&, GraphicData& gd,
                          Renderer* r, ByteArray* plotfcns, U16 docid);
  void      PlotMetafile(const PlotData&, GraphicData& gd, ImageId imid, 
                                Renderer* r);
  I32       GetPlotPoint( GraphicData* gd, I32 x, I32 y, I32 w, I32 h, TCIString& str, bool polar);

  void      ReDrawPlot(U32 id, ImageId imid, Renderer* r);
*/

  void SetPlotRect(U32 id, int left, int top, int right, int bottom);
  void SetPlotWindow(U32 id, U32 window);
  void SetSeriesOrder(I32);
  void SetSolveMaxDegree(I32);
  void SetSolvePvalOnly(I32);
  void SetSolveIgnoreSCases(I32);
  void SetDigits(I32);
//  void      SetDiscont( bool b ) { discont = b; }
  int GetLastPlotID();
  void KillPlot(U32 plotID);
  void DrawOnDC(U32 id, U32 win, U32 hdc);
//  int       GetViewRanges( GraphicData& gd, U8* s, U32 len );

private:
    bool Init(const TCIString & libDir);
  bool Exec(ByteArray & ba);
  void LoadDLL(const TCIString & path);
  void UnloadDLL(void);
  void InitPlots(void);
  void ClearPlots(void);

  void HandleMapleException(void);

  char *AppendBA2HeapStr(char *zheap_str, U32 & buffer_ln, ByteArray & ba);
  void *MapleH;
  MCallBackVec *mcb;
  MPlotDeviceVec *PlotCallBacks;

  char *zeng_instr;
  U32 zeng_inln;
  bool instr_sent;

  char *zeng_outstr;
  U32 zeng_outln;
  bool outstr_sent;

  char *zeng_errorstr;
  U32 zeng_errorln;
  bool errorstr_sent;

//  bool  discont; //False for functions Re & Im

public:
// public so callback functions can access

    bool bCorrupt;
  bool errorFlag;
  ByteArray returnStr;
  TCIString statusStr;

  bool restoringdefs;
  bool plot_status;

  bool fcnsloaded;
  bool pkgsloaded[ENGINE_PACKAGES];
};

#endif
#endif
