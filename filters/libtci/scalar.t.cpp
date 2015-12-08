
// Unit test for component FileSpec

#include "scalar.h"
#include "unittest.h"

TCI_BOOL bSuppressAssertions = FALSE;

void main()
{
   UnitTest  ut("Scalar");

   
   // Test 1. xn_over_d
   ut.test("1.1", xn_over_d( 123456789, 1470369258, 975318642 ) == 186120781);
   
   // Test 2. RoundToScale
   ut.test("2.1", RoundToScale(473, 0x40000) == 472);
   ut.test("2.2", RoundToScale(-1371591, 0x8000) == -1371591);
   ut.test("2.3", RoundToScale(2489, 15555) == 2489);
   
   // Test 3. RoundUpToScale
   ut.test("3.1", RoundUpToScale(73, 0x40000) == 76);
   ut.test("3.2", RoundUpToScale(-365, 19, 2) ==  -371);
   
   // Test 4. RoundDownToScale
   ut.test("4.1", RoundDownToScale(73, 0x40000) == 72);
   ut.test("4.2", RoundDownToScale(-365, 19, 2) == -361);

   // Test 5. LongMult
   U32 res_hi, res_lo;
   LongMult(0,0,res_hi,res_lo);
   ut.test("5.1", res_hi==0&&res_lo==0);
   LongMult(1,1,res_hi,res_lo);
   ut.test("5.2", res_hi==0&&res_lo==1);
   LongMult(123456789,123456789,res_hi,res_lo);
   ut.test("5.3", res_hi==3548706&&res_lo==0x9738A3B9);
   LongMult(0x10000,0x10000,res_hi,res_lo);
   ut.test("5.4", res_hi==1&&res_lo==0);
}