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

class StaticIoiObj
{
public:
    char  ioiName[64];      // IOI binder producer name
    char  m_shortName[64];  // short name for display on console
    INT32 ioiChan;          // deos ioi channel id
    bool ioiValid;          // is the ioi opened
    bool ioiRunning;        // is the ioi being output on a regular basis
    bool ioiIsInput;
    INT32 m_updateCount;    // how many times has the value been read/written

    StaticIoiObj(char* name, bool isInput=false);
    bool OpenIoi();
    void SetRunState(bool newState);
    void SetIO(bool isInput) {ioiIsInput = isInput;}

    // virtual functions
    //virtual IocResponse GetStaticIoiData() {;}
    virtual bool SetStaticIoiData(SecRequest& request) {ioiRunning = true;}

    // streamSize: Byte, Integer
    // streamData: String, IntegerPtr
    virtual bool GetStaticIoiData(IocResponse& m_response) {}

    virtual bool Update() {}
    virtual bool WriteStaticIoi(void* data);
    virtual bool ReadStaticIoi(void* data);
    virtual char* Display(char* dest, UINT32 dix);

    char* CompressName(char* src, int size);
};


class StaticIoiByte : public StaticIoiObj
{
public:
    StaticIoiByte(char* name, unsigned char value, bool isInput=false)
        : StaticIoiObj(name, isInput)
        , data(value)
    {}
    virtual bool SetStaticIoiData(SecRequest& request);
    virtual bool GetStaticIoiData(IocResponse& m_response);
    virtual bool Update();
    virtual char*  Display(char* dest, UINT32 dix);
    unsigned char data;
};

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
};

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
};

class StaticIoiStr : public StaticIoiObj
{
public:
    StaticIoiStr(char* name, char* value, int size, bool isInput=false);
    virtual bool SetStaticIoiData(SecRequest& request);
    virtual bool GetStaticIoiData(IocResponse& m_response);
    virtual bool Update();
    virtual char*  Display(char* dest, UINT32 dix);
    char* data;
    int bytes;
};

class StaticIoiIntPtr : public StaticIoiObj
{
public:
    StaticIoiIntPtr(char* name, int* value, int size, bool isInput=false);
    //virtual bool SetStaticIoiData(SecRequest& request);
    virtual bool GetStaticIoiData(IocResponse& m_response);
    virtual bool Update();
    virtual char*  Display(char* dest, UINT32 dix);
    int* data;
    int bytes;
};

//=============================================================================================
class StaticIoiContainer
{
public:
    StaticIoiContainer();
    void OpenIoi();

    //IocResponse GetStaticIoiData(SecRequest& request);
    bool SetStaticIoiData(SecRequest& request);
    bool GetStaticIoiData(SecComm& secComm);    
    void SetNewState(SecRequest& request);
    void Reset();
    void ResetApatIoi();
    void UpdateStaticIoi();

    StaticIoiObj* m_staticIoiOut[MAX_STATIC_IOI];
    UINT32 m_ioiStaticOutCount;
    StaticIoiObj* m_staticIoiIn[MAX_STATIC_IOI];
    UINT32 m_ioiStaticInCount;
    UINT32 m_updateIndex;
    UINT32 m_validIoiOut;
    UINT32 m_validIoiIn;
    UINT32 m_writeError;
    UINT32 m_readError;
};

#endif
