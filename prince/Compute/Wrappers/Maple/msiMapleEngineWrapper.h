// Copyright (c) 2006 MacKichan Software, Inc.  All Rights Reserved.

#include "msiIEngineWrapper.h"
#include "MplWrap.h"

#define MSI_MAPLEENGINEWRAPPER_CID \
{ 0xce34a52e, 0x7afe, 0x4707, { 0xa4, 0x57, 0x32, 0x26, 0x6b, 0x4c, 0xb3, 0x01 } }

#define MSI_MAPLEENGINEWRAPPER_CONTRACTID "@mackichan.com/mapleenginewrapper;1"


class msiMapleEngineWrapper : public msiIEngineWrapper
{
public:
    msiMapleEngineWrapper();
    virtual ~msiMapleEngineWrapper();

    NS_DECL_ISUPPORTS

    NS_DECL_MSIIENGINEWRAPPER

private:
  MapleWrapper *Maple_wrapper;
};
