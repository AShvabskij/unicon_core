#pragma once

#include "libcanard-2/libcanard/canard.h"

/* valid bits in CAN ID for frame formats */
#define CAN_SFF_MASK 0x000007FFU /* standard frame format (SFF) */
#define CAN_EFF_MASK 0x1FFFFFFFU /* extended frame format (EFF) */
#define CAN_ERR_MASK 0x1FFFFFFFU /* omit EFF, RTR, ERR flags */
#define CAN_SFF_ID_BITS		11
#define CAN_EFF_ID_BITS		29

// Transmit Channels
#define APP_TX_FIFO CAN_FIFO_CH2

// Receive Channels
#define APP_RX_FIFO CAN_FIFO_CH1

typedef struct
{

	uint8_t _error_RX;// = 0;
	uint8_t _error_TX;// = 0;
	long  (*init)();
	long  (*canPop)(CanardFrame*);
	long  (*canPush)(CanardFrame*);
} CAN_MCP2518FD;


#define CAN_MCP2518FD_DEFAULTS {0, 0, CAN_MCP2518FD_init,CAN_MCP2518FD_canPop, CAN_MCP2518FD_canPush}

long CAN_MCP2518FD_init();
long CAN_MCP2518FD_canPop(CanardFrame*);
long CAN_MCP2518FD_canPush(CanardFrame*);


