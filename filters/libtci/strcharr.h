#ifndef STRCHARR_H
#define STRCHARR_H

#ifndef CHMTYPES_H
  #include "chmtypes.h"
#endif

// Since there is no `base' object in our development project,
// this class will take and return pointers to void (i.e. void*)
typedef void  *voidPtr;

class StretchyArray {

public:
  StretchyArray();
  StretchyArray( const StretchyArray& );
  ~StretchyArray();

  voidPtr&  operator[] ( U32 ix );
  voidPtr   Get( U32 ix ) const;
  void      operator=( const StretchyArray& );

  U32       MaxIndex() const { return highmark; }
  TCI_BOOL  SetSize(U32 ix) {return ix>highmark ? SetHighMark(ix) : TRUE;}

private:
  voidPtr&  Index( U32 ix );
  TCI_BOOL   SetHighMark( U32 n );

  void  **pointers;
  U32   highmark;
};



#endif // #ifndef STRCHARR_H
