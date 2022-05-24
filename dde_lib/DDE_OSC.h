#pragma once

#include "DDE_TYPES.h"
#include "DDE_OSC_TYPES.h"
#include "DDE_INTERFACES.h"

class IOscDataWorker;

class DDE_OSC : public IDDE_OSC
{
public:
	DDE_OSC();
	~DDE_OSC();

    virtual _dde_func_return_t init(char* system_type);
    virtual _dde_func_return_t open(uint16_t deviceId);
    virtual _dde_func_return_t close(uint16_t deviceId);

    virtual _dde_func_return_t get(DDE_GET_OSC_HEADER& p);
    virtual _dde_func_return_t get(DDE_GET_OSC_DATA& p);
    virtual _dde_func_return_t set(DDE_GET_OSC_HEADER& p);
};
