// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

/*  Design Notes

  A "MathServiceRequest" is intended to carry data from a computation
  client to a "MathWorkShop".  This data identifies the client, the task,
  the engine, and includes the math markup and any parameters needed.

  This object is "owned" by the client.  Within MathWorkShop the data
  carried by a MathServiceRequest is read-only (ie. const).

  We may want to add an "internal_param_list" pointer for params
  generated and managed within MathWorkShop.
*/

#include "MRequest.h"
#include "fltutils.h"
#include <string>

MathServiceRequest::MathServiceRequest()
{
  markup_type = MT_UNDEFINED;
  a_markup = NULL;
  w_markup = NULL;

  op_name[0] = 0;
  op_ID = 0;
  engine_name[0] = 0;
  engine_ID = 0;

  param_list = NULL;
  defstore = NULL;
}

MathServiceRequest::~MathServiceRequest()
{
  delete[] a_markup;
  delete[] w_markup;

  DisposeParamList();
}

void MathServiceRequest::PutEngineName(const char *eng_name)
{
  if (eng_name) {
    size_t zln = strlen(eng_name);
    if (zln < 80)
      strcpy(engine_name, eng_name);
    else
      TCI_ASSERT(0);
  } else {
    engine_name[0] = 0;
  }
}

void MathServiceRequest::PutOpName(const char *opname)
{
  if (opname) {
    size_t zln = strlen(opname);
    if (zln < 80)
      strcpy(op_name, opname);
    else
      TCI_ASSERT(0);
  } else {
    op_name[0] = 0;
  }
}

void MathServiceRequest::PutASCIIMarkup(const char *src)
{
  if (a_markup) {
    delete[] a_markup;
    a_markup = NULL;
  }
  if (src) {
    size_t zln = strlen(src);
    a_markup = new char[zln + 1];
    strcpy(a_markup, src);
  }
}

void MathServiceRequest::PutWideMarkup(const U16 * src)
{
  if (w_markup) {
    delete[]w_markup;
    w_markup = NULL;
  }
  if (src) {
    const U16 *p = src;
    while (*p)
      p++;
    U32 zln = p - src;
    w_markup = new U16[zln + 1];
    p = src;
    U32 i = 0;
    while (i <= zln) {
      w_markup[i++] = *p;
      p++;
    }
  }
}

const char *MathServiceRequest::GetParam(U32 targ_ID, U32 & p_type)
{
  const char *rv = NULL;
  p_type = 0;

  PARAM_SPEC *rover = param_list;
  while (rover) {
    if (rover->param_ID == targ_ID) {
      rv = rover->ASCII_data;
      p_type = rover->data_type;
      break;
    }
    rover = rover->next;
  }

  return rv;
}

void MathServiceRequest::PutParam(U32 targ_ID, U32 p_type,
                                  const char *str_val)
{
  bool done = false;
  PARAM_SPEC *rover = param_list;
  while (rover && !done) {
    if (rover->param_ID == targ_ID) {

      // Here, the target param already exists. We reset it.
      if (rover->ASCII_data) {
        delete[] rover->ASCII_data;
        rover->ASCII_data = NULL;
      }
      if (str_val) {
        size_t zln = strlen(str_val);
        char *tmp = new char[zln + 1];
        strcpy(tmp, str_val);
        rover->ASCII_data = tmp;
      }
      rover->data_type = p_type;

      if (rover->WIDE_data) {
        delete[] rover->WIDE_data;
        rover->WIDE_data = NULL;
      }

      DisposeSList(rover->p_semantics);
      rover->p_semantics = NULL;
      done = true;
    }
    rover = rover->next;
  }

  if (!done) {
    PARAM_SPEC *new_rec = new PARAM_SPEC();
    new_rec->next = param_list;
    new_rec->param_ID = targ_ID;
    new_rec->data_type = p_type;
    if (str_val) {
      size_t zln = strlen(str_val);
      char *tmp = new char[zln + 1];
      strcpy(tmp, str_val);
      new_rec->ASCII_data = tmp;
    } else {
      new_rec->ASCII_data = NULL;
    }
    new_rec->WIDE_data = NULL;
    new_rec->p_semantics = NULL;
    param_list = new_rec;
  }
}

void MathServiceRequest::PutWideParam(U32 p_ID, U32 p_type,
                                      const U16 * wide_str)
{
  if (p_type == zPT_WIDE_natural
      || p_type == zPT_WIDE_mmlmarkup
      || p_type == zPT_WIDE_text || p_type == zPT_WIDE_real) {
    const char *z_ascii = NULL;
    if (wide_str)
      z_ascii = WideToASCII(wide_str);
    U32 new_type = 0;
    if (p_type == zPT_WIDE_natural)
      new_type = zPT_ASCII_natural;
    else if (p_type == zPT_WIDE_mmlmarkup)
      new_type = zPT_ASCII_mmlmarkup;
    else if (p_type == zPT_WIDE_text)
      new_type = zPT_ASCII_text;
    else if (p_type == zPT_WIDE_real)
      new_type = zPT_ASCII_real;
    PutParam(p_ID, new_type, z_ascii);
  } else {
    TCI_ASSERT(0);
  }
}

void MathServiceRequest::AddSemanticsToParam(U32 targ_ID,
                                             SEMANTICS_NODE * semantics_tree)
{
  PARAM_SPEC *rover = param_list;
  while (rover) {
    if (rover->param_ID == targ_ID)
      break;
    rover = rover->next;
  }

  if (rover) {
    DisposeSList(rover->p_semantics);
    rover->p_semantics = NULL;
    if (semantics_tree)
      rover->p_semantics = semantics_tree;
  }
}

SEMANTICS_NODE *MathServiceRequest::GetSemanticsFromParam(U32 targ_ID)
{
  PARAM_SPEC *rover = param_list;
  while (rover) {
    if (rover->param_ID == targ_ID)
      return rover->p_semantics;
    rover = rover->next;
  }

  return NULL;
}

int MathServiceRequest::nMarkupParams()
{
  int rv = 0;

  PARAM_SPEC *rover = param_list;
  while (rover) {
    if (rover->data_type == zPT_ASCII_mmlmarkup ||
        rover->data_type == zPT_WIDE_mmlmarkup)
      rv++;
    rover = rover->next;
  }

  return rv;
}

void MathServiceRequest::DisposeParamList()
{
  PARAM_SPEC *rover = param_list;
  while (rover) {
    PARAM_SPEC *del = rover;
    rover = rover->next;
    delete[] del->ASCII_data;
    delete[] del->WIDE_data;
    DisposeSList(del->p_semantics);
    delete del;
  }
}

vector<string> PlotServiceRequest::paramNames;  
//-------------------------------------------------------------
PlotServiceRequest::PlotServiceRequest() {
}

PlotServiceRequest::~PlotServiceRequest() {
  for (vector<string>::size_type ix=0; ix!=pspecs.size(); ++ix) {
    PARAM_SPEC *rover = pspecs[ix];
    while (rover) {
      PARAM_SPEC *del = rover;
      rover = rover->next;
      delete[] del->ASCII_data;
      delete[] del->WIDE_data;
      DisposeSList(del->p_semantics);
      delete del;
    }
  }  
}

// Return the PID of the named parameter
U32 PlotServiceRequest::NameToPID (const char *name) {
  string s = name;
  if (s == "ImageFile")         return (PID_GraphImageFile);
  if (s == "XAxisLabel")        return (PID_GraphXAxisLabel);
  if (s == "YAxisLabel")        return (PID_GraphYAxisLabel);            
  if (s == "ZAxisLabel")        return (PID_GraphZAxisLabel);            
  if (s == "Width")             return (PID_GraphWidth );            
  if (s == "Height")            return (PID_GraphHeight );            
  if (s == "PrintAttribute")    return (PID_GraphPrintAttribute );            
  if (s == "Placement")         return (PID_GraphPlacement );            
  if (s == "Offset")            return (PID_GraphOffset );            
  if (s == "Float")             return (PID_GraphFloat );            
  if (s == "PrintFrame")        return (PID_GraphPrintFrame );            
  if (s == "Key")               return (PID_GraphKey );            
  if (s == "Name")              return (PID_GraphName );            
  if (s == "CaptionText")       return (PID_GraphCaptionText );            
  if (s == "CaptionPlace")      return (PID_GraphCaptionPlace );            
  if (s == "Units")             return (PID_GraphUnits );            
  if (s == "AxesType")          return (PID_GraphAxesType );            
  if (s == "EqualScaling")      return (PID_GraphEqualScaling );            
  if (s == "EnableTicks")       return (PID_GraphEnableTicks );            
  if (s == "XTickCount")        return (PID_GraphXTickCount );            
  if (s == "YTickCount")        return (PID_GraphYTickCount );            
  if (s == "ZTickCount")        return (PID_GraphZTickCount );            
  if (s == "AxesTips")          return (PID_GraphAxesTips );            
  if (s == "GridLines")         return (PID_GraphGridLines );            
  if (s == "BGColor")           return (PID_GraphBGColor );            
  if (s == "Dimension")         return (PID_GraphDimension);            
  if (s == "AxisScale")         return (PID_GraphAxesScaling);
  if (s == "ViewingBoxXMin")    return (PID_GraphViewBoxXMin);
  if (s == "ViewingBoxXMin")    return (PID_GraphViewBoxXMax);
  if (s == "ViewingBoxXMin")    return (PID_GraphViewBoxYMin);
  if (s == "ViewingBoxXMin")    return (PID_GraphViewBoxYMax);
  if (s == "ViewingBoxXMin")    return (PID_GraphViewBoxZMin);
  if (s == "ViewingBoxXMin")    return (PID_GraphViewBoxZMax);
  if (s == "CameraLocationX")   return (PID_GraphCameraLocationX);  
  if (s == "CameraLocationY")   return (PID_GraphCameraLocationY);  
  if (s == "CameraLocationZ")   return (PID_GraphCameraLocationZ);  
  if (s == "FocalPointX")       return (PID_GraphFocalPointX    );  
  if (s == "FocalPointY")       return (PID_GraphFocalPointY    );  
  if (s == "FocalPointZ")       return (PID_GraphFocalPointZ    );  
  if (s == "UpVectorX")         return (PID_GraphUpVectorX      );  
  if (s == "UpVectorY")         return (PID_GraphUpVectorY      );  
  if (s == "UpVectorZ")         return (PID_GraphUpVectorZ      );  
  if (s == "ViewingAngle")      return (PID_GraphViewingAngle   );  
  if (s == "OrthogonalProjection") return (PID_GraphOrthogonalProjection);  
  if (s == "KeepUp")            return (PID_GraphKeepUp);  
          
//  if (s == "PlotStatus")          return (PID_PlotStatus);            
  if (s == "PlotType")          return (PID_PlotType);            
  if (s == "LineStyle")         return (PID_PlotLineStyle);            
  if (s == "LineThickness")     return (PID_PlotLineThickness);            
  if (s == "LineColor")         return (PID_PlotLineColor);                  
  if (s == "Expression")        return (PID_PlotExpression);            
  if (s == "XMin")              return (PID_PlotXMin);                  
  if (s == "XMax")              return (PID_PlotXMax);                  
  if (s == "YMin")              return (PID_PlotYMin);                  
  if (s == "YMax")              return (PID_PlotYMax);                  
  if (s == "ZMin")              return (PID_PlotZMin);                  
  if (s == "ZMax")              return (PID_PlotZMax);                  
  if (s == "AnimMin")           return (PID_PlotAnimMin);                  
  if (s == "AnimMax")           return (PID_PlotAnimMax);                  
  if (s == "XVar")              return (PID_PlotXVar);                  
  if (s == "YVar")              return (PID_PlotYVar);                  
  if (s == "ZVar")              return (PID_PlotZVar);                  
  if (s == "AnimVar")           return (PID_PlotAnimVar);                  
  if (s == "XPts")              return (PID_PlotXPts);                  
  if (s == "YPts")              return (PID_PlotYPts);                  
  if (s == "ZPts")              return (PID_PlotZPts);                  
  if (s == "DiscAdjust")        return (PID_PlotDiscAdjust);            
  if (s == "DirectionalShading")return (PID_PlotDirectionalShading);            
  if (s == "BaseColor")         return (PID_PlotBaseColor);            
  if (s == "SecondaryColor")    return (PID_PlotSecondaryColor);            
  if (s == "PointStyle")        return (PID_PlotPointSymbol);            
  if (s == "IncludePoints")     return (PID_PlotIncludePoints);            
  if (s == "IncludeLines")      return (PID_PlotIncludeLines);            
  if (s == "SurfaceStyle")      return (PID_PlotSurfaceStyle);            
  if (s == "SurfaceMesh")       return (PID_PlotSurfaceMesh);            
  if (s == "AnimationVariable") return (PID_PlotAnimationVariable);            
  if (s == "CameraLocationX")   return (PID_PlotCameraLocationX);            
  if (s == "CameraLocationY")   return (PID_PlotCameraLocationY);            
  if (s == "CameraLocationZ")   return (PID_PlotCameraLocationZ);            
  if (s == "FontFamily")        return (PID_PlotFontFamily); 
  if (s == "IncludeLines")      return (PID_PlotIncludeLines);
  if (s == "TubeRadius")        return (PID_PlotTubeRadius);
  if (s == "TubeRadialPts")     return (PID_PlotTubeRadialPts);
  if (s == "AISubIntervals")    return (PID_PlotAISubIntervals);
  if (s == "AIMethod")          return (PID_PlotAIMethod);
  if (s == "AIInfo")            return (PID_PlotAIInfo);
  if (s == "FillPattern")       return (PID_PlotFillPattern);
  if (s == "Animate")           return (PID_PlotAnimate );            
  if (s == "AnimateStart")      return (PID_PlotAnimateStart);    
  if (s == "AnimateEnd")        return (PID_PlotAnimateEnd);   	 
  if (s == "AnimateFPS")        return (PID_PlotAnimateFPS);  	 
  if (s == "AnimateVisBefore")  return (PID_PlotAnimateVisBefore);
  if (s == "AnimateVisAfter")   return (PID_PlotAnimateVisAfter); 
  if (s == "ConfHorizontalPts") return (PID_PlotConfHorizontalPts); 
  if (s == "ConfVerticalPts")   return (PID_PlotConfVerticalPts); 

  return (-1);
}


// Parameters for PSRs are stored in a vector of pspecs. The 0th element
// of the vector stores Graph parameters (one per graph). The others store
// Plot parameters (one for each plot); determined by the constants
// PID_GraphFirstParam and PID_PlotFirstParam
void PlotServiceRequest::StorePlotParam(U32 plot_no, const char *str_key, 
                                    const char *str_val, U32 p_type) {
  // if pspecs doesn't have at least plot_no, add them.
  for (std::vector<string>::size_type last = pspecs.size(); last <= plot_no; ++last) {
    PARAM_SPEC *new_rec = new PARAM_SPEC();
    new_rec->next = NULL;
    new_rec->param_ID = 0;
    new_rec->data_type = 0;
    pspecs.push_back (new_rec);
  }
  
  // lookup the key and store either with the plot parameters or the graph parameters
  U32 key = NameToPID (str_key);
  if ((key >= PID_GraphFirstParam) && (key < PID_PlotFirstParam))
    plot_no = 0;
  PutPlotParam (pspecs[plot_no], key, p_type, str_val);
}

// Default the plot_no to zero
void PlotServiceRequest::StorePlotParam(const char *str_key, 
                                    const char *str_val, U32 p_type) {
  U32 plot_no = 0;
  StorePlotParam (plot_no, str_key, str_val, p_type);
}

// return the string associated with this parameter key for this plot
const char *PlotServiceRequest::RetrievePlotParam(U32 plot_no, U32 targ_id, U32 & p_type) {
  if ((pspecs.size() <= plot_no) || (pspecs[plot_no] == NULL)) {
    TCI_ASSERT (!"RetrievePlotParam plot_number out of range");
    return ("ERROR in PSR::RetrievePlotParam plot number out of range");
  }
  if ((targ_id >= PID_GraphFirstParam) && (targ_id < PID_PlotFirstParam))
     plot_no = 0;
  return (GetPlotParam(pspecs[plot_no], targ_id, p_type));  
}

// return the string associated with this parameter key for this plot
const char *PlotServiceRequest::RetrievePlotParam(U32 plot_no, const char *str_key, U32 & p_type) {
  return (RetrievePlotParam(plot_no, NameToPID(str_key), p_type));  
}

// return the string associated with this parameter key for this plot
const char *PlotServiceRequest::RetrievePlotParam(const char *str_key, U32 & p_type) {
  U32 plot_no = 0;
  return (RetrievePlotParam (plot_no, str_key, p_type));
}

// return the string associated with this parameter key for this plot
const char *PlotServiceRequest::RetrievePlotParam(U32 targ_id, U32 & p_type) {
  U32 plot_no = 0;
  return (RetrievePlotParam (plot_no, targ_id, p_type));
}

const char *PlotServiceRequest::GetPlotParam(PARAM_SPEC *param_list, U32 targ_ID, U32 & p_type) {
  const char *rv = NULL;
  p_type = 0;
  PARAM_SPEC *rover = param_list;
  while (rover) {
    if (rover->param_ID == targ_ID) {
      rv = rover->ASCII_data;
      p_type = rover->data_type;
      break;
    }
    rover = rover->next;
  }
  if (!rv) {
    char *x = new char[1];
	x[0] = 0;
	rv = x;
  }
  return rv;
}

const char *PlotServiceRequest::GetPlotParam(U32 plot_no, U32 targ_ID, U32 & p_type) {
  return (GetPlotParam(pspecs[plot_no], targ_ID, p_type));  
}

bool PlotServiceRequest::HasPlot(U32 plot_no) {
  if ((pspecs.size() <= plot_no) || (pspecs[plot_no] == NULL)) {
    return (false);
  }
  return (true);  
}

int PlotServiceRequest::PlotCount() {
  return (pspecs.size() - 1);  
}

void PlotServiceRequest::PutPlotParam(PARAM_SPEC *param_list, U32 targ_ID, U32 p_type, const char *str_val){
  bool done = false;
  PARAM_SPEC *rover = param_list;
  while (rover && !done) {
    if (rover->param_ID == targ_ID) {

      // Here, the target param already exists. We reset it.
      if (str_val) {
        size_t zln = strlen(str_val);
        char *tmp = new char[zln + 1];
        strcpy(tmp, str_val);
        rover->ASCII_data = tmp;
      }
      rover->data_type = p_type;

      DisposeSList(rover->p_semantics);
      rover->p_semantics = NULL;
      done = true;
    }
    rover = rover->next;
  }

  if (!done) {
    PARAM_SPEC *new_rec = new PARAM_SPEC();
    new_rec->next = NULL;
    new_rec->param_ID = targ_ID;
    new_rec->data_type = p_type;
    if (str_val) {
      size_t zln = strlen(str_val);
      char *tmp = new char[zln + 1];
      strcpy(tmp, str_val);
      new_rec->ASCII_data = tmp;
    } else {
      new_rec->ASCII_data = NULL;
    }
    new_rec->WIDE_data = NULL;
    new_rec->p_semantics = NULL;
    for (rover = param_list; rover->next; rover = rover->next) {
    }
    rover->next = new_rec;
  }
}

void PlotServiceRequest::AddSemanticsToParam(U32 plot_no, const char *key,
                                             SEMANTICS_NODE * semantics_tree) {
  AddSemanticsToParam (plot_no, NameToPID (key), semantics_tree);
}

void PlotServiceRequest::AddSemanticsToParam(U32 plot_no, U32 targ_ID,
                                             SEMANTICS_NODE * semantics_tree) {
  if ((targ_ID >= PID_GraphFirstParam) && (targ_ID < PID_PlotFirstParam))
    plot_no = 0;                                             
  if ((pspecs.size() >= plot_no) && (pspecs[plot_no] != NULL)) {
    PARAM_SPEC *rover = pspecs[plot_no];
    while (rover) {
      if (rover->param_ID == targ_ID)
        break;
      rover = rover->next;
    }

    if (rover) {
      DisposeSList(rover->p_semantics);
      rover->p_semantics = semantics_tree;
    }
    else {
      // ERROR AddSemantics didn't find a place for semantics tree
    }  
  }
}

SEMANTICS_NODE *PlotServiceRequest::GetSemanticsFromParam(U32 plot_no, const char *key) {
  return (GetSemanticsFromParam (plot_no, NameToPID (key)));
}

SEMANTICS_NODE *PlotServiceRequest::GetSemanticsFromParam(U32 plot_no, U32 targ_ID) {
  // ensure it's within range first ...
  if ((targ_ID < PID_GraphFirstParam) || (targ_ID >= PID_last))
    return NULL;
  if ((targ_ID >= PID_GraphFirstParam) && (targ_ID < PID_PlotFirstParam))
    plot_no = 0;                                             
  if ((pspecs.size() > plot_no) && (pspecs[plot_no] != NULL)) {
    PARAM_SPEC *rover = pspecs[plot_no];
    while (rover) {
      if (rover->param_ID == targ_ID) {
        return rover->p_semantics;
      }  
      rover = rover->next;
    }
  }
  return NULL;
}
