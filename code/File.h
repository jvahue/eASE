#ifndef FILE_H
#define FILE_H

#include <stdlib.h>
#include <cffsapi.h>
#include "alt_stdtypes.h"

class File
{
    public:
    // Constructor
        File();

    // Object management methods
        BOOLEAN Open(const char* fileName,  UINT32 partitionId, const char mode);
        void Reset();

    // Interface methods
        BOOLEAN Read (void *pBuff, UNSIGNED32 size);
        BOOLEAN Write(void *pBuff, UNSIGNED32 size);
        BOOLEAN Delete(void);

        void Close();
        BOOLEAN IsOpen();
        const char* GetFileName() const {return m_fileName;}

    protected:
        enum FileConstants
        {
            eNumPorts = 1,
            eMaxResName = 32,
            eMaxFileName = 128,
            ePortIndex   = 1
        };
        BOOLEAN bFirstReadCall;

        // cffs control structures
        cffsInfoRequest m_infoReq;
        cffsDataRequest m_dataReq;
        cffsFindRequest m_findReq;
        accessStyle     m_accessStyle;

        void       *m_sAdr;       // Ptr to server metadata res
        void       *m_cAdr;       // Ptr to client Access res
        UNSIGNED32 m_resSize;

        resourceStatus  m_resStatus;
        cffsStatus      m_cffsStatus;

        platformResourceHandle m_hPlatformRes;

        UNSIGNED32 m_portSize [eNumPorts];
        void*      m_portAddr [eNumPorts];

        char m_clientAccessRes[eMaxResName]; // Client Access name (CAR)
        char m_partitionName[eMaxResName];   // name of the XXXX-partition where this file resides.
        char m_fileName[eMaxFileName];       // name of the file to be used.

    private:

};
#endif
