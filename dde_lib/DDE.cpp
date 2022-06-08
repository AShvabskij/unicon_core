//---------------------------------------------------------------------------

#include "DDE.h"

#include "stdint.h"
#include "stdlib.h"

//#include "ipcmem_lib.h"
//#include "db_sqlib.h"

DDE::DDE()
{


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

_dde_func_return_t DDE::set_params_header(DDE_SET_PARAMS_HEADER& p)
{
    uint32_t res = params->set(p);
    return res;
}
_dde_func_return_t DDE::get_params_header(DDE_GET_PARAMS_HEADER&p)
{
    uint32_t res = params->get(p);
    return res;
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
    params = new DDE_PARAMS();

   	params->init(device_description);
    //params.
    //	evlog->init();
    //	osc->init(); }

    return 0;
}



void DDE::update()
{
      params->update();  
      //osc->update();
      //evlog->update();
      //trend->update();
}