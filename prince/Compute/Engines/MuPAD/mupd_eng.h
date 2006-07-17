// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

#ifndef MUPD_ENG_H
#define MUPD_ENG_H

#include <ByteArry.h>
#include <TCIStrin.h>

#define ENGINE_PACKAGES  35

class MTkernel;
class MuPADMssgHandler;
//class CompEngine;
//class MuPlotStore;

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
/*
  bool  Undefine( const U8* keyname,U32 mtype,U32 procnum );
  bool  HackUseDefaultVariableNames( GraphicData &gData, int nItem, const ByteArray* expr);
  bool  GetVariableAliases( GraphicData& gData, int nItem, 
                        ByteArray& vars, ByteArray* expr, int numVars, const TCIString& prefixStr );
  bool  IsScalarValued( ByteArray& name );
  U32       QueryMType( TCIString& mname );

  void      PlotGraphic(ImageId imid, const PlotData&, GraphicData& gd,
                          Renderer* r, ByteArray* plotfcns, U16 docid);
  void      PlotGraphicInImage(ImageId imid, const PlotData&, GraphicData& gd,
                          Renderer* r, ByteArray* plotfcns, U16 docid);
  void      PlotMetafile(const PlotData&, GraphicData& gd, ImageId imid, Renderer* r);
  I32       GetPlotPoint( GraphicData* gd, I32 x, I32 y, I32 w, I32 h, TCIString& str, bool polar);

  void      ReDrawPlot(U32 id, ImageId imid, Renderer* r);
  void      SetPlotRect(U32 id, int left, int top, int right, int bottom );                                                
  void      SetPlotWindow(U32 id, U32 window );
  void      SetSeriesOrder(I32);
  void      SetSolveMaxDegree(I32);
  void      SetSolvePvalOnly(I32);
  void      SetSolveIgnoreSCases(I32);
  void      SetDigits(I32);
  int       GetLastPlotID();
  void      KillPlot(U32 plotID);
  void      DrawOnDC( U32 id, U32 win, U32 hdc );
  int       GetViewRanges( GraphicData& gd, U8* s, U32 len );
  SWClientItem*
            GetPlotOLEClientItem(U32 plotID) const;
  bool  SortOutPlotVars(GraphicData& gData, int nItem, ByteArray& vars,
                  int numVars, const TCIString& prefixStr, ByteArray* pPlotExpression);
*/

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
/*
  bool GeneratePlotRequest(ByteArray& baPlot, const PlotData& data,
                              GraphicData& gd, ByteArray* plotfcns, U16 docID);
*/

  char *AppendBA2HeapStr(char *zheap_str, U32 & buffer_ln, ByteArray & ba);

private:
    bool m_useDLL;
  MTkernel *m_kernel;
  MuPADMssgHandler *m_client;
//  MuPlotStore*      m_pPlotStore;

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

//  bool    fcnsloaded;
//  bool    pkgsloaded[ENGINE_PACKAGES];
};

#endif
