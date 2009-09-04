// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

#ifndef MREQUEST_H
#define MREQUEST_H

#include "CmpTypes.h"
#include "SNode.h"
#include <string.h>
#include <vector>
using std::vector;
#include <string>
using std::string;

typedef struct tagPARAM_SPEC
{
  tagPARAM_SPEC *next;
  U32 param_ID;
  U32 data_type;
  char *ASCII_data;
  U16 *WIDE_data;
  SEMANTICS_NODE *p_semantics;
} PARAM_SPEC;

class DefStore;

class MathServiceRequest
{
public:
  MathServiceRequest();
  virtual ~MathServiceRequest();

  U32 GetClientHandle() { return client_ID; }
  void PutClientHandle(U32 h) { client_ID = h; }

  const char *GetEngineNamePtr() { return engine_name; }
  void PutEngineName(const char *eng_name);

  U32 GetEngineID() { return engine_ID; }
  void PutEngineID(U32 new_ID) { engine_ID = new_ID; }

  DefStore *GetDefStore() { return defstore; }
  void PutDefStore(DefStore * new_store) { defstore = new_store; }

  const char *GetOpNamePtr() { return op_name; }
  void PutOpName(const char *opname);

  U32 GetOpID() { return op_ID; }
  void PutOpID(int opcode) { op_ID = opcode; }

  MarkupType GetMarkupType() { return markup_type; }
  void PutMarkupType(MarkupType new_type) { markup_type = new_type; }

  const char *GetASCIIMarkupPtr() { return a_markup; }
  void PutASCIIMarkup(const char *src);

  const U16 *GetWideMarkupPtr() { return w_markup; }
  void PutWideMarkup(const U16 * src);

  void PutParam(U32 p_ID, U32 p_type, const char *str_val);
  const char *GetParam(U32 p_ID, U32 & p_type);
  void PutWideParam(U32 p_ID, U32 p_type, const U16 * wide_str);
  void AddSemanticsToParam(U32 p_ID, SEMANTICS_NODE * semantics_tree);
  SEMANTICS_NODE *GetSemanticsFromParam(U32 p_ID);
  int nMarkupParams();

protected:
  void DisposeParamList();

  U32 client_ID;
  char engine_name[80];
  U32 engine_ID;
  char op_name[80];
  U32 op_ID;

  MarkupType markup_type;
  // Two "owned" heap variables
  char *a_markup;
  U16 *w_markup;

  PARAM_SPEC *param_list;
  DefStore *defstore;
};


class PlotServiceRequest: public MathServiceRequest
{
public:
  PlotServiceRequest();
  virtual ~PlotServiceRequest();
  void StorePlotParam(U32 plot_no, const char *str_key, const char *str_val, U32 p_type);
  void StorePlotParam(const char *str_key, const char *str_val, U32 p_type);
  bool HasPlot(U32 plot_no);
  int  PlotCount ();
  const char *RetrievePlotParam(U32 targ_ID, U32 & p_type);
  const char *RetrievePlotParam(const char *str_key, U32 & p_type);
  const char *RetrievePlotParam(U32 plot_no, const char *str_key, U32 & p_type);
  const char *RetrievePlotParam(U32 plot_no, U32 targ_ID, U32 & p_type);

  void AddSemanticsToParam(U32 plot_no, const char * key, SEMANTICS_NODE * semantics_tree);
  void AddSemanticsToParam(U32 plot_no, U32 targ_ID, SEMANTICS_NODE * semantics_tree);
  SEMANTICS_NODE *GetSemanticsFromParam(U32 plot_no, const char * key);
  SEMANTICS_NODE *GetSemanticsFromParam(U32 plot_no, U32 targ_ID);
    
private:
  U32 NameToPID (const char *name);
  void PutPlotParam(PARAM_SPEC *param_list, U32 targ_ID, U32 p_type, const char *str_val);
  const char *GetPlotParam(PARAM_SPEC *param_list, U32 targ_ID, U32 & p_type);
  const char *GetPlotParam(U32 plot_no, U32 targ_ID, U32 & p_type);

  static vector<string> paramNames;  
  std::vector<PARAM_SPEC*> pspecs;
};


#endif
