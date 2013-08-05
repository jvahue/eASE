#ifndef AseCommon_h
#define AseCommon_h

// File: AseCommon.h

#include <deos.h>

/**********************************************************************************************
* Description: Common/Standard definitons for the ASE modules.
*
*
*/
#define ARRAY(i, max) (((i) >=0 && (i) < (max)))

// Handy #defs for accessing fields in AseCommon
#define GET_SYSTEM_TICK (*(m_pCommon->systemTickPtr))
#define IS_CONNECTED      (m_pCommon->bConnected)
#define IS_SCRIPT_ACTIVE  (m_pCommon->bScriptRunning)
#define IS_POWER_ON       (m_pCommon->bPowerOnState)
#define IS_MS_ONLINE      (m_pCommon->bMsOnline)


///////////////////////////////////////////////////////////////////////////////////////////////
enum AseSystemConstants {
    eAseParamNameSize  = 32,  // SEC/IOC size of a sensor name (UUT uses 32)
    eAseMaxParams = 3000,      // a maximum of 3000 parameters in the system
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


typedef char ParameterName[eAseParamNameSize];

// Structure of control attribs for managing the UUT
typedef struct
{
    UNSIGNED32 *systemTickPtr; // Pointer to the system tick value.
    bool       bConnected;     // ePySte Connection
    bool       bScriptRunning; // Is a script actively running
    bool       bPowerOnState;  // Current "virtual" power state of UUT. True = PwrOn, FALSE = PwrOff
    bool       bMsOnline;      // is the MS online
} AseCommon;

#endif
