//
//----------------------------------------------------------------------------------------
// This file has been auto-generated as part of the ADRF/ASE IOI XML update process.
// ********************* PLEASE DO NOT MODIFY THIS FILE BY HAND. *************************
//----------------------------------------------------------------------------------------
#ifndef AseStaticIoi_h
#define AseStaticIoi_h

#include "ioiStatic.h"

#define ASE_IN_MAX 67
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
char _a664_fr_eicas2_fdr[3208];

StaticIoiInt                  _aircraft_id_1_("aircraft_id_1" , 0);                              // 0
StaticIoiInt                  _aircraft_id_2_("aircraft_id_2" , 0);                              // 1
StaticIoiInt                  _aircraft_id_3_("aircraft_id_3" , 0);                              // 2
StaticIoiInt                  _aircraft_id_4_("aircraft_id_4" , 0);                              // 3
StaticIoiInt                  _aircraft_id_5_("aircraft_id_5" , 0);                              // 4
StaticIoiInt                _aircraft_id_par_("aircraft_id_par" , 0);                            // 5
StaticIoiInt                   _disc_spare_1_("disc_spare_1" , 0);                               // 6
StaticIoiInt                   _disc_spare_2_("disc_spare_2" , 0);                               // 7
StaticIoiInt                   _disc_spare_3_("disc_spare_3" , 0);                               // 8
StaticIoiInt            _ground_service_mode_("ground_service_mode" , 0);                        // 9
StaticIoiInt               _weight_on_wheels_("weight_on_wheels" , 0);                           // 10
StaticIoiInt                  _wifi_override_("wifi_override" , 0);                              // 11
StaticIoiStr                 _rtc_io_rd_date_("rtc_io_rd_date", _rtc_io_rd_date, 1);             // 12
StaticIoiStr                  _rtc_io_rd_day_("rtc_io_rd_day", _rtc_io_rd_day, 1);               // 13
StaticIoiStr                 _rtc_io_rd_hour_("rtc_io_rd_hour", _rtc_io_rd_hour, 1);             // 14
StaticIoiStr              _rtc_io_rd_minutes_("rtc_io_rd_minutes", _rtc_io_rd_minutes, 1);       // 15
StaticIoiStr                _rtc_io_rd_month_("rtc_io_rd_month", _rtc_io_rd_month, 1);           // 16
StaticIoiStr              _rtc_io_rd_seconds_("rtc_io_rd_seconds", _rtc_io_rd_seconds, 1);       // 17
StaticIoiStr                 _rtc_io_rd_year_("rtc_io_rd_year", _rtc_io_rd_year, 1);             // 18
StaticIoiInt                _ubmf_health_ind_("ubmf_health_ind" , 0);                            // 19
StaticIoiInt              _umbf_status_word1_("umbf_status_word1" , 0);                          // 20
StaticIoiInt                    _BrdTempFail_("BrdTempFail" , 0);                                // 21
StaticIoiInt                _BrdTempInitFail_("BrdTempInitFail" , 0);                            // 22
StaticIoiInt                  _BrdTempOpFail_("BrdTempOpFail" , 0);                              // 23
StaticIoiInt                 _BrdTempRngFail_("BrdTempRngFail" , 0);                             // 24
StaticIoiInt                 _HLEIFFaultInd2_("HLEIFFaultInd2" , 0);                             // 25
StaticIoiInt           _HLEIFFaultIndication_("HLEIFFaultIndication" , 0);                       // 26
StaticIoiInt               _HLEIFStatusWord1_("HLEIFStatusWord1" , 0);                           // 27
StaticIoiInt            _hmu_option_data_raw_("hmu_option_data_raw" , 0);                        // 28
StaticIoiInt       _micro_server_health_ind1_("micro_server_health_ind1" , 0);                   // 29
StaticIoiInt       _micro_server_health_ind2_("micro_server_health_ind2" , 0);                   // 30
StaticIoiInt   _micro_server_internal_status_("micro_server_internal_status" , 0);               // 31
StaticIoiInt      _micro_server_status_word1_("micro_server_status_word1" , 0);                  // 32
StaticIoiFloat                  _BatInputVdc_("BatInputVdc", 0.0f);                              // 33
StaticIoiFloat                  _BatSwOutVdc_("BatSwOutVdc", 0.0f);                              // 34
StaticIoiFloat                  _BrdTempDegC_("BrdTempDegC", 0.0f);                              // 35
StaticIoiStr                  _HMUPartNumber_("HMUPartNumber", _HMUPartNumber, 16);              // 36
StaticIoiStr                _HMUSerialNumber_("HMUSerialNumber", _HMUSerialNumber, 16);          // 37
StaticIoiStr                  _PWSwDwgNumber_("PWSwDwgNumber", _PWSwDwgNumber, 17);              // 38
StaticIoiStr                _UTASSwDwgNumber_("UTASSwDwgNumber", _UTASSwDwgNumber, 17);          // 39
StaticIoiInt                    _fault_word3_("fault_word3" , 0);                                // 40
StaticIoiInt                    _fault_word4_("fault_word4" , 0);                                // 41
StaticIoiInt            _prg_flash_cbit_fail_("prg_flash_cbit_fail" , 0);                        // 42
StaticIoiInt                    _fault_word7_("fault_word7" , 0);                                // 43
StaticIoiInt           _hleif_eicas_annun_tx_("hleif_eicas_annun_tx" , 0);                       // 44
StaticIoiInt                _hmu_eicas_annun_("hmu_eicas_annun" , 0);                            // 45
StaticIoiStr                     _1738780_32_("1738780_32", _1738780_32, 8);                     // 46
StaticIoiStr                     _1738780_64_("1738780_64", _1738780_64, 8);                     // 47
StaticIoiStr                      _231680_32_("231680_32", _231680_32, 8);                       // 48
StaticIoiStr                      _231680_64_("231680_64", _231680_64, 8);                       // 49
StaticIoiStr                      _231688_32_("231688_32", _231688_32, 8);                       // 50
StaticIoiStr                      _231688_64_("231688_64", _231688_64, 8);                       // 51
StaticIoiStr                      _231689_32_("231689_32", _231689_32, 8);                       // 52
StaticIoiStr                      _231689_64_("231689_64", _231689_64, 8);                       // 53
StaticIoiStr                      _231696_32_("231696_32", _231696_32, 8);                       // 54
StaticIoiStr                      _231696_64_("231696_64", _231696_64, 8);                       // 55
StaticIoiStr                      _231704_32_("231704_32", _231704_32, 8);                       // 56
StaticIoiStr                      _231704_64_("231704_64", _231704_64, 8);                       // 57
StaticIoiStr                      _231712_32_("231712_32", _231712_32, 12);                      // 58
StaticIoiStr                      _231712_64_("231712_64", _231712_64, 12);                      // 59
StaticIoiStr                      _231720_32_("231720_32", _231720_32, 12);                      // 60
StaticIoiStr                      _231720_64_("231720_64", _231720_64, 12);                      // 61
StaticIoiStr                      _231728_32_("231728_32", _231728_32, 16);                      // 62
StaticIoiStr                      _231728_64_("231728_64", _231728_64, 16);                      // 63
StaticIoiStr                      _231782_32_("231782_32", _231782_32, 12);                      // 64
StaticIoiStr                      _231782_64_("231782_64", _231782_64, 12);                      // 65
StaticIoiStr                      _231790_32_("231790_32", _231790_32, 20);                      // 66
StaticIoiStr                      _231790_64_("231790_64", _231790_64, 20);                      // 67
StaticIoiStr                      _231798_32_("231798_32", _231798_32, 8);                       // 68
StaticIoiStr                      _231798_64_("231798_64", _231798_64, 8);                       // 69
StaticIoiStr                      _231806_32_("231806_32", _231806_32, 16);                      // 70
StaticIoiStr                      _231806_64_("231806_64", _231806_64, 16);                      // 71
StaticIoiStr                      _231838_32_("231838_32", _231838_32, 4);                       // 72
StaticIoiStr                      _231838_64_("231838_64", _231838_64, 4);                       // 73
StaticIoiStr                      _231846_32_("231846_32", _231846_32, 8);                       // 74
StaticIoiStr                      _231846_64_("231846_64", _231846_64, 8);                       // 75
StaticIoiStr                      _231854_32_("231854_32", _231854_32, 8);                       // 76
StaticIoiStr                      _231854_64_("231854_64", _231854_64, 8);                       // 77
StaticIoiStr                      _231862_32_("231862_32", _231862_32, 8);                       // 78
StaticIoiStr                      _231862_64_("231862_64", _231862_64, 8);                       // 79
StaticIoiStr                      _231878_32_("231878_32", _231878_32, 40);                      // 80
StaticIoiStr                      _231878_64_("231878_64", _231878_64, 40);                      // 81
StaticIoiStr                      _231886_32_("231886_32", _231886_32, 4);                       // 82
StaticIoiStr                      _231886_64_("231886_64", _231886_64, 4);                       // 83
StaticIoiStr                      _231918_32_("231918_32", _231918_32, 32);                      // 84
StaticIoiStr                      _231918_64_("231918_64", _231918_64, 32);                      // 85
StaticIoiStr                      _8313856_1_("8313856_1", _8313856_1, 240);                     // 86
StaticIoiStr                     _8313856_16_("8313856_16", _8313856_16, 240);                   // 87
StaticIoiStr                     _8313864_20_("8313864_20", _8313864_20, 240);                   // 88
StaticIoiStr                     _8313864_35_("8313864_35", _8313864_35, 240);                   // 89
StaticIoiStr             _a664_fr_eicas2_fdr_("a664_fr_eicas2_fdr", _a664_fr_eicas2_fdr, 3208);  // 90
StaticIoiInt                     _OMS_BUTTON_("OMS_BUTTON" , 0);                                 // 91
StaticIoiInt                     _OMS_PAGEID_("OMS_PAGEID" , 0);                                 // 92

StaticIoiObj* aseIoiOut[ASE_OUT_MAX] = {
    &_aircraft_id_1_                ,  // 0
    &_aircraft_id_2_                ,  // 1
    &_aircraft_id_3_                ,  // 2
    &_aircraft_id_4_                ,  // 3
    &_aircraft_id_5_                ,  // 4
    &_aircraft_id_par_              ,  // 5
    &_disc_spare_1_                 ,  // 6
    &_disc_spare_2_                 ,  // 7
    &_disc_spare_3_                 ,  // 8
    &_ground_service_mode_          ,  // 9
    &_weight_on_wheels_             ,  // 10
    &_wifi_override_                ,  // 11
    &_rtc_io_rd_date_               ,  // 12
    &_rtc_io_rd_day_                ,  // 13
    &_rtc_io_rd_hour_               ,  // 14
    &_rtc_io_rd_minutes_            ,  // 15
    &_rtc_io_rd_month_              ,  // 16
    &_rtc_io_rd_seconds_            ,  // 17
    &_rtc_io_rd_year_               ,  // 18
    &_ubmf_health_ind_              ,  // 19
    &_umbf_status_word1_            ,  // 20
    &_BrdTempFail_                  ,  // 21
    &_BrdTempInitFail_              ,  // 22
    &_BrdTempOpFail_                ,  // 23
    &_BrdTempRngFail_               ,  // 24
    &_HLEIFFaultInd2_               ,  // 25
    &_HLEIFFaultIndication_         ,  // 26
    &_HLEIFStatusWord1_             ,  // 27
    &_hmu_option_data_raw_          ,  // 28
    &_micro_server_health_ind1_     ,  // 29
    &_micro_server_health_ind2_     ,  // 30
    &_micro_server_internal_status_ ,  // 31
    &_micro_server_status_word1_    ,  // 32
    &_BatInputVdc_                  ,  // 33
    &_BatSwOutVdc_                  ,  // 34
    &_BrdTempDegC_                  ,  // 35
    &_HMUPartNumber_                ,  // 36
    &_HMUSerialNumber_              ,  // 37
    &_PWSwDwgNumber_                ,  // 38
    &_UTASSwDwgNumber_              ,  // 39
    &_fault_word3_                  ,  // 40
    &_fault_word4_                  ,  // 41
    &_prg_flash_cbit_fail_          ,  // 42
    &_fault_word7_                  ,  // 43
    &_hleif_eicas_annun_tx_         ,  // 44
    &_hmu_eicas_annun_              ,  // 45
    &_1738780_32_                   ,  // 46
    &_1738780_64_                   ,  // 47
    &_231680_32_                    ,  // 48
    &_231680_64_                    ,  // 49
    &_231688_32_                    ,  // 50
    &_231688_64_                    ,  // 51
    &_231689_32_                    ,  // 52
    &_231689_64_                    ,  // 53
    &_231696_32_                    ,  // 54
    &_231696_64_                    ,  // 55
    &_231704_32_                    ,  // 56
    &_231704_64_                    ,  // 57
    &_231712_32_                    ,  // 58
    &_231712_64_                    ,  // 59
    &_231720_32_                    ,  // 60
    &_231720_64_                    ,  // 61
    &_231728_32_                    ,  // 62
    &_231728_64_                    ,  // 63
    &_231782_32_                    ,  // 64
    &_231782_64_                    ,  // 65
    &_231790_32_                    ,  // 66
    &_231790_64_                    ,  // 67
    &_231798_32_                    ,  // 68
    &_231798_64_                    ,  // 69
    &_231806_32_                    ,  // 70
    &_231806_64_                    ,  // 71
    &_231838_32_                    ,  // 72
    &_231838_64_                    ,  // 73
    &_231846_32_                    ,  // 74
    &_231846_64_                    ,  // 75
    &_231854_32_                    ,  // 76
    &_231854_64_                    ,  // 77
    &_231862_32_                    ,  // 78
    &_231862_64_                    ,  // 79
    &_231878_32_                    ,  // 80
    &_231878_64_                    ,  // 81
    &_231886_32_                    ,  // 82
    &_231886_64_                    ,  // 83
    &_231918_32_                    ,  // 84
    &_231918_64_                    ,  // 85
    &_8313856_1_                    ,  // 86
    &_8313856_16_                   ,  // 87
    &_8313864_20_                   ,  // 88
    &_8313864_35_                   ,  // 89
    &_a664_fr_eicas2_fdr_           ,  // 90
    &_OMS_BUTTON_                   ,  // 91
    &_OMS_PAGEID_                   ,  // 92
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
char _8204051_32[84];
char _8204052_32[84];
char _8204053_32[84];
char _8204055_32[84];
char _8204056_32[84];
char _8204059_32[84];
char _8204060_32[84];
char _8204061_32[84];
char _8204062_32[84];
char _8204063_32[84];
char _8204064_32[84];
char _8204065_32[84];
char _8204066_32[84];
char _8204067_32[84];
char _8204068_32[84];
char _8204069_32[84];
char _8204070_32[84];
char _8204071_32[84];
char _8204072_32[84];
char _8204057_32[84];
char _8204058_32[84];
char _8204051_64[84];
char _8204052_64[84];
char _8204053_64[84];
char _8204055_64[84];
char _8204056_64[84];
char _8204059_64[84];
char _8204060_64[84];
char _8204061_64[84];
char _8204062_64[84];
char _8204063_64[84];
char _8204064_64[84];
char _8204065_64[84];
char _8204066_64[84];
char _8204067_64[84];
char _8204068_64[84];
char _8204069_64[84];
char _8204070_64[84];
char _8204071_64[84];
char _8204072_64[84];
char _8204057_64[84];
char _8204058_64[84];
char _adrf_pat_udt_remain_a[84];
char _adrf_pat_udt_remain_b[84];

StaticIoiStr            _ADRF_FAULT_DATA_("ADRF_FAULT_DATA", _ADRF_FAULT_DATA, 52, true);              // 0
StaticIoiInt              _adrf_apm_wrap_("adrf_apm_wrap", 0, true);                                   // 1
StaticIoiInt      _adrf_data_cumflt_time_("adrf_data_cumflt_time", 0, true);                           // 2
StaticIoiInt _adrf_data_flt_time_current_("adrf_data_flt_time_current", 0, true);                      // 3
StaticIoiStr         _adrf_data_operator_("adrf_data_operator", _adrf_data_operator, 32, true);        // 4
StaticIoiStr            _adrf_data_owner_("adrf_data_owner", _adrf_data_owner, 32, true);              // 5
StaticIoiInt     _adrf_data_power_on_cnt_("adrf_data_power_on_cnt", 0, true);                          // 6
StaticIoiInt    _adrf_data_power_on_time_("adrf_data_power_on_time", 0, true);                         // 7
StaticIoiInt            _adrf_health_ind_("adrf_health_ind", 0, true);                                 // 8
StaticIoiStr            _adrf_rtc_source_("adrf_rtc_source", _adrf_rtc_source, 2, true);               // 9
StaticIoiStr              _adrf_rtc_time_("adrf_rtc_time", _adrf_rtc_time, 6, true);                   // 10
StaticIoiStr            _adrf_ships_time_("adrf_ships_time", _adrf_ships_time, 6, true);               // 11
StaticIoiInt          _adrf_status_word1_("adrf_status_word1", 0, true);                               // 12
StaticIoiInt          _adrf_status_word2_("adrf_status_word2", 0, true);                               // 13
StaticIoiStr             _rtc_io_wr_date_("rtc_io_wr_date", _rtc_io_wr_date, 1, true);                 // 14
StaticIoiStr              _rtc_io_wr_day_("rtc_io_wr_day", _rtc_io_wr_day, 1, true);                   // 15
StaticIoiStr             _rtc_io_wr_hour_("rtc_io_wr_hour", _rtc_io_wr_hour, 1, true);                 // 16
StaticIoiStr          _rtc_io_wr_minutes_("rtc_io_wr_minutes", _rtc_io_wr_minutes, 1, true);           // 17
StaticIoiStr            _rtc_io_wr_month_("rtc_io_wr_month", _rtc_io_wr_month, 1, true);               // 18
StaticIoiStr          _rtc_io_wr_seconds_("rtc_io_wr_seconds", _rtc_io_wr_seconds, 1, true);           // 19
StaticIoiStr             _rtc_io_wr_year_("rtc_io_wr_year", _rtc_io_wr_year, 1, true);                 // 20
StaticIoiInt       _adrf_data_batt_timer_("adrf_data_batt_timer", 0, true);                            // 21
StaticIoiStr                 _8204051_32_("8204051_32", _8204051_32, 84, true);                        // 22
StaticIoiStr                 _8204052_32_("8204052_32", _8204052_32, 84, true);                        // 23
StaticIoiStr                 _8204053_32_("8204053_32", _8204053_32, 84, true);                        // 24
StaticIoiStr                 _8204055_32_("8204055_32", _8204055_32, 84, true);                        // 25
StaticIoiStr                 _8204056_32_("8204056_32", _8204056_32, 84, true);                        // 26
StaticIoiStr                 _8204059_32_("8204059_32", _8204059_32, 84, true);                        // 27
StaticIoiStr                 _8204060_32_("8204060_32", _8204060_32, 84, true);                        // 28
StaticIoiStr                 _8204061_32_("8204061_32", _8204061_32, 84, true);                        // 29
StaticIoiStr                 _8204062_32_("8204062_32", _8204062_32, 84, true);                        // 30
StaticIoiStr                 _8204063_32_("8204063_32", _8204063_32, 84, true);                        // 31
StaticIoiStr                 _8204064_32_("8204064_32", _8204064_32, 84, true);                        // 32
StaticIoiStr                 _8204065_32_("8204065_32", _8204065_32, 84, true);                        // 33
StaticIoiStr                 _8204066_32_("8204066_32", _8204066_32, 84, true);                        // 34
StaticIoiStr                 _8204067_32_("8204067_32", _8204067_32, 84, true);                        // 35
StaticIoiStr                 _8204068_32_("8204068_32", _8204068_32, 84, true);                        // 36
StaticIoiStr                 _8204069_32_("8204069_32", _8204069_32, 84, true);                        // 37
StaticIoiStr                 _8204070_32_("8204070_32", _8204070_32, 84, true);                        // 38
StaticIoiStr                 _8204071_32_("8204071_32", _8204071_32, 84, true);                        // 39
StaticIoiStr                 _8204072_32_("8204072_32", _8204072_32, 84, true);                        // 40
StaticIoiStr                 _8204057_32_("8204057_32", _8204057_32, 84, true);                        // 41
StaticIoiStr                 _8204058_32_("8204058_32", _8204058_32, 84, true);                        // 42
StaticIoiStr                 _8204051_64_("8204051_64", _8204051_64, 84, true);                        // 43
StaticIoiStr                 _8204052_64_("8204052_64", _8204052_64, 84, true);                        // 44
StaticIoiStr                 _8204053_64_("8204053_64", _8204053_64, 84, true);                        // 45
StaticIoiStr                 _8204055_64_("8204055_64", _8204055_64, 84, true);                        // 46
StaticIoiStr                 _8204056_64_("8204056_64", _8204056_64, 84, true);                        // 47
StaticIoiStr                 _8204059_64_("8204059_64", _8204059_64, 84, true);                        // 48
StaticIoiStr                 _8204060_64_("8204060_64", _8204060_64, 84, true);                        // 49
StaticIoiStr                 _8204061_64_("8204061_64", _8204061_64, 84, true);                        // 50
StaticIoiStr                 _8204062_64_("8204062_64", _8204062_64, 84, true);                        // 51
StaticIoiStr                 _8204063_64_("8204063_64", _8204063_64, 84, true);                        // 52
StaticIoiStr                 _8204064_64_("8204064_64", _8204064_64, 84, true);                        // 53
StaticIoiStr                 _8204065_64_("8204065_64", _8204065_64, 84, true);                        // 54
StaticIoiStr                 _8204066_64_("8204066_64", _8204066_64, 84, true);                        // 55
StaticIoiStr                 _8204067_64_("8204067_64", _8204067_64, 84, true);                        // 56
StaticIoiStr                 _8204068_64_("8204068_64", _8204068_64, 84, true);                        // 57
StaticIoiStr                 _8204069_64_("8204069_64", _8204069_64, 84, true);                        // 58
StaticIoiStr                 _8204070_64_("8204070_64", _8204070_64, 84, true);                        // 59
StaticIoiStr                 _8204071_64_("8204071_64", _8204071_64, 84, true);                        // 60
StaticIoiStr                 _8204072_64_("8204072_64", _8204072_64, 84, true);                        // 61
StaticIoiStr                 _8204057_64_("8204057_64", _8204057_64, 84, true);                        // 62
StaticIoiStr                 _8204058_64_("8204058_64", _8204058_64, 84, true);                        // 63
StaticIoiStr      _adrf_pat_udt_remain_a_("adrf_pat_udt_remain_a", _adrf_pat_udt_remain_a, 84, true);  // 64
StaticIoiStr      _adrf_pat_udt_remain_b_("adrf_pat_udt_remain_b", _adrf_pat_udt_remain_b, 84, true);  // 65
StaticIoiInt          _adrf_data_flt_leg_("adrf_data_flt_leg", 0, true);                               // 66

StaticIoiObj* aseIoiIn[ASE_IN_MAX] = {
    &_ADRF_FAULT_DATA_            , // 0
    &_adrf_apm_wrap_              , // 1
    &_adrf_data_cumflt_time_      , // 2
    &_adrf_data_flt_time_current_ , // 3
    &_adrf_data_operator_         , // 4
    &_adrf_data_owner_            , // 5
    &_adrf_data_power_on_cnt_     , // 6
    &_adrf_data_power_on_time_    , // 7
    &_adrf_health_ind_            , // 8
    &_adrf_rtc_source_            , // 9
    &_adrf_rtc_time_              , // 10
    &_adrf_ships_time_            , // 11
    &_adrf_status_word1_          , // 12
    &_adrf_status_word2_          , // 13
    &_rtc_io_wr_date_             , // 14
    &_rtc_io_wr_day_              , // 15
    &_rtc_io_wr_hour_             , // 16
    &_rtc_io_wr_minutes_          , // 17
    &_rtc_io_wr_month_            , // 18
    &_rtc_io_wr_seconds_          , // 19
    &_rtc_io_wr_year_             , // 20
    &_adrf_data_batt_timer_       , // 21
    &_8204051_32_                 , // 22
    &_8204052_32_                 , // 23
    &_8204053_32_                 , // 24
    &_8204055_32_                 , // 25
    &_8204056_32_                 , // 26
    &_8204059_32_                 , // 27
    &_8204060_32_                 , // 28
    &_8204061_32_                 , // 29
    &_8204062_32_                 , // 30
    &_8204063_32_                 , // 31
    &_8204064_32_                 , // 32
    &_8204065_32_                 , // 33
    &_8204066_32_                 , // 34
    &_8204067_32_                 , // 35
    &_8204068_32_                 , // 36
    &_8204069_32_                 , // 37
    &_8204070_32_                 , // 38
    &_8204071_32_                 , // 39
    &_8204072_32_                 , // 40
    &_8204057_32_                 , // 41
    &_8204058_32_                 , // 42
    &_8204051_64_                 , // 43
    &_8204052_64_                 , // 44
    &_8204053_64_                 , // 45
    &_8204055_64_                 , // 46
    &_8204056_64_                 , // 47
    &_8204059_64_                 , // 48
    &_8204060_64_                 , // 49
    &_8204061_64_                 , // 50
    &_8204062_64_                 , // 51
    &_8204063_64_                 , // 52
    &_8204064_64_                 , // 53
    &_8204065_64_                 , // 54
    &_8204066_64_                 , // 55
    &_8204067_64_                 , // 56
    &_8204068_64_                 , // 57
    &_8204069_64_                 , // 58
    &_8204070_64_                 , // 59
    &_8204071_64_                 , // 60
    &_8204072_64_                 , // 61
    &_8204057_64_                 , // 62
    &_8204058_64_                 , // 63
    &_adrf_pat_udt_remain_a_      , // 64
    &_adrf_pat_udt_remain_b_      , // 65
    &_adrf_data_flt_leg_          , // 66
};

#endif
