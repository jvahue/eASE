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
#include "video.h"
#include "SecComm.h"
#include "CmProcess.h"
#include "ioiProcess.h"
#include "AseCommon.h"
#include "File.h"

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

/*****************************************************************************/
/* Global Variables                                                          */
/*****************************************************************************/
AseCommon aseCommon;

/*****************************************************************************/
/* Constant Data                                                             */
/*****************************************************************************/
CmdRspThread* cmdRspThreads[MAX_CMD_RSP] = {
  &cmProc,
  &ioiProc
};

/*****************************************************************************/
/* Local Function Prototypes                                                 */
/*****************************************************************************/
static BOOLEAN CheckCmds(SecComm& secComm);
processStatus CreateAdrfProcess();

void FileSystemTestSmall();
void FileSystemTestBig();

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
    const UINT32 MAX_IDLE_FRAMES = (5 * 60) * systemTickTimeInHz;

    UINT32 i;

    SecComm secComm;
    UINT32 frames = 0;
    UINT32 lastCmdAt = 0;

    memset( &aseCommon, 0, sizeof(aseCommon));

    // default to MS being online
    aseCommon.bMsOnline = true;

    // Grab the system tick pointer, all threads/tasks should use GET_SYSTEM_TICK
    aseCommon.systemTickPtr = systemTickPointer();

    debug_str_init();

    // Initially create the adrf to start it running.
    adrfProcStatus  = createProcess( adrfName, adrfTmplName, 0, TRUE, &adrfProcHndl);
    debug_str(AseMain, 5, 0, "Initial Create of adrf returned: %d", adrfProcStatus);

    aseCommon.bPowerOnState = (processSuccess == adrfProcStatus);

    secComm.Run();

    // Run all of the cmd response threads
    for (i=0; i < MAX_CMD_RSP; ++i)
    {
        cmdRspThreads[i]->Run(&aseCommon);
    }

    //FileSystemTestBig();

    debug_str(AseMain, 2, 0, "Last Cmd Id: 0");


    // The main thread goes into an infinite loop.
    while (1)
    {
        // Write the system tick value to video memory.
        debug_str(AseMain, 0, 0, "SecComm(%s) %d",
                  secComm.GetSocketInfo(),
                  frames);

        debug_str(AseMain, 1, 0, "Rx(%d) Tx(%d)",
                  secComm.GetRxCount(),
                  secComm.GetTxCount()
                  );

        debug_str(AseMain, 3, 0, "%s", secComm.GetErrorMsg());

        // Yield the CPU and wait until the next period to run again.
        waitUntilNextPeriod();
        frames += 1;

        // Any new cmds seen
        if (CheckCmds( secComm))
        {
            lastCmdAt = frames;
        }
        else if ((frames - lastCmdAt) > MAX_IDLE_FRAMES)
        {
            secComm.forceConnectionClosed = TRUE;
            lastCmdAt = frames;
        }

        aseCommon.bConnected = secComm.IsConnected();
    }
}

//-------------------------------------------------------------------------------------------------
static BOOLEAN CheckCmds(SecComm& secComm)
{
    UINT32 i;
    BOOLEAN cmdSeen = FALSE;
    BOOLEAN serviced = FALSE;
    ResponseType rType = eRspNormal;

    if (secComm.IsCmdAvailable())
    {
        cmdSeen = TRUE;
        SecRequest request = secComm.m_request;

        debug_str(AseMain, 2, 0, "Last Cmd Id: %d        ", request.cmdId);

        switch (request.cmdId)
        {
        case eRunScript:
            aseCommon.bScriptRunning = TRUE;
            secComm.m_response.successful = TRUE;
            serviced = TRUE;
            break;

        case eScriptDone:
            aseCommon.bScriptRunning = FALSE;
            secComm.m_response.successful = TRUE;
            serviced = TRUE;
            break;

        case eShutdown:
            aseCommon.bScriptRunning = FALSE;
            secComm.m_response.successful = TRUE;
            serviced = TRUE;
            break;

        case ePing:
            secComm.m_response.successful = TRUE;
            serviced = TRUE;
            break;

        case ePowerOn:
            // Create the ADRF process to simulate behavior during power on
            if ( adrfProcHndl == NULL)
            {
                adrfProcStatus = createProcess( "adrf", "adrf-template", 0, TRUE, &adrfProcHndl);
                debug_str(AseMain, 5, 0, "PowerOn: Create process %s returned: %d",
                                                                 adrfName,
                                                                 adrfProcStatus);
                // Update the global-shared data block
                aseCommon.bPowerOnState = TRUE;
            }
            secComm.m_response.successful = TRUE;
            serviced = TRUE;
            break;

        case ePowerOff:
            // Kill the ADRF process to simulate behavior during power off
            if (adrfProcHndl != NULL)
            {
                adrfProcStatus = deleteProcess( adrfProcHndl);
                debug_str(AseMain, 5, 0, "PowerOff: Delete process %s returned: %d",
                                                   adrfName,
                                                   adrfProcStatus);
                adrfProcStatus =  processNotActive;
                adrfProcHndl   = NULL;
            }
            // Update the global-shared data block
            aseCommon.bPowerOnState = FALSE;

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

        case eVideoRedirect:
            // Kill the ADRF process to simulate behavior during power off
            if (request.variableId >= VID_SYS || request.variableId < VID_MAX)
            {
                videoRedirect = (VID_DEFS)request.variableId;
                secComm.m_response.successful = TRUE;
            }
            else
            {
                secComm.ErrorMsg("Invalid Video Screen ID: Accept 0..%d, got %d", VID_MAX-1, request.variableId);
                secComm.m_response.successful = FALSE;
            }
            serviced = TRUE;
            break;



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

processStatus CreateAdrfProcess()
{
    return createProcess( "adrf", "adrf-template", 0, TRUE, &adrfProcHndl);
}

void FileSystemTestSmall()
{
    char writeTest[] =
    {   "[01 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ]"
        "[02 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ]"
        "[03 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ]"
        "[04 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ]"
        "[05 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ]"
        "[06 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ]"
        "[07 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ]"
        "[08 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ]"
        "[09 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ]"
        "[10 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ]"
        "[11 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ]"
        "[12 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ]"
        "[13 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ]"
        "[14 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ]"
        "[15 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ]"
        "[16 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ]"
        "[17 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ]"
        "[18 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ]"
        "[19 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ]"
        "[20 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ]"
        "[21 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ]"
        "[22 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ]"
        "[23 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ]"
        "[24 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ]"
        "[25 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ]"
        "[26 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ]"
        "[27 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ]"
        "[28 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ]"
        "[29 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ]"
        "[30 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ]"
        "[31 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ]"
        "[32 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ]"
        "[33 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ]"
        "[34 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ]"
        "[35 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ]"
        "[36 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ]"
        "[37 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ]"
        "[38 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ]"
        "[39 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ]"
        "[40 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ]"
        "[41 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ]"
        "[42 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ]"
        "[43 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ]"
        "[44 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ]"
        "[45 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ]"
        "[46 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ]"
        "[47 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ]"
        "[48 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ]"
   };
    SIGNED16   i, recSize = 41;
    char readTest[50*41];
    UNSIGNED32 offset  = 0;
    BOOLEAN ok = TRUE;
    File      fileObj;

    fileObj.Delete("TestFile.cfg", File::ePartCmProc);

    // PERFORM WRITE
    if (1)
    {
        fileObj.Open("TestFile.cfg", File::ePartCmProc, 'w');
        do
        {
            if (!fileObj.Write(&writeTest[offset], recSize))
            {
                ok = FALSE;
            }
            offset += recSize;
        }
        while( ok && offset < strlen(writeTest) );
        fileObj.Close();
    }

    // PERFORM READ
    fileObj.Open("TestFile.cfg", File::ePartCmProc, 'r');
    offset  = 0;
    SIGNED32 bytesRead = 0;
    memset(readTest, 0x20, sizeof(readTest) );
    do
    {
        bytesRead = fileObj.Read(&readTest[offset], recSize);
        if (bytesRead == 0)
        {
            ok = FALSE;
        }
        offset += bytesRead;
    }
    while(ok && offset < strlen(writeTest));
    fileObj.Close();

    // COMPARE
    UNSIGNED32 size = strlen(writeTest);
    if ( 0 == strncmp(writeTest, readTest, size) )
    {
        i = 42;
    }
    else
    {
        i = 42;
    }
}

// write and read back/verify 8K integer numbers
void FileSystemTestBig()
{
    UNSIGNED32 limit = 8192;
    SIGNED16   i, recSize;
    SIGNED32   len;

    BOOLEAN ok = TRUE;
    #define SIZE 13
    UNSIGNED32 writeBuff[SIZE];
    UNSIGNED32 expectBuff[SIZE];
    UNSIGNED32 readBuff[SIZE];
    File      fileObj;
    void* ptr;

    recSize = sizeof(writeBuff);
    len     = SIZE;

    fileObj.Delete("TestFile.cfg", File::ePartCmProc);

    // PERFORM WRITE
    if (1)
    {
        fileObj.Open("TestFile.cfg", File::ePartCmProc, 'w');
        ptr = (void*)&writeBuff;
        for (i = 0; i < limit; ++i)
        {
            // Fill a buffer with incrementing numbers
            writeBuff[i%SIZE] = i;

            // When the buffer is full, write it out
            if ((i%SIZE) == (SIZE -1))
            {
                if (!fileObj.Write( ptr, recSize ))
                {
                    ok = FALSE;
                }
            }
        }
        fileObj.Close();
    }

    // PERFORM READ
    fileObj.Open("TestFile.cfg", File::ePartCmProc, 'r');
    ok = TRUE;
    SIGNED32 bytesRead = 0;
    BOOLEAN result = TRUE;

    ptr = (void*)&readBuff;
    void* ptrE = (void*)&expectBuff;

    for (i = 0; (i < limit + 2) && ok; ++i)
    {

        // Fill a buffer with incrementing numbers
        expectBuff[i%SIZE] = i;

        // When the buffer is full read the next rec and compare to expect
        if ((i%SIZE) == (SIZE -1))
        {
            bytesRead = fileObj.Read(ptr, recSize);
            if (bytesRead == 0)
            {
                ok = FALSE;
            }

            if (memcmp(readBuff,expectBuff,recSize) != 0)
            {
                result = FALSE;
            }
        }
    }
    fileObj.Close();
}

