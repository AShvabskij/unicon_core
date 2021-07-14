//---------------------------------------------------------------------------


#include "DDE.h"

#include "stdint.h"
#include "stdlib.h"
//#include "unistd.h"

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

_dde_func_return_t DDE::get_params_header(DDE_GET_PARAMS_HEADER&)
{
    return 0;
}

_dde_func_return_t DDE::get_params_data(DDE_GET_PARAMS_DATA&)
{
    return 0;
}

_dde_func_return_t DDE::set_params_data(DDE_SET_PARAMS_DATA&)
{
    return 0;
}


_dde_func_return_t DDE::get_osc_header(DDE_GET_OSC_HEADER&)
{
    return 0;
}

_dde_func_return_t DDE::get_osc_data(DDE_GET_OSC_DATA&)
{
    return 0;
}

_dde_func_return_t DDE::set_osc_data(DDE_SET_OSC_DATA&)
{
    return 0;
}

_dde_func_return_t DDE::get_evlog_header(DDE_GET_EVLOG_HEADER&)
{
    return 0;
}

_dde_func_return_t DDE::get_evlog_data(DDE_GET_EVLOG_DATA&)
{
    return 0;
}

_dde_func_return_t DDE::set_evlog_data(DDE_SET_EVLOG_DATA&)
{
    return 0;
}

//void DDE::params_callback(int) { return ; }
//void DDE::dlog_callback(int) { return ; }
//void DDE::evlog_callback(int) { return ; }
//void DDE::trend_callback(int) { return ; }

_dde_func_return_t DDE::init(int mode)
{
    //params = new DDE_PARAMS(NULL);
    //evlog = new DDE_EVLOG(NULL);

    //interface_can.init();

    //{
    //	node
    //	{
    //	params->init();
    //	evlog->init();
    //	osc->init(); }
    //
    //}
    return 0;
}
