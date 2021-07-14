#pragma once

#include "DDE/DDE.h"
#include "DDE/DDE_PARAMS_EMUL.h"

class DDE_EMUL : public IDDE
{

public:

    DDE_EMUL();
    ~DDE_EMUL();

	_dde_func_return_t init(int mode);

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
    DDE_PARAMS_EMUL params;
/*
    DDE_OSC_EMUL osc;
    DDE_EVLOG_EMUL evlog;
*/
private:

};
