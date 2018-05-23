#ifndef A717QAR_H
#define A717QAR_H
/******************************************************************************
Copyright (C) 2018 Knowlogic Software Corp.
All Rights Reserved. Proprietary and Confidential.

File:        A717QAR.h

Description: This file declares test control object for the UTAS Physical A717 
QAR and the sub-frame (SF) data and status.


VERSION
$Revision: $  $Date: $

******************************************************************************/


/*****************************************************************************/
/* Compiler Specific Includes                                                */
/*****************************************************************************/

/*****************************************************************************/
/* Software Specific Includes                                                */
/*****************************************************************************/
#include <ioiapi.h>
#include "SecComm.h"

/*****************************************************************************/
/* Local Defines                                                             */
/*****************************************************************************/
//  Defines for UTAS QAR_STATUS statusRegister field masks
// TODO DaveB trim to 16bits to match agreed interface
#define SYNC_MASK         0x00000001   // Overall SYNC indicator
#define SF1_VLD_MASK      0x00000002   // Sub-Frame 1 Valid status
#define SF2_VLD_MASK      0x00000004   // Sub-Frame 2 Valid status
#define SF3_VLD_MASK      0x00000008   // Sub-Frame 3 Valid status
#define SF4_VLD_MASK      0x00000010   // Sub-Frame 4 Valid status
#define RCV_OVRUN_MASK    0x00000020   // Receiver Overrun Error
#define RCV_OP_ERR_MASK   0x00000040   // Receiver Operational Error
#define RCV_BUF_FULL_MASK 0x00000080   // Receiver Buffer Full
#define RCV_ACTY_MASK     0x00000100   // Receiver Activity - Latched
#define TST_MODE_ACT_MASK 0x00000200   // Test Mode Active
#define SYNC_MOD_ACT_MASK 0x00000C00   // selected/active low level SYNC module 0->3 (1..4)

/*****************************************************************************/
/* Local Typedefs                                                            */
/*****************************************************************************/
enum
{
  eQARSkipSF     = -1, // mask of SF to not transmit/simulate error
  eQARSetSize    = -2, // Set the sf size in words, 64,128,256,512,1024
  eNUM_SUBFRAMES =  4,
  eTICKS_PER_SEC = 100, // Object is invoked every 10 mSecs
  eSEND_TICK     = (eNUM_SUBFRAMES * eTICKS_PER_SEC), // Send a SF every 4 secs
  MAX_QAR_WORDS  = 1024,  // max # of 32 bit words per SF.
};

// Note: keep QAR_FORMAT in sync ParamSrcA717QAR.h <-> Ase/A717QAR.h 
typedef enum
{
  QAR_BIPOLAR_RETURN_ZERO, // (default)
  QAR_HARVARD_BIPHASE,
}QAR_FORMAT;

// Note: keep QAR_RUN_STATE in sync ParamSrcA717QAR.h <-> Ase/A717QAR.h
typedef enum 
{
  eRUNNING = 0,
  eNO_VALID_CFG,
  eERR_CBIT,
  eERR_PBIT,
} QAR_RUN_STATE;

typedef struct
{
  UINT32* pdata;     // ptr to memory buffer for IOI read/write
  int     fd;        // ioi file descriptor returned from ioiOpen. 
} A717_IOI_STRUCT, *A717_QAR_DATA_PTR;


// UTAS QAR status msg. Output at 1 Hz. This is the same structure used by
// ParamA717QAR.h
#pragma pack(1)
typedef struct
{
  UINT8 bDisabled;              // Indicates whether the QAR Module is disabled.
  QAR_RUN_STATE qarRunState;    // RUNNING, NO_VALID_CFG,ERR_CBIT, ERR_PBIT
  UINT8 subframeID;             // 1–4 indicating the latest SF written to IOI, 0 if no data
  BOOLEAN bRevSync;             // Flag indicating barker codes are bit-reversed
                                // 0: Regular sync pattern, 1: Reversed sync pattern 
  UINT16 numWords;              // # of 32-bits words per SF: 64,128,256,512 or 1024
  UINT32 fmt;                   // encoding enum 0=BIPOLAR_RETURN_ZERO,1=HARVARD_BIPHASE
  UINT32 statusRegister;        // Echo of current AR717 RX Status Register
}QAR_MODULE_STATUS;
#pragma pack()

// TEST Control is where cmds from Test Environment are stored.
// These are used to affect processing flow

typedef struct
{
  BOOLEAN m_bSynced;      // Simulate the QAR modules being in or out sync.
  BOOLEAN m_bQarEnabled;  // The module is enabled and will generate output based on settings.
  BOOLEAN m_acceptCfgReq; // Accept/Reject cfg request from ADRF
  QAR_RUN_STATE qarRunState; // The commanded state to be in
} TEST_CONTROL;

/*****************************************************************************/
/* Local Variables                                                           */
/*****************************************************************************/

/*****************************************************************************/
/* Constant Data                                                             */
/*****************************************************************************/

/*****************************************************************************/
/* Local Function Prototypes                                                 */
/*****************************************************************************/

/*****************************************************************************/
/* Public Functions                                                          */
/*****************************************************************************/

/*****************************************************************************/
/* Class Definitions                                                         */
/*****************************************************************************/

/*****************************************************************************/
/* Forward Declaration                                                       */
/*****************************************************************************/

//=============================================================================================
// A717Subframe encapsulates the behavior and properties of one of the 4 SF
// managed by the A717QAR
class A717Subframe
{
  public:
    ioiStatus   m_ioiStatusSf;   // The most recent ioi Status for this SF
    bool        m_ioiValid;      // Status of the SF ioi for this A717Subframe
    const CHAR* m_pIoiName;      // ptr to ioi name for outputting this SF's data

    A717Subframe( const CHAR* pIoiName, int sfIdx, int barker );
    void InitIoi();    
    void Reset();
    void ResetBarker();
    bool UpdateBuffer( UINT32 offset, UINT8* pBuffer, UINT32 byteCnt );

    BOOLEAN UpdateIoi();  // write the SF content to the IOI    

  protected:    
    A717_IOI_STRUCT   m_ioiObj;  // This SF output IOI buffer and file desc.

    int m_frameSize;        // Number of 32 bit words in the SF
    int m_mySFNum;          // my object's SF number, 1..4
    int m_mySfMask;         // My SF mask to identify commands to this SF-obj
    INT16 m_stdBarker;      // The normal Barker code for this SF.
    INT16 m_revBarker;      // The reverse Barker code for this SF.
    INT16 m_bDefaultBarker; // Use default barker unless tester specified a value
    int m_writeErrCnt;

    UINT32 m_qarWords[MAX_QAR_WORDS]; // 1024L = 4096Bytes
};

//=============================================================================================
/* The A717QAR is responsible for implementing the behavior and state of a UTAS A717 QAR
 *
 * This includes:
 *  Configuration protocol
 *  Setting and sending a status update msg @1Hz
 *  Sending the A717Subframe buffer at 1Hz
 */

class A717Qar
{
  public:
    A717Qar();

    // Methods called by ioiProcess
    void Reset();
    void UpdateIoi();
    void InitIoi();

    // SEC command handlers.
    void SetRunState( QAR_RUN_STATE newState);
    QAR_RUN_STATE GetRunState()   { return m_qarModStatus.qarRunState;}
    void SetQarData( UINT8 sfMask, UINT32 offset, UINT8* pBuffer, UINT32 byteCnt );
    void ResetBarkers( UINT8 sfMask );
    void SetWordSize( UINT32 wordSize );

  protected:
    BOOLEAN m_bInit;         // Used to complete init tasks not possible during construction
    BOOLEAN m_bInitSFOutput; // QAR obj is in startup mode.  send SF sequence starting with 1

    TEST_CONTROL m_testCtrl; // Structure holding the state of cmd setting from the test env

    // These properties represent the register values in m_qarStatus.register.

    //int m_sendTimeTick; // The tick cnt value at which this SF will send it's buffer
    int m_crntTick;     // The current tick value. Incremented when ioiProcess calls UpdateIoi.
    UINT8 m_nextSfIdx;  // The idx into m_pSF array of the next sf to send.
    int m_writeErrCnt;  // total write error counts

    // Objects to handle each A717 SF IOI
    A717Subframe m_a717QarSf1;           // why not A717Subframe m_a717QarSf[4]; ?
    A717Subframe m_a717QarSf2;           
    A717Subframe m_a717QarSf3;
    A717Subframe m_a717QarSf4;
    A717Subframe* m_pSF[eNUM_SUBFRAMES]; // convenience array of ptr to the SF obj

    // UTAS Status msg to HMU listeners
    // It is written at 1Hz regardless of whether the QAR is enabled or synced    
    QAR_MODULE_STATUS m_qarModStatus;    // msg struct of UTAS A717 Module status
    A717_IOI_STRUCT   m_moduleStatusIoi; // output IOI for the 1Hz QAR status broadcast    
    BOOLEAN           m_statusIoiValid;  // status validity for the status ioi
    ioiStatus         m_ioiStatus;       // TBD may not need it.


    // Config request/response IOIs
    A717_IOI_STRUCT m_reqCfgIoi; // input  IOI to handle cfg requests from ADRF
    A717_IOI_STRUCT m_rspCfgIoi; // input  IOI to handle cfg requests from ADRF 
        
    void Initialize();
    void WriteQarStatusMsg(UINT8 subframeID);
};

#endif
//
