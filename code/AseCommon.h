#ifndef AseCommon_h
#define AseCommon_h

// File: AseCommon.h

#include <deos.h>

#include "alt_stdtypes.h"

#include "ParamMgr.h"
#include "Time.h"

/**********************************************************************************************
* Description: Common/Standard definitions for the ASE modules.
*
*
*/
# define version "v0.4.4"  " " __DATE__ " " __TIME__

#define ARRAY(i, ul) (((i) >=0 && (i) < (ul)))
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))

// Handy #defs for accessing fields in AseCommon
#define GET_SYSTEM_TICK (*(m_pCommon->systemTickPtr))
#define IS_CONNECTED      (m_pCommon->bConnected)
#define IS_SCRIPT_ACTIVE  (m_pCommon->bScriptRunning)
#define IS_ADRF_ON        (m_pCommon->adrfState != eAdrfOff)
#define IS_MS_ONLINE      (m_pCommon->bMsOnline)

// high speed timer
#define HsTimer() getTimeStamp()

// return us
#define HsTimeDiff(start) ((HsTimer() - start) * aseCommon.clockFreqInv) 

///////////////////////////////////////////////////////////////////////////////////////////////
enum AseSystemConstants {
    eAseParamNameSize  = 32,  // SEC/IOC size of a sensor name (UUT uses 32)
    eAseMaxParams = 3000,      // a maximum of 3000 parameters in the system
    eAseCharDataSize = 2048,
    eAseStreamSize = 3500
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
/*enum PARAM_FMT_ENUM {
  PARAM_FMT_NONE=0,  // FMT not specified
  PARAM_FMT_BIN_A664,// FMT is IDL binary
  PARAM_FMT_FLT_A664,// FMT is IDL floating-point
  PARAM_FMT_A429,    // FMT is A429 standard data
//  PARAM_FMT_HS_ELECT,// FMT is HS Electric System data (not needed, use BIN_664)
  PARAM_FMT_MAX
};

typedef enum {
  PARAM_SRC_NONE=0, // SRC not specified
  PARAM_SRC_HMU,    // SRC from HMU interface - Inc data fmt of A492, A664
  PARAM_SRC_A429,   // SRC from A429 interface - Place Holder
  PARAM_SRC_A664,   // SRC from A664 interface - Place Holder
  PARAM_SRC_CROSS,  // SRC from cross channel interface
  PARAM_SRC_MAX
} PARAM_SRC_ENUM;

typedef struct {
    SINT32     tm_sec;   // seconds  0..59
    SINT32     tm_min;   // minutes  0..59
    SINT32     tm_hour;  // hours    0..23
    SINT32     tm_mday;  // day of the month  1..31
    SINT32     tm_mon;   // month    1..12
    SINT32     tm_year;  //
} LINUX_TM_FMT;

*/

typedef char ParameterName[eAseParamNameSize];

enum AdrfState {
    eAdrfOff,
    eAdrfOn,
    eAdrfReady
};

// Structure of control attributes for managing the UUT
typedef struct
{
    AdrfState    adrfState;      // Current state of UUT. off, on, rdy = gse connection active
    UNSIGNED32   *systemTickPtr; // Pointer to the system tick value.
    bool         bConnected;     // ePySte Connection
    bool         bScriptRunning; // Is a script actively running
    bool         bMsOnline;      // is the MS online
    LINUX_TM_FMT time;           // what time is it
    bool         recfgSuccess;   // recfg success reset CCDL mode CmReconfig sets, CCDL clears
    bool         isChannelA;     // true when we are in channel A
    UINT32       clockFreq;
    float        clockFreqInv;
} AseCommon;

extern AseCommon aseCommon;

#endif
