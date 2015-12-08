#ifndef TCITIME_H
#define TCITIME_H

#ifndef CHMTYPES_H
  #include "chmtypes.h"
#endif

#include <time.h>

class TCIString;


typedef struct _SYSTEMTIME SYSTEMTIME;    // can't include Windows.h in core
typedef struct _FILETIME FILETIME;

class TCITime
{
public:

// Constructors
	static TCITime GetCurrTime();

	TCITime()
	  { }
	TCITime(time_t time)
	  { m_time = time; }
	TCITime(int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec);
	TCITime(const TCICHAR* pstr);
	TCITime(const tm* ptm);
	TCITime(U16 wDosDate, U16 wDosTime);
	TCITime(const TCITime& timeSrc)
	  { m_time = timeSrc.m_time; }

	TCITime(const SYSTEMTIME& sysTime);
	TCITime(const FILETIME& fileTime);
	const TCITime& operator=(const TCITime& timeSrc)
		{ m_time = timeSrc.m_time; return *this; }
	const TCITime& operator=(time_t t)
	  { m_time = t; return *this; }

// Attributes
	struct tm* GetGmtTm(struct tm* ptm = NULL) const;
	struct tm* GetLocalTm(struct tm* ptm = NULL) const;

	time_t GetTime() const
	  { return m_time; }
	int GetYear() const
	  { return (GetLocalTm(NULL)->tm_year) + 1900; }
	int GetMonth() const       // month of year (1 = Jan)
	  { return GetLocalTm(NULL)->tm_mon + 1; }
	int GetDay() const         // day of month
	  { return GetLocalTm(NULL)->tm_mday; }
	int GetHour() const
	  { return GetLocalTm(NULL)->tm_hour; }
	int GetMinute() const
	  { return GetLocalTm(NULL)->tm_min; }
	int GetSecond() const
	  { return GetLocalTm(NULL)->tm_sec; }
	int GetDayOfWeek() const   // 1=Sun, 2=Mon, ..., 7=Sat
	  { return GetLocalTm(NULL)->tm_wday + 1; }

	TCI_BOOL GetSYSTEMTIME(SYSTEMTIME& sysTime) const;
	TCI_BOOL GetFILETIME(FILETIME& fileTime) const;

// Operations
	// time math
	TCI_BOOL operator==(TCITime time) const
	  { return m_time == time.m_time; }
	TCI_BOOL operator!=(TCITime time) const
	  { return m_time != time.m_time; }
	TCI_BOOL operator<(TCITime time) const
	  { return m_time < time.m_time; }
	TCI_BOOL operator>(TCITime time) const
	  { return m_time > time.m_time; }
	TCI_BOOL operator<=(TCITime time) const
	  { return m_time <= time.m_time; }
	TCI_BOOL operator>=(TCITime time) const
	  { return m_time >= time.m_time; }
		
	void ResetToCurrentTime();

	// formatting using "C" strftime
	TCIString Format(const TCICHAR* pFormat) const;
	TCIString FormatGmt(const TCICHAR* pFormat) const;
		// Note: for _UNICODE variant, pFormat is still 'char'

private:
	time_t m_time;
};

 //ifndef TCITIME_HXX
#endif
