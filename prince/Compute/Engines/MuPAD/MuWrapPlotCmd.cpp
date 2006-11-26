// Copyright (c) 2006 MacKichan Software, Inc.  All Rights Reserved.

/*
  For development purposes, these public functions are implemented
  by constructing an SWP "MuPADEngine" object and making calls to it.
*/

#include "MuWrapPlotCmd.h"
#include "MuWrap.h"
#include "../EngWrap.h"
#include "mupd_eng.h"
#include "../../MRequest.h"
#include "MuPADPlot.h"
#include <string>
#include <iostream>
using namespace std;

MuPADPlotWrapper::MuPADPlotWrapper(void *ID_dBase, void *NOM_dBase)
{
}

MuPADPlotWrapper::~MuPADPlotWrapper()
{
}

char *MuPADPlotWrapper::CreatePLOTQueryeStr(MuPADWrapper * const mpw, PlotServiceRequest * psr,
                                       const char *eng_zstr, int& error_code)
{
  error_code = 0;
  string cmd;
  string ptype = MuPADPlotWrapper::CreatePlotString (*psr, 1, "PlotType", error_code);
  MuPADPlot *plot = new MuPADPlot (mpw, psr, 1);
  string q = "@QUERY@";
  cmd += plot->ToString(q);
  
  size_t zln = cmd.length();
  char *zh_rv = new char[zln + 1];
  strcpy(zh_rv, cmd.c_str());
  return zh_rv;
}


// Given a PSR, a plot, and a parameter ID, return the parameter
// value as a string.
string MuPADPlotWrapper::CreatePlotString(PlotServiceRequest & psr,
                        U32 plot_no, const char *str_key, int & error_code){
  error_code = 0;
  U32 p_type;
  const char *x = psr.RetrievePlotParam (plot_no, str_key, p_type);
  string zh_rv (x);
  return zh_rv;
}

	
// Given a PSR, a plot, and a parameter ID, transform its semantics to a string.
// TODO: Need to catch and correctly handle cases where ProcessSemList returns error_code>0
string MuPADPlotWrapper::SemanticsToString (MuPADWrapper * const mpw, PlotServiceRequest &psr,
                          U32 plot_no, const char *str_key, int & error_code){
  string zh_rv;
  SEMANTICS_NODE *sn = psr.GetSemanticsFromParam (plot_no, str_key);
  if (sn) {
     char *ret = mpw->ProcessSemList(sn, NULL, error_code);
     if (error_code == 0)
       zh_rv = ret;
  } else {
     zh_rv = CreatePlotString(psr, plot_no, str_key, error_code);
  }
  return (zh_rv);
}


char *MuPADPlotWrapper::CreatePLOTFUNCeStr(MuPADWrapper * const mpw, PlotServiceRequest * psr,
                                       const char *eng_zstr, int& error_code) {
  error_code = 0;
  string ptype = MuPADPlotWrapper::CreatePlotString (*psr, 1, "PlotType", error_code);
  string cmd = "plot(";
  string camstr = "@camera@";

  MuPADPlot *plot = new MuPADPlot (mpw, psr, 1);  
  camstr = plot->ToString(camstr);
  delete plot;
  
  if (camstr != "") {
    cmd += camstr;
    cmd += ", ";
  }
  for (int pc = 1; psr->HasPlot(pc); pc++) {
    if (pc > 1)
      cmd += ", ";
    plot = new MuPADPlot (mpw, psr, pc);  
    string plotstr = "@PLOT@";
    cmd += plot->ToString(plotstr);
    delete plot;
  }
  cmd += ")";
  
  // << "\n\nSMR Final CMD is " << cmd << "\n" << endl;
  size_t zln = cmd.length();
  char *zh_rv = new char[zln + 1];
  strcpy(zh_rv, cmd.c_str());
  return zh_rv;
}


                                

// fairly brain dead check that just looks for clearly wrong syntax: "(," ",," and ",)"
// if wrong syntax, return  CR_badplotformat, otherwise, return 0
int MuPADPlotWrapper::CheckCmdString (char * cmd) {
  // remove all white space
  int retval = 0;
  string space = " ";
  string empty = "";
  string mistake;
  string::size_type start;
  string newcmd = MuPADPlot::ExtendedReplace (cmd, space, empty);
  
  // search for (,
  mistake = "(,";
  start = newcmd.find(mistake);
  if (start != string::npos) retval = CR_badplotformat;

  // search for ,)
  mistake = ",,";
  start = newcmd.find(mistake);
  if (start != string::npos) retval = CR_badplotformat;
  
  // search for ,,
  mistake = ",)";
  start = newcmd.find(mistake);
  if (start != string::npos) retval = CR_badplotformat;

  return retval;
}
