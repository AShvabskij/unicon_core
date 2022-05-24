#include "DDE_EMUL.h"

// #include "DDE_PARAMS_EMUL.h"
#include "DDE_OSC_EMUL.h"
#include "DDE_OSC_FILE.h"
#include "DDE_PARAMS_FILE.h"

_dde_func_return_t DDE_EMUL::init(char* sys_type)
{
    m_params = new DDE_PARAMS_FILE();
    m_params->init(sys_type);

    m_osc = new DDE_OSC_FILE();
    m_osc->init(sys_type);

	return 0;
}

_dde_func_return_t DDE_EMUL::get_params_header(DDE_GET_PARAMS_HEADER& p)
{
    return m_params->get(p);
}

_dde_func_return_t DDE_EMUL::get_params_data(DDE_GET_PARAMS_DATA& p)
{
    return m_params->get(p);
}

_dde_func_return_t DDE_EMUL::set_params_data(DDE_SET_PARAMS_DATA& p)
{
    return m_params->set(p);
}

_dde_func_return_t DDE_EMUL::get_osc_header(DDE_GET_OSC_HEADER& p)
{
    return m_osc->get(p);
}

_dde_func_return_t DDE_EMUL::get_osc_data(DDE_GET_OSC_DATA& p)
{
    return m_osc->get(p);
}

_dde_func_return_t DDE_EMUL::set_osc_data(DDE_SET_OSC_DATA&)
{
	return 0;
}


_dde_func_return_t DDE_EMUL::get_evlog_header(DDE_GET_EVLOG_HEADER& p)
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


_dde_func_return_t DDE_EMUL::get_evlog_data(DDE_GET_EVLOG_DATA& p)
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

_dde_func_return_t DDE_EMUL::set_evlog_data(DDE_SET_EVLOG_DATA&)
{
	return 0;
}

