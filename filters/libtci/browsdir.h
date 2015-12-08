#ifndef BROWSEDIR_H
#define BROWSEDIR_H


class CWnd; //Added for Rational Rose
class FileSpec;
class TCIString;


int BrowseDirectory(int dialogID, FileSpec& path,
                    bool setInitialDir,
                    const TCIString& dialogTitle,
                    CWnd * pParentWnd = 0);


#endif
