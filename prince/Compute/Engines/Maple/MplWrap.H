// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

/* This object is intented for early development only.

  When interfaces stabilize, the public functions of this object
  will consistute the access functions to a family of DLLs - 
  MuPADWrapDLL, Maple5WrapDLL, etc.

*/

#ifndef MapleWrapper_H
#define MapleWrapper_H

#include "../../CmpTypes.h"
#include "../EngWrap.h"
#include "../fltutils.h"

class eAnalyzer;
class Grammar;
class MathServiceRequest;
class MathResult;
class Maple2Tree;


class MapleWrapper {
public:
  MapleWrapper( void* ID_dBase,void* NOM_dBase );
  ~MapleWrapper();

  bool iLoadStrsAndDLL( const char* engpath, const char* libpath );
  bool iInitialize( const char* libpath, const char* champath );
  bool SetEngineState( int attr_ID, const char* new_value );
  void     iDisposeSList( SEMANTICS_NODE* slist );
  int      iGetEngStrPtr( int str_ID,char** engstr, int* engstr_ln );

  bool GetdBaseNamedRecord( const char* bin_name, const char* op_name,
                                const char** eng_dbase_rec, U32& ID, U32& subID );
  bool GetdBaseIDRecord( const char* bin_name, U32 ID, U32 subID,
                             const char** eng_dbase_rec, char* name_buffer, size_t lim );

  int      iExecute( const char* cmd );

  SEMANTICS_NODE* iProcessRequest( MathServiceRequest* msr,MathResult* mr );

  void      iReleaseClient( U32 client_ID, const char* def_list );
  void      iCleanupAndUnloadDLL();

private:
  char*     Execute( const char* cmd );

  char*     CreateEngineCommandStr( MathServiceRequest& msr,U32 cmd_ID, const char* eng_expr, int& error_code );
  char*     ProcessSemList(  SEMANTICS_NODE* head, int& error_code );
  char*     ProcessSemNode(  SEMANTICS_NODE* handle, int& error_code );

  char*     VARIABLEtoEstr(   SEMANTICS_NODE* node_info, int& error_code );
  char*     QVARIABLEtoEstr(  SEMANTICS_NODE* node_info, int& error_code );
  char*     NUMBERtoEstr(     SEMANTICS_NODE* node_info, int& error_code );
  char*     MIXEDNUMBERtoEstr(SEMANTICS_NODE* node_info, int& error_code );
  char*     UCONSTANTtoEstr(  SEMANTICS_NODE* snode, int& error_code );
  char*     FUNCTIONtoEstr(   SEMANTICS_NODE* node_info, int& error_code );
  char*     LIMFUNCtoEstr(    SEMANTICS_NODE* node_info, int& error_code );
  char*     BIGOPtoEstr(      SEMANTICS_NODE* node_info, int& error_code );
  char*     INTEGRALtoEstr(   SEMANTICS_NODE* node_info, int& error_code );
  char*     OPERATORtoEstr(   SEMANTICS_NODE* node_info, bool& is_infix, int& error_code );
  char*     POLISHOPtoEstr(   SEMANTICS_NODE* node_info, bool& is_infix, int& error_code );
  char*     INTERVALtoEstr(   SEMANTICS_NODE* node_info, int& error_code );
  char*     FRACTIONtoEstr(   SEMANTICS_NODE* node_info, const char* key, int& error_code );
  char*     POWERFORMtoEstr(  SEMANTICS_NODE* node_info, bool no_surds, int& error_code );
  char*     SQROOTtoEstr(     SEMANTICS_NODE* node_info, int& error_code );
  char*     ROOTtoEstr(       SEMANTICS_NODE* node_info, int& error_code );
  char*     TABULATIONtoEstr( SEMANTICS_NODE* node_info, int& error_code );
  char*     SIMPLEXtoEstr(    SEMANTICS_NODE* node_info, int& error_code );
  char*     NUMSYSTEMtoEstr(  SEMANTICS_NODE* node_info, int& error_code );
  char*     SOLVESYSTEMtoEstr(SEMANTICS_NODE* node_info, int& error_code );
  char*     ODEtoEstr(        SEMANTICS_NODE* node_info, int& error_code );
  char*     ODENumerictoEstr( SEMANTICS_NODE* node_info, int& error_code );
  char*     TEXTtoEstr(       SEMANTICS_NODE* node_info, int& error_code );
  char*     UNITtoEstr(       SEMANTICS_NODE* node_info, int& error_code );
  char*     SETtoEstr(        SEMANTICS_NODE* set_info, int& error_code );
  char*     CONJUGATEtoEstr(  SEMANTICS_NODE* node_info, int& error_code );
  char*     TRANSPOSEtoEstr(  SEMANTICS_NODE* node_info, char TorH, int& error_code );
  char*     LISTtoEstr(       SEMANTICS_NODE* func_info,
								    char* start_str, char* end_str,
								    char* join_str, int& error_code );
  char*     VARASSIGNtoEstr(  SEMANTICS_NODE* node_info, int& error_code );
  char*     FUNCDEFtoEstr(    SEMANTICS_NODE* node_info, int& error_code );
  char*     SURDasPOWERtoEstr(SEMANTICS_NODE* surd_info, int& error_code );
  char*     POWERasSURDtoEstr(SEMANTICS_NODE* s_pform, int& error_code );
  char*     POWERTRANSPOSEtoEstr( SEMANTICS_NODE* s_powerform,
                                    char which_one, int& error_code );

  char*     DERIVATIVEtoEstr( SEMANTICS_NODE* derivative_info, int& error_code );

  char*     PRIMEStoEstr( SEMANTICS_NODE* derivative_info, int& error_code );
  char*     ODEBoundarytoEstr( SEMANTICS_NODE* derivative_info, int& error_code );
  char*     SUBSTITUTIONtoEstr( SEMANTICS_NODE* subst_info, int& error_code );
  char*     MULTIARGFUNCtoEstr( SEMANTICS_NODE* subst_info, int& error_code );

  char*     MODtoEstr( SEMANTICS_NODE* s_mod, int& err_code );

  char*     PROBFUNCtoEstr( SEMANTICS_NODE* sem_op_node, const char* func_template, U32 ID, int& error_code );
  char*     TRANSFORMtoEstr( SEMANTICS_NODE* s_func, const char* expr, const char* production, int& error_code );
  char*     VCALCOPtoEstr( MathServiceRequest* msr, SEMANTICS_NODE* s_func, U32 func_ID,
                           const char* expr, const char* production,
                           int& error_code );
  char*     NORMtoEstr( SEMANTICS_NODE* snode, int& error_code );
  char*     MATRIXMINUStoEstr( SEMANTICS_NODE* s_minus, const char* production, int& error_code );

  ClientAliases_REC*  CreateClientRecord( U32 client_num );
  ClientAliases_REC*  GetClientAliasesRec( U32 client_ID );
  NAME2AKA_REC*       PushNamingRecord( const char* eng_name, const char* canon_name, const char* user_name,
                                    NameIlk obj_ilk, NAME2AKA_REC* aka_list );
  void            DisposeAKAsList( NAME2AKA_REC* aka_list );
  NAME2AKA_REC*   GetAKARec( NAME2AKA_REC* aka_list, const char* canonical_targ, const char* user_targ );
  void            PurgeDEFuncNames();

  char*     CreateEngName( U32 client_ID, const char* var_canonical_name,
                           const char* base_name, bool use_alias, NameIlk function_ilk );
  char*     GetEngNameForVar( U32 frame_ID,const char* user_name, const char* canonical_ID );

  bool  CmdGoesToEngine( U32 cmd_ID );
  bool  LoadPackages( const char* new_packages );

  char*     CanonicalNumberToEngineStr( const char* num_str, int& error_code );
  void      SetWrapperCmdContext( U32 ID, char* new_packages );

  char*     CreateStandardeStr( const char* eng_expr, const char* cmd_template, int& error_code );
  char*     CreateEQN2MATeStr( MathServiceRequest& msr, const char* eng_expr, const char* cmd_template,
                                        int& error_code );
  char*     CreateCOLLECTeStr( MathServiceRequest& msr, const char* eng_expr, const char* cmd_template,
                                        int& error_code );
  char*     CreatePSERIESeStr( MathServiceRequest& msr, const char* eng_expr, const char* cmd_template,
                                        int& error_code );
  char*     CreateQUANTILEeStr( MathServiceRequest& msr, const char* eng_expr, const char* cmd_template,
                                        int& error_code );
  char*     CreateMOMENTeStr( MathServiceRequest& msr, const char* eng_expr, const char* cmd_template,
                                        int& error_code );
  char*     CreateRANDMATRIXeStr( MathServiceRequest& msr, const char* eng_expr, const char* cmd_template,
                                        int& error_code );
  char*     CreateRANDOMNUMeStr( MathServiceRequest& msr, int& error_code );
  char*     CreateRESHAPEeStr( const char* eng_expr, const char* cmd_template, int& error_code );
  char*     CreateWHICHVAReStr( MathServiceRequest& msr, U32 cmd_ID,
                                    const char* eng_expr, int& error_code );
  char*     CreateTRANSFORMeStr( const char* eng_expr, const char* cmd_template, int& error_code );
  char*     CreateVCALCeStr( MathServiceRequest& msr, const char* eng_zstr, const char* cmd_template,
                                            int& error_code );
  char*     CreateITERATEeStr( MathServiceRequest& msr, const char* cmd_template, int& error_code );
  char*     CreateSIntegraleStr( MathServiceRequest& msr, U32 cmd_ID, const char* eng_zstr, const char* cmd_template,
                                    int& error_code );
  char*     CreateAPPROXINTeStr( MathServiceRequest& msr, const char* eng_zstr, const char* cmd_template,
                                    int& error_code );
  char*     CreateFITCURVEeStr( MathServiceRequest& msr, const char* eng_zstr, int& error_code );
  char*     CreateFILLMATRIXeStr( MathServiceRequest& msr, int& error_code );
  char*     CreateMAPSeStr( MathServiceRequest& msr, const char* eng_zstr, const char* cmd_template,
                                            int& error_code );

  bool  IsMatrix( SEMANTICS_NODE* s_node );
  char*     GetEngNameForUserFunc( SEMANTICS_NODE* s_func );
  char*     GetDiffVarStr( SEMANTICS_NODE* var_snode, int& error_code );
  SEMANTICS_NODE* LocateVar( BUCKET_REC* expr_bucket );
  char*     Subst2Estr( SEMANTICS_NODE* s_subst,
                                 bool is_lower,
                                 BUCKET_REC* expr_bucket,
                                 BUCKET_REC* other_subst_bucket,
                                 int& error_code );
  char*     Piecewise2Estr( SEMANTICS_NODE* pw_fence, int& err_code );
  char*     Recursion2Estr( SEMANTICS_NODE* recursion, int& err_code );

  int       GetEngineMessageID( SEMANTICS_NODE* s_tree );

  char*     SemDomainToEstr( SEMANTICS_NODE* s_list,
                                        int& error_code );
  void      ODEFuncName2eStr( SEMANTICS_NODE* curr_handle,
                                char** sfor_func_str,U32& sfor_ln );
  bool  NumericFuncName2eStr( SEMANTICS_NODE* curr_handle,
                                char** sfor_func_str,U32& sfor_ln );
  char*     GetEngNameForODEfunc( SEMANTICS_NODE* s_func,
                                            int& error_code );
  void      KillIndVar( SEMANTICS_NODE* ODE_info );
  int       CountMatrixOperands( SEMANTICS_NODE* s_rpolish_op );

  void      GetShapeArgs( SEMANTICS_NODE* s_op2,
                            char** str_s1,char** str_s2,char** str_s3,
                            int& error_code );
  bool  IsTransposeIndicator( SEMANTICS_NODE* s_power,
                                            char& which_one );

  bool  QueryEngineIsMatrix( SEMANTICS_NODE* s_variable );
  bool  QueryEngineIsDefined( char* var_nom );

  bool  DoEvals( ContextType curr_context );
  void      SetVarTypes( SEMANTICS_NODE* root,int& nmatrices );
  char*     VarsStrFromSNODE( SEMANTICS_NODE* s_param, int& error_code );
  bool  QualifierIsSumVar( SEMANTICS_NODE* s_qvar );

  bool  CmdAllowed( MathServiceRequest* msr, MathResult* mr,
                            U32 cmd_ID, SEMANTICS_NODE* root );

  bool  VarsAreDefined( SEMANTICS_NODE* s_tree );

  char*     CreateVParameStr( MathServiceRequest& msr,
                                        U32 param_ID,
                                        U32 alt_ptype,
                                        int& error_code );

  U32         client_ID;
  U32         mangle_number;

  ClientAliases_REC*  client_list;

  char*       loaded_packages;

  //BEGIN CONTEXT STRUCT
  char*       new_packages;
  ContextType context_cmdtype;
  ContextTemplateType context_cmdtemplatetype;
  ContextDataType context_datatype;
  int         context_nmatrices;
  VAR_REC*    context_variables;
  //END CONTEXT STRUCT

  Grammar* id_dBase;
  Grammar* nom_dBase;

  MNODE*      out_tree;
  Maple2Tree* maple_parser;

  eAnalyzer*  engstr_analyzer;

  LOG_MSG_REC*   msg_list;
  MathServiceRequest* curr_msr;

  int ODE_num_procnum;
  char global_sum_var[80];

  bool matrix_form_required;
};

#endif

