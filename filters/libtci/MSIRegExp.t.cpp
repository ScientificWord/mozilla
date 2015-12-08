
// Unit test for component MSIRegExp

#include "regex/regex.h"
#include "MSIRegExp.h"
#include "unittest.h"

TCI_BOOL bSuppressAssertions = FALSE;

static TCICHAR  *cstr  = "This is a test." ;

void main()
{
   UnitTest  ut("MSIRegExp");
   // Construct on the stack
   TCIString temp;
      
   // Test 1. Simple searching
   TCIString simpleTestStr("Just want to test it");
   TCIString simpleTestPattern("[a-zA-Z]+ ([a-zA-Z]+)");

   MSIRegExpression ourRegExpression;
   MSIRegExpression::foundMatch ourMatches;
   ourRegExpression.setSearchExpression("want");
   TCI_BOOL bMatched = ourRegExpression.doSearch(simpleTestStr, ourMatches);
   ut.test("1.1.1", bMatched == TRUE);
   ut.test("1.1.2", ourMatches.getMatchedString() == "want");
   
   ourRegExpression.setSearchExpression(simpleTestPattern);
   bMatched = ourRegExpression.doSearch(simpleTestStr, ourMatches);
   ut.test("1.2.1", bMatched == TRUE);
   ut.test("1.2.2", ourMatches.getMatchedString() == "Just want");
   ut.test("1.2.3", ourMatches.getNumSubMatches() == 1);
   ut.test("1.2.4", ourMatches.getSubExpressionMatch(1) == "want");

   // Test 2. More complex searches
   TCIString fileFilterSample("TeX Files (*.tex)|*.tex|Text Files (*.txt)|*.txt|RTF Files (*.rtf)|*.rtf|HTML Files (*.html)|*.html|");
   TCIString fileFilterSearchExp1("\\(\\*\\.([A-Za-z]+)\\)");
   TCIString fileFilterSearchExp2("([A-Za-z]+) Files \\(\\*\\.([A-Za-z]+)\\)");
   TCIString fileFilterSearchExp3("(\\|?([A-Za-z]+) Files \\(\\*\\.[A-Za-z]+\\)\\|(\\*\\.[a-zA-Z]+))");
   for (int ix = 0; ix < 2; ++ix)
     fileFilterSearchExp3 += fileFilterSearchExp3;  //quadruple it
   ourRegExpression.setSearchExpression(fileFilterSearchExp1);
   bMatched = ourRegExpression.doSearch(fileFilterSample, ourMatches);
   ut.test("2.1.1", bMatched == TRUE);
   ut.test("2.1.2", ourMatches.getMatchedString() == "(*.tex)");
   ourRegExpression.setSearchExpression(fileFilterSearchExp2);
   bMatched = ourRegExpression.doSearch(fileFilterSample, ourMatches);
   ut.test("2.2.1", bMatched == TRUE);
   ut.test("2.2.2", ourMatches.getMatchedString() == "TeX Files (*.tex)");
   ut.test("2.2.3", ourMatches.getNumSubMatches() == 2);
   ut.test("2.2.4", ourMatches.getSubExpressionMatch(1) == "TeX");
   ut.test("2.2.5", ourMatches.getSubExpressionMatch(2) == "tex");
   ourRegExpression.setSearchExpression(fileFilterSearchExp3);
   bMatched = ourRegExpression.doSearch(fileFilterSample, ourMatches);
   ut.test("2.3.1", bMatched == TRUE);
   ut.test("2.3.2", ourMatches.getMatchedString() == fileFilterSample.Left(fileFilterSample.GetLength() - 1));
   ut.test("2.3.3", ourMatches.getNumSubMatches() == 12);
   ut.test("2.3.4", ourMatches.getSubExpressionMatch(1) == "TeX Files (*.tex)|*.tex");
   ut.test("2.3.4", ourMatches.getSubExpressionMatch(2) == "TeX");
   ut.test("2.3.5", ourMatches.getSubExpressionMatch(3) == "*.tex");
   ut.test("2.3.6", ourMatches.getSubExpressionMatch(10) == "|HTML Files (*.html)|*.html");
   ut.test("2.3.7", ourMatches.getSubExpressionMatch(11) == "HTML");
   ut.test("2.3.8", ourMatches.getSubExpressionMatch(12) == "*.html");

   TCIString quoteSample("Here's an escaped quote within a \\\"quoted\\\" string.");
   TCIString quoteSearchExp1("'([^\"]+)");
   TCIString quoteSearchExp2("'(([^\"]|(\\\"))+)");
   ourRegExpression.setSearchExpression(quoteSearchExp1);
   bMatched = ourRegExpression.doSearch(quoteSample, ourMatches);
   ut.test("2.4.1", bMatched == TRUE);
   ut.test("2.4.2", ourMatches.getMatchedString() == "'s an escaped quote within a \\");
   ourRegExpression.setSearchExpression(quoteSearchExp2);
   bMatched = ourRegExpression.doSearch(quoteSample, ourMatches);
   ut.test("2.4.3", bMatched == TRUE);
   ut.test("2.4.4", ourMatches.getMatchedString() == "'s an escaped quote within a \\\"quoted\\\" string.");

   // Test 3. Try the replacements as well.
   MSIRegExpression::subStringReplacement replacements[3];
   TCIString registrySample1("DocManParameter.0001");
   TCIString registrySearchExp1("DocManParameter\\.([0-9][0-9][0-9][0-9])");
   TCIString replaceSubExpression1("0132");
   ourRegExpression.setSearchExpression(registrySearchExp1);
   bMatched = ourRegExpression.doSearch(registrySample1, ourMatches);
   ut.test("3.1.1", bMatched == TRUE);
   ut.test("3.1.2", ourMatches.getNumSubMatches() == 1);
   ut.test("3.1.3", ourMatches.getSubExpressionMatch(1) == "0001");
   replacements[0].set(1, replaceSubExpression1);
   ourMatches.replaceSubExpressions(1, replacements);
   ut.test("3.1.4", ourMatches.getMatchedString() == "DocManParameter.0132");

   TCIString registrySample2("/iC:\\swp45\\docman.ini");
   TCIString registrySearchExp2("/[a-zA-Z](.*)");
   TCIString replaceSubExpression2("C:\\swp41\\docman.ini");
   TCIString registrySearchExp3("/[a-zA-Z](.*[\\/]([^\\/]+))$");
   TCIString replaceSubExpression3("docman.INI");
   ourRegExpression.setSearchExpression(registrySearchExp2);
   bMatched = ourRegExpression.doSearch(registrySample2, ourMatches);
   ut.test("3.2.1", bMatched == TRUE);
   ut.test("3.2.2", ourMatches.getNumSubMatches() == 1);
   ut.test("3.2.3", ourMatches.getSubExpressionMatch(1) == "C:\\swp45\\docman.ini");
   replacements[0].set(1, replaceSubExpression2);
   ourMatches.replaceSubExpressions(1, replacements);
   ut.test("3.2.4", ourMatches.getMatchedString() == "/iC:\\swp41\\docman.ini");
   ourRegExpression.setSearchExpression(registrySearchExp3);
   bMatched = ourRegExpression.doSearch(registrySample2, ourMatches);
   ut.test("3.2.5", bMatched == TRUE);
   ut.test("3.2.6", ourMatches.getSubExpressionMatch(2) == "docman.ini");
   replacements[0].set(2, replaceSubExpression3);
   ourMatches.replaceSubExpressions(1, replacements);
   ut.test("3.2.7", ourMatches.getMatchedString() == "/iC:\\swp45\\docman.INI");

   TCIString registrySample3("TrueTeX,%INS_DVIEXE1%,!%x \"-w%s%v,%d,%o\" %r -c%n -o%b:%e:%i -i% %I %f -z!,%INS_PVINI1%,N!-b!!,!!!,!!!,,%v,,,1");
   TCIString baseRegistrySearchExp4(",?(([^,]+|(\"[^\"]*\"))*)");
   TCIString registrySearchExp4(baseRegistrySearchExp4);
   for (ix = 0; ix < 11; ++ix)
     registrySearchExp4 += baseRegistrySearchExp4;
   ourRegExpression.setSearchExpression(registrySearchExp4);
   bMatched = ourRegExpression.doSearch(registrySample3, ourMatches);
   ut.test("3.3.1", bMatched == TRUE);
   ut.test("3.3.2", ourMatches.getNumSubMatches() == 36);
   ut.test("3.3.3", ourMatches.getSubExpressionMatch(13) == "N!-b!!");
   replacements[0].set(7, "!%x \"anything in quotes\" %r -c%n -o%b:%e:%i -i% %I %f -z!");
   replacements[1].set(4, "%INS_DVIEXE22%");
   replacements[2].set(25, "this was a %v");
   ourMatches.replaceSubExpressions(3, replacements);
   ut.test("3.3.4", ourMatches.getMatchedString() == "TrueTeX,%INS_DVIEXE22%,!%x \"anything in quotes\" %r -c%n -o%b:%e:%i -i% %I %f -z!,%INS_PVINI1%,N!-b!!,!!!,!!!,,this was a %v,,,1");

  // Test 4. The test suite that came with regex.dll.

}