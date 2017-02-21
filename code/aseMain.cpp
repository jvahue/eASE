/******************************************************************************
Copyright (C) 2013-2017 Knowlogic Software Corp.
All Rights Reserved. Proprietary and Confidential.

File:        aseMain.cpp

Description: The main ASE process

VERSION
$Revision: $  $Date: $

******************************************************************************/

/*****************************************************************************/
/* Compiler Specific Includes                                                */
/*****************************************************************************/
#include <deos.h>
#include <stdio.h>
#include <string.h>
#include <videobuf.h>

/*****************************************************************************/
/* Software Specific Includes                                                */
/*****************************************************************************/
#include "AseCommon.h"

#include "CmProcess.h"
#include "ioiProcess.h"
#include "video.h"

/*****************************************************************************/
/* Local Defines                                                             */
/*****************************************************************************/
#define MAX_CMD_RSP 2
#define _50msec 5

// Battery Control Signals mask bit
#define BATTERY_MAP_SIZE 4
#define LEVC_ADRF_BATT_CMD 1

// Battery Status Signals
#define LEVA_BATT_IN_SW_MASTER_EN 0x01 // Level A has enabled Battery Latch
#define LEVC_ADRF_BATT_CMD_WA     0x02 // Level C ADRF has enabled Battery Latch
#define BATT_SW_ENA_N             0x04 // On Battery Power: active low
#define LOSS_AF28V_N              0x20 // indicate the loss of Bus Power

#define LVL_A_ENABLE  (batteryStsMirror |  LEVA_BATT_IN_SW_MASTER_EN)
#define LVL_A_DISABLE (batteryStsMirror & ~LEVA_BATT_IN_SW_MASTER_EN)

#define LVL_C_BAT_LATCH   ((batteryStsMirror |  LEVC_ADRF_BATT_CMD_WA) & ~BATT_SW_ENA_N)
#define LVL_C_BAT_UNLATCH ((batteryStsMirror & ~LEVC_ADRF_BATT_CMD_WA) |  BATT_SW_ENA_N)

#define SET_BUS_POWER_ON  (batteryStsMirror |  LOSS_AF28V_N)
#define SET_BUS_POWER_OFF (batteryStsMirror & ~LOSS_AF28V_N)

// Status Checks
#define IS_BUS_POWER_ON  (aseCommon.scriptPowerOn & 0x1)
#define IS_INVERT_POWER  (aseCommon.scriptPowerOn & 0x2)
#define IS_LVL_A_ON      (batteryStsMirror & LEVA_BATT_IN_SW_MASTER_EN)
#define IS_LVL_C_ON_BATT (batteryStsMirror & LEVC_ADRF_BATT_CMD_WA)
#define IS_BATT_EN       (~batteryStsMirror & BATT_SW_ENA_N)
#define IS_BATT_LATCH_EN (IS_LVL_A_ON && IS_BATT_EN)

#define eDyHdr 0
#define eDyASE 1
#define eDyMs  2
#define eDyRem 3
#define eDyShip 4
#define eDySec  5
#define eDyCom  6
#define eDyBatt 7
#define eDyLast 8
#define eDyErr  9
#define eDyAdrfOn 10
#define eDyAdrfOff 11
#define eDyShHx 12
#define eDyMax 13

#define kCmProc 0
#define kIoiProc 1
#define kAseMain 2
#define kTotalRqst 3

/*****************************************************************************/
/* Local Typedefs                                                            */
/*****************************************************************************/
enum BatteryTestControlState {
    eBattDisabled,  // No battery latching is available
    eBattEnabled,   // Battery Latching works as expected
    eBattStuckLo,   // Battery Never Latched
    eBattStuckHi,   // Battery Always Latched
    eBattMax        // Max value
};

typedef struct
{
    platformResourceHandle handle;
    accessStyle style;
    BYTE *address;
} PLATFORM_UINT08;

typedef struct
{
    platformResourceHandle handle;
    accessStyle style;
    UINT32 *address;
} PLATFORM_UINT32;

/*****************************************************************************/
/* Local Variables                                                           */
/*****************************************************************************/
CmProcess cmProc;
IoiProcess ioiProc;

// adrf.exe process control vars
const char adrfName[] = "adrf";
const char adrfTmplName[] = "adrf-template";
processStatus adrfProcStatusOn = processNotActive;
processStatus adrfProcStatusOff = processNotActive;
process_handle_t adrfProcHndl = NULL;
int adrfOnCount;
int adrfOnCall;
int adrfOffCount;
int adrfOffCall;

PLATFORM_UINT08 nvm;
//static platformResourceHandle nvmHandle;
//static BYTE* nvm.address;
static const UINT32 NvmSize = 0x31000;

//----------------------------------------------------------------------------
// Battery Control Variables
// This affects the power-on/off state
PLATFORM_UINT32 battStsReg;
PLATFORM_UINT32 battCtlReg;

BatteryTestControlState batteryState;
INT32 _50MsTimer;         // times out the 50ms holdup

UINT32 batteryStsMirror; // 0: Lvl A Batt Enable, 1: Lvl C Batt Cmd
UINT32 batteryCtlMirror; // 0: Lvl C Batt Cmd

// Four Battery Control Tables for each mode of BatteryTestControlState are mapped into the 
// batteyrSts word.  Each value in each tables represents the positive logic asserted state for 
// PowerOn: bit 0, BattLatched: bit 1, LvlC_WA: bit 2
// The Lookup key is based on Script Power: Bit 0 and LvlC_CMD: bit 1
UINT8 batMapDisable[BATTERY_MAP_SIZE] = {0, 1, 4, 5};
UINT8 batMapEnable[BATTERY_MAP_SIZE]  = {0, 1, 6, 5};
UINT8 batMapHigh[BATTERY_MAP_SIZE]    = {2, 3, 6, 7};
UINT8 batMapLow[BATTERY_MAP_SIZE]     = {0, 1, 4, 5};
UINT8* batMapLookup[eBattMax] = {batMapDisable, batMapEnable, batMapHigh, batMapLow};

UINT8 powerKey;
UINT8 batMapActive[BATTERY_MAP_SIZE];

/*****************************************************************************/
/* Global Variables                                                          */
/*****************************************************************************/
AseCommon aseCommon;

FlightTriggerHistory HistTrigBuff;    // Flight Trigger in NVM to send Ref 1
FlightTriggerHistory HistTrigBuffRx; // Flight Trigger in NVM received Ref 2

/*****************************************************************************/
/* Constant Data                                                             */
/*****************************************************************************/
CmdRspThread* cmdRspThreads[] = {
    &cmProc,   // make sure kCmProc  = 0
    &ioiProc,  // make sure kIoiProc = 1
    NULL
};

// 0 = ioiProc, 1 = cmProc, 2 = aseMain, 3 = total requests
UINT32 cmdHandler[4] = {0, 0, 0, 0};

/*****************************************************************************/
/* Local Function Prototypes                                                 */
/*****************************************************************************/
static BOOLEAN CheckCmds(SecComm& secComm);
static void SetTime(SecRequest& request);
static void UpdateTime();
static void UpdateShipDate();
static void UpdateShipTime();
static void UpdateBattery();
static void PowerCtl();
static void PowerOn();
static void PowerOff();

static BOOLEAN NvmRead(SecComm& secComm);
static BOOLEAN NvmWrite(SecComm& secComm);
static void NV_WriteAligned(void* dest, const void* src, UINT32 size);

/*****************************************************************************/
/* Public Functions                                                          */
/*****************************************************************************/
// Function: main
// Description: The function main() implements this process' main thread
// (sometimes referred to as its primary thread).  That is, when Deos
// automatically creates the main thread, main() will execute.
// Declare some VideoStream objects that will allow this thread to output text to the target's
// video memory (essentially, a printf-style debugging aid).  For each VideoStream object,
// we specify four parameters (y, x, numY, numX), where:
// y=starting row, x=starting column, numY=number of rows, and numX=number of columns
//VideoStream videoOutTitle(14, 0, 1, 50);   // here is where we'll output the title string
//VideoStream videoOut1(20, 40, 1, 40);     // here is where we'll output the system tick value
int main(void)
{
    // These variables are used to hold values we want to output to video memory.
    const UNSIGNED32 systemTickTimeInHz = 1000000 / systemTickInMicroseconds();
    const UINT32 MAX_IDLE_FRAMES = 15 * systemTickTimeInHz;

    void *theAreg;
    accessStyle asA;
    UNSIGNED32 myChanID;
    resourceStatus  status;
    platformResourceHandle hA;
    accessStyle access_style;

    char batStsStr[5];
    UINT32 i;
    UINT32 start;
    UINT32 td;
    SecComm secComm;
    UINT32 frames = 0;
    UINT32 cmdIdle = 0;
    UINT32 lastCmdAt = 0;
    UINT32 updateDisplayLine = 0;

    debug_str_init();
    videoRedirect = AseMain;

    memset( &aseCommon, 0, sizeof(aseCommon));

    for (int i = 0; i < eClkMax; ++i)
    {
        aseCommon.clocks[i].Init();
    }
    UpdateShipDate();
    UpdateShipTime();

    aseCommon.clockFreq = getSystemInfoDEOS()->eventLogClockFrequency;
    aseCommon.clockFreqInv = 1.0f/float(aseCommon.clockFreq);

    // default to MS being on-line
    aseCommon.bMsOnline = true;

    // Grab the system tick pointer, all threads/tasks should use GET_SYSTEM_TICK
    aseCommon.systemTickPtr = systemTickPointer();

    //---------------------------------------------------------------------
    // Start Running All of the ASE Threads
    secComm.Run();

    // Run all of the cmd response threads
    for (i=0; cmdRspThreads[i] != NULL; ++i)
    {
        cmdRspThreads[i]->Run(&aseCommon);
    }
    //---------------------------------------------------------------------

    // default to Channel A
    aseCommon.isChannelA = ioiProc.GetChanId() == 1;

    strcpy(aseCommon.adrfVer, "Unknown");

    // Attach to NVM
    status = attachPlatformResource("","ADRF_NVRAM",&nvm.handle,
        &nvm.style, (void**)&nvm.address);

    // overhead of timing
    start = HsTimer();
    td = HsTimeDiff(start);

    // see CheckCmds - where this is updated
    debug_str(AseMain, eDyLast, 0, "Last Cmd Id: 0x");

    // POWER CONTROL SETUP
    status = attachPlatformResource("","FPGA_BATT_MSPWR_DAL_C", &battCtlReg.handle,
        &battCtlReg.style, (void**)&battCtlReg.address);

    status = attachPlatformResource("","FPGA_BATT_MSPWR_DAL_C", &battStsReg.handle,
        &battStsReg.style, (void**)&battStsReg.address);

    // move the Status Register Address forward 4 bytes
    battStsReg.address = &battStsReg.address[1];

    // No battery latching operations enabled
    *battCtlReg.address = 0;
    *battStsReg.address = 0;

    _50MsTimer = 0;
    aseCommon.asePowerState = ePsOff;
    aseCommon.scriptPowerOn = 1;  // Pon, no invert
    batteryState = eBattDisabled;
    memcpy(batMapActive, batMapLookup[batteryState], sizeof(batMapActive));

    adrfOnCount = 0;
    adrfOnCall = 0;
    adrfOffCount = 0;
    adrfOffCall = 0;

    PowerCtl();

    // The main thread goes into an infinite loop.
    while (1)
    {
        // call the base class to display the first row
        cmdRspThreads[0]->CmdRspThread::UpdateDisplay(AseMain, eDyHdr);

        if (updateDisplayLine == eDyASE)
        {
            debug_str(AseMain, eDyASE, 0, 
                "ASE: %04d/%02d/%02d %02d:%02d:%02d.%0.3d %s in channel %s",
                aseCommon.clocks[eClkRtc].m_time.tm_year,
                aseCommon.clocks[eClkRtc].m_time.tm_mon,   // month    0..11
                aseCommon.clocks[eClkRtc].m_time.tm_mday,  // day of the month  1..31
                aseCommon.clocks[eClkRtc].m_time.tm_hour,  // hours    0..23
                aseCommon.clocks[eClkRtc].m_time.tm_min,   // minutes  0..59
                aseCommon.clocks[eClkRtc].m_time.tm_sec,   // seconds  0..59
                aseCommon.clocks[eClkRtc].m_10ms,
                version,
                aseCommon.isChannelA ? "A" : "B"
                );
        }

        else if (updateDisplayLine == eDyMs)
        {
            debug_str(AseMain, eDyMs, 0, "MS : %04d/%02d/%02d %02d:%02d:%02d.%0.3d ADRF: %s",
                aseCommon.clocks[eClkMs].m_time.tm_year,
                aseCommon.clocks[eClkMs].m_time.tm_mon,   // month    0..11
                aseCommon.clocks[eClkMs].m_time.tm_mday,  // day of the month  1..31
                aseCommon.clocks[eClkMs].m_time.tm_hour,  // hours    0..23
                aseCommon.clocks[eClkMs].m_time.tm_min,   // minutes  0..59
                aseCommon.clocks[eClkMs].m_time.tm_sec,   // seconds  0..59
                aseCommon.clocks[eClkMs].m_10ms,
                aseCommon.adrfVer);
        }

        else if (updateDisplayLine == eDyRem)
        {
        debug_str(AseMain, eDyRem, 0, "REM: %04d/%02d/%02d %02d:%02d:%02d.%0.3d",
            aseCommon.clocks[eClkRemote].m_time.tm_year,
            aseCommon.clocks[eClkRemote].m_time.tm_mon,   // month    0..11
            aseCommon.clocks[eClkRemote].m_time.tm_mday,  // day of the month  1..31
            aseCommon.clocks[eClkRemote].m_time.tm_hour,  // hours    0..23
            aseCommon.clocks[eClkRemote].m_time.tm_min,   // minutes  0..59
            aseCommon.clocks[eClkRemote].m_time.tm_sec,   // seconds  0..59
            aseCommon.clocks[eClkRemote].m_10ms);
        }

        else if (updateDisplayLine == eDyShip)
        {
            debug_str(AseMain, eDyShip, 0, "SHP: %04d/%02d/%02d %02d:%02d:%02d.%0.3d",
                aseCommon.clocks[eClkShips].m_time.tm_year,
                aseCommon.clocks[eClkShips].m_time.tm_mon,   // month    0..11
                aseCommon.clocks[eClkShips].m_time.tm_mday,  // day of the month  1..31
                aseCommon.clocks[eClkShips].m_time.tm_hour,  // hours    0..23
                aseCommon.clocks[eClkShips].m_time.tm_min,   // minutes  0..59
                aseCommon.clocks[eClkShips].m_time.tm_sec,   // seconds  0..59
                aseCommon.clocks[eClkShips].m_10ms);
        }

        else if (updateDisplayLine == eDySec)
        {
            // Write the system tick value to video memory.
            debug_str(AseMain, eDySec, 0, "SecComm(%s) %d - %d",
                secComm.GetSocketInfo(),
                frames, td);
        }

        else if (updateDisplayLine == eDyCom)
        {
            debug_str(AseMain, eDyCom, 0, "Rx(%d) Tx(%d) IsRx: %s CloseConn: %s Idle Time: %4d/%d",
                secComm.GetRxCount(),
                secComm.GetTxCount(),
                secComm.isRxing ? "Yes" : "No ",
                secComm.forceConnectionClosed ? "Yes" : "No",
                cmdIdle+1,
                MAX_IDLE_FRAMES
                );
        }

        else if (updateDisplayLine == eDyErr)
        {
            debug_str(AseMain, eDyErr, 0, "%s", secComm.GetErrorMsg());
        }

        else if (updateDisplayLine == eDyBatt)
        {
            batStsStr[0] = (batteryStsMirror  & LOSS_AF28V_N) ? 'P' : ' ';
            batStsStr[1] = (~batteryStsMirror & BATT_SW_ENA_N) ? 'B' : ' ';
            batStsStr[2] = (batteryStsMirror  & LEVC_ADRF_BATT_CMD_WA) ? 'C' : ' ';
            batStsStr[3] = (batteryStsMirror  & LEVA_BATT_IN_SW_MASTER_EN) ? 'A' : ' ';
            batStsStr[4] = '\0';
            debug_str(AseMain, eDyBatt, 0, 
                "ScrPwr: %d Batt Ctl|Sts: 0x%x|0x%02x %d:(%d, %d, %d, %d) %d/<%d> %s", 
                aseCommon.scriptPowerOn,
                batteryCtlMirror & 0xf, batteryStsMirror & 0xff, batteryState,
                batMapActive[0], batMapActive[1], batMapActive[2], batMapActive[3],
                powerKey, batMapActive[powerKey], batStsStr);
        }

        else if (updateDisplayLine == eDyAdrfOn)
        {
            debug_str(AseMain, eDyAdrfOn, 0,  "PowerOn (%4d/%4d): %d ioi: %6d ase: %6d", 
                adrfOnCount, adrfOnCall, adrfProcStatusOn,
                cmdHandler[kIoiProc], cmdHandler[kAseMain]);
        }

        else if (updateDisplayLine == eDyAdrfOff)
        {
            UINT32 x = cmdHandler[kTotalRqst] - 
                (cmdHandler[0] + cmdHandler[1] + cmdHandler[kAseMain]);

            debug_str(AseMain, eDyAdrfOff, 0, "PowerOff(%4d/%4d): %d  cm: %6d Ttl: %6d/%d", 
                adrfOffCount, adrfOffCall, adrfProcStatusOff,
                cmdHandler[kCmProc], cmdHandler[kTotalRqst], x);
        }

        updateDisplayLine += 1;
        if (eDyMax == updateDisplayLine)
        {
            updateDisplayLine = 0;
        }
       
        // Yield the CPU and wait until the next period to run again.
        waitUntilNextPeriod();

        UpdateTime();

        PowerCtl();

        frames += 1;

        // Any new cmds seen
        if ( secComm.IsConnected())
        {
            if (CheckCmds( secComm))
            {
                lastCmdAt = frames;
            }
            else
            {
                // no msg loss timeout if we are running a script - we could be in along delay
                cmdIdle = frames - lastCmdAt;
                if (cmdIdle > MAX_IDLE_FRAMES)
                {
                    secComm.forceConnectionClosed = !aseCommon.bScriptRunning;
                    lastCmdAt = frames;
                }
            }
        }
        else
        {
            secComm.forceConnectionClosed = FALSE;
            cmdIdle = 0;

            // general housekeeping
            aseCommon.bScriptRunning = FALSE;
            // reset the battery latch logic to default (i.e., no battery latching
            batteryState = eBattDisabled;
            memcpy(batMapActive, batMapLookup[batteryState], sizeof(batMapActive));

        }

        aseCommon.bConnected = secComm.IsConnected();
    }
}

//---------------------------------------------------------------------------------------------
static BOOLEAN CheckCmds(SecComm& secComm)
{
    accessStyle asA;
    UNSIGNED32 myChanID;
    resourceStatus  status;
    platformResourceHandle hA;

    UINT32 i;
    BOOLEAN cmdSeen = FALSE;
    BOOLEAN serviced = TRUE;
    ResponseType rType = eRspNormal;
    SecRequest request = secComm.m_request;

    if (secComm.IsCmdAvailable())
    {
        debug_str(AseMain, eDyLast, 0, "Last Cmd Id: %3d", request.cmdId);
        cmdSeen = TRUE;
        cmdHandler[kTotalRqst] += 1; 
        videoRedirect = (VID_DEFS)request.videoDisplay;

        switch (request.cmdId)
        {
        case eRunScript:
            SetTime(request);
            aseCommon.bScriptRunning = TRUE;
            // reset the power counters
            adrfOnCount = 0;
            adrfOnCall = 0;
            adrfOffCount = 0;
            adrfOffCall = 0;

            secComm.m_response.successful = TRUE;
            break;

        case eScriptDone:
            SetTime(request);
            aseCommon.bScriptRunning = FALSE;
            // reset the battery latch logic to default (i.e., no battery latching
            batteryState = eBattDisabled;
            memcpy(batMapActive, batMapLookup[batteryState], sizeof(batMapActive));
            secComm.m_response.successful = TRUE;
            break;

        case eShutdown:
            SetTime(request);
            aseCommon.bScriptRunning = FALSE;
            secComm.m_response.successful = TRUE;
            break;

        case ePing:
            // ping carries the current time and next time
            SetTime(request);
            secComm.m_response.successful = TRUE;
            break;

        case ePowerOn:
            SetTime(request);
            aseCommon.scriptPowerOn = (secComm.m_request.variableId & 0x3);
            secComm.m_response.successful = TRUE;
            break;

        case ePowerOff:
            SetTime(request);
            // request the power off
            aseCommon.scriptPowerOn = (secComm.m_request.variableId & 0x3);
            secComm.m_response.successful = TRUE;
            break;

        case eMsState:
            // Kill the ADRF process to simulate behavior during power off
            if (request.variableId == 0 || request.variableId == 1)
            {
                aseCommon.bMsOnline = request.variableId == 1;
                secComm.m_response.successful = TRUE;
            }
            else
            {
                secComm.ErrorMsg("Ms State Error: Accept 0,1 - got %d", request.variableId);
                secComm.m_response.successful = FALSE;
            }
            break;

            //---------------
        case eSetChanId:
            ioiProc.SetChanId(request.variableId);

            // default to Channel A
            aseCommon.isChannelA = ioiProc.GetChanId() == 1;

            secComm.m_response.successful = TRUE;
            break;

            //---------------
        case eNvmRead:
            // check which memory area the user wants to read from
            if (secComm.m_request.resetAll == 0)
            {
                if (nvm.address != NULL)
                {
                    secComm.m_response.successful = NvmRead(secComm);
                    secComm.m_response.successful = TRUE;
                }
                else
                {
                    secComm.ErrorMsg("NVM Memory Handle Error");
                    secComm.m_response.successful = FALSE;
                }
            }
            else if (secComm.m_request.resetAll == 1)
            {
                // range check the read
                UINT32 totalSize = sizeof(HistTrigBuffRx);
                UINT32 offset = secComm.m_request.variableId;
                UINT32 size = secComm.m_request.sigGenId;

                if ((offset + size) <= totalSize)
                {
                    memcpy(
                        secComm.m_response.streamData, (void*)&HistTrigBuffRx[offset], size);
                    secComm.m_response.streamSize = size;
                }
                else
                {
                    secComm.ErrorMsg("Memory Read boundary error: attempt to read %d - max %d", 
                        (offset + size), totalSize);
                    secComm.m_response.successful = FALSE;
                }
            }
            else
            {
                secComm.ErrorMsg("Invalid memory identifier %d", secComm.m_request.resetAll);
                secComm.m_response.successful = FALSE;
            }

            break;

            //---------------
        case eNvmWrite:
            if (nvm.address != NULL)
            {
                secComm.m_response.successful = NvmWrite(secComm);
                secComm.m_response.successful = TRUE;
            }
            else
            {
                secComm.ErrorMsg("NVM Memory Handle Error");
                secComm.m_response.successful = FALSE;
            }

            break;

            //------------------------
        case eGetAseVersion:
            strcpy(secComm.m_response.streamData, version);
            secComm.m_response.streamSize = strlen(version);
            secComm.m_response.successful = TRUE;
            break;

            //------------------------
        case eSetBatteryCtrl:
            // variableId => Battery Control State
            // sigGenId   => Power-Off Delay Timer, 0: never power-off
            if (request.variableId >= (INT32)eBattDisabled &&
                request.variableId <= (INT32)eBattStuckHi)
            {
                batteryState = BatteryTestControlState(request.variableId);
                // update the active table map
                memcpy(batMapActive, batMapLookup[batteryState], sizeof(batMapActive));
                if (request.charDataSize == 4)
                {
                    // override the default values
                    for (int i = 0; i < 4; ++i)
                    {
                        batMapActive[i] = (UINT8)request.charData[i];
                    }
                }
                secComm.m_response.successful = TRUE;
            }
            else
            {
                secComm.ErrorMsg("Battery Control Value Error (%d)", request.variableId);
                secComm.m_response.successful = FALSE;
            }
            break;

            //------------------------
        case eGetBatterySts:
            // pack the Power State and Battery State into the
            sprintf(secComm.m_response.streamData,
                "Power: %d, Battery: %d, Ctl: 0x%08x, Sts: 0x%08x",
                aseCommon.asePowerState,
                batteryState,
                batteryCtlMirror,
                batteryStsMirror);
            secComm.m_response.streamSize = strlen(secComm.m_response.streamData);
            secComm.m_response.successful = TRUE;
            secComm.m_response.successful = FALSE;
            break;

        case eSetAdrfVersion:
            memcpy(aseCommon.adrfVer, 
                   secComm.m_request.charData, 
                   secComm.m_request.charDataSize);
            aseCommon.adrfVer[secComm.m_request.charDataSize] = '\0';
            secComm.m_response.successful = TRUE;
            break;

        default:
            serviced = FALSE;
            break;
        }

        if (serviced)
        {
            secComm.SetHandler("AseMain");
            secComm.IncCmdServiced(rType);
            cmdHandler[kAseMain] += 1;
        }
        else
        {
            // Run all of the cmd response threads
            for (i=0; i < MAX_CMD_RSP; ++i)
            {
                if (cmdRspThreads[i]->CheckCmd(secComm))
                {
                    cmdHandler[i] += 1;
                    break;
                }
            }
        }
    }

    return cmdSeen;
}

//---------------------------------------------------------------------------------------------
// UpdateBattery - This function Performs the IOI battery feedback logic based on battery state
static void UpdateBattery()
{
    UINT8 batSts;
    UINT8 lvlCOn;

    // create the key for our power sts map
    powerKey = IS_BUS_POWER_ON ? 1 : 0;

    // read battery control from the ADRF
    batteryCtlMirror =  *battCtlReg.address;
    lvlCOn = (batteryCtlMirror & LEVC_ADRF_BATT_CMD);
    powerKey |= lvlCOn ? 2 : 0;

    batSts = batMapActive[powerKey];

    // clear the status mirror and rebuild it
    batteryStsMirror = 0;
    
    batteryStsMirror |= (batteryState != eBattDisabled) ? LEVA_BATT_IN_SW_MASTER_EN : 0; // A
    batteryStsMirror |= (batSts & 4) ? LEVC_ADRF_BATT_CMD_WA : 0; // Lvl C WA
    batteryStsMirror |= (batSts & 2) ? 0 : BATT_SW_ENA_N;         // Batt Enable Asserted Low
    batteryStsMirror |= (batSts & 1) ? LOSS_AF28V_N : 0;          // Power Loss Asserted Low

    // provide status
    *battStsReg.address = batteryStsMirror;
}

//---------------------------------------------------------------------------------------------
static void PowerCtl()
{
    // update the battery status and ctrl words
    UpdateBattery();

    switch (aseCommon.asePowerState)
    {
    case ePsOff:
        if (IS_BUS_POWER_ON)
        {
            PowerOn();
        }
        break;

    case ePsOn:
        // check to see if we lost bus power
        if (!IS_BUS_POWER_ON)
        {
            // see if the script is allowing battery latching
            if (IS_BATT_LATCH_EN)
            {
                aseCommon.asePowerState = ePsLatch;
            }
            else  // simulate Hold up CAP
            {
                _50MsTimer = _50msec;
                aseCommon.asePowerState = ePs50;
            }
        }
        break;

    case ePs50:
        // if bus power comes back turn us on
        if (IS_BUS_POWER_ON)
        {
            PowerOn();
        }
        else if (IS_BATT_LATCH_EN && _50MsTimer > 0)
        {
            _50MsTimer = 0;
            aseCommon.asePowerState = ePsLatch;
        }
        else
        {
            if (--_50MsTimer == 0)
            {
                // we will default to latch state but if we really turn off because 
                // !IS_INVERT_POWER we will be ePsOff
                _50MsTimer = 0;
                aseCommon.asePowerState = ePsLatch;

                // shut off as the ADRF has not latched the battery
                PowerOff();
            }
        }
        break;

    case ePsLatch:
        if (IS_BUS_POWER_ON)
        {
            PowerOn();
        }
        else if (!IS_BATT_LATCH_EN)  // SCR-322
        {
            PowerOff();
        }
        break;
    }
}

//---------------------------------------------------------------------------------------------
static void PowerOn()
{
    ++adrfOnCall;
    // Create the ADRF process to simulate behavior during power on
    if ( adrfProcHndl == NULL)
    {
        if (!IS_INVERT_POWER)
        {
            ++adrfOnCount;
            adrfProcStatusOn = createProcess( "adrf", "adrf-template", 0, TRUE, &adrfProcHndl);

            // Update the global state info
            if (processSuccess == adrfProcStatusOn)
            {
                aseCommon.asePowerState = ePsOn;
                aseCommon.adrfState = eAdrfOn;
            }
            else
            {

                aseCommon.asePowerState = ePsOff;
                aseCommon.adrfState = eAdrfOff;
            }

            _50MsTimer = 0;
        }
    }
    else
    {
        aseCommon.asePowerState = ePsOn;
        aseCommon.adrfState = eAdrfOn;
    }
}

//---------------------------------------------------------------------------------------------
static void PowerOff()
{
    ++adrfOffCall;

    // Kill the ADRF process to simulate behavior during power off
    if (adrfProcHndl != NULL && !IS_INVERT_POWER)
    {
        ++adrfOffCount;
        adrfProcStatusOff = deleteProcess( adrfProcHndl);

        if (processSuccess == adrfProcStatusOff)
        {
            adrfProcHndl = NULL;

            // Update the global-shared data block
            aseCommon.asePowerState = ePsOff;
            aseCommon.adrfState = eAdrfOff;
        }

        _50MsTimer = 0;
    }
}

//---------------------------------------------------------------------------------------------
static void SetTime(SecRequest& request)
{
    PyTimeStruct* timeObjs = (PyTimeStruct*)request.charData;

    for (int i=0; i < eClkMax; ++i)
    {
        aseCommon.clocks[i].SetTime(timeObjs[i]);
    }

    // compute new remote base time
    aseCommon.newBaseTimeRqst += 1;
}

//---------------------------------------------------------------------------------------------
// Read from n bytes <n=sigGenId> NVM memory <ref=resetRequest> at the offset address specified 
// <offset=variableId>.  The offset address is from the base of NVM memory.  This assumes all 
// addresses are inside our NVM [0..NvmSize-1] bytes.
//
// This code services three memory areas:
// 0. The actual NVM area
// 1. Our copy of the Flight Trigger area
// 2. The Rx buffer holding the Flight trigger data form the remote channel
static BOOLEAN NvmRead(SecComm& secComm)
{
    UINT32 offset = secComm.m_request.variableId;
    UINT32 bytes = secComm.m_request.sigGenId;

    if ((offset + bytes) < NvmSize)
    {
        if (bytes <= eSecStreamSize)
        {
            memcpy(secComm.m_response.streamData, (void*)(nvm.address + offset), bytes);
            secComm.m_response.streamSize = bytes;
            return TRUE;
        }
        else
        {
            secComm.ErrorMsg("NvmRead: Requested bytes (%d) exceeds max (%d)",
                bytes, eSecStreamSize);
            return FALSE;
        }
    }
    else
    {
        secComm.ErrorMsg("NvmRead: Requested read outside of NVM by %d bytes",
            (NvmSize - (offset + bytes)));
        return FALSE;
    }
}

//---------------------------------------------------------------------------------------------
// NVM Read/Write Logic
static BOOLEAN NvmWrite(SecComm& secComm)
{
    UINT32 offset = secComm.m_request.variableId;
    UINT32 size = secComm.m_request.charDataSize;
    UINT16* data;

    if ((offset + size) < NvmSize)
    {
        NV_WriteAligned((void*)(nvm.address + offset),
            (void*)secComm.m_request.charData,
            size);
    }

    return TRUE;
}

//---------------------------------------------------------------------------------------------
static
void NV_WriteAligned(void* dest, const void* src, UINT32 size)
{
    UINT8* ptr8;
    UINT16 aligner;
    UINT16 *dest_ptr16 = (UINT16*)dest;
    const UINT16   *src_ptr16 = (UINT16*)src;

    if(size > 0)
    {
        if((1 & (UINT32)dest) == 1)
        {
            //Set destination to 1 previous byte address to make it even.
            ptr8 = (UINT8*)dest_ptr16;
            ptr8--;
            dest_ptr16 = (UINT16*)ptr8;
            aligner = *dest_ptr16;
            //Mask unaligned byte (big endian, mask least significant dest (higher address)
            //and swap for most significant (lower address) source)
            aligner &= 0xFF00;
            aligner |= (*src_ptr16 & 0xFF00)  >> 8;
            //Write aligned word to destination
            *dest_ptr16 = aligner;
            //Increment to next even address.  Increment source for the
            //1 byte masked to the odd address
            dest_ptr16++;
            ptr8 = (UINT8*)src_ptr16;
            ptr8++;
            src_ptr16 = (UINT16*)ptr8;
            size--;
        }

        //Copy bytes 2 at a time until zero or 1 bytes remain.
        for(;size >= 2;size-=2)
        {
            *dest_ptr16++ = *src_ptr16++;
        }

        if(size != 0)
        {
            //Read destination aligned
            aligner = *dest_ptr16;
            //Mask unaligned byte (big endian, mask and write most significant dest byte)
            aligner &= 0xFF;
            aligner |= (*src_ptr16 & 0xFF00);
            //Write aligned word to destination
            *dest_ptr16 = aligner;
        }
    }
}

//---------------------------------------------------------------------------------------------
// Update the wall clock time - PySte will resend every now and then but we need to maintain
// it between those updates.

static void UpdateTime()
{
    aseCommon.remElapsedMif += 1;

    for (int i=0; i < eClkMax; ++i)
    {
        aseCommon.clocks[i].UpdateTime();
    }

    // update the ships time
    if (aseCommon.clocks[eClkShips].m_10ms == 0)
    {
        UpdateShipDate();
        UpdateShipTime();
        debug_str(AseMain, eDyShHx, 0, "Ship Date: 0x%08x Time: 0x%08x",
            aseCommon.shipDate,
            aseCommon.shipTime);
    }
}

//---------------------------------------------------------------------------------------------
// Update the ships date -
// pack into label 0260 reverse => 0x0D
// sdi = 1
// ssm = 11
static void UpdateShipDate()
{
#define DATE_SSM 0x60000000
#define DATE_SDI 0x100
#define DATE_LABEL 0x0d

    UINT8 dayX10 = aseCommon.clocks[eClkShips].m_time.tm_mday / 10;
    UINT8 dayX1  = aseCommon.clocks[eClkShips].m_time.tm_mday % 10;
    UINT8 monthX10 = aseCommon.clocks[eClkShips].m_time.tm_mon / 10;
    UINT8 monthX1  = aseCommon.clocks[eClkShips].m_time.tm_mon % 10;
    UINT8 yearX10  = (aseCommon.clocks[eClkShips].m_time.tm_year - 2000) / 10;
    UINT8 yearX1   = (aseCommon.clocks[eClkShips].m_time.tm_year - 2000) % 10;

    UINT32 dateData;
    dateData  = (dayX10 << 27) & 0x18000000L;
    dateData |= (dayX1  << 23) & 0x07800000L;
    dateData |= (monthX10 << 22) & 0x00400000L;
    dateData |= (monthX1 << 18) & 0x003c0000L;
    dateData |= (yearX10 << 14) & 0x0003c000L;
    dateData |= (yearX1 << 10) & 0x00003c00L;

    aseCommon.shipDate = DATE_SSM | dateData | DATE_SDI | DATE_LABEL;
}

//---------------------------------------------------------------------------------------------
// Update the ships date -
// pack into label 0150 reverse =>
// sdi = 1
// ssm = 11
static void UpdateShipTime()
{
#define TIME_SSM 0x60000000
#define TIME_SDI 0x100
#define TIME_LABEL 0x16

    UINT32 timeData = (aseCommon.clocks[eClkShips].m_time.tm_hour << 23) & 0x0f800000L;
    timeData |= (aseCommon.clocks[eClkShips].m_time.tm_min << 17) & 0x007e0000L;
    timeData |= (aseCommon.clocks[eClkShips].m_time.tm_sec << 11) & 0x0001f800L;

    aseCommon.shipTime = TIME_SSM | timeData | TIME_SDI | TIME_LABEL;
}

