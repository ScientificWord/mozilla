// Copyright (c) 2006 MacKichan Software, Inc.  All Rights Reserved.

//  Plot command strings for MuPad

#ifndef MuPADPlotWrapper_H
#define MuPADPlotWrapper_H

#include "../../CmpTypes.h"
#include "../fltutils.h"
#include "../EngWrap.h"
#include "MuWrap.h"
#include <string>
using std::string;

class MathServiceRequest;
class PlotServiceRequest;
//class MathResult;
//class MuPAD2Tree;

class MuPADPlotWrapper {
public:
  MuPADPlotWrapper( void* ID_dBase,void* NOM_dBase );
  ~MuPADPlotWrapper();

  static string    CreatePlotString(PlotServiceRequest & psr, U32 plot_no, const char *str_key, int & error_code);
  static char*     CreatePLOTQueryeStr(MuPADWrapper * const mpw, PlotServiceRequest * psr, const char *eng_zstr, int& error_code);
  static char*     CreatePLOTFUNCeStr(MuPADWrapper * const mpw, PlotServiceRequest * psr, const char *eng_zstr, int& error_code);
  static string    SemanticsToString (MuPADWrapper * const mpw, PlotServiceRequest &psr, U32 plot_no, const char *str_key, int & error_code);
  static int       CheckCmdString (char * cmd);

private:
  bool  RenderPlot( const char* VCamFile, const char* outFile);
};

#endif

