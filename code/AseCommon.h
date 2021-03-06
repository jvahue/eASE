#ifndef AseCommon_h
#define AseCommon_h
/******************************************************************************
Copyright (C) 2013-2017 Knowlogic Software Corp.
All Rights Reserved. Proprietary and Confidential.

File:        AseCommon.h

Description: Common ASE definitions

VERSION
$Revision: $  $Date: $

******************************************************************************/

/*****************************************************************************/
/* Compiler Specific Includes                                                */
/*****************************************************************************/
#include <deos.h>

/*****************************************************************************/
/* Software Specific Includes                                                */
/*****************************************************************************/
#include "alt_basic.h"     // from adrf
#include "alt_stdtypes.h"  // from adrf

#include "ParamMgr.h"      // from adrf

#include "TimeSrcObject.h"

/**********************************************************************************************
* Description: Common/Standard definitions for the ASE modules.
*
*
*/
# define version "v2.2.0"

#ifdef ARRAY
#undef ARRAY
#endif
#ifdef MIN
#undef MIN
#endif
#ifdef MAX
#undef MAX
#endif

#define ARRAY(i, ul) (((i) >= 0 && (i) < (ul)))
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))

#define INC_WRAP( v, m)  (((v) >= ((m)-1)) ? 0 : ((v)+1))

// Handy #defs for accessing fields in AseCommon
#define GET_SYSTEM_TICK (*(m_pCommon->systemTickPtr))
#define IS_CONNECTED      (m_pCommon->bConnected)
#define IS_SCRIPT_ACTIVE  (m_pCommon->bScriptRunning)
#define IS_ADRF_ON        (m_pCommon->adrfState != eAdrfOff)
#define IS_MS_ONLINE      (m_pCommon->bMsOnline)

// Flight Trigger History Constants
#define HIST_TRIG_BUFF 86400 // Make this div by 1800 byte blks.  Current NVM size is 85496
#define HIST_BLK_SIZE 1800
#define HIST_BLK_MAX (HIST_TRIG_BUFF/HIST_BLK_SIZE)


// high speed timer
#define HsTimer() getTimeStamp()

// return us
#define HsTimeDiff(start) ((HsTimer() - start) * aseCommon.clockFreqInv)

///////////////////////////////////////////////////////////////////////////////////////////////
enum AseSystemConstants {
    eAseParamNameSize  = 32,  // SEC/IOC size of a sensor name (UUT uses 32)
    eAseMaxParams = 3000,     // a maximum of 3000 parameters in the system
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
    eSGpwm1,       // A better PWM - no period delay at the start
    eSGrandom,     // random values uniform dist (min, max)
    eSGunit,       // value passed is a UINT32 - used when send full 32 bit values
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
PARAM_SRC_CALC,   // SRC from internally calculated data
PARAM_SRC_CYCLE,  // SRC from cycle object, current cycle value
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

enum PowerState {
    ePsOff,  // Power is off (assert adrfProcHndl == NULL)
    ePsOn,   // Power is on  (assert adrfProcHndl != NULL)
    ePs50,   // Bus Power Lost waiting for Latch (assert adrfProcHndl != NULL)
    ePsLatch // Battery Latched (assert adrfProcHndl != NULL)
};

enum AdrfState {
    eAdrfOff,
    eAdrfOn,
    eAdrfReady
};

// Structure of control attributes for managing the UUT
typedef struct AseCommonTag
{
    AdrfState    adrfState;      // Current state of UUT. off, on, rdy = gse connection active
    PowerState   asePowerState;  // Current Ase Power State
    BYTE         scriptPowerOn;  // Script want ADRF power On/off
    UNSIGNED32   *systemTickPtr; // Pointer to the system tick value.
    bool         bConnected;     // ePySte Connection
    bool         bScriptRunning; // Is a script actively running
    bool         bMsOnline;      // is the MS on-line
    bool         recfgSuccess;   // recfg success reset CCDL mode CmReconfig sets, CCDL clears
    bool         isChannelA;     // true when we are in channel A
    UINT32       clockFreq;
    float        clockFreqInv;
    UINT32       shipDate;       // the entire word
    UINT32       shipTime;       //
    TimeSrcObj   clocks[eClkMax];  // what time is it
    UINT32       newBaseTimeRqst;  // Number of clock updates Rxed
    UINT32       newBaseTimeSrv;   // Number of remote clock updates serviced
    UINT32       remElapsedMif;    // number of elapsed MIF since base time
    BYTE         *nvmAddress;      // share this with the CCDL object
    char         adrfVer[32];      // ADRF version
} AseCommon;

typedef BYTE FlightTriggerHistory[(HIST_TRIG_BUFF)];

extern AseCommon aseCommon;

// these are declared here so our NVM memory reader has access to them
extern FlightTriggerHistory HistTrigBuff;
extern FlightTriggerHistory HistTrigBuffRx;
#endif
