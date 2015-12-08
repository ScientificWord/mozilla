#ifndef INETTRAN_H
#define INETTRAN_H

#ifndef CHMTYPES_H
  #include "chmtypes.h"
#endif

#ifndef FILESPEC_H
  #include "filespec.h"
#endif


class FileSpec;

typedef TCI_BOOL (*INET_OPEN)( const FileSpec& url, TCICHAR* cachename );
typedef TCI_BOOL (*INET_CLOSE)( const FileSpec& url );

class INetTranslator
{
  public:
    INetTranslator();
    ~INetTranslator(); 

    void      Init(
      INET_OPEN       Open,
      INET_CLOSE      Close
    );

    TCI_BOOL  Open(const FileSpec& url, TCICHAR* cachename);
    TCI_BOOL  Close(const FileSpec& url);

  private:
    INET_OPEN       GlobalOpen;
    INET_CLOSE      GlobalClose;
};

#endif //INETTRANSLATOR_H
