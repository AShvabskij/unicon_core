//---------------------------------------------------------------------------

#include "DDE.h"
#include "DDE_OSC.h"
#include "DDE_OSC_FILE.h"
#include "DDE_PARAMS_FILE.h"

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
    uint32_t res = _params->set(p);
    return res;
}
_dde_func_return_t DDE::get_params_header(DDE_GET_PARAMS_HEADER&p)
{
    uint32_t res = _params->get(p);
    return res;
}

_dde_func_return_t DDE::get_params_data(DDE_GET_PARAMS_DATA&p)
{
    int res;
    res = _params->get(p);
    return res;

}

_dde_func_return_t DDE::set_params_data(DDE_SET_PARAMS_DATA&p)
{
    int res;
    res= _params->set(p);
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

_dde_func_return_t DDE::init(char* system_type)
{
/*
    if (strcmp(system_type, "FILE") == 0) {
        _params = new DDE_PARAMS_FILE();
    } else {
        _params = new DDE_PARAMS();
    }
*/
    _params = new DDE_PARAMS();
    _params->init(system_type);

    if (strcmp(system_type, "FILE") == 0) {
        _osc = new DDE_OSC_FILE();
    } else if (strcmp(system_type, "MVCP") == 0) {
//      _osc = new OscMVCPWorker();
    } else {
        _osc = new DDE_OSC();
    }

    return 0;
}

void DDE::update()
{
      _params->update();
      //osc->update();
      //evlog->update();
      //trend->update();
}
