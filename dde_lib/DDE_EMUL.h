#pragma once

#include "DDE.h"

class DDE_EMUL : public IDDE
{

public:

    DDE_EMUL() = default;
    ~DDE_EMUL() = default;

    _dde_func_return_t init(char* device_description);


	_dde_func_return_t get_params_header(DDE_GET_PARAMS_HEADER& p);
	_dde_func_return_t get_params_data(DDE_GET_PARAMS_DATA& p);
	_dde_func_return_t set_params_data(DDE_SET_PARAMS_DATA& p);

	_dde_func_return_t get_osc_header(DDE_GET_OSC_HEADER& p);
	_dde_func_return_t get_osc_data(DDE_GET_OSC_DATA& p);
	_dde_func_return_t set_osc_data(DDE_SET_OSC_DATA& p) ;

	_dde_func_return_t get_evlog_header(DDE_GET_EVLOG_HEADER& p);
	_dde_func_return_t get_evlog_data(DDE_GET_EVLOG_DATA& p);
	_dde_func_return_t set_evlog_data(DDE_SET_EVLOG_DATA& p);

protected:
    IDDE_PARAMS *m_params;
    IDDE_OSC *m_osc;
//  IDDE_EVLOG *m_evlog;

};
