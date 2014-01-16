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
StaticIoiByte  si17("rtc_io_rd_year", 0);                      // 17
StaticIoiInt   si18("ubmf_health_ind", 0);                     // 18
StaticIoiInt   si19("umbf_status_word1", 0);                   // 19
StaticIoiInt   si20("flight_leg_raw", 1);                      // 20
StaticIoiInt   si21("umbf_status_word1", 0);                   // 21
StaticIoiInt   si22("BrdTempFail", 0);                         // 22
StaticIoiInt   si23("BrdTempInitFail", 0);                     // 23
StaticIoiInt   si24("BrdTempOpFail", 0);                       // 24
StaticIoiInt   si25("BrdTempRngFail", 0);                      // 25
StaticIoiInt   si26("HLEIFFaultInd2", 0);                      // 26
StaticIoiInt   si27("HLEIFFaultIndication", 0);                // 27
StaticIoiInt   si28("HLEIFStatusWord1", 0);                    // 28
StaticIoiInt   si29("hmu_option_data_raw", 0);                 // 29
StaticIoiInt   si30("micro_server_health_ind1", 0);            // 30
StaticIoiInt   si31("micro_server_health_ind2", 0);            // 31
StaticIoiInt   si32("micro_server_internal_status", 0);        // 32
StaticIoiInt   si33("micro_server_status_word1", 0);           // 33
StaticIoiInt   si34("ac_tail_num_high_raw", 0);                // 34
StaticIoiInt   si35("ac_tail_num_low_raw", 0);                 // 35
StaticIoiInt   si36("ac_tail_num_mid_raw", 0);                 // 36
StaticIoiFloat si37("BatInputVdc", 27.9f);                     // 37
StaticIoiFloat si38("BatSwOutVdc", 28.2f);                     // 38
StaticIoiFloat si39("BrdTempDegC", 10.0f);                     // 39
StaticIoiStr   si40("HMUPartNumber", HMUpartNumber);           // 40
StaticIoiStr   si41("HMUSerialNumber", HMUSerialNumber);       // 41
StaticIoiStr   si42("PWSwDwgNumber", PWSwDwgNumber);           // 42
StaticIoiStr   si43("UTASSwDwgNumber", UTASSwDwgNumber);       // 43

//----------------------------------------------------------------------------/
// Local Function Prototypes                                                 -/
//----------------------------------------------------------------------------/

//----------------------------------------------------------------------------/
// Public Functions                                                          -/
//----------------------------------------------------------------------------/

//----------------------------------------------------------------------------/
// Class Definitions                                                         -/
//----------------------------------------------------------------------------/
StaticIoiObj::StaticIoiObj(char* name)
{
    strcpy(ioiName, name);
    strcpy(m_shortName, name);
    CompressName(m_shortName, 18);
}

//---------------------------------------------------------------------------------------------
bool StaticIoiObj::OpenIoi()
{
    ioiValid = ioi_open(ioiName, ioiWritePermission, (int*)&ioiChan) == ioiSuccess;
    return ioiValid;
}

//---------------------------------------------------------------------------------------------
bool StaticIoiObj::WriteStaticIoi(void* data)
{
    ioiStatus writeStatus = (ioiStatus)42;

    if (ioiValid)
    {
        writeStatus = ioi_write(ioiChan, data);
    }

    return writeStatus == ioiSuccess;
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


//=============================================================================================
//IocResponse StaticIoiByte::GetStaticIoiData()
//{
//
//}

//---------------------------------------------------------------------------------------------
bool StaticIoiByte::SetStaticIoiData( SecRequest& request )
{
    data = (unsigned char)request.resetRequest;
    Update();
    return true;
}

//---------------------------------------------------------------------------------------------
char* StaticIoiByte::Display( char* dest, UINT32 dix )
{
    sprintf(dest, "%2d:%s: %d 0x%02x", dix, m_shortName, data, data);
    return dest;
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
    Update();
    return true;
}

//---------------------------------------------------------------------------------------------
char* StaticIoiInt::Display( char* dest, UINT32 dix )
{
    sprintf(dest, "%2d:%s: %d 0x%08x", dix, m_shortName, data, data);
    return dest;
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
    Update();
    return true;
}

//---------------------------------------------------------------------------------------------
char* StaticIoiFloat::Display( char* dest, UINT32 dix )
{
    sprintf(dest, "%2d:%s: %f", dix, m_shortName, data);
    return dest;
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
    Update();
    return true;
}

//---------------------------------------------------------------------------------------------
char* StaticIoiStr::Display( char* dest, UINT32 dix )
{
    sprintf(dest, "%2d:%s: %s", dix, m_shortName, data);
    return dest;
}

//=============================================================================================
StaticIoiContainer::StaticIoiContainer()
{

    strcpy(HMUpartNumber, "HmuPart");
    strcpy(HMUSerialNumber, "HmuSerial");
    strcpy(PWSwDwgNumber, "PwSwDwg");
    strcpy(UTASSwDwgNumber, "UtasSwDwg");


    // The Order MUST match AseIoiInfo in eFastCmds.py
    m_staticIoi[ 0] = &si00;  // 00
    m_staticIoi[ 1] = &si01;  // 01
    m_staticIoi[ 2] = &si02;  // 02
    m_staticIoi[ 3] = &si03;  // 03
    m_staticIoi[ 4] = &si04;  // 04
    m_staticIoi[ 5] = &si05;  // 05
    m_staticIoi[ 6] = &si06;  // 06
    m_staticIoi[ 7] = &si07;  // 07
    m_staticIoi[ 8] = &si08;  // 08
    m_staticIoi[ 9] = &si09;  // 09
    m_staticIoi[10] = &si10;  // 10
    m_staticIoi[11] = &si11;  // 11
    m_staticIoi[12] = &si12;  // 12
    m_staticIoi[13] = &si13;  // 13
    m_staticIoi[14] = &si14;  // 14
    m_staticIoi[15] = &si15;  // 15
    m_staticIoi[16] = &si16;  // 16
    m_staticIoi[17] = &si17;  // 17
    m_staticIoi[18] = &si18;  // 18
    m_staticIoi[19] = &si19;  // 19
    m_staticIoi[20] = &si20;  // 20
    m_staticIoi[21] = &si21;  // 21
    m_staticIoi[22] = &si22;  // 22
    m_staticIoi[23] = &si23;  // 23
    m_staticIoi[24] = &si24;  // 24
    m_staticIoi[25] = &si25;  // 25
    m_staticIoi[26] = &si26;  // 26
    m_staticIoi[27] = &si27;  // 27
    m_staticIoi[28] = &si28;  // 28
    m_staticIoi[29] = &si29;  // 29
    m_staticIoi[30] = &si30;  // 30
    m_staticIoi[31] = &si31;  // 31
    m_staticIoi[32] = &si32;  // 32
    m_staticIoi[33] = &si33;  // 33
    m_staticIoi[34] = &si34;  // 34
    m_staticIoi[35] = &si35;  // 35
    m_staticIoi[36] = &si36;  // 36
    m_staticIoi[37] = &si37;  // 37
    m_staticIoi[38] = &si38;  // 38
    m_staticIoi[39] = &si39;  // 39
    m_staticIoi[40] = &si40;  // 40
    m_staticIoi[41] = &si41;  // 41
    m_staticIoi[42] = &si42;  // 42
    m_staticIoi[43] = &si43;  // 43

    m_ioiStaticCount = 44;
    m_updateIndex = 0;
    m_validIoi = 0;
}

//---------------------------------------------------------------------------------------------
void StaticIoiContainer::OpenIoi()
{
    for (int i =0; i < m_ioiStaticCount; ++i)
    {
        if (m_staticIoi[i]->OpenIoi())
        {
            m_validIoi += 1;
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
    if (request.variableId < m_ioiStaticCount)
    {
        return m_staticIoi[request.variableId]->SetStaticIoiData(request);
    }
    else
    {
        return false;
    }
}

//---------------------------------------------------------------------------------------------
void StaticIoiContainer::UpdateStaticIoi()
{
    // copy the current time into the rtc_ IOI
    // StaticIoiByte  si12("rtc_io_rd_date", 0);                      // 12
    // StaticIoiByte  si13("rtc_io_rd_day", 0);                       // 13
    // StaticIoiByte  si14("rtc_io_rd_hour", 0);                      // 14
    // StaticIoiByte  si15("rtc_io_rd_minutes", 0);                   // 15
    // StaticIoiByte  si16("rtc_io_rd_month", 0);                     // 16
    // StaticIoiByte  si17("rtc_io_rd_year", 0);                      // 17
    si12.data = aseCommon.time.tm_mday;
    si13.data = aseCommon.time.tm_mday;
    si14.data = aseCommon.time.tm_hour;
    si15.data = aseCommon.time.tm_min;
    si16.data = aseCommon.time.tm_mon;
    si17.data = aseCommon.time.tm_year - 2000;

    m_staticIoi[m_updateIndex]->Update();
    m_updateIndex += 1;
    if (m_updateIndex >= m_ioiStaticCount)
    {
        m_updateIndex = 0;
    }
}
