#ifndef IOISTATIC_H
#define IOISTATIC_H

#include "ioiStaticCls.h"  // Used to implement a UTAS QAR object
#include "SecComm.h"

#define MAX_STATIC_IOI 100

// File: ioiProcess.h
//enum IoiStaticTypes {
//    eStaticNone = 0,
//    eStaticBit = 1,
//    eStaticByte = 2,
//    eStaticInt = 3,
//    eStaticFloat = 4,
//    eStaticStr = 5,
//};

//=============================================================================================
class StaticIoiContainer
{
public:
    StaticIoiContainer();
    void OpenIoi();

    //IocResponse GetStaticIoiData(SecRequest& request);
    void UpdateStaticIoi();

    void UpdateRtcClock();
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

    UINT32 m_aseInIndex;
    UINT32 m_aseOutIndex;
    UINT32 m_validIoiOut;
    UINT32 m_validIoiIn;
    UINT32 m_writeError;
    UINT32 m_writeErrorZ1;
    UINT32 m_readError;
    UINT32 m_readErrorZ1;

    // Smart Handlers for Static IOIs with behavior
    A664Qar m_a664Qar;
    A717Qar m_a717Qar;
};

#endif
