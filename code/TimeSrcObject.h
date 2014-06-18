#ifndef TimeSrcObject_h
#define TimeSrcObject_h
/******************************************************************************
Copyright (C) 2014 Knowlogic Software Corp.
All Rights Reserved. Proprietary and Confidential.

File:        ccdl.h

Description: This file implements the base class for simulating the Cross
channel Data Link (CCDL)

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
