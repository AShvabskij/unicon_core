#pragma once

//#include "DDE_UAVCAN.h"
#include "libcanard-2/libcanard/canard.h"
//#include "interface_CAN.h"

///(interface_CAN* interface_CAN, uint8_t node_ID);






#ifdef __cplusplus
extern "C"
{
#endif



//typedef struct
//{
//	unsigned long id;
//	unsigned char data[64];
//	unsigned char dlc;
//} TCAN_MSG;




 struct SDDE_UAVCAN_DEVICE
{

	 int state;
	 int state_prev;
	 int E;
	 int counter;

	 int node_id;

	 void (*init)(struct SDDE_UAVCAN_DEVICE*);
	 void (*slow_calc)(struct SDDE_UAVCAN_DEVICE*);
	 void (*ms_calc)(struct SDDE_UAVCAN_DEVICE*);
	 
	 int (*can_init)();				 //this should be replaced by HAL implemantation
	 int (*can_tx)(CanardFrame*); //this should be replaced by HAL implemantation
	 int (*can_rx)(CanardFrame*); //this should be replaced by HAL implementation

} DDE_UAVCAN_DEVICE;

typedef struct SDDE_UAVCAN_DEVICE TDDE_UAVCAN_DEVICE;

#define DDE_UAVCAN_DEVICE_DEFAULTS { \
			0,0,0,0,\
			0,\
		DDE_UAVCAN_DEVICE_init,\
		DDE_UAVCAN_DEVICE_slow_calc,\
		DDE_UAVCAN_DEVICE_ms_calc,\
		DDE_UAVCAN_DEVICE_can_init,\
		DDE_UAVCAN_DEVICE_can_tx,\
		DDE_UAVCAN_DEVICE_can_rx\
		}

int DDE_UAVCAN_DEVICE_init(TDDE_UAVCAN_DEVICE*);
int DDE_UAVCAN_DEVICE_slow_calc(TDDE_UAVCAN_DEVICE*);
int DDE_UAVCAN_DEVICE_ms_calc(TDDE_UAVCAN_DEVICE*);

int DDE_UAVCAN_DEVICE_can_init();
int DDE_UAVCAN_DEVICE_can_tx(CanardFrame*);
int DDE_UAVCAN_DEVICE_can_rx(CanardFrame*);

#ifdef __cplusplus
}
#endif
