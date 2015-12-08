
// CLASS MSIRegExp

// MSIRegExp is an interface to Regular Expression parsing.
// Currently this is just implemented as an interface to the public domain "regex" package.

#ifndef MSIRegExp_H
#define MSIRegExp_H

#ifndef TCISTRIN_H
  #include "tcistrin.h"
#endif
#ifndef _REGEX_H_
  #include "regex\regex.h"
#endif
#include <stdarg.h>

class TCIString;

//struct regex_t;  //defined in the external DLL's regex.h include file
//struct regmatch_t;  //defined in the external DLL's regex.h include file

class MSIRegExpression
{

//Nested structure definitions:
public:
  enum
  { regBasic	= 0, regExtended = 1, regIcase = 2, regNosub = 4,
    regNewline = 0x10, regNospec	= 0x20, regPend = 0x40, lastRegFlag,
    defaultMSIRegFlags = (regExtended | regNewline)
//    defaultMSIRegFlags = (regBasic | regNewline)
  };  //regular expression flags
  enum
  {
    normal = 0, noStartLine = 1, noEndLine = 2,  specifiedStartAndEnd = 4,
    defaultMSIStringFlags = normal
  };  //flags for target strings
  
  enum errorCode
  {
    noError = 0, noMatch, badPattern, invalidCollatingElement, invalidCharacterClass,
    escapedUnescapableCharacter, invalidBackReference, unmatchedBrackets, unmatchedParens,
    unmatchedBraces, invalidRepetitionCounts, invalidCharacterRange, outOfMemory,
    invalidRepetitionOperand, emptySubExpression, invalidParameter, unknownError
  };
  
  class subStringReplacement
  {
  public:
    subStringReplacement() : m_nSubString(0) {}
    subStringReplacement(int nSubString, const TCICHAR* pReplacement)
      : m_nSubString(nSubString), m_replacement(pReplacement) {}
    subStringReplacement(const subStringReplacement& src)
      : m_nSubString(src.m_nSubString), m_replacement(src.m_replacement) {}
    subStringReplacement& operator=(const subStringReplacement& src)
    { m_nSubString = src.m_nSubString; m_replacement = src.m_replacement;
      return *this; }
    
    void setIndex(int nSubString) {m_nSubString = nSubString;}
    int getIndex() const {return m_nSubString;}
    void setReplacementStr(const TCICHAR* pReplacement) {m_replacement = pReplacement;}
    TCIString getReplacementStr() const {return m_replacement;}
    void set(int nSubString, const TCICHAR* pReplacement)
    { m_nSubString = nSubString; m_replacement = pReplacement; }
        
  protected:
    int m_nSubString;
    TCIString m_replacement;
  };
  
  class foundMatch
  {
  public:
    foundMatch() : m_pMatches(NULL), m_nMatches(0) {}
    foundMatch(const foundMatch& src) : m_matchedString(src.m_matchedString)
      { setMatchLocations(src.m_nMatches, src.m_pMatches); }
    ~foundMatch() {clearMatchLocations();}
    
    void setMatchLocations(int nMatches, const regmatch_t* pMatches);
    
    TCIString getMatchedString() const { return m_matchedString; }
    TCIString getSubExpressionMatch(int nWhich) const;  //a 1-based query - 0 returns entire matched string
    int getNumSubMatches() const {return (m_nMatches > 0) ? m_nMatches - 1 : 0;}
    
    void setErrorCode(errorCode nError) {m_nError = nError;}
    errorCode getErrorCode() const {return m_nError;}
    
    void setStringFlags(int theFlags) {m_nStringFlags = theFlags;}
    int getStringFlags() const {return m_nStringFlags;}
    
    regmatch_t* getSubMatchesPointer() const {return m_pMatches;}
    void collateMatchData(const TCIString& targetStr);
//    TCI_BOOL replaceSubExpression(const subStringReplacement& replacement)
//    { return replaceSubExpression(replacement.getIndex(), replacement.getReplacementStr()); }
    TCI_BOOL replaceSubExpressions(int nNumReplacements, const subStringReplacement* pReplacements);
    
  protected:
    void clearMatchLocations();
    void adjustMatchPositions(regmatch_t& oldPos, regmatch_t& newPos);
    TCI_BOOL replaceSubExpression(int nWhich, const TCICHAR* pReplacement);
    TCI_BOOL replaceFirst(int nFirstIndex, int nSecondIndex) const;
  
    TCIString m_matchedString;
    int m_nMatches;
    regmatch_t* m_pMatches;
    errorCode m_nError;
    int m_nStringFlags;
  };
  
//Public static functions:

  static TCI_BOOL isBadExpressionError(MSIRegExpression::errorCode ourError);
  static TCI_BOOL isSuccessful(MSIRegExpression::errorCode ourError);

//Public functions:
  MSIRegExpression() 
    : m_pDLLStruct(0), m_nFlags(defaultMSIRegFlags), m_nError(noError) {}
  MSIRegExpression(const MSIRegExpression& src)
    : m_nFlags(src.m_nFlags), m_searchExpression(src.m_searchExpression), m_pDLLStruct(0),
      m_nError(noError) {}

  MSIRegExpression::MSIRegExpression(const TCICHAR* pRegExpression, int nFlags)
    : m_nFlags(nFlags), m_searchExpression(pRegExpression), m_pDLLStruct(0), m_nError(noError)
      {}

  MSIRegExpression::~MSIRegExpression()
  { clearExtraData(); }

  MSIRegExpression& operator=(const MSIRegExpression& src);  //force regeneration of DLL data
  
  void setSearchExpression(const TCICHAR* pRegExpression);
  TCI_BOOL doSearch(const TCIString& targetStr, foundMatch& foundInfo, int nStrFlags = defaultMSIStringFlags);
  
protected:
//Protected functions:
  errorCode prepareSearchExpression();
  void clearExtraData();
  
//Static functions:  
  static int translateFlagsForDLL(int nOurFlags);
  static int translateStringFlagsForDLL(int nOurStringFlags);
  static errorCode convertFromDLLError(int nTheirError);
  static int convertToDLLError(errorCode nError);
  static TCIString getErrorString(errorCode nError);

//Data:  
  int m_nFlags;
  TCIString m_searchExpression;
  regex_t* m_pDLLStruct;
  errorCode m_nError;
};



#endif
