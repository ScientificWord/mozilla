// Copyright (c) 2006 MacKichan Software, Inc.  All Rights Reserved.

#include "msiIEngineWrapper.h"
#include "MuWrap.h"

#define MSI_MUPADENGINEWRAPPER_CID \
{ 0xa3f85155, 0x6cb7, 0x46aa, { 0x9f, 0xa4, 0x35, 0x10, 0x8f, 0x69, 0x61, 0x30 } }

#define MSI_MUPADENGINEWRAPPER_CONTRACTID "@mackichan.com/mupadenginewrapper;1"


class msiMuPADEngineWrapper : public msiIEngineWrapper
{
public:
    msiMuPADEngineWrapper();
    virtual ~msiMuPADEngineWrapper();

    NS_DECL_ISUPPORTS

    NS_DECL_MSIIENGINEWRAPPER

private:
  MuPADWrapper *MuPAD_wrapper;
};
