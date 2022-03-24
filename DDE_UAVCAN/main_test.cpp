
//DDE_template 
//Author A&D Mar 2021

//PROJECT UNICORN

#include <chrono>
#include <stdint.h>
#include <iostream>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <thread>


#include "DDE_UAVCAN.h"



//Hello world war 3
//xcvxcv
#include "RPI3B_SPI/RPI3B_drv_spi.h"
#include "RPI3B_SPI/RPI3B_SPI.h"
#include "CAN_MCP2518FD.h"
#include "CAN_ETHERNET.h"
#include "ipcmem_lib.h"

using namespace std;

DDE_UAVCAN* mDDE_UAVCAN;
static void print_modules(DDE_GET_PARAMS_HEADER& p);
static void print_params(int device_ID, int module_ID, DDE_GET_PARAMS_HEADER& p);
static void print_cmd_resp(int res, DDE_PARAMS_CMD& p);

static void thread_proc_test_app_call();

extern CAN_MCP2518FD can_mcp2518fd;

int main()
{

	std::cout << "DDE template started..." << std::endl;

																											/*auto spi = new RPI_SPI(0);
																											spi->init(0);*/
																											/*auto can0 = new CAN_MCP2518FD(spi);
																											can0->init(0);*/

#ifdef DEFINE_DDE_NIOS_NODE
	DRV_SPI_Initialize();
	uavcan_master.canInit = can_mcp2518fd.init;
	uavcan_master.canPop = can_mcp2518fd.canPop;
	uavcan_master.canPush = can_mcp2518fd.canPush;
	
#else
	CAN_ETHERNET* can_eth = new CAN_ETHERNET(0x10101010, 345);
	can_eth->init();
#endif
	/*DDE_GET_PARAMS_HEADER header;

	header.device_ID = 10;
	header.module_ID = 5;
	header.param_ID = 0;
	
	PARAMS_DESCR_get(header);

	header.module_name;
	for (int ii=0;ii<header.el_count;ii++)
	header.el_descr[ii].dim*/


	mDDE_UAVCAN = new DDE_UAVCAN();
	mDDE_UAVCAN->init(); 

	//DDE_GET_PARAMS_HEADER header;
	//header.device_ID = 11;
	//header.elem_ID = 0x105;

	//mDDE_UAVCAN->get_params_header(header);





																		//build params tree
																//DDE_GET_PARAMS_HEADER get_devices_header;

																		//std::cout << "HEADER el_count =" << get_devices_header.el_count << std::endl;

																		//for (int ii = 0; ii < get_devices_header.el_count; ii++) {
																		//	std::string s(get_devices_header.el_descr[ii].name);
																		//	std::cout << "	device name - " << s << " addr =" << get_devices_header.el_descr[ii].id << std::endl;

																		//	////try to get modules from device
																		//	DDE_GET_PARAMS_HEADER get_modules_header;
																		//	get_modules_header.device_ID = get_devices_header.el_descr[ii].id;

																		//	mDDE_UAVCAN->get_params_header(get_modules_header);
																		//	
																		//	print_modules(get_modules_header);

																		//}

	

	//start threads
	std::thread thread_test_app(thread_proc_test_app_call);
	thread_test_app.join();
	mDDE_UAVCAN->start();

	std::getchar();
}

static void thread_proc_test_app_call() {
	int res;
	DDE_GET_PARAMS_DATA get_params_data;
	DDE_SET_PARAMS_DATA set_params_data;
	DDE_PARAMS_CMD cmd;

	//DEVICE_ELEMENTS req;

	
	static uint32_t  value;
	while (1) {
											/*get_params_data.device_ID = 11;
											get_params_data.module_ID = 0x01;
											get_params_data.param_ID = 0x03;
											mDDE_UAVCAN->get_params_data(get_params_data);

											set_params_data.device_ID = 11;
											set_params_data.module_ID = 0x01;
											set_params_data.param_ID = 0x03;
											set_params_data.el.ivalue = value++;
											mDDE_UAVCAN->set_params_data(set_params_data);*/

//1) check IPCMEM for requiest
		
		
		res =PARAMS_DATA_read_cmd(cmd);

		if (res!=0) { 
			print_cmd_resp(res, cmd);

			if (cmd.nRW==1) { //SET request
				set_params_data.device_id = cmd.device_id;
				set_params_data.module_id = cmd.module_id;
				set_params_data.param_id = cmd.param_id;
				set_params_data.ivalue = cmd.ivalue;
				//mDDE_UAVCAN->set_params_data(set_params_data);
				
			}
			else {			 //GET requies		
				get_params_data.device_id = cmd.device_id;
				get_params_data.module_id = cmd.module_id;
				get_params_data.param_id = cmd.param_id;
				//mDDE_UAVCAN->get_params_data(get_params_data);
			}
		}


		usleep(10000); //0.1 sec
	} //end while (1) 

}


void print_cmd_resp(int res,DDE_PARAMS_CMD& p)
{

	std::cout <<	"dev_id=" << p.device_id<< \
					"mod_id="<<p.module_id<<\
					"par_id="<<p.param_id<<\
					"nRW="<<p.nRW<<\
					"res="<<res<<std::endl;

}


void print_modules(DDE_GET_PARAMS_HEADER& p)
{
		std::cout << "		modules_count =" << p.el_count << std::endl;

	for (int ii = 0; ii < p.el_count; ii++)
	{
		std::string s(p.el_descr[ii].name);
		std::cout << ii<<":                 module[" << hex<<p.el_descr[ii].id << "]  name = " << s << std::endl;

			DDE_GET_PARAMS_HEADER get_params_header;
			get_params_header.device_ID = p.device_ID;
			//get_params_header.param_ID = p.el_descr[ii].param_ID;
			mDDE_UAVCAN->get_params_header(get_params_header);
			print_params(p.device_ID,ii,get_params_header);
	}
	std::cout << std::endl;
}

void print_params(int device_ID, int module_ID, DDE_GET_PARAMS_HEADER& p)
{
	std::cout << "HEADER params_count =" << p.el_count << std::endl;

	for (int ii = 0; ii < p.el_count; ii++)
	{
		std::string s(p.el_descr[ii].name);
		std::cout << ii << ":param[" << hex << p.el_descr[ii].id << "]  name = " << s << std::endl;
	}
	std::cout << std::endl;
}

void str2hex();



	//Example for DLL interaction
	//dll_dde_params_request();

	/*std::cout << DDE_UAVCAN->evlog.get(&evlog_data) << std::endl;

	DDE_GET_PARAMS_DATA get_params;
	get_params.index = 100;
	get_params.sub_index = 1;
	DDE_UAVCAN->params.get(&get_params);
*/
//	struct tm * timeinfo;


	//DDE_UAVCAN->params.get()

	//for (int ii = 0; ii < 4; ii++) {
	//	timeinfo = localtime(&get_params.el[ii].timestamp);


	//	std::cout << "index=" << DDE_UAVCAN->params.get()
	//		.el[ii].index << \
	//		"ivalue=" << DDE_UAVCAN->.el[ii].ivalue << \
	//		"fvalue=" << DDE_UAVCAN->.el[ii].fvalue << \
	//		"time=" << asctime(timeinfo) << std::endl;
	//}


//
//	getchar();
//	return 0;
//}

