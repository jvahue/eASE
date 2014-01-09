#ifndef SigGen_h
#define SigGen_h

// File: SigGen.h
#include "Random.h"

// This is not very object oriented (ok not at all) but it makes life simple right now
class SignalGenerator
{
public:

    SignalGenerator();

    float Reset( float lastValue);
    float Update( float oldValue, bool sgRun);

    bool SetParams( int type, int updateMs,
                    float param1, float param2, float param3=0.0f, float param4=0.0f);

    void GetParams( int updateMs,
                    float& p1, float& p2, float& p3, float& p4) const;

    void GetRepresentation(char* buffer) const;
    void GetSgName(char* buffer) const;

    SigGenEnum m_type;
    float m_param1;
    float m_param2;
    float m_param3;
    float m_param4;

    float m_orgParam1;
    float m_orgParam2;
    float m_orgParam3;
    float m_orgParam4;

    int   m_counter;     // used by 1/n shot sig gen
    float m_degrees;     // sine wave
    float m_step0;

    bool  m_firstRun;
    UINT32 m_last;

    RandGen m_random;
};

#endif
