// Copyright (c) 2006 MacKichan Software, Inc.  All Rights Reserved.

#ifndef MuPADPlot_H
#define MuPADPlot_H

#include "../../CmpTypes.h"
#include "../fltutils.h"
#include "../EngWrap.h"
#include "MuWrap.h"
#include <vector>
#include <cctype>
#include <string>
using namespace std;

class MathServiceRequest;
class PlotServiceRequest;

// relate a token with a function that generates a string to replace a token
class MuPADPlot {
public:
  MuPADPlot ( MuPADWrapper * const thismpw, PlotServiceRequest * thispsr, int thisplot_no );
  ~MuPADPlot ();
  string ToString (string cmdstr);
  static string ExtendedReplace (string target, string oldstr, string newstr);
  static string SingleReplace   (string target, string oldstr, string newstr);


private:
  typedef struct {
    char *token;
    string (*fp)(MuPADPlot* T);
  } REPTOKFN;

  MuPADWrapper * const mpw;
  PlotServiceRequest * psr;
  int plotno;
  static  REPTOKFN tokenfns[];
    
  string dim, ptype;
  bool animate;
  int numvars;
  int error_code;

  string ReplaceToken (string cmd, string token);
  string TransformToken (string token);
  string GetNextReplaceableToken (string str, string delim);
  string GetNextReplaceableToken (string str, string delim, string::size_type &start);

  static string AIColor(MuPADPlot* T);
  static string AIFnParamsStr(MuPADPlot* T);
  static string AIMethodStr (MuPADPlot* T);
  static string AIParamsStr (MuPADPlot* T);
  static string AIShowInfoStr (MuPADPlot* T);
  static string AnimateStartStr (MuPADPlot* T);
  static string AnimateEndStr (MuPADPlot* T);
  static string AnimateFPSStr (MuPADPlot* T);
  static string AnimateVisBeforeStr (MuPADPlot* T);
  static string AnimateVisAfterStr (MuPADPlot* T);
  static string AxesTipsStr (MuPADPlot* T);
  static string AxesTypeStr (MuPADPlot* T);
  static string AxesScalingStr (MuPADPlot* T);
  static string BaseColorStr (MuPADPlot* T);
  static string BGColorStr (MuPADPlot* T);
  static string CameraStr (MuPADPlot* T);
  static string DiscAdjustStr (MuPADPlot* T);                          
  static string EqualScalingStr (MuPADPlot* T);
  static string ExpStr (MuPADPlot* T);
  static string FillPatternStr (MuPADPlot* T);
  static string GlobalParamsStr (MuPADPlot* T);
  static string GridLinesStr (MuPADPlot *T);
  static string IncludeLinesStr (MuPADPlot *T);
  static string IncludePointsStr (MuPADPlot *T);
  static string LocalOptsStr (MuPADPlot* T);
  static string LineColorStr (MuPADPlot* T);
  static string LineStyleStr (MuPADPlot* T);
  static string LineThicknessStr (MuPADPlot* T);
  static string MeshStr (MuPADPlot* T);
  static string OrientationStr (MuPADPlot* T);
  static string PlotFuncStr (MuPADPlot* T);
  static string PlotStr (MuPADPlot* T);
  static string PointStyleStr (MuPADPlot* T);
  static string QueryStr (MuPADPlot* T);
  static string ReqParamsStr (MuPADPlot* T);
  static string SecondaryColorStr (MuPADPlot* T);
  static string StringToColorArrayStr (string val);
  static string SurfaceStyleStr (MuPADPlot* T);
  static string SurfaceMeshStr (MuPADPlot* T);
  static string TitleStr (MuPADPlot* T);
  static string UNIMPLEMENTED(MuPADPlot* T);
  static string VarsStr (MuPADPlot* T);
};

#endif

