#ifndef IOI_STATIC_CLS_H
#define IOI_STATIC_CLS_H
/******************************************************************************
Copyright (C) 2018 Knowlogic Software Corp.
All Rights Reserved. Proprietary and Confidential.

File:        ioiStaticCls.h

Description: This file declares all the various IOI static class types.  Some
are dumb, some are smart.

VERSION
$Revision: $  $Date: $

******************************************************************************/

/*****************************************************************************/
/* Compiler Specific Includes                                                */
/*****************************************************************************/
#include <ioiapi.h>

/*****************************************************************************/
/* Software Specific Includes                                                */
/*****************************************************************************/
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
    eQARSkipSF = -1, // mask of SF to not transmit/simulate error
    eQARSetSize = -2, // Set the sf size in words, 64,128,256,512,1024
    eNUM_SUBFRAMES = 4,
    eTICKS_PER_SEC = 100, // Object is invoked every 10 mSecs
    eSEND_TICK = (eNUM_SUBFRAMES * eTICKS_PER_SEC), // Send a SF every 4 secs
    MAX_QAR_WORDS = 1024,  // max # of 32 bit words per SF.
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

// UTAS QAR status msg. Output at 1 Hz. This is the same structure used by
// ParamA717QAR.h
#pragma pack(1)
typedef struct
{
    UINT8 bDisabled;       // Indicates whether the QAR Module is disabled.
    UINT8 qarRunState;     // RUNNING, NO_VALID_CFG,ERR_CBIT, ERR_PBIT
    UINT8 subframeID;      // 1–4 indicating the latest SF written to IOI, 0 if no data
    UINT8 bRevSync;        // Flag indicating barker codes are bit-reversed
                           // 0: Regular sync pattern, 1: Reversed sync pattern 
    UINT8  numWords;       // # of 32-bits words per SF: 64,128,256,512 or 1024
    UINT8  fmt;            // encoding enum 0=BIPOLAR_RETURN_ZERO,1=HARVARD_BIPHASE
    UINT16 statusRegister; // Echo of current AR717 RX Status Register
}QAR_MODULE_STATUS;
#pragma pack()

// TEST Control is where cmds from Test Environment are stored.
// These are used to affect processing flow

typedef struct
{
    BOOLEAN m_bSynced;      // Simulate the QAR modules being in or out of sync.
    BOOLEAN m_bQarEnabled;  // Module is enabled and will generate output based on settings.
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
//=============================================================================================
class StaticIoiObj
{
public:
    char  m_ioiName[64];   // IOI binder producer name
    char  m_shortName[64]; // short name for display on console
    INT32 m_ioiChan;       // DEOS ioi channel id
    bool  m_ioiValid;      // is the ioi opened
    bool  m_ioiRunning;    // is the ioi being output on a regular basis
    bool  m_isAseInput;    // this signal is an input to ASE
    bool  m_isParam;       // indicates this IDL is being run by a Parameter
    INT32 m_updateCount;   // how many times has the value been read/written

    StaticIoiObj(char* name, bool isInput = false);
    bool OpenIoi();
    void SetRunState(bool newState);
    char* CompressName(char* src, int size);

    // virtual functions
    virtual bool SetStaticIoiData(SecRequest& request) { m_ioiRunning = true; }

    // streamSize: Byte, Integer
    // streamData: String, IntegerPtr
    virtual bool GetStaticIoiData(IocResponse& m_response) {}

    virtual bool Update() { return true; }
    virtual bool WriteStaticIoi(void* data);
    virtual bool ReadStaticIoi(void* data);
    virtual char* Display(char* dest, UINT32 dix);

    virtual UINT8* Data(UINT16* destSize) {
        *destSize = 0;
        return NULL;
    }
};

//=============================================================================================
class StaticIoiInt : public StaticIoiObj
{
public:
    StaticIoiInt(char* name, int value, bool isInput = false)
        : StaticIoiObj(name, isInput)
        , data(value)
    {}
    virtual bool SetStaticIoiData(SecRequest& request);
    virtual bool GetStaticIoiData(IocResponse& m_response);
    virtual bool Update();
    virtual char* Display(char* dest, UINT32 dix);
    int data;
    virtual UINT8* Data(UINT16* destSize) {
        *destSize = 4;
        return (UINT8*)&data;
    }
};

//=============================================================================================
class StaticIoiFloat : public StaticIoiObj
{
public:
    StaticIoiFloat(char* name, float value, bool isInput = false)
        : StaticIoiObj(name, isInput)
        , data(value)
    {}
    virtual bool SetStaticIoiData(SecRequest& request);
    virtual bool GetStaticIoiData(IocResponse& m_response);
    virtual bool Update();
    virtual char* Display(char* dest, UINT32 dix);
    float data;
    virtual UINT8* Data(UINT16* destSize) {
        *destSize = 4;
        return (UINT8*)&data;
    }
};

//=============================================================================================
class StaticIoiStr : public StaticIoiObj
{
public:
    StaticIoiStr(char* name, char* value, int size, bool isInput = false);
    virtual bool SetStaticIoiData(SecRequest& request);
    virtual bool GetStaticIoiData(IocResponse& m_response);
    virtual bool Update();
    virtual char* Display(char* dest, UINT32 dix);
    int displayAt;
    char* data;
    int bytes;
    virtual UINT8* Data(UINT16* destSize) {
        *destSize = (UINT16)bytes;
        return (UINT8*)data;
    }
};

//=============================================================================================
class A664Qar
{
public:
    enum a664QarConst {
        // Set Data Control Commands
        eQarNdo = -1,
        eQarSfSeq = -2,
        eQarWordSeq = -3,
        eQarWordSeqState = -4,
        eQarRandom = -5,
        eQarGarbage = -6,
        eQarStop = 0x0f0f,

        eSfCount = 4,
        eBurstCount = 20,        // 16 small bursts, 4 large bursts = 1024 words
        eSmallBurstSize = 51,
        eTotalSmallBurstWords = (16 * eSmallBurstSize),
        eLargeBurstSize = 52,
        eMaxBurstWords = 102,
        eSfWordCount = 1024,
    };
    A664Qar();
    void Reset(StaticIoiObj* buffer);

    void NextSf();      // compute the next SF to run
    UINT32 NextWord();  // compute the next word in this sub-frame to send, returns busrtWord

    void Garbage();

    virtual int UpdateIoi();
    virtual bool Update();
    virtual bool HandleRequest(StaticIoiObj* targetIoi);  // is one of our static IOI
    virtual bool TestControl(SecRequest& request);

    StaticIoiStr* m_idl;     // the IOI buffer sending the data

    int m_sf;                      // which sub-frame are we outputting
    int m_sfWordIndex;             // word count of the 1024 words in a SF
    int m_burst;                   // which burst of the sub-frame are we sending
    int m_burstWord;               // which word in the bust we are on
    int m_burstSize[eBurstCount];  // the size of each of the 20 bursts being sent / SF
    bool m_endBurst;               // end of a burst of data words
    int m_random;                  // number of random values to put in 0-50, default 50
    int m_kMaxRandom;              // maximum number of random words
    int m_randomSave;              // save the random value when running Word Seq Buf Operation
    int m_garbageSet;              // number of total garbage bursts to send
    int m_garbageCnt;              // remaining count of garbage

    UINT16 m_wordSeqEnabled;         // is Word Seq Buf Operation enabled
    UINT16 m_wordSeq[eSfCount][eSfWordCount]; //Word Seq Buf Operation opcodes & operand
    int m_repeatCount;
    int m_repeatIndex;

    int m_ndo[eSfCount];
    int m_nonNdo;                  // a value that is not one of the 4 NDO values and not 0
    int m_frameCount;              // how many frames have been sent since the last reset

    UINT32 m_schedule;   // used to schedule A664 QAR at 20Hz
    UINT32 m_a664QarSF;  // used to schedule A664 QAR SF 0 .. 3

                         // ERROR injection control
    int m_skipSfMask;  // which SF should we skip? bit0=SF1, bi1=SF2, etc.

                       // four sub-frames worth of data
    UINT16 m_qarWords[eSfCount][eSfWordCount];  // data to be transmitted
};

//=============================================================================================
class A717Qar : public A664Qar
{
public:
    A717Qar();

    // Methods called by the IOI Static container
    void InitIoi();
    void Reset(StaticIoiObj* cfgRqst, StaticIoiObj* cfgRsp, StaticIoiObj* sts,
               StaticIoiObj* sf1, StaticIoiObj* sf2, StaticIoiObj* sf3, StaticIoiObj* sf4);

    // SEC command handlers.
    virtual int UpdateIoi();
    virtual bool TestControl(SecRequest& request);
    virtual bool HandleRequest(StaticIoiObj* targetIoi);  // is one of our static IOI

    UINT32  m_qarSfWordCount; // number of wrods we are configured to send

    BOOLEAN m_bInit;         // Used to complete init tasks not possible during construction
    BOOLEAN m_bInitSFOutput; // QAR obj is in startup mode.  send SF sequence starting with 1

    TEST_CONTROL m_testCtrl; // Structure holding the state of cmd setting from the test env

                             // These properties represent the register values in m_qarStatus.register.

                             //int m_sendTimeTick; // The tick cnt value at which this SF will send it's buffer
    int m_crntTick;     // The current tick value. Incremented when ioiProcess calls UpdateIoi.
    UINT8 m_nextSfIdx;  // The idx into m_pSF array of the next sf to send.
    int m_writeErrCnt;  // total write error counts

                        // UTAS Status msg to HMU listeners
                        // It is written at 1Hz regardless of whether the QAR is enabled or synced    
    BOOLEAN           m_statusIoiValid;  // status validity for the status ioi
    ioiStatus         m_ioiStatus;       // TBD may not need it.

    QAR_MODULE_STATUS m_qarModStatus;    // msg struct of UTAS A717 Module status

    StaticIoiStr* m_cfgRqst;      // cfg rqst IOI
    StaticIoiStr* m_cfgRsp;       // cfg rsp IOI
    StaticIoiStr* m_status;       // status IOI
    StaticIoiStr* m_sf[eSfCount]; // SF IOIs

    void WriteQarStatusMsg(UINT8 subframeID);
};


/*****************************************************************************/
/* Forward Declaration                                                       */
/*****************************************************************************/


#endif
//
