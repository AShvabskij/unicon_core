//---------------------------------------------------------------------------

#include "DDE.h"

#include "stdint.h"
#include "stdlib.h"

DDE::DDE()
{
    //if (sdasd)
    //evlog = new EVLOG_CAN();

    /*REQ_FIFO = new DDE_FIFO(DDE_FIFO_size);
    */
    //status.open = false;
    //status.device = 0;
    //status.baudrate = 0;
    //status.error = 0;
    //status.last_error_source[0] = 0;
}

DDE::~DDE()
{
}

//void DDE::get_status(DDE_STATUS*p)
//{
//	p->open = status.open;
//	p->device = status.device;
//	p->baudrate = status.baudrate;
//	p->error = status.error;
//	//strcpy(p->last_error_source, status.last_error_source);
//	//status.error = 0;
//	//status.last_error_source[0] = 0;
//
//}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------

_dde_func_return_t DDE::get_params_header(DDE_GET_PARAMS_HEADER&p)
{
    return 0;
}

_dde_func_return_t DDE::get_params_data(DDE_GET_PARAMS_DATA&p)
{
    int res;
    res = params->get(p);
    return res;

}

_dde_func_return_t DDE::set_params_data(DDE_SET_PARAMS_DATA&p)
{
    int res;
    res= params->set(p);
    return res;
}


_dde_func_return_t DDE::get_osc_header(DDE_GET_OSC_HEADER&p)
{
    return 0;
}

_dde_func_return_t DDE::get_osc_data(DDE_GET_OSC_DATA&p)
{
    return 0;
}

_dde_func_return_t DDE::set_osc_data(DDE_SET_OSC_DATA&p)
{
    return 0;
}

_dde_func_return_t DDE::get_evlog_header(DDE_GET_EVLOG_HEADER&p)
{
    return 0;
}

_dde_func_return_t DDE::get_evlog_data(DDE_GET_EVLOG_DATA&p)
{
    return 0;
}

_dde_func_return_t DDE::set_evlog_data(DDE_SET_EVLOG_DATA&p)
{
    return 0;
}

_dde_func_return_t DDE::init(char* device_description)
{
    DDE_PARAMS* params = new DDE_PARAMS();

   	params->init(device_description);

    //	evlog->init();
    //	osc->init(); }

    return 0;
}

void DDE::update()
{

    DDE_GET_PARAMS_DATA get_params;
    DDE_SET_PARAMS_DATA set_params;
    DDE_PARAMS_CMD cmd;

    int res = params->pop_next_get_request(get_params);// get_list.front();

    if (res == _return_OK) {
        cmd.device_id = get_params.device_id;
        cmd.module_id = get_params.module_id;
        cmd.param_id = get_params.param_id;
        cmd.nRW = 0;
        //      PARAMS_DATA_write_cmd(cmd);
    }
    else
    {
        //		req_counter = 0;
    }

    res = params->pop_next_set_request(set_params);// get_list.front();

    if (res == _return_OK) {
        cmd.device_id = set_params.device_id;
        cmd.module_id = set_params.module_id;
        cmd.param_id = set_params.param_id;
        cmd.ivalue = set_params.ivalue;
        cmd.nRW = 1;
        //    PARAMS_DATA_write_cmd(cmd);
    }
    else
    {
        //		req_counter = 0;
    }
}