#pragma once
//---------------------------------------------------------------------------

#include "DDE_TYPES.h"
#include "DDE_PARAMS_TYPES.h"
#include "DDE_OSC_TYPES.h"
#include "DDE_EVLOG_TYPES.h"

//---------------------------------------------------------------------------

class IDDE
{
public:

    virtual ~IDDE() {};

    virtual _dde_func_return_t init(int mode) = 0;
    virtual _dde_func_return_t get_subsystem_header(DDE_GET_SUBSYSTEM& p) = 0;

    virtual _dde_func_return_t get_params_header(DDE_GET_PARAMS_HEADER& p) = 0;
    virtual _dde_func_return_t get_params_data(DDE_GET_PARAMS_DATA& p) = 0;
    virtual _dde_func_return_t set_params_data(DDE_SET_PARAMS_DATA& p) = 0;

    virtual _dde_func_return_t get_osc_header(DDE_GET_OSC_HEADER& p) = 0;
    virtual _dde_func_return_t get_osc_data(DDE_GET_OSC_DATA& p) = 0;
    virtual _dde_func_return_t set_osc_data(DDE_SET_OSC_DATA& p) = 0;

    virtual _dde_func_return_t get_evlog_header(DDE_GET_EVLOG_HEADER& p) = 0;
    virtual _dde_func_return_t get_evlog_data(DDE_GET_EVLOG_DATA& p) = 0;
    virtual _dde_func_return_t set_evlog_data(DDE_SET_EVLOG_DATA& p) = 0;
};

class DDE : public IDDE
{
public:

    DDE();
    virtual ~DDE();

    virtual _dde_func_return_t init(int mode);
    virtual _dde_func_return_t get_subsystem_header(DDE_GET_SUBSYSTEM& p);

    virtual _dde_func_return_t get_params_header(DDE_GET_PARAMS_HEADER& p);
    virtual _dde_func_return_t get_params_data(DDE_GET_PARAMS_DATA& p);
    virtual _dde_func_return_t set_params_data(DDE_SET_PARAMS_DATA& p);

    virtual _dde_func_return_t get_osc_header(DDE_GET_OSC_HEADER& p);
    virtual _dde_func_return_t get_osc_data(DDE_GET_OSC_DATA& p);
    virtual _dde_func_return_t set_osc_data(DDE_SET_OSC_DATA& p);

    virtual _dde_func_return_t get_evlog_header(DDE_GET_EVLOG_HEADER& p);
    virtual _dde_func_return_t get_evlog_data(DDE_GET_EVLOG_DATA& p);
    virtual _dde_func_return_t set_evlog_data(DDE_SET_EVLOG_DATA& p);

protected: // Protected members are accessible in the class that defines them and in classes that inherit from that class.
    IDDE_PARAMS *m_params;
    IDDE_OSC *m_osc;
    IDDE_EVLOG *m_evlog;
};

////virtual _dde_func_return_t get_params_header(DDE_GET_PARAMS_HEADER& p);// = 0;
//	{
//		IParamsService* m_params_service;
//		m_params_service = interface.getParamsService(p.itreafeceID);
//		if 
//		m_params_service->get_params_header()
//
//		if (p.itreafeceID == INTERFACE_CAN_ID)
//		{
//			interface_can.get_params_header();
//		}
//		else if (p.itreafeceID == INTERFACE_MBUS_ID)
//		{
//			interface_mbus.get_params_header();
//		}
//	}

//virtual _dde_func_return_t get_params_data(DDE_GET_PARAMS_DATA& p);// = 0;

//virtual _dde_func_return_t get_interfaces_header(DDE_GET_INTERFACES_HEADER& p);// = 0;

