/******************************************************************************
            Copyright (C) 2013 Knowlogic Software Corp.
         All Rights Reserved. Proprietary and Confidential.

File:        SecComm.cpp

Description: This file implements the SEC Communications

VERSION
$Revision: $  $Date: $

******************************************************************************/


/*****************************************************************************/
/* Compiler Specific Includes                                                */
/*****************************************************************************/
#include <deos.h>
#include <socketapi.h>
#include <lwip-socket.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#include <transport.h>

/*****************************************************************************/
/* Software Specific Includes                                                */
/*****************************************************************************/
#include "SecComm.h"


/*****************************************************************************/
/* Local Defines                                                             */
/*****************************************************************************/

/*****************************************************************************/
/* Local Typedefs                                                            */
/*****************************************************************************/

/*****************************************************************************/
/* Local Variables                                                           */
/*****************************************************************************/

/*****************************************************************************/
/* Constant Data                                                             */
/*****************************************************************************/
static const CHAR* conSts[] = {
    "No Socket",
    "Wait Conn",
    "No Conn",
    "Connected",
};

/*****************************************************************************/
/* Local Function Prototypes                                                 */
/*****************************************************************************/

/*****************************************************************************/
/* Public Functions                                                          */
/*****************************************************************************/

/*****************************************************************************/
/* Class Definitions                                                         */
/*****************************************************************************/
SecComm::SecComm()
    : m_isValid(FALSE)
    , m_connected(FALSE)
    , m_port(eSecPortNumber)
    , m_lastSequence(0)
    , forceConnectionClosed(FALSE)
    , isRxing(false)
    , m_rspType(eRspWait)
    , m_cmdRequest(0)
    , m_cmdServiced(0)
    , m_cmdHandlerName(NULL)
    , m_rxCount(0)        // how many bytes have come in
    , m_txCount(0)        // how many bytes have gone out
    , m_connState(eConnNoSocket)
    , m_socket(-1)
    , m_clientSocket(-1)
    , m_acceptCount(0)
{
    memset((void*)&m_request, 0, sizeof(m_request));
    memset((void*)&m_bufRqst, 0, sizeof(m_bufRqst));

    memset((void*)&m_response, 0, sizeof(m_response));
    memset((void*)&m_snsNames, 0, sizeof(m_snsNames));

    memset((void*)m_errMsg, 0, sizeof(m_errMsg));
    memset((void*)m_ipPort, 0, sizeof(m_ipPort));

}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Function: Create
// Description: Create a Socket Rx thread to recieve the cmds from PySte
void SecComm::Run()
{
    // spawn the thread
    AseThread::Launch("SecCommRx", "StdThreadTemplate");

    if (m_state == eError)
    {
        m_isValid = FALSE;
    }
    else
    {
        m_isValid = TRUE;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Function: OpenConnection
// Description: Create a Socket connection
void SecComm::OpenConnection()
{
    if (SetupTransport( m_pysteHandle, (CHAR*)"pySteConnection") == transportSuccess)
    {
        // Create a TCP socket (SOCK_STREAM); default set up is "blocking"
        m_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (m_socket != SOCKET_ERROR)
        {
            // Bind the socket to port 23
            m_socketAddr.sin_family = AF_INET;
            m_socketAddr.sin_port = htons(m_port);  // port number must be in network byte order
            m_socketAddr.sin_addr = INADDR_ANY;
            if (bind(m_socket, (sockaddr*)&m_socketAddr, sizeof(sockaddr_in)) == SOCKET_ERROR)
            {
                sprintf(m_errMsg, "Error binding TCP socket");
                m_isValid = FALSE;
            }
            else
            {
                // Listen for a connection request from a client
                listen(m_socket, 1);
                m_isValid = TRUE;
            }
        }
        else
        {
            sprintf(m_errMsg, "Error creating TCP socket");
            m_isValid = FALSE;
        }
    }
    else
    {
       // Error Message was set in SetupTransport
       m_isValid = FALSE;
    }
 }

///////////////////////////////////////////////////////////////////////////////////////////////////
// Function: Process
// Description: Rx Thread processing
void SecComm::Process()
{
    if (m_isValid)
    {
        const int MAX_RX = 4096;
        char buffer[MAX_RX];
        int rxed = 0;
        int result = -1;
        int cbBytesRet = 0;
        int size = sizeof(SecRequest);
        int socketAddrLen = sizeof(sockaddr_in);

        // open the socket connection
        OpenConnection();

        // infinite loop to try and get a connection then read bytes from it
        while (1)
        {
            if (forceConnectionClosed)
            {
                forceConnectionClosed = FALSE;
                ResetConn();
                waitUntilNextPeriod();
            }

            // if we don't have a client try and get one
            if (m_connState != eConnConnected)
            {
                m_connState = eConnWait;
                m_acceptCount += 1;
                m_clientSocket = accept(m_socket, (sockaddr*)&m_socketAddr, &socketAddrLen);

                if (m_clientSocket != SOCKET_ERROR)
                {
                    m_connState = eConnConnected;
                    forceConnectionClosed = false;
                }
                else
                {
                    // get last error
                    socketErrorType* lastErr;
                    lastErr = socketTransportLastError();
                    sprintf(m_errMsg, "Error accept - Rtn: %d, Err: %d",
                            lastErr->accept.returnValue, lastErr->accept.errorValue);

                    m_connState = eConnNotConnected;
                }
            }

            rxed = 0;
            cbBytesRet = 0;
            while (m_connState == eConnConnected && cbBytesRet != SOCKET_ERROR && rxed < size && !forceConnectionClosed)
            {
                isRxing = true;
                cbBytesRet = recv(m_clientSocket, &buffer[rxed], MAX_RX-rxed, 0);
                isRxing = false;

                if (cbBytesRet != SOCKET_ERROR && cbBytesRet != 0)
                {
                    rxed += cbBytesRet;
                    m_rxCount += cbBytesRet;
                    forceConnectionClosed = false;
                }
                else if (cbBytesRet == 0)
                {
                    m_connState = eConnNotConnected;
                }
            }

            if (m_connState == eConnConnected && cbBytesRet != SOCKET_ERROR)
            {
                if (cbBytesRet > 0)
                {
                    CheckCmd( buffer, size);
                }
            }
            else if (!forceConnectionClosed)
            {
                socketErrorType* lastErr;
                lastErr = socketTransportLastError();
                sprintf(m_errMsg, "Error recv - Rtn: %d, Err: %d",
                        lastErr->recv.returnValue, lastErr->recv.errorValue);

                ResetConn();
                waitUntilNextPeriod();
            }
        }
    }

    m_state = eComplete;
}

//-------------------------------------------------------------------------------------------------
void SecComm::CheckCmd(const char* buffer, const int size)
{
    UINT8 errCode = 0;
    memcpy( &m_bufRqst, buffer, size);

    // got some data, verify the contents
    if ( m_bufRqst.header1 == eSecAseH1 && m_bufRqst.header2 == eSecAseH2 && m_bufRqst.size == size)
    {
        UINT32 checksum = Checksum(&m_bufRqst, size);

        if ( checksum == m_bufRqst.checksum)
        {
            // copy the temp buffer into the request buffer
            m_request = m_bufRqst;
            IncCmdRequest();

            // wait for the command to be serviced - main will handles unknowns
            while (IsCmdAvailable())
            {
                waitUntilNextPeriod();
                // check for Tx ready
                SendResponse();
            }

            // check for tx ready
            SendResponse();
        }
        else
        {
            sprintf(m_errMsg, "Checksum Err: A/C 0x%08x/0x%08x", m_bufRqst.checksum, checksum);
            errCode = eChecksumError;
            SendAny( (const char*)&errCode, 1);
        }
    }
    else
    {
        sprintf(m_errMsg, "Header Mismatch A/E 0x%08x/0x%08x 0x%08x/0x%08x Size: %d/%d",
                m_bufRqst.header1, eSecAseH1,
                m_bufRqst.header2, eSecAseH2,
                m_bufRqst.size, size);
        errCode = eHeaderError;
        SendAny( (const char*)&errCode, 1);
    }
}

//-------------------------------------------------------------------------------------------------
void SecComm::ResetConn()
{
    m_connState = eConnNotConnected;
    closesocket(m_clientSocket);

    m_clientSocket = -1;
    m_rxCount = 0;
    m_txCount = 0;
}

//-------------------------------------------------------------------------------------------------
void SecComm::SendResponse()
{
    if ( m_rspType != eRspWait)
    {
        if (m_rspType == eRspNormal)
        {
            // send rsp payload
            m_response.header1 = eAseSecH1;
            m_response.header2 = eAseSecH2;
            m_response.size = sizeof(m_response);
            m_response.sequence = m_request.sequence;
            m_response.checksum = Checksum( &m_response, sizeof( m_response));
            SendAny( (char*)&m_response, sizeof( m_response));
            memset((void*)&m_response, 0, sizeof(m_response));
        }
        else if (m_rspType == eRspSensors)
        {
            // send sensor payload
            m_snsNames.header1 = eAseSecH1;
            m_snsNames.header2 = eAseSecH2;
            m_snsNames.size = sizeof(m_snsNames);
            m_snsNames.sequence = m_request.sequence;
            m_snsNames.checksum = Checksum( &m_snsNames, sizeof( m_snsNames));
            SendAny( (char*)&m_snsNames, sizeof( m_snsNames));
            memset((void*)&m_snsNames, 0, sizeof(m_snsNames));
       }

        m_rspType = eRspWait;
    }
}

//-------------------------------------------------------------------------------------------------
void SecComm::SendAny(const char* data, UINT32 size)
{
    int sts = send(m_clientSocket, data, size, 0);
    if (sts == SOCKET_ERROR)
    {
        socketErrorType* lastErr;
        lastErr = socketTransportLastError();
        sprintf(m_errMsg, "Error send - Rtn: %d, Err: %d",
                lastErr->send.returnValue, lastErr->send.errorValue);

        m_connState = eConnNotConnected;
        closesocket(m_clientSocket);

        m_clientSocket = -1;
        m_rxCount = 0;
        m_txCount = 0;
    }
    else
    {
        m_txCount += size;
    }
}

//**************************************************************************************************
// Function used to set up the mailbox transport for a network connection.
// The steps for setting up the transport layer and establishing a connection with the Network
// Server are identical for UDP and TCP.  This example uses one MTL configuration file to configure
// both connections (UDP and TCP). Within Deos, you are only allowed one MTL config file per process.
// For details on the format and content of the config file refer to Chapter 3. Configuration File
// in the User Guide for the Deos Mailbox Transport Library.
//**************************************************************************************************
INT32 SecComm::SetupTransport(clientConnectionHandleType &connectionHandle, CHAR* connectionId)
{
    INT32 setupStatus;
    int   setupError;
    void * sendBuffer;
    DWORD bufferSizeInBytes;

    setupStatus = socketTransportInitialize("mailbox-transport.config",
                                            "transportConfigurationId",
                                            (DWORD)waitIndefinitely,
                                            &setupError);
    if (setupStatus != transportSuccess)
    {
        sprintf(m_errMsg, "socketTransportInitialize returned 0x%08x, error: %d",
                setupStatus, setupError);
    }
    else
    {
        setupStatus = socketTransportClientInitialize((DWORD)waitIndefinitely,
                                                      &setupError);
        if (setupStatus != transportSuccess)
        {
            sprintf(m_errMsg,
                    "socketTransportClientInitialize returned 0x%08x, error: %d",
                    setupStatus, setupError);
        }
        else
        {
            setupStatus = socketTransportCreateConnection(connectionId,
                                                          (DWORD)waitIndefinitely,
                                                          COMPATIBILITY_ID_2,
                                                          &connectionHandle,
                                                          &sendBuffer,
                                                          &bufferSizeInBytes,
                                                          &setupError);
            if (setupStatus != transportSuccess)
            {
                sprintf(m_errMsg, "socketTransportCreateConnection returned 0x%08x, error: %d",
                        setupStatus, setupError);
            }
            else
            {
                setupStatus = socketTransportSetConnectionForThread( currentThreadHandle(),
                                                                     connectionHandle,
                                                                     (DWORD)waitIndefinitely,
                                                                     &setupError);
                if (setupStatus != transportSuccess)
                {
                    sprintf(m_errMsg, "socketTransportSetConnectionForThread returned 0x%08x, error: %d",
                            setupStatus, setupError);

                }
            }
        }
    }

    return setupStatus;
}

//--------------------------------------------------------------------------------------------------
UINT32 SecComm::Checksum( void* ptr, int size)
{
    UINT32 checksum = 0;
    unsigned char* buffer = (unsigned char*)ptr;

    // exclude the checksum
    for ( int i = 0; i < size-4; ++i)
    {
        checksum += buffer[i];
    }
    return checksum;
}

//--------------------------------------------------------------------------------------------------
const CHAR* SecComm::GetSocketInfo()
{
    int sinAddr = htonl(m_socketAddr.sin_addr);

    int p4 = sinAddr & 0xff;
    int p3 = (sinAddr >> 8) & 0xff;
    int p2 = (sinAddr >> 16) & 0xff;
    int p1 = (sinAddr >> 24) & 0xff;

    sprintf(m_ipPort, "%s %d/%d(%d) %d.%d.%d.%d:%d Cmd: %d/%d",
            conSts[m_connState],
            m_socket, m_clientSocket, m_acceptCount,
            p1, p2, p3, p4, eSecPortNumber,
            m_cmdRequest, m_cmdServiced);
    return m_ipPort;
}

//--------------------------------------------------------------------------------------------------
void SecComm::ErrorMsg( char* format, ...)
{
    char buffer[1024];

    va_list args;
    va_start( args, format);
    vsprintf( buffer, format, args );
    va_end(args);

    int len = (int)strlen( buffer);
    for ( int i=0; i < len; ++i)
    {
        if ( buffer[i] == '\t' || buffer[i] == '\n')
        {
            buffer[i] = ' ';
        }
    }

    if ( buffer[len-1] == ' ')
    {
        buffer[len-1] = '\0';
    }

    strncpy( m_response.errorMsg, buffer, eSecErrorMsgSize);
}

