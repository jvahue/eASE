//-----------------------------------------------------------------------------
//          Copyright (C) 2014-2017 Knowlogic Software Corp.
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

#include <ioiapi.h>

//----------------------------------------------------------------------------/
// Software Specific Includes                                                -/
//----------------------------------------------------------------------------/
#include "AseCommon.h"

#include "ioiStatic.h"
#include "ioiStaticCls.h"

//----------------------------------------------------------------------------/
// Local Defines                                                             -/
//----------------------------------------------------------------------------/
#define TEN(x) ( ((x >> 4) & 0xF) * 10)
#define ONE(x) (x & 0xF)
#define VALUE(x) TEN(x) + ONE(x)

//----------------------------------------------------------------------------/
// Local Typedefs                                                            -/
//----------------------------------------------------------------------------/

//----------------------------------------------------------------------------/
// Local Variables                                                           -/
//----------------------------------------------------------------------------/

//----------------------------------------------------------------------------/
// Constant Data                                                             -/
//----------------------------------------------------------------------------/
#include "AseStaticIoiInfo.h"

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

//=============================================================================================
StaticIoiContainer::StaticIoiContainer()
    : m_ioiStaticOutCount(0)
    , m_ioiStaticInCount(0)
    , m_aseInIndex(0)
    , m_aseOutIndex(0)
    , m_validIoiOut(0)
    , m_validIoiIn(0)
    , m_writeError(0)
    , m_writeErrorZ1(0)
    , m_readError(0)
    , m_readErrorZ1(0)
    , m_a664Qar()
    , m_a717Qar()
{
    // initialize a few values
    _BatInputVdc_.data = 27.9f;
    _BatSwOutVdc_.data = 28.2f;
    _BrdTempDegC_.data = 10.0f;

    strcpy(_HMUSerialNumber, "0000999999");
    strcpy(_HMUPartNumber,   "5316928SK05");
    strcpy(_UTASSwDwgNumber, "Y1022429-005");
    strcpy(_PWSwDwgNumber,   "5318410-17SK05");

    // TBD: do we really need to do this? 5/26/18 No 
    for (int i = 0; i < ASE_OUT_MAX; ++i)
    {
        m_staticAseOut[i] = aseIoiOut[i];
    }
    m_aseOutIndex = 0;
    m_ioiStaticOutCount = ASE_OUT_MAX;
    m_validIoiOut = 0;

    for (int i = 0; i < ASE_IN_MAX; ++i)
    {
        m_staticAseIn[i] = aseIoiIn[i];
    }
    m_aseInIndex = 0;
    m_ioiStaticInCount = ASE_IN_MAX;
    m_validIoiIn = 0;

    //----- initialize the "Smart" static IOI objects -----
    //----- QAR processing elements
    m_a664Qar.Reset(FindIoi("a664_fr_eicas2_fdr"));

    m_a717Qar.Reset(FindIoi("A717_Cfg_Request"),  // Cfg Request
                    FindIoi("A717_Cfg_Response"), // Cfg Response
                    FindIoi("A717Status"),        // Status Msg
                    FindIoi("A717Subframe1"),     // SF1
                    FindIoi("A717Subframe2"),     // SF2
                    FindIoi("A717Subframe3"),     // SF3
                    FindIoi("A717Subframe4"));    // SF4
}

//---------------------------------------------------------------------------------------------
void StaticIoiContainer::OpenIoi()
{
    for (int i = 0; i < m_ioiStaticOutCount; ++i)
    {
        if (m_staticAseOut[i]->OpenIoi())
        {
            m_validIoiOut += 1;
        }
    }

    for (int i = 0; i < m_ioiStaticInCount; ++i)
    {
        if (m_staticAseIn[i]->OpenIoi())
        {
            m_validIoiIn += 1;
        }
    }
}

//---------------------------------------------------------------------------------------------
void StaticIoiContainer::UpdateStaticIoi()
{
    //----- Run the Smart Static IOI objects -----
    m_writeError += m_a664Qar.UpdateIoi(); // A664QAR
    m_writeError += m_a717Qar.UpdateIoi(); // A664QAR

    ProcessAdrfStaticInput();
    ProcessAdrfStaticOutput();
}

//---------------------------------------------------------------------------------------------
void StaticIoiContainer::ProcessAdrfStaticInput()
{
    // compute max count to provide a 10Hz update rate 100ms/10ms => 10 frames
    const int kOutMaxCount = ((m_ioiStaticOutCount) / 10) + 1;

    UpdateRtcClock();

    // set the UUT Input maintaining about a 10Hz update rate
    m_writeErrorZ1 = m_writeError;
    for (int i = 0; i < kOutMaxCount; ++i)
    {
        if (!m_staticAseOut[m_aseOutIndex]->m_isParam)
        {
            if (!m_staticAseOut[m_aseOutIndex]->Update())
            {
                m_writeError += 1;
            }
        }

        m_aseOutIndex += 1;
        if (m_aseOutIndex >= m_ioiStaticOutCount)
        {
            m_aseOutIndex = 0;
        }
    }
}

//---------------------------------------------------------------------------------------------
// Export our ASE RTC time ... so we can read it back in ...
void StaticIoiContainer::UpdateRtcClock()
{
    static unsigned char lastMin = 0;
    static unsigned char lastHr = 0;
    static unsigned char lastDay = 0;
    static unsigned char lastMo = 0;
    static unsigned char lastYr = 0;

    // byte
    unsigned char ones;
    unsigned char tens;
    unsigned char data;

    // copy the current time into the rtc_IOI when things change
    // seconds updated
    ones = aseCommon.clocks[eClkRtc].m_time.tm_sec % 10;
    tens = aseCommon.clocks[eClkRtc].m_time.tm_sec / 10;
    data = tens << 4 | ones;
    _rtc_io_rd_seconds[0] = data;

    // minutes updated
    if (lastMin != aseCommon.clocks[eClkRtc].m_time.tm_min)
    {
        ones = aseCommon.clocks[eClkRtc].m_time.tm_min % 10;
        tens = aseCommon.clocks[eClkRtc].m_time.tm_min / 10;
        data = tens << 4 | ones;
        _rtc_io_rd_minutes[0] = data;
        lastMin = aseCommon.clocks[eClkRtc].m_time.tm_min;
    }

    // hours updated
    if (lastHr != aseCommon.clocks[eClkRtc].m_time.tm_hour)
    {
        ones = aseCommon.clocks[eClkRtc].m_time.tm_hour % 10;
        tens = aseCommon.clocks[eClkRtc].m_time.tm_hour / 10;
        data = tens << 4 | ones;
        _rtc_io_rd_hour[0] = data;
        lastHr = aseCommon.clocks[eClkRtc].m_time.tm_hour;
    }

    // day updated
    if (lastDay != aseCommon.clocks[eClkRtc].m_time.tm_mday)
    {
        ones = aseCommon.clocks[eClkRtc].m_time.tm_mday % 10;
        tens = aseCommon.clocks[eClkRtc].m_time.tm_mday / 10;
        data = tens << 4 | ones;
        _rtc_io_rd_date[0] = data;
        lastDay = aseCommon.clocks[eClkRtc].m_time.tm_mday;
    }

    // month updated
    if (lastMo != aseCommon.clocks[eClkRtc].m_time.tm_mon)
    {
        ones = aseCommon.clocks[eClkRtc].m_time.tm_mon % 10;
        tens = aseCommon.clocks[eClkRtc].m_time.tm_mon / 10;
        data = tens << 4 | ones;
        _rtc_io_rd_month[0] = data;
        lastMo = aseCommon.clocks[eClkRtc].m_time.tm_mon;
    }

    // year updated
    if (lastYr != aseCommon.clocks[eClkRtc].m_time.tm_year)
    {
        ones = (aseCommon.clocks[eClkRtc].m_time.tm_year - 2000) % 10;
        tens = (aseCommon.clocks[eClkRtc].m_time.tm_year - 2000) / 10;
        data = tens << 4 | ones;
        _rtc_io_rd_year[0] = data;
        lastYr = aseCommon.clocks[eClkRtc].m_time.tm_year;
    }
}

//---------------------------------------------------------------------------------------------
void StaticIoiContainer::ProcessAdrfStaticOutput()
{
    // compute max count to provide a 20Hz update rate 50ms/10ms => 5 frames
    const int kInMaxCount = (m_ioiStaticInCount / 5) + 1;

    static unsigned int lastYrCnt = 0;
    static unsigned int lastMoCnt = 0;
    static unsigned int lastDayCnt = 0;
    static unsigned int lastHrCnt = 0;
    static unsigned int lastMinCnt = 0;
    static unsigned int lastSecCnt = 0;

    // read the ADRF outputs at 20Hz
    m_readErrorZ1 = m_readError;
    for (int i = 0; i < kInMaxCount; ++i)
    {
        if (!m_staticAseIn[m_aseInIndex]->Update())
        {
            m_readError += 1;
        }

        m_aseInIndex += 1;
        if (m_aseInIndex >= m_ioiStaticInCount)
        {
            m_aseInIndex = 0;
        }
    }

    // update the RTC time based on what we received
    if (lastYrCnt != _rtc_io_wr_year_.m_updateCount    &&
        lastMoCnt != _rtc_io_wr_month_.m_updateCount   &&
        lastDayCnt != _rtc_io_wr_date_.m_updateCount    &&
        lastHrCnt != _rtc_io_wr_hour_.m_updateCount    &&
        lastMinCnt != _rtc_io_wr_minutes_.m_updateCount &&
        lastSecCnt != _rtc_io_wr_seconds_.m_updateCount
        )
    {
        lastYrCnt = _rtc_io_wr_year_.m_updateCount;
        lastMoCnt = _rtc_io_wr_month_.m_updateCount;
        lastDayCnt = _rtc_io_wr_date_.m_updateCount;
        lastHrCnt = _rtc_io_wr_hour_.m_updateCount;
        lastMinCnt = _rtc_io_wr_minutes_.m_updateCount;
        lastSecCnt = _rtc_io_wr_seconds_.m_updateCount;

        // Move the new values into RTC time
        // sec: data = tens << 4 | ones;
        aseCommon.clocks[eClkRtc].m_time.tm_year = VALUE(_rtc_io_wr_year[0]) + 2000;
        aseCommon.clocks[eClkRtc].m_time.tm_mon = VALUE(_rtc_io_wr_month[0]);
        aseCommon.clocks[eClkRtc].m_time.tm_mday = VALUE(_rtc_io_wr_date[0]);
        aseCommon.clocks[eClkRtc].m_time.tm_hour = VALUE(_rtc_io_wr_hour[0]);
        aseCommon.clocks[eClkRtc].m_time.tm_min = VALUE(_rtc_io_wr_minutes[0]);
        aseCommon.clocks[eClkRtc].m_time.tm_sec = VALUE(_rtc_io_wr_seconds[0]);
    }
}

//---------------------------------------------------------------------------------------------
bool StaticIoiContainer::SetStaticIoiData(SecComm& secComm)
{
    SecRequest request = secComm.m_request;
    if (request.variableId < m_ioiStaticOutCount)
    {
        // see if any of our smart static objects want to handle this message
        if (m_a664Qar.HandleRequest(m_staticAseOut[request.variableId]))
        {
            return m_a664Qar.TestControl(request);
        }
        else if (m_a717Qar.HandleRequest(m_staticAseOut[request.variableId]))
        {
            return m_a717Qar.TestControl(request);
        }
        else
        {
            return m_staticAseOut[request.variableId]->SetStaticIoiData(request);
        }
    }
    else if (request.variableId == m_ioiStaticOutCount)
    {
        ResetStaticIoi();
        return true;
    }
    else
    {
        return false;
    }
}

//---------------------------------------------------------------------------------------------
bool StaticIoiContainer::GetStaticIoiData(SecComm& secComm)
{
    if (secComm.m_request.variableId < m_ioiStaticInCount)
    {
        // here is a little trick we play for String types to big to fit into the streamData
        // PySte passes us where to start in the request object, we move it over into the 
        // response object so the StaticIoiStr know what offset to start at
        secComm.m_response.streamSize = secComm.m_request.sigGenId;

        // get the value and return true
        m_staticAseIn[secComm.m_request.variableId]->GetStaticIoiData(secComm.m_response);
        return true;
    }
    else
    {
        return false;
    }
}

//---------------------------------------------------------------------------------------------
void StaticIoiContainer::SetNewState(SecRequest& request)
{
    if (request.variableId < m_ioiStaticOutCount)
    {
        // if not valid leave the running state at disabled
        bool newState = ((request.sigGenId == 1) 
                         && m_staticAseOut[request.variableId]->m_ioiValid);
        m_staticAseOut[request.variableId]->SetRunState(newState);
    }
}

//---------------------------------------------------------------------------------------------
// Called when no script is running
void StaticIoiContainer::Reset()
{
    for (int i = 0; i < m_ioiStaticOutCount; ++i)
    {
        // do not reset the running state if it is invalid
        m_staticAseOut[i]->SetRunState(m_staticAseOut[i]->m_ioiValid);
    }

    // when the script is done reset these
    _OMS_PAGEID_.data = 0;  // pat_scr = 0 : the main screen
    _OMS_BUTTON_.data = 0;  // pat_button = 0 : 0

    ResetStaticIoi();
}

//---------------------------------------------------------------------------------------------
void StaticIoiContainer::ResetStaticIoi()
{
    // and values we want to reset if no script is running
    // use 0xffffff to indicate the value has not been updated, I know it looks like it should
    // be  0xffffffff but after shifting in PySte it becomes 0xffffff

    //memset(_8204050_32, 0xff, sizeof(_8204050_32));
    memset(_8204051_32, 0xff, sizeof(_8204051_32));
    memset(_8204052_32, 0xff, sizeof(_8204052_32));
    memset(_8204053_32, 0xff, sizeof(_8204053_32));
    //memset(_8204054_32, 0xff, sizeof(_8204054_32));
    memset(_8204055_32, 0xff, sizeof(_8204055_32));
    memset(_8204056_32, 0xff, sizeof(_8204056_32));
    memset(_8204059_32, 0xff, sizeof(_8204059_32));
    memset(_8204060_32, 0xff, sizeof(_8204060_32));
    memset(_8204061_32, 0xff, sizeof(_8204061_32));
    memset(_8204062_32, 0xff, sizeof(_8204062_32));
    memset(_8204063_32, 0xff, sizeof(_8204063_32));
    memset(_8204064_32, 0xff, sizeof(_8204064_32));
    memset(_8204065_32, 0xff, sizeof(_8204065_32));
    memset(_8204066_32, 0xff, sizeof(_8204066_32));
    memset(_8204067_32, 0xff, sizeof(_8204067_32));
    memset(_8204068_32, 0xff, sizeof(_8204068_32));
    memset(_8204069_32, 0xff, sizeof(_8204069_32));
    memset(_8204070_32, 0xff, sizeof(_8204070_32));
    memset(_8204071_32, 0xff, sizeof(_8204071_32));
    memset(_8204072_32, 0xff, sizeof(_8204072_32));
    memset(_8204057_32, 0xff, sizeof(_8204057_32));
    memset(_8204058_32, 0xff, sizeof(_8204058_32));

    //memset(_8204050_64, 0xff, sizeof(_8204050_64));
    memset(_8204051_64, 0xff, sizeof(_8204051_64));
    memset(_8204052_64, 0xff, sizeof(_8204052_64));
    memset(_8204053_64, 0xff, sizeof(_8204053_64));
    //memset(_8204054_64, 0xff, sizeof(_8204054_64));
    memset(_8204055_64, 0xff, sizeof(_8204055_64));
    memset(_8204056_64, 0xff, sizeof(_8204056_64));
    memset(_8204059_64, 0xff, sizeof(_8204059_64));
    memset(_8204060_64, 0xff, sizeof(_8204060_64));
    memset(_8204061_64, 0xff, sizeof(_8204061_64));
    memset(_8204062_64, 0xff, sizeof(_8204062_64));
    memset(_8204063_64, 0xff, sizeof(_8204063_64));
    memset(_8204064_64, 0xff, sizeof(_8204064_64));
    memset(_8204065_64, 0xff, sizeof(_8204065_64));
    memset(_8204066_64, 0xff, sizeof(_8204066_64));
    memset(_8204067_64, 0xff, sizeof(_8204067_64));
    memset(_8204068_64, 0xff, sizeof(_8204068_64));
    memset(_8204069_64, 0xff, sizeof(_8204069_64));
    memset(_8204070_64, 0xff, sizeof(_8204070_64));
    memset(_8204071_64, 0xff, sizeof(_8204071_64));
    memset(_8204072_64, 0xff, sizeof(_8204072_64));
    memset(_8204057_64, 0xff, sizeof(_8204057_64));
    memset(_8204058_64, 0xff, sizeof(_8204058_64));

    memset(_adrf_pat_udt_remain_a, 0xff, sizeof(_adrf_pat_udt_remain_a));
    memset(_adrf_pat_udt_remain_b, 0xff, sizeof(_adrf_pat_udt_remain_b));

    // clear any error injection and reset data and NDO

}

//---------------------------------------------------------------------------------------------
// Any time we load a Cfg call this to disconnect the static IOI from param control
void StaticIoiContainer::ResetStaticParams()
{
    for (int i = 0; i < m_ioiStaticOutCount; ++i)
    {
        // do not reset the running state if it is invalid
        m_staticAseOut[i]->m_isParam = false;
    }

    //----- re-initialize the "Smart" static IOI objects -----
    //----- QAR processing elements
    m_a664Qar.Reset( FindIoi( "a664_fr_eicas2_fdr" ) );

    m_a717Qar.Reset( FindIoi( "A717_Cfg_Request" ),  // Cfg Request
                     FindIoi( "A717_Cfg_Response" ), // Cfg Response
                     FindIoi( "A717Status" ),        // Status Msg
                     FindIoi( "A717Subframe1" ),     // SF1
                     FindIoi( "A717Subframe2" ),     // SF2
                     FindIoi( "A717Subframe3" ),     // SF3
                     FindIoi( "A717Subframe4" ) );    // SF4

}

//---------------------------------------------------------------------------------------------
// Search through the ASE static IOI outputs for a particular name.  Return a pointer to the 
// object when found, NULL otherwise
StaticIoiObj* StaticIoiContainer::FindIoi(char* name)
{
    // check for names like QAR_A717_
    for (int i = 0; i < m_ioiStaticOutCount; ++i)
    {
        // do not reset the running state if it is invalid
        if (strncmp(name, m_staticAseOut[i]->m_ioiName, eAseParamNameSize) == 0)
        {
            // ok indicate that this IOI will be managed by a parameter
            m_staticAseOut[i]->m_isParam = true;
            return m_staticAseOut[i];
        }
    }

    return NULL;
}

