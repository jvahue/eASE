// File class implementation

/*****************************************************************************/
/* Compiler Specific Includes                                                */
/*****************************************************************************/
#include <deos.h>
#include <videobuf.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>



/*****************************************************************************/
/* Software Specific Includes                                                */
/*****************************************************************************/
#include "File.h"


/*****************************************************************************/
/* Local Defines                                                             */
/*****************************************************************************/

#define CFFS_POSTVER_50

#ifdef CFFS_POSTVER_50
  #define CFFS_WRITE( a, b, c, d, e, f, g, h, i, j, k, l, m, n, o) \
          cffsCommit( a, b, c, d, e, f, g, h, i, j, k, l, m, n   )
#else
  #define CFFS_WRITE( a, b, c, d, e, f, g, h, i, j, k, l, m, n, o) \
          cffsCommit( a, b, c, d, e, f, g, h, i, j, k, l, m, n, o)
#endif

#define BYTES_TO_DWORDS(x) ((x / sizeof(UINT32))+1)
//#define MAILBOX_DEBUG

#define DEFAULT_PORTSIZE  (200 * 1024)
#define DEFAULT_READ_SIZE (DEFAULT_PORTSIZE - (1 * 1024)) // Don't read thru end of Blk
                                               // Note PORTREAD_SIZE must be 512 aligned
#define DEF_NUMPORTS 1

#define CFFS_FLUSH       TRUE
#define CFFS_NO_FLUSH    FALSE
#define CFFS_TRUNC       TRUE
#define CFFS_NO_TRUNC    FALSE
#define CFFS_BLOCKING    TRUE
#define CFFS_NO_BLOCKING FALSE

#define CFFS_OFFSET_NOT_FROM_END FALSE

#define SRV_METADATA_RES "CFFS_RAM_RO"

/*****************************************************************************/
/* Local Typedefs                                                            */
/*****************************************************************************/


/*****************************************************************************/
/* Local Variables                                       */
/*****************************************************************************/
const char* partName[] = {"CM-partition", "ADRF-partition"};

/*****************************************************************************/
/* Local Function Prototypes                                                 */
/*****************************************************************************/

/*****************************************************************************/
/* Class Function Definitions                                                */
/*****************************************************************************/

/*****************************************************************************
 * Function:    File
 *
 * Description: Constructor for encapsulating the behavior of a file supported
 *              under the CFFS system
 *
 * Parameters:  [in] fileName: name of the file to be opened
 *
 * Returns:     None
 *
 * Notes:
 *
 ****************************************************************************/
File::File()
    : m_sAdr(NULL)
    , m_cAdr(NULL)
    , m_bInit(FALSE)
{
    Reset();
    UNSIGNED32  i;
    UNSIGNED32  initVal;
    UNSIGNED32* aliveCnt;

    Reset();

    strncpy(m_clientAccessRes, "CM-CAR", eMaxResName);

    // Open m_portAddr[] to 0 and the set the portsize.
    for (i = 0; i < eNumPorts; i++)
    {
        m_portAddr[i] = 0;
        m_portSize[i] = DEFAULT_PORTSIZE;
    }

    // Attach to the server's metadata resource
    m_resStatus = attachPlatformResource("", SRV_METADATA_RES,
                                             &m_hPlatformRes,
                                             &m_accessStyle,
                                             &m_sAdr);

    if (m_resStatus == resValid)
    {
        // Get a pointer to the alive counter,
        // wait for increment to show the cffs is running
        aliveCnt = cffsAliveCounter( m_sAdr );
        initVal  = *aliveCnt;
        do
        {
            if (*aliveCnt < initVal)
            {
                // Note the rollover case is handled
                initVal = *aliveCnt;
            }
            waitUntilNextPeriod();
        }
        while (*aliveCnt <= initVal);

        // Attach to the client access resource (CAR)
        m_resStatus = attachPlatformResource("", m_clientAccessRes,
                                                 &m_hPlatformRes,
                                                 &m_accessStyle,
                                                 &m_cAdr);
    }

    // if SRV and Client resource attachments are good set up port info
    if (m_resStatus == resValid)
    {
        // Get the platform res size.
        m_resStatus = platformResourceSize(m_hPlatformRes,&m_resSize);

        cffsCheckPortLayout(m_resSize, eNumPorts, m_portSize);

        // Set the port layout(s) using the sizes returned via the
        // cffsCheckPortLayout, then get the addresses of the ports.
        cffsSetPortLayout(m_cAdr, eNumPorts, m_portSize);

        // Get the cffs server info
        m_infoReq.sizeofStruct = sizeof(m_infoReq);
        cffsGetInfo(&m_infoReq);

        m_findReq.sizeofStruct  = sizeof(m_findReq);
        m_findReq.maxNameLength = eMaxFileName;
        m_findReq.foundName     = m_fileName;
        m_findReq.sizeInBytes   = DEFAULT_PORTSIZE; // for writing

        m_dataReq.sizeofStruct = sizeof(m_dataReq);
        cffsGetPortInfo(m_cAdr, ePortIndex, &m_dataReq);

        m_portAddr[0] = m_dataReq.dataAddr;

        // Object init successful.
        m_bInit = TRUE;
    }
}
// Function: Reset
// Description:

void File::Reset()
{
    m_bFirstCalled   = FALSE;
    m_portBytesInUse = 0;
    m_physOffset     = 0;
    m_nextRead       = 0;
    m_fileSize       = 0;
    m_bytesMoved     = 0;
    m_bEOF           = FALSE;
    m_bOpen = FALSE;

    //m_sAdr = NULL;
    //m_cAdr = NULL;
    m_resSize = 0;
    m_mode    = '\0';

    m_cffsStatus = cffsNoStatus;  // make call to IsOpen == FALSE
    m_resStatus  = resInvalidHandle;

    memset(m_clientAccessRes, 0, sizeof(m_clientAccessRes));
    memset(m_partitionName,   0, sizeof(m_partitionName));
    memset(m_fileName, 0,        sizeof(m_fileName));
}
//------------------------------------------------------------------------------
// Function: Open
// Description:

BOOLEAN File::Open(const char* fileName, File::PartitionType partType, char mode)
{
    if (m_bInit)
    {
        Reset();
        // convert partition enum to string
        strncpy(m_partitionName, partName[partType - eBasePart], eMaxResName );

        strncpy(m_fileName, fileName, eMaxFileName);

        // access mode
        m_mode = (mode == 'r' || mode == 'w') ? mode : '\0';
    }
    m_bOpen = m_bInit;

    return m_bOpen;
}

//------------------------------------------------------------------------------
// Function: Read
// Description:

SIGNED32 File::Read(void *pBuff, UNSIGNED32 size)
{
    UNSIGNED16 i;
    //UNSIGNED32 bytesAvailable;// number of bytes in port available for 'reading' to pBuff
    SIGNED32   bytesRead = 0; // count of bytes copied to pBuff
    SIGNED32   debug_readCnt;

    void*      pDest;    // working-address in pBuff for putting bytes frm port
    UNSIGNED32 destSize; // working-cnt of bytes avail in pBuff

    if ( !m_bOpen || size > MAX_READ_SIZE || m_mode != 'r')
    {
        return -1;
    }
    // Read from cffs for the requested size or EOF, whichever comes first.
    // if fewer bytes in port than requested by size, the function will transfer
    // available bytes from port -> pBuff and issue a read to fetch more bytes from
    // phys->port if available. Loop until the pBuff has been filled or
    // partial (e.g. EOF)

    pDest = pBuff;
    do
    {
        //bytesAvailable = m_portBytesInUse - m_nextRead;
        destSize = (size - ((unsigned char*)pDest - (unsigned char*)pBuff));

        // If port has bytes available for 'reading', transfer as many as
        // the read buffer can accept.
        if(m_portBytesInUse > 0)
        {
            UNSIGNED32 maxBytes = min(destSize, m_portBytesInUse );
            memcpy(pDest, ((unsigned char*)m_portAddr[ePortIndex] +  m_nextRead), maxBytes );
            m_nextRead += maxBytes;
            bytesRead  += maxBytes;
            m_portBytesInUse -= maxBytes;
            debug_readCnt = m_nextRead / size;

            // Move ptr in dest buffer in case this is a partial fill on this pass
            pDest = (void*)((unsigned char*)pDest + maxBytes);
        }
        else // port is empty, try to re-populate from media
        {
            // Call set up function before the first read.
            if ( !m_bFirstCalled )
            {
                m_bFirstCalled = TRUE;
                m_cffsStatus = cffsFirst(m_sAdr,
                                         fileSpecific,
                                         m_partitionName,
                                         m_fileName,
                                         &m_findReq);

                m_fileSize    = m_findReq.sizeInBytes;
                m_physOffset  = 0;
            }

            // If the offset into the physical media is within the file size,
            // read the next block into the port, else flag that current block
            // was the last.
            if (m_physOffset < m_fileSize )
            {
                m_dataReq.sizeofStruct = sizeof(m_dataReq);
                cffsGetPortInfo(m_cAdr, (UNSIGNED32)ePortIndex, &m_dataReq);//TBD still needed in v3.7.1

                m_cffsStatus = cffsSeekX(m_sAdr,
                                         m_cAdr,
                                         ePortIndex,
                                         m_partitionName,
                                         m_fileName,
                                         m_physOffset,
                                         CFFS_OFFSET_NOT_FROM_END,
                                         DEFAULT_PORTSIZE,
                                         CFFS_BLOCKING,
                                         &m_dataReq );

                m_physOffset     += m_dataReq.numBytes; // Increment for next cffsSeekX
                m_nextRead       = 0;                   // Set read offset to start of port
                m_portBytesInUse = m_dataReq.numBytes;
            }
            else
            {
                m_bEOF = TRUE;
            }
        }
    }
    while (bytesRead < size && !m_bEOF);

    bytesRead = (m_cffsStatus == cffsSuccess) ? bytesRead : -1;

    m_bytesMoved += bytesRead;
    return bytesRead;

}
//------------------------------------------------------------------------------
// Function: Write
// Description:

BOOLEAN File::Write(void *pBuff, UNSIGNED32 size)
{
    UNSIGNED32  bytesFree;      // number of bytes in port available for storing out-msg
    UNSIGNED32  bytesToSend;    // number of bytes from 'size' to be written/buffered
    void*       pDestAddr;      // Address in 'port' to buffer next outgoing block.

    static BOOLEAN bFirstTime = TRUE;


    if ( !m_bOpen || size > MAX_WRITE_SIZE || m_mode != 'w')
    {
        return FALSE;
    }

    m_bytesMoved += size;

    // Fill the port before committing(writing)
    bytesToSend = size; // TODO once working use 'size' directly to track outstanding bytes
    do
    {
        bytesFree = m_portSize[ePortIndex] - m_portBytesInUse;
        pDestAddr = (void*)((char*)m_portAddr[ePortIndex] + m_portBytesInUse);

        // Is there enough room in port to fit pBuff?
        if ( bytesToSend <= bytesFree )
        {
            // space is available, copy to port
            memcpy ( pDestAddr, pBuff, bytesToSend);
            m_portBytesInUse += bytesToSend;
            bytesToSend = 0;
            // Assume the 'write' is OK
            m_cffsStatus = cffsSuccess;
        }
        else
        {
            // not enough room in port, copy what we can to port, flush, then
            // buffer the remainder on the next pass.
            memcpy ( pDestAddr, pBuff, bytesFree);
            m_portBytesInUse += bytesFree;

            bytesToSend -= bytesFree;
            pBuff = (void*)((unsigned char*)pBuff + bytesFree);

            // Perform a phys write from port to cffs...
            // (NOTE: Flush will resets m_bytesInUse to zero and set m_cffsStatus)
            Flush();
        }
    }
    while(bytesToSend > (0 && m_cffsStatus == cffsSuccess));

    return (m_cffsStatus == cffsSuccess);
}

//------------------------------------------------------------------------------
// Function: Delete
// Description:

BOOLEAN File::Delete(const char* fileName, File::PartitionType partType)
{
    if ( 0 == strlen(fileName) )
    {
        return FALSE;
    }

    // If file not open, do it now to set up filename and partition name
    if (!IsOpen())
    {
        Open(fileName, partType , m_mode);
    }

    // Delete the file
    m_dataReq.sizeofStruct = sizeof(m_dataReq);
    cffsGetPortInfo(m_cAdr, (UNSIGNED32)ePortIndex, &m_dataReq);
    m_cffsStatus = cffsDelete( m_sAdr, m_cAdr, ePortIndex, m_partitionName,
                               m_fileName, CFFS_BLOCKING, CFFS_FLUSH );

    if (m_cffsStatus != cffsSuccess)
    {
        int bp = 42;
    }
    // Reset this file so no other ops allowed without an 'Open' call
    Reset();
    return (m_cffsStatus == cffsSuccess || m_cffsStatus == cffsFileNotFound );
}

//------------------------------------------------------------------------------
// Function: Flush
// Description:

BOOLEAN File::Flush(void)
{
    BOOLEAN status = FALSE;
    if ( m_bOpen && m_mode == 'w' && m_portBytesInUse > 0)
    {
        cffsGetPortInfo(m_cAdr, ePortIndex, &m_dataReq);

        // Flush is called when the 512-aligned port is full or on final write
        // so there should never be issues with cffs over-writing  on the previous
        // media boundary.
        m_cffsStatus = CFFS_WRITE( m_sAdr,                   // ptr Srv metadata
                                   m_cAdr,                   // ptr client access res.
                                   ePortIndex,               // Data port to write
                                   m_partitionName,          // partition name
                                   m_fileName,               // file to write/create
                                   m_physOffset,             // dest offset into phys file
                                   CFFS_OFFSET_NOT_FROM_END,
                                   m_portBytesInUse,         // # bytes to write
                                   m_portAddr[ePortIndex],   // address of src port
                                   0x0,
                                   0xFF,
                                   CFFS_BLOCKING,
                                   CFFS_NO_TRUNC,
                                   CFFS_NO_FLUSH,
                                   &m_dataReq );

         if(m_cffsStatus == cffsSuccess)
         {
             // m_bytesInUse will either be a full port size or some
             // portion on the final flush.
             m_physOffset += m_portBytesInUse;
             m_portBytesInUse = 0;
             status = TRUE;
         }
     }
     return status;
}

//------------------------------------------------------------------------------
// Function: Close
// Description:

BOOLEAN File::Close(void)
{
    if( m_bOpen )
    {
        Flush(); // Force a phys write.
        Reset();
    }

    return TRUE;
}

//------------------------------------------------------------------------------
// Function: GetFileStatus
// Description:

char* File::GetFileStatus(char* buffer)
{
    sprintf(buffer, "Name: %s Open: %s I/O: %d sAdr: 0x%08x cAdr: 0x%08x",
            m_fileName,
            IsOpen() ? "Yes" : "No",
            m_bytesMoved,
            m_sAdr,
            m_cAdr);

    return buffer;
}
