// SigGen.cpp : implementation file
//
/*****************************************************************************/
/* Compiler Specific Includes                                                */
/*****************************************************************************/
#include <deos.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define fabs __builtin_fabs


/*****************************************************************************/
/* Software Specific Includes                                                */
/*****************************************************************************/
#include "AseCommon.h"

#include "SigGen.h"

/*****************************************************************************/
/* Local Defines                                                             */
/*****************************************************************************/
#define TimeBase 1000.0f  // one ms frames in a sec
#define EqFp(x,y) (fabs(x-y) < 0.00001)

/*****************************************************************************/
/* Local Data                                                                */
/*****************************************************************************/
// Signal Generator Options - make them the same length so the display is nice
static char* modeNames[eMaxSensorMode] = {
    "Manual  ",
    "Ramp    ",
    "RampHold",
    "Triangle",
    "Sine    ",
    "1-Shot  ",
    "N-Shot  ",
    "PWM     ",
    "PWM1    ",
    "Random  "
};

/*****************************************************************************/
/* Public Functions                                                          */
/*****************************************************************************/

//--------------------------------------------------------------------------------------------------
SignalGenerator::SignalGenerator()
: m_type( eSGmanual)
, m_param1(0.0f)
, m_param2(0.0f)
, m_param3(0.0f)
, m_param4(0.0f)
, m_orgParam1(0.0f)
, m_orgParam2(0.0f)
, m_orgParam3(0.0f)
, m_orgParam4(0.0f)
, m_counter(-1)
, m_degrees(0.0f)
, m_step0(0.0f)
, m_firstRun(true)
, m_last(0)
{
    // for deterministic results start with the same seed
    m_random.Seed(0);
}

//--------------------------------------------------------------------------------------------------
// The user just hit reset so reset the SG
float SignalGenerator::Reset( float lastValue)
{
    float newValue = lastValue;

    m_counter = 0;
    m_degrees = 0.0f;

    switch ( m_type)
    {
    case eSGmanual:
        newValue = lastValue;
        break;
    case eSGramp:
    case eSGrampHold:
        newValue = m_param1;  // set to the initial value
        break;
    case eSGtriangle:
        newValue = m_param1;  // set to the initial value
        m_param3 = m_step0;
        break;
    case eSGsine:
        newValue = m_param3;  // set to the bias value
        break;
    case eSGpwm1:
        m_counter = (int)m_param3; // force PWM toggling right away
        // Fall-through
    case eSGpwm:
        newValue = m_param2;  // set to the initial value
        break;
    case eSG1Shot:
    case eSGnShot:
        newValue = m_param1;  // set to the low value
        break;
    case eSGrandom:
        // for deterministic results start with the same seed on each reset
        m_random.Seed(0);
        newValue = m_param1;  // set to the low value
        break;
    }

    return newValue;
}

//--------------------------------------------------------------------------------------------------
// The user just hit set so initialize or update the SG params
bool SignalGenerator::SetParams( int type, int updateMs,
                                float param1, float param2, float param3, float param4)
{
    bool status = true;
    m_type = static_cast<SigGenEnum>(type);

    m_orgParam1 = param1;
    m_orgParam2 = param2;
    m_orgParam3 = param3;
    m_orgParam4 = param4;

    m_param1 = param1;
    m_param2 = param2;
    m_param3 = param3;
    m_param4 = param4;

    // Validate the parameters i.e., max > min, hi > low etc.
    // what about frequencies too high, PWM too small, etc.
    if (m_type == eSGmanual)
    {
        //m_orgParam1 = m_param1 = 0.0f;
        m_orgParam2 = 0.0f;
        m_orgParam3 = 0.0f;
        m_orgParam4 = 0.0f;

        m_param2 = 0.0f;
        m_param3 = 0.0f;
        m_param4 = 0.0f;
    }
    else if (m_type == eSGramp || m_type == eSGrampHold || m_type == eSGtriangle)
    {
        // stepSize = ((max-min)/(seconds))/(1000/updateMs)
        //m_param3 = ((m_param2 - m_param1)/param3)/(1000.0f/float(updateMs));
        m_param3 = ((m_param2 - m_param1)/param3)/TimeBase;

        if (m_type == eSGtriangle)
        {
            m_step0 = m_param3;
        }

        m_orgParam4 = 0.0;  // Unused
        m_param4 = 0.0;
    }
    else if (m_type == eSGsine)
    {
        // Freq gets turned into m_angleDegrees/sample
        //m_param1 = (param1 * 360.0f)/(1000/updateMs);
        m_param1 = (param1 * 360.0f)/TimeBase;
        m_orgParam4 = 0.0;
        m_param4 = 0.0;
        if ( m_param1 >= 360.0f)
        {
            status = false;
        }
    }
    else if (m_type == eSG1Shot)
    {
        m_orgParam4 = 0.0;
        m_param4 = 0.0;
    }
    else if (m_type == eSGnShot)
    {
        // do nothing
    }
    else if ((m_type == eSGpwm) || (m_type == eSGpwm1))
    {
        m_param3 = param3 * (TimeBase/float(updateMs)); // total frames
        m_param4 = m_param3 * param4/100.0f;            // frames high
        if ( m_param3 < 1.0f || m_param4 < 1.0f)
        {
            status = false;
        }

        if (m_type == eSGpwm1)
        {
            m_counter = (int)m_param3;
        }
    }
    else if (m_type == eSGrandom)
    {
        m_orgParam3 = 0.0;
        m_orgParam4 = 0.0;

        m_param3 = 0.0;
        m_param4 = 0.0;
    }
    else
    {
        status = false;
    }

    // check if params are valid and we can generate the requested signal
    GetParams(updateMs, param1, param2, param3, param4);
    if ( !status ||
        !EqFp( param1, m_orgParam1) || !EqFp( param2, m_orgParam2) ||
        !EqFp( param3, m_orgParam3) || !EqFp( param4, m_orgParam4))
    {
        m_type = eSGmanual;
        return false;
    }
    else
    {
        return true;
    }
}

//--------------------------------------------------------------------------------------------------
// When the user selects a currently configured SG this function returns the parameters to the
// display
void SignalGenerator::GetParams( int updateMs,
                                float& param1, float& param2, float& param3, float& param4) const
{
    // BUG: the could be set to m_orgParamX but we reverse compute these until we get the error
    //     checking in SetParams
    param1 = m_param1;
    param2 = m_param2;
    param3 = m_param3;
    param4 = m_param4;

    if( m_type == eSGmanual || m_type == eSG1Shot || 
        m_type == eSGnShot  || m_type == eSGrandom)
    {
        // do nothing
    }
    else if (m_type == eSGramp || m_type == eSGrampHold || m_type == eSGtriangle)
    {
        // stepSize = ((max-min)/(seconds))/(1000/updateRate) ... solve for seconds
        //param3 = fabs((float(updateMs)/(m_param3*1000.0f))*(m_param2 - m_param1));
        param3 = fabs(1.0f / (m_param3 * TimeBase) * (m_param2 - m_param1));
    }
    else if (m_type == eSGsine)
    {
        // Freq gets turned into m_angleDegrees
        //param1 = ((m_param1 * 1000.0f)/float(updateMs))/360.0f;
        param1 = ((m_param1 * TimeBase))/360.0f;
    }
    else if ((m_type == eSGpwm) || (m_type == eSGpwm1))
    {
        param3 = (m_param3)/(TimeBase/float(updateMs));
        param4 = 100.0f * (m_param4/m_param3);
    }
}

//--------------------------------------------------------------------------------------------------
// Update the SG
float SignalGenerator::Update( float oldValue, bool sgRun)
{
    int frames;
    int startAt;
    double radians;
    double sineValue;
    float lowest;
    float range;

    float newValue = oldValue;

    UNSIGNED32 *systemTickPtr = systemTickPointer();
    UINT32 now = *systemTickPtr * 10;

    if ( sgRun && !m_firstRun)
    {
        UINT32 delta = now - m_last;

        switch (m_type)
        {
        case eSGmanual:
            newValue = oldValue;
            break;

        case eSGramp:
            //newValue = oldValue + m_param3;
            newValue = oldValue + (m_param3 * delta);
            if (m_param3 > 0.0f && newValue > m_param2)
            {
                newValue = m_param1;
            }
            else if (m_param3 < 0.0f && newValue < m_param2)
            {
                newValue = m_param1;
            }
            break;

        case eSGrampHold:
            //newValue = oldValue + m_param3;
            newValue = oldValue + (m_param3 * delta);
            if (m_param3 > 0.0f && newValue > m_param2)
            {
                newValue = m_param2;
            }
            else if (m_param3 < 0.0f && newValue < m_param2)
            {
                newValue = m_param2;
            }
            break;

        case eSGtriangle:
            //newValue = oldValue + m_param3;
            newValue = oldValue + (m_param3 * delta);
            if (newValue > max( m_param1, m_param2))
            {
                newValue = max( m_param1, m_param2);
                m_param3 *= -1.0f;
            }
            else if (newValue < min( m_param1, m_param2))
            {
                newValue = min( m_param1, m_param2);
                m_param3 *= -1.0f;
            }
            break;

        case eSGsine:
            radians = m_degrees * (3.1415926535897932384626433832795f / 180.0f);
            sineValue = sin( radians);
            newValue = (float(sineValue) * m_param2) + m_param3;
            //m_degrees += m_param1;
            m_degrees += m_param1 * delta;
            break;

        case eSG1Shot:
            //startAt = int(m_param3);
            if ( m_counter == int(m_param3))
            {
                newValue = m_param2;
            }
            else
            {
                newValue = m_param1;
            }
            ++m_counter;
            break;

        case eSGnShot:
            startAt = int(m_param3);
            frames  = int(m_param4);
            if ( m_counter >= startAt && m_counter < (startAt + frames))
            {
                newValue = m_param2;
            }
            else
            {
                newValue = m_param1;
            }
            ++m_counter;
            break;

        case eSGpwm:
        case eSGpwm1:
            ++m_counter;
            if ( m_counter >= int(m_param4) && m_counter < int(m_param3))
            {
                newValue = m_param2;
            }
            else if ( m_counter >= int(m_param3))
            {
                newValue = m_param1;
                m_counter = 0;
            }
            break;

        case eSGrandom:
            lowest = m_param1;
            range  = m_param2 - lowest;
            if (oldValue > m_param2)
            {
                newValue -= 0.5;
            }
            else
            {
                newValue += 0.5f;
            }
            newValue = lowest + ((range * float(m_random.Rand())) / (float(RAND_MAX) + 1.0f));
            break;

        default:
            newValue = oldValue;
            break;
        };
    }
    else
    {
        newValue = oldValue;
        m_firstRun = !sgRun;
    }

    m_last = now;

    return newValue;
}

void SignalGenerator::GetRepresentation(char* buffer) const
{
    switch (m_type)
    {
    case eSGmanual:
        sprintf(buffer, "manual(%.2f)", m_orgParam1);
        break;
    case eSGramp:
        sprintf(buffer, "ramp(%.2f, %.2f, %.2f)", m_orgParam1, m_orgParam2, m_orgParam3);
        break;
    case eSGrampHold:
        sprintf(buffer, "rampHold(%.2f, %.2f, %.2f)", m_orgParam1, m_orgParam2, m_orgParam3);
        break;
    case eSGtriangle:
        sprintf(buffer, "triangle(%.2f, %.2f, %.2f)", m_orgParam1, m_orgParam2, m_orgParam3);
        break;
    case eSGsine:
        sprintf(buffer, "sine(%.2f, %.2f, %.2f)", m_orgParam1, m_orgParam2, m_orgParam3);
        break;
    case eSG1Shot:
        sprintf(buffer, "oneshot(%.2f, %.2f, %.2f)", m_orgParam1, m_orgParam2, m_orgParam3);
        break;
    case eSGnShot:
        sprintf(buffer, "nshot(%.2f, %.2f, %.2f, %.2f)",
            m_orgParam1, m_orgParam2, m_orgParam3, m_orgParam4);
        break;
    case eSGpwm:
    case eSGpwm1:
        sprintf(buffer, "pwm(%.2f, %.2f, %.2f, %.2f)",
            m_orgParam1, m_orgParam2, m_orgParam3, m_orgParam4);
        break;
    case eSGrandom:
        sprintf(buffer, "random(%.2f, %.2f)", m_orgParam1, m_orgParam2);
        break;
    default:
        sprintf(buffer, "Unknown(%d)", m_type);
        break;

    }
}

void SignalGenerator::GetSgName(char* buffer) const
{
    strcpy( buffer, modeNames[m_type]);
}

