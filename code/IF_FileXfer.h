/*
 * IF_FileXfer.h
 *
 *  Created on: Aug 7, 2013
 *      Author: p916214
 */

#ifndef IF_FILEXFER_H_
#define IF_FILEXFER_H_

/*****************************************************************************/
/* Local Defines                                                             */
/*****************************************************************************/
#define MAX_MSG_LEN   512         // max length of mailbox message (bytes)
#define CM_FILE_NAME_LEN  128     // max length of file name for comm mgr

/*****************************************************************************/
/* Local Typedefs                                                            */
/*****************************************************************************/
// command message ID
enum CM_XFR_MSG_ID
{
  CM_ID_XFR       = 1,      // start file transfer        ->CM
  CM_ID_ACK       = 2,      // start transfer ack         <-CM
  CM_ID_CONFIRM   = 3,      // file transfer confirmation <-CM
  CM_ID_CRC_VAL   = 4,      // CRC validation             ->CM
  CM_ID_COMPLETE  = 5       // file transfer complete     <-CM
} ;

// command ACK status
enum CM_CMD_STATUS
{
  CM_ACK      = 1,      // Transfer Request ACK
  CM_NACK     = 2,      // Transfer Request NACK
  CM_XFR_ACK  = 0,      // Transfer Confirmation ACK
  CM_XFR_NACK = 1,      // Transfer Confirmation NACK
  CM_CRC_ACK  = 0,      // Transfer CRC ACK
  CM_CRC_NACK = 1,      // Transfer CRC NACK
  CM_CRC_NACK_LAST = 2  // Transfer CRC NACK Last Retry
} ;

// query file transfer status
enum CM_ACK_INFO
{
  CM_FILE_XFR           = 1,      // file being transferred
  CM_QUEUED_MS_OK       = 2,      // file in transfer queue - MS OK
  CM_QUEUED_MS_OFFLINE  = 3,      // file in transfer queue - MS Offline
  CM_XFR_FILE_MISMATCH  = 4,      // request filename doesn't match current xfr
  CM_INVALID_FILE       = 5       // file not accessable or non-existant
} ;

//----------------------------------------

// current file transfer status
enum FILE_XFR_INFO
{
  XFR_NONE = 0,           // no file transfer yet (initial state)
  XFR_COMPLETE,           // file transfer complete
  XFR_IN_PROGRESS,        // file being transferred
  XFR_QUEUED_MS_OK,       // file in transfer queue - MS OK
  XFR_QUEUED_MS_OFFLINE,  // file in transfer queue - MS Offline
  XFR_FILE_MISMATCH,      // request filename doesn't match current xfr
  XFR_INVALID_FILE,       // file not accessable or non-existant
  XFR_CM_MAILBOX_ERR,     // Comm Mgr mailbox error
  XFR_WAIT_FOR_ACK        // waiting for request ACK
} ;

// file transfer execution state
enum FILE_XFR_STATE
{
  XFR_INIT  = 0,
  XFR_READY,
  XFR_BUSY
} ;

// transfer protocol execution state
enum FILE_XFR_WAIT_STATE
{
  XFR_START = 0,
  XFR_WAIT_ACK,
  XFR_WAIT_CONFIRM,
  XFR_WAIT_COMPLETE
} ;

struct FILE_RCV_MSG
{
  UINT8     msgId;                  // message ID (FILE_XFR_MSG_ID)
  CHAR      msg[MAX_MSG_LEN - 1];   // message
};

// file transfer/status request (send)
struct FILE_XFR_MSG
{
  UINT8     msgId;                      // message ID (FILE_XFR_MSG_ID)
  CHAR      filename[CM_FILE_NAME_LEN]; // filename to transfer
  UINT8     xfrOverride;                // override current file transfer
  UINT8     pad[2];                     // pad bytes
};

// file transfer request acknowledgement (receive)
struct FILE_ACK_MSG
{
  UINT8     msgId;                      // message ID (FILE_XFR_MSG_ID)
  CHAR      filename[CM_FILE_NAME_LEN]; // filename to transfer
  UINT8     ackStatus;                  // ACK status
  UINT8     ackInfo;                    // ACK status info
};

// file transfer confirmation (receive)
struct FILE_CONFIRM_MSG
{
  UINT8     msgId;                      // message ID (FILE_XFR_MSG_ID)
  CHAR      filename[CM_FILE_NAME_LEN]; // filename to transfer
  UINT8     xfrResult;                  // transfer result
  UINT32    msCrc;                      // CRC from microserver
};

// file transfer CRC validation (send)
struct FILE_CRC_VAL_MSG
{
  UINT8     msgId;                      // message ID (FILE_XFR_MSG_ID)
  CHAR      filename[CM_FILE_NAME_LEN]; // filename to transfer
  UINT8     crcValid;                   // crc check result
};

// file transfer complete (receive)
struct FILE_COMPLETE_MSG
{
  UINT8     msgId;                      // message ID (FILE_XFR_MSG_ID)
  CHAR      filename[CM_FILE_NAME_LEN]; // filename to transfer
};




#endif /* IF_FILEXFER_H_ */
