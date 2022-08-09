////////////////////////////////////////////////////////////////////////////////
// syserrs.h -- SYSERR definitions
//
// @Note Source file from Pulse V1
////////////////////////////////////////////////////////////////////////////////

#ifndef _syserrs_h_INCLUDED_
#define _syserrs_h_INCLUDED_

// SYSERR codes
// ------------
typedef enum
{ SYSERR_NONE         =  0  // "no error" placeholder
, SYSERR_MAX_IDX      =  0  // used to get highest index

// Assert
, SYSERR_ASSERT_FAIL  =  1  // !!! Failed assertion !!!

// I2C
, SYSERR_ACC_I2C_FAIL  // Accelerometer I2C operation failed
, SYSERR_PWR_I2C_FAIL  // Power supply I2C operation failed

// Time Service 
, SYSERR_TIMESVC_FAIL

// Debug UART
, SYSERR_UART_TX_ERR
, SYSERR_UART_COMM_ERR
, SYSERR_UART_FIFO_ERR

// Nordic UART Service
, SYSERR_NUSW_RX_OVF
, SYSERR_NUSW_RX_UNF
, SYSERR_NUSW_TX_ERR

// Console
, SYSERR_CONSOLE_STATE

// ADAE
, SYSERR_ADAE_STATE
, SYSERR_ADAE_BADCMD
, SYSERR_ADAE_ABORT

// App Sequencer
, SYSERR_APP_STATE

// App Data Model
, SYSERR_ADM_ALLOC
, SYSERR_ADM_FREE

// number of syserrs defined
, NUM_SYSERRS
} syserr_t;

// List of Culprits (for assert failures)
// ----------------
typedef enum
{ CULPRIT_UNKNOWN = 0
, CULPRIT_LED_ARG
, NUM_CULPRITS
} culprit_t;

#endif // _syserrs_h_INCLUDED_
