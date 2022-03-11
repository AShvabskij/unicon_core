
#include "ipcmem.h"
#include "sqlite3.h"

#include <string>
#include <cmath>
#include <chrono>


#include "DDE_PARAMS.h"
#include "ipcmem_lib.h"
using namespace std;
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
DDE_PARAMS::DDE_PARAMS()
{
	//1) clear
	//memset(device, 0, sizeof(device));
	


	//2) fill with names devices
    static uint16_t amplitude = 10;
    static uint16_t frequency_hertz = 1;

	//for (int ii = 1; ii < 33; ii=ii+11)
	//{
	//	devices_count++;
	//	//device[ii].device_ID = ii; A&D exluded as duplication
	//	sprintf(device[ii].name, "Device Power Unit Type %d", ii);
	//	
	//	
	//	string s;

	//	int param_count = 2;// (rand() / RAND_MAX) * 60 + 3;

	//	//fill device with random params
	//	for (int jj = 0; jj <=param_count; jj++)
	//	{
	//		s = "module_" + to_string(jj);
	//		for (int subix = 0; subix < 4; subix++) {
	//			int param_ID = (jj << 6) + subix;
	//			device[ii].el_descr[param_ID].id = param_ID; // (jj << 6) + subix;

	//			device[ii].el[param_ID].format = GLIO_ELEMENT_FORMAT_ENUM::FORMAT_INT;
	//			device[ii].el[param_ID].ivalue = -1;
	//			device[ii].el[param_ID].fvalue = -1;
	//			string s1;
	//			s1 = s + "_param_"+ to_string(subix);
	//			strcpy(device[ii].el_descr[param_ID].name, s1.c_str());
	//			device[ii].el[param_ID].scale = 0;
	//			device[ii].el[param_ID].timestamp = 0;
	//		}
	//	}
	//}

	//this->list_get_max = 10; // fifo_size;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
DDE_PARAMS::~DDE_PARAMS()
{

}

_dde_func_return_t DDE_PARAMS::init() {

//	std::thread*thr_params = new std::thread(&DDE_PARAMS::thread_proc, this);
	PARAMS_DATA_init("UAVCAN");
	
	DDE_PARAMS_CMD cmd;
	cmd.device_id = 0x2;
	cmd.cmd_flag = 1;
	cmd.nRW = 1;

	PARAMS_DATA_write_cmd(cmd);

	DDE_SET_PARAMS_DATA set;
	for (int ii = 0; ii < 4096; ii++) {
		set.device_ID = 2;
		set.module_ID = (ii >> 6) & 0x3f;
		set.param_ID = ii & 0x3f;
		set.el.ivalue = ii;
		PARAMS_DATA_direct_write(set);
	}
	DDE_GET_PARAMS_DATA get;
	get.device_ID = 2;
	for (int ii = 0; ii < 64; ii++) {
		get.module_ID = ii;
		get.param_ID = 0;
		PARAMS_DATA_direct_read(get);
		printf("module=%d ", ii);
		for (int yy = 0; yy < 64; yy++)
			printf("%d ", get.el[yy].ivalue);
		printf("\n");
	}

	//PARAMS_DESCR_init("UAVCAN"); // from SQLite3_lib

	//thr_params.join();
	return 0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
_dde_func_return_t DDE_PARAMS::get(DDE_GET_PARAMS_HEADER &p)
{


	//PARAMS_DESCR_get(p);

    return 0;
}

_dde_func_return_t DDE_PARAMS::get(DDE_GET_PARAMS_DATA& p)
{
	int ii = 0;

	//TODO - add check that requiest is not already in the queue. If it is do not push it. 

	//0) set timeout counter to 0	
	p.timeout = 0;

	//1) Add request to queue
    if (list_get.size() < list_get_max) {
		list_get.push_back(p);
    } else {
		assert("if (get_queue.size< get_queue_max_size)");
    }

	//2) read params immediatly
	//direct_read(p);
	
	return _return_OK;

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
_dde_func_return_t DDE_PARAMS::set(DDE_SET_PARAMS_DATA& p)
{

	int ii = 0;

	//TODO - add check that requiest is not already in the queue. If it is do not push it. 

	//0) set timeout counter to 0	
	//p.timeout = 0;

	//1) Add request to queue
	if (list_set.size() < list_set_max) {
		list_set.push_back(p);
	}
	else {
		assert("if (set_queue.size< get_queue_max_size)");
	}


	return 0;
}

_dde_func_return_t DDE_PARAMS::pop_next_get_request(DDE_GET_PARAMS_DATA& p)
{
	//DDE_GET_PARAMS_DATA p_data;

	if (list_get.empty()) return _return_FAIL;
	
	p = list_get.front();
	list_get.pop_front();	
	return _return_OK;
}

_dde_func_return_t DDE_PARAMS::pop_next_set_request(DDE_SET_PARAMS_DATA& p)
{
	if (list_set.empty()) return _return_FAIL;

	p = list_set.front();
	list_set.pop_front();
	return _return_OK;
}

_dde_func_return_t DDE_PARAMS::direct_write(DDE_SET_PARAMS_DATA& set)
{
	time_t time;
	int el_id;
	assert(set.device_ID < DEVICE_ID_MAX);
	assert(set.param_ID < DEVICE_ID_MAX);

																	//el_id = set.param_ID;// (set.module_ID << 6) | set.param_ID;

																	/*device[set.device_ID].el->ivalue = set.el.ivalue;
																	if (set.el.timestamp == 0) {
																		localtime(&time);
																		device[set.device_ID].el->timestamp = time;
																	}
																	else
																	device[set.device_ID].el[el_id].timestamp = set.el.timestamp;*/

	PARAMS_DATA_direct_write(set);
	
	return _return_OK;
}
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
_dde_func_return_t DDE_PARAMS::direct_read(DDE_GET_PARAMS_DATA& get_params)
{

	time_t system_time = systemTime();

	PARAMS_DATA_direct_read(get_params);



	
	//if (get_params.callback_func != NULL) get_params.callback_func();
}

inline time_t DDE_PARAMS::systemTime()
{
    time_t timeMsc = std::chrono::duration_cast< std::chrono::milliseconds >(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();

    // std::time(&system_time);
    // std::cout << "time = " << timeMsc << "\n";

    return timeMsc;
}



