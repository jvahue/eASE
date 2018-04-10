#ifndef IOISTATIC_H
#define IOISTATIC_H

#include <ioiapi.h>

#include "SecComm.h"

//?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|
// NOTE: 
//   This should be done with templates and that is left as an exercise for the reader
//?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|?|
#define MAX_STATIC_IOI 100

// File: ioiProcess.h
enum IoiStaticTypes {
    eStaticNone = 0,
    eStaticBit = 1,
    eStaticByte = 2,
    eStaticInt = 3,
    eStaticFloat = 4,
    eStaticStr = 5,
};

//=============================================================================================
class StaticIoiObj
{
public:
    char  m_ioiName[64];   // IOI binder producer name
    char  m_shortName[64]; // short name for display on console
    INT32 m_ioiChan;       // deos ioi channel id
    bool  m_ioiValid;      // is the ioi opened
    bool  m_ioiRunning;    // is the ioi being output on a regular basis
    bool  m_isAseInput;    // this signal is an input to ASE
    bool  m_isParam;       // indicates this IDL is being run by a Parameter
    INT32 m_updateCount;   // how many times has the value been read/written

    StaticIoiObj(char* name, bool isInput=false);
    bool OpenIoi();
    void SetRunState(bool newState);
    //void SetIO(bool isInput) {m_isAseInput = isInput;}

    // virtual functions
    //virtual IocResponse GetStaticIoiData() {;}
    virtual bool SetStaticIoiData(SecRequest& request) {m_ioiRunning = true;}

    // streamSize: Byte, Integer
    // streamData: String, IntegerPtr
    virtual bool GetStaticIoiData(IocResponse& m_response) {}

    virtual bool Update() {return true;}
    virtual bool WriteStaticIoi(void* data);
    virtual bool ReadStaticIoi(void* data);
    virtual char* Display(char* dest, UINT32 dix);

    char* CompressName(char* src, int size);

    virtual UINT8* Data(UINT16* destSize) {
        destSize = 0;
        return NULL;
    }
};

//=============================================================================================
//class StaticIoiByte : public StaticIoiObj
//{
//public:
//    StaticIoiByte(char* name, unsigned char value, bool isInput=false)
//        : StaticIoiObj(name, isInput)
//        , data(value)
//    {}
//    virtual bool SetStaticIoiData(SecRequest& request);
//    virtual bool GetStaticIoiData(IocResponse& m_response);
//    virtual bool Update();
//    virtual char*  Display(char* dest, UINT32 dix);
//    unsigned char data;
//};

//=============================================================================================
class StaticIoiInt : public StaticIoiObj
{
public:
    StaticIoiInt(char* name, int value, bool isInput=false)
        : StaticIoiObj(name, isInput) 
        , data(value)
    {}
    virtual bool SetStaticIoiData(SecRequest& request);
    virtual bool GetStaticIoiData(IocResponse& m_response);
    virtual bool Update();
    virtual char*  Display(char* dest, UINT32 dix);
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
    StaticIoiFloat(char* name, float value, bool isInput=false)
        : StaticIoiObj(name, isInput)
        , data(value)
    {}
    virtual bool SetStaticIoiData(SecRequest& request);
    virtual bool GetStaticIoiData(IocResponse& m_response);
    virtual bool Update();
    virtual char*  Display(char* dest, UINT32 dix);
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
    StaticIoiStr(char* name, char* value, int size, bool isInput=false);
    virtual bool SetStaticIoiData(SecRequest& request);
    virtual bool GetStaticIoiData(IocResponse& m_response);
    virtual bool Update();
    virtual char*  Display(char* dest, UINT32 dix);
    int displayAt;
    char* data;
    int bytes;
    virtual UINT8* Data(UINT16* destSize) {
        *destSize = (UINT16)bytes;
        return (UINT8*)data;
    }

};

//class StaticIoiIntPtr : public StaticIoiObj
//{
//public:
//    StaticIoiIntPtr(char* name, int* value, int size, bool isInput=false);
//    //virtual bool SetStaticIoiData(SecRequest& request);
//    virtual bool GetStaticIoiData(IocResponse& m_response);
//    virtual bool Update();
//    virtual char*  Display(char* dest, UINT32 dix);
//    int* data;
//    int bytes;
//};

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
    A664Qar(StaticIoiStr* buffer);
    void Reset();

    bool TestControl(SecRequest& request);

    void NextSf();      // compute the next SF to run
    UINT32 NextWord();  // compute the next word in this sub-frame to send, returns busrtWord

    void Update();
    void Garbage();
    
    StaticIoiStr* m_ioiBuffer;     // the IOI buffer sending the data

    int m_sf;                      // which sub-frame are we outputting
    int m_sfWordIndex;             // word count of the 1024 words in a SF
    int m_burst;                   // which burst of the sub-frame are we sending
    int m_burstWord;               // which word in the bust we are on
    int m_burstSize[eBurstCount];  // the size of each of the 20 bursts being sent / SF
    bool m_endBurst;               // end of a burst of data words
    int m_random;                  // number of random values to put in 0-50, default 50
    int m_kMaxRandom;              // maximum number of random words
    int m_randomSave;              // save the random value when running the WSB
    int m_garbageSet;              // number of total garbage bursts to send
    int m_garbageCnt;              // remaining count of garbage

    UINT16 m_wordSeqEnabled;         // is the WSB enabled
    UINT16 m_wordSeq[eSfCount][eSfWordCount]; // word sequence
    int m_repeatCount;
    int m_repeatIndex;

    int m_ndo[eSfCount];
    int m_nonNdo;                  // a value that is not one of the 4 NDO values and not 0
    int m_frameCount;              // how many frames have been sent since the last reset

    // ERROR injection control
    int m_skipSfMask;  // which SF should we skip? bit0=SF1, bi1=SF2, etc.


    // four sub-frames worth of data
    UINT16 m_qarWords[eSfCount][eSfWordCount];
};

//=============================================================================================
class A717Qar
{
  public:
    enum a717QarConst
    {
        eQARSkipSF     = -1, // mask of SF to not transmit/simulate error
        eQARSetSize    = -2, // Set the sf size in words, 64,128,256,512,1024
        eNUM_SUBFRAMES = 4,
        eTICKS_PER_SEC = 100, // Object is invoked every 10 mSecs
        eSEND_TICK = (eNUM_SUBFRAMES * eTICKS_PER_SEC), // Send a SF every 4 secs
        eSfWordCount   = 1024,
    };
    UINT32 m_writeErrCnt;
    StaticIoiStr* m_pIOIStr; // ptr to IOI buffer for outputting this SF's data

    A717Qar(StaticIoiStr* pBuffer, int sfIdx, int barker);
    void Reset();
    void UpdateIOI();
    bool TestControl(SecRequest& request);

  protected:
    int m_frameSize;  // Number of 32 bit words in the subframe
    int m_mySFNum;    // my object's SF number, 1..4
    int m_mySfMask;   // My SF mask to identify commands to this SF-obj
    int m_barker;     // Barker code for this SF. UTAS A717 barker cannot be configured!

    int m_crntTick;  // My current tick to control prep and sending SF buffers. 1 tick = 10mSec
    int m_sendTimeTick;  // The tick cnt value at which this SF will send it's buffer
    bool m_bStartup; // Flag to control timing when this sf will be sent on startup.
    UINT32 m_skipSfMask;// Broadcast to all A717 obj. SF to be bypassed for updating. Set by Pyste/SEC

    UINT32 m_qarWords[eSfWordCount]; // 1024L = 4096Bytes
    void Update();   // Perform ops on my SF data before handing off to IOI.
};

//=============================================================================================
class StaticIoiContainer
{
public:
    StaticIoiContainer();
    void OpenIoi();

    //IocResponse GetStaticIoiData(SecRequest& request);
    void UpdateStaticIoi();
    bool SetStaticIoiData(SecComm& secComm);
    bool GetStaticIoiData(SecComm& secComm);    
    void SetNewState(SecRequest& request);
    void Reset();
    void ResetStaticIoi();
    void ResetStaticParams();
    StaticIoiObj* FindIoi(char* name);

    StaticIoiObj* m_staticAseOut[MAX_STATIC_IOI];
    UINT32 m_ioiStaticOutCount;
    StaticIoiObj* m_staticAseIn[MAX_STATIC_IOI];
    UINT32 m_ioiStaticInCount;

    A664Qar m_a664Qar;

    // Objects to handle each A717 subframe IOI
    A717Qar m_a717QarSf1;
    A717Qar m_a717QarSf2;
    A717Qar m_a717QarSf3;
    A717Qar m_a717QarSf4;

    UINT32 m_aseInIndex;
    UINT32 m_aseOutIndex;
    UINT32 m_validIoiOut;
    UINT32 m_validIoiIn;
    UINT32 m_writeError;
    UINT32 m_writeErrorZ1;
    UINT32 m_readError;
    UINT32 m_readErrorZ1;
    UINT32 m_a664QarSched;  // used to schedule A664 QAR at 20Hz
    UINT32 m_a664QarSF;     // used to schedule A664 QAR SF 0 .. 3
};

#endif
