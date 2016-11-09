#ifndef AseStaticIoi_h
#define AseStaticIoi_h

#include "ioiStatic.h"

#define ASE_IN_MAX 70
#define ASE_OUT_MAX 48

// -------------------- U U T   O U T P U T S ------------------------ 
// -------------------- ASE Consumed - ADRf Produced -----------------                  
// --------- The Order MUST match adrfOutMap in eFastCmds.py ---------                   
int adrfFault[13];                                                                       
int adrfTime[2];                                                                         
int shipTime[2];                                                                         
char operatorName[64];                                                                   
char ownerName[64];                                                                      
char rtcSource[5];                                                                       
       
StaticIoiObj& aseIoiIn[ASE_IN_MAX] = (
    StaticIoiIntPtr("ADRF_FAULT_DATA", adrfFault, 13, true);       // 00                
    StaticIoiInt("adrf_apm_wrap", 0, true);                        // 01                
    StaticIoiInt("adrf_data_cumflt_time", 0, true);                // 02                
    StaticIoiInt("adrf_data_flt_time_current", 0, true);           // 03                
    StaticIoiStr("adrf_data_operator", operatorName, 64, true);    // 04                
    StaticIoiStr("adrf_data_owner", ownerName, 64, true);          // 05                
    StaticIoiInt("adrf_data_power_on_cnt", 0, true);               // 06                
    StaticIoiInt("adrf_data_power_on_time", 0, true);              // 07                
    StaticIoiInt("adrf_health_ind", 0, true);                      // 08                
    StaticIoiStr("adrf_rtc_source", rtcSource, 2, true);           // 09                
    StaticIoiIntPtr("adrf_rtc_time", adrfTime, 2, true);           // 10                
    StaticIoiIntPtr("adrf_ships_time", shipTime, 2, true);         // 11                
    StaticIoiInt("adrf_status_word1", 0, true);                    // 12                
    StaticIoiInt("adrf_status_word2", 0, true);                    // 13                
    StaticIoiByte("rtc_io_wr_date", 0, true);                      // 14                
    StaticIoiByte("rtc_io_wr_day", 0, true);                       // 15                
    StaticIoiByte("rtc_io_wr_hour", 0, true);                      // 16                
    StaticIoiByte("rtc_io_wr_minutes", 0, true);                   // 17                
    StaticIoiByte("rtc_io_wr_month", 0, true);                     // 18                
    StaticIoiByte("rtc_io_wr_seconds", 0, true);                   // 19                
    StaticIoiByte("rtc_io_wr_year", 0, true);                      // 20                
    StaticIoiInt("adrf_data_batt_timer", 0, true);                 // 21                
                                                                                             
    // A429 Floating point values, so pass them back as integer                              
    StaticIoiInt("8204050_32", 0, true); // 22 - 0x7D2F1220, # A Engine Inlet Angle 1   
    StaticIoiInt("8204051_32", 0, true); // 23 - 0x7D2F1320, # A Idle Time, A           
    StaticIoiInt("8204052_32", 0, true); // 24 - 0x7D2F1420, # A PAT Total Test Time    
    StaticIoiInt("8204053_32", 0, true); // 25 - 0x7D2F1520, # A Shutdown time Delay    
    StaticIoiInt("8204054_32", 0, true); // 26 - 0x7D2F1620, # A Wind Speed             
    StaticIoiInt("8204055_32", 0, true); // 27 - 0x7D2F1720, # A N1 Setting Tolerance   
    StaticIoiInt("8204056_32", 0, true); // 28 - 0x7D2F1820, # A Test Stabilization Time
    StaticIoiInt("8204059_32", 0, true); // 29 - 0x7D2F1B20, # A PAT_MIN_IDLE_TREMAIN   
    StaticIoiInt("8204060_32", 0, true); // 30 - 0x7D2F1C20, # A PAT_N1_TARGET          
    StaticIoiInt("8204061_32", 0, true); // 31 - 0x7D2F1D20, # A PAT_N1_TARGET_TIME     
    StaticIoiInt("8204062_32", 0, true); // 32 - 0x7D2F1E20, # A PAT_IDLE_CLOSE_TIME    
    StaticIoiInt("8204063_32", 0, true); // 33 - 0x7D2F1F20, # A PAT_N1_STABLE_AVG      
    StaticIoiInt("8204064_32", 0, true); // 34 - 0x7D2F2020, # A PAT_N2_STABLE_AVG      
    StaticIoiInt("8204065_32", 0, true); // 35 - 0x7D2F2120, # A PAT_N2_LIM_MIN         
    StaticIoiInt("8204066_32", 0, true); // 36 - 0x7D2F2220, # A PAT_N2_LIM_MAX         
    StaticIoiInt("8204067_32", 0, true); // 37 - 0x7D2F2320, # A PAT_EGT_STABLE_AVG     
    StaticIoiInt("8204068_32", 0, true); // 38 - 0x7D2F2420, # A PAT_EGT_LIM_MIN        
    StaticIoiInt("8204069_32", 0, true); // 39 - 0x7D2F2520, # A PAT_EGT_LIM_MAX        
    StaticIoiInt("8204070_32", 0, true); // 40 - 0x7D2F2620, # A PAT_WF_STABLE_AVG      
    StaticIoiInt("8204071_32", 0, true); // 41 - 0x7D2F2720, # A PAT_WF_LIM_MIN         
    StaticIoiInt("8204072_32", 0, true); // 42 - 0x7D2F2820, # A PAT_WF_LIM_MAX         
    StaticIoiInt("8204057_32", 0, true); // 43 - 0x7D2F1920, # A Sts Wd1                
    StaticIoiInt("8204058_32", 0, true); // 44 - 0x7D2F1A20, # A Sts Wd2                
                                                                                             
    // A429 Floating point values, so pass them back as integer                              
    StaticIoiInt("8204050_64", 0, true); // 45 - 0x7D2F1240, # B Engine Inlet Angle 1   
    StaticIoiInt("8204051_64", 0, true); // 46 - 0x7D2F1340, # B Idle Time, A           
    StaticIoiInt("8204052_64", 0, true); // 47 - 0x7D2F1440, # B PAT Total Test Time    
    StaticIoiInt("8204053_64", 0, true); // 48 - 0x7D2F1540, # B Shutdown time Delay    
    StaticIoiInt("8204054_64", 0, true); // 49 - 0x7D2F1640, # B Wind Speed             
    StaticIoiInt("8204055_64", 0, true); // 50 - 0x7D2F1740, # B N1 Setting Tolerance   
    StaticIoiInt("8204056_64", 0, true); // 51 - 0x7D2F1840, # B Test Stabilization Time
    StaticIoiInt("8204059_64", 0, true); // 52 - 0x7D2F1B40, # B PAT_MIN_IDLE_TREMAIN   
    StaticIoiInt("8204060_64", 0, true); // 53 - 0x7D2F1C40, # B PAT_N1_TARGET          
    StaticIoiInt("8204061_64", 0, true); // 54 - 0x7D2F1D40, # B PAT_N1_TARGET_TIME     
    StaticIoiInt("8204062_64", 0, true); // 55 - 0x7D2F1E40, # B PAT_IDLE_CLOSE_TIME    
    StaticIoiInt("8204063_64", 0, true); // 56 - 0x7D2F1F40, # B PAT_N1_STABLE_AVG      
    StaticIoiInt("8204064_64", 0, true); // 57 - 0x7D2F2040, # B PAT_N2_STABLE_AVG      
    StaticIoiInt("8204065_64", 0, true); // 58 - 0x7D2F2140, # B PAT_N2_LIM_MIN         
    StaticIoiInt("8204066_64", 0, true); // 59 - 0x7D2F2240, # B PAT_N2_LIM_MAX         
    StaticIoiInt("8204067_64", 0, true); // 60 - 0x7D2F2340, # B PAT_EGT_STABLE_AVG     
    StaticIoiInt("8204068_64", 0, true); // 61 - 0x7D2F2440, # B PAT_EGT_LIM_MIN        
    StaticIoiInt("8204069_64", 0, true); // 62 - 0x7D2F2540, # B PAT_EGT_LIM_MAX        
    StaticIoiInt("8204070_64", 0, true); // 63 - 0x7D2F2640, # B PAT_WF_STABLE_AVG      
    StaticIoiInt("8204071_64", 0, true); // 64 - 0x7D2F2740, # B PAT_WF_LIM_MIN         
    StaticIoiInt("8204072_64", 0, true); // 65 - 0x7D2F2840, # B PAT_WF_LIM_MAX         
    StaticIoiInt("8204057_64", 0, true); // 66 - 0x7D2F1940, # B Sts Wd1                
    StaticIoiInt("8204058_64", 0, true); // 67 - 0x7D2F1A40, # B Sts Wd2                
    // and two that we originally missed that do not have NDO numbers ...                    
    StaticIoiInt("adrf_pat_udt_remain_a", 0, true); // 68 adrf_pat_udt_remain_a         
    StaticIoiInt("adrf_pat_udt_remain_b", 0, true); // 69 adrf_pat_udt_remain_b         
);

// ------------------- U U T   I N P U T S ---------------------------                   
//----------------- ASE Producer - ADRF Consumer ---------------------
char HMUpartNumber[20];                                                                  
char HMUSerialNumber[20];                                                                
char PWSwDwgNumber[20];                                                                  
char UTASSwDwgNumber[20];                                                                
                                                                                         
// --------- The Order MUST match AseIoiInfo in eFastCmds.py --------- 
StaticIoiObj& aseIoiOut[ASE_OUT_MAX] = {
    StaticIoiInt("aircraft_id_1", 0);                       // 00                     
    StaticIoiInt("aircraft_id_2", 0);                       // 01                     
    StaticIoiInt("aircraft_id_3", 0);                       // 02                     
    StaticIoiInt("aircraft_id_4", 0);                       // 03                     
    StaticIoiInt("aircraft_id_5", 0);                       // 04                     
    StaticIoiInt("aircraft_id_par", 0);                     // 05                     
    StaticIoiInt("disc_spare_1", 0);                        // 06                     
    StaticIoiInt("disc_spare_2", 0);                        // 07                     
    StaticIoiInt("disc_spare_3", 0);                        // 08                     
    StaticIoiInt("ground_service_mode", 0);                 // 09                     
                                                                                             
    StaticIoiInt("weight_on_wheels", 0);                    // 10                     
    StaticIoiInt("wifi_override", 0);                       // 11                     
    StaticIoiByte("rtc_io_rd_date", 0);                     // 12                     
    StaticIoiByte("rtc_io_rd_day", 0);                      // 13                     
    StaticIoiByte("rtc_io_rd_hour", 0);                     // 14                     
    StaticIoiByte("rtc_io_rd_minutes", 0);                  // 15                     
    StaticIoiByte("rtc_io_rd_month", 0);                    // 16                     
    StaticIoiByte("rtc_io_rd_seconds", 0);                  // 17                     
    StaticIoiByte("rtc_io_rd_year", 0);                     // 18                     
    StaticIoiInt("ubmf_health_ind", 0);                     // 19                     
                                                                                             
    StaticIoiInt("umbf_status_word1", 0);                   // 20                     
    StaticIoiInt("BrdTempFail", 0);                         // 21                     
    StaticIoiInt("BrdTempInitFail", 0);                     // 22                     
    StaticIoiInt("BrdTempOpFail", 0);                       // 23                     
    StaticIoiInt("BrdTempRngFail", 0);                      // 24                     
    StaticIoiInt("HLEIFFaultInd2", 0);                      // 25                     
    StaticIoiInt("HLEIFFaultIndication", 0);                // 26                     
    StaticIoiInt("HLEIFStatusWord1", 0);                    // 27                     
    StaticIoiInt("hmu_option_data_raw", 0);                 // 28                     
    StaticIoiInt("micro_server_health_ind1", 0);            // 29                     
                                                                                             
    StaticIoiInt("micro_server_health_ind2", 0);            // 30                     
    StaticIoiInt("micro_server_internal_status", 0);        // 31                     
    StaticIoiInt("micro_server_status_word1", 0);           // 32                     
    StaticIoiFloat("BatInputVdc", 27.9f);                   // 33                     
    StaticIoiFloat("BatSwOutVdc", 28.2f);                   // 34                     
    StaticIoiFloat("BrdTempDegC", 10.0f);                   // 35                     
    StaticIoiStr("HMUPartNumber", HMUpartNumber, 20);       // 36                     
    StaticIoiStr("HMUSerialNumber", HMUSerialNumber, 20);   // 37                     
    StaticIoiStr("PWSwDwgNumber", PWSwDwgNumber, 20);       // 38                     
    StaticIoiStr("UTASSwDwgNumber", UTASSwDwgNumber, 20);   // 39                     
                                                                                             
    StaticIoiInt("pat_scr", 0);                             // 40                     
    StaticIoiInt("pat_scr_button", 0);                      // 41                     
                                                                                             
    // SCR-355 New Internal signals                                                          
    StaticIoiInt("fault_word3", 0);                         // 42 OL354               
    StaticIoiInt("fault_word4", 0);                         // 43 OL355               
    StaticIoiInt("prg_flash_cbit_fail", 0);                 // 44 OL356               
    StaticIoiInt("fault_word7", 0);                         // 45 OL357               
    StaticIoiInt("hleif_eicas_annun_tx", 0);                // 46 OL270               
    StaticIoiInt("hmu_eicas_annun", 0);                     // 47 OL271               
);
                                                                                         

#endif