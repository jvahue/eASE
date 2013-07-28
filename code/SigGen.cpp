// SigGen.cpp : implementation file
//

#include "math.h"

#include "SigGen.h"

//--------------------------------------------------------------------------------------------------
SignalGenerator::SignalGenerator()
  : m_type( eSGmanual)
  , m_param1(0.0f)
  , m_param2(0.0f)
  , m_param3(0.0f)
  , m_param4(0.0f)
  , m_counter(-1)
  , m_degrees(0.0f)
{
    // for deterministic results start with the same seed
    srand(0);
}

//--------------------------------------------------------------------------------------------------
// The user just hit reset so reset the SG
float SignalGenerator::Reset( float lastValue)
{
    float newValue;

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
    case eSGpwm:
        newValue = m_param2;  // set to the low value
        break;
    case eSG1Shot:
    case eSGnShot:
        newValue = m_param1;  // set to the low value
        break;
    case eSGrandom:
        // for deterministic results start with the same seed on each reset
        srand(0);
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
#define EqFp(x,y) (fabs(x-y) < 0.00001)

    bool status = true;
    m_type = static_cast<SigGenEnum>(type);

    m_orgParam1 = m_param1 = param1;
    m_orgParam2 = m_param2 = param2;
    m_orgParam3 = m_param3 = param3;
    m_orgParam4 = m_param4 = param4;
    
    // BUG: Validate the parameters i.e., max > min, hi > low etc. 
    //      what about frequencies too high, PWM too small, etc.
    switch ( m_type)
    {
    case eSGmanual:
        //m_orgParam1 = m_param1 = 0.0f;
        m_orgParam2 = m_param2 = 0.0f;
        m_orgParam3 = m_param3 = 0.0f;
        m_orgParam4 = m_param4 = 0.0f;
        break;
    case eSGramp:
    case eSGrampHold:
    case eSGtriangle:
        // stepSize = ((max-min)/(seconds))/(1000/updateMs)
        //m_param3 = ((m_param2 - m_param1)/param3)/(1000.0f/float(updateMs)); 
        m_param3 = ((m_param2 - m_param1)/param3)/1000.0f; 

        if (m_type == eSGtriangle)
        {
            m_step0 = m_param3; 
        }

        m_orgParam4 = m_param4 = 0.0;  // Unused
        break;
    case eSGsine:
        // Freq gets turned into m_angleDegrees/sample
        //m_param1 = (param1 * 360.0f)/(1000/updateMs);
        m_param1 = (param1 * 360.0f)/(1000.0f);
        m_orgParam4 = m_param4 = 0.0;
        if ( m_param1 >= 360.0f)
        {
            status = false;
        }
        break;
    case eSG1Shot:
        m_orgParam4 = m_param4 = 0.0;
        break;
    case eSGnShot:
        break;
    case eSGpwm:
        m_param3 = param3 * (1000.0f/float(updateMs)); // total frames
        m_param4 = m_param3 * param4/100.0f;           // frames high
        if ( m_param3 < 1.0f || m_param4 < 1.0f)
        {
            status = false;
        }
        break;
    case eSGrandom:
        m_orgParam3 = m_param3 = 0.0;
        m_orgParam4 = m_param4 = 0.0;
        break;
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

    switch ( m_type)
    {
    case eSGmanual:
        break;
    case eSGramp:
    case eSGrampHold:
    case eSGtriangle:
        // stepSize = ((max-min)/(seconds))/(1000/updateRate) ... solve for seconds
        //param3 = fabs((float(updateMs)/(m_param3*1000.0f))*(m_param2 - m_param1));
        param3 = fabs(1.0f/(m_param3*1000.0f)*(m_param2 - m_param1));
        break;
    case eSGsine:
        // Freq gets turned into m_angleDegrees
        //param1 = ((m_param1 * 1000.0f)/float(updateMs))/360.0f;
        param1 = ((m_param1 * 1000.0f))/360.0f;
        break;
    case eSG1Shot:
    case eSGnShot:
        break;
    case eSGpwm:
        param3 = (m_param3)/(1000.0f/float(updateMs));
        param4 = 100.0f * (m_param4/m_param3);
        break;
    case eSGrandom:
        break;
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

    clock_t now = clock();

    if ( sgRun && !m_firstRun)
    {
        clock_t delta = now - m_last;

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
            startAt = int(m_param3);
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
            newValue = lowest + ((range * float(rand())) / (float(RAND_MAX) + 1.0f));
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

