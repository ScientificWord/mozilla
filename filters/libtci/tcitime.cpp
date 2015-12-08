
#include "tcitime.h"
#include "strutil.h"
#include "tcistrin.h"

#include <string.h>
#include <stdio.h>

#define NOGDICAPMASKS     
#define NOVIRTUALKEYCODES 
#define NOWINMESSAGES     
#define NOWINSTYLES       
#define NOSYSMETRICS      
#define NOMENUS           
#define NOICONS           
#define NOKEYSTATES       
#define NOSYSCOMMANDS     
#define NORASTEROPS       
#define NOSHOWWINDOW      
#define OEMRESOURCE       
#define NOATOM            
#define NOCLIPBOARD       
#define NOCOLOR           
#define NOCTLMGR          
#define NODRAWTEXT        
#define NOGDI             
#define NOKERNEL          
#define NOUSER            
#define NONLS             
#define NOMB              
#define NOMEMMGR          
#define NOMETAFILE        
#define NOMINMAX          
#define NOMSG             
#define NOOPENFILE        
#define NOSCROLL          
#define NOSERVICE         
#define NOSOUND           
#define NOTEXTMETRIC      
#define NOWH              
#define NOWINOFFSETS      
#define NOCOMM            
#define NOKANJI           
#define NOHELP            
#define NOPROFILER        
#define NODEFERWINDOWPOS  
#define NOMCX             
#include <windows.h>

//
// TCITime -- local version of MFC CTime
//
// The archive functions were removed and an additional constructor
// from a char string were added.
//


#ifdef TESTING
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define _tcsftime strftime

// do we have a localization problem here?
static const char* MonthsOfYear[] = {
  "Jan",
	"Feb",
	"Mar",
	"Apr",
	"May",
	"Jun",
	"Jul",
	"Aug",
	"Sep",
	"Oct",
	"Nov",
	"Dec"
};


/////////////////////////////////////////////////////////////////////////////
// TCITime - absolute time

TCITime::TCITime(int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec)
{
	struct tm atm;
	atm.tm_sec = nSec;
	atm.tm_min = nMin;
	atm.tm_hour = nHour;
	TCI_ASSERT(nDay >= 1 && nDay <= 31);
	atm.tm_mday = nDay;
	TCI_ASSERT(nMonth >= 1 && nMonth <= 12);
	atm.tm_mon = nMonth - 1;        // tm_mon is 0 based
	TCI_ASSERT(nYear >= 1900);
	atm.tm_year = nYear - 1900;     // tm_year is 1900 based
	atm.tm_isdst = -1;
	m_time = mktime(&atm);
	TCI_ASSERT(m_time != -1);       // indicates an illegal input time
}

TCITime::TCITime(const tm *ptm)
{
  m_time = mktime((tm *)ptm);
  TCI_ASSERT(m_time != -1);
}

// Read ASCII string of date and time in same format as ASCTIME (and CTIME)
// string contains exactly 26 characters and has the form
//    "Wed Jan 02 02:03:55 1980\n\0"

TCITime::TCITime(const TCICHAR* pstr)
{
  int nYear, nMonth, nDay, nHour, nMin, nSec;
	int cnt;
	TCICHAR sMonth[4], sDay[4];
	
	cnt = sscanf( pstr, "%3s %3s %2d %2d:%2d:%2d %4d",
	                  sDay, sMonth, &nDay, &nHour, &nMin, &nSec, &nYear );
	if (cnt < 7) {
	  TCI_ASSERT(FALSE);
		m_time = -1;
	} else {
	  TCI_ASSERT(sMonth[3]=='\0');
		nMonth = 0;
		for (int i=0; i < 12; ++i) {
		  if (stricmp( sMonth, MonthsOfYear[i] ) == 0) {
			  nMonth = i+1;
				break;
		  } 
		}
		// throw away sDay since already have nDay
		TCI_ASSERT(nMonth);
		if (nMonth == 0)
		    m_time = -1;
		else {
		  TCITime timeT( nYear, nMonth, nDay, nHour, nMin, nSec);
			*this = timeT;
		}	
	}		
}

TCITime::TCITime(U16 wDosDate, U16 wDosTime)
{
	struct tm atm;
	atm.tm_sec = (wDosTime & ~0xFFE0) << 1;;
	atm.tm_min = (wDosTime & ~0xF800) >> 5;
	atm.tm_hour = wDosTime >> 11;

	atm.tm_mday = wDosDate & ~0xFFE0;
	atm.tm_mon = ((wDosDate & ~0xFE00) >> 5) - 1;
	atm.tm_year = (wDosDate >> 9) + 80;
	atm.tm_isdst = -1;
	m_time = mktime(&atm);
	TCI_ASSERT(m_time != -1);       // indicates an illegal input time
}

TCITime::TCITime(const SYSTEMTIME& sysTime)
{
	if (sysTime.wYear < 1900)
	{
		time_t time0 = 0L;
		TCITime timeT(time0);
		*this = timeT;
	}
	else
	{
		TCITime timeT(
			(int)sysTime.wYear, (int)sysTime.wMonth,  (int)sysTime.wDay,
			(int)sysTime.wHour, (int)sysTime.wMinute, (int)sysTime.wSecond);
		*this = timeT;
	}
}

TCITime::TCITime(const FILETIME& fileTime)
{
	// first convert file time (UTC time) to local time
	FILETIME localTime;
	TCI_VERIFY(FileTimeToLocalFileTime(&fileTime, &localTime));

	SYSTEMTIME sysTime;
	TCI_VERIFY(FileTimeToSystemTime(&localTime, &sysTime));

	// then convert the system time to a time_t (C-runtime UTC time)
	TCITime timeT(sysTime);
	*this = timeT;
}

TCITime TCITime::GetCurrTime()
// return the current system time
{
  return TCITime(::time(NULL));
}

void TCITime::ResetToCurrentTime()
// reset to the current system time
{
	m_time = ::time(NULL);
}

struct tm* TCITime::GetGmtTm(struct tm* ptm) const
{
	if (ptm != NULL)
	{
		*ptm = *gmtime(&m_time);
		return ptm;
	}
	else
		return gmtime(&m_time);
}

struct tm* TCITime::GetLocalTm(struct tm* ptm) const
{
	if (ptm != NULL)
	{
		struct tm* ptmTemp = localtime(&m_time);
		if (ptmTemp == NULL)
			return NULL;    // indicates the m_time was not initialized!

		*ptm = *ptmTemp;
		return ptm;
	}
	else
		return localtime(&m_time);
}

TCI_BOOL TCITime::GetSYSTEMTIME(SYSTEMTIME& sysTime) const
{
  struct tm *ptm = GetGmtTm(NULL);
  sysTime.wYear      = ptm->tm_year + 1900;
  sysTime.wMonth     = ptm->tm_mon + 1;
  sysTime.wDayOfWeek = ptm->tm_wday;
  sysTime.wDay       = ptm->tm_mday;
  sysTime.wHour      = ptm->tm_hour;
  sysTime.wMinute    = ptm->tm_min;
  sysTime.wSecond    = ptm->tm_sec;
  sysTime.wMilliseconds = 0;
  return TRUE;  // can always convert
}

TCI_BOOL TCITime::GetFILETIME(FILETIME& fileTime) const
{
  SYSTEMTIME stime;
  GetSYSTEMTIME(stime);
//sls - Why not?
//  FILETIME utcftime;
//  TCI_VERIFY(SystemTimeToFileTime( &stime,&utcftime ));
//  return FileTimeToLocalFileTime( &utcftime, &fileTime );
  return SystemTimeToFileTime(&stime,&fileTime) ? TRUE  : FALSE;
}


/////////////////////////////////////////////////////////////////////////////
// String formatting

#define maxTimeBufferSize       128
	// Verifies will fail if the needed buffer size is too large

TCIString TCITime::Format(const char* pFormat) const
{
	TCICHAR    szBuffer[maxTimeBufferSize];

	struct tm* ptmTemp = localtime(&m_time);
	TCI_ASSERT(ptmTemp != NULL); // make sure the time has been initialized!
	if (!_tcsftime(szBuffer, maxTimeBufferSize, pFormat, ptmTemp))
		szBuffer[0] = '\0';
	return szBuffer;
}

TCIString TCITime::FormatGmt(const char* pFormat) const
{
	TCHAR    szBuffer[maxTimeBufferSize];

	if (!_tcsftime(szBuffer, maxTimeBufferSize, pFormat, gmtime(&m_time)))
		szBuffer[0] = '\0';
	return szBuffer;
}

/////////////////////////////////////////////////////////////////////////////
