// File class implementation

/*****************************************************************************/
/* Compiler Specific Includes                                                */
/*****************************************************************************/


/*****************************************************************************/
/* Software Specific Includes                                                */
/*****************************************************************************/
#include <deos.h>
#include <videobuf.h>
#include <string.h>
#include <stdlib.h>

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

#define DEFAULT_PORTSIZE  512  //(200 * 1024)
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
{
    Reset();
}
// Function: Reset
// Description:

void File::Reset()
{
    m_sAdr = NULL;
    m_cAdr = NULL;
    m_bFirstCalled = FALSE;
    m_bytesInPort = 0;
    m_physOffset  = 0;
    m_fileSize    = 0;
    m_bEOF        = FALSE;
    m_nextRead    = 0;

    memset(m_clientAccessRes, 0, sizeof(m_clientAccessRes));
    memset(m_fileName, 0,        sizeof(m_fileName));
    memset(m_partitionName,   0, sizeof(m_partitionName));
}
//------------------------------------------------------------------------------
// Function: Open
// Description:

BOOLEAN File::Open(const char* fileName, PartitionType partType, char mode)
{
    UNSIGNED32  i;
    UNSIGNED32  initVal;
    UNSIGNED32* aliveCnt;
    const char* partName[] = {"CM-partition", "ADRF-partition", };

    Reset();
    // convert partition enum to string
    i = partType - eBasePart;
    strncpy(m_partitionName, partName[i], eMaxResName );

    strncpy(m_fileName, fileName, eMaxFileName);

    // access mode
    m_mode = (mode == 'r' || mode == 'w') ? mode : '\0';
    //TODO check for invalid mode

    strncpy(m_clientAccessRes, "CM-CAR", eMaxResName);

    // Open m_portAddr[] to 0 and the portsize according to constructor param.
    for (i = 0; i < eNumPorts; i++)
    {
        m_portAddr[i] = 0;
        m_portSize[i] = DEFAULT_PORTSIZE;
    }

    // Attach to the server's metadata resource
    m_resStatus = attachPlatformResource("",
                                          SRV_METADATA_RES,
                                          &m_hPlatformRes,
                                          &m_accessStyle,
                                          &m_sAdr);

    // Get a pointer to the alive counter,
    // wait for increment to show the
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

    // Setup Ports
    // Attach client access resource
    m_resStatus = attachPlatformResource("", m_clientAccessRes,
                                         &m_hPlatformRes,
                                         &m_accessStyle,
                                         &m_cAdr);
    if (m_resStatus != resValid)
    {
        i = 42; // place to put a BP if needed.
    }
    else
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
    }

    return (m_cffsStatus == cffsSuccess);
}

//------------------------------------------------------------------------------
// Function: Read
// Description:

SIGNED32 File::Read(void *pBuff, UNSIGNED32 size)
{
    UNSIGNED16 i;
    UNSIGNED32 bytesAvailable;// number of bytes in port available for transfering to pBuff
    SIGNED32   bytesRead = 0; // count of bytes copied to pBuff

    void*      pDest;    // working-address in pBuff for putting bytes frm port
    UNSIGNED32 destSize; // working-cnt of bytes avail in pBuff

    if (size > MAX_READ_SIZE)
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
        bytesAvailable = m_bytesInPort - m_nextRead;
        destSize = (size - (&pDest - &pBuff));

        if(bytesAvailable > 0)
        {
            UNSIGNED32 limit = min(destSize, bytesAvailable );
            memcpy(pDest, (m_portAddr[ePortIndex] +  m_nextRead), limit );
            m_nextRead += limit;
            bytesRead  += limit;
            m_bytesInPort -= limit;
        }
        else // port is empty, try to re-fill from media
        {
            if ( !m_bFirstCalled )
            {
                m_cffsStatus = cffsFirst(m_sAdr,
                                         fileSpecific,
                                         m_partitionName,
                                         m_fileName,
                                         &m_findReq);
                m_bFirstCalled = TRUE;
                m_fileSize    = m_findReq.sizeInBytes;
                m_physOffset  = 0;
            }

            if (m_physOffset < m_fileSize )
            {
                m_dataReq.sizeofStruct = sizeof(m_dataReq);
                cffsGetPortInfo(m_cAdr, ePortIndex, &m_dataReq);//TBD still needed in v3.7.1

                m_cffsStatus = cffsSeekX(m_sAdr,
                                         m_cAdr,
                                         ePortIndex,
                                         m_partitionName,
                                         m_fileName,
                                         m_physOffset,
                                         CFFS_OFFSET_NOT_FROM_END,
                                         m_findReq.sizeInBytes,
                                         CFFS_BLOCKING,
                                         &m_dataReq );

                m_physOffset  += m_dataReq.numBytes;
                m_nextRead    = 0;
                m_bytesInPort = m_dataReq.numBytes;
            }
            else
            {
                m_bEOF = TRUE;
            }
        }
    }
    while (bytesRead < size && !m_bEOF);

    bytesRead = (m_cffsStatus == cffsSuccess) ? bytesRead : -1;

    return bytesRead;

}
//------------------------------------------------------------------------------
// Function: Write
// Description:

BOOLEAN File::Write(void *pBuff, UNSIGNED32 size)
{
    UNSIGNED32  bytesAvailable; // number of bytes available for storing in port data struct
    UNSIGNED32  bytesToSend;    // number of bytes from 'size' to be written/buffered
    void*       pDestAddr;      // Address in 'port' to buffer next block.

    // Fill the port before committing(writing)
    bytesToSend = size; // TODO once working use 'size' directly to track outstanding bytes
    do
    {
        bytesAvailable = m_portSize[ePortIndex] - m_bytesInPort;
        pDestAddr      = (void*)((UNSIGNED32*)m_portAddr[ePortIndex] + m_bytesInPort);

        // Is there enough room in port to fit pBuff?
        if ( bytesToSend <= bytesAvailable )
        {
            // space is available, copy to port
            memcpy ( pDestAddr, pBuff, bytesToSend);
            m_bytesInPort += bytesToSend;
            bytesToSend = 0;
        }
        else // not enough room, copy what we can to port, flush then buffer the remainder
        {
            memcpy ( pDestAddr, pBuff, bytesAvailable);
            bytesToSend -= bytesAvailable;

            // Perform a phys write from port to cffs...
            // (NOTE: Flush will resets m_bytesInUse to zero)
            Flush();
        }
    }
    while(bytesToSend > 0);

    return (m_cffsStatus == cffsSuccess);
}

//------------------------------------------------------------------------------
// Function: Delete
// Description:

BOOLEAN File::Delete(void)
{
    cffsGetPortInfo(m_cAdr, ePortIndex, &m_dataReq);//TBD still needed in v3.7.1
    // Delete the file
    m_cffsStatus = cffsDelete( m_sAdr, m_cAdr, ePortIndex, m_partitionName,
                               m_fileName, CFFS_BLOCKING, CFFS_FLUSH );

    if (m_cffsStatus != cffsSuccess)
    {
        int bp = 42;
    }
    return (m_cffsStatus == cffsSuccess);
}

//------------------------------------------------------------------------------
// Function: Flush
// Description:

BOOLEAN File::Flush(void)
{
    if (m_bytesInPort > 0)
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
                                   m_bytesInPort,            // # bytes to write
                                   m_portAddr[ePortIndex],  // address of src port
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
             m_physOffset += m_bytesInPort;
             m_bytesInPort = 0;
         }
     }
     return (m_cffsStatus == cffsSuccess);
}

BOOLEAN File::Close(void)
{
    Flush();                      // Force a phys write.
    m_cffsStatus = cffsNoStatus;  // make call to IsOpen == FALSE
    return TRUE;
}

