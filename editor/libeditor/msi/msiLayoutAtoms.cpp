// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.


#include "msiLayoutAtoms.h"
#include "nsStaticAtom.h"
 
// define storage for all atoms
#define GK_ATOM(_name, _value) nsIAtom* msiLayoutAtoms::_name;
#include "nsGkAtomList.h"
#undef GK_ATOM


static PRBool msiLayoutAtoms_initalized(PR_FALSE);
static const nsStaticAtom msiLayoutAtoms_info[] = {
#define GK_ATOM(name_, value_) { value_, &msiLayoutAtoms::name_ },
#include "nsGkAtomList.h"
#undef GK_ATOM
};

void msiLayoutAtoms::AddRefAtoms()
{
  if (!msiLayoutAtoms_initalized)
  {
    NS_RegisterStaticAtoms(msiLayoutAtoms_info, NS_ARRAY_LENGTH(msiLayoutAtoms_info));
    msiLayoutAtoms_initalized = PR_TRUE;
  }  
}

