
// Unit test for component FileSpec

#include "filespec.h"
#include "unittest.h"

TCI_BOOL bSuppressAssertions = FALSE;

static TCICHAR  *cstr  = "filename" ;
static TCICHAR  *CSTR  = "FILENAME" ;

void main()
{
   UnitTest  ut("FileSpec");
   // Construct on the stack
   FileSpec fsDefault;                      // Default constructor
   FileSpec fsExplicit(cstr);              // TCIString& constructor
   FileSpec fsCopy(fsExplicit);           // Copy constructor

   // Construct on the heap and delete
   FileSpec* pFsHeap = new FileSpec(cstr);
   delete pFsHeap;
   
   FileSpec temp, temp2;
         
   // Test 1. IsValid
   ut.test("1.1", fsDefault.IsValid() == FALSE);
   ut.test("1.2", fsExplicit.IsValid() == TRUE);

   // Test 2. IsEmpty
   ut.test("2.1", fsDefault.IsEmpty() == TRUE);
   ut.test("2.2", fsExplicit.IsEmpty() == FALSE);
   
   // Test 3. Empty, GetName, GetPath
   
   // Test 4. Comparisons
   FileSpec fsExt(".ext"), fsExt2(".ex2"), fsExtTeX(".tex");
   FileSpec fsWithExt("filename.ext"), fsWithExt2("filename.tex"), fsWithExt3("filenam2.ext");
   
   ut.test("4.1", fsExplicit == fsCopy);
   // you need an explicit cast on the string, otherwise the FileSpec::operator const TCICHAR * cast is used.
   ut.test("4.2", fsExplicit == FileSpec(cstr));
   ut.test("4.3", FileSpec(cstr) == fsExplicit);

   temp = CSTR;  // case insensistivity
   ut.test("4.4", fsExplicit == temp);
   ut.test("4.5", temp == fsExplicit);

   temp = "junk";  // inequality
   ut.test("4.6", fsExplicit != temp);
   ut.test("4.7", temp != fsExplicit);

   temp = fsExt;   // extensions
   ut.test("4.8",     fsExt  == temp);
   ut.test("4.9",     fsExt2 != temp);
   ut.test("4.10a",   fsExt2 != fsExt);
   ut.test("4.10b", !(fsExt2 == fsExt));
   temp = fsWithExt;
   ut.test("4.10c",   fsWithExt  == temp);
   ut.test("4.10d",   fsWithExt  != fsWithExt2);
   ut.test("4.11a",   fsWithExt  != fsWithExt3);
   ut.test("4.11b",   fsWithExt2 != fsWithExt3);

   // Test 5. GetDrive
   FileSpec fsDrive("c:"), fsDriveFull("c:\\");
   FileSpec fsLongPath("c:\\swp30\\pictures\\image.bmp");
   FileSpec fsUNCDrive("\\\\manawa\\winbuild.swpTL");
   FileSpec fsUNCPath("\\\\manawa\\winbuild.swpTL\\swp30\\pictures\\image.bmp");

   ut.test("5.1", fsDefault.GetDrive()  == fsDefault);
   ut.test("5.2", fsExplicit.GetDrive() == fsDefault);
   ut.test("5.3", fsDrive.GetDrive()    == fsDriveFull);
   ut.test("5.4", fsDriveFull.GetDrive()== fsDriveFull);
   ut.test("5.5", fsLongPath.GetDrive() == fsDriveFull);
   ut.test("5.6", fsUNCDrive.GetDrive() == fsUNCDrive);
   ut.test("5.7", fsUNCPath.GetDrive()  == fsUNCDrive);
   
   // Test 6. Assignment
   
   // Test 7. operator including += (there is no operator +)
   temp = fsExplicit; temp += fsExplicit;
   temp2 = fsExplicit; temp2.MakePath(); temp2 += fsExplicit;
   ut.test("7.1", temp == temp2);
   FileSpec fsURLBase("http://www.scinotebook.com");
   temp = fsURLBase;
   temp += fsWithExt;
   FileSpec fsURLFull("http://www.scinotebook.com/filename.ext");
   ut.test("7.2", temp == fsURLFull);
   
   // Test 8. extraction

   // Test 9. GetExtension
   FileSpec fsExtQiz(".qiz");
   FileSpec fsRelExt("./MyDoc.qiz");
   FileSpec fsRelExt2(".\\MyDoc.qiz");
   FileSpec fsRelExt3("../MyDoc.qiz");
   FileSpec fsRelExt4("..\\MyDoc.qiz");
   FileSpec fsRelExt5("../directory/MyDoc.qiz");
   FileSpec fsRelExt6("..\\directory\\MyDoc.qiz");
   FileSpec fsRelExt7("./directory/MyDoc.qiz");
   FileSpec fsRelExt8(".\\directory\\MyDoc.qiz");
   FileSpec fsNonRelExt("MyDoc.qiz");
   FileSpec fsNonRelExt2("directory/MyDoc.qiz");
   FileSpec fsNoExt("MyDoc");
   FileSpec fsAbsExt("D:/MyDoc.qiz");
   FileSpec fsAbsExt2("D:\\MyDoc.qiz");
   FileSpec fsUNCExt("\\\\manawa\\winbuild.swpTL\\swp30\\pictures\\MyDoc.qiz");
   FileSpec fsUNCExt2("\\\\manawa\\winbuild.swpTL\\MyDoc.qiz");
   FileSpec fsURLExt("http://www.hipnt.com/testing/MyDoc.qiz");
   FileSpec fsURLExt2("ftp://www.hipnt.com/MyDoc.qiz");
   FileSpec fsURLExt3("ftp://www.hipnt.com/MyDoc.qiz?chap=one&subj=Two");
   FileSpec fsURLmail("mailto:support@mackichan.com");
   FileSpec fsURLnews("news:comp.sci.text.tex");
   FileSpec fsExtraDot("C:/Some.Doc.tex");

   ut.test("9.1",   fsRelExt.GetExtension()    == fsExtQiz);
   ut.test("9.2a",  fsRelExt2.GetExtension()   == fsExtQiz);
   ut.test("9.2b",  fsRelExt3.GetExtension()   == fsExtQiz);
   ut.test("9.2c",  fsRelExt4.GetExtension()   == fsExtQiz);
   ut.test("9.2d",  fsRelExt5.GetExtension()   == fsExtQiz);
   ut.test("9.2e",  fsRelExt6.GetExtension()   == fsExtQiz);
   ut.test("9.2f",  fsRelExt7.GetExtension()   == fsExtQiz);
   ut.test("9.2g",  fsRelExt8.GetExtension()   == fsExtQiz);
   ut.test("9.3a",  fsNonRelExt.GetExtension() == fsExtQiz);
   ut.test("9.3b",  fsNonRelExt2.GetExtension()== fsExtQiz);
   ut.test("9.4",   fsAbsExt.GetExtension()    == fsExtQiz);
   ut.test("9.5",   fsAbsExt2.GetExtension()   == fsExtQiz);
   ut.test("9.6",   fsUNCExt.GetExtension()    == fsExtQiz);
   ut.test("9.7",   fsURLExt.GetExtension()    == fsExtQiz);
   ut.test("9.8",   fsURLExt2.GetExtension()   == fsExtQiz);
   ut.test("9.9",   fsURLExt3.GetExtension()   == fsExtQiz);
   ut.test("9.10",  fsExtraDot.GetExtension()  == fsExtTeX);
   FileSpec fsExtraDotName( fsExtraDot.GetName(TRUE) );
   fsExtraDot.SetExtension( fsExtQiz );
   ut.test("9.11",  fsExtraDot.GetExtension()  == fsExtQiz);
   ut.test("9.12",  fsExtraDot.GetName(TRUE)   == fsExtraDotName);

   // Test 10. GetName
   ut.test("10.1",   fsRelExt.GetName()    == fsNonRelExt);
   ut.test("10.2a",  fsRelExt2.GetName()   == fsNonRelExt);
   ut.test("10.2b",  fsRelExt3.GetName()   == fsNonRelExt);
   ut.test("10.2c",  fsRelExt4.GetName()   == fsNonRelExt);
   ut.test("10.2d",  fsRelExt5.GetName()   == fsNonRelExt);
   ut.test("10.2e",  fsRelExt6.GetName()   == fsNonRelExt);
   ut.test("10.2f",  fsRelExt6.GetName()   == fsNonRelExt);
   ut.test("10.2g",  fsRelExt6.GetName()   == fsNonRelExt);
   ut.test("10.3a",  fsNonRelExt.GetName() == fsNonRelExt);
   ut.test("10.3b",  fsNonRelExt2.GetName()== fsNonRelExt);
   ut.test("10.4",   fsAbsExt.GetName()    == fsNonRelExt);
   ut.test("10.5",   fsAbsExt2.GetName()   == fsNonRelExt);
   ut.test("10.6",   fsUNCExt.GetName()    == fsNonRelExt);
   ut.test("10.7",   fsURLExt.GetName()    == fsNonRelExt);
   ut.test("10.8",   fsURLExt2.GetName()   == fsNonRelExt);
   ut.test("10.9",   fsURLExt3.GetName()   == fsNonRelExt);
   ut.test("10.10",  fsExtQiz.GetName()    == fsDefault);

   ut.test("10T.1",   fsRelExt.GetName(TRUE)    == fsNoExt);
   ut.test("10T.2a",  fsRelExt2.GetName(TRUE)   == fsNoExt);
   ut.test("10T.2b",  fsRelExt3.GetName(TRUE)   == fsNoExt);
   ut.test("10T.2c",  fsRelExt4.GetName(TRUE)   == fsNoExt);
   ut.test("10T.2d",  fsRelExt5.GetName(TRUE)   == fsNoExt);
   ut.test("10T.2e",  fsRelExt6.GetName(TRUE)   == fsNoExt);
   ut.test("10T.2f",  fsRelExt6.GetName(TRUE)   == fsNoExt);
   ut.test("10T.2g",  fsRelExt6.GetName(TRUE)   == fsNoExt);
   ut.test("10T.3a",  fsNonRelExt.GetName(TRUE) == fsNoExt);
   ut.test("10T.3b",  fsNonRelExt2.GetName(TRUE)== fsNoExt);
   ut.test("10T.4",   fsAbsExt.GetName(TRUE)    == fsNoExt);
   ut.test("10T.5",   fsAbsExt2.GetName(TRUE)   == fsNoExt);
   ut.test("10T.6",   fsUNCExt.GetName(TRUE)    == fsNoExt);
   ut.test("10T.7",   fsURLExt.GetName(TRUE)    == fsNoExt);
   ut.test("10T.8",   fsURLExt2.GetName(TRUE)   == fsNoExt);
   ut.test("10T.9",   fsURLExt3.GetName(TRUE)   == fsNoExt);

   // Test 11. IsRelative()
   ut.test("11.1",   fsRelExt.IsRelative() );
   ut.test("11.2a",  fsRelExt2.IsRelative() );
   ut.test("11.2b",  fsRelExt3.IsRelative() );
   ut.test("11.2c",  fsRelExt4.IsRelative() );
   ut.test("11.2d",  fsRelExt5.IsRelative() );
   ut.test("11.2e",  fsRelExt6.IsRelative() );
   ut.test("11.2f",  fsRelExt7.IsRelative() );
   ut.test("11.2g",  fsRelExt8.IsRelative() );
   ut.test("11.3a",  fsNonRelExt.IsRelative() );
   ut.test("11.3b",  fsNonRelExt2.IsRelative() );
   ut.test("11.3c",  fsNoExt.IsRelative() );
   ut.test("11.4",   !fsAbsExt.IsRelative() );
   ut.test("11.5",   !fsAbsExt2.IsRelative() );
   ut.test("11.6",   !fsUNCExt.IsRelative() );
   ut.test("11.7",   !fsURLExt.IsRelative() );
   ut.test("11.8",   !fsURLExt2.IsRelative() );
   ut.test("11.9",   !fsURLExt3.IsRelative() );
   
   // Test 12.  AbbrevName()
   TCIString aName;
   ut.test("12.1",  fsRelExt.AbbrevName(aName, 100, TRUE)  && aName == fsRelExt);
   ut.test("12.1f", fsRelExt.AbbrevName(aName, 100, FALSE) && aName == fsRelExt);

   //   fsUNCExt("\\\\manawa\\winbuild.swpTL\\swp30\\pictures\\MyDoc.qiz");
   ut.test("12.2",   !fsUNCExt.AbbrevName(aName, 5, TRUE)  || aName != fsUNCExt);
   ut.test("12.2f",  !fsUNCExt.AbbrevName(aName, 5, FALSE) || aName != fsUNCExt);
   ut.test("12.21",  fsUNCExt.AbbrevName(aName, 10, TRUE)  && aName == "MyDoc.qiz");
   ut.test("12.22",  fsUNCExt.AbbrevName(aName, 20, TRUE)  && aName == "...\\...\\MyDoc.qiz");
   ut.test("12.23",  fsUNCExt.AbbrevName(aName, 38, TRUE)  && aName == "\\\\manawa\\winbuild.swpTL\\...\\MyDoc.qiz");
   //   fsUNCExt2("\\\\manawa\\winbuild.swpTL\\MyDoc.qiz");
   ut.test("12.25",   !fsUNCExt2.AbbrevName(aName, 5, TRUE)  || aName != fsUNCExt2);
   ut.test("12.25f",  !fsUNCExt2.AbbrevName(aName, 5, FALSE) || aName != fsUNCExt2);
   ut.test("12.26",  fsUNCExt2.AbbrevName(aName, 10, TRUE)  && aName == "MyDoc.qiz");
   ut.test("12.27",  fsUNCExt2.AbbrevName(aName, 20, TRUE)  && aName == "...\\MyDoc.qiz");
   ut.test("12.28",  fsUNCExt2.AbbrevName(aName, 38, TRUE)  && aName == fsUNCExt2);

   FileSpec fsLongDir("c:\\an unnecessarily long directory name\\a.tex");
   ut.test("12.3",   fsLongDir.AbbrevName(aName, 10, TRUE)  && aName == "...\\a.tex");
   ut.test("12.3f",  fsLongDir.AbbrevName(aName, 10, FALSE) && aName == "...\\a.tex");
   ut.test("12.31",  fsLongDir.AbbrevName(aName, 12, TRUE)  && aName == "c:\\...\\a.tex");
   ut.test("12.31f", fsLongDir.AbbrevName(aName, 12, FALSE) && aName == "c:\\...\\a.tex");
   ut.test("12.32",  fsLongDir.AbbrevName(aName, 8, TRUE)  && aName == "a.tex");
   //   fsURLExt("http://www.hipnt.com/testing/MyDoc.qiz");
   ut.test("12.4",   !fsURLExt.AbbrevName(aName, 5, TRUE)  || aName != fsURLExt);
   ut.test("12.4f",  !fsURLExt.AbbrevName(aName, 5, FALSE) || aName != fsURLExt);
   ut.test("12.41",  fsURLExt.AbbrevName(aName, 10, TRUE)  && aName == "MyDoc.qiz");
   ut.test("12.42",  fsURLExt.AbbrevName(aName, 20, TRUE)  && aName == "...\\...\\MyDoc.qiz");
   ut.test("12.43",  fsURLExt.AbbrevName(aName, 35, TRUE)  && aName == "http://www.hipnt.com\\...\\MyDoc.qiz");
   //   fsURLExt2("ftp://www.hipnt.com/MyDoc.qiz");
   ut.test("12.45",   !fsUNCExt2.AbbrevName(aName, 5, TRUE)  || aName != fsUNCExt2);
   ut.test("12.45f",  !fsUNCExt2.AbbrevName(aName, 5, FALSE) || aName != fsUNCExt2);
   ut.test("12.46",  fsUNCExt2.AbbrevName(aName, 10, TRUE)  && aName == "MyDoc.qiz");
   ut.test("12.47",  fsUNCExt2.AbbrevName(aName, 20, TRUE)  && aName == "...\\MyDoc.qiz");
   ut.test("12.48",  fsUNCExt2.AbbrevName(aName, 38, TRUE)  && aName == fsUNCExt2);
  //   fsAbsExt("D:/MyDoc.qiz");
   aName = "";
   ut.test("12.5",   !fsAbsExt.AbbrevName(aName, 5, FALSE)  && aName != "MyDoc.qiz");
   ut.test("12.5f",  !fsAbsExt.AbbrevName(aName, 5, TRUE) && aName == "MyDoc.qiz");
   ut.test("12.51",  fsAbsExt.AbbrevName(aName, 10, TRUE)  && aName == "MyDoc.qiz");
   ut.test("12.52",  fsAbsExt.AbbrevName(aName, 13, TRUE)  && aName == fsAbsExt);
//   fsAbsExt2("D:\\MyDoc.qiz");
   aName = "";
   ut.test("12.55",   !fsAbsExt2.AbbrevName(aName, 5, FALSE)  && aName != "MyDoc.qiz");
   ut.test("12.55f",  !fsAbsExt2.AbbrevName(aName, 5, TRUE) && aName == "MyDoc.qiz");
   ut.test("12.56",   fsAbsExt2.AbbrevName(aName, 10, TRUE)  && aName == "MyDoc.qiz");
   ut.test("12.57",   fsAbsExt2.AbbrevName(aName, 13, TRUE)  && aName == fsAbsExt2);

   FileSpec fsLongDir2("c:\\an unnecessarily long directory name\\was I thinking\\a.tex");
   ut.test("12.6",   fsLongDir2.AbbrevName(aName, 10, TRUE)  && aName == "...\\a.tex");
   ut.test("12.6f",  fsLongDir2.AbbrevName(aName, 10, FALSE) && aName == "...\\a.tex");
   ut.test("12.61",  fsLongDir2.AbbrevName(aName, 12, TRUE)  && aName == "c:\\...\\a.tex");
   ut.test("12.61f", fsLongDir2.AbbrevName(aName, 12, FALSE) && aName == "c:\\...\\a.tex");
   ut.test("12.62",  fsLongDir2.AbbrevName(aName, 8, TRUE)  && aName == "a.tex");
   ut.test("12.63",  fsLongDir2.AbbrevName(aName, 30, TRUE)  && aName == "c:\\...\\...\\a.tex");
   ut.test("12.64",  fsLongDir2.AbbrevName(aName, 35, TRUE)  && aName == "c:\\...\\was I thinking\\a.tex");

   FileSpec fsLongDir3("c:\\was I thinking\\an unnecessarily long directory name\\a.tex");
   ut.test("12.7",   fsLongDir3.AbbrevName(aName, 10, TRUE)  && aName == "...\\a.tex");
   ut.test("12.7f",  fsLongDir3.AbbrevName(aName, 10, FALSE) && aName == "...\\a.tex");
   ut.test("12.71",  fsLongDir3.AbbrevName(aName, 12, TRUE)  && aName == "c:\\...\\a.tex");
   ut.test("12.71f", fsLongDir3.AbbrevName(aName, 12, FALSE) && aName == "c:\\...\\a.tex");
   ut.test("12.72",  fsLongDir3.AbbrevName(aName, 8, TRUE)  && aName == "a.tex");
   ut.test("12.73",  fsLongDir3.AbbrevName(aName, 20, TRUE)  && aName == "c:\\...\\...\\a.tex");
   ut.test("12.74",  fsLongDir3.AbbrevName(aName, 30, TRUE)  && aName == "c:\\was I thinking\\...\\a.tex");

   FileSpec fsLongDir4("\\an unnecessarily long directory name\\a.tex");
   ut.test("12.8",   fsLongDir4.AbbrevName(aName, 8, TRUE)  && aName == "a.tex");
   ut.test("12.8f",  fsLongDir4.AbbrevName(aName, 8, FALSE) && aName == "a.tex");
   ut.test("12.81",  fsLongDir4.AbbrevName(aName, 12, TRUE)  && aName == "\\...\\a.tex");
   ut.test("12.81f", fsLongDir4.AbbrevName(aName, 12, FALSE) && aName == "\\...\\a.tex");
   ut.test("12.82",  fsLongDir4.AbbrevName(aName, 8, TRUE)  && aName == "a.tex");

   // Test 13.  IsURL()
   ut.test("13.1", !fsNonRelExt.IsURL());
   ut.test("13.2", !fsNonRelExt2.IsURL());
   ut.test("13.3", !fsNoExt.IsURL());
   ut.test("13.4", !fsAbsExt.IsURL());
   ut.test("13.5", !fsAbsExt2.IsURL());
   ut.test("13.6", !fsUNCExt.IsURL());
   ut.test("13.7", !fsUNCExt2.IsURL());
   ut.test("13.8",  fsURLExt.IsURL());
   ut.test("13.9",  fsURLExt2.IsURL());
   ut.test("13.10", fsURLExt3.IsURL());
   ut.test("13.11", fsURLmail.IsURL());
   ut.test("13.12", fsURLnews.IsURL());
}