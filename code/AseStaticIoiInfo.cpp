//
//----------------------------------------------------------------------------------------
// This file has been auto-generated as part of the ADRF/ASE IOI XML update process.
// ********************* PLEASE DO NOT MODIFY THIS FILE BY HAND. *************************
//----------------------------------------------------------------------------------------
#ifndef AseStaticIoi_h
#define AseStaticIoi_h

#include "ioiStatic.h"

#define ASE_IN_MAX 70
#define ASE_OUT_MAX 93

// ------------------- U U T   I N P U T S ---------------------------
// -------------------- ASE Produced - ADRf Consumed -----------------
// ------ The Order MUST match aseOutMap in PyStaticIoiInfo.py -------
char _rtc_io_rd_date[1];
char _rtc_io_rd_day[1];
char _rtc_io_rd_hour[1];
char _rtc_io_rd_minutes[1];
char _rtc_io_rd_month[1];
char _rtc_io_rd_seconds[1];
char _rtc_io_rd_year[1];
char _HMUPartNumber[16];
char _HMUSerialNumber[16];
char _PWSwDwgNumber[17];
char _UTASSwDwgNumber[17];
char _1738780_32[8];
char _1738780_64[8];
char _231680_32[8];
char _231680_64[8];
char _231688_32[8];
char _231688_64[8];
char _231689_32[8];
char _231689_64[8];
char _231696_32[8];
char _231696_64[8];
char _231704_32[8];
char _231704_64[8];
char _231712_32[12];
char _231712_64[12];
char _231720_32[12];
char _231720_64[12];
char _231728_32[16];
char _231728_64[16];
char _231782_32[12];
char _231782_64[12];
char _231790_32[20];
char _231790_64[20];
char _231798_32[8];
char _231798_64[8];
char _231806_32[16];
char _231806_64[16];
char _231838_32[4];
char _231838_64[4];
char _231846_32[8];
char _231846_64[8];
char _231854_32[8];
char _231854_64[8];
char _231862_32[8];
char _231862_64[8];
char _231878_32[40];
char _231878_64[40];
char _231886_32[4];
char _231886_64[4];
char _231918_32[32];
char _231918_64[32];
char _8313856_1[240];
char _8313856_16[240];
char _8313864_20[240];
char _8313864_35[240];
char _a664_to_ioc_eicas[4096];

StaticIoiInt   si0("aircraft_id_1", 0);
StaticIoiInt   si1("aircraft_id_2", 0);
StaticIoiInt   si2("aircraft_id_3", 0);
StaticIoiInt   si3("aircraft_id_4", 0);
StaticIoiInt   si4("aircraft_id_5", 0);
StaticIoiInt   si5("aircraft_id_par", 0);
StaticIoiInt   si6("disc_spare_1", 0);
StaticIoiInt   si7("disc_spare_2", 0);
StaticIoiInt   si8("disc_spare_3", 0);
StaticIoiInt   si9("ground_service_mode", 0);
StaticIoiInt   si10("weight_on_wheels", 0);
StaticIoiInt   si11("wifi_override", 0);
StaticIoiStr   si12("rtc_io_rd_date", _rtc_io_rd_date, 1);
StaticIoiStr   si13("rtc_io_rd_day", _rtc_io_rd_day, 1);
StaticIoiStr   si14("rtc_io_rd_hour", _rtc_io_rd_hour, 1);
StaticIoiStr   si15("rtc_io_rd_minutes", _rtc_io_rd_minutes, 1);
StaticIoiStr   si16("rtc_io_rd_month", _rtc_io_rd_month, 1);
StaticIoiStr   si17("rtc_io_rd_seconds", _rtc_io_rd_seconds, 1);
StaticIoiStr   si18("rtc_io_rd_year", _rtc_io_rd_year, 1);
StaticIoiInt   si19("ubmf_health_ind", 0);
StaticIoiInt   si20("umbf_status_word1", 0);
StaticIoiInt   si21("BrdTempFail", 0);
StaticIoiInt   si22("BrdTempInitFail", 0);
StaticIoiInt   si23("BrdTempOpFail", 0);
StaticIoiInt   si24("BrdTempRngFail", 0);
StaticIoiInt   si25("HLEIFFaultInd2", 0);
StaticIoiInt   si26("HLEIFFaultIndication", 0);
StaticIoiInt   si27("HLEIFStatusWord1", 0);
StaticIoiInt   si28("hmu_option_data_raw", 0);
StaticIoiInt   si29("micro_server_health_ind1", 0);
StaticIoiInt   si30("micro_server_health_ind2", 0);
StaticIoiInt   si31("micro_server_internal_status", 0);
StaticIoiInt   si32("micro_server_status_word1", 0);
StaticIoiFloat si33("BatInputVdc", 0.0f);
StaticIoiFloat si34("BatSwOutVdc", 0.0f);
StaticIoiFloat si35("BrdTempDegC", 0.0f);
StaticIoiStr   si36("HMUPartNumber", _HMUPartNumber, 16);
StaticIoiStr   si37("HMUSerialNumber", _HMUSerialNumber, 16);
StaticIoiStr   si38("PWSwDwgNumber", _PWSwDwgNumber, 17);
StaticIoiStr   si39("UTASSwDwgNumber", _UTASSwDwgNumber, 17);
StaticIoiInt   si40("pat_scr", 0);
StaticIoiInt   si41("pat_scr_button", 0);
StaticIoiInt   si42("fault_word3", 0);
StaticIoiInt   si43("fault_word4", 0);
StaticIoiInt   si44("prg_flash_cbit_fail", 0);
StaticIoiInt   si45("fault_word7", 0);
StaticIoiInt   si46("hleif_eicas_annun_tx", 0);
StaticIoiInt   si47("hmu_eicas_annun", 0);
StaticIoiStr   si48("1738780_32", _1738780_32, 8);
StaticIoiStr   si49("1738780_64", _1738780_64, 8);
StaticIoiStr   si50("231680_32", _231680_32, 8);
StaticIoiStr   si51("231680_64", _231680_64, 8);
StaticIoiStr   si52("231688_32", _231688_32, 8);
StaticIoiStr   si53("231688_64", _231688_64, 8);
StaticIoiStr   si54("231689_32", _231689_32, 8);
StaticIoiStr   si55("231689_64", _231689_64, 8);
StaticIoiStr   si56("231696_32", _231696_32, 8);
StaticIoiStr   si57("231696_64", _231696_64, 8);
StaticIoiStr   si58("231704_32", _231704_32, 8);
StaticIoiStr   si59("231704_64", _231704_64, 8);
StaticIoiStr   si60("231712_32", _231712_32, 12);
StaticIoiStr   si61("231712_64", _231712_64, 12);
StaticIoiStr   si62("231720_32", _231720_32, 12);
StaticIoiStr   si63("231720_64", _231720_64, 12);
StaticIoiStr   si64("231728_32", _231728_32, 16);
StaticIoiStr   si65("231728_64", _231728_64, 16);
StaticIoiStr   si66("231782_32", _231782_32, 12);
StaticIoiStr   si67("231782_64", _231782_64, 12);
StaticIoiStr   si68("231790_32", _231790_32, 20);
StaticIoiStr   si69("231790_64", _231790_64, 20);
StaticIoiStr   si70("231798_32", _231798_32, 8);
StaticIoiStr   si71("231798_64", _231798_64, 8);
StaticIoiStr   si72("231806_32", _231806_32, 16);
StaticIoiStr   si73("231806_64", _231806_64, 16);
StaticIoiStr   si74("231838_32", _231838_32, 4);
StaticIoiStr   si75("231838_64", _231838_64, 4);
StaticIoiStr   si76("231846_32", _231846_32, 8);
StaticIoiStr   si77("231846_64", _231846_64, 8);
StaticIoiStr   si78("231854_32", _231854_32, 8);
StaticIoiStr   si79("231854_64", _231854_64, 8);
StaticIoiStr   si80("231862_32", _231862_32, 8);
StaticIoiStr   si81("231862_64", _231862_64, 8);
StaticIoiStr   si82("231878_32", _231878_32, 40);
StaticIoiStr   si83("231878_64", _231878_64, 40);
StaticIoiStr   si84("231886_32", _231886_32, 4);
StaticIoiStr   si85("231886_64", _231886_64, 4);
StaticIoiStr   si86("231918_32", _231918_32, 32);
StaticIoiStr   si87("231918_64", _231918_64, 32);
StaticIoiStr   si88("8313856_1", _8313856_1, 240);
StaticIoiStr   si89("8313856_16", _8313856_16, 240);
StaticIoiStr   si90("8313864_20", _8313864_20, 240);
StaticIoiStr   si91("8313864_35", _8313864_35, 240);
StaticIoiStr   si92("a664_to_ioc_eicas", _a664_to_ioc_eicas, 4096);

StaticIoiObj* aseIoiOut[ASE_OUT_MAX] = {
    &si0,
    &si1,
    &si2,
    &si3,
    &si4,
    &si5,
    &si6,
    &si7,
    &si8,
    &si9,
    &si10,
    &si11,
    &si12,
    &si13,
    &si14,
    &si15,
    &si16,
    &si17,
    &si18,
    &si19,
    &si20,
    &si21,
    &si22,
    &si23,
    &si24,
    &si25,
    &si26,
    &si27,
    &si28,
    &si29,
    &si30,
    &si31,
    &si32,
    &si33,
    &si34,
    &si35,
    &si36,
    &si37,
    &si38,
    &si39,
    &si40,
    &si41,
    &si42,
    &si43,
    &si44,
    &si45,
    &si46,
    &si47,
    &si48,
    &si49,
    &si50,
    &si51,
    &si52,
    &si53,
    &si54,
    &si55,
    &si56,
    &si57,
    &si58,
    &si59,
    &si60,
    &si61,
    &si62,
    &si63,
    &si64,
    &si65,
    &si66,
    &si67,
    &si68,
    &si69,
    &si70,
    &si71,
    &si72,
    &si73,
    &si74,
    &si75,
    &si76,
    &si77,
    &si78,
    &si79,
    &si80,
    &si81,
    &si82,
    &si83,
    &si84,
    &si85,
    &si86,
    &si87,
    &si88,
    &si89,
    &si90,
    &si91,
    &si92,
};

// -------------------- U U T   O U T P U T S ------------------------
// -------------------- ASE Consumed - ADRf Produced -----------------
// ------ The Order MUST match aseInMap in PyStaticIoiInfo.py -------
char _ADRF_FAULT_DATA[52];
char _adrf_data_operator[32];
char _adrf_data_owner[32];
char _adrf_rtc_source[2];
char _adrf_rtc_time[6];
char _adrf_ships_time[6];
char _rtc_io_wr_date[1];
char _rtc_io_wr_day[1];
char _rtc_io_wr_hour[1];
char _rtc_io_wr_minutes[1];
char _rtc_io_wr_month[1];
char _rtc_io_wr_seconds[1];
char _rtc_io_wr_year[1];

StaticIoiStr   so0("ADRF_FAULT_DATA", _ADRF_FAULT_DATA, 52, true);
StaticIoiInt   so1("adrf_apm_wrap", 0, true);
StaticIoiInt   so2("adrf_data_cumflt_time", 0, true);
StaticIoiInt   so3("adrf_data_flt_time_current", 0, true);
StaticIoiStr   so4("adrf_data_operator", _adrf_data_operator, 32, true);
StaticIoiStr   so5("adrf_data_owner", _adrf_data_owner, 32, true);
StaticIoiInt   so6("adrf_data_power_on_cnt", 0, true);
StaticIoiInt   so7("adrf_data_power_on_time", 0, true);
StaticIoiInt   so8("adrf_health_ind", 0, true);
StaticIoiStr   so9("adrf_rtc_source", _adrf_rtc_source, 2, true);
StaticIoiStr   so10("adrf_rtc_time", _adrf_rtc_time, 6, true);
StaticIoiStr   so11("adrf_ships_time", _adrf_ships_time, 6, true);
StaticIoiInt   so12("adrf_status_word1", 0, true);
StaticIoiInt   so13("adrf_status_word2", 0, true);
StaticIoiStr   so14("rtc_io_wr_date", _rtc_io_wr_date, 1, true);
StaticIoiStr   so15("rtc_io_wr_day", _rtc_io_wr_day, 1, true);
StaticIoiStr   so16("rtc_io_wr_hour", _rtc_io_wr_hour, 1, true);
StaticIoiStr   so17("rtc_io_wr_minutes", _rtc_io_wr_minutes, 1, true);
StaticIoiStr   so18("rtc_io_wr_month", _rtc_io_wr_month, 1, true);
StaticIoiStr   so19("rtc_io_wr_seconds", _rtc_io_wr_seconds, 1, true);
StaticIoiStr   so20("rtc_io_wr_year", _rtc_io_wr_year, 1, true);
StaticIoiInt   so21("adrf_data_batt_timer", 0, true);
StaticIoiInt   so22("8204050_64", 0, true);
StaticIoiInt   so23("8204050_32", 0, true);
StaticIoiInt   so24("8204051_32", 0, true);
StaticIoiInt   so25("8204051_64", 0, true);
StaticIoiInt   so26("8204052_32", 0, true);
StaticIoiInt   so27("8204052_64", 0, true);
StaticIoiInt   so28("8204053_32", 0, true);
StaticIoiInt   so29("8204053_64", 0, true);
StaticIoiInt   so30("8204054_32", 0, true);
StaticIoiInt   so31("8204054_64", 0, true);
StaticIoiInt   so32("8204055_32", 0, true);
StaticIoiInt   so33("8204055_64", 0, true);
StaticIoiInt   so34("8204056_32", 0, true);
StaticIoiInt   so35("8204056_64", 0, true);
StaticIoiInt   so36("8204057_32", 0, true);
StaticIoiInt   so37("8204057_64", 0, true);
StaticIoiInt   so38("8204058_32", 0, true);
StaticIoiInt   so39("8204058_64", 0, true);
StaticIoiInt   so40("8204059_32", 0, true);
StaticIoiInt   so41("8204059_64", 0, true);
StaticIoiInt   so42("8204060_32", 0, true);
StaticIoiInt   so43("8204060_64", 0, true);
StaticIoiInt   so44("8204061_32", 0, true);
StaticIoiInt   so45("8204061_64", 0, true);
StaticIoiInt   so46("8204062_32", 0, true);
StaticIoiInt   so47("8204062_64", 0, true);
StaticIoiInt   so48("8204063_32", 0, true);
StaticIoiInt   so49("8204063_64", 0, true);
StaticIoiInt   so50("8204064_32", 0, true);
StaticIoiInt   so51("8204064_64", 0, true);
StaticIoiInt   so52("8204065_32", 0, true);
StaticIoiInt   so53("8204065_64", 0, true);
StaticIoiInt   so54("8204066_32", 0, true);
StaticIoiInt   so55("8204066_64", 0, true);
StaticIoiInt   so56("8204067_32", 0, true);
StaticIoiInt   so57("8204067_64", 0, true);
StaticIoiInt   so58("8204068_32", 0, true);
StaticIoiInt   so59("8204068_64", 0, true);
StaticIoiInt   so60("8204069_32", 0, true);
StaticIoiInt   so61("8204069_64", 0, true);
StaticIoiInt   so62("8204070_32", 0, true);
StaticIoiInt   so63("8204071_32", 0, true);
StaticIoiInt   so64("8204070_64", 0, true);
StaticIoiInt   so65("8204071_64", 0, true);
StaticIoiInt   so66("8204072_32", 0, true);
StaticIoiInt   so67("8204072_64", 0, true);
StaticIoiInt   so68("adrf_pat_udt_remain_b", 0, true);
StaticIoiInt   so69("adrf_pat_udt_remain_a", 0, true);

StaticIoiObj* aseIoiIn[ASE_IN_MAX] = {
    &so0,
    &so1,
    &so2,
    &so3,
    &so4,
    &so5,
    &so6,
    &so7,
    &so8,
    &so9,
    &so10,
    &so11,
    &so12,
    &so13,
    &so14,
    &so15,
    &so16,
    &so17,
    &so18,
    &so19,
    &so20,
    &so21,
    &so22,
    &so23,
    &so24,
    &so25,
    &so26,
    &so27,
    &so28,
    &so29,
    &so30,
    &so31,
    &so32,
    &so33,
    &so34,
    &so35,
    &so36,
    &so37,
    &so38,
    &so39,
    &so40,
    &so41,
    &so42,
    &so43,
    &so44,
    &so45,
    &so46,
    &so47,
    &so48,
    &so49,
    &so50,
    &so51,
    &so52,
    &so53,
    &so54,
    &so55,
    &so56,
    &so57,
    &so58,
    &so59,
    &so60,
    &so61,
    &so62,
    &so63,
    &so64,
    &so65,
    &so66,
    &so67,
    &so68,
    &so69,
};

#endif
