
#include "DDE_PARAMS.h"

#include "ipcmem_lib.h"
#include "db_sqlib.h"


#include <string>
#include <cmath>
#include <chrono>
#include <unistd.h>



using namespace std;
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
DDE_PARAMS::DDE_PARAMS()
{
	//1) clear
	//memset(device, 0, sizeof(device));
	


	//2) fill with names devices
    /*static uint16_t amplitude = 10;
    static uint16_t frequency_hertz = 1;*/

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

_dde_func_return_t DDE_PARAMS::init(char* device_description)
{

	//	std::thread*thr_params = new std::thread(&DDE_PARAMS::thread_proc, this);
	PARAMS_DATA_init(device_description);


	DDE_SET_PARAMS_DATA set;
	for (int ii = 0; ii < 4096; ii++) {
		set.device_id = 2;
		set.module_id = (ii >> 6) & 0x3f;
		set.param_id = ii & 0x3f;
		set.ivalue = ii;
		PARAMS_DATA_direct_write(set);
	}
	DDE_GET_PARAMS_DATA get;
	get.device_id = 2;
	for (int ii = 0; ii < 64; ii++) {
		get.module_id = ii;
		get.param_id = 0;
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
std::string ascii_4chars_decode(uint32_t ivalue)//0x6D766370
{
	std::string res;
	char char_buf[5];
	uint32_t arr[5];

	arr[0] = (ivalue & 0xFF000000) >> 24;
	arr[1] = (ivalue & 0x00FF0000) >> 16;
	arr[2] = (ivalue & 0x0000FF00) >> 8;
	arr[3] =  ivalue & 0x000000FF;
	arr[4] = 0;

	for (int i = 0; i < 5; i++)
		char_buf[i] = (char)arr[i];

	res = char_buf;
	return res;
}

_dde_func_return_t DDE_PARAMS::get(DDE_GET_PARAMS_HEADER &p)
{
	int res;
	//we need to look in db for correct table according device_name and device_revision
	
	assert(p.device_id < DEVICE_ID_MAX);
	assert(p.module_id < MODULES_ID_MAX);
	assert(p.param_id < PARAMS_ID_MAX);
	
	DDE_GET_PARAMS_DATA dat;
	dat.device_id = p.device_id; dat.module_id = 0;

	dat.param_id = DDE_MODULE0_PARAM1_DEVICE_NAME; direct_read(dat);
	string name = ascii_4chars_decode(dat.el->ivalue);

	dat.param_id = DDE_MODULE0_PARAM2_HW_REV; direct_read(dat);
	string hw_rev = ascii_4chars_decode(dat.el->ivalue);

	dat.param_id = DDE_MODULE0_PARAM3_SW_REV; direct_read(dat);
	string sw_rev = ascii_4chars_decode(dat.el->ivalue);

	dat.param_id = DDE_MODULE0_PARAM4_SPARE; direct_read(dat);
	string spare = ascii_4chars_decode(dat.el->ivalue);

	string table_name = name + hw_rev + sw_rev + spare;

	ParamDescr* hdr = new ParamDescr();
	res = hdr->init(table_name.c_str(), "NONE", db_type::usual);
	res = hdr->get(&p, db_type::usual);

	return res;
}

//------------------------------------------------------------------------------

_dde_func_return_t DDE_PARAMS::set(DDE_SET_PARAMS_HEADER& p)
{
	int res;
	//assert(p.el.device_id < DEVICE_ID_MAX);
	assert(p.module_id < MODULES_ID_MAX);
	assert(p.param_id < PARAMS_ID_MAX);

	DDE_GET_PARAMS_DATA dat;
	dat.device_id = p.device_id; dat.module_id = 0;

	dat.param_id = DDE_MODULE0_PARAM1_DEVICE_NAME; direct_read(dat);
	string name = ascii_4chars_decode(dat.el->ivalue);

	dat.param_id = DDE_MODULE0_PARAM2_HW_REV; direct_read(dat);
	string hw_rev = ascii_4chars_decode(dat.el->ivalue);

	dat.param_id = DDE_MODULE0_PARAM3_SW_REV; direct_read(dat);
	string sw_rev = ascii_4chars_decode(dat.el->ivalue);

	dat.param_id = DDE_MODULE0_PARAM4_SPARE; direct_read(dat);
	string spare = ascii_4chars_decode(dat.el->ivalue);

	string table_name = name + hw_rev + sw_rev + spare;

	ParamDescr* hdr = new ParamDescr();
	res = hdr->init(table_name.c_str(), "NONE", db_type::usual);
	res = hdr->set(&p, db_type::usual);

	return res;
}

//
//------------------------------------------------------------------------------


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
		perror("if (get_queue.size< get_queue_max_size)");
		return -1;
    }

	//2) read params immediatly
	direct_read(p);
	
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
	
		perror("if (set_queue.size< get_queue_max_size)");
		return -1;
	}


	return 0;
}

_dde_func_return_t DDE_PARAMS::pop_next_get_request(DDE_GET_PARAMS_DATA& p)
{
	//DDE_GET_PARAMS_DATA p_data;

	if (list_get.empty()) return _return_FAIL;
	
	p = list_get.front();
	int a1 = list_get.size();
	list_get.pop_front();	
	int a2 = list_get.size();


	return _return_OK;
}

_dde_func_return_t DDE_PARAMS::pop_next_set_request(DDE_SET_PARAMS_DATA& p)
{
	if (list_set.empty()) return _return_FAIL;

	p = list_set.front();
	list_set.pop_front();
	return _return_OK;
}


// wrapper for IPCMEM
_dde_func_return_t DDE_PARAMS::direct_write(DDE_SET_PARAMS_DATA& set)
{
	time_t time;
	int el_id;



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
//wrapper for IPCMEM
_dde_func_return_t DDE_PARAMS::direct_read(DDE_GET_PARAMS_DATA& get_params)
{

	time_t system_time = systemTime();

	PARAMS_DATA_direct_read(get_params);

	//if (get_params.callback_func != NULL) get_params.callback_func();
}


//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
inline time_t DDE_PARAMS::systemTime()
{
    time_t timeMsc = std::chrono::duration_cast< std::chrono::milliseconds >(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();

    // std::time(&system_time);
    // std::cout << "time = " << timeMsc << "\n";

    return timeMsc;
}




void DDE_PARAMS::update()
{

	DDE_GET_PARAMS_DATA get_params;
	DDE_SET_PARAMS_DATA set_params;
	DDE_PARAMS_CMD cmd;


	//proceed GET and SET buffers - there are many options here.
	//A&D	- cmd_flag may hang as bottom service may be not active, so what to do?
	//		- calculate iterations 
	//		- do while both buffer not empty
	//if timeout then request maybe be lost as it already pop out of requiest list and not proceeded

		bool get_empty = false;	
		bool set_empty = false;
		bool timeout = false;
		uint32_t attempts = 0;
		while (!((get_empty && set_empty) || timeout))
		{
			int res = pop_next_get_request(get_params);// get_list.front();

			if (res == _return_OK) {
				uint8_t device_id = get_params.device_id;
				cmd.module_id = get_params.module_id;
				cmd.param_id = get_params.param_id;
				cmd.nRW = 0;
				res = 0; attempts = 0;
				while (res != 1) {
					res = PARAMS_DATA_write_cmd(device_id, cmd);
					if (res != 1) {
						attempts++;
						if (attempts > 10) timeout = true;
						usleep(100);
					}
				}

			}
			else
			{
				get_empty = true;
			}

			res = pop_next_set_request(set_params);// get_list.front();

			if (res == _return_OK) {
				uint8_t device_id = set_params.device_id;
				cmd.module_id = set_params.module_id;
				cmd.param_id = set_params.param_id;
				cmd.ivalue = set_params.ivalue;
				cmd.nRW = 1;
				res = 0; attempts = 0;
				while (res != 1) {
					res = PARAMS_DATA_write_cmd(device_id, cmd);
					if (res != 1) {
						attempts++;
						if (attempts > 10) timeout = true;
						usleep(100);
					}
				}
			}
			else
			{
				set_empty = true;
			}
		}


		if (timeout == true) perror("while ((get_empty && set_empty) || timeout) resulted with timeout");
}

