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
#define BYTES_TO_DWORDS(x) ((x / sizeof(UINT32))+1)
//#define MAILBOX_DEBUG

#define PORTSIZES (200 * 1024)
#define PORTREAD_SIZE (PORTSIZES - (1 * 1024)) // Don't read thru end of Blk
                                               // Note PORTREAD_SIZE must be 512 aligned
#define DEF_NUMPORTS 1

#define CFFS_FLUSH    TRUE
#define CFFS_NO_FLUSH FALSE

#define CFFS_TRUNC    TRUE
#define CFFS_NO_TRUNC FALSE

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


//------------------------------------------------------------------------------
// Function: Reset
// Description:

void File::Reset()
{
    m_sAdr = NULL;
    m_cAdr = NULL;
    bFirstReadCall = FALSE;

    memset(m_clientAccessRes, 0, sizeof(m_clientAccessRes));
    memset(m_fileName, 0,        sizeof(m_fileName));
    memset(m_partitionName,   0, sizeof(m_partitionName));
}
//------------------------------------------------------------------------------
// Function: Init
// Description:

BOOLEAN File::Open(const char* fileName,
                   UINT32 filePartitionName,
                   const char mode)
{
    UNSIGNED32  i;
    UNSIGNED32  initVal;
    UNSIGNED32* aliveCnt;

    strncpy(m_partitionName, filePartitionName, eMaxResName );

    strncpy(m_clientAccessRes, clientRes, eMaxResName);
    //strncpy(m_clientAccessRes, "CM-CAR", eMaxResName);

    strncpy(m_fileName, fileName, eMaxFileName);


    // Init m_portAddr[] to 0 and the portsize according to constructor param.
    for (i = 0; i < eNumPorts; i++)
    {
        m_portAddr[i] = 0;
        m_portSize[i] = portSize;
    }

    // Attach to the server's metadata resource
    m_resStatus = attachPlatformResource("",
                                          SRV_METADATA_RES,
                                          &m_hPlatformRes,
                                          &m_accessStyle,
                                          &m_sAdr);

    // Get a pointer to the alive counter,
    // wait for increment to show the
    aliveCnt = cffsAliveCounter( &m_sAdr );
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

        cffsGetInfo(&m_infoReq);

        cffsCheckPortLayout(m_resSize, eNumPorts, m_portSize);

        // Set the port layout(s) using the sizes returned via the
        // cffsCheckPortLayout, then get the addresses of the ports.
        cffsSetPortLayout(m_cAdr, eNumPorts, m_portSize);

        // Get the cffs server info
        m_infoReq.sizeofStruct = sizeof(m_infoReq);
        m_dataReq.sizeofStruct = sizeof(m_dataReq);

        m_findReq.sizeofStruct  = sizeof(m_findReq);
        m_findReq.maxNameLength = eMaxFileName;
        m_findReq.foundName     = m_fileName;

        cffsGetPortInfo(m_cAdr, ePortIndex, &m_dataReq);
        m_portAddr[i] = m_dataReq.dataAddr;
    }



    return (m_cffsStatus == cffsSuccess);
}

//------------------------------------------------------------------------------
// Function: Read
// Description:

BOOLEAN File::Read(void *pBuff, UNSIGNED32 size)
{
    UNSIGNED16 i;

    if ( !bFirstReadCall )
    {
        cffsGetPortInfo(m_cAdr, ePortIndex, &m_dataReq);//TBD still needed in v3.7.1
        m_cffsStatus = cffsFirst(m_sAdr, fileIterate, m_partitionName,
                                 m_fileName, &m_findReq);
        bFirstReadCall = TRUE;
    }
    else
    {
        cffsGetPortInfo(m_cAdr, ePortIndex, &m_dataReq);//TBD still needed in v3.7.1

        m_cffsStatus = cffsSeekX(m_sAdr, m_cAdr, ePortIndex,
                                 m_partitionName, m_fileName, 0, CFFS_OFFSET_NOT_FROM_END,
                                 m_findReq.sizeInBytes, CFFS_BLOCKING, &m_dataReq );
    }

    return (m_cffsStatus == cffsSuccess);

}
//------------------------------------------------------------------------------
// Function: Init
// Description:

BOOLEAN File::Write(void *pBuff, UNSIGNED32 size)
{
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


