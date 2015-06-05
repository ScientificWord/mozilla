// Copyright (c) 2006 MacKichan Software, Inc.  All Rights Reserved.

#include "msiISimpleComputeEngine.h"



#define MSI_SIMPLECOMPUTEENGINE2_CID \
{ 0xa3f85155, 0x6cb7, 0x46aa, { 0x9f, 0xa4, 0x35, 0x10, 0x8f, 0x69, 0x61, 0x31 } }

#define MSI_SIMPLECOMPUTEENGINE2_CONTRACTID "@mackichan.com/simplecomputeengine;2"


class msiSimpleComputeEngine2 : public msiISimpleComputeEngine
{
public:
    msiSimpleComputeEngine2();
    virtual ~msiSimpleComputeEngine2();
    static msiSimpleComputeEngine2* GetInstance();
    // nsISupports interface
    NS_DECL_ISUPPORTS

    NS_DECL_MSIISIMPLECOMPUTEENGINE


private:
    NS_IMETHOD BasicCommand(const PRUnichar *expr, PRUnichar **result, int cmdCode);
    NS_IMETHOD CommandWithArgs(const PRUnichar *expr, PRUnichar **result, int cmdCode, ...);
    nsresult   DoTransaction(PRUint32 trans_ID, PRUnichar **result);
    PRUnichar *GetResultStrs(PRUint32 trans_ID);

    bool       didInit;
    PRUint32   MuPAD_eng_ID;
    PRUint32   client_handle;
    
    PRUnichar *sent_to_engine;
    PRUnichar *received_from_engine;
    PRUnichar *engine_errors;
};
