#pragma once

#include "DDE_TYPES.h"
#include "DDE_OSC_TYPES.h"
#include "DDE_INTERFACES.h"

class DDE_OSC_DISPATCHER : public IDDE_OSC
{
public:
    DDE_OSC_DISPATCHER();
	~DDE_OSC_DISPATCHER();

    virtual _dde_func_return_t init(const char* sys_type);
    virtual _dde_func_return_t open(uint16_t oscId);
    virtual _dde_func_return_t close();

    virtual _dde_func_return_t get(DDE_OSC_HEADER&);
    virtual _dde_func_return_t set(const DDE_OSC_HEADER&);
    virtual _dde_func_return_t get(DDE_GET_OSC_DATA&);
    virtual _dde_func_return_t set(const DDE_SET_OSC_DATA&);

    virtual void update();

private:
    IDDE_OSC* m_osc = nullptr;
};
