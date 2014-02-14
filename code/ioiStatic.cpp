//-----------------------------------------------------------------------------
//            Copyright (C) 2014 Knowlogic Software Corp.
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
char HMUpartNumber[20];
char HMUSerialNumber[20];
char PWSwDwgNumber[20];
char UTASSwDwgNumber[20];

// --------- The Order MUST match AseIoiInfo in eFastCmds.py ---------
StaticIoiInt   si00("aircraft_id_1", 0);                       // 00
StaticIoiInt   si01("aircraft_id_2", 0);                       // 01
StaticIoiInt   si02("aircraft_id_3", 0);                       // 02
StaticIoiInt   si03("aircraft_id_4", 0);                       // 03
StaticIoiInt   si04("aircraft_id_5", 0);                       // 04
StaticIoiInt   si05("aircraft_id_par", 0);                     // 05
StaticIoiInt   si06("disc_spare_1", 0);                        // 06
StaticIoiInt   si07("disc_spare_2", 0);                        // 07
StaticIoiInt   si08("disc_spare_3", 0);                        // 08
StaticIoiInt   si09("ground_service_mode", 0);                 // 09

StaticIoiInt   si10("weight_on_wheels", 0);                    // 10
StaticIoiInt   si11("wifi_override", 0);                       // 11
StaticIoiByte  si12("rtc_io_rd_date", 0);                      // 12
StaticIoiByte  si13("rtc_io_rd_day", 0);                       // 13
StaticIoiByte  si14("rtc_io_rd_hour", 0);                      // 14
StaticIoiByte  si15("rtc_io_rd_minutes", 0);                   // 15
StaticIoiByte  si16("rtc_io_rd_month", 0);                     // 16
StaticIoiByte  si17("rtc_io_rd_seconds", 0);                   // 17
StaticIoiByte  si18("rtc_io_rd_year", 0);                      // 18
StaticIoiInt   si19("ubmf_health_ind", 0);                     // 19

StaticIoiInt   si20("umbf_status_word1", 0);                   // 20
StaticIoiInt   si21("BrdTempFail", 0);                         // 21
StaticIoiInt   si22("BrdTempInitFail", 0);                     // 22
StaticIoiInt   si23("BrdTempOpFail", 0);                       // 23
StaticIoiInt   si24("BrdTempRngFail", 0);                      // 24
StaticIoiInt   si25("HLEIFFaultInd2", 0);                      // 25
StaticIoiInt   si26("HLEIFFaultIndication", 0);                // 26
StaticIoiInt   si27("HLEIFStatusWord1", 0);                    // 27
StaticIoiInt   si28("hmu_option_data_raw", 0);                 // 28
StaticIoiInt   si29("micro_server_health_ind1", 0);            // 29

StaticIoiInt   si30("micro_server_health_ind2", 0);            // 30
StaticIoiInt   si31("micro_server_internal_status", 0);        // 31
StaticIoiInt   si32("micro_server_status_word1", 0);           // 32
StaticIoiFloat si33("BatInputVdc", 27.9f);                     // 33
StaticIoiFloat si34("BatSwOutVdc", 28.2f);                     // 34
StaticIoiFloat si35("BrdTempDegC", 10.0f);                     // 35
StaticIoiStr   si36("HMUPartNumber", HMUpartNumber, 20);       // 36
StaticIoiStr   si37("HMUSerialNumber", HMUSerialNumber, 20);   // 37
StaticIoiStr   si38("PWSwDwgNumber", PWSwDwgNumber, 20);       // 38
StaticIoiStr   si39("UTASSwDwgNumber", UTASSwDwgNumber, 20);   // 39

// --------- The Order MUST match adrfOutMap in eFastCmds.py ---------
int adrfFault[13];
int adrfTime[2];
int shipTime[2];
char operatorName[64];
char ownerName[64];
char rtcSource[5];

StaticIoiIntPtr so00("ADRF_FAULT_DATA", adrfFault, 13, true);       // 00
StaticIoiInt    so01("adrf_apm_wrap", 0, true);                     // 01
StaticIoiInt    so02("adrf_data_cumflt_time", 0, true);             // 02
StaticIoiInt    so03("adrf_data_flt_time_current", 0, true);        // 03
StaticIoiStr    so04("adrf_data_operator", operatorName, 64, true); // 04
StaticIoiStr    so05("adrf_data_owner", ownerName, 64, true);       // 05
StaticIoiInt    so06("adrf_data_power_on_cnt", 0, true);            // 06
StaticIoiInt    so07("adrf_data_power_on_time", 0, true);           // 07
StaticIoiInt    so08("adrf_health_ind", 0, true);                   // 08
StaticIoiStr    so09("adrf_rtc_source", rtcSource, 2, true);        // 09
StaticIoiIntPtr so10("adrf_rtc_time", adrfTime, 2, true);           // 10
StaticIoiIntPtr so11("adrf_ships_time", shipTime, 2, true);         // 11
StaticIoiInt    so12("adrf_status_word1", 0, true);                 // 12
StaticIoiInt    so13("adrf_status_word2", 0, true);                 // 13
StaticIoiByte   so14("rtc_io_wr_date", 0, true);                    // 14
StaticIoiByte   so15("rtc_io_wr_day", 0, true);                     // 15
StaticIoiByte   so16("rtc_io_wr_hour", 0, true);                    // 16
StaticIoiByte   so17("rtc_io_wr_minutes", 0, true);                 // 17
StaticIoiByte   so18("rtc_io_wr_month", 0, true);                   // 18
StaticIoiByte   so19("rtc_io_wr_seconds", 0, true);                 // 19
StaticIoiByte   so20("rtc_io_wr_year", 0, true);                    // 20

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
    return false;
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
    UINT32 x = 0;

    strcpy(HMUpartNumber, "HmuPart");
    strcpy(HMUSerialNumber, "HmuSerial");
    strcpy(PWSwDwgNumber, "PwSwDwg");
    strcpy(UTASSwDwgNumber, "UtasSwDwg");

    // The Order MUST match AseIoiInfo in eFastCmds.py
    m_staticIoiOut[x++] = &si00;  // 00
    m_staticIoiOut[x++] = &si01;  // 01
    m_staticIoiOut[x++] = &si02;  // 02
    m_staticIoiOut[x++] = &si03;  // 03
    m_staticIoiOut[x++] = &si04;  // 04
    m_staticIoiOut[x++] = &si05;  // 05
    m_staticIoiOut[x++] = &si06;  // 06
    m_staticIoiOut[x++] = &si07;  // 07
    m_staticIoiOut[x++] = &si08;  // 08
    m_staticIoiOut[x++] = &si09;  // 09

    m_staticIoiOut[x++] = &si10;  // 10
    m_staticIoiOut[x++] = &si11;  // 11
    m_staticIoiOut[x++] = &si12;  // 12
    m_staticIoiOut[x++] = &si13;  // 13
    m_staticIoiOut[x++] = &si14;  // 14
    m_staticIoiOut[x++] = &si15;  // 15
    m_staticIoiOut[x++] = &si16;  // 16
    m_staticIoiOut[x++] = &si17;  // 17
    m_staticIoiOut[x++] = &si18;  // 18
    m_staticIoiOut[x++] = &si19;  // 19

    m_staticIoiOut[x++] = &si20;  // 20
    m_staticIoiOut[x++] = &si21;  // 21
    m_staticIoiOut[x++] = &si22;  // 22
    m_staticIoiOut[x++] = &si23;  // 23
    m_staticIoiOut[x++] = &si24;  // 24
    m_staticIoiOut[x++] = &si25;  // 25
    m_staticIoiOut[x++] = &si26;  // 26
    m_staticIoiOut[x++] = &si27;  // 27
    m_staticIoiOut[x++] = &si28;  // 28
    m_staticIoiOut[x++] = &si29;  // 29

    m_staticIoiOut[x++] = &si30;  // 30
    m_staticIoiOut[x++] = &si31;  // 31
    m_staticIoiOut[x++] = &si32;  // 32
    m_staticIoiOut[x++] = &si33;  // 33
    m_staticIoiOut[x++] = &si34;  // 34
    m_staticIoiOut[x++] = &si35;  // 35
    m_staticIoiOut[x++] = &si36;  // 36
    m_staticIoiOut[x++] = &si37;  // 37
    m_staticIoiOut[x++] = &si38;  // 38
    m_staticIoiOut[x++] = &si39;  // 39

    m_ioiStaticOutCount = x;
    m_updateIndex = 0;
    m_validIoiOut = 0;

    memset(&adrfFault, 0, sizeof(adrfFault));
    memset(&adrfTime, 0, sizeof(adrfTime));
    memset(&shipTime, 0, sizeof(shipTime));
    memset(&operatorName, 0, sizeof(operatorName));
    memset(&ownerName, 0, sizeof(ownerName));
    memset(&rtcSource, 0, sizeof(rtcSource));

    x = 0;
    m_staticIoiIn[x++] = &so00;
    m_staticIoiIn[x++] = &so01;
    m_staticIoiIn[x++] = &so02;
    m_staticIoiIn[x++] = &so03;
    m_staticIoiIn[x++] = &so04;
    m_staticIoiIn[x++] = &so05;
    m_staticIoiIn[x++] = &so06;
    m_staticIoiIn[x++] = &so07;
    m_staticIoiIn[x++] = &so08;
    m_staticIoiIn[x++] = &so09;
    m_staticIoiIn[x++] = &so10;
    m_staticIoiIn[x++] = &so11;
    m_staticIoiIn[x++] = &so12;
    m_staticIoiIn[x++] = &so13;
    m_staticIoiIn[x++] = &so14;
    m_staticIoiIn[x++] = &so15;
    m_staticIoiIn[x++] = &so16;
    m_staticIoiIn[x++] = &so17;
    m_staticIoiIn[x++] = &so18;
    m_staticIoiIn[x++] = &so19;
    m_staticIoiIn[x++] = &so20;

    m_ioiStaticInCount = x;
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
    // compute max count to provide a 2 Hz update rate 500ms/100ms => 5 frames
    const int kMaxCount = (m_ioiStaticOutCount/5) + 1;

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
    // seconds updated
    ones = aseCommon.time.tm_sec % 10;
    tens = aseCommon.time.tm_sec / 10;
    data = tens << 4 | ones;
    si17.data = data;

    // minutes updated
    if ( lastMin != aseCommon.time.tm_min)
    {
        ones = aseCommon.time.tm_min % 10;
        tens = aseCommon.time.tm_min / 10;
        data = tens << 4 | ones;
        si15.data = data;
        lastMin = aseCommon.time.tm_min;
    }

    // hours updated
    if ( lastHr != aseCommon.time.tm_hour)
    {
        ones = aseCommon.time.tm_hour % 10;
        tens = aseCommon.time.tm_hour / 10;
        data = tens << 4 | ones;
        si14.data = data;
        lastHr = aseCommon.time.tm_hour;
    }

    // day updated
    if ( lastDay != aseCommon.time.tm_mday)
    {
        ones = aseCommon.time.tm_mday % 10;
        tens = aseCommon.time.tm_mday / 10;
        data = tens << 4 | ones;
        si12.data = data;
        lastDay = aseCommon.time.tm_mday;
    }

    // month updated
    if ( lastMo != aseCommon.time.tm_mon)
    {
        ones = aseCommon.time.tm_mon % 10;
        tens = aseCommon.time.tm_mon / 10;
        data = tens << 4 | ones;
        si16.data = data;
        lastMo != aseCommon.time.tm_mon;
    }

    // year updated
    if ( lastYr != aseCommon.time.tm_year)
    {
        ones = (aseCommon.time.tm_year - 2000) % 10;
        tens = (aseCommon.time.tm_year - 2000) / 10;
        data = tens << 4 | ones;
        si18.data = data;
        lastYr = aseCommon.time.tm_year;
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
void StaticIoiContainer::Reset()
{
    for (int i = 0; i < m_ioiStaticOutCount; ++i)
    {
        // do not reset the running state if it is invalid
        m_staticIoiOut[i]->SetRunState(m_staticIoiOut[i]->ioiValid);
    }
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
