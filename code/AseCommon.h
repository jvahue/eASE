#ifndef AseCommon_h
#define AseCommon_h

// File: AseCommon.h

/**********************************************************************************************
* Description: Common/Standard definitons for the ASE modules.
*
*
*/
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



typedef char ParameterName[eAseSensorNameSize];

#endif
