#pragma once

//#include "DDE_UAVCAN.h"
#include "libcanard-2/libcanard/canard.h"
//#include "interface_CAN.h"

///(interface_CAN* interface_CAN, uint8_t node_ID);

#define UAVCAN_DEVICES_MAX		127
#define UAVCAN_PARAMS_ID_MAX	0xfff




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




struct SDDE_UAVCAN_MASTER
{

	int state;
	int state_prev;
	int E;
	int counter;

	int node_id;

	//void (*slow_calc)();
	int (*init)();		
	int (*ReceiveProcess)(CanardRxTransfer*);
	int (*FreeReceiveBuff)(CanardRxTransfer*);
	int (*TransmitProcess)();


	int (*get_param)(uint16_t node_id, uint16_t param_id);
	int (*set_param)(uint16_t node_id, uint16_t param_id, uint32_t ivalue);
	int (*file_request)(uint16_t node_id, uint16_t file_id, uint16_t addr, uint16_t size);

	long (*canInit)();				//this should be replaced by HAL implemantation
	long (*canPop)(CanardFrame*);	//this should be replaced by HAL implemantation
	long (*canPush)(CanardFrame*);	//this should be replaced by HAL implementation

};

typedef struct SDDE_UAVCAN_MASTER TDDE_UAVCAN_MASTER;

#define DDE_UAVCAN_MASTER_DEFAULTS { \
									0,0,0,0,\
									0,\
		DDE_UAVCAN_MASTER_init,\
		DDE_UAVCAN_MASTER_ReceiveProcess,\
		DDE_UAVCAN_MASTER_FreeReceiveBuff,\
		DDE_UAVCAN_MASTER_TransmitProcess,\
		DDE_UAVCAN_MASTER_get_param,\
		DDE_UAVCAN_MASTER_set_param,\
		DDE_UAVCAN_MASTER_file_request,\
		DDE_UAVCAN_MASTER_canInit,\
		DDE_UAVCAN_MASTER_canPop,\
		DDE_UAVCAN_MASTER_canPush\
		}

	void DDE_UAVCAN_MASTER_init();
	//void DDE_UAVCAN_MASTER_slow_calc();
	int DDE_UAVCAN_MASTER_ReceiveProcess(CanardRxTransfer* rx_transfer);
	int DDE_UAVCAN_MASTER_FreeReceiveBuff(CanardRxTransfer* rx_transfer);

	int DDE_UAVCAN_MASTER_TransmitProcess();


	int DDE_UAVCAN_MASTER_get_param(uint16_t node_id, uint16_t param_id);
	int DDE_UAVCAN_MASTER_set_param(uint16_t node_id, uint16_t param_id, uint32_t ivalue);
	
	int DDE_UAVCAN_MASTER_file_request(uint16_t node_id, uint16_t file_id, uint16_t addr, uint16_t size);
	
	long DDE_UAVCAN_MASTER_canInit();
	long DDE_UAVCAN_MASTER_canPush(CanardFrame* );
	long DDE_UAVCAN_MASTER_canPop(CanardFrame* );

	extern TDDE_UAVCAN_MASTER uavcan_master;
	

#ifdef __cplusplus
}
#endif