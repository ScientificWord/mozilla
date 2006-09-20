// Copyright (c) 2005, MacKichan Software, Inc.  All rights reserved.


#include "msiEditingAtoms.h"
#include "nsStaticAtom.h"
 
// define storage for all atoms
#define MSI_ATOM(_name, _value) nsIAtom* msiEditingAtoms::_name;
#include "msiEditingAtomList.h"
#undef MSI_ATOM


static PRBool msiEditingAtoms_initalized(PR_FALSE);
static const nsStaticAtom msiEditingAtoms_info[] = {
#define MSI_ATOM(name_, value_) { value_, &msiEditingAtoms::name_ },
#include "msiEditingAtomList.h"
#undef MSI_ATOM
};

void msiEditingAtoms::AddRefAtoms()
{
  if (!msiEditingAtoms_initalized)
  {
    NS_RegisterStaticAtoms(msiEditingAtoms_info, NS_ARRAY_LENGTH(msiEditingAtoms_info));
    msiEditingAtoms_initalized = PR_TRUE;
  }  
}

