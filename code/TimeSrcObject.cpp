#include <string.h>

#include "TimeSrcObject.h"

//Leap years:
//Every fourth years are leap-years in most of the cases.
//Years divisible with 100 aren't leap-years but years
//divisible with 400 are leap-years.
#define IS_LEAP_YEAR(Yr) ( ((Yr & 0x3) != 0) ? FALSE:\
    ((Yr % 100) != 0) ? TRUE : \
    ((Yr % 400) != 0) ? FALSE: TRUE )

#define CM_DAYS_IN_YEAR_NO_LEAP    365          // Ignoring leap year
#define CM_DAYS_IN_YEAR_WITH_LEAP  365.242199f  // Including leap year

#define CM_SEC_IN_MIN  60
#define CM_MIN_IN_HR   60
#define CM_HR_IN_DAY   24

#define CM_SEC_IN_DAY  (CM_SEC_IN_MIN * CM_MIN_IN_HR * CM_HR_IN_DAY)
#define CM_SEC_IN_HR   3600

#define CM_SEC_IN_NON_LEAP_YR (CM_SEC_IN_DAY * CM_DAYS_IN_YEAR_NO_LEAP)


static const UINT8    DaysPerMonthNoLeap [] =    {31, //Jan
28, //Feb
31, //Mar
30, //Apr
31, //May
30, //Jun
31, //Jul
31, //Aug
30, //Sep
31, //Oct
30, //Nov
31};//Dec

static const UINT8   DaysPerMonthLeap [] = {31, //Jan
29, //Feb
31, //Mar
30, //Apr
31, //May
30, //Jun
31, //Jul
31, //Aug
30, //Sep
31, //Oct
30, //Nov
31};//Dec


TimeSrcObj::TimeSrcObj()
{
    Init();
}

void TimeSrcObj::Init()
{
    memset(&m_time, 0, sizeof(m_time));
    m_time.tm_year    = 2013; 
    m_time.tm_mon     = 7; 
    m_time.tm_mday    = 26; 

    memset(&m_tomorrow, 0, sizeof(m_tomorrow));
    m_tomorrow.tm_year  = 2013; 
    m_tomorrow.tm_mon   = 7; 
    m_tomorrow.tm_mday  = 27; 

    m_10ms = 0;
}

void TimeSrcObj::SetTime( PyTimeStruct& timeRep)
{
    m_time.tm_year    = timeRep.Year     ; 
    m_time.tm_mon     = timeRep.Month    ; 
    m_time.tm_mday    = timeRep.Day      ; 
    m_time.tm_hour    = timeRep.Hour     ; 
    m_time.tm_min     = timeRep.Minute   ; 
    m_time.tm_sec     = timeRep.Seconds  ;

    m_tomorrow.tm_year  = timeRep.NextYear ; 
    m_tomorrow.tm_mon   = timeRep.NextMonth; 
    m_tomorrow.tm_mday  = timeRep.NextDay  ; 

    m_10ms = 0;
}

void TimeSrcObj::UpdateTime()
{
    // keep time
    m_10ms += 10;
    if (m_10ms >= 1000)
    {
        m_10ms = 0;
        m_time.tm_sec += 1;
        if (m_time.tm_sec >= 60)
        {
            m_time.tm_sec = 0;
            m_time.tm_min += 1;
            if (m_time.tm_min >= 60)
            {
                m_time.tm_min = 0;
                m_time.tm_hour += 1;
                if (m_time.tm_hour >= 24)
                {
                    m_time.tm_hour = 0;
                    m_time.tm_year = m_tomorrow.tm_year;
                    m_time.tm_mon = m_tomorrow.tm_mon;
                    m_time.tm_mday = m_tomorrow.tm_mday;
                }
            }
        }
    }
}

//---------------------------------------------------------------------------------------------
// Pack our current time into a DateTime structure
TIMESTAMP TimeSrcObj::GetTimeStamp()
{
    TIMESTAMP  Ts;

    Ts.Timestamp =
        ((UINT32) ((m_time.tm_sec & 0x3F)))        +
        ((UINT32) ((m_time.tm_min & 0x3F))  << 6)  +
        ((UINT32) ((m_time.tm_mon  & 0x0F)) << 12) +
        ((UINT32) ((m_time.tm_mday & 0x1F)) << 16) +
        ((UINT32) ((m_time.tm_year - BASE_YEAR) * 24 + m_time.tm_hour) << 21);

    Ts.MSecond = m_10ms;

    return (Ts);
}   

//---------------------------------------------------------------------------------------------
UINT32 TimeSrcObj::GetSecSinceBaseYr()
{
    UINT16 i;
    UINT32 sec;
    UINT32 itemp;
    SINT16 year;
    UINT8 *pDayPerMonth;

    // Determine years since 1997 in sec
    // Note: Year will be > 1997 as year in ts format is years since Base Year !
    // Note: Leap yr calc not compliant to /4 and /100 is not a leap yr unless also
    //       /400.  Leap yr calc would fail for 2100.
    // Fix leap year issue
    itemp = 0;
    if ( m_time.tm_year >= 2000 )
    {
        // Determine # leap days, from last leap year of 2000+.
        year = m_time.tm_year - 2000;
        for (i = 1; i < (MAX_YEAR - BASE_YEAR); i++)
        {
            year -= 4;
            if (year <= 0) {
                break;
            }
        }
        itemp = i;
        // itemp = ((time_struct.Year - 2000) / 4);
    }
    // else date between 1997 and 2000, no leap yr calc required
    itemp = (CM_DAYS_IN_YEAR_NO_LEAP * (m_time.tm_year - BASE_YEAR)) + itemp;
    sec = itemp * CM_SEC_IN_DAY;

    // Determine # months for current year in sec.  Acct for leap year
    if ( (((m_time.tm_year - BASE_YEAR)+1)%4) == 0 )
    {
        // Leap Year
        pDayPerMonth = (UINT8 *) &DaysPerMonthLeap;
    }
    else
    {
        // Non Leap Year
        pDayPerMonth = (UINT8 *) &DaysPerMonthNoLeap;
    }

    itemp = 0;
    for (i=0; i < (m_time.tm_mon - 1); i++) // Sub "1" because months starts on 1 not 0.
    {
        itemp += *pDayPerMonth;  // Add up all the seconds
        pDayPerMonth++;          // Point to next month
    }
    sec += (itemp * CM_SEC_IN_DAY);

    // Determine # days for current year in sec
    sec += ( (m_time.tm_mday - 1) * CM_SEC_IN_DAY); // Sub "1" because days start on 1 not 0.

    // Determine # hours in sec
    sec += (m_time.tm_hour * CM_SEC_IN_MIN * CM_MIN_IN_HR);

    // Determine # min in sec
    sec += (m_time.tm_min * CM_SEC_IN_MIN);

    // Add in current sec
    sec += m_time.tm_sec;

    return sec;
}
