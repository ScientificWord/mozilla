// Copyright (c) 2006 MacKichan Software, Inc.  All Rights Reserved.
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
     
// PLOT        ::= plot(<PLOTCMD1> [,<PLOTCMD>]*)
// PLOTCMD1    ::= plot::<PLOTFUNC>(<REQPARMS><LOCALPARMS><GLOBALPARM>) 
// PLOTCMD     ::= plot::<PLOTFUNC>(<REQPARMS>[,<LOCALPARMS)])
// PLOTFUNC    ::= Polar | Implicit2d | Inequality | Curve2d | Conformal | VectorField2d | 
//                 VectorField2d | Ode2d | Cylindrical | Spherical | Function3d | Implicit3d | 
//                 Inequality | Curve3d | Conformal | VectorField3d | VectorField3d | Ode3d | 
//                 Tube | ApproximateIntegral | Curve3d
// REQPARMS    ::= <EXPRESSION>,<VARS>,<MESH> 
// MESH        ::= Mesh={<int> | [<int>,<int>] | [<int>,<int>,<int>]}
// VARS        ::= <VAR>[,<VAR>[,<VAR>]]
// VAR         ::= <VARNAME>=<real>..<real>
// LOCALPARMS  ::= [,<LOCALPARM>]*
// LOCALPARM   ::= LineStyle={Dashed | Solid | Dotted}
//                 LineWidth=<INT0..9>
//                 PointStyle={Squares | Circles | Crosses | Diamonds | FilledSquares | FilledCircles |
//                             FilledDiamonds | Stars | XCrosses}
//                 Filled=<BOOL>
//                 MeshVisible=<BOOL>
//                 XLinesVisible=<BOOL>
//                 YLinesVisible=<BOOL>
//                 DiscontinuitySearch=<BOOL>
//                 CameraPoint={Automatic | [<REAL>,<REAL>,<REAL>]}
// GLOBALPARMS ::= [,<GLOBALPARM>]*
// GLOBALPARM  ::= Width=<MEASURE> 
//                 Height=<MEASURE> 
//                 Title=<STRING> 
//                 TitlePlacement={Above | Below}    
//                 Axes={None | Origin | Frame | Boxed | Automatic}   
//                 Scaling={Constrained | Unconstrained}              
//                 CoordinateType={LinLin | LinLog | LogLin | LogLog} 
//                 AxesTitles=[<STRING>,<STRING>,<STRING>]            
//                 TicksVisible=<BOOL>                                
//                 TicksNumber={None | Low | Normal | High}           
//                 AxesTips=<BOOL>                                    
//                 GridVisible=<BOOL>
// MEASURE     ::= nn*unit::<UNIT>
// UNIT        ::=  mm | cm | in
// BOOL        ::= {TRUE | FALSE}

// In the following, the command is built by transformations from the initial PLOT string. There are 
// four types of non-terminals: @TOKEN@, #TOKEN#, %TOKEN%, and $TOKEN$. The @TOKEN@ non-terminals are 
// replaced by some function. The matching function is found in the tokenfns[] array (see below).
// The #TOKEN# and %TOKEN% are replaced by data derived from the <graph> element. Tokens with '#'
// signs are assumed to have semantics (i.e., come from mathml), and tokens with '%' are values 
// without sementics. $TOKEN$ are for user input strings. We replace these more carefully, since
// users may try to use one of the token delimiters in their input strings.

#include "MuPADPlot.h"
#include "MuWrap.h"
#include "../EngWrap.h"
#include "mupd_eng.h"
#include "../../MRequest.h"
#include "MuWrapPlotCmd.h"

#include <string>
#include <cmath>
#include <iostream>
#include <sstream>
using namespace std;

string msiToString(double val) {
    stringstream strm;
    strm << val;
    return strm.str();
}

string msiToString(int val) {
    stringstream strm;
    strm << val;
    return strm.str();
}

int PlotVarsNeeded (string dim, string ptype, string animate) {
  int nvars = 0;  
  if ((ptype == "curve") || (ptype == "tube")) {
    nvars = 0;            // gets set to 1 below
  } else if (ptype == "explicitList") {
    nvars = 0;  
  } else if ((ptype == "rectangular") || (ptype == "polar") ||
           (ptype == "parametric")  || (ptype == "approximateIntegral") ||
           (ptype == "spherical")   || (ptype == "cylindrical")) {
    nvars = 1;                                                
  } else if ((ptype == "inequality") || (ptype == "implicit") ||
           (ptype == "conformal")  || (ptype == "vectorField") ||
           (ptype == "gradient")   || (ptype == "ode")) {
    nvars = 2;
  } else {
    TCI_ASSERT(!"ERROR: invalid plot type line 93 MuPADPlot.cpp");

  }  
  if (dim == "3") 
    nvars++;
  if (animate == "true")
    nvars++;
  return (nvars);
}  


MuPADPlot::MuPADPlot ( MuPADWrapper * const thismpw, PlotServiceRequest * thispsr, int thisplot_no ): mpw(thismpw) {
  this->psr = thispsr;
  this->plotno = thisplot_no;
  
  dim   = MuPADPlotWrapper::CreatePlotString (*psr, plotno, "Dimension", error_code);
  ptype = MuPADPlotWrapper::CreatePlotString (*psr, plotno, "PlotType", error_code);
  U32 plottype;                                                                                    
  string val = psr->RetrievePlotParam (plotno, PID_PlotAnimate, plottype);                                     
  this->animate = (val == "true") ? true : false;

  int nvars = PlotVarsNeeded (dim, ptype, val);
  this->numvars = nvars;
}

MuPADPlot::~MuPADPlot () {
}

// convert a plot to an engine command string
// Tokens are delimited by one of four symbols. The "@" symbol is for
// top-level tranformations. For each of these tokens, there is a function that
// determines the appropriate replacement string, which may contain other 
// replaceable tokens. The "#" and "%" delimited tokens are replaced by 
// semantic information. "$" delimited tokens are replaced by user character 
// strings. We have to treat these separately. User character strings may contain
// weird things, such as any of the delimiter symbols. Therefore, we save these
// replacements until the end, and we replace them right-to-left in the 
// final engine string.
//
string MuPADPlot::ToString (string newcmd) {
  vector<string> delims;
  string d1 = "@";   delims.push_back (d1);
         d1 = "#";   delims.push_back (d1);
         d1 = "%";   delims.push_back (d1);
//         d1 = "$";   delims.push_back (d1);
                                                 
  for (vector<string>::size_type ix=0; ix!=delims.size(); ix++) {
    for (string token=GetNextReplaceableToken (newcmd, delims[ix]); 
         token!=""; 
         token=GetNextReplaceableToken (newcmd, delims[ix])) {
      newcmd = ReplaceToken (newcmd, delims[ix]+token+delims[ix]);
    }   
  }  
  // special handling for $ delimited text: walk string (right-to-left)
  // don't allow substitutions in text after replace
  string::size_type i=newcmd.length();
  d1 = "$";
  for (string token=GetNextReplaceableToken (newcmd, d1, i); 
       token!=""; 
       token=GetNextReplaceableToken (newcmd, d1, i)) {
    newcmd = ReplaceToken (newcmd, d1+token+d1);
    i = i - 1;
  }   
  return newcmd;
}


// replace a token with a string derived from a function
string MuPADPlot::ReplaceToken (string cmd, string token) {
  string replstr = "";  
  if (token[0] == '@') {
    replstr = TransformToken (token);
    replstr = ExtendedReplace (cmd, token, replstr);
  } else if (token[0] == '#') {
    string semtok = token.substr (1, token.length()-2);
    replstr = MuPADPlotWrapper::SemanticsToString (mpw, *psr, plotno, semtok.c_str(), error_code);
    replstr = ExtendedReplace (cmd, token, replstr);
  } else if (token[0] == '%') {
    string semtok = token.substr (1, token.length()-2);
    replstr = MuPADPlotWrapper::SemanticsToString (mpw, *psr, plotno, semtok.c_str(), error_code);
    replstr = ExtendedReplace (cmd, token, replstr);
  } else if (token[0] == '$') {
    string semtok = token.substr (1, token.length()-2);
    replstr = MuPADPlotWrapper::SemanticsToString (mpw, *psr, plotno, semtok.c_str(), error_code);
    replstr = SingleReplace (cmd, token, replstr);
  }  
  return replstr;
}



string MuPADPlot::TransformToken (string token) {
  int j;
  string s = "";
  for (j=0; ((tokenfns[j].token) && (token!=tokenfns[j].token)); j++);
  if (tokenfns[j].token) {
    s = (tokenfns[j].fp)(this);
  } else {
    cout << "WARNING: MuPADPlot token " << token << " not supported, ignored\n";
  }
  return   s;
}


// scan input string from right to left looking for tokens. Optional start specifies 
// the offset at which to start the search. Otherwise, start at the rightmost character.
string MuPADPlot::GetNextReplaceableToken (string str, string delim) {
  string::size_type start = str.length();
  return (GetNextReplaceableToken (str, delim, start));
}

string MuPADPlot::GetNextReplaceableToken (string str, string delim, string::size_type &start) {
  string returnstr = "";
  string::size_type begin, tend;
  tend = str.find_last_of (delim, start);
  if (tend != string::npos) {
    begin = str.find_last_of (delim, tend-1);
	if (begin != string::npos) {
	  returnstr = str.substr (begin+1, tend-begin-1);
	}
  }
  start = begin;
  return returnstr;
}

// replace all occurences of %oldstr% in target with newstr
string MuPADPlot::ExtendedReplace (string target, string oldstr, string newstr) {
  string retstr = target;
  string::size_type start;
  for (start=retstr.find(oldstr); start!=string::npos; start=retstr.find(oldstr)) {
    retstr = retstr.replace (start, oldstr.length(), newstr);
  }
  return retstr;
}


// replace the leftmost occurence of oldstr in target with newstr
string MuPADPlot::SingleReplace (string target, string oldstr, string newstr) {
  string retstr = target;
  string::size_type start;
  start=retstr.find(oldstr); 
  if (start != string::npos) {
    retstr = retstr.replace (start, oldstr.length(), newstr);
  }
  return retstr;
}

//============= Tranform Functions ============================
MuPADPlot::REPTOKFN MuPADPlot::tokenfns[] = { 
     "@aicolor@",        MuPADPlot::AIColor,
     "@aifnparams@",     MuPADPlot::AIFnParamsStr,
     "@aimethod@",       MuPADPlot::AIMethodStr,
     "@aiparams@",       MuPADPlot::AIParamsStr,
     "@aishowinfo@",     MuPADPlot::AIShowInfoStr,
     "@animatestart@",   MuPADPlot::AnimateStartStr,
     "@animateend@",     MuPADPlot::AnimateEndStr,
     "@animatefps@",     MuPADPlot::AnimateFPSStr,
     "@animatevisbefore@",MuPADPlot::AnimateVisBeforeStr,
     "@animatevisafter@",MuPADPlot::AnimateVisAfterStr,
     "@axesscaling@",    MuPADPlot::AxesScalingStr,
     "@axestips@",       MuPADPlot::AxesTipsStr,
     "@axestype@",       MuPADPlot::AxesTypeStr,
     "@basecolor@",      MuPADPlot::BaseColorStr,
     "@bgcolor@",        MuPADPlot::BGColorStr,
     "@camera@",         MuPADPlot::CameraStr,
     "@equalscaling@",   MuPADPlot::EqualScalingStr,
     "@discadjust@",     MuPADPlot::DiscAdjustStr,
     "@exp@",            MuPADPlot::ExpStr,
     "@fillpattern@",    MuPADPlot::FillPatternStr,
     "@globalparams@",   MuPADPlot::GlobalParamsStr,
     "@gridlines@",      MuPADPlot::GridLinesStr,
     "@includelines@",   MuPADPlot::IncludeLinesStr,
     "@includepoints@",  MuPADPlot::IncludePointsStr,
     "@linecolor@",      MuPADPlot::LineColorStr,
     "@linestyle@",      MuPADPlot::LineStyleStr,
     "@linethickness@",  MuPADPlot::LineThicknessStr,
     "@localopts@",      MuPADPlot::LocalOptsStr,
     "@mesh@",           MuPADPlot::MeshStr,
     "@orientation@",    MuPADPlot::OrientationStr,
     "@plotfunc@",       MuPADPlot::PlotFuncStr,
     "@PLOT@",           MuPADPlot::PlotStr,                        
     "@pointstyle@",     MuPADPlot::PointStyleStr,                        
     "@QUERY@",          MuPADPlot::QueryStr,
     "@reqparams@",      MuPADPlot::ReqParamsStr,
     "@basecolor@",      MuPADPlot::BaseColorStr,
     "@secondarycolor@", MuPADPlot::SecondaryColorStr,
     "@surfacestyle@",   MuPADPlot::SurfaceStyleStr,
     "@surfacemesh@",    MuPADPlot::SurfaceMeshStr,
     "@title@",          MuPADPlot::TitleStr,
     "@vars@",           MuPADPlot::VarsStr,
     (char *)NULL,       NULL                
};                                   
  


string MuPADPlot::AIColor (MuPADPlot* T) {
  string pattern = "";
  cout << "WARNING: MuPADPlot::AIColor not implemented\n";
  return pattern;
}


//plot(plot::Integral(plot::Function2d(x^5,x=0..5),5,IntMethod=Trapezoid,
//  Color=[.5,.2,.3],ShowInfo=[IntMethod,Integral,Error]),
//  plot::Function2d(x^5,x=0..5))
string MuPADPlot::AIFnParamsStr (MuPADPlot* T) {
  string pattern = "plot::Function2d(@exp@,@vars@)";
  return pattern;
}


string MuPADPlot::AIMethodStr (MuPADPlot* T) {
  string pattern = "Simpson";
  U32 plottype;
  string val = T->psr->RetrievePlotParam (T->plotno, PID_PlotAIMethod, plottype);                                     
  if (val == "Exact") {
    pattern = "Exact";
  } else if (val == "Left") {
    pattern = "RiemannLeft";
  } else if (val == "Right") {
    pattern = "RiemannRight";
  } else if (val == "Middle") {
    pattern = "RiemannMiddle";
  } else if (val == "Lower") {
    pattern = "RiemannLower";
  } else if (val == "Upper") {
    pattern = "RiemannUpper";
  } else if (val == "LowerAbsolute") {
    pattern = "RiemannLowerAbs";
  } else if (val == "UpperAbsolute") {
    pattern = "RiemannUpperAbs";
  } else if (val == "Trapezoidal") {
    pattern = "Trapezoid";
  } 
  return pattern;
}


string MuPADPlot::AIParamsStr (MuPADPlot* T) {
  string pattern = ",%AISubIntervals%,IntMethod=@aimethod@@aicolor@@aishowinfo@@fillpattern@";
  return pattern;
}


string MuPADPlot::AIShowInfoStr (MuPADPlot* T) {
  string pattern = ", ShowInfo = [IntMethod, Integral = \"Exact value\"]";
  U32 plottype;
  string val = T->psr->RetrievePlotParam (T->plotno, PID_PlotAIInfo, plottype);                                     
  if (val == "NoInfo") {
    pattern = ", ShowInfo = []";
  } else if (val == "ApproximatedValue") {
    pattern = ", ShowInfo = [IntMethod]";
//  } else if (val == "ApproximatedandExactValue") {
//    pattern = ", ShowInfo = [IntMethod, Integral = \"Exact value\"]";
  } else if (val == "BothValuesandError") {
    pattern = ", ShowInfo = [IntMethod, Integral = \"Exact value\", Error = \"Error      \"]";
  } 
  return pattern;
}


string MuPADPlot::AnimateEndStr (MuPADPlot* T) {
  U32 plottype;                                                                                    
  string s = "";
  string val = T->psr->RetrievePlotParam (T->plotno, PID_PlotAnimateEnd, plottype);                                     
  if (T->animate) 
    s=", TimeEnd="+val;
  return s;
}


string MuPADPlot::AnimateFPSStr (MuPADPlot* T) {
  string s = "";
  U32 plottype;                                                                                    
  if (T->animate) {
    string fps   = T->psr->RetrievePlotParam (T->plotno, PID_PlotAnimateFPS, plottype);                                     
    string last  = T->psr->RetrievePlotParam (T->plotno, PID_PlotAnimateEnd, plottype);                                     
    string first = T->psr->RetrievePlotParam (T->plotno, PID_PlotAnimateStart, plottype);                                     
    int start, end, frames;
    sscanf (fps.c_str(),"%d",&frames);
    sscanf (last.c_str(),"%d",&end);
    sscanf (first.c_str(),"%d",&start);
    frames = (end-start)*frames;
    if ((frames > 0) && (frames < 1000))
      s = ", Frames=" + msiToString(frames);
  }    
  return s;
}


string MuPADPlot::AnimateStartStr (MuPADPlot* T) {
  U32 plottype;                                                                                    
  string s = "";
  if (T->animate) {
    string val = T->psr->RetrievePlotParam (T->plotno, PID_PlotAnimateStart, plottype);                                     
    s=", TimeBegin="+val;
  }  
  return s;
}


string MuPADPlot::AnimateVisBeforeStr (MuPADPlot* T) {
  string s="";
  if (T->animate) {
    U32 plottype;                                                                                    
    string val = T->psr->RetrievePlotParam (T->plotno, PID_PlotAnimateVisBefore, plottype);  
    if (val == "false")
      s = ", VisibleBeforeBegin=FALSE";
    else
      s = ", VisibleBeforeBegin=TRUE";                                   
  }    
  return s;
}


string MuPADPlot::AnimateVisAfterStr (MuPADPlot* T) {
  string s="";
  if (T->animate) {
    U32 plottype;                                                                                    
    string val = T->psr->RetrievePlotParam (T->plotno, PID_PlotAnimateVisAfter, plottype);                                     
    string s="";
    if (val == "false")
      s = ", VisibleAfterEnd=FALSE";
    else
      s = ", VisibleAfterEnd=TRUE";                                   
  }    
  return s;
}


string MuPADPlot::AxesScalingStr (MuPADPlot* T) {
  U32 plottype;                                                                                    
  string val = T->psr->RetrievePlotParam (T->plotno, PID_GraphAxesScaling, plottype);                                     
  string s=", CoordinateType=";
  string t="LinLin";
  if ((val == "LinLog") || (val == "LogLog") || (val == "LogLin")) t = val;
  return s+t;
}


string MuPADPlot::AxesTipsStr (MuPADPlot* T) {
  U32 plottype;                                                                                    
  string val = T->psr->RetrievePlotParam (T->plotno, PID_GraphAxesTips, plottype);                                     
  string s=", AxesTips=";
  s += ((val=="true") ? "TRUE" : "FALSE");
  return s;
}


string MuPADPlot::AxesTypeStr (MuPADPlot* T) {
  U32 plottype;                                                                                    
  string val = T->psr->RetrievePlotParam (T->plotno, PID_GraphAxesType, plottype);                                     
  string s=", Axes=";
  string t="Automatic";
  if ((val=="None") || (val == "Origin") || (val == "Frame") || (val == "Boxed"))
     t = val;
  return s+t;
}


string MuPADPlot::BaseColorStr (MuPADPlot* T) {
  U32 plottype;                                                                                    
  string val = T->psr->RetrievePlotParam (T->plotno, PID_PlotBaseColor, plottype); 
  val = StringToColorArrayStr (val);                                    
  string ret = ", FillColor=";
  return ret+val;                                              
}


string MuPADPlot::BGColorStr (MuPADPlot* T) {
  U32 plottype;                                                                                    
  string val = T->psr->RetrievePlotParam (T->plotno, PID_GraphBGColor, plottype); 
  val = StringToColorArrayStr (val);                                    
  string ret = ", BackgroundColor=";
  return ret+val;                                              
}


// if some of the camera parameters are here, generate 
//plot(plot::Camera([100,100,200],[0,0,1],.2),plot::Function3d(x*y,x=0..5,y=0..5))
// [position],[focalpoint],angle

string MuPADPlot::CameraStr (MuPADPlot* T) {
  U32 plottype;                                                                                    
  string val = T->psr->RetrievePlotParam (T->plotno, PID_GraphCameraLocationX, plottype);
  string ret = ""; 
  if (val != "")
    ret = "plot::Camera([#CameraLocationX#,#CameraLocationY#,#CameraLocationZ#],[#FocalPointX#,#FocalPointY#,#FocalPointZ#],#ViewingAngle#)";
  return ret;                                              
}


string MuPADPlot::DiscAdjustStr (MuPADPlot* T) {                           
  string ret = "";
  if ((T->dim[0] == '2') && (T->ptype == "rectangular")) {
    U32 plottype;                                                                                    
    string val = T->psr->RetrievePlotParam (T->plotno, PID_PlotDiscAdjust, plottype);                                     
    if (val == "off") {
       ret = ", DiscontinuitySearch=FALSE, VerticalAsymptotesVisible=FALSE";
    } else if (val == "on") {
       ret = ", DiscontinuitySearch=TRUE, VerticalAsymptotesVisible=FALSE";
    } else {
       ret = ", DiscontinuitySearch=TRUE, VerticalAsymptotesVisible=TRUE";
    }
  }    
  return ret;                                              
}                                                          


string MuPADPlot::EqualScalingStr (MuPADPlot* T) {
  U32 plottype;                                                                                    
  string val = T->psr->RetrievePlotParam (T->plotno, PID_GraphEqualScaling, plottype);                                     
  string s=", Scaling=";
  s += ((val=="true") ? "Constrained" : "Unconstrained");
  return s;
}


string MuPADPlot::ExpStr (MuPADPlot* T) {
  string e = "#Expression#";
  // for gradient plots, take the partial derivative wrt each axis
  if (T->ptype == "tube") {
    e = "#Expression#,#TubeRadius#";
  } else if (T->ptype == "gradient") {
    if (T->dim[0] == '2') {
      e = "[eval(tcidiff(#Expression#,#XVar#)), eval(tcidiff(#Expression#,#YVar#))]";
    } else if (T->dim[0] == '3') {  
      e = "[eval(tcidiff(#Expression#,#XVar#)), eval(tcidiff(#Expression#,#YVar#)), eval(tcidiff(#Expression#,#ZVar#))]";
    }  
  }
  return e;
}


string MuPADPlot::FillPatternStr (MuPADPlot* T) {
  string pattern = ", FillPattern=";
  U32 plottype;
  string val = T->psr->RetrievePlotParam (T->plotno, PID_PlotFillPattern, plottype);
  if (val == "Solid") {
    pattern += "Solid";
  } else if (val == "CrossedLines") {
    pattern += "CrossedLines";
  } else if (val == "DiagonalLines") {
    pattern += "DiagonalLines";
  } else if (val == "ForwardDiagonalLines") {
    pattern += "FDiagonalLines";
  } else if (val == "HorizontalLines") {
    pattern += "HorizontalLines";
  } else if (val == "VerticalLines") {
    pattern += "VerticalLines";
  } else if (val == "CrossedDiagonalLines") {
    pattern += "XCrossedLines";
  } else {
    pattern = "";
  }
  return pattern;
}


// same as expandLocalOpts, but different set of parameters about which we care
string MuPADPlot::GlobalParamsStr (MuPADPlot* T) {
  U32 plottype;                                                                                    
  string retstr   = "";                                                                           
  int titlecount  = 0;
  int cameracount = 0;
  for (int i=PID_GraphFirstParam; i<PID_PlotFirstParam; i++) {                                             
	string val = T->psr->RetrievePlotParam (T->plotno, i, plottype);                                     
    if (val != "") {                                                                            
	  switch (i) {                                                                              
        case PID_GraphXAxisLabel:
        case PID_GraphYAxisLabel:
        case PID_GraphZAxisLabel:
          if (titlecount == 0) {
            retstr += "@title@";
            titlecount++;
          }  
          break;

//         case PID_GraphCameraLocationX:
//         case PID_GraphCameraLocationY:
//         case PID_GraphCameraLocationZ:
//         case PID_GraphFocalPointX    :
//         case PID_GraphFocalPointY    :
//         case PID_GraphFocalPointZ    :
//         case PID_GraphViewingAngle   :
//           if (cameracount == 0) {
//             retstr += "@camera@";
//             cameracount++;
//           }
//           break;  
          
        case PID_GraphAxesType	   :  
          retstr += "@axestype@";                                                          
		  break;                                                                                
        case PID_GraphAxesScaling  :                                                            
          retstr += "@axesscaling@";
		  break;                                                                                
        case PID_GraphEqualScaling  :  
          retstr += "@equalscaling@";
          break;                                                                                
        case PID_GraphEnableTicks   :                                                           
		  break;                                                                                
        case PID_GraphXTickCount	   :                                                        
		  break;                                                                                
        case PID_GraphYTickCount	   :                                                        
		  break;                                                                                
        case PID_GraphZTickCount	   :                                                        
		  break;                                                                                
        case PID_GraphAxesTips	   : 
          retstr += "@axestips@";
		  break;                                                                                
        case PID_GraphGridLines	   :                                                            
          retstr += "@gridlines@";
		  break;                                                                                
        case PID_GraphBGColor	   :
          retstr += "@bgcolor@";                                                            
		  break;                                                                                
      } // end switch                                                                           
  	} // end if                                                                                 
                                                                                                
  } // end for                                                                                  
  return retstr;                                                                                
}                                                                                               


string MuPADPlot::GridLinesStr (MuPADPlot *T) {
  U32 plottype;                                                                                    
  string val = T->psr->RetrievePlotParam (T->plotno, PID_GraphGridLines, plottype);
            string s=", GridVisible=";
  s += ((val=="true") ? "TRUE" : "FALSE");
  return s;
}


string MuPADPlot::IncludeLinesStr (MuPADPlot* T) {
  U32 plottype;                                                                                    
  string val = T->psr->RetrievePlotParam (T->plotno, PID_PlotIncludeLines, plottype);                                     
  string ret = "";
  if ((T->dim[0]=='2') && (T->ptype != "vectorField") && (T->ptype != "gradient") 
      && (T->ptype != "inequality") && (T->ptype != "conformal")) {
    ret = ", LinesVisible=";
    ret += ((val=="true")?"TRUE":"FALSE");              
  }
  return ret;                                              
}                                                          


string MuPADPlot::IncludePointsStr (MuPADPlot* T) {
  U32 plottype;                                 
  string ret = "";                                                   
  string val = T->psr->RetrievePlotParam (T->plotno, PID_PlotIncludePoints, plottype);  
  if ((T->ptype != "implicit") && (T->ptype != "vectorField") && (T->ptype != "gradient") 
      && (T->ptype != "inequality") && (T->ptype != "conformal")) {
    ret = ", PointsVisible=";
    ret += ((val=="true")?"TRUE":"FALSE");              
  }  
  return ret;                                              
}                                                          


int hexval (char c) {
  if ((c >= '0') && (c <= '9')) 
    return (c-'0');
  if ((c>='A') && (c<='Z'))
    return (10 + c - 'A');
  if ((c>='a') && (c<='z'))
    return (10 + c - 'a');
  return 0; 
}


// A color is a #rrggbb, where rr, gg, bb are $00 to $ff
string MuPADPlot::LineColorStr (MuPADPlot* T) {
  U32 plottype;                                                                                    
  string val = T->psr->RetrievePlotParam (T->plotno, PID_PlotLineColor, plottype); 
  val = StringToColorArrayStr (val);                                    
  string ret = ", LineColor=";
  return ret+val;                                              
}                                                          


string MuPADPlot::LineStyleStr (MuPADPlot* T) {
  string ret = "";
  if ((T->ptype != "vectorField") && (T->ptype != "gradient") && (T->ptype != "tube")) {
    U32 plottype;                                                                                    
    string val = T->psr->RetrievePlotParam (T->plotno, PID_PlotLineStyle, plottype);                                     
    ret = ", LineStyle=Solid";                        
    if ((val == "Dotted") || (val == "Dashed"))              
      ret = ", LineStyle=" + val;                            
  }    
  return ret;                                              
}                                                          
                                                           

string MuPADPlot::LineThicknessStr (MuPADPlot* T) {
  U32 plottype;                                                                                    
  string val = T->psr->RetrievePlotParam (T->plotno, PID_PlotLineThickness, plottype);                                     
  string ret = "";
  if (val == "thin") {
    ret == ", LineWidth=0";
  } else if (val == "medium") {
    ret == ", LineWidth=1";
  } else if (val == "thick") {
    ret == ", LineWidth=2";
  } 
  return ret;                                              
}                                                          


// for local plot attributes, attach to any plot                                                
string MuPADPlot::LocalOptsStr (MuPADPlot* T) {
  string retstr = "";                                                                           
  U32 plottype;                                                                                    
                                                                                                
  // walk the list of possible parameters extracting the value of the parameter
  // if the parameter has a value and is in this list, 
  //   add the parameter string to the return string
  for (int i=PID_PlotFirstParam; i<PID_last; i++) {                                             
	string val = T->psr->RetrievePlotParam (T->plotno, i, plottype);                                     
    if (val != "") {                    
	  switch (i) {                                                                              
        case PID_PlotLineStyle :                                                                
          retstr += "@linestyle@";
          break;                                                                                
        case PID_PlotLineThickness :                                                            
          retstr += "@linethickness@";
		  break;                                                                                
        case PID_PlotLineColor :                                                                
          retstr += "@linecolor@";
		  break;                                                                                
        case PID_PlotDiscAdjust :                                                               
            retstr += "@discadjust@";
		  break;                                                                                
        case PID_PlotDirectionalShading :                                                       
		  break;                                                                                
        case PID_PlotBaseColor		    :                                                       
          retstr += "@basecolor@";
		  break;                                                                                
        case PID_PlotSecondaryColor	    :                                                       
          retstr += "@secondarycolor@";
		  break;                                                                                
        case PID_PlotPointSymbol		:  
          retstr += "@pointstyle@";
		  break;                                                                                
        case PID_PlotIncludePoints		:  
          retstr += "@includepoints@";
		  break;                                                                                
        case PID_PlotIncludeLines		:  
          retstr += "@includelines@";
		  break;                                                                                
        case PID_PlotSurfaceStyle	    : 
          retstr += "@surfacestyle@";
		  break;                                                                                
        case PID_PlotSurfaceMesh		:                                                       
          retstr += "@surfacemesh@";
		  break;                                                                                
        case PID_PlotCameraLocationX	:                                                       
		  break;                                                                                
        case PID_PlotCameraLocationY	:                                                       
		  break;                                                                                
        case PID_PlotCameraLocationZ	:                                                       
		  break;                                                                                
        case PID_PlotFontFamily		    :                                                       
          break;         
        case PID_PlotAnimateStart       :
          retstr += "@animatestart@";
		  break;                                                             
        case PID_PlotAnimateEnd   	    :
          retstr += "@animateend@";
		  break;                                                             
        case PID_PlotAnimateFPS  	    :
          retstr += "@animatefps@";
		  break;                                                             
        case PID_PlotAnimateVisBefore   :
          retstr += "@animatevisbefore@";
		  break;                                                             
        case PID_PlotAnimateVisAfter    :
          retstr += "@animatevisafter@";
		  break;                                                                                
      } // end switch                                                                           
	} // end if                                                                                 
  } // end for                                                                                  
  return retstr;                                                                                
}                                                                                               
                                                                                                
                                                                                                
string MuPADPlot::MeshStr (MuPADPlot* T) {
  string m = "";
  int nv = T->numvars;
  if (T->animate) nv--;
  if (T->ptype == "tube") {
    m = "Mesh=[#XPts#, #XPts#]";
  } else if (T->ptype == "conformal") {
    m = "Mesh=[#XPts#, #YPts#],Submesh=[#ConfHorizontalPts#,#ConfVerticalPts#]";
  } else {  
    switch (nv) {
      case 1:      m = "Mesh=#XPts#"; break;
      case 2:      m = "Mesh=[#XPts#, #YPts#]";   break;
      case 3:      m = "Mesh=[#XPts#, #YPts#, #ZPts#]";   break;
      case 4:      m = "Mesh=[#XPts#, #YPts#, #ZPts#]";   break;
    }
  }  
  return m;
}


string MuPADPlot::OrientationStr (MuPADPlot* T) {
//  string m = ", CameraDirection=[" + msiToString(x) + "," + msiToString(y) + "," + msiToString(z) + "]";
  string m="";
  return m;
}


// should turn into "plot::Function2d"
string MuPADPlot::PlotFuncStr(MuPADPlot* T) {
  string s = "plot::Function2d";
  if (T->dim[0] == '2') {
    if ((T->ptype == "polar")) {
      s = "plot::Polar";
    } else if (T->ptype == "explicitList") {
      s = "plot::Polygon2d";
    } else if (T->ptype == "implicit") {
      s = "plot::Implicit2d";
    } else if (T->ptype == "approximateIntegral") {
      s = "plot::Integral";
    } else if (T->ptype == "inequality") {
      s = "plot::Inequality";
    } else if (T->ptype == "parametric") {
      s = "plot::Curve2d";
    } else if (T->ptype == "conformal") {
      s = "plot::Conformal";
    } else if (T->ptype == "gradient") {
      s = "plot::VectorField2d";
    } else if (T->ptype == "vectorField") {
      s = "plot::VectorField2d";
    } else if (T->ptype == "ODE") {
      s = "plot::Ode2d";
    }
 
  } else if (T->dim[0] == '3') {
    if (T->ptype == "cylindrical") {
      s = "plot::Cylindrical";
    } else if (T->ptype == "spherical") {
      s = "plot::Spherical";
    } else if (T->ptype == "rectangular") {
      s = "plot::Function3d";
    } else if (T->ptype == "curve") {
      s = "plot::Curve3d";
    } else if (T->ptype == "explicitList") {
      s = "plot::Polygon3d";
    } else if (T->ptype == "implicit") {
      s = "plot::Implicit3d";
    } else if (T->ptype == "inequality") {
      s = "plot::Inequality";
    } else if (T->ptype == "parametric") {
      s = "plot::Surface";
    } else if (T->ptype == "conformal") {
      s = "plot::Conformal";
    } else if (T->ptype == "gradient") {
      s = "plot::VectorField3d";
    } else if (T->ptype == "vectorField") {
      s = "plot::VectorField3d";
    } else if (T->ptype == "ODE") {
      s = "plot::Ode3d";
    } else if (T->ptype == "tube") {
      s = "plot::Tube";
    }
 
  } else {
    TCI_ASSERT(!"ERROR: invalid plot dimension");
  }
  return s;
}


string MuPADPlot::PlotStr(MuPADPlot* T) {
  string pstr;
  if (T->ptype == "approximateIntegral") {
    pstr = "plot::Integral(@aifnparams@@aiparams@),plot::Function2d(@reqparams@)";
  } else if (T->plotno == 1) {
    pstr = "@plotfunc@(@reqparams@@localopts@@globalparams@)";
  } else {
    pstr = "@plotfunc@(@reqparams@@localopts@)";
  }
  return pstr;
}


string MuPADPlot::PointStyleStr (MuPADPlot* T) {
  string ret = "";
  if ((T->ptype != "vectorField") && (T->ptype != "gradient")) {
    U32 plottype;                                                                                    
    string val = T->psr->RetrievePlotParam (T->plotno, PID_PlotPointSymbol, plottype);                                     
    string ret = "";
    if ((val == "Squares") || (val == "Circles") || (val == "Crosses") || (val == "Diamonds") ||
        (val == "FilledSquares") || (val == "FilledCircles") || (val == "FilledDiamonds") || 
        (val == "Stars") || (val == "XCrosses"))               
      ret = ", PointStyle=" + val;                            
  }    
  return ret;                                              
}                                                          


string MuPADPlot::QueryStr(MuPADPlot* T) {
  string ret = "eval(#Expression#)";
  return ret;
}


string MuPADPlot::ReqParamsStr (MuPADPlot* T) {
  string pattern = "@exp@,@vars@,@mesh@";
  if (T->ptype == "explicitList")
    pattern = "@exp@";
  return pattern;
}


string MuPADPlot::SecondaryColorStr (MuPADPlot* T) {
  U32 plottype;                                                                                    
  string val = T->psr->RetrievePlotParam (T->plotno, PID_PlotSecondaryColor, plottype); 
  val = StringToColorArrayStr (val);                                    
  string ret = ", FillColor2=";
  return ret+val;                                              
}


string MuPADPlot::StringToColorArrayStr (string val) {
  char str[80];                                         
  double r = ((double) (hexval(val[1])*16+hexval(val[2])))/256.0;
  double g = ((double) (hexval(val[3])*16+hexval(val[4])))/256.0;
  double b = ((double) (hexval(val[5])*16+hexval(val[6])))/256.0;
  sprintf (str, "[%0.3f, %0.3f, %0.3f]", r, g, b);
  string ret = str;
  return ret;                                              
}


string MuPADPlot::SurfaceStyleStr (MuPADPlot* T) {
  string pattern = "";
  if ((T->ptype != "vectorField") && (T->ptype != "gradient")) {
    U32 plottype;                                                                                    
    string val = T->psr->RetrievePlotParam (T->plotno, PID_PlotSurfaceStyle, plottype);  
    if (val == "colorpatch") {
      pattern = ", FillColorType=Rainbow, Filled=TRUE";
    } else if (val == "hiddenline") {
      pattern = ", XLinesVisible=FALSE, YLinesVisible=FALSE";
      pattern += ", FillColorType=Rainbow, Filled=TRUE";
    } else if (val == "wireframe") {
      pattern = ", Filled=FALSE";
    } else if (val == "point") {
    } 
  }  
  return pattern;
}


string MuPADPlot::SurfaceMeshStr (MuPADPlot* T) {
  string pattern = "";
  if ((T->ptype != "tube") && (T->dim =="3")) {
    U32 plottype;                                                                                    
    string val = T->psr->RetrievePlotParam (T->plotno, PID_PlotSurfaceMesh, plottype);       
    if (val == "none") {
      pattern = ", XLinesVisible=FALSE, YLinesVisible=FALSE";
    } else if (val == "vlines") {
      pattern = ", XLinesVisible=FALSE";
    } else if (val == "hlines") {
      pattern = ", XLinesVisible=FALSE";
    } 
  }  
  return pattern;
}


string MuPADPlot::TitleStr (MuPADPlot *T) {
  U32 plottype;                                                                                    
  string val = T->psr->RetrievePlotParam (T->plotno, PID_GraphXAxisLabel, plottype);        
  string s=", AxesTitles=[\"$XAxisLabel$\",\"$YAxisLabel$\"]";
  if (T->dim[0] == '3')
    s=", AxesTitles=[\"$XAxisLabel$\",\"$YAxisLabel$\", \"$ZAxisLabel$\"]";
  return s;
}


string MuPADPlot::UNIMPLEMENTED(MuPADPlot* T) {
  string pstr = "";
  cout << "MuPADPlot:: A required transformation method is not yet implemented\n";
  return pstr;
}


string MuPADPlot::VarsStr (MuPADPlot* T) {
  string v = "";
    
  if (T->ptype == "conformal") {
    if (T->animate) {
      v = "%XVar%=#XMin#+(#YMin#)*I..#XMax#+(#YMax#)*I,%ZVar%=#ZMax#..#ZMin#";
    } else {
      v = "%XVar%=#XMin#+(#YMin#)*I..#XMax#+(#YMax#)*I";
    }  
  } else if (T->numvars == 1) {
    v = "%XVar%=#XMin#..#XMax#";
  } else if (T->numvars == 2) {
    v = "%XVar%=#XMin#..#XMax#, %YVar%=#YMin#..#YMax#";
  } else if (T->numvars == 3) {
    v = "%XVar%=#XMin#..#XMax#, %YVar%=#YMin#..#YMax#, %ZVar%=#ZMin#..#ZMax#";
  } else if (T->numvars == 4) {
    v = "ERROR 4 vars not supported";  
  }
  return v;
}

/////////////////////////////////////////////
