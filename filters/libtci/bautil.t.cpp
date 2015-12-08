
// Unit test for component FileSpec

#include "bautil.h"
#include "bytearry.h"
#include "tcistrin.h"
#include "unittest.h"

TCI_BOOL bSuppressAssertions = FALSE;

static TCICHAR  *strOne   = "One little stringy dingy.";
static TCICHAR  *strTwo   = "Two little stringy dingies.";
static TCICHAR  *strThree = "Three little stringy dingies.";
static TCICHAR  *strFour  = "Four little stringy dingies.";
static TCICHAR  *multiLineOne = "This be a bit of text.\nIt includes a newline or\r\n\r\ntwo.";
static TCICHAR  *compareBuffer0 = "\r\nIt includes a newline or\r\n\r\ntwo.";
static TCICHAR  *compareBuffer1 = "This be a bit of text.\nThis text been different;\r\n\r\ntwo.";
static TCICHAR  *compareBuffer2 = "This be a bit of text.\nIt includes a newline or\r\nThis text been different;\r\ntwo.";
static TCICHAR  *compareBuffer3 = "This be a bit of text.\nIt includes a newline or\r\n\r\nThis text been different;\r\n";
static TCICHAR  *compareBuffer4 = "This be a bit of text.\nIt includes a newline or\r\n\r\ntwo.\0This text been different;\r\n";

TCI_BOOL ByteArrayEqualsBuffer(const ByteArray& ba, const U8* compareBuffer, 
                         int nNumBytes, int nOffset = 0)
{
  const U8* buff1 = ba.ReadOnlyLock();
  TCI_BOOL rv = !(memcmp(buff1 + nOffset, compareBuffer + nOffset, nNumBytes));
  ba.ReadOnlyUnlock();
  return rv;
}
                         
void main()
{
   UnitTest  ut("BAUtil");
   // Construct on the stack
   ByteArray baDefault;                      // Default constructor
   ByteArray baExplicit;
   baExplicit.AddString(strOne);              // TCIString& constructor
   baExplicit.AddString( strTwo );
   baExplicit.AddString( strThree );
   baExplicit.AddString( strFour );
   ByteArray baCopy(baExplicit);           // Copy constructor
   ByteArray baBySize(256);                 //Explicit size constructor
   ByteArray baMultiLine;
   baMultiLine.AddString(multiLineOne);
   ByteArray baReplaceLine;

   // Construct on the heap and delete
   ByteArray* pBAHeap = new ByteArray(baBySize);
   delete pBAHeap;
   
   ByteArray temp, temp2;
   U8* pBuffer = new U8[1024]; //this number may want increasing
   memset(pBuffer, 0, 1024); 
   int nBytes = 0;
   U32 nOffset = 0;
   U32 nLineOffset1 = 0, nLineOffset2 = 0, nLineOffset3 = 0;
         
   // Test 1. GetNextString
   TCIString nextStr;
   ut.test("1.1", BAUtil::GetNextString(baExplicit, nOffset, nextStr) && nextStr==strOne);
   ut.test("1.2", BAUtil::GetNextString(baExplicit, nOffset, nextStr) && nextStr==strTwo);
   ut.test("1.3", BAUtil::GetNextString(baExplicit, nOffset, nextStr) && nextStr==strThree);
   ut.test("1.4", BAUtil::GetNextString(baExplicit, nOffset, nextStr) && nextStr==strFour);
   ut.test("1.5", !BAUtil::GetNextString(baExplicit, nOffset, nextStr));
   nOffset = 0;
   ut.test("1.6", !BAUtil::GetNextString(baDefault, nOffset, nextStr));
   
   // Test 4. GetNextParagraph?? Harder to set up correctly?
   
   // Test 2. FindString, HasString, StartsWithString, MatchesString
   ut.test("2.1", BAUtil::HasString(baExplicit, strThree));
   ut.test("2.2", !BAUtil::HasString(baExplicit, "String not in the above"));
   ut.test("2.3", BAUtil::FindString(baExplicit, strFour) == (int)(strlen(strOne) + strlen(strTwo) + strlen(strThree) + 3));
   ut.test("2.4", BAUtil::FindString(baDefault, strFour) == -1);
   TCIString firstStr(strOne);
   ut.test("2.5", BAUtil::StartsWithString(baExplicit, firstStr.Left( firstStr.GetLength() / 2 )));
   
   // Test 3. GetNextLine
   nOffset = 0;
   ut.test("3.1", BAUtil::GetNextLine(baMultiLine, nOffset, nextStr) && nextStr=="This be a bit of text.");
   nLineOffset1 = nOffset;
   ut.test("3.2", BAUtil::GetNextLine(baMultiLine, nOffset, nextStr) && nextStr=="It includes a newline or");
   nLineOffset2 = nOffset;
   ut.test("3.3", BAUtil::GetNextLine(baMultiLine, nOffset, nextStr) && nextStr=="");
   nLineOffset3 = nOffset;
   ut.test("3.4", BAUtil::GetNextLine(baMultiLine, nOffset, nextStr) && nextStr=="two.");
   ut.test("3.5", !BAUtil::GetNextLine(baMultiLine, nOffset, nextStr));
   
   // Test 4. ReplaceLine
   baReplaceLine = baMultiLine;
   ut.test("4.1", BAUtil::ReplaceLine(baReplaceLine, nLineOffset1, "This text been different;") && ByteArrayEqualsBuffer(baReplaceLine, (const U8*)compareBuffer1, strlen(compareBuffer1), 0));
   baReplaceLine = baMultiLine;
   ut.test("4.2", BAUtil::ReplaceLine(baReplaceLine, nLineOffset2, "This text been different;") && ByteArrayEqualsBuffer(baReplaceLine, (const U8*)compareBuffer2, strlen(compareBuffer2), 0));
   baReplaceLine = baMultiLine;
   ut.test("4.3", BAUtil::ReplaceLine(baReplaceLine, 0, "") && ByteArrayEqualsBuffer(baReplaceLine, (const U8*)compareBuffer0, strlen(compareBuffer0), 0));
   baReplaceLine = baMultiLine;
   ut.test("4.4", BAUtil::ReplaceLine(baReplaceLine, nLineOffset3, "This text been different;") && ByteArrayEqualsBuffer(baReplaceLine, (const U8*)compareBuffer3, strlen(compareBuffer3), 0));
   baReplaceLine = baMultiLine;
   ut.test("4.5", BAUtil::ReplaceLine(baReplaceLine, baReplaceLine.GetByteCount(), "This text been different;") && ByteArrayEqualsBuffer(baReplaceLine, (const U8*)compareBuffer4, strlen(compareBuffer4), 0));
}