#ifndef DBUGUTIL_H
#define DBUGUTIL_H

#ifndef CHAMDEFS_H
  #include "chamdefs.h"
#endif
#ifndef CHMTYPES_H
  #include "chmtypes.h"
#endif

void MSISetSuppressAssertions(TCI_BOOL bSuppress);

#define Log_ViewPercent     "LogViewPercentSetting"
//#define Log_ViewPercentFlag

class TestingAppLogList
{
public:
  static TCICHAR* m_appLogList[];
  static TCICHAR* m_logAllString;
};


class CTestingSetLogFileHeader
{
  public:
  CTestingSetLogFileHeader(int nTotalLen, int nIdStrOffset, int nFileStrOffset, 
                         TCI_BOOL bClearLog, TCI_BOOL bCaseInsensitive)
						  : m_nTotalLen(nTotalLen), m_nIdStrOffset(nIdStrOffset), 
						    m_nFileStrOffset(nFileStrOffset), m_bClearLog(bClearLog), 
						    m_bCaseInsensitive(bCaseInsensitive) 
						    {m_idString[0] = 0;}
  CTestingSetLogFileHeader(const CTestingSetLogFileHeader& src)
                          : m_nTotalLen(src.m_nTotalLen), m_nIdStrOffset(src.m_nIdStrOffset), 
                            m_nFileStrOffset(src.m_nFileStrOffset), m_bClearLog(src.m_bClearLog),
						    m_bCaseInsensitive(src.m_bCaseInsensitive)
						    {m_idString[0] = 0;}
  CTestingSetLogFileHeader(const TCICHAR* idStr, const TCICHAR* fileStr,
                           TCI_BOOL bClearLog, TCI_BOOL bCaseInsensitive);

  int GetTotalLength() const {return m_nTotalLen;}
  int CopyToBuffer(U8* pBuffer, int& nBufferSize, 
                   const TCICHAR* idStr, const TCICHAR* fileStr) const;
  const TCICHAR* GetIdStringPointer() const;
  const TCICHAR* GetFileStringPointer() const;

  int m_nTotalLen, m_nIdStrOffset, m_nFileStrOffset;
  int m_bClearLog, m_bCaseInsensitive;
  TCICHAR m_idString[1];
};

class CTestingWriteLogFileHeader
{
  public:
  CTestingWriteLogFileHeader(int nTotalLen, int nIdStrOffset, int nDataStrOffset)
                            : m_nTotalLen(nTotalLen), m_nIdStrOffset(nIdStrOffset),
							  m_nDataStrOffset(nDataStrOffset)
						    {m_idString[0] = 0;}
  CTestingWriteLogFileHeader(const CTestingWriteLogFileHeader& src)
                            : m_nTotalLen(src.m_nTotalLen), m_nIdStrOffset(src.m_nIdStrOffset),
							  m_nDataStrOffset(src.m_nDataStrOffset)
						    {m_idString[0] = 0;}
  CTestingWriteLogFileHeader(const TCICHAR* idStr, const TCICHAR* dataStr);
                           
  int GetTotalLength() const {return m_nTotalLen;}
  int CopyToBuffer(U8* pBuffer, int& nBufferSize, const TCICHAR* idStr, 
                         const TCICHAR* dataStr) const;
  const TCICHAR* GetIdStringPointer() const;
  const TCICHAR* GetDataStringPointer() const;

  int m_nTotalLen;
  int m_nIdStrOffset;
  int m_nDataStrOffset;
  TCICHAR m_idString[1];
};

#endif
