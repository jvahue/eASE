//-----------------------------------------------------------------------------
//          Copyright (C) 2014-2016 Knowlogic Software Corp.
//         All Rights Reserved. Proprietary and Confidential.
//
//    File: ioiStatic.cpp
//
//    Description: Implements the Static IOI processing
//
//-----------------------------------------------------------------------------


//----------------------------------------------------------------------------/
// Compiler Specific Includes                                                -/
//----------------------------------------------------------------------------/
#include <stdio.h>
#include <string.h>

//----------------------------------------------------------------------------/
// Software Specific Includes                                                -/
//----------------------------------------------------------------------------/
#include "AseCommon.h"

#include "ioiStatic.h"

//----------------------------------------------------------------------------/
// Local Defines                                                             -/
//----------------------------------------------------------------------------/

//----------------------------------------------------------------------------/
// Local Typedefs                                                            -/
//----------------------------------------------------------------------------/

//----------------------------------------------------------------------------/
// Local Variables                                                           -/
//----------------------------------------------------------------------------/

//----------------------------------------------------------------------------/
// Constant Data                                                             -/
//----------------------------------------------------------------------------/
#include "AseStaticIoiInfo.cpp"

#if MAX_STATIC_IOI <= ASE_IN_MAX
#error Need to Increase MAX_STATIC_IOI to be greater than ASE_IN_MAX
#endif

#if MAX_STATIC_IOI <= ASE_OUT_MAX
#error Need to Increase MAX_STATIC_IOI to be greater than ASE_OUT_MAX
#endif

//----------------------------------------------------------------------------/
// Local Function Prototypes                                                 -/
//----------------------------------------------------------------------------/

//----------------------------------------------------------------------------/
// Public Functions                                                          -/
//----------------------------------------------------------------------------/

//----------------------------------------------------------------------------/
// Class Definitions                                                         -/
//----------------------------------------------------------------------------/
StaticIoiObj::StaticIoiObj(char* name, bool isInput)
: ioiChan(0)
, ioiValid(false)
, ioiRunning(true)
, ioiIsInput(isInput)
, m_updateCount(0)
{
    strcpy(ioiName, name);
    strcpy(m_shortName, name);
    CompressName(m_shortName, 18);
}

//---------------------------------------------------------------------------------------------
bool StaticIoiObj::OpenIoi()
{
    ioiStatus openStatus;

    if (ioiIsInput)
    {
        openStatus = ioi_open(ioiName, ioiReadPermission, (int*)&ioiChan);
    }
    else
    {
        openStatus = ioi_open(ioiName, ioiWritePermission, (int*)&ioiChan);
    }
    ioiValid = openStatus == ioiSuccess;
    ioiRunning = ioiValid;
    return ioiValid;
}

//---------------------------------------------------------------------------------------------
// Return the status of an actual IOI write.  For params we skip just return success
bool StaticIoiObj::WriteStaticIoi(void* data)
{
    ioiStatus writeStatus = ioiSuccess;

    if (ioiValid && ioiRunning)
    {
        writeStatus = ioi_write(ioiChan, data);
    }

    m_updateCount += (ioiValid && ioiRunning && writeStatus == ioiSuccess) ? 1 : 0;
    return writeStatus == ioiSuccess;
}

//---------------------------------------------------------------------------------------------
// Return the status of an actual IOI write.  For params we skip just return success
bool StaticIoiObj::ReadStaticIoi(void* data)
{
    ioiStatus readStatus = ioiSuccess;

    if (ioiValid && ioiRunning)
    {
        readStatus = ioi_read(ioiChan, data);
    }

    m_updateCount += (ioiValid && ioiRunning && readStatus == ioiSuccess) ? 1 : 0;
    return readStatus == ioiSuccess;
}

//---------------------------------------------------------------------------------------------
char* StaticIoiObj::Display( char* dest, UINT32 dix )
{
    dest = '\0';
    return dest;
}

//---------------------------------------------------------------------------------------------
char* StaticIoiObj::CompressName(char* src, int size)
{
    char* vowel;
    char* from;
    char* to;
    int at = strlen(src) - 1;

    // remove vowels from the back until less than 24 chars long
    while (strlen(src) > size && at >= 0)
    {
        vowel = strpbrk(&src[at], "aeiouAEIOU");
        if (vowel != NULL)
        {
            to = vowel;
            from = ++vowel;
            while (*from != NULL && from != &src[at])
            {
                *to++ = *from++;
            }
            *to = '\0';
        }
        at -= 1;
    }

    if (strlen(src) > size)
    {
        src[size] = '\0';
    }

    return src;
}

//---------------------------------------------------------------------------------------------
void StaticIoiObj::SetRunState(bool newState)
{
    ioiRunning = newState;
}

//=============================================================================================
//IocResponse StaticIoiByte::GetStaticIoiData()
//{
//
//}

//---------------------------------------------------------------------------------------------
bool StaticIoiByte::SetStaticIoiData( SecRequest& request )
{
    data = (unsigned char)request.resetRequest;
    ioiRunning = ioiValid;
    Update();
    return true;
}

//---------------------------------------------------------------------------------------------
char* StaticIoiByte::Display( char* dest, UINT32 dix )
{
    if (ioiRunning)
    {
        sprintf(dest, "%2d:%s: 0x%02x", dix, m_shortName, data);
    }
    else
    {
        sprintf(dest, "xx:%s: 0x%02x", dix, m_shortName, data);
    }
    return dest;
}

//---------------------------------------------------------------------------------------------
bool StaticIoiByte::GetStaticIoiData(IocResponse& m_response)
{
    m_response.streamSize = data;
    return true;
}

bool StaticIoiByte::Update()
{
    if (ioiIsInput)
    {
        return ReadStaticIoi(&data);
    }
    else
    {
        return WriteStaticIoi(&data);
    }
}

//=============================================================================================
//IocResponse StaticIoiInt::GetStaticIoiData()
//{
//
//}

//---------------------------------------------------------------------------------------------
bool StaticIoiInt::SetStaticIoiData( SecRequest& request )
{
    data = request.resetRequest;
    ioiRunning = ioiValid;
    Update();
    return true;
}

//---------------------------------------------------------------------------------------------
char* StaticIoiInt::Display( char* dest, UINT32 dix )
{
    if ( ioiRunning)
    {
        sprintf(dest, "%2d:%s: 0x%08x", dix, m_shortName, data);
    }
    else
    {
        sprintf(dest, "xx:%s: 0x%08x", m_shortName, data);

    }
    return dest;
}

//---------------------------------------------------------------------------------------------
bool StaticIoiInt::Update()
{
    if (ioiIsInput)
    {
        return ReadStaticIoi(&data);
    }
    else
    {
        return WriteStaticIoi(&data);
    }
}

//---------------------------------------------------------------------------------------------
bool StaticIoiInt::GetStaticIoiData(IocResponse& m_response)
{
    m_response.streamSize = data;
    return true;
}

//=============================================================================================
//IocResponse StaticIoiFloat::GetStaticIoiData()
//{
//
//}

//---------------------------------------------------------------------------------------------
bool StaticIoiFloat::SetStaticIoiData( SecRequest& request )
{
    data = request.value;
    ioiRunning = ioiValid;
    Update();
    return true;
}

//---------------------------------------------------------------------------------------------
char* StaticIoiFloat::Display( char* dest, UINT32 dix )
{
    if ( ioiRunning)
    {
        sprintf(dest, "%2d:%s: %f", dix, m_shortName, data);
    }
    else
    {
        sprintf(dest, "xx:%s: %f", m_shortName, data);
    }
    return dest;
}

//---------------------------------------------------------------------------------------------
bool StaticIoiFloat::Update()
{
    if (ioiIsInput)
    {
        return ReadStaticIoi(&data);
    }
    else
    {
        return WriteStaticIoi(&data);
    }
}

//---------------------------------------------------------------------------------------------
bool StaticIoiFloat::GetStaticIoiData(IocResponse& m_response)
{
    m_response.value = data;
    return true;
}

//=============================================================================================
//IocResponse StaticIoiStr::GetStaticIoiData()
//{
//
//}

//---------------------------------------------------------------------------------------------
bool StaticIoiStr::SetStaticIoiData( SecRequest& request )
{
    strncpy(data, request.charData, request.charDataSize);
    data[request.charDataSize] = '\0';
    ioiRunning = ioiValid;
    Update();
    return true;
}

//---------------------------------------------------------------------------------------------
char* StaticIoiStr::Display( char* dest, UINT32 dix )
{
    if (ioiRunning)
    {
        sprintf(dest, "%2d:%s: %s", dix, m_shortName, data);
    }
    else
    {
        sprintf(dest, "xx:%s: %s", m_shortName, data);
    }
    return dest;
}

//---------------------------------------------------------------------------------------------
bool StaticIoiStr::Update()
{
    if (ioiIsInput)
    {
        return ReadStaticIoi(data);
    }
    else
    {
        return WriteStaticIoi(data);
    }
}

//---------------------------------------------------------------------------------------------
bool StaticIoiStr::GetStaticIoiData( IocResponse& m_response)
{
    strncpy(m_response.streamData, data, bytes);
    m_response.streamSize = bytes;
    return true;
}

//---------------------------------------------------------------------------------------------
bool StaticIoiIntPtr::Update()
{
    if (ioiIsInput)
    {
        return ReadStaticIoi(data);
    }
    else
    {
        return WriteStaticIoi(data);
    }
}

//---------------------------------------------------------------------------------------------
char* StaticIoiIntPtr::Display( char* dest, UINT32 dix )
{
    sprintf(dest, "%2d:?? Integers", dix);
    return dest;
}

//---------------------------------------------------------------------------------------------
bool StaticIoiIntPtr::GetStaticIoiData(IocResponse& m_response)
{
    memcpy(m_response.streamData, data, bytes);
    m_response.streamSize = bytes;
    return true;
}

StaticIoiIntPtr::StaticIoiIntPtr( char* name, int* value, int size, bool isInput)
: StaticIoiObj(name, isInput)
{
    data = value;
    bytes = size * sizeof(int);
}
//=============================================================================================
StaticIoiContainer::StaticIoiContainer()
: m_ioiStaticOutCount(0)
, m_ioiStaticInCount(0)
, m_updateIndex(0)
, m_validIoiOut(0)
, m_validIoiIn(0)
, m_readError(0)
, m_writeError(0)
{
    strcpy(_HMUSerialNumber, "0000999999");
    strcpy(_HMUPartNumber, "5316928SK01");
    strcpy(_UTASSwDwgNumber, "PY1022429-028");
    strcpy(_PWSwDwgNumber, "5318410-12SK01");

    // copy the object references into our container array (TBD: do we really need to do this?
    for (int i = 0; i < ASE_OUT_MAX; ++i)
    {
        m_staticIoiOut[i] = aseIoiOut[i];
    }

    m_ioiStaticOutCount = ASE_OUT_MAX;
    m_updateIndex = 0;
    m_validIoiOut = 0;

    //memset(&_ADRF_FAULT_DATA,    0, sizeof(_ADRF_FAULT_DATA));
    //memset(&_adrf_rtc_time,      0, sizeof(_adrf_rtc_time));
    //memset(&_adrf_ships_time,    0, sizeof(_adrf_ships_time));
    //memset(&_adrf_data_operator, 0, sizeof(_adrf_data_operator));
    //memset(&_adrf_data_owner,    0, sizeof(_adrf_data_owner));
    //memset(&_adrf_rtc_source,    0, sizeof(_adrf_rtc_source));

    for (int i = 0; i < ASE_IN_MAX; ++i)
    {
        m_staticIoiIn[i] = aseIoiIn[i];
    }

    m_ioiStaticInCount = ASE_IN_MAX;
    m_validIoiIn = 0;
}

//---------------------------------------------------------------------------------------------
void StaticIoiContainer::OpenIoi()
{
    for (int i = 0; i < m_ioiStaticOutCount; ++i)
    {
        if (m_staticIoiOut[i]->OpenIoi())
        {
            m_validIoiOut += 1;
        }
    }

    for (int i = 0; i < m_ioiStaticInCount; ++i)
    {
        if (m_staticIoiIn[i]->OpenIoi())
        {
            m_validIoiIn += 1;
        }
    }
}

//---------------------------------------------------------------------------------------------
//IocResponse StaticIoiContainer::GetStaticIoiData( SecRequest& request )
//{
//
//}

//---------------------------------------------------------------------------------------------
bool StaticIoiContainer::SetStaticIoiData( SecRequest& request )
{
    if (request.variableId < m_ioiStaticOutCount)
    {
        return m_staticIoiOut[request.variableId]->SetStaticIoiData(request);
    }
    else if (request.variableId == m_ioiStaticOutCount)
    {
        ResetApatIoi();
        return true;
    }
    else
    {
        return false;
    }
}

//---------------------------------------------------------------------------------------------
// si13 => rtc_io_rd_day  1 - 7
// si18 => rtc_io_rd_year
// si16 => rtc_io_rd_month
// si12 => rtc_io_rd_date
// si14 => rtc_io_rd_hour
// si15 => rtc_io_rd_minutes
// si17 => rtc_io_rd_seconds
void StaticIoiContainer::UpdateStaticIoi()
{
#define TEN(x) ( ((x >> 4) & 0xF) * 10)
#define ONE(x) (x & 0xF)
#define VALUE(x) TEN(x) + ONE(x)

    // compute max count to provide a 2 Hz update rate 500ms/100ms => 5 frames
    const int kMaxCount = (m_ioiStaticOutCount/5) + 1;

    static unsigned int lastYrCnt  = 0;
    static unsigned int lastMoCnt  = 0;
    static unsigned int lastDayCnt = 0;
    static unsigned int lastHrCnt  = 0;
    static unsigned int lastMinCnt = 0;
    static unsigned int lastSecCnt  = 0;

    static unsigned char lastMin = 0;
    static unsigned char lastHr  = 0;
    static unsigned char lastDay = 0;
    static unsigned char lastMo  = 0;
    static unsigned char lastYr  = 0;

    // byte
    unsigned char ones;
    unsigned char tens;
    unsigned char data;

    // copy the current time into the rtc_IOI when things change
    //StaticIoiByte  si12("rtc_io_rd_date", 0);                      // 12
    //StaticIoiByte  si13("rtc_io_rd_day", 0);                       // 13
    //StaticIoiByte  si14("rtc_io_rd_hour", 0);                      // 14
    //StaticIoiByte  si15("rtc_io_rd_minutes", 0);                   // 15
    //StaticIoiByte  si16("rtc_io_rd_month", 0);                     // 16
    //StaticIoiByte  si17("rtc_io_rd_seconds", 0);                   // 17
    //StaticIoiByte  si18("rtc_io_rd_year", 0);                      // 18
    // seconds updated
    ones = aseCommon.clocks[eClkRtc].m_time.tm_sec % 10;
    tens = aseCommon.clocks[eClkRtc].m_time.tm_sec / 10;
    data = tens << 4 | ones;
    _rtc_io_rd_seconds[0] = data;

    // minutes updated
    if ( lastMin != aseCommon.clocks[eClkRtc].m_time.tm_min)
    {
        ones = aseCommon.clocks[eClkRtc].m_time.tm_min % 10;
        tens = aseCommon.clocks[eClkRtc].m_time.tm_min / 10;
        data = tens << 4 | ones;
        _rtc_io_rd_minutes[0] = data;
        lastMin = aseCommon.clocks[eClkRtc].m_time.tm_min;
    }

    // hours updated
    if ( lastHr != aseCommon.clocks[eClkRtc].m_time.tm_hour)
    {
        ones = aseCommon.clocks[eClkRtc].m_time.tm_hour % 10;
        tens = aseCommon.clocks[eClkRtc].m_time.tm_hour / 10;
        data = tens << 4 | ones;
        _rtc_io_rd_hour[0] = data;
        lastHr = aseCommon.clocks[eClkRtc].m_time.tm_hour;
    }

    // day updated
    if ( lastDay != aseCommon.clocks[eClkRtc].m_time.tm_mday)
    {
        ones = aseCommon.clocks[eClkRtc].m_time.tm_mday % 10;
        tens = aseCommon.clocks[eClkRtc].m_time.tm_mday / 10;
        data = tens << 4 | ones;
        _rtc_io_rd_date[0] = data;
        lastDay = aseCommon.clocks[eClkRtc].m_time.tm_mday;
    }

    // month updated
    if ( lastMo != aseCommon.clocks[eClkRtc].m_time.tm_mon)
    {
        ones = aseCommon.clocks[eClkRtc].m_time.tm_mon % 10;
        tens = aseCommon.clocks[eClkRtc].m_time.tm_mon / 10;
        data = tens << 4 | ones;
        _rtc_io_rd_month[0] = data;
        lastMo != aseCommon.clocks[eClkRtc].m_time.tm_mon;
    }

    // year updated
    if ( lastYr != aseCommon.clocks[eClkRtc].m_time.tm_year)
    {
        ones = (aseCommon.clocks[eClkRtc].m_time.tm_year - 2000) % 10;
        tens = (aseCommon.clocks[eClkRtc].m_time.tm_year - 2000) / 10;
        data = tens << 4 | ones;
        _rtc_io_rd_year[0] = data;
        lastYr = aseCommon.clocks[eClkRtc].m_time.tm_year;
    }

    for (int i = 0; i < kMaxCount; ++i)
    {
        if (!m_staticIoiOut[m_updateIndex]->Update())
        {
            m_writeError += 1;
        }

        m_updateIndex += 1;
        if (m_updateIndex >= m_ioiStaticOutCount)
        {
            m_updateIndex = 0;
        }
    }

    // read all of the ADRF outputs
    for (int i = 0; i < m_ioiStaticInCount; ++i)
    {
        if (!m_staticIoiIn[i]->Update())
        {
            m_readError += 1;
        }
    }

    // update the RTC time based on what we received
    //StaticIoiByte   so20("rtc_io_wr_year", 0, true);                    // 20
    //StaticIoiByte   so18("rtc_io_wr_month", 0, true);                   // 18
    //StaticIoiByte   so14("rtc_io_wr_date", 0, true);                    // 14
    //StaticIoiByte   so16("rtc_io_wr_hour", 0, true);                    // 16
    //StaticIoiByte   so17("rtc_io_wr_minutes", 0, true);                 // 17
    //StaticIoiByte   so19("rtc_io_wr_seconds", 0, true);                 // 19
    if (lastYrCnt  != _rtc_io_wr_year_.m_updateCount    &&
        lastMoCnt  != _rtc_io_wr_month_.m_updateCount   &&
        lastDayCnt != _rtc_io_wr_day_.m_updateCount     &&
        lastHrCnt  != _rtc_io_wr_hour_.m_updateCount    &&
        lastMinCnt != _rtc_io_wr_minutes_.m_updateCount &&
        lastSecCnt != _rtc_io_wr_seconds_.m_updateCount
        )
    {
        lastYrCnt  = _rtc_io_wr_year_.m_updateCount   ;
        lastMoCnt  = _rtc_io_wr_month_.m_updateCount  ;
        lastDayCnt = _rtc_io_wr_day_.m_updateCount    ;
        lastHrCnt  = _rtc_io_wr_hour_.m_updateCount   ;
        lastMinCnt = _rtc_io_wr_minutes_.m_updateCount;
        lastSecCnt = _rtc_io_wr_seconds_.m_updateCount;

        // Move the new values into RTC time
        // sec: data = tens << 4 | ones;
        aseCommon.clocks[eClkRtc].m_time.tm_year = VALUE(_rtc_io_wr_year[0]) + 2000;
        aseCommon.clocks[eClkRtc].m_time.tm_mon  = VALUE(_rtc_io_wr_month[0]);
        aseCommon.clocks[eClkRtc].m_time.tm_mday = VALUE(_rtc_io_wr_day[0]);
        aseCommon.clocks[eClkRtc].m_time.tm_hour = VALUE(_rtc_io_wr_hour[0]);
        aseCommon.clocks[eClkRtc].m_time.tm_min  = VALUE(_rtc_io_wr_minutes[0]);
        aseCommon.clocks[eClkRtc].m_time.tm_sec  = VALUE(_rtc_io_wr_seconds[0]);
    }
}

//---------------------------------------------------------------------------------------------
void StaticIoiContainer::SetNewState( SecRequest& request)
{
    if (request.variableId < m_ioiStaticOutCount)
    {
        // if not valid leave the running state at disabled
        bool newState = (bool)request.sigGenId && m_staticIoiOut[request.variableId]->ioiValid;
        m_staticIoiOut[request.variableId]->SetRunState(newState);
    }
}

//---------------------------------------------------------------------------------------------
// Called when no script is running
void StaticIoiContainer::Reset()
{
    for (int i = 0; i < m_ioiStaticOutCount; ++i)
    {
        // do not reset the running state if it is invalid
        m_staticIoiOut[i]->SetRunState(m_staticIoiOut[i]->ioiValid);
    }

    // when the script is done reset these
    _pat_scr_.data = 0;         // pat_scr = 0 : the main screen
    _pat_scr_button_.data = 0;  // pat_button = 0 : 0

    ResetApatIoi();
}

void StaticIoiContainer::ResetApatIoi()
{
    // and values we want to reset if no script is running
    // use 0xffffff to indicate the value has not been updated

    _8204050_32_.data = 0xffffff;
    _8204051_32_.data = 0xffffff;
    _8204052_32_.data = 0xffffff;
    _8204053_32_.data = 0xffffff;
    _8204054_32_.data = 0xffffff;
    _8204055_32_.data = 0xffffff;
    _8204056_32_.data = 0xffffff;
    _8204059_32_.data = 0xffffff;
    _8204060_32_.data = 0xffffff;
    _8204061_32_.data = 0xffffff;
    _8204062_32_.data = 0xffffff;
    _8204063_32_.data = 0xffffff;
    _8204064_32_.data = 0xffffff;
    _8204065_32_.data = 0xffffff;
    _8204066_32_.data = 0xffffff;
    _8204067_32_.data = 0xffffff;
    _8204068_32_.data = 0xffffff;
    _8204069_32_.data = 0xffffff;
    _8204070_32_.data = 0xffffff;
    _8204071_32_.data = 0xffffff;
    _8204072_32_.data = 0xffffff;
    _8204057_32_.data = 0xffffff;
    _8204058_32_.data = 0xffffff;

    _8204050_64_.data = 0xffffff;
    _8204051_64_.data = 0xffffff;
    _8204052_64_.data = 0xffffff;
    _8204053_64_.data = 0xffffff;
    _8204054_64_.data = 0xffffff;
    _8204055_64_.data = 0xffffff;
    _8204056_64_.data = 0xffffff;
    _8204059_64_.data = 0xffffff;
    _8204060_64_.data = 0xffffff;
    _8204061_64_.data = 0xffffff;
    _8204062_64_.data = 0xffffff;
    _8204063_64_.data = 0xffffff;
    _8204064_64_.data = 0xffffff;
    _8204065_64_.data = 0xffffff;
    _8204066_64_.data = 0xffffff;
    _8204067_64_.data = 0xffffff;
    _8204068_64_.data = 0xffffff;
    _8204069_64_.data = 0xffffff;
    _8204070_64_.data = 0xffffff;
    _8204071_64_.data = 0xffffff;
    _8204072_64_.data = 0xffffff;
    _8204057_64_.data = 0xffffff;
    _8204058_64_.data = 0xffffff;

    _adrf_pat_udt_remain_a_.data = 0xffffff;
    _adrf_pat_udt_remain_b_.data = 0xffffff;
}

bool StaticIoiContainer::GetStaticIoiData( SecComm& secComm )
{
    if (secComm.m_request.variableId < m_ioiStaticInCount)
    {
        // get the value and return true
        m_staticIoiIn[secComm.m_request.variableId]->GetStaticIoiData(secComm.m_response);
        return true;
    }
    else
    {
        return false;
    }
}
