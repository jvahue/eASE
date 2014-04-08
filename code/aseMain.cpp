/******************************************************************************
            Copyright (C) 2013 Knowlogic Software Corp.
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

/*****************************************************************************/
/* Local Typedefs                                                            */
/*****************************************************************************/

/*****************************************************************************/
/* Local Variables                                                           */
/*****************************************************************************/
CmProcess cmProc;
IoiProcess ioiProc;

// adrf.exe process control vars
const char adrfName[] = "adrf";
const char adrfTmplName[] = "adrf-template";
processStatus    adrfProcStatus = processNotActive;
process_handle_t adrfProcHndl = NULL;

LINUX_TM_FMT nextTime;
UINT32 _10msec;

static platformResourceHandle nvmHandle;
static BYTE* nvmBaseAddr;
static const UINT32 NvmSize = 0x31000;

/*****************************************************************************/
/* Global Variables                                                          */
/*****************************************************************************/
AseCommon aseCommon;

/*****************************************************************************/
/* Constant Data                                                             */
/*****************************************************************************/
CmdRspThread* cmdRspThreads[] = {
  &cmProc,
  &ioiProc,
  NULL
};

/*****************************************************************************/
/* Local Function Prototypes                                                 */
/*****************************************************************************/
static BOOLEAN CheckCmds(SecComm& secComm);
static void SetTime(SecRequest& request);
static void UpdateTime();
static void UpdateShipDate();
static void UpdateShipTime();

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

    UINT32 i;
    UINT32 start;
    UINT32 td;
    SecComm secComm;
    UINT32 frames = 0;
    UINT32 cmdIdle = 0;
    UINT32 lastCmdAt = 0;

    _10msec = 0;

    memset( &aseCommon, 0, sizeof(aseCommon));
    memset( &nextTime, 0, sizeof(nextTime));

    // default time 
    aseCommon.time.tm_year = 2013;
    aseCommon.time.tm_mon  = 7;
    aseCommon.time.tm_mday = 26;

    nextTime.tm_year = 2013;
    nextTime.tm_mon  = 7;
    nextTime.tm_mday = 27;

    UpdateShipDate();
    UpdateShipTime();

    aseCommon.clockFreq = getSystemInfoDEOS()->eventLogClockFrequency;
    aseCommon.clockFreqInv = 1.0f/float(aseCommon.clockFreq);

    // default to MS being online
    aseCommon.bMsOnline = true;

    // Grab the system tick pointer, all threads/tasks should use GET_SYSTEM_TICK
    aseCommon.systemTickPtr = systemTickPointer();

    debug_str_init();
    videoRedirect = AseMain;

    secComm.Run();

    // Run all of the cmd response threads
    for (i=0; cmdRspThreads[i] != NULL; ++i)
    {
        cmdRspThreads[i]->Run(&aseCommon);
    }

    // default to Channel A
    aseCommon.isChannelA = ioiProc.GetChanId() == 1;

    status = attachPlatformResource("","ADRF_NVRAM",&nvmHandle,
             &access_style,(void**)&nvmBaseAddr);

    // overhead of timing
    start = HsTimer();
    td = HsTimeDiff(start);
    td = HsTimeDiff(start);

    // see CheckCmds - where this is updated
    debug_str(AseMain, 5, 0, "Last Cmd Id: 0");

    // Initially create the adrf to start it running.
    adrfProcStatus  = createProcess( adrfName, adrfTmplName, 0, TRUE, &adrfProcHndl);
    debug_str(AseMain, 6, 0, "Initial Create of adrf returned: %d", adrfProcStatus);

    aseCommon.adrfState = (processSuccess == adrfProcStatus) ? eAdrfOn : eAdrfOff;

    // The main thread goes into an infinite loop.
    while (1)
    {
        // call the base class to get the first row
        cmdRspThreads[0]->CmdRspThread::UpdateDisplay(AseMain, 0);

        debug_str(AseMain, 1, 0, "ASE: %s %04d/%02d/%02d %02d:%02d:%02d.%0.3d in channel %s",
                  version,
                  aseCommon.time.tm_year,
                  aseCommon.time.tm_mon,   // month    0..11
                  aseCommon.time.tm_mday,  // day of the month  1..31
                  aseCommon.time.tm_hour,  // hours    0..23
                  aseCommon.time.tm_min,   // minutes  0..59
                  aseCommon.time.tm_sec,   // seconds  0..59
                  _10msec,
                  aseCommon.isChannelA ? "A" : "B"
                  );

        // Write the system tick value to video memory.
        debug_str(AseMain, 2, 0, "SecComm(%s) %d - %d",
                  secComm.GetSocketInfo(),
                  frames, td);

        debug_str(AseMain, 3, 0, "Rx(%d) Tx(%d) IsRx: %s CloseConn: %s Idle Time: %4d/%d",
                  secComm.GetRxCount(),
                  secComm.GetTxCount(),
                  secComm.isRxing ? "Yes" : "No ",
                  secComm.forceConnectionClosed ? "Yes" : "No",
                  cmdIdle+1,
                  MAX_IDLE_FRAMES
                  );

        debug_str(AseMain, 4, 0, "%s", secComm.GetErrorMsg());

        // Yield the CPU and wait until the next period to run again.
        waitUntilNextPeriod();
        
        UpdateTime();

        frames += 1;

        // Any new cmds seen
        if (CheckCmds( secComm))
        {
            lastCmdAt = frames;
        }
        else
        {
            // no connection loss timeout if we are running a script
            cmdIdle = frames - lastCmdAt;
            if (cmdIdle > MAX_IDLE_FRAMES)
            {
                secComm.forceConnectionClosed = TRUE;
                lastCmdAt = frames;
            }
        }

        aseCommon.bConnected = secComm.IsConnected();
    }
}

//-------------------------------------------------------------------------------------------------
// Update the wall clock time - PySte will resend every now and then but wee need to maintain it
// between those updates.
static void UpdateTime()
{
    // keep time
    _10msec += 10;
    if (_10msec >= 1000)
    {
        _10msec = 0;
        aseCommon.time.tm_sec += 1;
        if (aseCommon.time.tm_sec >= 60)
        {
            aseCommon.time.tm_sec = 0;
            aseCommon.time.tm_min += 1;
            if (aseCommon.time.tm_min >= 60)
            {
                aseCommon.time.tm_min = 0;
                aseCommon.time.tm_hour += 1;
                if (aseCommon.time.tm_hour >= 24)
                {
                    aseCommon.time.tm_hour = 0;
                    aseCommon.time.tm_year = nextTime.tm_year;
                    aseCommon.time.tm_mon = nextTime.tm_mon;
                    aseCommon.time.tm_mday = nextTime.tm_mday;
                }
            }
        }
    }

    // update the ships time
    if (_10msec == 0)
    {
        UpdateShipDate();
        UpdateShipTime();
        debug_str(AseMain, 7, 0, "Ship Date: 0x%08x Time: 0x%08x",
            aseCommon.shipDate,
            aseCommon.shipTime);
    }
}

//-------------------------------------------------------------------------------------------------
// Update the ships date - 
// pack into label 0260 reverse => 0x0D
// sdi = 1
// ssm = 11
static void UpdateShipDate()
{
#define DATE_SSM 0x60000000
#define DATE_SDI 0x100
#define DATE_LABEL 0x0d

    UINT8 dayX10 = aseCommon.time.tm_mday / 10;
    UINT8 dayX1  = aseCommon.time.tm_mday % 10;
    UINT8 monthX10 = aseCommon.time.tm_mon / 10;
    UINT8 monthX1  = aseCommon.time.tm_mon % 10;
    UINT8 yearX10  = (aseCommon.time.tm_year - 2000) / 10;
    UINT8 yearX1   = (aseCommon.time.tm_year - 2000) % 10;

    UINT32 dateData;
    dateData  = (dayX10 << 27) & 0x18000000L;
    dateData |= (dayX1  << 23) & 0x07800000L;
    dateData |= (monthX10 << 22) & 0x00400000L;
    dateData |= (monthX1 << 18) & 0x003c0000L;
    dateData |= (yearX10 << 14) & 0x0003c000L;
    dateData |= (yearX1 << 10) & 0x00003c00L;

    aseCommon.shipDate = DATE_SSM | dateData | DATE_SDI | DATE_LABEL;
}

//-------------------------------------------------------------------------------------------------
// Update the ships date - 
// pack into label 0150 reverse => 
// sdi = 1
// ssm = 11
static void UpdateShipTime()
{
#define TIME_SSM 0x60000000
#define TIME_SDI 0x100
#define TIME_LABEL 0x16

    UINT32 timeData = (aseCommon.time.tm_hour << 23) & 0x0f800000L;
    timeData |= (aseCommon.time.tm_min << 17) & 0x007e0000L;
    timeData |= (aseCommon.time.tm_sec << 11) & 0x0001f800L;

    aseCommon.shipTime = TIME_SSM | timeData | TIME_SDI | TIME_LABEL;
}

//-------------------------------------------------------------------------------------------------
static BOOLEAN CheckCmds(SecComm& secComm)
{
    void *theAreg;
    accessStyle asA;
    UNSIGNED32 myChanID;
    resourceStatus  status;
    platformResourceHandle hA;

    UINT32 i;
    BOOLEAN cmdSeen = FALSE;
    BOOLEAN serviced = FALSE;
    ResponseType rType = eRspNormal;

    if (secComm.IsCmdAvailable())
    {
        cmdSeen = TRUE;
        SecRequest request = secComm.m_request;

        videoRedirect = (VID_DEFS)request.videoDisplay;

        debug_str(AseMain, 5, 0, "Last Cmd Id: %d        ", request.cmdId);

        switch (request.cmdId)
        {
        case eRunScript:
            aseCommon.bScriptRunning = TRUE;
            SetTime(request);
            secComm.m_response.successful = TRUE;
            serviced = TRUE;
            break;

        case eScriptDone:
            aseCommon.bScriptRunning = FALSE;
            SetTime(request);
            secComm.m_response.successful = TRUE;
            serviced = TRUE;
            break;

        case eShutdown:
            aseCommon.bScriptRunning = FALSE;
            SetTime(request);
            secComm.m_response.successful = TRUE;
            serviced = TRUE;
            break;

        case ePing:
            // ping carries the current time and next time
            SetTime(request);
            secComm.m_response.successful = TRUE;
            serviced = TRUE;
            break;

        case ePowerOn:
            // Create the ADRF process to simulate behavior during power on
            if ( adrfProcHndl == NULL)
            {
                adrfProcStatus = createProcess( "adrf", "adrf-template", 0, TRUE, &adrfProcHndl);
                debug_str(AseMain, 6, 0, "PowerOn: Create process %s returned: %d",
                                                                 adrfName,
                                                                 adrfProcStatus);
                // Update the global-shared data block
                aseCommon.adrfState = (processSuccess == adrfProcStatus) ? eAdrfOn : eAdrfOff;
            }
            SetTime(request);
            secComm.m_response.successful = TRUE;
            serviced = TRUE;
            break;

        case ePowerOff:
            // Kill the ADRF process to simulate behavior during power off
            if (adrfProcHndl != NULL)
            {
                adrfProcStatus = deleteProcess( adrfProcHndl);
                debug_str(AseMain, 6, 0, "PowerOff: Delete process %s returned: %d",
                                                   adrfName,
                                                   adrfProcStatus);
                adrfProcStatus =  processNotActive;
                adrfProcHndl   = NULL;
            }
            // Update the global-shared data block
            aseCommon.adrfState = eAdrfOff;

            SetTime(request);
            secComm.m_response.successful = TRUE;
            serviced = TRUE;
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
            serviced = TRUE;
            break;

        //---------------
        case eSetChanId:
            ioiProc.SetChanId(request.variableId);

            // default to Channel A
            aseCommon.isChannelA = ioiProc.GetChanId() == 1;

            secComm.m_response.successful = TRUE;
            serviced = TRUE;
            break;

         //---------------
        case eNvmRead:
            if (nvmBaseAddr != NULL)
            {
                secComm.m_response.successful = NvmRead(secComm);
            }
            else
            {
                secComm.ErrorMsg("NVM Memory Handle Error");
                secComm.m_response.successful = FALSE;
            }
            serviced = TRUE;
            break;

         //---------------
        case eNvmWrite:
            if (nvmBaseAddr != NULL)
            {
                secComm.m_response.successful = NvmWrite(secComm);
            }
            else
            {
                secComm.ErrorMsg("NVM Memory Handle Error");
                secComm.m_response.successful = FALSE;
            }

            serviced = TRUE;
            break;

        case eGetAseVersion:
            strcpy(secComm.m_response.streamData, version);
            secComm.m_response.streamSize = strlen(version);
            secComm.m_response.successful = TRUE;
            serviced = TRUE;

        default:
            break;
        }

        if (serviced)
        {
            secComm.SetHandler("AseMain");
            secComm.IncCmdServiced(rType);
        }
        else
        {
            // Run all of the cmd response threads
            for (i=0; i < MAX_CMD_RSP; ++i)
            {
                if (cmdRspThreads[i]->CheckCmd(secComm))
                {
                    break;
                }
            }
        }
    }

    return cmdSeen;
}

//-------------------------------------------------------------------------------------------------
static void SetTime(SecRequest& request)
{
    aseCommon.time.tm_year = request.variableId;  // year from 1900
    aseCommon.time.tm_mon  = request.sigGenId;   // month    0..11
    aseCommon.time.tm_mday = request.resetRequest;  // day of the month  1..31
    aseCommon.time.tm_hour = request.clearCfgRequest;  // hours    0..23
    aseCommon.time.tm_min  = int(request.value);   // minutes  0..59
    aseCommon.time.tm_sec  = int(request.param1);   // seconds  0..59

    nextTime.tm_year = int(request.param2);
    nextTime.tm_mon  = int(request.param3);
    nextTime.tm_mday = int(request.param4);

    _10msec = 0;
}

//-------------------------------------------------------------------------------------------------
// Read from n bytes <sigGenId> NVM memory at the offset address specified <variableId>.  The
// offset address is from the base of NVM memory.  This assumes all addresses are inside our 
// NVM [0..NvmSize-1] bytes.
//
static BOOLEAN NvmRead(SecComm& secComm)
{
    UINT32 offset = secComm.m_request.variableId;
    UINT32 bytes = secComm.m_request.sigGenId;

    if ((offset + bytes) < NvmSize)
    {
        if (bytes <= eSecStreamSize)
        {
            memcpy(secComm.m_response.streamData, (void*)(nvmBaseAddr + offset), bytes);
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
static BOOLEAN NvmWrite(SecComm& secComm)
{
    UINT32 offset = secComm.m_request.variableId;
    UINT32 size = secComm.m_request.charDataSize;
    UINT16* data;

    if ((offset + size) < NvmSize)
    {
        NV_WriteAligned((void*)(nvmBaseAddr+offset),
                        (void*)secComm.m_request.charData,
                        size);
    }

   return TRUE;
}

static
void NV_WriteAligned(void* dest, const void* src, UINT32 size)
{
    UINT16 aligner;
    UINT16         *dest_ptr16 = (UINT16*)dest;
    const UINT16   *src_ptr16 = (UINT16*)src;
    UINT8*         ptr8;

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


