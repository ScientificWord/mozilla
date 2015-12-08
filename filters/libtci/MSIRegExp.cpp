
#ifdef TESTING
  #undef THIS_FILE
  static char THIS_FILE[] = __FILE__;
#endif

//#include "tcistrin.h"
//#include "strutil.h"

//#include "regex/regex.h"   //external DLL's header file
#include "MSIRegExp.h"
#include "tcidebug.h"

#include <ctype.h>
//#include <stdio.h>
//#include <stdlib.h>


MSIRegExpression& MSIRegExpression::operator=(const MSIRegExpression& src)
{
  m_nFlags = src.m_nFlags;
  setSearchExpression(src.m_searchExpression);
  return *this;
}
  
void MSIRegExpression::setSearchExpression(const TCICHAR* pRegExpression)
{
  if (m_searchExpression == pRegExpression)
    return;

  clearExtraData();
  m_searchExpression = pRegExpression;
}

TCI_BOOL MSIRegExpression::doSearch(const TCIString& targetStr, MSIRegExpression::foundMatch& foundInfo,
                                     int nStrFlags )
{
  if (!m_pDLLStruct)
    prepareSearchExpression();
  if (isBadExpressionError(m_nError))
  {
    foundInfo.setErrorCode(m_nError);
    return FALSE;
  }
  foundInfo.setMatchLocations( m_pDLLStruct->re_nsub + 1, NULL );
  int nDLLFlags = translateStringFlagsForDLL(nStrFlags);
  int nErr = regexec(m_pDLLStruct, (const TCICHAR*)targetStr, m_pDLLStruct->re_nsub + 1, 
                                      foundInfo.getSubMatchesPointer(), nDLLFlags);
  errorCode ourError = convertFromDLLError(nErr);
  foundInfo.setErrorCode(ourError);
  if (isSuccessful(ourError))
  {
    foundInfo.collateMatchData(targetStr);
    return TRUE;
  }
  return FALSE;
}
  
TCI_BOOL MSIRegExpression::isBadExpressionError(MSIRegExpression::errorCode ourError)
{
  switch(ourError)
  {
    case noError:                        
    case noMatch:                        
    case outOfMemory:                
    case invalidParameter:           
    case unknownError:
      return FALSE;
    break;

    case badPattern:                 
    case invalidCollatingElement:    
    case invalidCharacterClass:      
    case escapedUnescapableCharacter:
    case invalidBackReference:       
    case unmatchedBrackets:          
    case unmatchedParens:            
    case unmatchedBraces:            
    case invalidRepetitionCounts:    
    case invalidCharacterRange:      
    case invalidRepetitionOperand:   
    case emptySubExpression:         
    default:                             
      return TRUE;
    break;
  }
}

TCI_BOOL MSIRegExpression::isSuccessful(MSIRegExpression::errorCode ourError)
{
  return (ourError == noError);
}

void MSIRegExpression::clearExtraData()
{
  if (m_pDLLStruct)
  {
    regfree(m_pDLLStruct);
    m_pDLLStruct = NULL;
  }
  m_nError = noError;
}

MSIRegExpression::errorCode MSIRegExpression::prepareSearchExpression()
{
  clearExtraData();
  if (m_searchExpression.GetLength())
  {
    m_pDLLStruct = TCI_NEW(regex_t);
    m_pDLLStruct->re_endp = NULL;
    int nError = regcomp(m_pDLLStruct, (const TCICHAR*)m_searchExpression, translateFlagsForDLL(m_nFlags));
    m_nError = MSIRegExpression::convertFromDLLError(nError);
  }
  return m_nError;
}

//Static functions on MSIRegExpression
MSIRegExpression::errorCode MSIRegExpression::convertFromDLLError(int nTheirError)
{
  errorCode rv = unknownError;
  switch(nTheirError)
  {
    case 0:               rv = noError;                        break;
    case REG_NOMATCH:     rv = noMatch;                        break;
    case REG_BADPAT:      rv = badPattern;                     break;
    case REG_ECOLLATE:    rv = invalidCollatingElement;        break;
    case REG_ECTYPE:      rv = invalidCharacterClass;          break;
    case REG_EESCAPE:     rv = escapedUnescapableCharacter;    break;
    case REG_ESUBREG:     rv = invalidBackReference;           break;
    case REG_EBRACK:      rv = unmatchedBrackets;              break;
    case REG_EPAREN:      rv = unmatchedParens;                break;
    case REG_EBRACE:      rv = unmatchedBraces;                break;
    case REG_BADBR:       rv = invalidRepetitionCounts;        break;
    case REG_ERANGE:      rv = invalidCharacterRange;          break;
    case REG_ESPACE:      rv = outOfMemory;                    break;
    case REG_BADRPT:      rv = invalidRepetitionOperand;       break;
    case REG_EMPTY:       rv = emptySubExpression;             break;
    case REG_INVARG:      rv = invalidParameter;               break;
    case REG_ASSERT:
    default:                                                   break;
  }
  return rv;
}

int MSIRegExpression::convertToDLLError(MSIRegExpression::errorCode nError)
{
  int rv = REG_ASSERT;
  switch(nError)
  {
    case noError:                        rv = 0;               break;
    case noMatch:                        rv = REG_NOMATCH;     break;
    case badPattern:                     rv = REG_BADPAT;      break;
    case invalidCollatingElement:        rv = REG_ECOLLATE;    break;
    case invalidCharacterClass:          rv = REG_ECTYPE;      break;
    case escapedUnescapableCharacter:    rv = REG_EESCAPE;     break;
    case invalidBackReference:           rv = REG_ESUBREG;     break;
    case unmatchedBrackets:              rv = REG_EBRACK;      break;
    case unmatchedParens:                rv = REG_EPAREN;      break;
    case unmatchedBraces:                rv = REG_EBRACE;      break;
    case invalidRepetitionCounts:        rv = REG_BADBR;       break;
    case invalidCharacterRange:          rv = REG_ERANGE;      break;
    case outOfMemory:                    rv = REG_ESPACE;      break;
    case invalidRepetitionOperand:       rv = REG_BADRPT;      break;
    case emptySubExpression:             rv = REG_EMPTY;       break;
    case invalidParameter:               rv = REG_INVARG;      break;
    case unknownError:
    default:                                                   break;
  }
  return rv;
}

TCIString MSIRegExpression::getErrorString(errorCode nError)
{
  TCIString rv;
  int nBuffSize = 256;
  TCICHAR* pErrorBuffer = rv.GetBuffer(nBuffSize);
  int nErrCode = convertToDLLError(nError);
  const regex_t* pRegEx = NULL;
  int nChars = regerror(nErrCode, pRegEx, pErrorBuffer, nBuffSize);
  if (nChars > nBuffSize)
  {
    rv.ReleaseBuffer();
    nBuffSize = nChars;
    pErrorBuffer = rv.GetBuffer(nBuffSize);
    nChars = regerror(nErrCode, pRegEx, pErrorBuffer, nBuffSize);
    TCI_ASSERT(nChars <= nBuffSize);
  }
  rv.ReleaseBuffer();
  return rv;
}

int MSIRegExpression::translateFlagsForDLL(int nOurFlags)
{
  int nTheirFlags = 0;
  if (nOurFlags & regExtended)
    nTheirFlags |= REG_EXTENDED;
  if (nOurFlags & regIcase)
    nTheirFlags |= REG_ICASE;
  if (nOurFlags & regNosub)
    nTheirFlags |= REG_NOSUB;
  if (nOurFlags & regNewline)
    nTheirFlags |= REG_NEWLINE;
  if (nOurFlags & regNospec)
    nTheirFlags |= REG_NOSPEC;
  if (nOurFlags & regPend)
    nTheirFlags |= REG_PEND;
  return nTheirFlags;
}

int MSIRegExpression::translateStringFlagsForDLL(int nOurStringFlags)
{
  int nTheirFlags = 0;
  if (nOurStringFlags & noEndLine)
    nTheirFlags |= REG_NOTEOL;
  if (nOurStringFlags & noStartLine)
    nTheirFlags |= REG_NOTBOL;
  if (nOurStringFlags & specifiedStartAndEnd)
    nTheirFlags |= REG_STARTEND;
  return nTheirFlags;
}

//////////////////////////////////////////////////////////////////////////////////////////
//class 

TCIString MSIRegExpression::foundMatch::getSubExpressionMatch(int nWhich) const
{
  if (nWhich > m_nMatches || (m_pMatches[nWhich].rm_eo == m_pMatches[nWhich].rm_so))
    return EmptyTCIString;
  if (nWhich == 0)
    return m_matchedString;
  return m_matchedString.Mid( m_pMatches[nWhich].rm_so, m_pMatches[nWhich].rm_eo - m_pMatches[nWhich].rm_so );
}
 
TCI_BOOL MSIRegExpression::foundMatch::replaceSubExpression(int nWhich, const TCICHAR* pReplacement)
{
  if (nWhich > m_nMatches || m_pMatches[nWhich].rm_so == 0xffffffff)
    return FALSE;
  TCI_BOOL rv = TRUE;
  int nCurrLength = m_pMatches[nWhich].rm_eo - m_pMatches[nWhich].rm_so;
  int nNewLength = strlen(pReplacement);
  TCIString newStr(m_matchedString.Left(m_pMatches[nWhich].rm_so));
  newStr += pReplacement;
  newStr += m_matchedString.Mid(m_pMatches[nWhich].rm_eo);
  regmatch_t oldMatchPos = m_pMatches[nWhich];
  regmatch_t newMatchPos = m_pMatches[nWhich];
  newMatchPos.rm_eo = m_pMatches[nWhich].rm_so + nNewLength;
  adjustMatchPositions(oldMatchPos, newMatchPos);
  m_matchedString = newStr;
  return TRUE;
}

TCI_BOOL MSIRegExpression::foundMatch::replaceFirst(int nFirstIndex, int nSecondIndex) const
{
  if (m_pMatches[nFirstIndex].rm_so == 0xffffffff)
    return FALSE;
  if (m_pMatches[nSecondIndex].rm_so == 0xffffffff)
    return TRUE;
  return ( (m_pMatches[nFirstIndex].rm_so > m_pMatches[nSecondIndex].rm_so)
           || 
           ((m_pMatches[nFirstIndex].rm_so == m_pMatches[nSecondIndex].rm_so) 
             && (m_pMatches[nFirstIndex].rm_eo < m_pMatches[nSecondIndex].rm_eo)) );
           //yes, the last '<' is correct, though it looks wrong.
}

TCI_BOOL MSIRegExpression::foundMatch::replaceSubExpressions(int nNumReplacements, const MSIRegExpression::subStringReplacement* pReplacements)
{
  TCI_BOOL rv = TRUE;
  const MSIRegExpression::subStringReplacement** pReplacementPtrs = TCI_NEW(const MSIRegExpression::subStringReplacement*[nNumReplacements]);
  for (int ix = 0; ix < nNumReplacements; ++ix)
    pReplacementPtrs[ix] = &(pReplacements[ix]);
  for (int ix = nNumReplacements - 1; ix >= 0; --ix)
  {
    int nDoFirst = nNumReplacements - 1;
    for (int jx = nNumReplacements - 1; jx >= 0; --jx)
    {
      if (!pReplacementPtrs[jx] || jx == nDoFirst)
        continue;
      if ( !pReplacementPtrs[nDoFirst] || replaceFirst(pReplacementPtrs[jx]->getIndex(), pReplacementPtrs[nDoFirst]->getIndex()) )
        nDoFirst = jx;
    }
    if (!pReplacementPtrs[nDoFirst])
    {
      TCI_ASSERT(FALSE);
      continue;
    }
    rv = replaceSubExpression(pReplacementPtrs[nDoFirst]->getIndex(), pReplacementPtrs[nDoFirst]->getReplacementStr()) && rv;
    pReplacementPtrs[nDoFirst] = 0;
  }
  delete pReplacementPtrs;
  return rv;
}


void MSIRegExpression::foundMatch::adjustMatchPositions(regmatch_t& oldPos, regmatch_t& newPos)
{
  int nNewLength = newPos.rm_eo - newPos.rm_so;
  int nOldLength = oldPos.rm_eo - oldPos.rm_so;
  for (int ix = 1; ix < m_nMatches; ++ix)  
  //don't adjust m_pMatches[0]; this holds our original position relative to the original string
  {
    if (m_pMatches[ix].rm_so == 0xffffffff)
      continue;  //don't adjust non-matches
    if (m_pMatches[ix].rm_so >= oldPos.rm_eo)
    {
      m_pMatches[ix].rm_so += nNewLength - nOldLength;
      m_pMatches[ix].rm_eo += nNewLength - nOldLength;
    }
    else if (m_pMatches[ix].rm_eo >= oldPos.rm_eo && m_pMatches[ix].rm_so <= oldPos.rm_so)
    {
      m_pMatches[ix].rm_eo += nNewLength - nOldLength;
    }
    else if (m_pMatches[ix].rm_eo <= oldPos.rm_eo && m_pMatches[ix].rm_so >= oldPos.rm_so)
    {
      m_pMatches[ix].rm_so = m_pMatches[ix].rm_eo = -1;  //was internal to this one; don't try to guess
    }
    else
      TCI_ASSERT(m_pMatches[ix].rm_eo <= oldPos.rm_so);
  }
}

void MSIRegExpression::foundMatch::clearMatchLocations()
{
  delete[] m_pMatches;
  m_pMatches = 0;
  m_nMatches = 0;
}

void MSIRegExpression::foundMatch::setMatchLocations(int nMatches, const regmatch_t* pMatches)
{
  clearMatchLocations();
  m_nMatches = nMatches;
  if (!nMatches)
    return;
  m_pMatches = TCI_NEW(regmatch_t[nMatches]);
  for (int ix = 0; ix < nMatches; ++ix)
  {
    if (pMatches)
    {
      m_pMatches[ix] = pMatches[ix];
    }
    else
      m_pMatches[ix].rm_eo = m_pMatches[ix].rm_so = -1;
  }
}

void MSIRegExpression::foundMatch::collateMatchData(const TCIString& targStr)
{
  for (int ix = 1; ix < m_nMatches; ++ix)
  {
    m_pMatches[ix].rm_so -= m_pMatches[0].rm_so;
    m_pMatches[ix].rm_eo -= m_pMatches[0].rm_so;
  }
  m_matchedString = targStr.Mid(m_pMatches[0].rm_so, m_pMatches[0].rm_eo - m_pMatches[0].rm_so);
}
