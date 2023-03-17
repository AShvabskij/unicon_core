//---------------------------------------------------------------------------

#include "DDE_TOP.h"
#include "DDE_PARAMS.h"
#include "DDE_OSC_DISPATCHER.h"
#include "DDE_EVLOG.h"

#include "stdint.h"
#include "stdlib.h"

//#include "ipcmem_lib.h"
//#include "db_sqlib.h"

DDE_TOP::DDE_TOP()
{


}

DDE_TOP::~DDE_TOP()
{
}

//void DDE_TOP::get_status(DDE_STATUS*p)
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

_dde_func_return_t DDE_TOP::set_params_header(DDE_SET_PARAMS_HEADER& p)
{
    _dde_func_return_t res = m_params->set(p);
    return res;
}
_dde_func_return_t DDE_TOP::get_params_header(DDE_GET_PARAMS_HEADER& p)
{
    _dde_func_return_t res = m_params->get(p);
    return res;
}

_dde_func_return_t DDE_TOP::get_params_data(DDE_GET_PARAMS_DATA& p)
{
    _dde_func_return_t res = m_params->get(p);
    return res;

}

_dde_func_return_t DDE_TOP::set_params_data(DDE_SET_PARAMS_DATA& p)
{
    _dde_func_return_t res = m_params->set(p);
    return res;
}


_dde_func_return_t DDE_TOP::get_osc_header(DDE_OSC_HEADER& p)
{
    m_osc->open(p.device_id);
    return m_osc->get(p);
}

_dde_func_return_t DDE_TOP::set_osc_header(DDE_OSC_HEADER& p)
{
    return m_osc->set(p);
}

_dde_func_return_t DDE_TOP::get_osc_data(DDE_GET_OSC_DATA& p)
{
    return m_osc->get(p);
}

_dde_func_return_t DDE_TOP::set_osc_data(DDE_SET_OSC_DATA& )
{
    return _return_OK; // m_osc->set(p);
}

_dde_func_return_t DDE_TOP::get_evlog_header(DDE_GET_EVLOG_HEADER& )
{
    return 0;
}

_dde_func_return_t DDE_TOP::get_evlog_data(DDE_GET_EVLOG_DATA& )
{
    return 0;
}

_dde_func_return_t DDE_TOP::set_evlog_data(DDE_SET_EVLOG_DATA& )
{
    return 0;
}

_dde_func_return_t DDE_TOP::init(const char* system_type)
{
    m_sysType = system_type;

    m_params = new DDE_PARAMS();
    m_params->init(system_type);

    m_osc = new DDE_OSC_DISPATCHER();
    m_osc->init(system_type);

    m_updThread = new std::thread(&DDE_TOP::thread_proc, this);

    return 0;
}

const char* DDE_TOP::system_type()
{
    return m_sysType.c_str();
}

void DDE_TOP::update()
{
    m_params->update();
    m_osc->update();

    //evlog->update();
    //trend->update();
}

int DDE_TOP::thread_proc()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    while (1)
    {

        update();

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
