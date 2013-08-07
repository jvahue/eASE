#ifndef FILE_H
#define FILE_H

#include <stdlib.h>
#include <cffsapi.h>
#include "alt_stdtypes.h"
#include "SecComm.h"      // for stream size matching.

#define MAX_WRITE_SIZE eSecCharDataSize  // max size of PySte command
#define MAX_READ_SIZE  eSecStreamSize    // max size of



class File
{
    public:
        enum PartitionType
        {
            ePartCmProc = 301,
            eBasePart   = ePartCmProc,
            ePartAdrf   = 302
        };
    // Constructor
        File();

    // Object management methods
        BOOLEAN Open(const char* fileName, PartitionType partType, char mode);
        void Reset();

    // Interface methods
        SIGNED32 Read (void *pBuff, UNSIGNED32 size);
        BOOLEAN Write(void *pBuff, UNSIGNED32 size);
        BOOLEAN Flush(void);
        BOOLEAN Delete(const char* fileName, File::PartitionType partType);
        BOOLEAN Close(void);

    // Accessors
        const char* GetFileName() {return m_fileName;}
        BOOLEAN     IsOpen()
        {
            return m_bOpen;
        }

        char* GetFileStatus(char* buffer);

    protected:
        enum FileConstants
        {
            eNumPorts    = 1,
            eMaxResName  = 32,
            eMaxFileName = 128,
            ePortIndex   = 0
        };

        // Object mgmt attributes
        BOOLEAN    m_bFirstCalled;   // used to control read calls(cffsFirst,cffsSeekX)
        UNSIGNED32 m_portBytesInUse; // # bytes currently buffered in port, awaiting write/read
        UNSIGNED32 m_physOffset;     // Offset into phys file for read/writing.
        UNSIGNED32 m_nextRead;       // Index port to fetch next 'n' bytes
        UNSIGNED32 m_fileSize;       // size of file being read(bytes).
        UINT32     m_bytesMoved; // the number of bytes read or written since opening
        BOOLEAN    m_bEOF;
        BOOLEAN    m_bOpen;

        // cffs control structures
        cffsInfoRequest m_infoReq;
        cffsDataRequest m_dataReq;
        cffsFindRequest m_findReq;
        accessStyle     m_accessStyle;

        void       *m_sAdr;       // Ptr to server metadata res
        void       *m_cAdr;       // Ptr to client Access res
        UNSIGNED32 m_resSize;
        char       m_mode;

        resourceStatus  m_resStatus;
        cffsStatus      m_cffsStatus;

        platformResourceHandle m_hPlatformRes;

        UNSIGNED32 m_portSize [eNumPorts];
        void*      m_portAddr [eNumPorts];

        char m_clientAccessRes[eMaxResName]; // Client Access name (CAR)
        char m_partitionName  [eMaxResName]; // name of the XXXX-partition where this file resides.
        char m_fileName       [eMaxFileName];// name of the file to be used.

    private:

};
#endif
