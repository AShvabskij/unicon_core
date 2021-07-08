#include "DDE_CAN.h"


DDE_CAN::DDE_CAN()
{
}

DDE_CAN::~DDE_CAN()
{
}


_dde_func_return_t DDE_CAN::init(int mode)
{
	return 0;
}

_dde_func_return_t DDE_CAN::get_params_header(DDE_GET_PARAMS_HEADER& p)
{
	//this func provices description for device, modules and params
	

	//check valid input
    if (p.device_ID > 127 || p.elem_ID > PARAMS_ID_MAX ) {
		memset(&p, 0, sizeof(DDE_GET_PARAMS_HEADER));
		return -1;
	}

    uint8_t _index = (p.elem_ID>>6)&0x3f;
    uint8_t _subindex = p.elem_ID & 0x3f;

	//check level 1 request for device names
	if (p.device_ID == 0) {

		p.el_count = 0;// params.devices_count;
		for (int ii = 0; ii < 64; ii++) {
			if (params.device[ii].name[0] != 0) {		
				memcpy(&p.el_descr[p.el_count].name, &params.device[ii].name, DDE_PARAMS_NAME_LENGTH);
				p.el_descr[p.el_count].id = ii;
				p.el_count++;
			}
		}

	}
	else
	{
		//check level 2 (requiest for  modules names)
		if (_index == 0)
		{
			
			p.el_count = 0; // params.device[p.device_ID].modules_count;
			for (int ii = 0; ii < 64; ii++) {
				int module_id = (ii<<6);
				if (params.device[p.device_ID].el_descr[module_id].name[0] != 0) {
					memcpy(&p.el_descr[p.el_count], &params.device[p.device_ID].el_descr[module_id], sizeof(GLIO_ELEMENT_DESCR));
					p.el_descr[p.el_count].id = (ii << 6);
					p.el_count++;
				}
			}

		}
		else {
			if (_subindex == 0)  //level 3 request for params names
			{
	
				p.el_count = 0;// params.device[p.device_ID].el_descr[p.param_ID].params_count;
                for (int ii = p.elem_ID; ii < p.elem_ID + 64; ii++) {
                    if (params.device[p.device_ID].el_descr[p.elem_ID + ii].name[0] != 0)
					{
                        memcpy(&p.el_descr[p.el_count], &params.device[p.device_ID].el_descr[p.elem_ID + ii], sizeof(GLIO_ELEMENT_DESCR));
						p.el_descr[p.el_count].id = ii;
						p.el_count++;
					}
				}
			}
			else //level 4 (request for individual param name - not used
			{
				p.el_count = 1;
                memcpy(&p.el_descr[0], &params.device[p.device_ID].el_descr[p.elem_ID], sizeof(GLIO_ELEMENT_DESCR));
			}
		}
					
		
	}

    return 0;
}

_dde_func_return_t DDE_CAN::get_params_data(DDE_GET_PARAMS_DATA& p)
{
        return params.get(p);
}

_dde_func_return_t DDE_CAN::set_params_data(DDE_SET_PARAMS_DATA& p)
{
	return 0;
}


_dde_func_return_t DDE_CAN::get_osc_header(DDE_GET_OSC_HEADER& p)
{
	return 0;
}

_dde_func_return_t DDE_CAN::get_osc_data(DDE_GET_OSC_DATA& p)
{
	return 0;
}

_dde_func_return_t DDE_CAN::set_osc_data(DDE_SET_OSC_DATA& p) 
{
	return 0;
}


_dde_func_return_t DDE_CAN::get_evlog_header(DDE_GET_EVLOG_HEADER& p)
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


_dde_func_return_t DDE_CAN::get_evlog_data(DDE_GET_EVLOG_DATA& p)
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

_dde_func_return_t DDE_CAN::set_evlog_data(DDE_SET_EVLOG_DATA& p)
{
	return 0;
}

