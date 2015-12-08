
// Unit test for component FileSys

#include "filesys.h"
#include "filespec.h"
#include "unittest.h"

static TCICHAR  *cstr  = "filename" ;
static TCICHAR  *CSTR  = "FILENAME" ;

void * GetInstanceHandle() { return NULL; }

void main()
{
   UnitTest  ut("FileSys");
   // Construct on the stack
   FileSpec fsDefault;       
   FileSpec fsExplicit(cstr);
   FileSpec fsDrive("c:"), fsDriveFull("c:\\"), fsTempDir("c:\\temp");
         
   // Test 1. IsDirectory
   ut.test("1.1", FileSys::IsDirectory(fsDefault) == FALSE);
   ut.test("1.2", FileSys::IsDirectory(fsDriveFull) == TRUE);
   ut.test("1.3", FileSys::IsDirectory(fsTempDir) == TRUE);

   // Test 2. CreateDir
   FileSpec fsRDir("r:\\temp\\testdir");
   ut.test("2.1", FileSys::CreateDir(fsDefault) == FALSE);
   // assume drive r: doesn't exist (bug 4640)
   ut.test("2.2", FileSys::CreateDir(fsRDir) == FALSE);
   ut.test("2.3", FileSys::CreateDir(fsDriveFull) == TRUE);
   FileSpec fsRDrive = fsRDir.GetDrive();
   ut.test("2.4", FileSys::CreateDir(fsRDrive) == FALSE);

   //Test 3. GetHTTPFileString
   TCIString httpParentStr("../"), httpChildStr("temp/"), httpExplicitTempStr("file:///c:/temp/");
   FileSpec fsManawaGraphic("\\\\MANAWA\\swp-proTL\\Extras\\Las Cruces\\ORGAN MOUNTAINS.JPG");
   FileSpec fsManawaOtherPath("\\\\MANAWA\\swp-proTL\\swp35\\Common");
   TCIString manawaStr("file://MANAWA/swp-proTL/Extras/Las Cruces/ORGAN MOUNTAINS.JPG");
   TCIString manawaRelativeStr("../../Extras/Las Cruces/ORGAN MOUNTAINS.JPG");
   ut.test("3.1", FileSys::GetURIFileString(fsDriveFull, fsTempDir) == httpParentStr);
   ut.test("3.2", FileSys::GetURIFileString(fsTempDir, fsDriveFull) == httpChildStr);
   ut.test("3.3", FileSys::GetURIFileString(fsTempDir, fsDefault) == httpExplicitTempStr);
   ut.test("3.4", FileSys::GetURIFileString(fsManawaGraphic, fsTempDir) == manawaStr);
   ut.test("3.5", FileSys::GetURIFileString(fsManawaGraphic, fsManawaOtherPath) == manawaRelativeStr);
}