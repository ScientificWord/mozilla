// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.
#include "nsIGenericFactory.h"

#include "msiEditingManagerCID.h"
#include "msiEditingManager.h"

////////////////////////////////////////////////////////////////////////
// Define the contructor function for the objects
//
// NOTE: This creates an instance of objects by using the default constructor
//
NS_GENERIC_FACTORY_CONSTRUCTOR(msiEditingManager)

////////////////////////////////////////////////////////////////////////
// Define a table of CIDs implemented by this module along with other
// information like the function to create an instance, contractid, and
// class name.
//
static const nsModuleComponentInfo components[] = {
  { "msiEditing", MSI_EDITINGMANAGER_CID, MSI_EDITING_MANAGER_CONTRACTID, msiEditingManagerConstructor },
};

////////////////////////////////////////////////////////////////////////
// Implement the NSGetModule() exported function for your module
// and the entire implementation of the module object.
//
NS_IMPL_NSGETMODULE(msieditingModule, components)
