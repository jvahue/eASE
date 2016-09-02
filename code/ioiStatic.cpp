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
char HMUpartNumber[20];
char HMUSerialNumber[20];
char PWSwDwgNumber[20];
char UTASSwDwgNumber[20];

// ------------------- U U T   I N P U T S ---------------------------
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

StaticIoiInt   si40("pat_scr", 0);                             // 40
StaticIoiInt   si41("pat_scr_button", 0);                      // 41

// -------------------- U U T   O U T P U T S ------------------------
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
StaticIoiInt    so21("adrf_data_batt_timer", 0, true);              // 21

// A429 Floating point values, so pass them back as integer
StaticIoiInt so22("8204050_32", 0, true); // 22 - 0x7D2F1220, # A Engine Inlet Angle 1
StaticIoiInt so23("8204051_32", 0, true); // 23 - 0x7D2F1320, # A Idle Time, A
StaticIoiInt so24("8204052_32", 0, true); // 24 - 0x7D2F1420, # A PAT Total Test Time
StaticIoiInt so25("8204053_32", 0, true); // 25 - 0x7D2F1520, # A Shutdown time Delay
StaticIoiInt so26("8204054_32", 0, true); // 26 - 0x7D2F1620, # A Wind Speed
StaticIoiInt so27("8204055_32", 0, true); // 27 - 0x7D2F1720, # A N1 Setting Tolerance
StaticIoiInt so28("8204056_32", 0, true); // 28 - 0x7D2F1820, # A Test Stabilization Time
StaticIoiInt so29("8204059_32", 0, true); // 29 - 0x7D2F1B20, # A PAT_MIN_IDLE_TREMAIN
StaticIoiInt so30("8204060_32", 0, true); // 30 - 0x7D2F1C20, # A PAT_N1_TARGET
StaticIoiInt so31("8204061_32", 0, true); // 31 - 0x7D2F1D20, # A PAT_N1_TARGET_TIME
StaticIoiInt so32("8204062_32", 0, true); // 32 - 0x7D2F1E20, # A PAT_IDLE_CLOSE_TIME
StaticIoiInt so33("8204063_32", 0, true); // 33 - 0x7D2F1F20, # A PAT_N1_STABLE_AVG
StaticIoiInt so34("8204064_32", 0, true); // 34 - 0x7D2F2020, # A PAT_N2_STABLE_AVG
StaticIoiInt so35("8204065_32", 0, true); // 35 - 0x7D2F2120, # A PAT_N2_LIM_MIN
StaticIoiInt so36("8204066_32", 0, true); // 36 - 0x7D2F2220, # A PAT_N2_LIM_MAX
StaticIoiInt so37("8204067_32", 0, true); // 37 - 0x7D2F2320, # A PAT_EGT_STABLE_AVG
StaticIoiInt so38("8204068_32", 0, true); // 38 - 0x7D2F2420, # A PAT_EGT_LIM_MIN
StaticIoiInt so39("8204069_32", 0, true); // 39 - 0x7D2F2520, # A PAT_EGT_LIM_MAX
StaticIoiInt so40("8204070_32", 0, true); // 40 - 0x7D2F2620, # A PAT_WF_STABLE_AVG
StaticIoiInt so41("8204071_32", 0, true); // 41 - 0x7D2F2720, # A PAT_WF_LIM_MIN
StaticIoiInt so42("8204072_32", 0, true); // 42 - 0x7D2F2820, # A PAT_WF_LIM_MAX
StaticIoiInt so43("8204057_32", 0, true); // 43 - 0x7D2F1920, # A Sts Wd1
StaticIoiInt so44("8204058_32", 0, true); // 44 - 0x7D2F1A20, # A Sts Wd2

// A429 Floating point values, so pass them back as integer
StaticIoiInt so45("8204050_64", 0, true); // 45 - 0x7D2F1240, # B Engine Inlet Angle 1
StaticIoiInt so46("8204051_64", 0, true); // 46 - 0x7D2F1340, # B Idle Time, A
StaticIoiInt so47("8204052_64", 0, true); // 47 - 0x7D2F1440, # B PAT Total Test Time
StaticIoiInt so48("8204053_64", 0, true); // 48 - 0x7D2F1540, # B Shutdown time Delay
StaticIoiInt so49("8204054_64", 0, true); // 49 - 0x7D2F1640, # B Wind Speed
StaticIoiInt so50("8204055_64", 0, true); // 50 - 0x7D2F1740, # B N1 Setting Tolerance
StaticIoiInt so51("8204056_64", 0, true); // 51 - 0x7D2F1840, # B Test Stabilization Time
StaticIoiInt so52("8204059_64", 0, true); // 52 - 0x7D2F1B40, # B PAT_MIN_IDLE_TREMAIN
StaticIoiInt so53("8204060_64", 0, true); // 53 - 0x7D2F1C40, # B PAT_N1_TARGET
StaticIoiInt so54("8204061_64", 0, true); // 54 - 0x7D2F1D40, # B PAT_N1_TARGET_TIME
StaticIoiInt so55("8204062_64", 0, true); // 55 - 0x7D2F1E40, # B PAT_IDLE_CLOSE_TIME
StaticIoiInt so56("8204063_64", 0, true); // 56 - 0x7D2F1F40, # B PAT_N1_STABLE_AVG
StaticIoiInt so57("8204064_64", 0, true); // 57 - 0x7D2F2040, # B PAT_N2_STABLE_AVG
StaticIoiInt so58("8204065_64", 0, true); // 58 - 0x7D2F2140, # B PAT_N2_LIM_MIN
StaticIoiInt so59("8204066_64", 0, true); // 59 - 0x7D2F2240, # B PAT_N2_LIM_MAX
StaticIoiInt so60("8204067_64", 0, true); // 60 - 0x7D2F2340, # B PAT_EGT_STABLE_AVG
StaticIoiInt so61("8204068_64", 0, true); // 61 - 0x7D2F2440, # B PAT_EGT_LIM_MIN
StaticIoiInt so62("8204069_64", 0, true); // 62 - 0x7D2F2540, # B PAT_EGT_LIM_MAX
StaticIoiInt so63("8204070_64", 0, true); // 63 - 0x7D2F2640, # B PAT_WF_STABLE_AVG
StaticIoiInt so64("8204071_64", 0, true); // 64 - 0x7D2F2740, # B PAT_WF_LIM_MIN
StaticIoiInt so65("8204072_64", 0, true); // 65 - 0x7D2F2840, # B PAT_WF_LIM_MAX
StaticIoiInt so66("8204057_64", 0, true); // 66 - 0x7D2F1940, # B Sts Wd1
StaticIoiInt so67("8204058_64", 0, true); // 67 - 0x7D2F1A40, # B Sts Wd2


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
    UINT32 x = 0;

    strcpy(HMUSerialNumber, "0000999999");
    strcpy(HMUpartNumber, "5316928SK01");
    strcpy(UTASSwDwgNumber, "PY1022429-026");
    strcpy(PWSwDwgNumber, "5318410-12SK01");

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

	m_staticIoiOut[x++] = &si40;  // 40
	m_staticIoiOut[x++] = &si41;  // 41

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
    m_staticIoiIn[x++] = &so21;
    
    m_staticIoiIn[x++] = &so22;
    m_staticIoiIn[x++] = &so23;
    m_staticIoiIn[x++] = &so24;
    m_staticIoiIn[x++] = &so25;
    m_staticIoiIn[x++] = &so26;
    m_staticIoiIn[x++] = &so27;
    m_staticIoiIn[x++] = &so28;
    m_staticIoiIn[x++] = &so29;
    m_staticIoiIn[x++] = &so30;
    m_staticIoiIn[x++] = &so31;
    m_staticIoiIn[x++] = &so32;
    m_staticIoiIn[x++] = &so33;
    m_staticIoiIn[x++] = &so34;
    m_staticIoiIn[x++] = &so35;
    m_staticIoiIn[x++] = &so36;
    m_staticIoiIn[x++] = &so37;
    m_staticIoiIn[x++] = &so38;
    m_staticIoiIn[x++] = &so39;
    m_staticIoiIn[x++] = &so40;
    m_staticIoiIn[x++] = &so41;
    m_staticIoiIn[x++] = &so42;
    
    m_staticIoiIn[x++] = &so43;
    m_staticIoiIn[x++] = &so44;
    
    m_staticIoiIn[x++] = &so45;
    m_staticIoiIn[x++] = &so46;
    m_staticIoiIn[x++] = &so47;
    m_staticIoiIn[x++] = &so48;
    m_staticIoiIn[x++] = &so49;
    m_staticIoiIn[x++] = &so50;
    m_staticIoiIn[x++] = &so51;
    m_staticIoiIn[x++] = &so52;
    m_staticIoiIn[x++] = &so53;
    m_staticIoiIn[x++] = &so54;
    m_staticIoiIn[x++] = &so55;
    m_staticIoiIn[x++] = &so56;
    m_staticIoiIn[x++] = &so57;
    m_staticIoiIn[x++] = &so58;
    m_staticIoiIn[x++] = &so59;
    m_staticIoiIn[x++] = &so60;
    m_staticIoiIn[x++] = &so61;
    m_staticIoiIn[x++] = &so62;
    m_staticIoiIn[x++] = &so63;
    m_staticIoiIn[x++] = &so64;
    m_staticIoiIn[x++] = &so65;
    
    m_staticIoiIn[x++] = &so66;
    m_staticIoiIn[x++] = &so67;

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
    si17.data = data;

    // minutes updated
    if ( lastMin != aseCommon.clocks[eClkRtc].m_time.tm_min)
    {
        ones = aseCommon.clocks[eClkRtc].m_time.tm_min % 10;
        tens = aseCommon.clocks[eClkRtc].m_time.tm_min / 10;
        data = tens << 4 | ones;
        si15.data = data;
        lastMin = aseCommon.clocks[eClkRtc].m_time.tm_min;
    }

    // hours updated
    if ( lastHr != aseCommon.clocks[eClkRtc].m_time.tm_hour)
    {
        ones = aseCommon.clocks[eClkRtc].m_time.tm_hour % 10;
        tens = aseCommon.clocks[eClkRtc].m_time.tm_hour / 10;
        data = tens << 4 | ones;
        si14.data = data;
        lastHr = aseCommon.clocks[eClkRtc].m_time.tm_hour;
    }

    // day updated
    if ( lastDay != aseCommon.clocks[eClkRtc].m_time.tm_mday)
    {
        ones = aseCommon.clocks[eClkRtc].m_time.tm_mday % 10;
        tens = aseCommon.clocks[eClkRtc].m_time.tm_mday / 10;
        data = tens << 4 | ones;
        si12.data = data;
        lastDay = aseCommon.clocks[eClkRtc].m_time.tm_mday;
    }

    // month updated
    if ( lastMo != aseCommon.clocks[eClkRtc].m_time.tm_mon)
    {
        ones = aseCommon.clocks[eClkRtc].m_time.tm_mon % 10;
        tens = aseCommon.clocks[eClkRtc].m_time.tm_mon / 10;
        data = tens << 4 | ones;
        si16.data = data;
        lastMo != aseCommon.clocks[eClkRtc].m_time.tm_mon;
    }

    // year updated
    if ( lastYr != aseCommon.clocks[eClkRtc].m_time.tm_year)
    {
        ones = (aseCommon.clocks[eClkRtc].m_time.tm_year - 2000) % 10;
        tens = (aseCommon.clocks[eClkRtc].m_time.tm_year - 2000) / 10;
        data = tens << 4 | ones;
        si18.data = data;
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
    if (lastYrCnt  != so20.m_updateCount &&
        lastMoCnt  != so18.m_updateCount &&
        lastDayCnt != so14.m_updateCount &&
        lastHrCnt  != so16.m_updateCount &&
        lastMinCnt != so17.m_updateCount &&
        lastSecCnt != so19.m_updateCount
        )
    {
        lastYrCnt  = so20.m_updateCount;
        lastMoCnt  = so18.m_updateCount;
        lastDayCnt = so14.m_updateCount;
        lastHrCnt  = so16.m_updateCount;
        lastMinCnt = so17.m_updateCount;
        lastSecCnt = so19.m_updateCount;

        // Move the new values into RTC time 
        // sec: data = tens << 4 | ones;
        aseCommon.clocks[eClkRtc].m_time.tm_year = VALUE(so20.data) + 2000;
        aseCommon.clocks[eClkRtc].m_time.tm_mon  = VALUE(so18.data);
        aseCommon.clocks[eClkRtc].m_time.tm_mday = VALUE(so14.data);
        aseCommon.clocks[eClkRtc].m_time.tm_hour = VALUE(so16.data);
        aseCommon.clocks[eClkRtc].m_time.tm_min  = VALUE(so17.data);
        aseCommon.clocks[eClkRtc].m_time.tm_sec  = VALUE(so19.data);
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

	// and values we want to reset if no script is running
	si40.data = 0;  // pat_scr = 0 : the main screen
	si41.data = 0;  // pat_button = 0 : 0
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
