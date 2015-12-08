
// Unit test for component TCIString

#include "tcistrin.h"
#include "unittest.h"

TCI_BOOL bSuppressAssertions = FALSE;


static U8       *u8str = reinterpret_cast<U8*>("This is a test.");
static TCICHAR  *cstr  = "This is a test." ;

void main(){

   UnitTest  ut("TCIString");
   // Construct on the stack
   TCIString strDefault;                     // Default constructor
   TCIString strExplicit(cstr);              // TCICHAR* constructor
   TCIString strCopy(strExplicit);           // Copy constructor
   TCIString strU8(u8str);                   // U8* constructor
   
   // Construct on the heap and delete
   TCIString* pStrHeap = new TCIString(cstr);
   delete pStrHeap;
   
   TCIString temp;
      
   // Test 1. Length
   ut.test("1.1", strDefault.GetLength() == 0);
   ut.test("1.2", strExplicit.GetLength() == (int)strlen(cstr));

   // Test 2. IsEmpty
   ut.test("2.1", strDefault.IsEmpty());
   ut.test("2.2", !strExplicit.IsEmpty());
   
   // Test 3. Get At
   ut.test("3.1", strExplicit.GetAt(3) == 's');
   ut.test("3.2", strExplicit[3] == 's');
   ut.test("3.3", strExplicit[strExplicit.GetLength()-1] == '.');
   
   // Test 4. Comparisons
   ut.test("4.1", strExplicit == strCopy);
   ut.test("4.2", strExplicit == cstr);
   ut.test("4.3", cstr == strExplicit);
   ut.test("4.4", strExplicit == strU8);
   
   temp = "b";
   ut.test("4.5", temp == "b");
   ut.test("4.6", temp < "c");
   ut.test("4.7", temp > "a");
   ut.test("4.8", temp <= "c");
   
   // Test 5. Set At
   strCopy.SetAt(3,'Z');
   ut.test("5.1", strCopy[3] == 'Z');
   ut.test("5.2", strCopy != strExplicit);
   
   // Test 6. Assignment (including +=)
   temp = strExplicit;
   ut.test("6.1", strExplicit == temp);
   temp = "A";
   ut.test("6.2", "A" == temp);
   temp += "B";
   ut.test("6.3", temp == "AB");
   temp += strDefault;
   ut.test("6.3", temp == "AB");
   temp += temp;
   ut.test("6.4", temp == "ABAB");
   
   // Test 7. operator +
   temp = strExplicit;
   temp += strExplicit;
   ut.test("7.1", temp == strExplicit + strExplicit);
   
   // Test 8. extraction
   
   temp = strExplicit.Left(4);
   ut.test("8.1", temp == "This");
   temp = strExplicit.Right(5);
   ut.test("8.2", temp == "test.");
   temp = strExplicit.Mid(3, 4);
   ut.test("8.3", temp == "s is");

    // Test 9. Find
    TCIString strBase("Search string sea String");
    ut.test("9.1", strBase.Find("Search") == 0);
    ut.test("9.2", strBase.Find("search") == -1);
    ut.test("9.3", strBase.Find("string") == 7);
    ut.test("9.4", strBase.Find("STring") == -1);
    ut.test("9.5", strBase.Find("sea") == 14);
    ut.test("9.6", strBase.Find("String") == 18);
    
    // Test 10. FindNoCase        
    ut.test("10.1", strBase.FindNoCase("Search") == 0);
    ut.test("10.2", strBase.FindNoCase("search") == 0);
    ut.test("10.3", strBase.FindNoCase("search search") == -1);
    ut.test("10.4", strBase.FindNoCase("string") == 7);
    ut.test("10.5", strBase.FindNoCase("String") == 7);
    ut.test("10.6", strBase.FindNoCase("STring") == 7);
    ut.test("10.7", strBase.FindNoCase("String Search") == -1);
    ut.test("10.8", strBase.FindNoCase("sea") == 0);

}