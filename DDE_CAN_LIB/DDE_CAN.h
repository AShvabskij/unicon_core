#pragma once

#include "DDE/DDE.h"

class DDE_CAN : public DDE
{


protected: // Protected members are accessible in the class that defines them and in classes that inherit from that class.
//DDE_STATUS status;

private: // Private members are only accessible within the class defining them.

public:

	DDE_CAN();
	~DDE_CAN();

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
};

