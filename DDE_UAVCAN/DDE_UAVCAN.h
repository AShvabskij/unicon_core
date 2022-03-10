#pragma once

#include "DDE.h"

#include "interface_CAN.h"
#include "DDE_UAVCAN_MASTER.h"
//#include "DDE_UAVCAN_DEVICE.h"
//#include "DDE_UAVCAN_VIRT_DEVICES.h"


//static const uint16_t HeartbeatSubjectID = 7509;
//static const uint16_t RegisterAccessServiceID = 1000;

/// Should match the wildcard "uavcan.pub.*" for publishers. Likewise, "uavcan.sub.*" for subscribers.
//* static const char MeasurementSubjectIDRegisterName[] = "uavcan.pub.measurement";

//* static uint16_t g_measurement_subject_id = UINT16_MAX;

struct DDE_DEVICE
{
	int heartbeat_counter = 10000;
	int link;
	
};

class DDE_UAVCAN : public DDE
{


protected: // Protected members are accessible in the class that defines them and in classes that inherit from that class.
//DDE_STATUS status;

private: // Private members are only accessible within the class defining them.
	
	uint16_t state;
	uint16_t state_prev;
	uint16_t E;
	uint16_t counter;

	DDE_DEVICE device[127];
	
	DDE_PARAMS* params;
	IDDE_OSC* osc;
	IDDE_EVLOG* evlog;

	//interface_CAN* i_CAN; interface in not allowed to mix with c-style programming. May be improved lately
	

	_dde_func_return_t update();
	//_dde_func_return_t update_params();
	_dde_func_return_t check_evlog(CanardRxTransfer* rx);
	_dde_func_return_t check_rsu(CanardRxTransfer* rx);
	_dde_func_return_t check_heatbeat(CanardRxTransfer* rx);
	_dde_func_return_t check_params(CanardRxTransfer* rx);
	_dde_func_return_t check_osc(CanardRxTransfer* rx);
	_dde_func_return_t check_file_transfer(CanardRxTransfer* rx);
	_dde_func_return_t update_file_transfer(int dev_id, int file_id,std::string& file_name);
	_dde_func_return_t update_heartbeat();
	_dde_func_return_t update_osc();

	_dde_func_return_t save_dict(std::string& hash, uint8_t* buff, uint16_t size);
	_dde_func_return_t load_dict(std::string& hash);


	std::thread*thr_DDE_UAVCAN;

	int thread_proc();
public:

	DDE_UAVCAN();
	~DDE_UAVCAN();

	_dde_func_return_t init();
	_dde_func_return_t start();
	_dde_func_return_t get_params_header(DDE_GET_PARAMS_HEADER& p);
	_dde_func_return_t get_params_data(DDE_GET_PARAMS_DATA& p);
	_dde_func_return_t set_params_data(DDE_SET_PARAMS_DATA& p);

	_dde_func_return_t get_osc_header(DDE_GET_OSC_HEADER& p);
	_dde_func_return_t get_osc_data(DDE_GET_OSC_DATA& p);
	_dde_func_return_t set_osc_data(DDE_SET_OSC_DATA& p) ;

	_dde_func_return_t get_evlog_header(DDE_GET_EVLOG_HEADER& p);
	_dde_func_return_t get_evlog_data(DDE_GET_EVLOG_DATA& p);
	_dde_func_return_t set_evlog_data(DDE_SET_EVLOG_DATA& p);
};

