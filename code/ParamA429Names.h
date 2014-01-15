/*
 * ParamA429Names.h
 *
 *  Created on: Aug 16, 2013
 *      Author: p916214
 */

#ifndef PARAMA429NAMES_H_
#define PARAMA429NAMES_H_

#ifndef ALLOW_A429_NAMES
    #error "Only allowed in ParamConverters.cpp"
#endif

#define A429_SRC_MAX_DATA 128 // This value must be > numIOINames
#define A429_IOI_NAME_SDI_IGNORE_VAL 4
typedef struct
{
  UINT8 octal;
  UINT8 sdi;    // Note, 4 = SDI Ignore
  CHAR  name[32];
} A429_IOI_NAME;

const A429_IOI_NAME ioiA429Names[] =
{
        { 0241, 0, "ac_aoa1_raw" },    // Param 1
        { 0241, 1, "ac_aoa2_raw" },    // Param 2
        { 0241, 2, "ac_aoa3_raw" },    // Param 3
        { 0241, 3, "ac_aoa4_raw" },    // Param 4
        { 0226, 0, "ac_ser_num_raw" }, // Param 5
        { 0167, 0, "ac_type_raw" },    // Param 6
        {  075, 1, "ac_weight1_raw" }, // Param 7
        {  075, 2, "ac_weight2_raw" }, // Param 8
        { 0270, 0, "adsp_statwd1_1_raw" }, // Param 9
        { 0270, 1, "adsp_statwd1_2_raw" }, // Param 10
        { 0270, 2, "adsp_statwd1_3_raw" }, // Param 11
        { 0270, 3, "adsp_statwd1_4_raw" }, // Param 12
        { 0271, 0, "adsp_statwd2_1_raw" }, // Param 13
        { 0271, 1, "adsp_statwd2_2_raw" }, // Param 14
        { 0271, 2, "adsp_statwd2_3_raw" }, // Param 15
        { 0271, 3, "adsp_statwd2_4_raw" }, // Param 16
        { 0272, 0, "adsp_statwd3_1_raw" }, // Param 17
        { 0272, 1, "adsp_statwd3_2_raw" }, // Param 18
        { 0272, 2, "adsp_statwd3_3_raw" }, // Param 19
        { 0272, 3, "adsp_statwd3_4_raw" }, // Param 20
        { 0206, 0, "airspeed1_raw" }, // Param 21
        { 0206, 1, "airspeed2_raw" }, // Param 22
        { 0206, 2, "airspeed3_raw" }, // Param 23
        { 0206, 3, "airspeed4_raw" }, // Param 24
        { 0137, 1, "flap_pos1_raw" }, // Param 25
        { 0137, 3, "flap_pos2_raw" }, // Param 26
        { 0332, 1, "lat_accel1_raw" }, // Param 27
        { 0332, 2, "lat_accel2_raw" }, // Param 28
        { 0332, 3, "lat_accel3_raw" }, // Param 29
        { 0110, 4, "latpos_coarse_raw" }, // Param 30   NOTE: SDI 4 == IGNORE
        { 0120, 4, "latpos_fine_raw" },   // Param 31   NOTE: SDI 4 == IGNORE
        { 0335, 1, "left_inbd_wspd1_raw" },  // Param 32
        { 0335, 2, "left_inbd_wspd2_raw" },  // Param 33
        { 0334, 1, "left_outbd_wspd1_raw" }, // Param 34
        { 0334, 2, "left_outbd_wspd2_raw" }, // Param 35
        { 0164, 1, "left_radalt_raw" },      // Param 36
        { 0331, 1, "long_accel1_raw" },      // Param 37
        { 0331, 2, "long_accel2_raw" },      // Param 38
        { 0331, 3, "long_accel3_raw" },      // Param 39
        { 0111, 4, "longpos_coarse_raw" },   // Param 40   NOTE: SDI 4 == IGNORE
        { 0121, 4, "longpos_fine_raw" },     // Param 41   NOTE: SDI 4 == IGNORE
        { 0205, 0, "mach1_raw" },            // Param 42
        { 0205, 1, "mach2_raw" },            // Param 43
        { 0205, 2, "mach3_raw" },            // Param 44
        { 0205, 3, "mach4_raw" },            // Param 45
        { 0333, 1, "norm_accel1_raw" },      // Param 46
        { 0333, 2, "norm_accel2_raw" },      // Param 47
        { 0333, 3, "norm_accel3_raw" },      // Param 48
        { 0324, 1, "pitch_angle1_raw" },     // Param 49
        { 0324, 2, "pitch_angle2_raw" },     // Param 50
        { 0324, 3, "pitch_angle3_raw" },     // Param 51
        { 0326, 1, "pitch_rate1_raw" },      // Param 52
        { 0326, 2, "pitch_rate2_raw" },      // Param 53
        { 0326, 3, "pitch_rate3_raw" },      // Param 54
        { 0276, 1, "pos_status1_raw" },      // Param 55
        { 0276, 2, "pos_status2_raw" },      // Param 56
        { 0203, 0, "press_alt1_raw" },       // Param 57
        { 0203, 1, "press_alt2_raw" },       // Param 58
        { 0203, 2, "press_alt3_raw" },       // Param 59
        { 0203, 3, "press_alt4_raw" },       // Param 60
        { 0336, 1, "right_inbd_wspd1_raw" }, // Param 61
        { 0336, 2, "right_inbd_wspd2_raw" }, // Param 62
        { 0337, 1, "right_outbd_wspd1_raw" }, // Param 63
        { 0337, 2, "right_outbd_wspd2_raw" }, // Param 64
        { 0164, 2, "right_radalt_raw" },      // Param 65
        { 0325, 1, "roll_angle1_raw" },      // Param 66
        { 0325, 2, "roll_angle2_raw" },      // Param 67
        { 0325, 3, "roll_angle3_raw" },      // Param 68
        { 0327, 1, "roll_rate1_raw" },    // Param 69
        { 0327, 2, "roll_rate2_raw" },    // Param 70
        { 0327, 3, "roll_rate3_raw" },    // Param 71
        { 0260, 1, "ships_date_raw" },   // Param 72  Ch A
        { 0260, 2, "ships_date_raw" },   // Param 73  Ch B
        { 0150, 1, "ships_time_raw" },   // Param 74  Ch A
        { 0150, 2, "ships_time_raw" },   // Param 75  Ch B
        { 0127, 0, "slat_pos1_raw" },   // Param 76
        { 0127, 2, "slat_pos2_raw" },   // Param 77
        { 0250, 0, "sldslp_angle1_raw" }, // Param 78
        { 0250, 1, "sldslp_angle2_raw" }, // Param 79
        { 0250, 2, "sldslp_angle3_raw" }, // Param 80
        { 0250, 3, "sldslp_angle4_raw" }, // Param 81
        { 0274, 1, "woffw_pos1_raw" },   // Param 82
        { 0274, 2, "woffw_pos2_raw" },   // Param 83
        { 0330, 1, "yaw_rate1_raw" },   // Param 84
        { 0330, 2, "yaw_rate2_raw" },   // Param 85
        { 0330, 3, "yaw_rate3_raw" },   // Param 86
        { 0301, 0, "ac_tail_num_low_raw" }, // Param 87
        { 0302, 0, "ac_tail_num_mid_raw" }, // Param 88
        { 0303, 0, "ac_tail_num_high_raw" }, // Param 89
        { 0251, 0, "flight_leg_raw" },   // Param 90
        {    0, 0, "\0" }
};

//UINT8 ioiNameUsed[A429_SRC_MAX_DATA];

#endif /* PARAMA429NAMES_H_ */
