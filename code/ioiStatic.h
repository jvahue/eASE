#ifndef IOISTATIC_H
#define IOISTATIC_H

#include <ioiapi.h>

#include "SecComm.h"

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
    char  m_shortName[64];  // IOI binder producer name
    INT32 ioiChan;          // deos ioi channel id
    bool ioiValid;          // is the ioi opened
    IoiStaticTypes type;    // what type of IOI is it
    UINT32 stringSize;      // for strings holds the string size

    StaticIoiObj(char* name);
    bool OpenIoi();

    // virtual functions
    //virtual IocResponse GetStaticIoiData() {;}
    virtual bool SetStaticIoiData(SecRequest& request) {}
    virtual bool Update() {}
    virtual bool WriteStaticIoi(void* data);
    virtual char* Display(char* dest, UINT32 dix);

    char* CompressName(char* src, int size);
};


class StaticIoiByte : public StaticIoiObj
{
public:
    StaticIoiByte(char* name, unsigned char value) 
        : StaticIoiObj(name)
        , data(value)
    {}
    //virtual IocResponse GetStaticIoiData();
    virtual bool SetStaticIoiData(SecRequest& request);
    virtual bool Update() {return WriteStaticIoi(&data);}
    virtual char*  Display(char* dest, UINT32 dix);
    unsigned char data;
};

class StaticIoiInt : public StaticIoiObj
{
public:
    StaticIoiInt(char* name, int value) 
        : StaticIoiObj(name) 
        , data(value)
    {}
    //virtual IocResponse GetStaticIoiData();
    virtual bool SetStaticIoiData(SecRequest& request);
    virtual bool Update() {return WriteStaticIoi(&data);}
    virtual char*  Display(char* dest, UINT32 dix);
    int data;
};

class StaticIoiFloat : public StaticIoiObj
{
public:
    StaticIoiFloat(char* name, float value) 
        : StaticIoiObj(name)
        , data(value)
    {}
    //virtual IocResponse GetStaticIoiData();
    virtual bool SetStaticIoiData(SecRequest& request);
    virtual bool Update() {return WriteStaticIoi(&data);}
    virtual char*  Display(char* dest, UINT32 dix);
    float data;
};

class StaticIoiStr : public StaticIoiObj
{
public:
    StaticIoiStr(char* name, char* value) 
        : StaticIoiObj(name)
        ,data(value)
    {}
    //virtual IocResponse GetStaticIoiData();
    virtual bool SetStaticIoiData(SecRequest& request);
    virtual bool Update() {return WriteStaticIoi(data);}
    virtual char*  Display(char* dest, UINT32 dix);
    char* data;
};

//================================================================================================
class StaticIoiContainer
{
public:
    StaticIoiContainer();
    void OpenIoi();

    //IocResponse GetStaticIoiData(SecRequest& request);
    bool SetStaticIoiData(SecRequest& request);
    void UpdateStaticIoi();

    StaticIoiObj* m_staticIoi[100];
    UINT32 m_ioiStaticCount;
    UINT32 m_updateIndex;
    UINT32 m_validIoi;
    UINT32 m_writeError;
};

#endif
