// Copyright (c) 2005, MacKichan Software, Inc.  All rights reserved.

#ifndef msiEditingAtoms_h___
#define msiEditingAtoms_h___

#include "nsIAtom.h"

class msiEditingAtoms 
{
public:

  static void AddRefAtoms();

  /* Declare all atoms

     The atom names and values are stored in nsGkAtomList.h and
     are brought to you by the magic of C preprocessing

     Add new atoms to msiEditingAtomList.h and all support logic will be auto-generated
   */
#define MSI_ATOM(_name, _value) static nsIAtom* _name;
#include "msiEditingAtomList.h"
#undef MSI_ATOM
};  

#endif /* msiEditingAtoms_h___ */
