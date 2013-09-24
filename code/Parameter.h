#ifndef Parameter_h
#define Parameter_h

// File: Parameter.h

#include "alt_stdtypes.h"

#include "AseCommon.h"
#include "ParamConverters.h"
#include "SigGen.h"

/**************************************************************************************************
* Description: Defines ASE Parameters which are input to the ADRF via IOI and IO processes.
*
*
*/
class Parameter : public ParamConverter
{
public:
enum ParamConstants {eParamShort = 23};

    Parameter();
    void Reset();
    void Init(ParamCfg* paramInfo);
    virtual UINT32 Update(UINT32 sysTick, bool sgRun);
    bool IsChild(Parameter& other);  // indicates if this parameter is a 'child' match for the other parameter
    char* Display(char* buffer);
    char* ParamInfo(char* buffer, int row);
    char* CompressName(char* src, int size);
    bool IsRunning() {return m_isRunning;}

    bool m_ioiValid;    // ADRF update rate for the parameter in Hz
    bool m_isRunning;
    ParameterName m_name;       // the parameter name
    ParameterName m_shortName;  // the short parameter name for the display
    UINT32  m_index;
    FLOAT32 m_value;       // the current value for the parameter
    UINT32  m_rawValue;    // binary image to send via IOI

    INT32    m_ioiChan;     // deos ioi channel id
    UINT32   m_ioiValue;    // current ioi value after Update

    UINT32  m_rateHz;        // ADRF update rate for the parameter in Hz
    UINT32  m_updateMs;      // ASE update rate for the parameter in Hz = 2x m_rateHz
    UINT32  m_updateIntervalTicks; // ASE update rate for the parameter in Hz = 2x m_rateHz
    UINT32  m_offset;        // frame offset 0-90 step 10
    UINT32  m_nextUpdate;    // sys tick for next param update
    UINT32  m_updateCount;   // how many times has this param been updated

    // handle child relationships
    Parameter* m_link;       // link to a child of this param
    bool   m_isChild;        // indicates this is a child (no IOI required)

    SignalGenerator m_sigGen;       // the parameter's signal generator
    UINT32 m_updateDuration;
    UINT32 m_childCount;
};

#endif
