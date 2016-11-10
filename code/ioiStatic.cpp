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

//char HMUpartNumber[20];
//char HMUSerialNumber[20];
//char PWSwDwgNumber[20];
//char UTASSwDwgNumber[20];
//
//// ------------------- U U T   I N P U T S ---------------------------
//// --------- The Order MUST match AseIoiInfo in eFastCmds.py ---------
//StaticIoiInt   si00("aircraft_id_1", 0);                       // 00
//StaticIoiInt   si01("aircraft_id_2", 0);                       // 01
//StaticIoiInt   si02("aircraft_id_3", 0);                       // 02
//StaticIoiInt   si03("aircraft_id_4", 0);                       // 03
//StaticIoiInt   si04("aircraft_id_5", 0);                       // 04
//StaticIoiInt   si05("aircraft_id_par", 0);                     // 05
//StaticIoiInt   si06("disc_spare_1", 0);                        // 06
//StaticIoiInt   si07("disc_spare_2", 0);                        // 07
//StaticIoiInt   si08("disc_spare_3", 0);                        // 08
//StaticIoiInt   si09("ground_service_mode", 0);                 // 09
//
//StaticIoiInt   si10("weight_on_wheels", 0);                    // 10
//StaticIoiInt   si11("wifi_override", 0);                       // 11
//StaticIoiByte  si12("rtc_io_rd_date", 0);                      // 12
//StaticIoiByte  si13("rtc_io_rd_day", 0);                       // 13
//StaticIoiByte  si14("rtc_io_rd_hour", 0);                      // 14
//StaticIoiByte  si15("rtc_io_rd_minutes", 0);                   // 15
//StaticIoiByte  si16("rtc_io_rd_month", 0);                     // 16
//StaticIoiByte  si17("rtc_io_rd_seconds", 0);                   // 17
//StaticIoiByte  si18("rtc_io_rd_year", 0);                      // 18
//StaticIoiInt   si19("ubmf_health_ind", 0);                     // 19
//
//StaticIoiInt   si20("umbf_status_word1", 0);                   // 20
//StaticIoiInt   si21("BrdTempFail", 0);                         // 21
//StaticIoiInt   si22("BrdTempInitFail", 0);                     // 22
//StaticIoiInt   si23("BrdTempOpFail", 0);                       // 23
//StaticIoiInt   si24("BrdTempRngFail", 0);                      // 24
//StaticIoiInt   si25("HLEIFFaultInd2", 0);                      // 25
//StaticIoiInt   si26("HLEIFFaultIndication", 0);                // 26
//StaticIoiInt   si27("HLEIFStatusWord1", 0);                    // 27
//StaticIoiInt   si28("hmu_option_data_raw", 0);                 // 28
//StaticIoiInt   si29("micro_server_health_ind1", 0);            // 29
//
//StaticIoiInt   si30("micro_server_health_ind2", 0);            // 30
//StaticIoiInt   si31("micro_server_internal_status", 0);        // 31
//StaticIoiInt   si32("micro_server_status_word1", 0);           // 32
//StaticIoiFloat si33("BatInputVdc", 27.9f);                     // 33
//StaticIoiFloat si34("BatSwOutVdc", 28.2f);                     // 34
//StaticIoiFloat si35("BrdTempDegC", 10.0f);                     // 35
//StaticIoiStr   si36("HMUPartNumber", HMUpartNumber, 20);       // 36
//StaticIoiStr   si37("HMUSerialNumber", HMUSerialNumber, 20);   // 37
//StaticIoiStr   si38("PWSwDwgNumber", PWSwDwgNumber, 20);       // 38
//StaticIoiStr   si39("UTASSwDwgNumber", UTASSwDwgNumber, 20);   // 39
//
//StaticIoiInt   si40("pat_scr", 0);                             // 40
//StaticIoiInt   si41("pat_scr_button", 0);                      // 41
//
//// SCR-355 New Internal signals
//StaticIoiInt   si42("fault_word3", 0);                         // 42 OL354
//StaticIoiInt   si43("fault_word4", 0);                         // 43 OL355
//StaticIoiInt   si44("prg_flash_cbit_fail", 0);                 // 44 OL356
//StaticIoiInt   si45("fault_word7", 0);                         // 45 OL357
//StaticIoiInt   si46("hleif_eicas_annun_tx", 0);                // 46 OL270
//StaticIoiInt   si47("hmu_eicas_annun", 0);                     // 47 OL271
//
//// -------------------- U U T   O U T P U T S ------------------------
//// --------- The Order MUST match adrfOutMap in eFastCmds.py ---------
//int adrfFault[13];
//int adrfTime[2];
//int shipTime[2];
//char operatorName[64];
//char ownerName[64];
//char rtcSource[5];
//
//StaticIoiIntPtr so00("ADRF_FAULT_DATA", adrfFault, 13, true);       // 00
//StaticIoiInt    so01("adrf_apm_wrap", 0, true);                     // 01
//StaticIoiInt    so02("adrf_data_cumflt_time", 0, true);             // 02
//StaticIoiInt    so03("adrf_data_flt_time_current", 0, true);        // 03
//StaticIoiStr    so04("adrf_data_operator", operatorName, 64, true); // 04
//StaticIoiStr    so05("adrf_data_owner", ownerName, 64, true);       // 05
//StaticIoiInt    so06("adrf_data_power_on_cnt", 0, true);            // 06
//StaticIoiInt    so07("adrf_data_power_on_time", 0, true);           // 07
//StaticIoiInt    so08("adrf_health_ind", 0, true);                   // 08
//StaticIoiStr    so09("adrf_rtc_source", rtcSource, 2, true);        // 09
//StaticIoiIntPtr so10("adrf_rtc_time", adrfTime, 2, true);           // 10
//StaticIoiIntPtr so11("adrf_ships_time", shipTime, 2, true);         // 11
//StaticIoiInt    so12("adrf_status_word1", 0, true);                 // 12
//StaticIoiInt    so13("adrf_status_word2", 0, true);                 // 13
//StaticIoiByte   so14("rtc_io_wr_date", 0, true);                    // 14
//StaticIoiByte   so15("rtc_io_wr_day", 0, true);                     // 15
//StaticIoiByte   so16("rtc_io_wr_hour", 0, true);                    // 16
//StaticIoiByte   so17("rtc_io_wr_minutes", 0, true);                 // 17
//StaticIoiByte   so18("rtc_io_wr_month", 0, true);                   // 18
//StaticIoiByte   so19("rtc_io_wr_seconds", 0, true);                 // 19
//StaticIoiByte   so20("rtc_io_wr_year", 0, true);                    // 20
//StaticIoiInt    so21("adrf_data_batt_timer", 0, true);              // 21
//
//// A429 Floating point values, so pass them back as integer
//StaticIoiInt so22("8204050_32", 0, true); // 22 - 0x7D2F1220, # A Engine Inlet Angle 1
//StaticIoiInt so23("8204051_32", 0, true); // 23 - 0x7D2F1320, # A Idle Time, A
//StaticIoiInt so24("8204052_32", 0, true); // 24 - 0x7D2F1420, # A PAT Total Test Time
//StaticIoiInt so25("8204053_32", 0, true); // 25 - 0x7D2F1520, # A Shutdown time Delay
//StaticIoiInt so26("8204054_32", 0, true); // 26 - 0x7D2F1620, # A Wind Speed
//StaticIoiInt so27("8204055_32", 0, true); // 27 - 0x7D2F1720, # A N1 Setting Tolerance
//StaticIoiInt so28("8204056_32", 0, true); // 28 - 0x7D2F1820, # A Test Stabilization Time
//StaticIoiInt so29("8204059_32", 0, true); // 29 - 0x7D2F1B20, # A PAT_MIN_IDLE_TREMAIN
//StaticIoiInt so30("8204060_32", 0, true); // 30 - 0x7D2F1C20, # A PAT_N1_TARGET
//StaticIoiInt so31("8204061_32", 0, true); // 31 - 0x7D2F1D20, # A PAT_N1_TARGET_TIME
//StaticIoiInt so32("8204062_32", 0, true); // 32 - 0x7D2F1E20, # A PAT_IDLE_CLOSE_TIME
//StaticIoiInt so33("8204063_32", 0, true); // 33 - 0x7D2F1F20, # A PAT_N1_STABLE_AVG
//StaticIoiInt so34("8204064_32", 0, true); // 34 - 0x7D2F2020, # A PAT_N2_STABLE_AVG
//StaticIoiInt so35("8204065_32", 0, true); // 35 - 0x7D2F2120, # A PAT_N2_LIM_MIN
//StaticIoiInt so36("8204066_32", 0, true); // 36 - 0x7D2F2220, # A PAT_N2_LIM_MAX
//StaticIoiInt so37("8204067_32", 0, true); // 37 - 0x7D2F2320, # A PAT_EGT_STABLE_AVG
//StaticIoiInt so38("8204068_32", 0, true); // 38 - 0x7D2F2420, # A PAT_EGT_LIM_MIN
//StaticIoiInt so39("8204069_32", 0, true); // 39 - 0x7D2F2520, # A PAT_EGT_LIM_MAX
//StaticIoiInt so40("8204070_32", 0, true); // 40 - 0x7D2F2620, # A PAT_WF_STABLE_AVG
//StaticIoiInt so41("8204071_32", 0, true); // 41 - 0x7D2F2720, # A PAT_WF_LIM_MIN
//StaticIoiInt so42("8204072_32", 0, true); // 42 - 0x7D2F2820, # A PAT_WF_LIM_MAX
//StaticIoiInt so43("8204057_32", 0, true); // 43 - 0x7D2F1920, # A Sts Wd1
//StaticIoiInt so44("8204058_32", 0, true); // 44 - 0x7D2F1A20, # A Sts Wd2
//
//// A429 Floating point values, so pass them back as integer
//StaticIoiInt so45("8204050_64", 0, true); // 45 - 0x7D2F1240, # B Engine Inlet Angle 1
//StaticIoiInt so46("8204051_64", 0, true); // 46 - 0x7D2F1340, # B Idle Time, A
//StaticIoiInt so47("8204052_64", 0, true); // 47 - 0x7D2F1440, # B PAT Total Test Time
//StaticIoiInt so48("8204053_64", 0, true); // 48 - 0x7D2F1540, # B Shutdown time Delay
//StaticIoiInt so49("8204054_64", 0, true); // 49 - 0x7D2F1640, # B Wind Speed
//StaticIoiInt so50("8204055_64", 0, true); // 50 - 0x7D2F1740, # B N1 Setting Tolerance
//StaticIoiInt so51("8204056_64", 0, true); // 51 - 0x7D2F1840, # B Test Stabilization Time
//StaticIoiInt so52("8204059_64", 0, true); // 52 - 0x7D2F1B40, # B PAT_MIN_IDLE_TREMAIN
//StaticIoiInt so53("8204060_64", 0, true); // 53 - 0x7D2F1C40, # B PAT_N1_TARGET
//StaticIoiInt so54("8204061_64", 0, true); // 54 - 0x7D2F1D40, # B PAT_N1_TARGET_TIME
//StaticIoiInt so55("8204062_64", 0, true); // 55 - 0x7D2F1E40, # B PAT_IDLE_CLOSE_TIME
//StaticIoiInt so56("8204063_64", 0, true); // 56 - 0x7D2F1F40, # B PAT_N1_STABLE_AVG
//StaticIoiInt so57("8204064_64", 0, true); // 57 - 0x7D2F2040, # B PAT_N2_STABLE_AVG
//StaticIoiInt so58("8204065_64", 0, true); // 58 - 0x7D2F2140, # B PAT_N2_LIM_MIN
//StaticIoiInt so59("8204066_64", 0, true); // 59 - 0x7D2F2240, # B PAT_N2_LIM_MAX
//StaticIoiInt so60("8204067_64", 0, true); // 60 - 0x7D2F2340, # B PAT_EGT_STABLE_AVG
//StaticIoiInt so61("8204068_64", 0, true); // 61 - 0x7D2F2440, # B PAT_EGT_LIM_MIN
//StaticIoiInt so62("8204069_64", 0, true); // 62 - 0x7D2F2540, # B PAT_EGT_LIM_MAX
//StaticIoiInt so63("8204070_64", 0, true); // 63 - 0x7D2F2640, # B PAT_WF_STABLE_AVG
//StaticIoiInt so64("8204071_64", 0, true); // 64 - 0x7D2F2740, # B PAT_WF_LIM_MIN
//StaticIoiInt so65("8204072_64", 0, true); // 65 - 0x7D2F2840, # B PAT_WF_LIM_MAX
//StaticIoiInt so66("8204057_64", 0, true); // 66 - 0x7D2F1940, # B Sts Wd1
//StaticIoiInt so67("8204058_64", 0, true); // 67 - 0x7D2F1A40, # B Sts Wd2
//// and two that we originally missed that do not have NDO numbers ...
//StaticIoiInt so68("adrf_pat_udt_remain_a", 0, true); // 68 adrf_pat_udt_remain_a 
//StaticIoiInt so69("adrf_pat_udt_remain_b", 0, true); // 69 adrf_pat_udt_remain_b 

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

    memset(&_ADRF_FAULT_DATA,    0, sizeof(_ADRF_FAULT_DATA));
    memset(&_adrf_rtc_time,      0, sizeof(_adrf_rtc_time));
    memset(&_adrf_ships_time,    0, sizeof(_adrf_ships_time));
    memset(&_adrf_data_operator, 0, sizeof(_adrf_data_operator));
    memset(&_adrf_data_owner,    0, sizeof(_adrf_data_owner));
    memset(&_adrf_rtc_source,    0, sizeof(_adrf_rtc_source));

    for (int i = 0; i < ASE_IN_MAX; ++i)
    {
        m_staticIoiIn[i] = aseIoiIn[i];
        // Find the following ioiNames and keep track of them
        if (strcmp(aseIoiIn[i]->ioiName, 'rtc_io_wr_seconds') == 0) m_ssXi = i;
        if (strcmp(aseIoiIn[i]->ioiName, 'rtc_io_wr_minutes') == 0) m_mmXi = i;
        if (strcmp(aseIoiIn[i]->ioiName, 'rtc_io_wr_hour')    == 0) m_hhXi = i;
        if (strcmp(aseIoiIn[i]->ioiName, 'rtc_io_wr_date')    == 0) m_ddXi = i;
        if (strcmp(aseIoiIn[i]->ioiName, 'rtc_io_wr_month')   == 0) m_moXi = i;
        if (strcmp(aseIoiIn[i]->ioiName, 'rtc_io_wr_year')    == 0) m_yyXi = i;
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
    if (lastYrCnt  != m_staticIoiIn[m_yyXi]->m_updateCount &&
        lastMoCnt  != m_staticIoiIn[m_mmXi]->m_updateCount &&
        lastDayCnt != m_staticIoiIn[m_ddXi]->m_updateCount &&
        lastHrCnt  != m_staticIoiIn[m_hhXi]->m_updateCount &&
        lastMinCnt != m_staticIoiIn[m_mmXi]->m_updateCount &&
        lastSecCnt != m_staticIoiIn[m_ssXi]->m_updateCount
        )
    {
        lastYrCnt  = m_staticIoiIn[m_yyXi]->m_updateCount;
        lastMoCnt  = m_staticIoiIn[m_mmXi]->m_updateCount;
        lastDayCnt = m_staticIoiIn[m_ddXi]->m_updateCount;
        lastHrCnt  = m_staticIoiIn[m_hhXi]->m_updateCount;
        lastMinCnt = m_staticIoiIn[m_mmXi]->m_updateCount;
        lastSecCnt = m_staticIoiIn[m_ssXi]->m_updateCount;

        // Move the new values into RTC time
        // sec: data = tens << 4 | ones;
        aseCommon.clocks[eClkRtc].m_time.tm_year = VALUE(m_staticIoiIn[m_yyXi]->data) + 2000;
        aseCommon.clocks[eClkRtc].m_time.tm_mon  = VALUE(m_staticIoiIn[m_mmXi]->data);
        aseCommon.clocks[eClkRtc].m_time.tm_mday = VALUE(m_staticIoiIn[m_ddXi]->data);
        aseCommon.clocks[eClkRtc].m_time.tm_hour = VALUE(m_staticIoiIn[m_hhXi]->data);
        aseCommon.clocks[eClkRtc].m_time.tm_min  = VALUE(m_staticIoiIn[m_mmXi]->data);
        aseCommon.clocks[eClkRtc].m_time.tm_sec  = VALUE(m_staticIoiIn[m_ssXi]->data);
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
    si40.data = 0;  // pat_scr = 0 : the main screen
    si41.data = 0;  // pat_button = 0 : 0

    ResetApatIoi();
}

void StaticIoiContainer::ResetApatIoi()
{
    // and values we want to reset if no script is running
    // use 0xffffff to indicate the value has not been updated

    so22.data = 0xffffff;
    so23.data = 0xffffff;
    so24.data = 0xffffff;
    so25.data = 0xffffff;
    so26.data = 0xffffff;
    so27.data = 0xffffff;
    so28.data = 0xffffff;
    so29.data = 0xffffff;
    so30.data = 0xffffff;
    so31.data = 0xffffff;
    so32.data = 0xffffff;
    so33.data = 0xffffff;
    so34.data = 0xffffff;
    so35.data = 0xffffff;
    so36.data = 0xffffff;
    so37.data = 0xffffff;
    so38.data = 0xffffff;
    so39.data = 0xffffff;
    so40.data = 0xffffff;
    so41.data = 0xffffff;
    so42.data = 0xffffff;
    so43.data = 0xffffff;
    so44.data = 0xffffff;

    so45.data = 0xffffff;
    so46.data = 0xffffff;
    so47.data = 0xffffff;
    so48.data = 0xffffff;
    so49.data = 0xffffff;
    so50.data = 0xffffff;
    so51.data = 0xffffff;
    so52.data = 0xffffff;
    so53.data = 0xffffff;
    so54.data = 0xffffff;
    so55.data = 0xffffff;
    so56.data = 0xffffff;
    so57.data = 0xffffff;
    so58.data = 0xffffff;
    so59.data = 0xffffff;
    so60.data = 0xffffff;
    so61.data = 0xffffff;
    so62.data = 0xffffff;
    so63.data = 0xffffff;
    so64.data = 0xffffff;
    so65.data = 0xffffff;
    so66.data = 0xffffff;
    so67.data = 0xffffff;

    so68.data = 0xffffff;
    so69.data = 0xffffff;
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
