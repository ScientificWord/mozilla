
// Unit test for component TCIString

#include "tcistrin.h"
#include "strutil.h"
#include "unittest.h"

void* GetInstanceHandle() {return NULL;}

static U8       *u8str = reinterpret_cast<U8*>("This is a test.");
static TCICHAR  *cstr  = "This is a test." ;

TCI_BOOL testGetParamString(TCICHAR* pBuff, int& nTheLen, const TCIString& paramName, void* pData);
//TCI_BOOL testGetParamString(TCIString& paramVal, const TCIString& paramName, void* pData);
TCI_BOOL testGetParamInt(int& paramVal, const TCIString& paramName, void* pData);
TCI_BOOL testGetParamUInt(U32& paramVal, const TCIString& paramName, void* pData);
TCI_BOOL testGetParamChar(TCICHAR& paramVal, const TCIString& paramName, void* pData);
TCI_BOOL testGetParamDouble(double& paramVal, const TCIString& paramName, void* pData);
TCI_BOOL testGetParamPtr(void*& paramVal, const TCIString& paramName, void* pData);
TCI_BOOL testGetParamWChar(TCIWCHAR& paramVal, const TCIString& paramName, void* pData);


void main()
{
   UnitTest  ut("StrUtil");
   // Construct on the stack
   TCIString strDefault;                     // Default constructor
   TCIString strExplicit(cstr);              // TCICHAR* constructor
   TCIString strCopy(strExplicit);           // Copy constructor
   TCIString strU8(u8str);                   // U8* constructor
   
   TCIString temp;
      
   // Test 1. SelectSubstringUsingIndex
   TCIString fileFilterSample("TeX Files (*.tex)|*.tex|Text Files (*.txt)|*.txt|RTF Files (*.rtf)|*.rtf|HTML Files (*.html)|*.html|");
   TCIString leadingSeparator("%xxx%yyy%");
   TCIString testItAgain("000#1#222##3333#44###5#");
   ut.test("1.1", StrUtil::SelectSubstringUsingIndex(strDefault,0,',') == strDefault);
   ut.test("1.2", StrUtil::SelectSubstringUsingIndex(strExplicit,0,' ') == "This");
   ut.test("1.3", StrUtil::SelectSubstringUsingIndex(strExplicit,1,' ') == "is");
   ut.test("1.4", StrUtil::SelectSubstringUsingIndex(strExplicit,2,' ') == "a");
   ut.test("1.5", StrUtil::SelectSubstringUsingIndex(strExplicit,3,' ') == "test.");
   ut.test("1.6", StrUtil::SelectSubstringUsingIndex(fileFilterSample,2,'|') == "Text Files (*.txt)");
   ut.test("1.7", StrUtil::SelectSubstringUsingIndex(fileFilterSample,6,'|') == "HTML Files (*.html)");
   ut.test("1.8", StrUtil::SelectSubstringUsingIndex(testItAgain,2,'#') == "222");
   ut.test("1.9", StrUtil::SelectSubstringUsingIndex(testItAgain,5,'#') == "44");
   ut.test("1.10", StrUtil::SelectSubstringUsingIndex(testItAgain,6,'#') == strDefault);
   ut.test("1.11", StrUtil::SelectSubstringUsingIndex(leadingSeparator,1,'%') == "yyy");

   // Test 2. GetNextSubstring
   StrUtil::substringEnumerator subEnum = StrUtil::StartEnumSubstrings(testItAgain, '#');
   TCIString testNum;
   TCIString nextString;
   ut.test("2.1", StrUtil::GetNextSubstring(subEnum, nextString) && (nextString=="000"));
   ut.test("2.2", StrUtil::GetNextSubstring(subEnum, nextString) && (nextString=="1"));
   ut.test("2.3", StrUtil::GetNextSubstring(subEnum, nextString) && (nextString=="222"));
   ut.test("2.4", StrUtil::GetNextSubstring(subEnum, nextString) && (nextString==""));
   ut.test("2.5", StrUtil::GetNextSubstring(subEnum, nextString) && (nextString=="3333"));
   ut.test("2.6", StrUtil::GetNextSubstring(subEnum, nextString) && (nextString=="44"));
   ut.test("2.7", StrUtil::GetNextSubstring(subEnum, nextString) && (nextString==""));
   ut.test("2.8", StrUtil::GetNextSubstring(subEnum, nextString) && (nextString==""));
   ut.test("2.9", StrUtil::GetNextSubstring(subEnum, nextString) && (nextString=="5"));
   ut.test("2.10", !StrUtil::GetNextSubstring(subEnum, nextString));
   TCIString aDifferentTest("no|ending|separator");
   TCIString aSimpleTest("justAWord");
   StrUtil::substringEnumerator subEnum2 = StrUtil::StartEnumSubstrings(aDifferentTest, '|');
   ut.test("2.11", StrUtil::GetNextSubstring(subEnum2, nextString) && (nextString=="no"));
   ut.test("2.12", StrUtil::GetNextSubstring(subEnum2, nextString) && (nextString=="ending"));
   ut.test("2.13", StrUtil::GetNextSubstring(subEnum2, nextString) && (nextString=="separator"));
   ut.test("2.14", !StrUtil::GetNextSubstring(subEnum2, nextString));
   StrUtil::substringEnumerator subEnum3 = StrUtil::StartEnumSubstrings(aSimpleTest, '&');
   ut.test("2.15", StrUtil::GetNextSubstring(subEnum3, nextString) && (nextString=="justAWord"));
   ut.test("2.16", !StrUtil::GetNextSubstring(subEnum3, nextString));
   // Test 3. StringToDouble and DoubleToString
   ut.test("3.1", StrUtil::StringToDouble("0") == 0.0);
   ut.test("3.2", StrUtil::StringToDouble("-0.2") == -0.2);
   ut.test("3.3", StrUtil::StringToDouble("-4.13E-4") == -.000413);
   ut.test("3.4", StrUtil::StringToDouble("+10987.34") == 10987.34);
   ut.test("3.5", StrUtil::StringToDouble("abc-3.04") == -3.04);
   ut.test("3.6", StrUtil::DoubleToString(2468013579) == "2.46801e+009");
   ut.test("3.7", StrUtil::DoubleToString(-0.00099) == "-0.00099");
   ut.test("3.8", StrUtil::DoubleToString(-0.0000099) == "-9.9e-006");

   // Test 4. Formatting strings with "parameters".
   StrUtil::stringWithParamsFormatter fmt;
   fmt.setCallbacks(testGetParamString, testGetParamInt, testGetParamUInt,
                        testGetParamChar, testGetParamDouble, 
                        testGetParamPtr, testGetParamWChar);
   ut.test("4.1", fmt.format( "The book I'm reading is %s, and I'm on chapter %s.|<book>|<chapter>" ) == "The book I'm reading is The Decline and Fall of the Roman Empire, and I'm on chapter III.");
   ut.test("4.2", fmt.format("Winter temperatures may reach %d Celsuis, while summer ones can climb to %d Celsius.|<lowTemp>|<highTemp>") == "Winter temperatures may reach -15 Celsuis, while summer ones can climb to 27 Celsius.");
   ut.test("4.3", fmt.format("Today is %u/%u/%u.|<month>|<day>|<year>") == "Today is 4/5/2003.");
   ut.test("4.4", fmt.format("&amp; gets translated to '%c', while %s turns into '%c'.|<&amp;>||<&sp;>", "&sp;") == "&amp; gets translated to '&', while &sp; turns into ' '.");
   ut.test("4.5", fmt.format("The speed of light in a vacuum is approximately %.8e.|c") == "The speed of light in a vacuum is approximately 2.99792458e+008.");
   ut.test("4.6", fmt.format("The value of %s is approximately %.*g.|<pi>||<pi>",6) == "The value of pi is approximately 3.14159.");
   TCIString shouldComeOut = StrUtil::Format( "The routine 'strlen' is located at address %p.", &strlen );
   ut.test("4.7", fmt.format("The routine 'strlen' is located at address %p.|<strlen>") == shouldComeOut);
   fmt.setCallbackData( &aSimpleTest );
   void* theAddress = aSimpleTest.GetBuffer(0);
   shouldComeOut = StrUtil::Format("The data for the tcistring aSimpleTest is located at %p.", theAddress);
   ut.test("4.8", fmt.format("The data for the tcistring %s is located at %p.||<string data>", "aSimpleTest") == shouldComeOut);
   fmt.setCallbackData( NULL );

   //Test 5. BasedNumberString
   int basedTestNum = 1479803;
   ut.test("5.1", StrUtil::BasedNumberString(basedTestNum, 36, NULL, 6) == "VPTN");
   ut.test("5.2", StrUtil::BasedNumberString(basedTestNum, 36, NULL, 6, TRUE) == "00VPTN");

}


TCI_BOOL testGetParamString(TCICHAR* pBuff, int& nTheLen, const TCIString& paramName, void* pData)
{
  TCI_BOOL rv = TRUE;
  TCIString retVal;
  if (paramName == "book")
    retVal = "The Decline and Fall of the Roman Empire";
  else if (paramName == "chapter")
    retVal = "III";
  else if (paramName == "pi")
    retVal = "pi";
  else
    rv = FALSE;
  if (rv)
  {
    int nOurLen = retVal.GetLength();
    if (nTheLen <= nOurLen)
      rv = FALSE;
    else
    {
      strncpy( pBuff, (const TCICHAR*)retVal, nTheLen );
      pBuff[nTheLen] = 0;
    }
    nTheLen = nOurLen;
  }
  return rv;
}

TCI_BOOL testGetParamInt(int& paramVal, const TCIString& paramName, void* pData)
{
  TCI_BOOL rv = TRUE;
  if (paramName == "lowTemp")
    paramVal = -15;
  else if (paramName == "highTemp")
    paramVal = 27;
  else
    rv = FALSE;
  return rv;
}

TCI_BOOL testGetParamUInt(U32& paramVal, const TCIString& paramName, void* pData)
{
  TCI_BOOL rv = TRUE;
  if (paramName == "day")
    paramVal = 5;
  else if (paramName == "month")
    paramVal = 4;
  else if (paramName == "year")
    paramVal = 2003;
  else
    rv = FALSE;
  return rv;
}

TCI_BOOL testGetParamChar(TCICHAR& paramVal, const TCIString& paramName, void* pData)
{ 
  TCI_BOOL rv = TRUE;
  if (paramName == "&amp;")
    paramVal = '&';
  else if (paramName == "&sp;")
    paramVal = ' ';
  else
    rv = FALSE;
  return rv;
}

TCI_BOOL testGetParamDouble(double& paramVal, const TCIString& paramName, void* pData)
{
  TCI_BOOL rv = TRUE;
  if (paramName == "pi")
    paramVal = 3.14159;
  else if (paramName == "c")
    paramVal = 2.99792458E8;
  else if (paramName == "Atomic Mass Constant")
    paramVal = 1.6605402E7;
  else
    rv = FALSE;
  return rv;
}

TCI_BOOL testGetParamPtr(void*& paramVal, const TCIString& paramName, void* pData)
{
  TCI_BOOL rv = TRUE;
  if (paramName == "strlen")
    paramVal = &strlen;
  else if (paramName == "string data" && pData != NULL)
    paramVal = ((TCIString*)pData)->GetBuffer(0);
  else
    rv = FALSE;
  return rv;
}

TCI_BOOL testGetParamWChar(TCIWCHAR& paramVal, const TCIString& paramName, void* pData)
{
  TCI_BOOL rv = TRUE;
  if (paramName == "Real Numbers")
    paramVal = 0x211d;
  else if (paramName == "male")
    paramVal = 0x2642;
  else if (paramName == "Arabic Jeem")
    paramVal = 0xfe9d;
  else if (paramName == "eighth notes")
    paramVal = 0x2668;
  else if (paramName == "pi")
    paramVal = 0x03c0;
  else
    rv = FALSE;
  return rv;
}
