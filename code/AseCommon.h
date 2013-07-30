#ifndef AseCommon_h
#define AseCommon_h

// File: AseCommon.h

/**********************************************************************************************
* Description: Common/Standard definitons for the ASE modules.
*
*
*/
#define ARRAY(i, max) (((i) >=0 && (i) < (max)))


///////////////////////////////////////////////////////////////////////////////////////////////
enum AseSystemConstants {
    eAseSensorNameSize  = 32,         // SEC/IOC size of a sensor name (UUT uses 32)
};

///////////////////////////////////////////////////////////////////////////////////////////////
// Signal Generator Types
enum SigGenEnum {
    eSGmanual,     // no signal generator
    eSGramp,       // ramp from low to high, then back to low and repeat (min, max, time(s))
    eSGrampHold,   // ramp to a value and hold at max
    eSGtriangle,   // ramp up/down (min, max, time(s))
    eSGsine,       // sine wave starting at 0 deg, (freq(Hz), amplitude)
    eSG1Shot,      // one shot a signal (start, oneShotValue, frameIndex)
    eSGnShot,      // n-shot (baseline, nValue, frameIndex, nFrames)
    eSGpwm,        // PWM between two values (value1, value2) varying duty cycle
    eSGrandom,     // random values uniform dist (min, max)
    eMaxSensorMode
};

///////////////////////////////////////////////////////////////////////////////////////////////
// FROM ADRF
enum PARAM_FMT_ENUM {
  PARAM_FMT_NONE=0,  // FMT not specified
  PARAM_FMT_BIN_A664,// FMT is IDL binary
  PARAM_FMT_FLT_A664,// FMT is IDL floating-point
  PARAM_FMT_A429,    // FMT is A429 standard data
//  PARAM_FMT_HS_ELECT,// FMT is HS Electric System data (not needed, use BIN_664)
  PARAM_FMT_MAX
};


typedef char ParameterName[eAseSensorNameSize];

#endif
