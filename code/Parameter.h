#ifndef Parameter_h
#define Parameter_h

// File: Parameter.h

#include "alt_stdtypes.h"
#include "AseCommon.h"
#include "ParamConverters.h"

/**************************************************************************************************
* Description: Defines ASE Parameters which are input to the ADRF via IOI and IO processes.
*
*
*/
class Parameter
{
public:
    Parameter();
    void Reset( char* name, PARAM_FMT_ENUM fmt, UINT32 gpa, UINT32 gpb, UINT32 gpc, UINT32 scale);
    void SetConverter( ParamConverter* cvt) {m_converter = cvt;}
    virtual void Update();
    
protected:
    ParameterName m_name;  // the parameter name
    FLOAT32 m_value;       // the current value for the parameter
    UINT32  m_rateHz;      // ADRF update rate for the parameter in Hz
    UINT32  m_updateHz;    // ASE update rate for the parameter in Hz = 2x m_rateHz
    UINT32  m_offset;      // frame offset 0-90 step 10
    
    SigGen m_sigGen;       // the parameter's signal generator
    ParamConverter m_converter; // the parameter's format converter
    
    void A429ParseGps();
};

#endif
