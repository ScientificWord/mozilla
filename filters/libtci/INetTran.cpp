
#ifdef TESTING
  #undef THIS_FILE
  static char THIS_FILE[] = __FILE__;
#endif

#include "chmtypes.h"
#include "tcistrin.h"

#ifdef INET_ON
#include <INetTran.h>

INetTranslator::INetTranslator() { 
}

INetTranslator::~INetTranslator() { 
}

void  INetTranslator::Init(
      INET_OPEN       Open,
      INET_CLOSE      Close
    )
{
  GlobalOpen    =  Open;
  GlobalClose   =  Close;
}

TCI_BOOL INetTranslator::Open(const FileSpec& urlSpec, TCICHAR* cachename) {

  return GlobalOpen( urlSpec, cachename );
}

TCI_BOOL INetTranslator::Close(const FileSpec& urlSpec)
{
  return GlobalClose( urlSpec );
}

#endif  // INET_ON

