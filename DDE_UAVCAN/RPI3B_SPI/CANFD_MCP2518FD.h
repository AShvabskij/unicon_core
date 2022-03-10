/*
 * MCP2518FD.h
 *
 *  Created on: 29 марта 2021 г.
 *      Author: Ko
 */

#ifndef HAL_CANFD_MCP2518FD_H_
#define HAL_CANFD_MCP2518FD_H_

#include <stdbool.h>

 // DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility
extern "C" {
#endif
    // DOM-IGNORE-END
// *****************************************************************************
// *****************************************************************************
// Section: Type Definitions
// *****************************************************************************
// *****************************************************************************

//! Use RX and TX Interrupt pins to check FIFO status
//#define APP_USE_RX_INT
//#define APP_USE_TX_INT
//#define APP_USE_RX_TX_INT


//! Blink LEDs after changing bit time
//#define APP_BLINK_LED_AFTER_SETBITTIME

//TODO was 50
#define MAX_TXQUEUE_ATTEMPTS 1

// Switches
//#define APP_DEBOUNCE_TIME  100
//#define APP_SWITCH_PRESSED  false
//#define APP_SWITCH_RELEASED true

// Special LEDs
//#define APP_INIT_LED    APP_LED_D8
//#define APP_TX_LED      APP_LED_D2
//#define APP_RX_LED      APP_LED_D3
//#define APP_TX_JPG_LED  APP_LED_D4
//#define APP_RX_JPG_LED  APP_LED_D5

//#define APP_LED_TIME    50000

// Message IDs
#define FILE_START_ID 0xd0
#define FILE_STOP_ID  0xdf
#define FILE_DATA_ID  0xda

#define M1_ADC_SID    		0x55 
#define M2_CMD_LOGIC_SIS    0x200
#define M3_MEAS_SID    		0x300

#define M3_CMD_LOGIC_OUT_SID	0x555

#define TX_REQUEST_ID       0x400
#define TX_RESPONSE_ID      0x401
#define LED_STATUS_ID       0x402
#define BITTIME_SET_ID      0x403

//#define BITTIME_CFG_GET_ID  0x600
//#define BITTIME_CFG_125K_ID 0x601
//#define BITTIME_CFG_250K_ID 0x602
//#define BITTIME_CFG_500K_ID 0x603
//#define BITTIME_CFG_1M_ID   0x604

// Transmit Channels
#define APP_TX_FIFO CAN_FIFO_CH2

// Receive Channels
#define APP_RX_FIFO CAN_FIFO_CH1



// Payload

typedef struct {
    bool On;
    uint8_t Dlc;
    bool ModeRandData;
    uint8_t Counter;
    uint8_t Delay;
    bool BoudRateSwitched;
} APP_Payload;


// *****************************************************************************

/* Application states

  Summary:
    Application states enumeration

  Description:
    This enumeration defines the valid application states.  These states
    determine the behavior of the application at various times.
 */

typedef enum {
    // Initialization
    APP_STATE_INIT = 0,
    APP_STATE_REQUEST_CONFIG,
    APP_STATE_WAIT_FOR_CONFIG,
    APP_STATE_INIT_TXOBJ,

    // POR signaling
    APP_STATE_FLASH_LEDS,

    // Transmit and Receive
    APP_STATE_TRANSMIT,
    APP_STATE_RECEIVE,
    APP_STATE_PAYLOAD,

    // Switch monitoring
    APP_STATE_SWITCH_CHANGED
} APP_STATES;


// *****************************************************************************

/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    Application strings and buffers are be defined outside this structure.
 */

typedef struct {
    /* The application's current state */
    APP_STATES state;

    /* TODO: Define any additional data used by the application. */


} APP_DATA;


typedef struct
{

	char		tx_buff[64];
	char 		rx_buff[64];
	APP_STATES state;
	//HAL_SPI * spi_module;
	void 			(*init)();
	int 			(*update)();
} CANFD_MCP2528FD;

//LOAD_FLASH load_flash;

//Init for  LOAD_FLASH
#define CANFD_MCP2528FD_DEFAULTS {{0},{0},\
				APP_STATE_INIT,\
				CANFD_MCP2528FD_Init,\
				CANFD_MCP2528FD_Update }

void CANFD_MCP2528FD_Init();
int  CANFD_MCP2528FD_Update();


APP_STATES APP_ReceiveMessage_Tasks();


extern CANFD_MCP2528FD CANFD;


#endif /* HAL_CANFD_MCP2518FD_H_ */

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END