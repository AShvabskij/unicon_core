#include "DDE_UAVCAN.h"

#include "DDE_PARAMS.h"
#include "DDE_OSC.h"
#include "DDE_EVLOG.h"
#include "DDE_UAVCAN_defs.h"




DDE_UAVCAN::DDE_UAVCAN()
{

}

DDE_UAVCAN::~DDE_UAVCAN()
{

}


_dde_func_return_t DDE_UAVCAN::init()
{

	
	params = new DDE_PARAMS();
    osc = new DDE_OSC();
    evlog = new DDE_EVLOG();

	params->init();
	//osc->

	//TDDE_UAVCAN_MASTER _uavcan_master = DDE_UAVCAN_MASTER_DEFAULTS;
	uavcan_master.init();

	//_uavcan_master.can_tx = &DDE_UAVCAN::can_tx; //TODO 
	//_uavcan_master.can_rx = &DDE_UAVCAN::can_rx;

	// Create thread for CAN
	thr_DDE_UAVCAN = new std::thread(&DDE_UAVCAN::thread_proc, this);



	return 0;
}
_dde_func_return_t DDE_UAVCAN::start()
{
	thr_DDE_UAVCAN->join();
}
/*-----------------------------------------------------------------------------------------*/
//
//
//
//
/*-----------------------------------------------------------------------------------------*/

int DDE_UAVCAN::thread_proc()
{
	while (1) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

		update();
	}
}

_dde_func_return_t DDE_UAVCAN::update()
{

	static uint16_t req_counter = 0;
	counter++;
	if (counter > 1000) {
		counter = 0;
		for (int ii = 1; ii < 127; ii++)
			if (device[ii].link == 1) printf("	dev %3d online;", ii);
		printf("\n");
	}

	//1 check CAN interface is not full
	//if (interfaceCAN->full() == true) return;


	//2 Update tx_transfer for params
	DDE_GET_PARAMS_DATA get_params;
	DDE_SET_PARAMS_DATA set_params;

	
	int res = params->pop_next_get_request(get_params);// get_list.front();

	if (res == _return_OK) {
		uint16_t addr = ((get_params.param_ID&0xff) << 8) | get_params.module_ID&0xff;
		//uint32_t ivalue = 100;
		uavcan_master.get_param(get_params.device_ID, addr);
		req_counter++;
		//printf("req_counter= %d\n", req_counter);
	}
	else
	{
		req_counter = 0;
	}

	res = params->pop_next_set_request(set_params);// get_list.front();

	if (res == _return_OK) {
		uint16_t addr = set_params.param_ID;// ((set_params.param_ID & 0xff) << 8) | set_params.module_ID & 0xff;
		//uint32_t ivalue = 100;
		uavcan_master.set_param(set_params.device_ID, addr, set_params.el.ivalue);
		req_counter++;
		//printf("req_counter= %d\n", req_counter);
	}
	else
	{
		req_counter = 0;
	}

	//Syncronous CAN trannsmite libcanard
	uavcan_master.TransmitProcess();

	//3 Update rx_transfer
	CanardRxTransfer rx_transfer;
	CanardRxTransfer* rx = &rx_transfer;

	res = uavcan_master.ReceiveProcess(rx); //was (&rx_transfer);
	if (res == 1) 
	{   //this is a node trying to say smthing
		if (rx->metadata.remote_node_id < 127) 
		{
			res=check_heatbeat(rx);
			if(res== _return_FAIL)
			res=check_params(rx);
			if (res== _return_FAIL)
			res= check_file_transfer(rx);
		}		// someone broadcasting	
		else if (rx->metadata.remote_node_id==255)
		{

		}
				// undefined yet	
		else
		{

		}
		if (rx->metadata.port_id== uavcan_node_Heartbeat_1_0_FIXED_PORT_ID_)


	uavcan_master.FreeReceiveBuff(&rx_transfer);
	}

	update_heartbeat();
	
	//emul file transfer always
	std::string file = "dict.txt";

	//update_file_transfer(11, 1, file);
	
	//		request_list.pop();

		//m_params->next_get_request

		//bool done = m_params->get_list.empty();
		//if (done) return;

		//m_params->next_set(set_params);

		//
		//m_params->direct_write();


}




_dde_func_return_t DDE_UAVCAN::check_osc(CanardRxTransfer* rx) {}




_dde_func_return_t DDE_UAVCAN::get_params_header(DDE_GET_PARAMS_HEADER& p)
{
    return params->get(p);
}

_dde_func_return_t DDE_UAVCAN::get_params_data(DDE_GET_PARAMS_DATA& p)
{
        return params->get(p);
}

_dde_func_return_t DDE_UAVCAN::set_params_data(DDE_SET_PARAMS_DATA& p)
{

	return params->set(p);
}


_dde_func_return_t DDE_UAVCAN::get_osc_header(DDE_GET_OSC_HEADER& /*p*/)
{
	return 0;
}

_dde_func_return_t DDE_UAVCAN::get_osc_data(DDE_GET_OSC_DATA& /*p*/)
{
	return 0;
}

_dde_func_return_t DDE_UAVCAN::set_osc_data(DDE_SET_OSC_DATA& /*p*/)
{
	return 0;
}


_dde_func_return_t DDE_UAVCAN::get_evlog_header(DDE_GET_EVLOG_HEADER& p)
{
	p.overflow = 0;

	if (p.device_ID != 0)
		p.evlog_param1 = 10; //number of msgs
	else 
	{
		p.evlog_param1 = 0x1000; //bit that some new msgs on device 12
		p.evlog_param2 = 0x0000; //bit that some new msgs on device
		p.evlog_param2 = 0x0000; //bit that some new msgs on device
		p.evlog_param2 = 0x0000; //bit that some new msgs on device
	}
	return 0;
}

_dde_func_return_t DDE_UAVCAN::get_evlog_data(DDE_GET_EVLOG_DATA& p)
{
	static uint32_t counter = 0;

	counter++;
	if (p.device_ID != 0)
	{
		p.msg_num = 1; //number of msgs
		p.msg[0].code_ID = counter&0xff;
		p.msg[0].source_ID = (counter>>5)&0x3f;
		p.msg[0].timestamp = time(NULL);
	}
	else
		p.msg_num = 0; 
	
	return 0;
}

_dde_func_return_t DDE_UAVCAN::set_evlog_data(DDE_SET_EVLOG_DATA& /*p*/)
{
	return 0;
}




//----------------------------------------------------------------------------------------------------------------------------
//void DDE_UAVCAN::GetParamsRequest2CanMsg(DDE_GET_PARAMS_DATA* get_params, CAN_MSG*can_msg)
//{
//
//}

_dde_func_return_t check_osc() {}
_dde_func_return_t check_evlog() {}
_dde_func_return_t check_rsu() {}


//void DDE_UAVCAN::MSG_Filter()
//{
//	//проверяем нет ли "свежих" сообщений в RX_FIFO
//	while (interface_CAN->RX_FIFO->get_num_of_msgs() != 0)
//	{
//		if (interface_CAN->RX_FIFO->read(&R_msg) == CAN_FIFO_SUCCESSFUL)
//		{
//			Receive_counter++;
//			//сообщение прочитано, далее необходимо его отфильтровать и
//			//положить куда нужно
//			if (((R_msg.id & 0x780) == CAN_OPEN_ID_EMERGENCY) && ((R_msg.id & 0x7F) != 0))
//			{//EMERGENCY
//
//				continue;
//			}
//			if ((R_msg.id & 0x780) == CAN_OPEN_ID_PDO1)
//			{//PDO1
//
//				continue;
//			}
//			if ((R_msg.id & 0x780) == CAN_OPEN_ID_PDO2)
//			{//PDO2
//
//				continue;
//			}
//			if ((R_msg.id & 0x780) == CAN_OPEN_ID_PDO3)
//			{//PDO3
//
//				continue;
//			}
//			if ((R_msg.id & 0x780) == CAN_OPEN_ID_PDO4)
//			{//PDO4
//
//				continue;
//			}
//			if ((R_msg.id & 0x780) == CAN_OPEN_ID_SDO_ANSWER)
//			{//пришел SDO ответ
//
//				SDO_counter++;
//				msg_repeater_cnt = 0;
//				if (SDO_FIFO->write(&R_msg) == CAN_FIFO_FULL)
//				{//Фифо заполнилось, нужно об этом сообщить
//				}
//				continue;
//			}
//			if ((R_msg.id & 0x780) == CAN_OPEN_ID_SDO_REQUEST)
//			{//пришел SDO запрос, но мы не обрабатываем запросы
//			 //поэтому не нужно обрабатывать это сообщение
//
//				continue;
//			}
//			if ((R_msg.id & 0x780) == CAN_OPEN_ID_HEARTBEAT)
//			{//пришел HEARTBEAT
//				if (HEARTBEAT_FIFO->write(&R_msg) == CAN_FIFO_FULL)
//				{//Фифо заполнилось, нужно об этом сообщить
//				}
//			}
//		}//if(h_CAN_module->RX_FIFO.read(&R_msg) == CAN_FIFO_SUCCESSFUL)
//
//	}// while(h_CAN_module->RX_FIFO.get_num_of_msgs() != 0)
//}