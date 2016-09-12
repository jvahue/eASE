#ifndef TimeSrcObject_h
#define TimeSrcObject_h
/******************************************************************************
Copyright (C) 2014-2016 Knowlogic Software Corp.
All Rights Reserved. Proprietary and Confidential.

File:        TimeSrcObject.h

Description: This file implements the base class for simulating time in the
system. It allows for biasing time to skew clock times to check resyncs.

VERSION
$Revision: $  $Date: $

******************************************************************************/

/*****************************************************************************/
/* Compiler Specific Includes                                                */
/*****************************************************************************/
#include "Time.h"

/*****************************************************************************/
/* Software Specific Includes                                                */
/*****************************************************************************/

/*****************************************************************************/
/* Local Defines                                                             */
/*****************************************************************************/

/*****************************************************************************/
/* Local Typedefs                                                            */
/*****************************************************************************/
enum ClockSource {
    eClkRtc,
    eClkMs,
    eClkRemote,
    eClkShips,
    eClkMax
};

struct PyTimeStruct 
{
    UINT32 Year;
    UINT32 Month;
    UINT32 Day;
    UINT32 Hour;
    UINT32 Minute;
    UINT32 Seconds;
    UINT32 NextYear;
    UINT32 NextMonth;
    UINT32 NextDay;
};

class TimeSrcObj
{
public:
    TimeSrcObj();

    void Init();
    void SetTime(PyTimeStruct& time);
    void UpdateTime();

    UINT32 GetSecSinceBaseYr();
    TIMESTAMP GetTimeStamp();

    LINUX_TM_FMT m_time;
    LINUX_TM_FMT m_tomorrow;
    int m_10ms;
};


#endif
