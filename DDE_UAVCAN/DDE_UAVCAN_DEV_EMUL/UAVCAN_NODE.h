#pragma once

#include "stdint.h"
#include "canard.h"

typedef struct
{

	int state;
	int 			(*init)(int);
	int 			(*ms_calc)();
	int 			(*update)();
	long  (*canInit)();
	long  (*canPop)(CanardFrame*);
	long  (*canPush)(CanardFrame*);
} UAVCAN_NODE;

#define UAVCAN_NODE_DEFAULTS {0,\
								UAVCAN_NODE_init,\
								UAVCAN_NODE_1ms_calc,\
								UAVCAN_NODE_update,\
								UAVCAN_NODE_canInit,\
								UAVCAN_NODE_canPop,\
								UAVCAN_NODE_canPush\
								}

int UAVCAN_NODE_init(int node_id);
int UAVCAN_NODE_1ms_calc();
int UAVCAN_NODE_update();

long UAVCAN_NODE_canInit();
long UAVCAN_NODE_canPop(CanardFrame*);
long UAVCAN_NODE_canPush(CanardFrame*);

extern UAVCAN_NODE uavcan_node;
