//-----------------------------------------------------------------------------
//          Copyright (C) 2014-2016 Knowlogic Software Corp.
//         All Rights Reserved. Proprietary and Confidential.
//
//    File: ioiStatic.cpp
//
//    Description: Implements the Static IOI processing
//
//-----------------------------------------------------------------------------


//----------------------------------------------------------------------------/
// Compiler Specific Includes                                                -/
//----------------------------------------------------------------------------/
#include <stdio.h>
#include <string.h>

//----------------------------------------------------------------------------/
// Software Specific Includes                                                -/
//----------------------------------------------------------------------------/
#include "AseCommon.h"

#include "ioiStatic.h"

//----------------------------------------------------------------------------/
// Local Defines                                                             -/
//----------------------------------------------------------------------------/
#define TEN(x) ( ((x >> 4) & 0xF) * 10)
#define ONE(x) (x & 0xF)
#define VALUE(x) TEN(x) + ONE(x)

//----------------------------------------------------------------------------/
// Local Typedefs                                                            -/
//----------------------------------------------------------------------------/

//----------------------------------------------------------------------------/
// Local Variables                                                           -/
//----------------------------------------------------------------------------/

//----------------------------------------------------------------------------/
// Constant Data                                                             -/
//----------------------------------------------------------------------------/
#include "AseStaticIoiInfo.h"

#if MAX_STATIC_IOI <= ASE_IN_MAX
#error Need to Increase MAX_STATIC_IOI to be greater than ASE_IN_MAX
#endif

#if MAX_STATIC_IOI <= ASE_OUT_MAX
#error Need to Increase MAX_STATIC_IOI to be greater than ASE_OUT_MAX
#endif

UINT32 mirrorQarA664[A664Qar::eSfCount][A664Qar::eSfWordCount];

//----------------------------------------------------------------------------/
// Local Function Prototypes                                                 -/
//----------------------------------------------------------------------------/

//----------------------------------------------------------------------------/
// Public Functions                                                          -/
//----------------------------------------------------------------------------/

//----------------------------------------------------------------------------/
// Class Definitions                                                         -/
//----------------------------------------------------------------------------/
StaticIoiObj::StaticIoiObj(char* name, bool isInput)
: ioiChan(0)
, ioiValid(false)
, ioiRunning(true)
, ioiIsInput(isInput)
, m_updateCount(0)
{
    strcpy(ioiName, name);
    strcpy(m_shortName, name);
    CompressName(m_shortName, 18);
}

//---------------------------------------------------------------------------------------------
bool StaticIoiObj::OpenIoi()
{
    ioiStatus openStatus;

    if (ioiIsInput)
    {
        openStatus = ioi_open(ioiName, ioiReadPermission, (int*)&ioiChan);
    }
    else
    {
        openStatus = ioi_open(ioiName, ioiWritePermission, (int*)&ioiChan);
    }
    ioiValid = openStatus == ioiSuccess;
    ioiRunning = ioiValid;
    return ioiValid;
}

//---------------------------------------------------------------------------------------------
// Return the status of an actual IOI write.  For params we skip just return success
bool StaticIoiObj::WriteStaticIoi(void* data)
{
    ioiStatus writeStatus = ioiSuccess;

    if (ioiValid && ioiRunning)
    {
        writeStatus = ioi_write(ioiChan, data);
    }

    m_updateCount += (ioiValid && ioiRunning && writeStatus == ioiSuccess) ? 1 : 0;
    return writeStatus == ioiSuccess;
}

//---------------------------------------------------------------------------------------------
// Return the status of an actual IOI write.  For params we skip just return success
bool StaticIoiObj::ReadStaticIoi(void* data)
{
    ioiStatus readStatus = ioiSuccess;

    if (ioiValid && ioiRunning)
    {
        readStatus = ioi_read(ioiChan, data);
    }

    m_updateCount += (ioiValid && ioiRunning && readStatus == ioiSuccess) ? 1 : 0;
    return readStatus == ioiSuccess;
}

//---------------------------------------------------------------------------------------------
char* StaticIoiObj::Display( char* dest, UINT32 dix )
{
    dest = '\0';
    return dest;
}

//---------------------------------------------------------------------------------------------
char* StaticIoiObj::CompressName(char* src, int size)
{
    char* vowel;
    char* from;
    char* to;
    int at = strlen(src) - 1;

    // remove vowels from the back until less than 24 chars long
    while (strlen(src) > size && at >= 0)
    {
        vowel = strpbrk(&src[at], "aeiouAEIOU");
        if (vowel != NULL)
        {
            to = vowel;
            from = ++vowel;
            while (*from != NULL && from != &src[at])
            {
                *to++ = *from++;
            }
            *to = '\0';
        }
        at -= 1;
    }

    if (strlen(src) > size)
    {
        src[size] = '\0';
    }

    return src;
}

//---------------------------------------------------------------------------------------------
void StaticIoiObj::SetRunState(bool newState)
{
    ioiRunning = newState;
}

//=============================================================================================
//IocResponse StaticIoiByte::GetStaticIoiData()
//{
//
//}

//---------------------------------------------------------------------------------------------
bool StaticIoiByte::SetStaticIoiData( SecRequest& request )
{
    data = (unsigned char)request.resetRequest;
    Update();
    return true;
}

//---------------------------------------------------------------------------------------------
char* StaticIoiByte::Display( char* dest, UINT32 dix )
{
    if (ioiRunning)
    {
        sprintf(dest, "%2d:%s: 0x%02x", dix, m_shortName, data);
    }
    else
    {
        sprintf(dest, "xx:%s: 0x%02x", dix, m_shortName, data);
    }
    return dest;
}

//---------------------------------------------------------------------------------------------
bool StaticIoiByte::GetStaticIoiData(IocResponse& m_response)
{
    m_response.streamSize = data;
    return true;
}

bool StaticIoiByte::Update()
{
    if (ioiIsInput)
    {
        return ReadStaticIoi(&data);
    }
    else
    {
        return WriteStaticIoi(&data);
    }
}

//=============================================================================================
//IocResponse StaticIoiInt::GetStaticIoiData()
//{
//
//}

//---------------------------------------------------------------------------------------------
bool StaticIoiInt::SetStaticIoiData( SecRequest& request )
{
    data = request.resetRequest;
    Update();
    return true;
}

//---------------------------------------------------------------------------------------------
char* StaticIoiInt::Display( char* dest, UINT32 dix )
{
    if ( ioiRunning)
    {
        sprintf(dest, "%2d:%s: 0x%08x", dix, m_shortName, data);
    }
    else
    {
        sprintf(dest, "xx:%s: 0x%08x", m_shortName, data);

    }
    return dest;
}

//---------------------------------------------------------------------------------------------
bool StaticIoiInt::Update()
{
    if (ioiIsInput)
    {
        return ReadStaticIoi(&data);
    }
    else
    {
        return WriteStaticIoi(&data);
    }
}

//---------------------------------------------------------------------------------------------
bool StaticIoiInt::GetStaticIoiData(IocResponse& m_response)
{
    m_response.streamSize = data;
    return true;
}

//=============================================================================================
//IocResponse StaticIoiFloat::GetStaticIoiData()
//{
//
//}

//---------------------------------------------------------------------------------------------
bool StaticIoiFloat::SetStaticIoiData( SecRequest& request )
{
    data = request.value;
    Update();
    return true;
}

//---------------------------------------------------------------------------------------------
char* StaticIoiFloat::Display( char* dest, UINT32 dix )
{
    if ( ioiRunning)
    {
        sprintf(dest, "%2d:%s: %f", dix, m_shortName, data);
    }
    else
    {
        sprintf(dest, "xx:%s: %f", m_shortName, data);
    }
    return dest;
}

//---------------------------------------------------------------------------------------------
bool StaticIoiFloat::Update()
{
    if (ioiIsInput)
    {
        return ReadStaticIoi(&data);
    }
    else
    {
        return WriteStaticIoi(&data);
    }
}

//---------------------------------------------------------------------------------------------
bool StaticIoiFloat::GetStaticIoiData(IocResponse& m_response)
{
    m_response.value = data;
    return true;
}

//=============================================================================================
//IocResponse StaticIoiStr::GetStaticIoiData()
//{
//
//}

StaticIoiStr::StaticIoiStr(char* name, char* value, int size, bool isInput/*=false*/) 
: StaticIoiObj(name, isInput)
, displayAt(0)
, data(value)
, bytes(size)
{
    memset(data, 0, bytes);
}

//---------------------------------------------------------------------------------------------
bool StaticIoiStr::SetStaticIoiData( SecRequest& request )
{
    UINT32 offset = request.clearCfgRequest;

    if (offset == 0)
    {
        memset(data, 0, bytes);
    }

    memcpy(&data[offset], request.charData, request.charDataSize);
    Update();
    return true;
}

//---------------------------------------------------------------------------------------------
char* StaticIoiStr::Display( char* dest, UINT32 dix )
{
    unsigned int* dp = (unsigned int*)&data[displayAt];
    if (ioiRunning)
    {
        sprintf(dest, "%2d:%s: 0x%08x", dix, m_shortName, *dp);
    }
    else
    {
        sprintf(dest, "xx:%s: 0x%08x", m_shortName, *dp);
    }

    return dest;
}

//---------------------------------------------------------------------------------------------
bool StaticIoiStr::Update()
{
    if (ioiIsInput)
    {
        return ReadStaticIoi(data);
    }
    else
    {
        return WriteStaticIoi(data);
    }
}

//---------------------------------------------------------------------------------------------
// To handle data larger than the response streamData we pass in the streamSize variable the
// start offset given to us by PySte for eGetStaticIoi requests
bool StaticIoiStr::GetStaticIoiData( IocResponse& m_response)
{
    UINT32 offset = m_response.streamSize;
    UINT32 left = bytes - offset;

    memcpy(m_response.streamData, &data[offset], left);
    m_response.streamSize = left;
    return true;
}

//---------------------------------------------------------------------------------------------
bool StaticIoiIntPtr::Update()
{
    if (ioiIsInput)
    {
        return ReadStaticIoi(data);
    }
    else
    {
        return WriteStaticIoi(data);
    }
}

//---------------------------------------------------------------------------------------------
char* StaticIoiIntPtr::Display( char* dest, UINT32 dix )
{
    sprintf(dest, "%2d:?? Integers", dix);
    return dest;
}

//---------------------------------------------------------------------------------------------
bool StaticIoiIntPtr::GetStaticIoiData(IocResponse& m_response)
{
    memcpy(m_response.streamData, data, bytes);
    m_response.streamSize = bytes;
    return true;
}

StaticIoiIntPtr::StaticIoiIntPtr( char* name, int* value, int size, bool isInput)
: StaticIoiObj(name, isInput)
{
    data = value;
    bytes = size * sizeof(int);
}


//=============================================================================================
A664Qar::A664Qar(StaticIoiStr* buffer)
{
    m_ioiBuffer = buffer;  // the buffer associated with the IOI

    Reset();
}

void A664Qar::Reset()
{
    // which sub-frame are we outputting
    m_sf = 0;          // start sending from SF1
    m_skipSfMask = 0;  // don't skip any
    m_sfWordIndex = 0; // start sending from word index 0
    m_random = 0;      // default 0 random words in a burst

    // which burst of the sub-frame are we sending
    m_burst = 0;       // start with the first burst group
    m_burstWord = 0;   // which word we are on in the burst
    for (int i=0; i < eBurstCount-4; ++i)
    {
        m_burstSize[i] = 51;
    }
    m_burstSize[16] = 52;
    m_burstSize[17] = 52;
    m_burstSize[18] = 52;
    m_burstSize[19] = 52;

    // ensure these are not zero as that terminates processing in the UUT
    m_ndo[0] = 0x97560801;
    m_ndo[1] = 0x97561002;
    m_ndo[2] = 0x97561803;
    m_ndo[3] = 0x97562004;
    m_nonNdo = (m_ndo[0] | m_ndo[1] | m_ndo[2] | m_ndo[3]) + 1;
    m_frameCount = 0;

    m_wordSeqEnabled = 0;
    memset(m_wordSeq, 0, sizeof(m_wordSeq));
    m_repeatCount = -1;
    m_repeatIndex = 0;

    // zero out all of the QAR data values all 4096 of them
    memset(m_qarWords, 0, sizeof(m_qarWords));
}

//---------------------------------------------------------------------------------------------
// This function allows for setting data values in the QAR data stream, additionally it can
// be used to set the NDO SF identifier values
// variableId = index for _a664_to_ioc_eicas_
// sigGenId = set operation mode
//   1: Set Data - we are in this mode to be here
//   2: disable IOI output - handled in IoiProcess.CheckCmd level for eSetStaticIoi
// charData: holds the data to be written UINT8 or UINT32 (NDO)
// charSize: holds the number of bytes to move
// clearCfgRequest: the byte offset into m_qarWords
bool A664Qar::TestControl(SecRequest& request)
{
    bool status = true;
    SINT32 offset = (SINT32)request.clearCfgRequest;
    UINT32 details = request.resetRequest;

    if (offset == eQarNdo)
    {
        m_nonNdo = 0;  // init recompute the nonNdo Id

        // setting up the NDO values
        UINT32* data = (UINT32*)request.charData;
        // set the SF Word Count and NDO ID values
        for (int i = 0; i < eSfCount; ++i)
        {
            m_ndo[i] = *data++;
            m_nonNdo |= m_ndo[i];
        }

        m_nonNdo += 1;  // after ORing these all together + 1 to make it a non-NDO
    }
    else if (offset == eQarSfSeq)
    {
        int* data = (int*)request.charData;
        m_skipSfMask = *data;
    }
    else if (offset == eQarWordSeq)
    {
        // packed data is word index (corrected for SF) and cmd value
        UINT16* index = (UINT16*)request.charData;
        UINT16* cmdWord = (UINT16*)request.charData;
        UINT16* dest = (UINT16*)&m_wordSeq[0][0];

        cmdWord += 1;

        while (*index != 0 && cmdWord != 0)
        {
            *(dest + *index) = *cmdWord;
            index += 2;
            cmdWord += 2;
        }

        // clear the QAR-A664 data mirror
        memset(mirrorQarA664, 0, sizeof(mirrorQarA664));
    }
    else if (offset == eQarWordSeqState)
    {
        UINT16* dest = &m_wordSeqEnabled;
        // here we are moving data into our SF data array
        *dest = *(UINT16*)request.charData;
        if (m_wordSeqEnabled == 2)
        {
            // reset the WSB
            memset(m_wordSeq, 0, sizeof(m_wordSeq));
            m_wordSeqEnabled = 0;
        }

        if (m_wordSeqEnabled == 1)
        {
            // disable random data insets
            m_randomSave = m_random;
            m_random = 0;
        }
        else
        {
            m_random = m_randomSave;
        }
    }
    else if (offset == eQarRandom)
    {
        int* dest = &m_random;
        *dest = *(int*)request.charData;
    }
    else
    {
        // here we are moving data into our SF data array
        UINT8* dest = (UINT8*)&m_qarWords[0][0] + offset;
        memcpy(dest, request.charData, request.charDataSize);
    }

    return status;
}

//---------------------------------------------------------------------------------------------
// This function fills in a bursts - the most data this will fill in is:
// 52 QAR words in a burst + 60 random words or (52 + 60) * 8 = 896 of 1024 byte buffer
// It provides the following capabilities for testing:
// A. Fill in randomly placed non-QAR NDO and data, up to 50 extra intra QAR data + 10 post QAR
// Error Injection
// 1. Skip a SF
// 2. Skip words in a burst
// 3. ???
void A664Qar::Update()
{
    UINT32 lastSf;
    UINT32 lastSfWord;
    UINT32 wordValue;
    // Fill in the IOI buffer with content from the sf/burst going out
    UINT32 totalInsert = 0;   // number of "words"  insert NDO/DATA
    UINT32 randomInsert = 0;  // number of randomly insert NDO/DATA
    UINT32* fillPtr = (UINT32*)m_ioiBuffer->data;    // where the data is going

    *(fillPtr++) = m_frameCount++;

    m_endBurst = false;

    // Fill in the data for the sf/burst 
    // no need for the memset below ***see termination w/0 at end of func
    // memset(m_ioiBuffer->data, 0, m_ioiBuffer->bytes); 
    while (totalInsert < eMaxBurstWords && !m_endBurst)
    {
        // check random data insert
        if (randomInsert < m_random && (HsTimer() & 1))
        {
            // insert random data
            *(fillPtr++) = (m_nonNdo + randomInsert);
            *(fillPtr++) = randomInsert << 16;  // insert in MSW
            randomInsert += 1;
        }
        else
        {
            // if we are not skipping every SF
            if ( m_skipSfMask < 0xF)
            {
                // insert burst data
                *(fillPtr++) = m_ndo[m_sf];

                // save these because NextWord will update them
                lastSf = m_sf;
                lastSfWord = m_sfWordIndex;

                wordValue = NextWord();
                *(fillPtr++) = wordValue;

                mirrorQarA664[lastSf][lastSfWord] = wordValue;
            }
        }
        totalInsert += 1;
    }

    // ensure we cannot insert more than eMaxBurstWords
    randomInsert = eMaxBurstWords - totalInsert;

    //-----------------------------------------------------------
    // fill in a few more random based on how many we have done
    if (randomInsert < m_random)
    {
        // fill in until we have 50
        while (randomInsert < m_random)
        {
            *(fillPtr++) = (m_nonNdo + randomInsert);
            *(fillPtr++) = randomInsert << 16;  // insert in MSW
            randomInsert += 1;
        }
    }

    // terminate the data set (max 896 bytes + 4 here = 900 bytes of 1024) 
    *(fillPtr++) = 0;    
}

/*---------------------------------------------------------------------------------------------
Compute the next word from the current SF to send.  This function implements the word 
sequence command buffer that can be used to specify ways to screw up a standard data stream
of QAR data.The word sequence buffer (WSB) is either enabled or disabled.  When disabled data 
proceeds from word to word in each SF.  When the WSB is enabled it implements the following 
commands on a word by word basis:

  0. eWsbNop increment to next natural word number
     only used to clear an existing entry on a word, otherwise don't use
  1. eWsbGoto: skip N words and resume
  2. eWsbRepeat: repeat a given word N times,
                 repeat 1 is normal operation so repeat 2 causes it to repeat
  3. eWsbSf: Send in a bad SF identifier for this word
  4. eWsbWc: Send in a bad WC for this word

The WSB consists of 4 x 1024 word that hold opcodes and data.  Opcode is in the least
significant 3 bits allowing for 8 opcodes, of which we have used 3, the upper 13 bits are
used for the operand. The word sequence buff is initialized to NOP for all words. To set it use 
the provided functions
---------------------------------------------------------------------------------------------*/
UINT32 A664Qar::NextWord()
{
    int opcode;
    int operand;
    int nextWord;
    int wordStep;
    int sfMove;
    UINT32 wordValue;

    int sfId = -1;
    int wordId = -1;

    // pre-fill this based on the m_sfWordIndex/sf before we change them
    wordValue = (m_sfWordIndex << 20) |                  // word index 0 .. 1023
                (m_qarWords[m_sf][m_sfWordIndex] << 8) | // word value
                (m_sf + 1);                              // SF 1 .. 4

    if (m_wordSeqEnabled)
    {
        // have we just finished repeatedly sending the same word, then move forward how times
        // it was repeated
        if (m_repeatCount == 0)
        {
            m_repeatCount = -1;
            m_sfWordIndex += (m_wordSeq[m_sf][m_sfWordIndex] >> 3) - 1;
        }

        // find out what we should be doing at this word position
        opcode = m_wordSeq[m_sf][m_sfWordIndex] & 0x7;
        operand =  m_wordSeq[m_sf][m_sfWordIndex] >> 3;

        switch (opcode)
        {
        case 0: // NOP
            m_sfWordIndex += 1;
            m_burstWord += 1;
            break;

        case 1: // Goto Word - offset from current word is in operand
            sfMove = operand / eSfWordCount;  // how many SF are we moving max move 8191
            wordStep = operand - (sfMove * eSfWordCount);
            nextWord = m_sfWordIndex + wordStep;

            for (int i = 0; i < sfMove; ++i) 
            {
                NextSf();
            }

            // we are in the same SF let's correct which burst we are in and the busrtWord
            // there are 16 51 word busts followed by 4 52 word bursts
            if (nextWord < eTotalSmallBurstWords)
            {
                m_burst = nextWord / eSmallBurstSize;
                m_burstWord = nextWord % eSmallBurstSize;
            }
            else
            {
                m_burst = (nextWord - eTotalSmallBurstWords) / eLargeBurstSize;
                m_burstWord = (nextWord - eTotalSmallBurstWords) % eLargeBurstSize;
            }
            m_sfWordIndex = nextWord;
            break;

        case 2: // Repeat Word n times
            if (m_repeatCount == -1)
            {
                // initialize the repeat process
                m_repeatCount = operand - 1;
                m_repeatIndex = m_sfWordIndex;
            }
            else if (m_repeatCount > 0)
            {
                m_repeatCount -= 1;
            }

            m_burstWord += 1;
            break;

        case 3: // Send in a bad SF identifier for this word
            sfId = operand;
            m_sfWordIndex += 1;
            m_burstWord += 1;
            break;

        case 4: // Send in a bad word identifier for this word
            wordId = operand;
            m_sfWordIndex += 1;
            m_burstWord += 1;
            break;

        default:
            m_sfWordIndex += 1;
            m_burstWord += 1;
            break;
        }
    }
    else
    {
        // move through the SF one word at a time
        m_sfWordIndex += 1;
        m_burstWord += 1;
    }

    // check for the end of a burst and roll SF if needed
    if (m_burstWord >= m_burstSize[m_burst])
    {
        m_burstWord = 0;
        m_endBurst = true;
        if (++m_burst >= eBurstCount)
        {
            NextSf();
        }
    }

    if (sfId != -1)
    {
        wordValue = (wordValue & ~0xFF) | (sfId & 0xff);
    }

    if (wordId != -1)
    {
        wordValue = (wordValue & !(0xfff << 20)) | (wordId << 20);
    }

    return wordValue;
}

//---------------------------------------------------------------------------------------------
// Compute the next sub-frame to send, normally just cycle 1-4.  Use m_skipSfMask to indicate 
// which SF to skip.  Reset Burst index and wordIndex to default next SF values.
void A664Qar::NextSf()
{
    m_burst = 0;
    m_burstWord = 0;
    m_sfWordIndex = 0;

    m_sf = INC_WRAP(m_sf, eSfCount);
    if (m_skipSfMask > 0 && m_skipSfMask < 0xF)
    {
        // check error injection (1) skip sub-frame
        while (BIT(m_skipSfMask, m_sf))
        {
            m_sf = INC_WRAP(m_sf, eSfCount);
        }
    }
}

//=============================================================================================
StaticIoiContainer::StaticIoiContainer()
: m_ioiStaticOutCount(0)
, m_ioiStaticInCount(0)
, m_updateIndex(0)
, m_validIoiOut(0)
, m_validIoiIn(0)
, m_writeError(0)
, m_writeErrorZ1(0)
, m_readError(0)
, m_readErrorZ1(0)
, m_a664QarSched(0)
, m_a664Qar(&_a664_to_ioc_eicas_)
{
    // initialize a few values
    _BatInputVdc_.data = 27.9f;
    _BatSwOutVdc_.data = 28.2f;
    _BrdTempDegC_.data = 10.0f;

    strcpy(_HMUSerialNumber, "0000999999");
    strcpy(_HMUPartNumber, "5316928SK01");
    strcpy(_UTASSwDwgNumber, "PY1022429-028");
    strcpy(_PWSwDwgNumber, "5318410-12SK01");



    // copy the object references into our container array (TBD: do we really need to do this?
    for (int i = 0; i < ASE_OUT_MAX; ++i)
    {
        m_staticIoiOut[i] = aseIoiOut[i];
    }

    m_ioiStaticOutCount = ASE_OUT_MAX;
    m_updateIndex = 0;
    m_validIoiOut = 0;

    for (int i = 0; i < ASE_IN_MAX; ++i)
    {
        m_staticIoiIn[i] = aseIoiIn[i];
    }

    m_ioiStaticInCount = ASE_IN_MAX;
    m_validIoiIn = 0;
}

//---------------------------------------------------------------------------------------------
void StaticIoiContainer::OpenIoi()
{
    for (int i = 0; i < m_ioiStaticOutCount; ++i)
    {
        if (m_staticIoiOut[i]->OpenIoi())
        {
            m_validIoiOut += 1;
        }
    }

    for (int i = 0; i < m_ioiStaticInCount; ++i)
    {
        if (m_staticIoiIn[i]->OpenIoi())
        {
            m_validIoiIn += 1;
        }
    }
}

//---------------------------------------------------------------------------------------------
void StaticIoiContainer::UpdateStaticIoi()
{
    // compute max count to provide a 20 Hz update rate 50ms/10ms => 5 frames
    // m_ioiStaticOutCount - 1: because we handle _a664_to_ioc_eicas_ directly
    const int kMaxCount = ((m_ioiStaticOutCount - 1)/5) + 1;

    static unsigned int lastYrCnt  = 0;
    static unsigned int lastMoCnt  = 0;
    static unsigned int lastDayCnt = 0;
    static unsigned int lastHrCnt  = 0;
    static unsigned int lastMinCnt = 0;
    static unsigned int lastSecCnt  = 0;

    static unsigned char lastMin = 0;
    static unsigned char lastHr  = 0;
    static unsigned char lastDay = 0;
    static unsigned char lastMo  = 0;
    static unsigned char lastYr  = 0;

    // byte
    unsigned char ones;
    unsigned char tens;
    unsigned char data;

    // copy the current time into the rtc_IOI when things change
    // seconds updated
    ones = aseCommon.clocks[eClkRtc].m_time.tm_sec % 10;
    tens = aseCommon.clocks[eClkRtc].m_time.tm_sec / 10;
    data = tens << 4 | ones;
    _rtc_io_rd_seconds[0] = data;

    // minutes updated
    if ( lastMin != aseCommon.clocks[eClkRtc].m_time.tm_min)
    {
        ones = aseCommon.clocks[eClkRtc].m_time.tm_min % 10;
        tens = aseCommon.clocks[eClkRtc].m_time.tm_min / 10;
        data = tens << 4 | ones;
        _rtc_io_rd_minutes[0] = data;
        lastMin = aseCommon.clocks[eClkRtc].m_time.tm_min;
    }

    // hours updated
    if ( lastHr != aseCommon.clocks[eClkRtc].m_time.tm_hour)
    {
        ones = aseCommon.clocks[eClkRtc].m_time.tm_hour % 10;
        tens = aseCommon.clocks[eClkRtc].m_time.tm_hour / 10;
        data = tens << 4 | ones;
        _rtc_io_rd_hour[0] = data;
        lastHr = aseCommon.clocks[eClkRtc].m_time.tm_hour;
    }

    // day updated
    if ( lastDay != aseCommon.clocks[eClkRtc].m_time.tm_mday)
    {
        ones = aseCommon.clocks[eClkRtc].m_time.tm_mday % 10;
        tens = aseCommon.clocks[eClkRtc].m_time.tm_mday / 10;
        data = tens << 4 | ones;
        _rtc_io_rd_date[0] = data;
        lastDay = aseCommon.clocks[eClkRtc].m_time.tm_mday;
    }

    // month updated
    if ( lastMo != aseCommon.clocks[eClkRtc].m_time.tm_mon)
    {
        ones = aseCommon.clocks[eClkRtc].m_time.tm_mon % 10;
        tens = aseCommon.clocks[eClkRtc].m_time.tm_mon / 10;
        data = tens << 4 | ones;
        _rtc_io_rd_month[0] = data;
        lastMo = aseCommon.clocks[eClkRtc].m_time.tm_mon;
    }

    // year updated
    if ( lastYr != aseCommon.clocks[eClkRtc].m_time.tm_year)
    {
        ones = (aseCommon.clocks[eClkRtc].m_time.tm_year - 2000) % 10;
        tens = (aseCommon.clocks[eClkRtc].m_time.tm_year - 2000) / 10;
        data = tens << 4 | ones;
        _rtc_io_rd_year[0] = data;
        lastYr = aseCommon.clocks[eClkRtc].m_time.tm_year;
    }

    // specialized handling for _a664_to_ioc_eicas_ at 20Hz
    m_a664QarSched += 1;
    if (m_a664QarSched == 4)
    {
        // update the burst data
        m_a664Qar.Update();
    }
    else if (m_a664QarSched == 5)
    {
        // send the burst data
        m_a664QarSched = 0;
        if (!_a664_to_ioc_eicas_.Update())
        {
            m_writeError += 1;
        }
    }

    // set the UUT Input maintaining about a 2Hz update rate
    m_writeErrorZ1 = m_writeError;
    for (int i = 0; i < kMaxCount; ++i)
    {
        if (m_staticIoiOut[m_updateIndex] != &_a664_to_ioc_eicas_)
        {
            if (!m_staticIoiOut[m_updateIndex]->Update())
            {
                m_writeError += 1;
            }
        }

        m_updateIndex += 1;
        if (m_updateIndex >= m_ioiStaticOutCount)
        {
            m_updateIndex = 0;
        }
    }

    // read all of the ADRF outputs
    m_readErrorZ1 = m_readError;
    for (int i = 0; i < m_ioiStaticInCount; ++i)
    {
        if (!m_staticIoiIn[i]->Update())
        {
            m_readError += 1;
        }
    }

    // update the RTC time based on what we received
    if (lastYrCnt  != _rtc_io_wr_year_.m_updateCount    &&
        lastMoCnt  != _rtc_io_wr_month_.m_updateCount   &&
        lastDayCnt != _rtc_io_wr_date_.m_updateCount     &&
        lastHrCnt  != _rtc_io_wr_hour_.m_updateCount    &&
        lastMinCnt != _rtc_io_wr_minutes_.m_updateCount &&
        lastSecCnt != _rtc_io_wr_seconds_.m_updateCount
        )
    {
        lastYrCnt  = _rtc_io_wr_year_.m_updateCount   ;
        lastMoCnt  = _rtc_io_wr_month_.m_updateCount  ;
        lastDayCnt = _rtc_io_wr_date_.m_updateCount    ;
        lastHrCnt  = _rtc_io_wr_hour_.m_updateCount   ;
        lastMinCnt = _rtc_io_wr_minutes_.m_updateCount;
        lastSecCnt = _rtc_io_wr_seconds_.m_updateCount;

        // Move the new values into RTC time
        // sec: data = tens << 4 | ones;
        aseCommon.clocks[eClkRtc].m_time.tm_year = VALUE(_rtc_io_wr_year[0]) + 2000;
        aseCommon.clocks[eClkRtc].m_time.tm_mon  = VALUE(_rtc_io_wr_month[0]);
        aseCommon.clocks[eClkRtc].m_time.tm_mday = VALUE(_rtc_io_wr_date[0]);
        aseCommon.clocks[eClkRtc].m_time.tm_hour = VALUE(_rtc_io_wr_hour[0]);
        aseCommon.clocks[eClkRtc].m_time.tm_min  = VALUE(_rtc_io_wr_minutes[0]);
        aseCommon.clocks[eClkRtc].m_time.tm_sec  = VALUE(_rtc_io_wr_seconds[0]);
    }
}

//---------------------------------------------------------------------------------------------
bool StaticIoiContainer::SetStaticIoiData(SecComm& secComm)
{
    SecRequest request = secComm.m_request;
    if (request.variableId < m_ioiStaticOutCount)
    {
        // catch set action directed at _a664_to_ioc_eicas_ and redirect to m_a664Qar
        if (m_staticIoiOut[request.variableId] == &_a664_to_ioc_eicas_)
        {
            return m_a664Qar.TestControl(request);
        }
        else
        {
            return m_staticIoiOut[request.variableId]->SetStaticIoiData(request);
        }
    }
    else if (request.variableId == m_ioiStaticOutCount)
    {
        ResetStaticIoi();
        return true;
    }
    else
    {
        return false;
    }
}

//---------------------------------------------------------------------------------------------
bool StaticIoiContainer::GetStaticIoiData( SecComm& secComm )
{
    if (secComm.m_request.variableId < m_ioiStaticInCount)
    {
        // here is a little trick we play for String types to big to fit into the streamData
        secComm.m_response.streamSize = secComm.m_request.sigGenId;

        // get the value and return true
        m_staticIoiIn[secComm.m_request.variableId]->GetStaticIoiData(secComm.m_response);
        return true;
    }
    else
    {
        return false;
    }
}

//---------------------------------------------------------------------------------------------
void StaticIoiContainer::SetNewState( SecRequest& request)
{
    if (request.variableId < m_ioiStaticOutCount)
    {
        // if not valid leave the running state at disabled
        bool newState = (request.sigGenId == 1) && 
                        m_staticIoiOut[request.variableId]->ioiValid;
        m_staticIoiOut[request.variableId]->SetRunState(newState);
    }
}

//---------------------------------------------------------------------------------------------
// Called when no script is running
void StaticIoiContainer::Reset()
{
    for (int i = 0; i < m_ioiStaticOutCount; ++i)
    {
        // do not reset the running state if it is invalid
        m_staticIoiOut[i]->SetRunState(m_staticIoiOut[i]->ioiValid);
    }

    // when the script is done reset these
    _pat_scr_.data = 0;         // pat_scr = 0 : the main screen
    _pat_scr_button_.data = 0;  // pat_button = 0 : 0

    ResetStaticIoi();
}

//---------------------------------------------------------------------------------------------
void StaticIoiContainer::ResetStaticIoi()
{
    // and values we want to reset if no script is running
    // use 0xffffff to indicate the value has not been updated

    _8204050_32_.data = 0xffffff;
    _8204051_32_.data = 0xffffff;
    _8204052_32_.data = 0xffffff;
    _8204053_32_.data = 0xffffff;
    _8204054_32_.data = 0xffffff;
    _8204055_32_.data = 0xffffff;
    _8204056_32_.data = 0xffffff;
    _8204059_32_.data = 0xffffff;
    _8204060_32_.data = 0xffffff;
    _8204061_32_.data = 0xffffff;
    _8204062_32_.data = 0xffffff;
    _8204063_32_.data = 0xffffff;
    _8204064_32_.data = 0xffffff;
    _8204065_32_.data = 0xffffff;
    _8204066_32_.data = 0xffffff;
    _8204067_32_.data = 0xffffff;
    _8204068_32_.data = 0xffffff;
    _8204069_32_.data = 0xffffff;
    _8204070_32_.data = 0xffffff;
    _8204071_32_.data = 0xffffff;
    _8204072_32_.data = 0xffffff;
    _8204057_32_.data = 0xffffff;
    _8204058_32_.data = 0xffffff;

    _8204050_64_.data = 0xffffff;
    _8204051_64_.data = 0xffffff;
    _8204052_64_.data = 0xffffff;
    _8204053_64_.data = 0xffffff;
    _8204054_64_.data = 0xffffff;
    _8204055_64_.data = 0xffffff;
    _8204056_64_.data = 0xffffff;
    _8204059_64_.data = 0xffffff;
    _8204060_64_.data = 0xffffff;
    _8204061_64_.data = 0xffffff;
    _8204062_64_.data = 0xffffff;
    _8204063_64_.data = 0xffffff;
    _8204064_64_.data = 0xffffff;
    _8204065_64_.data = 0xffffff;
    _8204066_64_.data = 0xffffff;
    _8204067_64_.data = 0xffffff;
    _8204068_64_.data = 0xffffff;
    _8204069_64_.data = 0xffffff;
    _8204070_64_.data = 0xffffff;
    _8204071_64_.data = 0xffffff;
    _8204072_64_.data = 0xffffff;
    _8204057_64_.data = 0xffffff;
    _8204058_64_.data = 0xffffff;

    _adrf_pat_udt_remain_a_.data = 0xffffff;
    _adrf_pat_udt_remain_b_.data = 0xffffff;

    // clear any error injection and reset data an NDO
    m_a664Qar.Reset();
}

