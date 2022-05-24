#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <thread>

#include "DDE_TYPES.h"
#include "DDE_OSC_TYPES.h"
#include "DDE_INTERFACES.h"

#include "oscfiledataworker.h"

class DDE_OSC_FILE : public IDDE_OSC
{
public:
    DDE_OSC_FILE();

    virtual _dde_func_return_t init(char* /*system_type*/);
    virtual _dde_func_return_t open(uint16_t device_id);
    virtual _dde_func_return_t close(uint16_t device_id);

    virtual _dde_func_return_t get(DDE_GET_OSC_HEADER& p);
    virtual _dde_func_return_t get(DDE_GET_OSC_DATA& p);
    virtual _dde_func_return_t set(DDE_GET_OSC_HEADER& p);

private:
    OSC_VAR createOscVar(const OSC_FILE::VAR_DESCR& descr, uint16_t deviceId);

    OscFileDataWorker* m_worker = nullptr;
    
};
