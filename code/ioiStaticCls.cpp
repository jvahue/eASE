//-----------------------------------------------------------------------------
//          Copyright (C) 2014-2018 Knowlogic Software Corp.
//         All Rights Reserved. Proprietary and Confidential.
//
//    File: ioiStaticCls.cpp
//
//    Description: Implements test control object for the UTAS A717 Module
//
//-----------------------------------------------------------------------------


//----------------------------------------------------------------------------/
// Compiler Specific Includes                                                -/
//----------------------------------------------------------------------------/
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ioiapi.h>

//----------------------------------------------------------------------------/
// Software Specific Includes                                                -/
//----------------------------------------------------------------------------/
#include "AseCommon.h"
#include "alt_basic.h"
#include "ioiStaticCls.h"

//----------------------------------------------------------------------------/
// Local Defines                                                             -/
//----------------------------------------------------------------------------/
#define QAR_A717_CFG_RQST_NAME "ADRF_A717_CONFIG_REQ"
#define QAR_A717_CFG_RSP_NAME "A717_ADRF_CONFIG_ACK"
#define QAR_A717_STATUS_NAME "A717_STATUS_MSG"

#define QAR_A717_IOI_NAME1 "A717Subframe1"
#define QAR_A717_IOI_NAME2 "A717Subframe2"
#define QAR_A717_IOI_NAME3 "A717Subframe3"
#define QAR_A717_IOI_NAME4 "A717Subframe4"

//----------------------------------------------------------------------------/
// Local Typedefs                                                            -/
//----------------------------------------------------------------------------/

//----------------------------------------------------------------------------/
// Local Variables                                                           -/
//----------------------------------------------------------------------------/
UINT32 mirrorQarA664[A664Qar::eSfCount][A664Qar::eSfWordCount];
UINT32 mirrorQarA717[A664Qar::eSfCount][A664Qar::eSfWordCount];

//----------------------------------------------------------------------------/
// Constant Data                                                             -/
//----------------------------------------------------------------------------/
//#include "AseStaticIoiInfo.h"

const CHAR* ioiSfName[] = { QAR_A717_IOI_NAME1,
                            QAR_A717_IOI_NAME2,
                            QAR_A717_IOI_NAME3,
                            QAR_A717_IOI_NAME4 };

const CHAR* ioiStatusName = QAR_A717_STATUS_NAME;

//---------------------------------------------------------------------------------------------
UINT16 ReverseBarker(UINT16 barker)
{

    UINT16 revBarker;
    UINT16 x;

    // Shift the bits thru each other left to right.
    x = barker;
    x = (((x & 0xaaaa) >> 1) | ((x & 0x5555) << 1));
    x = (((x & 0xcccc) >> 2) | ((x & 0x3333) << 2));
    x = (((x & 0xf0f0) >> 4) | ((x & 0x0f0f) << 4));
    x = (((x & 0xff00) >> 8) | ((x & 0x00ff) << 8));
    revBarker = ((x >> 8) | (x << 8));

    // the reversed value is aligned in the 
    // high-order byte, Barker is 12 bits, so right-shift 4.
    revBarker >>= 4;
    return revBarker;
}

//=============================================================================================
// Public interfaces
//=============================================================================================
StaticIoiObj::StaticIoiObj(char* name, bool isInput)
    : m_ioiChan(0)
    , m_ioiValid(false)
    , m_ioiRunning(true)
    , m_isAseInput(isInput)
    , m_isParam(false)
    , m_updateCount(0)
{
    strcpy(m_ioiName, name);
    strcpy(m_shortName, name);
    CompressName(m_shortName, 18);
}

//---------------------------------------------------------------------------------------------
bool StaticIoiObj::OpenIoi()
{
    ioiStatus openStatus;

    if (m_isAseInput)
    {
        openStatus = ioi_open(m_ioiName, ioiReadPermission, (int*)&m_ioiChan);
    }
    else
    {
        openStatus = ioi_open(m_ioiName, ioiWritePermission, (int*)&m_ioiChan);
    }
    m_ioiValid = openStatus == ioiSuccess;
    m_ioiRunning = m_ioiValid;
    return m_ioiValid;
}

//---------------------------------------------------------------------------------------------
// Return the status of an actual IOI write.  For params we skip just return success
bool StaticIoiObj::WriteStaticIoi(void* data)
{
    ioiStatus writeStatus = ioiSuccess;

    // if we are valid and running, [and not being run by a parameter?]
    if (m_ioiValid && m_ioiRunning)
    {
        writeStatus = ioi_write(m_ioiChan, data);
    }

    m_updateCount += (m_ioiValid && m_ioiRunning && writeStatus == ioiSuccess) ? 1 : 0;
    return writeStatus == ioiSuccess;
}

//---------------------------------------------------------------------------------------------
// Return the status of an actual IOI write.  For params we skip just return success
bool StaticIoiObj::ReadStaticIoi(void* data)
{
    ioiStatus readStatus = ioiSuccess;

    if (m_ioiValid && m_ioiRunning)
    {
        readStatus = ioi_read(m_ioiChan, data);
    }

    m_updateCount += (m_ioiValid && m_ioiRunning && readStatus == ioiSuccess) ? 1 : 0;
    return readStatus == ioiSuccess;
}

//---------------------------------------------------------------------------------------------
char* StaticIoiObj::Display(char* dest, UINT32 dix)
{
    *dest = '\0';
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
    m_ioiRunning = newState;
}

//---------------------------------------------------------------------------------------------
bool StaticIoiInt::SetStaticIoiData(SecRequest& request)
{
    data = request.resetRequest;
    Update();
    return true;
}

//---------------------------------------------------------------------------------------------
char* StaticIoiInt::Display(char* dest, UINT32 dix)
{
    if (m_ioiRunning)
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
    if (m_isAseInput)
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

//---------------------------------------------------------------------------------------------
bool StaticIoiFloat::SetStaticIoiData(SecRequest& request)
{
    data = request.value;
    Update();
    return true;
}

//---------------------------------------------------------------------------------------------
char* StaticIoiFloat::Display(char* dest, UINT32 dix)
{
    if (m_ioiRunning)
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
    if (m_isAseInput)
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

//---------------------------------------------------------------------------------------------
StaticIoiStr::StaticIoiStr(char* name, char* value, int size, bool isInput/*=false*/)
    : StaticIoiObj(name, isInput)
    , displayAt(0)
    , data(value)
    , bytes(size)
{
    memset(data, 0, bytes);
}

//---------------------------------------------------------------------------------------------
bool StaticIoiStr::SetStaticIoiData(SecRequest& request)
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
char* StaticIoiStr::Display(char* dest, UINT32 dix)
{
    unsigned int* dp = (unsigned int*)&data[displayAt];
    if (m_ioiRunning)
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
    if (m_isAseInput)
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
bool StaticIoiStr::GetStaticIoiData(IocResponse& m_response)
{
    UINT32 offset = m_response.streamSize;
    UINT32 left = bytes - offset;

    memcpy(m_response.streamData, &data[offset], left);
    m_response.streamSize = left;
    return true;
}

//=============================================================================================
A664Qar::A664Qar()
{
}

void A664Qar::Reset(StaticIoiObj* buffer/*=NULL*/)
{
#define kBusrtBytes  ((52 * 8) - 4)
    //----- operation and configuration data -----
    m_idl = static_cast<StaticIoiStr*>(buffer);  // the buffer associated with the IOI
    m_qarSfWordCount = 1024;  // A664 defaults to 1024

    //----- Run Time Data -----
    m_sf = 0;          // start sending from SF1
    m_sfWordIndex = 0; // start sending from word index 0
    m_schedule = 0;

    // which burst of the sub-frame are we sending
    m_endBurst = false;
    m_burst = 0;       // start with the first burst group
    m_burstWord = 0;   // which word we are on in the burst
    for (int i = 0; i < eBurstCount - 4; ++i)
    {
        m_burstSize[i] = 51;
    }
    m_burstSize[16] = 52;
    m_burstSize[17] = 52;
    m_burstSize[18] = 52;
    m_burstSize[19] = 52;

    m_random = 0;      // default 0 random words in a burst
    m_kMaxRandom = (m_idl->bytes - kBusrtBytes) / 8;

    m_garbageSet = 0;
    m_garbageCnt = 0;

    // ensure these are not zero as that terminates processing in the UUT
    m_ndo[0] = 0x97560801;
    m_ndo[1] = 0x97561002;
    m_ndo[2] = 0x97561803;
    m_ndo[3] = 0x97562004;
    m_nonNdo = (m_ndo[0] | m_ndo[1] | m_ndo[2] | m_ndo[3]) + 1;

    m_oneSecondClk = 0;

    //----- Execution Status ------
    m_frameCount = 0;

    //----- QAR A664 ERROR injection control -----
    // which sub-frame are we outputting
    m_skipSfMask = 0;  // don't skip any

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

    if (offset == eQar664Ndo)
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
    else if (offset == eQar664SfSeq)
    {
        int* data = (int*)request.charData;
        m_skipSfMask = *data;
    }
    else if (offset == eQar664WdSeq)
    {
        // packed data is word index (corrected for SF) and cmd value
        UINT16* index = (UINT16*)request.charData;
        UINT16* cmdWord = (UINT16*)request.charData;
        UINT16* dest = (UINT16*)&m_wordSeq[0][0];

        cmdWord += 1;

        do
        {
            *(dest + *index) = *cmdWord;
            index += 2;
            cmdWord += 2;
        } while (!(*index == eQar664Stop && *cmdWord == eQar664Stop));

        // clear the QAR-A664 data mirror
        memset(mirrorQarA664, 0, sizeof(mirrorQarA664));
    }
    else if (offset == eQar664WdSeqState)
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
    else if (offset == eQar664Random)
    {
        // we limit this to 50 in PySte and add suspenders to our belt here
        int* dest = &m_random;
        *dest = *(int*)request.charData;
        if (m_random > m_kMaxRandom)
        {
            m_random = m_kMaxRandom;
        }
    }
    else if (offset == eQar664Garbage)
    {
        // send multiple sets of garbage data - not used during testing 5/25/18
        int* dest = &m_garbageSet;
        *dest = *(int*)request.charData;
        if (m_garbageSet > 2)
        {
            m_garbageSet = 2;
        }
        m_garbageCnt = m_garbageSet;
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
// Parameters call this function to set a SF data word to a specific value
void A664Qar::SetData(UINT16 value,
                      UINT16 mask, UINT8 sfMask, UINT32 rate, UINT16 base, UINT8 index)
{
    // see if this word belongs in this SF
    if (BIT(sfMask, m_sf))
    {
        UINT16 wordIndex = base + ((m_qarSfWordCount / rate) * index);
        // compute the word offset based on the index and the number of words in the SF
        UINT16* data = &m_qarWords[m_sf][wordIndex];

        *data &= mask;  // turn off the bits we are packing this into
        *data |= value; // insert the value
    }
}

//---------------------------------------------------------------------------------------------
// This function handles all the processing associated with the A664 QAR object
int A664Qar::UpdateIoi()
{
    int writeErr = 0;

    m_oneSecondClk = INC_WRAP(m_oneSecondClk, eTICKS_PER_SEC);  // update our internal time

    // specialized handling for _a664_to_ioc_eicas_ at 20Hz
    m_schedule += 1;

    // --------------------------------------------------------------------------
    // BUGjv: This code make me nervous as it seems we might try to 
    //        write garbage and data in the same MIF 
    // scripts do not use the eQarGarbage cmd as of 5/25/18 so this is inactive
    if (m_schedule < 5)
    {
        if (m_garbageCnt == 0)
        {
            m_garbageCnt = m_garbageSet;
        }

        if (m_garbageCnt > 0)
        {
            m_garbageCnt -= 1;
            // send total garbage until 
            Garbage();
            if (!m_idl->Update())
            {
                writeErr = 1;
            }
        }
    }

    if (m_schedule == 4)
    {
        // update the burst data
        Update();
    }
    else if (m_schedule == 5)  // send data at 20Hz
    {
        // send the burst data
        m_schedule = 0;
        if (!m_idl->Update())
        {
            writeErr = 1;
        }
    }

    return writeErr;
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
bool A664Qar::Update()
{
    UINT32 lastSf;
    UINT32 lastSfWord;
    UINT32 wordValue;
    // Fill in the IOI buffer with content from the sf/burst going out
    UINT32 totalInsert = 0;   // number of "words"  insert NDO/DATA
    UINT32 randomInsert = 0;  // number of randomly insert NDO/DATA
    UINT32* fillPtr = (UINT32*)m_idl->data;    // where the data is going

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
            if (m_skipSfMask < 0xF)
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

    //-----------------------------------------------------------
    // fill in a few more random based on how many we have done
    if (randomInsert < m_random)
    {
        // fill in until we have m_random
        while (randomInsert < m_random)
        {
            *(fillPtr++) = (m_nonNdo + randomInsert);
            *(fillPtr++) = randomInsert << 16;  // insert in MSW
            randomInsert += 1;
        }
    }

    // terminate the data set (max 896 bytes + 4 here = 900 bytes of 1024) 
    *(fillPtr++) = 0;

    return true;
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
        opcode = m_wordSeq[m_sf][m_sfWordIndex] & 0x7;  // 7 actions and a NOP
        operand = m_wordSeq[m_sf][m_sfWordIndex] >> 3;  // 13 bits

        switch (opcode)
        {
        case 0: // NOP = just pack the word and move to the next word
            m_sfWordIndex += 1;
            m_burstWord += 1;
            break;

        case 1: // Goto Word - offset from current word is in operand
            sfMove = operand / eSfWordCount;  // how many SF are we moving max move 8191
            wordStep = operand - (sfMove * eSfWordCount);
            nextWord = m_sfWordIndex + wordStep;
            if (nextWord > eSfWordCount)
            {
                NextSf();
                nextWord -= eSfWordCount;
            }

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

        default:  // see NOP (0)
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
        wordValue = (wordValue & ~(0xfff << 20)) | (wordId << 20);
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

//---------------------------------------------------------------------------------------------
// determine if this set ioi is targeted to this object
bool A664Qar::HandleRequest(StaticIoiObj* targetIoi)
{
    return targetIoi == m_idl;
}

//---------------------------------------------------------------------------------------------
// send garbage data to the ADRF
void A664Qar::Garbage()
{
    UINT32* fillPtr = (UINT32*)m_idl->data;    // where the data is going

    *(fillPtr++) = m_frameCount++;

    // now fill in 400 words of garbage
    for (int i = 0; i < 400; ++i)
    {
        *(fillPtr++) = (i * 2) + 1;
        *(fillPtr++) = (i * 2) + 1;
    }
}

//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------
//                                 QAR A717
//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------
A717Qar::A717Qar()
    : A664Qar()
{
    m_testCtrl.bSynced = FALSE;
    m_testCtrl.bQarEnabled = FALSE;
    m_testCtrl.bAcceptCfgReq = TRUE;
    m_testCtrl.bCfgReqReceived = FALSE;
    m_testCtrl.bCfgRespAvail = FALSE;
    m_testCtrl.bAutoRespondAck = TRUE;
    m_testCtrl.bAutoRespondNack = FALSE;
    m_testCtrl.qarBitState = BITSTATE_NO_FAIL;

    // default A717 word count to 64
    m_qarSfWordCount = eDefaultSfWdCnt; // temp until recfg is implemented
    m_qaRevSyncFlag = 0;
    m_qarFmtEnum = (UINT8)QAR_BIPOLAR_RETURN_ZERO;
    m_qarWordSizeEnum = (UINT8)QAR_64_WORDS;
}

//---------------------------------------------------------------------------------------------
void A717Qar::Reset(StaticIoiObj* cfgRqst, StaticIoiObj* cfgRsp, StaticIoiObj* sts,
                    StaticIoiObj* sf1, StaticIoiObj* sf2,
                    StaticIoiObj* sf3, StaticIoiObj* sf4)
{
    UINT32 tempWordCount = m_qarSfWordCount;
    A664Qar::Reset();
    m_qarSfWordCount = tempWordCount;

    m_cfgRqst = static_cast<StaticIoiStr*>(cfgRqst);
    m_cfgResp = static_cast<StaticIoiStr*>(cfgRsp);
    m_status = static_cast<StaticIoiStr*>(sts);

    m_sfObjs[0] = static_cast<StaticIoiStr*>(sf1);
    m_sfObjs[1] = static_cast<StaticIoiStr*>(sf2);
    m_sfObjs[2] = static_cast<StaticIoiStr*>(sf3);
    m_sfObjs[3] = static_cast<StaticIoiStr*>(sf4);

    m_statusIoiValid = true; // This flag enables/disables A717_STATUS_MSG ioi xmit

    m_oneSecondClk = 0;

    // default the 4 barker codes
    m_qarWords[0][0] = 0x247;
    m_qarWords[1][0] = 0x5b8;
    m_qarWords[2][0] = 0xa47;
    m_qarWords[3][0] = 0xdb8;
}

//---------------------------------------------------------------------------------------------
int A717Qar::UpdateIoi()
{
    // This function is called at 100Hz
    // Determine if it is time to send out a 1Hz SF/status update, etc...
    // Check if QAR is enabled for this configuration
    UINT8 sfUpdateFlags[4];

    // UTAS ICD says: QAR_STATUS.subframeID shall be zero if no data sent.
    // Init the array of SF update status.
    memset(sfUpdateFlags, 0, sizeof(sfUpdateFlags));

    ReadCfgRequestMsg();

    // Update the tick count
    m_oneSecondClk += 1;

    // Is it time to send a SF - at 1Hz?
    // Output the next SF (if QAR active) and the status msg.
    if (m_oneSecondClk >= eTICKS_PER_SEC)
    {
        // UTAS will probably read and respond to a Recfg Request in the same frame as
        // the Status Msg and Subframe writes, so check here if something needs to go out.
        WriteCfgRespMsg();

        // If enabled and running, send the next expected subframe...
        if (m_testCtrl.bQarEnabled)
        {
            // copy the local SF image to the IOI 16 -> 32 bits
            UINT16* src = m_qarWords[m_sf];
            UINT32* dst = (UINT32*)&m_sfObjs[m_sf]->data[0];
            UINT32* mrr = (UINT32*)&mirrorQarA717[m_sf][0];

            for (int i = 0; i < m_qarSfWordCount; ++i)
            {
                *dst++ = *src;
                *mrr++ = *src++;  // keep a local copy for us to look at
            }

            // send next SF data via its IOI
            if (m_sfObjs[m_sf]->Update())
            {
                // a subframe was written, indicate this in the status msg
                sfUpdateFlags[m_sf] = 1;
            }
            else
            {
                m_writeErrCnt += 1;
            }

            // Set up next SF to go out in 1 sec.      
            m_sf = INC_WRAP(m_sf, eNUM_SUBFRAMES);
        }

        // A QAR_STATUS msg is ALWAYS sent at 1Hz, regardless of run-state
        WriteStatusMsg(sfUpdateFlags);

        m_oneSecondClk = 0;
    }

    return 0;
}

//---------------------------------------------------------------------------------------------
bool A717Qar::TestControl(SecRequest& request)
{
    bool status = true;
    SINT32 offset = (SINT32)request.clearCfgRequest;
    UINT32 details = request.resetRequest;

    if (offset == eQar717Status)
    {
        UINT8* pStatus = (UINT8*)request.charData;

        // Set the fields to be used in status msg
        SetStatusMsgFields(pStatus[0], pStatus[1], pStatus[2], pStatus[3], pStatus[4]);
    }
    else if (offset == eQar717ReCfgResp)
    {
        UINT8* cfgData = (UINT8*)request.charData;
        // Set up the response to a reconfigure request, disable auto respond
        SetCfgRespFields(cfgData[0], cfgData[1], cfgData[2], cfgData[3]);
        m_testCtrl.bAutoRespondAck = false;
    }
    else if (offset == eQar717AutoResp)
    {
        UINT8* pAutoModeCmd = (UINT8*)request.charData;
        m_testCtrl.bAutoRespondAck = pAutoModeCmd[0] == 0 ? false : true;
        // If auto respond is enabled, then disable bCfgRespAvail until the user sets it again 
        if (m_testCtrl.bAutoRespondAck)
        {
            m_testCtrl.bCfgRespAvail = false;
            m_testCtrl.bAutoRespondNack = false;
        }
    }
    else if (offset == eQar717AutoNack)
    {
        UINT8* pAutoModeCmd = (UINT8*)request.charData;
        m_testCtrl.bAutoRespondNack = pAutoModeCmd[0] == 0 ? false : true;

        // If auto respond is enabled, then disable bCfgRespAvail until the user sets it again 
        if (m_testCtrl.bAutoRespondNack)
        {
            m_testCtrl.bCfgRespAvail = false;
            m_testCtrl.bAutoRespondAck = false;
        }
    }
    else if (offset == eQar717SkipSF)
    {
        // TODO
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
bool A717Qar::HandleRequest(StaticIoiObj* targetIoi)
{
    bool status = (targetIoi == m_cfgResp || targetIoi == m_status);

    // if no match yet, check if the targetIoi is a SF
    for (int ix = 0; !status && ix < eNUM_SUBFRAMES; ++ix)
    {
        status = (targetIoi == m_sfObjs[ix]);
    }
    return status;
}

//---------------------------------------------------------------------------------------------
void A717Qar::SetCfgRespFields(UINT8 sfWc, UINT8 reverseFlag, UINT8 format, UINT8 respType)
{
    // fill the response msg to be sent as a reply to the next cfg request.
    m_cfgRespMsg.rspType = respType;
    m_cfgRespMsg.cfg.bRevSync = reverseFlag;
    m_cfgRespMsg.cfg.numWords = sfWc;
    m_cfgRespMsg.cfg.fmt = format;

    m_testCtrl.bCfgRespAvail = TRUE;
}


//---------------------------------------------------------------------------------------------
void A717Qar::SetStatusMsgFields(UINT8 disableFlag, UINT8 bitState,
                                 UINT8 sfWc, UINT8 reverseFlag, UINT8 format)
{
  // for now just reset the word count
    m_testCtrl.bQarEnabled = disableFlag ? 0 : 1; // Damn negative-logic in ICD
    m_testCtrl.qarBitState = (BIT_STATE)bitState;

    // TODO DaveB, setting
    m_qarSfWordCount = 1 << (sfWc + 6);    // Convert enum to value  
    m_qaRevSyncFlag = reverseFlag;
    m_qarFmtEnum = format;
}

//---------------------------------------------------------------------------------------------
void A717Qar::WriteStatusMsg(UINT8* pSfArray)
{
    UINT16 reg = 0xF51E; // All OK
    UINT32 cfgReg = 0x00000020; // show DMA ENABLED, TEST bits off.
    //ioiStatus ioiStat;

    // The ICD defines the mode as disabled vs enabled.
    m_qarMgrStatusMsg.bDisabled = m_testCtrl.bQarEnabled ? 0 : 1;
    m_qarMgrStatusMsg.qarBitState = m_testCtrl.qarBitState;

    // Copy the array indicating which SF(s) have been updated during this second.
    memcpy(m_qarMgrStatusMsg.sfUpdateFlags, pSfArray, sizeof(m_qarMgrStatusMsg.sfUpdateFlags));

    // Convert the SF word count back to the enum value.
    m_qarMgrStatusMsg.cfg.numWords = (log10(m_qarSfWordCount) / log10(2)) - 6;

    m_qarMgrStatusMsg.cfg.bRevSync = m_qaRevSyncFlag;
    m_qarMgrStatusMsg.cfg.fmt = m_qarFmtEnum;

    // Set the overall sync status to match the high level flag in status msg
    UINT32 syncStatus = (m_testCtrl.bSynced) ? 1 : 0;
    reg = reg | (syncStatus & SYNC_MASK);
    m_qarMgrStatusMsg.statusRegister = reg;

    // Set the UTAS Rx Cfg Register to look somewhat realistic to the true settings
    cfgReg = cfgReg | (m_qaRevSyncFlag << 4);           // REVERSE BARKER FLAG
    cfgReg = cfgReg | (m_qarFmtEnum << 3);              //  QAR FMT 0 - BPRZ, 1 - HBP
    cfgReg = cfgReg | (m_qarMgrStatusMsg.cfg.numWords); // QAR WORDCNT enum

    m_qarMgrStatusMsg.rxCfgRegister = cfgReg;

    // Move the content of QAR Module status to the output buffer for Updating.
    memcpy(m_status->data, &m_qarMgrStatusMsg, sizeof(m_qarMgrStatusMsg));

    if (m_statusIoiValid)
    {
        m_status->Update();
    }
}

//---------------------------------------------------------------------------------------------
void A717Qar::ReadCfgRequestMsg()
{
    // Check if a request-for-configuration change msg has been received from the ADRF
    // and handle as needed. 
    if (m_cfgRqst->Update())
    {
        m_testCtrl.bCfgReqReceived = TRUE;
        memcpy(&m_cfgReqMsg, m_cfgRqst->data, sizeof(m_cfgReqMsg));

        // If auto-respond is enabled, use the cfg data in request to format the cfg response
        // and the status msg.
        if (m_testCtrl.bAutoRespondAck)
        {
            m_cfgRespMsg.cfg.bRevSync = m_cfgReqMsg.cfg.bRevSync;
            m_cfgRespMsg.cfg.numWords = m_cfgReqMsg.cfg.numWords;
            m_cfgRespMsg.cfg.fmt = m_cfgReqMsg.cfg.fmt;

            switch (m_cfgReqMsg.reqType)
            {
            case 0: // Clear
                m_cfgRespMsg.rspType = 0; // Cleared-OK, CFG-Request is cleared          
                break;

            case 1: // Reconfig/Enable myself
                m_cfgRespMsg.rspType = 1; // ACK OK         

                // Use the SetStatusMsgFields function to update
                // the status msg fields AND adjust subframe size to comply
                SetStatusMsgFields(0, m_testCtrl.qarBitState, m_cfgReqMsg.cfg.numWords,
                                   m_cfgReqMsg.cfg.bRevSync, m_cfgReqMsg.cfg.fmt);
                break;

            case 2: // Disable myself
                m_cfgRespMsg.rspType = 1; // ACK OK
                // Don't change the cfg fields
                SetStatusMsgFields(1, m_testCtrl.qarBitState, m_cfgReqMsg.cfg.numWords,
                                   m_cfgReqMsg.cfg.bRevSync, m_cfgReqMsg.cfg.fmt);
                break;
            }
        }
        else if (m_testCtrl.bAutoRespondNack)
        {
          // Automatically NACK attempts to reconfig
          // The cfg data in the response should match whatever is in the status msg
            m_cfgRespMsg.cfg.bRevSync = m_qaRevSyncFlag;
            m_cfgRespMsg.cfg.numWords = (log10(m_qarSfWordCount) / log10(2)) - 6;
            m_cfgRespMsg.cfg.fmt = m_qarFmtEnum;

            switch (m_cfgReqMsg.reqType)
            {
            case 0: // Clear
                m_cfgRespMsg.rspType = 0; // ALWAYS accept a clear request
                break;

            case 1: // Reconfig/Enable NACK'ed
            case 2: // Disable myself NACK'ed
                m_cfgRespMsg.rspType = 2;
                break;
            }
        }
        else if (m_testCtrl.bCfgRespAvail)
        {
            // If the test script has provided a canned response handle it in WriteCfgRespMsg     
        }
    }
}

//---------------------------------------------------------------------------------------------
void A717Qar::WriteCfgRespMsg()
{
    // If a cfg request was received from ADRF, and response data was sent is avail
    // send it back
    if (m_testCtrl.bCfgReqReceived)
    {
        if (m_testCtrl.bAutoRespondAck || m_testCtrl.bAutoRespondNack)
        {
            // The response was formatted during the request... copy to buffer and send it out
            m_testCtrl.bCfgReqReceived = FALSE;
            memcpy(m_cfgResp->data, &m_cfgRespMsg, sizeof(m_cfgRespMsg));
            m_cfgResp->Update();
        }
        else if (m_testCtrl.bCfgRespAvail)
        {
            m_testCtrl.bCfgRespAvail = FALSE;
            m_testCtrl.bCfgReqReceived = FALSE;
            memcpy(m_cfgResp->data, &m_cfgRespMsg, sizeof(m_cfgRespMsg));
            m_cfgResp->Update();
        }
    }
}

//=============================================================================================
// Protected methods
//=============================================================================================
