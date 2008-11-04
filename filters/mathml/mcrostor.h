
#ifndef MacroStore_h
#define MacroStore_h
																																																								
/*
  Class MacroStore  - A server for Grammar.
A LaTeX file can contain definitions of new commands - commands
that aren't defined in standard LaTeX.  These commands can then
be used within the source LaTeX script.  This object stores the
definitions of any new commands (ie. macros) encountered during
a filter run.  In the current filter, instances of macros are
handled by expanding the macro - substituting the definition -
at the time tokens are extracted from the input.
*/

#include "flttypes.h"


typedef struct tagMACRO_REC {
  tagMACRO_REC*   next;
  U8*             name;
  U8*             def;
  U16             param_count;
  U16             scope;
} MACRO_REC;

typedef struct tagMATHOP_REC {
  tagMATHOP_REC*  next;
  U8*             name;
  U8*             def;
  bool            is_starred;
  U16             scope;
} MATHOP_REC;


class MacroStore {

public:
  MacroStore();
  ~MacroStore();

  U8*     GetMacroDef( U8* macro,U16& n_params );
  U8*     GetMathOpDef( U8* mathop,bool& is_starred );

  U16     GetNewScope();
  void    ClearScope( U16 scope );

  void    AddMacro( U8* nom,U8* def,U16 p_count,U16 scope );
  void    AddMathOp( U8* nom,U8* def,bool is_starred,U16 scope );

private:

  MACRO_REC*  macros;
  MATHOP_REC* mathops;
  U16         scope_handle;
};

#endif

