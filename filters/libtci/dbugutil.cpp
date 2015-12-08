#include "dbugutil.h"
#include "TCI_new.h"

#include <string.h>

TCI_BOOL bSuppressAssertions = FALSE;

void MSISetSuppressAssertions(TCI_BOOL bSuppress)
{ bSuppressAssertions = bSuppress; }


TCICHAR* TestingAppLogList::m_appLogList[] = {
             Log_ViewPercent,
             ""};


TCICHAR* TestingAppLogList::m_logAllString = "All";

CTestingSetLogFileHeader::CTestingSetLogFileHeader(const TCICHAR* idStr, 
                                  const TCICHAR* fileStr, TCI_BOOL bClearLog, 
                                  TCI_BOOL bCaseInsensitive)
						: m_bClearLog(bClearLog), m_bCaseInsensitive(bCaseInsensitive)
{
  m_idString[0] = 0;
  m_nIdStrOffset = (U8*)m_idString - (U8*)this;
  int nStrLen = idStr ? strlen(idStr) : 0;
  m_nFileStrOffset = m_nIdStrOffset + (nStrLen + 1) * sizeof(TCICHAR);
  nStrLen = fileStr ? strlen(fileStr) : 0;
  m_nTotalLen = m_nFileStrOffset + (nStrLen + 1) * sizeof(TCICHAR);
}


int CTestingSetLogFileHeader::CopyToBuffer(U8* pBuffer, int& nBufferSize, 
                                    const TCICHAR* idStr, const TCICHAR* fileStr) const
{
  *((CTestingSetLogFileHeader*)pBuffer) = *this;
  if (m_nTotalLen > nBufferSize)
  {
	nBufferSize = m_nTotalLen;
	return 0;
  }
  int nStrLen = (m_nFileStrOffset - m_nIdStrOffset) / sizeof(TCICHAR);
    strncpy((TCICHAR*)(pBuffer + m_nIdStrOffset), idStr, nStrLen-1);
  nStrLen = (m_nTotalLen - m_nFileStrOffset) / sizeof(TCICHAR);
//  if (nStrLen > 1)
    strncpy((TCICHAR*)(pBuffer + m_nFileStrOffset), fileStr, nStrLen-1);
//  else
//    *((TCICHAR*)(pBuffer + pBuffer->m_nFileStrOffset)) = 0;
  return m_nTotalLen;
}

const TCICHAR* CTestingSetLogFileHeader::GetIdStringPointer() const
{
  return (this==NULL || m_idString[0]==0) ? NULL : (TCICHAR*)((U8*)this + m_nIdStrOffset);
}

const TCICHAR* CTestingSetLogFileHeader::GetFileStringPointer() const
{
  if (!this)
    return NULL;
  const TCICHAR* pFileStr = (TCICHAR*)((U8*)this + m_nFileStrOffset);
  if (m_idString[0] == 0 || *pFileStr == 0)
    return NULL;
  return pFileStr;
}

CTestingWriteLogFileHeader::CTestingWriteLogFileHeader(const TCICHAR* idStr, 
                                                       const TCICHAR* dataStr)
{
  m_nIdStrOffset = (U8*)m_idString - (U8*)this;
  m_nDataStrOffset = m_nIdStrOffset + (strlen(idStr) + 1) * sizeof(TCICHAR);
  m_nTotalLen = m_nDataStrOffset + (strlen(dataStr) + 1) * sizeof(TCICHAR);
  m_idString[0] = 0;
}						    

int CTestingWriteLogFileHeader::CopyToBuffer(U8* pBuffer, int& nBufferSize, 
                                    const TCICHAR* idStr, const TCICHAR* dataStr) const
{
  *((CTestingWriteLogFileHeader*)pBuffer) = *this;
  if (m_nTotalLen > nBufferSize)
  {
	nBufferSize = m_nTotalLen;
	return 0;
  }
  int nStrLen = (m_nDataStrOffset - m_nIdStrOffset) / sizeof(TCICHAR);
    strncpy((TCICHAR*)(pBuffer + m_nIdStrOffset), idStr, nStrLen-1);
  nStrLen = (m_nTotalLen - m_nDataStrOffset) / sizeof(TCICHAR);
//  if (nStrLen > 1)
    strncpy((TCICHAR*)(pBuffer + m_nDataStrOffset), dataStr, nStrLen-1);
//  else
//    *((TCICHAR*)(pBuffer + pBuffer->m_nFileStrOffset)) = 0;
  return m_nTotalLen;
}

const TCICHAR* CTestingWriteLogFileHeader::GetIdStringPointer() const
{
  return (this==NULL || m_idString[0]==0) ? NULL : (TCICHAR*)((U8*)this + m_nIdStrOffset);
}

const TCICHAR* CTestingWriteLogFileHeader::GetDataStringPointer() const
{
  if (!this)
    return NULL;
  const TCICHAR* pDataStr = (TCICHAR*)((U8*)this + m_nDataStrOffset);
  if (m_idString[0] == 0 || *pDataStr == 0)
    return NULL;
  return pDataStr;
}


