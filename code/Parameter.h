#ifndef Parameter_h
#define Parameter_h

// File: Parameter.h

#include "ParamConverters.h"
#include "SigGen.h"
#include "ioiStatic.h"

/**************************************************************************************************
* Description: Defines ASE Parameters which are input to the ADRF via IOI and IO processes.
*
*
*/
#define MAX_FLEX_SEQ1_WORDS 16
#define MAX_FLEX_SEQ1_TBLS  8

typedef UINT32 FlexSeq1Data[MAX_FLEX_SEQ1_WORDS];
typedef struct FlexSeq1TblTag
{
    SINT32 assignIndex;  // parameter index this table is assigned to
    FlexSeq1Data data;   // the seq data for this flex param
} FlexSeq1Tbl;

class Parameter;

class Parameter : public ParamConverter
{
public:
    enum ParamConstants { eParamShort = 23 };
    enum ParamFlex { eFlexNone, eFlexSeq1, eFlexBlock1, eFlexBlock2 };

    Parameter();
    void Reset();
    void Init(ParamCfg* paramInfo, StaticIoiContainer& ioiStatic);
    virtual UINT32 Update(UINT32 sysTick, bool sgRun);
    bool IsChild(Parameter& other);  // determine if this parameter is a 'child'
    char* Display(char* buffer);
    char* ParamInfo(char* buffer, int row);
    char* CompressName(char* src, int size);
    void SetFlex(UINT32 rawValue, int index);

    bool m_ioiValid;    // ADRF update rate for the parameter in Hz
    bool m_isRunning;
    bool m_useUint;     // if the user gives us a UINT32 value and wants to use that do it

    ParameterName m_name;       // the parameter name
    ParameterName m_shortName;  // the short parameter name for the display
    UINT32  m_index;
    UINT32  m_nextIndex;   // the next valid parameter - for processing
    UINT32  m_uintValue;   // in case the user passed down a UINT32 save the value here
    FLOAT32 m_value;       // the current value for the parameter
    UINT32  m_rawValue;    // binary image to send via IOI

    INT32    m_ioiChan;    // deos ioi channel id
    UINT32   m_ioiValue;   // current ioi value after Update
    StaticIoiObj* m_idl;   // if we attached to an IDL this will be non-NULL

    UINT32  m_rateHz;      // ADRF update rate for the parameter in Hz
    UINT32  m_updateMs;    // ASE update rate for the parameter in Hz = 2x m_rateHz
    UINT32  m_updateIntervalTicks; // ASE update rate for the parameter in Hz = 2x m_rateHz
    UINT32  m_offset;      // frame offset 0-90 step 10
    UINT32  m_nextUpdate;  // sys tick for next param update
    UINT32  m_updateCount; // how many times has this param been updated
    UINT16  m_ccdlId;      // the Id used if this param is src=CROSS

    // handle child relationships
    Parameter* m_link;     // link to a child of this param
    bool   m_isChild;      // indicates this is a child (no IOI required)
    UINT32 m_childCount;

    // handle flex roots
    ParamFlex  m_flexType;        // for flex roots, what type is this
    SINT32     m_flexDataTbl;     // Which seq table is this root associated with
    // flex children
    SINT32     m_flexParentIndex; // for sequential flex the index
    SINT32     m_flexSeq;         // param seq index/root TX index
    Parameter* m_flexParent;      // for flex children and pointer to their parent

    SignalGenerator m_sigGen;       // the parameter's signal generator
    UINT32 m_updateDuration;
};

#endif
