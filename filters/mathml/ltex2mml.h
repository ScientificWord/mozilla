
#ifndef LaTeX2MMLTree_h
#define LaTeX2MMLTree_h
																																																								
/*
  Class LaTeX2MMLTree  - Convert an Notebook LaTeX parse tree
   into it's equivalent for MathML.
*/

#include "flttypes.h"
#include "fltutils.h"


class Grammar;
//class MMLAttrMan;
class LogFiler;
class TreeGenerator;

typedef struct tagGROUP_INFO {	// Struct to define an attribute node.
  U8        opening_delim[32];
  U8        closing_delim[32];
  TCI_BOOL  has_mtext;
  TCI_BOOL  is_mod;
  U16       lowest_precedence;
  U16       operator_count;
  U16       separator_count;
  U16       n_interior_nodes;
  U16       n_arrays;
} GROUP_INFO;

typedef struct tagIDENTIFIER_NODE {	// Struct
  tagIDENTIFIER_NODE* next;
  U8* zidentifier;
} IDENTIFIER_NODE;


typedef struct tagMATH_CONTEXT_INFO {	// Struct
  U16 row_number;
  U16 column_number;
} MATH_CONTEXT_INFO;


typedef struct tagOP_GRAMMAR_INFO {	// Struct
  U16 form;
  U16 precedence;
  U8* unicode;
  ATTRIB_REC* attr_list;
} OP_GRAMMAR_INFO;


class LaTeX2MMLTree {

public:
  LaTeX2MMLTree( TreeGenerator* parent,
                    Grammar* src_mml_grammar,
  				 	          Grammar* dest_mml_grammar,
                        LogFiler* logger,
							            USERPREFS* preferences,
							              const char** context_zstrs );
  ~LaTeX2MMLTree();

  TNODE*  NBLaTeXTreeToMML( TNODE* src_tree,
		  				              MATH_CONTEXT& mc,
  								          ANOMALY_REC* anomalies,
                                   U16& result );
  void	  AddNumbering( TNODE* mrow,
  		  				            TNODE* xml,
		  					            TCI_BOOL is_eqnarray );

private :

  TNODE* TranslateMathObject( TNODE* TeX_obj_node );

  TNODE* TranslateTeXDisplay( TNODE* tex_display_node,
          							U16 id,TNODE** out_of_flow_list );
  TNODE* TranslateTeXDollarMath( TNODE* tex_dollar_node,
          							        TNODE** out_of_flow_list );
  TNODE* TranslateTeXEqnArray( TNODE* tex_eqnarray_node,
									              TNODE** out_of_flow_list,
		   								          U16 usubtype );
  TNODE* LaTeXCases2MML( TNODE* src_cases,
									          TNODE** out_of_flow_list );
  TNODE* NestedTeXEqnArray2MML( TNODE* tex_eqnarray_node,
									              TNODE** out_of_flow_list,
		   								          U16 usubtype );

  TNODE* TranslateMathList( TNODE* LaTeX_parse_list,
		 					  	          TCI_BOOL do_bindings,
 									          MATH_CONTEXT_INFO* m_context,
                           	U16& tex_nodes_done,
                           	U16& error_code,
                            TNODE** out_of_flow_list );

  void    UnicodesToSymbols( TNODE* LaTeX_list );
  TNODE*  MathSymbolToMML( TNODE* rover,
                            TNODE* LaTeX_list,
                              TNODE** ppVS_list,
                                U16& tex_nodes_done );
  TNODE*  BoldSymbolToMML( TNODE* tex_BF_node,
		  						U16& local_advance );
  TNODE*  MathAccToMML( TNODE* tex_acc_node,U16 uID,
  							TNODE** out_of_flow_list );

  TNODE*  MathStructureToMML( TNODE* obj_node,
								TNODE** ppVS_list,
								MATH_CONTEXT_INFO* m_context,
                                U16& tex_nodes_done,
                                    U16& error_code );
  TNODE*    Decoration2MML( TNODE* obj_node,
                               TNODE** ppVS_list,
                               U16 TeX_subclass,
                               U16 TeX_uID );
  TNODE*    XArrow2MML( TNODE* obj_node,
										           TNODE** out_of_flow_list,
										           U16 tex_uID );
  TNODE*    OverOrUnder2MML( TNODE* obj_node,
				   			U16 TeX_subclass,U16 tex_uID,
			   			    TNODE** out_of_flow_list );
  TNODE*    Fraction2MML( TNODE* obj_node,U16 subclass,
                            TNODE** ppVS_list );
  TNODE*    Array2MML( TNODE* tex_array_node,
                            TNODE** out_of_flow_list );
  TNODE*    Matrix2MML( TNODE* tex_array_node,
                            TNODE** out_of_flow_list,
                            U16 subtype );
  TNODE*    GenFrac2MML( TNODE* obj_node,U16 subclass,TNODE** ppVS_list );
  TNODE*    Script2PsuedoMML( TNODE* tex_script_node,
                            MATH_CONTEXT_INFO* m_context,
                            TNODE** out_of_flow_list );
  TNODE*    ScriptStack2MML( TNODE* tex_stacked_script_node,
                            U16 subclass,TNODE** out_of_flow_list );
  TNODE*    SbAndSp2MML( TNODE* SborSp_node,
                            TNODE** out_of_flow_list );
  TNODE*    Radical2MML( TNODE* tex_radical_node,
                            U16 subclass,TNODE** out_of_flow_list );

  TNODE*    Tabular2MML( TNODE* tex_table_node,
                            TNODE** ppVS_list,U16 usub );
  TNODE*    LaTeXFence2MML( TNODE* obj_node,TNODE** out_of_flow_list );
  TNODE*    BiglmrToMML( TNODE* tex_fence_node,
                            U16 subclass,U16 id );

  TNODE*    BigOp2MML( TNODE* tex_bigop_node,
                            U16& tex_nodes_done,
                            TNODE** out_of_flow_list );
  TNODE*    LimFunc2MML( TNODE* tex_bigop_node,
                            U8* func_name,
                            U16 usubtype,U16 uID,
                            TNODE** out_of_flow_list );
  TNODE*    Function2MML( TNODE* src_tex_func,
                            U16 usubtype,U16 uID,
                            U16& tex_nodes_done,
                            TNODE** out_of_flow_list );

  TNODE*    TextInMath2MML( TNODE* text_node,
								TNODE** ppVS_list,
  								TCI_BOOL no_line_breaks,
								TCI_BOOL is_unit );
  TNODE*    UnitInMath2MML( TNODE* unit_contents,
                            TNODE** ppVS_list,
                            TCI_BOOL nested,
                            U16& tex_nodes_done );
  TNODE*    CoelesceFollowingUnits( TNODE* TeX_unit_node,
                            TNODE** ppVS_list,
                            TNODE* MML_rv,
                            U16& tex_nodes_done );
  TNODE*    TaggedText2MML( TNODE* obj_node,
  								TNODE** ppVS_list,
								 U8* tag_nom,U16 subclass );
  TNODE*    TaggedMath2MML( TNODE* obj_node,
							  TNODE* TeX_cont,
							    U16 tag_ilk,U8* tag_nom,
								  TNODE** out_of_flow_list );

  TNODE*  LaTeXHSpacing2MML( TNODE* rover,U16 usubtype,U16 uID,
				 			                U16& nodes_done );
  TNODE*  MathHSpacesToMML( TNODE* tex_math_space,
				 			                U16& nodes_done );

  TNODE*  BindDelimitedGroups( TNODE* MML_list );

  TNODE*  BindScripts( TNODE* MML_list );

  TNODE*  BindDelimitedIntegrals( TNODE* MML_list );
  TNODE*  BindIntegral( TNODE* list_head,
		  				TNODE* MML_integral_node,
		  				  TNODE** pnew_mrow );
  TNODE*  BindUnits( TNODE* MML_list );
  TNODE*  BindMixedNumbers( TNODE* MML_list );
  TNODE*  BindDegMinSec( TNODE* MML_list );
  TNODE*  BindByOpPrecedence( TNODE* MML_list,
  									U16 p_max,U16 p_min );


  TNODE*      AddOperatorInfo( TNODE* MML_parse_list );
  void        SetMultiformOpAttrs( TNODE* MML_rover,TNODE* MML_op_node );
  void        SetTeXOperatorSpacing( TNODE* MML_rover,
										    TNODE* op_node,
										    TCI_BOOL in_script );
  I16         GetTeXSpacing( U16 left_ilk,U16 op_ilk );
  TNODE*      AbsorbMSpaces( TNODE* MML_parse_list );

  TNODE*      CreateMError( const char* err_text,U32 src_linenum );
  TNODE*      CreateElemWithBucketAndContents( U16 uobjtype,
			  							U16 usubtype,U16 uID,
			  						U16 bucketID,TNODE* contents );
  TNODE*      CreateBucketWithContents(
							U16 uobjtype,U16 usubtype,U16 bucketID,
								 TNODE* contents );

  void        GetNumberStr( TNODE* tex_symbol_node,
							  TNODE** ppVS_list,
  								TNODE* mn_node,
  								  U16& advance );
  TCI_BOOL    NumberContinues( TNODE* rover,
			  							TCI_BOOL got_decimal );
  TNODE*      GetIdentifier( TNODE* tex_symbol_node,
 								    TCI_BOOL forces_geometry,U16& advance );

  U16         ClassifyTeXSymbol( TNODE* tex_symbol_node,
                                TNODE* LaTeX_list,
                                TCI_BOOL& forces_geometry,
                                TNODE* mml_rv );

  void        SetMMLOpNode( TNODE* tex_symbol_node,
			  					U16& tex_nodes_done,
									  TNODE* mml_rv );

  TCI_BOOL    IsASCIImmlOp( TNODE* tex_symbol_node,U8** mml_uID );
  TCI_BOOL    IsMultiTeXSymOp( TNODE* tex_sym_node,
			  				    U8** zop_info,U16& advance );
  TCI_BOOL    GetLaTeXOpSymbol( TNODE* tex_op_node,
			  				  	U8* zdest,U16& advance );

  void        CheckForOperands( TNODE* MML_op_node,
                                TCI_BOOL& has_left_operand,
                                TCI_BOOL& has_right_operand,
                                I16& left_space,
                                I16& right_space );
  U16         GetCompoundOpForm( TNODE* tex_sym_node,U8 c1,U8 c2 );


  void        SetChData( TNODE* node,U8* ent_val,U8* uni_val );
  U8*         GetFuncName( TNODE* char_list );

  TNODE*      MMLlistToMRow( TNODE* list );
  U16         SetMMLBigOpEntity( TNODE* tex_bigop_node,
						  		              TNODE* base_op );
  TCI_BOOL    TeXFuncHasLimits( U16 func_uID );

  void        CheckFenceBody( U8* left_uID,U8* right_uID,
								TNODE* contents,
                              	  TCI_BOOL& enclosed_list );

  void      TeXDelimToMMLOp( U8* TeX_fence_uID,
                                U16 left_center_right,
                                U8* mml_fence_nom,
                                U8* unicode_nom,
                                U8* mml_fence_attrs );

//  TNODE*	  AtomicElemToNBTeX( TNODE* src_node,U16& advance,U16& result );

  I16         GetDetailNum( TNODE* node,U16 field_ID );
  void        SetDetailNum( TNODE* node,U16 field_ID,I16 num );
  void        SetDetailzStr( TNODE* node,U16 field_ID,U8* zstr );

  void        SetNodeAttrib( TNODE* mo_node,U8* attr_nom,
  											U8* attr_val );

  void        InsertAF( TNODE* mml_rover,TCI_BOOL arg_is_delimited );
  void        InsertApplyFunction( TNODE* MML_list );
  void        InsertInvisibleTimes( TNODE* MML_list );
  TCI_BOOL    OperandExists( TNODE* base_node,
                                TCI_BOOL left,U16 curr_prec,
                                U16& n_space_nodes,U16& nodes_spanned,
                                I16& space_width );
  TCI_BOOL    LocateOperand( TNODE* base_node,TCI_BOOL left,
  						U16& n_space_nodes,U16& nodes_spanned,
  							  U16 form,U16 precedence );
  TNODE*      NestNodesInMrow( TNODE* rv,
                               	TNODE* anchor,TNODE* first,
			  			    	TCI_BOOL on_the_right,
			  				    U16 nodes_spanned,
			  				    TNODE** p_new_mrow );

  TCI_BOOL    MMLNodeIsFunction( TNODE* mml_rover );
  TCI_BOOL    FunctionHasArg( TNODE* mml_list,
                                U16& n_space_nodes,
                                U16& n_arg_nodes,
                                TCI_BOOL& is_delimited );
  TCI_BOOL    LocateMatchingDelim( TNODE* mml_fence_mo,
                                    U8* fence_op_nom,
                                    U16 biglmr_size,
                                    U16& nodes_spanned );
  void        GetPrecedenceBounds( TNODE* MML_list,
			  						U16& lowest,U16& highest );
  void        SetAttrsForFenceMO( TNODE* mml_op_node,
                        U16 subtype,TCI_BOOL is_left );
  TCI_BOOL    IsMMLFactor( TNODE* mml_node,
                                TCI_BOOL include_differential,
									              TCI_BOOL& is_capitol_letter );
  TCI_BOOL    IsMMLFactorFromInternals( TNODE* mml_node,
                                TCI_BOOL include_differential,
									              TCI_BOOL& is_capitol_letter );
  TCI_BOOL    IsMMLNumber( TNODE* mml_node );
  TCI_BOOL    IsVector( TNODE* mml_node );
  TCI_BOOL    IsQualifier( TNODE* mml_node );
  TCI_BOOL    MMLDelimitersMatch( GROUP_INFO& gi );

  TCI_BOOL    IsDifferentiald( TNODE* tex_sym_node,
                                    TNODE* LaTeX_list );
  TCI_BOOL    IsDifferentialD( TNODE* tex_sym_node,
                                    TNODE* LaTeX_list );
  TCI_BOOL    TeXListHasIntegral( TNODE* LaTeX_list,
                                    TNODE* tex_d_node );
  TCI_BOOL    IsPowerForm( TNODE* tex_node );

  TNODE*      FinishMMLBindings( TNODE* mml_list );
  void        GetMatchingFence( U8* delim,
  			  	U8* matching_delim,U8* secondary_match );
  U16         GetOPFormFlags( TNODE* op_node );
  void        GetGroupInfo( TNODE* MML_list,
									GROUP_INFO& gi );
  void        GetGroupInsideInfo( TNODE* MML_cont,
									GROUP_INFO& gi );
  TCI_BOOL    IsFenceMO( TNODE* mml_node );

  TCI_BOOL    TeXNodeDenotesFunction( TNODE* mml_node );
  TCI_BOOL    DoNestFenceBody( TNODE* fixed_interior );

  TCI_BOOL    LeftGroupHasSamePrec( TNODE* MML_rover,
							U16 curr_prec,U16 l_space_nodes );
  TNODE*      ElevateLeftOperand( TNODE* rv,TNODE* MML_rover,
											U16 l_space_nodes );
  TCI_BOOL    LookBackForIntegral( TNODE* tex_sym_node );

  void        TranscribeAttribs( TNODE* xml_line,
								    TNODE* mml_line,
								      U16 entry_num );

  void      GetXMLAttrValues( ATTRIB_REC* attrib_list,
            					U8** zcounter,
            					U8** zordinal,
            					U8** zlabel,
            					U8** zis_tagged,
            					U8** zno_number );
  TCI_BOOL  LocateAttribVal( ATTRIB_REC* attrib_list,
								U8* attr_name,U8** zvalue );
  TNODE*    ScriptedUnit2MML( TNODE* script_node,
									TCI_BOOL& is_degree );
  TNODE*    PrimeAsScript2MML( U16 n_primes );

  TNODE*    MakeMMLScript( TNODE* list_head,
                            TNODE* first,U16 precedence,
								TNODE** p_new_mscript,
							      TCI_BOOL missing_left );
  TNODE*    MakeMMLSubSup( TNODE* list_head,
                            TNODE* script_op_node,
								TNODE** p_new_mscript,
								    TCI_BOOL missing_left );
  void      SetDetailIsExpr( TNODE* mrow,
									              I16 precedence );
  U16       IsGroupAFuncArg( TNODE* mrow_group );

  TCI_BOOL  TeXScriptIsUnit( TNODE* psuedo_script_node );
  void      SetDetailsForBoundPostfix( TNODE* mrow,
											          I16 curr_prec );
  TCI_BOOL  LocateScriptBase( TNODE* psuedo_script,
									U16& nodes_spanned );

  U32       ExtractUNICODE( TNODE* tex_uni_node );
  TNODE*    Unicode2MML( TNODE* obj_node );
  TNODE*    CmdSymbol2MML( TNODE* obj_node );

  TCI_BOOL  IsTeXVariable( TNODE* TeX_node );
  U8        GetHexDigit( TNODE* TeX_node );
  TNODE*    MakeSmallmspace();

  TCI_BOOL  MMLNodeCouldStartGroup( TNODE* mml_node,
                                    U8* nom,U16& biglmr_size );
  TNODE*    AbsorbMMLSpaceNode( TNODE* head,TNODE* op_node,
								  TCI_BOOL space_on_right );

  TCI_BOOL  MMLNodeCouldBeFunction( TNODE* mml_node,
								U8* identifier_nom,U16 lim );
  IDENTIFIER_NODE* AddIdentifier( IDENTIFIER_NODE* i_list,
								  	U8* identifier_nom );
  TCI_BOOL  IdentifierInList( IDENTIFIER_NODE* i_list,
  									U8* identifier_zstr );
  void      DisposeIdentifierList( IDENTIFIER_NODE* i_list );

  TCI_BOOL  ScriptIsCirc( TNODE* TeX_contents );
  TCI_BOOL  BaseIsDiffD( TNODE* tex_script_node );
  TNODE*    TranslateDDsubscript( TNODE* TeX_contents,
									TNODE** out_of_flow_list );
  TNODE*    TranslateEqnLine( TNODE** TeX_contents );

  TCI_BOOL  IsMsup( TNODE* MML_node );

  void      SetMTableAttribs( TNODE* mtable,TCI_BOOL is_tabular,
                                TNODE* tex_matrix_node );
  void      AppendToFnom( U8* zdest,U16 limit,U8* zstr );
  void      ExtractFuncNameFromMrow( U8* zdest,U16 limit,
	                               	TNODE* mml_cont_list );
  void      GetScriptedFuncName( TNODE* mml_node,U16 stype,
                            U8* identifier_nom,U16 limit );

  TCI_BOOL  FuncTakesTrigArgs( TNODE* mml_func_node );
  U16       GetQTRuSubID( U8* run_name,TCI_BOOL in_math );
  U8*       QTRuSubIDtoName( U16 sub_ID );
  void      GetUnicodeForBoldSymbol( TNODE* mml_rover,
                            U8* unicode );
  U16       GetMathRunTagFromTeXSwitch( TNODE* TeX_cont,
                            U8* switch_nom );
  TCI_BOOL  IsImpliedEllipsis( TNODE* TeX_period_node );

  TNODE*    SubscriptBody2MML( TNODE* cont,
                            MATH_CONTEXT_INFO* m_context );

  TCI_BOOL  FuncIsMMLOperator( U16 subtype,U16 uID );
  TNODE*    BindPrefixOP( TNODE* head,TNODE* pre_op_node,
                            U16 curr_prec,TNODE** new_mrow );

  TCI_BOOL  IsTeXThinOrThickSpace( TNODE* MML_node );
  TCI_BOOL  MTextIsWhiteSpace( TNODE* mtext );
  TCI_BOOL  MTextIsWord( TNODE* mtext );

  TCI_BOOL  IsMultiFormOp( U8* op_name,
                            U16& forms );
  TNODE*    FBox2MML( TNODE* tex_fbox_node,
                            TNODE** ppVS_list,
                            U16 subclass );
  TCI_BOOL  IsFuncDerivative( TNODE* mml_node );

  U8*       GetMtableRowLines( TNODE* TeX_tabularORarray_node,
                                TCI_BOOL is_tabular,
                                U16& top_lines,
                                U16& bottom_lines );
  void      GetMtableColumnAttrs( TNODE* TeX_cols_list,
                                U8** columnlines,
                                U8** c_align_attrs,
                                U16& left_lines,
                                U16& right_lines );
  TNODE*    MBox2MML( TNODE* tex_mbox_node,
                                TNODE** ppVS_list,U16 uID );
  TNODE*    MoveNodeToList( TNODE* VSpace_list,TNODE* v_node );
  TNODE*    VSpace2MML( TNODE* MML_rv,TNODE* VSpace_list,
                          U16 vspace_context );
  TNODE*    Labels2MML( TNODE* MML_rv,TNODE* label_list );
  void      RecordAnomaly( U16 ilk,U8* ztext,
                                U32 off1,U32 off2 );

  TCI_BOOL  IsIDotsInt( TNODE* tex_bigop_node );
  TNODE*    IDotsInt2MML();

  void      HyperObj2MML( TNODE** ppVS_list,TNODE* TeX_hyper_obj,
                                U16 subclass,TCI_BOOL in_math,
                                TNODE** node_to_insert );

  TCI_BOOL  MMLNodeIsOperator( TNODE* MML_node );
  U16       GetPrecedence( TNODE* mml_node,U16 op_form );

  void      GetAttribsFromGammarInfo( U8* zinfo,
								    OP_GRAMMAR_INFO& op_info );
  ATTRIB_REC* ExtractAttrs( char* zattrs );
  TCI_BOOL  IsDeprecatedOp( TNODE* tex_sym_node,
							        U8** name,U8** production );

  void		  MergeMOAttribs( TNODE* op_node,
								    ATTRIB_REC* attr_list );

  void      StyleIDtoStr( U16 ID,U8* style_nom );
  TNODE*    Rule2MML( TNODE* obj_node,TNODE** ppVS_list );
  TCI_BOOL  RuleIsPureVSpace( TNODE* TeX_rule_node );
  void      GetVSpaceFromRule( TNODE* TeX_rule_node,
  								    U8* depth,U8* height );
  void      GetValueFromGlue( TNODE* glue_bucket,
                                U8* val,TCI_BOOL is_relative,
                                I16& ems_val );

  U16       TeXUnitToID( char* z_dimen );
  double    ConvertTeXDimen( double s,U16 stype,U16 dtype );
  void      AttachUnit( char* buffer,U16 unit_type );
  U16       GetMMLdimenType( U16 TeX_dimen_type );

  TNODE*    DisposeOutOfFlowList( TNODE* out_of_flow_list,
									U16 id1,U16 id2 );
  TNODE*    Struts2MML( TNODE* MML_rv,TNODE* out_of_flow_list );
  TNODE*    HandleOutOfFlowObjects( TNODE* MML_list,
									TNODE** local_oof_list,
									TNODE** out_of_flow_list,
									U16 vspace_context );
  TNODE*    Multicolumn2MML( TNODE* tex_node,
								    TNODE** out_of_flow_list,
									TCI_BOOL in_math,
									U16& mcol_align,
									U16& cols_spanned );
  TNODE*    Hdotsfor2MML( TNODE* tex_node,
							        TCI_BOOL in_math,
									U16& cols_spanned );
  TCI_BOOL  BucketContainsMath( TNODE* display_cont );

  TNODE*    FORMULA2MML( TNODE* TeX_FORMULA_node,
					                TNODE** out_of_flow_list );


  TNODE*    MathComment2MML( TNODE* TeX_comment_node,
									U16 &advance );
  void      ProcessTextComment( TNODE* TeX_comment_node,
								    U32& unicode,
								    U16& advance );
  TNODE*    GetTCIMacroParts( U8* zTCIMacro,
								    TNODE* comment_node,
									TNODE** MML_expansion,
									U16& advance );
  U8*       TCIMacroToBuffer( TNODE* comment_node );
  TNODE*    LocateEndExpansion( TNODE* start_node,
	                                  U16& advance );
  TCI_BOOL  LocateBraces( U8* TCIData,
                            U16& s_off,U16& e_off );

  void      SetMTRAttribs( TNODE* mtr_node,
                    U16 alignment,U16 ncols_spanned );
  void      GetEQNAlignVals( U16 ilk,U8* align_vals,U16 lim );

  TNODE*    AddEQNAttribs( TNODE* mml_eqn_node,
                                        U16 usubtype );
  TNODE*    SetBigOpSize( TNODE* base_op,
                                U8 tex_size_attr );

  TCI_BOOL  ContainsAllCaps( TNODE* TeX_decoration_node );
  TNODE*    BindGeometryObjs( TNODE* MML_list );
  TCI_BOOL  SpanGeometricObj( TNODE* MML_rover,
                                U16& node_to_nest );
  TCI_BOOL  IsPrimedPoint( TNODE* msup );
  TCI_BOOL  IsSubscriptedPoint( TNODE* msub );

  U16       ClassDeltaFromContext( TNODE* tex_sym_node );

  TCI_BOOL  GetVSpaceFromOOFList( TNODE* oof_list,double& extra_depth );

  TNODE*    DecorateVarLim( TNODE* mo,U16 uID );
  TNODE*    MarkingCmd2MML( TNODE* obj_node );
  TNODE*    EOLSpace2MML( TNODE* mml_rv,
							        TNODE* TeX_EOL );
  void      GetEOLSpace( TNODE* TeX_EOL,U8* depth );

  TNODE*    AddNumberToEquation( TNODE* mml_equation );
  U16       EqnHasNumberedLines( U16 usub );
  void      SetTableLineTaggingAttrs( TNODE* mml_rv );

  void      AppendEntityToBuffer( U8* zuID,U8* entity_buffer,U8* unicode_buffer );
  TCI_BOOL  GetUnicodeEntity( U8* d_template,
                                U8* entity_it );
  void      SetBiglSizingAttrs( TNODE* mml_delim_op,
                                U16 biglmr_size );
  void      SetMMLAttribs( TNODE* mml_node,U8* attr_str );
  void      SetMOSpacing( TNODE* mo,TCI_BOOL is_left,U8* zval );
  void      SetMOSpacingFromID( TNODE* op_node,I16 TeX_space_ID,
                                TCI_BOOL is_left,TCI_BOOL in_script );
  ATTRIB_REC* LocateAttribRec( TNODE* op_node,U8* targ_attr_nom );
  TCI_BOOL  GetScriptStatus();

  void      FormatCurrEqnNumber( U8* ztag );
  TNODE*    FixImpliedMRow( TNODE* mml_node );

  void      CreateUnicodeEntity( U8* buffer,U16 offset,U32 unicode );
  TCI_BOOL  InsideOfList( TNODE* node );

  TCI_BOOL  IsMMLMultOp( TNODE* mml_node );
  TCI_BOOL  NonSpacedUnit( TNODE* MML_unit_mi );
  TCI_BOOL  IsKnownOperator( U8* limfunc_name );
  TCI_BOOL  IsDegreeMinSec( TNODE* mml_rover );
  U16       GetDegMinSec( TNODE* MML_node );

  TreeGenerator*    lparser;
  Grammar*    s_mml_grammar;
  Grammar*    d_mml_grammar;
  LogFiler*	  logfiler;
  TCI_BOOL	  in_display;
  U16         script_level;
  USERPREFS*  p_userprefs;
  TCI_BOOL	  do_trig_args;

  TCI_BOOL	  renderer_implements_displays;
  TCI_BOOL	  renderer_implements_baselining;
  TCI_BOOL	  renderer_implements_mlabeledtr;
  TCI_BOOL	  do_equation_numbers;
  U16	      output_entities_as_unicodes;
  TCI_BOOL	  stretch_for_long_arrows;
  U16         mo_spacing_mode;
  U16         spacing_mode_in_scripts;

  char* zMMLStyleAttrs;         // mathcolor="red" etc.
  char* zMMLFunctionAttrs;      // mathcolor="gray"
  char* zMMLUnitAttrs;          // mathcolor="green"
  char* zMMLTextAttrs;          // mathcolor="black"
  char* zMMLHyperLinkAttrs;     // mathcolor="green"

  IDENTIFIER_NODE* funcs_list;
  IDENTIFIER_NODE* vars_list;

  //MMLAttrMan* attrman;

  ANOMALY_REC* p_anomalies;

  U16 math_field_ID;
  U16 mml_version;

  I16 theequation;
  I16 thesection;
  I16 thechapter;

  U8 entity_ic[32];
  U8 entity_it[32];
  U8 entity_ic_unicode[32];
  U8 entity_it_unicode[32];
};

#endif

